//AI(W906-IFORGPT-IG5) 20261009: Execute AddLoadingCount's G08/G11/G12/G13
// and ProcessSCKARTLoadingCount case 200. Golden 0618 ainarm9045.cpp:
// 2724-2728 / 2741-2759 / 2769-2770 / 2796-2799 / 7332-7333.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "cSocket.h"
#include "common.h"
#include "Config.h"
#include "LastSet.h"
#include "ainarm9045.h"
#include "mykitsuck.h"
#include "Motor/mymotor.h"
#include "forms/fLotInfo.h"
#include "forms/fMesSystem.h"
#include "forms/fObserver.h"
#include "atester_shims.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

extern void AddLoadingCount(int, int, int, int);
extern bool ProcessSCKARTLoadingCount(bool);
extern bool bPickFromLoader;
extern AnsiString W906_ShowErrorMessage_LastCode;
extern AnsiString W906_ShowErrorMessage_LastErrPart;
extern int W906_ShowErrorMessage_LastKCode;
extern int W906_ShowErrorMessage_Count;
extern bool W906_ShowErrorMessage_LastDuplicate;
extern int (*W906_ShowErrorMessage_Hook)(const char*, int, int);
extern void W906_ShowErrorMessage_Reset();

static int checks = 0, failures = 0;
static const char* scenario = "fixture";
static void check(bool ok, const char* expression, int line)
{
    ++checks;
    if (!ok)
    {
        ++failures;
        std::printf("FAIL [%s] line %d: %s\n", scenario, line, expression);
    }
}
#define CHECK(c) check((c), #c, __LINE__)

static std::string readFile(const AnsiString& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    std::ostringstream data;
    data << file.rdbuf();
    return data.str();
}

// Satisfy the same tray-record invariant as boot. No LoadMachineConfig,
// worker, socket or installed hardware driver is involved.
static void fixture()
{
    W906_TestEnsureSimMotors();
    static TRAY_TYPE_PARA trayForm;
    trayForm.XDivision = 4;
    trayForm.YDivision = 3;
    LoadForm = &trayForm;
    TestSocket.iShtRow = 2;
    TestSocket.iShtCol = 4;
    if (!MOT[MMTrayY].Tray.PordRec[3][2])
    {
        MOT[MMTrayY].Tray.PordRec[3][2] = new TMyProductionRecord;
    }
    InArmSuck.Suck[1][1].sName = "IG5-picker-B2";
    InArmSuck.iWhichSht = 0;
    InArmSuck.iWhichKit = 0;
    ASET_ScheduleNAME = "IG5-schedule";
    ASET_StartTimeNAME = "20261009_130000";
    ASE_InTrayNum = 7;
    bCanRunSCKART = false;
    CosFunction.bART_SECSGEM_93K = false;
    IniConfig.bP57LoaderAutoCleanOutByInputCT = false;
    IniConfig.bI27_ManualSortMode = false;
    IniConfig.bE74_InspectArmPosition = false;
    IniConfig.bI37_EnableFIFOMode = false;
    IniConfig.bVTESTFunction = false;
    IniConfig.bSIGURDFunction = false;
    IniConfig.bA10_AutoReTest = false;
    CosFunction.bUseARTSortCount = false;
    CosFunction.bPickerLifeAlmNeedOneCycle = false;
    TestIF.bContinuousLoader = false;
    TestIF_File.bContinuousLoader = false;
    TestIF_File.bContinuousLoader_RT = false;
    fContact->fShow = false;
    fLotInfo->tsLotID->TabVisible = false;
    fLotInfo->edtSysLotID->Text = "";
    CUSTOMER_CODE = 0;
    LastSet.iTester = ON_LINE;
    LastSet.iRunStartMode = rsmInitialStart;
    iRunStartMode = FT;
    TestIF.iContinuousLoaderCount = 1000;
    TestIF.iContinuousLoaderCount_RT = 1000;
}

static void pick()
{
    MOT[MMTrayY].Tray.PordRec[3][2]->InitialRecord();
    MOT[MMTrayY].Tray.Data[3][2] = HAS_IC;
    AddLoadingCount(1, 1, 2, 3);
}

