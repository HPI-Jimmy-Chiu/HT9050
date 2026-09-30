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
//       //AI(W906-EVB1) 20260928 [W906] X-2：含 position／activePageIndex（只給 C 路；A 形狀帶了 ⇒ bad-payload，見 FileRW/_FormEvent.h）。
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

//AI(W906-EVB1) 20260928 [W906] X-2：選用的整數欄位（position／activePageIndex，同 itemIndex 的型別規則）：
//   沒有或 null ⇒ *has=false；不是數字、不是整數、超出 ±1e9 ⇒ 回 false
bool OptWhole(const cJSON* root, const char* key, bool* has, int* out) {
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(root, key);
    *has = false;
    if (!v || cJSON_IsNull(v)) return true;
    if (!cJSON_IsNumber(v) || v->valuedouble != std::floor(v->valuedouble) || v->valuedouble < -1e9 || v->valuedouble > 1e9)
        return false;
    *has = true;
    *out = (int)v->valuedouble;
    return true;
}

}  namespace formevent { namespace d013 { void Begin(const std::string& valueJson, bool hasBarcode, const std::string& barcode); void End(); void Ack(webbridge::JsonWriter& w); } }   // namespace（第一個 } 收上面的無名 namespace）  //AI(W906-D013) 20260929 [W906]：R126／R128 事件期間的 value 原文與 golden 條碼框（本體檔尾，說明 FileRW/_FormEventCtx.h）；接在同一行，不移動行號

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
        *err = Fail("bad-payload", "value must be a JSON object string {\"form\",\"control\",\"event\",\"itemIndex\",\"text\",\"checked\",\"state\",\"position\",\"activePageIndex\"}");   //AI(W906-EVB1) 20260928 [W906] X-2：多列兩個鍵
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
    //AI(W906-EVB1) 20260928 [W906] X-2：控制項自己的新值（TTrackBar／TScrollBar／TUpDown 的 position、TPageControl 的 activePageIndex；
    //   規則見 FileRW/_FormEvent.h 檔頭，套值與範圍在 FileRW/_EditPage.cpp RunPageEvent 第 3／5 步）
    if (bad.empty() && !OptWhole(root, "position", &r.hasPosition, &r.position))
        bad = "position must be a whole number or null";
    if (bad.empty() && !OptWhole(root, "activePageIndex", &r.hasPageIndex, &r.activePageIndex))
        bad = "activePageIndex must be a whole number or null";
    std::string d013Barcode; bool d013HasBarcode = false; if (bad.empty() && !OptString(root, "barcode", &d013HasBarcode, &d013Barcode)) bad = "barcode must be a string or null";   cJSON_Delete(root);   //AI(W906-D013) 20260929 [W906]：R126 "barcode"＝操作員在網頁條碼框刷到的字（A／C 兩路都收；FileRW/_FormEventCtx.h）；接在同一行
    if (bad.empty() && !r.form.empty() && r.form != formClass)
        bad = "form \"" + r.form + "\" does not match the golden form of this page (" + formClass + ")";
    //AI(W906-EVB1) 20260928 [W906] X-2：A 形狀（formbridge::RunEvent，JsonBridge/FormBridge.cpp）不讀這兩個鍵 —— 唯一的 A 形狀頁
    //   Setup.HotPlate 沒有 TTrackBar／TScrollBar／TUpDown／TPageControl 的事件（tools/formbridge/TfHotPlate.py events）。
    //   照收會讓處理器看到舊值、頁面以為送到了 ⇒ 明白拒絕。
    if (bad.empty() && b && (r.hasPosition || r.hasPageIndex))
        bad = std::string(r.hasPosition ? "position" : "activePageIndex") +
              " is only read by the C route (FileRW/_EditPage.cpp RunPageEvent); the A-shape page " + tag +
              " has no TTrackBar / TScrollBar / TUpDown / TPageControl event";
    if (!bad.empty()) { *err = Fail("bad-payload", bad); return false; }

    // 4. 跑 golden
    formevent::Result res;
    bool ok = false;
    ht9045::formjson::FormLock();  formevent::d013::Begin(valueJson, d013HasBarcode, d013Barcode);   //AI(W906-D013) 20260929 [W906]：處理器期間的 value 原文／條碼框（檔尾）；接在同一行
    try {
        ok = b ? ht9045::formbridge::RunEvent(*b, r, &res) : filerw::RunPageEvent(*d, r, &res);
    } catch (const std::exception& x) {
        ok = false; res.code = "handler-failed"; res.why = std::string("exception: ") + x.what();
    } catch (...) {
        ok = false; res.code = "handler-failed"; res.why = "non-std exception";
    }
    formevent::d013::End();  ht9045::formjson::FormUnlock();   //AI(W906-D013) 20260929 [W906]：同上，收掉（框的紀錄留給下面的 ack）；接在同一行
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
    w.Key("todo").RawValue(res.todoJson);  w.Key("closed").Bool(res.closed);  formevent::d013::Ack(w);   //AI(W906-D013) 20260929 [W906]：golden 開了條碼框才多一個 "barcode" 陣列（檔尾）  //AI(W906-EVB10A) 20260929 [W906]：golden 處理器 Close() 了（C 路 "closed" 記號；A 形狀恆 false）⇒ 頁面照 golden 關視窗；接在同一行
    w.EndObject();
    *ack = w.Str();
    return true;
}

