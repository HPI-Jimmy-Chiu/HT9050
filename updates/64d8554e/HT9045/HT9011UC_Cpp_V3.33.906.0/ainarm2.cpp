// =============================================================================
//  ainarm2.cpp  --  in-arm shared helpers (offline Sim-HAL translation)
//
//  Translation wave: W6.2b-canary (in-arm per-site-config variant dispatch)
//  Translator: AI(W6.2b-canary) 20260626
//
//  Mirror of the golden BCB6 ainarm2.cpp (D:/HT9045/
//  HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp).  W6.2b only needs the two
//  in-arm helpers that the freshly-translated site-variant SMs
//  (ainarm9045_1x1_1 / _2x4_16 / _2x8_32) call into but that had no body yet:
//
//    * SetInArmUseSuckToHasNullIC(int, int)  -- golden ainarm2.cpp:201
//        Re-classifies the in-arm pick grid's NULL_IC nozzles to HAS_NULL_IC
//        (the "I am committed to picking here" marker) honouring the picker
//        kind (AxEx / AxxG step) and the 2x8 close-site sub-modes.  Pure grid
//        logic over the Sim HAL InArmSuck object -- NO hardware touched.
//    * InitInArmPickFromHotPlateTask()       -- golden ainarm2.cpp:623
//        Resets the HP-pick task cursor to 1.  (Its 340/50 siblings already
//        live in aHotPlateSubstrate.cpp.)
//
//  Both are FAITHFUL line-for-line translations -- names, formulas, branches and
//  comments preserved verbatim.  The declarations these realize already exist in
//  aHotPlateSubstrate.h:610/611 (so no header edit is needed).
//
//  Also hosts the single active definition of bDestoryOnSht (golden moved it
//  ainarm2 -> cmydef by RogerYang 20251021; cmydef.h:5868 declares it extern,
//  but the cmydef.cpp definition sits inside a #if 0 // TODO(W6) block, so the
//  one live definition is provided here in this linked TU -- exactly one
//  definition, no ODR dupes).
//
//  Big5: Chinese comments preserved as UTF-8 (cp950).  NO U+FFFD.
// =============================================================================
#include "cmydef.h"                 // iInArmType / MInArmX / NULL_IC / HAS_NULL_IC consts
#include "MachineType.h"            // eTestMode (SingleSite) / e9045_* picker enums
#include "cprod.h"                  // TestIF (iTestMode)
#include "ainarm9045.h"             // bUseAxExPicker / bUseAxxGPicker / GetJStep
#include "aHotPlateSubstrate.h"     // InArmSuck grid + iInArmPickFromHotPlateTask / iCloseSiteModeFor2x8 cursors

// AI(pt-wave) 20260811 PT-W7e: includes the appended bodies need. Hoisted from part
// 03050, which injected them MID-FILE (a part file may not touch the file head).
#include "FormsFacade.h"
#include "canary_support.h"
#include "csystem.h"
#include "mysensor.h"
#include "myswitch.h"                   // SW[]                     -- myswitch.h:43
#include "acatchtray_shims.h"           // NewRecordProcess
#include "aoutarm9045.h"                // GetOutArmPitchX/Y_9045
#include "SECSGEM/SecsEventType.h"      // SECS_EVENT
#include "SECSGEM/SecsEventReport.h"    // EventReport
// AI(pt-wave) 20260811 PT-W7e-part2: hoisted from part files, with fNote.h corrected to forms/fNote.h
#include "FormsFacade.h"
#include "LastSet.h"
#include "RotateKit/aRotateKIT.h"
#include "SECSGEM/SecsEventReport.h"
#include "SECSGEM/SecsEventType.h"
#include "acatchtray_shims.h"
#include "ainarm9045_w7_shims.h"
#include "ainarm_SearchPlacePlate.h"
#include "aoutarm9045.h"
#include "atester.h"
#include "atester_shims.h"
// AI(pt-wave) 20260811: cMyDB.h deliberately NOT hoisted -- at file scope it
//   ambiguates NewRecordProcess/RecordProcess/MyDBIProcess against
//   canary_support.h and acatchtray_shims.h (duplicate declarations with
//   defaults). The part that needs it keeps it local, which is why it was local.
#include "canary_support.h"
#include "cinitial.h"
#include "common.h"
#include "csystem.h"
#include "csystem_shims.h"
#include "forms/fNote.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"

// ---------------------------------------------------------------------------
//  PT-W7e: the THIRTY-SEVEN file-scope globals golden declares BETWEEN functions.
//  Copied VERBATIM from golden by line number, not retyped from a type+name summary --
//  PT-W7d got three of seven wrong that way: two array types declared as scalars (one of
//  which compiled fine) and two initial values written 0 where golden has 1. SEVEN of the
//  declarations below carry array dimensions (golden :88, :102, :2952, :2956, :2959, :2960,
//  :3527), which is exactly what a type+name summary drops.
//  Two are typed HTimer in golden (:2951, :3792) and are declared TQPF_Timer here, because
//  the port's only HTimer is atester_shims.h:463 whose Off() is hard-coded true and would
//  silently zero every dwell; each carries that reason inline.
//  Added by the integrator, never by an agent: two agents emitting one global is a
//  multiple-definition link error.
//  AI(pt-wave) 20260811
// ---------------------------------------------------------------------------
bool bDoPreciser=false;   // golden ainarm2.cpp:50 -- AI(pt-wave) 20260811: missed by wave_targets (its name appears in a port comment, so port_has() saw it)
const int cMaxInArmOrder=2;   // golden :66
int cMaxKit=1;   // golden :67
int iNextPlateRow;   // golden :81
TQPF_Timer VibrationShuttleDelay;                                               //JerryYang 20171205 (Steven) Shuttle 震動馬達 delay   // golden :87
TQPF_Timer VibrationOutShuttleDelay[2];                                         //JerryYang 20171205 (Steven) Shuttle 震動馬達 delay   // golden :88
int iAutoCleanCnt=0;                                                            //JerryYang 20171127 (Steven) add auto clean清潔次數   // golden :93
bool bInArmZNeedCheck[4]={false, false, false, false};   // golden :102
bool bRecordVacuumTim = false;                                                  //kevin 20210224 記錄計算真空產生器壽命   // golden :103
bool bAutoCleanUse_13_24=false;                                                 //ChungHung 20131118 add for SCK AutoClean use HotPlate   // golden :109
int iInArmZCheckPosTask=1;                                                      //Richard 2017.11.1 After pause check arm Z in position   // golden :110
int iInArmDevicePosPrecise=1;                                                   //Frank 20180410 (Steven) : InArm Preciser Station   // golden :111
TQPF_Timer ShakeShuttleDelay;   // golden :2313
TQPF_Timer ShakeDelay;   // golden :2314
int iKnockShuttleTask;   // golden :2449
int iKnockShtFirstTask;   // golden :2535
int iProcessTrayMapDataErrorTask;   // golden :2839
int iOneKitPreciser=0;   // golden :2944
bool bOneKitPreciser=false;   // golden :2945
TQPF_Timer DoInArmDevicePosPreciseDelay;   // golden :2951 -- golden type HTimer; TQPF_Timer because atester_shims.h:463 HTimer::Off() is always true
int  iPrecise_XP[X_PITCH_COUNT];   // golden :2952
int  iPrecise_YP=0;   // golden :2953
int  iPrecise_MoveXP=12000;   // golden :2954
int  iPrecise_MoveYP=6000;   // golden :2955
int  iPrecise_Z[MAX_ARM_Row][MAX_ARM_Col];   // golden :2956
bool bPrecise_XYZ=false;   // golden :2957
bool bPreciseFlag=false;   // golden :2958
bool bPrecise_Z[MAX_ARM_Row][MAX_ARM_Col];   // golden :2959
bool bPrecise_SuckFlag[MAX_ARM_Row][MAX_ARM_Col];   // golden :2960
int iInitialFlag    =0;   // golden :2962
int iMoveDownPlace  =1;   // golden :2963
int iMoveDownAdd200 =2;   // golden :2964
int iMoveDownPick   =3;   // golden :2965
int iVibrateShuttleTask;   // golden :3459
int iVibrateOutShuttleTask[2];   // golden :3527
int  InDieCleanTask=1;   // golden :3791
TQPF_Timer DoInDieCleanDelay;   // golden :3792 -- golden type HTimer; TQPF_Timer because atester_shims.h:463 HTimer::Off() is always true
int  iOCRWaitTask=1;   // golden :3948

// =============================================================================
//  bDestoryOnSht  (golden cmydef.cpp:5955 -- RogerYang 20251021: 從inarm2移過來)
//  在Shuttle吹氣與資料交換的Flag (Steven 20170905 wei).  cmydef.cpp's definition
//  is gated #if 0 // TODO(W6); this is the single ACTIVE definition.
// =============================================================================
#ifndef HT9045_bDestoryOnSht_DEFINED
#define HT9045_bDestoryOnSht_DEFINED
bool bDestoryOnSht=false;                                                       //RogerYang 20251021 : 從inarm2移過來 //Steven 20170905 (wei) : 在Shuttle吹氣與資料交換的Flag
#endif

// =============================================================================
void SetInArmUseSuckToHasNullIC(int iSht, int iKit)
{
//    int iKit32=(iKit==1)?4:0;

    if(iInArmType==e9045_1x4_8_Hot ||                                           //Steven 20240119 : fixed for 2x4_16
       iInArmType==e9045_1x2_4_Hot ||
       iInArmType==e9045_2x2_8_Hot)
    {
//        iSht=0;
//        iKit32=0;
    }

    if(TestIF.iTestMode==SingleSite ||                                          //Steven 20140614 : for Auto Clean Single Site
       iInArmType==e9045_1x4_1_Ac)                                              //Steven 20200720 : 1x4只開site Ac
    {
    }
    else if(bUseAxExPicker() ||                                                 //Steven 20241102 : Fixed for HP
            bUseAxxGPicker())
    {
        for(int i=0; i<InArmSuck.iPickRow; i++)
        {
            for(int j=0; j<InArmSuck.iPickCol; j++)
            {
                int j2=j*GetJStep();
                if(j2<InArmSuck.iMaxCol &&
                   InArmSuck.Item[i][j2]==NULL_IC)
                {
                    InArmSuck.SetItemData(i, j2, HAS_NULL_IC);
                }
            }
        }
    }
    else
    {
        for(int i=0; i<InArmSuck.iPickRow; i++)
        {
            for(int j=0; j<InArmSuck.iPickCol; j++)
            {
                if(InArmSuck.Item[i][j]==NULL_IC)
                    InArmSuck.SetItemData(i, j, HAS_NULL_IC);
            }
        }
    }

    if(iCloseSiteModeFor2x8==e2x8Run2x2_13)
    {
        InArmSuck.SetItemData(0, 1, NULL_IC);
        InArmSuck.SetItemData(0, 3, NULL_IC);
        InArmSuck.SetItemData(1, 1, NULL_IC);
        InArmSuck.SetItemData(1, 3, NULL_IC);
    }
    else if(iCloseSiteModeFor2x8==e2x8Run2x2_14)
    {
        InArmSuck.SetItemData(0, 1, NULL_IC);
        InArmSuck.SetItemData(0, 2, NULL_IC);
        InArmSuck.SetItemData(1, 1, NULL_IC);
        InArmSuck.SetItemData(1, 2, NULL_IC);
    }
}
//==============================================================================
void InitInArmPickFromHotPlateTask()
{
    iInArmPickFromHotPlateTask=1;
}
//==============================================================================
// =============================================================================
//  Check_QA_ModeCount()   golden ainarm2.cpp:160-199   (Steven 20111019 : QA Mode)
//  Translation wave: PT-W7e (ainarm2.cpp leaf batch)  Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- QA-Mode lot-length governor, polled from the in-arm loader-pick path.
//    Counts loader cycles (iQAModeLoaderCT) against Prod.iQAModeCount and drives the
//    three-stage QA-Mode exit: (1) at/over the count -> force a quick Clean Out via
//    InitOneCycle + iOneCycle=1 and return true; (2) past the count -> flip the tester
//    OFF_LINE, restore the backed-up in-arm speed mode and fire the real Clean Out;
//    (3) approaching the count -> switch the in-arm to FIX speed mode so the last
//    devices are produced deterministically.
//
//  WAVE SCOPE -- Check_QA_ModeCount (golden :160-199) ACTIVE, no gates.
//  GATE REGISTER -- (empty: this function needed no #if 0.)
//
//  NET-NEW SYMBOL.  Unlike 9 of this wave's 11 leaves, Check_QA_ModeCount has NO
//  stub and NO declaration anywhere in the port tree (measured 2026-08-11 15:05:
//  python walk of 34,016 .cpp/.h files under D:/HT9045/HT9011UC_Cpp_V3.33.906.0,
//  _w7e_parts excluded -- 0 hits for 'Check_QA_ModeCount').  Its golden declaration
//  is ainarm2.h:81 and this tree has no ainarm2.h, so INTEGRATOR: the golden callers
//  (ainarm9045*.cpp loader-pick arms) still need a declaration -- until then this is
//  TRAP-1 shape (a), nobody references it.  Deliberately NOT `static` (shape (d)).
//
//  ------------------------------------------------------------------------------
//  STITCH HAND-OFF: INCLUDES THE STITCHED ainarm2.cpp NEEDS FOR THIS WAVE'S PARTS.
//  The port ainarm2.cpp today includes only cmydef.h / MachineType.h / cprod.h /
//  ainarm9045.h / aHotPlateSubstrate.h.  Verified by g++ -fsyntax-only on a scratch
//  TU carrying all 11 bodies of this wave (2026-08-11) -- the additions are exactly
//  the header set the sibling engines asortarm.cpp:505-527 / aoutarm9045.cpp:53-79
//  already use:
//     #include "MachineDefine.h"        // <windows.h> -> ZeroMemory (CheckInArmSuckInitial)
//     #include "csystem.h"              // CheckSuckInitialStatus:390 / IndexHasIC / ShuttleHasIC /
//                                        // InitOneCycle:88 / InitCleanOutFunction:74
//     #include "Motor/mymotor.h"        // MOT[] / InArmZSafe:440 / DETECT_ALL_FLAG / DETECT_SENSOR_FLAG
//     #include "mycylin.h"              // Cylinder[] (DoKnockShuttle)
//     #include "cpublic.h"              // CosFunction
//     #include "ainarm9045_w7_shims.h"  // IndexZCanMove[2]:58 / bShuttleKnock:62 (+ the
//                                        // DoKnockShuttle decl:48 that carries the default arg)
//     #include "csystem_shims.h"        // bShuttleShake:73 (def csystem_shims.cpp:38)
//     #include "atester_shims.h"        // IsNNMode:277 / NN_2Row
//     #include "FormsFacade.h"          // fMain (forms/fMain.h:217 ModifyTester, :168 CleanOut)
//     #include "acatchtray_shims.h"     // NewRecordProcess:388
//     #include "canary_support.h"       // ShowErrorMessage:66 / ShowMyMessage:80 / RecordProcess /
//                                        // LastSet / IniConfig / K_RETRY|K_SKIP / __FUNC__:45
//  (TrayForm comes from cprod.h:1359 and ArmSpeed[] from cprod.h:2875 -- already included.)
//  ------------------------------------------------------------------------------
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
bool Check_QA_ModeCount()                                                       //Steven 20111019 : QA Mode //Dell 20111024
{
    if(IniConfig.bQAMode==true && LastSet.iRunStartMode==rsmQAMode)             //Steven 20111005 : QA Mode
    {
        if(iQAModeLoaderCT>=Prod.iQAModeCount && bQAModeQuickCleanOut==false && bQAModeFinishCleanOut==false)  //數量到的話要先快速CleanOut
        {                                                                       //jou 2012-05-03 增加QA mode使用獨立的Loader Count
            if(bQAModeQuickCleanOut==false && bQAModeFinishCleanOut==false)
            {
                if(CUSTOMER_CODE!=CC_KYEC_LEE)  //Eastsun 20260526 #026-1.1 Ifor 20201102 add:KYEC QA Mode 不顯示 Tray Feed
                    TrayForm.bAutoFeed=false;
                bQAModeQuickCleanOut=true;
                InitOneCycle("QA Mode");
                iOneCycle=1;
                return true;
            }
        }
        else if(iQAModeLoaderCT>Prod.iQAModeCount)                              //數量比設定值多，表示已經做完，所以就直接送Bin 1
        {
            if(bQAModeQuickCleanOut==true && bQAModeFinishCleanOut==false)
            {
                fMain->ModifyTester(OFF_LINE);                                  //Steven 20191218 : 整合修改LastSet.iTester
                NewRecordProcess("MES2155", "Change to Off_Line", "by Check_QA_ModeCount QAMode1");                     //ChungHung 20140722 add add record

                ArmSpeed[InArm].bVariModeFIX=IniConfig.bBackUpInArmMode;
                bQAModeFinishCleanOut=true;
                bQAModeQuickCleanOut=false;

                if(iCleanOut==0 &&
                   CUSTOMER_CODE!=CC_KYEC_LEE)                                  //Eastsun 20260526 #026-1.2 Ifor 20201102 add:KYEC QA Mode 不自動Clean Out
                    InitCleanOutFunction();
            }
        }
        else if(iQAModeLoaderCT>Prod.iQAModeCount-((InArmSuck.iMaxRow*InArmSuck.iMaxCol*2>20)?(InArmSuck.iMaxRow*InArmSuck.iMaxCol*2):20) &&
                iQAModeLoaderCT<Prod.iQAModeCount)                              //數量接近時，要改用Fix方式生產
        {
            ArmSpeed[InArm].bVariModeFIX=true;
        }
    }
    return false;
}
//==============================================================================

//==============================================================================
//== ROLE: AdjustShuttlePlaceOrderForASM -- choose which shuttle Auto Site Mapping
//==       places the next HotPlate device into.  In fixed single-shuttle mode
//==       (iShuttleMode==1) it mirrors the operator shuttle selection; otherwise
//==       it either resets the cursor to shuttle 0 or advances it round-robin over
//==       shuttle 0 and shuttle 1.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   AdjustShuttlePlaceOrderForASM  golden ainarm2.cpp:748-770  -- ACTIVE
//==     (golden lines emitted VERBATIM; ZERO gates)
//==
//== GATE REGISTER: (none)
//==   * TestIF_File.iShuttleMode (cprod.h:1654) and .iShuttle_Sel (cprod.h:1655);
//==     the object is extern cprod.h:2577 -- resolved by the include block already
//==     at the top of port ainarm2.cpp.
//==   * iAutoSiteMapHPToSht -- declared cmydef.h:4386, really DEFINED at
//==     cmydef.cpp:4537 (int, initial 0).  A real definition, not a stub.
//==
//== INTEGRATION NOTE -- TRAP 1 shape (a), NO LIVE CALLER YET:
//==   grep -rn --include=*.cpp "AdjustShuttlePlaceOrderForASM" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==   -- ran 2026-08-11 06:24 +0800: exactly three hits, and ALL THREE sit inside
//==   block comments that golden itself opened (ainarm_SearchPlacePlate.cpp:4839
//==   inside the comment opened at its :4835; csystem.cpp:7226 and :7262 inside
//==   the comment opened at its :7219).  So after this part lands the archive
//==   member is still not extracted by anyone -- a green build proves nothing.
//==   There is also NO declaration anywhere in the port.  Golden ainarm2.h:129 is
//==     extern void AdjustShuttlePlaceOrderForASM(bool bResetSht=false);
//==   and the integrator must add it.  The default argument belongs on the
//==   DECLARATION only: golden's definition at :748 carries none and this part
//==   matches that, so a later header add cannot become a re-default error.
//==   Not static -- once that declaration lands it is non-static (shape (d)
//==   avoided).
//==
//== Big5 -> UTF-8: the trailing Chinese comment on the signature line is
//== transcribed character for character from cp950.  Zero U+FFFD.
//==============================================================================
void AdjustShuttlePlaceOrderForASM(bool bResetSht)                              //Steven 20250206 : fixed ASM之中作one cycle會造成關arm 1時異常
{
    if(TestIF_File.iShuttleMode==1)
    {
        if(TestIF_File.iShuttle_Sel==0)
            iAutoSiteMapHPToSht=0;
        else if(TestIF_File.iShuttle_Sel==1)
            iAutoSiteMapHPToSht=1;
    }
    else
    {
        if(bResetSht)
        {
            iAutoSiteMapHPToSht=0;
        }
        else
        {
            iAutoSiteMapHPToSht++;
            if(iAutoSiteMapHPToSht>=2)
                iAutoSiteMapHPToSht=0;
        }
    }
}

//==============================================================================
//  ResetInArmParam()  --  golden ainarm2.cpp:923-926
//
//  ROLE          One-line wrapper: reset the whole in-arm parameter set by
//                re-running InitInArmTask().
//  WAVE SCOPE    ResetInArmParam()          ACTIVE   golden :923-926
//  GATE REGISTER (none -- InitInArmTask() has a linkable body)
//
//  HAND-OFF, NOT A GATE (TRAP 1 shape (b), stated so it is not mistaken for
//  "working"): InitInArmTask() resolves to aHotPlateSubstrate.cpp:1062
//  `void InitInArmTask() {}` -- a no-op STUB (decl aHotPlateSubstrate.h:896, and
//  a second decl at acarry_shims.h:217).  So this call is translated FAITHFULLY
//  and links, but offline it resets nothing.  The degradation is entirely in the
//  callee and is NOT this wave's to fix; do not read a green build as evidence
//  that in-arm params are being reset.  1 live call site today.
//==============================================================================
void ResetInArmParam()
{
    InitInArmTask();
}

//==============================================================================
//  DoInArm_SuckerMapForCloseArm()                         golden ainarm2.cpp:928-963
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : when only one of the two in-arms is selected (iShuttleMode==1),
//               blanks the OTHER arm`s whole 2x8 in-use nozzle map, then snapshots
//               both arms` maps into fNeedToCheckASM (the "these nozzles still owe an
//               Auto-Site-Map result" grid) and clears bInitialAutoSiteMap.
//  WAVE SCOPE : DoInArm_SuckerMapForCloseArm()  golden :928-963  -- ACTIVE
//  GATE REGISTER: none.  Prod.fInArmSuck4x8 (cprod.h:820), Prod.fNeedToCheckASM
//               (cprod.h:818), Prod.bInitialAutoSiteMap (cprod.h:816) and
//               TestIF_File (cprod.h) are all reachable from this TU`s existing
//               include head.
//  TRAP 1     : shape (a) applies TODAY -- the port has NO declaration for this   //AI(W906-AMB-L2) 20260929: no longer -- golden caller ainarm9045.cpp:4814 is now translated in port DoInArm_9045_SuckerMap (ainarm9045.cpp, local extern of golden ainarm2.h:149)
//               function in any header (golden home ainarm2.h:149) and its only
//               golden caller is golden ainarm9045.cpp:4814, which the port has not
//               reached.  So this body links in but has no caller until the
//               integrator adds the declaration.  It is deliberately NON-static:
//               making it static to silence a "defined but not used" style warning
//               would create shape (d) the moment that declaration lands.  There is
//               no pre-existing stub to retire (checked: 0 definitions tree-wide).
//==============================================================================
void DoInArm_SuckerMapForCloseArm()
{
    if(TestIF_File.iShuttleMode==1)
    {
        if(TestIF_File.iShuttle_Sel==0)                                         //只用Arm 1
        {
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<8; j++)
                {
                    Prod.fInArmSuck4x8[1][i][j]=false;
                }
            }
        }
        else                                                                    //只用Arm 2
        {
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<8; j++)
                {
                    Prod.fInArmSuck4x8[0][i][j]=false;
                }
            }
        }
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Prod.fNeedToCheckASM[0][i][j]=Prod.fInArmSuck4x8[0][i][j];
            Prod.fNeedToCheckASM[1][i][j]=Prod.fInArmSuck4x8[1][i][j];
        }
    }
    Prod.bInitialAutoSiteMap=false;
}

//==============================================================================
//  ChangeHotPlateData(bool bSwapSht)  --  golden ainarm2.cpp:976-991
//  Wave PT-W7e, part k1-ainarm2.  Translator: AI(W7E-k1-ainarm2) 20260811
//
//  ROLE  After a one-cycle / order change, re-point the in-arm at the Shuttle+Kit
//        of the LAST recorded hot-plate suck team that still holds data (so UPH
//        does not collapse by re-planning from scratch); if there is no such team,
//        fall back to re-ordering the plain shuttle place order.
//
//  WAVE SCOPE  ChangeHotPlateData  PARTLY GATED  golden :976-991
//              (:978-987 gated, :988-991 ACTIVE -- see GATE k1-G1).
//
//  GOLDEN BUG KEPT: the `bSwapSht' parameter is never read by golden's body.
//  Kept verbatim (name included) -- callers pass it and golden ignores it.
//
//  GATE REGISTER
//   GATE k1-G1 -- golden :978-987: `uHPSuckTeam *HTTeam;' +
//     `HTTeam=PickFromHPList->ExtractLastTeamHasData();' + the whole
//     `if(HTTeam!=NULL){...}' arm (InArmSuck.iWhichSht/iWhichKit <- HTTeam->iSht/
//     iKit + AdjustShuttleWhichKitOrder()).  The `else' keyword is gated with it;
//     its BLOCK stays active, so control still runs AdjustShuttlePlaceOrder().
//
//     WHY GATED -- THIS IS A TRAP 3 / TRAP 5 CASE, NOT A PLAIN ABSENCE.
//     A real port body for ExtractLastTeamHasData DOES exist:
//     Public/HTEditList.cpp:2234 (declared Public/HTEditList.h:365), and that
//     header also carries a COMPLETE uHPSuckTeam with the very iSht/iKit fields
//     this code reads (Public/HTEditList.h:258-282).  Taking it is still WRONG:
//       * There are TWO different `class uPlateInfo' in the port -- the minimal
//         mirror at aHotPlateSubstrate.h:672-716 (NO data members at all, so
//         sizeof==1, and no ExtractLastTeamHasData) and the full golden-shaped one
//         at Public/HTEditList.h.  They can never coexist in one TU (redefinition),
//         so including HTEditList.h here would SWAP the class identity of
//         PickFromHPList for this TU only.
//       * The live object is allocated by the MINIMAL class:
//         aHotPlateSubstrate.cpp:855-862 `if(PickFromHPList==NULL)
//         PickFromHPList = new uPlateInfo();' -- a data-member-less object.  The
//         single definition of the pointer is Public/HTEditList.cpp:204.
//         Calling the FULL class's ExtractLastTeamHasData on that object reads its
//         TList members past the end of a 1-byte allocation.  It would compile,
//         it would link, and it would corrupt memory -- exactly TRAP 5.
//       * uHPSuckTeam as seen from THIS TU (aHotPlateSubstrate.h:652) is an
//         incomplete type, so `HTTeam->iSht' cannot compile at all without that
//         swap.
//     So the honest answer to "why SHOULD this be gated" is not "the symbol is
//     missing" -- it is "the symbol belongs to a second, incompatible definition of
//     uPlateInfo, and this TU's PickFromHPList does not point at that layout".
//     Retire this gate only when the two uPlateInfo classes are unified (the
//     cross-cutting job HTEditList.h:40-63 already describes and defers).
//
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: with the minimal uPlateInfo the team
//     list can never be non-empty -- every mutator (AddHPSuckGroup /
//     UpdateHPSuckGroup / ClearGroupList / ExtractFirstTeam) is an unconditional
//     no-op and the class holds no list (documented at aHotPlateSubstrate.h:653-671).
//     So ExtractLastTeamHasData() would return NULL, golden would take its OWN
//     `else' arm, and AdjustShuttlePlaceOrder() is exactly what runs here.  The
//     gated form reproduces golden's offline behaviour statement for statement.
//     REAL-MACHINE DIFFERENCE: on hardware with the hot-plate list populated,
//     golden resumes at the last team's Shuttle/Kit and calls
//     AdjustShuttleWhichKitOrder(); gated, the arm instead re-orders the plain
//     shuttle place order, i.e. it may target a different Shuttle/Kit than the
//     half-finished team -- a UPH / place-order difference, not a motion-safety one
//     (both arms only rewrite selectors, they move nothing).
//     ABSENCE COMMAND (exact -- proves the ABSENCE IS ONLY FROM THIS TU'S HEADER):
//       rg -n -w ExtractLastTeamHasData --glob '*.cpp' --glob '*.h' D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//         -> 3 hits, all in Public/HTEditList.{h:365,cpp:75,cpp:2234}; ZERO in
//            aHotPlateSubstrate.h (the uPlateInfo this TU sees).
//       rg -n "class uPlateInfo" --glob '*.h' D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//         -> TWO classes (aHotPlateSubstrate.h + Public/HTEditList.h).
//     MEASURED AT: 2026-08-11T06:36:51+0800 (first) / 2026-08-11T06:44 (second).
//     TRAP 2 RE-RUN, 2026-08-11T06:48:10+0800 -- the ABSENCE premise is unchanged (no
//     sibling part adds ExtractLastTeamHasData to aHotPlateSubstrate.h's uPlateInfo,
//     and unifying the two classes is explicitly out of this wave), BUT TWO OTHER
//     THINGS IN THIS FUNCTION MOVED and they must be stated:
//       * _w7e_parts/00849_AdjustShuttleWhichKitOrder.txt landed, so the call INSIDE
//         this gate (:985) now has a REAL body waiting -- the gate is withholding a
//         real call, not a no-op.  It stays gated only because reaching it requires
//         the wrong uPlateInfo; the moment the classes are unified, retiring this gate
//         becomes a behaviour change, not a cosmetic one.
//       * _w7e_parts/00707_AdjustShuttlePlaceOrder.txt landed, so the ACTIVE else-arm
//         (:989) also stops being a no-op (it is aHotPlateSubstrate.cpp:1066
//         `void AdjustShuttlePlaceOrder(int) {}' today).  Its declaration
//         aHotPlateSubstrate.h:905 carries golden's default `int iReset=-1', which is
//         what makes golden's zero-argument call at :989 compile -- the sibling's
//         DEFINITION carries no default, correctly.
//     Both make the offline default MORE faithful, not less; neither changes the gate.
//
//  TRAP 5 STATEMENT: this part relies on aHotPlateSubstrate.h for uPlateInfo /
//  uHPSuckTeam / InArmSuck (TMyKitSuck) -- the mirror 177 TUs already use.  It does
//  NOT include Public/HTEditList.h or mykitsuck.h.  The object PickFromHPList
//  points at is created in aHotPlateSubstrate.cpp:857; the pointer itself is
//  defined in Public/HTEditList.cpp:204.
//
//  Big5: the TWO Chinese trailing comments in this range -- golden :976 and :985
//  (measured: exactly the non-ASCII lines of :976-991) -- are transcribed character
//  for character from cp950.  ZERO U+FFFD.
//==============================================================================
void ChangeHotPlateData(bool bSwapSht)                                          //Steven 20100816 : 修正OneCycle時，會導致UPH降低的問題
{
#if 0 // GATE k1-G1 -- golden :978-987 (uHPSuckTeam/ExtractLastTeamHasData belong to the OTHER uPlateInfo; see GATE REGISTER above).  The `else` keyword at :987 is gated with the arm; its BLOCK (:988-990) stays ACTIVE so AdjustShuttlePlaceOrder() still runs -- which is exactly what golden does when the team list is empty.
    uHPSuckTeam *HTTeam;

    HTTeam=PickFromHPList->ExtractLastTeamHasData();
    if(HTTeam!=NULL)
    {
        InArmSuck.iWhichSht=HTTeam->iSht;
        InArmSuck.iWhichKit=HTTeam->iKit;
        AdjustShuttleWhichKitOrder();                                           //Steven 20160201 : 修正Shuttle放料順序
    }
    else
#endif // GATE k1-G1
    {
        AdjustShuttlePlaceOrder();
    }
}

// =============================================================================
//  ChangeHotPlateDataToOneShuttle()   golden ainarm2.cpp:993-1014
//  (JerryYang 20171218 (Steven) -- hot-mode dual-arm -> single-arm hang-up fix)
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- hot-mode dual-arm -> single-arm recovery: when the machine is idle enough
//    (index OR shuttle OR in-arm carries no IC) and single-shuttle mode is selected,
//    rewrite the WHOLE iHotWhichShuttle[2][50][50] hot-plate reservation grid to the one
//    selected shuttle, so devices already reserved for the now-closed shuttle cannot
//    hang the flow.
//
//  WAVE SCOPE -- ChangeHotPlateDataToOneShuttle (golden :993-1014) ACTIVE, no gates.
//  GATE REGISTER -- (empty.)
//
//  NET-NEW SYMBOL (this wave's second, with Check_QA_ModeCount): 0 hits tree-wide in
//  the 34,016-file walk measured 2026-08-11 15:05.  Golden's declaration is ainarm2.h;
//  this tree has no ainarm2.h, so no caller can reach it yet (TRAP-1 shape (a)).  NOT
//  `static` (shape (d)).  Its golden caller is the ainarm2.cpp:640-671 hot-mode
//  dispatcher, which is not in this wave.
//  BOUNDS NOTE -- golden walks j,k<50 over iHotWhichShuttle[2][50][50] (golden :73):
//  the literal 50s are golden's own array extents, NOT the runtime plate size.  Kept.
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
void ChangeHotPlateDataToOneShuttle()                                           //JerryYang 20171218 (Steven) fix 加熱模式雙arm切單arm hang up問題
{
    if(LastSet.iTemperature==Tempture_Hot)
    {
        if(IndexHasIC()==false || ShuttleHasIC()==false || InArmSuck.HasIC()==false)
        {
            if(TestIF.iShuttleMode==1)
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<50; j++)
                    {
                        for(int k=0; k<50; k++)
                        {
                            iHotWhichShuttle[i][j][k]=TestIF_File.iShuttle_Sel;
                        }
                    }
                }
            }
        }
    }
}
//==============================================================================

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:1140-1184 -----------------------------
//  ROLE          -- MoveInArmZToPlateSafe() plus Ifor's 20221227 guard: after the
//                   in-arm has picked off the Loader tray and its Z is back at the
//                   plate-safe height, verify the machine did not lift the LOADER
//                   TRAY ITSELF along with the ICs.  Suspicion = the sure-tray or
//                   pre-detect sensor went OFF, or the Y fixer cylinder is pushed
//                   out while its push sensor reads ON.  Three consecutive
//                   suspicious passes raise JAM0929 (RETRY|SKIP); SKIP clears the
//                   tray data + fHasTray and lets the flow continue, RETRY keeps
//                   the caller blocked.
//  WAVE SCOPE    -- MoveInArmZToPlateSafeAndCheckLoaderTray(int) ACTIVE golden :1140-1184
//  GATE REGISTER -- (empty: this range contains no #if 0 -- every symbol it needs
//                   has a real port home; see the include block below)
//
//  FIDELITY NOTES (golden quirks kept, NOT fixed)
//   * `int ret=0;` is dead on every path that does not reach the JAM -- kept.
//   * `bfirstIn=true;` is written twice on the SKIP path (:1162 before the alarm
//     and again :1168 inside the SKIP arm) -- redundant, kept verbatim.
//   * Because :1162 sets bfirstIn=true BEFORE the alarm, a RETRY answer restarts
//     the 3-strike counter from zero rather than resuming it.  That is golden's
//     behaviour and callers depend on the resulting re-count; NOT changed.
//   * `LastSet.iRealDummy>=HAS_TRAY` is an ordered compare on the dummy-mode enum,
//     not an equality -- reproduced exactly.
//   * The 2 function-local `static`s are golden's own and are PODs with constant
//     initialisers -- no ctor runs, so TRAP 4 does not apply.  They are function-
//     local, NOT the file-scope `static` function definition TRAP 1 (d) forbids.
//
//  TRAP 1 (a) HAND-OFF -- NOTHING IN THE PORT CALLS THIS YET.  Golden's 7+ call
//  sites are all in ainarm9045.cpp (:7667 :8008 :8028 :8156 :8167 :8173 :8254 ...)
//  and none of them survived into the port: `grep -rIn "Ifor 20221227"` over the
//  port tree returns ZERO hits (run 2026-08-11 06:31 +0800), while the port's
//  ainarm9045.cpp instead has bare `MoveInArmZToPlateSafe(Task)` at :1226 :1309
//  :1376 :1473.  So this body will link only once a caller is (re)wired, and until
//  then the loader-tray-lifted JAM0929 protection is ABSENT AT RUNTIME even though
//  the function exists -- exactly the "build green proves nothing" shape.  Also,
//  golden declares it at ainarm2.h:140; the port has no ainarm2.h and
//  aHotPlateSubstrate.h (the mirror) does not declare it, so the integrator must
//  add `extern bool MoveInArmZToPlateSafeAndCheckLoaderTray(int iTask);` there.
//
//  DEPENDENCY: the wrapped MoveInArmZToPlateSafe(int) is declared
//  aHotPlateSubstrate.h:901 and its CURRENT port body :1065 is the offline stub
//  `return true;` (Z reaches safe immediately).  Sibling part
//  _w7e_parts/01025_MoveInArmZToPlateSafe.txt lands the real golden body (:1025) in
//  this same stitch -- re-checked 2026-08-11 06:46:04 +0800 (`ls _w7e_parts | grep
//  MoveInArmZToPlateSafe` -> 01025, 01140 (mine)).  Until that lands, this guard
//  runs on EVERY tick instead of only after the Z is genuinely back at safe height,
//  which makes the 3-strike counter reach 3 far sooner.  Both parts must land
//  together for the timing to be golden-faithful.
// ---------------------------------------------------------------------------
//AI(k3-ainarm2) 20260811: HOIST ME to ainarm2.cpp's include head -- mid-file
//  include block, the convention already used at ainarm9045.cpp:5389/:10133/:10607.
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "mysensor.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "mycylin.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// ---------------------------------------------------------------------------
bool MoveInArmZToPlateSafeAndCheckLoaderTray(int iTask)                         //Ifor 20221227 add:新增Loader 取料後判斷是否吸到Loader Tray
{
    static bool bfirstIn=true;
    static int iCheckCount=0;
    int ret=0;
    if(MoveInArmZToPlateSafe(iTask))
    {
        if(bfirstIn==true)
        {
            bfirstIn=false;
            iCheckCount=0;
        }

        if(LastSet.iRealDummy>=HAS_TRAY && MOT[MMTrayY].fHasTray==true)
        {
            if(Sen[SnLoaderSureTray].IsOff()==true || Sen[SnLoaderPreDete].IsOff()==true ||
              (Cylinder[C_TrayY_Fixer].GetOutBit()==true && Sen[SnLoaderFixCyPush].IsOn()==true))
            {
                iCheckCount++;

                if(iCheckCount>=3)
                {
                    bfirstIn=true;
                    ret=ShowErrorMessage("JAM0929", K_RETRY|K_SKIP, MMTrayY, false, "CheckLoaderTray");                 //Steven 20120925 : JAM0909 -> JAM0929
                    if(ret==K_SKIP)
                    {
                        MOT[MMTrayY].ClearTray(__FUNC__);
                        MOT[MMTrayY].fHasTray=false;
                        bfirstIn=true;
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                return false;
            }
        }

        bfirstIn=true;
        return true;
    }
    return false;
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 1188..1204
// -----------------------------------------------------------------------------
//  ROLE        : true -- and forces a re-home -- if ANY in-arm Z motor is off its taught safe position; golden's own section header above it reads 'Z axis safe-position check, kevin 20120217'.
//  WAVE SCOPE  : CheckInArmZ -- ACTIVE, golden ainarm2.cpp:1188..1204.
//  INCLUDES     : this body needs, beyond what port ainarm2.cpp already includes
//              : (cmydef.h / MachineType.h / cprod.h / ainarm9045.h /
//              : aHotPlateSubstrate.h):
//              : Motor/mymotor.h (MOT[] and ReadPos), aHotPlateSubstrate.h (SetInArmHome :906) -- already included
//  GATE REGISTER: none.  The whole selector line is already proven compilable in this
//              : tree: acatchtray.cpp:407 (inside InArmZInSafe) carries the
//              : byte-identical `(USE_PICKER_COUNT==ep16Picker &&
//              : InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:
//              : InArmSuck.Suck[i][j].iMotNo` expression.  Prod.ZInArmSafe cprod.h:403
//              : inside PROD_INFO_ST (which closes at cprod.h:1137; `extern
//              : PROD_INFO_ST Prod;` cprod.h:1138).
//  TRAP 5       : InArmSuck is the aHotPlateSubstrate.h:365 TMyKitSuck (declared
//              : aHotPlateSubstrate.h:624, object DEFINED aHotPlateSubstrate.cpp:80),
//              : whose TMySucker carries iMotNo at aHotPlateSubstrate.h:132.  NOT the
//              : mykitsuck.h:274 class -- that one has a different layout, and picking
//              : it would link cleanly and then read iMotNo at the wrong offset.
//  EXPIRED GATE : landing this body makes RotateKit/aRotateKIT_In.cpp's GATE (9) stale.
//              : That file routes golden :53 / :878 through
//              : `#define W906RKIN_CHECKINARMZ() (CheckInArmZ())` at
//              : aRotateKIT_In.cpp:377 with prose at :142, :375, :438 and :1276 claiming
//              : CheckInArmZ has no compiled body.  That premise dies here.  Re-ask
//              : TRAP 3 before retiring it: this function has a real side effect
//              : (SetInArmHome), so ungating CHANGES machine behaviour and belongs in
//              : its own measured commit, not in this stitch.
// =============================================================================
bool CheckInArmZ()
{
    int iMot;
    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iMot=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
            if(MOT[iMot].ReadPos()!=Prod.ZInArmSafe[i][j])
            {
                SetInArmHome();
                return true;
            }
        }
    }
    return false;
}

//==============================================================================
//  MoveInArm2XYToLoaderWait()                             golden ainarm2.cpp:1374-1426
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : parks the in-arm at the NOT-hot wait position above the Loader tray,
//               used for one-cycle / clean-out / tray-feed.  Refuses while an
//               alarm-triggered ServoOff on the in-arm is still pending; refuses
//               (after HARD-STOPPING both in-arm axes and telling the operator) if
//               the Tray arm is not clear of the empty-tray X position; otherwise
//               samples the current pitch-axis positions so the pitch mechanism is
//               carried rather than re-driven, marks iInArmWaitPosition=1, and issues
//               the continuous XY move to (Tray pick X +6000, Tray pick Y -10000).
//  WAVE SCOPE : MoveInArm2XYToLoaderWait()  golden :1374-1426  -- ACTIVE (no gate)
//  GATE REGISTER: none.  Every symbol resolves: IniConfig.bAlarmNeedServoOff
//               (Config.h:125, reachable via cmydef.h -> cprod.h:6),
//               fNote->bMyServoOffInArm (forms/fNote.h:99), MOT[] +
//               TTrayMotor::ReadPos/PCIL132_StopMotor (Motor/mymotor.h:385/:264,
//               reachable via aHotPlateSubstrate.h:68), Prod.iXTrayEmpty
//               (cprod.h:470), ShowMyMessage (canary_support.h:80),
//               USE_IN_OUT_ARM_Y_PITCH + the four iXYPitch* consts + the MInArm*
//               motor indices (cmydef.h:2103/:2133/:2134/:2162/:2163/:2877-:2888),
//               InArmContinuousMove_9045 (Motor/mymotor.h:396 -- 7 args, bLoader
//               defaults false, signature matches golden exactly),
//               Prod.XInArm_Tray_Pick / YInArm_Tray_Pick (cprod.h:481-482),
//               iInArmXBase / iInArmYBase (cmydef.h:3638-3639).  ZeroMemory comes
//               from <windows.h>, pulled by vclcompat/vcl_compat.h.
//  TRAP 1     : shape (b) IS ALREADY PRESENT in the tree and this body does NOT fix
//               it -- csystem.cpp:4880-4881 carries
//                 static bool W7C1_MoveInArm2XYToLoaderWait(){ return true; }
//                 #define MoveInArm2XYToLoaderWait  W7C1_MoveInArm2XYToLoaderWait
//               so csystem.cpp`s call sites (:5578, :7333, :14702, plus the :11952
//               note) keep calling that TU-local always-true stub even after this
//               real body lands.  The macro must be deleted (main-loop task #15) or
//               "build green" will again prove nothing.  Note the stub returns TRUE
//               where this body returns FALSE on both refusal paths -- deleting the
//               macro is a real behaviour change, and that is the point.  Different
//               NAME, so this is not shape (d).
//  GOLDEN QUIRK PRESERVED (not fixed here): at golden :1396-1398 RogerYang 20250820
//               commented out the `if(...) return false; else` around the
//               ShowMyMessage, leaving :1399 indented as if it were still the else
//               arm.  It now runs unconditionally on that path.  Indentation and the
//               dead comment lines are transcribed verbatim.
//
//  HOIST ME to ainarm2.cpp`s include head at integration:
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "forms/fNote.h"
//  iInArmWaitPosition (golden ainarm2.h:219 -- THIS file is its golden home) has a
//  temporary port definition at acatchtray.cpp:135 and NO declaration in any header,
//  so it is declared TU-locally here exactly as asendic_Loader.cpp:233 already does.
//  TODO(integrate): when the golden-home definition moves into ainarm2.cpp, delete
//  this extern and the acatchtray.cpp:135 definition together.
extern int iInArmWaitPosition;                                                  // golden ainarm2.h:219
//==============================================================================
bool MoveInArm2XYToLoaderWait()                                                 //Steven 20120314 : 要移動到不熱的等待位置
{
    int iXVariable[X_PITCH_COUNT];
    int iYVariable=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
        if(fNote->bMyServoOffInArm)
        {
            return false;
        }
    }

    if(MOT[MTrayX].ReadPos()<Prod.iXTrayEmpty)                                  //JerryYang 20200206 one cycle, clean out, tray feed時in arm移動到loader
    {
        MOT[MInArmX].PCIL132_StopMotor();
        MOT[MInArmY].PCIL132_StopMotor();
//        if(MOT[MInArmY].ReadPos()<Prod.iInArmSafeY)                             //RogerYang 20250820 : InArmY到loader區才判斷
//            return false;
//        else
            ShowMyMessage("Tray arm not Safe pos", "MoveInArm2XYToLoaderWait");

        return false;
    }

    iXVariable[0]=MOT[MInArmPitch].ReadPos();
    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||
       USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                            //Ztex 2024.02.24 Add HT-1132
    {
        iXVariable[1]=MOT[MInArmPitchX2].ReadPos();
        iYVariable   =MOT[MInArmPitchY].ReadPos();
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                         //Steven for HT1032
            USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        iXVariable[1]=MOT[MInArmPitchX2].ReadPos();
        iXVariable[2]=MOT[MInArmPitchX3].ReadPos();
        iXVariable[3]=MOT[MInArmPitchX4].ReadPos();
        iYVariable   =MOT[MInArmPitchY].ReadPos();
    }

    iInArmWaitPosition=1;                                                       //Ifor 20191105 : add InArm 安全位置判斷 0:Wait 1:LoaderWait 2:DecayWait 3:Shuttle2
    if(InArmContinuousMove_9045(Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase]+6000, Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase]-10000, iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        return true;
    }
    return false;
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 1482..1535
// -----------------------------------------------------------------------------
//  ROLE        : move the in-arm X/Y onto the Decay (ion-fan) teach point, carrying the current pitch-motor positions through so the pitch axes are not disturbed; refuses and demands a re-home if an alarm left the in-arm servo off.
//  WAVE SCOPE  : MoveInArm2XYToDecayTeach -- ACTIVE, golden ainarm2.cpp:1482..1535.
//  COLLISION    : none -- no port body exists today (see EXPIRED GATES below).  This
//              : is a NET-NEW body and it is deliberately NON-static (TRAP 1(d)):
//              : golden declares `extern bool MoveInArm2XYToDecayTeach();` at
//              : ainarm2.h:145 and three port TUs already reference the name.
//  INCLUDES     : this body needs, beyond what port ainarm2.cpp already includes
//              : (cmydef.h / MachineType.h / cprod.h / ainarm9045.h /
//              : aHotPlateSubstrate.h):
//              : Motor/mymotor.h (MOT[] + InArmContinuousMove_9045 :396), Config.h (IniConfig.bAlarmNeedServoOff :125), forms/fNote.h (fNote->bMyServoOffInArm :99), canary_support.h (ShowMyMessage :78-79), cMyDB.h or acatchtray_shims.h (NewRecordProcess -- only inside the DEBUG_AutoHomeLog block)
//  GATE REGISTER: none.  Every symbol resolves: fAllMotorHome cmydef.h:222,
//              : CUSTOMER_CODE cmydef.h:3181, CC_SIGURD_PeiXing MachineType.h:319 (=946),
//              : MInArmPitch / MInArmPitchX2 / X3 / X4 / MInArmPitchY,
//              : USE_IN_OUT_ARM_Y_PITCH plus iXYPitchVariable / iXYPitchIn_Bb_Out_Bc /
//              : iXYPitch16Picker / iXYPitch16Bd_Be, Prod.iInArmDecay_X cprod.h:871 and
//              : Prod.iInArmDecay_Y cprod.h:873, bMoveInArm2XYToWaitOk cmydef.h:3509,
//              : X_PITCH_COUNT MachineType.h:408 (=4), InArmContinuousMove_9045
//              : Motor/mymotor.h:396 (8 parameters, last one defaulted -- golden passes 7).
//  ONE DECL HOIST: iInArmWaitPosition -- see the block just below this banner.
//  #ifdef KEPT  : the `#ifdef DEBUG_AutoHomeLog` block (golden :1496-:1501) is
//              : transcribed verbatim.  It is INACTIVE in this tree -- MachineType.h:32
//              : carries the define commented out, `//#define DEBUG_AutoHomeLog` --
//              : which is exactly golden's shipped configuration.  Not removed and not
//              : rewritten: a translation preserves every #ifdef block.
//  EXPIRED GATES: this body retires the PREMISE of THREE existing port gates, each of
//              : which states in prose that MoveInArm2XYToDecayTeach has no body
//              : anywhere in the tree:
//              :   (1) ainarm9045.cpp:1317  `#if 0 // TODO(W7) -- golden :9205 ...`
//              :       -- its #else substitutes `bDecayReached=true;`, i.e. it FAKES the
//              :       move as reached.  That is the dangerous direction (a stub turning
//              :       not-done into done) and it should be the first one retired.
//              :   (2) csystem.cpp:20296-20320  GATE G01 -- its own DELTA text explains
//              :       it took the fail-loud direction (bflag2 stays false, WAR2026 keeps
//              :       firing) precisely because faking the move would blow the ion fan
//              :       at an untaught X/Y.  THAT PREMISE IS THIS AGENT'S PREMISE TOO --
//              :       see gate W7E-K8-C in part 03799, which reuses G01's reasoning
//              :       instead of re-deriving it.
//              :   (3) csystem.cpp:31215-31240  the third call site.
//              : DO NOT retire any of the three inside this stitch: all three drive real
//              : motor moves, so each is a measured behaviour commit of its own.
// =============================================================================
// -----------------------------------------------------------------------------
//  DECLARATION HOIST (a declaration -- NOT a definition, NOT a global, NOT a gate)
//  iInArmWaitPosition has a real port DEFINITION with external linkage at
//  acatchtray.cpp:135 (`int  iInArmWaitPosition = 0;`, golden home cmydef.cpp,
//  Ifor 20191105) but NO `extern` declaration in ANY port header.  Command, run from
//  D:/HT9045/HT9011UC_Cpp_V3.33.906.0 at 2026-08-11 06:33:21 local:
//      rg -n --no-messages -g '*.h' 'iInArmWaitPosition' .
//  -> 1 hit, and that hit is PROSE INSIDE A COMMENT (acatchtray_shims.h:435, the note
//  recording that acatchtray.cpp owns this global this wave).  So the symbol LINKS but
//  does not COMPILE from here.  TRAP 3 applied: the answer to "no declaration" is to
//  DECLARE, not to gate -- gating would throw away real behaviour over a prototype.
//  INTEGRATOR: please hoist this one line into the stitched file's declaration block
//  beside the ainarm2 globals, and delete it from here.
#ifndef HT9045_W7E_K8_iInArmWaitPosition_DECLARED
#define HT9045_W7E_K8_iInArmWaitPosition_DECLARED
extern int iInArmWaitPosition;
#endif
// -----------------------------------------------------------------------------
bool MoveInArm2XYToDecayTeach()                                                 //Ifor 20151210 :新增InArm Move to Decay Teach 點位
{
    int iXVariable[X_PITCH_COUNT];
    int iYVariable=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
        if(fNote->bMyServoOffInArm)
        {
            #ifdef DEBUG_AutoHomeLog
            if(fAllMotorHome==true)
            {
                NewRecordProcess("", "fAllMotorHome", "MoveInArm2XYToDecayTeach");
            }
            #endif

            fAllMotorHome=false;                                                //Ifor 20151211 須重新回home
            if(CUSTOMER_CODE==CC_SIGURD_PeiXing)
            {
                ShowMyMessage("Input Arm Servo Off, need home");                //JerryYang 20160328 for 矽格北興,觸發回home的地方補上log
            }
            return false;
        }
    }

    iXVariable[0]=MOT[MInArmPitch].ReadPos();
    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||
       USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                            //Ztex 2024.02.24 Add HT-1132
    {
        iXVariable[1]=MOT[MInArmPitchX2].ReadPos();
        iYVariable   =MOT[MInArmPitchY].ReadPos();
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                         //Steven for HT1032
            USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        iXVariable[1]=MOT[MInArmPitchX2].ReadPos();
        iXVariable[2]=MOT[MInArmPitchX3].ReadPos();
        iXVariable[3]=MOT[MInArmPitchX4].ReadPos();
        iYVariable   =MOT[MInArmPitchY].ReadPos();
    }

    iInArmWaitPosition=2;                                                       //Ifor 20191105 : add InArm 安全位置判斷 0:Wait 1:LoaderWait 2:DecayWait 3:Shuttle2
    if(InArmContinuousMove_9045(Prod.iInArmDecay_X, Prod.iInArmDecay_Y, iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        bMoveInArm2XYToWaitOk=true;                                             //Steven 20131025 : 要先動到安全位置才可以再到別的地方
        return true;
    }
    return false;
}

//==============================================================================
//== ROLE: TransferLoaderRatio -- the Loader software gear ratio.  Re-scales an
//==       in-arm XY Loader-tray target about the teach base cell
//==       Prod.[XY]InArm_Tray_Pick[iInArmYBase][iInArmXBase] by whichever loader
//==       tray scale applies: tri-temp Hot, tri-temp Cold, the per-setup-file one,
//==       or the plain one.  No expansion coefficient is used on this path.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   TransferLoaderRatio  golden ainarm2.cpp:2112-2183  -- ACTIVE with 1 GATE
//==     (golden lines emitted VERBATIM apart from the #if 0 / #endif wrapping
//==      golden :2117)
//==
//== GATE REGISTER
//== -- GATE k2ai2-G2 ------------------------------------------------------------
//==   GOLDEN LINE         : ainarm2.cpp:2117
//==                         CheckInArmXYScaleByAutoTeach(*iXPos, *iYPos, AOA_Loader)
//==   WHY THE OFFLINE DEFAULT IS FAITHFUL:
//==     Identical premise to GATE k2ai2-G1 in part 01767 -- and per the task
//==     instruction I state the carry-over rather than re-deriving it.  The gated
//==     statement is the whole body of the
//==     if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true)
//==     arm; MACHINE_HAS_AUTO_ALIGNMENT_CCD is DEFINED false at cmydef.cpp:5316,
//==     so offline the arm is never entered and an empty arm is bit-identical.
//==     The JUSTIFICATION, though, is the absence premise below, not that
//==     equivalence (TRAP 2).
//==   REAL-MACHINE BEHAVIOUR DIFFERENCE:
//==     On an AOA machine golden replaces the loader gear ratio with the CCD
//==     auto-teach correction.  With this gate on such a machine gets NEITHER --
//==     the raw loader XY passes through with no compensation at all.
//==   EXACT COMMAND PROVING THE SYMBOL IS ABSENT (TRAP 2):
//==     grep -rnE "^[[:space:]]*(extern[[:space:]]+)?void[[:space:]]+CheckInArmXYScaleByAutoTeach" --include=*.cpp --include=*.h D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==     -> no output, exit 1.
//==   TIME I RAN IT: 2026-08-11 06:37:25 +0800.
//==   TRAP 3 -- WHY THIS SHOULD STAY GATED EVEN IF THE PREMISE DIES:
//==     Same as k2ai2-G1: the AutoClean.cpp:232 static stand-in must be deleted in
//==     the SAME commit that retires this gate, or that file gains a static shadow
//==     of a now-real function while still calling it at its :2261 (TRAP 1 shape
//==     (d)).  Also re-check the new signature takes int& / int&.
//==
//== GOLDEN QUIRKS PRESERVED (not fixed):
//==   * golden :2161 re-declares double fi; inside the
//==     else if(TestIF_File.bInArmUseDifferentScaleBySetupFile) block, SHADOWING
//==     the outer fi from :2121.  Reproduced verbatim -- expect a -Wshadow warning
//==     from the project warn flags; that warning is golden's, not a defect of
//==     this translation.
//==   * golden :2124 repeats &&LastSet.iTemperature==Tempture_Hot (with no space
//==     before the &&) inside an if that already tested exactly that.  Reproduced
//==     verbatim including the spacing.
//==   * Because the Hot arm at :2122 swallows the whole tri-temp block, the
//==     setup-file scale (:2159) and the plain scale (:2172) are UNREACHABLE while
//==     iTemperature==Tempture_Hot.  Reproduced verbatim.
//==   * The outer double fi at :2121 is left uninitialised, exactly as golden has
//==     it; every path that reads it assigns it first.
//==   * The double-to-int narrowing on *iXPos= / *iYPos= is golden's truncation
//==     and is load-bearing.  There is NO int/int division in this function.
//==
//== GATE-FREE DEPENDENCIES (all real, all resolved in this TU):
//==   Prod.{X,Y}InArm_Tray_Pick cprod.h:481/482 (object cprod.h:1138);
//==   LastSet.fLoaderTray{X,Y}Scale_Hot LastSet.h:546/547, _Cold :557/558, plain
//==   :381/382; TestIF_File.fLoaderTray{X,Y}ScaleBySetupFile cprod.h:2423/2424;
//==   AOA_Loader cprod.h:3175; the rest as listed in part 01767.
//==
//== INTEGRATION NOTE -- TRAP 1 shapes (a) AND (c), TWO STEPS:
//==   Unlike its HotPlate sibling this function has NO stub and NO declaration
//==   anywhere in the port.  Verified with
//==     grep -rn --include=*.cpp --include=*.h "TransferLoaderRatio" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==   ran 2026-08-11 06:26 +0800: every hit is either a comment or a GATED call.
//==   (1) ADD the declaration -- golden ainarm2.h:160 is
//==       extern void TransferLoaderRatio(int *iXPos, int *iYPos);
//==       Without it no caller can even name this function, so nothing extracts
//==       the archive member and the build stays green while doing nothing.
//==   (2) TWO LIVE GATES ELSEWHERE GO STALE THE MOMENT THIS LANDS, which is
//==       exactly TRAP 2 seen from the other side:
//==         * ainarm9045.cpp:7967  GATE k4-G4  -- its own text says "LIVE,
//==           re-verified absent 2026-08-11 01:50:31 +0800"; that claim expires
//==           against this part.
//==         * cInArmPlacement.cpp:724  GATE (1)
//==       Both must be retired in the same commit.  ainarm9045_2x8_32.cpp:1217 is
//==       a third call site but it is DEAD -- that file is #if 0 from its line 91
//==       to EOF -- so it needs no action beyond the eventual whole-file ungate.
//==   Not static -- once the declaration lands it is non-static (shape (d)
//==   avoided).
//==
//== TRAP 4 ANSWER: no file-scope object is added by this part.
//==
//== Big5 -> UTF-8: the Chinese comments on golden :2112, :2115, :2121 and :2158
//== are transcribed character for character from cp950.  Zero U+FFFD.
//==============================================================================
void TransferLoaderRatio(int *iXPos, int *iYPos)                                //Steven 20141029 : Loader的軟體齒輪比
{
    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&                                        //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
       TestIF.bEnableAutoAlignment==true)                                       //KenHsieh 20210824 : add AOA功能開啟不補償
    {
#if 0 // GATE k2ai2-G2 -- golden ainarm2.cpp:2117 CheckInArmXYScaleByAutoTeach: same dead symbol as GATE k2ai2-G1, the only body in the tree is the static no-op at AutoClean/AutoClean.cpp:232.  Retire BOTH gates together.  See this part's GATE REGISTER for the absence command, its timestamp and the real-machine delta.
        CheckInArmXYScaleByAutoTeach(*iXPos, *iYPos, AOA_Loader);
#endif
    }
    else
    {
        double fi;                                                              //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
        if(LastSet.iTemperature==Tempture_Hot)
        {
            if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE30_1InArmUseDifferentScale_Hot &&LastSet.iTemperature==Tempture_Hot)
            {
                fi=*iXPos-Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];
                fi*=LastSet.fLoaderTrayXScale_Hot;
                *iXPos=fi+Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];

                fi=*iYPos-Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
                fi*=LastSet.fLoaderTrayYScale_Hot;
                *iYPos=fi+Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
            }
            else
            {
                if(IniConfig.bE30_1InArmUseDifferentScale_Hot==true && Temperature.fWorkTemperBase>=26)
                {
                    fi=*iXPos-Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];
                    fi*=LastSet.fLoaderTrayXScale_Hot;
                    *iXPos=fi+Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];

                    fi=*iYPos-Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
                    fi*=LastSet.fLoaderTrayYScale_Hot;
                    *iYPos=fi+Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
                }

                if(IniConfig.bE30_2InArmUseDifferentScale_Cold==true && Temperature.fWorkTemperBase<26)
                {
                    fi=*iXPos-Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];
                    fi*=LastSet.fLoaderTrayXScale_Cold;
                    *iXPos=fi+Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];

                    fi=*iYPos-Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
                    fi*=LastSet.fLoaderTrayYScale_Cold;
                    *iYPos=fi+Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
                }
            }
        }                                                                       //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
        else if(TestIF_File.bInArmUseDifferentScaleBySetupFile)
        {
            double fi;

            fi=*iXPos-Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];
            fi*=TestIF_File.fLoaderTrayXScaleBySetupFile;
            *iXPos=fi+Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];

            fi=*iYPos-Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
            fi*=TestIF_File.fLoaderTrayYScaleBySetupFile;
            *iYPos=fi+Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
        }
        //pig 20220101 AddInputScaleBySetup
        else if(IniConfig.bE30InArmUseDifferentScale)
        {
            fi=*iXPos-Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];
            fi*=LastSet.fLoaderTrayXScale;
            *iXPos=fi+Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase];

            fi=*iYPos-Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
            fi*=LastSet.fLoaderTrayYScale;
            *iYPos=fi+Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase];
        }
    }
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 2620..2722
// -----------------------------------------------------------------------------
//  ROLE        : AutoTeach helper: move the IN arm (iInArm==0) or the OUT arm to the hard-coded per-nozzle teach X/Y for Z-number ZNumber, after the HotPlate-ratio transform; a Reset call just answers false.
//  WAVE SCOPE  : bMoveInArmXYToTeach -- ACTIVE, golden ainarm2.cpp:2620..2722.
//  COLLISION    : none in the port TODAY -- no port body, no port caller (measured
//              : 2026-08-11 06:20 local, `rg -w bMoveInArmXYToTeach` over
//              : D:/HT9045/HT9011UC_Cpp_V3.33.906.0 -> 0 hits).  BUT SEE THE GOLDEN BUG.
//  *** GOLDEN BUG -- NOT FIXED HERE, RECORDED AS REQUIRED ***
//              : golden defines `bool bMoveInArmXYToTeach(bool Reset, int ZNumber,
//              : int iInArm)` TWICE, both NON-static and both unconditionally compiled:
//              :     ainarm2.cpp:2620             (this part)
//              :     AutoTeach/AutoTeach.cpp:1972 (a MATERIALLY DIFFERENT body)
//              : Both translation units are in HT9045.bpr (`ainarm2.cpp` / `ainarm2.obj`
//              : and `AutoTeach\AutoTeach.cpp` / `AutoTeach.obj` all appear in it), and
//              : neither definition sits inside any #if or #ifdef -- I walked the
//              : preprocessor nesting of both files and the nesting depth at both
//              : definition lines is 0.  golden ainarm2.h does NOT declare the symbol,
//              : so each TU binds its own copy at the source level while the linker sees
//              : one public name: which body actually serves AutoTeach.cpp's 8 call
//              : sites (:2222, :2243, :2257, :2268, :2282, :2292) is link-order
//              : dependent.  The two bodies are NOT equivalent -- THIS one drives
//              : InArmContinuousMove_9045 / OutArmContinuousMove_9045 with the
//              : X_PITCH_COUNT pitch array and HAS the `USE_IN_Y_IS_AUTO_PITCH &&
//              : MACHINE_HAS_AUTO_ALIGNMENT_CCD -> iYPitchData=4000` branch, while the
//              : AutoTeach one drives individual MOT[].MotorMove() latched through
//              : static bMove[10] / bOutMove[10] and has NO 4000 branch.
//              : CONSEQUENCE FOR THE PORT: landing this body is collision-free now,
//              : because AutoTeach/AutoTeach.cpp is untranslated, and it WILL become a
//              : `multiple definition` link error the day AutoTeach lands.  Whoever
//              : translates AutoTeach must decide which body golden's AutoTeach form
//              : really gets -- that is a behaviour decision, not a build fix.
//  INCLUDES     : this body needs, beyond what port ainarm2.cpp already includes
//              : (cmydef.h / MachineType.h / cprod.h / ainarm9045.h /
//              : aHotPlateSubstrate.h):
//              : Motor/mymotor.h (MOT[], InArmContinuousMove_9045 :396, OutArmContinuousMove_9045 :409), aoutarm9045.h (GetOutArmPitchY_9045 :56, GetOutArmPitchX_9045 :57)
//  GATE REGISTER: none.  GetInArmPitchX_9045 ainarm9045.h:48 and GetInArmPitchY_9045
//              : ainarm9045.h:50, USE_IN_Y_IS_AUTO_PITCH cmydef.h:2890,
//              : MACHINE_HAS_AUTO_ALIGNMENT_CCD cmydef.h:5237, MachineTypeChoice
//              : cmydef.h:3286, Type_HT9046_LS MachineType.h:428 (=300),
//              : iInArmAutoYTeachOffset cmydef.h:4203, iOutArmAutoYTeachOffset
//              : cmydef.h:4204, TransferHotPlateRatio aHotPlateSubstrate.h:898.
//  INTEGER DIV  : `3000*(ZNumber/2)` at golden :2658, :2667, :2698 and :2706 is INTEGER
//              : division and it is LOAD-BEARING -- it is what makes nozzle pairs
//              : (A,B), (C,D) and so on share one X column while ZNumber%2 picks the Y
//              : row.  Left as int/int; converting it to floating point would smear
//              : the columns.  Likewise `ZNumber%2` stays integer modulo.
//  #ifdef KEPT  : golden's trailing `#ifdef SOFT_SIMULTE / return true / #else /
//              : return false / #endif` (:2717-:2721) is transcribed verbatim.
//  TRAP 1(a)    : NOTHING in the port calls this yet -- golden's only callers are the 8
//              : sites in the untranslated AutoTeach.cpp.  So a green build proves
//              : NOTHING about it: it is an unreferenced archive member and will not
//              : even be extracted from the archive.  Do not report it as 'linked'.
// =============================================================================
bool bMoveInArmXYToTeach(bool Reset, int ZNumber, int iInArm)
{                                                                               //目前使用吸嘴
    int iXPos=0, iYPos=0, iYPitchData=0;
    int iXVariable[X_PITCH_COUNT];
    int iYVariable  =0;
    int iMovePitchX =9000;
    int iMovePitchY =6000;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    int iOffsetPos=-1;
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(Reset)
    {
        return false;
    }

    if(iInArm==0)                                                               //INARM
    {
        iYVariable=GetInArmPitchY_9045(iMovePitchY, iOffsetPos);
        for(int i=0; i<X_PITCH_COUNT; i++)
            iXVariable[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);

        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||                          //kevin 20181203 (Steven) : Y變距
           USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                        //Ztex 2024.02.24 Add HT-1132
        {
            iYPitchData=3300;
        }

        if(USE_IN_Y_IS_AUTO_PITCH==true && MACHINE_HAS_AUTO_ALIGNMENT_CCD==true)                                        //Ztex 2024.02.24 Add HT-1132 HT-1032AT  //JerryYang 20251218 : IN/OUT ARM支援不同模組
        {
            iYPitchData=4000;
        }

        if(ZNumber%2==0)                                                        //吸嘴 A C E G
        {
            iXPos=29659-3000*(ZNumber/2);                                       //Tech.iInArmTeachX-3000*(ZNumber/2);

            if(MachineTypeChoice==Type_HT9046_LS)
                iYPos=-69249+iInArmAutoYTeachOffset+iYPitchData;                //kevin 20181203//kevin 20171115 add    //jou 2014-10-07 Auto Teach Z 新增 HT9046LS 點位
            else
                iYPos=-52130+iInArmAutoYTeachOffset;                            //kevin 20171115 add//Tech.iInArmTeachY;
        }
        else
        {
            iXPos=29659-3000*(ZNumber/2);

            if(MachineTypeChoice==Type_HT9046_LS)
                iYPos=-69249+6000+iInArmAutoYTeachOffset+iYPitchData;           //kevin 20181203//kevin 20171115 add   //jou 2014-10-07 Auto Teach Z 新增 HT9046LS 點位
            else
                iYPos=-52130+6000+iInArmAutoYTeachOffset;                       //kevin 20171115 add
        }
        TransferHotPlateRatio(false, &iXPos, &iYPos);                           //Steven 20110324

        if(InArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
            return true;
    }
    else
    {
        iYVariable=GetOutArmPitchY_9045(iMovePitchY, iOffsetPos);
        for(int i=0; i<X_PITCH_COUNT; i++)
            iXVariable[i]=GetOutArmPitchX_9045(iMovePitchX, i, iOffsetPos);

        if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||                          //kevin 20181203 (Steven) : Y變距
           USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                        //Ztex 2024.02.24 Add HT-1132
        {
            iYPitchData=3300;
        }

        if(USE_IN_Y_IS_AUTO_PITCH==true && MACHINE_HAS_AUTO_ALIGNMENT_CCD==true)                                        //Ztex 2024.02.24 Add HT-1132 HT-1032AT  //JerryYang 20251218 : IN/OUT ARM支援不同模組
        {
            iYPitchData=4000;
        }

        if(ZNumber%2==0)                                                        //吸嘴 A C E G
        {
            iXPos=-20193-3000*(ZNumber/2);
            if(MachineTypeChoice==Type_HT9046_LS)
                iYPos=-69249+iOutArmAutoYTeachOffset+iYPitchData;               //kevin 20181203 kevin 20171115 add     //jou 2014-10-07 Auto Teach Z 新增 HT9046LS 點位
            else
                iYPos=-52526+iOutArmAutoYTeachOffset;                           //kevin 20171115 add
        }
        else
        {
            iXPos=-20193-3000*(ZNumber/2);
            if(MachineTypeChoice==Type_HT9046_LS)
                iYPos=-69249+6000+iOutArmAutoYTeachOffset+iYPitchData;          //kevin 20181203//kevin 20171115 add    //jou 2014-10-07 Auto Teach Z 新增 HT9046LS 點位
            else
                iYPos=-52526+6000+iOutArmAutoYTeachOffset;                      //kevin 20171115 add
        }
        TransferHotPlateRatio(false, &iXPos, &iYPos);                           //Steven 20110324

        if(OutArmContinuousMove_9045(iXPos, iYPos, iXVariable, iYVariable, bZFlag, iZPos, false))
            return true;
    }
    #ifdef SOFT_SIMULTE
        return true;
    #else
        return false;
    #endif
}

//==============================================================================
//  bOffsetClean()                                         golden ainarm2.cpp:2726-2758
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : the operator`s "clear ALL taught position offsets" one-shot.  For
//               each of the 14 offset parts it picks the ambient or the Hot offset
//               data file, force-creates the directory, and writes 0.0 over the
//               part`s PickUp / Place keys plus every per-nozzle "PickUp <letter>" /
//               "Place <letter>" key of the in/out-arm suction grid.
//  WAVE SCOPE : bOffsetClean()  golden :2726-2758  -- GATED (whole body, golden
//               :2728-2757).  The function shell is ACTIVE and non-static so the
//               eventual ainarm2.h declaration binds to it.
//  RETIRES    : nothing -- there is no port stub and no port declaration for this
//               function (0 definitions tree-wide).  TRAP 1 shape (a): it will have
//               no caller until the integrator lands golden ainarm2.h`s declaration
//               and the fOffSet UI that calls it.
//
//  GATE REGISTER
//  ------------------------------------------------------------------------------
//  [k4e-G02]  golden :2728-2757   the entire body
//    SYMBOL (a): TfOffSet::GetOffsetPath()  -- golden cOffSet.h, called golden
//               :2731.  The port`s TfOffSet (forms/fOffSet.h:18-24) has exactly two
//               methods, UseAutoOffsetFunction(AnsiString) and UseInArmSetupTeach(int).
//               There is no GetOffsetPath anywhere.   ⚠ AI(W906-W2-OFFSET) 20260926: (a) GetOffsetPath 已過期 —— 8bfbab2f（JerryYang 0924）起 forms/fOffSet.h:320 宣告、cOffSet.cpp 定義（nm：ht9045_sm T），與 golden cOffSet.cpp:1426-1462 逐行 0 差異（NB2 Q3）
//      ABSENCE COMMAND (cwd D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//               rg -nw --glob "*.h" --glob "!build/**" GetOffsetPath .
//               RESULT: 0 lines.  RUN AT: 2026-08-11 06:31:32 +0800.
//               (SECSGEM/uHGemHT9045.cpp does write fOffSet->GetOffsetPath(...) at
//               :1132-:1145, but that file`s own note at :827-828 records the same
//               absence, so those uses are not evidence of a home.)
//    SYMBOL (b): CapStr[]  -- golden cOffSet.h:503 `extern AnsiString CapStr[]`,
//               indexed golden :2746/:2747/:2752/:2753.  The port has NO cOffSet.h at   ⚠ AI(W906-W2-OFFSET) 20260926: (b) 已過期 —— 0827 FW3-OFS2 起 forms/fOffSet.h:420 `extern AnsiString CapStr[];`，定義在 fOffSet.cpp（nm：ht9045_forms B）。(a)(b) 都到了，但本閘照下面 TRAP 3 維持閘住：0 個呼叫者、唯一效果是把真配方的取放補償寫成 0，要等測試路徑導向一起做
//               all, so the array does not exist.
//      ABSENCE COMMAND (same cwd):
//               rg -n --glob "*.cpp" --glob "*.h" --glob "!build/**" "CapStr\s*\[" .
//               RESULT: 0 lines.  RUN AT: 2026-08-11 06:31:32 +0800.
//               (language.h:34 / language.cpp:409 `AnsiString *CapStr` is a PARAMETER
//               of TfLan::GotLanString, not this array -- hand-checked 2026-08-11
//               06:20 +0800.)
//    EVERYTHING ELSE IN THE BODY DOES EXIST: MyForceDirectories (common.h:341),
//               WriteIniData(...,double) (common.h:249), LastSet.iTemperature,
//               Tempture_Hot, the Ofs* enumerators (MachineType.h:741/:765/:770/:782/
//               :787) and InArmSuck.iMotRow/iMotCol (aHotPlateSubstrate.h:419-420).
//               (a) and (b) are the only blockers.
//    WHY THE OFFLINE DEFAULT (empty body) IS FAITHFUL: bOffsetClean is a pure
//               operator-initiated side effect with no return value and no state any
//               SM reads back, and it has ZERO call sites in the port tree, so an
//               empty body is observationally identical to golden on every path
//               offline can reach.
//    REAL-MACHINE DIFFERENCE: pressing "clear offsets" silently does nothing --
//               every previously taught PickUp/Place value for all 14 parts, and
//               every per-nozzle letter key, survives instead of being zeroed.
//    TRAP 3 -- WHY IT SHOULD STAY GATED EVEN AFTER (a) AND (b) LAND: the body`s ONLY
//               effect is WriteIniData(..., 0.0) into
//               "<GetOffsetPath()>\Position Offset.Data" and
//               "...\Position Offset Hot.Data".  GetOffsetPath resolves under the
//               machine`s real setup tree, so an ungated bOffsetClean reachable from
//               ctest would ZERO the real machine`s taught position offsets on disk --
//               the same hazard class as WriteLastDataFile hard-coding
//               D:\HT9045\system\lastdata.dat.  Un-gate only together with a test
//               path redirection, never merely because the two symbols appeared.
//  ------------------------------------------------------------------------------
//  GOLDEN BUG PRESERVED (not fixed here): the loop bound is iSelPartData<=13, but the
//               Hot-file selector at golden :2740-2741 tests
//               iSelPartData>=OfsInSh1LB (27) and >=OfsOutSh1LB (44).  Those two
//               sub-conditions can NEVER be true inside a 0..13 loop, so the "For 32
//               Site Loader and Shuttle Offset" and "For 32 Site Out Shuttle Offset"
//               additions are dead: the 32-site LB/RB parts are never cleared at all,
//               and if they ever were reached they would be written to the ambient
//               file.  Kept verbatim.
//==============================================================================
void bOffsetClean()
{
#if 0 // GATE k4e-G02 -- golden :2728-2757, whole body.  fOffSet->GetOffsetPath() and
      // CapStr[] are both absent from the port; see the GATE REGISTER above for the
      // two absence commands + timestamps, the real-machine difference, and the
      // TRAP 3 reason this must stay gated even after both symbols land.
    AnsiString str;
    AnsiString szDir="", szFilePath="";

    szDir=fOffSet->GetOffsetPath();                                             //Steven 20190109 : 整合Offset路徑
    szFilePath=szDir;
    MyForceDirectories(szDir);

    for(int iSelPartData=0; iSelPartData<=13; iSelPartData++)
    {
        szDir=szFilePath;                                                       //kevin 20150105
        if(LastSet.iTemperature==Tempture_Hot && ((iSelPartData==OfsInSh1  || iSelPartData==OfsInSh2 ||
                                                   iSelPartData==OfsOutSh1 || iSelPartData==OfsOutSh2) ||
                                                  (iSelPartData>=OfsInSh1LB && iSelPartData<=OfsInSh2RB)||              //Steven 20140811 : For 32 Site Loader and Shuttle Offset
                                                  (iSelPartData>=OfsOutSh1LB && iSelPartData<=OfsOutSh2RB)))            //Steven 20190802 : For 32 Site Out Shuttle Offset
            szDir+="\\Position Offset Hot.Data";
        else
            szDir+="\\Position Offset.Data";

        WriteIniData(szDir, CapStr[iSelPartData], "PickUp",   0.0);
        WriteIniData(szDir, CapStr[iSelPartData], "Place",    0.0);
        for(int i=0; i<InArmSuck.iMotRow; i++)                                  //8=In/out arm suction
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)                              //8=In/out arm suction
            {
                WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("PickUp %c", 'A'+i+j*2), 0.0);
                WriteIniData(szDir, CapStr[iSelPartData], str.sprintf("Place %c", 'A'+i+j*2),  0.0);
            }
        }
        szDir="";
    }
#endif // GATE k4e-G02
}

//==============================================================================
//== ROLE: RecordAutoSiteMapStart -- when Auto Site Mapping is enabled, emit the
//==       SECS/GEM SiteMappingStart event (CEID 51) if SECS/GEM is on, and write
//==       the "Auto Site Map Start." line to the process log.  When ASM is
//==       disabled it does nothing at all.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   RecordAutoSiteMapStart  golden ainarm2.cpp:2760-2768  -- ACTIVE
//==     (golden lines emitted VERBATIM; ZERO gates)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   * IniConfig.bI21EnableASM Config.h:769, .bEnable_SECS_GEM Config.h:92
//==     (object Config.h:1499).
//==   * EventReport(unsigned) -- declared SECSGEM/SecsEventReport.h:55.  This is
//==     the Sim-first entry point, NOT golden UsecegemMainFrom.cpp:191; it has an
//==     observable counter side effect instead of talking to THGem.  That
//==     substitution predates this wave and is documented in that header, so it is
//==     not a gate of mine.
//==   * SECS_EVENT.SiteMappingStart -- SECSGEM/SecsEventType.h:93.  The trailing
//==     "51     Site Mapping Start" comment is golden's own CEID annotation.
//==   * RecordProcess(AnsiString,AnsiString) -- declared cMyDB.h:122.
//==
//== INTEGRATION NOTE -- TRAP 1 shape (a), NO CALLER AND NO DECLARATION:
//==   grep -rn --include=*.cpp --include=*.h "RecordAutoSiteMapStart" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==   -- ran 2026-08-11 06:16 +0800: ZERO hits.  Nothing in the port names this
//==   function today, so landing it cannot change any behaviour and a green build
//==   is no evidence of anything.  Golden ainarm2.h:228 is
//==     extern void RecordAutoSiteMapStart();
//==   which the integrator must add before any caller can reach it.  The natural
//==   first callers are golden's ASM start paths in main.cpp / csystem.cpp, which
//==   are not part of this wave.
//==   Not static -- once that declaration lands it is non-static (shape (d)
//==   avoided).
//==
//== Big5 -> UTF-8: the Chinese comment on golden :2760 is transcribed character
//== for character from cp950.  Zero U+FFFD.
//==============================================================================
void RecordAutoSiteMapStart()                                                   //Steven 20230117 : 修正Auto Site map的訊息
{
    if(IniConfig.bI21EnableASM)
    {
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.SiteMappingStart);                           //51     Site Mapping Start
        RecordProcess("Auto Site Map Start.");                                  //kevin 20160121  add
    }
}

//==============================================================================
//== ROLE: RecordAutoSiteMapFinish -- the closing half of RecordAutoSiteMapStart.
//==       With ASM enabled it emits SECS/GEM SiteMappingEnd (CEID 52) when
//==       SECS/GEM is on and logs "Auto Site Map Finish."; with ASM disabled it
//==       logs "Auto Site Map Disabled." instead.  EITHER WAY it clears
//==       bAutoSiteMapHasPickHP -- the flag is cleared on BOTH arms, which is the
//==       whole point of golden's Steven 20230210 fix.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   RecordAutoSiteMapFinish  golden ainarm2.cpp:2770-2784  -- ACTIVE
//==     (golden lines emitted VERBATIM; ZERO gates)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   Same four dependencies as part 02760, plus:
//==   * SECS_EVENT.SiteMappingEnd -- SECSGEM/SecsEventType.h:94.
//==   * bAutoSiteMapHasPickHP -- declared cmydef.h:4398 (bool).
//==
//== INTEGRATION NOTE -- TRAP 1 shape (a), NO CALLER AND NO DECLARATION:
//==   grep -rn --include=*.cpp --include=*.h "RecordAutoSiteMapFinish" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==   -- ran 2026-08-11 06:16 +0800: ZERO hits.  Golden ainarm2.h:229 is
//==     extern void RecordAutoSiteMapFinish();
//==   and must be added before anything can call it.  Because this function is the
//==   only writer that clears bAutoSiteMapHasPickHP on the ASM-disabled path,
//==   wiring the caller is a BEHAVIOUR change and should be measured, not folded
//==   into a translation-only commit.
//==   Not static -- shape (d) avoided.
//==
//== Big5 -> UTF-8: the Chinese comment on golden :2770 is transcribed character
//== for character from cp950.  Zero U+FFFD.
//==============================================================================
void RecordAutoSiteMapFinish()                                                  //Steven 20230117 : 修正Auto Site map的訊息
{
    if(IniConfig.bI21EnableASM)
    {
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.SiteMappingEnd);                             //52     Site Mapping End
        RecordProcess("Auto Site Map Finish.");                                 //kevin 20160121  add
        bAutoSiteMapHasPickHP=false;                                            //Steven 20230210 : Fixed for auto site map
    }
    else
    {
        RecordProcess("Auto Site Map Disabled.");
        bAutoSiteMapHasPickHP=false;                                            //Steven 20230210 : Fixed for auto site map
    }
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 2929..2932
// -----------------------------------------------------------------------------
//  ROLE        : rewind the after-pause in-arm-Z position-check cursor to 1.
//  WAVE SCOPE  : InitInArmZCheckPos -- ACTIVE, golden ainarm2.cpp:2929..2932.
//  COLLISION    : none -- no port body, no port declaration, no port caller.
//  INCLUDES     : none beyond the current set.
//  GATE REGISTER: none.  iInArmZCheckPosTask is golden ainarm2.cpp:110
//              : `int iInArmZCheckPosTask=1;` (Richard 2017.11.1) -- one of the ainarm2
//              : file-scope globals THE INTEGRATOR ADDS, so this part must not and does
//              : not emit it.  It is absent from the port until then: command run from
//              : D:/HT9045/HT9011UC_Cpp_V3.33.906.0 at 2026-08-11 06:20 local,
//              :     rg -w -g '*.cpp' -g '*.h' 'iInArmZCheckPosTask' .
//              : -> 0 hits.  If the integrator's global block does not carry golden
//              : :110, this part does not compile -- that is the correct loud failure
//              : mode, not a licence for this agent to define the global.
//  TRAP 1(a)    : golden itself has NO caller for this function.  An exhaustive scan of
//              : the golden tree finds only its own definition (ainarm2.cpp:2929), its
//              : declaration (ainarm2.h:202) and main.cpp:9945, which registers the
//              : CURSOR, not the function -- `QueueTaskList[206].SetAliasAndTask(
//              : "InArmZCheckPosTask", &iInArmZCheckPosTask)` takes the variable's
//              : address.  So this is a faithful translation of a golden function that
//              : nobody calls, and a green build says nothing whatsoever about it.
// =============================================================================
void InitInArmZCheckPos()
{
    iInArmZCheckPosTask=1;
}

//==============================================================================
//  InitInArmDevicePosPrecise()                            golden ainarm2.cpp:2946-2949
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : homes the in-arm Preciser-station (device-position-precise) task
//               cursor to step 1.
//  WAVE SCOPE : InitInArmDevicePosPrecise()  golden :2946-2949  -- ACTIVE
//  GATE REGISTER: none.
//  FILE-SCOPE GLOBALS DELIBERATELY NOT EMITTED: iInArmDevicePosPrecise is golden
//               ainarm2.cpp:111 and belongs to the integrator`s one-place block, so
//               it is USED here and assumed to exist.  Golden :2944 iOneKitPreciser
//               and :2945 bOneKitPreciser sit immediately above this function and are
//               ALSO integrator-owned -- they are outside my assigned range
//               (2946..2949) and are not emitted.
//  TRAP 1     : shape (a) today.  No port declaration exists (golden home
//               ainarm2.h:222 region) and the only textual caller,
//               ainarm9045.cpp:1174, sits inside that file`s
//               "#if 0 // TODO(W7) -- golden :3221-3239" block, so it is not a live
//               caller.  Non-static on purpose.  No stub to retire (0 definitions
//               tree-wide).  IN-WAVE UPDATE (checked 2026-08-11 06:45 +0800, 61
//               sibling part files present): the SM this cursor drives IS landing --
//               _w7e_parts/03050_DoInArmDevicePosPrecise.txt, whose golden :3052 does
//               `int &Task=iInArmDevicePosPrecise;` -- but NO sibling part calls
//               InitInArmDevicePosPrecise itself, so the no-caller status is
//               unchanged until ainarm9045.cpp:1174 is ungated.
//==============================================================================
void InitInArmDevicePosPrecise()                                                //Frank 20180410 (Steven) : InArm Preciser Station
{
    iInArmDevicePosPrecise=1;
}

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 02966 : SetPrecisorZPos
//  Golden: ainarm2.cpp:2966-3048 (cp950, CRLF).
//
//  ROLE          : precomputes, per in-arm nozzle, the Precisor-station Z target
//                  (iPrecise_Z[][]) and the "this nozzle goes down" flag (bPrecise_Z[][])
//                  for the four actions DoInArmDevicePosPrecise drives: iMoveDownPlace /
//                  iMoveDownPick take Prod.iInArmPreciserPlaceZ / iInArmPreciserPickUpZ,
//                  iMoveDownAdd200 takes the same +200, and anything else (iInitialFlag)
//                  clears every flag and parks every Z at ZSafePos.  Each of the first
//                  three actions has a one-kit variant (bOneKitPreciser: only row
//                  iOneKitPreciser) and a whole-grid variant.
//  WAVE SCOPE    : SetPrecisorZPos(int)  golden ainarm2.cpp:2966-3048  ACTIVE
//  GATE REGISTER : (none)  InArmSuck aHotPlateSubstrate.cpp:80 of class
//                  aHotPlateSubstrate.h:365 (see TRAP 5 in the report) ;
//                  Prod.iInArmPreciserPlaceZ/PickUpZ cprod.h:941/:942 ; ZSafePos
//                  Motor/mymotor.h:392 ; NULL_IC/HAS_NULL_IC cmydef.h.  iOneKitPreciser /
//                  bOneKitPreciser / iPrecise_Z / bPrecise_Z / iMoveDownPlace /
//                  iMoveDownAdd200 / iMoveDownPick are the integrator's file-scope globals
//                  (golden :2944/:2945/:2952-2965) -- USED, not defined here.
//  GOLDEN BUGS (kept verbatim -- do NOT "fix" at integration):
//                  (1) :2995 and :3028 test `InArmSuck.Item[j][j]!=HAS_NULL_IC` where every
//                      neighbouring line uses [i][j].  With MachineType.h:387-388
//                      MAX_ARM_Row=2 / MAX_ARM_Col=4 the loop runs j=0..3, so j=2 and j=3
//                      index PAST Item's row bound -- an in-object out-of-range read of
//                      TMyKitSuck (garbage, not a segfault) -- and the HAS_NULL_IC exclusion
//                      silently misfires for the whole non-one-kit path.  Preserved.
//                  (2) the iMoveDownAdd200 one-kit branch (:3017) adds 200 to
//                      iInArmPreciserPickUpZ while its whole-grid twin (:3031) adds 200 to
//                      iInArmPreciserPlaceZ.  Asymmetric in golden; preserved.
//                  (3) the else-branch comment at :3037 says `if(iAction==iInitialFlag)` but
//                      the branch is a bare else, so ANY unknown action parks at ZSafePos.
//  INTEGRATOR    : no stub to retire -- SetPrecisorZPos has no port definition and no
//                  header declaration (golden declares it nowhere either: its only callers
//                  are golden ainarm2.cpp:3130/:3161/:3259/:3302 inside
//                  DoInArmDevicePosPrecise, which is NOT in this wave).  Defined
//                  NON-static, exactly as golden, so no TRAP-1(d) static shadow.  Until
//                  DoInArmDevicePosPrecise lands this body has NO caller (TRAP-1(a));
//                  that is expected and is stated in the report.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0 (Chinese comment at golden :2966 transcribed
//                  character-for-character)
//==============================================================================
//==============================================================================
void SetPrecisorZPos(int iAction)                                               //Steven 20241009 : 重新整理Precisor Function
{
//    SetPrecisorZPos(iInitialFlag);

    if(iAction==iMoveDownPlace ||
       iAction==iMoveDownPick)
    {
        if(bOneKitPreciser==true)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(InArmSuck.Item[iOneKitPreciser][j]!=NULL_IC &&
                   InArmSuck.Item[iOneKitPreciser][j]!=HAS_NULL_IC)
                {
                    bPrecise_Z[iOneKitPreciser][j]=true;
                    if(iAction==iMoveDownPick)
                        iPrecise_Z[iOneKitPreciser][j]=Prod.iInArmPreciserPickUpZ[iOneKitPreciser][j];
                    else
                        iPrecise_Z[iOneKitPreciser][j]=Prod.iInArmPreciserPlaceZ[iOneKitPreciser][j];
                }
            }
        }
        else
        {
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(InArmSuck.Item[i][j]!=NULL_IC &&
                       InArmSuck.Item[j][j]!=HAS_NULL_IC)
                    {
                        bPrecise_Z[i][j]=true;
                        if(iAction==iMoveDownPick)
                            iPrecise_Z[i][j]=Prod.iInArmPreciserPickUpZ[i][j];
                        else
                            iPrecise_Z[i][j]=Prod.iInArmPreciserPlaceZ[i][j];
                    }
                }
            }
        }
    }
    else if(iAction==iMoveDownAdd200)
    {
        if(bOneKitPreciser==true)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(InArmSuck.Item[iOneKitPreciser][j]!=NULL_IC &&
                   InArmSuck.Item[iOneKitPreciser][j]!=HAS_NULL_IC)
                {
                    bPrecise_Z[iOneKitPreciser][j]=true;
                    iPrecise_Z[iOneKitPreciser][j]=Prod.iInArmPreciserPickUpZ[iOneKitPreciser][j]+200;
                }
            }
        }
        else
        {
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(InArmSuck.Item[i][j]!=NULL_IC &&
                       InArmSuck.Item[j][j]!=HAS_NULL_IC)
                    {
                        bPrecise_Z[i][j]=true;
                        iPrecise_Z[i][j]=Prod.iInArmPreciserPlaceZ[i][j]+200;
                    }
                }
            }
        }
    }
    else                                                                        //if(iAction==iInitialFlag)
    {
        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                bPrecise_Z[i][j]=false;
                iPrecise_Z[i][j]=ZSafePos;
            }
        }
    }
}

//------------------------------------------------------------------------------
//  DoInArmDevicePosPrecise()  --  golden ainarm2.cpp:3050-3421 (372 lines)
//  Wave PT-W7e, part k1-ainarm2.  Translator: AI(W7E-k1-ainarm2) 20260811
//
//  ROLE  The InArm PRECISER STATION state machine (Frank 20180410).  One pass per
//        kernel tick over `iInArmDevicePosPrecise'; returns true exactly once, at
//        case 2000, when the whole re-centring cycle is done.  Sequence:
//          1     decide one-kit vs two-kit precising (no Y pitch + 2 pick rows ->
//                one kit at a time), arm the open speed          -> 100
//          80    (one-kit second pass) re-open the precisor jaws  -> 100
//          100   Z back to plate-safe, then branch on USE_PRECISER / the two
//                Preciser-detect sensors                         -> 150 / 200
//          150   refuse to continue while an IC is still sitting in the Preciser
//          200   XY to the Preciser position (+ optional one-kit Y pitch offset,
//                + the auto-Y-pitch correction), compute the X/Y pitch vectors,
//                open the jaws, and sanity-check that the home sensor is OFF
//          300   Z down to the PLACE height                      -> 400
//          400   close the jaws onto the device (open->close span scaled by the
//                0.2mm/2.44mm tray tolerance and the operator offset), optionally
//                stopping for a manual offset entry              -> 420 / 1000
//          420   wait for the offset dialog, restart at 1 or retry 400
//          1000  mark every nozzle that did NOT go down as "already blown"
//                (FALLS THROUGH to 1050 -- golden has no break here)
//          1050  blow the devices off the nozzles onto the jaws   -> 1100
//          1100  Z down another 200 and return the nozzles to normal
//          1150  close the jaws the rest of the way (this is the actual centring)
//          1200  dwell
//          1300  Z down to the PICK height, then re-suck               -> 1310
//          1310  suck every nozzle back up; if any nozzle errored go 1400 to
//                re-open the jaws, else straight to 1500
//          1400  re-open the jaws after an error
//          1500  Z back to plate-safe, re-classify any dropped IC, and either
//                run the second kit (80) or finish (2000)
//          2000  clear bWaitPreciserFinish and return true
//        bWaitPreciserFinish is raised on EVERY entry (golden :3059) and only
//        cleared at 2000, which is what makes the other mechanisms wait.
//
//  WAVE SCOPE  DoInArmDevicePosPrecise  ACTIVE except ONE line  golden :3050-3421
//              (the only gate is k1-G3 at :3307; the gate k1-G2 that :3151-3152 was
//              going to get is WITHDRAWN -- see the GATE REGISTER, its premise died
//              inside the wave and that is recorded there rather than silently fixed).
//
//  GOLDEN BUGS FOUND -- ALL KEPT VERBATIM (do not "fix" on review):
//   (a) :3320 `flag=true;' in case 1310 is never set false anywhere in that case
//       (its sibling case 1050 does set it false at :3238).  So the
//       `if(flag==false) break;' at :3351-3352 is DEAD: case 1310 cannot wait for
//       a slow suck, it always falls through to the error scan in the same tick.
//   (b) :3337-3340 in case 1310 treats `Suck[i][j].Error==true' as "this nozzle is
//       done" (bDestroyflag=true), i.e. a vacuum error is swallowed as completion;
//       only the later Error scan at :3361 diverts to case 1400.
//   (c) :3216 case 1000 has NO `break' -- deliberate fall-through into case 1050
//       in the same tick.  Preserved exactly (do not add a break).
//   (d) :3212/:3312 `bDestroyflag[i][j]=!bPrecise_Z[i][j];' walks iMaxRow/iMaxCol
//       while bDestroyflag's initialiser at :3054 only names MAX_ARM_Row x
//       MAX_ARM_Col entries -- fine today, but the two bounds are independent.
//   (e) :3368 `if(Task!=1400)` reads Task right after a loop that may have set it
//       to 1400 -- the KevinCheng 20260521 comment says this is intentional
//       ("skip to case1500"), it just makes the loop's last write the decision.
//   (f) :3184 gates the manual-offset stop on AccessLevel>=1 only, so the
//       Sen[SnRKManualTStart] read happens on every 400 tick.
//
//  INTEGER / FLOAT NOTE (this is the trap that has cost this port before):
//   :3177-3179 and :3284-3285 are golden's own DOUBLE arithmetic --
//   dTrayTorence=((open-close)*0.2)/2.44 and dOffset=(open-close)*
//   (Offset.iPreciserOpen/244), where Offset.iPreciserOpen is a double
//   (cprod.h:252).  So `/244' here is NOT integer division and must stay double;
//   the result is then truncated by MotorMove(int) exactly as golden truncates it.
//   :3114 `iYPos+=(TestIF_File.dPreciserYPitch*100);' likewise truncates a double
//   into an int on assignment -- kept.  No float helper substituted anywhere.
//
//  HARD DEPENDENCIES -- DECLARED AND CALLED, NOT GATED (read this at integration)
//   Two functions this body calls live in THIS SAME GOLDEN FILE, a few dozen lines
//   outside my assigned range, and are on this wave's own target list
//   (tools/census/wave_targets.py ainarm2.cpp):
//     HD1  void SetPrecisorZPos(int iAction)      golden ainarm2.cpp:2966-3048 (83)
//     HD2  void PreciserPitchCalculate(int &iXPitch) golden ainarm2.cpp:3423-3457
//   Neither exists anywhere in the port yet.  I did NOT gate them, and that is a
//   deliberate, reported decision rather than an oversight:
//     * They are not absent cross-module dependencies -- they are in-file peers of
//       my range, and SetPrecisorZPos is the ONLY writer of bPrecise_Z[][] /
//       iPrecise_Z[][], i.e. of every Z target this SM hands to InArmZMoveDown /
//       InArmContinuousMove_9045.  Gating its four call sites (:3130, :3161, :3259,
//       :3302) would leave the file-scope bPrecise_Z all-false and iPrecise_Z all
//       zero, so the SM would sail through all four Z-down steps WITHOUT MOVING Z
//       and still report success -- a silent, safety-relevant behaviour loss, which
//       is worse than a loud failure.  Gating PreciserPitchCalculate would silently
//       pin the X pitch at the file-scope initial 12000 instead of the recipe value.
//     * So instead this part emits a GUARDED FORWARD DECLARATION of each (golden's
//       exact signature, no body, no `static' -- so it cannot become TRAP 1 shape
//       (d), and a sibling part's real definition redeclares compatibly).  If the
//       sibling parts for golden :2966 and :3423 are NOT part of this wave, the
//       stitch fails LOUDLY with `undefined reference to SetPrecisorZPos(int)' /
//       `...PreciserPitchCalculate(int&)' and the integrator must add them.  Both
//       are pure Item/Prod-grid computation with no HAL and total ~118 golden lines.
//     ABSENCE COMMANDS (exact):
//       rg -c -w SetPrecisorZPos D:/HT9045/HT9011UC_Cpp_V3.33.906.0        -> 0 files
//       rg -c -w PreciserPitchCalculate D:/HT9045/HT9011UC_Cpp_V3.33.906.0 -> 0 files
//     MEASURED AT: 2026-08-11T06:36:51+0800  (port .cpp/.h only; parts dir empty then).
//     *** TRAP 2 RE-RUN, 2026-08-11T06:48:10+0800 -- THE PREMISE IS ALREADY DEAD ***
//     Both bodies landed as SIBLING PARTS while this part was being written:
//       _w7e_parts/02966_SetPrecisorZPos.txt        `void SetPrecisorZPos(int iAction)'
//       _w7e_parts/03423_PreciserPitchCalculate.txt `void PreciserPitchCalculate(int &iXPitch)'
//     Signatures checked against these declarations: identical, non-static, external
//     linkage.  So the decision NOT to gate them was right, and the LOUD-failure risk
//     described above no longer exists.
//     KEEP BOTH DECLARATIONS ANYWAY -- one of them is now load-bearing: parts are
//     stitched in golden-line order, so 02966 lands BEFORE this part (its declaration
//     is redundant) but 03423 lands AFTER it, so without the forward declaration
//     `PreciserPitchCalculate(iPrecise_MoveXP)' at :3121 would not compile.  Removing
//     either one is only safe if the integrator also moves the corresponding body
//     above this range.
//   NOTE ON SetPrecisorZPos ITSELF (for whoever translates golden :2966): golden
//   :2995 and :3028 read `InArmSuck.Item[j][j]' where every neighbour reads
//   `[i][j]' -- a real golden bug that must be KEPT.  Flagging it here because my
//   range is its only caller.
//
//  GATE REGISTER
//   GATE k1-G2 -- *** WITHDRAWN.  NOT PRESENT IN THIS PART.  This entry is kept as
//   the record of a TRAP 2 expiry that was caught before delivery, not after. ***
//     It would have gated golden :3151-3152, inside case 200's home-sensor check:
//       bool bPreciNeedHome=true;
//       SetInArmHome(bPreciNeedHome);
//     WHY IT WAS DROPPED: re-listing the parts directory at 2026-08-11T06:48:10+0800
//     found _w7e_parts/01266_SetInArmHome.txt, a sibling part whose definition line is
//     `void SetInArmHome(bool bPrecisorNeedHome)' -- GOLDEN'S ARITY, non-static,
//     landing in THIS SAME TU and (golden-line order) BEFORE my range.  With it in the
//     TU the call site compiles: the header's zero-arg `SetInArmHome()' and the
//     sibling's `SetInArmHome(bool)' form an overload set and `SetInArmHome(bool)' is
//     the exact match.  Gating a call whose real body arrives in the same file in the
//     same wave would have been a defect, so the two lines are ACTIVE and faithful.
//     THE FORWARD DECLARATION below makes this independent of part ordering, and it
//     deliberately carries NO default argument (golden's .cpp definition has none
//     either): adding `=false' while aHotPlateSubstrate.h:906's zero-arg declaration
//     is still visible would make every existing `SetInArmHome()' call AMBIGUOUS.
//     INTEGRATION HAZARD THE INTEGRATOR STILL OWNS (this is the part that did NOT go
//     away): the tree then holds TWO functions -- the empty no-op
//     `void SetInArmHome() {}' at aHotPlateSubstrate.cpp:1067, which all 15 existing
//     zero-arg call sites (ainarm9045_2x4_16.cpp:1410/:1576/:2029, ainarm9045_2x8_32
//     .cpp:1909/:4553, ainarm_SearchPickPlate.cpp:1165, AutoClean/AutoClean.cpp:4277/
//     :4893, csystem.cpp:1491/:2786/:19938/:24159, Motor/mymotor.cpp:639,
//     RotateKit/aRotateKIT_In.cpp:3692) keep resolving to, and the real body.  A green
//     build proves nothing here -- this is exactly TRAP 1 shape (b).  Fixing it means
//     changing aHotPlateSubstrate.h:906 to golden arity WITH the default and deleting
//     the no-op, in one commit, which is what 01266_SetInArmHome.txt's own banner also
//     asks for.  MY range is the ONLY caller that passes the flag.
//     ABSENCE COMMAND still worth re-running (it is what made the gate look needed):
//       rg -n -e "SetInArmHome\s*\(\s*bool" --glob '*.cpp' --glob '*.h' D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//       -> 0 hits at 2026-08-11T06:36:51+0800 and STILL 0 at 06:46:03 (part files are
//          .txt, so they are invisible to that glob -- the parts DIRECTORY listing is
//          the check that matters during a wave, and that is the lesson here).
//
//   [historical text of the withdrawn gate, kept for the audit trail]
//     SIGNATURE MISMATCH, not a missing name.  Golden declares
//     `void SetInArmHome(bool bPrecisorNeedHome=false)' (golden ainarm2.h:136, body
//     golden ainarm2.cpp:1266 -- also on this wave's target list).  The port has
//     ONLY a ZERO-ARG `void SetInArmHome()' (declared aHotPlateSubstrate.h:906,
//     defined aHotPlateSubstrate.cpp:1067 as `{}'), and all 15 existing port call
//     sites pass no argument.  Passing the flag therefore does not compile, and
//     dropping the argument silently would be worse than gating: it would claim the
//     "precisor also needs homing" request was honoured when nothing carries it.
//     The DECLARATION of bPreciNeedHome is gated together with the call because the
//     variable exists solely to feed it.
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: the port's only SetInArmHome IS an empty
//     no-op body, so even the zero-arg call would change nothing offline; the
//     ShowMyMessage, `Task=100;' and `break;' around it stay ACTIVE, so the SM still
//     refuses to advance and still re-runs case 100 exactly as golden does.
//     REAL-MACHINE DIFFERENCE: on hardware, golden re-homes the in-arm Z axes AND
//     asks for a precisor home before retrying; gated, the operator gets the same
//     "Precisor open but home sensor is on!" message and the SM loops back to case
//     100 without a home request -- so a genuinely mis-homed precisor would keep
//     failing the same check instead of being re-homed.  It fails SAFE (it will not
//     proceed with a bad precisor) but it will not self-recover.
//     ABSENCE COMMAND (exact):
//       rg -n -e "SetInArmHome\s*\(\s*bool" --glob '*.cpp' --glob '*.h' D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//       -> 0 hits (the only declaration is the zero-arg aHotPlateSubstrate.h:906).
//     MEASURED AT: 2026-08-11T06:36:51+0800.
//     TRAP 2/TRAP 3: golden :1266 WAS on this wave's target list and DID land (see the
//     WITHDRAWN header above), which is precisely why this gate no longer exists.
//     [end of historical text]
//
//   GATE k1-G3 -- golden :3307 `InArmSuck.ClearAllError();' inside case 1300.
//     TRAP 5 SHAPE, and the answer is still "stay gated".  A real body exists --
//     mykitsuck.cpp:2689 `void TMyKitSuck::ClearAllError()' (declared
//     mykitsuck.h:392) -- but it belongs to the OTHER TMyKitSuck.  This TU's
//     InArmSuck is the aHotPlateSubstrate.h:365 mirror (aHotPlateSubstrate.cpp:80),
//     the one 177 TUs use, and that class has NO ClearAllError.  mykitsuck.cpp is
//     DELIBERATELY not registered in the build (CMakeLists.txt:2099 "*** DELIBERATELY
//     NOT REGISTERED IN THIS WAVE: mykitsuck.cpp ***", :2105 notes both headers
//     declare the same extern globals with different layouts), so switching this TU
//     to mykitsuck.h would read every InArmSuck field at the wrong offset while
//     linking perfectly.  Reaching for it "just for one error-clear" is exactly the
//     dependency the MyMessageBox precedent says not to take.
//     WHY THE OFFLINE DEFAULT IS FAITHFUL: golden's ClearAllError only clears the
//     per-nozzle TMySucker::Error flags before the re-suck at :3333.  Offline
//     TMySucker::Suck()/Destroy() never SET Error (aHotPlateSubstrate.h:109 is only
//     ever read in this file's paths), so there is nothing to clear and the
//     following case-1310 scan behaves identically.
//     REAL-MACHINE DIFFERENCE: on hardware a vacuum error latched during the blow-off
//     phase (cases 1050/1100) would survive into case 1310; combined with golden bug
//     (b) above that nozzle is then marked done AND diverts the SM to case 1400
//     (jaw re-open) even though the re-suck never happened.  Net effect: an extra
//     jaw-open/close and a device that may not be held.  This is the one gate here
//     with a real hardware consequence -- it must be retired together with the
//     TMyKitSuck unification (task #-shaped work, same as GATE k1-G1's).
//     ABSENCE COMMAND (exact):
//       rg -n -w ClearAllError --glob '*.cpp' --glob '*.h' D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//       -> 4 hits, ALL in mykitsuck.{h:392,cpp:112,cpp:2689,cpp:2749}; ZERO in
//          aHotPlateSubstrate.h.
//       rg -n "mykitsuck.cpp" CMakeLists.txt  -> only the "NOT REGISTERED" comment.
//     MEASURED AT: 2026-08-11T06:36:51+0800.
//
//  TRAP 4 STATEMENT: this part adds NO file-scope object of any kind -- the only
//  static it introduces is golden's own FUNCTION-LOCAL `static bool bDestroyflag
//  [MAX_ARM_Row][MAX_ARM_Col]' (:3054), a POD with a constant initialiser, so no
//  constructor runs before main() and no NULL global can be touched.  fMain is
//  dereferenced only inside case 400 (:3187), at run time, exactly as golden does,
//  and the existing tree already relies on that (acarry.cpp:3994, acatchtray.cpp:7007,
//  ainarm9045.cpp:6127).
//
//  TRAP 5 STATEMENT: InArmSuck / TMySucker come from aHotPlateSubstrate.h (the
//  :365 TMyKitSuck, object defined aHotPlateSubstrate.cpp:80).  NOT mykitsuck.h.
//  No TInLaserCheck is touched.
//
//  TRAP 1 STATEMENT: nothing here is `static' at file scope; the definition is a
//  plain external one, so it can only be reached by resolving the real symbol.
//  There is no pre-existing stub of DoInArmDevicePosPrecise in the tree -- its one
//  call site (ainarm9045.cpp:1203) sits inside `#if 0 // TODO(W7)' (ainarm9045.cpp
//  :1192), so shape (b) cannot hide this body: retiring that gate is what makes it
//  reachable, and until then this body has NO live caller (shape (a)).  Say so out
//  loud rather than let a green build imply otherwise.
//    rg -n -w DoInArmDevicePosPrecise --glob '*.cpp' --glob '*.h' D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//    -> 3 hits, all in ainarm9045.cpp (:1119 comment, :1192 gate text, :1203 the
//       gated call).  MEASURED AT 2026-08-11T06:33:56+0800.
//
//  Big5: TWENTY non-ASCII golden lines in this range -- :3051, :3059, :3062, :3081,
//  :3086, :3099, :3108, :3116, :3132, :3142, :3160, :3169, :3170, :3176, :3277,
//  :3278, :3368, :3373, :3376, :3415 (measured, not eyeballed) -- transcribed
//  character for character from cp950, including the Chinese STRING LITERAL passed
//  to ShowMyMessage at :3099.  ZERO U+FFFD.
//
//  INCLUDES this part needs (each one MEASURED with a scratchpad probe TU -- g++
//  6.3 -std=c++1z -fsyntax-only over port ainarm2.cpp + this part, first run
//  2026-08-11T06:47:12+0800 gave exactly these five "was not declared in this
//  scope" errors and nothing else; adding them cleared the TU).  HOIST ME to the
//  ainarm2.cpp include head at integration -- they are mid-file only because a part
//  file may not touch the head, the same convention ainarm9045.cpp:3057/:5389/
//  :6091/:10607 already uses.
//    "mysensor.h"      Sen[] / TMySensor::Enable / IsOn()  (mysensor.h:42/44/48)
//                      -- guarded header, pulls only vclcompat + MachineType.h, so
//                      it cannot swap a same-named class into this TU (TRAP 5 check:
//                      it does NOT include mykitsuck.h).
//    "canary_support.h" ShowMyMessage(AnsiString,...)      (canary_support.h:80)
//    "FormsFacade.h"    fMain->Pause(AnsiString)           (forms/fMain.h:142)
//    "csystem.h"        CheckInArmSuckICFallDownToHasNullIC(bool bAlarm=true)
//                                                          (csystem.h:99)
//------------------------------------------------------------------------------
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "mysensor.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "FormsFacade.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem.h"
//------------------------------------------------------------------------------
// HD1/HD2 forward declarations -- golden's own signatures (golden ainarm2.cpp:2966
// and :3423; golden needs no declaration there only because both bodies precede
// this one in golden's file order, which the port's append-only stitch does not
// guarantee).  Guarded so a sibling part's identical declaration is harmless; a
// declaration is NOT a definition, so no ODR/duplicate-symbol risk either way.
#ifndef HT9045_SetPrecisorZPos_DECLARED
#define HT9045_SetPrecisorZPos_DECLARED
void SetPrecisorZPos(int iAction);                                              // golden ainarm2.cpp:2966
#endif
#ifndef HT9045_PreciserPitchCalculate_DECLARED
#define HT9045_PreciserPitchCalculate_DECLARED
void PreciserPitchCalculate(int &iXPitch);                                      // golden ainarm2.cpp:3423
#endif
// SetInArmHome at GOLDEN'S arity (golden ainarm2.h:136 declares it with a default
// argument; golden's DEFINITION at ainarm2.cpp:1266 carries none, and neither does
// this -- see the WITHDRAWN GATE k1-G2 note above for why adding `=false' here would
// make every existing zero-arg call ambiguous against aHotPlateSubstrate.h:906).
// Body: sibling part _w7e_parts/01266_SetInArmHome.txt, same TU.
#ifndef HT9045_SetInArmHome_BOOL_DECLARED
#define HT9045_SetInArmHome_BOOL_DECLARED
void SetInArmHome(bool bPrecisorNeedHome);                                      // golden ainarm2.cpp:1266
#endif
//------------------------------------------------------------------------------
bool DoInArmDevicePosPrecise()                                                  //Frank 20180410 (Steven) : InArm Preciser Station
{                                                                               //Steven 20241009 : 重新整理Precisor Function
    int &Task=iInArmDevicePosPrecise;

    static bool bDestroyflag[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
    double dOffset=0, dTrayTorence=0;
    int iXPos=0, iYPos=0;
    bool flag;

    bWaitPreciserFinish=true;                                                   //Ifor 20180703 (Steven) : : 等待 Preciser 完成
    switch(Task)
    {
        case 1:                                                                 //決定哪幾隻要下去做Precise
            if(TestIF_File.dPreciserYPitch==0 &&
               InArmSuck.iPickRow==2)
                bOneKitPreciser=true;
            else
                bOneKitPreciser=false;

            iOneKitPreciser=0;
            MOT[MPreciser].SetSpeed(ArmSpeed_File[0].PrecisorOpenSp);           //JerryYang 20250120 : add
            Task=100;
            break;
        case 80:
            if(DoInArmDevicePosPreciseDelay.Off())
            {
                bPreciseFlag=MOT[MPreciser].MotorMove(Prod.iPreciserOpenPitch);
                if(bPreciseFlag)
                    Task=100;
            }
            break;
        case 100:                                                               //Z軸回Safe位置
            if(MoveInArmZToPlateSafe(Task))
            {
                bPreciseFlag=false;
                bPrecise_XYZ=false;
                if(USE_PRECISER==2)                                             //Steven 20191113 : 使用HP當Preciser
                    Task=200;
                else if(Sen[SnPreciserDetect1].Enable==true ||
                        Sen[SnPreciserDetect2].Enable==true)
                    Task=150;
                else
                    Task=200;
            }
            break;
        case 150:
            if((Sen[SnPreciserDetect1].Enable && Sen[SnPreciserDetect1].IsOn()==false) ||
               (Sen[SnPreciserDetect2].Enable && Sen[SnPreciserDetect2].IsOn()==false))
            {
                ShowMyMessage("請將Preciser內IC移除");
            }
            else
            {
                bPreciseFlag=false;
                bPrecise_XYZ=false;
                Task=200;
            }
            break;
        case 200:                                                               //移動至Precise 位置  Precise打開 X Pitch Y Pitch 開合完成
            iXPos=Prod.iInArmPreciserX+InArmOffSet[InOfsPreciser]->GetX();
            iYPos=Prod.iInArmPreciserY+InArmOffSet[InOfsPreciser]->GetY();

            iPrecise_MoveYP=TestIF_File.iARM_Y_PITCH;
            if(bOneKitPreciser && iOneKitPreciser==1)
                iYPos+=(TestIF_File.dPreciserYPitch*100);

            if(USE_IN_Y_IS_AUTO_PITCH==true)                                    //Steven 20131002 : XY變距  //JerryYang 20251218 : IN/OUT ARM支援不同模組
            {
                iYPos=(iYPos-TestIF_File.iARM_Y_PITCH);
            }

            PreciserPitchCalculate(iPrecise_MoveXP);
            if(iPrecise_MoveXP>12000)                                           //JerryYang 20250120 : add
            {
                iPrecise_MoveXP=12000;
            }
            for(int i=0; i<X_PITCH_COUNT; i++)
                iPrecise_XP[i]=GetInArmPitchX_9045(iPrecise_MoveXP, i, InOfsPreciser);
            iPrecise_YP=GetInArmPitchY_9045(iPrecise_MoveYP, InOfsPreciser);

            SetPrecisorZPos(iInitialFlag);

            if(USE_PRECISER==2)                                                 //Steven 20191113 : 使用HP當Preciser
                bPreciseFlag=true;
            else if(bPreciseFlag==false)
                bPreciseFlag=MOT[MPreciser].MotorMove(Prod.iPreciserOpenPitch);

            if(bPrecise_XYZ==false)
                bPrecise_XYZ=InArmContinuousMove_9045(iXPos, iYPos, iPrecise_XP, iPrecise_YP, bPrecise_Z, iPrecise_Z, false);

            if(bPrecise_XYZ && bPreciseFlag)
            {
                if(USE_PRECISER!=2)                                             //KevinCheng 20260521 : HP當Preciser時不檢查Sensor
                {
                    MOT[MPreciser].ScanMotorStatus();

                    MOT[MPreciser].SetSpeed(ArmSpeed_File[0].PrecisorCloseSp);      //JerryYang 20250120 : add

                    if(MOT[MPreciser].Led[iHomeLed]==true)
                    {
                        ShowMyMessage("Precisor open but home sensor is on!");
                        bool bPreciNeedHome=true;
                        SetInArmHome(bPreciNeedHome);
                        Task=100;
                        break;
                    }
                }
                Task=300;
            }
            break;
        case 300:                                                               //使用的Z軸下降
            SetPrecisorZPos(iMoveDownPlace);

            bPrecise_XYZ=InArmZMoveDown(bPrecise_Z, iPrecise_Z, false);
            if(bPrecise_XYZ==true)
            {
                Task=400;
            }
            break;
        case 400:                                                               //Precise 夾持
            if(USE_PRECISER==2)                                                 //Steven 20191113 : 使用HP當Preciser
            {
                bPreciseFlag=true;
            }
            else
            {
//              0.2mm/2.44mm=x/(Prod.iPreciserOpenPitch-Prod.iPreciserClosePitch)   //0.2mm為20條 = Tray盤Torence
                dTrayTorence=((Prod.iPreciserOpenPitch-Prod.iPreciserClosePitch)*0.2)/2.44;
                dOffset     =(Prod.iPreciserOpenPitch-Prod.iPreciserClosePitch)*(Offset.iPreciserOpen/244);
                bPreciseFlag=MOT[MPreciser].MotorMove(Prod.iPreciserClosePitch+dTrayTorence+dOffset);
            }

            if(bPreciseFlag)
            {
                if(AccessLevel>=1 && Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoInArmDevicePosPrecise");
                    Task=420;
                    break;
                }

                DoInArmDevicePosPreciseDelay.SetMSAndOn(100);
                Task=1000;
            }
            break;
        case 420:
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=1;
            }
            else
            {
                Task=400;
            }
            break;
        case 1000:
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    bDestroyflag[i][j]=!bPrecise_Z[i][j];
                }
            }
            Task=1050;
        case 1050:
            if(DoInArmDevicePosPreciseDelay.Off())
            {
                flag=true;
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if((bOneKitPreciser==false) ||
                           (bOneKitPreciser==true && i==iOneKitPreciser))
                        {
                            if(bDestroyflag[i][j]==false)
                            {
                                if(InArmSuck.Item[i][j]!=NULL_IC &&
                                   InArmSuck.Item[i][j]!=HAS_NULL_IC)
                                {
                                    if(InArmSuck.Suck[i][j].Destroy())
                                    {
                                        bDestroyflag[i][j]=true;
                                    }
                                    else if(InArmSuck.Suck[i][j].Error==false)
                                    {
                                        flag=false;
                                    }
                                }
                                else
                                {
                                    bDestroyflag[i][j]=true;
                                }
                            }
                        }
                    }
                }

                if(flag==false)
                    break;
                DoInArmDevicePosPreciseDelay.SetMSAndOn(100);
                Task=1100;
            }
            break;
        case 1100:
            if(DoInArmDevicePosPreciseDelay.Off())
            {
                SetPrecisorZPos(iMoveDownAdd200);
                bPrecise_XYZ=InArmZMoveDown(bPrecise_Z, iPrecise_Z, false);

                if(bPrecise_XYZ==true)
                {
                    for(int i=0; i<InArmSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iMaxCol; j++)
                        {
                            if(bPrecise_Z[i][j]==true)
                                InArmSuck.Suck[i][j].Normal();
                        }
                    }

                    Task=1150;
                }
            }
            break;
        case 1150:                                                              //Precise 夾持
            if(USE_PRECISER==2)                                                 //Steven 20191113 : 使用HP當Preciser
            {
                bPreciseFlag=true;
            }
            else
            {
                dOffset     =(Prod.iPreciserOpenPitch-Prod.iPreciserClosePitch)*(Offset.iPreciserClose/244);
                bPreciseFlag=MOT[MPreciser].MotorMove(Prod.iPreciserClosePitch+dOffset);
            }

            if(bPreciseFlag)
            {
                DoInArmDevicePosPreciseDelay.SetMSAndOn(100);
                bPreciseFlag=false;
                Task=1200;
            }
            break;
        case 1200:
            if(DoInArmDevicePosPreciseDelay.Off())
            {
                Task=1300;
            }
            break;
        case 1300:
            SetPrecisorZPos(iMoveDownPick);
            bPrecise_XYZ=InArmZMoveDown(bPrecise_Z, iPrecise_Z, false);

            if(bPrecise_XYZ==true)
            {
//AI(W906-W2-A46) 20260926: gate LIFTED -- A4-6 (8ff6c754) merged TMyKitSuck/TMySucker into the one golden layout (mykitsuck.h), premise dead: TMyKitSuck::ClearAllError() is mykitsuck.h:392 (nm T) -- golden :3307
                InArmSuck.ClearAllError();                                      //Steven 20141107 : Add for Loader pickup Error
//AI(W906-W2-A46) 20260926: (end of lifted gate)
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        bDestroyflag[i][j]=!bPrecise_Z[i][j];
                    }
                }

                Task=1310;
            }
            break;
        case 1310:
            flag=true;
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if((bOneKitPreciser==false) ||
                       (bOneKitPreciser==true && i==iOneKitPreciser))
                    {
                        if(bDestroyflag[i][j]==false)
                        {
                            if(InArmSuck.Item[i][j]!=NULL_IC &&
                               InArmSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                if(InArmSuck.Suck[i][j].Suck())
                                {
                                    bDestroyflag[i][j]=true;
                                }
                                else if(InArmSuck.Suck[i][j].Error==true)
                                {
                                    bDestroyflag[i][j]=true;
                                }
                            }
                            else
                            {
                                bDestroyflag[i][j]=true;
                            }
                        }
                    }
                }
            }

            if(flag==false)
                break;

            DoInArmDevicePosPreciseDelay.SetMSAndOn(100);
            bPreciseFlag=false;

            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(InArmSuck.Suck[i][j].Error)
                    {
                        Task=1400;
                    }
                }
            }

            if(Task!=1400)                                                      //KevinCheng 20260521 : 補到case1500的路徑
            {
                Task=1500;
            }
            break;
        case 1400:                                                              //Precise 打開
            if(DoInArmDevicePosPreciseDelay.Off())
            {
                if(USE_PRECISER==2)                                             //Steven 20191113 : 使用HP當Preciser
                    bPreciseFlag=true;
                else
                    bPreciseFlag=MOT[MPreciser].MotorMove(Prod.iPreciserOpenPitch);

                if(bPreciseFlag)
                {
                    bPreciseFlag=false;
                    Task=1500;
                }
            }
            break;
        case 1500:
            if(MoveInArmZToPlateSafe(Task))
            {
                DoInArmDevicePosPreciseDelay.SetMSAndOn(100);
                CheckInArmSuckICFallDownToHasNullIC();
                bPreciseFlag=false;
                if(bOneKitPreciser==true)
                {
                    if(iOneKitPreciser==0)
                    {
                        iOneKitPreciser=1;
                        Task=80;
                    }
                    else
                    {
                        Task=2000;
                    }
                }
                else
                {
                    Task=2000;
                }
            }
            break;
        case 2000:
            if(DoInArmDevicePosPreciseDelay.Off())
            {
                bWaitPreciserFinish=false;                                      //Ifor 20180703 : 等待 Preciser 完成
                return true;
            }
            break;
    }
    return false;
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 3423..3457
// -----------------------------------------------------------------------------
//  ROLE        : compute the InArm Preciser-station X pitch, in 0.01 mm motor units, for the current test mode and write it through the int& out-parameter.
//  WAVE SCOPE  : PreciserPitchCalculate -- ACTIVE, golden ainarm2.cpp:3423..3457.
//  COLLISION    : none -- no port body, no port declaration, no port caller.
//  INCLUDES     : none beyond the current set.  TestIF / TestIF_File are cprod.h:2577
//              : and dPreciserXPitch is cprod.h:1663, a double.
//  GATE REGISTER: none -- every symbol resolves.
//  ARITHMETIC IS LOAD-BEARING, in three separate places, all transcribed verbatim:
//              : (1) golden :3425 `int iPreciserXPitch=TestIF_File.dPreciserXPitch*100;`
//              :     -- double*100 TRUNCATED to int, so 40.99 mm becomes 4099, not 4100.
//              : (2) golden :3439 `iPreciserXPitch+iPreciserXPitch/2` -- INTEGER
//              :     division; an odd value loses its half count right here.  NOT
//              :     converted to any float helper: a previous float-helper
//              :     substitution in this campaign broke rounding and produced stepped
//              :     position drift.
//              : (3) golden :3455 `iXPitch=iPreciserXPitch*1.5;` -- int = int*double,
//              :     truncated.  Left exactly as written; NOT rewritten as *3/2.
//  GOLDEN QUIRK : the `else if(TestIF.iTestMode==DualSite2x1)` arm at :3441-:3444 has an
//              : EMPTY body whose only content is the commented-out `// return false;`
//              : -- inside a void function.  It exists purely to stop DualSite2x1 from
//              : falling into the DualSite arm below, so iXPitch keeps the plain
//              : iPreciserXPitch assigned at :3427.  Transcribed including the dead
//              : comment; the commented-out `return false;` is NOT reinstated.
//  TRAP 1 / TRAP 2 -- MY OWN ABSENCE CLAIM HERE EXPIRED INSIDE THE WAVE, AND THIS IS
//              : THE CORRECTED TEXT.  At 2026-08-11 06:20 local `rg -w
//              : PreciserPitchCalculate D:/HT9045/HT9011UC_Cpp_V3.33.906.0` returned 0
//              : hits and I was about to report this as TRAP 1 shape (a), nobody
//              : references it.  Re-checked at 2026-08-11 06:43 local against the part
//              : directory and that was already FALSE: sibling part
//              : _w7e_parts/03050_DoInArmDevicePosPrecise.txt had landed golden's only
//              : call site (golden ainarm2.cpp:3121) AND a guarded forward declaration
//              : `void PreciserPitchCalculate(int &iXPitch);` of exactly this signature,
//              : deliberately un-gated so a missing body fails loudly.  So after the
//              : stitch this body IS referenced from inside the same TU, the archive
//              : member IS extracted, and its int& out-parameter really does drive that
//              : SM's iPrecise_MoveXP.  The lesson recorded, not just the fact: an
//              : "unreferenced" claim about an in-file peer is the shortest-lived kind
//              : of absence claim there is -- re-run it at integration anyway.
// =============================================================================
void PreciserPitchCalculate(int &iXPitch)
{
    int iPreciserXPitch=TestIF_File.dPreciserXPitch*100;

    iXPitch=iPreciserXPitch;
    if(TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site2X4 ||
       TestIF.iTestMode==_16Site2X8  || TestIF.iTestMode==_12Site2X6 ||
       TestIF.iTestMode==TriSite1X3  ||                                         //Frank 20160329 add for 1x3_4
       TestIF.iTestMode==_6Site2X3   ||                                         //ChungHung 20140115 add for 2x3
       TestIF.iTestMode==_8Site1X4   ||                                         //ChungHung 20150528 add for 海思 _8Site1x4
       TestIF.iTestMode==_16Site4X4  ||                                         //Sam 20190226 : 16Site4X4
       TestIF.iTestMode==_32Site4X8N)                                           //2013-01-15    Dell    Add nn Mode
    {
        if(TestIF_File.dPreciserXPitch<41.0)                                    //Steven 20210412 : 修正Preciser的Pitch錯誤問題
            iXPitch=iPreciserXPitch*3;
        else
            iXPitch=iPreciserXPitch+iPreciserXPitch/2;
    }
    else if(TestIF.iTestMode==DualSite2x1)
    {
//        return false;
    }
    else if(TestIF.iTestMode==QualSite2X2)
    {
        if(TestIF_File.dPreciserXPitch<41.0)                                    //Steven 20210412 : 修正Preciser的Pitch錯誤問題
            iXPitch=iPreciserXPitch*3;
    }
    else if(TestIF.iTestMode==DualSite)                                         //Steven 20150505 : 1x2加大支援X-Pitch 120mm
    {
        if(iPreciserXPitch>8000)
            iXPitch=12000;
        else
            iXPitch=iPreciserXPitch*1.5;
    }
}

//==============================================================================
//== ROLE: DoVibrateOutShuttle -- the out-shuttle vibration-motor state machine,
//==       one task cursor per shuttle.  Step 1 switches the vibrator on, arms the
//==       dwell for IniConfig.iF25VibrateTime*100 ms and accumulates the motor
//==       run-time counter; step 100 switches it off and arms a fixed 1000 ms
//==       settle; step 200 ends the cycle by returning true.  Returns true only
//==       once, at the end of a completed vibrate-and-settle cycle.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   DoVibrateOutShuttle  golden ainarm2.cpp:3528-3583  -- ACTIVE
//==     (golden lines emitted VERBATIM; ZERO gates)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   * SW[] -- extern class TMySwitch SW[MAX_SWITCH_ITEM], myswitch.h:43; On()
//==     myswitch.h:32, Off() myswitch.h:33.  Reached through the extra include
//==     block emitted by part 00320.
//==   * SwShuttleVibration1 / SwShuttleVibration2 -- extern const int,
//==     cmydef.h:1872/1873.
//==   * IniConfig.iF25VibrateTime Config.h:694, .iVibratorSht1 Config.h:249,
//==     .iVibratorSht2 Config.h:250 (object Config.h:1499).
//==   * iVibrateOutShuttleTask[2] -- golden ainarm2.cpp:3527, i.e. ON THE MAIN
//==     LOOP'S file-scope-globals list.  This part does NOT emit it (two agents
//==     emitting one global is a multiple-definition link error); it just uses it.
//==   * VibrationOutShuttleDelay[2] -- golden ainarm2.cpp:88, TQPF_Timer, also on
//==     the main loop's globals list and likewise NOT emitted here.  It is already
//==     TQPF_Timer in golden, so it is NOT one of the two HTimer cases the
//==     integrator has to retype: .SetMSAndOn(DWORD) is myTimer.h:38 and .Off() is
//==     myTimer.h:30, a REAL timer whose Off() actually measures elapsed time.  If
//==     it were mis-declared as the atester_shims.h:463 HTimer instead, that Off()
//==     is hard-coded true and BOTH dwells here (the iF25VibrateTime one and the
//==     fixed 1000 ms settle) would silently collapse to zero.
//==
//== GOLDEN QUIRKS PRESERVED (not fixed):
//==   * golden :3551 and :3555 compute int(IniConfig.iF25VibrateTime/10.0) --
//==     FLOATING division by 10.0 on purpose, then truncated by the int cast.
//==     Transcribed EXACTLY.  It is deliberately NOT rewritten as /10, and no
//==     integer division exists anywhere in this function, so the known
//==     float-helper-substitution regression cannot apply here.
//==   * case 150 (golden :3568-:3573) is UNREACHABLE -- nothing in golden ever
//==     assigns 150 to iVibrateOutShuttleTask.  Kept verbatim, break intact.
//==   * iShuttleVibraCount (golden :3539) is incremented at :3558 and zeroed at
//==     :3577 but never read anywhere.  Kept, including its function-local static
//==     storage.
//==   * AnsiString str; (golden :3537) is never used.  Kept verbatim.  MEASURED:
//==     it does NOT produce an unused-variable warning under -Wall -Wextra,
//==     because AnsiString has a non-trivial constructor, so GCC assumes the
//==     declaration has a side effect.  Likewise iShuttleVibraCount produces no
//==     -Wunused-but-set-variable because it has static storage.
//==   * The two statics bInitial and iShuttleVibraCount are FUNCTION-LOCAL, which
//==     is golden's design: bInitial is NOT per-iShuttle, so the first call for
//==     shuttle 1 after shuttle 0 has initialised does NOT re-initialise -- callers
//==     pass bNeedInitial=true to force it.  Being function-local they create no
//==     TRAP 1 shape (d) file-scope static shadow.
//==
//== INTEGRATION NOTE -- TRAP 1 shapes (a) AND (e), THREE STEPS:
//==   grep -rn --include=*.cpp --include=*.h "DoVibrateOutShuttle" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==   -- ran 2026-08-11 06:16 +0800: no real body anywhere; every hit is a
//==   comment, a gate, or the renamed stand-in below.
//==   (1) ADD the declaration -- golden ainarm2.h:215 is
//==       extern bool DoVibrateOutShuttle(int iShuttle, bool bNeedInitial=false);
//==       The default argument matters: golden calls the one-argument form.  It
//==       belongs on the DECLARATION only -- golden's definition at :3528 carries
//==       no default and this part matches that.
//==   (2) aTester_Rear.cpp:392 holds static bool W64bT2_DoVibrateOutShuttle(int,
//==       bool=false) -- a RENAMED TU-local stand-in, TRAP 1 shape (e) inside the
//==       engine rather than inside tests.  It does not collide with this
//==       definition, which is precisely the danger: the build stays green and
//==       aTester_Rear.cpp:704 and :774 keep calling the stand-in forever.  Delete
//==       it and repoint both call sites.
//==   (3) aTester_Front.cpp:602 and :671 hold the same two calls under
//==       #if 0 // TODO(W7) with the reason "ainarm2.h, no translated home yet".
//==       That premise dies with this part; retire both.
//==   Not static -- once the declaration lands it is non-static (shape (d)
//==   avoided).
//==
//== TRAP 4 ANSWER: no file-scope object is added by this part.
//==
//== Big5 -> UTF-8: the Chinese comments on golden :3528, :3549, :3561, :3569 and
//== :3575 are transcribed character for character from cp950.  Zero U+FFFD.
//==============================================================================
bool DoVibrateOutShuttle(int iShuttle, bool bNeedInitial)                       //JerryYang 20190123 shuttle震動馬達
{
    static bool bInitial=true;
    if(bInitial || bNeedInitial)
    {
        iVibrateOutShuttleTask[iShuttle]=1;
        bInitial=false;
        return false;
    }
    AnsiString str;
    int iShuttleVibration=0;
    static int iShuttleVibraCount=0;
    if(iShuttle==0)
        iShuttleVibration=SwShuttleVibration1;
    else
        iShuttleVibration=SwShuttleVibration2;
    switch(iVibrateOutShuttleTask[iShuttle])
    {
        case 1:
            SW[iShuttleVibration].On();
            VibrationOutShuttleDelay[iShuttle].SetMSAndOn(IniConfig.iF25VibrateTime*100);
            if(iShuttle==0)                                                     //JerryYang 20200612 振動馬達作動時間累計
            {
                IniConfig.iVibratorSht1=IniConfig.iVibratorSht1+int(IniConfig.iF25VibrateTime/10.0);
            }
            else
            {
                IniConfig.iVibratorSht2=IniConfig.iVibratorSht2+int(IniConfig.iF25VibrateTime/10.0);
            }
            iVibrateOutShuttleTask[iShuttle]=100;
            iShuttleVibraCount++;
            break;
        case 100:
            if(VibrationOutShuttleDelay[iShuttle].Off())                        //JerryYang 20171205 (Steven) shuttle震動馬達功能
            {
                SW[iShuttleVibration].Off();
                iVibrateOutShuttleTask[iShuttle]=200;
                VibrationOutShuttleDelay[iShuttle].SetMSAndOn(1000);
            }
            break;
        case 150:
            if(VibrationOutShuttleDelay[iShuttle].Off())                        //JerryYang 20171205 (Steven) shuttle震動馬達功能
            {
                iVibrateOutShuttleTask[iShuttle]=1;
            }
            break;
        case 200:
            if(VibrationOutShuttleDelay[iShuttle].Off())                        //JerryYang 20171212 (Steven) 關閉震動後先delay 1秒,避免shuttle馬上移動IC會飛走
            {
                iShuttleVibraCount=0;
                return true;
            }
            break;
    }
    return false;
}

//------------------------------------------------------------------------------
//  EnableTraymapCheckFunction(int iCheck)  --  golden ainarm2.cpp:3585-3610
//  Wave PT-W7e, part k1-ainarm2.  Translator: AI(W7E-k1-ainarm2) 20260811
//
//  ROLE  One predicate, two questions selected by iCheck, both meaning "is the
//        tray-map DATA check feature live right now?":
//          iCheck==0 -> ... && TestIF_File.bEnableSuckMapCheck   (do the check)
//          iCheck==1 -> ... && bTrayMapDataCheckErrorSkip        (auto-SKIP on a
//                                                                 data mismatch)
//        Both share the same three preconditions: tray-ID running (bRTNoRunTrayID
//        false), tray mapping installed (USE_TRAY_MAPPING != etmUninstall), and a
//        recipe that enables either the tray map or tray-ID-with-FTP-download.
//        Any other iCheck returns the initial false.
//        Golden's header gives it a default argument (`int iCheck=0',
//        golden ainarm2.h:216) -- the DEFINITION carries none, exactly as here.
//
//  WAVE SCOPE  EnableTraymapCheckFunction  ACTIVE  golden :3585-3610.
//              CheckTrayMapData (golden :3612) is NOT in this part.
//
//  GATE REGISTER  none -- every symbol is real and verified by header+line:
//    bRTNoRunTrayID cmydef.h:4869, USE_TRAY_MAPPING cmydef.h:3006, etmUninstall
//    MachineType.h:910 (enum eTrayMap), TestIF_File.bEnableTrayMap cprod.h:2253,
//    .bEnableTrayID cprod.h:2254, .bEnableTrayIDDownFTP cprod.h:2265,
//    .bEnableSuckMapCheck cprod.h:2264, bTrayMapDataCheckErrorSkip cmydef.h:4263.
//
//  *** THIS PART EXPIRES FIVE EXISTING GATES (TRAP 3 -- and here the premise
//  GENUINELY dies, so they should be RETIRED, not rewritten) ***
//  Five #if 0 blocks in the tree are gated on exactly "EnableTraymapCheckFunction
//  has no port body and no port declaration".  Landing this body kills that
//  premise.  They must be re-examined together (all five call it with a literal
//  0 and/or 1, nothing else):
//    ainarm9045.cpp:5110  GATE W7D-K2-G05   golden ainarm9045.cpp:2637-2648
//    ainarm9045.cpp:5546  GATE W7D-G01      golden ainarm9045.cpp:2900
//    ainarm9045.cpp:10423 GATE k6-G4        golden ainarm9045.cpp:7132-7137
//    aTester_Rear.cpp:9793   TODO(G18)
//    aTester_Front.cpp:9863  GATE W7F3-G03
//  (ainarm9045.cpp:10255 already predicts this: "retired together when ainarm2.cpp's
//  EnableTraymapCheckFunction lands".)  Note W7D-K2-G05 also gates MyDeCodeASCII,
//  so THAT one may have to stay gated for its second, independent reason -- check
//  before deleting.  Retiring them changes behaviour only when the tray-map recipe
//  flags are on, and each of those gates documents its own direction.
//  A DECLARATION is still missing tree-wide: golden's home is ainarm2.h:216 and the
//  port has no ainarm2.h, so the integrator must publish
//    extern bool EnableTraymapCheckFunction(int iCheck=0);
//  (aHotPlateSubstrate.h's "-- functions (golden ainarm2.cpp)" block, next to
//  ResetInToShtFlag at :1066, is where its siblings live) before any of the five
//  call sites can be ungated.  I did not edit that header -- out of my boundary.
//    rg -c --glob '*.cpp' --glob '*.h' -w EnableTraymapCheckFunction D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//    -> before this part: 3 files (ainarm9045.cpp / aTester_Rear.cpp /
//       aTester_Front.cpp), every hit inside gate text or a gated call; no
//       declaration, no body.  MEASURED AT 2026-08-11T06:33:56+0800.
//------------------------------------------------------------------------------
bool EnableTraymapCheckFunction(int iCheck)
{
    bool Result=false;

    if(iCheck==0)
    {
        if(bRTNoRunTrayID==false &&
           USE_TRAY_MAPPING!=etmUninstall &&
           (TestIF_File.bEnableTrayMap || (TestIF_File.bEnableTrayID && TestIF_File.bEnableTrayIDDownFTP)) &&
           TestIF_File.bEnableSuckMapCheck)                                     //wei 20170302 (Steven) Tray map data error
        {
            Result=true;
        }
    }
    else if(iCheck==1)
    {
        if(bRTNoRunTrayID==false &&
           USE_TRAY_MAPPING!=etmUninstall &&
           (TestIF_File.bEnableTrayMap || (TestIF_File.bEnableTrayID && TestIF_File.bEnableTrayIDDownFTP)) &&
           bTrayMapDataCheckErrorSkip)                                          //wei 20170302 Tray map data error
        {
            Result=true;
        }
    }
    return Result;
}

//==============================================================================
//  RestoreLoadeIC(int)  --  golden ainarm2.cpp:3660-3725
//
//  ROLE          End of the "try the last row once more" rescue (E62): when the
//                in-arm has reached the far row of the loader tray, clear
//                bTryPickLastRowIng, FLIP the tray scan Direction back (0<->3,
//                1<->2), and restore every cell the buffer still says had an IC
//                but the live map had degraded to NULL_IC -- excluding the row
//                just attempted.
//  WAVE SCOPE    RestoreLoadeIC()           ACTIVE   golden :3660-3725
//  GATE REGISTER (none -- every symbol has a real linkable home)
//                  IniConfig.bE62TryPickLastRow Config.h; bTryPickLastRowIng
//                    cmydef.h/cmydef.cpp (real global, not one of the 37).
//                  TrayForm.Loader.Direction cprod.h:1265; MOT[] Motor/mymotor.h
//                    :385 with Tray.XItem/.YItem mytray.h:33/:34, Tray.Data :40,
//                    Tray.BufferData :46, SetTraySingleData(int,int,int,int=0)
//                    Motor/mymotor.h:357; MMTrayY cmydef.h; HAS_IC/NULL_IC cmydef.h.
//
//  GOLDEN ODDITIES PRESERVED (deliberate, verified against golden, not tidied):
//    * :3692-3694 an EMPTY `else { }`.  Kept byte-for-byte.
//    * the two arms scan DIFFERENT row windows on purpose -- Direction 0/1 uses
//      `j < YItem-1` and Direction 2/3 uses `j = 1 ... YItem`.  Each excludes the
//      end row it has just finished retrying.  This asymmetry is the feature; do
//      not "symmetrise" it.
//    * golden's header declares a default argument (ainarm2.h:220
//      `RestoreLoadeIC(int iLoaderY=0)`); the DEFINITION carries none.  Kept as
//      golden has it -- the default belongs on the declaration only.
//  OFFLINE REACHABILITY: net-new symbol (no prior stub anywhere).  Its 1 live call
//    site is inert offline because IniConfig.bE62TryPickLastRow is false, so the
//    whole body is skipped -- landing it is behaviour-neutral offline.
//==============================================================================
void RestoreLoadeIC(int iLoaderY)                                               //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
{
    if(IniConfig.bE62TryPickLastRow && bTryPickLastRowIng==true)
    {
        if(TrayForm.Loader.Direction==0 || TrayForm.Loader.Direction==1)
        {
            if(iLoaderY==MOT[MMTrayY].Tray.YItem-1)
            {
                bTryPickLastRowIng=false;
                if(TrayForm.Loader.Direction==0)
                {
                    TrayForm.Loader.Direction=3;
                }
                else if(TrayForm.Loader.Direction==1)
                {
                    TrayForm.Loader.Direction=2;
                }

                for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                {
                    for(int j=0; j<MOT[MMTrayY].Tray.YItem-1; j++)
                    {
                        if(MOT[MMTrayY].Tray.BufferData[i][j]==HAS_IC)
                        {
                            if(MOT[MMTrayY].Tray.Data[i][j]==NULL_IC)
                            {
                                MOT[MMTrayY].SetTraySingleData(i, j, HAS_IC);
                            }
                        }
                    }
                }
            }
            else
            {
            }
        }
        else
        {
            if(iLoaderY==0)
            {
                bTryPickLastRowIng=false;
                if(TrayForm.Loader.Direction==2)
                {
                    TrayForm.Loader.Direction=1;
                }
                else if(TrayForm.Loader.Direction==3)
                {
                    TrayForm.Loader.Direction=0;
                }
                for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                {
                    for(int j=1; j<MOT[MMTrayY].Tray.YItem; j++)
                    {
                        if(MOT[MMTrayY].Tray.BufferData[i][j]==HAS_IC)
                        {
                            if(MOT[MMTrayY].Tray.Data[i][j]==NULL_IC)
                            {
                                MOT[MMTrayY].SetTraySingleData(i, j, HAS_IC);
                            }
                        }
                    }
                }
            }
        }
    }
}

//==============================================================================
//  VacuumOnOffLog(AnsiString sLog)                        golden ainarm2.cpp:3727-3744
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : appends one CSV row to the vacuum-generator on/off life log, bucketed
//               into a per-day directory and a per-hour file, writing the column
//               header first if that hour`s file does not exist yet.
//  WAVE SCOPE : VacuumOnOffLog(AnsiString)  golden :3727-3744  -- ACTIVE (no gate)
//  GATE REGISTER: none.  asVacuumOnOffLogPath (common.h:155, defined common.cpp:194 =
//               "D:\HT9045_log\VacuumOnOffLog"), MyForceDirectories (common.h:341),
//               WriteDataToFile (common.h:338), FileExists (vclcompat/SysUtils.h:90)
//               and SystemYear/SystemMonth/SystemDate/SystemHour (cmydef.h:227/:226,
//               type Word) all have real bodies.
//  WHY NOT GATED DESPITE WRITING UNDER D:\ (contrast GATE k4e-G02 in this same wave):
//               this is an APPEND-ONLY log under D:\HT9045_log, not a rewrite of
//               machine state.  It cannot destroy a taught value or a saved lot, and
//               the port already writes several sibling logs to the same tree
//               (asEPLogPath, asTorqLogPath, asBarCodeLogPath).  It also only runs
//               when a caller asks; there is no timer behind it.
//  TRAP 1     : shape (a) today -- no port declaration and no other definition (0
//               tree-wide), so no caller until golden ainarm2.h`s declaration lands.
//               Non-static on purpose.  Nothing to retire.
//  FIDELITY NOTE: golden passes raw AnsiString objects to the printf-family
//               (s.sprintf("%s", sLog) and Path.sprintf with asVacuumOnOffLogPath).
//               That is transcribed VERBATIM, with no .c_str() added:
//               vclcompat/AnsiString.h:142 is a variadic template whose conv()
//               overload (:177) converts an AnsiString argument to c_str() for
//               exactly this BCB6 idiom.  Adding .c_str() would be a gratuitous edit.
//  GOLDEN QUIRKS PRESERVED (not fixed here): s.sprintf("%s", sLog) is a pointless
//               copy of sLog; Path is formatted twice and the first value is used
//               only for MyForceDirectories; the header string
//               "Time,Use Site,Test Time, Vacuum On,Vacuum Off" has an inconsistent
//               space before "Vacuum On"; and the two-statement if-body at golden
//               :3739-3740 is indented with 3 spaces, not 4.
//
//  HOIST ME to ainarm2.cpp`s include head at integration:
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "common.h"
//==============================================================================
void VacuumOnOffLog(AnsiString sLog)
{
    AnsiString s;
    AnsiString Path, buffer;

    s.sprintf("%s", sLog);
    Path.sprintf("%s\\%04d_%02d_%02d", asVacuumOnOffLogPath, SystemYear, SystemMonth, SystemDate);
    MyForceDirectories(Path);

    Path.sprintf("%s\\%04d_%02d_%02d\\%04d_%02d_%02d_%02d.txt", asVacuumOnOffLogPath, SystemYear, SystemMonth, SystemDate, SystemYear, SystemMonth, SystemDate , SystemHour);
    if(FileExists(Path)==false)
    {
       buffer.sprintf("Time,Use Site,Test Time, Vacuum On,Vacuum Off");
       WriteDataToFile(Path, buffer);
    }

    WriteDataToFile(Path, s);
}

//==============================================================================
//  MoveInArm2XYToClean()  --  golden ainarm2.cpp:3746-3787
//
//  ROLE          Move the in-arm to the in-rotate (die-clean) station with every
//                nozzle commanded DOWN to the die-clean height: derive the X pitch
//                per axis from a fixed 12000 and Y from 6000 through the
//                InOfsRotate_In offset family, set bZFlag/iZPos for the whole
//                nozzle grid, and hand the move to InArmContinuousMove_9045 with
//                ZAxisDown.
//  WAVE SCOPE    MoveInArm2XYToClean()      ACTIVE   golden :3746-3787
//  GATE REGISTER (none -- every symbol has a real linkable home)
//                  GetInArmPitchX_9045(int,int,int=-1) ainarm9045.h:48;
//                    GetInArmPitchY_9045(int,int=-1) ainarm9045.h:50.
//                  InArmSuck -- TMyKitSuck.  ** TRAP 5 ** resolved via
//                    aHotPlateSubstrate.h:365, the copy 177 TUs use and the copy
//                    the port ainarm2.cpp ALREADY includes and reads
//                    (.iPickRow/.iPickCol/.Item/.SetItemData at port ainarm2.cpp
//                    :69-99).  NOT mykitsuck.h:274, whose layout differs; the
//                    object itself is defined in aHotPlateSubstrate.cpp.  Only
//                    .iMaxRow/.iMaxCol are read here.
//                  ArmSpeed_File[] cprod.h:2876, .dDieCleanHeight cprod.h:2860.
//                  Prod.iInArm_RotateX/Y cprod.h:802/:803 (int).
//                  InArmOffSet[] cprod.h:233, GetX()/GetY() cprod.h:197/:198.
//                  ZAxisDown aHotPlateSubstrate.h:738; InOfsRotate_In MachineType.h;
//                    CUSTOMER_CODE / CC_TSMC_TAINAN MachineType.h.
//                  InArmContinuousMove_9045 Motor/mymotor.h:396.
//
//  ** NULL-DATA HAZARD, DELIBERATELY NOT GUARDED ** InArmOffSet[] is defined at
//    cprod.cpp:67 as a bare array of ARM_OFFSET* and nothing in this tree ever
//    allocates its elements (golden allocates them at startup; offline they are
//    NULL -- open task #10).  So `InArmOffSet[InOfsRotate_In]->GetX()` here, and
//    the same deref inside GetInArmPitchX_9045/GetInArmPitchY_9045 via
//    ainarm9045.cpp:190-191, WILL segfault if this function is ever called before
//    task #10 lands.  No `if(ptr)` guard is added, for the reason the tree already
//    wrote down at AutoClean/AutoClean.cpp:4505-4507 and repeated for GATE
//    (W7a-I4)/(W7d-I1): this is a POSITION calculation, and silently dropping an
//    offset is a worse failure mode than crashing.  TRAP 4 is NOT triggered --
//    this file adds no file-scope object, so nothing runs before main().
//  SAFE TO LAND TODAY: this symbol has ZERO callers in the port (verified by grep,
//    20260811 06:35 local) and no prior stub, so nothing can reach the deref.  It
//    must NOT be wired to a caller before task #10.
//
//  INTEGER SEMANTICS: `iZPos[i][j]=-(ArmSpeed_File[InArm].dDieCleanHeight*100)`
//    keeps golden's double multiply then narrows on store, and the TSMC_TAINAN arm
//    keeps golden's sign flip plus its `0+(...)` on the other arm.  Copied exactly,
//    including the redundant `0+`.
//==============================================================================
bool MoveInArm2XYToClean()
{
    int iXVariable[X_PITCH_COUNT];
    int iYVariable =0;
    int iMovePitchX=12000;
    int iMovePitchY=6000;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
        if(fNote->bMyServoOffInArm)
        {
            return false;
        }
    }

    for(int i=0; i<X_PITCH_COUNT; i++)
        iXVariable[i]=GetInArmPitchX_9045(iMovePitchX, i, InOfsRotate_In);
    iYVariable=GetInArmPitchY_9045(iMovePitchY, InOfsRotate_In);

    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            bZFlag[i][j]=true;
            if(CUSTOMER_CODE==CC_TSMC_TAINAN)
                iZPos[i][j]=-(ArmSpeed_File[InArm].dDieCleanHeight*100);
            else
                iZPos[i][j]=0+(ArmSpeed_File[InArm].dDieCleanHeight*100);
        }
    }

    if(InArmContinuousMove_9045(Prod.iInArm_RotateX+InArmOffSet[InOfsRotate_In]->GetX(), Prod.iInArm_RotateY+InArmOffSet[InOfsRotate_In]->GetY(), iXVariable, iYVariable, bZFlag, iZPos, ZAxisDown))
    {
        return true;
    }
    return false;
}

//==============================================================================
//  InitInDieCleanTask()  --  golden ainarm2.cpp:3794-3797
//
//  ROLE          Reset the in-arm die-clean cursor to 1.
//  WAVE SCOPE    InitInDieCleanTask()       ACTIVE   golden :3794-3797
//  GATE REGISTER (none)
//                  InDieCleanTask -- golden ainarm2.cpp:3791 `int InDieCleanTask=1;`
//                    is one of the 37 file-scope globals the INTEGRATOR adds; it is
//                    NOT emitted here, only used.  Verified absent today: the only
//                    tree hit for the string is ainarm9045.cpp:1169, which is the
//                    substring inside `InitInDieCleanTask();` -- checked, not
//                    assumed (grep -rn "InDieCleanTask", 20260811 06:36 local).
//                  DoInDieCleanDelay (golden :3792) is NOT touched by this range.
//                    Note for the integrator: golden types it HTimer, and per the
//                    wave brief it must be declared TQPF_Timer instead, because the
//                    port's only HTimer (atester_shims.h:463) has Off() hard-coded
//                    true and would zero every die-clean dwell.
//
//  NET-NEW, AND CURRENTLY UNREACHABLE: no prior stub anywhere.  Its single call
//    site, ainarm9045.cpp:1169, sits INSIDE `#if 0 // TODO(W7)` (that gate opens at
//    ainarm9045.cpp:1166), so this body has zero live callers and cannot change
//    offline behaviour.  It is groundwork for the DoInDieClean SM, not a live fix;
//    do not report it as an activated feature.
//==============================================================================
void InitInDieCleanTask()                                                       //wei 20170418
{
    InDieCleanTask=1;
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 3799..3869
// -----------------------------------------------------------------------------
//  ROLE        : the InArm Die-Clean state machine: Z to plate-safe, X/Y to the clean position (with an optional AccessLevel>=1 manual-key pause/offset detour), settle, open the clean blow plus suck for the configured delay, Z back to safe, close them and report done.
//  WAVE SCOPE  : DoInDieClean -- ACTIVE with ONE gated condition, golden ainarm2.cpp:3799..3869.
//  COLLISION    : none -- no port body today.  Its Init sibling InitInDieCleanTask
//              : (golden :3794) is NOT part of this part file.
//  GLOBALS      : InDieCleanTask (golden :3791) and DoInDieCleanDelay (golden :3792)
//              : are integrator-owned file-scope globals and are NOT emitted here.
//              : DoInDieCleanDelay is golden-typed HTimer; the integrator declares it
//              : TQPF_Timer instead, and that is the right call -- the port's only
//              : HTimer (atester_shims.h:463) has `Off()` hard-coded to `return true`,
//              : which would silently zero BOTH dwells in this SM: the 500 ms settle at
//              : case 200 and the whole dDieCleanDelay blow at case 400.  TQPF_Timer
//              : (myTimer.h:10) really does provide `void SetMSAndOn(DWORD)` at :39 and
//              : `bool Off()` at :30, which is exactly what this body calls.
//  INCLUDES     : this body needs, beyond what port ainarm2.cpp already includes
//              : (cmydef.h / MachineType.h / cprod.h / ainarm9045.h /
//              : aHotPlateSubstrate.h):
//              : mysensor.h (Sen[] :48, IsOn :42), myswitch.h (SW[] :43, On :32, Off :33), forms/fMain.h (fMain->Pause :142), myTimer.h (TQPF_Timer) -- MoveInArmZToPlateSafe is aHotPlateSubstrate.h:901, already included
//  GATE REGISTER: ONE gate, W7E-K8-C, on golden :3813 -- full WHAT/WHY/DELTA plus the
//              : exact absence command and the time it was run are inline at the gate.
//  ACTIVE DEPS  : AccessLevel cmydef.h:3503, SnRKManualTStart cmydef.h:702,
//              : bEnterOffset cmydef.h:2734, SwDieClean cmydef.h:1875 (=187,
//              : cmydef.cpp:2089), SwDieCleanSuck cmydef.h:1876 (=297,
//              : cmydef.cpp:2219), ArmSpeed_File cprod.h:2876 whose dDieCleanDelay is
//              : cprod.h:2859 (double), InArm MachineType.h:682 (eSpeedPart, =0).
//  SIGNATURE    : golden's `fMain->Pause("DoInDieClean")` PASSES an argument and the
//              : port's TfMain::Pause takes `AnsiString Func` (forms/fMain.h:142), so
//              : the call is transcribed VERBATIM.  Do NOT swap the literal for
//              : __FUNC__ the way aoutarm9045S_1x4_4.cpp had to -- that substitution
//              : was for golden call sites that take NO argument.  Offline Pause()
//              : returns false and never pauses, but it does record
//              : W906_PauseCallCount / W906_PauseLastFunc, so the literal
//              : "DoInDieClean" is observable from tests.
//  DOUBLE->DWORD: golden :3846 `SetMSAndOn(ArmSpeed_File[InArm].dDieCleanDelay*1000)`
//              : converts a double to DWORD, truncating; negative or huge values wrap.
//              : Verbatim -- not clamped, not rounded.
// =============================================================================
bool DoInDieClean()                                                             //wei 20170418
{
    int &Task=InDieCleanTask;
    bool Result=false;

    switch(Task)
    {
        case 1:                                                                 // z safe
            if(MoveInArmZToPlateSafe(199))
            {
                Task=100;
            }
            break;
        case 100:                                                               //xy move clean pos
//------------------------------------------------------------------------------
//  GATE W7E-K8-C -- golden ainarm2.cpp:3813
//  WHAT  : case 100 waits for MoveInArm2XYToClean() -- move the in-arm X/Y onto the
//  WHAT  : taught Die-Clean position -- before it starts the blow/suck sequence.
//  ⚠ AI(W906-NL-ABSENCE) 20260916: 下面這個前提**已經過期**。
//  WHY   : `MoveInArm2XYToClean()` 現在定義在 **ainarm2.cpp:2861 —— 同一個檔案，
//  WHY   : 本閘上方 164 行**，本檔自己的 WAVE SCOPE 橫幅 :2823 也標著 ACTIVE。
//  WHY   : 連結器層級證據：nm --defined-only 掃 build/*.a 找到
//  WHY   : `__Z19MoveInArm2XYToCleanv`。golden 不靠標頭（:3746 先於 :3813），
//  WHY   : 同檔也不需要宣告，所以「NO port declaration」那半也不再構成阻礙。
//  WHY   : ⛔ **閘仍然不開，而且夜間迴圈不會開它**：這是 Die-Clean 的 XY 移動，
//  WHY   : 屬運動/安全。下面 DELTA 記的 fail-loud 選擇與「唯一 caller
//  WHY   : ainarm9045.cpp:1194 也在 #if 0 裡、要解就兩個一起解」的耦合都仍然成立，
//  WHY   : 那才是現在該拿去問使用者的東西。原文保留為沿革：
//  WHY   : MoveInArm2XYToClean has NO port body and NO port declaration.  Command,
//  WHY   : run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0 at 2026-08-11 06:33:21 local:
//  WHY   :     rg -n --no-messages -g '*.cpp' -g '*.h' 'MoveInArm2XYToClean' .
//  WHY   : -> zero hits, exit status 1.
//  WHY   : TRAP 2 -- THIS ABSENCE CAN EXPIRE INSIDE THIS SAME WAVE: golden's body is
//  WHY   : ainarm2.cpp:3746, i.e. THIS file, 53 lines above this call and just outside
//  WHY   : this agent's assigned ranges.  Re-run the command above at integration.
//  WHY   : TRAP 3 -- if it HAS landed, retiring is correct here and needs no prototype:
//  WHY   : golden has none either (ainarm2.h does not declare it -- golden relies on
//  WHY   : :3746 preceding :3813 inside one file), and a part file named 03746_* stitches
//  WHY   : ahead of this one.  If it has NOT landed, the gate stays.
//  WHY   : RE-CHECKED, AND IT HAS NOT LANDED.  At 2026-08-11T06:43:57+0800 I re-ran
//  WHY   : the command above (still zero hits, exit 1) AND listed the whole part
//  WHY   : directory: 50 part files, none named 03746_*, and
//  WHY   :     rg -n 'MoveInArm2XYToClean' D:/HT9045/HT9011UC_Cpp_V3.33.906.0/_w7e_parts
//  WHY   : matched only this part's own banner and gated line.  Two of my three
//  WHY   : original absence claims DID expire in that window (ChangeHotPlateData in
//  WHY   : part 00654 and the caller of PreciserPitchCalculate in part 03423) and
//  WHY   : both were fixed before shipping -- this one genuinely did not.
//  WHY   : Re-run it once more at integration anyway.
//  DELTA : `false` means the XY move NEVER reports complete, so case 100 never advances
//  DELTA : and DoInDieClean never returns true.  THIS IS THE DELIBERATE FAIL-LOUD
//  DELTA : CHOICE, and the reasoning is not re-derived here: it is exactly the argument
//  DELTA : csystem.cpp:20296-20320 GATE G01 already wrote for the ion-fan twin --
//  DELTA : defaulting the move to 'reached' would run the actuator sequence with the arm
//  DELTA : parked at WHATEVER X/Y it happened to be at, i.e. open SwDieClean and
//  DELTA : SwDieCleanSuck over an untaught position and then report a clean that cleaned
//  DELTA : nothing.  A visible stall beats a silent wrong-position air blast.
//  DELTA : Not reachable today either way: the only caller, ainarm9045.cpp:1194, is
//  DELTA : itself inside the `#if 0 // TODO(W7)` block opened at ainarm9045.cpp:1192.
//  DELTA : If that caller is ever ungated while this gate is still in place, DoInArm's
//  DELTA : case 10000 will hang -- ungate the two together, or not at all.
#if 0 // GATE W7E-K8-C -- golden ainarm2.cpp:3813 (text below is golden VERBATIM)
            if(MoveInArm2XYToClean())
#else
            if(false)                                                           // GATE W7E-K8-C: Die-Clean XY move never reports complete
#endif
            {
                if(AccessLevel>=1 && Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoInDieClean");
                    Task=120;
                    break;
                }
                DoInDieCleanDelay.SetMSAndOn(500);
                Task=200;
            }
            break;
        case 120:
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=1;
            }
            else
            {
                Task=100;
            }
            break;
        case 200:                                                               //到位延遲
            if(DoInDieCleanDelay.Off())
            {
                Task=300;
            }
            break;
        case 300:                                                               //開吹氣
            SW[SwDieClean].On();                                                //wei 20170525
            SW[SwDieCleanSuck].On();                                            //wei 20210326
            DoInDieCleanDelay.SetMSAndOn(ArmSpeed_File[InArm].dDieCleanDelay*1000);
            Task=400;
            break;
        case 400:                                                               //吹氣延遲
            if(DoInDieCleanDelay.Off())
            {
                Task=500;
            }
            break;
        case 500:
            if(MoveInArmZToPlateSafe(599))
            {
                Task=600;
            }
            break;
        case 600:                                                               //關吹氣
            SW[SwDieClean].Off();                                               //wei 20170525
            SW[SwDieCleanSuck].Off();                                           //wei 20210326
            Result=true;
            break;
    }

    return Result;
}
//==============================================================================
//  CheckInArmFinishAllPickerAction()  --  golden ainarm2.cpp:133-155
//  Wave PT-W7e, part k1-ainarm2.  Translator: AI(W7E-k1-ainarm2) 20260811
//
//  ROLE  "May another mechanism move now?"  The tray arm / shuttle ask this
//        before they commit to a move: it returns false while the in-arm still
//        owns an action (loader pick, hot-plate place/pick, shuttle blow-off with
//        the InSht latch idle, a half-finished 2-step shuttle place, or a wait on
//        the Preciser / Rotator), and true once the arm is parked -- either
//        because both XY motors are already unable to move and the arm sits at
//        its wait position, or because no action flag is raised.
//
//  WAVE SCOPE  CheckInArmFinishAllPickerAction  ACTIVE  golden :133-155.
//
//  GATE REGISTER  none -- this part contains no #if 0.
//
//  *** STUB RETIREMENT (per the task's note 1) -- integrator action required ***
//  acatchtray_shims.cpp:112 currently holds
//        bool CheckInArmFinishAllPickerAction() { return true; }
//  (declared acatchtray_shims.h:363, "offline: finished (true)").  That stub is
//  the OPPOSITE default from this real body in every busy state, and its single
//  live consumer is acatchtray.cpp:2798.  Delete the stub when this part lands or
//  the link fails with a duplicate definition.
//
//  DEPENDENCY NOTE (not a gate) -- AND A LIVE EXAMPLE OF TRAP 2, RECORDED HONESTLY:
//  IsMoveInArm2XYToWait() is golden ainarm2.h:138 / body golden ainarm2.cpp:1295.
//    * FIRST measurement, 2026-08-11T06:19:13+0800:
//        rg -n -w IsMoveInArm2XYToWait --glob '*.cpp' --glob '*.h' D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//      -> the only DEFINITION was acatchtray_shims.cpp:113 `return true;', i.e. my
//      first arm ("both XY motors cannot move" -> at wait) would read true whenever
//      both fCanMove are false.
//    * RE-RUN at 2026-08-11T06:48:10+0800: a SIBLING part _w7e_parts/
//      01295_IsMoveInArm2XYToWait.txt has since landed, so that premise is DEAD --
//      after the stitch the real golden body serves this call and the shim at
//      acatchtray_shims.cpp:113 must be deleted with it.
//  Either way the call itself was never gated (it is faithful; only the callee was
//  provisional), so nothing in this part changes -- but the note would have been a
//  false statement by integration time if left as first written.
//  ORDER MATTERS for the forward declaration below: part 00133 is stitched BEFORE
//  part 01295, so this TU still needs a declaration ahead of the call.
//
//  bDestoryOnSht is ALREADY defined in this file (port ainarm2.cpp:46, behind
//  HT9045_bDestoryOnSht_DEFINED) -- this part only reads it.
//
//  Big5: the FOUR Chinese trailing comments in this range -- golden :133, :135,
//  :147, :150 (measured: those are exactly the non-ASCII lines of :133-155) -- are
//  transcribed character for character from cp950.  ZERO U+FFFD.
//
//  INCLUDES/DECLS this part needs (measured with a scratchpad probe TU, g++ 6.3
//  -std=c++1z -fsyntax-only, 2026-08-11T06:50+0800 -- everything else in this part
//  was already reachable from the port ainarm2.cpp include block):
//    * IsMoveInArm2XYToWait -- declared NOWHERE this TU can see.  Golden's home is
//      ainarm2.h:138 (`extern bool IsMoveInArm2XYToWait();'), i.e. THIS file's own
//      header, which the port does not have; the port's only declaration is
//      acatchtray_shims.h:364.  A guarded forward declaration with golden's exact
//      signature is used instead of pulling in that 438-line shims header (it also
//      drags SECSGEM/SecsEventType.h + SecsEventReport.h).  It is a DECLARATION, so
//      it cannot become TRAP 1 shape (d) and it stays compatible if a sibling part
//      lands golden :1295 in this same file.
//==============================================================================
#ifndef HT9045_IsMoveInArm2XYToWait_DECLARED
#define HT9045_IsMoveInArm2XYToWait_DECLARED
bool IsMoveInArm2XYToWait();                                                    // golden ainarm2.h:138 (body golden ainarm2.cpp:1295)
#endif
//==============================================================================
bool CheckInArmFinishAllPickerAction()                                          //Steven 20171226 (Wei) : 修改in arm讓開的flag
{
    if(MOT[MInArmX].fCanMove==false &&                                          //jou 20240910 : 修正tray arm 讓位 hang up
       MOT[MInArmY].fCanMove==false)
    {
        if(IsMoveInArm2XYToWait())
            return true;
    }

    if(bPickFromLoader || bPlaceToHotplate ||                                   //JerryYang 20241118 : fix auto clean hang up
        bPickFromHotplate ||
        (bDestoryOnSht &&
        (In_Shuttle_Auto_Latch!=eInSHAutoLtc ||
        (In_Shuttle_Auto_Latch==eInSHAutoLtc &&
        (bInSh1DoLtc==false && bInSh2DoLtc==false)))) ||                        //KenHsieh 20251119 : Add InShtLtc作動旗標，避免因放貨候機料未轉移，導致Hangup
        bPlaceToShuttle2Step ||
        bWaitPreciserFinish ||
        bWaitRotateFinish)                                                      //Ifor 20180704 新增 Preciser
    {
        return false;
    }
    return true;
}

//==============================================================================
//  SetInArmUseSuckToHasTrySuckIC(int iSht, int iKit)      golden ainarm2.cpp:261-318
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : re-classifies the in-arm pick grid`s NULL_IC nozzles to
//               HAS_TRY_SUCK_IC -- the "this nozzle is committed to a HotPlate
//               TRY-pick" marker -- honouring the 1-picker special case, the
//               TSMC-Tainan site-map filter, and the two 2x8 close-site sub-modes.
//               Pure grid logic over the Sim-HAL InArmSuck object; no hardware.
//  WAVE SCOPE : SetInArmUseSuckToHasTrySuckIC()  golden :261-318  -- ACTIVE
//  GATE REGISTER: none.  Every symbol has a port home reachable from this TU`s
//               existing include head (cmydef.h: USE_PICKER_COUNT :2915 /
//               HAS_TRY_SUCK_IC :167 / HAS_NULL_IC / NULL_IC / CUSTOMER_CODE :3181
//               / iInArmType :4594;  MachineType.h: ep1Picker :1313 /
//               CC_TSMC_TAINAN :186 / e9045_*_Hot;  cprod.h: Prod.iSiteMap :822;
//               aHotPlateSubstrate.h: InArmSuck :624 + iCloseSiteModeFor2x8 :868 +
//               e2x8Run2x2_13/_14).
//  TRAP 5     : InArmSuck is the TMyKitSuck of aHotPlateSubstrate.h:365 (object
//               defined aHotPlateSubstrate.cpp:80), NOT mykitsuck.h:274 /
//               mykitsuck.cpp:205.  This file`s existing include head already
//               commits to aHotPlateSubstrate.h and deliberately does NOT pull
//               MyKitSuck.h (golden ainarm2.cpp:17 does) -- the two classes have
//               different layouts, so mixing them links clean and then reads every
//               field at the wrong offset.
//  RETIRES    : the offline stub at aHotPlateSubstrate.cpp:1590-1596, which set only
//               nozzle (0,0).  REAL BEHAVIOUR CHANGE for every non-1-picker config:
//               the 2x4 sub-grid now really gets marked.
//  GOLDEN QUIRKS PRESERVED (not fixed here):
//    * the else arm walks a HARD-CODED 2x4 (i<2, j<4) instead of
//      InArmSuck.iPickRow/iPickCol the way its sibling SetInArmUseSuckToHasNullIC
//      (golden :201) does -- so on a 2x8 grid only columns 0..3 are ever marked.
//    * JerryYang 20251013 commented OUT the Prod.fInArmSuck4x8[iSht][i][j2]==true
//      guard (golden :284-285), so for every CUSTOMER_CODE except CC_TSMC_TAINAN
//      even a NOT-in-use nozzle gets HAS_TRY_SUCK_IC.
//    * iKit32 is computed from iKit but consumed ONLY inside the CC_TSMC_TAINAN
//      branch; iSht likewise.
//==============================================================================
void SetInArmUseSuckToHasTrySuckIC(int iSht, int iKit)
{
    int iKit32=(iKit==1)?4:0;

    if(iInArmType==e9045_1x4_8_Hot ||                                           //Steven 20240119 : fixed for 2x4_16
       iInArmType==e9045_1x2_4_Hot ||
       iInArmType==e9045_2x2_8_Hot)
    {
//        iSht=0;
        iKit32=0;
    }

    if(USE_PICKER_COUNT==ep1Picker)
    {
        InArmSuck.SetItemData(0, 0, HAS_TRY_SUCK_IC);
    }
    else
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                int j2=j+iKit32;
    //            if(Prod.fInArmSuck4x8[iSht][i][j2]==true &&
    //               InArmSuck.Item[i][j]==NULL_IC)
                if(InArmSuck.Item[i][j]==NULL_IC)                               //JerryYang 20251013 : fix hot plate check 關Site位置錯誤
                {
                    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                           //wei 20160624 Hotplate clean out
                    {
                        if(Prod.iSiteMap[iSht][i][j2]>0)
                            InArmSuck.SetItemData(i, j, HAS_TRY_SUCK_IC);
                        else
                            InArmSuck.SetItemData(i, j, HAS_NULL_IC);
                    }
                    else
                    {
                        InArmSuck.SetItemData(i, j, HAS_TRY_SUCK_IC);
                    }
                }
            }
        }
    }

    if(iCloseSiteModeFor2x8==e2x8Run2x2_13)
    {
        InArmSuck.SetItemData(0, 1, NULL_IC);
        InArmSuck.SetItemData(0, 3, NULL_IC);
        InArmSuck.SetItemData(1, 1, NULL_IC);
        InArmSuck.SetItemData(1, 3, NULL_IC);
    }
    else if(iCloseSiteModeFor2x8==e2x8Run2x2_14)
    {
        InArmSuck.SetItemData(0, 1, NULL_IC);
        InArmSuck.SetItemData(0, 2, NULL_IC);
        InArmSuck.SetItemData(1, 1, NULL_IC);
        InArmSuck.SetItemData(1, 2, NULL_IC);
    }
}

//==============================================================================
//== ROLE: SetInArm_Unuse_SuckToNullICForHP -- immediately before an in-arm
//==       HotPlate pick, demote back to NULL_IC every nozzle whose SITE IS
//==       TURNED OFF for the 32-site shuttle/kit currently being picked
//==       (Prod.fInArmSuck4x8[iShtHP][row][col+iKit32] == false), then apply the
//==       two 2x8-close-site sub-mode masks.  Pure Item-grid logic over the
//==       Sim-HAL InArmSuck object -- NO hardware is touched.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   SetInArm_Unuse_SuckToNullICForHP  golden ainarm2.cpp:320-357  -- ACTIVE
//==     (golden lines emitted VERBATIM, cp950 -> UTF-8; ZERO gates)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   Provenance of everything it touches, so the "no gate" claim is checkable:
//==     * InArmSuck -- TRAP 5 ANSWER: I rely on the aHotPlateSubstrate.h:365
//==       TMyKitSuck (declared aHotPlateSubstrate.h:624, OBJECT DEFINED IN
//==       aHotPlateSubstrate.cpp:80).  That is the layout 177 TUs use and the one
//==       the port ainarm2.cpp include block already pulls.  It is NOT the second
//==       TMyKitSuck at mykitsuck.h:274 whose object is mykitsuck.cpp:205 -- that
//==       header has a DIFFERENT field layout and mykitsuck.cpp is DELIBERATELY
//==       NOT REGISTERED (CMakeLists.txt:2099).  Fields used here:
//==       iWhichShtPickFor32 (aHotPlateSubstrate.h:410), iWhichKitPickFor32
//==       (:411), SetItemData.
//==     * Prod.fInArmSuck4x8[2][2][8] -- cprod.h:820, object cprod.h:1138.
//==       Index range provably safe: j2 = j + iKit32 with j in 0..3 and iKit32
//==       either 0 or 4, so j2 stays in 0..7.
//==     * iInArmType / e9045_1x4_8_Hot / e9045_1x2_4_Hot / e9045_2x2_8_Hot /
//==       iCloseSiteModeFor2x8 / e2x8Run2x2_13 / e2x8Run2x2_14 / NULL_IC -- all
//==       already resolved in THIS TU by SetInArmUseSuckToHasNullIC
//==       (port ainarm2.cpp:50-108), so nothing new is demanded.
//==
//== INTEGRATION NOTE -- TRAP 1 shape (b), MUST ACT:
//==   port aHotPlateSubstrate.cpp:1519 currently holds an EMPTY no-op body for
//==   SetInArm_Unuse_SuckToNullICForHP.  That stub already satisfies every call
//==   site, which is exactly why "build green" proves nothing here.  DELETE it or
//==   this part is a duplicate definition.  Behaviour genuinely changes at
//==   integration: today NO nozzle is demoted, so a disabled site is still
//==   committed to for the HP pick; afterwards disabled-site nozzles return to
//==   NULL_IC and the two 2x8 close-site masks start applying.
//==   Enumerate the call sites with
//==     grep -rn --include=*.cpp "SetInArm_Unuse_SuckToNullICForHP" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==   -- ran 2026-08-11 06:20 +0800, 20 hits across 15 in-arm variant TUs.  NOT
//==   all of them are live: several variant TUs carry whole-file #if 0 regions
//==   (ainarm9045_2x8_32.cpp, for one, is #if 0 from its line 91 to EOF), so the
//==   integrator must re-check reachability instead of trusting the hit count.
//==   The declaration already exists NON-static at aHotPlateSubstrate.h:1067, so
//==   this definition is NOT static -- TRAP 1 shape (d) avoided.
//==
//== TRAP 4 ANSWER: this part adds NO file-scope object of any kind, so no static
//== initialiser of mine can run before main() and touch a NULL global.
//==
//== Big5 -> UTF-8: golden lines are reproduced CHARACTER FOR CHARACTER (read with
//== cp950, written UTF-8 bare-LF).  Zero U+FFFD.
//==============================================================================
//== EXTRA INCLUDES for the k2-ainarm2 parts (golden ainarm2.cpp gets these via
//== BCB6 vcl.h plus its own header chain).  Measured with g++ -fsyntax-only
//== against the port include path on 2026-08-11 06:45 +0800: the five includes
//== already at the top of port ainarm2.cpp resolve everything EXCEPT the six
//== below.  All six headers are self-guarded, so re-emitting them here is
//== idempotent and the main loop may instead hoist them into the file-top block.
//==   LastSet.h                  -- LastSet (fHotPlate*Scale*, fLoaderTray*Scale*, iTemperature)
//==   myswitch.h                 -- SW[] (DoVibrateOutShuttle)
//==   cMyDB.h                    -- RecordProcess / NewRecordProcess log sinks
//==   csystem.h                  -- InitDoArmZHome (SetInArmHome)
//==   SECSGEM/SecsEventType.h    -- SECS_EVENT.SiteMappingStart / SiteMappingEnd
//==   SECSGEM/SecsEventReport.h  -- EventReport(unsigned)
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "LastSet.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "myswitch.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "cMyDB.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "SECSGEM/SecsEventType.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "SECSGEM/SecsEventReport.h"
//==============================================================================
void SetInArm_Unuse_SuckToNullICForHP()
{
    int iShtHP=InArmSuck.iWhichShtPickFor32;
    int iKit32=(InArmSuck.iWhichKitPickFor32==1)?4:0;

    if(iInArmType==e9045_1x4_8_Hot ||                                           //Steven 20240119 : fixed for 2x4_16
       iInArmType==e9045_1x2_4_Hot ||
       iInArmType==e9045_2x2_8_Hot)
    {
//        iSht=0;
        iKit32=0;
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            int j2=j+iKit32;
            if(Prod.fInArmSuck4x8[iShtHP][i][j2]==false)
                InArmSuck.SetItemData(i, j, NULL_IC);
        }
    }

    if(iCloseSiteModeFor2x8==e2x8Run2x2_13)
    {
        InArmSuck.SetItemData(0, 1, NULL_IC);
        InArmSuck.SetItemData(0, 3, NULL_IC);
        InArmSuck.SetItemData(1, 1, NULL_IC);
        InArmSuck.SetItemData(1, 3, NULL_IC);
    }
    else if(iCloseSiteModeFor2x8==e2x8Run2x2_14)
    {
        InArmSuck.SetItemData(0, 1, NULL_IC);
        InArmSuck.SetItemData(0, 2, NULL_IC);
        InArmSuck.SetItemData(1, 1, NULL_IC);
        InArmSuck.SetItemData(1, 2, NULL_IC);
    }
}

//==============================================================================
//  CloseSiteState(bool bPlace)                            golden ainarm2.cpp:387-531
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : collapses the whole close-site picture into ONE tri-state the
//               in-arm / index SMs branch on -- 0 = full site (both nozzle rows in
//               use, or "close nothing"), 1 = only row1 usable, 2 = only row2.  It
//               fuses four independent sources: the runtime "close site any time"
//               flag, the Auto-Clean selected-arm override, the Auto-Site-Mapping /
//               2DID-order-check socket map, and a set of test-mode / HotPlate-
//               geometry escape hatches that force 0 because closing a row in those
//               layouts hangs the machine.
//  WAVE SCOPE : CloseSiteState()  golden :387-531  -- ACTIVE (no gate)
//  GATE REGISTER: none.
//  RETIRES    : the offline stub `int CloseSiteState(bool){ return 0; }` at
//               aHotPlateSubstrate.cpp:1054.  REAL BEHAVIOUR CHANGE: the :484-499
//               socket-map arm and the :523-528 ladder can now return 1 or 2 where
//               the stub always said 0.
//  TRAP 1     : callers reach this through the aHotPlateSubstrate.h declaration
//               (see that file :38); DoInArm_SuckerMap() (golden :969-970) is the
//               live caller, and it IS landing in this same wave -- sibling part
//               _w7e_parts/00965_DoInArm_SuckerMap.txt, observed 2026-08-11
//               06:45 +0800 -- so this body has a real in-wave consumer, not just a
//               header declaration.  This definition is NON-static -- it must be, or the
//               header declaration and this body become two different symbols
//               (shape (d)).  The main loop must DELETE the :1054 stub in the SAME
//               commit; otherwise the archive keeps resolving to the stub and
//               "build green" proves nothing (shape (b)).
//  TRAP 5     : InArmSuck / iShtCol come from aHotPlateSubstrate.h:365 (object
//               aHotPlateSubstrate.cpp:80), not mykitsuck.h:274.  HotPlateForm is
//               the TRAY_TYPE_PARA of cprod.h:1361 whose member is XDivision
//               (cprod.h:1262) -- NOT the iXDivision of cmydef.h:3935, a different
//               struct with a similar name.
//  GOLDEN QUIRKS PRESERVED (not fixed here):
//    * both `else` arms of the Auto-Clean split (golden :445-462 and :464-481)
//      handle ONLY IsNNMode()==NN_2Row.  On a non-NN machine neither bRow flag is
//      ever set from Prod.fInArmSuck4x8, so unless :484-499 fires the ladder falls
//      off the end and returns the initialiser 0.
//    * golden :509 and :522 assign iState=0, already its initial value.
//    * golden :392-393 read "bCanAutoCloseSite && bPlace==false" -> claims BOTH rows
//      are in use, i.e. the runtime-close path deliberately reports full site for
//      the PICK side.
//
//  HOIST ME to ainarm2.cpp`s include head at integration:
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "atester_shims.h"
                                 // also declares IsNNMode() :277 (real body cinitial.cpp:7253; the
                                 // atester_shims.cpp copy at :330 is inside "#if 0 PT-W7a RETIRED")
//  (CosFunction comes free: cmydef.h -> cprod.h:7 -> CosFunction.h:488.
//   NN_1Row/NN_2Row are MachineType.h:1416-1417, already in the head.)
//==============================================================================
int CloseSiteState(bool bPlace)
{
    int iState=0;
    bool bRow1HasUse=false, bRow2HasUse=false;

    if(bCanAutoCloseSite &&                                                     //Steven 20200420 : 整合Auto Site Off
       bPlace==false)                                                           //Steven 20150924 : 隨時開關Site功能
    {
        bRow1HasUse=true;
        bRow2HasUse=true;
    }
    else
    {
        if(bRunAutoClean && TestIF_File.iAutoClean_SelectArm<2)                 //JerryYang 20160219 Fix 8site 整排關site時hang up
        {
            if(CosFunction.bUseAutoCleanCloseSiteAlsoDo==true)                  //Ifor 20181222 add 新增Auto Clean Close Site 一樣執行
            {
                if(IsNNMode()==NN_2Row)
                {
                    for(int j=0; j<InArmSuck.iShtCol; j++)
                    {
                        if(TestIF.iSiteMap[0][j] || TestIF.iSiteMap[2][j])
                        {
                            bRow1HasUse=true;
                        }

                        if(TestIF.iSiteMap[1][j] || TestIF.iSiteMap[3][j])
                        {
                            bRow2HasUse=true;
                        }
                    }
                }
                else if(IsNNMode()==NN_1Row)
                {
                    for(int j=0; j<InArmSuck.iShtCol; j++)
                    {
                        if(TestIF.iSiteMap[0][j] || TestIF.iSiteMap[1][j])
                        {
                            bRow1HasUse=true;
                        }
                    }
                }
                else
                {
                    for(int j=0; j<InArmSuck.iShtCol; j++)
                    {
                        if(TestIF.iSiteMap[0][j])
                        {
                            bRow1HasUse=true;
                        }

                        if(TestIF.iSiteMap[1][j])
                        {
                            bRow2HasUse=true;
                        }
                    }
                }
            }
            else
            {
                if(IsNNMode()==NN_2Row)
                {
                    for(int j=0; j<InArmSuck.iShtCol; j++)
                    {
                        if(Prod.fInArmSuck4x8[0][0][j] || Prod.fInArmSuck4x8[1][0][j])
                        {
                            bRow1HasUse=true;
                        }

                        if(Prod.fInArmSuck4x8[0][1][j] || Prod.fInArmSuck4x8[1][1][j])
                        {
                            bRow2HasUse=true;
                        }
                    }
                }
            }
        }
        else
        {
            if(IsNNMode()==NN_2Row)
            {
                for(int j=0; j<InArmSuck.iShtCol; j++)
                {
                    if(Prod.fInArmSuck4x8[0][0][j] || Prod.fInArmSuck4x8[1][0][j])
                    {
                        bRow1HasUse=true;
                    }

                    if(Prod.fInArmSuck4x8[0][1][j] || Prod.fInArmSuck4x8[1][1][j])
                    {
                        bRow2HasUse=true;
                    }
                }
            }
        }
    }

    if(bRunAutoSiteMapping==true ||                                             //Ifor 20180518 : add 簡化Site Mapping 旗標
       fContact->IsRun2DCheck()==true)                                          //JerryYang 20250220 : 2DID硬體順序檢查功能
    {                                                                           //Ifor 20180515 : add 避免Auto Site Mapping 模式錯誤導致Hang up
        for(int i=0; i<InArmSuck.iShtCol; i++)
        {
            if(LastSet.bUseTestSocket[0][0][i] || LastSet.bUseTestSocket[1][0][i])                                      //ChungHung 20130910 alter for SCK can close site by Index
            {
                bRow1HasUse=true;
            }

            if(LastSet.bUseTestSocket[0][1][i] || LastSet.bUseTestSocket[1][1][i])                                      //ChungHung 20130910 alter for SCK can close site by Index
            {
                bRow2HasUse=true;
            }
        }
    }

    if(i8PickerHPMode==iHPWideHP &&                                             //JerryYang 20161007 簡化Hotplate判斷式
       bRunAutoClean==false)                                                    //jou 2012-08-10 會造成Auto clean錯亂  //Steven 20151117 : 2x2 8Picker at Hot mode
    {
        if(HotPlateForm.XDivision==6 &&
           (iInArmType==e9045_2x4_8  ||
            iCloseSiteModeFor2x8>e2x8OneByOne  ||
            iCloseSiteModeFor2x6>e2x6OneByOne))
        {
            iState=0;
            return iState;
        }
    }

    if(bRunAutoClean==false &&                                                  //jou 2013-01-22 Auto Clean 16 site 不能直接return 0 , Hang up
       fContact->IsRun2DCheck()==false &&                                       //JerryYang 20250220 : 2DID硬體順序檢查功能                                                 //jou 2013-01-22 Auto Clean 16 site 不能直接return 0 , Hang up
       ((TestIF.iTestMode==_12Site2X6 && iCloseSiteModeFor2x6<=e2x6OneByOne) ||                                         // Lee 20100823 : 2x8關Site會Hang UP
       (TestIF.iTestMode==_16Site2X8 && iCloseSiteModeFor2x8<=e2x8OneByOne) ||
       TestIF.iTestMode==_32Site4X8N  ||                                        //Steven 20140512: For HT-9047
       iInArmType==e9045_2x4_8 ||                                               //Steven 20210420 : 修正12site關site跑2x4 mode, HP=6x11 hang up
       ((TestIF.iTestMode==_8Site2X4 || TestIF.iTestMode==_16Site4X4) &&        //Sam 20190226 : 16Site4X4                                    //Steven 20170321 (wei) : fixed for close row2 hang up
         ArmCanSuck4IC(0)==true)))                                              //DeviceForm_File.XDimension<20 && DeviceForm_File.YDimension<20)) //Sam 20190701 : 修正 Xpitch 60mm, ic 21.5X21.5mm 關 Site hang up  //kevin 20170626 (Steven) shuttle X Pitch 60 IC 28X28 關SITE 上排
        iState=0;
    else if(bRow1HasUse && bRow2HasUse)
        iState=0;                                                               // full site
    else if(bRow1HasUse && bRow2HasUse==false)
        iState=1;                                                               // only row1
    else if(bRow1HasUse==false && bRow2HasUse)
        iState=2;                                                               // only row2

    return iState;
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 533..565
// -----------------------------------------------------------------------------
//  ROLE        : true iff the two HotPlate grids still hold at least iLimit non-NULL cells, i.e. 'clear all the hot IC before you go pick a load IC' is already satisfied.
//  WAVE SCOPE  : CheckClearAllHotICThenPickLoadIC -- ACTIVE, golden ainarm2.cpp:533..565.
//  COLLISION    : *** INTEGRATOR MUST DELETE THE EXISTING PARTIAL/STUB BODY ***
//              : aHotPlateSubstrate.cpp:1533  `bool CheckClearAllHotICThenPickLoadIC() { return false; }`
//              : It is a NON-static definition of the same signature, so leaving
//              : it in place is a `multiple definition` link error, not a silent
//              : shadow.  Its `extern` DECLARATION stays where it is -- only the
//              : body is deleted.
//  INCLUDES     : this body needs, beyond what port ainarm2.cpp already includes
//              : (cmydef.h / MachineType.h / cprod.h / ainarm9045.h /
//              : aHotPlateSubstrate.h):
//              : ainarm_SearchPlacePlate.h (HotPlateYPitchCanPutAll :51, GetHotPlateYHalfPos :49), Motor/mymotor.h (MOT[])
//  GATE REGISTER: none -- every symbol resolves.  HotPlateForm cprod.h:1361
//              : (TRAY_TYPE_PARA .XDivision:1262 .YDivision:1263 .iPlateSelect:1271),
//              : MMPlate1 cmydef.h:2257 (=169 cmydef.cpp:2538), MOT[] Motor/mymotor.h:385,
//              : Tray.Data mytray.h:40, NULL_IC cmydef.h, InArmSuck.iPickRow
//              : aHotPlateSubstrate.h:404, iYHalf cmydef.h:3152.
//  TRAP 5       : InArmSuck is the aHotPlateSubstrate.h:365 TMyKitSuck (declared
//              : aHotPlateSubstrate.h:624, object DEFINED aHotPlateSubstrate.cpp:80) --
//              : the 177-TU layout, NOT the different-layout mykitsuck.h:274 class.
//  GOLDEN QUIRK : `HotPlateForm.iPlateSelect & 2-i` (golden :542) parses as
//              : `iPlateSelect & (2-i)` because binary minus binds tighter than the
//              : bitwise and, so the loop tests bit1 for i==0 and bit0 for i==1.
//              : Golden's own comment on the line says `MMPlate1+0=NO 2 HP`.
//              : Transcribed verbatim; deliberately NOT parenthesised.
//  GOLDEN QUIRK : iLimit stays 0 unless iPickRow==2 AND HotPlateYPitchCanPutAll(), and
//              : `if(iLimit<=iCT) return true;` with iLimit==0 is then true for EVERY
//              : iCT>=0 -- i.e. the 1-row and cannot-put-all cases answer true
//              : unconditionally.  Kept as-is: golden behaviour, not a typo to fix.
//  NOTE         : this part also carries the INCLUDE HOIST block for all eleven k8
//              : parts (it is the lowest-numbered of them, so it stitches first).
//              : If the integrator drops or reorders this part, hoist that block to
//              : the include head first -- parts 00654 / 01482 / 02620 / 03799 all
//              : depend on it.
// =============================================================================
// -----------------------------------------------------------------------------
//  INCLUDE HOIST for ALL ELEVEN parts of k8-ainarm2 (golden :533 .. :3869).
//  HOIST ME to port ainarm2.cpp's include head at integration -- these live
//  mid-file only because a part file may not touch the head, the same convention
//  sibling part 03050_DoInArmDevicePosPrecise.txt and ainarm9045.cpp:3057/:5389/
//  :6091/:10607 already use.  Headers are include-guarded, so a sibling emitting
//  any of the same lines is free.
//
//  MEASURED, not guessed.  Probe TU = port ainarm2.cpp verbatim + probe-only
//  definitions of the integrator-owned ainarm2 globals + all eleven k8 parts,
//  compiled with
//      g++ -std=c++1z -fsyntax-only -D_WIN32_WINNT=0x0601 -DWINVER=0x0601
//          -I<root> -I<root>/vclcompat -I<root>/Motor -I<root>/forms
//          -I<root>/Public -I<root>/Common -I<root>/Interface -I<root>/Automation
//  at 2026-08-11 06:50 local.  With NO extra includes it reported exactly 10
//  "was not declared in this scope" errors and nothing else; with the seven lines
//  below it reported 0 errors, exit 0.  Everything else these parts touch (MOT[],
//  IniConfig, InArmContinuousMove_9045, OutArmContinuousMove_9045,
//  GetInArmPitchX_9045/Y_9045, MoveInArmZToPlateSafe, SetInArmHome,
//  TransferHotPlateRatio, NewRecordProcess, fAllMotorHome, AccessLevel,
//  bEnterOffset, ArmSpeed_File, Prod, TestIF/TestIF_File) already resolves through
//  the includes port ainarm2.cpp carries today.
//
//  TRAP 5 CHECK, done on the preprocessed probe (-E) and not assumed: the whole TU
//  contains EXACTLY ONE `class TMyKitSuck', and it comes from
//  aHotPlateSubstrate.h:365 -- the 177-TU layout whose object is defined at
//  aHotPlateSubstrate.cpp:80.  mykitsuck.h is never pulled in by any of the seven
//  headers below (grep for it in the -E output: zero hits).  Likewise exactly one
//  `class TInLaserCheck', also from aHotPlateSubstrate.h (:836), never the
//  OmronLaser/LaserSensorInArm.h:34 one.
// -----------------------------------------------------------------------------
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "ainarm_SearchPlacePlate.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "forms/fNote.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "aoutarm9045.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "mysensor.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "myswitch.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "FormsFacade.h"
// -----------------------------------------------------------------------------
bool CheckClearAllHotICThenPickLoadIC()                                         //ChungHung 201205104 Clear HotIC then Pick Load IC
{
    int iCT=0;
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<HotPlateForm.XDivision; j++)
        {
            for(int k=0; k<HotPlateForm.YDivision; k++)
            {
                if(HotPlateForm.iPlateSelect & 2-i)                             //MMPlate1+0=NO 2 HP
                {
                    if(MOT[MMPlate1+i].Tray.Data[j][k]!=NULL_IC)
                        iCT++;
                }
            }
        }
    }

    int iLimit=0;
    if(InArmSuck.iPickRow==2)
    {
        if(HotPlateYPitchCanPutAll())
        {
            GetHotPlateYHalfPos();
            iLimit=iYHalf*2*HotPlateForm.XDivision;
        }
    }

    if(iLimit<=iCT)
        return true;

    return false;
}

//==============================================================================
//  InArmAddSpeed()  --  golden ainarm2.cpp:569-596
//
//  ROLE          Auto-speed ramp-UP for the in-arm: nudge AutoArmSpeed[InArm]'s
//                body / AC-DC-body speed back toward the taught ArmSpeed[InArm]
//                ceiling (+5 per call while >=5 below it, then snap to exact),
//                and push the new values to the servo via SetInArmSpeed(true).
//  WAVE SCOPE    InArmAddSpeed()            ACTIVE   golden :569-596
//  GATE REGISTER (none -- every symbol has a real linkable home)
//                  AutoArmSpeed[]/ArmSpeed[]  cprod.h:2877/:2875 (ARM_CONDITION,
//                    iBodySP cprod.h:2823, iACDCBodySP cprod.h:2826) -- ints, so
//                    the -5/+5 arithmetic stays integer, as in golden.
//                  SetInArmSpeed(bool,bool=false)  cinitial.h:254 -- REAL body in
//                    cinitial.cpp (not a stub).
//                  InArm  eSpeedPart index (MachineType.h).
//
//  RETIRES A STUB: aHotPlateSubstrate.cpp:1069 `void InArmAddSpeed() {}` (decl
//    aHotPlateSubstrate.h:909) must be REMOVED by the integrator or this is a
//    `multiple definition`.  NOT written `static` here (that would be TRAP 1
//    shape (d) against that non-static declaration).  BEHAVIOUR CHANGE, and it is
//    the faithful direction: 60 live call sites currently ramp nothing; after this
//    they ramp and call SetInArmSpeed(true).  Measure on a fresh Debug+Release.
//==============================================================================
void InArmAddSpeed()
{
    if(AutoArmSpeed[InArm].iBodySP     <= (ArmSpeed[InArm].iBodySP-5) ||
       AutoArmSpeed[InArm].iACDCBodySP <= (ArmSpeed[InArm].iACDCBodySP-5))
    {
        if(AutoArmSpeed[InArm].iBodySP<=(ArmSpeed[InArm].iBodySP-5))
            AutoArmSpeed[InArm].iBodySP+=5;

        if(AutoArmSpeed[InArm].iACDCBodySP<=(ArmSpeed[InArm].iACDCBodySP-5))
            AutoArmSpeed[InArm].iACDCBodySP+=5;

        SetInArmSpeed(true);
    }
    else
    {
        if(AutoArmSpeed[InArm].iBodySP     < ArmSpeed[InArm].iBodySP ||
           AutoArmSpeed[InArm].iACDCBodySP < ArmSpeed[InArm].iACDCBodySP)
        {
            if(AutoArmSpeed[InArm].iBodySP<ArmSpeed[InArm].iBodySP )
                AutoArmSpeed[InArm].iBodySP=ArmSpeed[InArm].iBodySP;

            if(AutoArmSpeed[InArm].iACDCBodySP<ArmSpeed[InArm].iACDCBodySP)
                AutoArmSpeed[InArm].iACDCBodySP=ArmSpeed[InArm].iACDCBodySP;

            SetInArmSpeed(true);
        }
    }
}

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 00598 : InArmSubSpeed
//  Golden: HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp:598-611 (cp950, CRLF).
//
//  ROLE          : steps the in-arm AUTO speed profile DOWN one notch -- takes 5 off
//                  the body speed and off the accel/decel body speed (each only while
//                  still above the floor of 25) and re-pushes the profile to the arm
//                  motors via SetInArmSpeed(true).  The per-site in-arm SMs call it
//                  after a pick/place retry so the arm slows down.
//  WAVE SCOPE    : InArmSubSpeed()   golden ainarm2.cpp:598-611   ACTIVE
//  GATE REGISTER : (none -- every symbol has a real port body)
//                  AutoArmSpeed[] cprod.h:2877 / InArm MachineType.h:682 /
//                  SetInArmSpeed(bool,bool) cinitial.h:254, body cinitial.cpp:16553.
//  INTEGRATOR    : RETIRE the no-op stub `void InArmSubSpeed() {}` at
//                  aHotPlateSubstrate.cpp:1068 (keep the aHotPlateSubstrate.h:908
//                  declaration).  BEHAVIOUR CHANGE and it is the faithful direction:
//                  today every one of the ~30 variant-SM call sites is a no-op, so the
//                  arm never slows after a retry.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "cinitial.h"
//==============================================================================
void InArmSubSpeed()
{
    if(AutoArmSpeed[InArm].iBodySP>25 ||
       AutoArmSpeed[InArm].iACDCBodySP>25)
    {
        if(AutoArmSpeed[InArm].iBodySP>25)
            AutoArmSpeed[InArm].iBodySP-=5;

        if(AutoArmSpeed[InArm].iACDCBodySP>25)
            AutoArmSpeed[InArm].iACDCBodySP-=5;

        SetInArmSpeed(true);
    }
}

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:613-616 -------------------------------
//  ROLE          -- resets the in-arm "try-pick from HotPlate" task cursor to its
//                   entry step (1), so the next DoInArmTryPickFromHotPlate_* pass
//                   restarts that SM from the top.  Pure cursor write, no HAL.
//  WAVE SCOPE    -- InitInArmTryPickFromHotPlateTask()   ACTIVE   golden :613-616
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  *** INTEGRATE-STEP COLLISION -- READ BEFORE STITCHING ***
//  aHotPlateSubstrate.cpp:1491 ALREADY carries a byte-equivalent real body for
//  this exact golden function ("void InitInArmTryPickFromHotPlateTask(){
//  iInArmTryPickFromHotPlateTask=1; }     // golden :613"), declared at
//  aHotPlateSubstrate.h:1086.  It is NOT a no-op stub -- it is a correct earlier
//  translation parked outside golden's home file.  Landing this part therefore
//  makes TWO external definitions of one symbol => `multiple definition` at link.
//  ACTION FOR THE INTEGRATOR: DELETE aHotPlateSubstrate.cpp:1491 and keep this one
//  (same convention aTester_Rear.cpp used for its 6 retired atester_shims bodies).
//  This is now UNAMBIGUOUS, not a judgement call: sibling part
//  _w7e_parts/00618_InitInArmTryPickFromHotPlateTask100.txt brings golden :618 --
//  the other half of this two-function family -- home to ainarm2.cpp in the same
//  stitch, so keeping mine in aHotPlateSubstrate.cpp would split the family.
//  If instead you drop this part, nothing is lost behaviourally (the two bodies are
//  identical) but the family stays split.
//  Measured 2026-08-11 06:24 +0800:
//    grep -n "InitInArmTryPickFromHotPlateTask" aHotPlateSubstrate.cpp aHotPlateSubstrate.h
//  Sibling-part cross-check re-run 2026-08-11 06:46:04 +0800:
//    ls _w7e_parts | grep InitInArmTryPickFromHotPlateTask   -> 00613 (mine), 00618 (k?)
//  Symbol used: iInArmTryPickFromHotPlateTask -- golden ainarm2.cpp:62 file-scope
//  global, already in the port at aHotPlateSubstrate.cpp:1117 (extern .h:1056);
//  NOT emitted here (globals belong to the main loop).
// ---------------------------------------------------------------------------
void InitInArmTryPickFromHotPlateTask()                                         //ChungHung 20120206 Hotplate check
{
    iInArmTryPickFromHotPlateTask=1;
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 618..621
// -----------------------------------------------------------------------------
//  ROLE        : parks the HotPlate try-pick cursor at 100 -- the golden HangUp-release entry point (its :1 sibling InitInArmTryPickFromHotPlateTask is golden :613, not in this part).
//  WAVE SCOPE  : InitInArmTryPickFromHotPlateTask100 -- ACTIVE, golden ainarm2.cpp:618..621.
//  COLLISION    : *** INTEGRATOR MUST DELETE THE EXISTING PARTIAL/STUB BODY ***
//              : ainarm9045_2x4_16_shims.cpp:72  `void InitInArmTryPickFromHotPlateTask100()               {}`
//              : It is a NON-static definition of the same signature, so leaving
//              : it in place is a `multiple definition` link error, not a silent
//              : shadow.  Its `extern` DECLARATION stays where it is -- only the
//              : body is deleted.
//  INCLUDES     : none beyond the current set.
//  GATE REGISTER: none.  iInArmTryPickFromHotPlateTask already exists for real --
//              : aHotPlateSubstrate.cpp:1117 `int iInArmTryPickFromHotPlateTask = 1;`,
//              : declared aHotPlateSubstrate.h:1056.
//  BEHAVIOUR    : this is NOT a no-op-to-no-op swap.  The stub above DISCARDED the
//              : write, so all 23 variant call sites in ainarm9045_*.cpp that ask for
//              : the 100 entry silently kept whatever cursor value they had; once this
//              : part lands they really do jump to case 100.  Expect live movement in
//              : the HotPlate try-pick hang-up release path -- that is the point.
// =============================================================================
void InitInArmTryPickFromHotPlateTask100()                                      //ChungHung 20120206 Hotplate check
{
    iInArmTryPickFromHotPlateTask=100;
}

//==============================================================================
//== ROLE: InitInArmPickFromHotPlateTask340 -- park the in-arm HotPlate-pick task
//==       cursor at step 340, the hang-up release entry point.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   InitInArmPickFromHotPlateTask340  golden ainarm2.cpp:628-631  -- ACTIVE
//==     (golden lines emitted VERBATIM; ZERO gates)
//==
//== GATE REGISTER: (none)
//==   iInArmPickFromHotPlateTask is already resolved in THIS TU -- its sibling
//==   InitInArmPickFromHotPlateTask (golden :623) writes it at port
//==   ainarm2.cpp:110-113.
//==
//== INTEGRATION NOTE -- THIS IS A DUPLICATE, NOT A STUB RETIREMENT:
//==   port aHotPlateSubstrate.cpp:1474 ALREADY carries a fully faithful one-line
//==   body for this function, behaviourally identical to golden, and the
//==   declaration lives at aHotPlateSubstrate.h:1079.  So landing this part is a
//==   pure MOVE with ZERO behaviour change -- golden home is ainarm2.cpp, which is
//==   why it is in scope, but the main loop must EITHER delete
//==   aHotPlateSubstrate.cpp:1474 OR drop this part.  Keeping both is a
//==   multiple-definition link error.  Note also that the port ainarm2.cpp file
//==   banner (its lines 17-19) still claims the 340/50 siblings "already live in
//==   aHotPlateSubstrate.cpp"; that sentence goes stale the moment this part lands
//==   and should be corrected in the same commit.
//==   Not static -- a non-static declaration exists at aHotPlateSubstrate.h:1079
//==   (TRAP 1 shape (d) avoided).
//==
//== Big5 -> UTF-8: the trailing Chinese comment is transcribed character for
//== character from cp950.  Zero U+FFFD.
//==============================================================================
void InitInArmPickFromHotPlateTask340()                                         //ChungHung 20120502 HangUp 解除
{
    iInArmPickFromHotPlateTask=340;
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 654..672
// -----------------------------------------------------------------------------
//  ROLE        : after a shuttle/kit reset, decide whether to re-order the shuttle place sequence (Ambient, or Hot with both plates empty) or to re-map the HotPlate data (Hot with ICs left on a plate).
//  WAVE SCOPE  : ResetShuttleWhichKit -- ACTIVE, all three arms, golden ainarm2.cpp:654..672.
//  COLLISION    : *** INTEGRATOR MUST DELETE THE EXISTING PARTIAL/STUB BODY ***
//              : aHotPlateSubstrate.cpp:1053  `void ResetShuttleWhichKit() {}`
//              : It is a NON-static definition of the same signature, so leaving
//              : it in place is a `multiple definition` link error, not a silent
//              : shadow.  Its `extern` DECLARATION stays where it is -- only the
//              : body is deleted.
//  INCLUDES     : LastSet comes from canary_support.h, in the INCLUDE HOIST block in
//              : part 00533_CheckClearAllHotICThenPickLoadIC.txt.  MOT[] and
//              : TTrayMotor::HasIC (Motor/mymotor.h:353) already resolve -- measured.
//  GATE REGISTER: EMPTY -- zero #if 0 in this part.  There WAS a gate W7E-K8-A on
//              : golden :669 and it was RETIRED BEFORE SHIPPING because its premise
//              : expired inside this same wave.  The full story, both timestamps and
//              : the TRAP 2 / TRAP 3 reasoning are in the DECLARATION HOIST block
//              : immediately below this banner.  Read it -- it is the record of an
//              : absence claim that died 10 minutes after I measured it.
//  NOTE         : AdjustShuttlePlaceOrder is real (aHotPlateSubstrate.h:905) and the
//              : port declares it `AdjustShuttlePlaceOrder(int iReset=-1)`, so golden's
//              : no-argument calls at :658 and :665 bind to the default.  ACTIVE.
//  BEHAVIOUR    : replacing the `{}` stub means those two AdjustShuttlePlaceOrder calls
//              : now really fire from csystem.cpp:5482 / csystem.cpp:6894 and
//              : ainarm_SearchPlacePlate.cpp:2233.  Real, intended change.
// =============================================================================
// -----------------------------------------------------------------------------
//  DECLARATION HOIST for golden :669's callee (a declaration -- NOT a definition,
//  NOT a global, NOT a gate).  THIS REPLACED A GATE, ON PURPOSE: read the story.
//
//  I first wrote this as `#if 0 // GATE W7E-K8-A' because ChangeHotPlateData had no
//  port body and no port declaration.  Command, run from
//  D:/HT9045/HT9011UC_Cpp_V3.33.906.0 at 2026-08-11 06:33:20 local:
//      rg -n --no-messages -g '*.cpp' -g '*.h' 'ChangeHotPlateData' .
//  -> zero hits, exit status 1.
//  TRAP 2 IN THE FLESH: I re-ran that command at 2026-08-11 06:43 local, before
//  finishing, and the premise HAD EXPIRED INSIDE THE SAME WAVE -- a sibling part
//  _w7e_parts/00976_ChangeHotPlateData.txt had landed golden :976-991 in the
//  meantime (its own banner calls itself "PARTLY GATED", but the FUNCTION and its
//  AdjustShuttleWhichKitOrder/AdjustShuttlePlaceOrder tail are real, so the symbol
//  will exist).  Leaving my gate in would have shipped a born-expired gate -- the
//  exact defect class this campaign already has 50 open instances of.
//  TRAP 3 RE-ASKED, "why SHOULD this be gated now": it should NOT.  Only ONE thing
//  is still missing, and it is not a body -- it is a prototype.  golden :976
//  stitches AFTER this call site at :654, sibling part 00976 emits no declaration of
//  its own, and the port has no ainarm2.h, so `ChangeHotPlateData();' at :669 cannot
//  name it.  The honest fix for "no declaration" is to DECLARE, so the real golden
//  behaviour runs.  Golden's own declaration, ainarm2.h:104, is reproduced verbatim
//  below, DEFAULT ARGUMENT INCLUDED -- golden's call at :669 passes no argument, so
//  the default is load-bearing, not decoration.
//  Guarded, because a duplicate default-argument declaration in one TU is an error
//  ("redefinition of default argument") even though duplicate plain declarations are
//  legal.  Verified at 2026-08-11 06:43 local that no other part file in
//  _w7e_parts declares it: rg -n 'ChangeHotPlateData' D:/HT9045/HT9011UC_Cpp_V3.33.906.0/_w7e_parts
//  returns only sibling 00976's banner/definition and this part.
//  IF SIBLING 00976 IS DROPPED FROM THE STITCH, this becomes an undefined reference
//  at link -- loud, which is the correct failure mode, and strictly better than a
//  silent no-remap.  INTEGRATOR: hoist this line to the declaration block and delete
//  it from here.
#ifndef HT9045_W7E_K8_ChangeHotPlateData_DECLARED
#define HT9045_W7E_K8_ChangeHotPlateData_DECLARED
// AI(pt-wave) 20260811: local forward declaration removed -- the integrator published this in aHotPlateSubstrate.h with golden's default, and giving the default twice is an error.
#endif
// -----------------------------------------------------------------------------
void ResetShuttleWhichKit()                                                     //Steven 20140710
{
    if(LastSet.iTemperature!=Tempture_Hot)
    {
        AdjustShuttlePlaceOrder();                                              //Steven 20160213 : 解決Input擺放順序問題
    }
    else
    {
        if(MOT[MMPlate1].HasIC()==false &&
           MOT[MMPlate2].HasIC()==false)
        {
            AdjustShuttlePlaceOrder();                                          //Steven 20160213 : 解決Input擺放順序問題
        }
        else
        {
            ChangeHotPlateData();
        }
    }
}

// =============================================================================
//  AdjustShuttlePlaceOrder_AutoSiteMapping()   golden ainarm2.cpp:674-705
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- JCET-only Auto-Site-Map variant of AdjustShuttlePlaceOrder: chooses WHICH
//    shuttle the in-arm picks for next (InArmSuck.iWhichShtPickFor32) while running
//    rsmAutoSiteMap.  Single-shuttle mode pins it to iShuttle_Sel; the D58
//    arm1-place/arm2-test mode pins it to 0; otherwise it alternates 0,1,0,1...
//    NN_2Row returns early and leaves the cursor untouched.
//
//  WAVE SCOPE -- AdjustShuttlePlaceOrder_AutoSiteMapping (golden :674-705) ACTIVE, no gates.
//  GATE REGISTER -- (empty.)
//
//  RETIRE-AT-INTEGRATION: a no-op stub of this exact symbol lives at
//  ainarm9045_2x4_16_shims.cpp:70 (`void AdjustShuttlePlaceOrder_AutoSiteMapping() {}`),
//  declared ainarm9045_2x4_16_shims.h:80 and aHotPlateSubstrate.h:1116.  Two live call
//  sites already reach it (ainarm9045S_1x4_4.cpp:2401, ainarm9045S_2x4_4_13.cpp:2719),
//  so this is TRAP-1 shape (b): until that stub is deleted the linker keeps satisfying
//  them with the no-op and this body is never extracted.
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
void AdjustShuttlePlaceOrder_AutoSiteMapping()                                  //jou 2016-11-07 JCET fix Auto Site mapping Hot mode hangup
{
    if(CosFunction.bUSEJCETSiteMapMode)                                         //jou 2016-11-07 JCET fix Auto Site mapping Hot mode hangup
    {
        if(IniConfig.bUseAutoSiteMapping &&
           LastSet.iRunStartMode==rsmAutoSiteMap)
        {
            if(IsNNMode()==NN_2Row)                                             //Steven 20220531 : for NN mode Auto Site Map
                return;

            if(TestIF_File.iShuttleMode==1)
            {
                if(TestIF_File.iShuttle_Sel==0)
                    InArmSuck.iWhichShtPickFor32=0;
                else if(TestIF_File.iShuttle_Sel==1)
                    InArmSuck.iWhichShtPickFor32=1;
            }
            else if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
                    TestIF_File.bArm1PickPlaceArm2Test==true &&
                    TestIF_File.bArm1OnlyPlaceArm2TestAndSuck==true)
            {
                InArmSuck.iWhichShtPickFor32=0;
            }
            else
            {
                InArmSuck.iWhichShtPickFor32++;
                if(InArmSuck.iWhichShtPickFor32>=2)
                    InArmSuck.iWhichShtPickFor32=0;
            }
        }
    }
}
//==============================================================================

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:707-746 -------------------------------
//  ROLE          -- picks WHICH in-shuttle (InArmSuck.iWhichSht) the in-arm will
//                   place into next, from the run-time shuttle mode: a forced
//                   single-shuttle selection (TestIF_File.iShuttleMode==1 ->
//                   iShuttle_Sel), the D58 "Arm1 only places / Arm2 tests+sucks"
//                   config (always shuttle 0), or the plain alternating order
//                   driven by the iReset argument.  iReset==-1 additionally zeroes
//                   iWhichKit.  Pure selector state, no HAL, no motion.
//  WAVE SCOPE    -- AdjustShuttlePlaceOrder(int)          ACTIVE   golden :707-746
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  FIDELITY NOTES
//   * The default argument (`int iReset=-1`) lives ONLY in the declaration
//     (aHotPlateSubstrate.h:905, mirroring golden ainarm2.h:128); golden's
//     definition line has no default and neither does this one.
//   * The iReset==-1 / ==0 / else arms of the final ladder all assign 0 -- golden
//     writes them out as three separate branches anyway.  PRESERVED VERBATIM
//     (collapsing them would be a redesign, and the branch shape is what the next
//     maintainer diffs against).
//  INTEGRATE-STEP NOTE: aHotPlateSubstrate.cpp:1066 currently holds the offline
//  no-op stub `void AdjustShuttlePlaceOrder(int) {}` for this symbol -- REMOVE it
//  as part of the stitch (same pattern as aTester_Rear.cpp's retired shims).
//  Symbols used, all already present: InArmSuck (aHotPlateSubstrate.h:624 object,
//  defined aHotPlateSubstrate.cpp:80 -- see TRAP 5 note in the report),
//  TestIF_File (cprod.h:2577), IniConfig (Config.h:1499 via cprod.h).
// ---------------------------------------------------------------------------
void AdjustShuttlePlaceOrder(int iReset)
{
    if(iReset==-1)
    {
        InArmSuck.iWhichKit=0;
    }

    if(TestIF_File.iShuttleMode==1)                                             //Steven 20160223 : TestIF --> TestIF_File
    {
        if(TestIF_File.iShuttle_Sel==0)
            InArmSuck.iWhichSht=0;
        else if(TestIF_File.iShuttle_Sel==1)
            InArmSuck.iWhichSht=1;
    }
    else if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                     //Jimmychiu 20250722 : Arm1 Only Place Arm2 Test And Suck
            TestIF_File.bArm1PickPlaceArm2Test==true &&
            TestIF_File.bArm1OnlyPlaceArm2TestAndSuck==true)
    {
        InArmSuck.iWhichSht=0;
    }
    else
    {
        if(iReset==-1)
        {
            InArmSuck.iWhichSht=0;
        }
        else if(iReset==0)
        {
            InArmSuck.iWhichSht=0;
        }
        else if(iReset==1)
        {
            InArmSuck.iWhichSht=1;
        }
        else
        {
            InArmSuck.iWhichSht=0;
        }
    }
}

//==============================================================================
//  AdjustShtOrderWhenPlaceToSht(int iMode)                golden ainarm2.cpp:772-847
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : the single post-place "which shuttle / which kit do I feed next"
//               adjuster.  Two independent jobs in one function: (1) if the
//               initial-delay-as-Soak-time scheme is armed and this is still the
//               first hot input, clear bFirstInput/bOneTimeWait at the exact
//               iMode-specific moment the last kit of the batch lands (iMode 1=Sht1,
//               2=Sht2, 3=16Site Sht1 Kit1, 4=16Site Sht2 Kit1); and (2) re-seed the
//               shuttle/kit order for the ambient, hot-QA-cleanout, hot-Auto-Site-Map
//               and 1-picker-hot cases, then -- for JCET/JSCS Auto-Site-Mapping --
//               also re-seed the ASM place order.
//  WAVE SCOPE : AdjustShtOrderWhenPlaceToSht(int)  golden :772-847  -- ACTIVE except
//               ONE gated call, see GATE k4e-G01 below.  That gate`s ORIGINAL premise
//               ("no port body") died inside this wave when sibling part
//               00748_AdjustShuttlePlaceOrderForASM.txt landed at 06:47; it stays
//               gated for a NEW and much smaller reason.  Read the gate, not the
//               absence command.
//  RETIRES    : the offline no-op `void AdjustShtOrderWhenPlaceToSht(int){}` at
//               aHotPlateSubstrate.cpp:1541.  REAL BEHAVIOUR CHANGE: the four
//               AdjustShuttleWhichKitOrder() call sites now actually fire (that
//               callee is itself still the no-op stub at
//               aHotPlateSubstrate.cpp:1537, so the visible offline change is the
//               bFirstInput/bOneTimeWait clears, which the stub swallowed).
//
//  GATE REGISTER
//  ------------------------------------------------------------------------------
//  [k4e-G01]  golden :844   AdjustShuttlePlaceOrderForASM();
//    SYMBOL   : void AdjustShuttlePlaceOrderForASM(bool bResetSht=false)
//               -- golden ainarm2.h:129, body golden ainarm2.cpp:748-770.
//    *** READ THIS FIRST -- THE ORIGINAL PREMISE DIED INSIDE THE WAVE, AND THE GATE
//        STILL STAYS FOR A DIFFERENT, SMALLER REASON (TRAP 3). ***
//               Re-checked 2026-08-11 06:47:58 +0800 (71 sibling part files):
//               _w7e_parts/00748_AdjustShuttlePlaceOrderForASM.txt HAS NOW LANDED
//               with the full golden :748-770 body, ACTIVE and ungated.  So "the
//               symbol has no port body" is FALSE from integration onward and the
//               06:31:32 absence measurement below is a dead snapshot, kept only so
//               the main loop can see which claim died and when.
//               WHY IT MUST STILL STAY GATED: golden puts the DEFAULT ARGUMENT on the
//               DECLARATION (ainarm2.h:129 `bool bResetSht=false`), and the sibling`s
//               definition correctly carries none -- while the port has NO ainarm2.h
//               and no declaration for this function anywhere.  golden :844 calls it
//               with ZERO arguments, so ungating it now is a hard compile error
//               ("too few arguments"), even though the definition precedes it in
//               golden line order.  This is a MISSING-DEFAULT-ARGUMENT gate, not a
//               missing-symbol gate.
//               RETIRE IT the moment the integrator publishes
//                 extern void AdjustShuttlePlaceOrderForASM(bool bResetSht=false);
//               (aHotPlateSubstrate.h`s "functions (golden ainarm2.cpp)" block, next
//               to AdjustShuttleWhichKitOrder at :1068, is where its siblings live) --
//               ideally in the SAME commit, because that declaration is also what the
//               00748 part needs to stop being TRAP 1 shape (a).  I did NOT substitute
//               an explicit `(false)` for the zero-arg call: golden`s default happens
//               to be false, so it would be behaviourally equivalent TODAY, but it
//               edits golden`s text and hides the missing declaration -- and "the
//               degraded value happens to be equivalent" is exactly the reasoning this
//               campaign has already paid for.
//               (Its dependencies are all real: TestIF_File cprod.h:2577,
//               .iShuttleMode cprod.h:1654, .iShuttle_Sel cprod.h:1655,
//               iAutoSiteMapHPToSht declared PORT cmydef.h:4386 and really defined
//               cmydef.cpp:4537 -- so nothing else blocks the retire.)
//    WHY THE OFFLINE DEFAULT (call dropped) IS FAITHFUL: the enclosing if
//               (golden :835-837) needs LastSet.iRunStartMode==rsmAutoSiteMap AND
//               (CosFunction.bUSEJCETSiteMapMode || CUSTOMER_CODE==CC_SCS).  Offline
//               the run start mode is not rsmAutoSiteMap, the JCET site-map flag is
//               false and CUSTOMER_CODE is not CC_SCS (944), so the block is
//               unreachable and dropping the call changes nothing any offline path
//               observes.  Stated plainly: this is a real dropped call, not an
//               equivalence -- the premise is unreachability, not "the degraded
//               value happens to match".
//    REAL-MACHINE DIFFERENCE: on a JCET / JSCS machine running Auto Site Mapping
//               with a single (non-bUseTwoArm32Site) head, iAutoSiteMapHPToSht is no
//               longer re-seeded after a place, so a one-cycle taken during ASM
//               leaves the stale HotPlate->shuttle target -- exactly the "one cycle
//               during ASM hangs when arm 1 is closed" fault Steven 20250206 fixed.
//    ABSENCE COMMAND (cwd D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//               rg -n --glob "*.cpp" --glob "*.h" --glob "!build/**"
//                  -e "^[A-Za-z_].*\bAdjustShuttlePlaceOrderForASM\s*\("
//                  -e "extern\s+void\s+AdjustShuttlePlaceOrderForASM" .
//               RESULT: 0 lines.  RUN AT: 2026-08-11 06:31:32 +0800.
//               The three textual hits of the bare name
//               (ainarm_SearchPlacePlate.cpp:4839, csystem.cpp:7226,
//               csystem.cpp:7262) are ALL inside golden`s own /* */ comment blocks
//               -- hand-checked 2026-08-11 06:22 +0800, so there is no live caller
//               either.
//    TRAP 2 -- THIS CLAIM CAN EXPIRE INSIDE THIS WAVE: golden :748 sits only 24
//               lines above my range start (:772), so a sibling agent may well own
//               it.  RE-CHECKED AGAINST THE LIVE WAVE at 2026-08-11 06:45:15 +0800
//               with 61 sibling part files already present in _w7e_parts: the
//               adjacent ranges DID land (00674_AdjustShuttlePlaceOrder_AutoSiteMapping
//               = golden :674-706 and 00707_AdjustShuttlePlaceOrder = golden
//               :707-746) but there is still NO part covering golden :748-770 --
//               `ls _w7e_parts | grep ^00748` -> 0 hits.  Parts were still arriving
//               at that minute, so RE-RUN BOTH the rg command and that ls at
//               integration.  IT DID LAND, at 06:47 -- see the "READ THIS FIRST"
//               block above, which supersedes this paragraph and names the one
//               remaining blocker (the missing default argument).
//  ------------------------------------------------------------------------------
//
//  HOIST ME to ainarm2.cpp`s include head at integration:
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
//  (CosFunction: cmydef.h -> cprod.h:7 -> CosFunction.h:488.  Temperature:
//   cprod.h:1646.  rsmQAMode/rsmAutoSiteMap/ep1Picker/CC_SCS: MachineType.h.
//   AdjustShuttleWhichKitOrder: aHotPlateSubstrate.h:1068.  All already in head.)
//==============================================================================
void AdjustShtOrderWhenPlaceToSht(int iMode)                                    //Steven 20180601 : 整合放完蝦頭後的調整
{
    if(CosFunction.bUseInitialDelayAsSoakTime &&                                //Steven 20170511 (wei) : 使用initial delay當 Soak time
       Temperature.bUseInitialDelayAsSoakTime &&
       LastSet.iTemperature==Tempture_Hot &&
       bFirstInput==true)
    {
        if(iMode==4)                                                            //16Site, Sht2, Kit1
        {
            if(InArmSuck.iWhichKit==1)
            {
                bFirstInput=false;
                bOneTimeWait=false;
            }
        }
        else if(iMode==3)                                                       //16Site, Sht1, Kit1
        {
            if(TestIF_File.iShuttleMode==1)
            {
                if(TestIF_File.iShuttle_Sel==0 && InArmSuck.iWhichKit==1)
                {
                    bFirstInput=false;
                    bOneTimeWait=false;
                }
            }
        }
        else if(iMode==1)                                                       //Sht1
        {
            if(TestIF_File.iShuttleMode==1)
            {
                if(TestIF_File.iShuttle_Sel==0)
                {
                    bFirstInput=false;
                    bOneTimeWait=false;
                }
            }
        }
        else if(iMode==2)                                                       //Sht2
        {
            bFirstInput=false;
            bOneTimeWait=false;
        }
        AdjustShuttleWhichKitOrder();
    }
    else if(LastSet.iTemperature==Tempture_Hot &&
            LastSet.iRunStartMode==rsmQAMode &&
            bQAModeFinishCleanOut==true)                                        //Steven 20180601 : 修正高溫QA mode放料
    {
        AdjustShuttleWhichKitOrder();
    }
    else if(LastSet.iTemperature==Tempture_Ambient ||
            LastSet.iTemperature==Tempture_AmbientHot ||
            (LastSet.iTemperature==Tempture_Hot &&
             LastSet.iRunStartMode==rsmAutoSiteMap &&
             CosFunction.bUSEJCETSiteMapMode==false))                           //kevin 20161124 add hot AutoSitmap hang up
    {
        AdjustShuttleWhichKitOrder();                                           //Steven 20160201 : 修正Shuttle放料順序
    }
    else if(USE_PICKER_COUNT==ep1Picker && LastSet.iTemperature==Tempture_Hot)
    {
        AdjustShuttleWhichKitOrder();
    }

    if((CosFunction.bUSEJCETSiteMapMode ||
        CUSTOMER_CODE==CC_SCS) &&                                               //jou 20230626 : 修正JSCS auto site mapping 2x8 hangup
       LastSet.iRunStartMode==rsmAutoSiteMap)                                   //Steven 20220913 : Fixed for ASM function
    {
        if(bUseTwoArm32Site==true)
        {
        }
        else
        {
#if 0 // GATE k4e-G01 -- golden :844  AdjustShuttlePlaceOrderForASM()  (golden home
      // ainarm2.h:129, body golden ainarm2.cpp:748-770, OUTSIDE this agent`s ranges).
      // The BODY landed in-wave (sibling part 00748), so this is NO LONGER a
      // missing-symbol gate: the only remaining blocker is that golden puts the
      // DEFAULT ARGUMENT on the declaration and the port has no ainarm2.h, so this
      // zero-arg call would not compile.  RETIRE THIS GATE as soon as
      //   extern void AdjustShuttlePlaceOrderForASM(bool bResetSht=false);
      // is published.  ONLY THE CALL is gated: the enclosing if/else, the empty
      // bUseTwoArm32Site==true arm and all four AdjustShuttleWhichKitOrder() call
      // sites stay ACTIVE.  Full detail in the GATE REGISTER banner above.
            AdjustShuttlePlaceOrderForASM();                                    //Steven 20250206 : fixed ASM之中作one cycle會造成關arm 1時異常
#endif // GATE k4e-G01
        }
    }
}

// =============================================================================
//  AdjustShuttleWhichKitOrder()   golden ainarm2.cpp:849-921   (Steven 20140626)
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- advances the in-arm's shuttle-KIT cursor after a place-to-shuttle.  For the
//    listed per-site-config picker kinds it steps InArmSuck.iWhichKit and, on wrap
//    (>=2), hands over to AdjustShuttlePlaceOrder(iWhichSht+1) and re-seats the kit --
//    for 2x8_32 starting at kit 1 when the LEFT half is fully closed but the right half
//    is not.  Every branch also drives the G12 'contract mode' manual test-signal
//    counter (2 touch-downs -> fMain->CleanOut).  Tail: re-map the suckers and clear
//    bInArmPlaceToShuttleFinish.
//
//  WAVE SCOPE -- AdjustShuttleWhichKitOrder (golden :849-921) ACTIVE, 1 gate.   //AI(W906-AMB-L2) 20260929: now 0 gates -- W7E-2 retired
//
//  GATE REGISTER
//   [W7E-2] golden ainarm2.cpp:919  DoInArm_SuckerMap();   //AI(W906-AMB-L2) 20260929: RETIRED (see :4439) -- body is at :4488 below, extern at aHotPlateSubstrate.h:1096; the text below is history
//     WHY GATED -- the function has NO callable body in this tree.  Its golden home is
//       ainarm2.cpp:965 (declared golden ainarm2.h:148) -- i.e. THIS VERY FILE, just
//       outside this wave's range and not among this wave's part files.  The only
//       occurrence anywhere in the port is `static void DoInArm_SuckerMap() {}` at
//       AutoClean/AutoClean.cpp:127 -- file-scope STATIC, invisible outside that TU
//       (that is TRAP-1 shape (d)/(e), and it is pre-existing, not mine); AutoClean.cpp
//       will keep calling its own stub even after the real body lands.
//     THIS GATE EXPIRES THE MOMENT golden ainarm2.cpp:965 IS TRANSLATED INTO THIS FILE
//       (very likely the next ainarm2 wave) -- at that point delete the #if 0 and call it.
//     OFFLINE DEFAULT + WHY FAITHFUL-ENOUGH -- the call is skipped.  It is the LAST
//       statement before `bInArmPlaceToShuttleFinish=false;` and returns void; nothing in
//       this function reads back what it computes, so control flow here is unchanged.
//     REAL-MACHINE DIFFERENCE -- SERIOUS, do not treat as cosmetic: on the machine this
//       call rebuilds the in-arm nozzle->site map after the kit/shuttle order advanced
//       (golden's own comment at AutoClean.cpp call sites: 開放site 治具使用異常修正).
//       Skipping it leaves the previous map in place, so with a closed/opened site the
//       next pick can address the wrong nozzle set.  Offline the map is only read by
//       code paths that are themselves stubbed, which is why it is survivable HERE and
//       nowhere near acceptable as a permanent state.
//     ABSENCE COMMAND (re-run at integration; TRAP 2) --
//       grep -rn "DoInArm_SuckerMap" --include=*.h --include=*.cpp D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//       expect: ONLY AutoClean/AutoClean.cpp (a static stub at :127 + its call sites) and
//       no declaration in any header.  MEASURED 2026-08-11 15:31.
//
//  RETIRE-AT-INTEGRATION: no-op stub at aHotPlateSubstrate.cpp:1537 (decl
//  aHotPlateSubstrate.h:1068).  At least 7 live call sites already bind to it
//  (ainarm9045_1x1_1.cpp:549, ainarm9045S_1x4_4.cpp:637/2031/2828,
//  ainarm9045S_2x4_4_13.cpp:934/2343/3128) -- TRAP-1 shape (b) until the stub goes.
//  NOTE -- golden :867 calls AdjustShuttlePlaceOrder(InArmSuck.iWhichSht+1) while :897
//  calls the 0-arg form; both are the SAME golden function (ainarm2.cpp:707, default
//  argument in golden ainarm2.h:129).  Kept exactly as golden wrote them; the port's
//  declaration must therefore keep its default argument.
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
void AdjustShuttleWhichKitOrder()                                               //Steven 20140626 : Modify to Function
{
    iInArmPutIcToSH[InArmSuck.iWhichSht][InArmSuck.iWhichKit]=0;                //Ifor 20171121 : Test 查看異常資料

    if(iInArmType==e9045_1x3_2_14 ||                                            //Steven 20220425 : 1x3_14
       iInArmType==e9045_1x4_2_14 ||
       iInArmType==e9045_2x3_6_14 ||                                            //Steven 20200219 : 2x3mode, Layout: 120x70mm
       iInArmType==e9045_2x4_4_13 ||
       iInArmType==e9045_2x4_4_14 ||                                            //Steven 20240221 : fixed for 1x4_2_14
       iInArmType==e9045_2x5_8    ||
       (iInArmType==e9045_2x6_8 && iCloseSiteModeFor2x6<=e2x6OneByOne) ||       //Steven 20240417 : 12site關中間4site
       (iInArmType==e9045_2x8_8 && iCloseSiteModeFor2x8<=e2x8OneByOne) ||
       iInArmType==e9045_2x8_32)
    {
        InArmSuck.iWhichKit++;
        if(InArmSuck.iWhichKit>=2)
        {
            //InArmSuck.iWhichKit=0;
            AdjustShuttlePlaceOrder(InArmSuck.iWhichSht+1);

            if(iInArmType==e9045_2x8_32)                                        //Sam 20250417 : 修正 CloseSite 異常，左邊半邊全關 右半邊沒有全關 Kit 要從1開始。
            {
                if(InArmSideAllCloseWithKit(InArmSuck.iWhichSht,0)==true &&
                   InArmSideAllCloseWithKit(InArmSuck.iWhichSht,1)==false)
                    InArmSuck.iWhichKit=1;
                else
                    InArmSuck.iWhichKit=0;
            }
            else
            {
                 InArmSuck.iWhichKit=0;
            }

            if(IniConfig.bG12ContractModeManualMessage)                         //kevin 20180222 (Steven) Arm 1 Arm2 吸取IC 做CONTRACT MODE
            {
                iContractModeTest++;                                            //kevin 20180222 (Steven) contract mode 手動送測試訊號
                if(iContractModeTest>=2)
                {
                    iContractModeTest=0;
                    fMain->CleanOut("AdjustShuttleWhichKitOrder 1");
                }
            }
        }
    }
    else
    {
        if(TestIF_File.iShuttleMode==1)                                         //JerryYang 20161107 修正關arm異常
        {
            AdjustShuttlePlaceOrder();
            if(IniConfig.bG12ContractModeManualMessage)                         //kevin 20180222 (Steven) Arm 1 Arm2 吸取IC 做CONTRACT MODE
            {
                fMain->CleanOut("AdjustShuttleWhichKitOrder 2");
            }
        }
        else
        {
            AdjustShuttlePlaceOrder(InArmSuck.iWhichSht+1);

            if(IniConfig.bG12ContractModeManualMessage)                         //kevin 20180222 (Steven) Arm 1 Arm2 吸取IC 做CONTRACT MODE
            {
                iContractModeTest++;                                            //kevin 20180222 (Steven) contract mode 手動送測試訊號
                if(iContractModeTest>=2)
                {
                    iContractModeTest=0;
                    fMain->CleanOut("AdjustShuttleWhichKitOrder 3");
                    bLoadContractModeTest=false;
                }
            }
        }
    }
//#if 0 // GATE W7E-2 -- golden ainarm2.cpp:919 (no callable DoInArm_SuckerMap body in this tree; see banner)   //AI(W906-AMB-L2) 20260929: gate retired -- the stated reason no longer holds: the real body is DoInArm_SuckerMap() at ainarm2.cpp:4488 (golden ainarm2.cpp:965-972), declared at aHotPlateSubstrate.h:1096 (included :37)
    DoInArm_SuckerMap();                                                        //AI(W906-AMB-L2) 20260929: live again, golden ainarm2.cpp:919
//#endif //AI(W906-AMB-L2) 20260929: gate W7E-2 retired (see line above the call)
    bInArmPlaceToShuttleFinish=false;                                           //Ifor 20171121 : add iSht & iWhichKit 切換旗標 避免沒切換狀態造成Hangup
}
//==============================================================================

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:965-972 -------------------------------
//  ROLE          -- rebuilds the in-arm nozzle/site map after anything changed
//                   which sites are open: re-derives the picker TYPE, re-derives
//                   the sucker map, then re-caches the close-site state for both
//                   the place side (CloseSiteState(true)) and the pick side
//                   (CloseSiteState(false)).  Two global writes, no HAL.
//  WAVE SCOPE    -- DoInArm_SuckerMap()                   ACTIVE   golden :965-972
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  *** THIS BODY RETIRES A KNOWN OPEN GATE -- ckernel.cpp:1033-1034 ***   //AI(W906-AMB-L2) 20260929: DONE -- ckernel.cpp:1033 gate, W7E-2 (:4439) and the csystem.cpp:4152-4153 macro seam are retired; AutoClean.cpp:135 static stub was already #if 0'd
//  ckernel.cpp:1033 reads: `#if 0 // TODO(W7-L2/INTEGRATOR): DoInArm_SuckerMap()
//  has no ported definition -- only two file-local no-op stubs elsewhere in the
//  tree.  Un-gate when the real ainarm2 body lands.`  This IS that body; the gate
//  is now expired and must be un-gated (golden ckernel.cpp:523 is a LIVE call).
//  Tracked in docs/DEVLOG.md:2726 / :2830 / :2965 / :3097.
//
//  TRAP 1 (d) -- TWO file-local `static` shadows exist and my definition CANNOT
//  displace them; the integrator must delete them or those TUs keep calling a
//  no-op even after this lands:
//     AutoClean/AutoClean.cpp:127  `static void DoInArm_SuckerMap() {}`
//        -> silently no-ops its 4 live call sites :2684 :2785 :8320 :9090
//     csystem.cpp:6492             `#define DoInArm_SuckerMap W7C2_DoInArm_SuckerMap`
//        -> macro-renames its 2 live call sites :6943 :6955 away from this body
//  (This is exactly the standing "25 macro seams" audit item.)  My definition is
//  NON-static, external linkage -- no ODR shadow created here.
//  HAND-OFF: golden declares this at ainarm2.h:148.  The port has no ainarm2.h;
//  aHotPlateSubstrate.h is its mirror (see :1075/:1143) but does NOT yet declare
//  DoInArm_SuckerMap.  Add `extern void DoInArm_SuckerMap();` there so ckernel.cpp
//  and csystem.cpp can actually see this body.  I may not edit that header.
//
//  Symbols used, all already present: DoInArm_9045_Type() (ainarm9045.h:65 --
//  NOTE its port body ainarm9045.cpp:2294 is still an empty `{}` stub, so the
//  first statement is presently inert; that is a pre-existing gap, not mine),
//  DoInArm_9045_SuckerMap() (real, ainarm9045.cpp:1040), CloseSiteState(bool)
//  (declared aHotPlateSubstrate.h:895; its CURRENT port body :1054 is the offline
//  stub `return 0;`, but sibling part _w7e_parts/00387_CloseSiteState.txt lands the
//  real golden body in this same stitch -- so by the time this body runs, both
//  iCloseSiteState and iCloseSiteStateForPick receive real values.  Re-checked
//  2026-08-11 06:46:04 +0800: `ls _w7e_parts | grep CloseSiteState` -> 00387),
//  iCloseSiteState / iCloseSiteStateForPick (cmydef.cpp:3403-3404).
// ---------------------------------------------------------------------------
void DoInArm_SuckerMap()
{
    DoInArm_9045_Type();                                                        //Steven 20210420 : 修正12site關site跑2x4 mode, HP=6x11 hang up
    DoInArm_9045_SuckerMap();
    iCloseSiteState=CloseSiteState(true);                                       //Eliot 2009_03_06
    iCloseSiteStateForPick=CloseSiteState(false);                               //Steven 20150924 : 隨時開關Site功能
    return;
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 1016..1023
// -----------------------------------------------------------------------------
//  ROLE        : reset the Loader-pick task cursor plus the four flags/pre-sets a fresh in-arm Loader pick needs: pick-error latch, destroy-active bookkeeping, Preciser re-arm, additional-function pre-set.
//  WAVE SCOPE  : InitArmPickFromLoadStageTask -- ACTIVE, all five statements, golden ainarm2.cpp:1016..1023.
//  COLLISION    : *** INTEGRATOR MUST DELETE THE EXISTING PARTIAL/STUB BODY ***
//              : aHotPlateSubstrate.cpp:1514  `void InitArmPickFromLoadStageTask() { iPickFromLoadStageTask=1; }` -- a PARTIAL translation (cursor only), not a bare stub; deleting it is what lets the other three statements run
//              : It is a NON-static definition of the same signature, so leaving
//              : it in place is a `multiple definition` link error, not a silent
//              : shadow.  Its `extern` DECLARATION stays where it is -- only the
//              : body is deleted.
//  INCLUDES     : this body needs, beyond what port ainarm2.cpp already includes
//              : (cmydef.h / MachineType.h / cprod.h / ainarm9045.h /
//              : aHotPlateSubstrate.h):
//              : ainarm9045.h (CheckInArmDestroyActiveFinishInitial :112, PreSetInArmAdditionalFlag :125) -- already included by port ainarm2.cpp
//  GATE REGISTER: EMPTY -- zero #if 0 in this part.  There WAS a gate W7E-K8-B on
//              : golden :1021 (bDoPreciser) and it was RETIRED BEFORE SHIPPING; the
//              : statement is ACTIVE and now carries a HARD DEPENDENCY note instead.
//              : Full reasoning, both timestamps, and the one line the integrator must
//              : add are inline at that statement.  ONE ACTION REQUIRED OF THE
//              : INTEGRATOR: add `bool bDoPreciser=false;` (golden ainarm2.cpp:50) to
//              : the ainarm2 globals block, or this part and sibling part 01207 both
//              : fail to compile.
//  ACTIVE DEPS  : iPickFromLoadStageTask (real), bInArmPickErrFromLoader cmydef.h:4332,
//              : CheckInArmDestroyActiveFinishInitial ainarm9045.h:112 -- its body at
//              : ainarm9045.cpp:2307 is itself an empty `{}`, a pre-existing stub in
//              : ANOTHER file and NOT this part's business; the call is faithful --
//              : and PreSetInArmAdditionalFlag ainarm9045.h:125, real body
//              : ainarm9045.cpp:1252.
//  BEHAVIOUR    : deleting the partial body above turns three dropped statements back
//              : on.  PreSetInArmAdditionalFlag in particular has a real body, so the
//              : in-arm additional-function flags (Precisor / Rotator / Bottom CCD /
//              : Die Clean) start being pre-set once per Loader pick.  Intended.
// =============================================================================
void InitArmPickFromLoadStageTask()
{
    iPickFromLoadStageTask=1;
    bInArmPickErrFromLoader=false;                                              //Steven 20170828 (wei) : 確認是否有在Loader吸料異常
    CheckInArmDestroyActiveFinishInitial();
//------------------------------------------------------------------------------
//  HARD DEPENDENCY, NOT A GATE -- golden ainarm2.cpp:1021 stays ACTIVE.
//  The next statement clears bDoPreciser so the following Loader pick re-arms the
//  InArm Preciser station (golden's own comment: Frank 20180410 (Steven)).
//
//  bDoPreciser is a FILE-SCOPE GLOBAL OF GOLDEN ainarm2.cpp -- :50
//  `bool bDoPreciser=false;', declared golden ainarm2.h:58 -- so it is the
//  integrator's to add in ONE place and this agent must not emit it.  It is missing
//  from the port today.  Command, run from D:/HT9045/HT9011UC_Cpp_V3.33.906.0 at
//  2026-08-11 06:33:20 local:
//      rg -n --no-messages -g '*.cpp' -g '*.h' 'bDoPreciser' .
//  -> exactly 2 hits, both in ainarm9045.cpp and both inert: :1135 is prose inside a
//  comment, and :1208 `bDoPreciser=true;' sits inside the `#if 0 // TODO(W7)' block
//  opened at ainarm9045.cpp:1192.
//
//  WHY THIS IS *NOT* GATED (I wrote it as a gate first, then removed the gate):
//    * The task's own rule for this file is explicit -- the ~37 ainarm2 file-scope
//      globals the port lacks get added by the integrator in one place, and parts
//      "just USE them and assume they exist".  bDoPreciser is exactly that species;
//      the enumerated line list simply skips golden :50.  INTEGRATOR: please add
//      `bool bDoPreciser=false;' to that same block.  It is one line.
//    * It is no longer only my problem.  Re-checked at 2026-08-11 06:43 local:
//      sibling part _w7e_parts/01207_CheckInArmSuckInitial.txt READS bDoPreciser
//      UN-GATED at its own line 66 (golden ainarm2.cpp:1234,
//      `if(bDoPreciser==false && ...'), and it emits no declaration either.  So the
//      global is already forced by another part of this same wave: gating my write
//      would not save the stitch, it would only make MY statement silently vanish
//      while the sibling's read still demanded the symbol -- i.e. a gate that is
//      expired the moment it lands (the campaign already carries 50 of those).
//    * TRAP 2's warning applies to the "happens to be equivalent" excuse as well:
//      today golden's only reader is untranslated, so dropping the write changes
//      nothing observable -- and that is precisely NOT a reason to drop it.
//  FAILURE MODE IF THE GLOBAL IS FORGOTTEN: a compile error, `bDoPreciser was not
//  declared in this scope', at this line and at sibling 01207's line 66.  Loud, at
//  the exact spot, one line to fix.  That is the intended outcome, not an accident.
//  REAL-MACHINE MEANING of the statement: golden re-arms the Preciser on every fresh
//  Loader-pick task; a flag left set makes a later pick skip the Preciser station
//  (golden :1234 and golden ainarm9045.cpp:3146) or repeat it.
    bDoPreciser=false;                                                          //Frank 20180410 (Steven) : InArm Preciser Station
    PreSetInArmAdditionalFlag();
}

// =============================================================================
//  MoveInArmZToPlateSafe(int iTask)   golden ainarm2.cpp:1025-1138
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- the in-arm Z 'all nozzles at plate-safe height' mover+verifier that every
//    in-arm SM polls before an X/Y move.  Walks the motor grid commanding each Z to
//    Prod.ZInArmSafe[i][j]; on a FAILED command it runs the JerryYang 20200924
//    lost-step check (home LED on while ReadPos < -200 -> log/alarm + SetInArmHome), and
//    on a SUCCESSFUL one the Steven 20220522 over-travel check (pos > target+100 ->
//    alarm + SetInArmHome).  Then InArmZSafe(DETECT_ALL_FLAG) behind a 50-poll retry
//    ladder with a 3-strike SetInArmHome before WAR0157; clears fCMD on every Z; finally
//    re-confirms with InArmZSafe(DETECT_SENSOR_FLAG).  iTask is golden's unused caller tag.
//
//  WAVE SCOPE -- MoveInArmZToPlateSafe (golden :1025-1138) ACTIVE, 1 gate.
//
//  GATE REGISTER
//   [W7E-1] golden ainarm2.cpp:1057  MyDBIProcess("Motion", sData, "MoveInArmZToPlateSafe")
//     WHY IT SHOULD BE GATED (asked fresh, per TRAP 3 -- the answer is NOT 'the symbol
//       is missing') -- golden's MyDBIProcess is the 3-arg (table, msg, function) DB
//       logger.  The declaration visible to this file's include set is the 2-arg
//       aHotPlateSubstrate.h:924 `void MyDBIProcess(AnsiString S1, AnsiString S2)` whose
//       body is aHotPlateSubstrate.cpp:772.  A 3-arg __fastcall MyDBIProcess IS declared
//       at cMyDB.h:81 -- but its only body lives in SECSGEM/uHGemEquipment.cpp:3475,
//       i.e. in the ht9045_secsgem archive, and including cMyDB.h here would ALSO make
//       every 2-arg MyDBIProcess call in the TU ambiguous (2-param exact vs 3-param
//       with one default).  Taking a SECSGEM archive dependency plus an
//       overload-ambiguity risk to move one DB log line is the wrong trade -- exactly
//       what asortarm.cpp:2476-2480 already decided for the OutArm twin
//       MoveOutArmZToPlateSafe (golden asortarm.cpp:1936).  Stay gated.
//     OFFLINE DEFAULT + WHY FAITHFUL -- the 2-arg call with golden's 3rd argument (the
//       DB function-name column) folded into the message text.  Nothing is dropped and
//       no control flow changes: this arm is a pure logging side effect on the CC_ATEC /
//       CC_KYEC_LEE customers, which log INSTEAD of raising the operator dialog.
//       Byte-identical idiom to asortarm.cpp:2479.
//     REAL-MACHINE DIFFERENCE -- on a real machine the Process DB row lands in the
//       'Motion' table with the function name in its own column; here it arrives as one
//       string ('<motor> pos <n> home sensor on [MoveInArmZToPlateSafe]') into a no-op
//       sink.  No SM state and no alarm decision differs.
//     ABSENCE/SHAPE COMMAND (re-run at integration; TRAP 2) --
//       grep -n "MyDBIProcess" D:/HT9045/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.h
//       grep -n "MyDBIProcess" D:/HT9045/HT9011UC_Cpp_V3.33.906.0/cMyDB.h
//       expect aHotPlateSubstrate.h:924 = 2-arg, cMyDB.h:81 = 3-arg __fastcall (body in
//       SECSGEM/uHGemEquipment.cpp).  MEASURED 2026-08-11 14:52.
//
//  RETIRE-AT-INTEGRATION: `bool MoveInArmZToPlateSafe(int) { return true; }` at
//  aHotPlateSubstrate.cpp:1065 (decl aHotPlateSubstrate.h:901, plus a local extern at
//  acarry.cpp:3261) -- it answers 'Z already safe' unconditionally, and ~30 live call
//  sites across ainarm9045*.cpp / ainarm9045S_*.cpp / acarry.cpp bind to it.  TRAP-1
//  shape (b).
//  SOFT_SIMULTE -- golden's two `#ifndef SOFT_SIMULTE` blocks are preserved exactly.
//  The port does not define SOFT_SIMULTE, so both blocks are LIVE here (same as the
//  already-translated OutArm twin in asortarm.cpp:2446-2549).
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
bool MoveInArmZToPlateSafe(int iTask)
{
    bool OK=true;
    int iAlarmSuck=0, iAlarmSuck1=0;
    static int iRetryCount=0;
    static int iRetryFail=0;
    int iMot=0;
    #ifndef SOFT_SIMULTE
    bool bHomeFlag=false;
    int iZPos=0;
    AnsiString sData="";
    #endif

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iMot=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
            if(MOT[iMot].MotorMove(Prod.ZInArmSafe[i][j])==false)
            {
                OK=false;
                #ifndef SOFT_SIMULTE
                MOT[iMot].ScanMotorStatus();                                    //JerryYang 20200924 : Z軸失步偵測
                bHomeFlag=MOT[iMot].Led[iHomeLed];
                if(bHomeFlag)
                {
                    iZPos=MOT[iMot].ReadPos();
                    if(iZPos<-200)
                    {
                        sData.sprintf("%s pos %d home sensor on", MOT[iMot].Alias, iZPos);
                        if(CUSTOMER_CODE==CC_ATEC ||                            //Steven 20221224 : 艾科暫時不檢查Z軸Home Sensor
                           CUSTOMER_CODE==CC_KYEC_LEE)
#if 0 // GATE W7E-1 -- golden ainarm2.cpp:1057 (3-arg MyDBIProcess; see banner)
                            MyDBIProcess("Motion", sData, "MoveInArmZToPlateSafe");
#else
                            MyDBIProcess("Motion", sData+AnsiString(" [MoveInArmZToPlateSafe]"));   // GATE W7E-1: golden 3rd arg (DB function-name column) folded into the message, not dropped -- idiom of asortarm.cpp:2479
#endif
                        else
                            ShowMyMessage(sData, "", "MoveInArmZToPlateSafe");
                        SetInArmHome();
                    }
                }
                #endif
            }
            else                                                                //Steven 20220522 : 針對Z軸加上位置確認
            {
                #ifndef SOFT_SIMULTE
                iZPos=MOT[iMot].ReadPos();
                if(iZPos>Prod.ZInArmSafe[i][j]+100)
                {
                    sData.sprintf("Motor %s pos %d error!!", MOT[iMot].Alias, iZPos);
                    ShowMyMessage(sData, "");
                    SetInArmHome();
                }
                #endif
            }
        }
    }

    if(OK)
    {
        iAlarmSuck=InArmZSafe(DETECT_ALL_FLAG);
        if(iAlarmSuck!=-1)
        {
            if(iRetryCount>50)
            {
                iRetryCount=0;
                if(iRetryFail<3)
                {
                    iRetryFail++;
                    SetInArmHome();
                }
                else
                {
                    ShowErrorMessage("WAR0157", 0, MInArmX, false, MOT[iAlarmSuck].NumberAlias);                        //入料手臂 Z 沒有在歸零的位
                    iRetryFail=0;
                }
                return false;
            }
            else
            {
                iRetryCount++;
                return false;
            }
        }
        iRetryFail=0;
        iRetryCount=0;
    }

    if(OK)
    {
        for(int i=0; i<InArmSuck.iMotRow; i++)
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                iMot=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
                MOT[iMot].fCMD=false;
            }
        }
    }

    if(OK==true && iAlarmSuck==-1)                                              //Jou 2011-02-09
    {
        iAlarmSuck1=InArmZSafe(DETECT_SENSOR_FLAG);
        if(iAlarmSuck1==-1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
//==============================================================================

// =============================================================================
//  CheckInArmSuckInitial()   golden ainarm2.cpp:1207-1263   (Steven 20100104)
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- pre-pick vacuum sanity gate for the whole in-arm nozzle grid.  Runs
//    CheckSuckInitialStatus() over iMaxRow x iMaxCol, records a WAR0132 no-save error
//    row per offending nozzle with the live X/Y motor position, and raises ONE WAR0132
//    Retry|Skip dialog naming all of them.  Preciser modes (or RT-preciser during
//    rsmContinuRetest) swallow the failure instead.  On K_SKIP every nozzle at or past
//    Vaccum_Initial_Off is logged again (saving this time) and re-typed: HAS_HOT_IC ->
//    NULL_IC, everything else -> HAS_NULL_IC.  Returns !bHasFail.
//
//  WAVE SCOPE -- CheckInArmSuckInitial (golden :1207-1263) ACTIVE, no gates.
//  GATE REGISTER -- (empty.)  Both production-record calls are REAL here:
//    AddErrorRecordNoSave has a faithful body in Public/MyProductionRecord.cpp (wired
//    into CMakeLists.txt:1472) and AddErrorRecord a declared no-op stand-in in
//    aHotPlateSubstrate.cpp; both are already called ungated elsewhere in this tree
//    (aoutarm.cpp:3096 / acarry.cpp:2440), so gating them would be the defect.
//
//  TRAP 5 -- CheckSuckInitialStatus is declared csystem.h:390 as
//    `int CheckSuckInitialStatus(class TMyKitSuck &Ptr, int iR, int iC)`, and TWO
//    different TMyKitSuck layouts exist in this tree (aHotPlateSubstrate.h:365, the one
//    177 TUs use, and mykitsuck.h:274).  I relied on the aHotPlateSubstrate.h one,
//    because the InArmSuck object passed here is defined in aHotPlateSubstrate.cpp and
//    the port ainarm2.cpp already includes aHotPlateSubstrate.h at its :37.  csystem.cpp
//    must compile its CheckSuckInitialStatus body against the SAME header or every field
//    read lands at the wrong offset while linking perfectly; aoutarm9045.cpp:929 already
//    passes OutArmSuck (same header) to it, so the convention is established.
//  SEAM NOTE -- canary_support.cpp's ShowErrorMessage returns K_RETRY unconditionally
//    unless W906_ShowErrorMessage_SimReturn is set, so the K_SKIP arm below is only
//    reachable in tests that drive that seam.  Pre-existing project-wide seam, not a
//    gate of mine; the arm is translated in full so it becomes reachable when fed.
//  RETIRE-AT-INTEGRATION: `bool CheckInArmSuckInitial() { return true; }` at
//    aHotPlateSubstrate.cpp:1588 (decl aHotPlateSubstrate.h:1087) -- 'always OK' --
//    with live call sites at ainarm9045S_1x4_4.cpp:1644 and
//    ainarm9045S_2x4_4_13.cpp:1945.  TRAP-1 shape (b).
//
//  *** INTEGRATOR ACTION REQUIRED: bDoPreciser HAS NO DEFINITION IN THE TREE ***
//    golden ainarm2.cpp:50 `bool bDoPreciser=false;` is NOT in the 37-global list I was
//    handed (:66,:67,:81,:87,:88,:93,:102,:103,:109,:110,:111,:2313,:2314,:2449,:2535,
//    :2839,:2944,:2945,:2951,:2952-:2965,:3459,:3527,:3791,:3792,:3948), i.e. it was
//    assumed already present -- it is not.  Its ONLY two occurrences tree-wide are
//    ainarm9045.cpp:1135 (a COMMENT) and ainarm9045.cpp:1208 (an assignment that sits
//    INSIDE the `#if 0 // TODO(W7)` block spanning ainarm9045.cpp:1192-1247), so there is
//    no storage and no declaration anywhere.
//    COMMAND: grep -rn "bDoPreciser" --include=*.h --include=*.cpp D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//    -> 2 hits, both above; 0 in any header.  MEASURED 2026-08-11 15:24.
//    I therefore emit only an `extern` DECLARATION below (repeating an extern is legal, so
//    a sibling part may carry the same line without collision) and NOT the definition,
//    per the 'file-scope globals are not yours' rule.  If nobody adds
//    `bool bDoPreciser=false;` the failure is a loud undefined-reference naming it -- not
//    a silent behaviour change.  Add it with the other 37.
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
extern bool bDoPreciser;                                                        // golden ainarm2.cpp:50 -- DECLARATION ONLY (see banner: definition missing tree-wide)
bool CheckInArmSuckInitial()                                                    //Steven 20100104 : 將所有錯誤整合只顯示一次
{
    int ret=0;                                                                  //Jimmychiu 20230307 initialized in the constructor
    int flag[MAX_ARM_Row][MAX_ARM_Col]={{0, 0, 0, 0}, {0, 0, 0, 0}};
    bool bHasFail=false;
    int iXpos, iYpos;
    AnsiString errSuck="at in arm Suck";
    ZeroMemory(flag, sizeof(flag));

    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            flag[i][j]=CheckSuckInitialStatus(InArmSuck, i, j);
            if(flag[i][j]!=0)
            {
                iXpos=MOT[MInArmX].ReadPos();
                iYpos=MOT[MInArmY].ReadPos();
                bHasFail=true;
                errSuck+=InArmSuck.Suck[i][j].sName;
                InArmSuck.PordRec[i][j].AddErrorRecordNoSave("WAR0132", true, iXpos, iYpos);
            }
        }
    }

    if(bHasFail)
    {
        if(bDoPreciser==false &&                                                //Steven 20191113 : 使用HP當Preciser
           (TestIF_File.bEnableUsePreciser==true ||
            (TestIF_File.bEnableRTPreciser && LastSet.iRunStartMode==rsmContinuRetest)))
            bHasFail=false;
        else
            ret=ShowErrorMessage("WAR0132", K_RETRY|K_SKIP, MInArmX, false, errSuck);
    }

    if(ret==K_SKIP)
    {
        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(flag[i][j]>=Vaccum_Initial_Off)
                {
                    iXpos=MOT[MInArmX].ReadPos();
                    iYpos=MOT[MInArmY].ReadPos();
                    InArmSuck.PordRec[i][j].AddErrorRecord("WAR0132", true, iXpos, iYpos);                              //Steven 20160114 (jou) : Add Production Error Log

                    if(InArmSuck.Item[i][j]==HAS_HOT_IC)
                        InArmSuck.SetItemData(i, j, NULL_IC);
                    else
                        InArmSuck.SetItemData(i, j, HAS_NULL_IC);
                }
            }
        }
    }
    return !bHasFail;
}
//==============================================================================

//==============================================================================
//== ROLE: SetInArmHome -- arm the in-arm homing request set.  Records the current
//==       in-arm Z home-sensor status string into the process log, rewinds the
//==       arm-Z home task, raises the need-home / is-homing / pitch-home-check
//==       flags, sets or clears the precisor home-check flag from the argument,
//==       and marks every motor-grid cell as needing a Z home.  Flags only -- no
//==       motion is commanded here.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   SetInArmHome  golden ainarm2.cpp:1266-1293  -- ACTIVE
//==     (golden lines emitted VERBATIM; ZERO gates)
//==
//== GATE REGISTER: (none -- this function needed no #if 0)
//==   * sInArmZHomeState() -- REAL non-static body, Motor/mymotor.cpp:2244,
//==     declared Motor/mymotor.h:442.  It currently RETURNS THE EMPTY STRING, so
//==     the "InArm home sensor status" log line lands blank offline.  That is a
//==     PRE-EXISTING degradation owned by mymotor.cpp, NOT a gate of mine, and I
//==     deliberately did not paper over it -- golden calls it unconditionally and
//==     so does this translation.
//==   * NewRecordProcess(AnsiString,AnsiString,AnsiString) -- declared cMyDB.h:121
//==     (real body acatchtray_shims.cpp per the HOMECOMING NOTICE there).
//==   * InitDoArmZHome() -- declared csystem.h:94.
//==   * bNeedArmZHome cmydef.h:2598, bIsInArmHome cmydef.h:2599,
//==     bInArmPitchNeedHomeCheck cmydef.h:2626, bPrecisorNeedHomeCheck
//==     cmydef.h:2629, bInArmZNeedHome[MAX_ARM_Row][MAX_ARM_Col] cmydef.h:2619.
//==   * InArmSuck.iMotRow / .iMotCol -- aHotPlateSubstrate.h:419/420 (TRAP 5: the
//==     aHotPlateSubstrate.h:365 TMyKitSuck, object aHotPlateSubstrate.cpp:80 --
//==     see the part 00320 banner for the full two-header argument).
//==
//== INTEGRATION NOTE -- ARITY MISMATCH, TWO EDITS THAT MUST LAND TOGETHER:
//==   golden ainarm2.h:136 declares
//==     extern void SetInArmHome(bool bPrecisorNeedHome=false);
//==   but the PORT declares the WRONG ARITY at aHotPlateSubstrate.h:906:
//==     extern void SetInArmHome();
//==   and satisfies it with an EMPTY no-op at aHotPlateSubstrate.cpp:1067.  Every
//==   existing call site writes SetInArmHome() with no argument
//==   (ainarm9045_2x4_16.cpp:1410/1576/2029, ainarm9045_2x8_32.cpp:1909/4553,
//==   ainarm_SearchPickPlate.cpp:1165, AutoClean/AutoClean.cpp:4277 -- found with
//==   grep -rn --include=*.cpp "SetInArmHome" D:/HT9045/HT9011UC_Cpp_V3.33.906.0,
//==   ran 2026-08-11 06:20 +0800).  So if the main loop deletes only the stub and
//==   leaves the 0-argument declaration, THIS DEFINITION BECOMES A DIFFERENT
//==   OVERLOAD: it links clean, no caller ever reaches it, and every
//==   SetInArmHome() call site turns into an undefined reference.  BOTH edits are
//==   required in one commit: change aHotPlateSubstrate.h:906 to golden arity with
//==   the =false default, and delete aHotPlateSubstrate.cpp:1067.  Behaviour then
//==   genuinely changes: today homing is never actually requested; afterwards the
//==   five flags plus the whole bInArmZNeedHome grid are raised, with
//==   bPrecisorNeedHomeCheck CLEARED on the default (no-argument) path.
//==   Not static -- shape (d) avoided.
//==
//== Big5 -> UTF-8: the Chinese comment on golden :1268 is transcribed character
//== for character from cp950.  Zero U+FFFD.
//==============================================================================
void SetInArmHome(bool bPrecisorNeedHome)
{
    AnsiString sHomeState="";                                                   //Sam 20230707 : 新增 InOutArm Z Home前Home sensor 狀態
    sHomeState=sInArmZHomeState();
    NewRecordProcess("", "InArm home sensor status", sHomeState);
    InitDoArmZHome();
    bNeedArmZHome=true;
    bIsInArmHome=true;

    bInArmPitchNeedHomeCheck=true;

    if(bPrecisorNeedHome)                                                       //JerryYang 20250120 : precisor homing
    {
        bPrecisorNeedHomeCheck=true;
    }
    else
    {
        bPrecisorNeedHomeCheck=false;
    }

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            bInArmZNeedHome[i][j]=true;
        }
    }
}

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:1295-1308 -----------------------------
//  ROLE          -- pure predicate: "is the in-arm ALREADY parked at its XY safe
//                   (wait) position?"  Both X and Y must be out of in-position
//                   LED state AND agree with Prod.iInArmSafeX/Y on BOTH the
//                   encoder (gap 9) and the command pulse (gap 2).  No motion, no
//                   state written.
//  WAVE SCOPE    -- IsMoveInArm2XYToWait()                ACTIVE   golden :1295-1308
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  FIDELITY NOTES
//   * The two tolerance literals are load-bearing and ASYMMETRIC: encoder gap 9
//     (Steven 20250605 widened it from 2), command gap 2 (JerryYang 20250312).
//     Kept exactly; do not "harmonise" them.
//   * CompareEncoderPos/CompareCommandPos return int and golden compares ==1 --
//     NOT a bool coercion.  Kept.
//  INTEGRATE-STEP NOTE: acatchtray_shims.cpp:113 currently holds the offline stub
//  `bool IsMoveInArm2XYToWait() { return true; }` (declared acatchtray_shims.h:364)
//  -- REMOVE it as part of the stitch.  Note the stub answered TRUE ("already at
//  wait"), so this real body is a BEHAVIOUR CHANGE for every existing caller, not
//  a no-op swap: offline MOT[] positions will usually make it FALSE.  That is the
//  faithful answer, but it will move ctest outcomes -- measure, do not assume.
//  Symbols used, all already present: MOT[] + Led[] + CompareEncoderPos/
//  CompareCommandPos (Motor/mymotor.h:258-259, :176), iInposLed (Motor/HTMotor.h:49,
//  reached via Motor/mymotor.h:66), MInArmX/MInArmY (cmydef.h:2101-2102),
//  Prod.iInArmSafeX/Y (cprod.h:404-405, object cprod.h:1138).
// ---------------------------------------------------------------------------
bool IsMoveInArm2XYToWait()
{
    if(MOT[MInArmX].Led[iInposLed]==false &&
       MOT[MInArmY].Led[iInposLed]==false &&
       MOT[MInArmX].CompareEncoderPos(Prod.iInArmSafeX, 9)==1 &&                //Steven 20240110 : fixed for ART hang up   //Steven 20250605 : in arm 到位卡控 2 --> 9
       MOT[MInArmY].CompareEncoderPos(Prod.iInArmSafeY, 9)==1 &&                //Steven 20241130 : CompareCommandPos --> CompareEncoderPos
       MOT[MInArmX].CompareCommandPos(Prod.iInArmSafeX, 2)==1 &&
       MOT[MInArmY].CompareCommandPos(Prod.iInArmSafeY, 2)==1)                  //JerryYang 20250312 : 避免encoder在安全位置但Command pulse已經被改變
    {
        return true;
    }

    return false;
}

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:1310-1372 -----------------------------
//  ROLE          -- drives the in-arm XY to its safe/wait position.  Four early
//                   refusals first (device-remain tray check in progress; a
//                   pending alarm-triggered servo-off on the in-arm; the E61
//                   "stand-by on loader" config during one-cycle-to-loader), then
//                   an early SUCCESS if the tray-arm is an under-conveyor type or
//                   the arm is already parked.  Otherwise it samples the current
//                   pitch-motor positions into iXVariable[]/iYVariable (which of
//                   the 4 X pitch axes are read depends on USE_IN_OUT_ARM_Y_PITCH),
//                   clears iInArmWaitPosition to 0 (=Wait) and issues the
//                   continuous XY move.
//  WAVE SCOPE    -- MoveInArm2XYToWait()                  ACTIVE   golden :1310-1372
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  FIDELITY NOTES
//   * bMoveInArm2XYToWaitOk is golden's "you must reach the safe position before
//     you may go anywhere else" latch (Steven 20131025).  It is set true on BOTH
//     success returns and cleared at :1365 immediately before the move is issued;
//     the three refusal returns leave it untouched.  Order preserved exactly.
//   * The three ZeroMemory() calls stay (they define the array contents the move
//     call reads); ZeroMemory reaches this TU via MachineType.h:6 -> <windows.h>.
//   * bZFlag/iZPos are zeroed and passed straight through with ZNeedDown=false --
//     i.e. XY only, Z untouched.  The 7-arg call matches Motor/mymotor.h:396
//     (8th parameter bLoader defaults to false).  No integer arithmetic here, so
//     no int-division hazard.
//   * `IniConfig.bE61InArmStandbyPosOnLoader && bOneCycleInArmToLoader` sits
//     OUTSIDE the bMoveInArm2XYToWaitOk latch block (JerryYang 20260129 moved it
//     out deliberately) -- kept at golden's nesting level.
//  INTEGRATE-STEP NOTE: TWO offline stubs for this symbol must go:
//     acatchtray_shims.cpp:110 `bool MoveInArm2XYToWait() { return true; }`
//     (declared acatchtray_shims.h:361 AND again at atester_shims.h:282)
//  Same warning as IsMoveInArm2XYToWait: the stub said TRUE unconditionally, so
//  landing the real body CHANGES observable behaviour for aTester_*/acatchtray
//  callers.  Measure, do not assume.
// ---------------------------------------------------------------------------
//AI(k3-ainarm2) 20260811: HOIST ME to ainarm2.cpp's include head.
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "FormsFacade.h"
// -----------------------------------------------------------------------------
//  DECLARATION HOISTS (declarations, NOT definitions, NOT globals, NOT gates).
//  Both are needed to COMPILE this part; both symbols LINK fine already.
//   (1) iInArmWaitPosition (golden :1364) -- real external definition at
//       acatchtray.cpp:135 (`int iInArmWaitPosition = 0;`) but NO `extern` in any
//       port header: the single .h hit is PROSE inside a comment
//       (acatchtray_shims.h:435).  Verified 2026-08-11 06:24 +0800 with
//         grep -rn --include=*.h -w iInArmWaitPosition .
//       TRAP 3 applied: the answer to "no declaration" is to DECLARE, not to gate --
//       gating would throw away a real state write over a missing prototype.
//       Sibling part 01482_MoveInArm2XYToDecayTeach.txt reached the SAME conclusion
//       independently and uses the guard macro below -- I reuse its EXACT name so
//       the two blocks are idempotent in any stitch order.
//   (2) IsMoveInArm2XYToWait (golden :1342) -- my own sibling part 01295 defines it;
//       the port's only declaration is acatchtray_shims.h:364, which I do not pull
//       in (it drags SECSGEM headers).  Guard name reused verbatim from sibling part
//       00133_CheckInArmFinishAllPickerAction.txt, same reason.
//  INTEGRATOR: hoist both lines into the stitched file's declaration block and
//  delete them from here.
#ifndef HT9045_W7E_K8_iInArmWaitPosition_DECLARED
#define HT9045_W7E_K8_iInArmWaitPosition_DECLARED
extern int iInArmWaitPosition;
#endif
#ifndef HT9045_IsMoveInArm2XYToWait_DECLARED
#define HT9045_IsMoveInArm2XYToWait_DECLARED
bool IsMoveInArm2XYToWait();                                                    // golden ainarm2.h:138 (body golden ainarm2.cpp:1295)
#endif
// ---------------------------------------------------------------------------
bool MoveInArm2XYToWait()
{
    int iXVariable[X_PITCH_COUNT];
    int iYVariable=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(bMoveInArm2XYToWaitOk==true)                                             //Steven 20131025 : 要先動到安全位置才可以再到別的地方
    {
        if(bDoTrayDeviceCheck)                                                  //wei 20170317 (steven) Device Remain 殘料檢測
        {
            return false;
        }

        if(IniConfig.bAlarmNeedServoOff)                                        //Steven 20111226 : In Arm 咖啡杯解除
        {
            if(fNote->bMyServoOffInArm)
            {
                return false;
            }
        }
    }

    if(IniConfig.bE61InArmStandbyPosOnLoader && bOneCycleInArmToLoader)         //JerryYang 20260129 : 移到外面  //JerryYang 20200206 one cycle, clean out, tray feed時in arm移動到loader
    {
        return false;
    }

    if(TRAY_ARM_MODE==eUnderCoveyor ||
       IsMoveInArm2XYToWait())
    {
        bMoveInArm2XYToWaitOk=true;                                             //Steven 20131025 : 要先動到安全位置才可以再到別的地方
        return true;
    }

    iXVariable[0]=MOT[MInArmPitch].ReadPos();
    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||
       USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                            //Ztex 2024.02.24 Add HT-1132
    {
        iXVariable[1]=MOT[MInArmPitchX2].ReadPos();
        iYVariable   =MOT[MInArmPitchY].ReadPos();
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                         //Steven for HT1032
            USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        iXVariable[1]=MOT[MInArmPitchX2].ReadPos();
        iXVariable[2]=MOT[MInArmPitchX3].ReadPos();
        iXVariable[3]=MOT[MInArmPitchX4].ReadPos();
        iYVariable   =MOT[MInArmPitchY].ReadPos();
    }

    iInArmWaitPosition=0;                                                       //Ifor 20191105 : add InArm 安全位置判斷 0:Wait 1:LoaderWait 2:DecayWait 3:Shuttle2
    bMoveInArm2XYToWaitOk=false;                                                //Steven 20131025 : 要先動到安全位置才可以再到別的地方
    if(InArmContinuousMove_9045(Prod.iInArmSafeX, Prod.iInArmSafeY, iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        bMoveInArm2XYToWaitOk=true;                                             //Steven 20131025 : 要先動到安全位置才可以再到別的地方
        return true;
    }
    return false;
}

// =============================================================================
//  IsMoveInArm2XYToShuttle2Wait()   golden ainarm2.cpp:1428-1438   (Sam 20250522)
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- pure position predicate: is the in-arm ALREADY parked exactly on the
//    shuttle-2 place point (X and Y both equal to Prod.XInArm_Shuttle2_Place /
//    Prod.YInArm_Shuttle2_Place at the current [iInArmYBase][iInArmXBase] cell)?
//    Callers use it to avoid re-issuing the move while a residual-material check runs.
//
//  WAVE SCOPE -- IsMoveInArm2XYToShuttle2Wait (golden :1428-1438) ACTIVE, no gates.
//  GATE REGISTER -- (empty.)
//
//  RETIRE-AT-INTEGRATION: `bool IsMoveInArm2XYToShuttle2Wait() { return false; }` at
//  acatchtray_shims.cpp:114 (decl acatchtray_shims.h:368) -- 'never at the wait point'.
//  TRAP-1 shape (b).  The two are NOT behaviourally equivalent offline: the sim motors
//  DO reach commanded positions, so the real body can return true where the stub never
//  did -- which is the point of translating it.
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
bool IsMoveInArm2XYToShuttle2Wait()                                             //Sam 20250522 : 增加保護，避免已經在跑殘料檢測時 InArm 在干涉區
{
    int iX=0, iY=0;
    iX=MOT[MInArmX].ReadPos();
    iY=MOT[MInArmY].ReadPos();
    if(iX==Prod.XInArm_Shuttle2_Place[iInArmYBase][iInArmXBase] && iY==Prod.YInArm_Shuttle2_Place[iInArmYBase][iInArmXBase])
    {
        return true;
    }
    return false;
}
//==============================================================================

//==============================================================================
//  MoveInArm2XYToShuttle2Wait()  --  golden ainarm2.cpp:1440-1480
//
//  ROLE          Drive the in-arm XY (plus every pitch axis the machine has) to
//                the Shuttle-2 PLACE teach point as a "wait" pose: sample the
//                current pitch positions, mark iInArmWaitPosition=3, and hand the
//                whole move to InArmContinuousMove_9045 with Z NOT commanded down.
//  WAVE SCOPE    MoveInArm2XYToShuttle2Wait()   ACTIVE   golden :1440-1480
//  GATE REGISTER (none -- every symbol has a real linkable home)
//                  IniConfig.bAlarmNeedServoOff  Config.h; fNote forms/fNote.h:169
//                    (bMyServoOffInArm forms/fNote.h) -- this deref is inside a
//                    runtime function, so TRAP 4 (a static-init ctor touching a
//                    NULL global) does not apply: this file adds NO file-scope
//                    object at all.
//                  MOT[] Motor/mymotor.h:385 (TTrayMotor; ReadPos :171);
//                    MInArmPitch/MInArmPitchX2..X4/MInArmPitchY cmydef.h.
//                  USE_IN_OUT_ARM_Y_PITCH + iXYPitch* cmydef.h/cmydef.cpp.
//                  InArmContinuousMove_9045 Motor/mymotor.h:396.
//                  Prod.XInArm_Shuttle2_Place/YInArm_Shuttle2_Place cprod.h.
//                  X_PITCH_COUNT / MAX_ARM_Row / MAX_ARM_Col MachineType.h/cprod.h.
//
//  RETIRES A STUB: acatchtray_shims.cpp:111 `bool MoveInArm2XYToShuttle2Wait()
//    { return true; }` (decl acatchtray_shims.h:362) must be REMOVED by the
//    integrator.  NOT written `static` here (TRAP 1 shape (d) against that
//    non-static declaration).
//  ** LANDING HAZARD, MEASURE BEFORE COMMITTING ** Motor/mymotor.cpp:2211 is
//    itself still `bool InArmContinuousMove_9045(...) { return false; }`.  So
//    retiring the `{return true;}` shim flips 9 live call sites from "arrived"
//    to "never arrives".  That is the faithful direction (the sim motor really
//    has not arrived), but it can hang an in-arm SM in ctest.  Same family as
//    open task #10 (sim-motor un-gate).  Do NOT paper over it here.
//==============================================================================
// iInArmWaitPosition -- golden DECLARATION ainarm2.h:219; the port's single
// DEFINITION is acatchtray.cpp:135 (`int iInArmWaitPosition = 0;`).  No ported
// header exposes it (asendic_Loader.cpp:233 carries its own identical local
// extern for exactly this reason), so a declaration -- never a definition -- is
// forward-declared here.  #ifndef-guarded so a sibling part's identical decl in
// the stitched TU is harmless.  NOT one of the 37 integrator-owned file-scope
// globals: this is a declaration, so it cannot cause `multiple definition`.
#ifndef HT9045_W7E_FWD_iInArmWaitPosition
#define HT9045_W7E_FWD_iInArmWaitPosition
extern int  iInArmWaitPosition;                                                 //Ifor 20191105 : add In Arm 目前移動的等待位置 0:Wait 1:LoaderWait 2:DecayWait 3:Shuttle2 // golden ainarm2.h:219
#endif
bool MoveInArm2XYToShuttle2Wait()
{
    int iXVariable[X_PITCH_COUNT];
    int iYVariable=0;
    bool bZFlag[MAX_ARM_Row][MAX_ARM_Col];
    int iZPos[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iXVariable, sizeof(iXVariable));
    ZeroMemory(bZFlag, sizeof(bZFlag));
    ZeroMemory(iZPos, sizeof(iZPos));

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20111226 : In Arm 咖啡杯解除
    {
        if(fNote->bMyServoOffInArm)
        {
            return false;
        }
    }

    iXVariable[0]=MOT[MInArmPitch].ReadPos();
    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||
       USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                            //Ztex 2024.02.24 Add HT-1132
    {
        iXVariable[1]=MOT[MInArmPitchX2].ReadPos();
        iYVariable   =MOT[MInArmPitchY].ReadPos();
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                         //Steven for HT1032
            USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        iXVariable[1]=MOT[MInArmPitchX2].ReadPos();
        iXVariable[2]=MOT[MInArmPitchX3].ReadPos();
        iXVariable[3]=MOT[MInArmPitchX4].ReadPos();
        iYVariable   =MOT[MInArmPitchY].ReadPos();
    }

    iInArmWaitPosition=3;                                                       //Ifor 20191105 : add InArm 安全位置判斷 0:Wait 1:LoaderWait 2:DecayWait 3:Shuttle2
    if(InArmContinuousMove_9045(Prod.XInArm_Shuttle2_Place[iInArmYBase][iInArmXBase], Prod.YInArm_Shuttle2_Place[iInArmYBase][iInArmXBase], iXVariable, iYVariable, bZFlag, iZPos, false))
    {
        return true;
    }
    return false;
}

// =============================================================================
//  InitInArmPlaceToHotPlateTask()   golden ainarm2.cpp:1539-1542
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- resets the in-arm place-to-hot-plate task cursor to step 1.
//
//  WAVE SCOPE -- InitInArmPlaceToHotPlateTask (golden :1539-1542) ACTIVE, no gates.
//  GATE REGISTER -- (empty.)
//
//  *** COLLISION WARNING -- THIS ONE IS NOT A STUB ***  aHotPlateSubstrate.cpp:1488
//  already carries a FAITHFUL, behaviourally identical body of this function
//  (`void InitInArmPlaceToHotPlateTask() { iInArmPlaceToHotPlateTask=1; }`), sitting
//  next to its golden :1544 / :1549 siblings (:1489/:1490) and the
//  iInArmPlaceToHotPlateTask definition (:1115).  Landing this part file WITHOUT
//  deleting that body is a `multiple definition` link error, and deleting it buys
//  nothing behaviourally.  RECOMMENDATION: integrator DROPS this part (keep the
//  existing body where its two siblings live) or moves all three siblings together.
//  It is emitted only because the task list named golden :1539.
// =============================================================================
void InitInArmPlaceToHotPlateTask()
{
    iInArmPlaceToHotPlateTask=1;
}
//==============================================================================

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 01544 : InitInArmPlaceToHotPlateTask400
//  Golden: ainarm2.cpp:1544-1547 (cp950, CRLF).
//
//  ROLE          : parks the place-to-HotPlate sub-SM cursor at step 400 -- the
//                  HangUp-release entry point the variant SMs jump to when the operator
//                  clears a HotPlate place hang up.
//  WAVE SCOPE    : InitInArmPlaceToHotPlateTask400()  golden ainarm2.cpp:1544-1547  ACTIVE
//  GATE REGISTER : (none)  iInArmPlaceToHotPlateTask is declared aHotPlateSubstrate.h
//                  and defined in aHotPlateSubstrate.cpp.
//  INTEGRATOR    : *** DUPLICATE *** aHotPlateSubstrate.cpp:1489 ALREADY carries a
//                  byte-identical faithful body:
//                    void InitInArmPlaceToHotPlateTask400() { iInArmPlaceToHotPlateTask=400; }
//                  Landing this part WITHOUT deleting that line is a `multiple
//                  definition` link error.  Golden's home is THIS file, so the
//                  aHotPlateSubstrate.cpp line is the one to delete -- but it is a
//                  pure duplicate either way: NO behaviour change.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0
//==============================================================================
//==============================================================================
void InitInArmPlaceToHotPlateTask400()                                          //ChungHung 20120502
{
    iInArmPlaceToHotPlateTask=400;
}

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:1549-1552 -----------------------------
//  ROLE          -- parks the in-arm "place to HotPlate" task cursor at step 100
//                   (the mid-sequence re-entry point, as opposed to 1 = restart
//                   and 400 = ChungHung's 20120502 hang-up escape).  Pure cursor
//                   write, no HAL.
//  WAVE SCOPE    -- InitInArmPlaceToHotPlateTask100()     ACTIVE   golden :1549-1552
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  *** INTEGRATE-STEP COLLISION -- READ BEFORE STITCHING ***
//  aHotPlateSubstrate.cpp:1490 ALREADY carries a byte-equivalent real body
//  ("void InitInArmPlaceToHotPlateTask100() { iInArmPlaceToHotPlateTask=100; }
//  // golden :1549"), declared aHotPlateSubstrate.h:1084, sitting next to its
//  :1488/:1489 siblings for golden :1539/:1544.  Landing this part unchanged gives
//  TWO external definitions => `multiple definition` at link.
//  ACTION FOR THE INTEGRATOR: DELETE aHotPlateSubstrate.cpp:1488-1490 (ALL THREE)
//  and keep this one.  My first draft hedged here ("this would orphan the :1539/:1544
//  siblings"); that hedge is now VOID -- a sibling-part re-check shows
//  _w7e_parts/01539_InitInArmPlaceToHotPlateTask.txt and
//  _w7e_parts/01544_InitInArmPlaceToHotPlateTask400.txt bring golden :1539 and :1544
//  home to ainarm2.cpp in this same stitch, so the whole 3-function family lands
//  together and aHotPlateSubstrate.cpp:1488-1490 all become duplicates.
//  Measured 2026-08-11 06:24 +0800:
//    grep -n "InitInArmPlaceToHotPlateTask" aHotPlateSubstrate.cpp aHotPlateSubstrate.h
//  Sibling-part cross-check re-run 2026-08-11 06:46:04 +0800:
//    ls _w7e_parts | grep InitInArmPlaceToHotPlateTask  -> 01539, 01544, 01549 (mine)
//  Symbol used: iInArmPlaceToHotPlateTask -- already in the port at
//  aHotPlateSubstrate.cpp:1115 (extern .h:1054); NOT emitted here.
// ---------------------------------------------------------------------------
void InitInArmPlaceToHotPlateTask100()
{
    iInArmPlaceToHotPlateTask=100;
}

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 01554 : RowCanDualSite
//  Golden: ainarm2.cpp:1554-1564 (cp950, CRLF).
//
//  ROLE          : asks whether ONE in-arm row may serve TWO sites on the HotPlate
//                  place path.  Read by the 2x2_8_Hot / 2x8_32 place SMs
//                  (GetPlaceHotPlate_4/_8/_8All).
//  WAVE SCOPE    : RowCanDualSite()  golden ainarm2.cpp:1554-1564  ACTIVE
//  GOLDEN BUG (kept) : three of the four exits return false and the fourth is
//                  `if(HotPlateForm.XDivision==6) return false;` immediately followed by
//                  `return false;` -- i.e. the XDivision==6 test is dead code and the
//                  ONLY true-returning path is ArmCanSuck4IC(0)==false.  Golden line
//                  order, the dead test and both returns are preserved verbatim.
//  GATE REGISTER : (none)  ArmCanSuck4IC(int,bool) csystem.h:162, body csystem.cpp:20840;
//                  TestIF cprod.h:2576 ; QualSite1X4/_8Site1X4 MachineType.h:444/:460 ;
//                  HotPlateForm cprod.h:1361.
//  INTEGRATOR    : RETIRE `bool RowCanDualSite() { return false; }` at
//                  aHotPlateSubstrate.cpp:1061.  BEHAVIOUR CHANGE -- the stub is NOT
//                  equivalent: golden returns TRUE whenever ArmCanSuck4IC(0)==false,
//                  which the stub answered false.  The stub comment at
//                  aHotPlateSubstrate.cpp:1055-1060 already flagged exactly this.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem.h"
//==============================================================================
bool RowCanDualSite()
{
    if((TestIF.iTestMode==QualSite1X4 || TestIF.iTestMode==_8Site1X4) && TestIF.iUseSuckMode==4)                        //ChungHung 20150528 add for 海思 _8Site1x4
        return false;
    if(ArmCanSuck4IC(0)==false)
        return true;
    if(HotPlateForm.XDivision==6)
        return false;

    return false;
}

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 01566 : InitInArmTask
//  Golden: ainarm2.cpp:1566-1618 (cp950, CRLF).
//
//  ROLE          : the master in-arm SM RESET.  Clears the shake / knock / continuous-
//                  rotate flags and the No9 placement action, then rewinds iArmTask to 1
//                  -- EXCEPT for three keep-your-place cases: (a) the Auto-In-Shuttle-
//                  latch No9 hand-off at iArmTask==2000 with the place-to-shuttle cursor
//                  at 1400/2400, (b) P17 "full pick from Loader" where a half-picked arm
//                  (iArmTask==100 && InArmSuck.HasIC()) rewinds only the Loader pick sub-SM
//                  and a loaded rotate station (iArmTask==5100) restarts the rotate sub-SM,
//                  and (c) bPlaceShuttle && iArmTask==2000 (mid place-to-shuttle) in both
//                  the P17 and the plain path.
//  WAVE SCOPE    : InitInArmTask()  golden ainarm2.cpp:1566-1618  ACTIVE
//  GATE REGISTER : (none -- every symbol resolves to a real port declaration)
//                  bShuttleShake csystem_shims.h:73 ; bShuttleKnock ainarm9045_w7_shims.h:62 ;
//                  bContinusRotate cmydef.h:3404 ; fMain->cInplace forms/fMain.h:199 with
//                  bNo9Action :106 and InArmPlacementEnable() :107 ; iArmTask
//                  aHotPlateSubstrate.h:936 ; iInArmPlaceToShuttleTask :1055 ;
//                  iInArmInArmCheckShtFloatTask ainarm9045.h:129 ; In_Shuttle_Auto_Latch
//                  cmydef.h:5853 ; eInSHAutoLtc MachineType.h:1513 ;
//                  IniConfig.bP17InArmFullPickFromLoader Config.h:1401 ; InArmSuck.HasIC()
//                  aHotPlateSubstrate.h:462 ; InitArmPickFromLoadStageTask
//                  aHotPlateSubstrate.h:1085 ; MOT[MInRotateKit].HasIC() Motor/mymotor.h:353
//                  + cmydef.h:2144 ; InitialInArmRotateKIT RotateKit/aRotateKIT.h:61, body
//                  RotateKit/aRotateKIT_In.cpp:425 ; bPlaceShuttle aHotPlateSubstrate.h:1098.
//  OFFLINE NOTE  : the facade's InArmPlacementEnable() returns false (forms/fMain.h:107),
//                  so the inner `bNo9Action=true` at golden :1581 cannot fire offline.  That
//                  is the PRE-EXISTING facade decision documented at forms/fMain.h:100-106,
//                  NOT a gate of mine -- the call is translated ACTIVE and un-degraded.
//  INTEGRATOR    : RETIRE the no-op stub `void InitInArmTask()  {}` at
//                  aHotPlateSubstrate.cpp:1062.  BEHAVIOUR CHANGE and it is the faithful
//                  direction: the stub never rewound iArmTask, so acarry.cpp:2803 /
//                  acatchtray.cpp:7920 / ainarm9045.cpp:970 and my own
//                  DisableAutoSiteMapWhenCleanOut (part 02786) all silently did nothing.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0 (Chinese comments at golden :1568/:1573/:1584/
//                  :1586/:1591/:1595 transcribed character-for-character)
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem_shims.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "ainarm9045_w7_shims.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "FormsFacade.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "RotateKit/aRotateKIT.h"
//==============================================================================
void InitInArmTask()
{
    bShuttleShake=false;                                                        //ChungHung 20120612 hangUp 解除
    bShuttleKnock=false;
    bContinusRotate=false;
    fMain->cInplace->bNo9Action=false;                                          //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.

    if(iArmTask==2000                 &&  //吸到一半將吸起IC做完                //KenHsieh 20251106 : 不論P17 是否開啟都應優先判斷 //KenHsieh 20221102 : Fix Pickup Error Placement
      (iInArmPlaceToShuttleTask==1400 ||
       iInArmPlaceToShuttleTask==2400))
    {
        if(iInArmInArmCheckShtFloatTask==2100 &&
           fMain->cInplace->InArmPlacementEnable())
        {
            if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                             //KenHsieh 20251105 : Auto In shuttle latch combine No9 func.
                fMain->cInplace->bNo9Action=true;
        }
        else if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                iInArmInArmCheckShtFloatTask==3000)                             //KenHsieh 20251119 : 回Home 後能繼續做In shuttle latch
        {
            ;//續做Auto In shuttle Latch 流程
        }
    }
    else if(IniConfig.bP17InArmFullPickFromLoader)                              //Steven 20111026 : In Arm Full Pick from Loader Tray
    {
        if(iArmTask==100 && InArmSuck.HasIC())                                  //如果In Arm吸到一半被中斷,要回來繼續吸
        {
            InitArmPickFromLoadStageTask();
        }
        else if(iArmTask==5100 && MOT[MInRotateKit].HasIC())                    //2013-04-12    Dell :旋轉站; 做rotate中途被歸零
        {
            InitialInArmRotateKIT();
            bContinusRotate=true;
        }
        else if(bPlaceShuttle==true && iArmTask==2000)
        {
        }
        else
        {
            iArmTask=1;
        }
    }
    else
    {
        if(bPlaceShuttle==true && iArmTask==2000)
        {
        }
        else
        {
            iArmTask=1;
        }
    }
}

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:1620-1765 -----------------------------
//  ROLE          -- the in-arm TOP-LEVEL tick.  It is a gate stack, not a state
//                   machine: refuse to run until the Initial-Start Index Check is
//                   done; if a HotPlate hang-up was recorded, dump a state record,
//                   raise WAR0150, clear the hang-up and bail; if the F16
//                   shuttle-broken-sensor check is running, force both in-shuttles
//                   "can move" (to avoid manufacturing a hang-up) and bail; in QA
//                   Mode top the pick grid up with HAS_NULL_IC once the QA count is
//                   reached; run the CCD auto-alignment arbitration (tray timing /
//                   clean-kit timing) which can also swallow the tick; then, if
//                   configured to move the in-arm out of the way on an Index jam,
//                   either finish the in-flight picker action, park at the safe
//                   position, or (D43) refuse the tick while a test head is backing
//                   out with HAS_NULL_IC risk.  All surviving paths end in
//                   DoInArm_9045().
//  WAVE SCOPE    -- DoInArm()                             ACTIVE   golden :1620-1765
//                   (2 GATED symbols inside, appearing at 4 call sites: golden
//                    :1683 :1684 :1692 :1693.  Golden :1653 was gated in my first
//                    draft and is now ACTIVE again -- see [G1] below.)
//
//  GATE REGISTER  (2 LIVE entries -- every #if 0 in this part -- plus 1 RETIRED
//                  entry kept as the audit trail of a TRAP 2 catch)
//  -------------------------------------------------------------------------
//  [G1] golden :1653-1664  `if(Check_QA_ModeCount()) { ...HAS_NULL_IC top-up... }`
//       *** GATE WITHDRAWN BEFORE DELIVERY.  THE CALL IS ACTIVE.  TRAP 2 CAUGHT ***
//       I first gated this call and wrote the register entry below.  Then I re-ran
//       the absence command 11 minutes later and it was already FALSE.  Timeline:
//         06:35:23 +0800  my absence sweep: `grep -rIn --include=*.h --include=*.cpp
//                         --exclude-dir=.git --exclude-dir=tools -w
//                         "Check_QA_ModeCount" .`  from
//                         D:/HT9045/HT9011UC_Cpp_V3.33.906.0  -> 0 hits.
//                         (0 was CORRECT and also MISLEADING: sibling parts live in
//                          _w7e_parts/*.txt, which no *.h/*.cpp filter can see.)
//         06:39:05 +0800  sibling agent AI(k7-ainarm2) writes
//                         _w7e_parts/00160_Check_QA_ModeCount.txt -- a NON-static
//                         `bool Check_QA_ModeCount()`, golden :160-199, external
//                         linkage, landing in THIS SAME stitched translation unit.
//         06:46:04 +0800  re-check `grep -n "^bool Check_QA_ModeCount" _w7e_parts/*.txt`
//                         -> 00160_Check_QA_ModeCount.txt:44.  PREMISE DEAD.
//       TRAP 3 re-asked -- "why SHOULD this stay gated?"  ANSWER: it should NOT.
//         Unlike MyMessageBox (body exists but only in another archive) and
//         WriteLastDataFile (linkable but writes the real machine's
//         D:\HT9045\system\lastdata.dat), there is no second reason here:
//           - same TU, so no archive/extraction question at all;
//           - no filesystem or device side effect -- the callee's whole body is
//             wrapped in `if(IniConfig.bQAMode && LastSet.iRunStartMode==rsmQAMode)`,
//             the very same condition my call site at golden :1646 already sits
//             inside, so in any default offline config it returns false having
//             touched nothing;
//           - the behaviour it guards is a QA-lot length governor.  Gating it would
//             have meant a QA lot silently over-runs its sample size on a real
//             machine -- a genuine safety-of-result defect, not a cosmetic gap.
//       DEPENDENCY (loud, on purpose): this ACTIVE call needs part 00160 in the same
//         stitch.  Filename order puts 00160 before 01620, so the definition
//         precedes the call; a guarded forward declaration is also emitted below so
//         the part is order-independent.  If the integrator DROPS 00160, this
//         becomes an undefined-reference LINK error -- which is exactly the outcome
//         I want, because a silent false premise is the defect the trap is about.
//
//  [G2] golden :1683      `CheckInArmAutoAlignmentTrayModeBeUse(lInArmAutoAlignmentTrayTimingFlag, true)`
//  [G3] golden :1684     `CheckInArmAutoAlignmentCKModeBeUse(lInArmAutoAlignmentCKTimingFlag, true)`
//       (and the identical pair repeated at golden :1692-1693 -- same two gates,
//        second occurrence, registered here once)
//       WHY GATED: golden's homes are AutoAlignment/AutoAlignment.h:265-266 with
//         bodies AutoAlignment/AutoAlignment.cpp:8769/:8791.  The port has NO
//         AutoAlignment directory at all -- the whole CCD auto-alignment subsystem
//         is untranslated.  The only two things in the tree wearing these names are
//         TU-LOCAL `static` stand-ins invisible to this translation unit:
//           asendic_Loader.cpp:274 `static bool W7L1L_CheckInArmAutoAlignmentTray...`
//                            :304 `#define CheckInArmAutoAlignmentTrayModeBeUse ...`
//           AutoClean/AutoClean.cpp:243 `static bool CheckInArmAutoAlignmentCK...
//                            (int, bool=false) { return false; }`
//         i.e. TRAP 1 shape (e)/(d) territory -- linking against them is impossible
//         and MUST NOT be attempted from here.
//       OFFLINE DEFAULT CHOSEN: only the two CALLS are compiled out.  The declaring
//         line `bool bDoInArmAutoAlignment=false;` (:1668) and the write-back
//         `bRunInArmAutoAlignment = bDoInArmAutoAlignment;` (:1685/:1694) stay
//         ACTIVE, so both flags land on false -- BIT-IDENTICAL to what the tree's
//         own two stand-ins already return (`return false;` in both).  So this is
//         not an invented default; it is the established tree-wide answer.  I did
//         NOT gate the surrounding block, because that would also have dropped the
//         `bLoaderNeedTrayMustFinish=false;` writes and the `return;` statements --
//         real control flow with nothing absent about it.
//       REAL-MACHINE DIFFERENCE: on a machine with MACHINE_HAS_AUTO_ALIGNMENT_CCD
//         and TestIF.bEnableAutoAlignment on, the CCD alignment pass is never
//         requested and, worse, `bSet=true` means golden also CONSUMES/arms the
//         timing flag as a side effect -- so lInArmAutoAlignmentTrayTimingFlag /
//         lInArmAutoAlignmentCKTimingFlag are never advanced either.  Alignment
//         silently never runs.  Not observable offline (no CCD).
//       EXACT ABSENCE COMMAND (from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//         grep -rIn --include=*.h --include=*.cpp --exclude-dir=.git \
//           --exclude-dir=tools -wE \
//           "CheckInArmAutoAlignment(Tray|CK)ModeBeUse" .
//         -> hits ONLY asendic_Loader.cpp (3) and AutoClean/AutoClean.cpp (2), all
//            `static`/`#define`, none a header declaration or external definition.
//         RUN AT: 2026-08-11 06:35:23 +0800.
//       TRAP 2/3 note: if a sibling lands AutoAlignment.cpp this wave, re-ask
//         whether to un-gate -- and the answer is genuinely "un-gate", because
//         unlike TRAP 3's MyMessageBox this is not a one-bool cross-archive
//         dependency: DoInArm is the primary arbiter of the alignment pass and
//         needs the real predicate, side effect included.
//  -------------------------------------------------------------------------
//
//  FIDELITY NOTES
//   * `bDoInArmAutoAlignment |= Check...` is a bitwise-OR-assign on a bool, NOT
//     `||`, so golden evaluates BOTH predicates unconditionally (both have the
//     bSet=true side effect).  Preserved as-is inside the gate text.
//   * The `IniConfig.bQAMode` branch's `int iKit32=(iKit==1)?4:0;` is integer
//     arithmetic used as a column offset into Prod.fInArmSuck4x8 -- no float
//     substitution anywhere (int division is load-bearing in this family).
//   * The empty `if(CosFunction.bQAModeUseUnloadCnt){ }` then-branch is golden's
//     own (JerryYang 20221004, Maxim QA mode) -- kept, not collapsed.
//   * The two D43 refusal ladders (:1738 inside the IndexJamInArmAway arm and
//     :1754 in the else arm) are DIFFERENT: the first additionally requires
//     LastSet.iTemperature==Tempture_Hot, the matching iWhichKitPickFor32, and
//     bPickFromLoader/bPlaceToHotplate/bDestoryOnSht all false; the second checks
//     only the two bIndexPickUpErrMoveSht flags.  Kept distinct.
//   * bDestoryOnSht is read at :1742 and its single ACTIVE definition is already
//     in THIS translation unit (port ainarm2.cpp:46).
//   * Trailing `return;` at :1764 kept.
//  INTEGRATE-STEP NOTE: csystem_shims.cpp:48 currently holds the offline no-op
//  `void DoInArm() {}` -- REMOVE it as part of the stitch.  It is a real behaviour
//  swap (no-op -> the whole in-arm tick), so re-measure ctest, do not assume.
// ---------------------------------------------------------------------------
//AI(k3-ainarm2) 20260811: HOIST ME to ainarm2.cpp's include head.
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "FormsFacade.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "ainarm_SearchPlacePlate.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "atester.h"
//  NOTE: sibling part 00160_Check_QA_ModeCount.txt proposes ONE consolidated include
//  block for the whole stitched ainarm2.cpp; the 5 lines above are a subset of it.
//  Hoist once, then delete the per-part copies (headers are guarded, so duplicates
//  are harmless until then).
// -----------------------------------------------------------------------------
//  DECLARATION HOISTS (declarations only -- no definitions, no globals, no gates).
//  All three symbols get real bodies in THIS stitched TU from sibling parts, but
//  none of them is declared in any header this TU includes, and filename stitch
//  order is the only thing that would otherwise save me.  Guard macros make these
//  idempotent with the sibling parts that emit the same declarations.
//    Check_QA_ModeCount            body: part 00160 (golden :160)   golden decl ainarm2.h:81
//    CheckInArmFinishAllPickerAction body: part 00133 (golden :133) golden decl ainarm2.h:95
//    MoveInArm2XYToWait            body: part 01310 (golden :1310, mine) golden decl ainarm2.h:146
//  (CheckInArmFinishAllPickerAction and MoveInArm2XYToWait DO have port declarations
//  in acatchtray_shims.h:361/:363 and atester_shims.h:281/:282, but those shims
//  headers also carry the no-op stub decls this stitch is retiring and drag SECSGEM
//  headers in, so a local guarded prototype is the lighter, safer choice.)
//  INTEGRATOR: hoist these into the stitched file's declaration block, delete here.
#ifndef HT9045_Check_QA_ModeCount_DECLARED
#define HT9045_Check_QA_ModeCount_DECLARED
bool Check_QA_ModeCount();                                                      // golden ainarm2.h:81  (body golden ainarm2.cpp:160)
#endif
#ifndef HT9045_CheckInArmFinishAllPickerAction_DECLARED
#define HT9045_CheckInArmFinishAllPickerAction_DECLARED
bool CheckInArmFinishAllPickerAction();                                         // golden ainarm2.h:95  (body golden ainarm2.cpp:133)
#endif
#ifndef HT9045_MoveInArm2XYToWait_DECLARED
#define HT9045_MoveInArm2XYToWait_DECLARED
bool MoveInArm2XYToWait();                                                      // golden ainarm2.h:146 (body golden ainarm2.cpp:1310)
#endif
// ---------------------------------------------------------------------------
void DoInArm()
{
    if(bInitialStartIndexCheckDone==false)                                      //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
    {
         return;
    }

    if(iHPHangUpCount!=0)                                                       //Steven 20161102 : 針對加熱盤Hang Up進行紀錄
    {
        RecordProcess("Auto State Record by in arm");
        fMain->DoStateRecord(0, false);                                         //KenHsieh 20230116 : 區分手動或自動(sbclick -> Function)
        ShowErrorMessage("WAR0150", 0, MInArmX, false, sHPHangUpFunc);
        ClearHotPlateHangUp();
        fAllMotorHome=false;
        return;
    }

    if(IniConfig.bF16CheckShuttleSensorBroken && bDoingF16)                     //Steven 20221213 : 確認shuttle 有沒有斷線
    {
        MOT[MInShuttle1].fCanMoveL=true;                                        //避免造成hang up
        MOT[MInShuttle2].fCanMoveL=true;
        MOT[MInShuttle1].fCanMoveM=true;
        MOT[MInShuttle2].fCanMoveM=true;
        return;
    }

    if(IniConfig.bQAMode==true && LastSet.iRunStartMode==rsmQAMode)             //Steven 20111005 : QA Mode
    {
        if(CosFunction.bQAModeUseUnloadCnt)                                     //JerryYang 20221004 : Maxim版本QA mode
        {
        }
        else
        {
            if(Check_QA_ModeCount())                                            //Steven 20111019 : QA Mode
            {
                int iKit=InArmSuck.iWhichKit;
                int iKit32=(iKit==1)?4:0;
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)                      //ChungHung 20130910 alter QA mode HangUp
                        if(Prod.fInArmSuck4x8[InArmSuck.iWhichSht][i][j+iKit32]==true &&
                           InArmSuck.Item[i][j]==NULL_IC)                       //ChungHung 20130910 alter for SCK can close site by Index
                            InArmSuck.SetItemData(i, j, HAS_NULL_IC);
                }
            }
        }
    }

    bool bDoInArmAutoAlignment=false;
    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true &&
       (LastSet.iRealDummy==HAS_TRAY || LastSet.iRealDummy==REALLY))            //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    {
        if(bLoaderNeedTrayMustFinish)
        {
            if(iCleanOut==1 && HasICUnderMachine()==false)                      //process clean out ,don't supply new tray
            {
                bLoaderNeedTrayMustFinish=false;
                return;
            }
            else
            {
                if(MOT[MMTrayY].fHasTray)
                {
                    #if 0 // TODO(k3-ainarm2/INTEGRATOR) GATE [G2]+[G3]: CheckInArmAutoAlignmentTrayModeBeUse / CheckInArmAutoAlignmentCKModeBeUse -- golden AutoAlignment/AutoAlignment.h:265-266; the port has NO AutoAlignment directory.  Only TU-local `static` stand-ins exist (asendic_Loader.cpp:274/:304, AutoClean/AutoClean.cpp:243), both `return false`.  Compiling the calls out leaves bDoInArmAutoAlignment false -- bit-identical to those stand-ins.  Measured 2026-08-11 06:35:23 +0800.
                    bDoInArmAutoAlignment  = CheckInArmAutoAlignmentTrayModeBeUse(lInArmAutoAlignmentTrayTimingFlag, true);
                    bDoInArmAutoAlignment |= CheckInArmAutoAlignmentCKModeBeUse(lInArmAutoAlignmentCKTimingFlag, true);
                    #endif // GATE [G2]+[G3] -- golden :1683-1684
                    bRunInArmAutoAlignment = bDoInArmAutoAlignment;

                    bLoaderNeedTrayMustFinish=false;
                    return;
                }
                else if(iCleanOut==1)                                           //KenHsieh 20210830 : add CCD AUTO ALIGNMENT
                {
                    #if 0 // TODO(k3-ainarm2/INTEGRATOR) GATE [G2]+[G3]: CheckInArmAutoAlignmentTrayModeBeUse / CheckInArmAutoAlignmentCKModeBeUse -- golden AutoAlignment/AutoAlignment.h:265-266; the port has NO AutoAlignment directory.  Only TU-local `static` stand-ins exist (asendic_Loader.cpp:274/:304, AutoClean/AutoClean.cpp:243), both `return false`.  Compiling the calls out leaves bDoInArmAutoAlignment false -- bit-identical to those stand-ins.  Measured 2026-08-11 06:35:23 +0800.
                    bDoInArmAutoAlignment  = CheckInArmAutoAlignmentTrayModeBeUse(lInArmAutoAlignmentTrayTimingFlag, true);
                    bDoInArmAutoAlignment |= CheckInArmAutoAlignmentCKModeBeUse(lInArmAutoAlignmentCKTimingFlag, true);
                    #endif // GATE [G2]+[G3] -- golden :1692-1693
                    bRunInArmAutoAlignment = bDoInArmAutoAlignment;

                    bLoaderNeedTrayMustFinish=false;
                }
                else
                {
                    return;
                }
            }
        }
        else if(bRunInArmAutoAlignment || lOutArmAutoAlignmentFlag)
        {
            return;
        }
    }
    else
    {
        bLoaderNeedTrayMustFinish=false;
        bRunInArmAutoAlignment=false;
    }

    if(IniConfig.bIndexJamInArmAway)                                            //ChungHung 20110302
    {
        if(bInArmNeedToSafePos==true)                                           //Shuttle jam InArm 讓開
        {
            if(CheckInArmFinishAllPickerAction()==false)                        //Steven 20171226 (Wei) : 修改in arm讓開的flag
            {
                DoInArm_9045();
                return;
            }
            InitInArmTask();
            if(MoveInArm2XYToWait())
            {
                bInArmNeedToSafePos=false;
            }
        }
        else if(bShuttle1Pause==false && bShuttle2Pause==false)
        {
            if(CheckInArmFinishAllPickerAction()==false)                        //JerryYang 20180109 (Steven) fix in arm讓位後的hang up
            {
                DoInArm_9045();
                return;
            }

            if(IniConfig.bD43IndexDropErrorCanRetryandSkip  &&                  //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
               LastSet.iTemperature==Tempture_Hot &&
               ((InArmSuck.iWhichKitPickFor32==0 && bIndexPickUpErrMoveSht1==true) ||
                (InArmSuck.iWhichKitPickFor32==1 && bIndexPickUpErrMoveSht2==true)) &&
               bPickFromLoader==false && bPlaceToHotplate==false && bDestoryOnSht==false)
            {
                return;
            }
            else
            {
                DoInArm_9045();
            }
        }
    }
    else
    {
        if(IniConfig.bD43IndexDropErrorCanRetryandSkip  &&                      //Steven 20171221 (Wei) : 修正[D43]當蝦頭退出來要回去前,如果In Arm補了HAS_NULL_IC在蝦頭上會造Hang up
           (bIndexPickUpErrMoveSht1==true || bIndexPickUpErrMoveSht2==true))
        {
            return;
        }
        else
        {
            DoInArm_9045();
        }
    }
    return;
}

//==============================================================================
//== ROLE: TransferHotPlateRatio -- the HotPlate software gear ratio.  Given an
//==       in-arm XY target for HotPlate pick or place, re-scale it about the
//==       teach base cell Prod.[XY]InArm_Plate<n>_Pick[iInArmYBase][iInArmXBase]
//==       by the applicable X/Y scale (tri-temp Hot/Cold variant, setup-file
//==       variant, hot-mode variant, or the plain one) and by the HotPlate thermal
//==       expansion coefficient.  Finally it refuses any correction larger than
//==       iRet==200 by restoring the original value.
//==
//== WAVE SCOPE (k2-ainarm2):
//==   TransferHotPlateRatio  golden ainarm2.cpp:1767-1970  -- ACTIVE with 1 GATE
//==     (golden lines emitted VERBATIM apart from the two #if 0 / #endif lines
//==      wrapping golden :1788-:1791)
//==
//== GATE REGISTER
//== -- GATE k2ai2-G1 ------------------------------------------------------------
//==   GOLDEN LINES        : ainarm2.cpp:1788-1791 -- the whole if(iPlate==0)/else
//==                         plus its two arms
//==                         CheckInArmXYScaleByAutoTeach(*iXPos, *iYPos, AOA_Plate2 / AOA_Plate1)
//==                         The if/else HEADER is inside the gate deliberately:
//==                         gating only the two calls leaves a dangling if with no
//==                         statement, which is a hard compile error (caught by my
//==                         own full-TU -fsyntax-only probe, then fixed).
//==   WHY THE OFFLINE DEFAULT IS FAITHFUL:
//==     The gated region is the entire body of the
//==     if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true)
//==     arm, and MACHINE_HAS_AUTO_ALIGNMENT_CCD is DEFINED false
//==     (cmydef.cpp:5316) and only ever raised by reading the ini key of the same
//==     name (database.cpp:1130); the offline harness leaves it false and
//==     tests/test_AutoClean.cpp:103 pins it false explicitly.  So offline the
//==     guard is never taken and an empty arm is bit-identical to golden.
//==     BUT PER TRAP 2 THAT EQUIVALENCE IS NOT THE JUSTIFICATION -- the
//==     justification is the absence premise below, and that premise is what the
//==     main loop must re-test at integration.
//==   REAL-MACHINE BEHAVIOUR DIFFERENCE:
//==     On a machine with MACHINE_HAS_AUTO_ALIGNMENT_CCD=1 and AOA enabled, golden
//==     applies the auto-teach CCD alignment correction to the HotPlate XY and
//==     SKIPS the whole gear-ratio else branch.  With this gate on, such a machine
//==     gets NO alignment correction and NO gear-ratio correction either -- the
//==     raw XY passes through untouched.  That is a real loss of compensation, not
//==     a cosmetic one, so this gate MUST NOT ship to a real AOA machine.
//==   EXACT COMMAND PROVING THE SYMBOL IS ABSENT (TRAP 2):
//==     grep -rnE "^[[:space:]]*(extern[[:space:]]+)?void[[:space:]]+CheckInArmXYScaleByAutoTeach" --include=*.cpp --include=*.h D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==     -> no output, exit 1.  The companion broad grep
//==     grep -rn --include=*.cpp --include=*.h "CheckInArmXYScaleByAutoTeach" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//==     returns only AutoClean/AutoClean.cpp:225 (a comment), :232 (the static
//==     no-op stand-in), :239 (a comment) and :2261 (that file's own call).
//==   TIME I RAN IT: 2026-08-11 06:37:25 +0800.
//==   TRAP 3 -- WHY THIS SHOULD STAY GATED EVEN IF THE PREMISE DIES:
//==     If a sibling lands a real CheckInArmXYScaleByAutoTeach, the gate must NOT
//==     simply be deleted.  AutoClean.cpp:232 would then be a TU-local static
//==     SHADOW of the real function inside a file that also has a live call site
//==     at its :2261 -- that is TRAP 1 shape (d) waiting to happen.  Retire this
//==     gate ONLY in the same commit that deletes the AutoClean.cpp static
//==     stand-in, and verify the new body takes int& / int& (golden passes *iXPos
//==     and *iYPos as lvalues, so a by-value signature would silently discard the
//==     correction and look exactly like a working ungate).
//==
//== GOLDEN BUGS PRESERVED (not fixed -- reported instead):
//==   * golden :1967-1968 -- the Y overshoot guard tests abs(*iYPos-iOldY)>iRet but
//==     then restores *iXPos=iOldX.  A Y overshoot therefore clobbers X and leaves
//==     the bad Y in place.  Reproduced verbatim.
//==   * golden :1875 -- in the Plate2 non-tri-temp branch the Y expansion
//==     coefficient multiply sits INSIDE the innermost else, so it is skipped
//==     whenever the setup-file or hot-mode Y scale is used.  X (:1856) and both
//==     Plate1 axes (:1941, :1960) apply it unconditionally.  Reproduced verbatim.
//==   * The double-to-int narrowing on every *iXPos= / *iYPos= assignment is
//==     golden's own truncation and is load-bearing; no rounding helper was
//==     introduced.  Note there is NO int/int division anywhere in this function,
//==     so nothing here was at risk of the float-substitution regression.
//==
//== GATE-FREE DEPENDENCIES (all real, all resolved in this TU):
//==   MACHINE_HAS_AUTO_ALIGNMENT_CCD cmydef.h:5237 (def cmydef.cpp:5316);
//==   TestIF.bEnableAutoAlignment cprod.h:2419; AOA_Plate1/AOA_Plate2 cprod.h:3176
//==   /3177; Tri_Temp_Machine cmydef.h:5536; Tempture_Hot cmydef.h:2760;
//==   LastSet object LastSet.h:587; LastSet.fHotPlate{X,Y}Scale_Hot[2]
//==   LastSet.h:548/549, _Cold[2] :559/560, plain [2] :383/384;
//==   IniConfig.bE30InArmUseDifferentScale Config.h:597,
//==   bE30_1InArmUseDifferentScale_Hot :647, bE30_2InArmUseDifferentScale_Cold
//==   :650 (object Config.h:1499); Temperature.fWorkTemperBase cprod.h:1392
//==   (object cprod.h:1646); TestIF_File.bInArmUseDifferentScaleBySetupFile
//==   cprod.h:2422 and fHotPlate{X,Y}ScaleBySetupFile[2] :2425/2426;
//==   CosFunction.bHotModeUseDiffScale CosFunction.h:478 (object :488);
//==   Prod.{X,Y}InArm_Plate1_Pick cprod.h:381/382 and _Plate2_Pick :383/384;
//==   iInArmXBase/iInArmYBase cmydef.h:3638/3639;
//==   fHotPlateExpansionCoefficient cmydef.h:3274;
//==   iPickPlate[2]/iPlacePlate[2] aHotPlateSubstrate.h:740/741.
//==
//== INTEGRATION NOTE -- TRAP 1 shape (b), MUST ACT:
//==   port aHotPlateSubstrate.cpp:1064 holds an EMPTY no-op
//==   TransferHotPlateRatio(bool, int *, int *) which already satisfies the live
//==   callers ainarm9045_2x2_8_Hot.cpp:1049, ainarm_SearchPickPlate.cpp:727 and
//==   ainarm_SearchPlacePlate.cpp:1967 (the two further hits in
//==   ainarm9045_2x8_32.cpp:2201/2604 are dead -- that file is #if 0 from its line
//==   91 to EOF).  DELETE the stub or this part is a duplicate definition.  The
//==   declaration at aHotPlateSubstrate.h:898 ALREADY matches golden ainarm2.h:158
//==   arity exactly, so no header arity change is needed here (unlike SetInArmHome
//==   in part 01266).  Behaviour genuinely changes: today the HotPlate gear ratio
//==   and thermal expansion are simply NOT APPLIED.
//==   Not static -- shape (d) avoided.
//==
//== TRAP 4 ANSWER: no file-scope object is added by this part.
//==
//== Big5 -> UTF-8: every Chinese comment (golden :1767, :1786, :1797, :1837,
//== :1882, :1922) is transcribed character for character from cp950.  Zero U+FFFD.
//==============================================================================
void TransferHotPlateRatio(bool bPick, int *iXPos, int *iYPos)                  //Steven 20110324 : 加熱盤的軟體齒輪比
{
    double fi=0.0;
    int iOldX=0, iOldY=0, iRet=200;
    int iPlate=0;

    iOldX=*iXPos;
    iOldY=*iYPos;

    if(bPick==true)
    {
        iPlate=iPickPlate[0];
    }
    else
    {
        iPlate=iPlacePlate[0];
    }

    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&                                        //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
       TestIF.bEnableAutoAlignment==true)                                       //KenHsieh 20210824 : add AOA功能開啟不補償
    {
#if 0 // GATE k2ai2-G1 -- golden ainarm2.cpp:1788-1791, the whole if(iPlate==0)/else whose only two statements are CheckInArmXYScaleByAutoTeach: the ONLY body in the port tree is a static (internal-linkage) no-op stand-in at AutoClean/AutoClean.cpp:232, so there is no linkable symbol at all.  The if/else HEADER is gated together with its arms on purpose -- gating only the two calls would leave a dangling if with no statement.  See the GATE REGISTER in this part's banner for the exact absence command, the time it ran, and the real-machine delta.
        if(iPlate==0)
            CheckInArmXYScaleByAutoTeach(*iXPos, *iYPos, AOA_Plate2);           //KenHsieh 20210830 : add CCD AUTO ALIGNMENT
        else
            CheckInArmXYScaleByAutoTeach(*iXPos, *iYPos, AOA_Plate1);           //KenHsieh 20210830 : add CCD AUTO ALIGNMENT
#endif
    }
    else
    {
        if(iPlate==0)
        {
            if(Tri_Temp_Machine==1 && LastSet.iTemperature==Tempture_Hot)       //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
            {
                if(IniConfig.bE30_1InArmUseDifferentScale_Hot==true && Temperature.fWorkTemperBase>=26)
                {
                    fi=*iXPos-Prod.XInArm_Plate2_Pick[iInArmYBase][iInArmXBase];
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateXScale_Hot[1];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                    *iXPos=fi+Prod.XInArm_Plate2_Pick[iInArmYBase][iInArmXBase];

                    fi=*iYPos-Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase];
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateYScale_Hot[1];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                    *iYPos=fi+Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase];
                }

                if(IniConfig.bE30_2InArmUseDifferentScale_Cold==true && Temperature.fWorkTemperBase<26)
                {
                    fi=*iXPos-Prod.XInArm_Plate2_Pick[iInArmYBase][iInArmXBase];
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateXScale_Cold[1];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                    *iXPos=fi+Prod.XInArm_Plate2_Pick[iInArmYBase][iInArmXBase];

                    fi=*iYPos-Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase];
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateYScale_Cold[1];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                    *iYPos=fi+Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase];
                }
            }
            else                                                                //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
            {
                fi=*iXPos-Prod.XInArm_Plate2_Pick[iInArmYBase][iInArmXBase];    //Steven 20141110 : [0][2] --> [iInArmYBase][iInArmXBase]

                if(TestIF_File.bInArmUseDifferentScaleBySetupFile)              //pig 20220101 AddInputScaleBySetup
                {
                    fi*=TestIF_File.fHotPlateXScaleBySetupFile[1];              //pig 20220624 ModifyInputScaleBySetup
                }
                else if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE30_1InArmUseDifferentScale_Hot)
                {
                    fi=fi*LastSet.fHotPlateXScale_Hot[1];
                }
                else
                {
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateXScale[1];
                    }
                }
                fi=fi*fHotPlateExpansionCoefficient;
                *iXPos=fi+Prod.XInArm_Plate2_Pick[iInArmYBase][iInArmXBase];

                fi=*iYPos-Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase];

                if(TestIF_File.bInArmUseDifferentScaleBySetupFile)
                {
                    fi*=TestIF_File.fHotPlateYScaleBySetupFile[1];              //pig 20220624 ModifyInputScaleBySetup
                }
                else if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE30_1InArmUseDifferentScale_Hot)
                {
                    fi=fi*LastSet.fHotPlateYScale_Hot[1];
                }
                else
                {
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateYScale[1];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                }
                *iYPos=fi+Prod.YInArm_Plate2_Pick[iInArmYBase][iInArmXBase];
            }
        }
        else
        {
            if(Tri_Temp_Machine==1 && LastSet.iTemperature==Tempture_Hot)       //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
            {
                if(IniConfig.bE30_1InArmUseDifferentScale_Hot==true && Temperature.fWorkTemperBase>=26)
                {
                    fi=*iXPos-Prod.XInArm_Plate1_Pick[iInArmYBase][iInArmXBase];
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateXScale_Hot[0];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                    *iXPos=fi+Prod.XInArm_Plate1_Pick[iInArmYBase][iInArmXBase];

                    fi=*iYPos-Prod.YInArm_Plate1_Pick[iInArmYBase][iInArmXBase];
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateYScale_Hot[0];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                    *iYPos=fi+Prod.YInArm_Plate1_Pick[iInArmYBase][iInArmXBase];
                }

                if(IniConfig.bE30_2InArmUseDifferentScale_Cold==true && Temperature.fWorkTemperBase<26)
                {
                    fi=*iXPos-Prod.XInArm_Plate1_Pick[iInArmYBase][iInArmXBase];
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateXScale_Cold[0];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                    *iXPos=fi+Prod.XInArm_Plate1_Pick[iInArmYBase][iInArmXBase];

                    fi=*iYPos-Prod.YInArm_Plate1_Pick[iInArmYBase][iInArmXBase];
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateYScale_Cold[0];
                    }
                    fi=fi*fHotPlateExpansionCoefficient;
                    *iYPos=fi+Prod.YInArm_Plate1_Pick[iInArmYBase][iInArmXBase];
                }
            }
            else                                                                //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
            {
                fi=*iXPos-Prod.XInArm_Plate1_Pick[iInArmYBase][iInArmXBase];

                if(TestIF_File.bInArmUseDifferentScaleBySetupFile)
                {
                    fi*=TestIF_File.fHotPlateXScaleBySetupFile[0];              //pig 20220624 ModifyInputScaleBySetup
                }
                else if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE30_1InArmUseDifferentScale_Hot)
                {
                    fi=fi*LastSet.fHotPlateXScale_Hot[0];
                }
                else
                {
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateXScale[0];
                    }
                }
                fi=fi*fHotPlateExpansionCoefficient;
                *iXPos=fi+Prod.XInArm_Plate1_Pick[iInArmYBase][iInArmXBase];

                fi=*iYPos-Prod.YInArm_Plate1_Pick[iInArmYBase][iInArmXBase];
                if(TestIF_File.bInArmUseDifferentScaleBySetupFile)
                {
                    fi*=TestIF_File.fHotPlateYScaleBySetupFile[0];              //pig 20220624 ModifyInputScaleBySetup
                }
                else if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE30_1InArmUseDifferentScale_Hot)
                {
                    fi=fi*LastSet.fHotPlateYScale_Hot[0];
                }
                else
                {
                    if(IniConfig.bE30InArmUseDifferentScale)
                    {
                        fi=fi*LastSet.fHotPlateYScale[0];
                    }
                }
                fi=fi*fHotPlateExpansionCoefficient;
                *iYPos=fi+Prod.YInArm_Plate1_Pick[iInArmYBase][iInArmXBase];
            }
        }

        if(abs(*iXPos-iOldX)>iRet)
            *iXPos=iOldX;
        if(abs(*iYPos-iOldY)>iRet)
            *iXPos=iOldX;
    }
}

//==============================================================================
//  TransferInShuttleRatio(int,int*,int*,int,int)  --  golden ainarm2.cpp:1972-2110
//
//  ROLE          In-shuttle "software gear ratio": rescale a taught in-arm place
//                position about the Shuttle-1/2 PLACE teach point for the given
//                (iRow,iCol), choosing the scale set by priority -- Tri-temp
//                Hot/Cold pair > setup-file pair > the legacy E32 pair (which
//                itself picks Hot-mode scales when CosFunction says so).
//  WAVE SCOPE    TransferInShuttleRatio()   ACTIVE (one inner gate)  golden :1972-2110
//
//  GATE REGISTER
//    #if 0 -- golden lines :1976-1979 (the whole `if(iShuttle==0) Check... else
//      Check...` pair, gated as a BLOCK so the enclosing `if(...){ }` stays
//      well-formed -- gating only the two call lines would leave `if(..) else`).
//      SYMBOL: void CheckInArmXYScaleByAutoTeach(int&,int&,int)  -- golden home
//        AutoAlignment/AutoAlignment.h:230.  The whole AutoAlignment (AOA)
//        subsystem is untranslated in this tree.
//      WHY THE OFFLINE DEFAULT IS FAITHFUL: the branch is double-gated by
//        `MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true`,
//        and MACHINE_HAS_AUTO_ALIGNMENT_CCD is false in this build, so the gated
//        region is unreachable offline -- iXPos/iYPos are never read or written
//        by it.  This is the SAME premise the tree already recorded at
//        AutoClean/AutoClean.cpp:225-232 for the identical symbol; that premise
//        carries over unchanged and is not re-derived here.
//      REAL-MACHINE BEHAVIOUR DIFFERENCE: on a machine that HAS the alignment CCD
//        and has bEnableAutoAlignment on, golden replaces the taught position with
//        the AOA-corrected one and then SKIPS all gear-ratio scaling (that is what
//        golden's own `//KenHsieh 20210824 : add AOA功能開啟不補償` on the else
//        means).  Gated, the position is passed through UNCORRECTED and STILL
//        unscaled -- i.e. a raw taught position with no AOA compensation.  That is
//        a real positional difference, not an equivalence; it is acceptable only
//        because the branch cannot be entered in this build.
//      COMMAND PROVING ABSENCE (from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//        grep -rn --include=*.h --include=*.cpp "CheckInArmXYScaleByAutoTeach" . | grep -v /build | grep -v /tools/
//        -> ONLY AutoClean/AutoClean.cpp:225(comment) :232(a TU-local `static`
//           no-op) :239(comment) :2261(its own call).  A `static` in another TU
//           has internal linkage, so there is NO body callable from here.
//      RUN AT: 20260811 06:38:59 local (Asia/Taipei).  TRAP 2: re-run at
//        integration -- a sibling agent may land AutoAlignment mid-wave.
//      NOTE (why this file does NOT copy AutoClean's shape): AutoClean.cpp:232
//        defines a file-scope `static` with the SAME NAME as golden's non-static
//        symbol.  That is latent TRAP 1 shape (d): the day AutoAlignment.h is
//        included there, the declaration and the static collide.  Reported in
//        deviations rather than replicated.
//
//  RETIRES A STUB: aHotPlateSubstrate.cpp:1551 `void TransferInShuttleRatio(...)
//    {}` (decl aHotPlateSubstrate.h:1065) must be REMOVED by the integrator.  NOT
//    written `static` (TRAP 1 shape (d)).  Offline behaviour after retirement is
//    UNCHANGED (identity) on all 36 live call sites, because every selector --
//    Tri_Temp_Machine, IniConfig.bE32_1ShuttleUseDifferentScale_Hot,
//    IniConfig.bE32_2ShuttleUseDifferentScale_Cold,
//    TestIF_File.bInArmUseDifferentScaleBySetupFile,
//    IniConfig.bE32ShuttleUseDifferentScale -- is false/0 offline.  The header decl
//    names its first parameter `iSht`; golden names it `iShuttle`.  Parameter names
//    do not affect linkage, so golden's name is kept.
//
//  GOLDEN DEFECTS PRESERVED (do NOT "fix"):
//    * :2010 the *_Cold* scale pair is selected INSIDE the branch that already
//      required `LastSet.iTemperature==Tempture_Hot` (:1984), gated on
//      fWorkTemperBase<26.  Kept exactly.
//    * :2087 and :2098 test bE32_1ShuttleUseDifferentScale_Hot (the *_1* flag)
//      while operating on Shuttle **2** scales -- there is no _2_ flag in the
//      condition.  Kept exactly, including golden's missing space in
//      `&&LastSet.iTemperature`.
//    * :2036 re-declares `double fi=0.0;` shadowing the :1983 `fi`.  Kept.
//    * `*iXPos=fi+...` stores a double into an int* target (truncation toward
//      zero).  Kept -- this IS the rounding golden ships.  No float helper was
//      substituted anywhere.
//==============================================================================
void TransferInShuttleRatio(int iShuttle, int *iXPos, int *iYPos, int iRow, int iCol)                                   //Steven 20110324 : In Shuttle的軟體齒輪比
{
    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true)     //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    {
#if 0 // GATE (W7e-G1): golden :1976-1979 CheckInArmXYScaleByAutoTeach -- AOA subsystem untranslated; branch unreachable offline (MACHINE_HAS_AUTO_ALIGNMENT_CCD false). See GATE REGISTER above.
        if(iShuttle==0)
            CheckInArmXYScaleByAutoTeach(*iXPos, *iYPos, AOA_InSht1);
        else
            CheckInArmXYScaleByAutoTeach(*iXPos, *iYPos, AOA_InSht2);
#endif // GATE (W7e-G1)
    }
    else                                                                        //KenHsieh 20210824 : add AOA功能開啟不補償
    {
        double fi=0.0;                                                          //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
        if(Tri_Temp_Machine==1 && LastSet.iTemperature==Tempture_Hot)
        {
            if(IniConfig.bE32_1ShuttleUseDifferentScale_Hot==true && Temperature.fWorkTemperBase>=26)
            {
                if(iShuttle==0)
                {
                    fi=*iXPos-Prod.XInArm_Shuttle1_Place[iRow][iCol];
                    fi*=LastSet.fInShuttleXScale_Hot[0];
                    *iXPos=fi+Prod.XInArm_Shuttle1_Place[iRow][iCol];

                    fi=*iYPos-Prod.YInArm_Shuttle1_Place[iRow][iCol];
                    fi*=LastSet.fInShuttleYScale_Hot[0];
                    *iYPos=fi+Prod.YInArm_Shuttle1_Place[iRow][iCol];
                }
                else
                {
                    fi=*iXPos-Prod.XInArm_Shuttle2_Place[iRow][iCol];
                    fi*=LastSet.fInShuttleXScale_Hot[1];
                    *iXPos=fi+Prod.XInArm_Shuttle2_Place[iRow][iCol];

                    fi=*iYPos-Prod.YInArm_Shuttle2_Place[iRow][iCol];
                    fi*=LastSet.fInShuttleYScale_Hot[1];
                    *iYPos=fi+Prod.YInArm_Shuttle2_Place[iRow][iCol];
                }
            }

            if(IniConfig.bE32_2ShuttleUseDifferentScale_Cold==true && Temperature.fWorkTemperBase<26)
            {
                if(iShuttle==0)
                {
                    fi=*iXPos-Prod.XInArm_Shuttle1_Place[iRow][iCol];
                    fi*=LastSet.fInShuttleXScale_Cold[0];
                    *iXPos=fi+Prod.XInArm_Shuttle1_Place[iRow][iCol];

                    fi=*iYPos-Prod.YInArm_Shuttle1_Place[iRow][iCol];
                    fi*=LastSet.fInShuttleYScale_Cold[0];
                    *iYPos=fi+Prod.YInArm_Shuttle1_Place[iRow][iCol];
                }
                else
                {
                    fi=*iXPos-Prod.XInArm_Shuttle2_Place[iRow][iCol];
                    fi*=LastSet.fInShuttleXScale_Cold[1];
                    *iXPos=fi+Prod.XInArm_Shuttle2_Place[iRow][iCol];

                    fi=*iYPos-Prod.YInArm_Shuttle2_Place[iRow][iCol];
                    fi*=LastSet.fInShuttleYScale_Cold[1];
                    *iYPos=fi+Prod.YInArm_Shuttle2_Place[iRow][iCol];
                }
            }
        }                                                                       //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
        else if(TestIF_File.bInArmUseDifferentScaleBySetupFile)
        {
            double fi=0.0;
            if(iShuttle==0)
            {
                fi=*iXPos-Prod.XInArm_Shuttle1_Place[iRow][iCol];
                fi*=TestIF_File.fInShuttleXScaleBySetupFile[0];
                *iXPos=fi+Prod.XInArm_Shuttle1_Place[iRow][iCol];

                fi=*iYPos-Prod.YInArm_Shuttle1_Place[iRow][iCol];
                fi*=TestIF_File.fInShuttleYScaleBySetupFile[0];
                *iYPos=fi+Prod.YInArm_Shuttle1_Place[iRow][iCol];
            }
            else
            {
                fi=*iXPos-Prod.XInArm_Shuttle2_Place[iRow][iCol];
                fi*=TestIF_File.fInShuttleXScaleBySetupFile[1];
                *iXPos=fi+Prod.XInArm_Shuttle2_Place[iRow][iCol];

                fi=*iYPos-Prod.YInArm_Shuttle2_Place[iRow][iCol];
                fi*=TestIF_File.fInShuttleYScaleBySetupFile[1];
                *iYPos=fi+Prod.YInArm_Shuttle2_Place[iRow][iCol];
            }
        }
        else if(IniConfig.bE32ShuttleUseDifferentScale)                         //jou 980716 start : add Scale
        {
            if(iShuttle==0)
            {
                fi=*iXPos-Prod.XInArm_Shuttle1_Place[iRow][iCol];
                if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE32_1ShuttleUseDifferentScale_Hot && LastSet.iTemperature==Tempture_Hot)
                {
                    fi*=LastSet.fInShuttleXScale_Hot[0];
                }
                else
                {
                    fi*=LastSet.fInShuttleXScale[0];
                }
                *iXPos=fi+Prod.XInArm_Shuttle1_Place[iRow][iCol];

                fi=*iYPos-Prod.YInArm_Shuttle1_Place[iRow][iCol];
                if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE32_1ShuttleUseDifferentScale_Hot && LastSet.iTemperature==Tempture_Hot)
                {
                    fi*=LastSet.fInShuttleYScale_Hot[0];
                }
                else
                {
                    fi*=LastSet.fInShuttleYScale[0];
                }
                *iYPos=fi+Prod.YInArm_Shuttle1_Place[iRow][iCol];
            }
            else
            {
                fi=*iXPos-Prod.XInArm_Shuttle2_Place[iRow][iCol];
                if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE32_1ShuttleUseDifferentScale_Hot &&LastSet.iTemperature==Tempture_Hot)
                {
                    fi*=LastSet.fInShuttleXScale_Hot[1];
                }
                else
                {
                    fi*=LastSet.fInShuttleXScale[1];
                }
                *iXPos=fi+Prod.XInArm_Shuttle2_Place[iRow][iCol];

                fi=*iYPos-Prod.YInArm_Shuttle2_Place[iRow][iCol];
                if(CosFunction.bHotModeUseDiffScale==true && IniConfig.bE32_1ShuttleUseDifferentScale_Hot &&LastSet.iTemperature==Tempture_Hot)
                {
                    fi*=LastSet.fInShuttleYScale_Hot[1];
                }
                else
                {
                    fi*=LastSet.fInShuttleYScale[1];
                }
                *iYPos=fi+Prod.YInArm_Shuttle2_Place[iRow][iCol];
            }
        }
    }
}

// =============================================================================
//  AI(W906-PT-W7e-k8) 20260811 -- golden ainarm2.cpp translation part
//  golden file : D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618/ainarm2.cpp (cp950, CRLF)
//  this part   : golden lines 2199..2302
// -----------------------------------------------------------------------------
//  ROLE        : true when the HotPlate check / try-pick sweep is finished -- counts NULL_IC cells and real-IC cells over the selected plate(s) and compares against a per-test-mode expected count.  Golden's misspelling 'Finsih' is part of the symbol name.
//  WAVE SCOPE  : IsHotPlateCheckFinsih -- ACTIVE, golden ainarm2.cpp:2199..2302.
//  COLLISION    : *** INTEGRATOR MUST DELETE THE EXISTING PARTIAL/STUB BODY ***
//              : aHotPlateSubstrate.cpp:1583  `bool IsHotPlateCheckFinsih() { return true; }` -- that stub answers 'finished' UNCONDITIONALLY, so today all 21 variant call sites terminate the sweep immediately
//              : It is a NON-static definition of the same signature, so leaving
//              : it in place is a `multiple definition` link error, not a silent
//              : shadow.  Its `extern` DECLARATION stays where it is -- only the
//              : body is deleted.
//  INCLUDES     : this body needs, beyond what port ainarm2.cpp already includes
//              : (cmydef.h / MachineType.h / cprod.h / ainarm9045.h /
//              : aHotPlateSubstrate.h):
//              : Motor/mymotor.h (MOT[]) -- HotPlateForm / HotPlateForm_File and every test-mode enum come from cprod.h and MachineType.h, both already included
//  GATE REGISTER: none.  All 16 test-mode enumerators exist (MachineType.h:443, :445,
//              : :447, :450, :453, :454, :460 plus the earlier ones), USE_PICKER_COUNT
//              : and ep1Picker are present, bOneTimeHotPlateCheckAll cmydef.h:2707,
//              : bHPCleanout cmydef.h:4134, iPlacePlate / iPlacePlateX / iPlacePlateY
//              : aHotPlateSubstrate.h:741, HotPlateForm_File.XPitch cprod.h:1258
//              : (a double, compared against 80).
//  BEHAVIOUR    : THIS IS THE BIGGEST BEHAVIOUR CHANGE IN THIS AGENT'S SET.  Going from
//              : `return true` to the real body means the HP try-pick sweep can now
//              : answer NOT-finished and keep sweeping.  That is the faithful
//              : direction, and it is also the direction that can newly expose a loop
//              : offline, because an offline HotPlate is never populated.  Worth
//              : watching in ctest.
//  GOLDEN QUIRK : the same `iPlateSelect & 2-i` precedence quirk as part 00533, here at
//              : golden :2234.  Transcribed verbatim.
//  GOLDEN QUIRK : golden :2236-2237 `if(HotPlateForm.iPlateSelect==3 && i==0) continue;`
//              : skips plate 1 entirely whenever BOTH plates are selected, so with
//              : iPlateSelect==3 only plate 2 is ever counted.  Kept -- do not fix.
//  GOLDEN QUIRK : iDiscount is initialised to 0 and is only ever ASSIGNED 0 (golden
//              : :2251), so the `iCT-iDiscount<iCheckCT` test at :2297 is just
//              : `iCT<iCheckCT`.  Dead arithmetic, transcribed verbatim.
//  GOLDEN QUIRK : when bOneTimeHotPlateCheckAll is false the function RETURNS from
//              : inside the else block (:2260, :2265, :2267, :2271) and never reaches
//              : the iCheckCT comparison at all -- iCheckCT and the whole first
//              : computation are dead on that path.  Preserved exactly.
// =============================================================================
bool IsHotPlateCheckFinsih()                                                    //ChungHung 20120206 Hotplate check
{
    int iCT=0, iDiscount=0, iCheckCT=0;
    int iHAS_IC=0;
    if(TestIF.iTestMode==_8Site2X4 && HotPlateForm_File.XPitch>80)              //Ifor 20151013 add For Xilinx
        iCheckCT=4;
    else if(TestIF.iTestMode==_8Site2X4 ||
            TestIF.iTestMode==_12Site2X6 ||
            TestIF.iTestMode==_10Site2X5)                                       //wei 20190614 10 site
        iCheckCT=8;
    else if(TestIF.iTestMode==QualSite2X2 ||
            TestIF.iTestMode==QualSite1X4 ||
            TestIF.iTestMode==_8Site1X4  ||                                     //ChungHung 20150528 add for 海思 _8Site1x4
            TestIF.iTestMode==TriSite1X3 ||
            TestIF.iTestMode==_6Site2X3N ||                                     //Steven 20220425 : 2X3NN Mode
            TestIF.iTestMode==_8Site2X4N)                                       //Wei 20231211 : 2X4NN Mode
        iCheckCT=4;
    else if(TestIF.iTestMode==DualSite ||
            TestIF.iTestMode==DualSite2x1 ||
            TestIF.iTestMode==QualSite2X2N)                                     //Frank 20200520 2X2NN Mode
        iCheckCT=2;
    else
        iCheckCT=8;

    if(USE_PICKER_COUNT==ep1Picker)                                             //Ifor 20260313 add:
    {
        iCheckCT=1;
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<HotPlateForm.XDivision; j++)
        {
            for(int k=0; k<HotPlateForm.YDivision;k++)
            {
                if(HotPlateForm.iPlateSelect & 2-i)                             //MMPlate1+0=NO 2 HP
                {
                    if(HotPlateForm.iPlateSelect==3 && i==0)
                        continue;
                    if(MOT[MMPlate1+i].Tray.Data[j][k]==NULL_IC)
                        iCT++;
                    if(MOT[MMPlate1+i].Tray.Data[j][k]==HAS_IC ||
                       MOT[MMPlate1+i].Tray.Data[j][k]==HAS_HOT_IC ||
                       MOT[MMPlate1+i].Tray.Data[j][k]==HAS_NULL_IC)
                        iHAS_IC++;
                }
            }
        }
    }

    if(bOneTimeHotPlateCheckAll)
    {
        iDiscount=0;
    }
    else
    {
        if(iPlacePlateX[0]==0 && iPlacePlateY[0]==0)
        {
            if((HotPlateForm.iPlateSelect & 2) && iPlacePlate[0]==0)
            {
                if(iHAS_IC!=0)
                    return true;
            }
            else if((HotPlateForm.iPlateSelect==1) && iPlacePlate[0]==1)
            {
                if(iHAS_IC!=0)
                    return true;
            }
            return false;
        }
        else
        {
            return false;
        }
    }

    if(TestIF.iTestMode==_8Site2X4 && HotPlateForm_File.XPitch>80)              //Ifor 20151013 add For Xilinx
    {
        if(iCT<=0)
            return true;
    }
    else if(bHPCleanout)                                                        //wei 20160624 Hotplate clean out
    {
        if(TestIF.iTestMode==_6Site2X3 ||
           TestIF.iTestMode==_8Site2X4 ||
           TestIF.iTestMode==_16Site4X4)                                        //Sam 20190226 : 16Site4X4
        {
            if(iCT<4)
                return true;
        }
        else
        {
            if(iCT<=0)
                return true;
        }
    }
    else
    {
        if(iCT-iDiscount<iCheckCT)
            return true;
    }

    return false;
}

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 02315 : DoShakeShuttle
//  Golden: ainarm2.cpp:2315-2447 (cp950, CRLF).
//
//  ROLE          : the in-shuttle SHAKE sub-SM (golden calls it the "抖抖" function).  On
//                  bNeedInitial it arms a 10 s watchdog, raises bShuttleKnock and returns
//                  false.  Then, unless an Index Z is still down (in Auto-Clean it merely
//                  CLEARS that down-flag instead of waiting), it runs
//                  essSetFlag -> essMoveRight -> essMoveLeft (iShakeCycles times) -> essEnd:
//                  swaps in the Shake accel/scale speeds, takes both IndexZCanMove
//                  interlocks, frees the shuttle's CanMoveL, jogs the shuttle
//                  iShakeDistance*100 right of Prod.InSHT[].iLeft and back with
//                  dShakeDelay between strokes, and on the last stroke restores the
//                  normal SH speeds, drops the knocker cylinder, sets iShuttleLog=2,
//                  re-inits DoKnockShuttle and returns true.  The watchdog branch logs,
//                  pops a message and pauses.
//  WAVE SCOPE    : DoShakeShuttle(int,bool)  golden ainarm2.cpp:2315-2447  ACTIVE
//  GATE REGISTER : (none -- no #if 0 in this part; every callee has a real port body)
//                  RecordProcess/ShowMyMessage/__FUNC__ canary_support.h:70/:80/:45 ;
//                  IndexZCanMove ainarm9045_w7_shims.h:58 (def ainarm9045_w7_shims.cpp:47) ;
//                  DoKnockShuttle ainarm9045_w7_shims.h:48 (still the offline `return true`
//                  stub at ainarm9045_w7_shims.cpp:39 -- I only RE-INIT it, so the stub is
//                  harmless here; a sibling part may retire it) ;
//                  SetMotorAccelSpeed/SetMotorScaleSpeed cinitial.h:256/:184 (bodies are
//                  offline no-ops at acatchtray_shims.cpp:171/:175 -- PRE-EXISTING TRAP-1(b)
//                  shape, not introduced here) ; MOT[].MotorMoveShuttleShake Motor/mymotor.h:164 ;
//                  InSHT1InLF/InSHT2InLF csystem.h:166 ; bIndexWaitingInArmAway atester.h:51 ;
//                  bUseFix3CylinderActive cmydef.h:3701 ; Cylinder[].Off() mycylin.h ;
//                  SHSpeed/SHSpeed_File cprod.h:2944/:2945 ; SetShuttlefCanMoveL is my own
//                  part 03910 (declared aHotPlateSubstrate.h:1070 -- default arg lives there,
//                  so neither definition repeats it).
//  INTEGRATOR HOLE (NOT in your 37-global list -- please read):
//                  golden ainarm2.cpp:2311 `int iShakeShuttleTask;` and golden ainarm2.h:180-185
//                  `enum eShakeShuttle{essSetFlag=1,...}` have NO home anywhere in the port
//                  (measured 20260811 06:40:27 / 06:41:13 local; commands below).  Both sit just OUTSIDE my assigned
//                  2315..2447 range, and neither is in the integrator list (:2313/:2314 are).
//                  I therefore emit both below inside #ifndef guards -- the tree's own idiom
//                  (cf. port ainarm2.cpp:44 HT9045_bDestoryOnSht_DEFINED) -- so that a sibling
//                  part translating DoKnockShuttle (golden :2450, same enum) cannot collide.
//                  IF YOU ADD THEM CENTRALLY, either delete these two blocks or #define the
//                  two guard macros before the parts are stitched: an UNGUARDED central
//                  `int iShakeShuttleTask;` in the same TU as my guarded one is a redefinition
//                  error.  ABSENCE COMMANDS -- verbatim, both run in Git-bash from
//                  D:/HT9045/HT9011UC_Cpp_V3.33.906.0 (TRAP 2: re-run these at integrate,
//                  a sibling could land either symbol after my clock time).
//                  Each command below is ONE shell line, wrapped here for the banner --
//                  no trailing backslashes, they would splice the // comment):
//                    [1] ran 2026-08-11 06:40:27 local
//                        grep -rn --include=*.h --include=*.cpp "iShakeShuttleTask"
//                             --exclude-dir=build* --exclude-dir=.git .
//                        -> exactly 3 hits, all acarry.cpp: :169 `extern int
//                           iShakeShuttleTask;` plus :4674 and :6463 which are BOTH inside
//                           `//` comments.  So: DECLARED once, DEFINED nowhere.
//                    [2] ran 2026-08-11 06:40:39 .. 06:41:13 local
//                        grep -rnE --include=*.h --include=*.cpp
//                             "\b(essSetFlag|essMoveRight|essMoveLeft|essKnockOn|essKnockOff|essEnd|eShakeShuttle)\b"
//                             --exclude-dir=.git . | grep -v _w7e_parts
//                        -> 0 hits.  The \b anchors are load-bearing: an unbounded `essEnd`
//                           false-hits sProcessEndInfo (cmydef.h:5695 / cmydef.cpp:5734).
//  GOLDEN BUGS (kept verbatim, see report):  :2390-2392 str3 is sprintf'd twice (the short
//                  form is overwritten unread) and the long format string has 8 conversions
//                  for 9 arguments because `fCanMoveM:d` lost its % -- so the value printed
//                  after "fCanMove:" is actually fCanMoveM and fCanMove is dropped;
//                  :2391 also prints bShuttleShake twice.  :2394 ShowMyMessage(str1, str2)
//                  passes str2 while it is still "".
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0 (Chinese comments at golden :2315/:2324/:2325/
//                  :2332/:2345/:2354/:2355/:2357/:2359/:2366/:2368/:2396/:2401/:2404/:2406/
//                  :2411/:2414/:2424/:2432/:2433/:2439 transcribed character-for-character)
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem_shims.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "ainarm9045_w7_shims.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "cinitial.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "atester.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "mycylin.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "FormsFacade.h"

// -- INTEGRATOR HOLE #1 : golden ainarm2.h:180-185 (also commented out at golden
//    ainarm2.cpp:2305-2310).  Guarded so a sibling DoKnockShuttle part cannot collide.
#ifndef HT9045_eShakeShuttle_DEFINED
#define HT9045_eShakeShuttle_DEFINED
enum eShakeShuttle{essSetFlag=1,
                   essMoveRight,
                   essMoveLeft,
                   essKnockOn,
                   essKnockOff,
                   essEnd};                                                     // golden ainarm2.h:180-185
#endif
// -- INTEGRATOR HOLE #2 : golden ainarm2.cpp:2311 (one line above my range).
#ifndef HT9045_iShakeShuttleTask_DEFINED
#define HT9045_iShakeShuttleTask_DEFINED
int iShakeShuttleTask;                                                          // golden ainarm2.cpp:2311
#endif
//==============================================================================
bool DoShakeShuttle(int iShuttle, bool bNeedInitial)                            //Steven 20120801 : 修改抖抖功能
{
    static bool bInitial=true;
    static int iSCT;
    ishuttleShake=iShuttle;                                                     //kevin 20190107 add shuttle shake state
    if(bInitial || bNeedInitial)
    {
        iShakeShuttleTask=essSetFlag;
        bInitial=false;
        bShuttleKnock=true;                                                     //Jou 2013-03-08 修改敲敲功能
        ShakeShuttleDelay.SetMSAndOn(10000);                                    //Ifor 20180329 : add Shake Shuttle Time Out 判斷
        if(CUSTOMER_CODE==CC_KYEC_XILINX)
            RecordProcess("Shake Shuttle Initial");
        return false;
    }

    if(bRunAutoClean && TestIF.iAutoClean_Function &&
       TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)                            //Sam 20220705 : 會 Hang up AutoClean 不需要檢查。
    {
        if((iShuttle==0 && bZ1Isdownflag==true) || (iShuttle==1 && bZ2Isdownflag==true))
        {
            if(iShuttle==0)
                bZ1Isdownflag=false;
            else if(iShuttle==1)
                bZ2Isdownflag=false;
        }
    }
    else
    {
       if((iShuttle==0 && bZ1Isdownflag==true) ||
          (iShuttle==1 && bZ2Isdownflag==true))                                 //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
       {
           return false;
       }
    }

    switch(iShakeShuttleTask)
    {
        case essSetFlag:
            if((bRunAutoClean && TestIF.iAutoClean_Function && TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir) ||         //Steven 20211213 : For Auto Clean置偏
               ((IndexZCanMove[0]==true && IndexZCanMove[1]==true) &&                                                   //JerryYang 20190628 強制shuttle shake
                MOT[MInShuttle1+iShuttle].fCanMoveM==true &&
                MOT[MInShuttle1+iShuttle].fCanMove==true))                                                              //ChungHung 20150520 add fix Fix3 full 和 ShuttleShanke 衝突  // now Index Z1,Z2 not down
            {
                SetMotorAccelSpeed(MInShuttle1+iShuttle, SHSpeed_File.iShakeAccDec);                                    //Sam 20250326 : 新增 Shake 條件設定
                SetMotorScaleSpeed(MInShuttle1+iShuttle, IniConfig.iF01ShuttleShakeSpeed);

                IndexZCanMove[0]=false;
                IndexZCanMove[1]=false;
                bShuttleShake=true;                                                                                     // disable all process exclude inarm
                SetShuttlefCanMoveL(iShuttle, true, __FUNC__, IntToStr(essSetFlag));
                iSCT=SHSpeed_File.iShakeCycles;                                                                         //Sam 20250326 : 新增 Shake 條件設定
                iShakeShuttleTask=essMoveRight;
                ShakeDelay.SetSecAndOn(0.01);                                                                           //Sam 20250326 : 新增 Shake 條件設定
                break;
            }
            else if(bIndexWaitingInArmAway==true &&                                                                     //Steven 20171228 (Wei) : Index在等In Arm讓開
                   IndexZCanMove[0]==true &&
                   IndexZCanMove[1]==true &&
                   bUseFix3CylinderActive==false)
            {
                if(iShuttle==0 && MOT[MInShuttle1].fCanMoveM==false && InSHT1InLF())
                {
                    MOT[MInShuttle1].fCanMoveM=true;
                }

                if(iShuttle==1 && MOT[MInShuttle2].fCanMoveM==false && InSHT2InLF())
                {
                    MOT[MInShuttle2].fCanMoveM=true;
                }
            }
            else if(ShakeShuttleDelay.Off())                                                                            //Ifor 20180329 : add Shake Shuttle Time Out 判斷
            {
                AnsiString str1, str2, str3;
                str1.sprintf("Shake Shuttle Time Out!!");
                str3.sprintf("ShuttleShake:%d  Fix3Cylinder:%d", bShuttleShake, bUseFix3CylinderActive);
                str3.sprintf("Shake Shuttle Time Out!!, ShuttleShake:%d  Fix3Cylinder:%d, bIndexWaitingInArmAway:%d, IndexZCanMove[0]:%d, IndexZCanMove[1]:%d, bShuttleShake:%d, fCanMoveL:%d, fCanMoveM:d, fCanMove:%d",
                             bShuttleShake, bUseFix3CylinderActive, bIndexWaitingInArmAway, IndexZCanMove[0], IndexZCanMove[1], bShuttleShake, MOT[MInShuttle1+iShuttle].fCanMoveL, MOT[MInShuttle1+iShuttle].fCanMoveM, MOT[MInShuttle1+iShuttle].fCanMove);
                RecordProcess(str3);
                ShowMyMessage(str1, str2);
                bUseFix3CylinderActive=false;
                ShakeShuttleDelay.SetMSAndOn(10000);                                                                    //kevin 20190107 add time resetIfor 20180329 : add Shake Shuttle Time Out 判斷
                fMain->Pause("DoShakeShuttle");
            }
            break;
        case essMoveRight:
            if(ShakeDelay.Off()==false && SHSpeed_File.dShakeDelay!=0.0)                                                                                        //Sam 20250326 : 新增 Shake 條件設定
                break;

            if(MOT[MInShuttle1+iShuttle].MotorMoveShuttleShake(Prod.InSHT[iShuttle].iLeft+SHSpeed_File.iShakeDistance*100))                                     //Sam 20250326 : 新增 Shake 條件設定
            {
                ShakeDelay.SetSecAndOn(SHSpeed_File.dShakeDelay);                                                                                               //Sam 20250326 : 新增 Shake 條件設定
                iShakeShuttleTask=essMoveLeft;
            }
            break;
        case essMoveLeft:
            if(ShakeDelay.Off()==false && SHSpeed_File.dShakeDelay!=0.0)                                                //Sam 20250326 : 新增 Shake 條件設定
                break;

            if(MOT[MInShuttle1+iShuttle].MotorMoveShuttleShake(Prod.InSHT[iShuttle].iLeft))                             //JerryYang 20190628 shuttle shake專用command
            {
                iSCT--;
                if(iSCT<=0)
                {
                    iShakeShuttleTask=essEnd;
                    ShakeShuttleDelay.SetMSAndOn(50);
                }
                else
                {
                    ShakeDelay.SetSecAndOn(SHSpeed_File.dShakeDelay);                                                   //Sam 20250326 : 新增 Shake 條件設定
                    iShakeShuttleTask=essMoveRight;
                }
            }
            break;
        case essEnd:
            if(ShakeShuttleDelay.Off())
            {
                Cylinder[C_Shuttle_Knocker_1+iShuttle].Off();                   //Steven 20130108 : 避免蝦頭敲敲汽缸打開撞斷
                SetMotorAccelSpeed(MInShuttle1+iShuttle, SHSpeed.iSH1ACDCSp);   //Steven 20160601 : fix shuttle shake speed
                SetMotorScaleSpeed(MInShuttle1+iShuttle, SHSpeed.iSH1Sp);

                IndexZCanMove[0]=true;                                          //Steven 20200519 : fix for hang up
                IndexZCanMove[1]=true;
                bInitial=true;
                iShuttleLog=2;                                                  //JerryYang 20181121 (Steven) : add log,記錄in shuttle device是被震動馬達or搖搖功能導正
                DoKnockShuttle(iShuttle, true);
                SetShuttlefCanMoveL(iShuttle, false, __FUNC__, IntToStr(essEnd));
                return true;
            }
            break;
    }
    return false;
}

// =============================================================================
//  DoKnockShuttle(int iShuttle, bool bNeedInitial)   golden ainarm2.cpp:2450-2533
//  (Jou 2013-03-08 -- knock-shuttle rework)
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- the 'knock the shuttle' recovery SM (F14).  Once the shuttle motor may move
//    and neither index Z is down it locks both index Zs out, raises bShuttleShake, lets
//    the shuttle travel to its LEFT position, then -- if F14 is enabled and the knocker
//    cylinder is present -- pulses that cylinder iF14KnockShuttleNo times with
//    dF14KnockShuttleInterval seconds between transitions, and finally closes the
//    cylinder, re-arms bInitial, clears bShuttleKnock and releases both index Zs.
//    Returns true exactly once, in essEnd.
//
//  WAVE SCOPE -- DoKnockShuttle (golden :2450-2533) ACTIVE, no gates.
//  GATE REGISTER -- (empty: no #if 0 in this body.)
//
//  ABSENCE FINDING AND WHAT I DID ABOUT IT (TRAP 2).  The enumerators this body
//  switches on -- essSetFlag / essMoveLeft / essKnockOn / essKnockOff / essEnd -- exist
//  NOWHERE in the port tree, and neither does a port ainarm2.h:
//    ABSENCE COMMAND 1 -- python walk of every .cpp/.h/.hpp/.c under
//      D:/HT9045/HT9011UC_Cpp_V3.33.906.0 (34,016 files; .svn/.git/_w7e_parts/build
//      excluded), word-boundary match on
//      essSetFlag|essMoveLeft|essKnockOn|essKnockOff|essEnd  ->  0 hits.
//      MEASURED 2026-08-11 15:05.  (Script kept: scratchpad/scan.py.)
//    ABSENCE COMMAND 2 -- ls D:/HT9045/HT9011UC_Cpp_V3.33.906.0/ainarm2.h
//      -> No such file or directory.  MEASURED 2026-08-11 14:41.
//  Their golden home is ainarm2.h:180-185, reproduced below VERBATIM (the values are
//  load-bearing: essSetFlag=1, then essMoveRight, essMoveLeft, essKnockOn, essKnockOff,
//  essEnd -- and note golden ainarm2.cpp:2305-2310 keeps a commented-out copy of the
//  same enum).  It is a TYPE, not one of the 37 file-scope globals the integrator owns,
//  so I emit it -- behind a guard macro, because the sibling parts of THIS wave
//  (DoShakeShuttle golden :2315-2447, DoKnockShuttleFirst golden :2536-, DoVibrate*
//  golden :3460-/:3528-) need the same enum and a second unguarded copy in the stitched
//  TU is a redefinition error.  INTEGRATOR: keep ONE copy (ideally in a new port
//  ainarm2.h) and delete the rest.
//
//  GLOBALS I ONLY USE, NEVER DEFINE (integrator's list): iKnockShuttleTask (golden
//  :2449) and ShakeShuttleDelay (golden :2313, TQPF_Timer).  bShuttleShake (golden :78)
//  and bShuttleKnock (golden :79) already exist in this tree; IndexZCanMove[2] is
//  golden's own mid-file extern (golden :2312).  No definition of any of them is here.
//
//  GOLDEN BUG, PRESERVED (not fixed): essSetFlag sets bShuttleShake=true but this SM
//  never clears it, while essEnd clears bShuttleKnock, which this SM never sets -- the
//  pair is crossed in golden.  Behaviour kept byte-for-byte.  SECOND GOLDEN QUIRK:
//  essSetFlag is `if(...){...break;} break;` -- the inner break is redundant, both arms
//  leave the switch identically.  Kept verbatim.
//
//  RETIRE-AT-INTEGRATION: `bool DoKnockShuttle(int, bool) { return true; }` at
//  ainarm9045_w7_shims.cpp:39 (decl ainarm9045_w7_shims.h:48 WITH the default argument
//  -- so this definition must NOT repeat `=false`; golden's definition does not
//  either).  Live call sites: ainarm9045.cpp:1690 (init) and :1734 (poll), plus a local
//  extern at ainarm9045.cpp:1417.  TRAP-1 shape (b): the shim answers 'knock finished'
//  on the very first poll, so today the SM never actually knocks.
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
// ---- eShakeShuttle -- golden ainarm2.h:180-185, VERBATIM (see banner) --------
#ifndef HT9045_eShakeShuttle_DEFINED
#define HT9045_eShakeShuttle_DEFINED
enum eShakeShuttle{essSetFlag=1,
                   essMoveRight,
                   essMoveLeft,
                   essKnockOn,
                   essKnockOff,
                   essEnd};
#endif
//------------------------------------------------------------------------------
bool DoKnockShuttle(int iShuttle, bool bNeedInitial)                            //Jou 2013-03-08 修改敲敲功能
{
    static bool bInitial=true;
    static int iSCT=3;
    if(bInitial || bNeedInitial)
    {
        iKnockShuttleTask=essSetFlag;
        bInitial=false;

        if(CUSTOMER_CODE==CC_KYEC_XILINX)
            RecordProcess("Knock Shuttle Initial");
        return false;
    }

    switch(iKnockShuttleTask)
    {
        case essSetFlag:
            if(MOT[MInShuttle1+iShuttle].fCanMoveM==true &&
               MOT[MInShuttle1+iShuttle].fCanMove==true)                        // now Index Z1,Z2 not down
            {
                IndexZCanMove[0]=false;
                IndexZCanMove[1]=false;
                bShuttleShake=true;                                             // disable all process exclude inarm
                SetShuttlefCanMoveL(iShuttle, true, __FUNC__, IntToStr(essSetFlag));
                iKnockShuttleTask=essMoveLeft;
                break;
            }
            break;
        case essMoveLeft:
            if(MOT[MInShuttle1+iShuttle].MotorMove(Prod.InSHT[iShuttle].iLeft))
            {
                if(IniConfig.bF14KnockShuttle &&
                   Cylinder[C_Shuttle_Knocker_1+iShuttle].Enable)               //Steven 20120801 : Shuttle敲敲
                {
                    iKnockShuttleTask=essKnockOn;
                    ShakeShuttleDelay.SetSecAndOn(IniConfig.dF14KnockShuttleInterval);
                    iSCT=IniConfig.iF14KnockShuttleNo;
                }
                else
                {
                    iKnockShuttleTask=essEnd;
                    ShakeShuttleDelay.SetMSAndOn(50);
                }
                SetShuttlefCanMoveL(iShuttle, false, __FUNC__, IntToStr(essMoveLeft));
            }
            break;
        case essKnockOn:
            if(ShakeShuttleDelay.Off())
            {
                Cylinder[C_Shuttle_Knocker_1+iShuttle].On();
                ShakeShuttleDelay.SetSecAndOn(IniConfig.dF14KnockShuttleInterval);
                iKnockShuttleTask=essKnockOff;
            }
            break;
        case essKnockOff:
            if(ShakeShuttleDelay.Off())
            {
                Cylinder[C_Shuttle_Knocker_1+iShuttle].Off();
                ShakeShuttleDelay.SetSecAndOn(IniConfig.dF14KnockShuttleInterval);
                iSCT--;
                if(iSCT<=0)
                {
                    iKnockShuttleTask=essEnd;
                }
                else
                {
                    iKnockShuttleTask=essKnockOn;
                }
            }
            break;
        case essEnd:
            if(ShakeShuttleDelay.Off())
            {
                Cylinder[C_Shuttle_Knocker_1+iShuttle].Off();                   //Steven 20130108 : 避免蝦頭敲敲汽缸打開撞斷
                bInitial=true;
                bShuttleKnock=false;
                IndexZCanMove[0]=true;                                          //Steven 20200519 : fix for hang up
                IndexZCanMove[1]=true;
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------

//==============================================================================
//  DoKnockShuttleFirst(int,bool)  --  golden ainarm2.cpp:2536-2616
//
//  ROLE          Pre-load "first knock" SM for one in-shuttle: freeze both index
//                Z interlocks, drive the shuttle to its LEFT position, then pulse
//                the Shuttle-Knocker cylinder On/Off iF14KnockShuttleNoFirst times
//                at dF14KnockShuttleIntervalFirst spacing, and release the
//                interlocks on finish.  Returns true exactly once, at essEnd.
//  WAVE SCOPE    DoKnockShuttleFirst()      ACTIVE   golden :2536-2616
//  GATE REGISTER (none -- every symbol has a real linkable home)
//                  MOT[] Motor/mymotor.h:385; fCanMove :141 / fCanMoveM :143 /
//                    fCanMoveL :144; MotorMove(int) :163.
//                  IndexZCanMove[2]  ainarm9045_w7_shims.cpp:47 (real bool[2]).
//                  bShuttleShake     csystem_shims.cpp:38 (real bool).
//                  bShuttleKnock     ainarm9045_w7_shims.cpp:48 (real bool).
//                  SetShuttlefCanMoveL(int,bool,AnsiString,AnsiString="")
//                    aHotPlateSubstrate.h:1070, REAL body aHotPlateSubstrate.cpp:1567.
//                  Cylinder[] mycylin.h:176; .Enable :72, .On() :112, .Off() :113.
//                  C_Shuttle_Knocker_1 cmydef.h.  Prod.InSHT[].iLeft cprod.h:406/:58.
//                  IniConfig.bF14_1KnockShuttleFirst / dF14KnockShuttleIntervalFirst /
//                    iF14KnockShuttleNoFirst  Config.h.
//                  __FUNC__ canary_support.h:45; IntToStr vclcompat/SysUtils.h:23.
//                  ShakeShuttleDelay -- golden ainarm2.cpp:2313 `TQPF_Timer
//                    ShakeShuttleDelay;`, one of the 37 file-scope globals the
//                    INTEGRATOR adds.  NOT defined here.  Off()/SetSecAndOn(double)/
//                    SetMSAndOn(DWORD) are myTimer.h:30/:38/:39.  It must be typed
//                    TQPF_Timer, not atester_shims.h:463 HTimer, whose Off() is
//                    hard-coded true and would collapse every knock interval to 0.
//                  iKnockShtFirstTask -- golden :2535, also integrator-owned.
//                    NOT defined here.
//
//  RETIRES A STUB: ainarm9045_w7_shims.cpp:40 `bool DoKnockShuttleFirst(int,bool)
//    { return true; }` (decl ainarm9045_w7_shims.h:49, plus a duplicate extern at
//    ainarm9045.cpp:1418) must be REMOVED by the integrator.  NOT written `static`
//    (TRAP 1 shape (d) against those declarations).
//  ** LANDING HAZARD, MEASURE BEFORE COMMITTING ** the 2 call sites
//    ainarm9045.cpp:1493/:1504 are LIVE (inside the un-gated W7-A1
//    DoInArmCheckShuttleFloating).  Today they get "knocked, done" on the first
//    poll.  After retirement they must walk essSetFlag -> essMoveLeft ->
//    (essKnockOn/essKnockOff)* -> essEnd, which requires MOT[MInShuttle1+n]
//    .fCanMoveM && .fCanMove to be true and MotorMove() to converge.  Offline,
//    Cylinder[C_Shuttle_Knocker_1+n].Enable is false, so essMoveLeft is expected
//    to jump straight to essEnd with a 50 ms dwell -- but that is a PREDICTION
//    about the sim HAL, not a measurement.  Run ctest on a fresh build dir.
//==============================================================================
// eShakeShuttle -- golden ainarm2.h:180-185, transcribed verbatim.  This is a
// TYPE, not one of the 37 integrator-owned file-scope globals, and the port has
// NO copy of it anywhere (command + timestamp below), yet DoKnockShuttleFirst
// cannot compile without it.  Emitted #ifndef-guarded because DoShakeShuttle
// (golden :2318) and DoKnockShuttle (golden :2450) share this enum, so a sibling
// agent translating either will need an identical copy in the SAME stitched TU;
// the guard makes the duplicate harmless (this is the idiom the port ainarm2.cpp
// already uses at :44 for bDestoryOnSht).  An enum definition has no linkage, so
// it can never cause `multiple definition` -- only a same-TU redefinition error,
// which the guard prevents.
//   ABSENCE COMMAND (from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//     grep -rn --include=*.h --include=*.cpp -w "essMoveLeft\|essKnockOn\|eShakeShuttle" . | grep -v /build | grep -v /tools/
//     -> zero hits.  (The bare string "essEnd" DOES hit cmydef.h:5695, but that is
//        a substring of `sProcessEndInfo`, not this enumerator -- checked, not
//        assumed.)  RUN AT: 20260811 06:39:36 local (Asia/Taipei).  TRAP 2: re-run
//        at integration; if a sibling landed a real ainarm2.h, drop this block.
#ifndef HT9045_eShakeShuttle_DEFINED
#define HT9045_eShakeShuttle_DEFINED
enum eShakeShuttle{essSetFlag=1,
                   essMoveRight,
                   essMoveLeft,
                   essKnockOn,
                   essKnockOff,
                   essEnd};
#endif
bool DoKnockShuttleFirst(int iShuttle, bool bNeedInitial)                       //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
{
    static bool bInitial=true;
    static int iSCT=3;
    if(bInitial || bNeedInitial)
    {
        iKnockShtFirstTask=essSetFlag;
        bInitial=false;
        return false;
    }

    switch(iKnockShtFirstTask)
    {
        case essSetFlag:
            if(MOT[MInShuttle1+iShuttle].fCanMoveM==true &&
               MOT[MInShuttle1+iShuttle].fCanMove==true)                        // now Index Z1,Z2 not down
            {
                IndexZCanMove[0]=false;
                IndexZCanMove[1]=false;
                bShuttleShake=true;                                             // disable all process exclude inarm
                SetShuttlefCanMoveL(iShuttle,true,__FUNC__,IntToStr(essSetFlag));
                iKnockShtFirstTask=essMoveLeft;
                break;
            }
            break;
        case essMoveLeft:
            if(MOT[MInShuttle1+iShuttle].MotorMove(Prod.InSHT[iShuttle].iLeft))
            {
                if(IniConfig.bF14_1KnockShuttleFirst && Cylinder[C_Shuttle_Knocker_1+iShuttle].Enable)
                {
                    iKnockShtFirstTask=essKnockOn;
                    ShakeShuttleDelay.SetSecAndOn(IniConfig.dF14KnockShuttleIntervalFirst);
                    iSCT=IniConfig.iF14KnockShuttleNoFirst;
                }
                else
                {
                    iKnockShtFirstTask=essEnd;
                    ShakeShuttleDelay.SetMSAndOn(50);
                }
            }
            break;
        case essKnockOn:
            if(ShakeShuttleDelay.Off())
            {
                Cylinder[C_Shuttle_Knocker_1+iShuttle].On();
                ShakeShuttleDelay.SetSecAndOn(IniConfig.dF14KnockShuttleIntervalFirst);
                iKnockShtFirstTask=essKnockOff;
                MOT[MInShuttle1+iShuttle].fCanMoveL=false;                      //Steven 20200422 : add protection
                SetShuttlefCanMoveL(iShuttle, false, __FUNC__, IntToStr(essKnockOn));
            }
            break;
        case essKnockOff:
            if(ShakeShuttleDelay.Off())
            {
                Cylinder[C_Shuttle_Knocker_1+iShuttle].Off();
                ShakeShuttleDelay.SetSecAndOn(IniConfig.dF14KnockShuttleIntervalFirst);
                iSCT--;
                if(iSCT<=0)
                {
                    iKnockShtFirstTask=essEnd;
                }
                else
                {
                    iKnockShtFirstTask=essKnockOn;
                }
            }
            break;
        case essEnd:
            if(ShakeShuttleDelay.Off())
            {
                Cylinder[C_Shuttle_Knocker_1+iShuttle].Off();
                bInitial=true;
                bShuttleKnock=false;
                IndexZCanMove[0]=true;                                          //Steven 20200519 : fix for hang up
                IndexZCanMove[1]=true;
                return true;
            }
            break;
    }
    return false;
}

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 02786 : DisableAutoSiteMapWhenCleanOut
//  Golden: ainarm2.cpp:2786-2808 (cp950, CRLF).
//
//  ROLE          : abandons Auto-Site-Map mode when clean-out finds the Loader empty at
//                  site-mapping step 2: declares the site-map check OK, switches the run
//                  start mode back to rsmContinuStart, zeroes BOTH HotPlate trays'
//                  SiteMapData over the configured X*Y division grid (Ifor 20210913: so a
//                  later run does not inherit Has-Null-IC marks), clears the HotPlate-save
//                  flag, rewinds the in-arm SM via InitInArmTask() and refreshes the
//                  test-head composite display.
//  WAVE SCOPE    : DisableAutoSiteMapWhenCleanOut()  golden ainarm2.cpp:2786-2808  ACTIVE
//  GATE REGISTER : (none)  iResetSiteMappingStep cmydef.h:5223 ; LastSet LastSet.h:587 (via
//                  canary_support.h:59) ; rsmAutoSiteMap/rsmContinuStart MachineType.h:607/:603 ;
//                  bSiteMappingCHKOK cmydef.h:3276 ; SetRunStartMode aHotPlateSubstrate.h:897 ;
//                  HotPlateForm cprod.h:1361 ; MOT[MMPlate1+i].Tray.SiteMapData
//                  Motor/mymotor.h:336 + mytray.h:52 + cmydef.h:2257 ;
//                  bAutoSiteMapHotplateSave cmydef.h:4373 ; InitInArmTask() = my part 01566 ;
//                  fMain->ShowTestHeadComp forms/fMain.h:156.
//  HAND-OFF (pre-existing TRAP-1(b), NOT introduced here): two of the six effects land on
//                  no-op bodies today -- SetRunStartMode(int) is `{}` at
//                  aHotPlateSubstrate.cpp:1063 (so the mode does NOT actually leave
//                  rsmAutoSiteMap) and TfMain::ShowTestHeadComp is an empty facade method.
//                  I translate both calls ACTIVE and un-degraded; retiring those two stubs
//                  is somebody else's wave.
//  INTEGRATOR    : RETIRE the no-op stub `void DisableAutoSiteMapWhenCleanOut() {}` at
//                  aHotPlateSubstrate.cpp:1601.  BEHAVIOUR CHANGE and it is the faithful
//                  direction: today the ~20 variant-SM call sites (e.g.
//                  ainarm9045_1x1_1.cpp:1912) do nothing at all.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0 (Chinese comments at golden :2786/:2799
//                  transcribed character-for-character)
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "FormsFacade.h"
//==============================================================================
void DisableAutoSiteMapWhenCleanOut()                                           //Steven 20220902 : 修正Auto Site Map, Loader沒有IC
{
    if(iResetSiteMappingStep==2 && LastSet.iRunStartMode==rsmAutoSiteMap)
    {
        bSiteMappingCHKOK=true;
        SetRunStartMode(rsmContinuStart);

        for(int i=0; i<2; i++)
        {
            for(int j=0; j<HotPlateForm.XDivision; j++)
            {
                for(int k=0; k<HotPlateForm.YDivision; k++)
                {
                    MOT[MMPlate1+i].Tray.SiteMapData[j][k]=0;                   //Ifor 20210913 add: 清除Auto Site Mapping 補回Has Null IC資料
                }
            }
        }
        bAutoSiteMapHotplateSave=false;
        InitInArmTask();

        fMain->ShowTestHeadComp(false);
    }
}

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:2812-2834 -----------------------------
//  ROLE          -- one-shot Auto-Site-Mapping position checkpoint.  When a site-
//                   mapping run has just saved a HotPlate position and the mapping
//                   check is not yet OK, consume the save flag and advance the
//                   site cursor -- but for JCET (bUSEJCETSiteMapMode) and ASE
//                   Malaysia (CC_ASE_M) only advance once the previous site's test
//                   result has come back PASS -- then run DoSiteMappingCHK().
//  WAVE SCOPE    -- DoCheckAutoSiteMappingPosition()      ACTIVE   golden :2812-2834
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  FIDELITY NOTES
//   * bAutoSiteMapHotplateSave is cleared BEFORE the inner ready/CHK-OK test, so
//     the flag is consumed even when the body does nothing.  Order preserved.
//   * The customer-code condition is an OR of a CosFunction flag and a hard
//     CUSTOMER_CODE compare (Ifor 20171013 / Ifor 20180417) -- kept verbatim,
//     including that CC_ASE_M is a plain #define (MachineType.h:309, value 937).
//   * `DoSiteMappingCHK()` is called with no argument; the port declares
//     `void DoSiteMappingCHK(bool bAdd=true)` (csystem.h:216) -- matching golden.
//     PRE-EXISTING TREE HAZARD, not introduced here: csystem_shims.cpp:261 also
//     defines `void DoSiteMappingCHK(bool){}` alongside the real csystem.cpp:26644
//     body, so which one this call reaches depends on archive/link order (TRAP 1
//     shape (b)/(c)).  Worth an integrator check; I touched neither file.
//  INTEGRATE-STEP NOTE: aHotPlateSubstrate.cpp:1529 currently holds the offline
//  no-op `void DoCheckAutoSiteMappingPosition() {}` (declared
//  aHotPlateSubstrate.h:1143) -- REMOVE it as part of the stitch.
// ---------------------------------------------------------------------------
//AI(k3-ainarm2) 20260811: HOIST ME to ainarm2.cpp's include head.
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "csystem.h"
// ---------------------------------------------------------------------------
void DoCheckAutoSiteMappingPosition()
{
    if(bRunAutoSiteMapping==true &&
       bAutoSiteMapHotplateSave==true)                                          //Ifor 20180518 : add 簡化Site Mapping 旗標
    {
        bAutoSiteMapHotplateSave=false;
        if(bAutoSiteMapHotplateReady==true &&
           bSiteMappingCHKOK==false        )
        {
            if(CosFunction.bUSEJCETSiteMapMode==true ||                         //Ifor 20171013 (wei) : 長電Auto Site Mapping 需Pass才可測試下一顆
               CUSTOMER_CODE==CC_ASE_M)                                         //Ifor 20180417: add ASE_MAuto Site Mapping 需Pass才可測試下一顆
            {
                if(bAutoSiteMapWaitTestPass==true)
                    iDoSiteMappingStep++;
            }
            else
            {
                iDoSiteMappingStep++;
            }
            DoSiteMappingCHK();
        }
    }
}

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:2840-2914 -----------------------------
//  ROLE          -- recovery SM for "tray map data error": the ICs the in-arm just
//                   picked came from tray cells the map says should be empty, so
//                   they must go BACK.  case 1 blows (Destroy) every flagged
//                   nozzle and arms a 0.5 s release delay; case 100 waits it out;
//                   case 150 raises Z to plate-safe; case 200 builds the offending
//                   nozzle-name list, raises WAR0122 (SKIP only), writes NULL_IC
//                   back into each recorded tray cell, clears the per-nozzle flags
//                   and coordinates, and returns true.  bReset arms the SM and
//                   raises bCheckTrayDevice.
//  WAVE SCOPE    -- ProcessTrayMapDataError(bool)         ACTIVE   golden :2840-2914
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  FIDELITY NOTES
//   * `int &Task=iProcessTrayMapDataErrorTask;` -- a REFERENCE to the file-scope
//     cursor (golden :2839).  That global is on the main loop's 37-global list and
//     is deliberately NOT emitted here.
//   * The reset path returns bResult (false) rather than a literal -- kept.
//   * case 200 walks the grid TWICE (once to build ErrPart, once to write back)
//     with the alarm in between; the second walk is additionally guarded by
//     `iTrayMapDataX/Y != -1`.  Both walks and the guard asymmetry kept verbatim.
//   * GOLDEN DEFECT KEPT: the write-back clears bTrayMapDataError / iTrayMapDataX /
//     iTrayMapDataY INSIDE the `!=-1` guard, so a nozzle flagged with -1
//     coordinates keeps its error flag forever and would re-fire this SM every
//     cycle.  NOT fixed.  (Reachable only if a flag is set without coordinates;
//     ainarm9045.cpp:5107-5117 always writes both together, so it is latent.)
//   * The switch has no fall-through: every case ends in `break`, including
//     case 200 (:2911) after `bResult=true;`.  Verified line by line.
//   * The definition has NO default argument (golden ainarm2.h:197 carries
//     `bool bReset=false`; the port's declaration ainarm9045_2x4_16_shims.h:64 has
//     none).  I do not add one -- that would clash with a declaration default.
//  INTEGRATE-STEP NOTE: ainarm9045_2x4_16_shims.cpp:39 currently holds the offline
//  stub `bool ProcessTrayMapDataError(bool) { return false; }` -- REMOVE it as part
//  of the stitch.  It returned FALSE forever, meaning the recovery could never
//  report completion; the real body can now return true, which is a behaviour
//  change for its callers.  Measure.
// ---------------------------------------------------------------------------
//AI(k3-ainarm2) 20260811: HOIST ME to ainarm2.cpp's include head.
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
// ---------------------------------------------------------------------------
bool ProcessTrayMapDataError(bool bReset)
{
    AnsiString ErrPart="";
    bool bResult=false;
    if(bReset)
    {
        iProcessTrayMapDataErrorTask=1;
        bCheckTrayDevice=true;                                                  //wei 20170302 (Steven) Tray map data error
        return bResult;
    }

    int &Task=iProcessTrayMapDataErrorTask;
    switch(Task)
    {
        case 1:
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(bTrayMapDataError[i][j])
                    {
                        InArmSuck.Suck[i][j].Destroy();
                    }
                }
            }
            InArmReleaseDelay.SetSecAndOn(0.5);
            Task=100;
            break;
        case 100:
            if(InArmReleaseDelay.Off())
            {
                Task=150;
            }
            break;
        case 150:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=200;
            }
            break;
        case 200:
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(bTrayMapDataError[i][j])
                    {
                        ErrPart+=InArmSuck.Suck[i][j].sName;
                    }
                }
            }
            ShowErrorMessage("WAR0122", K_SKIP, MInArmX, false, ErrPart);       //wei 20170317 (Steven) 因重覆所以修改 WAR0121 --> WAR0122

            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(iTrayMapDataX[i][j]!=-1 && iTrayMapDataY[i][j]!=-1)
                    {
                        if(bTrayMapDataError[i][j])
                        {
                            MOT[MMTrayY].SetTraySingleData(iTrayMapDataX[i][j], iTrayMapDataY[i][j], NULL_IC);
                        }
                        bTrayMapDataError[i][j]=false;
                        iTrayMapDataX[i][j]=-1;
                        iTrayMapDataY[i][j]=-1;
                    }
                }
            }

            bResult=true;
            break;
    }
    return bResult;
}

//==============================================================================
//  LoadTrayCanUse8Suck()  --  golden ainarm2.cpp:2916-2927
//
//  ROLE          Ask whether the loader tray's Y pitch divides the arm Y pitch
//                exactly, i.e. whether all 8 nozzles can pick in one shot.
//                Returns 1 when TestIF.iARM_Y_PITCH % TrayYPitch == 0, else 0.
//  WAVE SCOPE    LoadTrayCanUse8Suck()      ACTIVE   golden :2916-2927
//  GATE REGISTER (none -- every symbol has a real linkable home)
//                  TrayForm cprod.h:1359 (SYSTEM_TRAY_FORM, .Loader.iTrayType);
//                  UserDefForm[4] cprod.h:1363 (TRAY_TYPE_PARA, .YPitch is
//                  `double` at cprod.h:1259); TestIF.iARM_Y_PITCH cprod.h:1931 (int).
//
//  INTEGER SEMANTICS ARE LOAD-BEARING, transcribed exactly:
//    * `int TrayYPitch=UserDefForm[Ld].YPitch;` narrows double->int (truncation
//      toward zero).  Golden does this; it is NOT widened to double here, and no
//      float helper was substituted.  Widening would turn 7999.6 -> 7999 into a
//      non-integral modulo and flip the answer.
//    * `%` stays integer modulo on ints.
//  GOLDEN DEFECT PRESERVED: if UserDefForm[Ld].YPitch truncates to 0 (unset tray
//    type / uninitialised setup file) this is integer modulo by ZERO -- UB, and on
//    x86 a divide-error trap.  Golden has no guard; none is added, per the tree's
//    standing policy that silently substituting a value in a POSITION / pitch
//    calculation is a worse failure mode than crashing (AutoClean/AutoClean.cpp
//    :4505-4507 records the same decision for NULL InArmOffSet[]).  Flagged, not
//    fixed.
//
//  SHADOW TO RETIRE (TRAP 1 shape (d), pre-existing, NOT introduced here):
//    aoutarm.cpp:559 holds `static int LoadTrayCanUse8Suck() { return 0; }` --
//    internal linkage, so it will NOT collide with this definition at link time,
//    and the build will go GREEN while aoutarm.cpp keeps calling its own no-op 0.
//    A green build is therefore NOT evidence that aoutarm is using this body.
//    The integrator must delete that static and let aoutarm.cpp see the real one
//    (which returns 1 on machines where the pitches divide -- a behaviour change
//    on aoutarm's ORDINARY-vs-8-nozzle path, so it needs its own measurement).
//    Golden ainarm2.h:200 declares this non-static, which is exactly why that
//    static is a latent hard error the day ainarm2.h lands.
//==============================================================================
int LoadTrayCanUse8Suck()                                                       //kevin 20171120 (wei) 判斷LOAD TRAY Y PITCH是否是 ARM Y PITCH可以一次8吸嘴吸IC
{
    int Ld=TrayForm.Loader.iTrayType;
    int TrayYPitch=UserDefForm[Ld].YPitch;
    int iPitchY=TestIF.iARM_Y_PITCH%TrayYPitch;

    if(iPitchY==0)
    {
        return 1;
    }
    return 0;
}

//------------------------------------------------------------------------------
//  InArmAddSpeedDisplay()  --  golden ainarm2.cpp:2934-2937
//  Wave PT-W7e, part k1-ainarm2.  Translator: AI(W7E-k1-ainarm2) 20260811
//
//  ROLE  Bump the in-arm speed-display counter by one (the UI shows how many
//        speed-up nudges the arm took this cycle).  Pure counter, no HAL.
//
//  WAVE SCOPE  InArmAddSpeedDisplay  ACTIVE  golden :2934-2937.
//              InArmSubSpeedDisplay (golden :2939-2942) is NOT in this part.
//
//  GATE REGISTER  none -- this part contains no #if 0.
//
//  *** HOMECOMING COLLISION -- THE INTEGRATOR MUST DELETE THE OTHER COPY ***
//  aHotPlateSubstrate.cpp:1508 already holds a LIVE one-line body
//  (`void InArmAddSpeedDisplay() { iInArmSpeed1++; }', declared
//  aHotPlateSubstrate.h:1072), with its InArmSubSpeedDisplay twin on :1509.
//  Delete the Add copy there when this part lands, or the link fails with a
//  duplicate definition.  iInArmSpeed1 itself is golden cmydef.h:4557, stays there.
//  CORRECTED after a TRAP 2 re-run: this banner first said "leave the Sub twin
//  alone (golden :2939 is not in this wave's parts)".  Re-listed
//  2026-08-11T06:48:10+0800 -- _w7e_parts/02939_InArmSubSpeedDisplay.txt EXISTS, so
//  BOTH aHotPlateSubstrate.cpp:1508 AND :1509 must be deleted together.  Re-check at
//  integration; do not delete :1509 if that sibling part is dropped.
//  NOTE for the wave census: this is one of THREE targets in MY part -- and 50 of the
//  71 parts wave-wide (measured 06:48:10) -- that the target list reported as missing
//  while a live body existed in a DIFFERENT port file.  See the k1 report.
//
//  Big5: the trailing comment (golden :2934, including the full-width colon in
//  "(Steven)：Add Speed Display") is transcribed character for character.
//------------------------------------------------------------------------------
void InArmAddSpeedDisplay()                                                     //KaiChen 20171225 (Steven)：Add Speed Display
{
    iInArmSpeed1++;
}

//==============================================================================
//== ROLE: InArmSubSpeedDisplay -- decrement the in-arm speed-display counter
//==       iInArmSpeed1 by one (the UI speed override step-down).
//==
//== WAVE SCOPE (k2-ainarm2):
//==   InArmSubSpeedDisplay  golden ainarm2.cpp:2939-2942  -- ACTIVE
//==     (golden lines emitted VERBATIM; ZERO gates)
//==
//== GATE REGISTER: (none)
//==   iInArmSpeed1 -- declared cmydef.h:4570 (int).  Nothing else is touched.
//==   NOTE the neighbouring globals iOneKitPreciser (golden :2944) and
//==   bOneKitPreciser (golden :2945) sit immediately AFTER this function in golden
//==   and are on the main loop's file-scope-globals list; this part deliberately
//==   stops at golden :2942 and emits neither.
//==
//== INTEGRATION NOTE -- THIS IS A DUPLICATE, NOT A STUB RETIREMENT:
//==   port aHotPlateSubstrate.cpp:1509 ALREADY carries a faithful body for this
//==   function and the declaration is at aHotPlateSubstrate.h:1073.  Landing this
//==   part is therefore a pure MOVE to golden's real home with ZERO behaviour
//==   change; the main loop must delete aHotPlateSubstrate.cpp:1509 or drop this
//==   part, since keeping both is a multiple-definition link error.  There are
//==   many live callers across the in-arm variant TUs, so unlike parts
//==   00748/02760/02770 this one really is reachable -- it just already works.
//==   Not static -- shape (d) avoided.
//==
//== Big5 -> UTF-8: the full-width colon in golden's trailing comment
//== (KaiChen 20171225 (Steven)) is transcribed character for character from cp950.
//== Zero U+FFFD.
//==============================================================================
void InArmSubSpeedDisplay()                                                     //KaiChen 20171225 (Steven)：Add Speed Display
{
    iInArmSpeed1--;
}

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 03460 : DoVibrateShuttle
//  Golden: ainarm2.cpp:3460-3525 (cp950, CRLF).
//
//  ROLE          : the in-shuttle VIBRATION-MOTOR sub-SM.  bNeedInitial arms case 1 and
//                  returns false.  case 1 switches SwShuttleVibration1/2 on for
//                  iF23ShuttleVibrationTime*100 ms, accrues the per-shuttle cumulative
//                  vibrator run-time counter (IniConfig.iVibratorSht1/2) and counts the
//                  burst.  case 100 switches the motor off and either finishes (case 200)
//                  or, while under iF23ShuttleVibrationCount, waits 1 s, logs MES2155 and
//                  loops back through case 150 -> case 1.  case 200 waits the 1 s settle
//                  (so an IC lifted by the vibration can drop back), marks iShuttleLog=1,
//                  zeroes the burst counter and returns true.
//  WAVE SCOPE    : DoVibrateShuttle(int,bool)  golden ainarm2.cpp:3460-3525  ACTIVE
//  GATE REGISTER : (none)  SW[] myswitch.h:43 with SwShuttleVibration1/2 cmydef.h:1872-1873 ;
//                  VibrationShuttleDelay is the integrator's golden :87 TQPF_Timer ;
//                  iVibrateShuttleTask is the integrator's golden :3459 ; IniConfig
//                  Config.h:1499 (iF23ShuttleVibrationTime :690, Count :691, iVibratorSht1 :249) ;
//                  NewRecordProcess acatchtray_shims.h:388 ; iShuttleLog cmydef.h:4910 ;
//                  CUSTOMER_CODE/CC_KYEC_XILINX + RecordProcess as in part 02315.
//  FIDELITY NOTE : golden :3486/:3490 accumulate `int(IniConfig.iF23ShuttleVibrationTime/10.0)`
//                  -- that division is FLOATING in golden (literal 10.0) and is transcribed
//                  exactly.  It is NOT an int/int to protect; equally it must NOT be
//                  "tidied" to /10, which would change the truncation for odd values.
//  INTEGRATOR    : RETIRE `bool DoVibrateShuttle(int,bool) { return true; }` at
//                  ainarm9045_w7_shims.cpp:41 (keep the ainarm9045_w7_shims.h:50
//                  declaration -- the default argument lives there and golden's definition
//                  does not repeat it).  BEHAVIOUR CHANGE and it is the faithful direction:
//                  the stub reported "vibration finished" on the FIRST poll, so
//                  ainarm9045.cpp:1740 never actually pulsed SW[] nor waited.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0 (Chinese comments at golden :3460/:3484/:3496/
//                  :3502/:3503/:3504/:3510/:3516/:3518 transcribed character-for-character)
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "myswitch.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "acatchtray_shims.h"
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "canary_support.h"
//==============================================================================
bool DoVibrateShuttle(int iShuttle, bool bNeedInitial)                          //JerryYang 20190123 shuttle震動馬達
{
    static bool bInitial=true;
    if(bInitial || bNeedInitial)
    {
        iVibrateShuttleTask=1;
        bInitial=false;

        if(CUSTOMER_CODE==CC_KYEC_XILINX)
            RecordProcess("Vibrate Shuttle Initial");
        return false;
    }
    AnsiString str;
    int iShuttleVibration=0;
    static int iShuttleVibraCount=0;
    if(iShuttle==0)
        iShuttleVibration=SwShuttleVibration1;
    else
        iShuttleVibration=SwShuttleVibration2;
    switch(iVibrateShuttleTask)
    {
        case 1:
            SW[iShuttleVibration].On();
            VibrationShuttleDelay.SetMSAndOn(IniConfig.iF23ShuttleVibrationTime*100);
            if(iShuttle==0)                                                     //JerryYang 20200612 振動馬達作動時間累計
            {
                IniConfig.iVibratorSht1=IniConfig.iVibratorSht1+int(IniConfig.iF23ShuttleVibrationTime/10.0);
            }
            else
            {
                IniConfig.iVibratorSht2=IniConfig.iVibratorSht2+int(IniConfig.iF23ShuttleVibrationTime/10.0);
            }
            iVibrateShuttleTask=100;
            iShuttleVibraCount++;
            break;
        case 100:
            if(VibrationShuttleDelay.Off())                                     //JerryYang 20171205 (Steven) shuttle震動馬達功能
            {
                SW[iShuttleVibration].Off();
                iVibrateShuttleTask=200;
                if(iShuttleVibraCount<IniConfig.iF23ShuttleVibrationCount)
                {
                    VibrationShuttleDelay.SetMSAndOn(1000);                     //Steven 20240427 : 移到裡面
                    str=IntToStr(iShuttleVibraCount);                           //JerryYang 20191111 shuttle振動馬達可以多震幾次
                    NewRecordProcess("MES2155", "Shuttle count", str);          //ChungHung 20140722 add add record
                    iVibrateShuttleTask=150;
                }
            }
            break;
        case 150:
            if(VibrationShuttleDelay.Off())                                     //JerryYang 20171205 (Steven) shuttle震動馬達功能
            {
                iVibrateShuttleTask=1;
            }
            break;
        case 200:
            if(VibrationShuttleDelay.Off())                                     //JerryYang 20171212 (Steven) 關閉震動後先delay 1秒,避免shuttle馬上移動IC會飛走
            {
                iShuttleLog=1;                                                  //JerryYang 20181121 (Steven) : add log,記錄in shuttle device是被震動馬達or搖搖功能導正
                iShuttleVibraCount=0;
                return true;
            }
            break;
    }
    return false;
}

//==============================================================================
//  CheckTrayMapData(int iTrayRow, int iTrayCol)           golden ainarm2.cpp:3612-3629
//  Translation wave: PT-W7e   Translator: AI(k4-ainarm2) 20260811
//
//  ROLE       : after a Loader pick, cross-checks the picked tray cell against the
//               downloaded tray map: if the cell`s map datum does not decode to "1"
//               (and, when double-check is enabled, the second map disagrees too) it
//               arms bTrayMapDataCheckErrorSkip so the cell is skipped.
//  WAVE SCOPE : CheckTrayMapData(int,int)  golden :3612-3629  -- GATED (whole body,
//               golden :3614-3628).  Function shell ACTIVE and non-static.
//               NOTE: the sibling part that owns golden :3585 (EnableTraymapCheckFunction)
//               is landing in this same wave, which kills ONE of this gate`s two
//               premises.  The gate stays anyway -- read the TRAP 3 paragraph, not
//               just the absence commands.
//  RETIRES    : the offline stub `void CheckTrayMapData(int,int) {}` at
//               ainarm9045_2x4_16_shims.cpp:38.  Behaviourally IDENTICAL (the gate
//               leaves an empty body), so this is a pure homecoming with a real GATE
//               REGISTER attached rather than a silent no-op.  The main loop must
//               delete exactly one of the two.
//
//  GATE REGISTER
//  ------------------------------------------------------------------------------
//  [k4e-G03]  golden :3614-3628   the entire body (three absences, one gate -- the
//             same shape already audited at ainarm9045.cpp:5110 (W7D-K2-G05),
//             ainarm9045.cpp:5551 (W7D-G01b), aTester_Front.cpp:9863 and
//             aTester_Rear.cpp:9793.  Where those gates` premise carries over it is
//             INHERITED here, not re-derived.)
//    SYMBOL (a): bool EnableTraymapCheckFunction(int)  -- golden ainarm2.h:216, body
//               golden ainarm2.cpp:3585-3610, OUTSIDE this agent`s ranges.  It is the
//               whole condition, so nothing is left to test if it is gated.
//      ABSENCE COMMAND (cwd D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//               rg -n --glob "*.cpp" --glob "*.h" --glob "!build/**"
//                  -e "^[A-Za-z_].*\bEnableTraymapCheckFunction\s*\("
//                  -e "extern\s+bool\s+EnableTraymapCheckFunction" .
//               RESULT: 0 lines.  RUN AT: 2026-08-11 06:31:32 +0800.
//      *** TRAP 2: THIS PREMISE EXPIRED INSIDE THE WAVE -- IT IS NOW FALSE. ***
//               Re-checked 2026-08-11 06:45 +0800: sibling part
//               _w7e_parts/03585_EnableTraymapCheckFunction.txt (written 06:43) carries
//               the FULL golden :3585-3610 body, ACTIVE with no gates of its own.  So
//               by integration this symbol WILL have a port body, and the 06:31:32
//               measurement above is a dead snapshot -- recorded here only so the
//               main loop can see which claim died and when, NOT as a live
//               justification.  That sibling part`s own banner lists five other
//               tree gates it expires (ainarm9045.cpp:5110/:5546/:10423,
//               aTester_Rear.cpp:9793, aTester_Front.cpp:9863) -- this one is a
//               sixth, and it belongs on main-loop task #16 (expired gates).
//               A declaration is still missing tree-wide (the port has no ainarm2.h),
//               so the integrator must publish
//               `extern bool EnableTraymapCheckFunction(int iCheck=0);` before ANY
//               of these call sites can be ungated.
//    SYMBOL (b): fTrayMapping->iTrayMappingDateCheck[][]  -- golden
//               cTrayMapping.h:659 `int iTrayMappingDateCheck[30][70]`.  The port`s
//               TfTrayMapping (acatchtray_shims.h:219-313) carries ONLY
//               iTrayMappingDate[35][70] at :257.  This is a COMPILE blocker, and it
//               is INDEPENDENT of (a).
//      ABSENCE COMMAND (same cwd):
//               rg -n --glob "*.cpp" --glob "*.h" --glob "!build/**"
//                  "int\s+iTrayMappingDateCheck\s*\[" .
//               RESULT: 0 lines.  RUN AT: 2026-08-11 06:31:47 +0800.
//               The four hits of the bare name are three GATE comments plus one line
//               already inside ainarm9045.cpp`s "#if 0" (:5491, :5502, :5551, :5561)
//               -- enumerated by hand at 06:31:49 +0800.
//    SYMBOL (c): fTrayMapping itself (acatchtray_shims.h:313) and MyDeCodeASCII(int)
//               (EJ1N/TextProcess.h:106) DO exist but are not in this TU`s include
//               set.  Because (a) is the condition, this part deliberately does NOT
//               take those two includes -- one fewer structural change.  If (a) and
//               (b) ever land, BOTH headers must be added in the same commit.
//    WHY THE OFFLINE DEFAULT (empty body) IS FAITHFUL: EnableTraymapCheckFunction(0)
//               is a pure config predicate -- golden :3591-3594 needs
//               bRTNoRunTrayID==false AND USE_TRAY_MAPPING!=etmUninstall AND
//               (bEnableTrayMap || (bEnableTrayID && bEnableTrayIDDownFTP)) AND
//               bEnableSuckMapCheck.  With tray mapping uninstalled offline it is
//               false, so golden`s own body writes nothing either; and this function
//               has no statement outside that if.  Same state, not merely an
//               equivalent-looking value.
//    REAL-MACHINE DIFFERENCE: with tray-map checking ON, a tray cell whose map datum
//               does not decode to "1" no longer raises bTrayMapDataCheckErrorSkip,
//               so the skip path never arms and the handler tests material the tray
//               map says is not there.
//    TRAP 3 -- (a) IS DEAD, THE GATE STILL STAYS, FOR A DIFFERENT REASON.  A dead
//               premise is not automatically a retired gate, so the question is
//               re-asked from scratch: why SHOULD this be gated now?  Because (b) is
//               a COMPILE blocker and it is completely independent of (a) -- there is
//               no iTrayMappingDateCheck member to dereference, at any value of any
//               config flag.  That alone is decisive, and it is the ONLY reason this
//               gate is still here.
//    WHY NOT A NARROWER GATE (the honest trade-off, so the integrator can overrule me
//               with a compiler in hand -- I cannot -fsyntax-only a fragment):
//               with (a) landing, a partial gate becomes structurally possible --
//               keep the outer if, the first MyDeCodeASCII/iTrayMappingDate test and
//               the `else bTrayMapDataCheckErrorSkip=true;` arm LIVE, and gate only
//               the one inner line at golden :3620-3621, leaving
//               `if(bEnableTrayMapDoubleCheck){ }` empty (offline default = "the
//               second map agrees", so the skip is never armed under double-check).
//               That needs TWO new includes in this TU, and they are NOT equally
//               safe: acatchtray_shims.h (fTrayMapping :313) is a PROVEN combination
//               here -- ainarm9045.cpp and ~25 sibling ainarm9045_*.cpp TUs already
//               include it next to cmydef.h / cprod.h / aHotPlateSubstrate.h --
//               whereas EJ1N/TextProcess.h (MyDeCodeASCII :106) is included by only
//               THREE unrelated .cpp files in the whole tree (bthermo.cpp,
//               OmronLaser/LaserSensor.cpp, EJ1N/TextProcess.cpp) and its own header
//               comment flags a windows.h/rpcndr.h `byte` collision hazard.  Taking
//               an unproven header into a fragment I cannot compile is how a stitch
//               breaks, so I kept the whole-body gate.  Narrow it at integration if
//               the build stays green; the behaviour delta of doing so is exactly the
//               REAL-MACHINE DIFFERENCE paragraph above, minus the double-check arm.
//    ALSO STILL REQUIRED EITHER WAY: a tree-wide declaration for
//               EnableTraymapCheckFunction (see the TRAP 2 note under (a)).
//  ------------------------------------------------------------------------------
//  NOTE FOR WHOEVER UN-GATES: golden compares MyDeCodeASCII(...)!=1 while
//               MyDeCodeASCII returns AnsiString (golden EJ1N/TextProcess.h:9), so
//               golden is really comparing a STRING against AnsiString(1) == "1".
//               The port reproduces that exactly -- vclcompat/AnsiString.h:67 has a
//               non-explicit AnsiString(int) and :215 an
//               operator!=(const AnsiString&,const AnsiString&) -- so it will compile
//               and behave identically.  Do NOT "fix" it into an int compare.
//  GOLDEN INCONSISTENCY PRESERVED: this function indexes
//               iTrayMappingDate[iTrayCol][iTrayRow] (column first), whereas golden`s
//               own inline twin at golden ainarm9045.cpp:2639 indexes
//               [iTrayRow][iTrayCol].  One of the two must be transposed; golden`s
//               order for THIS function is kept verbatim.
//==============================================================================
void CheckTrayMapData(int iTrayRow, int iTrayCol)
{
#if 0 // GATE k4e-G03 -- golden :3614-3628, whole body.  The LOAD-BEARING reason is
      // iTrayMappingDateCheck: no such member exists on the port TfTrayMapping, so the
      // inner double-check line cannot compile at any config value.  The other premise
      // (EnableTraymapCheckFunction had no port body) DIED INSIDE THIS WAVE -- see the
      // TRAP 2 / TRAP 3 paragraphs in the GATE REGISTER above, which also record what a
      // narrower gate would need and why I did not take it blind.
    if(EnableTraymapCheckFunction(0))                                           //wei 20170302 (Steven) Tray map data error
    {
        if(MyDeCodeASCII(fTrayMapping->iTrayMappingDate[iTrayCol][iTrayRow])!=1)
        {
            if(TestIF_File.bEnableTrayMapDoubleCheck)                           //wei 20190506
            {
                if(MyDeCodeASCII(fTrayMapping->iTrayMappingDateCheck[iTrayCol][iTrayRow])!=1)
                    bTrayMapDataCheckErrorSkip=true;
            }
            else
            {
                bTrayMapDataCheckErrorSkip=true;
            }
        }
    }
#endif // GATE k4e-G03
}

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 03631 : RecordShtSuperflous
//  Golden: ainarm2.cpp:3631-3658 (cp950, CRLF).
//
//  ROLE          : writes the MES record that says an in-shuttle device was
//                  un-superfluous'd, choosing the code by WHICH corrective motion did it
//                  (iShuttleLog==1 vibration -> MES0401/MES0402, ==2 shaking ->
//                  MES0404/MES0405, per shuttle), then clears iShuttleLog.  Korea-function
//                  customers are opted out with an early return.
//  WAVE SCOPE    : RecordShtSuperflous(int)  golden ainarm2.cpp:3631-3658  ACTIVE
//  GATE REGISTER : (none)  IniConfig.bKoreaFunction Config.h:131 ; iShuttleLog cmydef.h:4910 ;
//                  NewRecordProcess acatchtray_shims.h:388.
//  HAND-OFF      : NewRecordProcess resolves to acatchtray_shims.h:388, whose body
//                  (acatchtray_shims.cpp:135) is `{}` -- so the record still sinks offline.
//                  cMyDB.h:121 has golden's real signature but MUST NOT be included in this
//                  TU: with canary_support.h already in (needed for RecordProcess/__FUNC__)
//                  cMyDB.h re-specifies the default argument of RecordProcess and
//                  re-declares MyDBIProcessNew with __fastcall.  MEASURED: a 3-line TU
//                  containing only `#include "canary_support.h"` + `#include "cMyDB.h"`
//                  + `int dummy;` compiled with the ht9045_sm flag set (g++ -std=c++1z
//                  -fsyntax-only -DCSYSTEM_TICK_ORACLE -DDLLDIR_EX -DMN200DLL_EXPORTS
//                  -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -I. -IMotor/vendor
//                  -IEtherCAT/vendor -ISECSGEM -Ithird_party/sqlite3) yields exactly
//                  2 errors -- ran 2026-08-11 06:41:27 local.  Pre-existing tree
//                  condition, not introduced here.
//  GOLDEN NOTE   : the shuttle-1 strings say "Un-superfluous" and the shuttle-2 strings say
//                  "Unsuperfluous" (no hyphen).  Transcribed verbatim -- do not normalise.
//  INTEGRATOR    : RETIRE `void RecordShtSuperflous(int) {}` at
//                  ainarm9045_w7_shims.cpp:44.  Behaviour becomes "clears iShuttleLog and
//                  attempts the log" instead of "does nothing" -- note the iShuttleLog=0
//                  reset at golden :3657 IS a real state change the stub was dropping,
//                  which is why ainarm9045.cpp:1862 could re-log the same event.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0 (Chinese comments at golden :3631/:3633
//                  transcribed character-for-character)
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "acatchtray_shims.h"
//==============================================================================
void RecordShtSuperflous(int iShuttle)                                          //JerryYang 20181121 (Steven) : add log,記錄in shuttle device是被震動馬達or搖搖功能導正
{
    if(IniConfig.bKoreaFunction)                                                //Steven 20230825 : 韓國客戶說不想看到這個log
        return;
    if(iShuttle==0)
    {
        if(iShuttleLog==1)
        {
            NewRecordProcess("MES0401", "In Shuttle 1 Device Un-superfluous By Vibration!");
        }
        else if(iShuttleLog==2)
        {
            NewRecordProcess("MES0404", "In Shuttle 1 Device Un-superfluous By Shaking!");
        }
    }
    else if(iShuttle==1)
    {
        if(iShuttleLog==1)
        {
            NewRecordProcess("MES0402", "In Shuttle 2 Device Unsuperfluous By Vibration!");
        }
        else if(iShuttleLog==2)
        {
            NewRecordProcess("MES0405", "In Shuttle 2 Device Unsuperfluous By Shaking!");
        }
    }
    iShuttleLog=0;
}

//==============================================================================
//  bNeedPickupErrorICToRecycleBin()  --  golden ainarm2.cpp:3871-3895
//
//  ROLE          Latch-and-run the "pickup error IC -> recycle bin" flow: if the
//                loader tray now holds nothing but HAS_SKIP_IC and NULL_IC, latch
//                bNeedRunRecycle and kick DoSuckToRecycleFromLoader(first=true);
//                on later polls drive it with first=false until it reports done,
//                then clear the latch.  Returns the latch, i.e. "recycle busy".
//  WAVE SCOPE    bNeedPickupErrorICToRecycleBin()  ACTIVE (two inner gates)
//                                             golden :3871-3895
//
//  GATE REGISTER
//    #if 0 (x2) -- golden lines :3881-3884 and :3891.  Both are the SAME symbol,
//      gated per call site.  Gated as blocks that keep every brace balanced, so
//      the enclosing `if(...){ }` bodies stay well-formed and merely become empty.
//      SYMBOL: cInArmPlacement::DoSuckToRecycleFromLoader(bool,AnsiString&) --
//        golden cInArmPlacement.h:86.
//      ** TRAP 5 -- TWO CLASSES, AND THIS IS THE WHOLE REASON FOR THE GATE **
//        `fMain->cInplace` is declared `TfMainInplace *cInplace;` at
//        forms/fMain.h:199, and TfMainInplace (forms/fMain.h:93) is the offline
//        FACADE class -- it carries only iNo9ShtErrICCt / bNo9ShtErrNo /
//        bNo9Action / InArmPlacementEnable().  The REAL class that owns
//        DoSuckToRecycleFromLoader is a DIFFERENT type, `cInArmPlacement`
//        (cInArmPlacement.h:41, body cInArmPlacement.cpp).  So the method is not
//        merely unlinked -- it is not a member of the type this expression has.
//        Reaching it needs either a facade method or a retype of cInplace, both
//        cross-file edits outside this agent's write boundary.  I relied on
//        forms/fMain.h for the type of fMain->cInplace (forms/fMain.h:716
//        `extern TfMain *fMain;` is the only declaration of the object in the
//        tree) and on forms/fMain.cpp:39 for its behaviour.
//      PRECEDENT: ainarm9045.cpp:1443-1451 gates the sibling
//        DoSuckToRecycleFromShuttle for exactly this reason.  That premise carries
//        over; it is not re-derived here.
//      WHY THE OFFLINE DEFAULT IS FAITHFUL: forms/fMain.cpp:39 is
//        `bool TfMainInplace::InArmPlacementEnable() { return false; }`, so golden
//        :3873-3876 returns false BEFORE either gated line is reached.  The gated
//        region is unreachable offline, and this function's offline return is
//        identical to the stub it replaces (ainarm9045_2x4_16_shims.cpp:73
//        `{ return false; }`).
//      REAL-MACHINE BEHAVIOUR DIFFERENCE (stated because "equivalent offline" is
//        NOT the same as "fine"): on a machine with placement enabled, the recycle
//        SM is never driven, so no skipped IC is ever moved to the bin, AND the
//        function-local `static bool bNeedRunRecycle` LATCHES TRUE FOREVER on the
//        first tray that trips HasOnlyDataICAndNullIC -- because the only line that
//        clears it (:3883) lives inside the first gate.  Every caller would then
//        see "recycle busy" permanently.  This gate MUST be closed before any
//        placement-enabled configuration is supported.
//      COMMAND PROVING ABSENCE (from D:/HT9045/HT9011UC_Cpp_V3.33.906.0):
//        grep -n "DoSuckToRecycleFromLoader\|class TfMainInplace" forms/fMain.h forms/fMain.cpp FormsFacade.h
//        -> ONLY `forms/fMain.h:93:class TfMainInplace` ; ZERO
//           DoSuckToRecycleFromLoader hits in any of the three files.
//        (Cross-check that the real one exists elsewhere, i.e. that this is a
//         wrong-type problem and not a missing-symbol problem:
//         grep -n "DoSuckToRecycleFromLoader" cInArmPlacement.h cInArmPlacement.cpp
//         -> cInArmPlacement.h:86 + its body.)
//      RUN AT: 20260811 06:39:36 local (Asia/Taipei).  TRAP 2: re-run at
//        integration -- a sibling may add the facade method mid-wave, in which case
//        DELETE both gates rather than keeping them.
//
//  RETIRES A STUB: ainarm9045_2x4_16_shims.cpp:73 `bool
//    bNeedPickupErrorICToRecycleBin() { return false; }` (decl
//    ainarm9045_2x4_16_shims.h:83) must be REMOVED by the integrator, or this is a
//    `multiple definition`.  NOT written `static` (TRAP 1 shape (d)).  Offline
//    behaviour after retirement is UNCHANGED (false), for the reason above.
//  KEPT DELIBERATELY: `AnsiString asErrorPickup=AnsiString("");` (golden :3878) is
//    transcribed even though both of its uses are now gated -- it is golden text,
//    and the gate is what removes the uses.  Expect an unused-local warning; it is
//    not an error, and dropping the line would be a silent edit.
//==============================================================================
bool bNeedPickupErrorICToRecycleBin()                                           //JimmyChiu 20220908 add Pickup Error Placement
{
    if(fMain->cInplace->InArmPlacementEnable()==false)
    {
        return false;
    }
    static bool bNeedRunRecycle=false;
    AnsiString asErrorPickup=AnsiString("");
    if(bNeedRunRecycle)
    {
#if 0 // GATE (W7e-G2a): golden :3881-3884 -- fMain->cInplace is TfMainInplace (forms/fMain.h:93/:199), which has NO DoSuckToRecycleFromLoader; the real owner is cInArmPlacement (cInArmPlacement.h:86). Unreachable offline (InArmPlacementEnable()==false). See GATE REGISTER above.
        if(fMain->cInplace->DoSuckToRecycleFromLoader(false,asErrorPickup))
        {
            bNeedRunRecycle=false;
        }
#endif // GATE (W7e-G2a)
    }
    else
    {
        bNeedRunRecycle=MOT[MMTrayY].HasOnlyDataICAndNullIC(HAS_SKIP_IC);
        if(bNeedRunRecycle)
        {
#if 0 // GATE (W7e-G2b): golden :3891 -- same symbol/type mismatch as (W7e-G2a). See GATE REGISTER above.
            fMain->cInplace->DoSuckToRecycleFromLoader(true,asErrorPickup);
#endif // GATE (W7e-G2b)
        }
    }
    return bNeedRunRecycle;
}

// =============================================================================
//  SetInArmNeedDestory(bool bPlace, int iShtRow, int iShtCol, int iRow, int iCol)
//  golden ainarm2.cpp:3897-3908
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- per-nozzle bookkeeping around a shuttle transfer.  bPlace==true (in-arm is
//    PLACING): every nozzle that is not NULL_IC gets SetNeedDestroy(true) so the blow-off
//    runs for it.  bPlace==false (in-arm is about to PICK): copy the shuttle cell's IC
//    type into InArmSuck.iNeedSuck[iRow][iCol] -- 'this nozzle must suck that type'.
//    Golden's spelling 'Destory' is kept: it is the linked symbol name.
//
//  WAVE SCOPE -- SetInArmNeedDestory (golden :3897-3908) ACTIVE, 1 gate.
//
//  GATE REGISTER
//   [W7E-3] golden ainarm2.cpp:3902  InArmSuck.Suck[iRow][iCol].SetNeedDestroy(true);
//     WHY GATED -- the TMySucker that this tree's InArmSuck is built from
//       (aHotPlateSubstrate.h:106, the 'MINIMAL mirror') exposes the FIELD
//       `bool bNeedDestroy;` (:149, public) and the getter GetNeedDestroyStatus() (:150)
//       but NOT the setter SetNeedDestroy().  The setter exists only on the OTHER,
//       differently-laid-out TMySucker in mykitsuck.h:251 -- see TRAP 5 below.  So the
//       golden call as written does not compile against the class actually in use.
//     OFFLINE DEFAULT + WHY FAITHFUL -- golden's setter body is one line:
//       `void SetNeedDestroy(bool Value) {bNeedDestroy=Value;}` (mykitsuck.h:251), and
//       the field it writes is public on this tree's class, so the #else arm does exactly
//       that assignment.  This is an EXACT inline of the golden setter -- same field,
//       same value, same sequence point -- not a degraded stand-in.  Same 'faithful
//       free equivalent, not an approximated no-op' pattern aTester_Rear.cpp already
//       used for TMyKitSuck::SetUnuseToNullIC.
//     REAL-MACHINE DIFFERENCE -- none in behaviour.  The only difference is which line
//       the compiler sees; the nozzle still ends up flagged for the destroy/blow-off.
//     ABSENCE COMMAND (re-run at integration; TRAP 2) --
//       grep -n "SetNeedDestroy" D:/HT9045/HT9011UC_Cpp_V3.33.906.0/aHotPlateSubstrate.h
//       expect: NO hit (and grep the same token in mykitsuck.h -> :251, the wrong class).
//       MEASURED 2026-08-11 15:38.  If a later wave adds the setter to
//       aHotPlateSubstrate.h's TMySucker, flip this gate back to golden's line.
//
//  TRAP 5 -- two of the objects here belong to classes that exist TWICE in this tree.
//    InArmSuck / ptrInSHT: I relied on aHotPlateSubstrate.h (TMyKitSuck at :365 -- the
//    layout 177 TUs use; iNeedSuck at :555, inside the HT9045_KITSUCK_AUTOCLEAN_ADDED
//    guard; ptrInSHT declared :1032), NOT mykitsuck.h (TMyKitSuck :274, iNeedSuck :312,
//    ptrInSHT :474), whose layout differs.  The objects themselves are defined in
//    aHotPlateSubstrate.cpp (ptrInSHT at :1097) and the port ainarm2.cpp already
//    includes aHotPlateSubstrate.h at its :37, so this file cannot silently bind the
//    other layout.  TMySucker::SetNeedDestroy likewise comes from aHotPlateSubstrate.h's
//    TMySucker, not mykitsuck.h:251.
//  TRAP 4 -- ptrInSHT is a POINTER global that only AutoClean setup code makes
//    non-NULL.  Golden dereferences it unguarded here and so do I: adding an
//    if(ptrInSHT) would CHANGE behaviour, and this part adds no file-scope object with a
//    running constructor.  HAND-OFF: the bPlace==false arm will fault offline until
//    whichever wave lands ptrInSHT's assignment (today aHotPlateSubstrate.cpp:1097 only
//    defines it).
//  RETIRE-AT-INTEGRATION: 5-arg no-op stub at aHotPlateSubstrate.cpp:1546 (decl
//    aHotPlateSubstrate.h:1064) with 10+ live call sites (ainarm9045_1x1_1.cpp:191-203,
//    ainarm9045S_1x4_4.cpp:208-233, ainarm9045S_2x4_4_13.cpp:277-302).  TRAP-1 shape (b).
//  Big5 golden -> UTF-8 here; Chinese comments transcribed character-for-character.
// =============================================================================
void SetInArmNeedDestory(bool bPlace, int iShtRow, int iShtCol, int iRow, int iCol)
{
    if(bPlace==true)
    {
        if(InArmSuck.Item[iRow][iCol]!=NULL_IC)
#if 1 // AI(W906-W2-A46) 20260926: gate FLIPPED to the golden arm -- A4-6 (8ff6c754) merged TMyKitSuck/TMySucker into the one golden layout (mykitsuck.h), premise dead: TMySucker::SetNeedDestroy is mykitsuck.h:251; the #else inline was already equivalent -- golden :3902
            InArmSuck.Suck[iRow][iCol].SetNeedDestroy(true);
#else
            InArmSuck.Suck[iRow][iCol].bNeedDestroy=true;                       // GATE W7E-3: exact inline of golden mykitsuck.h:251 `SetNeedDestroy(bool Value) {bNeedDestroy=Value;}`
#endif
    }
    else
    {
        InArmSuck.iNeedSuck[iRow][iCol]=ptrInSHT->Item[iShtRow][iShtCol];
    }
}
//==============================================================================

//==============================================================================
//  AI(k6-ainarm2) 20260811 -- PT-W7e part 03910 : SetShuttlefCanMoveL
//  Golden: ainarm2.cpp:3910-3944 (cp950, CRLF).
//
//  ROLE          : the single writer of MOT[MInShuttle1+iShuttle].fCanMoveL (the
//                  "shuttle may move left" interlock).  It formats a
//                  "Sht%d CanMove = %s, %s-%s" line from the caller's function/task tags and
//                  de-duplicates it against a function-static sOldStr, returning early when
//                  the flag did not change AND the message repeats.  Under
//                  DEBUG_ShuttleCanMoveL it also appends the line to a per-day/per-hour
//                  log file.
//  WAVE SCOPE    : SetShuttlefCanMoveL(int,bool,AnsiString,AnsiString)
//                  golden ainarm2.cpp:3910-3944  ACTIVE
//  GATE REGISTER : (none)  MOT[] Motor/mymotor.h:385 with fCanMoveL :144 ; MInShuttle1
//                  cmydef.h:2112 (=11, cmydef.cpp:2345; MInShuttle2=12, so golden's
//                  MInShuttle1+iShuttle indexing is exact) ; AnsiString::sprintf handles a
//                  raw AnsiString for %s by design (vclcompat/AnsiString.h:139-142) ;
//                  SystemYear.. cmydef.h:226-227 ; MyForceDirectories common.h:341 and
//                  WriteDataToFile common.h:338 (both real: common.cpp:1888/:1822).
//                  DEBUG_ShuttleCanMoveL is commented out in BOTH trees (golden
//                  MachineType.h:35, port MachineType.h:40), so the #ifdef block is dead in
//                  both -- faithful, and kept verbatim rather than deleted.
//  GOLDEN BUGS (kept verbatim):
//                  (1) :3918 and :3923 build the SAME Str twice, once per branch.
//                  (2) :3940 `Path.sprintf("%s\\%04d_%02d_%02d_%02d.txt", asLogPath,
//                      SystemYear, SystemMonth, SystemDate, SystemYear, SystemMonth,
//                      SystemDate, SystemHour)` has FIVE conversions and EIGHT arguments, so
//                      the final `_%02d` is filled with SystemYear (again) and SystemHour is
//                      dropped -- the intended per-hour file name never happens.  Inside the
//                      dead #ifdef, so no live effect.
//  INTEGRATOR    : RETIRE the simplified body at aHotPlateSubstrate.cpp:1567.  It writes the
//                  same MOT[] flag (`(iShuttle==0)?MInShuttle1:MInShuttle2`, numerically
//                  identical) but drops the sFun/sTask log line and the sOldStr dedup, and it
//                  names its 3rd/4th parameters /*sFun*/ /*sTask*/.  Retiring it is a pure
//                  gain: no MOT[] behaviour change, plus the log.  Keep the
//                  aHotPlateSubstrate.h:1070 declaration -- the `sTask=""` default lives
//                  there and golden's definition does not repeat it.
//  VERIFY        : port ainarm2.cpp + the integrator-owned file-scope globals + all 10
//                  k6-ainarm2 parts pass  g++ -std=c++1z -fsyntax-only -Wall -Wextra
//                  CLEAN (exit 0) in a SCRATCH stitched copy, measured 20260811
//                  06:45:05 local.  A fragment cannot be syntax-checked on its own --
//                  the REAL stitched ainarm2.cpp is the main loop's check, not this.
//  SELF-CHECK    : brace delta 0 ; U+FFFD 0 (Chinese comment at golden :3923 transcribed
//                  character-for-character)
//==============================================================================
// AI(pt-wave) 20260811: #include hoisted to the file head by the integrator (a part file
//   may not touch the head). Original: #include "common.h"
//==============================================================================
void SetShuttlefCanMoveL(int iShuttle, bool bCanMoveL, AnsiString sFun, AnsiString sTask)
{
    AnsiString Str="", sflag="";
    static AnsiString sOldStr="";
    sflag=(bCanMoveL)?"true":"false";
    if(MOT[MInShuttle1+iShuttle].fCanMoveL!=bCanMoveL)
    {
        MOT[MInShuttle1+iShuttle].fCanMoveL=bCanMoveL;
        Str.sprintf("Sht%d CanMove = %s, %s-%s", iShuttle+1, sflag, sFun, sTask);
        sOldStr=Str;
    }
    else
    {
        Str.sprintf("Sht%d CanMove = %s, %s-%s", iShuttle+1, sflag, sFun, sTask);                                       //Sam 20250327 : 新增Log
        if(sOldStr!=Str)
        {
            sOldStr=Str;
        }
        else
        {
            return;
        }
    }

    #ifdef DEBUG_ShuttleCanMoveL
    AnsiString asLogPath, Log="", Path="";
    asLogPath.sprintf("D:\\HT9045_Log\\ShuttleCanMoveL\\%04d_%02d_%02d", SystemYear, SystemMonth, SystemDate);
    MyForceDirectories(asLogPath, "SetShuttlefCanMoveL");

    Log.sprintf("%04d-%02d-%02d, %02d:%02d:%02d:%03d, %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Str);
    Path.sprintf("%s\\%04d_%02d_%02d_%02d.txt", asLogPath, SystemYear, SystemMonth, SystemDate, SystemYear, SystemMonth, SystemDate , SystemHour);

    WriteDataToFile(Path, Log);
    #endif
}

// =============================================================================
//  InitInOCRWaitTask()   golden ainarm2.cpp:3950-3953   (wei 20170901)
//  Translation wave: PT-W7e   Translator: AI(k7-ainarm2) 20260811
//
//  ROLE -- resets the OCR 'in-arm to wait position' task cursor to step 1.
//
//  WAVE SCOPE -- InitInOCRWaitTask (golden :3950-3953) ACTIVE, no gates.
//  GATE REGISTER -- (empty.)
//
//  RETIRE-AT-INTEGRATION: `void InitInOCRWaitTask() {}` at aHotPlateSubstrate.cpp:1575
//  (decl aHotPlateSubstrate.h:1074) -- a TRUE no-op, so today the cursor is never
//  re-armed even though live call sites exist (ainarm9045_1x1_1.cpp:822,
//  ainarm9045S_1x4_4.cpp:911/1274, ainarm9045S_2x4_4_13.cpp:1210/1573).  TRAP-1 shape (b).
//  GLOBAL I ONLY USE, NEVER DEFINE: iOCRWaitTask (golden :3948, integrator's list).
// =============================================================================
void InitInOCRWaitTask()                                                        //wei 20170901
{
    iOCRWaitTask=1;
}
//==============================================================================

//==============================================================================
// ---- k3-ainarm2 / golden ainarm2.cpp:3955-3984 -----------------------------
//  ROLE          -- the OCR variant of "park the in-arm": a 3-step SM that first
//                   raises Z to plate-safe (with the magic caller tag 123), then
//                   moves XY to the wait position, then holds at case 200 until
//                   the tray-arm no longer needs to be waited for, at which point
//                   it reports true and rewinds the cursor to 1.
//  WAVE SCOPE    -- OCRMoveInArm2XYToWait()               ACTIVE   golden :3955-3984
//  GATE REGISTER -- (empty: this range contains no #if 0)
//
//  FIDELITY NOTES
//   * `int &Task=iOCRWaitTask;` references the file-scope cursor at golden :3948,
//     which is on the main loop's 37-global list -- NOT emitted here (nor is
//     InitInOCRWaitTask() at golden :3950, outside my range).
//   * The literal 123 passed to MoveInArmZToPlateSafe is a caller tag, not a task
//     number -- kept exactly.
//   * case 200 rewinds Task to 1 ONLY on success, so a blocked tray-arm re-enters
//     at 200 rather than restarting the Z/XY moves.  Kept.
//  INTEGRATE-STEP NOTE: aHotPlateSubstrate.cpp:1576 currently holds the offline
//  stub `bool OCRMoveInArm2XYToWait() { return true; }` (declared
//  aHotPlateSubstrate.h:1075) -- REMOVE it as part of the stitch.  The stub
//  answered TRUE immediately; the real SM needs at least 3 ticks, so any caller
//  that assumed one-shot completion will now iterate.  Measure.
// ---------------------------------------------------------------------------
// DECLARATION HOIST: MoveInArm2XYToWait (golden :3969) -- my own sibling part 01310
// defines it; no header this TU includes declares it (the port has no ainarm2.h; the
// only port decls are acatchtray_shims.h:361 / atester_shims.h:282, both of which
// also carry the no-op stub decl this stitch retires).  Guard macro shared with the
// identical block in part 01620, so the two are idempotent in any stitch order.
// It is a DECLARATION, so it cannot become TRAP 1 shape (d).
#ifndef HT9045_MoveInArm2XYToWait_DECLARED
#define HT9045_MoveInArm2XYToWait_DECLARED
bool MoveInArm2XYToWait();                                                      // golden ainarm2.h:146 (body golden ainarm2.cpp:1310)
#endif
// ---------------------------------------------------------------------------
bool OCRMoveInArm2XYToWait()                                                    //wei 20170901
{
    int &Task=iOCRWaitTask;
    bool Result=false;

    switch(Task)
    {
        case 1:
            if(MoveInArmZToPlateSafe(123))
            {
                Task=100;
            }
            break;
        case 100:
            if(MoveInArm2XYToWait())
            {
                Task=200;
            }
            break;
        case 200:
            if(NeedWaitTrayArm==false)
            {
                Result=true;
                Task=1;
            }
            break;
    }

    return Result;
}
