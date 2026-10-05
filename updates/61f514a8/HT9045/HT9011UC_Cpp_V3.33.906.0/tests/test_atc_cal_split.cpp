// =============================================================================
//  test_atc_cal_split.cpp  --  AI(W906-I08) 20261004 (Ifor01)
//
//  I-08 (docs/handoff/TO_IFOR.md §4 1003 14:3x; RULINGS_20261003 #22 (#47 = A) and #1): the ATC ambient calibration file is
//  split cold / hot as in V912 (uTemp_Set.cpp:3005-3012 read, :4467-4474 write). With eNewATCSystem + bATCActiveCooling +
//  CosFunction.bATCUseTempAdjustment, ReadTempFile and spbSaveClick use DefineTemp\Temperature_ATC.Data for Hot / AmbientHot
//  and DefineTemp\Temperature_ATC_Cold.Data for every other mode -- the same split the web temperature page already had
//  (FileRW/Temperature.gen.inc:4034-4041 / :5532-5539). golden 0618 (uTemp_Set.cpp:2976 / :4428) had one _ATC file.
//
//    [1] read, Hot (recipe [Mode] Mode=0)        -> [Low OffSet] Base from _ATC      (11.0)
//    [2] read, AmbientHot (Mode=3)               -> _ATC                               (11.0)
//    [3] read, Ambient (Mode=1)                  -> _ATC_Cold                          (22.0)
//    [4] read, Ambient, _ATC_Cold missing        -> copied from Temperature.Data (V912 / golden copy-on-missing), read 33.0
//    [5] ATC cooling off                         -> Temperature.Data (33.0) in any mode (the golden path, unchanged)
//    [6] save, Ambient: neither _ATC file exists -> spbSaveClick creates _ATC_Cold only
//    [7] save, Hot:     neither _ATC file exists -> spbSaveClick creates _ATC only
//    [8] source pin: uTemp_Set.cpp and FileRW/Temperature.gen.inc both carry the _ATC_Cold branch twice (read + write), so
//        the two routes cannot drift apart again (argv[1] = the port root, read only)
//    [9] the machine's own files are untouched (D:\HT9045\IniData\DefineTemp\*.Data, config\ATC.ini, system\Gerneral.ini)
//
//  Sandbox: %TEMP%\ht9045_atccal_<tick>\ (DataPath / DefaultPath / OffsetPath / LastDataPath are pointed there before any call;
//  AuthPath and asGeneralPath follow ctest's scratch). Refuses to run outside ctest's redirect roots (st02_test_containment.h),
//  as test_settemp_save.cpp. The sandbox is removed on a green run.
//  Reverse check (RULINGS_20261003 #15): put either uTemp_Set.cpp site back to the one golden 0618 _ATC file => [3] / [4]
//  (read) or [6] (write) go red.
// =============================================================================
#include "forms/fTemp_Set.h"
#include "forms/fMain.h"
#include "common.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "MachineType.h"
#include "acarry_shims.h"     // ATC_InterfaceForm
#include "st02_test_containment.h"
#include "canary_support.h"   // W906_ShowMyMessage_Count (the watchdog report)
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_atc_cal_split.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string g_root;

