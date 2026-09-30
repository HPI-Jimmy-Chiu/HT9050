// =============================================================================
//  test_b8_m11_ftrt.cpp -- B8 M-11：主畫面 FT／RT 小方塊（golden TfMain::palFTClick／palRTClick → DoFTRTClick → FTClick／RTClick）
//
//  //AI(W906-B8-M11) 20260930 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「M-11」
//    （Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
//  golden：V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp :30700-30708（palFTClick／palRTClick）、:35752-35774（DoFTRTClick）、
//    :30710-30832（FTClick）、:30834-31001（RTClick）；點不點得到＝palFT／palRT->Visible 與 ChangeLevelAttr :12937-13038 算的 Enabled。
//  受測的是 wb_serve 編進去的同一份 FileRW/MainClick.cpp（檔尾 W906_Main_FtRtOp／W906_Main_DoFTRTClick／FtRtStartModeEnabled，經真的
//    WS 分派 W906_Main_EvB6Op("act.main.ftrt", …)）；CheckCanChangeRealDummy／HasICUnderMachine／HasAnyICInMachine 等都是 ht9045_sm 的活本體
//    （god-stack RESCAN）。有沒有料由 tests/test_b8_su7_ic.cpp（SU-7 的注入 TU）改活物件的格子。MainClick.cpp 其他入口要的 wb_serve 專屬函式
//    由 tests/test_main_ctlbuttons_stubs.cpp（筆電 FLOW-4 的連結替身，原樣共用、不改；被叫到就 abort）給。
//  ⚠ golden cbRunStartModeChange（Jimmy 的 RunStartMode.cpp W906_CbRunStartModeChange → SetRunStartMode）會寫機台檔（test_tester_connect_rules 記過），
//    ctest 不能跑 ⇒ 換成記錄器（本段的測試縫 W906_FtRt_RunStartModeChangeFn）：驗「golden 把下拉設成哪一項、有沒有叫 OnChange」。
//    同理不開 SPIL（Clarn_Data 真本體只在 wb_serve 裝）、[O21]（ClearCount 寫檔）、2DID 白名單 Lot End（sbSECSLotEndClick 出報表）—— 這三條由 [9] 棘輪看。
//    [1] get：機台空、等級夠、authMainForm[9] ⇒ 起動模式可改，FT／RT 看得見、按得到
//    [2] FT：Re-Test Continuous → Initial Start（機台空）；Continuous Start 留著；OnChange 叫一次；Run Mode 字 "Normal"
//    [3] RT：Continuous Start → Re-Test Continuous（W906_bRTContinueNeedClearCT）；Initial Start → Re-Test Initial Start；Run Mode 字 "RT"
//    [4] 鎖住（golden ChangeLevelAttr）：六種有料、Loader 有料、等級不夠、authMainForm[9] 關、運轉中 ⇒ 小方塊停用、回 disabled、什麼都沒動
//    [5] 看不見（palFT／palRT->Visible=false）⇒ hidden
//    [6] golden 回傳碼（W906_Main_DoFTRTClick，遠端那條路的形狀）：1 運轉中、2／4 [FT Bin=RT Bin] 有料、3／5 起動模式鎖住、4／6 Loader 有料、
//        7 SPIL＋SCK ART；SECS 切換中（iSecsGemSwitchFTRT!=0）不看鎖、做完設 2；bForceRunStartModeEnabled＝遠端先把 Enabled 設 true
//    [7] [FT & RT 鈕可以按]、SECS、Murata 都沒開 ⇒ golden 回 0 但不改模式（OnChange 沒叫、todo 說明）
//    [8] JCET：小方塊不跟鎖（golden :13031）、REALLY＋有料 ⇒ 回 4＋「Please finish ONE CYCLE」；等級不夠 ⇒ 回 8＋等級訊息
//    [9] 接上了沒（原始碼棘輪，argv[1]＝移植樹根目錄、argv[2]＝web\page，唯讀）
//  開跑前後比對 D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini、D:\HT9045\system\lastdata.dat 的內容，有變就失敗。
// =============================================================================
#include "forms/fMain.h"
#include "forms/fLotInfo.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "MachineType.h"
#include "Motor/mymotor.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

