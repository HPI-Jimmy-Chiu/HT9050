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
struct Shown { bool shown = false; int level = -1; bool closeRan = false; };   //AI(W906-EVB10C) 20260929 [W906]: closeRan＝這一次開窗 golden FormClose 已經跑過（檔尾 PageFormCloseRan；PageWindowClosed 清）；同一行附加
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
    OpenEnterRecord(d.tag, ShownOf()[d.tag].shown);   //AI(W906-FRW-S165) 20260927 [W906]：golden 開窗鈕在 ShowModal（→ FormShow）之前記的 "Enter ..."（RULINGS_20260926 S165＝R101；表與規則在本檔檔尾）。shown 在下面才設 true ⇒ 這裡讀到的是開頁前的值：存檔後引擎自動重讀（shown 還是 true）不記，golden Close()（"closed"）之後的 editlist.get 再記
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
    //AI(W906-FRW-S158) 20260927 [W906]：頁面目前分頁（TPageControl 的 activePageIndex，見 _EditList.h ELApplyProxies）先套 ——
    //   在 beforeApply 重播的 golden 事件之前：事件看分頁時要看到使用者的分頁（例 TrayForm BeforeApply 重播 rgFixTrayModeClick
    //   → ShowCompnet 看 pgRunMode->ActivePageIndex，golden V912 cTrayAssignment.cpp:1033-1034），同 RunPageEvent 先套 state 再跑處理器。
    //   這幾筆從 root 拿掉（後面的丟值／套值不再看它；點不到的由 ELApplyProxies 的 ELOperable 判，理由進 todo、不進 ignored），
    //   之後回 400 時還原成套值前的分頁（保持「400＝替身沒被頁面值改動」）。頁面沒送這個鍵 ⇒ 這段什麼都不做（行為不變）。
    std::vector<std::string> notes, tabApplied;   // notes：activePageIndex 不收的理由（SessionBegin 之後才記得住，見下）
    std::vector<std::pair<TPageControl*, int> > tabsBefore;
    {
        cJSON* tabs = cJSON_CreateObject();
        for (cJSON* it = root->child; it;) {
            cJSON* next = it->next;
            TPageControl* pc = it->string ? dynamic_cast<TPageControl*>(ELFind(d.form, it->string)) : nullptr;
            if (pc && cJSON_IsObject(it) && cJSON_GetObjectItemCaseSensitive(it, "activePageIndex")) {
                const std::string name = it->string;
                tabsBefore.push_back(std::pair<TPageControl*, int>(pc, pc->ActivePageIndex));
                cJSON_AddItemToObject(tabs, name.c_str(), cJSON_Duplicate(it, 1));
                cJSON_DeleteItemFromObjectCaseSensitive(root, name.c_str());
            }
            it = next;
        }
        char* ts = cJSON_PrintUnformatted(tabs);
        const std::string tabsJson = ts ? ts : "{}";
        if (ts) cJSON_free(ts);
        cJSON_Delete(tabs);
        std::vector<std::string> tabUnknown;
        if (!tabsBefore.empty() && !ELApplyProxies(d.form, tabsJson, &tabApplied, &tabUnknown, err, &notes)) {
            cJSON_Delete(root);   // 不會發生（activePageIndex 不整批拒，見 _EditList.cpp TypeOk）；保險：什麼都沒套
            return 400;
        }
    }
    const auto restoreTabs = [&tabsBefore]() {
        for (std::size_t i = 0; i < tabsBefore.size(); ++i) tabsBefore[i].first->ActivePageIndex = tabsBefore[i].second;
    };
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
    if (!ELApplyProxies(d.form, filtered, &applied, &unknown, err, &notes)) {
        restoreTabs();                     //AI(W906-FRW-S158) 20260927 [W906]
        if (!events.empty()) d.reload();   // 事件已改過替身 → 還原（同 golden 關頁 FormClose 的 ReadFile）
        return 400;
    }
    applied.insert(applied.end(), tabApplied.begin(), tabApplied.end());   //AI(W906-FRW-S158) 20260927 [W906]：先套的分頁也算 applied
    for (std::size_t i = 0; i < unknown.size(); ++i)
        for (int j = 0; j < d.nSaveReads; ++j)
            if (unknown[i] == d.saveReads[j]) {
                restoreTabs();   //AI(W906-FRW-S158) 20260927 [W906]
                d.reload();
                *err = "refused: save reads " + unknown[i] + " but its value kind cannot be applied";
                return 400;
            }
    for (std::size_t i = 0; i < events.size(); ++i) applied.push_back(events[i]);

    if (!d.beforeApply) SessionBegin(answersJson);   // 有 beforeApply 的頁面在事件之前已開始
    for (std::size_t i = 0; i < notes.size(); ++i) ELTodo(notes[i].c_str());   //AI(W906-FRW-S158) 20260927 [W906]：→ ack.session.todo
    d.saveFlow();
    const bool saved = ELMarked(d.savedMark);
    // golden 沒寫檔的路徑（A02 權限、答 NO…）：替身留著頁面值；golden 關頁 FormClose 會 ReadFile 重讀 → 這裡立刻還原
    if (!saved) d.reload();
    if (ELMarked("closed")) { ShownOf()[d.tag].shown = false;  ShownOf()[d.tag].closeRan = true; }   // golden Close()：下次存檔前要重新開頁  //AI(W906-EVB10C) 20260929 [W906]: 大括號＋closeRan（golden Close() → OnClose＝FormClose 已經跑過 ⇒ 這一次開窗的關窗邊緣不再跑，檔尾 PageCloseEdgeRefused）；同一行

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

//AI(W906-EVB1) 20260928 [W906] 第 4 步 state 的事件控制項（B2 頁面工程師 20260928 回報的缺口，見 RunPageEvent 第 4 步註解）
namespace {
// 這一列是不是「這個控制項的值改變時 golden 一定會跑的那一支」：VCL 使用者改勾選框／單選鈕／單選群組／下拉／滑桿·捲軸·UpDown／
// 分頁／點圖（Tag）都經過自己的 OnClick／OnChange ⇒ 是。TCustomEdit 只有 "change" 列才算：它的 "click" 列是「點一下開小鍵盤」，
// 可以打字的輸入框打字不經 OnClick（golden 也一樣，值可以不經處理器改變）；ReadOnly 的輸入框第 4 步原本就丟（ELEditable）。
bool EvValueBound(const char* form, const PageEvent& row)
{
    if (dynamic_cast<TCustomEdit*>(ELFind(form, row.control))) return std::strcmp(row.event, "change") == 0;
    return true;
}
// state 的這一筆跟伺服器端替身目前的值不同嗎（只比值的鍵；替身沒帶 tag＝0；不是物件＝不同）
bool EvStateDiffers(const cJSON* cur, const cJSON* entry)
{
    if (!cJSON_IsObject(entry)) return true;
    static const char* const kVal[] = {"checked", "itemIndex", "text", "position", "activePageIndex", "tag", "dateTime", "cells"};
    const cJSON* p = (cur && entry->string) ? cJSON_GetObjectItemCaseSensitive(cur, entry->string) : nullptr;
    for (const char* k : kVal) {
        const cJSON* v = cJSON_GetObjectItemCaseSensitive(entry, k);
        if (!v || cJSON_IsNull(v)) continue;
        const cJSON* pv = p ? cJSON_GetObjectItemCaseSensitive(p, k) : nullptr;
        if (!pv) {
            if (!(std::strcmp(k, "tag") == 0 && cJSON_IsNumber(v) && v->valuedouble == 0)) return true;
            continue;
        }
        if (!cJSON_Compare(pv, v, 1)) return true;
    }
    return false;
}
}  // namespace

