// =============================================================================
//  tests/test_armcell_plan.cpp  --  ArmCellPlan.cpp (the Teach "Arm Cell" tab's pure plan) with hand-computed values
//
//  AI(W906-ARMCELL) 20261002 [W906]: RULINGS_20261002 #18 (NB2 spec §5 "新 ctest：公式"). The inputs are an HT9050-like machine
//  built by hand. From recipe machines/HT9050/recipes/FT005054_9050: Tray.Data X/Y Start 25.150 / 23.100 mm, pitch 42.800 /
//  38.400, 3 x 8, Block* 0.100; HotPlate.Data X/Y Start 60 / 30, pitch 40 / 40, 2 x 8, Using Flag=2 (HP2 only), Use Wide
//  Hotplate=1; HandlerCondition.Data Shuttle Mode=1, Shuttle1 Cancel=0; AutoClean_Function=1, AutoClean_Tray=2 (Clean Kit).
//  Gerneral.ini USE_PICKER_COUNT=1 (ep8Picker, the 0928 machine copy) with only ZA enabled on each arm (Mot_Table M03 / M22),
//  Type_HT9046_LS (=> golden bUse8Picker). MADE UP (not in that recipe folder, or on the machine only -- spec §6): the site
//  pitch 80 / 60 mm and 2 x 2, the Clean Kit start / pitch / division, every Tech teach value, the offsets, the Z sub values.
//  Every expected number is worked out in the comment next to it from the golden formula the spec cites -- NOT by calling
//  the code under test a second time.
//  Sections: [1] helpers  [2] catalog  [3] In areas  [4] Out areas  [5] D1 ep1 vs ep8  [6] golden quirks  [7] refusals
//  CONTROL: each check was run against a broken variant once (see the report) -- e.g. dropping D1 brings back the +8000.
//  Pure: no file, no global, no god-stack; -Wall -Wextra.
// =============================================================================
#include "ArmCellPlan.h"

#include <cstdio>
#include <string>

using namespace ht9045;

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                         \
    do {                                                                         \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }

static ArmCellAxis Ax(int mi, const char* alias, bool is1203, bool enable)
{
    ArmCellAxis a;
    a.mi = mi; a.alias = alias; a.present = true; a.is1203 = is1203; a.enable = enable;
    return a;
}

static ArmCellForm TrayType0()
{
    ArmCellForm f;
    f.xStart = 2515.0; f.yStart = 2310.0; f.xPitch = 4280.0; f.yPitch = 3840.0; f.xDivision = 3; f.yDivision = 8;
    f.zDepth = 12.19;
    f.blockXStart = 10.0; f.blockYStart = 10.0; f.blockPitchX = 10.0; f.blockPitchY = 10.0;   // Tray.Data Block*=0.100 (x100)
    f.blockXItem = 2; f.blockYItem = 5;                                         // !bP06 zeroes these (SetTechDataToProd_TrayArm)
    return f;
}

