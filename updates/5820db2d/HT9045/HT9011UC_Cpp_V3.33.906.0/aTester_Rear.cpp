//---------------------------------------------------------------------------
//  aTester_Rear.cpp  --  TESTER/INDEX ENGINE, Rear (Z2/Arm2) narrow-scope slice
//
//  Translation wave: W64b-Translate (paired with a sibling aTester_Front.cpp
//  translated concurrently by another agent -- this file is self-contained
//  except for the one CheckZ1IsDown() cross-call declared in aTester_Front.h)
//  Translator: AI(W64b-Translate) 20260706
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp (9787
//  lines, cp950), translating exactly:
//    * bArm2PutDuplicateErr[][]      golden :89-92
//    * ResetArm2PutFlag()            golden :126-135
//    * CheckZ2IsDown()                golden :170-222
//    * TestZ2OutRandge()              golden :224-255
//    * TestZ2SetPos()                 golden :291-298
//    * iRearTestDestroyICTask + InitRearTestDestroyICTask()  golden :300-305
//    * DoRearTestDestroyIC(bool)      golden :309-886
//
//  WAVE 2 (Translator: AI(W5-aTester_Rear-Translate) 20260710) EXTENDS the file
//  above (same conventions, Wave-1 content untouched) with:
//    * ResetInxedArm2Flag()                            golden :100-113
//    * ResetArm2VacFlag()                              golden :115-124
//    * CheckBTFullSite()                               golden :137-168
//    * iRearTestSuckICTask + InitRearTestSuckICTask()   golden :888-895
//    * DoRearTestSuckIC()                               golden :898-2209
//  (ResetInxedArm2Flag/ResetArm2VacFlag/CheckBTFullSite precede ResetArm2PutFlag
//  in golden line order but are appended AFTER DoRearTestDestroyIC here to avoid
//  touching Wave-1 reviewed code -- same out-of-golden-order append convention
//  already used by aHotPlateSubstrate.h's "-- W6.x ADD" blocks.)
//  All other golden aTester_Rear.cpp content (DoBTestSuckTestIC,
//  DoRearTestPurgBeforePickShuttle/InitRearTestPurgBeforePickShuttle, DoArm2Suck,
//  TestZ2OutRandge2, BTestCombineSLK/BTestSeparateSLK, DoBRTCAutoModelVerify, the
//  D44/RTC/torque/purge sub-engines beyond what DoRearTestSuckIC itself inlines,
//  etc.) remains OUT OF SCOPE and stays routed through atester_shims.cpp as today.
//
//  KNOWN GAPS discovered while translating (none silently dropped -- each is
//  gated below with a compiling offline default and a golden line citation;
//  see the W64bT2 stub block right before DoRearTestDestroyIC, plus the two
//  #if 0 // TODO(W7) blocks inside case 310):
//    * TMyKitSuck::bNeedCheck[][]              (aHotPlateSubstrate.h) -- absent
//    * TMyKitSuck::MoveSuckData(...)           (aHotPlateSubstrate.h) -- absent
//    * TMyKitSuck::SetHasNullIcToNullIc()      (aHotPlateSubstrate.h) -- absent,
//      ALREADY a documented gap (docs/DEVLOG.md, MIGRATION_ROADMAP.md) and
//      already gated the same way in csystem.cpp (W7C2_BLCARRYKIT_SETHASNULLIC)
//    * TMyProductionRecord::AddIndexPlaceShuttleRecord(...) -- absent
//    * TMySucker::sRecordStartOffDelayTime/sRecordEndOffDelayTime/
//      iRecordOffDelayTime -- absent (gated inline, #if 0)
//    * LAST_GENERAL_SET::iIndexInputOutPut[] (canary_support.h shim) -- absent
//      (gated inline, #if 0; canary_support.h is a shared file this task may
//      not edit)
//    * DoVibrateOutShuttle(int,bool) (golden ainarm2.h:215) -- ainarm2.cpp only
//      has SetInArmUseSuckToHasNullIC/InitInArmPickFromHotPlateTask so far
//    * RecordIndexAirOnTime2() (golden cObserver.h:565) -- cObserver.cpp not
//      yet translated
//    * TfiosetviewShim::ProcessIndexSuckDestroy2(int) (golden iosetview.h:3019)
//      -- the shim (atester_shims.h) exposes bIndexSuck[][][] but not this
//      method (atester.cpp has its own zero-arg W7T1_ precedent for this exact
//      gap; mine takes the arg the Rear call sites actually pass)
//    * ATC_TYPE_33 / ATC_TYPE_35 (golden ATC/ATC_Handler_Side.h:24-25) -- that
//      header pulls in the untranslated ATC subsystem, so the two fixed
//      literal values are supplied locally instead (not a behavioural
//      approximation -- exact real values)
//    * TQPF_Timer BTempRiseHasICDelay (golden :308, file-scope peer of
//      DoRearTestDestroyIC, analogous to bArm2PutDuplicateErr) -- added here
//    * TMyProductionRecord::AddIndexCycleTimeRecord(double) (golden
//      MyProductionRecord.h:38) -- same declaration-only forward-stub class as
//      AddIndexPlaceShuttleRecord above (no .cpp this wave); gated the same
//      way (golden :514).  AI(W64b-Integrate) 20260706.
//
//  WAVE 2 KNOWN GAPS (AI(W5-aTester_Rear-Translate) 20260710 -- same "gate with
//  a compiling offline default + golden line citation" discipline; new stub
//  block sits right before DoRearTestSuckIC, after ResetInxedArm2Flag /
//  ResetArm2VacFlag / CheckBTFullSite):
//    * TMyKitSuck::MoveSuckData(...) -- same absent method as above; reuses the
//      Wave-1 W64bT2_MoveSuckData(dst,src,i,j) stub verbatim (golden :1940/:2127)
//    * TMyKitSuck::SetUnuseToNullIC() (golden MyKitSuck.cpp:354-366) -- absent
//      from aHotPlateSubstrate.h's minimal mirror, BUT its golden body only
//      touches members already exposed (SetItemData/iMaxRow/iMaxCol/iShtRow/
//      iShtCol) -- so W64bT2_SetUnuseToNullIC(kit) is a FAITHFUL free-function
//      equivalent, not an approximated no-op (golden :2142)
//    * TMyProductionRecord::AddErrorRecord(...)/AddErrorRecordNoSave(...) --
//      TMyProductionRecord (Public/MyProductionRecord.h) has NO .cpp anywhere in
//      the tree yet (same blanket gap as Wave-1's AddIndexCycleTimeRecord/
//      AddIndexPlaceShuttleRecord) -- gated individually per call site
//      (golden :1730/:1800/:2083); AddErrorRecordNoSave is not even declared in
//      the current minimal MyProductionRecord.h (a further sub-gap)
//    * TfSCKART::iInfo_MultiLotCnt (golden Automation/SCK_ART.h:323) + LotSummary/
//      TLotSummary (golden cSocket.h, entire class untranslated) -- the whole
//      `if(fSCKART->iInfo_MultiLotCnt>1){...}` block is gated (golden :2145-2158)
//    * APAX_WriteData(bool,WORD,int=0) (golden adam6024.h) -- a DIFFERENT DAQ
//      family from the already-shimmed ADAM_* (atester_shims.h); adam6024.cpp
//      itself is not translated.  W64bT2_APAX_WriteData is a TU-local no-op,
//      mirroring the ADAM_* convention (golden :2163)
//    * fAutoTeach (golden AutoTeach.h TfAutoTeach*) -- unlike chkReadTorque/
//      edTorue1 below, fAutoTeach has NO home ANYWHERE in the tree yet (checked:
//      every other TU's fAutoTeach-> use is either fully `#if 0`-gated or a
//      TU-local `#define ...(false)` macro in csystem.cpp).  The single call
//      here (`fAutoTeach->SetIndexBinError(1)`, golden :2165) is gated rather
//      than stubbed since it is a pure UI auto-align marker with no state the
//      SM reads back
//    * fMain->chkReadTorque1/chkReadTorque2/edTorue1 (golden main.h) -- the
//      FormsFacade TfMain (FormsFacade.h) lacks these torque-read UI widgets.
//      atester.cpp's DoTestHeadMotor (T1) already carries an analogous TU-local
//      seam (W7T1_TfMainTorqueSeam) for the SAME golden gap; this file defines
//      its own independent twin (W64bT2_TfMainTorqueSeam, this file's own
//      prefix) rather than reach across translation units (golden :1269-1274/
//      :1443/:1553-1557/:1644)
//
//  INTEGRATE-STEP FIXES (AI(W64b-Integrate) 20260706 -- discovered as
//  undefined-reference LINK errors while wiring this file into the build, not
//  caught by the translation-time call-graph review since both are golden
//  symbols DECLARED in an already-translated header with NO compiled body
//  anywhere in the tree):
//    * OutSHT2InLF() / CheckIndexAllSuckICFallDown() (golden csystem.cpp:700 /
//      :2417) -- csystem.h declared them but csystem_predicates.cpp never
//      defined them.  Translated FAITHFULLY (not gated) into
//      csystem_predicates.cpp.
//    * GetSiteCount(bool) (golden cprod.cpp:305, declared cprod.h:3299) --
//      real body exists but sits inside cprod.cpp's whole-region
//      `#if 0 // TODO(W6)` gate (cprod.cpp:184-4036); a faithful duplicate was
//      added to atester_shims.cpp (same pattern already used there for
//      IsInArmCleanOutFinish/IsInArmOneCycleFinish).
//  Integrate-step note: atester_shims.h/.cpp TODAY still declare+define
//  offline-stub bodies for all 6 symbols this file now defines for real
//  (iRearTestDestroyICTask, InitRearTestDestroyICTask, DoRearTestDestroyIC,
//  CheckZ2IsDown, TestZ2OutRandge, TestZ2SetPos) -- REMOVED from
//  atester_shims.h/.cpp as part of this same integrate step.
//
//  WAVE 2 INTEGRATE-STEP NOTE (AI(W5-aTester_Rear-Translate) 20260710):
//  atester_shims.h/.cpp TODAY still declare+define offline-stub bodies for the
//  3 symbols this file now defines for real (iRearTestSuckICTask,
//  InitRearTestSuckICTask, DoRearTestSuckIC) -- REMOVE from atester_shims.h/.cpp
//  as part of the integrate step (same pattern as the Wave-1 note above).
//  bArm2SuckFinish[][] stays in atester_shims.cpp UNCHANGED (a plain data
//  global both DoRearTestDestroyIC and DoRearTestSuckIC merely read/write --
//  not a function this file now defines, so no duplicate-definition risk).
//
//  TRANSLATION RULES (mirrors atester.cpp / aTester_Front.cpp):
//    * AnsiString/TQPF_Timer via the substrate headers; numeric semantics EXACT.
//    * SOFT_SIMULTE / DEBUG_INDEX_UPH NOT defined in this tree -> the #ifdef
//      bodies for both are reproduced VERBATIM (inert, matches golden text
//      byte-for-byte including golden's own missing-paren typo in the dead
//      DEBUG_INDEX_UPH branch of case 100 -- never parsed since the branch is
//      preprocessed away, exactly as it never was in the golden BCB6 build).
//    * Big5 Chinese comments decoded via cp950, preserved as UTF-8.  ZERO U+FFFD.
//---------------------------------------------------------------------------
#include "MachineDefine.h"

#include "aTester_Rear.h"

#include "atester.h"                // GetIndexZSpeed / DoTemperatureRise / IndexAlarmInArmAway / IndexCheck4Site
#include "atester_shims.h"          // fiosetview (TfiosetviewShim) / IsNNMode
#include "aTester_Front.h"          // CheckZ1IsDown() -- sibling translation unit (parallel wave)
// AI(W5-Automation-Integrate) 20260710: ProcessPiggyBackFunction is now real
// (atester_ProcessCount.cpp, removed from atester_shims.h) -- include its own header.
#include "atester_ProcessCount.h"
#include "aArmHeader.h"             // guard-only shim (matches atester.cpp convention)

#include "MachineType.h"           // eSpeedPart::IndexArm / eNewATCSystem / MAX_Index_Row / MAX_SOCKET_ROW/COL / CC_KYEC_LEE
#include "Motor/mymotor.h"         // MOT[] / TTrayMotor / ShowIndexMotorError / RecordIndexPositionError
#include "vclcompat/vcl_compat.h"  // AnsiString
#include "csystem.h"                // InShtInLF / OutSHT2InLF / CheckIndexAllSuckICFallDown / IsInArmOneCycleFinish / IsInArmCleanOutFinish / InSHT2InRT
#include "cprod.h"                  // Prod / TestIF / TestIF_File / ArmSpeed_File / GetSiteCount / Temperature
#include "cmydef.h"                  // global scalar universe + IC consts + K_SKIP/K_RETRY/REALLY + bArm2D44SuckCheck/...
#include "aHotPlateSubstrate.h"     // BLCarryKit / BRCarryKit / BTestSuck / TestSocket (TMyKitSuck) + TMySucker
#include "CosFunction.h"            // CosFunction
#include "Config.h"                  // IniConfig
#include "acarry_shims.h"           // ATC_InterfaceForm (TATC_InterfaceFormShim*)
#include "canary_support.h"         // LastSet / ShowMyMessage / ShowErrorMessage / __FUNC__
#include "FormsFacade.h"            // (Wave 2) fMain->ResetRecordforPiggyBack / fSCKART
#include "acarry.h"                 // (Wave 2) CheckCFixTrayFullPlace / CheckShuttle2EncoderPos
// AI(pt-wave) 20260810 PT-W7b: includes the appended bodies need. Each symbol was
// checked for a LINKABLE definition first, not just a declaration:
#include "mysensor.h"              // Sen[]              -- mysensor.h:48 / mysensor.cpp
#include "mycylin.h"               // Cylinder[]         -- mycylin.h:176 / mycylin.cpp
#include "MessageDef.h"            // HHandler2Gpib (MessageDef.cpp:268) + MSG_CMD_* consts
// (ATC/ATCInterface.h include removed with GATE (W7b-I1) -- its only two
//  consumers in this file are now gated, so the header is unnecessary.)
#include "Automation/AMR.h"        // AMR                -- Automation/AMR.cpp:66
//---------------------------------------------------------------------------

// golden :89-92 -- plain global,排除重複的Alarm Code (Steven 20100105 Start)
bool bArm2PutDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                           {false, false, false, false, false, false, false, false},
                                                           {false, false, false, false, false, false, false, false},
                                                           {false, false, false, false, false, false, false, false}};
//------------------------------------------------------------------------------
// golden :126-135 (Steven 20241017 : 打包成function).  NOTE: golden Rear's
// ResetArm2PutFlag loops over BTestSuck (its OWN kit instance), NOT FTestSuck --
// verified against golden text directly (deviates from the up-front task
// paraphrase, which is expected/flagged per the task instructions).
void ResetArm2PutFlag()
{
    for(int i=0; i<BTestSuck.iMaxRow; i++)
    {
        for(int j=0; j<BTestSuck.iMaxCol; j++)
        {
            bArm2PutDuplicateErr[i][j]=false;
        }
    }
}
//------------------------------------------------------------------------------
bool CheckZ2IsDown()
{
    #ifdef SOFT_SIMULTE
        return true;
    #else
        long lPos[4]={0, 0, 0, 0};                                              //kevin 20150915

        if(IniConfig.bD52InterFaceErrHeadNeedUp && bNowDoInterFaceErrorStep)
            return false;

        if(bContactTimeOverStep)
            return false;

        if(CosFunction.bSortingBy2DList==true &&
           LastSet.iTester==_2D_SORT &&
           TestIF_File.bSortingBy2DIDList==true)                                //Frank 20221122 : 2DID sorting for ATK
        {
            return true;
        }

        if(MOT[MTestZ2].Motor->Enable==false)
            return true;

        if(TestIF_File.iShuttleMode==1 &&                                       //20111114  Dell for Disable Index Arm    Start
           TestIF_File.iShuttle_Sel==0)                                         //jou 2014-11-13 remake code , close arm index postion error
        {
            return true;
        }

        if(bUseTwoArm32Site==true)
        {                                                                       //nn Mode 是同時有吸取IC動作
            return true;
        }
        else
        {
            //==> Eastsun 20260511 F007 整合: Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
            if(CUSTOMER_CODE==CC_KYEC_LEE && iSECSGEM_ConsecutiveFailureAlarm==2)
            {
                return true;
            }
            else if(MOT[MTestZ2].Gali_ReadEncoderPos()>Prod.iTestDown_Z2)
            //<== Eastsun 20260511 F007 整合
            {
                lPos[2]=MOT[MTestZ2].Gali_ReadEncoderPos();                     //kevin 20150915
                lPos[3]=Prod.iTestDown_Z2;                                      //kevin 20150915
                RecordIndexPositionError("CheckZ2IsDown()", false, false, true, true, &lPos[0]);                        //kevin 20150915 record
                ShowIndexMotorError(AnsiString("CheckZ2IsDown"));
                return false;
            }
        }
        return true;
    #endif
}
//==============================================================================
bool TestZ2OutRandge()
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
        int Pos1=MOT[MTestZ2].Gali_ReadPos();
        int Pos2=MOT[MTestZ2].Gali_ReadEncoderPos();
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
void TestZ2SetPos()
{
    int Pos3=ChangeToFloatNonPcnt((double)(MOT[MTestZ2].Gali_ReadEncoderPos()), (double)(MOT[MTestZ2].Motor->GearRatio));                                       //Steven 20100728 Start: 要加上齒輪比
    int Pos4=ChangeToFloatNonPcnt((double)(MOT[MTestY2].Gali_ReadEncoderPos()), (double)(MOT[MTestY2].Motor->GearRatio));
    char str[256];
    sprintf(str, "DP,,%d,%d", 0-Pos3, 0-Pos4);
    MOT[MTestY2].Gali_Command(str, __FUNC__);
}
//---------------------------------------------------------------------------
int iRearTestDestroyICTask=1;
void InitRearTestDestroyICTask()
{
    iRearTestDestroyICTask=1;
    bZ2Isdownflag=false;
}
//------------------------------------------------------------------------------
// golden :308 -- file-scope peer of DoRearTestDestroyIC (same status as
// bArm2PutDuplicateErr above); golden :307 CheckSocketHasICDelay2 is NOT added
// -- every golden reference to it inside DoRearTestDestroyIC (case 700/750) is
// itself commented-out dead code, reproduced verbatim as comments below.
TQPF_Timer BTempRiseHasICDelay;

// ---------------------------------------------------------------------------
//  PT-W7b: the EIGHT file-scope globals golden declares BETWEEN functions.
//
//  These were missed by the wave plan, and the miss was mine: PT-W7b selected its
//  targets by FUNCTION span (census counts functions), so declarations sitting in the
//  gaps between golden functions belonged to no agent. Three independent audits caught
//  it -- iWaitIndexArm2 alone is referenced 4x in chunk 1 and 2x in chunk 2 with no
//  declaration anywhere in the tree. Verified absent tree-wide 20260810 before adding:
//    grep -rl "\b<name>\b" --include=*.cpp --include=*.h .  -> ABSENT x8
//
//  TYPE NOTE (TRAP 5 -- and the reason these are NOT declared as HTimer):
//  golden types DoTestYRearDelay/DoTestYRearDelay2 as HTimer, whose real definition is
//  D:\HT9045\elec\Component\htimer.h -- OUTSIDE the version tree, and never ported.
//  What this TU would get instead is atester_shims.h:463
//      struct HTimer { bool Off(){ return true; } void SetSecAndOn(double){} };
//  whose Off() is hard-coded TRUE, i.e. every dwell expires instantly, and which lacks
//  SetMSAndOn/SetSec entirely (this wave calls Off 47x, SetSecAndOn 39x, SetMSAndOn 13x,
//  SetSec 1x). Declaring them HTimer would compile, link clean, and silently zero every
//  delay in eleven DoTestYRear states -- the classic wrong-type-links-fine failure.
//  TQPF_Timer (myTimer.h) is a real QPC timer exposing all four methods, and aliasing
//  golden HTimer -> TQPF_Timer is the idiom ALREADY established three times in this tree
//  (acatchtray.cpp:114, CanBus/cMyDNM100UD.cpp:85, MyPLC/MyPLC_IO_Modbus.cpp:49). A
//  local `typedef TQPF_Timer HTimer;` is impossible HERE because this TU includes
//  atester_shims.h (:151), so the struct is already in scope -- hence TQPF_Timer direct.
//  AI(pt-wave) 20260810
// ---------------------------------------------------------------------------
int iHangupCTArm2=0;                                                            // golden aTester_Rear.cpp:76
bool bReadTorqueOK=false;                                                       //kevin 20211129  (golden :78)
TQPF_Timer BTorqueTimeOutDelay;                                                 //kevin 2021018  (golden :98, golden type TQPF_Timer)
TQPF_Timer DoTestYRearDelay, DoTestYRearDelay2;                                 // golden :4253 (golden type HTimer -- see TYPE NOTE)
TQPF_Timer iWaitIndexArm2;                                                      // golden :5358
TQPF_Timer hTestZ1Delay;                                                        // golden :5359
TQPF_Timer hFRTCTimeOutDelay;                                                   //JerryYang 2022  (golden :5360)

// =============================================================================
//  W64bT2 offline gap stubs -- see file-top "KNOWN GAPS" list for the full
//  rationale on each.  All are file-scope (static where applicable) so they
//  cannot collide at link time with a future real implementation landing in
//  aHotPlateSubstrate.h / ainarm2.cpp / cObserver.cpp / atester_shims.h.
//  Integrate: once the real members/functions exist, drop this block and the
//  call-site substitutions below and restore the golden text verbatim.
// =============================================================================
// -- TMyKitSuck::bNeedCheck[][] (golden MyKitSuck.h:174) -- per-nozzle "destroy
//    just fired, needs a vacuum re-check" latch (D44 recheck sub-flow, case
//    310/500/600).  Offline default: no site ever needs a recheck (false),
//    which routes the SM through the "no recheck needed" branch -- mirrors how
//    other offline gaps in this tree default (e.g. atester.cpp's
//    W7T1_CHECKVACINIT leaves its out-param false).
static bool W64bT2_BNeedCheckGet(int i, int j) { return BTestSuck.bNeedCheck[i][j]; }   // AI(W906-W2-A46) 20260926: 不再是 no-op（mykitsuck.h:303）；呼叫點 golden 全是 BTestSuck（:505／:668／:705／:757／:769／:814），唯一的 FTestSuck（:8412）在 :10615 直接寫
static void W64bT2_BNeedCheckSet(int i, int j, bool val) { BTestSuck.bNeedCheck[i][j] = val; }   // AI(W906-W2-A46) 20260926: 不再是 no-op（同上）

// -- TMyKitSuck::MoveSuckData (golden MyKitSuck.h:268) -- copies one site's
//    decoded per-site data from Source[SourceR][SourceC] into this kit's same
//    cell.  Offline default: no-op (BRCarryKit's grid keeps its current data;
//    the test-result decode path reads off TestSocket directly, not
//    BRCarryKit, so no result data is lost by this gap).
static void W64bT2_MoveSuckData(TMyKitSuck &dst, TMyKitSuck &src, int r, int c)   // AI(W906-W2-KITDATA) 20260926: 不再是 no-op —— 轉呼叫真的 TMyKitSuck::MoveSuckData（mykitsuck.h:374，A4-6 起存在；本檔 :5869-5874 早就記錄這個替身「silently dropping data」—— golden 就是要把 TestSuck 那格的資料搬到飛梭那格）
{
    dst.MoveSuckData(src, r, c);
}

// -- TMyProductionRecord::AddIndexPlaceShuttleRecord (golden MyProductionRecord.h)
//    per-site Index->OutShuttle place log record.  Offline default: no-op
//    (pure logging, no state read back by the SM).
static void W64bT2_AddIndexPlaceShuttleRecord(TMyProductionRecord &rec, int siteMap, int r, int c)   // AI(W906-W2-PRODREC) 20260926: 不再是 no-op —— TMyProductionRecord::AddIndexPlaceShuttleRecord(int,int,int) is defined (MyProductionRecord.cpp.obj, nm)
{
    rec.AddIndexPlaceShuttleRecord(siteMap, r, c);
}

// -- TMyKitSuck::SetHasNullIcToNullIc (golden MyKitSuck.h) -- KNOWN, already
//    documented gap (docs/DEVLOG.md, MIGRATION_ROADMAP.md); TMyKitSuck only has
//    the inverse SetNullIcToHasNullIc.  Gated the same way csystem.cpp already
//    gates it (W7C2_BLCARRYKIT_SETHASNULLIC): offline no-op.
static void W64bT2_SetHasNullIcToNullIc(TMyKitSuck &kit) { kit.SetHasNullIcToNullIc(); }   // AI(W906-W2-KITDATA) 20260926: 不再是 no-op —— 真方法 mykitsuck.h:387 A4-6 起存在

// -- DoVibrateOutShuttle (golden ainarm2.h:215 `bool DoVibrateOutShuttle(int
//    iShuttle, bool bNeedInitial=false);`) -- ainarm2.cpp only carries
//    SetInArmUseSuckToHasNullIC/InitInArmPickFromHotPlateTask so far.  Offline
//    default: report "done" (true) immediately so the SM does not stall on it
//    (mirrors the "offline: destroy done immediately" default used elsewhere,
//    e.g. ainarm9045.cpp ProcessSCKARTLoadingCount case 1).
static bool W64bT2_DoVibrateOutShuttle(int iShuttle, bool bNeedInitial=false)
{
    (void)iShuttle; (void)bNeedInitial;
    return true;
}

// -- RecordIndexAirOnTime2 (golden cObserver.h:565 `void RecordIndexAirOnTime2();`)
//    -- cObserver.cpp not yet translated.  Offline no-op (pure logging).
static void W64bT2_RecordIndexAirOnTime2() { extern void RecordIndexAirOnTime2(); RecordIndexAirOnTime2(); }   // AI(W906-W2-PRODREC) 20260926: 不再是 no-op —— RecordIndexAirOnTime2() is defined in cObserver.cpp (nm); it only fills the observer grid

// -- TfiosetviewShim::ProcessIndexSuckDestroy2(int) (golden iosetview.h:3019
//    `bool ProcessIndexSuckDestroy2(int iType=0);`) -- atester_shims.h's
//    TfiosetviewShim exposes bIndexSuck[][][] but not this method.  atester.cpp
//    already has its own zero-arg W7T1_ precedent for the SAME gap; this one
//    takes the int arg the Rear call sites actually pass.  Offline default:
//    report suck self-check "done" (true), same offline value as the W7T1
//    precedent.
static bool W64bT2_ProcessIndexSuckDestroy2(int iType=0) { return fiosetview->ProcessIndexSuckDestroy2(iType); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）

// -- ATC_TYPE_33 / ATC_TYPE_35 (golden ATC/ATC_Handler_Side.h:24-25, fixed
//    literal #defines 33/35) -- that header pulls in the full untranslated ATC
//    subsystem, so not included here; the two values are hard-coded locally
//    (exact real values, not an approximated default).
#ifndef ATC_TYPE_33
#define ATC_TYPE_33   33
#endif
#ifndef ATC_TYPE_35
#define ATC_TYPE_35   35
#endif
//------------------------------------------------------------------------------
TQPF_Timer CheckSocketHasICDelay2;                                              //kevin 20190819 add
bool DoRearTestDestroyIC(bool bCheckZ1)
{
    static int iSiteCount=0;
    static int iRetry=0;                                                        //kevin 20190709  index check one by one

    int &Task=iRearTestDestroyICTask;
    int sp=0, iPos=0;
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
            iIndexTakeDeviceChk2=0;                                             //Steven 20250224 回吸檢測狀態
            if(BRCarryKit.UseSiteHasIC())
            {
                return false;
            }

            if(OutSHT2InLF()==false)
            {
                Task=100;                                                       //JerryYang 20170106 (Steven) fix out shuttle 偵測殘料後hang up
                return false;
            }

            MOT[MInShuttle2].fCanMoveM=false;
            Task=200;
            if(CheckIndexAllSuckICFallDown(false, true))                        //jou 20210713 : 增加index destroy ic時掉料偵測
            {
                ShowMyMessage("Index Arm2 掉料，請檢查IC是否有在Out Shuttle中!!");
            }

            if(IsNNMode()==NN_2Row)
                break;
            break;                                                              //JerryYang 20170106 (Steven) fix out shuttle 偵測殘料後hang up
        case 100:
            #ifdef DEBUG_INDEX_UPH
            if(MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Safe, iIndexSpeed, iIndexAcc);                                    //Steven 20170818 : 測試UPH用
            #else
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed))
            #endif
            {
                if(bHasDropErr)                                                                                         //JerryYang 20241122 : Index arm放料到shuttle偵測drop error
                {
                    bHasDropErr=false;
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(BTestSuck.Suck[i][j].Enable       &&
                               BTestSuck.Suck[i][j].SenUsing!="" &&
                               BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                               BTestSuck.Item[i][j]!=NULL_IC)
                            {
                                if(BTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    ErrPart+=IndexSuckName[i][j];
                                    bHasErr=true;
                                }
                            }
                        }
                    }

                    if(LastSet.iRealDummy==REALLY && bHasErr)
                    {
                        if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                     //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                            bIsTestSitICFallDown=true;
                        ShowErrorMessage("JAM0304", K_SKIP, MTestZ2, false, ErrPart);                                   //Steven 20100129 : Device Drop Error          //Jimmychiu 20260223 : 修正Index Arm 異常顯示位置錯誤
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(BTestSuck.Suck[i][j].Error ||
                                   (BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                    BTestSuck.Item[i][j]!=NULL_IC &&
                                    BTestSuck.Suck[i][j].GetStatus()==false))                                           //有用到且有吸到IC的卻掉了
                                {
                                    BTestSuck.SetItemData(i, j, HAS_NULL_IC);                                           //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                                    BTestSuck.Suck[i][j].Normal();                                                      //Steven 20110829 : 把真空關掉
                                }
                            }
                        }
                    }
                }

                if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                     //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    TestIF_File.bArm1PickPlaceArm2Test==true) &&
                    TestIF_File.bArm1OnlyPlaceArm2TestAndSuck==true)                                                    //JerryYang 20250723 : avoid shuttle hang up
                {
                    if(BLCarryKit.HasRealIC()==false &&                                                                 //Arm 2 no料
                       BRCarryKit.HasRealIC()==false)
                    {
                        W64bT2_SetHasNullIcToNullIc(BLCarryKit);                                                        // AI(W64b-Translate) 20260706: golden :407 BLCarryKit.SetHasNullIcToNullIc() -- gap stub (see file-top)
                        W64bT2_SetHasNullIcToNullIc(BRCarryKit);                                                        // AI(W64b-Translate) 20260706: golden :408 BRCarryKit.SetHasNullIcToNullIc() -- gap stub (see file-top)
                        MOT[MInShuttle2].fCanMoveM=true;
                    }
                }
                bSuckingFlagZ2=false;                                                                                   //Steven 20240916 : index下降到shuttle吸放料
                Task=1;
            }
            break;
        case 200:
            bHasDropErr|=CheckIndexAllSuckICFallDown(false, true);              //JerryYang 20241122 : Index arm放料到shuttle偵測drop error
            if(bHasDropErr)
            {
                Task=100;
                break;
            }

            if(InShtInLF(1)==false)                                             //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
            {
                bZ2Isdownflag=false;                                            //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
                ShowMyMessage("Shuttle 2 position error", "Shuttle 2 位置錯誤");
                MOT[MInShuttle2].fCanMoveM=true;
                Task=1;
            }
            else if(bCheckZ1==false || CheckZ1IsDown())                         //ChungHung 20111115
            {
                bSuckingFlagZ2=true;                                            //Steven 20240916 : index下降到shuttle吸放料
                flag2=false;                                                    //ChungHung 20110816 add Index Place slow down
                bZ2Isdownflag=true;                                             //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
                if(BTestSuck.HasRealIC()==false)
                {
                    flag2=true;
                }
                else
                {
                    if(IniConfig.bD54SlowDown)
                    {
                        sp=GetIndexZSpeed(1);                                   //Steven 20160524 : Index Z軸速度整合為Function
                        #ifdef DEBUG_INDEX_UPH
                        if(MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Place, iIndexSpeed, iIndexAcc))
                        #else
                        if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Place, sp))
                        #endif
                        {
                            flag2=true;
                        }
                    }
                    else
                    {
                        #ifdef DEBUG_INDEX_UPH
                        if(MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Place, iIndexSpeed, iIndexAcc))
                        #else
                        if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Place, MOT[MTestZ2].GailSpeed, IniConfig.iD46WaitIndexDestroyTime))
                        #endif
                        {
                            flag2=true;
                        }
                    }
                }

                if(flag2)
                {
                    BTestSuck.ResetAll();
                    if(ATC_SYSTEM==eNewATCSystem && Temperature.bEnableTempRise==true && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35))
                    {
                        DoTemperatureRise(1, true);
                        BTempRiseHasICDelay.SetSecAndOn(Temperature.dTempRiseDelay);
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
            if(BTempRiseHasICDelay.Off())
            {
                DoTemperatureRise(1, false);
                Task=310;
            }
            break;
        case 310:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j])
                    {
                        if(BTestSuck.Item[i][j]==HAS_NULL_IC ||
                           BTestSuck.Suck[i][j].Destroy())
                        {
                            if(IniConfig.bD44CheckIndexICDestroy)
                            {
                                if(BTestSuck.Item[i][j]!=HAS_NULL_IC)
                                {
                                    BTestSuck.Suck[i][j].Off();                                                         //Suck Off and Destroy On
                                    W64bT2_BNeedCheckSet(i, j, true);                                                   // AI(W64b-Translate) 20260706: golden :505 BTestSuck.bNeedCheck[i][j]=true -- gap stub (see file-top)
                                    iIndexTakeDeviceChk2=2;                                                             //kevin 20190103 回吸檢測狀態
                                }
                            }

                            if(BTestSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                //AI(W906-W2-PRODREC) 20260926: gate LIFTED -- LAST_GENERAL_SET shim retired (canary_support.h now includes LastSet.h; long iIndexInputOutPut[4] at LastSet.h:433) -- golden :512
                                LastSet.iIndexInputOutPut[1]++;                                                         //kevin 20130125
                                //AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
// AI(W64b-Integrate) 20260706: TMyProductionRecord::AddIndexCycleTimeRecord is
// declared (Public/MyProductionRecord.h:38) but that whole class is a
// declaration-only forward-stub this wave (no .cpp; see its file-top banner) --
// same documented gap as W64bT2_AddIndexPlaceShuttleRecord above; gated the
// same way (pure logging, no state the SM reads back).
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddIndexCycleTimeRecord(double) is defined (MyProductionRecord.cpp.obj, nm); fRecindexCycleTim is cmydef.h:3632 -- golden :513 (AI(W906-R82) 20260927: was ":514", off by one -- NB2 R82)
                                BTestSuck.PordRec[i][j].AddIndexCycleTimeRecord(fRecindexCycleTim);                     //Frank 20160505 add
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                                if(CosFunction.RecordIndexAirOnTime)                                                    //Sam 20220329 : Record Index Air On Time
                                {
                                    //AI(W906-W2-PRODREC) 20260926: gate LIFTED -- golden-layout TMySucker has sRecordStartOffDelayTime/sRecordEndOffDelayTime/iRecordOffDelayTime (mykitsuck.h:264-266) since A4-6; QueueAirOnTime1/2 defined in cmydef.cpp -- golden :515
                                    QueueAirOnTime2.Add(BTestSuck.Suck[i][j].sRecordStartOffDelayTime, BTestSuck.Suck[i][j].sRecordEndOffDelayTime, BTestSuck.Suck[i][j].iRecordOffDelayTime);
                                    //AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                                }
                            }
                            W64bT2_MoveSuckData(BRCarryKit, BTestSuck, i, j);                                           // AI(W64b-Translate) 20260706: golden :517 BRCarryKit.MoveSuckData(BTestSuck,i,j) -- gap stub (see file-top)
                            W64bT2_AddIndexPlaceShuttleRecord(BRCarryKit.PordRec[i][j], TestIF.iSiteMap[i][j], i, j);    // AI(W64b-Translate) 20260706: golden :518 BRCarryKit.PordRec[i][j].AddIndexPlaceShuttleRecord(...) -- gap stub (see file-top)  //Sam 20201216 : Add record //Sam 20200716 : Add record
                            bArm2PutDuplicateErr[i][j]=false;
                        }
                    }
                }
            }
            flag=false;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j] &&
                       BTestSuck.Suck[i][j].Error==false)
                    {
                        flag=true;
                        break;
                    }
                }
            }

            if(flag==false)
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error)
                        {
                            Task=320;
                            return false;
                        }
                    }
                }
            }

            if(BTestSuck.UseSiteHasIC())
            {
                iRetry=0;
                break;
            }

            if(TestZ2OutRandge())
            {
                iRetry++;
                if(iRetry>20)
                {
                    iRetry=0;
                    TestZ2SetPos();
                }
                else
                {
                    break;
                }
            }
            W64bT2_RecordIndexAirOnTime2();                                                                             // AI(W64b-Translate) 20260706: golden :572 RecordIndexAirOnTime2() -- gap stub (see file-top)  //Sam 20220329 : Record Index Air On Time
            iRetry=0;
            ResetArm2PutFlag();                                                                                         //Steven 20241017 : 打包成function
            if(IniConfig.bF25VibrateForOutShuttle)
            {
                W64bT2_DoVibrateOutShuttle(1, true);                                                                    // AI(W64b-Translate) 20260706: golden :577 DoVibrateOutShuttle(1,true) -- gap stub (see file-top)
            }

            if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                         //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                TestIF_File.bArm1PickPlaceArm2Test==true) &&
                TestIF_File.bArm1OnlyPlaceArm2TestAndSuck==true)                                                        //Steven 20150129 : 需要確認Arm2有沒有粘料
            {
                BLCarryKit.SetNullIcToHasNullIc();
                BRCarryKit.SetNullIcToHasNullIc();
            }
            Task=500;
            break;
        case 320:                                                                                                       //破壞異常處理
            W64bT2_RecordIndexAirOnTime2();                                                                             // AI(W64b-Translate) 20260706: golden :590 RecordIndexAirOnTime2() -- gap stub (see file-top)  //Sam 20220329 : Record Index Air On Time
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bArm2PutDuplicateErr[i][j])
                        bHasDuplicateErr=true;
                    if(BTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i][j];
                        BTestSuck.Suck[i][j].Error=false;                                                               //Steven 20101229 : 換位置
                        bArm2PutDuplicateErr[i][j]=true;
                    }
                    else
                    {
                        bArm2PutDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bHasErr)
                ShowErrorMessage("JAM0327", K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);                               //Vacuum sensor OFF error
            bHasErr=false;
            if(ATC_SYSTEM==eNewATCSystem && Temperature.bEnableTempRise==true && (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35))
            {
                DoTemperatureRise(1, true);
                BTempRiseHasICDelay.SetSecAndOn(Temperature.dTempRiseDelay);
                Task=309;
            }
            else
            {
                Task=310;
            }
            BTestSuck.ResetAll();                                                                                       //Sam 20220330 : 補 ResetAll
            break;
        case 500:
            if(IniConfig.bL42_UseOutShuttleDesoakTime==true)                    //Ztex 2023.04.19 Add HT-1032 TriTemp Function
            {
                BTestSuck.TSoakTime.SetSecAndOn(IniConfig.iL42_UseOutShuttleDesoakTime);
            }
            iPos=Prod.TestZ1_Safe;
            sp=GetIndexZSpeed(1);                                               //Steven 20160524 : Index Z軸速度整合為Function
            #ifdef DEBUG_INDEX_UPH
            if(MOT[MTestZ2].Gali_MotMove2(iPos, iIndexSpeed, iIndexAcc))
            #else
            if(MOT[MTestZ2].Gali_MotMoveNoWait(iPos, sp, 0))
            #endif
            {
                bSuckingFlagZ2=true;                                            //Steven 20240916 : index下降到shuttle吸放料
                bZ2Isdownflag=false;                                            //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖 //Steven 20180522 : 換位置, 解除Hang up
                flag=false;
                if(IniConfig.bF25VibrateForOutShuttle)
                {
                    if(W64bT2_DoVibrateOutShuttle(1)==false)                                                            // AI(W64b-Translate) 20260706: golden :647 DoVibrateOutShuttle(1) -- gap stub (see file-top)
                    {
                        break;
                    }
                }
                MOT[MInShuttle2].fCanMoveM=true;                                //JerryYang 20181219 fix D44異常時 shuttle剛好要搖搖會發生hang up
                if(ArmSpeed_File[IndexArm].bDevicConfirm &&
                   INDEX_SUCKER_TYPE==1)                                        //kevin 20190819 add index confim 一次4個SITE
                {
                    TotalErrPart="";
                    Task=650;
                    W64bT2_SetHasNullIcToNullIc(BRCarryKit);                                                            // AI(W64b-Translate) 20260706: golden :658 BRCarryKit.SetHasNullIcToNullIc() -- gap stub (see file-top)
                    return false;
                }

                if(IniConfig.bD44CheckIndexICDestroy)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(W64bT2_BNeedCheckGet(i, j))                                                              // AI(W64b-Translate) 20260706: golden :668 BTestSuck.bNeedCheck[i][j] -- gap stub (see file-top)
                            {
                                bIndexPlaceIcCheck=true;                        //kevin 20110505
                                flag=true;
                                if(INDEX_SUCKER_TYPE==1)                        //Steven 20111202
                                {
                                    fiosetview->bIndexSuck[1][i][j]=true;
                                }
                                else
                                {
                                    BTestSuck.Suck[i][j].On();
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    bD44Arm2CheckVacOn=true;                                    //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                    W64bT2_ProcessIndexSuckDestroy2(1);                                                                 // AI(W64b-Translate) 20260706: golden :688 fiosetview->ProcessIndexSuckDestroy2(1) -- gap stub (see file-top)  //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆 //kevin 20181102 add 提前動作
                    DoRearTestDestroyICDelay.SetMSAndOn(IniConfig.iD44TestHeadCheckVacuumTime);
                    Task=600;
                    break;
                }
                else
                {
                    bD44Arm2CheckVacOn=false;                                   //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                }
                W64bT2_SetHasNullIcToNullIc(BRCarryKit);                                                                // AI(W64b-Translate) 20260706: golden :697 BRCarryKit.SetHasNullIcToNullIc() -- gap stub (see file-top)  //jou 2011-06-29 out shuttle has_null_ic hang up fix    //Steven 20160524 : ADD SetHasNullIcToNullIc
                if(IniConfig.bD44CheckIndexICDestroy)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            bArm2PutDuplicateErr[i][j]=false;
                            if(W64bT2_BNeedCheckGet(i, j))                                                              // AI(W64b-Translate) 20260706: golden :705 BTestSuck.bNeedCheck[i][j] -- gap stub (see file-top)
                            {
                                BTestSuck.Suck[i][j].OffDestroy();              //kevin 20130719 關吹氣
                            }
                        }
                    }
                }
                iIndexTakeDeviceChk2=0;                                         //kevin 20190103 回吸檢測狀態
                if(((iOneCycle && IsInArmOneCycleFinish()) ||
                    (iCleanOut && IsInArmCleanOutFinish())) &&
                    BLCarryKit.UseSiteNoIC())                                   //Sam 20230809 : OneCycle 最後一次 IndexArm 動作，需要等待 OutShuttle 動作做完才能做 IndexCheck，避免黏料壓壞 IC
                    Task=550;
                else
                    return true;
            }
            break;
        case 550:
            if(BRCarryKit.UseSiteNoIC() ||
               InSHT2InRT())
            {
                return true;
            }
            break;
        case 600:
            if(INDEX_SUCKER_TYPE==1)                                                                                    //jou 2012-01-04 需確認Index suck已經完整做完
            {
                bArm2D44SuckCheck=false;
                bArm2D44SuckCheck=W64bT2_ProcessIndexSuckDestroy2(1);                                                   // AI(W64b-Translate) 20260706: golden :732 fiosetview->ProcessIndexSuckDestroy2(1) -- gap stub (see file-top)  //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆
            }
            else
            {
                bArm2D44SuckCheck=true;
            }

            if(DoRearTestDestroyICDelay.Off() && bArm2D44SuckCheck==true)
            {
                flag=false;
                ErrPart=" ";
                bHasErr=false;
                bHasDuplicateErr=false;
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(bArm2PutDuplicateErr[i][j])
                            bHasDuplicateErr=true;
                    }
                }
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(W64bT2_BNeedCheckGet(i, j))                                                                  // AI(W64b-Translate) 20260706: golden :757 BTestSuck.bNeedCheck[i][j] -- gap stub (see file-top)
                        {
                            if(BTestSuck.Suck[i][j].GetStatus())
                            {
                                bHasErr=true;
                                ErrPart+=IndexSuckName[i][j];
                                flag=true;
                                bArm2PutDuplicateErr[i][j]=true;
                                iIndexTakeDeviceChk2=3;                                                                 //kevin 20190103 回吸檢測狀態
                            }
                            else
                            {
                                W64bT2_BNeedCheckSet(i, j, false);                                                      // AI(W64b-Translate) 20260706: golden :769 BTestSuck.bNeedCheck[i][j]=false -- gap stub (see file-top)
                                BTestSuck.Suck[i][j].Normal();
                                bArm2PutDuplicateErr[i][j]=false;
                            }
                        }

                        if(INDEX_SUCKER_TYPE==1)                                                                        //jou 2011-11-01負壓不能一直開著真空，必須關掉
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                bIndexPlaceIcCheck=false;                                                                               //kevin 20110505
                bD44Arm2CheckVacOn=false;                                                                               //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                if(bHasErr)
                {
                    if(MOT[MInArmY].ReadPos()<Prod.YInArm_Shuttle1_Place[iInArmYBase][iInArmXBase])                     //kevin 20181107 shuttle 1
                    {
                       ShowErrorMessage("JAM0327", K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);                        //Vacuum Sensor Off Error jou 2010-11-23
                    }
                    else if(IndexAlarmInArmAway()==true)                                                                //Steven 20130613 : Index異常時, In Arm要先讓位功能
                    {
                        ShowErrorMessage("JAM0327", K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);                       //Vacuum Sensor Off Error jou 2010-11-23
                    }
                    Task=500;                                                                                           //kevin 20181102
                    return false;
                }
                else
                {
                    iIndexTakeDeviceChk2=0;                                                                             //Steven 20250224 回吸檢測狀態
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
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                if(W64bT2_BNeedCheckGet(i, j))                                                          // AI(W64b-Translate) 20260706: golden :814 BTestSuck.bNeedCheck[i][j] -- gap stub (see file-top)
                                {
                                    bIndexPlaceIcCheck=true;                                                            //kevin 20110505
                                    if(INDEX_SUCKER_TYPE==1)                                                            //Steven 20111202
                                    {
                                        fiosetview->bIndexSuck[1][i][j]=true;
                                    }
                                    else
                                    {
                                        BTestSuck.Suck[i][j].On();
                                    }
                                }
                            }
                        }
                    }
                    DoRearTestDestroyICDelay.SetMSAndOn(IniConfig.iD44TestHeadCheckVacuumTime);
                }
            }
            break;
        case 650:                                                               //kevin 2019819 add onecycle index check 4 Site
            if((ArmSpeed_File[IndexArm].bDevicConfirm ||
               (IniConfig.bD44CheckIndexICDestroy &&
                (CosFunction.bD44Use4ea && GetSiteCount()>4))) &&               //JerryYang 20250120 : modify
                INDEX_SUCKER_TYPE==1)                                           //kevin 20190709 add 20190629 回吸檢測一次 4 個 SITE   //kevin 20190530 add index check
            {
                IndexCheck4Site(true, 1, iSiteCount);
                Task=700;
            }
            break;
        case 700:                                                               //kevin 20190709 20190531 index check 4 Site
            if(IndexCheck4Site(false, 1, iSiteCount))
            {
                iSiteCount++;
                if(iSiteCount<TestSocket.iShtCol/2)                             //JerryYang 20250120 : modify
                    Task=650;
                else
                {
                    Task=750;
                    iSiteCount=0;
//                    CheckSocketHasICDelay2.SetSecAndOn(0.5);     //Steven 20110908 : 上來後也要Delay一下
                }
            }
            break;
        case 750:
//            if(CheckSocketHasICDelay2.Off())
            {
                flag=false;
                if((ArmSpeed_File[IndexArm].bDevicConfirm ||
                   (IniConfig.bD44CheckIndexICDestroy &&
                    (CosFunction.bD44Use4ea && GetSiteCount()>4))) &&           //JerryYang 20250120 : modify
                    TotalErrPart!="")                                           //kevin 20190629 add error show
                {
                   flag=true;
                   ShowErrorMessage("JAM0327", K_RETRY, MTestY2, false, TotalErrPart);
                }
                TotalErrPart="";
                if(flag)                                                        //Steven 20120726 : 有跑IC才檢查Socket
                {
                    Task=650;                                                   //kevin 20181102
                    return false;
                }
                else
                {
                    Task=800;                                                   //pass
                }
            }
            break;
        case 800:
            iIndexTakeDeviceChk2=0;                                             //kevin 20190103 回吸檢測狀態
            return true;
    }
    return false;
}

// =============================================================================
//  WAVE 2 gap stubs -- see "WAVE 2 KNOWN GAPS" in the file-top banner for the
//  full rationale on each.  File-scope (static where applicable) so they
//  cannot collide at link time with a future real implementation.
//  Integrate: once the real members/functions/FormsFacade widgets exist, drop
//  this block and the call-site substitutions in DoRearTestSuckIC/
//  ResetInxedArm2Flag, and restore the golden text verbatim.
// =============================================================================
// -- TMyKitSuck::SetUnuseToNullIC (golden MyKitSuck.cpp:354-366) -- absent from
//    aHotPlateSubstrate.h's minimal mirror.  FAITHFUL free-function equivalent:
//    the golden body only touches members already exposed here (SetItemData/
//    iMaxRow/iMaxCol/iShtRow/iShtCol), so this is not an approximation.
static void W64bT2_SetUnuseToNullIC(TMyKitSuck &kit)
{
    for(int i=0; i<kit.iMaxRow; i++)
    {
        for(int j=0; j<kit.iMaxCol; j++)
        {
            if(i>=kit.iShtRow || j>=kit.iShtCol)
                kit.SetItemData(i, j, NULL_IC);
        }
    }
}

// -- APAX_WriteData (golden adam6024.h `void APAX_WriteData(bool bDir, WORD
//    wdata, int iArm=0);`) -- a DIFFERENT DAQ family from the already-shimmed
//    ADAM_* (atester_shims.h); adam6024.cpp itself is not translated.  Offline:
//    no-op (mirrors the ADAM_* convention).  `int` in place of `WORD` for the
//    2nd param avoids depending on <windows.h> WORD being visible in this TU;
//    the only call site passes the literal 0, so this is behaviour-preserving.
static void W64bT2_APAX_WriteData(bool bDir, int wdata, int iArm=0)
{
    APAX_WriteData(bDir, (unsigned short)wdata, iArm);   //AI(W906-ST02-ADAM) 20261002 (St02-E helper H4): the gap stub forwards to golden APAX_WriteData (912 adam6024.cpp:2305-2673, Adam6024Apax_St02.cpp, H3) -- its one caller :2442 is golden 912 aTester_Rear.cpp:2243 (906 :2163); declaration via atester_shims.h (:151) -> Adam6024_St02.h (claim C-1)
}

// -- fMain torque-read UI widgets (golden main.h) -- FormsFacade TfMain
//    (FormsFacade.h) lacks chkReadTorque1/chkReadTorque2/edTorue1.
//    atester.cpp's DoTestHeadMotor (T1) already carries an analogous TU-local
//    seam (W7T1_TfMainTorqueSeam) for the SAME golden gap; this is aTester_Rear.
//    cpp's own independent twin (this file's own W64bT2_ prefix) rather than
//    reaching across translation units.  Integrate: add the real widgets
//    (TCheckBox/TEdit) to TfMain + drop the W64bT2_FMAIN_* call-site macros.
//    AI(W906-W7-F2) 20260729: W64bT2_TCheckSeam / W64bT2_TEditSeam RETIRED -- the
//    members below name vclcompat/Controls.h's unified stand-ins instead (plan D4).
//    Golden classes re-read from golden main.h for this change: chkReadTorque1/2
//    :464-465 TCheckBox, edTorue1 :467 TEdit.  Zero behaviour change (same single
//    member, same default; the only instances are the members of the one file-scope
//    object below, so no by-value copy / aggregate initialisation is affected).
struct W64bT2_TfMainTorqueSeam
{
    TCheckBox chkReadTorque1, chkReadTorque2;                  // golden main.h:464-465 (TCheckBox*)
    TEdit     edTorue1;                                        // golden main.h:467 (TEdit*)
};
static W64bT2_TfMainTorqueSeam W64bT2_fMainTorque;
// golden derefs these as POINTERS (fMain->chkReadTorque1->Checked, ->edTorue1->
// Text); macros yield &member so the golden `->` deref pattern is preserved.
#define W64bT2_FMAIN_CHKREADTORQUE1 (&W64bT2_fMainTorque.chkReadTorque1)
#define W64bT2_FMAIN_CHKREADTORQUE2 (&W64bT2_fMainTorque.chkReadTorque2)
#define W64bT2_FMAIN_EDTORUE1       (&W64bT2_fMainTorque.edTorue1)

// =============================================================================
//  WAVE 2 (AI(W5-aTester_Rear-Translate) 20260710) -- adds DoRearTestSuckIC + three small
//  golden-earlier prerequisite leaves it (and ResetInxedArm2Flag) call.  These
//  three precede ResetArm2PutFlag in golden line order (Wave 1 already
//  translated ResetArm2PutFlag) but are appended here, after
//  DoRearTestDestroyIC, to avoid touching Wave-1 reviewed code -- same
//  out-of-golden-order append convention already used by aHotPlateSubstrate.h's
//  "-- W6.x ADD" blocks.
// =============================================================================
//------------------------------------------------------------------------------
// golden :100-113 (Steven 20241017 : 打包成function)
void ResetInxedArm2Flag()
{
    for(int i=0; i<BTestSuck.iMaxRow; i++)
    {
        for(int j=0; j<BTestSuck.iMaxCol; j++)
        {
            bArm2SuckFinish[i][j]=false;                                        //Steven 20110301 : 初始化，都當作還沒做完
            bArm2DuplicateErr[i][j]=false;
            BTestSuck.Suck[i][j].Reset();                                       //Steven 20140213 : Jordan說Index下去不吸直接Alarm
        }
    }

    W64bT2_SetUnuseToNullIC(BLCarryKit);                                        // AI(W5-aTester_Rear-Translate) 20260710: golden :107 BLCarryKit.SetUnuseToNullIC() -- gap stub, see below (Steven 20241017 : 清除沒用到的資料)
}
//------------------------------------------------------------------------------
// golden :115-124 (Steven 20241017 : 打包成function)
void ResetArm2VacFlag()
{
    for(int i=0; i<BTestSuck.iMaxRow; i++)
    {
        for(int j=0; j<BTestSuck.iMaxCol; j++)
        {
            bSkipNeedCheckVac[1][i][j]=false;
        }
    }
}
//------------------------------------------------------------------------------
// golden :137-168 (wei 20151228 No FullSite delay)
void CheckBTFullSite()
{
    int iCount=0, iUseSiteCount=0;

    iArmsitecount[1]=0;
    for(int i=0; i<BTestSuck.iShtRow; i++)
    {
        for(int j=0; j<BTestSuck.iShtCol; j++)
        {
            if(BTestSuck.Item[i][j]==HAS_IC ||
               BTestSuck.Item[i][j]==HAS_HOT_IC)
            {
                iCount++;
            }
            iUseSiteCount++;
        }
    }

    if(iCount!=0)
    {
        iArmsitecount[1]=iUseSiteCount-iCount;
        if(iCount!=iUseSiteCount)
        {
            iInitContactCount=0;                                                //Steven 20160519 : 恆溫時溫度要補Offset
            bDoWhenNoFullSiteUseInitialDelay=true;
        }
        else
        {
            bDoWhenNoFullSiteUseInitialDelay=false;
        }
    }
}

// golden :67/:69 -- two more file-scope prerequisites of DoRearTestSuckIC that
// precede bArm2PutDuplicateErr (:89, Wave 1's first translated line); appended
// here (out of golden order, same convention as the rest of this Wave-2 block)
// rather than inserted at the true top of the file.
static bool bIndexZ2NeedUp=false;                                               //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
extern bool CheckCFixTrayFullPlace();                                           // real body already lives in the translated acarry.cpp:225 (golden acarry.cpp:225); golden itself forward-declares this locally rather than via a header, mirrored verbatim
//------------------------------------------------------------------------------
TQPF_Timer hDoRearTestSuckIC;
TQPF_Timer hDoRearTestSuckICdelay;                                              //JerryYang 20220923 : add
int iRearTestSuckICTask=1;
void InitRearTestSuckICTask()
{
    iRearTestSuckICTask=1;
}
//------------------------------------------------------------------------------
extern int CheckOneCycleAction(int iTask);                                      //Steven 20240326 : 判斷one cycle的時候要不要繼續放料
TQPF_Timer dwStartInitialCount2;
bool DoRearTestSuckIC()
{
    static int iHomeAlarm=0;
    static bool bOverHappen=false, bHasDropAtShuttle=false, bHasICErr=false;
    static AnsiString ErrPart2="";
    int ZPos=0;
    int sp=0;
    int &Task=iRearTestSuckICTask, ret=0;
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

            if(iOneCycle &&
               bUseTwoArm32Site &&                                              //針對NN mode的one cycle動作做額外卡控
               BTestSuck.UseSiteNoIC() &&                                       //Arm 2沒料
               BLCarryKit.UseSiteNoIC())
            {
                if(FTestSuck.HasRealIC()==false &&                              //Arm 1有料
                   FLCarryKit.HasRealIC()==false)
                {
                    FTestSuck.SetAllToNullIC();
                    FLCarryKit.SetAllToNullIC();
                    MOT[MInShuttle2].fCanMoveM=true;
                    return true;
                }
                else
                {
                    if(iOneCycle && IsInArmOneCycleFinish())                    //Sam 20250417 : 修正 AutoSiteOff 後 OneCycle Hangup 問題
                    {
                        return true;
                    }
                    else if(InSHT2InLF())                                       //等待
                    {
                        MOT[MInShuttle2].fCanMoveM=false;
                        return false;
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            if((iOneCycle && IsInArmOneCycleFinish()) ||
               (iCleanOut && IsInArmCleanOutFinish()))
            {
                if(BTestSuck.UseSiteNoIC() &&
                   BLCarryKit.UseSiteNoIC() &&
                   InArmSuck.HasRealIC()==false)                                //JerryYang 20170801 (wei) 修正按one cycle hang up問題
                {
                    MOT[MInShuttle2].fCanMoveM=true;                            //RogerYang 20250820 : 1 --> 2
                    return true;
                }
            }

            if(iCleanOut          &&
               BLCarryKit.UseSiteNoIC()  &&
               InArmSuck.UseSiteNoIC()   &&
               MOT[MMPlate1].HasIC()==false &&
               MOT[MMPlate2].HasIC()==false)
            {
                return true;
            }

            if(CheckCFixTrayFullPlace() &&
               MOT[MTestZ2].Led[iHomeLed]==true)                                //Steven 20220524 : fixed for fix 3 氣缸 Hang up
            {
                MOT[MInShuttle2].fCanMoveM=true;
                return false;
            }

            if(BLCarryKit.UseSiteHasIC())
            {
                if(CanYieldAlarmRemainInSHT()==true)                            //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                {
                    return true;
                }

                if(InSHT2InRT()==false)
                {
                    if((bUseTwoArm32Site==false && IndexStatus==Z1Down_Z2Up) ||
                       (bUseTwoArm32Site==true  && IndexStatus==Z1_Z2_Normal))
                        MOT[MInShuttle2].fCanMoveM=true;
                    return false;
                }

                if(bReadAndCheckCPUName==true &&
                   CosFunction.bInShuttleDetectByLatch &&                       //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
                   Prod.bF18InshuttleDetect &&
                   MOTION_CARD_TYPE==MotionCard_Contec &&
                   LastSet.iRealDummy==REALLY)                                  //jou 20170418 (Steven) : 矽品-世明要求修改config F18至Recipe設定
                {
                    if(bInSht2LtcDetectTesterCanMove==false)
                    {
                        return false;
                    }
                }

                if((Prod.bF18InshuttleDetect ||                                 //Sam 20250925 : 修正關 Arm 後檢查 F18 功能被卡死 hangeup
                    IniConfig.bF20InShuttleProminentDetect) &&
                    CosFunction.bInShuttleDetectByLatch==false)
                {
                    if(bF18CheckShuttle2MustHasIC)
                        break;
                }

                if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                   bInSh2DoLtc==true)                                           //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
                    return false;
                MOT[MInShuttle2].fCanMoveM=false;
                bSuckingFlagZ2=true;
                Task=300;
                bIndexZ2NeedUp=false;                                           //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
            }
            break;
        case 300:
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&                   //ChungHung 20120717 add Index Drop Error Can Retry and Start
               bShuttle2MoveToRight)                                            //ChungHung 20131015 fix hangup
            {
                if(InSHT2InRT()!=true)
                {
                    return false;
                }
            }
            bIndexPickUpErrMoveSht2=false;                                      //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
            MOT[MInShuttle2].fCanMoveM=false;
            bShuttle2MoveToRight=false;
            bShuttle2HasPickErr=false;                                          //Steven 20220712 : 避免In arm 偷放料
            if(CosFunction.bPurgeBeforePickShuttle)                             //JerryYang 20160906 PurgeBeforePickShuttle改成可以By客戶功能開啟
                InitRearTestPurgBeforePickShuttle();                            //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue Start
            flag2=false;
            Task=301;
            if(IniConfig.bIndexPickupWait==true &&
               (LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot) &&
               Temperature.iInitialStart2Time!=0)                               //kevin 20180905 (Steven) : add  iInitialStart2Time
            {
                bInitialStart2Time=true;                                        //wei 20171020 (jou) InitialStart1 秒數倒數
            }

            if(IsNNMode()==NN_2Row)
                break;
        case 301:                                                                                                                                               //確認Z軸位置 位置在下->吸取IC->Z軸到位後->Task=310
            if(IniConfig.bIndexPickupWait==true &&
               (LastSet.iTemperature==Tempture_Hot ||                                                                                                           //jou 2012-06-29 Index Pick up need wait Soak Time
                LastSet.iTemperature==Tempture_AmbientHot) &&                                                                                                   //kevin 20180903 (Steven) : add 恆溫控制
               Temperature.iInitialStart2Time!=0)
            {
                dwEndShuttle2Soak=MyTickCount();                                                                                                                //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常  //kevin 20180905 add  iInitialStart2Time
                if(iInitialStart2Count>0)
                {
                    iInitialStart2Count=(Temperature.iInitialStart2Time)-((dwEndShuttle2Soak-dwStartShuttle2Soak)/1000);                                        //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常
                }

                if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)                                                                                            //Steven 20161213 : Fix iInitialStart2 關ARM Hangup
                {
                }
                else
                {
                    if(BLCarryKit.TSoakTime.Off()==false)
                        break;
                }
                bInitialStart2Time=false;                                                                                                                       //JerryYang 20181001 (Steven) : fix Shuttle soak time 倒數秒數異常
            }

            if(bNeedTesterDuckingFinishLightYellowAndAlarmOn)                                                                                                   //ChungHung 20141015 add for SCK When the handler start running (finished Start count function),Yellow light blinking & alarm on (press "alarm reset" then clear)
            {
                bNeedTesterDuckingFinishLightYellowAndAlarmOn=false;
                bTesterDuckingFinishLightYellowAndAlarmOn=true;
            }

            if(TestIF_File.bIndexPickICWhenOutShtNoIC)                                                                                                          //Steven 20221207 : Index必須在out shuttle沒料才可以吸
            {
                if(BRCarryKit.HasRealIC())
                {
                    return false;
                }
            }

            if(TestIF.iShuttleMode==1 &&                                                                                                                        //jou 980317 SOFT_SIMULTE hang
               TestIF.iShuttle_Sel!=0 &&
               bCheckShuttle2Flag)
                return false;
            MOT[MInShuttle2].ScanMotorStatus();
            if(BLCarryKit.UseSiteHasIC() && bCheckShuttle2Flag==false)
            {
                flag2=false;
                if(CheckZ1IsDown())
                {
                    if(BLCarryKit.HasRealIC()==false)
                    {
                        flag2=true;
                    }
                    else
                    {
                        ZPos=MOT[MTestZ1].Gali_ReadPos();
                        //==> Eastsun 20260511 F007 整合: Ifor 20240430 add:secs gem cmd Index Yiel Fail
                        if(CUSTOMER_CODE==CC_KYEC_LEE && iSECSGEM_ConsecutiveFailureAlarm==2)
                        {
                        }
                        else if(CosFunction.bSortingBy2DList==true &&           //jou Index更換Offset會hang up? 2010-04-13
                           LastSet.iTester==_2D_SORT &&
                           TestIF_File.bSortingBy2DIDList==true)                                                                                                //Frank 20221122 : 2DID sorting for ATK
                        //<== Eastsun 20260511 F007 整合
                        {
                        }
                        else
                        {
                            if(bUseTwoArm32Site==false)
                            {
                                if(ZPos>=(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+500))                                                                        //2008/06/24
                                {
                                    if(ZPos!=iBackUpZ1DownPosition)
                                        return false;
                                }
                            }
                        }

                        if(MOT[MTestZ2].Gali_ReadPos()<(Prod.TestZ2_Pick+1000))                                                                                 //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                        {
                            bZ2PickShuttle=true;
                        }

                        if(flag2==false)
                        {
                            bSuckingFlagZ2=true;                                                                                                                //Steven 20240916 : index下降到shuttle吸放料
                            sp=GetIndexZSpeed(1);                                                                                                               //Steven 20160524 : Index Z軸速度整合為Function
                            if(CosFunction.bPurgeBeforePickShuttle)                                                                                             //JerryYang 20160906 PurgeBeforePickShuttle改成可以By客戶功能開啟
                            {
                                if(DeviceForm_File.bPurgeBeforePickShuttle)                                                                                     //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue
                                {
                                    if(DoRearTestPurgBeforePickShuttle(sp)==false)
                                        break;
                                }
                            }
                            #ifdef DEBUG_INDEX_UPH
                            flag2=MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Pick, iIndexSpeed, iIndexAcc);                                                         //Steven 20170818 : 測試UPH用
                            #else
                            flag2=MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Pick, sp);
                            #endif
                        }
                    }
                }
            }
            else if(CosFunction.bIndexPickErrSkipNeedCheckVac &&                                                                                                //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                    IniConfig.bD50IndexPickErrSkipNeedCheckVac &&
                    bArm2PressSkipNeedDownCheckVac==true &&
                    BLCarryKit.UseSiteNoIC())
            {
                flag2=false;
                if(CheckZ1IsDown())
                {
                    ZPos=MOT[MTestZ1].Gali_ReadPos();
                    //==> Eastsun 20260511 F007 整合: Ifor 20240430 add:secs gem cmd Index Yiel Fail
                    if(CUSTOMER_CODE==CC_KYEC_LEE && iSECSGEM_ConsecutiveFailureAlarm==2)
                    {
                    }
                    else if(IsNNMode()!=NN_2Row)                                                                                                                     //jou Index更換Offset會hang up? 2010-04-13
                    //<== Eastsun 20260511 F007 整合
                    {
                        if(ZPos>=(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+500))                                                                                //2008/06/24
                        {
                            if(ZPos!=iBackUpZ1DownPosition)
                                return false;
                        }
                    }

                    if(MOT[MTestZ2].Gali_ReadPos()<(Prod.TestZ2_Pick+1000))                                                                                     //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                    {
                        bZ2PickShuttle=true;
                    }

                    if(flag2==false)
                    {
                        bSuckingFlagZ2=true;                                                                                                                    //Steven 20240916 : index下降到shuttle吸放料
                        if(CUSTOMER_CODE==CC_AMKOR_Philippines)                                                                                                 //Frank QQ
                        {
                            flag2=true;
                        }
                        else
                        {
                            #ifdef DEBUG_INDEX_UPH
                            flag2=MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Pick, iIndexSpeed, iIndexAcc);                                                         //Steven 20170818 : 測試UPH用
                            #else
                            flag2=MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Pick, iSpeedSlow);                                                                      //JerryYang 20170610 這裡改慢速
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

            if(LastSet.iRealDummy==REALLY && ArmSpeed[IndexArm].bSuckOnDown && INDEX_SUCKER_TYPE==0)
            {
                for(int i=0; i<BLCarryKit.iShtRow; i++)
                {
                    for(int j=0; j<BLCarryKit.iShtCol; j++)
                    {
                        if(BLCarryKit.Item[i][j]!=NULL_IC &&
                           BLCarryKit.Item[i][j]!=HAS_NULL_IC)
                        {
                            BTestSuck.Suck[i][j].On();
                        }
                    }
                }
            }

            if(flag2)
            {
                RecordIndexPosition(2, 0);                                                                                                                      //Isaac 20200922 : 紀錄indexArmY encoder值和command值，Arm2/Shuttle
                EncoderTeachingMaxMinCount(2);                                                                                                                  //Isaac 20201012 : 每次完成動作，比較紀錄Encoder和Teaching點的差值
                flag2=false;                                                                                                                                    //JerryYang 20170610 (wei) 移到上面
                bZ2PickShuttle=true;                                                                                                                            //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                if(CosFunction.bIndexPickErrSkipNeedCheckVac &&
                   IniConfig.bD50IndexPickErrSkipNeedCheckVac &&
                   bArm2PressSkipNeedDownCheckVac==true &&
                   BLCarryKit.UseSiteNoIC())                                                                                                                    //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(bSkipNeedCheckVac[1][i][j]==true)
                            {
                                bArm2SuckFinish[i][j]=false;                                                                                                    //Steven 20110301 : 初始化，都當作還沒做完
                                bArm2DuplicateErr[i][j]=false;
                                BTestSuck.Suck[i][j].Reset();                                                                                                   //Steven 20140213 : Jordan說Index下去不吸直接Alarm
                            }
                        }
                    }
                    Task=307;                                                                                                                                   //JerryYang 20170610 (wei) 這裡跳去檢查按skip的真空
                    break;
                }
                ResetInxedArm2Flag();                                                                                                                           //Steven 20241017 : 打包成function
                if(CosFunction.bUseShuttlePickShiftDetect==true &&
                   DeviceForm_File.dDropByPassDetect!=0)                                                                                                        //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
                {
                    if(TestIF_File.iShuttleMode==0 ||                                                                                                           //Ifor 20221013 add:關Arm不處理避免資料轉移異常
                       (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1))
                    {
                        bIndexZ2NeedUp=true;
                    }
                    else
                    {
                        bIndexZ2NeedUp=false;
                    }
                }
                else
                {
                    bIndexZ2NeedUp=false;
                }

                if(USE_IO_CHANGE_TOQUE==true &&                                                                                                                 //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
                   (bNeedCheckIndexToque==true ||
                    bNeedCheckIndexToque2==true))                                                                                                               //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
                {
                    bNeedCheckIndexToque=false;
                    bNeedCheckIndexToque2=false;
                    W64bT2_FMAIN_CHKREADTORQUE1->Checked=false;
                    W64bT2_FMAIN_CHKREADTORQUE2->Checked=true;
                    W64bT2_FMAIN_EDTORUE1->Text="";
                    bOverHappen=false;
                    hDoRearTestSuckIC.SetSecAndOn(1);
                }
                bHasDropAtShuttle=false;
                if(IniConfig.bD81IndexCheckVacuumOnShuttle)
                {
                    for(int i=0; i<MAX_Index_Row; i++)                                                                                                          //JerryYang 20241002 : add
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(BLCarryKit.Item[i][j]==NULL_IC || BLCarryKit.Item[i][j]==HAS_NULL_IC)
                            {
                                if(INDEX_SUCKER_TYPE==1 && bDropAtSht2NeedCheckVac[i][j]==true)                                                                 //Steven 20111202
                                {
                                    fiosetview->bIndexSuck[1][i][j]=true;
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
            bArm2SuckComplete=true;
            ErrPart="";
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bSkipNeedCheckVac[1][i][j] &&
                       BLCarryKit.Item[i][j]==NULL_IC)
                    {
                        if(BLCarryKit.Item[i][j]==NULL_IC &&
                           bArm2SuckFinish[i][j]==true)                         //Steven 20110301
                        {
                            if(BTestSuck.Item[i][j]==HAS_NULL_IC ||
                               BTestSuck.Item[i][j]==NULL_IC)
                            {
                                BTestSuck.Suck[i][j].Normal();                  //Steven 20111201 : 預防負壓壓降
                            }
                        }
                        else
                        {
                            if(BTestSuck.Item[i][j]==HAS_NULL_IC &&
                               BTestSuck.Suck[i][j].Suck())
                            {
                                bHasErr=true;                                   //按skip後卻吸到IC，要跳alarm
                                bArm2DuplicateErr[i][j]=true;
                                bArm2SuckFinish[i][j]=true;
                            }
                            else if(BTestSuck.Suck[i][j].Error)                 //Steven 20110301 : 有錯誤的不做
                            {
                                bArm2SuckFinish[i][j]=true;
                            }
                            else if(BTestSuck.Item[i][j]==HAS_IC &&
                                    BTestSuck.Suck[i][j].Suck())                //Ifor 20171122 (Steven) :add 修正bArmSuckFinish 資料異常造成Hangup
                            {
                                bArm2SuckFinish[i][j]=true;
                            }
                            else
                            {
                                bArm2SuckComplete=false;                        //jou 2011-08-16 只要有未完成的就繼續等
                            }
//                            else
//                                flag1=false;                                  //jou 2011-08-16 只要有未完成的就繼續等
                        }
                    }
                    else
                    {
                        if(BTestSuck.Item[i][j]==HAS_NULL_IC ||
                           BTestSuck.Item[i][j]==NULL_IC)
                            BTestSuck.Suck[i][j].Normal();                      //Steven 20111201 : 預防負壓壓降
                        bArm2SuckFinish[i][j]=true;                             //Steven 20110301 : 沒有東西的地方要跳過
                    }
                }
            }
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bArm2SuckFinish[i][j]==false)                            //只要有未完成的就繼續等
                        bArm2SuckComplete=false;
                }
            }

            if(bArm2SuckComplete==true)                                         //Steven 20110301 : 所有吸嘴都做完
            {
                bArm2SuckComplete=false;
                if(bHasErr)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(bSkipNeedCheckVac[1][i][j]==true &&
                               bArm2DuplicateErr[i][j]==true)
                                BTestSuck.Suck[i][j].Normal();
                        }
                    }
                    Task=308;
                    return false;
                }

                if(BLCarryKit.HasRealIC())
                    break;
                ZeroMemory(bArm2DuplicateErr, sizeof(bArm2DuplicateErr));
                if(BLCarryKit.UseSiteHasIC())
                    break;
                iD43AutoRetryWhenIndexPickErrCnt[0]=0;
                bResetIndexArm1Pick=false;
                bArm2PressSkipNeedDownCheckVac=false;
                ResetArm2VacFlag();                                             //Steven 20241017 : 打包成function
                Task=500;
            }
            break;
        case 308:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, iSpeedSlow))         //JerryYang 20180628 (wei) 只有pick up error的arm要上升
            {
                bSuckingFlagZ2=false;                                           //Steven 20240916 : index下降到shuttle吸放料
                Task=309;
            }
            break;
        case 309:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                bArm2PressSkipNeedDownCheckVac=false;
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(bSkipNeedCheckVac[1][i][j]==true)
                            ErrPart+=IndexSuckName[i][j];
                        if(CUSTOMER_CODE!=CC_AMKOR_Philippines)                 //Frank QQ
                            bSkipNeedCheckVac[1][i][j]=false;
                    }
                }

                if(CUSTOMER_CODE==CC_AMKOR_Philippines)                         //Frank QQ
                {
                    bArm2PressSkipNeedDownCheckVac=true;
                    str.sprintf("Arm2 detect %s has device",ErrPart);
                    ShowMyMessage(str,"請檢查Arm2上是否有IC");
                    ErrPart="";
                    Task=3091;
                }
                else
                {
                    str.sprintf("Arm2 detect Shuttle %s has device",ErrPart);
                    ShowMyMessage(str,"請檢查shutle2上是否有IC");
                    ErrPart="";
                    Task=500;
                }
            }
            break;
        case 3091:
            ResetInxedArm2Flag();                                               //Steven 20241017 : 打包成function
            Task=307;
            break;
        case 3099:                                                              //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
            bSuckingFlagZ2=true;                                                //Steven 20240916 : index下降到shuttle吸放料
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Pick+DeviceForm_File.dDropByPassDetect*100, iSpeedSlow))
            {
                ResetInxedArm2Flag();                                           //Steven 20241017 : 打包成function
                Task=310;
            }
            break;
        case 310:                                                                                                       //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
            if(USE_IO_CHANGE_TOQUE==true && bOverHappen==false)                                                         //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
            {
                if(W64bT2_FMAIN_EDTORUE1->Text!="")
                {
                    if(atoi(W64bT2_FMAIN_EDTORUE1->Text.c_str())>=ciIndex5FSetValue)
                    {
                        ShowMyMessage("Index Z2 Torque Over,Z2 Pick from Shuttle need up","Index Z2 Torque 過大,需往上調整");
                    }
                    bOverHappen=true;
                }
                else if(hDoRearTestSuckIC.Off()==false)
                {
                    break;                                                                                              //Wait Read Torque
                }
                else
                {
                    bOverHappen=true;                                                                                   //Over Wait Time
                }
            }
            bArm2NeedSuck=true;                                                                                         //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
            bArm2SuckComplete=true;
            DoArm2Suck();                                                                                               //JerryYang 20190123 把index arm吸真空&交換狀態包成函式
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bArm2SuckFinish[i][j]==false)                                                                    //只要有未完成的就繼續等
                        bArm2SuckComplete=false;
                }
            }

            if(bArm2SuckComplete==true)                                                                                 //Steven 20110301 : 所有吸嘴都做完
            {
                if(bIndexZ2NeedUp)                                                                                      //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
                {
                    bIndexZ2NeedUp=false;
                    Task=3099;
                    break;
                }
                BTestSuck.HotCount=BLCarryKit.HotCount;                                                                 //JerryYang 20230204 : add hot plate放置順序資料
                if(IniConfig.bD81IndexCheckVacuumOnShuttle && bHasDropAtShuttle==true)                                  //JerryYang 20241002 : add
                {
                    if(INDEX_SUCKER_TYPE==1)
                    {
                        if(W64bT2_ProcessIndexSuckDestroy2()==true) // AI(W5-aTester_Rear-Translate) 20260710: golden :1485 fiosetview->ProcessIndexSuckDestroy2() -- reuses Wave-1 gap stub (see file-top)
                        {
                            bHasICErr=false;
                            ErrPart2="";
                            for(int i=0; i<MAX_Index_Row; i++)
                            {
                                for(int j=0; j<NEW_MAX_Index_Col; j++)
                                {
                                    #ifdef SOFT_SIMULTE
                                    if((BTestSuck.Item[i][j]==NULL_IC || BTestSuck.Item[i][j]==HAS_NULL_IC) && i==0 && j==3)
                                    #else
                                    if((BTestSuck.Item[i][j]==NULL_IC || BTestSuck.Item[i][j]==HAS_NULL_IC) && BTestSuck.Suck[i][j].GetStatus()==true)
                                    #endif
                                    {
                                        bHasICErr=true;
                                        bSHT2_DropPosHasIC[i][j]=true;
                                        if(TestIF_File.iTestMode==_32Site4X8M ||
                                           TestIF_File.iTestMode==_32Site4X8N ||
                                           TestIF_File.iTestMode==_16Site4X4)                                           //Sam 20190226 : 16Site4X4 ///kevin 20180504 add  error pos
                                            ErrPart2+=IndexSuckName[i+2][j];
                                        else if(TestIF_File.iTestMode==QualSite2X2N ||
                                                TestIF_File.iTestMode==_6Site2X3N)
                                            ErrPart2+=IndexSuckName[i+1][j];                                            //Steven 20230712 : 修正NN mode alarm顯示
                                        else
                                            ErrPart2+=IndexSuckName[i][j];
                                        BTestSuck.Suck[i][j].Normal();
                                    }
                                }
                            }

                            if(bHasICErr==false)
                            {
                                for(int i=0; i<MAX_Index_Row; i++)
                                {
                                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                                    {
                                        bDropAtSht2NeedCheckVac[i][j]=false;
                                        bSHT2_DropPosHasIC[i][j]=false;
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
                bHasErr=false;
                //──── DEBUG: 強制觸發 JAM0302 Rear pick-up error ────
//                static bool bDebugForceJAM0302Rear = true;   // ← 設 true 重現，false 關閉
//                if(bDebugForceJAM0302Rear)
//                {
//                    bHasErr = true;
//                    BTestSuck.Suck[0][0].Error = true;
//                    bDebugForceJAM0302Rear = false;           // 只觸發一次
//                }
                //──── DEBUG END ────
                bArm2NeedSuck=false;
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error)
                        {
                            bHasErr=true;
                            if(USE_IO_CHANGE_TOQUE==true)                                                               //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
                            {                                                                                           //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
                                W64bT2_FMAIN_CHKREADTORQUE1->Checked=false;
                                W64bT2_FMAIN_CHKREADTORQUE2->Checked=true;
                                W64bT2_FMAIN_EDTORUE1->Text="";
                                bOverHappen=false;
                                hDoRearTestSuckIC.SetSecAndOn(1);
                            }

                            if(IniConfig.bD62PickUpErrorNeedPurge)                                                      //Steveb 20161024 : 吸取異常需要吹氣一次
                            {
                                BTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }

                if(bHasErr || (IniConfig.bD81IndexCheckVacuumOnShuttle && bHasICErr))
                {
                    bShuttle2HasPickErr=true;                                                                           //Steven 20230118 : 避免In arm 偷放料, 往上移動
                    bIndexPickUpErrMoveSht2=true;                                                                       //Steven 20221107 : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
                    if(CosFunction.bIndexPickErrSkipBlowAirTime)                                                        //Frank QQ
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

                if(BLCarryKit.HasRealIC())
                    break;
                if(TestIF_File.iShuttleMode==0 ||
                   (TestIF_File.iShuttleMode==1 &&
                    TestIF_File.iShuttle_Sel==1))                                                                       //Jou 20230224 : 修正Auto Site Map關arm異常
                {
                    if(bAutoSiteMapWaitTestResult==true &&
                       BTestSuck.HasRealIC()==false &&
                       FTestSuck.HasRealIC()==false &&                                                                  //Steven 20200326 : 修正JCET Auto site map發生inarm掉料會hang up
                       TestSocket.HasRealIC()==false &&
                       FLCarryKit.HasRealIC()==false &&
                       BLCarryKit.HasRealIC()==false)
                    {
                        if(IsNNMode()==NN_2Row)
                        {
                        }
                        else
                        {
                            bAutoSiteMapWaitTestResult=false;
                        }
                    }
                }
                ZeroMemory(bArm2DuplicateErr, sizeof(bArm2DuplicateErr));
                if(BLCarryKit.UseSiteHasIC())
                    break;
                bResetIndexArm2Pick=false;
                if(CUSTOMER_CODE==CC_TSMC_TAINAN &&
                   (Prod.bWhenNoFullSiteUseInitialDelay &&                                                              //wei 20161102 No FullSite delay修改||->&&
                    IniConfig.bL18NofullsiteaddTemperatureoffset))                                                      //wei 20151228 No FullSite delay
                {
                    CheckBTFullSite();
                }
                Task=500;
                if(DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==true &&
                   LastSet.iTemperature==Tempture_Hot &&                                                                //JerryYang 20220805 : 下壓shuttle預熱功能initial第一次吸shuttle時需下壓等待Jam soak time
                   FTestSuck.HasRealIC()==false)
                {
                    hDoRearTestSuckICdelay.SetSecAndOn(Temperature.fJamSoakTime);
                }
                iD43AutoRetryWhenIndexPickErrCnt[1]=0;                                                                  //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
            }
            break;
        case 3051:                                                              //Frank QQ
            bSuckingFlagZ2=true;                                                //Steven 20240916 : index下降到shuttle吸放料
            if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Place, MOT[MTestZ2].GailSpeed, 0))
            {
                hDoRearTestSuckIC.SetSecAndOn(IniConfig.iD62IndexBlowAirTime);
                Task=3052;
            }
            break;
        case 3052:                                                              //Frank QQ
            if(hDoRearTestSuckIC.Off())
            {
                Task=305;
            }
            break;
        case 305:
            if(USE_IO_CHANGE_TOQUE==true &&                                     //jou 2012-06-12 即時更新扭力值不能開，不然會衝突
               bOverHappen==false)                                              //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
            {
                if(W64bT2_FMAIN_EDTORUE1->Text!="")
                {
                    if(atoi(W64bT2_FMAIN_EDTORUE1->Text.c_str())>=ciIndex5FSetValue)
                    {
                        ShowMyMessage("Index Z2 Torque Over,Z2 Pick from Shuttle need up","Index Z2 Torque 過大,需往上調整");
                    }
                    bOverHappen=true;
                }
                else if(hDoRearTestSuckIC.Off()==false)
                {
                    break;                                                      //Wait Read Torque
                }
                else
                {
                    bOverHappen=true;                                           //Over Wait Time
                }
            }
            CheckIndexAllSuckICFallDown(false, true);                           //Steven 20110725 : 修改負壓檢查方式
            if(MOT[MTestZ2].Gali_ReadPos()>(Prod.TestZ2_Pick+1000))             //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
            {
                bZ2PickShuttle=false;
            }
            #ifdef DEBUG_INDEX_UPH
            if(MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Safe, iIndexSpeed, iIndexAcc))
            #else
            if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed, 0))
            #endif
            {
                bSuckingFlagZ2=false;                                           //Steven 20240916 : index下降到shuttle吸放料
                Task=306;                                                       //Steven 20160718 : Index pick up error with [D43]
            }
            break;
        case 306:
            bZ2PickShuttle=false;                                               //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
            {
                if(BRCarryKit.UseSiteNoIC())                                    //ChungHung 20120717 add Index Drop Error Can Retry and Start
                {
                    MOT[MInShuttle2].fCanMoveM=true;
                    bShuttle2MoveToLeft=true;
                    bShuttle2HasPickErr=true;                                   //Steven 20220712 : 避免In arm 偷放料
                    bIndexPickErrShtStayRight2=false;                           //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                    bCheckNullIC2=false;                                        //JerryYang 20170623 (wei) 修正有裝out shuttle 前後對照的機台發生index arm吸取異常無法跳出alram造成hang up
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
               bShuttle2MoveToLeft)                                             //ChungHung 20131015 fix hangup
            {
                if(InSHT2InLF()!=true)
                {
                    return false;
                }
            }
            bShuttle2MoveToLeft=false;
            MOT[MInShuttle2].fCanMoveM=false;
            Task=321;
            if(IsNNMode()==NN_2Row)
                break;
        case 321:                                                                                                       //ChungHung 20130924 add
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            bSht1OnlyHasICErr=false;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bArm2DuplicateErr[i][j])
                        bHasDuplicateErr=true;
                    if(BTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i][j];
                        if(IniConfig.bD62PickUpErrorNeedPurge)                                                          //Steveb 20161024 : 吸取異常需要吹氣一次
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddErrorRecordNoSave is a real body (MyProductionRecord.cpp.obj, nm) -- golden :1730
                        BLCarryKit.PordRec[i][j].AddErrorRecordNoSave("JAM0302");
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                    }
                    else
                    {
                        BTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(bHasErr)
            {
                if(IndexAlarmInArmAway()==false)                                                                        //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    return false;
                }
                bHasErr=false;
                if(IniConfig.bD81IndexCheckVacuumOnShuttle && bHasICErr==true)
                {
                    bHasICErr=false;
                    str.sprintf("Need to remove device of Shuttle2 %s", ErrPart2);
                    ShowMyMessage(str);
                }

                if(IniConfig.bD43AutoRetryWhenIndexPickErr &&                                                           //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                   iD43AutoRetryWhenIndexPickErrCnt[1]==0)
                {
                    ret=K_RETRY;
                }
                else if(IniConfig.bNewResetFunction==true &&
                        bResetIndexArm2Pick==true)
                {
                    ret=K_SKIP;
                }
                else
                {
                    if(CosFunction.bJAM0301NeedOpenChamberDoor)                                                         //wei : JAM0301 & JAM0302需要開啟Chamber門10秒
                    {
                        bIsTestSitICFallDown=true;
                    }

                    if(IniConfig.bIndexPickErrOnlySKIP==true ||                                                         //jou 2012-02-13 index pick-up error only skip
                       IniConfig.bD64IndexPickErrOnlySKIP)                                                              //kevin 20171103 (wei) add retry function
                        ret=ShowErrorMessage("JAM0302", K_SKIP, MTestZ2, bHasDuplicateErr, ErrPart);                    //Devicr Pick-Up Error
                    else
                        ret=ShowErrorMessage("JAM0302", K_SKIP|K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);            //Devicr Pick-Up Error
                }

                if(ret==K_SKIP)
                {
                    if(LastSet.iRunStartMode==rsmAutoSiteMap)
                        iAutoSiteMapCount++;                                                                            //Steven 20220811 : 紀錄目前是哪個Shuttle
                    bAutoSiteMapWaitTestResult=false;                                                                   //Ifor 20180115 (Steven) : add Site Mapping SKIP 需清除旗標
                    bIndexPickUpErrMoveSht2=false;                                                                      //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
                    iD43AutoRetryWhenIndexPickErrCnt[1]=0;                                                              //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Item[i][j]==NULL_IC)                                                           //jou 2011-12-27 有發生過Skip又重吸一次,改成下面的方式
                            {
                                if(BTestSuck.Suck[i][j].Error)                                                          //JerryYang 20200303 fix 沒IC的地方jam count被++
                                {
                                    if(CosFunction.bUseSCKART)                                                          //Steven 20161214 (wei) : For SCK ART
                                    {
                                        if(TestIF_File.bRENESAS_EnableFTCT==true)                                       //RogeryYang 20251014 : FTCT add to HdRejectBT
                                            fSCKART->iInputJamCnt++;
                                        else
                                            fSCKART->AddOutputJamCnt(i, j, ret);                                        //RogerYang 20250923 : 整合ART OutArm JamCount
                                    }
                                }
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddErrorRecord is DECLARED and links, but today it resolves to the no-op stand-in at aHotPlateSubstrate.cpp:773 (body gated G-5, see Public/MyProductionRecord.h BODY STATUS) -- call site now faithful, behaviour unchanged until G-5 opens -- golden :1800
                                BLCarryKit.PordRec[i][j].AddErrorRecord("JAM0302");                                     //Steven 20161214 : 加上Index異常Skip的ErrorLog
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                                BTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                BTestSuck.Suck[i][j].Normal();
                                if(CosFunction.bIndexPickErrSkipNeedCheckVac==true &&
                                   IniConfig.bD50IndexPickErrSkipNeedCheckVac)                                          //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
                                {
                                    bSkipNeedCheckVac[1][i][j]=true;
                                    bArm2PressSkipNeedDownCheckVac=true;
                                }
                            }
                            BLCarryKit.SetItemData(i, j, NULL_IC);
                            bArm2DuplicateErr[i][j]=false;
                        }
                    }

                    if(IniConfig.bNewResetFunction==true &&
                       bResetIndexArm2Pick==true)
                    {
                    }
                    else
                    {
                        if(IniConfig.bD42IndexPickICShuttlePause)
                        {
                            bInArmNeedToSafePos=true;
                            bShuttle2Pause=true;
                            bIndexArm2PickupErrStop=true;                                                               //jou 2012-02-29 index pick up error,index arm move to center & alarm
                            bShowShuttle2Device=true;                                                                   //kevin 20180504 index pick up error
                        }
                    }
                    bIndexPickUpErrorWaitRetry=false;                                                                   //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                    if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                     //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                    {
                        bIndexPickErrShtStayRight2=false;
                    }

                    if(IniConfig.bD43IndexPickErrCheckSocket==true)                                                     //Steven 20190115 : SCC要求吸取異常要檢查Socket
                    {
                        bIndexArm2PickUpErrNeedPiggyback=true;
                        if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                                            //Sam 20230108 : 矽格俊堯要求不要 Reset IC
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
                    if(IniConfig.bD43AutoRetryWhenIndexPickErr &&                                                       //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                       iD43AutoRetryWhenIndexPickErrCnt[1]==0)
                    {
                        iD43AutoRetryWhenIndexPickErrCnt[1]++;
                    }
                    else
                    {
                        iD43AutoRetryWhenIndexPickErrCnt[1]=0;                                                          //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                                if(BTestSuck.Suck[i][j].Error)
                                    bArm2DuplicateErr[i][j]=true;
                    }
                    bIndexPickUpErrorWaitRetry=true;                                                                    //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                    if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                     //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                    {
                        bIndexPickErrShtStayRight2=true;
                    }
                    bIndexPickUpErrorWaitRetry=true;                                                                    //Ifor 20171119 : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
                }
                BTestSuck.ResetAll();                                                                                   //Steven 20160323 : 避免未開啟真空
            }
            else if(IniConfig.bD81IndexCheckVacuumOnShuttle && bHasICErr==true)
            {
                if(IndexAlarmInArmAway()==false)                                                                        //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    return false;
                }
                bHasICErr=false;
                bSht2OnlyHasICErr=true;
                str.sprintf("Need to remove device of Shuttle2 %s", ErrPart2);
                ShowMyMessage(str);
                if(IniConfig.bD42IndexPickICShuttlePause)
                {
                    bInArmNeedToSafePos=true;
                    bShuttle2Pause=true;
                    bIndexArm2PickupErrStop=true;                                                                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                    bShowShuttle2Device=true;                                                                           //kevin 20180504 index pick up error
                }
            }

            if(bShuttle2Pause)
                MOT[MInShuttle2].SetSpeed(10);                                                                          //kevin 20180226 (Steven) add pick up error  shuttle down speed
            if(IniConfig.bNewResetFunction==true && bResetIndexArm2Pick==true)
            {
                bResetIndexArm2Pick=false;
            }
            else
            {
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                         //ChungHung 20120717 add Index Drop Error Can Retry and Start
                {
                    MOT[MInShuttle2].fCanMoveM=true;                                                                    //等待Shuttle 移至右邊
                    bShuttle2MoveToRight=true;
                    bShuttle2HasPickErr=true;                                                                           //Steven 20220712 : 避免In arm 偷放料
                }
            }

            if(IniConfig.bD42IndexPickICShuttlePause &&                                                                 //AI(staterecord-analysis) 20260417 (RogerYang) : D42 SKIP後強制走500, 避免300讓Z2再次下降造成四方死鎖
               bShuttle2Pause)
            {
                Task=500;
            }
            else if(BLCarryKit.HasRealIC())
            {
                Task=300;
            }
            else if(CosFunction.bIndexPickErrSkipNeedCheckVac==true &&
                    IniConfig.bD50IndexPickErrSkipNeedCheckVac &&
                    bArm2PressSkipNeedDownCheckVac==true)                                                               //JerryYang 20170610 (wei) JSCC要求index pick up error 需再慢速下降吸一次
            {
                Task=300;
            }
            else
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BLCarryKit.Item[i][j])
                        {
                            if(BLCarryKit.Item[i][j]==HAS_NULL_IC &&
                               BTestSuck.Item[i][j]!=NULL_IC)
                            {
                                BLCarryKit.SetItemData(i, j, NULL_IC);
                            }

                            if(BLCarryKit.Item[i][j]==HAS_NULL_IC)
                            {
                                BTestSuck.Suck[i][j].Normal();
                                W64bT2_MoveSuckData(BTestSuck, BLCarryKit, i, j);                                       // AI(W5-aTester_Rear-Translate) 20260710: golden :1940 BTestSuck.MoveSuckData(BLCarryKit,i,j) -- gap stub (TMyKitSuck::MoveSuckData absent from aHotPlateSubstrate.h; reuses Wave-1's W64bT2_MoveSuckData(dst,src,i,j))
                                bArm2DuplicateErr[i][j]=false;
                            }
                        }
                    }
                }

                if(BLCarryKit.UseSiteHasIC())
                {
                    Task=300;
                    break;
                }
                Task=500;
            }
            break;
        case 500:
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&                   //ChungHung 20120717 add Index Drop Error Can Retry and Start
               bShuttle2MoveToRight)                                            //ChungHung 20131015 fix hangup
            {
                if(InSHT2InRT()!=true)
                {
                    return false;
                }
            }
            CheckShuttle2EncoderPos();                                          //JerryYang 20230131 : add shuttle shift log
            MOT[MInShuttle2].fCanMoveM=false;
            bShuttle2MoveToRight=false;
            bShuttle2HasPickErr=false;                                          //Steven 20220712 : 避免In arm 偷放料
            bIndexPickUpErrorWaitRetry=false;                                   //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
            if(DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==true &&
               LastSet.iTemperature==Tempture_Hot)                              //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
            {
                if(fFrontNeedTest==true && bD52IndexArmUp==false)               //JerryYang 20200812 : fix RTC error hang up
                    break;
                if(FTestSuck.HasRealIC()==false)                                //JerryYang 20220805 : 下壓shuttle預熱功能initial第一次吸shuttle時需下壓等待Jam soak time
                {
                    if(hDoRearTestSuckICdelay.Off()==false)
                        break;
                }
            }
            Task=501;
            iHomeAlarm=0;
            if(IsNNMode()==NN_2Row)
                break;
        case 501:                                                                                                       //ChungHung 20130924 add
            if(MOT[MTestZ2].Gali_ReadPos()>(Prod.TestZ2_Pick+1000))                                                     //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
            {
                bZ2PickShuttle=false;
            }
            sp=GetIndexZSpeed(1);                                                                                       //Steven 20160524 : Index Z軸速度整合為Function
            #ifdef DEBUG_INDEX_UPH
            flag1=MOT[MTestZ2].Gali_MotMove2(Prod.TestZ2_Safe, iIndexSpeed, iIndexAcc);
            #else
            flag1=MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Safe, sp, 0);
            #endif
            if(flag1)
            {
                bSuckingFlagZ2=true;                                                                                    //Steven 20240916 : index下降到shuttle吸放料
                #ifndef SOFT_SIMULTE
                if(TestIF_File.iShuttleMode==0 ||                                                                       //Ifor 20160303 新增馬達回Home保護機制
                   (TestIF_File.iShuttleMode==1 &&
                    TestIF_File.iShuttle_Sel==1))
                {
                    MOT[MTestZ2].ScanMotorStatus();
                    if(MOT[MTestZ2].Led[iHomeLed]==false)
                    {
                        if(iHomeAlarm>10)
                        {
                            ShowMyMessage("Motor Z2 not at the origin!!", "馬達Z2不在原點上!");
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
                bZ2PickShuttle=false;                                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                         //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
                {
                    bIndexPickErrShtStayRight2=false;
                }
                #ifndef SOFT_SIMULTE
                {
                    if(LastSet.iRealDummy==REALLY)                                                                      //Steven 20110131 Start
                    {
                        ErrPart=" ";
                        bHasErr=false;
                        bHasDuplicateErr=false;
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                if(bArm2DuplicateErr[i][j])
                                    bHasDuplicateErr=true;
                                if(BTestSuck.Item[i][j]!=NULL_IC &&
                                   BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                   BTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    bHasErr=true;
                                    BTestSuck.Suck[i][j].Normal();                                                      //Steven 20110707 : 預防負壓掉料
                                    BTestSuck.Suck[i][j].Error=true;
                                    ErrPart+=IndexSuckName[i][j];
                                }
                                else
                                {
                                    BTestSuck.Suck[i][j].Error=false;
                                }
                            }
                        }

                        if(bHasErr)
                        {
                            if(IndexAlarmInArmAway()==false)                                                            //Steven 20130613 : Index異常時, In Arm要先讓位功能
                            {
                                return false;
                            }
                            bHasErr=false;
                            if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                 //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                                bIsTestSitICFallDown=true;                                                              //kevin 20130706
                            if(IniConfig.bIndexDropOnlySKIP==true ||
                               IniConfig.bKoreaFunction)                                                                //jou 2012-02-13 index drop error only skip
                            {
                                ret=ShowErrorMessage("JAM0304", K_SKIP, MTestZ2, bHasDuplicateErr, ErrPart);            //Device Drop Error
                            }
                            else
                            {
                                ret=ShowErrorMessage("JAM0304", K_SKIP|K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);    //Device Drop Error
                            }

                            if(ret==K_SKIP)
                            {
                                bAutoSiteMapWaitTestResult=false;                                                       //Ifor 20180115 (Steven) : add Site Mapping SKIP 需清除旗標
                                for(int i=0; i<BTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<BTestSuck.iShtCol; j++)
                                    {
                                        if(BTestSuck.Suck[i][j].Error)
                                        {
//AI(W906-W2-PRODREC) 20260926: gate LIFTED -- TMyProductionRecord::AddErrorRecord is DECLARED and links, but today it resolves to the no-op stand-in at aHotPlateSubstrate.cpp:773 (body gated G-5, see Public/MyProductionRecord.h BODY STATUS) -- call site now faithful, behaviour unchanged until G-5 opens -- golden :2083
                                            BTestSuck.PordRec[i][j].AddErrorRecord("JAM0304");                          //Steven 20161214 : 加上Index異常Skip的ErrorLog
//AI(W906-W2-PRODREC) 20260926: (end of lifted gate)
                                            if(CosFunction.bUseSCKART)                                                  //Steven 20161214 (wei) : For SCK ART
                                            {
                                                fSCKART->AddOutputJamCnt(i, j, ret,
                                                    Prod.bIsPassBin[BTestSuck.iBinData[i][j]]);                         //RogerYang 20250923 : 整合ART OutArm JamCount
                                            }
                                            BTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                            BTestSuck.Suck[i][j].Error=false;
                                        }
                                        bArm2DuplicateErr[i][j]=false;
                                    }
                                }

                                if(IniConfig.bD42IndexPickICShuttlePause)                                               //ChungHung 20110302 start
                                {
                                    bInArmNeedToSafePos=true;
                                    bShuttle2Pause=true;
                                    bIndexArm2PickupErrStop=true;                                                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                                    bShowShuttle2Device=true;                                                           //kevin 20180504 index pick up error
                                }

                                if(IniConfig.bD43IndexPickErrCheckSocket==true)                                         //Steven 20190115 : SCC要求吸取異常要檢查Socket
                                {
                                    bIndexArm2PickUpErrNeedPiggyback=true;
                                    if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                                //Sam 20230108 : 矽格俊堯要求不要 Reset IC
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
                                for(int i=0; i<BTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<BTestSuck.iShtCol; j++)
                                    {
                                        if(BTestSuck.Suck[i][j].Error)
                                        {
                                            bArm2DuplicateErr[i][j]=true;
                                            W64bT2_MoveSuckData(BLCarryKit, BTestSuck, i, j);                          // AI(W5-aTester_Rear-Translate) 20260710: golden :2127 BLCarryKit.MoveSuckData(BTestSuck,i,j) -- gap stub (see above)
                                            BTestSuck.Suck[i][j].Error=false;
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
                if(bShuttle2Pause)
                    MOT[MInShuttle2].SetSpeed(10);                                                                      //kevin 20180226 (Steven) add pick up error  shuttle down speed
                MOT[MInShuttle2].fCanMoveM=true;
                W64bT2_SetUnuseToNullIC(BTestSuck);                                                                     // AI(W5-aTester_Rear-Translate) 20260710: golden :2142 BTestSuck.SetUnuseToNullIC() -- gap stub, see file-top (Steven 20241017 : 清除沒用到的資料)
                bSuckingFlagZ2=false;
                ResetInxedArm2Flag();                                                                                   //Steven 20241017 : 打包成function
#if 0 // TODO(W5-Automation) -- golden :2145-2158 fSCKART->iInfo_MultiLotCnt / LotSummary.AddByLotLoadCount
                // (AI(W5-aTester_Rear-Translate) 20260710: TfSCKART FormsFacade (FormsFacade.h) lacks iInfo_MultiLotCnt
                //  (golden Automation/SCK_ART.h:323, Automation subsystem not yet home);
                //  LotSummary/TLotSummary (golden cSocket.h) not yet translated at all.
                //  Pure per-2DID lot-load bookkeeping, no state the SM reads back --
                //  gating the whole guarded block is behaviourally identical offline.)
                if(fSCKART->iInfo_MultiLotCnt>1)
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Item[i][j]!=NULL_IC &&
                               BTestSuck.cDeviceInf[i][j]!="")
                            {
                                LotSummary.AddByLotLoadCount(BTestSuck.cDeviceInf[i][j]);
                            }
                        }
                    }
                }
#endif

                if(DeviceForm_File.ContactMode==DirectContactSoftEP ||
                   DeviceForm_File.ContactMode==DropContactSoftEP)                                                      //kevin 20130608 Soft Contact mode
                {
                     W64bT2_APAX_WriteData(true, 0, 2);                                                               // AI(W5-aTester_Rear-Translate) 20260710: golden :2163 APAX_WriteData (adam6024.h APAX_* family; different from the already-shimmed ADAM_* -- gap stub, see file-top)  //ARM2 shuttle 吸完ic 浮動頭不充氣
                }
#if 0 // TODO(W7) -- golden :2165 fAutoTeach SetIndexBinError(1) (TfAutoTeach VCL form -- unlike chkReadTorque/edTorue1, NO FormsFacade/TU-local stand-in exists anywhere in the tree yet; pure UI auto-align marker, no state the SM reads back)  //JimmyChiu 20211020 : Auto alignment mode
                fAutoTeach->SetIndexBinError(1);                                                                        //JimmyChiu 20211020 : Auto alignment mode
#endif
                if(bHPCleanout)                                                                                         //wei 20160624 Hotplate clean out
                {
                    return true;
                }
                else if(LastSet.iTemperature==Tempture_Hot && bInitialSackTime &&                                       //2013-11-27   Dell    需要做Index soak time
                        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false &&                                       //JerryYang 20220805 : 有開下壓shuttle預熱功能就不要做懸空預熱
                        (Temperature.iInitialStart1Time>0 || Temperature.iIndexSoakTime>0))                             //kevin 20131112 第一次吸取ic等待時間)
                {
                    int iSetTmr=Temperature.iInitialStart1Time;
                    if(Temperature.iIndexSoakTime>Temperature.iInitialStart1Time)
                        iSetTmr=Temperature.iIndexSoakTime;

                    DoTestHeadMotorDelay2.SetSecAndOn(iSetTmr);
                    Task=502;
                    bInitialSackTime=false;
                    bInitialStart1Time=true;                                                                            //wei 20171020 (jou) InitialStart1 秒數倒數
                    dwStartInitialCount2.LatchCycleTime(true);
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
                    iInitialStart1Count=(Temperature.iInitialStart1Time)-(dwStartInitialCount2.LatchCycleTime()/1000);
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
//------------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  PART FILE (stitch fragment, wave label k9-small-six) for aTester_Rear.cpp
//  Translator: AI(W7b-k9) 20260810
//  Golden: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp (9,787 lines,
//  cp950, 100% CRLF).  This fragment is UTF-8 + 100% CRLF, matching the port
//  aTester_Rear.cpp (2,448 CRLF / 0 bare LF).  Chinese comments transcribed
//  character for character from cp950; zero U+FFFD.
//
//  ROLE          -- Encoder-vs-command position sanity gate for the REAR index
//                   head: returns true ("out of range") if the command/encoder
//                   mismatch on MTestZ2, MTestY1 or MTestY2 exceeds 10 counts.
//                   Same shape as the already-translated TestZ2OutRandge (golden
//                   :224-255, port aTester_Rear.cpp:248-279) but checks all THREE
//                   axes and returns early after each -- golden text differs only
//                   in the blank lines before the MTestY1/MTestY2 blocks, both
//                   preserved verbatim here.
//  WAVE SCOPE    -- TestZ2OutRandge2()  ACTIVE  golden :257-289
//  GATE REGISTER -- none.  Every symbol is real in the port: MOT[] / MTestZ2 /
//                   MTestY1 / MTestY2 (Motor/mymotor.h), TMotor::Gali_ReadPos and
//                   Gali_ReadEncoderPos (Motor/mymotor.h:214-215, both returning
//                   long -- assigned to golden int Pos1/Pos2 exactly as golden
//                   does, narrowing preserved).  SOFT_SIMULTE is NOT defined in
//                   this build (CMakeLists.txt:920 states so) so the #ifdef body
//                   is reproduced verbatim and inert, matching the convention the
//                   file banner already documents.
//  INTEGRATE     -- retires the offline stub atester_shims.cpp:218
//                   `bool TestZ2OutRandge2() { return false; }`; the non-static
//                   declaration stays at atester_shims.h:142 (that declaration is
//                   exactly why this body is NOT static -- TRAP 1 shape (d)).
//---------------------------------------------------------------------------
bool TestZ2OutRandge2()
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
        int Pos1=MOT[MTestZ2].Gali_ReadPos();
        int Pos2=MOT[MTestZ2].Gali_ReadEncoderPos();
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
//  PART FILE (stitch fragment, wave label k9-small-six) for aTester_Rear.cpp
//  Translator: AI(W7b-k9) 20260810
//  Golden: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp (9,787 lines,
//  cp950, 100% CRLF).  This fragment is UTF-8 + 100% CRLF, matching the port
//  aTester_Rear.cpp (2,448 CRLF / 0 bare LF).  Chinese comments transcribed
//  character for character from cp950; zero U+FFFD.
//
//  ROLE          -- Reset entry for the rear test-head suck-test state machine:
//                   rewinds the DoBTestSuckTestIC cursor to step 1 and clears the
//                   "Z2 is down" latch.
//  WAVE SCOPE    -- InitBTestSuckTestICTask()  ACTIVE  golden :2211-2215
//  GATE REGISTER -- none.
//  OUT OF MY RANGE (hand-off, do NOT let this drift): golden :2210
//                   `int iBTestSuckTestICTask=1;` is NOT in my assigned range
//                   2211..2215, so it is deliberately not defined here.  It lives
//                   today as a real data definition at atester_shims.cpp:189
//                   (declared atester_shims.h:112) and can simply STAY there --
//                   it is plain data, not a stub body, so there is no
//                   duplicate-definition risk with this fragment.  bZ2Isdownflag
//                   is the same global the already-translated
//                   InitRearTestDestroyICTask writes (port aTester_Rear.cpp:294).
//  INTEGRATE     -- retires the no-op stub atester_shims.cpp:205
//                   `void InitBTestSuckTestICTask() {}`; keep the non-static
//                   declaration at atester_shims.h:128 (body here is NOT static).
//---------------------------------------------------------------------------
void InitBTestSuckTestICTask()
{
    iBTestSuckTestICTask=1;
    bZ2Isdownflag=false;
}

// ==========================================================================
//  PART FILE  _w7b_parts/02222_DoBTestSuckTestIC.txt   (label k4-DoBTestSuckTestIC)
//  Stitch target: aTester_Rear.cpp -- append AFTER the existing Wave-1/Wave-2
//  content (this part REUSES Wave-1 symbols already in that file; see ORDERING).
//  Translator: AI(W7Bk4) 20260810.
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp (9,787
//  lines, cp950, 100% CRLF), lines 2222..3878 -- DoBTestSuckTestIC() and nothing
//  else.  Emitted UTF-8 / 100% CRLF, zero U+FFFD.
//
//  ROLE
//  ----
//  DoBTestSuckTestIC() is the REAR (Arm2 / Z2) socket-test state machine: with the
//  rear test head already pressed into the sockets it runs the whole test
//  transaction -- optional [D01] torque read, pre-test site census, tester
//  START/END handshake (MTI/PTI DoStartMode/DoEndMode), result collection
//  (ProcessTestResult), Double-Contact / OverDrive / ReContact re-press loops,
//  tester TIME-OUT handling, [D37] manual-process key waits, ProcessCount/
//  RecordHistroy bookkeeping, the TestSocket<->BTestSuck grid hand-off, Murata XML
//  re-upload retries, and the KYEC F007 tail ([D79] index-pick purge / SECS
//  consecutive-fail / ATC active-cooling wait).  Returns true exactly when the
//  rear test cycle is complete and the caller may lift Z2.
//
//  WAVE SCOPE
//  ----------
//    * DoBTestSuckTestIC()                golden :2222-3878   ACTIVE
//      Every case, branch and loop is ACTIVE.  Ten individual STATEMENTS inside
//      it are #if 0 gated -- see GATE REGISTER (G1..G10).
//
//  ORDERING (why this part must be appended, not prepended)
//  -------------------------------------------------------
//  It REUSES, rather than duplicates (duplication = redefinition in the same TU):
//    * W64bT2_FMAIN_CHKREADTORQUE1 / _CHKREADTORQUE2 / _EDTORUE1  (port
//      aTester_Rear.cpp:1029-1031) -- the SAME golden fMain torque widgets this
//      function derefs at golden :2290-2292 / :2299-2300 / :3491-3494 / :3502 /
//      :3510 / :3526 / :3533 / :3541-3542.
//    * W64bT2_ProcessIndexSuckDestroy2(int=0) (port aTester_Rear.cpp:367) -- the
//      SAME gap at golden :3733 `fiosetview->ProcessIndexSuckDestroy2()`.
//  If the main loop places this part BEFORE Wave-1 content, hoist those four.
//
//  PREREQUISITES EMITTED HERE THAT ARE OUTSIDE MY 2222..3878 RANGE
//  --------------------------------------------------------------
//  Marked [W7Bk4-PREREQ].  golden :2217 DoBTestSuckTestICDelay, golden :2219-2221
//  DoBTestSuckOffDelay (+ its two F007 banner comments, transcribed verbatim) and
//  golden :94 hBTestTimeOutDelay are plain file-scope TQPF_Timer data globals that
//  THIS function is the only consumer of, and none of the three exists anywhere in
//  the port (measured -- see GATE REGISTER absence commands).  Without them the
//  stitched file cannot compile.
//  *** MAIN LOOP: if a sibling part also emits any of the three, delete the
//  duplicate -- grep the marker [W7Bk4-PREREQ]. ***
//  SIBLING-OVERLAP RE-CHECK (TRAP 2, run against _w7b_parts/ itself at
//  2026-08-10 19:50, command:
//    grep -n "^TQPF_Timer\|^static TQPF_Timer" _w7b_parts/*.txt
//    grep -n "DoBTestSuckTestICDelay|DoBTestSuckOffDelay|hBTestTimeOutDelay"
//         _w7b_parts/*.txt | grep -v ^02222     [alternation escaped for grep])
//  -> NO sibling part defines any of my three.  Two live findings from that scan:
//    (a) sibling 07960_DoTestYRear_c3.txt USES DoBTestSuckTestICDelay (its lines
//        1004 and 1010), so my single definition serves that part too -- keep
//        exactly one copy.
//    (b) sibling 09335_BTestSeparateSLK.txt emits golden :95-97 (the three
//        BTestSocketClamp* timers) and its banner explicitly anticipates a
//        sibling emitting "golden :94-98 as one block".  I emit ONLY golden :94
//        (hBTestTimeOutDelay) and NOT :95-97, so there is no overlap with it --
//        but if the integrate step later re-homes golden :94-98 as one block,
//        drop my [W7Bk4-PREREQ] hBTestTimeOutDelay line.
//  NOT emitted, deliberately (outside my range AND already owned elsewhere):
//    * iBTestSuckTestICTask -- real data global atester_shims.cpp:189 (declared
//      atester_shims.h:112).  Referenced, never defined, here.
//    * InitBTestSuckTestICTask() -- golden :2211-2215.  OWNED BY A SIBLING PART:
//      _w7b_parts/02211_InitBTestSuckTestICTask.txt (AI(W7b-k9) 20260810) carries
//      the real body, so at integrate the no-op stub atester_shims.cpp:205 is
//      retired by THAT part, not left wrong.  (CORRECTION: an earlier draft of
//      this banner claimed nobody in the wave owned golden :2210-2221 and that the
//      cursor would therefore never be re-homed -- that absence claim was already
//      stale when written, and the re-check above is what caught it.  Verified
//      2026-08-10 19:50 by reading that sibling part.)
//    * `extern int SendTestResultToHttp();` -- golden :2218; see GATE REGISTER S2.
//
//  TU-LOCAL FAITHFUL EQUIVALENTS (NOT gates, NOT degraded stubs)
//  ------------------------------------------------------------
//  Five symbols golden calls have no port member/function, but their golden
//  bodies touch ONLY things the port already has, so they are transcribed IN FULL
//  as TU-local free functions instead of being gated (exact precedent: this same
//  file's W64bT2_SetUnuseToNullIC, port :985-995):
//    * W7Bk4_CopyFrom               == golden MyKitSuck.cpp:1151-1193
//    * W7Bk4_MoveAllItem            == golden MyKitSuck.cpp:1043-1109
//    * W7Bk4_Tested                 == golden MyKitSuck.cpp:1017-1030
//    * W7Bk4_CheckVaccumIsIniaialON == golden MyKitSuck.cpp:2732-2746
//    * W7Bk4_NowNoteIsShow          == golden note.cpp:4286-4289 (`return
//      fNote->fShow;`) -- the port HAS a real fNote with a real fShow
//      (forms/fNote.h:133 / :169, object forms/fNote.cpp:42, library ht9045_forms)
//
//  TRAP 5 STATEMENT (two headers, same class name)
//  ----------------------------------------------
//  The first four above are TMyKitSuck methods, and TMyKitSuck EXISTS TWICE with
//  DIFFERENT LAYOUTS: mykitsuck.h:274 and aHotPlateSubstrate.h:365.  I relied on
//  **aHotPlateSubstrate.h:365-622** -- the one this file already includes and the
//  one that declares the objects I touch (BTestSuck / TestSocket,
//  aHotPlateSubstrate.h:635; the objects themselves are DEFINED in
//  aHotPlateSubstrate.cpp).  I did NOT include mykitsuck.h anywhere.  Every field
//  the four bodies touch was verified present in THAT class: Suck :369, Item :370,
//  iWhichSite :371, PordRec :372 (its asBuffer/bUse/InitialRecord are real --
//  Public/MyProductionRecord.h:158/:163 + MyProductionRecord.cpp), iMaxRow :377,
//  iMaxCol :402, iShtRow :406, iShtCol :438, iBinData :439, cDeviceInf :440,
//  cSBin :441, iWhichAuto :450, bPass :451, bNeedReTest :452, iAutoCleanRecX/Y
//  :556-557, iWhichIndex :558, iNeedRotAng :559, iCurrRotAng :560, bQATray :561,
//  iCleanCount :562, bFliped :563, iBinDataBackUp :564, cReDeviceInf :565,
//  b2DIDNG :566, iAOIResult :567, SetItemData :497.  The same header's own
//  MoveSuckData/MoveSuckDataDiff (:572/:598) already move that identical field
//  set faithfully, which is the independent proof the set is complete.
//
//  GATE REGISTER
//  -------------
//  Every #if 0 this part writes (G1..G10), plus the three value-returning
//  stand-ins (S1..S3) and the one widget seam (S4) -- those four are listed here
//  too because they rest on the same kind of claim and must expire the same way.
//  Every absence command below was run with CWD = D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//  on 2026-08-10 (per-entry time stamp).  TRAP 2: RE-RUN EVERY ONE AT INTEGRATE --
//  a sibling part of THIS wave can land any of these minutes after I measured, and
//  "the degraded value happens to be equivalent" would not make a dead premise OK.
//
//  G1 (golden :2663) and G8 (golden :3130)   SetHangupMaxTime()
//    WHY THE OFFLINE DEFAULT IS FAITHFUL: it is a pure "re-arm the hang-up
//    watchdog" helper.  Skipping it leaves HangTime running on the value the same
//    path just set (golden :2436 HangTime.SetSecAndOn(Prod.iHangupMaxTime), and
//    :2662 bInitialMaxTime=false stays ACTIVE), so no watchdog is left unarmed.
//    REAL-MACHINE DIFFERENCE: the deadline is not recomputed from the CURRENT
//    recipe here, so a mid-lot recipe change is not picked up until the next
//    SetSecAndOn -- a stale hang-up deadline, never a missing one.
//    WHY IT SHOULD BE GATED (TRAP 3 re-ask): not "unknown symbol" -- it IS
//    declared (cinitial.h:145) but has ZERO definitions tree-wide, so calling it
//    is an undefined-reference LINK error.  cinitial.cpp gates its own call for
//    exactly this reason (cinitial.cpp:8551-8552, "GATE n2-4 ... 0 definitions
//    tree-wide").
//    ABSENCE COMMAND: grep -rn "^void SetHangupMaxTime" --include=*.cpp .
//    MEASURED 2026-08-10 21:22 -> 0 hits (only the cinitial.h:145 declaration and
//    atester_32Site.cpp's own TU-local W5_32S_SetHangupMaxTime no-op).
//
//  G2 (golden :2671)   fLotInfo->SetATCOffset(true)
//    WHY FAITHFUL: pushes the ATC initial temperature offset once, on the
//    iInitContactCount == Temperature.iCintactCntForTempOffsetAtInitial edge.
//    There is no ATC controller offline (TATC_InterfaceFormShim ships
//    iATC_MODE_TYPE == 0), so "no offset pushed" IS the offline hardware state.
//    golden :2672 iInitContactCount++ stays ACTIVE, so the edge still passes
//    exactly once and the SM cannot re-enter.
//    REAL-MACHINE DIFFERENCE: the ATC never receives its initial-contact
//    temperature offset, so the first sites test at un-offset temperature.
//    WHY GATED: TfLotInfo (forms/fLotInfo.h) has no SetATCOffset member at all,
//    and atester.cpp's identical call (atester.cpp:2250) is ALREADY inside an
//    #if 0 there (verified by a preprocessor-nesting scan, not by eye).
//    ABSENCE COMMAND: grep -rn "SetATCOffset" --include=*.h --include=*.cpp .
//    MEASURED 2026-08-10 21:18 -> 1 hit: atester.cpp:2250, itself gated.
//
//  G3 (golden :2707) and G7 (golden :3125)
//                      fSocketCommunication->bSendBinMapReport=true
//    WHY FAITHFUL: the ASE_KR socket-tester bin-map report trigger, reached only
//    when IniConfig.bEnableSocketCommunication is on.  No socket-tester form
//    exists offline, so "no report queued" is the offline state, and nothing in
//    this function reads the flag back.
//    REAL-MACHINE DIFFERENCE: an ASE_KR machine would not send its per-cycle
//    bin-map report to the socket tester.
//    WHY GATED: no TfSocketCommunication class and no fSocketCommunication object
//    exist anywhere in the ported tree.
//    ABSENCE COMMAND: grep -rn "fSocketCommunication" --include=*.h --include=*.cpp .
//    MEASURED 2026-08-10 21:16 -> 2 hits, both non-code (a csystem.cpp comment and
//    a generated tools/dfm2rc layout string); 0 declarations.
//
//  G4 (golden :2906)
//        ATC_InterfaceForm->SiteTesting(iATC_Use_Heat_Count, bATCSiteTest)
//    WHY FAITHFUL: opens ATC per-site 2nd-point temperature monitoring for the
//    sites under test.  With no ATC offline there is nothing to monitor.  Golden's
//    loop that fills bATCSiteTest[] (:2904-2905) is left ACTIVE, so the array is
//    still built exactly as golden builds it, and :2901 bATC_SITE_2ND_CHECK[1]=
//    false also stays ACTIVE, so no consumer sees a stale "checked" state.
//    REAL-MACHINE DIFFERENCE: ATC 2nd-point monitoring is not armed for this
//    contact, so a single-site over/under-temperature would not raise the ATC
//    site alarm.
//    WHY GATED: TATC_InterfaceFormShim (acarry_shims.h:110-116) exposes exactly
//    one member, iATC_MODE_TYPE.  atester.cpp's three identical calls (:1478,
//    :1957, :8787) are ALREADY inside #if 0 there.
//    ABSENCE COMMAND: sed -n '/class TATC_InterfaceFormShim/,/^};/p' acarry_shims.h
//    MEASURED 2026-08-10 21:26 -> body is { int iATC_MODE_TYPE; ctor; } only.
//
//  G5 (golden :2910)   ATCInterfaceForm->SendTestEnd(1)
//    *** THIS IS THE TRAP-3 SHAPE: A REAL PORT BODY EXISTS AND I STILL GATED IT,
//    FOR A DIFFERENT REASON THAN "THE SYMBOL IS ABSENT". ***
//    STATUS (not an absence claim): TATCInterfaceForm::SendTestEnd(int) is
//    DECLARED ATC/ATCInterface.h:468; the object is real (ATC/ATCInterface.cpp:207
//    `TATCInterfaceForm *ATCInterfaceForm = new TATCInterfaceForm(NULL);`); and
//    ATC/ATCInterface.cpp IS compiled (CMakeLists.txt:2119, into ht9045_sm).
//    WHY IT SHOULD STILL BE GATED HERE: reaching it costs this file a NEW include
//    of ATC/ATCInterface.h, which drags in ATC/ATCSystem.h + vclcompat/
//    ClientSocket.h + vclcompat/ServerSocket.h + vclcompat/LedCore.h
//    (ATCInterface.h:154-161).  LedCore.h re-declares `typedef int TColor` plus a
//    9-constant cl* palette that cmydef.h:16 -- already included by this file --
//    also declares, i.e. a namespace-scope const redefinition hazard; and
//    aTester_Rear.cpp is a translation unit that sibling agents are writing into
//    during this same wave.  Taking a whole socket/LED header chain for ONE
//    fire-and-forget notification, inside a concurrently edited TU, is the same
//    trade the MyMessageBox gate already refused.  RECOMMENDATION: retire G5 at
//    the integrate step by adding that include ONCE after the stitch compiles --
//    not from inside a part file.
//    REAL-MACHINE DIFFERENCE: on an ATC_SYSTEM==eATCHonPrecType machine the
//    HonPrec ATC is not told "test end" for arm 1, so its soak/cool state machine
//    does not advance on this event.  ATC_SYSTEM is eATCUninstall on the offline
//    build, so the branch is not entered there today.
//    COMMANDS PROVING THE ABOVE: grep -n "ATC/ATCInterface.cpp" CMakeLists.txt ;
//      grep -n "SendTestEnd" ATC/ATCInterface.h ; grep -n "#include" ATC/ATCInterface.h
//    MEASURED 2026-08-10 21:34.
//
//  G6 (golden :2958) and G9 (golden :3237)
//                      ATC_InterfaceForm->SendHandler2DID(0, false)
//    WHY FAITHFUL: hands the just-tested 2D-ID string to the ATC side channel.  No
//    ATC offline means nothing to hand over, and no state in this function is read
//    back from it.
//    REAL-MACHINE DIFFERENCE: ATC-side 2DID traceability loses this cycle's entry.
//    WHY GATED: same one-member TATC_InterfaceFormShim as G4.  atester.cpp's
//    identical call (:3992) is ALREADY inside #if 0; atester_32Site.cpp routes it
//    through the no-op W5_32S_ATC_SENDHANDLER2DID.
//    ABSENCE COMMAND: grep -rn "SendHandler2DID" --include=*.h .
//    MEASURED 2026-08-10 21:26 -> 0 hits in any header.
//
//  G10 (golden :3534)   COM2->InitReadTorueTask()
//    WHY FAITHFUL: re-arms the serial torque-reader task before a second read.
//    There is no torque MCU offline, and the two golden lines bracketing it stay
//    ACTIVE (:3533 clears edTorue1->Text, :3535 sets bReadMCU2=true), so the SM
//    still takes its "no torque value came back" path (case 5300 -> 5350 -> 5000)
//    exactly as on a machine whose torque reader never answers.
//    REAL-MACHINE DIFFERENCE: the second torque read is never re-armed, so a
//    machine with [D01] on always takes the retry/abandon path instead of reading
//    a torque value on attempt 2.
//    WHY GATED: the shared TCOM2Shim (atester_shims.h:461) has no
//    InitReadTorueTask member; the only two occurrences in the tree are the
//    TU-LOCAL seam structs of atester.cpp (:5607) and atester_32Site.cpp, and
//    reaching across a TU to borrow one is not available.
//    ABSENCE COMMAND: grep -rn "InitReadTorueTask" --include=*.h .
//    MEASURED 2026-08-10 21:20 -> 0 hits in any header.
//
//  S1 (golden :3113 and :3136)   NowNoteIsShow()   -- stand-in, but FAITHFUL
//    Golden note.cpp:4286-4289 is literally `return fNote->fShow;`, and the port
//    has a real fNote with a real fShow, so W7Bk4_NowNoteIsShow carries the golden
//    body verbatim.  No offline default is invented and no behaviour is degraded.
//    ABSENCE COMMAND (for the FREE FUNCTION NAME only):
//      grep -rn "NowNoteIsShow" --include=*.h --include=*.cpp .
//    MEASURED 2026-08-10 21:12 -> 0 hits tree-wide (only the form half, fNote,
//    has been ported; the free function has no home).
//    TRAP 4 NOTE: golden derefs fNote unconditionally and so do I -- no `if(fNote)`
//    guard was added, because golden :3168 in THIS function (`if(fNote->fShow)`)
//    already derefs it unconditionally and ckernel.cpp:1749 does the same.  Safe
//    because the deref happens only from the running SM, never during static init,
//    and this part adds no file-scope object whose constructor runs code.
//
//  S2 (golden :3673)   SendTestResultToHttp()  -> offline 1 (== upload OK)
//    WHY FAITHFUL: 1 is golden's own "no retry needed" value and is exactly what
//    the Murata XML path sees when there is no HTTP endpoint, which IS the offline
//    condition.  Returning != 1 would instead drive the SM into the WAR16321
//    "3 upload failures" alarm, i.e. fabricate a failure.
//    REAL-MACHINE DIFFERENCE: results are never actually POSTed.
//    ABSENCE COMMAND: grep -rn "SendTestResultToHttp" --include=*.h --include=*.cpp .
//    MEASURED 2026-08-10 21:10 -> 1 hit: atester_32Site.cpp:394's own TU-local
//    `static int W5_32S_SendTestResultToHttp(){ return 1; }` (same value).  No
//    declaration and no body anywhere; golden itself only forward-declares it
//    locally at golden :2218, which is OUTSIDE my range.
//    TRAP 1(d) NOTE: my stand-in is `static` but carries a DIFFERENT NAME
//    (W7Bk4_SendTestResultToHttp), so it cannot be a static shadow -- there is no
//    non-static declaration of that name anywhere to shadow.
//
//  S3 (golden :2894)   RecordEndTestTime(1)  -> offline 1
//    WHY FAITHFUL: same shape as S2 -- ret2 == 1 means "cycle-time record written,
//    no XML retry"; anything else fabricates a retry storm.
//    REAL-MACHINE DIFFERENCE: per-cycle index cycle-time telemetry is not recorded.
//    ABSENCE COMMAND: grep -rn "RecordEndTestTime" --include=*.h --include=*.cpp .
//    MEASURED 2026-08-10 21:10 -> 1 hit: atester_32Site.cpp:393 TU-local stand-in
//    (same value 1).  No declaration, no body.
//
//  S4 (golden :2726, :2747, :2761, :2782, :2785, :3384, :3447, :3449)
//        fMain->BtnZUpDown      and   (golden :3495) fMain->lbArm1Torque
//    WHY FAITHFUL: TfMain (forms/fMain.h) HAS BtnSTEP :696 and BtnT_Start :697 --
//    both used ACTIVE here, including ->Enabled (TControl :216) and ->Color -- but
//    has neither BtnZUpDown nor lbArm1Torque.  The stand-ins default Down=false /
//    Enabled=false / Caption="", and Down==false is the real state of a
//    never-pressed toggle, so every golden `if(...->Down)` takes its else path,
//    which is the ordinary non-TSMC-manual path.
//    REAL-MACHINE DIFFERENCE: on a TSMC machine with
//    CosFunction.bEnableSoftWareControlButton the operator's "Z up/down" latch can
//    no longer divert case 3000/3100, and the "2:Reading" torque caption is not
//    displayed.  Pure UI; no SM state is read back from either widget.
//    ABSENCE COMMAND: grep -n "BtnZUpDown\|lbArm1Torque" forms/fMain.h FormsFacade.h
//    MEASURED 2026-08-10 21:30 -> 0 hits.
//    PRECEDENT: identical shape to this file's own W64bT2_TfMainTorqueSeam (port
//    :1021-1031), naming vclcompat/Controls.h's unified stand-ins (TSpeedButton
//    :421-428 has Down; TPanel :240-246 has Caption), exactly as the
//    AI(W906-W7-F2) 20260729 plan-D4 note in that block requires.
//
//  EXPIRED GATES FOUND WHILE MEASURING (other files -- flagged, not touched)
//  ------------------------------------------------------------------------
//    * TMyKitSuck::MoveSuckData IS REAL NOW (aHotPlateSubstrate.h:598, faithful
//      body aHotPlateSubstrate.cpp:1406) -- so this file's own Wave-1
//      W64bT2_MoveSuckData no-op (port :325-328), aTester_Front.cpp's
//      W64B_MoveSuckData and atester_32Site.cpp's W5_32S_MOVESUCKDATA are all
//      EXPIRED premises now.
//    * TMyProductionRecord::AddTestResultRecord IS REAL NOW
//      (Public/MyProductionRecord.h:82, body MyProductionRecord.cpp:411, compiled
//      at CMakeLists.txt:1472) -- I therefore call it ACTIVE at golden :2379,
//      :2568 and :2600, while atester_32Site.cpp still routes it through the no-op
//      W5_32S_ADD_TEST_RESULT_RECORD.  EXPIRED there.
//    * ShowMainScreenPresure(int) IS REAL NOW (declared cinitial.h:250, ACTIVE
//      body cinitial.cpp:18785, not inside any #if 0) -- I call it ACTIVE at
//      golden :3509, :3519 and :3553, while atester.cpp:5498-5499 still #defines
//      it to a TU-local no-op.  EXPIRED there.
//    * fNote->fShow IS REAL NOW (forms/fNote.h:133) -- atester_32Site.cpp's
//      W5_32S_FNOTE_FSHOW (false) is an EXPIRED premise.
//    * fAutomation->TestMode IS REAL NOW (atester_shims.h:322, a member of
//      TfAutomationShim) -- I use it ACTIVE at golden :2411 and :2700, while
//      atester_32Site.cpp still hard-codes W5_32S_FAUTOMATION_TESTMODE (0).
//
//  GOLDEN DEFECTS PRESERVED (NOT fixed -- behaviour kept as golden has it)
//  ----------------------------------------------------------------------
//   1. golden :2348  `ErrPart+=BTestSuck.Item[i][j]+", ";`  -- Item is `int`, so
//      this is POINTER ARITHMETIC on the 3-byte string literal ", " (const char*
//      + int), NOT "append the number".  With Item == TEST_PASS+n (a large int) it
//      reads far past the literal: undefined behaviour / garbage inside the
//      already-tested diagnostic string.  Transcribed verbatim -- the identical
//      expression compiles to the identical pointer arithmetic in standard C++, so
//      being faithful here means keeping the bug.  Reached only on the
//      bAlreadyTested abort path (golden :2342-2359).
//   2. golden :3070  `Task=3200;` ([P65] ARM QA Mode ReTest, Arm2) jumps to a case
//      label THAT DOES NOT EXIST in this switch -- grepped the entire 2222..3878
//      range: there is no `case 3200:`.  Once taken, every later call falls off the
//      switch to `return false` forever: a permanent stall of the rear test SM.
//      Reachable only with IniConfig.bP65EnableArmQAMode &&
//      IniConfig.iP65ArmQAModeValue>0 && all-bin-1 && BTestSuck.HasRealIC().
//      Reproduced exactly; NOT "fixed" to 2400 or 3000.
//   3. golden :2232 `int &Task=iBTestSuckTestICTask, ret, iMaxDoubleContact, ret2;`
//      and :2237 `bool bAlreadyTested;` -- uninitialised locals (golden relies on
//      every reaching path assigning first).  Left uninitialised, verbatim.
//   4. golden :3322 -- the closing brace of case 2410's else block is indented to
//      column 17 instead of 13.  Cosmetic golden quirk, transcribed as-is.
//
//  TRANSLATION RULES (same as the rest of this file)
//  ------------------------------------------------
//    * SOFT_SIMULTE and TEST_BIN_MISS_SIMULATE are NOT defined in this tree; the
//      #ifdef blocks at golden :2279-2281, :2938-2947, :3386-3397, :3406-3425 and
//      :3858-3860 are reproduced VERBATIM (both branches), letting the same
//      MachineType.h decide, exactly as golden does.
//    * Statement order, every formula, every magic number and every switch
//      fall-through (case 1 -> 100, 100 -> 200, 200 -> 2200, 2200 -> 2400)
//      preserved.
//    * INTEGER DIVISION UNTOUCHED: golden :3290
//      `((dwEndAfterTestCount-dwStartAfterTestCount)/1000)`, :3437
//      `MOT[MTestZ2].GailSpeed/2`, :3617 and :3632 `*Prod.TestZ_Drop_Speed/100`,
//      :2932 `TestSocket.iShtCol/2` all stay integer.  No float helper anywhere.
//    * Big5 comments decoded via cp950 and preserved character-for-character as
//      UTF-8.  ZERO U+FFFD (asserted by the generator that emitted this file).
// ==========================================================================

// ---- [W7Bk4-INCLUDE] three headers this part adds.  MAIN LOOP: hoist them into
//      aTester_Rear.cpp's include block if you prefer -- all three are
//      include-guarded and order-independent here.
//        myswitch.h                -- SW[] / TMySwitch::Off()   (golden :2710, :3238)
//        SECSGEM/SecsEventType.h   -- SECS_EVENT (ETypeStruct)  (golden :3856)
//        SECSGEM/SecsEventReport.h -- void EventReport(unsigned) (golden :3856)
//      All three are compiled: myswitch.cpp CMakeLists.txt:1007,
//      SECSGEM/SecsEventType.cpp :1125, SECSGEM/SecsEventReport.cpp :1126.
#include "myswitch.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

// ---- [W7Bk4-EXTERN] local forward declarations, mirroring golden's own
//      local-extern idiom (golden :2218) and this file's existing precedent
//      (port aTester_Rear.cpp:1110 `extern bool CheckCFixTrayFullPlace();`).
//      Every one names a REAL, linkable port symbol -- none of these is a gate.
extern bool bEcho;                                                              // golden main.cpp; port definition atester_shims.cpp:101
extern bool bEchoStop;                                                          // port definition atester_shims.cpp:102   //ChungHung 20130326 add
bool RespondASECom(AnsiString S1);                                              // golden cpublic.h:41 (port body canary_support.cpp:163)
bool WaitManualStartKey();                                                      // golden ckernel.cpp:96-131 -> port ckernel.h:96, ACTIVE body ckernel.cpp:301
bool WaitManualStepKey();                                                       // golden ckernel.cpp:54-94  -> port ckernel.h:97, ACTIVE body ckernel.cpp:226
void ShowMainScreenPresure(int index);                                         // golden cinitial.h:39 -> port cinitial.h:250, ACTIVE body cinitial.cpp:18785
extern const unsigned int MSG_CMD_OverDrive;                                    // port MessageDef.h:120 / MessageDef.cpp (=35)
extern const unsigned int MSG_CMD_ReContact;                                    // port MessageDef.h:121 / MessageDef.cpp (=36)

// ---- [W7Bk4-PREREQ] golden file-scope peers that live OUTSIDE 2222..3878 but of
//      which DoBTestSuckTestIC is the sole consumer.  Delete any one of the three
//      if a sibling part also emits it.  Transcribed verbatim from golden.
TQPF_Timer DoBTestSuckTestICDelay;                                     // golden :2217
//==> Eastsun 20260511 F007 整合: T8 case 8011/8012 用 SuckOff delay timer
TQPF_Timer DoBTestSuckOffDelay;                                                     //Ifor 20200622 add:Rear 版 Index Pick Shuttle Err Need Purge
//<== Eastsun 20260511 F007 整合
TQPF_Timer hBTestTimeOutDelay;                                                  //Jou 20101018   // golden :94 -- read at golden :3135 / :3165

// ==========================================================================
//  W7Bk4 SEAM BLOCK -- five FAITHFUL free-function equivalents (golden bodies
//  transcribed in full), three offline stand-ins (S1..S3) and one widget seam
//  (S4).  All are `static` under names no other declaration in the tree uses, so
//  none can become a TRAP-1(d) static shadow of a non-static declaration.
// ==========================================================================

// -- golden MyKitSuck.cpp:1151-1193  TMyKitSuck::CopyFrom(TMyKitSuck &Source)
//    FAITHFUL: golden body verbatim, implicit `this->` spelled `dst.`.  Not
//    declared by aHotPlateSubstrate.h:365-622; every field it touches IS.
static void W7Bk4_CopyFrom(TMyKitSuck &dst, TMyKitSuck &Source)
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
static void W7Bk4_MoveAllItem(TMyKitSuck &dst, TMyKitSuck &Source)
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
static bool W7Bk4_Tested(TMyKitSuck &kit)
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
//    only Suck[][].GetStatus()/Normal() and Item[][], all real members.
static void W7Bk4_CheckVaccumIsIniaialON(TMyKitSuck &kit, int iRow, int iCol, bool &flag)
{
    if(kit.Suck[iRow][iCol].GetStatus())
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
static bool W7Bk4_NowNoteIsShow()                                              //ChungHung 20130110 add
{
    return W906_FormShowing("fNote", fNote->fShow);  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
}

// -- S3 and S2: offline "OK" stand-ins; see GATE REGISTER S3 / S2 for why 1 is
//    the faithful value and what a real machine loses.
static int  W7Bk4_RecordEndTestTime(int iWhich) { extern int RecordEndTestTime(int); return RecordEndTestTime(iWhich); }   // golden (Sam 20201231); AI(W906-ELA-W48B) 20260928 (St02-E, laptop-approved claim): the real stamp, cObserver.cpp:3111
static int  W7Bk4_SendTestResultToHttp()            { return 1; }               // golden :2218 forward-decl only

// -- S4: the two fMain widgets TfMain does not have.  Golden derefs them as
//    POINTERS (fMain->BtnZUpDown->Down, fMain->lbArm1Torque->Caption), so the
//    macros yield &object and golden's `->` pattern survives untouched.
static TSpeedButton W7Bk4_BtnZUpDown;                                          // golden main.h (TBtnPanel*) -- ->Down / ->Enabled only
static TPanel       W7Bk4_lbArm1Torque;                                        // golden main.h (TPanel*)    -- ->Caption only
#define W7Bk4_FMAIN_BTNZUPDOWN     (&W7Bk4_BtnZUpDown)
#define W7Bk4_FMAIN_LBARM1TORQUE   (&W7Bk4_lbArm1Torque)

//---------------------------------------------------------------------------
// ==== GOLDEN aTester_Rear.cpp :2222-3878 BEGINS HERE -- verbatim except the
//      call-site redirects and the ten #if 0 gates documented above ====
bool DoBTestSuckTestIC()
{
    static int iTestCount=0, iReadTIntervel=0, iReadCount=0, iXMLRetryCnt=0;
    static int iDoubleCount=0;                                                  //ChungHung 20140709 add for SPIL
    static int iArm2ReadtorquCount=0;                                           //kevin 20210902 add 扭力讀取
    static bool ShowTestStatus=false;                                           //ChungHung 20150526 add for QualComm US
    static bool bFirstIn=true;                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將out shuttle lose IC對應的site設成error bin
    static DWORD dwNowTickCount, dwOldTickCount=0;
    static DWORD dwStartAfterTestCount=0;                                       //ChungHung 20140730 add for ATK function after tested delay time

    int &Task=iBTestSuckTestICTask, ret, iMaxDoubleContact, ret2;
    int iMaxPreasure=0, iErrCnt=0, iTestCh=0;
    bool bHasFailIC=false;
    bool bManualStep=false, bManualTStart=false;                                //ChungHung 20150526 add for QualComm US
    bool bATCSiteTest[32];
    bool bAlreadyTested;
    DWORD dwEndAfterTestCount=0;                                                //ChungHung 20140730 add for ATK function after tested delay time
    AnsiString ErrPart="", ErrCnt="";                                           //kevin 20130418
    AnsiString SData="@e02019Arm2,sideA";                                       //kevin 20191029 add loadcell reaad
    int iDbContactHigh=1000;                                                    //RogerYang 20260126 : JSCC_OS 第二次contact要拉高慢放
    int iDbContactHighSlowSpd;
    //==> Eastsun 20260511 F007 整合: T8 case 8000~9101 所需區域變數
    bool bSuck_OK=false;                                                            //Ifor 20200622 add:T8 case 8001 真空判定結果
    bool flag2=false;                                                               //T8 case 8003 內 vac initial 旗標
    int iIndexUpPos=0;                                                              //T8 case 8002 Above Socket Z 高度
    static AnsiString AllErrPart="";                                                //T8 case 8003/8010 累計錯位字串
    //<== Eastsun 20260511 F007 整合

    switch(Task)
    {
        case 1:
            iXMLRetryCnt=0;                                                     //Steven 20201102 : For Murata資料上拋
            iArm2ReadtorquCount=0;                                              //kevin 20210902 add 扭力讀取

            if(CUSTOMER_CODE==CC_KYEC_XILINX &&
               IniConfig.bD01EnableReadTorque)                                  //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
            {
                bReadRearTestArmTorque=true;
            }
            else
            {
                if(IniConfig.bD01EnableReadTorque &&
                   IniConfig.iD01ReadTorqueTimeCount>0)                         //Steven 20100617 Start: Add form 9080A for 即時更新扭力值
                {
                    iReadTIntervel++;
                    if(iReadTIntervel>=IniConfig.iD01ReadTorqueTimeCount)       //kevin 20130611   if(iReadTIntervel>LastSet.iReadTorqueTimeCount)
                    {
                        iReadTIntervel=0;
                        bReadRearTestArmTorque=true;
                    }
                }
            }

            if(CosFunction.bSocketSensorCheckICAtArmDown==true)
            {
                CheckICExistInSocket(__FUNC__);
            }
            #ifdef SOFT_SIMULTE
            bReadRearTestArmTorque=false;
            #endif
            if(IniConfig.bD02OffReadTorqueDuringTest==true)
                bReadRearTestArmTorque=false;

            if(CosFunction.bHiSiliconFunction==true ||                          //Ifor 20190912 :add 海思 V02.30 版 Record Torque
               CUSTOMER_CODE==CC_KYEC_LEE)
            {
                bRearTestArmTorqueFinish=false;
                bFrontTestArmTorqueFinish=true;
                W64bT2_FMAIN_CHKREADTORQUE1->Checked=false;
                W64bT2_FMAIN_CHKREADTORQUE2->Checked=true;
                W64bT2_FMAIN_EDTORUE1->Text="";
            }
            else
            {
                if(bReadRearTestArmTorque)
                {
                    bReadRearTestArmTorque=false;
                    W64bT2_FMAIN_CHKREADTORQUE1->Checked=false;
                    W64bT2_FMAIN_CHKREADTORQUE2->Checked=true;
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
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]!=NULL_IC     &&
                       BTestSuck.Item[i][j]!=HAS_NULL_IC)
                    {
                        if(BTestSuck.Item[i][j]>=TEST_PASS)
                            bAlreadyTested=true;

                        if((CosFunction.bBarcodeErrNoTestAndShowH==true ||                                              //jou 20191007 : Barcode Error No Test & Show "H"
                            TestIF_File.iNoCodeDeviceToErr==2) &&                                                       //Steven 20200909 : 將2DID all site fail變成選項
                           TestIF_File.bEnableBarCode==true &&
                           (BTestSuck.cDeviceInf[i][j]==asBarCodeErrorSend ||
                            BTestSuck.cDeviceInf[i][j]==""))
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
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        ErrPart+=BTestSuck.Item[i][j]+", ";
                        if(BTestSuck.Item[i][j]!=NULL_IC     &&
                           BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                           BTestSuck.Item[i][j]<TEST_PASS)
                        {
                            BTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                        }
                    }
                }
                ShowMyMessage("IC in index already tested, abort process.", ErrPart);
                return true;
            }

            if(iTestCh==0)                                                                                              //Steven 20250318 : 如果都沒要測試就return true;
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]!=NULL_IC &&
                           BTestSuck.Item[i][j]<TEST_PASS &&
                           BTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            if((CosFunction.bBarcodeErrNoTestAndShowH==true ||                                          //jou 20191007 : Barcode Error No Test & Show "H"
                                TestIF_File.iNoCodeDeviceToErr==2) &&                                                   //Steven 20200909 : 將2DID all site fail變成選項
                               TestIF_File.bEnableBarCode==true &&
                               (BTestSuck.cDeviceInf[i][j]==asBarCodeErrorSend ||
                                BTestSuck.cDeviceInf[i][j]==""))
                            {
                                BTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                BTestSuck.iBinData[i][j]=iTestBinCount;
                                BTestSuck.PordRec[i][j].AddTestResultRecord(iTestBinCount, BTestSuck.cSBin[i][j], "NonTestToRBin");
                            }
                        }
                    }
                }
                return true;
            }

            Task=200;
        case 200:
            W7Bk4_CopyFrom(TestSocket, BTestSuck);
            bDoubleContact=false;
            InitTestTask();
            ShowTestStatus=false;                                               //ChungHung 20150526 add for QualComm US
            iTestCount=0;
            ZeroMemory(iReContactCnt, sizeof(iReContactCnt));                   //Steven 20231205 : 計算某site contact 次數

            if(CUSTOMER_CODE==CC_MTI ||
               CUSTOMER_CODE==CC_PTI)                                           //Sam 20190429 : Add CC_PTI_NEWWORK
            {
                InitDoStartMode();                                              //ChungHung 聚成 Init Test Start
            }

            Task=2200;
//            break;                                                            //Steven 20150724 : Add index speed
        case 2200:
            if(CUSTOMER_CODE==CC_MTI ||                                         //ChungHung 聚成 Test Start
               CUSTOMER_CODE==CC_PTI)                                           //Sam 20190429 : Add CC_PTI_NEWWORK
            {
                if(TestIF_File.iShuttleMode==0 ||                               //Sam 20200214 : 修正關 Arm 不需要送通訊
                   (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1))
                {
                    if(DoStartMode(fAutomation->TestMode)==false)
                        return false;
                }
            }

            if(IniConfig.bD37EnableManualProcess)                               //ChungHung 20150526 add for QualComm US
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

            ProcessStartTestData(1);                                            //畫面顯示黃色，測試中
            SetTestTimeOutTimer(1);                                             //Steven 20200407 : 整合Time Out時間設定
            HangTime.SetSecAndOn(Prod.iHangupMaxTime);

            if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)                  //Sam 20190429 : Add CC_PTI_NEWWORK
            {
                InitDoEndMode();                                                //ChungHung 聚成 only
            }

//            ShowIndexTime(1);                                                 //Steven 20140619 : 測試 //到這裡大概0.00~0.02Sec
            if(IniConfig.bL10IndexTestlogTemp &&
               (TestIF_File.iShuttleMode==0 ||
                (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)))  //kevin 20190323 : index 測試時才記錄溫度
            {
                TemperatureStorageLog(1);                                       //kevin 20190323 add Steven 20140617 : for 海思
            }

            if(IniConfig.bD67LoadCellMeasure)
            {
                SData="@e02019Arm2,sideA";
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
                SetTestTimeOutTimer(1);                                                                                                                         //Steven 20200407 : 整合Time Out時間設定
                HangTime.SetSecAndOn(Prod.iHangupMaxTime);
            }
            else
            {
                dwNowTickCount=MyTickCount();
                if(dwNowTickCount<dwOldTickCount)
                {
                    dwOldTickCount=dwNowTickCount;
                    SetTestTimeOutTimer(1);                                                                                                                     //Steven 20200407 : 整合Time Out時間設定
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);                                                                                                  //Jou 20101018 Start : 重新設定Time Out時間時,Hang Up時間也要重設
                }
                else
                {
                    if(bHandlerPause ||                                                                                                                         //jou 20170526 (wei) 修正長時間alarm未處理,測試當下會發生tester time out
                       bTesterSendPause)                                                                                                                        //kevin 20161101 check problem time out reset
                    {
                        if(CosFunction.bStopMustTestTimeOut==false ||
                           bEnterTestIF==true ||
                           bTesterSendPause)                                                                                                                    //ChungHung 20121221 add
                        {
                            bEnterTestIF=false;
                            SetTestTimeOutTimer(1);                                                                                                             //Steven 20200407 : 整合Time Out時間設定
                        }

                        HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    }
                    dwOldTickCount=dwNowTickCount;
                }
            }

            if(bDoOverDrive || bDoReContact)                                                                                                                    //Steven 20151207 : OverDrive for TSMC  //Steven 20151207 : Recontact for TSMC
            {
                iDoubleCount=0;                                                                                                                                 //jou 20230823 : 修正GPIB ReContact 命令hang up異常
                Task=6000;
                return false;
            }

            if(IniConfig.bD23EveryDeviceDoubleContactFirstNoTesting)                                                                                            //ChungHung 20140709 add for SPIL  //ChungHung 20140709 add for Spil 第一次Contact 不測試 第二次才測
            {
                iDoubleCount++;
                if(iDoubleCount<IniConfig.iD23_MultiContactCount)                                                                                               //Steven 20151001 : Add for TSMC
                {
                    Task=3000;
                    return false;
                }
            }

            if((TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true ||
                TestIF_File.bIndexDropICSetErrUntilOneCycle) &&                                                                                                 //JerryYang 20220923 : add index arm drop error設ERROR BIN
               bFirstIn==true)                                                                                                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將out shuttle lose IC對應的site設成error bin
            {
                bFirstIn=false;
                if(bOutShtLoseICNeedSetErrBin==true ||
                   bIndexDropICNeedSetErrBin==true)                                                                                                             //JerryYang 20220923 : add index arm drop error設ERROR BIN
                {
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(bTestSiteNeedSetErrBin[i][j]==true &&
                               TestSocket.Item[i][j]!=NULL_IC     &&
                               TestSocket.Item[i][j]!=HAS_NULL_IC &&
                               TestSocket.Item[i][j]<TEST_PASS      )
                            {
                                TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                iErrCnt++;
                            }
                        }
                    }

                    if(iErrCnt>0)
                    {
                        if(bOutShtLoseICNeedSetErrBin)
                        {
                            ErrCnt.sprintf("Out Shuttle lose IC, Arm 2 set to error Bin (before test) : %d pcs",iErrCnt);
                            MyDBIProcess("Message", ErrCnt);
                        }
                        else
                        {
                            ErrCnt.sprintf("Index Arm Drop error, Arm 2 set to error Bin (before test) : %d pcs",iErrCnt);
                            MyDBIProcess("Message", ErrCnt);
                        }
                    }
                }
            }
            bFinshTest=false;

            if(IniConfig.bI27_ManualSortMode &&                                                                                                                 //Steven 20150915 : For TSMC 手動整盤功能
               LastSet.iTester==OFF_LINE &&
               bRunManualSortMode==true)                                                                                                                        //ChungHung 20140730 add ContinuousFailHaveOneCycle add bIndexArmNoTestting
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
            else if(bIndexArmNoTestting ||
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
                        ret=ProcessTestResult(1);
                    }
                }
                else
                {
                    ret=ProcessTestResult(1);
                }
            }

            if(TestSocket.HasRealIC()==false)                                                                                                                   //Steven 20151016 : Fixed for Hang up while no devices in socket.
            {
                ret=1;
            }

            iWhichIndexArm=2;                                                                                                                                   //Sam 20231214 : Temp offset use ready temp range
            if(ret==1)
            {
                if(IniConfig.bD23EveryDeviceDoubleContactFirstNoTesting)
                    iDoubleCount=0;                                                                                                                             //ChungHung 20140709 add for SPIL
                bFirstIn=true;                                                                                                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將out shuttle lose IC對應的site設成error bin

                if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0)                                                                                  //Ifor 20220317 add:關Arm不累積Contact次數)
                {
                }
                else
                {
                    if(IniConfig.bL28TempOfsUseReadyTempRange &&
                       bEnable_KLT_Function==false)                                                                                                             //Sam 20231214 : Temp offset use ready temp range
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
                        iInitContactCount++;                                                                                                                    //Steven 20141117 : 起測時溫度要補Offset
                    }
                }

                bTestingStopAllMotor=false;                                                                                                                     //jou 2013-09-25 Testing Need Stop All Motor
                bTestDuplicateErr=false;                                                                                                                        //Steven 20120102 : 測試ok要重置Flag
                bInitialMaxTime=false;                                                                                                                          //jou 2011-11-09 增加initial max time set
#if 0 // [W7Bk4 GATE G1] golden :2663  SetHangupMaxTime() -- see GATE REGISTER in this part's banner
                SetHangupMaxTime();                                                                                                                             //Wei 20230224 : 重置死機時間
#endif // [W7Bk4 GATE G1]
                if(iInitContactCount>Temperature.iCintactCntForTempOffsetAtInitial)                                                                             //kevin 20160312
                {
                    bTestOverTimeTempOffsetF=false;
                }

                if(iInitContactCount==Temperature.iCintactCntForTempOffsetAtInitial)                                                                            //Ifor 20190605 :add ATC Init Offset 設定
                {
#if 0 // [W7Bk4 GATE G2] golden :2671  fLotInfo->SetATCOffset(bool) -- see GATE REGISTER in this part's banner
                    fLotInfo->SetATCOffset(true);                                                                                                               //Ifor 20190605 :add ATC Init Offset 設定
#endif // [W7Bk4 GATE G2]
                    iInitContactCount++;
                }

                if(bloadcellRece)                                                                                                                               //kevin 20191029 add load cell read
                {
                    if(iloadcellRece==2)                                                                                                                        //kevin 20190906 add load cell command
                    {
                                                                                //kevin 20190906 add load cell NG
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
                    if(TestIF_File.iShuttleMode==0 ||
                       (TestIF_File.iShuttleMode==1 &&
                        TestIF_File.iShuttle_Sel==1))                                                                                                           //Sam 20200214 : 修正關 Arm 不需要送通訊
                    {
                        if(DoEndMode(fAutomation->TestMode)==false)                                                                                             //ChungHung 聚成 Test End
                            return false;
                    }
                }

                if(IniConfig.bEnableSocketCommunication)                                                                                                        //ChungHung 20130112 add for ASE_KR Socket Tester
                {
#if 0 // [W7Bk4 GATE G3] golden :2707  fSocketCommunication->bSendBinMapReport -- see GATE REGISTER in this part's banner
                    fSocketCommunication->bSendBinMapReport=true;
#endif // [W7Bk4 GATE G3]
                }

                SW[SwTesterPower].Off();
                bHasFailIC=false;

                if(IniConfig.bD37EnableManualProcess)                                                                                                           //ChungHung 20150526 add for QualComm US
                {
                    bHangTimePause=true;
                    if(ShowTestStatus==false)
                    {
                        ShowTestStatus=true;
                        ProcessShowTestStatus(1);
                    }

                    if(CosFunction.bEnableSoftWareControlButton)                                                                                                //ChungHung 20150609 add only for TSMC
                    {
                        fMain->BtnT_Start->Enabled=true;
                        fMain->BtnSTEP->Enabled=true;
                        W7Bk4_FMAIN_BTNZUPDOWN->Enabled=true;
                    }
                    bManualStep=WaitManualStepKey();
                    if(bManualStep==false)
                        bManualTStart=WaitManualStartKey();
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    if(bManualTStart)
                    {
                        SetTestTimeOutTimer(1);                                                                                                                 //Steven 20200407 : 整合Time Out時間設定

                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                if(TestSocket.Item[i][j]!=NULL_IC && TestSocket.Item[i][j]!=HAS_NULL_IC)
                                    TestSocket.SetItemData(i, j, HAS_IC);
                            }
                        }

                        if(CosFunction.bEnableSoftWareControlButton)                                                                                            //ChungHung 20150609 add only for TSMC
                        {
                            if(W7Bk4_FMAIN_BTNZUPDOWN->Down)
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
                            W7Bk4_FMAIN_BTNZUPDOWN->Enabled=false;
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
                        W7Bk4_FMAIN_BTNZUPDOWN->Enabled=false;
                        fMain->BtnSTEP->Color=(TColor)0x00804000;
                        fMain->BtnT_Start->Color=(TColor)0x00804000;
                        W7Bk4_FMAIN_BTNZUPDOWN->Down=false;
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

                if(bHasFailIC || DeviceForm.DummyMode ||
                   bNeedReplunge_RFMD)                                                                                                                          //Steven 20201022 : For RFMD
                {
                    iTestCount++;
                    iMaxDoubleContact=2;
                    if(Prod.bD22SupportMultiDoubleContact)
                      // IniConfig.bD22VerifyMode)                              //Sam 20231117 : 整合到 QA 模式 //Sam 20221012 : 新增 VerifyMode 功能
                    {
                        if(CUSTOMER_CODE==CC_TERAPOWER && iTestCount==1)                                                                                        //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
                        {
                            for(int i=0; i<TestSocket.iShtRow; i++)
                            {
                                for(int j=0; j<TestSocket.iShtCol; j++)
                                {
                                    TestSocket.iBinDataBackUp[i][j]=TestSocket.iBinData[i][j];
                                }
                            }
                            ProcessCount(1);
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
                        ProcessShowTestStatus(1);
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                if(TestSocket.Item[i][j]!=NULL_IC     &&
                                   TestSocket.Item[i][j]!=HAS_NULL_IC)
                                    TestSocket.SetItemData(i, j, HAS_IC);
                            }
                        }
                        bDoubleContact=true;
                        Task=3000;
                        break;
                    }
                    else if(iTestCount<iMaxDoubleContact)
                    {
                        ProcessShowTestStatus(1);
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
                    ret2=W7Bk4_RecordEndTestTime(1);                                                                                                                  //Sam 20201231 : 修正關 Arm 後，Index Cycle time 異常。0:arm1 1:arm2 2:雙Arm
                else
                    ret2=1;

                if(DeviceForm.ContactMode==DropContact)
                    DropContactTimer1.LatchCycleTime(true);                                                                                                     //JerryYang 20170425 (wei) 第一段時間, 測試完成到另一支arm下降到drop高度

                bATC_SITE_2ND_CHECK[1]=false;
                if(ATC_SYSTEM==eNewATCSystem)                                                                                                                   //Ifor 20160509 add ATC 測試時開啟第二點溫度監控
                {
                    for(int i=0; i<iATC_Use_Heat_Count; i++)                                                                                                    //Ifor 20160516 修改ATC Heat 設定數
                        bATCSiteTest[i]=false;
#if 0 // [W7Bk4 GATE G4] golden :2906  ATC_InterfaceForm->SiteTesting(int,bool*) -- see GATE REGISTER in this part's banner
                    ATC_InterfaceForm->SiteTesting(iATC_Use_Heat_Count, bATCSiteTest);
#endif // [W7Bk4 GATE G4]
                }
                else if(ATC_SYSTEM==eATCHonPrecType)
                {
#if 0 // [W7Bk4 GATE G5] golden :2910  ATCInterfaceForm->SendTestEnd(int) -- see GATE REGISTER in this part's banner
                    ATCInterfaceForm->SendTestEnd(1);
#endif // [W7Bk4 GATE G5]
                }

                SetNoiseDelay=false;
                TestISTimeOut=false;

                if(CUSTOMER_CODE==CC_TERAPOWER &&
                   Prod.bD22SupportMultiDoubleContact)                                                                                                          //Sam 20180612 : 有開啟 D22 Double Contact Contact， 以第一次的測試結果來做 ProcessCount
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
                        ProcessCount(1);
                    }
                }
                else
                {
                    ProcessCount(1);
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
                    RecordHistroy(1);
                    W7Bk4_MoveAllItem(BTestSuck, TestSocket);
                }
                bP65QAReTest=false;                                                                                                                             //Ifor 20260407 add: [P65] QA ReTest done, clear flag
                bFinshTest=true;
                bInitStartDelayNotFinish=true;                                                                                                                  //Ifor 20181220 : add Init Start Delay Time Not Finish
                bTJControlMode=false;                                                                                                                           //Ifor 20190328 : add TJ Temp Over Range
#if 0 // [W7Bk4 GATE G6] golden :2958  ATC_InterfaceForm->SendHandler2DID(int,bool) -- see GATE REGISTER in this part's banner
                ATC_InterfaceForm->SendHandler2DID(0, false);
#endif // [W7Bk4 GATE G6]
                if(IniConfig.bTesterTimeUpErrorNeedPassword==true &&
                   CUSTOMER_CODE==CC_LINGSEN &&                                                                                                                 //jou 2012-08-28 菱生要求 Test Time Up Error 之後的兩次測試, 不管結果如何都排到R
                   iTestTimeUpErrContinueR>0)
                {
                    iTestTimeUpErrContinueR--;
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Item[i][j]!=NULL_IC &&
                               BTestSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                BTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                            }
                        }
                    }

                    MyDBIProcess("Message", "Arm 2 Tester Time Up error place to R Bin : "+AnsiString(BTestSuck.CountRealIC())+" pcs" );
                }
                else if(TestIF_File.bOutShtLoseICSetErrUntilOneCycle==true ||                                                                                   //JerryYang 20170610 (wei) JSCC要求Out shuttle lose IC需自動one cycle,並將對應的site設為Error bin
                        TestIF_File.bIndexDropICSetErrUntilOneCycle)                                                                                            //JerryYang 20220923 : add index arm drop error設ERROR BIN
                {
                    if(bOutShtLoseICNeedSetErrBin==true ||
                       bIndexDropICNeedSetErrBin==true)                                                                                                         //JerryYang 20220923 : add index arm drop error設ERROR BIN
                    {
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                if(bTestSiteNeedSetErrBin[i][j]==true &&
                                   BTestSuck.Item[i][j]!=NULL_IC     &&
                                   BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                   BTestSuck.Item[i][j]!=TEST_PASS+iTestBinCount)
                                {
                                    BTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                    iErrCnt++;
                                }
                            }
                        }

                        if(iErrCnt>0)
                        {
                            if(bOutShtLoseICNeedSetErrBin)
                            {
                                ErrCnt.sprintf("Out Shuttle lose IC, Arm 2 set to error Bin(After test) : %d pcs",iErrCnt);
                                MyDBIProcess("Message", ErrCnt);
                            }
                            else
                            {
                                ErrCnt.sprintf("Index Arm Drop error, Arm 2 set to error Bin(After test) : %d pcs",iErrCnt);
                                MyDBIProcess("Message", ErrCnt);
                            }
                        }
                    }
                }

                if(ret2!=1)                                                                                                                                     //Steven 20201102 : For Murata資料上拋
                {
                    iXMLRetryCnt++;
                    DoBTestSuckTestICDelay.SetSecAndOn(60);
                    Task=7000;
                }
                else
                {
                    iXMLRetryCnt=0;                                                                                                                             //Steven 20201102 : For Murata資料上拋
                    if(CheckContactOver())
                    {
                        iDoInterFaceErrorStepTask=1;
                        bContactCTOverCHK=true;
                        Task=2500;
                    }
                    else
                    {
                        if(bEchoStop==true)                                                                                                                     //ChungHung 20130326 add
                        {
                            Task=2500;
                            iDoInterFaceErrorStepTask=1;
                            return false;
                        }

                        iDoubleCount=0;                                                                                                                         //ChungHung 20140709 add for SPIL
                        //Ifor 20260407 add: [P65] ARM QA Mode ReTest - Arm2
                        //==>
                        if(IniConfig.bP65EnableArmQAMode && IniConfig.iP65ArmQAModeValue > 0 && iP65QAReTestCount < IniConfig.iP65ArmQAModeValue)
                        {
                            bool bAllBin1_P65=true;
                            for(int qi=0; qi<BTestSuck.iShtRow; qi++)
                            {
                                for(int qj=0; qj<BTestSuck.iShtCol; qj++)
                                {
                                    if(BTestSuck.Item[qi][qj]!=NULL_IC && BTestSuck.Item[qi][qj]!=HAS_NULL_IC)
                                    {
                                        if(BTestSuck.bPass[qi][qj]==false)
                                            bAllBin1_P65=false;
                                    }
                                }
                            }

                            if(bAllBin1_P65 && BTestSuck.HasRealIC())
                            {
                                RecordProcess("P65 QA ReTest Arm2: count="+AnsiString(iP65QAReTestCount+1)+"/"+AnsiString(IniConfig.iP65ArmQAModeValue));
                                for(int qi=0; qi<BTestSuck.iShtRow; qi++)
                                {
                                    for(int qj=0; qj<BTestSuck.iShtCol; qj++)
                                    {
                                        if(BTestSuck.Item[qi][qj]!=NULL_IC && BTestSuck.Item[qi][qj]!=HAS_NULL_IC && BTestSuck.bPass[qi][qj])
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
                        //Ifor 20260407 add: [P65] ARM QA Mode ReTest - Arm2
                        if(CosFunction.bEnableAfterTestedDelay)                                                                                                 //ChungHung 20140730 add for ATK function after tested delay time
                        {
                            iAfterTestedCount=Prod.dAfterTestedDelay;
                            dwStartAfterTestCount=MyTickCount();
                            Task=2410;
                        }
                        //==> Eastsun 20260511 F007 整合: Ifor 20200622+20240430 KYEC 包覆 D79 Purge + SECS Yiel + ATC cooling 三段
                        else if(CUSTOMER_CODE==CC_KYEC_LEE)
                        {
                            if(CosFunction.bUseIndexPickShuttleErrNeedPurge==true     &&
                               IniConfig.bD79EnableIndexPickShuttleErrNeedPurge==true &&
                               bBTestSuckHasError==true                              ) //Ifor 20200622 add
                            {
                                Task=8000;
                                bBTestSuckHasError=false;
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
                        //<== Eastsun 20260511 F007 整合
                        else
                        {
                            return true;
                        }
                    }
                }
            }
            else if(ret==2 && W7Bk4_NowNoteIsShow()==false)                                                                                                           //interface error  //ChungHung 20130110 add 防止訊息重複進入
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
#if 0 // [W7Bk4 GATE G7] golden :3125  fSocketCommunication->bSendBinMapReport -- see GATE REGISTER in this part's banner
                    fSocketCommunication->bSendBinMapReport=true;
#endif // [W7Bk4 GATE G7]
                }
                bFirstIn=true;                                                                                                                                  //JerryYang 20170610 (wei) 只需進來一次,測試前需將Out shuttle lose IC對應未測的site設為error bin
                RecordPiggyBackStartEnd(true);
                bInitialMaxTime=false;                                                                                                                          //jou 2011-11-09 增加initial max time set
#if 0 // [W7Bk4 GATE G8] golden :3130  SetHangupMaxTime() -- see GATE REGISTER in this part's banner
                SetHangupMaxTime();                                                                                                                             //Wei 20230224 : 重置死機時間
#endif // [W7Bk4 GATE G8]
                SetNoiseDelay=false;
                Task=2200;
            }
            else if((LastSet.iTester==ON_LINE &&                                                                                                                //Steven 20150713 : 整理LastSet.iTester
                     hBTestTimeOutDelay.Off() &&                                                                                                                //Jou 20101018  //Time Out
                     W7Bk4_NowNoteIsShow()==false) ||                                                                                                                 //ChungHung 20130110 add 防止訊息重複進入
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
                        bD52IndexArmUp=true;                                                                                                                    //JerryYang 20200812 : fix RTC error hang up
                        break;
                    }
                }

                if((LastSet.iTester==ON_LINE &&                                                                                                                 //Steven 20150713 : 整理LastSet.iTester
                    hBTestTimeOutDelay.Off()) ||                                                                                                                //Jou 20101018
                   (bEcho && bTimeOutForNoFullSite==true))                                                                                                      //Steven 20141016 : FullSite的Test Time Out
                {
                    if(W906_FormShowing("fNote", fNote->fShow))                                                                                                                            //JerryYang 20200408 : 修正Alarm畫面佔住時,不會發出Test time out的問題  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
                        break;
                    bEcho=false;                                                                                                                                //Steven 20150306 : Fixed for FullSite的Test Time Out
                    bTimeOutForNoFullSite=false;

                    ret=ProcessTesterTimeOut(1);

                    if(ret==2)
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
                    else
                    {
                        if(ret==1)
                            ProcessTestResult(1);

                        SetNoiseDelay=false;
                        TestISTimeOut=false;
                        ProcessCount(1);

                        if(TestSocket.HasRealIC())                                                                                                              //Steven 20210218 : 修正測試時間的紀錄
                            RecordHistroy(1);

                        if(TestSocket.UseSiteHasIC())                                                                                                           //KenHsieh 20231208 : 修改為有資料才傳，避免重複移資料導致被覆蓋為NULL
                            W7Bk4_MoveAllItem(BTestSuck, TestSocket);

                        bFinshTest=true;
                        bInitStartDelayNotFinish=true;                                                                                                          //Ifor 20181220 : add Init Start Delay Time Not Finish
                        bTJControlMode=false;                                                                                                                   //Ifor 20190328 : add TJ Temp Over Range
                        bTimeOutForNoFullSite=false;                                                                                                            //ChungHung 20141017 fix Full Site Test Time Out problem
#if 0 // [W7Bk4 GATE G9] golden :3237  ATC_InterfaceForm->SendHandler2DID(int,bool) -- see GATE REGISTER in this part's banner
                        ATC_InterfaceForm->SendHandler2DID(0, false);
#endif // [W7Bk4 GATE G9]
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
                            //==> Eastsun 20260511 F007 整合: Ifor 20200622+20240430 KYEC 包覆 D79 Purge + SECS Yiel + ATC cooling 三段
                            else if(CUSTOMER_CODE==CC_KYEC_LEE)
                            {
                                if(CosFunction.bUseIndexPickShuttleErrNeedPurge==true     &&
                                   IniConfig.bD79EnableIndexPickShuttleErrNeedPurge==true &&
                                   bBTestSuckHasError==true                              ) //Ifor 20200622 add
                                {
                                    Task=8000;
                                    bBTestSuckHasError=false;
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
                            //<== Eastsun 20260511 F007 整合
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
                //==> Eastsun 20260511 F007 整合: Ifor 20200622+20240430 KYEC 包覆 D79 Purge + SECS Yiel + ATC cooling 三段 (case 2410 body)
                if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    if(CosFunction.bUseIndexPickShuttleErrNeedPurge==true     &&
                       IniConfig.bD79EnableIndexPickShuttleErrNeedPurge==true &&
                       bBTestSuckHasError==true                              )  //Ifor 20200622 add
                    {
                        Task=8000;
                        bBTestSuckHasError=false;
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
                //<== Eastsun 20260511 F007 整合
                }
            break;
        case 2500:
            if(SoftStop || SystemStart==false)                                                                          //Steven 20111004
                break;

            if(DoInterFaceErrorStep(TESTZ2UP))
            {
                HangTime.SetSecAndOn(Prod.iHangupMaxTime);                                                              //ChungHung 20140516 add  使用 LastSet.bInterFaceErrHeadNeedUp 時 會發生Hangup 訊息
                bHangTimePause=true;                                                                                    //Steven 20090827 Start: Hang Up dectector
                RecordProcess("TESTZ2UP");
                if(W7Bk4_Tested(BTestSuck))                                                                                  //是否有測試完的IC
                {
                    return true;
                }
                else
                {
                    W7Bk4_CopyFrom(TestSocket, BTestSuck);
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

            DoBTestSuckTestICDelay.SetSecAndOn(0.3);
            Task=2700;
            break;
        case 2700:
            if(DoBTestSuckTestICDelay.Off())
            {
                Task=2200;
            }
            break;
        case 3000:
            if(CosFunction.bEnableSoftWareControlButton &&
               W7Bk4_FMAIN_BTNZUPDOWN->Down)                                         //ChungHung 20150609 add only for TSMC
            {
                #ifdef SOFT_SIMULTE
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+10000, 1))
                {
                    DoBTestSuckTestICDelay.SetSecAndOn(0.5);
                    Task=3050;
                }
                break;
                #else
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+500, MOT[MTestZ2].GailSpeed))
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
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+10000, 1))
                {
                    DoBTestSuckTestICDelay.SetSecAndOn(0.5);
                    Task=3050;
                }
                break;
            #else
                if(CUSTOMER_CODE==CC_JSCC_OS)                                                                           //RogerYang 20260126 : JSCC_OS 第二次contact要拉高慢放
                {
                    iDbContactHigh=5000;
                    if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+iDbContactHigh, MOT[MTestZ2].GailSpeed))
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
            if(DoBTestSuckTestICDelay.Off())
            {
                Task=3100;
            }
            break;
        case 3100:
            if(CUSTOMER_CODE==CC_JSCC_OS)                                                                               //RogerYang 20260126 : JSCC_OS 第二次contact要拉高慢放
            {
                iDbContactHighSlowSpd=MOT[MTestZ2].GailSpeed/2;
            }
            else
            {
                iDbContactHighSlowSpd=MOT[MTestZ2].GailSpeed;
            }

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iDbContactHighSlowSpd))              //Alick 20161122 修正-Prod.TestZ2_Drop_Offset，開D23功能下壓會不到位造成O/S
            {
                if(CosFunction.bEnableSoftWareControlButton &&
                   W7Bk4_FMAIN_BTNZUPDOWN->Down)                                                                             //ChungHung 20150609 add only for TSMC
                {
                    W7Bk4_FMAIN_BTNZUPDOWN->Down=false;
                    ProcessStartTestData(0);                                                                            //畫面顯示黃色，測試中
                    InitTestTask();
                    Task=2400;
                }
                else
                {
                    if(bDoubleContact)                                                                                  //JerryYang 20220923 : After double contact use initial delay
                    {
                        RecordProcess("After double contact use initial delay");
                        bNeedInitialTestDelay=true;                                                                     //ChungHung 20140425 add for TSMC Device
                    }
                    InitTestTask();
                    iRTCErrorCount=0;                        //wei 20221222 RTC ARM Error
                    Task=2200;
                }
            }
            break;
        case 5000:
            if(CUSTOMER_CODE==CC_KYEC_XILINX && IniConfig.bD01EnableReadTorque &&
               IniConfig.bChangeKitNoHardStop==true &&                          //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
               IniConfig.bRemeberAutoHeight==true)
            {
                DoBTestSuckTestICDelay.SetSecAndOn(IniConfig.dD01ReadTorqueDelayTime);
                Task=5050;
            }
            else                                                                //Steven 20100617 Start: Add form 9080A for 即時更新扭力值
            {
                DoBTestSuckTestICDelay.SetSecAndOn(0.01);                       // for switch read torque relay
                Task=5100;
            }
            break;
        case 5050:
            if(DoBTestSuckTestICDelay.Off())
            {
                DoBTestSuckTestICDelay.SetSecAndOn(0.01);                       // for switch read torque relay
                Task=5100;
            }
            break;
        case 5100:
            if(DoBTestSuckTestICDelay.Off())
            {
                W64bT2_FMAIN_CHKREADTORQUE1->Checked=false;                           //2008/06/24 lee
                W64bT2_FMAIN_CHKREADTORQUE2->Checked=true;                            //2008/06/24 lee

                W64bT2_FMAIN_EDTORUE1->Text="";
                W7Bk4_FMAIN_LBARM1TORQUE->Caption="2:Reading";
                iReadCount=0;
                DoBTestSuckTestICDelay.SetSecAndOn(5);                          //kevin 20210902 2->5   //2008/07/15 lee
                Task=5300;
            }
            break;
        case 5300:
            if(W64bT2_FMAIN_EDTORUE1->Text!="")
            {
                iReadCount++;

                if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                   IniConfig.bD01EnableReadTorque)                              //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
                {
                    ShowMainScreenPresure(1);
                    if(atoi(W64bT2_FMAIN_EDTORUE1->Text.c_str())>=(IniConfig.dD01ReadTorque+DeviceForm_File.dZ2Torue))
                        Task=5400;
                    else
                        Task=100;                                               //kevin 20130611
                }
                else
                {
                    if(iReadCount>1)                                            //2008/07/15 lee
                    {
                        ShowMainScreenPresure(1);

                        if(IniConfig.bControlTorque)                            //jou 2013-11-05 Index Control Torque
                            iMaxPreasure=DeviceForm.iIndexTorqueMax;
                        else
                            iMaxPreasure=Prod.iMaxPreasure;

                        if(atoi(W64bT2_FMAIN_EDTORUE1->Text.c_str())>=iMaxPreasure)
                            Task=5400;
                        else
                            Task=100;                                           //kevin 20130611
                    }
                    else
                    {
                        W64bT2_FMAIN_EDTORUE1->Text="";
#if 0 // [W7Bk4 GATE G10] golden :3534  COM2->InitReadTorueTask() -- see GATE REGISTER in this part's banner
                        COM2->InitReadTorueTask();
#endif // [W7Bk4 GATE G10]
                        bReadMCU2=true;                                         //kevin 20220225 read MCU DATA
                    }
                }
            }
            else if(DoBTestSuckTestICDelay.Off())
            {
                W64bT2_FMAIN_CHKREADTORQUE1->Checked=true;                            //2008/07/15 lee
                W64bT2_FMAIN_CHKREADTORQUE2->Checked=false;                           //2008/07/15 lee
                Task=5350;
                iArm2ReadtorquCount++;                                          //kevin 20210902 add 扭力讀取
                DoBTestSuckTestICDelay.SetSecAndOn(5);                          //kevin 20210902 2->5
            }
            break;
        case 5350:
            if(DoBTestSuckTestICDelay.Off())
            {
                if(iArm2ReadtorquCount>3)
                {
                    ShowMainScreenPresure(1);
                    iArm2ReadtorquCount=0;
                    Task=100;                                                   //kevin 20130611
                    break;
                }
                Task=5000;
            }
            break;
        case 5400:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, 30000, "DoBTestSuckTestIC 5400"))
            {
                Task=5500;
                if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                   IniConfig.bD01EnableReadTorque)                              //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
                {
                    Task=5450;
                }
            }
            break;
        case 5450:                                                              //Frank 20170626 add Xilinx 浮動Shuttle Kit 強制開啟[D01]
            if(IndexAlarmInArmAway()==true)
            {
                ErrPart="The test head 2, contact force over error";            //kevin 20130418
                bIsContactforce=true;                                           //kevin 20130418 contact force over 需開們確認
                ShowErrorMessage("WAR0321", K_SKIP, MTestZ2,false, ErrPart);
                fAllMotorHome=false;
                iHome=1;
                Task=1;
            }
            break;
        case 5500:
            ErrPart="The test head 2, contact force over error";                //kevin 20130418
            bIsContactforce=true;                                               //kevin 20130418 contact force over 需開們確認

            if(CosFunction.bIndexAreaOnlyCanUseSkip)                            //Steven 20141105 : Index內的所有異常都只能用Skip
                ShowErrorMessage("WAR0321", K_SKIP, MTestZ2, false, ErrPart);
            else
                ShowErrorMessage("WAR0321", K_RETRY, MTestZ2, false, ErrPart);

            fAllMotorHome=false;
            iHome=1;
            Task=1;
            break;
        case 6000:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+1000, MOT[MTestZ2].GailSpeed))
            {
                DoBTestSuckTestICDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);
                Task=6100;
            }
            break;
        case 6100:
            if(DoBTestSuckTestICDelay.Off())
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
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iOverDriveDistance, MOT[MTestZ2].GailSpeed*Prod.TestZ_Drop_Speed/100))
            {
                DoBTestSuckTestICDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);
                Task=6300;
            }
            break;
        case 6300:
            if(DoBTestSuckTestICDelay.Off())
            {
                fMain->SendMSG_CMD(MSG_CMD_OverDrive);
                bDoOverDrive=false;
                Task=2400;
            }
            break;
        case 6500:                                                              //Steven 20151207 : Recontact for TSMC
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed/100))
            {
                iDoubleCount++;
                DoBTestSuckTestICDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);
                Task=6600;
            }
            break;
        case 6600:
            if(DoBTestSuckTestICDelay.Off())
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
        case 7000:                                                                                                      //Steven 20201102 : For Murata資料上拋
            if(iXMLRetryCnt>=3)
            {
                ret=ShowErrorMessage("WAR16321", K_RETRY|K_RESET, MTestZ2, false);                                      //Steven 20201113 : 上傳失敗3次要有Alarm
                if(ret==K_RESET)
                {
                    fMain->Reset("DoBTestSuckTestIC");
                    Task=7100;
                }
                else
                {
                    iXMLRetryCnt=0;
                    DoBTestSuckTestICDelay.SetSecAndOn(30);
                    Task=7000;
                }
            }
            else if(DoBTestSuckTestICDelay.Off())
            {
                ret2=W7Bk4_SendTestResultToHttp();

                if(ret2!=1)
                {
                    iXMLRetryCnt++;
                    DoBTestSuckTestICDelay.SetSecAndOn(30);
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
                if(bEchoStop==true)                                             //ChungHung 20130326 add
                {
                    Task=2500;
                    iDoInterFaceErrorStepTask=1;
                    return false;
                }

                iDoubleCount=0;                                                 //ChungHung 20140709 add for SPIL
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
        //==> Eastsun 20260511 F007 整合: Ifor 20200622+20240430 IndexPickShuttleErrPurge case 8000~8012 + KYEC SECS case 9000/9001 + ATC cooling case 9100/9101 (T8 Rear)
        //Steven 20201102 : For Murata資料上拋
        case 8000:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bBTestSuckError[i][j]==true)
                    {
                        fiosetview->bIndexSuck[1][i][j]=true;
                    }
                }
            }
            Task=8001;
            break;
        case 8001:
            bSuck_OK=W64bT2_ProcessIndexSuckDestroy2();
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
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);  //Steven 20140620 : 整合為Function
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos, iSpeedFast))
            {
                Task=8003;
            }
            break;
        case 8003:  //Check IO Status
            AllErrPart="";
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bBTestSuckError[i][j]==true)
                    {
                        W7Bk4_CheckVaccumIsIniaialON(BTestSuck, i, j, flag2);
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
                for(int i=0; i<MAX_Index_Row; i++)
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                        bBTestSuckError[i][j]=false;

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
            if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+300, MOT[MTestZ2].GailSpeed, 0))
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bBTestSuckError[i][j]==true)
                        {
                            BTestSuck.Suck[i][j].Off();
                        }
                    }
                }
                DoBTestSuckOffDelay.SetMSAndOn(ArmSpeed[IndexArm].dCTAirOn*1000);
                Task=8012;
            }
            break;
        case 8012:
            if(DoBTestSuckOffDelay.Off())
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bBTestSuckError[i][j]==true)
                        {
                            BTestSuck.Suck[i][j].Normal();
                            bBTestSuckError[i][j]=false;
                        }
                    }
                }
                ShowErrorMessage("WAR0310", K_HOME, MTestY2, false, AllErrPart);
                fAllMotorHome=false;
                iHome=1;
            }
            break;
        case 9000:
            if((TestIF_File.iShuttleMode==1) && (TestIF_File.iShuttle_Sel==0))
            {
                return true;
            }
            else
            {
                if(iSECSGEM_ConsecutiveFailureAlarm==1)                         //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
                {
                    iSECSGEM_ConsecutiveFailureAlarm=2;
                }

                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed))
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
                ShowErrorMessage("WAR07362", K_RETRY, MTestZ2, false, "");  //Vacuum Sensor Off Error
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
        //<== Eastsun 20260511 F007 整合 (T8)
        }
    return false;
}
// ==== GOLDEN aTester_Rear.cpp :2222-3878 ENDS HERE ====

//------------------------------------------------------------------------------
//  PART FILE  k6-BRTC-pair 1/2  --  golden aTester_Rear.cpp:3882-3999 (cp950)
//  Translator: AI(k6-BRTC-pair) 20260810
//
//  ROLE
//    DoBRTCUseSocketFloat(bool bInitial) -- the Rear (Z2/Arm2) "use socket
//    float" RTC full-view sequence.  Parks Z2 at TestZ2_Safe, checks both Z
//    encoders, drives Y1/Y2 to Front/Rear, asks the RTC vision system for a
//    CHECKNULL + full-view OK, handles the NG path (WAR0346 Retry/Skip) and the
//    time-out path (WAR0337), then steps Y2 to Middle, presses Z2 down to test
//    height (honouring TestZ2_Drop_Offset) and leaves IndexStatus=Z1Up_Z2Down.
//
//  WAVE SCOPE
//    DoBRTCUseSocketFloat(bool)                 ACTIVE   golden :3882-3999
//    TQPF_Timer DoUseSocketTestYRearDelay       ACTIVE   golden :3880
//      (file-scope peer global, OUTSIDE the assigned 3882..3999 range -- see
//       PEER GLOBAL below for why it is here and how to remove it)
//
//  GATE REGISTER (shared by both part files of this pair)
//  ---------------------------------------------------------------------------
//  G-k6-SEAM  (NOT an #if 0 -- a member-substitution seam; registered so it is
//             not mistaken for a real wiring)
//    GOLDEN LINES     :3915-3918, :3923, :3927, :3932, :3993 (part 1) and
//                     :4054, :4064-4075, :4207-4218, :4227, :4235, :4239 (part 2)
//                     -- i.e. every `COM2->` except DoReleaseAndInspEnd, plus
//                     DoReleaseAndInspEnd itself (see note in the seam block).
//    WHY THE OFFLINE DEFAULT IS FAITHFUL
//                     Every RT channel reads false, so each golden branch that
//                     asks "did vision answer?" takes the golden ELSE arm that
//                     already exists for the no-answer case -- :3937 (WAR0337
//                     FullT time-out -> Task=100 retry) and :4242 (DoGiveWayDelay
//                     time-out -> ShowMyMessage + Task=1).  No branch is invented
//                     and no branch is skipped.  SendCommToVision/RTC_ResetAlarm/
//                     InitRealTimeCCDPara are pure serial-TX / reset side effects
//                     with no value the SM reads back, so no-op'ing them removes
//                     no decision.  sRealTimeCom_Send[] IS kept assignable, so
//                     golden :4054's @TESTSITE frame is still built and stored.
//    REAL-MACHINE DIFFERENCE
//                     On the machine the RTC vision controller receives
//                     CHECKNULL / CHECKIC / TESTMAP / FullTOK and answers OK or
//                     NG; DoBRTCUseSocketFloat would advance case 150 -> 200 and
//                     DoBRTCGiveWayCheck case 20 would RETURN TRUE.  Under the
//                     seam neither ever gets an answer, so both spin their own
//                     golden time-out path: DoBRTCUseSocketFloat loops
//                     100 -> 150 -> 100 raising WAR0337 each pass, and
//                     DoBRTCGiveWayCheck loops 20 -> 1 raising "RTC Release Time
//                     out of Give Way Check".  NEITHER EVER COMPLETES OFFLINE.
//                     That is the truthful offline behaviour of a machine with no
//                     RTC camera, and it is what the port already does in
//                     atester_32Site.cpp -- but it does mean these two SMs must
//                     not be put on any ctest "must finish" path until COM2 is
//                     real.
//    ABSENCE COMMAND  python <scratch>/absence.py rtCHECKNULL rtCHECKIC rtTESTMAP sRealTimeCom_Send RTC_ResetAlarm InitRealTimeCCDPara
//                     (walks D:/HT9045/HT9011UC_Cpp_V3.33.906.0 for *.cpp/*.h,
//                      prints every hit + HITS[..] per pattern; source below)
//                       for r,d,fs in os.walk(root): skip .svn/build/_w7b_parts
//                       -> read utf-8 errors=replace, print 'file:line:text'
//                     RESULT: HITS[rtCHECKNULL]=0  HITS[rtCHECKIC]=0
//                             HITS[rtTESTMAP]=0
//                             HITS[sRealTimeCom_Send]=1 -- and that ONE hit is
//                               atester.cpp:11932, INSIDE the `#if 0 // GOLDEN
//                               VERBATIM` block atester.cpp:11862-11942, i.e. not
//                               compiled and not a home.
//                             HITS[RTC_ResetAlarm]=5 / HITS[InitRealTimeCCDPara]=8
//                               -- ALL of them are atester_32Site.cpp's OWN
//                               TU-local seam members (:370/:371) plus comments
//                               and its own call sites; none is a member of the
//                               real TCOM2Shim (atester_shims.h:420-461 read in
//                               full).
//    MEASURED AT      2026-08-10 19:11:30 +0800
//  ---------------------------------------------------------------------------
//  G-k6-1  (#if 0)
//    GOLDEN LINE      :3960   fContact->InitROILearningTask();
//    WHY THE OFFLINE DEFAULT IS FAITHFUL
//                     fContact EXISTS in the port (TfContactShim*,
//                     atester_shims.h:154-251) but has NO InitROILearningTask.
//                     Golden calls it on the K_SKIP arm of the WAR0346
//                     "RTC FullView Socket Has Device" dialog, purely to rearm
//                     the contact form's ROI-learning task counter; the SM
//                     reads nothing back from it -- the two statements that
//                     decide behaviour on that arm (bRTCFullViewError=false;
//                     Task=200;) are both KEPT ACTIVE immediately below the
//                     gate.  So skipping it changes no transition here.
//    REAL-MACHINE DIFFERENCE
//                     On the machine the contact form's ROI-learning SM is
//                     left holding its PREVIOUS task index instead of being
//                     reset to step 1, so the next Contact-mode ROI learn can
//                     resume mid-sequence rather than from the start.  Nothing
//                     in the index/test SM observes it.
//    ABSENCE COMMAND  python <scratch>/absence.py InitROILearningTask
//                     RESULT: HITS[InitROILearningTask]=7 and NOT ONE is a
//                     declaration or definition -- 3 are atester.cpp comments/
//                     the W7T1_FCONTACT_INITROI no-op macro (:5532/:5534/:5537),
//                     4 are atester_32Site.cpp comments/its own W5_32S_FCONTACT_
//                     INITROI no-op macro (:59/:109/:216 + banner).  Confirmed
//                     against class TfContactShim read in full.
//    MEASURED AT      2026-08-10 19:11:30 +0800
//    WHY IT SHOULD STAY GATED EVEN IF A SIBLING LANDS THE MEMBER (TRAP 3)
//                     If TfContactShim gains InitROILearningTask this gate can
//                     retire -- but ONLY together with atester.cpp:5537 and
//                     atester_32Site.cpp:216, which are the same golden call
//                     behind no-op MACROS (invisible at the call site).
//                     Retiring this one alone would leave the tree with the
//                     same call live in one file and silently dead in two.
//------------------------------------------------------------------------------
//  GOLDEN BUGS PRESERVED, NOT FIXED
//    * :3906-3907 the encoder guard is written
//          if(REAL_TIME_CCD==true && Z1.Gali_ReadEncoderMaxRandge(..)==false ||
//             Z2.Gali_ReadEncoderMaxRandge(..)==false)
//      which by C++ precedence is (A && B) || C -- so a Z2 encoder failure
//      raises ShowIndexMotorError even when REAL_TIME_CCD is false.  Almost
//      certainly a missing pair of parens.  KEPT VERBATIM.
//    * :3907 the Z2 encoder is compared against Prod.TestZ1_Safe (the Z1
//      safe height), not Prod.TestZ2_Safe.  KEPT VERBATIM.
//    * :3884/:3901 static bool bVerifyNG is written and NEVER read anywhere in
//      the function -- dead state.  KEPT VERBATIM.
//    * :3892 int ret; is left uninitialised; it is only ever read on the one
//      path (:3958) that assigns it first (:3952/:3954).  KEPT VERBATIM.
//------------------------------------------------------------------------------
//  PEER GLOBAL -- golden :3880, i.e. OUTSIDE this part's assigned 3882..3999.
//  It is emitted here because NOTHING in the port defines it and the body below
//  cannot compile without it (absence command: python <scratch>/absence.py
//  DoUseSocketTestYRearDelay -> HITS=0, measured 2026-08-10 19:11:30 +0800).
//  Same precedent and same wording as this file's own Wave-1 note for
//  "TQPF_Timer BTempRiseHasICDelay (golden :308, file-scope peer of
//  DoRearTestDestroyIC)" -- aTester_Rear.cpp:62-64 / :301.
//  TRAP 4 CHECK: TQPF_Timer's ctor DOES run code before main
//  (myTimer.cpp:14-17 -> CalibratePerformanceCounterOverhead), but it touches
//  ONLY its own members and QueryPerformanceCounter -- no global object, no
//  pointer deref -- so it cannot repeat the fLaserSensor/elLaser failure.  The
//  port already has two such file-scope TQPF_Timers (atester.cpp:115
//  DoGiveWayDelay, aTester_Rear.cpp:301 BTempRiseHasICDelay).
//  INTEGRATE: if a sibling part also emits golden :3880, delete ONE of the two
//  -- the block is delimited by the two markers below for a single clean cut.
//  >>>>>> k6-BRTC-pair PEER GLOBAL golden:3880 BEGIN >>>>>>
TQPF_Timer DoUseSocketTestYRearDelay;
//  <<<<<< k6-BRTC-pair PEER GLOBAL golden:3880 END <<<<<<
//
//  NOTE golden :3881 `int iBRTCUseSocketFloatTask=1;` is deliberately NOT
//  emitted -- it already exists in the port at atester_shims.cpp:196 and is
//  extern-declared at atester_shims.h:118, which aTester_Rear.cpp already
//  includes.  Emitting it would be a duplicate definition.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  TU-LOCAL COM2 RTC-VISION SEAM  (k6-BRTC-pair).  Guarded so the two part files
//  of this pair can both carry it and only the FIRST one stitched defines it.
//
//  WHY: the port's real COM2 is TCOM2Shim* (atester_shims.h:420-461, object
//  atester_shims.cpp:361 `TCOM2Shim *COM2 = new TCOM2Shim();`) and exposes only
//  bCCDDummyRum / DoReleaseAndInspEnd() / ATCAlarmSenCheck().  The two functions
//  in this pair additionally touch bRealTimeCom_ReceiveOK[] / rtTESTMAP /
//  rtCHECKIC / rtCHECKNULL / rtFullTOK / rtFullTNG / sRealTimeCom_Send[] /
//  SendCommToVision() / RTC_ResetAlarm() / InitRealTimeCCDPara().  This is the
//  SAME gap, with the SAME resolution, that the port already uses TWICE:
//  atester.cpp:5599-5618 (W7T1_TCOM2Ext) and atester_32Site.cpp:357-386
//  (W5_32S_TCOM2Ext).  Offline posture is identical to both: every RT channel
//  "never received" (false) so each golden `if(COM2->bRealTimeCom_ReceiveOK[..])`
//  arm falls to its OWN already-present time-out/retry else-branch; the three
//  vision commands no-op.  Nothing is fabricated.
//
//  TWO DELIBERATE DIFFERENCES FROM THOSE TWO PRECEDENTS -- both required here:
//   (1) NO `#define COM2`.  Those two files own their whole TU; this file is
//       STITCHED FROM PART FILES WRITTEN BY PARALLEL AGENTS, so a TU-wide
//       `#define COM2` would silently hijack a sibling's COM2 uses (and two
//       siblings each emitting one would make "which seam wins" depend on stitch
//       order).  Every golden `COM2->` in this pair is therefore spelled
//       `K6BRTC_COM2->`; the integrate step is one mechanical rename back to
//       `COM2` once TCOM2Shim carries the members.  This pair names the bare
//       identifier `COM2` ZERO times, so it is immune to any sibling's #define.
//   (2) CONSTANT-INITIALISED, NO CONSTRUCTOR (TRAP 4).  Those two precedents use
//       a real ctor that runs a loop before main().  This one is an aggregate
//       with no user-declared ctor and no non-trivially-constructible member, so
//       it is CONSTANT-initialised: zero code runs before main().  The one
//       AnsiString store golden needs (sRealTimeCom_Send[]) lives in a
//       FUNCTION-LOCAL static inside the proxy's operator[], i.e. constructed on
//       first CALL, never during static init.
//------------------------------------------------------------------------------
#ifndef K6BRTC_COM2_SEAM
#define K6BRTC_COM2_SEAM
enum { K6BRTC_rtTESTMAP=0,  K6BRTC_rtCHECKIC=1, K6BRTC_rtCHECKNULL=2,
       K6BRTC_rtFullTOK=3,  K6BRTC_rtFullTNG=4, K6BRTC_RT_N=16 };

// Stand-in for golden TCOM2::sRealTimeCom_Send[] (golden rs232.h).  operator[]
// hands back a real, assignable AnsiString so golden :4054's store is KEPT, not
// dropped -- but the storage is a function-local static, so it is created on the
// first call and never by static init (TRAP 4).
struct K6BRTC_RTSendProxy
{
    AnsiString& operator[](int ch)
    {
        static AnsiString buf[K6BRTC_RT_N];
        return buf[(ch>=0 && ch<K6BRTC_RT_N) ? ch : 0];
    }
};

struct K6BRTC_TCOM2Ext
{
    bool               bRealTimeCom_ReceiveOK[K6BRTC_RT_N];      // golden rs232.h -- offline all false ("vision never responds")
    int                rtTESTMAP, rtCHECKIC, rtCHECKNULL,        // golden rs232.h -- RT channel index ids
                       rtFullTOK, rtFullTNG;
    K6BRTC_RTSendProxy sRealTimeCom_Send;                        // golden rs232.h -- see proxy above
    void SendCommToVision(int /*ch*/, bool /*b*/){}              // golden rs232.h -- offline no-op (== both precedents)
    void RTC_ResetAlarm(){}                                      // golden rs232.h -- offline no-op (== atester_32Site.cpp:370)
    void InitRealTimeCCDPara(){}                                 // golden rs232.h -- offline no-op (== atester_32Site.cpp:371)
    // DoReleaseAndInspEnd IS a real member of the real TCOM2Shim -- routed here
    // to a no-op only so this pair never names the bare identifier `COM2` (see
    // difference (1) above).  PROVABLY equivalent today, not an approximation:
    // TCOM2Shim::DoReleaseAndInspEnd's real body is `{}` (atester_shims.cpp:335,
    // read 2026-08-10 19:05 +0800).  Registered as GATE G-k6-SEAM so the day that
    // body stops being empty this detour is found, not silently kept.
    void DoReleaseAndInspEnd(){}                                 // golden rs232.h:160
};
static K6BRTC_TCOM2Ext k6brtc_com2_ext =
{
    {false, false, false, false, false, false, false, false,
     false, false, false, false, false, false, false, false},
    K6BRTC_rtTESTMAP, K6BRTC_rtCHECKIC, K6BRTC_rtCHECKNULL,
    K6BRTC_rtFullTOK, K6BRTC_rtFullTNG,
    {}                                                           // the empty send-proxy (explicit, so -Wextra stays quiet)
};
#define K6BRTC_COM2 (&k6brtc_com2_ext)
#endif // K6BRTC_COM2_SEAM
//------------------------------------------------------------------------------
// ---- golden aTester_Rear.cpp:3882-3999 -- DoBRTCUseSocketFloat ---------------
// NOT `static`: golden gives this file-scope external linkage (golden declares
// it in no header and no other TU calls it -- see TRAP 1 note in the report),
// and a file-scope `static` definition is exactly the shadow shape (d) forbidden
// by this wave's brief.
bool DoBRTCUseSocketFloat(bool bInitial)
{
    static bool bVerifyNG=false;

    if(bInitial==true)
    {
        iBRTCUseSocketFloatTask=1;
        return false;
    }

    int ret;
    int &Task=iBRTCUseSocketFloatTask;
    AnsiString ErrPart="";

    switch(Task)
    {
        case 1:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, MOT[MTestZ1].GailSpeed, "DoBRTCUseSocketFloat 1"))
            {
                bVerifyNG=false;
                Task=100;
            }
            break;
        case 100:
            if(REAL_TIME_CCD==true && MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false ||
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false)
            {
                ShowIndexMotorError(AnsiString("DoBRTCUseSocketFloat"));
                break;
            }

            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front,Prod.TestY2_Rear,MOT[MTestY1].GailSpeed, "DoBRTCUseSocketFloat 100"))
            {
                K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTNG]=false;
                K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtCHECKNULL, true);
                K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtFullTOK, true);
                DoUseSocketTestYRearDelay.SetSecAndOn(10);
                Task=150;
            }
            break;
        case 150:
            if(K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTOK])
            {
//                RecordProcess("REALTIME CCD Full OK");
//                COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC]=false;
                K6BRTC_COM2->RTC_ResetAlarm();     //wei 20221222 RTC ARM Error
                K6BRTC_COM2->DoReleaseAndInspEnd();
                bRTCFullViewError=false;                                        //Steven 20120206 : RTC重複錯誤
                Task=200;
            }
            else if(K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTNG])
            {
                Task=170;
                break;
            }
            else if(DoUseSocketTestYRearDelay.Off())
            {
                ShowErrorMessage("WAR0337", 0, MMIndex, 0, __FUNC__);           //RTC FullT Time Out Error.
                bSendRealCCDSendStart=true;                                     //Steven 20110824 : Real time CCD - 不可以關閉CCD
                K6BRTC_COM2->DoReleaseAndInspEnd();
                Task=100;
            }
            break;
        case 170:
            if(IndexAlarmInArmAway()==true)                                                                             //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)                                                    //Steven 20151022 : add for MAXIM
                    bIsTestSitICFallDown=true;
                K6BRTC_COM2->DoReleaseAndInspEnd();
                if(CosFunction.bRTCFullViewErrorOnlyRetry)                                                              //Steven 20140529 : RTCFullViewErrorOnlyRetry
                    ret=ShowErrorMessage("WAR0346", K_RETRY, MMCCD, bRTCFullViewError, ErrPart);                        //RTC FullView Socket Has Device Error!
                else
                    ret=ShowErrorMessage("WAR0346", K_RETRY|K_SKIP, MMCCD, bRTCFullViewError, ErrPart);                 //RTC FullView Socket Has Device Error!

                bRTCFullViewError=true;                                                                                 //Steven 20120206 : RTC重複錯誤

                if(ret==K_SKIP)                                                                                         //Steven 20120823 : Run Time出錯也要RTC
                {
#if 0 // GATE G-k6-1 -- golden :3960 fContact->InitROILearningTask() has no port body (see GATE REGISTER)
                    fContact->InitROILearningTask();
#endif // GATE G-k6-1
                    bRTCFullViewError=false;
                    Task=200;
                }
                else
                {
                    Task=100;
                }
            }
            break;
        case 200:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle,MOT[MTestY1].GailSpeed, "DoBRTCUseSocketFloat 200"))
            {
                Task=300;
            }
            break;
        case 300:
            if(Prod.TestZ2_Drop_Offset!=0)
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, MOT[MTestZ1].GailSpeed))
                {
                    Task=400;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, MOT[MTestZ1].GailSpeed))
                {
                    Task=400;                                                   //Steven 20110511
                }
            }
            break;
        case 400:
            K6BRTC_COM2->InitRealTimeCCDPara();
            bBTestSuckDrop=false;
            IndexStatus=Z1Up_Z2Down;
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
//  PART FILE  k6-BRTC-pair 2/2  --  golden aTester_Rear.cpp:4003-4251 (cp950)
//  Translator: AI(k6-BRTC-pair) 20260810
//
//  ROLE
//    DoBRTCGiveWayCheck(bool bInitial, bool bCheckHasIC, int iIndex) -- the
//    RTC2.0 "give way then full-check" SM.  Builds the @TESTSITE site map from
//    FTestSuck's Item grid, and if Y1/Y2 are already parked at Front/Rear it
//    fires the vision commands straight away; otherwise it temporarily
//    OVERWRITES the Prod Y-Middle / Z-Test / Z-Drop_Offset teach values with
//    safe/park values for the requested arm, swaps the two heads
//    (Z1UpZ2Down / Z1DownZ2Up, optionally two-speed via TMoveDrop), RESTORES
//    those Prod values, then sends the vision commands and waits for FullTOK /
//    FullTNG / time-out.
//
//  WAVE SCOPE
//    DoBRTCGiveWayCheck(bool,bool,int)          ACTIVE   golden :4003-4251
//    TQPF_Timer DoGiveWayFrontDelay             ACTIVE   golden :4002
//      (file-scope peer global, OUTSIDE the assigned 4003..4251 range -- see
//       PEER GLOBAL below)
//
//  GATE REGISTER (shared by both part files of this pair)
//  ---------------------------------------------------------------------------
//  G-k6-SEAM  (NOT an #if 0 -- a member-substitution seam; registered so it is
//             not mistaken for a real wiring)
//    GOLDEN LINES     :3915-3918, :3923, :3927, :3932, :3993 (part 1) and
//                     :4054, :4064-4075, :4207-4218, :4227, :4235, :4239 (part 2)
//                     -- i.e. every `COM2->` except DoReleaseAndInspEnd, plus
//                     DoReleaseAndInspEnd itself (see note in the seam block).
//    WHY THE OFFLINE DEFAULT IS FAITHFUL
//                     Every RT channel reads false, so each golden branch that
//                     asks "did vision answer?" takes the golden ELSE arm that
//                     already exists for the no-answer case -- :3937 (WAR0337
//                     FullT time-out -> Task=100 retry) and :4242 (DoGiveWayDelay
//                     time-out -> ShowMyMessage + Task=1).  No branch is invented
//                     and no branch is skipped.  SendCommToVision/RTC_ResetAlarm/
//                     InitRealTimeCCDPara are pure serial-TX / reset side effects
//                     with no value the SM reads back, so no-op'ing them removes
//                     no decision.  sRealTimeCom_Send[] IS kept assignable, so
//                     golden :4054's @TESTSITE frame is still built and stored.
//    REAL-MACHINE DIFFERENCE
//                     On the machine the RTC vision controller receives
//                     CHECKNULL / CHECKIC / TESTMAP / FullTOK and answers OK or
//                     NG; DoBRTCUseSocketFloat would advance case 150 -> 200 and
//                     DoBRTCGiveWayCheck case 20 would RETURN TRUE.  Under the
//                     seam neither ever gets an answer, so both spin their own
//                     golden time-out path: DoBRTCUseSocketFloat loops
//                     100 -> 150 -> 100 raising WAR0337 each pass, and
//                     DoBRTCGiveWayCheck loops 20 -> 1 raising "RTC Release Time
//                     out of Give Way Check".  NEITHER EVER COMPLETES OFFLINE.
//                     That is the truthful offline behaviour of a machine with no
//                     RTC camera, and it is what the port already does in
//                     atester_32Site.cpp -- but it does mean these two SMs must
//                     not be put on any ctest "must finish" path until COM2 is
//                     real.
//    ABSENCE COMMAND  python <scratch>/absence.py rtCHECKNULL rtCHECKIC rtTESTMAP sRealTimeCom_Send RTC_ResetAlarm InitRealTimeCCDPara
//                     (walks D:/HT9045/HT9011UC_Cpp_V3.33.906.0 for *.cpp/*.h,
//                      prints every hit + HITS[..] per pattern; source below)
//                       for r,d,fs in os.walk(root): skip .svn/build/_w7b_parts
//                       -> read utf-8 errors=replace, print 'file:line:text'
//                     RESULT: HITS[rtCHECKNULL]=0  HITS[rtCHECKIC]=0
//                             HITS[rtTESTMAP]=0
//                             HITS[sRealTimeCom_Send]=1 -- and that ONE hit is
//                               atester.cpp:11932, INSIDE the `#if 0 // GOLDEN
//                               VERBATIM` block atester.cpp:11862-11942, i.e. not
//                               compiled and not a home.
//                             HITS[RTC_ResetAlarm]=5 / HITS[InitRealTimeCCDPara]=8
//                               -- ALL of them are atester_32Site.cpp's OWN
//                               TU-local seam members (:370/:371) plus comments
//                               and its own call sites; none is a member of the
//                               real TCOM2Shim (atester_shims.h:420-461 read in
//                               full).
//    MEASURED AT      2026-08-10 19:11:30 +0800
//  ---------------------------------------------------------------------------
//  NO #if 0 IN THIS PART.  Every other symbol golden :4003-4251 touches has a
//  real port home, checked one by one: FTestSuck/iShtRow/iShtCol/Item
//  (aHotPlateSubstrate.h:636/:406/:438/:370), HAS_IC/HAS_HOT_IC (cmydef.h:158/
//  :160), AnsiString::sprintf (vclcompat/AnsiString.h:142 -- its variadic form
//  auto-converts the AnsiString argument for %s, which is exactly the BCB6 idiom
//  golden :4054 relies on), MOT[].Gali_ReadPos/Gali_MotMove/Z1UpZ2Down/
//  Z1DownZ2Up/GailSpeed (Motor/mymotor.h:214/:201/:210/:211/:196), MTestY1/
//  MTestY2/MTestZ1/MTestZ2 (cmydef.h:2117 block), every Prod.Test* field used
//  (cprod.h:425/:426/:427/:428/:431/:432/:433/:436/:437/:438/:1015/:1016/:1018),
//  iBackUpZ1DownPosition/iBackUpZ2DownPosition (cmydef.h:3001 + peer),
//  bDoBRTCGiveWayCheck (cmydef.h:4911), bRTCFullViewError (cmydef.h:3340),
//  TestIF_File.bArm1PickPlaceArm2Test (cprod.h:2577), DeviceForm.ContactMode
//  (cprod.h:1244/:1161), TMoveDrop (MachineType.h:528), IndexStatus/Z1Up_Z2Down/
//  Z1Down_Z2Up (cmydef.h:2590/:51/:52), ShowMyMessage (canary_support.h:80),
//  DoGiveWayDelay (atester.h:53, object atester.cpp:115), and abs() (stdlib.h,
//  pulled in by MachineDefine.h:16 which this file includes first).
//------------------------------------------------------------------------------
//  INTEGER DIVISION PRESERVED
//    :4126  MOT[MTestZ2].GailSpeed*Prod.TestZ_Up_Speed/100  -- GailSpeed is int
//    (mymotor.h:196) and TestZ_Up_Speed is int (cprod.h:1016), so this is an
//    all-int expression whose /100 TRUNCATES.  Copied character for character;
//    no float helper, no reordering.
//------------------------------------------------------------------------------
//  GOLDEN BUGS PRESERVED, NOT FIXED
//    * :4203-4224 case 18 sends the vision commands only `if(DoGiveWayFrontDelay
//      .Off())`, but then advances `Task=20;` UNCONDITIONALLY, outside that if.
//      So when the delay is still running the SM reaches the case-20 wait having
//      sent NOTHING and with bHasSend still false -- it can only leave case 20
//      via the DoGiveWayDelay time-out arm (:4242), and DoGiveWayDelay was never
//      armed on that path either.  KEPT VERBATIM.
//    * :4082-4101 the else arm saves the live Prod teach values into function
//      statics and overwrites Prod with park values.  If case 2 is entered a
//      SECOND time before case 10/15 restores them, the statics are overwritten
//      with the ALREADY-CLOBBERED values and the real teach positions are lost
//      for the rest of the run.  KEPT VERBATIM.
//    * :4090 / :4100 iBackUpZ2DownPosition (resp. Z1) is set from Prod.TestZ2_Test
//      on the line AFTER Prod.TestZ2_Test was overwritten with Prod.TestZ2_Safe,
//      so the "backup down position" records the SAFE height.  KEPT VERBATIM.
//    * :4066 / :4209 `if(bCheckHasIC==1)` compares a bool parameter against the
//      int 1.  KEPT VERBATIM.
//    * :4032 the site-map is only built when iIndex==2, but the strSiteMap static
//      is sent for BOTH arms (:4054 runs unconditionally), so an iIndex==1 call
//      transmits whatever map the previous iIndex==2 call left behind (and "" on
//      the very first call).  KEPT VERBATIM.
//------------------------------------------------------------------------------
//  PEER GLOBAL -- golden :4002, OUTSIDE this part's assigned 4003..4251.
//  Emitted for the same reason as part 1's golden :3880: nothing in the port
//  defines it (python <scratch>/absence.py DoGiveWayFrontDelay -> HITS=0,
//  measured 2026-08-10 19:11:30 +0800) and the body below cannot compile without
//  it.  Same TRAP 4 clearance as part 1 (TQPF_Timer ctor touches only its own
//  members + QueryPerformanceCounter; myTimer.cpp:14-17).
//  Golden :4001 `int iBRTCGiveWayCheckTask=1;` is deliberately NOT emitted --
//  it already exists at atester_shims.cpp:197 / atester_shims.h:119.
//  INTEGRATE: if a sibling part also emits golden :4002, delete ONE of the two;
//  the block below is delimited for a single clean cut.
//  >>>>>> k6-BRTC-pair PEER GLOBAL golden:4002 BEGIN >>>>>>
TQPF_Timer DoGiveWayFrontDelay;
//  <<<<<< k6-BRTC-pair PEER GLOBAL golden:4002 END <<<<<<
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  TU-LOCAL COM2 RTC-VISION SEAM  (k6-BRTC-pair).  Guarded so the two part files
//  of this pair can both carry it and only the FIRST one stitched defines it.
//
//  WHY: the port's real COM2 is TCOM2Shim* (atester_shims.h:420-461, object
//  atester_shims.cpp:361 `TCOM2Shim *COM2 = new TCOM2Shim();`) and exposes only
//  bCCDDummyRum / DoReleaseAndInspEnd() / ATCAlarmSenCheck().  The two functions
//  in this pair additionally touch bRealTimeCom_ReceiveOK[] / rtTESTMAP /
//  rtCHECKIC / rtCHECKNULL / rtFullTOK / rtFullTNG / sRealTimeCom_Send[] /
//  SendCommToVision() / RTC_ResetAlarm() / InitRealTimeCCDPara().  This is the
//  SAME gap, with the SAME resolution, that the port already uses TWICE:
//  atester.cpp:5599-5618 (W7T1_TCOM2Ext) and atester_32Site.cpp:357-386
//  (W5_32S_TCOM2Ext).  Offline posture is identical to both: every RT channel
//  "never received" (false) so each golden `if(COM2->bRealTimeCom_ReceiveOK[..])`
//  arm falls to its OWN already-present time-out/retry else-branch; the three
//  vision commands no-op.  Nothing is fabricated.
//
//  TWO DELIBERATE DIFFERENCES FROM THOSE TWO PRECEDENTS -- both required here:
//   (1) NO `#define COM2`.  Those two files own their whole TU; this file is
//       STITCHED FROM PART FILES WRITTEN BY PARALLEL AGENTS, so a TU-wide
//       `#define COM2` would silently hijack a sibling's COM2 uses (and two
//       siblings each emitting one would make "which seam wins" depend on stitch
//       order).  Every golden `COM2->` in this pair is therefore spelled
//       `K6BRTC_COM2->`; the integrate step is one mechanical rename back to
//       `COM2` once TCOM2Shim carries the members.  This pair names the bare
//       identifier `COM2` ZERO times, so it is immune to any sibling's #define.
//   (2) CONSTANT-INITIALISED, NO CONSTRUCTOR (TRAP 4).  Those two precedents use
//       a real ctor that runs a loop before main().  This one is an aggregate
//       with no user-declared ctor and no non-trivially-constructible member, so
//       it is CONSTANT-initialised: zero code runs before main().  The one
//       AnsiString store golden needs (sRealTimeCom_Send[]) lives in a
//       FUNCTION-LOCAL static inside the proxy's operator[], i.e. constructed on
//       first CALL, never during static init.
//------------------------------------------------------------------------------
#ifndef K6BRTC_COM2_SEAM
#define K6BRTC_COM2_SEAM
enum { K6BRTC_rtTESTMAP=0,  K6BRTC_rtCHECKIC=1, K6BRTC_rtCHECKNULL=2,
       K6BRTC_rtFullTOK=3,  K6BRTC_rtFullTNG=4, K6BRTC_RT_N=16 };

// Stand-in for golden TCOM2::sRealTimeCom_Send[] (golden rs232.h).  operator[]
// hands back a real, assignable AnsiString so golden :4054's store is KEPT, not
// dropped -- but the storage is a function-local static, so it is created on the
// first call and never by static init (TRAP 4).
struct K6BRTC_RTSendProxy
{
    AnsiString& operator[](int ch)
    {
        static AnsiString buf[K6BRTC_RT_N];
        return buf[(ch>=0 && ch<K6BRTC_RT_N) ? ch : 0];
    }
};

struct K6BRTC_TCOM2Ext
{
    bool               bRealTimeCom_ReceiveOK[K6BRTC_RT_N];      // golden rs232.h -- offline all false ("vision never responds")
    int                rtTESTMAP, rtCHECKIC, rtCHECKNULL,        // golden rs232.h -- RT channel index ids
                       rtFullTOK, rtFullTNG;
    K6BRTC_RTSendProxy sRealTimeCom_Send;                        // golden rs232.h -- see proxy above
    void SendCommToVision(int /*ch*/, bool /*b*/){}              // golden rs232.h -- offline no-op (== both precedents)
    void RTC_ResetAlarm(){}                                      // golden rs232.h -- offline no-op (== atester_32Site.cpp:370)
    void InitRealTimeCCDPara(){}                                 // golden rs232.h -- offline no-op (== atester_32Site.cpp:371)
    // DoReleaseAndInspEnd IS a real member of the real TCOM2Shim -- routed here
    // to a no-op only so this pair never names the bare identifier `COM2` (see
    // difference (1) above).  PROVABLY equivalent today, not an approximation:
    // TCOM2Shim::DoReleaseAndInspEnd's real body is `{}` (atester_shims.cpp:335,
    // read 2026-08-10 19:05 +0800).  Registered as GATE G-k6-SEAM so the day that
    // body stops being empty this detour is found, not silently kept.
    void DoReleaseAndInspEnd(){}                                 // golden rs232.h:160
};
static K6BRTC_TCOM2Ext k6brtc_com2_ext =
{
    {false, false, false, false, false, false, false, false,
     false, false, false, false, false, false, false, false},
    K6BRTC_rtTESTMAP, K6BRTC_rtCHECKIC, K6BRTC_rtCHECKNULL,
    K6BRTC_rtFullTOK, K6BRTC_rtFullTNG,
    {}                                                           // the empty send-proxy (explicit, so -Wextra stays quiet)
};
#define K6BRTC_COM2 (&k6brtc_com2_ext)
#endif // K6BRTC_COM2_SEAM
//------------------------------------------------------------------------------
// ---- golden aTester_Rear.cpp:4003-4251 -- DoBRTCGiveWayCheck ----------------
// NOT `static`: golden gives it external linkage and its FOUR call sites are
// golden aTester_Rear.cpp:8293/:8297/:8315/:8324 and :8783/:8787/:8798/:8807 --
// i.e. inside DoTestYRear, which a SIBLING AGENT is translating this same wave.
// Golden order puts this definition (4003) before those calls (8293+), so the
// stitched file needs no forward declaration.
bool DoBRTCGiveWayCheck(bool bInitial,bool bCheckHasIC,int iIndex)
{
    static int iTestY1_Middle=0;                                                //Ifor 20190719 : add 加快讓位移動速度
    static int iTestZ1_Test=0;
    static int iTestZ1_Drop=0;
    static int iTestY2_Middle=0;
    static int iTestZ2_Test=0;
    static int iTestZ2_Drop=0;
    static bool bHasSend=false;
    static AnsiString strSiteMap="";

    if(bInitial==true)
    {
        iBRTCGiveWayCheckTask=1;
        return false;
    }

    int iY2Pos=0;
    int &Task=iBRTCGiveWayCheckTask;
    long lZ2pos=0;
    long lY2pos=0;
    long lZ1pos=0;
    long lY1pos=0;
    AnsiString str="";

    switch(Task)
    {
        case 1:
            bHasSend=false;
            if(iIndex==2 && bCheckHasIC==true)
            {
                strSiteMap="";
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_IC ||
                           FTestSuck.Item[i][j]==HAS_HOT_IC)
                        {
                            strSiteMap=strSiteMap+"1";
                        }
                        else
                        {
                            strSiteMap=strSiteMap+"0";
                        }
                    }

                    if(i<FTestSuck.iShtRow-1)
                        strSiteMap=strSiteMap+",";
                }
            }
            K6BRTC_COM2->sRealTimeCom_Send[K6BRTC_COM2->rtTESTMAP] = str.sprintf("@TESTSITE:%s+" , strSiteMap);                       //Ifor 20200522 fix:RTC2.0 命令變更
            Task=2;
        case 2:
            lZ1pos=MOT[MTestZ1].Gali_ReadPos();
            lY1pos=MOT[MTestY1].Gali_ReadPos();
            lZ2pos=MOT[MTestZ2].Gali_ReadPos();
            lY2pos=MOT[MTestY2].Gali_ReadPos();
            if(lZ2pos>=0 && lY2pos==Prod.TestY2_Rear &&
               lZ1pos>=0 && lY1pos==Prod.TestY1_Front)
            {
                K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTOK]=false;
                K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTNG]=false;
                if(bCheckHasIC==1)
                {
                    K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtTESTMAP, true);
                    K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtCHECKIC, true);
                    K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtFullTOK, true);
                }
                else
                {
                    K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtCHECKNULL, true);
                    K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtFullTOK, true);
                }
                DoGiveWayDelay.SetSecAndOn(10);
                Task=20;
            }
            else
            {
                if(iIndex==2)
                {
                    iTestY2_Middle            =Prod.TestY2_Middle;
                    iTestZ2_Test              =Prod.TestZ2_Test;
                    iTestZ2_Drop              =Prod.TestZ2_Drop_Offset;
                    Prod.TestY2_Middle        =Prod.TestY2_Rear;
                    Prod.TestZ2_Test          =Prod.TestZ2_Safe;
                    Prod.TestZ2_Drop_Offset   =0;
                    iBackUpZ2DownPosition     =Prod.TestZ2_Test;
                }
                else
                {
                    iTestY1_Middle            =Prod.TestY1_Middle;
                    iTestZ1_Test              =Prod.TestZ1_Test;
                    iTestZ1_Drop              =Prod.TestZ1_Drop_Offset;
                    Prod.TestY1_Middle        =Prod.TestY1_Front;
                    Prod.TestZ1_Test          =Prod.TestZ1_Safe;
                    Prod.TestZ1_Drop_Offset   =0;
                    iBackUpZ1DownPosition     =Prod.TestZ1_Test;
                }

                DoGiveWayFrontDelay.SetMSAndOn(0);                              //50 ms 5.9~6.0 Decode
                Task=10;
                bDoBRTCGiveWayCheck=true;
            }

            if(Task!=10)
            {
                break;
            }
        case 10:
            bDoBRTCGiveWayCheck=true;
            if(TestIF_File.bArm1PickPlaceArm2Test &&
               DeviceForm.ContactMode==TMoveDrop)                               //二段速上升
            {
                if(iIndex==2)
                {
                    Task=15;
                }
                else
                {
                    iY2Pos=MOT[MTestY2].Gali_ReadPos();
                    if(abs(iY2Pos-Prod.TestY2_Middle)<30)
                    {
                        if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+Prod.TestZ2_Up_Offset, MOT[MTestZ2].GailSpeed*Prod.TestZ_Up_Speed/100))
                        {
                            DoGiveWayFrontDelay.SetSecAndOn(Prod.TestZ_Up_Wait);
                            Task=12;
                        }
                    }
                }
            }
            else
            {
                if(iIndex==2)
                {
                    if(MOT[MTestY1].Z1UpZ2Down(MOT[MTestZ2].GailSpeed, true, false))
                    {
                        Prod.TestY2_Middle      =iTestY2_Middle;
                        Prod.TestZ2_Test        =iTestZ2_Test;
                        Prod.TestZ2_Drop_Offset =iTestZ2_Drop;
                        iBackUpZ2DownPosition   =Prod.TestZ2_Test;
                        IndexStatus=Z1Down_Z2Up;
                        Task=18;
                    }
                }
                else
                {
                    if(MOT[MTestY1].Z1DownZ2Up(MOT[MTestZ1].GailSpeed, true, false))
                    {
                        Prod.TestY1_Middle      =iTestY1_Middle;
                        Prod.TestZ1_Test        =iTestZ1_Test;
                        Prod.TestZ1_Drop_Offset =iTestZ1_Drop;
                        iBackUpZ1DownPosition   =Prod.TestZ1_Test;
                        IndexStatus=Z1Up_Z2Down;
                        Task=18;
                    }
                }
            }

            if(Task!=12)
            {
                break;
            }
        case 12:
            if(DoGiveWayFrontDelay.Off())
            {
                Task=15;
            }

            if(Task!=15)
            {
                break;
            }
        case 15:
            if(iIndex==2)
            {
                if(MOT[MTestY1].Z1UpZ2Down(MOT[MTestZ2].GailSpeed, true, false))
                {
                    Prod.TestY2_Middle      =iTestY2_Middle;
                    Prod.TestZ2_Test        =iTestZ2_Test;
                    Prod.TestZ2_Drop_Offset =iTestZ2_Drop;
                    iBackUpZ2DownPosition   =Prod.TestZ2_Test;
                    IndexStatus=Z1Down_Z2Up;
                    Task=18;
                }
            }
            else
            {
                if(MOT[MTestY1].Z1DownZ2Up(MOT[MTestZ1].GailSpeed, true, false))
                {
                    Prod.TestY1_Middle      =iTestY1_Middle;
                    Prod.TestZ1_Test        =iTestZ1_Test;
                    Prod.TestZ1_Drop_Offset =iTestZ1_Drop;
                    iBackUpZ1DownPosition   =Prod.TestZ1_Test;
                    IndexStatus=Z1Up_Z2Down;
                    Task=18;
                }
            }
            break;
        case 18:
            if(bHasSend==false)
            {
                if(DoGiveWayFrontDelay.Off())
                {
                    K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTOK]=false;
                    K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTNG]=false;
                    if(bCheckHasIC==1)
                    {
                        K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtTESTMAP, true);
                        K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtCHECKIC, true);
                        K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtFullTOK, true);
                    }
                    else
                    {
                        K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtCHECKNULL, true);
                        K6BRTC_COM2->SendCommToVision(K6BRTC_COM2->rtFullTOK, true);
                    }
                    DoGiveWayDelay.SetSecAndOn(10);
                    bHasSend=true;
                }
            }
            Task=20;
            break;
        case 20:
            if(K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTOK])
            {
                K6BRTC_COM2->DoReleaseAndInspEnd();                                    //JerryYang 20260506 : Add
                bRTCFullViewError=false;                                        //Steven 20120206 : RTC重複錯誤
                iBRTCGiveWayCheckTask=1;
                bDoBRTCGiveWayCheck=false;
                return true;
            }
            else if(K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTNG])
            {
                ShowMyMessage("RTC Full View NG");
                K6BRTC_COM2->DoReleaseAndInspEnd();
                K6BRTC_COM2->bRealTimeCom_ReceiveOK[K6BRTC_COM2->rtFullTNG]=false;
                Task=1;
            }
            else if(DoGiveWayDelay.Off())
            {
                K6BRTC_COM2->DoReleaseAndInspEnd();
                ShowMyMessage("RTC Release Time out of Give Way Check");
                Task=1;
            }
            break;
    }
    return false;
}

//==============================================================================
//  PART FILE  k5-DoBRTCAutoModelVerify   --  golden aTester_Rear.cpp :4255-5089
//  Translator: AI(k5-DoBRTCAutoModelVerify) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp
//                 (9,787 lines, cp950, 100% CRLF).  This part covers EXACTLY
//                 golden :4255-5089 (835 golden lines) and nothing else.
//
//  ROLE
//  ----
//  DoBRTCAutoModelVerify(bool bInitial) -- the REAR (Arm2 / Z2 / BTestSuck)
//  RTC "auto model verify" state machine.  With the real-time CCD (RTC/DTK
//  vision) wired, it teaches the camera a new device model without an operator:
//  park Z2 safe -> split Y to Front/Rear -> push the site map to the RTC and
//  run the all-pass verify -> step Z2 down in +10-count release increments
//  releasing/re-gripping the ICs in the socket so the camera sees them seated
//  (12020..12220, with a drop-detect + JAM0304 arm at 12050/12055) -> re-suck
//  and negative-pressure re-check (12300..12390) -> the OPEN-LIVE / RELEASE /
//  INSP-END / OPEN-VERIFY / ALL-FAIL-VERIFY / CLOSE-VERIFY vision handshake
//  (12400..13700) -> pick the ICs back off the socket (13500/13800, JAM0302 on
//  pick-up error) -> restore EP pressure and re-send the real site map
//  (13900..15200), then hand the machine back as IndexStatus=Z1Up_Z2Down.
//
//  WAVE SCOPE
//  ----------
//    * DoBRTCAutoModelVerify(bool bInitial)   golden :4255-5089   ACTIVE
//      (no #if 0 anywhere in the body -- see GATE REGISTER for why every gap
//      resolved to an ACTIVE seam or an extern hand-off instead)
//
//  GATE REGISTER  (see the per-entry command + timestamp; TRAP 2)
//  ------------------------------------------------------------------------
//  This part emits ZERO `#if 0` blocks.  Four golden symbols were nevertheless
//  absent from the port and are registered here in full, because each was
//  resolved by a NAMED seam / extern hand-off rather than by a gate, and the
//  main loop must re-verify all four at integration exactly as if they were
//  gates.  Nothing golden calls was silently dropped.
//
//  [K5-G1]  COM2 real-time-vision surface -- bRealTimeCom_ReceiveOK[],
//           rtSiteMap / rtOpenLive / rtCloseLive / rtRelease / rtInspEnd /
//           rtInspStart / rtOPENVERIFYOK / rtOPENVERIFYNG / rtALLFAILOK /
//           rtALLFAILNG / rtCLOSEVERIFY, SendCommToVision(int,bool),
//           OpenRTCComPortAgain(), InitRealTimeCCDPara()
//           GOLDEN LINES (all 48 `COM2` lines in :4255-5089, enumerated, not
//           paraphrased): 4313 4320 4322 4650 4651 4662 4668 4670 4682 4683
//             4688 4694 4696 4701 4702 4703 4708 4709 4710 4720 4722 4732 4733
//             4738 4742 4749 4751 4756 4757 4762 4775 4782 4784 4789 4790 4791
//             4796 4797 4798 4808 4862 4867 4873 4875 5055 5062 5064 5077
//           PARTIAL-GAP NOTE (be precise): ONE of the members golden uses here,
//             DoReleaseAndInspEnd(), DOES exist on the real TCOM2Shim
//             (atester_shims.h, declared beside bCCDDummyRum) -- so lines 4322
//             4670 4696 4703 4722 4751 4784 4791 4875 5064 would compile
//             against the real object unchanged.  The seam re-declares it as an
//             identical no-op only so that the rebinding of `COM2` is total and
//             no line silently splits across two different objects.  Every
//             OTHER member above is genuinely absent.
//           WHY THE OFFLINE DEFAULT IS FAITHFUL: golden's own no-camera state.
//             bRealTimeCom_ReceiveOK[] all false == "the vision box has not
//             answered yet", which is precisely what golden observes on a
//             handler with no RTC camera on the port; every wait case then
//             falls into its ALREADY-PRESENT `else if(DoTestYRearDelay.Off())`
//             time-out/retry arm, which is golden's own real behaviour.
//             OpenRTCComPortAgain()==false is golden's "re-open did not
//             succeed" value, which SUPPRESSES the ShowMyMessage popup and
//             keeps the retry silent -- the conservative arm.  Nothing
//             fabricates a verify PASS or a verify FAIL.
//           REAL-MACHINE DIFFERENCE: on a machine WITH the RTC camera the
//             handshake completes and the SM walks 150->160->200->...->15200
//             and returns true.  Offline it cannot get that far at all -- and
//             not because of this seam: case 200 gates on DoAllPassVerifyRTC(),
//             whose LIVE port body is `return false;` (atester.cpp:8596-8602,
//             itself gated as G-PTk4-DoAllPassVerifyRTC).  So offline the SM
//             PARKS AT case 200 and the whole COM2 surface below is
//             unreachable -- the same posture atester_32Site.cpp's
//             Do32RTCAutoModelVerify is already proven to have
//             (tests/test_atester_32site.cpp:161-166).
//           ABSENCE COMMAND (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//             rg -n 'bRealTimeCom_ReceiveOK|SendCommToVision|OpenRTCComPortAgain|InitRealTimeCCDPara|rtSiteMap' atester_shims.h
//               -> NO MATCH (exit 1): none of these is a member of TCOM2Shim
//                  (atester_shims.h:420-460, the class `extern TCOM2Shim *COM2`
//                  points at).  TCOM2Shim has exactly bCCDDummyRum,
//                  DoReleaseAndInspEnd(), ATCAlarmSenCheck().
//             rg -n 'bRealTimeCom_ReceiveOK' -g '*.cpp' -g '*.h' . | rg -v 'atester_32Site.cpp|atester\.cpp|csystem\.cpp'
//               -> NO MATCH outside those three files, and inside them every
//                  hit binds to that file's OWN TU-local seam + `#define COM2`
//                  (atester.cpp:5599-5618 W7T1_TCOM2Ext;
//                  atester_32Site.cpp:358-386 W5_32S_TCOM2Ext).  There is NO
//                  shared home to reach for.
//             MEASURED AT: 2026-08-10 19:09:09 +0800 (re-measured 19:05:36 and
//                          19:10:30 with the same result)
//           WHY IT SHOULD *STAY* SEAMED, not be "fixed" by taking a dependency
//           (TRAP 3): the real home is golden rs232.h/rs232.cpp (TCOM2), an
//           untranslated serial/vision subsystem.  Widening the SHARED
//           TCOM2Shim from a part file is out of this task's boundary anyway,
//           and doing it for this one function would drag the RTC protocol into
//           every one of the ~180 TUs that see atester_shims.h.  The seam is
//           the same decision atester.cpp and atester_32Site.cpp already made
//           for the identical member family.
//
//  [K5-G2]  iHangupCTArm2 -- golden aTester_Rear.cpp:76 `int iHangupCTArm2=0;`
//           GOLDEN LINE USED HERE: :4857 (`iHangupCTArm2=0;` in case 13500)
//           RESOLUTION: NOT gated.  This is not a foreign symbol -- it is
//             golden's OWN file-scope counter in the very file being
//             translated, and its defining line (:76) simply falls OUTSIDE this
//             part's assigned :4255-5089 range.  Dropping or gating the reset
//             would be a real behaviour change: golden :7662-7665 does
//             `iHangupCTArm2++; if(iHangupCTArm2>1){ iHangupCTArm2=0; ...}`, so
//             failing to zero it here would make the hang-up detector fire one
//             cycle early after every auto-verify.  Emitted instead as a plain
//             `extern int iHangupCTArm2;` declaration (below) so the write
//             stays ACTIVE and a missing definition is a LOUD link error rather
//             than a silent loss.
//           HAND-OFF (main loop): add golden :76 `int iHangupCTArm2=0;` to
//             aTester_Rear.cpp EXACTLY ONCE.  The sibling translating golden
//             :5307/:5339/:7662 needs the same object -- do not let two parts
//             each define it.
//           ABSENCE COMMAND: rg -n -w 'iHangupCTArm2' -g '*.cpp' -g '*.h' .
//             -> NO MATCH (exit 1) anywhere in the port tree, tests included.
//           MEASURED AT: 2026-08-10 19:09:09 +0800
//
//  [K5-G3]  DoTestYRearDelay -- golden aTester_Rear.cpp:4253
//           `HTimer DoTestYRearDelay, DoTestYRearDelay2;`
//           GOLDEN LINES USED HERE: :4308 :4318 :4475 :4481 :4496 :4501 :4510
//             :4516 :4546 :4576 :4652 :4666 :4684 :4692 :4704 :4715 :4734
//             :4747 :4758 :4780 :4792 :4803 :4863 :4871 :4950 :4955 :5050 :5060
//           RESOLUTION: NOT gated -- golden :4253 is one line above this part's
//             range, so it is the main loop's line to emit.  Declared here as
//             `extern TQPF_Timer DoTestYRearDelay;` (below).
//           *** TRAP-5-SHAPED HAZARD, READ BEFORE EMITTING :4253 ***
//             The token `HTimer` resolves to TWO DIFFERENT TYPES in this tree:
//               (a) atester_shims.h:463  `struct HTimer { bool Off(){ return
//                   true; } void SetSecAndOn(double){} };`  <-- Off() is
//                   HARD-CODED true, and atester_shims.h IS included by
//                   aTester_Rear.cpp (line 151), so this is what a literal
//                   `HTimer DoTestYRearDelay;` would pick;
//               (b) the tree-wide golden-HTimer convention `typedef TQPF_Timer
//                   HTimer;` (acatchtray.cpp:114, and the extern TQPF_Timer
//                   form at aoutarm9045_1x1_1.cpp:250, aoutarm9045_1x2_2.cpp
//                   :181, aoutarm9045_1x3_4.cpp:137, ...).
//             Choosing (a) LINKS AND COMPILES PERFECTLY and then makes every
//             `if(DoTestYRearDelay.Off())` in this function fire on the FIRST
//             pass -- i.e. every 10-second RTC wait would time out instantly
//             and every 0.3s/1s/3s settle delay would be skipped, silently
//             turning the release/re-grip stepping at 12100/12200/12210 into a
//             no-dwell loop.  aTester_Rear.cpp already uses (b) for its own
//             timers (`TQPF_Timer BTempRiseHasICDelay;` :301,
//             `TQPF_Timer CheckSocketHasICDelay2;` :380) -- keep that.
//           HAND-OFF (main loop): emit golden :4253 as
//             `TQPF_Timer DoTestYRearDelay, DoTestYRearDelay2;`.
//             DoTestYRearDelay2 is NOT touched by this part (it belongs to
//             DoTestYRear, golden :5100+) -- do not add it here.
//           ABSENCE COMMAND: rg -n -w 'DoTestYRearDelay|DoTestYRearDelay2' -g '*.cpp' -g '*.h' .
//             -> NO MATCH (exit 1).
//           MEASURED AT: 2026-08-10 19:09:09 +0800
//
//  [K5-G4]  CONTACT_NORMAL -- golden cContact.cpp:74 `const int CONTACT_NORMAL =0;`
//           GOLDEN LINES: :5035 :5079 (`iContactMode!=CONTACT_NORMAL`)
//           RESOLUTION: NOT gated -- substituted for the TU-local
//             `static const int K5_CONTACT_NORMAL = 0;` (below).  The value is
//             golden's EXACT value re-read from golden cContact.cpp:74 this
//             wave, not an offline approximation, so both call sites keep
//             golden's control flow bit-for-bit.
//           WHY IT SHOULD STAY LOCAL (TRAP 3 -- the first premise I wrote here
//             DIED and the answer is still "do not reach for it"): a real,
//             translated CONTACT_NORMAL DOES exist at cContact.h:81, and
//             MachineDefine.h:137 does `#include "cContact.h"` -- so a naive
//             "absent from the port" claim would be FALSE.  It is still
//             unreachable from THIS translation unit for a different, verified
//             reason: MachineDefine.h:137 sits INSIDE the `#if 0` opened at
//             MachineDefine.h:34 and closed at :146 (preprocessor-depth map
//             run this wave), so cContact.h is never actually included by
//             aTester_Rear.cpp.  And cContact.h's own banner (:66-80) warns
//             that its CONTACT_NORMAL is a plain internal-linkage `const int`
//             that collides by redefinition with the second copy at
//             BarCode/BarCode_Shuttle2_CCDScan.h:187 -- so pulling that header
//             into this TU from a part file is exactly the ODR trap it warns
//             about.  A one-token local const with golden's own value is the
//             lower-risk faithful choice.
//           ABSENCE COMMAND:
//             rg -n 'CONTACT_NORMAL' cmydef.h atester.h atester_shims.h MachineType.h cprod.h CosFunction.h Config.h canary_support.h FormsFacade.h aHotPlateSubstrate.h csystem.h
//               -> NO MATCH (exit 1) in any header aTester_Rear.cpp includes
//                  directly, and the only transitive path (MachineDefine.h:137)
//                  is inside `#if 0`.
//           MEASURED AT: 2026-08-10 19:10:30 +0800
//
//  NOT A GATE, BUT THE MAIN LOOP MUST ACT (integration debts)
//  ---------------------------------------------------------
//    * atester_shims.h:145 declares AND atester_shims.cpp:221 DEFINES
//      `bool DoBRTCAutoModelVerify(bool){ return true; }`.  That no-op stub
//      must be REMOVED when this part lands or the link gets a duplicate
//      definition.  Note the stub returns TRUE -- i.e. today every caller is
//      told the auto-verify already SUCCEEDED.  This part's real body returns
//      true only from case 15200, so retiring the stub is a genuine behaviour
//      change (offline the SM will now park at case 200 instead of reporting
//      "verified").  That is the correct, golden behaviour; flag it, do not
//      "fix" it.  (TRAP 1 shape (b): while that stub exists, this part's
//      object file is never pulled out of the archive and a green build proves
//      NOTHING about it being linked.)
//    * iBRTCAutoModelVerifyTask: golden :4254 is `int iBRTCAutoModelVerifyTask=1;`
//      and is OUTSIDE this part's range, so this part does NOT define it.  It is
//      already declared atester_shims.h:120 and defined atester_shims.cpp:197
//      (=1, the same initial value), so `int &Task=iBRTCAutoModelVerifyTask;`
//      binds today.  If the main loop emits golden :4254 into aTester_Rear.cpp,
//      it MUST delete atester_shims.cpp:197 in the same step.
//    * This part's body is NOT `static` and its signature matches the
//      atester_shims.h:145 declaration exactly -- deliberately, to avoid
//      TRAP 1 shape (d) (an internal-linkage shadow that links clean and then
//      explodes once the real declaration is visible in the same TU; it IS
//      visible here, aTester_Rear.cpp:151 includes atester_shims.h).
//    * STITCH ORDER REQUIREMENT: this part calls
//      W64bT2_ProcessIndexSuckDestroy2() (aTester_Rear.cpp:367, a pre-existing
//      TU-local static with a defaulted arg).  This part must be stitched
//      AFTER line 367.  Appending at end-of-file satisfies that.
//    * OBSERVED WHILE CHECKING, not used by this part: TMyKitSuck::MoveSuckData
//      now EXISTS (aHotPlateSubstrate.h:598) and MoveSuckDataDiff at :572, so
//      this file's own Wave-1/Wave-2 banner claims that "MoveSuckData is
//      absent" (aTester_Rear.cpp:40, :74) are EXPIRED absence-claims and the
//      W64bT2_MoveSuckData no-op at :325 is now silently dropping data that a
//      real method could carry.  Not touched here (out of range), reported.
//
//  TRAP 4 COMPLIANCE
//  -----------------
//  This part adds exactly ONE file-scope object, `k5_com2_ext`, and it is an
//  AGGREGATE with NO user-declared constructor, initialised entirely from
//  integral/boolean constant expressions -> constant (static) initialisation,
//  no dynamic initialiser runs before main() at all.  It touches no global, no
//  pointer, nothing outside its own storage.  (Both existing precedents,
//  atester.cpp:5614 W7T1_com2_ext and atester_32Site.cpp:383 W5_32S_com2_ext,
//  DO have running constructors; this one deliberately does not.)  No other
//  object, no `new`, no initialiser that calls anything.
//
//  TRAP 5 COMPLIANCE -- which header this part relied on
//  ----------------------------------------------------
//  BTestSuck (TMyKitSuck) and BTestSuck.Suck[][] (TMySucker): this part relies
//  on aHotPlateSubstrate.h -- class TMyKitSuck at aHotPlateSubstrate.h:365,
//  TMySucker members Enable :140 / SenUsing :117 / Error :109 / Suck() :119 /
//  On() :121 / Off() :122 / Normal() :130 / GetStatus() :131, and
//  TMyKitSuck::SetItemData :497 -- because aHotPlateSubstrate.h is what
//  aTester_Rear.cpp:164 includes.  The OBJECT this binds to is defined in
//  aHotPlateSubstrate.cpp:93 (`TMyKitSuck BTestSuck;`).  The decoy is
//  mykitsuck.h:274 (a SECOND `class TMyKitSuck`, different layout) with its own
//  second definition of the same-named global at mykitsuck.cpp:212 -- picking
//  that one would link and read every field at the wrong offset.  Not used
//  here.  fiosetview->bIndexSuck[1][i][j] (golden :4537) is
//  TfiosetviewShim::bIndexSuck[2][4][8], atester_shims.h:407 -- [1] is the rear
//  arm and is in range.
//
//  TRANSCRIPTION RULES OBSERVED
//  ----------------------------
//    * Golden :4255-5089 transcribed CHARACTER FOR CHARACTER (cp950 -> UTF-8
//      only).  Chinese comments preserved verbatim, never reworded, never
//      translated.  ZERO U+FFFD.
//    * Statement order, cursor values, formulas and magic numbers VERBATIM,
//      including `iSpeedSlow/3` at golden :4837 -- INT/INT, left as integer
//      division on purpose (a float helper here would drift the Z2 pick
//      height).  `+750` :4355, `+100` :4391/:4398, `+10` :4837, `+1000` :4522,
//      `iReleaseCT+=10` / `<100` :4503-4504, `50000` :4405, `4.0` :4646, all
//      as golden wrote them.
//    * GOLDEN FALL-THROUGH PRESERVED: `case 12310:` ends with `Task=12320;` and
//      NO `break;` (golden :4548), falling straight into `case 12320:` -- so on
//      the pass that turns the vacuum back on it ALSO runs the 12320 body
//      immediately.  Kept exactly; do not "fix" it.
//    * GOLDEN QUIRK KEPT, NOT FIXED (golden :4520 / :4636-4641): case 12220
//      moves to 12300 which drives Z2 to TestZ2_Test+1000, and case 12390 then
//      re-issues the SAME `Gali_MotMove(Prod.TestZ2_Safe, iSpeedSlow)` that
//      case 12320 already completed at :4560 -- a redundant second safe move.
//      Reproduced verbatim.
//    * GOLDEN QUIRK KEPT (golden :4623): case 12340 re-tests `if(bHasErr)`,
//      but case 12330 only routes to 12340 when bHasErr is ALREADY true
//      (:4586 sets it, :4600-4602 branches on it), so the test can never be
//      false and the no-error path through the `break;` at :4635 is
//      unreachable.  There is no else arm to drop -- reproduced verbatim.
//    * GOLDEN QUIRK KEPT (golden :4732-4733 and :4756-4757): case 12900 zeroes
//      bRealTimeCom_ReceiveOK[rtOPENVERIFYOK] (:4732) but then SENDS
//      rtOPENVERIFYNG (:4733); case 13100 zeroes [rtALLFAILNG] (:4756) but
//      SENDS rtALLFAILOK (:4757).  The cleared channel and the sent channel are
//      crossed in golden both times.  Reproduced verbatim.
//    * GOLDEN QUIRK KEPT (golden :4475): the `DoTestYRearDelay.SetSecAndOn(0.3)`
//      inside case 12100 sits INSIDE the i/j double loop, so it is re-armed
//      once per site instead of once per case.  Reproduced verbatim.
//    * GOLDEN QUIRK KEPT (golden :5081): the `if(...)` arm of case 15200's last
//      test is an EMPTY block `{ }` and the real work is in the `else`.
//      Reproduced verbatim.
//    * GOLDEN QUIRK KEPT (golden :4805-4809): case 13220's bRTCRetry==true arm
//      re-opens the RTC port and shows the time-out message but, unlike its
//      twin at :4717-4722, does NOT call COM2->DoReleaseAndInspEnd() before
//      going back to 13210.  Reproduced verbatim.
//    * Only FOUR tokens differ from golden text in the whole body, each marked
//      inline with an `AI(k5-DoBRTCAutoModelVerify) 20260810:` note and each
//      registered above: :4553 and :4569 (ProcessIndexSuckDestroy2 -> the
//      existing W64bT2_ stub) and :5035 and :5079 (CONTACT_NORMAL ->
//      K5_CONTACT_NORMAL).  `COM2` itself is textually UNCHANGED -- it is
//      rebound by the scoped macro below.
//==============================================================================

// ===========================================================================
//  K5_SEAM -- everything this part needs that the port does not have.  Scoped
//  as tightly as possible because sibling agents are writing other parts of
//  this same eventual translation unit:
//    * unique `K5_`/`k5_` prefix on every new name -> no name collision with a
//      sibling part;
//    * the `COM2` rebinding is #undef'd again immediately after this part's
//      function, so it CANNOT leak into a sibling's text (unlike the
//      file-scope-for-the-rest-of-the-TU form used by atester.cpp:5618 and
//      atester_32Site.cpp:386);
//    * the two externs are DECLARATIONS ONLY -- a duplicate identical extern
//      from a sibling is legal, a duplicate DEFINITION would not be.
//  TODO(integrate): converge with the sibling parts' COM2 seams into ONE, then
//  delete this block; the call sites bind unchanged.
// ===========================================================================
#ifndef K5_SEAM
#define K5_SEAM

// -- golden aTester_Rear.cpp:76 `int iHangupCTArm2=0;` -- see GATE [K5-G2].
//    Definition owed by the main loop (once, for the whole file).
extern int iHangupCTArm2;

// -- golden aTester_Rear.cpp:4253 `HTimer DoTestYRearDelay, DoTestYRearDelay2;`
//    -- see GATE [K5-G3].  Definition owed by the main loop; the type MUST be
//    TQPF_Timer, NOT atester_shims.h:463's `struct HTimer` (whose Off() is
//    hard-coded true).
extern TQPF_Timer DoTestYRearDelay;

// -- golden cContact.cpp:74 `const int CONTACT_NORMAL =0;` -- see GATE [K5-G4].
static const int K5_CONTACT_NORMAL = 0;

// -- COM2 real-time-vision surface -- see GATE [K5-G1].  Aggregate, NO
//    constructor (TRAP 4): constant-initialised, nothing runs before main().
//    Channel ids are golden's rs232.h enum members; their NUMERIC values are
//    private to the seam (golden only ever uses them as indices into
//    bRealTimeCom_ReceiveOK[], never compares them to a literal), so any
//    distinct set is faithful -- what matters is that they are distinct and in
//    range, which the K5_RT_N bound guarantees.
enum { K5_rtSiteMap=0, K5_rtOpenLive=1, K5_rtCloseLive=2, K5_rtRelease=3,
       K5_rtInspEnd=4, K5_rtInspStart=5, K5_rtOPENVERIFYOK=6, K5_rtOPENVERIFYNG=7,
       K5_rtALLFAILOK=8, K5_rtALLFAILNG=9, K5_rtCLOSEVERIFY=10, K5_RT_N=16 };
struct K5_TCOM2Ext
{
    bool bRealTimeCom_ReceiveOK[K5_RT_N];                   // golden rs232.h -- offline all false ("the vision box never answered")
    int  rtSiteMap, rtOpenLive, rtCloseLive, rtRelease, rtInspEnd, rtInspStart,
         rtOPENVERIFYOK, rtOPENVERIFYNG, rtALLFAILOK, rtALLFAILNG, rtCLOSEVERIFY;
    void DoReleaseAndInspEnd() {}                           // golden rs232.h:160 -- offline no-op (same as the real TCOM2Shim's, atester_shims.h)
    void SendCommToVision(int /*iCh*/, bool /*bFlag*/) {}    // golden rs232.h -- offline no-op
    bool OpenRTCComPortAgain() { return false; }             // golden rs232.h -- offline: re-open did not succeed (suppresses the popup, keeps the retry silent)
    void InitRealTimeCCDPara() {}                            // golden rs232.h -- offline no-op
};
static K5_TCOM2Ext k5_com2_ext =
{
    { false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false },
    K5_rtSiteMap, K5_rtOpenLive, K5_rtCloseLive, K5_rtRelease, K5_rtInspEnd,
    K5_rtInspStart, K5_rtOPENVERIFYOK, K5_rtOPENVERIFYNG, K5_rtALLFAILOK,
    K5_rtALLFAILNG, K5_rtCLOSEVERIFY
};

#endif // K5_SEAM

// Rebind COM2 for THIS PART ONLY (matching #undef right after the function).
#ifdef COM2
#undef COM2
#endif
#define COM2 (&k5_com2_ext)
//------------------------------------------------------------------------------

bool DoBRTCAutoModelVerify(bool bInitial)
{
    static int iReleaseCT=0;
    static bool bVerifyNG=false;
    static bool bHasErr=false, bIndexSuckCheck=false;
    static bool bBTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bBTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};
    static bool bRTCRetry=false;                                                //Ifor 20251023 add:RTC無回應Retry 一次
    if(bInitial==true)
    {
        iBRTCAutoModelVerifyTask=1;
        for(int i=0; i<BTestSuck.iMaxRow; i++)
        {
            for(int j=0; j<BTestSuck.iMaxCol; j++)
            {
                bBTestSuckUse[i][j]=false;
                bBTSuckFinish[i][j]=false;
            }
        }
        return false;
    }

    int ret=0;
    int &Task=iBRTCAutoModelVerifyTask;
    bool bCheckAllSuck=false, flag=false;
    AnsiString ErrPart="";

    switch(Task)
    {
        case 1:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeedSlow, "DoBRTCAutoModelVerify 1"))
            {
                bVerifyNG=false;
                bPickUpErrReAutoVerify=false;                                   //JerryYang 20220215 : RTC Auto Verify half view check
                Task=100;
            }
            break;
        case 100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front,Prod.TestY2_Rear, iSpeedY, "DoBRTCAutoModelVerify 100"))
            {
                Task=150;
            }
            break;
        case 150:
            if(SendSiteMapToRTC(true, 2)!=-1)
            {
                DoTestYRearDelay.SetSecAndOn(10);
                Task=160;
            }
            break;
        case 160:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtSiteMap])
            {
                DoAllPassVerifyRTC(true);
                Task=200;
            }
            else if(DoTestYRearDelay.Off())
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
                if(CosFunction.bRTCHalfViewAutoVerify )                         //JerryYang 20220215 : RTC Auto Verify half view check
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
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoBRTCAutoModelVerify 12010"))
            {
                iReleaseCT=0;
                Task=12020;
            }
            break;
        case 12020:                                                                                                                                             //Z2 Down

            if(MOT[MTestZ2].Gali_ReadPos()>(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+750))                                                                      //kevin 20140612 add start
            {
                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            //Steven 20110725 : 不再使用IsSuckICFallDown
                            if(BTestSuck.Suck[i][j].Enable       &&
                               BTestSuck.Suck[i][j].SenUsing!="" &&
                               BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                               BTestSuck.Item[i][j]!=NULL_IC)
                            {
                                if(BTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    BTestSuck.Suck[i][j].Normal();                                                                                              //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                    flag=true;
                                    bRecIndexDropAlarm2=true;                                                                                                   //jou 2012-01-17 紀錄index Drop alarm
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ2].Gali_Command("ST", __FUNC__);
                    Task=12050;
                    return false;
                }
            }

            if(Prod.TestZ2_Drop_Offset!=0)
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+100+IniConfig.iD36_RTCAutoVerifyReleaseHeight+iReleaseCT, iSpeedSlow))    //jou 20180226 : 200 -> 150 -> 130
                {
                    Task=12100;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+100+IniConfig.iD36_RTCAutoVerifyReleaseHeight+iReleaseCT, iSpeedSlow))                            //jou 20180226 : 200 -> 150 -> 130
                {
                    Task=12100;                                                                                                                                 //Steven 20110511
                }
            }
            break;
        case 12050:                                                             //kevin 20140612  上升方便取料
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, 50000))              //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                Task=12055;
            }
            break;
        case 12055:
            bHasErr|=CheckIndexAllSuckICFallDown(false, true);
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(BTestSuck.Suck[i][j].Enable       &&
                       BTestSuck.Suck[i][j].SenUsing!="" &&
                       BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                       BTestSuck.Item[i][j]!=NULL_IC)
                    {
                        if(BTestSuck.Suck[i][j].GetStatus()==false)
                        {
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
                    if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                         //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                        bIsTestSitICFallDown=true;
                    ShowErrorMessage("JAM0304", K_SKIP, MTestZ2, false, ErrPart);                                       //Steven 20100129 : Device Drop Error

                    bPickUpErrReAutoVerify=true;                                                                        //JerryYang 20241220 : auto verify發生index drop error要重做

                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(BTestSuck.Suck[i][j].Error ||
                               (BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                BTestSuck.Item[i][j]!=NULL_IC &&
                                BTestSuck.Suck[i][j].GetStatus()==false))                                               //有用到且有吸到IC的卻掉了
                            {
                                BTestSuck.SetItemData(i, j, HAS_NULL_IC);                                               //Steven 20110829 : 把有IC掉料的位置改成Has Null IC
                                BTestSuck.Suck[i][j].Normal();                                                          //Steven 20110829 : 把真空關掉
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
            bBTestSuckDrop=true;

            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]==HAS_HOT_IC || BTestSuck.Item[i][j]==HAS_IC)
                    {
                        BTestSuck.Suck[i][j].Off();
                    }

                    DoTestYRearDelay.SetSecAndOn(0.3);
                }
            }
            Task=12200;
            break;
        case 12200:
            if(DoTestYRearDelay.Off())
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

                bIndexCheckNoStopVaccum=false;
                DoTestYRearDelay.SetSecAndOn(0.3);
                Task=12210;
            }
            break;
        case 12210:
            if(DoTestYRearDelay.Off())
            {
                iReleaseCT+=10;
                if(iReleaseCT<100)
                {
                    Task=12020;
                }
                else
                {
                    DoTestYRearDelay.SetSecAndOn(3.0);
                    Task=12220;
                }
            }
            break;
        case 12220:
            if(DoTestYRearDelay.Off())
            {
                Task=12300;
            }
            break;
        case 12300:                                                             //Z1 Up //Steven 20180417 : RTC Auto Verify新增回吸偵測
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+1000, iSpeedSlow))
            {
                Task=12310;
            }
            break;
        case 12310:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]==HAS_HOT_IC ||
                       BTestSuck.Item[i][j]==HAS_IC)
                    {
                        if(INDEX_SUCKER_TYPE==1)                                //Steven 20111202
                        {
                            fiosetview->bIndexSuck[1][i][j]=true;
                        }
                        else
                        {
                            BTestSuck.Suck[i][j].On();                          //kevin 20110504 check 掉料
                        }
                    }
                }
            }
            DoTestYRearDelay.SetSecAndOn(1);
            bIndexSuckCheck=false;
            Task=12320;
        case 12320:
            if(INDEX_SUCKER_TYPE==1)
            {
                bIndexSuckCheck=false;
                bIndexSuckCheck=W64bT2_ProcessIndexSuckDestroy2();   // AI(k5-DoBRTCAutoModelVerify) 20260810: golden :4553 fiosetview->ProcessIndexSuckDestroy2() -- reuses THIS file's Wave-1 gap stub (aTester_Rear.cpp:367), same as :1709
            }
            else
            {
                bIndexSuckCheck=true;
            }

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, iSpeedSlow))
            {
                Task=12330;
            }
            break;
        case 12330:
            if(INDEX_SUCKER_TYPE==1)
            {
                bIndexSuckCheck=false;
                bIndexSuckCheck=W64bT2_ProcessIndexSuckDestroy2();   // AI(k5-DoBRTCAutoModelVerify) 20260810: golden :4569 fiosetview->ProcessIndexSuckDestroy2() -- reuses THIS file's Wave-1 gap stub (aTester_Rear.cpp:367), same as :1709
            }
            else
            {
                bIndexSuckCheck=true;
            }

            if(DoTestYRearDelay.Off() && bIndexSuckCheck==true)
            {
                bHasErr=false;

                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].GetStatus())
                        {
                            bHasErr=true;
                        }
                        else
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }

                        if(INDEX_SUCKER_TYPE==1)                                //jou 2011-11-01負壓不能一直開著真空，必須關掉
                        {
                            BTestSuck.Suck[i][j].Normal();
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
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Suck[i][j].GetStatus())
                    {
                        ErrPart+=IndexSuckName[i][j];
                    }
                }
            }

            if(bHasErr)
            {                                                                                                           //jou 2012-03-05 開啟D44偵測到卡料時，In arm也要讓位。
                if(IndexAlarmInArmAway()==true)                                                                         //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    ShowErrorMessage("JAM0327", K_RETRY, MTestZ2, false, ErrPart);                                      //Vacuum Sensor Off Error
                    Task=12310;
                }
                else
                {
                    return false;
                }
            }
            break;
        case 12390:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, iSpeedSlow))
            {
                Task=12400;
            }
            break;                                                              //Steven 20180417 : RTC Auto Verify新增回吸偵測
        case 12400:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoBRTCAutoModelVerify 12400"))
            {
                bRTCAutoVerifyControlEP=true;
                ADAM_WriteVoltage(4.0);

                if(IniConfig.bD36_1EnableRTCAutoModelVerifyLive==true)
                {
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtOpenLive]=false;
                    COM2->SendCommToVision(COM2->rtOpenLive, true);
                    DoTestYRearDelay.SetSecAndOn(10);
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
            else if(DoTestYRearDelay.Off())
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
            DoTestYRearDelay.SetSecAndOn(10);
            Task=12430;
            break;
        case 12430:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtCloseLive])
            {
                Task=12500;
            }
            else if(DoTestYRearDelay.Off())
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
            DoTestYRearDelay.SetSecAndOn(10);
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
            else if(DoTestYRearDelay.Off())
            {
                if(bRTCRetry==true)
                {
                    bRTCRetry=false;
                    if(COM2->OpenRTCComPortAgain())
                        ShowMyMessage("RTC Release Time out");                  //JerryYang 20220215 : Release跟InspEnd一起送
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
            DoTestYRearDelay.SetSecAndOn(10);
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
            else if(DoTestYRearDelay.Off())
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
            DoTestYRearDelay.SetSecAndOn(10);
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
            else if(DoTestYRearDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Verify All Fail Time out");
                COM2->DoReleaseAndInspEnd();
                Task=12500;
            }
            break;
        case 13210:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease]=false;                //JerryYang 20220215 : Release跟InspEnd一起送
            COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd]=false;
            COM2->DoReleaseAndInspEnd();
            DoTestYRearDelay.SetSecAndOn(10);
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
            else if(DoTestYRearDelay.Off())
            {
                if(bRTCRetry==true)
                {
                    bRTCRetry=false;
                    if(COM2->OpenRTCComPortAgain())
                        ShowMyMessage("RTC Release Time out");
                }
                else
                {
                    bRTCRetry=true;
                }
                Task=13210;
            }
            break;
        case 13300:
            if(IndexAlarmInArmAway()==true)
            {
                Task=13400;
            }
            break;
        case 13350:
            if(DoHalfViewAllFailVerifyRTC())                                    //JerryYang 20220215 : RTC Auto Verify half view check
            {
                Task=13400;
            }
            break;
        case 13400:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoBRTCAutoModelVerify 13400"))
            {
                Task=13500;
            }
            break;
        case 13500:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+10+IniConfig.iD36_RTCAutoVerifyPickHeight, iSpeedSlow/3))                     //jou 20180226 : 0 -> +110 -> +60 -> 0 -> +10
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]!=NULL_IC &&
                           BTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            bBTestSuckUse[i][j]=true;
                        }
                        else
                        {
                            bBTestSuckUse[i][j]=false;
                        }

                        BTestSuck.Suck[i][j].Error=false;
                        bBTSuckFinish[i][j]=false;
                    }
                }
                iHangupCTArm2=0;
                Task=13600;
            }
            break;
        case 13600:
            COM2->SendCommToVision(COM2->rtCLOSEVERIFY, true);
            DoTestYRearDelay.SetSecAndOn(10);
            Task=13700;
            break;
        case 13700:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtCLOSEVERIFY])
            {
                Task=13800;
            }
            else if(DoTestYRearDelay.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Close Verify Time out");
                COM2->DoReleaseAndInspEnd();
                Task=13600;
            }
            break;
        case 13800:
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
                bHasErr=false;
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error)
                        {
                            bHasErr=true;
                            if(IniConfig.bD62PickUpErrorNeedPurge)              //Steven 20161024 : 吸取異常需要吹氣一次
                            {
                                BTestSuck.Suck[i][j].Off();
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
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeedSlow, "DoBRTCAutoModelVerify 13900"))
            {
                bRTCAutoVerifyControlEP=false;
                ADAM_WriteVoltage(DeviceForm.dPress);                           //JerryYang 20220215 : 修正EP氣量錯誤
                DoTestYRearDelay.SetSecAndOn(3);
                Task=13950;
            }
            break;
        case 13950:
            if(DoTestYRearDelay.Off())
            {
                if(bVerifyNG==true || bHasErr==true)
                    Task=14000;
                else
                    Task=14400;
            }
            break;
        case 14000:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "DoBRTCAutoModelVerify 14000"))
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
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Suck[i][j].Error)
                    {
                        ErrPart+=IndexSuckName[i][j];
                        if(IniConfig.bD62PickUpErrorNeedPurge)                  //Steveb 20161024 : 吸取異常需要吹氣一次
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                    }
                    else
                    {
                        BTestSuck.Suck[i][j].Error=false;
                    }
                }
            }
            bIsTestSitICFallDown=true;
            bPickUpErrReAutoVerify=true;                                        //JerryYang 20220215 : RTC Auto Verify half view check
            ret=ShowErrorMessage("JAM0302", K_SKIP, MTestZ2, false, ErrPart);   //Devicr Pick-Up Error

            if(ret==K_SKIP)
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error)
                        {
                            BTestSuck.SetItemData(i, j, HAS_NULL_IC);
                            BTestSuck.Suck[i][j].Normal();
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
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedY, "DoBRTCAutoModelVerify 14300"))
            {
                Task=14400;
            }
            break;
        case 14400:
            if(W906_FormShowing("fContact", fContact->fShow) && iContactMode!=K5_CONTACT_NORMAL)                 //JerryYang 20220215 : RTC Auto Verify half view check   // AI(k5-DoBRTCAutoModelVerify) 20260810: golden :5035 CONTACT_NORMAL -> K5_CONTACT_NORMAL (see K5_SEAM; cContact.h is unreachable from this TU)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
            {
                Task=15000;
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iSpeedSlow))
                {
                    Task=15000;
                }
            }
            break;
        case 15000:
            if(SendSiteMapToRTC(true,0)!=-1)
            {
                DoTestYRearDelay.SetSecAndOn(10);
                Task=15100;
            }
            break;
        case 15100:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtSiteMap])
            {
                DoAllPassVerifyRTC(true);
                Task=15200;
            }
            else if(DoTestYRearDelay.Off())
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
                if(bDoFRTCAutoModelVerify==false)
                    bRTCAutoModelVerifyFirstTime=true;
            }

            RecordProcess("RTC auto verify end");
            bDoFRTCAutoModelVerify=true;
            COM2->InitRealTimeCCDPara();
            bBTestSuckDrop=false;
            if(W906_FormShowing("fContact", fContact->fShow) && iContactMode!=K5_CONTACT_NORMAL)                 //JerryYang 20220215 : RTC Auto Verify half view check   // AI(k5-DoBRTCAutoModelVerify) 20260810: golden :5079 CONTACT_NORMAL -> K5_CONTACT_NORMAL (see K5_SEAM; cContact.h is unreachable from this TU)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
            {
            }
            else
            {
                IndexStatus=Z1Up_Z2Down;
            }
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
// End of part k5-DoBRTCAutoModelVerify (golden aTester_Rear.cpp :4255-5089).
// Release the scoped COM2 rebinding so a sibling part's text is untouched and
// the real `extern TCOM2Shim *COM2;` (atester_shims.h:461) is visible again.
#undef COM2
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// ROLE: Arm1 (front Z1/Y1) "pick-up error -> piggyback" recovery state machine --
//   after an Index Arm1 pick-up error it re-sucks every NULL_IC/HAS_NULL_IC site,
//   lifts Z1 to the socket-check height, re-reads the vacuum to decide whether an
//   IC is still sitting in a Test Socket, and if so raises the socket-residual
//   alarm (WAR0310) after moving the arms clear.  Returns true when finished.
//
// WAVE SCOPE (label k8-Arm2-decide):
//   * DoIndexArm1PickUpErrNeedPiggyback(bool)   golden :5093-5254   ACTIVE
//   * TQPF_Timer hCheckSockerDelay1             golden :5092        ACTIVE
//     (file-scope peer of the function, exactly the status BTempRiseHasICDelay
//     golden :308 already has in this file at line 301.  Golden references it
//     ONLY from this one function -- verified 2026-08-10T19:16:30+0800 with
//     `grep -rn "hCheckSockerDelay1" --include=*.cpp --include=*.h .` in the
//     golden tree: 6 hits, all inside golden aTester_Rear.cpp :5092-5161 -- so no
//     sibling part in this wave can double-define it.)
//
// NOT DEFINED HERE, ON PURPOSE (both are plain data globals that already exist):
//   * iIndexArm1PickUpErrNeedPiggybackTask (golden :5091) -- already defined at
//     atester_shims.cpp:201, declared atester_shims.h:124 (which this file
//     already includes).  It must STAY a shared global: golden main.cpp:9852
//     registers its ADDRESS in QueueTaskList[121].SetAliasAndTask(...), so a
//     TU-local copy here would silently detach the task-list monitor.  Same
//     treatment the file-top banner already documents for bArm2SuckFinish[][].
//   * iTestYRearTask (golden :5352) -- already defined atester_shims.cpp:194,
//     declared atester_shims.h:117 (see the InitTestYRearTask part file).
//
// GATE REGISTER
// -----------------------------------------------------------------------------
// GATE k8-G1 -- golden :5140  `fiosetview->ProcessIndexSuckDestroy1();`
//   ABSENT: the offline TfiosetviewShim (atester_shims.h:404-408) exposes ONLY
//     `bool bIndexSuck[2][4][8];` plus its ctor -- no ProcessIndexSuckDestroy1.
//   COMMAND PROVING ABSENCE (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//     grep -rn "ProcessIndexSuckDestroy1" --include=*.h .
//   RESULT: exit 1, zero hits.  RUN AT: 2026-08-10T19:15:44+0800.
//     (The only bodies in the tree are TU-local stand-ins in .cpp files:
//     atester.cpp:5517 `static bool W7T1_ProcessIndexSuckDestroy1(){return true;}`
//     -- reachable from atester.cpp ONLY, not from here.  Every raw
//     `fiosetview->ProcessIndexSuckDestroy1()` in atester.cpp (:4538, :9653,
//     :9807) sits inside an `#if 0` region -- measured, not assumed.)
//   WHY THE OFFLINE DEFAULT IS FAITHFUL: golden DISCARDS the return value at
//     this call site, so no control flow inside this function depends on it.
//     The stand-in returns true, matching the atester.cpp W7T1 precedent and
//     this file`s own W64bT2_ProcessIndexSuckDestroy2.
//   REAL-MACHINE BEHAVIOUR DIFFERENCE (this one is NOT cosmetic -- read it):
//     golden case 1040 is re-entered every scan for the whole 0.5 s window and
//     each pass PUMPS the index suck/destroy solenoids for the sites case 1030
//     flagged in bIndexSuck[0][][].  With the pump gated out, on a real machine
//     with INDEX_SUCKER_TYPE==1 the vacuum is never actually driven, so
//     CheckVaccumIsIniaialON at case 1060 reads "no vacuum", flag stays false,
//     and the function returns true at case 1060 -- i.e. the WHOLE piggyback
//     socket-residual check (cases 1070..2200, incl. the WAR0310 alarm) is
//     INERT for INDEX_SUCKER_TYPE==1.  The INDEX_SUCKER_TYPE!=1 path (case 1030
//     `else` branch, FTestSuck.Suck[i][j].On()) is NOT affected by this gate.
//     Retire the gate by giving TfiosetviewShim the method; do not ship a real
//     machine on this gate.
//
// GATE k8-G2 -- golden :5307 and :5339 `iHangupCTArm2=0;` live in the
//   CheckAnyCaseNeedToDoArm2 part file; the full entry is written there.
//
// NOT GATED, DELIBERATELY -- TMyKitSuck::CheckVaccumIsIniaialON (golden :5173).
//   The method is absent from the aHotPlateSubstrate.h mirror of TMyKitSuck
//   (`grep -n "CheckVaccumIsIniaialON" aHotPlateSubstrate.h` -> exit 1, zero
//   hits, run 2026-08-10T19:14:09+0800), and atester.cpp works around the same
//   gap with a DEGRADED macro (atester.cpp:5528
//   `#define W7T1_CHECKVACINIT(kit,i,j,out) do { (out)=false; } while(0)`) that
//   throws the detection away.  That is NOT necessary: golden`s own body
//   (golden mykitsuck.cpp:2732-2746) touches ONLY members the mirror already
//   has -- Suck[][].GetStatus() (aHotPlateSubstrate.h:131), Suck[][].Normal()
//   (:130), Item[][] (:370), and NULL_IC/HAS_NULL_IC.  So this file carries a
//   FAITHFUL free-function transcription instead (zero new dependencies), the
//   same treatment the file-top banner already justifies for
//   W64bT2_SetUnuseToNullIC.  HONEST CONSEQUENCE: TMySucker::GetStatus is
//   itself an offline stub today (aHotPlateSubstrate.cpp:240 `return false;`),
//   so OFFLINE this reaches the `else` branch and leaves flag false -- the same
//   net offline value as atester.cpp`s degraded macro.  The difference that
//   matters: when GetStatus becomes a real sensor read, THIS code is already
//   correct and needs no edit, whereas the macro would stay silently broken.
//   Equivalent offline values are not the point -- the false premise is.
//
// GOLDEN QUIRKS PRESERVED, NOT FIXED:
//   * case 1 has NO break: it sets Task=1030 and falls THROUGH into case 1030.
//   * case 1030 ends with golden`s own commented-out `//            break;`
//     (:5136) and therefore falls THROUGH into case 1040 on the same scan.
//   * case 2200 has no trailing break (last label in the switch).
//   * `int iIndexCheckOffSet=IniConfig.fIndexCheckOffset*100;` -- fIndexCheckOffset
//     is a double (Config.h:83); golden TRUNCATES the product to int.  Kept as an
//     int initialised from the double product, exactly as golden.  Do NOT
//     "clean this up" into a rounding helper.
//   * `ErrPart=" "` (a single SPACE, not "") at :5164 while :5235 resets it to "".
//   * `bIndexCheckNoStopVaccum=true` is re-assigned inside the per-site loop.
//   * ErrPart is a function-static AnsiString, so it survives across scans.
//
// TRANSLATION NOTES: golden text is otherwise verbatim, including trailing
// comment column positions.  Big5 comments decoded via cp950 and preserved as
// UTF-8.  Non-static definition (golden is non-static too; golden declares it in
// no header at all -- its only caller, DoTestYRear golden :5362, lives in the
// same translation unit at golden :9132/:9136).
// Translator: AI(k8-Arm2-decide) 20260810
//------------------------------------------------------------------------------

// golden :5092 -- file-scope peer of DoIndexArm1PickUpErrNeedPiggyback.
// TRAP-4 CHECKED: TQPF_Timer`s ctor (myTimer.cpp:14-17) only calls its own
// CalibratePerformanceCounterOverhead(), which touches QueryPerformanceCounter()
// and this object`s own members (myTimer.cpp:19-30).  It reads NO global pointer,
// so this static-init ctor cannot repeat the fLaserSensor/elLaser SEGFAULT shape.
// Same shape as the already-shipping TQPF_Timer BTempRiseHasICDelay at line 301.
TQPF_Timer hCheckSockerDelay1;

// =============================================================================
//  k8-Arm2-decide offline seams.  Uniquely prefixed (W7bK8_) so that being
//  file-scope `static` here can never become TRAP-1 shape (d): no non-static
//  declaration of either name exists anywhere in the tree, so there is nothing
//  for these to shadow.  The three GOLDEN functions in this wave
//  (DoIndexArm1PickUpErrNeedPiggyback / CheckAnyCaseNeedToDoArm2 /
//  InitTestYRearTask) are deliberately NOT static.
// =============================================================================
// -- FAITHFUL transcription of golden TMyKitSuck::CheckVaccumIsIniaialON
//    (golden mykitsuck.cpp:2732-2746), as a free function because the method is
//    absent from the aHotPlateSubstrate.h mirror.  Body is golden`s, statement
//    for statement -- NOT an offline approximation.  See the banner above.
static void W7bK8_CheckVaccumIsIniaialON(TMyKitSuck &kit, int iRow, int iCol, bool &flag)
{
    if(kit.Suck[iRow][iCol].GetStatus())
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

// -- GATE k8-G1 stand-in for fiosetview->ProcessIndexSuckDestroy1() (golden
//    :5140).  Offline: report the index suck self-check "done" (true), the same
//    offline value as atester.cpp:5517`s W7T1_ProcessIndexSuckDestroy1 and this
//    file`s W64bT2_ProcessIndexSuckDestroy2.  Golden discards the result here.
static bool W7bK8_ProcessIndexSuckDestroy1() { return fiosetview->ProcessIndexSuckDestroy1(); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）
//------------------------------------------------------------------------------
bool DoIndexArm1PickUpErrNeedPiggyback(bool bInitial)                           //Steven 20190115 : SCC要求吸取異常要檢查Socket
{
    static AnsiString ErrPart="";

    if(bInitial==true)
    {
        iIndexArm1PickUpErrNeedPiggybackTask=1;
        return false;
    }

    int iIndexUpPos=0;
    int iNN=IsNNMode();
    int iIndexCheckOffSet=IniConfig.fIndexCheckOffset*100;                      //ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset
    int &Task=iIndexArm1PickUpErrNeedPiggybackTask;
    bool flag=false, flag2=false;

    switch(Task)
    {
        case 1:
            Task=1030;                                                          //對齊DoCheckSocketHasIC()的Task
        case 1030:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==NULL_IC ||
                       FTestSuck.Item[i][j]==HAS_NULL_IC)
                    {
                        if(INDEX_SUCKER_TYPE==1)
                        {
                            fiosetview->bIndexSuck[0][i][j]=true;
                            bIndexCheckNoStopVaccum=true;
                        }
                        else
                        {
                            FTestSuck.Suck[i][j].On();                          //Sam 20230110 : 修正連續 Start<>Stop Drop Contact 掉料問題
                        }
                    }
                }
            }

            hCheckSockerDelay1.SetSecAndOn(0.5);
            Task=1040;
//            break;
        case 1040:
            if(INDEX_SUCKER_TYPE==1)
            {
                //AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE k8-G1 -- golden :5140 fiosetview->ProcessIndexSuckDestroy1() (TfiosetviewShim, atester_shims.h:404-408, has bIndexSuck[][][] but not this method)
                fiosetview->ProcessIndexSuckDestroy1();
                //AI(W906-IDXSUCK) （原 #endif）
                //AI(W906-IDXSUCK) 20260927: 上面已是 golden 的呼叫，這行離線替身停用（不然同一拍泵跑兩次）：W7bK8_ProcessIndexSuckDestroy1();                                       // AI(k8-Arm2-decide) 20260810: GATE k8-G1 offline stand-in for the golden :5140 call above
            }

            if(hCheckSockerDelay1.Off())
            {
                Task=1050;
            }
            break;
        case 1050:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);                                                                                                    //Steven 20140620 : 整合為Function

            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos+iIndexCheckOffSet, iSpeedFast))                                   //ChungHung 20140807 add for ATK TestZ_Test
            {
                Task=1060;
                if(CUSTOMER_CODE==CC_Greatek)                                                                                                                   //Wei 20160413
                    hCheckSockerDelay1.SetSecAndOn(5);                                                                                                          //Steven 20110908 : 上來後也要Delay一下
                else
                    hCheckSockerDelay1.SetSecAndOn(0.5);                                                                                                        //Steven 20110908 : 上來後也要Delay一下
            }
            break;
        case 1060:
            if(hCheckSockerDelay1.Off())
            {
                flag=false;
                ErrPart=" ";
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==NULL_IC ||
                           FTestSuck.Item[i][j]==HAS_NULL_IC)
                        {
                            flag2=false;
                            W7bK8_CheckVaccumIsIniaialON(FTestSuck, i, j, flag2);        // AI(k8-Arm2-decide) 20260810: golden :5173 FTestSuck.CheckVaccumIsIniaialON(i, j, flag2) -- FAITHFUL free-function transcription (method absent from the aHotPlateSubstrate.h mirror; see banner)
                            if(flag2==true)
                            {
                                flag=true;
                                ErrPart+=IndexSuckName[i+iNN][j];

                                if(CUSTOMER_CODE==CC_SCS)                       //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                                    FTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }

                if(flag && LastSet.iRealDummy==REALLY)                          //Steven 20120726 : 有跑IC才檢查Socket
                {
                    IndexStatus=Z1_Z2_Normal;
                    Task=1070;
                }
                else
                {
                    return true;
                }
            }
            break;
        case 1070:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoIndexArm1PickUpErrNeedPiggyback 1070"))
            {
                if(CUSTOMER_CODE==CC_SCS)                                       //jou 20170516 (Steven) : SCS要求index check偵測到device時需吹氣
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]==NULL_IC ||
                               FTestSuck.Item[i][j]==HAS_NULL_IC)
                            {
                                FTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                }
                Task=1080;
            }
            break;
        case 1080:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front,Prod.TestY2_Rear, iSpeedY, "DoIndexArm1PickUpErrNeedPiggyback 1080"))
            {
                Task=1090;
            }
            break;
        case 1090:
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                bIsTestSitICFallDown=true;                                      //Steven 20130613
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)            //Steven 20130604 : Socket殘料要按Z1
                {
                    ShowErrorMessage("WAR0310", K_RETRY, MTestY1, false, ErrPart);
                }
                else
                {
                    ShowMyMessage("Arm1 detect Test Socket has IC error", "Arm 1偵測到Socket有IC殘留!!", "DoIndexArm1PickUpErrNeedPiggyback 1090");
                }
                ErrPart="";
                Task=1100;
            }
            break;
        case 1100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedY, "DoIndexArm1PickUpErrNeedPiggyback 1100"))
            {
                IndexStatus=Z1Down_Z2Up;
                Task=2200;
            }
            break;
        case 2200:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test, iSpeedFast))         //ChungHung 20140807 add for ATK TestZ_Test
            {
                return true;
            }
    }

    return false;
}

//------------------------------------------------------------------------------
// ROLE: the Arm2 (rear) DISPATCH DECISION -- a pure priority ladder that returns
//   the Task number DoTestYRear should jump to next.  In order: Arm1 piggyback
//   socket check (16000), SLK separability combine (14000), RTC auto model
//   verify (12000), RTC socket-float (15000), tester side-push retract (18500),
//   then the normal vacuum-ON (1 / 11040) vs vacuum-OFF (211 / 215 / 11040)
//   branch.  Returns 0 only if the VacuumMode ladder somehow assigns nothing.
//
// WAVE SCOPE (label k8-Arm2-decide):
//   * CheckAnyCaseNeedToDoArm2()   golden :5256-5350   ACTIVE
//
// GATE REGISTER
// -----------------------------------------------------------------------------
// GATE k8-G2 -- golden :5307 and golden :5339, both `iHangupCTArm2=0;`
//   ABSENT: the global iHangupCTArm2 does not exist anywhere in the port tree --
//     not declared, not defined, not shimmed, not even mentioned in a comment.
//   COMMAND PROVING ABSENCE (run in D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//     grep -rn "iHangupCTArm2" --include=*.h --include=*.cpp .
//   RESULT: exit 1, zero hits.  RUN AT: 2026-08-10T19:15:44+0800.
//   WHY THE OFFLINE DEFAULT IS FAITHFUL: within the whole scope of this label
//     iHangupCTArm2 is WRITE-ONLY -- neither of golden :5307/:5339 ever reads it,
//     and no other line in golden :5093-5356 mentions it.  So dropping the two
//     stores changes NO control flow and NO return value of this function: the
//     ladder still returns exactly the same Task number for every input state.
//   REAL-MACHINE BEHAVIOUR DIFFERENCE: golden CLEARS the Arm2 contact-test
//     hang-up counter every time this ladder reaches either VacuumMode branch.
//     Whoever increments and reads that counter (the Arm2 contact-test hang-up
//     watchdog, not yet translated) would therefore see a counter that is never
//     reset, so a count accumulated across earlier cycles could latch a spurious
//     Arm2 hang-up.  This is a real hole, not a cosmetic one.
//   WHY NOT JUST DEFINE IT HERE: a TU-local `int iHangupCTArm2;` would be a
//     private shadow of what golden makes a SHARED global (cmydef-class state
//     read by another module) -- precisely TRAP-1 shape (e)/(d).  It would link
//     clean and silently decouple the writer from the reader.  HAND-OFF: add
//     `extern int iHangupCTArm2;` to cmydef.h plus `int iHangupCTArm2=0;` to
//     cmydef.cpp, then delete both #if 0 blocks below.  Not done here: cmydef
//     is outside this task`s write boundary.
//
// DEPENDENCIES VERIFIED PRESENT (so they are translated ACTIVE, not gated):
//   * BTestCombineSLK(bool)          atester_shims.h:143 -- declared; its body
//     is still the offline stub in atester_shims.cpp (returns true).  The CALL is
//     faithful; retiring that stub is the integrator`s job, not a gate here.
//   * SendSiteMapToRTC(bool,int)      real body atester.cpp:11943, NOT inside any
//     `#if 0` (the :11863 twin IS gated -- measured, not assumed).
//   * CheckIndexAllSuckICFallDown(bool,bool)  real body csystem_predicates.cpp
//     (see this file`s file-top INTEGRATE-STEP FIXES note).
//   * RecordProcess(AnsiString,AnsiString="")  canary_support.h:70.
//   * BTestSuck.UseSiteHasIC()/AlreadyTest()/CountRealIC()  aHotPlateSubstrate.h
//     :467/:464/:455.  TRAP-5: BTestSuck is the aHotPlateSubstrate.h TMyKitSuck
//     (class at :365, object `extern` at :637) -- the 177-TU one -- NOT the
//     mykitsuck.h:274 class with the different layout.  This file already
//     includes aHotPlateSubstrate.h and already uses BTestSuck, so no new or
//     conflicting header dependency is introduced by this part.
//   * COM2->bCCDDummyRum  atester_shims.h:461/:423.  NOTE (pre-existing, not
//     mine): the shim ctor sets bCCDDummyRum=true offline, so `!COM2->bCCDDummyRum`
//     is false and BOTH REAL_TIME_CCD ladders below short-circuit offline.  That
//     is the shim`s documented offline value, not a gate introduced here.
//
// GOLDEN QUIRKS PRESERVED, NOT FIXED:
//   * The `else { bRTCAutoVerifyControlEP=false; }` at :5288-5291 hangs off the
//     OUTER RTC `if`.  So when the outer RTC conditions ARE all true but the
//     inner UseSiteHasIC/AlreadyTest/SendSiteMapToRTC checks fail, the flag is
//     NOT cleared and keeps its previous value.  Looks like a misplaced else;
//     kept exactly as golden.
//   * `Task` is initialised to 0 and the four early `return`s bypass it entirely.
//   * The identical bIndexArm2SupplyLight/bForEgisTecTest/bD58.. triple-condition
//     block is DUPLICATED in both VacuumMode branches (:5315-5321 and :5341-5347),
//     which is why the vacuum-OFF branch can overwrite its own 211/215 with 11040.
//   * `iHangupCTArm2=0;` sits BEFORE the light check in the ON branch (:5307) but
//     AFTER the 211/215 assignment in the OFF branch (:5339) -- asymmetric on
//     purpose in golden; both positions are held by the #if 0 blocks below.
//   * golden`s own commented-out `// ShowIndexTime(1);` at :5324 is reproduced
//     verbatim, including its trailing timing note.
//
// Translator: AI(k8-Arm2-decide) 20260810.  Non-static (golden declares it
// `extern int CheckAnyCaseNeedToDoArm2();` in golden aTester_Rear.h:18; its caller
// is DoTestYRear at golden :7535).
//------------------------------------------------------------------------------
int CheckAnyCaseNeedToDoArm2()
{
    int Task=0;
    if(IniConfig.bD43IndexPickErrCheckSocket &&
       bIndexArm1PickUpErrNeedPiggyback==true)
    {
        bIndexArm1PickUpErrNeedPiggyback=false;
        return 16000;
    }

    if(TestIF_File.bUseSLKClamp && TestIF_File.iSeparabilityTest==1)            //JerryYang 20160429 分離模式
    {
        BTestCombineSLK(true);                                                  //初始化流程
        return 14000;                                                           //JerryYang 20160429 分離模式, 測試完成 Z1 SLK要與Clamp結合,再將IC吸起流程
    }

    if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum && LastSet.iRealDummy==REALLY &&
       CosFunction.bRTCAutoModelVerify==true && IniConfig.bD36EnableRTCAutoModelVerify==true &&
       bRTCAutoModelVerifyFirstTime==true)                                      //jou 2014-06-24 RTC 自動進行Model驗證
    {
        if(BTestSuck.UseSiteHasIC() &&
           BTestSuck.AlreadyTest()==false)
        {
            if(SendSiteMapToRTC(false, 2)==BTestSuck.CountRealIC())
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

    if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum && TestIF_File.bUseSocketFloat==true)
    {
        return 15000;
    }

    if(CosFunction.bTesterSidePushFunction==true &&
              DeviceForm.bTesterSidePush==true &&
              DeviceForm_File.iSidePushMode==1)                                 //Richard 20230301 : 測試完成, 側推縮回
    {
        return 18500;
    }

    if(DeviceForm.VacuumMode==VacuumONMode)
    {
        // AI(pt-wave) 20260810: GATE k8-G2 RETIRED at integration -- its premise ("iHangupCTArm2 exists nowhere") died in this same wave (TRAP 2). golden aTester_Rear.cpp:76 declares it, and PT-W7b now declares it at file scope in THIS TU (see the eight-globals block above), which is golden's own home for it rather than cmydef. Re-asked TRAP 3 "why SHOULD it stay gated": no reason survives -- it is a plain reset with no side effects, and gating it would drop golden's hangup-counter reset.
        iHangupCTArm2=0;
        if(DeviceForm.ContactMode==DropContact &&                               //JerryYang 20170522 drop contact改為邊吸邊測
           CosFunction.bSuckDevicesDuringTest==true &&                          //JerryYang 20170804 (Steven) 移除邊吸邊測的選項
           INDEX_SUCKER_TYPE==1)
        {
            CheckIndexAllSuckICFallDown(true, false);
        }

        if(IniConfig.bIndexArm2SupplyLight==true ||                             //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
           TestIF_File.bForEgisTecTest==true     ||                             //Steven 20140922 : Arm2當作指紋測試
           (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                     //kevin 20150127 Arm1 下壓 arm2 測試
            TestIF_File.bArm1PickPlaceArm2Test==true))                          //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
        {
            Task=11040;
        }
        else
        {
//            ShowIndexTime(1);                                                 //Steven 20140619 : 測試      //到這裡大概0.015~0.031Sec
            Task=1;
        }
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

        // AI(pt-wave) 20260810: GATE k8-G2 RETIRED at integration -- its premise ("iHangupCTArm2 exists nowhere") died in this same wave (TRAP 2). golden aTester_Rear.cpp:76 declares it, and PT-W7b now declares it at file scope in THIS TU (see the eight-globals block above), which is golden's own home for it rather than cmydef. Re-asked TRAP 3 "why SHOULD it stay gated": no reason survives -- it is a plain reset with no side effects, and gating it would drop golden's hangup-counter reset.
        iHangupCTArm2=0;

        if(IniConfig.bIndexArm2SupplyLight==true ||                             //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
           TestIF_File.bForEgisTecTest==true     ||                             //Steven 20140922 : Arm2當作指紋測試
           (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                     //kevin 20150127 Arm1 下壓 arm2 測試
            TestIF_File.bArm1PickPlaceArm2Test==true))                          //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
        {
            Task=11040;
        }
    }
    return Task;
}

//------------------------------------------------------------------------------
// ROLE: reset the rear (Arm2) test-Y state machine back to its first step, by
//   setting the iTestYRearTask task index to 1.  Called before (re)entering the
//   DoTestYRear engine so it restarts from a known step.
//
// WAVE SCOPE (label k8-Arm2-decide):
//   * InitTestYRearTask()   golden :5353-5356   ACTIVE
//
// GATE REGISTER: none -- this part writes no #if 0.  Its single dependency,
//   iTestYRearTask, is PRESENT: defined at atester_shims.cpp:194 and declared
//   `extern int iTestYRearTask;` at atester_shims.h:117, which this file already
//   includes (line 151).  Verified 2026-08-10T19:15:44+0800 with
//   `grep -rn "iTestYRearTask" --include=*.h --include=*.cpp .`
//
// DELIBERATELY NOT DEFINED HERE: golden :5352 `int iTestYRearTask=1;`.  It is a
//   plain data global that ALREADY exists in atester_shims.cpp:194 with the same
//   initial value 1, so re-defining it here would be a duplicate definition and
//   break the link.  It stays in the shim -- the same treatment this file`s
//   file-top banner already documents for bArm2SuckFinish[][].
//
// TRAP-1 WARNING FOR THE INTEGRATOR (shape (b), the invisible one): the no-op
//   stub `void InitTestYRearTask() {}` at atester_shims.cpp:204 ALREADY satisfies
//   every existing call, so once this real body lands the build and the link both
//   stay green while the STUB is what actually runs -- and `nm --undefined-only`
//   cannot see it, because the symbol is defined, just by the wrong object.  The
//   stub MUST be deleted from atester_shims.cpp in the same integrate step.  The
//   same applies to `int CheckAnyCaseNeedToDoArm2() { return 0; }` at
//   atester_shims.cpp:203 -- and note THAT stub is not merely inert: returning 0
//   instead of the real Task number silently sends DoTestYRear to Task 0.
//
// Translator: AI(k8-Arm2-decide) 20260810.  Non-static -- golden is non-static,
// and a `static` definition here would be TRAP-1 shape (d) against the existing
// non-static declaration in atester_shims.h.
//------------------------------------------------------------------------------
void InitTestYRearTask()
{
    iTestYRearTask=1;
}

// ---- chunk 1/3 of DoTestYRear, golden 5362..6537 ----
bool DoTestYRear()
{
    static int iRetry=0;
    static bool bOneTimeFlag=true, flag1=false, bSocketCheckSkip=false, bSucketHasICError=false;
    static bool bGetTime1=true, bGetTime2=true, bOnce=false;                    //JerryYang 20220923 : add
    static bool bintered2=true;                                                 //Isaac 20200922 : 紀錄indexArmY encoder值和command值
    static bool bIndexFinish=false;
    static bool bPlaceTestIC=false;                                             //Ifor 20190713 : add Arm1 丟下IC
    static bool bCheckShuttle=false;
    static bool bEPfirst=true, bFirst=true;                                     //Ifor 20150803 : 新增EP量測旗標，第一次才執行Log紀錄  //JerryYang 20180817 (Steven) : fix EOT delay
    static bool bSLKClampProcessFinish=false;                                   //JerryYang 20160526
    static bool bFTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false} };

    static bool bFTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},                                        //Steven 20110301 : 確認吸取完成
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bArmDuplicateErr[4][8]={{false, false, false, false, false, false, false, false},
                                        {false, false, false, false, false, false, false, false},
                                        {false, false, false, false, false, false, false, false},
                                        {false, false, false, false, false, false, false, false}};

    static AnsiString ErrPart="";
    static AnsiString str="";
    //==> Eastsun 20260511 F007 整合: T11/T12 Index 掉料當下馬達位置
    AnsiString StrPos="";                                                          //Ifor 20220125 add:Index 掉料當下 馬達位置
    int iArm1_Y=0, iArm1_Z=0, iArm2_Y=0, iArm2_Z=0;                                //Ifor 20220125 add:Index 掉料當下 馬達位置
    //<== Eastsun 20260511 F007 整合
    DoTestYRear_ReStart:                                                        //2008/10/20 lee

    QueueTaskList[23].CheckTaskChange();                                        //Steven 20200821 : 使用Goto也要記錄Task變化

    int &Task=iTestYRearTask;
    int iIndexZPos=0;                                                           //Ifor 20220906 add:讀取Index Z 目前位置
    int iIndexUpPos=0, iCT=0;
    int ret=0, iFlag=0;
    int iNN=IsNNMode();
    int iFinishTestUpWaitHeight=0;                                              //jou 2011-10-31 start : 丟測模式與不同速度模式也要支援兩段式上升
    int iAZ1=0, iAZ2=0, iAZ3=0;                                                 //kevin 20150506
    bool TMode=false;
    bool bCheckAllSuck=false, flag=false, bCheckSuck=false, bCheckDestroy=false, bHasErr=false, bHasDuplicateErr=false;
    long lPos[4]={0, 0, 0, 0};                                                  //kevin 20150915
    AnsiString str1;
    AnsiString StrDate="";
    AnsiString sBufferT="";                                                     //kevin 20150506
    AnsiString sTesterSidePush="Cylinder Side Push Status Error";               //Richard 20220321 : 渠梁Side Push
    static int bRetryRTC=false;                                                // AI(k1-DoTestYRear-c1) 20260810: golden :5412 is "static bRetryRTC=false;" -- K&R implicit int, accepted by BCB6, REJECTED by ISO C++.  GOLDEN BUG kept behaviourally identical by naming the same implicit type (int), NOT bool: every write is true/false and every read is a truth test, so 1/0 is exact.

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
            bIndex1Suck=false;                                                  //kevin 20220105 Index 在下真空建立 pause 不能關閉
            fRearNeedSuck=false;
            fRearNeedDestroy=false;
            InitRearTestSuckICTask();
            InitRearTestDestroyICTask();
            fFrontNeedSuckIC=false;

            if(BTestSuck.UseSiteNoIC())
            {
                InitRearTestSuckICTask();
                if(CanYieldAlarmRemainInSHT())                                  //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                {
                }
                else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)       //AI(ht9045-index-flow) 20260415 (RogerYang) : Shuttle2取消時不需要後臂吸取, 避免fCanMoveM鎖住造成F18偵測死鎖
                {
                }
                else
                {
                    fRearNeedSuck=true;
                }
            }
            else
            {
                if((BTestSuck.UseSiteHasIC() &&
                    BTestSuck.AlreadyTest()) ||
                   BTestNeedDestroy())
                {
                    fRearNeedDestroy=true;                                      // 先放掉已測IC,再進行新IC吸取
                    if(CanYieldAlarmRemainInSHT())                              //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
                    {
                    }
                    else if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)   //AI(ht9045-index-flow) 20260415 (RogerYang) : Shuttle2取消時不需要後臂吸取, 避免fCanMoveM鎖住造成F18偵測死鎖
                    {
                    }
                    else
                    {
                        fRearNeedSuck=true;
                    }
                    InitRearTestSuckICTask();
                    InitRearTestDestroyICTask();
                }
            }

            fFrontNeedTest=false;
            if(FTestSuck.UseSiteHasIC() &&
               FTestSuck.AlreadyTest()==false)
            {
                fFrontNeedTest=true;
                InitFTestSuckTestICTask();
                bArm1IsTest=false;                                              //JerryYang 20180629 (wei) : 用來判斷是否在測試中
            }

            Task=100;
            if(bNeedCheckRTCReport==true)
            {
                Task=2;
                DoGiveWayDelay.SetSecAndOn(10);
            }
            goto DoTestYRear_ReStart;                                           //2008/10/20 lee
        case 2:
#if 0 // GATE K1G1 -- golden :5488 COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK] (TCOM2Shim, atester_shims.h:420-460, declares NEITHER member)
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK])
#else
            if(false)                                                        // GATE K1G1 offline: RTC vision never answers (bRealTimeCom_ReceiveOK[] all false)
#endif
            {
                COM2->DoReleaseAndInspEnd();                                    //JerryYang 20260506 : Add
                bRTCFullViewError=false;                                        //Steven 20120206 : RTC重複錯誤
                Task=100;
                bNeedCheckRTCReport=false;
                goto DoTestYRear_ReStart;                                       //2008/10/20 lee
            }
#if 0 // GATE K1G1 -- golden :5496 COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG])
#else
            else if(false)                                                   // GATE K1G1 offline: RTC vision never answers
#endif
            {
                COM2->DoReleaseAndInspEnd();
                ShowMyMessage("RTC Full View NG");
#if 0 // GATE K1G1 -- golden :5500 COM2->bRealTimeCom_ReceiveOK[] write (no such member; offline no-op)
                COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]=false;
#endif
                Task=3;
            }
            else if(DoGiveWayDelay.Off())
            {
                COM2->DoReleaseAndInspEnd();
                ShowMyMessage("RTC Release Time out Test Y Rear");
                Task=3;
            }
            break;
        case 3:
#if 0 // GATE K1G1 -- golden :5511-5514 COM2->bRealTimeCom_ReceiveOK[] writes + COM2->SendCommToVision(rtCHECKNULL/rtFullTOK) (no such members; offline no-op)
            COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK]=false;
            COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]=false;
            COM2->SendCommToVision(COM2->rtCHECKNULL, true);
            COM2->SendCommToVision(COM2->rtFullTOK, true);
#endif
            DoGiveWayDelay.SetSecAndOn(10);
            Task=2;
            break;
        //-------------------------
        //掉料時的處置方式
        //-------------------------
        case 50:
            MOT[MTestZ1].MovFlag=false;
            MOT[MTestZ1].bScanFlag=false;
            ShowIndexTime(-2);                                                  //Steven 20200715 : 重新計算Cycle Time
            DoTestYRearDelay.SetMSAndOn(300);
            Task=52;
            break;
        case 52:
            if(DoTestYRearDelay.Off())
                Task=55;
            break;
        case 55:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 50000, "DoTestYRear 55"))
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
#if 0 // GATE K1G2 -- golden :5548 COM2->RTC_AlarmType() (TCOM2Shim has no such method)
                    if(REAL_TIME_CCD==true && COM2->RTC_AlarmType())    //wei 20221222 RTC ARM Error
#else
                    if(REAL_TIME_CCD==true && false)                    // GATE K1G2 offline: no RTC alarm latched  //wei 20221222 RTC ARM Error
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
            if(IniConfig.bI26TestCloseSiteHaveBin &&bTestBinDataError==0)       //kevin 20150202 需強致將arm上 ic取出
            {
                Task=55;
                return false;
            }
            else
            {
#if 0 // GATE K1G3 -- golden :5577 ShowMyMessageUp(...) (no declaration anywhere in the ported tree; the ported operator dialogs are ShowMyMessage/ShowErrorMessage only). Offline: no-op, so case 57 spins on its own guard exactly as golden does while the operator has not answered.
                ShowMyMessageUp("Test Bin error, must take out all IC for test.", "測試bin 設定有問題,將ARM 2 上產品取出", false);
#endif
            }
            break;
        case 60:
            if(MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false ||
               MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false)
            {
                lPos[0]=Prod.TestZ1_Safe;                                                                               //kevin 20150915
                RecordIndexPositionError("DoTestYRear()Task=60", true, false, false, false, &lPos[0]);                  //kevin 20150915 record

                ShowIndexMotorError(AnsiString("DoTestYRear60"));
                break;
            }

            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYRear 60"))
            {
                Task=6000;
            }
            break;
        case 6000:
            if(IndexAlarmInArmAway()==true)                                                                             //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                str1.sprintf("DoTestYRear 6000: iShowSocketSensor=%d, bSucketHasICError=%d", iShowSocketSensor, (bSucketHasICError)?1:0);
                RecordProcess(str1);

                if(TestIF_File.bEnSocketSensor &&                                                                       //Steven 20200709 : 換位置
                   iShowSocketSensor)                                                                                   //kevin 20130504 socket sensor
                {
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)                                                //Steven 20151022 : add for MAXIM
                        bIsTestSitICFallDown=true;
                    bIsSocketSensor=true;

                    if(iShowSocketSensor==2)                                                                            //Steven 20200604 : Arm1丟 Arm2測的時候, Arm2只檢查置偏
                    {
                        ShowErrorMessage("WAR0323", K_RETRY, MTestZ1, false, sSocketSensorErr);                         //Socket detect device floting error
                        Task=78;                                                                                        //Steven 20200615 : 置偏不用做RTC的檢查
                        iShowSocketSensor=0;                                                                            //Jimmychiu 20230817 : Socket sensor異常顯示後參數歸零
                        CheckSocketSensor(1, "DoTestYRear_6000", true);
                        break;
                    }
                    else if(iShowSocketSensor==1)
                    {
                        ShowErrorMessage("WAR0322", K_RETRY, MTestZ1, false, sSocketSensorErr);                         //kevin 20130504 socket sensor
                    }
                    iShowSocketSensor=0;
                    CheckSocketSensor(1, "DoTestYRear_6000", true);
                }

                if(bSucketHasICError)
                {
                    bIsTestSitICFallDown=true;
                    bSucketHasICError=false;
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)                                                //Steven 20130604 : Socket殘料要按Z1
                    {
                        ShowErrorMessage("WAR0310", K_RETRY, MTestY2, false, ErrPart);                                  //Steven 20101122
                    }
                    else
                    {
                        ShowMyMessage("Socket has IC error", "Socket有IC殘留!!", "DoTestYRear 60");
                    }
                }

                if(IniConfig.bVTESTFunction==true &&                                                                    //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
                   IniConfig.bGetRcsCheckingResult==true)
                {
#if 0 // GATE K1G4 -- golden :5642-5646 fMesSystem->asGetRcsCheckingResult / ->GetRcsCheckingResult(true).  fMesSystem has NO home in the ported tree (every other consumer is itself #if 0: cprod.cpp:1259, atester.cpp:1361, csystem.cpp:10139). Offline: RCS result treated as already PASS -> fall through to Task=61, i.e. the MES gate cannot hold the index arm up. Golden's two enclosing IniConfig guards stay ACTIVE above.
                    if(fMesSystem->asGetRcsCheckingResult!="PASS")
                    {
                        if(fMesSystem->GetRcsCheckingResult(true)==false)
                            break;
                    }
#endif
                }

                Task=61;                                                                                                //Steven 20201014 : 修正掉料異常時, index arm要分開
            }
            break;
        case 61:
            if(REAL_TIME_CCD==true &&                                           //----- by dell ccd realtime-------------
               (MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false ||
                MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false))
            {
                ShowIndexMotorError(AnsiString("DoTestYRear61"));
                break;
            }

            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYRear 61"))
            {
                //----- by dell ccd realtime-------------
//                if(REAL_TIME_CCD==true && COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
#if 0 // GATE K1G2 -- golden :5665 COM2->RTC_AlarmType()
                if(REAL_TIME_CCD==true && COM2->RTC_AlarmType())    //wei 20221222 RTC ARM Error
#else
                if(REAL_TIME_CCD==true && false)                        // GATE K1G2 offline: no RTC alarm latched  //wei 20221222 RTC ARM Error
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
            if(bRecIndexDropAlarm2==true)
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
            else
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
            break;
        case 64:
            if(IndexAlarmInArmAway()==true)                                                                             //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
//                if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
#if 0 // GATE K1G2 -- golden :5705 COM2->RTC_AlarmType()
                if(COM2->RTC_AlarmType())       //wei 20221222 RTC ARM Error
#else
                if(false)                        // GATE K1G2 offline: no RTC alarm latched  //wei 20221222 RTC ARM Error
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

#if 0 // GATE K1G1 -- golden :5723 COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm1NG]
                    if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm1NG])           //wei 20221222 RTC ARM Error
#else
                    if(false)                                                        // GATE K1G1 offline: RTC vision never answers  //wei 20221222 RTC ARM Error
#endif
                    {
                        iRTCErrorCount++;
                        if(iRTCErrorCount>1)
                        {
                            if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)     //Steven 20151022 : add for MAXIM
                                bIsTestSitICFallDown=true;

                            if(CUSTOMER_CODE==CC_KYEC_LEE)
                                ShowErrorMessage("WAR0354", 0, MMIndex, bRTCArm2HalfViewError, ErrPart);
                            else
                                ShowErrorMessage("WAR0354", 0, MMIndex, bRTCArm2HalfViewError, __FUNC__);

                            iRTCErrorCount=0;
                        }
                        else
                        {
                            sBufferT.printf("RTC ALARM Arm1 NG Auto Retry:%d", iRTCErrorCount);
                            MyDBIProcess("Message", sBufferT);
                        }
                    }
#if 0 // GATE K1G1 -- golden :5744 COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm2NG]
                    else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm2NG])      //wei 20221222 RTC ARM Error
#else
                    else if(false)                                                   // GATE K1G1 offline: RTC vision never answers  //wei 20221222 RTC ARM Error
#endif
                    {
                        iRTCErrorCount++;
                        if(iRTCErrorCount>1)
                        {
                            if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)     //Steven 20151022 : add for MAXIM
                                bIsTestSitICFallDown=true;

                            if(CUSTOMER_CODE==CC_KYEC_LEE)
                                ShowErrorMessage("WAR0355", 0, MMIndex, bRTCArm2HalfViewError, ErrPart);
                            else
                                ShowErrorMessage("WAR0355", 0, MMIndex, bRTCArm2HalfViewError, __FUNC__);

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
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)                                                //Steven 20151022 : add for MAXIM
                        bIsTestSitICFallDown=true;

                        if(CUSTOMER_CODE==CC_KYEC_LEE)
                            ShowErrorMessage("WAR0342", 0, MMIndex, bRTCArm2HalfViewError, ErrPart);   //RTC Socket Has Device Error
                        else
                    ShowErrorMessage("WAR0342", 0, MMIndex, bRTCArm2HalfViewError, __FUNC__);                           //RTC Socket Has Device Error
                    }
//                    COM2->RTC_ResetAlarm();     //wei 20221222 RTC ARM Error
                    COM2->DoReleaseAndInspEnd();                                                                        //Steven 20120522 : 換位置
                    bRTCArm2HalfViewError=true;                                                                         //Steven 20120206 : RTC重複錯誤
                    if(IniConfig.bD49RTCAlarmSetIndexToErrBin)                                                          //JerryYang 20160712 for 力成,發生RTC Alarm時把Index上所有IC設為Errorbin
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
                    Task=65;                                                                                            //if RealTime NG ---->Next Start FullView
                }
            }
            break;
        case 65:
            if(REAL_TIME_CCD==true &&                                           //----- by dell ccd realtime-------------
               (MOT[MTestZ1].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false ||
                MOT[MTestZ2].Gali_ReadEncoderMaxRandge(Prod.TestZ1_Safe)==false))
            {
                ShowIndexMotorError(AnsiString("DoTestYRear65"));
                break;
            }

            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYRear 65"))
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //Ifor 20190712 need modify
                   TestIF_File.bArm1PickPlaceArm2Test==true     &&
                   TestIF_File.bRTC20CheckFunction==true        &&
                   bPlaceTestIC==true                           )               //Ifor 20190713 : add Arm1 丟下IC
                {
//                    COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC]=false;
//                    COM2->RTC_ResetAlarm();     //wei 20221222 RTC ARM Error
                    COM2->DoReleaseAndInspEnd();
                    bRTCFullViewError=false;                                    //Steven 20120206 : RTC重複錯誤
                    Task=70;
                }
                else
                {
#if 0 // GATE K1G1 -- golden :5819-5820 COM2->bRealTimeCom_ReceiveOK[] write + SendCommToVision(rtFullTOK) (offline no-op)
                    COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]=false;        //----- by dell ccd realtime-------------
                    COM2->SendCommToVision(COM2->rtFullTOK, true);
#endif
                    iWaitIndexArm2.SetSecAndOn(10);
                    Task=66;
                }
            }
            break;
        case 66:
#if 0 // GATE K1G1 -- golden :5827 COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK]
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK])
#else
            if(false)                                                        // GATE K1G1 offline: RTC vision never answers
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
#if 0 // GATE K1G5 -- golden :5837 fContact->InitROILearningTask() (TfContactShim, atester_shims.h:154-290, has no such method; atester.cpp:5537 already carries the same gap as a do{}while(0) macro). Offline no-op.
                    fContact->InitROILearningTask();
#endif
                    Task=68;
                }
                else
                {
                    Task=70;
                }
#if 0 // GATE K1G1 -- golden :5844 COM2->RTC_ResetAlarm() (no such method; offline no-op)
                COM2->RTC_ResetAlarm();     //wei 20221222 RTC ARM Error
#endif
                bRetryRTC=false;
                break;
            }

#if 0 // GATE K1G1 -- golden :5849 COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG])
#else
            if(false)                                                        // GATE K1G1 offline: RTC vision never answers
#endif
            {
                Task=67;
                break;
            }

            if(iWaitIndexArm2.Off())                                            //Steven 20110824 : Real time CCD - 不可以關閉CCD
            {                                                                   //ChungHung 20140520 add 不要第一次TimeOut就秀錯誤訊息
#if 0 // GATE K1G1 -- golden :5857 COM2->OpenRTCComPortAgain() (no such method)
                if(COM2->OpenRTCComPortAgain())                                 //ChungHung 20121005 add
#else
                if(false)                                                       // GATE K1G1 offline: RTC com port is never re-opened  //ChungHung 20121005 add
#endif
                    ShowErrorMessage("WAR0335", 0, MTestY1, 0, __FUNC__);       //RTC Arm1 Error!

                bSendRealCCDSendStart=true;
                bSendRealCCDSendVerify=true;
                COM2->DoReleaseAndInspEnd();                                    //JerryYang 20220923 : add
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
#if 0 // GATE K1G5 -- golden :5882 fContact->InitROILearningTask() (see :5837)
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
#if 0 // GATE K1G6 -- golden :5893 fContact->Do_ROILearning(true): TfContactShim declares only the 0-ARG overload (atester_shims.h:224, real body atester_shims.cpp:237 returns true)
            if(fContact->Do_ROILearning(true))
#else
            if(fContact->Do_ROILearning())                                   // GATE K1G6 offline substitute: the REAL shim body (still linked), bQuickLearn arg dropped
#endif
            {
                Task=70;
            }
            break;
        case 70:
            if(IniConfig.bEnableCCDUSETCPIP)
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
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYRear 72"))
            {
                fMain->lbCCDStatus->Visible=true;
                fMain->LightOn();
                DoTestYRearDelay.SetSecAndOn(3);
                iCCDTimeOutCount=0;
                if(CCDInterfaceForm->bAtestScanCCDProgram)
                {
#if 0 // GATE K1G7 -- golden :5922 CCDInterfaceForm->CloseCCDForm() (TCCDInterfaceFormShim, atester_shims.h:296-303, exposes only bAtestScanCCDProgram/CCDTimerOnOff/CCDRunExec). Offline no-op; the RecordProcess("Stop CCD check.") beside it stays ACTIVE.
                    CCDInterfaceForm->CloseCCDForm();
#endif
                    RecordProcess("Stop CCD check.");
                }
                Task=73;
            }
            break;
        case 73:
            if(DoTestYRearDelay.Off())
            {
#if 0 // GATE K1G7 -- golden :5931-5932 CCDInterfaceForm->CCDIdentificationOpen() / ->CCDIdentification() (no such methods). Offline no-op: no CCD program is launched.
                CCDInterfaceForm->CCDIdentificationOpen();
                CCDInterfaceForm->CCDIdentification();                          //kevin 20110811
#endif
                RecordProcess("Start CCD check.");
                DoTestYRearDelay.SetSecAndOn(20);                               //CCDTimeOutSec);  20110810 設定5秒太短會一直取像
                DoTestYRearDelay2.SetMSAndOn(200);
                Task=75;
            }
            break;
        case 75:
#if 0 // GATE K1G7 -- golden :5940-5941 CCDInterfaceForm->bCCDProgramExistence (no such member)
            if(DoTestYRearDelay2.Off()==false ||
               CCDInterfaceForm->bCCDProgramExistence==false)                   //ChungHung 20121127 add 等待程式開啟
#else
            if(DoTestYRearDelay2.Off()==false ||
               false)                                                        // GATE K1G7 offline: bCCDProgramExistence  //AI(W906-W120) 20261006 (Frank01, TO_FRANK 18:0x W-120 / NBG-Q10): CORRECTED from `true` to `false`, as the front side's PT-W7c did 20260811 (aTester_Front.cpp case 75, its long note). `true` mirrored golden's state (no CCD program offline) but the same gate also removed golden :5946 CCDIdentificationOpen(), so nothing could ever make the program exist: every tick re-armed the 20 s DoTestYRearDelay just above and returned false, Task never left 75 and the timeout arm below (golden's own CCD-timeout exit: iCCDTimeOutCount++, Task=73, "CCD Time out" every >10) was unreachable -- a silent hang. `false` takes that exit. Not reachable on the HT9050 machine (case 70 needs IniConfig.bEnableCCDUSETCPIP && bC02InstallCCD; the machine's LastSet bC02InstallCCD=0); reachable on profiles that turn CCD-TCPIP on (e.g. CUSTOMER_CODE 0). ctest CcdCase75Gate. Same line, no line moves.
#endif
            {
                if(DoTestYRearDelay2.Off()==true)
                {
                    DoTestYRearDelay2.SetMSAndOn(200);
#if 0 // GATE K1G7 -- golden :5946 CCDInterfaceForm->CCDIdentificationOpen() (see :5931)
                    CCDInterfaceForm->CCDIdentificationOpen();
#endif
                    DoTestYRearDelay.SetSecAndOn(20);                           //CCDTimeOutSec);  20110810 設定5秒太短會一直取像
                }
                return false;
            }

#if 0 // GATE K1G7 -- golden :5952-5970 CCDInterfaceForm->bIdentificationFinish / ->iIdentificationStatus (no such members), and inside it TWO further absences: SW[SwCCDLight].Off() (see GATE K1G8) and fShowMessage->FormClick(fShowMessage) (TfShowMessage, forms/fShowMessage.h:22-28, has ShowSpeed only -- GATE K1G9). Offline: identification NEVER completes, matching the shim family's "the vision/CCD subsystem never answers" convention.
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

            if(DoTestYRearDelay.Off())
            {
                iCCDTimeOutCount++;
#if 0 // GATE K1G7 -- golden :5975 CCDInterfaceForm->RetrunUpStep() (no such method; offline no-op)
                CCDInterfaceForm->RetrunUpStep();
#endif
                if(iCCDTimeOutCount>(ChangeToFloatNonPcnt((double)(100), (double)(CCDTimeOutSec))))
                {
#if 0 // GATE K1G7 -- golden :5978 CCDInterfaceForm->CloseCCDForm() (see :5922)
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
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, 50000, "DoTestYRear 7500"))
            {
                RecordProcess("CCD check socket FAIL !!!!!!!!");
                fMain->Pause("CCD check socket fail");
                Task=70;
            }
            break;
        case 76:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, 50000, "DoTestYRear 76"))
            {
                iCCDTimeOutCount=0;
                Task=78;
            }
            break;
        case 78:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 50000, "DoTestYRear 78"))
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
            if(bRecIndexDropAlarm1==true)                                       //JerryYang 20180122 (Steven) index arm掉料要用掉料的arm做index check
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, 50000, "DoTestYRear 80"))
                {
                    Task=10081;
                }
            }
            else
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, 50000, "DoTestYRear 80"))
                {
                    Task=81;
                }
            }
            break;
        case 10081:                                                                                                                                             //JerryYang 20180122 (Steven) index arm掉料要用掉料的arm做index check
            if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset-iIndexArmCheck_SG_Arm1, 50000, 0))                                      //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                IndexStatus=Z1Down_Z2Up;
                if(LastSet.bD41TestSocketICCheckSkip)                                                                                                           //打勾的話就跳過檢查
                {
                    bRecIndexDropAlarm1=false;
                    Task=78;
                    break;
                }

//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE K1G10 -- golden :6046 fiosetview->ResetIndexSuck() (TfiosetviewShim, atester_shims.h:404-410, exposes bIndexSuck[][][]  ONLY). Offline no-op, same as atester_32Site.cpp:207 W5_32S_FIOSET_RESET().
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
                            if(bTestSiteUse[0][i][j]==true)                                                                                                     //jou 20110504 //增加關Site時就不開真空偵測，
                            {
                                flag1=false;                                                                                                                    //jou 20110503 start
                                fiosetview->bIndexSuck[0][i][j]=true;
                                do
                                {
                                    if(flag1==false)
//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE K1G11 -- golden :6065 fiosetview->ProcessIndexSuckDestroy1() (not a TfiosetviewShim member; every other TU stubs it locally: aTester_Front.cpp:163, atester.cpp:5517, atester_32Site.cpp:205, AutoClean.cpp:277)
                                        flag1=fiosetview->ProcessIndexSuckDestroy1();
//AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                                        flag1=true;                             // GATE K1G11 offline: suck self-check "done" -- LOAD-BEARING, this sits inside do{...}while(flag1==false) so false would hang the UI thread forever
//AI(W906-IDXSUCK) （原 #endif）

                                    MySleepEx(1, true);
                                }
                                while(flag1==false);                                                                                                            //jou 20110503 end
                            }
                        }
                    }
                }

                DoTestYRearDelay.SetSecAndOn(0.5);
                Task=10082;
                flag1=false;
            }
            break;
        case 10082:
            if(DoTestYRearDelay.Off() || INDEX_SUCKER_TYPE==1)                  //jou 20110503
            {
                Task=10084;
            }
            break;
        case 10084:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);                                                                                                    //Steven 20140620 : 整合為Function

            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm1, iSpeedFast))                              //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                if(CUSTOMER_CODE==CC_Greatek)                                                                                                                   //Wei 20160413
                    DoTestYRearDelay.SetSecAndOn(5);                                                                                                            //Steven 20110908 : 上來後也要Delay一下
                else
                    DoTestYRearDelay.SetSecAndOn(0.5);                                                                                                          //Steven 20110908 : 上來後也要Delay一下
                Task=10090;
            }
            break;
        case 10090:
            if(DoTestYRearDelay.Off())
            {
                bSucketHasICError=false;                                        //Steven 20101214 : 換位置
                if(LastSet.iRealDummy==REALLY)
                {
                    ErrPart=" ";
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(CheckTestSuckICOn(FTestSuck, i, j))
                            {
                                bSucketHasICError=true;
                                ErrPart+=IndexSuckName[i+iNN][j];
                            }
                        }
                    }
                }
                Task=10095;
            }
            break;
        case 10095:
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
            DoTestYRearDelay.SetSecAndOn(0.05);
            Task=10097;
            break;
        case 10097:
            if(DoTestYRearDelay.Off())
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

                if(bSucketHasICError)                                           // sucket has ic error
                {                                                               //Steven 20101214 : 換位置，換到Delay.Off()裡面
                    Task=50;
                    if(bSocketCheckSkip)
                    {
                        bRecIndexDropAlarm1=false;
                        Task=78;
                    }
                }
                else
                {
                    bRecIndexDropAlarm1=false;
                    Task=78;
                }
            }
            break;
        case 81:
            if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset-iIndexArmCheck_SG_Arm2, 50000, 0))                                      //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                IndexStatus=Z1Up_Z2Down;
                if(LastSet.bD41TestSocketICCheckSkip)                                                                                                           //打勾的話就跳過檢查
                {
                    Task=100;
                    break;
                }

//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE K1G10 -- golden :6176 fiosetview->ResetIndexSuck() (see :6046)
                fiosetview->ResetIndexSuck();
//AI(W906-IDXSUCK) （原 #endif）

                if(TestIF_File.bArm1PickPlaceArm2Test==false ||
                   ((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                     TestIF_File.bArm1PickPlaceArm2Test==true) &&                                                                                               //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
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
                            {                                                                                                                                   //jou 20170516 (Steven) : fix index drop error ,index check arm2失效.
                                if((BTestSuck.Item[i][j]==NULL_IC ||                                                                                            //有個盲點就是如果掉落IC剛好掉到關Site的地方，就會無法知道。
                                    BTestSuck.Item[i][j]==HAS_NULL_IC) &&                                                                                       //Steven 20210105 : 修正有IC的地方不檢查
                                   bTestSiteUse[1][i][j]==true)                                                                                                 //jou 20110504  //增加關Site時就不開真空偵測，
                                {
                                    flag1=false;                                                                                                                //jou 20110503 start
                                    fiosetview->bIndexSuck[1][i][j]=true;
                                    do
                                    {
                                        if(flag1==false)
//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE K1G12 -- golden :6202 fiosetview->ProcessIndexSuckDestroy2() (not a TfiosetviewShim member)
                                           flag1=fiosetview->ProcessIndexSuckDestroy2();
//AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                                           flag1=W64bT2_ProcessIndexSuckDestroy2();   // GATE K1G12 offline substitute: this file's OWN Wave-1 stub (aTester_Rear.cpp:367, default iType=0), exactly as reused at aTester_Rear.cpp:1709
//AI(W906-IDXSUCK) （原 #endif）

                                        MySleepEx(1, true);
                                    }
                                    while(flag1==false);                                                                                                        //jou 20110503 end
                                }
                            }
                        }
                    }
                }

                DoTestYRearDelay.SetSecAndOn(0.5);
                Task=82;
                flag1=false;
            }
            break;
        case 82:
            if(DoTestYRearDelay.Off() || INDEX_SUCKER_TYPE==1)                  //jou 20110503
            {
                Task=84;
            }
            break;
        case 84:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);                                                                                                    //Steven 20140620 : 整合為Function

            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos-iIndexArmCheck_SG_Arm2, iSpeedFast))                              //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
            {
                if(CUSTOMER_CODE==CC_Greatek)                                                                                                                   //Wei 20160413
                    DoTestYRearDelay.SetSecAndOn(5);                                                                                                            //Steven 20110908 : 上來後也要Delay一下
                else
                    DoTestYRearDelay.SetSecAndOn(0.5);                                                                                                          //Steven 20110908 : 上來後也要Delay一下
                Task=90;
            }
            break;
        case 90:
            if(DoTestYRearDelay.Off())
            {
                bSucketHasICError=false;                                        //Steven 20101214
                if(LastSet.iRealDummy==REALLY)
                {
                    ErrPart=" ";
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(CheckTestSuckICOn(BTestSuck, i, j))
                            {
                                bSucketHasICError=true;
                                ErrPart+=IndexSuckName[i][j];
                            }
                        }
                    }
                }
                Task=95;
            }
            break;
        case 95:
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
            DoTestYRearDelay.SetMSAndOn(500);
            Task=97;
            break;
        case 97:
            if(DoTestYRearDelay.Off())
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

                if(bSucketHasICError)                                           // sucket has ic error
                {                                                               //Steven 20101214 : 換位置，換到Delay.Off()裡面
                    Task=50;
                    if(CUSTOMER_CODE==CC_AMKOR_China ||                         //Steven 20101112
                       IniConfig.bKoreaFunction ||
                       CUSTOMER_CODE==CC_QUALCOMM)                              //JerryYang 20170412 (Steven) add QUALCOMM
                    {
                        if(bSocketCheckSkip)
                        {
                            bSocketCheckSkip=false;
                            Task=100;
                        }
                    }
                }
                else
                {
                    Task=100;
                }
            }
            break;
        case 100:
            if(CheckIndexStatus("DoTestYRear100"))
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                   TestIF_File.bArm1PickPlaceArm2Test==true    &&
                    bCheckGiveWay==true                         )
                {
                    Prod.TestY2_Middle      =iBackupTestY2_Middle;
                    Prod.TestZ2_Test        =iBackupTestZ2_Test;
                    Prod.TestZ2_Drop_Offset =iBackupTestZ2_Drop;
                    iBackUpZ2DownPosition   =Prod.TestZ2_Test;                  //JerryYang 20220923 : add
                    bCheckGiveWay=false;
                }

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
                //==> Eastsun 20260511 F007 整合: Ifor 20220308+20230524 KLT 兩段式上升 (Rear 版 TestZ2_Up_Offset)
                if(CUSTOMER_CODE == CC_KYEC_LEE                                    &&
                   bEnable_KLT_Function==true                                      &&
                   DeviceForm.ContactMode==DropContactModeDiffentSpeed             &&
                   Prod.TestZ2_Up_Offset!=0                                        ) //Ifor 20220308 add + 20230524 add 設定不為0才走
                {
                    Task=20000;
                }
                //<== Eastsun 20260511 F007 整合
                else if(IniConfig.bD21EnableFinishTestUpWait &&                 //jou 2011-10-31 start : 丟測模式與不同速度模式也要支援兩段式上升
                   IndexStatus==Z1Up_Z2Down)                                    //Steven 20180613 (Jou) : 增加保護機制, 避免撞機
                {
                    Task=105;
                }
                else
                {
                    Task=108;
                }
                goto DoTestYRear_ReStart;                                       //2008/10/20 lee
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
                iFinishTestUpWaitHeight=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+IniConfig.iD21FinishTestUpWaitHeight;
            }
            else
            {
                iFinishTestUpWaitHeight=Prod.TestZ2_Test+IniConfig.iD21FinishTestUpWaitHeight;
            }

            if(MOT[MTestZ2].Gali_MotMove(iFinishTestUpWaitHeight, 10000))
            {
                Task=106;
                DoTestYRearDelay.SetMSAndOn(IniConfig.iD21FinishTestUpWaitTime);
            }

            if(Task!=106)                                                       //Steven 20180813 : add index arm speed
                break;
        case 106:
            if(DoTestYRearDelay.Off())
            {
                Task=108;
            }

            if(Task!=108)                                                       //Steven 20180813 : add index arm speed
                break;
        case 108:
            if(CUSTOMER_CODE==CC_ASE_KaohSiung_K12 &&
               LastSet.iTemperature==Tempture_Hot &&
               bFirstZ1UPZ2Down &&
               TestIF_File.bUseSocketFloat==false)                              //kevin 20131112 加熱模式預先動作避免第一顆溫度過高
            {
                bCheckShuttle=false;
                Task=110;
            }
            else
            {
                if(MOT[MTestZ1].Gali_ReadPos()!=Prod.TestZ1_Safe)
                {
                    if(IndexStatus==Z1Down_Z2Up)
                        Task=110;
                    else
                        Task=109;
                }
                else
                {
                    if(IndexStatus==Z1Up_Z2Down &&
                       REAL_TIME_CCD==true &&                                   //----- by dell ccd realtime-------------
                       !COM2->bCCDDummyRum &&
                       TestIF_File.bUseSocketFloat==false)                      //Frank QQ
                    {
                        if(bRealCCDSendArm)
                        {
#if 0 // GATE K1G1 -- golden :6416 COM2->SendCommToVision(COM2->rtArmIndex1, true) (offline no-op)
                            COM2->SendCommToVision(COM2->rtArmIndex1, true);
#endif
                            if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                               TestIF_File.bArm1PickPlaceArm2Test==true     &&
                               TestIF_File.bRTC20CheckFunction==true        &&
                               TestIF_File.bRTC20GiveWayCheck==false        )
                            {
#if 0 // GATE K1G1 -- golden :6422 COM2->SendCommToVision(COM2->rtCHECKNULL, true) (offline no-op)
                                COM2->SendCommToVision(COM2->rtCHECKNULL, true);
#endif
                            }
                            iWaitIndexArm2.SetMSAndOn(200);
                            bRealCCDSendArm=false;
                        }
                    }
                    Task=110;
                }
            }
            goto DoTestYRear_ReStart;                                           //2008/10/20 lee
        case 109:
            #ifdef DEBUG_INDEX_UPH
            if(MOT[MTestZ1].Gali_MotMove2(Prod.TestZ1_Safe, iIndexSpeed, iIndexAcc))
            #else
            if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed, 0))
            #endif
            {
                if(IndexStatus==Z1Up_Z2Down &&
                   REAL_TIME_CCD==true &&                                       //----- by dell ccd realtime-------------
                   !COM2->bCCDDummyRum &&
                   TestIF_File.bUseSocketFloat==false)                          //Frank QQ
                {
                    if(bRealCCDSendArm)
                    {
#if 0 // GATE K1G1 -- golden :6446 COM2->SendCommToVision(COM2->rtArmIndex1, true) (offline no-op)
                        COM2->SendCommToVision(COM2->rtArmIndex1, true);
#endif
                        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                           TestIF_File.bArm1PickPlaceArm2Test==true     &&
                           TestIF_File.bRTC20CheckFunction==true        &&
                           TestIF_File.bRTC20GiveWayCheck==false        )
                        {
#if 0 // GATE K1G1 -- golden :6452 COM2->SendCommToVision(COM2->rtCHECKIC, true) (offline no-op)
                            COM2->SendCommToVision(COM2->rtCHECKIC, true);
#endif
                        }
                        iWaitIndexArm2.SetMSAndOn(200);
                        bRealCCDSendArm=false;
                    }
                }

                if(Prod.bIndexUpSpeed)
                    DoTestYRearDelay.SetSec(20);                                //kevin 20190913 add
                Task=110;
            }

            if(Task!=110)                                                       //Steven 20180813 : add index arm speed
                break;
        case 110:                                                               //kevin 20170524 (wei) two speed up
            QueueTaskList[23].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化    //Steven 20201201 : 19 --> 23
            CheckSocketSensor(1, "DoTestYRear_110", true);                      //Z2上升時, 檢查socket sensor
            iAZ1=MOT[MTestY2].Gali_ReadPos();
            iAZ2=abs(Prod.TestY2_Middle-iAZ1);                                  //kevin 2019404 add 測區位置 > 5有問題
            iAZ3=abs(Prod.TestY2_Rear-iAZ1);

            if(Prod.bIndexUpSpeed)
            {
                if(iAZ2<=5)                                                     //kevin 20190404
                {
                    if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+300, MOT[MTestZ2].GailSpeed*Prod.TestZ_Drop_Speed/100))
                    {
                        if(InputShuttleRearHasIC() && InShtInLF(1))             //KaiChen 20171225 (Steven)：Add Speed Display
                            IndexAddSpeedDisplay();
                        else
                            IndexSubSpeedDisplay();
                        Task=115;
                    }
                }
                else if(iAZ3<=5)                                                //kevin 20190404
                {
                    if(InputShuttleRearHasIC() && InShtInLF(1))                 //KaiChen 20171225 (Steven)：Add Speed Display
                        IndexAddSpeedDisplay();
                    else
                        IndexSubSpeedDisplay();
                    Task=115;
                }
            }
            else
            {
                if(InputShuttleRearHasIC() && InShtInLF(1))                     //KaiChen 20171225 (Steven)：Add Speed Display
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
                bZ1DownZ2UpAndPlace=CheckPlaceOutShuttle(1);                    //ChungHung 20171116 modify for Index Action
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
                    if((iAZ2<=5 || iAZ2>5) && iAZ1!=Prod.TestY2_Middle)
                    {
                        sBufferT.sprintf("Arm 2 Prod.TestY2_Middle:%d    ReadPos():%d ", Prod.TestY2_Middle ,iAZ1);
#if 0 // GATE K1G13 -- golden :6529 SaveFile(asHandlePath, sBufferT).  SaveFile itself IS ported (csystem.h:259) -- it is asHandlePath (common.h:116, common.cpp:185) that is UNREACHABLE from this TU: aArmHeader.h keeps its whole include list, common.h included, inside a dead god-header guard (aArmHeader.h:97 "#endif // TODO(W6.x/W7)"). Offline: the index-Y position-mismatch log line is not written.
                        SaveFile(asHandlePath, sBufferT);                       //kevin 20190411 add log
#endif
                    }
                    else if((iAZ3<=5 || iAZ3>5) && (iAZ1!=Prod.TestY2_Rear))
                    {
                        sBufferT.sprintf("Arm 2 Prod.TestY2_Rear:%d    ReadPos():%d ", Prod.TestY2_Rear ,iAZ1);
#if 0 // GATE K1G13 -- golden :6534 SaveFile(asHandlePath, sBufferT) (see :6529)
                        SaveFile(asHandlePath, sBufferT);                       //kevin 20190411 add log
#endif
                    }
                }
            }

// ---- chunk 2/3 of DoTestYRear, golden 6538..7959 ----
        case 115:
            QueueTaskList[23].CheckTaskChange();                                                                                                                //Steven 20200821 : 使用Goto也要記錄Task變化
            if(bGetTime1)
            {
                bGetTime1=false;
                bGetTime2=true;
//                ShowIndexTime(3);                                             //Steven 20140619 : 測試
            }

            str="check socket sensor : ";                                                                                                                       //kevin 20130504 socket sensor
            if(USE_IO_CHANGE_TOQUE==true)                                                                                                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(bCheckShuttle==false)
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
                   (LastSet.iTemperature==Tempture_Hot) && bFirstZ1UPZ2Down)                                                                                    //kevin 20131112 加熱模式預先動作避免第一顆溫度過高
                {
                    bFirstZ1UPZ2Down=false;                                                                                                                     //kevin 20131112 加熱時z1在shuttle 1上面 z2在下
                }
                else if(MOT[MTestZ1].Gali_ReadPos()!=Prod.TestZ1_Safe)
                {
                    if(IndexStatus!=Z1Down_Z2Up)
                    {
                        ShowIndexTime(-2);                                                                                                                      //Steven 20200715 : 重新計算Cycle Time
                        Task=108;
                        break;
                    }
                }
                bCheckShuttle=true;
            }

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                                                                                                            //Frank 20221122 : 2DID sorting for ATK
            {
                if(IndexStatus==Z1_Z2_Normal)
                {
                    HHandler2Gpib.iLotStatus=TestIF_File.iTestMode;                                                                                             //Ifor 20190321 : add Test Mode & ATC Enables Site Send to GPIB
                    #ifdef AMD_Version
                    sprintf(HHandler2Gpib.UseSiteMapData, aSendSiteMapping.c_str());                                                                            //Ifor 20201030 add:送Site Mapping 資料給GPIB
                    #else
                    if(TestIF.iGpibMode==InterfaceType_Delta_Castle)                                                                                            //Steven 20260428 : Delta Castle 強制送SiteMap (取代AMD_Version compile flag)
                    {
                        memset(HHandler2Gpib.UseSiteMapData, '\0', sizeof(HHandler2Gpib.UseSiteMapData));
                        sprintf(HHandler2Gpib.UseSiteMapData, aSendSiteMapping.c_str());
                    }
                    #endif
                    if(DeviceForm.ContactMode==DropContact)
                    {
                        fObserver->AddTimeData(18, DropContactTimer1.LatchCycleTime()/1000.0);                                                                  //JerryYang 20170425 (wei) 第一段時間, 測試完成到另一支arm下降到drop高度
                        DropContactTimer2.LatchCycleTime(true);
                    }

                    if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                                                         //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                        TestIF_File.bArm1PickPlaceArm2Test==true) &&
                        TestIF_File.bCheckArm2Vacuum==true)                                                                                                     //Steven 20150129 : 需要確認Arm2有沒有粘料
                    {
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                BTestSuck.Suck[i][j].Normal();                                                                                                  //關掉吹氣
                            }
                        }
                    }

                    bGetTime1=true;
                    bGetTime2=true;

                    bIndexArm1PickupErrStop=false;
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    bCheckShuttle=false;
                    bOneTimeFlag=true;
                    ShowIndexTime(-2);
                    #if 0 // TODO(G01) AI(W7B-DoTestYRear-c2) 20260810 -- golden :6629 ShowMainScreenPresure(0) -- golden main.h free function, NO body anywhere in the port tree (atester.cpp carries a TU-local W7T1_ShowMainScreenPresure stand-in; this TU has none). Pure main-screen Z1/Z2-encoder UI paint, no state the SM reads back.
                    ShowMainScreenPresure(0);                                                                                                                   //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                    #endif // G01
                    bRearHeadICFallDown=false;
                    Task=200;
                }
            }
            else
            {
                if(IndexStatus==Z1Down_Z2Up)
                {
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    bCheckShuttle=false;
                    bOneTimeFlag=true;
                    #if 0 // TODO(G02) AI(W7B-DoTestYRear-c2) 20260810 -- golden :6642-6644 ShowMainScreenPresure(0) -- same gap as G01. The WHOLE if-statement is gated (not just the call) because golden has no braces here: gating only the call would leave if(...) with no statement.
                    if(USE_ReadIndex_TOQUE==false &&
                       TestIF_File.bEnableReadAndCheckTorque==false)                                                                                            //kevin 20210804 change
                        ShowMainScreenPresure(0);                                                                                                               //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                    #endif // G02
    //                ShowIndexTime(-2);
                    bRearHeadICFallDown=false;
                    ShowIndexTime(-2);                                                                                                                          //Steven 20200715 : 重新計算Cycle Time
                    Task=200;
                    goto DoTestYRear_ReStart;                                                                                                                   //2008/10/20 lee
                }
                else if(IndexStatus==Z1_Z2_Normal)
                {
                    if(TestIF_File.bUseSocketFloat)
                    {
                        ShowIndexTime(-2);                                                                                                                      //Steven 20200715 : 重新計算Cycle Time
                        Task=116;
                        break;
                    }
                    else
                    {
                        lPos[0]=IndexStatus;                                                                                                                    //kevin 20150915
                        RecordIndexPositionError("DoTestYRear()Z1_Z2_Normal", true, false, false, false, &lPos[0]);                                             //kevin 20150915 record
                        ShowIndexMotorError(AnsiString("DoTestYRear115"));
                        return false;
                    }
                }
                else
                {
                    if(IniConfig.bVTESTFunction==true &&                                                                                                        //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
                       IniConfig.bGetRcsCheckingResult==true)
                    {
                        #if 0 // TODO(G03) AI(W7B-DoTestYRear-c2) 20260810 -- golden :6672-6677 fMesSystem->asGetRcsCheckingResult (golden Mes/fVATMesFileSys.h:204) -- fMesSystem is not declared anywhere in the port. Gated arm is unreachable offline anyway: the enclosing guard needs IniConfig.bVTESTFunction && bGetRcsCheckingResult.
                        if(LastSet.iRunStartMode!=rsmAutoSiteMap &&
                           fMesSystem->asGetRcsCheckingResult!="PASS")
                        {
                            Task=50;
                            return false;
                        }
                        #endif // G03
                    }

//                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&    //JerryYang 20241122 : 一丟一測也要偵測drop error
//                       TestIF_File.bArm1PickPlaceArm2Test==true)
//                    {
//                    }
//                    else
                    {                                                                                                                                           //jou 2011-08-16 +100 -> +500 drop mode容易造成誤判
                        iIndexZPos=MOT[MTestZ2].Gali_ReadPos();                                                                                                 //jou 2012-01-17 +500 -> +750 疑似有誤判的情況發生，一直來來回回Hang up (備注 : 750條 = 7.5mm )
                        if((iIndexZPos>(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+750)) ||
                           (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0))                                                                                  //2008/10/20 lee //Ifor 20210716 add:避免關ARM不偵測掉料
                        {
                            flag=false;
                            if(LastSet.iRealDummy==REALLY)
                            {
                                if(DeviceForm_File.dDropByPassDetect!=0 &&
                                   (iIndexZPos>(Prod.TestZ2_Test+DeviceForm_File.dDropByPassDetect*100)))
                                {
                                }
                                else
                                {
                                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                                       TestIF_File.bArm1PickPlaceArm2Test==true &&
                                       bPlaceTestIC==true)                                                                                                      //JerryYang 20241122 : 一丟一測也要偵測drop error
                                    {
                                    }
                                    else
                                    {
                                        for(int i=0; i<BTestSuck.iShtRow; i++)
                                        {
                                            for(int j=0; j<BTestSuck.iShtCol; j++)
                                            {
                                                if(BTestSuck.Suck[i][j].Enable       &&
                                                    BTestSuck.Suck[i][j].SenUsing!="" &&
                                                    BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                                    BTestSuck.Item[i][j]!=NULL_IC)                                                                              //Steven 20110725 : 不再使用IsSuckICFallDown
                                                {
                                                    if(BTestSuck.Suck[i][j].GetStatus()==false)
                                                    {
                                                        BTestSuck.Suck[i][j].Normal();                                                                          //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                                        flag=true;
                                                        bRecIndexDropAlarm2=true;                                                                               //jou 2012-01-17 紀錄index Drop alarm
                                                        //==> Eastsun 20260511 F007 整合: Ifor 20220125 add: Index 掉料當下 馬達位置設定 (Index2 / Rear)
                                                        if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true)
                                                        {
                                                            MOT[MTestZ2].Gali_Command("ST");
                                                            StopAllMotor();
                                                            iArm1_Y=MOT[MTestY1].Gali_ReadPos();
                                                            iArm1_Z=MOT[MTestZ1].Gali_ReadPos();
                                                            iArm2_Y=MOT[MTestY2].Gali_ReadPos();
                                                            iArm2_Z=MOT[MTestZ2].Gali_ReadPos();

                                                            StrPos.sprintf("Index2 %d %d Drop :ARM1_Y_%d_Z_%d_ARM2_Y_%d_Z_%d", i, j, iArm1_Y, iArm1_Z, iArm2_Y, iArm2_Z);
                                                            RecordProcess(StrPos);
                                                        }
                                                        //<== Eastsun 20260511 F007 整合
                                                        }
                                                }

                                                if(FTestSuck.Suck[i][j].Enable       &&                                                                         //jou 2012-04-24 增加IC掉落檢查
                                                    FTestSuck.Suck[i][j].SenUsing!="" &&
                                                    FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                                    FTestSuck.Item[i][j]!=NULL_IC)
                                                    {
                                                        if(FTestSuck.Suck[i][j].W906_GetStatusHeld()==false)  // AI(W906-F03-QUADVAC) 20261004: 群組：4 個都要有真空，掉了要持續 100 ms 才算掉料
                                                        {
                                                            FTestSuck.Suck[i][j].Normal();                                                                      //jou 2012-01-17 直接關掉，避免掉到shuttle去，也避免要掉不掉Hang up
                                                            flag=true;
                                                            bRecIndexDropAlarm1=true;                                                                           //jou 2012-01-17 紀錄index Drop alarm
                                                            //==> Eastsun 20260511 F007 整合: Ifor 20220125 add: Index 掉料當下 馬達位置設定 (Index1 / Rear 對稱記錄)
                                                            if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==true)
                                                            {
                                                                MOT[MTestZ1].Gali_Command("ST");
                                                                StopAllMotor();
                                                                iArm1_Y=MOT[MTestY1].Gali_ReadPos();
                                                                iArm1_Z=MOT[MTestZ1].Gali_ReadPos();
                                                                iArm2_Y=MOT[MTestY2].Gali_ReadPos();
                                                                iArm2_Z=MOT[MTestZ2].Gali_ReadPos();

                                                                StrPos.sprintf("Index1 %d %d Drop :ARM1_Y_%d_Z_%d_ARM2_Y_%d_Z_%d", i, j, iArm1_Y, iArm1_Z, iArm2_Y, iArm2_Z);
                                                                RecordProcess(StrPos);
                                                            }
                                                            //<== Eastsun 20260511 F007 整合
                                                            }
                                                    }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        for(int i=0; i<Gali_MaxAxis; i++)
                        {
                            if(USE_INDEX_ARM_AXES==IndexArm_3_Axis &&                                                                                           //JimmyChiu 20220708 : add Index Arm Axis
                               (MTestY1+i)==MTestY2)
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
                                #if 0 // TODO(G04) AI(W7B-DoTestYRear-c2) 20260810 -- golden :6786-6789 ScanBtnThd->Stop() (golden ScanBtnThread) -- no port declaration. Offline: the RTC scan thread does not exist, so there is nothing to stop.
                                if(REAL_TIME_CCD==true)
                                {
                                    ScanBtnThd->Stop();
                                }
                                #endif // G04
                                Task=50;
                                str.sprintf("Motor:%s, Alarm:%d, ServoAlarm:%d, Servo On:%d", MOT[MTestY1+i].Alias, MOT[MTestY1+i].Led[iAlarmLed], MOT[MTestY1+i].Led[iServoalarmLed], MOT[MTestY1+i].Led[iServoOn]);
                                ShowErrorMessage("WAR0349", 0, MMSystem, 0, str);                                                                               //JerryYang 20240111 : 改成跳alarm code
                                #if 0 // TODO(G22) AI(W7B-DoTestYRear-c2) 20260810 -- golden :6793 fHome->GaliMotorServoOff(AnsiString) (golden uhome.h:77). TfHome DOES exist in the port (forms/fHome.h) but DELIBERATELY omits this member -- forms/fHome.h:30-34 says declaring it would invite retiring the existing seams without translating the uhome.cpp body; csystem.cpp:6417 gates the identical call (W7C2_FHOME_SERVOOFF). SAFETY-RELEVANT REAL-MACHINE DIFFERENCE: on a WAR0349 index-motor servo/alarm fault golden drops the Galil servo while catching the Z brake; offline that servo-off does not happen.
                                fHome->GaliMotorServoOff("DoTestYRear");                                                                                        //Steven 20230712 : 修正SwServoOn.Off時, 要抓住Z煞車
                                #endif // G22
                                return false;
                            }
                        }

                        if(flag)
                        {
                            bRearHeadICFallDown=true;
                            MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:115"));
                            #if 0 // TODO(G05) AI(W7B-DoTestYRear-c2) 20260810 -- golden :6802-6805 ScanBtnThd->Stop() -- same gap as G04.
                            if(REAL_TIME_CCD==true)
                            {
                                ScanBtnThd->Stop();
                            }
                            #endif // G05
                            str1.sprintf("DoTestYRear 115 to 50: bRecIndexDropAlarm1=%d, bRecIndexDropAlarm2=%d", bRecIndexDropAlarm1?1:0, bRecIndexDropAlarm2?1:0);
                            RecordProcess(str1);
                            Task=50;
                            return false;
                        }
                        else
                        {
                            bRecIndexDropAlarm1=false;                                                                                                          //jou 2012-01-17 紀錄index Drop alarm
                            bRecIndexDropAlarm2=false;                                                                                                          //jou 2012-01-17 紀錄index Drop alarm
                        }
                    }

                    if(CheckSocketSensor(1, "DoTestYRear_115_"))                                                                                                //Z1DownZ2Up //Steven 20200615 : Socket Sensor整合成Function
                    {
                        Task=50;
                        return false;
                    }

                    if(IniConfig.bI26TestCloseSiteHaveBin &&
                       bTestBinDataError!=0)                                                                                                                    //kevin 20150202 需強致將arm上 ic取出
                    {
                        Task=50;
                        RecordProcess("[I26] Test Close Site Have Bin Error!", __FUNC__);                                                                       //Steven 20201201 : add event log for debug
                        return false;
                    }

                    if(bOneTimeFlag)
                    {
                        bOneTimeFlag=false;
                        if(TestIF_File.bUseSLKClamp &&
                           TestIF_File.iSeparabilityTest==1 &&
                           TestIF_File.bArm1PickPlaceArm2Test==false)                                                                                           //JerryYang 20180518 (wei) : add 分離式SLK ARM1丟IC ARM2分離CLAMP流程
                        {
                            iBackUpZ1DownPosition=Prod.TestZ1_Test+1500;                                                                                        //JerryYang 20160523 Z1分離高度要上升1500
                        }
                        else
                        {
                            iBackUpZ1DownPosition=Prod.TestZ1_Test;
                        }

                        FTestSeparateSLK(true);                                                                                                                 //JerryYang 20180518 (wei) : 分離流程初始化
                        BTestSeparateSLK(true);
                        iRealCCDSendArmCT=0;                                                                                                                    //jou 2013-08-23 修正RTC 斷訊不會Alarm的錯誤.
                    }

                    #if 0 // TODO(G06) AI(W7B-DoTestYRear-c2) 20260810 -- golden :6851-6886 WHOLE RTC/CCD half-view send-and-wait region. Absent: COM2->SendCommToVision / ->rtArmIndex1 / ->OpenRTCComPortAgain, ScanBtnThd->Start(), fLotInfo->ALed2. BEHAVIOUR-NEUTRAL OFFLINE: the region head requires !COM2->bCCDDummyRum and TCOM2Shim ctor sets bCCDDummyRum=true (atester_shims.cpp:334), so golden would never enter it offline either.
                    if(REAL_TIME_CCD==true &&                                                                                                                   //----- by dell ccd realtime-------------
                       !COM2->bCCDDummyRum &&
                       TestIF_File.bUseSocketFloat==false)
                    {
                        if(bRealCCDSendArm)
                        {
                            COM2->SendCommToVision(COM2->rtArmIndex1, true);
                            iWaitIndexArm2.SetMSAndOn(200);
                            bRealCCDSendArm=false;
                        }

                        if(Sen[SnRealTimeCCDIndexArm].IsOn())
                        {
                            if(iWaitIndexArm2.Off())
                            {
                                bRealCCDSendArm=true;
                                iRealCCDSendArmCT++;                                                                                                            //jou 2013-08-23 修正RTC 斷訊不會Alarm的錯誤.
                                if(iRealCCDSendArmCT>15)
                                {
                                    iRealCCDSendArmCT=0;                                                                                                        //ChungHung 20140520 add 不要第一次TimeOut就秀錯誤訊息
                                    if(COM2->OpenRTCComPortAgain())                                                                                             //ChungHung 20121005 add
                                        ShowErrorMessage("WAR0336", 0, MMIndex, 0, __FUNC__);
                                    else
                                        RecordProcess("WAR0336 auto retry com port.", __FUNC__);                                                                //Steven 20201201 : add event log for debug
                                    bSendRealCCDSendStart=true;                                                                                                 //Sam 20250220 :　修正一直報警 RTC WAR0335 RTC Arm1 Error! 問題
                                    COM2->DoReleaseAndInspEnd();                                                                                                //JerryYang 20220923 : add
                                    bRealCCDSendArm=true;
                                }
                                return false;
                            }
                            return false;
                        }
                        iRealCCDSendArmCT=0;                                                                                                                    //jou 2013-08-23 修正RTC 斷訊不會Alarm的錯誤.
                        fLotInfo->ALed2->Value=true;                                                                                                            //Steven 20110916
                        ScanBtnThd->Start();
                    }
                    #endif // G06
                    //---------------------------------------

                    if(bGetTime2)
                    {
                        bGetTime2=false;
    //                    ShowIndexTime(4);                                     //Steven 20140619 : 測試
                    }

                    #if 0 // TODO(G07) AI(W7B-DoTestYRear-c2) 20260810 -- golden :6895-6970 WHOLE RTC arm-state result region (incl. the #ifdef RTCErrorType arms, which ARE live in this tree -- MachineType.h:60). Absent: ScanBtnThd->GetArmState()/SetArmState(), COM2->bRealTimeCom_ReceiveOK[]/rtAlarHasIC/rtALARMArm1NG/rtALARMArm2NG/rtAlarGrabTimeOut/rtRelease/rtErrorType/SendCommToVision. BEHAVIOUR-NEUTRAL OFFLINE: region head requires COM2->bCCDDummyRum==false, and the shim ctor sets it true.
                    if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)                                                                                              //----- by dell ccd realtime-------------
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

                            if(bOnce==false)                                                                                                                    //JerryYang 20220923 : add RTC timeout
                            {
                                bOnce=true;
                                hFRTCTimeOutDelay.SetSecAndOn(15.0);
                            }

                            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
                            {
                                MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(", case:115"));
                                bRearHeadICFallDown=true;
                                ScanBtnThd->SetArmState(false);
                                iRTCErrorSend=false;                            //JerryYang 20260306 : fix沒有清除flag
                                RecordProcess("RTC alarm has IC", __FUNC__);                                                                                    //Steven 20201201 : add event log for debug
                                Task=50;
                            }
                            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm1NG])      //wei 20221222 RTC ARM Error
                            {
                                MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(", case:115"));
                                ScanBtnThd->SetArmState(false);
                                iRTCErrorSend=false;
                                RecordProcess("RTC alarm Arm 1 NG", __FUNC__);    //Steven 20201201 : add event log for debug
                                Task=50;
                            }
                            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALARMArm2NG])      //wei 20221222 RTC ARM Error
                            {
                                MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(", case:115"));
                                ScanBtnThd->SetArmState(false);
                                iRTCErrorSend=false;
                                RecordProcess("RTC alarm Arm 2 NG", __FUNC__);    //Steven 20201201 : add event log for debug
                                Task=50;
                            }
                            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarGrabTimeOut])
                            {
                                COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarGrabTimeOut]=false;
                                ShowErrorMessage("WAR0341", 0, MMIndex, 0, __FUNC__);                                                                           //RTC Grab TimeOut Error!
                                COM2->SendCommToVision(COM2->rtRelease, false);
                                MySleep(100);
                                ScanBtnThd->SetArmState(false);
                            }
                            else
                            {
                                if(hFRTCTimeOutDelay.Off())                                                                                                     //JerryYang 20220923 : add RTC timeout
                                {
                                    ShowMyMessage("RTC Arm1 half view check timeout error");
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
                    #endif // G07
                    bOnce=false;                                                                                                                                //JerryYang 20220923 : add RTC timeout

                    if(IniConfig.bIndexPickupErrStop==false)
                    {
                        bIndexArm1PickupErrStop=false;
                        bIndexArm2PickupErrStop=false;
                    }

                    if(DeviceForm.ContactMode==TMove ||
                       DeviceForm.ContactMode==TMoveDrop ||                                                                                                     //jou 2012-02-03 新增T Move Drop
                       DeviceForm.ContactMode==TMoveDropSlowContact ||
                       DeviceForm.ContactMode==TMoveSlowContact)                                                                                                //Steven 20160130 : TMove Soft contact
                    {
                        TMode=true;
                    }
                    else
                    {
                        if(IniConfig.bIndexPickupErrStop==true &&
                           bIndexArm1PickupErrStop==true)                                                                                                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                        {
                            TMode=true;
                        }
                        else
                        {
                            TMode=false;
                        }
                    }

                    if(DeviceForm_File.ContactMode==DirectContactSoftEP ||                                                                                      //kevin 20130608 soft contact 到測區上面2mm 開通ep 充氣
                       DeviceForm_File.ContactMode==DropContactSoftEP)                                                                                          //JerryYang 20151202 add DropContactSoftEP
                    {
                        if(MOT[MTestZ1].Gali_ReadPos()<(Prod.TestZ1_Test+1500))                                                                                 //JerryYang 20231205 : SoftEP高度由+1000改為+1500避免撞到guide pin
                        {
//                            bContSoftEpSwitch(0, false);                        //ARM2 先浮動頭充氣    //JerryYang 20151202 false->true
                            EPSwitchOnOff(eEPSwArm2);
                            if(DeviceForm.fAireForce==0)
                                DeviceForm.fAireForce=DeviceForm.dPress;                                                                                        //kevin 20220215 EP 會被設為 0 ，恢復生產重量
                            ADAM_WriteVoltage(DeviceForm.fAireForce);                                                                                           //JerryYang 20210119 修正誤用die force造成錯誤
                        }
                    }

                    if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                                                         //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                        TestIF_File.bArm1PickPlaceArm2Test==true) &&
                        TestIF_File.bCheckArm2Vacuum==true)                                                                                                     //Steven 20150129 : 需要確認Arm2有沒有粘料
                    {
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                BTestSuck.Suck[i][j].Off();                                                                                                     //要把IC吹掉
                            }
                        }
                    }

                    if(bintered2==true)                                                                                                                         //Isaac 20200922 : 紀錄indexArmY encoder值和command值
                    {
                        bintered2=false;
                        RecordIndexPosition(2, 1);                                                                                                              //Isaac 20200922 : 紀錄indexArmY encoder值和command值，Arm2/Socket
                        EncoderTeachingMaxMinCount(2);                                                                                                          //Isaac 20201012 : 每次完成動作，比較紀錄Encoder和Teaching點的差值
                    }

                    #ifdef INDEX_PROTECT_TMOVE
                    if(bOverRangeDoTMode==true && bTriger4Indexhome==false)                                                                                     //Isaac 20201012 : index Y超過範圍，做一次Tmode
                    {
                        bTriger4Indexhome=true;
                        bOverRange4Indexhome=true;
                        TrigerIndexAxisHome();                                                                                                                  //Isaac 20201012 : index Y超過範圍，做一次Tmode，初始化，開始自動校正
                        return false;
                    }
                    #endif

                    ShowIndexTime(-1);                                                                                                                          //Steven 20200715 : 重新計算Cycle Time

                    #ifndef SOFT_SIMULTE
                    if(TorqueUseHPComCard &&
                       TestIF_File.bEnableReadAndCheckTorque)                                                                                                   //KenHsieh 20220317 : 確認有Get value後Test Arm才能上升
                    {
                        #if 0 // TODO(G08) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7048-7049 COM2->bGetValue (golden rs232.h) -- not a TCOM2Shim member. Leaves the TorqueUseHPComCard arm empty; UNREACHABLE OFFLINE because TorqueUseHPComCard=false (cmydef.cpp:3469), so the else arm at golden :7053-7055 does the Z1DownZ2Up call exactly as golden.
                        if(COM2->bGetValue)
                            bIndexFinish=MOT[MTestY1].Z1DownZ2Up(MOT[MTestZ1].GailSpeed, TMode, bIndexArm1PickupErrStop);                                       //JimmyChiu 20220708 : add Index Arm Axis MTestY2 --> MTestY1
                        #endif // G08
                    }
                    else
                    #endif
                    {
                        bIndexFinish=MOT[MTestY1].Z1DownZ2Up(MOT[MTestZ1].GailSpeed, TMode, bIndexArm1PickupErrStop);                                           //JimmyChiu 20220708 : add Index Arm Axis
                    }

                    //==> Eastsun 20260511 F007 整合: Ifor 20220317 add: KYEC 要求 溫度還沒到不執行
                    if(CUSTOMER_CODE==CC_KYEC_LEE                                  &&
                       LastSet.iTemperature==Tempture_Hot                          &&
                       fHeaterOK==false                                            )
                    {
                        bIndexFinish=false;
                    }
                    //<== Eastsun 20260511 F007 整合
                    if(bIndexFinish)
                    {
                        bIndex1Suck=false;                                                                                                                      //kevin 20220105 Index 在下真空建立 pause 不能關閉
                        bintered2=true;
                        if(DeviceForm_File.ContactMode==DirectContactSoftEP ||
                           DeviceForm_File.ContactMode==DropContactSoftEP)                                                                                      //kevin 20220616 add EP //JerryYang 20151202 add DropContactSoftEP
                        {
//                            bContSoftEpSwitch(0, true);                         //ARM2 先浮動頭充氣     //JerryYang 20151202 false->true
                            EPSwitchOnOff(eEPSwBoth);
                            if(DeviceForm.fAireForce==0)
                               DeviceForm.fAireForce=DeviceForm.dPress;                                                                                         //kevin 20220215 EP 會被設為 0 ，恢復生產重量
                            ADAM_WriteVoltage(DeviceForm.fAireForce);                                                                                           //JerryYang 20210119 修正誤用die force造成錯誤
                        }

                        if(ATC_SYSTEM==eNewATCSystem &&                                                                                                         //Ifor 20190321 : add Test Mode & ATC Enables Site Send to GPIB
                           Temperature.bATCActiveCooling==true &&
                           TestIF_File.i2DIDFormat==eAMD)                                                                                                       //JerryYang 20200422 2DID format選項改用下拉選單
                        {
                            for(int i=0; i<MAX_SOCKET_TOTAL; i++)
                            {
                                HHandler2Gpib.Site[i]=bATC_EnablesChannel[i];
                            }
                        }
                        HHandler2Gpib.iLotStatus=TestIF_File.iTestMode;

                        if(ATC_SYSTEM==eNewATCSystem &&
                           Temperature.bATCActiveCooling==true)                                                                                                 //JerryYang 20220815 : send ATC which ARM
                        {
                            #if 0 // TODO(G09) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7093 ATC_InterfaceForm->HandlerArm(0) -- TATC_InterfaceFormShim (acarry_shims.h:109-115) exposes only iATC_MODE_TYPE. Pure notify-ATC-which-arm message, no state this SM reads back. Guard also needs ATC_SYSTEM==eNewATCSystem && Temperature.bATCActiveCooling.
                            ATC_InterfaceForm->HandlerArm(0);
                            #endif // G09
                        }
                        iRTCErrorCount=0;                                       //wei 20221222 RTC ARM Error
                        iWhichArmDown=1;                                                                                                                        //JerryYang 20200316 add SVID 哪支arm下壓在測區
                        #if 0 // TODO(G10) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7097 fContact->ATC_SwitchTjSignal(1,false) (golden cContact.h:603) -- TfContactShim has no such member and cContact.cpp is untranslated. Outbound ATC TJ signal only.
                        fContact->ATC_SwitchTjSignal(1, false);                                                                                                 //Ifor 20210622 add: ATC Switch TJ
                        #endif // G10

                        if(DeviceForm.ContactMode==DropContact)
                        {
                            fObserver->AddTimeData(18, DropContactTimer1.LatchCycleTime()/1000.0);                                                              //JerryYang 20170425 (wei) 第一段時間, 測試完成到另一支arm下降到drop高度
                            DropContactTimer2.LatchCycleTime(true);
                        }

                        if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                                                     //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                            TestIF_File.bArm1PickPlaceArm2Test==true) &&
                            TestIF_File.bCheckArm2Vacuum==true)                                                                                                 //Steven 20150129 : 需要確認Arm2有沒有粘料
                        {
                            for(int i=0; i<BTestSuck.iShtRow; i++)
                            {
                                for(int j=0; j<BTestSuck.iShtCol; j++)
                                {
                                    BTestSuck.Suck[i][j].Normal();                                                                                              //關掉吹氣
                                }
                            }
                        }

                        bGetTime1=true;
                        bGetTime2=true;

                        bIndexArm1PickupErrStop=false;
                        MOT[MInShuttle1].fCanMoveM=true;
                        MOT[MInShuttle2].fCanMoveM=true;
                        bCheckShuttle=false;
                        bOneTimeFlag=true;
                        ShowIndexTime();
                        #if 0 // TODO(G11) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7127 ShowMainScreenPresure(0) -- same gap as G01.
                        ShowMainScreenPresure(0);                                                                                                               //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                        #endif // G11

                        bRearHeadICFallDown=false;
                        Task=200;
                        #if 0 // TODO(G12) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7131-7136 RTC arm-finish notify block. Absent: COM2->SendCommToVision/->rtArmFinish, ScanBtnThd->Stop(). BEHAVIOUR-NEUTRAL OFFLINE (head requires COM2->bCCDDummyRum==false; shim ctor sets it true). NOTE: bRTCArm2HalfViewError=false is gated with it -- it is only read inside the other RTC regions (G07), all gated too.
                        if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)
                        {
                            bRTCArm2HalfViewError=false;                                                                                                        //Steven 20120206 : RTC重複錯誤
                            COM2->SendCommToVision(COM2->rtArmFinish, false);
                            ScanBtnThd->Stop();
                        }
                        #endif // G12

                        if(IniConfig.bTModeMotorFree==true)
                        {
                            if(TMode==true)                                                                                                                     //20110923 Tmode 煞車
                            {
                                hTestZ1Delay.SetSecAndOn(0.1);
                                Task=130;
                            }
                        }

                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                                                                     //JerryYang 20170523 (wei) ReadEPData會造成index cycle time增加
                        {
                            bReadEpTime=true;                                                                                                                   //kevin 20170524 add ep read change time
                        }

                        if(ATC_SYSTEM==eATCHonPrecType &&
                           ATC_SYSTEM==eATCHonPrecType)                                                                                                         //Dell 20140509
                        {
                            // GATE (W7b-I1): golden aTester_Rear.cpp -- ATCInterfaceForm->ATC_SYS /
                            //   ->ATC_60_SYS. The FORM object is linkable (ATC/ATCInterface.cpp:207,
                            //   ht9045_sm), but every ATCSystem/ATC60System METHOD it forwards to lives in
                            //   ATC/ATCSystem.cpp, which CMakeLists.txt:1332 places in ht9045_comms -- and
                            //   target_link_libraries(ht9045_sm ...) does NOT list ht9045_comms, so 25
                            //   ATCSystem::*/ATC60System::* symbols came back undefined (measured 20260810:
                            //   build rc=2, 0 multiple definition, 25 distinct undefined). Same shape and same
                            //   resolution as GATE (W7a-I3), which gated InitPLCIO for exactly this archive
                            //   boundary. BEHAVIOUR: neutral offline -- both call sites are already inside
                            //   runtime ATC_SYSTEM guards and there is no ATC controller on the wire. RETIRES
                            //   WHEN: ht9045_sm gains the comms edge, or ATCSystem.cpp moves. NOTE: reading
                            //   CMakeLists fully this time, its three "cycle" comments (:1286, :1308, :1482)
                            //   are about making Interface/InterfaceSYS.cpp a separate library and about
                            //   ht9045_motor -- none of them actually forbids an sm->comms edge, so that option
                            //   is open and merely wider than this wave needs. AI(pt-wave) 20260810
                            #if 0 // GATE (W7b-I1)
                            ATCInterfaceForm->ATC_SYS.SetNowArm(0);                                                                                             //ATC
                            #endif // GATE (W7b-I1)
                        }

                        if(ATC_SYSTEM==eATC60 || ATC_SYSTEM==eATC30)                                                                                            //20141204 ChungHung add for ATC3.0 //2014-05-30    Dell    for ATC6.0
                        {
                            #if 0 // GATE (W7b-I1)
                            ATCInterfaceForm->ATC_60_SYS.SetHandlerNowArm(0);
                            #endif // GATE (W7b-I1)
                        }

                        #ifdef ASE_KaohSiung
                            if(TestIF_File.bEnableReadAndCheckTorque==true &&                                                                                   //KaiHuang 20201222 : Index Check 後,用第一次 Contact 的扭力值當標準
                               FTestSuck.UseSiteHasIC()==true)                                                                                                  //kevin 20210804 change
                            {
                                iReadTorqueError=0;
                                fMain->edTorue0->Text="";
                                bReadArm1_Torque=true;
                                hTorqueDelay.SetSecAndOn(TestIF_File.dReadTorqueDelayTime);                                                                     //kevin 20210804 等待時間去讀取扭力 change by setup
                            }
                        #endif
                        goto DoTestYRear_ReStart;                                                                                                               //2008/10/20 lee
                    }
                }
            }
            break;
        case 130:
            if(hTestZ1Delay.Off())
            {
                SW[SwFMotorBreaker].Off();
                Task=200;
                goto DoTestYRear_ReStart;
            }
            break;
        case 140:
            if(hTestZ1Delay.Off())
            {
                MOT[MTestZ1].ServoOnOff(false);
                Task=200;
                goto DoTestYRear_ReStart;
            }
            break;
        case 200:
            if(DeviceForm_File.ContactMode==DirectContactSoftEP ||
               DeviceForm_File.ContactMode==DropContactSoftEP)                  //kevin 20130608 Soft Contact mode
            {
                SoftContactTim.SetMSAndOn(500);
            }

            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].On();
            }

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                            //JerryYang 20230322 : 2D SORT模式index arm不用下壓到socket
            {
                Task=2091;
                break;
            }

            if(Prod.TestZ1_Drop_Offset!=0)                                      //DropContact or DirectContactModeDiffentSpeed
            {
                Task=300;
                break;
            }

            if(IniConfig.bIndexArm2SupplyLight==true ||                         //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
               TestIF_File.bForEgisTecTest==true     ||                         //Steven 20140922 : Arm2當作指紋測試
               (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                 //kevin 20150127 Arm1 下壓 arm2 測試
                TestIF_File.bArm1PickPlaceArm2Test==true))                      //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
            {
                Task=300;
            }
            else
            {
                if(USE_ReadIndex_TOQUE &&
                   TestIF_File.bEnableReadAndCheckTorque)                       //kevin 20210804 change
                {
                    bNeedCheckIndexToque=false;
                    bNeedCheckIndexToque1=false;
                    W64bT2_FMAIN_CHKREADTORQUE1->Checked=true;                                                                                               // AI(W7B-DoTestYRear-c2) 20260810: golden :7235 fMain->chkReadTorque1->Checked=true -- routed through this file's existing W64bT2_TfMainTorqueSeam
                    W64bT2_FMAIN_CHKREADTORQUE2->Checked=false;                                                                                              // AI(W7B-DoTestYRear-c2) 20260810: golden :7236 fMain->chkReadTorque2->Checked=false -- same seam
                    #if 0 // TODO(G13) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7237 fMain->edTorue0 (golden main.h) -- this file's W64bT2_TfMainTorqueSeam (aTester_Rear.cpp:1019-1031) carries chkReadTorque1/2 + edTorue1 but NOT edTorue0, and the seam is file-scope so a mid-function chunk cannot extend it. Pure UI text clear.
                    fMain->edTorue0->Text="";
                    #endif // G13
                }
                else if(TestIF_File.bEnableReadAndCheckTorque)                  //kevin 20210804 change
                {
                    bNeedCheckIndexToque=false;
                    bNeedCheckIndexToque1=false;
                    W64bT2_FMAIN_CHKREADTORQUE1->Checked=true;                                                                                               // AI(W7B-DoTestYRear-c2) 20260810: golden :7243 fMain->chkReadTorque1->Checked=true -- same seam
                    W64bT2_FMAIN_CHKREADTORQUE2->Checked=false;                                                                                              // AI(W7B-DoTestYRear-c2) 20260810: golden :7244 fMain->chkReadTorque2->Checked=false -- same seam
                    #if 0 // TODO(G14) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7245 fMain->edTorue0 -- same gap as G13.
                    fMain->edTorue0->Text="";
                    #endif // G14
                    bRetryReadToqu=false;                                       //kevin 20210419 重讀扭力
                }

                bReadTorqueOK=false;                                            //kevin 20211129 read torque ok
                Task=2091;
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                           //Steven 20150304 : Add GPIB LOG
            }
            goto DoTestYRear_ReStart;                                           //Steven 20180813 : add index arm speed
//            break;
        case 209:
            QueueTaskList[23].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化
            if(DeviceForm_File.ContactMode==DropContactSoftEP)
            {
//                bContSoftEpSwitch(0, true);                                     //ARM1 先浮動頭充氣  //JerryYang 20151202 flase->true
                EPSwitchOnOff(eEPSwBoth);
                SoftContactTim.SetMSAndOn(500);
            }
            Task=2091;
//            break;                                                            //Steven 20180813 : add index arm speed
        case 2091:
            QueueTaskList[23].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化

            if(CUSTOMER_CODE==CC_KYEC_CHEN &&
               DeviceForm.VacuumMode==VacuumOFFMode)                            //jou 2015-07-22 新增Index Vacuum Off mode
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        FTestSuck.Suck[i][j].Normal();
                    }
                }
            }

            if(DeviceForm_File.ContactMode==DirectContactSoftEP ||              //kevin 20130608 Soft Contact mode
               DeviceForm_File.ContactMode==DropContactSoftEP)
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
        case 2092:
            QueueTaskList[23].CheckTaskChange();                                //Steven 20200821 : 使用Goto也要記錄Task變化
            if(bNeedUpDonwOneTome==true)
            {
                bNeedUpDonwOneTome=false;
                Task=1000;
                break;
            }

            HangTime.SetSecAndOn(Prod.iHangupMaxTime);                          //Steven 20090827 Start: Hang Up dectector
            bHangTimePause=false;

            if(CosFunction.bAfterInitialDelayUseOtherArm &&                     //JerryYang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
               Prod.bUseOtherArmToTestAfterInitialDelay &&
               bArm1IsTest==false && bFirst==true)                              //JerryYang 20180817 (Steven) : fix EOT delay
            {
                if(fFrontNeedTest==true && FTestSuck.HasRealIC())               //JerryYang 20220923 : 修正換arm預熱問題
                {
                    bFirst=false;                                               //JerryYang 20180817 (Steven) : fix EOT delay
                    if(bArm1Delay==false && bArm2Delay==false)                  //JerryYang 20200909 修正預熱功能,Arm1下壓->Arm2預熱-> Arm 1測試
                    {
                        CheckInitialStartDelayInSocket();
                        if(bNeedInitialTestDelay==true)
                        {
                            if(InArmSuck.HasIC()            ||
                               BLCarryKit.UseSiteHasIC()   ||
                               //AI(W906-W2-A46) 20260926: gate LIFTED -- A4-6 (8ff6c754) merged TMyKitSuck/TMySucker into the one golden layout (mykitsuck.h), premise dead: TMyKitSuck::HasNotTestYet() is mykitsuck.h:404 (nm T); the 5-term OR is whole again -- golden :7318
                               BTestSuck.HasNotTestYet()    ||
                               //AI(W906-W2-A46) 20260926: (end of lifted gate)
                               bInArmWaitOneCycle==false    ||                  //JerryYang 20220923 : 修正換arm預熱問題
                              (iOneCycle==0 &&
                               (MOT[MMPlate1].HasIC() ||
                                MOT[MMPlate2].HasIC())))                        //Sam 20240911 : 避免 HotPlate 還在預熱時就直接測試
                            {
                                bArm2Delay=true;
                            }
                        }
                    }
                    else
                    {
                        if(bNeedInitialTestDelay==true)
                        {
                            if(bArm1Delay==true)
                            {
                                dwStartInitialCount.LatchCycleTime(true);
                            }
                        }
                    }
                }
            }

            BTorqueTimeOutDelay.Set0_1SecAndOn(50);                             //kevin 20210823 扭力 time
            Task=210;
        case 210:                                                                                                       // 編號不得改變 *******************
            QueueTaskList[23].CheckTaskChange();                                                                        //Steven 20200821 : 使用Goto也要記錄Task變化
            if(bHangTimePause==true || bRunAutoClean)                                                                   //Steven 20090827 Start: Hang Up dectector DoTestYRear
            {
                Task=209;
                break;
            }

            if(DeviceForm.ContactMode==DropContact &&
               CosFunction.bSuckDevicesDuringTest==true &&                                                              //JerryYang 20170804 (Steven) 移除邊吸邊測的選項
               INDEX_SUCKER_TYPE==1)                                                                                    //JerryYang 20170522 (wei) drop contact改為邊吸邊測
            {
                //AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：TODO(G16) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7355 fiosetview->ProcessIndexSuckDestroy1() -- TfiosetviewShim (atester_shims.h) exposes bIndexSuck[][][] but not this method. The RETURN VALUE IS LOAD-BEARING (bCheckSuck gates the "index cycle finished" test at golden :7512-7517), so an #if 0 alone would leave bCheckSuck false forever and STALL the state machine. Offline default true = "suck self-check reports done", identical to atester.cpp's W7T1_ProcessIndexSuckDestroy1 and to this file's W64bT2_ProcessIndexSuckDestroy2.
                bCheckSuck=fiosetview->ProcessIndexSuckDestroy1();
                //AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                bCheckSuck=true;                                                                                                                                // AI(W7B-DoTestYRear-c2) 20260810: offline default -- suck self-check reports done (see gate note above)
                //AI(W906-IDXSUCK) （原 #endif）
            }
            else
            {
                bCheckSuck=true;
            }

            if(fFrontNeedTest)
            {
                if(TestIF_File.bForEgisTecTest==true && bSwitchArm2)                                                    //Steven 20140922 : Arm2當作指紋測試
                {
                    Task=11000;
                    bSwitchArm2=false;
                    break;
                }

                if(CosFunction.bAfterInitialDelayUseOtherArm &&                                                         //JerryYang 20180607 (wei) : Initail delay後不直接測試,換用另外一支arm下去測,用意是避免device溫度被socket帶走後直接測試容易fail
                   Prod.bUseOtherArmToTestAfterInitialDelay &&
                   bArm1IsTest==false)
                {
                    if(bNeedInitialTestDelay==true)
                    {
                        if(bArm1Delay==true)                                                                            //JerryYang 20200909 修正預熱功能,Arm1下壓->Arm2預熱-> Arm 1測試
                        {
                            if(fRearNeedDestroy)                                                                        //JerryYang 20190626 預熱時另一支arm要能吸放IC
                            {
                                bZ1PickShuttle=false;                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                                bZ2PickShuttle=false;                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                                fRearNeedDestroy=!DoRearTestDestroyIC(true);                                            //ChungHung 20111115
                            }
                            else if(fRearNeedSuck)
                            {
                                if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                                   (LastSet.iTemperature==Tempture_Hot ||
                                    LastSet.iTemperature==Tempture_AmbientHot) &&                                       //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
                                   DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)                              //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
                                {
                                    if(fFrontNeedTest==false)
                                        fRearNeedSuck=!DoRearTestSuckIC();
                                }
                                else if(DeviceForm.bSuckShuttleDeviceAfterTested &&                                     //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
                                        (LastSet.iTemperature==Tempture_Ambient ||
                                         LastSet.iTemperature==Tempture_AmbientHot) &&                                  //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
                                        CUSTOMER_CODE==CC_KYEC_LEE &&
                                        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)
                                {
                                    if(fFrontNeedTest==false)
                                        fRearNeedSuck=!DoRearTestSuckIC();
                                }
                                else
                                {
                                    fRearNeedSuck=!DoRearTestSuckIC();
                                }
                            }

                            if(iInitialCount>0)
                            {
                                iInitialCount=(Prod.iInitialDelay)-(dwStartInitialCount.LatchCycleTime()/1000);
                                break;
                            }

                            str.sprintf("Arm 1 intial delay finish, device: %d ea", FTestSuck.CountRealIC());
                            RecordProcess(str);
                            bArm1Delay=false;
                            fFrontNeedTest=false;
                            bNeedInitialTestDelay=false;
                            bFirstTest=true;                                                                            //JerryYang 20180817 (Steven) : fix EOT delay
                            break;
                        }
                        else if(bArm2Delay==true)                                                                       //換arm 2再做預熱
                        {
                            fFrontNeedTest=false;
                            break;
                        }
                    }
                }

                if(TestIF.bUseSLKClamp==true &&
                   TestIF_File.iSeparabilityTest==1 &&
                   bSLKClampProcessFinish==false)                                                                       //JerryYang 20160524 分離模式
                {
                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                  //JerryYang 20180518 (wei) : 分離式SLK ARM1丟IC ARM2分離CLAMP流程
                       TestIF_File.bArm1PickPlaceArm2Test==true)                                                        //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    {
                        if(FTestSeparateSLK()==true)                                                                    //JerryYang 20160429 (wei) 分離模式, Z2 SLK與Clamp分離流程
                        {
                            bSLKClampProcessFinish=true;
                            bArm2NeedCombine=true;
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        if(BTestSeparateSLK()==true)                                                                    //JerryYang 20160429 (wei) 分離模式, Z1 SLK與Clamp分離流程
                        {
                            bSLKClampProcessFinish=true;
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                bArm1IsTest=true;                                                                                       //JerryYang 20180629 (wei) : 用來判斷是否在測試中
                fFrontNeedTest=!DoFTestSuckTestIC();                                                                    //做測試頭下降至測試點測試動作
                if(bCheckSuck==true)                                                                                    //JerryYang 20170522 drop contact改為邊吸邊測)  //JerryYang 20170522 drop contact改為邊吸邊測
                {
                    CheckIndexAllSuckICFallDown(true, false);                                                           //Steven 20110725 : 修改負壓檢查方式
                }
            }
            else                                                                                                        //JerryYang 20200909 修正預熱功能,Arm1下壓->Arm2預熱-> Arm 1測試
            {
                bArm1Delay=false;
            }

            if(fRearNeedDestroy)
            {
                bZ1PickShuttle=false;                                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                bZ2PickShuttle=false;                                                                                   //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                fRearNeedDestroy=!DoRearTestDestroyIC(true);                                                            //ChungHung 20111115
            }
            else if(fRearNeedSuck)
            {
                if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                   (LastSet.iTemperature==Tempture_Hot ||
                    LastSet.iTemperature==Tempture_AmbientHot) &&                                                       //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
                   DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)                                              //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
                {
                    if(fFrontNeedTest==false)
                        fRearNeedSuck=!DoRearTestSuckIC();
                }
                else if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                        (LastSet.iTemperature==Tempture_Ambient ||
                         LastSet.iTemperature==Tempture_AmbientHot) &&                                                  //JerryYang 20181214 : non hotplate mode也要支援溫度優先功能
                        CUSTOMER_CODE==CC_KYEC_LEE &&                                                                   //KaiChen 20180125 (Steven) ：京元-竹南 Suck Shuttle Device After Tested 功能，開啟常溫使用
                        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)                                         //Ifor 20180606 (wei) : add Index 再Shuttle 上等待測試結果
                {
                    if(fFrontNeedTest==false)
                        fRearNeedSuck=!DoRearTestSuckIC();
                }
                else
                {
                    fRearNeedSuck=!DoRearTestSuckIC();
                }
            }

            if(DeviceForm.ContactMode==DropContact &&
               CosFunction.bSuckDevicesDuringTest==true &&                                                              //JerryYang 20170522 drop contact改為邊吸邊測
               bCheckSuck==true)                                                                                        //JerryYang 20170804 (Steven) 移除邊吸邊測的選項
            {
                bFTestSuckDrop=false;
            }

            if(fRearNeedDestroy==false &&
               fRearNeedSuck==false &&
               fFrontNeedTest==false &&
               fFrontNeedSuckIC==false &&
               bCheckSuck==true)                                                                                        //JerryYang 20170522 (wei) drop contact改為邊吸邊測
            {
                bEPfirst=true;                                                                                          //Ifor 20150803 : 新增EP量測旗標，第一次才執行Log紀錄
                bFirst=true;                                                                                            //JerryYang 20180817 (Steven) : fix EOT delay
                bSLKClampProcessFinish=false;
                bArm1IsTest=false;                                                                                      //JerryYang 20180629 (wei) : 用來判斷是否在測試中
                iWhichArmDown=0;                                                                                        //JerryYang 20200316 add SVID 哪支arm下壓在測區
                if(USE_IO_CHANGE_TOQUE==true)                                                                           //jou 2012-06-21 Enable index I/O Change Toque
                {
                    SW[SwIndexChangeToque1].Off();
                    SW[SwIndexChangeToque2].Off();
                }

                if(IniConfig.bTModeMotorFree==true)
                {
                    SW[SwFMotorBreaker].On();                                                                           //20110923 Tmode 煞車
                    MySleep(200);
                }

                iFlag=CheckAnyCaseNeedToDoArm2();                                                                       //Steven 20190115 : SCC要求吸取異常要檢查Socket
                if(iFlag>1)
                {
                    Task=iFlag;
                    if(iFlag==211 || iFlag==215 || iFlag==11040)                                                        //Ifor 20191119 :Fix Arm1 Pick/place Arm2 測試後Arm1不吸料問題
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(FTestSuck.Item[i][j]!=NULL_IC &&
                                   FTestSuck.Item[i][j]!=HAS_NULL_IC)
                                {
                                    FTestSuck.Suck[i][j].Error=false;
                                    bFTestSuckUse[i][j]=true;
                                }
                                bFTSuckFinish[i][j]=false;                                                              //Steven 20110301
                            }
                        }
                    }
                }
                else if(iFlag==1)
                {
                    if(USE_ReadIndex_TOQUE &&
                       TestIF_File.bEnableReadAndCheckTorque)                                                           //kevin 20210804 change
                    {
                        #if 0 // TODO(G17) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7561 ShowMainScreenPresure(0) -- same gap as G01.
                        ShowMainScreenPresure(0);                                                                       //kevin 20201027  //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                        #endif // G17
                        CheckAndRecodrTorque(0);
                    }

                    #ifdef ASE_KaohSiung
                        if( TestIF_File.bEnableReadAndCheckTorque)                                                      //kevin 20210804 change
                        {
                            if(TestIF_File.iShuttleMode==1 &&
                               TestIF_File.iShuttle_Sel==1)                                                             //kevin 20210304
                            {
                                return true;
                            }
                            else
                            {
                                if(NewCheckAndRecodeTorque(0)==false)
                                {
                                    if(bRetryReadToqu==false)                                                           //kevin 20210419 重讀扭力
                                    {
                                        bRetryReadToqu=true;
                                        bNeedCheckIndexToque=false;                                                     //KEVIN 20210419 ADD TORQUE
                                        bNeedCheckIndexToque1=false;
                                        fMain->chkReadTorque1->Checked=true;
                                        fMain->chkReadTorque2->Checked=false;
                                        COM2->ReadIndexTorqueSetting(0);
                                    }

                                    if(BTorqueTimeOutDelay.Off()==false)                                                //kevin 2021018 扭力
                                       return false;
                                }
                            }
                        }
                        ShowMainScreenPresure(0);
                    #endif
                    bIndex1Suck =false;                                                                                 //kevin 20220105 Index 在下真空建立 pause 不能關閉
                    return true;
                }
            }

            if(IniConfig.bRecordSkipPosition)                                                                           //jou 20150320   auto skip 次數時間導致Hang up
            {
                if(bAutoSkipFlag==true && ArmSpeed[InArm].bAutoSKIP==true)
                {
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    bAutoSkipFlag=false;
                }
            }

            //#if 0 // TODO(G18) AI(W7B-DoTestYRear-c2) 20260810 -- golden :7608-7616 EnableTraymapCheckFunction(int) (golden ainarm2.h:216, body ainarm2.cpp:3585) -- no port body. Offline: condition reads false, so the auto-skip HangTime re-arm at golden :7613 does not happen (golden :7599-7605 bRecordSkipPosition path still does). REAL-MACHINE DIFFERENCE: a traymap-check-driven auto-skip could trip the hang-up watchdog earlier than golden.   //AI(W906-S09-DEC) 20260930 (laptop OK 20260930 15:25): gate retired -- EnableTraymapCheckFunction is defined (ainarm2.cpp:2631) -- the reason on this line is STALE; twin of ainarm9045.cpp:11678 (k6-G4); body = golden 906_0625_Steven aTester_Rear.cpp:7608-7616
            if(EnableTraymapCheckFunction(0) &&
               EnableTraymapCheckFunction(1))                                                                           //JerryYang 20250120 : fix誤發hang up
            {
                if(bAutoSkipFlag==true)
                {
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    bAutoSkipFlag=false;
                }
            }
            //#endif // G18   //AI(W906-S09-DEC) 20260930: see :9793

            if(bAskStopPort[ePortLoader] ||                                                                             //JerryYang 20250529 : avoid hang up
               bAskStopPort[ePortAuto1] ||
               bAskStopPort[ePortAuto2] ||
               bAskStopPort[ePortAuto3] ||
               bWaitingAMR==true)                                                                                       //RogerYang 20250617 Load已清空但還沒滿bundle，等料車來
            {
                HangTime.SetSecAndOn(Prod.iHangupMaxTime);
            }

            if(HangTime.Off() &&
               TestISTimeOut==false &&
               bHangTimePause==false)
            {
                if(AMR.NeedAMRTransport())                                                                              //Sam 20250423 : AMR 搬運花費時間太久可能會報警需要 By Pass
                {
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    break;
                }

                if(IniConfig.bTModeMotorFree==true)
                {
                    SW[SwFMotorBreaker].On();                                                                           //20110923 Tmode 煞車
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
                RecordProcess("Auto State Record by DoTestYRear");
                fMain->DoStateRecord(0, false);                                                                         //Steven 20120705 : Hang Up時，會自動存畫面     //KenHsieh 20230116 : 區分手動或自動(sbclick -> Function)

                #ifdef DEBUG_HANGUP_NO_HOME
                    ShowErrorMessage("JAM0317", K_SKIP, MTestZ2);
                    Task=209;
                #else
                    iHangupCTArm2++;
                    if(iHangupCTArm2>1)
                    {
                        iHangupCTArm2=0;
                        ShowErrorMessage("JAM0317", K_SKIP, MTestZ2);
                        Task=209;
                    }
                    else
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                             //kevin 20160722 ASE 高雄取消
                        {
                            ShowErrorMessage("JAM0317", K_SKIP, MTestZ1);
                            Task=209;
                        }
                        else
                        {
                            MyDBIProcessNew("Motion", "WAR2206", "Auto homing", "0317");                                //Steven 20120705 : Hang Up時，會自動存畫面
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
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Pick, MOT[MTestZ1].GailSpeed))                                     //Z1 上升至安全位置
            {
                Task=212;
            }
            else
            {
                break;
            }
        case 212:                                                               //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestY1].Gali_MotMove(Prod.TestY1_Middle, MOT[MTestY1].GailSpeed))
            {
                Task=213;
            }
            else
            {
                break;
            }
        case 213:                                                               //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, MOT[MTestZ1].GailSpeed))
            {
                Task=215;
            }
            else
            {
                break;
            }
        case 215:
            bCheckAllSuck=true;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTSuckFinish[i][j]==false)                              //Steven 20110301
                    {
                        if(bFTestSuckUse[i][j])
                        {
                            if(FTestSuck.Suck[i][j].Suck())
                            {
                                bFTSuckFinish[i][j]=true;                       //Steven 20110301
                                bFTestSuckUse[i][j]=false;
                            }
                            else if(FTestSuck.Suck[i][j].Error)                 //jou 2011-08-16
                            {
                                bFTSuckFinish[i][j]=true;                       //Steven 20110301
                            }
                            else
                            {
                                bCheckAllSuck=false;
                            }
                        }
                        else
                        {
                            bFTSuckFinish[i][j]=true;                           //Steven 20110301
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

            if(flag)                                                            //Steven 20110301
            {
                if(bCheckAllSuck)
                {
                    bHasICinSocket=false;
                    return true;
                }
            }
            break;
        case 220:
            DoTestYRearDelay.SetMSAndOn(OverEncoderDelay);
            Task=230;
//             break;
        case 230:
            if(DoTestYRearDelay.Off())
            {
                if(TestZ1OutRandge())
                {
                    DoTestYRearDelay.SetMSAndOn(1);
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
            if(DoTestYRearDelay.Off())
            {
                if(TestZ1OutRandge2())
                {
                    DoTestYRearDelay.SetSecAndOn(1);
                    Task=240;
                    iRetry++;
                    if(iRetry<=10)
                        break;
                    else
                        ShowMyMessage("Test Head Ecncoder error, maybe press too low!", "Index錯誤，可能是壓力太低!!", "DoTestYRear 240");
                }
                TestZ1SetPos();
                Task=1;
                return true;
            }
            break;
        case 300:
            if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||         //Eliot 2011_0318 Start
               DeviceForm.ContactMode==DirectContactSoftEP ||                   //Steven 20160530
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
            }
            else
            {
                bFTestSuckDrop=true;
                fFrontNeedSuckIC=false;                                         //jou 2011-12-13 丟測才需要設成true

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                           FTestSuck.Item[i][j]==HAS_IC)
                        {
                            FTestSuck.Suck[i][j].Off();

                            if(IniConfig.bD44CheckIndexICDestroy)               //JerryYang 20241122 : 啟用D44功能時，INDEX ARM丟料後會持續吹氣，一直到INDEX ARM上抬到丟料高度
                            {
                                //AI(W906-W2-A46) 20260926: gate LIFTED -- A4-6 (8ff6c754) merged TMyKitSuck/TMySucker into the one golden layout (mykitsuck.h), premise dead: bNeedCheck is mykitsuck.h:303 -- golden :7828
                                FTestSuck.bNeedCheck[i][j]=true;                //Destroy finish
                                //AI(W906-W2-A46) 20260926: (end of lifted gate)
                                iIndexTakeDeviceChk1=2;                         //kevin 20190103 回吸檢測狀態
                            }
                        }
                    }
                }
            }

            if(CosFunction.bTesterSidePushFunction==true &&                     //Richard 20220321 : 渠梁Side Push
               DeviceForm.bTesterSidePush==true)
            {
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

            if(TestIF_File.iShuttleMode==0 ||
               (TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) ||
               (IniConfig.bD58UseArm1PickPlaceArm2Test==true && TestIF_File.bArm1PickPlaceArm2Test==true))  //JerryYang 20260508 : fix 一丟一測沒有delay
            {
                DoTestYRearDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);             // delay 0.3 sec for ic down        //Steven 20140909 : 換到迴圈外面
            }
            else
            {
                DoTestYRearDelay.SetSecAndOn(0);
            }

            Task=310;
            break;
        case 310:                                                                                                       //丟測放下IC
            bCheckDestroy=true;
            if(bCheckDestroy==true && DoTestYRearDelay.Off())
            {                                                                                                           //Steven 20210617 : Add for 修正Soft Contact的Delay Time
                if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||                                             //Eliot 2011_0318 Start //ChungHung 20150528 Mark
                   DeviceForm.ContactMode==DirectContactSoftEP ||                                                       //Steven 20160530
                   DeviceForm.ContactMode==TMoveSlowContact)                                                            //Steven 20160130 : TMove Soft contact
                {
                    Task=320;
                }
                else
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

                    if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                 //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                        TestIF_File.bArm1PickPlaceArm2Test==true) &&
                        TestIF_File.bArm1OnlyPlaceArm2TestAndSuck==true)                                                //Steven 20150129 : 需要確認Arm2有沒有粘料
                    {
                        if(TestSocket.HasRealIC()==false)
                        {
                            //AI(W906-W2-A46) 20260926: gate LIFTED -- A4-6 (8ff6c754) merged TMyKitSuck/TMySucker into the one golden layout (mykitsuck.h), premise dead: TMyKitSuck::CopyFrom is mykitsuck.h:369 (nm T); both lines lifted together as golden has them -- golden :7894-7895
                            TestSocket.CopyFrom(FTestSuck);                                                             //JerryYang 20250723 : add
                            FTestSuck.SetAllToNullIC();                                                                 //JerryYang 20250722 : add
                            //AI(W906-W2-A46) 20260926: (end of lifted gate)
                        }
                    }

                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                       TestIF_File.bArm1PickPlaceArm2Test==true)                                                        //JerryYang 20241122 : 啟用D44功能時，INDEX ARM丟料後會持續吹氣，一直到INDEX ARM上抬到丟料高度
                    {
                        if(IniConfig.bD44CheckIndexICDestroy)
                        {
                            for(int i=0; i<MAX_Index_Row; i++)
                            {
                                for(int j=0; j<NEW_MAX_Index_Col; j++)
                                {
                                    if(FTestSuck.Item[i][j]==HAS_HOT_IC || FTestSuck.Item[i][j]==HAS_IC)
                                    {
                                        FTestSuck.Suck[i][j].Destroy();
                                    }
                                }
                            }
                        }
                    }
                    bIndexCheckNoStopVaccum=false;
                    bHasICinSocket=true;

                    if(IniConfig.bIndexArm2SupplyLight==true ||                                                         //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                       (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                 //kevin 20150127 Arm1 下壓 arm2 測試
                        TestIF_File.bArm1PickPlaceArm2Test==true))                                                      //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    {
                        Task=11000;
                        bPlaceTestIC=true;                                                                              //Ifor 20190713 : add Arm1 丟下IC
                    }
                    else
                    {
                        if(IniConfig.bC08_SocketSensor &&
                           TestIF_File.bEnSocketSensor &&
                           TestIF_File.bCheckSocketFloating)                                                            //Steven 20180313 : 使用Socket Sensor驗證置偏
                        {
                            Task=311;
                            break;
                        }
                        else
                        {
                            Task=320;
                        }
                    }
                }

                if(CosFunction.bTesterSidePushFunction==true &&                                                         //Richard 20220321 : 渠梁Side Push
                   DeviceForm.bTesterSidePush==true)
                {
                    DoTestYRearDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);                                    //JerryYang 20240111 : add
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
                //<==
            }
            break;

        //---- chunk 3/3 of DoTestYRear, golden 7960..9232 ----
        case 311:                                                               //Steven 20180313 (Jou) : 使用Socket Sensor驗證置偏
            flag=false;
            str="";
            for(int i=0; i<TestIF_File.iSocketCount; i++)
            {
                if(TestIF_File.iSensorCheckType[i]==2 &&                        //Steven 20200420 : Socket Sensor功能可以選
                   Sen[SThreadPara.iSocketSensor[i]].IsOn())
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:311"));
                    flag=true;
                    str+=IntToStr(i+1)+",";
                }
            }

            if(flag)
            {
                sBufferT="Z1DownZ2Up: Z1Pos "+IntToStr(iAZ1)+">"+IntToStr(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000)+"Z2Pos "+IntToStr(iAZ1)+">"+IntToStr(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000);
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
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 50000, "DoTestYRear 312"))
            {
                Task=313;
            }
            break;
        case 313:                                                               //Y軸分開
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestYRear 313"))
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

                ShowErrorMessage("WAR0323", K_RETRY, MTestZ1, false, str);
                Task=315;
            }
            break;
        case 315:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, 50000, "DoTestYRear 315"))
            {
                Task=316;
            }
            break;
        case 316:                                                               //丟測放下IC後,偵測到置偏排除後, Index往下移動
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test, ((MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed)/100)))
            {
                Task=311;
            }
            break;                                                              //Steven 20180313 (Jou) : 使用Socket Sensor驗證置偏
        case 320:
            if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||                                                 //Eliot 2011_0318 Start
               DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
               DeviceForm.ContactMode==TMoveSlowContact ||                                                              //Steven 20160130 : TMove Soft contact
               DeviceForm.ContactMode==TMoveDropSlowContact)
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, ((MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed)/100)))
                {
                    if(TestIF_File.bForEgisTecTest==true)                                                               //Steven 20140922 : Arm2當作指紋測試
                    {
                        Task=10500;
                    }
                    else
                    {
                        if(DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
                           DeviceForm.ContactMode==TMoveDropSlowContact)
                        {
                            Task=330;                                                                                   //Steven 20160518 : Fixed for DropContactModeDiffentSpeed
                        }
                        else
                        {
                            Task=209;
                            fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                                                       //Steven 20150304 : Add GPIB LOG
                        }
                    }
                }
            }
            else if(DeviceForm.ContactMode==DropPlaceShiftContact)                                                      //ChungHung 20150528 add for 海思 _8Site1x4
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Pick, MOT[MTestZ1].GailSpeed))                                 //Z2 上升至安全位置
                {
                    Task=325;
                }
            }
            else
            {
                if(CosFunction.bTesterSidePushFunction==true &&
                   DeviceForm.bTesterSidePush==true &&
                   DoTestYRearDelay.Off()==false)                                                                       //JerryYang 20240111 : add
                {
                    break;
                }

                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed/100))
                {
                    if(DeviceForm.ContactMode==DropContact)
                    {
                        fObserver->AddTimeData(19, DropContactTimer2.LatchCycleTime()/1000.0);                          //JerryYang 20170503 (wei) 第二段drop contact計時,吹氣drop wait time+移動約2mm到contact高度
                        DropContactTimer3.LatchCycleTime(true);

                        if(CosFunction.bTesterSidePushFunction==true &&                                                 //Richard 20230301 : 渠梁Side Push 新模式
                           DeviceForm.bTesterSidePush==true &&
                           DeviceForm_File.iSidePushMode==0)
                        {
                            DoTestYRearDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
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

                    if(TestIF_File.bForEgisTecTest==true)                                                               //Steven 20140922 : Arm2當作指紋測試
                    {
                        Task=10500;
                    }
                    else
                    {
                        Task=330;                                                                                       //Steven 20141218 : 209 --> 330 修正丟測模式異常
                    }
                }
            }
            break;
        case 325:                                                               //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestY1].Gali_MotMove(Prod.TestY1_Middle-TestIF.dSiteYPitch, MOT[MTestY1].GailSpeed))
            {
                Task=326;
            }
            break;
        case 326:                                                                                                       //ChungHung 20150528 add for 海思 _8Site1x4
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, MOT[MTestZ1].GailSpeed))             //ChungHung 20150528 Mark Index2 下壓至測試高度
                Task=330;
            break;
        case 330:
            if(CosFunction.bTesterSidePushFunction==true && DeviceForm.bTesterSidePush==true && DoTestYRearDelay.Off()==false)                                  //JerryYang 20240111 : add
            {
                break;
            }

            if(USE_ReadIndex_TOQUE && TestIF_File.bEnableReadAndCheckTorque)                                                                                    //kevin 20210804 change
            {
                bNeedCheckIndexToque=false;
                bNeedCheckIndexToque1=false;
                W64bT2_FMAIN_CHKREADTORQUE1->Checked=true;                      // AI(W7R3-c3) 20260810: golden :8122 fMain->chkReadTorque1 -> this file's OWN W64bT2_TfMainTorqueSeam macro (aTester_Rear.cpp:1029). SEAM W7R3-G01.
                W64bT2_FMAIN_CHKREADTORQUE2->Checked=false;                     // AI(W7R3-c3) 20260810: golden :8123 fMain->chkReadTorque2 -> same seam (aTester_Rear.cpp:1030). SEAM W7R3-G01.
#if 0 // GATE W7R3-G02 -- golden :8124 fMain->edTorue0 (golden main.h:466 TEdit*). FormsFacade TfMain has NO edTorue0 -- forms/fMain.h:386-402 records it as DELIBERATELY not added -- and this file's W64bT2_TfMainTorqueSeam carries edTorue1 ONLY, never edTorue0. Substituting edTorue1 would write the ARM-2 torque field instead of ARM-1's, so it is gated, not aliased.
                fMain->edTorue0->Text="";
#endif
                //bOverHappen=false;
            }

            bIndex1Suck=true;                                                                                                                                   //kevin 20220105 Index 在下真空建立 pause 不能關閉
            if(DeviceForm.VacuumMode==VacuumONMode)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                           FTestSuck.Item[i][j]==HAS_IC)
                        {
                            if(INDEX_SUCKER_TYPE==1)                                                                                                            //Steven 20111202
                            {
                                fiosetview->bIndexSuck[0][i][j]=true;
                                bIndexCheckNoStopVaccum=true;                                                                                                   //Sam 20220816 : 增加保護避免暫停 AllSite 掉料
                            }
                            else
                            {
                                FTestSuck.Suck[i][j].On();                                                                                                      //Sam 20230110 : 修正連續 Start<>Stop Drop Contact 掉料問題
                            }
                        }
                    }
                }
            }

            if(DeviceForm.ContactMode==DropContact &&                                                                                                           //JerryYang 20170804 (Steven) 移除邊吸邊測的選項
               CosFunction.bSuckDevicesDuringTest==true)                                                                                                        //JerryYang 20170522 (wei) drop contact改為邊吸邊測
            {
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                                                                                                           //Steven 20150304 : Add GPIB LOG
                Task=209;
                break;
            }

            DoTestYRearDelay.SetMSAndOn(100);                                                                                                                   // delay 0.3 sec for ic down
            Task=340;
            break;
        case 340:
            if(INDEX_SUCKER_TYPE==1)
            {
//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE W7R3-G03 -- golden :8166 fiosetview->ProcessIndexSuckDestroy1() (0-arg). TfiosetviewShim (atester_shims.h:404-409) exposes bIndexSuck[2][4][8] ONLY -- no pump method at all.
                bCheckSuck=fiosetview->ProcessIndexSuckDestroy1();
//AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                bCheckSuck=true;                                                // GATE W7R3-G03 offline: suck self-check reported DONE -- identical default to aTester_Front.cpp:1037 W64B_ProcessIndexSuckDestroy1_V0() and atester.cpp:5517 W7T1_ProcessIndexSuckDestroy1()
//AI(W906-IDXSUCK) （原 #endif）
            }
            else
            {
                bCheckSuck=true;
            }

            if(bCheckSuck==true && DoTestYRearDelay.Off())
            {
                bIndexCheckNoStopVaccum=false;                                  //Sam 20220902 : 修正 All Site 掉料
                bFTestSuckDrop=false;
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                           //Steven 20150304 : Add GPIB LOG
                Task=209;
            }
            break;
        case 350:
            bFTestSuckDrop=false;
            Task=209;
            break;
        case 1000:
            #ifdef  SOFT_SIMULTE
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+10000, MOT[MTestZ1].GailSpeed))
                    Task=1100;
            #else
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000, MOT[MTestZ1].GailSpeed))
                    Task=1100;
            #endif
            break;
        case 1100:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, MOT[MTestZ1].GailSpeed))
                Task=209;
            break;
        case 1200:
            DoTestYRearDelay.SetMSAndOn(300);
            Task=1300;
            break;
        case 1300:
            if(DoTestYRearDelay.Off())
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
        case 10500:                                                             //Steven 20140922 : Arm2當作指紋測試
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(bCheckShuttle==false)
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

                if(MOT[MTestZ2].Gali_ReadPos()!=Prod.TestZ2_Safe)
                {
                    Task=11000;
                    break;
                }
                bCheckShuttle=true;
            }

            if(IndexStatus==Z1Down_Z2Up)
            {
                MOT[MInShuttle1].fCanMoveM=true;                                //jou 2011-04-19 start : Task直接跳到200，一些參數沒有初始化
                MOT[MInShuttle2].fCanMoveM=true;
                bCheckShuttle=false;
                bOneTimeFlag=true;
                tIndexTimer.LatchCycleTime(true);
                ShowMainScreenPresure(0);                                       //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                bSwitchArm2=false;
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                           //Steven 20150304 : Add GPIB LOG
                Task=209;
                break;
            }
            else if(IndexStatus==Z1_Z2_Normal)
            {
                ShowIndexMotorError(AnsiString("DoTestYRear10500"));
                return false;
            }
            break;
        case 11000:                                                             //jou 2012-10-19 Index Arm 2 供應光源 for CMOS start
            if(MOT[MTestZ2].Gali_ReadPos()!=Prod.TestZ2_Safe)
            {
                Task=11010;
            }
            else
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //JerryYang 20241122 : 啟用D44功能時，INDEX ARM丟料後會持續吹氣，一直到INDEX ARM上抬到丟料高度
                   TestIF_File.bArm1PickPlaceArm2Test==true     &&
                   IndexStatus==Z1Down_Z2Up)
                {
                    Task=11010;
                }
                else
                {
                    Task=11020;
                }
            }

            if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                   TestIF_File.bArm1PickPlaceArm2Test==true     &&
                   TestIF_File.bRTC20CheckFunction==true        )
                {
                    if(TestIF_File.bRTC20GiveWayCheck==true)
                    {
                        if(LastSet.iRealDummy==REALLY || LastSet.iRealDummy==DUMMY)
                        {
                            DoBRTCGiveWayCheck(true, true, 2);
                        }
                        else
                        {
                            DoBRTCGiveWayCheck(true, false, 2);
                        }
                        Task=11002;
                        goto DoTestYRear_ReStart;
                    }
                    else
                    {
#if 0 // GATE W7R3-G04 -- golden :8304-8305 COM2->SendCommToVision(COM2->rtArmIndex2/rtCHECKIC, true). None of the three tokens exists on the SHARED TCOM2Shim (atester_shims.h:420-461 = bCCDDummyRum / DoReleaseAndInspEnd / ATCAlarmSenCheck); SendCommToVision + the rt* channel ids live ONLY on atester.cpp's TU-LOCAL W7T1_TCOM2Ext seam (atester.cpp:5600-5612) and atester_32Site.cpp's own W5_32S seam (:357-379), neither reachable from this TU. Reachable only under REAL_TIME_CCD && !bCCDDummyRum, which is FALSE offline (TCOM2Shim ctor sets bCCDDummyRum=true) -> behaviour-identical offline. Real machine: the RTC would never be told "arm 2 is in place, go check the ICs", so the vision full-check would not run for arm 2. NOTE the gate is NARROWED to these two lines only -- golden's whole enclosing guard chain and the DoBRTCGiveWayCheck sister branch above stay LIVE.
                        COM2->SendCommToVision(COM2->rtArmIndex2, true);
                        COM2->SendCommToVision(COM2->rtCHECKIC, true);
#endif
                        bRealCCDSendArm=false;
#if 0 // GATE W7R3-G05 -- golden :8307 ScanBtnThd->Start(). The global scan-button thread pointer is declared NOWHERE in the port: class ScanBtn exists (ScanBtnThread.h:63) but no `ScanBtn *ScanBtnThd` object, and the only two textual uses in the tree (atester.cpp:10822/:10997) sit inside atester.cpp's `#if 0 // GOLDEN VERBATIM` block opened at atester.cpp:10664 (measured with a preprocessor-stack walk, not by eyeballing). Behaviour-identical offline (same false RTC guard as GATE W7R3-G04). Real machine: the RTC grab-result poller would not be started. golden :8306 bRealCCDSendArm=false is left LIVE -- it is a plain global that does exist.
                            ScanBtnThd->Start();
#endif
                    }
                }
            }
            break;
        case 11002:
            if(LastSet.iRealDummy==REALLY || LastSet.iRealDummy==DUMMY)
            {
                if(DoBRTCGiveWayCheck(false, true, 2)==true)
                {
                    Task=11020;
                    bCheckShuttle=true;
                    goto DoTestYRear_ReStart;
                }
            }
            else
            {
                if(DoBRTCGiveWayCheck(false, false, 2)==true)
                {
                    Task=11020;
                    bCheckShuttle=true;
                    goto DoTestYRear_ReStart;
                }
            }
            break;
        case 11010:
            if(MOT[MTestZ2].Gali_MotMoveNoWait(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed, 0))
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //JerryYang 20241122 : 啟用D44功能時，INDEX ARM丟料後會持續吹氣，一直到INDEX ARM上抬到丟料高度
                   TestIF_File.bArm1PickPlaceArm2Test==true     &&
                   IndexStatus==Z1Down_Z2Up)
                {
                    Task=11015;
                }
                else
                {
                    Task=11020;
                }
            }
            break;
        case 11015:
            if(DeviceForm.ContactMode==DropContact ||
               DeviceForm.ContactMode==DropContactModeDiffentSpeed ||           //jou 2011-10-31 start : 丟測模式與不同速度模式也要支援兩段式上升
               DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||
               DeviceForm.ContactMode==TMoveDrop   ||
               DeviceForm.ContactMode==TMoveDropSlowContact ||
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //kevin 20150127 Arm1 下壓 arm2 測試
                   TestIF_File.bArm1PickPlaceArm2Test==true)
                {
                    iFinishTestUpWaitHeight=Prod.TestZ1_Test+Prod.TestZ1_Up_Offset;
                }
                else
                {
                    iFinishTestUpWaitHeight=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+Prod.TestZ1_Up_Offset;
                }
            }
            else
            {
                iFinishTestUpWaitHeight=Prod.TestZ1_Test+Prod.TestZ1_Up_Offset;
            }

            if(MOT[MTestZ1].Gali_MotMove(iFinishTestUpWaitHeight, ((MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed)/100)))
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                   TestIF_File.bArm1PickPlaceArm2Test==true)
                {
                    if(IniConfig.bD44CheckIndexICDestroy)
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                                   FTestSuck.Item[i][j]==HAS_IC)
                                {
                                    FTestSuck.Suck[i][j].Normal();
                                }
                            }
                        }
                    }
                }

                Task=11016;
                DoTestYRearDelay.SetSecAndOn(Prod.TestZ_Up_Wait);
            }
            break;
        case 11016:
            if(DoTestYRearDelay.Off())
            {
                if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&             //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    TestIF_File.bArm1PickPlaceArm2Test==true) &&
                    TestIF_File.bArm1OnlyPlaceArm2TestAndSuck==true)            //Steven 20150129 : 需要確認Arm2有沒有粘料
                {
                    InitRearTestDestroyICTask();                                //JerryYang 20250722 : add
                }

                Task=11020;
                if(IniConfig.bD44CheckIndexICDestroy)
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.bNeedCheck[i][j])                        //Suck On Destry Off -- AI(W7R3-c3) 20260810: golden :8412 FTestSuck.bNeedCheck[i][j]; SEAM W7R3-G06 reuses this file's existing kit-agnostic stub (aTester_Rear.cpp:317)   // AI(W906-W2-A46) 20260926: golden 是 FTestSuck，不能走 BTestSuck 的替身
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
//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE W7R3-G07 -- golden :8433 fiosetview->ProcessIndexSuckDestroy1(1) as a bare statement (return value discarded). Same absent method as GATE W7R3-G03. Offline the DAQ pump is a no-op, so dropping the statement IS the offline default -- nothing on this line is observed.
                    fiosetview->ProcessIndexSuckDestroy1(1);                    //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆 //kevin 20181108 add 提前動作
//AI(W906-IDXSUCK) （原 #endif）
                    DoFrontTestDestroyICDelay.SetMSAndOn(IniConfig.iD44TestHeadCheckVacuumTime);
                    Task=11017;
                    break;
                }
                else
                {
                    bD44Arm1CheckVacOn=false;                                   //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
                }
            }
            break;
        case 11017:                                                                                                     //jou 2012-01-04 需確認Index suck已經完整做完
            if(INDEX_SUCKER_TYPE==1)
            {
                bArm1D44SuckCheck=false;                                                                                //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
//AI(W906-IDXSUCK) 20260927: 閘解開 —— TfiosetviewShim 現在有 golden 照翻的泵（atester_shims.cpp 檔尾），原閘的前提不成立。原閘：GATE W7R3-G08 -- golden :8448 fiosetview->ProcessIndexSuckDestroy1(1). Same absent method as GATE W7R3-G03/G07. NOTE golden :8447 already writes bArm1D44SuckCheck=false one line above and :8448 overwrites it -- that redundancy is GOLDEN'S OWN and is preserved, not tidied.
                bArm1D44SuckCheck=fiosetview->ProcessIndexSuckDestroy1(1);                                              //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆
//AI(W906-IDXSUCK) 以下是原本的離線替代（#else），已停用：
//                bArm1D44SuckCheck=true;                                                                                 // GATE W7R3-G08 offline: suck self-check reported DONE (exactly the value aTester_Front.cpp:712 W64B_FIOSET_PISD1(1) yields at the mirrored Front call site)
//AI(W906-IDXSUCK) （原 #endif）
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
                bHasDuplicateErr=false;
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                        if(bArmDuplicateErr[i][j])
                            bHasDuplicateErr=true;
                }

                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.bNeedCheck[i][j])                                                                    //確認真空狀態 -- AI(W7R3-c3) 20260810: golden :8472 FTestSuck.bNeedCheck[i][j]; SEAM W7R3-G06   // AI(W906-W2-R61) 20260926: golden 是 FTestSuck，不能走 W64bT2_ 替身（那個讀寫 BTestSuck）；NB2 R61 抓到，033358a2 的錯
                        {
                            if(FTestSuck.Suck[i][j].W906_GetStatusAnyOn())  // AI(W906-F03-QUADVAC) 20261004: 群組：任 1 個有真空就算（黏料／殘料／不該有料）
                            {
                                bHasErr=true;
                                ErrPart+=IndexSuckName[i+iNN][j];                                                       //kevin 20190418 add 32 site show IC name
                                flag=true;
                                bArmDuplicateErr[i][j]=true;                                                            //Steven 20110127
                                iIndexTakeDeviceChk1=3;                                                                 //kevin 20190103 回吸檢測狀態
                            }
                            else
                            {
                                FTestSuck.bNeedCheck[i][j]=false;                                                           // AI(W7R3-c3) 20260810: golden :8484 FTestSuck.bNeedCheck[i][j]=false; SEAM W7R3-G06   // AI(W906-W2-R61) 20260926: golden 是 FTestSuck，不能走 W64bT2_ 替身（那個讀寫 BTestSuck）；NB2 R61 抓到，033358a2 的錯
                                FTestSuck.Suck[i][j].Normal();
                                bArmDuplicateErr[i][j]=false;                                                           //Steven 20110127
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
                    Task=11016;                                                                                         //kevin 20181102
                    return false;
                }
                bIndexAlarmInArmAway=false;                                                                             //kevin 20181102 (Steven) :  index掉料 inarm 讓位 inarm先不要動
                bHasErr=false;
                //Steven 20100105 End

                if(flag==false)
                {
                    Task=11016;
                }
                else
                {
                    if(IniConfig.bD44CheckIndexICDestroy)                                                               //jou 2011-11-01負壓不能一直開著真空，必須關掉
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(FTestSuck.bNeedCheck[i][j])                                                            // AI(W7R3-c3) 20260810: golden :8527 FTestSuck.bNeedCheck[i][j]; SEAM W7R3-G06   // AI(W906-W2-R61) 20260926: golden 是 FTestSuck，不能走 W64bT2_ 替身（那個讀寫 BTestSuck）；NB2 R61 抓到，033358a2 的錯
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
        case 11020:
            if(USE_IO_CHANGE_TOQUE==true)                                                                               //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(bCheckShuttle==false)
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

                if(MOT[MTestZ2].Gali_ReadPos()!=Prod.TestZ2_Safe)
                {
                    Task=11000;
                    break;
                }
                bCheckShuttle=true;
            }

            if(IndexStatus==Z1Up_Z2Down)
            {
                if(TestIF_File.bForEgisTecTest==true)                                                                   //Steven 20140922 : Arm2當作指紋測試
                {
                    fMain->SendMSG_CMD(MSG_CMD_SwitchArmOK);
                }
                else
                {
                    MOT[MInShuttle1].fCanMoveM=true;                                                                    //jou 2011-04-19 start : Task直接跳到200，一些參數沒有初始化
                    MOT[MInShuttle2].fCanMoveM=true;
                    bCheckShuttle=false;
                    bOneTimeFlag=true;
                    ShowMainScreenPresure(1);                                                                           //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                }

                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                                                                   //Steven 20150304 : Add GPIB LOG
                Task=209;
                break;
            }
            else if(IndexStatus==Z1_Z2_Normal)
            {
                ShowIndexMotorError(AnsiString("DoTestYRear11020"));
                return false;
            }
            else
            {
                if(bOneTimeFlag)
                {
                    bOneTimeFlag=false;
                    if(TestIF_File.bUseSLKClamp &&
                       TestIF_File.iSeparabilityTest==1)                                                                //JerryYang 20160523 Z1分離高度要上升1500
                    {
                        iBackUpZ2DownPosition=Prod.TestZ2_Test+1500;
                    }
                    else
                    {
                        iBackUpZ2DownPosition=Prod.TestZ2_Test;
                    }
                    BTestSeparateSLK(true);                                                                             //JerryYang 20180518 (wei) : 分離流程初始化
                    FTestSeparateSLK(true);
                }

                if(CheckSocketSensor(1, "DoTestYRear_11020_"))                                                          //Z1UpZ2Down //Steven 20200615 : Socket Sensor整合成Function
                {
                    Task=50;
                    return false;
                }

                if(IniConfig.bIndexPickupErrStop==false)
                {
                    bIndexArm1PickupErrStop=false;
                    bIndexArm2PickupErrStop=false;
                }

                if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                     //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    TestIF_File.bArm1PickPlaceArm2Test==true) &&
                    TestIF_File.bArm1OnlyPlaceArm2TestAndSuck==true)                                                    //Steven 20150129 : 需要確認Arm2有沒有粘料
                {
                    if(fRearNeedDestroy)                                                                                //JerryYang 20250723 : arm 2 destroy
                    {
                        bZ1PickShuttle=false;                                                                           //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                        bZ2PickShuttle=false;                                                                           //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
                        fRearNeedDestroy=!DoRearTestDestroyIC(true);                                                    //ChungHung 20111115
                        return false;
                    }
                }

                if(DeviceForm.ContactMode==TMove ||
                   DeviceForm.ContactMode==TMoveDrop ||                                                                 //jou 2012-02-03 新增T Move Drop
                   DeviceForm.ContactMode==TMoveDropSlowContact)
                {
                    TMode=true;
                }
                else
                {
                    if(IniConfig.bIndexPickupErrStop==true &&                                                           //jou 2012-02-29 index pick up error,index arm move to center & alarm
                        bIndexArm2PickupErrStop==true)
                    {
                        TMode=true;
                    }
                    else
                    {
                        TMode=false;
                    }
                }

                if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)                                                          //----- by dell ccd realtime-------------
                {
#if 0 // GATE W7R3-G09 -- golden :8663-8681, the body of case 11020's RTC block. SIX absent symbols, all of them only on TU-local seams elsewhere: ScanBtnThd (see GATE W7R3-G05), COM2->bRealTimeCom_ReceiveOK[], COM2->rtAlarHasIC, COM2->rtAlarGrabTimeOut, COM2->SendCommToVision, COM2->rtRelease. The whole body is gated as one unit because the OUTER predicate itself (ScanBtnThd->GetArmState()) is one of the absent symbols -- it cannot be narrowed further. golden's enclosing `if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)` on :8661-8662 stays LIVE and is false offline, so gating is behaviour-identical offline. Real machine: an RTC "socket has IC" alarm or grab time-out during the arm-2 head swap would no longer divert the SM to Task=50 / raise WAR0341.
                    if(ScanBtnThd->GetArmState())
                    {
                        if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
                        {
                            MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:11020"));
                            ScanBtnThd->SetArmState(false);
                            Task=50;
                        }
                        else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarGrabTimeOut])
                        {
                            COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarGrabTimeOut]=false;
                            ShowErrorMessage("WAR0341", 0, MMIndex, 0, __FUNC__+AnsiString(",case:11020"));             //RTC Grab TimeOut Error!
                            COM2->SendCommToVision(COM2->rtRelease, false);
                            MySleep(100);

                            ScanBtnThd->SetArmState(false);
                        }
                        return false;
                    }
#endif
                }

                #ifdef INDEX_PROTECT_TMOVE
                if(bOverRangeDoTMode==true && bTriger4Indexhome==false)                                                 //Isaac 20201012 : index Y超過範圍，做一次Tmode
                {
                    bTriger4Indexhome=true;
                    bOverRange4Indexhome=true;
                    TrigerIndexAxisHome();                                                                              //Isaac 20201012 : index Y超過範圍，做一次Tmode，初始化，開始自動校正
                    return false;
                }
                #endif

                if(MOT[MTestY1].Z1UpZ2Down(MOT[MTestZ2].GailSpeed, TMode, bIndexArm2PickupErrStop))
                {
                    if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)
                    {
                        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                           TestIF_File.bArm1PickPlaceArm2Test==true     &&
                           TestIF_File.bRTC20CheckFunction==true        )
                        {
#if 0 // GATE W7R3-G10 -- golden :8702 ScanBtnThd->Stop(); same absent global as GATE W7R3-G05. Narrowed to the single call: golden's two enclosing guards on :8696-8701 stay LIVE.
                            ScanBtnThd->Stop();
#endif
                        }
                    }

                    fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                                                               //Steven 20150304 : Add GPIB LOG
                    if(ATC_SYSTEM==eNewATCSystem &&
                       Temperature.bATCActiveCooling==true)                                                             //JerryYang 20220815 : send ATC which ARM
                    {
#if 0 // GATE W7R3-G11 -- golden :8710 ATC_InterfaceForm->HandlerArm(1). TATC_InterfaceFormShim (acarry_shims.h:109-115) carries iATC_MODE_TYPE ONLY. TRAP-5 WARNING for whoever retires this: the `HandlerArm` token DOES exist in the tree (ATC/ATCSystem.h + atester.cpp) but on a DIFFERENT class -- calling it through this shim is not merely absent, it is a name collision waiting to bind to the wrong object. Offline the enclosing guard is false anyway (ATC_SYSTEM=0/eATCUninstall, cmydef.cpp:3567, vs eNewATCSystem). Real machine: the ATC would not be told arm 2 is the arm now in the test area.
                        ATC_InterfaceForm->HandlerArm(1);
#endif
                    }
                    iRTCErrorCount=0;                                           //wei 20221222 RTC ARM Error
                    iWhichArmDown=2;                                                                                    //JerryYang 20200316 add SVID 哪支arm下壓在測區
#if 0 // GATE W7R3-G12 -- golden :8714 fContact->ATC_SwitchTjSignal(2). The token ATC_SwitchTjSignal exists NOWHERE in the port tree (not even on a TU-local seam); TfContactShim (atester_shims.h:154+) has no such method. Offline: pure output toward the ATC TJ line -- nothing in the SM reads it back. Real machine: the ATC would keep arm-1 TJ thermal feedback selected while arm 2 is the one testing.
                    fContact->ATC_SwitchTjSignal(2);                                                                    //Ifor 20210622 add: ATC Switch TJ
#endif
                    bIndexArm2PickupErrStop=false;
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    bCheckShuttle=false;
                    bOneTimeFlag=true;
                    ShowMainScreenPresure(1);                                                                           //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                    Task=11030;
                }
            }
            break;
        case 11030:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, MOT[MTestZ2].GailSpeed*Prod.TestZ_Drop_Speed/100))
            {
                if(TestIF_File.bForEgisTecTest==true)                           //Steven 20140922 : Arm2當作指紋測試
                {
                    fMain->SendMSG_CMD(MSG_CMD_SwitchArmOK);
                }

                fRearNeedDestroy=false;
                fRearNeedSuck=false;
                fMain->SendMSG_CMD(MSG_CMD_Arm2Down);                           //Steven 20150304 : Add GPIB LOG
                if((IniConfig.bD58UseArm1PickPlaceArm2Test==true &&             //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    TestIF_File.bArm1PickPlaceArm2Test==true) &&
                    TestIF_File.bArm1OnlyPlaceArm2TestAndSuck==true)            //Steven 20150129 : 需要確認Arm2有沒有粘料
                {
                    BTestSuck.ResetAll();
//AI(W906-W2-TRAP5) 20260926: gate LIFTED -- A4-6 (8ff6c754) merged the two TMyKitSuck/TMySucker classes into the one golden layout, so the TRAP-5 premise is dead: TMyKitSuck::CopyFrom(TMyKitSuck&) is mykitsuck.h:369 / mykitsuck.cpp (nm) and TestSocket is a TMyKitSuck (mykitsuck.h:463) -- golden :8741
                    BTestSuck.CopyFrom(TestSocket);
//AI(W906-W2-TRAP5) 20260926: (end of lifted gate)
                    for(int i=0; i<BTestSuck.iShtRow; i++)                      //JerryYang 20160727 修正沒使用的真空產生器也吸真空
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Item[i][j]==HAS_IC)
                            {
                                fiosetview->bIndexSuck[1][i][j]=true;
                            }
                        }
                    }
                    Task=11035;
                }
                else
                {
                    Task=209;
                }
            }
            break;
        case 11035:                                                             //JerryYang 20250723 : SUCK 把socket資料過到arm上面
            bCheckSuck=W64bT2_ProcessIndexSuckDestroy2();                        // AI(W7R3-c3) 20260810: golden :8761 fiosetview->ProcessIndexSuckDestroy2() -> this file's OWN existing stub (aTester_Rear.cpp:367; offline true = suck self-check done). SEAM W7R3-G14.
            if(bCheckSuck==true)
            {
                Task=209;
            }
            break;
        case 11040:
            if(CheckIndexStatus("DoTestYRear11040"))
            {
                iRetry=0;
                Task=11050;                                                     //Steven 20200708 : 改成強制跑Drop Height
                if(REAL_TIME_CCD &&                                             //Ifor 20191120 : add 測試完畢需檢查IC有無置偏
                   COM2->bCCDDummyRum==false &&
                   fMain->palMainStatus->Caption!="Index Check")
                {
                    if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                       TestIF_File.bArm1PickPlaceArm2Test==true     &&
                       TestIF_File.bRTC20CheckFunction==true        &&
                       TestIF_File.bRTC20GiveWayCheck==true         )
                    {
                        if(LastSet.iRealDummy==REALLY || LastSet.iRealDummy==DUMMY)
                        {
                            DoBRTCGiveWayCheck(true, true, 1);
                        }
                        else
                        {
                            DoBRTCGiveWayCheck(true, false, 1);
                        }
                        Task=11041;
                        goto DoTestYRear_ReStart;
                    }
                }
            }
            break;
        case 11041:
            if(LastSet.iRealDummy==REALLY || LastSet.iRealDummy==DUMMY)
            {
                if(DoBRTCGiveWayCheck(false, true, 1)==true)
                {
                    Task=11080;
                    bCheckShuttle=true;
                    goto DoTestYRear_ReStart;
                }
            }
            else
            {
                if(DoBRTCGiveWayCheck(false, false, 1)==true)
                {
                    Task=11080;
                    bCheckShuttle=true;
                    goto DoTestYRear_ReStart;
                }
            }
            break;
        case 11050:
            //jou 2011-10-31 start : 丟測模式與不同速度模式也要支援兩段式上升
            if(DeviceForm.ContactMode==DropContact ||
               DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
               DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||
               DeviceForm.ContactMode==TMoveDrop   ||
               DeviceForm.ContactMode==TMoveDropSlowContact ||
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
                if(IniConfig.bD21EnableFinishTestUpWait)                        //jou 2011-10-31 start : 丟測模式與不同速度模式也要支援兩段式上升
                    iFinishTestUpWaitHeight=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+IniConfig.iD21FinishTestUpWaitHeight;
                else
                    iFinishTestUpWaitHeight=Prod.TestZ2_Test;
            }
            else
            {
                if(IniConfig.bD21EnableFinishTestUpWait)
                    iFinishTestUpWaitHeight=Prod.TestZ2_Test+IniConfig.iD21FinishTestUpWaitHeight;
                else
                    iFinishTestUpWaitHeight=Prod.TestZ2_Test;
            }
            //jou 2011-10-31 end

            //Steven 20200630 : 測試完後要吹一下, 避免殘料
            //==>
            if(TestIF_File.bCheckArm2Vacuum)
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Item[i][j]!=NULL_IC && FTestSuck.Item[i][j]!=HAS_NULL_IC)
                            BTestSuck.Suck[i][j].Off();
                    }
                }
            }
            //<==
            //Steven 20200630 : 測試完後要吹一下, 避免殘料

            if(MOT[MTestZ2].Gali_MotMove(iFinishTestUpWaitHeight, ((MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed)/100)))
            {
                Task=11060;
                if(IniConfig.bD21EnableFinishTestUpWait)                        //Steven 20200708
                    DoTestYRearDelay.SetMSAndOn(IniConfig.iD21FinishTestUpWaitTime);
                else
                    DoTestYRearDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);
            }
            break;
        case 11060:
            if(DoTestYRearDelay.Off())
            {
                //Steven 20200630 : 測試完後要吹一下, 避免殘料
                //==>
                if(TestIF_File.bCheckArm2Vacuum)
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC && FTestSuck.Item[i][j]!=HAS_NULL_IC)
                                BTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                //<==
                //Steven 20200630 : 測試完後要吹一下, 避免殘料
                Task=11080;
            }
            break;
        case 11080:
            if(MOT[MTestZ1].Gali_ReadPos()!=Prod.TestZ1_Safe)
            {
                Task=11090;
            }
            else
            {
                Task=11100;
                if(Prod.bIndexUpSpeed &&
                   (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&             //kevin 20190219 add Arm pick arm2 place
                    TestIF_File.bArm1PickPlaceArm2Test==true))                  //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    Task=11095;
            }

            if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)
            {
                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                   TestIF_File.bArm1PickPlaceArm2Test==true     &&
                   TestIF_File.bRTC20CheckFunction==true        )
                {
                    if(TestIF_File.bRTC20GiveWayCheck==true)
                    {
                    }
                    else
                    {
#if 0 // GATE W7R3-G15 -- golden :8909-8910 COM2->SendCommToVision(COM2->rtArmIndex1/rtCHECKIC, true); the arm-1 twin of GATE W7R3-G04, same three absent tokens. golden :8911 bRealCCDSendArm=false stays LIVE. GOLDEN ODDITY PRESERVED VERBATIM just above: :8904-8906 is `if(TestIF_File.bRTC20GiveWayCheck==true){}` with a completely EMPTY then-branch and all the work in the else -- kept exactly, not "cleaned up".
                        COM2->SendCommToVision(COM2->rtArmIndex1, true);
                        COM2->SendCommToVision(COM2->rtCHECKIC, true);
#endif
                        bRealCCDSendArm=false;
#if 0 // GATE W7R3-G16 -- golden :8912 ScanBtnThd->Start(); arm-1 twin of GATE W7R3-G05, same absent global.
                        ScanBtnThd->Start();
#endif
                    }
                }
            }
            break;
        case 11090:
            if(MOT[MTestZ1].Gali_MotMoveNoWait(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed, 0))
            {
                Task=11100;
                if(Prod.bIndexUpSpeed &&
                   (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&             //kevin 20190219 add Arm pick arm2 place
                    TestIF_File.bArm1PickPlaceArm2Test==true))                  //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                    Task=11095;
            }
            break;
        case 11095:
            if(MOT[MTestY2].Gali_ReadPos()==Prod.TestY2_Middle)                 //kevin 20190215 add ARM 2 在測區
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+500, MOT[MTestZ2].GailSpeed*Prod.TestZ_Drop_Speed/100))
                {
                    DoBTestSuckTestICDelay.SetSecAndOn(Prod.TestZ_Drop_Wait);
                    Task=11096;
                }
            }
            break;
        case 11096:
            if(DoBTestSuckTestICDelay.Off())
            {
                Task=11100;
            }
            break;
        case 11100:
            if(USE_IO_CHANGE_TOQUE==true)                                                                               //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(bCheckShuttle==false)
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

                if(MOT[MTestZ1].Gali_ReadPos()!=Prod.TestZ1_Safe)
                {
                    Task=11080;
                    break;
                }
                bCheckShuttle=true;
            }

            if(IndexStatus==Z1Down_Z2Up)
            {
                MOT[MInShuttle1].fCanMoveM=true;                                                                        //jou 2011-04-19 start : Task直接跳到200，一些參數沒有初始化
                MOT[MInShuttle2].fCanMoveM=true;
                bCheckShuttle=false;
                bOneTimeFlag=true;
                ShowMainScreenPresure(0);                                                                               //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                bRearHeadICFallDown=false;
                Task=215;
            }
            else if(IndexStatus==Z1_Z2_Normal)
            {
                ShowIndexMotorError(AnsiString("DoTestYRear11100"));
                return false;
            }
            else
            {
                if(bOneTimeFlag)
                {
                    bOneTimeFlag=false;
                    iBackUpZ1DownPosition=Prod.TestZ1_Test;
                }

                if(CheckSocketSensor(1, "DoTestYRear_11100_"))                                                          //Z1DownZ2Up //Steven 20200615 : Socket Sensor整合成Function
                {
                    Task=50;
                    return false;
                }

                if(IniConfig.bIndexPickupErrStop==false)
                {
                    bIndexArm1PickupErrStop=false;
                    bIndexArm2PickupErrStop=false;
                }

                if(DeviceForm.ContactMode==TMove ||
                   DeviceForm.ContactMode==TMoveDrop ||                                                                 //jou 2012-02-03 新增T Move Drop
                   DeviceForm.ContactMode==TMoveDropSlowContact ||
                   DeviceForm.ContactMode==TMoveSlowContact)                                                            //Steven 20160130 : TMove Soft contact
                {
                    TMode=true;
                }
                else
                {
                    if(IniConfig.bIndexPickupErrStop==true &&                                                           //jou 2012-02-29 index pick up error,index arm move to center & alarm
                       bIndexArm1PickupErrStop==true)
                    {
                        TMode=true;
                    }
                    else
                    {
                        TMode=false;
                    }
                }

                if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)                                                          //----- by dell ccd realtime-------------
                {
#if 0 // GATE W7R3-G17 -- golden :9029-9047, the arm-1 twin of GATE W7R3-G09 (same six absent ScanBtnThd/COM2 symbols, same un-narrowable outer predicate ScanBtnThd->GetArmState()). golden's `if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)` on :9027-9028 stays LIVE and is false offline.
                    if(ScanBtnThd->GetArmState())
                    {
                        if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC])
                        {
                            MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:11100"));
                            ScanBtnThd->SetArmState(false);
                            Task=50;
                        }
                        else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarGrabTimeOut])
                        {
                            COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarGrabTimeOut]=false;
                            ShowErrorMessage("WAR0341", 0, MMIndex, 0, __FUNC__);                                       //RTC Grab TimeOut Error!
                            COM2->SendCommToVision(COM2->rtRelease, false);
                            MySleep(100);

                            ScanBtnThd->SetArmState(false);
                        }
                        return false;
                    }
#endif
                }

                #ifdef INDEX_PROTECT_TMOVE
                if(bOverRangeDoTMode==true && bTriger4Indexhome==false)                                                 //Isaac 20201012 : index Y超過範圍，做一次Tmode
                {
                    bTriger4Indexhome=true;
                    bOverRange4Indexhome=true;
                    TrigerIndexAxisHome();                                                                              //Isaac 20201012 : index Y超過範圍，做一次Tmode，初始化，開始自動校正
                    return false;
                }
                #endif

                if(MOT[MTestY1].Z1DownZ2Up(MOT[MTestZ1].GailSpeed, TMode, bIndexArm1PickupErrStop))
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC &&
                               FTestSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                BTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }

                    if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)
                    {
                        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                           TestIF_File.bArm1PickPlaceArm2Test==true     &&
                           TestIF_File.bRTC20CheckFunction==true        )
                        {
#if 0 // GATE W7R3-G18 -- golden :9080 ScanBtnThd->Stop(); arm-1 twin of GATE W7R3-G10.
                            ScanBtnThd->Stop();
#endif
                        }
                    }
                    fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                                                               //Steven 20150304 : Add GPIB LOG

                    if(ATC_SYSTEM==eNewATCSystem &&
                       Temperature.bATCActiveCooling==true)                                                             //JerryYang 20220815 : send ATC which ARM
                    {
#if 0 // GATE W7R3-G19 -- golden :9088 ATC_InterfaceForm->HandlerArm(0); arm-1 twin of GATE W7R3-G11, same absent shim method and same TRAP-5 name-collision warning.
                        ATC_InterfaceForm->HandlerArm(0);
#endif
                    }
                    iRTCErrorCount=0;                                           //wei 20221222 RTC ARM Error
                    iWhichArmDown=1;                                                                                    //JerryYang 20200316 add SVID 哪支arm下壓在測區
#if 0 // GATE W7R3-G20 -- golden :9092 fContact->ATC_SwitchTjSignal(1); arm-1 twin of GATE W7R3-G12, same absent method.
                    fContact->ATC_SwitchTjSignal(1);                                                                    //Ifor 20210622 add: ATC Switch TJ
#endif
                    bIndexArm1PickupErrStop=false;
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    bCheckShuttle=false;
                    bOneTimeFlag=true;
                    ShowMainScreenPresure(0);                                                                           //jou 2010-06-23 畫面Z1,Z2 encoder 顯示
                    bRearHeadICFallDown=false;
                    Task=11110;
                }
            }
            break;
        case 11110:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, MOT[MTestZ1].GailSpeed))
            {
                fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                           //Steven 20150304 : Add GPIB LOG
                FTestSuck.ResetAll();
                Task=215;
                bPlaceTestIC=false;                                             //Ifor 20190713 : add Arm1 丟下IC
                bCheckGiveWay=true;                                             //Ifor 20190723 : add 測試完畢 Arm2 不下去測區
            }
            break;                                                              //jou 2012-10-19 Index Arm 2 供應光源 for CMOS end
        case 12000:                                                             //jou 2014-06-24 RTC 自動進行Model驗證 start
            DoBRTCAutoModelVerify(true);
            Task=12010;
            break;
        case 12010:
            if(DoBRTCAutoModelVerify(false))
            {
                bNeedWaitRTCAutoVerify=false;
                return true;
            }
            break;
        case 14000:
            if(BTestCombineSLK()==true)                                         //JerryYang 20160429 分離模式, 測試完成 Z1 SLK要與Clamp結合,再將IC吸起流程
            {
                return true;
            }
            break;
        case 16000:                                                             //Steven 20190115 : SCC要求吸取異常要檢查Socket
            DoIndexArm1PickUpErrNeedPiggyback(true);
            Task=16100;
//            break;
        case 16100:
            if(DoIndexArm1PickUpErrNeedPiggyback(false))
            {
                Task=1;
                return true;
            }
            break;                                                              //Steven 20190115 : SCC要求吸取異常要檢查Socket
        case 17900:                                                             //Richard 20220321 : 渠梁Side Push
            if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)      //Richard 20221110 : 渠梁Side Push 新增關SITE不動作
            {
                RecordProcess("DoTestYRear_Case17900");
                Task=320;
                break;
            }

            if(DoTestYRearDelay.Off()==false)
            {
                break;
            }

            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-100, MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed/100))
            {
                DoTestYRearDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                Task=18000;
            }
            break;
        case 18000:
            if(DoTestYRearDelay.Off()==false)
            {
                break;
            }

            if(DoTesterSidePush(true)==1)
            {
                DoTestYRearDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);

                if(IniConfig.bIndexArm2SupplyLight==true ||                     //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                   (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&             //kevin 20150127 Arm1 下壓 arm2 測試
                    TestIF_File.bArm1PickPlaceArm2Test==true))                  //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                {
                    Task=11000;
                    bPlaceTestIC=true;                                          //Ifor 20190713 : add Arm1 丟下IC
                }
                else
                {
                    Task=320;                                                   //Richard 20230301 : 渠梁Side Push 新模式   18100->320
                }
            }
            break;
        case 18100:
            if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)      //Richard 20221110 : 渠梁Side Push 新增關SITE不動作
            {
                RecordProcess("DoTestYRear_Case18100");
                Task=330;
                break;
            }

            if(DoTestYRearDelay.Off()==false)
            {
                break;
            }

            if(DoTesterSidePush(false)==10)
            {
                DoTestYRearDelay.SetSecAndOn(DeviceForm_File.dSitePushWaitTime);
                Task=330;                                                       //Richard 20230301 : 渠梁Side Push 新模式   320->330
            }
            break;
        case 18500:
            if(TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1)      //Richard 20221110 : 渠梁Side Push 新增關SITE不動作
            {
                RecordProcess("DoTestYFront_Case18500");
                return true;
            }

            if(DoTesterSidePush(false)==10)
            {
                return true;
            }
            break;
        //==> Eastsun 20260511 F007 整合: Ifor 20220308 add: Index 兩段式上升 case 20000/20001 (從 a-side L8648-8660 verbatim)
        case 20000:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+Prod.TestZ2_Up_Offset, MOT[MTestZ2].GailSpeed*Prod.TestZ_Up_Speed/100, __FUNC__))
            {
                DoTestYRearDelay.SetSecAndOn(Prod.TestZ_Up_Wait);
                Task=20001;
            }
            break;
        case 20001:
            if(DoTestYRearDelay.Off())
            {
                Task=108;
            }
            break;
        //<== Eastsun 20260511 F007 整合
        }
    return false;
}

//---------------------------------------------------------------------------
//  PART FILE (stitch fragment, wave label k9-small-six) for aTester_Rear.cpp
//  Translator: AI(W7b-k9) 20260810
//  Golden: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp (9,787 lines,
//  cp950, 100% CRLF).  This fragment is UTF-8 + 100% CRLF, matching the port
//  aTester_Rear.cpp (2,448 CRLF / 0 bare LF).  Chinese comments transcribed
//  character for character from cp950; zero U+FFFD.
//
//  ROLE          -- Reset entry for the rear pre-pick purge state machine
//                   (blow the rear test-head nozzles before picking off the
//                   shuttle); rewinds its cursor to step 1.
//  WAVE SCOPE    -- InitRearTestPurgBeforePickShuttle()  ACTIVE  golden :9237-9240
//  GATE REGISTER -- none.
//  OUT OF MY RANGE (hand-off): golden :9234 `int iRearTestPurgBeforePickShuttle=1;`
//                   is not in my range; it already exists as a real data
//                   definition at atester_shims.cpp:200 (declared
//                   atester_shims.h:123) and may STAY there.  golden :9235
//                   `TQPF_Timer hDoRearTestPurgBeforePickShuttle;` is emitted in
//                   the SIBLING part file 09242_DoRearTestPurgBeforePickShuttle.txt
//                   (its only user), NOT here -- do not add a second definition.
//  INTEGRATE     -- retires the no-op stub atester_shims.cpp:211
//                   `void InitRearTestPurgBeforePickShuttle() {}`; the non-static
//                   declaration at atester_shims.h:135 stays.
//---------------------------------------------------------------------------
void InitRearTestPurgBeforePickShuttle()                                        //ChungHung 20150517 add for ATK Try to Fix TSMC Device Die Crack Issue Start
{
    iRearTestPurgBeforePickShuttle=1;
}

//---------------------------------------------------------------------------
//  PART FILE (stitch fragment, wave label k9-small-six) for aTester_Rear.cpp
//  Translator: AI(W7b-k9) 20260810
//  Golden: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp (9,787 lines,
//  cp950, 100% CRLF).  This fragment is UTF-8 + 100% CRLF, matching the port
//  aTester_Rear.cpp (2,448 CRLF / 0 bare LF).  Chinese comments transcribed
//  character for character from cp950; zero U+FFFD.
//
//  ROLE          -- Rear pre-pick purge state machine.  Moves MTestZ2 to the
//                   purge stand-off (TestZ2_Pick + offset*100), then repeats
//                   iPurgeBeforePickShuttleInterval times: vacuum OFF on every
//                   rear nozzle that is NOT holding an IC, wait
//                   iPurgeBeforePickShuttleTime, back to Normal, wait 1s.  On the
//                   last pass (case 400) it restores Normal and reports done.
//  WAVE SCOPE    -- hDoRearTestPurgBeforePickShuttle       ACTIVE  golden :9235
//                   DoRearTestPurgBeforePickShuttle(int)   ACTIVE  golden :9242-9321
//  GATE REGISTER -- none.  All symbols verified real in the port on 20260810:
//                   Prod.TestZ2_Pick (cprod.h:434, PROD_INFO_ST, extern Prod
//                   cprod.h:1138); DeviceForm_File.iPurgeBdforePickShuttleOffSet /
//                   iPurgeBeforePickShuttleInterval / iPurgeBeforePickShuttleTime
//                   (cprod.h:1219/1218/1217 in SYSTEM_DEVICE_FORM, extern
//                   cprod.h:1245); TMotor::Gali_MotMove(int,int,AnsiString="")
//                   (Motor/mymotor.h:201); BTestSuck.iShtRow/iShtCol/Item/Suck
//                   (aHotPlateSubstrate.h:406/438/370/369, object defined
//                   aHotPlateSubstrate.cpp:93); HAS_IC / HAS_HOT_IC (cmydef.h:158
//                   /160); TQPF_Timer::SetSecAndOn/Off (myTimer.h:38/30).
//  TRAP 5 (which header) -- BTestSuck is TMyKitSuck as declared in
//                   aHotPlateSubstrate.h:365 (extern :637) and DEFINED in
//                   aHotPlateSubstrate.cpp:93.  That is the layout this whole
//                   file already uses (its include list, port line 164).  The
//                   rival mykitsuck.h:274 TMyKitSuck has a DIFFERENT layout and
//                   is deliberately not relied on here.
//  TRAP 4 (static init) -- the one file-scope object added,
//                   hDoRearTestPurgBeforePickShuttle, is a TQPF_Timer whose ctor
//                   (myTimer.cpp:13-16) only calls its own private
//                   CalibratePerformanceCounterOverhead(), i.e. QueryPerformance-
//                   Counter into its OWN members -- it touches no global pointer.
//                   Four identical file-scope TQPF_Timer peers already live in
//                   this TU (port aTester_Rear.cpp:301/380/1112/1113/1121).  It is
//                   non-static, exactly as golden :9235, and nothing else in the
//                   tree declares that name (checked, see report).
//  GOLDEN QUIRKS preserved verbatim (NOT fixed):
//                   (a) case 100 FALLS THROUGH into case 200 when the interval
//                       counter has not expired (Task=200 with no break) -- the
//                       else branch is the only one that breaks;
//                   (b) the `if(...HAS_IC || ...HAS_HOT_IC) ;` empty-then /
//                       else-Off idiom appears three times (golden :9265-9269,
//                       :9286-9290, :9308-9312) and is kept as written;
//                   (c) `static int iCT=0;` is a FUNCTION-local static (no ctor,
//                       so not a TRAP 4 hazard) and is never reset on entry --
//                       only case 1 zeroes it, so an abort between case 100 and
//                       case 400 leaves the count where it was.  Faithful.
//  INTEGRATE     -- retires the offline stub atester_shims.cpp:216
//                   `bool DoRearTestPurgBeforePickShuttle(int) { return true; }`
//                   (that stub reported the purge instantly COMPLETE, which is a
//                   real behaviour change, not a cosmetic one); the non-static
//                   declaration at atester_shims.h:140 stays -- it is why this
//                   body is not static (TRAP 1 shape (d) avoided).
//---------------------------------------------------------------------------
// golden :9235 -- file-scope peer of DoRearTestPurgBeforePickShuttle, emitted
// here because this function is its ONLY user in the whole golden tree (checked
// 20260810, see report).  Same "file-scope peer added with its function" pattern
// Wave 1 used for golden :308 BTempRiseHasICDelay (port aTester_Rear.cpp:301).
TQPF_Timer hDoRearTestPurgBeforePickShuttle;
//---------------------------------------------------------------------------
bool DoRearTestPurgBeforePickShuttle(int isp)
{
    int &Task=iRearTestPurgBeforePickShuttle;
    static int iCT=0;

    switch(Task)
    {
        case 1:                                                                 //移至預吹氣位置
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Pick+DeviceForm_File.iPurgeBdforePickShuttleOffSet*100, isp))
            {
                iCT=0;
                Task=100;
            }
            break;
        case 100:
            if(iCT<DeviceForm_File.iPurgeBeforePickShuttleInterval)
            {
                iCT++;
                hDoRearTestPurgBeforePickShuttle.SetSecAndOn(DeviceForm_File.iPurgeBeforePickShuttleTime);
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_IC ||
                           BTestSuck.Item[i][j]==HAS_HOT_IC)                    //Steven 20151016 : Fixed for shuttle pruge function
                            ;
                        else
                            BTestSuck.Suck[i][j].Off();
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
            if(hDoRearTestPurgBeforePickShuttle.Off())
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_IC ||
                           BTestSuck.Item[i][j]==HAS_HOT_IC)                    //Steven 20151016 : Fixed for shuttle pruge function
                            ;
                        else
                            BTestSuck.Suck[i][j].Normal();
                    }
                }
                hDoRearTestPurgBeforePickShuttle.SetSecAndOn(1);
                Task=300;
            }
            break;
        case 300:
            if(hDoRearTestPurgBeforePickShuttle.Off())
            {
                Task=100;
            }
            break;
        case 400:
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]==HAS_IC ||
                       BTestSuck.Item[i][j]==HAS_HOT_IC)                        //Steven 20151016 : Fixed for shuttle pruge function
                        ;
                    else
                        BTestSuck.Suck[i][j].Normal();
                }
            }
            Task=500;
            break;
        case 500:
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// ===== W7b / k7-SLK-trio -- part 1 of 3 =======================================
//  Translator: AI(W7b-k7-SLK) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp
//  (9,787 lines, cp950, 100% CRLF).  This part translates golden :9323-9333.
//
//  ROLE -- BTestNeedDestroy(): "does the REAR (Arm2 / Z2) test-head suck grid
//  need a destroy-blow before the head may leave the socket?"  A pure BTestSuck
//  Item-grid predicate -- no HAL, no motion, no state written.  Live consumers
//  in this tree: atester.cpp:4363 and atester_32Site.cpp:1401.
//
//  WAVE SCOPE
//    * BTestNeedDestroy()                 golden :9323-9333   ACTIVE
//
//  GOLDEN BUG -- KEPT, NOT FIXED: golden's 2nd branch (`else if(BTestSuck.
//  All_HAS_NULL_IC())`, golden :9327) is textually IDENTICAL to the 1st
//  (golden :9325), so it is unreachable dead code.  Reproduced exactly as-is;
//  the plausible intent was FTestSuck or a different predicate, but changing it
//  would be a redesign, not a translation.
//
//  GATE REGISTER
//  [G-k7-1]  golden :9325 and :9327  --  BTestSuck.All_HAS_NULL_IC()
//    STATUS: golden TEXT gated (#if 0 GOLDEN VERBATIM block below); BEHAVIOUR
//            fully ACTIVE via a faithful free-function transcription.
//    WHY IT MUST STAY GATED (not just "the symbol is missing"): TMyKitSuck::
//      All_HAS_NULL_IC() is declared only on the OTHER TMyKitSuck -- mykitsuck.h
//      :408, body mykitsuck.cpp:595.  The object BTestSuck is defined at
//      aHotPlateSubstrate.cpp:93 against aHotPlateSubstrate.h:365's TMyKitSuck,
//      a DIFFERENT class with a DIFFERENT layout, and that one does not declare
//      All_HAS_NULL_IC.  mykitsuck.cpp is DELIBERATELY NOT REGISTERED in the
//      build (CMakeLists.txt:2099-2106: it re-defines 24 globals against
//      aHotPlateSubstrate.cpp's 14).  Linking it in to obtain one bool predicate
//      would silently re-offset every kit-grid field in the 177 TUs that use the
//      aHotPlateSubstrate.h layout.  So: a body exists and is even reachable in
//      principle -- and the gate still stays, for the ODR/layout reason.
//    WHY THE LIVE PATH IS FAITHFUL (NOT AN OFFLINE DEFAULT): golden's method
//      body (golden MyKitSuck.cpp:406-430) reads ONLY iMaxRow, iMaxCol,
//      Item[][], HAS_NULL_IC and NULL_IC.  aHotPlateSubstrate.h's TMyKitSuck
//      already exposes all of them (iMaxRow :377, iMaxCol :402, Item :370), so
//      W7bK7_All_HAS_NULL_IC(kit) below is a statement-for-statement copy of the
//      golden method as a free function -- identical return value for identical
//      grid state.  Established precedent in THIS SAME FILE for exactly this
//      shape: W64bT2_SetUnuseToNullIC (aTester_Rear.cpp:981-995).
//    REAL-MACHINE BEHAVIOUR DIFFERENCE: none.
//    ABSENCE COMMAND (run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//      rg -n "All_HAS_NULL_IC" aHotPlateSubstrate.h            -> exit 1, no output
//      rg -n "All_HAS_NULL_IC" --glob '*.h' --glob '*.cpp' -g '!build*' .
//                                                              -> only atester.cpp
//                                                                 :3561 (inside the
//                                                                 #if 0 GATE
//                                                                 G-PTk2-ProcessTest
//                                                                 Result region),
//                                                                 mykitsuck.h:408,
//                                                                 mykitsuck.cpp:44/595
//      rg -n "^\s*mykitsuck\.cpp" CMakeLists.txt              -> exit 1, no output
//    MEASURED AT: 2026-08-10 19:10:33 +0800.
//
//  TRAP 1 (archive extraction) -- BTestNeedDestroy below has EXTERNAL linkage;
//  no `static`, matching the non-static declaration at atester_shims.h:138.
//  INTEGRATE MUST DELETE the no-op stub `bool BTestNeedDestroy() { return false; }`
//  at atester_shims.cpp:214, otherwise this is TRAP-1 shape (b): the stub already
//  satisfies every reference, the real body is never extracted, the build is green
//  and nothing changed.  nm --undefined-only cannot see that.
//
//  TRAP 4 (static init) -- this part adds NO file-scope object of any kind.
//------------------------------------------------------------------------------
// STITCH REQUIREMENT (parts 2 and 3 of this trio): the SLK state machines below
// deref Sen[] (mysensor.h:48) and Cylinder[] (mycylin.h:176), and NEITHER header
// is reachable from aTester_Rear.cpp's current include block nor transitively
// from MachineDefine.h / cmydef.h / csystem.h / aHotPlateSubstrate.h (checked
// 2026-08-10 19:10 +0800).  Both headers carry include guards (mysensorH,
// mycylinH), so these two lines are idempotent -- at integrate please HOIST them
// into the file-top include block next to "Motor/mymotor.h" and delete them here.
#include "mysensor.h"               // Sen[]      -- SnSocketClampPush1/2, SnSocketHasClamp1/2, SnSocketClampPull1/2
#include "mycylin.h"                // Cylinder[] -- C_Socket_Clamp/Unclamp, C_SLK1_Clamp/Unclamp
//------------------------------------------------------------------------------
// -- TMyKitSuck::All_HAS_NULL_IC (golden MyKitSuck.h:306, body golden
//    MyKitSuck.cpp:406-430) -- FAITHFUL free-function transcription; see
//    [G-k7-1] above for why the golden member call itself stays gated.
//    Statement order, counter names and the three-way count/decision are
//    golden's verbatim; nothing is defaulted or approximated.
static bool W7bK7_All_HAS_NULL_IC(TMyKitSuck &kit)
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
#if 0 // GOLDEN VERBATIM -- golden aTester_Rear.cpp:9323-9333  (GATE G-k7-1, begin)
bool BTestNeedDestroy()
{
    if(BTestSuck.All_HAS_NULL_IC())
        return true;
    else if(BTestSuck.All_HAS_NULL_IC())
        return true;
    else if(BTestSuck.UseSiteNoIC())
        return true;
    else
        return false;
}
#endif // GOLDEN VERBATIM -- golden aTester_Rear.cpp:9323-9333  (GATE G-k7-1, end)
bool BTestNeedDestroy()
{
    if(W7bK7_All_HAS_NULL_IC(BTestSuck))                                        // AI(W7b-k7-SLK) 20260810: golden :9325 BTestSuck.All_HAS_NULL_IC() -- faithful local transcription, see [G-k7-1]
        return true;
    else if(W7bK7_All_HAS_NULL_IC(BTestSuck))                                   // AI(W7b-k7-SLK) 20260810: golden :9327 -- golden's own duplicate of :9325 (unreachable), preserved
        return true;
    else if(BTestSuck.UseSiteNoIC())
        return true;
    else
        return false;
}

//------------------------------------------------------------------------------
// ===== W7b / k7-SLK-trio -- part 2 of 3 =======================================
//  Translator: AI(W7b-k7-SLK) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp
//  (9,787 lines, cp950, 100% CRLF).  This part translates golden :9335-9427,
//  plus the three file-scope TQPF_Timer peers it needs (golden :95-97, see
//  PREREQUISITE below).
//
//  ROLE -- BTestSeparateSLK(bool bReset): the "Socket-clamp separability" MODE
//  hand-off SM for the FRONT-arm-places / REAR-arm-tests configuration.  Z1 is
//  already down at test height; this SM closes the Socket clamp, breaks vacuum
//  so the ICs are left standing in the sockets, then releases the SLK1 clamp and
//  lifts Arm1 5 mm (1500 counts) clear before the test fires.  Returns true only
//  on the final lift completing.  bReset (or the feature being off) re-homes the
//  cursor to Task=1 and returns false.
//
//  WAVE SCOPE
//    * BTestSeparateSLK(bool)             golden :9335-9427   ACTIVE
//
//  GATE REGISTER -- EMPTY.  This part writes NO #if 0 of its own and makes NO
//  offline substitution: every symbol golden touches here already exists in the
//  port and was verified 2026-08-10 19:10:33 +0800 --
//    Cylinder[] mycylin.h:176 (.On/.Off :?/.OnStatus :117); C_Socket_Clamp /
//    C_Socket_Unclamp / C_SLK1_Clamp / C_SLK1_Unclamp cmydef.h:358-363;
//    Sen[] mysensor.h:48 (.IsOn :42); SnSocketClampPush1/2, SnSocketHasClamp1/2
//    cmydef.h; FTestSuck aHotPlateSubstrate.cpp:92 (iShtRow :406, iShtCol :438,
//    Suck :369, TMySucker::Normal :130); MOT[] Motor/mymotor.h (Gali_MotMove
//    :201); MTestZ1 cmydef.h:2115; MTestY1 cmydef.h:2114; MMIndex cmydef.h:2401;
//    iSpeedSlow cmydef.h:3293; Prod.TestZ1_Test cprod.h:431;
//    TestIF_File.bUseSLKClamp / .iSeparabilityTest / .bArm1PickPlaceArm2Test
//    cprod.h; IniConfig.bD58UseArm1PickPlaceArm2Test Config.h:563;
//    ShowErrorMessage / K_RETRY canary_support.h; iBTestSocketClampCloseTask
//    atester_shims.cpp:198.
//
//  PREREQUISITE ADDITION (outside the stated 9335..9427 range -- flagged, not
//  smuggled): golden defines the three clamp timers at FILE SCOPE, golden
//  aTester_Rear.cpp:95-97, in the same block as bArm2PutDuplicateErr (golden
//  :89-92) and hBTestTimeOutDelay (golden :94).  They are referenced ONLY by
//  BTestSeparateSLK and BTestCombineSLK in the whole golden tree (checked: rg
//  over golden *.h/*.cpp finds hits only at golden aTester_Rear.cpp:9360-9577),
//  and no golden header declares them extern.  They are absent from the port
//  (rg -n "BTestSocketClampCloseDelay|BTestSocketClampOpenDelay|
//  BTestSocketClampTimeOutDelay" --glob '*.h' --glob '*.cpp' -g '!build*' .
//  -> exit 1, no output, measured 2026-08-10 19:10:33 +0800), so without them
//  this part does not link.  Emitted VERBATIM from golden :95-97, non-static,
//  exactly as Wave 1 of this file already did for its own file-scope peer
//  `TQPF_Timer BTempRiseHasICDelay;` (aTester_Rear.cpp:301, golden :308).
//    DUPLICATE-DEFINITION HAND-OFF: if a SIBLING agent in this same wave emits
//    golden :94-98 as one block (hBTestTimeOutDelay .. BTorqueTimeOutDelay --
//    both of those are still absent from the port and DoTestYRear /
//    DoBTestSuckTestIC need them), the stitch will double-define these three.
//    Integrate must keep exactly ONE copy.
//
//  TRAP 1 (archive extraction) -- BTestSeparateSLK below has EXTERNAL linkage;
//  no `static`.  atester_shims.h:144 declares it as
//  `bool BTestSeparateSLK(bool bReset=false);` -- the DEFAULT ARGUMENT LIVES IN
//  THAT DECLARATION and is deliberately NOT repeated here (repeating it is a
//  hard compile error once both are visible in one TU, and atester_shims.h is
//  included by aTester_Rear.cpp:151).  INTEGRATE MUST DELETE the no-op stub
//  `bool BTestSeparateSLK(bool) { return true; }` at atester_shims.cpp:220 --
//  otherwise TRAP-1 shape (b): the stub keeps satisfying every reference, this
//  body is never extracted, and the SM silently reports "separate done" forever,
//  which on a real machine means the test fires with vacuum still holding.
//
//  TRAP 4 (static init) -- the three TQPF_Timer objects below DO run a ctor
//  before main() (myTimer.cpp:14-17 -> CalibratePerformanceCounterOverhead(),
//  1001 x QueryPerformanceCounter).  That ctor touches ONLY its own members and
//  a Win32 API -- it dereferences NO global pointer, so it cannot repeat the
//  fLaserSensor/elLaser failure mode.  Identical, already-shipped precedent in
//  this same TU: aTester_Rear.cpp:301.
//
//  SOFT_SIMULTE -- NOT defined in this tree (file-top rule, aTester_Rear.cpp
//  :139-143).  Both #ifdef SOFT_SIMULTE bodies below are therefore inert and are
//  reproduced VERBATIM, and the `#ifndef SOFT_SIMULTE / int ret; / #endif`
//  declaration at golden :9339-9341 IS live -- `ret` really is declared.
//------------------------------------------------------------------------------
// PREREQUISITE, golden aTester_Rear.cpp:95-97 (verbatim) -- see banner above.
TQPF_Timer BTestSocketClampCloseDelay;                                          //JerryYang 20160524
TQPF_Timer BTestSocketClampOpenDelay;                                           //JerryYang 20160524
TQPF_Timer BTestSocketClampTimeOutDelay;                                        //JerryYang 20160524
//------------------------------------------------------------------------------
bool BTestSeparateSLK(bool bReset)                                              //JerryYang 20160429 Socket clamp模式, Z1將IC放到Socket流程
{
    bool bResult=false;
    int &Task=iBTestSocketClampCloseTask;
    #ifndef SOFT_SIMULTE
    int ret;
    #endif

    if(bReset ||
       TestIF_File.bUseSLKClamp==false ||                                       //JerryYang 20180518 (wei) : 新增保護
       TestIF_File.iSeparabilityTest==0 ||
       (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
        TestIF_File.bArm1PickPlaceArm2Test==true))                              //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
    {
        Task=1;
        return bResult;
    }

    switch(Task)
    {
        case 1:
            {
                Cylinder[C_Socket_Unclamp].Off();
                Cylinder[C_Socket_Clamp].On();                                  //Z1已經下降到測試高度,Socket clamp夾持
                Task=10;
                BTestSocketClampCloseDelay.SetMSAndOn(300);
                BTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
            }
            break;
        case 10:
            if(BTestSocketClampCloseDelay.Off())
            {
                #ifdef SOFT_SIMULTE
                    Task=20;
                #else
                if(Sen[SnSocketClampPush1].IsOn()==true && Sen[SnSocketClampPush2].IsOn()==true &&
                   Sen[SnSocketHasClamp1].IsOn()==true && Sen[SnSocketHasClamp2].IsOn()==true)
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            FTestSuck.Suck[i][j].Normal();                      //破真空將IC放到SOCKET上
                        }
                    }
                    Task=20;
                }
                else
                {
                    if(BTestSocketClampTimeOutDelay.Off())
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
        case 20:                                                                // Socket clamp夾持後,SLK1 放開clamp
            Cylinder[C_SLK1_Clamp].Off();
            Cylinder[C_SLK1_Unclamp].On();
            BTestSocketClampCloseDelay.SetMSAndOn(500);
            BTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
            Task=30;
            break;
        case 30:
            if(BTestSocketClampCloseDelay.Off())
            {
                #ifdef SOFT_SIMULTE
                    Task=40;
                #else
                if(Cylinder[C_SLK1_Unclamp].OnStatus())
                    Task=40;
                if(BTestSocketClampTimeOutDelay.Off())
                {
                    ret=ShowErrorMessage("JAM0373", K_RETRY, MTestY1);          //ARM1 SLK放開clamp異常
                    if(ret==K_RETRY)
                    {
                        Task=20;
                    }
                }
                #endif
            }
            break;
        case 40:                                                                //JerryYang 2016042 Arm 1上升5mm再測試
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+1500, iSpeedSlow))
                bResult=true;
    }
    return bResult;
}

//------------------------------------------------------------------------------
// ===== W7b / k7-SLK-trio -- part 3 of 3 =======================================
//  Translator: AI(W7b-k7-SLK) 20260810
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp
//  (9,787 lines, cp950, 100% CRLF).  This part translates golden :9429-9644.
//
//  ROLE -- BTestCombineSLK(bool bReset): the mirror of BTestSeparateSLK.  Test
//  has finished and the ICs are standing in the sockets; this SM releases SLK1,
//  brings Arm1 back down to test height, re-clamps SLK1 onto the ICs, opens the
//  Socket clamp, then sucks every site that was in use back onto the nozzles
//  (per-site latches bFTSuckFinish / bFTestSuckUse) and returns true.  Early-out
//  at Task=10 returns true immediately when Arm1 carried no IC at all.
//
//  WAVE SCOPE
//    * BTestCombineSLK(bool)              golden :9429-9644   ACTIVE
//
//  GATE REGISTER -- EMPTY.  No #if 0 and no offline substitution: every symbol
//  golden touches here exists in the port.  Beyond part 2's verified list, this
//  part additionally needs (all verified 2026-08-10 19:10:33 +0800):
//    iBTestSocketClampOpenTask atester_shims.cpp:199; BTestSocketClampOpenDelay
//    (added by part 2 from golden :96); Cylinder[].OnSensor() mycylin.h:119;
//    Sen[].IsOff() mysensor.h:43; SnSocketClampPull1/2 cmydef.h; FTestSuck
//    .iMaxRow/.iMaxCol/.Item/.Suck[][].Suck()/.Suck[][].Error (aHotPlate
//    Substrate.h :377/:402/:370, TMySucker::Suck :119, TMySucker::Error :109);
//    MOT[MTestZ1].Gali_ReadPos() Motor/mymotor.h:214; NULL_IC / HAS_NULL_IC
//    cmydef.h; MAX_SOCKET_ROW/MAX_SOCKET_COL MachineType.h:391-392 (= 4 and 8,
//    exactly matching the 4x8 initialiser literals golden writes AND
//    aHotPlateSubstrate.h's _MAX_SUCK_ROW_ITEM/_MAX_SUCK_COL_ITEM :102-103, so
//    the iMaxRow/iMaxCol reset walk at golden :9560-9568 cannot overrun the two
//    MAX_SOCKET_*-dimensioned latch arrays).
//
//  TRAP 4 (static init) -- the three statics below (bSLK1NeedClamp,
//  bFTSuckFinish[][], bFTestSuckUse[][]) are FUNCTION-scope statics of scalar
//  type with constant initialisers: zero dynamic initialisation, no ctor, no
//  global dereferenced before main().  This part adds NO file-scope object.
//
//  TRAP 5 (two headers, same class name) -- FTestSuck is the
//  aHotPlateSubstrate.h:365 TMyKitSuck, NOT mykitsuck.h:274's.  The object is
//  defined at aHotPlateSubstrate.cpp:92 (`TMyKitSuck FTestSuck;`) and
//  aTester_Rear.cpp:164 already includes aHotPlateSubstrate.h for exactly this
//  reason; mykitsuck.cpp is deliberately unregistered (CMakeLists.txt:2099-2106).
//  Every member this part reads was checked against aHotPlateSubstrate.h, line by
//  line, in the GATE REGISTER note above.
//
//  TRAP 1 (archive extraction) -- BTestCombineSLK below has EXTERNAL linkage; no
//  `static`.  atester_shims.h:143 declares it as
//  `bool BTestCombineSLK(bool bReset=false);`; the default argument stays THERE
//  and is deliberately not repeated here.  INTEGRATE MUST DELETE the no-op stub
//  `bool BTestCombineSLK(bool) { return true; }` at atester_shims.cpp:219 --
//  otherwise TRAP-1 shape (b): the stub answers every reference, this body never
//  gets extracted from the archive, and on a real machine the arm would leave the
//  socket without ever sucking the tested ICs back up.
//
//  SOFT_SIMULTE -- NOT defined in this tree; the one #ifdef SOFT_SIMULTE body
//  (golden :9538-9540, case 70) is inert and reproduced VERBATIM.
//
//  Note on faithfulness: golden re-arms BTestSocketClampTimeOutDelay AFTER
//  setting Task in case 40 / case 60 (Task=50 / Task=70 come first) -- statement
//  order preserved exactly.  Integer literals (300 / 500 / 2.5 / 1500) and the
//  `Prod.TestZ1_Test` target are copied verbatim; no arithmetic was rewritten,
//  so no int/int was promoted to floating point anywhere in this part.
//------------------------------------------------------------------------------
bool BTestCombineSLK(bool bReset)                                               //JerryYang 20160429 Socket clamp模式, 測試完成 Z1要將IC從Socket吸起流程
{
    static bool bSLK1NeedClamp=false;
    static bool bFTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bFTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false,false,false,false,false,false,false,false},
                                                               {false,false,false,false,false,false,false,false},
                                                               {false,false,false,false,false,false,false,false},
                                                               {false,false,false,false,false,false,false,false}};

    int &Task=iBTestSocketClampOpenTask;
    int ret=0;
    bool bCheckAllSuck=false, flag=false;
    bool bResult=false;

    if(bReset ||
       TestIF_File.bUseSLKClamp==false ||                                       //JerryYang 20180518 (wei) : 新增保護
       TestIF_File.iSeparabilityTest==0 ||
       (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
        TestIF_File.bArm1PickPlaceArm2Test==true))                              //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
    {
        Task=1;
        for(int i=0; i<FTestSuck.iMaxRow; i++)
        {
            for(int j=0; j<FTestSuck.iMaxCol; j++)
            {
                bFTestSuckUse[i][j]=false;
                bFTSuckFinish[i][j]=false;
            }
        }
        return bResult;
    }

    switch(Task)
    {
        case 1:
            {
                bSLK1NeedClamp=false;
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]!=NULL_IC &&
                           FTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            FTestSuck.Suck[i][j].Error=false;
                            bFTestSuckUse[i][j]=true;
                            bSLK1NeedClamp=true;
                        }
                        bFTSuckFinish[i][j]=false;                              //Steven 20110301
                    }
                }
                Task=10;
            }
            break;
        case 10:
            if(bSLK1NeedClamp==true)
            {
                Task=15;                                                        //JerryYang 20160714 修正index上有已測IC, 回home後hang up問題
            }
            else
            {
                return true;                                                    //arm1上無IC 無須進行流程
            }
            break;
        case 15:
            if(MOT[MTestZ1].Gali_ReadPos()==Prod.TestZ1_Test)                   //JerryYang 20160714 修正index上有已測IC, 回home後hang up問題
            {
                Task=30;
            }
            else
            {
                Cylinder[C_SLK1_Unclamp].On();
                Cylinder[C_SLK1_Clamp].Off();
                BTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
                Task=20;
            }
            break;
        case 20:                                                                //確認SLK Clamp放開
            if(Cylinder[C_SLK1_Unclamp].OnSensor())
            {
                Task=30;
            }
            else if(BTestSocketClampTimeOutDelay.Off())
            {
                ret=ShowErrorMessage("JAM0373", K_RETRY, MTestY1);              //JerryYang 20160714 ARM1 SLK放開clamp異常
                if(ret==K_RETRY)
                {
                    Task=15;
                }
            }
            break;
        case 30:                                                                //測試完,Arm1要下降
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test, iSpeedSlow))
            {
                Task=40;
            }
            break;
        case 40:                                                                //SLK Clamp 夾持
            Cylinder[C_SLK1_Unclamp].Off();
            Cylinder[C_SLK1_Clamp].On();
            BTestSocketClampOpenDelay.SetMSAndOn(500);
            Task=50;
            BTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
            break;
        case 50:
            if(BTestSocketClampOpenDelay.Off())
            {
                if(Cylinder[C_SLK1_Clamp].OnSensor())
                {
                    Task=60;
                }
                else
                {
                    if(BTestSocketClampTimeOutDelay.Off())
                    {
                        ret=ShowErrorMessage("JAM0371", K_RETRY, MTestY1);      //ARM2 SLK clamp未到位
                        if(ret==K_RETRY)
                        {
                            Task=40;
                        }
                    }
                }
            }
            break;
        case 60:
            Cylinder[C_Socket_Clamp].Off();                                     //Socket Clamp 打開
            Cylinder[C_Socket_Unclamp].On();
            BTestSocketClampOpenDelay.SetMSAndOn(500);
            Task=70;
            BTestSocketClampTimeOutDelay.SetSecAndOn(2.5);
            break;
        case 70:
            if(BTestSocketClampOpenDelay.Off())
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
                    if(BTestSocketClampTimeOutDelay.Off())
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

//---------------------------------------------------------------------------
//  PART FILE (stitch fragment, wave label k9-small-six) for aTester_Rear.cpp
//  Translator: AI(W7b-k9) 20260810
//  Golden: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp (9,787 lines,
//  cp950, 100% CRLF).  This fragment is UTF-8 + 100% CRLF, matching the port
//  aTester_Rear.cpp (2,448 CRLF / 0 bare LF).  Chinese comments transcribed
//  character for character from cp950; zero U+FFFD.
//
//  ROLE          -- "Index arm 2 sucks + swaps state" helper (golden comment:
//                   JerryYang 20190123).  For every rear nozzle it reconciles the
//                   rear-left carry kit (BLCarryKit, the shuttle side) against the
//                   rear test-head kit (BTestSuck): re-Normals nozzles to stop
//                   vacuum droop, fires Suck() where an IC is waiting, MOVES the
//                   per-site data from the carry kit into the test-head kit, logs
//                   the pick, bumps the Index input counter and the per-nozzle
//                   picker-life counter, and marks bArm2SuckFinish per site.  When
//                   CosFunction.bUseShuttlePickShiftDetect + dDropByPassDetect +
//                   bIndexZ2NeedUp are all set (ASEM: rise first, then judge) the
//                   DATA MOVE is deliberately deferred (bNeedWaitUp==true) while
//                   the flags are still set -- preserved exactly.
//  WAVE SCOPE    -- DoArm2Suck()  ACTIVE  golden :9646-9746
//                   (one statement inside it GATED: golden :9724)
//  STITCH ORDER REQUIREMENT (read this before placing the fragment): this body
//                   reads `bIndexZ2NeedUp`, which is a TU-LOCAL
//                   `static bool bIndexZ2NeedUp=false;` -- static in GOLDEN TOO
//                   (golden aTester_Rear.cpp:67) and already present at port
//                   aTester_Rear.cpp:1109.  So this fragment MUST be stitched
//                   AFTER port line 1109, and must NOT define its own copy.  It
//                   also reuses the Wave-1 TU-local gap stubs at port lines
//                   317-367, so "append after the existing content" is correct.
//  EXPIRED GATES RETIRED HERE (TRAP 3 re-asked per symbol -- the answer for all
//  four is "go ACTIVE", and each was verified in the port on 20260810):
//                   * golden :9689 / :9716 BTestSuck.MoveSuckData(BLCarryKit,i,j)
//                     -- the REAL method now exists:
//                     `void TMyKitSuck::MoveSuckData(TMyKitSuck &Source, int
//                     SourceR, int SourceC, int TargetR=-1, int TargetC=-1)`
//                     declared aHotPlateSubstrate.h:598, defined
//                     aHotPlateSubstrate.cpp:1406, and its -1/-1 default path
//                     (aHotPlateSubstrate.cpp:1408-1412) resolves target to the
//                     SAME row/col, which is exactly what golden 3-arg form means.
//                     So calling it directly is MORE faithful than reusing Wave-1
//                     no-op W64bT2_MoveSuckData -- that stub silently dropped the
//                     move.  (I do NOT touch the older call sites that still use
//                     the stub -- port lines 602/2168/2357 are not my file; they
//                     are an expired-gate cleanup for the main loop.)
//                   * golden :9717 BTestSuck.PordRec[i][j].AddIndexPickShuttle-
//                     Record(TestIF.iSiteMap[i][j], i, j) -- REAL: declared
//                     Public/MyProductionRecord.h:108, defined
//                     Public/MyProductionRecord.cpp:577 (3 args, exact match), and
//                     runtime-safe because the ctor allocates asBuffer
//                     (Public/MyProductionRecord.cpp:135-140) -- so no NULL deref
//                     from a by-value PordRec member.  TestIF.iSiteMap is
//                     cprod.h:1653 int[MAX_SOCKET_ROW][MAX_SOCKET_COL].
//                   * golden :9721 LastSet.iIndexInputOutPut[0]++ -- REAL: the
//                     canary_support.h LAST_GENERAL_SET shim was RETIRED (it now
//                     just `#include "LastSet.h"`, canary_support.h:49-59); the
//                     field is `long iIndexInputOutPut[4]` at LastSet.h:433 and
//                     the object is defined at LastSet.cpp:39.  NOTE for the main
//                     loop: the SAME golden counter is still gated at port
//                     aTester_Rear.cpp:584-586 for golden :512 -- that gate is now
//                     EXPIRED (its premise, "shim lacks the field", is dead).  Not
//                     my file to edit; reported instead.
//                   * golden :9723 CheckPickerLifeNeedOneCycle() -- REAL body at
//                     csystem.cpp:15747 with NO enclosing #if 0 (verified by
//                     preprocessor-depth scan), and csystem.cpp is in the SAME
//                     static archive as aTester_Rear.cpp (both inside
//                     add_library(ht9045_sm STATIC ...), CMakeLists.txt:1387 with
//                     aTester_Rear.cpp:1829 and csystem.cpp:1891) -- so this is a
//                     genuine intra-archive resolution, not TRAP 1 (a)/(c).  There
//                     is no rival global stub: the only other definition is
//                     aoutarm.cpp:574, a `static` TU-LOCAL macro seam that cannot
//                     satisfy or shadow my call (that seam is main-loop task #15,
//                     not mine).
//  GATE REGISTER (the single #if 0 in this fragment):
//   * golden line :9724 -- BTestSuck.PordRec[i][j].AddIndexPickVacuum(
//     BTestSuck.Suck[i][j].GetIOValue())  //Sam 20230210 VacuumUnit module.
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: this statement is pure telemetry --
//     it writes the measured vacuum reading of the just-completed pick into the
//     per-site production record string buffer.  Nothing in DoArm2Suck, and
//     nothing in the rear index state machine, reads that field back; control
//     flow, IC state, counters and error latches are all untouched.  Skipping it
//     therefore cannot change any decision the SM makes offline.
//     REAL-MACHINE DIFFERENCE: the per-IC production log row for a rear index
//     pick will have an EMPTY vacuum-value column, so downstream yield/vacuum
//     analysis of rear picks loses that one measurement per IC.  No motion, no
//     interlock and no alarm behaviour changes.
//     TWO INDEPENDENT ABSENCES, both must be fixed before un-gating:
//       (1) TMyProductionRecord::AddIndexPickVacuum is not even DECLARED --
//           Public/MyProductionRecord.h:31 says so in as many words ("also stays
//           undeclared") and there is no definition anywhere.
//       (2) TMySucker::GetIOValue() does not exist on the TMySucker layout this
//           TU uses.  THIS IS TRAP 5, not a plain absence: GetIOValue IS real at
//           mykitsuck.cpp:2902, but it belongs to mykitsuck.h TMySucker, whereas
//           BTestSuck.Suck[][] is the aHotPlateSubstrate.h:106 TMySucker (object
//           defined aHotPlateSubstrate.cpp:93, class aHotPlateSubstrate.h:365) --
//           a DIFFERENT class with a different layout, whose member list
//           (aHotPlateSubstrate.h:106-140) has Suck/Destroy/On/Off/OnSuck/
//           OnDestroy/OffDestroy/Normal/GetStatus but NO GetIOValue.  Reaching for
//           the mykitsuck.h one would link and then read the wrong offsets.
//     ABSENCE COMMANDS (TRAP 2 -- re-run these at integration; they were run at
//     2026-08-10 19:21:41 +0800, and BOTH must still come back empty):
//       grep -rn "AddIndexPickVacuum" --include=*.h --include=*.cpp D:/HT9045/HT9011UC_Cpp_V3.33.906.0 | grep -v atester_32Site | grep -v tests
//       grep -n "GetIOValue" D:/HT9045/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.h
//     (the first command intentionally excludes atester_32Site.cpp, whose two
//     hits are its own W5_32S_ADD_INDEX_PICK_VACUUM no-op macro for this SAME
//     golden gap, and tests/, whose hit is a comment.)
//  SOFT_SIMULTE  -- not defined in this build; golden :9665-9671 (the
//                   fMain->cbIndexDrop drop-injection block) is reproduced
//                   VERBATIM and inert, per this file existing convention.
//  INTEGRATE     -- retires the no-op stub atester_shims.cpp:212
//                   `void DoArm2Suck() {}` (that stub made the rear index arm
//                   never suck and never move data -- a real behaviour change);
//                   keep the non-static declaration atester_shims.h:136.
//                   bArm2SuckFinish[][] (atester_shims.cpp:202) is plain data and
//                   STAYS -- this fragment only reads/writes it.
//---------------------------------------------------------------------------
void DoArm2Suck()                                                               //JerryYang 20190123 把index arm吸真空&交換狀態包成函式
{
    bool bNeedWaitUp=false;

    if(CosFunction.bUseShuttlePickShiftDetect==true &&
       DeviceForm_File.dDropByPassDetect!=0 &&
       bIndexZ2NeedUp==true)                                                    //Ifor 20220906 add:ASEM要求Shuttle 吸料後上升設定高度再判斷是否有無吸到IC
    {
        bNeedWaitUp=true;
    }
    else
    {
        bNeedWaitUp=false;
    }

    for(int i=0; i<BTestSuck.iShtRow; i++)
    {
        for(int j=0; j<BTestSuck.iShtCol; j++)
        {
            #ifdef SOFT_SIMULTE
            if(fMain->cbIndexDrop->Checked==true)
            {
                bArm2SuckFinish[0][1]=true;
                BTestSuck.Suck[0][1].Error=true;
            }
            #endif

            if(BLCarryKit.Item[i][j])                                           // && bArm2SuckFinish[i][j]==false)
            {
                if(BLCarryKit.Item[i][j]==HAS_NULL_IC)
                {
                    if(BTestSuck.Item[i][j]==HAS_NULL_IC ||
                       BTestSuck.Item[i][j]==NULL_IC)                           //Steven 20111202 : Retry會掉料
                    {
                        BTestSuck.Suck[i][j].Normal();                          //Steven 20111201 : 預防負壓壓降
                    }

                    if(BLCarryKit.Item[i][j]==HAS_NULL_IC &&
                       BTestSuck.Item[i][j]==NULL_IC)                           //Steven 20160817 : 移到上面
                    {
                        BTestSuck.Suck[i][j].Normal();
                        if(bNeedWaitUp==false)
                        {
                            BTestSuck.MoveSuckData(BLCarryKit, i, j);
                        }
                    }
                    else if(BLCarryKit.Item[i][j]==HAS_NULL_IC)                 //Index有料, 蝦頭沒料, 這個怪怪的
                    {
                        if(bNeedWaitUp==false)
                        {
                            BLCarryKit.SetItemData(i, j, NULL_IC);
                        }
                    }
                    else                                                        //蝦頭有料, index也有料, 怪怪的
                    {
                    }

                    bArm2SuckFinish[i][j]=true;                                 //Steven 20110301
                }
                else if(BLCarryKit.Item[i][j]!=NULL_IC)
                {
                    if(BTestSuck.Suck[i][j].Error)                              //Steven 20110301 : 有錯誤的不做
                    {
                        bArm2SuckFinish[i][j]=true;
                    }
                    else if(BTestSuck.Item[i][j]==NULL_IC &&                    //Steven 20160817 (Jou) : 不能吸了又吸
                            BTestSuck.Suck[i][j].Suck())
                    {
                        if(bNeedWaitUp==false)
                        {
                            BTestSuck.MoveSuckData(BLCarryKit, i, j);
                            BTestSuck.PordRec[i][j].AddIndexPickShuttleRecord(TestIF.iSiteMap[i][j], i, j);             //Sam 20201216 : Add record  //Sam 20200716 : Add record
                            bArm2DuplicateErr[i][j]=false;                      //Steven 20100105
                            bArm2SuckFinish[i][j]=true;                         //Steven 20110301 : 吸取完成的不做
                            if(bRunAutoClean==false)                            //kevin 20120531
                                LastSet.iIndexInputOutPut[0]++;                 //kevin 20120511 Index 吸取IC計數
                            TestIF_File.Arm2PickerLifeCnt[i][j]++;              //JerryYang 20220923 : add
                            CheckPickerLifeNeedOneCycle();                      //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化
                            //AI(W906-W2-TRAP5) 20260926: gate LIFTED -- A4-6 (8ff6c754) merged the two TMyKitSuck/TMySucker classes into the one golden layout, so the TRAP-5 premise is dead: TMySucker::GetIOValue() (mykitsuck.h:237, mykitsuck.cpp) and TMyProductionRecord::AddIndexPickVacuum(double) (Public/MyProductionRecord.h:188, MyProductionRecord.cpp) are both defined (nm). GetIOValue reads two 1203 DI bytes via MyLaneIO (no comms wait); without HAVE_PCI1203 it returns 999.0 -- telemetry only -- golden :9724
                            BTestSuck.PordRec[i][j].AddIndexPickVacuum(BTestSuck.Suck[i][j].GetIOValue());              //Sam 20230210 : 新增 VacuumUnit 通訊模組
                            //AI(W906-W2-TRAP5) 20260926: (end of lifted gate)
                        }
                        bArm2DuplicateErr[i][j]=false;                          //Steven 20100105
                        bArm2SuckFinish[i][j]=true;                             //Steven 20110301 : 吸取完成的不做
                    }
                    else
                    {
                        bArm2SuckComplete=false;                                //jou 2011-08-16 只要有未完成的就繼續等
                    }
                }
            }
            else
            {
                if(BTestSuck.Item[i][j]==HAS_NULL_IC ||
                   BTestSuck.Item[i][j]==NULL_IC)                               //Steven 20111202 : Retry會掉料
                {
                    BTestSuck.Suck[i][j].Normal();                              //Steven 20111201 : 預防負壓壓降
                }
                bArm2SuckFinish[i][j]=true;                                     //Steven 20110301 : 沒有東西的地方要跳過
            }
        }
    }
}

//---------------------------------------------------------------------------
//  PART FILE (stitch fragment, wave label k9-small-six) for aTester_Rear.cpp
//  Translator: AI(W7b-k9) 20260810
//  Golden: HT9011UC_Code_V3.33.906.0_20260618/aTester_Rear.cpp (9,787 lines,
//  cp950, 100% CRLF).  This fragment is UTF-8 + 100% CRLF, matching the port
//  aTester_Rear.cpp (2,448 CRLF / 0 bare LF).  Chinese comments transcribed
//  character for character from cp950; zero U+FFFD.
//
//  ROLE          -- "Index arm 2 suck-back (D44) vacuum re-check" helper (golden
//                   comment: JerryYang 20190123).  On negative-pressure heads
//                   (INDEX_SUCKER_TYPE==1) it first asks the IO view to run the
//                   suck-back detection, then -- once the destroy delay has
//                   expired and the check passed -- walks the rear nozzles: any
//                   nozzle latched as "needs re-check" whose vacuum sensor has
//                   already let go is un-latched and returned to Normal; and on
//                   INDEX_SUCKER_TYPE==1 every nozzle is returned to Normal
//                   unconditionally (golden comment: negative pressure must not
//                   stay on).
//  WAVE SCOPE    -- DoArm2D44VacCheck()  ACTIVE  golden :9748-9786
//                   (three statements routed through Wave-1 gap stubs: golden
//                   :9753, :9767, :9774)
//  STITCH ORDER REQUIREMENT -- reuses the Wave-1 TU-local stubs at port
//                   aTester_Rear.cpp:317 / :318 / :367, so this fragment must be
//                   stitched AFTER them (appending after existing content is
//                   fine).  This is the same reuse the Wave-2 content already
//                   does (see port line 1709 and its comment).
//  REAL SYMBOLS verified in the port 20260810: INDEX_SUCKER_TYPE (cmydef.h:2943,
//                   defined cmydef.cpp:3167); bArm2D44SuckCheck (cmydef.h:4824,
//                   defined cmydef.cpp:4880); DoRearTestDestroyICDelay -- a REAL
//                   TQPF_Timer at cmydef.h:4822 / cmydef.cpp:4878, already used by
//                   the translated DoRearTestDestroyIC (port lines 774/824/914);
//                   BTestSuck.iShtRow/iShtCol/Suck and TMySucker::GetStatus/Normal
//                   (aHotPlateSubstrate.h:406/438/369/131/130).
//  TRAP 5 (which header) -- BTestSuck is the aHotPlateSubstrate.h:365 TMyKitSuck
//                   (extern :637), object DEFINED at aHotPlateSubstrate.cpp:93;
//                   its Suck[][] elements are the aHotPlateSubstrate.h:106
//                   TMySucker.  The rival mykitsuck.h:274 / mykitsuck.h TMySucker
//                   pair is NOT used -- and that distinction is exactly what
//                   forces gate G-k9-BNEEDCHECK below to stay in place.
//  GATE REGISTER (three stub-routed statements -- no #if 0 was needed because
//  Wave 1 already owns compiling stand-ins, but each carries the same burden):
//   * G-k9-BNEEDCHECK -- golden lines :9767 (read) and :9774 (clear),
//     BTestSuck.bNeedCheck[i][j], routed to W64bT2_BNeedCheckGet/Set (port
//     aTester_Rear.cpp:317-318, offline default: no site ever needs a re-check).
//     WHY IT MUST STAY GATED (TRAP 3 answer, and it is NOT "the field is missing
//     from the tree"): bNeedCheck DOES exist in the tree -- mykitsuck.h:303
//     `bool bNeedCheck[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]` -- but on the
//     WRONG TMyKitSuck.  The class this TU uses (aHotPlateSubstrate.h:365, the
//     one BTestSuck is actually built from, 177 TUs) has no such member, so this
//     is TRAP 5, not an absence.  Un-gating requires adding the member to the
//     aHotPlateSubstrate.h mirror -- a shared header this task may not edit --
//     not swapping headers.
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: the latch is only ever SET by the same
//     gap stub (nothing in the port can make it true -- port lines 577/854 write
//     through the no-op setter), so a permanently-false read is internally
//     consistent with the rest of the translated tree; the re-check branch is
//     dead in both directions rather than half-wired.
//     REAL-MACHINE DIFFERENCE (stated honestly, per TRAP 2 -- "equivalent" is not
//     the justification): on a real negative-pressure head with
//     INDEX_SUCKER_TYPE!=1, golden returns a nozzle to Normal as soon as its
//     suck-back sensor releases; here that per-nozzle release never fires, so the
//     nozzle keeps its current solenoid state until some other path Normals it.
//     With INDEX_SUCKER_TYPE==1 there is no difference at all, because golden
//     :9779-9782 Normals every nozzle unconditionally right after.
//     ABSENCE COMMAND (re-run at integration, ran 2026-08-10 19:21:41 +0800):
//       grep -n "bNeedCheck" D:/HT9045/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.h
//     must stay EMPTY.  (Cross-check that the only header hit in the tree is still
//     the wrong class: grep -rn "bNeedCheck\[" --include=*.h D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//     -> mykitsuck.h:303 only.  Note atester.cpp:9757/9781 DO name
//     FTestSuck/BTestSuck.bNeedCheck, but both sit inside the
//     `#if 0 // GOLDEN VERBATIM` block atester.cpp:9730-9848, i.e. not compiled --
//     do not mistake them for proof the member exists.)
//   * G-k9-SUCKDESTROY2 -- golden line :9753,
//     fiosetview->ProcessIndexSuckDestroy2(1), routed to
//     W64bT2_ProcessIndexSuckDestroy2(1) (port aTester_Rear.cpp:367, offline
//     default true).
//     WHY IT MUST STAY GATED: TfiosetviewShim (atester_shims.h) exposes
//     bIndexSuck[][][] but has no ProcessIndexSuckDestroy2 method at all, and the
//     golden owner (iosetview.h:3019) belongs to the untranslated IO-set-view
//     form.  Identical treatment to the already-reviewed port call sites at lines
//     773/817/1709, so this fragment stays consistent with its own file.
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: `true` means "suck-back self-check
//     passed", which makes the INDEX_SUCKER_TYPE==1 path behave like the
//     INDEX_SUCKER_TYPE!=1 path that golden itself hard-codes to
//     bArm2D44SuckCheck=true two lines later (golden :9757) -- i.e. the offline
//     value is one golden branch verbatim, not an invented value.
//     REAL-MACHINE DIFFERENCE: a genuine suck-back (IC stuck to the nozzle)
//     detection result is replaced by "clean", so the nozzle-Normal sweep below
//     runs on the first pass instead of waiting for the detector.
//     ABSENCE COMMAND (ran 2026-08-10 19:21:41 +0800):
//       grep -n "ProcessIndexSuckDestroy" D:/HT9045/HT9011UC_Cpp_V3.33.906.0/atester_shims.h
//     must stay EMPTY.
//  INTEGRATE     -- retires the no-op stub atester_shims.cpp:213
//                   `void DoArm2D44VacCheck() {}`; keep the non-static
//                   declaration atester_shims.h:137 (body here is NOT static, so
//                   no TRAP 1 shape (d) shadow).
//---------------------------------------------------------------------------
void DoArm2D44VacCheck()                                                        //JerryYang 20190123 把index arm回吸檢查包成函式
{
    if(INDEX_SUCKER_TYPE==1)
    {
        bArm2D44SuckCheck=false;
        bArm2D44SuckCheck=W64bT2_ProcessIndexSuckDestroy2(1);                   // AI(W7b-k9) 20260810: golden :9753 fiosetview->ProcessIndexSuckDestroy2(1) -- reuses the Wave-1 TU-local gap stub at port aTester_Rear.cpp:367 (see GATE REGISTER)  //Sam 20241225 : 修正回吸偵測
    }
    else
    {
        bArm2D44SuckCheck=true;
    }

    if(DoRearTestDestroyICDelay.Off() &&
       bArm2D44SuckCheck==true)
    {
        for(int i=0; i<BTestSuck.iShtRow; i++)
        {
            for(int j=0; j<BTestSuck.iShtCol; j++)
            {
                if(W64bT2_BNeedCheckGet(i, j))                                  // AI(W7b-k9) 20260810: golden :9767 BTestSuck.bNeedCheck[i][j] -- gap stub, port aTester_Rear.cpp:317 (see GATE REGISTER)
                {
                    if(BTestSuck.Suck[i][j].GetStatus())
                    {
                    }
                    else
                    {
                        W64bT2_BNeedCheckSet(i, j, false);                      // AI(W7b-k9) 20260810: golden :9774 BTestSuck.bNeedCheck[i][j]=false -- gap stub, port aTester_Rear.cpp:318 (see GATE REGISTER)
                        BTestSuck.Suck[i][j].Normal();
                    }
                }

                if(INDEX_SUCKER_TYPE==1)                                        //jou 2011-11-01負壓不能一直開著真空，必須關掉
                {
                    BTestSuck.Suck[i][j].Normal();
                }
            }
        }
    }
}
