// =============================================================================
//  test_i129_b37.cpp -- AI(W906-I129-B37) 20261002
//  ctest: I129_Batch37
//
//  INBOX 129 (a)(b) census, batch 37 (RULINGS_20261001 #0: golden fill-ins are done, not queued):
//    A  #9 SetShuttleToHasNullIC_9045 (ainarm9045.cpp:793, golden :343-382): the closed shuttle-kit cells go NULL_IC ->
//       HAS_NULL_IC; iKit==0 only the first iShtKitStep columns ("0 = only half"), else (1 or the default -1) all iShtCol;
//       HAS_IC cells are left alone; iSht 0 / 1 picks FLCarryKit / BLCarryKit.  [R] with the old `#if 0` stub nothing moved.
//    B  #8 SetFixTrayFullIC (aoutarm.cpp:1408, golden :809) now reaches csystem.cpp's ClearFixTray(i, .., 3) ->
//       MOT[iMMAuto[i]].SetNullIcToHasIc(): a Fix tray whose up half is full gets its empty cells marked HAS_IC, so it is
//       changed as golden.  [R] with the empty PTW4_ClearFixTray stand-in the empty cells stayed.  Linked trays and trays
//       with neither half full are left alone (both before and after).
//    C  source ratchet (argv[1] = port root, read only): ainarm2.cpp's DoInDieClean case 100 is golden :3813
//       `if(MoveInArm2XYToClean())` again (GATE W7E-K8-C lifted -- with `if(false)` a machine with USE_DIE_CLEAN=1 and
//       iEnableDieClean=1 stalled at DoInArmAdditionalFunction case 10000), and aoutarm.cpp has no PTW4_ClearFixTray
//       definition / `#define ClearFixTray` left.  A ratchet, not a behaviour test: MoveInArm2XYToClean is a continuous
//       X/Y move (InArmContinuousMove_9045).
//       #1 (C4-C8): golden MoveInArm2XYToDecayTeach() (ainarm2.cpp:925) is called again at its three golden places --
//       DoInArmIonFanGiveWay case 3 (it used to "treat the decay-teach position as reached": the in-arm never gave way),
//       DoIonFanAutoClean case 200 (never advanced, WAR2026 every 30 s) and DoAutoDecayCheck case 40 (stalled).
//  No IO: two kit grids, one tray and a few globals are written and restored; ClearFixTray's RecordProcess line goes to
//  the directory-wide redirect roots (_w906_env_all_tests in tests/CMakeLists.txt).
// =============================================================================
#include "MachineDefine.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "cpublic.h"
#include "CosFunction.h"
#include "Config.h"
#include "ainarm9045.h"                 // BEFORE aHotPlateSubstrate.h: its InArmLeftSide* defaults sit under #ifndef ainarm9045H (:893)
#include "mykitsuck.h"
#include "aHotPlateSubstrate.h"
#include "Motor/mymotor.h"
#include "aoutarm.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static int CountKit(TMyKitSuck &k, int rows, int cols, int v)
{
    int n = 0;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            if (k.Item[i][j] == v) ++n;
    return n;
}