//AI(W906-EVB1) 20260928 [W906] X-2：第 3 步多驗、第 5 步多套「控制項自己的 position（ELTrackBar）／activePageIndex（TPageControl）」
//   （格式 FileRW/_FormEvent.h 檔頭；_EditPage.h 的流程說明不在這次可改的檔，以本檔為準）。沒帶這兩個鍵的請求行為不變（驗值、套值、ack 都同以前）。
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
    //AI(W906-EVB1) 20260928 [W906] X-2：控制項自己的新位置／新分頁（FileRW/_FormEvent.h 檔頭）。只在「這個元件自己的 OnChange」收：
    //   C 路的 TTrackBar／TScrollBar／TUpDown 替身都是 ELTrackBar（TScrollBar 由各結構的 Boot 先建成 ELTrackBar，例 TrayForm.cpp:438）。
    //   別的事件（例 IniConfig udD46 的 btNext／btPrev：處理器自己照 Increment 走一格）帶 position 會走兩次 ⇒ 拒絕。
    //   驗不過一樣什麼都不動（state 在第 4 步才套）。
    ELTrackBar* tb = dynamic_cast<ELTrackBar*>(c);
    TPageControl* pc = dynamic_cast<TPageControl*>(c);
    if (r.hasPosition && (!tb || r.event != "change")) {
        out->code = "bad-payload";
        out->why = !tb ? "position is only for a TTrackBar / TScrollBar / TUpDown proxy; " + r.control + " is not one"
                       : "position is only taken with the control's own OnChange (event \"change\"), not \"" + r.event +
                         "\" -- that handler moves " + r.control + "->Position itself";
        return false;
    }
    if (r.hasPageIndex) {
        std::string why;
        if (!pc) why = "activePageIndex is only for a TPageControl proxy; " + r.control + " is not one";
        else if (r.event != "change") why = "activePageIndex is only taken with the TPageControl's own OnChange (event \"change\"), not \"" + r.event + "\"";
        else if (!(why = ELPageIndexRefused(d.form, r.control.c_str(), r.activePageIndex)).empty())
            why = "activePageIndex " + EvNum(r.activePageIndex) + " of " + r.control + ": " + why + " -- reload the page";
        if (!why.empty()) { out->code = "bad-payload"; out->why = why; return false; }
    }
    // 4. 頁面其他控制項目前的值（選用）：不可改的丟掉（同 PageSave），控制項自己的值由 3 決定
    std::vector<std::string> stateNotes;   //AI(W906-FRW-S158) 20260927 [W906]：state 的 activePageIndex 不收的理由（6 的 SessionBegin 之後補記）
    if (!r.stateJson.empty()) {
        cJSON* root = cJSON_Parse(r.stateJson.c_str());
        if (!root || !cJSON_IsObject(root)) {
            if (root) cJSON_Delete(root);
            out->code = "bad-payload"; out->why = "state is not a JSON object"; return false;
        }
        cJSON_DeleteItemFromObjectCaseSensitive(root, r.control.c_str());
        //AI(W906-EVB1) 20260928 [W906] 事件控制項不收 state（B2 頁面工程師 20260928 回報）：這一頁事件表上有自己一列的控制項
        //   （EvValueBound），state 裡的值一律丟掉、伺服器保留原值 —— 同上一行丟 r.control。golden 裡它們的值只會經過自己的處理器改變
        //   （VCL 使用者點／選／拖 ⇒ OnClick／OnChange），state 照套＝「值變了、處理器沒跑」：例 TrayForm rgFixTrayMode 跳過 Fix 盤有 IC
        //   的互鎖（R95），存檔時 BeforeApply 看到頁面值＝伺服器值就不再重查；Temp_Set rgIndexHeatMode 跳過 R97 拒存（舊表存進新模式的檔）、
        //   TS-7 rb*Point／rgBasePoint 跳過 BasePointReplay。要改它們頁面就送它們自己的 form.event（處理器跑過、互鎖查過）。
        //   選丟不選拒（bad-payload）：state 的定義是「頁面其他控制項目前的值」，通用引擎會整頁送；拒絕會讓有兩個以上事件控制項的頁
        //   每一次事件都失敗。跟伺服器值不同的才記 todo（頁面有 bug 看得到；相同的丟了沒有影響，不記）。
        {
            cJSON* cur = nullptr;   // 替身目前的值（ProxyStateJson），第一次要比時才取
            for (cJSON* it = root->child; it;) {
                cJSON* next = it->next;
                const PageEvent* row = nullptr;
                for (int i = 0; it->string && i < et->second.n && !row; ++i)
                    if (std::strcmp(et->second.t[i].control, it->string) == 0 && EvValueBound(d.form, et->second.t[i]))
                        row = &et->second.t[i];
                if (row) {
                    if (!cur) cur = cJSON_Parse(ProxyStateJson(d.form).c_str());
                    if (EvStateDiffers(cur, it))
                        stateNotes.push_back(std::string("state.") + it->string + " ignored: " + it->string + " has its own golden event (" +
                                             row->golden + ", event \"" + row->event + "\") -- in BCB its value only changes through that "
                                             "handler, so the server keeps its value; send form.event for it first");
                    cJSON_Delete(cJSON_DetachItemViaPointer(root, it));
                }
                it = next;
            }
            if (cur) cJSON_Delete(cur);
        }
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
        if (!ELApplyProxies(d.form, filtered, &applied, &unknown, &err, &stateNotes)) {   // 先全部驗、再一次套；失敗時一個都沒動
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
    //AI(W906-EVB1) 20260928 [W906] X-2：VCL 使用者拖滑桿／捲軸 ⇒ Position 先變（TTrackBar CN_HSCROLL、TScrollBar DoScroll→SetPosition；
    //   夾在 Min..Max、TUpDown 更新 Associate 的字）→ 才 OnChange；點分頁 ⇒ 先換 ActivePage → 才 OnChange（TPageControl.Change）。
    //   SetPosition(v, false)：不從替身的 OnChange 指標再觸發一次 —— OnChange 就是下面第 6 步表上的處理器（例 Speed 的 R119 包裝
    //   FileRW/ArmSpeed_File.cpp R119EvKeep 要包住它，替身的 OnChange 指標直接指 golden，繞過包裝）。處理器一律跑一次（同其他種類）。
    std::string clampNote;   // 被夾過：第 6 步 SessionBegin 之後補記進 todo，changed 補帶實際值
    if (tb && r.hasPosition) {
        tb->SetPosition(r.position, false);
        if ((int)tb->Position != r.position)
            clampNote = "position " + EvNum(r.position) + " of " + r.control + " is outside Min..Max (" + EvNum((int)tb->Min) + ".." +
                        EvNum((int)tb->Max) + "): VCL clamps, golden " + e->golden + " ran with " + EvNum((int)tb->Position) +
                        " (the page's slider range is stale)";
    } else if (pc && r.hasPageIndex) {
        pc->ActivePageIndex = r.activePageIndex;   // 第 3 步已照 ELPageIndexRefused 驗過
    }
    // 6. golden 處理器（Sender＝這個元件的替身）；changed＝前後快照有差的替身
    SessionBegin("");
    for (std::size_t i = 0; i < stateNotes.size(); ++i) ELTodo(stateNotes[i].c_str());   //AI(W906-FRW-S158) 20260927 [W906]：→ ack.todo
    if (!clampNote.empty()) ELTodo(clampNote.c_str());   //AI(W906-EVB1) 20260928 [W906] X-2：→ ack.todo
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
    //AI(W906-EVB1) 20260928 [W906] X-2：被夾過的位置在「處理器之前」就變了（before 快照在第 5 步之後）⇒ EvDiff 看不到；
    //   頁面的滑桿停在它送的值，要補帶伺服器的實際值（處理器自己又改了 Position 時 EvDiff 已帶，不覆蓋）
    if (!clampNote.empty()) {
        cJSON* ch = cJSON_Parse(out->changedJson.c_str());
        if (ch && cJSON_IsObject(ch)) {
            cJSON* self = cJSON_GetObjectItemCaseSensitive(ch, r.control.c_str());
            if (!self) { self = cJSON_CreateObject(); cJSON_AddItemToObject(ch, r.control.c_str(), self); }
            if (cJSON_IsObject(self) && !cJSON_GetObjectItemCaseSensitive(self, "position"))
                cJSON_AddItemToObject(self, "position", cJSON_CreateNumber((int)tb->Position));
            char* p = cJSON_PrintUnformatted(ch);
            if (p) { out->changedJson = p; cJSON_free(p); }
        }
        if (ch) cJSON_Delete(ch);
    }
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
    if (ELMarked("closed")) { ShownOf()[d.tag].shown = false;  ShownOf()[d.tag].closeRan = true; }  out->closed = ELMarked("closed");   // golden Close()：下次要重新開頁  //AI(W906-EVB10A) 20260929 [W906]：大括號只為 -Wmisleading-indentation（行為不變）；ack.closed（golden 處理器 Close() 了 ⇒ 頁面關視窗，例 Setup／Temp_Set 的 Exit 鈕 sbtExitClick）；接在同一行  //AI(W906-EVB10C) 20260929 [W906]: closeRan＝golden FormClose 已經跑過（同 PageSave；檔尾 PageCloseEdgeRefused）；同一行
    return true;
}

}  // namespace filerw