static void lotCases()
{
    scenario = "G08 visible lot";
    fLotInfo->tsLotID->TabVisible = true;
    fLotInfo->edtSysLotID->Text = "IG5-LOT-unique";
    MOT[MInArmX].Motor->SetPosition(1234);
    MOT[MInArmY].Motor->SetPosition(-5678);
    const int before = LastSet.SendCT[0];
    pick();
    TMyProductionRecord& rec = InArmSuck.PordRec[1][1];
    CHECK(rec.asBuffer->Strings[eScheduleName] == "IG5-LOT-unique");
    CHECK(rec.asBuffer->Strings[eStartTime] == "20261009_130000");
    CHECK(rec.GetLoaderNum() == 7 && rec.GetLoaderX() == 3 && rec.GetLoaderY() == 2);
    CHECK(rec.asBuffer->Strings[eLoadXPos] == "1234");
    CHECK(rec.asBuffer->Strings[eLoadYPos] == "-5678");
    CHECK(rec.bUse && InArmSuck.Item[1][1] == HAS_IC);
    CHECK(MOT[MMTrayY].Tray.Data[3][2] == NULL_IC);
    CHECK(LastSet.SendCT[0] == before + 1);

    scenario = "G08 hidden lot fallback";
    fLotInfo->tsLotID->TabVisible = false;
    pick();
    CHECK(rec.asBuffer->Strings[eScheduleName] == "IG5-schedule");
    scenario = "G08 empty lot fallback";
    fLotInfo->tsLotID->TabVisible = true;
    fLotInfo->edtSysLotID->Text = "";
    pick();
    CHECK(rec.asBuffer->Strings[eScheduleName] == "IG5-schedule");
}

static void mesCase(const char* label, bool enabled, bool checkFile,
                    int count, bool initialized, int tester)
{
    scenario = label;
    IniConfig.bVTESTFunction = enabled;
    IniConfig.bCheckFile = checkFile;
    LastSet.SendCT[0] = count;
    LastSet.iTester = tester;
    fMesSystem->bFormShowJustInitial = initialized;
    fMesSystem->iJamRateTotalForAlways = 901;
    LastSet.iBinCTForAlways[0][eAuto1] = 902;
    LastSet.iBinCTForAlways[1][iFixRightHalf] = 903;
    LastSet.iBinCTForAlways[2][eAuto1] = 904;
    LastSet.iSiteTotalCTForAlways[0][0] = 905;
    LastSet.iSiteBinCTForAlways[MAX_SOCKET_ROW-1][MAX_SOCKET_COL-1][TEST_MAX_BIN-1] = 906;
    bool resets = enabled && checkFile && count == 0 && !initialized;
#ifndef SOFT_SIMULTE
    resets = resets && tester == ON_LINE;
#endif
    pick();
    CHECK(fMesSystem->bFormShowJustInitial == (initialized || resets));
    CHECK(fMesSystem->iJamRateTotalForAlways == (resets ? 0 : 901));
    CHECK(LastSet.iBinCTForAlways[0][eAuto1] == (resets ? 0 : 902));
    CHECK(LastSet.iBinCTForAlways[1][iFixRightHalf] == (resets ? 0 : 903));
    CHECK(LastSet.iBinCTForAlways[2][eAuto1] == 904); // golden skips rows 2/3
    CHECK(LastSet.iSiteTotalCTForAlways[0][0] == (resets ? 0 : 905));
    CHECK(LastSet.iSiteBinCTForAlways[MAX_SOCKET_ROW-1][MAX_SOCKET_COL-1][TEST_MAX_BIN-1] == (resets ? 0 : 906));
    CHECK(LastSet.SendCT[0] == count + 1);
}

static void observerCases()
{
    IniConfig.bVTESTFunction = false;
    IniConfig.bSIGURDFunction = true;
    iOneDayLoaderCount = 41;
    const AnsiString path = IncludeTrailingBackslash(AnsiString(std::getenv("W906_EVENTLOG_ROOT"))) +
                            "SGJamCount\\LoaderCount.txt";
    CHECK(W906CtestGuardInScratch(path.c_str()));
    scenario = "G12 enabled persisted count";
    pick();
    CHECK(iOneDayLoaderCount == 42);
    CHECK(readFile(path).find("Count=42") != std::string::npos);
    pick();
    CHECK(iOneDayLoaderCount == 43);
    CHECK(readFile(path).find("Count=43") != std::string::npos);
    scenario = "G12 disabled preserves file";
    const std::string saved = readFile(path);
    IniConfig.bSIGURDFunction = false;
    pick();
    CHECK(iOneDayLoaderCount == 43);
    CHECK(readFile(path) == saved);
}

