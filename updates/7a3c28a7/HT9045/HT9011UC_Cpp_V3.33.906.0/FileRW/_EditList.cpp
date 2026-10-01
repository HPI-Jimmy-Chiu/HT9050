// ===========================================================================
//  FileRW/_EditList.cpp -- C 類共用層本體。說明見 _EditList.h。
//
//  Steven 20260924.  NOT in golden.
// ===========================================================================
#include "FileRW/_EditList.h"

#include <cstdio>   //AI(W906-FRW-S158) 20260927 [W906]：snprintf（MinGW 6.3 沒有 std::to_string）
#include <map>
#include <typeinfo>
#include <utility>
#include <vector>

#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"

namespace filerw {

namespace {
struct Proxies {
    std::map<std::string, std::string> parentOf;     // "表單類別.名稱" → 父元件名稱
    std::map<std::string, bool> readOnly;            // "表單類別.名稱"（golden DFM ReadOnly=True）
    std::map<std::string, TControl*> byKey;          // "表單類別.名稱"
    std::map<TControl*, std::string> formOf;
    std::vector<std::pair<std::string, std::string> > order;   // (表單類別, 名稱)
    std::map<std::string, std::vector<std::string> > pageOrder;   //AI(W906-FRW-S158) 20260927 [W906]："表單類別.分頁控制" → 分頁（ELSetPageOrder）
};
std::string Key(const char* form, const char* name) { return std::string(form) + "." + name; }
Proxies& P() {
    static Proxies p;
    return p;
}

// THTEdit 的值從它的替身讀（golden SaveEditTextToFile 讀的也是這幾個屬性）
void PutValue(webbridge::JsonWriter& w, TControl* c) {
    if (!c) return;
    if (TCheckBox* x = dynamic_cast<TCheckBox*>(c))            { w.Key("checked").Bool(x->Checked); return; }
    if (TComboBox* x = dynamic_cast<TComboBox*>(c))            { w.Key("itemIndex").Number((wb_int64)x->ItemIndex);
                                                                w.Key("text").String(x->Text.c_str()); return; }
    if (TRadioGroup* x = dynamic_cast<TRadioGroup*>(c))        { w.Key("itemIndex").Number((wb_int64)x->ItemIndex); return; }
    if (TDateTimePicker* x = dynamic_cast<TDateTimePicker*>(c)) { w.Key("dateTime").Number(x->DateTime); return; }
    if (TCustomEdit* x = dynamic_cast<TCustomEdit*>(c))        { w.Key("text").String(x->Text.c_str()); return; }
}
// 替身的值（golden FormShow／InitialDataToEdit 填的），欄位名與 ELApplyProxies 收的一致
void PutProxyValue(webbridge::JsonWriter& w, TControl* c) {
    if (TRadioButton* x = dynamic_cast<TRadioButton*>(c)) { w.Key("checked").Bool(x->Checked); return; }
    if (ELTrackBar* x = dynamic_cast<ELTrackBar*>(c))     { w.Key("position").Number((wb_int64)x->Position); return; }
    if (ELStringGrid* x = dynamic_cast<ELStringGrid*>(c)) {
        w.Key("cells").BeginArray();
        for (int ci = 0; ci < (int)x->grid.ColCount; ++ci) {
            w.BeginArray();
            for (int ri = 0; ri < (int)x->grid.RowCount; ++ri) w.String(x->Cells[ci][ri].c_str());
            w.EndArray();
        }
        w.EndArray();
        return;
    }
    if (TLabel* x = dynamic_cast<TLabel*>(c)) { if (!x->Caption.IsEmpty()) w.Key("caption").String(x->Caption.c_str()); return; }
    //AI(W906-FRW-S158) 20260927 [W906]：TPageControl 目前分頁（ELApplyProxies 收同一個鍵；見 _EditList.h）；不 return ——
    //   Tag 照舊由最後一行決定（非 0 才帶）
    if (TPageControl* x = dynamic_cast<TPageControl*>(c)) w.Key("activePageIndex").Number((wb_int64)x->ActivePageIndex);
    if (dynamic_cast<TCheckBox*>(c) || dynamic_cast<TComboBox*>(c) || dynamic_cast<TRadioGroup*>(c) ||
        dynamic_cast<TDateTimePicker*>(c) || dynamic_cast<TCustomEdit*>(c)) { PutValue(w, c); return; }
    // golden 型別 vclcompat 沒有、退回 TControl 的（例 TImage imgI37_3：golden 拿 Tag 當值 0..7）→ 一律帶 tag（0 也要）；
    // 面板／分頁等有自己型別的只在非 0 時帶
    if (typeid(*c) == typeid(TControl) || c->Tag) w.Key("tag").Number((wb_int64)c->Tag);
}
}  // namespace

TControl* ELFind(const char* form, const char* name) {
    std::map<std::string, TControl*>::const_iterator it = P().byKey.find(Key(form, name));
    return it == P().byKey.end() ? nullptr : it->second;
}

void ELKeep(const char* form, const char* name, TControl* c) {
    // VCL 的設計期預設是 Visible=true、Enabled=true、TTabSheet::TabVisible=true（DFM 只記「與預設不同」的值，
    // 由 IC_DfmState 套上）。vclcompat 的建構子三者都是 false —— 替身不改的話，權限判斷會把每個元件都當成
    // 看不見／停用（實測：原值存檔 ignored 913 個，連 edA01 都改不到）。
    c->Visible = true;
    c->Enabled = true;
    if (TTabSheet* t = dynamic_cast<TTabSheet*>(c)) t->TabVisible = true;
    P().byKey[Key(form, name)] = c;
    P().formOf[c] = form;
    P().order.push_back(std::make_pair(std::string(form), std::string(name)));
    HTEditList_RegisterControlName(c, AnsiString(name));
}

const char* ELFormOf(TControl* c) {
    std::map<TControl*, std::string>::const_iterator it = P().formOf.find(c);
    return it == P().formOf.end() ? "" : it->second.c_str();
}

std::string ProxyStateJson(const char* form) {
    webbridge::JsonWriter w;
    w.BeginObject();
    for (std::size_t i = 0; i < P().order.size(); ++i) {
        if (P().order[i].first != form) continue;
        TControl* c = P().byKey[Key(form, P().order[i].second.c_str())];
        w.Key(P().order[i].second).BeginObject();
        w.Key("visible").Bool(c->Visible);
        w.Key("enabled").Bool(c->Enabled);
        // 審查 H1（第 6 輪）：頁面要照「自己＋所有上層」決定能不能改，和存檔丟值的判斷（ELEditable）同一個 ——
        // 只送自己的 Enabled 時，停用容器底下 Enabled=true 的子元件會被頁面重新打開
        w.Key("editable").Bool(ELEditable(form, P().order[i].second.c_str()));
        if (TTabSheet* t = dynamic_cast<TTabSheet*>(c)) w.Key("tabVisible").Bool(t->TabVisible);
        PutProxyValue(w, c);
        w.EndObject();
    }
    w.EndObject();
    return w.Str();
}

std::string EditListToJson(HTEditList* el, const char* onlyForm) {
    webbridge::JsonWriter w;
    w.BeginObject();
    if (!el) {
        w.Key("available").Bool(false);
        w.EndObject();
        return w.Str();
    }
    w.Key("available").Bool(true);
    w.Key("count").Number((wb_int64)el->FEditList->Count);
    w.Key("entries").BeginArray();
    for (int i = 0; i < el->FEditList->Count; ++i) {
        THTEdit* it = static_cast<THTEdit*>(el->FEditList->Items[i]);
        if (onlyForm && std::string(ELFormOf(it->SourceControl)) != onlyForm) continue;
        w.BeginObject();
        w.Key("id").String(it->ControlName.c_str());
        w.Key("form").String(ELFormOf(it->SourceControl));
        w.Key("group").String(it->IniGroupName.c_str());
        w.Key("key").String(it->IniKeyName.c_str());
        w.Key("content").Number((wb_int64)it->Content);
        w.Key("readFromFile").Bool(it->bReadFromFile);
        w.Key("visible").Bool(it->bVisible);
        w.Key("enabled").Bool(it->bEnable);
        if (it->iTransformType) w.Key("transform").Number((wb_int64)it->iTransformType);
        if (!it->MinValue.IsEmpty()) w.Key("min").String(it->MinValue.c_str());
        if (!it->MaxValue.IsEmpty()) w.Key("max").String(it->MaxValue.c_str());
        PutValue(w, it->SourceControl);
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    return w.Str();
}

namespace {
struct Session {
    std::map<std::string, int> answers;
    std::vector<std::pair<std::string, std::string> > messages;
    std::vector<std::pair<std::pair<std::string, std::string>, int> > asked;
    std::vector<std::string> todo;
    std::vector<std::string> trace;
};
Session& S() {
    static Session s;
    return s;
}
}  // namespace

void SessionBegin(const std::string& answersJson) {
    S() = Session();
    cJSON* a = answersJson.empty() ? nullptr : cJSON_Parse(answersJson.c_str());
    if (a && cJSON_IsObject(a))
        for (const cJSON* it = a->child; it; it = it->next)
            if (cJSON_IsNumber(it)) S().answers[it->string] = it->valueint;
    if (a) cJSON_Delete(a);
}

std::string SessionJson() {
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("messages").BeginArray();
    for (std::size_t i = 0; i < S().messages.size(); ++i)
        w.BeginObject().Key("en").String(S().messages[i].first).Key("zh").String(S().messages[i].second).EndObject();
    w.EndArray();
    w.Key("asked").BeginArray();
    for (std::size_t i = 0; i < S().asked.size(); ++i)
        w.BeginObject().Key("en").String(S().asked[i].first.first).Key("zh").String(S().asked[i].first.second)
         .Key("answer").Number((wb_int64)S().asked[i].second).EndObject();
    w.EndArray();
    w.Key("todo").BeginArray();
    for (std::size_t i = 0; i < S().todo.size(); ++i) w.String(S().todo[i]);
    w.EndArray();
    w.Key("trace").BeginArray();
    for (std::size_t i = 0; i < S().trace.size(); ++i) w.String(S().trace[i]);
    w.EndArray();
    w.EndObject();
    return w.Str();
}

void ELMessage(AnsiString S1, AnsiString S2, AnsiString, bool, bool) {
    S().messages.push_back(std::make_pair(std::string(S1.c_str()), std::string(S2.c_str())));
}

int ELAsk(AnsiString S1, AnsiString S2, AnsiString) {
    std::map<std::string, int>::const_iterator it = S().answers.find(S1.c_str());
    int ans = it == S().answers.end() ? 2 : it->second;      // golden：1＝YES、2＝NO
    S().asked.push_back(std::make_pair(std::make_pair(std::string(S1.c_str()), std::string(S2.c_str())), ans));
    return ans;
}

void ELTodo(const char* what) { S().todo.push_back(what); }

void ELMark(const char* what) { S().trace.push_back(what); }

void ELChangeCompomentEnabled(TControl* c, bool bEnable, bool bMustEnable) {
    if (!c) return;
    // golden：bMustEnable==false 時只會關、不會開；==true 時照 bEnable
    if (bMustEnable) c->Enabled = bEnable;
    else if (!bEnable) c->Enabled = false;
}

void ELSetParents(const char* form, const char* const (*pairs)[2], int n) {
    for (int i = 0; i < n; ++i) P().parentOf[Key(form, pairs[i][0])] = pairs[i][1];
}

void ELSetReadOnly(const char* form, const char* name) { P().readOnly[Key(form, name)] = true; }

//AI(W906-FRW-S158) 20260927 [W906]：見 _EditList.h
void ELSetPageOrder(const char* form, const char* pageControl, const char* const* tabs, int n) {
    std::vector<std::string>& v = P().pageOrder[Key(form, pageControl)];
    v.clear();
    for (int i = 0; i < n; ++i) v.push_back(tabs[i]);
}

void ELEnableNames(const char* form, const char* const* names, int n) {
    for (int i = 0; i < n; ++i)
        if (TControl* c = ELFind(form, names[i])) c->Enabled = true;
}

bool ELEditable(const char* form, const char* name) {
    if (P().readOnly.count(Key(form, name))) return false;
    return ELOperable(form, name);   // AI(W906-FRW-S157) 20260927 [W906]：原本的祖先鏈迴圈搬到 ELOperable（行為不變）
}

bool ELOperable(const char* form, const char* name) {
    std::string cur = name;
    for (int depth = 0; depth < 64 && !cur.empty(); ++depth) {
        TControl* c = ELFind(form, cur.c_str());
        if (c && (!c->Enabled || !c->Visible)) return false;
        if (TTabSheet* t = dynamic_cast<TTabSheet*>(c)) if (!t->TabVisible) return false;
        std::map<std::string, std::string>::const_iterator it = P().parentOf.find(Key(form, cur.c_str()));
        cur = it == P().parentOf.end() ? std::string() : it->second;
    }
    return true;
}

bool ELMarked(const char* what) {
    for (std::size_t i = 0; i < S().trace.size(); ++i)
        if (S().trace[i] == what) return true;
    return false;
}

bool ELInAnyList(TControl* c, HTEditList* const* lists, int n) {
    for (int k = 0; k < n; ++k) {
        HTEditList* el = lists[k];
        if (!el) continue;
        for (int i = 0; i < el->FEditList->Count; ++i)
            if (static_cast<THTEdit*>(el->FEditList->Items[i])->SourceControl == c) return true;
    }
    return false;
}

namespace {
// 一個替身要哪個欄位、型別對不對；套用時照同一個判斷寫進去
enum class PK { None, Checked, Index, Text, Position, DateTime, Cells, Tag, PageIndex };   //AI(W906-FRW-S158) 20260927 [W906]：PageIndex
PK KindOf(TControl* c) {
    if (dynamic_cast<TCheckBox*>(c) || dynamic_cast<TRadioButton*>(c)) return PK::Checked;   // 審查 H1：TRadioButton 不是 TCheckBox 衍生
    if (dynamic_cast<TComboBox*>(c) || dynamic_cast<TRadioGroup*>(c)) return PK::Index;
    if (dynamic_cast<ELTrackBar*>(c)) return PK::Position;
    if (dynamic_cast<TDateTimePicker*>(c)) return PK::DateTime;
    if (dynamic_cast<ELStringGrid*>(c)) return PK::Cells;
    if (dynamic_cast<TCustomEdit*>(c)) return PK::Text;
    return PK::None;
}
const char* FieldOf(PK k) {
    switch (k) {
        case PK::Checked: return "checked";
        case PK::Index: return "itemIndex";
        case PK::Text: return "text";
        case PK::Position: return "position";
        case PK::DateTime: return "dateTime";
        case PK::Cells: return "cells";
        case PK::Tag: return "tag";
        case PK::PageIndex: return "activePageIndex";   //AI(W906-FRW-S158) 20260927 [W906]
        default: return "";
    }
}
bool TypeOk(PK k, const cJSON* v, TControl* c) {
    switch (k) {
        case PK::Checked: return cJSON_IsBool(v);
        case PK::Index: case PK::Position: case PK::DateTime: case PK::Tag: return cJSON_IsNumber(v);
        case PK::Text: return cJSON_IsString(v);
        case PK::Cells: {
            // 審查 M2：vclcompat Cells 越界會丟例外（套到一半）→ 檢查時就先比欄列數
            if (!cJSON_IsArray(v)) return false;
            ELStringGrid* g = static_cast<ELStringGrid*>(c);
            if (cJSON_GetArraySize(v) > (int)g->grid.ColCount) return false;
            for (const cJSON* col = v->child; col; col = col->next) {
                if (!cJSON_IsArray(col) || cJSON_GetArraySize(col) > (int)g->grid.RowCount) return false;
                for (const cJSON* x = col->child; x; x = x->next) if (!cJSON_IsString(x)) return false;
            }
            return true;
        }
        case PK::PageIndex: return true;   //AI(W906-FRW-S158) 20260927 [W906]：型別與範圍都由 PageIndexRefused 判（不合法只丟這一筆）
        default: return false;
    }
}

//AI(W906-FRW-S158) 20260927 [W906]：TPageControl 的分頁數（見 _EditList.h ELSetPageOrder）。*exact＝是登記的頁序（否則是下限）。
int PageCountOf(const char* form, const char* pc, bool* exact) {
    std::map<std::string, std::vector<std::string> >::const_iterator o = P().pageOrder.find(Key(form, pc));
    if (o != P().pageOrder.end()) {
        *exact = true;
        return (int)o->second.size();
    }
    *exact = false;
    const std::string pre = std::string(form) + ".";   // parentOf 的鍵是 "表單類別.名稱"、值是不帶表單的父元件名稱
    int n = 0;
    for (std::map<std::string, std::string>::const_iterator it = P().parentOf.begin(); it != P().parentOf.end(); ++it)
        if (it->second == pc && it->first.compare(0, pre.size(), pre) == 0 &&
            dynamic_cast<TTabSheet*>(ELFind(form, it->first.c_str() + pre.size())))
            ++n;
    return n;
}
std::string PNum(double v) {
    char b[40];
    std::snprintf(b, sizeof(b), "%.15g", v);
    return b;
}
//AI(W906-EVB1) 20260928 [W906] X-2：理由那一段拆成 PageIndexWhy，給 form.event 共用（ELPageIndexRefused，本檔下方）；
//   PageIndexRefused 的輸出一字不變（tests/test_editlist_pageindex.cpp 比對的字串）
std::string PageIndexWhy(const char* form, const char* name, const cJSON* v) {
    bool exact = false;
    const int n = PageCountOf(form, name, &exact);
    const double d = cJSON_IsNumber(v) ? v->valuedouble : 0;
    std::string why;
    if (!cJSON_IsNumber(v)) why = "not a number";
    else if (d != std::floor(d)) why = "not a whole number";
    else if (n <= 0) why = "the server knows no tab sheet of it (no ELSetPageOrder, none in the golden DFM parent table)";
    else if (d < 0 || d >= n)
        why = "outside 0.." + PNum(n - 1) + (exact ? " (golden DFM tab order)"
                                               : " (tab sheets in the golden DFM parent table -- a lower bound, no ELSetPageOrder)");
    else if (!ELOperable(form, name)) why = "it or a container is disabled or hidden, so the golden user cannot switch its tabs";
    return why;
}
// 頁面送的 activePageIndex 不收的理由（給 todo）；收得下回 ""
std::string PageIndexRefused(const char* form, const char* name, const cJSON* v) {
    const TPageControl* pc = static_cast<const TPageControl*>(ELFind(form, name));
    const double d = cJSON_IsNumber(v) ? v->valuedouble : 0;
    const std::string why = PageIndexWhy(form, name, v);   //AI(W906-EVB1) 20260928 [W906] X-2：原本的判斷搬到 PageIndexWhy（行為不變）
    if (why.empty()) return why;
    return std::string(form) + "." + name + " activePageIndex " + (cJSON_IsNumber(v) ? PNum(d) : std::string("(non-number)")) +
           " ignored: " + why + " -- the server keeps tab " + PNum(pc->ActivePageIndex);
}
}  // namespace

//AI(W906-EVB1) 20260928 [W906] X-2：見 _EditList.h
std::string ELPageIndexRefused(const char* form, const char* name, int index) {
    if (!dynamic_cast<TPageControl*>(ELFind(form, name))) return std::string(form) + "." + name + " is not a TPageControl proxy";
    cJSON* v = cJSON_CreateNumber(index);
    const std::string why = PageIndexWhy(form, name, v);
    cJSON_Delete(v);
    return why;
}

bool ELApplyProxies(const char* form, const std::string& widgetsJson, std::vector<std::string>* applied,
                    std::vector<std::string>* unknown, std::string* err,
                    std::vector<std::string>* notes) {   //AI(W906-FRW-S158) 20260927 [W906]：notes
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        *err = "widgets is not a JSON object";
        return false;
    }
    std::vector<std::pair<TControl*, const cJSON*> > todo;
    std::vector<PK> kinds;
    std::vector<std::string> bad;
    std::vector<std::string> skipped;   //AI(W906-FRW-S158) 20260927 [W906]：activePageIndex 不收的理由
    for (const cJSON* it = root->child; it; it = it->next) {
        TControl* c = ELFind(form, it->string);
        if (!c) { unknown->push_back(it->string); continue; }
        PK k = KindOf(c);
        //AI(W906-FRW-S158) 20260927 [W906]：TPageControl 帶了 activePageIndex → 分頁種類；沒帶＝照舊（下一行的 tag，否則 unknown）
        if (k == PK::None && cJSON_IsObject(it) && dynamic_cast<TPageControl*>(c) &&
            cJSON_GetObjectItemCaseSensitive(it, "activePageIndex"))
            k = PK::PageIndex;
        // 審查 H1：golden 讀 ->Tag 當值的元件（例 imgI37_3，TImage 點一下 Tag 0..7 循環）→ 頁面送 "tag"
        if (k == PK::None && cJSON_IsObject(it) && cJSON_GetObjectItemCaseSensitive(it, "tag")) k = PK::Tag;
        const cJSON* v = cJSON_IsObject(it) ? cJSON_GetObjectItemCaseSensitive(it, FieldOf(k)) : nullptr;
        if (k == PK::None) { unknown->push_back(it->string); continue; }   // 面板／標籤：值不歸頁面管
        if (!v || !TypeOk(k, v, c)) { bad.push_back(std::string(it->string) + "." + FieldOf(k)); continue; }
        if (k == PK::PageIndex) {   //AI(W906-FRW-S158) 20260927 [W906]：不合法 → 只丟這一筆（見 _EditList.h）
            const std::string why = PageIndexRefused(form, it->string, v);
            if (!why.empty()) { skipped.push_back(why); continue; }
        }
        kinds.push_back(k);
        todo.push_back(std::make_pair(c, it));
    }
    if (!bad.empty()) {
        *err = "refused: wrong or missing value type for:";
        for (std::size_t i = 0; i < bad.size() && i < 40; ++i) *err += (i ? ", " : " ") + bad[i];
        cJSON_Delete(root);
        return false;
    }
    for (std::size_t i = 0; i < skipped.size(); ++i) {   //AI(W906-FRW-S158) 20260927 [W906]：整批會套才記（整批拒時什麼都沒發生）
        if (notes) notes->push_back(skipped[i]);
        else ELTodo(skipped[i].c_str());
    }
    for (std::size_t n = 0; n < todo.size(); ++n) {
        TControl* c = todo[n].first;
        const cJSON* o = todo[n].second;
        const PK k = kinds[n];
        const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, FieldOf(k));
        switch (k) {
            case PK::Checked:
                if (TCheckBox* x = dynamic_cast<TCheckBox*>(c)) x->Checked = cJSON_IsTrue(v) != 0;
                else static_cast<TRadioButton*>(c)->Checked = cJSON_IsTrue(v) != 0;
                break;
            case PK::Tag: c->Tag = v->valueint; break;
            case PK::PageIndex: static_cast<TPageControl*>(c)->ActivePageIndex = (int)v->valuedouble; break;   //AI(W906-FRW-S158) 20260927 [W906]：已驗過範圍
            case PK::Index:
                if (TComboBox* b = dynamic_cast<TComboBox*>(c)) {
                    b->ItemIndex = v->valueint;
                    const cJSON* t = cJSON_GetObjectItemCaseSensitive(o, "text");
                    if (t && cJSON_IsString(t)) b->Text = AnsiString(t->valuestring);
                } else {
                    static_cast<TRadioGroup*>(c)->ItemIndex = v->valueint;
                }
                break;
            case PK::Text: dynamic_cast<TCustomEdit*>(c)->Text = AnsiString(v->valuestring); break;
            case PK::Position: static_cast<ELTrackBar*>(c)->SetPosition(v->valueint, false); break;   // 頁面最後狀態：不觸發 OnChange
            case PK::DateTime: dynamic_cast<TDateTimePicker*>(c)->DateTime = v->valuedouble; break;
            case PK::Cells: {
                ELStringGrid* g = static_cast<ELStringGrid*>(c);
                int ci = 0;
                for (const cJSON* col = v->child; col; col = col->next, ++ci) {
                    int ri = 0;
                    for (const cJSON* x = col->child; x; x = x->next, ++ri) g->Cells[ci][ri] = AnsiString(x->valuestring);
                }
                break;
            }
            default: break;
        }
        applied->push_back(o->string);
    }
    cJSON_Delete(root);
    return true;
}

