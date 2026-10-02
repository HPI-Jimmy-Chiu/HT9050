// =============================================================================
//  test_settemp_save.cpp  --  AI(W906-I01C) 20261002 (Ifor01)
//
//  I-01 phase 2 (2) (docs/handoff/TO_IFOR.md §4 1002 08:5x ①, W-05 = (a)): golden TfMain::SetTemp (main.cpp:23890-23981,
//  forms/fMain_Heater.cpp) saves the new set point through fTemp_Set->spbSaveClick -> SaveSetupFile (uTemp_Set.cpp), whose
//  SAFETY GATE (S1) / (S2) and GATE(dep-FormHS) are open now. The test FAILS with those gates put back: the file keeps the
//  old set point and ReadTempFile reads it back (the "silent half-success" that kept the hook out in phases 1 / 2 (1)).
//
//    [1] before: ReadTempFile(true) reads the sandbox recipe (Temperature 50.0, Soak 10.0)
//    [2] SetTemp(false, 85.0, 30.0) through the hook (W906_InstallSetTemp) returns 0; the sandbox recipe's Temperature.Data now
//        has [Mode] Temperature=85.0 and [Time] Soak=30.0 (golden SaveSetupFile, "0.0" format); Temperature.fWorkTemperBase /
//        fSoakTime read back 85 / 30 (golden :23960 ReadTempFile(true)); fHeaterOK=false, iThermoTask=1 (golden :23966-23972);
//        the C route's page hook got "fTemp_Set" once (port-only: the web temperature page must reopen)
//    [3] SystemStart -> returns 1 and writes nothing (golden :23896-23897)
//    [4] the machine's own files are untouched: D:\HT9045\SetUp.inf's recipe Temperature.Data, config\ATC.ini, config\config.ini,
//        system\Gerneral.ini, IniData\DefineTemp\Temperature.Data (size + write time)
//
//  Sandbox: %TEMP%\ht9045_settemp_<tick>\ (DataPath / DefaultPath / OffsetPath / LastDataPath are pointed there before any call;
//  AuthPath and asGeneralPath follow ctest's W906_AUTH_PATH / W906_GENERAL_INI_PATH). Refuses to run (exit 2, nothing called)
//  outside ctest's redirect roots (st02_test_containment.h). The sandbox is removed on a green run.
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
#include "bthermo.h"          // iThermoTask
#include "st02_test_containment.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

