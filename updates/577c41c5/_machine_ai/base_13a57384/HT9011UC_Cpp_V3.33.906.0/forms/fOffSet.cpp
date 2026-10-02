// =============================================================================
//  forms/fOffSet.cpp  --  definitions for the fOffSet facade
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.cpp by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Bodies moved VERBATIM.
// =============================================================================
#include "forms/fOffSet.h"

// --- W6.2: TfOffSet --------------------------------------------------------
bool TfOffSet::UseAutoOffsetFunction(AnsiString /*sName*/) { return false; }  // no auto-offset offline
// AI(W906-ARM1) 20260927: RETIRED stub TfOffSet::UseInArmSetupTeach（以前回 false）—— real body at end of file (golden cOffSet.cpp:2955-2974)
TfOffSet *fOffSet = new TfOffSet();

// =============================================================================
//  AI(W906-FW-NOTE-W33) 20260826: cOffSet Wave A -- 22 支唯讀方向 methods。
//  上面的 W7-F0 內容一行未動（append-only）。
//
//  波次範圍、分母重量方式、取批標準、DEVIATION 與完整 GATE REGISTER 都在
//  forms/fOffSet.h 的檔頭 banner，這裡不複製第二份（避免兩份漂移）。
//
//  INCLUDES -- 為什麼不需要動 CMakeLists：
//    cmydef.cpp / cprod.cpp / Config 相關全域都在 ht9045_globals；
//    common.cpp（OnlyNumberAndDotInPut / ReadIniData）在 ht9045_core；
//    forms/fQwertyKey.cpp 與 forms/fMain.cpp 與本檔同屬 ht9045_forms
//    （CMakeLists.txt:655/:699 vs 本檔 :661）。
//    ht9045_forms 已經宣告 PUBLIC vclcompat + ht9045_globals + ht9045_core
//    （CMakeLists.txt:714-721），所以本波沒有新的 link edge，
//    也沒有新的 translation unit。
//    這與 forms/fSetup.cpp 的既有作法完全一致。
// =============================================================================
#include "cmydef.h"          // CUSTOMER_CODE / USE_PICKER_COUNT / iSortUnloadT6 / Tempture_Hot
#include "cprod.h"           // InputLimit / Temperature / TrayForm / UserDefForm_File / DeviceForm
#include "Config.h"          // IniConfig
#include "LastSet.h"         // LastSet
#include "common.h"          // OnlyNumberAndDotInPut / ReadIniData
#include "MachineType.h"     // CC_KYEC_LEE / CC_ASE_CL / ep16Picker / eBtnAOI_*
#include "vclcompat/SysUtils.h"  // FloatToStr
#include "forms/fQwertyKey.h"    // fQwertyKey (:406) + ShowQwertyKey (:370)
#include "forms/fMain.h"         // fMain->cbSetupFileName
#include <cstdlib>               // AI(W906-FW3-OFS2) 20260827: atof (edReleaseMouseDown)

// ---------------------------------------------------------------------------
//  golden cOffSet.cpp:3219 `bool bhasKeyDown=false;` -- golden 的 file-scope
//  global，見 forms/fOffSet.h 的宣告註解。這裡照 golden 原字寫 =false。
// ---------------------------------------------------------------------------
bool bhasKeyDown = false;

// ---------------------------------------------------------------------------
//  golden cOffSet.cpp:31-86 -- `AnsiString CapStr[OfsTotal]=...` -- golden's
//  file-scope global (cOffSet.h:503 `extern AnsiString CapStr[];`), not a
//  TfOffSet member. Literal table copied verbatim (index == enum eSECSOffset
//  value from MachineType.h, already ported/live -- checked 20260827).
//  AI(W906-FW3-OFS2) 20260827: added -- edArmXMouseDown / edReleaseMouseDown
//  both index it via `palOffsetParts->Caption==CapStr[OfsXxx]`.
// ---------------------------------------------------------------------------
AnsiString CapStr[OfsTotal]=
{
    "Loader",                           //0
    "Hot Plate1",                       //1
    "Hot Plate2",                       //2
    "Input Shuttle1",                   //3
    "Input Shuttle2",                   //4
    "Output Shuttle1",                  //5
    "Output Shuttle2",                  //6
    "Auto1",                            //7
    "Auto2",                            //8
    "Auto3",                            //9
    "Auto4",                            //10
    "Auto5",                            //11
    "Auto6",                            //12
    "Fix1",                             //13
    "Fix2",                             //14
    "Fix3",                             //15
    "Fix4",                             //16
    "Fix5",                             //17
    "Fix6",                             //18
    "Auto Clean",                       //19
    "OCR",                              //20
    "Input Rotate",                     //21
    "Output Rotate",                    //22
    "Top View",                         //23
    "PAD View",                         //24
    "BGA View",                         //25
    "Loader Row B",                     //26
    "In Shuttle 1 Left B",              //27
    "In Shuttle 1 Right A",             //28
    "In Shuttle 1 Right B",             //29
    "In Shuttle 2 Left B",              //30
    "In Shuttle 2 Right A",             //31
    "In Shuttle 2 Right B",             //32
    "Input Shuttle1 Auto Clean",        //33
    "In Shuttle 1 Left B Auto Clean",   //34
    "In Shuttle 1 Right A Auto Clean",  //35
    "In Shuttle 1 Right B Auto Clean",  //36
    "Input Shuttle2 Auto Clean",        //37
    "In Shuttle 2 Left B Auto Clean",   //38
    "In Shuttle 2 Right A Auto Clean",  //39
    "In Shuttle 2 Right B Auto Clean",  //40
    "Auto Shuttle 1",                   //41
    "Auto Shuttle 2",                   //42
    "Preciser",                         //43
    "Out Shuttle 1 Left B",             //44
    "Out Shuttle 1 Right A",            //45
    "Out Shuttle 1 Right B",            //46
    "Out Shuttle 2 Left B",             //47
    "Out Shuttle 2 Right A",            //48
    "Out Shuttle 2 Right B",            //49
    "Scan AOI",                         //50
    "InArm Placement",                  //51
    "Bottom 2D",                        //52
};

