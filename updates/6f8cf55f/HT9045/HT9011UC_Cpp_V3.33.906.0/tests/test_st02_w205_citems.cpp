// ===========================================================================
//  tests/test_st02_w205_citems.cpp -- W-205 (W-195 (2) general items of the 37 file-scope rows, golden 913).
//  AI(W906-W195) 20261009 (St02-E).  One part per item (one MR per item): L05 now; L03 / L10 are added by their MRs.
//
//  Memory only: the motors stay unconstructed (MOT[].Motor == NULL, so the golden NULL guard skips every card read), the EventLog
//  goes to the ctest log scratch (the W906_*_ROOT redirects of every ctest, re-checked below), argv[1] = the tree root (read only).
//
//    L05  golden 913 acarry.cpp:3484-3515 W906_ShtMoveTimeoutDiag_St02: the exact golden-format interlock snapshot row
//         ("ShtMoveTimeout Diag SHT<n> : ...") through the 2-arg MyDBIProcess adapter; source pins: in DoShtMoveTimeoutHandle the
//         call sits after the latch-dump MyDBIProcess and before the first ShowMyMessage (golden: "必須在 ShowMyMessage 之前"),
//         and the operator text is golden 913 :3520.
//    L10  [C25] floodgate, golden 913 acarry.cpp:8784-8810 / csystem.cpp:10986 / :11326 / Config.h:436: DoFloodGateClose truth table
//         over (SHUTTLE_FLOODGATE, C25, ATC active cooling) -- true only for (1,1,1); DoFloodGateCloseAtShuttleLeft closes only the
//         Out-Shuttle floodgate whose In Shuttle command position is within +-2 of Left, nothing when SHUTTLE_FLOODGATE=0 (HT9050).
//         The floodgate cylinders stay Enable=false (no IO) and the shuttle motors unconstructed (memory Position).  Source pins:
//         no 912 chiller rule left in DoFloodGateClose; the calls at HOME done / Tray Feed done sit at golden's positions.
//         (L03 has its own test, St02_W205AsmBorrow, MR !397.)
// ===========================================================================
#include "MachineDefine.h"
#include "acarry.h"
#include "aHotPlateSubstrate.h"     // W906_MyDBIProcess_LastS1 / _LastS2 / _Reset
#include "Motor/mymotor.h"
#include "cmydef.h"
#include "atester.h"
#include "common.h"
#include "forms/fTemp_Set.h"        // the W-195 rule: create fTemp_Set first (see main)
#include "cprod.h"                  // L10: Prod.InSHT[].iLeft, Temperature.bATCActiveCooling
#include "Config.h"                 // L10: IniConfig.bC25FloodGateCloseByATCEnable
#include "mycylin.h"                // L10: Cylinder[C_OutShuttle1Floodgate / 2]

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

extern bool IndexZCanMove[2];
extern int  iPickFromShuttle1Task, iPickFromShuttle2Task;
void W906_ShtMoveTimeoutDiag_St02(int iSelSHT);

static int g_fail = 0;
static int g_total = 0;
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

static bool Forbidden(const char* p)
{
    std::string s = p ? p : "";
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    if (s.find("\\machine_log_scratch") != std::string::npos) return false;   // AI(W906-W205) 20261009 (laptop, batch 146 integration): the ctest redirect counts as contained wherever the build dir lives -- the laptop's gate builds under D:\HT9045\.claude\worktrees\..., so the bare d:\ht9045 prefix test below refused to run (same containment rule as test_c10_hana_rms.cpp:254)
    return s.compare(0, 9, "d:\\ht9045") == 0 || s.compare(0, 6, "d:\\rms") == 0;
}