// ===========================================================================
//  AI(W906-FRW-S158) 20260927 [W906]：C 路開頁（WS editlist.get）／存檔（WS editlist.save）重查 golden 的開窗閘。
//  依據：Q41 盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md 第一節第 2 項、第二節 C-1／C-2。
//  分工照 Q42（D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md「### Q42.」）：網頁的開窗
//  權限表（D:\HT9045\.github\specs\page-access-policy.md、D:\HT9045\web\background.html MODAL_POLICY）歸 Jimmy，本段是
//  C++ 端「不信任前端」的重查 —— 做法同 WebBuilder.cpp:255-267 RouteGuard（每次操作都重查選單等級＋頁等級）。
//
//  golden 開一個設定表單要過兩道（V912 main.cpp，cp950）：
//    選單鈕：工具選單 palSetup ＝ :29030-29037 sbSettingClick（SystemStart 時 return；fSecurity->Insufficient(0)）
//            設定選單 palConfig ＝ :29009-29016 sbConfigClick （SystemStart 時 return；fSecurity->Insufficient(1)）
//    頁面鈕：各自的 sbXxxClick（Yield 39、BarCode 88、DIO 31、Start Mode 24、C.Select 25 …，見下面 kOpenGates）。
//  少數頁不在兩個選單裡（主畫面工具列的 Speed／Offset、Motion View 裡的 Teach／Shuttle Maintain／AOA、溫度視窗的
//  Handler System），照它們自己的 golden 入口查。表以 editlist.* 的 tag 查（含 IniConfig／Teach／BinSelect／Offset_File
//  四個不走 PageDesc 的入口），不加進 PageDesc（理由同上面的 PageEvent：加欄位會讓每個結構多一個
//  -Wmissing-field-initializers 警告）。tools/wb_serve.cpp 的兩臂在跑 golden 之前各問一次（拒絕時 golden 一行都不跑，
//  同 golden 按鈕 return 在 ShowModal 之前）。
//
//  SystemStart 時開頁擋不擋（照 golden 判斷）：兩個選單鈕的第一行就是 if(SystemStart) return;，而且運轉中
//  DoMainPadProcess（:3970-3978）把 palSetup／palConfig 兩個選單藏起來 ⇒ 經選單開的頁運轉中都開不了，editlist.get 擋。
//  不經選單的照各自的鈕：Speed 只在 SystemStart && iHome（運轉中又在回原點）時不開（:28680-28681）；Offset（:28708-28722）
//  與 AOA 頁籤不查 SystemStart ⇒ 開頁放行（存檔仍被 wb_serve 的 R0927-7 擋）；Teach／Shuttle Maintain／Handler System
//  自己查 SystemStart ⇒ 擋。SoftStart 開頁不擋：golden 兩個選單鈕只看 SystemStart（存檔的 SystemStart||SoftStart 是
//  RULINGS_20260927 第 2 條第 7 題，不在這裡）。
//
//  ⚠ 偏離 golden（寫明）：
//    1. golden 的 fSecurity->Insufficient(n) 預設 bAlarm=true，不夠時跳 WAR1676（ShowErrorMessage）。這裡一律傳 false：
//       網頁的第一道是 Jimmy 的開窗表，這裡是重查；理由改成回給頁面（同 WebTowerLight.cpp:11 的做法）。另一個原因：
//       單獨開的頁面沒有 background 的對話框宿主，WAR1676 會讓伺服器停在警報等人回答（tools/webprobe/data_builder_probe.py
//       檔頭記過）。要不要照 golden 跳 WAR1676 列給 Steven 決定。
//    2. 同一頁 golden 有兩條以上的開法時，任一條過就放行（C++ 分不出網頁是從哪裡開的）。只列「一般機台都有」的路：
//       只對某客戶看得見的另一顆鈕（Contact 頁的 btBarcode＝CC_KYEC_XILINX、cContact.cpp:1636-1637；CCLink 頁的
//       btShuttlePositionMove＝CC_Greatek、MyCCLinkSensor.cpp:564）沒列 ⇒ 那兩家客戶會比 golden 多擋（往擋得住的那側）。
//       程式自己開表單的地方（RPDefault.cpp:53-130 Show 完就 Close、ProductionInfo.cpp:2242／:2413 MO 下載、uLotInfo.cpp:2721
//       換配方後的高度校正）不是操作員的入口，不列。
//    3. 同一條 if 裡的客戶分支照翻（純比較，不翻就是偏離）：CC_SCS 的 Configuration 30、CC_SPIL_CHINA_SUZHOU 的 Speed、
//       ASE 高雄的 Handler System。要硬體或互動的客戶閘不做：Barcode_Reader 刷條碼（KYEC，例 sbOffsetClick :28710）、
//       sbSetupClick 的 bASEK15UsePW 密碼（:28461-28468）—— 盤點第四節「客戶專屬，Steven 20260925 已決定先跳過」。
//    4. 沒做（不是等級）：sbAutoCleanClick 的 InitialOK（:29661-29664；⛔ 20260927 更正：原寫「wb_serve 沒有把 InitialOK 設成 true、照翻會讓 Cleaning 頁永遠開不了」不對——開機 PumpInit 成功時會設（移植樹 WebBridgeTags.cpp:563，tools/wb_serve.cpp:4212 呼叫；同 decisions-pending R74 的更正），
//       照翻只會在 PumpInit 失敗（例 sim canary 拒絕）或開機完成前擋；仍沒做、行為不變）、sbShuttleMaintainClick 的「機台內還有料」（:33689-33698，要 InArmSuck／OutArmSuck 的
//       HasIC，本檔不碰 TMyKitSuck）、sbTeaching 按鈕本身的 Enabled（:13041-13051、:22840-22856）、Handler System 的
//       隱藏手勢＋確認框＋寫死密碼（cTemperFrom.cpp:1754-1776，屬 C-3 密碼那一條）。
//       ⛔ 20260930 更正（AI(W906-AUTHMAINFORM)）：sbTeaching 的 Enabled 已做 bAnyLevelCanGetStateRecode==false 那一支（:13041-13043
//       =authMainForm[11]，見下段）；還沒做的只剩 ==true 那一支（:13045-13051：只有 AccessLevel==iDefHonPrecLevel 亮，不是開關）。
//       labTestSiteClick 的寫入（:22840-22856）一拍之內就被 Timer2Timer（:21675）呼叫的 ChangeLevelAttr 蓋掉，穩態看 ChangeLevelAttr。
//  AI(W906-AUTHMAINFORM) 20260930 [W906]：主畫面鈕的 Enabled 另要 Security_new.def [Main] 的開關 authMainForm[n]。golden V912
//    TfMain::ChangeLevelAttr（main.cpp:12926-13191）由 Timer2Timer 每一拍呼叫（:21675，不在任何 if 裡）⇒ 它算的就是按鈕的穩態；開關是 0
//    ⇒ 鈕是灰的、sbXxxClick 根本不會跑 ⇒ 回 disabled。開關：golden cAuthority.cpp:19 bool authMainForm[12]、:44-58 鍵名 MainForm[]、
//    :363-367 GetMainAuth 讀 D:\HT9045\config\Security_new.def [Main]（缺鍵預設 1）；移植樹 cAuthority.cpp:119／:144-158／:457-461，
//    開機 tools/wb_serve.cpp:4051 呼叫（golden TfMain::TfMain main.cpp:1738）。
//      工具選單 sbSetting  :12951 Enabled=(AccessLevel>=LevelSet.AccessLevel[0] && authMainForm[0])  [Main] Tool      → GToolsMenu
//      設定選單 sbConfig   :12952 Enabled=(AccessLevel>=LevelSet.AccessLevel[1] && authMainForm[1])  [Main] Maintance → GConfigMenu
//      Teach    sbTeaching :13041-13043 bAnyLevelCanGetStateRecode==false 時 Enabled=authMainForm[11]  [Main] Teaching  → GTeach
//    查的順序：SystemStart（:12937-12940 運轉中兩顆選單鈕都灰）→ 開關 → 等級。開關關著時換更高的等級也開不了，所以先講它。
//    查過、不用改的：sbSpeed（:12954 停機時恆 true）與 sbOffset（ChangeLevelAttr 不碰）不看開關；Speed／Offset 表單自己的
//    authMainForm[3]／[2] 在 FormShow 停用分頁／容器（golden cSpeed.cpp:182-188、cOffSet.cpp:584-590），產生檔
//    ArmSpeed_File.gen.inc:1173-1179、Offset_File.gen.inc:1022-1031 已經照翻。同一條設定選單閘另有三份（WebBuilder.cpp、WebSmartDiag.cpp、
//    WebTowerLight.cpp 的 RouteGuard），同批補 authMainForm[1]。
//  沒登記在 kOpenGates 的 tag 一律拒絕（no-gate）：新增 C 路頁要照 golden 的 sbXxxClick 補一列。
// ===========================================================================
#include "forms/fSecurity.h"   // fSecurity->Insufficient（移植樹 cSecurity.cpp:650；golden V912 cSecurity.cpp:573-597）
#include "cprod.h"             // LevelSet（LAST_LEVEL_SET，system\levelset.dat；開機 W906_SecurityBoot 讀、WebLevelSet.cpp 改）
#include "Config.h"            // IniConfig.bAnyLevelCanGetStateRecode
#include "CosFunction.h"       // CosFunction.bSecurityHave5Level／bUseDynamicKitDiameter／bHiSiliconFunction
#include "MachineType.h"       // CC_*
extern bool bResetMNet;        // Motor/myMN200motor.h:198（定義 Motor/myMN200motor.cpp:340）；只要這一個，不拉馬達標頭
extern int iMaxLevelItem;      // cSecurity.cpp:47（開機 W906_SecurityBoot 設 180；0＝權限表還沒載入）
extern bool authMainForm[12];  // AI(W906-AUTHMAINFORM) 20260930: cAuthority.h:56（本體 cAuthority.cpp:119，開機 GetMainAuth 填）；不 include cAuthority.h（帶進 language.h，與 HTEditList.h 衝突，同 Offset_File.gen.inc:27）

