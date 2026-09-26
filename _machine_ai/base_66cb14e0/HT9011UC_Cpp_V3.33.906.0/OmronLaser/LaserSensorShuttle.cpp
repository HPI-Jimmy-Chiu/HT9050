// =============================================================================
//  OmronLaser/LaserSensorShuttle.cpp  --  In/Out shuttle laser floating-check
//                                         state machines.
//
//  Faithful translation of golden OmronLaser/LaserSensorShuttle.cpp (1535
//  lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W3-laser) 20260807
//  Translation wave: W906-PT-W3 ("laser" group), file 3 of 3.
//
//  ROLE: ShtLaserInit/CheckShtFloating own the ORIGINAL per-machine laser
//  scheme (fLaserSensor's own COM1/COM2 A/B-row sensors, driven by
//  fLaserSensor->btGetValueSht{1,2}_{1,2}Row); UseInArmCheckShtFloating/
//  UseOutArmCheckShtFloating (called from LaserSensor.cpp's own
//  DoGetLaserValue/DoGetOutLaserValue) are the NEWER "measure with the
//  In/OutArm's OWN mounted laser head, one shuttle cell at a time by driving
//  the arm to sit over it" scheme added 20260525 ("Eastsun laser 整合").
//  Both schemes share the SThreadPara.iCHKStep-indexed tray geometry and the
//  SetShtTrayColor/SetShtTrayNumber cell-paint helpers at the top of the file.
//
//  DO NOT TREAT UseInArmCheckShtFloating AND UseOutArmCheckShtFloating AS
//  MIRROR IMAGES -- per this wave's own brief, they were checked line-by-line
//  and genuinely differ in one place: In case 100 (golden :1014-1071), flag1
//  is set ONLY by `LaserShtTimeOut.Off()` -- the `if(asLaserValue[0]!="")`
//  block that follows never touches flag1. In the Out sibling's case 100
//  (golden :1312-1370), the analogous `if(asLaserValue[1]!="")` block ENDS
//  with an unconditional `flag1=true;` (golden :1367) that the In version
//  does not have anywhere in its block. Both are reproduced EXACTLY as
//  golden wrote them below -- this is not a copy-paste error to "fix", it is
//  the one deliberate behavioural asymmetry between the two functions.
//
//  WAVE SCOPE -- ACTIVE (all 9 golden functions + 1 forward-declared-not-
//  redefined function, whole bodies, verbatim numeric/string semantics):
//    SetShtTrayColor              golden :40,42-96  (2 declarations: forward
//                                  default-arg :40, definition :42)
//    SetShtGoldenTrayColor        golden :98-101
//    SetShtTrayNumber             golden :41,103-228 (forward :41, def :103)
//    SetShtGoldenTrayNumber       golden :230-233
//    ShtLaserInit                 golden :235-453
//    CheckShtFloating             golden :455-739
//    MoveInArmXYToSHLaserPos      golden :839-888
//    MoveOutArmXYToSHLaserPos     golden :890-939
//    UseInArmCheckShtFloating     golden :942-1236
//    UseOutArmCheckShtFloating    golden :1240-1533
//   Plus 11 golden file-scope globals (golden :25-37).
//  NOT RE-DEFINED, SATISFIED BY AN EXISTING REAL BODY IN A DIFFERENT PORTED
//  FILE (see GATE REGISTER item (1) for why this is deliberate, not an
//  omission): `int GetRowCol(int&,int&)` (golden :742-834). This tree's own
//  atester.cpp:191 already carries a byte-for-byte identical branch ladder
//  (verified this wave, side by side) -- see GATE REGISTER (1).
//  SATISFIED-BY-SUBSTRATE (already real elsewhere in this tree):
//    MOT[]/TMyMotor::MotorMove+IsCanMove (Motor/mymotor.h),
//    InArmContinuousMove_9045/OutArmContinuousMove_9045 (Motor/mymotor.cpp:
//    1356 / sibling, sim stub `return false`), GetInArmPitch_9045
//    (ainarm9045.cpp:112), GetOutArmPitch_9045 (aoutarm9045.cpp:206),
//    TMyKitSuck + ptrInSHT/ptrOutSHT/FLCarryKit/BLCarryKit/FRCarryKit/
//    BRCarryKit (mykitsuck.h), ShowErrorMessage/ShowMyMessage
//    (canary_support.h), LastSet (LastSet.h via canary_support.h),
//    TestIF_File/Prod/SThreadPara (cprod.h), cmydef.h constants (MInShuttle1/
//    2, MOutShuttle1/2, K_RETRY, K_SKIP, DUMMY, NULL_IC, HAS_NULL_IC,
//    IndexSuckName, iInArmYBase/XBase, iInArmShtXCenterPos/YCenterPos),
//    ZAxisDown (aHotPlateSubstrate.cpp:830), X_PITCH_COUNT/MAX_ARM_Row/
//    MAX_ARM_Col (MachineType.h), fLaserSensor->mt*/bt*Click (TrayCore /
//    TLaserClickButton, OmronLaser/LaserSensor.h),
//    fLaserSensor->SaveShuttleLaserValue/WriteToMemo (OmronLaser/
//    LaserSensor.cpp), asLaserValue[2] (OmronLaser/LaserSensor.cpp).
//
//  GATE REGISTER -- 2 gates:
//   (1) `int GetRowCol(int &iRow, int &iCol)` -- golden DEFINES this function
//       itself at :742-834, with its own comment (golden :741, decoded)
//       "從 a-side atester.cpp 移植 GetRowCol (c-side 無此全域函式)" i.e.
//       "ported from a-side atester.cpp because c-side [this 906 tree] has
//       no such global function". THAT premise is FALSE for the golden tree
//       actually being ported here, verified this wave: golden's OWN
//       atester.cpp:144 already defines `int GetRowCol(int&,int&)` with the
//       IDENTICAL branch ladder (same TestIF_File.iTestMode cases, same
//       iRow/iCol/iSiteMapRTC assignments, checked side-by-side this wave) --
//       so golden itself would hit a duplicate-symbol LINK ERROR if
//       atester.cpp and this file were ever linked into the same golden
//       target (a real BCB6 project can dodge this via separate .obj/.lib
//       boundaries or an internal-linkage `static` golden's own text does not
//       show; either way it is not this port's job to reproduce a link
//       ambiguity). This port's atester.cpp ALREADY carries the same body
//       (translated independently, W6.4, from the same source), so this file
//       does NOT redefine GetRowCol -- it forward-declares it exactly the way
//       golden itself does at :20/:38 (`extern int GetRowCol(int&,int&);`)
//       and links against atester.cpp's real one. BEHAVIOUR DELTA: none --
//       the two bodies are identical, verified line-by-line, not merely
//       assumed from the shared naming.
//   (2) `GetShuttleSize(int&,int&,int&,int&)` (golden call sites :864,:915;
//       forward-declared golden :21) -- golden DEFINES this in cinitial.cpp
//       :15236-15240 (`GetRowCol(YItem,XItem); PitchX=TestIF.dSiteXPitch;
//       PitchY=TestIF.dSiteYPitch;`), which is NOT part of this wave's scope
//       and has genuinely no compiled body anywhere in this port tree today
//       (grepped, whole tree, this wave). Gated #if 0/#else. WHY THE #else
//       DEFAULT IS FAITHFUL, NOT DEGRADED: cinitial.cpp's own 4-line body
//       uses only symbols that ARE already real in this tree (GetRowCol --
//       see gate (1) above -- and TestIF.dSiteXPitch/dSiteYPitch, cprod.h),
//       so the #else arm below reproduces that exact 4-line body inline
//       rather than approximating it. BEHAVIOUR DELTA: none observable --
//       this is the same computation, just performed at the call site
//       instead of through a not-yet-landed free function. Retire the moment
//       cinitial.cpp's own GetShuttleSize lands: delete the #else arm,
//       restore the #if 0 arm (the verbatim golden call).
//   (3) (documented for completeness, not #if 0'd -- see rationale below)
//       `ShowMyMessageBox_YES_NO(AnsiString,AnsiString)` (golden :1185,:1482)
//       -- golden's own modal Yes/No dialog (mymessbox.h) has NO tree-wide
//       compiled body; every existing consumer in this port (atester.cpp:
//       1551-1552, csystem.cpp:2868-2869) instead defines its OWN
//       file-scope `static int W?_ShowMyMessageBox_YES_NO(...){ return 0; }`
//       + `#define ShowMyMessageBox_YES_NO` pair (both commented "offline:
//       NO(0)"), i.e. an ALREADY-ESTABLISHED per-TU idiom, not a single
//       linkable symbol to forward-declare. This file follows the SAME
//       established idiom (own static shim + macro, see just above
//       ShtLaserInit... actually placed near the top of this file, by the
//       includes) rather than #if 0/#else gating a single call site, because
//       the idiom itself already IS the tree's registered answer to "this
//       symbol has no body" for this exact function -- redeclaring it as a
//       fresh #if 0 gate here would just be a third copy of the same
//       decision already made twice. BEHAVIOUR DELTA: identical to the two
//       existing consumers' -- offline, both `ret==2` (golden :1186/:1483)
//       and `ret==K_SKIP` (golden UseIn/OutArmCheckShtFloating :1150/:1447,
//       K_SKIP defined cmydef.h:277) are unreachable (0 matches neither), so
//       the "save + report" / "retry" arms are what runs.
//
//  VCL/Borland conversions: `#pragma hdrstop`/`#pragma package(smart_init)`
//  dropped (golden :2,23). No __fastcall/__property/VCL type anywhere in
//  this file (golden has none either).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "LaserSensorShuttle.h" / "LaserSensor.h" / "MyMotor.h" /
//     "note.h" / "cprod.h" / "cmydef.h" / "MyKitSuck.h" / "ainarm2.h" /
//     "cprod.h" (repeated) / "ainarm9045.h" / "aoutarm9045.h" / "cinitial.h" /
//     "mymessbox.h" / "mycylin.h"
//   ; #pragma package(smart_init)
//
//   NOT re-included here, and why:
//     "note.h"/"mymessbox.h" -- ShowErrorMessage/ShowMyMessage's declared
//       homes; this tree's compiled bodies live behind canary_support.h
//       instead (same substitution LaserSensor.cpp/LaserSensorInArm.cpp make).
//     "ainarm2.h" -- has NO PORT ANYWHERE in this tree (grepped: zero
//       `ainarm2.h` files exist, only a golden-side header of that name).
//       This file's 10 ACTIVE bodies reference zero symbols that would need
//       it (verified this wave); dropped, not gated, because nothing calls
//       through it.
//     "cinitial.h" -- only golden symbol this file would reach through it is
//       GetShuttleSize, itself GATED (see GATE (2) above) rather than
//       resolved via a declaration; cinitial.h is not needed for the gate's
//       inline #else body (which only needs GetRowCol + cprod.h's TestIF).
//     "mycylin.h" -- golden includes it but this file's 10 bodies reference
//       ZERO TMyCylinder symbols (grepped this wave); dropped per the tree's
//       "not one symbol referenced" precedent (aRotateKIT.cpp's own banner).
//     "MyMotor.h" -- golden's spelling of this tree's real path
//       "Motor/mymotor.h" (TriTemp.cpp precedent, same as the other 2 files
//       in this wave).
//     "MyKitSuck.h" -- this tree's real path is "mykitsuck.h" (lower-case;
//       same file, Windows filesystem is case-insensitive, but the tree's
//       own on-disk name is spelled this way -- matches golden's OWN spelling
//       closely enough that no rename comment is needed, unlike the
//       MyMotor.h/HTEditList.h directory-relocation cases).
// =============================================================================
#include "MachineDefine.h"

