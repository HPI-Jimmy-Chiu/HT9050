// ===========================================================================
//  tests/test_st02_h6_a77.cpp -- H6 (RULINGS_20261009 #14): golden 913 cConfiguration.cpp:992-999 (RogerYang 20260916) -- on a HANA
//  machine the RMS interlock switch moved from [A76] (908.x) to [A77] (912); the edit-list init copies the old value once, so the
//  interlock does not silently turn off.  Generated into FileRW/IniConfig.gen.inc by St01's generator (tools/editlist/IniConfig.py
//  replace row on golden 912 :992).  AI(W906-W195) 20261009 (St02-E).
//
//  FileRW/IniConfig.cpp is a wb_serve source (it cannot be linked into a ctest -- tests/CMakeLists.txt test_weblogin_reauth note), so
//  the generated statement is checked as text and run here: H6_CODE below is the replace row's code; [1] the generated line must be
//  exactly H6_CODE + golden 912 :992's Add, in the CC_HANA_MICRON branch only; [2]-[4] the very same H6_CODE runs against a config.ini
//  in a private folder under ctest's machine_config_scratch (the run stops before any call when AuthPath is not that scratch).
//    [2] [A76]=1, no [A77]  -> [A77] A77_EnableHanaRMSInterlock=1 written
//    [3] [A76]=1, [A77]=0   -> [A77] stays 0 (CheckKeyExist A77 true -> no copy)
//    [4] no [A76], no [A77] -> nothing written
// ===========================================================================
#include "MachineDefine.h"
#include "common.h"                 // AuthPath, CheckKeyExist, ReadIniData, WriteIniData

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#define H6_STR2(x) #x
#define H6_STR(x) H6_STR2(x)
#define H6_CODE AnsiString sRMSCfg=AuthPath+"config.ini"; if(CheckKeyExist(sRMSCfg, "A77", "A77_EnableHanaRMSInterlock")==false && CheckKeyExist(sRMSCfg, "A76", "A76_EnableHanaRMSInterlock")==true) { WriteIniData(sRMSCfg, "A77", "A77_EnableHanaRMSInterlock", ReadIniData(sRMSCfg, "A76", "A76_EnableHanaRMSInterlock", false)); }

static void RunH6() { H6_CODE }

static int g_fail = 0, g_total = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}
static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}

static void Case(const std::string& base, const char* name, const char* ini, bool wantA77, bool wantValue, const char* what)
{
    const std::string dir = base + "h6_" + name + "\\";
    ::CreateDirectoryA(dir.c_str(), 0);
    { std::ofstream f((dir + "config.ini").c_str(), std::ios::binary); f << ini; }
    AuthPath = AnsiString(dir.c_str());
    RunH6();
    const AnsiString cfg = AuthPath + "config.ini";
    const bool has = CheckKeyExist(cfg, "A77", "A77_EnableHanaRMSInterlock");
    const bool val = ReadIniData(cfg, "A77", "A77_EnableHanaRMSInterlock", false);
    std::printf("     %s: [A77] present %d value %d\n", name, (int)has, (int)val);
    check(has == wantA77 && (!wantA77 || val == wantValue), what);
}

int main(int argc, char** argv)
{
    const AnsiString savAuth = AuthPath;
    if (Lower(AuthPath.c_str()).find("machine_config_scratch") == std::string::npos) {
        std::printf("STOP: AuthPath %s is not ctest's machine_config_scratch -- nothing called\n", AuthPath.c_str());
        return 1;
    }

    std::printf("-- [1] the generated line (FileRW/IniConfig.gen.inc) --\n");
    if (argc > 1) {
        const std::string g = Read(std::string(argv[1]) + "/FileRW/IniConfig.gen.inc");
        const std::string addShow = " elConfig->Add(EL<TCheckBox>(\"TfConfiguration\", \"cbA77\"), &IniConfig.bA77_EnableHanaRMSInterlock, ECBool, "
                                    "\"A77\", \"A77_EnableHanaRMSInterlock\", bShow, bEnable, bReadFromFile, 0);";
        const std::string want = std::string(H6_STR(H6_CODE)) + addShow;
        const size_t fn = g.find("static void IC_InitConfigEdtList_ItemA()");
        const size_t hana = g.find("if(CUSTOMER_CODE==CC_HANA_MICRON)                                           //RogerYang 20260725 : HANA RMS Interlock enable", fn);
        const size_t at = g.find(want, hana);
        const size_t btn = g.find("EL<TButton>(\"TfConfiguration\", \"btnA77RMSSetting\")->Visible=true;", hana);
        const size_t els = g.find("\"A77_EnableHanaRMSInterlock\", bNoShow, bDisable, bReadFromFile, 0);", hana);
        check(fn != std::string::npos && hana != std::string::npos && at != std::string::npos && at < btn && btn < els &&
              g.find(H6_STR(H6_CODE)) == at && g.find(H6_STR(H6_CODE), at + 1) == std::string::npos,
              "[1] IC_InitConfigEdtList_ItemA, CC_HANA_MICRON branch: the migration then golden 912 :992's Add, once; not in the else branch "
              "(golden 913 cConfiguration.cpp:990-1000)");
        check(g.find("#if 0 // GATE (S12-C save) golden cConfiguration.cpp:992-992 -- AI(W906-W195) 20261009 (St02-E) H6", hana) != std::string::npos,
              "[1] the generator kept golden 912 :992 under its replace gate");
    } else
        check(false, "argv[1] = the tree root");

    std::printf("-- [2]-[4] the same statement on scratch config.ini files --\n");
    char tick[32];
    std::snprintf(tick, sizeof(tick), "%lu", (unsigned long)::GetTickCount());
    std::string base = std::string(savAuth.c_str());
    if (!base.empty() && base[base.size() - 1] != '\\') base += "\\";
    base += std::string("h6_") + tick + "\\";
    ::CreateDirectoryA(base.c_str(), 0);
    Case(base, "migrate", "[A76]\r\nA76_EnableHanaRMSInterlock=1\r\n", true, true,
         "[2] [A76]=1 and no [A77] -> [A77] A77_EnableHanaRMSInterlock=1 (golden 913 :994-998)");
    Case(base, "keep", "[A76]\r\nA76_EnableHanaRMSInterlock=1\r\n[A77]\r\nA77_EnableHanaRMSInterlock=0\r\n", true, false,
         "[3] [A77] already there -> left as it is (golden 913 :994)");
    Case(base, "none", "[Other]\r\nx=1\r\n", false, false, "[4] no [A76] -> nothing written (golden 913 :995)");

    AuthPath = savAuth;
    std::printf("test_st02_h6_a77: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