namespace filerw {

// AI(W906-B8-AG1) 20260930 [W906]：AGV 頁（TestIF_File_AGV）的 spbAGV 看不看得見（golden V912 main.cpp:24333 {spbAGV, USE_E84_Sensor}）。
//   FileRW/TestIF_File_AGV.cpp 開機裝上；沒裝（那一頁沒開機）＝看不見。用函式指標是為了本檔不必連 USE_E84_Sensor
//   （test_openenter_log／test_formevent_position／test_evd013_barcode 單獨連本檔、不連 god-stack）。
bool (*W906_AgvGateVisible)() = nullptr;

namespace {
struct GateWhy {
    std::string code;     // not-authorized／running／hidden／not-ready
    std::string detail;   // 給頁面看的中文
    std::string golden;   // golden 出處
};
typedef bool (*GateFn)(GateWhy* w);   // true＝這條路走得通

std::string GNum(int v) {
    char b[24];
    std::snprintf(b, sizeof(b), "%d", v);   // MinGW 6.3：不用 std::to_string
    return b;
}

// 等級名稱：golden TMySecurity::SetParent（V912 cSecurity.cpp:860-902；移植樹 cSecurity.cpp:985-1017）RadioGroup 的項目
std::string GLevelName(int lv) {
    static const char* const k4[] = {"Operator", "Engineer", "Supervisor", "HonPrec"};
    static const char* const k5[] = {"Open", "Operator", "Engineer", "Supervisor", "HonPrec"};
    static const char* const k5Kyec[] = {"Operator", "Engineer", "PEngineer", "Supervisor", "HonPrec"};
    const char* const* t = k4;
    int n = 4;
    if (CosFunction.bSecurityHave5Level) {
        t = CUSTOMER_CODE == CC_KYEC_LEE ? k5Kyec : k5;
        n = 5;
    }
    return lv >= 0 && lv < n ? std::string(t[lv]) + "(" + GNum(lv) + ")" : GNum(lv);
}

// golden fSecurity->Insufficient(item)（回 true＝等級夠）。bAlarm 傳 false：見本段檔頭「偏離 1」。
bool GLv(int item, const char* caption, const char* where, const char* golden, GateWhy* w) {
    if (fSecurity != 0 && fSecurity->Insufficient(item, false)) return true;
    w->code = "not-authorized";
    w->detail = std::string("等級不足：") + where + "需要等級 " + GNum(item) + "（權限項目 " + caption + "；levelset.dat 設定 " +
                (item >= 0 && item < 256 ? GLevelName(LevelSet.AccessLevel[item]) : std::string("?")) + " 以上，目前登入 " +
                GLevelName(AccessLevel) + "）";
    if (fSecurity == 0 || item > iMaxLevelItem)
        w->detail += "——權限表還沒載入（fSecurity／iMaxLevelItem=" + GNum(iMaxLevelItem) + "，開機 W906_SecurityBoot 沒跑）";
    w->golden = golden;
    return false;
}

bool GNotRunning(const char* where, const char* golden, GateWhy* w) {
    if (!SystemStart) return true;
    w->code = "running";
    w->detail = std::string("機台運轉中（SystemStart）不能開") + where;
    w->golden = golden;
    return false;
}

// AI(W906-AUTHMAINFORM) 20260930 [W906]：golden 主畫面鈕的 Enabled 裡的 authMainForm[n]（本段檔頭「AUTHMAINFORM」）。
//   key＝Security_new.def [Main] 的鍵名（golden cAuthority.cpp:44-58 MainForm[n]）。
bool GAuth(int n, const char* key, const char* where, const char* golden, GateWhy* w) {
    if (n >= 0 && n < 12 && authMainForm[n]) return true;
    w->code = "disabled";
    w->detail = std::string("停用：") + where + "的鈕在 golden 主畫面是灰的、按不到（D:\\HT9045\\config\\Security_new.def [Main] " + key +
                "=0 ⇒ authMainForm[" + GNum(n) + "] 關；開機 GetMainAuth 讀，缺鍵預設 1）";
    w->golden = golden;
    return false;
}

// ---- 選單（golden TfMain 的兩顆選單鈕）----
// 工具選單 palSetup：golden V912 main.cpp:29030-29037 sbSettingClick
bool GToolsMenu(GateWhy* w) {
    return GNotRunning("工具選單", "golden V912 main.cpp:29033-29034 sbSettingClick if(SystemStart) return;"
                                   "（運轉中 DoMainPadProcess :3970-3978 也把 palSetup 藏起來）", w) &&
           GAuth(0, "Tool", "工具選單", "golden V912 main.cpp:12951 ChangeLevelAttr sbSetting->Enabled=(AccessLevel>=LevelSet.AccessLevel[0] && "
                                         "authMainForm[0])（Timer2Timer :21675 每拍重算；[Main] Tool，cAuthority.cpp:366）", w) &&   // AI(W906-AUTHMAINFORM) 20260930
           GLv(0, "[00] Main - Tools", "工具選單", "golden V912 main.cpp:29036 sbSettingClick fSecurity->Insufficient(0)", w);
}
// 設定選單 palConfig：golden V912 main.cpp:29009-29016 sbConfigClick
bool GConfigMenu(GateWhy* w) {
    return GNotRunning("設定選單", "golden V912 main.cpp:29012-29013 sbConfigClick if(SystemStart) return;"
                                   "（運轉中 DoMainPadProcess :3970-3978 也把 palConfig 藏起來）", w) &&
           GAuth(1, "Maintance", "設定選單", "golden V912 main.cpp:12952 ChangeLevelAttr sbConfig->Enabled=(AccessLevel>=LevelSet.AccessLevel[1] && "
                                              "authMainForm[1])（Timer2Timer :21675 每拍重算；[Main] Maintance，cAuthority.cpp:366）", w) &&   // AI(W906-AUTHMAINFORM) 20260930
           GLv(1, "[01] Main - Config", "設定選單", "golden V912 main.cpp:29015 sbConfigClick fSecurity->Insufficient(1)", w);
}
// Motion View（main.dfm tsMotionView）：golden 兩個入口 —— 設定選單的 sbMotionView（:29901-29908：設定選單＋Insufficient(86)）、
// 主畫面的 Test Site 標籤 labTestSiteClick（:22834-22864：bAnyLevelCanGetStateRecode==false 時 Insufficient(86)，不查 SystemStart）。
// 第一條過就一定過第二條 ⇒ 兩條合起來＝只看 labTestSite 那條。
bool GMotionView(GateWhy* w) {
    if (IniConfig.bAnyLevelCanGetStateRecode) return true;   // golden :22836
    return GLv(86, "[86] Config - Motion View", "Motion View",
               "golden V912 main.cpp:22836-22838 labTestSiteClick（主畫面 Test Site）／:29904 sbMotionViewClick fSecurity->Insufficient(86)", w);
}

// ---- 各頁（golden 的頁面鈕）----
// 工具選單裡、頁面鈕自己不再查等級的頁
bool GTools(GateWhy* w) { return GToolsMenu(w); }
// golden V912 main.cpp:28500-28506 sbYieldClick
bool GYield(GateWhy* w) {
    return GToolsMenu(w) && GLv(39, "[39] Tools - Yield Monitoring", "Yield Monitoring",
                                "golden V912 main.cpp:28505 sbYieldClick fSecurity->Insufficient(39)", w);
}
// golden V912 main.cpp:29910-29914 sbBarCodeClick
bool GBarCode(GateWhy* w) {
    return GToolsMenu(w) && GLv(88, "[88] Tools - BarCode", "Bar Code",
                                "golden V912 main.cpp:29913 sbBarCodeClick fSecurity->Insufficient(88)", w);
}
// Contact Force：golden 從 Contact 頁的 btContactForce 開（cContact.cpp:15237-15240），Contact 頁從工具選單 sbContact 開
// （main.cpp:28302-28317，不查等級）；鈕看不看得見由 TfContact::FormShow（cContact.cpp:1094）的 :1406-1419 決定
bool GContactForce(GateWhy* w) {
    if (!GToolsMenu(w)) return false;
    if (!(CUSTOMER_CODE == CC_KYEC_LEE && CosFunction.bHiSiliconFunction) &&
        (WEIGHT_CALIBRATION != 0 || CUSTOMER_CODE == CC_HONPREC_QC || CosFunction.bUseDynamicKitDiameter))
        return GLv(152, "[152] Contact - Contact Force Calibration", "Contact Force",
                   "golden V912 cContact.cpp:1414 btContactForce->Visible=fSecurity->Insufficient(152, false)", w);
    w->code = "hidden";
    w->detail = "這台機器 Contact 頁的 Contact Force 鈕看不見（要 WEIGHT_CALIBRATION、CC_HONPREC_QC 或 "
                "CosFunction.bUseDynamicKitDiameter；CC_KYEC_LEE＋海思功能一律不顯示），golden 開不到這一頁";
    w->golden = "golden V912 cContact.cpp:1406-1419（TfContact::FormShow）";
    return false;
}
// AI(W906-B8-AG1) 20260930 [W906]：AMR Setting（B8 AG-1）：golden spbAGV 在工具選單 palSetup（main.dfm），spbAGVClick（main.cpp:35776-35781）
//   自己不查等級、不查 SystemStart；鈕看不看得見＝USE_E84_Sensor（:24333，Gerneral.ini [System] AGVModal）。看不見 golden 開不到這一頁。
bool GAgv(GateWhy* w) {
    if (!GToolsMenu(w)) return false;
    if (W906_AgvGateVisible && W906_AgvGateVisible()) return true;
    w->code = "hidden";
    w->detail = "這台機器工具選單的 AMR（spbAGV）鈕看不見（要 Gerneral.ini [System] AGVModal＝USE_E84_Sensor 不是 0），golden 開不到這一頁";
    w->golden = "golden V912 main.cpp:24333 SBPtr[] {spbAGV, USE_E84_Sensor}";
    return false;
}
// golden V912 main.cpp:28599-28607 sbConfigurationClick（只有 CC_SCS 另查 30）
bool GConfiguration(GateWhy* w) {
    if (!GConfigMenu(w)) return false;
    if (CUSTOMER_CODE == CC_SCS)
        return GLv(30, "[30] Config - Configuration", "Configuration（CC_SCS）",
                   "golden V912 main.cpp:28603-28607 sbConfigurationClick fSecurity->Insufficient(30)", w);
    return true;
}
// golden V912 main.cpp:28660-28664 sbDioSetClick
bool GDio(GateWhy* w) {
    return GConfigMenu(w) && GLv(31, "[31] Config - DIO Setting", "DIO Setting",
                                 "golden V912 main.cpp:28663 sbDioSetClick fSecurity->Insufficient(31)", w);
}
// golden V912 main.cpp:28546-28550 sbStartModeClick
bool GStartMode(GateWhy* w) {
    return GConfigMenu(w) && GLv(24, "[24] Config - Start Mode", "Start Mode",
                                 "golden V912 main.cpp:28549 sbStartModeClick fSecurity->Insufficient(24)", w);
}
// golden V912 main.cpp:28558-28562 sbSeleteClick（C.Select）
bool GCounterSel(GateWhy* w) {
    return GConfigMenu(w) && GLv(25, "[25] Config - C.Select", "Counter Select",
                                 "golden V912 main.cpp:28561 sbSeleteClick fSecurity->Insufficient(25)", w);
}
// AOA Info 頁籤（tsMotionView > pgMotionView > ts1，main.dfm:16447-16448）：golden 切頁籤不跑程式、OffsetSaveClick
// （:34958）不查 ⇒ 只看 Motion View
bool GAoa(GateWhy* w) { return GMotionView(w); }
// golden V912 main.cpp:28826-28838 sbTeachingClick（tsMotionView 的 sbTeaching）
bool GTeach(GateWhy* w) {
    if (!GMotionView(w)) return false;
    if (!IniConfig.bAnyLevelCanGetStateRecode &&   // AI(W906-AUTHMAINFORM) 20260930: sbTeaching 的 Enabled；==true 那一支（:13045-13051 只有 HonPrec）沒做，見檔頭第 4 條
        !GAuth(11, "Teaching", "Teach", "golden V912 main.cpp:13041-13043 ChangeLevelAttr if(IniConfig.bAnyLevelCanGetStateRecode==false) "
                                        "sbTeaching->Enabled=authMainForm[11]（Timer2Timer :21675 每拍重算；[Main] Teaching，cAuthority.cpp:366）", w))
        return false;
    if (!GNotRunning("Teach", "golden V912 main.cpp:28829-28830 sbTeachingClick if(SystemStart) return;", w)) return false;
    if (!GLv(87, "[87] Main - Teaching", "Teach", "golden V912 main.cpp:28832 sbTeachingClick fSecurity->Insufficient(87)", w))
        return false;
    if (bResetMNet) {
        w->code = "not-ready";
        w->detail = "馬達模組正在重置（bResetMNet，24V 還沒開好），golden 不開 Teach";
        w->golden = "golden V912 main.cpp:28835-28838 sbTeachingClick";
        return false;
    }
    return true;
}
// golden V912 main.cpp:33683-33687 sbShuttleMaintainClick（tsMotionView 的 sbShuttleMaintain）；:33689-33698 機台內有料不開＝沒做（見檔頭 4）
bool GShuttleMove(GateWhy* w) {
    return GMotionView(w) &&
           GNotRunning("Shuttle Maintain", "golden V912 main.cpp:33686-33687 sbShuttleMaintainClick if(SystemStart) return;", w);
}
// golden V912 main.cpp:28677-28692 sbSpeedClick（主畫面工具列 tsMain 的 sbSpeed，main.dfm:832；不經選單）
bool GSpeed(GateWhy* w) {
    // AI(W906-D016-SPEED) 20260930 St01 (todo D-016): golden ChangeLevelAttr (V912 main.cpp:12937-12944, every Timer2 tick :21675) sets
    //   sbSpeed->Enabled=false for the whole of SystemStart, so the button can't be pressed while running at all;
    //   sbSpeedClick's own `SystemStart && iHome` return (:28680-28681) is only a second line behind it. The gate was
    //   looser (it refused only SystemStart && iHome); now it follows the button state.
    if (SystemStart) {
        w->code = "running";
        w->detail = std::string("機台運轉中不能開 Speed（golden 運轉中主畫面的 Speed 鈕是灰的") + (iHome ? "；而且正在回原點" : "") + "）";
        w->golden = "golden V912 main.cpp:12937-12944 ChangeLevelAttr（SystemStart ⇒ sbSpeed->Enabled=false）＋:28680-28681 sbSpeedClick";
        return false;
    }
    if (CUSTOMER_CODE == CC_SPIL_CHINA_SUZHOU) {
        if (AccessLevel != 0) return true;
        w->code = "not-authorized";
        w->detail = "等級不足：CC_SPIL_CHINA_SUZHOU 的 Speed 不給 Operator（AccessLevel==0）開（目前登入 " + GLevelName(AccessLevel) + "）";
        w->golden = "golden V912 main.cpp:28683-28687 sbSpeedClick";
        return false;
    }
    return GLv(3, "[03] Main - Speed", "Speed", "golden V912 main.cpp:28690 sbSpeedClick fSecurity->Insufficient(3)", w);
}
// golden V912 main.cpp:28708-28722 sbOffsetClick（主畫面工具列 tsMain 的 sbOffset，main.dfm:716）：不查等級、不查 SystemStart
// （:28710 KYEC_LEE 刷條碼＝客戶專屬，見檔頭 3）
bool GOffset(GateWhy*) { return true; }
// golden V912 cTemperFrom.cpp:1744-1778 TfTemperFrom::Panel71MouseDown（主畫面溫度視窗）→ :1778 HandlerSystem->ShowModal()
bool GHandlerSys(GateWhy* w) {
    if (CUSTOMER_CODE == CC_ASE_KaohSiung || CUSTOMER_CODE == CC_ASE_KaohSiung_K12) {
        w->code = "hidden";
        w->detail = "ASE 高雄（CC_ASE_KaohSiung／K12）golden 一律不開 Handler System";
        w->golden = "golden V912 cTemperFrom.cpp:1747-1749 Panel71MouseDown";
        return false;
    }
    if (!GNotRunning("Handler System", "golden V912 cTemperFrom.cpp:1751-1752 Panel71MouseDown if(SystemStart || ...) return;", w))
        return false;
    if (AccessLevel >= iDefHonPrecLevel) return true;
    w->code = "not-authorized";
    w->detail = "等級不足：Handler System 要 " + GLevelName(iDefHonPrecLevel) + " 以上（AccessLevel<iDefHonPrecLevel；目前登入 " +
                GLevelName(AccessLevel) + "）";
    w->golden = "golden V912 cTemperFrom.cpp:1751-1752 Panel71MouseDown if(... || AccessLevel<iDefHonPrecLevel) return;";
    return false;
}

struct OpenGate {
    const char* tag;    // WS editlist.* 的 tag
    const char* from;   // golden 從哪裡開（文件用；行號 golden V912 main.cpp，除非另寫檔名）
    GateFn main;        // 主路（網頁選單對應的那條；都不通時回它的理由）
    GateFn alt;         // golden 另一條開得到這個表單的路（沒有＝nullptr）；任一條通就放行（檔頭「偏離 2」）
    const char* altName;  // alt 那條路的名稱（拒絕理由用）
};
const OpenGate kOpenGates[] = {
    // ---- 工具選單 palSetup（sbSettingClick：SystemStart 不開、Insufficient(0)；sbSetting->Enabled 另要 authMainForm[0] [Main] Tool，AI(W906-AUTHMAINFORM) 20260930）----
    {"Ld_UldDelayTime",             "工具選單 sbLdUld :28448-28455", GTools, nullptr, nullptr},
    {"UserDefForm_File",            "工具選單 sbTrayForm :28404-28414", GTools, nullptr, nullptr},
    {"TrayForm",                    "工具選單 sbTrayAssign :28428-28438", GTools, nullptr, nullptr},
    {"BinSelect",                   "工具選單 sbBin :28319-28328", GTools, nullptr, nullptr},
    {"TestIF_File_TesterIF",        "工具選單 sbTester :28330-28345", GTools, nullptr, nullptr},
    {"Temperature",                 "工具選單 sbTempOffset :28347-28402（Contact 頁 btTempOffset／btnTempSet cContact.cpp:15254-15258、:17475-17478 也在工具選單下）", GTools, nullptr, nullptr},
    {"DeviceForm_File",             "工具選單 sbContact :28302-28317", GTools, nullptr, nullptr},
    {"ContactForce",                "工具選單 sbContact → Contact 頁 btContactForce cContact.cpp:15237-15240（看得見才開得到，:1406-1419）", GContactForce, nullptr, nullptr},
    {"TestIF_File_SetUp",           "工具選單 sbSetup :28457-28498", GTools, nullptr, nullptr},
    {"TestIF_File_YieldMonitoring", "工具選單 sbYield :28500-28512", GYield, nullptr, nullptr},
    {"TestIF_File_Cleaning",        "工具選單 sbAutoClean :29659-29677", GTools, nullptr, nullptr},
    {"TestIF_File_QAMode",          "工具選單 sbQAMode :29919-29926（Insufficient(85) golden 自己註解掉）", GTools, nullptr, nullptr},
    {"TestIF_File_BarCode",         "工具選單 sbBarCode :29910-29917；Teach 頁 sbBarCode uteach.cpp:4603-4607（不查等級）", GBarCode, GTeach, "Teach 頁的 sbBarCode（golden V912 uteach.cpp:4603-4607）"},
    {"TestIF_File_VacuumUnit",      "工具選單 sbVacuumUnit :35557-35561", GTools, nullptr, nullptr},
    {"GroundMan",                   "工具選單 spbGroundMan :34640-34645", GTools, nullptr, nullptr},
    {"TestIF_File_AGV",             "工具選單 spbAGV :35776-35781（看得見＝USE_E84_Sensor :24333）", GAgv, nullptr, nullptr},   // AI(W906-B8-AG1) 20260930 [W906]
    // ---- 設定選單 palConfig（sbConfigClick：SystemStart 不開、Insufficient(1)；sbConfig->Enabled 另要 authMainForm[1] [Main] Maintance，AI(W906-AUTHMAINFORM) 20260930）----
    {"IniConfig",                   "設定選單 sbConfiguration :28599-28658", GConfiguration, nullptr, nullptr},
    {"TTLCfg",                      "設定選單 sbDioSet :28660-28675", GDio, nullptr, nullptr},
    {"StartCondition",              "設定選單 sbStartMode :28546-28556", GStartMode, nullptr, nullptr},
    {"IniConfig_CounterSel",        "設定選單 sbSelete :28558-28567", GCounterSel, nullptr, nullptr},
    // ---- Motion View（tsMotionView：labTestSite :22834 或設定選單 sbMotionView :29901）----
    {"AOAOffset",                   "Motion View 的 AOA Info 頁籤（main.dfm:16447-16448；OffsetSaveClick :34958）", GAoa, nullptr, nullptr},
    {"Teach",                       "Motion View 的 sbTeaching :28826-28855", GTeach, nullptr, nullptr},
    {"ShuttleMove",                 "Motion View 的 sbShuttleMaintain :33683-33699", GShuttleMove, nullptr, nullptr},
    // ---- 不經選單 ----
    {"ArmSpeed_File",               "主畫面工具列 sbSpeed（main.dfm:832）:28677-28700", GSpeed, nullptr, nullptr},
    {"Offset_File",                 "主畫面工具列 sbOffset（main.dfm:716）:28708-28722", GOffset, nullptr, nullptr},
    {"HSys",                        "主畫面溫度視窗 TfTemperFrom::Panel71MouseDown cTemperFrom.cpp:1744-1778", GHandlerSys, nullptr, nullptr},
};
}  // namespace

bool OpenGateRefused(const std::string& tag, bool save, std::string* why) {
    const OpenGate* g = nullptr;
    for (std::size_t i = 0; i < sizeof(kOpenGates) / sizeof(kOpenGates[0]) && !g; ++i)
        if (tag == kOpenGates[i].tag) g = &kOpenGates[i];
    if (!g) {
        *why = "no-gate: " + tag + " 沒有登記 golden 開窗閘（FileRW/_EditPage.cpp kOpenGates）——C++ 重查預設拒絕；"
               "新增 C 路頁要照 golden 的 sbXxxClick 補一列";
        return true;
    }
    GateWhy w;
    if (g->main(&w)) return false;
    GateWhy w2;
    if (g->alt && g->alt(&w2)) return false;
    *why = w.code + ": " + w.detail + "——" + w.golden;
    if (g->alt) *why += std::string("；另一條路 ") + g->altName + " 也不通：" + w2.detail;
    *why += save ? "（存檔前重查，Q41 C-1／C-2）" : "（開頁重查，Q41 C-1／C-2）";
    return true;
}

}  // namespace filerw



