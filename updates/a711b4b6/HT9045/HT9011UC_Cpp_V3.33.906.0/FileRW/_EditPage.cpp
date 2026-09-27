// ===========================================================================
//  FileRW/_EditPage.cpp -- C 路共用頁面層（見 _EditPage.h）。規則照 FileRW/IniConfig.cpp（第一個 C 路結構）。
//  Steven 20260924.  NOT in golden.
// ===========================================================================
#include "FileRW/_EditPage.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <exception>
#include <map>
#include <vector>

#include "FileRW/_EditList.h"
#include "Public/cJSON.h"
#include "cmydef.h"                 // AccessLevel
#include "WebBridge/JsonWriter.h"

namespace filerw {

namespace {
std::map<std::string, const PageDesc*>& Reg() {
    static std::map<std::string, const PageDesc*> r;
    return r;
}
// 審查 H-A／M-B（IniConfig 同規則）：golden 的 Enabled 是開頁 FormShow 時依當下 AccessLevel 算的
struct Shown { bool shown = false; int level = -1; };
std::map<std::string, Shown>& ShownOf() {
    static std::map<std::string, Shown> s;
    return s;
}
std::vector<HTEditList*> ListsOf(const PageDesc& d) {
    std::vector<HTEditList*> v;
    for (int i = 0; i < d.nLists; ++i) v.push_back(*d.lists[i]);
    return v;
}
bool InLists(const PageDesc& d, TControl* c) {
    std::vector<HTEditList*> v = ListsOf(d);
    return c && ELInAnyList(c, v.data(), (int)v.size());
}
bool Contains(const std::vector<std::string>& v, const std::string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}
std::string EvPageJson(const PageDesc& d);   // AI(W906-FRW-S157) 20260927 [W906]：本檔下方（form.event 段）
}  // namespace

void RegisterPage(const PageDesc* d) { Reg()[d->tag] = d; }

const PageDesc* FindPage(const std::string& tag) {
    std::map<std::string, const PageDesc*>::const_iterator it = Reg().find(tag);
    return it == Reg().end() ? nullptr : it->second;
}

int PageJson(const PageDesc& d, std::string* json) {
    if (!d.booted()) { *json = std::string(d.tag) + " edit lists are not booted"; return 409; }
    SessionBegin("");   // 審查第 8 輪 L-1：golden FormShow 裡的 ELTodo／ELMessage 回給頁面（例 Tray CSV 未移植）
    d.formShow();
    ShownOf()[d.tag].shown = true;
    ShownOf()[d.tag].level = AccessLevel;
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("struct").String(d.tag);
    w.Key("form").String(d.form);
    w.Key("booted").Bool(true);
    w.Key("lists").BeginObject();
    for (int i = 0; i < d.nLists; ++i) w.Key(d.listNames[i]).RawValue(EditListToJson(*d.lists[i], d.form));
    w.EndObject();
    w.Key("proxies").RawValue(ProxyStateJson(d.form));
    w.Key("session").RawValue(SessionJson());
    w.Key("mustSend").BeginArray();
    for (int i = 0; i < d.nSaveReads; ++i)
        if (!InLists(d, ELFind(d.form, d.saveReads[i]))) w.String(d.saveReads[i]);
    w.EndArray();
    if (d.extraJson) w.Key("extra").RawValue(d.extraJson());   // Steven 團隊 20260925：可選（見 _EditPage.h）
    // AI(W906-FRW-S157) 20260927 [W906]：WS form.event —— 接了事件的頁（TrayForm、Cleaning）多帶 "events"：
    //   {控制項:{event, golden, items?, operable}}。items＝golden FormShow 剛填的下拉清單（proxies 只帶 itemIndex／text，
    //   沒有 Items —— 例 cbTrayType1..3／cbbSelectTray 的 Tray 表清單只能從這裡拿）；沒接事件的頁不多這個鍵（回應不變）。
    { const std::string ev = EvPageJson(d); if (!ev.empty()) w.Key("events").RawValue(ev); }
    w.EndObject();
    *json = w.Str();
    return 200;
}

int PageSave(const PageDesc& d, const std::string& widgetsJson, const std::string& answersJson,
             std::string* ack, std::string* err) {
    if (!d.booted()) { *err = std::string(d.tag) + " edit lists are not booted"; return 409; }
    const Shown sh = ShownOf()[d.tag];
    if (!sh.shown || sh.level != AccessLevel) {
        *err = std::string("reload page: open the page (editlist.get ") + d.tag +
               " = golden FormShow) with the current access level before saving";
        return 409;
    }
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        *err = "widgets is not a JSON object";
        return 400;
    }
    // 必送（先於套用）
    std::vector<std::string> need;
    for (int i = 0; i < d.nSaveReads; ++i) {
        if (InLists(d, ELFind(d.form, d.saveReads[i]))) continue;
        if (!cJSON_GetObjectItemCaseSensitive(root, d.saveReads[i])) need.push_back(d.saveReads[i]);
    }
    if (!need.empty()) {
        cJSON_Delete(root);
        *err = "refused: golden save reads these widgets, they are not in any HTEditList (golden FormShow fills them), "
               "and the page did not send them:";
        for (std::size_t i = 0; i < need.size(); ++i) *err += (i ? ", " : " ") + need[i];
        return 400;
    }
    for (int i = 0; i < d.nSaveReads; ++i)
        if (!ELFind(d.form, d.saveReads[i])) {
            cJSON_Delete(root);
            *err = std::string("internal: save proxy not created: ") + d.saveReads[i];
            return 500;
        }
    // Steven 團隊 20260925：套值前的 golden 事件（可選，見 _EditPage.h beforeApply）。在「不可改的丟掉」之前：
    // 事件改過的可見／可改（例：新 Test Mode 的 Site 格子）才是丟值要看的狀態。
    std::vector<std::string> events;
    if (d.beforeApply) {
        SessionBegin(answersJson);   // 事件裡的 golden 訊息／待辦（例 ELPasswordRefused）要進這次 ack
        char* s0 = cJSON_PrintUnformatted(root);
        const std::string all = s0 ? s0 : "{}";
        if (s0) cJSON_free(s0);
        d.beforeApply(all, &events);
        for (std::size_t i = 0; i < events.size(); ++i) cJSON_DeleteItemFromObjectCaseSensitive(root, events[i].c_str());
    }
    // 不可改的丟掉（自己／祖先停用或看不見、ReadOnly、清單筆 bEnable=false）
    std::vector<std::string> ignored;
    const std::vector<HTEditList*> lists = ListsOf(d);
    for (cJSON* it = root->child; it;) {
        cJSON* next = it->next;
        bool drop = !ELEditable(d.form, it->string);
        if (!drop) {
            TControl* c = ELFind(d.form, it->string);
            for (std::size_t k = 0; k < lists.size() && c && !drop; ++k)
                for (int i = 0; lists[k] && i < lists[k]->FEditList->Count; ++i) {
                    THTEdit* e = static_cast<THTEdit*>(lists[k]->FEditList->Items[i]);
                    if (e->SourceControl == c && !e->bEnable) { drop = true; break; }
                }
        }
        if (drop) {
            ignored.push_back(it->string);
            cJSON_DeleteItemFromObjectCaseSensitive(root, it->string);
        }
        it = next;
    }
    char* s = cJSON_PrintUnformatted(root);
    const std::string filtered = s ? s : "{}";
    if (s) cJSON_free(s);
    cJSON_Delete(root);

