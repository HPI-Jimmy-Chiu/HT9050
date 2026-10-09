// =============================================================================
//  test_pool2_lotinfo2.cpp  --  AI(W906-POOL2) 20261008 (Ifor01)
//
//  W-168（TO_IFOR §3 1008 06:3x）：forms/fLotInfo.cpp 第二輪——10/06 留的 8 個 `#if 0` 照 golden 0618 開
//  （RULINGS_20261001 第 0 條蓋過舊的「安全紅線」「S25 客戶專屬」「模式切換＝Jimmy」標籤）：
//    WC-19 FormShow→SetLotStart（golden uLotInfo.cpp:1158）、LOT-W1-MAINPANELS 開批鎖主畫面五個控制（:1816-1820）、
//    S117-ATKAMR 不重複送 DoLotEnd（:2020）、S25-TSMC（:2112-2113）、S25-UTAC（:2215-2225）、
//    S117-RSM ×2 Clean Out 拒絕前同步模式（:1363／:1371）、S117-MAINPANELS 結批解鎖（:1393-1397）。
//  執行的部分改寫自 Ifor-GPT 的 IG-1 盲做測試（v906/iforgpt-ig1-lotinfo d6cf8424，tests/test_ig1_lotinfo.cpp；
//  筆電 1008 06:3x 說可以搬）：T2、T4～T9 的情境與期望值照它；原始碼檢查改成用閘名找（不寫死行號）。
//    [S] 原始碼（argv[1] = 移植樹根目錄）：8 個 W-168 開閘註記各一、緊接 golden 那一行；6 處「沒跑」回報已作廢；
//        ATKAMR 另外兩個（0618 之後的 sTrackOutType_ATK、缺成員的旗標重設）仍是 `#if 0`
//    T2 FormShow 呼叫一次 SetLotStart（讀檔版）；T9 結批解鎖五個控制
//    T4 CYUEAN 開批鎖住五個控制、其他客戶不鎖
//    T5 已提前送過 DoLotEnd ⇒ 不重送；沒送過 ⇒ 送一次；SECS 關 ⇒ 不送
//    T6 TSMC 結批時 sbSECSLotStart->Down=true（在 SetLotComponents 之前看得到）、不再列為「沒跑」
//    T7 UTAC FT 結批送狀態 8、RT 送 10、其他模式不送
//    T8（第二支 ctest，argv[2]="interlocks"）Clean Out 兩個拒絕點會先把模式同步回去、照舊拒絕；運轉中直接返回
//    T10 真實的 Gerneral.ini／config.ini 內容不變
//  反向驗證見 MR 說明：任一個改回 `#if 0` 重編 ⇒ 對應的 T 紅＋[S] 紅。
// =============================================================================
#include "forms/fLotInfo.h"
#include "forms/fMain.h"
#include "forms/fAGV.h"
#include "forms/fSCKART.h"
#include "aHotPlateSubstrate.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
extern bool bLotFirstKeyIn;                     // forms/fLotInfo.cpp
extern int iTestHeadMotorTask;                  // atester.cpp (same declaration as forms/fLotInfo.cpp)
#include "common.h"
#include "LogObjects.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"
#include "TesterComm/TesterWndSeat.h"
#include "MessageDef.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "forms/fTemp_Set.h"   // AI(W906-N1G5) 20261008 (Ifor01): fTemp_Set (see main)

