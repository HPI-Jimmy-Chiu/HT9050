//AI(W906-IFORGPT-IG-4) 20261008: Execute all six POOL2 CProd gates.
// Golden 0618:1182/2165/2321/2364/3001/3263. Real callers, no source checks.
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "cSocket.h"
#include "Config.h"
#include "common.h"
#include "database.h"
#include "forms/fLotInfo.h"
#include "forms/fMesSystem.h"
#include "forms/fAGV.h"
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "SECSGEM/uHGemClass.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include "w906_ctest_guard.h"
#include "w906_test_tmpname.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

extern void ProcessLastSetIni_FTP(bool bRead);
extern void ProcessLastSetIni_RMS(bool bRead);

static int g_checks = 0;
static int g_failed = 0;

static void check(bool ok, const char* text, int line)
{
    ++g_checks;
    if (!ok)
    {
        ++g_failed;
        std::printf("FAIL line %d: %s\n", line, text);
    }
    else
    {
        std::printf("PASS: %s\n", text);
    }
}
#define CHECK(c) check((c), #c, __LINE__)

#ifdef IFORGPT_IO_WRAP
// Link wrapping is confined to this test executable. The actual filename and
// HTGem updates are never replaced. Only filesystem boundaries are redirected.
static AnsiString g_scratch;
static AnsiString g_requestedSave;
static AnsiString g_savedReport;
static int g_machineDirectoryProbes = 0;
static bool machinePath(const AnsiString& path)
{
    return path.SubString(1, 3).UpperCase() == "D:\\";
}
extern "C" bool realDirectory(const AnsiString&) asm("___real__ZN9vclcompat15DirectoryExistsERKNS_10AnsiStringE");
extern "C" bool wrapDirectory(const AnsiString&) asm("___wrap__ZN9vclcompat15DirectoryExistsERKNS_10AnsiStringE");
extern "C" bool wrapDirectory(const AnsiString& path)
{
    if (machinePath(path))
    {
        ++g_machineDirectoryProbes;
        return true; // Fixture directories already exist; never mkdir/XCOPY on D:.
    }
    return realDirectory(path);
}
extern "C" int realForce(AnsiString, AnsiString) asm("___real__Z18MyForceDirectoriesN9vclcompat10AnsiStringES0_");
extern "C" int wrapForce(AnsiString, AnsiString) asm("___wrap__Z18MyForceDirectoriesN9vclcompat10AnsiStringES0_");
extern "C" int wrapForce(AnsiString path, AnsiString function)
{
    if (machinePath(path))
    {
        path = g_scratch + "report\\";
    }
    if (!W906CtestGuardInScratch(path.c_str()))
    {
        std::fprintf(stderr, "REFUSED mkdir outside scratch: %s\n", path.c_str());
        std::exit(2);
    }
    return realForce(path, function);
}
extern "C" void __attribute__((thiscall)) realSave(const TStringList*, const AnsiString&)
    asm("___real__ZNK9vclcompat11TStringList10SaveToFileERKNS_10AnsiStringE");
extern "C" void __attribute__((thiscall)) wrapSave(const TStringList*, const AnsiString&)
    asm("___wrap__ZNK9vclcompat11TStringList10SaveToFileERKNS_10AnsiStringE");
extern "C" void __attribute__((thiscall)) wrapSave(const TStringList* list, const AnsiString& path)
{
    AnsiString target = path;
    if (machinePath(path) || !W906CtestGuardInScratch(path.c_str()))
    {
        // Also contain the mutant's empty filename; a closed gate must fail
        // its behavior assertions instead of writing an accidental root path.
        g_requestedSave = path;
        target = g_scratch + "report\\captured.txt";
        ForceDirectories(g_scratch + "report\\");
        g_savedReport = target;
    }
    if (!W906CtestGuardInScratch(target.c_str()))
    {
        std::exit(2);
    }
    realSave(list, target);
}
#endif

struct Scratch
{
    AnsiString savedAuth;
    AnsiString path;
    Scratch() : savedAuth(AuthPath)
    {
        path = IncludeTrailingBackslash(AnsiString(std::getenv("W906_AUTH_PATH"))) +
               AnsiString(W906_TestTmpName("iforgpt_cprod").c_str()) + "\\";
        if (W906CtestGuardInScratch(path.c_str()))
        {
            ForceDirectories(path);
            AuthPath = path;
        }
    }
    ~Scratch()
    {
        CloseIniFile();
        AuthPath = savedAuth;
        // The absolute path is verified before recursive cleanup, including failures.
        if (W906CtestGuardInScratch(path.c_str()) && path ==
            IncludeTrailingBackslash(AnsiString(std::getenv("W906_AUTH_PATH"))) +
            AnsiString(W906_TestTmpName("iforgpt_cprod").c_str()) + "\\")
        {
            W906_TestTmpRemoveTree(path.c_str());
        }
    }
};