// ---------------------------------------------------------------------------
//  golden cOffSet.cpp:158-171 -- `AnsiString SpecialOffSetName[trayOfsTotal]`
//  -- another golden file-scope global. ⚠ Unlike CapStr, golden cOffSet.h
//  has **no** `extern` for this one (checked 20260827: 0 hits outside
//  cOffSet.cpp) -- so it's scoped to this stand-in .cpp only, same as golden.
//  AI(W906-FW3-OFS2) 20260827: added -- edReleaseMouseDown indexes it via
//  `pnlIndexOffset->Caption==SpecialOffSetName[tOfsIndex1/2]`.
// ---------------------------------------------------------------------------
AnsiString SpecialOffSetName[trayOfsTotal]=
{
    "TrayArm && Loader Track",
    "TrayArm && Empty Track",
    "TrayArm && Color Track",
    "TrayArm && Auto1 Track",
    "TrayArm && Auto2 Track",
    "TrayArm && Auto3 Track",
    "TrayArm && Auto4 Track",
    "TrayArm && Auto5 Track",
    "TrayArm && Auto6 Track",
    "TestArm1 && Shuttle1",
    "TestArm2 && Shuttle2"
};

// ---------------------------------------------------------------------------
//  golden cOffSet.cpp:856-882 -- SetXYPitchVCLVisible
//  純 widget Visible 串接 + 一個 USE_PICKER_COUNT 分支。golden 原文逐字。
//  ⚠ golden :858 的 `if(bVisible)` 只包住 edPitchX1 一行（Steven 20151103
//  的 32Site YPitch 修正），其餘六顆是無條件指派 —— 看起來像縮排錯誤，
//  但那是 golden 自己的行為，照翻不修。
//  ⚠ offline 語意：本樹沒有視窗，Visible 只是存值，不會有任何東西顯示或
//  隱藏。USE_PICKER_COUNT 的初值是 1（cmydef.cpp:3154），ep16Picker==3
//  （MachineType.h:1312），所以 offline 走的是 else 臂 —— 四顆 PitchX3/X4
//  一律 false，與 golden 在非 16-picker 機台上的行為相同。
// ---------------------------------------------------------------------------
void TfOffSet::SetXYPitchVCLVisible(bool bVisible)                              //Steven 20140510 : XY變距
{
    if(bVisible)                                                                //Steven 20151103 : Fixed for 32Site YPitch
    {
        edPitchX1->Visible  =bVisible;
    }
    lblPitchX2->Visible     =bVisible;
    lblPitchY->Visible      =bVisible;
    edPitchX2->Visible      =bVisible;
    edPitchY->Visible       =bVisible;
    lblPitchX2Range->Visible=bVisible;
    lblPitchYRange->Visible =bVisible;
    if(USE_PICKER_COUNT==ep16Picker)
    {
        lblPitchX3->Visible     =bVisible;
        lblPitchX4->Visible     =bVisible;
        edPitchX3->Visible      =bVisible;
        edPitchX4->Visible      =bVisible;
    }
    else
    {
        lblPitchX3->Visible     =false;
        lblPitchX4->Visible     =false;
        edPitchX3->Visible      =false;
        edPitchX4->Visible      =false;
    }
}

// golden cOffSet.cpp:2792-2795 -- `TObject *Sender` dropped, never read (D-1).
// `Key=NULL` 是 golden 原字，同 forms/fSetup.cpp:74 的既有先例；
// target 帶 -Wno-conversion-null（CMakeLists.txt:728）所以不會有警告。
void TfOffSet::edArmXKeyPress(char &Key)
{
    if(OnlyNumberAndDotInPut(Key)==false)   Key=NULL;
}

// golden cOffSet.cpp:2797-2801 -- `TObject *Sender` dropped, never read (D-1).
// Close() 是 offline no-op（D-2）：淨效果只有 sbtExit->Down=false。
void TfOffSet::sbtExitClick()
{
    sbtExit->Down=false;
    Close();
}

// golden cOffSet.cpp:2881-2886 -- Sender 直接宣告成 TEdit*（golden 自己就是
// cast 目標），`TMouseButton Button, TShiftState Shift, int X, int Y` 全部
// dropped（本體從未讀取）。D-1。
// ⚠ ShowQwertyKey 的 offline 語意見 forms/fQwertyKey.h BEHAVIOUR NOTE：
// ShowModal() 是 no-op，等於「使用者開了鍵盤立刻送出」，所以這裡的淨效果是
// 把 Sender->Text 過一次 CheckRange 再寫回（bCheckRange=true 且 N_DOUBLE）。
// 那是 golden 自己的 submit 路徑，不是本移植的發明。
// ⚠ fQwertyKey 是裸全域指標，全樹唯一建立點是 Public/HTEdit.cpp 的 lazy
// new，所以平時是 NULL；本支今天全樹 0 個 caller（不接線，facade 規則 3），
// 所以到不了。安全是因為沒接線，不是因為有守衛 —— 與 forms/fSetup.h
// GATE (WA-1) 已開閘的兄弟站點同一曝露。
void TfOffSet::IndexArmOffSet3MouseDown(TEdit *Sender)
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, InputLimit.dContactHigh, InputLimit.dContactLow); //Steven 20140123 : Contact Height的Offset限制
    iIndexChange=3;                                                             //kevin 20211211 index socket value change
}