// ===========================================================================
//  AI(W906-FRW-S165) 20260927 [W906]：C 路開頁（WS editlist.get）記 golden 開窗鈕的 "Enter ..." 事件。
//  依據：Steven「要記」＝ D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md 檔尾 S165；
//        D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md「### R101.」。
//
//  golden 每一顆開設定表單的鈕在 ShowModal／Show 之前記一筆（V912 main.cpp，cp950；例 :28306 sbContactClick
//  NewRecordProcess("MES2170", "Enter Contact") → :28314 fContact->Show()、:28323 sbBinClick、:28452 sbLdUldClick、
//  :28694 sbSpeedClick）。這裡照 golden 的呼叫記同一筆：
//    * 代碼與字樣是 golden 呼叫端的字面值（golden NewRecordProcess 的 S 是呼叫端給的，不查訊息表）。
//      20260927 逐列核對：21 列都與 V912 main.cpp 該行相同，也與 D:\HT9045\Error\AlarmCodeList.txt 同代碼的字樣相同。
//    * Debug 用 golden 預設值 " "（V912 cMyDB.h:62）；GroundMan 的 golden 是 RecordProcess("Enter Ground Man Form")
//      （:34643，沒有代碼，S2 預設 ""，cMyDB.h:63）。
//    * golden 的順序是先記、再 ShowModal／Show（→ FormShow）⇒ 呼叫端在跑 golden FormShow 之前呼叫。開窗閘
//      （OpenGateRefused，tools/wb_serve.cpp 在 editlist.get 臂先問）拒絕時 golden 鈕在 NewRecordProcess 之前 return ⇒ 呼叫端
//      根本沒走到這裡、不記；沒開機（409）也不記。
//  寫到哪裡：直接呼叫 golden 的入口（同 WebTowerLight.cpp:238、WebBuilder.cpp:480 的 "Enter ..."）：
//    NewRecordProcess(code, text, " ")／RecordProcess(text, "")。⚠ 移植樹的 NewRecordProcess 目前是空殼
//    （acatchtray_shims.cpp:152；golden 本體 V912 cMyDB.cpp:1545 → MyDBIProcessNew :724 寫 sqlite Process 表、EventLogTxt
//    文字檔、SaveEventLogInfo；移植樹 cMyDB.cpp:1865 #if 0 等 homecoming），RecordProcess 只印 stdout（canary_support.cpp:117）
//    ⇒ 今天不寫任何檔（D:\HT9045_Log 不動）；這裡另外印一行 stdout 讓 wb_serve 主控台看得到。homecoming 之後這裡
//    一個字都不用改，同一筆就會進 golden 的檔。
//    不走 JsonBridge/EventLog.cpp 的 LogAppend（它會多一份記憶體 ring）：EventLog.cpp 只在 wb_serve 的來源清單，
//    tests/ 裡用 god-stack 連本檔的測試（例 test_hsys_heater_mix）會連不到；要不要讓 "Enter" 進 ring 列給 Steven。
//  什麼時候不記（alreadyShown）：伺服器端這一頁已經開著 —— 引擎存檔後一定重讀（web\page\ht9045_wire_engine.js:1290
//    「規則 3：寫完一定重讀」）、頁面的重讀鈕（同檔 :2096）都會再送 editlist.get，那不是 golden 的開窗鈕。
//    PageJson 傳 ShownOf().shown（golden Close()＝ELMarked("closed") 才變回 false ⇒ 之後的 editlist.get＝重新開窗，再記）；
//    三個自己的入口傳各自的開頁旗標（Teach／Offset_File 開過就一直是 true；IniConfig 的 g_formShown 每次存檔都變回 false，
//    因為它的 golden 存檔就是 FormClose（IniConfig.cpp:347）⇒ IniConfig 存檔後的自動重讀會再記一筆，見交件）。
//    BinSelect 經 PageJson（BinSelect.cpp:705），不另外呼叫。
//  ⚠ 偏離／限制（寫明）：
//    1. 同一頁 golden 有兩條開法時照主路（工具選單那顆鈕）記：Temperature 的 Contact 頁 btTempOffset／btnTempSet
//       （cContact.cpp:15254-15258、:17475-17478）與 TestIF_File_BarCode 的 Teach 頁 sbBarCode（uteach.cpp:4603-4607）
//       golden 不記，這裡分不出網頁從哪裡開，一律記（同 kOpenGates「偏離 2」）。
//    2. golden 開窗鈕在 ShowModal 之前還有別的動作（記之前：Teach 的 WAR16100／MyLaneIO.BackUpOutputData :28840-28844、
//       Speed 的 ProceeToolBar :28693；記之後：Contact 的 RTC vision :28308-28312）—— 網頁入口本來就沒跑，這裡不補（R101 只要記）。
//    3. kOpenGates 沒做的 golden 前置條件（本檔 S158 段「沒做 4」、「偏離 3」）：ShuttleMove 機台內有料（:33689-33698）、
//       Cleaning 的 InitialOK（:29661-29664）、Set Up 的 ASE K15 密碼（:28461-28468）—— golden 那時不記也不開；網頁照樣開、
//       這裡照樣記。
//    4. ⚠ editlist.get 不等於操作員按了開窗鈕：web\background.html 的非 lazy 視窗開站就把 iframe 載入（藏著，:906-907），
//       引擎 attach 就 load()（ht9045_wire_engine.js:2108）⇒ 每一頁第一次 editlist.get 發生在 wb_serve 起來後第一次開站；
//       之後操作員開視窗（background.html:746 openWin）只是取消隱藏、送 ui.windows.put（WebWindowRegistry.cpp），不再送
//       editlist.get。所以這裡的「Enter」記在開站那一刻（閘有過的頁各一筆），不是操作員開窗那一刻 —— 要準確得改成看
//       視窗總表的 never／closed → open（交件列給 Steven）。
// ===========================================================================
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);   // cMyDB.h:129（Debug 預設 " "）；本體 acatchtray_shims.cpp:152（空殼）
void RecordProcess(AnsiString S, AnsiString S2);                               // cMyDB.h:130（S2 預設 ""）；本體 canary_support.cpp:117（印 stdout）

