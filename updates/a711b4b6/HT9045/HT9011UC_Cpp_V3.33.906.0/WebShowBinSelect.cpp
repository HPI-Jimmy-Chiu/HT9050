// =============================================================================
//  WebShowBinSelect.cpp -- Status.ShowBinSelect.html 的 Index 分頁 Clear Counter 鈕給網頁用。
//
//  AI(W906-PROD-S114) 20260926（Steven 團隊）。NOT in golden as a file：golden 本體是 V912 cShowBinSelect.cpp:2411-2426
//  TfShowBinSelect::btnClearCountClick（逐行照翻在下面 W906_ShowBinSelectClearCount 裡，每行標 golden 行號）；
//  這裡另外只是 WS 的 JSON 包裝。
//
//  為什麼不呼叫移植樹的 fShowBinSelect->btnClearCountClick()：那一支被 GATE (B6) 擋著（cShowBinSelect.cpp:1106-1138），
//    確認框 ShowMyMessageBox_YES_NO 沒有翻，fail-closed 成「No」並在前面無條件 return —— 呼叫它什麼都不會發生。
//    網頁版的確認框由瀏覽器問（兩段式，同 WebSortCT.cpp），所以 golden 的四行在這裡重排成兩段；GATE (B6) 本身不動
//    （別人的段；那一支若還有其他呼叫者，行為維持原狀）。
//  放在 wb_serve 的來源清單（同 WebSortCT.cpp）：它要用 WebBridge 的 JsonWriter／cJSON，而 cShowBinSelect.cpp 在 ht9045_sm，
//    ht9045_sm 不連 ht9045_webbridge。CMakeLists.txt 與 tools/wb_serve.cpp 是共用檔，接線片段見交件報告。
//
//  golden（V912 cShowBinSelect.cpp）：
//    :2413-2417  if(CUSTOMER_CODE!=CC_Greatek){ if(fSecurity->Insufficient(108)==false) return; }   // 超豐清除不用權限
//    :2419-2423  int ret=ShowMyMessageBox_YES_NO("Sure To Clear Counter?", "確定是否要重新計數？"); if(ret==2) return;
//    :2425       fCounterClear->ClearCount(ctIndexCount);
//  另外：FormShow :832-835 在 CC_KYEC_LEE 把 btnClearCount 藏起來 —— golden 的操作員按不到，這裡回 not-visible。
//
//  兩段式確認（golden 的模態框在本樹沒有，wb_serve 單執行緒也不能在命令裡等框）：
//    confirmed=false → 跑 golden 守衛到確認框為止：
//        擋下       → {"executed":false,"guard":...,"goldenLine":...}
//        走到確認框 → {"executed":false,"needConfirm":true,"prompt":["Sure To Clear Counter?","確定是否要重新計數？"]}
//        （golden 對每個客戶都問，連不查權限的 CC_Greatek 也問）
//    confirmed=true  → 守衛重跑（不信任前端），確認框回 YES，執行 golden :2425 → {"executed":true,...}
//  權限：golden 自己的 fSecurity->Insufficient(108)（bAlarm 預設 true）—— 不足時 golden 會跳 WAR1676（cSecurity.cpp:669），照舊。
//
//  寫入：**沒有檔案**。golden ClearCount(ctIndexCount)（V912 cCounterClear.cpp:268-273，移植樹 :284-289）只把
//    LastSet.iIndexInputOutPut[0..3] 清 0，然後設 bRefreshCount=true；golden btnClearCountClick 自己也不存檔 ——
//    清掉的 0 要等下一個事件觸發的 WriteLastDataFile 才落地 system\lastdata.dat（golden 同）。
//    bRefreshCount 之後 golden TfMain::Timer10Timer（main.cpp:35332-35351）的收尾：AI(W906-PROD-S111) 20260926 起由 WebBridgeTags.cpp 檔尾 W906_CounterRefreshTick 在下一拍做（寫 system\Arm*.dat）。
//  不在 WebCmdGuard 白名單（會動機台記憶體）：同一個 value 連送會回 busy:。兩段的 value 不同（confirmed false／true），不會互擋。
// =============================================================================
#include <cstdio>
#include <string>

#include "WebShowBinSelect.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fSecurity.h"       // fSecurity->Insufficient
#include "forms/fCounterClear.h"   // fCounterClear->ClearCount
#include "MachineType.h"           // CC_Greatek／CC_KYEC_LEE／ctIndexCount
#include "cmydef.h"                // CUSTOMER_CODE
#include "LastSet.h"               // LastSet.iIndexInputOutPut

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:149-150

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