// the HT9050-like baseline
static ArmCellInputs Ht9050()
{
    ArmCellInputs in;
    in.usePickerCount = kAcEp8; in.pickerUse = kAcPickMot;
    in.hotPlatePos0 = true; in.use8Picker = true; in.zSafePos = 50;
    in.ht9050Type = true;                // review M1: W906_GpibModel 9050GPIB -> ArmCellIsHt9050 (the type decodes as Type_HT9046_LS)
    in.hotPlateExpansion = 1.0;
    in.siteXPitch = 8000.0; in.siteYPitch = 6000.0; in.shtRow = 2; in.shtCol = 2; in.shuttleMode = 1; in.shuttleSel = 0;
    in.acFunction = 1; in.acTray = kAcCKPosCleanKit; in.acXStart = 6000.0; in.acYStart = 3000.0; in.acXPitch = 4000.0;
    in.acYPitch = 4000.0; in.acXDivision = 2; in.acYDivision = 2; in.armYPitch = 6000; in.acPadThickness = 10; in.acTeachPickZ = -2200;
    in.loadForm = TrayType0();
    in.hotPlateForm.xStart = 6000.0; in.hotPlateForm.yStart = 3000.0; in.hotPlateForm.xPitch = 4000.0; in.hotPlateForm.yPitch = 4000.0;
    in.hotPlateForm.xDivision = 2; in.hotPlateForm.yDivision = 8; in.hotPlateForm.plateSelect = 2; in.hotPlateForm.useWideHotplate = true;
    for (int t = 0; t < kAcTrayCount; ++t) in.autoForm[t] = TrayType0();
    in.userDefFileZDepth[0] = 12.19;
    in.plateSelect[0] = true; in.plateSelect[1] = false;                         // Using Flag=2: HP2 only ([0] = HP2, golden)
    for (int t = kAcAuto1; t <= kAcAuto1 + 2; ++t) in.trayType[t] = kAcTrayAuto;
    for (int t = kAcFix1; t <= kAcFix3; ++t) in.trayType[t] = kAcTrayFix;
    in.fixRightIsFix3 = true;
    // Tech (made up, see the file head)
    in.inLoadX = 10000; in.inLoadY = -50000; in.inLoadZ = -2000;
    in.inPlate2X = 30000; in.inPlate2Y = -20000; in.inPlate1X = 1000; in.inPlate1Y = -1000; in.inPlateZ = -1500;
    in.inShtX[0] = 50000; in.inShtY[0] = -30000; in.inShtZ = -1800;
    in.inACX = 70000; in.inACY = -10000;
    in.outAutoX[0] = -54127; in.outAutoY[0] = -72735;
    in.outFixX[0] = -44650;  in.outFixY[0] = -18536;
    in.outPlaceZ2 = -2000; in.outPlaceFixZ1 = -2100; in.outPlaceFix2Z1 = -2600;
    in.outShtX[0] = -48821; in.outShtY[0] = -53632; in.outShtPickZ2 = -2500;
    in.techInShtLeft[0] = 1000; in.techInShtRight[0] = 90000; in.shLeftPod[0] = 0.5; in.shRightPod[0] = 0.0;
    in.techOutShtRight[0] = 4321;
    // offsets: the Loader one has an X / Y offset, OneByOne off (golden GetArmX returns -dArmX then)
    in.inOfsLoader.x = 15.0; in.inOfsLoader.y = -25.0; in.inOfsLoader.pickUp = -30.0; in.inOfsLoader.pickUpRC[0][0] = 5.0;
    in.inOfsInSh[0].x = 12.7; in.inOfsInSh[0].y = -3.9;
    // axes
    in.inShuttle[0] = Ax(11, "MInShutte1", true, true);    // the Mot_Table alias typo is the machine's
    in.inShuttle[1] = Ax(12, "MInShutte2", false, false);
    in.outShuttle[0] = Ax(17, "MOutShuttle1", true, true);
    in.outShuttle[1] = Ax(18, "MOutShuttle2", true, true);
    const char* const zin[2][4]  = { { "MInArmZA", "MInArmZC", "MInArmZE", "MInArmZG" }, { "MInArmZB", "MInArmZD", "MInArmZF", "MInArmZH" } };
    const char* const zout[2][4] = { { "MOutArmZA", "MOutArmZC", "MOutArmZE", "MOutArmZG" }, { "MOutArmZB", "MOutArmZD", "MOutArmZF", "MOutArmZH" } };
    for (int arm = 0; arm < 2; ++arm) {
        ArmCellArmInputs& a = in.arm[arm];
        a.xBase = 2; a.yBase = 0;                                               // iXPitch60 / ep8 keep the default base Ac = [0][2]
        a.shtXCenter = -2000; a.shtYCenter = -3000;
        a.motRow = 2; a.motCol = 4; a.maxRow = 2; a.maxCol = 4;                 // ep8Picker: the 2 x 4 head
        a.x = arm == 0 ? Ax(0, "MInArmX", true, true) : Ax(19, "MOutArmX", true, true);
        a.y = arm == 0 ? Ax(1, "MInArmY", true, true) : Ax(20, "MOutArmY", true, true);
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 4; ++j)
                a.z[i][j] = (i == 0 && j == 0) ? Ax(arm == 0 ? 3 : 22, arm == 0 ? "MInArmZA" : "MOutArmZA", true, true)
                                               : Ax(arm == 0 ? 4 + i + 2 * j : 23 + i + 2 * j, arm == 0 ? zin[i][j] : zout[i][j], false, false);
        a.pitch.push_back(Ax(arm == 0 ? 2 : 21, arm == 0 ? "MInArmPitch" : "MOutArmPitch", false, false));
        a.zHeightSub[0][0] = (arm == 0) ? 10 : 110;                             // [InArmZSub] / [OutArmZSub] Picker Aa
    }
    return in;
}

static ArmCellRequest Req(const char* arm, const char* area, int col, int row, bool zDown = false)
{
    ArmCellRequest q;
    q.arm = arm; q.area = area; q.col = col; q.row = row; q.zDown = zDown;
    return q;
}