static void PartL05(const char* src)
{
    std::printf("\n-- L05: Shuttle Move Timeout interlock snapshot --\n");
    check(MOT[MTestY1].Motor == NULL && MOT[MTestZ1].Motor == NULL && MOT[MTestZ2].Motor == NULL && MOT[MTestY2].Motor == NULL,
          "precondition: no motor objects (the golden NULL guard skips every card read)");
    const bool savZ0 = IndexZCanMove[0], savZ1 = IndexZCanMove[1];
    IndexZCanMove[0] = true; IndexZCanMove[1] = false;
    iTestTask = 1234; iTestHeadMotorTask = 567; iTestYTask = 89;
    fFrontNeedTest = true; fFrontNeedSuck = false; fFrontNeedSuckIC = false;
    fRearNeedSuck = true; fRearNeedSuckIC = false; fRearNeedDestroy = false;
    AutoSHT1Task = 11; AutoSHT2Task = 120; iPickFromShuttle1Task = 7; iPickFromShuttle2Task = 42;
    MOT[MInShuttle2].fCanMove = false; MOT[MInShuttle2].fCanMoveL = true; MOT[MInShuttle2].fCanMoveR = false; MOT[MInShuttle2].fCanMoveM = false;

    W906_MyDBIProcess_Reset();
    W906_ShtMoveTimeoutDiag_St02(1);
    const std::string want =
        "ShtMoveTimeout Diag SHT2 : CanMove=0 L=1 R=0 M=0 | IdxZCanMove=1/0 | "
        "FrontNeedTest=1 FrontNeedSuck=0 FrontNeedSuckIC=0 RearNeedSuck=1 RearNeedSuckIC=0 RearNeedDestroy=0 | "
        "TestTask=1234 HeadMotTask=567 TestYTask=89 SHT1Task=11 SHT2Task=120 PickSht1=7 PickSht2=42 | "
        "Y1 0/0 Y2 0/0 Z1 0/0 Z2 0/0 (cmd/enc)";
    const std::string got = W906_MyDBIProcess_LastS2.c_str();
    std::printf("     row: %s\n", got.c_str());
    check(std::string(W906_MyDBIProcess_LastS1.c_str()) == "Message" && got == want,
          "L05: one EventLog row \"Message\", exactly golden 913 :3503-3514's format with the seeded flags / tasks / positions");
    IndexZCanMove[0] = savZ0; IndexZCanMove[1] = savZ1;
    check(savZ0 == false && savZ1 == false, "L05: V906 IndexZCanMove is {false,false} (GATE W4G-1) -> a real row logs 0/0 (accepted, W-205)");

    if (src) {
        const std::string a = Read(std::string(src) + "/acarry.cpp");
        const size_t fn = a.find("\nvoid DoShtMoveTimeoutHandle(int iSelSHT)");
        const size_t end = a.find("\n}", fn == std::string::npos ? 0 : fn);
        const std::string body = (fn == std::string::npos || end == std::string::npos) ? "" : a.substr(fn, end - fn);
        const size_t dump = body.find("MyDBIProcess(\"Message\", sLog);");
        const size_t diag = body.find("{ void W906_ShtMoveTimeoutDiag_St02(int iSelSHT); W906_ShtMoveTimeoutDiag_St02(iSelSHT); }");
        const size_t msg = body.find("ShowMyMessage(");
        check(dump != std::string::npos && diag != std::string::npos && msg != std::string::npos && dump < diag && diag < msg,
              "L05 source: the snapshot call is after the latch dump and before the operator message (golden 913 :3482 < :3484-3515 < :3524)");
        check(body.find("\"Press OK to Home, then do STATE RECORD before START.\",") != std::string::npos &&
              body.find("then START to resume.\",") == std::string::npos,
              "L05 source: the operator text is golden 913 :3520");
    } else
        check(false, "argv[1] = the tree root");
}