// golden cOffSet.cpp:3049-3053 -- 同上 D-1 / ShowQwertyKey 說明。
// golden 的兩個 `(double)` cast 保留原字（dShuttleHigh/Low 在 cprod.h 已是
// double，cast 是 golden 自己的冗字，照翻不清理）。
void TfOffSet::IndexArmOffSet5MouseDown(TEdit *Sender)
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dShuttleHigh, (double)InputLimit.dShuttleLow);  //ChungHung 20150115 add for ATK +/-2 mm
}

// golden cOffSet.cpp:3055-3086 -- 同上 D-1。
// ⚠ GOLDEN ODDITY，照翻並記錄：:3071-3082 的三分支在 dTrayXPitch 與
// dTrayYPitch 相等時走 else，指派的仍是 dTrayXPitch，與第一個 if 臂同值 ——
// 也就是說 else if 那支只在 Y>X 時有作用，整段等價於 max(X,Y)。
// golden 這樣寫，本移植不「寫得更好」。
// ⚠ 最後一行把 max 當上限、-(max) 當下限傳給 ShowQwertyKey；注意 golden 的
// 參數順序是 (…, min, max)（見 forms/fQwertyKey.h:370），所以 golden 這裡
// 傳的是 min=dMaxDimension、max=-(dMaxDimension)，上下限是反的。
// 這同樣是 golden 自己的行為 —— 兄弟站點 IndexArmOffSet5MouseDown 傳的是
// (High, Low) 也是同一個反向 —— 照翻，不修。
void TfOffSet::edShtFor2DMouseDown(TEdit *Sender)
{
    double dMaxDimension;                                                       //Frank 20171030 (Steven) add Floating Shuttle調整Offset
    double dTrayXPitch=0.0;
    double dTrayYPitch=0.0;

    dTrayXPitch=UserDefForm_File[TrayForm.Loader.iTrayType].XPitch;
    dTrayYPitch=UserDefForm_File[TrayForm.Loader.iTrayType].YPitch;

    if(UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1)                //Ifor 20251017 add:2D Offset改用IC大小判斷
    {
        dMaxDimension=120.0;
    }
    else
    {
        if(dTrayXPitch>dTrayYPitch)
        {
            dMaxDimension=dTrayXPitch;
        }
        else if(dTrayYPitch>dTrayXPitch)
        {
            dMaxDimension=dTrayYPitch;
        }
        else
        {
            dMaxDimension=dTrayXPitch;
        }
    }

    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, dMaxDimension, -(dMaxDimension));
}

// golden cOffSet.cpp:3088-3092 -- 同上 D-1。
// ⚠ GOLDEN ODDITY：第三個實參 golden 寫 `0.0`，但 ShowQwertyKey 的第三個
// 形參是 `int iDP`（forms/fQwertyKey.h:370）。double 0.0 隱式轉成 int 0。
// 保留 golden 原字 `0.0` 而不「順手」改成 0 —— 值相同，字面忠實。
void TfOffSet::edPreciserOpenMouseDown(TEdit *Sender)
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 0.0, true, 0.5, 0.0);  //Frank 20180410 (Steven) : InArm Preciser Station
}

// golden cOffSet.cpp:3220-3227 -- `TObject *Sender, WORD &Key, TShiftState
// Shift` 全部 dropped（本體從未讀取）。D-1。
// ⚠ CUSTOMER_CODE 的 offline 值決定這支等不等於 no-op：非 KYEC 機台上
// 整段不執行，bhasKeyDown 永遠留在 false。
void TfOffSet::IndexArmOffSet1KeyDown()
{
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20190930 add : KYEC Index Offset 讀檔不卡Range，有修改才卡Range
    {
        bhasKeyDown=true;
    }
}

// golden cOffSet.cpp:3229-3237 -- 同上 D-1。
// 注意 iIndexChange=1 在 if 之外，非 KYEC 機台也會執行（golden 就是這樣）。
void TfOffSet::IndexArmOffSet3KeyDown()
{
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20190930 add : KYEC Index Offset 讀檔不卡Range，有修改才卡Range
    {
        bhasKeyDown=true;
    }
    iIndexChange=1;                                                             //kevin 20211211 index socket value change
}

// golden cOffSet.cpp:3239-3247 -- 同上 D-1。
void TfOffSet::IndexArmOffSet1KeyUp()
{
    if(CUSTOMER_CODE==CC_KYEC_LEE && bhasKeyDown==true)                         //Ifor 20190930 add : KYEC Index Offset 讀檔不卡Range，有修改才卡Range
    {
        IndexArmOffSet1->Text="0";
        bhasKeyDown=false;
    }
}

// golden cOffSet.cpp:3249-3257 -- 同上 D-1。
void TfOffSet::IndexArmOffSet3KeyUp()
{
    if(CUSTOMER_CODE==CC_KYEC_LEE && bhasKeyDown==true)                         //Ifor 20190930 add : KYEC Index Offset 讀檔不卡Range，有修改才卡Range
    {
        IndexArmOffSet3->Text="0";
        bhasKeyDown=false;
    }
}

// golden cOffSet.cpp:3259-3262 -- `TObject *Sender` dropped, never read (D-1).
void TfOffSet::IndexArmOffSet3Change()
{
    iIndexChange=2;                                                             //kevin 20211211 index socket value change
}

// golden cOffSet.cpp:3264-3278 -- `TObject *Sender` dropped, never read (D-1).
// ⚠ offline：USE_Scanner_AOI_Inspection 由 database.cpp:1431 從 ini 讀入，
// 預設 eBtnAOI_UniInstall(0)，兩個比較都 false，所以整支是 no-op ——
// 這與 golden 在沒裝 Scanner AOI 的機台上完全相同，不是移植的降級。
// Height 只存值不排版（D-4）。
void TfOffSet::ck_ScanAOIClick()
{
    if((USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall) ||
        (USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall))                   //Jimmychiu 20240322 : Top & Bottom Inspect
    {
        if(ck_ScanAOI->Checked)
        {
            Pnl_ScanAOI->Height=81;
        }
        else
        {
            Pnl_ScanAOI->Height=33;
        }
    }
}