static const ArmCellArea* FindArea(const ArmCellCatalog& c, const char* id)
{
    for (std::size_t i = 0; i < c.areas.size(); ++i) if (c.areas[i].id == id) return &c.areas[i];
    return 0;
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    // ------------------------------------------------------------------ [1]
    std::printf("[1] golden helpers\n");
    CHECK(ArmCellShtStartPos(2, 48000, 8000) == 44000, "GetShtStartPos(2 items, 48000, 8000) = 48000 - (int)(0.5*8000) = 44000");
    CHECK(ArmCellShtStartPos(2, -33000, -6000) == -30000, "GetShtStartPos(2 items, -33000, -6000) = -33000 - (int)(0.5*-6000) = -30000");
    CHECK(ArmCellShtStartPos(3, 1000, 7) == 993, "GetShtStartPos(3 items, 1000, 7) = 1000 - (int)(1.0*7) = 993");
    CHECK(ArmCellShtStartPos(4, 0, 5) == -7, "GetShtStartPos(4 items, 0, 5) = 0 - (int)(1.5*5 = 7.5) = -7 (truncation)");
    CHECK(ArmCellFloatQuotient(1, 0) == 0.0f, "ChangeToFloatNonPcnt(1, 0) = 0");
    CHECK(ArmCellFloatQuotient(2, 3) == (float)(2.0 / 3.0), "ChangeToFloatNonPcnt(2, 3) is a float");
    CHECK(ArmCellIsHt9050("9050GPIB", false) && ArmCellIsHt9050("", true) && !ArmCellIsHt9050("9046_32GPIB", false) && !ArmCellIsHt9050("", false),
          "review M1: ArmCellIsHt9050 = W906_GpibModel 9050GPIB (decodes as Type_HT9046_LS) || Type_HT9050 (WebBridgeTags.cpp W906_MainCaption)");
    {
        ArmCellOffset o; o.x = 15.0; o.y = -25.0; o.posX[0][0] = 7.0; o.posY[0][0] = -2.0;
        CHECK(o.ArmX(0, 0) == -15.0 && o.ArmY(0, 0) == 25.0, "golden GetArmX/Y with OneByOne off = -dArmX / -dArmY (cprod.h:209-222)");
        o.oneByOne = true;
        CHECK(o.ArmX(0, 0) == 7.0 && o.ArmY(0, 0) == -2.0, "golden GetArmX/Y with OneByOne on = the nozzle's own value");
    }

    // ------------------------------------------------------------------ [2]
    std::printf("[2] catalog (HT9050-like)\n");
    {
        ArmCellCatalog c;
        ArmCellBuildCatalog(Ht9050(), c);
        CHECK(c.nozzles[0].size() == 1 && c.nozzles[0][0].z.alias == "MInArmZA" && c.nozzles[0][0].i == 0 && c.nozzles[0][0].j == 0,
              "In nozzles = the Enable Z only: MInArmZA [0][0] (Q4 = A)");
        CHECK(c.nozzles[1].size() == 1 && c.nozzles[1][0].z.alias == "MOutArmZA", "Out nozzles = MOutArmZA only");
        CHECK(c.armWhy[0].empty() && c.armWhy[1].empty(), "both arms pass the v1 arm check");
        CHECK(Has(c.note[0], "USE_PICKER_COUNT=1") && Has(c.note[0], "2×4") && Has(c.note[0], "MInArmZA") && Has(c.note[0], "D1"),
              "D1 note: ini head 2x4, only ZA, single-nozzle math");
        CHECK(c.areas.size() == 22, "22 golden areas listed (Q5 = C)");
        const char* v1[] = { "Loader", "HotPlate2", "InShuttle1", "CleanKit", "OutShuttle1", "Auto1", "Auto2", "Auto3", "Fix1", "Fix2", "Fix3" };
        int usable = 0;
        for (int k = 0; k < 11; ++k) { const ArmCellArea* a = FindArea(c, v1[k]); if (a && a->usable && a->v1) ++usable; else std::printf("    not usable: %s %s\n", v1[k], a ? a->why.c_str() : "missing"); }
        CHECK(usable == 11, "the 11 HT9050 areas (RULINGS_20261002 #18) are usable");
        int grey = 0;
        for (std::size_t i = 0; i < c.areas.size(); ++i) if (!c.areas[i].usable) { ++grey; if (c.areas[i].why.empty()) std::printf("    greyed without a reason: %s\n", c.areas[i].id.c_str()); }
        CHECK(grey == 11, "the other 11 are greyed");
        bool allWhy = true;
        for (std::size_t i = 0; i < c.areas.size(); ++i) if (!c.areas[i].usable && c.areas[i].why.empty()) allWhy = false;
        CHECK(allWhy, "every greyed area carries a reason (灰掉寫原因)");
        const ArmCellArea* hp1 = FindArea(c, "HotPlate1");
        CHECK(hp1 && !hp1->installed && Has(hp1->why, "bPlateSelect[1]"), "HotPlate1: not installed, Using Flag reason (q1: [1] = HP1)");
        const ArmCellArea* is2 = FindArea(c, "InShuttle2");
        CHECK(is2 && !is2->installed && Has(is2->why, "MInShutte2") && Has(is2->why, "Enable=0"), "InShuttle2: MInShutte2 Enable=0 (key by index, alias as the table writes it)");
        const ArmCellArea* os2 = FindArea(c, "OutShuttle2");
        CHECK(os2 && !os2->usable && !os2->why.empty(), "OutShuttle2 greyed with a reason");
        const ArmCellArea* a4 = FindArea(c, "Auto4");
        CHECK(a4 && !a4->installed && Has(a4->why, "eAuto4"), "Auto4: not an Auto tray here (AUTO_EMPTY_COLOR)");
        const ArmCellArea* f4 = FindArea(c, "Fix4");
        CHECK(f4 && Has(f4->label, "Fix 1 下半"), "Fix4 on a 3-tray layout is labelled the lower half of Fix1 (spec §4.2)");
        const ArmCellArea* bb = FindArea(c, "BinBox");
        const ArmCellArea* mg = FindArea(c, "Magazine");
        CHECK(bb && !bb->zDownAllowed && Has(bb->zDownWhy, "ZPlace[eBulkBox]") && mg && !mg->zDownAllowed, "q6: BinBox / Magazine Z down disabled");
        const ArmCellArea* ld = FindArea(c, "Loader");
        CHECK(ld && ld->cols == 3 && ld->rows == 8 && ld->zKind == "pick", "Loader 3 x 8 (block items 1 when !bP06), Z = pick");
        const ArmCellArea* ish = FindArea(c, "InShuttle1");
        CHECK(ish && ish->cols == 2 && ish->rows == 2 && ish->zDownAllowed && Has(ish->zDownWhy, "D2"), "InShuttle1 2 x 2, Z down only with D2");
        const ArmCellArea* osh = FindArea(c, "OutShuttle1");
        CHECK(osh && osh->zKind == "pick", "OutShuttle1 Z = pick (spec §4.4)");
        const ArmCellArea* ck = FindArea(c, "CleanKit");
        CHECK(ck && ck->cols == 2 && ck->rows == 2 && ck->zDownAllowed, "CleanKit 2 x 2, Z down allowed (bUse8Picker)");
    }

    // ------------------------------------------------------------------ [3]
    std::printf("[3] In Arm areas (FT005054_9050)\n");
    {
        ArmCellPlan p; std::string why;
        // Loader (3, 8) 1-based = col 2, row 7. base X = 10000 + 2515 + 0 (block, !P06) - 1000 + 15 + 0 = 11530;
        // base Y = -50000 - 2310 - 0 + 1000 + (-25) - 0 = -51335. cell X = 11530 + 2*4280 = 20090, Y = -51335 - 7*3840 = -78215;
        // + GetArmX/Y (OneByOne off: -15 / +25) -> (20075, -78190). Z = -2000 + (-30) = -2030 base; ZA != base Ac -> + 10 + 5 = -2015.
        const bool ok = ArmCellBuildPlan(Ht9050(), Req("in", "Loader", 2, 7, true), p, why);
        CHECK(ok, "Loader (3,8) planned");
        if (!ok) std::printf("    why: %s\n", why.c_str());
        CHECK(p.baseX == 11530 && p.baseY == -51335, "Loader base = Tech + XStart - KitPitch + offset (block zeroed: !bP06)");
        CHECK(p.xTarget == 20075 && p.yTarget == -78190, "Loader (3,8) target (20075, -78190): golden GetArmX quirk takes the 15 / -25 back out (f1)");
        CHECK(p.zTarget == -2015 && p.zKind == "pick", "Loader Z pick = -2030 + ZHeightSub[0][0] 10 + GetPickUp(0,0) 5 = -2015");
        CHECK(p.zSafe == 50 && p.zLift.size() == 1 && p.zLift[0].alias == "MInArmZA", "S1 lifts every Enable Z to the runtime ZSafePos (50)");
        CHECK(p.teachX == 10000 && p.teachY == -50000 && p.teachZ == -2000, "raw teach point echoed");
        CHECK(!p.d1Note.empty() && !p.notes.empty() && p.notes[0] == p.d1Note, "the plan carries the D1 note first");
        // Loader (1,1): (11530 - 15, -51335 + 25)
        ArmCellBuildPlan(Ht9050(), Req("in", "Loader", 0, 0), p, why);
        CHECK(p.xTarget == 11515 && p.yTarget == -51310, "Loader (1,1) = (11515, -51310) = LoadStageX + 1515, LoadStageY - 1310 (spec §4.2 + f1)");
        // HotPlate2 (2,8): base X = 30000 + 6000 - 9000 + 0 = 27000, Y = -20000 - 3000 + 1000 + 0 = -22000;
        // cell X = 27000 + 4000*1 = 31000, Y = -22000 - 4000*7 = -50000; expansion 1.0 -> identity. Z place = -1500 + 10 + 200 + 0 + 0 = -1290.
        CHECK(ArmCellBuildPlan(Ht9050(), Req("in", "HotPlate2", 1, 7, true), p, why), "HotPlate2 (2,8) planned");
        CHECK(p.baseX == 27000 && p.baseY == -22000 && p.xTarget == 31000 && p.yTarget == -50000, "HotPlate2 (2,8) = (31000, -50000): Plate2X - 3000 + 4000*col, Plate2Y - 2000 - 4000*row");
        CHECK(p.zTarget == -1290 && p.zKind == "place", "HotPlate2 Z place = PickZ2 + ZSub 10 + 200 (+ offsets 0) = -1290");
        // InShuttle1 (2,2): centre X = -2000 + 50000 = 48000 -> start 48000 - 4000 = 44000 -> + 8000 = 52000; + (int)12.7 = 52012
        //   centre Y = -3000 - 30000 = -33000 -> start -33000 + 3000 = -30000 -> - 6000 = -36000; + (int)-3.9 = -36003. Z = -1800 + 10 = -1790.
        CHECK(ArmCellBuildPlan(Ht9050(), Req("in", "InShuttle1", 1, 1, true), p, why), "InShuttle1 (2,2) planned");
        CHECK(p.xTarget == 52012 && p.yTarget == -36003, "InShuttle1 (2,2) = (52012, -36003): GetShtRowColPos + (int)offset");
        CHECK(p.zTarget == -1790, "InShuttle1 Z place = -1800 + 10 = -1790");
        CHECK(p.d2 && p.d2Axis.alias == "MInShutte1" && p.d2Target == 1000, "D2: MInShutte1 at Prod.InSHT[0].iLeft = (int)(1000 + 0.5 + 0) = 1000");
        ArmCellBuildPlan(Ht9050(), Req("in", "InShuttle1", 0, 0), p, why);
        CHECK(p.xTarget == 44012 && p.yTarget == -30003, "InShuttle1 (1,1) = Sht1X - 6000 + off, Sht1Y + off (spec §4.2)");
        // CleanKit (2,2): base X = 70000 + 6000 - 18000 + 0 = 58000, Y = -10000 - 3000 - 2250 + 0 = -15250;
        //   cell X = 58000 + 4000*1 = 62000, Y = -15250 - 4000*1 = -19250 (bUse8Picker: no +iARM_Y_PITCH for row A);
        //   Z place = -2200 + 200 + 0 = -2000 -> + 10 + 0 = -1990 -> + iArmPlaceTrayPos (10 + 0) = -1980.
        CHECK(ArmCellBuildPlan(Ht9050(), Req("in", "CleanKit", 1, 1, true), p, why), "CleanKit (2,2) planned");
        CHECK(p.baseX == 58000 && p.baseY == -15250 && p.xTarget == 62000 && p.yTarget == -19250, "CleanKit (2,2) = (62000, -19250): AutoCleanX - 12000 + 4000*col, Y - 5250 - 4000*row");
        CHECK(p.zTarget == -1980, "CleanKit Z = place -1990 + iPadThickness 10 = -1980");
    }

    // ------------------------------------------------------------------ [4]
    std::printf("[4] Out Arm areas\n");
    {
        ArmCellPlan p; std::string why;
        // Auto1 (3,8): XStart = -54127 + 2515 + 0 - 1000 + 0 + 0 = -52612; YStart = -72735 - 2310 - 0 + 1000 + 0 - 0 = -74045;
        //   X = -52612 + 2*4280 = -44052, Y = -74045 - 7*3840 = -100925; no iVariablePara term (D1: the ep8 head would add +8000);
        //   GetArmX/Y (OneByOne off, x = y = 0) -> 0. Z = -2000 + 0 = -2000 -> + 110 = -1890.
        CHECK(ArmCellBuildPlan(Ht9050(), Req("out", "Auto1", 2, 7, true), p, why), "Auto1 (3,8) planned");
        CHECK(p.baseX == -52612 && p.baseY == -74045, "Auto1 base = Tech + XStart - KitPitch + offset");
        CHECK(p.xTarget == -44052 && p.yTarget == -100925, "Auto1 (3,8) = (-44052, -100925): no +8000 (D1, single nozzle)");
        CHECK(p.zTarget == -1890 && p.zKind == "place", "Auto1 Z place = PlaceZ2 -2000 + ZSub 110 = -1890");
        // Fix1 (1,1): -44650 + 2515 - 1000 = -43135; -18536 - 2310 + 1000 = -19846. Z = -2100 (FixZ1) + 110 = -1990.
        CHECK(ArmCellBuildPlan(Ht9050(), Req("out", "Fix1", 0, 0, true), p, why), "Fix1 (1,1) planned");
        CHECK(p.xTarget == -43135 && p.yTarget == -19846 && p.zTarget == -1990, "Fix1 (1,1) = (-43135, -19846), Z = FixZ1 -2100 + 110");
        // OutShuttle1 (2,2): centre X = -2000 - 48821 = -50821 -> start -54821 -> + 8000 = -46821;
        //   centre Y = -3000 - 53632 = -56632 -> start -53632 -> - 6000 = -59632. Z pick = -2500 + 110 = -2390.
        CHECK(ArmCellBuildPlan(Ht9050(), Req("out", "OutShuttle1", 1, 1, true), p, why), "OutShuttle1 (2,2) planned");
        CHECK(p.xTarget == -46821 && p.yTarget == -59632 && p.zTarget == -2390 && p.zKind == "pick", "OutShuttle1 (2,2) = (-46821, -59632), Z pick -2390");
        // D2 on the HT9050 (review M1): its Out Arm picks from MOutShuttle1 (Mot_Table M17) -> golden OutSHT1InRT's Type_HT9050 arm,
        //   Prod.OutSHT[0].iRight = Tech.iOutShuttle1Right 4321 -- NOT the In shuttle (MInShutte1 / InSHT[0].iRight 90000)
        CHECK(p.d2 && p.d2Axis.alias == "MOutShuttle1" && p.d2Target == 4321, "D2 (HT9050, review M1): MOutShuttle1 at Prod.OutSHT[0].iRight = Tech 4321, not the In shuttle");
        ArmCellInputs not9050 = Ht9050(); not9050.ht9050Type = false;            // e.g. a real HT9046 LS: golden OutSHT1InRT -> InSHT1InRT
        ArmCellBuildPlan(not9050, Req("out", "OutShuttle1", 1, 1, true), p, why);
        CHECK(p.d2Axis.alias == "MInShutte1" && p.d2Target == 90000, "D2 (not an HT9050): MInShutte1 at Prod.InSHT[0].iRight 90000 (golden OutSHT1InRT else arm)");
        ArmCellInputs noOut = Ht9050(); noOut.outShuttle[0].enable = false;
        ArmCellCatalog oc; ArmCellBuildCatalog(noOut, oc);
        const ArmCellArea* o1 = FindArea(oc, "OutShuttle1");
        CHECK(o1 && !o1->installed && Has(o1->why, "MOutShuttle1"), "HT9050: Out Shuttle 1 greyed when MOutShuttle1 is Enable=0 (its D2 axis), whatever the In shuttle");
    }

    // ------------------------------------------------------------------ [5]
    std::printf("[5] D1: ep8 ini with only ZA == ep1 arithmetic\n");
    {
        const char* areas[][2] = { { "in", "Loader" }, { "in", "HotPlate2" }, { "in", "InShuttle1" }, { "in", "CleanKit" },
                                   { "out", "Auto1" }, { "out", "Fix2" }, { "out", "OutShuttle1" } };
        ArmCellInputs ep1 = Ht9050(); ep1.usePickerCount = kAcEp1;
        int same = 0;
        for (int k = 0; k < 7; ++k) {
            ArmCellPlan a, b; std::string w1, w2;
            const bool o1 = ArmCellBuildPlan(Ht9050(), Req(areas[k][0], areas[k][1], 1, 1, true), a, w1);
            const bool o2 = ArmCellBuildPlan(ep1, Req(areas[k][0], areas[k][1], 1, 1, true), b, w2);
            if (o1 && o2 && a.xTarget == b.xTarget && a.yTarget == b.yTarget && a.zTarget == b.zTarget) ++same;
            else std::printf("    %s: ep8 (%d,%d,%d) ep1 (%d,%d,%d) %s %s\n", areas[k][1], a.xTarget, a.yTarget, a.zTarget, b.xTarget, b.yTarget, b.zTarget, w1.c_str(), w2.c_str());
        }
        CHECK(same == 7, "7 areas: ep8 + one Enable Z gives the ep1 targets (D1: every multi-nozzle term 0)");
        ArmCellPlan b; std::string w;
        ArmCellBuildPlan(ep1, Req("out", "Auto1", 0, 0), b, w);
        CHECK(b.d1Note.empty(), "ep1Picker: no D1 note");
    }

    // ------------------------------------------------------------------ [6]
    std::printf("[6] golden quirks kept\n");
    {
        ArmCellPlan p; std::string why;
        // f1: OneByOne on -> the nozzle's own X / Y are added on top of the table offset
        ArmCellInputs in = Ht9050();
        in.inOfsLoader.oneByOne = true; in.inOfsLoader.posX[0][0] = 7.0; in.inOfsLoader.posY[0][0] = -2.0;
        ArmCellBuildPlan(in, Req("in", "Loader", 0, 0), p, why);
        CHECK(p.xTarget == 11530 + 7 && p.yTarget == -51335 - 2, "f1: OneByOne on -> base (with the table's 15 / -25) + GetArmX(0,0) 7 / GetArmY -2");
        // q3: block mode + bP06, synthetic form (exact quotients): xDivision 2, yDivision 4, col 1, row 2 -> 0.5 / 0.5
        in = Ht9050();
        in.trayBlockMode = true; in.p06 = true;
        ArmCellForm f; f.xStart = 0; f.yStart = 0; f.xPitch = 1000; f.yPitch = 1000; f.xDivision = 2; f.yDivision = 4;
        f.blockPitchX = 3000; f.blockPitchY = 5000; f.blockXStart = 0; f.blockYStart = 0;
        in.loadForm = f;
        for (int t = 0; t < kAcTrayCount; ++t) in.autoForm[t] = f;
        in.inOfsLoader = ArmCellOffset();
        // Loader: base X = 10000 + 0 + 0 - 1000 = 9000; X = 9000 + 1000 = 10000 (the block X is computed and lost);
        //   Y: base -50000 + 1000 = -49000; -49000 - 2*1000 = -51000; block Y: - 0.5*5000 + 0.5*(1000*4) = -51000 - 2500 + 2000 = -51500.
        ArmCellBuildPlan(in, Req("in", "Loader", 1, 2), p, why);
        CHECK(p.xTarget == 10000 && p.yTarget == -51500, "q3 Loader: X block pitch lost, Y kept once");
        // Auto1: XStart -54127 + 0 - 1000 = -55127; X = -55127 + 1000 = -54127; block once on X: + 0.5*3000 - 0.5*2000 = +500 -> -53627;
        //   YStart -72735 + 1000 = -71735; Y = -71735 - 2000 = -73735; block twice on Y: 2 * (-2500 + 2000) = -1000 -> -74735.
        ArmCellBuildPlan(in, Req("out", "Auto1", 1, 2), p, why);
        CHECK(p.xTarget == -53627 && p.yTarget == -74735, "q3 Auto1: X block pitch once, Y twice (OutArmAddBlockPitch on AutoForm[0])");
        // q4: HotPlate2 expansion 1.01: X 27000 + 4000*1.01 = 31040 (d 40); Y -22000 - 28000*1.01 = -50280 (d 280 > 200) -> golden
        //   writes *iXPos=iOldX: X back to 31000, Y stays -50280.
        in = Ht9050(); in.hotPlateExpansion = 1.01;
        ArmCellBuildPlan(in, Req("in", "HotPlate2", 1, 7), p, why);
        CHECK(p.xTarget == 31000 && p.yTarget == -50280, "q4: TransferHotPlateRatio's Y clamp resets X (Y keeps -50280)");
        // f2: setup-file scales 1.0 + expansion 1.002: Plate2 X expands (4000*1.002 = 4008 -> 31008), Plate2 Y does not (-50000)
        in = Ht9050(); in.hotPlateExpansion = 1.002; in.setupFileScale = true;
        in.hpXScaleFile[0] = in.hpXScaleFile[1] = 1.0; in.hpYScaleFile[0] = in.hpYScaleFile[1] = 1.0;
        ArmCellBuildPlan(in, Req("in", "HotPlate2", 1, 7), p, why);
        CHECK(p.xTarget == 31008 && p.yTarget == -50000, "f2: Plate2's Y skips the expansion outside its last else");
        // f3: NS7000 shift -- In Shuttle 1 adds SiteYPitch/2 (3000), the Out shuttle subtracts it
        in = Ht9050(); in.ns7000Kit = true;
        ArmCellBuildPlan(in, Req("in", "InShuttle1", 0, 0), p, why);
        CHECK(p.yTarget == -30003 + 3000, "f3: NS7000 + MInShuttle1 -> Y + 3000");
        ArmCellBuildPlan(in, Req("out", "OutShuttle1", 0, 0), p, why);
        CHECK(p.yTarget == -53632 - 3000, "f3: NS7000 + an out shuttle -> Y - 3000");
        // f3: TransferOutShuttleRatio setup-file branch scales Y with the X factor: X scale 1.5, Y scale 1.0 (ignored)
        in = Ht9050(); in.setupFileScale = true; in.outShtXScaleFile[0] = 1.5;
        ArmCellBuildPlan(in, Req("out", "OutShuttle1", 1, 1), p, why);
        // base (-48821, -53632); cell (-46821, -59632): dX 2000*1.5 = 3000 -> -45821; dY -6000*1.5 = -9000 -> -62632
        CHECK(p.xTarget == -45821 && p.yTarget == -62632, "f3: out-shuttle setup-file Y scaled by the X factor");
        // f4: bUse8Picker false -> Clean Kit Y + iARM_Y_PITCH (6000) for every row, and row-A Z down not allowed
        in = Ht9050(); in.use8Picker = false;
        ArmCellBuildPlan(in, Req("in", "CleanKit", 1, 1), p, why);
        CHECK(p.yTarget == -19250 + 6000 && !p.zDownAllowed && Has(p.zDownWhy, "B 排"), "f4: !bUse8Picker -> Y + 6000, ZA (row A) cannot go down");
        // f5: HotPlate place Z with E33 + E34: the per-nozzle offset from InOfsLoader, the plate offset from InOfsHP1 for HP2 too
        in = Ht9050(); in.e33 = true; in.e34 = true; in.inOfsLoader.placeRC[0][0] = 3.0; in.inOfsHP1.place = 40.0; in.inOfsHP2.place = 900.0;
        ArmCellBuildPlan(in, Req("in", "HotPlate2", 0, 0, true), p, why);
        CHECK(p.zTarget == -1500 + 10 + 200 + 3 + 40, "f5: E33 + E34 -> HP2 place Z uses InOfsLoader(0,0) + InOfsHP1 (not HP2's 900)");
        // thick tray: CosFunction + E70: Loader Z + (12.19*100 - 635) = + 584 (1219 >= 600)
        in = Ht9050(); in.trayThickAdjustZ = true; in.e70 = true;
        ArmCellBuildPlan(in, Req("in", "Loader", 0, 0, true), p, why);
        CHECK(p.zTarget == -2015 + 584, "Loader thick tray: + (ZDepth*100 - 635) = +584");
        in = Ht9050(); in.e88 = true;
        ArmCellBuildPlan(in, Req("in", "Loader", 0, 0, true), p, why);
        CHECK(p.zTarget == -2015 - 200, "Loader E88: -200");
    }

    // ------------------------------------------------------------------ [7]
    std::printf("[7] refusals (v1 scope, D1 limits, request checks)\n");
    {
        ArmCellPlan p; std::string why;
        CHECK(!ArmCellBuildPlan(Ht9050(), Req("in", "HotPlate1", 0, 0), p, why) && Has(why, "HotPlate 1"), "HotPlate1 refused (not installed here)");
        ArmCellInputs both = Ht9050(); both.plateSelect[1] = true;
        CHECK(!ArmCellBuildPlan(both, Req("in", "HotPlate1", 0, 0), p, why) && Has(why, "第一版未做"), "HotPlate1 installed elsewhere -> still refused in v1 (第一版未做)");
        CHECK(!ArmCellBuildPlan(Ht9050(), Req("out", "BinBox", 0, 0), p, why), "BinBox refused");
        CHECK(!ArmCellBuildPlan(Ht9050(), Req("in", "Loader", 3, 0), p, why) && Has(why, "超出範圍"), "col 4 of 3 refused");
        CHECK(!ArmCellBuildPlan(Ht9050(), Req("in", "Loader", 0, 8), p, why), "row 9 of 8 refused");
        CHECK(!ArmCellBuildPlan(Ht9050(), Req("in", "Loader", -1, 0), p, why), "col 0 (1-based) refused");
        ArmCellRequest q = Req("in", "Loader", 0, 0); q.nozzle = "MInArmZE";
        CHECK(!ArmCellBuildPlan(Ht9050(), q, p, why) && Has(why, "MInArmZA"), "a nozzle that is not the Enable Z refused");
        CHECK(!ArmCellBuildPlan(Ht9050(), Req("out", "Loader", 0, 0), p, why), "an In area on the Out arm refused");
        CHECK(!ArmCellBuildPlan(Ht9050(), Req("up", "Loader", 0, 0), p, why), "arm other than in / out refused");
        ArmCellInputs in = Ht9050(); in.arm[0].pitch[0].enable = true;
        CHECK(!ArmCellBuildPlan(in, Req("in", "Loader", 0, 0), p, why) && Has(why, "pitch"), "an Enable pitch axis -> refused (v1)");
        in = Ht9050(); in.arm[1].z[0][1].enable = true; in.arm[1].z[0][1].is1203 = true;
        CHECK(!ArmCellBuildPlan(in, Req("out", "Auto1", 0, 0), p, why) && Has(why, "2 支"), "two Enable Z -> refused (multi-nozzle, v1)");
        ArmCellCatalog c; ArmCellBuildCatalog(in, c);
        CHECK(c.nozzles[1].size() == 2 && !c.armWhy[1].empty(), "catalog: two nozzles listed, the arm refused with a reason");
        in = Ht9050(); in.usePickerCount = kAcEp16; in.pickerUse = kAcPickMotCyn;
        CHECK(!ArmCellBuildPlan(in, Req("in", "Loader", 0, 0), p, why) && Has(why, "eptUseMotCyn"), "ep16 + eptUseMotCyn refused");
        in = Ht9050(); in.arm[0].yAutoPitch = true;
        CHECK(!ArmCellBuildPlan(in, Req("in", "Loader", 0, 0), p, why) && Has(why, "Y pitch"), "automatic Y pitch refused (v1)");
        in = Ht9050(); in.aoa = true;
        CHECK(!ArmCellBuildPlan(in, Req("out", "Auto1", 0, 0), p, why) && Has(why, "AOA"), "AOA refused (v1)");
        in = Ht9050(); in.arm[0].x.is1203 = false;
        CHECK(!ArmCellBuildPlan(in, Req("in", "Loader", 0, 0), p, why) && Has(why, "PCI1203"), "a non-1203 arm axis refused (v1)");
        in = Ht9050(); in.loaderRatioUngated = true;
        CHECK(!ArmCellBuildPlan(in, Req("in", "Loader", 0, 0), p, why) && Has(why, "TransferLoaderRatio"), "Loader refused once W906_TRANSFER_LOADER_RATIO is 1 (its copy is not written)");
        CHECK(ArmCellBuildPlan(in, Req("in", "HotPlate2", 0, 0), p, why), "... the other areas still plan");
        in = Ht9050(); in.shuttleSel = 1;
        CHECK(!ArmCellBuildPlan(in, Req("in", "InShuttle1", 0, 0), p, why) && Has(why, "Shuttle Mode=1"), "single-shuttle mode on shuttle 2 -> InShuttle1 refused");
        in = Ht9050(); in.acTray = kAcCKPosHP2;
        CHECK(!ArmCellBuildPlan(in, Req("in", "CleanKit", 0, 0), p, why) && Has(why, "Clean pad"), "clean pads on HotPlate 2 -> CleanKit refused");
    }

    std::printf("\nRESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