extern int (*W906_SetTempHook)(TfMain*, bool, double, double);   // forms/fMain.cpp:511
void W906_InstallSetTemp();                                       // forms/fMain_Heater.cpp
extern void (*W906_SetTempPageClosedHook)(const char*);           // forms/fMain_Heater.cpp

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_settemp_save.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string g_root;
static int g_pageClosed=0;
static std::string g_pageForm;
static void FakePageClosed(const char* form) { g_pageClosed++; g_pageForm=form ? form : ""; }

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
static bool UnderMachineTree(const std::string& p)
{
    std::string l=W906TestSafeLower(p.c_str());
    return l.find("d:\\ht9045\\")==0 || l.find("d:/ht9045/")==0;
}
// [section] key=value, case-insensitive on section / key; "" when missing
static std::string IniValue(const std::string& text, const char* section, const char* key)
{
    std::string sec;
    size_t pos=0;
    while(pos<text.size())
    {
        size_t e=text.find('\n', pos);
        if(e==std::string::npos) e=text.size();
        std::string line=text.substr(pos, e-pos);
        pos=e+1;
        while(!line.empty() && (line[line.size()-1]=='\r' || line[line.size()-1]==' ')) line.erase(line.size()-1);
        if(line.size()>=2 && line[0]=='[' && line[line.size()-1]==']') { sec=W906TestSafeLower(line.substr(1, line.size()-2).c_str()); continue; }
        const size_t eq=line.find('=');
        if(eq==std::string::npos || sec!=W906TestSafeLower(section)) continue;
        std::string k=line.substr(0, eq);
        while(!k.empty() && k[k.size()-1]==' ') k.erase(k.size()-1);
        if(W906TestSafeLower(k.c_str())==W906TestSafeLower(key)) return line.substr(eq+1);
    }
    return std::string();
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
static std::string MachineRecipeTemperature()
{
    std::string inf=Slurp("D:\\HT9045\\SetUp.inf");
    while(!inf.empty() && (inf[inf.size()-1]=='\r' || inf[inf.size()-1]=='\n' || inf[inf.size()-1]==' ')) inf.erase(inf.size()-1);
    if(inf.empty() || inf=="<missing>") return std::string();
    return "D:\\HT9045\\IniData\\Data\\"+inf+"\\Temperature.Data";
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("SetTempSave\n");
    if(!W906TestInsideCtestRoots("SetTempSave"))
        return 2;

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root=std::string(tmp)+"ht9045_settemp_"+stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root+"\\Data").c_str(), 0);
    ::CreateDirectoryA((g_root+"\\Data\\STR").c_str(), 0);
    ::CreateDirectoryA((g_root+"\\DefineTemp").c_str(), 0);
    ::CreateDirectoryA((g_root+"\\Offset").c_str(), 0);
    Spit(g_root+"\\SetUp.inf", "STR\r\n");
    const std::string recipe=g_root+"\\Data\\STR\\Temperature.Data";
    Spit(recipe, "[Mode]\r\nMode=0\r\nTemperature=50.0\r\n[Time]\r\nSoak=10.0\r\n");

    DataPath    =AnsiString((g_root+"\\Data\\").c_str());
    DefaultPath =AnsiString((g_root+"\\").c_str());
    OffsetPath  =AnsiString((g_root+"\\Offset\\").c_str());
    LastDataPath=AnsiString((g_root+"\\SetUp.inf").c_str());
    // DataPath / DefaultPath / LastDataPath: the %TEMP% sandbox, never under D:\HT9045. AuthPath / asGeneralPath: ctest's own scratch
    // (W906_AUTH_PATH / W906_GENERAL_INI_PATH, already checked by W906TestInsideCtestRoots) -- which itself lives under the build tree,
    // possibly D:\HT9045\...\Obj, so for those two the test is "is it a scratch dir", not "is it outside D:\HT9045".
    const bool authScratch=W906TestSafeLower(AuthPath.c_str()).find("scratch")!=std::string::npos;
    const bool genScratch=W906TestSafeLower(asGeneralPath.c_str()).find("scratch")!=std::string::npos;
    if(UnderMachineTree(DataPath.c_str()) || UnderMachineTree(DefaultPath.c_str()) || UnderMachineTree(LastDataPath.c_str()) ||
       !authScratch || !genScratch)
    {
        std::printf("  ABORT: a path is not in the sandbox (Data %s, Default %s, LastData %s, Auth %s, General %s) -- nothing was called\n",
                    DataPath.c_str(), DefaultPath.c_str(), LastDataPath.c_str(), AuthPath.c_str(), asGeneralPath.c_str());
        return 2;
    }

    const char* const kMachine[]={ "D:\\HT9045\\config\\ATC.ini", "D:\\HT9045\\config\\config.ini", "D:\\HT9045\\system\\Gerneral.ini",
                                   "D:\\HT9045\\IniData\\DefineTemp\\Temperature.Data" };
    const int kN=(int)(sizeof(kMachine)/sizeof(kMachine[0]));
    Stamp before[kN+1];
    for(int i=0; i<kN; i++) before[i]=StampOf(kMachine[i]);
    const std::string machineRecipe=MachineRecipeTemperature();
    before[kN]=machineRecipe.empty() ? Stamp() : StampOf(machineRecipe);

    OpenGeneralIniFile();   // wb_serve opens INIFileGeneral in LoadMachineConfig (database.cpp:3142) and keeps it open until exit; SaveLastSetIni
                            // (spbSaveClick) writes through it (WriteIniDataGeneral, common.cpp, no NULL check -- golden). Here on ctest's sandbox path.
    CHECK(fMain!=0, "fMain exists (the facade's global)");
    if(fMain==0) { std::printf("FAIL: no fMain\n"); return 1; }
    TfTemp_Set* const oldTempSet=fTemp_Set;
    fTemp_Set=new TfTemp_Set();
    fTemp_Set->Init();                                              // the wb_serve boot does the same (tools/wb_serve.cpp:3112 / :3121)
    SystemStart=false;
    IniConfig.bEnable_SECS_GEM=false;
    IniConfig.bA02DisableSaveParsWhenSwitchToOp=false;
    CosFunction.bSaveTemperatureByMachine=false;
    CosFunction.bNotClearAllHotBuffer=false;

    std::printf("[1] before: ReadTempFile(true) reads the sandbox recipe\n");
    fTemp_Set->ReadTempFile(true);
    CHECK(Temperature.fWorkTemperBase==50.0, "Temperature.fWorkTemperBase == 50.0 from the sandbox recipe");
    CHECK(Temperature.fSoakTime==10.0, "Temperature.fSoakTime == 10.0 from the sandbox recipe");
    CHECK(LastSet.iTemperature==Tempture_Hot, "[Mode] Mode=0 -> Tempture_Hot");

    std::printf("[2] SetTemp(false, 85.0, 30.0) through the hook\n");
    W906_InstallSetTemp();
    W906_SetTempPageClosedHook=&FakePageClosed;
    fMain->W906_SetTemp_Sim=0;
    fHeaterOK=true;
    iThermoTask=42;
    const int r=fMain->SetTemp(false, 85.0, 30.0);
    CHECK(r==0, "SetTemp returns 0 (golden :23979)");
    const std::string after=Slurp(recipe);
    CHECK(IniValue(after, "Mode", "Temperature")=="85.0", "recipe [Mode] Temperature == 85.0 (golden SaveSetupFile, uTemp_Set.cpp S1)");
    CHECK(IniValue(after, "Time", "Soak")=="30.0", "recipe [Time] Soak == 30.0");
    CHECK(Temperature.fWorkTemperBase==85.0, "Temperature.fWorkTemperBase == 85.0 read back (golden :23960 ReadTempFile(true))");
    CHECK(Temperature.fSoakTime==30.0, "Temperature.fSoakTime == 30.0 read back");
    CHECK(fHeaterOK==false, "fHeaterOK reset (golden :23966)");
    CHECK(iThermoTask==1, "iThermoTask = 1 (golden :23972)");
    CHECK(g_pageClosed==1 && g_pageForm=="fTemp_Set", "the web temperature page hook got \"fTemp_Set\" once (port-only)");
    if(IniValue(after, "Mode", "Temperature")!="85.0")
        std::printf("     recipe now:\n%s\n", after.c_str());

    std::printf("[3] SystemStart: returns 1, writes nothing (golden :23896-23897)\n");
    {
        const std::string snap=Slurp(recipe);
        SystemStart=true;
        CHECK(fMain->SetTemp(false, 95.0, 5.0)==1, "SystemStart -> 1");
        SystemStart=false;
        CHECK(Slurp(recipe)==snap, "the recipe is unchanged");
        CHECK(g_pageClosed==1, "the page hook is not called again");
    }
    W906_SetTempHook=0;
    W906_SetTempPageClosedHook=0;
    fTemp_Set=oldTempSet;                                           // the instance is left alive (VCL stand-in teardown not exercised here)

    CloseGeneralIniFile();
    std::printf("[4] the machine's own files are untouched\n");
    for(int i=0; i<kN; i++)
        CHECK(SameStamp(before[i], StampOf(kMachine[i])), kMachine[i]);
    if(!machineRecipe.empty())
        CHECK(SameStamp(before[kN], StampOf(machineRecipe)), machineRecipe.c_str());

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
