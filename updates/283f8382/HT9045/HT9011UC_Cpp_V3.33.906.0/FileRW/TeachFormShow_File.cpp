// ===========================================================================
//  FileRW/TeachFormShow_File.cpp -- S12 第二型 bridge（只有顯示）：golden TfTeach::FormShow 的畫面部分。
//
//  AI(W906-TEACH-FORMSHOW) 20261002.  NOT generated (tools/gen_formbridge.py does not handle FormShow) -- hand-written by the
//  same rule:  widget->Prop = 右式;  ->  J.SetProp("widget", 右式);   control flow and the right-hand sides copied verbatim.
//  EastSun 20261001「請檢查每個頁面元件」「不是只有檢查按鈕喔 我說的是所有元件」: the every-component check found that the
//  web Teach page applied none of golden FormShow's ~150 Visible / TabVisible / Caption / Color / Enabled / ActivePage
//  assignments -- every panel showed as drawn in uteach.dfm whatever this machine's options are (OCR, Preciser, laser,
//  Rotate Kit, Auto Clean, 16 pickers, Bin Box, X/Y pitch...).
//
//  Source: golden D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\uteach.cpp:1413-2012 (V906, Big5, read only).
//  Served as GET /api/form/HW.teach.html (FormJson.cpp BridgePageJson); page/ht9045_teach_formshow_c.js applies it on load
//  and on every window open (golden FormShow runs at every Show).
//
//  NOT copied (each on purpose, so a GET never changes the machine):
//    :1415-1422 SetFocus / fShow / Left / Top / ReadFile / InitAutoAlignmentTask   (window + file side effects; the C path
//                                                                                  FileRW/Teach.cpp loads the values)
//    :1472-1597 TechPara / TechTwoPara / TechSuckPara Tag, GroupIndex and SetEdit->Text, MOT[].fCanMove*, ActiveFlag
//                                                                                  (data: the C path; machine state)
//    :1598 fiosetview->SetCompomentIO, :1605 SetMotorSpeed, :1606 fAllMotorHome=false, :1946 TriTemp_Teach (motor speed),
//    :1818 FrmAOI->fAOI_ReadFile, :2011 myLog                                      (side effects)
//    :1753 edShtCheckRange->Text, :1868 SetEditAutoClean->Text, :1961 InSHZDownRange->Text   (the C path loads them)
//    :1823-1826 gbTrayZ / grpTrayZAxis, :1844-1855 pnl*Z / grpAuto456              (page/ht9045_teach_trayz_c.js does them)
//    :1680-1681 / :1686-1687 MotorOutRE / MotorOutRG ->Flat, :1877 lblFindPhaseNotes->Width,
//    :1952-1953 grpSortSHTAxis->Top / ->Width                                      (geometry / bevel only)
// ===========================================================================
#include "JsonBridge/FormBridge.h"
#include "cmydef.h"
#include "Config.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "forms/fTeach.h"
#include "forms/fMain.h"