// AI(W906-I08) 20261005 (Ifor01), W-59: the laptop's SIM gate ran this test into its 120 s limit at [6] (spbSaveClick, Ambient)
//   while IFOR-NB2 runs it in ~10 s in both builds. So every ReadTempFile / spbSaveClick is timed, and a watchdog thread reports a
//   step that runs past 30 s (every 15 s: which step, how long, how many ShowMyMessage calls so far -- in a test executable
//   ShowMyMessage only prints, canary_support.cpp:157, so a dialog cannot be what waits). A hang now leaves its place in the log.
static const char* volatile g_step = "";
static volatile DWORD g_stepT0 = 0;
static DWORD WINAPI Watchdog(LPVOID)
{
    DWORD lastReport = 0;
    for(;;)
    {
        ::Sleep(1000);
        const char* s = g_step;
        if(!s || !*s) { lastReport = 0; continue; }
        const DWORD el = ::GetTickCount() - g_stepT0;
        if(el > 30000 && (lastReport == 0 || ::GetTickCount() - lastReport >= 15000))
        {
            std::printf("  WATCHDOG: \"%s\" still running after %lu s (ShowMyMessage calls so far: %d)\n",
                        s, (unsigned long)(el / 1000), W906_ShowMyMessage_Count);
            std::fflush(stdout);
            lastReport = ::GetTickCount();
        }
    }
    return 0;
}
static void StepBegin(const char* s) { g_stepT0 = ::GetTickCount(); g_step = s; }
static void StepEnd()
{
    const DWORD el = ::GetTickCount() - g_stepT0;
    std::printf("    (%s: %lu ms)\n", g_step, (unsigned long)el);
    g_step = "";
}

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static void Spit(const std::string& p, const std::string& s)
{
    FILE* f=std::fopen(p.c_str(), "wb");
    if(f) { std::fwrite(s.data(), 1, s.size(), f); std::fclose(f); }
}
static bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str())!=INVALID_FILE_ATTRIBUTES; }
static bool UnderMachineTree(const std::string& p)
{
    std::string l=W906TestSafeLower(p.c_str());
    return l.find("d:\\ht9045\\")==0 || l.find("d:/ht9045/")==0;
}
static int Count(const std::string& hay, const std::string& needle)
{
    int n=0;
    for(size_t p=hay.find(needle); p!=std::string::npos; p=hay.find(needle, p+needle.size())) n++;
    return n;
}
struct Stamp { bool there; DWORD size; FILETIME t; };
static Stamp StampOf(const std::string& p)
{
    Stamp s; std::memset(&s, 0, sizeof(s));
    WIN32_FILE_ATTRIBUTE_DATA a;
    if(::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &a)) { s.there=true; s.size=a.nFileSizeLow; s.t=a.ftLastWriteTime; }
    return s;
}
static bool SameStamp(const Stamp& a, const Stamp& b)
{
    return a.there==b.there && a.size==b.size && a.t.dwLowDateTime==b.t.dwLowDateTime && a.t.dwHighDateTime==b.t.dwHighDateTime;
}