// golden cOffSet.cpp:3341-3345 -- 同上 D-1 / ShowQwertyKey 說明。
void TfOffSet::edOffsetContactForceMouseDown(TEdit *Sender)
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 3, true, 0.0, 10.0);   //JimmyChiu 20220114 : Index 總壓力 Offset，加總後數值不顯示於Contact Form
}

// golden cOffSet.cpp:3347-3350 -- 純顯示。DeviceForm.fAireForce 是 double
// （cprod.h:1191），FloatToStr 取 double（vclcompat/SysUtils.h:40）。
void TfOffSet::ShowFinalAirForce()                                              //JimmyChiu 20220114 : Index 總壓力 Offset，加總後數值不顯示於Contact Form
{
    lbFinalAirForce->Caption=FloatToStr(DeviceForm.fAireForce);
}

// golden cOffSet.cpp:3363-3367 -- `TObject *Sender` dropped, never read (D-1).
// ActivePage 只存值不切頁（D-3）。
void TfOffSet::btnBackClick()
{
    btnBack->Down=false;
    PageControl1->ActivePage=tsInOutArmOffset;
}

// golden cOffSet.cpp:3376-3380 -- 同上 D-1 / D-3。
void TfOffSet::btnToIndexOffsetClick()
{
    btnToIndexOffset->Down=false;
    PageControl1->ActivePage=tsIndexOffset;
}

// golden cOffSet.cpp:3382-3386 -- 同上 D-1 / D-3。
void TfOffSet::btnToArmOffsetClick()
{
    btnToArmOffset->Down=false;
    PageControl1->ActivePage=tsInOutArmOffset;
}

// ---------------------------------------------------------------------------
//  golden cOffSet.cpp:3526-3543 -- Tri_Position_Offset
//  純讀 Temperature.fWorkTemperBase（cprod.h:1646 SYSTEM_TEMPERATURE）與
//  LastSet.iTemperature（LastSet.h:587），回傳一段檔名尾巴。不開檔、不寫檔。
//  ⚠ GOLDEN ODDITY，照翻並記錄：兩個臂的條件在 fWorkTemperBase==25 時
//  同時成立，第一個 if 先贏，所以 25 度整會拿到 "cool"。golden 這樣寫。
//  ⚠ 回傳字串前面的 "\\" 是 golden 原字（單一反斜線），呼叫端負責接在
//  目錄後面。本波沒有呼叫端 —— 這支今天全樹 0 個 caller。
// ---------------------------------------------------------------------------
AnsiString TfOffSet::Tri_Position_Offset()                                      //Ztex 2024.03.29 Tri Temp Position Offset
{
    AnsiString sResult="";

    if(Temperature.fWorkTemperBase<=25 && LastSet.iTemperature==Tempture_Hot)
    {
        sResult="\\Position Offset cool.Data";
    }
    else if(Temperature.fWorkTemperBase>=25 && LastSet.iTemperature==Tempture_Hot)
    {
        sResult="\\Position Offset Hot.Data";
    }
    else
    {
        sResult="\\Position Offset.Data";
    }
    return sResult;
}

// golden cOffSet.cpp:3545-3549 -- 同上 D-1 / D-3。
void TfOffSet::sbZcalibrationClick()
{
    sbZcalibration->Down=false;
    PageControl1->ActivePage=tsZCalibration;
}

// ---------------------------------------------------------------------------
//  golden cOffSet.cpp:3956-3994 -- LoadASECLOffsetIndexValues
//  純讀：ReadIniData 是 common.cpp 的 **純讀** 家族（回傳預設值時不回寫），
//  與 CheckAndReadIniData 不同 —— 後者在 key 不存在時會 WriteInteger 種回
//  檔案（common.cpp:603），那是寫入路徑，本波一律不用。這支用的是前者。
//  ⚠ 讀的檔是 D:\HT9045\Data\<setup>.ini，不是 system\Gerneral.ini，
//  所以與 --dry 保護的那條路無關；而且是唯讀，不會改動客戶檔案。
//  ⚠ offline：CUSTOMER_CODE 不是 CC_ASE_CL(933) 時 :3963-3964 直接 return 0，
//  連檔案都不會開。也就是說非日月光昆山機台上這支不碰磁碟。
//  ⚠ golden :3959 的 `AnsiString(...)` 是把 TComboBox->Text 這個 VCL
//  property 轉成 AnsiString 的 BCB6 慣用寫法；本樹 fMain->cbSetupFileName
//  ->Text 已經是 AnsiString，這個 cast 因此是恆等的，保留原字。
//  golden :3965-3967 的三行註解掉的 iGearRatio 一併保留（沿革）。
// ---------------------------------------------------------------------------
int TfOffSet::LoadASECLOffsetIndexValues(int iType, int iPos)
{
    AnsiString aPath, S;
    S=AnsiString(fMain->cbSetupFileName->Text);
    aPath.sprintf("D:\\HT9045\\Data\\%s.ini",S);
    int iOffsetPos=0;

    if(CUSTOMER_CODE!=CC_ASE_CL)
        return 0;
//    double iGearRatio=0.5;
//    if(IndexGearRatio==1)
//        iGearRatio=1.0;

    switch(iType)
    {
        case 14:
            if(iPos==0)
            {
                iOffsetPos = ReadIniData(aPath,"Index1","ShuttleLOffset",0);
            }
            else
            {
                iOffsetPos = ReadIniData(aPath,"Index1","ShuttleROffset",0);
            }
            break;

        case 15:
            if( iPos==0 )
            {
                iOffsetPos = ReadIniData(aPath,"Index2","ShuttleLOffset",0);
            }
            else
            {
                iOffsetPos = ReadIniData(aPath,"Index2","ShuttleROffset",0);
            }
            break;
    }
    return iOffsetPos;
}

