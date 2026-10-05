// ===========================================================================
//  FileRW/Teach.gen.inc  --  產生的，不要手改（tools/gen_teach_editlist.py）。
//  golden：D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\uteach.cpp（cp950 → UTF-8）
//  1. IC_InitialTeachEditList = golden TfTeach::InitialTeachEditList（:3131-3358），203 筆 elTeach->Add，
//     元件引數改寫成具名替身 filerw::EL<TEdit>("TfTeach", 名稱)，其餘逐字。
//  2. kTeachKeyWidget：TECH_PARA 274 筆＋TECH_TWOPARA 52 筆的 Key → 畫面元件名稱（fTeachRegistry.cpp 的註解）。
//  3. kTeInArm／kTeOutArm／kTeSortArm：TECH_SUCKPARA 的元件（golden :286／:289／:292）。
// ===========================================================================
static void IC_InitialTeachEditList()   // golden uteach.cpp:3131
{
    bool bShow=true;
    bool bEnable=true;                                                          //, bDisable=false;
//    bool bNoShow=(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker)?true:false;
    bool bReadFromFile=true;
//    bool bFixedValue=(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||
//                      USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)?true:false;      //Ztex 2023.12.06 Add HT-1032

    bool bFixedValue=false;

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "SetEditAutoClean"),              &Teach.iAutoCleanPick           , ECInteger, "InArm",        "AutoCleanPick",           bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "setEdLoadCellY1"),               &Teach.iLoadCellY1              , ECInteger, "Index",        "iLoadCellY1",             bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "setEdLoadCellY2"),               &Teach.iLoadCellY2              , ECInteger, "Index",        "iLoadCellY2",             bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "setEdLoadCellZ1"),               &Teach.iLoadCellZ1Down          , ECInteger, "Index",        "iLoadCellZ1Down",         bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "setEdLoadCellZ2"),               &Teach.iLoadCellZ2Down          , ECInteger, "Index",        "iLoadCellZ2Down",         bShow, bEnable, bReadFromFile,   0, false);

    //舊資料
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Pitch_PosX[0]   , ECInteger, "ArmAlignment", "InArmAlignmentPitch1",    bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Pitch_PosX[1]   , ECInteger, "ArmAlignment", "InArmAlignmentPitch2",    bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Pitch_PosX[2]   , ECInteger, "ArmAlignment", "InArmAlignmentPitch3",    bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Pitch_PosX[3]   , ECInteger, "ArmAlignment", "InArmAlignmentPitch4",    bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Pitch_PosX[4]   , ECInteger, "ArmAlignment", "InArmAlignmentPitch5",    bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Pitch_PosX[5]   , ECInteger, "ArmAlignment", "InArmAlignmentPitch6",    bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosZ[0]  , ECInteger, "ArmAlignment", "InArmAlignmentPick1_Z",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosZ[1]  , ECInteger, "ArmAlignment", "InArmAlignmentPick2_Z",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosZ[2]  , ECInteger, "ArmAlignment", "InArmAlignmentPick3_Z",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosZ[3]  , ECInteger, "ArmAlignment", "InArmAlignmentPick4_Z",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosZ[4]  , ECInteger, "ArmAlignment", "InArmAlignmentPick5_Z",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosZ[5]  , ECInteger, "ArmAlignment", "InArmAlignmentPick6_Z",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosZ[6]  , ECInteger, "ArmAlignment", "InArmAlignmentPick7_Z",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosZ[7]  , ECInteger, "ArmAlignment", "InArmAlignmentPick8_Z",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosX[0]  , ECInteger, "ArmAlignment", "InArmAlignmentPick1_X",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosX[1]  , ECInteger, "ArmAlignment", "InArmAlignmentPick2_X",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosX[2]  , ECInteger, "ArmAlignment", "InArmAlignmentPick3_X",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosX[3]  , ECInteger, "ArmAlignment", "InArmAlignmentPick4_X",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosX[4]  , ECInteger, "ArmAlignment", "InArmAlignmentPick5_X",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosX[5]  , ECInteger, "ArmAlignment", "InArmAlignmentPick6_X",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosX[6]  , ECInteger, "ArmAlignment", "InArmAlignmentPick7_X",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosX[7]  , ECInteger, "ArmAlignment", "InArmAlignmentPick8_X",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosY[0]  , ECInteger, "ArmAlignment", "InArmAlignmentPick1_Y",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosY[1]  , ECInteger, "ArmAlignment", "InArmAlignmentPick2_Y",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosY[2]  , ECInteger, "ArmAlignment", "InArmAlignmentPick3_Y",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosY[3]  , ECInteger, "ArmAlignment", "InArmAlignmentPick4_Y",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosY[4]  , ECInteger, "ArmAlignment", "InArmAlignmentPick5_Y",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosY[5]  , ECInteger, "ArmAlignment", "InArmAlignmentPick6_Y",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosY[6]  , ECInteger, "ArmAlignment", "InArmAlignmentPick7_Y",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iInArmCCD_Picker_PosY[7]  , ECInteger, "ArmAlignment", "InArmAlignmentPick8_Y",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Pitch_PosX[0]  , ECInteger, "ArmAlignment", "OutArmAlignmentPitch1",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Pitch_PosX[1]  , ECInteger, "ArmAlignment", "OutArmAlignmentPitch2",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Pitch_PosX[2]  , ECInteger, "ArmAlignment", "OutArmAlignmentPitch3",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Pitch_PosX[3]  , ECInteger, "ArmAlignment", "OutArmAlignmentPitch4",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Pitch_PosX[4]  , ECInteger, "ArmAlignment", "OutArmAlignmentPitch5",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Pitch_PosX[5]  , ECInteger, "ArmAlignment", "OutArmAlignmentPitch6",   bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosZ[0] , ECInteger, "ArmAlignment", "OutArmAlignmentPick1_Z",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosZ[1] , ECInteger, "ArmAlignment", "OutArmAlignmentPick2_Z",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosZ[2] , ECInteger, "ArmAlignment", "OutArmAlignmentPick3_Z",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosZ[3] , ECInteger, "ArmAlignment", "OutArmAlignmentPick4_Z",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosZ[4] , ECInteger, "ArmAlignment", "OutArmAlignmentPick5_Z",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosZ[5] , ECInteger, "ArmAlignment", "OutArmAlignmentPick6_Z",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosZ[6] , ECInteger, "ArmAlignment", "OutArmAlignmentPick7_Z",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosZ[7] , ECInteger, "ArmAlignment", "OutArmAlignmentPick8_Z",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosX[0] , ECInteger, "ArmAlignment", "OutArmAlignmentPick1_X",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosX[1] , ECInteger, "ArmAlignment", "OutArmAlignmentPick2_X",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosX[2] , ECInteger, "ArmAlignment", "OutArmAlignmentPick3_X",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosX[3] , ECInteger, "ArmAlignment", "OutArmAlignmentPick4_X",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosX[4] , ECInteger, "ArmAlignment", "OutArmAlignmentPick5_X",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosX[5] , ECInteger, "ArmAlignment", "OutArmAlignmentPick6_X",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosX[6] , ECInteger, "ArmAlignment", "OutArmAlignmentPick7_X",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosX[7] , ECInteger, "ArmAlignment", "OutArmAlignmentPick8_X",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosY[0] , ECInteger, "ArmAlignment", "OutArmAlignmentPick1_Y",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosY[1] , ECInteger, "ArmAlignment", "OutArmAlignmentPick2_Y",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosY[2] , ECInteger, "ArmAlignment", "OutArmAlignmentPick3_Y",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosY[3] , ECInteger, "ArmAlignment", "OutArmAlignmentPick4_Y",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosY[4] , ECInteger, "ArmAlignment", "OutArmAlignmentPick5_Y",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosY[5] , ECInteger, "ArmAlignment", "OutArmAlignmentPick6_Y",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosY[6] , ECInteger, "ArmAlignment", "OutArmAlignmentPick7_Y",  bShow, bEnable, bReadFromFile,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),     &Tech._iOutArmCCD_Picker_PosY[7] , ECInteger, "ArmAlignment", "OutArmAlignmentPick8_Y",  bShow, bEnable, bReadFromFile,   0, false);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmCCDXResolution"),        &Tech.dInArmCCDXResolution      , ECDouble , "ArmAlignment", "InArmCCDXResolution",     bShow, bEnable, bReadFromFile, 0.0, false, 10.0, -10.0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmCCDYResolution"),        &Tech.dInArmCCDYResolution      , ECDouble , "ArmAlignment", "InArmCCDYResolution",     bShow, bEnable, bReadFromFile, 0.0, false, 10.0, -10.0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmCCDXRadian"),            &Tech.dInArmCCDXRadian          , ECDouble , "ArmAlignment", "InArmCCDXRadian",         bShow, bEnable, bReadFromFile, 0.0, false, 10.0, -10.0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmCCDYRadian"),            &Tech.dInArmCCDYRadian          , ECDouble , "ArmAlignment", "InArmCCDYRadian",         bShow, bEnable, bReadFromFile, 0.0, false, 10.0, -10.0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmCCDXResolution"),       &Tech.dOutArmCCDXResolution     , ECDouble , "ArmAlignment", "OutArmCCDXResolution",    bShow, bEnable, bReadFromFile, 0.0, false, 10.0, -10.0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmCCDYResolution"),       &Tech.dOutArmCCDYResolution     , ECDouble , "ArmAlignment", "OutArmCCDYResolution",    bShow, bEnable, bReadFromFile, 0.0, false, 10.0, -10.0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmCCDXRadian"),           &Tech.dOutArmCCDXRadian         , ECDouble , "ArmAlignment", "OutArmCCDXRadian",        bShow, bEnable, bReadFromFile, 0.0, false, 10.0, -10.0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmCCDYRadian"),           &Tech.dOutArmCCDYRadian         , ECDouble , "ArmAlignment", "OutArmCCDYRadian",        bShow, bEnable, bReadFromFile, 0.0, false, 10.0, -10.0);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXAa"),            &Tech.iInArmCCD_Pitch_PosX[0][0], ECInteger, "ArmAlignment", "InArmPitchXAlignmentAa",    bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXAb"),            &Tech.iInArmCCD_Pitch_PosX[0][1], ECInteger, "ArmAlignment", "InArmPitchXAlignmentAb",    bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXAc"),            &Tech.iInArmCCD_Pitch_PosX[0][2], ECInteger, "ArmAlignment", "InArmPitchXAlignmentAc",    bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXAd"),            &Tech.iInArmCCD_Pitch_PosX[0][3], ECInteger, "ArmAlignment", "InArmPitchXAlignmentAd",    bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXAe"),            &Tech.iInArmCCD_Pitch_PosX[0][4], ECInteger, "ArmAlignment", "InArmPitchXAlignmentAe",    bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXAf"),            &Tech.iInArmCCD_Pitch_PosX[0][5], ECInteger, "ArmAlignment", "InArmPitchXAlignmentAf",    bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXAg"),            &Tech.iInArmCCD_Pitch_PosX[0][6], ECInteger, "ArmAlignment", "InArmPitchXAlignmentAg",    bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),                       &Tech.iInArmCCD_Pitch_PosX[0][7], ECInteger, "ArmAlignment", "InArmPitchXAlignmentAh",    bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXBa"),            &Tech.iInArmCCD_Pitch_PosX[1][0], ECInteger, "ArmAlignment", "InArmPitchXAlignmentBa",    bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXBb"),            &Tech.iInArmCCD_Pitch_PosX[1][1], ECInteger, "ArmAlignment", "InArmPitchXAlignmentBb",    bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXBc"),            &Tech.iInArmCCD_Pitch_PosX[1][2], ECInteger, "ArmAlignment", "InArmPitchXAlignmentBc",    bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXBd"),            &Tech.iInArmCCD_Pitch_PosX[1][3], ECInteger, "ArmAlignment", "InArmPitchXAlignmentBd",    bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXBe"),            &Tech.iInArmCCD_Pitch_PosX[1][4], ECInteger, "ArmAlignment", "InArmPitchXAlignmentBe",    bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXBf"),            &Tech.iInArmCCD_Pitch_PosX[1][5], ECInteger, "ArmAlignment", "InArmPitchXAlignmentBf",    bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInAlignPitchXBg"),            &Tech.iInArmCCD_Pitch_PosX[1][6], ECInteger, "ArmAlignment", "InArmPitchXAlignmentBg",    bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),                       &Tech.iInArmCCD_Pitch_PosX[1][7], ECInteger, "ArmAlignment", "InArmPitchXAlignmentBh",    bShow, bEnable, bFixedValue,   0, false, 4000, 0);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmAlignPitchY"),           &Tech.iInArmCCD_Pitch_PosY      , ECInteger, "ArmAlignment", "InArmAlignmentPitch_Y",    bShow, bEnable, bReadFromFile,   0, false, 7500, 1500);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignAa"),             &Tech.iInArmCCD_Picker_PosZ[0][0], ECInteger, "ArmAlignment", "InArmZAlignmentAa",   bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignAb"),             &Tech.iInArmCCD_Picker_PosZ[0][1], ECInteger, "ArmAlignment", "InArmZAlignmentAb",   bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignAc"),             &Tech.iInArmCCD_Picker_PosZ[0][2], ECInteger, "ArmAlignment", "InArmZAlignmentAc",   bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignAd"),             &Tech.iInArmCCD_Picker_PosZ[0][3], ECInteger, "ArmAlignment", "InArmZAlignmentAd",   bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignAe"),             &Tech.iInArmCCD_Picker_PosZ[0][4], ECInteger, "ArmAlignment", "InArmZAlignmentAe",   bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignAf"),             &Tech.iInArmCCD_Picker_PosZ[0][5], ECInteger, "ArmAlignment", "InArmZAlignmentAf",   bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignAg"),             &Tech.iInArmCCD_Picker_PosZ[0][6], ECInteger, "ArmAlignment", "InArmZAlignmentAg",   bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignAh"),             &Tech.iInArmCCD_Picker_PosZ[0][7], ECInteger, "ArmAlignment", "InArmZAlignmentAh",   bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignBa"),             &Tech.iInArmCCD_Picker_PosZ[1][0], ECInteger, "ArmAlignment", "InArmZAlignmentBa",   bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignBb"),             &Tech.iInArmCCD_Picker_PosZ[1][1], ECInteger, "ArmAlignment", "InArmZAlignmentBb",   bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignBc"),             &Tech.iInArmCCD_Picker_PosZ[1][2], ECInteger, "ArmAlignment", "InArmZAlignmentBc",   bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignBd"),             &Tech.iInArmCCD_Picker_PosZ[1][3], ECInteger, "ArmAlignment", "InArmZAlignmentBd",   bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignBe"),             &Tech.iInArmCCD_Picker_PosZ[1][4], ECInteger, "ArmAlignment", "InArmZAlignmentBe",   bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignBf"),             &Tech.iInArmCCD_Picker_PosZ[1][5], ECInteger, "ArmAlignment", "InArmZAlignmentBf",   bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignBg"),             &Tech.iInArmCCD_Picker_PosZ[1][6], ECInteger, "ArmAlignment", "InArmZAlignmentBg",   bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmZAlignBh"),             &Tech.iInArmCCD_Picker_PosZ[1][7], ECInteger, "ArmAlignment", "InArmZAlignmentBh",   bShow, bEnable, bFixedValue,   0, false, -500, -3000);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignAa"),             &Tech.iInArmCCD_Picker_PosX[0][0], ECInteger, "ArmAlignment", "InArmXAlignmentAa",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignAb"),             &Tech.iInArmCCD_Picker_PosX[0][1], ECInteger, "ArmAlignment", "InArmXAlignmentAb",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignAc"),             &Tech.iInArmCCD_Picker_PosX[0][2], ECInteger, "ArmAlignment", "InArmXAlignmentAc",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignAd"),             &Tech.iInArmCCD_Picker_PosX[0][3], ECInteger, "ArmAlignment", "InArmXAlignmentAd",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignAe"),             &Tech.iInArmCCD_Picker_PosX[0][4], ECInteger, "ArmAlignment", "InArmXAlignmentAe",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignAf"),             &Tech.iInArmCCD_Picker_PosX[0][5], ECInteger, "ArmAlignment", "InArmXAlignmentAf",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignAg"),             &Tech.iInArmCCD_Picker_PosX[0][6], ECInteger, "ArmAlignment", "InArmXAlignmentAg",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignAh"),             &Tech.iInArmCCD_Picker_PosX[0][7], ECInteger, "ArmAlignment", "InArmXAlignmentAh",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignBa"),             &Tech.iInArmCCD_Picker_PosX[1][0], ECInteger, "ArmAlignment", "InArmXAlignmentBa",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignBb"),             &Tech.iInArmCCD_Picker_PosX[1][1], ECInteger, "ArmAlignment", "InArmXAlignmentBb",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignBc"),             &Tech.iInArmCCD_Picker_PosX[1][2], ECInteger, "ArmAlignment", "InArmXAlignmentBc",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignBd"),             &Tech.iInArmCCD_Picker_PosX[1][3], ECInteger, "ArmAlignment", "InArmXAlignmentBd",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignBe"),             &Tech.iInArmCCD_Picker_PosX[1][4], ECInteger, "ArmAlignment", "InArmXAlignmentBe",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignBf"),             &Tech.iInArmCCD_Picker_PosX[1][5], ECInteger, "ArmAlignment", "InArmXAlignmentBf",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignBg"),             &Tech.iInArmCCD_Picker_PosX[1][6], ECInteger, "ArmAlignment", "InArmXAlignmentBg",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmXAlignBh"),             &Tech.iInArmCCD_Picker_PosX[1][7], ECInteger, "ArmAlignment", "InArmXAlignmentBh",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignAa"),             &Tech.iInArmCCD_Picker_PosY[0][0], ECInteger, "ArmAlignment", "InArmYAlignmentAa",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignAb"),             &Tech.iInArmCCD_Picker_PosY[0][1], ECInteger, "ArmAlignment", "InArmYAlignmentAb",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignAc"),             &Tech.iInArmCCD_Picker_PosY[0][2], ECInteger, "ArmAlignment", "InArmYAlignmentAc",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignAd"),             &Tech.iInArmCCD_Picker_PosY[0][3], ECInteger, "ArmAlignment", "InArmYAlignmentAd",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignAe"),             &Tech.iInArmCCD_Picker_PosY[0][4], ECInteger, "ArmAlignment", "InArmYAlignmentAe",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignAf"),             &Tech.iInArmCCD_Picker_PosY[0][5], ECInteger, "ArmAlignment", "InArmYAlignmentAf",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignAg"),             &Tech.iInArmCCD_Picker_PosY[0][6], ECInteger, "ArmAlignment", "InArmYAlignmentAg",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignAh"),             &Tech.iInArmCCD_Picker_PosY[0][7], ECInteger, "ArmAlignment", "InArmYAlignmentAh",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignBa"),             &Tech.iInArmCCD_Picker_PosY[1][0], ECInteger, "ArmAlignment", "InArmYAlignmentBa",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignBb"),             &Tech.iInArmCCD_Picker_PosY[1][1], ECInteger, "ArmAlignment", "InArmYAlignmentBb",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignBc"),             &Tech.iInArmCCD_Picker_PosY[1][2], ECInteger, "ArmAlignment", "InArmYAlignmentBc",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignBd"),             &Tech.iInArmCCD_Picker_PosY[1][3], ECInteger, "ArmAlignment", "InArmYAlignmentBd",   bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignBe"),             &Tech.iInArmCCD_Picker_PosY[1][4], ECInteger, "ArmAlignment", "InArmYAlignmentBe",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignBf"),             &Tech.iInArmCCD_Picker_PosY[1][5], ECInteger, "ArmAlignment", "InArmYAlignmentBf",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignBg"),             &Tech.iInArmCCD_Picker_PosY[1][6], ECInteger, "ArmAlignment", "InArmYAlignmentBg",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtInArmYAlignBh"),             &Tech.iInArmCCD_Picker_PosY[1][7], ECInteger, "ArmAlignment", "InArmYAlignmentBh",   bShow, bEnable, bFixedValue,     0, false, 99999, -99999);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXAa"),          &Tech.iOutArmCCD_Pitch_PosX[0][0], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentAa",   bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXAb"),          &Tech.iOutArmCCD_Pitch_PosX[0][1], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentAb",   bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXAc"),          &Tech.iOutArmCCD_Pitch_PosX[0][2], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentAc",   bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXAd"),          &Tech.iOutArmCCD_Pitch_PosX[0][3], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentAd",   bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXAe"),          &Tech.iOutArmCCD_Pitch_PosX[0][4], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentAe",   bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXAf"),          &Tech.iOutArmCCD_Pitch_PosX[0][5], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentAf",   bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXAg"),          &Tech.iOutArmCCD_Pitch_PosX[0][4], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentAg",   bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),                      &Tech.iOutArmCCD_Pitch_PosX[0][5], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentAh",   bShow, bEnable, bFixedValue,   0, false);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXBa"),          &Tech.iOutArmCCD_Pitch_PosX[1][0], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentBa",   bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXBb"),          &Tech.iOutArmCCD_Pitch_PosX[1][1], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentBb",   bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXBc"),          &Tech.iOutArmCCD_Pitch_PosX[1][2], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentBc",   bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXBd"),          &Tech.iOutArmCCD_Pitch_PosX[1][3], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentBd",   bShow, bEnable, bReadFromFile,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXBe"),          &Tech.iOutArmCCD_Pitch_PosX[1][4], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentBe",   bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXBf"),          &Tech.iOutArmCCD_Pitch_PosX[1][5], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentBf",   bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutAlignPitchXBg"),          &Tech.iOutArmCCD_Pitch_PosX[1][4], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentBg",   bShow, bEnable, bFixedValue,   0, false, 4000, 0);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "EdtTemp"),                      &Tech.iOutArmCCD_Pitch_PosX[1][5], ECInteger, "ArmAlignment", "OutArmPitchXAlignmentBh",   bShow, bEnable, bFixedValue,   0, false);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmAlignPitchY"),         &Tech.iOutArmCCD_Pitch_PosY      , ECInteger, "ArmAlignment", "OutArmAlignmentPitch_Y",   bShow, bEnable, bReadFromFile,   0, false, 7500, 1500);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignAa"),           &Tech.iOutArmCCD_Picker_PosZ[0][0], ECInteger, "ArmAlignment", "OutArmZAlignmentAa",  bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignAb"),           &Tech.iOutArmCCD_Picker_PosZ[0][1], ECInteger, "ArmAlignment", "OutArmZAlignmentAb",  bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignAc"),           &Tech.iOutArmCCD_Picker_PosZ[0][2], ECInteger, "ArmAlignment", "OutArmZAlignmentAc",  bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignAd"),           &Tech.iOutArmCCD_Picker_PosZ[0][3], ECInteger, "ArmAlignment", "OutArmZAlignmentAd",  bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignAe"),           &Tech.iOutArmCCD_Picker_PosZ[0][4], ECInteger, "ArmAlignment", "OutArmZAlignmentAe",  bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignAf"),           &Tech.iOutArmCCD_Picker_PosZ[0][5], ECInteger, "ArmAlignment", "OutArmZAlignmentAf",  bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignAg"),           &Tech.iOutArmCCD_Picker_PosZ[0][6], ECInteger, "ArmAlignment", "OutArmZAlignmentAg",  bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignAh"),           &Tech.iOutArmCCD_Picker_PosZ[0][7], ECInteger, "ArmAlignment", "OutArmZAlignmentAh",  bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignBa"),           &Tech.iOutArmCCD_Picker_PosZ[1][0], ECInteger, "ArmAlignment", "OutArmZAlignmentBa",  bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignBb"),           &Tech.iOutArmCCD_Picker_PosZ[1][1], ECInteger, "ArmAlignment", "OutArmZAlignmentBb",  bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignBc"),           &Tech.iOutArmCCD_Picker_PosZ[1][2], ECInteger, "ArmAlignment", "OutArmZAlignmentBc",  bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignBd"),           &Tech.iOutArmCCD_Picker_PosZ[1][3], ECInteger, "ArmAlignment", "OutArmZAlignmentBd",  bShow, bEnable, bReadFromFile,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignBe"),           &Tech.iOutArmCCD_Picker_PosZ[1][4], ECInteger, "ArmAlignment", "OutArmZAlignmentBe",  bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignBf"),           &Tech.iOutArmCCD_Picker_PosZ[1][5], ECInteger, "ArmAlignment", "OutArmZAlignmentBf",  bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignBg"),           &Tech.iOutArmCCD_Picker_PosZ[1][6], ECInteger, "ArmAlignment", "OutArmZAlignmentBg",  bShow, bEnable, bFixedValue,   0, false, -500, -3000);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmZAlignBh"),           &Tech.iOutArmCCD_Picker_PosZ[1][7], ECInteger, "ArmAlignment", "OutArmZAlignmentBh",  bShow, bEnable, bFixedValue,   0, false, -500, -3000);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignAa"),           &Tech.iOutArmCCD_Picker_PosX[0][0], ECInteger, "ArmAlignment", "OutArmXAlignmentAa",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignAb"),           &Tech.iOutArmCCD_Picker_PosX[0][1], ECInteger, "ArmAlignment", "OutArmXAlignmentAb",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignAc"),           &Tech.iOutArmCCD_Picker_PosX[0][2], ECInteger, "ArmAlignment", "OutArmXAlignmentAc",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignAd"),           &Tech.iOutArmCCD_Picker_PosX[0][3], ECInteger, "ArmAlignment", "OutArmXAlignmentAd",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignAe"),           &Tech.iOutArmCCD_Picker_PosX[0][4], ECInteger, "ArmAlignment", "OutArmXAlignmentAe",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignAf"),           &Tech.iOutArmCCD_Picker_PosX[0][5], ECInteger, "ArmAlignment", "OutArmXAlignmentAf",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignAg"),           &Tech.iOutArmCCD_Picker_PosX[0][6], ECInteger, "ArmAlignment", "OutArmXAlignmentAg",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignAh"),           &Tech.iOutArmCCD_Picker_PosX[0][7], ECInteger, "ArmAlignment", "OutArmXAlignmentAh",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignBa"),           &Tech.iOutArmCCD_Picker_PosX[1][0], ECInteger, "ArmAlignment", "OutArmXAlignmentBa",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignBb"),           &Tech.iOutArmCCD_Picker_PosX[1][1], ECInteger, "ArmAlignment", "OutArmXAlignmentBb",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignBc"),           &Tech.iOutArmCCD_Picker_PosX[1][2], ECInteger, "ArmAlignment", "OutArmXAlignmentBc",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignBd"),           &Tech.iOutArmCCD_Picker_PosX[1][3], ECInteger, "ArmAlignment", "OutArmXAlignmentBd",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignBe"),           &Tech.iOutArmCCD_Picker_PosX[1][4], ECInteger, "ArmAlignment", "OutArmXAlignmentBe",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignBf"),           &Tech.iOutArmCCD_Picker_PosX[1][5], ECInteger, "ArmAlignment", "OutArmXAlignmentBf",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignBg"),           &Tech.iOutArmCCD_Picker_PosX[1][6], ECInteger, "ArmAlignment", "OutArmXAlignmentBg",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmXAlignBh"),           &Tech.iOutArmCCD_Picker_PosX[1][7], ECInteger, "ArmAlignment", "OutArmXAlignmentBh",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);

    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignAa"),           &Tech.iOutArmCCD_Picker_PosY[0][0], ECInteger, "ArmAlignment", "OutArmYAlignmentAa",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignAb"),           &Tech.iOutArmCCD_Picker_PosY[0][1], ECInteger, "ArmAlignment", "OutArmYAlignmentAb",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignAc"),           &Tech.iOutArmCCD_Picker_PosY[0][2], ECInteger, "ArmAlignment", "OutArmYAlignmentAc",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignAd"),           &Tech.iOutArmCCD_Picker_PosY[0][3], ECInteger, "ArmAlignment", "OutArmYAlignmentAd",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignAe"),           &Tech.iOutArmCCD_Picker_PosY[0][4], ECInteger, "ArmAlignment", "OutArmYAlignmentAe",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignAf"),           &Tech.iOutArmCCD_Picker_PosY[0][5], ECInteger, "ArmAlignment", "OutArmYAlignmentAf",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignAg"),           &Tech.iOutArmCCD_Picker_PosY[0][6], ECInteger, "ArmAlignment", "OutArmYAlignmentAg",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignAh"),           &Tech.iOutArmCCD_Picker_PosY[0][7], ECInteger, "ArmAlignment", "OutArmYAlignmentAh",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignBa"),           &Tech.iOutArmCCD_Picker_PosY[1][0], ECInteger, "ArmAlignment", "OutArmYAlignmentBa",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignBb"),           &Tech.iOutArmCCD_Picker_PosY[1][1], ECInteger, "ArmAlignment", "OutArmYAlignmentBb",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignBc"),           &Tech.iOutArmCCD_Picker_PosY[1][2], ECInteger, "ArmAlignment", "OutArmYAlignmentBc",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignBd"),           &Tech.iOutArmCCD_Picker_PosY[1][3], ECInteger, "ArmAlignment", "OutArmYAlignmentBd",  bShow, bEnable, bReadFromFile,   0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignBe"),           &Tech.iOutArmCCD_Picker_PosY[1][4], ECInteger, "ArmAlignment", "OutArmYAlignmentBe",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignBf"),           &Tech.iOutArmCCD_Picker_PosY[1][5], ECInteger, "ArmAlignment", "OutArmYAlignmentBf",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignBg"),           &Tech.iOutArmCCD_Picker_PosY[1][6], ECInteger, "ArmAlignment", "OutArmYAlignmentBg",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
    elTeach->Add(filerw::EL<TEdit>("TfTeach", "edtOutArmYAlignBh"),           &Tech.iOutArmCCD_Picker_PosY[1][7], ECInteger, "ArmAlignment", "OutArmYAlignmentBh",  bShow, bEnable, bFixedValue,     0, false, 99999, -99999);
}

