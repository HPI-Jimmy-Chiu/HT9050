// =============================================================================
//  test_nb2_pool2_yieldtemper.cpp -- AI(W906-POOL2-NB2) 20261008 (NB2-1, README R260)
//
//  POOL-2: uYieldMonitoring.cpp GATE(G-YM-TemperFT / G-YM-TemperRT / G-YM-TemperTail) (:3130 / :3141 / :3149) lifted.
//  golden 0618 uYieldMonitoring.cpp:1332-1361, inside TfYieldMonitoring::ReadFile:
//      if(IniConfig.bShowFunctionWindow) { FT or RT branch: 4x fTemperFrom->SetShowYield(esytDoubleDevice / esytCGoodBin /
//      esytYieldMonitor / esytConsAlarm, ...); SetShowYield(esytTest2, bFTContinueON); strShowYield[esytTest2].bShow=...;
//      ShowYieldFuntion(); }
//  The port adds a NULL guard (fTemperFrom starts NULL, cTemperFrom.cpp:1458; the boot ReadFile at tools/wb_serve.cpp:3251
//  runs before W906_TemperFromBootShow creates the form) and a boot replay (W906_YieldMonitoringShowYieldReplay,
//  uYieldMonitoring.cpp file end, called by W906_TemperFromBootShow) -- the same shape as St02-E's OCR slot
//  (tests/test_st02_s09_setup_temperfrom.cpp).  This test runs the REAL TfYieldMonitoring::ReadFile on a Tester.Data
//  seeded at the path ReadFile itself reads (GetRecipeFileName("Tester.Data") under the redirected DataPath):
//    [1] fTemperFrom==NULL: ReadFile reads the seed and does not crash; the form stays NULL
//    [2] boot order: W906_TemperFromBootShow right after [1] -> the four FT slots + esytTest2 show what [1] read (replay)
//    [3] form alive, FT run mode, the opposite seed -> every one of the four slots flips; bFTContinueON off -> Test2 off/hidden
//    [4] RT run mode (rsmContinuRetest), seed A -> the RT keys win (slots = the "_RT" values, opposite of the FT ones)
//    [5] IniConfig.bShowFunctionWindow==false -> no SetShowYield (sentinels survive)
//    [6] boot order again with seed B on a fresh form (the replay is not a constant)
//    [7] machine files: D:\HT9045\system\Gerneral.ini, D:\HT9045\config\config.ini, D:\HT9045\SetUp.inf byte-identical
//  REVERSE (done when this landed): put the three `#if 0` back -> [3] [4] red; drop the replay call in
//  W906_TemperFromBootShow -> [2] [6] red.
//  CONTAINMENT: refuses (exit 2) unless ctest's redirect roots are in force (st02_test_containment.h, w906_ctest_guard.h) and
//  DataPath / LastDataPath sit in ctest's scratch.  Writes only <DataPath>NB2_POOL2_YIELDTEMPER\Tester.Data and
//  <DataPath>NB2_POOL2_YT_SetUp.inf (deleted when green).  Memory otherwise; no hardware, no IO table.
// =============================================================================
#include "st02_test_containment.h"   // W906TestInsideCtestRoots (includes common.h)
#include "w906_ctest_guard.h"        // W906TestRequireCtestRedirects / W906CtestGuardInScratch
#include "forms/fYieldMonitoring.h"  // fYieldMonitoring, TfYieldMonitoring::ReadFile (uYieldMonitoring.cpp:2676)
#include "forms/fTemperFrom.h"       // fTemperFrom, TfTemperFrom::strShowYield / esyt*
#include "MachineType.h"             // rsmContinuStart / rsmContinuRetest, CC_PTI
#include "cmydef.h"                  // CUSTOMER_CODE, InitialOK, bSystemClose, bUseTwoArm32Site
#include "cprod.h"                   // TestIF_File
#include "LastSet.h"                 // LastSet.iRunStartMode
#include "Config.h"                  // IniConfig
#include "CosFunction.h"             // CosFunction
#include "common.h"                  // DataPath, LastDataPath, GetRecipePath, GetRecipeFileName, MyForceDirectories, CloseIniFile

