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
//
//  AI(W906-FRW-S65) 20260926: 同一條 act.sortCT.clearCount 另外承載 golden pnlAuto1DblClick（cSortCT.cpp:1933-1950，
//    [O22] 點兩下清單站數量，會寫 lastdata.dat）：value = {"op":"dblClick","panel":"pnlAuto3Yield"}。一段式（golden 沒有確認框）。
//    說明與守衛在本檔 W906_SortCTDblClickOp 上方。
//
//  AI(W906-FRW-S97) 20260926: 同一條通道再承載 golden Timer1Timer（cSortCT.cpp:869-884，Lot ID 欄 → config.ini [Count] Lot ID）：
//    value = {"op":"lotId","text":"<edLotID 的字>"}。只有 CC_ASE_M 看得到這一欄（golden FormShow :179-183）。
//    說明在本檔 W906_SortCTLotIdOp 上方；每拍的 Timer1 見 W906_SortCTTimer1Tick（AI(W906-FRW-S97) 20260927：已接在 tools/wb_serve.cpp 主迴圈底部 :5953，St01 自己的行尾）。
// =============================================================================
#include <chrono>     // AI(W906-FRW-S97) 20260926: W906_SortCTTimer1Tick 的 1000 ms 節流
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
void W906_SortCTDblClickRun(TObject *Sender, int* exitCode, bool* wrote);     // cSortCT.cpp（golden pnlAuto1DblClick :1933-1950）   //AI(W906-FRW-S65) 20260926
void W906_SortCTTimer1Run(int* exitCode);                                     // cSortCT.cpp（golden Timer1Timer :869-884）           //AI(W906-FRW-S97) 20260926

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

//---------------------------------------------------------------------------
//  AI(W906-FRW-S65) 20260926: golden TfSortCT::pnlAuto1DblClick（V912 cSortCT.cpp:1933-1950；本檔行號同檔頭＝D:\HT9045 那份 V912，
//    D:\HT9045_ref 那份在 :410 少 8 行，這一段是 :1925-1942）的網頁入口。
//    S65 盤點第 9 項：golden 的 WriteLastDataFile 觸發者（:1946），移植樹原本沒有（forms/fSortCT.cpp 檔頭列為「沒翻」）。
//    分派：沿用既有的 act.sortCT.clearCount（value={"op":"dblClick","panel":"<面板名>"}），不另開指令 ——
//      tools/wb_serve.cpp 的指令分派行在 Jimmy 的「先不要動」清單（docs/handoff/TO_STEVEN.md §1）；同一個 golden
//      表單 TfSortCT 的另一個清除事件走同一條通道，wb_serve 不用改。沒帶 op（或 op="clearCount"）＝原本的 Clear Count。
//    守衛（不信任前端，每次重查；之後才進 golden 本體）：
//      P1 panel 必須是 golden 有接 OnDblClick=pnlAuto1DblClick 的面板：建構子 :127-135（eBulkBox 以外每列的
//         pnlYield／pnlCount，照 myCountPanel 實際指到的物件比，含 golden :109 把 eFix12 接到 pnlFix11Yield 的錯接），
//         加 DFM 的 pnlLoadCID（:107）／pnlCoverTrayD（:184）。其他面板 golden 點兩下什麼都不做 → not-wired，沒有副作用。
//      P2 面板看不見（UpForm 沒設定的站 SetVisible(false)）→ golden 的操作員點不到 → not-visible。
//    golden 本體第一個 return：SystemStart 或 O22（config.ini [O_Count] bO22_ClearSortCntByDoubleClick）沒開 → 什麼都不做。
//    寫入（本體走完時）：D:\HT9045\system\lastdata.dat、lastdata_backup.dat（WriteLastDataFile 寫死路徑，整塊 LastSet）。
//---------------------------------------------------------------------------
namespace {
struct NamedPanel { const char* name; TfSortCTPanel* p; };
const int kDblNone = 0, kDblGoldenReturn = 1, kDblBadTag = 2, kDblDone = 3;   // cSortCT.cpp W906_SortCTDblClickExit（同一組值）

std::string DblRefuse(const std::string& panel, const char* guard, const char* goldenLine, const std::string& detail)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("op").String("dblClick");
    w.Key("panel").String(panel);
    w.Key("guard").String(guard);
    w.Key("goldenLine").String(goldenLine);
    w.Key("detail").String(detail);
    w.EndObject();
    std::printf("sortct.dblClick %s -> refused (%s)\n", panel.c_str(), guard);
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}
}  // namespace