#include "OmronLaser/LaserSensorShuttle.h"
#include "OmronLaser/LaserSensor.h"
//---------------------------------------------------------------------------
#include "Motor/mymotor.h"           // golden "MyMotor.h" -- see file-head note; MOT[]/InArmContinuousMove_9045/OutArmContinuousMove_9045
#include "cprod.h"                   // TestIF_File, Prod, SThreadPara
#include "cmydef.h"                  // MInShuttle1/2, MOutShuttle1/2, K_RETRY/K_SKIP, DUMMY, NULL_IC, HAS_NULL_IC, IndexSuckName, iInArmYBase/XBase, iInArmShtXCenterPos/YCenterPos
// AI(W906-PT-W3-integrate) 20260808: ainarm9045.h is included HERE, i.e. BEFORE
//   aHotPlateSubstrate.h, and the order is load-bearing -- not cosmetic.
//   aHotPlateSubstrate.h:856-861 redeclares InArmLeftSideHasIC/NoIC WITH default
//   args (`int iRow=2`) behind `#ifndef ainarm9045H`, precisely so that a TU which
//   already has the engine's own header does not get the same default argument
//   twice (C++ forbids repeating it in one scope -- it is a hard error, which is
//   how this surfaced).  ainarm9045.h:133-134 gives those defaults, so it must be
//   seen FIRST for that guard to fire.  Substrate-then-engine (the order this file
//   had while it still included mykitsuck.h, which declares neither) does not
//   compile.  Only GetInArmPitch_9045 (:47) is actually wanted from here; see :940.
#include "ainarm9045.h"               // GetInArmPitch_9045 (:47) -- MUST precede aHotPlateSubstrate.h, see above
// AI(W906-PT-W3-integrate) 20260808: was `#include "mykitsuck.h"`.  SWAPPED to the
//   substrate header, and this is a CORRECTNESS fix, not a style choice.
//   mykitsuck.h and aHotPlateSubstrate.h BOTH define `class TMyKitSuck`, with
//   DIFFERENT layouts (substrate begins `TMySucker Suck[][]`; golden's begins
//   `TALed *pLed[][]` + `bool bLed[][]`).  The objects this file actually links
//   against -- FLCarryKit/BLCarryKit/FRCarryKit/BRCarryKit/ptrInSHT -- are defined by
//   aHotPlateSubstrate.cpp:80-93/:1068, because mykitsuck.cpp is NOT in the build (see
//   CMakeLists ht9045_sm's PT-W3 note).  Compiling this TU against golden's layout
//   would have LINKED CLEANLY and then read every field at the wrong offset -- silent,
//   and invisible to any test that does not exercise the shuttle laser path.
//   Verified before swapping: the ONLY member this file touches on any kit object is
//   `.Item[][]`, which aHotPlateSubstrate.h's TMyKitSuck has (`int Item[][]`, :369).
//   Retire this swap when the MyKitSuck substrate-homecoming wave lands.
#include "aHotPlateSubstrate.h"       // golden "MyKitSuck.h" surface AS LINKED: TMyKitSuck (:365), FLCarryKit/BLCarryKit/FRCarryKit/BRCarryKit (:625-631), ptrInSHT (:1032), ZAxisDown
//   ptrOutSHT is NOT declared by aHotPlateSubstrate.h.  Its real definition is
//   acarry.cpp:99 (`TMyKitSuck *ptrOutSHT = 0;`, external linkage, assigned
//   &FRCarryKit/&BRCarryKit at acarry.cpp:2110/2112) -- same object golden's
//   MyKitSuck.h:379-381 family names.  Declared here rather than widening the
//   substrate header, which this wave may not edit for one symbol.
extern TMyKitSuck *ptrOutSHT;
#include "aoutarm9045.h"              // GetOutArmPitch_9045
#include "canary_support.h"           // ShowErrorMessage, ShowMyMessage, LastSet
//---------------------------------------------------------------------------
extern int GetRowCol(int &iRow, int &iCol);                                 //Eastsun 20260525 laser 整合 -- see GATE (1): real body is atester.cpp's, not redefined here.

// ZAxisDown -- golden ainarm2.h:42, real definition aHotPlateSubstrate.cpp:830
// (`const bool ZAxisDown = true;`). Forward-declared here rather than
// #include-ing aHotPlateSubstrate.h, for the SAME reason LaserSensor.cpp's
// MyDBIProcess forward-declare gives (that header's narrower TList/
// uPlateInfo/TInLaserCheck shims would collide with the real ones this file
// reaches via mykitsuck.h/LaserSensorInArm.h) -- matches golden's own idiom
// of forward-declaring single externs (golden LaserSensor.cpp:581) rather
// than pulling in a whole god-header for one symbol.
extern const bool ZAxisDown;

// GATE (3) -- ShowMyMessageBox_YES_NO: no tree-wide body; own static shim +
// macro, matching the SAME idiom atester.cpp:1551-1552 / csystem.cpp:
// 2868-2869 already established for this exact golden symbol.
static int W906_PTW3_ShowMyMessageBox_YES_NO(AnsiString /*s1*/, AnsiString /*s2*/="") { return 0; } // golden mymessbox.h -- offline: NO(0)
#define ShowMyMessageBox_YES_NO W906_PTW3_ShowMyMessageBox_YES_NO

#pragma package(smart_init)