namespace ht9045 {
namespace formbridge {

namespace f_TfTeach {

static const long kclYellow = 0x0000FFFF, kclWhite = 0x00FFFFFF;   // VCL clYellow / clWhite (vclcompat BtnPanelCore.h:42-43)

// golden uteach.cpp:1413  TfTeach::FormShow(TObject *Sender) -- the screen half
static void B_FormShow(FormState& J)
{
    J.SetTabVisible("tsAOI", (USE_AOI_Inspection ||                                      // :1423
                       USE_Scanner_AOI_Inspection>(int)eBtnAOI_Uninstall ||
                       USE_Fix_AI_CCD ||
                       USE_Top_Scanner_AOI_Inspection==true));
    J.SetVisible("SetButton070", false);                                                 // :1427
    J.SetVisible("GoButton070", false);
    J.SetVisible("SetButton071", false);
    J.SetVisible("GoButton071", false);
    J.SetVisible("SetButton064", true);
    J.SetVisible("GoButton064", true);
    J.SetVisible("SetButton065", true);
    J.SetVisible("GoButton065", true);
    J.SetColor("pnlStop", 0x00DFD9CC);                                                   // :1435

    J.SetVisible("pnlOcr_Go", (INSTALL_OCR!=eocrUninstal));                               // :1437
    J.SetVisible("pnlOcr_XY", (INSTALL_OCR!=eocrUninstal));

    J.SetVisible("pnlBottom2D_Go", (BOTTOM_2DID!=0));                                     // :1440
    J.SetVisible("pnlBottom2D_XY", (BOTTOM_2DID!=0));

    J.SetVisible("pnlPrecisor_Go", (USE_PRECISER==1));                                    // :1443
    J.SetVisible("pnlPreciser_XY", (USE_PRECISER==1));
    J.SetVisible("pnlPrecisor_Place", (USE_PRECISER==1));
    J.SetVisible("grpPrecisor_Open", (USE_PRECISER==1));

    J.SetVisible("pnlNGBin_Go", (CUSTOMER_CODE==CC_ASE_KaohSiung_K3 && TRAY_MAPPING_GRAB==2));   // :1448
    J.SetVisible("pnlNGBin_XY", (CUSTOMER_CODE==CC_ASE_KaohSiung_K3 && TRAY_MAPPING_GRAB==2));
    J.SetVisible("pnlNGBin_Place", (CUSTOMER_CODE==CC_ASE_KaohSiung_K3 && TRAY_MAPPING_GRAB==2));

    J.SetVisible("pnlAutoClean_Go", (IniConfig.bEnableAutoCleanFunction));                // :1452
    J.SetVisible("pnlAutoClean_XY", (IniConfig.bEnableAutoCleanFunction));
    J.SetVisible("pnlAutoClean_Pick", (IniConfig.bEnableAutoCleanFunction));

    J.SetActivePage("pgcShuttle", "tsShuttlePos");                                       // :1456
    J.SetVisible("pnlInArmZ", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("pnlOutArmZ", (USE_PICKER_COUNT==ep16Picker));

    J.SetVisible("pnlInPickerCCD1", (USE_PICKER_COUNT==ep16Picker));                      // :1460
    J.SetVisible("pnlInPickerCCD2", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("pnlInPickerCCD3", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("pnlOutPickerCCD1", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("pnlOutPickerCCD2", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("pnlOutPickerCCD3", (USE_PICKER_COUNT==ep16Picker));

    J.SetVisible("pnlInDecay_Go", (CosFunction.bESDAutoDecayTeachFunction==true));       // :1467
    J.SetVisible("pnlInDecay_XY", (CosFunction.bESDAutoDecayTeachFunction==true));
    J.SetVisible("pnlOutDecay_Go", (CosFunction.bESDAutoDecayTeachFunction==true));
    J.SetVisible("pnlOutDecay_XY", (CosFunction.bESDAutoDecayTeachFunction==true));

    J.SetActivePageIndex("PageControl2", 8);                                             // :1599
    J.SetActivePage("PageControl2", "tsAxleCtrl");                                       // :1600
    J.SetActivePageIndex("pcHandPitch", 0);                                              // :1608

    J.SetVisible("pnlBinBox_Go", (IniConfig.bBinBox || CosFunction.bHWBinBox)?true:false);   // :1611
    J.SetVisible("pnlBinBox_XY", (IniConfig.bBinBox || CosFunction.bHWBinBox)?true:false);
    J.SetVisible("pnlBinBox_Place", (IniConfig.bBinBox || CosFunction.bHWBinBox)?true:false);

    J.SetVisible("palBarCodeInShuttle", !(BAR_CODE_INSTALL==ebctUninstall));              // :1617
    J.SetVisible("palBarCodeOutShuttle", !(BAR_CODE_INSTALL==ebctUninstall));
    J.SetVisible("palLaserInShuttle", !(USE_LASER_DISTANCE==0));
    J.SetVisible("pnlLaserHP1_Go", !(USE_LASER_DISTANCE==0));
    J.SetVisible("pnlLaserHP1_XY", !(USE_LASER_DISTANCE==0));
    J.SetVisible("pnlLaserHP2_Go", !(USE_LASER_DISTANCE==0));
    J.SetVisible("pnlLaserHP2_XY", !(USE_LASER_DISTANCE==0));

    {   // :1627-1647 Steven 20130126 Fix3 滿盤功能 -- golden sets setEditInSht1X / setEditInSht2X twice (:1641-1644), once is enough
        static const char* const kFix3[] = { "setEditInSht1Left", "setEditInSht1Right", "setEditInSht2Left", "setEditInSht2Right",
            "setEditOutSht1KitPos", "setEditOutSht2KitPos", "edtEditInSht1OctSiteKit", "edtEditInSht2OctSiteKit",
            "setEditOutSht1OneRowKit", "setEditOutSht2OneRowKit", "edtSetEditOS1BarCode", "edtSetEditOS2BarCode",
            "edtSetEditIS1BarCode", "edtSetEditIS2BarCode", "setEditInSht1X", "setEditInSht2X",
            "setEditFix1X", "setEditFix2X", "setEditFix3X" };
        for (unsigned i = 0; i < sizeof(kFix3) / sizeof(kFix3[0]); ++i)
            J.SetColor(kFix3[i], (FIX3_FULL_PLACE==Fix3K_ShortShuttle)?kclYellow:kclWhite);
    }

    J.SetVisible("grpRotate_Axis", ((USE_ROTATE_KIT==1) && (iRotate_Type==e1MotRotate || iRotate_Type==e1MotRotate1Dut || iRotate_Type==eInOutArm1Motor)));   // :1651
    J.SetVisible("grpRotate_Kit", ((USE_ROTATE_KIT==1) && (iRotate_Type==e1MotRotate || iRotate_Type==e1MotRotate1Dut || iRotate_Type==eInOutArm1Motor)));

    J.SetVisible("pnlInRot_Go", (((USE_ROTATE_KIT==1) && (iRotate_Type!=eCynRotate)) || (USE_DIE_CLEAN==1)));   // :1654
    J.SetVisible("pnlInRot_XY", (((USE_ROTATE_KIT==1) && (iRotate_Type!=eCynRotate)) || (USE_DIE_CLEAN==1)));
    J.SetVisible("grpInRot_Axis", ((USE_ROTATE_KIT==1)  && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut)));
    J.SetVisible("pnlInRot_Pick", (USE_ROTATE_KIT==1 || USE_DIE_CLEAN==1));
    J.SetVisible("pnlInRot_Place", (USE_ROTATE_KIT==1 || USE_DIE_CLEAN==1));

    J.SetVisible("pnlOutRot_Go", (((USE_ROTATE_KIT==1) && (iRotate_Type!=eCynRotate)) || (USE_DIE_CLEAN==1)));  // :1660
    J.SetVisible("pnlOutRot_XY", (((USE_ROTATE_KIT==1) && (iRotate_Type!=eCynRotate)) || (USE_DIE_CLEAN==1)));
    J.SetVisible("grpOutRot_Axis", ((USE_ROTATE_KIT==1)  && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut)));
    J.SetVisible("pnlOutRot_Pick", (USE_ROTATE_KIT==1 || USE_DIE_CLEAN==1));
    J.SetVisible("pnlOutRot_Place", (USE_ROTATE_KIT==1 || USE_DIE_CLEAN==1));

    J.SetTabVisible("tsRotate", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut)));   // :1668
    J.SetVisible("MotorInRC", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("MotorInRD", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("MotorInRG", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("MotorInRH", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("MotorOutRA", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("MotorOutRB", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("MotorOutRE", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("MotorOutRF", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));

    if(iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut)                      // :1678
    {
        /* golden :1680-1681 MotorOutRE->Flat=false; MotorOutRG->Flat=true; (bevel only) */
        J.SetVisible("lblOutRotateFixE", false);
    }
    else
    {
        /* golden :1686-1687 MotorOutRE->Flat=true; MotorOutRG->Flat=false; (bevel only) */
        J.SetVisible("lblOutRotateFixG", false);
    }
    J.SetVisible("grbInRB", ((USE_ROTATE_KIT==1) && (iRotate_Type!=e2MotRotate2Dut)));                 // :1690
    J.SetVisible("grbInRC", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("grbInRD", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("grbInRF", ((USE_ROTATE_KIT==1) && (iRotate_Type!=e2MotRotate2Dut)));
    J.SetVisible("grbInRG", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("grbInRH", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("grbOutRA", ((USE_ROTATE_KIT==1) && (iRotate_Type!=e2MotRotate2Dut)));
    J.SetVisible("grbOutRB", ((USE_ROTATE_KIT==1) && (iRotate_Type!=e2MotRotate2Dut)));
    J.SetVisible("grbOutRC", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate || iRotate_Type==e2MotRotate2Dut)));
    J.SetVisible("grbOutRD", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));
    J.SetVisible("grbOutRE", ((USE_ROTATE_KIT==1) && (iRotate_Type!=e2MotRotate2Dut)));
    J.SetVisible("grbOutRF", ((USE_ROTATE_KIT==1) && (iRotate_Type!=e2MotRotate2Dut)));
    J.SetVisible("grbOutRG", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate || iRotate_Type==e2MotRotate2Dut)));
    J.SetVisible("grbOutRH", ((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate)));

    J.SetCaption("lblInRot_XY", AnsiString((USE_DIE_CLEAN==1)?"CLEAN":"Rotate"));         // :1707

    const bool bXY = (USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable || USE_PICKER_COUNT==ep16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc);   // :1710 (same expression, golden repeats it per line)
    J.SetVisible("gbInX240mm", bXY);
    J.SetVisible("gbInX340mm", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("gbInX440mm", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("gbOutX240mm", bXY);
    J.SetVisible("gbOutX340mm", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("gbOutX440mm", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("gbInX2120", bXY);
    J.SetVisible("gbInX3120", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("gbInX4120", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("gbOutX2120", bXY);
    J.SetVisible("gbOutX3120", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("gbOutX4120", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("palXYPitch", bXY);
    J.SetVisible("pnlYPitch", bXY);
    if(bXY)                                                                              // :1724
    {
        bool bUse=(USE_OUT_ARM_Y_PITCH==iXYPitchVariable);                               // :1726
        J.SetVisible("btnOutXPitch2", bUse);
        J.SetVisible("setEditOutX240", bUse);
        J.SetVisible("setEditOutX2120", bUse);
        J.SetVisible("btnOutYPitch", bUse);
        J.SetVisible("setEditOutY15", bUse);
        J.SetVisible("setEditOutY60", bUse);
        J.SetVisible("gbOutX240mm", bUse);
        J.SetVisible("gbOutX2120", bUse);
        J.SetVisible("gbOutY15", bUse);
        J.SetVisible("gbOutY60", bUse);
    }
    J.SetVisible("palXYPitch1", (USE_PICKER_COUNT==ep16Picker));                         // :1738
    J.SetTabVisible("tsYPitch15", bXY);
    J.SetTabVisible("tsYPitch60", bXY);
    J.SetVisible("MotorInArmPitchY", bXY);
    J.SetVisible("MotorInArmPitchX2", bXY);
    J.SetVisible("MotorOutArmPitchY", (USE_OUT_Y_IS_AUTO_PITCH==iXYPitchVariable || USE_PICKER_COUNT==ep16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc));    // :1743
    J.SetVisible("MotorOutArmPitchX2", (USE_OUT_Y_IS_AUTO_PITCH==iXYPitchVariable || USE_PICKER_COUNT==ep16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc));

    J.SetVisible("pnlInArm16Z", (USE_PICKER_COUNT==ep16Picker));                          // :1746
    J.SetVisible("pnlOutArm16Z", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("MotorInArmPitchX3", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("MotorInArmPitchX4", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("MotorOutArmPitchX3", (USE_PICKER_COUNT==ep16Picker));
    J.SetVisible("MotorOutArmPitchX4", (USE_PICKER_COUNT==ep16Picker));

    J.SetTabVisible("tsShuttleSensor", (AUTO_SENSOR_INSTALL==1));                         // :1754

    J.SetVisible("gbTrayMapping", (USE_TRAY_MAPPING==etmInstall));                        // :1756
    J.SetTabVisible("tsHinge", (USE_LOADER_HINGE==1));
    J.SetTabVisible("tsMR", (USE_MR_SYSTEM));
    J.SetTabVisible("tsPickUpErrorPlacement", (fMain && fMain->cInplace) ? (fMain->cInplace->InArmPlacementEnable()) : false);   // :1759 (no fMain in this process => false, the port's offline InArmPlacementEnable)
    J.SetVisible("gbFix3", (FIX3_FULL_PLACE==Fix3K_UseStepperMotor));                     // :1760
    J.SetVisible("gbAxesCtrlFix3", (FIX3_FULL_PLACE==Fix3K_UseStepperMotor));
    J.SetCaption("SetButtonPlace6", AnsiString((FIX3_FULL_PLACE==Fix3K_UseStepperMotor)?"Fix3":"Fix2"));
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall ||                         // :1763
       USE_Top_Scanner_AOI_Inspection==true)
    {
        J.SetVisible("pnlPadView_Go", false);
        J.SetVisible("pnlPadView_Place", false);

        J.SetVisible("pnlBGAView_Go", false);
        J.SetVisible("pnlBGAView_Place", false);

        J.SetVisible("pnlTopViewKit_Pick", false);
        J.SetVisible("pnlTopViewKit_Place", false);

        J.SetVisible("pnlTopView_Go", (USE_Top_Scanner_AOI_Inspection==true));
        J.SetVisible("pnlTopView_Pick", (USE_Top_Scanner_AOI_Inspection==true));
        J.SetVisible("pnlTopView_Place", (USE_Top_Scanner_AOI_Inspection==true));

        J.SetVisible("pnlAOISafePos_Go", (USE_Top_Scanner_AOI_Inspection==true));
    }

    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)                       // :1782
    {
        // :1785-1803 every TSpeedButton / TEdit child of grpAOICtrl and of GroupBox1 ->Visible=false: the bridge has no
        //   control list, so it names the parent with a "/children:" key and the page hides those children
        J.SetVisible("grpAOICtrl/children:TSpeedButton,TEdit", false);
        J.SetVisible("GroupBox1/children:TSpeedButton,TEdit", false);

        J.SetCaption("SetBtnScannerAOI", AnsiString("TopBtm AOI"));                       // :1805
        J.SetVisible("SetBtnScannerAOI", true);
        J.SetVisible("GoBtnScannerAOI", true);
        J.SetVisible("setEditScannerAOIX", true);
        J.SetVisible("setEditScannerAOIY", true);
        J.SetVisible("lblAOI_X", true);
        J.SetVisible("lblAOI_Y", true);
        J.SetCaption("SetBtnScannerAOI_Z", AnsiString("TopBtm AOI Z"));
        J.SetVisible("SetBtnScannerAOI_Z", true);
        J.SetVisible("GoBtnScannerAOI_Z", true);
        J.SetVisible("setEditScannerAOIZ", true);
        J.SetVisible("Label6", true);
        J.SetVisible("gbTopBtmAOI", true);
        /* golden :1818 FrmAOI->fAOI_ReadFile(); (file side effect) */
    }

    // :1823-1826 gbTrayZ / grpTrayZAxis: page/ht9045_teach_trayz_c.js
    J.SetVisible("gbTrayArmZ", (TRAY_ARM_MODE==eUnderCoveyor));                           // :1827
    J.SetEnabled("MotorTrayZ", (TRAY_ARM_MODE==eUnderCoveyor));
    J.SetVisible("MotorTrayZ", (TRAY_ARM_MODE==eUnderCoveyor));
    J.SetVisible("pnlAuto4ZUp", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("pnlAuto5ZUp", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("pnlAuto6ZUp", (AUTO_EMPTY_COLOR>=4));

    J.SetVisible("pnl6Auto_Go", (AUTO_EMPTY_COLOR>=3));                                    // :1834
    J.SetVisible("pnlAuto6_Go", (AUTO_EMPTY_COLOR>=4));
    J.SetVisible("pnlAuto6_XY", (AUTO_EMPTY_COLOR>=4));
    J.SetVisible("pnlAuto4_XY", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("pnlAuto5_XY", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("pnlAuto6_XY", (AUTO_EMPTY_COLOR>=4));
    J.SetVisible("pnlFix4_XY", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("pnlFix5_XY", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("pnlFix6_XY", (AUTO_EMPTY_COLOR>=3));

    // :1844-1855 pnlLoaderZ ... grpAuto456: page/ht9045_teach_trayz_c.js
    J.SetVisible("btnAuto4", (AUTO_EMPTY_COLOR>=3));                                       // :1856
    J.SetVisible("GoBtnAuto4", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("setEdtAuto4", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("btnAuto5", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("GoBtnAuto5", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("setEdtAuto5", (AUTO_EMPTY_COLOR>=3));
    J.SetVisible("btnAuto6", (AUTO_EMPTY_COLOR>=4));
    J.SetVisible("GoBtnAuto6", (AUTO_EMPTY_COLOR>=4));
    J.SetVisible("setEdtAuto6", (AUTO_EMPTY_COLOR>=4));

    if(IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true &&                       // :1872
       (bY1ModifyDistanceRef==true || bY2ModifyDistanceRef==true))
    {
        J.SetActivePage("PageControl2", "TabSheet10");                                   // :1875 強制切換到index teaching分頁
        J.SetCaption("lblFindPhaseNotes", AnsiString("Index Y馬達尋相和原始值差異過大，請重新確認Index Y Teaching 點位"));
        /* golden :1877 lblFindPhaseNotes->Width=697; (geometry) */
    }
    else
    {
        J.SetCaption("lblFindPhaseNotes", AnsiString(" "));                               // :1881
    }

    J.SetCaption("lblArm1", AnsiString((USE_INDEX_ARM_AXES==IndexArm_3_Axis)?"Arm Y":"Arm 1"));   // :1888
    J.SetVisible("setEditIndex2ToSocketY", (USE_INDEX_ARM_AXES==IndexArm_3_Axis)?false:true);
    J.SetVisible("setEditIndex2ToSht2Y", (USE_INDEX_ARM_AXES==IndexArm_3_Axis)?false:true);
    J.SetVisible("lblArm2", (USE_INDEX_ARM_AXES==IndexArm_3_Axis)?false:true);
    if (fTeach) {                                                                        // :1893-1939 (TfTeach methods, forms/fTeach.cpp:518-550)
        static const char* const kMinX[] = { "SetButtonInX140", "SetButtonInX240", "SetButtonInX340", "SetButtonInX440",
                                             "SetButtonOutX140", "SetButtonOutX240", "SetButtonOutX340", "SetButtonOutX440" };
        for (unsigned i = 0; i < 8; ++i) J.SetCaption(kMinX[i], fTeach->GetMinXPitchCaptionName());
        J.SetCaption("gbInX140mm", fTeach->GetMinXPitchCaptionName("In XP 1"));
        J.SetCaption("gbInX240mm", fTeach->GetMinXPitchCaptionName("In XP 2"));
        J.SetCaption("gbInX340mm", fTeach->GetMinXPitchCaptionName("In XP 3"));
        J.SetCaption("gbInX440mm", fTeach->GetMinXPitchCaptionName("In XP 4"));
        J.SetCaption("gbOutX140mm", fTeach->GetMinXPitchCaptionName("Out XP 1"));
        J.SetCaption("gbOutX240mm", fTeach->GetMinXPitchCaptionName("Out XP 2"));
        J.SetCaption("gbOutX340mm", fTeach->GetMinXPitchCaptionName("Out XP 3"));
        J.SetCaption("gbOutX440mm", fTeach->GetMinXPitchCaptionName("Out XP 4"));
        J.SetCaption("tsXPitch40", fTeach->GetMinXPitchCaptionName("X Pitch"));
        static const char* const kMaxX[] = { "SetButtonInX1120", "SetButtonInX2120", "SetButtonInX3120", "SetButtonInX4120",
                                             "SetButtonOutX1120", "SetButtonOutX2120", "SetButtonOutX3120", "SetButtonOutX4120" };
        for (unsigned i = 0; i < 8; ++i) J.SetCaption(kMaxX[i], fTeach->GetMaxXPitchCaptionName());
        J.SetCaption("gbInX1120", fTeach->GetMaxXPitchCaptionName("In XP 1"));
        J.SetCaption("gbInX2120", fTeach->GetMaxXPitchCaptionName("In XP 2"));
        J.SetCaption("gbInX3120", fTeach->GetMaxXPitchCaptionName("In XP 3"));
        J.SetCaption("gbInX4120", fTeach->GetMaxXPitchCaptionName("In XP 4"));
        J.SetCaption("gbOutX1120", fTeach->GetMaxXPitchCaptionName("Out XP 1"));
        J.SetCaption("gbOutX2120", fTeach->GetMaxXPitchCaptionName("Out XP 2"));
        J.SetCaption("gbOutX3120", fTeach->GetMaxXPitchCaptionName("Out XP 3"));
        J.SetCaption("gbOutX4120", fTeach->GetMaxXPitchCaptionName("Out XP 4"));
        J.SetCaption("tsXPitch120", fTeach->GetMaxXPitchCaptionName("X Pitch"));
        J.SetCaption("SetButtonInY15", fTeach->GetMinYPitchCaptionName());
        J.SetCaption("SetButtonOutY15", fTeach->GetMinYPitchCaptionName());
        J.SetCaption("gbInY15", fTeach->GetMinYPitchCaptionName("In YP"));
        J.SetCaption("gbOutY15", fTeach->GetMinYPitchCaptionName("Out YP"));
        J.SetCaption("tsYPitch15", fTeach->GetMinYPitchCaptionName("Y Pitch"));
        J.SetCaption("SetButtonInY60", fTeach->GetMaxYPitchCaptionName());
        J.SetCaption("SetButtonOutY60", fTeach->GetMaxYPitchCaptionName());
        J.SetCaption("gbInY60", fTeach->GetMaxYPitchCaptionName("In YP"));
        J.SetCaption("gbOutY60", fTeach->GetMaxYPitchCaptionName("Out YP"));
        J.SetCaption("tsYPitch60", fTeach->GetMaxYPitchCaptionName("Y Pitch"));
    } else {
        J.Todo("TfTeach facade not built in this process: golden :1893-1939 X/Y pitch captions not computed (dfm captions stay)");
    }

    J.SetTabVisible("tsMagazine", (AUTO3_IS_MAGAZINE==1));                                // :1943
    J.SetTabVisible("tsOutSort", (USE_OUT_SORT_ARM!=eartUninstall));
    J.SetVisible("pgc_UsePitchX", (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be));
    /* golden :1946 TriTemp_Teach(); (SetMotorScaleSpeed for every motor -- machine state, not screen) */

    if(USE_OUT_SORT_ARM!=eartUninstall)                                                  // :1948
    {
        J.SetVisible("grpSortArmAxis", true);
        J.SetVisible("grpSortSHTAxis", true);
        /* golden :1952-1953 grpSortSHTAxis->Top / ->Width = gbOutSh2's (geometry) */
    }

    if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                              // :1956
    {
        J.SetVisible("grpInShLtcZ", true);
        J.SetVisible("grpInShLtcZPos", true);
        J.SetVisible("grpInShLtcPos", true);
        /* golden :1961 InSHZDownRange->Text=iInShtZRange; (the C path FileRW/Teach.cpp loads it) */
    }

    if(INSTALL_OCR_YMot==eocrYMotInstal)                                                 // :1964
    {
        J.SetVisible("grpLoadY", true);
        J.SetVisible("MotorLoaderYCCW", false);
        J.SetVisible("MotorAuto1YCW", false);
        J.SetVisible("MotorAuto1YCCW", false);
        J.SetVisible("MotorAuto2YCW", false);
        J.SetVisible("MotorAuto2YCCW", false);
    }
    else if(INSTALL_OCR!=eocrUninstal &&
            CosFunction.bTrayOCR)                                                        // :1973
    {
        J.SetVisible("grpLoadY", true);
        J.SetVisible("MotorLoaderYCCW", false);
        J.SetVisible("MotorAuto1YCW", false);
        J.SetVisible("MotorAuto1YCCW", false);
        J.SetVisible("MotorAuto2YCW", false);
        J.SetVisible("MotorAuto2YCCW", false);
        J.SetVisible("btnLoaderRotZ", false);
    }
    else if(USE_LdUldCassetteMode)                                                       // :1984
    {
        J.SetVisible("grpCassetteZ", true);
        J.SetCaption("grpLoadY", AnsiString("Cassette Y"));
        J.SetVisible("grpLoadY", true);
        J.SetVisible("btnLoaderRotZ", false);
    }
    else
    {
        J.SetVisible("grpLoadY", false);                                                 // :1993
    }

    if(INSTALL_OCR_YMot==eocrYMotInstal)                                                 // :1996
        J.SetVisible("gbOCRYMot", true);
    else
        J.SetVisible("gbOCRYMot", false);

    if(CUSTOMER_CODE==CC_ASE_CL)                                                         // :2001
    {
        J.SetVisible("gbContactRelative", true);
    }
    else
    {
        J.SetVisible("gbContactRelative", false);
    }
}

static void Display(FormState& J)
{
    B_FormShow(J);
}

}  // namespace f_TfTeach

extern const BridgeDesc kBridge_TfTeach = {
    "HW.teach.html", "TfTeach", "uteach.cpp",
    &f_TfTeach::Display, nullptr, nullptr,   // display only: no save bridge (Teach saves through the C path, FileRW/Teach.cpp)
    "",
    "",
    nullptr, 0,                              // no form.event table (the Teach buttons go through motor.access)
    ""
};

}  // namespace formbridge
}  // namespace ht9045