std::string W906_Main_EvB6Op(const std::string& cmd, const std::string& payloadJson, bool* ok);   // FileRW/MainClick.cpp
int  W906_Main_DoFTRTClick(bool bIsRT, bool bIsMan, bool bForceRunStartModeEnabled);            // FileRW/MainClick.cpp 檔尾
extern void (*W906_FtRt_RunStartModeChangeFn)();                                                  // FileRW/MainClick.cpp 檔尾（測試縫）
extern bool W906_bRTContinueNeedClearCT;                                                          // FileRW/MainClick.cpp 檔尾
extern bool authMainForm[12];                                                                     // cAuthority.h:56
extern int  W906_ShowMyMessage_Count;                                                             // canary_support.h:168
extern AnsiString W906_ShowMyMessage_LastS1;                                                      // canary_support.h:167
void Su7SetIC(int which, bool on);                                                                // tests/test_b8_su7_ic.cpp

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}

// golden cbRunStartModeChange(this) 的記錄器（見檔頭）
int g_rsmCalls = 0;
std::string g_rsmText;
int g_rsmIndex = -99;
void RecRsm()
{
    ++g_rsmCalls;
    g_rsmText = fMain->cbRunStartMode->Text.c_str();
    g_rsmIndex = fMain->cbRunStartMode->ItemIndex;
}

struct Ack {
    bool ok = false;
    std::string raw;
    cJSON* j = nullptr;
    ~Ack() { if (j) cJSON_Delete(j); }
    std::string Str(const char* k) const { const cJSON* v = j ? cJSON_GetObjectItemCaseSensitive(j, k) : nullptr; return v && cJSON_IsString(v) ? v->valuestring : std::string(); }
    int Int(const char* k) const { const cJSON* v = j ? cJSON_GetObjectItemCaseSensitive(j, k) : nullptr; return v && cJSON_IsNumber(v) ? v->valueint : -999; }
    int Bool(const char* k) const { const cJSON* v = j ? cJSON_GetObjectItemCaseSensitive(j, k) : nullptr; return !v ? -1 : (cJSON_IsTrue(v) ? 1 : 0); }
    bool TodoHas(const char* s) const
    {
        const cJSON* t = j ? cJSON_GetObjectItemCaseSensitive(j, "todo") : nullptr;
        char* p = t ? cJSON_PrintUnformatted(t) : nullptr;
        const bool r = p && std::strstr(p, s);
        if (p) cJSON_free(p);
        return r;
    }
};
void Op(const std::string& payload, Ack* a)
{
    a->raw = W906_Main_EvB6Op("act.main.ftrt", payload, &a->ok);
    if (a->j) cJSON_Delete(a->j);
    a->j = cJSON_Parse(a->raw.c_str());
}
void Click(const char* tile, Ack* a) { Op(std::string("{\"op\":\"click\",\"tile\":\"") + tile + "\"}", a); }

// 基準狀態：停機、機台空、Dummy、等級夠、權限檔允許、[FT & RT 鈕可以按]、兩個小方塊看得見
void Base(const char* startMode)
{
    SystemStart = false; SoftStart = false;
    for (int i = 0; i < 6; ++i) Su7SetIC(i, false);
    MOT[MMTrayY].fHasTray = false; MOT[MMTrayY_Car].fHasTray = false;
    LastSet.iRealDummy = DUMMY;
    authMainForm[9] = true;
    AccessLevel = 0; LevelSet.AccessLevel[12] = 0;
    CUSTOMER_CODE = 0;
    IniConfig.bShowFTandRTButtonCanClick = true; IniConfig.bEnable_SECS_GEM = false; IniConfig.bFTBin2RTBin = false;
    IniConfig.bSPILFunction = false; IniConfig.bVTESTFunction = false; IniConfig.bO21FTAfterTrayEndClearFailBinCount = false;
    IniConfig.bI50_EnableAutoSiteMappingTrigger = false; IniConfig.bUseAutoSiteMapping = false; IniConfig.bNewResetFunction = false;
    bCanRunSCKART = false; iSecsGemSwitchFTRT = 0; bART_RT2RunNoChangeMode = false; bRunAutoClean = false; fAllMotorHome = false;
    RunInfo.bLotStart = false;
    fMain->palFT->Visible = true; fMain->palRT->Visible = true;
    fMain->cbRunStartMode->Text = startMode;
    fMain->cbRunStartMode->ItemIndex = -1;
    fLotInfo->cbRunMode->Text = "";
    W906_bRTContinueNeedClearCT = false;
    g_rsmCalls = 0; g_rsmText.clear(); g_rsmIndex = -99;
}

