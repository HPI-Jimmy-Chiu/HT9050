// =============================================================================
//  aRotateKIT.cpp  --  shared RotateKit helpers (teach geometry, motor
//                      resolution, nozzle selection, rotate-station sensors)
//
//  Faithful translation of golden RotateKit/aRotateKIT.cpp (208 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-ROTKIT) 20260807
//  Translation wave: W906-ROTKIT (pure-translation completion campaign).  Small
//  shared leaf of the RotateKit family; the two big siblings
//  aRotateKIT_In.cpp and aRotateKIT_Out.cpp depend on this unit's header and
//  landed in the SAME wave (AI(W906-PT-W1-integrate) 20260807 corrected this
//  sentence -- it originally said they were "NOT part of this wave", which is
//  what made GATE (2) below look justified when it no longer was).
//
//  ROLE: owns the rotate-kit teach geometry (Home / Auto start + pitch), the
//  2x2-kit nozzle selector (i2x2Suck / i2x2Suck_Out), the rotate-motor
//  degrees-to-pulse resolution table (incl. RogerYang 20260113 backlash
//  compensation), the rotate-station stack/misplace sensor predicate, and the
//  in-arm "does this pick need the rotate station" predicate.  NO state machine,
//  NO switch(Task) cursor -- five free functions, ten file-scope globals.
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim -- all 5 golden functions, whole file):
//     RotateKIT_FormSet          golden :39-55
//     SetMotorResolution         golden :60-114
//     InitSuckState              golden :116-135
//     CheckRotateSensor          golden :139-186
//     CheckNeedRotateWithoutHotIC golden :188-207
//   Plus all 10 golden file-scope globals (golden :27-37).
//   SATISFIED-BY-SUBSTRATE (already real in this tree, nothing shimmed here):
//     MOT[]/TMyMotor::GetRotatorBacklash (Motor/mymotor.cpp:835),
//     Sen[]/TMySensor::Enable+IsOn (mysensor.cpp:117), TMyKitSuck InArmSuck
//     ::HasRealIC/::HasType (aHotPlateSubstrate.cpp:521/:384), tRotate
//     (aHotPlateSubstrate.cpp:885), Prod/TestIF/TestIF_File/TrayForm (cprod.h),
//     LastSet (LastSet.cpp), iInRotateFinish (acatchtray.cpp:138), and the
//     cmydef.cpp constants MInRotateKit=41 / MOutRotateKit=42 /
//     SnRotateRowIn1..2=437,438 / SnRotateRowOut1..2=439,440 / HAS_HOT_IC=4 /
//     Tempture_Hot=1 / USE_ROTATE_KIT / iRotate_Type.
//
//  GATE REGISTER -- exactly TWO gates, both re-cited at their call site below.
//  Both were checked against the WHOLE port tree first (grep over every .cpp/.h
//  outside build*/), not guessed:
//   (1) ArmCanSuck4IC(int,bool)  -- golden csystem.cpp:723; used by InitSuckState
//       (golden :118, twice).  It IS DECLARED in this tree at ported
//       csystem.h:162, but there is NO COMPILED BODY ANYWHERE: golden
//       csystem.cpp:723 has not been translated, and grep for `ArmCanSuck4IC`
//       over every ported .cpp finds only two COMMENT lines
//       (aHotPlateSubstrate.cpp:1008-1013).  An active call therefore link-fails
//       with `undefined reference to ArmCanSuck4IC(int, bool)`.  This is the same
//       "declared in an already-translated header, no body anywhere" class of gap
//       aTester_Front.cpp records in its own INTEGRATE-STEP FIXES list; the fix
//       used there (translate the golden body into a shim TU) is not available to
//       this wave, which may only touch its own two files.
//       Handled with the tree's #if 0/#else macro-pair idiom (aTester_Front.cpp
//       :150-156): golden's call stays VERBATIM in the #if 0 arm, and the ACTIVE
//       arm evaluates to `true`.
//       WHY `true` IS THE FAITHFUL DEFAULT: golden's ArmCanSuck4IC is a ladder of
//       five "restriction" early-outs over a terminal `return true;`
//       (csystem.cpp:725-766).  For the only three test modes that can reach
//       golden :118 at all (_8Site2X4 / _16Site4X4 / _16Site2X8) none of the
//       DualSite / DualSite2x1 / SingleSite / QualSite2X2 early-outs applies, so
//       `true` is the dominant outcome and `false` is the exceptional large-IC /
//       wide-site-pitch path.  With `true` the whole golden condition at :118 is
//       false and the ladder falls through to golden's own final `else`
//       (i2x2Suck=2 / i2x2Suck_Out=2) -- i.e. the branch is not bypassed, it is
//       resolved to golden's default arm.  This also matches the precedent this
//       tree already set for the SAME symbol: ported aHotPlateSubstrate.cpp:1008
//       -1013 reasons RowCanDualSite() to `false` on exactly the grounds that the
//       ArmCanSuck4IC(0)==false early-out does not fire offline.
//       BEHAVIOUR DELTA, STATED PLAINLY: on a real 2x4/4x4/2x8 machine running a
//       large IC (or a wide site pitch) golden would set i2x2Suck=3 /
//       i2x2Suck_Out=3 and this build sets 2/2.  Retire this gate the moment
//       csystem.cpp:723 lands -- delete the macro, restore the #if 0 arm.
//   (2) RETIRED AT INTEGRATION -- AI(W906-PT-W1-integrate) 20260807.  This entry
//       used to gate InitialInArmRotateKIT() (golden :201) on the ground that
//       "the identifier does not occur ANYWHERE in the port tree".  That was
//       false at the moment it was written: aRotateKIT_In.cpp landed in the SAME
//       wave, defines the function at :425 and the cursor iInArmRotateKit at
//       :406, this unit's own header declares it at aRotateKIT.h:61, and both
//       .cpp files go into ht9045_sm.  Per ungate-what-LINKS the call is now
//       ACTIVE and the gate is gone; the register keeps this hole so the banner
//       and the wave report still line up.
//       WHAT THE GATE COST WHILE IT STOOD: the side effect iInArmRotateKit=1,
//       the rewind of the in-arm rotate cursor.  DoInArmRotateKIT_Cylinder never
//       performs that rewind itself -- its terminal steps are Task=4300 /
//       Task=4000 (aRotateKIT_In.cpp:1065/1069) -- so without the call the
//       rotate SM would restart mid-sequence the next time it ran.
//       LESSON, KEPT DELIBERATELY: a gate rationale that says "does not occur
//       anywhere" must name the grep AND the date it was run.  Two sibling
//       units landing in one wave is exactly when such a claim goes stale.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init) dropped
//  (golden :2 and :25).  No __fastcall / __property / __published in this file --
//  golden has none either.  No AnsiString use in this unit.  SOFT_SIMULTE is NOT
//  defined: the #ifdef SOFT_SIMULTE arm of CheckRotateSensor (golden :141-142) is
//  reproduced verbatim but inert, and the #else real-machine arm compiles.
//  Numeric semantics kept EXACT (no int/float rewrites); the golden
//  `A && B || C && D` precedence at :118 is kept UNPARENTHESISED exactly as
//  golden wrote it (it means (A&&B)||(C&&D); adding the clarifying parentheses
//  would be an edit, so it is not made -- expect a -Wparentheses note under
//  -Wall, which ht9045_sm does not escalate to an error).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "aRotateKIT.h" / "aRotateKIT_In.h" / "aRotateKIT_Out.h" /
//     "aArmHeader.h" / "cprod.h" / "csystem.h" / "mymotor.h" / "MyKitSuck.h" /
//     "mycylin.h" / "mysensor.h" / "main.h" / "note.h" / "fRotate.h" /
//     "uhome.h" / "cinitial.h" / "cMyDB.h" / "cmydef.h" / "SCK_ART.h" / "fAOI.h"
//   ; #pragma package(smart_init)
//
//   NOT re-included here, and why:
//     aRotateKIT_In.h / aRotateKIT_Out.h -- nothing in THIS file's five bodies
//       needs them; the only In/Out symbol reached is InitialInArmRotateKIT,
//       which this unit's own header already declares at :61.  (Both sibling
//       units ARE translated -- they landed in the same wave; the earlier
//       "unwritten" wording was corrected by AI(W906-PT-W1-integrate) 20260807.)
//     mycylin.h / main.h / note.h / uhome.h / cinitial.h / cMyDB.h / SCK_ART.h /
//       fAOI.h -- golden's god-stack; not one symbol from any of them is
//       referenced by the translated bodies.  fAOI is touched only inside
//       golden's own commented-out block (:85-89), preserved below as comments.
//     fRotate.h -- golden's home for `tRotate`; in THIS tree tRotate is the
//       tRotateShim at aHotPlateSubstrate.h:824-832 (forms/fRotate.h holds the
//       TFrmRotate form stand-in instead, which this file does not use).
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)
#include "aRotateKIT.h"             // this unit's own contract (the 5 functions + 10 globals defined below)
#include "aArmHeader.h"             // golden's arm god-header -- guard-only shim in this tree (aArmHeader.h:8-23)
#include "cprod.h"                  // Prod (iIn_iRotateA / iOut_iRotateA), TestIF + TestIF_File (iTestMode, UseRotateForHT7000HPKit), TrayForm (iRotateKIT_InputType)
#include "csystem.h"                // ArmCanSuck4IC(int,bool) DECLARATION only -- see GATE (1)
#include "Motor/mymotor.h"          // MOT[] (TTrayMotor) + TMyMotor::GetRotatorBacklash (:312, body mymotor.cpp:835)
#include "mysensor.h"               // Sen[] (TMySensor::Enable / TMySensor::IsOn)
#include "aHotPlateSubstrate.h"     // golden MyKitSuck.h -> TMyKitSuck InArmSuck (HasRealIC / HasType); golden fRotate.h -> tRotate (tRotateShim.ActiveRotate); iInRotateFinish
#include "cmydef.h"                 // MInRotateKit / MOutRotateKit, SnRotateRowIn1|2, SnRotateRowOut1|2, HAS_HOT_IC, Tempture_Hot, USE_ROTATE_KIT, iRotate_Type
#include "MachineType.h"            // eRotateType (e4MotRotate / e8MotRotate / e2MotRotate2Dut / eInOutArm1Motor) + test-mode enum (_8Site2X4 / _16Site4X4 / _16Site2X8 / QualSite2X2 / DualSite)
#include "canary_support.h"         // LastSet (LAST_GENERAL_SET::iTemperature) -- golden reached it via main.h
//---------------------------------------------------------------------------
int iRotateKIT_Start_X_H = 7800;
int iRotateKIT_Start_Y_H = 7975;
int iRotateKIT_Pitch_X_H = 8000;
int iRotateKIT_Pitch_Y_H = 6000;
int iRotateKIT_Start_X_A = 2800;
int iRotateKIT_Start_Y_A = 7000;
int iRotateKIT_Pitch_X_A = 8000;
int iRotateKIT_Pitch_Y_A = 6000;