bool ELPasswordRefused(const char* what) {
    S().todo.push_back(std::string(what) + ": golden 密碼框在網頁端還沒有對應，視同密碼錯誤（回 false）");
    // 審查 M4：這會讓 golden 把勾選改回 false 再存（例 I37_1 FIFO），要讓操作者看得到
    S().messages.push_back(std::make_pair(
        std::string("Password dialog is not available on the web page; treated as wrong password (") + what + ")",
        std::string("網頁端還沒有密碼框，視同密碼錯誤：需要密碼的設定不會被打開（") + what + "）"));
    return false;
}

//AI(W906-EVB10B) 20260929 [W906] X-3：VCL 程式設值觸發 OnClick（見 _EditList.h）。登記表以替身指標為鍵（替身一輩子不換）。
namespace {
std::map<TControl*, ELClickFn>& OnClickTable() {
    static std::map<TControl*, ELClickFn> t;
    return t;
}
long g_clickCount = 0;
void FireOnClick(TControl* c) {
    ELClickFn fn = ELOnClickOf(c);
    if (!fn) return;
    ++g_clickCount;
    fn(c);   // VCL：Click → FOnClick(Self)；Sender＝這個元件
}
}  // namespace

void ELSetOnClick(TControl* c, ELClickFn fn) {
    if (!c) return;
    if (fn) OnClickTable()[c] = fn;
    else OnClickTable().erase(c);
}