namespace {
int failures = 0;
void check(bool ok, const char* label)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", label);
    if (!ok)
        ++failures;
}
std::string read(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    std::ostringstream out;
    out << file.rdbuf();
    return out.str();
}
bool skipped(const char* needle)
{
    for (int i=0; i<fLotInfo->W906_LotEndSkipped->Count; ++i)
        if (std::string(fLotInfo->W906_LotEndSkipped->Strings[i].c_str()).find(needle) != std::string::npos)
            return true;
    return false;
}
class ProbeLot : public TfLotInfo           // records the calls FormShow / sbSECSLotEndClick make
{
public:
    int starts = 0;
    int ends = 0;
    bool fromFile = false;
    AnsiString caller;
    void SetLotStart(AnsiString func, bool from) override { ++starts; fromFile = from; caller = func; }
    void SetLotEnd(AnsiString) override { ++ends; }
};
class TsmcLot : public TfLotInfo            // sees sbSECSLotStart->Down at the moment golden's later SetLotComponents runs
{
public:
    bool downAtComponents = false;
    void SetLotComponents(bool end) override
    {
        downAtComponents = sbSECSLotStart->Down;
        TfLotInfo::SetLotComponents(end);
    }
};
void enabled(bool value)
{
    fMain->cbRunStartMode->Enabled = value;
    fMain->palFT->Enabled = value;
    fMain->palRT->Enabled = value;
    fMain->palOffLine->Enabled = value;
    fMain->palEQC->Enabled = value;
}
bool allEnabled(bool value)
{
    return fMain->cbRunStartMode->Enabled == value && fMain->palFT->Enabled == value &&
           fMain->palRT->Enabled == value && fMain->palOffLine->Enabled == value &&
           fMain->palEQC->Enabled == value;
}
bool bridgeFound() { return true; }
HWND bridgeWindow() { return reinterpret_cast<HWND>(1); }
int packets = 0;
int lotStatus = -1;
void sendPacket(COPYDATASTRUCT* packet)
{
    const MV* mv = reinterpret_cast<const MV*>(packet->lpData);
    if (static_cast<unsigned>(mv->iSendCommand) == MSG_CMD_LotStatus)
    {
        ++packets;
        lotStatus = mv->iLotStatus;
    }
}
std::string trim(const std::string& l)
{
    size_t a = l.find_first_not_of(" \t\r");
    size_t b = l.find_last_not_of(" \t\r");
    return a == std::string::npos ? std::string() : l.substr(a, b - a + 1);
}
void sources(const std::string& root)
{
    const char* control = std::getenv("W906_POOL2_LOTINFO2_SOURCE");   // reverse checks may point at an edited copy
    const std::string text = read(control ? control : root + "/forms/fLotInfo.cpp");
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line))
        lines.push_back(line);
    struct Gate { const char* tag; const char* golden; };
    static const Gate gates[] = {
        { "W-168 WC-19 ",             "SetLotStart(\"fLotInfo::FormShow\", true);" },
        { "W-168 LOT-W1-MAINPANELS ", "fMain->cbRunStartMode->Enabled=false;" },
        { "W-168 S117-ATKAMR ",       "if(fAGV->bATK_AMR_DoLotEndSent==false)" },
        { "W-168 S25-TSMC ",          "if(CUSTOMER_CODE==CC_TSMC_TAINAN)" },
        { "W-168 S25-UTAC ",          "if(CUSTOMER_CODE==CC_UTAC)" },
        { "W-168 S117-MAINPANELS ",   "fMain->cbRunStartMode->Enabled=true;" },
    };
    for (const Gate& g : gates)
    {
        int hits = 0, good = 0;
        for (size_t i=0; i+1<lines.size(); ++i)
            if (lines[i].rfind("#if 1 // was: #if 0 ", 0) == 0 && lines[i].find(g.tag) != std::string::npos)
            {
                ++hits;
                if (trim(lines[i+1]).compare(0, std::string(g.golden).size(), g.golden) == 0) ++good;
            }
        char label[160];
        std::snprintf(label, sizeof(label), "S1 %sopened in place, golden statement under it (%d / %d)", g.tag, hits, good);
        check(hits == 1 && good == 1, label);
    }
    int rsm = 0;
    for (size_t i=0; i+1<lines.size(); ++i)
        if (lines[i].find("W-168 S117-RSM ") != std::string::npos && lines[i].rfind("#if 1 // was: #if 0 ", 0) == 0 &&
            trim(lines[i+1]) == "SetRunStartMode((eRunStartMode)LastSet.iRunStartMode);")
            ++rsm;
    check(rsm == 2, "S1 both S117-RSM opened in place, SetRunStartMode under each");
    int retired = 0;
    for (const std::string& l : lines)
        if (trim(l).rfind("//AI(W906-POOL2) 20261008 (Ifor01): W-168 retired, the golden", 0) == 0) ++retired;
    check(retired == 7, "S2 the 'not run' reports of the opened statements are retired (TSMC 2 + UTAC 2 + RSM 2 + MAINPANELS 1 lines)");
    int atk = 0;
    for (const std::string& l : lines)
        if (trim(l) == "#if 0 // GATE (W906-PROD-S117-ATKAMR)") ++atk;
    check(atk == 2, "S3 the two other S117-ATKAMR gates stay (post-0618 sTrackOutType_ATK; flag resets need members TfAGV lacks)");
}
}