    std::vector<std::string> applied, unknown;
    if (!ELApplyProxies(d.form, filtered, &applied, &unknown, err)) {
        if (!events.empty()) d.reload();   // 事件已改過替身 → 還原（同 golden 關頁 FormClose 的 ReadFile）
        return 400;
    }
    for (std::size_t i = 0; i < unknown.size(); ++i)
        for (int j = 0; j < d.nSaveReads; ++j)
            if (unknown[i] == d.saveReads[j]) {
                d.reload();
                *err = "refused: save reads " + unknown[i] + " but its value kind cannot be applied";
                return 400;
            }
    for (std::size_t i = 0; i < events.size(); ++i) applied.push_back(events[i]);

    if (!d.beforeApply) SessionBegin(answersJson);   // 有 beforeApply 的頁面在事件之前已開始
    d.saveFlow();
    const bool saved = ELMarked(d.savedMark);
    // golden 沒寫檔的路徑（A02 權限、答 NO…）：替身留著頁面值；golden 關頁 FormClose 會 ReadFile 重讀 → 這裡立刻還原
    if (!saved) d.reload();
    if (ELMarked("closed")) ShownOf()[d.tag].shown = false;   // golden Close()：下次存檔前要重新開頁

    std::vector<std::string> kept;
    for (std::size_t k = 0; k < lists.size(); ++k) {
        if (!lists[k]) continue;
        for (int i = 0; i < lists[k]->FEditList->Count; ++i) {
            THTEdit* it = static_cast<THTEdit*>(lists[k]->FEditList->Items[i]);
            if (std::string(ELFormOf(it->SourceControl)) != d.form) continue;   // 別的表單登記的筆（例 TfConfiguration 的 edP16_1）
            const std::string n = it->ControlName.c_str();
            if (n.empty() || Contains(applied, n) || Contains(ignored, n) || Contains(kept, n)) continue;
            kept.push_back(n);
        }
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("struct").String(d.tag);
    w.Key("saved").Bool(saved);
    w.Key("applied").Number((wb_int64)applied.size());
    w.Key("ignored").BeginArray();
    for (std::size_t i = 0; i < ignored.size(); ++i) w.String(ignored[i]);
    w.EndArray();
    w.Key("kept").BeginArray();
    for (std::size_t i = 0; i < kept.size(); ++i) w.String(kept[i]);
    w.EndArray();
    w.Key("unknown").BeginArray();
    for (std::size_t i = 0; i < unknown.size(); ++i) w.String(unknown[i]);
    w.EndArray();
    if (d.beforeApply) {                              // Steven 團隊 20260925：存檔前重播了哪些 golden 事件（依序）
        w.Key("events").BeginArray();
        for (std::size_t i = 0; i < events.size(); ++i) w.String(events[i]);
        w.EndArray();
    }
    w.Key("session").RawValue(SessionJson());
    w.EndObject();
    *ack = w.Str();
    return 200;
}


// ===========================================================================
//  AI(W906-FRW-S157) 20260927 [W906]：WS form.event 的 C 路（Steven ★ Q40＝A，RULINGS_20260926 S157；
//  格式 FROM_STEVEN 20260927 10:15）。說明見 _EditPage.h 與 FileRW/_FormEvent.h。
// ===========================================================================
namespace {
struct EvTable {
    const PageEvent* t = nullptr;
    int n = 0;
};
std::map<std::string, EvTable>& Events() {
    static std::map<std::string, EvTable> e;
    return e;
}
std::string EvNum(int v) {
    char b[24];
    std::snprintf(b, sizeof(b), "%d", v);   // MinGW 6.3：不用 std::to_string
    return b;
}
TStringList* ItemsOf(TControl* c) {
    if (TComboBox* x = dynamic_cast<TComboBox*>(c)) return x->Items;
    if (TRadioGroup* x = dynamic_cast<TRadioGroup*>(c)) return x->Items;
    return nullptr;
}
// 替身狀態快照：ProxyStateJson（visible／enabled／editable／tabVisible＋值）＋TComboBox／TRadioGroup 的 Items
cJSON* EvSnap(const char* form) {
    cJSON* s = cJSON_Parse(ProxyStateJson(form).c_str());
    if (!s || !cJSON_IsObject(s)) {
        if (s) cJSON_Delete(s);
        return cJSON_CreateObject();
    }
    for (cJSON* it = s->child; it; it = it->next) {
        TStringList* items = it->string ? ItemsOf(ELFind(form, it->string)) : nullptr;
        if (!items) continue;
        cJSON* a = cJSON_CreateArray();
        for (int i = 0; i < items->Count; ++i) cJSON_AddItemToArray(a, cJSON_CreateString(items->Strings[i].c_str()));
        cJSON_AddItemToObject(it, "items", a);
    }
    return s;
}
void EvRaw(webbridge::JsonWriter& w, const char* key, const cJSON* v) {
    char* p = cJSON_PrintUnformatted(v);
    w.Key(key).RawValue(p ? p : "null");
    if (p) cJSON_free(p);
}
// 前後快照有差的替身 → {名稱:{有變的鍵…}}（鍵名同 /api/form 第二型 display 的 widget；沒變的鍵不出現）。
// TComboBox 的 text／itemIndex 任一有變就兩個一起送（VCL 兩者連動；頁面套值時要一起看）。
std::string EvDiff(const char* form, const cJSON* before, const cJSON* after) {
    webbridge::JsonWriter w;
    w.BeginObject();
    for (const cJSON* a = after->child; a; a = a->next) {
        if (!a->string) continue;
        const cJSON* b = cJSON_GetObjectItemCaseSensitive(before, a->string);
        if (b && cJSON_Compare(a, b, 1)) continue;
        const bool combo = dynamic_cast<TComboBox*>(ELFind(form, a->string)) != nullptr;
        bool comboValue = false;
        w.Key(a->string).BeginObject();
        for (const cJSON* k = a->child; k; k = k->next) {
            const cJSON* bk = b ? cJSON_GetObjectItemCaseSensitive(b, k->string) : nullptr;
            const bool diff = !bk || !cJSON_Compare(k, bk, 1);
            if (combo && (!std::strcmp(k->string, "text") || !std::strcmp(k->string, "itemIndex"))) {
                comboValue = comboValue || diff;
                continue;
            }
            if (diff) EvRaw(w, k->string, k);
        }
        if (comboValue) {
            if (const cJSON* k = cJSON_GetObjectItemCaseSensitive(a, "itemIndex")) EvRaw(w, "itemIndex", k);
            if (const cJSON* k = cJSON_GetObjectItemCaseSensitive(a, "text")) EvRaw(w, "text", k);
        }
        w.EndObject();
    }
    w.EndObject();
    return w.Str();
}
// editlist.get 的 "events"（見 PageJson）。這一頁沒接事件回 ""。
std::string EvPageJson(const PageDesc& d) {
    std::map<std::string, EvTable>::const_iterator et = Events().find(d.tag);
    if (et == Events().end() || et->second.n <= 0) return std::string();
    webbridge::JsonWriter w;
    w.BeginObject();
    for (int i = 0; i < et->second.n; ++i) {
        const PageEvent& e = et->second.t[i];
        TControl* c = ELFind(d.form, e.control);
        w.Key(e.control).BeginObject();
        w.Key("event").String(e.event);
        w.Key("golden").String(e.golden);
        w.Key("operable").Bool(c && ELOperable(d.form, e.control));   // false ⇒ form.event 會回 bad-payload（golden 點不到）
        if (TStringList* items = ItemsOf(c)) {
            w.Key("items").BeginArray();
            for (int k = 0; k < items->Count; ++k) w.String(items->Strings[k].c_str());
            w.EndArray();
        }
        w.EndObject();
    }
    w.EndObject();
    return w.Str();
}
}  // namespace

void RegisterPageEvents(const char* tag, const PageEvent* table, int n) {
    EvTable t;
    t.t = table;
    t.n = n;
    Events()[tag] = t;
}

const PageDesc* FindPageForEvent(const std::string& tag) {
    if (const PageDesc* d = FindPage(tag)) return d;
    const PageDesc* first = nullptr;
    for (std::map<std::string, const PageDesc*>::const_iterator it = Reg().begin(); it != Reg().end(); ++it) {
        const std::string p = it->second->page ? it->second->page : "";
        if (p != tag && p != tag + ".html") continue;
        if (Events().count(it->first)) return it->second;   // 同一頁有兩個結構時，挑有事件表的那個
        if (!first) first = it->second;
    }
    return first;
}

bool RunPageEvent(const PageDesc& d, const formevent::Request& r, formevent::Result* out) {
    if (!d.booted()) { out->code = "handler-failed"; out->why = std::string(d.tag) + " edit lists are not booted"; return false; }
    const PageEvent* e = nullptr;
    std::map<std::string, EvTable>::const_iterator et = Events().find(d.tag);
    if (et != Events().end())
        for (int i = 0; i < et->second.n; ++i)
            if (r.control == et->second.t[i].control && r.event == et->second.t[i].event) { e = &et->second.t[i]; break; }
    TControl* c = ELFind(d.form, r.control.c_str());
    if (!e) {
        out->code = c ? "no-handler" : "unknown-control";
        out->why = c ? r.control + " \"" + r.event + "\" has no translated golden handler (" + d.form +
                           " events: tools/editlist/" + d.tag + ".py)"
                     : r.control + " is not a " + d.form + " proxy (golden widget name; editlist.get " + d.tag + " lists them)";
        return false;
    }
    out->golden = e->golden;
    if (!c) { out->code = "handler-failed"; out->why = "internal: event proxy not created: " + r.control; return false; }
    // 1. 開過頁：golden 的事件只發生在開著的表單上；元件的可見／可改是開頁 FormShow 依當下 AccessLevel 算的（同 PageSave）
    const Shown sh = ShownOf()[d.tag];
    if (!sh.shown || sh.level != AccessLevel) {
        out->code = "bad-payload";
        out->why = std::string("reload page: open the page (editlist.get ") + d.tag +
                   " = golden FormShow) with the current access level before sending events";
        return false;
    }
    // 2. golden 使用者點得到嗎：自己或任一層容器停用／看不見 ⇒ 滑鼠到不了，OnChange／OnClick 不會發生
    if (!ELOperable(d.form, r.control.c_str())) {
        out->code = "bad-payload";
        out->why = r.control + " cannot be operated now (it or a container is disabled or hidden after golden FormShow / DFM), "
                   "so golden " + e->golden + " cannot fire -- reload the page";
        return false;
    }
    // 3. 控制項自己的值先驗（驗不過就什麼都不動）
    TComboBox* cb = dynamic_cast<TComboBox*>(c);
    TRadioGroup* rg = dynamic_cast<TRadioGroup*>(c);
    if (r.hasIndex && (cb || rg)) {
        TStringList* items = ItemsOf(c);
        const int n = items ? items->Count : 0;
        if (r.itemIndex < -1 || r.itemIndex >= n) {
            out->code = "bad-payload";
            out->why = "itemIndex " + EvNum(r.itemIndex) + " is outside the golden list of " + r.control + " (" + EvNum(n) +
                       " items) -- reload the page";
            return false;
        }
        // golden 處理器拿 ItemIndex 當資料列（例 cTrayForm.cpp:577→:589-600）：頁面清單舊了、文字對不上 ⇒ 會填錯列，拒絕
        if (cb && r.hasText && r.itemIndex >= 0 && std::string(items->Strings[r.itemIndex].c_str()) != r.text) {
            out->code = "bad-payload";
            out->why = "text \"" + r.text + "\" is not item " + EvNum(r.itemIndex) + " of the golden list (\"" +
                       std::string(items->Strings[r.itemIndex].c_str()) + "\") -- the page's list is stale, reload the page";
            return false;
        }
    }
    // 4. 頁面其他控制項目前的值（選用）：不可改的丟掉（同 PageSave），控制項自己的值由 3 決定
    if (!r.stateJson.empty()) {
        cJSON* root = cJSON_Parse(r.stateJson.c_str());
        if (!root || !cJSON_IsObject(root)) {
            if (root) cJSON_Delete(root);
            out->code = "bad-payload"; out->why = "state is not a JSON object"; return false;
        }
        cJSON_DeleteItemFromObjectCaseSensitive(root, r.control.c_str());
        const std::vector<HTEditList*> lists = ListsOf(d);
        for (cJSON* it = root->child; it;) {
            cJSON* next = it->next;
            bool drop = !ELEditable(d.form, it->string);
            if (!drop) {
                TControl* x = ELFind(d.form, it->string);
                for (std::size_t k = 0; k < lists.size() && x && !drop; ++k)
                    for (int i = 0; lists[k] && i < lists[k]->FEditList->Count; ++i) {
                        THTEdit* he = static_cast<THTEdit*>(lists[k]->FEditList->Items[i]);
                        if (he->SourceControl == x && !he->bEnable) { drop = true; break; }
                    }
            }
            if (drop) cJSON_DeleteItemFromObjectCaseSensitive(root, it->string);
            it = next;
        }
        char* s = cJSON_PrintUnformatted(root);
        const std::string filtered = s ? s : "{}";
        if (s) cJSON_free(s);
        cJSON_Delete(root);
        std::vector<std::string> applied, unknown;
        std::string err;
        if (!ELApplyProxies(d.form, filtered, &applied, &unknown, &err)) {   // 先全部驗、再一次套；失敗時一個都沒動
            out->code = "bad-payload"; out->why = "state: " + err; return false;
        }
    }
    // 5. 控制項自己的值（VCL：選清單第 n 項 ⇒ ItemIndex=n、Text=Items[n]；點勾選框 ⇒ Checked）
    if (cb) {
        if (r.hasIndex) {
            cb->ItemIndex = r.itemIndex;
            if (r.hasText) cb->Text = AnsiString(r.text.c_str());
            else if (r.itemIndex >= 0) cb->Text = cb->Items->Strings[r.itemIndex];
        } else if (r.hasText) {
            ELComboText(cb, AnsiString(r.text.c_str()));
        }
    } else if (rg) {
        if (r.hasIndex) rg->ItemIndex = r.itemIndex;
    } else if (TCheckBox* x = dynamic_cast<TCheckBox*>(c)) {
        if (r.hasChecked) x->Checked = r.checked;
    } else if (TRadioButton* x = dynamic_cast<TRadioButton*>(c)) {
        if (r.hasChecked) x->Checked = r.checked;
    } else if (TCustomEdit* x = dynamic_cast<TCustomEdit*>(c)) {
        if (r.hasText) x->Text = AnsiString(r.text.c_str());
    }
    // 6. golden 處理器（Sender＝這個元件的替身）；changed＝前後快照有差的替身
    SessionBegin("");
    cJSON* before = EvSnap(d.form);
    std::string what;
    try {
        e->handler(c);
    } catch (const std::exception& x) {
        what = x.what();
    } catch (...) {
        what = "non-std exception";
    }
    if (!what.empty()) {
        cJSON_Delete(before);
        // ⚠ 例外之前處理器已改的替身不還原 —— 與 golden 相同（VCL 例外框之後表單停在那個狀態）
        out->code = "handler-failed"; out->why = std::string("golden ") + e->golden + " threw: " + what; return false;
    }
    cJSON* after = EvSnap(d.form);
    out->changedJson = EvDiff(d.form, before, after);
    cJSON_Delete(after);
    cJSON_Delete(before);
    // 7. golden 訊息／待辦（ELMessage／ELTodo）；ELAsk 在事件裡沒有頁面答案（一律 NO），照實記進 todo
    cJSON* sj = cJSON_Parse(SessionJson().c_str());
    if (sj) {
        const cJSON* m = cJSON_GetObjectItemCaseSensitive(sj, "messages");
        cJSON* t = cJSON_GetObjectItemCaseSensitive(sj, "todo");
        const cJSON* a = cJSON_GetObjectItemCaseSensitive(sj, "asked");
        if (t && cJSON_IsArray(t) && a && cJSON_IsArray(a))
            for (const cJSON* q = a->child; q; q = q->next) {
                const cJSON* en = cJSON_GetObjectItemCaseSensitive(q, "en");
                cJSON_AddItemToArray(t, cJSON_CreateString((std::string("golden asked \"") +
                    (en && cJSON_IsString(en) ? en->valuestring : "") + "\" -- form.event has no answers, taken as NO (2)").c_str()));
            }
        char* ms = m ? cJSON_PrintUnformatted(m) : nullptr;
        char* ts = t ? cJSON_PrintUnformatted(t) : nullptr;
        if (ms) { out->messagesJson = ms; cJSON_free(ms); }
        if (ts) { out->todoJson = ts; cJSON_free(ts); }
        cJSON_Delete(sj);
    }
    if (ELMarked("closed")) ShownOf()[d.tag].shown = false;   // golden Close()：下次要重新開頁
    return true;
}

}  // namespace filerw
