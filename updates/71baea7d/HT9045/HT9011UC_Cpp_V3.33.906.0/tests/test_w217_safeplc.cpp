// =============================================================================
//  test_w217_safeplc.cpp -- AI(W906-W217) 20261010 (Ifor01)
//
//  W-217 SAFEPLC-913 (TO_IFOR 1010 11:2x; Ifor 1010 chose B: type + judgement only, ReeR comms / InitPLCIO_All untouched):
//    [1] golden 913 E_SafePLCIOType (cmydef.h:5656-5663): Uninstall 0, Schneider 1, ReeR 2, Count 3; g_eSafePLCIOType starts Uninstall
//    [2] IsSafePLCIOType_Schneider / _ReeR / IsSafePLCIOInstall (cmydef.h:5666-5687) for SafePlcIO 0/1/2 -- and the consistency
//        assertion the laptop asked for: IsSafePLCIOInstall() == the transition bool Enable_PLCSafety_IO gets from the same key
//        (database.cpp reads SafePlcIO into both; int -> bool), so csystem.cpp / Command.cpp (still on the bool) agree with the rest
//    [3] source pins (argv[1] = port root, read only): database.cpp reads both from SafePlcIO on one line; the 13 swapped call sites
//        use IsSafePLCIOInstall; csystem.cpp / Command.cpp are untouched (still the bool, no IsSafePLCIOInstall)   [AI(W906-W226) 20261010 (St02-E): W-226 moved their 14 sites to IsSafePLCIOInstall(); 3c follows, St02_W226SafePlcSites pins each line]
// =============================================================================
#include "MachineDefine.h"
#include "cmydef.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg) { if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; } else { std::printf("  FAIL: %s\n", msg); ++g_fail; } }
static std::string Slurp(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); std::stringstream s; s << f.rdbuf(); return s.str(); }
static int Count(const std::string& s, const std::string& k) { int n = 0; for (size_t p = s.find(k); p != std::string::npos; p = s.find(k, p + 1)) n++; return n; }
// lines whose code (before any //) carries the snippet; a snippet with leading blanks must start the line (its indentation is the anchor)
static int CodeLines(const std::string& src, const std::string& snip)
{
    int n = 0;
    std::istringstream in(src);
    for (std::string l; std::getline(in, l); ) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        const size_t c = l.find("//");
        const std::string code = c == std::string::npos ? l : l.substr(0, c);
        if (snip[0] == ' ' ? (code.compare(0, snip.size(), snip) == 0) : (code.find(snip) != std::string::npos)) n++;
    }
    return n;
}