// golden cOffSet.cpp:3997-4000 -- 同上 D-1 / ShowQwertyKey 說明。
void TfOffSet::edLodXClick(TEdit *Sender)
{
    fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 6, true, 0.95, 1.05);
}

// =============================================================================
//  AI(W906-FW3-OFS2) 20260827: cOffSet Wave B -- 3 支唯讀方向 methods。
//  波次範圍、分母重量方式、取批標準與 GATE (O-10)/(O-11) 都在
//  forms/fOffSet.h 的檔頭 banner，這裡不複製第二份。
//
//  沒有新增 link edge：cprod.h（InputLimit/DeviceForm）、Config.h
//  （IniConfig）、MachineType.h（eSECSOffset/eTrayOffset 兩個 enum）都已在
//  本檔既有 include 清單裡；此波唯一的新 include 是 <cstdlib>（atof）。
// =============================================================================

// golden cOffSet.cpp:2601-2652 -- edArmXMouseDown
// `TObject *Sender` 從未以原型別讀取，唯一用法都是 `(TEdit *)Sender`，故直接
// 宣告成 TEdit*，C-style cast 消失（D-1）。TMouseButton Button, TShiftState
// Shift, int X, int Y 全部 dropped（本體從未讀取）。
// ⚠ offline：IniConfig.bSPILFunction 預設 false（Config.cpp 零初始化），
// iNowOffsetSel 預設 -1，兩者都不命中任何具名分支 -> 落到最後一個 else，
// 傳 InputLimit.iOffsetXYHigh/iOffsetXYLow 給 ShowQwertyKey。與 golden 在
// 未裝 SPIL 功能、尚未選定 offset part 的機台上行為相同。
void TfOffSet::edArmXMouseDown(TEdit *Sender)                                      //JerryYang 20220923 : 矽品蘇州要求offset limit要By區域設定
{
    if(IniConfig.bSPILFunction)
    {
        if(iNowOffsetSel==OfsOCR)                                                  //JerryYang 20240111 : add
        {
            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 100.0, -100.0);
        }
        else if(palOffsetParts->Caption==CapStr[OfsLoader])
        {
            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dLoaderOffsetXYHigh, (double)InputLimit.dLoaderOffsetXYLow);
        }
        else if(palOffsetParts->Caption==CapStr[OfsHP1] || palOffsetParts->Caption==CapStr[OfsHP2])
        {
            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dHPOffsetXYHigh, (double)InputLimit.dHPOffsetXYLow);
        }
        else if(palOffsetParts->Caption==CapStr[OfsInSh1]   || palOffsetParts->Caption==CapStr[OfsInSh2]   ||
                palOffsetParts->Caption==CapStr[OfsInSh1LB] || palOffsetParts->Caption==CapStr[OfsInSh1RA] ||
                palOffsetParts->Caption==CapStr[OfsInSh1RB] || palOffsetParts->Caption==CapStr[OfsInSh2LB] ||
                palOffsetParts->Caption==CapStr[OfsInSh2RA] || palOffsetParts->Caption==CapStr[OfsInSh2RB])
        {
            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dInShtOffsetXYHigh, (double)InputLimit.dInShtOffsetXYLow);
        }
        else if(palOffsetParts->Caption==CapStr[OfsOutSh1]   || palOffsetParts->Caption==CapStr[OfsOutSh1]   ||
                palOffsetParts->Caption==CapStr[OfsOutSh1LB] || palOffsetParts->Caption==CapStr[OfsOutSh1RB] ||
                palOffsetParts->Caption==CapStr[OfsOutSh1RB] || palOffsetParts->Caption==CapStr[OfsOutSh2LB] ||
                palOffsetParts->Caption==CapStr[OfsOutSh2RA] || palOffsetParts->Caption==CapStr[OfsOutSh2RB])
        {
            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dOutShtOffsetXYHigh, (double)InputLimit.dOutShtOffsetXYLow);
        }
        else if(palOffsetParts->Caption==CapStr[OfsAuto1] || palOffsetParts->Caption==CapStr[OfsAuto2] || palOffsetParts->Caption==CapStr[OfsAuto3] ||
                palOffsetParts->Caption==CapStr[OfsAuto4] || palOffsetParts->Caption==CapStr[OfsAuto5] || palOffsetParts->Caption==CapStr[OfsAuto6] ||
                palOffsetParts->Caption==CapStr[OfsFix1]  || palOffsetParts->Caption==CapStr[OfsFix2]  || palOffsetParts->Caption==CapStr[OfsFix3]  ||
                palOffsetParts->Caption==CapStr[OfsFix4]  || palOffsetParts->Caption==CapStr[OfsFix5]  || palOffsetParts->Caption==CapStr[OfsFix6] )
        {
            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dUnloadOffsetXYHigh, (double)InputLimit.dUnloadOffsetXYLow);
        }
        else
        {
            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetXYHigh, (double)InputLimit.iOffsetXYLow);
        }
    }
    else if(iNowOffsetSel==OfsOCR)
    {
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 100.0, -100.0);
    }
    else
    {
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetXYHigh, (double)InputLimit.iOffsetXYLow);
    }
}