namespace filerw {

namespace {
// 與 kOpenGates 平行（同一個 tag；分組照 kOpenGates）。行號 golden V912 main.cpp（cp950）。
const OpenEnter kOpenEnters[] = {
    // ---- 工具選單 palSetup ----
    {"Ld_UldDelayTime",             "TfMain::sbLdUldClick",           "MES2176",  "Enter Load / Unload",    "main.cpp:28452"},
    {"UserDefForm_File",            "TfMain::sbTrayFormClick",        "MES2173",  "Enter Tray Form",        "main.cpp:28408"},
    {"TrayForm",                    "TfMain::sbTrayAssignClick",      "MES2175",  "Enter Tray Assignment",  "main.cpp:28432"},
    {"BinSelect",                   "TfMain::sbBinClick",             "MES2171",  "Enter Bin",              "main.cpp:28323"},
    {"TestIF_File_TesterIF",        "TfMain::sbTesterClick",          "MES2172",  "Enter Tester I/F",       "main.cpp:28335"},
    {"Temperature",                 "TfMain::sbTempOffsetClick",      "MES21109", "Enter Temp. Offset",     "main.cpp:28351"},
    {"DeviceForm_File",             "TfMain::sbContactClick",         "MES2170",  "Enter Contact",          "main.cpp:28306"},
    {"TestIF_File_SetUp",           "TfMain::sbSetupClick",           "MES2177",  "Enter Set Up",           "main.cpp:28469"},
    {"TestIF_File_YieldMonitoring", "TfMain::sbYieldClick",           "MES2178",  "Enter Yield Monitoring", "main.cpp:28508"},
    {"TestIF_File_Cleaning",        "TfMain::sbAutoCleanClick",       "MES2194",  "Enter Auto Clean Form",  "main.cpp:29667"},
    {"TestIF_File_QAMode",          "TfMain::sbQAModeClick",          "MES21102", "Enter QA Mode Form",     "main.cpp:29924"},
    {"TestIF_File_BarCode",         "TfMain::sbBarCodeClick",         "MES21101", "Enter 2D Bar Code Form", "main.cpp:29915"},
    {"GroundMan",                   "TfMain::spbGroundManClick",      "",         "Enter Ground Man Form",  "main.cpp:34643"},   // golden RecordProcess
    {"TestIF_File_AGV",             "TfMain::spbAGVClick",            "MES2198",  "Enter AGV Form",         "main.cpp:35779"},   // AI(W906-B8-AG1) 20260930 [W906]：golden 呼叫端的字面值；⚠ D:\HT9045\Error\AlarmCodeList.txt 的 MES2198 是 "Enter OCR Form"（golden 重用了代碼，照 golden 記 "Enter AGV Form"）
    // ---- 設定選單 palConfig ----
    {"IniConfig",                   "TfMain::sbConfigurationClick",   "MES2185",  "Enter Configuration",    "main.cpp:28616"},
    {"TTLCfg",                      "TfMain::sbDioSetClick",          "MES2186",  "Enter DIO Form",         "main.cpp:28668"},
    {"StartCondition",              "TfMain::sbStartModeClick",       "MES2180",  "Enter Start Condition",  "main.cpp:28552"},
    {"IniConfig_CounterSel",        "TfMain::sbSeleteClick",          "MES2181",  "Enter Counter Select",   "main.cpp:28564"},
    // ---- Motion View ----
    {"Teach",                       "TfMain::sbTeachingClick",        "MES2189",  "Enter Teach Form",       "main.cpp:28845"},
    {"ShuttleMove",                 "TfMain::sbShuttleMaintainClick", "MES21107", "Enter Shuttle Maintain", "main.cpp:33692"},
    // ---- 不經選單（主畫面工具列）----
    {"ArmSpeed_File",               "TfMain::sbSpeedClick",           "MES2187",  "Enter Speed",            "main.cpp:28694"},
    {"Offset_File",                 "TfMain::sbOffsetClick",          "MES2188",  "Enter Offset",           "main.cpp:28717"},
};
// kOpenGates 裡 golden 開窗鈕不記、所以沒有列的頁（20260927 讀 golden V912 核對）：
//   ContactForce            TfContact::btContactForceClick（cContact.cpp:15237-15240）只有 fContactForce->Show()
//   TestIF_File_VacuumUnit  TfMain::sbVacuumUnitClick（main.cpp:35557-35561）只有 Down=false、fVacuumUnit->Show()
//   AOAOffset               Motion View 的 AOA Info 頁籤（切頁籤不跑程式）；進 Motion View 的 sbMotionViewClick（:29901-29908）
//                           與 labTestSiteClick（:22834-22864）也不記
//   HSys                    TfTemperFrom::Panel71MouseDown（cTemperFrom.cpp:1744-1778）→ HandlerSystem->ShowModal()，不記
}  // namespace

const OpenEnter* FindOpenEnter(const std::string& tag) {
    for (std::size_t i = 0; i < sizeof(kOpenEnters) / sizeof(kOpenEnters[0]); ++i)
        if (tag == kOpenEnters[i].tag) return &kOpenEnters[i];
    return nullptr;
}

bool EnterWindowGate(const std::string& tag, bool alreadyShown);   // AI(W906-PAGETAB-Q51) 20260928 [W906]：本檔檔尾（步驟 5）
bool OpenEnterRecord(const std::string& tag, bool alreadyShown) {
    if (EnterWindowGate(tag, alreadyShown)) return false;   // 存檔後自動重讀／重讀鈕：golden 沒有再按一次開窗鈕  //AI(W906-PAGETAB-Q51) 20260928 [W906] 裝了頁面表的關窗邊緣（wb_serve）⇒ 改成「每一次開窗記一次」（R108／R110，本檔檔尾）；沒裝＝原本的 alreadyShown
    const OpenEnter* e = FindOpenEnter(tag);
    if (!e) return false;                           // golden 開窗鈕不記
    if (!*e->code) {                                // golden RecordProcess(S)：S2 預設 ""（V912 cMyDB.h:63）
        ::RecordProcess(e->text, "");               // 替身自己會印 stdout（canary_support.cpp:117）
        return true;
    }
    ::NewRecordProcess(e->code, e->text, " ");      // golden NewRecordProcess(AlarmCode, S)：Debug 預設 " "（V912 cMyDB.h:62）
    std::printf("editlist.get %s: golden %s %s NewRecordProcess(\"%s\", \"%s\")\n",   // 移植樹的入口是空殼（不印、不寫）
                tag.c_str(), e->button, e->golden, e->code, e->text);
    return true;
}

}  // namespace filerw