ELClickFn ELOnClickOf(TControl* c) {
    std::map<TControl*, ELClickFn>::const_iterator it = OnClickTable().find(c);
    return it == OnClickTable().end() ? nullptr : it->second;
}

void ELClickIndex(TRadioGroup* g, int v) {
    if (!g) return;
    const int n = g->Items ? g->Items->Count : 0;
    if (v < -1) v = -1;                              // VCL TCustomRadioGroup.SetItemIndex（非讀 DFM 時）
    if (v >= n) v = n - 1;
    if (g->ItemIndex == v) return;
    g->ItemIndex = v;
    if (v >= 0) FireOnClick(g);                      // 新的那顆 TGroupButton.Checked:=True → Click → ButtonClick → OnClick
}

void ELClickChecked(TCheckBox* c, bool v) {
    if (!c || c->Checked == v) return;               // VCL TCustomCheckBox.SetState：值有變才 Click
    c->Checked = v;
    FireOnClick(c);
}

void ELClickChecked(TRadioButton* c, bool v) {
    if (!c || c->Checked == v) return;               // VCL TRadioButton.SetChecked：值有變；設成 true 才 Click
    c->Checked = v;
    if (v) FireOnClick(c);
}

long ELClickCount() { return g_clickCount; }

void ELHTEditSetChecked(TCheckBox* c, bool v) {
    if (c && ELOnClickOf(c)) ELClickChecked(c, v);
    else if (c) c->Checked = v;                      // 沒登記：跟以前一樣純設值
}