int i2x2Suck                =0;     //判斷2x2kit時用第幾支吸嘴IC <2500 =2 (Suck[0][2]) ;IC >2500 =3  (suck[0][3]);
int i2x2Suck_Out            =0;
//---------------------------------------------------------------------------
void RotateKIT_FormSet(bool bInput, int iStartX, int iStartY, int iPitchX, int iPitchY)
{
    if(bInput==true)
    {
        iRotateKIT_Start_X_H = iStartX;
        iRotateKIT_Start_Y_H = iStartY;
        iRotateKIT_Pitch_X_H = iPitchX;
        iRotateKIT_Pitch_Y_H = iPitchY;
    }
    else
    {
        iRotateKIT_Start_X_A = iStartX;
        iRotateKIT_Start_Y_A = iStartY;
        iRotateKIT_Pitch_X_A = iPitchX;
        iRotateKIT_Pitch_Y_A = iPitchY;
    }
}
//---------------------------------------------------------------------------
//kevin 20131003 傳送馬達角度pluse
//const int MotorResolution=8000;       //日本Tamagawa 步進馬達
//---------------------------------------------------------------------------
int SetMotorResolution(int &Ang45, int &Ang90, bool bInRotate)
{
    int buffer=0;
    int iBacklash=0;
//    int iAOIInAngle=0;
//    int iAOIOutAngle=0;

    if(iRotate_Type==e4MotRotate ||
       iRotate_Type==e8MotRotate ||
       iRotate_Type==e2MotRotate2Dut)
    {                                                                           //Steven 20170427 (wei) : 六軸步進版就是3200轉一圈
        Ang45=400;
        Ang90=800;
    }
    else if(iRotate_Type==eInOutArm1Motor)                                      //Frank 20250812 : modify InOutArm1Motor
    {
        Ang45=625;                                                              // 5000轉一圈
        Ang90=1250;
    }
    else
    {
        Ang45=1000;                                                             //MotorResolution/8; //kevin 20130710 旋轉45 度
        Ang90=2000;                                                             //MotorResolution/4;
    }

//    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)
//    {
//        iAOIInAngle=FrmAOI->ttbInsp->iRotateKitAngOffset_In;
//        iAOIOutAngle=FrmAOI->ttbInsp->iRotateKitAngOffset_Out;
//    }

    if(bInRotate)
    {   //inrotate 0 度 水平位置
        buffer=Prod.iIn_iRotateA;
        iBacklash=MOT[MInRotateKit].GetRotatorBacklash(buffer, true);           //RogerYang 20260113 : Rotator新增背隙補償
        buffer+=iBacklash;
    }
    else
    {   //outrotate shuttle 一樣方向位置 0 度 水平位置
//        buffer=Ang90*tRotate.RotationTime+Prod.iOut_iRotateA ;
        //jou 2014-03-06 make code 看不懂為什麼要先轉
//        if(DeviceForm_File.XDimension != DeviceForm_File.YDimension)            //20140307 wei 如果長方形的要先轉  //JerryYang 20160603 MARK掉,修正旋轉角度錯誤
//        {
//            buffer=Ang90*tRotate.RotationTime+Prod.iOut_iRotateA ;
//        }
//        else
//        {
        buffer=Prod.iOut_iRotateA;
        iBacklash=MOT[MOutRotateKit].GetRotatorBacklash(buffer, false);         //RogerYang 20260113 : Rotator新增背隙補償
        buffer+=iBacklash;
//        }
    }

    return buffer;
}
//---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Gate helper -- see GATE (1) in the file banner for the full rationale.
//  golden csystem.cpp:723 ArmCanSuck4IC(int Direct, bool flag=true) is DECLARED
//  at ported csystem.h:162 but has NO compiled body anywhere in this tree, so a
//  live call is an undefined reference.  The golden call is kept verbatim in the
//  #if 0 arm; the ACTIVE arm resolves to golden's own dominant outcome (`true`,
//  the terminal return of csystem.cpp:725-766 for the _8Site2X4 / _16Site4X4 /
//  _16Site2X8 modes that are the only ones able to reach the test below), which
//  steers InitSuckState into golden's final `else` (i2x2Suck=2, i2x2Suck_Out=2).
//  Retire the moment golden csystem.cpp:723 is translated.
// ---------------------------------------------------------------------------
#if 0 // TODO(W7): golden csystem.cpp:723 ArmCanSuck4IC -- declared csystem.h:162, no compiled body in this tree
#define W906RK_ARMCANSUCK4IC(Direct)   (ArmCanSuck4IC(Direct))
#else
#define W906RK_ARMCANSUCK4IC(Direct)   (true)
#endif
//AI(W906-ROTKIT) 20260807: GATE (1) -- golden :118 calls ArmCanSuck4IC(0) twice;
//  no compiled body exists in this tree (declared csystem.h:162 only), so the two
//  calls go through the W906RK_ARMCANSUCK4IC macro above, which resolves to golden's
//  dominant `true`.  Rest of the ladder is golden verbatim.  See banner GATE (1).
void InitSuckState()
{
    if((TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4) && W906RK_ARMCANSUCK4IC(0)==false || TestIF.iTestMode==_16Site2X8 && W906RK_ARMCANSUCK4IC(0)==false) //Sam 20190226 : 16Site4X4 // 2x4 && IC 大於2500mm
    {
        i2x2Suck=3;
        //i2x2Suck_Out=2;
        i2x2Suck_Out=3;                                                         //Sam 20211224 : 修正為使用 1 4 吸嘴
    }
    else if(TestIF.iTestMode==QualSite2X2 ||
            TestIF.iTestMode==DualSite)                                         //Sam 20250428 : 修正 1X2 OutRotate 旋轉異常
    {
        i2x2Suck=2;
        i2x2Suck_Out=1;                                                         //SetInOutArmParameter_2X2_4 已經將 OutArm 的資料搬到 1 2 位置上  //Sam 20250217 : 修正 2X2 OutRotate 旋轉異常
    }
    else// if(tRotate.DutNum==tDutType_4)
    {
        i2x2Suck=2;
        i2x2Suck_Out=2;
    }
}
//---------------------------------------------------------------------------
//Rotate 偵測是否疊料或放不好   kevin 20130524
//---------------------------------------------------------------------------
bool CheckRotateSensor(bool bRotateIn)
{
    #ifdef SOFT_SIMULTE
        return true;
    #else
    if(bRotateIn)// In rotate
    {
        //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
        if(Sen[SnRotateRowIn1].Enable==true && Sen[SnRotateRowIn2].Enable==false)
        {
            if(Sen[SnRotateRowIn1].IsOn())
                return true;
        }
        else
        {
            if(Sen[SnRotateRowIn1].Enable==false || Sen[SnRotateRowIn2].Enable==false)
            {
                return true;
            }
            else if(Sen[SnRotateRowIn1].IsOn() && Sen[SnRotateRowIn2].IsOn())
            {
                return true;
            }
        }
    }
    else
    {   //out rotate
        //Ifor 20180110 (Steven) : add 僅有一個Rotate sensor
        if(Sen[SnRotateRowOut1].Enable==true && Sen[SnRotateRowOut2].Enable==false)
        {
            if(Sen[SnRotateRowOut1].IsOn())
                return true;
        }
        else
        {
            if(Sen[SnRotateRowOut1].Enable==false || Sen[SnRotateRowOut2].Enable==false)
            {
                return true;
            }
            else if(Sen[SnRotateRowOut1].IsOn() && Sen[SnRotateRowOut2].IsOn())
            {
                return true;
            }
        }
    }
    return false;
    #endif
}
//---------------------------------------------------------------------------
bool CheckNeedRotateWithoutHotIC()  //Steven 20160809 : 改成Function
{
    bool bHasHotICNeedRotate=false;
    if(TestIF.iTestMode==DualSite && LastSet.iTemperature==Tempture_Hot && TestIF_File.UseRotateForHT7000HPKit)   //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit
        bHasHotICNeedRotate=true; //加熱完的要放 Rotate

    bool Result=false;
    if(InArmSuck.HasRealIC()==true &&           //Steven 20161102 : 修正從加熱盤吸取全部都是HAS_NULL_IC時會Hang up
       InArmSuck.HasType(HAS_HOT_IC)==bHasHotICNeedRotate &&  //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit //ChungHung 20131219 HAS_HOT_IC 不需跑ROTATE
       USE_ROTATE_KIT==1 && tRotate.ActiveRotate && TrayForm.iRotateKIT_InputType!=0 && (iInRotateFinish==0 || iInRotateFinish==1)) //kevin 20130524 發生alarm inarm到等待點
    {
        if(iInRotateFinish==0) //尚未到旋轉站。
        {
//AI(W906-PT-W1-integrate) 20260807: GATE (2) RETIRED -- ungate-what-LINKS.  The gate's
//  stated reason ("the identifier does not exist anywhere in this tree") was already
//  false when it was written: aRotateKIT_In.cpp landed in the SAME wave and defines
//  InitialInArmRotateKIT at :425, aRotateKIT.h:61 declares it, and CMakeLists.txt
//  compiles aRotateKIT_In.cpp into the same archive (ht9045_sm).  The call links, so it
//  is active, restoring golden :201's side effect iInArmRotateKit=1 -- the rewind of the
//  in-arm rotate cursor, which DoInArmRotateKIT_Cylinder never performs itself (its
//  terminal steps are Task=4300 / Task=4000, aRotateKIT_In.cpp:1065/1069).
            InitialInArmRotateKIT();
//            iInRotateFinish=1;
        }
        Result=true;
    }
    return Result;
}
//---------------------------------------------------------------------------