// ===========================================================================
//  AI(W906-PAGETAB-Q51) 20260928 [W906] 步驟 5：C 路頁的「開窗／關窗」跟著網頁視窗走（Steven 20260928 S168；Q49＝B＋D，由我們做）。
//  設計：D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md §3.5、§4.3 H4。
//
//  白話：golden 每按一次開窗鈕 ⇒ 查開窗閘、記 "Enter ..."、FormShow；按 Exit／✕ ⇒ FormClose。網頁版以前
//    (a) 開站就把每一頁的 iframe 載好，引擎一 attach 就 editlist.get ⇒ Enter 記在開站那一刻（R108），操作員開窗時什麼都不跑；
//    (b) 關窗不經過 C++ ⇒ 這裡的「開過了」（ShownOf().shown）一直是 true，下次開窗不算「重新開頁」；
//    (c) golden 存檔＝FormClose 的頁（例 Configuration，IniConfig.cpp:347）存完引擎自動重讀 ⇒ 又記一筆 Enter（R110）。
//  現在：
//    * 網頁引擎（D:\HT9045\web\page\ht9045_wire_engine.js，var H4 那一段）只在視窗真的打開時才 editlist.get ⇒ (a) 沒了；
//    * 頁面表（WebPageTable.cpp PageTableTick (a)）看到某個網頁列「開→關」⇒ W906_EditPageWindowClosed(golden 物件名) ⇒
//      這個表單的每一個 C 路 tag 的 shown＝false（下次存檔／form.event 前一定要重新開頁，golden：關掉的表單不能存、沒有事件）
//      並把「這一次開窗已經記過 Enter」清掉 ⇒ (b) 沒了；
//    * 裝了邊緣（wb_serve 開機 W906_EditPageWindowEdgesArm，tools/wb_serve.cpp:4389）之後，Enter 改成「每一次開窗記一次」：
//      同一次開窗裡的 editlist.get（存檔後自動重讀、重讀鈕、golden Close() 之後的重讀）都不再記 ⇒ (c) 沒了。
//      沒裝（ctest、wb_publish）⇒ 照 S165 的舊規則（alreadyShown），tests/test_openenter_log.cpp [4]～[8] 不變。
//  golden 物件名 → 類別：物件名前面加 "T"（fContact→TfContact、FTestIF→TFTestIF、HandlerSystem→THandlerSystem），比 PageDesc.form。
//  不經 PageDesc 開頁的三個自己的入口（Teach.cpp:256、Offset_File.cpp:353、IniConfig.cpp:494 傳各自的開頁旗標）只清 Enter 那一半
//  （它們自己的開頁旗標與存檔規則不動；form.event 用的別名頁 Config.Configuration／Setup.OffSet 是 PageDesc，照上面清 shown，
//  重新開頁時各自的入口會再呼叫 PageJson(kEvPage) 設回來：IniConfig.cpp:680、Offset_File.cpp:641）。
//  ⚠ 限制：開→關→開 在同一拍（500 ms）裡完成的，頁面表看不到那一次關 ⇒ 不算重新開頁（不記 Enter、不清 shown）。
// ===========================================================================
#include <set>