static bool writeFixture(const AnsiString& path)
{
    FILE* file = std::fopen((path + "config.ini").c_str(), "wb");
    if (!file)
    {
        return false;
    }
    const char* data =
        "[RMS]\r\nRMS Path=RMS_Normal\\\r\nRMS Download Path=RMS_Download\\\r\n"
        "[Server]\r\nServer Path=Server_Normal\\\r\nServer Download Path=Server_Download\\\r\nServer Enable=1\r\n";
    const int result = std::fputs(data, file);
    const int closed = std::fclose(file);
    return result >= 0 && closed == 0;
}

static void rmsCases()
{
    std::puts("T1 RMS/Server x exact Normal/download x feature flag");
    const int customers[] = { CC_SCC, CC_SCK, CC_PTI };
    const char* modes[] = { "Normal", "Engineer", "normal" };
    IniConfig.bShowLotInfo = true;
    for (int customer = 0; customer < 3; ++customer)
    {
        CUSTOMER_CODE = customers[customer];
        for (int feature = 0; feature < 2; ++feature)
        {
            CosFunction.bDownloadRecipeLevelMode = feature != 0;
            for (int mode = 0; mode < 3; ++mode)
            {
                fLotInfo->coLevelMode->Text = modes[mode];
                IniConfig.sRmsPath = "stale_path";
                IniConfig.sRmsDownPath = "untouched_download";
                ReadRmsPath();
                const bool download = feature != 0 && mode != 0;
                const AnsiString expected = customer < 2 ?
                    (download ? "RMS_Download" : "RMS_Normal") :
                    (download ? "Server_Download" : "Server_Normal");
                std::printf("customer=%d feature=%d mode=%s\n", CUSTOMER_CODE, feature, modes[mode]);
                CHECK(IniConfig.sRmsPath == expected);
                CHECK(IniConfig.sRmsDownPath == (customer < 2 ?
                      AnsiString("RMS_Download") : AnsiString("untouched_download")));
            }
        }
    }
    std::puts("T2 hidden LotInfo preserves both paths");
    IniConfig.bShowLotInfo = false;
    CosFunction.bDownloadRecipeLevelMode = true;
    CUSTOMER_CODE = CC_SCC;
    fLotInfo->coLevelMode->Text = "Engineer";
    IniConfig.sRmsPath = "hidden_normal";
    IniConfig.sRmsDownPath = "hidden_download";
    ReadRmsPath();
    CHECK(IniConfig.sRmsPath == "hidden_normal");
    CHECK(IniConfig.sRmsDownPath == "hidden_download");
}

static void murataCases()
{
    std::puts("T3 Murata fields come from the real IniConfig values");
    CosFunction.bFTPFunction = false;
    IniConfig.bFTPJamCodeUpload = false;
    IniConfig.sN23_2_Line = "line_A";
    IniConfig.sN23_2_Process = "process_B";
    IniConfig.sN23_2_Product = "product_C";
    fLotInfo->edtLine->Text = "old_line";
    fLotInfo->edtProcessName->Text = "old_process";
    fLotInfo->edtProduct->Text = "old_product";
    CUSTOMER_CODE = CC_Murata;
    ProcessLastSetIni_FTP(true);
    CHECK(fLotInfo->edtLine->Text == "line_A");
    CHECK(fLotInfo->edtProcessName->Text == "process_B");
    CHECK(fLotInfo->edtProduct->Text == "product_C");

    std::puts("T4 non-Murata does not replace the widgets");
    // Seed this case independently so a T3 failure does not also fail its control.
    fLotInfo->edtLine->Text = "line_A";
    fLotInfo->edtProcessName->Text = "process_B";
    fLotInfo->edtProduct->Text = "product_C";
    CUSTOMER_CODE = CC_PTI;
    IniConfig.sN23_2_Line = "different_line";
    IniConfig.sN23_2_Process = "different_process";
    IniConfig.sN23_2_Product = "different_product";
    ProcessLastSetIni_FTP(true);
    CHECK(fLotInfo->edtLine->Text == "line_A");
    CHECK(fLotInfo->edtProcessName->Text == "process_B");
    CHECK(fLotInfo->edtProduct->Text == "product_C");

    std::puts("T5 Murata empty values clear previously populated widgets");
    CUSTOMER_CODE = CC_Murata;
    IniConfig.sN23_2_Line = "";
    IniConfig.sN23_2_Process = "";
    IniConfig.sN23_2_Product = "";
    ProcessLastSetIni_FTP(true);
    CHECK(fLotInfo->edtLine->Text.IsEmpty());
    CHECK(fLotInfo->edtProcessName->Text.IsEmpty());
    CHECK(fLotInfo->edtProduct->Text.IsEmpty());

    std::puts("T6 null form is accepted by the real caller");
    TfLotInfo* saved = fLotInfo;
    fLotInfo = NULL;
    ProcessLastSetIni_FTP(true);
    fLotInfo = saved;
    CHECK(fLotInfo == saved);
}