static const char* const kTeachKeyWidget[][2] = {   // {Key, 元件名稱}
    { "SetEditAuto1CassetteZStart", "SetEditAuto1CassetteZStart" },
    { "SetEditAuto1Front", "SetEditAuto1Front" },
    { "SetEditAuto1FrontBack", "SetEditAuto1FrontBack" },
    { "SetEditAuto1Rear", "SetEditAuto1Rear" },
    { "SetEditAuto1RearBack", "SetEditAuto1RearBack" },
    { "SetEditAuto2CassetteZStart", "SetEditAuto2CassetteZStart" },
    { "SetEditAuto2Front", "SetEditAuto2Front" },
    { "SetEditAuto2FrontBack", "SetEditAuto2FrontBack" },
    { "SetEditAuto2Rear", "SetEditAuto2Rear" },
    { "SetEditAuto2RearBack", "SetEditAuto2RearBack" },
    { "SetEditAutoClean", "SetEditAutoClean" },
    { "SetEditHP", "SetEditHP" },
    { "SetEditLDCassetteZStart", "SetEditLDCassetteZStart" },
    { "SetEditLDFront", "SetEditLDFront" },
    { "SetEditLDFrontBack", "SetEditLDFrontBack" },
    { "SetEditLDRear", "SetEditLDRear" },
    { "SetEditLDRearBack", "SetEditLDRearBack" },
    { "SetEditPickInRotate", "SetEditPickInRotate" },
    { "SetEditPickLoader", "SetEditPickLoader" },
    { "SetEditPickOutRotate", "SetEditPickOutRotate" },
    { "SetEditPickOutSht", "SetEditPickOutSht" },
    { "SetEditPlaceAuto", "SetEditPlaceAuto" },
    { "SetEditPlaceFix", "SetEditPlaceFix" },
    { "SetEditPlaceFix2", "SetEditPlaceFix2" },
    { "SetEditPlaceInRotate", "SetEditPlaceInRotate" },
    { "SetEditPlaceInShuttle", "SetEditPlaceInShuttle" },
    { "SetEditPlaceNGBinBoxZ", "SetEditPlaceNGBinBoxZ" },
    { "SetEditPlaceOutRotate", "SetEditPlaceOutRotate" },
    { "SetEditPlacePreciserZ", "SetEditPlacePreciserZ" },
    { "edtBinBoxX", "edtBinBoxX" },
    { "edtBinBoxY", "edtBinBoxY" },
    { "edtBinBoxZ", "edtBinBoxZ" },
    { "edtBt2DX", "edtBt2DX" },
    { "edtBt2DY", "edtBt2DY" },
    { "edtEditInSht1OctSiteKit", "edtEditInSht1OctSiteKit" },
    { "edtEditInSht2OctSiteKit", "edtEditInSht2OctSiteKit" },
    { "edtEditMagZStandby", "edtEditMagZStandby" },
    { "edtEditMagZTray1", "edtEditMagZTray1" },
    { "edtInArmXBasePickerPos", "edtInArmXBasePickerPos" },
    { "edtInArmXCCDPos", "edtInArmXCCDPos" },
    { "edtInArmYBasePickerPos", "edtInArmYBasePickerPos" },
    { "edtInArmYCCDPos", "edtInArmYCCDPos" },
    { "edtOutArmXBasePickerPos", "edtOutArmXBasePickerPos" },
    { "edtOutArmXCCDPos", "edtOutArmXCCDPos" },
    { "edtOutArmYBasePickerPos", "edtOutArmYBasePickerPos" },
    { "edtOutArmYCCDPos", "edtOutArmYCCDPos" },
    { "edtOutSortArmAuto4_X", "edtOutSortArmAuto4_X" },
    { "edtOutSortArmAuto4_Y", "edtOutSortArmAuto4_Y" },
    { "edtOutSortArmAuto5_X", "edtOutSortArmAuto5_X" },
    { "edtOutSortArmAuto5_Y", "edtOutSortArmAuto5_Y" },
    { "edtOutSortArmAuto6_X", "edtOutSortArmAuto6_X" },
    { "edtOutSortArmAuto6_Y", "edtOutSortArmAuto6_Y" },
    { "edtOutSortArmShtL_X", "edtOutSortArmShtL_X" },
    { "edtOutSortArmShtL_Y", "edtOutSortArmShtL_Y" },
    { "edtOutSortSht_L", "edtOutSortSht_L" },
    { "edtOutSortSht_R", "edtOutSortSht_R" },
    { "edtOuttArmShtR_X", "edtOuttArmShtR_X" },
    { "edtOuttArmShtR_Y", "edtOuttArmShtR_Y" },
    { "edtSetEditIS1BarCode", "edtSetEditIS1BarCode" },
    { "edtSetEditIS2BarCode", "edtSetEditIS2BarCode" },
    { "edtSetEditOS1BarCode", "edtSetEditOS1BarCode" },
    { "edtSetEditOS2BarCode", "edtSetEditOS2BarCode" },
    { "edtSetOutArmToSortShtPlace", "edtSetOutArmToSortShtPlace" },
    { "edtSetPickSortSHT", "edtSetPickSortSHT" },
    { "edtSetPlaceSortAuto", "edtSetPlaceSortAuto" },
    { "edtSetSH1_16SiteKit", "edtSetSH1_16SiteKit" },
    { "edtSetSH2_16SiteKit", "edtSetSH2_16SiteKit" },
    { "edtSetSht1Laser", "edtSetSht1Laser" },
    { "edtSetSht2Laser", "edtSetSht2Laser" },
    { "edtSetSortZSafeHeight", "edtSetSortZSafeHeight" },
    { "edtsetSortXPitchMax", "edtsetSortXPitchMax" },
    { "edtsetSortXPitchMin", "edtsetSortXPitchMin" },
    { "setAuto1Z", "setAuto1Z" },
    { "setAuto1ZUp", "setAuto1ZUp" },
    { "setAuto2Z", "setAuto2Z" },
    { "setAuto2ZUp", "setAuto2ZUp" },
    { "setAuto3Z", "setAuto3Z" },
    { "setAuto3ZUp", "setAuto3ZUp" },
    { "setAuto4Z", "setAuto4Z" },
    { "setAuto4ZUp", "setAuto4ZUp" },
    { "setAuto5Z", "setAuto5Z" },
    { "setAuto5ZUp", "setAuto5ZUp" },
    { "setAuto6Z", "setAuto6Z" },
    { "setAuto6ZUp", "setAuto6ZUp" },
    { "setColorZ", "setColorZ" },
    { "setColorZUp", "setColorZUp" },
    { "setEdGabageX", "setEdGabageX" },
    { "setEdGabageY", "setEdGabageY" },
    { "setEdLoadCellY1", "setEdLoadCellY1" },
    { "setEdLoadCellY2", "setEdLoadCellY2" },
    { "setEdLoadCellZ1", "setEdLoadCellZ1" },
    { "setEdLoadCellZ2", "setEdLoadCellZ2" },
    { "setEditAlignInZAa", "setEditAlignInZAa" },
    { "setEditAlignInZAb", "setEditAlignInZAb" },
    { "setEditAlignInZAc", "setEditAlignInZAc" },
    { "setEditAlignInZAd", "setEditAlignInZAd" },
    { "setEditAlignInZAe", "setEditAlignInZAe" },
    { "setEditAlignInZAf", "setEditAlignInZAf" },
    { "setEditAlignInZAg", "setEditAlignInZAg" },
    { "setEditAlignInZAh", "setEditAlignInZAh" },
    { "setEditAlignInZBa", "setEditAlignInZBa" },
    { "setEditAlignInZBb", "setEditAlignInZBb" },
    { "setEditAlignInZBc", "setEditAlignInZBc" },
    { "setEditAlignInZBd", "setEditAlignInZBd" },
    { "setEditAlignInZBe", "setEditAlignInZBe" },
    { "setEditAlignInZBf", "setEditAlignInZBf" },
    { "setEditAlignInZBg", "setEditAlignInZBg" },
    { "setEditAlignInZBh", "setEditAlignInZBh" },
    { "setEditAlignOutZAa", "setEditAlignOutZAa" },
    { "setEditAlignOutZAb", "setEditAlignOutZAb" },
    { "setEditAlignOutZAc", "setEditAlignOutZAc" },
    { "setEditAlignOutZAd", "setEditAlignOutZAd" },
    { "setEditAlignOutZAe", "setEditAlignOutZAe" },
    { "setEditAlignOutZAf", "setEditAlignOutZAf" },
    { "setEditAlignOutZAg", "setEditAlignOutZAg" },
    { "setEditAlignOutZAh", "setEditAlignOutZAh" },
    { "setEditAlignOutZBa", "setEditAlignOutZBa" },
    { "setEditAlignOutZBb", "setEditAlignOutZBb" },
    { "setEditAlignOutZBc", "setEditAlignOutZBc" },
    { "setEditAlignOutZBd", "setEditAlignOutZBd" },
    { "setEditAlignOutZBe", "setEditAlignOutZBe" },
    { "setEditAlignOutZBf", "setEditAlignOutZBf" },
    { "setEditAlignOutZBg", "setEditAlignOutZBg" },
    { "setEditAlignOutZBh", "setEditAlignOutZBh" },
    { "setEditAuto1X", "setEditAuto1X" },
    { "setEditAuto1Y", "setEditAuto1Y" },
    { "setEditAuto2X", "setEditAuto2X" },
    { "setEditAuto2Y", "setEditAuto2Y" },
    { "setEditAuto3X", "setEditAuto3X" },
    { "setEditAuto3Y", "setEditAuto3Y" },
    { "setEditAuto4X", "setEditAuto4X" },
    { "setEditAuto4Y", "setEditAuto4Y" },
    { "setEditAuto5X", "setEditAuto5X" },
    { "setEditAuto5Y", "setEditAuto5Y" },
    { "setEditAuto6X", "setEditAuto6X" },
    { "setEditAuto6Y", "setEditAuto6Y" },
    { "setEditAutoCleanX", "setEditAutoCleanX" },
    { "setEditAutoCleanY", "setEditAutoCleanY" },
    { "setEditBGAViewX", "setEditBGAViewX" },
    { "setEditBGAViewY", "setEditBGAViewY" },
    { "setEditBGAViewZ", "setEditBGAViewZ" },
    { "setEditBuffer10X", "setEditBuffer10X" },
    { "setEditBuffer10Z", "setEditBuffer10Z" },
    { "setEditBuffer1X", "setEditBuffer1X" },
    { "setEditBuffer1Z", "setEditBuffer1Z" },
    { "setEditBuffer2X", "setEditBuffer2X" },
    { "setEditBuffer2Z", "setEditBuffer2Z" },
    { "setEditBuffer3X", "setEditBuffer3X" },
    { "setEditBuffer3Z", "setEditBuffer3Z" },
    { "setEditBuffer4X", "setEditBuffer4X" },
    { "setEditBuffer4Z", "setEditBuffer4Z" },
    { "setEditBuffer5X", "setEditBuffer5X" },
    { "setEditBuffer5Z", "setEditBuffer5Z" },
    { "setEditBuffer6X", "setEditBuffer6X" },
    { "setEditBuffer6Z", "setEditBuffer6Z" },
    { "setEditBuffer7X", "setEditBuffer7X" },
    { "setEditBuffer7Z", "setEditBuffer7Z" },
    { "setEditBuffer8X", "setEditBuffer8X" },
    { "setEditBuffer8Z", "setEditBuffer8Z" },
    { "setEditBuffer9X", "setEditBuffer9X" },
    { "setEditBuffer9Z", "setEditBuffer9Z" },
    { "setEditCCDYCal1x1", "setEditCCDYCal1x1" },
    { "setEditCCDYSite1x1", "setEditCCDYSite1x1" },
    { "setEditCatchMagFront", "setEditCatchMagFront" },
    { "setEditCatchMagRear", "setEditCatchMagRear" },
    { "setEditFix1X", "setEditFix1X" },
    { "setEditFix1Y", "setEditFix1Y" },
    { "setEditFix2X", "setEditFix2X" },
    { "setEditFix2Y", "setEditFix2Y" },
    { "setEditFix3X", "setEditFix3X" },
    { "setEditFix3Y", "setEditFix3Y" },
    { "setEditFix4X", "setEditFix4X" },
    { "setEditFix4Y", "setEditFix4Y" },
    { "setEditFix5X", "setEditFix5X" },
    { "setEditFix5Y", "setEditFix5Y" },
    { "setEditFix6X", "setEditFix6X" },
    { "setEditFix6Y", "setEditFix6Y" },
    { "setEditHP1X", "setEditHP1X" },
    { "setEditHP1Y", "setEditHP1Y" },
    { "setEditHP2X", "setEditHP2X" },
    { "setEditHP2Y", "setEditHP2Y" },
    { "setEditHingeRotateEmpty", "setEditHingeRotateEmpty" },
    { "setEditHingeRotateLoader", "setEditHingeRotateLoader" },
    { "setEditInRA", "setEditInRA" },
    { "setEditInRB", "setEditInRB" },
    { "setEditInRC", "setEditInRC" },
    { "setEditInRD", "setEditInRD" },
    { "setEditInRE", "setEditInRE" },
    { "setEditInRF", "setEditInRF" },
    { "setEditInRG", "setEditInRG" },
    { "setEditInRH", "setEditInRH" },
    { "setEditInSh1LtcSenZ1", "setEditInSh1LtcSenZ1" },
    { "setEditInSh1LtcSenZ2", "setEditInSh1LtcSenZ2" },
    { "setEditInSh2LtcSenZ1", "setEditInSh2LtcSenZ1" },
    { "setEditInSh2LtcSenZ2", "setEditInSh2LtcSenZ2" },
    { "setEditInSht1Left", "setEditInSht1Left" },
    { "setEditInSht1Right", "setEditInSht1Right" },
    { "setEditInSht1X", "setEditInSht1X" },
    { "setEditInSht1Y", "setEditInSht1Y" },
    { "setEditInSht2Left", "setEditInSht2Left" },
    { "setEditInSht2Right", "setEditInSht2Right" },
    { "setEditInSht2X", "setEditInSht2X" },
    { "setEditInSht2Y", "setEditInSht2Y" },
    { "setEditInX2120", "setEditInX2120" },
    { "setEditInX240", "setEditInX240" },
    { "setEditInX3120", "setEditInX3120" },
    { "setEditInX340", "setEditInX340" },
    { "setEditInX4120", "setEditInX4120" },
    { "setEditInX440", "setEditInX440" },
    { "setEditInXPitch120", "setEditInXPitch120" },
    { "setEditInXPitch40", "setEditInXPitch40" },
    { "setEditInY15", "setEditInY15" },
    { "setEditInY60", "setEditInY60" },
    { "setEditInZSafeHeight", "setEditInZSafeHeight" },
    { "setEditInarmPlacementX", "setEditInarmPlacementX" },
    { "setEditInarmPlacementXOffsetByBasicSuck", "setEditInarmPlacementXOffsetByBasicSuck" },
    { "setEditInarmPlacementY", "setEditInarmPlacementY" },
    { "setEditInarmPlacementYOffsetByBasicSuck", "setEditInarmPlacementYOffsetByBasicSuck" },
    { "setEditIndex1ToOutSht1Z", "setEditIndex1ToOutSht1Z" },
    { "setEditIndex1ToSht1Y", "setEditIndex1ToSht1Y" },
    { "setEditIndex1ToSht1Z", "setEditIndex1ToSht1Z" },
    { "setEditIndex1ToSocketY", "setEditIndex1ToSocketY" },
    { "setEditIndex2ToSht2Y", "setEditIndex2ToSht2Y" },
    { "setEditIndex2ToSht2Z", "setEditIndex2ToSht2Z" },
    { "setEditIndex2ToSocketY", "setEditIndex2ToSocketY" },
    { "setEditLoadPort1Z", "setEditLoadPort1Z" },
    { "setEditLoadPort2Z", "setEditLoadPort2Z" },
    { "setEditLoadPort3Z", "setEditLoadPort3Z" },
    { "setEditLoadPort4Z", "setEditLoadPort4Z" },
    { "setEditLoadPortBufferZ", "setEditLoadPortBufferZ" },
    { "setEditLoadPortZ", "setEditLoadPortZ" },
    { "setEditLoadSafeZ", "setEditLoadSafeZ" },
    { "setEditLoadTemporaryZ", "setEditLoadTemporaryZ" },
    { "setEditLoadXGabage", "setEditLoadXGabage" },
    { "setEditLoadYGabage", "setEditLoadYGabage" },
    { "setEditLoaderX", "setEditLoaderX" },
    { "setEditLoaderY", "setEditLoaderY" },
    { "setEditNGBinBoxX", "setEditNGBinBoxX" },
    { "setEditNGBinBoxY", "setEditNGBinBoxY" },
    { "setEditOutRA", "setEditOutRA" },
    { "setEditOutRB", "setEditOutRB" },
    { "setEditOutRC", "setEditOutRC" },
    { "setEditOutRD", "setEditOutRD" },
    { "setEditOutRE", "setEditOutRE" },
    { "setEditOutRF", "setEditOutRF" },
    { "setEditOutRG", "setEditOutRG" },
    { "setEditOutRH", "setEditOutRH" },
    { "setEditOutSht1KitPos", "setEditOutSht1KitPos" },
    { "setEditOutSht1Left", "setEditOutSht1Left" },
    { "setEditOutSht1OneRowKit", "setEditOutSht1OneRowKit" },
    { "setEditOutSht1Right", "setEditOutSht1Right" },
    { "setEditOutSht1X", "setEditOutSht1X" },
    { "setEditOutSht1Y", "setEditOutSht1Y" },
    { "setEditOutSht2KitPos", "setEditOutSht2KitPos" },
    { "setEditOutSht2Left", "setEditOutSht2Left" },
    { "setEditOutSht2OneRowKit", "setEditOutSht2OneRowKit" },
    { "setEditOutSht2Right", "setEditOutSht2Right" },
    { "setEditOutSht2X", "setEditOutSht2X" },
    { "setEditOutSht2Y", "setEditOutSht2Y" },
    { "setEditOutX2120", "setEditOutX2120" },
    { "setEditOutX240", "setEditOutX240" },
    { "setEditOutX3120", "setEditOutX3120" },
    { "setEditOutX340", "setEditOutX340" },
    { "setEditOutX4120", "setEditOutX4120" },
    { "setEditOutX440", "setEditOutX440" },
    { "setEditOutXPitch120", "setEditOutXPitch120" },
    { "setEditOutXPitch40", "setEditOutXPitch40" },
    { "setEditOutY15", "setEditOutY15" },
    { "setEditOutY60", "setEditOutY60" },
    { "setEditOutZSafeHeight", "setEditOutZSafeHeight" },
    { "setEditPADViewX", "setEditPADViewX" },
    { "setEditPADViewY", "setEditPADViewY" },
    { "setEditPADViewZ", "setEditPADViewZ" },
    { "setEditPreciserPitchClose", "setEditPreciserPitchClose" },
    { "setEditPreciserPitchOpen", "setEditPreciserPitchOpen" },
    { "setEditPreciserX", "setEditPreciserX" },
    { "setEditPreciserY", "setEditPreciserY" },
    { "setEditRotateA", "setEditRotateA" },
    { "setEditRotateOutA", "setEditRotateOutA" },
    { "setEditRotateOutX", "setEditRotateOutX" },
    { "setEditRotateOutY", "setEditRotateOutY" },
    { "setEditRotateX", "setEditRotateX" },
    { "setEditRotateY", "setEditRotateY" },
    { "setEditSafePosX", "setEditSafePosX" },
    { "setEditSafePosY", "setEditSafePosY" },
    { "setEditScannerAOIX", "setEditScannerAOIX" },
    { "setEditScannerAOIY", "setEditScannerAOIY" },
    { "setEditScannerAOIZ", "setEditScannerAOIZ" },
    { "setEditSht1BusyHalf", "setEditSht1BusyHalf" },
    { "setEditSht1Pitch120", "setEditSht1Pitch120" },
    { "setEditSht1Pitch180", "setEditSht1Pitch180" },
    { "setEditSht1XGabage", "setEditSht1XGabage" },
    { "setEditSht1YGabage", "setEditSht1YGabage" },
    { "setEditSht2BusyHalf", "setEditSht2BusyHalf" },
    { "setEditSht2Pitch120", "setEditSht2Pitch120" },
    { "setEditSht2Pitch180", "setEditSht2Pitch180" },
    { "setEditSht2XGabage", "setEditSht2XGabage" },
    { "setEditSht2YGabage", "setEditSht2YGabage" },
    { "setEditStackedAuto1X", "setEditStackedAuto1X" },
    { "setEditStackedAuto1Z", "setEditStackedAuto1Z" },
    { "setEditStackedAuto2X", "setEditStackedAuto2X" },
    { "setEditStackedAuto2Z", "setEditStackedAuto2Z" },
    { "setEditStackedAuto3X", "setEditStackedAuto3X" },
    { "setEditStackedAuto3Z", "setEditStackedAuto3Z" },
    { "setEditStackedConversionX", "setEditStackedConversionX" },
    { "setEditStackedConversionZ", "setEditStackedConversionZ" },
    { "setEditStackedEmptyX", "setEditStackedEmptyX" },
    { "setEditStackedEmptyZ", "setEditStackedEmptyZ" },
    { "setEditStackedLoaderX", "setEditStackedLoaderX" },
    { "setEditStackedLoaderZ", "setEditStackedLoaderZ" },
    { "setEditTestZSafePos", "setEditTestZSafePos" },
    { "setEditTopViewKitZ", "setEditTopViewKitZ" },
    { "setEditTopViewKitZup", "setEditTopViewKitZup" },
    { "setEditTopViewX", "setEditTopViewX" },
    { "setEditTopViewY", "setEditTopViewY" },
    { "setEditTopView_Pick", "setEditTopView_Pick" },
    { "setEditTopView_Place", "setEditTopView_Place" },
    { "setEditTrayAuto1X", "setEditTrayAuto1X" },
    { "setEditTrayAuto2X", "setEditTrayAuto2X" },
    { "setEditTrayAuto3X", "setEditTrayAuto3X" },
    { "setEditTrayBracketConversionZ", "setEditTrayBracketConversionZ" },
    { "setEditTrayBracketSaftZ", "setEditTrayBracketSaftZ" },
    { "setEditTrayCleanX", "setEditTrayCleanX" },
    { "setEditTrayColorX", "setEditTrayColorX" },
    { "setEditTrayEmptyX", "setEditTrayEmptyX" },
    { "setEditTrayIDX", "setEditTrayIDX" },
    { "setEditTrayLoaderX", "setEditTrayLoaderX" },
    { "setEditTrayMapX", "setEditTrayMapX" },
    { "setEditTrayOCRX", "setEditTrayOCRX" },
    { "setEditUnloadPort1Z", "setEditUnloadPort1Z" },
    { "setEditUnloadPort2Z", "setEditUnloadPort2Z" },
    { "setEditUnloadPort3Z", "setEditUnloadPort3Z" },
    { "setEditUnloadPort4Z", "setEditUnloadPort4Z" },
    { "setEditUnloadPortBufferZ", "setEditUnloadPortBufferZ" },
    { "setEditWaitTestZDown", "setEditWaitTestZDown" },
    { "setEdtAuto4", "setEdtAuto4" },
    { "setEdtAuto5", "setEdtAuto5" },
    { "setEdtAuto6", "setEdtAuto6" },
    { "setEdtHP1LaserX", "setEdtHP1LaserX" },
    { "setEdtHP1LaserY", "setEdtHP1LaserY" },
    { "setEdtHP2LaserX", "setEdtHP2LaserX" },
    { "setEdtHP2LaserY", "setEdtHP2LaserY" },
    { "setEdtINDecayX", "setEdtINDecayX" },
    { "setEdtINDecayY", "setEdtINDecayY" },
    { "setEdtOUTDecayX", "setEdtOUTDecayX" },
    { "setEdtOUTDecayY", "setEdtOUTDecayY" },
    { "setEmptyZ", "setEmptyZ" },
    { "setEmptyZUp", "setEmptyZUp" },
    { "setFix3L", "setFix3L" },
    { "setFix3R", "setFix3R" },
    { "setInPickX", "setInPickX" },
    { "setInPickY", "setInPickY" },
    { "setLoaderZ", "setLoaderZ" },
    { "setLoaderZUp", "setLoaderZUp" },
    { "setOutPickX", "setOutPickX" },
    { "setOutPickY", "setOutPickY" },
    { "setYCarPos", "setYCarPos" },
    { "setYOCRPos", "setYOCRPos" },
    { "setYSurePos", "setYSurePos" },
    { "seteditContactZ1Relative", "seteditContactZ1Relative" },
    { "seteditContactZ2Relative", "seteditContactZ2Relative" },
};
static const int kTeachKeyWidgetCount = (int)(sizeof(kTeachKeyWidget) / sizeof(kTeachKeyWidget[0]));

static const char* const kTeInArm[2][8]  = {{ "setEditZ1A", "setEditZ1C", "setEditZ1E", "setEditZ1G", "setEditZ1I", "setEditZ1K", "setEditZ1M", "setEditZ1O" },
                                            { "setEditZ1B", "setEditZ1D", "setEditZ1F", "setEditZ1H", "setEditZ1J", "setEditZ1L", "setEditZ1N", "setEditZ1P" }};
static const char* const kTeOutArm[2][8] = {{ "setEditZ2A", "setEditZ2C", "setEditZ2E", "setEditZ2G", "setEditZ2I", "setEditZ2K", "setEditZ2M", "setEditZ2O" },
                                            { "setEditZ2B", "setEditZ2D", "setEditZ2F", "setEditZ2H", "setEditZ2J", "setEditZ2L", "setEditZ2N", "setEditZ2P" }};
static const char* const kTeSortArm[2]   = { "editsetEditZ3A", "editsetEditZ3B" };