bool CodeHas(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    bool gated = false;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; continue; }
        const std::string::size_type p = l.find("//");
        if ((p == std::string::npos ? l : l.substr(0, p)).find(needle) != std::string::npos) return true;
    }
    return false;
}

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_b8_m11_ftrt -- B8 M-11 golden palFTClick / palRTClick -> DoFTRTClick -> FTClick / RTClick (V912 main.cpp)\n");
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini", "D:\\HT9045\\system\\lastdata.dat"};
    std::string before[3];
    bool had[3];
    for (int i = 0; i < 3; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    W906_FtRt_RunStartModeChangeFn = &RecRsm;
    Ack a;

    std::printf("[1] get\n");
    Base("Continuous Start");
    Op("{\"op\":\"get\"}", &a);
    Check(a.ok && a.Bool("runStartModeEnabled") == 1 && a.Bool("ftVisible") == 1 && a.Bool("rtVisible") == 1 && a.Bool("ftEnabled") == 1 &&
          a.Bool("rtEnabled") == 1 && a.Str("caption") == "Continuous Start",
          "[1] empty machine, level ok, authMainForm[9] -> start mode changeable, FT / RT visible and enabled (" + a.raw.substr(0, 160) + ")");

    std::printf("[2] FT\n");
    Base("Re-Test Continuous");
    Click("FT", &a);
    Check(a.ok && a.Bool("executed") == 1 && a.Int("result") == 0 && g_rsmCalls == 1 && g_rsmText == "Initial Start" && g_rsmIndex == rsmInitialStart &&
          std::string(fLotInfo->cbRunMode->Text.c_str()) == "Normal" && a.Str("captionBefore") == "Re-Test Continuous",
          "[2] Re-Test Continuous -> Initial Start (machine empty, golden :30796-30797), OnChange once, ItemIndex follows, cbRunMode \"Normal\"");
    Base("Continuous Start");
    Click("FT", &a);
    Check(a.ok && a.Int("result") == 0 && g_rsmCalls == 1 && g_rsmText == "Continuous Start", "[2] Continuous Start stays Continuous Start (golden :30788-30791), OnChange once");
    Base("Auto Site Mapping");
    fMain->cbRunStartMode->Text = StartModeName[rsmAutoSiteMap];
    Click("FT", &a);
    Check(a.ok && g_rsmCalls == 1 && g_rsmText == std::string(StartModeName[rsmAutoSiteMap].c_str()), "[2] Auto Site Mapping stays (golden :30784-30787)");
    Base("Re-Test Initial Start");
    IniConfig.bVTESTFunction = true;
    Click("FT", &a);
    Check(a.ok && std::string(fLotInfo->cbRunMode->Text.c_str()) == "FT1", "[2] VTEST: cbRunMode \"FT1\" (golden :30801-30804)");

    std::printf("[3] RT\n");
    Base("Continuous Start");
    Click("RT", &a);
    Check(a.ok && a.Int("result") == 0 && g_rsmCalls == 1 && g_rsmText == "Re-Test Continuous" && std::string(fLotInfo->cbRunMode->Text.c_str()) == "RT" &&
          W906_bRTContinueNeedClearCT && a.TodoHas("bRTContinueNeedClearCT"),
          "[3] Continuous Start -> Re-Test Continuous, cbRunMode \"RT\", bRTContinueNeedClearCT set (no reader in the port: todo)");
    Base("Initial Start");
    Click("RT", &a);
    Check(a.ok && g_rsmCalls == 1 && g_rsmText == "Re-Test Initial Start" && !W906_bRTContinueNeedClearCT,
          "[3] Initial Start -> Re-Test Initial Start (machine empty, golden :30958-30961)");
    Base("Initial Start");
    IniConfig.bI50_EnableAutoSiteMappingTrigger = true; IniConfig.bI50_RT = false;
    Click("RT", &a);
    Check(a.ok && g_rsmCalls == 1 && g_rsmText == "Re-Test Initial Start", "[3] [I50] ASM trigger branch (golden :30929-30944): same result for an empty machine");

    std::printf("[4] locked by golden ChangeLevelAttr (tile disabled)\n");
    {
        int ok = 0;
        const char* const kName[6] = {"Plate1", "Plate2", "Shuttle", "Index", "InArm", "OutArm"};
        for (int w = 0; w < 6; ++w) {
            Base("Continuous Start");
            Su7SetIC(w, true);
            Click("FT", &a);
            if (!a.ok && a.Str("guard") == "disabled" && g_rsmCalls == 0) ++ok;
            else std::printf("    %s: ok=%d %s\n", kName[w], (int)a.ok, a.raw.substr(0, 200).c_str());
        }
        Check(ok == 6, "[4] IC on Plate1 / Plate2 / Shuttle / Index / In Arm / Out Arm -> FT tile disabled (CheckCanChangeRealDummy / HasICUnderMachine), nothing run");
        Base("Continuous Start");
        Su7SetIC(2, true);
        Op("{\"op\":\"get\"}", &a);
        Check(a.ok && a.Bool("runStartModeEnabled") == 0 && a.Bool("ftEnabled") == 0 && a.Str("runStartModeLock").find(":13023") != std::string::npos,
              "[4] get says why: " + a.Str("runStartModeLock"));
    }
    Base("Continuous Start");
    LastSet.iRealDummy = REALLY; MOT[MMTrayY].fHasTray = true;
    Click("RT", &a);
    Check(!a.ok && a.Str("guard") == "disabled" && g_rsmCalls == 0, "[4] loader tray in the machine -> RT tile disabled");
    Base("Continuous Start");
    LevelSet.AccessLevel[12] = 5; AccessLevel = 1;
    Click("FT", &a);
    Check(!a.ok && a.Str("guard") == "disabled" && a.Str("detail").find("AccessLevel[12]") != std::string::npos, "[4] AccessLevel below LevelSet.AccessLevel[12] -> disabled");
    Base("Continuous Start");
    authMainForm[9] = false;
    Click("FT", &a);
    Check(!a.ok && a.Str("guard") == "disabled" && a.Str("detail").find("authMainForm[9]") != std::string::npos, "[4] authMainForm[9] off -> disabled");
    Base("Continuous Start");
    SystemStart = true;
    Op("{\"op\":\"get\"}", &a);
    Check(a.ok && a.Bool("runStartModeEnabled") == 0 && a.Str("runStartModeLock").find("SystemStart") != std::string::npos && a.Bool("ftEnabled") == 1,
          "[4] SystemStart: the start mode is locked (:12942) but golden does not touch palFT->Enabled while running (:13036 is in the stopped arm)");
    Click("FT", &a);
    Check(a.ok && a.Int("result") == 1 && a.Bool("executed") == 0 && a.Str("guard").find("SystemStart") != std::string::npos && g_rsmCalls == 0,
          "[4] SystemStart, empty machine: the click reaches FTClick, which returns 1 (golden :30712; DoFTRTClick logs \"FTClick fail! (1)\")");
    Su7SetIC(2, true);
    Click("RT", &a);
    Check(!a.ok && a.Str("guard") == "disabled" && g_rsmCalls == 0, "[4] SystemStart with IC on the shuttle: the tile is disabled (last stopped-arm value)");
    Base("Continuous Start");
    IniConfig.bNewResetFunction = true; bResetIsPressed = true; Su7SetIC(3, true);
    Op("{\"op\":\"get\"}", &a);
    Check(a.ok && a.Bool("runStartModeEnabled") == 1, "[4] [New Reset] pressed: golden skips the IC locks (:13001-13003)");
    bResetIsPressed = false;

    std::printf("[5] hidden\n");
    Base("Continuous Start");
    fMain->palRT->Visible = false;
    Click("RT", &a);
    Check(!a.ok && a.Str("guard") == "hidden" && g_rsmCalls == 0, "[5] palRT->Visible false -> hidden, nothing run");
    Click("FT", &a);
    Check(a.ok && a.Int("result") == 0, "[5] palFT still clickable");

    std::printf("[6] golden return codes (W906_Main_DoFTRTClick, the remote path's shape)\n");
    Base("Continuous Start");
    SystemStart = true;
    Check(W906_Main_DoFTRTClick(false, false, true) == 1 && W906_Main_DoFTRTClick(true, false, true) == 1, "[6] SystemStart -> FT 1 / RT 1");
    Base("Continuous Start");
    IniConfig.bFTBin2RTBin = true; Su7SetIC(4, true);
    Check(W906_Main_DoFTRTClick(false, false, true) == 2 && W906_Main_DoFTRTClick(true, false, true) == 4 && g_rsmCalls == 0,
          "[6] [FT Bin = RT Bin] + IC on In Arm -> FT 2 / RT 4");
    Base("Continuous Start");
    Su7SetIC(1, true);
    Check(W906_Main_DoFTRTClick(false, false, false) == 3 && W906_Main_DoFTRTClick(true, false, false) == 5 && g_rsmCalls == 0,
          "[6] start mode locked (IC on Plate2, not forced) -> FT 3 / RT 5");
    Check(W906_Main_DoFTRTClick(false, false, true) == 0 && g_rsmCalls == 1, "[6] forced Enabled (golden remote path sets cbRunStartMode->Enabled=true first) -> FT runs");
    Base("Continuous Start");
    LastSet.iRealDummy = REALLY; MOT[MMTrayY].fHasTray = true; MOT[MMTrayY].Tray.XItem = 1; MOT[MMTrayY].Tray.YItem = 1; MOT[MMTrayY].Tray.Data[0][0] = 1;
    Check(W906_Main_DoFTRTClick(false, false, true) == 4 && W906_Main_DoFTRTClick(true, false, true) == 6 && g_rsmCalls == 0,
          "[6] REALLY + loader tray has IC -> FT 4 / RT 6");
    MOT[MMTrayY].Tray.Data[0][0] = 0; MOT[MMTrayY].fHasTray = false;
    Base("Continuous Start");
    MOT[MMTrayY_Car].fHasTray = true; LastSet.iRealDummy = REALLY;
    Check(W906_Main_DoFTRTClick(false, false, true) == 4, "[6] REALLY + a tray on the tray car -> FT 4");
    Base("Continuous Start");
    IniConfig.bSPILFunction = true; bCanRunSCKART = true;
    Check(W906_Main_DoFTRTClick(true, false, true) == 7, "[6] SPIL + SCK ART -> RT 7");
    IniConfig.bSPILFunction = false; bCanRunSCKART = false;
    Base("Continuous Start");
    Su7SetIC(0, true); iSecsGemSwitchFTRT = 1;
    Check(W906_Main_DoFTRTClick(false, false, false) == 0 && iSecsGemSwitchFTRT == 2 && g_rsmCalls == 1,
          "[6] SECS/GEM switch pending (iSecsGemSwitchFTRT=1): the start-mode lock is not checked, runs, sets 2 (golden :30765 / :30825)");

    std::printf("[7] none of [FT & RT button can click] / SECS / Murata\n");
    Base("Re-Test Continuous");
    IniConfig.bShowFTandRTButtonCanClick = false;
    Click("FT", &a);
    Check(a.ok && a.Int("result") == 0 && g_rsmCalls == 0 && a.TodoHas("[FT & RT button can click]") && a.Str("caption") == "Re-Test Continuous",
          "[7] golden returns 0 without touching the start mode; todo explains");
    Base("Re-Test Continuous");
    IniConfig.bShowFTandRTButtonCanClick = false; IniConfig.bEnable_SECS_GEM = true;
    Click("FT", &a);
    Check(a.ok && g_rsmCalls == 1 && g_rsmText == "Initial Start", "[7] SECS/GEM on -> the FT arm runs (golden :30749)");
    Base("Re-Test Continuous");
    IniConfig.bShowFTandRTButtonCanClick = false; CUSTOMER_CODE = CC_Murata;
    Click("RT", &a);
    Check(a.ok && g_rsmCalls == 1, "[7] Murata -> the RT arm runs (golden :30895)");

    std::printf("[8] JCET\n");
    Base("Continuous Start");
    CUSTOMER_CODE = CC_JCET; LastSet.iRealDummy = REALLY; Su7SetIC(2, true);
    {
        const int m0 = W906_ShowMyMessage_Count;
        Click("FT", &a);
        Check(a.ok && a.Int("result") == 4 && W906_ShowMyMessage_Count == m0 + 1 && a.Str("message").find("ONE CYCLE") != std::string::npos && g_rsmCalls == 0,
              "[8] JCET tile is not locked (golden :13031); REALLY + IC -> FT 4 + \"Please finish ONE CYCLE before Change Mode!\"");
    }
    Base("Continuous Start");
    CUSTOMER_CODE = CC_JCET; LevelSet.AccessLevel[12] = 5; AccessLevel = 1; fMain->cbUserSelect->Text = "Operator";
    Click("RT", &a);
    Check(a.ok && a.Int("result") == 8 && a.Str("message").find("Current level \"Operator\"") != std::string::npos,
          "[8] JCET level below [12] -> RT 8 + \"Current level ...\" (spbUserName -> cbUserSelect text)");
    CUSTOMER_CODE = 0;

    std::printf("[9] wiring ratchet (source, read-only)\n");
    if (argc < 3) {
        Check(false, "[9] argv[1] (port tree root) / argv[2] (web page dir) missing");
    } else {
        std::string mc, wb, js;
        const bool rd = ReadAll(std::string(argv[1]) + "/FileRW/MainClick.cpp", &mc) && ReadAll(std::string(argv[1]) + "/tools/wb_serve.cpp", &wb) &&
                        ReadAll(std::string(argv[2]) + "/ht9045_main_st01_ev.js", &js);
        Check(rd, "[9] read FileRW/MainClick.cpp, tools/wb_serve.cpp, ht9045_main_st01_ev.js");
        Check(CodeHas(mc, "else if (cmd == \"act.main.ftrt\") { std::string W906_Main_FtRtOp(const std::string&, bool*); return W906_Main_FtRtOp(payloadJson, ok); }"),
              "[9] MainClick.cpp: act.main.ftrt dispatched in W906_Main_EvB6Op (code, before the trailing //)");
        Check(CodeHas(mc, "fLotInfo->sbSECSLotEndClick(fLotInfo->sbSECSLotEnd);") && CodeHas(mc, "fYieldMonitoring->ClearYieldCount();") &&
              CodeHas(mc, "fMain->Clarn_Data(4, \"SPIL_FTClick\");") && CodeHas(mc, "fMain->Clarn_Data(3, \"SPIL_RTClick\");") &&
              CodeHas(mc, "fCounterClear->ClearCount(ctFailBinCount);") && CodeHas(mc, "EventReport(SECS_EVENT.TesterFT);") &&
              CodeHas(mc, "EventReport(SECS_EVENT.TesterRT);") && CodeHas(mc, "SetRunStartMode(rsmCInitialRetest);") &&
              CodeHas(mc, "void (*W906_FtRt_RunStartModeChangeFn)() = &W906_CbRunStartModeChange;"),
              "[9] the golden tails not run here are live code (2DID Lot End, SPIL Yield / Clarn_Data, [O21] ClearCount, XinYun SECS, JCET ASM, real OnChange)");
        Check(wb.find("|| wc.cmd == \"act.main.cleanOut\" || wc.cmd == \"act.main.ftrt\") {") != std::string::npos,
              "[9] wb_serve.cpp: act.main.ftrt joins St01's act.main.* arm");
        Check(js.find("wireIcon(t[0], 'act.main.ftrt', t[1], { op: 'click', tile: t[1] }") != std::string::npos &&
              js.find("[['palFT', 'FT'], ['palRT', 'RT']]") != std::string::npos,
              "[9] page: palFT / palRT send act.main.ftrt click");
    }

    W906_FtRt_RunStartModeChangeFn = nullptr;
    for (int i = 0; i < 6; ++i) Su7SetIC(i, false);
    for (int i = 0; i < 3; ++i) {
        std::string after;
        const bool has = ReadAll(kGuard[i], &after);
        Check(has == had[i] && after == before[i], std::string("[guard] unchanged: ") + kGuard[i]);
    }
    std::printf("test_b8_m11_ftrt: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