static std::string g_recipe, g_base, g_atc, g_cold;
static std::string Cal(double base) { char b[96]; std::snprintf(b, sizeof(b), "[Mode]\r\nPoints=1\r\n[Low OffSet]\r\nBase=%.1f\r\n", base); return b; }
// ReadTempFile reads Temperature.bATCActiveCooling from the recipe ([ATC] Active Cooling, uTemp_Set.cpp:2974 / :2985), so the
// ATC switch goes into the recipe, as on a machine -- setting the global alone is overwritten by the read.
static void Recipe(int mode, int cooling=1) { char b[160]; std::snprintf(b, sizeof(b), "[Mode]\r\nMode=%d\r\nTemperature=50.0\r\n[Time]\r\nSoak=10.0\r\n[ATC]\r\nActive Cooling=%d\r\n", mode, cooling); Spit(g_recipe, b); }
static void ResetCalFiles() { Spit(g_base, Cal(33.0)); Spit(g_atc, Cal(11.0)); Spit(g_cold, Cal(22.0)); }
// what ReadTempFile saw, printed after each read (diagnostics for a red run)
static void Show(const char* tag)
{
    std::printf("    %s: fLowBase=%.1f iTemperature=%d ATC_SYSTEM=%d(eNewATCSystem=%d) bATCActiveCooling=%d bATCUseTempAdjustment=%d\n",
                tag, Temperature.fLowBase, LastSet.iTemperature, ATC_SYSTEM, (int)eNewATCSystem,
                (int)Temperature.bATCActiveCooling, (int)CosFunction.bATCUseTempAdjustment);
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("AtcCalSplit\n");
    ::CreateThread(NULL, 0, Watchdog, NULL, 0, NULL);
    if(!W906TestInsideCtestRoots("AtcCalSplit"))
        return 2;
    const std::string port=(argc>1) ? argv[1] : "";

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root=std::string(tmp)+"ht9045_atccal_"+stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root+"\\Data").c_str(), 0);
    ::CreateDirectoryA((g_root+"\\Data\\STR").c_str(), 0);
    ::CreateDirectoryA((g_root+"\\DefineTemp").c_str(), 0);
    ::CreateDirectoryA((g_root+"\\Offset").c_str(), 0);
    Spit(g_root+"\\SetUp.inf", "STR\r\n");
    g_recipe=g_root+"\\Data\\STR\\Temperature.Data";
    g_base  =g_root+"\\DefineTemp\\Temperature.Data";
    g_atc   =g_root+"\\DefineTemp\\Temperature_ATC.Data";
    g_cold  =g_root+"\\DefineTemp\\Temperature_ATC_Cold.Data";
    ResetCalFiles();
    Recipe(0);

    DataPath    =AnsiString((g_root+"\\Data\\").c_str());
    DefaultPath =AnsiString((g_root+"\\").c_str());
    OffsetPath  =AnsiString((g_root+"\\Offset\\").c_str());
    LastDataPath=AnsiString((g_root+"\\SetUp.inf").c_str());
    const bool authScratch=W906TestSafeLower(AuthPath.c_str()).find("scratch")!=std::string::npos;
    const bool genScratch=W906TestSafeLower(asGeneralPath.c_str()).find("scratch")!=std::string::npos;
    if(UnderMachineTree(DataPath.c_str()) || UnderMachineTree(DefaultPath.c_str()) || UnderMachineTree(LastDataPath.c_str()) ||
       !authScratch || !genScratch)
    {
        std::printf("  ABORT: a path is not in the sandbox (Data %s, Default %s, LastData %s, Auth %s, General %s) -- nothing was called\n",
                    DataPath.c_str(), DefaultPath.c_str(), LastDataPath.c_str(), AuthPath.c_str(), asGeneralPath.c_str());
        return 2;
    }

    const char* const kMachine[]={ "D:\\HT9045\\IniData\\DefineTemp\\Temperature.Data", "D:\\HT9045\\IniData\\DefineTemp\\Temperature_ATC.Data",
                                   "D:\\HT9045\\IniData\\DefineTemp\\Temperature_ATC_Cold.Data", "D:\\HT9045\\config\\ATC.ini",
                                   "D:\\HT9045\\system\\Gerneral.ini" };
    const int kN=(int)(sizeof(kMachine)/sizeof(kMachine[0]));
    Stamp before[kN];
    for(int i=0; i<kN; i++) before[i]=StampOf(kMachine[i]);

    OpenGeneralIniFile();   // as test_settemp_save.cpp: SaveLastSetIni (spbSaveClick) writes through INIFileGeneral
    CHECK(fMain!=0, "fMain exists (the facade's global)");
    if(fMain==0) { std::printf("FAIL: no fMain\n"); return 1; }
    TfTemp_Set* const oldTempSet=fTemp_Set;
    fTemp_Set=new TfTemp_Set();
    fTemp_Set->Init();
    SystemStart=false;
    IniConfig.bEnable_SECS_GEM=false;
    IniConfig.bA02DisableSaveParsWhenSwitchToOp=false;
    CosFunction.bSaveTemperatureByMachine=false;
    CosFunction.bNotClearAllHotBuffer=false;
    CosFunction.bTempCalByRecipe=false;
    CosFunction.bATCUseTempAdjustment=true;
    ATC_SYSTEM=eNewATCSystem;
    ATC_InterfaceForm->iATC_MODE_TYPE=0;            // not ATC_TYPE_33 / 35: no warm-up guard in spbSaveClick
    Temperature.bATCActiveCooling=true;                     // re-read from each recipe by ReadTempFile (see Recipe())
    for(int i=0; i<tcTotalCount; i++) bUT150Install[i]=false;   // no CheckTempOffset limits in spbSaveClick

    std::printf("[1] read, Hot -> _ATC\n");
    Recipe(0); StepBegin("ReadTempFile"); fTemp_Set->ReadTempFile(true); StepEnd(); Show("read");
    CHECK(LastSet.iTemperature==Tempture_Hot, "[1] [Mode] Mode=0 -> Tempture_Hot");
    CHECK(Temperature.bATCActiveCooling==true, "[1] recipe [ATC] Active Cooling=1 read back");
    CHECK(Temperature.fLowBase==11.0, "[1] Hot reads Temperature_ATC.Data (Base 11.0)");

    std::printf("[2] read, AmbientHot -> _ATC\n");
    Recipe(3); StepBegin("ReadTempFile"); fTemp_Set->ReadTempFile(true); StepEnd(); Show("read");
    CHECK(LastSet.iTemperature==Tempture_AmbientHot, "[2] [Mode] Mode=3 -> Tempture_AmbientHot");
    CHECK(Temperature.fLowBase==11.0, "[2] AmbientHot reads Temperature_ATC.Data (Base 11.0)");

    std::printf("[3] read, Ambient -> _ATC_Cold\n");
    Recipe(1); StepBegin("ReadTempFile"); fTemp_Set->ReadTempFile(true); StepEnd(); Show("read");
    CHECK(LastSet.iTemperature==Tempture_Ambient, "[3] [Mode] Mode=1 -> Tempture_Ambient");
    CHECK(Temperature.fLowBase==22.0, "[3] Ambient reads Temperature_ATC_Cold.Data (Base 22.0), not the hot _ATC file (V912 :3011)");

    std::printf("[4] read, Ambient, _ATC_Cold missing -> copied from Temperature.Data\n");
    ::DeleteFileA(g_cold.c_str());
    StepBegin("ReadTempFile"); fTemp_Set->ReadTempFile(true); StepEnd(); Show("read");
    CHECK(Exists(g_cold), "[4] Temperature_ATC_Cold.Data created");
    CHECK(Slurp(g_cold)==Slurp(g_base), "[4] ... as a copy of Temperature.Data (V912 :3014-3018)");
    CHECK(Temperature.fLowBase==33.0, "[4] and read: Base 33.0");
    CHECK(Slurp(g_atc)==Cal(11.0), "[4] the hot _ATC file is untouched");

    std::printf("[5] ATC cooling off (recipe [ATC] Active Cooling=0) -> Temperature.Data in any mode\n");
    ResetCalFiles();
    Recipe(1, 0); StepBegin("ReadTempFile"); fTemp_Set->ReadTempFile(true); StepEnd(); Show("read");
    CHECK(Temperature.fLowBase==33.0, "[5] Ambient, cooling off: Temperature.Data (Base 33.0)");
    Recipe(0, 0); StepBegin("ReadTempFile"); fTemp_Set->ReadTempFile(true); StepEnd(); Show("read");
    CHECK(Temperature.fLowBase==33.0, "[5] Hot, cooling off: Temperature.Data (Base 33.0)");

    std::printf("[6] save, Ambient -> creates _ATC_Cold only\n");
    Recipe(1); StepBegin("ReadTempFile"); fTemp_Set->ReadTempFile(true); StepEnd(); Show("read");
    fTemp_Set->rgTemperatureMode->ItemIndex=1;
    ::DeleteFileA(g_atc.c_str()); ::DeleteFileA(g_cold.c_str());
    LastSet.iTemperature=Tempture_Ambient;
    StepBegin("spbSaveClick"); fTemp_Set->spbSaveClick(NULL); StepEnd();   // NULL as forms/fMain_Heater.cpp:2439 (the facade is no TObject)
    CHECK(Exists(g_cold), "[6] Ambient save wrote Temperature_ATC_Cold.Data (V912 :4467-4474)");
    CHECK(!Exists(g_atc), "[6] ... and did not create the hot Temperature_ATC.Data");
    // spbSaveClick reads the files back after saving, and that read alone would create _ATC_Cold as a plain copy of
    // Temperature.Data (copy-on-missing). The save's own write is what makes it more than that copy.
    CHECK(Exists(g_cold) && Slurp(g_cold)!=Slurp(g_base), "[6] _ATC_Cold holds the save's write, not just the copy of Temperature.Data");

    std::printf("[7] save, Hot -> creates _ATC only\n");
    Recipe(0); StepBegin("ReadTempFile"); fTemp_Set->ReadTempFile(true); StepEnd(); Show("read");
    fTemp_Set->rgTemperatureMode->ItemIndex=0;
    ::DeleteFileA(g_atc.c_str()); ::DeleteFileA(g_cold.c_str());
    LastSet.iTemperature=Tempture_Hot;
    StepBegin("spbSaveClick"); fTemp_Set->spbSaveClick(NULL); StepEnd();   // NULL as forms/fMain_Heater.cpp:2439 (the facade is no TObject)
    CHECK(Exists(g_atc), "[7] Hot save wrote Temperature_ATC.Data");
    CHECK(!Exists(g_cold), "[7] ... and did not create Temperature_ATC_Cold.Data");
    CHECK(Exists(g_atc) && Slurp(g_atc)!=Slurp(g_base), "[7] _ATC holds the save's write, not just the copy of Temperature.Data");

    fTemp_Set=oldTempSet;                                           // the instance is left alive (as test_settemp_save.cpp)
    CloseGeneralIniFile();

    std::printf("[8] source pin: both routes carry the split\n");
    if(port.empty())
        CHECK(false, "[8] argv[1] (the port root) is missing");
    else
    {
        const std::string cold="szDir.sprintf(\"%sDefineTemp\\\\Temperature_ATC_Cold.Data\", DefaultPath);";
        const std::string ts=Slurp(port+"\\uTemp_Set.cpp");
        const std::string gen=Slurp(port+"\\FileRW\\Temperature.gen.inc");
        CHECK(Count(ts, cold)==2, "[8] uTemp_Set.cpp: the _ATC_Cold branch at the read and the write site (I-08)");
        CHECK(Count(gen, cold)==2, "[8] FileRW/Temperature.gen.inc: the web route's _ATC_Cold branch, read and write (already V912)");
    }

    std::printf("[9] the machine's own files are untouched\n");
    for(int i=0; i<kN; i++)
        CHECK(SameStamp(before[i], StampOf(kMachine[i])), kMachine[i]);

    if(g_fail==0)
    {
        std::string cmd="rmdir /s /q \""+g_root+"\" >nul 2>&1";
        std::system(cmd.c_str());
    }
    else
        std::printf("  sandbox kept for inspection: %s\n", g_root.c_str());
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_pass, g_pass+g_fail);
    return g_fail ? 1 : 0;
}