void ELHTEditSetItemIndex(TRadioGroup* g, int v) {
    if (g && ELOnClickOf(g)) ELClickIndex(g, v);
    else if (g) g->ItemIndex = v;                    // 沒登記：跟以前一樣純設值、不夾
}

//AI(W906-EVB10B) 20260929 [W906] X-5／CC-L1：見 _EditList.h
TControl* ELActivePage(const char* form, const char* pageControl) {
    TPageControl* pc = dynamic_cast<TPageControl*>(ELFind(form, pageControl));
    std::map<std::string, std::vector<std::string> >::const_iterator o = P().pageOrder.find(Key(form, pageControl));
    if (!pc || o == P().pageOrder.end()) return nullptr;
    const int i = pc->ActivePageIndex;
    if (i < 0 || i >= (int)o->second.size()) return nullptr;
    return ELFind(form, o->second[i].c_str());
}

int ELPageIndexOf(const char* form, const char* pageControl, const char* tab) {
    std::map<std::string, std::vector<std::string> >::const_iterator o = P().pageOrder.find(Key(form, pageControl));
    if (o == P().pageOrder.end() || !tab) return -1;
    for (std::size_t i = 0; i < o->second.size(); ++i)
        if (o->second[i] == tab) return (int)i;
    return -1;
}

