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
//       打不開設定畫面，V912 main.cpp:3969-3978 TfMain::DoMainPadProcess 把 palSetup／palConfig 藏起來）。⛔ 20260930 補 //AI(W906-FE-RUNEXC)：這個前提漏了 golden 的非模態表單（Show() 開著、運轉中照樣按得到）⇒ 運轉中例外表（檔尾 formevent::runexc）：表上的 (表單, 元件, 事件) 照 golden 放行，其他照舊拒收。
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

}  namespace formevent { namespace d013 { void Begin(const std::string& valueJson, bool hasBarcode, const std::string& barcode); void End(); void Ack(webbridge::JsonWriter& w); } }  namespace formevent { namespace afterack { void Ack(webbridge::JsonWriter& w); } }   // namespace（第一個 } 收上面的無名 namespace）  //AI(W906-B8-OS1B) 20261001 [W906]：after-ack 動作寫進 ack（本體檔尾）  //AI(W906-D013) 20260929 [W906]：R126／R128 事件期間的 value 原文與 golden 條碼框（本體檔尾，說明 FileRW/_FormEventCtx.h）；接在同一行，不移動行號

bool W906_FormEvent(const std::string& tag, const std::string& valueJson, std::string* ack, std::string* err)
{
    formevent::afterack::Reset();   // 1. 哪一頁   //AI(W906-B8-OS1B) 20261001 [W906]：上一次事件登記、沒被 wb_serve 跑掉的 after-ack 動作作廢（檔尾）；同一行
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
    std::string feRunWhy;  if ((SystemStart || SoftStart) && !formevent::runexc::Allowed(formClass, valueJson, &feRunWhy)) {   //AI(W906-FE-RUNEXC) 20260930 [W906]：運轉中例外表（檔尾）——(表單, 元件, 事件) 在表上、那一列允許目前的狀態、頁面表說那個表單開著 ⇒ 照 golden 往下走第 3 步；其他照舊拒收（訊息同以前）；同一行
        *err = Fail("running", !feRunWhy.empty() ? feRunWhy : SystemStart   //AI(W906-FE-RUNEXC) 20260930 [W906]：表上有這一列、但狀態格或頁面表不成立 ⇒ 回那一列的理由；同一行
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
    if (!ok) {  formevent::afterack::Reset();   //AI(W906-B8-OS1B) 20261001 [W906]：處理器失敗 ⇒ 它登記的 after-ack 動作不跑（golden 處理器丟例外就走不到後面的 fMain->Start）；同一行
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
    w.Key("todo").RawValue(res.todoJson);  w.Key("closed").Bool(res.closed);  formevent::d013::Ack(w);  formevent::afterack::Ack(w);   //AI(W906-B8-OS1B) 20261001 [W906]：登記了 after-ack 動作才多一個 "afterAck" 陣列（檔尾）  //AI(W906-D013) 20260929 [W906]：golden 開了條碼框才多一個 "barcode" 陣列（檔尾）  //AI(W906-EVB10A) 20260929 [W906]：golden 處理器 Close() 了（C 路 "closed" 記號；A 形狀恆 false）⇒ 頁面照 golden 關視窗；接在同一行
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

// ===========================================================================
//  //AI(W906-FE-RUNEXC) 20260930 [W906] 運轉中例外表（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md P-3）。
//    宣告在 FileRW/_FormEvent.h 檔尾（formevent::runexc）；呼叫端是上面 W906_FormEvent 第 2 步（:77）。安全層的改動：Steven 看過才合。
//  為什麼：第 2 步原本 SystemStart||SoftStart 一律回 running，理由是「golden 運轉中打不開設定畫面」（V912 main.cpp:3969-3978
//    TfMain::DoMainPadProcess 運轉中把 palSetup／palConfig 藏起來）。那只擋住「運轉中從兩個選單開新的表單」：golden 用 Show()（非模態）
//    開的表單運轉中照樣開著、上面的鈕照樣按得到；不在兩個選單裡的鈕（主畫面工具列 sbOffset）運轉中照樣開得了表單；golden START
//    還會自己把 Offset 打開。RULINGS_20260927 第 2 條第 7 題 A 講的是 editlist.save（運轉中存設定），不在這裡、不變。
//  規則：運轉中（SystemStart 或 SoftStart）照舊一律拒收，**除非**下面三條都成立：
//    (1) (golden 表單類別, 元件, 事件) 三個都跟下表某一列相同。表單類別用分派到的 formClass（頁面帶的 "form" 第 3 步照舊比對）；
//    (2) 那一列允許目前的狀態：SystemStart、SoftStart 各一格，照 golden 決定——處理器自己查 ⇒ 那一格 false（照舊拒收）；
//        golden 在那個狀態下表單開不著 ⇒ false。兩個同時成立要兩格都允許；
//    (3) 頁面表說那個 golden 表單現在開在某一個 HMI 上：W906_FormShowing(物件名, false)（Steven Q51 的單一函式，W906FormShowing.h；
//        成員傳 false：forms/fOffSet.h 的 TfOffSet、forms/fAGV.h 的 TfAGV 都沒有 fShow 成員）。golden 的鈕只有表單開著才按得到。
//        沒有頁面表（ctest、hook 是 0）＝關 ⇒ 不放行（往擋得住的那一側倒）。
//    放行之後照舊走第 3 步以後：解析 value、form 相符、RunPageEvent 的「開過頁、同一個等級」（第 1 步）「ELOperable 點得到」（第 2 步），
//    以及 golden 處理器自己的條件（Offset：A30／OFF_LINE／bNeedSetupTeach；AGV：golden 沒有條件）。
//    表上有這一列、但 (2) 或 (3) 不成立 ⇒ 照舊 running，理由換成那一列的（說清楚哪一條不成立）。
//  不變的：不在表上的事件（包括同一頁別的鈕：Offset 的微調鈕／部位鈕；同一顆鈕的別種事件）運轉中照舊回 running，訊息同以前；
//    editlist.save（RULINGS_20260927 第 2 條第 7 題 A）、form.save（R87）、開窗閘（FileRW/_EditPage.cpp kOpenGates）都不經過這裡。
//  執行緒：form.event 在 wb_serve 主迴圈跑，跟 MainProc 同一條（tools/wb_serve.cpp 檔頭 AI(W906-WD)）⇒ 處理器不會跟運轉中的狀態機同時跑。
//  加列（CT-3 的 Contact T.Start／T.Step 之後照同一個格式加）：每一列附 golden 出處——表單是非模態、運轉中開著或 golden 自己開、
//    處理器本身有沒有 SystemStart／SoftStart 檢查；每一列要有 ctest 證明「運轉中會跑、golden 的條件照套」
//    （目前 14 列：tests/test_b8_os5_sortbuttons.cpp [7]、tests/test_b8_ag1_initial.cpp [4]，各自核對自己那個表單的列數）。   //AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a 加 3 列（TfContact，第 15～17 列，見下面列表）⇒ 17 列；tests/test_b8_ct3a_contactflags.cpp [7] 核對 TfContact 那 3 列；同一行附加  //AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01)：CT-3b' 第 18～19 列＋Steven Q65＝B（1002 08:0x）第 20～21 列（TfContact palOTD_4／palOTD_6）⇒ 21 列；TfContact 7 列：tests/test_b8_ct3b_contactstartpause.cpp [7] 核對總數、tests/test_b8_ct3d_otddock.cpp [7] 核對 OTD 那 2 列（tests/test_b8_ct3a_contactflags.cpp [7] 只數 CT-3a 的 3 列）；同一行附加
//    Contact 的 golden 物件 fContact 在移植樹有成員 fContact->fShow；要把成員 OR 進來時再加一欄（今天的 14 列都不需要）。   //AI(W906-B8-CT3A) 20261001 [W906] (St01)：CT-3a 的 3 列也不需要——TfContactShim::fShow（atester_shims.h:157）只有建構子設 false（atester_shims.cpp:297），移植樹沒有人設 true（ctest 除外），OR 進來恆等於 false；開著沒有照問頁面表 W906_FormShowing("fContact", false)；同一行附加  //AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01)：CT-3b' 第 18～19 列、CT-3d 第 20～21 列也一樣（都問頁面表 fContact）；同一行附加
//  等級（//AI(W906-Q61-LEVEL) 20260930，Steven 20260930 Q61「golden應該是有卡權限吧? 依照golden」）：本表**不自己查等級**，等級照停機時同一套——
//    開頁的 golden 閘（FileRW/_EditPage.cpp kOpenGates；wb_serve editlist.get 在跑 golden FormShow 之前查）＋RunPageEvent 第 1 步「在同一個等級
//    開過頁」＋第 2 步 ELOperable（golden FormShow 依等級停用的容器）。golden 也是只在開窗時查、這幾顆的處理器不查（V912，cp950）：
//    Offset：sbOffsetClick main.cpp:28738-28752 不查等級（sbOffset 在 tsMain main.dfm:601，ChangeLevelAttr 不碰它 ⇒ GOffset 恆放行）；FormShow 的
//      等級只停用 pnlPicker／btnOffsetList／pnlIndexOfs（cOffSet.cpp:584-590，[02] Main - Offset＋authMainForm[2]）與 tsScale（:575-582），排序鈕在
//      tsSetupTeach > grpSetupTeach > grpOutArm（cOffSet.dfm:12765-13212），不在裡面 ⇒ 任何等級都按得到（golden 同）。
//    AGV：spbAGV 在工具選單 palSetup（main.dfm:10380），只能經 sbSettingClick（main.cpp:29060-29077：SystemStart return＋Insufficient(0)＝[00] Main - Tools）
//      開 ⇒ GAgv→GToolsMenu 同一條；等級不夠開不了頁 ⇒ 停機、運轉中都按不到。開頁後等級降了 ⇒ reload page——比 golden 嚴（golden fAGV 開著照樣按得到，
//      全樹沒有 fAGV->Close、AGV.cpp 不查等級）。沒照翻的一條（不是等級）：sbSetting->Enabled 另要 authMainForm[0]（ChangeLevelAttr main.cpp:12951，
//      config\Security_new.def [Main] Tool），GToolsMenu 沒查——所有工具選單頁都一樣，交 ST01-E（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md P-3）。⛔ 20260930 已補（AI(W906-AUTHMAINFORM)）：GToolsMenu 查 authMainForm[0]、GConfigMenu 查 [1]（:12952 [Main] Maintance）、GTeach 查 [11]（:13041-13043 [Main] Teaching），開關 0 ⇒ disabled；ctest OpenEnterLog [11]、B8_Ag1_Initial [7]、B8_Ag1_AgvIni [3]。
//    加列時每一列另寫「那個表單的開窗閘是 kOpenGates 的哪一條、FormShow 有沒有依等級停用那顆鈕的容器」；golden 處理器自己查等級的，照翻進處理器。
//    ctest：B8_Os5_SortButtons [10]、B8_Ag1_Initial [7]（等級不夠／夠 × 停機／運轉中）。
//
//  列（行號＝golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\，cp950）：   //AI(W906-E030-CITE) 20261003 [W906] (St01)：第 15～21 列（St01 的 CT-3a／CT-3b'／CT-3d）改成 golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003] 的行號＋（V912 :N），kCt*Golden 字串同（todo E-030 第 4 部分）；1～14 列不動；同一行附加
//   1-12  TfOffSet btnSortAuto1～6／btnSortFix1～6 click → cOffSet.cpp:3211-3221 btnSortAuto1Click（12 顆共用）。
//         處理器只查 IniConfig.bA30SetupTeachFunction／LastSet.iTester==OFF_LINE／LastSet.bNeedSetupTeach，不查 SystemStart／SoftStart
//         （同檔 sb_AutoOffset*Click :2894-2933 與 :3292 都有 if(SystemStart)，這一支沒有）；它設的 iSortUnloadT6 是運轉中 Out Arm 放料在讀
//         （aoutarm.cpp:3641-3643）。fOffSet 非模態：主畫面工具列的 sbOffset（main.dfm:601，父層 tsMain，不在 palSetup／palConfig 裡）
//         sbOffsetClick main.cpp:28738-28752 不查 SystemStart 就 fOffSet->Show()；A30＋bNeedSetupTeach＋離線、Contact 沒開時，
//         TfMain::Start 自己 fOffSet->Show()（main.cpp:5094-5112，在 SoftStart=true 的 :6463／:6474／:6497 之前）
//         ⇒ SystemStart 放行、SoftStart 放行。（移植樹 WebStart.cpp:1813-1848 把 Start 的這一段閘掉了（SAFETY-GATE W906-ST-W2-J）
//         ⇒ 今天是操作員自己開 Offset 頁；Offset 開頁運轉中本來就放行：FileRW/_EditPage.cpp GOffset。）
//   13    TfAGV btInitalLoad   click → Automation\AGV.cpp:1316-1322 btInitalLoadClick（InitialE84LoadTask／InitialE84LoadSensor :197-229、
//   14    TfAGV btInitalUnLoad click → Automation\AGV.cpp:1324-1330 btInitalUnLoadClick   InitialE84UnLoaderTask／InitialE84UnloadSensor、
//         清 bE84*Actionflag[0..2]；不查任何狀態）。golden 自己的 E84 狀態機運轉中也會按這兩顆：btInitalLoad->Click() :387／:723／:744、
//         btInitalUnLoad->Click() :834／:1150／:1171。fAGV 非模態：spbAGVClick main.cpp:35806-35811 fAGV->Show()。spbAGV 在 palSetup
//         （main.dfm:10380），SystemStart 時藏起來（:3969-3978，只看 SystemStart）⇒ 運轉中開不了新的，但停機或 SoftStart 時開著的會一直開著
//         （AMR 卡在埠口時操作員就是這樣救）⇒ SystemStart 放行、SoftStart 放行。移植樹開頁同 golden（FileRW/_EditPage.cpp GAgv：
//         SystemStart 時 editlist.get 回 running，SoftStart 放行）。
//         ⚠ 按下去就關 6 顆 E84 交握輸出、交握狀態機回第 1 步、放掉軌道互鎖旗標：交握進行中按＝AMR 那邊的交握中斷（golden 就是這樣）；不動軸。
//   15    TfContact btnTStart   click → 906 cContact.cpp:2251-2254（V912 :2280-2283） btnTStartClick（bSetupStart=true）     //AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」P-3）
//   16    TfContact btnTStep    click → 906 cContact.cpp:2256-2259（V912 :2285-2288） btnTStepClick（bSetupStep=true）
//   17    TfContact spbOneCycle click → 906 cContact.cpp:16861-16865（V912 :17152-17156） spbOneCycleClick → OneCycleProcess :11741-11746（V912 :11770-11775）（bContinueContact 反相）
//         三支都不查 SystemStart／SoftStart，而且就是給運轉中按的：T.Start／T.Step 是流程運轉中在等的手動鍵（golden ckernel.cpp:96-131（906＝V912 同行號） WaitManualStartKey、
//         :54-94 WaitManualStepKey 讀 fContact->bSetupStart／bSetupStep；呼叫點 Contact 狀態機 906 cContact.cpp:5976（V912 :6005）／:5999（V912 :6028）…、[D37] Manual Process
//         aTester_Front.cpp:2442（V912 :2522）／:2754-2756（V912 :2834-2836）、aTester_Rear.cpp:2424（V912 :2504）／:2728-2730（V912 :2808-2810）、BarCode\BarCode_Sh1.cpp:5397（V912 :5395）／:5499（V912 :5497））；bContinueContact 是 Contact 狀態機
//         運轉中讀的（Do_Z1_AutoGetHeight :7077（V912 :7106）／:7084（V912 :7113）、Do_Z2_AutoGetHeight :9680（V912 :9709）、Do_ContactTest_32Site :10787（V912 :10816）、DoTestContactFunction :11775（V912 :11804）／:13290（V912 :13394），
//         後者 :11771（V912 :11800） 不是 SystemStart 就 return）。fContact 非模態：sbContactClick 906 main.cpp:27312-27327（V912 :28332-28347） fContact->Show()（:27324（V912 :28344）「ShowModal --> Show」）；
//         sbContact 在 palSetup ⇒ 運轉中開不了新的（移植樹開頁閘 FileRW/_EditPage.cpp kOpenGates 的 DeviceForm_File 列＝GTools，同），停機時開著的運轉中一直開著（Contact 開著按 START＝
//         Contact 模式）⇒ SystemStart 放行、SoftStart 放行。看得見才點得到照 RunPageEvent 第 2 步：[D16] Step Contact Test（FormShow :1603-1605（V912 :1630-1632）），
//         SOFT_SIMULTE 一律看得見（:1637-1639（V912 :1664-1666））。同一頁的模式單選（rbModeNormalClick :15330（V912 :15555））不在表上：golden 運轉中只有機台沒料時點得到
//         （timerContact :21355-21365（V912 :21648-21658） 一有料就停用 rgHandlerMode，移植樹沒有這支計時器）⇒ 照舊 running（比 golden 嚴）。
//   18    TfContact btnStart    click → 906 cContact.cpp:13965-13970（V912 :14072-14077） btnStartClick（fMain->BtnStartClick＝主畫面 START 鈕 906 main.cpp:6261-6323（V912 :6529-6593）；     //AI(W906-B8-CT3B) 20261001 [W906] (St01)：B8 CT-3b'（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-3」P-2／P-3）
//                                 ＋edTorue0／edTorue1->Text="10"）
//   19    TfContact btnPause    click → 906 cContact.cpp:13972-13975（V912 :14079-14082） btnPauseClick（fMain->BtnPauseClick → Pause("BtnPauseClick") 906 main.cpp:6965-6971（V912 :7387-7393））
//         兩支都不查 SystemStart／SoftStart；golden 主畫面的 BtnStart 運轉中也不停用（ProcessKeyFlush 906 main.cpp:4098-4262（V912 :4228-4393） 只停用 CleanOut／TrayEnd／Home／
//         Reset／AlarmReset），fContact 非模態（同 15～17 列）⇒ Contact 開著、運轉中照樣按得到。運轉中按 START：golden BtnStartClick 照樣叫 Start，
//         Start 在 906 main.cpp:5871（V912 :6137） if(SystemStart) return false（在前面的檢查之後；移植樹 StartFromWeb 同一句 WebStart.cpp:3165）；扭力框照設 "10"。
//         SoftStart（回原點／加速中）時 Start 不擋（golden 同）。PAUSE 運轉中就是要停機。⇒ SystemStart 放行、SoftStart 放行。
//         兩支在處理器裡只登記 form.event after-ack 動作（本檔檔尾；FileRW/DeviceForm_File.cpp 檔尾 W906_Contact_B8BtnStartClick／W906_Contact_B8BtnPauseClick），
//         回覆之後才跑 TfMainWeb::StartFromWeb／PauseFromWeb（都可能等瀏覽器回答框）。看得見才點得到照 RunPageEvent：[D16]（FormShow :1601-1602（V912 :1628-1629）），
//         SOFT_SIMULTE 一律看得見（:1635-1636（V912 :1662-1663））。
//   20    TfContact palOTD_4 click → 906 cContact.cpp:15170-15193（V912 :15395-15418） palOTD_4Click（"OTD Under 240KG"：Dock Y 鎖、X 不鎖；再按一次回 Off 組）     //AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01)：Steven Q65＝B（1002 08:0x）
//   21    TfContact palOTD_6 click → 906 cContact.cpp:15195-15218（V912 :15420-15443） palOTD_6Click（"OTD Over 360KG"：Dock Y、X 都鎖；再按一次回 Off 組）
//         兩支都沒有任何檢查（不查 SystemStart／SoftStart、門、Index 位置、Dock 感測器），直接 Cylinder[C_DockYAxisOn／Off、C_DockXAxisOn／Off].On()／Off()
//         （產生檔 FileRW/DeviceForm_File.gen.inc DF_palOTD_4Click／DF_palOTD_6Click；跳板 Ct3dRun 在 FileRW/DeviceForm_File.cpp 檔尾，也不查狀態）。
//         fContact 非模態（sbContactClick 906 main.cpp:27312-27327（V912 :28332-28347），fContact->Show() :27324（V912 :28344））⇒ 停機時開著的 Contact 運轉中一直開著，golden 的 OTD 照樣按得到。
//         跟 15～19 列不同：流程不等這兩顆，按了就在運轉中動 Dock 氣缸 ⇒ CT-3d 交件時沒加列、交 Steven（Q65）；Steven Q65＝B（1002 08:0x）：照 golden 放行
//         ⇒ SystemStart 放行、SoftStart 放行（規則例外，human-review C；上機由 EastSun 在 USE_OTD==1 的機台看）。看得見才點得到照 RunPageEvent 第 2 步：
//         USE_OTD==1（FormShow :1310-1311（V912 :1329-1330））。處理器之後照舊補 TimerEP、OTDTimer 各一拍（golden OTDTimer 運轉中也在跑）。
// ===========================================================================
#include "W906FormShowing.h"
#include <cstdio>

namespace formevent {
namespace runexc {
namespace {
const char kSortGolden[] =
    "golden V912 cOffSet.cpp:3211-3221 TfOffSet::btnSortAuto1Click (no SystemStart / SoftStart check); fOffSet is non-modal: "
    "sbOffsetClick main.cpp:28708-28722 (main toolbar, not in palSetup / palConfig), TfMain::Start main.cpp:5094-5112 opens it itself";
const char kAgvLoadGolden[] =
    "golden V912 Automation\\AGV.cpp:1316-1322 TfAGV::btInitalLoadClick (no state check; golden's own E84 flow clicks it too, :387 / :723 / :744); "
    "fAGV is non-modal: spbAGVClick main.cpp:35776-35781";
const char kAgvUnloadGolden[] =
    "golden V912 Automation\\AGV.cpp:1324-1330 TfAGV::btInitalUnLoadClick (no state check; golden's own E84 flow clicks it too, :834 / :1150 / :1171); "
    "fAGV is non-modal: spbAGVClick main.cpp:35776-35781";
const char kCtTStartGolden[] =   //AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a 第 15～17 列（見上面列表）
    "golden 906 cContact.cpp:2251-2254 (V912 :2280-2283) TfContact::btnTStartClick (bSetupStart=true; no SystemStart / SoftStart check); read during the run by "
    "ckernel.cpp:96-131 (same in V912) WaitManualStartKey (contact SM cContact.cpp:5976 (V912 :6005) / :6497 (V912 :6526), aTester_Front.cpp:2442 (V912 :2522) / :2756 (V912 :2836), aTester_Rear.cpp:2424 (V912 :2504) / :2730 (V912 :2810)); "
    "fContact is non-modal: sbContactClick 906 main.cpp:27312-27327 (V912 :28302-28317), fContact->Show() :27324 (V912 :28314)";
const char kCtTStepGolden[] =
    "golden 906 cContact.cpp:2256-2259 (V912 :2285-2288) TfContact::btnTStepClick (bSetupStep=true; no SystemStart / SoftStart check); read during the run by "
    "ckernel.cpp:54-94 (same in V912) WaitManualStepKey (contact SM cContact.cpp:5999 (V912 :6028) / :6164 (V912 :6193), aTester_Front.cpp:2754 (V912 :2834), aTester_Rear.cpp:2728 (V912 :2808), BarCode_Sh1.cpp:5397 (V912 :5395)); "
    "fContact is non-modal: sbContactClick 906 main.cpp:27312-27327 (V912 :28302-28317), fContact->Show() :27324 (V912 :28314)";
const char kCtOneCycleGolden[] =
    "golden 906 cContact.cpp:16861-16865 (V912 :17152-17156) TfContact::spbOneCycleClick -> OneCycleProcess :11741-11746 (V912 :11770-11775) (no SystemStart / SoftStart check); "
    "bContinueContact is read during the run by the contact SMs (Do_Z1_AutoGetHeight :7077 (V912 :7106), Do_Z2_AutoGetHeight :9680 (V912 :9709), Do_ContactTest_32Site :10787 (V912 :10816), "
    "DoTestContactFunction :11775 (V912 :11804) / :13290 (V912 :13394)); fContact is non-modal: sbContactClick 906 main.cpp:27312-27327 (V912 :28302-28317), fContact->Show() :27324 (V912 :28314)";
const char kCtStartGolden[] =   //AI(W906-B8-CT3B) 20261001 [W906] (St01)：B8 CT-3b' 第 18～19 列（見上面列表）
    "golden 906 cContact.cpp:13965-13970 (V912 :14072-14077) TfContact::btnStartClick -> TfMain::BtnStartClick 906 main.cpp:6261-6323 (V912 :6529-6593) (main START button; no SystemStart / "
    "SoftStart check; while running TfMain::Start refuses at 906 main.cpp:5871 (V912 :6137)) + edTorue0 / edTorue1 = 10; main BtnStart stays enabled while running "
    "(ProcessKeyFlush 906 main.cpp:4098-4262 (V912 :4228-4393)); fContact is non-modal: sbContactClick 906 main.cpp:27312-27327 (V912 :28302-28317), fContact->Show() :27324 (V912 :28314)";
const char kCtPauseGolden[] =
    "golden 906 cContact.cpp:13972-13975 (V912 :14079-14082) TfContact::btnPauseClick -> TfMain::BtnPauseClick 906 main.cpp:6965-6971 (V912 :7387-7393) -> Pause(BtnPauseClick) 906 main.cpp:6325 (V912 :6595) "
    "(no SystemStart / SoftStart check; pausing a run is its purpose); fContact is non-modal: sbContactClick 906 main.cpp:27312-27327 (V912 :28302-28317), fContact->Show() :27324 (V912 :28314)";
const char kCtOtd4Golden[] =   //AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01)：B8 CT-3d 第 20～21 列（見上面列表），Steven Q65＝B（1002 08:0x）
    "golden 906 cContact.cpp:15170-15218 (V912 :15395-15443) TfContact::palOTD_4Click :15170-15193 (V912 :15395-15418) (OTD Under 240KG; no check at all -- Cylinder[C_DockYAxisOn / "
    "C_DockYAxisOff / C_DockXAxisOn / C_DockXAxisOff].On() / Off()); fContact is non-modal: sbContactClick 906 main.cpp:27312-27327 (V912 :28302-28317), "
    "fContact->Show() :27324 (V912 :28314); allowed while running: Steven Q65 = B (1002 08:0x)";
const char kCtOtd6Golden[] =
    "golden 906 cContact.cpp:15170-15218 (V912 :15395-15443) TfContact::palOTD_6Click :15195-15218 (V912 :15420-15443) (OTD Over 360KG; no check at all -- Cylinder[C_DockYAxisOn / "
    "C_DockYAxisOff / C_DockXAxisOn / C_DockXAxisOff].On() / Off()); fContact is non-modal: sbContactClick 906 main.cpp:27312-27327 (V912 :28302-28317), "
    "fContact->Show() :27324 (V912 :28314); allowed while running: Steven Q65 = B (1002 08:0x)";
const RowInfo kRows[] = {
    // 表單類別    元件              事件     golden 物件  SystemStart  SoftStart  golden 出處
    {"TfOffSet", "btnSortAuto1",   "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortAuto2",   "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortAuto3",   "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortAuto4",   "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortAuto5",   "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortAuto6",   "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortFix1",    "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortFix2",    "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortFix3",    "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortFix4",    "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortFix5",    "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfOffSet", "btnSortFix6",    "click", "fOffSet",  true,        true,      kSortGolden},
    {"TfAGV",    "btInitalLoad",   "click", "fAGV",     true,        true,      kAgvLoadGolden},
    {"TfAGV",    "btInitalUnLoad", "click", "fAGV",     true,        true,      kAgvUnloadGolden},
    {"TfContact", "btnTStart",     "click", "fContact", true,        true,      kCtTStartGolden},     //AI(W906-B8-CT3A) 20261001 [W906] (St01)：B8 CT-3a
    {"TfContact", "btnTStep",      "click", "fContact", true,        true,      kCtTStepGolden},
    {"TfContact", "spbOneCycle",   "click", "fContact", true,        true,      kCtOneCycleGolden},
    {"TfContact", "btnStart",      "click", "fContact", true,        true,      kCtStartGolden},      //AI(W906-B8-CT3B) 20261001 [W906] (St01)：B8 CT-3b'
    {"TfContact", "btnPause",      "click", "fContact", true,        true,      kCtPauseGolden},
    {"TfContact", "palOTD_4",      "click", "fContact", true,        true,      kCtOtd4Golden},       //AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01)：Steven Q65＝B（1002 08:0x）
    {"TfContact", "palOTD_6",      "click", "fContact", true,        true,      kCtOtd6Golden},
};
const int kRowCount = (int)(sizeof(kRows) / sizeof(kRows[0]));
}  // namespace

int RowCount() { return kRowCount; }
const RowInfo* RowAt(int i) { return i >= 0 && i < kRowCount ? &kRows[i] : nullptr; }

bool Allowed(const std::string& formClass, const std::string& valueJson, std::string* why)
{
    why->clear();
    std::string control, event;                 // 只讀這兩個鍵；value 其他的驗證照舊在第 3 步（解析不了＝不在表上＝照舊拒收）
    cJSON* v = cJSON_Parse(valueJson.c_str());
    if (v && cJSON_IsObject(v)) {
        const cJSON* c = cJSON_GetObjectItemCaseSensitive(v, "control");
        const cJSON* e = cJSON_GetObjectItemCaseSensitive(v, "event");
        if (cJSON_IsString(c) && c->valuestring) control = c->valuestring;
        if (cJSON_IsString(e) && e->valuestring) event = e->valuestring;
    }
    if (v) cJSON_Delete(v);
    const RowInfo* row = nullptr;
    for (int i = 0; i < kRowCount && !row; ++i)
        if (formClass == kRows[i].form && control == kRows[i].control && event == kRows[i].event) row = &kRows[i];
    if (!row) return false;                     // 不在表上：照舊拒收（W906_FormEvent 用原本的訊息）
    const std::string state = SystemStart && SoftStart ? "SystemStart+SoftStart" : SystemStart ? "SystemStart" : "SoftStart";
    const std::string what = formClass + "." + control + " \"" + event + "\"";
    if ((SystemStart && !row->systemStart) || (SoftStart && !row->softStart)) {
        *why = "機台運轉中（" + state + "）：" + what + " 在運轉中例外表上，但這一列 " +
               (SystemStart && !row->systemStart ? "SystemStart" : "SoftStart") + " 時不放行（照 golden；FileRW/_FormEvent.cpp 檔尾）";
        return false;
    }
    if (!W906_FormShowing(row->shownObj, false)) {
        *why = "機台運轉中（" + state + "）：" + what + " 只有 golden 表單 " + row->shownObj +
               " 開著時才收（golden 的鈕只有表單開著才按得到）——頁面表說它現在沒有開在任何 HMI 上（W906_FormShowing；FileRW/_FormEvent.cpp 檔尾）";
        return false;
    }
    std::printf("[FE-RUNEXC] form.event %s while %s: allowed, %s is open -- %s\n", what.c_str(), state.c_str(), row->shownObj, row->golden);
    std::fflush(stdout);
    return true;
}
}  // namespace runexc
}  // namespace formevent

// ===========================================================================
//  //AI(W906-B8-OS1B) 20261001 [W906] form.event 的 after-ack 動作（宣告 FileRW/_FormEvent.h 檔尾；
//    D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「OS-1b」7.「StartFromWeb 在 form.event 持 FormLock 的時候呼叫會不會卡住」）。
//  為什麼：golden 有處理器最後叫 fMain->Start（第一個使用者：B8 OS-1b，TfOffSet::sb_AutoOffsetUp／Down／Right／LeftClick，
//    906 cOffSet.cpp:2895-2898（V912 :2899-2902）／:2908-2911（V912 :2912-2915）／:2921-2924（V912 :2925-2928）／:2934-2937（V912 :2938-2941））。移植樹的 START 是 TfMainWeb::StartFromWeb（WebStart.cpp），
//    裡面有等瀏覽器回答的 YES／NO 框（ShowMyMessageBox_YES_NO → tools/wb_serve.cpp MbWait 的巢狀 pump）。W906_FormEvent 跑處理器時
//    持 FormLock（JsonBridge/FormJson.cpp，可重入的 CRITICAL_SECTION）；WebBridgeServer 只有一條 socket 執行緒（WebBridge/WebBridgeServer.h:16），
//    它服務 GET /api/editlist 時也拿這把鎖（tools/wb_serve.cpp ApiRoute）——在鎖裡等框的回答，socket 執行緒卡在鎖上就收不到那個回答＝整台卡死。
//    所以處理器只「登記」，wb_serve 的 form.event 臂在 CompleteCommand（ack 已送出）之後、鎖外、同一條主迴圈執行緒上跑——跟 start.run 同一個位置與執行緒。
//  時序：跟 golden 一樣在同一次點擊裡、處理器跑完之後；中間只隔「送 ack」，同一個 drain 迴圈，沒有 MainProc 拍子夾在中間。
//  規則：W906_FormEvent 開頭 Reset（上一次沒被跑的作廢：ctest、沒有 wb_serve 臂的程式不會累積）；處理器失敗 ⇒ Reset（不跑）；
//    成功 ⇒ ack 多一個 "afterAck":[說明…]（沒登記＝ack 跟以前一模一樣）；W906_FormEventRunAfterAck(ok) 先把清單搬出來再跑——
//    跑的時候可能進巢狀 pump、又收到下一個 form.event，那一個的 Reset 不會動到正在跑的這一份。每一項各自 try／catch。
//  不在這裡判斷能不能跑：每一個動作自己帶 golden 的檢查（OS-1b：START 全套在 StartFromWeb；FileRW/Offset_File.cpp 檔尾）。
// ===========================================================================
namespace formevent {
namespace afterack {
namespace {
struct Item {
    void (*fn)(const std::string& arg);
    std::string arg;
    std::string what;
};
std::vector<Item> g_afterAck;
}  // namespace
void Defer(void (*fn)(const std::string& arg), const std::string& arg, const std::string& what)
{
    if (!fn) return;
    Item it;
    it.fn = fn;
    it.arg = arg;
    it.what = what;
    g_afterAck.push_back(it);
}
void Reset() { g_afterAck.clear(); }
int Pending() { return (int)g_afterAck.size(); }
void Ack(webbridge::JsonWriter& w)
{
    if (g_afterAck.empty()) return;
    w.Key("afterAck").BeginArray();
    for (const Item& it : g_afterAck) w.String(it.what);
    w.EndArray();
}
void RunAll(bool ok)
{
    std::vector<Item> run;
    run.swap(g_afterAck);
    if (!ok) {
        if (!run.empty()) std::printf("[FE-AFTERACK] form.event failed -- %d queued action(s) dropped\n", (int)run.size());
        return;
    }
    for (const Item& it : run) {
        std::printf("[FE-AFTERACK] run: %s\n", it.what.c_str());
        std::fflush(stdout);
        try {
            it.fn(it.arg);
        } catch (const std::exception& x) {
            std::printf("[FE-AFTERACK] exception in \"%s\": %s\n", it.what.c_str(), x.what());
        } catch (...) {
            std::printf("[FE-AFTERACK] non-std exception in \"%s\"\n", it.what.c_str());
        }
    }
    std::fflush(stdout);
}
}  // namespace afterack
}  // namespace formevent

void W906_FormEventRunAfterAck(bool ok) { formevent::afterack::RunAll(ok); }