static void PartA_ShtNull()
{
    std::printf("PART A -- #9 SetShuttleToHasNullIC_9045 (golden ainarm9045.cpp:343-382)\n");
    int savedFL[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM], savedBL[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    std::memcpy(savedFL, FLCarryKit.Item, sizeof(savedFL));
    std::memcpy(savedBL, BLCarryKit.Item, sizeof(savedBL));
    const int sRow = InArmSuck.iShtRow, sCol = InArmSuck.iShtCol, sStep = InArmSuck.iShtKitStep;
    InArmSuck.iShtRow = 2; InArmSuck.iShtCol = 4; InArmSuck.iShtKitStep = 2;

    for (int i = 0; i < _MAX_SUCK_ROW_ITEM; ++i)
        for (int j = 0; j < _MAX_SUCK_COL_ITEM; ++j)
            FLCarryKit.Item[i][j] = BLCarryKit.Item[i][j] = NULL_IC;
    FLCarryKit.Item[0][0] = HAS_IC;                                            // a real IC must stay
    SetShuttleToHasNullIC_9045(0, 0);
    CHECK(FLCarryKit.Item[0][0] == HAS_IC, "A1 iKit 0: a HAS_IC cell is left alone");
    CHECK(FLCarryKit.Item[0][1] == HAS_NULL_IC && FLCarryKit.Item[1][0] == HAS_NULL_IC && FLCarryKit.Item[1][1] == HAS_NULL_IC,
          "A2 [R] iKit 0: the first iShtKitStep (2) columns NULL_IC -> HAS_NULL_IC");
    CHECK(FLCarryKit.Item[0][2] == NULL_IC && FLCarryKit.Item[1][3] == NULL_IC, "A3 iKit 0: columns 2..3 untouched (only half)");
    CHECK(CountKit(BLCarryKit, 2, 4, NULL_IC) == 8, "A4 iSht 0 does not touch BLCarryKit");

    SetShuttleToHasNullIC_9045(1);                                             // the default iKit (-1): all iShtCol
    CHECK(CountKit(BLCarryKit, 2, 4, HAS_NULL_IC) == 8, "A5 [R] iSht 1, default iKit: all 2x4 BLCarryKit cells -> HAS_NULL_IC");
    CHECK(BLCarryKit.Item[2][0] == NULL_IC && BLCarryKit.Item[0][4] == NULL_IC, "A6 nothing outside iShtRow x iShtCol");
    SetShuttleToHasNullIC_9045(0, 1);
    CHECK(CountKit(FLCarryKit, 2, 4, HAS_NULL_IC) == 7 && FLCarryKit.Item[0][0] == HAS_IC,
          "A7 [R] iSht 0, iKit 1: the other columns too; the HAS_IC cell still left alone");

    std::memcpy(FLCarryKit.Item, savedFL, sizeof(savedFL));
    std::memcpy(BLCarryKit.Item, savedBL, sizeof(savedBL));
    InArmSuck.iShtRow = sRow; InArmSuck.iShtCol = sCol; InArmSuck.iShtKitStep = sStep;
}

static void FillUpHalf(TTrayMotor &m)                                          // Data[x][y], up half = y < YItem/2
{
    m.Tray.ClearData();
    for (int x = 0; x < m.Tray.XItem; ++x)
        for (int y = 0; y < m.Tray.YItem / 2; ++y)
            m.Tray.Data[x][y] = HAS_IC;
}

static void PartB_FixTrayFull()
{
    std::printf("PART B -- #8 SetFixTrayFullIC -> csystem.cpp ClearFixTray(i, .., 3) (golden aoutarm.cpp:809, csystem.cpp:15853)\n");
    const int i = eFix1;
    const int savedMode = TrayForm.iFixTrayMode, savedMin = iFixMin, savedMax = iFixMax, savedMM = iMMAuto[i];
    const bool savedLink = TrayForm.bFixTrayLink[i], savedUD = CosFunction.bUseTrayUpDownSet;
    const bool savedP06 = IniConfig.bP06_LoaderUseCarrierTray;
    IniConfig.bP06_LoaderUseCarrierTray = false;                               // SetXYItem: plain X x Y
    if (iMMAuto[i] < 0 || iMMAuto[i] >= MAX_TRAY_MOTOR) iMMAuto[i] = 0;
    TTrayMotor &m = MOT[iMMAuto[i]];
    const int savedX = m.Tray.XItem, savedY = m.Tray.YItem;

    TrayForm.iFixTrayMode = 1; iFixMin = iFixMax = i; TrayForm.bFixTrayLink[i] = false; CosFunction.bUseTrayUpDownSet = false;
    m.Tray.SetXYItem(4, 6);

    FillUpHalf(m);
    CHECK(m.UpHalfIsFull() && !m.DownHalfIsFull() && m.HowManyDevice(NULL_IC) == 12, "B1 setup: 4x6 tray, up half full, 12 empty cells");
    SetFixTrayFullIC();
    CHECK(m.HowManyDevice(NULL_IC) == 0, "B2 [R] up half full -> ClearFixTray(.., 3) marked every empty cell HAS_IC (stand-in: 12 stayed)");

    m.Tray.ClearData();
    m.Tray.Data[0][0] = HAS_IC;
    SetFixTrayFullIC();
    CHECK(m.HowManyDevice(NULL_IC) == 23, "B3 neither half full -> left alone");

    FillUpHalf(m);
    TrayForm.bFixTrayLink[i] = true;
    SetFixTrayFullIC();
    CHECK(m.HowManyDevice(NULL_IC) == 12, "B4 a linked Fix tray is skipped (golden :814)");

    m.Tray.ClearData();
    m.Tray.SetXYItem(savedX, savedY);
    TrayForm.iFixTrayMode = savedMode; iFixMin = savedMin; iFixMax = savedMax; iMMAuto[i] = savedMM;
    TrayForm.bFixTrayLink[i] = savedLink; CosFunction.bUseTrayUpDownSet = savedUD;
    IniConfig.bP06_LoaderUseCarrierTray = savedP06;
}

static std::string ReadSource(const std::string &root, const char *rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    std::string t = ss.str(), out;
    out.reserve(t.size());
    for (char c : t) if (c != '\r') out += c;                                  // the gate checkout is CRLF
    return out;
}

static void PartC_Ratchet(const char *root)
{
    std::printf("PART C -- source ratchet (#2 Die Clean gate, #8 stand-in)\n");
    if (!root) { CHECK(false, "C0 argv[1] (port root) given"); return; }
    const std::string a2 = ReadSource(root, "ainarm2.cpp"), ao = ReadSource(root, "aoutarm.cpp");
    CHECK(!a2.empty() && !ao.empty(), "C0 ainarm2.cpp and aoutarm.cpp read");
    const std::size_t p = a2.find("\n#if 1 // GATE W7E-K8-C LIFTED");
    const std::size_t q = (p == std::string::npos) ? p : a2.find('\n', p + 1);
    CHECK(p != std::string::npos && a2.compare(q + 1, 37, "            if(MoveInArm2XYToClean())") == 0,
          "C1 DoInDieClean case 100: the live branch is golden :3813 if(MoveInArm2XYToClean())");
    CHECK(a2.find("\n#if 0 // GATE W7E-K8-C") == std::string::npos, "C2 the W7E-K8-C gate is not back");
    CHECK(ao.find("static void PTW4_ClearFixTray(") == std::string::npos && ao.find("#define ClearFixTray") == std::string::npos,
          "C3 aoutarm.cpp: no PTW4_ClearFixTray stand-in / #define ClearFixTray left");

    // #1: the three golden calls of MoveInArm2XYToDecayTeach() (ainarm2.cpp:925) are live again.
    const std::string a9 = ReadSource(root, "ainarm9045.cpp"), cs = ReadSource(root, "csystem.cpp");
    CHECK(!a9.empty() && !cs.empty(), "C4 ainarm9045.cpp and csystem.cpp read");
    struct Site { const std::string *src; const char *head; const char *live; const char *what; };
    const Site sites[] = {
        {&a9, "\n#if 1 // TODO(W7) LIFTED -- golden :9205", "            extern bool MoveInArm2XYToDecayTeach(); bDecayReached=MoveInArm2XYToDecayTeach();",
         "C5 DoInArmIonFanGiveWay case 3: the in-arm really moves to the decay-teach point (golden ainarm9045.cpp:9205)"},
        {&cs, "\n#if 1 // GATE G01 LIFTED -- golden csystem.cpp:210-211", "            extern bool MoveInArm2XYToDecayTeach(); if(bflag2==false)",
         "C6 DoIonFanAutoClean case 200: bflag2 from MoveInArm2XYToDecayTeach() again (golden csystem.cpp:210-211)"},
        {&cs, "\n            #if 1 // GATE h4-G2 LIFTED -- golden csystem.cpp:23879-23883", "            extern bool MoveInArm2XYToDecayTeach(); if(MoveInArm2XYToDecayTeach())",
         "C7 DoAutoDecayCheck case 40: waits for MoveInArm2XYToDecayTeach() again (golden csystem.cpp:23879-23883)"},
    };
    for (const Site &s : sites) {
        const std::size_t h = s.src->find(s.head);
        const std::size_t e = (h == std::string::npos) ? h : s.src->find('\n', h + 1);
        const std::size_t f = (e == std::string::npos) ? e : s.src->find('\n', e + 1);
        CHECK(h != std::string::npos && e != std::string::npos && f != std::string::npos && s.src->substr(e + 1, f - e - 1) == s.live, s.what);
    }
    CHECK(a9.find("\n#if 0 // TODO(W7) -- golden :9205") == std::string::npos && cs.find("\n#if 0 // GATE G01 --") == std::string::npos &&
          cs.find("\n            #if 0 // GATE h4-G2") == std::string::npos, "C8 none of the three #if 0 gates is back");
}

int main(int argc, char **argv)
{
    PartA_ShtNull();
    PartB_FixTrayFull();
    PartC_Ratchet(argc > 1 ? argv[1] : nullptr);
    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