int main(int argc, char** argv)
{
    std::printf("W217_SafePlcType\n");
    Check(eSafePLCIOType_Uninstall == 0 && eSafePLCIOType_InstallSchneider == 1 && eSafePLCIOType_InstallReeR == 2 && eSafePLCIOType_Count == 3,
          "1a. E_SafePLCIOType: Uninstall 0 / Schneider 1 / ReeR 2 / Count 3 (golden 913 cmydef.h:5656-5663)");
    Check(g_eSafePLCIOType == eSafePLCIOType_Uninstall && IsSafePLCIOInstall() == false, "1b. g_eSafePLCIOType starts Uninstall (golden 913 cmydef.cpp:5625)");

    const ESafePLCIOType saved = g_eSafePLCIOType;
    const bool savedBool = Enable_PLCSafety_IO;
    bool same = true;
    for (int v = 0; v <= 2; v++) {
        g_eSafePLCIOType = (ESafePLCIOType)v;                  // database.cpp: (ESafePLCIOType)CheckAndReadIniDataGeneral("System","SafePlcIO",0)
        Enable_PLCSafety_IO = v;                                // database.cpp: the same int into the bool
        same = same && (IsSafePLCIOInstall() == Enable_PLCSafety_IO);
        char m[160];
        std::snprintf(m, sizeof m, "2. SafePlcIO=%d: Schneider %d ReeR %d Install %d (golden 913 cmydef.h:5666-5687)", v, (int)IsSafePLCIOType_Schneider(),
                      (int)IsSafePLCIOType_ReeR(), (int)IsSafePLCIOInstall());
        Check(IsSafePLCIOType_Schneider() == (v == 1) && IsSafePLCIOType_ReeR() == (v == 2) && IsSafePLCIOInstall() == (v != 0), m);
    }
    Check(same, "2d. consistency: IsSafePLCIOInstall() == Enable_PLCSafety_IO for SafePlcIO 0/1/2 (csystem / Command still read the bool)");
    g_eSafePLCIOType = saved;
    Enable_PLCSafety_IO = savedBool;

    if (argc > 1) {
        const std::string root = std::string(argv[1]) + "/";
        const std::string db = Slurp(root + "database.cpp");
        Check(db.find("Enable_PLCSafety_IO         =CheckAndReadIniDataGeneral(\"System\",  \"SafePlcIO\",         0);  "
                      "g_eSafePLCIOType=(ESafePLCIOType)(CheckAndReadIniDataGeneral(\"System\",  \"SafePlcIO\", eSafePLCIOType_Uninstall));") != std::string::npos,
              "3a. database.cpp reads SafePlcIO into the type right after the bool (golden 913 database.cpp:1526), same line");
        struct Site { const char* file; const char* code; const char* golden; };
        static const Site kSites[] = {
            {"cinitial.cpp", "            if(IsSafePLCIOInstall() &&", "cinitial.cpp:2550"},
            {"cinitial.cpp", "                if(IsSafePLCIOInstall() &&", "cinitial.cpp:2687"},
            {"cinitial.cpp", "if(Tri_Temp_Machine==1 && IsSafePLCIOInstall()==true)", "cinitial.cpp:5796"},
            {"cinitial.cpp", "    if(IsSafePLCIOInstall())", "cinitial.cpp:5831"},
            {"ckernel.cpp", "    if(IsSafePLCIOInstall()==true)", "ckernel.cpp:950"},
            {"ckernel.cpp", "else if(IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff())", "ckernel.cpp:1036"},
            {"FileRW/IoSetViewFormShow_File.cpp", "J.SetTabVisible(\"tsSafePLC\", IsSafePLCIOInstall());", "iosetview.cpp:353"},
            {"FileRW/IoSetViewFormShow_File.cpp", "J.SetVisible(\"gpSafePLC\", (IsSafePLCIOInstall()));", "iosetview.cpp:1136"},
            {"MainTimerSegments.cpp", "if(IsSafePLCIOInstall()==true && Sen[SnSafeMode].IsOff())", "main.cpp:3133"},
            {"forms/fMain_OperateMode.cpp", "(IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff()) ||", "main.cpp:13496"},
            {"uhome.cpp", "if(IsSafePLCIOInstall()==1 &&", "uhome.cpp:830"},
            {"forms/fMotorTest.cpp", "if(IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff())", "uMotorTest.cpp:1296"},
            {"MyPLC/MyPLC_IO_Modbus.cpp", "        if(IsSafePLCIOInstall())", "MyPLC_IO_Modbus.cpp:113"},
        };
        int ok = 0;
        for (const Site& s : kSites) {
            const std::string src = Slurp(root + s.file);
            const bool hit = CodeLines(src, s.code) == 1;
            if (!hit) std::printf("    site missing: %s `%s` (golden 913 %s)\n", s.file, s.code, s.golden);
            ok += hit;
        }
        char m[120];
        std::snprintf(m, sizeof m, "3b. %d of 13 call sites use IsSafePLCIOInstall, one each (golden 913 lines in the notes)", ok);
        Check(ok == 13, m);
        const std::string cs = Slurp(root + "csystem.cpp"), cm = Slurp(root + "Command.cpp");
        Check(!cs.empty() && !cm.empty() && Count(cs, "IsSafePLCIOInstall()") >= 13   // AI(W906-W226) 20261010 (St02-E): W-226 (was: still the bool)
              && Count(cm, "IsSafePLCIOInstall()") >= 1,
              "3c. csystem.cpp / Command.cpp read IsSafePLCIOInstall() too since W-226 (St02_W226SafePlcSites pins each line)");
    } else std::printf("  (no argv[1]: source pins skipped)\n");

    std::printf("W217_SafePlcType: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