std::string Refuse(bool confirmed, const char* guard, const char* goldenLine, const char* detail)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("confirmed").Bool(confirmed);
    w.Key("guard").String(guard);
    w.Key("goldenLine").String(goldenLine);
    w.Key("detail").String(detail);
    w.EndObject();
    std::printf("showbinselect.clearCount confirmed=%d -> refused (%s)\n", confirmed ? 1 : 0, guard);
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

// Index 分頁的四格（golden cSortCT.cpp:425-428 寫進 IndexInput／IndexOut／OutArm_input／labInArm_input 的值）
void Counts(webbridge::JsonWriter& w)
{
    w.BeginArray();
    for (int k = 0; k < 4; ++k)
        w.Number((wb_int64)LastSet.iIndexInputOutPut[k]);
    w.EndArray();
}

}  // namespace

std::string W906_ShowBinSelectClearCount(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;

    bool confirmed = false;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0)
            return Refuse(false, "bad-payload", "", "value 不是合法 JSON（要 {\"confirmed\":bool}）");
        const cJSON* jc = cJSON_GetObjectItemCaseSensitive(root, "confirmed");
        confirmed = (jc && cJSON_IsBool(jc) && cJSON_IsTrue(jc));               // 明確 true 才算確認過
        cJSON_Delete(root);
    }

    FormLockGuard lock;                                                         // 寫 LastSet（同 counterclear.exe 持鎖的理由）

    if (CUSTOMER_CODE == CC_KYEC_LEE)                                           // golden FormShow :832-833 btnClearCount->Visible=false
        return Refuse(confirmed, "not-visible", "V912 cShowBinSelect.cpp:832-835",
                      "CC_KYEC_LEE 把 Index 分頁的 CLEAR 鈕藏起來，golden 的操作員按不到");

    if (CUSTOMER_CODE != CC_Greatek)                                            // golden :2413 //Sam 201700915 (Steven) : 超豐清除不用權限
    {
        if (fSecurity == 0)
            return Refuse(confirmed, "no-form", "", "fSecurity 是 NULL");
        if (fSecurity->Insufficient(108) == false)                              // golden :2415 //Steven 20240105 : add level for clear count
            return Refuse(confirmed, "not-authorized", "V912 cShowBinSelect.cpp:2413-2417",
                          "fSecurity->Insufficient(108)（Bin Clean Count 權限，system\\levelset.dat）；golden 同時跳 WAR1676");
    }

    // golden :2419-2423  int ret=ShowMyMessageBox_YES_NO("Sure To Clear Counter?", "確定是否要重新計數？"); if(ret==2) return;
    if (!confirmed) {
        webbridge::JsonWriter w;
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("needConfirm").Bool(true);
        w.Key("prompt").BeginArray();
        w.String("Sure To Clear Counter?");
        w.String("確定是否要重新計數？");
        w.EndArray();
        w.Key("goldenLine").String("V912 cShowBinSelect.cpp:2419");
        w.Key("counts");
        Counts(w);
        w.EndObject();
        if (ok) *ok = true;
        std::printf("showbinselect.clearCount confirmed=0 -> needConfirm\n");
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    if (fCounterClear == 0)
        return Refuse(confirmed, "no-form", "", "fCounterClear 是 NULL");

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(true);
    w.Key("confirmed").Bool(true);
    w.Key("goldenLine").String("V912 cShowBinSelect.cpp:2411-2426（btnClearCountClick → fCounterClear->ClearCount(ctIndexCount) :2425）");
    w.Key("before");
    Counts(w);
    fCounterClear->ClearCount(ctIndexCount);                                    // golden :2425
    w.Key("after");
    Counts(w);
    w.Key("writes").BeginArray().EndArray();                                    // golden 這支不存檔（見檔頭）
    w.Key("note").String("LastSet.iIndexInputOutPut[0..3] 已清 0（記憶體）；golden 這支不存檔，要等下一次事件觸發的 WriteLastDataFile 才寫進 system\\lastdata.dat");
    w.Key("deferred").BeginArray();                                             // AI(W906-PROD-S111) 20260926: 原本是 sideEffectsSkipped（S111 之前沒有讀者）
    w.String("bRefreshCount=true → 下一拍 PumpTick 的 W906_CounterRefreshTick（golden TfMain::Timer10Timer main.cpp:35332-35351）：寫 system\\Arm*.dat（WriteCTInfo）、重算 SortCT、清 Site Yield 旗標與 Yield 計數");
    w.EndArray();
    w.EndObject();
    if (ok) *ok = true;
    std::printf("showbinselect.clearCount confirmed=1 -> executed (LastSet.iIndexInputOutPut[0..3] -> 0)\n");
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}