// ===========================================================================
//  //AI(W906-D013) 20260929 [W906] todo D-013（Steven 20260929「照 BCB 的邏輯」）：form.event 跑 golden 處理器期間的狀態。
//    宣告與用途：FileRW/_FormEventCtx.h。W906_FormEvent 在 FormLock 之後 Begin、FormUnlock 之前 End（上面同一行），ack 寫 Ack。
//    R126：value 的 "barcode"＝操作員在網頁條碼框刷到的字。golden 一個框刷一次 ⇒ 只給這一次事件裡開的第一個框；
//      之後的框（同一個處理器叫第二次 Barcode_Reader）照「沒刷」關掉，ack 一樣列出來。
//    ack "barcode"：[{caption, inputType, scanned, accepted}]——只有 golden 真的開了框才有這個鍵（沒開＝ack 跟以前一模一樣）。
//      scanned＝這個框有沒有拿到頁面帶的字；accepted＝golden 的框交回非空字串（Barcode_Reader 回 1 繼續往下）。
//      scanned 且 !accepted＝刷到的字被 golden 的檢查擋掉（TimerKeyIn 清掉、或 btnEnterClick 長度 <4）。刷到的字不回傳（可能是密碼框）。
// ===========================================================================
#include "FileRW/_FormEventCtx.h"
#include <vector>

namespace formevent {
namespace {
struct D013Box {
    std::string caption, inputType;
    bool scanned = false;
    std::string result;
};
std::string g_d013Value;             // 目前事件的 value 原文（Begin～End）
bool g_d013Active = false;
bool g_d013HasScan = false;          // 頁面帶的字還沒交給任何一個框
std::string g_d013Scan;
std::vector<D013Box> g_d013Boxes;    // 這一次事件開過的框（下一次 Begin 清掉）
}  // namespace

const std::string& CurrentValueJson() { return g_d013Value; }

bool BarcodeBoxOpen(const char* caption, const char* inputType, bool* haveScan, std::string* scan)
{
    if (!g_d013Active) return false;
    D013Box b;
    b.caption = caption ? caption : "";
    b.inputType = inputType ? inputType : "";
    b.scanned = g_d013HasScan;
    *haveScan = g_d013HasScan;
    if (g_d013HasScan) { *scan = g_d013Scan; g_d013HasScan = false; }
    else scan->clear();
    g_d013Boxes.push_back(b);
    return true;
}

void BarcodeBoxClosed(const char* result)
{
    if (!g_d013Active || g_d013Boxes.empty()) return;
    g_d013Boxes.back().result = result ? result : "";
}

namespace d013 {
void Begin(const std::string& valueJson, bool hasBarcode, const std::string& barcode)
{
    g_d013Value = valueJson;
    g_d013Active = true;
    g_d013HasScan = hasBarcode;
    g_d013Scan = barcode;
    g_d013Boxes.clear();
}
void End()
{
    g_d013Value.clear();
    g_d013Active = false;
    g_d013HasScan = false;
    g_d013Scan.clear();
}
void Ack(webbridge::JsonWriter& w)
{
    if (g_d013Boxes.empty()) return;
    w.Key("barcode").BeginArray();
    for (const D013Box& b : g_d013Boxes) {
        w.BeginObject();
        w.Key("caption").String(b.caption);
        w.Key("inputType").String(b.inputType);
        w.Key("scanned").Bool(b.scanned);
        w.Key("accepted").Bool(!b.result.empty());
        w.EndObject();
    }
    w.EndArray();
}
}  // namespace d013
}  // namespace formevent
