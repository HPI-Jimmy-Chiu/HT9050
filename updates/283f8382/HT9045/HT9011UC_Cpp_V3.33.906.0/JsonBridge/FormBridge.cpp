// ===========================================================================
//  JsonBridge/FormBridge.cpp -- FormState 本體。說明見 FormBridge.h。
//
//  Steven 20260924.  NOT in golden.
// ===========================================================================
#include "JsonBridge/FormBridge.h"

#include <cstdio>
#include <exception>

#include "FileRW/_FormEvent.h"      // AI(W906-FRW-S157) 20260927 [W906]：formevent::Request／Result（RunEvent）
#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"

namespace ht9045 {
namespace formbridge {

extern const BridgeDesc* const kBridges[];
extern const std::size_t kBridgeCount;

WidgetState& FormState::W(const char* id) {
    std::map<std::string, WidgetState>::iterator it = w_.find(id);
    if (it != w_.end()) return it->second;
    order_.push_back(id);
    return w_[id];
}

const WidgetState* FormState::Find(const char* id) const {
    std::map<std::string, WidgetState>::const_iterator it = w_.find(id);
    return it == w_.end() ? nullptr : &it->second;
}

// AI(W906-FRW-S157) 20260927 [W906]：每個賦值另記 touched 位元（form.event 的 changed，見 FormBridge.h ClearTouched）
void FormState::SetText(const char* id, const AnsiString& v)    { WidgetState& s = W(id); s.hasText = true;       s.text = v;            s.touched |= kTouchText; }
void FormState::SetItemIndex(const char* id, int v)            { WidgetState& s = W(id); s.hasIndex = true;      s.itemIndex = v;       s.touched |= kTouchIndex; }
void FormState::SetChecked(const char* id, bool v)             { WidgetState& s = W(id); s.hasChecked = true;    s.checked = v;         s.touched |= kTouchChecked; }
void FormState::SetCaption(const char* id, const AnsiString& v) { WidgetState& s = W(id); s.hasCaption = true;    s.caption = v;         s.touched |= kTouchCaption; }
void FormState::SetVisible(const char* id, bool v)             { WidgetState& s = W(id); s.hasVisible = true;    s.visible = v;         s.touched |= kTouchVisible; }
void FormState::SetEnabled(const char* id, bool v)             { WidgetState& s = W(id); s.hasEnabled = true;    s.enabled = v;         s.touched |= kTouchEnabled; }
void FormState::SetTabVisible(const char* id, bool v)          { WidgetState& s = W(id); s.hasTabVisible = true; s.tabVisible = v;      s.touched |= kTouchTabVisible; }
void FormState::SetActivePageIndex(const char* id, int v)      { WidgetState& s = W(id); s.hasActivePage = true; s.activePageIndex = v; s.touched |= kTouchActivePage; }
void FormState::SetDown(const char* id, bool v)                { WidgetState& s = W(id); s.hasDown = true;       s.down = v;            s.touched |= kTouchDown; }
void FormState::SetColor(const char* id, long c)               { WidgetState& s = W(id); s.hasColor = true;      s.color = c;           s.touched |= kTouchColor; }            // AI(W906-TEACH-FORMSHOW) 20261002
void FormState::SetActivePage(const char* id, const char* sh)  { WidgetState& s = W(id); s.hasActivePageName = true; s.activePageName = sh ? sh : ""; s.touched |= kTouchActivePageName; }   // AI(W906-TEACH-FORMSHOW) 20261002
// AI(W906-TEACH-FORMSHOW) 20261002: VCL TColor 0x00BBGGRR -> "#rrggbb"; a system colour (clBtnFace… high byte set) -> "sys:0x…" (the page keeps its own)
static std::string ColorJs(long c) { char b[24]; unsigned long u = (unsigned long)c; if (u & 0xFF000000ul) std::snprintf(b, sizeof(b), "sys:0x%08lX", u); else std::snprintf(b, sizeof(b), "#%02lx%02lx%02lx", u & 0xFFul, (u >> 8) & 0xFFul, (u >> 16) & 0xFFul); return b; }

void FormState::ItemsClear(const char* id) {
    // VCL TComboBox::Clear 清掉 Items 與 Text，ItemIndex 回 -1。
    WidgetState& s = W(id);
    s.hasItems = true; s.items.clear();
    s.hasText = true;  s.text = AnsiString("");
    s.hasIndex = true; s.itemIndex = -1;
    s.touched |= kTouchItems | kTouchText | kTouchIndex;   // AI(W906-FRW-S157) 20260927 [W906]
}
void FormState::ItemsAdd(const char* id, const AnsiString& v) {
    WidgetState& s = W(id); s.hasItems = true; s.items.push_back(v); s.touched |= kTouchItems;   // AI(W906-FRW-S157) 20260927 [W906]
}
int FormState::ItemCount(const char* id) const {
    const WidgetState* s = Find(id);
    return s ? (int)s->items.size() : 0;
}
AnsiString FormState::Item(const char* id, int i) const {
    const WidgetState* s = Find(id);
    if (!s || i < 0 || i >= (int)s->items.size()) return AnsiString("");   // VCL 會丟 EStringListError
    return s->items[i];
}

void FormState::Miss(const char* id, const char* prop) const {
    std::string k = std::string(id) + "." + prop;
    for (std::size_t i = 0; i < missing_.size(); ++i) if (missing_[i] == k) return;
    missing_.push_back(k);
}

AnsiString FormState::GetText(const char* id) const {
    const WidgetState* s = Find(id);
    if (!s) { Miss(id, "text"); return AnsiString(""); }
    if (s->hasText && !(s->text.IsEmpty() && s->hasItems && s->itemIndex >= 0)) return s->text;
    if (s->hasItems && s->itemIndex >= 0 && s->itemIndex < (int)s->items.size()) return s->items[s->itemIndex];
    if (!s->hasText) Miss(id, "text");
    return s->text;
}
int FormState::GetItemIndex(const char* id) const {
    const WidgetState* s = Find(id);
    if (!s || !s->hasIndex) { Miss(id, "itemIndex"); return s ? s->itemIndex : -1; }
    return s->itemIndex;
}
bool FormState::GetChecked(const char* id) const {
    const WidgetState* s = Find(id);
    if (!s || !s->hasChecked) { Miss(id, "checked"); return s ? s->checked : false; }
    return s->checked;
}
AnsiString FormState::GetCaption(const char* id) const { const WidgetState* s = Find(id); return s ? s->caption : AnsiString(""); }
bool FormState::GetVisible(const char* id) const   { const WidgetState* s = Find(id); return s ? s->visible : true; }
bool FormState::GetEnabled(const char* id) const   { const WidgetState* s = Find(id); return s ? s->enabled : true; }
bool FormState::GetTabVisible(const char* id) const { const WidgetState* s = Find(id); return s ? s->tabVisible : true; }
bool FormState::GetDown(const char* id) const      { const WidgetState* s = Find(id); return s ? s->down : false; }

void FormState::Message(const AnsiString& en, const AnsiString& zh) { messages_.push_back(std::make_pair(en, zh)); }
void FormState::Todo(const std::string& what) { todos_.push_back(what); }

std::string FormState::WidgetsJson() const {
    webbridge::JsonWriter w;
    w.BeginObject();
    for (std::size_t k = 0; k < order_.size(); ++k) {
        const WidgetState& s = w_.find(order_[k])->second;
        w.Key(order_[k]).BeginObject();
        if (s.hasText)       w.Key("text").String(s.text.c_str());
        if (s.hasIndex)      w.Key("itemIndex").Number((wb_int64)s.itemIndex);
        if (s.hasChecked)    w.Key("checked").Bool(s.checked);
        if (s.hasCaption)    w.Key("caption").String(s.caption.c_str());
        if (s.hasVisible)    w.Key("visible").Bool(s.visible);
        if (s.hasEnabled)    w.Key("enabled").Bool(s.enabled);
        if (s.hasTabVisible) w.Key("tabVisible").Bool(s.tabVisible);
        if (s.hasActivePage) w.Key("activePageIndex").Number((wb_int64)s.activePageIndex);
        if (s.hasDown)       w.Key("down").Bool(s.down);
        if (s.hasColor) { w.Key("color").String(ColorJs(s.color)); } if (s.hasActivePageName) { w.Key("activePage").String(s.activePageName); }   // AI(W906-TEACH-FORMSHOW) 20261002
        if (s.hasItems) {
            w.Key("items").BeginArray();
            for (std::size_t i = 0; i < s.items.size(); ++i) w.String(s.items[i].c_str());
            w.EndArray();
        }
        w.EndObject();
    }
    w.EndObject();
    return w.Str();
}

std::string FormState::MessagesJson() const {
    webbridge::JsonWriter w;
    w.BeginArray();
    for (std::size_t i = 0; i < messages_.size(); ++i) {
        w.BeginObject();
        w.Key("en").String(messages_[i].first.c_str());
        w.Key("zh").String(messages_[i].second.c_str());
        w.EndObject();
    }
    w.EndArray();
    return w.Str();
}

std::string FormState::TodoJson() const {
    webbridge::JsonWriter w;
    w.BeginArray();
    for (std::size_t i = 0; i < todos_.size(); ++i) w.String(todos_[i]);
    w.EndArray();
    return w.Str();
}

bool FormState::FromJson(const std::string& widgetsJson, std::string* err) {
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        *err = "widgets is not a JSON object";
        return false;
    }
    for (const cJSON* e = root->child; e; e = e->next) {
        if (!e->string || !cJSON_IsObject(e)) continue;
        const cJSON* t = cJSON_GetObjectItemCaseSensitive(e, "text");
        const cJSON* x = cJSON_GetObjectItemCaseSensitive(e, "itemIndex");
        const cJSON* c = cJSON_GetObjectItemCaseSensitive(e, "checked");
        if (t && cJSON_IsString(t)) SetText(e->string, AnsiString(t->valuestring));
        if (x && cJSON_IsNumber(x)) SetItemIndex(e->string, x->valueint);
        if (c && cJSON_IsBool(c))   SetChecked(e->string, cJSON_IsTrue(c) != 0);
    }
    cJSON_Delete(root);
    return true;
}

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S157) 20260927 [W906]：WS form.event（Q40＝A，RULINGS_20260926 S157；格式 FROM_STEVEN 20260927 10:15）
// ---------------------------------------------------------------------------
void FormState::ClearTouched() {
    for (std::map<std::string, WidgetState>::iterator it = w_.begin(); it != w_.end(); ++it) it->second.touched = 0;
    msgMark_ = messages_.size();
    todoMark_ = todos_.size();
}

