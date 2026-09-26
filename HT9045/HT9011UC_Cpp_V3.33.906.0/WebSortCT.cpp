// =============================================================================
//  WebSortCT.cpp -- Data.SortCT.html 的 Clear Count 鈕（golden TfSortCT::btnClearCountClick）給網頁用。
//
//  Steven 20260925 (Data.SortCT)。NOT in golden as a file：golden 本體在根目錄 cSortCT.cpp
//  （W906Body_btnClearCountClick，逐行翻 V912 cSortCT.cpp:585-708）；這裡只是 WS 的 JSON 包裝。
//  放在 wb_serve 的來源清單（同 WebLogin.cpp）：它要用 WebBridge 的 JsonWriter／cJSON，
//  而 cSortCT.cpp 在 ht9045_sm，ht9045_sm 不連 ht9045_webbridge。
//
//  ⚠ 分派還沒接：tools/wb_serve.cpp 這一波有別的工程師在改（observer.get），所以這裡**只提供函式**，
//    分派由整合者加。建議（二選一）：
//      (a) JsonBridge/ChanAction.cpp 加 #include "WebSortCT.h"，HandleActionWithTag 加一行
//            if (cmd == "act.sortCT.clearCount") { bool ok = false; return ::W906_SortCTClearCount(payloadJson, &ok); }
//          （act.* 的 ok 判斷是回應裡有沒有 "executed":true —— 本函式的回應照這個慣例；
//            ⚠ 不要寫成區塊內 extern：HandleActionWithTag 在 namespace ht9045::sjson 裡，那樣會宣告成別的符號）
//      (b) tools/wb_serve.cpp 照 counterclear.* 的形狀加一臂（本函式自己持 FormLock，外面再包一層也不會死結：
//          FormLock 是 CRITICAL_SECTION，同執行緒可重入，JsonBridge/FormJson.cpp:149）。
//    網頁（web/page/ht9045_sortct_wire.js）送的是 act.sortCT.clearCount，value = JSON 字串 {"confirmed":bool}。
//
//  兩段式確認（golden :637 的模態框在本樹沒有，wb_serve 單執行緒也不能在命令裡等框）：
//    confirmed=false → golden 守衛跑到確認框為止：
//        擋下 → {"executed":false,"guard":...,"goldenLine":...}
//        走到確認框 → {"executed":false,"needConfirm":true,"prompt":["Clear Sort Count?","確定要清空計數？"]}
//        golden 不問的客戶（CC_Greatek，:635）→ 直接執行完 → {"executed":true,...}
//    confirmed=true  → 守衛重跑（不信任前端），確認框回 YES，執行 golden 本體 → {"executed":true,...}
//  權限：golden 自己的 fSecurity->Insufficient(108)（:595）—— 不足時 golden 會跳 WAR1676（cSecurity.cpp:668），照舊。
// =============================================================================
#include <cstdio>
#include <string>

#include "WebSortCT.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fSortCT.h"
#include "MachineType.h"
#include "cmydef.h"        // SystemStart / CUSTOMER_CODE
#include "cprod.h"         // RunInfo
#include "LastSet.h"       // LastSet

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:149-150

bool W906_SortCTInstall();                                                     // cSortCT.cpp
void W906_SortCTClearRun(bool confirmed, int* exitCode, bool* confirmReached); // cSortCT.cpp

namespace {

// cSortCT.cpp 的 W906_SortCTClearExit（同一組值；刻意不放標頭，避免 forms/fSortCT.h 背一個網頁用的列舉）
enum { kNone = 0, kSystemStart, kNotAuthorized, kTsmcIc, kAseClIc, kKyec, kConfirmNo, kSystemStartAfterConfirm, kDone };

struct ExitDoc { int code; const char* guard; const char* goldenLine; const char* detail; };
const ExitDoc kExitDocs[] = {
    { kNone,          "body-not-installed", "",                     "g_W906_SortCTBodies 是 NULL -- cSortCT.cpp 沒有連進來，btnClearCountClick 仍是 no-op" },
    { kSystemStart,   "SystemStart",        "V912 cSortCT.cpp:587-588", "機台運轉中不能清除（golden 第一行守衛）" },
    { kNotAuthorized, "not-authorized",     "V912 cSortCT.cpp:593-597", "fSecurity->Insufficient(108)（Bin Clean Count 權限，system\\levelset.dat）；golden 同時跳 WAR1676" },
    { kTsmcIc,        "ic-in-machine",      "V912 cSortCT.cpp:599-600", "CC_TSMC_TAINAN：機台內有 IC 不能清除 Count" },
    { kAseClIc,       "ic-in-machine",      "V912 cSortCT.cpp:602-610", "CC_ASE_CL：手臂／Shuttle／Index／旋轉站有 IC 不能清除 Count" },
    { kKyec,          "kyec-reauth",        "V912 cSortCT.cpp:612-634", "CC_KYEC_LEE 要強制登出後刷 Barcode 重登；本樹沒有登入框（cSortCT.cpp GATE G6）" },
    { kConfirmNo,     "confirm-no",         "V912 cSortCT.cpp:637-640", "確認框回答 NO" },
    { kSystemStartAfterConfirm, "SystemStart", "V912 cSortCT.cpp:643-644", "確認後機台已運轉（golden 的第二道守衛）" },
};

const ExitDoc* FindDoc(int code)
{
    for (std::size_t i = 0; i < sizeof(kExitDocs) / sizeof(kExitDocs[0]); ++i)
        if (kExitDocs[i].code == code) return &kExitDocs[i];
    return 0;
}

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

}  // namespace

