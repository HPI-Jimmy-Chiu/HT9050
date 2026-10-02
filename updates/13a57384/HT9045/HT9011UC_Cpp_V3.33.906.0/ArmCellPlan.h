// =============================================================================
//  ArmCellPlan.h  --  the Teach page "Arm Cell" tab: where a chosen In / Out Arm nozzle has to go to sit over a chosen
//                     tray cell (pure arithmetic, no global, no motion)
//
//  AI(W906-ARMCELL) 20261002 [W906]: RULINGS_20261002 #18 (NB2 spec RD5軟體_NB2規格_Teach頁ArmCell分頁_20261002_101914.md,
//  on origin/v906/nb2-assist; §4 is the formula part). golden has NO such button -- the HT160 equivalent is Teach ->
//  Advanced -> Sort Arm "Pick / Place Test" (HT160S uteach.cpp btnSaGoClick :1537, aSortArm.cpp MoveSuckerToCell
//  :2819-2905). What IS golden here are the production formulas the arm uses to reach a cell, copied into locals:
//
//    base point   = the [iIn/OutArmYBase][iIn/OutArmXBase] slot SetTechDataToProd_InArm / _OutArm would write NOW
//                   (golden cinitial.cpp:8572-9256 / :9511-10080; port cinitial.cpp:14756-15240 / :11432-11961),
//                   recomputed from the current Tech (a point re-taught in this Teach session is not in Prod yet --
//                   golden does not refresh Prod when Teach closes). Prod's current value is echoed next to it.
//    cell         = the single-nozzle movers: Loader GetInArmToLoaderPosition_Single + MoveInArmXYToLoader_9045
//                   (ainarm9045.cpp:8231 / :9139), HotPlate MoveInArmXYToHotPlatePlace (ainarm_SearchPlacePlate.cpp:1823),
//                   In Shuttle CheckXYPitch_All_1Pick (ainarm9045_All_1Pick.cpp:246) + GetShtRowColPos
//                   (ainarm9045.cpp:630-720), Clean Kit MoveInArmXYPickCleanKit (AutoClean/AutoClean.cpp:2129),
//                   Auto / Fix GetOutArmToUnLoaderPosition single branch (aoutarm.cpp:3898-4054), Out Shuttle
//                   CheckOutArmXYPitch_All_1Picker (aoutarm9045_All_1Picker.cpp:363).
//    compensation = TransferHotPlateRatio / TransferInShuttleRatio (ainarm2.cpp:5909 / :6185), TransferAutoRatio /
//                   TransferOutShuttleRatio (aoutarm.cpp:3328 / :3374), with the plate / shuttle passed explicitly.
//                   TransferLoaderRatio is gated off in V906 production (ainarm9045.cpp GATE k4-G4,
//                   cInArmPlacement.cpp GATE (1)) -- the three places share MachineType.h W906_TRANSFER_LOADER_RATIO.
//    Z (zDown)    = Loader pick Z, HotPlate / In Shuttle / Clean Kit / Auto / Fix place Z, Out Shuttle pick Z
//                   (NB2 spec §4.4), each the production per-nozzle value.
//
//  Rulings that shape it (RULINGS_20261002 #18, the user's words in the spec §1):
//    D1  one Enable Z on the arm => single-nozzle arithmetic: every multi-nozzle X/Y term is 0 ((iXBase-c)*step,
//        iVariablePara, HotPlate (2-c)*pitch/3, the nozzle-row Y term), whatever Gerneral.ini USE_PICKER_COUNT says;
//        the plan carries a one-line note when the ini head is not ep1Picker.
//    v1  only the areas HT9050 has (In Loader / HotPlate2 / InShuttle1 / CleanKit, Out OutShuttle1 / Auto1-3 / Fix1-3);
//        every other golden area is listed and refused with a reason. A machine with a pitch axis, ep16 + eptUseMotCyn,
//        cylinder pickers, more than one Enable Z, an automatic Y pitch, AOA, or a non-PCI1203 arm axis is refused
//        (9050 has none of them).
//    Q6  no anti-collision beyond golden teach (the job keeps golden's Z-at-home check); D2 (shuttle Z down only with
//        the shuttle parked at the arm side) is evaluated by the job on a fresh sample, the plan names the axis.
//
//  Units: every value is a user unit (1/100 mm), like Prod / Tech. The job converts once with MotorUserToCard.
//  Arithmetic: golden's int / double mix is kept operand by operand (a double sum assigned to an int truncates, like
//  BCB6); the quirks golden has are kept and listed in ArmCellPlan.cpp (NB2 spec §4.3 plus the ones found copying).
//
//  Linkage: this header is included by WebMotorAccess.h (types only); ArmCellPlan.cpp is linked into wb_serve and
//  its own ctest (tests/test_armcell_plan.cpp). ArmCellLive.cpp (wb_serve only) fills ArmCellInputs from the
//  god-stack on the tick thread. Nothing here may include a god-stack header.
// =============================================================================
#ifndef HT9045_ARMCELLPLAN_H
#define HT9045_ARMCELLPLAN_H

