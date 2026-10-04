//---------------------------------------------------------------------------
//  aTester_Front.cpp  --  TESTER/INDEX front (Z1) down-press test-head SM
//
//  Translation wave: W64b-Translate (front test-head destroy/down-press SM)
//  Translator: AI(W64b-Translate) 20260706
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8580 lines, cp950)
//
//  SCOPE (narrow, per translation-plan hand-off -- do NOT expand):
//    * bArm1PutDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]   golden :89-92
//    * ResetArm1PutFlag()                                     golden :127-136 (file-local; not in .h)
//    * CheckZ1IsDown()                                        golden :170-222
//    * TestZ1OutRandge()                                      golden :224-255
//    * TestZ1SetPos()                                         golden :291-298
//    * iFrontTestDestroyICTask + InitFrontTestDestroyICTask() golden :300-305
//    * FTempRiseHasICDelay (TQPF_Timer)                       golden :308 (file-scope global DoFrontTestDestroyIC needs; declared here, just outside the stated 309-869 range)
//    * DoFrontTestDestroyIC(bool bCheckZ2)                    golden :309-865 (full switch(Task) SM, verbatim)
//  Every OTHER golden aTester_Front.cpp function (DoFrontTestSuckIC, DoTestYFront,
//  TestZ1OutRandge2, DoArm1Suck, DoArm1D44VacCheck, FTestSeparateSLK/CombineSLK,
//  DoFRTCAutoModelVerify, DoFrontTestPurgBeforePickShuttle, ...) is OUT OF SCOPE
//  this wave and stays served by the existing atester_shims.h/.cpp offline shims.
//
//  TRANSLATION RULES (mirrors atester.cpp/ainarm9045.cpp conventions)
//  --------------------------------------------------------------------
//    * Off Borland: __fastcall removed; AnsiString/TQPF_Timer via vclcompat;
//      numeric semantics EXACT (no int/float rewrites).
//    * SOFT_SIMULTE is NOT defined in this target build (MachineType.h:30 keeps
//      the golden DEBUG_INDEX_UPH toggle commented out the same way) -- the
//      #ifdef SOFT_SIMULTE / #else and #ifdef DEBUG_INDEX_UPH / #else blocks are
//      reproduced VERBATIM (both branches) exactly as golden; the preprocessor
//      selects the #else branch today, same as every other translated engine file.
//    * Big5 Chinese comments decoded via cp950, preserved as UTF-8. ZERO U+FFFD.
//
//  GATED DEPENDENCIES (verified ABSENT from the already-translated substrate;
//  NOT part of the pre-verified call-graph list handed to this wave -- discovered
//  while translating DoFrontTestDestroyIC's body and gated per the "#if 0 //
//  TODO(W7)" convention, each with a minimal compiling default that mirrors the
//  golden park/default behaviour for that branch):
//    1. FTestSuck.bNeedCheck[][]              -- TMyKitSuck (aHotPlateSubstrate.h)
//       has no per-cell "D44 recheck" flag member yet (golden MyKitSuck.h).
//       Reads default false / writes no-op via W64B_NEEDCHECK_GET/SET (mirrors
//       IniConfig.bD44CheckIndexICDestroy's own zero-initialized default).
//    2. TMySucker::sRecordStartOffDelayTime / sRecordEndOffDelayTime /
//       iRecordOffDelayTime + the QueueAirOnTime1.Add() telemetry call that
//       reads them -- absent from the minimal TMySucker shim; telemetry-only,
//       parked whole (golden :502-503).
//    3. FRCarryKit.MoveSuckData(TMyKitSuck&,int,int) -- no such TMyKitSuck method
//       yet; parked (golden :505).
//    4. TMyProductionRecord::AddIndexPlaceShuttleRecord -- not in the translated
//       Public/MyProductionRecord.h yet; parked (golden :506).
//    5. RecordIndexAirOnTime1() -- golden cObserver.h, no translated home yet;
//       parked no-op (golden :559, :569).
//    6. LastSet.iIndexInputOutPut[] -- not in the LAST_GENERAL_SET minimal shim
//       (canary_support.h) yet; parked no-op (golden :500).
//    7. DoVibrateOutShuttle(int,bool) -- golden ainarm2.h, no translated home yet;
//       void call parked no-op (golden :564); bool-checked call defaults to
//       true ("vibrate done") so the SM is not stuck park-waiting, mirroring the
//       fiosetview ProcessIndexSuckDestroy1/2 "suck self-check done" precedent
//       already established in atester.cpp (golden :626).
//    8. fiosetview->ProcessIndexSuckDestroy1(int) -- TfiosetviewShim
//       (atester_shims.h) exposes bIndexSuck[][][] but not this method (SAME
//       documented gap atester.cpp already gates via W7T1_FIOSET_PISD1/2); this
//       file's own W64B_FIOSET_PISD1 mirrors that exact precedent (golden :666,
//       :709), default true ("done").
//    9. FRCarryKit.SetHasNullIcToNullIc() -- no such TMyKitSuck method yet (SAME
//       documented gap as csystem.cpp's W7C2_BLCARRYKIT_SETHASNULLIC); parked
//       no-op via W64B_FRCARRYKIT_SETHASNULLIC (golden :675, :860).
//   10. ATC_TYPE_33 / ATC_TYPE_35 -- golden ATC_Handler_Side.h manifest constants
//       (plain #define 33 / 35); that header has no translated home yet, but the
//       values are unambiguous data (not a behavior gate) -- defined locally.
//   11. TMyProductionRecord::AddIndexCycleTimeRecord -- Public/MyProductionRecord.h
//       is a declaration-only forward-stub this wave (no .cpp); parked, same
//       treatment as gate #4's AddIndexPlaceShuttleRecord (golden :501).
//       AI(W64b-Integrate) 20260706.
//  Every gate is individually re-cited at its call site below.
//
//  INTEGRATE-STEP FIXES (AI(W64b-Integrate) 20260706 -- discovered as
//  undefined-reference LINK errors, not caught by either translator's report
//  since neither is a "call graph" gap in the usual sense -- both are golden
//  symbols DECLARED in an already-translated header with NO compiled body
//  anywhere in the tree):
//    * OutSHT1InLF() / CheckIndexAllSuckICFallDown() (golden csystem.cpp:698 /
//      :2417) -- csystem.h declared them but csystem_predicates.cpp never
//      defined them.  Golden bodies translated FAITHFULLY (not gated) into
//      csystem_predicates.cpp -- OutSHT1InLF is a trivial one-line delegate,
//      CheckIndexAllSuckICFallDown is a self-contained FTestSuck/BTestSuck
//      grid scan with no further substrate gaps.
//    * GetSiteCount(bool) (golden cprod.cpp:305, declared cprod.h:3299) --
//      the real body exists in cprod.cpp but sits inside that file's whole-
//      region `#if 0 // TODO(W6)` gate (cprod.cpp:184-4036), so it never
//      compiled.  A faithful duplicate was added to atester_shims.cpp (same
//      "shim a real-but-gated-elsewhere body" pattern already used there for
//      IsInArmCleanOutFinish/IsInArmOneCycleFinish); TODO(W6): delete the
//      duplicate once cprod.cpp's gate lifts.
//  Integrate-step note: atester_shims.h/.cpp TODAY still declare+define
//  offline-stub bodies for all 6 symbols this file now defines for real
//  (iFrontTestDestroyICTask, InitFrontTestDestroyICTask, DoFrontTestDestroyIC,
//  CheckZ1IsDown, TestZ1OutRandge, TestZ1SetPos) -- REMOVED from
//  atester_shims.h/.cpp as part of this same integrate step to avoid a
//  duplicate-definition link error.
//---------------------------------------------------------------------------
#include "aTester_Front.h"
#include "aTester_Rear.h"           // CheckZ2IsDown() -- golden aTester_Rear.h; sibling W64b-Translate wave (translated in parallel; this header did not exist yet at the time this file was written -- see final report)
#include "atester_shims.h"          // IsNNMode() (golden cinitial.h:60, offline shim) -- ALSO still stubs the 6 symbols above; see integrate-step note
// AI(W5-Automation-Integrate) 20260710: ProcessPiggyBackFunction is now real
// (atester_ProcessCount.cpp, removed from atester_shims.h) -- include its own header.
#include "atester_ProcessCount.h"

#include "aArmHeader.h"             // __FUNC__ shim
#include "MachineType.h"            // MAX_SOCKET_ROW/COL, NN_2Row, eSpeedPart::IndexArm, ChangeToFloatNonPcnt
#include "Motor/mymotor.h"          // MOT[], TTrayMotor, RecordIndexPositionError, ShowIndexMotorError
#include "csystem.h"                // OutSHT1InLF/InShtInLF/InSHT1InRT/CheckIndexAllSuckICFallDown/IsInArmOneCycleFinish/IsInArmCleanOutFinish
#include "atester.h"                // GetIndexZSpeed/IndexAlarmInArmAway/IndexCheck4Site/DoTemperatureRise
#include "cmydef.h"                 // global scalar universe + IC consts + enums
#include "cprod.h"                  // Prod / TestIF / TestIF_File / CosFunction / ArmSpeed_File / GetSiteCount
#include "aHotPlateSubstrate.h"     // FTestSuck / FRCarryKit / FLCarryKit / TestSocket
#include "acarry_shims.h"           // ATC_InterfaceForm (golden ATC_Handler_Side.h, offline shim)
#include "canary_support.h"         // LastSet / ShowMyMessage / ShowErrorMessage
#include "FormsFacade.h"            // Wave 2: fMain->ResetRecordforPiggyBack / fSCKART->iInputJamCnt+AddOutputJamCnt
// AI(pt-wave) 20260811 PT-W7c: includes the appended bodies need. Two of these were
// injected MID-FILE by part files (02237, 08110); hoisted here, because a mid-file include
// in the stitched TU is a compile-order hazard and duplicates these lines.
#include "MessageDef.h"                 // HHandler2Gpib (MessageDef.cpp:268) + MSG_CMD_* consts
#include "myswitch.h"                   // SW[]        -- myswitch.h:43
#include "mysensor.h"                   // Sen[]       -- mysensor.h:48
#include "mycylin.h"                    // Cylinder[]  -- mycylin.h:176
#include "SECSGEM/SecsEventType.h"      // SECS event enums used by DoFTestSuckTestIC
#include "SECSGEM/SecsEventReport.h"    // EventReport()
#include "Automation/AMR.h"              // AMR -- Automation/AMR.cpp:66 (same as aTester_Rear.cpp)

// ---------------------------------------------------------------------------
//  PT-W7c: the SIXTEEN file-scope globals golden declares BETWEEN functions.
//  Enumerated by tools/census/wave_targets.py (built after PT-W7b missed the same class on
//  the Rear side) and re-verified absent at integration, because absence claims expire.
//  Added by the integrator, NOT by any agent: two agents emitting one global is a
//  multiple-definition link error, so the wave brief made these the main loop's job.
//
//  TYPE NOTE (the one that would link clean and lie): golden types DoTestYFrontDelay /
//  DoTestYFrontDelay2 as HTimer at :4071. The port has no HTimer implementation -- golden's
//  real one is D:\HT9045\elec\Component\htimer.h, outside the version tree and never ported
//  -- so this TU would bind to atester_shims.h:463 struct HTimer, whose Off() is hard-coded
//  true. That would silently zero every dwell in the DoTestYFront timing states, and it lacks
//  SetMSAndOn entirely. TQPF_Timer (myTimer.h) is the real timer and is the alias idiom
//  already used at acatchtray.cpp:114 / cMyDNM100UD.cpp:85 / MyPLC_IO_Modbus.cpp:49.
//  Three separate PT-W7c agents independently flagged this same requirement.
//  AI(pt-wave) 20260811
// ---------------------------------------------------------------------------
int iHangupCTArm1=0;                                                            // golden aTester_Front.cpp:75
bool bReadFrontTorqueOK=false;                                                  // golden :78
TQPF_Timer FTestSocketClampCloseDelay;                                          // golden :94
TQPF_Timer FTestSocketClampOpenDelay;                                           // golden :95
TQPF_Timer FTestSocketClampTimeOutDelay;                                        // golden :96
TQPF_Timer FTorqueTimeOutDelay;                                                 // golden :97
TQPF_Timer Z1UpZ2DownTimer;                                                     // golden :98
TQPF_Timer DoFTestSuckTestICDelay;                                              // golden :2232
TQPF_Timer DoFTestSuckOffDelay;                                                 // golden :2235
TQPF_Timer DoUseSocketTestYFrontDelay;                                          // golden :3943
TQPF_Timer DoTestYFrontDelay, DoTestYFrontDelay2;                               // golden :4071 (golden type HTimer -- see TYPE NOTE)
TQPF_Timer hCheckSockerDelay2;                                                  // golden :4921
TQPF_Timer iWaitIndexArm1;                                                      // golden :5160
TQPF_Timer hTestZ2Delay;                                                        // golden :5161
TQPF_Timer hBRTCTimeOutDelay;                                                   // golden :5162
TQPF_Timer hDoFrontTestPurgBeforePickShuttle;                                   // golden :8010
//---------------------------------------------------------------------------

// AI(W64b-Translate) 20260706: golden ATC_Handler_Side.h:24-25 manifest constants
// (plain int literals; that header has no translated home yet, but the values are
// unambiguous data, not a behavior gate) -- needed by the ATC-type compare in
// DoFrontTestDestroyIC case 200/500.
#define ATC_TYPE_33 33
#define ATC_TYPE_35 35

// AI(W64b-Translate) 20260706: golden :308 -- file-scope TQPF_Timer DoFrontTestDestroyIC
// needs (Index-arm temperature-rise soak delay).  Declared here, just outside the
// stated 309-869 translation range, because it is otherwise undeclared anywhere in
// the target tree.  (golden :307 TQPF_Timer CheckSocketHasICDelay1 is NOT declared --
// every golden reference to it inside DoFrontTestDestroyIC is itself commented out,
// so it is dead text, preserved verbatim as a comment below, not a live symbol.)
TQPF_Timer FTempRiseHasICDelay;                                                 //Ifor 20230418 add

// AI(W64b-Translate) 20260706: golden :89-92 -- per-cell "already alarmed this site"
// dedupe flag DoFrontTestDestroyIC reads/writes; file-local (golden aTester_Front.h
// does not declare it either).
bool bArm1PutDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},    //Steven 20100105 Start: 排除重複的Alarm Code
                                                           {false, false, false, false, false, false, false, false},
                                                           {false, false, false, false, false, false, false, false},
                                                           {false, false, false, false, false, false, false, false}};

// ---------------------------------------------------------------------------
//  Gate helpers (TODO(W7) -- see file-header "GATED DEPENDENCIES" list above for
//  the full rationale of each).  Each mirrors the golden default/park behaviour
//  for its branch so DoFrontTestDestroyIC's cursor flow compiles + pumps
//  unchanged when the gated sub-feature is (as today, by default) inert.
// ---------------------------------------------------------------------------
#if 1 // AI(W906-W2-A46) 20260926: gate FLIPPED to the golden arm -- A4-6 (8ff6c754) merged TMyKitSuck/TMySucker into the one golden layout (mykitsuck.h), premise dead: bool bNeedCheck[][] is a member again (mykitsuck.h:303); the #else arm read false / dropped writes -- golden :493/:646/:683/:734/:746/:791/:8560/:8567
#define W64B_NEEDCHECK_GET(i,j)      (FTestSuck.bNeedCheck[i][j])
#define W64B_NEEDCHECK_SET(i,j,v)    (FTestSuck.bNeedCheck[i][j]=(v))
#else
#define W64B_NEEDCHECK_GET(i,j)      (false)
#define W64B_NEEDCHECK_SET(i,j,v)    do{}while(0)
#endif

// AI(W64b-Translate) 20260706: mirrors atester.cpp's own W7T1_FIOSET_PISD1/2
// precedent for the SAME documented gap (TfiosetviewShim has bIndexSuck[][][]
// but not the ProcessIndexSuckDestroy1/2 pump methods, golden iosetview.h).
// Offline (no DAQ): report the suck self-check "complete" (true) so the D44
// recheck SM advances instead of parking forever.
static bool W64B_ProcessIndexSuckDestroy1(int iType){ return fiosetview->ProcessIndexSuckDestroy1(iType); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）        // golden iosetview.h:3018 -- offline: suck self-check done (true)
#define W64B_FIOSET_PISD1(iType)     W64B_ProcessIndexSuckDestroy1(iType)

// AI(W64b-Translate) 20260706: same documented gap as csystem.cpp's
// W7C2_BLCARRYKIT_SETHASNULLIC() (TMyKitSuck has no SetHasNullIcToNullIc()
// method yet, golden MyKitSuck.h; docs/MIGRATION_ROADMAP.md).
#define W64B_FRCARRYKIT_SETHASNULLIC()     FRCarryKit.SetHasNullIcToNullIc()   // golden FRCarryKit.SetHasNullIcToNullIc() -- AI(W906-W2-KITDATA) 20260926: 原本是空巨集（golden 的「out shuttle has_null_ic hang up fix」被吞掉），真方法 mykitsuck.h:387 A4-6 起存在

//------------------------------------------------------------------------------
void ResetArm1PutFlag()                                                         //Steven 20241017 : 打包成function
{
    for(int i=0; i<FTestSuck.iMaxRow; i++)
    {
        for(int j=0; j<FTestSuck.iMaxCol; j++)
        {
            bArm1PutDuplicateErr[i][j]=false;
        }
    }
}
//------------------------------------------------------------------------------
bool CheckZ1IsDown()
{
    #ifdef SOFT_SIMULTE
        return true;
    #else
        long lPos[4]={0, 0, 0, 0};                                              //kevin 20150915

        if(IniConfig.bD52InterFaceErrHeadNeedUp && bNowDoInterFaceErrorStep)
            return false;

        if(bContactTimeOverStep)
            return false;

        if(MOT[MTestZ1].Motor->Enable==false)
            return true;

        if(CosFunction.bSortingBy2DList==true &&
           LastSet.iTester==_2D_SORT &&
           TestIF_File.bSortingBy2DIDList==true)                                //Frank 20221122 : 2DID sorting for ATK
        {
            return true;
        }

        if(TestIF_File.iShuttleMode==1 &&                                       //20111114  Dell for Disable Index Arm    Start
           TestIF_File.iShuttle_Sel==1)                                         //jou 2014-11-13 remake code , close arm index postion error
        {
            return true;
        }

        if(bUseTwoArm32Site==true)
        {                                                                       //nn Mode 是同時有吸取IC動作
            return true;
        }
        else
        {
            //==> Eastsun 20260511 F006 整合: Ifor 20240430 add:secs gem cmd Index Yiel Fail
            if(CUSTOMER_CODE==CC_KYEC_LEE && iSECSGEM_ConsecutiveFailureAlarm==2)//Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
            {
                return true;
            }
            //<== Eastsun 20260511 F006 整合
            else if(MOT[MTestZ1].Gali_ReadEncoderPos()>Prod.iTestDown_Z1)
            {
                lPos[0]=MOT[MTestZ1].Gali_ReadEncoderPos();                     //kevin 20150915
                lPos[1]=Prod.iTestDown_Z1;                                      //kevin 20150915
                RecordIndexPositionError("CheckZ1IsDown()", true, true, false, false, &lPos[0]);                        //kevin 20150915 record
                ShowIndexMotorError(AnsiString("CheckZ1IsDown"));
                return false;
            }
        }
        return true;
    #endif
}
//==============================================================================
bool TestZ1OutRandge()
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
        int Pos1=MOT[MTestZ1].Gali_ReadPos();
        int Pos2=MOT[MTestZ1].Gali_ReadEncoderPos();
        Pos1-=Pos2;
        if(Pos1<0)
            Pos1=0-Pos1;

        if(Pos1>10)                                                             //Steven 20100728 :縮小範圍
            return true;
        Pos1=MOT[MTestY1].Gali_ReadPos();
        Pos2=MOT[MTestY1].Gali_ReadEncoderPos();
        Pos1-=Pos2;
        if(Pos1<0)
            Pos1=0-Pos1;

        if(Pos1>10)                                                             //Steven 20100728 :縮小範圍
            return true;
        Pos1=MOT[MTestY2].Gali_ReadPos();
        Pos2=MOT[MTestY2].Gali_ReadEncoderPos();
        Pos1-=Pos2;
        if(Pos1<0)
            Pos1=0-Pos1;

        if(Pos1>10)                                                             //Steven 20100728 :縮小範圍
            return true;
        return false;
    #endif
}
//==============================================================================
void TestZ1SetPos()
{
    int Pos1=ChangeToFloatNonPcnt((double)(MOT[MTestY1].Gali_ReadEncoderPos()), (double)(MOT[MTestY1].Motor->GearRatio));                                       //Steven 20100728 Start: 要加上齒輪比
    int Pos2=ChangeToFloatNonPcnt((double)(MOT[MTestZ1].Gali_ReadEncoderPos()), (double)(MOT[MTestZ1].Motor->GearRatio));
    char str[256];
    sprintf(str, "DP%d,%d", Pos1, 0-Pos2);
    MOT[MTestY1].Gali_Command(str, __FUNC__);
}
//---------------------------------------------------------------------------
int iFrontTestDestroyICTask=1;
void InitFrontTestDestroyICTask()
{
    iFrontTestDestroyICTask=1;
    bZ1Isdownflag=false;
}
//------------------------------------------------------------------------------
bool DoFrontTestDestroyIC(bool bCheckZ2)
{
    static int iRetry=0;
    static int iSiteCount=0;                                                    //kevin 20190709  index check one by one

    int &Task=iFrontTestDestroyICTask;
    int sp=0, iPos=0;
    int iNN=IsNNMode();
    bool flag=false, flag2=false;
    bool bHasDuplicateErr=false;
    bool bHasErr=false;
    AnsiString sTempMessge="";
    AnsiString ErrPart="";
    static bool bHasDropErr=false;

    switch(Task)
    {
        case 1:
            iSiteCount=0;                                                       //kevin 20190819  index check one by one
            bHasDropErr=false;
            iIndexTakeDeviceChk1=0;                                             //Steven 20250224 回吸檢測狀態
            if(FRCarryKit.UseSiteHasIC())
            {
                return false;
            }

            if(OutSHT1InLF()==false)
            {
                Task=100;                                                       //JerryYang 20170106 (Steven) fix out shuttle 偵測殘料後hang up
                return false;
            }

            MOT[MInShuttle1].fCanMoveM=false;
            Task=200;

            if(CheckIndexAllSuckICFallDown(true, false))                        //jou 20210713 : 增加index destroy ic時掉料偵測
            {
                ShowMyMessage("Index Arm1 掉料，請檢查IC是否有在Out Shuttle中!!");
            }

            if(iNN==NN_2Row)
                break;
            break;                                                              //JerryYang 20170106 (Steven) fix out shuttle 偵測殘料後hang up
        case 100:
            #ifdef DEBUG_INDEX_UPH
            if(MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Safe, iIndexSpeed, iIndexAcc);                                    //Steven 20170818 : 測試UPH用
            #else
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed))
            #endif
            {
                if(bHasDropErr)                                                                                         //JerryYang 20241122 : Index arm放料到shuttle偵測drop error
                {
                    bHasDropErr=false;
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Suck[i][j].Enable       &&
                               FTestSuck.Suck[i][j].SenUsing!="" &&
                               FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                               FTestSuck.Item[i][j]!=NULL_IC)
                            {
                                if(FTestSuck.Suck[i][j].W906_GetStatusHeld()==false)  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都要有真空，掉了要持續 100 ms 才算掉料
                                {
                                    ErrPart+=IndexSuckName[i+iNN][j];
                                    bHasErr=true;
                                }
                            }
                        }
                    }

                    if(LastSet.iRealDummy==REALLY && bHasErr)
                    {
                        if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                     //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                            bIsTestSitICFallDown=true;
                        ShowErrorMessage("JAM0303", K_SKIP, MTestZ1, false, ErrPart);                                   //Steven 20100129 : Device Drop Error
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(FTestSuck.Suck[i][j].Error ||
                                   (FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                    FTestSuck.Item[i][j]!=NULL_IC &&
                                    FTestSuck.Suck[i][j].W906_GetStatusHeld()==false))                                           //有用到且有吸到IC的卻掉了  AI(W906-F03-QUADVAC) 20261004: 群組：4 個都要有真空，掉了要持續 100 ms 才算掉料
                                {
                                    FTestSuck.SetItemData(i, j, HAS_NULL_IC);                                           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                                    FTestSuck.Suck[i][j].Normal();                                                      //Steven 20110829 : 把真空關掉
                                }
                            }
                        }
                    }
                }
                Task=1;
            }
            break;
        case 200:
            bHasDropErr|=CheckIndexAllSuckICFallDown(true, false);              //JerryYang 20241122 : Index arm放料到shuttle偵測drop error
            if(bHasDropErr)
            {
                Task=100;
                break;
            }

            if(InShtInLF(0)==false)                                             //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
            {
                ShowMyMessage("Shuttle 1 position error ", "Shuttle 1 位置錯誤");
                MOT[MInShuttle1].fCanMoveM=true;
                bZ1Isdownflag=false;                                            //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
                Task=1;
            }
            else if(bCheckZ2==false || CheckZ2IsDown())                         //ChungHung 20111115
            {
                flag2=false;
                bZ1Isdownflag=true;                                             //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
                if(FTestSuck.HasRealIC()==false)
                {
                    flag2=true;
                }
                else
                {
                    bSuckingFlagZ1=true;                                        //Steven 20240916 : index下降到shuttle吸放料
                    if(IniConfig.bD54SlowDown)                                  //ChungHung 20110816 add Index Place slow down
                    {
                        sp=GetIndexZSpeed(0);                                   //Steven 20160524 : Index Z軸速度整合為Function
                        #ifdef DEBUG_INDEX_UPH
                        if(MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Place, iIndexSpeed, iIndexAcc))
                        #else
                        if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Place, sp))
                        #endif
                        {
                            flag2=true;
                        }
                    }
                    else
                    {
                        #ifdef DEBUG_INDEX_UPH
                        if(MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Place, iIndexSpeed, iIndexAcc))
                        #else
                        if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Place, MOT[MTestZ1].GailSpeed, IniConfig.iD46WaitIndexDestroyTime))
                        #endif
                        {
                            flag2=true;
                        }
                    }
                }

                if(flag2)
                {
                    FTestSuck.ResetAll();
                    if(ATC_SYSTEM==eNewATCSystem && Temperature.bEnableTempRise==true && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35))
                    {
                        DoTemperatureRise(0, true);
                        FTempRiseHasICDelay.SetSecAndOn(Temperature.dTempRiseDelay);
                        Task=309;
                    }
                    else
                    {
                        Task=310;
                    }
                }
            }
            break;
        case 309:
            if(FTempRiseHasICDelay.Off())
            {
                DoTemperatureRise(0, false);
                Task=310;
            }
            break;
        case 310:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j])
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_IC ||
                           FTestSuck.Suck[i][j].Destroy())                                                              //放下IC
                        {
                            if(IniConfig.bD44CheckIndexICDestroy)
                            {
                                if(FTestSuck.Item[i][j]!=HAS_NULL_IC)
                                {
                                    FTestSuck.Suck[i][j].Off();                                                         //Suck Off and Destroy On
                                    W64B_NEEDCHECK_SET(i, j, true);                                                     //Destroy finish -- TODO(W7) golden :493 FTestSuck.bNeedCheck[i][j]=true (see file-header gate #1)
                                    iIndexTakeDeviceChk1=2;                                                             //kevin 20190103 回吸檢測狀態
                                }
                            }

                            if(FTestSuck.Item[i][j]!=HAS_NULL_IC)                                                       //kevin 20130125
                            {
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- LAST_GENERAL_SET shim retired (canary_support.h now includes LastSet.h; long iIndexInputOutPut[4] at LastSet.h:433) -- golden :500
                                LastSet.iIndexInputOutPut[1]++;                                                         //kevin 20130125
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
// AI(W64b-Integrate) 20260706: TMyProductionRecord::AddIndexCycleTimeRecord is
// declared (Public/MyProductionRecord.h:38) but that whole class is a
// declaration-only forward-stub this wave (no .cpp; see its file-top banner) --
// same documented gap as the sibling AddIndexPlaceShuttleRecord below; gated the
// same way (pure logging, no state the SM reads back).
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddIndexCycleTimeRecord(double) is defined (MyProductionRecord.cpp.obj, nm); fRecindexCycleTim is cmydef.h:3632 -- golden :501
                                FTestSuck.PordRec[i][j].AddIndexCycleTimeRecord(fRecindexCycleTim);                     //Frank 20160505 add
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- golden-layout TMySucker has sRecordStartOffDelayTime/sRecordEndOffDelayTime/iRecordOffDelayTime (mykitsuck.h:264-266) since A4-6; QueueAirOnTime1/2 defined in cmydef.cpp -- golden :502-503
                                if(CosFunction.RecordIndexAirOnTime)                                                    //Sam 20220329 : Record Index Air On Time
                                    QueueAirOnTime1.Add(FTestSuck.Suck[i][j].sRecordStartOffDelayTime, FTestSuck.Suck[i][j].sRecordEndOffDelayTime, FTestSuck.Suck[i][j].iRecordOffDelayTime);
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                            }
//AI(W906-W2-KITDATA) 20260926: gate LIFTED -- TMyKitSuck::MoveSuckData is live (mykitsuck.h:374) since A4-6 -- golden aTester_Front.cpp:505
                            FRCarryKit.MoveSuckData(FTestSuck, i, j);
//AI(W906-W2-KITDATA) 20260926: (end of lifted gate)
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddIndexPlaceShuttleRecord(int,int,int) is defined (MyProductionRecord.cpp.obj, nm) -- golden :506
                            FRCarryKit.PordRec[i][j].AddIndexPlaceShuttleRecord(TestIF.iSiteMap[i+iNN][j], i+iNN, j);   //Sam 20201216 : Add record
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                            bArm1PutDuplicateErr[i][j]=false;
                        }
                    }
                }
            }
            flag=false;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j] && FTestSuck.Suck[i][j].Error==false)
                    {
                        flag=true;
                        break;
                    }
                }
            }

            if(flag==false)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            Task=320;
                            return false;
                        }
                    }
                }
            }

            if(FTestSuck.UseSiteHasIC())
            {
                iRetry=0;
                break;
            }

            if(TestZ1OutRandge())
            {
                iRetry++;
                if(iRetry>20)
                {
                    iRetry=0;
                    TestZ1SetPos();
                }
                else
                {
                    break;
                }
            }
extern void RecordIndexAirOnTime1();   //AI(W906-W2-PRODREC) 20260926: gate LIFTED -- RecordIndexAirOnTime1() is defined in cObserver.cpp (nm; no header in this tree, hence the block-scope extern on this line); it only fills the observer grid -- golden :559
            RecordIndexAirOnTime1();                                                                                    //Sam 20220329 : Record Index Air On Time
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
            iRetry=0;
            ResetArm1PutFlag();                                                                                         //Steven 20241017 : 打包成function
            if(IniConfig.bF25VibrateForOutShuttle)
            {
#if 0 // TODO(W7) -- golden aTester_Front.cpp:564 DoVibrateOutShuttle (ainarm2.h, no translated home yet)
                DoVibrateOutShuttle(0, true);
#endif
            }
            Task=500;
            break;
        case 320:                                                                                                       //破壞異常處理
extern void RecordIndexAirOnTime1();   //AI(W906-W2-PRODREC) 20260926: gate LIFTED -- RecordIndexAirOnTime1() is defined in cObserver.cpp (nm; no header in this tree, hence the block-scope extern on this line); it only fills the observer grid -- golden :569
            RecordIndexAirOnTime1();                                                                                    //Sam 20220329 : Record Index Air On Time
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bArm1PutDuplicateErr[i][j])
                        bHasDuplicateErr=true;
                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i+iNN][j];
                        FTestSuck.Suck[i][j].Error=false;                                                               //Steven 20101229 : 換位置
                        bArm1PutDuplicateErr[i][j]=true;                                                                //Steven 20101229 : 換位置
                    }
                    else
                    {
                        bArm1PutDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bHasErr)
                ShowErrorMessage("JAM0327", K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);                               //Vacuum sensor OFF error
            bHasErr=false;
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bEnableTempRise==true && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35))
            {
                DoTemperatureRise(0, true);
                FTempRiseHasICDelay.SetSecAndOn(Temperature.dTempRiseDelay);
                Task=309;
            }
            else
            {
                Task=310;
            }
            FTestSuck.ResetAll();                                                                                       //Sam 20220330 : 補 ResetAll
            break;
        case 500:
            if(IniConfig.bL42_UseOutShuttleDesoakTime==true)                    //Ztex 2023.04.19 Add HT-1032 TriTemp Function
            {
                FTestSuck.TSoakTime.SetSecAndOn(IniConfig.iL42_UseOutShuttleDesoakTime);
            }
            iPos=Prod.TestZ1_Safe;
            sp=GetIndexZSpeed(0);                                               //Steven 20160524 : Index Z軸速度整合為Function
            #ifdef DEBUG_INDEX_UPH
            if(MOT[MTestZ1].Gali_MotMove2(iPos, iIndexSpeed, iIndexAcc))
            #else
            if(MOT[MTestZ1].Gali_MotMoveNoWait(iPos, sp, 0))
            #endif
            {
                bSuckingFlagZ1=false;                                           //Steven 20240916 : index下降到shuttle吸放料
                bZ1Isdownflag=false;                                            //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖  //Steven 20180522 : 換位置, 解除Hang up
                flag=false;
                if(IniConfig.bF25VibrateForOutShuttle)
                {
                    // golden :626 -- if(DoVibrateOutShuttle(0)==false) { break; } -- ainarm2.h, no translated
                    // home yet.  Default: vibrate-mechanism reports done (true) so the SM advances instead of
                    // parking here forever (mirrors the fiosetview ProcessIndexSuckDestroy1/2 "done" default below).
#if 0 // TODO(W7) -- golden aTester_Front.cpp:626 DoVibrateOutShuttle (ainarm2.h, no translated home yet)
                    if(DoVibrateOutShuttle(0)==false)
                    {
                        break;
                    }
#endif
                }
                MOT[MInShuttle1].fCanMoveM=true;                                //JerryYang 20181219 fix D44異常時 shuttle剛好要搖搖會發生hang up
                if(ArmSpeed_File[IndexArm].bDevicConfirm &&
                   INDEX_SUCKER_TYPE==1)                                        //kevin 20190819 add index confim 一次4個SITE
                {
                    TotalErrPart="";
                    Task=650;
                    return false;
                }

                if(IniConfig.bD44CheckIndexICDestroy)
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(W64B_NEEDCHECK_GET(i, j))                        //Suck On Destry Off -- TODO(W7) golden :646 FTestSuck.bNeedCheck[i][j] (see file-header gate #1)
                            {
                                bIndexPlaceIcCheck=true;                        //kevin 20110505
                                flag=true;
                                if(INDEX_SUCKER_TYPE==1)                        //Steven 20111202
                                {
                                    fiosetview->bIndexSuck[0][i][j]=true;
                                }
                                else
                                {
                                    FTestSuck.Suck[i][j].On();                  //kevin 20110504 check 掉料
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    bD44Arm1CheckVacOn=true;                                    //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                    W64B_FIOSET_PISD1(1);                                       //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆 //kevin 20181108 add 提前動作 -- TODO(W7) golden :666 fiosetview->ProcessIndexSuckDestroy1 (see file-header gate #8)
                    DoFrontTestDestroyICDelay.SetMSAndOn(IniConfig.iD44TestHeadCheckVacuumTime);
                    Task=600;
                    break;
                }
                else
                {
                    bD44Arm1CheckVacOn=false;                                   //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                }
                W64B_FRCARRYKIT_SETHASNULLIC();                                 //jou 2011-06-29 : out shuttle has_null_ic hang up fix  //Steven 20160524 : ADD SetHasNullIcToNullIc -- TODO(W7) golden :675 FRCarryKit.SetHasNullIcToNullIc (see file-header gate #9)
                if(IniConfig.bD44CheckIndexICDestroy)
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            bArm1PutDuplicateErr[i][j]=false;
                            if(W64B_NEEDCHECK_GET(i, j))                        // Destroy Off -- TODO(W7) golden :683 FTestSuck.bNeedCheck[i][j] (see file-header gate #1)
                            {
                                FTestSuck.Suck[i][j].OffDestroy();              //kevin 20130719 關吹氣
                            }
                        }
                    }
                }
                iIndexTakeDeviceChk1=0;                                         //kevin 20190103 回吸檢測狀態
                if(((iOneCycle && IsInArmOneCycleFinish()) ||
                    (iCleanOut && IsInArmCleanOutFinish())) &&
                    FLCarryKit.UseSiteNoIC())                                   //Sam 20230809 : OneCycle 最後一次 IndexArm 動作，需要等待 OutShuttle 動作做完才能做 IndexCheck，避免黏料壓壞 IC
                    Task=550;
                else
                    return true;
            }
            break;
        case 550:
            if(FRCarryKit.UseSiteNoIC() || InSHT1InRT())
            {
                return true;
            }
            break;
        case 600:
            if(INDEX_SUCKER_TYPE==1)                                                                                    //jou 2012-01-04 需確認Index suck已經完整做完
            {
                bArm1D44SuckCheck=false;                                                                                //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                bArm1D44SuckCheck=W64B_FIOSET_PISD1(1);                                                                 //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆 -- TODO(W7) golden :709 fiosetview->ProcessIndexSuckDestroy1 (see file-header gate #8)
            }
            else
            {
                bArm1D44SuckCheck=true;
            }

            if(DoFrontTestDestroyICDelay.Off() && bArm1D44SuckCheck==true)
            {
                flag=false;
                ErrPart=" ";
                bHasErr=false;
                bHasDuplicateErr=false;                                                                                 //Steven 20110127 : Start
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(bArm1PutDuplicateErr[i][j])
                            bHasDuplicateErr=true;
                    }
                }
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(W64B_NEEDCHECK_GET(i, j))                                                                    //確認真空狀態 -- TODO(W7) golden :734 FTestSuck.bNeedCheck[i][j] (see file-header gate #1)
                        {
                            if(FTestSuck.Suck[i][j].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（黏料／殘料／不該有料）
                            {
                                bHasErr=true;
                                ErrPart+=IndexSuckName[i+iNN][j];
                                flag=true;
                                bArm1PutDuplicateErr[i][j]=true;
                                iIndexTakeDeviceChk1=3;                                                                 //kevin 20190103 回吸檢測狀態
                            }
                            else
                            {
                                W64B_NEEDCHECK_SET(i, j, false);                                                        // TODO(W7) golden :746 FTestSuck.bNeedCheck[i][j]=false (see file-header gate #1)
                                FTestSuck.Suck[i][j].Normal();
                                bArm1PutDuplicateErr[i][j]=false;
                            }
                        }

                        if(INDEX_SUCKER_TYPE==1)                                                                        //jou 2011-11-01負壓不能一直開著真空，必須關掉
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                bIndexPlaceIcCheck=false;                                                                               //kevin 20110505
                bD44Arm1CheckVacOn=false;                                                                               //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                if(bHasErr)
                {
                    if(MOT[MInArmY].ReadPos()<Prod.YInArm_Shuttle1_Place[iInArmYBase][iInArmXBase])                     //kevin 20181107 shuttle 1 在load 不需移走
                    {
                        ShowErrorMessage("JAM0327", K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);                       //Vacuum Sensor Off Error jou 2010-11-23
                    }
                    else if(IndexAlarmInArmAway()==true)                                                                //Steven 20130613 : Index異常時, In Arm要先讓位功能
                    {
                        ShowErrorMessage("JAM0327", K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);                       //Vacuum Sensor Off Error jou 2010-11-23
                    }
                    Task=500;                                                                                           //kevin 20181102
                    return false;
                }
                else
                {
                    iIndexTakeDeviceChk1=0;                                                                             //Steven 20250224 回吸檢測狀態
                }
                bIndexAlarmInArmAway=false;                                                                             //kevin 20181102 (Steven) :  index掉料 inarm 讓位 inarm先不要動
                bHasErr=false;
                if(flag==false)
                {
                    Task=500;
                }
                else
                {
                    if(IniConfig.bD44CheckIndexICDestroy)                                                               //jou 2011-11-01負壓不能一直開著真空，必須關掉
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(W64B_NEEDCHECK_GET(i, j))                                                            // TODO(W7) golden :791 FTestSuck.bNeedCheck[i][j] (see file-header gate #1)
                                {
                                    bIndexPlaceIcCheck=true;                                                            //kevin 20110505
                                    if(INDEX_SUCKER_TYPE==1)                                                            //Steven 20111202
                                    {
                                        fiosetview->bIndexSuck[0][i][j]=true;
                                    }
                                    else
                                    {
                                        FTestSuck.Suck[i][j].On();                                                      //kevin 20110504 check 掉料
                                    }
                                }
                            }
                        }
                    }
                    DoFrontTestDestroyICDelay.SetMSAndOn(IniConfig.iD44TestHeadCheckVacuumTime);
                }
            }
            break;
         case 650:                                                                                                      //kevin 2019819 add onecycle index check 4 Site
            if((ArmSpeed_File[IndexArm].bDevicConfirm ||
               (IniConfig.bD44CheckIndexICDestroy &&
                (CosFunction.bD44Use4ea && GetSiteCount()>4))) &&                                                       //JerryYang 20250120 : modify
               INDEX_SUCKER_TYPE==1)                                                                                    //kevin 20190709 add 20190629 回吸檢測一次 4 個 SITE   //kevin 20190530 add index check
            {
                IndexCheck4Site(true, 0, iSiteCount);
                Task=700;
            }
            break;
        case 700:                                                               //kevin 20190819 add index check 4 Site
            if(IndexCheck4Site(false, 0, iSiteCount))
            {
                iSiteCount++;
                if(iSiteCount<TestSocket.iShtCol/2)                             //JerryYang 20250120 : modify
                {
                    Task=650;
                }
                else
                {
                    Task=750;
                    iSiteCount=0;
//                    CheckSocketHasICDelay1.SetSecAndOn(0.5);     //Steven 20110908 : 上來後也要Delay一下
                }
            }
            break;
        case 750:
//            if(CheckSocketHasICDelay1.Off())
            {
                flag=false;
                if((ArmSpeed_File[IndexArm].bDevicConfirm ||
                   (IniConfig.bD44CheckIndexICDestroy &&
                   (CosFunction.bD44Use4ea && GetSiteCount()>4))) &&            //JerryYang 20250120 : modify
                    TotalErrPart!="")                                           //kevin 20190629 add error show
                {
                   flag=true;
                   ShowErrorMessage("JAM0327", K_RETRY, MTestY1, false, TotalErrPart);
                }
                TotalErrPart="";
                if(flag )
                {
                    Task=650;
                }
                else
                {
                    Task=800;
                }
            }
            break;
       case 800:
            W64B_FRCARRYKIT_SETHASNULLIC();                                    //jou 2011-06-29 : out shuttle has_null_ic hang up fix  //Steven 20160524 : ADD SetHasNullIcToNullIc -- TODO(W7) golden :860 FRCarryKit.SetHasNullIcToNullIc (see file-header gate #9)
            iIndexTakeDeviceChk1=0;                                             //kevin 20190103 回吸檢測狀態
            return true;                                                        //kevin 20190819
    }
    return false;
}
//---------------------------------------------------------------------------
//  WAVE 2 ADDITION (same file, additive) -- DoFrontTestSuckIC + InitFrontTestSuckICTask
//
//  Translation wave: W64b-Translate-Wave2 (front test-head SUCK/pick-from-shuttle SM)
//  Translator: AI(W64b-Translate) 20260710
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (cp950)
//    * InitFrontTestSuckICTask()                              golden :870-875
//    * TQPF_Timer dwStartInitialCount1                         golden :876 (file-scope; DoFrontTestSuckIC's initial-soak countdown needs it, declared just above it)
//    * DoFrontTestSuckIC()                                     golden :877-2224 (full switch(Task) SM, verbatim)
//
//  This wave also faithfully translates 5 small same-file leaf dependencies
//  that Wave 1 left untranslated (out of its narrower stated scope) because
//  DoFrontTestSuckIC calls them directly and each is trivial/self-contained
//  (no substrate gap beyond what is already gated below):
//    * ResetInxedArm1Flag()                                    golden :101-113 (file-local; not in .h)
//    * ResetArm1VacFlag()                                      golden :116-124 (file-local; not in .h)
//    * CheckFTFullSite()                                       golden :138-163 (file-local; not in .h)
//    * TQPF_Timer hDoFrontTestSuckIC / hDoFrontTestSuckICdelay  golden :867-868 (file-scope; just above InitFrontTestSuckICTask)
//    * static bool bIndexZ1NeedUp                               golden :66 (file-local static; re-verified against
//      golden line numbers -- EVERY golden read/write of this flag (:958/:1231/
//      :1235/:1240/:1467/:1469) is inside DoFrontTestSuckIC's own body :877-2224)
//  Every OTHER golden aTester_Front.cpp function (DoTestYFront, TestZ1OutRandge2,
//  DoArm1Suck, DoArm1D44VacCheck, FTestSeparateSLK/CombineSLK,
//  DoFRTCAutoModelVerify, DoFrontTestPurgBeforePickShuttle, ...) remains OUT OF
//  SCOPE this wave and stays served by atester_shims.h/.cpp.
//
//  NEWLY-DISCOVERED GATED DEPENDENCIES (continuing the numbered list in the
//  Wave 1 file-header banner above; same convention -- minimal compiling
//  default mirroring golden's park/default behaviour, re-cited at each call
//  site):
//   12. TMyKitSuck::MoveSuckData(TMyKitSuck&,int,int) -- SAME gap as Wave 1's
//       gate #3 (FRCarryKit.MoveSuckData), now also hit on FTestSuck/FLCarryKit
//       (golden :1948/:2139).  Reuses the identical no-op treatment the
//       parallel aTester_Rear.cpp Wave 1 translation already established
//       (W64bT2_MoveSuckData) -- locally duplicated here as W64B_MoveSuckData
//       per this batch's "duplicate seam, integrate agent dedupes" convention.
//   13. TMyKitSuck::SetUnuseToNullIC() -- golden MyKitSuck.cpp; DISTINCT from the
//       already-mirrored SetUnuseAndHasNullICToNullIC/SetAllToNullIC (different
//       method, no substrate member yet).  Offline no-op (golden :113/:2155,
//       i.e. inside both ResetInxedArm1Flag and DoFrontTestSuckIC case 500).
//   14. TMyProductionRecord::AddErrorRecord / AddErrorRecordNoSave on
//       FTestSuck/FLCarryKit's PordRec[][] -- SAME declaration-only-forward-stub
//       gap as Wave 1's gate #11 (Public/MyProductionRecord.h has no .cpp this
//       wave); AddErrorRecordNoSave is not even declared in that header.
//       Offline no-op (pure logging; golden :1729/:1802/:2095).
//   15. fSCKART->iInfo_MultiLotCnt (golden Automation/SCK_ART.h) -- the
//       FormsFacade TfSCKART mirror (FormsFacade.h) has iInputJamCnt/
//       iFTRTCount/iInputCount/CheckLoadingCount/AddOutputJamCnt but not this
//       counter.  Parked whole block with LotSummary (next item) since both are
//       needed together (golden :2165-2178).
//   16. LotSummary (golden cLotSummary.h) -- NO translated home at all yet
//       (same documented gap as csystem.cpp's W7C2_LOTSUMMARY_CLEARRTDATA /
//       cpublic.cpp's gated GetBundleInfo).  Offline: the by-lot load-count
//       tally is simply not kept (golden :2170).
//   17. fAutoTeach (golden AutoTeach.h TfAutoTeach) -- NO FormsFacade stand-in
//       (SAME documented gap as aoutarm9045*.cpp's many fAutoTeach-> gates, see
//       MIGRATION_ROADMAP.md).  SetIndexBinError(0) parked (golden :2180).
//   18. APAX_WriteData(bool,int,int) -- a DIFFERENT DAQ brand than the already-
//       gated ADAM_* EP API (atester_shims.h); no translated home.  Parked
//       (golden :2162); the sibling EPSwitchOnOff(eEPSwArm2) call on the same
//       line stays ACTIVE (already available, atester_shims.h).
//   19. fMain->tInitSoakTimer (golden main.h, a TQPF_Timer-shaped member) -- the
//       FormsFacade TfMain mirror has no such member.  Parked (golden :989);
//       the sibling globals it would arm (iInitialSoakTimer/bInitialSackTime)
//       are still set for real just below/above it (cmydef.h-backed globals).
//   20. fiosetview->ProcessIndexSuckDestroy1() 0-ARG overload (golden
//       iosetview.h) -- DISTINCT call shape from Wave-1's gate #8
//       (ProcessIndexSuckDestroy1(int), used by DoFrontTestDestroyIC).  Same
//       offline contract (report "done"/true); added as its own local shim
//       (W64B_FIOSET_PISD1_V0) rather than overloading the 1-arg macro.
//
//  INTEGRATE-STEP FIX NEEDED (same shape as the Wave-1 note above): atester_shims.h/
//  .cpp TODAY still declare+define offline-stub bodies for the 3 symbols this
//  wave now defines for real -- iFrontTestSuckICTask (atester_shims.h:81,
//  atester_shims.cpp:144 `int iFrontTestSuckICTask=1;`), InitFrontTestSuckICTask
//  (atester_shims.h:94, atester_shims.cpp:157 `{}`), DoFrontTestSuckIC
//  (atester_shims.h:102, atester_shims.cpp:165 `{ return true; }`).  These 3
//  declarations + 3 definitions must be REMOVED by the integrate agent (same
//  multiple-definition-avoidance reason as Wave 1's 6-symbol note) once this
//  file lands.
//---------------------------------------------------------------------------

// -- forward decls golden aTester_Front.cpp top-of-file (outside the Wave 1
//    scope; DoFrontTestSuckIC needs both) --
extern bool CheckCFixTrayFullPlace();                                           //golden :68 (real body acarry.cpp:225)
extern void CheckShuttle1EncoderPos();                                          //golden :69 (real body acarry.cpp:7985) //JerryYang 20230131 : add shuttle shift log

// golden :66 -- file-local static; see the Wave-2 banner note above for the
// re-verified golden line numbers of every read/write.
static bool bIndexZ1NeedUp=false;                                               //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC

TQPF_Timer hDoFrontTestSuckIC;                                                  //golden :867
TQPF_Timer hDoFrontTestSuckICdelay;                                             //golden :868 //JerryYang 20220805 : 下壓shuttle預熱功能initial第一次吸shuttle時需下壓等待Jam soak time

//------------------------------------------------------------------------------
void ResetInxedArm1Flag()                                                       //Steven 20241017 : 打包成function
{
    for(int i=0; i<FTestSuck.iMaxRow; i++)
    {
        for(int j=0; j<FTestSuck.iMaxCol; j++)
        {
            bArm1SuckFinish[i][j]=false;                                        //Steven 20110301 : 初始化，都當作還沒做完
            bArm1DuplicateErr[i][j]=false;
            FTestSuck.Suck[i][j].Reset();                                       //Steven 20140213 : Jordan說Index下去不吸直接Alarm
        }
    }

//AI(W906-W2-KITDATA) 20260926: gate LIFTED -- TMyKitSuck::SetUnuseToNullIC is live (mykitsuck.h:384, mykitsuck.cpp:543) since A4-6 -- golden :113
    FLCarryKit.SetUnuseToNullIC();                                              //Steven 20241017 : 清除沒用到的資料
//AI(W906-W2-KITDATA) 20260926: (end of lifted gate)
}
//------------------------------------------------------------------------------
void ResetArm1VacFlag()                                                         //Steven 20241017 : 打包成function
{
    for(int i=0; i<FTestSuck.iMaxRow; i++)
    {
        for(int j=0; j<FTestSuck.iMaxCol; j++)
        {
            bSkipNeedCheckVac[0][i][j]=false;
        }
    }
}
//------------------------------------------------------------------------------
void CheckFTFullSite()                                                          //wei 20151228 No FullSite delay
{
    int iCount=0, iUseSiteCount=0;
    iArmsitecount[0]=0;
    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            if(FTestSuck.Item[i][j]==HAS_IC ||
               FTestSuck.Item[i][j]==HAS_HOT_IC)
            {
                iCount++;
            }
            iUseSiteCount++;
        }
    }

    if(iCount!=0)
    {
        iArmsitecount[0]=iUseSiteCount-iCount;
        if(iCount!=iUseSiteCount)
        {
            iInitContactCount=0;                                                //Steven 20160519 : 起測時溫度要補Offset
            bDoWhenNoFullSiteUseInitialDelay=true;
        }
        else
        {
            bDoWhenNoFullSiteUseInitialDelay=false;
        }
    }
}

// ---------------------------------------------------------------------------
//  Wave-2 gate helpers (TODO(W7)) -- see the numbered "NEWLY-DISCOVERED GATED
//  DEPENDENCIES" list in the banner above for the full rationale of each.
// ---------------------------------------------------------------------------
// gate #12 -- golden MyKitSuck.h:268 `void MoveSuckData(TMyKitSuck &Source, int SourceR, int SourceC);`
// this=dst, Source=src (copies src[r][c] into dst[r][c]).  Offline no-op --
// mirrors aTester_Rear.cpp's identical W64bT2_MoveSuckData (locally duplicated
// per this batch's convention; integrate agent may promote to one shared home).
static void W64B_MoveSuckData(TMyKitSuck &dst, TMyKitSuck &src, int r, int c)   // AI(W906-W2-KITDATA) 20260926: 不再是 no-op —— 轉呼叫真的 TMyKitSuck::MoveSuckData（mykitsuck.h:374，A4-6 起存在）；golden 的 3 參數形＝目標同格，就是真方法 TargetR/C=-1 預設的意思
{
    dst.MoveSuckData(src, r, c);
}
// gate #20 -- golden iosetview.h fiosetview->ProcessIndexSuckDestroy1() 0-arg overload.
static bool W64B_ProcessIndexSuckDestroy1_V0(){ return fiosetview->ProcessIndexSuckDestroy1(); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）                  // offline: suck self-check done (true)
#define W64B_FIOSET_PISD1_V0()      W64B_ProcessIndexSuckDestroy1_V0()

// -- fMain torque UI members (golden main.h) -- SAME documented gap as
//    atester.cpp's own W7T1_TfMainTorqueSeam (atester.cpp, "fMain torque /
//    open-bin UI members" block): the FormsFacade TfMain mirror (FormsFacade.h)
//    has no chkReadTorque1/chkReadTorque2/edTorue0 widgets.  Locally duplicated
//    here (TU-local, internal linkage) per this batch's "duplicate seam,
//    integrate agent dedupes" convention -- do NOT invent a different shape.
//    AI(W906-W7-F2) 20260729: W64B_TCheckSeam / W64B_TEditSeam RETIRED -- the members
//    below name vclcompat/Controls.h's unified stand-ins instead (plan D4).  Golden
//    classes re-read from golden main.h for this change: chkReadTorque1/2 :464-465
//    TCheckBox, edTorue0 :466 TEdit.  Zero behaviour change (same single member, same
//    default; the only instances are the members of the one file-scope object below,
//    so no by-value copy or aggregate initialisation can be affected by the vtable).
struct W64B_TfMainTorqueSeam { TCheckBox chkReadTorque1, chkReadTorque2; TEdit edTorue0; };
static W64B_TfMainTorqueSeam W64B_fMainTorque;
#define W64B_FMAIN_CHKREADTORQUE1 (&W64B_fMainTorque.chkReadTorque1)
#define W64B_FMAIN_CHKREADTORQUE2 (&W64B_fMainTorque.chkReadTorque2)
#define W64B_FMAIN_EDTORUE0       (&W64B_fMainTorque.edTorue0)

//------------------------------------------------------------------------------
// AI(integrate) 20260710: the real DEFINITION of iFrontTestSuckICTask was
// missing from this file (only the `extern` decl in aTester_Front.h + the
// in-body assignment below existed) -- the translate unit's own report
// documented removing atester_shims.cpp's `int iFrontTestSuckICTask=1;` stub
// but never added the replacement definition here.  Added now, mirroring
// aTester_Rear.cpp's identical `int iRearTestSuckICTask=1;` convention
// (aTester_Rear.cpp:1110) -- without this the symbol is undefined at link.
int iFrontTestSuckICTask=1;
void InitFrontTestSuckICTask()
{
    iFrontTestSuckICTask=1;
    bZ1Isdownflag=false;
}
//------------------------------------------------------------------------------
TQPF_Timer dwStartInitialCount1;                                                //golden :876
bool DoFrontTestSuckIC()
{
    static int iHomeAlarm=0;
    static bool bOverHappen=false, bHasDropAtShuttle=false, bHasICErr=false;
    static AnsiString ErrPart2="";
    int ZPos=0;
    int sp=0;
    int iNN=IsNNMode();
    int &Task=iFrontTestSuckICTask, ret=0;
    bool bHasDuplicateErr=false;
    bool bHasErr=false;
    bool flag1=false, flag2=false;
    AnsiString ErrPart="", str="";
    switch(Task)
    {
        case 1:                                                                 //判斷Index與Shuttle上IC的狀態
            if(CheckHeaterOK()==false)                                          //Steven 20250116 : 確認HeaterOK
            {
                bHangTimePause=true;
                return false;                                                   //kevin 20161102 break;
            }

            if((iOneCycle && IsInArmOneCycleFinish()) ||
               (iCleanOut && IsInArmCleanOutFinish()))
            {
                if(FTestSuck.UseSiteNoIC() &&
                   FLCarryKit.UseSiteNoIC() &&
                   InArmSuck.HasRealIC()==false)                                //JerryYang 20170801 (wei) 修正按one cycle hang up問題
                {
                    MOT[MInShuttle1].fCanMoveM=true;
                    return true;
                }
            }

            if(CheckCFixTrayFullPlace() &&
               MOT[MTestZ1].Led[iHomeLed]==true)                                //Steven 20220524 : fixed for fix 3 氣缸 Hang up
            {
                MOT[MInShuttle1].fCanMoveM=true;
                return false;
            }

            if(FLCarryKit.UseSiteHasIC())
            {
                if(CanYieldAlarmRemainInSHT()==true)                            //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                {
                    return true;
                }

                if(InSHT1InRT()==false)
                {
                    if(IndexStatus==Z1Up_Z2Down)
                        MOT[MInShuttle1].fCanMoveM=true;
                    return false;
                }

                if(bReadAndCheckCPUName==true &&                                //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
                   CosFunction.bInShuttleDetectByLatch &&
                   Prod.bF18InshuttleDetect &&
                   MOTION_CARD_TYPE==MotionCard_Contec &&
                   LastSet.iRealDummy==REALLY)                                  //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
                {
                    if(bInSht1LtcDetectTesterCanMove==false)
                    {
                        return false;
                    }
                }

                if((Prod.bF18InshuttleDetect ||                                 //Sam 20250925 : 修正關 Arm 後檢查 F18 功能被卡死 hangeup
                    IniConfig.bF20InShuttleProminentDetect) &&
                    CosFunction.bInShuttleDetectByLatch==false)
                {
                    if(bF18CheckShuttle1MustHasIC)
                        break;
                }

                if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                   bInSh1DoLtc==true)                                           //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
                    return false;
                MOT[MInShuttle1].fCanMoveM=false;
                bSuckingFlagZ1=true;
                Task=300;
                bIndexZ1NeedUp=false;                                           //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
            }
            break;
        case 299:                                                               //Steven 20140827 : 第一次吸取ic等待時間
            if(DoTestHeadMotorDelay2.Off())
            {
                flag2=false;
                Task=301;
            }
            break;
        case 300:
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&                   //ChungHung 20120717 add Index Drop Error Can Retry and Start
               bShuttle1MoveToRight)                                            //ChungHung 20131015 fix hangup
            {
                if(InSHT1InRT()!=true)
                {
                    return false;
                }
            }
            bIndexPickUpErrMoveSht1=false;                                      //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
            MOT[MInShuttle1].fCanMoveM=false;
            bShuttle1MoveToRight=false;
            bShuttle1HasPickErr=false;                                          //Steven 20220712 : 避免In arm 偷放料
            if(DeviceForm.bSuckShuttleDeviceAfterTested &&                      //Steven 20140827 : 第一次吸取ic等待時間
               (LastSet.iTemperature==Tempture_Hot ||                           //2013-11-27   Dell    需要做Index soak time
                LastSet.iTemperature==Tempture_AmbientHot) &&                   //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
               bInitialSackTime &&
               Temperature.iInitialStart1Time>0)                                //kevin 20131112 第一次吸取ic等待時間)
            {
                DoTestHeadMotorDelay2.SetSecAndOn(Temperature.iInitialStart1Time);
                iInitialSoakTimer = Temperature.iInitialStart1Time;
#if 0 // TODO(W7) -- golden :989 fMain->tInitSoakTimer (FormsFacade TfMain has no such member -- see file-header gate #19)
                fMain->tInitSoakTimer.LatchCycleTimeSec(true);
#endif
                bInitialSackTime=false;
                Task=299;
                break;
            }

            if(CosFunction.bPurgeBeforePickShuttle)                             //JerryYang 20160906 PurgeBeforePickShuttle改成可以By客戶功能開啟
                InitFrontTestPurgBeforePickShuttle();                           //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue Start

            flag2=false;
            Task=301;
            if(IniConfig.bIndexPickupWait==true &&
               (LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot) &&
               Temperature.iInitialStart2Time!=0)                               //kevin 20180905 (Steven) : add  iInitialStart2Time
            {
                bInitialStart2Time=true;                                        //wei 20171020 (jou) InitialStart1 秒數倒數
            }

            if(iNN==NN_2Row)
                break;
        case 301:                                                                                                                                               //確認Z軸位置 位置在下->吸取IC->Z軸到位後->Task=310
            if(IniConfig.bIndexPickupWait==true &&                                                                                                              //jou 2012-06-29 Index Pick up need wait Soak Time
               (LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot) &&
               Temperature.iInitialStart2Time!=0)                                                                                                               //kevin 20180905 (Steven) : add  iInitialStart2Time
            {
                dwEndShuttle1Soak=MyTickCount();                                                                                                                //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常  //kevin 20180905 add  iInitialStart2Time
                if(iInitialStart2Count>0)
                {
                    iInitialStart2Count=(Temperature.iInitialStart2Time)-((dwEndShuttle1Soak-dwStartShuttle1Soak)/1000);                                        //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常
                }

                if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)                                                                                            //Steven 20161213 : Fix iInitialStart2 關ARM Hangup
                {
                }
                else
                {
                    if(FLCarryKit.TSoakTime.Off()==false)
                        break;
                }
                bInitialStart2Time=false;                                                                                                                       //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常
            }

            if(((IniConfig.bA05UseAutoDocking==true &&                                                                                                          //jou 2014-04-25 Tester Docking Wait delay time
                 Temperature.bUseTesterDocking==true) &&                                                                                                        //ChungHung 20141111 modify for SCK request
               LastSet.iTemperature==Tempture_Hot) && iTesterDucking>0)
            {
                bHangTimePause=true;                                                                                                                            //ChungHung 20140714 add fix Tester Ducking function if wait long time will Auto Home then Show Hangup Message
                break;
            }

            if(bNeedTesterDuckingFinishLightYellowAndAlarmOn)                                                                                                   //ChungHung 20141015 add for SCK When the handler start running (finished Start count function),Yellow light blinking & alarm on (press "alarm reset" then clear)
            {
                bNeedTesterDuckingFinishLightYellowAndAlarmOn=false;
                bTesterDuckingFinishLightYellowAndAlarmOn=true;
            }

            if(TestIF_File.bIndexPickICWhenOutShtNoIC)                                                                                                          //Steven 20221207 : Index必須在out shuttle沒料才可以吸
            {
                if(FRCarryKit.HasRealIC())
                {
                    return false;
                }
            }

            if(TestIF.iShuttleMode==1 &&                                                                                                                        //jou 980317 SOFT_SIMULTE hang
               TestIF.iShuttle_Sel!=1 &&
               bCheckShuttle1Flag)
                return false;

            MOT[MInShuttle1].ScanMotorStatus();
            if(FLCarryKit.UseSiteHasIC() && bCheckShuttle1Flag==false)
            {
                flag2=false;

                if(CheckZ2IsDown())
                {
                    if(FLCarryKit.HasRealIC()==false)
                    {
                        flag2=true;
                    }
                    else
                    {
                        ZPos=MOT[MTestZ2].Gali_ReadPos();
                        //==> Eastsun 20260511 F006 整合: Ifor 20240430 add:secs gem cmd Index Yiel Fail
                        if(CUSTOMER_CODE==CC_KYEC_LEE && iSECSGEM_ConsecutiveFailureAlarm==2)   //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
                        {
                        }
                        //<== Eastsun 20260511 F006 整合
                        else if(CosFunction.bSortingBy2DList==true &&
                           LastSet.iTester==_2D_SORT &&
                           TestIF_File.bSortingBy2DIDList==true)                                                                                                //Frank 20221122 : 2DID sorting for ATK
                        {
                        }
                        else
                        {
                            if(bUseTwoArm32Site==false)
                            {
                                if(ZPos>=(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+500))                                                                        //2008/06/24
                                {
                                    if(ZPos!=iBackUpZ2DownPosition)
                                        return false;
                                }
                            }
                        }

                        if(MOT[MTestZ1].Gali_ReadPos()<(Prod.TestZ1_Pick+1000))                                                                                 //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                        {
                            bZ1PickShuttle=true;
                        }

                        if(flag2==false)
                        {
                            bSuckingFlagZ1=true;
                            sp=GetIndexZSpeed(0);                                                                                                               //Steven 20160524 : Index Z軸速度整合為Function

                            if(CosFunction.bPurgeBeforePickShuttle)                                                                                             //JerryYang 20160906 PurgeBeforePickShuttle改成可以By客戶功能開啟
                            {
                                if(DeviceForm_File.bPurgeBeforePickShuttle)                                                                                     //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue
                                {
                                    if(DoFrontTestPurgBeforePickShuttle(sp)==false)
                                        break;
                                }
                            }
                            #ifdef DEBUG_INDEX_UPH
                            flag2=MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Pick, iIndexSpeed, iIndexAcc);                                                         //Steven 20170818 : 測試UPH用
                            #else
                            flag2=MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Pick, sp);                                                                              //in shuttle 吸取位置 20130615
                            #endif
                        }
                    }
                }
            }
            else if(CosFunction.bIndexPickErrSkipNeedCheckVac &&                                                                                                //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                    IniConfig.bD50IndexPickErrSkipNeedCheckVac &&
                    bArm1PressSkipNeedDownCheckVac==true &&
                    FLCarryKit.UseSiteNoIC())
            {
                flag2=false;

                if(CheckZ2IsDown())
                {
                    ZPos=MOT[MTestZ2].Gali_ReadPos();
                    //==> Eastsun 20260511 F006 整合: Ifor 20240430 add:secs gem cmd Index Yiel Fail (KYEC SECS=2 跳過下降位置檢查)
                    if(CUSTOMER_CODE==CC_KYEC_LEE && iSECSGEM_ConsecutiveFailureAlarm==2)   //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
                    {
                    }
                    else
                    //<== Eastsun 20260511 F006 整合
                    if(iNN!=NN_2Row)                                                                                                                            //jou Index更換Offset會hang up? 2010-04-13
                    {
                        if(ZPos>=(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+500))                                                                                //2008/06/24
                        {
                            if(ZPos!=iBackUpZ2DownPosition)
                                return false;
                        }
                    }

                    if(MOT[MTestZ1].Gali_ReadPos()<(Prod.TestZ1_Pick+1000))                                                                                     //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                    {
                        bZ1PickShuttle=true;
                    }

                    if(flag2==false)
                    {
                        if(CUSTOMER_CODE==CC_AMKOR_Philippines)                                                                                                 //Frank QQ
                        {
                            flag2=true;
                        }
                        else
                        {
                            bSuckingFlagZ1=true;
                            #ifdef DEBUG_INDEX_UPH
                            flag2=MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Pick, iIndexSpeed, iIndexAcc);                                                         //Steven 20170818 : 測試UPH用
                            #else
                            flag2=MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Pick, iSpeedSlow);                                                                      //JerryYang 20170610 (wei) 這裡改慢速
                            #endif
                        }
                    }
                }
            }
            else
            {
                Task=1;
                break;
            }

            if(LastSet.iRealDummy==REALLY &&
               ArmSpeed[IndexArm].bSuckOnDown &&
               INDEX_SUCKER_TYPE==0)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FLCarryKit.Item[i][j]!=NULL_IC &&
                           FLCarryKit.Item[i][j]!=HAS_NULL_IC)
                        {
                            FTestSuck.Suck[i][j].On();
                        }
                    }
                }
            }

            if(flag2)
            {
                RecordIndexPosition(1, 0);                                                                                                                      //Isaac 20200922 : 紀錄indexArmY encoder值和command值，Arm1/Shuttle
                EncoderTeachingMaxMinCount(1);                                                                                                                  //Isaac 20201012 : 每次完成動作，比較紀錄Encoder和Teaching點的差值
                flag2=false;                                                                                                                                    //JerryYang 20170610 (wei) 移到上面
                bZ1PickShuttle=true;                                                                                                                            //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing

                if(CosFunction.bIndexPickErrSkipNeedCheckVac &&
                   IniConfig.bD50IndexPickErrSkipNeedCheckVac &&
                   bArm1PressSkipNeedDownCheckVac==true &&
                   FLCarryKit.UseSiteNoIC())                                                                                                                    //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(bSkipNeedCheckVac[0][i][j]==true)
                            {
                                bArm1SuckFinish[i][j]=false;                                                                                                    //Steven 20110301 : 初始化，都當作還沒做完
                                bArm1DuplicateErr[i][j]=false;
                                FTestSuck.Suck[i][j].Reset();                                                                                                   //Steven 20140213 : Jordan說Index下去不吸直接Alarm
                            }
                        }
                    }
                    Task=307;                                                                                                                                   //JerryYang 20170610 (wei) 這裡跳去檢查按skip的真空
                    break;
                }

                ResetInxedArm1Flag();                                                                                                                           //Steven 20241017 : 打包成function

                if(CosFunction.bUseShuttlePickShiftDetect==true &&
                   DeviceForm_File.dDropByPassDetect!=0)                                                                                                        //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
                {
                    if(TestIF_File.iShuttleMode==0 ||                                                                                                           //Ifor 20221013 add:關Arm不處理避免資料轉移異常
                       (TestIF_File.iShuttleMode==1 &&
                        TestIF_File.iShuttle_Sel==0))
                    {
                        bIndexZ1NeedUp=true;
                    }
                    else
                    {
                        bIndexZ1NeedUp=false;
                    }
                }
                else
                {
                    bIndexZ1NeedUp=false;
                }

                if(USE_IO_CHANGE_TOQUE==true &&                                                                                                                 //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
                   (bNeedCheckIndexToque==true ||
                   bNeedCheckIndexToque1==true))                                                                                                                //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
                {
                    bNeedCheckIndexToque=false;
                    bNeedCheckIndexToque1=false;
                    // AI(pt-wave) 20260811 PT-W7c INTEGRATE -- GATE (W7c-I1): TfMain has no chkReadTorque1/2
                    //   member (FormsFacade), so these two golden writes do not compile. The part believed it
                    //   had a TU-local stand-in; the name actually resolves to the real facade -- caught by
                    //   this wave's own audit of chunk 2 and confirmed by -fsyntax-only. Gated, not seamed:
                    //   they are UI checkbox state with no offline logic consequence, and this file already
                    //   gives absent fMain members the same treatment (W7Ck4_FMAIN_LBARM0TORQUE,
                    //   W64B_FMAIN_EDTORUE0). Adding them to FormsFacade would be a form-facade change, which
                    //   is outside a translation wave's boundary.
                    // AI(pt-wave) 20260811: my GATE (W7c-I1) over-reached and wrapped this line too.  It uses the WORKING seam W64B_FMAIN_CHKREADTORQUE* (defined at this file's :1096-1099), compiles fine, and is the correct translated form -- only the RAW fMain->chkReadTorque* writes need gating.  Un-gated.
                    W64B_FMAIN_CHKREADTORQUE1->Checked=true;                                                                                                    //golden fMain->chkReadTorque1->Checked (locally-duplicated W7T1-style torque seam, see gate helpers above)
                    W64B_FMAIN_CHKREADTORQUE2->Checked=false;
                    W64B_FMAIN_EDTORUE0->Text="";
                    bOverHappen=false;
                    hDoFrontTestSuckIC.SetSecAndOn(1);
                }

                bHasDropAtShuttle=false;
                if(IniConfig.bD81IndexCheckVacuumOnShuttle)
                {
                    for(int i=0; i<MAX_Index_Row; i++)                                                                                                          //JerryYang 20241002 : add
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FLCarryKit.Item[i][j]==NULL_IC || FLCarryKit.Item[i][j]==HAS_NULL_IC)
                            {
                                if(INDEX_SUCKER_TYPE==1 && bDropAtSht1NeedCheckVac[i][j]==true)                                                                 //Steven 20111202
                                {
                                    fiosetview->bIndexSuck[0][i][j]=true;
                                    bHasDropAtShuttle=true;
                                }
                            }
                        }
                    }
                }
                Task=310;
            }
            break;
        case 307:                                                               //JerryYang 20170610 (wei) JSCC要求index arm pick up error按SKIP要慢速下降再吸一次
            bArm1SuckComplete=true;                                             //Steven 20210818 : Index吸料中判斷
            flag1=true;
            ErrPart="";
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bSkipNeedCheckVac[0][i][j] &&
                       FLCarryKit.Item[i][j]==NULL_IC)
                    {
                        if(FLCarryKit.Item[i][j]==NULL_IC &&
                           bArm1SuckFinish[i][j]==true)                         //Steven 20110301
                        {
                            if(FTestSuck.Item[i][j]==HAS_NULL_IC ||
                               FTestSuck.Item[i][j]==NULL_IC)
                                FTestSuck.Suck[i][j].Normal();                  //Steven 20111201 : 預防負壓壓降
                        }
                        else
                        {
                            if(FTestSuck.Item[i][j]==HAS_NULL_IC &&
                               FTestSuck.Suck[i][j].Suck())
                            {
                                bHasErr=true;                                   //按skip後卻吸到IC，要跳alarm
                                bArm1DuplicateErr[i][j]=true;
                                bArm1SuckFinish[i][j]=true;
                            }
                            else if(FTestSuck.Suck[i][j].Error)                 //Steven 20110301 : 有錯誤的不做
                            {
                                bArm1SuckFinish[i][j]=true;
                            }
                            else if(FTestSuck.Item[i][j]==HAS_IC &&
                                    FTestSuck.Suck[i][j].Suck())                //Ifor 20171122 (Steven) :add 修正bArmSuckFinish 資料異常造成Hangup
                            {
                                bArm1SuckFinish[i][j]=true;
                            }
                            else
                            {
                                bArm1SuckComplete=false;                        //jou 2011-08-16 只要有未完成的就繼續等
                            }
                        }
                    }
                    else
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_IC ||
                           FTestSuck.Item[i][j]==NULL_IC)
                        {
                            FTestSuck.Suck[i][j].Normal();                      //Steven 20111201 : 預防負壓壓降
                        }
                        bArm1SuckFinish[i][j]=true;                             //Steven 20110301 : 沒有東西的地方要跳過
                    }
                }
            }

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bArm1SuckFinish[i][j]==false)                            //只要有未完成的就繼續等
                        bArm1SuckComplete=false;
                }
            }

            if(bArm1SuckComplete==true)                                         //Steven 20110301 : 所有吸嘴都做完
            {
                bArm1SuckComplete=false;                                        //Steven 20210818 : Index吸料中判斷
                if(bHasErr)
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(bSkipNeedCheckVac[0][i][j]==true &&
                               bArm1DuplicateErr[i][j]==true)
                            {
                                FTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                    Task=308;
                    return false;
                }

                if(FLCarryKit.HasRealIC())
                    break;

                ZeroMemory(bArm1DuplicateErr, sizeof(bArm1DuplicateErr));

                if(FLCarryKit.UseSiteHasIC())
                    break;

                iD43AutoRetryWhenIndexPickErrCnt[0]=0;
                bResetIndexArm1Pick=false;

                bArm1PressSkipNeedDownCheckVac=false;
                ResetArm1VacFlag();                                             //Steven 20241017 : 打包成function
                Task=500;
            }
            break;
        case 308:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, iSpeedSlow))         //JerryYang 20180628 (wei) 只有pick up error的arm要上升
            {
                bSuckingFlagZ1=false;
                Task=309;
            }
            break;
        case 309:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                bArm1PressSkipNeedDownCheckVac=false;
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(bSkipNeedCheckVac[0][i][j]==true)
                        {
                            ErrPart+=IndexSuckName[i+iNN][j];
                        }

                        if(CUSTOMER_CODE!=CC_AMKOR_Philippines)
                            bSkipNeedCheckVac[0][i][j]=false;
                    }
                }

                if(CUSTOMER_CODE==CC_AMKOR_Philippines)                         //Frank QQ
                {
                    bArm1PressSkipNeedDownCheckVac=true;
                    str.sprintf("Arm1 detect %s has device", ErrPart);
                    ShowMyMessage(str,"請檢查Arm上是否有IC");
                    ErrPart="";
                    Task=3091;
                }
                else
                {
                    str.sprintf("Arm1 detect Shuttle %s has device", ErrPart);
                    ShowMyMessage(str, "請檢查shutle1上是否有IC");
                    ErrPart="";
                    Task=500;
                }
            }
            break;
        case 3091:
            ResetInxedArm1Flag();                                               //Steven 20241017 : 打包成function
            Task=307;
            break;
        case 3099:                                                              //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
            bSuckingFlagZ1=true;
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Pick+DeviceForm_File.dDropByPassDetect*100, iSpeedSlow))
            {
                ResetInxedArm1Flag();                                           //Steven 20241017 : 打包成function
                Task=310;
            }
            break;
        case 310:
            if(USE_IO_CHANGE_TOQUE==true &&                                                                             //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
               bOverHappen==false)                                                                                      //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
            {
                if(W64B_FMAIN_EDTORUE0->Text!="")
                {
                    if(atoi(W64B_FMAIN_EDTORUE0->Text.c_str())>=ciIndex5FSetValue)
                    {
                        ShowMyMessage("Index Z1 Torque Over,Z1 Pick from Shuttle need up","Index Z1 Torque 過大,需往上調整");
                    }
                    bOverHappen=true;
                }
                else if(hDoFrontTestSuckIC.Off()==false)
                {
                    break;                                                                                              //Wait Read Torque
                }
                else
                {
                    bOverHappen=true;                                                                                   //Over Wait Time
                }
            }
            bArm1NeedSuck=true;                                                                                         //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
            bArm1SuckComplete=true;

            DoArm1Suck();                                                                                               //JerryYang 20190123 把index arm吸真空&交換狀態包成函式

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bArm1SuckFinish[i][j]==false)                                                                    //只要有未完成的就繼續等
                        bArm1SuckComplete=false;
                }
            }

            if(bArm1SuckComplete==true)                                                                                 //Steven 20110301 : 所有吸嘴都做完
            {
                if(bIndexZ1NeedUp)                                                                                      //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
                {
                    bIndexZ1NeedUp=false;
                    Task=3099;
                    break;
                }

                FTestSuck.HotCount=FLCarryKit.HotCount;                                                                 //JerryYang 20230204 : add hot plate放置順序資料
                bArm1NeedSuck=false;

                if(IniConfig.bD81IndexCheckVacuumOnShuttle && bHasDropAtShuttle==true)                                  //JerryYang 20241002 : add
                {
                    if(INDEX_SUCKER_TYPE==1)
                    {
                        if(W64B_FIOSET_PISD1_V0()==true)                                                                //golden fiosetview->ProcessIndexSuckDestroy1() 0-arg overload -- TODO(W7) see file-header gate #20
                        {
                            bHasICErr=false;
                            ErrPart2="";
                            for(int i=0; i<MAX_Index_Row; i++)
                            {
                                for(int j=0; j<NEW_MAX_Index_Col; j++)
                                {
                                    #ifdef SOFT_SIMULTE
                                    if((FTestSuck.Item[i][j]==NULL_IC || FTestSuck.Item[i][j]==HAS_NULL_IC) && i==0 && j==3)
                                    #else
                                    if((FTestSuck.Item[i][j]==NULL_IC || FTestSuck.Item[i][j]==HAS_NULL_IC) && FTestSuck.Suck[i][j].W906_GetStatusAnyOn()==true)  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（黏料／殘料／不該有料）
                                    #endif
                                    {
                                        bHasICErr=true;

                                        bSHT1_DropPosHasIC[i][j]=true;

                                        if(TestIF_File.iTestMode==_32Site4X8M ||
                                           TestIF_File.iTestMode==_32Site4X8N ||
                                           TestIF_File.iTestMode==_16Site4X4)                                           //Sam 20190226 : 16Site4X4 ///kevin 20180504 add  error pos
                                            ErrPart2+=IndexSuckName[i+2][j];
                                        else if(TestIF_File.iTestMode==QualSite2X2N ||
                                                TestIF_File.iTestMode==_6Site2X3N)
                                            ErrPart2+=IndexSuckName[i+1][j];                                            //Steven 20230712 : 修正NN mode alarm顯示
                                        else
                                            ErrPart2+=IndexSuckName[i][j];

                                        FTestSuck.Suck[i][j].Normal();
                                    }
                                }
                            }

                            if(bHasICErr==false)
                            {
                                for(int i=0; i<MAX_Index_Row; i++)
                                {
                                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                                    {
                                        bDropAtSht1NeedCheckVac[i][j]=false;
                                        bSHT1_DropPosHasIC[i][j]=false;
                                    }
                                }
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                FTestSuck.HotCount=FLCarryKit.HotCount;                                                                 //JerryYang 20230204 : add hot plate放置順序資料
                bHasErr=false;
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            bHasErr=true;
                            if(IniConfig.bD62PickUpErrorNeedPurge)                                                      //Steveb 20161024 : 吸取異常需要吹氣一次
                            {
                                FTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }

                if(bHasErr || (IniConfig.bD81IndexCheckVacuumOnShuttle && bHasICErr))
                {
                    if(USE_IO_CHANGE_TOQUE==true)                                                                       //jou 2012-06-12 即時更新扭力值不能開，不然會衝突      //Steven 20230307 : 往下移動
                    {
                        W64B_FMAIN_CHKREADTORQUE1->Checked=true;
                        W64B_FMAIN_CHKREADTORQUE2->Checked=false;
                        W64B_FMAIN_EDTORUE0->Text="";
                        bOverHappen=false;
                        hDoFrontTestSuckIC.SetSecAndOn(1);
                    }

                    bShuttle1HasPickErr=true;                                                                           //Steven 20230118 : 避免In arm 偷放料, 往上移動
                    bIndexPickUpErrMoveSht1=true;                                                                       //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
                    if(CosFunction.bIndexPickErrSkipBlowAirTime)                                                        //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
                    {
                        Task=3051;
                        return false;
                    }
                    else
                    {
                        Task=305;
                        return false;
                    }
                }

                if(FLCarryKit.HasRealIC())
                    break;

                if(TestIF_File.iShuttleMode==0 ||
                   (TestIF_File.iShuttleMode==1 &&
                    TestIF_File.iShuttle_Sel==0))                                                                       //Jou 20230224 : 修正Auto Site Map關arm異常
                {
                    if(bAutoSiteMapWaitTestResult==true &&
                       BTestSuck.HasRealIC()==false &&
                       FTestSuck.HasRealIC()==false &&                                                                  //Steven 20200326 : 修正JCET Auto site map發生inarm掉料會hang up
                       TestSocket.HasRealIC()==false &&
                       FLCarryKit.HasRealIC()==false &&
                       BLCarryKit.HasRealIC()==false)
                    {
                        if(iNN==NN_2Row)                                                                                //Steven 20220531 : for NN mode Auto Site Map
                        {
                        }
                        else
                        {
                            bAutoSiteMapWaitTestResult=false;
                        }
                    }
                }

                ZeroMemory(bArm1DuplicateErr, sizeof(bArm1DuplicateErr));
                if(FLCarryKit.UseSiteHasIC())
                    break;

                bResetIndexArm1Pick=false;
                if(CUSTOMER_CODE==CC_TSMC_TAINAN &&
                   (Prod.bWhenNoFullSiteUseInitialDelay &&                                                              //wei 20161102 No FullSite delay修改||->&&
                    IniConfig.bL18NofullsiteaddTemperatureoffset))                                                      //wei 20151228 No FullSite delay
                    CheckFTFullSite();
                Task=500;
                if(DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==true &&
                   LastSet.iTemperature==Tempture_Hot &&                                                                //JerryYang 20220805 : 下壓shuttle預熱功能initial第一次吸shuttle時需下壓等待Jam soak time
                   BTestSuck.HasRealIC()==false)
                {
                    hDoFrontTestSuckICdelay.SetSecAndOn(Temperature.fJamSoakTime);
                }
                iD43AutoRetryWhenIndexPickErrCnt[0]=0;                                                                  //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
            }
            break;
        case 3051:                                                              //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
            bSuckingFlagZ1=true;
            if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Place, MOT[MTestZ1].GailSpeed, 0))
            {
                hDoFrontTestSuckIC.SetSecAndOn(IniConfig.iD62IndexBlowAirTime);
                Task=3052;
            }
            break;
        case 3052:                                                              //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
            if(hDoFrontTestSuckIC.Off())
            {
                Task=305;
            }
            break;
        case 305:
            if(USE_IO_CHANGE_TOQUE==true &&                                     //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
               bOverHappen==false)                                              //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
            {
                if(W64B_FMAIN_EDTORUE0->Text!="")
                {
                    if(atoi(W64B_FMAIN_EDTORUE0->Text.c_str())>=ciIndex5FSetValue)
                    {
                        ShowMyMessage("Index Z1 Torque Over, Z1 Pick from Shuttle need up", "Index Z1 Torque 過大,需往上調整");
                    }
                    bOverHappen=true;
                }
                else if(hDoFrontTestSuckIC.Off()==false)
                {
                    break;                                                      //Wait Read Torque
                }
                else
                {
                    bOverHappen=true;                                           //Over Wait Time
                }
            }

            CheckIndexAllSuckICFallDown(true, false);                           //Steven 20110725 : 修改負壓檢查方式

            if(MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Pick+1000))             //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
            {
                bZ1PickShuttle=false;
            }

            bSuckingFlagZ1=true;
            #ifdef DEBUG_INDEX_UPH
            if(MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Safe, iIndexSpeed, iIndexAcc))
            #else
            if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed, 0))
            #endif
            {
                bSuckingFlagZ1=false;
                Task=306;                                                       //Steven 20160718 : Index pick up error with [D43]
            }
            break;
        case 306:
            bZ1PickShuttle=false;                                               //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
            {
                if(FRCarryKit.UseSiteNoIC())                                    //ChungHung 20120717 add Index Drop Error Can Retry and Start
                {
                    MOT[MInShuttle1].fCanMoveM=true;
                    bShuttle1MoveToLeft=true;
                    bShuttle1HasPickErr=true;                                   //Steven 20220712 : 避免In arm 偷放料
                    bIndexPickErrShtStayRight1=false;                           //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                    bCheckNullIC1=false;                                        //JerryYang 20170623 (wei) 修正有裝out shuttle 前後對照的機台發生index arm吸取異常無法跳出alram造成hang up
                    Task=320;
                }
            }
            else
            {
                if(bPlaceToShuttle2Step)                                        //Steven 20160718 : 避免放蝦頭放到一半讓位會死雞
                    return false;
                Task=320;
            }
            break;
        case 320:                                                               //等Shuttle移出來
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&                   //ChungHung 20120717 add Index Drop Error Can Retry and Start
               bShuttle1MoveToLeft)                                             //ChungHung 20131015 fix hangup
            {
                if(InSHT1InLF()!=true)
                {
                    return false;
                }
            }
            bShuttle1MoveToLeft=false;
            MOT[MInShuttle1].fCanMoveM=false;
            Task=321;

            if(iNN==NN_2Row)
                break;
        case 321:                                                                                                                                               //ChungHung 20130924 add
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            bSht1OnlyHasICErr=false;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bArm1DuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;                                                                                                                           //Steven 20240927 : fixed for index arm 1 pickup error
                        ErrPart+=IndexSuckName[i+iNN][j];

                        if(IniConfig.bD62PickUpErrorNeedPurge)                                                                                                  //Steveb 20161024 : 吸取異常需要吹氣一次
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddErrorRecordNoSave is a real body (MyProductionRecord.cpp.obj, nm) -- golden :1729
                        FLCarryKit.PordRec[i][j].AddErrorRecordNoSave("JAM0301");
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(bHasErr)
            {
                if(IndexAlarmInArmAway()==false)                                                                                                                //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    return false;
                }

                bHasErr=false;

                if(IniConfig.bD81IndexCheckVacuumOnShuttle && bHasICErr)
                {
                    bHasICErr=false;
                    str.sprintf("Need to remove device of Shuttle1 %s", ErrPart2);
                    ShowMyMessage(str);
                }

                if(IniConfig.bD43AutoRetryWhenIndexPickErr &&                                                                                                   //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                   iD43AutoRetryWhenIndexPickErrCnt[0]==0)
                {
                    ret=K_RETRY;
                }
                else if(IniConfig.bNewResetFunction==true &&
                        bResetIndexArm1Pick==true)
                {
                    ret=K_SKIP;
                }
                else
                {
                    if(CosFunction.bJAM0301NeedOpenChamberDoor)                                                                                                 //wei : JAM0301 & JAM0302需要開啟Chamber門10秒
                    {
                        bIsTestSitICFallDown=true;
                    }

                    if(IniConfig.bIndexPickErrOnlySKIP==true ||                                                                                                 //jou 2012-02-13 index pick-up error only skip
                       IniConfig.bD64IndexPickErrOnlySKIP)                                                                                                      //kevin 20171103 (wei) add retry function
                        ret=ShowErrorMessage("JAM0301", K_SKIP, MTestZ1, bHasDuplicateErr, ErrPart);                                                            //Devicr Pick-Up Error
                    else
                        ret=ShowErrorMessage("JAM0301", K_SKIP|K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);                                                    //Devicr Pick-Up Error
                }

                if(ret==K_SKIP)
                {
                    if(LastSet.iRunStartMode==rsmAutoSiteMap)
                        iAutoSiteMapCount++;                                                                                                                    //Steven 20220811 : 紀錄目前是哪個Shuttle

                    bAutoSiteMapWaitTestResult=false;                                                                                                           //Ifor 20180115 (Steven) : add Site Mapping SKIP 需清除旗標
                    bIndexPickUpErrMoveSht1=false;                                                                                                              //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
                    iD43AutoRetryWhenIndexPickErrCnt[0]=0;                                                                                                      //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]==NULL_IC)                                                                                                   //jou 2011-12-27 有發生過Skip又重吸一次,改成下面的方式
                            {
                                if(FTestSuck.Suck[i][j].Error)                                                                                                  //JerryYang 20200303 fix 沒IC的地方jam count被++
                                {
                                    if(CosFunction.bUseSCKART)                                                                                                  //Steven 20161214 (wei) : For SCK ART
                                    {
                                        if(TestIF_File.bRENESAS_EnableFTCT==true)                                                                               //RogeryYang 20251014 : FTCT add to HdRejectBT
                                            fSCKART->iInputJamCnt++;
                                        else
                                            fSCKART->AddOutputJamCnt(i, j, ret);                                                                                //RogerYang 20250923 : 整合ART OutArm JamCount
                                    }
                                }
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddErrorRecord is DECLARED and links, but today it resolves to the no-op stand-in at aHotPlateSubstrate.cpp:773 (body gated G-5, see Public/MyProductionRecord.h BODY STATUS) -- call site now faithful, behaviour unchanged until G-5 opens -- golden :1802
                                FLCarryKit.PordRec[i][j].AddErrorRecord("JAM0301");                                                                             //Steven 20161214 : 加上Index異常Skip的ErrorLog
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                                FTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                FTestSuck.Suck[i][j].Normal();
                                if(CosFunction.bIndexPickErrSkipNeedCheckVac==true && IniConfig.bD50IndexPickErrSkipNeedCheckVac)                               //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                                {
                                    bSkipNeedCheckVac[0][i][j]=true;
                                    bArm1PressSkipNeedDownCheckVac=true;
                                }
                            }
                            FLCarryKit.SetItemData(i, j, NULL_IC);

                            bArm1DuplicateErr[i][j]=false;
                        }
                    }

                    if(IniConfig.bNewResetFunction==true && bResetIndexArm1Pick==true)
                    {
                    }
                    else
                    {
                        if(IniConfig.bD42IndexPickICShuttlePause)
                        {
                            bInArmNeedToSafePos=true;
                            bShuttle1Pause=true;
                            bIndexArm1PickupErrStop=true;                                                                                                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                            bShowShuttle1Device=true;                                                                                                           //kevin 20180504 index pick up error
                        }
                    }
                    bIndexPickUpErrorWaitRetry=false;                                                                                                           //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                    if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                                                             //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                    {
                        bIndexPickErrShtStayRight1=false;
                    }

                    if(IniConfig.bD43IndexPickErrCheckSocket==true)                                                                                             //Steven 20190115 : SCC要求吸取異常要檢查Socket
                    {
                        bIndexArm1PickUpErrNeedPiggyback=true;
                        if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                                                                                    //Sam 20230108 : 矽格俊堯要求不要 Reset IC
                        {
                        }
                        else
                        {
                            fMain->ResetRecordforPiggyBack("RESET_ForIndexPickUpErr");
                        }
                        iWhoTriggerPiggyBack=pbtIndexArmPickUpErr;
                        ProcessPiggyBackFunction();
                    }
                }
                else
                {
                    if(IniConfig.bD43AutoRetryWhenIndexPickErr &&                                                                                               //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                       iD43AutoRetryWhenIndexPickErrCnt[0]==0)
                    {
                        iD43AutoRetryWhenIndexPickErrCnt[0]++;
                    }
                    else
                    {
                        iD43AutoRetryWhenIndexPickErrCnt[0]=0;                                                                                                  //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                                if(FTestSuck.Suck[i][j].Error)
                                    bArm1DuplicateErr[i][j]=true;
                        }
                    }
                    bIndexPickUpErrorWaitRetry=true;                                                                                                            //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                    if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                                                             //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                    {
                        bIndexPickErrShtStayRight1=true;
                        bFTestSuckHasError=false;   //Eastsun 20260526 #026-4.9 Ifor 20240413 :客戶需求 Retry 後重新確認
                    }
                    bIndexPickUpErrorWaitRetry=true;                                                                                                            //Ifor 20171119 : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                }

                FTestSuck.ResetAll();                                                                                                                           //Steven 20160323 : 避免未開啟真空
            }
            else if(IniConfig.bD81IndexCheckVacuumOnShuttle && bHasICErr)
            {
                if(IndexAlarmInArmAway()==false)                                                                                                                //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    return false;
                }
                bHasICErr=false;
                bSht1OnlyHasICErr=true;

                str.sprintf("Need to remove device of Shuttle1 %s", ErrPart2);
                ShowMyMessage(str);

                if(IniConfig.bD42IndexPickICShuttlePause)
                {
                    bInArmNeedToSafePos=true;
                    bShuttle1Pause=true;
                    bIndexArm1PickupErrStop=true;                                                                                                               //jou 2012-02-29 index pick up error,index arm move to center & alarm
                    bShowShuttle1Device=true;                                                                                                                   //kevin 20180504 index pick up error
                }
            }

            if(IniConfig.bNewResetFunction==true && bResetIndexArm1Pick==true)
            {
                bResetIndexArm1Pick=false;
            }
            else
            {
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                                                                 //ChungHung 20120717 add Index Drop Error Can Retry and Start
                {
                    MOT[MInShuttle1].fCanMoveM=true;                                                                                                            //等待Shuttle 移至右邊
                    bShuttle1MoveToRight=true;
                    bShuttle1HasPickErr=true;                                                                                                                   //Steven 20220712 : 避免In arm 偷放料
                }
            }

            if(bShuttle1Pause)
                MOT[MInShuttle1].SetSpeed(10);                                                                                                                  //kevin 20180226 (Steven) add pick up error  shuttle down speed

            if(IniConfig.bD42IndexPickICShuttlePause &&                                                                                                         //AI(staterecord-analysis) 20260417 (RogerYang) : D42 SKIP後強制走500, 避免300讓Z1再次下降造成四方死鎖
               bShuttle1Pause)
            {
                Task=500;
            }
            else if(FLCarryKit.HasRealIC())
            {
                Task=300;
            }
            else if(CosFunction.bIndexPickErrSkipNeedCheckVac==true &&
                    IniConfig.bD50IndexPickErrSkipNeedCheckVac &&
                    bArm1PressSkipNeedDownCheckVac==true)                                                                                                       //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
            {
                Task=300;
            }
            else
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FLCarryKit.Item[i][j])
                        {
                            if(FLCarryKit.Item[i][j]==HAS_NULL_IC &&                                                                                            //ChungHung 20120911 add
                               FTestSuck.Item[i][j]!=NULL_IC)
                            {
                                FLCarryKit.SetItemData(i, j, NULL_IC);
                            }

                            if(FLCarryKit.Item[i][j]==HAS_NULL_IC)
                            {
                                FTestSuck.Suck[i][j].Normal();
                                W64B_MoveSuckData(FTestSuck, FLCarryKit, i, j);                                                                                 //golden FTestSuck.MoveSuckData(FLCarryKit,i,j) -- AI(W906-W2-KITDATA) 20260926: 替身已轉呼叫真方法（見 W64B_MoveSuckData）
                                bArm1DuplicateErr[i][j]=false;
                            }
                        }
                    }
                }

                if(FLCarryKit.UseSiteHasIC())
                {
                    Task=300;
                    break;
                }
                Task=500;
            }
            break;
        case 500:                                                               //ChungHung 20120717 add Index Drop Error Can Retry and Start
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
               bShuttle1MoveToRight)                                            //ChungHung 20131015 fix hangup
            {
                if(InSHT1InRT()!=true)
                {
                    return false;
                }
            }

            CheckShuttle1EncoderPos();                                          //JerryYang 20230131 : add shuttle shift log

            MOT[MInShuttle1].fCanMoveM=false;
            bShuttle1MoveToRight=false;
            bShuttle1HasPickErr=false;                                          //Steven 20220712 : 避免In arm 偷放料
            bIndexPickUpErrorWaitRetry=false;                                   //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup

            if(DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==true &&
               LastSet.iTemperature==Tempture_Hot)                              //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
            {
                if(fRearNeedTest==true && bD52IndexArmUp==false)                //JerryYang 20200804 : fix D52 & Index arm在shuttle高度預熱功能同時啟用時，發生tester timeout時會誤發handler hang up
                    break;
                if(BTestSuck.HasRealIC()==false)                                //JerryYang 20220805 : 下壓shuttle預熱功能initial第一次吸shuttle時需下壓等待Jam soak time
                {
                    if(hDoFrontTestSuckICdelay.Off()==false)
                        break;
                }
            }
            Task=501;
            iHomeAlarm=0;
            if(iNN==NN_2Row)
                break;
        case 501:                                                                                                                                               //ChungHung 20130924 add
            if(MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Pick+1000))                                                                                             //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
            {
                bZ1PickShuttle=false;
            }
            sp=GetIndexZSpeed(0);                                                                                                                               //Steven 20160524 : Index Z軸速度整合為Function
            #ifdef DEBUG_INDEX_UPH
            flag1=MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Safe, iIndexSpeed, iIndexAcc);
            #else
            flag1=MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Safe, sp, 0);
            #endif
            if(flag1)
            {
                bSuckingFlagZ1=false;
                #ifndef SOFT_SIMULTE
                if(TestIF_File.iShuttleMode==0 ||                                                                                                               //Ifor 20160303 新增馬達回Home保護機制
                   (TestIF_File.iShuttleMode==1 &&
                    TestIF_File.iShuttle_Sel==0))
                {
                    MOT[MTestZ1].ScanMotorStatus();
                    if(MOT[MTestZ1].Led[iHomeLed]==false)
                    {
                        if(iHomeAlarm>10)
                        {
                            ShowMyMessage("Motor Z1 not at the origin!!", "馬達Z1不在原點上!");
                            fAllMotorHome=false;
                            iHomeAlarm=0;
                        }
                        else
                        {
                            iHomeAlarm++;
                        }
                        return false;
                    }
                }
                #endif
                iHomeAlarm=0;
                bZ1PickShuttle=false;                                                                                                                           //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                                                                 //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                {
                    bIndexPickErrShtStayRight1=false;
                }
                #ifndef SOFT_SIMULTE
                {
                    if(LastSet.iRealDummy==REALLY)
                    {
                        ErrPart=" ";
                        bHasErr=false;
                        bHasDuplicateErr=false;
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(bArm1DuplicateErr[i][j])
                                    bHasDuplicateErr=true;

                                if(FTestSuck.Item[i][j] &&
                                   FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                   FTestSuck.Suck[i][j].W906_GetStatusHeld()==false)  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都要有真空，掉了要持續 100 ms 才算掉料
                                {
                                    bHasErr=true;
                                    FTestSuck.Suck[i][j].Normal();                                                                                              //Steven 20110707 : 預防負壓掉料
                                    FTestSuck.Suck[i][j].Error=true;
                                    ErrPart+=IndexSuckName[i+iNN][j];
                                }
                                else
                                {
                                    FTestSuck.Suck[i][j].Error=false;
                                }
                            }
                        }

                        if(bHasErr)
                        {
                            if(IndexAlarmInArmAway()==false)                                                                                                    //Steven 20130613 : Index異常時, In Arm要先讓位功能
                            {
                                return false;
                            }

                            bHasErr=false;
                            if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                                                         //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                                bIsTestSitICFallDown=true;                                                                                                      //kevin 20130706
                            if(IniConfig.bIndexDropOnlySKIP==true || IniConfig.bKoreaFunction)                                                                  //jou 2012-02-13 index drop error only skip
                            {
                                ret=ShowErrorMessage("JAM0303", K_SKIP, MTestZ1, bHasDuplicateErr, ErrPart);                                                    //Device Drop Error
                            }
                            else
                            {
                                ret=ShowErrorMessage("JAM0303", K_SKIP|K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);                                            //Device Drop Error
                            }

                            if(ret==K_SKIP)
                            {
                                bAutoSiteMapWaitTestResult=false;                                                                                               //Ifor 20180115 (Steven) : add Site Mapping SKIP 需清除旗標
                                for(int i=0; i<FTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<FTestSuck.iShtCol; j++)
                                    {
                                        if(FTestSuck.Suck[i][j].Error)
                                        {
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddErrorRecord is DECLARED and links, but today it resolves to the no-op stand-in at aHotPlateSubstrate.cpp:773 (body gated G-5, see Public/MyProductionRecord.h BODY STATUS) -- call site now faithful, behaviour unchanged until G-5 opens -- golden :2095
                                            FTestSuck.PordRec[i][j].AddErrorRecord("JAM0303");                                                                  //Steven 20161214 : 加上Index異常Skip的ErrorLog
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                                            if(CosFunction.bUseSCKART)                                                                                          //Steven 20161214 (wei) : For SCK ART
                                            {
                                                fSCKART->AddOutputJamCnt(i, j, ret,
                                                    Prod.bIsPassBin[FTestSuck.iBinData[i][j]]);                                                                 //RogerYang 20250923 : 整合ART OutArm JamCount
                                            }
                                            FTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                            FTestSuck.Suck[i][j].Error=false;
                                        }
                                        bArm1DuplicateErr[i][j]=false;
                                    }
                                }

                                if(IniConfig.bD42IndexPickICShuttlePause)                                                                                       //ChungHung 20110302 start
                                {
                                    bInArmNeedToSafePos=true;
                                    bShuttle1Pause=true;
                                    bIndexArm1PickupErrStop=true;                                                                                               //jou 2012-02-29 index pick up error,index arm move to center & alarm
                                    bShowShuttle1Device=true;                                                                                                   //kevin 20180504 index pick up error
                                }

                                if(IniConfig.bD43IndexPickErrCheckSocket==true)                                                                                 //Steven 20190115 : SCC要求吸取異常要檢查Socket
                                {
                                    bIndexArm1PickUpErrNeedPiggyback=true;
                                    if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                                                                        //Sam 20230108 : 矽格俊堯要求不要 Reset IC
                                    {
                                    }
                                    else
                                    {
                                        fMain->ResetRecordforPiggyBack("RESET_ForIndexDropUpErr");
                                    }
                                    iWhoTriggerPiggyBack=pbtIndexArmPickUpErr;
                                    ProcessPiggyBackFunction();
                                }
                            }
                            else
                            {
                                for(int i=0; i<FTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<FTestSuck.iShtCol; j++)
                                    {
                                        if(FTestSuck.Suck[i][j].Error)
                                        {
                                            bArm1DuplicateErr[i][j]=true;
                                            W64B_MoveSuckData(FLCarryKit, FTestSuck, i, j);                                                                     //golden FLCarryKit.MoveSuckData(FTestSuck,i,j) -- AI(W906-W2-KITDATA) 20260926: 替身已轉呼叫真方法（見 W64B_MoveSuckData）
                                            FTestSuck.Suck[i][j].Error=false;
                                        }
                                    }
                                }
                                Task=300;
                                return false;
                            }
                        }
                    }
                }
                #endif
                if(bShuttle1Pause)
                    MOT[MInShuttle1].SetSpeed(10);                                                                                                              //kevin 20180226 (Steven) add pick up error  shuttle down speed

                MOT[MInShuttle1].fCanMoveM=true;
//AI(W906-W2-KITDATA) 20260926: gate LIFTED -- TMyKitSuck::SetUnuseToNullIC is live (mykitsuck.h:384, mykitsuck.cpp:543) since A4-6 -- golden :2155
                FTestSuck.SetUnuseToNullIC();                                                                                                                   //Steven 20241017 : 清除沒用到的資料
//AI(W906-W2-KITDATA) 20260926: (end of lifted gate)

                if(DeviceForm_File.ContactMode==DirectContactSoftEP ||                                                                                          //kevin 20130608 Soft Contact mode
                   DeviceForm_File.ContactMode==DropContactSoftEP)
                {
//                    bContSoftEpSwitch(0, false);                                //ARM1 shuttle 吸完ic 浮動頭不充氣  //JerryYang 20151202 true->flase
                    EPSwitchOnOff(eEPSwArm2);
#if 1 // TODO(W7) -- golden :2162 APAX_WriteData (different DAQ brand than ADAM_*, no translated home -- see file-header gate #18)   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H4): LIFTED -- APAX_WriteData = golden 912 adam6024.cpp:2305-2673 (Adam6024Apax_St02.cpp, H3); golden 912 aTester_Front.cpp:2241.  Returns at once unless INSTALL_DOUBLE_EP>=2 (HT9050: 1); EP live switch OFF = no-op (R2)
                    APAX_WriteData(true, 0, 1);
#endif
                }

#if 0 // TODO(W7) -- golden :2165-2178 fSCKART->iInfo_MultiLotCnt + LotSummary.AddByLotLoadCount (FormsFacade TfSCKART has no iInfo_MultiLotCnt member; LotSummary has no translated home at all -- see file-header gates #15/#16)
                if(fSCKART->iInfo_MultiLotCnt>1)
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC &&
                               FTestSuck.cDeviceInf[i][j]!="")
                            {
                                LotSummary.AddByLotLoadCount(FTestSuck.cDeviceInf[i][j]);
                            }
                        }
                    }
                }
#endif

#if 0 // TODO(W7) -- golden :2180 fAutoTeach->SetIndexBinError (no FormsFacade TfAutoTeach stand-in -- see file-header gate #17)
                fAutoTeach->SetIndexBinError(0);                                                                                                                //JimmyChiu 20211020 : Auto alignment mode
#endif
                if(bHPCleanout)                                                                                                                                 //wei 20160624 Hotplate clean out
                {
                    return true;
                }
                else if(DeviceForm.bSuckShuttleDeviceAfterTested==false &&                                                                                      //Steven 20140827 : 第一次吸取ic等待時間
                        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false &&                                                                               //JerryYang 20220805 : 有開下壓shuttle預熱功能就不要做懸空預熱
                        (LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot) && bInitialSackTime &&                                //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能              //2013-11-27   Dell    需要做Index soak time
                        (Temperature.iInitialStart1Time>0 || Temperature.iIndexSoakTime>0))                                                                     //kevin 20131112 第一次吸取ic等待時間)
                {
                    int iSetTmr=Temperature.iInitialStart1Time;
                    if(Temperature.iIndexSoakTime>Temperature.iInitialStart1Time)
                        iSetTmr=Temperature.iIndexSoakTime;

                    DoTestHeadMotorDelay2.SetSecAndOn(iSetTmr);
                    Task=502;
                    bInitialSackTime=false;
                    bInitialStart1Time=true;                                                                                                                    //wei 20171020 (jou) InitialStart1 秒數倒數
                    dwStartInitialCount1.LatchCycleTime(true);
                    iInitialStart1Count=Temperature.iInitialStart1Time;
                }
                else
                {
                    return true;
                }
            }
            break;
        case 502:                                                               //kevin 20131112  等待initial sock time
            if(bInitialStart1Time)                                              //wei 20171020 (jou) InitialStart1 秒數倒數
            {
                if(iInitialStart1Count>0)
                {
                    iInitialStart1Count=(Temperature.iInitialStart1Time)-(dwStartInitialCount1.LatchCycleTime()/1000);
                    break;
                }
            }

            if(DoTestHeadMotorDelay2.Off())
            {
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//  PART FILE  _w7c_parts/00257_TestZ1OutRandge2.txt   (label k8-small-seven)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing Wave-1/Wave-2
//  content (this part reuses nothing beyond that file's include list).
//  Translator: AI(k8-small-seven) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 257..289 -- TestZ1OutRandge2() and nothing
//  else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD; Chinese comments transcribed
//  character for character from cp950.
//
//  ROLE          -- Encoder-vs-command position sanity gate for the FRONT index
//                   head: returns true ("out of range") if the command/encoder
//                   mismatch on MTestZ1, MTestY1 or MTestY2 exceeds 10 counts.
//                   Wider than the already-translated TestZ1OutRandge (golden
//                   :224-255, port aTester_Front.cpp:238-269) which checks the same
//                   three axes but is a separate golden function -- both kept.
//  WAVE SCOPE    -- TestZ1OutRandge2()   ACTIVE   golden :257-289
//  GATE REGISTER -- EMPTY.  This part writes NO #if 0 and makes NO offline
//                   substitution.  Every symbol is real in the port: MOT[]
//                   (Motor/mymotor.h), MTestZ1 / MTestY1 / MTestY2, and
//                   TMotor::Gali_ReadPos / Gali_ReadEncoderPos
//                   (Motor/mymotor.h:214-215, both returning long -- assigned to
//                   golden's `int Pos1/Pos2` exactly as golden does, narrowing
//                   PRESERVED, not widened).
//  SYMMETRIC TWIN -- this is the exact mirror of TestZ2OutRandge2 (golden
//                   aTester_Rear.cpp:257-289, port aTester_Rear.cpp:2521-2554,
//                   PT-W7b).  Its GATE REGISTER is empty for the same reason and
//                   that premise HOLDS unchanged on the Front side: the only
//                   textual difference between the two golden bodies is the first
//                   axis (MTestZ1 here, MTestZ2 there).  Nothing else differs --
//                   including golden's own uneven blank lines before the MTestY1
//                   and MTestY2 blocks, reproduced verbatim.
//  SOFT_SIMULTE  -- NOT defined in this build (CMakeLists.txt states so); the
//                   #ifdef body is reproduced VERBATIM and inert, per this file's
//                   existing convention.
//  INTEGER DIVISION -- none in this function.  The three comparisons are integer
//                   subtraction + magnitude against the literal 10; no float
//                   helper is introduced anywhere.
//  TRAP 1 -- NOT `static`: golden aTester_Front.h:37 declares
//                   `extern bool TestZ1OutRandge2();` and the port ALSO declares it
//                   non-static at atester_shims.h:107, so a file-scope `static`
//                   definition would be shape (d).  It is deliberately non-static.
//                   Live port caller that this body will actually serve:
//                   aTester_Rear.cpp:9978 `if(TestZ1OutRandge2())` (inside the
//                   already-translated DoTestYRear) -- so this is NOT shape (a).
//  TRAP 4 -- adds no file-scope object of any kind.
//  INTEGRATE     -- retires the offline stub atester_shims.cpp:183
//                   `bool TestZ1OutRandge2() { return false; }`.  That stub said
//                   "never out of range", which SUPPRESSED the golden
//                   ShowIndexMotorError path at the live caller -- a real
//                   behaviour change, not cosmetic.  Keep the non-static
//                   declaration atester_shims.h:107 (it is exactly why this body
//                   is not static).
//---------------------------------------------------------------------------
bool TestZ1OutRandge2()
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
        int Pos1=MOT[MTestZ1].Gali_ReadPos();
        int Pos2=MOT[MTestZ1].Gali_ReadEncoderPos();
        Pos1-=Pos2;
        if(Pos1<0)
            Pos1=0-Pos1;

        if(Pos1>10)                                                             //Steven 20100728 :縮小範圍
            return true;

        Pos1=MOT[MTestY1].Gali_ReadPos();
        Pos2=MOT[MTestY1].Gali_ReadEncoderPos();
        Pos1-=Pos2;
        if(Pos1<0)
            Pos1=0-Pos1;

        if(Pos1>10)                                                             //Steven 20100728 :縮小範圍
            return true;
        Pos1=MOT[MTestY2].Gali_ReadPos();
        Pos2=MOT[MTestY2].Gali_ReadEncoderPos();
        Pos1-=Pos2;
        if(Pos1<0)
            Pos1=0-Pos1;

        if(Pos1>10)                                                             //Steven 20100728 :縮小範圍
            return true;
        return false;
    #endif
}

//---------------------------------------------------------------------------
//  PART FILE  _w7c_parts/02227_InitFTestSuckTestICTask.txt  (label k8-small-seven)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing content.
//  Translator: AI(k8-small-seven) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 2227..2230 -- InitFTestSuckTestICTask() and
//  nothing else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD.
//
//  ROLE          -- Reset entry for the FRONT test-head suck-test state machine
//                   (DoFTestSuckTestIC): rewinds its cursor to step 1.
//  WAVE SCOPE    -- InitFTestSuckTestICTask()   ACTIVE   golden :2227-2230
//  GATE REGISTER -- EMPTY.  No #if 0, no substitution.  The one symbol it writes,
//                   iFTestSuckTestICTask, is a REAL data definition already in the
//                   port at atester_shims.cpp:154 (declared atester_shims.h:77,
//                   and also golden aTester_Front.h:7).
//  OUT OF MY RANGE (hand-off -- do NOT let this drift): golden :2226
//                   `int iFTestSuckTestICTask=1;` is NOT in my assigned range
//                   2227..2230, so it is deliberately NOT defined here.  It lives
//                   today at atester_shims.cpp:154 and can simply STAY -- it is
//                   plain data, not a stub body, so there is no
//                   duplicate-definition risk with this fragment.  (Note it is
//                   also NOT one of the 16 main-loop-owned file-scope globals; do
//                   not add it there either.)
//  SYMMETRIC TWIN -- InitBTestSuckTestICTask (golden aTester_Rear.cpp:2211-2215,
//                   port aTester_Rear.cpp:2581-2585, PT-W7b).  THE TWIN IS NOT
//                   IDENTICAL AND MUST NOT BE COPIED: the Rear body is TWO
//                   statements (`iBTestSuckTestICTask=1; bZ2Isdownflag=false;`)
//                   while golden's FRONT body is ONE (`iFTestSuckTestICTask=1;`
//                   only -- it does NOT clear bZ1Isdownflag).  That asymmetry is
//                   golden's own and is PRESERVED, not "made consistent".  It is
//                   also not an oversight I may paper over: the Front tree does
//                   clear bZ1Isdownflag, but in a DIFFERENT function
//                   (InitFrontTestSuckICTask, port aTester_Front.cpp:1067-1071).
//  TRAP 1 -- NOT `static`: golden aTester_Front.h:22 declares
//                   `extern void InitFTestSuckTestICTask();` and the port declares
//                   it non-static at atester_shims.h:93, so `static` would be
//                   shape (d).  Live port callers this body will serve (so not
//                   shape (a)): atester.cpp:7497, atester_32Site.cpp:2071,
//                   aTester_Rear.cpp:7501.
//  TRAP 4 -- adds no file-scope object.
//  INTEGRATE     -- retires the no-op stub atester_shims.cpp:170
//                   `void InitFTestSuckTestICTask() {}`.  That stub is NOT
//                   behaviour-neutral: it left the DoFTestSuckTestIC cursor
//                   wherever the previous lot had abandoned it, so a re-armed
//                   test could resume mid-sequence.  Keep the declaration at
//                   atester_shims.h:93.
//---------------------------------------------------------------------------
void InitFTestSuckTestICTask()
{
    iFTestSuckTestICTask=1;
}

//---------------------------------------------------------------------------
//  PART FILE  _w7c_parts/02237_DoFTestSuckTestIC.txt   (label k4-DoFTestSuckTestIC)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing Wave-1/Wave-2
//  content (this part REUSES seams already defined in that file; see ORDERING).
//  Translator: AI(W7Ck4) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 2237..3941 -- DoFTestSuckTestIC() and nothing
//  else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD.
//
//  ROLE
//  ----
//  DoFTestSuckTestIC() is the FRONT (Arm1 / Z1) socket-test state machine: with
//  the front test head already pressed into the sockets it runs the whole test
//  transaction -- optional [D01] torque read, pre-test site census, tester
//  START/END handshake (MTI/PTI DoStartMode/DoEndMode), result collection
//  (ProcessTestResult), Double-Contact / OverDrive / ReContact re-press loops,
//  tester TIME-OUT handling, [D37] manual-process key waits, ProcessCount/
//  RecordHistroy bookkeeping, the TestSocket<->FTestSuck grid hand-off, Murata XML
//  re-upload retries, [P65] ARM QA-mode re-test, and the KYEC F006 tail ([D79]
//  index-pick purge / SECS consecutive-fail / ATC active-cooling wait).  Returns
//  true exactly when the front test cycle is complete and the caller may lift Z1.
//
//  WAVE SCOPE
//  ----------
//    * DoFTestSuckTestIC()                golden :2237-3941   ACTIVE
//      Every case, branch and loop is ACTIVE.  Eight individual STATEMENTS inside
//      it are #if 0 gated -- see GATE REGISTER (G1..G8).
//
//  SYMMETRIC TWIN -- WHAT I TOOK FROM IT AND WHERE I DELIBERATELY DIVERGED
//  ----------------------------------------------------------------------
//  The mirror is DoBTestSuckTestIC (golden aTester_Rear.cpp:2222-3878, port
//  aTester_Rear.cpp:3161-5066, PT-W7b part 02222).  I read it in full.  1,545 of
//  my 1,705 golden lines differ from it, so it is a reference, not a template.
//  I kept its shim choices for SetATCOffset / fSocketCommunication / SiteTesting /
//  SendTestEnd / SendHandler2DID / InitReadTorueTask / NowNoteIsShow /
//  SendTestResultToHttp / RecordEndTestTime / BtnZUpDown, and its four FAITHFUL
//  TU-local TMyKitSuck equivalents.  THREE DELIBERATE DIVERGENCES:
//
//   (D1) *** I DO NOT GATE SetHangupMaxTime(). *** aTester_Rear.cpp:3602 and
//        :4083 gate it (its G1/G8) on the premise "it IS declared (cinitial.h:145)
//        but has ZERO definitions tree-wide".  THAT PREMISE IS DEAD.  Measured
//        2026-08-10 23:48 with CWD=D:/HT9045/HT9011UC_Cpp_V3.33.906.0:
//            grep -rn "^void SetHangupMaxTime" --include=*.cpp --exclude-dir=.svn
//                 --exclude-dir=build .
//        -> 1 hit, cinitial.cpp:11111, an ACTIVE faithful body whose own banner
//        reads "ACTIVE, zero gates -- fully faithful, no external dependency".
//        A preprocessor-nesting walk of cinitial.cpp to line 11111 reports
//        #if-stack depth 0 and zero enclosing `#if 0`, and cinitial.cpp is
//        compiled (CMakeLists.txt:1456).  It touches only Prod.iHangupMaxTime /
//        TestIF.iInitialMaxTime / TestIF.iMaxTime / bInitialMaxTime -- all already
//        reachable from this TU.  So golden :2691 and golden :3178 are ACTIVE
//        here, with a local forward declaration (this file does not include
//        cinitial.h).  *** MAIN LOOP: aTester_Rear.cpp's G1/G8 and
//        atester_32Site.cpp:392-395's `#define SetHangupMaxTime
//        W5_32S_SetHangupMaxTime` no-op are BOTH expired premises. ***
//
//   (D2) The golden pointer-arithmetic bug the Rear part had to preserve at its
//        golden :2348 (`ErrPart+=BTestSuck.Item[i][j]+", ";`, i.e. const char* +
//        int) DOES NOT EXIST on the Front side: golden :2366 is
//        `ErrPart+=IndexSuckName[i+iNN][j]+", ";`, a real AnsiString concat.
//        Nothing to preserve, nothing to "make consistent" with Rear.
//
//   (D3) The Rear G5 banner justifies gating ATCInterfaceForm->SendTestEnd partly
//        on "LedCore.h re-declares `typedef int TColor` plus a 9-constant cl*
//        palette that cmydef.h:16 -- already included by this file -- also
//        declares, i.e. a namespace-scope const redefinition hazard".  I measured
//        that and it is OVERSTATED: cmydef.h:16 declares the TColor typedef ONLY
//        (a typedef redeclaration to the same type is legal), and grep finds no
//        clLime/clSilver in cmydef.h.  My G4 therefore rests on the OTHER, real
//        half of that argument (new dependency surface taken from inside a part
//        file, into a TU sibling agents are writing) plus offline unreachability.
//        Same verdict, honest reason.  See GATE REGISTER G4.
//
//  ORDERING (why this part must be APPENDED, not prepended)
//  -------------------------------------------------------
//  It REUSES, rather than duplicates (duplication = redefinition in one TU):
//    * W64B_FMAIN_CHKREADTORQUE1 / _CHKREADTORQUE2 / _EDTORUE0
//      (port aTester_Front.cpp:1054-1056) -- the SAME golden fMain torque widgets
//      this function derefs at golden :2307-2309, :2317-2318, :3538-3541, :3552,
//      :3560, :3576, :3583, :3590-3591.
//    * W64B_FIOSET_PISD1_V0() (port aTester_Front.cpp:1037-1038) -- the SAME gap
//      at golden :3796 `fiosetview->ProcessIndexSuckDestroy1()` (0-arg overload).
//  If the main loop places this part BEFORE that Wave-2 block, hoist those four.
//
//  PREREQUISITE EMITTED HERE THAT IS OUTSIDE MY 2237..3941 RANGE
//  ------------------------------------------------------------
//  Marked [W7Ck4-PREREQ].  golden :99 `TQPF_Timer hFTestTimeOutDelay;` is a plain
//  file-scope data global that THIS function is the only consumer of (read at
//  golden :3183 and :3212) and that has NO definition anywhere in the port.
//  Without it the stitched file cannot link.
//    ABSENCE COMMAND: grep -rn "hFTestTimeOutDelay" --include=*.h --include=*.cpp .
//    MEASURED 2026-08-10 23:44 -> 5 hits, ALL in atester_32Site.cpp, and all of
//    them that file's OWN TU-local seam (:337 `static TQPF_Timer
//    W5_32S_hFTestTimeOutDelay;` + :338 `#define hFTestTimeOutDelay ...`) or uses
//    of it.  Zero non-static definitions, zero header declarations.
//  SIBLING-OVERLAP RE-CHECK (TRAP 2), run against _w7c_parts/ itself at
//  2026-08-10 23:44:
//    grep -rn "hFTestTimeOutDelay|FTorqueTimeOutDelay|^TQPF_Timer|lbArm0Torque|
//              BtnZUpDown" _w7c_parts    [alternation escaped for grep]
//  -> NO sibling part defines any of them.  *** MAIN LOOP: golden :99 is NOT one
//  of the 16 main-loop-owned file-scope globals (that list is :75, :78, :94-:98,
//  :2232, :2235, :3943, :4071, :4921, :5160-:5162, :8010 -- :99 sits one line
//  past the :94-:98 block).  If you decide to re-home golden :94-:99 as one block,
//  delete my [W7Ck4-PREREQ] line; grep the marker. ***
//
//  NOT emitted, deliberately (outside my range AND/OR owned elsewhere):
//    * golden :2232 TQPF_Timer DoFTestSuckTestICDelay  -- main-loop-owned global.
//    * golden :2235 TQPF_Timer DoFTestSuckOffDelay     -- main-loop-owned global.
//      I USE both and assume they exist.
//    * golden :2233 `extern int SendTestResultToHttp();` -- see GATE REGISTER S2.
//    * golden :2226 `int iFTestSuckTestICTask=1;` -- real data definition already
//      at atester_shims.cpp:154.  Referenced, never defined, here.
//    * InitFTestSuckTestICTask() -- golden :2227-2230, OWNED BY SIBLING PART
//      _w7c_parts/02227_InitFTestSuckTestICTask.txt (AI(k8-small-seven) 20260810).
//
//  TRAP 1 -- WHY THIS IS REALLY LINKED, NOT JUST "BUILD GREEN"
//  ----------------------------------------------------------
//  `bool DoFTestSuckTestIC()` is emitted NON-STATIC on purpose.  A non-static
//  declaration already exists at atester_shims.h:104 and a no-op stub body at
//  atester_shims.cpp:180 `bool DoFTestSuckTestIC() { return true; }`, so writing
//  mine `static` would be exactly TRAP-1 shape (d) -- an internal-linkage shadow
//  that links clean and then explodes when atester_shims.h is included in the
//  same TU (it IS: port aTester_Front.cpp:103).
//  *** MAIN LOOP: this part retires atester_shims.cpp:180.  That stub is NOT
//  behaviour-neutral -- `return true` means "front test cycle complete" on the
//  very first pump, so the caller lifts Z1 having never tested anything.  Keep the
//  DECLARATION at atester_shims.h:104. ***
//  This is also NOT shape (b): the stub is the demand-satisfier being replaced,
//  not a second body left in place beside mine.
//
//  TU-LOCAL FAITHFUL EQUIVALENTS (NOT gates, NOT degraded stubs)
//  ------------------------------------------------------------
//  Four TMyKitSuck methods golden calls have no member on the port class, but
//  their golden bodies touch ONLY fields the port class already has, so they are
//  transcribed IN FULL as TU-local free functions instead of being gated (exact
//  precedent: aTester_Rear.cpp's W7Bk4_* block, port :2986-3137):
//    * W7Ck4_CopyFrom               == golden MyKitSuck.cpp:1151-1193
//    * W7Ck4_MoveAllItem            == golden MyKitSuck.cpp:1043-1109
//    * W7Ck4_Tested                 == golden MyKitSuck.cpp:1017-1030
//    * W7Ck4_CheckVaccumIsIniaialON == golden MyKitSuck.cpp:2732-2746
//  All four golden bodies were re-read from golden MyKitSuck.cpp for this part --
//  not copied on trust from the Rear part.
//
//  TRAP 5 STATEMENT (two headers, same class name)
//  ----------------------------------------------
//  TMyKitSuck EXISTS TWICE WITH DIFFERENT LAYOUTS: mykitsuck.h:274 and
//  aHotPlateSubstrate.h:365.  I relied on **aHotPlateSubstrate.h:365-622** -- the
//  one this file already includes (port aTester_Front.cpp:115) and the one that
//  declares the objects I touch.  The OBJECTS are DEFINED at
//  aHotPlateSubstrate.cpp:91 `TMyKitSuck TestSocket;` and :92 `TMyKitSuck FTestSuck;`,
//  declared aHotPlateSubstrate.h:635-636.  SECOND TRAP-5 CLASS IN THIS BODY: TMySucker.
//  golden :3812 / :3829 call FTestSuck.Suck[i][j].Off() / .Normal(), and
//  W7Ck4_CheckVaccumIsIniaialON calls .GetStatus() / .Normal(); I relied on the
//  TMySucker at **aHotPlateSubstrate.h:106-140** (Off :122, Normal :130, GetStatus
//  :131) -- the one whose grid member Suck[][] belongs to the TMyKitSuck above, NOT
//  mykitsuck.h's.  I did NOT include
//  mykitsuck.h anywhere and I did NOT call any method that exists only on the
//  mykitsuck.h twin.  Fields verified present in THAT class for the four bodies
//  above: Suck :369, Item :370, iWhichSite :371, PordRec :372 (asBuffer
//  Public/MyProductionRecord.h:158, bUse :164, InitialRecord :72), iMaxRow :377,
//  iMaxCol :402, iShtRow :406, iShtCol :438, iBinData :439, cDeviceInf :440,
//  cSBin :441, iWhichAuto :450, bPass :451, bNeedReTest :452, SetItemData :497,
//  iAutoCleanRecX/Y :556-557, iWhichIndex :558, iNeedRotAng :559, iCurrRotAng
//  :560, bQATray :561, iCleanCount :562, bFliped :563, iBinDataBackUp :564,
//  cReDeviceInf :565, b2DIDNG :566, iAOIResult :567.  Independent proof the field
//  set is complete: the same header's own MoveSuckData/MoveSuckDataDiff
//  (:572/:598) already move that identical set.  UseSiteHasIC :467, HasRealIC
//  :384 and CountRealIC :455 are REAL members of this class and are called
//  DIRECTLY (golden :3276, :2645/:2925/:2981/:3273/:4008-equivalent, :3008).
//
//  TRAP 4 STATEMENT (no static-init constructor may touch a NULL global)
//  --------------------------------------------------------------------
//  This part adds exactly THREE file-scope objects: `TQPF_Timer
//  hFTestTimeOutDelay` and the two S4 widget stand-ins `TSpeedButton
//  W7Ck4_BtnZUpDown` / `TPanel W7Ck4_lbArm0Torque`.  Verified by reading the ctor
//  chains, not assumed: TSpeedButton (vclcompat/Controls.h:421-428) is
//  `TSpeedButton() : Down(false), GroupIndex(0) {}` over TControl :213-223
//  (`Visible(false), Enabled(false), hCtl(0)`) over TObject
//  (vclcompat/TStringList.h:32-35, EMPTY -- virtual dtor only); TPanel :240-246 is
//  `TPanel() : Color(0) {}` over the same chain.  NOT ONE of them dereferences a
//  global, calls a free function, or adds to a container.  TQPF_Timer at file
//  scope has four existing precedents in this very file (port
//  aTester_Front.cpp:134, :960, :961, :1073).  fNote IS dereferenced by S1 and by
//  golden :3215 -- but only from the running state machine, never during static
//  init, and I add no object whose ctor could reach it (that is precisely the
//  fLaserSensor/elLaser shape this trap names).
//
//  GATE REGISTER
//  -------------
//  Every #if 0 this part writes (G1..G8), plus the three value-returning stand-ins
//  (S1..S3) and the one widget seam (S4) -- those four are listed because they
//  rest on the same kind of claim and must expire the same way.  Every command
//  below was RUN by me with CWD = D:/HT9045/HT9011UC_Cpp_V3.33.906.0 on
//  2026-08-10 at the per-entry time stamp.  TRAP 2: RE-RUN EVERY ONE AT INTEGRATE
//  -- a sibling part of THIS wave can land any of these minutes after I measured,
//  and "the degraded value happens to be equivalent" would NOT make a dead premise
//  acceptable.  (D1 above is exactly that failure mode, caught in the Rear part.)
//
//  *** WHOLE-REGISTER RE-CHECK, RUN AT 2026-08-10 23:56:54 (TRAP 2). ***  Six
//  sibling parts (05165/06119/07036 DoTestYFront c1-c3, 08098 FTestNeedDestroy,
//  08110 FTestSeparateSLK, 08201 FTestCombineSLK) landed in _w7c_parts AFTER my
//  first measurements, so every claim below was re-measured after they existed:
//    grep -n "hFTestTimeOutDelay|W7Ck4_|^TQPF_Timer|lbArm0Torque|BtnZUpDown"
//         _w7c_parts/*.txt | grep -v ^_w7c_parts/02237     -> 0 hits (no overlap)
//    grep -rn "^void SetHangupMaxTime" --include=*.cpp --exclude-dir=.svn
//         --exclude-dir=build --exclude-dir=_w7c_parts .   -> still cinitial.cpp:11111
//    grep -rn "SetATCOffset|fSocketCommunication|SendHandler2DID|SiteTesting|
//         InitReadTorueTask|NowNoteIsShow|SendTestResultToHttp|RecordEndTestTime|
//         lbArm0Torque|BtnZUpDown" --include=*.h --exclude-dir=.svn
//         --exclude-dir=build --exclude-dir=tools .        -> 0 hits, ALL STILL ABSENT
//  (alternations escaped for grep).  Every gate below is therefore live-premised as
//  of 23:56:54; re-run them once more at integrate.
//
//  G1 (golden :2699)   fLotInfo->SetATCOffset(true)
//    WHY THE OFFLINE DEFAULT IS FAITHFUL: it pushes the ATC initial-temperature
//    offset once, on the iInitContactCount == Temperature.
//    iCintactCntForTempOffsetAtInitial edge.  There is no ATC controller offline
//    (ATC_SYSTEM==0==eATCUninstall, cmydef.cpp:3567; TATC_InterfaceFormShim ships
//    iATC_MODE_TYPE==0), so "no offset pushed" IS the offline hardware state.
//    golden :2700 iInitContactCount++ stays ACTIVE, so the edge still passes
//    exactly once and the SM cannot re-enter.
//    REAL-MACHINE DIFFERENCE: the ATC never receives its initial-contact
//    temperature offset, so the first sites test at un-offset temperature.
//    WHY IT SHOULD BE GATED (TRAP 3 re-ask): TfLotInfo (forms/fLotInfo.h) has no
//    SetATCOffset member at all -- not "the object is missing", the METHOD is.
//    The only textual use in the tree, atester.cpp:2250, has no member either.
//    ABSENCE COMMAND: grep -rn "SetATCOffset" --include=*.h --exclude-dir=.svn
//                          --exclude-dir=build --exclude-dir=tools .
//    MEASURED 2026-08-10 23:47 -> 0 hits in ANY header.
//
//  G2 (golden :2733) and G6 (golden :3173)
//                      fSocketCommunication->bSendBinMapReport=true
//    WHY FAITHFUL: the ASE_KR socket-tester bin-map report trigger, reached only
//    when IniConfig.bEnableSocketCommunication is on.  No socket-tester form
//    exists offline, so "no report queued" is the offline state, and nothing in
//    this function reads the flag back.
//    REAL-MACHINE DIFFERENCE: an ASE_KR machine would not send its per-cycle
//    bin-map report to the socket tester.
//    WHY GATED: no TfSocketCommunication class and no fSocketCommunication object
//    exist anywhere in the ported tree -- there is nothing to point the call at.
//    ABSENCE COMMAND: grep -rn "fSocketCommunication" --include=*.h
//                          --exclude-dir=.svn --exclude-dir=build --exclude-dir=tools .
//    MEASURED 2026-08-10 23:47 -> 0 hits in any header.
//
//  G3 (golden :2938)
//        ATC_InterfaceForm->SiteTesting(iATC_Use_Heat_Count, bATCSiteTest)
//    WHY FAITHFUL: opens ATC per-site 2nd-point temperature monitoring for the
//    sites under test.  With no ATC offline there is nothing to monitor.  Golden's
//    loop that fills bATCSiteTest[] (:2936-2937) is left ACTIVE, so the array is
//    still built exactly as golden builds it, and :2933 bATC_SITE_2ND_CHECK[0]=
//    false also stays ACTIVE, so no consumer sees a stale "checked" state.  The
//    enclosing guard needs ATC_SYSTEM==eNewATCSystem(6) and ATC_SYSTEM is 0
//    offline, so this is behaviour-identical offline.
//    REAL-MACHINE DIFFERENCE: ATC 2nd-point monitoring is not armed for this
//    contact, so a single-site over/under-temperature would not raise the ATC
//    site alarm.
//    WHY GATED: TATC_InterfaceFormShim exposes EXACTLY one member.  Measured by
//    printing the class body, not by grep:
//    ABSENCE COMMAND: sed -n '/class TATC_InterfaceFormShim/,/^};/p' acarry_shims.h
//    MEASURED 2026-08-10 23:48 -> body is { int iATC_MODE_TYPE; ctor; } only.
//
//  G4 (golden :2942)   ATCInterfaceForm->SendTestEnd(0)
//    *** THIS IS THE TRAP-3 SHAPE: A REAL PORT BODY EXISTS AND I STILL GATE IT,
//    FOR A REASON THAT IS NOT "THE SYMBOL IS ABSENT". ***
//    STATUS (not an absence claim -- all three facts measured 2026-08-10 23:47):
//      grep -n "SendTestEnd" ATC/ATCInterface.h        -> :468 void SendTestEnd(int)
//      grep -n "ATCInterfaceForm *= *new" ATC/ATCInterface.cpp
//                                                     -> :207 real eager `new`
//      grep -n "ATC/ATCInterface.cpp" CMakeLists.txt   -> :2119 (compiled, ht9045_sm)
//    WHY IT SHOULD STILL BE GATED HERE: reaching it costs THIS translation unit a
//    NEW include of ATC/ATCInterface.h, which drags in ATC/ATCSystem.h +
//    vclcompat/ClientSocket.h + vclcompat/ServerSocket.h + vclcompat/LedCore.h
//    (measured: grep -n "#include" ATC/ATCInterface.h -> :154-161).  None of those
//    four is currently reachable from aTester_Front.cpp.  Taking a whole
//    socket/LED header chain for ONE fire-and-forget notification is not a
//    decision a part file may make unilaterally, least of all in a TU that sibling
//    agents of this same wave are appending to concurrently.  AND it is
//    behaviour-neutral offline: the branch needs ATC_SYSTEM==eATCHonPrecType(4)
//    while ATC_SYSTEM==0 (cmydef.cpp:3567).
//    NOT MY REASON (see D3 above): the Rear part's "TColor / cl* palette
//    redefinition hazard" is overstated -- cmydef.h:16 declares the typedef only.
//    RECOMMENDATION: retire G4 at the integrate step by adding that include ONCE
//    after the stitch compiles -- not from inside a part file.
//    REAL-MACHINE DIFFERENCE: on an ATC_SYSTEM==eATCHonPrecType machine the
//    HonPrec ATC is not told "test end" for arm 0 (the front arm), so its
//    soak/cool state machine does not advance on this event.
//
//  G5 (golden :2990) and G7 (golden :3282)
//                      ATC_InterfaceForm->SendHandler2DID(0, false)
//    WHY FAITHFUL: hands the just-tested 2D-ID string to the ATC side channel.  No
//    ATC offline means nothing to hand over, and no state in this function is read
//    back from it.  UNLIKE G3/G4 these two calls are NOT behind an ATC_SYSTEM
//    guard -- they run on every cycle -- so this gate DOES change what happens on
//    every front test cycle offline, from "call a no-op-ish notifier" to "nothing".
//    REAL-MACHINE DIFFERENCE: ATC-side 2DID traceability loses this cycle's entry.
//    WHY GATED: same one-member TATC_InterfaceFormShim as G3.  atester.cpp:3992
//    has the identical call and no member either; atester_32Site.cpp:504 routes it
//    through its own no-op W5_32S_ATC_SENDHANDLER2DID.
//    ABSENCE COMMAND: grep -rn "SendHandler2DID" --include=*.h --exclude-dir=.svn
//                          --exclude-dir=build --exclude-dir=tools .
//    MEASURED 2026-08-10 23:47 -> 0 hits in any header (only two prose comments in
//    atester_32Site.cpp:119/:2740).
//
//  G8 (golden :3584)   COM2->InitReadTorueTask()
//    WHY FAITHFUL: re-arms the serial torque-reader task before a second read.
//    There is no torque MCU offline, and the two golden lines bracketing it stay
//    ACTIVE (:3583 clears edTorue0->Text, and the enclosing else-arm structure is
//    untouched), so the SM still takes its "no torque value came back" path
//    (case 5300 -> 5350 -> 5000) exactly as on a machine whose torque reader never
//    answers.
//    REAL-MACHINE DIFFERENCE: the second torque read is never re-armed, so a
//    machine with [D01] on always takes the retry/abandon path
//    (iArm1ReadtorquCount>3 -> Task=100) instead of reading a torque value on
//    attempt 2.
//    WHY GATED: no header in the tree declares InitReadTorueTask on any class; the
//    only two occurrences are the TU-LOCAL seam structs of atester.cpp (:5607) and
//    atester_32Site.cpp (:372), and reaching across a TU to borrow one is not
//    available.  COM2 itself is not declared in this TU either.
//    ABSENCE COMMAND: grep -rn "InitReadTorueTask" --include=*.h --exclude-dir=.svn
//                          --exclude-dir=build --exclude-dir=tools .
//    MEASURED 2026-08-10 23:47 -> 0 hits in any header.
//
//  S1 (golden :3161 and :3184)   NowNoteIsShow()   -- stand-in, but FAITHFUL
//    Golden note.cpp:4286-4289 is literally `return fNote->fShow;`, and the port
//    has a real fNote (forms/fNote.h:169, object forms/fNote.cpp) with a real
//    fShow (forms/fNote.h:133), reachable here through FormsFacade.h:62.  So
//    W7Ck4_NowNoteIsShow carries the golden body verbatim: no offline default is
//    invented and no behaviour is degraded.  Independent corroboration that the
//    deref is the intended shape: golden :3215 IN THIS FUNCTION does
//    `if(fNote->fShow)` directly, and I keep that ACTIVE and unguarded too.
//    ABSENCE COMMAND (for the FREE FUNCTION NAME only):
//      grep -rn "NowNoteIsShow" --include=*.h --exclude-dir=.svn --exclude-dir=build
//           --exclude-dir=tools .
//    MEASURED 2026-08-10 23:47 -> 0 hits tree-wide (only the form half is ported;
//    the free function has no home).
//
//  S2 (golden :3720)   SendTestResultToHttp()  -> offline 1 (== upload OK)
//    WHY FAITHFUL: 1 is golden's own "no retry needed" value and is exactly what
//    the Murata XML path sees when there is no HTTP endpoint, which IS the offline
//    condition.  Returning != 1 would instead drive the SM into the WAR16321
//    "3 upload failures" alarm, i.e. FABRICATE a failure.
//    REAL-MACHINE DIFFERENCE: results are never actually POSTed.
//    ABSENCE COMMAND: grep -rn "SendTestResultToHttp" --include=*.h
//                          --exclude-dir=.svn --exclude-dir=build --exclude-dir=tools .
//    MEASURED 2026-08-10 23:47 -> 0 hits in any header; the only body in the tree
//    is atester_32Site.cpp:394's own TU-local `static int
//    W5_32S_SendTestResultToHttp(){ return 1; }` (SAME value).  golden itself only
//    forward-declares it locally at golden :2233, which is OUTSIDE my range.
//    TRAP 1(d) NOTE: my stand-in is `static` but carries a DIFFERENT NAME
//    (W7Ck4_SendTestResultToHttp), so it cannot be a static shadow -- there is no
//    non-static declaration of that name anywhere to shadow.  Same for S1/S3.
//
//  S3 (golden :2926)   RecordEndTestTime(0)  -> offline 1
//    WHY FAITHFUL: same shape as S2 -- ret2==1 means "cycle-time record written,
//    no XML retry"; anything else fabricates a retry storm through case 7000.
//    golden's own argument is 0 here (0:arm1 1:arm2 2:both), preserved.
//    REAL-MACHINE DIFFERENCE: per-cycle index cycle-time telemetry is not recorded.
//    ABSENCE COMMAND: grep -rn "RecordEndTestTime" --include=*.h --exclude-dir=.svn
//                          --exclude-dir=build --exclude-dir=tools .
//    MEASURED 2026-08-10 23:47 -> 0 hits in any header; only atester_32Site.cpp:393
//    TU-local stand-in (same value 1).
//
//  S4 (golden :2752, :2776, :2790, :2811, :2814, :3430, :3493, :3495)
//        fMain->BtnZUpDown      and   (golden :3542) fMain->lbArm0Torque
//    WHY FAITHFUL: TfMain (forms/fMain.h) HAS BtnSTEP :696 and BtnT_Start :697 --
//    both used ACTIVE here including ->Enabled (TControl, vclcompat/Controls.h:216)
//    and ->Color (TPanel :243) -- but has NEITHER BtnZUpDown NOR lbArm0Torque.  The
//    stand-ins default Down=false / Enabled=false / Caption="", and Down==false is
//    the real state of a never-pressed toggle, so every golden `if(...->Down)`
//    takes its else path, which is the ordinary non-TSMC-manual path.
//    REAL-MACHINE DIFFERENCE: on a TSMC machine with
//    CosFunction.bEnableSoftWareControlButton the operator's "Z up/down" latch can
//    no longer divert case 3000/3100, and the "1:Reading" torque caption is not
//    displayed.  Pure UI; no SM state is read back from either widget.
//    ABSENCE COMMAND: grep -rn "BtnZUpDown|lbArm0Torque" --include=*.h
//         --exclude-dir=.svn --exclude-dir=build --exclude-dir=tools .
//         [alternation escaped for grep]
//    MEASURED 2026-08-10 23:47 -> 0 hits in ANY header.  lbArm0Torque exists in the
//    tree only as atester.cpp:5563 and atester_32Site.cpp:276 TU-LOCAL TPanel
//    seams -- whose class choice (TPanel, golden main.h:798) I copy deliberately;
//    BtnZUpDown has zero occurrences anywhere.
//
//  GOLDEN DEFECTS PRESERVED (NOT fixed -- behaviour kept as golden has it)
//  ----------------------------------------------------------------------
//   1. golden :3118  `Task=3200;` ([P65] ARM QA Mode ReTest, Arm1) jumps to a case
//      label THAT DOES NOT EXIST in this switch.  Measured, not eyeballed: the
//      case labels present in 2237..3941 are 1, 100, 200, 2200, 2400, 2410, 2500,
//      2600, 2700, 3000, 3050, 3100, 5000, 5050, 5100, 5300, 5350, 5400, 5450,
//      5500, 6000, 6100, 6200, 6300, 6500, 6600, 7000, 7100, 8000, 8001, 8002,
//      8003, 8010, 8011, 8012, 9000, 9001, 9100, 9101 -- there is no `case 3200:`.
//      Once taken, every later call falls off the switch to `return false`
//      forever: a PERMANENT STALL of the front test SM.  Reachable only with
//      IniConfig.bP65EnableArmQAMode && IniConfig.iP65ArmQAModeValue>0 &&
//      iP65QAReTestCount<that value && all-bin-1 && FTestSuck.HasRealIC().
//      Reproduced exactly; NOT "fixed" to 2400 or 3000.  (Identical defect to the
//      Rear twin's golden :3070, reported by PT-W7b.)
//   2. golden :2232-2247 -- `int &Task=iFTestSuckTestICTask, ret, iMaxDoubleContact,
//      ret2;`, `int iDbContactHighSlowSpd;` and `bool bAlreadyTested;` are
//      UNINITIALISED locals (golden relies on every reaching path assigning
//      first).  Left uninitialised, verbatim.
//   3. golden :3367 -- the closing brace of case 2410's else block is indented to
//      column 17 instead of 13.  Cosmetic golden quirk, transcribed as-is.
//   4. golden :3860-3861 (case 9001) -- `#ifdef SOFT_SIMULTE
//      iSECSGEM_ConsecutiveFailureAlarm=0; #endif` means that on a REAL machine
//      the SECS consecutive-failure latch is never cleared here, so WAR07362 can
//      re-raise.  Golden's asymmetry, preserved.
//
//  TRANSLATION RULES (same as the rest of this file)
//  ------------------------------------------------
//    * SOFT_SIMULTE and TEST_BIN_MISS_SIMULATE are NOT defined in this tree; the
//      #ifdef blocks at golden :2296-2298, :2970-2979, :3432-3443, :3452-3471 and
//      :3861-3863 are reproduced VERBATIM (both branches), letting the same
//      MachineType.h decide, exactly as golden does.
//    * Statement order, every formula, every magic number and every switch
//      fall-through (case 1 -> 100, 100 -> 200, 200 -> 2200, 2200 -> 2400)
//      preserved.  golden :2420's commented-out `break;` is kept as a comment.
//    * INTEGER DIVISION UNTOUCHED: golden :3335
//      `((dwEndAfterTestCount-dwStartAfterTestCount)/1000)`, :3483
//      `MOT[MTestZ1].GailSpeed/2`, :3664 and :3679 `*Prod.TestZ_Drop_Speed/100`
//      all stay integer.  No float helper anywhere.
//    * Big5 comments decoded via cp950 and preserved character-for-character as
//      UTF-8.  ZERO U+FFFD (asserted by the generator that emitted this file).
// ==========================================================================

// ---- [W7Ck4-INCLUDE] three headers this part adds.  MAIN LOOP: hoist them into
//      aTester_Front.cpp's include block if you prefer -- all three are
//      include-guarded and order-independent here.
//        myswitch.h                -- SW[] / TMySwitch::Off()   (golden :2736, :3283)
//        SECSGEM/SecsEventType.h   -- SECS_EVENT (ETypeStruct)   (golden :3859)
//        SECSGEM/SecsEventReport.h -- void EventReport(unsigned)  (golden :3859)
//      All three are compiled: myswitch.cpp CMakeLists.txt:1007,
//      SECSGEM/SecsEventType.cpp :1125, SECSGEM/SecsEventReport.cpp :1126.
//      (aTester_Front.cpp had ZERO `SW[` uses before this part -- measured.)
// AI(pt-wave) 20260811: #include hoisted to the top of aTester_Front.cpp by the integrator -- a mid-file include lands in the middle of the stitched TU. Original: #include "myswitch.h"
// AI(pt-wave) 20260811: #include hoisted to the top of aTester_Front.cpp by the integrator -- a mid-file include lands in the middle of the stitched TU. Original: #include "SECSGEM/SecsEventType.h"
// AI(pt-wave) 20260811: #include hoisted to the top of aTester_Front.cpp by the integrator -- a mid-file include lands in the middle of the stitched TU. Original: #include "SECSGEM/SecsEventReport.h"

// ---- [W7Ck4-EXTERN] local forward declarations, mirroring golden's own
//      local-extern idiom (golden :2233) and this file's existing precedent
//      (port aTester_Front.cpp:953 `extern bool CheckCFixTrayFullPlace();`).
//      Every one names a REAL, LINKABLE port symbol -- none of these is a gate.
extern bool bEcho;                                                              // golden main.cpp; port definition atester_shims.cpp:101
extern bool bEchoStop;                                                          // port definition atester_shims.cpp:102   //ChungHung 20130326 add
bool RespondASECom(AnsiString S1);                                              // golden cpublic.h:41 (port body canary_support.cpp)
bool WaitManualStartKey();                                                      // golden ckernel.cpp -> port ckernel.h:96, ACTIVE body ckernel.cpp
bool WaitManualStepKey();                                                       // golden ckernel.cpp -> port ckernel.h:97, ACTIVE body ckernel.cpp
void ShowMainScreenPresure(int index);                                          // golden cinitial.h:39 -> port cinitial.h:250, ACTIVE body cinitial.cpp:18785
void SetHangupMaxTime();                                                        // golden cinitial.h:59 -> port cinitial.h:145, ACTIVE body cinitial.cpp:11111 -- see DIVERGENCE (D1)
extern const unsigned int MSG_CMD_OverDrive;                                    // port MessageDef.h:120 / MessageDef.cpp
extern const unsigned int MSG_CMD_ReContact;                                    // port MessageDef.h:121 / MessageDef.cpp

// ---- [W7Ck4-PREREQ] golden :99 -- file-scope peer OUTSIDE 2237..3941 of which
//      DoFTestSuckTestIC is the sole consumer (read at golden :3183 / :3212).
//      Delete if the main loop re-homes golden :94-:99 as one block.
TQPF_Timer hFTestTimeOutDelay;                                                  //Jou 20101018   // golden :99

// ==========================================================================
//  W7Ck4 SEAM BLOCK -- four FAITHFUL free-function equivalents (golden bodies
//  transcribed in full from golden MyKitSuck.cpp), three offline stand-ins
//  (S1..S3) and one widget seam (S4).  All are `static` under names no other
//  declaration in the tree uses, so none can become a TRAP-1(d) static shadow.
// ==========================================================================

// -- golden MyKitSuck.cpp:1151-1193  TMyKitSuck::CopyFrom(TMyKitSuck &Source)
//    FAITHFUL: golden body verbatim, implicit `this->` spelled `dst.`.  Not
//    declared by aHotPlateSubstrate.h:365-622; every field it touches IS.
static void W7Ck4_CopyFrom(TMyKitSuck &dst, TMyKitSuck &Source)
{
    int MinRow, MinCol;
    if(Source.iMaxRow>=dst.iMaxRow)
        MinRow=dst.iMaxRow;
    else
        MinRow=Source.iMaxRow;

    if(Source.iMaxCol>=dst.iMaxCol)
        MinCol=dst.iMaxCol;
    else
        MinCol=Source.iMaxCol;

    for(int i=0; i<MinRow; i++)
    {
        for(int j=0; j<MinCol; j++)
        {
            dst.SetItemData(i, j, Source.Item[i][j]);
            dst.PordRec[i][j].asBuffer->CommaText=Source.PordRec[i][j].asBuffer->CommaText;
            dst.PordRec[i][j].bUse  =Source.PordRec[i][j].bUse;                     //Frank 20160505 add

            dst.cDeviceInf[i][j]    =Source.cDeviceInf[i][j];
            dst.cReDeviceInf[i][j]  =Source.cReDeviceInf[i][j];
            dst.cSBin[i][j]         =Source.cSBin[i][j];                            //Steven 20220120 : Amlogic需要收SBIN
            dst.b2DIDNG[i][j]       =Source.b2DIDNG[i][j];                          //Steven 20200611 : for Murata, 2DID NG不測試

            dst.iWhichSite[i][j]    =Source.iWhichSite[i][j];
            dst.iWhichAuto[i][j]    =Source.iWhichAuto[i][j];
            dst.iCurrRotAng[i][j]   =Source.iCurrRotAng[i][j];                      //Steven 20170425 (wei) : Add rotate motor
            dst.iNeedRotAng[i][j]   =Source.iNeedRotAng[i][j];
            dst.iWhichIndex[i][j]   =Source.iWhichIndex[i][j];                      //ChungHung 20150205 add for ATK
            dst.bPass[i][j]         =Source.bPass[i][j];
            dst.iCleanCount[i][j]   =Source.iCleanCount[i][j];
            dst.bFliped[i][j]       =Source.bFliped[i][j];
            dst.iBinData[i][j]      =Source.iBinData[i][j];
            dst.iBinDataBackUp[i][j]=Source.iBinDataBackUp[i][j];                   //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
            dst.bQATray[i][j]       =Source.bQATray[i][j];
            dst.iAutoCleanRecX[i][j]=Source.iAutoCleanRecX[i][j];
            dst.iAutoCleanRecY[i][j]=Source.iAutoCleanRecY[i][j];
            dst.iAOIResult[i][j]    =Source.iAOIResult[i][j];                       //Sam 20240325 : 新增 DamageTrayMapping 功能
        }
    }
}

// -- golden MyKitSuck.cpp:1043-1109  TMyKitSuck::MoveAllItem(TMyKitSuck &Source)
//    FAITHFUL: golden body verbatim, implicit `this->` spelled `dst.`.  NOTE this
//    MOVES -- golden :1085-1106 clears every Source slot afterwards -- it is not a
//    copy, and that clearing is reproduced.
static void W7Ck4_MoveAllItem(TMyKitSuck &dst, TMyKitSuck &Source)
{
    int MinRow, MinCol;
    if(Source.iMaxRow>=dst.iMaxRow)
        MinRow=dst.iMaxRow;
    else
        MinRow=Source.iMaxRow;
    if(Source.iMaxCol>=dst.iMaxCol)
        MinCol=dst.iMaxCol;
    else
        MinCol=Source.iMaxCol;

    for(int i=0; i<MinRow; i++)
    {
        for(int j=0; j<MinCol; j++)
        {
            dst.SetItemData(i, j, Source.Item[i][j]);

            dst.cDeviceInf[i][j]            =Source.cDeviceInf[i][j];
            dst.b2DIDNG[i][j]               =Source.b2DIDNG[i][j];                  //Steven 20200611 : for Murata, 2DID NG不測試
            dst.cReDeviceInf[i][j]          =Source.cReDeviceInf[i][j];
            dst.cSBin[i][j]                 =Source.cSBin[i][j];                    //Steven 20220120 : Amlogic需要收SBIN

            dst.iWhichSite[i][j]            =Source.iWhichSite[i][j];
            dst.iCurrRotAng[i][j]           =Source.iCurrRotAng[i][j];              //Steven 20170425 (wei) : Add rotate motor
            dst.iNeedRotAng[i][j]           =Source.iNeedRotAng[i][j];
            dst.iWhichAuto[i][j]            =Source.iWhichAuto[i][j];
            dst.iWhichIndex[i][j]           =Source.iWhichIndex[i][j];              //ChungHung 20150205 add for ATK

            dst.bPass[i][j]                 =Source.bPass[i][j];
            dst.iCleanCount[i][j]           =Source.iCleanCount[i][j];
            dst.bFliped[i][j]               =Source.bFliped[i][j];

            dst.iBinData[i][j]              =Source.iBinData[i][j];
            dst.iBinDataBackUp[i][j]        =Source.iBinDataBackUp[i][j];           //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount

            dst.bQATray[i][j]               =Source.bQATray[i][j];
            dst.iAOIResult[i][j]            =Source.iAOIResult[i][j];               //Sam 20240325 : 新增 DamageTrayMapping 功能

            dst.PordRec[i][j].asBuffer->CommaText=Source.PordRec[i][j].asBuffer->CommaText;
            dst.PordRec[i][j].bUse          =Source.PordRec[i][j].bUse;             //Frank 20160505 add

            Source.SetItemData(i, j, NULL_IC);
            Source.PordRec[i][j].InitialRecord();

            Source.cDeviceInf[i][j]     ="";
            Source.cReDeviceInf[i][j]   ="";
            Source.cSBin[i][j]          ="";                                    //Steven 20220120 : Amlogic需要收SBIN
            Source.b2DIDNG[i][j]        =false;                                 //Steven 20200611 : for Murata, 2DID NG不測試

            Source.iWhichSite[i][j]     =-1;
            Source.iWhichAuto[i][j]     =-1;
            Source.iWhichIndex[i][j]    =-1;                                    //ChungHung 20150205 add for ATK

            Source.bPass[i][j]          =false;
            Source.iCleanCount[i][j]    =0;
            Source.bFliped[i][j]        =false;

            Source.iBinData[i][j]       =-1;
            Source.iCurrRotAng[i][j]    =0;                                     //Steven 20170425 (wei) : Add rotate motor
            Source.iNeedRotAng[i][j]    =0;

            Source.bQATray[i][j]        =false;
            Source.iAOIResult[i][j]     =0;                                     //Sam 20240325 : 新增 DamageTrayMapping 功能
        }
    }
}

// -- golden MyKitSuck.cpp:1017-1030  TMyKitSuck::Tested()
//    FAITHFUL: golden body verbatim (including golden's own commented-out upper
//    bound), implicit `this->` spelled `kit.`.
static bool W7Ck4_Tested(TMyKitSuck &kit)
{
    for(int i=0; i<kit.iMaxRow; i++)
    {
        for(int j=0; j<kit.iMaxCol; j++)
        {
            if(kit.Item[i][j]>=START_TEST)                                          // && kit.Item[i][j]<=TEST_FAIL8)             //Steven 20180522 : mark for [D52]導致Hang up問題
            {
                return true;
            }
        }
    }
    return false;
}

// -- golden MyKitSuck.cpp:2732-2746  TMyKitSuck::CheckVaccumIsIniaialON
//    FAITHFUL: golden body verbatim, implicit `this->` spelled `kit.`.  Touches
//    only Suck[][].GetStatus()/Normal() and Item[][], all real members.  Golden's
//    own three explanatory lines above the body (golden MyKitSuck.cpp:2729-2731)
//    are transcribed with it.
// if sensor Off  ,off vacuum ,flag not chage
// if        On   ,if has not Real IC then off vacuum,flag=true
//                 else do not change
static void W7Ck4_CheckVaccumIsIniaialON(TMyKitSuck &kit, int iRow, int iCol, bool &flag)
{
    if(kit.Suck[iRow][iCol].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（黏料／殘料／不該有料）
    {
        if(kit.Item[iRow][iCol]==NULL_IC || kit.Item[iRow][iCol]==HAS_NULL_IC)
        {
            kit.Suck[iRow][iCol].Normal();
            flag=true;
        }
    }
    else
    {
        kit.Suck[iRow][iCol].Normal();
    }
}

// -- S1: golden note.cpp:4286-4289  NowNoteIsShow()  -- FAITHFUL golden body.
static bool W7Ck4_NowNoteIsShow()                                              //ChungHung 20130110 add
{
    return W906_FormShowing("fNote", fNote->fShow);  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
}

// -- S3 and S2: offline "OK" stand-ins; see GATE REGISTER S3 / S2 for why 1 is
//    the faithful value and what a real machine loses.
static int  W7Ck4_RecordEndTestTime(int iWhich) { extern int RecordEndTestTime(int); return RecordEndTestTime(iWhich); }   // golden (Sam 20201231); AI(W906-ELA-W48B) 20260928 (St02-E, laptop-approved claim): the real stamp, cObserver.cpp:3111
static int  W7Ck4_SendTestResultToHttp()            { return 1; }               // golden :2233 forward-decl only

// -- S4: the two fMain widgets TfMain does not have.  Golden derefs them as
//    POINTERS (fMain->BtnZUpDown->Down, fMain->lbArm0Torque->Caption), so the
//    macros yield &object and golden's `->` pattern survives untouched.
//    Classes chosen to match golden main.h: BtnZUpDown TBtnPanel* (Down/Enabled
//    only -> TSpeedButton carries Down, TControl carries Enabled), lbArm0Torque
//    main.h:798 TPanel* (Caption only) -- the same class pair atester.cpp:5563 and
//    atester_32Site.cpp:276 already chose for lbArm0Torque.
static TSpeedButton W7Ck4_BtnZUpDown;                                          // golden main.h (TBtnPanel*) -- ->Down / ->Enabled only
static TPanel       W7Ck4_lbArm0Torque;                                        // golden main.h:798 (TPanel*) -- ->Caption only
#define W7Ck4_FMAIN_BTNZUPDOWN     (&W7Ck4_BtnZUpDown)
#define W7Ck4_FMAIN_LBARM0TORQUE   (&W7Ck4_lbArm0Torque)

//---------------------------------------------------------------------------
// ==== GOLDEN aTester_Front.cpp :2237-3941 BEGINS HERE -- verbatim except the
//      call-site redirects and the eight #if 0 gates documented above ====
bool DoFTestSuckTestIC()
{
    static int iTestCount=0, iReadTIntervel=0, iReadCount=0, iXMLRetryCnt=0;
    static int iDoubleCount=0;                                                  //ChungHung 20140709 add for SPIL
    static int iArm1ReadtorquCount=0;                                           //kevin 20210902 add 扭力讀取
    static bool ShowTestStatus=false;                                           //ChungHung 20150526 add for QualComm US
    static bool bFirstIn=true;                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將out shuttle lose IC對應的site設成error bin
    static DWORD dwNowTickCount, dwOldTickCount=0;
    static DWORD dwStartAfterTestCount=0;                                       //ChungHung 20140730 add for ATK function after tested delay time

    int &Task=iFTestSuckTestICTask, ret, iMaxDoubleContact, ret2;
    int iMaxPreasure=0, iErrCnt=0, iTestCh=0;
    int iNN=IsNNMode();
    bool bHasFailIC=false;
    bool bManualStep=false, bManualTStart=false;                                //ChungHung 20150526 add for QualComm US
    bool bATCSiteTest[32];
    bool bAlreadyTested;
    DWORD dwEndAfterTestCount=0;                                                //ChungHung 20140730 add for ATK function after tested delay time
    AnsiString ErrPart="", ErrCnt="";                                           //kevin 20130418
    AnsiString SData="@e02019Arm1,sideA";                                       //kevin 20191029 add loadcell reaad
    int iDbContactHigh=1000;                                                    //RogerYang 20260126 : JSCC_OS 第二次contact要拉高慢放
    int iDbContactHighSlowSpd;
    //==> Eastsun 20260511 F006 整合: T8 case 8000~9101 所需區域變數
    bool bSuck_OK=false;                                                            //Ifor 20200622 add:T8 case 8001 真空判定結果
    bool flag2=false;                                                               //T8 case 8003 內 vac initial 旗標
    int iIndexUpPos=0;                                                              //T8 case 8002 Above Socket Z 高度
    static AnsiString AllErrPart="";                                                //T8 case 8003/8010 累計錯位字串
    int i=0, j=0;                                                                   //T8 case 8000~9001 for-loop index
    //<== Eastsun 20260511 F006 整合

    switch(Task)
    {
        case 1:
            iXMLRetryCnt=0;                                                     //Steven 20201102 : For Murata資料上拋
            iArm1ReadtorquCount=0;                                              //kevin 20210902 add 扭力讀取

            if(CUSTOMER_CODE==CC_KYEC_XILINX &&
               IniConfig.bD01EnableReadTorque)                                  //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
            {
                bReadFrontTestArmTorque=true;
            }
            else                                                                //Steven 20100617 Start: Add form 9080A for 即時更新扭力值
            {
                if(IniConfig.bD01EnableReadTorque &&
                   IniConfig.iD01ReadTorqueTimeCount>0)
                {
                    iReadTIntervel++;
                    if(iReadTIntervel>=IniConfig.iD01ReadTorqueTimeCount)       //kevin 20130611   if(iReadTIntervel>LastSet.iReadTorqueTimeCount)
                    {
                        iReadTIntervel=0;
                        bReadFrontTestArmTorque=true;
                    }
                }
            }

            if(CosFunction.bSocketSensorCheckICAtArmDown==true)
            {
                CheckICExistInSocket(__FUNC__);
            }
            #ifdef SOFT_SIMULTE
            bReadFrontTestArmTorque=false;
            #endif
            if(IniConfig.bD02OffReadTorqueDuringTest==true)
                bReadFrontTestArmTorque=false;

            if(CosFunction.bHiSiliconFunction==true ||                          //Ifor 20190912 :add 海思 V02.30 版 Record Torque
               CUSTOMER_CODE==CC_KYEC_LEE)
            {
                bFrontTestArmTorqueFinish=false;
                bRearTestArmTorqueFinish=true;
                W64B_FMAIN_CHKREADTORQUE1->Checked=true;
                W64B_FMAIN_CHKREADTORQUE2->Checked=false;
                W64B_FMAIN_EDTORUE0->Text="";
                Task=100;
            }
            else
            {
                if(bReadFrontTestArmTorque)
                {
                    bReadFrontTestArmTorque=false;
                    W64B_FMAIN_CHKREADTORQUE1->Checked=true;
                    W64B_FMAIN_CHKREADTORQUE2->Checked=false;
                    Task=5000;
                    break;
                }
                else
                {
                    Task=100;
                }
            }
        case 100:                                                                                                       //kevin 20130611 fix
            bAlreadyTested=false;                                                                                       //Steven 20170517 (jou) : 新增保護,如果IC已經測過就離開
            ErrPart="";
            iTestCh=0;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]!=NULL_IC     &&
                       FTestSuck.Item[i][j]!=HAS_NULL_IC)
                    {
                        if(FTestSuck.Item[i][j]>=TEST_PASS)
                            bAlreadyTested=true;

                        if((CosFunction.bBarcodeErrNoTestAndShowH==true ||                                              //jou 20191007 : Barcode Error No Test & Show "H"
                            TestIF_File.iNoCodeDeviceToErr==2) &&                                                       //Steven 20200909 : 將2DID all site fail變成選項
                           TestIF_File.bEnableBarCode==true &&
                           (FTestSuck.cDeviceInf[i][j]==asBarCodeErrorSend ||
                            FTestSuck.cDeviceInf[i][j]==""))
                        {
                            ;
                        }
                        else
                        {
                            iTestCh++;                                                                                  //Steven 20250318 : 先統計有幾個site要測試
                        }
                    }
                }
            }

            if(bResetModeAndCleanOut && IniConfig.bI49_TesterTimeOutResetAllIC)                                         //Sam 20250916 : [I49] Restet 已測 IC 不需要報警
                bAlreadyTested=false;                                           //Steven 20260612 : Fix == to = (was comparison, not assignment)

            if(bAlreadyTested==true)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        ErrPart+=IndexSuckName[i+iNN][j]+", ";
                        if(FTestSuck.Item[i][j]!=NULL_IC     &&
                           FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                           FTestSuck.Item[i][j]<TEST_PASS)
                        {
                            FTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                        }
                    }
                }
                ShowMyMessage("IC in index already tested, abort process.", ErrPart);
                return true;
            }

            if(iTestCh==0)                                                                                              //Steven 20250318 : 如果都沒要測試就return true;
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]!=NULL_IC &&
                           FTestSuck.Item[i][j]<TEST_PASS &&
                           FTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            if((CosFunction.bBarcodeErrNoTestAndShowH==true ||                                          //jou 20191007 : Barcode Error No Test & Show "H"
                                TestIF_File.iNoCodeDeviceToErr==2) &&                                                   //Steven 20200909 : 將2DID all site fail變成選項
                               TestIF_File.bEnableBarCode==true &&
                               (FTestSuck.cDeviceInf[i][j]==asBarCodeErrorSend ||
                                FTestSuck.cDeviceInf[i][j]==""))
                            {
                                FTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                FTestSuck.iBinData[i][j]=iTestBinCount;
                                FTestSuck.PordRec[i][j].AddTestResultRecord(iTestBinCount, FTestSuck.cSBin[i][j], "NonTestToRBin");
                            }
                        }
                    }
                }
                return true;
            }

            Task=200;
        case 200:
            W7Ck4_CopyFrom(TestSocket, FTestSuck);
            bDoubleContact=false;
            InitTestTask();
            ShowTestStatus=false;                                               //ChungHung 20150526 add for QualComm US
            iTestCount=0;
            ZeroMemory(iReContactCnt, sizeof(iReContactCnt));                   //Steven 20231205 : 計算某site contact 次數

            if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)                  //Sam 20190429 : Add CC_PTI_NEWWORK
            {
                InitDoStartMode();                                              //ChungHung 聚成 Init Test Start
            }

            Task=2200;
//            break;                                                            //jou 2015-08-05 Index Arm1 多了一個break;
        case 2200:
            if(CUSTOMER_CODE==CC_MTI ||                                         //ChungHung 聚成 Test Start
               CUSTOMER_CODE==CC_PTI)                                           //Sam 20190429 : Add CC_PTI_NEWWORK
            {
                if(TestIF_File.iShuttleMode==0 ||
                   (TestIF_File.iShuttleMode==1 &&
                    TestIF_File.iShuttle_Sel==0))                               //Sam 20200214 : 修正關 Arm 不需要送通訊
                {
                    if(DoStartMode(fAutomation->TestMode)==false)
                        return false;
                }
            }
                                                                                //ChungHung 20150526 add for QualComm US
            if(IniConfig.bD37EnableManualProcess)
            {
                bHangTimePause=true;
                if(CosFunction.bEnableSoftWareControlButton)                    //ChungHung 20150609 add only for TSMC
                {
                    fMain->BtnT_Start->Enabled=true;
                }

                if(WaitManualStartKey()==false)
                    return false;
                bHangTimePause=false;
                if(CosFunction.bEnableSoftWareControlButton)                    //ChungHung 20150609 add only for TSMC
                {
                    fMain->BtnT_Start->Enabled=false;
                    fMain->BtnT_Start->Color=(TColor)0x00804000;                //ChungHung 20150609 add only for TSMC
                }
            }

            ProcessStartTestData(0);                                            //畫面顯示黃色，測試中
            SetTestTimeOutTimer(0);                                             //Steven 20200407 : 整合Time Out時間設定

            HangTime.SetSecAndOn(Prod.iHangupMaxTime);

            if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)                  //Sam 20190429 : Add CC_PTI_NEWWORK
            {
                InitDoEndMode();                                                //ChungHung 聚成 only
            }

//            ShowIndexTime(1);                                                 //Steven 20140619 : 測試      //到這裡大概0.00~0.02Sec
            if(IniConfig.bL10IndexTestlogTemp &&
               (TestIF_File.iShuttleMode==0 ||
               (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)))   //kevin 20190323 : index 測試時才記錄溫度
            {
                TemperatureStorageLog(1);                                       //kevin 20190323 add Steven 20140617 : for 海思
            }

            if(IniConfig.bD67LoadCellMeasure)
            {
                SData="@e02019Arm1,sideA";
                RespondASECom(SData);                                           //kevin 20191029 add
                bloadcellRece=false;
                sLoadCellReceData="";
                iloadcellRece=0;                                                //kevin 20190906
            }
            Task=2400;
        case 2400:                                                                                                                                              // 編號不得改變 *******************
            if(dwOldTickCount==0 || bNeedInitialTestDelay)                                                                                                      //ChungHung 20140619 add TSMC Function 不算在TimeOut time 內
            {
                dwOldTickCount=MyTickCount();
                dwNowTickCount=dwOldTickCount;
                SetTestTimeOutTimer(0);                                                                                                                         //Steven 20200407 : 整合Time Out時間設定
                HangTime.SetSecAndOn(Prod.iHangupMaxTime);
            }
            else
            {
                dwNowTickCount=MyTickCount();
                if(dwNowTickCount<dwOldTickCount)
                {
                    dwOldTickCount=dwNowTickCount;
                    SetTestTimeOutTimer(0);                                                                                                                     //Steven 20200407 : 整合Time Out時間設定
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);                                                                                                  //Jou 20101018 Start : 重新設定Time Out時間時,Hang Up時間也要重設
                }
                else
                {
                    if(bHandlerPause ||                                                                                                                         // pause happened
                       bTesterSendPause)                                                                                                                        //kevin 20161101 check problem time out reset
                    {
                        if(CosFunction.bStopMustTestTimeOut==false ||
                           bEnterTestIF==true ||
                           bTesterSendPause)                                                                                                                    //ChungHung 20121221 add
                        {
                            bEnterTestIF=false;
                            SetTestTimeOutTimer(0);                                                                                                             //Steven 20200407 : 整合Time Out時間設定
                        }

                        HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    }
                    dwOldTickCount=dwNowTickCount;
                }
            }

            if(bDoOverDrive ||                                                                                                                                  //Steven 20151207 : OverDrive for TSMC
               bDoReContact)                                                                                                                                    //Steven 20151207 : Recontact for TSMC
            {
                iDoubleCount=0;                                                                                                                                 //jou 20230823 : 修正GPIB ReContact 命令hang up異常
                Task=6000;
                return false;
            }

            if(IniConfig.bD23EveryDeviceDoubleContactFirstNoTesting)                                                                                            //ChungHung 20140709 add for SPIL
            {
                iDoubleCount++;                                                                                                                                 //ChungHung 20140709 add for Spil 第一次Contact 不測試 第二次才測
                if(iDoubleCount<IniConfig.iD23_MultiContactCount)                                                                                               //Steven 20151001 : Add for TSMC
                {
                    Task=3000;
                    return false;
                }
            }

            if((TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true ||
                TestIF_File.bIndexDropICSetErrUntilOneCycle) &&                                                                                                 //JerryYang 20220923 : index arm drop error設ERROR BIN
               bFirstIn==true)                                                                                                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將out shuttle lose IC對應的site設成error bin
            {
                bFirstIn=false;
                if(bOutShtLoseICNeedSetErrBin==true ||
                   bIndexDropICNeedSetErrBin==true)                                                                                                             //JerryYang 20220923 : index arm drop error設ERROR BIN
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(bTestSiteNeedSetErrBin[i+2][j]==true &&
                               TestSocket.Item[i][j]!=NULL_IC       &&
                               TestSocket.Item[i][j]!=HAS_NULL_IC   &&
                               TestSocket.Item[i][j]<TEST_PASS      )
                            {
                                TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                iErrCnt++;
                            }
                        }
                    }

                    if(iErrCnt>0)
                    {
                        if(bOutShtLoseICNeedSetErrBin)                                                                                                          //JerryYang 20220923 : add index arm drop error設ERROR BIN
                        {
                            ErrCnt.sprintf("Out Shuttle lose IC, Arm 1 set to error Bin (before test) : %d pcs", iErrCnt);
                            MyDBIProcess("Message", ErrCnt);
                        }
                        else
                        {
                            ErrCnt.sprintf("Index Arm Drop error, Arm 1 set to error Bin (before test) : %d pcs", iErrCnt);
                            MyDBIProcess("Message", ErrCnt);
                        }
                    }
                }
            }
            bFinshTest=false;

            if(IniConfig.bI27_ManualSortMode &&                                                                                                                 //Steven 20150915 : For TSMC 手動整盤功能
               LastSet.iTester==OFF_LINE &&
               bRunManualSortMode==true)
            {
                ret=1;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(TestSocket.Item[i][j]!=NULL_IC     &&
                           TestSocket.Item[i][j]!=HAS_NULL_IC)
                        {
                            iTesterBIN[i][j]=TestSocket.iWhichAuto[i][j];
                            TestSocket.iBinData[i][j]=iTesterBIN[i][j];
                            TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTesterBIN[i][j], TestSocket.cSBin[i][j]);                                             //Frank 20160505 add
                        }
                    }
                }
            }
            else if(bIndexArmNoTestting ||                                                                                                                      //ChungHung 20140730 add ContinuousFailHaveOneCycle add bIndexArmNoTestting
                    bResetMode ||                                                                                                                               //jou 981219 start : reset mode
                    bResetModeAndCleanOut ||                                                                                                                    //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
                    //bResetModeAndCleanOutAndNoTest ||                         //Jimmychiu 20210927 Reset clean out with no tester
                    (bOneTimeHotPlateCheckAll &&
                     IniConfig.bE39_1PutTheDevicesToErrorBin &&
                     bHotPlateCheckNeedTrayFeed))                                                                                                               //Steven 20120315 : CleanOut後的HotPlate Check, 要放到Error Bin
            {
                ret=1;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(TestSocket.Item[i][j]!=NULL_IC     &&
                           TestSocket.Item[i][j]!=HAS_NULL_IC &&
                           TestSocket.Item[i][j]<TEST_PASS)
                        {
                            if(bDoEmptySocketOneCycle)                                                                                                          //Steven 20220817 : Bin of ESC function iTestBinCount --> IniConfig.iI41_BinOfESC
                            {
                                iTesterBIN[i][j]=IniConfig.iI41_BinOfESC;
                            }
                            else
                            {
                                iTesterBIN[i][j]=iTestBinCount;                                                                                                 //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                            }

                            TestSocket.iBinData[i][j]=iTesterBIN[i][j];                                                                                         //Steven 20220830 : Add for ESC function
                            TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                            TestSocket.PordRec[i][j].AddTestResultRecord(iTesterBIN[i][j], TestSocket.cSBin[i][j], "RESET_Mode");                               //Frank 20160505 add
                        }
                    }
                }
            }
            else
            {
                if(DeviceForm.DummyMode)                                                                                                                        //contect form
                {
                    if(iTestCount<1)
                    {
                        ret=1;
                    }
                    else
                    {
                        ret=ProcessTestResult(0);
                    }
                }
                else
                {
                    ret=ProcessTestResult(0);
                }
            }

            if(TestSocket.HasRealIC()==false)                                                                                                                   //Steven 20151016 : Fixed for Hang up while no devices in socket.
            {
                ret=1;
            }
            iWhichIndexArm=1;                                                                                                                                   //Sam 20231214 : Temp offset use ready temp range
            if(ret==1)
            {
                if(IniConfig.bD23EveryDeviceDoubleContactFirstNoTesting)
                    iDoubleCount=0;                                                                                                                             //ChungHung 20140709 add for SPIL
                bFirstIn=true;                                                                                                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將out shuttle lose IC對應的site設成error bin
                /*
                if(Temperature.bCoolDownAfterDelayTForInitial)                                                                                                  //Sam 20240416 : 新增 Initial Temp. Ofs 開始測試時，延遲時間後降溫。
                {
                }
                else
                */
                {
                    if(TestIF_File.iShuttleMode==1 &&
                       TestIF_File.iShuttle_Sel==1)                                                                                                             //Ifor 20220317 add:關Arm不累積Contact次數)
                    {
                    }
                    else
                    {
                        if(IniConfig.bL28TempOfsUseReadyTempRange &&
                           bEnable_KLT_Function==false)                                                                                                         //Sam 20231214 : Temp offset use ready temp range
                        {
                            if(TestSocket.HasRealIC())
                            {
                                iInitContactCount++;
                                if(iInitContactCount<(Temperature.iCintactCntForTempOffsetAtInitial+Temperature.iCintactDelayCntForInitTempOffset))
                                {
                                    fHeaterOK=false;
                                    bNeedReCheckHeat=true;
                                }
                            }
                        }
                        else
                        {
                            iInitContactCount++;                                                                                                                //Steven 20141117 : 起測時溫度要補Offset
                        }
                    }
                }

                bTestingStopAllMotor=false;                                                                                                                     //jou 2013-09-25 Testing Need Stop All Motor
                bTestDuplicateErr=false;                                                                                                                        //Steven 20120102 : 測試ok要重置Flag
                bInitialMaxTime=false;                                                                                                                          //jou 2011-11-09 增加initial max time set
                SetHangupMaxTime();                                                                                                                             //Wei 20230224 : 重置死機時間
                if(iInitContactCount>Temperature.iCintactCntForTempOffsetAtInitial)                                                                             //kevin 20160312
                {
                    bTestOverTimeTempOffsetF=false;
                }

                if(iInitContactCount==Temperature.iCintactCntForTempOffsetAtInitial)                                                                            //Ifor 20190605 :add ATC Init Offset 設定
                {
#if 0 // [W7Ck4 GATE G1] golden :2699  fLotInfo->SetATCOffset(bool) -- see GATE REGISTER in this part's banner
                    fLotInfo->SetATCOffset(true);                                                                                                               //Ifor 20190605 :add ATC Init Offset 設定
#endif
                    iInitContactCount++;
                }

                if(bloadcellRece)                                                                                                                               //kevin 20191029 add load cell read
                {
                    if(iloadcellRece==2)                                                                                                                        //kevin 20190906 add load cell command
                    {                                                                                                                                           //kevin 20190906 add load cell NG
                        ShowErrorMessage("WAR0309", K_SKIP, MMSystem, false, sLoadCellReceData);
                        iloadcellRece=0;
                    }
                }

                if(bATCHasAlarmBinNeedToError)                                                                                                                  //Ifor 20160726 add 發生 ATC 異常時需將測中IC放至Error Bin
                {
                    iDoInterFaceErrorStepTask=1;
                    bContactCTOverCHK=false;                                                                                                                    //Ifor 20160726 避免ATC異常IC放至Error Bin 流程被影響到
                    Task=2500;                                                                                                                                  //不跑下面流程
                    break;
                }

                if(CUSTOMER_CODE==CC_MTI ||
                   CUSTOMER_CODE==CC_PTI)                                                                                                                       //Sam 20190429 : Add CC_PTI_NEWWORK
                {
                    if(TestIF_File.iShuttleMode==0 ||                                                                                                           //Sam 20200214 : 修正關 Arm 不需要送通訊
                       (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0))
                    {
                        if(DoEndMode(fAutomation->TestMode)==false)                                                                                             //ChungHung 聚成 Test End
                            return false;
                    }
                }

                if(IniConfig.bEnableSocketCommunication)                                                                                                        //ChungHung 20130112 add for ASE_KR Socket Tester
                {
#if 0 // [W7Ck4 GATE G2] golden :2733  fSocketCommunication->bSendBinMapReport -- see GATE REGISTER in this part's banner
                    fSocketCommunication->bSendBinMapReport=true;
#endif
                }

                SW[SwTesterPower].Off();
                bHasFailIC=false;

                if(IniConfig.bD37EnableManualProcess)                                                                                                           //ChungHung 20150526 add for QualComm US
                {
                    bHangTimePause=true;
                    if(ShowTestStatus==false)
                    {
                        ShowTestStatus=true;
                        ProcessShowTestStatus(0);
                    }

                    if(CosFunction.bEnableSoftWareControlButton)                                                                                                //ChungHung 20150609 add only for TSMC
                    {
                        fMain->BtnT_Start->Enabled=true;
                        fMain->BtnSTEP->Enabled=true;
                        W7Ck4_FMAIN_BTNZUPDOWN->Enabled=true;
                    }
                    bManualStep=WaitManualStepKey();
                    if(bManualStep==false)
                        bManualTStart=WaitManualStartKey();
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    if(bManualTStart)
                    {
                        SetTestTimeOutTimer(0);                                                                                                                 //Steven 20200407 : 整合Time Out時間設定

                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                if(TestSocket.Item[i][j]!=NULL_IC &&
                                   TestSocket.Item[i][j]!=HAS_NULL_IC)
                                {
                                    TestSocket.SetItemData(i, j, HAS_IC);
                                }
                            }
                        }

                        if(CosFunction.bEnableSoftWareControlButton)                                                                                            //ChungHung 20150609 add only for TSMC
                        {
                            if(W7Ck4_FMAIN_BTNZUPDOWN->Down)
                            {
                                ShowTestStatus=false;                                                                                                           //ChungHung 20150526 add for QualComm US
                                Task=3000;
                            }
                            else
                            {
                                InitTestTask();
                                ProcessStartTestData(0);                                                                                                        //畫面顯示黃色，測試中
                                ShowTestStatus=false;                                                                                                           //ChungHung 20150526 add for QualComm US
                                Task=2400;
                            }
                            fMain->BtnT_Start->Enabled=false;
                            fMain->BtnSTEP->Enabled=false;
                            W7Ck4_FMAIN_BTNZUPDOWN->Enabled=false;
                            fMain->BtnSTEP->Color=(TColor)0x00804000;
                            fMain->BtnT_Start->Color=(TColor)0x00804000;
                        }
                        else
                        {
                            InitTestTask();
                            ShowTestStatus=false;                                                                                                               //ChungHung 20150526 add for QualComm US
                            Task=2400;
                        }
                        bHangTimePause=false;
                        return false;
                    }

                    if(bManualStep==false && bManualTStart==false)
                        return false;
                    bHangTimePause=false;
                    if(CosFunction.bEnableSoftWareControlButton)                                                                                                //ChungHung 20150609 add only for TSMC
                    {
                        fMain->BtnT_Start->Enabled=false;
                        fMain->BtnSTEP->Enabled=false;
                        W7Ck4_FMAIN_BTNZUPDOWN->Enabled=false;
                        fMain->BtnSTEP->Color=(TColor)0x00804000;
                        fMain->BtnT_Start->Color=(TColor)0x00804000;
                        W7Ck4_FMAIN_BTNZUPDOWN->Down=false;
                    }
                }

                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(IniConfig.bD22_4_PassBinCanDoubleContact)                                                                                            //JerryYang 20230909 : pass bin也可以設定Double contact
                        {
                            if(TestSocket.Item[i][j]!=NULL_IC &&
                               TestSocket.Item[i][j]!=HAS_NULL_IC &&
                               TestSocket.bNeedReTest[i][j])
                            {
                                bHasFailIC=true;
                            }
                        }
                        else
                        {
                            if(TestSocket.Item[i][j]!=NULL_IC &&
                               TestSocket.Item[i][j]!=HAS_NULL_IC &&
                               TestSocket.bPass[i][j]==false  &&
                               TestSocket.bNeedReTest[i][j])
                            {
                                bHasFailIC=true;
                            }
                        }
                    }
                }

                if(bHasFailIC ||
                   DeviceForm.DummyMode ||
                   bNeedReplunge_RFMD)                                                                                                                          //Steven 20201022 : For RFMD
                {
                    iTestCount++;
                    iMaxDoubleContact=2;
                    if(Prod.bD22SupportMultiDoubleContact)
                    {
                        if(CUSTOMER_CODE==CC_TERAPOWER &&                                                                                                       //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                           iTestCount==1)
                        {
                            for(int i=0; i<TestSocket.iShtRow; i++)
                            {
                                for(int j=0; j<TestSocket.iShtCol; j++)
                                {
                                    TestSocket.iBinDataBackUp[i][j]=TestSocket.iBinData[i][j];
                                }
                            }
                            ProcessCount(0);
                        }

                        iMaxDoubleContact=Prod.iD22DoubleContactCount+2;                                                                                        //Sam 20231117 : 整合到 QA 模式
                        if(iMaxDoubleContact<2)
                            iMaxDoubleContact=2;
                        if(iMaxDoubleContact>10)
                            iMaxDoubleContact=10;
                    }

                    if(bNeedReplunge_RFMD)                                                                                                                      //Steven 20201022 : For RFMD
                    {
                        bNeedReplunge_RFMD=false;
                        ProcessShowTestStatus(0);
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                if(TestSocket.Item[i][j]!=NULL_IC     &&
                                   TestSocket.Item[i][j]!=HAS_NULL_IC)
                                {
                                    TestSocket.SetItemData(i, j, HAS_IC);
                                }
                            }
                        }
                        bDoubleContact=true;
                        Task=3000;
                        break;
                    }
                    else if(iTestCount<iMaxDoubleContact)
                    {
                        ProcessShowTestStatus(0);
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                if(IniConfig.bD22_4_PassBinCanDoubleContact)                                                                                    //JerryYang 20230909 : pass bin也可以設定Double contact
                                {
                                    if(TestSocket.Item[i][j]!=NULL_IC &&
                                       TestSocket.Item[i][j]!=HAS_NULL_IC &&
                                       TestSocket.bNeedReTest[i][j])
                                    {
                                        TestSocket.SetItemData(i, j, HAS_IC);
                                    }
                                }
                                else
                                {
                                    if(TestSocket.Item[i][j]!=NULL_IC &&
                                       TestSocket.Item[i][j]!=HAS_NULL_IC &&
                                       TestSocket.bPass[i][j]==false  &&
                                       TestSocket.bNeedReTest[i][j])
                                    {
                                        TestSocket.SetItemData(i, j, HAS_IC);
                                    }
                                }
                            }
                        }
                        bDoubleContact=true;
                        Task=3000;
                        break;
                    }
                }

                if(TestSocket.HasRealIC())                                                                                                                      //Steven 20210218 : 修正測試時間的紀錄
                    ret2=W7Ck4_RecordEndTestTime(0);                                                                                                                  //Sam 20201231 : 修正關 Arm 後，Index Cycle time 異常。0:arm1 1:arm2 2:雙Arm
                else
                    ret2=1;

                if(DeviceForm.ContactMode==DropContact)
                    DropContactTimer1.LatchCycleTime(true);                                                                                                     //JerryYang 20170425 (wei) 第一段時間, 測試完成到另一支arm下降到drop高度

                bATC_SITE_2ND_CHECK[0]=false;
                if(ATC_SYSTEM==eNewATCSystem)                                                                                                                   //Ifor 20160509 add ATC 測試時開啟第二點溫度監控
                {
                    for(int i=0; i<iATC_Use_Heat_Count; i++)                                                                                                    //Ifor 20160516 修改ATC Heat 設定數
                        bATCSiteTest[i]=false;
#if 0 // [W7Ck4 GATE G3] golden :2938  ATC_InterfaceForm->SiteTesting(int,bool*) -- see GATE REGISTER in this part's banner
                    ATC_InterfaceForm->SiteTesting(iATC_Use_Heat_Count, bATCSiteTest);
#endif
                }
                else if(ATC_SYSTEM==eATCHonPrecType)
                {
#if 0 // [W7Ck4 GATE G4] golden :2942  ATCInterfaceForm->SendTestEnd(int) -- REAL BODY EXISTS; see GATE REGISTER (TRAP 3)
                    ATCInterfaceForm->SendTestEnd(0);
#endif
                }

                SetNoiseDelay=false;
                TestISTimeOut=false;

                if(CUSTOMER_CODE==CC_TERAPOWER &&                                                                                                               //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                   Prod.bD22SupportMultiDoubleContact)
                {
                    if(iTestCount==0)
                    {
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                TestSocket.iBinDataBackUp[i][j]=TestSocket.iBinData[i][j];
                            }
                        }
                        ProcessCount(0);
                    }
                }
                else
                {
                    ProcessCount(0);
                }

                RecordPiggyBackStartEnd(true);                                                                                                                  //jou 2011-12-26 此funtion要在ProcessCount之後

                #ifdef TEST_BIN_MISS_SIMULATE
                if(MakeNoise==true)
                {
                    if(TestSocket.Item[0][0]==(TEST_PASS+1))
                        TestSocket.SetItemData(0, 0, (TEST_PASS+2));
                    else
                        TestSocket.SetItemData(0, 0, (TEST_PASS+1));
                    MakeNoise=false;
                }
                #endif

                if(TestSocket.HasRealIC())                                                                                                                      //Steven 20210218 : 修正測試時間的紀錄
                {
                    RecordHistroy(0);
                    W7Ck4_MoveAllItem(FTestSuck, TestSocket);
                }
                bP65QAReTest=false;                                                                                                                             //Ifor 20260407 add: [P65] QA ReTest done, clear flag
                bFinshTest=true;
                bInitStartDelayNotFinish=true;                                                                                                                  //Ifor 20181220 : add Init Start Delay Time Not Finish
                bTJControlMode=false;                                                                                                                           //Ifor 20190328 : add TJ Temp Over Range
#if 0 // [W7Ck4 GATE G5] golden :2990  ATC_InterfaceForm->SendHandler2DID(int,bool) -- see GATE REGISTER in this part's banner
                ATC_InterfaceForm->SendHandler2DID(0, false);
#endif
                if(IniConfig.bTesterTimeUpErrorNeedPassword==true &&
                   CUSTOMER_CODE==CC_LINGSEN &&                                                                                                                 //jou 2012-08-28 菱生要求 Test Time Up Error 之後的兩次測試, 不管結果如何都排到R
                   iTestTimeUpErrContinueR>0)
                {
                    iTestTimeUpErrContinueR--;
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC &&
                               FTestSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                FTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                            }
                        }
                    }

                    MyDBIProcess("Message", "Arm 1 Tester Time Up error place to R Bin : "+AnsiString(FTestSuck.CountRealIC())+" pcs" );
                }
                else if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true ||                                                                                   //JerryYang 20170610 (wei) JSCC要求Out shuttle lose IC需自動one cycle,並將對應的site設為Error bin
                        TestIF_File.bIndexDropICSetErrUntilOneCycle)                                                                                            //JerryYang 20220923 : index arm drop error設ERROR BIN
                {
                    if(bIndexDropICNeedSetErrBin ||                                                                                                             //JerryYang 20220923 : index arm drop error設ERROR BIN
                       bOutShtLoseICNeedSetErrBin)
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(bTestSiteNeedSetErrBin[i+2][j]==true &&
                                   FTestSuck.Item[i][j]!=NULL_IC     &&
                                   FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                   FTestSuck.Item[i][j]!=TEST_PASS+iTestBinCount)
                                {
                                    FTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                    iErrCnt++;
                                }
                            }
                        }

                        if(iErrCnt>0)
                        {
                            if(bOutShtLoseICNeedSetErrBin)
                            {
                                ErrCnt.sprintf("Out Shuttle lose IC, Arm 1 set to error Bin(After test) : %d pcs",iErrCnt);
                                MyDBIProcess("Message", ErrCnt);
                            }
                            else
                            {
                                ErrCnt.sprintf("Index Arm Drop error, Arm 1 set to error Bin(After test) : %d pcs",iErrCnt);
                                MyDBIProcess("Message", ErrCnt);
                            }
                        }
                    }
                }

                if(ret2!=1)                                                                                                                                     //Steven 20201102 : For Murata資料上拋
                {
                    iXMLRetryCnt++;
                    DoFTestSuckTestICDelay.SetSecAndOn(60);
                    Task=7000;
                }
                else
                {
                    if(CheckContactOver())
                    {
                        iDoInterFaceErrorStepTask=1;
                        bContactCTOverCHK=true;
                        Task=2500;
                    }
                    else
                    {
//Steven 20260427 : ATK P260427-ATK-H9-01 Auto Clean count fix --
//                  bACInterval is dead (never set true). Disable redundant
//                  increment here; ProcessAutoCleanCount() handles it.
                        #if 0  // disabled by Steven 20260427 (P260427-ATK-H9-01)
                        if(IniConfig.bEnableAutoCleanFunction &&                                                                                                //------Auto Clean Start------
                           bACInterval &&                                                                                                                       //kevin 20160906
                           (LastSet.iRunStartMode!=rsmAutoSiteMap))                                                                                             //kevin 20130226 add bACInterval
                        {
                            if(TestIF.iAutoClean_Function)                                                                                                      //Auto Clean
                                iAutoClean_IndexContactCount++;
                            else
                                iAutoClean_IndexContactCount=0;
                            fMain->AutoCleanContactCountLabel->Caption=iAutoClean_IndexContactCount;
                            EnableAutoclean(false);                                                                                                             //kevin 20120501 啟動autoclean
                        }

                        #endif // P260427-ATK-H9-01
                        if(bEchoStop==true)                                                                                                                     //ChungHung 20130326 add
                        {
                            Task=2500;
                            iDoInterFaceErrorStepTask=1;
                            return false;
                        }

                        iDoubleCount=0;                                                                                                                         //ChungHung 20140709 add for SPIL       //Steven 20150709 : Fix for [D23]
                        //Ifor 20260407 add: [P65] ARM QA Mode ReTest - Arm1
                        //==>
                        if(IniConfig.bP65EnableArmQAMode && IniConfig.iP65ArmQAModeValue > 0 && iP65QAReTestCount < IniConfig.iP65ArmQAModeValue)
                        {
                            bool bAllBin1_P65=true;
                            for(int qi=0; qi<FTestSuck.iShtRow; qi++)
                            {
                                for(int qj=0; qj<FTestSuck.iShtCol; qj++)
                                {
                                    if(FTestSuck.Item[qi][qj]!=NULL_IC && FTestSuck.Item[qi][qj]!=HAS_NULL_IC)
                                    {
                                        if(FTestSuck.bPass[qi][qj]==false)
                                            bAllBin1_P65=false;
                                    }
                                }
                            }

                            if(bAllBin1_P65 && FTestSuck.HasRealIC())
                            {
                                RecordProcess("P65 QA ReTest Arm1: count="+AnsiString(iP65QAReTestCount+1)+"/"+AnsiString(IniConfig.iP65ArmQAModeValue));
                                for(int qi=0; qi<FTestSuck.iShtRow; qi++)
                                {
                                    for(int qj=0; qj<FTestSuck.iShtCol; qj++)
                                    {
                                        if(FTestSuck.Item[qi][qj]!=NULL_IC && FTestSuck.Item[qi][qj]!=HAS_NULL_IC && FTestSuck.bPass[qi][qj])
                                            TestSocket.SetItemData(qi, qj, HAS_IC);
                                    }
                                }
                                iP65QAReTestCount++;
                                bP65QAReTest=true;                              //Ifor 20260407 add: [P65] QA ReTest, notify RunTestProgram to send iLotStatus=2
                                Task=3200;
                                break;
                            }
                        }
                        //<==
                        //Ifor 20260407 add: [P65] ARM QA Mode ReTest - Arm1
                        if(CosFunction.bEnableAfterTestedDelay)                                                                                                 //ChungHung 20140730 add for ATK function after tested delay time
                        {
                            iAfterTestedCount=Prod.dAfterTestedDelay;
                            dwStartAfterTestCount=MyTickCount();
                            Task=2410;
                        }
                        //==> Eastsun 20260511 F006 整合: KYEC 包覆 D79 Purge + SECS Yiel + ATC cooling 三段（非 KYEC 機台走外層 else { return true; }）
                        else if(CUSTOMER_CODE==CC_KYEC_LEE)
                        {
                            if(CosFunction.bUseIndexPickShuttleErrNeedPurge==true     &&
                               IniConfig.bD79EnableIndexPickShuttleErrNeedPurge==true &&
                               bFTestSuckHasError==true                              ) //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
                            {
                                Task=8000;
                                bFTestSuckHasError=false;
                            }
                            else if(iSECSGEM_ConsecutiveFailureAlarm==1)             //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
                            {
                                Task=9000;
                            }
                            else if(Temperature.bATCActiveCooling==true && Temperature.bTestCompleteWaitTemp==true)
                            {
                                Task=9100;
                            }
                            else
                            {
                                return true;
                            }
                        }
                        //<== Eastsun 20260511 F006 整合
                        else
                        {
                            return true;
                        }
                    }
                }
            }
            else if(ret==2 && W7Ck4_NowNoteIsShow()==false)                                                                                                           //interface error  //ChungHung 20130110 add 防止訊息重複進入
            {
                if(bATCHasAlarmBinNeedToError)                                                                                                                  //Ifor 20160726 add 發生 ATC 異常時需將測中IC放至Error Bin
                {
                    iDoInterFaceErrorStepTask=1;
                    bContactCTOverCHK=false;                                                                                                                    //Ifor 20160726 避免ATC異常IC放至Error Bin 流程被影響到
                    Task=2500;                                                                                                                                  //不跑下面流程
                    break;
                }

                if(IniConfig.bEnableSocketCommunication)                                                                                                        //ChungHung 20130112 add for ASE_KR Socket Tester
                {
#if 0 // [W7Ck4 GATE G6] golden :3173  fSocketCommunication->bSendBinMapReport -- see GATE REGISTER in this part's banner
                    fSocketCommunication->bSendBinMapReport=true;
#endif
                }
                bFirstIn=true;                                                                                                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將Out shuttle lose IC對應未測的site設為error bin
                RecordPiggyBackStartEnd(true);
                bInitialMaxTime=false;                                                                                                                          //jou 2011-11-09 增加initial max time set
                SetHangupMaxTime();                                                                                                                             //Wei 20230224 : 重置死機時間
                SetNoiseDelay=false;
                Task=2200;
            }
            else if((LastSet.iTester==ON_LINE &&                                                                                                                //Jou 20101018
                     hFTestTimeOutDelay.Off() &&                                                                                                                //Time Out
                     W7Ck4_NowNoteIsShow()==false) ||                                                                                                                 //ChungHung 20130110 add 防止訊息重複進入   //Steven 20150713 : 整理LastSet.iTester
                    (bEcho && bTimeOutForNoFullSite==true))                                                                                                     //Steven 20141016 : FullSite的Test Time Out
            {
                if(bATCHasAlarmBinNeedToError)                                                                                                                  //Ifor 20160726 add 發生 ATC 異常時需將測中IC放至Error Bin
                {
                    iDoInterFaceErrorStepTask=1;
                    bContactCTOverCHK=false;                                                                                                                    //Ifor 20160726 避免ATC異常IC放至Error Bin 流程被影響到
                    Task=2500;                                                                                                                                  //不跑下面流程
                    break;
                }
                bFirstIn=true;                                                                                                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將Out shuttle lose IC對應未測的site設為error bin
                RecordPiggyBackStartEnd(true);
                if(IniConfig.bD52InterFaceErrHeadNeedUp)
                {
                    if(CosFunction.bStopMustTestTimeOut &&                                                                                                      //Steven 20200330 : 暫停也要Time out跟 [D52]功能衝突
                       (SoftStop || SystemStart==false))
                    {
                    }
                    else
                    {
                        iDoInterFaceErrorStepTask=1;
                        Task=2500;                                                                                                                              //不跑下面流程
                        bD52IndexArmUp=true;                                                                                                                    //JerryYang 20200804 : fix D52 & Index arm在shuttle高度預熱功能同時啟用時，發生tester timeout時會誤發handler hang up
                        break;
                    }
                }

                if((LastSet.iTester==ON_LINE &&                                                                                                                 //Steven 20150713 : 整理LastSet.iTester
                    hFTestTimeOutDelay.Off()) ||                                                                                                                //Jou 20101018
                   (bEcho && bTimeOutForNoFullSite==true))                                                                                                      //Steven 20141016 : FullSite的Test Time Out
                {
                    if(W906_FormShowing("fNote", fNote->fShow))                                                                                                                            //JerryYang 20200408 : 修正Alarm畫面佔住時,不會發出Test time out的問題  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
                        break;
                    bEcho=false;                                                                                                                                //Steven 20150306 : Fixed for FullSite的Test Time Out
                    bTimeOutForNoFullSite=false;

                    ret=ProcessTesterTimeOut(0);

                    if(ret==2)                                                                                                                                  //Retry
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
                           CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                                                                                 //Steven 20131101 : Add ASE-K12
                        {
                            InitTestTask();
                        }
                        SetNoiseDelay=false;
                        TestISTimeOut=true;
                        if(TestIF.iTestType==GPIB_MODE)
                        {
                            if(IniConfig.bRetryNoNeedRestartGpib)                                                                                               //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
                            {                                                                                                                                   //連接OT BOX不需要重開GPIB
                                if(IniConfig.bI12TesterTimerOutNotNeedReTest==false)                                                                            //Steven 20181121 : fixed time out後會發生重測
                                    InitTestTask();
                                Task=2200;
                            }
                            else
                            {
                                Task=2600;
                            }
                        }
                        else if(TestIF_File.iTestType==TTL_MODE &&
                                (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))                                                                                         //Isaac 20210309 :TTL RS232兩塊板子
                        {
                            if(IniConfig.bI12TesterTimerOutNotNeedReTest==false)                                                                                //resend SOT
                            {
                                InitTestTask();
                                Task=2200;
                            }
                            else
                            {
                                Task=2600;
                            }
                        }
                        else
                        {
                            Task=2200;
                        }

                        bTimeOutForNoFullSite=false;                                                                                                            //ChungHung 20141017 fix Full Site Test Time Out problem
                    }
                    else                                                                                                                                        //Skip
                    {
                        if(ret==1)
                            ProcessTestResult(0);

                        SetNoiseDelay=false;
                        TestISTimeOut=false;
                        ProcessCount(0);

                        if(TestSocket.HasRealIC())                                                                                                              //Steven 20210218 : 修正測試時間的紀錄
                            RecordHistroy(0);

                        if(TestSocket.UseSiteHasIC())                                                                                                           //KenHsieh 20231208 : 修改為有資料才傳，避免重複移資料導致被覆蓋為NULL
                            W7Ck4_MoveAllItem(FTestSuck, TestSocket);
                        bFinshTest=true;
                        bInitStartDelayNotFinish=true;                                                                                                          //Ifor 20181220 : add Init Start Delay Time Not Finish
                        bTJControlMode=false;                                                                                                                   //Ifor 20190328 : add TJ Temp Over Range
                        bTimeOutForNoFullSite=false;                                                                                                            //ChungHung 20141017 fix Full Site Test Time Out problem
#if 0 // [W7Ck4 GATE G7] golden :3282  ATC_InterfaceForm->SendHandler2DID(int,bool) -- see GATE REGISTER in this part's banner
                        ATC_InterfaceForm->SendHandler2DID(0, false);
#endif
                        SW[SwTesterPower].Off();
                        if(CheckContactOver())
                        {
                            iDoInterFaceErrorStepTask=1;
                            bContactCTOverCHK=true;
                            Task=2500;
                        }
                        else
                        {
                            if(CosFunction.bEnableAfterTestedDelay)                                                                                             //ChungHung 20140730 add for ATK function after tested delay time
                            {
                                iAfterTestedCount=Prod.dAfterTestedDelay;
                                dwStartAfterTestCount=MyTickCount();
                                Task=2410;
                            }
                            //==> Eastsun 20260511 F006 整合: KYEC 包覆 D79 Purge + SECS Yiel + ATC cooling 三段（非 KYEC 機台走外層 else { return true; }）
                            else if(CUSTOMER_CODE==CC_KYEC_LEE)
                            {
                                if(CosFunction.bUseIndexPickShuttleErrNeedPurge==true     &&
                                   IniConfig.bD79EnableIndexPickShuttleErrNeedPurge==true &&
                                   bFTestSuckHasError==true                              ) //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
                                {
                                    Task=8000;
                                    bFTestSuckHasError=false;
                                }
                                else if(iSECSGEM_ConsecutiveFailureAlarm==1)             //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
                                {
                                    Task=9000;
                                }
                                else if(Temperature.bATCActiveCooling==true && Temperature.bTestCompleteWaitTemp==true)
                                {
                                    Task=9100;
                                }
                                else
                                {
                                    return true;
                                }
                            }
                            //<== Eastsun 20260511 F006 整合
                            else
                            {
                                return true;
                            }
                        }
                    }
                }
            }
            break;
        case 2410:                                                              //ChungHung 20140730 add for ATK function after tested delay time
            if(iAfterTestedCount>0)
            {
                dwEndAfterTestCount=MyTickCount();
                iAfterTestedCount= Prod.dAfterTestedDelay-((dwEndAfterTestCount-dwStartAfterTestCount)/1000);
            }
            else
            {
                //==> Eastsun 20260511 F006 整合: KYEC 包覆 D79 Purge + SECS Yiel + ATC cooling 三段（case 2410 body · 非 KYEC 直接 return true）
                if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    if(CosFunction.bUseIndexPickShuttleErrNeedPurge==true     &&
                       IniConfig.bD79EnableIndexPickShuttleErrNeedPurge==true &&
                       bFTestSuckHasError==true                              )  //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
                    {
                        Task=8000;
                        bFTestSuckHasError=false;
                    }
                    else if(iSECSGEM_ConsecutiveFailureAlarm==1)             //Ifor 20240430 add:secs gem cmd Index Yiel Fail
                    {
                        Task=9000;
                    }
                    else if(Temperature.bATCActiveCooling==true && Temperature.bTestCompleteWaitTemp==true)
                    {
                        Task=9100;
                    }
                    else
                    {
                        return true;
                    }
                }
                else
                {
                    return true;
                }
                //<== Eastsun 20260511 F006 整合
                }
            break;
        case 2500:
            if(SoftStop || SystemStart==false)                                                                          //Steven 20111004
                break;

            if(DoInterFaceErrorStep(TESTZ1UP))
            {
                HangTime.SetSecAndOn(Prod.iHangupMaxTime);                                                              //ChungHung 20140516 add  使用 LastSet.bInterFaceErrHeadNeedUp 時 會發生Hangup 訊息
                bHangTimePause=true;                                                                                    //Steven 20090827 Start: Hang Up dectector
                RecordProcess("TESTZ1UP");

                if(W7Ck4_Tested(FTestSuck))                                                                                  //是否有測試完的IC
                {
                    return true;
                }
                else
                {
                    CheckIndexSuckICFallDownSetToHasNullIC(0);                  //KevinCheng 20260423 : D52功能Arm上抬IC掉落警報
                    W7Ck4_CopyFrom(TestSocket, FTestSuck);
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
                       CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                                             //Steven 20131101 : Add ASE-K12
                    {
                        InitTestTask();
                    }
                    SetNoiseDelay=false;
                    TestISTimeOut=true;
                    if(TestIF.iTestType==GPIB_MODE)
                    {
                        if(IniConfig.bRetryNoNeedRestartGpib)                                                           //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
                        {                                                                                               //連接OT BOX不需要重開GPIB
                            if(IniConfig.bI12TesterTimerOutNotNeedReTest==false)                                        //Steven 20181121 : fixed time out後會發生重測
                                InitTestTask();
                            Task=2200;
                        }
                        else
                        {
                            Task=2600;
                        }
                    }
                    else
                    {
                        Task=2200;
                    }
                    return false;
                }
            }
            break;
        case 2600:
            if(SoftStop || SystemStart==false)
                break;

            DoFTestSuckTestICDelay.SetSecAndOn(0.3);
            Task=2700;
            break;
        case 2700:
            if(DoFTestSuckTestICDelay.Off())
            {
                Task=2200;
            }
            break;
        case 3000:
            if(CosFunction.bEnableSoftWareControlButton &&
               W7Ck4_FMAIN_BTNZUPDOWN->Down)                                         //ChungHung 20150609 add only for TSMC
            {
                #ifdef SOFT_SIMULTE
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+10000, 1))
                {
                    DoFTestSuckTestICDelay.SetSecAndOn(0.5);
                    Task=3050;
                }
                break;
                #else
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+500, MOT[MTestZ1].GailSpeed))
                    Task=3100;
                break;
                #endif
            }
            else if(IniConfig.bD22DoubleContactNoNeedReContact)                 //Steven 20131202 : Double Contact不需要Index Arm上下動
            {
                InitTestTask();
                Task=2200;
            }
            else
            {
            #ifdef SOFT_SIMULTE
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+10000, 1))
                {
                    DoFTestSuckTestICDelay.SetSecAndOn(0.5);
                    Task=3050;
                }
                break;
            #else
                if(CUSTOMER_CODE==CC_JSCC_OS)                                   //RogerYang 20260126 : JSCC_OS 第二次contact要拉高慢放
                {
                    iDbContactHigh=5000;
                    if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+iDbContactHigh, MOT[MTestZ1].GailSpeed))
                        Task=3100;
                }
                else
                {
                    Task=3100;
                }
                break;
            #endif
            }
            break;
        case 3050:
            if(DoFTestSuckTestICDelay.Off())
            {
                Task=3100;
            }
            break;
        case 3100:
            if(CUSTOMER_CODE==CC_JSCC_OS)                                                                               //RogerYang 20260126 : JSCC_OS 第二次contact要拉高慢放
            {
                iDbContactHighSlowSpd=MOT[MTestZ1].GailSpeed/2;
            }
            else
            {
                iDbContactHighSlowSpd=MOT[MTestZ1].GailSpeed;
            }

            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, iDbContactHighSlowSpd))              //Alick 20161122 修正-Prod.TestZ1_Drop_Offset，開D23功能下壓會不到位造成O/S
            {
                if(CosFunction.bEnableSoftWareControlButton &&
                   W7Ck4_FMAIN_BTNZUPDOWN->Down)                                                                             //ChungHung 20150609 add only for TSMC
                {
                    W7Ck4_FMAIN_BTNZUPDOWN->Down=false;
                    ProcessStartTestData(0);                                                                            //畫面顯示黃色，測試中
                    InitTestTask();
                    Task=2400;
                }
                else
                {
                    InitTestTask();
                    if(bDoubleContact)                                                                                  //JerryYang 20220923 : After double contact use initial delay
                    {
                        RecordProcess("After double contact use initial delay");
                        bNeedInitialTestDelay=true;                                                                     //ChungHung 20140425 add for TSMC Device
                    }
                    iRTCErrorCount=0;                        //wei 20221222 RTC ARM Error
                    Task=2200;
                }
            }
            break;
        case 5000:                                                              //Steven 20100617 Start: Add form 9080A for 即時更新扭力值
            if(CUSTOMER_CODE==CC_KYEC_XILINX &&
               IniConfig.bD01EnableReadTorque &&                                //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
               IniConfig.bChangeKitNoHardStop==true &&
               IniConfig.bRemeberAutoHeight==true)
            {
                DoFTestSuckTestICDelay.SetSecAndOn(IniConfig.dD01ReadTorqueDelayTime);
                Task=5050;
            }
            else
            {
                DoFTestSuckTestICDelay.SetSecAndOn(0.01);                       // for switch read torque relay
                Task=5100;
            }
            break;
        case 5050:
            if(DoFTestSuckTestICDelay.Off())
            {
                DoFTestSuckTestICDelay.SetSecAndOn(0.01);                       // for switch read torque relay
                Task=5100;
            }
            break;
        case 5100:
            if(DoFTestSuckTestICDelay.Off())
            {
                W64B_FMAIN_CHKREADTORQUE1->Checked=true;                            //2008/06/24 lee
                W64B_FMAIN_CHKREADTORQUE2->Checked=false;                           //2008/06/24 lee

                W64B_FMAIN_EDTORUE0->Text="";
                W7Ck4_FMAIN_LBARM0TORQUE->Caption="1:Reading";
                iReadCount=0;
                DoFTestSuckTestICDelay.SetSecAndOn(5);                          //kevin 20210902 2->5//2008/07/15 lee
                Task=5300;
            }
            break;
        case 5300:
            #if 0 // GATE (W7c-I1)
            fMain->chkReadTorque1->Checked=true;                                //2008/07/15 lee
            #endif // GATE (W7c-I1)
            #if 0 // GATE (W7c-I1)
            fMain->chkReadTorque2->Checked=false;                               //2008/07/15 lee
            #endif // GATE (W7c-I1)

            if(W64B_FMAIN_EDTORUE0->Text!="")
            {
                iReadCount++;

                if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                   IniConfig.bD01EnableReadTorque)                              //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
                {
                    ShowMainScreenPresure(0);
                    if(atoi(W64B_FMAIN_EDTORUE0->Text.c_str())>=(IniConfig.dD01ReadTorque+DeviceForm_File.dZ1Torue))
                        Task=5400;
                    else
                        Task=100;
                }
                else
                {
                    if(iReadCount>1)                                            //2008/07/15 lee
                    {
                        ShowMainScreenPresure(0);

                        if(IniConfig.bControlTorque)                            //jou 2013-11-05 Index Control Torque
                            iMaxPreasure=DeviceForm.iIndexTorqueMax;
                        else
                            iMaxPreasure=Prod.iMaxPreasure;

                        if(atoi(W64B_FMAIN_EDTORUE0->Text.c_str())>=iMaxPreasure)
                            Task=5400;
                        else
                            Task=100;                                           //kevin 20130611
                    }
                    else
                    {
                        W64B_FMAIN_EDTORUE0->Text="";
#if 0 // [W7Ck4 GATE G8] golden :3584  COM2->InitReadTorueTask() -- see GATE REGISTER in this part's banner
                        COM2->InitReadTorueTask();
#endif
                    }
                }
            }
            else if(DoFTestSuckTestICDelay.Off())
            {
                W64B_FMAIN_CHKREADTORQUE1->Checked=false;                           //2008/07/15 lee
                W64B_FMAIN_CHKREADTORQUE2->Checked=true;                            //2008/07/15 lee
                Task=5350;
                iArm1ReadtorquCount++;                                          //kevin 20210902 add 扭力讀取
                DoFTestSuckTestICDelay.SetSecAndOn(5);                          //kevin 20210902 2->5
            }
            break;
        case 5350:
            if(DoFTestSuckTestICDelay.Off())
            {
                if(iArm1ReadtorquCount>3)
                {
                    ShowMainScreenPresure(0);
                    iArm1ReadtorquCount=0;
                    Task=100;                                                   //kevin 20130611
                    break;
                }
                Task=5000;
            }
            break;
        case 5400:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "DoFTestSuckTestIC 5400"))
            {
                Task=5500;
                if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                   IniConfig.bD01EnableReadTorque)                              //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
                    Task=5450;
            }
            break;
        case 5450:                                                              //Frank 20170626 add Xilinx 浮動Shuttle Kit 強制開啟[D01]
            if(IndexAlarmInArmAway()==true)
            {
                ErrPart="The test head 1, contact force over error";            //kevin 20130418
                bIsContactforce=true;                                           //kevin 20130418 contact force over 需開們確認
                ShowErrorMessage("WAR0321", K_SKIP, MTestZ1,false, ErrPart);
                fAllMotorHome=false;
                iHome=1;
                Task=1;
            }
            break;
        case 5500:
            ErrPart="The test head 1, contact force over error";                //kevin 20130418
            bIsContactforce=true;                                               //kevin 20130418 contact force over 需開們確認

            if(CosFunction.bIndexAreaOnlyCanUseSkip)                            //Steven 20141105 : Index內的所有異常都只能用Skip
                ShowErrorMessage("WAR0321", K_SKIP, MTestZ1, false, ErrPart);
            else
                ShowErrorMessage("WAR0321", K_RETRY, MTestZ1, false, ErrPart);

            fAllMotorHome=false;
            iHome=1;
            Task=1;
            break;
        case 6000:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+1000, MOT[MTestZ1].GailSpeed))
            {
                DoFTestSuckTestICDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);
                Task=6100;
            }
            break;
        case 6100:
            if(DoFTestSuckTestICDelay.Off())
            {
                if(bDoOverDrive)                                                //Steven 20151207 : OverDrive for TSMC
                {
                    Task=6200;
                }
                else                                                            //Steven 20151207 : Recontact for TSMC
                {
                    Task=6500;
                }
            }
            break;
        case 6200:                                                              //Steven 20151207 : OverDrive for TSMC
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iOverDriveDistance, MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed/100))
            {
                DoFTestSuckTestICDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);
                Task=6300;
            }
            break;
        case 6300:
            if(DoFTestSuckTestICDelay.Off())
            {
                fMain->SendMSG_CMD(MSG_CMD_OverDrive);
                bDoOverDrive=false;
                Task=2400;
            }
            break;
        case 6500:                                                              //Steven 20151207 : Recontact for TSMC
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed/100))
            {
                iDoubleCount++;
                DoFTestSuckTestICDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);
                Task=6600;
            }
            break;
        case 6600:
            if(DoFTestSuckTestICDelay.Off())
            {
                if(iDoubleCount>iReContactCount)
                {
                    fMain->SendMSG_CMD(MSG_CMD_ReContact);
                    bDoOverDrive=false;
                    bDoReContact=false;                                         //jou 20230823 : 修正GPIB ReContact 命令hang up異常
                    Task=2400;
                }
                else
                {
                    Task=6000;
                }
            }
            break;
        case 7000:                                                              //Steven 20201102 : For Murata資料上拋
            if(iXMLRetryCnt>=3)                                                 //Steven 20201113 : 上傳失敗3次要有Alarm
            {
                ret=ShowErrorMessage("WAR16321", K_RETRY|K_RESET, MTestZ1, false);
                if(ret==K_RESET)
                {
                    fMain->Reset("DoFTestSuckTestIC");
                    Task=7100;
                }
                else
                {
                    iXMLRetryCnt=0;
                    DoFTestSuckTestICDelay.SetSecAndOn(30);
                    Task=7000;
                }
            }
            else if(DoFTestSuckTestICDelay.Off())
            {
                ret2=W7Ck4_SendTestResultToHttp();

                if(ret2!=1)
                {
                    iXMLRetryCnt++;
                    DoFTestSuckTestICDelay.SetSecAndOn(30);
                    Task=7000;
                }
                else
                {
                    Task=7100;
                }
            }
            break;
        case 7100:
            iXMLRetryCnt=0;
            if(CheckContactOver())
            {
                iDoInterFaceErrorStepTask=1;
                bContactCTOverCHK=true;
                Task=2500;
            }
            else
            {
//Steven 20260427 : ATK P260427-ATK-H9-01 Auto Clean count fix --
//                  bACInterval is dead (never set true). Disable redundant
//                  increment here; ProcessAutoCleanCount() handles it.
                #if 0  // disabled by Steven 20260427 (P260427-ATK-H9-01)
                if(IniConfig.bEnableAutoCleanFunction &&                        //------Auto Clean Start------
                   bACInterval &&                                               //kevin 20130226 add bACInterval
                   (LastSet.iRunStartMode!=rsmAutoSiteMap))                     //kevin 20160906
                {
                    if(TestIF.iAutoClean_Function)                              //Auto Clean
                        iAutoClean_IndexContactCount++;
                    else
                        iAutoClean_IndexContactCount=0;
                    fMain->AutoCleanContactCountLabel->Caption=iAutoClean_IndexContactCount;
                    EnableAutoclean(false);                                     //kevin 20120501 啟動autoclean
                }

                #endif // P260427-ATK-H9-01
                if(bEchoStop==true)                                             //ChungHung 20130326 add
                {
                    Task=2500;
                    iDoInterFaceErrorStepTask=1;
                    return false;
                }

                iDoubleCount=0;                                                 //ChungHung 20140709 add for SPIL       //Steven 20150709 : Fix for [D23]
                if(CosFunction.bEnableAfterTestedDelay)                         //ChungHung 20140730 add for ATK function after tested delay time
                {
                    iAfterTestedCount=Prod.dAfterTestedDelay;
                    dwStartAfterTestCount=MyTickCount();
                    Task=2410;
                }
                else
                {
                    return true;
                }
            }
            break;                                                              //Steven 20201102 : For Murata資料上拋
        //==> Eastsun 20260511 F006 整合: Ifor 20200622+20240430 IndexPickShuttleErrPurge case 8000~8012 + KYEC SECS case 9000 + ATC cooling case 9100/9101 (T8)
        case 8000:
            for(i=0; i<MAX_Index_Row; i++)
            {
                for(j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bFTestSuckError[i][j]==true)
                    {
                        fiosetview->bIndexSuck[0][i][j]=true;
                    }
                }
            }
            Task=8001;
            break;
        case 8001:
            bSuck_OK=W64B_FIOSET_PISD1_V0();
            if(bSuck_OK==true)
            {
                if((DeviceForm.iSocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==true) ||
                   (LastSet.iD41SocketInitialICCheckPosition==1 && IniConfig.bTestIcCheckInContact==false)) //Above Socket
                {
                    Task=8002;
                }
                else
                {
                    Task=8003;
                }
            }
            break;
        case 8002:  //Above Socket
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);  //Steven 20140620 : 整合為Function
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos, iSpeedFast))
            {
                Task=8003;
            }
            break;
        case 8003:  //Check IO Status
            AllErrPart="";
            for(i=0; i<MAX_Index_Row; i++)
            {
                for(j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bFTestSuckError[i][j]==true)
                    {
                        W7Ck4_CheckVaccumIsIniaialON(FTestSuck, i, j, flag2);
                        if(flag2==true)
                        {
                            ErrPart+=IndexSuckName[i][j];
                            AllErrPart+= ErrPart;
                        }
                    }
                }
            }
            Task=8010;
            break;
        case 8010:  //Alarm
            if(AllErrPart!="")
            {
                Task=8011;
            }
            else
            {
                for(i=0; i<MAX_Index_Row; i++)
                    for(j=0; j<NEW_MAX_Index_Col; j++)
                        bFTestSuckError[i][j]=false;

                if(CUSTOMER_CODE==CC_KYEC_LEE && iSECSGEM_ConsecutiveFailureAlarm==1)   //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
                {
                    Task=9000;
                }
                else if(Temperature.bATCActiveCooling==true && Temperature.bTestCompleteWaitTemp==true)
                {
                    Task=9100;
                }
                else
                {
                    return true;
                }
            }
            break;
        case 8011:  //上升吹氣
            if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+300, MOT[MTestZ1].GailSpeed, 0))
            {
                for(i=0; i<MAX_Index_Row; i++)
                {
                    for(j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bFTestSuckError[i][j]==true)
                        {
                            FTestSuck.Suck[i][j].Off();
                        }
                    }
                }
                DoFTestSuckOffDelay.SetMSAndOn(ArmSpeed[IndexArm].dCTAirOn*1000);
                Task=8012;
            }
            break;
        case 8012:
            if(DoFTestSuckOffDelay.Off())
            {
                for(i=0; i<MAX_Index_Row; i++)
                {
                    for(j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bFTestSuckError[i][j]==true)
                        {
                            FTestSuck.Suck[i][j].Normal();
                            bFTestSuckError[i][j]=false;
                        }
                    }
                }
                ShowErrorMessage("WAR0310", K_HOME, MTestY1, false, AllErrPart);
                fAllMotorHome=false;
                iHome=1;
            }
            break;
        case 9000:
            if((TestIF_File.iShuttleMode==1) && (TestIF_File.iShuttle_Sel==1))  //Ifor 20240430 add: 關Arm 不執行
            {
                return true;
            }
            else
            {
                if(iSECSGEM_ConsecutiveFailureAlarm==1)                         //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
                {
                    iSECSGEM_ConsecutiveFailureAlarm=2;
                }

                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed))
                {
                    bInArmNeedToSafePos=true;
                    Task=9001;
                }
            }
            break;
        case 9001:
            if(bInArmNeedToSafePos==false)
            {
                EventReport(SECS_EVENT.DoSecsGemIndexFail);   //Eastsun 20260515 F018 KYEC Index Fail
                ShowErrorMessage("WAR07362", K_RETRY, MTestZ1, false, "");  //Vacuum Sensor Off Error
                #ifdef  SOFT_SIMULTE
                    iSECSGEM_ConsecutiveFailureAlarm=0;
                #endif
                return true;
            }
            break;
        case 9100:
            fHeaterOK=false;
            bNeedReCheckHeat=true;
            Task=9101;
            break;
        case 9101:
            if(fHeaterOK==true)
            {
                return true;
            }
            break;
        //<== Eastsun 20260511 F006 整合 (T8)
        }
    return false;
}

// ==========================================================================
//  PART FILE  _w7c_parts/03945_DoFRTCUseSocketFloat.txt   (label k8-small-seven)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing content.
//  Translator: AI(k8-small-seven) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 3945..4069 -- DoFRTCUseSocketFloat(bool) and
//  nothing else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD; Chinese comments
//  transcribed character for character from cp950.
//
//  ROLE
//  ----
//  DoFRTCUseSocketFloat(bool bInitial) -- the FRONT (Z1/Arm1) "use socket float"
//  RTC full-view sequence.  Parks Z1 at TestZ1_Safe, checks BOTH Z encoders,
//  drives Y1/Y2 to Front/Rear, asks the RTC vision system for a CHECKNULL +
//  full-view OK, handles the NG path (WAR0346 Retry/Skip, with an In-Arm give-way
//  precondition) and the time-out path (WAR0337), then steps Y1 to Middle,
//  presses Z1 down to test height (honouring TestZ1_Drop_Offset) and leaves
//  IndexStatus=Z1Down_Z2Up.
//
//  WAVE SCOPE
//  ----------
//    DoFRTCUseSocketFloat(bool)   ACTIVE   golden :3945-4069
//      (ONE statement inside it GATED: golden :4030 -- see GATE REGISTER G-k8s7-1)
//    NOT EMITTED, deliberately -- golden :3943 `TQPF_Timer
//      DoUseSocketTestYFrontDelay;` is one of the 16 file-scope globals the MAIN
//      LOOP owns and adds in one place.  This part only USES it.  (This is where
//      this part DIVERGES from its Rear twin, which emitted golden :3880
//      DoUseSocketTestYRearDelay itself inside marker comments at port
//      aTester_Rear.cpp:4974-4976 -- correct for that wave, wrong for this one.)
//    NOT EMITTED -- golden :3944 `int iFRTCUseSocketFloatTask=1;` already exists
//      as real data at atester_shims.cpp:160 (extern atester_shims.h:83).
//
//  GATE REGISTER
//  -------------
//  G-k8s7-SEAM  (NOT an #if 0 -- a member-substitution seam, registered so it can
//               never be mistaken for real wiring)
//    GOLDEN LINES     :3979, :3980, :3981 (case 100); :3987, :3988, :3989
//                     (case 110); :3994, :3996, :3997, :3998 (case 150 OK arm);
//                     :4002 (case 150 NG test); :4011 (case 150 time-out arm);
//                     :4020 (case 170); :4063 (case 400) -- i.e. ALL 23 `COM2->`
//                     uses (14 lines; 9 of them name COM2 twice) in the range,
//                     INCLUDING DoReleaseAndInspEnd.  Count asserted by the
//                     generator, not eyeballed.
//    WHY THE OFFLINE DEFAULT IS FAITHFUL
//                     Every RT channel reads false, so each golden branch that
//                     asks "did vision answer?" takes the golden ELSE arm that
//                     golden itself already wrote for the no-answer case -- :4007
//                     (DoUseSocketTestYFrontDelay.Off() -> WAR0337 -> Task=100
//                     retry).  No branch is invented and none is skipped.
//                     SendCommToVision / RTC_ResetAlarm / InitRealTimeCCDPara are
//                     pure serial-TX / reset side effects with no value this SM
//                     reads back, so no-op'ing them removes no decision.
//                     DoReleaseAndInspEnd is a special case and is NOT an
//                     approximation: the real port body is literally `{}`
//                     (atester_shims.cpp:363) -- it is routed through the seam
//                     only to keep this part free of the bare identifier `COM2`
//                     (see the seam block's choice (1)).
//    REAL-MACHINE DIFFERENCE
//                     On the machine the RTC vision controller receives CHECKNULL
//                     / FullTOK and answers OK or NG, so case 150 advances to 200
//                     (OK) or 170 (NG).  Under the seam it never gets an answer,
//                     so it spins its own golden time-out path 100 -> 150 -> 100
//                     raising WAR0337 every pass: THIS FUNCTION NEVER COMPLETES
//                     OFFLINE.  That is the truthful behaviour of a machine with
//                     no RTC camera, and it is what atester_32Site.cpp and the
//                     Rear twin already do -- but it does mean this SM must not
//                     be put on any ctest "must finish" path until COM2 is real.
//    ABSENCE COMMAND  Grep(pattern="Gali_ReadEncoderMaxRandge|...|rtAlarHasIC|...",
//                     path=D:/HT9045/HT9011UC_Cpp_V3.33.906.0, glob=*.h)
//                     RESULT: rtAlarHasIC -> 0 hits in any header.  Cross-checked
//                     by reading class TCOM2Shim in full (atester_shims.h:420-461):
//                     its ONLY members are bCCDDummyRum, DoReleaseAndInspEnd,
//                     ATCAlarmSenCheck, TCOM2Shim().  Note rtCHECKNULL/rtFullTOK/
//                     rtFullTNG/SendCommToVision/RTC_ResetAlarm/
//                     InitRealTimeCCDPara DO appear elsewhere in the tree, but
//                     every hit is another file's OWN TU-local seam member
//                     (atester_32Site.cpp:357-386, aTester_Rear.cpp:5017-5061) or
//                     a comment -- never a member of the real TCOM2Shim.
//    MEASURED AT      2026-08-10 21:48 +0800
//    WHY IT SHOULD STAY A SEAM EVEN IF A SIBLING LANDS SOME MEMBERS (TRAP 3)
//                     Un-gating must be all-or-nothing per object: mixing real
//                     COM2 members with seam members in one function would give
//                     the SM a half-live vision channel (some sends real, all
//                     receives still false), which is a state no machine can be
//                     in.  Retire the whole seam in one edit, together with
//                     atester.cpp's W7T1_TCOM2Ext and atester_32Site.cpp's
//                     W5_32S_TCOM2Ext, or not at all.
//  ---------------------------------------------------------------------------
//  G-k8s7-1  (#if 0)
//    GOLDEN LINE      :4030   fContact->InitROILearningTask();
//    WHY THE OFFLINE DEFAULT IS FAITHFUL
//                     fContact EXISTS in the port (TfContactShim*, class
//                     atester_shims.h:154-251, object declared :251) but has NO
//                     InitROILearningTask member.  Golden calls it on the K_SKIP
//                     arm of the WAR0346 "RTC FullView Socket Has Device" dialog,
//                     purely to rearm the contact form's ROI-learning cursor; this
//                     SM reads nothing back from it.  The two statements that
//                     actually decide behaviour on that arm
//                     (`bRTCFullViewError=false;` and `Task=200;`, golden
//                     :4031-4032) are both KEPT ACTIVE immediately below the gate,
//                     so skipping this call changes no transition here.
//    REAL-MACHINE DIFFERENCE
//                     The contact form's ROI-learning SM keeps its PREVIOUS task
//                     index instead of being reset to step 1, so the next
//                     Contact-mode ROI learn can resume mid-sequence rather than
//                     from the start.  Nothing in the index/test SM observes it.
//    ABSENCE COMMAND  Grep(pattern="...|InitROILearningTask|...",
//                     path=D:/HT9045/HT9011UC_Cpp_V3.33.906.0, glob=*.h)
//                     RESULT: 0 hits in any header -> not declared anywhere.
//                     (In *.cpp there are 7 hits and NOT ONE is a declaration or
//                     definition: 3 are atester.cpp's W7T1_FCONTACT_INITROI no-op
//                     macro + comments, 4 are atester_32Site.cpp's
//                     W5_32S_FCONTACT_INITROI no-op macro + comments.)
//    MEASURED AT      2026-08-10 21:48 +0800
//    WHY IT SHOULD STAY GATED EVEN IF THE MEMBER LANDS (TRAP 3)
//                     If TfContactShim gains InitROILearningTask this gate can
//                     retire -- but ONLY together with atester.cpp:5537 and
//                     atester_32Site.cpp:216 and aTester_Rear.cpp:5146-5148, which
//                     are the SAME golden call behind no-op MACROS (invisible at
//                     the call site).  Retiring this one alone would leave the
//                     tree with the same call live in one file and silently dead
//                     in three.
//  ---------------------------------------------------------------------------
//  SYMMETRIC TWIN and where the Front genuinely DIFFERS (do not assume mirror):
//    Twin = DoBRTCUseSocketFloat, golden aTester_Rear.cpp:3882-3999, port
//    aTester_Rear.cpp:5068-5187 (PT-W7b, label k6-BRTC-pair).  Its two GATE
//    entries (the COM2 seam, and fContact->InitROILearningTask) have the SAME
//    premises and BOTH STILL HOLD on the Front side -- re-verified above, not
//    assumed.  FOUR golden differences carried faithfully here:
//      (a) Front has an extra `case 110:` (golden :3986-3992) that re-sends the
//          vision request WITHOUT re-driving Y -- the Rear golden has no case 110
//          at all, and Front's case 170 else-arm goes to Task=110 where Rear's
//          goes to Task=100.
//      (b) Front's case-150 OK arm KEEPS `bRealTimeCom_ReceiveOK[rtAlarHasIC]
//          =false;` LIVE (golden :3996); in Rear golden that same statement is
//          commented out (its :3915).  So Front needs the extra rtAlarHasIC
//          channel in the seam -- added.
//      (c) case 200 moves `(TestY1_Middle, TestY2_Rear)` here vs
//          `(TestY1_Front, TestY2_Middle)` in Rear -- i.e. it is the FRONT Y that
//          steps to Middle.
//      (d) case 300/400 use TestZ1_Drop_Offset / TestZ1_Test / bFTestSuckDrop and
//          end at IndexStatus=Z1Down_Z2Up (Rear: Z2 fields, Z1Up_Z2Down).
//    Also note Front's :4008 puts the golden comment
//    "//Steven 20110824 : Real time CCD - 不可以關閉CCD" on the `{` line while
//    Rear golden puts it on the bSendRealCCDSendStart line -- Front's placement is
//    reproduced as golden wrote it.
//  ---------------------------------------------------------------------------
//  GOLDEN BUGS PRESERVED, NOT FIXED (all four are the twin's too)
//    * :3969-3971 the encoder guard is written
//          if(REAL_TIME_CCD==true && Z1.Gali_ReadEncoderMaxRandge(..)==false ||
//             Z2.Gali_ReadEncoderMaxRandge(..)==false)
//      which by C++ precedence is (A && B) || C -- so a Z2 encoder failure raises
//      ShowIndexMotorError even when REAL_TIME_CCD is false.  Almost certainly a
//      missing pair of parens.  KEPT VERBATIM.
//    * :3971 the Z2 encoder is compared against Prod.TestZ1_Safe (the Z1 safe
//      height), NOT Prod.TestZ2_Safe.  KEPT VERBATIM.
//    * :3947/:3964 `static bool bVerifyNG` is written and NEVER read anywhere in
//      the function -- dead state.  KEPT VERBATIM.
//    * :3955 `int ret;` is left uninitialised; it is only ever read on the one
//      path (:4028) that assigns it first (:4022/:4024).  KEPT VERBATIM.
//  INTEGER DIVISION -- there is none in this function.  No int/int is converted to
//    floating point and no ChangeToFloatNonPcnt-style helper is introduced.
//  SOFT_SIMULTE  -- this range contains no #ifdef of any kind.
//  TRAP 1 -- the body is NOT `static` (see the note directly above the definition).
//    HONEST WARNING, shape (a): golden itself has NO CALLER for
//    DoFRTCUseSocketFloat anywhere in the whole golden tree -- the name occurs in
//    golden ONLY at aTester_Front.cpp:3945 (its definition) and inside its own
//    three motion-debug string literals :3962/:3973/:3977.  Verified by scanning
//    the golden tree for the five names of this batch (only csystem.cpp,
//    cContact.cpp, aTester_Rear.cpp, aTester_Front.cpp, aTester_Front.h matched
//    anything, and within them DoFRTCUseSocketFloat matched only the definition),
//    2026-08-10 21:49 +0800.  So this body is golden DEAD CODE and stays dead
//    after stitching; it will not be extracted from the archive by any reference
//    and that is FAITHFUL, not a wiring bug.  Do NOT "fix" it by inventing a call.
//  TRAP 4 -- the one file-scope object added (`static K8S7_TCOM2Ext
//    k8s7_com2_ext`) is a constant-initialised aggregate with NO constructor, so
//    zero code runs before main() and no global pointer is touched.
//  TRAP 5 -- this part touches no object of a duplicated class.  COM2 is reached
//    only through this part's own TU-local seam; `fContact` is named only inside
//    the #if 0.  No TMyKitSuck / TMySucker / TInLaserCheck object is referenced.
//  INTEGRATE     -- there is NO shim to retire: DoFRTCUseSocketFloat is declared
//    nowhere in the port (only the cursor int iFRTCUseSocketFloatTask is), so this
//    is a net-new definition with no duplicate-definition risk.  If the sibling
//    part that translates DoTestYFront turns out to call it, that call resolves to
//    this body.
// ==========================================================================
//---------------------------------------------------------------------------
//  TU-LOCAL RTC-VISION SEAM for the COM2 members this part needs (k8-small-seven).
//  Guarded so a duplicate stitch, or a sibling part choosing the same guard, can
//  only ever define it once.
//
//  WHY: the port's real COM2 is TCOM2Shim* (class atester_shims.h:420-461, object
//  `TCOM2Shim *COM2 = new TCOM2Shim();` atester_shims.cpp:389) and exposes ONLY
//  bCCDDummyRum / DoReleaseAndInspEnd() / ATCAlarmSenCheck().  golden
//  DoFRTCUseSocketFloat additionally touches bRealTimeCom_ReceiveOK[] /
//  rtCHECKNULL / rtFullTOK / rtFullTNG / rtAlarHasIC / SendCommToVision() /
//  RTC_ResetAlarm() / InitRealTimeCCDPara().  This is the SAME gap with the SAME
//  resolution the port already uses THREE times: atester.cpp:5599-5618
//  (W7T1_TCOM2Ext), atester_32Site.cpp:357-386 (W5_32S_TCOM2Ext) and
//  aTester_Rear.cpp:5017-5061 (K6BRTC_TCOM2Ext, the Rear twin of THIS pair).
//  Offline posture is identical to all three: every RT channel reads "never
//  received" (false) so each golden `if(K8S7_COM2->bRealTimeCom_ReceiveOK[..])`
//  arm falls to its OWN already-present golden time-out/retry else-branch; the
//  three vision commands no-op.  Nothing is fabricated and no branch is invented.
//
//  TWO DELIBERATE CHOICES, both copied from the Rear twin and both still required:
//   (1) NO `#define COM2`.  atester.cpp / atester_32Site.cpp own their whole TU;
//       aTester_Front.cpp is STITCHED FROM PART FILES WRITTEN BY PARALLEL AGENTS,
//       so a TU-wide `#define COM2` would silently hijack a sibling's COM2 uses
//       (and if two siblings each emitted one, "which seam wins" would depend on
//       stitch order).  Every golden `COM2->` in this part is therefore spelled
//       `K8S7_COM2->`; this part names the bare identifier `COM2` ZERO times, so
//       it is also immune to any sibling's `#define COM2`.  The integrate step is
//       one mechanical rename back to `COM2` the day TCOM2Shim carries the members.
//   (2) CONSTANT-INITIALISED, NO CONSTRUCTOR (TRAP 4).  This is an aggregate with
//       no user-declared constructor and no non-trivially-constructible member, so
//       it is CONSTANT-initialised: ZERO code runs before main().  It cannot
//       repeat the fLaserSensor/elLaser static-init failure.
//---------------------------------------------------------------------------
#ifndef K8S7_COM2_SEAM
#define K8S7_COM2_SEAM
enum { K8S7_rtCHECKNULL=2, K8S7_rtFullTOK=3, K8S7_rtFullTNG=4,
       K8S7_rtAlarHasIC=5, K8S7_RT_N=16 };

struct K8S7_TCOM2Ext
{
    bool bRealTimeCom_ReceiveOK[K8S7_RT_N];                  // golden rs232.h -- offline all false ("vision never responds")
    int  rtCHECKNULL, rtFullTOK, rtFullTNG, rtAlarHasIC;     // golden rs232.h -- RT channel index ids
    void SendCommToVision(int /*ch*/, bool /*b*/){}          // golden rs232.h -- offline no-op (== all three precedents)
    void RTC_ResetAlarm(){}                                  // golden rs232.h -- offline no-op (== atester_32Site.cpp:370)
    void InitRealTimeCCDPara(){}                             // golden rs232.h -- offline no-op (== atester_32Site.cpp:371)
    // DoReleaseAndInspEnd IS a real member of the real TCOM2Shim -- routed here to
    // a no-op ONLY so this part never names the bare identifier `COM2` (choice (1)
    // above).  PROVABLY equivalent TODAY, not an approximation: the real body is
    // literally `void TCOM2Shim::DoReleaseAndInspEnd() {}` (atester_shims.cpp:363,
    // read 2026-08-10 21:47 +0800).  Registered as GATE G-k8s7-SEAM so the day
    // that body stops being empty this detour is FOUND, not silently kept.
    void DoReleaseAndInspEnd(){}                             // golden rs232.h:160
};
static K8S7_TCOM2Ext k8s7_com2_ext =
{
    {false, false, false, false, false, false, false, false,
     false, false, false, false, false, false, false, false},
    K8S7_rtCHECKNULL, K8S7_rtFullTOK, K8S7_rtFullTNG, K8S7_rtAlarHasIC
};
#define K8S7_COM2 (&k8s7_com2_ext)
#endif // K8S7_COM2_SEAM
//---------------------------------------------------------------------------
// ---- golden aTester_Front.cpp:3945-4069 -- DoFRTCUseSocketFloat -------------
// NOT `static`: golden gives this file-scope EXTERNAL linkage.  It is declared in
// no golden header (verified against golden aTester_Front.h, read in full) and a
// file-scope `static` definition is exactly the shadow shape (d) this wave forbids.
// See the TRAP 1 note in the report: golden has NO caller for it either.
bool DoFRTCUseSocketFloat(bool bInitial)
{
    static bool bVerifyNG=false;

    if(bInitial==true)
    {
        iFRTCUseSocketFloatTask=1;
        return false;
    }

    int ret;
    int &Task=iFRTCUseSocketFloatTask;
    AnsiString ErrPart="";

    switch(Task)
    {
        case 1:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed, "DoFRTCUseSocketFloat 1"))
            {
                bVerifyNG=false;
                Task=100;
            }
            break;
        case 100:
            if(REAL_TIME_CCD==true &&
               MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false ||
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false)
            {
                ShowIndexMotorError(AnsiString("DoFRTCUseSocketFloat"));
                break;
            }

            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, MOT[MTestY1].GailSpeed, "DoFRTCUseSocketFloat 100"))
            {
                K8S7_COM2->bRealTimeCom_ReceiveOK[K8S7_COM2->rtFullTNG]=false;
                K8S7_COM2->SendCommToVision(K8S7_COM2->rtCHECKNULL, true);
                K8S7_COM2->SendCommToVision(K8S7_COM2->rtFullTOK, true);
                DoUseSocketTestYFrontDelay.SetSecAndOn(10);
                Task=150;
            }
            break;
        case 110:
            K8S7_COM2->bRealTimeCom_ReceiveOK[K8S7_COM2->rtFullTNG]=false;
            K8S7_COM2->SendCommToVision(K8S7_COM2->rtCHECKNULL, true);
            K8S7_COM2->SendCommToVision(K8S7_COM2->rtFullTOK, true);
            DoUseSocketTestYFrontDelay.SetSecAndOn(10);
            Task=150;
            break;
        case 150:
            if(K8S7_COM2->bRealTimeCom_ReceiveOK[K8S7_COM2->rtFullTOK])
            {
                K8S7_COM2->bRealTimeCom_ReceiveOK[K8S7_COM2->rtAlarHasIC]=false;
                K8S7_COM2->RTC_ResetAlarm();     //wei 20221222 RTC ARM Error
                K8S7_COM2->DoReleaseAndInspEnd();
                bRTCFullViewError=false;                                        //Steven 20120206 : RTC重複錯誤
                Task=200;
            }
            else if(K8S7_COM2->bRealTimeCom_ReceiveOK[K8S7_COM2->rtFullTNG])
            {
                Task=170;
                break;
            }
            else if(DoUseSocketTestYFrontDelay.Off())
            {                                                                   //Steven 20110824 : Real time CCD - 不可以關閉CCD
                ShowErrorMessage("WAR0337", 0, MMIndex, 0, __FUNC__);           //RTC FullT Time Out Error.
                bSendRealCCDSendStart=true;
                K8S7_COM2->DoReleaseAndInspEnd();
                Task=100;
            }
            break;
        case 170:
            if(IndexAlarmInArmAway()==true)                                                                             //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)                                                    //Steven 20151022 : add for MAXIM
                    bIsTestSitICFallDown=true;
                K8S7_COM2->DoReleaseAndInspEnd();
                if(CosFunction.bRTCFullViewErrorOnlyRetry)                                                              //Steven 20140529 : RTCFullViewErrorOnlyRetry
                    ret=ShowErrorMessage("WAR0346", K_RETRY, MMCCD, bRTCFullViewError, ErrPart);                        //RTC FullView Socket Has Device Error!
                else
                    ret=ShowErrorMessage("WAR0346", K_RETRY|K_SKIP, MMCCD, bRTCFullViewError, ErrPart);                 //RTC FullView Socket Has Device Error!

                bRTCFullViewError=true;                                                                                 //Steven 20120206 : RTC重複錯誤

                if(ret==K_SKIP)                                                                                         //Steven 20120823 : Run Time出錯也要RTC
                {
#if 0 // GATE G-k8s7-1 -- golden :4030 fContact->InitROILearningTask() has no port body (see GATE REGISTER)
                    fContact->InitROILearningTask();
#endif // GATE G-k8s7-1
                    bRTCFullViewError=false;
                    Task=200;
                }
                else
                {
                    Task=110;
                }
            }
            break;
        case 200:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, MOT[MTestY1].GailSpeed, "DoFRTCUseSocketFloat 200"))
            {
                Task=300;
            }
            break;
        case 300:
            if(Prod.TestZ1_Drop_Offset!=0)
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, MOT[MTestZ1].GailSpeed))
                {
                    Task=400;
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test, MOT[MTestZ1].GailSpeed))
                {
                    Task=400;                                                   //Steven 20110511
                }
            }
            break;
        case 400:
            K8S7_COM2->InitRealTimeCCDPara();
            bFTestSuckDrop=false;
            IndexStatus=Z1Down_Z2Up;
            return true;
    }
    return false;
}

//==============================================================================
//  PART FILE  k5-DoFRTCAutoModelVerify  --  golden aTester_Front.cpp :4073-4918
//  Translator: AI(k5-DoFRTCAutoModelVerify) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp
//                 (8,581 lines, cp950, 8,580 CRLF / 0 bare LF -- measured
//                 20260810 21:38 +0800).  This part covers EXACTLY golden
//                 :4073-4918 (846 golden lines) and nothing else.
//  Symmetric twin already in the tree: part k5-DoBRTCAutoModelVerify, golden
//  aTester_Rear.cpp :4255-5089, landed at aTester_Rear.cpp:5658-6860.  Read that
//  banner beside this one -- every gate below states whether the Rear premise
//  still holds on the Front side or had to be re-derived.
//
//  ROLE
//  ----
//  DoFRTCAutoModelVerify(bool bInitial) -- the FRONT (Arm1 / Z1 / FTestSuck) RTC
//  "auto model verify" state machine.  With the real-time CCD (RTC/DTK vision)
//  wired it teaches the camera a new device model with no operator: park Z1 safe
//  -> split Y to Front/Rear -> push the site map to the RTC (arm selector 1, the
//  Rear twin sends 2) and run the all-pass verify -> step Z1 down in +10-count
//  release increments, releasing and re-gripping the ICs in the socket so the
//  camera sees them seated (12020..12220, with a drop-detect + JAM0303 arm at
//  12050/12055) -> re-suck and negative-pressure re-check (12300..12390) -> the
//  OPEN-LIVE / RELEASE / INSP-END / OPEN-VERIFY / ALL-FAIL-VERIFY / CLOSE-VERIFY
//  vision handshake (12400..13700) -> pick the ICs back off the socket
//  (13500/13800, JAM0301 on pick-up error) -> restore EP pressure and re-send the
//  real site map (13900..15200), then hand the machine back as IndexStatus=
//  Z1Down_Z2Up.  (Note the Front/Rear asymmetry in that last value: Front ends
//  Z1Down_Z2Up, Rear ends Z1Up_Z2Down -- each parks its own arm down.)
//
//  WAVE SCOPE
//  ----------
//    * DoFRTCAutoModelVerify(bool bInitial)   golden :4073-4918   ACTIVE
//      This part emits ZERO `#if 0` blocks.  FIVE golden symbols were absent
//      from the port and were each resolved by a NAMED seam or an extern
//      hand-off rather than a gate -- they are registered in full below and the
//      main loop MUST re-run every absence command at integration exactly as if
//      they were gates (TRAP 2).  Nothing golden calls was silently dropped.
//
//  GATE REGISTER  (per-entry command + timestamp; TRAP 2)
//  ------------------------------------------------------------------------
//  [K5F-G1]  COM2 real-time-vision surface -- bRealTimeCom_ReceiveOK[],
//            rtSiteMap / rtOpenLive / rtCloseLive / rtRelease / rtInspEnd /
//            rtInspStart / rtOPENVERIFYOK / rtOPENVERIFYNG / rtALLFAILOK /
//            rtALLFAILNG / rtCLOSEVERIFY, SendCommToVision(int,bool),
//            OpenRTCComPortAgain(), InitRealTimeCCDPara()
//            GOLDEN LINES (all 47 `COM2` lines in :4073-4918, enumerated, not
//            paraphrased): 4133 4140 4142 4474 4475 4487 4493 4495 4507 4508
//              4513 4519 4521 4526 4527 4528 4533 4534 4535 4545 4547 4557 4558
//              4563 4567 4574 4576 4581 4582 4587 4600 4607 4609 4621 4622 4623
//              4628 4629 4630 4640 4689 4694 4700 4702 4883 4889 4891 4904
//              (plus ONE commented-out COM2 line, :4642, reproduced as a comment)
//            PARTIAL-GAP NOTE (be precise): one member golden uses here,
//              DoReleaseAndInspEnd(), DOES exist on the real TCOM2Shim
//              (atester_shims.h, beside bCCDDummyRum/ATCAlarmSenCheck), so lines
//              4142 4495 4521 4528 4547 4576 4609 4623 4702 4891 would compile
//              against the real object unchanged.  The seam re-declares it as an
//              identical no-op ONLY so the rebinding of `COM2` is total and no
//              single line silently splits across two different objects.  Every
//              OTHER member above is genuinely absent.
//            WHY THE OFFLINE DEFAULT IS FAITHFUL: it is golden`s own no-camera
//              state.  bRealTimeCom_ReceiveOK[] all false == "the vision box has
//              not answered yet", exactly what golden observes on a handler with
//              no RTC camera on the port; every wait case then falls into its
//              ALREADY-PRESENT `else if(DoTestYFrontDelay.Off())` time-out/retry
//              arm, which is golden`s own real behaviour.  OpenRTCComPortAgain()
//              ==false is golden`s "re-open did not succeed" value, which
//              SUPPRESSES the ShowMyMessage popup and keeps the retry silent --
//              the conservative arm.  Nothing fabricates a verify PASS or FAIL.
//            REAL-MACHINE DIFFERENCE: with the camera present the handshake
//              completes and the SM walks 150->160->200->...->15200 and returns
//              true.  Offline it CANNOT REACH case 200 AT ALL, and the reason is
//              upstream of this seam: case 150`s SendSiteMapToRTC(true,1) live
//              body returns 0 (atester.cpp:11943-11949, its golden text gated),
//              so `!=-1` passes and the SM arms a 10 s wait and enters case 160;
//              case 160 needs COM2->bRealTimeCom_ReceiveOK[rtSiteMap], which
//              never becomes true, so after 10 s it goes back to Task=1 and the
//              SM rings 1 -> 100 -> 150 -> 160 -> 1 forever.  Everything from
//              case 200 down (including DoAllPassVerifyRTC, whose live body is
//              also `return false;`, atester.cpp:8596-8602, GATE
//              G-PTk4-DoAllPassVerifyRTC) is unreachable offline.
//              CORRECTION TO THE REAR TWIN`S BANNER, stated rather than copied:
//              aTester_Rear.cpp:5728 says the SM "PARKS AT case 200".  That is
//              imprecise for both sides -- case 200 is never entered, because
//              case 160 gates on the same absent COM2 reply.  The conclusion
//              ("the COM2 surface below is unreachable offline") is unchanged;
//              the mechanism is the 160 time-out ring, not a 200 park.
//            ABSENCE COMMAND (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//              rg -n 'bRealTimeCom_ReceiveOK|SendCommToVision|OpenRTCComPortAgain|InitRealTimeCCDPara|rtSiteMap|rtOpenLive|rtCLOSEVERIFY' atester_shims.h
//                -> NO MATCH (exit 1): none of these is a member of TCOM2Shim
//                   (atester_shims.h:420-461, the class `extern TCOM2Shim *COM2`
//                   points at).  TCOM2Shim has exactly bCCDDummyRum,
//                   DoReleaseAndInspEnd(), ATCAlarmSenCheck().
//              rg -n -w 'bRealTimeCom_ReceiveOK' -g '*.cpp' -g '*.h' .
//                -> hits ONLY inside atester.cpp / atester_32Site.cpp /
//                   csystem.cpp / aTester_Rear.cpp, and in every one of them the
//                   name binds to that file`s OWN TU-local seam plus a `#define
//                   COM2` (atester.cpp:5599-5618 W7T1_TCOM2Ext;
//                   atester_32Site.cpp:358-386 W5_32S_TCOM2Ext;
//                   aTester_Rear.cpp:5992-6017 K5_TCOM2Ext and :5376 K6BRTC).
//                   There is NO shared home to reach for.
//              MEASURED AT: 2026-08-10 21:48:20 +0800 (re-run 21:48:24 with the
//                           same result)
//            WHY IT SHOULD *STAY* SEAMED, not "fixed" by taking a dependency
//            (TRAP 3): the real home is golden rs232.h/rs232.cpp (TCOM2), an
//            untranslated serial/vision subsystem.  Widening the SHARED
//            TCOM2Shim is outside a part file`s boundary anyway, and doing it
//            for this one function would drag the RTC protocol into every one of
//            the ~180 TUs that see atester_shims.h.  The seam is the same
//            decision atester.cpp, atester_32Site.cpp and aTester_Rear.cpp have
//            each already made for this identical member family -- FOUR TU-local
//            copies now exist, which is itself the integration debt to converge
//            (see the TODO in the seam below), not a reason to widen the shim
//            from here.
//
//  [K5F-G2]  iHangupCTArm1 -- golden aTester_Front.cpp:75 `int iHangupCTArm1=0;`
//            GOLDEN LINE USED HERE: :4684 (`iHangupCTArm1=0;` in case 13500)
//            RESOLUTION: NOT gated.  This is not a foreign symbol -- it is
//              golden`s OWN file-scope counter in the very file being translated,
//              and its defining line (:75) falls OUTSIDE this part`s :4073-4918
//              range, so it belongs to the main loop`s one-place global block.
//              Dropping or gating the reset would be a real behaviour change:
//              golden aTester_Front.cpp:6536-6546 does `iHangupCTArm1++; if(
//              iHangupCTArm1>1){...}`, so failing to zero it here would make the
//              front-arm hang-up detector fire one cycle early after every
//              auto-verify.  Emitted instead as a plain `extern int
//              iHangupCTArm1;` (below) so the write stays ACTIVE and a missing
//              definition is a LOUD link error rather than a silent loss.
//            HAND-OFF (main loop): add golden :75 `int iHangupCTArm1=0;` to
//              aTester_Front.cpp EXACTLY ONCE.  The sibling translating golden
//              DoTestYFront (:5165-8006, which touches :6536-6546) needs the same
//              object -- do not let two parts each define it.  Do NOT confuse it
//              with the already-present `int iHangupCTArm1_32=0;`
//              (atester_32Site.cpp:429), which is a different object.
//            ABSENCE COMMAND: rg -n -w 'iHangupCTArm1' -g '*.cpp' -g '*.h' .
//              -> NO MATCH (exit 1) anywhere in the port tree, tests included.
//            MEASURED AT: 2026-08-10 21:48:20 +0800
//            REAR PREMISE: identical in shape to Rear`s [K5-G2] (iHangupCTArm2,
//              golden aTester_Rear.cpp:76) and re-verified independently here --
//              the Front object is a DIFFERENT name, so Rear`s measurement does
//              not cover it.
//
//  [K5F-G3]  DoTestYFrontDelay -- golden aTester_Front.cpp:4071
//            `HTimer DoTestYFrontDelay, DoTestYFrontDelay2;`
//            GOLDEN LINES USED HERE: :4128 :4138 :4299 :4305 :4320 :4325 :4334
//              :4340 :4370 :4400 :4476 :4491 :4509 :4517 :4529 :4540 :4559 :4572
//              :4583 :4605 :4624 :4635 :4690 :4698 :4777 :4782 :4878 :4887
//            RESOLUTION: NOT gated -- golden :4071 is two lines above this part`s
//              range, so it is the main loop`s line to emit.  Declared here as
//              `extern TQPF_Timer DoTestYFrontDelay;` (below).
//            *** TRAP-5-SHAPED HAZARD, READ BEFORE EMITTING :4071 ***
//              The token `HTimer` resolves to TWO DIFFERENT TYPES in this tree:
//                (a) atester_shims.h:463 `struct HTimer -- bool Off() returns
//                    true, void SetSecAndOn(double) does nothing`.  Off() is
//                    HARD-CODED true, and atester_shims.h IS included by the port
//                    aTester_Front.cpp (its line 103), so that is what a literal
//                    `HTimer DoTestYFrontDelay;` would pick;
//                (b) the tree-wide golden-HTimer convention `typedef TQPF_Timer
//                    HTimer;` (acatchtray.cpp:114, CanBus/cMyDNM100UD.cpp:85,
//                    MyPLC/MyPLC_IO_Modbus.cpp:49), and the extern TQPF_Timer
//                    form already chosen for the mirror file
//                    (aTester_Rear.cpp:340 DoTestYRearDelay).
//              Choosing (a) COMPILES AND LINKS PERFECTLY and then makes every
//              `if(DoTestYFrontDelay.Off())` in this function fire on the FIRST
//              pass -- i.e. every 10-second RTC wait would time out instantly and
//              every 0.3s/1s/3s settle delay would be skipped, silently turning
//              the release / re-grip stepping at 12100/12200/12210 into a no-dwell
//              loop and the 12220 3-second settle into nothing.  The port
//              aTester_Front.cpp already uses (b) for its own timer
//              (`TQPF_Timer FTempRiseHasICDelay;` at its line 138) -- keep that.
//              A local `typedef TQPF_Timer HTimer;` is impossible in this TU
//              because atester_shims.h has already defined the struct.
//            HAND-OFF (main loop): emit golden :4071 as
//              `TQPF_Timer DoTestYFrontDelay, DoTestYFrontDelay2;`.
//              DoTestYFrontDelay2 is NOT touched by this part (it belongs to
//              DoTestYFront, golden :5165+) -- do not add it here.
//            ABSENCE COMMAND: rg -n -w 'DoTestYFrontDelay|DoTestYFrontDelay2' -g '*.cpp' -g '*.h' .
//              -> NO MATCH (exit 1).  The 20 hits for the SIMILAR name
//                 DoTestYFrontDelay_32 (atester_32Site.cpp:560 and its uses) are
//                 a different object and do not satisfy this.
//            MEASURED AT: 2026-08-10 21:48:20 +0800
//
//  [K5F-G4]  CONTACT_NORMAL -- golden cContact.cpp:74 `const int CONTACT_NORMAL =0;`
//            GOLDEN LINES: :4863 :4908 (`iContactMode!=CONTACT_NORMAL`)
//            RESOLUTION: NOT gated -- substituted for the TU-local
//              `static const int K5F_CONTACT_NORMAL = 0;` (below).  The value is
//              golden`s EXACT value, re-read this wave from the port`s own
//              translated copy at cContact.h:81 (`const int CONTACT_NORMAL = 0;
//              // golden cContact.cpp:74`), so both call sites keep golden`s
//              control flow bit-for-bit.  This is not an offline approximation.
//            WHY IT SHOULD STAY LOCAL (TRAP 3 -- the naive premise is FALSE and
//              the answer is still "do not reach for it"): a real, translated
//              CONTACT_NORMAL DOES exist at cContact.h:81, so "absent from the
//              port" would be a lie.  It is nevertheless UNREACHABLE FROM THIS
//              TRANSLATION UNIT, and for a simpler reason than the Rear twin`s:
//              the port aTester_Front.cpp does not include MachineDefine.h at all
//              (verified: `rg -n 'MachineDefine.h' aTester_Front.cpp` -> NO
//              MATCH, exit 1), and none of its 15 direct includes carries
//              cContact.h -- a full 107-header transitive closure walk of those
//              includes this wave contains neither cContact.h nor
//              BarCode/BarCode_Shuttle2_CCDScan.h.  (The Rear twin had to argue
//              instead that MachineDefine.h:137`s include of cContact.h sits
//              inside a `#if 0`; that argument is not needed here.)  And adding
//              the include from a part file would walk straight into the hazard
//              cContact.h`s own banner (:66-79) documents: its CONTACT_NORMAL is a
//              plain internal-linkage `const int` that is defined a SECOND time at
//              BarCode/BarCode_Shuttle2_CCDScan.h:187, and the banner states in
//              terms that the first TU to see both headers fails to compile with a
//              redefinition error.  A one-token local const carrying golden`s own
//              value is the lower-risk faithful choice; if the main loop prefers
//              the include, that is a deliberate one-line decision for the file`s
//              include block (not a part file`s), and it should retire this const
//              in the same edit.
//            ABSENCE COMMAND:
//              rg -n -w 'CONTACT_NORMAL' aTester_Front.h aTester_Rear.h atester_shims.h atester_ProcessCount.h aArmHeader.h MachineType.h Motor/mymotor.h csystem.h atester.h cmydef.h cprod.h aHotPlateSubstrate.h acarry_shims.h canary_support.h FormsFacade.h
//                -> NO MATCH (exit 1) in any header the port aTester_Front.cpp
//                   includes directly.
//              rg -n 'MachineDefine.h' aTester_Front.cpp   -> NO MATCH (exit 1)
//            MEASURED AT: 2026-08-10 21:48:38 +0800
//            NOTE: `iContactMode` itself is NOT part of this gap -- it is a real
//              global, cmydef.h:3102, already in scope via cmydef.h.  Only the
//              constant was missing.
//
//  [K5F-G5]  fiosetview->ProcessIndexSuckDestroy1()  (ZERO-ARG overload)
//            GOLDEN LINES: :4377 :4393 (both inside the 12310/12320/12330
//              negative-pressure re-check)
//            RESOLUTION: NOT gated -- rebound to W64B_FIOSET_PISD1_V0(), an
//              ALREADY-EXISTING TU-local seam in the port aTester_Front.cpp
//              (its :1037-1038, `static bool W64B_ProcessIndexSuckDestroy1_V0()
//              -- return true`), created by the Wave-2 translator for this exact
//              golden gap.  This part introduces NO new stub for it and does not
//              touch that seam.
//            WHY THE OFFLINE DEFAULT IS FAITHFUL: true == "the index-suck
//              self-check pump has completed this cycle".  Both call sites assign
//              it into bIndexSuckCheck, and case 12330 advances only on
//              `DoTestYFrontDelay.Off() && bIndexSuckCheck==true`, so false would
//              park the SM forever at 12330 -- a stall golden never has, because
//              on a real machine the DAQ pump does complete.  The vacuum verdict
//              itself is NOT faked: it is read from FTestSuck.Suck[i][j].
//              GetStatus() immediately afterwards (:4408), which is real.
//            REAL-MACHINE DIFFERENCE: with the index-suck IO view present, these
//              two lines actually drive the negative-pressure destroy pump and can
//              report "not finished yet" for a pass or two; offline they report
//              finished on the first pass, so the 12320/12330 pair is one pass
//              faster.  Only INDEX_SUCKER_TYPE==1 machines take this branch at
//              all; on every other machine golden itself hard-sets
//              bIndexSuckCheck=true (:4381, :4397), which is what the offline
//              default reproduces exactly.
//            ABSENCE COMMAND:
//              rg -n 'ProcessIndexSuckDestroy1|ProcessIndexSuckDestroy2' atester_shims.h
//                -> NO MATCH (exit 1): TfiosetviewShim (atester_shims.h:404-409)
//                   exposes ONLY `bool bIndexSuck[2][4][8];`, no pump methods.
//            MEASURED AT: 2026-08-10 21:48:20 +0800
//            WHY IT SHOULD STAY SEAMED (TRAP 3): golden`s home is iosetview.h
//              (Tfiosetview, the whole IO-set-view form), untranslated.  Note the
//              related but DIFFERENT sibling: `fiosetview->bIndexSuck[0][i][j]`
//              at golden :4361 is a REAL member and is left ACTIVE and unmodified
//              -- do not gate it by association.  Index [0] is the front arm
//              (Rear`s twin writes [1]) and is in range of the [2][4][8] grid.
//            STITCH-ORDER REQUIREMENT: because this part uses that macro, it must
//              be stitched AFTER port aTester_Front.cpp line 1038.  Appending at
//              end-of-file satisfies it.  The file contains no `#undef`
//              (verified), so the macro is still live at end-of-file.
//
//  NOT A GATE, BUT THE MAIN LOOP MUST ACT (integration debts)
//  ---------------------------------------------------------
//    * atester_shims.h:110 DECLARES and atester_shims.cpp:186 DEFINES
//      `bool DoFRTCAutoModelVerify(bool){ return true; }`.  That no-op stub must
//      be REMOVED when this part lands, or the link gets a duplicate definition.
//      Note the stub returns TRUE -- i.e. today every caller is told the
//      auto-verify already SUCCEEDED.  This part`s real body returns true ONLY
//      from case 15200, and offline it cannot get past case 160 (see [K5F-G1]),
//      so retiring the stub IS a behaviour change: a caller that loops on this
//      function will loop forever offline instead of being told "verified".  That
//      is the correct, golden behaviour -- flag it, do not "fix" it.
//      TRAP 1 shape (b) applies while that stub exists: the demand is already
//      satisfied, so a green build proves NOTHING about this body being reached.
//    * TRAP 1, the reachability half, stated plainly: NOTHING in the port calls
//      DoFRTCAutoModelVerify today (`rg -n 'DoFRTCAutoModelVerify' --glob '*.cpp' .`
//      -> only the flag bDoFRTCAutoModelVerify and banner text; measured
//      2026-08-10 21:50 +0800).  Golden`s callers are aTester_Front.cpp:7901/7905
//      (inside DoTestYFront, golden :5165-8006 -- a SIBLING agent`s target this
//      same wave) and cContact.cpp:13411/13465/13517 (untranslated).  Linkage is
//      still guaranteed once the stub goes, because this body lands in
//      aTester_Front.cpp, an object file already pulled in for
//      DoFrontTestDestroyIC -- so archive extraction is NOT the risk here;
//      DEAD CODE is.  Do not read "build green" as "wired".
//    * iFRTCAutoModelVerifyTask -- golden :4072 is `int iFRTCAutoModelVerifyTask=1;`
//      and is OUTSIDE this part`s range, so this part does NOT define it.  IT IS
//      ALSO MISSING FROM THE WAVE`S file-scope-globals list (that list names :4071
//      but not :4072) -- flagging it rather than assuming someone else has it.
//      Today it is declared atester_shims.h:84 and defined atester_shims.cpp:161
//      (=1, the same initial value), so `int &Task=iFRTCAutoModelVerifyTask;`
//      binds as-is and nothing more is needed.  If the main loop DOES emit golden
//      :4072 into aTester_Front.cpp, it MUST delete atester_shims.cpp:161 in the
//      same step.
//    * The port aTester_Front.h does NOT declare this function (golden
//      aTester_Front.h:42 does: `extern bool DoFRTCAutoModelVerify(bool
//      bInitial);`).  Today the declaration this body matches comes from
//      atester_shims.h:110.  Moving it to aTester_Front.h is a header edit and is
//      therefore the main loop`s call, not this part`s.
//    * This body is NOT `static`, and its signature matches atester_shims.h:110
//      exactly -- deliberately, to avoid TRAP 1 shape (d): an internal-linkage
//      shadow would link clean here (atester_shims.h IS included by this TU) and
//      then explode.  Every NEW name this part introduces is either `static` data
//      with a unique K5F_/k5f_ prefix (nothing anywhere else declares those
//      names) or an `extern` DECLARATION, so no shadow is possible.
//    * The 12055 drop-error latch has an EXTERNAL release, worth knowing before
//      anyone calls this offline: :4258 sets bIsTestSitICFallDown=true and :4281
//      leaves 12055 only when it is false, so this function cannot clear its own
//      latch.  Golden`s release paths are note.cpp:1369/:3404 and
//      mymessbox.cpp:392 (the alarm-dialog layer) plus csystem.cpp:3644/:3660/
//      :3677/:3693/:18056/:18064.  In the port only the csystem.cpp ones exist
//      (csystem.cpp:1940/:1948/:17700/:17716/:17733/:17749); canary_support.cpp`s
//      ShowErrorMessage sim does not clear it.  So in a ctest harness a simulated
//      drop at 12055 is a permanent park unless the csystem path runs.
//
//  GOLDEN QUIRKS AND ASYMMETRIES KEPT, NOT FIXED
//  ---------------------------------------------
//  (each verified by reading golden Front AND the Rear twin side by side this
//   wave -- a normalised line-level diff of Front :4073-4918 against Rear
//   :4255-5089, so "Front-only" below means measured, not assumed)
//    * :4193 -- the drop-detect in case 12020 sets `bRearHeadICFallDown=true;`
//      IN THE FRONT FUNCTION.  The Rear twin`s corresponding line (:4374) sets
//      `bRecIndexDropAlarm2=true;` instead.  So on the Front side the front-arm
//      drop of the auto-verify path is NOT recorded in bRecIndexDropAlarm1 (the
//      latch golden aTester_Front.cpp:5448 reads), while the Rear side does not
//      touch the chamber-door flag at all.  golden`s own DoTestYFront sets BOTH
//      (:6453 bRecIndexDropAlarm1, :6454 bRearHeadICFallDown), which is why this
//      reads like a copy/paste divergence rather than intent.  REPRODUCED
//      VERBATIM -- bRearHeadICFallDown is a real port global (cmydef.h:2758), so
//      the line is ACTIVE, not gated.
//    * :4258 -- Front sets `bIsTestSitICFallDown=true;` UNCONDITIONALLY, where
//      the Rear twin guards the same assignment with
//      `if(CosFunction.bJAM0303NeedOpenChamberDoor)` (Rear :4434-4435).  Front
//      has no such guard.  Reproduced verbatim.
//    * :4372-4373 -- GOLDEN FALL-THROUGH: `case 12310:` ends with `Task=12320;`
//      and NO `break;`, falling straight into `case 12320:`, so the pass that
//      turns the vacuum back on ALSO runs the 12320 body immediately.  Kept
//      exactly; do not "fix" it.
//    * :4299 -- the `DoTestYFrontDelay.SetSecAndOn(0.3);` in case 12100 sits
//      INSIDE the i/j double loop, so it is re-armed once per site instead of
//      once per case.  Reproduced verbatim.
//    * :4447 -- case 12340 re-tests `if(bHasErr)`, but case 12330 routes to 12340
//      ONLY when bHasErr is already true (:4402 clears it, :4410 sets it,
//      :4424-4431 branches on it), so the test can never be false and the
//      fall-out `break;` at :4459 is unreachable.  There is no else arm to drop.
//    * :4461 -- case 12390 re-issues the SAME `Gali_MotMove(Prod.TestZ1_Safe,
//      iSpeedSlow)` that case 12320 already completed at :4384 -- a redundant
//      second safe move.  Reproduced verbatim.
//    * :4557-4558 and :4581-4582 -- case 12900 zeroes
//      bRealTimeCom_ReceiveOK[rtOPENVERIFYOK] but then SENDS rtOPENVERIFYNG;
//      case 13100 zeroes [rtALLFAILNG] but SENDS rtALLFAILOK.  The cleared channel
//      and the sent channel are crossed in golden BOTH times (identically in the
//      Rear twin).  Reproduced verbatim.
//    * :4642 -- case 13220`s bRTCRetry==true arm has
//      `COM2->DoReleaseAndInspEnd();` COMMENTED OUT (with golden`s own odd `    //`
//      indentation), unlike its structural twin at :4547 which calls it.  The
//      comment line is transcribed verbatim.
//    * :4616 -- case 13300`s `ShowMyMessage("RTC Verify All Fail NG");` is
//      commented out in golden; transcribed verbatim as a comment.
//    * :4907-4910 -- case 15200`s last test has an EMPTY `if(...)` block and does
//      the real work in the `else`.  Reproduced verbatim.
//    * :4546 and :4641 -- the time-out message text is
//      "RTC Release Time out of FR Auto Model Verify" on the Front side
//      (the Rear twin says only "RTC Release Time out").  Transcribed as golden
//      wrote it.
//    * Front-only rows that the Rear twin does not have, all kept: the
//      `TestIF.iTestMode==_32Site4X8N || TestIF.iTestMode==_16Site4X4` ->
//      `IndexSuckName[i+2][j]` selector at :4244-4247; `iNN=IsNNMode()` at :4104
//      feeding `IndexSuckName[i+iNN][j]` at :4442 and :4815 (Rear indexes plain
//      [i][j]); and case 13300 appearing BEFORE case 13210 in source order
//      (:4613 vs :4620) -- switch-case order is irrelevant to behaviour but the
//      text order is golden`s and is preserved.
//    * Front LACKS the extra `DoAllPassVerifyRTC(true);` the Rear twin has at its
//      :5057 (in case 15100).  Not added.
//
//  FIDELITY NOTES
//  --------------
//    * Golden :4073-4918 transcribed CHARACTER FOR CHARACTER (cp950 -> UTF-8
//      only).  Chinese comments and the one Chinese STRING LITERAL at :4502
//      ("RTC Auto Verity ..." two-argument ShowMyMessage) preserved verbatim,
//      never reworded, never translated.  ZERO U+FFFD (asserted in the generator).
//    * INTEGER DIVISION LEFT ALONE: `iSpeedSlow/3` at :4664 is int/int and stays
//      int/int -- a float helper there would drift the Z1 pick height.  Likewise
//      the additive cursor arithmetic is verbatim: `+750` :4174, `+100` :4210 and
//      :4217, `+10` :4664, `+1000` :4346, `iReleaseCT+=10` / `<100` :4327-4328,
//      the raw speed literal `50000` :4225, `ADAM_WriteVoltage(4.0)` :4470.
//    * `IndexSuckName[i+2][j]` at :4245 keeps the literal 2 (it is NOT iNN).
//    * `COM2` is textually UNCHANGED everywhere -- it is rebound by the scoped
//      macro below and released by the matching `#undef` right after the
//      function, so a sibling part`s text is untouched.
//    * Only FOUR tokens in the whole 846-line body differ from golden text, each
//      marked inline with an `AI(k5-DoFRTCAutoModelVerify) 20260810:` note and
//      each registered above: :4377 and :4393 (ProcessIndexSuckDestroy1 ->
//      W64B_FIOSET_PISD1_V0) and :4863 and :4908 (CONTACT_NORMAL ->
//      K5F_CONTACT_NORMAL).
//
//  TRAP 4 COMPLIANCE
//  -----------------
//  This part adds exactly ONE file-scope object, `k5f_com2_ext`.  It is an
//  AGGREGATE with NO user-declared constructor, initialised entirely from
//  integral/boolean constant expressions -> CONSTANT (static) initialisation; no
//  dynamic initialiser runs before main() at all, and it touches no global, no
//  pointer, nothing outside its own storage.  No `new`, no TQPF_Timer, no object
//  whose constructor runs code (the two timers this function needs are declared
//  `extern` here and DEFINED by the main loop).  Both older precedents
//  (atester.cpp:5614 W7T1_com2_ext, atester_32Site.cpp:383 W5_32S_com2_ext) DO
//  have running constructors; this one deliberately does not, matching the Rear
//  twin`s k5_com2_ext.
//
//  TRAP 5 COMPLIANCE -- which header this part relied on
//  ----------------------------------------------------
//  FTestSuck (TMyKitSuck) and FTestSuck.Suck[][] (TMySucker): this part relies on
//  aHotPlateSubstrate.h -- class TMyKitSuck at aHotPlateSubstrate.h:365 (members
//  used: Suck :369, Item :370, iMaxRow :377, iMaxCol :402, iShtRow :406,
//  iShtCol :438, SetItemData :497), class TMySucker at :106 (Enable :140,
//  SenUsing :117, Error, Suck() :119, On() :121, Off() :122, Normal() :130,
//  GetStatus() :131) -- because aHotPlateSubstrate.h is what the port
//  aTester_Front.cpp:115 includes.  The OBJECT this binds to is
//  aHotPlateSubstrate.cpp:92 `TMyKitSuck FTestSuck;`.  THE DECOY IS REAL AND
//  DEFINES THE SAME GLOBAL NAME: mykitsuck.h:274 is a SECOND `class TMyKitSuck`
//  with a DIFFERENT layout, and mykitsuck.cpp:211 has its own
//  `TMyKitSuck FTestSuck;`.  Picking that header would link perfectly and read
//  every field at the wrong offset.  Not used here; mykitsuck.cpp must stay out
//  of this TU`s world.
//  fiosetview->bIndexSuck[0][i][j] (golden :4361) is TfiosetviewShim::
//  bIndexSuck[2][4][8], atester_shims.h:407, object atester_shims.cpp:359 --
//  index [0] is the FRONT arm and is in range.
//  fContact->fShow (golden :4862/:4907) is TfContactShim::fShow,
//  atester_shims.h:157, object declared atester_shims.h:251 -- offline fShow is
//  false, which sends case 14400 down the `else` (drive Z1 to TestZ1_Test -
//  TestZ1_Drop_Offset) and case 15200 down its `else` (set IndexStatus), i.e. the
//  non-contact-mode arms.  That is golden`s behaviour whenever the contact form
//  is closed, not an invention.
//==============================================================================

// ===========================================================================
//  K5F_SEAM -- everything this part needs that the port does not have.  Scoped
//  as tightly as possible because sibling agents are writing other parts of this
//  same eventual translation unit:
//    * unique `K5F_`/`k5f_` prefix on every new name -> no collision with a
//      sibling part and nothing anywhere in the tree to shadow (TRAP 1 (d));
//    * the `COM2` rebinding is #undef`d again immediately after this part`s
//      function, so it CANNOT leak into a sibling`s text (unlike the
//      file-scope-for-the-rest-of-the-TU form used by atester.cpp:5618 and
//      atester_32Site.cpp:386);
//    * the two externs are DECLARATIONS ONLY -- a duplicate identical extern from
//      a sibling part is legal; a duplicate DEFINITION would not be.
//  TODO(integrate): there are now FOUR TU-local copies of this RTC surface
//  (atester.cpp, atester_32Site.cpp, aTester_Rear.cpp x2 shapes, and this one).
//  Converge them into ONE when TCOM2Shim is widened, then delete this block; the
//  call sites bind unchanged.
// ===========================================================================
#ifndef K5F_SEAM
#define K5F_SEAM

// -- golden aTester_Front.cpp:75 `int iHangupCTArm1=0;` -- see GATE [K5F-G2].
//    Definition owed by the main loop (once, for the whole file).
extern int iHangupCTArm1;

// -- golden aTester_Front.cpp:4071 `HTimer DoTestYFrontDelay, DoTestYFrontDelay2;`
//    -- see GATE [K5F-G3].  Definition owed by the main loop; the type MUST be
//    TQPF_Timer, NOT atester_shims.h:463`s `struct HTimer` (whose Off() is
//    hard-coded true, which would zero every dwell in this state machine).
extern TQPF_Timer DoTestYFrontDelay;

// -- golden cContact.cpp:74 `const int CONTACT_NORMAL =0;` (port copy
//    cContact.h:81, unreachable from this TU) -- see GATE [K5F-G4].
static const int K5F_CONTACT_NORMAL = 0;

// -- COM2 real-time-vision surface -- see GATE [K5F-G1].  Aggregate, NO
//    constructor (TRAP 4): constant-initialised, nothing runs before main().
//    The channel ids are golden rs232.h enum members; their NUMERIC values are
//    private to the seam (golden only ever uses them as indices into
//    bRealTimeCom_ReceiveOK[] or as the first argument of SendCommToVision, never
//    compares them to a literal), so any DISTINCT and IN-RANGE set is faithful --
//    which the K5F_RT_N bound guarantees.
enum { K5F_rtSiteMap=0, K5F_rtOpenLive=1, K5F_rtCloseLive=2, K5F_rtRelease=3,
       K5F_rtInspEnd=4, K5F_rtInspStart=5, K5F_rtOPENVERIFYOK=6,
       K5F_rtOPENVERIFYNG=7, K5F_rtALLFAILOK=8, K5F_rtALLFAILNG=9,
       K5F_rtCLOSEVERIFY=10, K5F_RT_N=16 };
struct K5F_TCOM2Ext
{
    bool bRealTimeCom_ReceiveOK[K5F_RT_N];                  // golden rs232.h -- offline all false ("the vision box never answered")
    int  rtSiteMap, rtOpenLive, rtCloseLive, rtRelease, rtInspEnd, rtInspStart,
         rtOPENVERIFYOK, rtOPENVERIFYNG, rtALLFAILOK, rtALLFAILNG, rtCLOSEVERIFY;
    void DoReleaseAndInspEnd() {}                           // golden rs232.h:160 -- offline no-op (identical to the REAL TCOM2Shim member; rebound only so the COM2 rebinding is total)
    void SendCommToVision(int /*iCh*/, bool /*bFlag*/) {}   // golden rs232.h -- offline no-op
    bool OpenRTCComPortAgain() { return false; }            // golden rs232.h -- offline: re-open did not succeed (suppresses the popup, keeps the retry silent)
    void InitRealTimeCCDPara() {}                           // golden rs232.h -- offline no-op
};
static K5F_TCOM2Ext k5f_com2_ext =
{
    { false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false },
    K5F_rtSiteMap, K5F_rtOpenLive, K5F_rtCloseLive, K5F_rtRelease, K5F_rtInspEnd,
    K5F_rtInspStart, K5F_rtOPENVERIFYOK, K5F_rtOPENVERIFYNG, K5F_rtALLFAILOK,
    K5F_rtALLFAILNG, K5F_rtCLOSEVERIFY
};

#endif // K5F_SEAM

// Rebind COM2 for THIS PART ONLY (matching #undef right after the function).
#ifdef COM2
#undef COM2
#endif
#define COM2 (&k5f_com2_ext)
//------------------------------------------------------------------------------
bool DoFRTCAutoModelVerify(bool bInitial)
{
    static int iReleaseCT=0;
    static bool bVerifyNG=false;
    static bool bHasErr=false, bIndexSuckCheck=false;
    static bool bFTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false} };

    static bool bFTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},                                        //Steven 20110301 : 確認吸取完成
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bRTCRetry=false;                                                //Ifor 20251023 add:RTC無回應Retry 一次

    if(bInitial==true)
    {
        iFRTCAutoModelVerifyTask=1;
        for(int i=0; i<FTestSuck.iMaxRow; i++)
        {
            for(int j=0; j<FTestSuck.iMaxCol; j++)
            {
                bFTestSuckUse[i][j]=false;
                bFTSuckFinish[i][j]=false;
            }
        }
        return false;
    }

    int ret=0, iNN=IsNNMode();
    int &Task=iFRTCAutoModelVerifyTask;
    bool bCheckAllSuck=false, flag=false;
    AnsiString ErrPart="";

    switch(Task)
    {
        case 1:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoFRTCAutoModelVerify 1"))
            {
                bVerifyNG=false;
                bPickUpErrReAutoVerify=false;                                   //JerryYang 20220215 : RTC Auto Verify half view check
                Task=100;
            }
            break;
        case 100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoFRTCAutoModelVerify 100"))
            {
                Task=150;
            }
            break;
        case 150:
            if(SendSiteMapToRTC(true,1)!=-1)
            {
                DoTestYFrontDelay.SetSecAndOn(10);
                Task=160;
            }
            break;
        case 160:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtSiteMap])
            {
                DoAllPassVerifyRTC(true);
                Task=200;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Site Map Time out");
                COM2->DoReleaseAndInspEnd();
                Task=1;
            }
            break;
        case 200:
            if(DoAllPassVerifyRTC())
            {
                if(CosFunction.bRTCHalfViewAutoVerify)                          //JerryYang 20220215 : RTC Auto Verify half view check
                {
                    DoHalfViewAllPassVerifyRTC(true);
                    Task=12005;
                }
                else
                {
                    Task=12010;
                }
            }
            break;
        case 12005:
            if(DoHalfViewAllPassVerifyRTC())
            {
                Task=12010;
            }
            break;
        case 12010:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoFRTCAutoModelVerify 12010"))
            {
                iReleaseCT=0;
                Task=12020;
            }
            break;
        case 12020:                                                                                                                                             //Z1 Down
            if(MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+750))                                                                      //kevin 20140612 add start
            {
                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            //Steven 20110725 : 不再使用IsSuckICFallDown
                            if(FTestSuck.Suck[i][j].Enable       &&
                               FTestSuck.Suck[i][j].SenUsing!="" &&
                               FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                               FTestSuck.Item[i][j]!=NULL_IC)
                            {
                                if(FTestSuck.Suck[i][j].W906_GetStatusHeld()==false)  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都要有真空，掉了要持續 100 ms 才算掉料
                                {
                                    FTestSuck.Suck[i][j].Normal();                                                                                              //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                    flag=true;
                                    bRearHeadICFallDown=true;                                                                                                   //kevin 20131120 發ALARM 開CHAMBO門 按Z1
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                    Task=12050;
                    return false;
                }
            }                                                                                                                                                   //kevin 20140612 add end

            if(Prod.TestZ1_Drop_Offset!=0)
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+100+IniConfig.iD36_RTCAutoVerifyReleaseHeight+iReleaseCT, iSpeedSlow))    //jou 20180226 : 200 -> 150 -> 130
                {
                    Task=12100;
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+100+IniConfig.iD36_RTCAutoVerifyReleaseHeight+iReleaseCT, iSpeedSlow))                            //jou 20180226 : 200 -> 150 -> 130
                {
                    Task=12100;                                                                                                                                 //Steven 20110511
                }
            }
            break;
        case 12050:                                                             //掉料處理------   //kevin 20140612  上升方便取料

            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, 50000))
            {
                Task=12055;
            }
            break;
        case 12055:                                                                                                     //kevin 20140612 add
            bHasErr|=CheckIndexAllSuckICFallDown(true, false);                                                          //Steven 20131128 : 換位置

            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FTestSuck.Suck[i][j].Enable       &&
                       FTestSuck.Suck[i][j].SenUsing!="" &&
                       FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                       FTestSuck.Item[i][j]!=NULL_IC)
                    {
                        if(FTestSuck.Suck[i][j].W906_GetStatusHeld()==false)  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都要有真空，掉了要持續 100 ms 才算掉料
                        {
                            if(TestIF.iTestMode==_32Site4X8N || TestIF.iTestMode==_16Site4X4)                           //kevin 20191113 add //Sam 20190226 : 16Site4X4    //kevin 20180602 (wei) 32 Site ErrPart
                                ErrPart+=IndexSuckName[i+2][j];
                            else
                                ErrPart+=IndexSuckName[i][j];
                            bHasErr=true;
                        }
                    }
                }
            }

            if(LastSet.iRealDummy==REALLY && bHasErr)
            {
                if(IndexAlarmInArmAway()==true)
                {
                    bIsTestSitICFallDown=true;
                    ShowErrorMessage("JAM0303", K_SKIP, MTestZ1, false, ErrPart);                                       //Steven 20100129 : Device Drop Error

                    bPickUpErrReAutoVerify=true;                                                                        //JerryYang 20241220 : auto verify發生index drop error要重做

                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Suck[i][j].Error ||
                               (FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                FTestSuck.Item[i][j]!=NULL_IC &&
                                FTestSuck.Suck[i][j].W906_GetStatusHeld()==false))                                               //有用到且有吸到IC的卻掉了  AI(W906-F03-QUADVAC) 20261004: 群組：4 個都要有真空，掉了要持續 100 ms 才算掉料
                            {
                                FTestSuck.SetItemData(i, j, HAS_NULL_IC);                                               //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                                FTestSuck.Suck[i][j].Normal();                                                          //Steven 20110829 : 把真空關掉
                            }
                        }
                    }
                    break;
                }
            }

            if(bIsTestSitICFallDown==false)
            {
                Task=12020;
            }
            break;
        case 12100:                                                             //Place IC to socket
            bFTestSuckDrop=true;

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                       FTestSuck.Item[i][j]==HAS_IC)
                    {
                        FTestSuck.Suck[i][j].Off();
                    }

                    DoTestYFrontDelay.SetSecAndOn(0.3);
                }
            }
            Task=12200;
            break;
        case 12200:                                                             //吹氣Off
            if(DoTestYFrontDelay.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                           FTestSuck.Item[i][j]==HAS_IC)
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                }

                bIndexCheckNoStopVaccum=false;
                DoTestYFrontDelay.SetSecAndOn(0.3);
                Task=12210;
            }
            break;
        case 12210:
            if(DoTestYFrontDelay.Off())
            {
                iReleaseCT+=10;
                if(iReleaseCT<100)
                {
                    Task=12020;
                }
                else
                {
                    DoTestYFrontDelay.SetSecAndOn(3.0);
                    Task=12220;
                }
            }
            break;
        case 12220:
            if(DoTestYFrontDelay.Off())
            {
                Task=12300;
            }
            break;
        case 12300:                                                             //Z1 Up
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+1000, iSpeedSlow))
            {
                Task=12310;
            }
            break;
        case 12310:                                                             //Steven 20180417 : RTC Auto Verify新增回吸偵測
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                       FTestSuck.Item[i][j]==HAS_IC)
                    {
                        if(INDEX_SUCKER_TYPE==1)                                //Steven 20111202
                        {
                            fiosetview->bIndexSuck[0][i][j]=true;
                        }
                        else
                        {
                            FTestSuck.Suck[i][j].On();                          //kevin 20110504 check 掉料
                        }
                    }
                }
            }
            DoTestYFrontDelay.SetSecAndOn(1);
            bIndexSuckCheck=false;
            Task=12320;
        case 12320:
            if(INDEX_SUCKER_TYPE==1)
            {
                bIndexSuckCheck=false;
                bIndexSuckCheck=W64B_FIOSET_PISD1_V0();                        //AI(k5-DoFRTCAutoModelVerify) 20260810: golden `fiosetview->ProcessIndexSuckDestroy1()` -> the EXISTING TU-local seam aTester_Front.cpp:1037-1038; GATE [K5F-G5]
            }
            else
            {
                bIndexSuckCheck=true;
            }

            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, iSpeedSlow))
            {
                Task=12330;
            }
            break;
        case 12330:
            if(INDEX_SUCKER_TYPE==1)
            {
                bIndexSuckCheck=false;
                bIndexSuckCheck=W64B_FIOSET_PISD1_V0();                        //AI(k5-DoFRTCAutoModelVerify) 20260810: golden `fiosetview->ProcessIndexSuckDestroy1()` -> the EXISTING TU-local seam aTester_Front.cpp:1037-1038; GATE [K5F-G5]
            }
            else
            {
                bIndexSuckCheck=true;
            }

            if(DoTestYFrontDelay.Off() && bIndexSuckCheck==true)
            {
                bHasErr=false;

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（黏料／殘料／不該有料）
                        {
                            bHasErr=true;
                        }
                        else
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }

                        if(INDEX_SUCKER_TYPE==1)                                //jou 2011-11-01負壓不能一直開著真空，必須關掉
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                }

                if(bHasErr)
                {
                    Task=12340;
                }
                else
                {
                    Task=12390;
                }
            }
            break;
        case 12340:
            ErrPart=" ";
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Suck[i][j].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（黏料／殘料／不該有料）
                    {
                        ErrPart+=IndexSuckName[i+iNN][j];
                    }
                }
            }

            if(bHasErr)
            {                                                                                                           //jou 2012-03-05 開啟D44偵測到卡料時，In arm也要讓位。
                if(IndexAlarmInArmAway()==true)                                                                         //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    ShowErrorMessage("JAM0327", K_RETRY, MTestZ1, false, ErrPart);                                      //Vacuum Sensor Off Error
                    Task=12310;
                }
                else
                {
                    return false;
                }
            }
            break;
        case 12390:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, iSpeedSlow))
            {
                Task=12400;
            }
            break;                                                              //Steven 20180417 : RTC Auto Verify新增回吸偵測
        case 12400:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoFRTCAutoModelVerify 12400"))
            {
                bRTCAutoVerifyControlEP=true;
                ADAM_WriteVoltage(4.0);

                if(IniConfig.bD36_1EnableRTCAutoModelVerifyLive==true)
                {
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtOpenLive]=false;
                    COM2->SendCommToVision(COM2->rtOpenLive, true);
                    DoTestYFrontDelay.SetSecAndOn(10);

                    Task=12410;
                }
                else
                {
                    Task=12500;
                }
            }
            break;
        case 12410:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtOpenLive])
            {
                Task=12415;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Open Live Time out");
                COM2->DoReleaseAndInspEnd();
                Task=12400;
            }
            break;
        case 12415:
            if(IndexAlarmInArmAway())
            {
                ShowMyMessage("RTC Auto Verity 產品可能因為沾黏導致放置歪斜,請確認!","device may be skewed due to sticking. Please confirm!");
                Task=12420;
            }
            break;
        case 12420:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtCloseLive]=false;
            COM2->SendCommToVision(COM2->rtCloseLive, true);
            DoTestYFrontDelay.SetSecAndOn(10);
            Task=12430;
            break;
        case 12430:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtCloseLive])
            {
                Task=12500;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Close Live Time out");
                COM2->DoReleaseAndInspEnd();
                Task=12420;
            }
            break;
        case 12500:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease]=false;
            COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd]=false;
            COM2->DoReleaseAndInspEnd();                                        //JerryYang 20220215 : Release跟InspEnd一起送
            DoTestYFrontDelay.SetSecAndOn(10);
            Task=12600;
            break;
        case 12600:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease] &&
               COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd] &&
               COM2->bRealTimeCom_ReceiveOK[COM2->rtInspStart])                 //JerryYang 20220215 : Release跟InspEnd一起送
            {
                Task=12900;
                bRTCRetry=false;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(bRTCRetry==true)
                {
                    bRTCRetry=false;
                    if(COM2->OpenRTCComPortAgain())
                        ShowMyMessage("RTC Release Time out of FR Auto Model Verify");
                    COM2->DoReleaseAndInspEnd();
                }
                else
                {
                    bRTCRetry=true;
                }
                Task=12500;
            }
            break;
        case 12900:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYOK]=false;
            COM2->SendCommToVision(COM2->rtOPENVERIFYNG, true);
            DoTestYFrontDelay.SetSecAndOn(10);
            Task=13000;
            break;
        case 13000:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYOK])
            {
                Task=13100;
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYNG])
            {
                ShowMyMessage("RTC Open Verify NG");
                Task=12500;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Open Verify Time out");
                COM2->DoReleaseAndInspEnd();
                Task=12500;
            }
            break;
        case 13100:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILNG]=false;
            COM2->SendCommToVision(COM2->rtALLFAILOK, true);
            DoTestYFrontDelay.SetSecAndOn(10);
            Task=13200;
            break;
        case 13200:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILOK])
            {
                bVerifyNG=false;
                if(CosFunction.bRTCHalfViewAutoVerify)                          //JerryYang 20220215 : RTC Auto Verify half view check
                {
                    DoHalfViewAllFailVerifyRTC(true);
                    Task=13210;
                }
                else
                {
                    Task=13400;
                }
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILNG])
            {
                bVerifyNG=true;
                Task=13300;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Verify All Fail Time out");
                COM2->DoReleaseAndInspEnd();
                Task=12500;
            }
            break;
        case 13300:
            if(IndexAlarmInArmAway()==true)
            {
//                ShowMyMessage("RTC Verify All Fail NG");
                Task=13400;
            }
            break;
        case 13210:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease]=false;                //JerryYang 20220215 : Release跟InspEnd一起送
            COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd]=false;
            COM2->DoReleaseAndInspEnd();
            DoTestYFrontDelay.SetSecAndOn(10);
            Task=13220;
            break;
        case 13220:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease] &&
               COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd] &&
               COM2->bRealTimeCom_ReceiveOK[COM2->rtInspStart])
            {
                Task=13350;
                bRTCRetry=false;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(bRTCRetry==true)
                {
                    bRTCRetry=false;
                    if(COM2->OpenRTCComPortAgain())
                        ShowMyMessage("RTC Release Time out of FR Auto Model Verify");
    //                COM2->DoReleaseAndInspEnd();
                }
                else
                {
                    bRTCRetry=true;
                }
                Task=13210;
            }
            break;
        case 13350:
            if(DoHalfViewAllFailVerifyRTC())
            {
                Task=13400;
            }
            break;
        case 13400:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoFRTCAutoModelVerify 13400"))
            {
                Task=13500;
            }
            break;
        case 13500:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+10+IniConfig.iD36_RTCAutoVerifyPickHeight, iSpeedSlow/3))                     //jou 20180226 : 0 -> +110 -> +60 -> 0 -> +10
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]!=NULL_IC &&
                           FTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            bFTestSuckUse[i][j]=true;
                        }
                        else
                        {
                            bFTestSuckUse[i][j]=false;
                        }

                        FTestSuck.Suck[i][j].Error=false;
                        bFTSuckFinish[i][j]=false;
                    }
                }
                iHangupCTArm1=0;
                Task=13600;
            }
            break;
        case 13600:
            COM2->SendCommToVision(COM2->rtCLOSEVERIFY, true);
            DoTestYFrontDelay.SetSecAndOn(10);
            Task=13700;
            break;
        case 13700:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtCLOSEVERIFY])
            {
                Task=13800;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Close Verify Time out");
                COM2->DoReleaseAndInspEnd();
                Task=13600;
            }
            break;
        case 13800:
            bCheckAllSuck=true;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTSuckFinish[i][j]==false)
                    {
                        if(bFTestSuckUse[i][j])
                        {
                            if(FTestSuck.Suck[i][j].Suck())
                            {
                                bFTSuckFinish[i][j]=true;
                                bFTestSuckUse[i][j]=false;
                            }
                            else if(FTestSuck.Suck[i][j].Error)
                            {
                                bFTSuckFinish[i][j]=true;
                            }
                            else
                            {
                                bCheckAllSuck=false;
                            }
                        }
                        else
                        {
                            bFTSuckFinish[i][j]=true;
                        }
                    }
                }
            }

            flag=true;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTSuckFinish[i][j]==false)
                        flag=false;
                }
            }

            if(flag)
            {
                bHasErr=false;
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            bHasErr=true;
                            if(IniConfig.bD62PickUpErrorNeedPurge)              //Steveb 20161024 : 吸取異常需要吹氣一次
                            {
                                FTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }

                if(bCheckAllSuck)
                {
                    Task=13900;
                }
            }
            break;
        case 13900:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoFRTCAutoModelVerify 13900"))
            {
                bRTCAutoVerifyControlEP=false;
                ADAM_WriteVoltage(DeviceForm.dPress);                           //JerryYang 20220215 : 修正EP氣量錯誤
                DoTestYFrontDelay.SetSecAndOn(3);
                Task=13950;
            }
            break;
        case 13950:
            if(DoTestYFrontDelay.Off())
            {
                if(bVerifyNG==true || bHasErr==true)
                    Task=14000;
                else
                    Task=14400;
            }
            break;
        case 14000:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoFRTCAutoModelVerify 14000"))
            {
                if(bVerifyNG==true && bHasErr==false)
                {
                    ShowMyMessage("RTC Auto Verify fail!!");
                    Task=1;
                }
                else
                {
                    if(bVerifyNG==true && bHasErr==true)
                        bRTCAutoModelVerifyFirstTime=true;

                    Task=14100;
                }
            }
            break;
        case 14100:
            ErrPart=" ";
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Suck[i][j].Error)
                    {
                        ErrPart+=IndexSuckName[i+iNN][j];
                        if(IniConfig.bD62PickUpErrorNeedPurge)                  //Steveb 20161024 : 吸取異常需要吹氣一次
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].Error=false;
                    }
                }
            }
            bIsTestSitICFallDown=true;
            bPickUpErrReAutoVerify=true;                                        //JerryYang 20220215 : RTC Auto Verify half view check
            ret=ShowErrorMessage("JAM0301", K_SKIP, MTestZ1, false, ErrPart);   //Devicr Pick-Up Error

            if(ret==K_SKIP)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            FTestSuck.SetItemData(i, j, HAS_NULL_IC);
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
            }

            DoAllPassVerifyRTC(true);
            Task=14200;
            break;
        case 14200:
            if(DoAllPassVerifyRTC())
            {
                Task=14300;
            }
            break;
        case 14300:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoFRTCAutoModelVerify 14300"))
            {
                Task=14400;
            }
            break;
        case 14400:
            if(W906_FormShowing("fContact", fContact->fShow) &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
               iContactMode!=K5F_CONTACT_NORMAL)                                //JerryYang 20220215 : RTC Auto Verify half view check   //AI(k5-DoFRTCAutoModelVerify) 20260810: CONTACT_NORMAL -> K5F_CONTACT_NORMAL; GATE [K5F-G4]
            {
                Task=15000;
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, iSpeedSlow))
                {
                    Task=15000;
                }
            }
            break;
        case 15000:
            if(SendSiteMapToRTC(true,0)!=-1)
            {
                DoTestYFrontDelay.SetSecAndOn(10);
                Task=15100;
            }
            break;
        case 15100:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtSiteMap])
            {
                Task=15200;
            }
            else if(DoTestYFrontDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Site Map Time out");
                COM2->DoReleaseAndInspEnd();
                Task=15000;
            }
            break;
        case 15200:
            if(CheckTwoArmSiteMap()==true)
            {
                if(bDoBRTCAutoModelVerify==false)
                    bRTCAutoModelVerifyFirstTime=true;
            }

            RecordProcess("RTC auto verify end");
            bDoFRTCAutoModelVerify=true;
            COM2->InitRealTimeCCDPara();
            bFTestSuckDrop=false;

            if(W906_FormShowing("fContact", fContact->fShow) &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
               iContactMode!=K5F_CONTACT_NORMAL)                                //JerryYang 20220215 : RTC Auto Verify half view check   //AI(k5-DoFRTCAutoModelVerify) 20260810: CONTACT_NORMAL -> K5F_CONTACT_NORMAL; GATE [K5F-G4]
            {
            }
            else
            {
                IndexStatus=Z1Down_Z2Up;
            }
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
// End of part k5-DoFRTCAutoModelVerify (golden aTester_Front.cpp :4073-4918).
// Release the scoped COM2 rebinding so a sibling part`s text is untouched and the
// real `extern TCOM2Shim *COM2;` (atester_shims.h:461) is visible again.
#undef COM2
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ROLE: the Arm2 (rear Z2/Y2) "pick-up error -> piggyback" socket-residual
//   recovery state machine.  It lives in the FRONT translation unit because
//   golden CROSS-WIRES the pair -- golden aTester_Front.cpp owns the Arm2
//   checker, golden aTester_Rear.cpp owns the Arm1 one.  After an Index Arm2
//   pick-up error it re-sucks every NULL_IC/HAS_NULL_IC site of BTestSuck,
//   lifts Z2 to the socket-check height, re-reads the vacuum to decide whether
//   an IC is still sitting in a Test Socket, and if so raises the
//   socket-residual alarm WAR0310 after moving the arms clear.  Returns true
//   when the sequence has finished.
//
// WAVE SCOPE (label k7-Arm1-decide, golden file aTester_Front.cpp):
//   * DoIndexArm2PickUpErrNeedPiggyback(bool)  golden :4922-5078  ACTIVE  (this part)
//   * CheckAnyCaseNeedToDoArm1()               golden :5080-5152  ACTIVE  (sibling part file 05080_)
//   * InitTestYFrontTask()                     golden :5155-5158  ACTIVE  (sibling part file 05155_)
//
// SYMMETRIC TWIN: port aTester_Rear.cpp:7005-7169 is the mirror
//   DoIndexArm1PickUpErrNeedPiggyback (golden aTester_Rear.cpp :5093-5254),
//   translated by PT-W7b label k8-Arm2-decide.  Its GATE REGISTER, its
//   CheckVaccumIsIniaialON treatment and its banner conventions are followed
//   here; every place the two golden bodies DIFFER is called out under
//   "GOLDEN ASYMMETRIES" below, and the one place the Rear GATE PREMISE DOES
//   NOT CARRY OVER is called out inside GATE k7-G1.
//
// NOT DEFINED HERE, ON PURPOSE -- two plain data globals that must stay shared:
//   * iIndexArm2PickUpErrNeedPiggybackTask (golden :4920) -- ALREADY defined at
//     port atester_shims.cpp:163 and declared atester_shims.h:86, which this TU
//     already includes (aTester_Front.cpp:103).  It must STAY a shared global:
//     golden main.cpp registers task-cursor ADDRESSES in QueueTaskList, so a
//     TU-local copy would silently detach the task-list monitor.  Redefining it
//     here would also be a duplicate definition at link time.
//   * TQPF_Timer hCheckSockerDelay2 (golden :4921) -- one of the 16
//     between-function file-scope globals the MAIN LOOP adds in ONE place this
//     wave (golden :75, :78, :94-:98, :2232, :2235, :3943, :4071, :4921, :5160,
//     :5161, :5162, :8010).  This part USES it and assumes it exists.  See
//     GATE REGISTER entry NG-2 for the absence measurement and the hand-off.
//
// GATE REGISTER
// -----------------------------------------------------------------------------
// GATE k7-G1 -- golden :4965  `fiosetview->ProcessIndexSuckDestroy2();`
//   ABSENT: the offline TfiosetviewShim (port atester_shims.h:404-409) exposes
//     ONLY `bool bIndexSuck[2][4][8];` plus its ctor -- there is no
//     ProcessIndexSuckDestroy2 member (nor ProcessIndexSuckDestroy1).
//   COMMAND PROVING ABSENCE (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//     rg -n "ProcessIndexSuckDestroy2" -g "*.h" -g "*.cpp" .
//   RESULT: no declaration in ANY header; every body in the tree is a TU-LOCAL
//     stand-in reachable only from its own file --
//     atester.cpp:5518 W7T1_ProcessIndexSuckDestroy2,
//     atester_32Site.cpp:206 W5_32S_ProcessIndexSuckDestroy2,
//     aTester_Rear.cpp:409 W64bT2_ProcessIndexSuckDestroy2,
//     AutoClean/AutoClean.cpp:278 W906DIAC_ProcessIndexSuckDestroy2.
//     None of them is visible from aTester_Front.cpp.
//   RUN AT: 2026-08-10T21:45:10+0800.
//   WHY THE OFFLINE DEFAULT IS FAITHFUL: golden DISCARDS the return value at this
//     call site, so no control flow inside this function depends on it; the
//     stand-in returns true, matching the four precedents above.
//   REAL-MACHINE BEHAVIOUR DIFFERENCE -- AND WHY THE REAR PREMISE DOES **NOT**
//     CARRY OVER: port aTester_Rear.cpp:6906-6917 (GATE k8-G1) concluded that
//     gating the pump makes its WHOLE piggyback check inert when
//     INDEX_SUCKER_TYPE==1.  That conclusion is correct for REAR and WRONG for
//     FRONT, because the two golden case bodies differ: golden Rear :5108-5121
//     drives the nozzle EITHER through bIndexSuck[0][][] (type 1) OR through
//     FTestSuck.Suck.On() (else), so gating the pump leaves type 1 with no
//     vacuum at all.  Golden FRONT :4949-4956 calls BTestSuck.Suck[i][j].On()
//     UNCONDITIONALLY and only ADDS the bIndexSuck[1][][] flag for type 1 -- so
//     the vacuum IS still commanded here and the residual detection at case 2060
//     is NOT wholesale inert.  What IS lost on a real machine is the type-1
//     PUMPED suck/destroy sequencing (the iosetview pump re-driven every scan for
//     the whole 1 s window) together with the bIndexCheckNoStopVaccum
//     "keep pumping even while paused" behaviour -- i.e. vacuum settle timing and
//     therefore detection SENSITIVITY change, not the presence of the check.
//     Retire the gate by giving TfiosetviewShim the method; do not ship a real
//     machine on this gate.
//
// NG-1 -- not an absence in this part: see the sibling part file 05080_ for
//   iHangupCTArm1 (golden :75 / :5128 / :5149).
//
// NG-2 -- NOT GATED, DELIBERATELY: TQPF_Timer hCheckSockerDelay2 (golden :4921),
//   used at golden :4959, :4968, :4979, :4981, :4986.
//   ABSENCE COMMAND (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//     rg -n "hCheckSockerDelay2" -g "*.h" -g "*.cpp" .
//   RESULT: zero hits -- only the Rear twin hCheckSockerDelay1 exists
//     (port aTester_Rear.cpp:6969).  RUN AT: 2026-08-10T21:48:55+0800.
//   TRAP 3 RE-ASK, why should this NOT be gated: golden's dwell is load-bearing
//     (1 s for the suck to build, 0.5 s / 5 s for Greatek after the Z2 lift), and
//     the port's alternative `struct HTimer` (atester_shims.h) hard-codes
//     Off()==true, i.e. every dwell would expire instantly and the vacuum would
//     be read before it exists -- exactly the wrong-type-links-fine failure that
//     port aTester_Rear.cpp:321-335 documents.  So the right answer is a real
//     TQPF_Timer, owned by the main loop's single globals block.  TRAP-2 HAND-OFF:
//     if that block does not land, this part fails with a LOUD undefined
//     reference to hCheckSockerDelay2 -- never a silent behaviour change.
//   TRAP 4 CHECKED (for the main loop, since it owns the definition): TQPF_Timer's
//     ctor (myTimer.cpp:14-17) only calls its own
//     CalibratePerformanceCounterOverhead(), which touches
//     QueryPerformanceCounter() and this object's own members -- it reads NO
//     global pointer, so the static-init ctor cannot repeat the
//     fLaserSensor/elLaser SEGFAULT shape.  Same shape as the already-shipping
//     TQPF_Timer BTempRiseHasICDelay (port aTester_Rear.cpp:309).  This part adds
//     NO file-scope object of its own.
//
// NG-3 -- NOT GATED, DELIBERATELY: TMyKitSuck::CheckVaccumIsIniaialON (golden
//   :4998).  Same call this file's Rear twin makes at port aTester_Rear.cpp:7088,
//   and the same treatment is used: a FAITHFUL free-function transcription of
//   golden's own body, NOT a degraded stand-in.
//   ABSENCE COMMAND (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//     rg -n "CheckVaccumIsIniaialON" -g "*.h" .
//   RESULT: exactly ONE hit, mykitsuck.h:361 -- i.e. the method exists ONLY on the
//     OTHER TMyKitSuck (see TRAP 5 below), and is absent from the
//     aHotPlateSubstrate.h mirror this TU actually uses.
//   RUN AT: 2026-08-10T21:43:02+0800.
//   WHY THE TRANSCRIPTION IS FAITHFUL, not an approximation: golden's body
//     (golden mykitsuck.cpp:2732-2746, read in full) touches ONLY members the
//     aHotPlateSubstrate.h mirror already has -- Suck[][].GetStatus()
//     (aHotPlateSubstrate.h:131), Suck[][].Normal() (:130), Item[][] (:370) and
//     the NULL_IC/HAS_NULL_IC constants (cmydef.h) -- so it is copied statement
//     for statement.  Contrast atester.cpp:5528, which works around the SAME gap
//     with a DEGRADED macro that throws the detection away; that is not needed.
//   HONEST CONSEQUENCE: TMySucker::GetStatus is itself an offline stub today
//     (aHotPlateSubstrate.cpp, returns false), so OFFLINE this reaches the else
//     branch, leaves flag false, and case 2060 returns true -- the same net
//     offline value as the degraded macro.  Equivalent offline values are not the
//     point: when GetStatus becomes a real sensor read THIS code is already
//     correct and needs no edit.
//
// TRAP 5 -- WHICH TMyKitSuck / WHICH OBJECT.  Two different classes with this
//   name exist: mykitsuck.h:274 and aHotPlateSubstrate.h:365, with DIFFERENT
//   LAYOUTS, and BOTH .cpp files define an object called BTestSuck
//   (mykitsuck.cpp:212 and aHotPlateSubstrate.cpp:93).  This part relies on
//   aHotPlateSubstrate.h:365 (class), aHotPlateSubstrate.h:637 (`extern
//   TMyKitSuck BTestSuck;`) and aHotPlateSubstrate.cpp:93 (the object that
//   actually links) -- the 177-TU one, already included by aTester_Front.cpp:115.
//   mykitsuck.cpp is DELIBERATELY NOT REGISTERED in CMakeLists.txt (see its own
//   note at CMakeLists.txt:2099), so mykitsuck.h is NOT included here even though
//   it is the only header declaring CheckVaccumIsIniaialON -- including it to get
//   that one declaration is precisely the trap: it would link perfectly and read
//   every field at the wrong offset.
//
// TRAP 1 -- the three GOLDEN functions in this label are deliberately NON-static
//   (golden is non-static, and atester_shims.h:91-92 already declares
//   CheckAnyCaseNeedToDoArm1/InitTestYFrontTask non-static, so a `static`
//   definition would be shape (d)).  The two offline seams below ARE file-scope
//   static, which is safe because the W7cK7_ prefix is novel -- no non-static
//   declaration of either name exists anywhere in the tree, so there is nothing
//   for them to shadow.  INTEGRATOR, shape (b) WARNING: the no-op/zero stubs
//   `int CheckAnyCaseNeedToDoArm1() { return 0; }` (atester_shims.cpp:168) and
//   `void InitTestYFrontTask() {}` (atester_shims.cpp:169) ALREADY satisfy every
//   existing call, so once the real bodies land the build and the link both stay
//   green while the STUBS are what actually run, and `nm --undefined-only` cannot
//   see it.  Both stubs MUST be removed in the same integrate step -- and note the
//   CheckAnyCaseNeedToDoArm1 stub is not merely inert: returning 0 instead of the
//   real Task number sends DoTestYFront to Task 0.  DoIndexArm2PickUpErrNeedPiggyback
//   itself has NO stub (checked: it appears in no header and in no shim), so it is
//   genuinely new code and cannot be shadowed.
//
// GOLDEN ASYMMETRIES vs the Rear twin -- ALL PRESERVED, NONE "harmonised":
//   * case 2030 calls BTestSuck.Suck[i][j].On() UNCONDITIONALLY and then, only for
//     INDEX_SUCKER_TYPE==1, ALSO sets fiosetview->bIndexSuck[1][i][j] and
//     bIndexCheckNoStopVaccum.  Rear (golden :5108-5121) is an if/ELSE instead.
//   * case 2030 dwell is SetSecAndOn(1); Rear's is 0.5.
//   * case 2030 ends with a REAL `break;` (golden :4961), so Front does NOT fall
//     through into 2040 on the same scan.  Rear's break at that point is
//     COMMENTED OUT and therefore does fall through.
//   * case 2050 assigns Task=2060 AFTER the Greatek delay if/else; Rear assigns
//     Task=1060 BEFORE it.  Order kept as golden.
//   * case 2060 indexes IndexSuckName[i][j] with NO IsNNMode() row offset, while
//     Rear uses IndexSuckName[i+iNN][j].  Consequently this function declares NO
//     iNN at all.  Kept exactly as golden -- do NOT "fix" the missing offset.
//   * case 2090 reports the alarm against MTestY2; Rear reports MTestY1.
//   * case 2100 moves to (TestY1_Front, TestY2_Middle) and leaves
//     IndexStatus=Z1Up_Z2Down; Rear moves to (TestY1_Middle, TestY2_Rear) and
//     leaves Z1Down_Z2Up.  Mirror images, both kept.
//
// GOLDEN QUIRKS PRESERVED, NOT FIXED:
//   * case 1 has NO break: it sets Task=2030 and falls THROUGH into case 2030.
//   * case 2200 has no trailing break (it is the last label in the switch).
//   * `int iIndexCheckOffSet=IniConfig.fIndexCheckOffset*100;` -- fIndexCheckOffset
//     is a double (Config.h:83) and golden TRUNCATES the product to int.  Kept as an
//     int initialised from the double product, exactly as golden.  Do NOT "clean
//     this up" into a rounding helper, and do NOT turn any int/int here into
//     floating point.
//   * `ErrPart=" "` (a single SPACE, not "") at golden :4989, while golden :5060
//     resets it to "".
//   * `bIndexCheckNoStopVaccum=true` is re-assigned inside the per-site loop.
//   * ErrPart is a function-static AnsiString, so it survives across scans.
//   * case 2070 lifts via MOT[MTestZ1].Gali_Two_ZAxis_Move even though this is the
//     Arm2 routine -- NOT a bug: Gali_Two_ZAxis_Move is the both-Z-axes move
//     issued through the MTestZ1 handle, and the Rear twin does exactly the same
//     at port aTester_Rear.cpp:7113.
//
// DEPENDENCIES VERIFIED PRESENT (so they are ACTIVE, not gated):
//   GetSocketCheckPos(int) atester.h:91, real body atester.cpp:9541 (checked NOT
//   inside any #if 0); IndexAlarmInArmAway() atester.h:67 / atester.cpp:284;
//   IndexSuckName[4][8] cmydef.h:3239; IndexStatus cmydef.h:2590; Z1_Z2_Normal /
//   Z1Up_Z2Down cmydef.h:50-51; bIsTestSitICFallDown cmydef.h:2723;
//   bIndexCheckNoStopVaccum cmydef.h:3345; INDEX_SUCKER_TYPE cmydef.h:2943;
//   CC_SCS MachineType.h:317, CC_Greatek MachineType.h:330;
//   IniConfig.fIndexCheckOffset Config.h:83,
//   IniConfig.bD40IndexICFallDownMustPressFMotorDown Config.h:524;
//   Prod.TestZ2_Drop_Offset cprod.h:438, Prod.TestZ1_Safe cprod.h:432,
//   Prod.TestY2_Middle cprod.h:428.  NO NEW #include is required by this part.
//
// TRANSLATION NOTES: golden text is otherwise verbatim, including trailing
// comment column positions.  Big5 comments decoded via cp950 and preserved as
// UTF-8, character for character.  Written UTF-8 + CRLF to match the port file.
// Translator: AI(k7-Arm1-decide) 20260810
//------------------------------------------------------------------------------

// =============================================================================
//  k7-Arm1-decide offline seams.  Uniquely prefixed (W7cK7_) so that being
//  file-scope `static` here can never become TRAP-1 shape (d).
// =============================================================================
// -- NG-3: FAITHFUL transcription of golden TMyKitSuck::CheckVaccumIsIniaialON
//    (golden mykitsuck.cpp:2732-2746), as a free function because the method is
//    absent from the aHotPlateSubstrate.h mirror.  Body is golden's, statement
//    for statement -- NOT an offline approximation.  See NG-3 in the banner.
static void W7cK7_CheckVaccumIsIniaialON(TMyKitSuck &kit, int iRow, int iCol, bool &flag)
{
    if(kit.Suck[iRow][iCol].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（黏料／殘料／不該有料）
    {
        if(kit.Item[iRow][iCol]==NULL_IC || kit.Item[iRow][iCol]==HAS_NULL_IC)
        {
            kit.Suck[iRow][iCol].Normal();
            flag=true;
        }
    }
    else
    {
        kit.Suck[iRow][iCol].Normal();
    }
}

// -- GATE k7-G1 stand-in for fiosetview->ProcessIndexSuckDestroy2() (golden
//    :4965).  Offline: report the index suck self-check "done" (true), the same
//    offline value as the four TU-local precedents cited in the banner.  Golden
//    discards the result at this call site.
static bool W7cK7_ProcessIndexSuckDestroy2(int iType=0) { return fiosetview->ProcessIndexSuckDestroy2(iType); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）
//------------------------------------------------------------------------------
bool DoIndexArm2PickUpErrNeedPiggyback(bool bInitial)                           //jou 20180814 : Index pick up error need piggyback
{                                                                               //Steven 20190115 : SCC要求吸取異常要檢查Socket
    static AnsiString ErrPart="";

    if(bInitial==true)
    {
        iIndexArm2PickUpErrNeedPiggybackTask=1;
        return false;
    }

    int iIndexUpPos=0;
    int iIndexCheckOffSet=IniConfig.fIndexCheckOffset*100;                      //ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset
    int &Task=iIndexArm2PickUpErrNeedPiggybackTask;
    bool flag=false, flag2=false;

    switch(Task)
    {
        case 1:
            Task=2030;                                                          //對齊DoCheckSocketHasIC()的Task
        case 2030:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]==NULL_IC ||
                       BTestSuck.Item[i][j]==HAS_NULL_IC)
                    {
                        BTestSuck.Suck[i][j].On();
                        if(INDEX_SUCKER_TYPE==1)
                        {
                            fiosetview->bIndexSuck[1][i][j]=true;
                            bIndexCheckNoStopVaccum=true;
                        }
                    }
                }
            }

            hCheckSockerDelay2.SetSecAndOn(1);
            Task=2040;
            break;
        case 2040:
            if(INDEX_SUCKER_TYPE==1)                                            //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
            {
                //AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE k7-G1 -- golden :4965 fiosetview->ProcessIndexSuckDestroy2() : TfiosetviewShim (atester_shims.h:404-409) exposes bIndexSuck[2][4][8] and its ctor only, no such method
                fiosetview->ProcessIndexSuckDestroy2();
                //AI(W906-IDXSUCK) （原 #endif）
                //AI(W906-IDXSUCK) 20260927: 上面已是 golden 的呼叫，這行離線替身停用（不然同一拍泵跑兩次）：W7cK7_ProcessIndexSuckDestroy2();                               // AI(k7-Arm1-decide) 20260810: GATE k7-G1 offline stand-in for the golden :4965 call above
            }

            if(hCheckSockerDelay2.Off())
            {
                Task=2050;
            }
            break;
        case 2050:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);                                                                                                    //Steven 20140620 : 整合為Function

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos+iIndexCheckOffSet, iSpeedFast))                                   //ChungHung 20140807 add for ATK TestZ_Test
            {
                if(CUSTOMER_CODE==CC_Greatek)                                                                                                                   //Wei 20160413
                    hCheckSockerDelay2.SetSecAndOn(5);                                                                                                          //Steven 20110908 : 上來後也要Delay一下
                else
                    hCheckSockerDelay2.SetSecAndOn(0.5);                                                                                                        //Steven 20110908 : 上來後也要Delay一下
                Task=2060;
            }
            break;
        case 2060:
            if(hCheckSockerDelay2.Off())
            {
                flag=false;
                ErrPart=" ";
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]==NULL_IC ||
                           BTestSuck.Item[i][j]==HAS_NULL_IC)
                        {
                            flag2=false;
                            W7cK7_CheckVaccumIsIniaialON(BTestSuck, i, j, flag2);  // AI(k7-Arm1-decide) 20260810: golden :4998 BTestSuck.CheckVaccumIsIniaialON(i, j, flag2) -- FAITHFUL free-function transcription of golden mykitsuck.cpp:2732-2746 (method absent from the aHotPlateSubstrate.h TMyKitSuck mirror; see NG-3)
                            if(flag2==true)
                            {
                                flag=true;
                                ErrPart+=IndexSuckName[i][j];

                                if(CUSTOMER_CODE==CC_SCS)                       //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                                    BTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }

                if(flag && LastSet.iRealDummy==REALLY)                          //Steven 20120726 : 有跑IC才檢查Socket
                {
                    IndexStatus=Z1_Z2_Normal;
                    Task=2070;                                                  //fail
                }
                else
                {
                    return true;
                }
            }
            break;
        case 2070:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoIndexArm2PickUpErrNeedPiggyback 2070"))
            {
                if(CUSTOMER_CODE==CC_SCS)                                       //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Item[i][j]==NULL_IC ||
                               BTestSuck.Item[i][j]==HAS_NULL_IC)
                            {
                                BTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                }
                Task=2080;
            }
            break;
        case 2080:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoIndexArm2PickUpErrNeedPiggyback 2080"))
            {
                Task=2090;
            }
            break;
        case 2090:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                bIsTestSitICFallDown=true;                                      //Steven 20130613
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)            //Steven 20130604 : Socket殘料要按Z1
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY2, false, ErrPart);
                }
                else
                {
                    ShowMyMessage("Arm2 detect Test Socket has IC error", "Arm 2偵測到Socket有IC殘留!!", "DoIndexArm2PickUpErrNeedPiggyback 2090");
                }
                ErrPart="";
                Task=2100;
            }
            break;
        case 2100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoIndexArm2PickUpErrNeedPiggyback 2100"))
            {
                IndexStatus=Z1Up_Z2Down;
                Task=2200;
            }
            break;
        case 2200:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, iSpeedFast))         //ChungHung 20140807 add for ATK TestZ_Test
            {
                return true;
            }
    }
    return false;
}

//------------------------------------------------------------------------------
// ROLE: the Arm1 (front) DISPATCH DECISION -- a pure priority ladder that returns
//   the Task number DoTestYFront should jump to next.  In order: Arm2 piggyback
//   socket check (16000), SLK separability combine (14000), RTC auto model verify
//   (12000), tester side-push retract (18500), then the normal vacuum-ON (returns
//   1 directly) vs vacuum-OFF (211 / 215) branch.  Returns 0 only if the
//   VacuumMode ladder somehow assigns nothing.
//
// WAVE SCOPE (label k7-Arm1-decide, golden file aTester_Front.cpp):
//   * CheckAnyCaseNeedToDoArm1()   golden :5080-5152   ACTIVE  (this part)
//
// SYMMETRIC TWIN: port aTester_Rear.cpp:7249-7345 (CheckAnyCaseNeedToDoArm2,
//   golden aTester_Rear.cpp :5256-5350).  The FRONT ladder is genuinely SHORTER
//   -- see GOLDEN ASYMMETRIES.
//
// GATE REGISTER
// -----------------------------------------------------------------------------
// This part writes NO #if 0 at all.  One absence claim still has to be declared,
// because the symbol it depends on does not exist in the tree TODAY:
//
// NG-1 -- NOT GATED, DELIBERATELY: `iHangupCTArm1=0;` at golden :5128 and :5149.
//   ABSENT TODAY: the global iHangupCTArm1 (golden aTester_Front.cpp:75
//   `int iHangupCTArm1=0;`) exists nowhere in the port tree -- not declared, not
//   defined, not shimmed.
//   COMMAND PROVING ABSENCE (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//     rg -n "\biHangupCTArm1\b" -g "*.h" -g "*.cpp" .
//   RESULT: zero hits.  RUN AT: 2026-08-10T21:42:35+0800.
//   WHY IT IS STILL ACTIVE AND NOT GATED (TRAP 3 re-ask, "why SHOULD this be
//     gated"): no reason survives.  golden :75 is one of the 16 between-function
//     file-scope globals THE MAIN LOOP ADDS IN ONE PLACE this wave (:75, :78,
//     :94-:98, :2232, :2235, :3943, :4071, :4921, :5160, :5161, :5162, :8010), so
//     by the time this part is stitched the symbol is a normal file-scope global
//     of this very TU -- golden's own home for it.  This is exactly the history of
//     the Rear twin: its GATE k8-G2 gated both `iHangupCTArm2=0;` stores on the
//     premise "the global exists nowhere", the premise DIED IN THE SAME WAVE
//     (TRAP 2) once PT-W7b declared it, and the gate was retired at integration
//     (port aTester_Rear.cpp:7300 / :7333).  Repeating that gate here would
//     knowingly re-introduce a hole the twin already closed: golden CLEARS the
//     Arm1 contact-test hang-up counter every time this ladder reaches either
//     VacuumMode branch, and dropping the stores would leave a counter that is
//     never reset, so a count accumulated across earlier cycles could latch a
//     spurious Arm1 hang-up.
//   WHY NOT DEFINE IT HERE INSTEAD: a TU-local `int iHangupCTArm1=0;` in this part
//     would be a duplicate definition the moment the main loop's globals block
//     lands (the link failed twice this wave for exactly that reason), so it is
//     deliberately NOT emitted here.
//   TRAP-2 HAND-OFF FOR THE INTEGRATOR: re-run the rg above at integration.  If
//     the main loop's globals block does NOT land, this part fails with a LOUD
//     undefined reference to iHangupCTArm1 -- never a silent behaviour change.
//     That loud failure is the reason ACTIVE is the safe choice here.
//
// DEPENDENCIES VERIFIED PRESENT (so they are ACTIVE, not gated):
//   * IniConfig.bD43IndexPickErrCheckSocket   Config.h:536
//   * bIndexArm2PickUpErrNeedPiggyback        cmydef.h:4720 -- the flag the Rear TU
//     RAISES at port aTester_Rear.cpp:2107/:2378; this ladder is its only CONSUMER,
//     so the producer/consumer pair is complete once this part lands.
//   * TestIF_File.bUseSLKClamp cprod.h:2175 / .iSeparabilityTest cprod.h:2176
//   * FTestCombineSLK(bool)  declared atester_shims.h:109; its body is still the
//     offline stub atester_shims.cpp:185 (returns true).  The CALL is faithful --
//     retiring that stub is the integrator's job, not a gate here.  (The Rear twin
//     is in the identical position with BTestCombineSLK.)
//   * REAL_TIME_CCD cmydef.h:2868; COM2->bCCDDummyRum atester_shims.h.  NOTE
//     (pre-existing, not mine): the shim ctor sets bCCDDummyRum=true offline, so
//     `!COM2->bCCDDummyRum` is false and the whole RTC ladder short-circuits
//     offline -- the shim's documented offline value, not a gate introduced here.
//   * CosFunction.bRTCAutoModelVerify / .bTesterSidePushFunction (CosFunction.h:382)
//     / .bSuckDevicesDuringTest (CosFunction.h:191)
//   * IniConfig.bD36EnableRTCAutoModelVerify Config.h:516
//   * bRTCAutoModelVerifyFirstTime cmydef.h:3586, bNeedWaitRTCAutoVerify
//     cmydef.h:4560, bRTCAutoVerifyControlEP cmydef.h:4713
//   * DeviceForm.ContactMode / .VacuumMode / .bTesterSidePush (cprod.h:1196) and
//     DeviceForm_File.iSidePushMode (cprod.h:1197); DropContact=1 /
//     DropPlaceShiftContact=7 MachineType.h:525/:531; VacuumONMode=0
//     MachineType.h:537
//   * SendSiteMapToRTC(bool,int)  atester.h:127, real body atester.cpp:11943
//     (checked: the :11863 twin IS inside `#if 0`, the :11943 one is NOT)
//   * CheckIndexAllSuckICFallDown(bool,bool)  csystem.h:209.  MEASURED, not
//     assumed: the csystem_predicates.cpp:329 copy is now inside
//     `#if 0 // PT-W5c RETIRED`, so the live body is csystem.cpp:22851 (checked
//     NOT inside any #if 0).
//   * RecordProcess(AnsiString,AnsiString="")  canary_support.h; LastSet.iRealDummy
//     / REALLY canary_support.h + cmydef.h
//   * FTestSuck.UseSiteHasIC() aHotPlateSubstrate.h:467 / .AlreadyTest() :464 /
//     .CountRealIC() :455.  TRAP 5: FTestSuck here is the aHotPlateSubstrate.h
//     TMyKitSuck (class :365, `extern` :636, object aHotPlateSubstrate.cpp:92) --
//     the 177-TU one, NOT the mykitsuck.h:274 class with the different layout
//     (mykitsuck.cpp:211 also defines an FTestSuck, but mykitsuck.cpp is
//     DELIBERATELY NOT REGISTERED, CMakeLists.txt:2099).  Already included by
//     aTester_Front.cpp:115, so no new or conflicting header dependency.
//   NO NEW #include is required by this part.
//
// GOLDEN ASYMMETRIES vs the Rear twin -- ALL PRESERVED, NONE "harmonised":
//   * FRONT HAS NO `bUseSocketFloat -> 15000` step (Rear golden :5292-5295) and NO
//     bIndexArm2SupplyLight/bForEgisTecTest/bD58 -> 11040 block in EITHER
//     VacuumMode branch (Rear golden :5315-5321 / :5341-5347).  The Front
//     vacuum-ON branch therefore `return 1;` DIRECTLY instead of assigning Task.
//   * FRONT's RTC condition carries one EXTRA conjunct Rear does not have:
//     `DeviceForm.ContactMode!=DropPlaceShiftContact` (golden :5101).
//   * SendSiteMapToRTC(false, 1) here vs (false, 2) in Rear;
//     CheckIndexAllSuckICFallDown(false, true) here vs (true, false) in Rear --
//     each routine checks the OPPOSITE arm, consistent with golden cross-wiring
//     the Front/Rear files.  NOT a bug; do not "correct" the argument order.
//   * `iHangupCTArm1=0;` sits BEFORE the DropContact check in the ON branch
//     (:5128) but AFTER the 211/215 assignment in the OFF branch (:5149) --
//     asymmetric on purpose in golden; both positions kept.
//
// GOLDEN QUIRKS PRESERVED, NOT FIXED:
//   * The `else` carrying `bRTCAutoVerifyControlEP=false;` at golden :5114-5117
//     hangs off the OUTER RTC `if`.  So when the outer RTC conditions ARE all true
//     but the inner UseSiteHasIC/AlreadyTest/SendSiteMapToRTC checks fail, the flag
//     is NOT cleared and keeps its previous value.  Looks like a misplaced else;
//     kept exactly as golden.  Identical quirk in the Rear twin.
//   * `int Task=0;` is initialised and then bypassed entirely by the four early
//     returns and by the vacuum-ON branch's `return 1;`.
//   * golden's own commented-out `// ShowIndexTime(1);` at :5135 is reproduced
//     verbatim, including its trailing timing note and its column-0 indentation.
//   * The continuation lines of the side-push condition are indented far to the
//     right in golden (:5120-5121); kept byte for byte.
//
// TRAP 1 (shape (b), the invisible one): `int CheckAnyCaseNeedToDoArm1() { return 0; }`
//   at atester_shims.cpp:168 (declared atester_shims.h:91) ALREADY satisfies every
//   existing call, so once this real body lands the build and the link both stay
//   green while the STUB is what actually runs, and `nm --undefined-only` cannot
//   see it.  The stub MUST be deleted in the same integrate step -- and it is not
//   merely inert: returning 0 instead of the real Task number sends DoTestYFront
//   to Task 0.  Non-static here, deliberately: golden is non-static and
//   atester_shims.h:91 already declares it non-static, so `static` would be shape (d).
//
// Translator: AI(k7-Arm1-decide) 20260810
//------------------------------------------------------------------------------
int CheckAnyCaseNeedToDoArm1()                                                  //Steven 20190115 : SCC要求吸取異常要檢查Socket
{
    int Task=0;

    if(IniConfig.bD43IndexPickErrCheckSocket &&
       bIndexArm2PickUpErrNeedPiggyback==true)
    {
        bIndexArm2PickUpErrNeedPiggyback=false;
        return 16000;
    }

    if(TestIF_File.bUseSLKClamp &&
       TestIF_File.iSeparabilityTest==1)                                        //JerryYang 20160429 分離模式
    {
        FTestCombineSLK(true);                                                  //初始化
        return 14000;                                                           //JerryYang 20160429 分離模式, 測試完成 Z2 SLK要與Clamp結合,再將IC吸起流程
    }

    if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum && LastSet.iRealDummy==REALLY &&
       CosFunction.bRTCAutoModelVerify==true && IniConfig.bD36EnableRTCAutoModelVerify==true &&
       bRTCAutoModelVerifyFirstTime==true &&                                    //jou 2014-06-24 RTC 自動進行Model驗證
       DeviceForm.ContactMode!=DropPlaceShiftContact)                           //ChungHung 20150528 add for 海思 _8Site1x4
    {
        if(FTestSuck.UseSiteHasIC() && FTestSuck.AlreadyTest()==false)
        {
            if(SendSiteMapToRTC(false, 1)==FTestSuck.CountRealIC())
            {
                bNeedWaitRTCAutoVerify=true;
                bRTCAutoModelVerifyFirstTime=false;
                RecordProcess("RTC auto verify start");
                return 12000;
            }
        }
    }
    else
    {
        bRTCAutoVerifyControlEP=false;
    }

    if(CosFunction.bTesterSidePushFunction==true &&
              DeviceForm.bTesterSidePush==true &&
              DeviceForm_File.iSidePushMode==1)                                 //Richard 20230301 : 測試完成, 側推縮回
    {
        return 18500;
    }

    if(DeviceForm.VacuumMode==VacuumONMode)
    {
        iHangupCTArm1=0;                                                // AI(k7-Arm1-decide) 20260810: golden :5128 -- ACTIVE, NOT gated. iHangupCTArm1 is golden aTester_Front.cpp:75, one of the 16 between-function file-scope globals the MAIN LOOP adds in one place; see GATE REGISTER NG-1.
        if(DeviceForm.ContactMode==DropContact &&                               //JerryYang 20170522 drop contact改為邊吸邊測
           CosFunction.bSuckDevicesDuringTest==true &&                          //JerryYang 20170804 (Steven) 移除邊吸邊測的選項
           INDEX_SUCKER_TYPE==1)
        {
            CheckIndexAllSuckICFallDown(false, true);                           //Steven 20110725 : 修改負壓檢查方式
        }
//                    ShowIndexTime(1);                                         //Steven 20140619 : 測試      //到這裡大概0.015~0.031Sec
        return 1;
    }
    else
    {
        if(DeviceForm.ContactMode==DropPlaceShiftContact)                       //ChungHung 20150528 add for 海思 _8Site1x4
        {
            Task=211;
        }
        else
        {
            Task=215;
        }

        iHangupCTArm1=0;                                                // AI(k7-Arm1-decide) 20260810: golden :5149 -- ACTIVE, NOT gated. iHangupCTArm1 is golden aTester_Front.cpp:75, one of the 16 between-function file-scope globals the MAIN LOOP adds in one place; see GATE REGISTER NG-1.
    }
    return Task;
}

//------------------------------------------------------------------------------
// ROLE: reset the front (Arm1) test-Y state machine back to its first step, by
//   setting the iTestYFrontTask task index to 1.  Called before (re)entering the
//   DoTestYFront engine so it restarts from a known step.
//
// WAVE SCOPE (label k7-Arm1-decide, golden file aTester_Front.cpp):
//   * InitTestYFrontTask()   golden :5155-5158   ACTIVE  (this part)
//
// SYMMETRIC TWIN: port aTester_Rear.cpp:7381-7384 (InitTestYRearTask, golden
//   aTester_Rear.cpp :5353-5356).  Same shape, same reasoning, same hand-off; the
//   Rear premise carries over to the Front side unchanged here.
//
// GATE REGISTER: none -- this part writes no #if 0 and makes no absence claim.
//   Its single dependency, iTestYFrontTask, is PRESENT: defined at
//   atester_shims.cpp:159 and declared `extern int iTestYFrontTask;` at
//   atester_shims.h:82, which this TU already includes (aTester_Front.cpp:103).
//   COMMAND (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//     rg -n "iTestYFrontTask" -g "*.h" -g "*.cpp" .
//   RESULT: present exactly as above (atester_shims.cpp:159, atester_shims.h:82).
//   RUN AT: 2026-08-10T21:40:20+0800.
//
// DELIBERATELY NOT DEFINED HERE: golden :5154 `int iTestYFrontTask=1;`.  It is a
//   plain data global that ALREADY exists at atester_shims.cpp:159 with the same
//   initial value 1, so re-defining it here would be a duplicate definition and
//   break the link.  It is also correctly absent from the main loop's 16-globals
//   list, for the same reason.  It must STAY shared: golden main.cpp registers
//   task-cursor ADDRESSES in QueueTaskList, so a TU-local copy would silently
//   detach the task-list monitor -- the same treatment port aTester_Rear.cpp's
//   file-top banner already documents for bArm2SuckFinish[][].
//
// TRAP 1 WARNING FOR THE INTEGRATOR (shape (b), the invisible one): the no-op stub
//   `void InitTestYFrontTask() {}` at atester_shims.cpp:169 ALREADY satisfies every
//   existing call, so once this real body lands the build and the link both stay
//   green while the STUB is what actually runs -- and `nm --undefined-only` cannot
//   see it, because the symbol IS defined, just by the wrong object.  The stub MUST
//   be deleted from atester_shims.cpp in the same integrate step (its Rear twin
//   precedent: atester_shims.cpp:206-208, already wrapped in
//   `#if 0 // PT-W7b RETIRED (InitTestYRearTask)`).  Behaviour difference while the
//   stub wins: the task cursor is never reset, so DoTestYFront resumes mid-sequence
//   instead of restarting at step 1.
//
// Non-static, deliberately -- golden is non-static, and a `static` definition here
// would be TRAP-1 shape (d) against the existing non-static declaration at
// atester_shims.h:92.
//
// Translator: AI(k7-Arm1-decide) 20260810
//------------------------------------------------------------------------------
void InitTestYFrontTask()
{
    iTestYFrontTask=1;
}

// ---- chunk 1/3 of DoTestYFront, golden 5165..6118 ----
bool DoTestYFront()
{
    static int iRetry=0;
    static bool bOneTimeFlag=true, flag1=false, bSocketCheckSkip=false, bSucketHasICError=false;
    static bool bGetTime1=true, bGetTime2=true, bOnce=false;                    //JerryYang 20220923 : add
    static bool bintered1=true;                                                 //Isaac 20200922 : 紀錄indexArmY encoder值和command值
    static bool bIndexFinish=false;
    static bool bCheckShuttle1=false;
    static bool bEPfirst=true, bFirst=true;                                     //Ifor 20150803 : 新增EP量測旗標，第一次才執行Log紀錄  //JerryYang 20180817 (Steven) : fix EOT delay
    static bool bSLKClampProcessFinish=false;                                   //JerryYang 20160526
    static bool bBTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false} };

    static bool bBTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},                                        //Steven 20110301 : 確認吸取完成
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static AnsiString ErrPart="";
    static AnsiString str="";
    //==> Eastsun 20260511 F006 整合: T11/T12 Index 掉料馬達位置記錄區域變數
    AnsiString StrPos="";                                                          //Ifor 20220125 add:Index 掉料當下 馬達位置
    int iArm1_Y=0, iArm1_Z=0, iArm2_Y=0, iArm2_Z=0;                                //Ifor 20220125 add:Index 掉料當下 馬達位置
    //<== Eastsun 20260511 F006 整合
    TestYFront_ReStart:                                                         //2008/10/20 lee

    QueueTaskList[19].CheckTaskChange();                                        //Steven 20200821 : 使用Goto也要記錄Task變化

    int &Task=iTestYFrontTask;
    int iIndexZPos=0;                                                           //Ifor 20220906 add:讀取Index Z 目前位置
    int iIndexUpPos=0, iCT=0;
    int ret=0, iFlag=0;
    int iNN=IsNNMode();
    int iFinishTestUpWaitHeight=0;                                              //jou 2011-10-31 start : 丟測模式與不同速度模式也要支援兩段式上升
    int iAZ1=0, iAZ2=0, iAZ3=0;                                                 //kevin 20150506
    bool TMode=false;
    bool bSpecialCheck=false;                                                   //Ifor 20210815 add:非正常模式掉料檢測
    bool bCheckAllSuck=false, flag=false, bCheckSuck=false, bCheckDestroy=false;
    long lPos[4]={0, 0, 0, 0};                                                  //kevin 20150915
    AnsiString str1, str2;
    AnsiString StrDate="";
    AnsiString sBufferT="";                                                     //kevin 20150506
    AnsiString sTesterSidePush="Cylinder Side Push Status Error";               //Richard 20220321 : 渠梁Side Push
    static int bRetryRTC=false;                                                 // AI(k1-DoTestYFront-c1) 20260810: golden :5210 is "static bRetryRTC=false;" -- K&R implicit int, accepted by BCB6, REJECTED by ISO C++.  GOLDEN BUG kept behaviourally identical by naming the same implicit type (int), NOT bool: every write is true/false and every read is a truth test, so 1/0 is exact.  Same treatment as the Rear twin, port aTester_Rear.cpp:7437.
    if(CUSTOMER_CODE==CC_SIGURD_HUKOU)                                          //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
    {
        iIndexArmCheck_SG_Arm1=Offset.iIndexArmContact[0];
        iIndexArmCheck_SG_Arm2=Offset.iIndexArmContact[1];
    }
    else
    {
        iIndexArmCheck_SG_Arm1=0;
        iIndexArmCheck_SG_Arm2=0;
    }

    switch(Task)
    {
        case 1:
            bIndex2Suck=false;                                                  //kevin 20220105 Index 在下真空建立 pause 不能關閉
            fFrontNeedSuck=false;
            fFrontNeedDestroy=false;
            InitFrontTestSuckICTask();
            InitFrontTestDestroyICTask();
            InitBTestSuckTestICTask();
            fRearNeedSuckIC=false;

            if(FTestSuck.UseSiteNoIC())
            {
                InitFrontTestSuckICTask();
                if(CanYieldAlarmRemainInSHT())                                  //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                {
                }
                else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)       //AI(ht9045-index-flow) 20260415 (RogerYang) : Shuttle1取消時不需要前臂吸取, 避免fCanMoveM鎖住造成F18偵測死鎖
                {
                }
                else
                {
                    fFrontNeedSuck=true;
                }
                //---------------------------------------
            }
            else
            {
                if((FTestSuck.UseSiteHasIC() &&
                    FTestSuck.AlreadyTest()) ||
                   FTestNeedDestroy())
                {
                    fFrontNeedDestroy=true;                                     // 先放掉已測IC,再進行新IC吸取
                    if(CanYieldAlarmRemainInSHT())                              //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                    {
                    }
                    else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)   //AI(ht9045-index-flow) 20260415 (RogerYang) : Shuttle1取消時不需要前臂吸取, 避免fCanMoveM鎖住造成F18偵測死鎖
                    {
                    }
                    else
                    {
                        fFrontNeedSuck=true;
                    }
                    InitFrontTestSuckICTask();
                    InitFrontTestDestroyICTask();
                }
            }

            fRearNeedTest=false;
            if(BTestSuck.UseSiteHasIC() &&
               BTestSuck.AlreadyTest()==false)
            {
                fRearNeedTest=true;
                bArm2IsTest=false;                                              //JerryYang 20180629 (wei) : 用來判斷是否在測試中
                InitBTestSuckTestICTask();
            }
            else if(iOneCycle &&
                    IsInArmOneCycleFinish()==true &&
                    bCanNotDisableOneCycle==false &&
                    FLCarryKit.UseSiteNoIC())
            {
                fFrontNeedSuck=false;
            }
            Task=100;
            goto TestYFront_ReStart;                                            //2008/10/20 lee
        //-------------------------
        //掉料時的處置方式
        //-------------------------
        case 50:
            MOT[MTestZ2].MovFlag=false;
            MOT[MTestZ2].bScanFlag=false;
            ShowIndexTime(-2);                                                  //Steven 20200715 : 重新計算Cycle Time
            DoTestYFrontDelay.SetMSAndOn(300);
            Task=52;
            break;
        case 52:
            if(DoTestYFrontDelay.Off())
                Task=55;
            break;
        case 55:                                                                //----- by dell ccd realtime-------------
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, 50000, "DoTestYFront 55"))
            {
//                if(REAL_TIME_CCD==true && COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
//                if(REAL_TIME_CCD==true && COM2->RTC_AlarmType())    //wei 20221222 RTC ARM Error
//                    Task=61;
//                else
//                    Task=60;

                if(IniConfig.bI26TestCloseSiteHaveBin && bTestBinDataError!=0)  //kevin 20150202 需強致將arm上 ic取出
                {
                    Task=56;
                    return false;
                }
                else
                {
#if 0 // GATE K1F1 -- golden :5317 COM2->RTC_AlarmType()
                    if(REAL_TIME_CCD==true && COM2->RTC_AlarmType())    //wei 20221222 RTC ARM Error
#else
                    if(REAL_TIME_CCD==true && false)    //wei 20221222 RTC ARM Error   // GATE K1F1 offline: no RTC alarm latched (TCOM2Shim has no RTC_AlarmType)
#endif
                    {
                        bRetryRTC=true;
                        Task=61;
                    }
                    else
                    {
                        bRetryRTC=false;
                    Task=60;
                }
//                    Task=60;
                }
            }
            break;
        case 56:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                Task=57;
                return false;
            }
            break;
        case  57:                                                               //kevin 20150202 需將arm上 ic取出
            if(IniConfig.bI26TestCloseSiteHaveBin && bTestBinDataError==0)      //kevin 20150202 需強致將arm上 ic取出
            {
                Task=55;
                return false;
            }
            else
            {
#if 0 // GATE K1F2 -- golden :5346 ShowMyMessageUp(...): NO declaration anywhere in the ported tree (only ShowMyMessage/ShowErrorMessage). Offline no-op, so case 57 keeps spinning on its own guard exactly as golden does while the operator has not answered.
                ShowMyMessageUp("Test Bin error,must take out all IC for test.", "測試bin 設定有問題,將ARM 1 上產品取出", false);
#endif
            }
            break;
        case 60:
            if(MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false ||
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false)
            {
                lPos[0]=Prod.TestZ1_Safe;                                                                               //kevin 20150915
                RecordIndexPositionError("DoTestYFront()Task=60", true, false, false, false, &lPos[0]);                 //kevin 20150915 record

                ShowIndexMotorError(AnsiString("DoTestYFront60"));
                break;
            }

            bSocketCheckSkip=false;
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYFront 60"))
            {
                Task=6000;
            }
            break;
        case 6000:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                str1.sprintf("DoTestYFront 6000: iShowSocketSensor=%d, bSucketHasICError=%d", iShowSocketSensor, (bSucketHasICError)?1:0);
                RecordProcess(str1);

                if(TestIF_File.bEnSocketSensor &&                               //Steven 20200709 : 換位置
                   iShowSocketSensor>0)                                         //kevin 20130504 socket sensor
                {
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)        //Steven 20151022 : add for MAXIM
                        bIsTestSitICFallDown=true;

                    bIsSocketSensor=true;
                    if(iShowSocketSensor==2)                                    //JerryYang 20161024 Socket sensor改成偵測置偏
                    {
                        ShowErrorMessage("WAR0323", K_RETRY, MTestZ2, false, sSocketSensorErr);
                        iShowSocketSensor=0;                                    //Jimmychiu 20230817 : Socket sensor異常顯示後參數歸零
                        CheckSocketSensor(0, "DoTestYFront_6000", true);
                        Task=78;                                                //Steven 20200615 : 置偏不用做RTC的檢查
                        break;
                    }
                    else if(iShowSocketSensor==1)
                    {
                        ShowErrorMessage("WAR0322", K_RETRY, MTestZ2, false, sSocketSensorErr);
                    }
                    iShowSocketSensor=0;
                    CheckSocketSensor(0, "DoTestYFront_6000", true);
                }

                if(bSucketHasICError)
                {
                    bIsTestSitICFallDown=true;
                    bSucketHasICError=false;
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)        //Steven 20130604 : Socket殘料要按Z1
                    {
                        ShowErrorMessage("WAR0310", K_RETRY, MTestY1, false, ErrPart);
                    }
                    else
                    {
                        ShowMyMessage("Socket has IC error", "Socket有IC殘留!!", "DoTestYFront 60");
                    }
                }

                if(IniConfig.bVTESTFunction==true &&                            //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
                   IniConfig.bGetRcsCheckingResult==true)
                {
#if 0 // GATE K1F3 -- golden :5412-5416 fMesSystem->asGetRcsCheckingResult / ->GetRcsCheckingResult(true). fMesSystem has NO home in the ported tree. Offline: RCS result treated as already PASS -> falls through to Task=61, i.e. the MES gate cannot hold the index arm up. Golden two enclosing IniConfig guards stay ACTIVE.
                    if(fMesSystem->asGetRcsCheckingResult!="PASS")
                    {
                        if(fMesSystem->GetRcsCheckingResult(true)==false)
                            break;
                    }
#endif
                }

                Task=61;                                                        //Steven 20201014 : 修正掉料異常時, index arm要分開
            }
            break;
        case 61:                                                                //----- by dell ccd realtime-------------
            if(REAL_TIME_CCD==true &&
               (MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false ||
                MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false))
            {
                ShowIndexMotorError(AnsiString("DoTestYFront61"));
                break;
            }

            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYFront 61"))
            {
                //----- by dell ccd realtime-------------
//                if(REAL_TIME_CCD==true && COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
#if 0 // GATE K1F1 -- golden :5435 COM2->RTC_AlarmType()
                if(REAL_TIME_CCD==true && COM2->RTC_AlarmType())    //wei 20221222 RTC ARM Error
#else
                if(REAL_TIME_CCD==true && false)    //wei 20221222 RTC ARM Error   // GATE K1F1 offline: no RTC alarm latched
#endif
                {
                    bRetryRTC=true;
                    Task=64;                                                    //Only RealTime
                }
                else
                {
                    bRetryRTC=false;
                    Task=62;
                }
            }
            break;
        case 62:
            if(bRecIndexDropAlarm1==true)
            {
                if(CheckIndexSuckICFallDownSetToHasNullIC(0))
                {
                    if(REAL_TIME_CCD==true)                                     //----- by dell ccd realtime-------------
                    {
                        COM2->DoReleaseAndInspEnd();
                    }
                    Task=2000;
                }
            }
            else
            {
                if(CheckIndexSuckICFallDownSetToHasNullIC(1))
                {
                    if(REAL_TIME_CCD==true)                                     //----- by dell ccd realtime-------------
                    {
                        COM2->DoReleaseAndInspEnd();
                    }
                    Task=2000;
                }
            }
            break;
        case 64:
            if(IndexAlarmInArmAway()==true)                                                                                                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
//                if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
#if 0 // GATE K1F1 -- golden :5475 COM2->RTC_AlarmType()
                if(COM2->RTC_AlarmType())   //wei 20221222 RTC ARM Error
#else
                if(false)   //wei 20221222 RTC ARM Error   // GATE K1F1 offline: no RTC alarm latched
#endif
                {
                    bRetryRTC=true;
                    if(CUSTOMER_CODE==CC_KYEC_LEE)                              //Ifor 20210505 add:
                    {
                        AnsiString Str="";
                        ErrPart="";
                        for(int i=0; i<4; i++)
                        {
                            if(iRTC_CCD_NG[i]!=0)
                            {
                                Str.sprintf("CCD %d, ", i+1);
                                ErrPart=ErrPart+Str;
                                iRTC_CCD_NG[i]=0;
                            }
                        }
                    }

#if 0 // GATE K1F4 -- golden :5493 COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm1NG]
                    if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm1NG])       //wei 20221222 RTC ARM Error
#else
                    if(false)       //wei 20221222 RTC ARM Error   // GATE K1F4 offline: RTC vision never answers (bRealTimeCom_ReceiveOK[] all false)
#endif
                    {
                        iRTCErrorCount++;
                        if(iRTCErrorCount>1)
                {
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)                                                                                        //Steven 20151022 : add for MAXIM
                        bIsTestSitICFallDown=true;
                            if(CUSTOMER_CODE==CC_KYEC_LEE)
                                ShowErrorMessage("WAR0354", 0, MMIndex, (bRTCArm1HalfViewError || bRTCArm2HalfViewError), ErrPart);   //RTC Socket Has Device Error
                            else
                                ShowErrorMessage("WAR0354", 0, MMIndex, (bRTCArm1HalfViewError || bRTCArm2HalfViewError), __FUNC__);   //RTC Socket Has Device Error
                            iRTCErrorCount=0;
                        }
                        else
                        {
                            sBufferT.printf("RTC ALARM Arm1 NG Auto Retry:%d", iRTCErrorCount);
                            MyDBIProcess("Message", sBufferT);
                        }
                    }
#if 0 // GATE K1F4 -- golden :5512 COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm2NG]
                    else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm2NG])  //wei 20221222 RTC ARM Error
#else
                    else if(false)  //wei 20221222 RTC ARM Error   // GATE K1F4 offline: RTC vision never answers
#endif
                    {
                        iRTCErrorCount++;
                        if(iRTCErrorCount>1)
                        {
                            if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)     //Steven 20151022 : add for MAXIM
                                bIsTestSitICFallDown=true;
                            if(CUSTOMER_CODE==CC_KYEC_LEE)
                                ShowErrorMessage("WAR0355", 0, MMIndex, (bRTCArm1HalfViewError || bRTCArm2HalfViewError), ErrPart);   //RTC Socket Has Device Error
                            else
                                ShowErrorMessage("WAR0355", 0, MMIndex, (bRTCArm1HalfViewError || bRTCArm2HalfViewError), __FUNC__);   //RTC Socket Has Device Error
                            iRTCErrorCount=0;
                        }
                        else
                        {
                            sBufferT.printf("RTC ALARM Arm2 NG Auto Retry:%d", iRTCErrorCount);
                            MyDBIProcess("Message", sBufferT);
                        }
                    }
                    else
                {
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)        //Steven 20151022 : add for MAXIM
                        bIsTestSitICFallDown=true;
                        if(CUSTOMER_CODE==CC_KYEC_LEE)
                            ShowErrorMessage("WAR0342", 0, MMIndex, (bRTCArm1HalfViewError || bRTCArm2HalfViewError), ErrPart);   //RTC Socket Has Device Error
                        else
                    ShowErrorMessage("WAR0342", 0, MMIndex, (bRTCArm1HalfViewError || bRTCArm2HalfViewError), __FUNC__);                                        //RTC Socket Has Device Error
                    }
//                    COM2->RTC_ResetAlarm();       //wei 20221222 RTC ARM Error
                    COM2->DoReleaseAndInspEnd();                                                                                                                //Steven 20120522 : 換位置
                    bRTCArm1HalfViewError=true;                                                                                                                 //Steven 20120206 : RTC重複錯誤

                    if(IniConfig.bD49RTCAlarmSetIndexToErrBin)                                                                                                  //JerryYang 20160712 for 力成,發生RTC Alarm時把Index上所有IC設為Errorbin
                    {
                        iCT=FTestSuck.CountRealIC();
                        AnsiString sBuffer;
                        sBuffer.printf("RTC Alarm set Arm1 Place to Error bin : Device=%d;", iCT);
                        MyDBIProcess("Message", sBuffer);
                        FTestSuck.SetAllRealIC2InterfaceBin();

                        iCT=BTestSuck.CountRealIC();
                        sBuffer.printf("RTC Alarm set Arm2 Place to Error bin : Device=%d;", iCT);
                        MyDBIProcess("Message", sBuffer);
                        BTestSuck.SetAllRealIC2InterfaceBin();
                    }
                    Task=65;                                                                                                                                    //if RealTime NG ---->Next Start FullView
                }
            }
            break;
        case 65:                                                                //----- by dell ccd realtime-------------
            if(REAL_TIME_CCD==true && MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false ||
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false)
            {
                ShowIndexMotorError(AnsiString("DoTestYFront65"));
                break;
            }

            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front,Prod.TestY2_Rear, 50000, "DoTestYFront 65"))
            {
#if 0 // GATE K1F4 -- golden :5571-5573 COM2->bRealTimeCom_ReceiveOK[] write + two SendCommToVision(rtCHECKNULL/rtFullTOK) (TCOM2Shim has none of these). Offline no-op: no full-view request is ever sent, so case 66 can only time out on iWaitIndexArm1.
                COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]=false;            //----- by dell ccd realtime-------------
                COM2->SendCommToVision(COM2->rtCHECKNULL, true);
                COM2->SendCommToVision(COM2->rtFullTOK, true);
#endif
                iWaitIndexArm1.SetSecAndOn(10);
                Task=66;
            }
            break;
        case 66:
#if 0 // GATE K1F4 -- golden :5579 COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK]
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK])
#else
            if(false)   // GATE K1F4 offline: RTC vision never answers
#endif
            {
//                RecordProcess("REALTIME CCD Full OK");
//                COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC]=false;
//                COM2->RTC_ResetAlarm();     //wei 20221222 RTC ARM Error
                COM2->DoReleaseAndInspEnd();
                bRTCFullViewError=false;                                        //Steven 20120206 : RTC重複錯誤
                if(IniConfig.bD75OneCycleFinishedAlawayLearnRTCGolden ||        //Sam 20240117 : OneCycle 完成做完 Full view check 後都需要做 RTC Learning golden
                   bTriggerRTC_AutoSTD)                                         //JerryYang 20240829 : SPIL訓永 要求手動觸發RTC AUTO STD
                {
#if 0 // GATE K1F5 -- golden :5589 fContact->InitROILearningTask(): TfContactShim (atester_shims.h:154-290) has no such method. Offline no-op; Task=68 is still taken, where the ROI learn itself is served by the REAL 0-arg Do_ROILearning() (see GATE K1F6).
                    fContact->InitROILearningTask();
#endif
                    Task=68;
                }
                else
                {
                    Task=70;
                }
#if 0 // GATE K1F4 -- golden :5596 COM2->RTC_ResetAlarm() (no such method). Offline no-op.
                COM2->RTC_ResetAlarm();     //wei 20221222 RTC ARM Error
#endif
                bRetryRTC=false;
                break;
            }

#if 0 // GATE K1F4 -- golden :5601 COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG])
#else
            if(false)   // GATE K1F4 offline: RTC vision never answers
#endif
            {
                Task=67;
                break;
            }

            if(iWaitIndexArm1.Off())
            {                                                                   //Steven 20110824 : Real time CCD - 不可以關閉CCD
#if 0 // GATE K1F4 -- golden :5609 COM2->OpenRTCComPortAgain()
                if(COM2->OpenRTCComPortAgain())
#else
                if(false)   // GATE K1F4 offline: RTC com port is never re-opened, so the WAR0337 popup stays silent and case 66 just re-arms case 65
#endif
                    ShowErrorMessage("WAR0337", 0, MMIndex, 0, __FUNC__);       //RTC FullT Time Out Error.   //ChungHung 20140520 add 不要第一次TimeOut就秀錯誤訊息
                bSendRealCCDSendStart=true;
                Task=65;
            }
            break;
        case 67:
            if(IndexAlarmInArmAway()==true)                                                                             //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)                                                    //Steven 20151022 : add for MAXIM
                    bIsTestSitICFallDown=true;

                COM2->DoReleaseAndInspEnd();
                if(CosFunction.bRTCFullViewErrorOnlyRetry)                                                              //Steven 20140529 : RTCFullViewErrorOnlyRetry
                    ret=ShowErrorMessage("WAR0346", K_RETRY, MMCCD, bRTCFullViewError, ErrPart);                        //RTC FullView Socket Has Device Error!
                else
                    ret=ShowErrorMessage("WAR0346", K_RETRY|K_SKIP, MMCCD, bRTCFullViewError, ErrPart);                 //RTC FullView Socket Has Device Error!

                bRTCFullViewError=true;                                                                                 //Steven 20120206 : RTC重複錯誤

                if(ret==K_SKIP)                                                                                         //Steven 20120823 : Run Time出錯也要RTC
                {
#if 0 // GATE K1F5 -- golden :5631 fContact->InitROILearningTask() (see :5589).
                    fContact->InitROILearningTask();
#endif
                    bRTCFullViewError=false;
                    Task=68;
                }
                else
                {
                    Task=65;
                }
            }
            break;
        case 68:                                                                //Steven 20120823 : Run Time出錯也要RTC
#if 0 // GATE K1F6 -- golden :5642 fContact->Do_ROILearning(true)
            if(fContact->Do_ROILearning(true))
#else
            if(fContact->Do_ROILearning())   // GATE K1F6 offline substitute: the REAL shim body (still linked), bQuickLearn arg dropped
#endif
            {
                Task=70;
            }
            break;
        case 70:
            if(IniConfig.bEnableCCDUSETCPIP)                                    //Eliot 2010_1209 start
            {
                CCDInterfaceForm->CCDTimerOnOff(IniConfig.bC02InstallCCD);      //Steven 20110809
                if(IniConfig.bC02InstallCCD==true)
                    Task=72;
                else
                    Task=78;
                break;
            }
            else
            {
                Task=78;
                break;
            }
        case 72:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYFront 72"))
            {
                fMain->lbCCDStatus->Visible=true;
                fMain->LightOn();
                DoTestYFrontDelay.SetSecAndOn(3);
                iCCDTimeOutCount=0;
                if(CCDInterfaceForm->bAtestScanCCDProgram)
                {
#if 0 // GATE K1F7 -- golden :5671 CCDInterfaceForm->CloseCCDForm(): TCCDInterfaceFormShim (atester_shims.h:296-303) exposes only bAtestScanCCDProgram / CCDTimerOnOff / CCDRunExec. Offline no-op; the RecordProcess("Stop CCD check.") beside it stays ACTIVE.
                    CCDInterfaceForm->CloseCCDForm();
#endif
                    RecordProcess("Stop CCD check.");
                }
                Task=73;
            }
            break;
        case 73:
            if(DoTestYFrontDelay.Off())
            {
#if 0 // GATE K1F7 -- golden :5680-5681 CCDInterfaceForm->CCDIdentificationOpen() / ->CCDIdentification() (no such methods). Offline no-op: no CCD program is launched.
                CCDInterfaceForm->CCDIdentificationOpen();
                CCDInterfaceForm->CCDIdentification();                          //kevin 20110811
#endif
                RecordProcess("Start CCD check.");
                DoTestYFrontDelay.SetSecAndOn(20);                              //CCDTimeOutSec);  20110810 設定5秒太短會一直取像
                DoTestYFrontDelay2.SetMSAndOn(200);
                Task=75;
            }
            break;
        case 75:
#if 0 // GATE K1F7 -- golden :5689-5690 CCDInterfaceForm->bCCDProgramExistence (no such member)
            if(DoTestYFrontDelay2.Off()==false ||
               CCDInterfaceForm->bCCDProgramExistence==false)                   //ChungHung 20121127 add 等待程式開啟
#else
            if(DoTestYFrontDelay2.Off()==false ||
               // AI(pt-wave) 20260811 PT-W7c INTEGRATE -- GATE K1F7 offline default CORRECTED from `true`
               // to `false`, and the reason is worth stating because the original choice was the
               // SEMANTICALLY faithful one and still deadlocked.
               //   golden :5689-5690 is  if(DoTestYFrontDelay2.Off()==false || bCCDProgramExistence==false)
               //   Offline there genuinely is no CCD program, so bCCDProgramExistence==false is TRUE, and
               //   substituting `true` mirrors golden's state exactly. But the SAME gate removes golden
               //   :5695 CCDInterfaceForm->CCDIdentificationOpen(), so nothing can ever make the program
               //   exist -- the condition stays true forever, the `return false` below runs every tick,
               //   Task is never written, and case 75 parks permanently. The timeout arm at golden :5721
               //   (which this part's own gate note claimed was the exit) is unreachable because the
               //   return happens first. Reachable offline, not dead code: 55->60->61->62->2000 takes its
               //   else (atester_shims.cpp:362 constructs bCCDDummyRum=true) ->70, and case 70 needs only
               //   IniConfig.bEnableCCDUSETCPIP + bC02InstallCCD, both set by customer profiles in
               //   CosFunction.cpp -> 72 -> 73 -> 75.
               //   `false` instead routes through golden's OWN exit for a CCD that never answers: the
               //   20s DoTestYFrontDelay timeout arm, which does iCCDTimeOutCount++ and Task=73. That is
               //   golden's real behaviour on CCD timeout, and it matches the in-tree precedent for this
               //   same golden code -- atester.cpp:5878-5880 gates the identical CCD result poll and then
               //   ADVANCES rather than parking. TQPF_Timer::Off() is true on a never-armed timer
               //   (myTimer.cpp:40-44 returns now>=rEnd, and the ctor leaves rEnd at a construction-time
               //   counter), so the timeout arm is taken on the first tick. Found by this wave's own
               //   adversarial audit of chunk 1; verified line by line against golden before changing.
               false)                  // GATE K1F7 offline: bCCDProgramExistence -- see the note above
#endif
            {
                if(DoTestYFrontDelay2.Off()==true)
                {
                    DoTestYFrontDelay2.SetMSAndOn(200);
#if 0 // GATE K1F7 -- golden :5695 CCDInterfaceForm->CCDIdentificationOpen() (see :5680).
                    CCDInterfaceForm->CCDIdentificationOpen();
#endif
                    DoTestYFrontDelay.SetSecAndOn(20);                          //CCDTimeOutSec);  20110810 設定5秒太短會一直取像
                }
                return false;
            }

#if 0 // GATE K1F7 -- golden :5701-5719 CCDInterfaceForm->bIdentificationFinish / ->iIdentificationStatus (no such members). Offline: identification NEVER completes, matching this shim family "the vision/CCD subsystem never answers" convention, so case 75 always leaves via the DoTestYFrontDelay.Off() timeout arm below. TWO collateral symbols are gated out with it, and they are NOT absent: SW[SwCCDLight] IS real (myswitch.h:43 SW[] + cmydef.h:1769 SwCCDLight) and fMain->lbCCDStatus IS real (forms/fMain.h:176) -- they die only because their enclosing block does. The third one IS absent: fShowMessage->FormClick(fShowMessage) -- TfShowMessage (forms/fShowMessage.h:22-29) declares ShowSpeed(bool) only (SUB-GATE K1F8).
            if(CCDInterfaceForm->bIdentificationFinish==true)
            {
                if(CCDInterfaceForm->iIdentificationStatus==1)                  //0:未測試 1:Pass 2:Fail
                {
                    SW[SwCCDLight].Off();
                    Task=76;
                }
                else if(CCDInterfaceForm->iIdentificationStatus==2)
                {
                    Task=7500;
                }
                else
                {
                    Task=72;                                                    //jou 2012-12-06 修正 CCD 測試結果為0的時候會直接by pass
                }
                fShowMessage->FormClick(fShowMessage);
                fMain->lbCCDStatus->Visible=false;
                break;                                                          //kevin 20110811
            }
#endif

            if(DoTestYFrontDelay.Off())
            {
                iCCDTimeOutCount++;
#if 0 // GATE K1F7 -- golden :5724 CCDInterfaceForm->RetrunUpStep() (no such method). Offline no-op.
                CCDInterfaceForm->RetrunUpStep();
#endif
                if(iCCDTimeOutCount>(ChangeToFloatNonPcnt((double)(100), (double)(CCDTimeOutSec))))
                {
#if 0 // GATE K1F7 -- golden :5727 CCDInterfaceForm->CloseCCDForm() (see :5671).
                    CCDInterfaceForm->CloseCCDForm();
#endif
                    RecordProcess("Stop CCD check.");
                    ShowMyMessage("CCD Time out", "CCD判斷時間過長!!");
                    iCCDTimeOutCount=0;
                }
                Task=73;
                break;
            }
            break;
        case 7500:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, 50000, "DoTestYFront 7500"))
            {
                RecordProcess("CCD check socket fail!!!!!!!!");
                fMain->Pause("CCD check socket fail");
                Task=70;
            }
            break;
        case 76:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, 50000, "DoTestYFront 76"))
            {
                iCCDTimeOutCount=0;
                Task=78;
            }
            break;
        case 78:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 50000, "DoTestYFront 78"))
            {
                CCDInterfaceForm->CCDTimerOnOff(false);                         //Steven 20110809
                Task=79;
            }
            break;
        case 79:
            if(CheckSocketSensor(1, "DoTestYRear_79_"))                         //Steven 20200617 : Socket sensor Alarm後,要再檢查一次
            {
                Task=50;                                                        //Steven 20200821 : 61 --> 50
            }
            else
            {
                Task=80;
            }
            break;
        case 80:
            if(bRecIndexDropAlarm2==true)                                       //JerryYang 20180122 index arm掉料要用掉料的arm做index check
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, 50000, "DoTestYFront 80"))
                {
                    Task=10081;
                }
            }
            else
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, 50000, "DoTestYFront 80"))
                {
                    Task=81;
                }
            }
            break;
        case 10081:                                                                                                                                             //JerryYang 20180122 (Steven) index arm掉料要用掉料的arm做index check
            if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset-iIndexArmCheck_SG_Arm2, 50000, 0))                                      //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                IndexStatus=Z1Up_Z2Down;
                if(LastSet.bD41TestSocketICCheckSkip)                                                                                                           //打勾的話就跳過檢查
                {
                    bRecIndexDropAlarm2=false;
                    Task=78;
                    break;
                }

//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE K1F9 -- golden :5795 fiosetview->ResetIndexSuck(): TfiosetviewShim (atester_shims.h:404-410) exposes bIndexSuck[2][4][8] and its ctor ONLY. Offline no-op, same as atester_32Site.cpp:207 W5_32S_FIOSET_RESET().
                fiosetview->ResetIndexSuck();
//AI(W906-IDXSUCK) （原 #endif）

                if(TestIF_File.bArm1PickPlaceArm2Test==false ||
                   ((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                                                            //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                     TestIF_File.bArm1PickPlaceArm2Test==true) &&                                                                                               //Steven 20150129 : 需要確認Arm2有沒有粘料
                    TestIF_File.bCheckArm2Vacuum==true))                                                                                                        //Steven 20150129 : 需要確認Arm2有沒有粘料
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(INDEX_SUCKER_TYPE==0)
                            {
                                BTestSuck.Suck[i][j].On();
                            }
                            else                                                                                                                                //負壓檢查
                            {                                                                                                                                   //有個盲點就是如果掉落IC剛好掉到關Site的地方，就會無法知道。
                                if((BTestSuck.Item[i][j]==NULL_IC ||                                                                                            //jou 20170516 (Steven) : fix index drop error ,index check arm2失效.
                                    BTestSuck.Item[i][j]==HAS_NULL_IC) &&                                                                                       //Steven 20210309 : 有IC的地方不檢查
                                   bTestSiteUse[1][i][j]==true)                                                                                                 //增加關Site時就不開真空偵測，
                                {
                                    flag1=false;                                                                                                                //jou 20110503 start
                                    fiosetview->bIndexSuck[1][i][j]=true;
                                    do
                                    {
                                        if(flag1==false)
//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE K1F10 -- golden :5821 fiosetview->ProcessIndexSuckDestroy2()
                                           flag1=fiosetview->ProcessIndexSuckDestroy2();
//AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                                           flag1=true;   // GATE K1F10 offline: suck self-check "done" -- LOAD-BEARING, this sits inside do{...}while(flag1==false) so false would spin the UI thread forever
//AI(W906-IDXSUCK) （原 #endif）

                                        MySleepEx(1, true);
                                    }
                                    while(flag1==false);                                                                                                        //jou 20110503 end
                                }
                            }
                        }
                    }
                }

                DoTestYFrontDelay.SetSecAndOn(0.5);
                Task=10082;
                flag1=false;
            }
            break;
        case 10082:
            if(DoTestYFrontDelay.Off() || INDEX_SUCKER_TYPE==1)                 //jou 20110503
            {
                Task=10084;
            }
            break;
        case 10084:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);                                                                                                    //Steven 20140620 : 整合為Function

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm2,iSpeedFast))                               //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                if(CUSTOMER_CODE==CC_Greatek)                                                                                                                   //Wei 20160413
                    DoTestYFrontDelay.SetSecAndOn(5);                                                                                                           //Steven 20110908 : 上來後也要Delay一下
                else
                    DoTestYFrontDelay.SetSecAndOn(0.5);                                                                                                         //Steven 20110908 : 上來後也要Delay一下
                Task=10090;
            }
            break;
        case 10090:
            if(DoTestYFrontDelay.Off())
            {
                bSucketHasICError=false;                                        //Steven 20101214
                if(LastSet.iRealDummy==REALLY)
                {
                    ErrPart=" ";
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(CheckTestSuckICOn(BTestSuck, i, j))              //Steven 20241204 : 修正i, j相反的問題
                            {
                                bSucketHasICError=true;
                                ErrPart+=IndexSuckName[j][i];
                            }
                        }
                    }
                }
                Task=10095;
            }
            break;
        case 10095:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]==NULL_IC ||
                       BTestSuck.Item[i][j]==HAS_NULL_IC)
                    {
                        BTestSuck.Suck[i][j].Off();
                    }
                }
            }
            DoTestYFrontDelay.SetMSAndOn(500);
            Task=10097;
            break;
        case 10097:
            if(DoTestYFrontDelay.Off())
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]==NULL_IC ||
                           BTestSuck.Item[i][j]==HAS_NULL_IC)
                        {
                            BTestSuck.Suck[i][j].Normal();                      //jou 2011-11-01 開破壞不可能同時開真空，所以OffDestroy -> Normal
                        }
                    }
                }

                if(bSucketHasICError)                                           // sucket has ic error  //Steven 20101214 : 換位置，換到Delay.Off()裡面
                {
                    Task=50;
                    if(CUSTOMER_CODE==CC_AMKOR_China ||                         //Steven 20101112
                       IniConfig.bKoreaFunction ||
                       CUSTOMER_CODE==CC_QUALCOMM)                              //JerryYang 20170412 (Steven) add QUALCOMM
                    {
                        if(bSocketCheckSkip)
                        {
                            bSocketCheckSkip=false;
                            Task=78;
                            bRecIndexDropAlarm2=false;
                        }
                    }
                }
                else
                {
                    bRecIndexDropAlarm2=false;
                    Task=78;
                }
            }
            break;
        case 81:
            if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset-iIndexArmCheck_SG_Arm1, 50000, 0))                                      //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                IndexStatus=Z1Down_Z2Up;
                if(LastSet.bD41TestSocketICCheckSkip)                                                                                                           //打勾的話就跳過檢查
                {
                    Task=100;
                    break;
                }

//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE K1F9 -- golden :5939 fiosetview->ResetIndexSuck() (see :5795).
                fiosetview->ResetIndexSuck();
//AI(W906-IDXSUCK) （原 #endif）

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(INDEX_SUCKER_TYPE==0)
                        {
                            FTestSuck.Suck[i][j].On();
                        }
                        else                                                                                                                                    //負壓檢查
                        {                                                                                                                                       //有個盲點就是如果掉落IC剛好掉到關Site的地方，就會無法知道。
                            if((FTestSuck.Item[i][j]==NULL_IC ||
                                FTestSuck.Item[i][j]==HAS_NULL_IC) &&                                                                                           //Steven 20210105 : 修正有IC的地方不檢查
                               bTestSiteUse[0][i][j]==true)                                                                                                     //增加關Site時就不開真空偵測，
                            {
                                flag1=false;                                                                                                                    //jou 20110503 start
                                fiosetview->bIndexSuck[0][i][j]=true;
                                do
                                {
                                    if(flag1==false)
//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE K1F11 -- golden :5960 fiosetview->ProcessIndexSuckDestroy1()
                                        flag1=fiosetview->ProcessIndexSuckDestroy1();
//AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                                        flag1=W64B_FIOSET_PISD1(0);   // GATE K1F11 offline substitute: this TU own established seam (aTester_Front.cpp:168, returns true), iType=0 for golden zero-arg call
//AI(W906-IDXSUCK) （原 #endif）

                                    MySleepEx(1, true);
                                }
                                while(flag1==false);                                                                                                            //jou 20110503 end
                            }
                        }
                    }
                }

                DoTestYFrontDelay.SetSecAndOn(0.5);
                Task=82;
                flag1=false;
            }
            break;
        case 82:
            if(DoTestYFrontDelay.Off() || INDEX_SUCKER_TYPE==1)                 //jou 20110503
            {
                Task=84;
            }
            break;
        case 84:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);                                                                                                    //Steven 20140620 : 整合為Function

            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm1, iSpeedFast))                              //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                if(CUSTOMER_CODE==CC_Greatek)                                                                                                                   //Wei 20160413
                    DoTestYFrontDelay.SetSecAndOn(5);                                                                                                           //Steven 20110908 : 上來後也要Delay一下
                else
                    DoTestYFrontDelay.SetSecAndOn(0.5);                                                                                                         //Steven 20110908 : 上來後也要Delay一下
                Task=90;
            }
            break;
        case 90:
            if(DoTestYFrontDelay.Off())
            {
                bSucketHasICError=false;                                        //Steven 20101214 : 換位置
                if(LastSet.iRealDummy==REALLY)
                {
                    ErrPart=" ";
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(CheckTestSuckICOn(FTestSuck, i, j))              //Steven 20241204 : 修正i, j相反的問題
                            {
                                bSucketHasICError=true;
                                ErrPart+=IndexSuckName[i+iNN][j];
                            }
                        }
                    }
                }
                Task=95;
            }
            break;
        case 95:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==NULL_IC ||
                       FTestSuck.Item[i][j]==HAS_NULL_IC)
                    {
                        FTestSuck.Suck[i][j].Off();
                    }
                }
            }
            DoTestYFrontDelay.SetSecAndOn(0.05);
            Task=97;
            break;
        case 97:
            if(DoTestYFrontDelay.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==NULL_IC ||
                           FTestSuck.Item[i][j]==HAS_NULL_IC)
                        {
                            FTestSuck.Suck[i][j].Normal();                      //jou 2011-11-01 開破壞不可能同時開真空，所以OffDestroy -> Normal
                        }
                    }
                }

                if(bSucketHasICError)                                           // sucket has ic error  //Steven 20101214 : 換位置，換到Delay.Off()裡面
                {
                    Task=50;
                    if(bSocketCheckSkip)
                    {
                        bSocketCheckSkip=false;
                        Task=100;
                    }
                }
                else
                {
                    Task=100;
                }
            }
            break;
        case 100:
            if(CheckIndexStatus("DoTestYFront"))
            {
                if(ATC_SYSTEM==eATCSiliconType &&
                   Temperature.bATCActiveCooling==true &&                       //jou 2012-03-14 增加ATC控制方式
                   Temperature.bATCTestStrat==true &&
                   (TestIF.iTestMode==DualSite ||
                    TestIF.iTestMode==SingleSite))
                {
                    if(bATCInitialFinish==false ||
                       COM2->ATCAlarmSenCheck()==false)
                    {
                        ShowMyMessage("ATC Alarm Sensor Off, please check ATC system is OK!", "ATC警報偵測, 請確認ATC系統是否正常!");
                        return false;
                    }
                }

                iRetry=0;
                //==> Eastsun 20260511 F006 整合: Ifor 20220308+20230524 KLT 兩段式上升
                if(CUSTOMER_CODE == CC_KYEC_LEE && bEnable_KLT_Function==true && DeviceForm.ContactMode==DropContactModeDiffentSpeed && Prod.TestZ1_Up_Offset!=0)//Ifor 20220308 add:Index 兩段式上升 //Ifor 20230524 add: 設定不為0才執行
                {
                    Task=20000;
                }
                //<== Eastsun 20260511 F006 整合
                else if(IniConfig.bD21EnableFinishTestUpWait &&                 //jou 2011-10-31 start : 丟測模式與不同速度模式也要支援兩段式上升
                   IndexStatus==Z1Down_Z2Up)                                    //Steven 20180613 (Jou) : 增加保護機制, 避免撞機
                {
                    Task=105;
                }
                else
                {
                    Task=108;
                }
                goto TestYFront_ReStart;                                        //2008/10/20 lee
            }
            break;
        case 105:
            if(DeviceForm.ContactMode==DropContact ||                           //jou 2011-10-31 start : 丟測模式與不同速度模式也要支援兩段式上升
               DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
               DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||
               DeviceForm.ContactMode==TMoveDrop   ||
               DeviceForm.ContactMode==TMoveDropSlowContact ||
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
                iFinishTestUpWaitHeight=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+IniConfig.iD21FinishTestUpWaitHeight;
            }
            else
            {
                iFinishTestUpWaitHeight=Prod.TestZ1_Test+IniConfig.iD21FinishTestUpWaitHeight;
            }

            if(MOT[MTestZ1].Gali_MotMove(iFinishTestUpWaitHeight, 10000))
            {
                Task=106;
                DoTestYFrontDelay.SetMSAndOn(IniConfig.iD21FinishTestUpWaitTime);
            }

            if(Task!=106)                                                       //Steven 20180813 : add index arm speed
                break;

// ---- chunk 2/3 of DoTestYFront, golden 6119..7035 ----
        case 106:
            QueueTaskList[19].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化
            if(DoTestYFrontDelay.Off())
            {
                Task=108;
            }

            if(Task!=108)                                                       //Steven 20180813 : add index arm speed
                break;
        case 108:
            QueueTaskList[19].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化
            if(CUSTOMER_CODE==CC_ASE_KaohSiung_K12 &&
               (LastSet.iTemperature==Tempture_Hot) && bFirstZ1UPZ2Down)        //kevin 20131112 加熱模式預先動作避免第一顆溫度過高
            {
                if(IndexStatus==Z1Up_Z2Down && REAL_TIME_CCD==true &&           //----- by dell ccd realtime-------------
                   !COM2->bCCDDummyRum &&
                   TestIF_File.bUseSocketFloat==false)
                {
                    if(bRealCCDSendArm)
                    {
                        #if 0 // GATE F-G1 -- golden :6139 COM2->SendCommToVision(COM2->rtArmIndex2, true).  AI(W7C-DoTestYFront-c2) 20260810
                        //        WHY THE OFFLINE DEFAULT IS FAITHFUL: this is a pure OUTBOUND notify to the DTK real-time-CCD
                        //        vision box ("arm index 2 is clear"); nothing in this SM reads it back, and the enclosing guard
                        //        already requires REAL_TIME_CCD && !COM2->bCCDDummyRum, and TCOM2Shim's ctor sets
                        //        bCCDDummyRum=TRUE (atester_shims.cpp:362), so golden itself would never reach this line offline.
                        //        REAL-MACHINE DIFFERENCE: the RTC camera is never told the arm has moved clear, so it would not
                        //        start its half-view grab.  ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800
                        //        MIRROR: aTester_Rear.cpp:8530 gates the arm-1 twin identically (GATE K1G1); the premise holds
                        //        unchanged on the Front side -- same shim, same ctor default.
                        COM2->SendCommToVision(COM2->rtArmIndex2, true);
                        #endif // GATE F-G1
                        iWaitIndexArm1.SetMSAndOn(200);
                        bRealCCDSendArm=false;
                    }
                }

                bCheckShuttle1=false;
                Task=110;
            }
            else
            {
                if(MOT[MTestZ2].Gali_ReadPos()!=Prod.TestZ2_Safe)
                {
                    if(IndexStatus==Z1Up_Z2Down)                                //jou 20171102 (Steven) : fix index position error
                        Task=110;
                    else
                        Task=109;
                }
                else
                {
                    if(IndexStatus==Z1Down_Z2Up &&
                       REAL_TIME_CCD==true &&                                   //----- by dell ccd realtime-------------
                       !COM2->bCCDDummyRum &&
                       TestIF_File.bUseSocketFloat==false)                      //Frank QQ
                    {
                        if(bRealCCDSendArm)
                        {
                            #if 0 // GATE F-G1 -- golden :6166 COM2->SendCommToVision(COM2->rtArmIndex2, true) -- same gap and same reasoning as :6139.
                            //        MIRROR: aTester_Rear.cpp:8564 (GATE K1G1).  ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800
                            COM2->SendCommToVision(COM2->rtArmIndex2, true);
                            #endif // GATE F-G1
                            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                               TestIF_File.bArm1PickPlaceArm2Test==true     &&  //Steven 20150129 : Z1吸料後, RTC確認socket內無料
                               TestIF_File.bRTC20CheckFunction==true        &&
                               TestIF_File.bRTC20GiveWayCheck==false        )
                            {
                                #if 0 // GATE F-G1 -- golden :6172 COM2->SendCommToVision(COM2->rtCHECKNULL, true) -- same gap as :6139.  Outbound
                                //        "check socket empty" request only.  MIRROR: aTester_Rear.cpp:8538 (GATE K1G1).  ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800
                                COM2->SendCommToVision(COM2->rtCHECKNULL, true);
                                #endif // GATE F-G1
                            }
                            iWaitIndexArm1.SetMSAndOn(200);
                            bRealCCDSendArm=false;
                        }
                    }
                    bCheckShuttle1=false;
                    Task=110;
                }
            }
            goto TestYFront_ReStart;                                            //2008/10/20 lee
        case 109:
            #ifdef DEBUG_INDEX_UPH
            if(MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Safe, iIndexSpeed, iIndexAcc))
            #else
            if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed, 0))
            #endif
            {
                if(IndexStatus==Z1Down_Z2Up &&
                   REAL_TIME_CCD==true &&                                       //----- by dell ccd realtime-------------
                   !COM2->bCCDDummyRum &&
                   TestIF_File.bUseSocketFloat==false)                          //Frank QQ
                {
                    if(bRealCCDSendArm)
                    {
                        #if 0 // GATE F-G1 -- golden :6197 COM2->SendCommToVision(COM2->rtArmIndex2, true) -- same gap as :6139.
                        //        MIRROR: aTester_Rear.cpp:8564 (GATE K1G1).  ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800
                        COM2->SendCommToVision(COM2->rtArmIndex2, true);
                        #endif // GATE F-G1
                        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                           TestIF_File.bArm1PickPlaceArm2Test==true     &&
                           TestIF_File.bRTC20CheckFunction==true)
                        {
                            #if 0 // GATE F-G1 -- golden :6202 COM2->SendCommToVision(COM2->rtCHECKIC, true) -- same gap as :6139.  Outbound
                            //        "check socket has IC" request only.  MIRROR: aTester_Rear.cpp:8572 (GATE K1G1).  ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800
                            COM2->SendCommToVision(COM2->rtCHECKIC, true);
                            #endif // GATE F-G1
                        }
                        iWaitIndexArm1.SetMSAndOn(200);
                        bRealCCDSendArm=false;
                    }
                }
                //----------------------------------------
                bCheckShuttle1=false;
                Task=110;
                if(Prod.bIndexUpSpeed)
                    DoTestYFrontDelay.SetSec(20);                               //kevin 20190913 add
            }

            if(Task!=110)                                                       //Steven 20180813 : add index arm speed
                break;
        case 110:                                                               //kevin 20170524 (wei) two speed up
            QueueTaskList[19].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化
            CheckSocketSensor(0, "DoTestYRear 110", true);                      //Z1上升時, 檢查socket sensor
            iAZ1=MOT[MTestY1].Gali_ReadPos();
            iAZ2=abs(Prod.TestY1_Middle-iAZ1);                                  //kevin 2019404 add 測區位置 > 5有問題
            iAZ3=abs(Prod.TestY1_Front-iAZ1);

            if(Prod.bIndexUpSpeed)
            {
                if(iAZ2<=50)                                                    //kevin 20190404
                {
                    if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+300, MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed/100))
                    {
                        if(InputShuttleFrontHasIC() && InShtInLF(0))            //KaiChen 20171225 (Steven)：Add Speed Display
                            IndexAddSpeedDisplay();
                        else
                            IndexSubSpeedDisplay();
                        Task=115;
                    }
                }
                else if(iAZ3<=50)                                               //kevin 20190404
                {
                    if(InputShuttleFrontHasIC() && InShtInLF(0))                //KaiChen 20171225 (Steven)：Add Speed Display
                        IndexAddSpeedDisplay();
                    else
                        IndexSubSpeedDisplay();
                    Task=115;
                }
            }
            else
            {
                if(InputShuttleFrontHasIC() && InShtInLF(0))                    //KaiChen 20171225 (Steven)：Add Speed Display
                    IndexAddSpeedDisplay();
                else
                    IndexSubSpeedDisplay();
                Task=115;
            }
            bIndexFinish=false;

            if(IniConfig.bD38IndexPutICToShtNoWaitMotion &&                     //Steven 20181228 : Add Index Action
               IniConfig.bIndexArm2SupplyLight==false    &&
               TestIF_File.bForEgisTecTest==false        &&
               TestIF_File.bArm1PickPlaceArm2Test==false)
            {
                bZ1UpAndPlaceZ2Down=CheckPlaceOutShuttle(0);                    //ChungHung 20171116 modify for Index Action
            }

            if(Task!=115)                                                       //Steven 20180813 : add index arm speed
            {
                break;
            }
            else
            {
                if(CosFunction.bSortingBy2DList==true &&
                   LastSet.iTester==_2D_SORT &&
                   TestIF_File.bSortingBy2DIDList==true)                        //Frank 20221122 : 2DID sorting for ATK
                {
                }
                else
                {
                    if((iAZ2<=5 || iAZ2>5) && iAZ1!=Prod.TestY1_Middle)
                    {
                        sBufferT.sprintf("Arm 1 Prod.TestY1_Middle:%d    ReadPos():%d ",Prod.TestY1_Middle ,iAZ1);
                        #if 0 // GATE F-G4 -- golden :6280 SaveFile(asHandlePath, sBufferT).  AI(W7C-DoTestYFront-c2) 20260810
                        //        WHY FAITHFUL: log-only.  The sBufferT.sprintf above it stays ACTIVE, so the message is still
                        //        formatted; only the append to D:\HT9045_Log\Handlelog is skipped, and nothing reads that file back.
                        //        REAL-MACHINE DIFFERENCE: the index-Y command-vs-encoder mismatch line is not written to the
                        //        handler log, so a drifting Y teach point loses its paper trail.  ABSENCE PROOF: asHandlePath IS defined (common.cpp:185) and declared (common.h:116), but common.h is UNREACHABLE from this TU: the only include path to it is aArmHeader.h:94, and aArmHeader.h:23 opens `#if 0 // TODO(W6.x/W7)` that closes at :98 -- verified with grep -n "^#if\|^#endif" aArmHeader.h -> 23:#if 0 ... 98:#endif. SaveFile itself IS ported (csystem.h:259). RUN AT 2026-08-10T23:43:46+0800
                        //        MIRROR: aTester_Rear.cpp:8651 (GATE K1G13); the premise (aArmHeader.h god-header still #if 0)
                        //        holds identically for this TU -- aTester_Front.cpp:108 includes the same aArmHeader.h.
                        SaveFile(asHandlePath, sBufferT);                       //kevin 20190411 add log 位置誤差
                        #endif // GATE F-G4
                    }
                    else if((iAZ3<=5 || iAZ3>5) && (iAZ1!=Prod.TestY1_Front))
                    {
                        sBufferT.sprintf("Arm 1 Prod.TestY1_Front:%d    ReadPos():%d ",Prod.TestY1_Front ,iAZ1);
                        #if 0 // GATE F-G4 -- golden :6285 SaveFile(asHandlePath, sBufferT) -- same gap as :6280.  ABSENCE PROOF: asHandlePath IS defined (common.cpp:185) and declared (common.h:116), but common.h is UNREACHABLE from this TU: the only include path to it is aArmHeader.h:94, and aArmHeader.h:23 opens `#if 0 // TODO(W6.x/W7)` that closes at :98 -- verified with grep -n "^#if\|^#endif" aArmHeader.h -> 23:#if 0 ... 98:#endif. SaveFile itself IS ported (csystem.h:259). RUN AT 2026-08-10T23:43:46+0800
                        SaveFile(asHandlePath, sBufferT);                       //kevin 20190411 add log 位置誤差
                        #endif // GATE F-G4
                    }
                }
            }
        case 115:
            // AI(W7C-DoTestYFront-c2) 20260810 -- NOT A GATE (TRAP 3 answered: this must NOT be gated).
            // ShowMainScreenPresure IS real and ACTIVE in the port: body cinitial.cpp:18785, declaration cinitial.h:250, and
            // aTester_Rear.cpp calls it UNGATED at :4466/:4476/:4512.  The ONLY problem is that this TU does not include
            // cinitial.h (proved: rg -n 'include "cinitial.h"' over aTester_Front.cpp + all 14 headers it includes -> ZERO
            // hits, run 2026-08-10T23:54:19+0800) and a mid-function chunk may not add a file-scope include.  So the call
            // stays ACTIVE behind a block-scope extern declaration at switch-body scope, which covers all three sites in
            // this chunk (golden :6360, :6380, :6851).  Legal C++, and a harmless redeclaration if the integrator also lands
            // sibling part 02237_DoFTestSuckTestIC.txt:426, which emits the same declaration at FILE scope; sibling part
            // 07036_DoTestYFront_c3.txt:262 reached the same conclusion for the same function.  NOTE FOR THE INTEGRATOR:
            // aTester_Rear.cpp gates the identical calls (TODO(G01)/(G02)/(G11)/(G17), :8757/:8772/:9277/:9744) on the FALSE
            // premise "NO body anywhere in the port tree" -- those four gates are expired and should be retired too.
            void ShowMainScreenPresure(int index);                              // golden cinitial.h:39 -> port cinitial.h:250
            QueueTaskList[19].CheckTaskChange();                                                                        //Steven 20200821 : 使用Goto也要記錄Task變化
            if(bGetTime1)
            {
                bGetTime1=false;
                bGetTime2=true;
//                ShowIndexTime(3);                                             //Steven 20140619 : 測試
            }

            str="check socket sensor : ";                                                                               //kevin 20130504 socket sensor
            if(USE_IO_CHANGE_TOQUE==true)                                                                               //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(bCheckShuttle1==false)
            {
                if(CheckShuttlePos())
                {
                    MOT[MInShuttle1].fCanMoveM=false;
                    MOT[MInShuttle2].fCanMoveM=false;
                }
                else
                {
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    return false;
                }

                if(CUSTOMER_CODE==CC_ASE_KaohSiung_K12 &&
                   (LastSet.iTemperature==Tempture_Hot) && bFirstZ1UPZ2Down)                                            //kevin 20131112 加熱模式預先動作避免第一顆溫度過高
                {
                    bFirstZ1UPZ2Down=false;                                                                             //kevin 20131112 加熱時z1在shuttle 1上面 z2在下
                }
                else if(MOT[MTestZ2].Gali_ReadPos()!=Prod.TestZ2_Safe)
                {
                    if(IndexStatus!=Z1Up_Z2Down)                                                                        //jou 20171102 (Steven) : fix index position error
                    {
                        ShowIndexTime(-2);                                                                              //Steven 20200715 : 重新計算Cycle Time
                        Task=108;
                        break;
                    }
                }
                bCheckShuttle1=true;
            }

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                                                                    //Frank 20221122 : 2DID sorting for ATK
            {
                if(IndexStatus==Z1_Z2_Normal)
                {
                    #ifdef AMD_Version
                    sprintf(HHandler2Gpib.UseSiteMapData, aSendSiteMapping.c_str());                                    //Ifor 20201030 add:送Site Mapping 資料給GPIB
                    #else
                    if(TestIF.iGpibMode==InterfaceType_Delta_Castle)                                                    //Steven 20260428 : Delta Castle 強制送SiteMap (取代AMD_Version compile flag)
                    {
                        memset(HHandler2Gpib.UseSiteMapData, '\0', sizeof(HHandler2Gpib.UseSiteMapData));
                        sprintf(HHandler2Gpib.UseSiteMapData, aSendSiteMapping.c_str());
                    }
                    #endif
                    bGetTime1=true;
                    bGetTime2=true;

                    bIndexArm2PickupErrStop=false;
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    bCheckShuttle1=false;
                    bOneTimeFlag=true;
                    ShowIndexTime(-2);
                    ShowMainScreenPresure(1);                                                                           //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                    Task=200;
                }
                else
                {
                    lPos[0]=IndexStatus;                                                                                //kevin 20150915
                    RecordIndexPositionError("DoTestYFront()Z1_Z2_Normal", true, false, false, false, &lPos[0]);        //kevin 20150915 record
                    ShowIndexMotorError(AnsiString("DoTestYFront115"));
                }
            }
            else
            {
                if(IndexStatus==Z1Up_Z2Down)                                                                            //jou 2011-04-19 start : Task直接跳到200，一些參數沒有初始化
                {
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    bCheckShuttle1=false;
                    bOneTimeFlag=true;
                    if(USE_ReadIndex_TOQUE==false &&
                       TestIF_File.bEnableReadAndCheckTorque)                                                           //kevin 20210804
                        ShowMainScreenPresure(1);                                                                       //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                    ShowIndexTime(-2);                                                                                  //Steven 20200715 : 重新計算Cycle Time
                    Task=200;
                    goto TestYFront_ReStart;                                                                            //2008/10/20 lee
                }
                else if(IndexStatus==Z1_Z2_Normal)
                {
                    if(TestIF_File.bUseSocketFloat)
                    {
                        ShowIndexTime(-2);
                        Task=116;
                        break;
                    }
                    else
                    {
                        lPos[0]=IndexStatus;                                                                            //kevin 20150915
                        RecordIndexPositionError("DoTestYFront()Z1_Z2_Normal", true, false, false, false, &lPos[0]);    //kevin 20150915 record
                        ShowIndexMotorError(AnsiString("DoTestYFront115"));
                        return false;
                    }
                }
                else
                {
                    if(IniConfig.bVTESTFunction==true &&                                                                //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
                       IniConfig.bGetRcsCheckingResult==true)
                    {
                        #if 0 // GATE F-G3 -- golden :6406-6410 fMesSystem->asGetRcsCheckingResult (golden Mes/fVATMesFileSys.h:204).  AI(W7C-DoTestYFront-c2) 20260810
                        //        WHY FAITHFUL: fMesSystem has no declaration anywhere in the port, and the gated arm is unreachable
                        //        offline regardless -- its two enclosing guards (golden :6403-6404, kept ACTIVE above) require
                        //        IniConfig.bVTESTFunction && IniConfig.bGetRcsCheckingResult, both false in an offline build.
                        //        REAL-MACHINE DIFFERENCE: on a VTEST line the MES "RCS checking result" can no longer send the index
                        //        arm back to case 50; a not-yet-PASS lot would be allowed to press.  SAFETY-ADJACENT: this is an
                        //        inbound MES interlock, so it must be restored with the MES bridge, not quietly retired.  ABSENCE PROOF: rg -n "fMesSystem" --glob "*.h" . (excluding pure comment lines) -> ZERO declarations. RUN AT 2026-08-10T23:43:46+0800
                        //        MIRROR: aTester_Rear.cpp:8804 (TODO(G03)) -- identical premise, still true on the Front side.
                        if(LastSet.iRunStartMode!=rsmAutoSiteMap && fMesSystem->asGetRcsCheckingResult!="PASS")
                        {
                            Task=50;
                            return false;
                        }
                        #endif // GATE F-G3
                    }

                    if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)                                                //Ifor 20210815 add:非正常模式掉料檢測
                    {
                        bSpecialCheck=true;
                    }
                    else if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                            TestIF_File.bArm1PickPlaceArm2Test==true)                                                   //Steven 20150129 : 需要確認Arm1有沒有掉料
                    {
                        bSpecialCheck=true;
                    }
                    else
                    {
                        bSpecialCheck=false;
                    }
                                                                                //jou 2011-08-16 +100 -> +500 drop mode容易造成誤判
                    iIndexZPos=MOT[MTestZ1].Gali_ReadPos();                                                             //jou 2012-01-17 +500 -> +750 疑似有誤判的情況發生，一直來來回回Hang up (備注 : 750條 = 7.5mm )
                    if((iIndexZPos>(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+750)) ||
                       bSpecialCheck==true)                                                                             //2008/10/20 lee //Ifor 20210716 add:避免關ARM不偵測掉料
                    {
                        flag=false;
                        if(LastSet.iRealDummy==REALLY)
                        {
                            if(DeviceForm_File.dDropByPassDetect!=0 &&
                               (iIndexZPos>(Prod.TestZ1_Test+DeviceForm_File.dDropByPassDetect*100)))
                            {
                            }
                            else
                            {
                                for(int i=0; i<FTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<FTestSuck.iShtCol; j++)
                                    {
                                        if(FTestSuck.Suck[i][j].Enable       &&                                         //Steven 20110725 : 不再使用IsSuckICFallDown
                                           FTestSuck.Suck[i][j].SenUsing!="" &&
                                           FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                           FTestSuck.Item[i][j]!=NULL_IC)
                                        {
                                            if(FTestSuck.Suck[i][j].W906_GetStatusHeld()==false)  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都要有真空，掉了要持續 100 ms 才算掉料
                                            {
                                                FTestSuck.Suck[i][j].Normal();                                          //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                                flag=true;
                                                bRecIndexDropAlarm1=true;                                               //jou 2012-01-17 紀錄index Drop alarm
                                                bRearHeadICFallDown=true;                                               //kevin 20131120 發ALARM 開CHAMBO門 按Z1
                                                //==> Eastsun 20260511 F006 整合: Ifor 20220125 add:Index1 掉料當下 馬達位置
                                                if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true)    //Ifor 20220125 add:Index 掉料當下 馬達位置
                                                {
                                                    MOT[MTestZ1].Gali_Command("ST");
                                                    StopAllMotor();
                                                    iArm1_Y=MOT[MTestY1].Gali_ReadPos();
                                                    iArm1_Z=MOT[MTestZ1].Gali_ReadPos();
                                                    iArm2_Y=MOT[MTestY2].Gali_ReadPos();
                                                    iArm2_Z=MOT[MTestZ2].Gali_ReadPos();

                                                    StrPos.sprintf("Index1 %d %d Drop :ARM1_Y_%d_Z_%d_ARM2_Y_%d_Z_%d",i ,j ,iArm1_Y, iArm1_Z, iArm2_Y, iArm2_Z);
                                                    RecordProcess(StrPos);
                                                }
                                                //<== Eastsun 20260511 F006 整合
                                                }
                                        }

                                        if(BTestSuck.Suck[i][j].Enable       &&                                         //ChungHung 20140715 解開 沒問題  //ChungHung 20140704 delete 下面 show alarm 時 此條件位判斷 會 hangup
                                           BTestSuck.Suck[i][j].SenUsing!="" &&
                                           BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                           BTestSuck.Item[i][j]!=NULL_IC)                                               //jou 2012-04-24 增加IC掉落檢查
                                        {
                                            if(BTestSuck.Suck[i][j].GetStatus()==false)
                                            {
                                                BTestSuck.Suck[i][j].Normal();                                          //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                                flag=true;
                                                bRecIndexDropAlarm2=true;                                               //jou 2012-01-17 紀錄index Drop alarm
                                                bRearHeadICFallDown=true;                                               //kevin 20131120 發ALARM 開CHAMBO門 按Z1
                                                //==> Eastsun 20260511 F006 整合: Ifor 20220125 add:Index2 掉料當下 馬達位置
                                                if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true)    //Ifor 20220125 add:Index 掉料當下 馬達位置
                                                {
                                                    MOT[MTestZ2].Gali_Command("ST");
                                                    StopAllMotor();
                                                    iArm1_Y=MOT[MTestY1].Gali_ReadPos();
                                                    iArm1_Z=MOT[MTestZ1].Gali_ReadPos();
                                                    iArm2_Y=MOT[MTestY2].Gali_ReadPos();
                                                    iArm2_Z=MOT[MTestZ2].Gali_ReadPos();

                                                    StrPos.sprintf("Index2 %d %d Drop :ARM1_Y_%d_Z_%d_ARM2_Y_%d_Z_%d",i ,j ,iArm1_Y, iArm1_Z, iArm2_Y, iArm2_Z);
                                                    RecordProcess(StrPos);
                                                }
                                                //<== Eastsun 20260511 F006 整合
                                                }
                                        }
                                    }
                                }
                            }
                        }

                        for(int i=0; i<Gali_MaxAxis; i++)
                        {
                            if(USE_INDEX_ARM_AXES==IndexArm_3_Axis && (MTestY1+i)==MTestY2)                             //JimmyChiu 20220708 : add Index Arm Axis
                            {
                                continue;
                            }

                            MOT[MTestY1+i].Gali_ScanMotStatusTIMO();
                            if(MOT[MTestY1+i].Led[iAlarmLed] ||
                               MOT[MTestY1+i].Led[iServoalarmLed] ||
                               MOT[MTestY1+i].Led[iServoOn]==false)
                            {
                                iHome=1;
                                IndexMotorBreakerOFF();
                                MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:115"));
                                #if 0 // GATE F-G2 -- golden :6519-6522 ScanBtnThd->Stop() (golden :71 `extern ScanBtn *ScanBtnThd;`).  AI(W7C-DoTestYFront-c2) 20260810
                                //        WHY FAITHFUL: the RTC button-scan thread object does not exist offline, so there is nothing to stop;
                                //        the WAR0349 alarm itself (ShowErrorMessage below) stays ACTIVE, as does iHome=1 and
                                //        IndexMotorBreakerOFF().  REAL-MACHINE DIFFERENCE: on an index-Y servo/alarm fault the RTC scan thread
                                //        keeps running while the machine is already stopping.  ABSENCE PROOF: rg -n "ScanBtnThd" --glob "*.h" .  -> only comment text; rg -n "ScanBtnThd" --glob "*.cpp" . -> atester.cpp:10822/:10997 which sit inside that file's #if 0 at atester.cpp:10664, so NO live declaration of the object exists. NOTE the CLASS does exist (ScanBtnThread.h:63 class ScanBtn, with Stop/Start/GetArmState/SetArmState) -- it is only the golden global `extern ScanBtn *ScanBtnThd;` (golden :71) that has no home, and per TRAP 4 nobody may new it at file scope. RUN AT 2026-08-10T23:43:46+0800
                                //        MIRROR: aTester_Rear.cpp:8920 (TODO(G04)) -- same premise, still true.
                                if(REAL_TIME_CCD==true)
                                {
                                    ScanBtnThd->Stop();
                                }
                                #endif // GATE F-G2
                                Task=50;
                                str.sprintf("Motor:%s, Alarm:%d, ServoAlarm:%d, Servo On:%d", MOT[MTestY1+i].Alias, MOT[MTestY1+i].Led[iAlarmLed], MOT[MTestY1+i].Led[iServoalarmLed], MOT[MTestY1+i].Led[iServoOn]);

                                ShowErrorMessage("WAR0349", 0, MMSystem, 0, str);                                       //JerryYang 20240111 : 改成跳alarm code
                                #if 0 // GATE F-G6 -- golden :6527 fHome->GaliMotorServoOff("DoTestYFront") (golden uhome.h:77).  AI(W7C-DoTestYFront-c2) 20260810
                                //        WHY IT SHOULD STAY GATED (TRAP 3): TfHome DOES exist (forms/fHome.h) -- this is NOT plain absence.
                                //        forms/fHome.h:30-34 says the facade deliberately omits GaliMotorServoOff so that nobody "retires"
                                //        the seam without translating uhome.cpp:4891; csystem.cpp:6417 gates the identical call
                                //        (W7C2_FHOME_SERVOOFF).  Declaring it here to make one call compile would swap an honest no-op for a
                                //        silent one.  SAFETY-RELEVANT REAL-MACHINE DIFFERENCE: on a WAR0349 index-motor servo/alarm fault
                                //        golden drops the Galil servo WHILE catching the Z brake; offline that servo-off does not happen, so
                                //        the Z axis is not deliberately parked.  ABSENCE PROOF: rg -n "GaliMotorServoOff" --glob "*.h" . -> hits are COMMENT text only (forms/fHome.h:24 and :30); forms/fHome.h:30-34 states the facade DELIBERATELY omits it. RUN AT 2026-08-10T23:48:47+0800
                                //        MIRROR: aTester_Rear.cpp:8929 (TODO(G22)) -- same reason, verbatim.
                                fHome->GaliMotorServoOff("DoTestYFront");                                               //Steven 20230712 : 修正SwServoOn.Off時, 要抓住Z煞車
                                #endif // GATE F-G6
                                return false;
                            }
                        }

                        if(flag)
                        {
                            MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:115"));
                            #if 0 // GATE F-G2 -- golden :6535-6538 ScanBtnThd->Stop() -- same gap as :6519.  The Gali_Command("ST") above and the
                            //        RecordProcess/Task=50 below stay ACTIVE, so the drop-alarm path itself is unchanged.  ABSENCE PROOF: rg -n "ScanBtnThd" --glob "*.h" .  -> only comment text; rg -n "ScanBtnThd" --glob "*.cpp" . -> atester.cpp:10822/:10997 which sit inside that file's #if 0 at atester.cpp:10664, so NO live declaration of the object exists. NOTE the CLASS does exist (ScanBtnThread.h:63 class ScanBtn, with Stop/Start/GetArmState/SetArmState) -- it is only the golden global `extern ScanBtn *ScanBtnThd;` (golden :71) that has no home, and per TRAP 4 nobody may new it at file scope. RUN AT 2026-08-10T23:43:46+0800
                            //        MIRROR: aTester_Rear.cpp:8940 (TODO(G05)).
                            if(REAL_TIME_CCD==true)
                            {
                                ScanBtnThd->Stop();
                            }
                            #endif // GATE F-G2
                            str1.sprintf("DoTestYFront 115 to 50: bRecIndexDropAlarm1=%d, bRecIndexDropAlarm2=%d", bRecIndexDropAlarm1?1:0, bRecIndexDropAlarm2?1:0);
                            RecordProcess(str1);
                            Task=50;
                            return false;
                        }
                        else
                        {
                            bRecIndexDropAlarm1=false;                                                                  //jou 2012-01-17 紀錄index Drop alarm
                            bRecIndexDropAlarm2=false;                                                                  //jou 2012-01-17 紀錄index Drop alarm
                        }
                    }

                    if(CheckSocketSensor(0, "DoTestYFront_115_"))                                                       //Z1UpZ2Down //Steven 20200615 : Socket Sensor整合成Function
                    {
                        Task=50;
                        return false;
                    }

                    if(IniConfig.bI26TestCloseSiteHaveBin &&
                       bTestBinDataError!=0)                                                                            //kevin 20150202 需強致將arm上 ic取出
                    {
                        Task=50;
                        RecordProcess("[I26] Test Close Site Have Bin Error!", __FUNC__);                               //Steven 20201201 : add event log for debug
                        return false;
                    }

                    if(bOneTimeFlag)
                    {
                        bOneTimeFlag=false;
                        if(TestIF_File.bUseSLKClamp &&
                           TestIF_File.iSeparabilityTest==1)                                                            //JerryYang 20160523 Z2分離高度要上升1500
                        {
                            iBackUpZ2DownPosition=Prod.TestZ2_Test+1500;
                        }
                        else
                        {
                            iBackUpZ2DownPosition=Prod.TestZ2_Test;
                        }
                        FTestSeparateSLK(true);                                                                         //JerryYang 20180518 (wei) : 分離流程初始化
                        BTestSeparateSLK(true);
                        iRealCCDSendArmCT=0;                                                                            //jou 2013-08-23 修正RTC 斷訊不會Alarm的錯誤.
                    }

                    #if 0 // GATE F-G1 -- golden :6582-6620 -- the WHOLE RTC/CCD half-view send-and-wait region.  AI(W7C-DoTestYFront-c2) 20260810
                    //        ABSENT SYMBOLS IN THIS REGION: COM2->SendCommToVision / COM2->rtArmIndex2 / COM2->OpenRTCComPortAgain
                    //        (see the COM2 proof below), ScanBtnThd->Start() (see GATE F-G2),
                    //        and fLotInfo->ALed2 (ABSENCE PROOF: rg -c "ALed2" FormsFacade.h forms/fLotInfo.h -> ZERO hits. RUN AT 2026-08-10T23:43:46+0800).  Sen[SnRealTimeCCDIndexArm] and iWaitIndexArm1 DO exist
                    //        (cmydef.h:931 / golden :5160) and are gated only because they sit inside this region.
                    //        BEHAVIOUR-NEUTRAL OFFLINE: the region head requires !COM2->bCCDDummyRum, and TCOM2Shim's ctor sets
                    //        bCCDDummyRum=true (atester_shims.cpp:362) -- golden itself never enters this region offline, so the
                    //        gate removes nothing that would have run.  REAL-MACHINE DIFFERENCE: the arm-clear handshake with the
                    //        RTC camera, its 15-retry WAR0335 com-port recovery, and the ALed2 lamp all stop happening.
                    //        ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800
                    //        MIRROR: aTester_Rear.cpp:8991 (TODO(G06)).  NOTE the Front/Rear asymmetry is preserved verbatim:
                    //        Front tests Sen[...].IsOff() and sets fLotInfo->ALed2->Value=FALSE where Rear uses IsOn()/true.
                    if(REAL_TIME_CCD==true &&                                                                           //----- by dell ccd realtime-------------
                       !COM2->bCCDDummyRum &&
                       TestIF_File.bUseSocketFloat==false)
                    {
                        if(bRealCCDSendArm)
                        {
                            COM2->SendCommToVision(COM2->rtArmIndex2, true);
                            iWaitIndexArm1.SetMSAndOn(200);
                            bRealCCDSendArm=false;
                        }

//                        if(TestIF_File.bRTC20GiveWayCheck==false)
                        {
                            if(Sen[SnRealTimeCCDIndexArm].IsOff())
                            {
                                if(iWaitIndexArm1.Off())
                                {
                                    bRealCCDSendArm=true;
                                    iRealCCDSendArmCT++;
                                    if(iRealCCDSendArmCT>15)                                                            //jou 2013-08-23 修正RTC 斷訊不會Alarm的錯誤.
                                    {
                                        iRealCCDSendArmCT=0;
                                        if(COM2->OpenRTCComPortAgain())                                                 //ChungHung 20140520 add 不要第一次TimeOut就秀錯誤訊息
                                            ShowErrorMessage("WAR0335", 0, MMIndex, 0, __FUNC__);
                                        else
                                            RecordProcess("WAR0335 auto retry com port.", __FUNC__);                    //Steven 20201201 : add event log for debug
                                        bSendRealCCDSendStart=true;                                                     //Sam 20250220 :　修正一直報警 RTC WAR0335 RTC Arm1 Error! 問題
                                        bSendRealCCDSendVerify=true;
                                        bRealCCDSendArm=true;
                                    }
                                    return false;
                                }
                                return false;
                            }
                        }
                        iRealCCDSendArmCT=0;                                                                            //jou 2013-08-23 修正RTC 斷訊不會Alarm的錯誤.
                        fLotInfo->ALed2->Value=false;                                                                   //Steven 20110916
                        ScanBtnThd->Start();
                    }
                    #endif // GATE F-G1

                    if(bGetTime2)
                    {
                        bGetTime2=false;
    //                    ShowIndexTime(4);                                     //Steven 20140619 : 測試
                    }

                    #if 0 // GATE F-G1 -- golden :6628-6704 -- the WHOLE RTC arm-state result region (INCLUDING both #ifdef RTCErrorType arms,
                    //        which ARE live in this tree: MachineType.h:60 defines RTCErrorType).  AI(W7C-DoTestYFront-c2) 20260810
                    //        ABSENT: ScanBtnThd->GetArmState()/SetArmState() (GATE F-G2 proof), COM2->bRealTimeCom_ReceiveOK[] and
                    //        COM2->rtAlarHasIC / rtALARMArm1NG / rtALARMArm2NG / rtAlarGrabTimeOut / rtRelease / rtErrorType and
                    //        COM2->SendCommToVision.  hBRTCTimeOutDelay (golden :5162) and tRTCErrorTimeOut (cmydef.h:5750) DO
                    //        exist and are gated only because they sit inside this region.
                    //        BEHAVIOUR-NEUTRAL OFFLINE: region head requires COM2->bCCDDummyRum==false and the shim ctor sets it
                    //        true (atester_shims.cpp:362).  REAL-MACHINE DIFFERENCE: RTC "socket has IC" / "arm NG" / grab-timeout
                    //        verdicts can no longer route this SM to Task=50, i.e. the vision interlock is not enforced.
                    //        SAFETY-ADJACENT: inbound vision interlock -- restore together with the RTC bridge.  ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800
                    //        MIRROR: aTester_Rear.cpp:9037 (TODO(G07)).  NOTE golden's own asymmetry is preserved: the Front
                    //        message string is "RTC Arm 2 half view check timeout error" and Front does NOT set
                    //        bRearHeadICFallDown=true in the rtAlarHasIC arm, where Rear does.
                    if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)                                                      //----- by dell ccd realtime-------------
                    {
                        if(ScanBtnThd->GetArmState())
                        {
                            #ifdef RTCErrorType
                            if(bCheckThirdPos)
                            {
                                if(iRTCErrorSend==false)        //wei 20221222 RTC ARM Error
                                {
                                    COM2->SendCommToVision(COM2->rtErrorType, false);
                                    iRTCErrorSend=true;
                                    tRTCErrorTimeOut.SetSecAndOn(20);
                                }
                            }
                            #endif

                            if(bOnce==false)                                                                            //JerryYang 20220923 : add RTC timeout
                            {
                                bOnce=true;
                                hBRTCTimeOutDelay.SetSecAndOn(15.0);
                            }

                            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
                            {
                                MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:115"));
                                ScanBtnThd->SetArmState(false);
                                iRTCErrorSend=false;                            //JerryYang 20260306 : fix沒有清除flag
                                RecordProcess("RTC alarm has IC", __FUNC__);                                            //Steven 20201201 : add event log for debug
                                Task=50;
                            }
                            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm1NG])      //wei 20221222 RTC ARM Error
                            {
                                MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:115"));
                                ScanBtnThd->SetArmState(false);
                                iRTCErrorSend=false;
                                RecordProcess("RTC alarm Arm 1 NG", __FUNC__);    //Steven 20201201 : add event log for debug
                                Task=50;
                            }
                            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm2NG])      //wei 20221222 RTC ARM Error
                            {
                                MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:115"));
                                ScanBtnThd->SetArmState(false);
                                iRTCErrorSend=false;
                                RecordProcess("RTC alarm Arm 2 NG", __FUNC__);    //Steven 20201201 : add event log for debug
                                Task=50;
                            }
                            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarGrabTimeOut])
                            {
                                COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarGrabTimeOut]=false;
                                ShowErrorMessage("WAR0341", 0, MMIndex, 0, __FUNC__);                                   //RTC Grab TimeOut Error!
                                COM2->SendCommToVision(COM2->rtRelease, false);
                                MySleep(100);

                                ScanBtnThd->SetArmState(false);
                            }
                            else
                            {
                                if(hBRTCTimeOutDelay.Off())                                                             //JerryYang 20220923 : add RTC timeout
                                {
                                    ShowMyMessage("RTC Arm 2 half view check timeout error");
                                    bOnce=false;
                                }
                            }

                            #ifdef RTCErrorType
                            if(bCheckThirdPos)
                            {
                                if(tRTCErrorTimeOut.Off())      //wei 20221222 RTC ARM Error
                                {
                                    iRTCErrorSend=false;
                                    ShowErrorMessage("WAR0356", 0, MMIndex, (bRTCArm1HalfViewError || bRTCArm2HalfViewError), __FUNC__);
                                }
                            }
                            #endif
                            return false;
                        }
                    }
                    #endif // GATE F-G1
                    bOnce=false;                                                                                        //JerryYang 20220923 : add
                    //---------------------------------------
                    if(IniConfig.bIndexPickupErrStop==false)
                    {
                        bIndexArm1PickupErrStop=false;
                        bIndexArm2PickupErrStop=false;
                    }

                    if(DeviceForm.ContactMode==TMove ||
                       DeviceForm.ContactMode==TMoveDrop ||                                                             //jou 2012-02-03 新增T Move Drop
                       DeviceForm.ContactMode==TMoveDropSlowContact ||
                       DeviceForm.ContactMode==TMoveSlowContact)                                                        //Steven 20160130 : TMove Soft contact
                    {
                        TMode=true;
                    }
                    else
                    {
                        if(IniConfig.bIndexPickupErrStop==true &&                                                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                           bIndexArm2PickupErrStop==true)
                        {
                            TMode=true;
                        }
                        else
                        {
                            TMode=false;
                        }
                    }

                    if(DeviceForm_File.ContactMode==DirectContactSoftEP ||                                              //kevin 20130608 soft contact 到測區上面2mm 開通ep 充氣
                       DeviceForm_File.ContactMode==DropContactSoftEP)                                                  //JerryYang 20151202 add DropContactSoftEP
                    {
                        if(MOT[MTestZ2].Gali_ReadPos()<(Prod.TestZ2_Test+1500))                                         //JerryYang 20231205 : SoftEP高度由+1000改為+1500避免撞到guide pin
                        {
//                            bContSoftEpSwitch(1, false);                        //kevin 20220616 CHANGE ARM2 浮動頭氣
                            EPSwitchOnOff(eEPSwArm1);
                            if(DeviceForm.fAireForce==0)
                                DeviceForm.fAireForce=DeviceForm.dPress;                                                //kevin 20220215
                            ADAM_WriteVoltage(DeviceForm.fAireForce);                                                   //JerryYang 20210119 修正誤用die force造成錯誤
                        }
                    }

                    ShowIndexTime(-1);                                                                                  //Steven 20200715 : 重新計算Cycle Time

                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                  //Ifor 20190818 : add Arm1PickPlace Arm2Test模式下測試完Arm1 取料後 Arm2 不下去
                        TestIF_File.bArm1PickPlaceArm2Test==true    &&
                        bCheckGiveWay==true                         )
                    {
                        if(Prod.TestY2_Middle!=Prod.TestY2_Rear &&
                           Prod.TestZ2_Test!=Prod.TestZ2_Safe)
                        {
                            iBackupTestY2_Middle    =Prod.TestY2_Middle;
                            iBackupTestZ2_Test      =Prod.TestZ2_Test;
                            iBackupTestZ2_Drop      =Prod.TestZ2_Drop_Offset;
                            Prod.TestY2_Middle      =Prod.TestY2_Rear;
                            Prod.TestZ2_Test        =Prod.TestZ2_Safe;
                            Prod.TestZ2_Drop_Offset =0;
                            iBackUpZ2DownPosition   =Prod.TestZ2_Test;
                        }
                    }

                    if(bintered1==true)                                                                                 //Isaac 20200922 : 紀錄indexArmY encoder值和command值
                    {
                        bintered1=false;
                        RecordIndexPosition(1, 1);                                                                      //Isaac 20200922 : 紀錄indexArmY encoder值和command值Arm1/Socket
                        EncoderTeachingMaxMinCount(1);                                                                  //Isaac 20201012 : 每次完成動作，比較紀錄Encoder和Teaching點的差值
                    }

                    #ifdef INDEX_PROTECT_TMOVE
                    if(bOverRangeDoTMode==true && bTriger4Indexhome==false)                                             //Isaac 20201012 : index Y超過範圍，做一次Tmode
                    {
                        bTriger4Indexhome=true;
                        bOverRange4Indexhome=true;
                        TrigerIndexAxisHome();                                                                          //Isaac 20201012 : index Y超過範圍，做一次Tmode，初始化，開始自動校正
                        return false;
                    }
                    #endif

                    #ifndef SOFT_SIMULTE
                    if(TorqueUseHPComCard &&
                       TestIF_File.bEnableReadAndCheckTorque)                                                           //KenHsieh 20220317 : 確認有Get value後Test Arm才能上升
                    {
                        #if 0 // GATE F-G1 -- golden :6786-6787 COM2->bGetValue (golden rs232.h) -- not a TCOM2Shim member.  AI(W7C-DoTestYFront-c2) 20260810
                        //        WHY FAITHFUL: gating these two lines leaves the TorqueUseHPComCard arm EMPTY, which is unreachable
                        //        offline anyway because TorqueUseHPComCard=false (cmydef.cpp:3469) -- so the #else-side block at
                        //        golden :6791-6793 performs the Z1UpZ2Down call exactly as golden does.  REAL-MACHINE DIFFERENCE: on a
                        //        HonPrec torque comm-card machine the arm would rise without waiting for the torque value to arrive.
                        //        ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800  MIRROR: aTester_Rear.cpp:9192 (TODO(G08)).
                        if(COM2->bGetValue)
                            bIndexFinish=MOT[MTestY1].Z1UpZ2Down(MOT[MTestZ2].GailSpeed, TMode, bIndexArm2PickupErrStop);
                        #endif // GATE F-G1
                    }
                    else
                    #endif
                    {
                        bIndexFinish=MOT[MTestY1].Z1UpZ2Down(MOT[MTestZ2].GailSpeed, TMode, bIndexArm2PickupErrStop);
                    }

                    //==> Eastsun 20260511 F006 整合: Ifor 20220317 add: KYEC 要求 溫度還沒到不執行
                    if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false && LastSet.iTemperature==Tempture_Hot && fHeaterOK==false)//Ifor 20220317 add: KYEC 要求 溫度還沒到不執行
                    {
                        bIndexFinish=false;
                    }
                    //<== Eastsun 20260511 F006 整合

                    if(bIndexFinish)                                                                                    //ChungHung 20150528 Mark Z1 up Z2 Down
                    {
                        if(DeviceForm_File.ContactMode==DirectContactSoftEP ||
                           DeviceForm_File.ContactMode==DropContactSoftEP)                                              //kevin 20220616 add EP //JerryYang 20151202 add DropContactSoftEP
                        {
//                            bContSoftEpSwitch(1, true);                         //kevin 20220616 add //ARM2 先浮動頭充氣
                            EPSwitchOnOff(eEPSwBoth);
                            if(DeviceForm.fAireForce==0)
                                DeviceForm.fAireForce=DeviceForm.dPress;                                                //kevin 20220215
                            ADAM_WriteVoltage(DeviceForm.fAireForce);                                                   //JerryYang 20210119 修正誤用die force造成錯誤
                        }
                        bIndex2Suck=false;                                                                              //kevin 20220105 Index 在下真空建立 pause 不能關閉
                        bintered1=true;

                        if(ATC_SYSTEM==eNewATCSystem &&                                                                 //Ifor 20190321 : add Test Mode & ATC Enables Site Send to GPIB
                           Temperature.bATCActiveCooling==true &&
                           TestIF_File.i2DIDFormat==eAMD)                                                               //JerryYang 20200422 2DID format選項改用下拉選單
                        {
                            for(int i=0; i<MAX_SOCKET_TOTAL; i++)
                            {
                                HHandler2Gpib.Site[i]=bATC_EnablesChannel[i];
                            }
                        }
                        HHandler2Gpib.iLotStatus=TestIF_File.iTestMode;

                        if(ATC_SYSTEM==eNewATCSystem &&
                           Temperature.bATCActiveCooling==true)                                                         //JerryYang 20220815 : send ATC which ARM
                        {
                            #if 0 // GATE F-G7 -- golden :6830 ATC_InterfaceForm->HandlerArm(1).  AI(W7C-DoTestYFront-c2) 20260810
                            //        WHY FAITHFUL: pure outbound "which arm is down" notify to ATC; no state this SM reads back.  The
                            //        enclosing guard also needs ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling, neither true
                            //        offline.  REAL-MACHINE DIFFERENCE: a New-ATC controller is not told arm 1 is the one in the socket.
                            //        ABSENCE PROOF: rg -n "HandlerArm" --glob "*.h" . -> only ATC/ATCSystem.h:383 (an int MEMBER of a different class). TATC_InterfaceFormShim (acarry_shims.h:109-115) exposes only iATC_MODE_TYPE. RUN AT 2026-08-10T23:43:46+0800  MIRROR: aTester_Rear.cpp:9239 (TODO(G09), arm 0).
                            ATC_InterfaceForm->HandlerArm(1);
                            #endif // GATE F-G7
                        }
                        iRTCErrorCount=0;                            //wei 20221222 RTC ARM Error
                        iWhichArmDown=2;                                                                                //JerryYang 20200316 add SVID 哪支arm下壓在測區
                        #if 0 // GATE F-G8 -- golden :6834 fContact->ATC_SwitchTjSignal(2, false) (golden cContact.h:603).  AI(W7C-DoTestYFront-c2) 20260810
                        //        WHY FAITHFUL: outbound ATC TJ (junction-temperature feedback select) signal only; nothing read back.
                        //        REAL-MACHINE DIFFERENCE: the ATC keeps the other arm's TJ thermal feedback selected while this arm is
                        //        the one testing, so ATC closed-loop control tracks the wrong socket.  ABSENCE PROOF: rg -n "ATC_SwitchTjSignal" --glob "*.h" --glob "*.cpp" . -> hits are ONLY aTester_Rear.cpp's own gate comments/gated bodies; the token has no declaration anywhere. cContact.cpp is untranslated. RUN AT 2026-08-10T23:43:46+0800
                        //        MIRROR: aTester_Rear.cpp:9245 (TODO(G10)) and aTester_Rear.cpp:10929 (GATE W7R3-G12).
                        fContact->ATC_SwitchTjSignal(2, false);                                                         //Ifor 20210622 add: ATC Switch TJ
                        #endif // GATE F-G8
                        if(DeviceForm.ContactMode==DropContact)
                        {
                            fObserver->AddTimeData(18, DropContactTimer1.LatchCycleTime()/1000.0);                      //JerryYang 20170425 (wei) 第一段時間, 測試完成到另一支arm下降到drop高度
                            DropContactTimer2.LatchCycleTime(true);
                        }

                        bGetTime1=true;
                        bGetTime2=true;

                        bIndexArm2PickupErrStop=false;
                        MOT[MInShuttle1].fCanMoveM=true;
                        MOT[MInShuttle2].fCanMoveM=true;
                        bCheckShuttle1=false;
                        bOneTimeFlag=true;

                        ShowIndexTime();
                        ShowMainScreenPresure(1);                                                                       //jou 2010-06-23 畫面Z1,Z2 encoder 顯示

                        Task=200;
                        #if 0 // GATE F-G1 -- golden :6854-6872 -- the RTC arm-finish notify block.  AI(W7C-DoTestYFront-c2) 20260810
                        //        ABSENT: COM2->SendCommToVision / COM2->rtArmFinish / COM2->bRealTimeCom_ReceiveOK[] (with rtFullTOK /
                        //        rtFullTNG), ScanBtnThd->Stop().  BEHAVIOUR-NEUTRAL OFFLINE: head requires COM2->bCCDDummyRum==false,
                        //        shim ctor sets it true (atester_shims.cpp:362).  NOTE THREE LIVE SYMBOLS GO DOWN WITH IT and that is
                        //        correct: bRTCArm1HalfViewError=false, DoGiveWayDelay.SetSecAndOn(10) and bNeedCheckRTCReport=true
                        //        (atester.h:52-53) all exist, but each is only consumed by RTC paths that are themselves gated here or
                        //        in F-G1 :6628-6704 / the DoFRTCGiveWayCheck engine.  REAL-MACHINE DIFFERENCE: the camera is never told
                        //        "arm finished", and the Arm1-pick/Arm2-test residue re-check (bNeedCheckRTCReport) never arms.
                        //        ABSENCE PROOF: cd D:/HT9045/HT9011UC_Cpp_V3.33.906.0 && rg -c "SendCommToVision|bRealTimeCom_ReceiveOK|OpenRTCComPortAgain|bGetValue|RTC_AlarmType" --glob "*.h" .  -> ZERO hits (TCOM2Shim, atester_shims.h:420-461, declares only bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck). RUN AT 2026-08-10T23:43:46+0800  MIRROR: aTester_Rear.cpp:9283 (TODO(G12)).
                        if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)
                        {
                            bRTCArm1HalfViewError=false;                                                                //Steven 20120206 : RTC重複錯誤
                            COM2->SendCommToVision(COM2->rtArmFinish, false);
                            ScanBtnThd->Stop();
                            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                                TestIF_File.bArm1PickPlaceArm2Test==true    &&
                                TestIF_File.bRTCICResidueCheck==true        &&
                                bCheckGiveWay==true                         &&
                                fFrontNeedDestroy==true                      )                                          //Ifor 20200521 Fix: Index Check 時不檢查
                            {
                                COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK]=false;
                                COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]=false;
                                COM2->SendCommToVision(COM2->rtCHECKNULL, true);
                                COM2->SendCommToVision(COM2->rtFullTOK, true);
                                DoGiveWayDelay.SetSecAndOn(10);
                                bNeedCheckRTCReport=true;
                            }
                        }
                        #endif // GATE F-G1

                        if(IniConfig.bTModeMotorFree==true)
                        {
                            if(TMode==true)                                                                             //20110923 Tmode 煞車
                            {
                                hTestZ2Delay.SetSecAndOn(0.1);
                                Task=130;
                            }
                        }

                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                             //JerryYang 20170523 (wei) ReadEPData會造成index cycle time增加
                        {
                            bReadEpTime=true;                                                                           //kevin 20170524 (wei) add ep read change time
                        }

                        if(ATC_SYSTEM==eATCHonPrecType &&
                           ATC_SYSTEM==eATCHonPrecType)                                                                 //Dell 20140509
                        {
                            #if 0 // GATE F-G9 -- golden :6891 ATCInterfaceForm->ATC_SYS.SetNowArm(1).  AI(W7C-DoTestYFront-c2) 20260810
                            //        WHY IT SHOULD BE GATED (an ARCHIVE boundary, not absence -- TRAP 1(c)): the FORM object IS linkable
                            //        (ATC/ATCInterface.cpp, target ht9045_sm) and ATCInterfaceForm IS declared (ATC/ATCInterface.h:525),
                            //        but every ATCSystem/ATC60System method it forwards to lives in ATC/ATCSystem.cpp, which
                            //        CMakeLists.txt:1332 places in ht9045_comms, and target_link_libraries(ht9045_sm ...) does NOT list
                            //        ht9045_comms -- aTester_Rear.cpp:9309-9323 records the measurement: 25 distinct undefined
                            //        ATCSystem::*/ATC60System::* symbols, build rc=2.  Taking that whole archive edge for two
                            //        "tell ATC which arm" calls is not this wave's call.  ALSO NOTE: ATC/ATCInterface.h is not in this
                            //        TU's include set at all, so the call could not even compile here today.
                            //        BEHAVIOUR: neutral offline -- both sites are already inside runtime ATC_SYSTEM guards and there is no
                            //        ATC controller on the wire.  REAL-MACHINE DIFFERENCE: a HonPrec-type ATC is not told arm 1 is now the
                            //        testing arm.  RETIRES WHEN: ht9045_sm gains the comms edge (or ATCSystem.cpp moves), AND
                            //        aTester_Front.cpp includes ATC/ATCInterface.h.  MIRROR: aTester_Rear.cpp:9324 (GATE W7b-I1).
                            //        ABSENCE/BOUNDARY PROOF: rg -n "ATCInterfaceForm" --glob "*.h" . -> ATC/ATCInterface.h:525 only; rg -n 'include "ATC/ATCInterface.h"' aTester_Front.cpp -> ZERO. RUN AT 2026-08-10T23:48:47+0800
                            ATCInterfaceForm->ATC_SYS.SetNowArm(1);                                                     //ATC
                            #endif // GATE F-G9
                        }

                        if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                                                    //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
                        {
                            #if 0 // GATE F-G9 -- golden :6896 ATCInterfaceForm->ATC_60_SYS.SetHandlerNowArm(1) -- same archive boundary as :6891.
                            //        MIRROR: aTester_Rear.cpp:9331 (GATE W7b-I1).
                            ATCInterfaceForm->ATC_60_SYS.SetHandlerNowArm(1);
                            #endif // GATE F-G9
                        }

                        #ifdef ASE_KaohSiung
                            if(TestIF_File.bEnableReadAndCheckTorque &&                                                 //KaiHuang 20201222 : Index Check 後,用第一次 Contact 的扭力值當標準
                               BTestSuck.UseSiteHasIC())                                                                //kevin 20210804
                            {
                                iReadTorqueError=0;
                                fMain->edTorue1->Text="";
                                bReadArm2_Torque=true;
                                hTorqueDelay.SetSecAndOn(TestIF_File.dReadTorqueDelayTime);                             //kevin 20210804 等待時間去讀取扭力 change by setup
                                bReadFrontTorqueOK =false;                                                              //kevin 20211210 read torque ok
                            }
                        #endif
                        goto TestYFront_ReStart;                                                                        //2008/10/20 lee
                    }
                }
            }
            break;
        case 116:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoTestYFront 116"))
            {
                IndexStatus=Z1Up_Z2Down;
                if(IniConfig.bD38IndexPutICToShtNoWaitMotion &&                 //Steven 20181228 : Add Index Action
                   IniConfig.bIndexArm2SupplyLight==false    &&
                   TestIF_File.bForEgisTecTest==false        &&
                   TestIF_File.bArm1PickPlaceArm2Test==false)
                {
                    bZ1UpAndPlaceZ2Down=CheckPlaceOutShuttle(0);                //ChungHung 20171116 modify for Index Action
                }
                bIndexFinish=false;
                Task=115;
                goto TestYFront_ReStart;
            }
            break;
        case 130:
            if(hTestZ2Delay.Off())
            {
                SW[SwBMotorBreaker].Off();
                Task=200;
                goto TestYFront_ReStart;
            }
            break;
        case 140:
            if(hTestZ2Delay.Off())
            {
                MOT[MTestZ2].ServoOnOff(false);
                Task=200;
                goto TestYFront_ReStart;
            }
            break;
        case 200:
            if(DeviceForm_File.ContactMode==DirectContactSoftEP ||              //kevin 20130608 Soft Contact mode
               DeviceForm_File.ContactMode==DropContactSoftEP)
            {
                SoftContactTim.SetMSAndOn(500);
            }

            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].On();
                SW[SwIndexChangeToque2].Off();
            }

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                            //JerryYang 20230322 : 2D SORT模式index arm不用下壓到socket
            {
                Task=2091;
            }
            else if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                    TestIF_File.bArm1PickPlaceArm2Test==true     &&
                    bCheckGiveWay==true                          )
            {
                Task=2091;
                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                           //Steven 20150304 : Add GPIB LOG
            }
            else
            {
                if(Prod.TestZ2_Drop_Offset!=0)                                  //DropContact or DirectContactModeDiffentSpeed   //ChungHung 20150528 Mark Index2 在丟Device 位置
                {
                    Task=300;
                }
                else
                {
                    if(USE_ReadIndex_TOQUE &&                                   //kevin 20210804
                       TestIF_File.bEnableReadAndCheckTorque==false)            //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
                    {
                        bNeedCheckIndexToque=false;
                        bNeedCheckIndexToque1=false;
                        // AI(pt-wave) 20260811: my GATE (W7c-I1) over-reached and wrapped this line too.  It uses the WORKING seam W64B_FMAIN_CHKREADTORQUE* (defined at this file's :1096-1099), compiles fine, and is the correct translated form -- only the RAW fMain->chkReadTorque* writes need gating.  Un-gated.
                        W64B_FMAIN_CHKREADTORQUE1->Checked=false;                                // AI(W7C-DoTestYFront-c2) 20260810: golden :6986 fMain->chkReadTorque1->Checked=false -- routed through this file's existing W64B_TfMainTorqueSeam (aTester_Front.cpp:1054), same as aTester_Rear.cpp:9408
                        // AI(pt-wave) 20260811: my GATE (W7c-I1) over-reached and wrapped this line too.  It uses the WORKING seam W64B_FMAIN_CHKREADTORQUE* (defined at this file's :1096-1099), compiles fine, and is the correct translated form -- only the RAW fMain->chkReadTorque* writes need gating.  Un-gated.
                        W64B_FMAIN_CHKREADTORQUE2->Checked=true;                                 // AI(W7C-DoTestYFront-c2) 20260810: golden :6987 fMain->chkReadTorque2->Checked=true -- same seam (aTester_Front.cpp:1055)
                        #if 0 // GATE F-G10 -- golden :6988 fMain->edTorue1->Text="" (golden main.h:466-area TEdit).  AI(W7C-DoTestYFront-c2) 20260810
                        //        WHY FAITHFUL: pure UI text clear of the arm-1 torque read-out.  This file's existing torque seam
                        //        W64B_TfMainTorqueSeam (aTester_Front.cpp:1052) carries chkReadTorque1 / chkReadTorque2 / edTorue0 but
                        //        NOT edTorue1, and the seam is file-scope so a mid-function chunk cannot extend it.  The two
                        //        chkReadTorque writes on the lines ABOVE are kept ACTIVE, routed through the existing
                        //        W64B_FMAIN_CHKREADTORQUE1/2 macros (aTester_Front.cpp:1054-1055) exactly as aTester_Rear.cpp:9408-9409
                        //        does with its own seam.  REAL-MACHINE DIFFERENCE: the stale arm-1 torque number stays on screen.
                        //        ABSENCE PROOF: rg -n "edTorue1" aTester_Front.cpp -> ZERO hits (only edTorue0 on the seam, :1056); rg -n "edTorue" FormsFacade.h forms/fMain.h -> ZERO. RUN AT 2026-08-10T23:48:47+0800
                        //        MIRROR: aTester_Rear.cpp:9410 (TODO(G13)) gates the MIRROR-IMAGE member edTorue0 for the same reason --
                        //        each file's seam happens to carry the other arm's edit box.  RETIRE BOTH by adding the missing member
                        //        to each seam (or by giving FormsFacade's TfMain the two TEdits).
                        fMain->edTorue1->Text="";
                        #endif // GATE F-G10
                    }

                    bReadFrontTorqueOK =false;                                  //kevin 20211129 read torque ok
                    fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                       //Steven 20150304 : Add GPIB LOG
                    Task=2091;
                }
            }
            goto TestYFront_ReStart;                                            //Steven 20180813 : add index arm speed
//            break;
        case 209:
            if(DeviceForm_File.ContactMode==DropContactSoftEP)
            {
//                bContSoftEpSwitch(1, true);                                     //ARM1 先浮動頭充氣  //JerryYang 20151202 flase->true
                EPSwitchOnOff(eEPSwBoth);
                SoftContactTim.SetMSAndOn(500);
            }
            Task=2091;
//            break;                                                            //Steven 20180813 : add index arm speed
        case 2091:
            QueueTaskList[19].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化

            if(CUSTOMER_CODE==CC_KYEC_CHEN &&
               DeviceForm.VacuumMode==VacuumOFFMode)                            //jou 2015-07-22 新增Index Vacuum Off mode
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        BTestSuck.Suck[i][j].Normal();
                    }
                }
            }

            if(DeviceForm_File.ContactMode==DirectContactSoftEP ||
               DeviceForm_File.ContactMode==DropContactSoftEP)                  //kevin 20130608 Soft Contact mode
            {
                if(SoftContactTim.Off())
                    Task=2092;
                else
                    break;                                                      //Steven 20141022 : Add Index Speed
            }
            else
            {
                Task=2092;
            }
//            ShowIndexTime(1);                                                 //Steven 20140619 : 測試       //這裡會一直進來,不準
//            break;                                                            //Steven 20141022 : Add Index Speed

        //---- chunk 3/3 of DoTestYFront, golden 7036..8007 ----
        case 2092:
            QueueTaskList[19].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化
            if(bNeedUpDonwOneTome==true)
            {
                bNeedUpDonwOneTome=false;
                Task=1000;
                break;
            }

            HangTime.SetSecAndOn(Prod.iHangupMaxTime);                          //Steven 20090827 Start: Hang Up dectector
            bHangTimePause=false;

            if(CosFunction.bAfterInitialDelayUseOtherArm &&                     //JerryYang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
               Prod.bUseOtherArmToTestAfterInitialDelay &&                      //JerryYang 20180817 (Steven) : fix EOT delay
               bArm2IsTest==false &&
               bFirst==true)
            {
                if(fRearNeedTest==true && BTestSuck.HasRealIC())                //JerryYang 20220923 : 修正換arm預熱問題
                {
                    bFirst=false;                                               //JerryYang 20180817 (Steven) : fix EOT delay
                    if(bArm1Delay==false && bArm2Delay==false)                  //JerryYang 20220923 : 修正換arm預熱問題
                    {
                        CheckInitialStartDelayInSocket();
                        if(bNeedInitialTestDelay==true)
                        {
                            if(InArmSuck.HasIC()        ||
                              FLCarryKit.UseSiteHasIC() ||
                              #if 0 // GATE W7F3-G01 -- golden :7063 FTestSuck.HasNotTestYet().  TRAP 5: the TMyKitSuck this TU sees is aHotPlateSubstrate.h:365 (included at aTester_Front.cpp:115; the object is aHotPlateSubstrate.cpp:92 `TMyKitSuck FTestSuck;`) and it has NO HasNotTestYet member.  The method exists ONLY on the OTHER same-named class, the mykitsuck.h:274 family (declared mykitsuck.h:404, body mykitsuck.cpp:1077, object mykitsuck.cpp:211) -- a DIFFERENT LAYOUT, so switching headers would read every field at the wrong offset.  Absence command + time: see GATE REGISTER.  EXACT twin of aTester_Rear.cpp:9495 TODO(G15) (BTestSuck.HasNotTestYet()); the Front premise holds identically because both files include the same aHotPlateSubstrate.h.  Offline: one term of a 5-term OR is dropped, so bArm1Delay ("change arm and pre-heat") arms slightly LESS often -- it can never arm spuriously.
                              FTestSuck.HasNotTestYet() ||
                              #endif // GATE W7F3-G01
                              bInArmWaitOneCycle==false ||                      //JerryYang 20220923 : 修正換arm預熱問題
                              (iOneCycle==0 &&
                               (MOT[MMPlate1].HasIC() ||
                                MOT[MMPlate2].HasIC())))                        //Sam 20240911 : 避免 HotPlate 還在預熱時就直接測試
                            {
                                bArm1Delay=true;                                //換arm預熱
                            }
                        }
                    }
                    else
                    {
                        if(bNeedInitialTestDelay==true)
                        {
                            if(bArm2Delay==true)
                            {
                                dwStartInitialCount.LatchCycleTime(true);
                            }
                        }
                    }
                }
            }
            FTorqueTimeOutDelay.Set0_1SecAndOn(50);                             //kevin 2021421 扭力
            Task=210;                                                           //JerryYang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
        case 210:                                                                                                       // 編號不得改變 *******************
            QueueTaskList[19].CheckTaskChange();                                                                        //Steven 20200821 : 使用Goto也要記錄Task變化

            if(bHangTimePause==true || bRunAutoClean)                                                                   //Steven 20090827 Start: Hang Up dectector DoTestYFront
            {
                Task=209;
                break;
            }

            if(DeviceForm.ContactMode==DropContact &&                                                                   //JerryYang 20170522 (wei) drop contact改為邊吸邊測
               CosFunction.bSuckDevicesDuringTest==true &&                                                              //JerryYang 20170804 (Steven) 移除邊吸邊測的選項
               INDEX_SUCKER_TYPE==1)
            {
                //AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE W7F3-G02 -- golden :7100 fiosetview->ProcessIndexSuckDestroy2().  TfiosetviewShim (atester_shims.h:404-409) exposes bIndexSuck[2][4][8] and its ctor ONLY -- no pump method at all.  THE RETURN VALUE IS LOAD-BEARING: bCheckSuck gates the "index cycle finished" test at golden :7243 and the CheckIndexAllSuckICFallDown call at golden :7191, so an #if 0 with no substitute would leave bCheckSuck false forever and STALL the state machine.  Offline default true = "suck self-check reports done" -- the SAME value as every other stand-in for this one method in the tree: atester.cpp:5518 W7T1_ProcessIndexSuckDestroy2, atester_32Site.cpp:206 W5_32S_ProcessIndexSuckDestroy2, aTester_Rear.cpp:409 W64bT2_ProcessIndexSuckDestroy2, AutoClean/AutoClean.cpp:278 W906DIAC_ProcessIndexSuckDestroy2, and sibling part 04922`s W7cK7_ProcessIndexSuckDestroy2 -- all of which `return true`.  Twin: aTester_Rear.cpp:9534 TODO(G16) and :10361 GATE W7R3-G03.
                bCheckSuck=fiosetview->ProcessIndexSuckDestroy2();
                //AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                bCheckSuck=true;                                                // GATE W7F3-G02 offline default -- suck self-check reports done (see gate note above)
                //AI(W906-IDXSUCK) （原 #endif）
            }
            else
            {
                bCheckSuck=true;
            }

            if(fRearNeedTest)                                                                                           //Index2 需要測試 (Index2 已在測試區)
            {
                if(TestIF.bUseSLKClamp==true &&
                   TestIF_File.iSeparabilityTest==1 &&
                   bSLKClampProcessFinish==false)                                                                       //JerryYang 20160524 分離模式
                {
                    if(FTestSeparateSLK()==true)                                                                        //JerryYang 20160524 下降到contact高度後Z2需做分離流程
                    {
                        bSLKClampProcessFinish=true;
                        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                              //JerryYang 20180518 (wei) : 分離式SLK ARM1丟IC ARM2分離CLAMP流程
                           TestIF_File.bArm1PickPlaceArm2Test==true)                                                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                        {
                            bArm2NeedCombine=true;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                if(CosFunction.bAfterInitialDelayUseOtherArm &&                                                         //JerryYang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
                   Prod.bUseOtherArmToTestAfterInitialDelay &&
                   bArm2IsTest==false)
                {
                    if(bNeedInitialTestDelay==true)
                    {
                        if(bArm2Delay==true)                                                                            //JerryYang 20200909 修正預熱功能,Arm1下壓->Arm2預熱-> Arm 1測試
                        {                                                                                               //JerryYang 20190626 預熱時另一支arm要能吸放IC
                            if(fFrontNeedDestroy)                                                                       //Index1 上有IC是否要拋下IC (Index1 已在Shuttle1 上)
                            {
                                bZ1PickShuttle=false;                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                                bZ2PickShuttle=false;                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                                fFrontNeedDestroy=!DoFrontTestDestroyIC(true);
                            }
                            else if(fFrontNeedSuck)                                                                     //Index1 是否需要吸取IC
                            {
                                if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                                   (LastSet.iTemperature==Tempture_Hot ||
                                    LastSet.iTemperature==Tempture_AmbientHot) &&                                       //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
                                   DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)                              //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
                                {
                                    if(fRearNeedTest==false)
                                        fFrontNeedSuck=!DoFrontTestSuckIC();
                                }
                                else if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                                        (LastSet.iTemperature==Tempture_Ambient ||
                                         LastSet.iTemperature==Tempture_AmbientHot) &&                                  //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
                                        CUSTOMER_CODE==CC_KYEC_LEE &&                                                   //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
                                        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)
                                {
                                    if(fRearNeedTest==false)
                                        fFrontNeedSuck=!DoFrontTestSuckIC();
                                }
                                else
                                {
                                    fFrontNeedSuck=!DoFrontTestSuckIC();
                                }
                            }

                            if(iInitialCount>0)
                            {
                                iInitialCount=(Prod.iInitialDelay)-(dwStartInitialCount.LatchCycleTime()/1000);
                                break;
                            }

                            str.sprintf("Arm 2 intial delay finish, device: %d ea", BTestSuck.CountRealIC());
                            RecordProcess(str);
                            bArm2Delay=false;
                            fRearNeedTest=false;
                            bNeedInitialTestDelay=false;
                            bFirstTest=true;                                                                            //JerryYang 20180817 (Steven) : fix EOT delay
                            break;
                        }
                        else if(bArm1Delay)                                                                             //換arm 1再做預熱
                        {
                            fRearNeedTest=false;
                            break;
                        }
                    }
                }

                bArm2IsTest=true;                                                                                       //JerryYang 20180629 (wei) : 用來判斷是否在測試中
                fRearNeedTest=!DoBTestSuckTestIC();                                                                     //Index2 測試程序
                if(bCheckSuck==true)                                                                                    //JerryYang 20170522 drop contact改為邊吸邊測
                {
                    CheckIndexAllSuckICFallDown(false, true);                                                           //Steven 20110725 : 修改負壓檢查方式
                }
            }
            else                                                                                                        //JerryYang 20200909 修正預熱功能,Arm1下壓->Arm2預熱-> Arm 1測試
            {
                bArm2Delay=false;
            }

            if(fFrontNeedDestroy)                                                                                       //Index1 上有IC是否要拋下IC (Index1 已在Shuttle1 上)
            {
                bZ1PickShuttle=false;                                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                bZ2PickShuttle=false;                                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                fFrontNeedDestroy=!DoFrontTestDestroyIC(true);
            }
            else if(fFrontNeedSuck)                                                                                     //Index1 是否需要吸取IC
            {
                if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                   (LastSet.iTemperature==Tempture_Hot ||
                    LastSet.iTemperature==Tempture_AmbientHot) &&                                                       //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
                   DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)                                              //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
                {
                    if(fRearNeedTest==false)
                        fFrontNeedSuck=!DoFrontTestSuckIC();
                }
                else if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                        (LastSet.iTemperature==Tempture_Ambient ||
                         LastSet.iTemperature==Tempture_AmbientHot) &&                                                  //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
                        CUSTOMER_CODE==CC_KYEC_LEE &&                                                                   //KaiChen 20180125 (Steven) ：京元-竹南 Suck Shuttle Device After Tested 功能，開啟常溫使用
                        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)                                         //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果)
                {
                    if(fRearNeedTest==false)
                        fFrontNeedSuck=!DoFrontTestSuckIC();
                }
                else
                {
                    fFrontNeedSuck=!DoFrontTestSuckIC();
                }
            }

            if(DeviceForm.ContactMode==DropContact &&
               CosFunction.bSuckDevicesDuringTest==true &&                                                              //JerryYang 20170522 drop contact改為邊吸邊測
               bCheckSuck==true)                                                                                        //JerryYang 20170804 (Steven) 移除邊吸邊測的選項
            {
                bBTestSuckDrop=false;
            }

            if(fFrontNeedDestroy==false &&
               fFrontNeedSuck==false &&                                                                                 //Index1 and Index2動作皆已完成
               fRearNeedTest==false &&
               fRearNeedSuckIC==false &&
               bCheckSuck==true)                                                                                        //JerryYang 20170522 (wei) drop contact改為邊吸邊測
            {
                bEPfirst=true;                                                                                          //Ifor 20150803 : 新增EP量測旗標，第一次才執行Log紀錄
                bFirst=true;                                                                                            //JerryYang 20180817 (Steven) : fix EOT delay
                bSLKClampProcessFinish=false;                                                                           //JerryYang 20160429 進來這裡表示分離流程結束
                bArm2IsTest=false;                                                                                      //JerryYang 20180629 (wei) : 用來判斷是否在測試中
                iWhichArmDown=0;                                                                                        //JerryYang 20200316 add SVID 哪支arm下壓在測區
                if(USE_IO_CHANGE_TOQUE==true)                                                                           //jou 2012-06-21 Enable index I/O Change Toque
                {
                    // AI(k3-DoTestYFront-c3) 20260810 -- MAIN-LOOP INCLUDE HAND-OFF #1, NOT A GATE.  SW[] (port myswitch.h, object myswitch.cpp) is REAL and linkable, but aTester_Front.cpp`s include set (:101-118) does not reach myswitch.h, so this TU cannot see it yet.  The twin already solved this the same way: aTester_Rear.cpp:2950 adds `#include "myswitch.h"` MID-FILE for exactly this reason.  ACTION FOR THE MAIN LOOP: add `#include "myswitch.h"` to aTester_Front.cpp.  Deliberately NOT gated -- gating would silently delete the Tmode brake and the I/O torque-change outputs, i.e. turn real machine actions into no-ops, far worse than a loud undeclared-identifier error.  Covers golden :7252, :7253 and :7367.
                    SW[SwIndexChangeToque1].Off();
                    SW[SwIndexChangeToque2].Off();
                }

                if(IniConfig.bTModeMotorFree==true)
                {
                    SW[SwBMotorBreaker].On();                                                                           //20110923 Tmode 煞車
                    MySleep(200);
                }

                iFlag=CheckAnyCaseNeedToDoArm1();                                                                       //Steven 20190115 : SCC要求吸取異常要檢查Socket
                if(iFlag>1)
                {
                    Task=iFlag;
                    if(iFlag==211 || iFlag==215)
                    {
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                if(BTestSuck.Item[i][j]!=NULL_IC &&
                                   BTestSuck.Item[i][j]!=HAS_NULL_IC)
                                {
                                    BTestSuck.Suck[i][j].Error=false;
                                    bBTestSuckUse[i][j]=true;
                                }
                                bBTSuckFinish[i][j]=false;                                                              //Steven 20110301
                            }
                        }
                    }
                }
                else if(iFlag==1)
                {
                    if(USE_ReadIndex_TOQUE &&
                       TestIF_File.bEnableReadAndCheckTorque)                                                           //kevin 20210804 change
                    {
                        // AI(k3-DoTestYFront-c3) 20260810 -- NOT A GATE (TRAP 3 answered: this SHOULD NOT be gated).  ShowMainScreenPresure IS real and ACTIVE in the port: declared cinitial.h:250, body cinitial.cpp:18785.  The only obstacle is visibility -- aTester_Front.cpp:101-118 does not include cinitial.h, and a mid-function chunk may not add a file-scope declaration.  A BLOCK-SCOPE extern function declaration solves it self-containedly (compile-verified, see GATE REGISTER); it declares the SAME external entity, it is NOT a `static` shadow, so TRAP 1 shape (d) is not created.  The main loop may delete this line once it adds `#include "cinitial.h"` (or the aTester_Rear.cpp:2963-style file-scope declaration).  NOTE the twin`s TODO(G01)/(G11)/(G17) gates on this very call (aTester_Rear.cpp:8757, :9277, :9744) are EXPIRED -- the body landed after they were written, and aTester_Rear.cpp:4466 already calls it LIVE.
                        void ShowMainScreenPresure(int index);                   // AI(k3-DoTestYFront-c3) 20260810: block-scope extern decl of cinitial.h:250 (see note above)
                        ShowMainScreenPresure(1);                                                                       //kevin 20201027 //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                        CheckAndRecodrTorque(1);
                    }

                    #ifdef ASE_KaohSiung
                        if(TestIF_File.bEnableReadAndCheckTorque)                                                       //kevin 20210804 change
                        {
                            if(TestIF_File.iShuttleMode==1 &&
                               TestIF_File.iShuttle_Sel==0)
                            {
                                return true;
                            }
                            else
                            {
                                if(NewCheckAndRecodeTorque(1)==false)
                                {
                                    if(bRetryReadToqu==false)                                                           //kevin 20210419 重讀扭力
                                    {
                                        bRetryReadToqu=true;
                                        bNeedCheckIndexToque=false;                                                     //KEVIN 20210419 ADD TORQUE
                                        bNeedCheckIndexToque1=false;
                                        #if 0 // GATE (W7c-I1)
                                        fMain->chkReadTorque1->Checked=false;
                                        #endif // GATE (W7c-I1)
                                        #if 0 // GATE (W7c-I1)
                                        fMain->chkReadTorque2->Checked=true;
                                        #endif // GATE (W7c-I1)
                                        COM2->ReadIndexTorqueSetting(1);
                                    }

                                    if(FTorqueTimeOutDelay.Off()==false)                                                //kevin 2021018 扭力
                                       return false;
                                }
                            }
                        }
                        ShowMainScreenPresure(1);
                    #endif
                    bIndex2Suck=false;                                                                                  //kevin 20220105 Index drop mode 在下真空建立 不能關閉
                    return true;
                }
            }

            if(IniConfig.bRecordSkipPosition)                                                                           //jou 20150320   auto skip 次數時間導致Hang up
            {
                if(bAutoSkipFlag==true &&
                   ArmSpeed[InArm].bAutoSKIP==true)
                {
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    bAutoSkipFlag=false;
                }
            }

            if(bAskStopPort[ePortLoader] ||                                                                             //JerryYang 20250529 : avoid hang up
               bAskStopPort[ePortAuto1] ||
               bAskStopPort[ePortAuto2] ||
               bAskStopPort[ePortAuto3] ||
               bWaitingAMR==true)                                                                                       //RogerYang 20250617 Load已清空但還沒滿bundle，等料車來
            {
                HangTime.SetSecAndOn(Prod.iHangupMaxTime);
            }

            //#if 0 // GATE W7F3-G03 -- golden :7345-:7353 EnableTraymapCheckFunction(int) (golden ainarm2.h:216, body ainarm2.cpp:3585).  NO port body and NO declaration anywhere: the ONLY textual hit in the whole port tree is the twin`s own gate comment at aTester_Rear.cpp:9793-9795.  Absence command + time: see GATE REGISTER.  The WHOLE if-statement is gated, not just the two calls, because the calls ARE the condition -- there is nothing left to test.  Offline: the traymap-check branch of the auto-skip HangTime re-arm never runs; golden :7326-:7334 (bRecordSkipPosition) and :7336-:7343 (bAskStopPort/bWaitingAMR) still re-arm it, so the watchdog is not disabled, only re-armed on fewer paths.  REAL-MACHINE DIFFERENCE: a traymap-check-driven auto-skip could trip the JAM0316 hang-up watchdog earlier than golden would.  Twin: aTester_Rear.cpp:9793 TODO(G18) -- same premise, and it still holds on the Front side.   //AI(W906-S09-DEC) 20260930 (laptop OK 20260930 15:25): gate retired -- EnableTraymapCheckFunction is defined (ainarm2.cpp:2631) -- the reason on this line is STALE; twin of ainarm9045.cpp:11678 (k6-G4); body = golden 906_0625_Steven aTester_Front.cpp:7345-7353
            if(EnableTraymapCheckFunction(0) &&
               EnableTraymapCheckFunction(1))                                                                           //JerryYang 20250120 : fix誤發hang up
            {
                if(bAutoSkipFlag==true)
                {
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    bAutoSkipFlag=false;
                }
            }
            //#endif // GATE W7F3-G03   //AI(W906-S09-DEC) 20260930: see :9863

            if(HangTime.Off() &&
               TestISTimeOut==false &&
               bHangTimePause==false)
            {
                // AI(k3-DoTestYFront-c3) 20260810 -- MAIN-LOOP INCLUDE HAND-OFF #2, NOT A GATE.  AMR (`extern class TTeraPowerAMR AMR;`, Automation/AMR.h:65, object Automation/AMR.cpp:66) is REAL and linkable; aTester_Front.cpp:101-118 simply does not reach Automation/AMR.h.  The twin already does it: aTester_Rear.cpp:178 `#include "Automation/AMR.h"`.  ACTION FOR THE MAIN LOOP: add `#include "Automation/AMR.h"` to aTester_Front.cpp.  A block-scope extern cannot substitute here -- an elaborated-type-specifier at block scope would declare an INCOMPLETE TTeraPowerAMR and the member call would not compile.  Deliberately NOT gated: gating it would remove the AMR-transport hang-up bypass and make the JAM0316 watchdog fire during a legitimate AMR tray fetch.
                if(AMR.NeedAMRTransport())                                                                              //Sam 20250423 : AMR 搬運花費時間太久可能會報警需要 By Pass
                {
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    break;
                }

                if(IniConfig.bTModeMotorFree==true)
                {
                    SW[SwBMotorBreaker].On();                                                                           //20110923 Tmode 煞車
                    MySleep(200);
                }

                if(CheckHeaterOK()==false)                                                                              //Steven 20250116 : 確認HeaterOK
                {
                    bHangTimePause=true;
                    return false;                                                                                       //kevin 20161102 break;
                }

                bNoUseAutoRecord=true;                                                                                  //wei 20160311

                #ifdef SOFT_SIMULTE
                Task=209;
                break;
                #else
                RecordProcess("Auto State Record by DoTestYFront");
                fMain->DoStateRecord(0, false);                                                                         //Steven 20120705 : Hang Up時，會自動存畫面     //KenHsieh 20230116 : 區分手動或自動(sbclick -> Function)

                #ifdef DEBUG_HANGUP_NO_HOME
                    ShowErrorMessage("JAM0316", K_SKIP, MTestZ1);
                    Task=209;
                #else
                    iHangupCTArm1++;
                    if(iHangupCTArm1>1)
                    {
                        iHangupCTArm1=0;
                        ShowErrorMessage("JAM0316", K_SKIP, MTestZ1);
                        Task=209;
                    }
                    else
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                             //kevin 20160722 ASE 高雄取消
                        {
                            ShowErrorMessage("JAM0316", K_SKIP, MTestZ1);
                            Task=209;
                        }
                        else
                        {
                            MyDBIProcessNew("Motion", "WAR2206", "Auto homing", "0316");                                //Steven 20120705 : Hang Up時，會自動存畫面
                            bHomeByStart=true;
                            fAllMotorHome=false;
                        }
                    }
                #endif
                break;
                #endif
            }
            break;
        case 211:                                                                                                       //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Pick, MOT[MTestZ2].GailSpeed))                                     //Z2 上升至安全位置
            {
                Task=212;
            }
            else                                                                                                        //Steven 20180813 : add index arm speed
            {
                break;
            }
        case 212:                                                               //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, MOT[MTestY1].GailSpeed, __FUNC__))
            {
                Task=213;
            }
            else                                                                //Steven 20180813 : add index arm speed
            {
                break;
            }
        case 213:                                                               //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, MOT[MTestZ2].GailSpeed))
            {
                Task=215;
            }
            else                                                                //Steven 20180813 : add index arm speed
            {
                break;
            }
        case 215:
            bCheckAllSuck=true;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bBTSuckFinish[i][j]==false)                              //Steven 20110301
                    {
                        if(bBTestSuckUse[i][j])
                        {
                            if(BTestSuck.Suck[i][j].Suck())
                            {
                                bBTSuckFinish[i][j]=true;                       //Steven 20110301
                                bBTestSuckUse[i][j]=false;
                            }
                            else if(BTestSuck.Suck[i][j].Error)                 //jou 2011-08-16
                            {
                                bBTSuckFinish[i][j]=true;                       //Steven 20110301
                            }
                            else
                            {
                                bCheckAllSuck=false;
                            }
                        }
                        else
                        {
                            bBTSuckFinish[i][j]=true;                           //Steven 20110301
                        }
                    }
                }
            }

            flag=true;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bBTSuckFinish[i][j]==false)
                        flag=false;
                }
            }

            if(flag)                                                            //Steven 20110301
            {
                if(bCheckAllSuck)
                {
                    return true;
                }
            }
            break;
        case 220:
            DoTestYFrontDelay.SetMSAndOn(OverEncoderDelay);
            Task=230;
//             break;                                                           //Steven 20180813 : add index arm speed
        case 230:
            if(DoTestYFrontDelay.Off())
            {
                if(TestZ2OutRandge())
                {
                    DoTestYFrontDelay.SetSecAndOn(1);
                    Task=240;
                    iRetry=0;
                }
                else
                {
                    return true;
                }
            }
            else                                                                //Steven 20180813 : add index arm speed
            {
                break;
            }
        case 240:
            if(DoTestYFrontDelay.Off())
            {
                if(TestZ2OutRandge2())
                {
                    DoTestYFrontDelay.SetMSAndOn(20);
                    Task=240;
                    iRetry++;
                    if(iRetry<=10)
                        break;
                    else
                        ShowMyMessage("Test Head Ecncoder error, maybe press too low!", "Index錯誤，可能是壓力太低!!", "DoTestYFront 240");
                }
                TestZ2SetPos();
                Task=1;
                return true;
            }
            break;
        case 300:
            if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||         //Eliot 2011_0318 Start //ChungHung 20150528 Mark
               DeviceForm.ContactMode==DirectContactSoftEP ||                   //Steven 20160530
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
//                Task=320;                                                     //Steven 20210617 : Mark for 修正Soft Contact的Delay Time
//                break;
            }
            else
            {
                bBTestSuckDrop=true;
                fRearNeedSuckIC=false;                                          //jou 2011-12-13 丟測才需要設成true

                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_HOT_IC ||
                           BTestSuck.Item[i][j]==HAS_IC)
                        {
                            BTestSuck.Suck[i][j].Off();
                        }
                    }
                }
            }

            if(CosFunction.bTesterSidePushFunction==true &&                     //Richard 20220321 : 渠梁Side Push
               DeviceForm.bTesterSidePush==true)
            {
                // AI(k3-DoTestYFront-c3) 20260810 -- MAIN-LOOP INCLUDE HAND-OFF #3, NOT A GATE.  Cylinder[] (port mycylin.h, object mycylin.cpp) is REAL and linkable; aTester_Front.cpp:101-118 does not reach mycylin.h.  The twin adds it mid-file twice: aTester_Rear.cpp:2951 and :11714 `#include "mycylin.h"`.  ACTION FOR THE MAIN LOOP: add `#include "mycylin.h"` to aTester_Front.cpp.  Deliberately NOT gated -- this is a SAFETY interlock: gating golden :7564 would delete the JAM16130 "Cylinder Side Push Status Error" alarm and let the Index press down while the tester side-push cylinder is still extended.  Covers golden :7562, :7564, :7644 and :7769.
                if(Cylinder[C_TesterSidePush].Enable==true)
                {
                    if(Cylinder[C_TesterSidePush].OffStatus()==false)
                    {
                        #ifndef SOFT_SIMULTE
                        ShowErrorMessage("JAM16130", K_RETRY, MMSystem, false, sTesterSidePush);
                        break;
                        #endif
                    }
                }
            }

            if(IniConfig.bIndexArm2SupplyLight==true ||                         //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
               TestIF_File.bForEgisTecTest==true     ||                         //Steven 20140922 : Arm2當作指紋測試
               (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                 //kevin 20150127 Arm1 下壓 arm2 測試
                TestIF_File.bArm1PickPlaceArm2Test==true))                      //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
            {
                DoTestYFrontDelay.SetSecAndOn(0);                               // delay 0.3 sec for ic down        //Steven 20151117 : for Arm1 Pick Place Arm2 Test
            }
            else
            {
                if(TestIF_File.iShuttleMode==0 ||
                   (TestIF_File.iShuttleMode==1 &&
                    TestIF_File.iShuttle_Sel==0))
                {
                    DoTestYFrontDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);        // delay 0.3 sec for ic down        //Steven 20140909 : 換到迴圈外面
                }
                else
                {
                    DoTestYFrontDelay.SetSecAndOn(0);
                }
            }
            Task=310;
            break;
        case 310:                                                                                                       //丟測放下IC
            bCheckDestroy=true;
            if(bCheckDestroy==true && DoTestYFrontDelay.Off())                                                          //ChungHung 20150528 Mark Index2 Suck 切為 Normail
            {                                                                                                           //Steven 20210617 : Add for 修正Soft Contact的Delay Time
                if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||                                             //Eliot 2011_0318 Start //ChungHung 20150528 Mark
                   DeviceForm.ContactMode==DirectContactSoftEP ||                                                       //Steven 20160530
                   DeviceForm.ContactMode==TMoveSlowContact)                                                            //Steven 20160130 : TMove Soft contact
                {
                    Task=320;
                }
                else
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Item[i][j]==HAS_HOT_IC ||
                               BTestSuck.Item[i][j]==HAS_IC)
                            {
                                BTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }

                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                  //Steven 20180313 (Jou) : 使用Socket Sensor驗證置偏
                       TestIF_File.bArm1PickPlaceArm2Test==true)                                                        //Steven 20200604 : Arm1丟 Arm2測的時候, 只要檢查Arm1
                    {
                    }
                    else
                    {
                        if(IniConfig.bC08_SocketSensor &&
                           TestIF_File.bEnSocketSensor &&
                           TestIF_File.bCheckSocketFloating)
                        {
                            Task=311;
                            break;
                        }
                    }
                }

                if(CosFunction.bTesterSidePushFunction==true &&                                                         //Richard 20220321 : 渠梁Side Push
                   DeviceForm.bTesterSidePush==true)
                {
                    DoTestYFrontDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);                                   //JerryYang 20240111 : add
                    #ifdef SOFT_SIMULTE
                        Task=17900;
                        break;
                    #else
                    if(Cylinder[C_TesterSidePush].Enable==true)
                    {
                        Task=17900;
                        break;
                    }
                    #endif
                }
                Task=320;
            }
            break;
        case 311:                                                               //Steven 20180313 (Jou) : 使用Socket Sensor驗證置偏
            flag=false;
            str="";
            for(int i=0; i<TestIF_File.iSocketCount; i++)
            {
                if(TestIF_File.iSensorCheckType[i]==2 &&                        //Steven 20200420 : Socket Sensor功能可以選
                // AI(k3-DoTestYFront-c3) 20260810 -- MAIN-LOOP INCLUDE HAND-OFF #4, NOT A GATE.  Sen[] (port mysensor.h, object mysensor.cpp) is REAL and linkable; aTester_Front.cpp:101-118 does not reach mysensor.h.  The twin adds it mid-file twice: aTester_Rear.cpp:2950 and :11713 `#include "mysensor.h"`.  ACTION FOR THE MAIN LOOP: add `#include "mysensor.h"` to aTester_Front.cpp.  Deliberately NOT gated -- this is the Socket-Sensor IC-floating detection (WAR0323); gating it would report "no float" for every site and let the Index press a floating device.  SThreadPara.iSocketSensor[] and TestIF_File.iSensorCheckType[] both already resolve through cprod.h (measured).
                   Sen[SThreadPara.iSocketSensor[i]].IsOn())
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:311"));
                    flag=true;
                    str+=IntToStr(i+1)+",";
                }
            }

            if(flag)
            {
                sBufferT="Z1UpZ2Down: Z1Pos IC floating in socket "+IntToStr(iAZ1)+">"+IntToStr(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000)+" Z2Pos "+IntToStr(iAZ1)+">"+IntToStr(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000);
                RecordProcess(sBufferT);                                        //kevin 20150506
                str1.sprintf("%s, case 311", __FUNC__);
                RecordProcess(str1);
                Task=312;
                return false;
            }
            else
            {
                Task=320;
            }
            break;
        case 312:                                                               //丟測放下IC後,偵測到置偏,Index往上
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, 50000, "DoTestYFront 312"))
            {
                Task=313;
            }
            break;
        case 313:                                                               //Y軸分開
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYFront 313"))
            {
                Task=314;
            }
            break;
        case 314:                                                               //丟測放下IC後,偵測到置偏,Alarm
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)            //Steven 20151022 : add for MAXIM
                    bIsTestSitICFallDown=true;
                bIsSocketSensor=true;

                ShowErrorMessage("WAR0323", K_RETRY, MTestZ2, false, str);
                Task=315;
            }
            break;
        case 315:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, 50000, "DoTestYFront 315"))
            {
                Task=316;
            }
            break;
        case 316:                                                               //丟測放下IC後,偵測到置偏排除後, Index往下移動
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, ((MOT[MTestZ2].GailSpeed*Prod.TestZ_Drop_Speed)/100)))
            {
                Task=311;
            }
            break;                                                              //Steven 20180313 (Jou) : 使用Socket Sensor驗證置偏
        case 320:
            if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||                                                                                         //Eliot 2011_0318 Start
               DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
               DeviceForm.ContactMode==TMoveSlowContact ||                                                                                                      //Steven 20160130 : TMove Soft contact
               DeviceForm.ContactMode==TMoveDropSlowContact)
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, ((MOT[MTestZ2].GailSpeed*Prod.TestZ_Drop_Speed)/100)))
                {
                    if(DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
                       DeviceForm.ContactMode==TMoveDropSlowContact)
                    {
                        Task=330;                                                                                                                               //Steven 20160518 : Fixed for DropContactModeDiffentSpeed
                    }
                    else
                    {
                        // AI(k3-DoTestYFront-c3) 20260810 -- NOT A GATE.  MSG_CMD_Arm2Down is REAL (MessageDef.h:102 `extern const unsigned int MSG_CMD_Arm2Down;`, defined MessageDef.cpp); aTester_Front.cpp:101-118 just does not reach MessageDef.h.  A block-scope extern declaration of a complete scalar type IS legal and self-contained (compile-verified, see GATE REGISTER), so no main-loop dependency is created here.  Same entity, external linkage -- NOT a `static` shadow (TRAP 1 shape (d) avoided).  The twin instead declares it at file scope: aTester_Rear.cpp:2964-2965.  The main loop may delete these four declarations once it adds `#include "MessageDef.h"`.
                        extern const unsigned int MSG_CMD_Arm2Down;                                  // AI(k3-DoTestYFront-c3) 20260810: block-scope extern decl of MessageDef.h:102 (see note above)
                        fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                                                                                                   //Steven 20150304 : Add GPIB LOG
                        Task=209;
                    }
                }
            }
            else if(DeviceForm.ContactMode==DropPlaceShiftContact)                                                                                              //ChungHung 20150528 add for 海思 _8Site1x4
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Pick, MOT[MTestZ2].GailSpeed))                                                                         //Z2 上升至安全位置
                {
                    Task=325;
                }
            }
            else
            {
                if(CosFunction.bTesterSidePushFunction==true &&
                   DeviceForm.bTesterSidePush==true &&
                   DoTestYFrontDelay.Off()==false)                                                                                                              //JerryYang 20240111 : add
                {
                    break;
                }

                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, MOT[MTestZ2].GailSpeed*Prod.TestZ_Drop_Speed/100))                       //ChungHung 20150528 Mark Index2 下壓至測試高度 //Steven 20220523 : fix Arm 2 drop contact speed
                {
                    if(DeviceForm.ContactMode==DropContact)
                    {
                        fObserver->AddTimeData(19, DropContactTimer2.LatchCycleTime()/1000.0);                                                                  //JerryYang 20170503 (wei) 第二段drop contact計時,吹氣drop wait time+移動約2mm到contact高度
                        DropContactTimer3.LatchCycleTime(true);

                        if(CosFunction.bTesterSidePushFunction==true &&                                                                                         //Richard 20230301 : 渠梁Side Push 新模式
                           DeviceForm.bTesterSidePush==true &&
                           DeviceForm_File.iSidePushMode==0)                                                                                                    //這裡是先側推先收回, 再測試
                        {
                            DoTestYFrontDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                            #ifdef SOFT_SIMULTE
                                Task=18100;
                                break;
                            #else
                            if(Cylinder[C_TesterSidePush].Enable==true)
                            {
                                Task=18100;
                                break;
                            }
                            #endif
                        }
                    }
                    Task=330;
                }
            }
            break;
        case 325:                                                               //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle-TestIF.dSiteYPitch, MOT[MTestY1].GailSpeed, __FUNC__))
            {
                Task=326;
            }
            break;
        case 326:                                                                                                       //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset,MOT[MTestZ2].GailSpeed))              //ChungHung 20150528 Mark Index2 下壓至測試高度
                Task=330;
            break;
        case 330:
            if(CosFunction.bTesterSidePushFunction==true &&
               DeviceForm.bTesterSidePush==true &&
               DoTestYFrontDelay.Off()==false)                                  //JerryYang 20240111 : add
            {
                break;
            }

            if(USE_ReadIndex_TOQUE &&
               TestIF_File.bEnableReadAndCheckTorque)                           //kevin 20210804 change
            {
                bNeedCheckIndexToque=false;
                bNeedCheckIndexToque1=false;
                // AI(k3-DoTestYFront-c3) 20260810 -- SEAM W7F3-S01, NOT A GATE.  golden :7804-:7806 fMain->chkReadTorque1 / ->chkReadTorque2 / ->edTorue0 (golden main.h:464-466).  The FormsFacade TfMain mirror carries none of the three widgets, so these three lines are routed through THIS FILE`S OWN pre-existing torque seam, aTester_Front.cpp:1052-1056 (`struct W64B_TfMainTorqueSeam` + the W64B_FMAIN_* macros), which is defined far earlier in the file than my range and is already used the same way at aTester_Front.cpp:1448-1450 and :1753-1755.  NOT #if 0`d: all three writes really happen, into the seam object, so nothing is dropped.  DIFFERENCE FROM THE TWIN: aTester_Rear.cpp:10317 GATE W7R3-G02 had to GATE edTorue0 because its W64bT2_TfMainTorqueSeam carries edTorue1 only; the Front seam DOES carry edTorue0, so the Front side keeps all three writes ACTIVE.  Real-machine difference: the main screen`s torque checkboxes/field are not repainted (UI only -- and aTester_Front.cpp:1633/:1835 read the seam back, so the read/write pair stays self-consistent).
                W64B_FMAIN_CHKREADTORQUE1->Checked=false;
                W64B_FMAIN_CHKREADTORQUE2->Checked=true;
                W64B_FMAIN_EDTORUE0->Text="";
            }

            bIndex2Suck=true;                                                   //kevin 20220105 Index 在下真空建立 pause 不能關閉
            if(DeviceForm.VacuumMode==VacuumONMode)
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_HOT_IC ||
                           BTestSuck.Item[i][j]==HAS_IC)
                        {
                            if(INDEX_SUCKER_TYPE==1)                            //Steven 20111202
                            {
                                fiosetview->bIndexSuck[1][i][j]=true;
                                bIndexCheckNoStopVaccum=true;                   //Sam 20220816 : 增加保護避免暫停 AllSite 掉料
                            }
                            else
                            {
                                BTestSuck.Suck[i][j].On();                      //Sam 20230110 : 修正連續 Start<>Stop Drop Contact 掉料問題
                            }
                        }
                    }
                }
            }

            if(DeviceForm.ContactMode==DropContact &&                           //JerryYang 20170804 (Steven) 移除邊吸邊測的選項
               CosFunction.bSuckDevicesDuringTest==true)                        //JerryYang 20170522 (wei) drop contact改為邊吸邊測
            {
                // AI(k3-DoTestYFront-c3) 20260810 -- NOT A GATE.  MSG_CMD_Arm2Down is REAL (MessageDef.h:102 `extern const unsigned int MSG_CMD_Arm2Down;`, defined MessageDef.cpp); aTester_Front.cpp:101-118 just does not reach MessageDef.h.  A block-scope extern declaration of a complete scalar type IS legal and self-contained (compile-verified, see GATE REGISTER), so no main-loop dependency is created here.  Same entity, external linkage -- NOT a `static` shadow (TRAP 1 shape (d) avoided).  The twin instead declares it at file scope: aTester_Rear.cpp:2964-2965.  The main loop may delete these four declarations once it adds `#include "MessageDef.h"`.
                extern const unsigned int MSG_CMD_Arm2Down;                                  // AI(k3-DoTestYFront-c3) 20260810: block-scope extern decl of MessageDef.h:102 (see note above)
                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                           //Steven 20150304 : Add GPIB LOG
                Task=209;
                break;
            }

            DoTestYFrontDelay.SetMSAndOn(100);                                  // delay 0.1 sec for ic down
            Task=340;
            break;
        case 340:
            if(INDEX_SUCKER_TYPE==1)
            {
                //AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE W7F3-G04 -- golden :7847 fiosetview->ProcessIndexSuckDestroy2(), the case-340 twin of GATE W7F3-G02.  Same absent method on the same TfiosetviewShim (atester_shims.h:404-409), same load-bearing return value: bCheckSuck gates the case-340 exit at golden :7854, so no substitute would park case 340 forever.  Offline default true = suck self-check done, identical to every other stand-in listed in GATE W7F3-G02.  Twin: aTester_Rear.cpp:10361 GATE W7R3-G03.
                bCheckSuck=fiosetview->ProcessIndexSuckDestroy2();
                //AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                bCheckSuck=true;                                                // GATE W7F3-G04 offline default -- suck self-check reports done (see gate note above)
                //AI(W906-IDXSUCK) （原 #endif）
            }
            else
            {
                bCheckSuck=true;
            }

            if(bCheckSuck==true && DoTestYFrontDelay.Off())
            {
                bIndexCheckNoStopVaccum=false;                                  //Sam 20220902 : 修正 All Site 掉料
                // AI(k3-DoTestYFront-c3) 20260810 -- NOT A GATE.  MSG_CMD_Arm2Down is REAL (MessageDef.h:102 `extern const unsigned int MSG_CMD_Arm2Down;`, defined MessageDef.cpp); aTester_Front.cpp:101-118 just does not reach MessageDef.h.  A block-scope extern declaration of a complete scalar type IS legal and self-contained (compile-verified, see GATE REGISTER), so no main-loop dependency is created here.  Same entity, external linkage -- NOT a `static` shadow (TRAP 1 shape (d) avoided).  The twin instead declares it at file scope: aTester_Rear.cpp:2964-2965.  The main loop may delete these four declarations once it adds `#include "MessageDef.h"`.
                extern const unsigned int MSG_CMD_Arm2Down;                                  // AI(k3-DoTestYFront-c3) 20260810: block-scope extern decl of MessageDef.h:102 (see note above)
                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                           //Steven 20150304 : Add GPIB LOG
                bBTestSuckDrop=false;                                           //ChungHung 20150528 Mark Index2 下壓至測試高度 並確認完成
                Task=209;
            }
            break;
        case 350:
            bBTestSuckDrop=false;
            Task=209;
            break;
        case 1000:
            #ifdef SOFT_SIMULTE
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+10000, MOT[MTestZ2].GailSpeed))
                    Task=1100;
            #else
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000, MOT[MTestZ2].GailSpeed))
                    Task=1100;
            #endif
            break;
        case 1100:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, MOT[MTestZ2].GailSpeed))
            {
                // AI(k3-DoTestYFront-c3) 20260810 -- NOT A GATE.  MSG_CMD_Arm2Down is REAL (MessageDef.h:102 `extern const unsigned int MSG_CMD_Arm2Down;`, defined MessageDef.cpp); aTester_Front.cpp:101-118 just does not reach MessageDef.h.  A block-scope extern declaration of a complete scalar type IS legal and self-contained (compile-verified, see GATE REGISTER), so no main-loop dependency is created here.  Same entity, external linkage -- NOT a `static` shadow (TRAP 1 shape (d) avoided).  The twin instead declares it at file scope: aTester_Rear.cpp:2964-2965.  The main loop may delete these four declarations once it adds `#include "MessageDef.h"`.
                extern const unsigned int MSG_CMD_Arm2Down;                                  // AI(k3-DoTestYFront-c3) 20260810: block-scope extern decl of MessageDef.h:102 (see note above)
                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                           //Steven 20150304 : Add GPIB LOG
                Task=209;
            }
            break;
        case 1200:
            DoTestYFrontDelay.SetMSAndOn(300);
            Task=1300;
            break;
        case 1300:
            if(DoTestYFrontDelay.Off())
                Task=1000;
            break;
        case 2000:
            if(REAL_TIME_CCD==true && COM2->bCCDDummyRum==false)
            {
                Task=65;
            }
            else
            {
                Task=70;
            }
            break;
        case 12000:                                                             //jou 2014-06-24 RTC 自動進行Model驗證 start
            DoFRTCAutoModelVerify(true);
            Task=12010;
            break;
        case 12010:
            if(DoFRTCAutoModelVerify(false))
            {
                bNeedWaitRTCAutoVerify=false;
                return true;
            }
            break;
        case 14000:
            if(FTestCombineSLK()==true)                                         //JerryYang 20160429 分離模式, 測試完成 Z2 SLK要與Clamp結合,再將IC吸起流程
            {
                return true;
            }
            break;
        case 16000:                                                             //Steven 20190115 : SCC要求吸取異常要檢查Socket
            DoIndexArm2PickUpErrNeedPiggyback(true);
            Task=16100;
//            break;
        case 16100:
            if(DoIndexArm2PickUpErrNeedPiggyback(false))
            {
                Task=1;
                return true;
            }
            break;
        case 17900:                                                             //Richard 20220321 : 渠梁Side Push
            if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)      //Richard 20221110 : 渠梁Side Push 新增關SITE不動作
            {
                RecordProcess("DoTestYFront_Case17900");
                Task=320;
                break;
            }

            if(DoTestYFrontDelay.Off()==false)
            {
                break;
            }

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-100, MOT[MTestZ2].GailSpeed*Prod.TestZ_Drop_Speed/100))
            {
                DoTestYFrontDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                Task=18000;
            }
            break;
        case 18000:
            if(DoTestYFrontDelay.Off()==false)
            {
                break;
            }

            if(DoTesterSidePush(true)==1)
            {
                DoTestYFrontDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                Task=320;                                                       //Richard 20230301 : 渠梁Side Push 新模式   18100->320
            }
            break;
        case 18100:
            if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)      //Richard 20221110 : 渠梁Side Push 新增關SITE不動作
            {
                RecordProcess("DoTestYFront_Case18100");
                Task=330;
                break;
            }

            if(DoTestYFrontDelay.Off()==false)
            {
                break;
            }

            if(DoTesterSidePush(false)==10)
            {
                Task=330;                                                       //Richard 20230301 : 渠梁Side Push 新模式   320->210
                DoTestYFrontDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
            }
            break;
        case 18500:
            if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)      //Richard 20221110 : 渠梁Side Push 新增關SITE不動作
            {
                RecordProcess("DoTestYFront_Case18500");
                return true;
            }

            if(DoTesterSidePush(false)==10)
            {
                return true;
            }
            break;
        //==> Eastsun 20260514 F006 fix: Ifor 20220308 add Index two-stage rise case 20000/20001 (from a-side L7743-7756)
        case 20000:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+Prod.TestZ1_Up_Offset, MOT[MTestZ1].GailSpeed*Prod.TestZ_Up_Speed/100, __FUNC__))
            {
                DoTestYFrontDelay.SetSecAndOn(Prod.TestZ_Up_Wait);
                Task=20001;
            }
            break;
        case 20001:
            if(DoTestYFrontDelay.Off())
            {
                Task=108;
            }
            break;
        //<== Eastsun 20260514 F006 fix
        }
    return false;
}

//---------------------------------------------------------------------------
//  PART FILE  _w7c_parts/08011_InitFrontTestPurgBeforePickShuttle.txt
//  (label k8-small-seven)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing content.
//  Translator: AI(k8-small-seven) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 8011..8014 -- and nothing else.  Emitted
//  UTF-8 / 100% CRLF, zero U+FFFD; the golden trailing comment on the signature
//  line is transcribed character for character.
//
//  ROLE          -- Reset entry for the FRONT pre-pick purge state machine (blow
//                   the front test-head nozzles before picking off the shuttle);
//                   rewinds its cursor to step 1.
//  WAVE SCOPE    -- InitFrontTestPurgBeforePickShuttle()  ACTIVE  golden :8011-8014
//  GATE REGISTER -- EMPTY.  No #if 0, no substitution.  The single symbol it
//                   writes, iFrontTestPurgBeforePickShuttle, is REAL data already
//                   in the port at atester_shims.cpp:162 (extern
//                   atester_shims.h:85; golden aTester_Front.h:13).
//  OUT OF MY RANGE (hand-off, two separate things -- do NOT emit either here):
//                   * golden :8009 `int iFrontTestPurgBeforePickShuttle=1;` --
//                     already real at atester_shims.cpp:162, may simply STAY (plain
//                     data, not a stub body, so no duplicate-definition risk).
//                   * golden :8010 `TQPF_Timer hDoFrontTestPurgBeforePickShuttle;`
//                     -- one of the 16 file-scope globals the MAIN LOOP owns.  My
//                     sibling part 08016_DoFrontTestPurgBeforePickShuttle.txt only
//                     USES it.  NOTE this DIVERGES from the Rear twin, whose part
//                     file emitted the corresponding timer itself (port
//                     aTester_Rear.cpp:11557) -- correct there, wrong here.
//  SYMMETRIC TWIN -- InitRearTestPurgBeforePickShuttle (golden
//                   aTester_Rear.cpp:9237-9240, port aTester_Rear.cpp:11489-11492).
//                   Here the twin IS statement-for-statement identical modulo the
//                   Front/Rear cursor name, and its empty GATE REGISTER holds on
//                   this side too.
//  TRAP 1 -- NOT `static`: golden aTester_Front.h:24 declares it extern and the
//                   port declares it non-static at atester_shims.h:100, so
//                   `static` would be shape (d).  Live port caller this body will
//                   serve (so not shape (a)): aTester_Front.cpp:1195, inside the
//                   already-translated DoFrontTestSuckIC.
//  TRAP 4 -- adds no file-scope object.
//  INTEGRATE     -- retires the no-op stub atester_shims.cpp:176
//                   `void InitFrontTestPurgBeforePickShuttle() {}` (which left the
//                   purge cursor wherever the previous purge abandoned it); keep
//                   the declaration atester_shims.h:100.
//---------------------------------------------------------------------------
void InitFrontTestPurgBeforePickShuttle()                                       //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue Start
{
    iFrontTestPurgBeforePickShuttle=1;
}

//---------------------------------------------------------------------------
//  PART FILE  _w7c_parts/08016_DoFrontTestPurgBeforePickShuttle.txt
//  (label k8-small-seven)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing content.
//  Translator: AI(k8-small-seven) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 8016..8096 -- DoFrontTestPurgBeforePickShuttle
//  and nothing else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD; Chinese comments
//  transcribed character for character from cp950.
//
//  ROLE          -- FRONT pre-pick purge state machine.  Moves MTestZ1 to the
//                   purge stand-off (TestZ1_Pick + offset*100), then repeats
//                   iPurgeBeforePickShuttleInterval times: vacuum OFF on every
//                   front nozzle that is NOT holding an IC, wait
//                   iPurgeBeforePickShuttleTime, back to Normal, wait 1 s.  Once
//                   the interval counter is exhausted (case 400) it restores
//                   Normal on every nozzle and reports done (case 500).
//  WAVE SCOPE    -- DoFrontTestPurgBeforePickShuttle(int isp)  ACTIVE
//                                                               golden :8016-8096
//                   NOT EMITTED: golden :8010 `TQPF_Timer
//                   hDoFrontTestPurgBeforePickShuttle;` -- the MAIN LOOP owns it
//                   (one of the 16 file-scope globals).  This part only USES it.
//                   (The Rear twin's part file DID emit its own timer at port
//                   aTester_Rear.cpp:11557; that is the one thing not to copy.)
//  GATE REGISTER -- EMPTY.  This part writes NO #if 0 and makes NO offline
//                   substitution.  Every symbol verified real in the port at
//                   2026-08-10 21:45..21:50 +0800:
//                     * Prod.TestZ1_Pick                cprod.h:429 (PROD_INFO_ST)
//                     * DeviceForm_File.iPurgeBdforePickShuttleOffSet  cprod.h:1219
//                     * DeviceForm_File.iPurgeBeforePickShuttleInterval cprod.h:1218
//                     * DeviceForm_File.iPurgeBeforePickShuttleTime     cprod.h:1217
//                     * TMotor::Gali_MotMove(int,int,AnsiString="")
//                                                        Motor/mymotor.h:201
//                     * FTestSuck.iShtRow / iShtCol / Item / Suck
//                                                        aHotPlateSubstrate.h
//                     * HAS_IC / HAS_HOT_IC              cmydef.h:158 / :160
//                     * TQPF_Timer::SetSecAndOn / Off    myTimer.h:38 / :30
//                     * iFrontTestPurgBeforePickShuttle  atester_shims.cpp:162
//                   Absence command used for the timer hand-off claim:
//                     Grep(pattern="hDoFrontTestPurgBeforePickShuttle|...",
//                          path=D:/HT9045/HT9011UC_Cpp_V3.33.906.0, glob=*.{cpp,h})
//                     RESULT: 0 hits for hDoFrontTestPurgBeforePickShuttle (only
//                     the Rear name matched) -- so the main loop MUST add golden
//                     :8010 or this part will not link.  Measured 2026-08-10
//                     21:45 +0800.  RE-RUN AT INTEGRATION (TRAP 2).
//  TRAP 5 (which header) -- FTestSuck is the aHotPlateSubstrate.h TMyKitSuck
//                   (extern aHotPlateSubstrate.h:636, object DEFINED at
//                   aHotPlateSubstrate.cpp:92), and its Suck[][] elements are the
//                   aHotPlateSubstrate.h:106 TMySucker.  The RIVAL pair
//                   mykitsuck.h:274 TMyKitSuck / `extern TMyKitSuck FTestSuck;`
//                   mykitsuck.h:458 / object mykitsuck.cpp:211 has a DIFFERENT
//                   LAYOUT and is deliberately NOT relied on: aTester_Front.cpp's
//                   include list (port line 115) pulls aHotPlateSubstrate.h and
//                   never mykitsuck.h.  Picking the other one would link cleanly
//                   and read every field at the wrong offset.
//  GOLDEN QUIRKS preserved verbatim (NOT fixed):
//                   (a) case 100 FALLS THROUGH into case 200 when the interval
//                       counter has NOT expired (it sets Task=200 with no break);
//                       only the else-arm breaks.  Fall-through kept exactly.
//                   (b) the `if(...HAS_IC || ...HAS_HOT_IC) ;` empty-then /
//                       else-branch idiom appears three times (golden :8040-8044,
//                       :8061-8065, :8083-8087) and is kept as written.
//                   (c) `static int iCT=0;` is a FUNCTION-local static (no ctor,
//                       so no TRAP 4 hazard) and is never reset on entry -- only
//                       case 1 zeroes it, so an abort between case 100 and case
//                       400 leaves the count where it was.  Faithful.
//                   (d) golden :8035 has a BLANK LINE between SetSecAndOn and the
//                       nozzle loop that the Rear twin's golden does not; kept.
//  INTEGER DIVISION -- none.  The one arithmetic expression is
//                   `Prod.TestZ1_Pick+DeviceForm_File.iPurgeBdforePickShuttleOffSet*100`
//                   -- integer multiply, no division, no float helper.
//  SOFT_SIMULTE  -- this range contains no #ifdef of any kind.
//  TRAP 1 -- NOT `static`: golden aTester_Front.h:34 declares
//                   `extern bool DoFrontTestPurgBeforePickShuttle(int isp);` and
//                   the port declares it non-static at atester_shims.h:105, so
//                   `static` would be shape (d).  Live port caller this body will
//                   serve (so not shape (a)): aTester_Front.cpp:1309, inside the
//                   already-translated DoFrontTestSuckIC.
//  INTEGRATE     -- retires the offline stub atester_shims.cpp:181
//                   `bool DoFrontTestPurgBeforePickShuttle(int) { return true; }`.
//                   That stub reported the purge INSTANTLY COMPLETE -- a real
//                   behaviour change (no Z move, no vacuum cycling at all), not a
//                   cosmetic one.  Keep the declaration at atester_shims.h:105.
//---------------------------------------------------------------------------
bool DoFrontTestPurgBeforePickShuttle(int isp)
{
    int &Task=iFrontTestPurgBeforePickShuttle;
    static int iCT=0;

    switch(Task)
    {
        case 1:                                                                 //移至預吹氣位置
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Pick+DeviceForm_File.iPurgeBdforePickShuttleOffSet*100, isp))
            {
                iCT=0;
                Task=100;
            }
            break;
        case 100:
            if(iCT<DeviceForm_File.iPurgeBeforePickShuttleInterval)
            {
                iCT++;
                hDoFrontTestPurgBeforePickShuttle.SetSecAndOn(DeviceForm_File.iPurgeBeforePickShuttleTime);

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_IC ||
                           FTestSuck.Item[i][j]==HAS_HOT_IC)                    //Steven 20151016 : Fixed for shuttle pruge function
                            ;
                        else
                            FTestSuck.Suck[i][j].Off();
                    }
                }
                Task=200;
            }
            else
            {
                Task=400;
                break;
            }
        case 200:
            if(hDoFrontTestPurgBeforePickShuttle.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_IC ||
                           FTestSuck.Item[i][j]==HAS_HOT_IC)                    //Steven 20151016 : Fixed for shuttle pruge function
                            ;
                        else
                            FTestSuck.Suck[i][j].Normal();
                    }
                }
                hDoFrontTestPurgBeforePickShuttle.SetSecAndOn(1);
                Task=300;
            }
            break;
        case 300:
            if(hDoFrontTestPurgBeforePickShuttle.Off())
            {
                Task=100;
            }
            break;
        case 400:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==HAS_IC ||
                       FTestSuck.Item[i][j]==HAS_HOT_IC)                        //Steven 20151016 : Fixed for shuttle pruge function
                        ;
                    else
                        FTestSuck.Suck[i][j].Normal();
                }
            }
            Task=500;
            break;
        case 500:
            return true;
    }
    return false;
}

// ==========================================================================
//  PART FILE  _w7c_parts/08098_FTestNeedDestroy.txt   (label k6-SLK-trio, 1 of 3)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing content.
//  Translator: AI(k6-SLK-trio) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 8098..8108 -- FTestNeedDestroy() and nothing
//  else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD; Chinese comments transcribed
//  character for character from cp950.
//
//  ROLE
//  ----
//  FTestNeedDestroy(): "does the FRONT test-head suck grid (FTestSuck) need a
//  destroy-blow before the head may move?"  A pure FTestSuck Item-grid
//  predicate -- no HAL, no motion, nothing written.  Live consumers already in
//  the port: atester.cpp:4343 and atester_32Site.cpp:1385.
//
//  WAVE SCOPE
//  ----------
//    * FTestNeedDestroy()                golden :8098-8108   ACTIVE
//
//  GOLDEN BUG -- KEPT, NOT FIXED: golden's 2nd branch (golden :8102,
//  else if(FTestSuck.All_HAS_NULL_IC()) ) is textually IDENTICAL to the 1st
//  (golden :8100), so it is unreachable dead code.  Reproduced exactly as-is.
//  The mirror function on the rear side has the SAME duplicated branch (golden
//  aTester_Rear.cpp:9325/:9327, already translated at port aTester_Rear.cpp
//  :11760-11770), so this is a golden-wide copy/paste, not a Front typo.
//  Changing it would be a redesign, not a translation.
//
//  GATE REGISTER
//  -------------
//  [G-k6-1]  golden :8100 and :8102  --  FTestSuck.All_HAS_NULL_IC()
//    STATUS: golden TEXT gated (the #if 0 GOLDEN VERBATIM block below);
//            BEHAVIOUR fully ACTIVE via a faithful free-function transcription.
//            This is NOT an offline default -- the returned bool is identical
//            for identical grid state.
//    WHY IT MUST STAY GATED (TRAP 3, re-asked from scratch -- the answer is NOT
//      "the symbol is missing", because a real body does exist):
//      TMyKitSuck::All_HAS_NULL_IC() is declared only on the OTHER TMyKitSuck --
//      mykitsuck.h:408, body mykitsuck.cpp:595.  The object FTestSuck is defined
//      at aHotPlateSubstrate.cpp:92 against aHotPlateSubstrate.h:365's
//      TMyKitSuck, a DIFFERENT class with a DIFFERENT layout, and that one does
//      not declare All_HAS_NULL_IC at all.  mykitsuck.cpp is DELIBERATELY NOT
//      REGISTERED in the build (CMakeLists.txt:2099-2106 -- it re-defines 24
//      globals against aHotPlateSubstrate.cpp's 14).  Linking it in to obtain
//      one bool predicate would silently re-offset every kit-grid field in the
//      177 TUs that use the aHotPlateSubstrate.h layout.  So: a body exists and
//      is even reachable in principle -- and the gate still stays, for the
//      ODR/layout reason.  Identical conclusion, independently re-derived, to
//      GATE G-k7-1 on the rear side (port aTester_Rear.cpp:11661-11695); the
//      premise holds unchanged on the Front side because FTestSuck and BTestSuck
//      are the same class from the same header (aHotPlateSubstrate.h:636-637).
//    WHY THE LIVE PATH IS FAITHFUL: golden's method body (golden MyKitSuck.cpp
//      :406-430, read directly, not taken on trust from the rear part) touches
//      ONLY iMaxRow, iMaxCol, Item[][], HAS_NULL_IC and NULL_IC.
//      aHotPlateSubstrate.h's TMyKitSuck exposes all of them (iMaxRow :377,
//      iMaxCol :402, Item :370), so W7cK6_All_HAS_NULL_IC(kit) below is a
//      statement-for-statement copy of the golden method as a free function.
//      The three counters, their order, and the three-way decision are golden's
//      verbatim; nothing is defaulted or approximated.
//    REAL-MACHINE BEHAVIOUR DIFFERENCE: none.
//    ABSENCE COMMANDS (run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//      rg -n "All_HAS_NULL_IC" aHotPlateSubstrate.h
//        -> exit 1, no output
//      rg -n "All_HAS_NULL_IC" --glob '*.h' --glob '*.cpp' -g '!build*' .
//        -> exit 0; the ONLY hits are mykitsuck.h:408, mykitsuck.cpp:44/:595,
//           the rear part's own banner+gate text in aTester_Rear.cpp, and
//           atester.cpp:3561 -- and that one call site is INSIDE the #if 0
//           opened at atester.cpp:2614 (GATE G-PTk2-ProcessTestResult, golden
//           :2621-3579), i.e. not compiled.  Verified by reading atester.cpp
//           :2614 and :3561 directly, not inferred.
//      rg -n "^\s*mykitsuck\.cpp" CMakeLists.txt
//        -> exit 1, no output (the two textual hits at CMakeLists.txt:2099 and
//           :2105 are # comments explaining WHY it is not registered)
//    MEASURED AT: 2026-08-10 23:44:27 +0800 (all four commands, same run).
//
//  TRAP 1 (archive extraction) -- FTestNeedDestroy below has EXTERNAL linkage;
//  no `static`, matching the non-static declaration at atester_shims.h:103.
//  INTEGRATE MUST DELETE the no-op stub
//  `bool FTestNeedDestroy() { return false; }` at atester_shims.cpp:179,
//  otherwise this is TRAP-1 shape (b): the stub already satisfies both live
//  references, the real body is never extracted, the build is green and nothing
//  changed.  nm --undefined-only cannot see that.  The stub answers "no destroy
//  needed" unconditionally, so on a real machine leaving it in place means the
//  head can leave the socket with a HAS_NULL_IC site never blown clear.
//
//  TRAP 4 (static init) -- this part adds NO file-scope object of any kind.  The
//  only new file-scope entity is a function (W7cK6_All_HAS_NULL_IC); a function
//  definition runs no code before main().
//
//  TRAP 5 (two headers, same class name) -- FTestSuck is the
//  aHotPlateSubstrate.h:365 TMyKitSuck, NOT mykitsuck.h:274's.  The object is
//  defined at aHotPlateSubstrate.cpp:92 (`TMyKitSuck FTestSuck;`), declared
//  aHotPlateSubstrate.h:636, and port aTester_Front.cpp:115 already includes
//  aHotPlateSubstrate.h for exactly this reason.  W7cK6_All_HAS_NULL_IC takes
//  `TMyKitSuck &` and therefore binds to whichever class that include resolves
//  to -- which is why the member list was checked against
//  aHotPlateSubstrate.h line by line above rather than against mykitsuck.h.
//
//  NO NEW #include is required by this part (aHotPlateSubstrate.h, cmydef.h for
//  NULL_IC/HAS_NULL_IC are both already in aTester_Front.cpp:113-115).
// ==========================================================================
//------------------------------------------------------------------------------
// -- TMyKitSuck::All_HAS_NULL_IC (golden MyKitSuck.h:306, body golden
//    MyKitSuck.cpp:406-430) -- FAITHFUL free-function transcription; see
//    [G-k6-1] above for why the golden member call itself stays gated.
//    NAME DELIBERATELY DISTINCT from the rear side's W7bK7_All_HAS_NULL_IC
//    (port aTester_Rear.cpp:11721): that one has INTERNAL linkage in its own TU
//    and is not visible here, so this TU needs its own copy.  Because the name
//    is new tree-wide, `static` here cannot become TRAP-1 shape (d) -- there is
//    no non-static declaration of W7cK6_All_HAS_NULL_IC anywhere.
static bool W7cK6_All_HAS_NULL_IC(TMyKitSuck &kit)
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    for(int i=0; i<kit.iMaxRow; i++)
    {
        for(int j=0; j<kit.iMaxCol; j++)
        {
            if(kit.Item[i][j]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(kit.Item[i][j]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return false;
    else if(iHasNullICCount)
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
#if 0 // GOLDEN VERBATIM -- golden aTester_Front.cpp:8098-8108  (GATE G-k6-1, begin)
bool FTestNeedDestroy()
{
    if(FTestSuck.All_HAS_NULL_IC())
        return true;
    else if(FTestSuck.All_HAS_NULL_IC())
        return true;
    else if(FTestSuck.UseSiteNoIC())
        return true;
    else
        return false;
}
#endif // GOLDEN VERBATIM -- golden aTester_Front.cpp:8098-8108  (GATE G-k6-1, end)
bool FTestNeedDestroy()
{
    if(W7cK6_All_HAS_NULL_IC(FTestSuck))                                        // AI(k6-SLK-trio) 20260810: golden :8100 FTestSuck.All_HAS_NULL_IC() -- faithful local transcription, see [G-k6-1]
        return true;
    else if(W7cK6_All_HAS_NULL_IC(FTestSuck))                                   // AI(k6-SLK-trio) 20260810: golden :8102 -- golden's own duplicate of :8100 (unreachable), preserved
        return true;
    else if(FTestSuck.UseSiteNoIC())
        return true;
    else
        return false;
}

// ==========================================================================
//  PART FILE  _w7c_parts/08110_FTestSeparateSLK.txt   (label k6-SLK-trio, 2 of 3)
//  Stitch target: aTester_Front.cpp -- APPEND after part 1 of this trio.
//  Translator: AI(k6-SLK-trio) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 8110..8199 -- FTestSeparateSLK(bool) and
//  nothing else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD; Chinese comments
//  transcribed character for character from cp950.
//
//  ROLE
//  ----
//  FTestSeparateSLK(bool bReset): the "Socket-clamp separability" MODE hand-off
//  SM for the configuration where ARM 2 / Z2 is the arm that PLACES.  Z2 is
//  already down at test height; this SM closes the Socket clamp, breaks vacuum
//  on every ARM-2 nozzle so the ICs are left standing in the sockets, then
//  releases the SLK2 clamp and lifts Arm2 5 mm (1500 counts) clear before the
//  test fires.  Returns true only on that final lift completing.  bReset -- or
//  the feature being off -- re-homes the cursor to Task=1 and returns false.
//  Live consumers already in the port: aTester_Rear.cpp:8986, :9622, :10823
//  (inside DoTestYRear).
//
//  WAVE SCOPE
//  ----------
//    * FTestSeparateSLK(bool)            golden :8110-8199   ACTIVE
//
//  GOLDEN CROSS-NAMING -- READ BEFORE "FIXING" ANYTHING.  This is the F-named
//  function but it drives the REAR-side objects: BTestSuck (golden :8145-8149),
//  C_SLK2_Clamp / C_SLK2_Unclamp, MOT[MTestZ2], Prod.TestZ2_Test, MTestY2.  That
//  is golden's own convention here -- in the "分離模式" pairing the F* routine is
//  the one used while Arm2 places and Arm1 tests, which is precisely why the
//  mirror BTestSeparateSLK (golden aTester_Rear.cpp:9335, port aTester_Rear.cpp
//  :11853) drives FTestSuck / C_SLK1_* / MTestZ1 / Prod.TestZ1_Test / MTestY1.
//  Do NOT "correct" F->B here; both halves were read side by side to confirm the
//  crossing is deliberate and complete.
//
//  ASYMMETRY vs THE REAR TWIN -- DELIBERATE, PRESERVED.  The rear
//  BTestSeparateSLK early-out (port aTester_Rear.cpp:11861-11869) has a FOURTH
//  clause,  (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
//  TestIF_File.bArm1PickPlaceArm2Test==true) .  Golden's FRONT version at
//  :8117-8119 has only THREE clauses and no such guard.  Verified by reading
//  both golden files.  Kept exactly as golden has it: on the Front side this SM
//  still runs in Arm1-place/Arm2-test mode.
//
//  GATE REGISTER -- EMPTY.  This part writes NO #if 0 of its own and makes NO
//  offline substitution.  Every symbol golden touches here exists in the port,
//  each checked by name (all verified 2026-08-10 23:44 +0800):
//    iFTestSocketClampCloseTask   atester_shims.cpp:164 (extern :h:87)
//    Cylinder[]                   mycylin.h:176; .On() :112, .Off() :113,
//                                 .OnStatus() :117
//    C_Socket_Clamp / C_Socket_Unclamp        cmydef.h:362-363
//    C_SLK2_Clamp / C_SLK2_Unclamp            cmydef.h:360-361 (values 55/56,
//                                 cmydef.cpp:418-419)
//    Sen[]                        mysensor.h:48; .IsOn() :42
//    SnSocketClampPush1/2         cmydef.h:1048-1049
//    SnSocketHasClamp1/2          cmydef.h:1052-1053
//    BTestSuck                    aHotPlateSubstrate.h:637, object
//                                 aHotPlateSubstrate.cpp:93; iShtRow :406,
//                                 iShtCol :438, Suck[][] :369,
//                                 TMySucker::Normal() :130
//    MOT[] / Gali_MotMove         Motor/mymotor.h:201
//    MTestZ2 / MTestY2 / MMIndex  cmydef.h:2116 / :2117 / :2401
//    iSpeedSlow                   cmydef.h:3293
//    Prod.TestZ2_Test             cprod.h:436
//    TestIF_File.bUseSLKClamp / .iSeparabilityTest   cprod.h:2175 / :2176
//    ShowErrorMessage / K_RETRY   canary_support.h:66 / cmydef.h -- both already
//                                 used live in this TU (aTester_Front.cpp:592)
//
//  PREREQUISITE, NOT MINE TO EMIT (flagged, not smuggled).  This SM needs the
//  three file-scope clamp timers golden declares BETWEEN functions at golden
//  aTester_Front.cpp:94-96 --
//      TQPF_Timer FTestSocketClampCloseDelay;   //JerryYang 20160526
//      TQPF_Timer FTestSocketClampOpenDelay;    //JerryYang 20160526
//      TQPF_Timer FTestSocketClampTimeOutDelay; //JerryYang 20160526
//  They fall inside the golden :94-:98 block that the MAIN LOOP owns and adds in
//  ONE place, so this part deliberately does NOT define them (two agents
//  defining them is a `multiple definition` link error).  They are absent from
//  the port today, i.e. WITHOUT the main loop's block this part does not link:
//    rg -n "FTestSocketClampCloseDelay|FTestSocketClampOpenDelay|FTestSocketClam
//    pTimeOutDelay" --glob '*.h' --glob '*.cpp' -g '!build*' .
//      -> exit 1, no output.   MEASURED AT: 2026-08-10 23:44:41 +0800.
//  (The rear twin needed the same three on its side and part 2 of the k7 trio
//  emitted them itself, port aTester_Rear.cpp:11849-11851 -- those are the B*
//  names and do NOT collide with these F* names.)
//
//  TRAP 1 (archive extraction) -- FTestSeparateSLK below has EXTERNAL linkage;
//  no `static`.  atester_shims.h:108 declares it as
//  `bool FTestSeparateSLK(bool bReset=false);` -- THE DEFAULT ARGUMENT LIVES IN
//  THAT DECLARATION and is deliberately NOT repeated here (repeating it is a
//  hard compile error once both are visible in one TU, and atester_shims.h is
//  included by aTester_Front.cpp:103).  INTEGRATE MUST DELETE the no-op stub
//  `bool FTestSeparateSLK(bool) { return true; }` at atester_shims.cpp:184 --
//  otherwise TRAP-1 shape (b): the stub keeps answering all three live call
//  sites in aTester_Rear.cpp, this body is never extracted, and the SM silently
//  reports "separate done" on the very first poll.  On a real machine that means
//  the test fires with the Socket clamp never closed and vacuum still holding.
//
//  TRAP 4 (static init) -- this part adds NO file-scope object.  (The three
//  TQPF_Timer objects it needs are the main loop's, see PREREQUISITE; for the
//  record their ctor -- myTimer.cpp -- touches only its own members and a Win32
//  API, and dereferences no global pointer.)  This part has no function-scope
//  statics either.
//
//  TRAP 5 (two headers, same class name) -- BTestSuck is the
//  aHotPlateSubstrate.h:365 TMyKitSuck, NOT mykitsuck.h:274's.  Object defined
//  at aHotPlateSubstrate.cpp:93 (`TMyKitSuck BTestSuck;`), declared
//  aHotPlateSubstrate.h:637; aTester_Front.cpp:115 already includes that header.
//  iShtRow / iShtCol / Suck[][] and TMySucker::Normal() were each checked against
//  aHotPlateSubstrate.h, not against mykitsuck.h -- picking the other header links
//  clean and reads every field at the wrong offset.
//
//  SOFT_SIMULTE -- NOT defined in this target build (aTester_Front.cpp:26-28
//  states the file-top rule).  Both `#ifdef SOFT_SIMULTE` bodies below are
//  therefore inert and are reproduced VERBATIM, and the
//  `#ifndef SOFT_SIMULTE / int ret; / #endif` declaration at golden :8114-8116
//  IS live -- `ret` really is declared, so the two ShowErrorMessage assignments
//  compile.
//
//  FIDELITY NOTES: golden re-arms FTestSocketClampCloseDelay/TimeOutDelay AFTER
//  setting Task in case 1 (Task=10 comes first) and BEFORE setting it in case 20
//  (Task=30 comes last) -- statement order preserved exactly, both ways.  The
//  literals 300 / 500 / 2.5 / 1500 and the target Prod.TestZ2_Test+1500 are
//  copied verbatim; no arithmetic was rewritten and no int/int was promoted to
//  floating point anywhere in this part.  Note case 30 uses .OnStatus() while
//  the Combine twin (part 3) uses .OnSensor() on the same cylinder -- golden's
//  own inconsistency, preserved.  Golden case 40 has NO `break;` (it is the last
//  label before the closing brace of the switch) -- preserved.
// ==========================================================================
//------------------------------------------------------------------------------
// STITCH REQUIREMENT (this part and part 3 of the trio): the SLK state machines
// deref Sen[] (mysensor.h:48) and Cylinder[] (mycylin.h:176), and NEITHER header
// is in aTester_Front.cpp's current include block (aTester_Front.cpp:101-118),
// and the file has ZERO occurrences of Sen[ or Cylinder[ today, and none of the
// ELEVEN pre-existing sibling _w7c_parts fragments uses them either -- so this
// trio is the only reason to add them.  Commands, run from
// D:/HT9045/HT9011UC_Cpp_V3.33.906.0, MEASURED AT 2026-08-10 23:49:24 +0800:
//     rg -n '#include "mysensor.h"|#include "mycylin.h"' aTester_Front.cpp
//       -> exit 1, no output
//     rg -c "Sen\[|Cylinder\[" aTester_Front.cpp
//       -> exit 1, no output
//     rg -c "Sen\[|Cylinder\[" _w7c_parts/
//       -> exit 0, hits ONLY in this trio's own two files (08110: 13,
//          08201: 12); the other eleven fragments have none
// These two lines are NOT an invention: GOLDEN aTester_Front.cpp includes both,
// at golden :10 ("mycylin.h") and golden :15 ("mysensor.h"), in its own file-top
// block -- so hoisting them to the top restores golden's own arrangement.  Both
// carry include guards (mysensorH, mycylinH), so these two lines are idempotent.
// Exact precedent, already shipped and building: port aTester_Rear.cpp
// :11713-11714 (same two headers, mid-file, for the mirror trio) and
// aTester_Rear.cpp:173-174 (same two at file top).
//
// TRAP 2 -- THIS ABSENCE-CLAIM ALREADY EXPIRED ONCE, INSIDE THIS WAVE, AND THE
// FIX IS NOT OPTIONAL.  Re-measured 2026-08-10 23:50:06 +0800, i.e. 42 s after
// the run above, a sibling fragment had landed:
//     rg -c "Sen\[|Cylinder\[" _w7c_parts/
//       -> _w7c_parts/07036_DoTestYFront_c3.txt:7   (NEW)
//     rg -n "^#include" _w7c_parts/07036_DoTestYFront_c3.txt
//       -> exit 1, no output
// So that sibling derefs Sen[]/Cylinder[] SEVEN times and emits NO include of
// its own, and 07036 sorts BEFORE 08110.  If the stitch appends fragments in
// filename order and these two #include lines are left HERE, the sibling's uses
// land ABOVE the includes and the TU does not compile.  Therefore: HOISTING the
// two lines into the file-top include block next to "Motor/mymotor.h" (and
// deleting them here) is REQUIRED, not a tidy-up.
// AI(pt-wave) 20260811: #include hoisted to the top of aTester_Front.cpp by the integrator -- a mid-file include lands in the middle of the stitched TU. Original: #include "mysensor.h"               // Sen[]      -- SnSocketClampPush1/2, SnSocketHasClamp1/2, SnSocketClampPull1/2
// AI(pt-wave) 20260811: #include hoisted to the top of aTester_Front.cpp by the integrator -- a mid-file include lands in the middle of the stitched TU. Original: #include "mycylin.h"                // Cylinder[] -- C_Socket_Clamp/Unclamp, C_SLK2_Clamp/Unclamp
//------------------------------------------------------------------------------
bool FTestSeparateSLK(bool bReset)                                              //JerryYang 20160429 分離模式, Z2 SLK要與Clamp分離,將IC放到Socket流程
{
    bool bResult=false;
    int &Task=iFTestSocketClampCloseTask;
    #ifndef SOFT_SIMULTE
    int ret;
    #endif
    if(bReset ||
       TestIF_File.bUseSLKClamp==false ||
       TestIF_File.iSeparabilityTest==0)
    {
        Task=1;
        return bResult;
    }

    switch(Task)
    {
        case 1:
            {
                Cylinder[C_Socket_Unclamp].Off();
                Cylinder[C_Socket_Clamp].On();                                  //Z2已經下降到測試高度,Socket clamp夾持
                Task=10;
                FTestSocketClampCloseDelay.SetMSAndOn(300);
                FTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
            }
            break;
        case 10:
            if(FTestSocketClampCloseDelay.Off())
            {
                #ifdef SOFT_SIMULTE
                    Task=20;
                #else
                if(Sen[SnSocketClampPush1].IsOn()==true && Sen[SnSocketClampPush2].IsOn()==true &&
                   Sen[SnSocketHasClamp1].IsOn()==true && Sen[SnSocketHasClamp2].IsOn()==true)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            BTestSuck.Suck[i][j].Normal();                      //破真空將IC放到SOCKET上
                        }
                    }
                    Task=20;
                }
                else
                {
                    if(FTestSocketClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0375", K_RETRY, MMIndex);      //socket clamp異常
                        if(ret==K_RETRY)
                        {
                            Task=1;
                        }
                    }
                }
                #endif
            }
            break;
        case 20:                                                                // Socket clamp夾持後,SLK2 放開clamp
            Cylinder[C_SLK2_Clamp].Off();
            Cylinder[C_SLK2_Unclamp].On();
            FTestSocketClampCloseDelay.SetMSAndOn(500);
            FTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
            Task=30;
            break;
        case 30:
            if(FTestSocketClampCloseDelay.Off())
            {
                #ifdef SOFT_SIMULTE
                    Task=40;
                #else
                if(Cylinder[C_SLK2_Unclamp].OnStatus())
                    Task=40;
                if(FTestSocketClampTimeOutDelay.Off())
                {
                    ret=ShowErrorMessage("JAM0374", K_RETRY, MTestY2);          //ARM2 SLK放開clamp異常
                    if(ret==K_RETRY)
                    {
                        Task=20;
                    }
                }
                #endif
            }
            break;
        case 40:                                                                //Arm 2上升5mm再測試
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+1500, iSpeedSlow))
                bResult=true;
    }
    return bResult;
}

// ==========================================================================
//  PART FILE  _w7c_parts/08201_FTestCombineSLK.txt   (label k6-SLK-trio, 3 of 3)
//  Stitch target: aTester_Front.cpp -- APPEND after part 2 of this trio (part 2
//  carries the mysensor.h / mycylin.h includes this part also needs).
//  Translator: AI(k6-SLK-trio) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 8201..8437 -- FTestCombineSLK(bool) and
//  nothing else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD; Chinese comments
//  transcribed character for character from cp950.
//
//  ROLE
//  ----
//  FTestCombineSLK(bool bReset): the mirror of FTestSeparateSLK.  Test has
//  finished and the ICs are standing in the sockets; this SM releases SLK2,
//  brings Arm2 back down to test height, re-clamps SLK2 onto the ICs, opens the
//  Socket clamp, then sucks every ARM-2 site that was in use back onto the
//  nozzles (per-site latches bBTSuckFinish / bBTestSuckUse) and returns true.
//  Two early-outs return true without sucking: Task=10 when Arm2 carried no IC
//  at all, and Task=80 in the D58 "Arm1 places / Arm2 tests" mode.
//
//  WAVE SCOPE
//  ----------
//    * FTestCombineSLK(bool)             golden :8201-8437   ACTIVE
//
//  GOLDEN CROSS-NAMING -- as in part 2: the F-named function drives the REAR
//  objects (BTestSuck, C_SLK2_*, MOT[MTestZ2], Prod.TestZ2_Test, MTestY2), while
//  the mirror BTestCombineSLK (port aTester_Rear.cpp:12009) drives FTestSuck /
//  C_SLK1_* / MTestZ1.  Deliberate; do not "correct".
//
//  FOUR ASYMMETRIES vs THE REAR TWIN -- ALL DELIBERATE, ALL PRESERVED.  Read
//  before diffing this against port aTester_Rear.cpp:12009-12224:
//    (1) EARLY-OUT CLAUSE COUNT.  Rear's reset test has a fourth clause
//        (IniConfig.bD58UseArm1PickPlaceArm2Test && TestIF_File
//        .bArm1PickPlaceArm2Test).  Golden's Front version at :8219-8221 has
//        only three.  On the Front side the D58 mode is handled INSIDE the SM
//        instead -- see (2) and (3).
//    (2) case 1 has a D58 BRANCH the rear twin does not have (golden :8240-8248):
//        in Arm1-place/Arm2-test mode bSLK2NeedClamp is taken from the
//        bArm2NeedCombine latch and that latch is then consumed (set false),
//        INSTEAD of scanning the BTestSuck Item grid.  Consequence, preserved:
//        in that mode bBTestSuckUse[][] and bBTSuckFinish[][] are left at
//        whatever the previous cycle left them at -- the grid walk that would
//        have cleared bBTSuckFinish is in the `else` half.  Harmless only
//        because case 80 returns early in the same mode -- see (3).  This is
//        golden's shape, not a translation artefact.
//    (3) case 80 opens with a D58 early `return true;` (golden :8377-8381), so
//        in that mode the nozzles are never re-sucked here.  The rear twin has
//        no such block (its case 80 starts straight at bCheckAllSuck=true).
//    (4) case 40 and case 60 wrap their bodies in an extra brace block on the
//        Front side (golden :8313-8319, :8342-8349); the rear twin does not.
//        Cosmetic in golden, copied verbatim anyway.
//
//  GATE REGISTER -- EMPTY.  No #if 0 and no offline substitution.  Beyond part
//  2's verified symbol list, this part additionally needs (all verified
//  2026-08-10 23:44 +0800):
//    iFTestSocketClampOpenTask    atester_shims.cpp:165 (extern :h:88)
//    bArm2NeedCombine             atester_shims.cpp:166 (extern :h:89) -- golden
//                                 declares it at file scope, golden :77; it is
//                                 ALREADY in the port, so this part only USES it
//    FTestSocketClampOpenDelay    golden :95 -- the MAIN LOOP's, see part 2
//                                 PREREQUISITE; absent from the port today
//    Cylinder[].OnSensor()        mycylin.h:119
//    Sen[].IsOff()                mysensor.h:43
//    SnSocketClampPull1/2         cmydef.h:1050-1051
//    MOT[MTestZ2].Gali_ReadPos()  Motor/mymotor.h:214 (returns long; compared
//                                 against int Prod.TestZ2_Test -- golden's own
//                                 mixed comparison, kept, no cast added)
//    BTestSuck .iMaxRow / .iMaxCol / .Item[][] / .Suck[][].Suck() /
//                                 .Suck[][].Error   (aHotPlateSubstrate.h :377 /
//                                 :402 / :370, TMySucker::Suck() :119,
//                                 TMySucker::Error :109)
//    NULL_IC / HAS_NULL_IC        cmydef.h:156 / :162
//    MAX_SOCKET_ROW / MAX_SOCKET_COL   MachineType.h:391-392 == 4 and 8, which
//                                 exactly match both the 4x8 initialiser
//                                 literals golden writes AND
//                                 aHotPlateSubstrate.h's _MAX_SUCK_ROW_ITEM /
//                                 _MAX_SUCK_COL_ITEM (:102-103, also 4 and 8).
//                                 So the iMaxRow/iMaxCol reset walk at golden
//                                 :8224-8231 cannot overrun the two
//                                 MAX_SOCKET_*-dimensioned latch arrays.
//
//  TRAP 1 (archive extraction) -- FTestCombineSLK below has EXTERNAL linkage; no
//  `static`.  atester_shims.h:109 declares it as
//  `bool FTestCombineSLK(bool bReset=false);`; the default argument stays THERE
//  and is deliberately not repeated here.  INTEGRATE MUST DELETE the no-op stub
//  `bool FTestCombineSLK(bool) { return true; }` at atester_shims.cpp:185 --
//  otherwise TRAP-1 shape (b): the stub answers every reference, this body never
//  gets extracted, and on a real machine the arm leaves the socket without ever
//  sucking the tested ICs back up.
//    AND THE OTHER HALF OF TRAP 1, shape (a) -- THIS ONE IS LIVE RIGHT NOW.
//    FTestCombineSLK has ZERO call sites in the port at this moment:
//      rg -n "FTestCombineSLK" --glob '*.h' --glob '*.cpp' -g '!build*' .
//        -> exit 0, exactly TWO hits, both of them declarations of the thing to
//           be retired: atester_shims.h:109 and atester_shims.cpp:185 (the
//           stub).  No call site anywhere.  (The two hits for the substring
//           "CombineSLK" at aTester_Front.cpp:18 and :891 are banner prose, not
//           matches for this pattern.)
//      MEASURED AT: 2026-08-10 23:48:05 +0800.
//    Its two real callers are golden aTester_Front.cpp:5094 and :7912, i.e.
//    INSIDE DoTestYFront -- which a SIBLING agent is translating as chunks in
//    THIS SAME WAVE.  TRAP 2 IN THE FLESH: the claim above EXPIRED 99 SECONDS
//    AFTER IT WAS MEASURED.  Re-run at 2026-08-10 23:50:06 +0800:
//      rg -n "FTestCombineSLK|FTestSeparateSLK" _w7c_parts/07036_DoTestYFront_c3
//      .txt
//        -> :85   if(FTestSeparateSLK()==true)     (golden :7113)
//        -> :904  if(FTestCombineSLK()==true)      (golden :7912)
//    So the caller HAS landed, in _w7c_parts/07036_DoTestYFront_c3.txt, and the
//    good outcome is the actual outcome.  What integrate must still NOT do is
//    delete the stub, fail to stitch 07036, find no caller, and conclude the body
//    is dead: without that fragment this body has no reference at all and, once
//    aTester_Front.cpp is inside a static archive, is never extracted -- green
//    build, zero behaviour.  GATE CONDITION for integrate: re-run
//      rg -n "FTestCombineSLK" --glob '*.h' --glob '*.cpp' -g '!build*' .
//    on the STITCHED tree and require >= 1 call site outside atester_shims.*.
//
//  TRAP 4 (static init) -- the three statics below (bSLK2NeedClamp,
//  bBTSuckFinish[][], bBTestSuckUse[][]) are FUNCTION-scope statics of scalar
//  type with constant initialisers: zero dynamic initialisation, no ctor, no
//  global dereferenced before main().  This part adds NO file-scope object.
//
//  TRAP 5 (two headers, same class name) -- BTestSuck is the
//  aHotPlateSubstrate.h:365 TMyKitSuck (object aHotPlateSubstrate.cpp:93), not
//  mykitsuck.h:274's; every member listed in the GATE REGISTER above was checked
//  against aHotPlateSubstrate.h line by line.  mykitsuck.cpp stays unregistered
//  (CMakeLists.txt:2099-2106).
//
//  SOFT_SIMULTE -- NOT defined in this target build; the one
//  `#ifdef SOFT_SIMULTE` body (golden :8354-8356, case 70) is inert and
//  reproduced VERBATIM.  NOTE the asymmetry with part 2: this function declares
//  `int ret=0;` UNCONDITIONALLY (golden :8215), with no #ifndef wrapper.
//
//  FIDELITY NOTES: golden sets Task BEFORE re-arming
//  FTestSocketClampTimeOutDelay in case 40 and case 60 (Task=50 / Task=70 come
//  first) -- statement order preserved exactly.  Literals 500 / 2.5 and the
//  target Prod.TestZ2_Test are verbatim; no arithmetic rewritten, no int/int
//  promoted to floating point.  case 90 has NO `break;` (last label) --
//  preserved.  The `bBTestSuckUse` initialiser rows are written WITHOUT spaces
//  after the commas while `bBTSuckFinish` has them -- golden's own formatting,
//  copied byte for byte.
// ==========================================================================
//------------------------------------------------------------------------------
bool FTestCombineSLK(bool bReset)                                               //JerryYang 20160429 分離模式, 測試完成 Z2 SLK要與Clamp結合,再將IC吸起流程
{
    static bool bSLK2NeedClamp=false;
    static bool bBTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bBTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false,false,false,false,false,false,false,false},
                                                               {false,false,false,false,false,false,false,false},
                                                               {false,false,false,false,false,false,false,false},
                                                               {false,false,false,false,false,false,false,false}};

    int &Task=iFTestSocketClampOpenTask;
    int ret=0;
    bool bCheckAllSuck=false, flag=false;
    bool bResult=false;

    if(bReset ||
       TestIF_File.bUseSLKClamp==false ||
       TestIF_File.iSeparabilityTest==0)                                        //JerryYang 20180518 (wei) : 新增保護
    {
        Task=1;
        for(int i=0; i<BTestSuck.iMaxRow; i++)
        {
            for(int j=0; j<BTestSuck.iMaxCol; j++)
            {
                bBTestSuckUse[i][j]=false;
                bBTSuckFinish[i][j]=false;
            }
        }
        return bResult;
    }

    switch(Task)
    {
        case 1:
            {
                bSLK2NeedClamp=false;
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //JerryYang 20180518 (wei) : 分離式SLK ARM1丟IC ARM2分離CLAMP流程
                   TestIF_File.bArm1PickPlaceArm2Test==true)                    //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                {
                    if(bArm2NeedCombine)                                        //JerryYang 20180426 add
                        bSLK2NeedClamp=true;
                    else
                        bSLK2NeedClamp=false;
                    bArm2NeedCombine=false;
                }
                else
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Item[i][j]!=NULL_IC &&
                               BTestSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                BTestSuck.Suck[i][j].Error=false;
                                bBTestSuckUse[i][j]=true;
                                bSLK2NeedClamp=true;
                            }
                            bBTSuckFinish[i][j]=false;                          //Steven 20110301
                        }
                    }
                }
                Task=10;
            }
            break;
        case 10:
            if(bSLK2NeedClamp==true)
            {
                Task=15;                                                        //JerryYang 20160714 修正index上有已測IC, 回home後hang up問題
            }
            else
            {
                return true;
            }
            break;
        case 15:
            if(MOT[MTestZ2].Gali_ReadPos()==Prod.TestZ2_Test)                   //JerryYang 20160714 修正index上有已測IC, 回home後hang up問題
            {
                Task=30;
            }
            else
            {
                Cylinder[C_SLK2_Unclamp].On();
                Cylinder[C_SLK2_Clamp].Off();
                FTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
                Task=20;
            }
            break;
        case 20:                                                                //測試結束 Arm2下降前 確認clamp放開
            if(Cylinder[C_SLK2_Unclamp].OnSensor())
            {
                Task=30;
            }
            else if(FTestSocketClampTimeOutDelay.Off())
            {
                ret=ShowErrorMessage("JAM0374", K_RETRY, MTestY2);              //JerryYang 20160714 ARM2 SLK放開clamp異常
                if(ret==K_RETRY)
                {
                    Task=15;
                }
            }
            break;
        case 30:                                                                //測試完,Arm2要下降
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, iSpeedSlow))
            {
                Task=40;
            }
            break;
        case 40:                                                                //SLK Clamp 夾持
            {
                Cylinder[C_SLK2_Unclamp].Off();
                Cylinder[C_SLK2_Clamp].On();
                FTestSocketClampOpenDelay.SetMSAndOn(500);
                Task=50;
                FTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
            }
            break;
        case 50:
            if(FTestSocketClampOpenDelay.Off())
            {
                if(Cylinder[C_SLK2_Clamp].OnSensor())
                {
                    Task=60;
                }
                else
                {
                    if(FTestSocketClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0372", K_RETRY, MTestY2);      //Socket clamp未到位
                        if(ret==K_RETRY)
                        {
                            Task=40;
                        }
                    }
                }
            }
            break;
        case 60:
            {
                Cylinder[C_Socket_Clamp].Off();                                 //Socket Clamp 不夾
                Cylinder[C_Socket_Unclamp].On();
                Task=70;

                FTestSocketClampOpenDelay.SetMSAndOn(500);
                FTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
            }
            break;
        case 70:
            if(FTestSocketClampOpenDelay.Off())
            {
                #ifdef SOFT_SIMULTE
                    Task=80;
                #else
                if(Sen[SnSocketClampPull1].IsOn()==true && Sen[SnSocketClampPull2].IsOn()==true &&
                   Sen[SnSocketClampPush1].IsOff()==true && Sen[SnSocketClampPush2].IsOff()==true)
                {
                    Task=80;
                }
                else
                {
                    if(FTestSocketClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0376", K_RETRY, MMIndex);      //放開clamp異常
                        if(ret==K_RETRY)
                        {
                            Task=60;
                        }
                    }
                }
                #endif
            }
            break;
        case 80:                                                                //吸取IC
            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                  //JerryYang 20180518 (wei) : 分離式SLK ARM1丟IC ARM2分離CLAMP流程
               TestIF_File.bArm1PickPlaceArm2Test==true)                        //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
            {
                return true;
            }

            bCheckAllSuck=true;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bBTSuckFinish[i][j]==false)
                    {
                        if(bBTestSuckUse[i][j])
                        {
                            if(BTestSuck.Suck[i][j].Suck())
                            {
                                bBTSuckFinish[i][j]=true;
                                bBTestSuckUse[i][j]=false;
                            }
                            else if(BTestSuck.Suck[i][j].Error)
                            {
                                bBTSuckFinish[i][j]=true;
                            }
                            else
                            {
                                bCheckAllSuck=false;
                            }
                        }
                        else
                        {
                            bBTSuckFinish[i][j]=true;
                        }
                    }
                }
            }

            flag=true;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bBTSuckFinish[i][j]==false)
                        flag=false;
                }
            }

            if(flag)
            {
                if(bCheckAllSuck)
                {
                    Task=90;
                }
            }
            break;
        case 90:
            bResult=true;
    }

    return bResult;
}

// ==========================================================================
//  PART FILE  _w7c_parts/08439_DoArm1Suck.txt   (label k8-small-seven)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing content.  THIS
//  PART HAS A HARD STITCH-ORDER REQUIREMENT: see STITCH ORDER below.
//  Translator: AI(k8-small-seven) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 8439..8540 -- DoArm1Suck() and nothing else.
//  Emitted UTF-8 / 100% CRLF, zero U+FFFD; Chinese comments transcribed character
//  for character from cp950.
//
//  ROLE
//  ----
//  "Index arm 1 sucks + swaps state" helper (golden comment: JerryYang 20190123).
//  For every FRONT nozzle it reconciles the front-left carry kit (FLCarryKit, the
//  shuttle side) against the front test-head kit (FTestSuck): re-Normals nozzles
//  to stop vacuum droop, fires Suck() where an IC is waiting, MOVES the per-site
//  data from the carry kit into the test-head kit, logs the pick, bumps the Index
//  input counter and the per-nozzle picker-life counter, and marks
//  bArm1SuckFinish per site.  When CosFunction.bUseShuttlePickShiftDetect +
//  DeviceForm_File.dDropByPassDetect + bIndexZ1NeedUp are all set (ASEM: rise
//  first, then judge) the DATA MOVE is deliberately deferred (bNeedWaitUp==true)
//  while the finish/dup flags are still set -- preserved exactly.
//
//  WAVE SCOPE
//  ----------
//    DoArm1Suck()   ACTIVE   golden :8439-8540
//      (ONE statement inside it GATED: golden :8518 -- GATE REGISTER G-k8s7-2)
//    No file-scope global is emitted by this part at all.
//
//  STITCH ORDER REQUIREMENT (read this before placing the fragment)
//  ----------------------------------------------------------------
//  This body reads `bIndexZ1NeedUp`, which is a TU-LOCAL
//  `static bool bIndexZ1NeedUp=false;` -- static in GOLDEN TOO (golden
//  aTester_Front.cpp:66) and ALREADY PRESENT at port aTester_Front.cpp:958.  So
//  this fragment MUST be stitched AFTER port line 958 and must NOT define its own
//  copy.  "Append after the existing content" satisfies that.  It also reads
//  `bArm1SuckFinish[][]`, which is plain data at atester_shims.cpp:167 (extern
//  atester_shims.h:90) -- read/written only, never defined here.
//
//  EXPIRED GATES DELIBERATELY RETIRED HERE (TRAP 3 re-asked per symbol; the answer
//  for all five is "go ACTIVE", each verified in the port 2026-08-10 21:45 +0800)
//  ---------------------------------------------------------------------------
//    * golden :8483 / :8510  FTestSuck.MoveSuckData(FLCarryKit, i, j) -- the REAL
//      method exists: `void TMyKitSuck::MoveSuckData(class TMyKitSuck &Source,
//      int SourceR, int SourceC, int TargetR=-1, int TargetC=-1)` declared
//      aHotPlateSubstrate.h:598.  Its -1/-1 default path resolves the target to
//      the SAME row/col, which is exactly what golden's 3-arg form means, so
//      calling it directly is MORE faithful than reusing this file's older no-op
//      W64B_MoveSuckData (port aTester_Front.cpp:1032, which silently DROPPED the
//      move).  I do NOT touch the older call sites that still use that stub --
//      that is an expired-gate cleanup for the main loop, not my file to edit.
//      Same decision the Rear twin made (port aTester_Rear.cpp:12385/12412).
//    * golden :8511  FTestSuck.PordRec[i][j].AddIndexPickShuttleRecord(
//      TestIF.iSiteMap[i+iNN][j], i+iNN, j) -- REAL: declared
//      Public/MyProductionRecord.h:108, DEFINED Public/MyProductionRecord.cpp:577
//      (3 args, exact match).  The i+iNN site indexing is golden's own (the Rear
//      twin uses plain i because NN mode only shifts the front grid) and the SAME
//      i+iNN shape is already ACTIVE in this file at port line 497 for the
//      sibling AddIndexPlaceShuttleRecord -- so this is consistent, not novel.
//    * golden :8515  LastSet.iIndexInputOutPut[0]++ -- REAL: the canary_support.h
//      LAST_GENERAL_SET minimal shim was RETIRED (canary_support.h:59 now simply
//      `#include "LastSet.h"`), the field is `long iIndexInputOutPut[4]` at
//      LastSet.h:433, and the object is defined at LastSet.cpp:39.
//      MAIN-LOOP HAND-OFF: the SAME golden counter is STILL GATED in this very
//      file at port aTester_Front.cpp:477-478 (`#if 0 // TODO(W7) ... LAST_GENERAL_SET
//      minimal shim ... has no such field yet`) and its premise is now DEAD.  That
//      is an EXPIRED GATE, but it is not in my range and I did not touch it --
//      reported instead.  (The Rear twin reported the identical expired gate at
//      port aTester_Rear.cpp:584-586.)
//    * golden :8516  TestIF_File.Arm1PickerLifeCnt[i][j]++ -- REAL:
//      `int Arm1PickerLifeCnt[MAX_Index_Row][MAX_Index_Col]` cprod.h:2473, and the
//      field is already read live at csystem.cpp:10968/12884/12886.
//    * golden :8517  CheckPickerLifeNeedOneCycle() -- REAL body csystem.cpp:15747
//      with NO enclosing #if 0, declared cmydef.h:5824.  TRAP 1 (c) checked, not
//      assumed: csystem.cpp and aTester_Front.cpp are in the SAME static archive
//      (CMakeLists.txt:1828 aTester_Front.cpp alongside aTester_Rear.cpp:1829 and
//      csystem.cpp), so this is a genuine intra-archive resolution.  There is no
//      rival global stub: the only other definition in the tree is aoutarm.cpp:574
//      `static void PTW4_CheckPickerLifeNeedOneCycle(){}` behind a TU-LOCAL
//      `#define CheckPickerLifeNeedOneCycle` -- internal linkage in a different TU,
//      so it can neither satisfy nor shadow my call (that seam is a separate
//      main-loop cleanup).
//
//  GATE REGISTER (the single #if 0 in this fragment)
//  ------------------------------------------------
//  G-k8s7-2   <- AI(W906-W2-TRAP5) 20260926: LIFTED (see :11839)
//    GOLDEN LINE      :8518   FTestSuck.PordRec[i][j].AddIndexPickVacuum(
//                             FTestSuck.Suck[i][j].GetIOValue())
//                             //Sam 20230210 : 新增 VacuumUnit 通訊模組
//    WHY THE OFFLINE DEFAULT IS FAITHFUL
//                     This statement is pure TELEMETRY: it writes the measured
//                     vacuum reading of the just-completed pick into the per-site
//                     production-record string buffer.  Nothing in DoArm1Suck, and
//                     nothing in the front index state machine, reads that field
//                     back -- control flow, IC state, the two duplicate-error
//                     latches, bArm1SuckFinish, the Index input counter and the
//                     picker-life counter are ALL untouched by it and all remain
//                     ACTIVE around the gate.  Skipping it cannot change any
//                     decision the SM makes offline.
//    REAL-MACHINE DIFFERENCE
//                     The per-IC production log row for a FRONT index pick will
//                     have an EMPTY vacuum-value column, so downstream yield /
//                     vacuum analysis of front picks loses that one measurement
//                     per IC.  No motion, no interlock, no alarm behaviour changes.
//    TWO INDEPENDENT ABSENCES -- BOTH must be fixed before un-gating:
//                     (1) TMyProductionRecord::AddIndexPickVacuum is not even
//                         DECLARED anywhere -- Public/MyProductionRecord.cpp:19
//                         records it as deliberately left out, and there is no
//                         definition in the tree.
//                     (2) TMySucker::GetIOValue() does not exist on the TMySucker
//                         layout THIS TU uses.  THIS IS TRAP 5, NOT a plain
//                         absence: GetIOValue IS real at mykitsuck.cpp:2902
//                         (declared mykitsuck.h:237), but it belongs to
//                         MYKITSUCK.H's TMySucker, whereas FTestSuck.Suck[][] is
//                         the aHotPlateSubstrate.h:106 TMySucker -- a DIFFERENT
//                         class with a different layout, whose member list has
//                         Suck/Destroy/On/Off/OnSuck/OnDestroy/OffDestroy/Normal/
//                         GetStatus but NO GetIOValue.  Reaching for the
//                         mykitsuck.h one would LINK and then read wrong offsets.
//    ABSENCE COMMANDS (TRAP 2 -- re-run BOTH at integration; both must still come
//                     back with no real declaration):
//                       Grep(pattern="Arm1PickerLifeCnt|CheckPickerLifeNeedOneCycle|AddIndexPickShuttleRecord|AddIndexPickVacuum|GetIOValue",
//                            path=D:/HT9045/HT9011UC_Cpp_V3.33.906.0, glob=*.{cpp,h})
//                       Grep(pattern="bNeedCheck|MoveSuckData|...",
//                            path=D:/HT9045/HT9011UC_Cpp_V3.33.906.0, glob=*.h)
//                     RESULT at 2026-08-10 21:45 +0800: every AddIndexPickVacuum
//                     hit is a comment or atester_32Site.cpp's own
//                     W5_32S_ADD_INDEX_PICK_VACUUM no-op macro (:233/:4448/:4499)
//                     or the Rear twin's gate text -- ZERO declarations.  Every
//                     GetIOValue hit is mykitsuck.* / MyLaneIo.* / a gated
//                     MyProductionRecord.cpp call site -- none on the
//                     aHotPlateSubstrate.h TMySucker.
//    MEASURED AT      2026-08-10 21:45 +0800
//    WHY IT SHOULD STAY GATED (TRAP 3, re-asked)
//                     Even if AddIndexPickVacuum lands, this gate must STAY until
//                     the aHotPlateSubstrate.h TMySucker also gains GetIOValue --
//                     and the tempting "fix" (include mykitsuck.h instead) is
//                     exactly the wrong answer, because 177 TUs including this one
//                     are built against the aHotPlateSubstrate layout.  Un-gate
//                     together with atester_32Site.cpp:4448 and the Rear twin's
//                     port aTester_Rear.cpp:12420-12422, never alone.
//
//  SYMMETRIC TWIN and where the Front genuinely DIFFERS (do not assume mirror)
//  ---------------------------------------------------------------------------
//  Twin = DoArm2Suck, golden aTester_Rear.cpp:9646-9746, port
//  aTester_Rear.cpp:12342-12444 (PT-W7b, label k9-small-six).  Its single gate
//  premise (AddIndexPickVacuum + GetIOValue) HOLDS unchanged here -- re-verified
//  above, not assumed.  FIVE golden differences carried faithfully:
//    (a) Front declares `int iNN=IsNNMode();` (golden :8441) and uses it in the
//        AddIndexPickShuttleRecord site index; the Rear golden has no iNN at all.
//    (b) Front's outer test is `if(FLCarryKit.Item[i][j] &&
//        bArm1SuckFinish[i][j]==false)` -- the finish flag is LIVE in the
//        condition.  Rear golden has that half commented out
//        (`if(BLCarryKit.Item[i][j])   // && bArm2SuckFinish[i][j]==false)`).
//    (c) Front's second arm is a bare `else`; Rear's is
//        `else if(BLCarryKit.Item[i][j]!=NULL_IC)`.  So on the Front side a
//        carry-kit slot holding NULL_IC still enters the suck arm.  KEPT.
//    (d) Front's SOFT_SIMULTE block (golden :8459-8465) is ENTIRELY COMMENTED OUT
//        inside the #ifdef -- dead text in golden.  Rear's is live code inside the
//        #ifdef.  Front's five comment lines are reproduced VERBATIM as comments.
//    (e) Front uses bArm1DuplicateErr / bArm1SuckComplete / Arm1PickerLifeCnt and
//        the golden comment dates differ (kevin 20130125 vs Rear's 20120531).
//
//  GOLDEN BUGS / ODDITIES PRESERVED, NOT FIXED
//  -------------------------------------------
//    * :8477-8478 the inner test `if(FLCarryKit.Item[i][j]==HAS_NULL_IC && ...)`
//      re-tests a condition the enclosing `if` at :8469 already guaranteed, which
//      makes the `else if(FLCarryKit.Item[i][j]==HAS_NULL_IC)` at :8486 reachable
//      ONLY when FTestSuck.Item[i][j]!=NULL_IC -- and makes the final bare `else`
//      at :8493 (with its golden comment "蝦頭有料, index也有料, 怪怪的") DEAD
//      CODE, since the :8486 test can never be false there.  KEPT VERBATIM.
//    * :8512-8513 and :8520-8521 set bArm1DuplicateErr=false / bArm1SuckFinish=true
//      TWICE -- once inside `if(bNeedWaitUp==false)` and once after it.  Redundant
//      on the bNeedWaitUp==false path.  GOLDEN'S OWN.  KEPT VERBATIM.
//    * :8510 is indented ONE SPACE LESS than :8483 for the same MoveSuckData call
//      (27 vs 28 leading spaces).  Cosmetic golden inconsistency, reproduced
//      byte-for-byte rather than tidied.
//  INTEGER DIVISION -- none in this function.  No int/int anywhere, no float
//    helper introduced; `i+iNN` is integer addition only.
//  TRAP 1 -- NOT `static`: golden aTester_Front.h:26 declares
//    `extern void DoArm1Suck();` and the port declares it non-static at
//    atester_shims.h:101, so `static` would be shape (d).  Live port callers this
//    body will serve (so not shape (a)): aTester_Front.cpp:1653 (inside the
//    already-translated DoFrontTestSuckIC), csystem.cpp:1057 and csystem.cpp:2699.
//  TRAP 4 -- adds no file-scope object of any kind, so nothing new runs before
//    main().
//  TRAP 5 -- WHICH HEADER, stated explicitly: FTestSuck and FLCarryKit are the
//    aHotPlateSubstrate.h TMyKitSuck (externs aHotPlateSubstrate.h:636 and :625),
//    with the OBJECTS DEFINED at aHotPlateSubstrate.cpp:92 and
//    aHotPlateSubstrate.cpp:81.  Their Suck[][] elements are the
//    aHotPlateSubstrate.h:106 TMySucker.  The rival mykitsuck.h declarations
//    (mykitsuck.h:458 / :453) with objects at mykitsuck.cpp:211 / :206 have a
//    DIFFERENT LAYOUT and are NOT used: port aTester_Front.cpp includes
//    aHotPlateSubstrate.h (line 115) and never mykitsuck.h.  This is also exactly
//    why gate G-k8s7-2's GetIOValue half cannot be "fixed" by an include swap.
//  INTEGRATE     -- retires the no-op stub atester_shims.cpp:177
//    `void DoArm1Suck() {}`.  That stub is a SERIOUS behaviour change, not
//    cosmetic: with it the front index arm never sucked, never moved carry-kit
//    data into the test-head kit, and never marked bArm1SuckFinish -- so its three
//    live callers were pumping a no-op.  Keep the non-static declaration
//    atester_shims.h:101.  bArm1SuckFinish[][] (atester_shims.cpp:167) is plain
//    data and STAYS -- this fragment only reads/writes it.
// ==========================================================================
void DoArm1Suck()                                                               //JerryYang 20190123 把index arm吸真空&交換狀態包成函式
{
    int iNN=IsNNMode();
    bool bNeedWaitUp=false;

    if(CosFunction.bUseShuttlePickShiftDetect==true &&
       DeviceForm_File.dDropByPassDetect!=0 &&
       bIndexZ1NeedUp==true)                                                    //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
    {
        bNeedWaitUp=true;
    }
    else
    {
        bNeedWaitUp=false;
    }

    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            #ifdef SOFT_SIMULTE
//            if(fMain->cbIndexDrop->Checked==true)
//            {
//                bArm1SuckFinish[0][1]=true;
//                FTestSuck.Suck[0][1].Error=true;
//            }
            #endif

            if(FLCarryKit.Item[i][j] && bArm1SuckFinish[i][j]==false)
            {
                if(FLCarryKit.Item[i][j]==HAS_NULL_IC)
                {
                    if(FTestSuck.Item[i][j]==HAS_NULL_IC ||
                       FTestSuck.Item[i][j]==NULL_IC)                           //Steven 20111202 : Retry會掉料
                    {
                        FTestSuck.Suck[i][j].Normal();                          //Steven 20111201 : 預防負壓壓降
                    }

                    if(FLCarryKit.Item[i][j]==HAS_NULL_IC &&
                       FTestSuck.Item[i][j]==NULL_IC)                           //Steven 20160817 : 移到上面
                    {
                        FTestSuck.Suck[i][j].Normal();
                        if(bNeedWaitUp==false)
                        {
                            FTestSuck.MoveSuckData(FLCarryKit, i, j);
                        }
                    }
                    else if(FLCarryKit.Item[i][j]==HAS_NULL_IC)                 //Index有料, 蝦頭沒料, 這個怪怪的
                    {
                        if(bNeedWaitUp==false)
                        {
                            FLCarryKit.SetItemData(i, j, NULL_IC);
                        }
                    }
                    else                                                        //蝦頭有料, index也有料, 怪怪的
                    {
                    }

                    bArm1SuckFinish[i][j]=true;                                 //Steven 20110301
                }
                else
                {
                    if(FTestSuck.Suck[i][j].Error)                              //Steven 20110301 : 有錯誤的不做
                    {
                        bArm1SuckFinish[i][j]=true;
                    }
                    else if(FTestSuck.Item[i][j]==NULL_IC &&                    //Steven 20160817 : 不能吸了又吸
                            FTestSuck.Suck[i][j].Suck())
                    {
                        if(bNeedWaitUp==false)
                        {
                           FTestSuck.MoveSuckData(FLCarryKit, i, j);
                           FTestSuck.PordRec[i][j].AddIndexPickShuttleRecord(TestIF.iSiteMap[i+iNN][j], i+iNN, j);      //Sam 20201216 : Add record
                           bArm1DuplicateErr[i][j]=false;                       //Steven 20100105
                           bArm1SuckFinish[i][j]=true;                          //Steven 20110301 : 吸取完成的不做
                           if(bRunAutoClean==false)                             //kevin 20130125
                               LastSet.iIndexInputOutPut[0]++;                  //kevin 20130125 Index 吸取IC計數
                           TestIF_File.Arm1PickerLifeCnt[i][j]++;               //JerryYang 20220923 : add
                           CheckPickerLifeNeedOneCycle();                       //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化
                           //AI(W906-W2-TRAP5) 20260926: gate LIFTED -- A4-6 (8ff6c754) merged the two TMyKitSuck/TMySucker classes into the one golden layout, so the TRAP-5 premise is dead: TMySucker::GetIOValue() (mykitsuck.h:237, mykitsuck.cpp) and TMyProductionRecord::AddIndexPickVacuum(double) (Public/MyProductionRecord.h:188, MyProductionRecord.cpp) are both defined (nm). GetIOValue reads two 1203 DI bytes via MyLaneIO (no comms wait); without HAVE_PCI1203 it returns 999.0 -- telemetry only -- golden :8518
                           FTestSuck.PordRec[i][j].AddIndexPickVacuum(FTestSuck.Suck[i][j].GetIOValue());               //Sam 20230210 : 新增 VacuumUnit 通訊模組
                           //AI(W906-W2-TRAP5) 20260926: (end of lifted gate)
                        }
                        bArm1DuplicateErr[i][j]=false;                          //Steven 20100105
                        bArm1SuckFinish[i][j]=true;                             //Steven 20110301 : 吸取完成的不做
                    }
                    else
                    {
                        bArm1SuckComplete=false;                                //jou 2011-08-16 只要有未完成的就繼續等
                    }
                }
            }
            else
            {
                if(FTestSuck.Item[i][j]==HAS_NULL_IC ||
                   FTestSuck.Item[i][j]==NULL_IC)                               //Steven 20111202 : Retry會掉料
                {
                    FTestSuck.Suck[i][j].Normal();                              //Steven 20111201 : 預防負壓壓降
                }
                bArm1SuckFinish[i][j]=true;                                     //Steven 20110301 : 沒有東西的地方要跳過
            }
        }
    }
}

// ==========================================================================
//  PART FILE  _w7c_parts/08542_DoArm1D44VacCheck.txt   (label k8-small-seven)
//  Stitch target: aTester_Front.cpp -- APPEND after the existing content.  HARD
//  STITCH-ORDER REQUIREMENT: see STITCH ORDER below.
//  Translator: AI(k8-small-seven) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Front.cpp (8,581
//  lines, cp950, 100% CRLF), lines 8542..8579 -- DoArm1D44VacCheck() and nothing
//  else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD; Chinese comments transcribed
//  character for character from cp950.
//
//  ROLE          -- "Index arm 1 suck-back (D44) vacuum re-check" helper (golden
//                   comment: JerryYang 20190123).  On negative-pressure heads
//                   (INDEX_SUCKER_TYPE==1) it first asks the IO-set view to run the
//                   suck-back detection, then -- once the front destroy delay has
//                   expired AND the check passed -- walks the FRONT nozzles: any
//                   nozzle latched as "needs re-check" whose vacuum sensor has
//                   already let go is un-latched and returned to Normal; and on
//                   INDEX_SUCKER_TYPE==1 every nozzle is returned to Normal
//                   unconditionally (golden comment: negative pressure must not
//                   stay on or all sites drop).
//  WAVE SCOPE    -- DoArm1D44VacCheck()   ACTIVE   golden :8542-8579
//                   (THREE statements routed through this file's OWN existing
//                   Wave-1/Wave-2 gap seams: golden :8547, :8560, :8567)
//                   No file-scope global is emitted by this part.
//
//  STITCH ORDER REQUIREMENT -- this fragment REUSES seams that already exist in
//                   port aTester_Front.cpp rather than duplicating them:
//                     * W64B_FIOSET_PISD1(iType)  port aTester_Front.cpp:163-164
//                     * W64B_NEEDCHECK_GET/SET    port aTester_Front.cpp:150-156
//                   so it must be stitched AFTER port line 164.  "Append after the
//                   existing content" satisfies that.  Reusing them (instead of
//                   minting a k8s7_ twin) is deliberate: this file already routes
//                   the SAME two golden gaps through them at port lines 712 and
//                   (bNeedCheck) inside DoFrontTestDestroyIC, so one seam per gap
//                   per TU keeps the integrate step a single edit.
//
//  REAL SYMBOLS verified in the port 2026-08-10 21:45 +0800 (not gated, not
//  substituted):
//                     * INDEX_SUCKER_TYPE        cmydef.h (used live at port
//                                                aTester_Front.cpp:711-719)
//                     * bArm1D44SuckCheck        cmydef.h:4823 / cmydef.cpp:4879
//                     * DoFrontTestDestroyICDelay cmydef.h:4821 / cmydef.cpp:4877
//                                                -- a REAL TQPF_Timer already used
//                                                by the translated
//                                                DoFrontTestDestroyIC (port lines
//                                                670 / 719 / 809)
//                     * FTestSuck.iShtRow / iShtCol / Suck and
//                       TMySucker::GetStatus / Normal   aHotPlateSubstrate.h
//
//  GATE REGISTER (two entries; no NEW #if 0 was needed because this file already
//  owns compiling stand-ins -- but each carries exactly the same burden)
//  ---------------------------------------------------------------------------
//  G-k8s7-PISD1
//    GOLDEN LINE      :8547   bArm1D44SuckCheck=
//                             fiosetview->ProcessIndexSuckDestroy1(1);
//                             //Sam 20241225 : 修正回吸偵測
//                     routed to W64B_FIOSET_PISD1(1) -> the TU-local
//                     `static bool W64B_ProcessIndexSuckDestroy1(int){ return true; }`
//                     at port aTester_Front.cpp:163.
//    WHY IT MUST STAY GATED (TRAP 3 answer, re-asked)
//                     TfiosetviewShim (atester_shims.h) exposes bIndexSuck[][][]
//                     but has NO ProcessIndexSuckDestroy1 method at all, and the
//                     golden owner (iosetview.h:3018) belongs to the untranslated
//                     IO-set-view FORM.  Retiring this seam therefore means
//                     translating that form, not adding one method.  It must also
//                     retire together with port aTester_Front.cpp:712 (the same
//                     golden call already routed the same way in this file) and
//                     atester.cpp's / atester_32Site.cpp's twins -- never alone,
//                     or the same call is live in one place and dead in three.
//    WHY THE OFFLINE DEFAULT IS FAITHFUL
//                     `true` means "suck-back self-check passed", which makes the
//                     INDEX_SUCKER_TYPE==1 path behave like the
//                     INDEX_SUCKER_TYPE!=1 path that GOLDEN ITSELF hard-codes to
//                     `bArm1D44SuckCheck=true` five lines later (golden :8551) --
//                     i.e. the offline value is one golden branch verbatim, not an
//                     invented value.  Note golden :8546 already writes
//                     bArm1D44SuckCheck=false one line above and :8547 immediately
//                     overwrites it: that redundancy is GOLDEN'S OWN and is
//                     preserved, not tidied (the Rear tree records the identical
//                     quirk at port aTester_Rear.cpp:10653).
//    REAL-MACHINE DIFFERENCE
//                     A genuine suck-back result (IC still stuck to the nozzle) is
//                     replaced by "clean", so the nozzle-Normal sweep below runs on
//                     the first pass instead of waiting for the detector -- on a
//                     real negative-pressure head that is the difference between
//                     releasing a stuck IC deliberately and releasing it blind.
//    ABSENCE COMMAND  Grep(pattern="ProcessIndexSuckDestroy",
//                          path=D:/HT9045/HT9011UC_Cpp_V3.33.906.0/atester_shims.h)
//                     must stay EMPTY.  RE-RUN AT INTEGRATION (TRAP 2).
//    MEASURED AT      2026-08-10 21:47 +0800
//  ---------------------------------------------------------------------------
//  G-k8s7-NEEDCHECK
//    GOLDEN LINES     :8560 (read) `if(FTestSuck.bNeedCheck[i][j])` //確認真空狀態
//                     :8567 (clear) `FTestSuck.bNeedCheck[i][j]=false;`
//                     routed to W64B_NEEDCHECK_GET / W64B_NEEDCHECK_SET (port
//                     aTester_Front.cpp:150-156; offline default: no site ever
//                     needs a re-check, and the setter is a no-op).
//    WHY IT MUST STAY GATED (TRAP 3 answer, and it is NOT "the field is missing
//                     from the tree"): bNeedCheck DOES exist in the tree --
//                     mykitsuck.h:303
//                     `bool bNeedCheck[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]` --
//                     but on the WRONG TMyKitSuck.  The class this TU is built
//                     against (aHotPlateSubstrate.h, the one FTestSuck is actually
//                     made of, shared by 177 TUs) has no such member.  So this is
//                     TRAP 5, not an absence.  Un-gating requires ADDING the member
//                     to the aHotPlateSubstrate.h mirror -- a shared header this
//                     task may not edit -- NOT swapping headers.
//    WHY THE OFFLINE DEFAULT IS FAITHFUL
//                     The latch is only ever SET through the same no-op macro
//                     (nothing anywhere in the port can make it true), so a
//                     permanently-false read is internally CONSISTENT with the rest
//                     of the translated tree: the re-check branch is dead in both
//                     directions rather than half-wired.
//    REAL-MACHINE DIFFERENCE (stated honestly -- "the degraded value happens to be
//                     equivalent" is NOT the justification): on a real head with
//                     INDEX_SUCKER_TYPE!=1, golden returns a nozzle to Normal as
//                     soon as its suck-back sensor releases; here that per-nozzle
//                     release never fires, so the nozzle keeps its current solenoid
//                     state until some other path Normals it.  With
//                     INDEX_SUCKER_TYPE==1 there is no difference at all, because
//                     golden :8572-8575 Normals every nozzle unconditionally right
//                     after.
//    ABSENCE COMMAND  Grep(pattern="bNeedCheck",
//                          path=D:/HT9045/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.h)
//                     must stay EMPTY.  Cross-check that the only *.h hit in the
//                     tree is still the WRONG class:
//                          Grep(pattern="bNeedCheck|MoveSuckData|...",
//                               path=D:/HT9045/HT9011UC_Cpp_V3.33.906.0, glob=*.h)
//                          -> mykitsuck.h:303 only (all other hits are unrelated
//                             names: bNeedCheckRTCReport, bNeedCheckIndexToque,
//                             bNeedCheckWhitleList, ...).
//                     RE-RUN AT INTEGRATION (TRAP 2).
//    MEASURED AT      2026-08-10 21:46 +0800
//    MAIN-LOOP HAND-OFF -- this is the SAME defect already tracked as task #18
//                     ("bNeedCheck shim has no backing store: 12 call sites are
//                     permanently dead").  My two call sites make it 14.  I did
//                     NOT invent a backing store here: adding one silently (e.g. a
//                     TU-local array) would make the Front re-check branch live
//                     while the 12 existing sites stay dead, which is worse than a
//                     uniformly dead branch.
//
//  SYMMETRIC TWIN and where the Front genuinely DIFFERS
//  ----------------------------------------------------
//  Twin = DoArm2D44VacCheck, golden aTester_Rear.cpp:9748-9786, port
//  aTester_Rear.cpp:12543-12581 (PT-W7b).  Both of its gate premises hold here --
//  re-verified above.  TWO golden differences carried faithfully:
//    (a) Front calls ProcessIndexSuckDestroy**1**(1) (golden :8547); Rear calls
//        ProcessIndexSuckDestroy**2**(1).  They are DIFFERENT golden methods
//        (iosetview.h:3018 vs :3019) and this file already owns a seam for the
//        "1" variant, which is why no new k8s7_ stub was minted.
//    (b) Front's `if(DoFrontTestDestroyICDelay.Off() && bArm1D44SuckCheck==true)`
//        is ONE golden line (:8554); the Rear golden splits the same condition
//        across two lines.  Front's single-line form is kept.
//  INTEGER DIVISION -- none in this function.
//  SOFT_SIMULTE  -- this range contains no #ifdef of any kind.
//  TRAP 1 -- NOT `static`: golden aTester_Front.h:27 declares
//    `extern void DoArm1D44VacCheck();` and the port declares it non-static at
//    atester_shims.h:102, so `static` would be shape (d).  Live port caller this
//    body will serve (so not shape (a)): csystem.cpp:1068.
//  TRAP 4 -- adds no file-scope object.
//  TRAP 5 -- WHICH HEADER: FTestSuck is the aHotPlateSubstrate.h TMyKitSuck
//    (extern aHotPlateSubstrate.h:636, OBJECT DEFINED aHotPlateSubstrate.cpp:92),
//    and Suck[][] are its aHotPlateSubstrate.h:106 TMySucker elements.  The rival
//    mykitsuck.h:458 `extern TMyKitSuck FTestSuck;` / object mykitsuck.cpp:211 is
//    NOT used (port aTester_Front.cpp includes aHotPlateSubstrate.h at line 115 and
//    never mykitsuck.h).  That distinction is precisely what forces
//    G-k8s7-NEEDCHECK to stay in place.
//  INTEGRATE     -- retires the no-op stub atester_shims.cpp:178
//    `void DoArm1D44VacCheck() {}` (with which the front suck-back re-check and
//    the "negative pressure must not stay on" nozzle sweep never ran at all -- a
//    real behaviour change); keep the non-static declaration atester_shims.h:102.
// ==========================================================================
void DoArm1D44VacCheck()                                                        //JerryYang 20190123 把index arm回吸檢查包成函式
{
    if(INDEX_SUCKER_TYPE==1)
    {
        bArm1D44SuckCheck=false;
        bArm1D44SuckCheck=W64B_FIOSET_PISD1(1);                                 //Sam 20241225 : 修正回吸偵測  // AI(k8-small-seven) 20260810: golden :8547 fiosetview->ProcessIndexSuckDestroy1(1) -- routed to this file's OWN existing gap stub at port aTester_Front.cpp:163-164 (see GATE REGISTER G-k8s7-PISD1)
    }
    else
    {
        bArm1D44SuckCheck=true;
    }

    if(DoFrontTestDestroyICDelay.Off() && bArm1D44SuckCheck==true)
    {
        for(int i=0; i<FTestSuck.iShtRow; i++)
        {
            for(int j=0; j<FTestSuck.iShtCol; j++)
            {
                if(W64B_NEEDCHECK_GET(i, j))                                    //確認真空狀態  // AI(k8-small-seven) 20260810: golden :8560 FTestSuck.bNeedCheck[i][j] -- gap macro, port aTester_Front.cpp:150-156 (see GATE REGISTER G-k8s7-NEEDCHECK)
                {
                    if(FTestSuck.Suck[i][j].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（黏料／殘料／不該有料）
                    {
                    }
                    else
                    {
                        W64B_NEEDCHECK_SET(i, j, false);                        // AI(k8-small-seven) 20260810: golden :8567 FTestSuck.bNeedCheck[i][j]=false -- gap macro, port aTester_Front.cpp:150-156 (see GATE REGISTER G-k8s7-NEEDCHECK)
                        FTestSuck.Suck[i][j].Normal();
                    }
                }

                if(INDEX_SUCKER_TYPE==1)                                        //jou 2011-11-01負壓不能一直開著真空，必須關掉
                {
                    FTestSuck.Suck[i][j].Normal();
                }
            }
        }
    }
}