// *ok：本次請求被處理（執行完成，或走到確認框等瀏覽器回答）＝ true；被守衛擋下／參數錯＝ false。
std::string W906_SortCTClearCount(const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    if (ok) *ok = false;

    bool confirmed = false;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0) {
            w.BeginObject();
            w.Key("executed").Bool(false);
            w.Key("guard").String("bad-payload");
            w.Key("detail").String("value 不是合法 JSON（要 {\"confirmed\":bool}）");
            w.EndObject();
            return w.Str();
        }
        const cJSON* jc = cJSON_GetObjectItemCaseSensitive(root, "confirmed");
        confirmed = (jc && cJSON_IsBool(jc) && cJSON_IsTrue(jc));             // 明確 true 才算確認過
        cJSON_Delete(root);
    }

    int exitCode = kNone;
    bool reached = false;
    const long sendBefore  = LastSet.SendCT[0];
    const int  unloadBefore = RunInfo.iUnloadCount;
    {
        FormLockGuard lock;                                                     // 寫 LastSet／lastdata.dat（同 counterclear.exe 持鎖的理由）
        W906_SortCTInstall();
        W906_SortCTClearRun(confirmed, &exitCode, &reached);
    }

    w.BeginObject();
    if (exitCode == kDone) {
        w.Key("executed").Bool(true);
        w.Key("confirmed").Bool(confirmed);
        w.Key("goldenLine").String("V912 cSortCT.cpp:585-708（btnClearCountClick → fMain->Clarn_Data(8) :662）");
        w.Key("before").BeginObject();
        w.Key("loading").Number((wb_int64)sendBefore);
        w.Key("unload").Number((wb_int64)unloadBefore);
        w.EndObject();
        w.Key("after").BeginObject();
        w.Key("loading").Number((wb_int64)LastSet.SendCT[0]);
        w.Key("unload").Number((wb_int64)RunInfo.iUnloadCount);                 // ShowSortIC（經 Clarn_Data :15631）重算的 Sum
        w.EndObject();
        w.Key("writes").BeginArray();                                           // JsonBridge/actions/MainClarnData.h 檔頭
        w.String("D:\\HT9045\\system\\lastdata.dat（WriteLastDataFile(false)，Clarn_Data golden main.cpp:15629）");
        w.String("D:\\HT9045\\system\\lastdata_backup.dat（同上，cprod.cpp:2047）");
        w.String("D:\\HT9045_Log\\QtyData\\YYYYMM\\<PC>_<date>.csv（QtyLog；本行程第 2 次 Clarn_Data 起才寫，golden 的 static FileName）");
        w.EndArray();
        w.Key("sideEffectsSkipped").BeginArray();
        w.String("MyDBIProductionData(\"Clear Sorting Count\") -- wb_serve 沒開 sqlite（bUseMDB=false），文字 event log 也在 GA1-B4 閘內：一筆都不落地");
        w.String("fContactCT->sgYield->Refresh() -- VCL 重繪（G8），畫面由 tag 更新");
        w.String("fSCKART->UpdateCount() -- facade 沒有（G9；SCK 93K ART 視窗開著才走）");
        w.EndArray();
        if (ok) *ok = true;
    } else if (exitCode == kConfirmNo && !confirmed && reached) {
        w.Key("executed").Bool(false);
        w.Key("needConfirm").Bool(true);
        w.Key("prompt").BeginArray();                                           // golden :637 的兩行字（英文／中文）
        w.String("Clear Sort Count?");
        w.String("確定要清空計數？");
        w.EndArray();
        w.Key("goldenLine").String("V912 cSortCT.cpp:637");
        if (ok) *ok = true;
    } else {
        const ExitDoc* d = FindDoc(exitCode);
        w.Key("executed").Bool(false);
        w.Key("guard").String(d ? d->guard : "unknown-exit");
        w.Key("goldenLine").String(d ? d->goldenLine : "");
        w.Key("detail").String(d ? d->detail : "");
    }
    w.Key("exitCode").Number((wb_int64)exitCode);
    w.Key("confirmReached").Bool(reached);
    w.EndObject();

    std::printf("sortct.clear confirmed=%d -> exit=%d reached=%d (SendCT[0] %ld->%ld, iUnloadCount %d->%d)\n",
                confirmed ? 1 : 0, exitCode, reached ? 1 : 0, sendBefore, (long)LastSet.SendCT[0],
                unloadBefore, RunInfo.iUnloadCount);
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}