#include <windows.h>
#include <cstdio>
#include <string>

void W906_TemperFromBootShow();      // cTemperFrom.cpp:1989 (no header; same declaration as tests/test_st02_s09_setup_temperfrom.cpp)

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_nb2_pool2_yieldtemper.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static const char* const kRecipe="NB2_POOL2_YIELDTEMPER";

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static bool WriteText(const std::string& p, const std::string& text)
{
    FILE* f=std::fopen(p.c_str(), "wb");
    if(!f) return false;
    const bool ok=std::fwrite(text.data(), 1, text.size(), f)==text.size();
    return std::fclose(f)==0 && ok;
}

// Seed A: FT -> DoubleDevice / CGoodBin on, YieldMonitor / ConsAlarm off;  RT keys the other way round.
// Seed B: FT -> DoubleDevice / CGoodBin off, YieldMonitor / ConsAlarm on.   (keys: uYieldMonitoring.cpp:2692-3078)
static bool Seed(const std::string& file, bool a)
{
    CloseIniFile();                                                             // the cached INIFile reopens the file as written here
    const char* const sA=
        "[Alarm]\r\nContinuous Pass=1\r\nContinuous Pass By Socket=0\r\nContinuous Loader=0\r\nContinuous Contact=0\r\n"
        "SocketEnable=0\r\nHeadEnable=0\r\n"
        "Continuous Pass RT=0\r\nContinuous Pass RT By Socket=0\r\nContinuous Loader RT=0\r\nContinuous Contact RT=0\r\n"
        "SocketEnable RT=1\r\nHeadEnable RT=0\r\n"
        "[Low Yield Alarm]\r\nEnable=0\r\nEnable RT=1\r\n"
        "[Site Yield Alarm]\r\nSite Yield Compare=0\r\nSite Yield Different=0\r\nSite Yield Compare RT=0\r\nSite Yield Different RT=0\r\n";
    const char* const sB=
        "[Alarm]\r\nContinuous Pass=0\r\nContinuous Pass By Socket=0\r\nContinuous Loader=0\r\nContinuous Contact=0\r\n"
        "SocketEnable=0\r\nHeadEnable=1\r\n"
        "Continuous Pass RT=1\r\nContinuous Pass RT By Socket=0\r\nContinuous Loader RT=0\r\nContinuous Contact RT=0\r\n"
        "SocketEnable RT=0\r\nHeadEnable RT=0\r\n"
        "[Low Yield Alarm]\r\nEnable=1\r\nEnable RT=0\r\n"
        "[Site Yield Alarm]\r\nSite Yield Compare=0\r\nSite Yield Different=0\r\nSite Yield Compare RT=0\r\nSite Yield Different RT=0\r\n";
    return WriteText(file, a ? sA : sB);
}

static bool OnOff(TfTemperFrom::eShowYieldType t) { return fTemperFrom->strShowYield[t].OnOff; }

// The four FT/RT slots as one 4-char string, D C Y A = DoubleDevice CGoodBin YieldMonitor ConsAlarm ('1' on, '0' off).
static std::string Slots()
{
    std::string s;
    s+=OnOff(TfTemperFrom::esytDoubleDevice)?'1':'0';
    s+=OnOff(TfTemperFrom::esytCGoodBin)?'1':'0';
    s+=OnOff(TfTemperFrom::esytYieldMonitor)?'1':'0';
    s+=OnOff(TfTemperFrom::esytConsAlarm)?'1':'0';
    return s;
}

static void SetSlots(bool v)
{
    fTemperFrom->strShowYield[TfTemperFrom::esytDoubleDevice].OnOff=v;
    fTemperFrom->strShowYield[TfTemperFrom::esytCGoodBin].OnOff=v;
    fTemperFrom->strShowYield[TfTemperFrom::esytYieldMonitor].OnOff=v;
    fTemperFrom->strShowYield[TfTemperFrom::esytConsAlarm].OnOff=v;
}