namespace filerw {

namespace {
bool g_enterFollowsWindow = false;                     // wb_serve 裝了頁面表的關窗邊緣
std::set<std::string>& EnteredThisOpen() {             // 這一次開窗已經記過 Enter 的 tag（關窗邊緣清掉）
    static std::set<std::string> s;
    return s;
}
// 不經 PageDesc 開頁的自己的入口：tag → golden 物件名
const char* const kOwnEntryForms[][2] = {
    {"Teach",       "fTeach"},            // FileRW/Teach.cpp:256
    {"Offset_File", "fOffSet"},           // FileRW/Offset_File.cpp:353
    {"IniConfig",   "fConfiguration"},  {"BinSelect",   "fBinSel"},    // FileRW/IniConfig.cpp:494  //AI(W906-EVB10C) 20260929 [W906]: BinSelect＝FileRW/BinSelect.cpp:705，同一行附加；它走 PageJson(kPage) 但沒登記（BinSelect.cpp:651「不用 PageRegistrar」）⇒ 上面 Reg() 迴圈看不到它：以前「Enter Bin」只記開站後第一次、shown 關窗也不清。下面迴圈連 shown／closeRan 一起清（Teach／Offset_File／IniConfig 沒有這個 tag 的 ShownOf，不受影響）
};
}  // namespace

// true ＝ 這一次不記（OpenEnterRecord 開頭呼叫）。
bool EnterWindowGate(const std::string& tag, bool alreadyShown) {
    if (!g_enterFollowsWindow) return alreadyShown;      // 沒裝邊緣：S165 的舊規則
    return !EnteredThisOpen().insert(tag).second;       // 這一次開窗已經記過 ⇒ 不記
}

void PageWindowEdgesArm() { g_enterFollowsWindow = true; }
bool PageWindowEdgesArmed() { return g_enterFollowsWindow; }

int PageWindowClosed(const char* goldenObj) {
    if (!goldenObj || !*goldenObj) return 0;
    const std::string cls = std::string("T") + goldenObj;
    std::string tags;
    int n = 0;
    for (std::map<std::string, const PageDesc*>::const_iterator it = Reg().begin(); it != Reg().end(); ++it) {
        if (!it->second || !it->second->form || cls != it->second->form) continue;
        ShownOf()[it->first].shown = false;  ShownOf()[it->first].closeRan = false;   //AI(W906-EVB10C) 20260929 [W906]: 下一次開窗重新算「FormClose 跑過了沒」；同一行附加
        EnteredThisOpen().erase(it->first);
        tags += (n++ ? ", " : "") + it->first;
    }
    for (std::size_t i = 0; i < sizeof(kOwnEntryForms) / sizeof(kOwnEntryForms[0]); ++i) {
        if (std::strcmp(kOwnEntryForms[i][1], goldenObj) != 0) continue;
        EnteredThisOpen().erase(kOwnEntryForms[i][0]);  { std::map<std::string, Shown>::iterator s_ = ShownOf().find(kOwnEntryForms[i][0]); if (s_ != ShownOf().end()) { s_->second.shown = false; s_->second.closeRan = false; } }   //AI(W906-EVB10C) 20260929 [W906]: 有 ShownOf 的（只有 BinSelect，見上表）一起清；同一行附加
        tags += (n++ ? ", " : "") + std::string(kOwnEntryForms[i][0]) + "(Enter)";
    }
    if (n) {
        std::printf("[PAGETAB] %s window closed -> C route %s: next editlist.get is a fresh open (golden FormShow, open gate, Enter)\n",
                    goldenObj, tags.c_str());
        std::fflush(stdout);
    }
    return n;
}

}  // namespace filerw

// 全域入口（tools/wb_serve.cpp:4389 用同一行的 block-scope extern 呼叫，不必 include 本檔的標頭）
void W906_EditPageWindowEdgesArm() { filerw::PageWindowEdgesArm(); }
void W906_EditPageWindowClosed(const char* goldenForm) { filerw::PageWindowClosed(goldenForm); }

// AI(W906-EVB10A) 20260929 [W906]：事件批次 B10 part a —— 關窗邊緣的 golden FormClose 問「這一頁這一次開過、FormClose 還沒跑」
//   （宣告與說明 _EditPage.h 檔尾；呼叫端 FileRW/TestIF_File_SetUp.cpp、TestIF_File_YieldMonitoring.cpp、DeviceForm_File.cpp 檔尾）。
//   tools/wb_serve.cpp:4389 的邊緣掛勾先跑 W906_EvB10A_WindowEdge、再跑 W906_EditPageWindowClosed ⇒ 問的時候 shown 還沒被清。
namespace filerw {
bool PageShownNow(const std::string& tag) {
    std::map<std::string, Shown>::const_iterator it = ShownOf().find(tag);
    return it != ShownOf().end() && it->second.shown;
}
}  // namespace filerw

// AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c —— 「這一次開窗 golden FormClose 已經跑過」（宣告與說明 _EditPage.h 檔尾）。
//   記：PageSave／RunPageEvent 看到 "closed"（本檔 :211／:620 同一行）、或存檔本身就是 FormClose 的頁自己呼叫 PageFormCloseRan
//   （FileRW/IniConfig_CounterSel.cpp 檔尾）。清：PageWindowClosed（本檔 :1090、:1096 同一行；tools/wb_serve.cpp:4389 的邊緣掛勾在
//   W906_EvB10A_WindowEdge 之後呼叫 ⇒ 關窗邊緣問的時候還沒被清）。
namespace filerw {
void PageFormCloseRan(const std::string& tag) { ShownOf()[tag].closeRan = true; }

bool PageFormCloseRanNow(const std::string& tag) {
    std::map<std::string, Shown>::const_iterator it = ShownOf().find(tag);
    return it != ShownOf().end() && it->second.closeRan;
}

const char* PageCloseEdgeRefused(const std::string& tag) {
    if (PageFormCloseRanNow(tag))
        return "not run: golden FormClose already ran in this window-open (golden Close() in the save path / Exit button) -- not run twice";
    if (!PageShownNow(tag))
        return "not run: golden FormShow did not run in this window-open (open gate refused, or the page never read) -- golden: the form never opened";
    return nullptr;
}
}  // namespace filerw