// golden cOffSet.cpp:2654-2785 -- edReleaseMouseDown
// 同上 D-1：Buffer 直接接 Sender（golden 自己 `Buffer=(TEdit *)Sender;`，這裡
// cast 消失）；TMouseButton Button, TShiftState Shift, int X, int Y dropped。
// ⚠ GOLDEN ODDITY，照翻並記錄（Tag provenance，同 forms/fOffSet.h GATE
// (O-9) 記錄過的同一個坑）：:2707 的 `Buffer->Tag==99` 在本樹 Tag 一律讀 0
// （vclcompat TControl::Tag 的 dfm 設計期值本樹不載入，見 O-9 的完整說明），
// 所以這個特殊分支（KYEC Index Pickup Offset -0.5~10 的窄範圍）offline 永遠
// 不會命中，落到下一層 IniConfig.bSPILFunction 判斷。與 (O-9) 不同的是這裡
// 選錯的只是「傳給 ShowQwertyKey 的鍵盤數字範圍」，不是寫入會被別的模組消費
// 的全域，所以沒有 GATE，照翻並在此明講。
void TfOffSet::edReleaseMouseDown(TfOffSetEdit *Sender)
{
    int iPick1, iRelease1, i;
    double fBuf;
    TfOffSetEdit *Buffer;
    Buffer=Sender;
    //Steven 20210317 : 針對release要求unloader增設立設定
    //==>
    int iZLimitHigh;
    int iZLimitLow;
    if(iNowOffsetSel==OfsAuto1 ||
       iNowOffsetSel==OfsAuto2 ||
       iNowOffsetSel==OfsAuto3 ||
       iNowOffsetSel==OfsAuto4 ||                                                  //Steven 20230907 : For HT-9011UC
       iNowOffsetSel==OfsAuto5 ||
       iNowOffsetSel==OfsAuto6 ||
       iNowOffsetSel==OfsFix1  ||
       iNowOffsetSel==OfsFix2  ||
       iNowOffsetSel==OfsFix3  ||
       iNowOffsetSel==OfsFix4  ||                                                  //Steven 20230907 : For HT-9011UC
       iNowOffsetSel==OfsFix5  ||
       iNowOffsetSel==OfsFix6)
    {
        iZLimitHigh=InputLimit.iOffsetUnloaderZHigh;
        iZLimitLow =InputLimit.iOffsetUnloaderZLow;
    }
    else
    {
        iZLimitHigh=InputLimit.iOffsetZHigh;
        iZLimitLow =InputLimit.iOffsetZLow;
    }
    //<==
    //Steven 20210317 : 針對release要求unloader增設立設定

    //Steven 20090805 : Z Offset
    if(iNowOffsetSel==OfsRotate_In || iNowOffsetSel==OfsRotate_Out)                //In Rotate & Out Rotate
    {
        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, 5.0, -5.0);           //Steven 20141120 : Modify
    }
    else
    {
        if(IniConfig.bChangeKitNoHardStop && PageControl1->ActivePageIndex==1 &&   //jou 2015-12-08 Xilinx 版本用
           (pnlIndexOffset->Caption==SpecialOffSetName[tOfsIndex1] ||
            pnlIndexOffset->Caption==SpecialOffSetName[tOfsIndex2]))               //Frank 20171030 (Steven) add Floating Shuttle調整Offset
        {
            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dShuttleHigh, (double)InputLimit.dShuttleLow);
        }
        else
        {
            if((pnlIndexOffset->Caption==SpecialOffSetName[tOfsIndex1] ||
                pnlIndexOffset->Caption==SpecialOffSetName[tOfsIndex2]) &&
               CUSTOMER_CODE==CC_KYEC_LEE &&                                       //Ifor 20190919 : add KYEC 要求Index Pickup Offset -0.5~0.5 mm
               Buffer->Tag==99)
            {
                fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)10, (double)-0.5);        //Steven 20141120 : Modify
            }
            else
            {
                if(IniConfig.bSPILFunction)                                        //JerryYang 20220923 : 矽品蘇州要求offset limit要By區域設定
                {
                    if(palOffsetParts->Caption==CapStr[OfsLoader])
                    {
                        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dLoaderOffsetZHigh, (double)InputLimit.dLoaderOffsetZLow);
                    }
                    else if(palOffsetParts->Caption==CapStr[OfsHP1] || palOffsetParts->Caption==CapStr[OfsHP2])
                    {
                        if(Buffer->Name=="edPickUp")
                        {
                            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dHPOffsetZHigh, (double)InputLimit.dHPOffsetZLow);
                        }
                        else
                        {
                            fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dHPOffsetZRelHigh, (double)InputLimit.dHPOffsetZRelLow);
                        }
                    }
                    else if(palOffsetParts->Caption==CapStr[OfsInSh1]    || palOffsetParts->Caption==CapStr[OfsInSh2]    ||
                            palOffsetParts->Caption==CapStr[OfsOutSh1LB] || palOffsetParts->Caption==CapStr[OfsOutSh1RB] ||
                            palOffsetParts->Caption==CapStr[OfsOutSh1RB] || palOffsetParts->Caption==CapStr[OfsOutSh2LB] ||
                            palOffsetParts->Caption==CapStr[OfsOutSh2RA] || palOffsetParts->Caption==CapStr[OfsOutSh2RB])
                    {
                        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dInShtOffsetZHigh, (double)InputLimit.dInShtOffsetZLow);
                    }
                    else if(palOffsetParts->Caption==CapStr[OfsOutSh1]   || palOffsetParts->Caption==CapStr[OfsOutSh2]   ||
                            palOffsetParts->Caption==CapStr[OfsOutSh1LB] || palOffsetParts->Caption==CapStr[OfsOutSh1RB] ||
                            palOffsetParts->Caption==CapStr[OfsOutSh1RB] || palOffsetParts->Caption==CapStr[OfsOutSh2LB] ||
                            palOffsetParts->Caption==CapStr[OfsOutSh2RA] || palOffsetParts->Caption==CapStr[OfsOutSh2RB])
                    {
                        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dOutShtOffsetZHigh, (double)InputLimit.dOutShtOffsetZLow);
                    }
                    else if(palOffsetParts->Caption==CapStr[OfsAuto1] || palOffsetParts->Caption==CapStr[OfsAuto2] || palOffsetParts->Caption==CapStr[OfsAuto3] ||
                            palOffsetParts->Caption==CapStr[OfsAuto4] || palOffsetParts->Caption==CapStr[OfsAuto5] || palOffsetParts->Caption==CapStr[OfsAuto6] ||
                            palOffsetParts->Caption==CapStr[OfsFix1]  || palOffsetParts->Caption==CapStr[OfsFix2]  || palOffsetParts->Caption==CapStr[OfsFix3]  ||
                            palOffsetParts->Caption==CapStr[OfsFix4]  || palOffsetParts->Caption==CapStr[OfsFix5]  || palOffsetParts->Caption==CapStr[OfsFix6] )
                    {
                        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.dUnloadOffsetZHigh, (double)InputLimit.dUnloadOffsetZLow);
                    }
                    else
                    {
                        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetZHigh, (double)InputLimit.iOffsetZLow);
                    }
                }
                else
                {
                    if(Buffer==edRelease || Buffer==EdtRelsA || Buffer==EdtRelsB || Buffer==EdtRelsC ||
                       Buffer==EdtRelsD  || Buffer==EdtRelsE || Buffer==EdtRelsF || Buffer==EdtRelsG || Buffer==EdtRelsH)
                        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)iZLimitHigh, (double)iZLimitLow);
                    else
                        fQwertyKey->ShowQwertyKey(Sender, N_DOUBLE, 2, true, (double)InputLimit.iOffsetZHigh, (double)InputLimit.iOffsetZLow);       //Steven 20141120 : Modify
                }
            }
        }
    }

    if(pnlIndexOffset->Caption==SpecialOffSetName[tOfsIndex1] ||
       pnlIndexOffset->Caption==SpecialOffSetName[tOfsIndex2])
    {
        if(pnlIndexOffset->Caption==SpecialOffSetName[tOfsIndex1])
            i=0;
        else
            i=1;

        iPick1      =DeviceForm.IndexArmPick[i]+atof(IndexArmOffSet1->Text.c_str())*100;
        iRelease1   =DeviceForm.IndexPlace[i]+atof(IndexArmOffSet2->Text.c_str())*100;
        if(iRelease1<iPick1)
        {
            fBuf=iPick1-DeviceForm.IndexPlace[i];
            IndexArmOffSet2->Text=fBuf/100.0;
            lblShowMessage->Caption="Relase高度低於Pick高度 ; Relase high below the Pick high ";
        }
    }
}