static void CheckSlots(const char* tag, const char* want)
{
    const std::string got=Slots();
    char msg[200];
    std::snprintf(msg, sizeof(msg), "%s slots DoubleDevice/CGoodBin/YieldMonitor/ConsAlarm = %s (want %s)", tag, got.c_str(), want);
    std::printf("  %s\n", msg);
    CHECK(got==want, msg);
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("test_nb2_pool2_yieldtemper (POOL-2): TfYieldMonitoring::ReadFile -> fTemperFrom->SetShowYield (G-YM-Temper*)\n");
    if(!W906TestInsideCtestRoots("NB2_Pool2YieldTemper"))
        return 2;

    // ---- containment: the recipe dir and SetUp.inf live under DataPath, which must be ctest's scratch -----------------
    const std::string sDataPath=DataPath.c_str();
    const std::string sInf=sDataPath+"NB2_POOL2_YT_SetUp.inf";
    LastDataPath=AnsiString(sInf.c_str());                                     // GetLastOpenFN (common.cpp) reads it
    {
        const char* const rt[]={ "DataPath", sDataPath.c_str(), "LastDataPath", sInf.c_str(), 0 };
        if(!W906TestRequireCtestRedirects("NB2_Pool2YieldTemper", rt))
            return 2;
    }

    const std::string sGen0=Slurp("D:\\HT9045\\system\\Gerneral.ini");
    const std::string sCfg0=Slurp("D:\\HT9045\\config\\config.ini");
    const std::string sInf0=Slurp("D:\\HT9045\\SetUp.inf");

    MyForceDirectories(AnsiString(sDataPath.c_str()));
    if(!WriteText(sInf, std::string(kRecipe)+"\r\n"))
    {
        std::printf("  cannot write %s\n", sInf.c_str());
        return 1;
    }
    MyForceDirectories(GetRecipePath());
    const std::string sFile=GetRecipeFileName("Tester.Data").c_str();
    std::printf("  Tester.Data = %s\n", sFile.c_str());
    if(!W906CtestGuardInScratch(sFile.c_str()) || sFile.find(kRecipe)==std::string::npos)
    {
        std::printf("  REFUSED: the recipe file is not in ctest's scratch / not ours\n");
        return 2;
    }

    // ---- the state ReadFile reads besides the file: no customer branch touches these keys for CC_PTI (HT9050) ---------
    InitialOK=false;
    bSystemClose=false;
    CUSTOMER_CODE=CC_PTI;
    bUseTwoArm32Site=false;
    IniConfig.bD56YieldPiggyBackEnable=false;   // :3083 would force the continuous flags on
    CosFunction.bPiggyBackForASE=false;         // :3109 would force them off
    CosFunction.bUseSCKART=false;
    IniConfig.bShowFunctionWindow=true;
    IniConfig.bFTContinueON=true;
    LastSet.iRunStartMode=rsmContinuStart;      // FT branch

    // ---- [1] no form yet ----------------------------------------------------------------------------------------------
    std::printf("[1] fTemperFrom==NULL (before the boot show)\n");
    CHECK(fTemperFrom==NULL, "[1] fTemperFrom is NULL before the boot show (cTemperFrom.cpp:1458)");
    CHECK(Seed(sFile, true), "[1] seed A");
    TestIF_File.bContinuousPass=false;
    TestIF_File.bFailAlarmLowYield_RT=false;
    fYieldMonitoring->ReadFile();
    CHECK(TestIF_File.bContinuousPass==true, "[1] ReadFile read Continuous Pass=1 from the seeded Tester.Data (:2880)");
    CHECK(TestIF_File.bFailAlarmLowYield_RT==true, "[1] ReadFile read Low Yield Enable RT=1 (:2917)");
    CHECK(fTemperFrom==NULL, "[1] ReadFile does not create the form (and did not crash on the guarded calls)");

    // ---- [2] boot order: the replay -------------------------------------------------------------------------------------
    std::printf("[2] after W906_TemperFromBootShow (replay of [1])\n");
    W906_TemperFromBootShow();
    CHECK(fTemperFrom!=NULL, "[2] the boot show creates fTemperFrom");
    if(fTemperFrom==NULL) { std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail); return 1; }
    CheckSlots("[2] boot replay, seed A FT:", "1100");
    CHECK(OnOff(TfTemperFrom::esytTest2)==true, "[2] boot replay: esytTest2 OnOff = bFTContinueON (true)");
    CHECK(fTemperFrom->strShowYield[TfTemperFrom::esytTest2].bShow==true, "[2] boot replay: esytTest2 bShow = bFTContinueON (true)");

    // ---- [3] live form, FT, the opposite seed ---------------------------------------------------------------------------
    std::printf("[3] form alive, FT, seed B, bFTContinueON off\n");
    CHECK(Seed(sFile, false), "[3] seed B");
    IniConfig.bFTContinueON=false;
    fYieldMonitoring->ReadFile();
    CheckSlots("[3] FT seed B:", "0011");
    CHECK(OnOff(TfTemperFrom::esytTest2)==false, "[3] esytTest2 OnOff = bFTContinueON (false)");
    CHECK(fTemperFrom->strShowYield[TfTemperFrom::esytTest2].bShow==false, "[3] esytTest2 bShow = bFTContinueON (false)");
    IniConfig.bFTContinueON=true;

    // ---- [4] RT branch: the _RT keys ------------------------------------------------------------------------------------
    std::printf("[4] RT run mode, seed A\n");
    CHECK(Seed(sFile, true), "[4] seed A");
    fYieldMonitoring->ReadFile();
    CheckSlots("[4a] FT seed A:", "1100");
    LastSet.iRunStartMode=rsmContinuRetest;     // not in the FT list (golden :1334-1340) -> RT branch
    fYieldMonitoring->ReadFile();
    CheckSlots("[4b] RT seed A (RT keys):", "0011");
    LastSet.iRunStartMode=rsmContinuStart;

    // ---- [5] function window off: no calls ------------------------------------------------------------------------------
    std::printf("[5] bShowFunctionWindow==false\n");
    IniConfig.bShowFunctionWindow=false;
    SetSlots(false);
    fYieldMonitoring->ReadFile();               // seed A, FT: would set 1100 if it ran
    CheckSlots("[5] window off, sentinels kept:", "0000");
    IniConfig.bShowFunctionWindow=true;

    // ---- [6] boot order again, seed B, fresh form -----------------------------------------------------------------------
    std::printf("[6] boot order: ReadFile with fTemperFrom NULL (seed B), then the boot show\n");
    TfTemperFrom* const fFirst=fTemperFrom;                                    // left alive: nothing drives it any more
    fTemperFrom=NULL;
    CHECK(Seed(sFile, false), "[6] seed B");
    fYieldMonitoring->ReadFile();
    CHECK(fTemperFrom==NULL, "[6] still NULL after ReadFile");
    W906_TemperFromBootShow();
    CHECK(fTemperFrom!=NULL && fTemperFrom!=fFirst, "[6] the boot show built a fresh form");
    if(fTemperFrom!=NULL && fTemperFrom!=fFirst)
        CheckSlots("[6] boot replay, seed B FT:", "0011");

    // ---- [7] machine files --------------------------------------------------------------------------------------------
    CloseIniFile();
    CHECK(Slurp("D:\\HT9045\\system\\Gerneral.ini")==sGen0, "[7] Gerneral.ini unchanged");
    CHECK(Slurp("D:\\HT9045\\config\\config.ini")==sCfg0, "[7] config.ini unchanged");
    CHECK(Slurp("D:\\HT9045\\SetUp.inf")==sInf0, "[7] SetUp.inf unchanged");

    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    if(g_fail==0)
    {
        ::DeleteFileA(sFile.c_str());
        ::RemoveDirectoryA(GetRecipePath().c_str());
        ::DeleteFileA(sInf.c_str());
    }
    return g_fail==0 ? 0 : 1;
}