static void interlocks()
{
    CUSTOMER_CODE = CC_PTI;
    std::puts("BEGIN Clean Out interlocks");
    CosFunction.bAutoCleanShuttleDisable = false;
    SystemStart = false;
    LastSet.iRunStartMode = rsmContinuStart;
    InArmSuck.iMaxRow = 1;
    InArmSuck.iMaxCol = 1;
    InArmSuck.Item[0][0] = HAS_IC;
    check(!fMain->CheckCanChangeRealDummy(), "T8 first clean-out guard precondition sees real IC");
    OpenGeneralIniFile();                       // the real mode switch writes the (scratch) General ini; open it like the SCK_ART / ASM tests do
    fMain->cbRunStartMode->Text = "stale";
    fLotInfo->sbSECSLotEndClick(fLotInfo->sbSECSLotEnd);
    check(fLotInfo->W906_LotEndResult=="must-clean-out-1" &&
          fMain->cbRunStartMode->Text==StartModeName[rsmContinuStart] && !skipped("SetRunStartMode"),
          "T8 first clean-out refusal resynchronizes the run mode (golden :1363) and still rejects");
    InArmSuck.Item[0][0] = NULL_IC;
    check(fMain->CheckCanChangeRealDummy(), "T8 second guard precondition has an empty machine");
    iTestHeadMotorTask = 2;
    fAllMotorHome = true;
    OpenGeneralIniFile();
    fMain->cbRunStartMode->Text = "stale";
    fLotInfo->sbSECSLotEndClick(fLotInfo->sbSECSLotEnd);
    check(fLotInfo->W906_LotEndResult=="must-clean-out-2" &&
          fMain->cbRunStartMode->Text==StartModeName[rsmContinuStart] && !skipped("SetRunStartMode"),
          "T8 second clean-out refusal resynchronizes the run mode (golden :1371) and still rejects");
    SystemStart = true;
    fLotInfo->sbSECSLotEnd->Tag = 0;
    fMain->cbRunStartMode->Text = "untouched";
    fLotInfo->sbSECSLotEndClick(fLotInfo->sbSECSLotEnd);
    check(fLotInfo->W906_LotEndResult=="system-running" && fMain->cbRunStartMode->Text=="untouched",
          "T8 system-running interlock still returns before the mode synchronization");
}

