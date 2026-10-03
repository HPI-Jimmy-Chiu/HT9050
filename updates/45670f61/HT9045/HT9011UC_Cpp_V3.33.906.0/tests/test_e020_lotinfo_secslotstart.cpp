// =============================================================================
//  test_e020_lotinfo_secslotstart.cpp -- todo E-020 LI-1: golden TfLotInfo::sbSECSLotStartClick as W906_LotInfo_SECSLotStart
//
//  //AI(W906-E020-LI1) 20261002 [W906] (St01): new file.  Golden V912
//    D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:7501-8583.  Under test: FileRW/LotInfo_SECSLotStart.cpp,
//    compiled here exactly as wb_serve compiles it, against the real god stack (RESCAN group).
//    [0]  ctest sandbox (w906_ctest_guard.h + AuthPath / as9045LogPath / DataPath in scratch), real-file snapshot, log objects
//    [1]  success path (CYUEAN-like default): true, RunInfo.bLotStart, RunInfo.LotNo, config.ini [Lot Info] in the sandbox,
//         golden's start-of-click resets (bReadLotInfoFromART, iByBinCnt, iExceptAutoCnt, mmo2DLotInfo, dtStartLot), no box
//    [2]  default chain refusals: LotID / OPID empty, Run Mode empty when visible (and not when hidden), SPIL skips it
//    [3]  TCP_IP_MODE without SECS (:8071) vs XINYUN (LotID only, :8367)
//    [4]  Barcode Recipe required (:8082)
//    [5]  special characters (:8019-8054): field blanked + box + go on; '!' and '\' rejected; '-' '_' accepted
//    [6]  SCC 5..30 length (:8056-8069, #ifndef SOFT_SIMULTE -- pinned for both configurations)
//    [7]  Murata (:8095-8158) refusals and DAYDATE (:8517-8527)
//    [8]  TSI / Greatek / SIGURD_ChungXing (no return, golden oddity) / LEADYO / SJ_OS RFID / JCET (Down=false)
//    [9]  VTEST: LI1-1 VTENG password gate, empty IDs, silent already-downloaded return, VTEST_Shanghai OP<4 and LI1-7,
//         LI1-9 MES download gate, the no-download NoRTBin flags, LI1-12 kit check skip
//    [10] PTI: offline refusal + trace file (sandbox), Run Mode normalisation
//    [11] 2D sort (:7539-7755): N23 lot info missing, the old list deleted, Net Drive copy + LI1-4, copy fail -> WAR1684 + missing,
//         FTP LI1-3 skip, manual LI1-2 skip, SECS goes on; SPIL 2D chain refusals
//    [12] 2DID allow list (:7758-8017): Run Mode / ItemIndex, CORR, non-JCET silent return (btnASECL_LotStart->Down=false),
//         JCET lot length, LI1-5 gate, JCET FT1 without list goes on, manual LI1-2 skip; SPIL else-chain clears the barcode list
//    [13] OEE LI1-10, CSV-compare flag on = no skip (LI1-8 removed by E-030: 912-only), PANTHER LI1-11 skip + SystemAccSecond, AMD LI1-13 skip, TSMC box, First Tray
//    [14] source ratchets (argv[1] = port root): the callable is live (not under #if 0), on St01's wb_serve source line of
//         CMakeLists.txt before its '#', every gate LI1-1..13 except 8 present as #if 0 / GATE (LI1-8 and the CSV field absent), no START spelled in the file
//    [15] real files untouched: Gerneral.ini, config\config.ini, system\ArmByLot*.dat, and the D:\HT9045_Log folders this
//         function can write (stat / listing only)
// =============================================================================
#include "FileRW/LotInfo_SECSLotStart.h"

#include "MachineType.h"
#include "forms/fLotInfo.h"
#include "forms/fMesSystem.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "common.h"
#include "canary_support.h"
#include "cpublic.h"
#include "LogObjects.h"
#include "w906_ctest_guard.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

extern int W906_ClearBarcodeListCallCount;   // forms/fLotInfo.cpp:6551 (btClearBarcodeListClick seat)
bool HasICUnderMachine();                    // csystem.h:105
bool HasAnyICInMachine();                    // csystem.h:109

static int g_fail = 0, g_pass = 0;
static void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  ok   %s\n", what.c_str()); }
    else    { ++g_fail; std::printf("  FAIL %s\n", what.c_str()); }
}

// ---- every ShowMyMessage S1 / S2 (the seam keeps only the last) --------------------------------------------------------
static std::vector<std::string> g_box1, g_box2;
static void BoxRecorder(const char* s1, const char* s2) { g_box1.push_back(s1 ? s1 : ""); g_box2.push_back(s2 ? s2 : ""); }

// ---- files ----------------------------------------------------------------------------------------------------------------
struct Meta { bool exists; long long size; long long mtime; };
static Meta MetaOf(const std::string& p)
{
    struct stat st;
    Meta m = { false, 0, 0 };
    if (stat(p.c_str(), &st) == 0) { m.exists = true; m.size = (long long)st.st_size; m.mtime = (long long)st.st_mtime; }
    return m;
}
static bool SameMeta(const Meta& a, const Meta& b) { return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime; }
static std::string Listing(const std::string& dir)
{
    std::string out;
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return "(none)";
    do {
        std::ostringstream os;
        os << fd.cFileName << ':' << fd.nFileSizeLow << ':' << fd.ftLastWriteTime.dwLowDateTime << ';';
        out += os.str();
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return out;
}
static bool Exists(const std::string& p) { return MetaOf(p).exists; }
static void WriteAll(const std::string& p, const std::string& s) { std::ofstream f(p.c_str(), std::ios::binary); f << s; }
static std::string ReadAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::ostringstream os; os << f.rdbuf(); return os.str();
}
static std::string S(const AnsiString& a) { return std::string(a.c_str()); }
static std::string LotIni(const char* key) { return S(ReadIniData(AuthPath + "config.ini", "Lot Info", key, AnsiString("<none>"))); }
static bool Has(const std::vector<std::string>& v, const std::string& needle)
{
    for (size_t i = 0; i < v.size(); ++i) if (v[i].find(needle) != std::string::npos) return true;
    return false;
}
static bool Starts(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }

// ---- one call -------------------------------------------------------------------------------------------------------------
struct R { bool ran; std::string why; std::vector<std::string> skipped; };
static R Run(const char* lot, const char* op)
{
    fLotInfo->edtSysLotID->Text = lot;
    fLotInfo->edtSysOperatorID->Text = op;
    R r;
    r.ran = W906_LotInfo_SECSLotStart(&r.why, &r.skipped);
    return r;
}

static int g_savedTestType = 0, g_savedBarCodeInstall = 0;
static void RunMode(const char* text, int itemIndex, bool visible = true)
{
    fLotInfo->cbRunMode->Items->Clear();
    fLotInfo->cbRunMode->Items->Add("FT");
    fLotInfo->cbRunMode->Items->Add("RT");
    fLotInfo->cbRunMode->Items->Add("CORR");
    fLotInfo->cbRunMode->Items->Add("FT1");
    fLotInfo->cbRunMode->Items->Add("Normal");
    fLotInfo->cbRunMode->Text = text;
    fLotInfo->cbRunMode->ItemIndex = itemIndex;
    fLotInfo->cbRunMode->Visible = visible;
}

// Same starting point for every case (golden flags off, CYUEAN, nothing started, widgets enabled).
static void Baseline()
{
    CUSTOMER_CODE = CC_CYUEAN;
    IniConfig.bVTESTFunction = false; IniConfig.bEnable_SECS_GEM = false; IniConfig.bSPILFunction = false;
    IniConfig.iN23DownloadMethod = 0; IniConfig.bN23UseLotInfoFile = false; IniConfig.sN23LotInfoPath = "";
    IniConfig.sN23DownloadDrivePath = ""; IniConfig.cN23FtpDownloadPath = ""; IniConfig.sN23_4_URL = "";
    IniConfig.bEnableRms = false; IniConfig.bEnableFTP = false; IniConfig.bCheckFile = false;
    IniConfig.bC11UseMonitorView = false; IniConfig.bN33_1_NetChangeFileAndData = false; IniConfig.bLifeTimeCount[0] = false;
    IniConfig.bP62FirstTrayCheckOnUnloader = false; IniConfig.bP62AlwaysEnabledAtLotStart = false;
    IniConfig.bAMDFunction = false; IniConfig.bB03_TesterReport = false;
    CosFunction.bSortingBy2DList = false; CosFunction.bOEEFunction = false; CosFunction.bMakeWhite2DIDList = false;
    CosFunction.bUseHeadContactCount = false; CosFunction.bFirstTrayCheckOnUnloader = false; CosFunction.bRunModeFollowLotInfo = false;
    TestIF_File.b2DIDAllowList = false; TestIF_File.bSortingBy2DIDList = false; TestIF_File.bEnableBarCode = false;
    TestIF_File.bBarCodeMultiRecipe = false; TestIF_File.bEnableBarcodeCSVCompare = false; TestIF_File.bChkMakeWhite2DIDList = false;
    TestIF.iTestType = g_savedTestType; BAR_CODE_INSTALL = g_savedBarCodeInstall;
    LastSet.iTester = 1; LastSet.iRunStartMode = rsmInitialStart;
    OFFLINE_ALARM = false; AccessLevel = iDefSupervisorLevel; USE_RFID_READER = 0; SPIL_FOR_QLE = 0; iAMD_Function = 0;
    sTotalLotID = "E020TOT";
    fMesSystem->bDownloadLotInforFlag = false;
    fLotInfo->SetLotComponents(true);                 // golden lot-end widget state; RunInfo.bLotStart=false
    RunInfo.bLotStart = false;
    RunMode("FT", 0, true);
    fLotInfo->edtCusLotID->Text = ""; fLotInfo->edtBarcodeRecipe->Text = ""; fLotInfo->edPage->Text = "";
    fLotInfo->edtLine->Text = ""; fLotInfo->edtProcessName->Text = ""; fLotInfo->edtProduct->Text = "";
    fLotInfo->edCustomerLotId->Text = ""; fLotInfo->coStation->Text = ""; fLotInfo->edStationNum->Text = "";
    fLotInfo->pnlLoader->Caption = "";
    fLotInfo->sbSECSLotEnd->Down = false; fLotInfo->btnASECL_LotStart->Down = true;
    fLotInfo->cbFirstTrayCheckOnUnloader->Checked = false;
    W906_ShowMyMessage_Reset(); g_box1.clear(); g_box2.clear();
    W906_ShowErrorMessage_Reset(); W906_ShowErrorMessage_SimReturn = K_SKIP;   // a failing copy loops on K_RETRY (golden), so answer Skip
}

