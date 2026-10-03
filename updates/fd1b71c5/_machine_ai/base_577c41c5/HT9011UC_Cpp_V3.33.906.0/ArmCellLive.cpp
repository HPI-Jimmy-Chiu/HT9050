// =============================================================================
//  ArmCellLive.cpp  --  the Teach page "Arm Cell" tab: ArmCellInputs filled from the god-stack (wb_serve only)
//
//  AI(W906-ARMCELL) 20261002 [W906]: RULINGS_20261002 #18. Contract and formulas: ArmCellPlan.h / ArmCellPlan.cpp.
//  Runs on the tick thread only: WebMotorAccess.cpp calls it through the live backend (IMotorAccessBackend::
//  GoldenArmCellCatalog from WebMotorAccessLive.cpp OverlaySnapshot, GoldenArmCellPlan from the motor.access dispatch) --
//  the same thread that owns Prod / Tech / MOT[]. Read only: it copies values, it calls no production mover, writes no
//  global, never calls SetTechDataToProd (it would rewrite all of Prod mid-session; NB2 spec §4).
//  InArmSuck / OutArmSuck / TestSocket through aHotPlateSubstrate.h -- the header forms/fTeach.cpp, WebStart.cpp and
//  W906_TeachFixedMotorsLive read them through (the live TMyKitSuck; NEVER mykitsuck.h -- KNOWLEDGE "two TMyKitSuck").
// =============================================================================
#include "ArmCellPlan.h"
#include "WebMotorAccess.h"

#include <string>

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "LastSet.h"
#include "Config.h"
#include "CosFunction.h"
#include "aHotPlateSubstrate.h"
#include "Motor/mymotor.h"
#include "database.h"         // W906_GpibModel (review M1: the run-time HT9050 test)

extern int iShuttleTempPos[2][2];   // cinitial.cpp:4845 (golden cinitial.cpp:75; no header declares it -- file scope on purpose)

// the golden enums ArmCellPlan.h mirrors -- a renumbering there must break the build, not move the arm (int on both sides:
// two different enum types, -Wenum-compare)
static_assert((int)eAuto1 == (int)ht9045::kAcAuto1 && (int)eAuto6 == (int)ht9045::kAcAuto6 && (int)eFix1 == (int)ht9045::kAcFix1 &&
              (int)eFix2 == (int)ht9045::kAcFix2 && (int)eFix3 == (int)ht9045::kAcFix3 && (int)eFix6 == (int)ht9045::kAcFix6 &&
              (int)eBulkBox == (int)ht9045::kAcBulkBox && (int)eMag1 == (int)ht9045::kAcMag1 && (int)eMag14 == (int)ht9045::kAcMag14 &&
              (int)eTrayCount == (int)ht9045::kAcTrayCount, "e6TrayName changed (MachineType.h)");
static_assert((int)eCKPos_HP2 == (int)ht9045::kAcCKPosHP2 && (int)eCKPos_CleanKit == (int)ht9045::kAcCKPosCleanKit, "eCKPos changed (MachineType.h)");
static_assert((int)tNotUse == (int)ht9045::kAcTrayNotUse && (int)tTrayAuto == (int)ht9045::kAcTrayAuto && (int)tTrayFix == (int)ht9045::kAcTrayFix &&
              (int)tTrayMag == (int)ht9045::kAcTrayMag && (int)tTrayBox == (int)ht9045::kAcTrayBox, "tray types changed (MachineType.h)");
static_assert((int)ep4Picker == (int)ht9045::kAcEp4 && (int)ep8Picker == (int)ht9045::kAcEp8 && (int)ep2Picker == (int)ht9045::kAcEp2 &&
              (int)ep16Picker == (int)ht9045::kAcEp16 && (int)ep1Picker == (int)ht9045::kAcEp1, "ePickCount changed (MachineType.h)");
static_assert((int)eptUseCyn == (int)ht9045::kAcPickCyn && (int)eptUseMot == (int)ht9045::kAcPickMot && (int)eptUseMotCyn == (int)ht9045::kAcPickMotCyn,
              "ePickType changed (MachineType.h)");
static_assert((int)KitPitchX == (int)ht9045::kAcKitPitch && (int)KitPitchY == (int)ht9045::kAcKitPitch, "KitPitchX / KitPitchY changed (cmydef.h)");