int main(int argc, char** argv)
{
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // AI(W906-N1G5) 20261008 (Ifor01): golden boot creates TfTemp_Set (CreateForm HT9045.cpp:186) before anything reaches ChangeSite, which calls fTemp_Set->InitialAddrToATC() since N1-G5
    std::setvbuf(stdout, 0, _IONBF, 0);
    const char* const paths[] = {"AuthPath", AuthPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), 0};
    if (!W906TestRequireCtestRedirects("POOL2_LotInfo2", paths))
        return 2;
    if (argc != 2 && (argc != 3 || std::string(argv[2]) != "interlocks"))
        return 2;
    sources(argv[1]);
    if (failures)
        return 1;                               // a failed source check stops before any real method I/O
    const std::string realGeneral = read("D:/HT9045/system/Gerneral.ini");
    const std::string realConfig = read("D:/HT9045/config/config.ini");
    CosFunction.bOEEFunction = false;
    CosFunction.bSortingBy2DList = false;
    IniConfig.bB03_TesterReport = false;
    IniConfig.bVTESTFunction = false;
    IniConfig.bN22Enable_EventLog = false;
    CosFunction.bSpecailLowYeild = false;
    TestIF_File.bEnableBarCode = false;
    W906_CreateLogObjects();
    check(slEventLog != 0, "T0 boot log objects constructed");
    if (argc == 3)
    {
        interlocks();
        check(read("D:/HT9045/system/Gerneral.ini")==realGeneral && read("D:/HT9045/config/config.ini")==realConfig,
              "T10 real Gerneral.ini and config.ini contents unchanged");
        std::printf("POOL2_LotInfo2Interlocks: %d failure(s)\n", failures);
        return failures ? 1 : 0;
    }

    std::puts("BEGIN FormShow and Lot End unlock");
    TfLotInfo* originalLot = fLotInfo;
    ProbeLot probe;
    fLotInfo = &probe;
    CUSTOMER_CODE = CC_PTI;
    probe.FormShow();
    check(probe.starts==1 && probe.fromFile && probe.caller=="fLotInfo::FormShow",
          "T2 FormShow reaches SetLotStart exactly once, read-from-file true (golden :1158)");
    SystemStart = false;
    CosFunction.bAutoCleanShuttleDisable = true;
    enabled(false);
    probe.sbSECSLotEndClick(probe.sbSECSLotEnd);
    check(probe.ends==1 && allEnabled(true), "T9 an accepted Lot End unlocks all five mode controls (golden :1393-1397)");
    fLotInfo = originalLot;

    std::puts("BEGIN Lot Start lock");
    IniConfig.bEnable_SECS_GEM = false;
    CUSTOMER_CODE = CC_CYUEAN;
    enabled(true);
    fLotInfo->SetLotStart("W-168", true);
    check(allEnabled(false), "T4 CYUEAN Lot Start locks all five mode controls (golden :1816-1820)");
    CUSTOMER_CODE = CC_PTI;
    enabled(true);
    fLotInfo->SetLotStart("W-168", true);
    check(allEnabled(true), "T4 a non-lock customer keeps the controls enabled");

    std::puts("BEGIN Lot End: ATK AMR guard, TSMC");
    TsmcLot tsmcLot;
    fLotInfo = &tsmcLot;
    CUSTOMER_CODE = CC_TSMC_TAINAN;
    IniConfig.bEnable_SECS_GEM = true;
    RunInfo.bLotStart = true;
    fAGV->bATK_AMR_DoLotEndSent = true;
    ResetSimEventReport();
    fLotInfo->SetLotEnd("W-168");
    check(g_SimEventReportCount==0, "T5 an AMR Lot End already sent is not sent again (golden :2020)");
    check(tsmcLot.downAtComponents, "T6 TSMC Down=true is set before golden's later SetLotComponents (golden :2112-2113)");
    check(!skipped("TSMC_TAINAN"), "T6 the TSMC action is no longer reported as skipped");
    check(!fLotInfo->sbSECSLotStart->Down, "T6 golden's later SetLotComponents(true) supersedes Down=true");
    RunInfo.bLotStart = true;
    fAGV->bATK_AMR_DoLotEndSent = false;
    ResetSimEventReport();
    fLotInfo->SetLotEnd("W-168");
    check(g_SimEventReportCount==1 && g_SimLastEventReportCeid==SECS_EVENT.DoLotEnd, "T5 an unsent AMR Lot End emits exactly one DoLotEnd");
    IniConfig.bEnable_SECS_GEM = false;
    RunInfo.bLotStart = true;
    ResetSimEventReport();
    fLotInfo->SetLotEnd("W-168");
    check(g_SimEventReportCount==0, "T5 SECS disabled never sends DoLotEnd");

    std::puts("BEGIN UTAC tester bridge");
    fLotInfo = originalLot;
    W906_TesterForward.BridgeFound = &bridgeFound;
    W906_TesterBridgeWndHook = &bridgeWindow;
    W906_TesterSendToBridgeHook = &sendPacket;
    TestIF_File.iTestType = 0;
    TestIF.iTestType = GPIB_MODE;
    CUSTOMER_CODE = CC_UTAC;
    CosFunction.bAutoRetestGPIBmode = true;
    LastSet.iRunStartMode = rsmContinuStart;
    packets = 0;
    fLotInfo->SetLotEnd("W-168");
    check(packets==1 && lotStatus==8 && !skipped("UTAC"), "T7 UTAC FT Lot End reaches the tester bridge with state 8 (golden :2215-2225)");
    LastSet.iRunStartMode = rsmContinuRetest;
    packets = 0;
    fLotInfo->SetLotEnd("W-168");
    check(packets==1 && lotStatus==10, "T7 UTAC RT Lot End reaches the tester bridge with state 10");
    LastSet.iRunStartMode = rsmInitialStart;
    packets = 0;
    fLotInfo->SetLotEnd("W-168");
    check(packets==0, "T7 UTAC other run mode sends no lot-status packet");
    W906_TesterForward.BridgeFound = 0;
    W906_TesterBridgeWndHook = 0;
    W906_TesterSendToBridgeHook = 0;

    check(read("D:/HT9045/system/Gerneral.ini")==realGeneral && read("D:/HT9045/config/config.ini")==realConfig,
          "T10 real Gerneral.ini and config.ini contents unchanged");
    std::printf("POOL2_LotInfo2: %d failure(s)\n", failures);
    return failures ? 1 : 0;
}