std::string FormState::TouchedJson() const {
    webbridge::JsonWriter w;
    w.BeginObject();
    for (std::size_t k = 0; k < order_.size(); ++k) {
        const WidgetState& s = w_.find(order_[k])->second;
        if (!s.touched) continue;
        w.Key(order_[k]).BeginObject();
        if (s.touched & kTouchText)       w.Key("text").String(s.text.c_str());
        if (s.touched & kTouchIndex)      w.Key("itemIndex").Number((wb_int64)s.itemIndex);
        if (s.touched & kTouchChecked)    w.Key("checked").Bool(s.checked);
        if (s.touched & kTouchCaption)    w.Key("caption").String(s.caption.c_str());
        if (s.touched & kTouchVisible)    w.Key("visible").Bool(s.visible);
        if (s.touched & kTouchEnabled)    w.Key("enabled").Bool(s.enabled);
        if (s.touched & kTouchTabVisible) w.Key("tabVisible").Bool(s.tabVisible);
        if (s.touched & kTouchActivePage) w.Key("activePageIndex").Number((wb_int64)s.activePageIndex);
        if (s.touched & kTouchDown)       w.Key("down").Bool(s.down);
        if (s.touched & kTouchColor) { w.Key("color").String(ColorJs(s.color)); } if (s.touched & kTouchActivePageName) { w.Key("activePage").String(s.activePageName); }   // AI(W906-TEACH-FORMSHOW) 20261002
        if (s.touched & kTouchItems) {
            w.Key("items").BeginArray();
            for (std::size_t i = 0; i < s.items.size(); ++i) w.String(s.items[i].c_str());
            w.EndArray();
        }
        w.EndObject();
    }
    w.EndObject();
    return w.Str();
}