#ifdef IFORGPT_IO_WRAP
static void jamCases()
{
    std::puts("T7 actual VTEST filename and report serialization, upload disabled");
    IniConfig.bVTESTFunction = true;
    IniConfig.SocketHandlerID = "HANDLER_A";
    fMesSystem->lbledtCustLotNum->Text = "CUSTOMER_B";
    fMesSystem->LabeledEditLotNo->Text = "MESLOT_C";
    fMesSystem->lbledtC1->Text = "C1_D";
    fLotInfo->cbRunMode->Text = "MODE_E";
    fLotInfo->cbProcess->Text = "PROCESS_F";
    SystemYear = 2026;
    SystemMonth = 10;
    SystemDate = 8;
    SystemHour = 9;
    SystemMin = 45;
    SystemSec = 6;
    RunInfo.bLotStart = true;
    RunInfo.LotNo = "REPORT_LOT";
    RunInfo.LotStartTime = "START";
    RunInfo.LotEndTime = "END";
    RunInfo.vByLotJam.clear();
    TastCategory.iTotalSocket = 20;
    CUSTOMER_CODE = CC_PTI;
    sJamRatePath = "D:\\stale_jam_path";
    RunInfo.SaveJamRateByLot(false);
    const AnsiString expected = "D:\\PnPh\\report\\LotAlarm\\HANDLER_A-CUSTOMER_B-MESLOT_C-C1_D-MODE_E-PROCESS_F-20261008094506.txt";
    CHECK(RunInfo.JamRateFileName == expected);
    CHECK(g_requestedSave == expected);
    CHECK(sJamRatePath == "D:\\PnPh\\report\\LotAlarm");
    CHECK(W906CtestGuardInScratch(g_savedReport.c_str()));
    std::ifstream file(g_savedReport.c_str(), std::ios::binary);
    std::ostringstream content;
    content << file.rdbuf();
    CHECK(file.good() && content.str().find("Lot No: REPORT_LOT\r\n") != std::string::npos);
    CHECK(content.str().find("Unloading counter: 20\r\n") != std::string::npos);
    CHECK(content.str().find("Jam counter: 0\r\nMUBJ: 0/20\r\n") != std::string::npos);

    std::puts("T8 a changed MES field and date reach the real VTEST filename");
    fMesSystem->LabeledEditLotNo->Text = "MESLOT_CHANGED";
    SystemMonth = 2;
    SystemDate = 3;
    SystemHour = 4;
    SystemMin = 5;
    SystemSec = 9;
    sJamRatePath = "D:\\stale_jam_path";
    RunInfo.SaveJamRateByLot(false);
    CHECK(RunInfo.JamRateFileName == "D:\\PnPh\\report\\LotAlarm\\HANDLER_A-CUSTOMER_B-MESLOT_CHANGED-C1_D-MODE_E-PROCESS_F-20260203040509.txt");

    std::puts("T9 non-VTEST preserves its separate naming convention");
    IniConfig.bVTESTFunction = false;
    IniConfig.sMachineType = "MODEL";
    sJamRatePath = "D:\\control_jam";
    RunInfo.SaveJamRateByLot(false);
    CHECK(RunInfo.JamRateFileName == "D:\\control_jam\\MODEL_HANDLER_A_20260203-040509_REPORT_LOT_MODE_E_PROCESS_F_JamRateByLot.txt");
}