static void PartL10(const char* src)
{
    std::printf("\n-- L10: [C25] floodgate --\n");
    TMyCylinder& o1 = Cylinder[C_OutShuttle1Floodgate];
    TMyCylinder& o2 = Cylinder[C_OutShuttle2Floodgate];
    check(o1.Enable == false && o2.Enable == false && MOT[MInShuttle1].Motor == NULL && MOT[MInShuttle2].Motor == NULL,
          "precondition: floodgate cylinders Enable=false (On() writes no IO) and shuttle motors unconstructed (memory Position)");
    const int  savFG = SHUTTLE_FLOODGATE;
    const bool savC25 = IniConfig.bC25FloodGateCloseByATCEnable, savCool = Temperature.bATCActiveCooling;
    const int  savL0 = Prod.InSHT[0].iLeft, savL1 = Prod.InSHT[1].iLeft;
    const int  savP1 = MOT[MInShuttle1].Position, savP2 = MOT[MInShuttle2].Position;
    const bool savO1 = o1.bCylinderOn, savO2 = o2.bCylinderOn;
    check(savC25 == false, "L10: C25 starts at 0 (no settings row yet = golden's default)");

    int hits = 0, wrong = 0;
    for (int m = 0; m < 8; ++m) {
        SHUTTLE_FLOODGATE = (m & 1) ? 1 : 0;
        IniConfig.bC25FloodGateCloseByATCEnable = (m & 2) != 0;
        Temperature.bATCActiveCooling = (m & 4) != 0;
        const bool r = DoFloodGateClose();
        if (r) ++hits;
        if (r != (m == 7)) { ++wrong; std::printf("     FG=%d C25=%d cool=%d -> %d\n", m & 1, (m >> 1) & 1, (m >> 2) & 1, (int)r); }
    }
    check(hits == 1 && wrong == 0, "L10: DoFloodGateClose true only for SHUTTLE_FLOODGATE=1 + C25 on + ATC active cooling (golden 913 :8788-8794)");

    Prod.InSHT[0].iLeft = 1000; Prod.InSHT[1].iLeft = 3000;
    MOT[MInShuttle1].Position = 1002; MOT[MInShuttle2].Position = 3003;
    SHUTTLE_FLOODGATE = 0; IniConfig.bC25FloodGateCloseByATCEnable = true; Temperature.bATCActiveCooling = true;
    o1.bCylinderOn = false; o2.bCylinderOn = false;
    DoFloodGateCloseAtShuttleLeft();
    check(!o1.bCylinderOn && !o2.bCylinderOn, "L10: SHUTTLE_FLOODGATE=0 (HT9050) -> no floodgate touched (golden :8802-8803)");

    SHUTTLE_FLOODGATE = 1;
    DoFloodGateCloseAtShuttleLeft();
    std::printf("     Out1 %d Out2 %d (Sht1 %d vs Left 1000, Sht2 %d vs Left 3000)\n", (int)o1.bCylinderOn, (int)o2.bCylinderOn,
                MOT[MInShuttle1].Position, MOT[MInShuttle2].Position);
    check(o1.bCylinderOn && !o2.bCylinderOn, "L10: rule true -> Out1 closes (Sht1 at Left+2), Out2 stays (Sht2 at Left+3) (golden :8805-8809)");

    o1.bCylinderOn = false; MOT[MInShuttle2].Position = 2998;
    IniConfig.bC25FloodGateCloseByATCEnable = false;
    DoFloodGateCloseAtShuttleLeft();
    check(!o1.bCylinderOn && !o2.bCylinderOn, "L10: C25 off -> nothing closes even with both shuttles at Left");
    IniConfig.bC25FloodGateCloseByATCEnable = true;
    DoFloodGateCloseAtShuttleLeft();
    check(o1.bCylinderOn && o2.bCylinderOn, "L10: C25 on, Sht2 at Left-2 -> both Out floodgates close");

    SHUTTLE_FLOODGATE = savFG; IniConfig.bC25FloodGateCloseByATCEnable = savC25; Temperature.bATCActiveCooling = savCool;
    Prod.InSHT[0].iLeft = savL0; Prod.InSHT[1].iLeft = savL1;
    MOT[MInShuttle1].Position = savP1; MOT[MInShuttle2].Position = savP2;
    o1.bCylinderOn = savO1; o2.bCylinderOn = savO2;

    if (!src) { check(false, "argv[1] = the tree root"); return; }
    const std::string a = Read(std::string(src) + "/acarry.cpp");
    const size_t fn = a.find("\nbool DoFloodGateClose()");
    const size_t end = fn == std::string::npos ? std::string::npos : a.find("\n}", fn);
    const std::string body = (fn == std::string::npos || end == std::string::npos) ? "" : a.substr(fn, end - fn);
    check(body.find("        if(IniConfig.bC25FloodGateCloseByATCEnable==true &&") != std::string::npos &&
          body.find("\n        if(ATC_InterfaceForm->iATC_MODE_TYPE==36") == std::string::npos &&
          body.find("\n            if(dChillerTemp < -20)") == std::string::npos && body.find("\n                bRult=true;") != std::string::npos,
          "L10 source: DoFloodGateClose has the 913 C25 rule and no 912 chiller rule (golden 913 :8790-8793)");

    const std::string c = Read(std::string(src) + "/csystem.cpp");
    const size_t h0 = c.find("\nvoid DoHomeProcess()");
    const size_t h1 = h0 == std::string::npos ? std::string::npos : c.find("\n}", h0);
    const std::string home = (h0 == std::string::npos || h1 == std::string::npos) ? "" : c.substr(h0, h1 - h0);
    const size_t place = home.find("bPlaceShuttle=false;");
    const size_t hcall = home.find("DoFloodGateCloseAtShuttleLeft();");
    const size_t byStart = home.find("if(bHomeByStart==true)");
    const size_t t0 = c.find("\nvoid DoTrayFeedProcess()");
    const size_t t1 = t0 == std::string::npos ? std::string::npos : c.find("\n}", t0);
    const std::string feed = (t0 == std::string::npos || t1 == std::string::npos) ? "" : c.substr(t0, t1 - t0);
    const size_t fcall = feed.find("        iTrayFeed=0;  DoFloodGateCloseAtShuttleLeft();");
    const size_t fend = fcall == std::string::npos ? std::string::npos : feed.find("\n", fcall + 1);
    const size_t unload = fend == std::string::npos ? std::string::npos : feed.find_first_not_of(" \t\r\n", fend);
    check(place != std::string::npos && hcall != std::string::npos && byStart != std::string::npos && place < hcall && hcall < byStart &&
          unload != std::string::npos && feed.compare(unload, 35, "if(IniConfig.bEnableUnloadTrayFree)") == 0,
          "L10 source: called at HOME done (after bPlaceShuttle, before bHomeByStart) and Tray Feed done (with iTrayFeed=0, before UnloadTrayFree) (golden 913 csystem.cpp:10984-10988 / :11325-11327)");

    // the settings-page row (AI(W906-W195) 20261009 (St02-E), RULINGS_20261009 #14): St01's generator, golden 913 cConfiguration.cpp:1318-1322
    const std::string gi = Read(std::string(src) + "/FileRW/IniConfig.gen.inc");
    const size_t fc = gi.rfind("static void IC_InitConfigEdtList_ItemC()");   // the body (the first hit is the forward declaration)
    const size_t c24 = gi.find("\"bC24InitialStartCheckCylinder\",       bNoShow, bEnable, bFixedValue, 0);", fc);
    const std::string row = "    } if(SHUTTLE_FLOODGATE==1) elConfig->Add(EL<TCheckBox>(\"TfConfiguration\", \"cbC25\"), &IniConfig.bC25FloodGateCloseByATCEnable, "
                            "ECBool, \"Function\", \"bC25FloodGateCloseByATCEnable\", bShow, bEnable, bReadFromFile, 0); else elConfig->Add(EL<TCheckBox>("
                            "\"TfConfiguration\", \"cbC25\"), &IniConfig.bC25FloodGateCloseByATCEnable, ECBool, \"Function\", \"bC25FloodGateCloseByATCEnable\", "
                            "bNoShow, bEnable, bFixedValue, 0);";
    const size_t at = gi.find(row, c24 == std::string::npos ? 0 : c24);
    const size_t nextFn = gi.rfind("static void IC_InitConfigEdtList_ItemD()");
    check(fc != std::string::npos && c24 != std::string::npos && at != std::string::npos && at < nextFn && gi.find(row, at + 1) == std::string::npos,
          "L10 settings row: IniConfig.gen.inc ItemC has [C25] after the [C24] else, once -- shown + read from config.ini only when SHUTTLE_FLOODGATE==1, else fixed (golden 913 cConfiguration.cpp:1318-1322)");
}

int main(int argc, char** argv)
{
    const char* roots[] = { as9045LogPath.c_str(), asSaveEventLogPath.c_str() };
    for (size_t i = 0; i < sizeof(roots) / sizeof(roots[0]); ++i)
        if (Forbidden(roots[i])) {
            std::printf("STOP: log root %s is a machine path -- the ctest W906_*_ROOT redirect is missing; nothing called\n", roots[i]);
            return 1;
        }
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // W-195 rule (laptop batch 136)
    GetTimeInfo();

    PartL05(argc > 1 ? argv[1] : 0);
    PartL10(argc > 1 ? argv[1] : 0);

    std::printf("test_st02_w205_citems: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