static void ptiCase(const char* label, int customer, bool continuous, int mode,
                    bool fileContinuous, int alarm, bool refresh)
{
    scenario = label;
    CUSTOMER_CODE = customer;
    TestIF.bContinuousLoader = continuous;
    iTestRunMode = mode;
    TestIF_File.bContinuousLoader = fileContinuous;
    TestIF_File.bContinuousLoader_RT = fileContinuous;
    TestIF_File.iContinuousLoaderCount = alarm;
    TestIF_File.iContinuousLoaderCount_RT = alarm;
    LastSet.SendCT[2] = 19;
    fLotInfo->edLoaderCountNow->Text = "sentinel-now";
    fLotInfo->edLoaderCountAlarm->Text = "sentinel-alarm";
    pick();
    CHECK(LastSet.SendCT[2] == (continuous ? 20 : 19));
    CHECK(fLotInfo->edLoaderCountNow->Text == (refresh ? AnsiString(20) : AnsiString("sentinel-now")));
    CHECK(fLotInfo->edLoaderCountAlarm->Text == (refresh ? AnsiString(alarm) : AnsiString("sentinel-alarm")));
}

static int alarmPos = -1;
static int observeAlarm(const char*, int, int pos)
{
    alarmPos = pos;
    return 0; // retain the product's normal unattended answer
}
static void alarmCases()
{
    scenario = "case200 WAR0120";
    W906_ShowErrorMessage_Reset();
    W906_ShowErrorMessage_Hook = &observeAlarm;
    iLoadPickX = 1;
    iLoadPickY = 1;
    iProcessSCKARTLoadingCountTask = 200;
    bPickFromLoader = true;
    CHECK(ProcessSCKARTLoadingCount(false));
    CHECK(!bPickFromLoader);
    CHECK(W906_ShowErrorMessage_Count == 1);
    CHECK(W906_ShowErrorMessage_LastCode == "WAR0120");
    CHECK(W906_ShowErrorMessage_LastErrPart == "IG5-picker-B2");
    CHECK(W906_ShowErrorMessage_LastKCode == K_RETRY);
    CHECK(!W906_ShowErrorMessage_LastDuplicate && alarmPos == MInArmX);
    scenario = "case200 reset control";
    W906_ShowErrorMessage_Reset();
    CHECK(!ProcessSCKARTLoadingCount(true));
    CHECK(iProcessSCKARTLoadingCountTask == 1);
    CHECK(W906_ShowErrorMessage_Count == 0);
    W906_ShowErrorMessage_Hook = 0;
}

int main()
{
    if (!W906TestRequireCtestRedirects("IforGPT_LoadingCountRuntime"))
    {
        return 2;
    }
    std::setvbuf(stdout, 0, _IONBF, 0);
    fixture();
    lotCases();
    mesCase("G11 first online", true, true, 0, false, ON_LINE);
    mesCase("G11 first offline", true, true, 0, false, OFF_LINE);
    mesCase("G11 feature disabled", false, true, 0, false, ON_LINE);
    mesCase("G11 file disabled", true, false, 0, false, ON_LINE);
    mesCase("G11 subsequent load", true, true, 4, false, ON_LINE);
    mesCase("G11 already initialized", true, true, 0, true, ON_LINE);
    observerCases();
    ptiCase("G13 PTI FT", CC_PTI, true, FT, true, 99, true);
    ptiCase("G13 PTI RT", CC_PTI, true, RT, true, 77, true);
    ptiCase("G13 other customer", 0, true, FT, true, 99, false);
    ptiCase("G13 not continuous", CC_PTI, false, FT, true, 99, false);
    ptiCase("G13 file disabled", CC_PTI, true, FT, false, 99, false);
    alarmCases();
    std::printf("IforGPT_LoadingCountRuntime: %d/%d checks passed; %d failed\n",
                checks-failures, checks, failures);
    return failures ? 1 : 0;
}