static std::string W906_SortCTDblClickOp(const std::string& panel, bool* ok)
{
    if (ok) *ok = false;
    FormLockGuard lock;                                                         // 寫 LastSet／lastdata.dat（同 Clear Count 持鎖的理由）
    W906_SortCTInstall();
    TfSortCT* f = fSortCT;
    if (f == 0) return DblRefuse(panel, "no-form", "", "fSortCT 是 NULL");

#define W906_SCT(n) { #n, f->n }
    const NamedPanel kPanels[] = {
        W906_SCT(pnlAuto1), W906_SCT(pnlAuto1Yield),
        W906_SCT(pnlAuto2), W906_SCT(pnlAuto2Yield),
        W906_SCT(pnlAuto3), W906_SCT(pnlAuto3Yield),
        W906_SCT(pnlAuto4), W906_SCT(pnlAuto4Yield),
        W906_SCT(pnlAuto5), W906_SCT(pnlAuto5Yield),
        W906_SCT(pnlAuto6), W906_SCT(pnlAuto6Yield),
        W906_SCT(pnlFix1), W906_SCT(pnlFix1Yield),
        W906_SCT(pnlFix2), W906_SCT(pnlFix2Yield),
        W906_SCT(pnlFix3), W906_SCT(pnlFix3Yield),
        W906_SCT(pnlFix4), W906_SCT(pnlFix4Yield),
        W906_SCT(pnlFix5), W906_SCT(pnlFix5Yield),
        W906_SCT(pnlFix6), W906_SCT(pnlFix6Yield),
        W906_SCT(pnlFix7), W906_SCT(pnlFix7Yield),
        W906_SCT(pnlFix8), W906_SCT(pnlFix8Yield),
        W906_SCT(pnlFix9), W906_SCT(pnlFix9Yield),
        W906_SCT(pnlFix10), W906_SCT(pnlFix10Yield),
        W906_SCT(pnlFix11), W906_SCT(pnlFix11Yield),
        W906_SCT(pnlFix12), W906_SCT(pnlFix12Yield),
        W906_SCT(pnlBinBox), W906_SCT(pnlBinBoxYield),
        W906_SCT(pnlMag1), W906_SCT(pnlMag1Yield),
        W906_SCT(pnlMag2), W906_SCT(pnlMag2Yield),
        W906_SCT(pnlMag3), W906_SCT(pnlMag3Yield),
        W906_SCT(pnlMag4), W906_SCT(pnlMag4Yield),
        W906_SCT(pnlMag5), W906_SCT(pnlMag5Yield),
        W906_SCT(pnlMag6), W906_SCT(pnlMag6Yield),
        W906_SCT(pnlMag7), W906_SCT(pnlMag7Yield),
        W906_SCT(pnlMag8), W906_SCT(pnlMag8Yield),
        W906_SCT(pnlMag9), W906_SCT(pnlMag9Yield),
        W906_SCT(pnlMag10), W906_SCT(pnlMag10Yield),
        W906_SCT(pnlMag11), W906_SCT(pnlMag11Yield),
        W906_SCT(pnlMag12), W906_SCT(pnlMag12Yield),
        W906_SCT(pnlMag13), W906_SCT(pnlMag13Yield),
        W906_SCT(pnlMag14), W906_SCT(pnlMag14Yield),
        W906_SCT(pnlLoadCID), W906_SCT(pnlCoverTrayD),
    };
#undef W906_SCT
    TfSortCTPanel* p = 0;
    for (std::size_t i = 0; i < sizeof(kPanels) / sizeof(kPanels[0]); ++i)
        if (panel == kPanels[i].name) { p = kPanels[i].p; break; }
    if (p == 0)
        return DblRefuse(panel, "bad-payload", "V912 cSortCT.dfm",
                         "panel 只收 Sort Count 頁 33 站的數量／良率格（pnlAuto1、pnlAuto1Yield…pnlMag14Yield）與 pnlLoadCID／pnlCoverTrayD");

    // P1：golden 有沒有接這個面板（建構子 :127-135 ＋ DFM :107／:184）
    bool wired = (p == f->pnlLoadCID || p == f->pnlCoverTrayD);
    for (int i = 0; i < eTrayCount && !wired; ++i) {
        if (i == eBulkBox) continue;                                            // golden :129 if(i!=eBulkBox)
        if (f->myCountPanel[i].pnlYield == p || f->myCountPanel[i].pnlCount == p) wired = true;
    }
    if (!wired)
        return DblRefuse(panel, "not-wired", "V912 cSortCT.cpp:127-135",
                         "golden 沒有把這個面板的 OnDblClick 接到 pnlAuto1DblClick（Bulk Box 那一列、或 golden :109 錯接後沒人接的 pnlFix12Yield），點兩下什麼都不做");
    // P2：看得見才點得到
    if (!p->Visible)
        return DblRefuse(panel, "not-visible", "V912 cSortCT.cpp:712-815 UpForm（SetVisible）",
                         "這一站在這台機台不顯示，golden 的操作員點不到");

    const int tag = p->Tag;
    const int bin = (tag >= 0 && tag < eTrayCount) ? iTo3Unload[tag] : -1;
    const unsigned int before = (bin >= 0 && bin < 256) ? LastSet.BinCT[0][bin] : 0u;
    int exitCode = kDblNone;
    bool wrote = false;
    W906_SortCTDblClickRun(p, &exitCode, &wrote);                               // golden :1933-1950

    if (exitCode == kDblGoldenReturn)
        return DblRefuse(panel, SystemStart ? "SystemStart" : "O22-off", "V912 cSortCT.cpp:1938-1942",
                         SystemStart ? "機台運轉中，golden 點兩下不清除"
                                     : "config.ini [O_Count] bO22_ClearSortCntByDoubleClick 沒開（[O22] 點兩下清除數量），golden 點兩下不清除");
    if (exitCode != kDblDone)
        return DblRefuse(panel, exitCode == kDblBadTag ? "bad-tag" : "body-not-run", "V912 cSortCT.cpp:1944",
                         "面板的 Tag 不在 0～32（本樹的防呆；golden 接事件的面板都在範圍內，不應該走到）");

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(true);
    w.Key("op").String("dblClick");
    w.Key("panel").String(panel);
    w.Key("tag").Number((wb_int64)tag);
    w.Key("bin").Number((wb_int64)bin);
    w.Key("before").Number((wb_int64)before);
    w.Key("after").Number((wb_int64)((bin >= 0 && bin < 256) ? LastSet.BinCT[0][bin] : 0u));
    w.Key("written").Bool(wrote);
    w.Key("goldenLine").String("V912 cSortCT.cpp:1933-1950 pnlAuto1DblClick（事件接法 :127-135）");
    w.Key("writes").BeginArray();
    w.String("D:\\HT9045\\system\\lastdata.dat（WriteLastDataFile()，golden cSortCT.cpp:1946）");
    w.String("D:\\HT9045\\system\\lastdata_backup.dat（同上）");
    w.EndArray();
    w.EndObject();
    std::printf("sortct.dblClick %s tag=%d bin=%d BinCT[0] %u -> 0 (lastdata.dat %s)\n",
                panel.c_str(), tag, bin, before, wrote ? "written" : "WRITE FAILED");
    if (ok) *ok = true;
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

//---------------------------------------------------------------------------
//  AI(W906-FRW-S97) 20260926: golden TfSortCT::Timer1Timer（V912 cSortCT.cpp:869-884）＋ Lot ID 欄的網頁入口。
//    golden 的操作：Sort Count 視窗下方 gbLotID 的 edLotID 讓操作員打批號（沒有 OnChange、沒有存檔鈕）；Timer1 每 1000 ms
//    （DFM 沒寫 Interval＝VCL 預設）比一次，機台運轉中（SystemStart）字不同就寫 config.ini [Count] Lot ID。停機時打的字
//    留在欄位裡，等開始運轉的下一拍才寫。
//    gbLotID／Timer1 只有 FormShow（:174-208）在 CUSTOMER_CODE==CC_ASE_M 時打開（:179-183）—— 客戶專屬（S25）：
//      其他客戶 golden 的操作員看不到這一欄、Timer1 永遠停用 → 這裡回 customer-only，什麼都不動。
//    value = {"op":"lotId","text":"<字>"}：text＝操作員在 edLotID 打的字（不帶 text＝不改欄位，只跑一拍）。
//    步驟（不信任前端，每次重查）：
//      1. CUSTOMER_CODE!=CC_ASE_M → customer-only。
//      2. Timer1 還沒開 → 照 golden FormShow 的兩行打開：:178 edLotID->Text=IniConfig.sLotID、:182 Timer1->Enabled=true
//         （移植樹 wb_serve 沒有呼叫 fSortCT->FormShow，見 forms/fSortCT.cpp:425-430 的 FormShow 註解；網頁送得到這個 op ＝ 操作員看得到這一欄
//         ＝ golden 的視窗開過）。
//      3. 有 text → edLotID->Text=text（操作員打字）。
//      4. 立刻跑一拍 W906_SortCTTimer1Run（golden 最多 1 秒後的那一拍）。
//    停機時（exit NotRunning）字留在 edLotID，要等 W906_SortCTTimer1Tick 在開始運轉後的那一拍才寫（golden 同）；
//    AI(W906-FRW-S97) 20260927：Tick 已接上主迴圈（wb_serve.cpp :5953），停機時打的字在開始運轉後的第一拍（≤1 秒）自己寫進去（回應 pending=true 表示還在等那一拍）。
//    寫入：D:\HT9045\config\config.ini [Count] Lot ID（AuthPath+"config.ini"），另 RecordProcess("Lot ID : …")。
//---------------------------------------------------------------------------
namespace {
bool s_w906SortCTTimer1Enabled = false;   // golden Timer1->Enabled（DFM False；FormShow :182 在 CC_ASE_M 時設 true，之後沒人關）
const int kT1None = 0, kT1InitialNotOK = 1, kT1NotRunning = 2, kT1Same = 3, kT1Wrote = 4;   // cSortCT.cpp W906_SortCTTimer1Exit（同一組值）
const char* T1Name(int c)
{
    switch (c) {
    case kT1InitialNotOK: return "initial-not-ok";
    case kT1NotRunning:   return "not-running";
    case kT1Same:         return "same";
    case kT1Wrote:        return "wrote";
    default:              return "none";
    }
}
}  // namespace

static std::string W906_SortCTLotIdOp(bool hasText, const std::string& text, bool* ok)
{
    if (ok) *ok = false;
    webbridge::JsonWriter w;
    FormLockGuard lock;                                                         // 改 IniConfig.sLotID、寫 config.ini
    W906_SortCTInstall();
    TfSortCT* f = fSortCT;
    if (f == 0) {
        w.BeginObject().Key("executed").Bool(false).Key("op").String("lotId").Key("guard").String("no-form")
         .Key("detail").String("fSortCT 是 NULL").EndObject();
        return w.Str();
    }
    if (CUSTOMER_CODE != CC_ASE_M) {                                            // golden FormShow :179（gbLotID／Timer1 只在 ASE-M 打開）
        w.BeginObject().Key("executed").Bool(false).Key("op").String("lotId").Key("guard").String("customer-only")
         .Key("goldenLine").String("V912 cSortCT.cpp:179-183")
         .Key("detail").String("gbLotID（Lot ID 欄）與 Timer1 只在 CUSTOMER_CODE==CC_ASE_M 時打開；其他客戶 golden 看不到這一欄、不會寫 config.ini [Count] Lot ID（客戶專屬，S25）")
         .EndObject();
        std::printf("sortct.lotId -> refused (customer-only, CUSTOMER_CODE=%d)\n", (int)CUSTOMER_CODE);
        return w.Str();
    }
    bool shown = false;
    if (!s_w906SortCTTimer1Enabled) {
        f->edLotID->Text=IniConfig.sLotID;                                      //Steven 20140814   // golden FormShow :178
        f->gbLotID->Visible=true;                                               // golden FormShow :181
        s_w906SortCTTimer1Enabled=true;                                         // golden FormShow :182 Timer1->Enabled=true;
        shown = true;
    }
    if (hasText) f->edLotID->Text = AnsiString(text.c_str());                   // 操作員在 edLotID 打字（golden 沒有事件，等 Timer1）

    const std::string before = IniConfig.sLotID.c_str();
    int exitCode = kT1None;
    W906_SortCTTimer1Run(&exitCode);                                            // golden Timer1Timer :869-884（一拍）
    const std::string edText = f->edLotID->Text.c_str();
    const bool pending = (exitCode == kT1NotRunning && edText != std::string(IniConfig.sLotID.c_str()));

    w.BeginObject();
    w.Key("executed").Bool(exitCode == kT1Wrote);
    w.Key("op").String("lotId");
    w.Key("result").String(T1Name(exitCode));
    w.Key("goldenLine").String("V912 cSortCT.cpp:869-884 Timer1Timer（開啟：FormShow :178-182）");
    w.Key("timerEnabledNow").Bool(shown);
    w.Key("edLotID").String(edText);
    w.Key("before").String(before);
    w.Key("after").String(IniConfig.sLotID.c_str());
    w.Key("pending").Bool(pending);                                             // 停機中打的字：golden 等開始運轉的下一拍才寫
    w.Key("systemStart").Bool(SystemStart);
    w.Key("writes").BeginArray();
    if (exitCode == kT1Wrote) w.String("D:\\HT9045\\config\\config.ini [Count] Lot ID（AuthPath+\"config.ini\"，golden cSortCT.cpp:881）");
    w.EndArray();
    w.EndObject();
    std::printf("sortct.lotId text=%s -> %s (sLotID \"%s\" -> \"%s\"%s)\n", hasText ? "set" : "-", T1Name(exitCode),
                before.c_str(), IniConfig.sLotID.c_str(), pending ? ", pending until SystemStart" : "");
    if (ok) *ok = true;                                                         // 請求被處理（含 not-running／same）
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

// AI(W906-FRW-S97) 20260926: golden Timer1（1000 ms）的每拍呼叫。主迴圈每拍都可以叫（本函式自己節流到 1000 ms）；
//   Timer1 還沒開（非 ASE-M，或網頁還沒送過 op=lotId）什麼都不做。自己持 FormLock。呼叫點 tools/wb_serve.cpp :5953（主迴圈底部，每一圈都叫；AI(W906-FRW-S97) 20260927）。
void W906_SortCTTimer1Tick()
{
    if (!s_w906SortCTTimer1Enabled) return;                                     // golden Timer1->Enabled==false：OnTimer 不會被叫
    static bool s_ran = false;
    static std::chrono::steady_clock::time_point s_last;
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (s_ran && now - s_last < std::chrono::milliseconds(1000)) return;       // golden Timer1 Interval（DFM 沒寫＝VCL 預設 1000 ms）
    s_ran = true;
    s_last = now;
    FormLockGuard lock;
    int exitCode = kT1None;
    W906_SortCTTimer1Run(&exitCode);                                            // golden Timer1Timer :869-884
    if (exitCode == kT1Wrote)
        std::printf("sortct.Timer1 -> wrote config.ini [Count] Lot ID=\"%s\"\n", IniConfig.sLotID.c_str());
}

// *ok：本次請求被處理（執行完成，或走到確認框等瀏覽器回答）＝ true；被守衛擋下／參數錯＝ false。
std::string W906_SortCTClearCount(const std::string& payloadJson, bool* ok)
{
    webbridge::JsonWriter w;
    if (ok) *ok = false;

    bool confirmed = false;
    std::string op, panel;                                                     //AI(W906-FRW-S65) 20260926: {"op":"dblClick","panel":…} → golden pnlAuto1DblClick（見上）
    std::string lotText; bool hasLotText = false;                              //AI(W906-FRW-S97) 20260926: {"op":"lotId","text":…} → golden Timer1Timer（見上）
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
        const cJSON* jop = cJSON_GetObjectItemCaseSensitive(root, "op");        //AI(W906-FRW-S65) 20260926
        if (jop && cJSON_IsString(jop)) op = jop->valuestring;
        const cJSON* jpn = cJSON_GetObjectItemCaseSensitive(root, "panel");
        if (jpn && cJSON_IsString(jpn)) panel = jpn->valuestring;
        const cJSON* jtx = cJSON_GetObjectItemCaseSensitive(root, "text");      //AI(W906-FRW-S97) 20260926
        if (jtx && cJSON_IsString(jtx)) { lotText = jtx->valuestring ? jtx->valuestring : ""; hasLotText = true; }
        cJSON_Delete(root);
    }
    if (op == "dblClick") return W906_SortCTDblClickOp(panel, ok);           //AI(W906-FRW-S65) 20260926: golden pnlAuto1DblClick（沒有確認框）
    if (op == "lotId") return W906_SortCTLotIdOp(hasLotText, lotText, ok);   //AI(W906-FRW-S97) 20260926: golden Timer1Timer（沒有確認框）
    if (!op.empty() && op != "clearCount") {                                 //AI(W906-FRW-S65) 20260926: 不認得的 op 不當 Clear Count 跑
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("guard").String("bad-payload");
        w.Key("detail").String("op 只收 clearCount（或不帶）／dblClick／lotId");   //AI(W906-FRW-S97) 20260926
        w.EndObject();
        return w.Str();
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
