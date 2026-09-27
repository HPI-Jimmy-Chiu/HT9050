// ===========================================================================
//  FileRW/_FormEvent.cpp -- WS form.event 的入口（格式與錯誤碼見 FileRW/_FormEvent.h）。
//
//  AI(W906-FRW-S157) 20260927 [W906]  NOT in golden.
//  裁決：Steven ★ Q40＝A（RULINGS_20260926 S157）；格式：FROM_STEVEN 20260927 10:15（Jimmy 10:2x 同意）。
//
//  tools/wb_serve.cpp 的 form.event 臂（主迴圈）呼叫 W906_FormEvent：
//    1. tag → 哪一頁：A 形狀 formbridge::FindBridge（"Setup.HotPlate"／"Setup.HotPlate.html"）優先（同 /api/form），
//       否則 C 路 filerw::FindPageForEvent（結構名或 PageDesc::page）。都沒有 ⇒ unknown-page。
//    2. SystemStart||SoftStart ⇒ running（RULINGS_20260927 第 2 條第 7 題 A，同 editlist.save 的擋法：golden 運轉中
//       打不開設定畫面，V912 main.cpp:3969-3978 TfMain::DoMainPadProcess 把 palSetup／palConfig 藏起來）。
//    3. 解析 value（型別不對 ⇒ bad-payload；form 和分派到的 golden 表單不同 ⇒ bad-payload）。
//    4. 持 FormLock 跑 A：formbridge::RunEvent（JsonBridge/FormBridge.cpp）／C：filerw::RunPageEvent（FileRW/_EditPage.cpp）。
//  防連點（busy:）與權杖不在這裡：分派迴圈頭的 WebCmdGuard（form.event 不在白名單 ⇒ 同 cmd＋tag＋value 400 ms 內擋）、
//  WebBridgeServer 的單一操作員權杖（form.event 不在豁免表 ⇒ 要權杖，同 form.save）。
// ===========================================================================
#include "FileRW/_FormEvent.h"

#include <cmath>
#include <exception>
#include <string>

#include "FileRW/_EditPage.h"
#include "JsonBridge/FormJson.h"      // FormLock／FormUnlock（和 editlist.*、/api/form 同一把鎖）
#include "JsonBridge/FormBridge.h"
#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"

extern bool SystemStart;   // cmydef.h:221（cmydef.cpp:286）
extern bool SoftStart;     // cmydef.h:223（cmydef.cpp:288）

namespace {

std::string Fail(const char* code, const std::string& why) { return std::string(code) + ": " + why; }

// 選用的字串欄位：沒有或 null ⇒ *has=false；不是字串 ⇒ 回 false
bool OptString(const cJSON* root, const char* key, bool* has, std::string* out) {
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(root, key);
    *has = false;
    if (!v || cJSON_IsNull(v)) return true;
    if (!cJSON_IsString(v) || !v->valuestring) return false;
    *has = true;
    *out = v->valuestring;
    return true;
}

}  // namespace

bool W906_FormEvent(const std::string& tag, const std::string& valueJson, std::string* ack, std::string* err)
{
    // 1. 哪一頁
    const ht9045::formbridge::BridgeDesc* b = ht9045::formbridge::FindBridge(tag);
    if (!b) b = ht9045::formbridge::FindBridge(tag + ".html");
    const filerw::PageDesc* d = b ? nullptr : filerw::FindPageForEvent(tag);
    if (!b && !d) {
        *err = Fail("unknown-page", "no golden form bridge for tag \"" + tag +
                    "\" (A shape: GET /api/form/ lists pages; C route: editlist struct name or its page)");
        return false;
    }
    const std::string formClass = b ? b->formClass : d->form;

    // 2. 運轉中
    if (SystemStart || SoftStart) {
        *err = Fail("running", SystemStart
            ? "機台運轉中（SystemStart）不能從網頁操作設定畫面 —— golden 運轉中打不開設定畫面（V912 main.cpp:3969-3978 DoMainPadProcess 把 palSetup／palConfig 藏起來）"
            : "機台正要啟動或回原點（SoftStart）——這時不能從網頁操作設定畫面；golden 在設定畫面開著時根本不會進入這個狀態");
        return false;
    }

    // 3. 解析 value
    cJSON* root = cJSON_Parse(valueJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        *err = Fail("bad-payload", "value must be a JSON object string {\"form\",\"control\",\"event\",\"itemIndex\",\"text\",\"checked\",\"state\"}");
        return false;
    }
    formevent::Request r;
    std::string bad;
    bool has = false;
    if (!OptString(root, "form", &has, &r.form)) bad = "form must be a string";
    else if (!OptString(root, "control", &has, &r.control) || !has || r.control.empty()) bad = "control must be a non-empty string";
    else if (!OptString(root, "event", &has, &r.event) || !has || r.event.empty()) bad = "event must be a non-empty string (\"change\"／\"click\")";
    else if (!OptString(root, "text", &r.hasText, &r.text)) bad = "text must be a string or null";
    if (bad.empty()) {
        const cJSON* x = cJSON_GetObjectItemCaseSensitive(root, "itemIndex");
        if (x && !cJSON_IsNull(x)) {
            if (!cJSON_IsNumber(x) || x->valuedouble != std::floor(x->valuedouble) || x->valuedouble < -1e9 || x->valuedouble > 1e9)
                bad = "itemIndex must be a whole number or null";
            else { r.hasIndex = true; r.itemIndex = (int)x->valuedouble; }
        }
    }
    if (bad.empty()) {
        const cJSON* c = cJSON_GetObjectItemCaseSensitive(root, "checked");
        if (c && !cJSON_IsNull(c)) {
            if (!cJSON_IsBool(c)) bad = "checked must be true, false or null";
            else { r.hasChecked = true; r.checked = cJSON_IsTrue(c) != 0; }
        }
    }
    if (bad.empty()) {
        const cJSON* s = cJSON_GetObjectItemCaseSensitive(root, "state");
        if (s && !cJSON_IsNull(s)) {
            if (!cJSON_IsObject(s)) bad = "state must be an object {name:{text?,itemIndex?,checked?}} or null";
            else {
                char* p = cJSON_PrintUnformatted(s);
                r.stateJson = p ? p : "{}";
                if (p) cJSON_free(p);
            }
        }
    }
    cJSON_Delete(root);
    if (bad.empty() && !r.form.empty() && r.form != formClass)
        bad = "form \"" + r.form + "\" does not match the golden form of this page (" + formClass + ")";
    if (!bad.empty()) { *err = Fail("bad-payload", bad); return false; }

    // 4. 跑 golden
    formevent::Result res;
    bool ok = false;
    ht9045::formjson::FormLock();
    try {
        ok = b ? ht9045::formbridge::RunEvent(*b, r, &res) : filerw::RunPageEvent(*d, r, &res);
    } catch (const std::exception& x) {
        ok = false; res.code = "handler-failed"; res.why = std::string("exception: ") + x.what();
    } catch (...) {
        ok = false; res.code = "handler-failed"; res.why = "non-std exception";
    }
    ht9045::formjson::FormUnlock();
    if (!ok) {
        *err = Fail(res.code.empty() ? "handler-failed" : res.code.c_str(), res.why);
        return false;
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("form").String(formClass);
    w.Key("control").String(r.control);
    w.Key("event").String(r.event);
    w.Key("golden").String(res.golden);
    w.Key("route").String(b ? "A" : "C");
    w.Key("changed").RawValue(res.changedJson);
    w.Key("messages").RawValue(res.messagesJson);
    w.Key("todo").RawValue(res.todoJson);
    w.EndObject();
    *ack = w.Str();
    return true;
}