std::string FormState::MessagesJsonSinceClear() const {
    webbridge::JsonWriter w;
    w.BeginArray();
    for (std::size_t i = msgMark_; i < messages_.size(); ++i) {
        w.BeginObject();
        w.Key("en").String(messages_[i].first.c_str());
        w.Key("zh").String(messages_[i].second.c_str());
        w.EndObject();
    }
    w.EndArray();
    return w.Str();
}

std::string FormState::TodoJsonSinceClear() const {
    webbridge::JsonWriter w;
    w.BeginArray();
    for (std::size_t i = todoMark_; i < todos_.size(); ++i) w.String(todos_[i]);
    w.EndArray();
    return w.Str();
}

namespace {
bool InCsv(const char* csv, const std::string& name) {
    std::string cur;
    for (const char* p = csv ? csv : "";; ++p) {
        if (*p == ',' || *p == '\0') {
            if (!cur.empty() && cur == name) return true;
            cur.clear();
            if (!*p) break;
        } else {
            cur += *p;
        }
    }
    return false;
}
std::string Num(int v) {
    char b[24];
    std::snprintf(b, sizeof(b), "%d", v);   // MinGW 6.3：不用 std::to_string
    return b;
}
}  // namespace

bool RunEvent(const BridgeDesc& b, const formevent::Request& r, formevent::Result* out) {
    const EventDesc* e = nullptr;
    for (int i = 0; b.events && i < b.nEvents; ++i)
        if (r.control == b.events[i].control && r.event == b.events[i].event) { e = &b.events[i]; break; }
    if (!e) {
        if (InCsv(b.widgets, r.control)) {
            out->code = "no-handler";
            out->why = r.control + " \"" + r.event + "\" has no translated golden handler (" + b.formClass +
                       " events: tools/formbridge/" + b.formClass + ".py)";
        } else {
            out->code = "unknown-control";
            out->why = r.control + " is not a widget of golden " + b.formClass + " (" + b.golden + " header)";
        }
        return false;
    }
    out->golden = e->golden;

    // 1. golden 開表單（display）：這一頁的元件狀態由 golden 算（不信任頁面）。副作用與 GET /api/form 開頁相同
    //    （例 HotPlate：golden FormShow 的 ReadFile 重讀 HotPlate.Data、SetArmHotPlateYPitch）。
    FormState J;
    try {
        b.display(J);
    } catch (const std::exception& x) {
        out->code = "handler-failed"; out->why = std::string("golden display threw: ") + x.what(); return false;
    } catch (...) {
        out->code = "handler-failed"; out->why = "golden display threw a non-std exception"; return false;
    }
    // 2. golden 使用者點得到嗎：控制項自己或任一層容器停用／看不見 ⇒ 滑鼠到不了，OnChange／OnClick 不會發生
    for (int i = 0; i < e->nChain; ++i) {
        const EventGuard& g = e->chain[i];
        const WidgetState* s = J.Peek(g.name);
        const bool en = (s && s->hasEnabled) ? s->enabled : g.dfmEnabled;
        const bool vis = (s && s->hasVisible) ? s->visible : g.dfmVisible;
        if (!en || !vis) {
            out->code = "bad-payload";
            out->why = r.control + " cannot be operated now: " + g.name + (en ? " is not visible" : " is disabled") +
                       " (golden DFM + display), so golden " + e->golden + " cannot fire -- reload the page";
            return false;
        }
    }
    // 3. 頁面其他控制項目前的值（選用）
    if (!r.stateJson.empty()) {
        std::string err;
        if (!J.FromJson(r.stateJson, &err)) { out->code = "bad-payload"; out->why = "state: " + err; return false; }
    }
    // 4. 控制項自己的值（VCL：選清單第 n 項 ⇒ ItemIndex=n、Text=Items[n]；點勾選框 ⇒ Checked）
    const char* ctl = e->control;
    const WidgetState* self = J.Peek(ctl);
    const int nItems = (self && self->hasItems) ? (int)self->items.size() : -1;   // -1＝golden 沒給清單
    if (r.hasIndex) {
        if (r.itemIndex < -1 || (nItems >= 0 && r.itemIndex >= nItems)) {
            out->code = "bad-payload";
            out->why = "itemIndex " + Num(r.itemIndex) + " is outside the golden list of " + r.control + " (" +
                       Num(nItems) + " items) -- reload the page";
            return false;
        }
        const AnsiString item = (nItems >= 0 && r.itemIndex >= 0) ? self->items[r.itemIndex] : AnsiString("");
        // golden 處理器拿 ItemIndex 當資料列（例 cHotPlate.cpp:422-436）：頁面清單舊了、文字對不上 ⇒ 會填錯列，拒絕
        if (r.hasText && nItems >= 0 && r.itemIndex >= 0 && std::string(item.c_str()) != r.text) {
            out->code = "bad-payload";
            out->why = "text \"" + r.text + "\" is not item " + Num(r.itemIndex) + " of the golden list (\"" +
                       std::string(item.c_str()) + "\") -- the page's list is stale, reload the page";
            return false;
        }
        J.SetItemIndex(ctl, r.itemIndex);
        if (r.hasText) J.SetText(ctl, AnsiString(r.text.c_str()));
        else if (nItems >= 0 && r.itemIndex >= 0) J.SetText(ctl, item);
    } else if (r.hasText) {
        J.SetText(ctl, AnsiString(r.text.c_str()));
    }
    if (r.hasChecked) J.SetChecked(ctl, r.checked);
    // 5. golden 處理器；changed＝它有賦值的屬性
    J.ClearTouched();
    try {
        e->handler(J);
    } catch (const std::exception& x) {
        out->code = "handler-failed"; out->why = std::string("golden ") + e->golden + " threw: " + x.what(); return false;
    } catch (...) {
        out->code = "handler-failed"; out->why = std::string("golden ") + e->golden + " threw a non-std exception"; return false;
    }
    out->changedJson = J.TouchedJson();
    out->messagesJson = J.MessagesJsonSinceClear();
    out->todoJson = J.TodoJsonSinceClear();
    return true;
}

const BridgeDesc* FindBridge(const std::string& page) {
    for (std::size_t i = 0; i < kBridgeCount; ++i)
        if (page == kBridges[i]->page) return kBridges[i];
    return nullptr;
}
std::size_t BridgeCount() { return kBridgeCount; }
const BridgeDesc* BridgeAt(std::size_t i) { return i < kBridgeCount ? kBridges[i] : nullptr; }

}  // namespace formbridge
}  // namespace ht9045