namespace ht9045 {
namespace {

ArmCellAxis AxisOf(IMotorAccessBackend& be, int mi, bool tableEnableAlways)
{
    ArmCellAxis x;
    x.mi = mi;
    if (mi < 0) return x;
    x.alias = be.AliasOfMotor(mi);
    if (x.alias.empty()) return x;
    MotorAccessAxis a;
    if (!be.Resolve(x.alias, a)) return x;
    x.present = true;
    x.is1203 = a.Is1203();
    if (x.is1203 || tableEnableAlways) {
        x.enable = a.tableEnable;                                               // Mot_Table Enable (the 1203 paths' rule, WebMotorAccess.h)
    } else {
        MotorGolden g;
        x.enable = a.motIndex >= 0 && be.GoldenMotor(a.motIndex, g) && g.enable;   // golden Motor->Enable (NB2 spec §4.1)
    }
    return x;
}

void CopyOffset(const ARM_OFFSET* s, ArmCellOffset& d)
{
    d = ArmCellOffset();
    if (s == 0) return;
    d.oneByOne = s->bOneByOne;
    d.x = s->dArmX; d.y = s->dArmY; d.pickUp = s->dPickUp; d.place = s->dPlaceUp;
    if (s->SingleOffSet == 0) return;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 8; ++j) {
            d.posX[i][j]     = s->SingleOffSet->dPosOffSetX[i][j];
            d.posY[i][j]     = s->SingleOffSet->dPosOffSetY[i][j];
            d.pickUpRC[i][j] = s->SingleOffSet->dPickUpOffSet[i][j];
            d.placeRC[i][j]  = s->SingleOffSet->dPlaceOffSet[i][j];
        }
}

void CopyForm(const TRAY_TYPE_PARA* s, ArmCellForm& d)
{
    d = ArmCellForm();
    if (s == 0) return;
    d.xPitch = s->XPitch; d.yPitch = s->YPitch; d.xStart = s->XStart; d.yStart = s->YStart; d.zDepth = s->ZDepth;
    d.xDivision = s->XDivision; d.yDivision = s->YDivision; d.trayType = s->iTrayType; d.plateSelect = s->iPlateSelect;
    d.blockXStart = s->BlockXStart; d.blockYStart = s->BlockYStart; d.blockPitchX = s->BlockPitchX; d.blockPitchY = s->BlockPitchY;
    d.blockXItem = s->BlockXItem; d.blockYItem = s->BlockYItem;
    d.useWideHotplate = s->bUseWideHotplate; d.useThickTray = s->bUseThickTray;
}

void FillArm(IMotorAccessBackend& be, bool in, ArmCellArmInputs& a)
{
    const TMyKitSuck& k = in ? InArmSuck : OutArmSuck;
    a.xBase = in ? iInArmXBase : iOutArmXBase;
    a.yBase = in ? iInArmYBase : iOutArmYBase;
    a.shtXCenter = in ? iInArmShtXCenterPos : iOutArmShtXCenterPos;
    a.shtYCenter = in ? iInArmShtYCenterPos : iOutArmShtYCenterPos;
    a.yAutoPitch = in ? (USE_IN_Y_IS_AUTO_PITCH == true) : (USE_OUT_Y_IS_AUTO_PITCH == true);
    a.motRow = k.iMotRow; a.motCol = k.iMotCol;
    a.maxRow = k.iMaxRow; a.maxCol = k.iMaxCol;
    a.x = AxisOf(be, in ? MInArmX : MOutArmX, false);
    a.y = AxisOf(be, in ? MInArmY : MOutArmY, false);
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j)
            a.z[i][j] = (i < a.motRow && j < a.motCol) ? AxisOf(be, in ? InArmZIndex[i][j] : OutArmZIndex[i][j], false) : ArmCellAxis();
    const int pitchIn[5]  = { MInArmPitch, MInArmPitchY, MInArmPitchX2, MInArmPitchX3, MInArmPitchX4 };
    const int pitchOut[5] = { MOutArmPitch, MOutArmPitchY, MOutArmPitchX2, MOutArmPitchX3, MOutArmPitchX4 };
    a.pitch.clear();
    for (int i = 0; i < 5; ++i) a.pitch.push_back(AxisOf(be, in ? pitchIn[i] : pitchOut[i], true));   // a pitch axis = the hardware (Mot_Table Enable)
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 8; ++j)
            a.zHeightSub[i][j] = (j < 4) ? (in ? Tech.iInArmZHeightSub[i][j] : Tech.iOutArmZHeightSub[i][j])
                                         : (in ? Tech.iInArmZHeightSub_16[i][j - 4] : Tech.iOutArmZHeightSub_16[i][j - 4]);
}

}  // namespace