// golden cOffSet.cpp:3100-3205 -- TimerSetupTeachTimer
// `TObject *Sender` dropped, never read (D-1). golden 本體除了第一行的
// guard 之外，其餘 103 行整段是 `//` 註解掉的死碼（Setup Teach 面板顯示
// 邏輯，作者自己停用）。忠實保留原字（含註解），同
// LoadASECLOffsetIndexValues 保留 golden 註解掉的 iGearRatio 三行的既有
// 先例 -- 這不是本移植發明的降級，golden 現在就長這樣。
// ⚠ offline：IniConfig.bA30SetupTeachFunction 預設 false（Config.cpp 零
// 初始化），所以 offline 上這支永遠在第一行 return，是不折不扣的 no-op。
void TfOffSet::TimerSetupTeachTimer()                                              //JerryYang 20180921 Setup Teach功能
{
    if(IniConfig.bA30SetupTeachFunction==false)                                    //JimmyChiu 20211020 : Auto alignment mode
        return;

//    TPanel *palInArm[]={palLoader,          //0       //JerryYang 20230523 : 舊版無效, Mark掉
//                        palHP1,             //1
//                        palHP2,             //2
//                        palSht1,            //3
//                        palSht2             //4
//                       };
//    TPanel *palOutArm[]={palOutSht1,        //0
//                         palOutSht2,        //1
//                         palAuto1,          //2
//                         palAuto2,          //3
//                         palAuto3,          //4
//                         palFix1,           //5
//                         palFix2,           //6
//                         palFix3            //7
//                        };
//
//    if(LastSet.iTemperature==Tempture_Hot)  //加熱模式
//    {
//        if(HotPlateForm.iPlateSelect==1)
//        {
//            bInArmSetupTeach[InOfsHP1]=false;
//            bInArmSetupTeach[InOfsHP2]=true;
//        }
//        else if(HotPlateForm.iPlateSelect==2)
//        {
//            bInArmSetupTeach[InOfsHP1]=true;
//            bInArmSetupTeach[InOfsHP2]=false;
//        }
//        else
//        {
//            bInArmSetupTeach[InOfsHP1]=false;
//            bInArmSetupTeach[InOfsHP2]=false;
//        }
//    }
//    else
//    {
//        bInArmSetupTeach[InOfsHP1]=true;   //常態不檢查
//        bInArmSetupTeach[InOfsHP2]=true;
//    }
//
//    if(TestIF_File.iShuttleMode==0)
//    {
//        bInArmSetupTeach[InOfsInSh1]=false;
//        bOutArmSetupTeach[OutOfsOutSh1]=false;
//
//        bInArmSetupTeach[InOfsInSh2]=false;
//        bOutArmSetupTeach[OutOfsOutSh2]=false;
//    }
//    else if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) //Shuttle 1
//    {
//        bInArmSetupTeach[InOfsInSh1]=false;
//        bOutArmSetupTeach[OutOfsOutSh1]=false;
//
//        bInArmSetupTeach[InOfsInSh2]=true;
//        bOutArmSetupTeach[OutOfsOutSh2]=true;
//    }
//    else if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) //Shuttle 2
//    {
//        bInArmSetupTeach[InOfsInSh1]=true;
//        bOutArmSetupTeach[OutOfsOutSh1]=true;
//
//        bInArmSetupTeach[InOfsInSh2]=false;
//        bOutArmSetupTeach[OutOfsOutSh2]=false;
//    }
//
//    for(i=0; i<5; i++)
//    {
//        if(iInArmPickPlaceCnt[i]>=5 || bInArmSetupTeach[i]==true)  //JerryYang 20191007 10->5
//        {
//            palInArm[i]->Caption=asInArm[i]+"_Finish";
//            palInArm[i]->Color=clGreen;
//        }
//        else
//        {
//            palInArm[i]->Caption=asInArm[i];
//            palInArm[i]->Color=clGray;
//        }
//    }
//
//    for(i=0; i<8; i++)
//    {
//        if(iOutArmPickPlaceCnt[i]>=5 || bOutArmSetupTeach[i]==true)  //JerryYang 20191007 10->5
//        {
//            palOutArm[i]->Caption=asOutArm[i]+"_Finish";
//            palOutArm[i]->Color=clGreen;
//        }
//        else
//        {
//            palOutArm[i]->Caption=asOutArm[i];
//            palOutArm[i]->Color=clGray;
//        }
//    }

//    if(LastSet.bNeedSetupTeach==true) //JerryYang 20230523 : 舊版無效, Mark掉
//    {
//        if(CheckSetupFinish()==true)
//        {
//            LastSet.bNeedSetupTeach=false;
//        }
//    }
}