int ELTabIndexOf(const char* form, const char* pageControl, const char* tab) {
    const int k = ELPageIndexOf(form, pageControl, tab);
    if (k < 0) return -1;
    const std::vector<std::string>& pages = P().pageOrder[Key(form, pageControl)];
    TTabSheet* self = dynamic_cast<TTabSheet*>(ELFind(form, tab));
    if (!self || !self->TabVisible) return -1;       // VCL TTabSheet.GetTabIndex：自己的頁籤沒顯示 → -1
    int n = 0;
    for (int i = 0; i < k; ++i) {
        TTabSheet* t = dynamic_cast<TTabSheet*>(ELFind(form, pages[i].c_str()));
        if (!t || t->TabVisible) ++n;                // 沒有替身的頁＝轉出的 golden 程式從沒碰過它 ⇒ 算 VCL 預設 TabVisible=True（呼叫端要確認 DFM 沒有 TabVisible=False）
    }
    return n;
}

}  // namespace filerw
namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   struct W906_ELFormLock { W906_ELFormLock() { ht9045::formjson::FormLock(); } ~W906_ELFormLock() { ht9045::formjson::FormUnlock(); } };   //AI(W906-S09-Q3) 20260930 (St02-E): St01 R1 (FROM_STEVEN §4 20260930 23:40) -- the FormJson lock (JsonBridge/FormJson.cpp, recursive) around ELFind + the widget access below: GET /api/editlist reads the same widgets under it (tools/wb_serve.cpp:2845-2849) while these are called from the TCP pump / GPIB / SECS / HandlerBridgeCtl; ctests that compile this file without JsonBridge give a test-local FormLock
// cprod.cpp 用的轉接：golden SaveLastSetIni 讀 fConfiguration->cbN07_EnableHostStart->Checked
bool FileRW_ProxyChecked(const char* form, const char* name)
{
    W906_ELFormLock lk;   TCheckBox* c = dynamic_cast<TCheckBox*>(filerw::ELFind(form, name));   //AI(W906-S09-Q3) 20260930 (St02-E): St01 R1 (:554)
    return c ? c->Checked : false;
}