int iShtLaserInitTask=1;
int iShtLaserCheckTask=1;
int iUseInArmShtLaserCheckTask=1;
int iUseOutArmShtLaserCheckTask=1;
TQPF_Timer LaserShtDelay;
TQPF_Timer LaserShtTimeOut;
TQPF_Timer LaserOutShtTimeOut;
double dCurrentValue[2][MAX_Index_Row][MAX_Index_Col];
double dShtDelayTime=0.5;  //等待馬達整定的時間
double dShtTimeOutTime=3.0;//讀取多久就是逾時
double dShtRetryGap=3.0;   //大於多少需要重新擷取一次
int iShuttleRetryCount; //重試的次數
int iShuttleMaxRetry=2; //最大重試次數
//---------------------------------------------------------------------------
void SetShtTrayColor(int iSht, int iX, int iY, eLaserStatus Status, bool Golden=false, int iInOutSht=0);
void SetShtTrayNumber(int iSht, int iX, double dNum1, double dNum2, bool Golden=false, int iInOutSht=0);
void SetShtTrayColor(int iSht, int iX, int iY, eLaserStatus Status, bool Golden, int iInOutSht)
{
    int myX=SThreadPara.iCHKStep-iX-1;

    if(Golden)
    {
        if(iInOutSht==0)
        {
            if(iSht==0)
            {
                fLaserSensor->mtShuttle1_Golden->SetCellColorIndex(myX, iY, Status);
            }
            else
            {
                fLaserSensor->mtShuttle2_Golden->SetCellColorIndex(myX, iY, Status);
            }
        }
        else
        {
            if(iSht==0)
            {
                fLaserSensor->mtOutShuttle1_Golden->SetCellColorIndex(myX, iY, Status);
            }
            else
            {
                fLaserSensor->mtOutShuttle2_Golden->SetCellColorIndex(myX, iY, Status);
            }
        }
    }
    else
    {
        if(iInOutSht==0)
        {
            if(iSht==0)
            {
                fLaserSensor->mtShuttle1->SetCellColorIndex(myX, iY, Status);
            }
            else
            {
                fLaserSensor->mtShuttle2->SetCellColorIndex(myX, iY, Status);
            }
        }
        else
        {
            if(iSht==0)
            {
                fLaserSensor->mtOutShuttle1->SetCellColorIndex(myX, iY, Status);
            }
            else
            {
                fLaserSensor->mtOutShuttle2->SetCellColorIndex(myX, iY, Status);
            }
        }
    }
}
//---------------------------------------------------------------------------
void SetShtGoldenTrayColor(int iSht, int iX, int iY, eLaserStatus Status)
{
    SetShtTrayColor(iSht, iX, iY, Status, true);
}
//---------------------------------------------------------------------------
void SetShtTrayNumber(int iSht, int iX, double dNum1, double dNum2, bool Golden, int iInOutSht)
{
    int myX=SThreadPara.iCHKStep-iX-1;
    if(Golden)
    {
        if(USE_LASER_DISTANCE==2)
        {
            if(iInOutSht==0)
            {
                if(iSht==0)
                {
                    if(dNum1!=0.0 && dNum2==0.0)
                        fLaserSensor->mtShuttle1_Golden->SetCellNumber(myX, 0, dNum1);
                    else if(dNum1==0.0 && dNum2!=0.0)
                        fLaserSensor->mtShuttle1_Golden->SetCellNumber(myX, 1, dNum2);
                }
                else
                {
                    if(dNum1!=0.0 && dNum2==0.0)
                        fLaserSensor->mtShuttle2_Golden->SetCellNumber(myX, 0, dNum1);
                    else if(dNum1==0.0 && dNum2!=0.0)
                        fLaserSensor->mtShuttle2_Golden->SetCellNumber(myX, 1, dNum2);
                }
            }
            else
            {
                if(iSht==0)
                {
                    if(dNum1!=0.0 && dNum2==0.0)
                        fLaserSensor->mtOutShuttle1_Golden->SetCellNumber(myX, 0, dNum1);
                    else if(dNum1==0.0 && dNum2!=0.0)
                        fLaserSensor->mtOutShuttle1_Golden->SetCellNumber(myX, 1, dNum2);
                }
                else
                {
                    if(dNum1!=0.0 && dNum2==0.0)
                        fLaserSensor->mtOutShuttle2_Golden->SetCellNumber(myX, 0, dNum1);
                    else if(dNum1==0.0 && dNum2!=0.0)
                        fLaserSensor->mtOutShuttle2_Golden->SetCellNumber(myX, 1, dNum2);
                }
            }
        }
        else
        {
        if(iSht==0)
        {
            fLaserSensor->mtShuttle1_Golden->SetCellNumber(myX, 0, dNum1);
            fLaserSensor->mtShuttle1_Golden->SetCellNumber(myX, 1, dNum2);
        }
        else
        {
            if(SThreadPara.iScanSensor==2)
            {
                fLaserSensor->mtShuttle2_Golden->SetCellNumber(myX, 0, dNum1);
                fLaserSensor->mtShuttle2_Golden->SetCellNumber(myX, 1, dNum2);
            }
            else
            {
                fLaserSensor->mtShuttle2_Golden->SetCellNumber(myX, 1, dNum1);
                fLaserSensor->mtShuttle2_Golden->SetCellNumber(myX, 0, dNum2);
            }
        }
    }
    }
    else
    {
        if(USE_LASER_DISTANCE==2)
        {
            if(iInOutSht==0)
            {
                if(iSht==0)
                {
                    if(dNum1!=0.0 && dNum2==0.0)
                        fLaserSensor->mtShuttle1->SetCellNumber(myX, 0, dNum1);
                    else if(dNum1==0.0 && dNum2!=0.0)
                        fLaserSensor->mtShuttle1->SetCellNumber(myX, 1, dNum2);
                }
                else
                {
                    if(dNum1!=0.0 && dNum2==0.0)
                        fLaserSensor->mtShuttle2->SetCellNumber(myX, 0, dNum1);
                    else if(dNum1==0.0 && dNum2!=0.0)
                        fLaserSensor->mtShuttle2->SetCellNumber(myX, 1, dNum2);
                }
            }
            else
            {
                if(iSht==0)
                {
                    if(dNum1!=0.0 && dNum2==0.0)
                        fLaserSensor->mtOutShuttle1->SetCellNumber(myX, 0, dNum1);
                    else if(dNum1==0.0 && dNum2!=0.0)
                        fLaserSensor->mtOutShuttle1->SetCellNumber(myX, 1, dNum2);
                }
                else
                {
                    if(dNum1!=0.0 && dNum2==0.0)
                        fLaserSensor->mtOutShuttle2->SetCellNumber(myX, 0, dNum1);
                    else if(dNum1==0.0 && dNum2!=0.0)
                        fLaserSensor->mtOutShuttle2->SetCellNumber(myX, 1, dNum2);
                }
            }
        }
        else
        {
            if(iSht==0)
            {
                fLaserSensor->mtShuttle1->SetCellNumber(myX, 0, dNum1);
                fLaserSensor->mtShuttle1->SetCellNumber(myX, 1, dNum2);
            }
            else
            {
                if(SThreadPara.iScanSensor==2)
                {
                    fLaserSensor->mtShuttle2->SetCellNumber(myX, 0, dNum1);
                    fLaserSensor->mtShuttle2->SetCellNumber(myX, 1, dNum2);
                }
                else
                {
                    fLaserSensor->mtShuttle2->SetCellNumber(myX, 1, dNum1);
                    fLaserSensor->mtShuttle2->SetCellNumber(myX, 0, dNum2);
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void SetShtGoldenTrayNumber(int iSht, int iX, double dNum1, double dNum2)
{
    SetShtTrayNumber(iSht, iX, dNum1, dNum2, true);
}
//---------------------------------------------------------------------------
bool ShtLaserInit(int iSht, bool bReset)        //將讀頭歸零並取得每個位置的數值
{
    static int Step=0;
    static int Zero=0;

    AnsiString Str, Str1;
    bool flag=false;
    bool flag1=false;
    int &Task=iShtLaserInitTask;

    if(bReset)
    {
        for(int i=0; i<fLaserSensor->mtShuttle1_Golden->FXItem; i++)
        {
            for(int j=0; j<fLaserSensor->mtShuttle1_Golden->FYItem; j++)
            {
                SetShtGoldenTrayColor(iSht, i, j, lsNeedCheck);
            }
        }

        iShuttleRetryCount=0;
        Zero=0;
        Task=1;
        Step=0;
        return flag;
    }

    switch(Task)
    {
        case 1:         //先將數值歸零
            flag1=MOT[MInShuttle1+iSht].MotorMove(SThreadPara.iLaserCheckPos[iSht][0]);

            if(flag1)
            {
                LaserShtDelay.SetSecAndOn(dShtDelayTime);
                Task=100;
            }
            break;
        case 100:
            if(LaserShtDelay.Off())
            {
                fLaserSensor->iLaser1Task=1;
                fLaserSensor->iLaser2Task=1;
                if(iSht==0)
                {
                    fLaserSensor->btSetToZeroSh1->Click();
                }
                else
                {
                    fLaserSensor->btSetToZeroSh2->Click();
                }

                LaserShtTimeOut.SetSecAndOn(dShtTimeOutTime);
                Task=200;
            }
            break;
        case 200:       //確認數值歸零
            if((iSht==0 && fLaserSensor->bSetToZeroSh1==false) ||
               (iSht==1 && fLaserSensor->bSetToZeroSh2==false))
            {
                Zero++;     //多做兩次
                if(Zero<2)
                {
                    LaserShtDelay.SetSecAndOn(0.1);
                    Task=100;
                }
                else
                {
                    iShuttleRetryCount=0;
                    Task=300;
                }
            }
            else if(LaserShtTimeOut.Off())
            {
                ShowErrorMessage("WAR0413", K_RETRY, MInShuttle1);  //Input Shuttle Laser Read Error, Please Check COM Port Connection!!
                Task=1;
            }
            break;
        case 300:       //偵測第(Step)個位置的數值
            SetShtGoldenTrayColor(iSht, Step, 0, lsChecking);
            SetShtGoldenTrayColor(iSht, Step, 1, lsChecking);

            flag1=MOT[MInShuttle1+iSht].MotorMove(SThreadPara.iLaserCheckPos[iSht][Step]);
            if(flag1)
            {
                LaserShtDelay.SetSecAndOn(dShtDelayTime);
                Task=400;
            }
            break;
        case 400:
            if(LaserShtDelay.Off())
            {
                if(iSht==0)
                {
                    if(SThreadPara.iScanSensor==1)
                        fLaserSensor->btGetValueSht1_1Row->Click();
                    else
                        fLaserSensor->btGetValueSht1_2Row->Click();
                }
                else //if(iSht==1)
                {
                    if(SThreadPara.iScanSensor==1)
                        fLaserSensor->btGetValueSht2_1Row->Click();
                    else
                        fLaserSensor->btGetValueSht2_2Row->Click();
                }
                LaserShtTimeOut.SetSecAndOn(dShtTimeOutTime);
                Task=500;
            }
            break;
        case 500:       //確認是否全部讀取完畢
            flag1=false;
            if(iSht==0 && fLaserSensor->bSh1_2Row==false && fLaserSensor->bSh1_1Row==false)
            {
                TestIF_File.dLaserValue[iSht][1][Step]=atof(fLaserSensor->palLaserValue_InShuttle1_2->Caption.c_str());
                if(SThreadPara.iScanSensor==2)
                {
                    TestIF_File.dLaserValue[iSht][0][Step]=atof(fLaserSensor->palLaserValue_InShuttle1_1->Caption.c_str());
                }

                SetShtGoldenTrayNumber(iSht, Step, TestIF_File.dLaserValue[iSht][1][Step], TestIF_File.dLaserValue[iSht][0][Step]);

                if(abs(TestIF_File.dLaserValue[iSht][1][Step])>=dShtRetryGap ||
                  (abs(TestIF_File.dLaserValue[iSht][0][Step])>=dShtRetryGap && SThreadPara.iScanSensor==2))
                {
                    iShuttleRetryCount++;
                    SetShtGoldenTrayColor(iSht, Step, 0, lsFail);
                    SetShtGoldenTrayColor(iSht, Step, 1, lsFail);

                    if(iShuttleRetryCount>=iShuttleMaxRetry)
                    {
                        ShowErrorMessage("WAR0414", K_RETRY, MInShuttle1);  //Input Shuttle Laser Read Error, Please Check COM Port Connection!!
                        iShuttleRetryCount=0;
                    }

                    Task=1;
                    break;
                }

                flag1=true;
            }
            else if(iSht==1 && fLaserSensor->bSh2_2Row==false && fLaserSensor->bSh2_1Row==false)
            {
                TestIF_File.dLaserValue[iSht][0][Step]=atof(fLaserSensor->palLaserValue_InShuttle2_1->Caption.c_str());
                if(SThreadPara.iScanSensor==2)
                {
                    TestIF_File.dLaserValue[iSht][1][Step]=atof(fLaserSensor->palLaserValue_InShuttle2_1->Caption.c_str());
                }

                SetShtGoldenTrayNumber(iSht, Step, TestIF_File.dLaserValue[iSht][1][Step], TestIF_File.dLaserValue[iSht][0][Step]);

                if(abs(TestIF_File.dLaserValue[iSht][0][Step])>=dShtRetryGap ||
                  (abs(TestIF_File.dLaserValue[iSht][1][Step])>=dShtRetryGap && SThreadPara.iScanSensor==2))
                {
                    iShuttleRetryCount++;
                    SetShtGoldenTrayColor(iSht, Step, 0, lsFail);
                    SetShtGoldenTrayColor(iSht, Step, 1, lsFail);

                    if(iShuttleRetryCount>=iShuttleMaxRetry)
                    {
                        ShowErrorMessage("WAR0415", K_RETRY, MInShuttle2);  //Input Shuttle Laser Read Error, Please Check COM Port Connection!!
                        iShuttleRetryCount=0;
                    }

                    Task=1;
                    break;
                }

                flag1=true;
            }
            else if(LaserShtTimeOut.Off())
            {
                ShowErrorMessage("WAR0413", K_RETRY, MInShuttle1);  //Input Shuttle Laser Read Error, Please Check COM Port Connection!!
                Task=300;
                break;
            }

            if(flag1)
            {
                SetShtGoldenTrayColor(iSht, Step, 0, lsNormal);
                SetShtGoldenTrayColor(iSht, Step, 1, lsNormal);

                Step++;
                iShuttleRetryCount=0;
                if(Step<SThreadPara.iCHKStep)
                {
                    Task=300;
                }
                else
                {
                    Task=600;
                }
            }
            break;
        case 600:
            Str="InitValue, ";
            for(int i=0; i<SThreadPara.iCHKStep; i++)
            {
                Str1.sprintf("%0.3f", TestIF_File.dLaserValue[iSht][0][i]);
                Str+=Str1+", ";
                Str1.sprintf("%0.3f", TestIF_File.dLaserValue[iSht][1][i]);
                Str+=Str1+", ";
            }

            if(iSht==0)
            {
                fLaserSensor->WriteToMemo(fLaserSensor->MemoSh1, Str);
            }
            else
            {
                fLaserSensor->WriteToMemo(fLaserSensor->MemoSh2, Str);
            }

            fLaserSensor->SaveShuttleLaserValue(iSht);
            flag=true;
    }

    return flag;
}
//---------------------------------------------------------------------------
bool CheckShtFloating(int iSht, bool bReset)     //確認每個位置的數值
{
    static int Step=0;
    static bool bHasDuplicateErr=false;
    static AnsiString ErrPart;
    AnsiString Str, Str1;

    bool flag=false;
    bool bHasErr=false;
    bool flag1=false;
    double dTemp;

    int &Task=iShtLaserCheckTask;

    if(bReset)
    {
        for(int i=0; i<fLaserSensor->mtShuttle1->FXItem; i++)
        {
            for(int j=0; j<fLaserSensor->mtShuttle1->FYItem; j++)
            {
                SetShtTrayColor(iSht, i, j, lsNeedCheck);
            }
        }
        iShuttleRetryCount=0;
        bHasDuplicateErr=false;
        Task=1;
        Step=0;
        return flag;
    }

    switch(Task)
    {
        case 1:
            flag1=MOT[MInShuttle1+iSht].MotorMove(SThreadPara.iLaserCheckPos[iSht][Step]);
            SetShtTrayColor(iSht, Step, 0, lsChecking);
            SetShtTrayColor(iSht, Step, 1, lsChecking);
            if(flag1)
            {
                LaserShtDelay.SetSecAndOn(dShtDelayTime);
                Task=50;
            }
            break;
        case 50:
            if(LaserShtDelay.Off())
            {
                if(iSht==0)
                {
                    if(SThreadPara.iScanSensor==1)
                        fLaserSensor->btGetValueSht1_1Row->Click();
                    else
                        fLaserSensor->btGetValueSht1_2Row->Click();
                }
                else //if(iSht==1)
                {
                    if(SThreadPara.iScanSensor==1)
                        fLaserSensor->btGetValueSht2_1Row->Click();
                    else
                        fLaserSensor->btGetValueSht2_2Row->Click();
                }
                Task=100;

                LaserShtTimeOut.SetSecAndOn(dShtTimeOutTime);
            }
            break;
        case 100:
            flag1=false;
            if(iSht==0 && fLaserSensor->bSh1_2Row==false && fLaserSensor->bSh1_1Row==false)
            {
                dCurrentValue[iSht][1][Step]=atof(fLaserSensor->palLaserValue_InShuttle1_2->Caption.c_str());
                if(SThreadPara.iScanSensor==2)
                    dCurrentValue[iSht][0][Step]=atof(fLaserSensor->palLaserValue_InShuttle1_1->Caption.c_str());
                else
                    dCurrentValue[iSht][0][Step]=0.0;

                SetShtTrayNumber(iSht, Step, dCurrentValue[iSht][1][Step], dCurrentValue[iSht][0][Step]);

                if(abs(dCurrentValue[iSht][1][Step])>=dShtRetryGap ||
                  (abs(dCurrentValue[iSht][0][Step])>=dShtRetryGap && SThreadPara.iScanSensor==2))
                {
                    SetShtTrayColor(iSht, Step, 0, lsFail);
                    SetShtTrayColor(iSht, Step, 1, lsFail);

                    iShuttleRetryCount++;
                    if(iShuttleRetryCount>=iShuttleMaxRetry)
                    {
                        ShowErrorMessage("WAR0414", K_RETRY, MInShuttle1);  //Input Shuttle Laser Read Error, Please Check COM Port Connection!!
                        iShuttleRetryCount=0;
                    }

                    Task=1;
                    break;
                }

                flag1=true;
            }
            else if(iSht==1 && fLaserSensor->bSh2_2Row==false && fLaserSensor->bSh2_1Row==false)
            {
                dCurrentValue[iSht][0][Step]=atof(fLaserSensor->palLaserValue_InShuttle2_1->Caption.c_str());
                if(SThreadPara.iScanSensor==2)
                {
                    dCurrentValue[iSht][1][Step]=atof(fLaserSensor->palLaserValue_InShuttle2_2->Caption.c_str());
                }
                else
                {
                    dCurrentValue[iSht][1][Step]=0.0;
                }

                SetShtTrayNumber(iSht, Step, dCurrentValue[iSht][1][Step], dCurrentValue[iSht][0][Step]);

                if(abs(dCurrentValue[iSht][0][Step])>=dShtRetryGap ||
                  (abs(dCurrentValue[iSht][1][Step])>=dShtRetryGap && SThreadPara.iScanSensor==2))
                {
                    SetShtTrayColor(iSht, Step, 0, lsFail);
                    SetShtTrayColor(iSht, Step, 1, lsFail);

                    iShuttleRetryCount++;
                    if(iShuttleRetryCount>=iShuttleMaxRetry)
                    {
                        ShowErrorMessage("WAR0415", K_RETRY, MInShuttle2);  //Input Shuttle Laser Read Error, Please Check COM Port Connection!!
                        iShuttleRetryCount=0;
                    }

                    Task=1;
                    break;
                }

                flag1=true;
            }
            else if(LaserShtTimeOut.Off())
            {
                ShowErrorMessage("WAR0413", K_RETRY, MInShuttle1);  //Input Shuttle Laser Read Error, Please Check COM Port Connection!!
                Task=1;
                break;
            }

            if(flag1)
            {
                SetShtTrayColor(iSht, Step, 0, lsNormal);
                SetShtTrayColor(iSht, Step, 1, lsNormal);

                Step++;
                iShuttleRetryCount=0;
                if(Step<SThreadPara.iCHKStep)
                {
                    Task=1;
                }
                else
                {
                    Task=200;
                }
            }
            break;
        case 200:
            ErrPart="";
            if(LastSet.iRealDummy!=DUMMY)
            {
                for(int i=0; i<SThreadPara.iCHKStep; i++)
                {
                    if(iSht==0)
                    {
                        dTemp=dCurrentValue[iSht][1][i]-TestIF_File.dLaserValue[iSht][1][i];
                        if(dTemp<-(TestIF_File.dIcThickness+TestIF_File.dLaserThresholdSht))    //疊料或置偏
                        {
                            if((FLCarryKit.Item[0][SThreadPara.iCHKStep-i-1]!=NULL_IC && FLCarryKit.Item[0][SThreadPara.iCHKStep-i-1]!=HAS_NULL_IC))
                            {
                                SetShtTrayColor(iSht, i, 1, lsFail);
                                bHasErr=true;

                                ErrPart+=IndexSuckName[0][SThreadPara.iCHKStep-i-1];
                            }
                        }

                        if(SThreadPara.iScanSensor==2)
                        {
                            dTemp=dCurrentValue[iSht][0][i]-TestIF_File.dLaserValue[iSht][0][i];
                            if(dTemp<-(TestIF_File.dIcThickness+TestIF_File.dLaserThresholdSht))    //疊料或置偏
                            {
                                if((FLCarryKit.Item[1][SThreadPara.iCHKStep-i-1]!=NULL_IC && FLCarryKit.Item[1][SThreadPara.iCHKStep-i-1]!=HAS_NULL_IC))
                                {
                                    SetShtTrayColor(iSht, i, 0, lsFail);
                                    bHasErr=true;
                                    ErrPart+=IndexSuckName[1][SThreadPara.iCHKStep-i-1];
                                }
                            }
                        }
                    }
                    else
                    {
                        if(SThreadPara.iScanSensor==2)
                        {
                            dTemp=dCurrentValue[iSht][0][i]-TestIF_File.dLaserValue[iSht][0][i];
                            if(dTemp<-(TestIF_File.dIcThickness+TestIF_File.dLaserThresholdSht))    //疊料
                            {
                                if((BLCarryKit.Item[1][SThreadPara.iCHKStep-i-1]!=NULL_IC && BLCarryKit.Item[1][SThreadPara.iCHKStep-i-1]!=HAS_NULL_IC))
                                {
                                    SetShtTrayColor(iSht, i, 1, lsFail);
                                    ErrPart+=IndexSuckName[1][SThreadPara.iCHKStep-i-1];
                                    bHasErr=true;
                                }
                            }

                            dTemp=dCurrentValue[iSht][1][i]-TestIF_File.dLaserValue[iSht][1][i];
                            if(dTemp<-(TestIF_File.dIcThickness+TestIF_File.dLaserThresholdSht))    //疊料
                            {
                                if((BLCarryKit.Item[0][SThreadPara.iCHKStep-i-1]!=NULL_IC && BLCarryKit.Item[0][SThreadPara.iCHKStep-i-1]!=HAS_NULL_IC))
                                {
                                    SetShtTrayColor(iSht, i, 0, lsFail);
                                    bHasErr=true;
                                    ErrPart+=IndexSuckName[0][SThreadPara.iCHKStep-i-1];
                                }
                            }
                        }
                        else
                        {
                            dTemp=dCurrentValue[iSht][0][i]-TestIF_File.dLaserValue[iSht][0][i];
                            if(dTemp<-(TestIF_File.dIcThickness+TestIF_File.dLaserThresholdSht))    //疊料
                            {
                                if((BLCarryKit.Item[0][SThreadPara.iCHKStep-i-1]!=NULL_IC && BLCarryKit.Item[0][SThreadPara.iCHKStep-i-1]!=HAS_NULL_IC))
                                {
                                    SetShtTrayColor(iSht, i, 0, lsFail);
                                    ErrPart+=IndexSuckName[0][SThreadPara.iCHKStep-i-1];
                                    bHasErr=true;
                                }
                            }
                        }
                    }
                }
            }

            Str="RunValue, ";
            for(int i=0; i<SThreadPara.iCHKStep; i++)
            {
                Str1.sprintf("%0.3f", dCurrentValue[iSht][0][i]);
                Str+=Str1+", ";
                Str1.sprintf("%0.3f", dCurrentValue[iSht][1][i]);
                Str+=Str1+", ";
            }

            if(iSht==0)
            {
                fLaserSensor->MemoSh1->Lines->Add(Str);
            }
            else
            {
                fLaserSensor->MemoSh2->Lines->Add(Str);
            }

            if(bHasErr)
            {
                Task=300;
            }
            else
            {
                for(int i=0; i<fLaserSensor->mtShuttle1->FXItem; i++)
                {
                    for(int j=0; j<fLaserSensor->mtShuttle1->FYItem; j++)
                    {
                        SetShtTrayColor(iSht, i, j, lsNormal);
                    }
                }
                bHasDuplicateErr=false;
                flag=true;
            }
            break;
        case 300:
            flag1=MOT[MInShuttle1+iSht].MotorMove(Prod.InSHT[iSht].iLeft);
            if(flag1)
            {
                Task=400;
            }
            break;
        case 400:
            Task=1;
            Step=0;

            if(iSht==0)
                ShowErrorMessage("JAM0411", K_RETRY, MInShuttle1, bHasDuplicateErr, ErrPart);   //Input Shuttle 1 device floating error.
            else
                ShowErrorMessage("JAM0412", K_RETRY, MInShuttle2, bHasDuplicateErr, ErrPart);   //Input Shuttle 2 device floating error.

            bHasDuplicateErr=true;
            break;
    }
    return flag;
}
//---------------------------------------------------------------------------
//==> Eastsun 20260525 laser 整合 : KYEC LaserSensorShuttle 用 InArm/OutArm 移動檢查
//---------------------------------------------------------------------------
bool MoveInArmXYToSHLaserPos(int iShuttle, int iCol, int iRow)
{
    int iShuttleXItem, iShuttleYItem, iShuttleXPitch, iShuttleYPitch;
    int  iXpos=0, iYpos=0, iXPitch=0, iVari=0, iYVari=0;

    iXPitch=6000;  //使用iXpitchMaxX3會在 HT-9132LS遇到問題
    iVari=GetInArmPitch_9045(iXPitch);

    if(iShuttle==0)
    {
        iXpos=Prod.XInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];
        iYpos=Prod.YInArm_Shuttle1_Place[iInArmYBase][iInArmXBase];
    }
    else
    {
        iXpos=Prod.XInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
        iYpos=Prod.YInArm_Shuttle2_Place[iInArmYBase][iInArmXBase];
    }

    iXpos+=iInArmShtXCenterPos;
    iYpos+=iInArmShtYCenterPos;

    iXpos-=860;      //與基準軸的相對距離
    iYpos-=5500;     //與基準軸的相對距離
    iYpos+=1750;
#if 0
    // golden call -- see GATE (2): GetShuttleSize has no compiled body
    // anywhere in this port tree (it lives in golden cinitial.cpp, not yet
    // translated). Retire this gate the moment cinitial.cpp's own
    // GetShuttleSize lands: delete this #if 0 arm's comment-out and restore
    // the call below, delete the #else arm.
    GetShuttleSize(iShuttleXItem, iShuttleYItem, iShuttleXPitch, iShuttleYPitch);
#else
    // Faithful default: cinitial.cpp:15236-15240's own 4-line body, inlined
    // (both symbols it uses -- GetRowCol, TestIF.dSiteXPitch/dSiteYPitch --
    // are already real in this tree). Note the (Y,X) argument order golden
    // itself uses: GetRowCol(YItem, XItem), not (XItem, YItem).
    GetRowCol(iShuttleYItem, iShuttleXItem);
    iShuttleXPitch=(int)TestIF.dSiteXPitch;
    iShuttleYPitch=(int)TestIF.dSiteYPitch;
#endif

    iXpos+=(((iShuttleXItem-1)/2.0)*iShuttleXPitch)-iCol*iShuttleXPitch;

    if(iShuttleYItem==1)
        iYpos+=0;
    else if(iShuttleYItem==2)
        iYpos+=(iShuttleYPitch/2.0)-(iRow*iShuttleYPitch);

    iXpos+=TestIF_File.iDeviceXOffset;
    iYpos+=TestIF_File.iDeviceYOffset;

    //Eastsun 20260525 laser 整合 : 新架構 8 參數陣列簽名 (Vari[X_PITCH_COUNT], ZDownSel[Row][Col], iZPos[Row][Col])
    int  VariArr[X_PITCH_COUNT];
    bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col];
    int  iZPosArr[MAX_ARM_Row][MAX_ARM_Col];
    for(int k=0; k<X_PITCH_COUNT; k++) VariArr[k]=iVari;
    memset(ZDownSel, 0, sizeof(ZDownSel));
    memset(iZPosArr, 0, sizeof(iZPosArr));
    if(InArmContinuousMove_9045(iXpos, iYpos, VariArr, iYVari, ZDownSel, iZPosArr, ZAxisDown))
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool MoveOutArmXYToSHLaserPos(int iShuttle, int iCol, int iRow)
{
    int iShuttleXItem, iShuttleYItem, iShuttleXPitch, iShuttleYPitch;
    int  iXpos=0, iYpos=0, iXPitch=0, iVari=0, iYVari=0;

    iXPitch=6000;
    iVari=GetOutArmPitch_9045(iXPitch);

    if(iShuttle==0)
    {
        iXpos=Prod.XOutArm_Shuttle1_Pick[iInArmYBase][iInArmXBase];
        iYpos=Prod.YOutArm_Shuttle1_Pick[iInArmYBase][iInArmXBase];
    }
    else
    {
        iXpos=Prod.XOutArm_Shuttle2_Pick[iInArmYBase][iInArmXBase];
        iYpos=Prod.YOutArm_Shuttle2_Pick[iInArmYBase][iInArmXBase];
    }

    iXpos+=iInArmShtXCenterPos;
    iYpos+=iInArmShtYCenterPos;

    iXpos+=5850;
    iYpos-=5500;
    iYpos+=1750;
#if 0
    // golden call -- see GATE (2) at MoveInArmXYToSHLaserPos above (same gate,
    // same reasoning, not repeated here).
    GetShuttleSize(iShuttleXItem, iShuttleYItem, iShuttleXPitch, iShuttleYPitch);
#else
    GetRowCol(iShuttleYItem, iShuttleXItem);
    iShuttleXPitch=(int)TestIF.dSiteXPitch;
    iShuttleYPitch=(int)TestIF.dSiteYPitch;
#endif

    iXpos+=(((iShuttleXItem-1)/2.0)*iShuttleXPitch)-iCol*iShuttleXPitch;

    if(iShuttleYItem==1)
        iYpos+=0;
    else if(iShuttleYItem==2)
        iYpos+=(iShuttleYPitch/2.0)-(iRow*iShuttleYPitch);

    iXpos+=TestIF_File.iOutDeviceXOffset;
    iYpos+=TestIF_File.iOutDeviceYOffset;

    //Eastsun 20260525 laser 整合 : 新架構 8 參數陣列簽名
    int  VariArr[X_PITCH_COUNT];
    bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col];
    int  iZPosArr[MAX_ARM_Row][MAX_ARM_Col];
    for(int k=0; k<X_PITCH_COUNT; k++) VariArr[k]=iVari;
    memset(ZDownSel, 0, sizeof(ZDownSel));
    memset(iZPosArr, 0, sizeof(iZPosArr));
    if(OutArmContinuousMove_9045(iXpos, iYpos, VariArr, iYVari, ZDownSel, iZPosArr, ZAxisDown))
    {
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//==> Eastsun 20260525 laser 整合 UseInArmCheckShtFloating 完整照搬 a-side L789-L1085
bool UseInArmCheckShtFloating(int iSht, bool bReset, bool bSetGold)     //確認每個位置的數值
{
    static int Step=0;
    static int iRow=0, iCol=0;
    static int iNowRow=0, iNowCol=0;
    static bool bHasDuplicateErr=false;
    static AnsiString ErrPart;
    AnsiString Str, Str1;

    bool flag=false;
    bool bHasErr=false;
    bool flag1=false;
    double dTemp=0.0;
    double dValue[2];
    int ret;
    int myX;

    int &Task=iUseInArmShtLaserCheckTask;

    if(bReset)
    {
        GetRowCol(iRow, iCol);

        for(int i=0; i<fLaserSensor->mtShuttle1->FXItem; i++)
        {
            for(int j=0; j<fLaserSensor->mtShuttle1->FYItem; j++)
            {
                if(bSetGold)
                    SetShtTrayColor(iSht, i, j, lsNeedCheck, true);
                else
                    SetShtTrayColor(iSht, i, j, lsNeedCheck);
            }
        }
        iShuttleRetryCount=0;
        bHasDuplicateErr=false;
        iNowRow=0;
        iNowCol=0;
        Task=1;
        Step=0;
        return flag;
    }

    switch(Task)
    {
        case 1:
            if(bSetGold)
                ShowMyMessage("請放置IC至Shuttle上");

            Task=30;
            break;
        case 30:
            flag1=MoveInArmXYToSHLaserPos(iSht, iNowCol, iNowRow);
            if(bSetGold)
                SetShtTrayColor(iSht, iNowCol, iNowRow, lsChecking, true);
            else
                SetShtTrayColor(iSht, iNowCol, iNowRow, lsChecking);
            if(flag1)
            {
                LaserShtDelay.SetSecAndOn(dShtDelayTime);
                Task=50;
            }
            break;
        case 50:
            if(LaserShtDelay.Off())
            {
                //測距
                Task=100;
                asLaserValue[0]="";
                fLaserSensor->btGetShValue->Click();
                LaserShtTimeOut.SetSecAndOn(1);
            }
            break;
        case 100:
            if(LaserShtTimeOut.Off())
                flag1=true;

            if(asLaserValue[0]!="")
            {
                Str=asLaserValue[0].SubString(11, asLaserValue[0].Length()-14);
                dTemp=atof(Str.c_str());
                myX=SThreadPara.iCHKStep-iNowCol-1;
                if(bSetGold)
                {
                    if(iSht==0)
                    {
                        if(iNowRow==0)
                        {
                            TestIF_File.dLaserValue[iSht][0][myX]=dTemp;
                            SetShtTrayNumber(iSht, iNowCol, dTemp, 0.0, true);
                        }
                        else
                        {
                            TestIF_File.dLaserValue[iSht][1][myX]=dTemp;
                            SetShtTrayNumber(iSht, iNowCol, 0.0, dTemp, true);
                        }
                    }
                    else if(iSht==1)
                    {
                        if(iNowRow==0)
                        {
                            TestIF_File.dLaserValue[iSht][0][myX]=dTemp;
                            SetShtTrayNumber(iSht, iNowCol, dTemp, 0.0, true);
                        }
                        else
                        {
                            TestIF_File.dLaserValue[iSht][1][myX]=dTemp;
                            SetShtTrayNumber(iSht, iNowCol, 0.0, dTemp, true);
                        }
                    }
                }
                else
                {
                    if(iSht==0)
                    {
                        if(iNowRow==0)
                            SetShtTrayNumber(iSht, iNowCol, dTemp, 0.0);
                        else
                            SetShtTrayNumber(iSht, iNowCol, 0.0, dTemp);
                    }
                    else if(iSht==1)
                    {
                        if(iNowRow==0)
                            SetShtTrayNumber(iSht, iNowCol, dTemp, 0.0);
                        else
                            SetShtTrayNumber(iSht, iNowCol, 0.0, dTemp);
                    }
                }
            }

            if(flag1)
            {
                if(bSetGold)
                    SetShtTrayColor(iSht, iNowCol, iNowRow, lsNormal, true);
                else
                    SetShtTrayColor(iSht, iNowCol, iNowRow, lsNormal);

                iNowCol++;
                if(iNowCol<iCol)
                {
                }
                else
                {
                    if(iNowRow<iRow)
                    {
                        iNowCol=0;
                        iNowRow++;
                    }
                }

                if(iNowRow==iRow)
                {
                    if(bSetGold)
                        Task=300;
                    else
                        Task=200;
                }
                else
                {
                    Task=30;
                }
            }
            break;
        case 200:
            //比較
            flag1=false;
            ErrPart="";
            for(int i=0; i<fLaserSensor->mtShuttle1->FXItem; i++)
            {
                for(int j=0; j<fLaserSensor->mtShuttle1->FYItem; j++)
                {
                    if(iSht==0)
                    {
                        dValue[0]=atof(fLaserSensor->mtShuttle1->GetCellText(i, j).c_str());
                        dValue[1]=atof(fLaserSensor->mtShuttle1_Golden->GetCellText(i, j).c_str());
                    }
                    else
                    {
                        dValue[0]=atof(fLaserSensor->mtShuttle2->GetCellText(i, j).c_str());
                        dValue[1]=atof(fLaserSensor->mtShuttle2_Golden->GetCellText(i, j).c_str());
                    }

                    if(iSht==0)
                        ptrInSHT=&FLCarryKit;
                    else if(iSht==1)
                        ptrInSHT=&BLCarryKit;

                    if(ptrInSHT->Item[j][i]!=NULL_IC && ptrInSHT->Item[j][i]!=HAS_NULL_IC)
                    {
                        if((abs((dValue[0]-dValue[1])*100))>=TestIF_File.dInLaserThresholdSht*100)
                        {
                            SetShtTrayColor(iSht, SThreadPara.iCHKStep-i-1, j, lsFail);
                            bHasErr=true;
                            ErrPart+=IndexSuckName[j][i];
                            flag1=true;
    //                        Str.sprintf("Row %d Col %d 異常", j, i);
    //                        ShowMyMessage(Str);
                        }
                    }
                }
            }

            if(flag1)
            {
                if(iSht==0)
                    ret=ShowErrorMessage("JAM0411", K_RETRY|K_SKIP, MInShuttle1, bHasDuplicateErr, ErrPart);   //Input Shuttle 1 device floating error.
                else
                    ret=ShowErrorMessage("JAM0412", K_RETRY|K_SKIP, MInShuttle2, bHasDuplicateErr, ErrPart);   //Input Shuttle 2 device floating error.

                if(ret==K_SKIP)
                {
                    flag=true;
                }
                else
                {
                    Task=250;
                }
            }
            else
            {
                flag=true;
            }
            break;
        case 250:
            GetRowCol(iRow, iCol);

            for(int i=0; i<fLaserSensor->mtShuttle1->FXItem; i++)
            {
                for(int j=0; j<fLaserSensor->mtShuttle1->FYItem; j++)
                {
                    if(bSetGold)
                        SetShtTrayColor(iSht, i, j, lsNeedCheck, true);
                    else
                        SetShtTrayColor(iSht, i, j, lsNeedCheck);
                }
            }
            iShuttleRetryCount=0;
            bHasDuplicateErr=false;
            iNowRow=0;
            iNowCol=0;
            Task=1;
            Step=0;
            break;
        case 300:
            ret=ShowMyMessageBox_YES_NO("Sure To Setting Laser Value", "確定要儲存測距數值？");
            if(ret==2)
            {
                Task=400;
            }
            else
            {
                fLaserSensor->SaveShuttleLaserValue(iSht);
                ShowMyMessage("請取出Shuttle上IC");
                flag=true;
            }
            break;
        case 400:
            for(int i=0; i<fLaserSensor->mtShuttle1->FXItem; i++)
            {
                for(int j=0; j<fLaserSensor->mtShuttle1->FYItem; j++)
                {
                    SetShtTrayColor(iSht, i, j, lsNormal, true);
                    if(iSht==0)
                    {
                        if(j==0)
                        {
                            TestIF_File.dLaserValue[iSht][0][iNowCol]=0.0;
                            SetShtTrayNumber(iSht, i, dTemp, 0.0, true);
                        }
                        else
                        {
                            TestIF_File.dLaserValue[iSht][1][iNowCol]=0.0;
                            SetShtTrayNumber(iSht, i, 0.0, dTemp, true);
                        }
                    }
                    else if(iSht==1)
                    {
                        if(j==0)
                        {
                            TestIF_File.dLaserValue[iSht][0][iNowCol]=0.0;
                            SetShtTrayNumber(iSht, i, dTemp, 0.0, true);
                        }
                        else
                        {
                            TestIF_File.dLaserValue[iSht][1][iNowCol]=0.0;
                            SetShtTrayNumber(iSht, i, 0.0, dTemp, true);
                        }
                    }
                }
            }
            ShowMyMessage("請取出Shuttle上IC");
            flag=true;
            break;
    }
    return flag;
}
//---------------------------------------------------------------------------
//<== Eastsun 20260525 laser 整合
//---------------------------------------------------------------------------
bool UseOutArmCheckShtFloating(int iSht, bool bReset, bool bSetGold) //Eastsun 20260525 laser 整合 完整照搬 a-side (iInOutSht=1)
{
    static int Step=0;
    static int iRow=0, iCol=0;
    static int iNowRow=0, iNowCol=0;
    static bool bHasDuplicateErr=false;
    static AnsiString ErrPart;
    AnsiString Str, Str1;

    bool flag=false;
    bool bHasErr=false;
    bool flag1=false;
    double dTemp=0.0;
    double dValue[2];
    int ret;
    int myX;

    int &Task=iUseOutArmShtLaserCheckTask;

    if(bReset)
    {
        GetRowCol(iRow, iCol);

        for(int i=0; i<fLaserSensor->mtOutShuttle1->FXItem; i++)
        {
            for(int j=0; j<fLaserSensor->mtOutShuttle1->FYItem; j++)
            {
                if(bSetGold)
                    SetShtTrayColor(iSht, i, j, lsNeedCheck, true, 1);
                else
                    SetShtTrayColor(iSht, i, j, lsNeedCheck, false, 1);
            }
        }
        iShuttleRetryCount=0;
        bHasDuplicateErr=false;
        iNowRow=0;
        iNowCol=0;
        Task=1;
        Step=0;
        return flag;
    }

    switch(Task)
    {
        case 1:
            if(bSetGold)
                ShowMyMessage("請放置IC到Shuttle上");

            Task=30;
            break;
        case 30:
            flag1=MoveOutArmXYToSHLaserPos(iSht, iNowCol, iNowRow);
            if(bSetGold)
                SetShtTrayColor(iSht, iNowCol, iNowRow, lsChecking, true, 1);
            else
                SetShtTrayColor(iSht, iNowCol, iNowRow, lsChecking, false, 1);
            if(flag1)
            {
                LaserShtDelay.SetSecAndOn(dShtDelayTime);
                Task=50;
            }
            break;
        case 50:
            if(LaserShtDelay.Off())
            {
                //測距
                Task=100;
                asLaserValue[1]="";
                fLaserSensor->btGetOutShValue->Click();
                LaserOutShtTimeOut.SetSecAndOn(1);
            }
            break;
        case 100:
            if(LaserOutShtTimeOut.Off())
                flag1=true;

            if(asLaserValue[1]!="")
            {
                Str=asLaserValue[1].SubString(11, asLaserValue[1].Length()-14);
                dTemp=atof(Str.c_str());
                myX=SThreadPara.iCHKStep-iNowCol-1;
                if(bSetGold)
                {
                    if(iSht==0)
                    {
                        if(iNowRow==0)
                        {
                            TestIF_File.dLaserValueOutSh[iSht][0][myX]=dTemp;
                            SetShtTrayNumber(iSht, iNowCol, dTemp, 0.0, true, 1);
                        }
                        else
                        {
                            TestIF_File.dLaserValueOutSh[iSht][1][myX]=dTemp;
                            SetShtTrayNumber(iSht, iNowCol, 0.0, dTemp, true, 1);
                        }
                    }
                    else if(iSht==1)
                    {
                        if(iNowRow==0)
                        {
                            TestIF_File.dLaserValueOutSh[iSht][0][myX]=dTemp;
                            SetShtTrayNumber(iSht, iNowCol, dTemp, 0.0, true, 1);
                        }
                        else
                        {
                            TestIF_File.dLaserValueOutSh[iSht][1][myX]=dTemp;
                            SetShtTrayNumber(iSht, iNowCol, 0.0, dTemp, true, 1);
                        }
                    }
                }
                else
                {
                    if(iSht==0)
                    {
                        if(iNowRow==0)
                            SetShtTrayNumber(iSht, iNowCol, dTemp, 0.0, false, 1);
                        else
                            SetShtTrayNumber(iSht, iNowCol, 0.0, dTemp, false, 1);
                    }
                    else if(iSht==1)
                    {
                        if(iNowRow==0)
                            SetShtTrayNumber(iSht, iNowCol, dTemp, 0.0, false, 1);
                        else
                            SetShtTrayNumber(iSht, iNowCol, 0.0, dTemp, false, 1);
                    }
                }
                flag1=true;
            }

            if(flag1)
            {
                if(bSetGold)
                    SetShtTrayColor(iSht, iNowCol, iNowRow, lsNormal, true, 1);
                else
                    SetShtTrayColor(iSht, iNowCol, iNowRow, lsNormal, false, 1);

                iNowCol++;
                if(iNowCol<iCol)
                {
                }
                else
                {
                    if(iNowRow<iRow)
                    {
                        iNowCol=0;
                        iNowRow++;
                    }
                }

                if(iNowRow==iRow)
                {
                    if(bSetGold)
                        Task=300;
                    else
                        Task=200;
                }
                else
                {
                    Task=30;
                }
            }
            break;
        case 200:
            //比對
            flag1=false;
            ErrPart="";
            for(int i=0; i<fLaserSensor->mtOutShuttle1->FXItem; i++)
            {
                for(int j=0; j<fLaserSensor->mtOutShuttle1->FYItem; j++)
                {
                    if(iSht==0)
                    {
                        dValue[0]=atof(fLaserSensor->mtOutShuttle1->GetCellText(i, j).c_str());
                        dValue[1]=atof(fLaserSensor->mtOutShuttle1_Golden->GetCellText(i, j).c_str());
                    }
                    else
                    {
                        dValue[0]=atof(fLaserSensor->mtOutShuttle2->GetCellText(i, j).c_str());
                        dValue[1]=atof(fLaserSensor->mtOutShuttle2_Golden->GetCellText(i, j).c_str());
                    }

                    if(iSht==0)
                        ptrOutSHT=&FRCarryKit;
                    else if(iSht==1)
                        ptrOutSHT=&BRCarryKit;

                    if(ptrOutSHT->Item[j][i]!=NULL_IC && ptrOutSHT->Item[j][i]!=HAS_NULL_IC)
                    {
                        if((abs((dValue[0]-dValue[1])*100))>=TestIF_File.dOutLaserThresholdSht*100)
                        {
                            SetShtTrayColor(iSht, SThreadPara.iCHKStep-i-1, j, lsFail, false, 1);
                            bHasErr=true;
                            ErrPart+=IndexSuckName[j][i];
                            flag1=true;
                        }
                    }
                }
            }

            if(flag1)
            {
                if(iSht==0)
                    ret=ShowErrorMessage("JAM0501", K_RETRY|K_SKIP, MOutShuttle1, bHasDuplicateErr, ErrPart);   //Out Shuttle 1 device floating error.
                else
                    ret=ShowErrorMessage("JAM0501", K_RETRY|K_SKIP, MOutShuttle2, bHasDuplicateErr, ErrPart);   //TODO Eastsun: a-side 用 JAM0502 但 c-side note.cpp 未註冊, 暫共用 JAM0501

                if(ret==K_SKIP)
                {
                    flag=true;
                }
                else
                {
                    Task=250;
                }
            }
            else
            {
                flag=true;
            }
            break;
        case 250:
            GetRowCol(iRow, iCol);

            for(int i=0; i<fLaserSensor->mtOutShuttle1->FXItem; i++)
            {
                for(int j=0; j<fLaserSensor->mtOutShuttle1->FYItem; j++)
                {
                    if(bSetGold)
                        SetShtTrayColor(iSht, i, j, lsNeedCheck, true, 1);
                    else
                        SetShtTrayColor(iSht, i, j, lsNeedCheck, false, 1);
                }
            }
            iShuttleRetryCount=0;
            bHasDuplicateErr=false;
            iNowRow=0;
            iNowCol=0;
            Task=1;
            Step=0;
            break;
        case 300:
            ret=ShowMyMessageBox_YES_NO("Sure To Setting Laser Value", "確定要儲存此距數值？");
            if(ret==2)
            {
                Task=400;
            }
            else
            {
                fLaserSensor->SaveShuttleLaserValue(iSht, 1);
                ShowMyMessage("請取出Out Shuttle上IC");
                flag=true;
            }
            break;
        case 400:
            for(int i=0; i<fLaserSensor->mtOutShuttle1->FXItem; i++)
            {
                for(int j=0; j<fLaserSensor->mtOutShuttle1->FYItem; j++)
                {
                    SetShtTrayColor(iSht, i, j, lsNormal, true, 1);
                    if(iSht==0)
                    {
                        if(j==0)
                        {
                            TestIF_File.dLaserValueOutSh[iSht][0][iNowCol]=0.0;
                            SetShtTrayNumber(iSht, i, dTemp, 0.0, true, 1);
                        }
                        else
                        {
                            TestIF_File.dLaserValueOutSh[iSht][1][iNowCol]=0.0;
                            SetShtTrayNumber(iSht, i, 0.0, dTemp, true, 1);
                        }
                    }
                    else if(iSht==1)
                    {
                        if(j==0)
                        {
                            TestIF_File.dLaserValueOutSh[iSht][0][iNowCol]=0.0;
                            SetShtTrayNumber(iSht, i, dTemp, 0.0, true, 1);
                        }
                        else
                        {
                            TestIF_File.dLaserValueOutSh[iSht][1][iNowCol]=0.0;
                            SetShtTrayNumber(iSht, i, 0.0, dTemp, true, 1);
                        }
                    }
                }
            }
            ShowMyMessage("請取出Shuttle上IC");
            flag=true;
            break;
    }
    return flag;
}