static void gemCases()
{
    std::puts("T10 SPIL RMS caller updates the real HTGem instance");
    HTGem gem;
    HTGem* savedGem = HSys.MyGem;
    const AnsiString savedData = DataPath;
    HSys.MyGem = &gem;
    CUSTOMER_CODE = CC_PTI;
    IniConfig.bShowLotInfo = true;
    IniConfig.bVTESTFunction = false;
    IniConfig.bSPILFunction = true;
    CosFunction.bUseERMS = false;
    CosFunction.bDownloadRecipeLevelMode = false;
    DataPath = g_scratch;
    gem.DataPath = "stale_RMS";
    int probes = g_machineDirectoryProbes;
    ProcessLastSetIni_RMS(true);
    CHECK(IniConfig.bEnableRms);
    CHECK(DataPath == "D:\\HT9045\\IniData\\Data\\Active\\");
    CHECK(gem.DataPath == "D:\\HT9045\\IniData\\Data\\Active\\");
    CHECK(g_machineDirectoryProbes > probes);

    std::puts("T11 disabled RMS preserves GEM, including the null-GEM branch");
    IniConfig.bShowLotInfo = false;
    gem.DataPath = "disabled_RMS";
    ProcessLastSetIni_RMS(true);
    CHECK(gem.DataPath == "disabled_RMS");
    HSys.MyGem = NULL;
    IniConfig.bShowLotInfo = true;
    ProcessLastSetIni_RMS(true);
    CHECK(HSys.MyGem == NULL);
    HSys.MyGem = &gem;

    std::puts("T12 TSMC FTP on/off selects both actual GEM paths");
    CUSTOMER_CODE = CC_TSMC_TAINAN;
    CosFunction.bFTPFunction = false;
    IniConfig.bFTPJamCodeUpload = false;
    for (int enabled = 0; enabled < 2; ++enabled)
    {
        IniConfig.bEnableFTP = enabled != 0;
        gem.DataPath = "stale_FTP";
        probes = g_machineDirectoryProbes;
        ProcessLastSetIni_FTP(true);
        const AnsiString expected = enabled ? "D:\\HT9045\\IniData\\DataFTP\\" : "D:\\HT9045\\IniData\\Data\\";
        CHECK(DataPath == expected);
        CHECK(gem.DataPath == expected);
        CHECK(g_machineDirectoryProbes >= probes + 2);
    }
    CUSTOMER_CODE = CC_PTI;
    gem.DataPath = "non_TSMC";
    ProcessLastSetIni_FTP(true);
    CHECK(gem.DataPath == "non_TSMC");
    CUSTOMER_CODE = CC_TSMC_TAINAN;
    HSys.MyGem = NULL;
    ProcessLastSetIni_FTP(true);
    CHECK(HSys.MyGem == NULL);
    HSys.MyGem = &gem;

    std::puts("T13 real ReadLastSetIni initializes GEM from scratch-only inputs");
    CUSTOMER_CODE = CC_PTI;
    IniConfig.bSPILFunction = false;
    IniConfig.bVTESTFunction = false;
    IniConfig.bFTPJamCodeUpload = false;
    REAL_TIME_CCD = false;
    DataPath = g_scratch + "recipe\\";
    const AnsiString savedGeneral = asGeneralPath;
    asGeneralPath = g_scratch + "general.ini";
    CHECK(W906CtestGuardInScratch(DataPath.c_str()));
    CHECK(W906CtestGuardInScratch(asGeneralPath.c_str()));
    // Same prerequisite as the real config loader: General readers do not open it.
    OpenGeneralIniFile();
    gem.DataPath = "stale_LastSet";
    ReadLastSetIni();
    CHECK(gem.DataPath == "D:\\HT9045\\IniData\\Data\\");
    CHECK(DataPath == g_scratch + "recipe\\");
    HSys.MyGem = NULL;
    ReadLastSetIni();
    CHECK(HSys.MyGem == NULL);
    CloseGeneralIniFile();
    asGeneralPath = savedGeneral;
    DataPath = savedData;
    HSys.MyGem = savedGem;
}
#endif

int main()
{
#ifndef IFORGPT_IO_WRAP
    std::puts("SKIP: IG-4 requires MinGW test-only filesystem link wrapping");
    return 77;
#else
    std::setvbuf(stdout, NULL, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("IforGPT_CProdRuntime"))
    {
        return 2;
    }
    Scratch scratch;
    g_scratch = scratch.path;
    CHECK(AuthPath == scratch.path);
    CHECK(W906CtestGuardInScratch(AuthPath.c_str()));
    CHECK(writeFixture(scratch.path));
    if (g_failed)
    {
        return 1;
    }
    if (!fLotInfo)
    {
        fLotInfo = new TfLotInfo();
    }
    CHECK(fMesSystem && fMesSystem->lbledtCustLotNum &&
          fMesSystem->LabeledEditLotNo && fMesSystem->lbledtC1 && fAGV);
    CHECK(fLotInfo && fLotInfo->coLevelMode && fLotInfo->edtLine &&
          fLotInfo->edtProcessName && fLotInfo->edtProduct);
    if (g_failed)
    {
        return 1;
    }
    rmsCases();
    murataCases();
    jamCases();
    gemCases();
    std::printf("IforGPT_CProdRuntime: %d checks, %d failed\n", g_checks, g_failed);
    return g_failed ? 1 : 0;
#endif
}