// =============================================================================
//  GATED (Wave B) -- 宣告在 forms/fOffSet.h、本檔**刻意不定義**：
//    sbInArmZCalibrationMouseDown / sbOutArmZCalibrationMouseDown
//  兩支都會呼叫 fMain->Start(...) 且先跑 AutoTeachLoadTrayZ(...)（golden
//  :3551-3587 / :3589-3619），**啟動機台**，同 (O-5) 性質。理由見
//  forms/fOffSet.h 的 GATE (O-10)/(O-11)。
// =============================================================================

// =============================================================================
//  GATED -- 以下 12 支只在 forms/fOffSet.h 宣告，本檔**刻意不定義**，
//  由 linker 當互鎖（forms/fMotorTest.h 先例）：任何未來波次一旦接線呼叫
//  它們，會在 link 階段炸出 undefined reference，而不是靜默跑一個空殼。
//    CheckBox1Click / btnOffsetListClick        -- (O-1)(O-2) 呼叫未翻的大支
//    FormDestroy / GetOffsetPath                -- (O-3)(O-4) 相依在 ht9045_sm
//    sb_AutoOffset{Up,Down,Right,Left}Click     -- (O-5) 會寫檔並啟動機台
//    ClearIndexOffset                           -- (O-6) 寫教導／偏移值
//    edOffsetContactForceChange                 -- (O-7) fProductionInfo 缺 API
//    UseOutArmSetupTeach                        -- (O-8) 與既有 in-arm stub 不對稱
//    btnSortAuto1Click                          -- (O-9) Tag 值在本樹永遠讀 0
//  理由逐支見 forms/fOffSet.h 的 GATE REGISTER。
// =============================================================================

// ===========================================================================
//  AI(W906-ARM1) 20260927: golden TfOffSet::UseInArmSetupTeach／UseOutArmSetupTeach（cOffSet.cpp:2955-2974、:2976-3005）逐行照翻：
//  Setup Teach：IniConfig.bA30SetupTeachFunction 開、LastSet.bNeedSetupTeach 為 true 時，入／出料臂在每個點位第一次到的時候回 true（呼叫端 fMain->Pause 讓
//  操作員教點位），再用 bInArmStop[]／bOutArmStop[]（cmydef.cpp:5124／:5127）latch 住；SECS（uHGemHT9045.cpp:1501-1512）與 START（WebStart.cpp:1826-1846）照 golden 清。
//  以前：入料臂那支是回 false 的樁，出料臂那支只有宣告（fOffSet.h GATE O-8）。O-8 閘著的唯一理由是「兩支要一起翻、append-only 不能改既有那行」，不是缺相依。
//  全是 C++ 狀態（IniConfig／LastSet／CosFunction／全域陣列），不是畫面狀態。
// ===========================================================================
#include "CosFunction.h"   // CosFunction.bManualSteplAutoTeach（golden :2957）
// ---- golden cOffSet.cpp:2955-2974 ----
bool TfOffSet::UseInArmSetupTeach(int iArea)                         //JerryYang 20180921 Setup Teach功能
{
    if(CosFunction.bManualSteplAutoTeach &&
       IniConfig.bA56EnableAutoTeachFunciton)                                   //JimmyChiu 20211020 : Auto alignment mode
        return false;

    if(IniConfig.bA30SetupTeachFunction==false)
        return false;
    if(iArea>InOfsInSh2)
        return false;
    if(LastSet.bNeedSetupTeach)
    {
        if(bInArmStop[iArea]==false)
        {
            bInArmStop[iArea]=true;
            return true;
        }
    }
    return false;
}
// ---- golden cOffSet.cpp:2976-3005 ----
bool TfOffSet::UseOutArmSetupTeach(int iArea)                        //JerryYang 20180921 Setup Teach功能
{
    if(IniConfig.bA30SetupTeachFunction==false)
        return false;

    if(AUTO_EMPTY_COLOR==3)
    {
        if(iArea==OutOfsAuto6 || iArea>OutOfsFix6)
            return false;
    }
    else if(AUTO_EMPTY_COLOR>=4)
    {
        if(iArea>OutOfsFix6)
            return false;
    }
    else if(iArea>OutOfsFix3)
    {
        return false;
    }

    if(LastSet.bNeedSetupTeach)
    {
        if(bOutArmStop[iArea]==false)
        {
            bOutArmStop[iArea]=true;
            return true;
        }
    }
    return false;
}
