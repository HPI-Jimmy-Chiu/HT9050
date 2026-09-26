// ===========================================================================
//  FileRW/_EditPage.cpp -- C 路共用頁面層（見 _EditPage.h）。規則照 FileRW/IniConfig.cpp（第一個 C 路結構）。
//  Steven 20260924.  NOT in golden.
// ===========================================================================
#include "FileRW/_EditPage.h"

#include <algorithm>
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
    if (!ELApplyProxies(d.form, filtered, &applied, &unknown, err)) return 400;
    for (std::size_t i = 0; i < unknown.size(); ++i)
        for (int j = 0; j < d.nSaveReads; ++j)
            if (unknown[i] == d.saveReads[j]) {
                d.reload();
                *err = "refused: save reads " + unknown[i] + " but its value kind cannot be applied";
                return 400;
            }

    SessionBegin(answersJson);
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
    w.Key("session").RawValue(SessionJson());
    w.EndObject();
    *ack = w.Str();
    return 200;
}

}  // namespace filerw