bool ArmCellLiveInputs(ArmCellInputs& in, std::string& why)
{
    in = ArmCellInputs();
    why.clear();
    IMotorAccessBackend& be = MotorAccessLiveBackend();
    if (LoadForm == 0) { why = "Arm Cell：LoadForm 還沒建立（工作檔沒有載入）"; return false; }
    // machine
    in.usePickerCount  = USE_PICKER_COUNT;
    in.pickerUse       = InOutArmPickerUseMotor;
    in.hotPlatePos0    = (HOT_PLATE_POSITION == 0);
    in.ht1032          = (MachineTypeChoice == Type_HT1032);
    in.ht9050Type      = ArmCellIsHt9050(W906_GpibModel.c_str(), MachineTypeChoice == Type_HT9050);   //AI(W906-ARMCELL) 20261002: review M1 -- 9050GPIB decodes as Type_HT9046_LS; the tree's run-time HT9050 test (WebBridgeTags.cpp W906_MainCaption)
    in.use8Picker      = IniConfig.bE43AutoCleanUseHotplate || MachineTypeChoice == Type_HT9046_LS || MachineTypeChoice == Type_HT1032;   // golden Start :4719-4728 (WebStart.cpp:1629-1639)
    in.newCleanModeKit = bUseNewCleanModeKit;
    in.cleanKit16BdBe  = (USE_PICKER_COUNT == ep16Picker && USE_IN_OUT_ARM_Y_PITCH == iXYPitch16Bd_Be);
    in.aoa             = MACHINE_HAS_AUTO_ALIGNMENT_CCD && (TestIF.bEnableAutoAlignment || TestIF_File.bEnableAutoAlignment);
    in.loaderRatioUngated = (W906_TRANSFER_LOADER_RATIO != 0);
    in.zSafePos        = ZSafePos;
    // IniConfig / CosFunction
    in.e33 = IniConfig.bE33InOutArmZOffsetSameOne;   in.e34 = IniConfig.bE34InOutArmPitchZOffsetSameOne;
    in.e43 = IniConfig.bE43AutoCleanUseHotplate;     in.e70 = IniConfig.bE70_UseTrayThickAdjustZHeight;
    in.e88 = IniConfig.bE88_InArmHeightFollow7000;   in.p06 = IniConfig.bP06_LoaderUseCarrierTray;
    in.trayBlockMode = IniConfig.bUseTrayBlockMode;  in.hotPlateMove1CM = IniConfig.bHotPlateMove1CM;
    in.e30 = IniConfig.bE30InArmUseDifferentScale;   in.e30_1Hot = IniConfig.bE30_1InArmUseDifferentScale_Hot;  in.e30_2Cold = IniConfig.bE30_2InArmUseDifferentScale_Cold;
    in.e31 = IniConfig.bE31OutArmUseDifferentScale;  in.e31_1Hot = IniConfig.bE31_1OutArmUseDifferentScale_Hot; in.e31_2Cold = IniConfig.bE31_2OutArmUseDifferentScale_Cold;
    in.e32 = IniConfig.bE32ShuttleUseDifferentScale; in.e32_1Hot = IniConfig.bE32_1ShuttleUseDifferentScale_Hot; in.e32_2Cold = IniConfig.bE32_2ShuttleUseDifferentScale_Cold;
    in.hotModeUseDiffScale = CosFunction.bHotModeUseDiffScale;
    in.trayThickAdjustZ    = CosFunction.bUseTrayThickAdjustZHeight;
    in.triTempHot          = (Tri_Temp_Machine == 1 && LastSet.iTemperature == Tempture_Hot);
    in.lastSetHot          = (LastSet.iTemperature == Tempture_Hot);
    in.workTemperBase      = Temperature.fWorkTemperBase;
    in.hotPlateExpansion   = fHotPlateExpansionCoefficient;
    for (int i = 0; i < 2; ++i) {
        in.hpXScale[i] = LastSet.fHotPlateXScale[i];         in.hpYScale[i] = LastSet.fHotPlateYScale[i];
        in.hpXScaleHot[i] = LastSet.fHotPlateXScale_Hot[i];  in.hpYScaleHot[i] = LastSet.fHotPlateYScale_Hot[i];
        in.hpXScaleCold[i] = LastSet.fHotPlateXScale_Cold[i]; in.hpYScaleCold[i] = LastSet.fHotPlateYScale_Cold[i];
        in.inShtXScale[i] = LastSet.fInShuttleXScale[i];     in.inShtYScale[i] = LastSet.fInShuttleYScale[i];
        in.inShtXScaleHot[i] = LastSet.fInShuttleXScale_Hot[i];   in.inShtYScaleHot[i] = LastSet.fInShuttleYScale_Hot[i];
        in.inShtXScaleCold[i] = LastSet.fInShuttleXScale_Cold[i]; in.inShtYScaleCold[i] = LastSet.fInShuttleYScale_Cold[i];
        in.hpXScaleFile[i] = TestIF_File.fHotPlateXScaleBySetupFile[i];   in.hpYScaleFile[i] = TestIF_File.fHotPlateYScaleBySetupFile[i];
        in.inShtXScaleFile[i] = TestIF_File.fInShuttleXScaleBySetupFile[i]; in.inShtYScaleFile[i] = TestIF_File.fInShuttleYScaleBySetupFile[i];
        in.outShtXScaleFile[i] = TestIF_File.fOutShuttleXScaleBySetupFile[i];
    }
    in.setupFileScale = TestIF_File.bInArmUseDifferentScaleBySetupFile;
    for (int t = 0; t < kAcTrayCount; ++t) {
        in.trayXScale[t] = IniConfig.dTrayXScale[t];          in.trayYScale[t] = IniConfig.dTrayYScale[t];
        in.trayXScaleHot[t] = IniConfig.dTrayXScale_Hot[t];   in.trayYScaleHot[t] = IniConfig.dTrayYScale_Hot[t];
        in.trayXScaleCold[t] = IniConfig.dTrayXScale_Cold[t]; in.trayYScaleCold[t] = IniConfig.dTrayYScale_Cold[t];
    }
    // test IF
    in.siteXPitch = TestIF.dSiteXPitch;  in.siteYPitch = TestIF.dSiteYPitch;
    in.ns7000Kit = TestIF.bNS7000kit;    in.twoArm32Site = bUseTwoArm32Site;
    in.shtRow = TestSocket.iShtRow;      in.shtCol = TestSocket.iShtCol;
    in.shuttleMode = TestIF.iShuttleMode; in.shuttleSel = TestIF.iShuttle_Sel;
    // auto clean
    in.acFunction = TestIF.iAutoClean_Function;  in.acTray = TestIF_File.iAutoClean_Tray;
    in.acXStart = TestIF.dAutoClean_XStart;      in.acYStart = TestIF.dAutoClean_YStart;
    in.acXPitch = TestIF_File.dAutoClean_XPitch; in.acYPitch = TestIF_File.dAutoClean_YPitch;
    in.acXDivision = TestIF.iAutoClean_XDivision; in.acYDivision = TestIF.iAutoClean_YDivision;
    in.acUseTray = TestIF_File.bAutoClean_UseTray; in.acUseNSKit = TestIF_File.bAutoClean_UseNSKit;
    in.armYPitch = TestIF.iARM_Y_PITCH;
    in.acHotplateXOffset = TestIF_File.HotplatlXOffset; in.acHotplateYOffset = TestIF_File.HotplatlYOffset;
    in.acHotplatePlaceOffset = TestIF_File.HotplatlPlaceOffset;
    in.acPadThickness = TestIF_File.iPadThickness;
    in.acTeachPickZ = Teach.iAutoCleanPick;
    // forms
    CopyForm(LoadForm, in.loadForm);
    CopyForm(&HotPlateForm, in.hotPlateForm);
    for (int t = 0; t < kAcTrayCount; ++t) { CopyForm(AutoForm[t], in.autoForm[t]); in.autoTrayType[t] = TrayForm.Auto[t].iTrayType; }
    for (int k = 0; k < 4; ++k) in.userDefFileZDepth[k] = UserDefForm_File[k].ZDepth;
    in.userDefFile0Thick = UserDefForm_File[0].bUseThickTray;
    in.loaderTrayType = TrayForm.Loader.iTrayType;
    // prod
    in.trayKitStartX = Prod.iTrayKitStartX;  in.trayKitStartY = Prod.iTrayKitStartY;
    in.plateSelect[0] = Prod.bPlateSelect[0]; in.plateSelect[1] = Prod.bPlateSelect[1];
    for (int t = 0; t < kAcTrayCount; ++t) in.trayType[t] = Prod.iTrayType[t];
    in.fixRightIsFix3 = (iFixRight == eFix3);
    in.fix3FullPlace = (FIX3_FULL_PLACE == Fix3K_UseCylinder46LA) ? 1 : (FIX3_FULL_PLACE == Fix3K_UseStepperMotor) ? 2 : 0;
    for (int s = 0; s < 2; ++s) for (int k = 0; k < 2; ++k) in.shuttleTempPos[s][k] = iShuttleTempPos[s][k];
    // Tech (In)
    in.inLoadX = Tech.iInArmLoadStageX;  in.inLoadY = Tech.iInArmLoadStageY;  in.inLoadZ = Tech.iInArmLoadStagePickZ2;
    in.inPlate1X = Tech.iInArmPlate1X;   in.inPlate1Y = Tech.iInArmPlate1Y;
    in.inPlate2X = Tech.iInArmPlate2X;   in.inPlate2Y = Tech.iInArmPlate2Y;   in.inPlateZ = Tech.iInArmPlatePickZ2;
    in.inShtX[0] = Tech.iInArmShuttle1X; in.inShtY[0] = Tech.iInArmShuttle1Y;
    in.inShtX[1] = Tech.iInArmShuttle2X; in.inShtY[1] = Tech.iInArmShuttle2Y; in.inShtZ = Tech.iInArmShuttlePlaceZ;
    in.inACX = Tech.iInArmAutoCleanX;    in.inACY = Tech.iInArmAutoCleanY;
    // Tech (Out)
    const int ax[6] = { Tech.iOutArmAuto1X, Tech.iOutArmAuto2X, Tech.iOutArmAuto3X, Tech.iOutArmAuto4X, Tech.iOutArmAuto5X, Tech.iOutArmAuto6X };
    const int ay[6] = { Tech.iOutArmAuto1Y, Tech.iOutArmAuto2Y, Tech.iOutArmAuto3Y, Tech.iOutArmAuto4Y, Tech.iOutArmAuto5Y, Tech.iOutArmAuto6Y };
    const int fx[6] = { Tech.iOutArmFix1X, Tech.iOutArmFix2X, Tech.iOutArmFix3X, Tech.iOutArmFix4X, Tech.iOutArmFix5X, Tech.iOutArmFix6X };
    const int fy[6] = { Tech.iOutArmFix1Y, Tech.iOutArmFix2Y, Tech.iOutArmFix3Y, Tech.iOutArmFix4Y, Tech.iOutArmFix5Y, Tech.iOutArmFix6Y };
    for (int k = 0; k < 6; ++k) { in.outAutoX[k] = ax[k]; in.outAutoY[k] = ay[k]; in.outFixX[k] = fx[k]; in.outFixY[k] = fy[k]; }
    in.outPlaceZ2 = Tech.iOutArmPlaceZ2;  in.outPlaceFixZ1 = Tech.iOutArmPlaceFixZ1;  in.outPlaceFix2Z1 = Tech.iOutArmPlaceFix2Z1;
    in.outShtX[0] = Tech.iOutArmShuttle1X; in.outShtY[0] = Tech.iOutArmShuttle1Y;
    in.outShtX[1] = Tech.iOutArmShuttle2X; in.outShtY[1] = Tech.iOutArmShuttle2Y; in.outShtPickZ2 = Tech.iOutArmShuttlePickZ2;
    // shuttle arm-side points (D2)
    in.techInShtLeft[0] = Tech.iInShuttle1Left;   in.techInShtLeft[1] = Tech.iInShuttle2Left;
    in.techInShtRight[0] = Tech.iInShuttle1Right; in.techInShtRight[1] = Tech.iInShuttle2Right;
    in.techOutShtRight[0] = Tech.iOutShuttle1Right; in.techOutShtRight[1] = Tech.iOutShuttle2Right;
    for (int s = 0; s < 2; ++s) { in.shLeftPod[s] = Offset.iSHLeftPod[s]; in.shRightPod[s] = Offset.iSHRightPod[s]; }
    // offsets
    CopyOffset(InArmOffSet[InOfsLoader], in.inOfsLoader);
    CopyOffset(InArmOffSet[InOfsHP1], in.inOfsHP1);
    CopyOffset(InArmOffSet[InOfsHP2], in.inOfsHP2);
    CopyOffset(InArmOffSet[InOfsInSh1], in.inOfsInSh[0]);
    CopyOffset(InArmOffSet[InOfsInSh2], in.inOfsInSh[1]);
    CopyOffset(InArmOffSet[InOfsAutoClean], in.inOfsAutoClean);
    for (int k = 0; k < 6; ++k) { CopyOffset(OutArmOffSet[OutOfsAuto1 + k], in.outOfsAuto[k]); CopyOffset(OutArmOffSet[OutOfsFix1 + k], in.outOfsFix[k]); }
    CopyOffset(OutArmOffSet[OutOfsOutSh1], in.outOfsOutSh[0]);
    CopyOffset(OutArmOffSet[OutOfsOutSh2], in.outOfsOutSh[1]);
    // Prod's current base slots (echo)
    const int ib = iInArmYBase, ic = iInArmXBase, ob = iOutArmYBase, oc = iOutArmXBase;
    if (ib >= 0 && ib < MAX_ARM_Row && ic >= 0 && ic < MAX_ARM_Col) {
        in.prodInLoadX = Prod.XInArm_Tray_Pick[ib][ic];       in.prodInLoadY = Prod.YInArm_Tray_Pick[ib][ic];
        in.prodInPlateX[0] = Prod.XInArm_Plate2_Pick[ib][ic]; in.prodInPlateY[0] = Prod.YInArm_Plate2_Pick[ib][ic];   // [0] = HotPlate 2 (golden)
        in.prodInPlateX[1] = Prod.XInArm_Plate1_Pick[ib][ic]; in.prodInPlateY[1] = Prod.YInArm_Plate1_Pick[ib][ic];
        in.prodInShtX[0] = Prod.XInArm_Shuttle1_Place[ib][ic]; in.prodInShtY[0] = Prod.YInArm_Shuttle1_Place[ib][ic];
        in.prodInShtX[1] = Prod.XInArm_Shuttle2_Place[ib][ic]; in.prodInShtY[1] = Prod.YInArm_Shuttle2_Place[ib][ic];
        in.prodInACX = Prod.XInArm_AutoClean_Pick[ib][ic];    in.prodInACY = Prod.YInArm_AutoClean_Pick[ib][ic];
    }
    if (ob >= 0 && ob < MAX_ARM_Row && oc >= 0 && oc < MAX_ARM_Col) {
        for (int t = 0; t < kAcTrayCount; ++t) { in.prodOutStartX[t] = Prod.XStart[t][ob][oc]; in.prodOutStartY[t] = Prod.YStart[t][ob][oc]; }
        in.prodOutShtX[0] = Prod.XOutArm_Shuttle1_Pick[ob][oc]; in.prodOutShtY[0] = Prod.YOutArm_Shuttle1_Pick[ob][oc];
        in.prodOutShtX[1] = Prod.XOutArm_Shuttle2_Pick[ob][oc]; in.prodOutShtY[1] = Prod.YOutArm_Shuttle2_Pick[ob][oc];
    }
    // axes
    in.inShuttle[0] = AxisOf(be, MInShuttle1, false);   in.inShuttle[1] = AxisOf(be, MInShuttle2, false);
    in.outShuttle[0] = AxisOf(be, MOutShuttle1, false); in.outShuttle[1] = AxisOf(be, MOutShuttle2, false);
    if (InArmSuck.iMotRow < 0 || InArmSuck.iMotRow > 2 || InArmSuck.iMotCol < 0 || InArmSuck.iMotCol > 8 ||
        OutArmSuck.iMotRow < 0 || OutArmSuck.iMotRow > 2 || OutArmSuck.iMotCol < 0 || OutArmSuck.iMotCol > 8) {
        why = "Arm Cell：InArmSuck／OutArmSuck 的 iMotRow／iMotCol 超出 golden In／OutArmZIndex[2][8] —— 不讀表外的值";
        return false;
    }
    FillArm(be, true, in.arm[0]);
    FillArm(be, false, in.arm[1]);
    return true;
}

bool ArmCellLiveCatalog(ArmCellCatalog& out, std::string& why)
{
    ArmCellInputs in;
    if (!ArmCellLiveInputs(in, why)) { out = ArmCellCatalog(); return false; }
    ArmCellBuildCatalog(in, out);
    return true;
}

bool ArmCellLivePlan(const ArmCellRequest& q, ArmCellPlan& out, std::string& why)
{
    ArmCellInputs in;
    if (!ArmCellLiveInputs(in, why)) { out = ArmCellPlan(); return false; }
    return ArmCellBuildPlan(in, q, out, why);
}

}  // namespace ht9045
