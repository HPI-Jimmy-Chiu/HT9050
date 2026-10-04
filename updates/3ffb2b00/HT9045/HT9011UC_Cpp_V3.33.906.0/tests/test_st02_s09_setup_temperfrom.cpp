// =============================================================================
//  test_st02_s09_setup_temperfrom.cpp -- AI(W906-ST02-S09L) 20261003 (St02-E, helper)
//
//  S-09 tsv 540: cSetUp.cpp:1115-1117 GATE(G-SU-TemperFrom) lifted.  golden 0618 cSetUp.cpp:2852 (0625_Steven same;
//  912 cSetUp.cpp:2874 is the same statement), inside TfSetup::ReadFile:
//      if(INSTALL_OCR!=eocrUninstal) { fTemperFrom->SetShowYield(fTemperFrom->esytOcrOn, TestIF_File.bOcrFunction); ... }
//  The port adds a NULL guard (forms/fTemperFrom.h:400-406): fTemperFrom starts NULL (cTemperFrom.cpp:1458) and is created by
//  W906_TemperFromBootShow (cTemperFrom.cpp:1989, tools/wb_serve.cpp:4162), which runs AFTER the boot ReadFile
//  (tools/wb_serve.cpp:3202 / :3270).  This test runs the REAL TfSetup::ReadFile (fSetup->Init() first, as wb_serve.cpp:3198)
//  against a HandlerCondition.Data seeded at the path ReadFile itself reads (GetRecipeFileName, DataPath + GetLastOpenFN()):
//    [1] fTemperFrom==NULL, OCR installed, [Configuration] OCR Function=1: ReadFile runs to its end (IniConfig.bShuttleMode50
//        sentinel cleared at its last statement), TestIF_File.bOcrFunction==true (golden 0618 cSetUp.cpp:2568 read), no crash, form still NULL
//    [2] the form built the way the port builds it (W906_TemperFromBootShow -> ctor + FormClose + FormShow;
//        IniConfig.bShowFunctionWindow on so FormShow shows the OCR slot, golden 0618 cTemperFrom.cpp:128):
//        OCR Function=1 -> strShowYield[esytOcrOn].OnOff true, palOcrOn "OCR On" / clLime (golden 0618 cTemperFrom.cpp:1473 / :1486);
//        OCR Function=0 -> OnOff false, "OCR Off" / clRed
//   [2b] S-09 boot follow-up (AI(W906-ST02-S09B) 20261004): right after the boot show -- before any further ReadFile -- the
//        OCR slot already shows what the boot ReadFile of [1] read (OCR=1 -> OnOff true, "OCR On"): cTemperFrom.cpp:1992 replays
//        golden 0618 cSetUp.cpp:2850-2852 once after the port's CreateForm (golden creates the form before any ReadFile,
//        HT9045.cpp:183 / :185).  REVERSE: drop the replay on :1992 -> [2b] red (OnOff stays the ctor's, caption "OCR Off").
//    [3] OCR not installed (INSTALL_OCR==eocrUninstal): ReadFile forces bOcrFunction=false (golden 0618 cSetUp.cpp:2565-2566) and does NOT call
//        SetShowYield (golden 0618 cSetUp.cpp:2850) -- the slot keeps the value it had
//    [5] the same boot order for OCR=0 (OnOff false, "OCR Off") and OCR not installed (slot hidden, caption "", golden
//        FormShow :1497-1498); each case builds a fresh form (fTemperFrom reset to NULL, the earlier form left alive).
//        REVERSE: replay with `true` instead of TestIF_File.bOcrFunction -> [5] OCR=0 red.
//    [4] machine files: D:\HT9045\system\Gerneral.ini, D:\HT9045\config\config.ini and D:\HT9045\SetUp.inf byte-identical
//        before and after (the RTC config.ini write at cSetUp.cpp:1005 is kept off: IniConfig.bRTCbySystem=false)
//  CONTAINMENT: refuses (exit 2) unless ctest's redirect roots are in force (st02_test_containment.h, w906_ctest_guard.h) and
//  DataPath / LastDataPath sit in ctest's scratch.  Writes only <DataPath>ST02_S09L_TEMPERFROM\HandlerCondition.Data and
//  <DataPath>ST02_S09L_SetUp.inf (deleted when green).  Memory otherwise; no hardware, no IO table.
// =============================================================================
#include "st02_test_containment.h"   // W906TestInsideCtestRoots (includes common.h)
#include "w906_ctest_guard.h"        // W906TestRequireCtestRedirects / W906CtestGuardInScratch
#include "forms/fSetup.h"            // fSetup, TfSetup::Init / ReadFile (cSetUp.cpp)
#include "forms/fTemperFrom.h"       // fTemperFrom, TfTemperFrom::strShowYield / esytOcrOn / palOcrOn
#include "MachineType.h"             // eocrUninstal / eocrUseMessage
#include "cmydef.h"                  // INSTALL_OCR, REAL_TIME_CCD, InitialOK, bSystemClose, ATC_SYSTEM ...
#include "cprod.h"                   // TestIF_File
#include "Config.h"                  // IniConfig
#include "CosFunction.h"             // CosFunction
#include "common.h"                  // DataPath, LastDataPath, GetRecipePath, GetRecipeFileName, MyForceDirectories, CloseIniFile

