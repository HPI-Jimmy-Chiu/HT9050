// ===========================================================================
//  JsonBridge/FormBridge.cpp -- FormState 本體。說明見 FormBridge.h。
//
//  Steven 20260924.  NOT in golden.
// ===========================================================================
#include "JsonBridge/FormBridge.h"

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

void FormState::SetText(const char* id, const AnsiString& v)    { WidgetState& s = W(id); s.hasText = true;       s.text = v; }
void FormState::SetItemIndex(const char* id, int v)            { WidgetState& s = W(id); s.hasIndex = true;      s.itemIndex = v; }
void FormState::SetChecked(const char* id, bool v)             { WidgetState& s = W(id); s.hasChecked = true;    s.checked = v; }
void FormState::SetCaption(const char* id, const AnsiString& v) { WidgetState& s = W(id); s.hasCaption = true;    s.caption = v; }
void FormState::SetVisible(const char* id, bool v)             { WidgetState& s = W(id); s.hasVisible = true;    s.visible = v; }
void FormState::SetEnabled(const char* id, bool v)             { WidgetState& s = W(id); s.hasEnabled = true;    s.enabled = v; }
void FormState::SetTabVisible(const char* id, bool v)          { WidgetState& s = W(id); s.hasTabVisible = true; s.tabVisible = v; }
void FormState::SetActivePageIndex(const char* id, int v)      { WidgetState& s = W(id); s.hasActivePage = true; s.activePageIndex = v; }
void FormState::SetDown(const char* id, bool v)                { WidgetState& s = W(id); s.hasDown = true;       s.down = v; }

void FormState::ItemsClear(const char* id) {
    // VCL TComboBox::Clear 清掉 Items 與 Text，ItemIndex 回 -1。
    WidgetState& s = W(id);
    s.hasItems = true; s.items.clear();
    s.hasText = true;  s.text = AnsiString("");
    s.hasIndex = true; s.itemIndex = -1;
}
void FormState::ItemsAdd(const char* id, const AnsiString& v) {
    WidgetState& s = W(id); s.hasItems = true; s.items.push_back(v);
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

const BridgeDesc* FindBridge(const std::string& page) {
    for (std::size_t i = 0; i < kBridgeCount; ++i)
        if (page == kBridges[i]->page) return kBridges[i];
    return nullptr;
}
std::size_t BridgeCount() { return kBridgeCount; }
const BridgeDesc* BridgeAt(std::size_t i) { return i < kBridgeCount ? kBridges[i] : nullptr; }

}  // namespace formbridge
}  // namespace ht9045