//AI(W906-S09-Q3) 20260930 (St02-E, claim; ST01-E OK needed): golden 寫 fConfiguration->X 的等價寫法（筆電 Q3＝A：不建 TfConfiguration 全域）。
//   寫的是 Configuration 頁用的這一份替身：Command.cpp（SetTesterID、HTSET 314／315／316／804）、SECSGEM/uHGemHT9045.cpp（SPIL 的 Enable RCMD START）。
//   找不到替身就什麼都不做、回 false（跟上面的 FileRW_ProxyChecked 同義）；非 wb_serve 程式的後備在 FileRW/_fallback.cpp。
bool FileRW_ProxySetChecked(const char* form, const char* name, bool v)
{
    W906_ELFormLock lk;   TCheckBox* c = dynamic_cast<TCheckBox*>(filerw::ELFind(form, name));   // St01 R1 (:554)
    if (c) filerw::ELClickChecked(c, v);   // VCL：程式設 Checked 也觸發 OnClick（有登記才觸發；這幾個在 golden DFM 都沒有 OnClick）
    return c != 0;
}
bool FileRW_ProxySetText(const char* form, const char* name, const char* text)
{
    W906_ELFormLock lk;   TEdit* e = dynamic_cast<TEdit*>(filerw::ELFind(form, name));   // St01 R1 (:554)
    if (e) e->Text = text;
    return e != 0;
}
bool FileRW_ProxySetItemIndex(const char* form, const char* name, int n)
{
    W906_ELFormLock lk;   TComboBox* c = dynamic_cast<TComboBox*>(filerw::ELFind(form, name));   // St01 R1 (:554)
    if (c) filerw::ELComboIndex(c, n);     // VCL TComboBox::ItemIndex：Text 跟著換；超出範圍 → -1、空字串（不觸發 OnChange）
    return c != 0;
}
// AI(W906-COOLFAN) 20261001 (St02-E, claim): a TPageControl proxy's ActivePageIndex for code outside FileRW -- csystem.cpp
//   DoSwCoolingFan GATE H1-08 (golden csystem.cpp:20443 PageControl1->ActivePageIndex).  -1 = no such proxy.  The page's tab
//   change keeps it current (form.event: FileRW/_EditPage.cpp:564; editlist.save: :422).  Non-wb_serve programs: FileRW/_fallback.cpp.
int FileRW_ProxyPageIndex(const char* form, const char* name)
{
    W906_ELFormLock lk;   TPageControl* pc = dynamic_cast<TPageControl*>(filerw::ELFind(form, name));
    return pc ? pc->ActivePageIndex : -1;
}