#include <windows.h>
#include <cstdio>
#include <string>

void W906_TemperFromBootShow();      // cTemperFrom.cpp:1989 (no header; same declaration as tests/test_temperfrom_timer1.cpp)

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_st02_s09_setup_temperfrom.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static const char* const kRecipe="ST02_S09L_TEMPERFROM";

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

// The real source ReadFile reads: [Configuration] "OCR Function" in <DataPath><recipe>\HandlerCondition.Data (cSetUp.cpp:813).
// The cached INIFile (common.cpp:561 OpenIniFile) is closed first, so the next ReadIniData opens the file as written here.
static bool SeedOcr(const std::string& file, int ocr)
{
    CloseIniFile();
    char body[128];
    std::snprintf(body, sizeof(body), "[Configuration]\r\nOCR Function=%d\r\n", ocr);
    return WriteText(file, body);
}

// One ReadFile; returns true when it reached its last statement (cSetUp.cpp:1283-1294 clears bShuttleMode50 with RTC off).
static bool RunReadFile()
{
    IniConfig.bShuttleMode50=true;
    fSetup->ReadFile();
    return IniConfig.bShuttleMode50==false;
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("test_st02_s09_setup_temperfrom (S-09 tsv 540): TfSetup::ReadFile -> fTemperFrom->SetShowYield(esytOcrOn)\n");
    if(!W906TestInsideCtestRoots("St02_S09SetupTemperFrom"))
        return 2;

    // ---- containment: the recipe dir and SetUp.inf live under DataPath, which must be ctest's scratch -----------------
    const std::string sDataPath=DataPath.c_str();
    const std::string sInf=sDataPath+"ST02_S09L_SetUp.inf";
    LastDataPath=AnsiString(sInf.c_str());                                     // GetLastOpenFN (common.cpp:1446) reads it
    {
        const char* const rt[]={ "DataPath", sDataPath.c_str(), "LastDataPath", sInf.c_str(), 0 };
        if(!W906TestRequireCtestRedirects("St02_S09SetupTemperFrom", rt))
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
    const std::string sFile=GetRecipeFileName("HandlerCondition.Data").c_str();
    std::printf("  HandlerCondition.Data = %s\n", sFile.c_str());
    if(!W906CtestGuardInScratch(sFile.c_str()) || sFile.find(kRecipe)==std::string::npos)
    {
        std::printf("  REFUSED: the recipe file is not in ctest's scratch / not ours\n");
        return 2;
    }

    // ---- the state ReadFile and the boot show expect (kept away from every write but the recipe file) ----------------
    InitialOK=false;                       // cSetUp.cpp:1048 / InitShuttleThreadParameter early return (cinitial.cpp:12512)
    bSystemClose=false;
    REAL_TIME_CCD=false;                   // RTC branch (cSetUp.cpp:1071-1102) off
    CCD2_TEMPER=false;
    Index_ESDAir=false;
    ATC_SYSTEM=eATCUninstall;
    IniConfig.bRTCbySystem=false;          // no config.ini write (cSetUp.cpp:1005)
    CosFunction.bLockRTCByFile=false;      // no RTC write-back (cSetUp.cpp:1005 / :1025)
    CosFunction.bLockRTC=false;
    IniConfig.bEnableAutoCleanFunction=false;
    IniConfig.bShowFunctionWindow=true;    // FormShow shows the function strip (cTemperFrom.cpp:1487-1508)
    fSetup->fShow=false;                   // "external control" arm (cSetUp.cpp:1133)
    fSetup->Init();                        // as tools/wb_serve.cpp:3198 (tSiteMap; golden 0618 cSetUp.cpp:123-216)

    // ---- [1] no form yet ----------------------------------------------------------------------------------------------
    std::printf("[1] fTemperFrom==NULL (before the boot show)\n");
    CHECK(fTemperFrom==NULL, "[1] fTemperFrom is NULL before the boot show (cTemperFrom.cpp:1458)");
    INSTALL_OCR=eocrUseMessage;
    CHECK(SeedOcr(sFile, 1), "[1] seed OCR Function=1");
    TestIF_File.bOcrFunction=false;
    CHECK(RunReadFile(), "[1] ReadFile ran to its last statement with fTemperFrom NULL (no crash at :1116)");
    CHECK(TestIF_File.bOcrFunction==true, "[1] ReadFile read OCR Function=1 from the seeded recipe (cSetUp.cpp:813)");
    CHECK(fTemperFrom==NULL, "[1] ReadFile does not create the form");

    // ---- [2] the form, built the port's way -----------------------------------------------------------------------------
    std::printf("[2] after W906_TemperFromBootShow\n");
    W906_TemperFromBootShow();
    CHECK(fTemperFrom!=NULL, "[2] the boot show creates fTemperFrom");
    if(fTemperFrom==NULL) { std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail); return 1; }
    TfTemperFrom* f=fTemperFrom;
    SHOW_YIELD_TYPE& ocr=f->strShowYield[TfTemperFrom::esytOcrOn];
    CHECK(ocr.UsePanel==f->palOcrOn, "[2] esytOcrOn slot uses palOcrOn (golden 0618 cTemperFrom.cpp:104-116)");
    CHECK(ocr.bShow==true, "[2] FormShow shows the OCR slot when OCR is installed (golden FormShow, bShowFunctionWindow)");
    CHECK(ocr.OnOff==true, "[2b] boot order: the boot ReadFile's OCR=1 reaches the new form (cTemperFrom.cpp:1992 replays golden 0618 cSetUp.cpp:2852)");
    CHECK(f->palOcrOn->Caption=="OCR On" && f->palOcrOn->Color==0x0000FF00, "[2b] boot order: palOcrOn 'OCR On' / clLime straight after the boot show");

    ocr.OnOff=false;
    CHECK(SeedOcr(sFile, 1), "[2] seed OCR Function=1");
    CHECK(RunReadFile(), "[2] ReadFile (OCR=1) ran to its end");
    CHECK(ocr.OnOff==true, "[2] OCR=1 -> SetShowYield(esytOcrOn, true): OnOff true (golden 0618 cSetUp.cpp:2852)");
    CHECK(f->palOcrOn->Caption=="OCR On" && f->palOcrOn->Color==0x0000FF00, "[2] OCR=1 -> palOcrOn 'OCR On' / clLime (ShowYieldFuntion)");

    CHECK(SeedOcr(sFile, 0), "[2] seed OCR Function=0");
    CHECK(RunReadFile(), "[2] ReadFile (OCR=0) ran to its end");
    CHECK(TestIF_File.bOcrFunction==false, "[2] OCR=0 read");
    CHECK(ocr.OnOff==false, "[2] OCR=0 -> SetShowYield(esytOcrOn, false): OnOff false");
    CHECK(f->palOcrOn->Caption=="OCR Off" && f->palOcrOn->Color==0x000000FF, "[2] OCR=0 -> palOcrOn 'OCR Off' / clRed (not CC_ASE_M)");

    // ---- [3] OCR not installed: no SetShowYield --------------------------------------------------------------------------
    std::printf("[3] INSTALL_OCR==eocrUninstal\n");
    INSTALL_OCR=eocrUninstal;
    ocr.OnOff=true;                        // sentinel: golden 0618 cSetUp.cpp:2850 skips the call, so it must survive
    CHECK(SeedOcr(sFile, 1), "[3] seed OCR Function=1");
    CHECK(RunReadFile(), "[3] ReadFile ran to its end");
    CHECK(TestIF_File.bOcrFunction==false, "[3] OCR not installed -> bOcrFunction forced false (cSetUp.cpp:810-811)");
    CHECK(ocr.OnOff==true, "[3] OCR not installed -> SetShowYield not called (golden 0618 cSetUp.cpp:2850), the slot keeps its value");
    INSTALL_OCR=eocrUseMessage;

    // ---- [5] boot order, OCR=0 and OCR not installed (S-09 boot follow-up) -------------------------------------------
    std::printf("[5] boot order: ReadFile with fTemperFrom NULL, then the boot show\n");
    TfTemperFrom* const fFirst=fTemperFrom;            // left alive: nothing in this test drives it any more
    fTemperFrom=NULL;
    CHECK(SeedOcr(sFile, 0), "[5] seed OCR Function=0");
    CHECK(RunReadFile(), "[5] ReadFile (OCR=0, form NULL) ran to its end");
    CHECK(TestIF_File.bOcrFunction==false, "[5] OCR=0 read");
    W906_TemperFromBootShow();
    CHECK(fTemperFrom!=NULL && fTemperFrom!=fFirst, "[5] the boot show built a fresh form");
    if(fTemperFrom!=NULL && fTemperFrom!=fFirst)
    {
        SHOW_YIELD_TYPE& o0=fTemperFrom->strShowYield[TfTemperFrom::esytOcrOn];
        CHECK(o0.bShow==true, "[5] OCR=0: the slot is shown (OCR installed)");
        CHECK(o0.OnOff==false, "[5] OCR=0: the boot ReadFile's value reaches the new form (OnOff false)");
        CHECK(fTemperFrom->palOcrOn->Caption=="OCR Off" && fTemperFrom->palOcrOn->Color==0x000000FF, "[5] OCR=0: palOcrOn 'OCR Off' / clRed straight after the boot show");
    }
    TfTemperFrom* const fSecond=fTemperFrom;
    INSTALL_OCR=eocrUninstal;
    fTemperFrom=NULL;
    CHECK(SeedOcr(sFile, 1), "[5] seed OCR Function=1 (OCR not installed)");
    CHECK(RunReadFile(), "[5] ReadFile (not installed, form NULL) ran to its end");
    CHECK(TestIF_File.bOcrFunction==false, "[5] not installed -> bOcrFunction forced false");
    W906_TemperFromBootShow();
    CHECK(fTemperFrom!=NULL && fTemperFrom!=fSecond, "[5] the boot show built a fresh form (not installed)");
    if(fTemperFrom!=NULL && fTemperFrom!=fSecond)
    {
        CHECK(fTemperFrom->strShowYield[TfTemperFrom::esytOcrOn].bShow==false, "[5] not installed: the OCR slot is hidden (golden FormShow :1497)");
        CHECK(fTemperFrom->palOcrOn->Caption=="", "[5] not installed: palOcrOn caption empty (golden FormShow :1498)");
    }
    INSTALL_OCR=eocrUseMessage;

    // ---- [4] machine files --------------------------------------------------------------------------------------------
    CloseIniFile();
    CHECK(Slurp("D:\\HT9045\\system\\Gerneral.ini")==sGen0, "[4] Gerneral.ini unchanged");
    CHECK(Slurp("D:\\HT9045\\config\\config.ini")==sCfg0, "[4] config.ini unchanged");
    CHECK(Slurp("D:\\HT9045\\SetUp.inf")==sInf0, "[4] SetUp.inf unchanged");

    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    if(g_fail==0)
    {
        ::DeleteFileA(sFile.c_str());
        ::RemoveDirectoryA(GetRecipePath().c_str());
        ::DeleteFileA(sInf.c_str());
    }
    return g_fail==0 ? 0 : 1;
}