int main(int argc, char** argv)
{
    std::printf("test_e020_lotinfo_secslotstart -- E-020 LI-1 golden TfLotInfo::sbSECSLotStartClick (V912 uLotInfo.cpp:7501-8583)\n");
    const std::string root = argc > 1 ? argv[1] : "";

    // ---- [0] -----------------------------------------------------------------------------------------------------------
    std::printf("[0] sandbox\n");
    {
        const char* const rt[] = { "AuthPath", AuthPath.c_str(), "as9045LogPath", as9045LogPath.c_str(), "DataPath", DataPath.c_str(),
                                   "asGeneralPath", asGeneralPath.c_str(), 0 };
        if (!W906TestRequireCtestRedirects("E020_LotInfoSECSLotStart", rt))
            return 2;
    }
    const char* const kRealFiles[] = {
        "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini",
        "D:\\HT9045\\system\\ArmByLot0.dat", "D:\\HT9045\\system\\ArmByLot1.dat", "D:\\HT9045\\system\\ArmByLot2.dat",
        "D:\\HT9045\\system\\ArmByLot0_backup.dat", "D:\\HT9045\\system\\ArmByLot1_backup.dat", "D:\\HT9045\\system\\ArmByLot2_backup.dat", 0 };
    const char* const kRealDirs[] = {
        "D:\\HT9045_Log\\2D_SortList", "D:\\HT9045_Log\\2D_MappingResult", "D:\\HT9045_Log\\PTI_LotStartTrace",
        "D:\\HT9045_Log\\2DBarCode\\CheckResult", "D:\\HT9045_Log\\LotInfo", "D:\\HT9045\\IniData\\Data", 0 };
    std::vector<Meta> realMeta;
    std::vector<std::string> realList;
    for (int i = 0; kRealFiles[i]; ++i) realMeta.push_back(MetaOf(kRealFiles[i]));
    for (int i = 0; kRealDirs[i]; ++i) realList.push_back(Listing(kRealDirs[i]));

    const std::string logRoot = S(as9045LogPath);
    _mkdir(logRoot.c_str());
    _mkdir((logRoot + "\\2D_SortList").c_str());
    _mkdir(S(AuthPath).c_str());
    g_savedTestType = (TestIF.iTestType == TCP_IP_MODE) ? 0 : TestIF.iTestType;
    g_savedBarCodeInstall = (BAR_CODE_INSTALL == ebctUseCCDMode) ? 0 : BAR_CODE_INSTALL;
    GetTimeInfo();
    W906_CreateLogObjects();                              // wb_serve boot does this; SetLotStart derefs slEventLog
    Check(slEventLog != 0, "[0] log objects built (slEventLog), under as9045LogPath = " + logRoot);
    W906_ShowMyMessage_Hook = &BoxRecorder;

    // ---- [1] success -----------------------------------------------------------------------------------------------------
    std::printf("[1] success path\n");
    {
        Baseline();
        bReadLotInfoFromART = true;
        iByBinCnt[0] = 7; iByBinCnt[3] = 9; iExceptAutoCnt[0] = 5;
        fLotInfo->mmo2DLotInfo->Lines->Add("preset line");
        dtStartLot = 0.0;
        R r = Run("E020LOT1", "OP01");
        Check(r.ran && r.why.empty(), "[1] runs to the end (SetLotStart called), whyNot empty");
        Check(RunInfo.bLotStart, "[1] RunInfo.bLotStart = true (SetLotStart -> SetLotComponents(false))");
        Check(S(RunInfo.LotNo) == "E020LOT1", "[1] RunInfo.LotNo = the Lot ID (SetLotID :8536)");
        Check(LotIni("Lot ID") == "E020LOT1" && LotIni("Lot No") == "E020LOT1" && LotIni("Operator") == "OP01" && LotIni("Run Mode") == "FT",
              "[1] sandbox config.ini [Lot Info] Lot ID / Lot No / Operator / Run Mode (" + S(AuthPath) + "config.ini)");
        Check(bReadLotInfoFromART == false, "[1] :7512 bReadLotInfoFromART=false");
        Check(iByBinCnt[0] == 0 && iByBinCnt[3] == 0 && iExceptAutoCnt[0] == 0, "[1] :7520-7521 iByBinCnt / iExceptAutoCnt zeroed");
        Check(fLotInfo->mmo2DLotInfo->Lines->Count == 0, "[1] :7522 mmo2DLotInfo cleared");
        Check((double)dtStartLot > 40000.0, "[1] :7523 dtStartLot = Now()");
        Check(g_box1.empty() && r.skipped.empty(), "[1] no message box, nothing gated on this path");
        Check(fLotInfo->edtSysLotID->Enabled == false && fLotInfo->sbSECSLotStart->Down == true, "[1] lot widgets locked (golden SetLotComponents(false))");
    }

    // ---- [2] default chain -----------------------------------------------------------------------------------------------
    std::printf("[2] default chain (:8349-8395)\n");
    {
        Baseline();
        R r = Run("", "OP01");
        Check(!r.ran && r.why == "Please Enter LotID and Operator ID!!" && !RunInfo.bLotStart, "[2] empty Lot ID -> :8381 refusal");
        Check(fLotInfo->sbSECSLotEnd->Down == true && g_box1.size() == 1 && g_box1[0] == r.why, "[2] Down=true and golden's box shown");
        Baseline();
        r = Run("E020LOT2", "");
        Check(!r.ran && r.why == "Please Enter LotID and Operator ID!!", "[2] empty Operator ID -> :8381 refusal");
        Check(LotIni("Lot ID") == "E020LOT1", "[2] a refusal writes nothing (config.ini still holds [1]'s lot)");
        Baseline(); RunMode("", -1, true);
        r = Run("E020LOT2", "OP01");
        Check(!r.ran && r.why == "Please select run mode!!", "[2] Run Mode empty and visible -> :8392 refusal");
        Baseline(); RunMode("", -1, false);
        r = Run("E020LOT2", "OP01");
        Check(r.ran && RunInfo.bLotStart, "[2] Run Mode empty but hidden -> no check (golden cbRunMode->Visible)");
        Baseline(); RunMode("", -1, true); IniConfig.bSPILFunction = true;
        r = Run("E020LOT2", "OP01");
        Check(r.ran && RunInfo.bLotStart, "[2] SPIL: empty Run Mode is not checked (:8385 empty arm)");
    }

    // ---- [3] TCP / XINYUN ------------------------------------------------------------------------------------------------
    std::printf("[3] TCP_IP_MODE / XINYUN\n");
    {
        Baseline(); CUSTOMER_CODE = CC_XINYUN;
        R r = Run("E020LOT3", "");
        Check(r.ran && RunInfo.bLotStart, "[3] XINYUN needs only the Lot ID (:8367-8375)");
        Baseline(); CUSTOMER_CODE = CC_XINYUN; TestIF.iTestType = TCP_IP_MODE;
        r = Run("E020LOT3", "");
        Check(!r.ran && r.why == "Please Enter LotID and Operator ID!!" && g_box1.size() == 1, "[3] TCP_IP_MODE without SECS needs both, before the customer chain (:8071-8079)");
        Baseline(); CUSTOMER_CODE = CC_XINYUN; TestIF.iTestType = TCP_IP_MODE; IniConfig.bEnable_SECS_GEM = true;
        r = Run("E020LOT3", "");
        Check(r.ran, "[3] TCP_IP_MODE with SECS: no :8071 check");
        Baseline(); CUSTOMER_CODE = CC_XINYUN;
        r = Run("", "OP01");
        Check(!r.ran && r.why == "Please Enter LotID!!", "[3] XINYUN empty Lot ID -> :8373");
    }

    // ---- [4] barcode recipe ----------------------------------------------------------------------------------------------
    std::printf("[4] Barcode Recipe (:8082-8092)\n");
    {
        Baseline(); TestIF_File.bEnableBarCode = true; BAR_CODE_INSTALL = ebctUseCCDMode; TestIF_File.bBarCodeMultiRecipe = true;
        R r = Run("E020LOT4", "OP01");
        Check(!r.ran && r.why == "Please Enter Barcode Recipe!!" && fLotInfo->sbSECSLotEnd->Down, "[4] empty Barcode Recipe -> refusal");
        Baseline(); TestIF_File.bEnableBarCode = true; BAR_CODE_INSTALL = ebctUseCCDMode; TestIF_File.bBarCodeMultiRecipe = true;
        fLotInfo->edtBarcodeRecipe->Text = "R1";
        r = Run("E020LOT4", "OP01");
        Check(r.ran && RunInfo.bLotStart, "[4] with a Barcode Recipe -> runs");
    }

    // ---- [5] special characters ------------------------------------------------------------------------------------------
    std::printf("[5] special characters (:8019-8054)\n");
    {
        Baseline();
        R r = Run("E020/LOT", "OP01");
        Check(!r.ran && g_box1.size() == 2 && g_box1[0] == "Lot ID can not use special charater \\/:*?\"<>|" && g_box2[0] == "E020/LOT" &&
              r.why == "Please Enter LotID and Operator ID!!" && S(fLotInfo->edtSysLotID->Text) == "",
              "[5] '/' in Lot ID: box with the text, field blanked, no return there; the empty-ID check refuses");
        Baseline();
        r = Run("E020!LOT", "OP01");
        Check(!r.ran && g_box1.size() == 2 && Starts(g_box1[0], "Lot ID can not use"), "[5] '!' is rejected too (in golden's class, not in its message)");
        Baseline();
        r = Run("E020\\LOT", "OP01");
        Check(!r.ran && g_box1.size() == 2 && Starts(g_box1[0], "Lot ID can not use"), "[5] backslash rejected (the AnsiPos(\"\\\\\") half)");
        Baseline();
        r = Run("E020LOT5", "OP:1");
        Check(!r.ran && g_box1.size() == 2 && Starts(g_box1[0], "OP ID can not use") && r.why == "Please Enter LotID and Operator ID!!", "[5] ':' in OP ID");
        Baseline(); RunMode("F*T", 0, true);
        r = Run("E020LOT5", "OP01");
        Check(!r.ran && g_box1.size() == 2 && Starts(g_box1[0], "Run Mode can not use") && r.why == "Please select run mode!!", "[5] '*' in Run Mode -> blanked -> run mode refusal");
        Baseline();
        r = Run("E020-LOT_5", "OP-01");
        Check(r.ran && g_box1.empty(), "[5] '-' and '_' are accepted");
    }

    // ---- [6] SCC (#ifndef SOFT_SIMULTE) ----------------------------------------------------------------------------------
    std::printf("[6] SCC 5..30 (:8056-8069)\n");
    {
        Baseline(); CUSTOMER_CODE = CC_SCC;
        R r = Run("ABCD", "OPERATOR1");
#ifdef SOFT_SIMULTE
        Check(r.ran && RunInfo.bLotStart && S(RunInfo.LotNo) == "ABCD", "[6] SIM build (SOFT_SIMULTE): golden's SCC length block is compiled out -> a 4-char Lot ID starts");
#else
        Check(!r.ran && r.why == "Please Enter LotID and Operator ID!!" && LotIni("Lot ID") == "", "[6] SHIP build: 4-char Lot ID -> SetLotID(\"\") (config.ini Lot ID emptied) -> refusal");
#endif
        Baseline(); CUSTOMER_CODE = CC_SCC;
        r = Run("ABCDE", "OPERATOR1");
        Check(r.ran && RunInfo.bLotStart, "[6] 5-char Lot ID starts in both configurations");
    }

    // ---- [7] Murata ------------------------------------------------------------------------------------------------------
    std::printf("[7] Murata\n");
    {
        Baseline(); CUSTOMER_CODE = CC_Murata;
        R r = Run("E020LOT7", "OP01");
        Check(!r.ran && r.why == "Please Enter Line and Process and Product Name!!", "[7] :8142 Line / Process / Product");
        Baseline(); CUSTOMER_CODE = CC_Murata;
        fLotInfo->edtLine->Text = "L1"; fLotInfo->edtProcessName->Text = "P1"; fLotInfo->edtProduct->Text = "D1";
        r = Run("E020LOT7", "OP01");
        Check(!r.ran && r.why == "Please Enter lot information!!", "[7] :8149 Page empty");
        Baseline(); CUSTOMER_CODE = CC_Murata;
        fLotInfo->edtLine->Text = "L/1"; fLotInfo->edtProcessName->Text = "P1"; fLotInfo->edtProduct->Text = "D1"; fLotInfo->edPage->Text = "2";
        r = Run("E020LOT7", "OP01");
        Check(!r.ran && Starts(g_box1[0], "Line ID can not use") && r.why == "Please Enter Line and Process and Product Name!!", "[7] '/' in Line -> blanked -> refusal");
        Baseline(); CUSTOMER_CODE = CC_Murata; GetTimeInfo();
        fLotInfo->edtLine->Text = "L1"; fLotInfo->edtProcessName->Text = "P1"; fLotInfo->edtProduct->Text = "D1"; fLotInfo->edPage->Text = "2";
        AnsiString y; y.sprintf("%04d", SystemYear); AnsiString d; d.sprintf("%s%02d%02d", y.SubString(3, 2), SystemMonth, SystemDate);
        r = Run("M_DAYDATE", "OP01");
        Check(r.ran && S(RunInfo.LotNo) == "M_" + S(d), "[7] :8517-8527 DAYDATE -> YYMMDD (" + S(RunInfo.LotNo) + ")");
    }

    // ---- [8] other customers ---------------------------------------------------------------------------------------------
    std::printf("[8] TSI / Greatek / SIGURD_ChungXing / LEADYO / SJ_OS / JCET\n");
    {
        Baseline(); CUSTOMER_CODE = CC_TSI;
        R r = Run("", "OP01");
        Check(!r.ran && r.why == "Please Enter LotID and Operator ID!!", "[8] TSI empty -> :8167");
        Baseline(); CUSTOMER_CODE = CC_TSI; LastSet.TrayCount[0] = 5; LastSet.TrayCount[9] = 6;
        r = Run("E020LOT8", "OP01");
        Check(r.ran && LastSet.TrayCount[0] == 0 && LastSet.TrayCount[9] == 0, "[8] TSI not started -> :8172-8173 TrayCount[0..9]=0");

        Baseline(); CUSTOMER_CODE = CC_Greatek;
        r = Run("", "");
        Check(r.ran && !RunInfo.bLotStart && g_box1.empty(), "[8] Greatek checks nothing: true, but SetLotStart leaves RunInfo.bLotStart false with an empty Lot ID");

        Baseline(); CUSTOMER_CODE = CC_SIGURD_ChungXing;
        r = Run("E020LOT8", "OP01");
        Check(r.ran && g_box1.size() == 1 && g_box1[0] == "Please Enter lot information!!" && RunInfo.bLotStart,
              "[8] SIGURD_ChungXing: box but NO return (golden :8188 oddity) -> lot starts");

        Baseline(); CUSTOMER_CODE = CC_LEADYO; IniConfig.bN33_1_NetChangeFileAndData = true;
        r = Run("E020LOT8", "OP01");
        Check(!r.ran && Starts(r.why, "GATE (W906-E020-LI1-6)"), "[8] LEADYO [N33_1] -> GATE LI1-6 refusal");
        Baseline(); CUSTOMER_CODE = CC_LEADYO;
        r = Run("", "OP01");
        Check(!r.ran && r.why == "Please Enter LotID!!", "[8] LEADYO empty Lot ID -> :8328");

        Baseline(); CUSTOMER_CODE = CC_SJ_Semiconductor_OS; USE_RFID_READER = 1;
        r = Run("E020LOT8", "OP01");
        Check(!r.ran && r.why == "Please Enter Tray ID!!", "[8] SJ_OS + RFID reader, no tray ID -> :8242");

        Baseline(); CUSTOMER_CODE = CC_JCET; fLotInfo->sbSECSLotEnd->Down = true;
        r = Run("", "OP01");
        Check(!r.ran && r.why == "Please Enter LotID and Operator ID!!" && fLotInfo->sbSECSLotEnd->Down == false,
              "[8] JCET empty Lot ID -> :8346, and Down=false (golden oddity; true elsewhere)");
    }

    // ---- [9] VTEST -------------------------------------------------------------------------------------------------------
    std::printf("[9] VTEST\n");
    {
        Baseline(); IniConfig.bVTESTFunction = true;
        R r = Run("VTENGrecipe", "OP01");
        Check(!r.ran && Starts(r.why, "GATE (W906-E020-LI1-1)") && g_box1.empty(), "[9] VTENG lot -> GATE LI1-1 (DoPassword) refusal, first statement");
        Baseline(); IniConfig.bVTESTFunction = true;
        r = Run("", "OP01");
        Check(!r.ran && r.why == "Please Enter LotID and Operator ID!!", "[9] VTEST empty -> :8198");
        Baseline(); IniConfig.bVTESTFunction = true; RunInfo.bLotStart = true; fMesSystem->bDownloadLotInforFlag = true;
        r = Run("E020LOT9", "OP01");
        Check(!r.ran && Starts(r.why, "(golden returns without a message) VTEST") && g_box1.empty(), "[9] :8201-8205 silent return");
        Baseline(); IniConfig.bVTESTFunction = true; CUSTOMER_CODE = CC_VTEST_Shanghai;
        r = Run("E020LOT9", "OP1");
        Check(!r.ran && r.why == "OP ID 輸入小於4個字 | OP ID Length less than 4", "[9] VTEST_Shanghai OP ID < 4 -> :8213 (both texts)");
        Baseline(); IniConfig.bVTESTFunction = true; CUSTOMER_CODE = CC_VTEST_Shanghai;
        r = Run("E020LOT9", "OP01");
        Check(!r.ran && Starts(r.why, "GATE (W906-E020-LI1-7)"), "[9] VTEST_Shanghai -> GATE LI1-7 (RunModeRW) refusal");
        Baseline(); IniConfig.bVTESTFunction = true; IniConfig.bEnableRms = true; IniConfig.bCheckFile = true;
        r = Run("E020LOT9", "OP01");
        Check(!r.ran && Starts(r.why, "GATE (W906-E020-LI1-9)"), "[9] VTEST + RMS + check file -> GATE LI1-9 (MES download) refusal");
        Baseline(); IniConfig.bVTESTFunction = true;
        TrayForm.asNoRTBinFix[0] = "3"; TrayForm.asNoRTBinFix[1] = ""; TrayForm.asNoRTBinFix[2] = "";
        r = Run("E020LOT9", "OP01");
        Check(r.ran && bNoRTBinFixFlag[0] && !bNoRTBinFixFlag[1] && !bNoRTBinFixFlag[2] &&
              !fMesSystem->bNoRTBinFlag[0] && fMesSystem->bNoRTBinFlag[1] && fMesSystem->bNoRTBinFlag[2],
              "[9] no download: :8448-8454 bNoRTBinFixFlag / fMesSystem->bNoRTBinFlag from TrayForm.asNoRTBinFix");
        TrayForm.asNoRTBinFix[0] = "";
        Baseline(); IniConfig.bVTESTFunction = true; CosFunction.bUseHeadContactCount = true; IniConfig.bLifeTimeCount[0] = true;
        r = Run("E020LOT9", "OP01");
        Check(r.ran && Has(r.skipped, "GATE (W906-E020-LI1-12): golden uLotInfo.cpp:8562"), "[9] kit check (non-SECS) -> LI1-12 skipped and logged");
        Baseline(); IniConfig.bVTESTFunction = true; CosFunction.bUseHeadContactCount = true; IniConfig.bLifeTimeCount[0] = true; IniConfig.bEnable_SECS_GEM = true;
        r = Run("E020LOT9", "OP01");
        Check(r.ran && Has(r.skipped, "GATE (W906-E020-LI1-12): golden uLotInfo.cpp:8560"), "[9] kit check (SECS) -> LI1-12 pending flag skipped");
    }

    // ---- [10] PTI --------------------------------------------------------------------------------------------------------
    std::printf("[10] PTI\n");
    {
        Baseline(); CUSTOMER_CODE = CC_PTI; OFFLINE_ALARM = true; LastSet.iTester = OFF_LINE; AccessLevel = 0;
        R r = Run("E020LOT10", "OP01");
        Check(!r.ran && r.why == "Please check the tester mode is connected", "[10] PTI offline operator -> :7533");
        AnsiString dir, file;
        dir.sprintf("%s\\PTI_LotStartTrace\\%04d_%02d", as9045LogPath, SystemYear, SystemMonth);
        file.sprintf("%s\\%04d_%02d_%02d.txt", dir, SystemYear, SystemMonth, SystemDate);
        Check(ReadAll(S(file)).find("Stage=OfflineOperatorBlocked") != std::string::npos, "[10] trace written in the sandbox: " + S(file));
        Baseline(); CUSTOMER_CODE = CC_PTI; RunMode("", -1, true);
        r = Run("E020LOT10", "OP01");
        Check(r.ran && S(fLotInfo->cbRunMode->Text) == "Normal" && fLotInfo->cbRunMode->ItemIndex == 4 &&
              ReadAll(S(file)).find("Stage=NormalizeRunMode") != std::string::npos,
              "[10] PTI empty Run Mode -> \"Normal\" + ItemIndex + trace (golden :123-155)");
        Baseline(); CUSTOMER_CODE = CC_PTI; RunMode("", -1, true); IniConfig.bB03_TesterReport = true;
        r = Run("E020LOT10", "OP01");
        Check(S(fLotInfo->cbRunMode->Text) == "1'st" && fLotInfo->cbRunMode->ItemIndex == -1, "[10] [B03] -> \"1'st\" (not in the list: ItemIndex -1)");
    }

    // ---- [11] 2D sort ----------------------------------------------------------------------------------------------------
    std::printf("[11] 2D sort list (:7539-7755)\n");
    const std::string sortDir = logRoot + "\\2D_SortList";
    const std::string srcDir = logRoot + "\\E020_NetDrive";
    _mkdir(srcDir.c_str());
    {
        Baseline(); CosFunction.bSortingBy2DList = true; LastSet.iTester = _2D_SORT; TestIF_File.bSortingBy2DIDList = true;
        IniConfig.bN23UseLotInfoFile = true; IniConfig.sN23LotInfoPath = srcDir.c_str();
        R r = Run("E020LOT11", "OP01");
        Check(!r.ran && r.why == "The Lot info for 2DID sorting is missing | 找不到2DID sorting用的Lot info", "[11] N23 lot info file missing -> :7556");

        const std::string old = sortDir + "\\SortBy2DID_E020TOT.csv";
        WriteAll(old, "old");
        Baseline(); CosFunction.bSortingBy2DList = true; LastSet.iTester = _2D_SORT; TestIF_File.bSortingBy2DIDList = true;
        IniConfig.iN23DownloadMethod = 1; IniConfig.sN23DownloadDrivePath = srcDir.c_str();
        WriteAll(srcDir + "\\SortBy2DID_E020LOT11.csv", "ID1,1\r\nID2,2\r\n");
        std::remove((sortDir + "\\SortBy2DID_E020LOT11.csv").c_str());
        const bool ic = HasICUnderMachine() && HasAnyICInMachine();
        r = Run("E020LOT11", "OP01");
        Check(Exists(old) == ic, std::string("[11] :7544-7553 the previous list (sTotalLotID) ") + (ic ? "kept (IC in machine)" : "deleted (no IC)"));
        Check(Exists(sortDir + "\\SortBy2DID_E020LOT11.csv") && ReadAll(sortDir + "\\SortBy2DID_E020LOT11.csv") == "ID1,1\r\nID2,2\r\n",
              "[11] Net Drive :7571-7594 copies the list into the sandbox 2D_SortList");
        Check(!r.ran && Starts(r.why, "GATE (W906-E020-LI1-4)") && W906_ShowErrorMessage_Count == 0, "[11] list found -> GATE LI1-4 refusal (load not ported)");

        Baseline(); CosFunction.bSortingBy2DList = true; LastSet.iTester = _2D_SORT; TestIF_File.bSortingBy2DIDList = true;
        IniConfig.iN23DownloadMethod = 1; IniConfig.sN23DownloadDrivePath = srcDir.c_str();
        r = Run("E020NOFILE", "OP01");
        Check(!r.ran && r.why == "The 2DID sorting list is missing | 找不到2DID sorting list" && S(W906_ShowErrorMessage_LastCode) == "WAR1684" &&
              W906_ShowErrorMessage_LastKCode == (K_RETRY | K_SKIP) && S(W906_ShowErrorMessage_LastErrPart).find("SortBy2DID_E020NOFILE.csv") != std::string::npos,
              "[11] copy fails -> WAR1684 Retry/Skip (Skip) -> :7752-7753 missing");

        Baseline(); CosFunction.bSortingBy2DList = true; LastSet.iTester = _2D_SORT; TestIF_File.bSortingBy2DIDList = true;
        IniConfig.iN23DownloadMethod = 0;
        r = Run("E020NOFILE", "OP01");
        Check(!r.ran && Has(r.skipped, "GATE (W906-E020-LI1-3)") && r.why == "The 2DID sorting list is missing | 找不到2DID sorting list",
              "[11] FTP: LI1-3 skipped, D:\\RMS\\<lot>.csv not there -> missing");
        std::remove(old.c_str());                                            // str2 keeps this path on the manual arm
        Baseline(); CosFunction.bSortingBy2DList = true; LastSet.iTester = _2D_SORT; TestIF_File.bSortingBy2DIDList = true;
        IniConfig.iN23DownloadMethod = 3;
        r = Run("E020NOFILE", "OP01");
        Check(!r.ran && Has(r.skipped, "GATE (W906-E020-LI1-2)") && r.why == "The 2DID sorting list is missing | 找不到2DID sorting list",
              "[11] manual: LI1-2 skipped (rgSort2DID), default str2 -> missing");
        Baseline(); CosFunction.bSortingBy2DList = true; LastSet.iTester = _2D_SORT; TestIF_File.bSortingBy2DIDList = true;
        IniConfig.iN23DownloadMethod = 2; IniConfig.bSPILFunction = true;
        r = Run("E020LOT11", "");
        Check(!r.ran && r.why == "Please Enter LotID and Operator ID!!" && g_box1.size() == 1, "[11] SECS/GEM: no list step; SPIL 2D chain :8256");
        Baseline(); CosFunction.bSortingBy2DList = true; LastSet.iTester = _2D_SORT; TestIF_File.bSortingBy2DIDList = true;
        IniConfig.iN23DownloadMethod = 2; IniConfig.bSPILFunction = true; RunMode("FT", -1, true);
        r = Run("E020LOT11", "OP01");
        Check(!r.ran && r.why == "Please select the run mode in lot info!!", "[11] SPIL 2D chain: ItemIndex -1 -> :8263");
    }

    // ---- [12] 2DID allow list --------------------------------------------------------------------------------------------
    std::printf("[12] 2DID allow list (:7758-8017)\n");
    {
        Baseline(); TestIF_File.b2DIDAllowList = true; RunMode("FT", -1, true);
        R r = Run("E020LOT12", "OP01");
        Check(!r.ran && r.why == "Please select the run mode in lot info!!", "[12] ItemIndex -1 -> :7768");
        Baseline(); TestIF_File.b2DIDAllowList = true; RunMode("CORR", 2, true);
        r = Run("E020LOT12", "OP01");
        Check(r.ran && RunInfo.bLotStart, "[12] CORR skips the list (:7770) -> runs");
        const std::string search = sortDir + "\\Search2DIDByLot.txt";
        WriteAll(search, "x");
        Baseline(); TestIF_File.b2DIDAllowList = true; IniConfig.iN23DownloadMethod = 2;
        r = Run("E020LOT12", "OP01");
        Check(!Exists(search), "[12] :7772-7776 Search2DIDByLot.txt deleted (sandbox)");
        Check(!r.ran && Starts(r.why, "(golden returns without a message) 2DID allow list") && fLotInfo->btnASECL_LotStart->Down == false && g_box1.empty(),
              "[12] non-JCET, no list file -> :8011-8012 silent return, btnASECL_LotStart->Down=false");
        Baseline(); TestIF_File.b2DIDAllowList = true; IniConfig.iN23DownloadMethod = 3;
        r = Run("E020LOT12", "OP01");
        Check(!r.ran && Has(r.skipped, "GATE (W906-E020-LI1-2): golden uLotInfo.cpp:7957"), "[12] manual -> LI1-2 skipped");

        // JCET: JCETUseMakeWhite2DIDList() (golden BarCode.cpp:11340, translated locally) = the four flags
        Baseline(); CUSTOMER_CODE = CC_JCET; TestIF_File.b2DIDAllowList = true; TestIF_File.bEnableBarCode = true;
        CosFunction.bMakeWhite2DIDList = true; TestIF_File.bChkMakeWhite2DIDList = true; RunMode("FT1", 3, true);
        r = Run("ab", "OP01");
        Check(!r.ran && r.why == "批號 ab 異常，無法取得客戶代碼(前3碼)", "[12] JCET method 0, Lot ID < 3 -> :7787");
        Baseline(); CUSTOMER_CODE = CC_JCET; TestIF_File.b2DIDAllowList = true; TestIF_File.bEnableBarCode = true;
        CosFunction.bMakeWhite2DIDList = true; TestIF_File.bChkMakeWhite2DIDList = true; RunMode("FT1", 3, true);
        r = Run("E020LOT12", "OP01");
        Check(!r.ran && Starts(r.why, "GATE (W906-E020-LI1-5)"), "[12] JCET method 0 -> GATE LI1-5 (white list download) refusal");
        Baseline(); CUSTOMER_CODE = CC_JCET; TestIF_File.b2DIDAllowList = true; TestIF_File.bEnableBarCode = true;
        CosFunction.bMakeWhite2DIDList = true; TestIF_File.bChkMakeWhite2DIDList = true; RunMode("FT", 0, true);
        IniConfig.iN23DownloadMethod = 2;
        r = Run("E020LOT12", "OP01");
        Check(!r.ran && r.why == "The 2DID sorting list is missing | 找不到2DID sorting list", "[12] JCET, not FT1, no list -> :8005-8006");
        Baseline(); CUSTOMER_CODE = CC_JCET; TestIF_File.b2DIDAllowList = true; TestIF_File.bEnableBarCode = true;
        CosFunction.bMakeWhite2DIDList = true; TestIF_File.bChkMakeWhite2DIDList = true; RunMode("FT1", 3, true);
        IniConfig.iN23DownloadMethod = 2; fLotInfo->sbSECSLotEnd->Down = true;
        r = Run("E020LOT12", "OP01");
        Check(!r.ran && r.why == "Please Enter Cust. Lot ID!!" && fLotInfo->sbSECSLotEnd->Down == false,
              "[12] JCET FT1 goes on without a list; :8333-8338 Cust. Lot ID required, Down=false");
        Baseline(); CUSTOMER_CODE = CC_JCET; TestIF_File.b2DIDAllowList = true; TestIF_File.bEnableBarCode = true;
        CosFunction.bMakeWhite2DIDList = true; TestIF_File.bChkMakeWhite2DIDList = false; RunMode("FT", 0, true);
        IniConfig.iN23DownloadMethod = 2;
        r = Run("E020LOT12", "OP01");
        Check(r.ran && RunInfo.bLotStart, "[12] JCET with JCETUseMakeWhite2DIDList()==false: golden's CC_JCET arm has no else (:7999-8008) -> goes on without a list and starts");

        // SPIL else-chain (:8351-8366) needs CORR to get past the allow-list block
        Baseline(); IniConfig.bSPILFunction = true; TestIF_File.b2DIDAllowList = true; RunMode("CORR", 2, true);
        const int clr0 = W906_ClearBarcodeListCallCount;
        r = Run("", "OP01");
        Check(!r.ran && r.why == "Please Enter LotID!!" && W906_ClearBarcodeListCallCount == clr0 + 1,
              "[12] SPIL allow list: btClearBarcodeList->Click() = btClearBarcodeListClick() seat, then :8358");
    }

    // ---- [13] OEE / CSV / PANTHER / AMD / TSMC / First Tray -----------------------------------------------------------------
    std::printf("[13] gated actions and tails\n");
    {
        Baseline(); CosFunction.bOEEFunction = true;
        R r = Run("E020LOT13", "OP01");
        Check(!r.ran && Starts(r.why, "GATE (W906-E020-LI1-10)") && !RunInfo.bLotStart, "[13] OEE -> GATE LI1-10 refusal");
        Baseline(); TestIF_File.bEnableBarcodeCSVCompare = true;
        r = Run("E020LOT13", "OP01");
        Check(r.ran && r.skipped.empty(), "[13] CSV compare on -> nothing skipped (E-030: V912 :8397 ResetBarcodeCSVForLotStart is 912-only; golden 906 goes straight to VTEST)");
        Baseline(); CUSTOMER_CODE = CC_PANTHER; LastSet.SystemAccSecond[0][0] = 11; LastSet.SystemAccSecond[0][7] = 12; LastSet.SystemAccSecond[1][0] = 13;
        r = Run("E020LOT13", "OP01");
        Check(r.ran && Has(r.skipped, "GATE (W906-E020-LI1-11)") && LastSet.SystemAccSecond[0][0] == 0 && LastSet.SystemAccSecond[0][7] == 0 &&
              LastSet.SystemAccSecond[1][0] == 13, "[13] PANTHER: LI1-11 skipped, :8533-8534 SystemAccSecond[0][0..7]=0 (row 1 untouched)");
        Baseline(); CUSTOMER_CODE = CC_Greatek; IniConfig.bAMDFunction = true; iAMD_Function = 2;
        r = Run("", "");
        Check(r.ran && Has(r.skipped, "GATE (W906-E020-LI1-13)"), "[13] AMD (empty lot keeps edtSysLotID enabled) -> LI1-13 skipped");
        Baseline(); IniConfig.bAMDFunction = true; iAMD_Function = 2;
        r = Run("E020LOT13", "OP01");
        Check(r.ran && !Has(r.skipped, "LI1-13"), "[13] AMD after a real lot start: edtSysLotID disabled -> golden :8579 never records (golden oddity)");
        Baseline(); CUSTOMER_CODE = CC_TSMC_TAINAN; IniConfig.bEnable_SECS_GEM = true;
        r = Run("E020LOT13", "OP01");
        Check(r.ran && !g_box1.empty() && g_box1.back() == "Please pressed Start , RUN !!", "[13] TSMC_TAINAN + SECS -> :8569 box after SetLotStart");
        Baseline(); CosFunction.bFirstTrayCheckOnUnloader = true; IniConfig.bP62FirstTrayCheckOnUnloader = true; IniConfig.bP62AlwaysEnabledAtLotStart = true;
        r = Run("E020LOT13", "OP01");
        Check(r.ran && fLotInfo->cbFirstTrayCheckOnUnloader->Checked, "[13] :8572-8577 SetFirstTrayCheckOnUnloader");
    }

    // ---- [14] source ratchets ---------------------------------------------------------------------------------------------
    std::printf("[14] source ratchets\n");
    {
        const char* srcEnv = std::getenv("W906_E020_SRC_ROOT");                  // control run: a broken copy
        const std::string base = (srcEnv && *srcEnv) ? std::string(srcEnv) : root;
        const std::string src = ReadAll(base + "/FileRW/LotInfo_SECSLotStart.cpp");
        Check(!src.empty(), "[14] read " + base + "/FileRW/LotInfo_SECSLotStart.cpp");
        // the definition must be outside every #if 0 (linear depth scan, like tools/start_sites_census.py)
        std::istringstream is(src);
        std::string line;
        int depth = 0, zeroDepth = 0, defLive = 0, defDead = 0;
        std::vector<int> zeroStack;
        while (std::getline(is, line)) {
            size_t p = line.find_first_not_of(" \t");
            const std::string t = p == std::string::npos ? "" : line.substr(p);
            if (Starts(t, "#if")) { ++depth; const bool z = Starts(t, "#if 0"); zeroStack.push_back(z ? 1 : 0); if (z) ++zeroDepth; continue; }
            if (Starts(t, "#endif")) { if (!zeroStack.empty()) { if (zeroStack.back()) --zeroDepth; zeroStack.pop_back(); } --depth; continue; }
            if (Starts(t, "#else") && !zeroStack.empty()) { if (zeroStack.back()) { --zeroDepth; zeroStack.back() = 0; } continue; }
            if (Starts(t, "bool W906_LotInfo_SECSLotStart(std::string* whyNot, std::vector<std::string>* skipped)")) { if (zeroDepth) ++defDead; else ++defLive; }
        }
        Check(defLive == 1 && defDead == 0 && depth == 0, "[14] W906_LotInfo_SECSLotStart is defined once, live (not under #if 0)");
        for (int g = 1; g <= 13; ++g) {
            char tag[64]; std::snprintf(tag, sizeof(tag), "#if 0 // GATE (W906-E020-LI1-%d)", g);
            if (g == 8) { Check(src.find(tag) == std::string::npos, std::string("[14] ") + tag + " absent (E-030: 912-only, removed)"); continue; }
            Check(src.find(tag) != std::string::npos, std::string("[14] ") + tag + " present");
        }
        Check(src.find("bEnableBarcodeCSVCompare") == std::string::npos && src.find("ResetBarcodeCSVForLotStart(") == std::string::npos,
              "[14] no V912-only Barcode CSV Compare use in the file (E-030)");
        Check(src.find("fMain->Start" "(") == std::string::npos && src.find("SoftStart") == std::string::npos, "[14] no START path in the file (census unchanged)");
        const std::string cm = ReadAll(base + "/CMakeLists.txt");
        std::istringstream cs(cm);
        bool listed = false, found = false;
        while (std::getline(cs, line)) {
            if (line.find("FileRW/MainClick.cpp") == std::string::npos || line.find("FileRW/CfgTrayPlate.cpp") == std::string::npos) continue;
            found = true;
            const std::string code = line.substr(0, line.find('#'));
            if (code.find("FileRW/LotInfo_SECSLotStart.cpp") != std::string::npos) listed = true;
        }
        Check(found && listed, "[14] FileRW/LotInfo_SECSLotStart.cpp is on St01's wb_serve source line (CMakeLists.txt), before its '#'");
    }

    // ---- [15] real files -------------------------------------------------------------------------------------------------
    std::printf("[15] real files untouched\n");
    for (int i = 0; kRealFiles[i]; ++i)
        Check(SameMeta(realMeta[i], MetaOf(kRealFiles[i])), std::string("[15] ") + kRealFiles[i] + " (size + mtime)");
    for (int i = 0; kRealDirs[i]; ++i)
        Check(realList[i] == Listing(kRealDirs[i]), std::string("[15] ") + kRealDirs[i] + "\\* listing");

    W906_ShowMyMessage_Hook = 0;
    std::printf("%s: %d passed, %d failed\n", g_fail ? "FAILED" : "PASSED", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