#include <string>
#include <vector>

namespace ht9045 {

// golden e6TrayName (MachineType.h:1191-1225), eCKPos (:1059-1061), eTrayType (:701-707), ePickCount / ePickType
// (:1430-1442) -- ArmCellLive.cpp static_asserts each one against the golden enum, so a renumbering there breaks the
// build, not the arm.
enum {
    kAcAuto1 = 0, kAcAuto6 = 5, kAcFix1 = 6, kAcFix2 = 7, kAcFix3 = 8, kAcFix6 = 11, kAcBulkBox = 18, kAcMag1 = 19,
    kAcMag14 = 32, kAcTrayCount = 33,
    kAcCKPosHP2 = 1, kAcCKPosCleanKit = 2,
    kAcTrayNotUse = 0, kAcTrayAuto = 1, kAcTrayFix = 2, kAcTrayMag = 3, kAcTrayBox = 4,
    kAcEp4 = 0, kAcEp8 = 1, kAcEp2 = 2, kAcEp16 = 3, kAcEp1 = 4,
    kAcPickCyn = 0, kAcPickMot = 1, kAcPickMotCyn = 2,
    kAcKitPitch = 1000                                  // cmydef.h:48-49 KitPitchX / KitPitchY
};

// One golden ARM_OFFSET (cprod.h:162-232) as the live side copied it.
struct ArmCellOffset {
    bool   oneByOne = false;
    double x = 0.0, y = 0.0, pickUp = 0.0, place = 0.0;      // dArmX / dArmY / dPickUp / dPlaceUp
    double posX[4][8] = {}, posY[4][8] = {}, pickUpRC[4][8] = {}, placeRC[4][8] = {};   // SingleOffSet->dPosOffSetX/Y, dPickUpOffSet, dPlaceOffSet
    // golden GetArmX / GetArmY (cprod.h:209-222): OneByOne ? the nozzle's own value : -dArmX / -dArmY (sic: golden quirk,
    // the single-nozzle movers then take the table's X / Y back out of the base point -- kept)
    double ArmX(int r, int c) const;
    double ArmY(int r, int c) const;
    double PickUpRC(int r, int c) const;                // GetPickUp(i, j)
    double PlaceRC(int r, int c) const;                 // GetPlace(i, j)
};

// The TRAY_TYPE_PARA fields (cprod.h:1257-1299) the formulas read.
struct ArmCellForm {
    double xPitch = 0.0, yPitch = 0.0, xStart = 0.0, yStart = 0.0, zDepth = 0.0;
    int    xDivision = 0, yDivision = 0, trayType = 0, plateSelect = 0;
    double blockXStart = 0.0, blockYStart = 0.0, blockPitchX = 0.0, blockPitchY = 0.0;
    int    blockXItem = 1, blockYItem = 1;
    bool   useWideHotplate = false, useThickTray = false;
};

// One motor the plan names (the Mot_Table row behind MOT[mi]).
struct ArmCellAxis {
    int         mi = -1;         // MOT[] index (-1 = none)
    std::string alias;           // Mot_Table Alias ("" = no row)
    bool        present = false; // Mot_Table has a row for MOT[mi]
    bool        is1203 = false;  // CardModel PCI1203
    bool        enable = false;  // PCI1203: Mot_Table Enable; other cards: golden Motor->Enable (NB2 spec §4.1)
};

// One arm (0 = In, 1 = Out).
struct ArmCellArmInputs {
    int  xBase = 2, yBase = 0;           // iIn/OutArmXBase, iIn/OutArmYBase (cmydef; database.cpp sets them from USE_PICKER_COUNT / USE_*_Y_PITCH)
    int  shtXCenter = 0, shtYCenter = 0; // iIn/OutArmShtXCenterPos / ShtYCenterPos
    bool yAutoPitch = false;             // USE_IN_Y_IS_AUTO_PITCH / USE_OUT_Y_IS_AUTO_PITCH
    int  motRow = 0, motCol = 0;         // In/OutArmSuck.iMotRow / iMotCol (golden btnIn/OutZAllUpClick's grid)
    int  maxRow = 0, maxCol = 0;         // In/OutArmSuck.iMaxRow / iMaxCol (the grid SetTechDataToProd fills per nozzle)
    ArmCellAxis x, y;
    ArmCellAxis z[2][8];                 // In/OutArmZIndex[i][j] for i < motRow, j < motCol
    std::vector<ArmCellAxis> pitch;      // MIn/OutArmPitch, PitchY, PitchX2..X4 -- an Enable one = a pitch-axis machine
    int  zHeightSub[2][8] = {};          // golden iTemp: j < 4 ? Tech.iIn/OutArmZHeightSub[i][j] : ..._16[i][j-4]
};

// Everything the formulas read, copied on the tick thread (ArmCellLive.cpp) -- the plan never touches a global.
struct ArmCellInputs {
    // machine / Gerneral.ini
    int    usePickerCount = kAcEp1;      // USE_PICKER_COUNT (ePickCount)
    int    pickerUse = kAcPickMot;       // InOutArmPickerUseMotor (ePickType)
    bool   hotPlatePos0 = true;          // HOT_PLATE_POSITION == 0
    bool   ht1032 = false;               // MachineTypeChoice == Type_HT1032 (golden HotPlate mirror branch)
    bool   ht9050Type = false;           // the machine is an HT9050 (ArmCellIsHt9050 below): D2 of Out Shuttle 1 reads MOutShuttle1, as golden OutSHT1InRT's Type_HT9050 arm
    bool   use8Picker = false;           // golden Start() :4719-4728 (WebStart.cpp:1629): E43 || Type_HT9046_LS || Type_HT1032
    bool   newCleanModeKit = false;      // bUseNewCleanModeKit
    bool   cleanKit16BdBe = false;       // USE_PICKER_COUNT == ep16Picker && USE_IN_OUT_ARM_Y_PITCH == iXYPitch16Bd_Be
    bool   aoa = false;                  // MACHINE_HAS_AUTO_ALIGNMENT_CCD && (TestIF / TestIF_File).bEnableAutoAlignment
    bool   loaderRatioUngated = false;   // W906_TRANSFER_LOADER_RATIO != 0 (MachineType.h): the Loader copy of TransferLoaderRatio is not written
    int    zSafePos = 0;                 // global ZSafePos (Gerneral.ini [In Arm] ZSafePos; never hard-coded)
    // IniConfig / CosFunction
    bool   e33 = false, e34 = false, e43 = false, e70 = false, e88 = false, p06 = false, trayBlockMode = false, hotPlateMove1CM = false;
    bool   e30 = false, e30_1Hot = false, e30_2Cold = false, e31 = false, e31_1Hot = false, e31_2Cold = false;
    bool   e32 = false, e32_1Hot = false, e32_2Cold = false;
    bool   hotModeUseDiffScale = false, trayThickAdjustZ = false;
    bool   triTempHot = false;           // Tri_Temp_Machine==1 && LastSet.iTemperature==Tempture_Hot
    bool   lastSetHot = false;           // LastSet.iTemperature==Tempture_Hot
    double workTemperBase = 0.0;         // Temperature.fWorkTemperBase
    double hotPlateExpansion = 1.0;      // fHotPlateExpansionCoefficient
    // scales (LastSet / TestIF_File / IniConfig)
    double hpXScale[2] = {}, hpYScale[2] = {}, hpXScaleHot[2] = {}, hpYScaleHot[2] = {}, hpXScaleCold[2] = {}, hpYScaleCold[2] = {};
    double inShtXScale[2] = {}, inShtYScale[2] = {}, inShtXScaleHot[2] = {}, inShtYScaleHot[2] = {};
    double inShtXScaleCold[2] = {}, inShtYScaleCold[2] = {};
    bool   setupFileScale = false;       // TestIF_File.bInArmUseDifferentScaleBySetupFile
    double hpXScaleFile[2] = {}, hpYScaleFile[2] = {}, inShtXScaleFile[2] = {}, inShtYScaleFile[2] = {}, outShtXScaleFile[2] = {};
    double trayXScale[kAcTrayCount] = {}, trayYScale[kAcTrayCount] = {};
    double trayXScaleHot[kAcTrayCount] = {}, trayYScaleHot[kAcTrayCount] = {};
    double trayXScaleCold[kAcTrayCount] = {}, trayYScaleCold[kAcTrayCount] = {};
    // test IF
    double siteXPitch = 0.0, siteYPitch = 0.0;   // TestIF.dSiteXPitch / dSiteYPitch
    bool   ns7000Kit = false, twoArm32Site = false;  // TestIF.bNS7000kit, bUseTwoArm32Site
    int    shtRow = 0, shtCol = 0;       // TestSocket.iShtRow / iShtCol
    int    shuttleMode = 0, shuttleSel = 0;  // TestIF.iShuttleMode / iShuttle_Sel (1 / 0 = only shuttle 1)
    // auto clean
    int    acFunction = 0, acTray = 0;   // TestIF.iAutoClean_Function, TestIF_File.iAutoClean_Tray
    double acXStart = 0.0, acYStart = 0.0;   // TestIF.dAutoClean_XStart / YStart (the base point)
    double acXPitch = 0.0, acYPitch = 0.0;   // TestIF_File.dAutoClean_XPitch / YPitch (the cell)
    int    acXDivision = 0, acYDivision = 0; // TestIF.iAutoClean_XDivision / YDivision
    bool   acUseTray = false, acUseNSKit = false;   // TestIF_File.bAutoClean_UseTray / bAutoClean_UseNSKit
    int    armYPitch = 0;                // TestIF.iARM_Y_PITCH (GetYPitchOfCleanKit)
    double acHotplateXOffset = 0.0, acHotplateYOffset = 0.0, acHotplatePlaceOffset = 0.0;   // TestIF_File.Hotplatl* (InitialSet copies them into ints)
    int    acPadThickness = 0;           // TestIF_File.iPadThickness
    int    acTeachPickZ = 0;             // Teach.iAutoCleanPick
    // forms
    ArmCellForm loadForm, hotPlateForm, autoForm[kAcTrayCount];
    double userDefFileZDepth[4] = {};    // UserDefForm_File[k].ZDepth
    bool   userDefFile0Thick = false;    // UserDefForm_File[0].bUseThickTray
    int    loaderTrayType = 0;           // TrayForm.Loader.iTrayType
    int    autoTrayType[kAcTrayCount] = {};   // TrayForm.Auto[t].iTrayType
    // prod
    int    trayKitStartX = 0, trayKitStartY = 0;
    bool   plateSelect[2] = {};          // Prod.bPlateSelect: [0] = HotPlate 2, [1] = HotPlate 1 (golden)
    int    trayType[kAcTrayCount] = {};  // Prod.iTrayType
    bool   fixRightIsFix3 = false;       // iFixRight == eFix3 (3-tray layout: Fix4..6 are the halves of Fix1..3)
    int    fix3FullPlace = 0;            // FIX3_FULL_PLACE: 1 = Fix3K_UseCylinder46LA, 2 = Fix3K_UseStepperMotor, 0 = other
    int    shuttleTempPos[2][2] = {};    // iShuttleTempPos ([s][0] left, [s][1] right)
    // Tech (In)
    int    inLoadX = 0, inLoadY = 0, inLoadZ = 0, inPlate1X = 0, inPlate1Y = 0, inPlate2X = 0, inPlate2Y = 0, inPlateZ = 0;
    int    inShtX[2] = {}, inShtY[2] = {}, inShtZ = 0, inACX = 0, inACY = 0;
    // Tech (Out)
    int    outAutoX[6] = {}, outAutoY[6] = {}, outFixX[6] = {}, outFixY[6] = {};
    int    outPlaceZ2 = 0, outPlaceFixZ1 = 0, outPlaceFix2Z1 = 0;
    int    outShtX[2] = {}, outShtY[2] = {}, outShtPickZ2 = 0;
    // shuttle arm-side points (D2): golden SetTechDataToProd_Shuttle Prod.InSHT[s].iLeft / iRight, Prod.OutSHT[s].iRight
    int    techInShtLeft[2] = {}, techInShtRight[2] = {}, techOutShtRight[2] = {};
    double shLeftPod[2] = {}, shRightPod[2] = {};    // Offset.iSHLeftPod / iSHRightPod (double, sic)
    // offsets
    ArmCellOffset inOfsLoader, inOfsHP1, inOfsHP2, inOfsInSh[2], inOfsAutoClean;
    ArmCellOffset outOfsAuto[6], outOfsFix[6], outOfsOutSh[2];
    // Prod's current base values (echo only: "Prod 現值" next to the Tech recomputation)
    int    prodInLoadX = 0, prodInLoadY = 0, prodInPlateX[2] = {}, prodInPlateY[2] = {};   // plate index golden: [0] HP2
    int    prodInShtX[2] = {}, prodInShtY[2] = {}, prodInACX = 0, prodInACY = 0;
    int    prodOutStartX[kAcTrayCount] = {}, prodOutStartY[kAcTrayCount] = {}, prodOutShtX[2] = {}, prodOutShtY[2] = {};
    // axes
    ArmCellAxis inShuttle[2], outShuttle[2];  // MInShuttle1/2, MOutShuttle1/2
    ArmCellArmInputs arm[2];                  // 0 In, 1 Out
};

struct ArmCellRequest {
    std::string arm;             // "in" | "out"
    std::string nozzle;          // the Z alias ("" = the arm's only Enable Z)
    std::string area;            // ArmCellArea::id
    int         col = -1, row = -1;   // 0-based (the page shows 1-based)
    bool        zDown = false;
};

struct ArmCellArea {
    std::string arm, id, label;
    bool        v1 = false;          // in the first version's scope (the areas HT9050 has)
    bool        installed = false;   // this machine has it (data, not machine-type names)
    bool        usable = false;      // installed && v1 && the arm passes ArmCheck
    std::string why;                 // !usable: why (Traditional Chinese, shown on the page)
    int         cols = 0, rows = 0;
    bool        zDownAllowed = false;
    std::string zDownWhy;
    std::string zKind;               // "pick" | "place" | ""
};

struct ArmCellNozzleInfo { int i = 0, j = 0; ArmCellAxis z; };

struct ArmCellCatalog {
    std::vector<ArmCellNozzleInfo> nozzles[2];   // Enable Z only (Q4 = A)
    std::string                    note[2];      // D1 note
    std::string                    armWhy[2];    // "" = the arm passes ArmCheck
    ArmCellAxis                    x[2], y[2];
    std::vector<ArmCellArea>       areas;
    int                            zSafePos = 0;
};

struct ArmCellPlan {
    std::string  arm, area, label, zKind, nozzle;
    int          armIndex = -1, col = -1, row = -1, nr = -1, nc = -1;   // nr / nc = the nozzle's grid index
    ArmCellAxis  x, y, z;
    std::vector<ArmCellAxis> zLift;              // every Enable Z of the arm (S1)
    int          xTarget = 0, yTarget = 0, zTarget = 0, zSafe = 0;
    bool         zDownAllowed = false;
    std::string  zDownWhy;
    int          teachX = 0, teachY = 0, teachZ = 0;   // the raw Tech teach point (X, Y, Z on the ZE slot)
    int          baseX = 0, baseY = 0;           // the base slot recomputed from Tech
    bool         hasProd = false;
    int          prodBaseX = 0, prodBaseY = 0;   // Prod's current base slot
    int          cellX = 0, cellY = 0;           // the cell before the compensation (ratio)
    bool         d2 = false;                     // a shuttle area: the job checks the shuttle at its arm side before Z down
    ArmCellAxis  d2Axis;
    int          d2Target = 0;
    std::string  d2What;
    std::string  d1Note;
    std::vector<std::string> notes;
};

// golden GetShtStartPos (ainarm9045.cpp:615-627): centre - (int)(((itemCount-1)/2.0) * gap)
int   ArmCellShtStartPos(int itemCount, int centre, int gap);
// golden ChangeToFloatNonPcnt (MachineType.h:1729): a float quotient, 0 for a 0 denominator
float ArmCellFloatQuotient(double n, double d);
// AI(W906-ARMCELL) 20261002 [W906]: review M1 -- "is this machine an HT9050", the tree's existing run-time test
// (WebBridgeTags.cpp W906_MainCaption `W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050`; the same
// W906_GpibModel test is behind the HT9050 ORG-low hook, WebMotorAccessLive.cpp OrgActiveLowHT9050). An HT9050 decodes as
// Type_HT9046_LS (database.cpp:517, RULINGS_20260926 #25), so MachineTypeChoice == Type_HT9050 alone is never true on it.
// gpibModel = W906_GpibModel ([Version] Model of D:\GPIB9045\system\general.ini), typeIsHt9050 = MachineTypeChoice == Type_HT9050.
bool  ArmCellIsHt9050(const std::string& gpibModel, bool typeIsHt9050);

// The catalog (nozzles, every golden area with installed / why / cols / rows / zDownAllowed).
void ArmCellBuildCatalog(const ArmCellInputs& in, ArmCellCatalog& out);
// The plan of one request. false + why = refused (nothing to move).
bool ArmCellBuildPlan(const ArmCellInputs& in, const ArmCellRequest& q, ArmCellPlan& out, std::string& why);

// Implemented by ArmCellLive.cpp (wb_serve only; reads the god-stack on the tick thread).
bool ArmCellLiveInputs(ArmCellInputs& in, std::string& why);
bool ArmCellLiveCatalog(ArmCellCatalog& out, std::string& why);
bool ArmCellLivePlan(const ArmCellRequest& q, ArmCellPlan& out, std::string& why);

}  // namespace ht9045

#endif  // HT9045_ARMCELLPLAN_H
