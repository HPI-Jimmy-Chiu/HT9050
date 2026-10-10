// =============================================================================
//  ckernel.cpp  --  HT9045 KERNEL free-function translation unit
//
//  Mirrored from : HT9011UC_Code_V3.33.906.0_20260618/ckernel.cpp
//                  (READ-ONLY golden, 2588 lines, BCB6 / cp950)
//  Translation wave : W7-L2
//  Translator : AI(W906-W7-L2) 20260803
//
//  WHAT THIS FILE IS
//  -----------------
//  Not a class -- a flat free-function TU holding the operator/kernel layer that
//  sits between the panel hardware and the engines: the three manual-key polls
//  (WaitManualStepKey / WaitManualStartKey / WaitManualRetryKey), the
//  START/PAUSE edge handler (ScanSystemSensor), the panel-key dispatcher
//  (ScanPannelKey), the per-tick display scheduler (DoSystemMessage), the
//  motor/cylinder alarm drain (GetMotorAlarmCode / ProcessAlarm), and the small
//  predicates around them.  Seventeen top-level functions, in golden order.
//
//  DEFERRED IN THIS WAVE (charter, not discovery): ShowRunLed (golden :704-932)
//  and ShowRunLabel (golden :935-1726) -- 1,021 lines of near-pure VCL form
//  painting -- land as explicitly-labelled deferred stubs carrying the
//  W906_ShowRunLed_Count / W906_ShowRunLabel_Count call counters declared in
//  ckernel.h, so DoSystemMessage's scheduling stays falsifiable.  They are
//  scheduled for W7-U.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as real UTF-8.
//  Final gate: ZERO U+FFFD.
//
// -----------------------------------------------------------------------------
//  GOLDEN INCLUDE BLOCK (golden ckernel.cpp:1-49) -- PER-LINE DISPOSITION
// -----------------------------------------------------------------------------
//  Golden has 39 `#include` lines (counted this pass: golden :1, :3, :5-:8,
//  :10-:13, :15-:17, :19-:23, :25-:41, :44-:46, :49) plus two BCB6 pragmas
//  (:2, :51).  Each was decided individually by asking what ckernel.cpp
//  actually USES from it -- measured, per golden line, over the cp950-decoded
//  source, with every use attributed to its owning function so that "used only
//  by a DEFERRED function" could be told apart from "not used at all".
//  Tally: 26 KEPT -- 12 as-is or with only a ported path change, 14 RE-TARGETED
//  to this tree's home for the same symbols -- 13 DROPPED, 6 ADDED, and both
//  pragmas dropped.  The drops are listed below with their golden line and the
//  reason; nothing was dropped merely because a ported file was missing.
//
//  DROPPED -- nothing in ckernel.cpp needs them here:
//    golden :2   #pragma hdrstop            BCB6 precompiled-header boundary
//                                           marker; MinGW has no equivalent.
//                                           Same drop as acatchtray.cpp:70.
//    golden :10  MyLaneIo.h                 ZERO uses.  Scanned all 2588 lines
//                                           for LaneIO/LaneIo/`IO[` -- the only
//                                           hit is the #include itself (:10).
//    golden :25  ainarm2.h                  ZERO uses.  The only occurrence of
//                                           the string "ainarm2" in the whole
//                                           file is the #include at :25.
//    golden :26  uLotInfo.h                 fLotInfo used at :1057,:1058,:1579,
//                                           :1590,:1594,:1598,:1600,:1609,:1644,
//                                           :1646,:1650,:1656,:1660,:1662,:1667,
//                                           :1669 -- ALL 16 inside ShowRunLabel
//                                           (:935-1726), DEFERRED.  (Ported home
//                                           forms/fLotInfo.h:111 exists and is
//                                           what W7-U will add.)
//    golden :27  CCDInterface.h             CCDInterfaceForm used at :828-:829
//                                           only, inside ShowRunLed, DEFERRED.
//    golden :28  FTPClient.h                fFTPClient used at :1589,:1636 only,
//                                           inside ShowRunLabel, DEFERRED.
//    golden :22  cTowerLight.h              fTowerLight (:790) + the LED_* /
//                                           RunState state selection (:718-:891)
//                                           are entirely inside ShowRunLed,
//                                           DEFERRED.
//    golden :34  ATCInterface.h             The ATC symbols ckernel.cpp actually
//    golden :35  ATC_Handler_Side.h         touches are ATC_SYSTEM (cmydef.h:3358),
//                                           bATCInitialFinish (cmydef.h:241),
//                                           eATCSiliconType (MachineType.h:693),
//                                           Temperature.* (cprod.h:1646) and
//                                           COM2->ATCAlarmSenCheck()
//                                           (atester_shims.h:461) -- every one
//                                           reachable without these two headers.
//    golden :38  cShowBinSet.h              fShowBinSet used at :729 only, inside
//                                           ShowRunLed, DEFERRED.  CheckBinSet
//                                           (:211-:251) despite its name never
//                                           touches the form.
//    golden :39  common.h                   ZERO uses.  Scanned for the whole INI
//                                           helper family (ReadIniData /
//                                           WriteIniData / CheckIniData /
//                                           IniFile): no hit anywhere in the file.
//    golden :41  AutoTeach.h                fAutoTeach used at :1720 only, inside
//                                           ShowRunLabel, DEFERRED.
//    golden :44  MyPLC_IO_Modbus.h          Enable_PLCSafety_IO used at :921
//    golden :45  ModbusTCPClient.h          (ShowRunLed) and :1006 (ShowRunLabel)
//                                           only -- both DEFERRED.  No other
//                                           PLC/Modbus symbol appears.
//    golden :51  #pragma package(smart_init) BCB6 package-init pragma; dropped
//                                           for MinGW exactly as acarry.cpp:62
//                                           and acatchtray.cpp:71 already do.
//
//  When W7-U lands ShowRunLed / ShowRunLabel it must restore the six
//  deferred-only groups above (fLotInfo, CCD, FTP, tower light, bin-set form,
//  PLC safety) -- they are dropped because the BODIES are absent, not because
//  the symbols are unavailable.
// =============================================================================
#include "MachineDefine.h"                                                      // golden ckernel.cpp:1 -- ported include hub (vclcompat umbrella + system/STL)
//AI(W906-W7-L2) 20260803: golden :2 `#pragma hdrstop` dropped -- BCB6 precompiled-header boundary, no MinGW equivalent.
#include "ckernel.h"                                                            // golden ckernel.cpp:3 -- this TU's own interface
#include "vclcompat/vcl_compat.h"                                               //AI(W906-W7-L2) 20260803: ADDED -- AnsiString/TDateTime named explicitly (golden got them from <vcl.h>); same as csystem.cpp:84
#include "forms/fShowBinSet.h"                                                  //AI(W906-MSTATE-P2b) 20260924: fShowBinSet->fShow —— ShowRunLed golden :729（佔用原本的空行，不移動行號）
#include "MachineType.h"                                                        // golden ckernel.cpp:5  -- eATCSiliconType (:693), Tempture_Hot, eTrayCount, LED/mode enums
#include "cprod.h"                                                              // golden ckernel.cpp:6  -- Prod, TestIF (cprod.h:2576), Temperature (cprod.h:1646)
#include "mysensor.h"                                                           // golden ckernel.cpp:7  -- Sen[] (SnRKManualStep / SnFKRetry / SnAllEMG / ...)
#include "FormsFacade.h"                                                        // golden ckernel.cpp:8  -- RE-TARGET of main.h: fMain lives on the forms facade (forms/fMain.h:701)
#include "forms/fTowerLight.h"                                                  //AI(W906-MSTATE-P2b) 20260924: fTowerLight->fShow —— ShowRunLed golden :790（佔用原本的空行，不移動行號）
//AI(W906-W7-L2) 20260803: golden :10 MyLaneIo.h dropped -- zero uses (see header block).
#include "forms/fNote.h"                                                        // golden ckernel.cpp:11 -- RE-TARGET of note.h (form half): fNote (forms/fNote.h:169) -- DoPanelLamp :1749, ScanPannelKey :2007/:2038/:2184/:2215
#include "Motor/mymotor.h"                                                      // golden ckernel.cpp:12 -- MOT[] / HTMotor (GetMotorAlarmCode :2432-:2499)
#include "ckernel_shims.h"                                                      // golden ckernel.cpp:13 -- RE-TARGET of cinitial.h: SetWorkParameter (:371) + SetSuckRetryCount (:496); golden bodies live in cinitial.cpp, which this tree does not have

#include "aHotPlateSubstrate.h"                                                 // golden ckernel.cpp:15 -- RE-TARGET of MyKitSuck.h: TMySucker + InArmSuck/OutArmSuck/FTestSuck/... , plus StopAllMotor (:923), MyDBIProcess (:924) and the Zteach facade (:1134) golden takes from cMyDB.h / InOutArmZteach.h
#include "myswitch.h"                                                           // golden ckernel.cpp:16 -- SW[]
#include "csystem.h"                                                            // golden ckernel.cpp:17 -- IsEMGPressed (csystem.h:147) + the system-coordination interface

#include "canary_support.h"                                                     // golden ckernel.cpp:19 -- RE-TARGET of mymessbox.h (ShowMyMessage/ShowErrorMessage) AND of cMyDB.h's MyDBIProcessNew (:396); also LastSet, RecordProcess (:256), PopUpAlarm/ClearAllAlarm and the three note.h alarm free functions (section 5)
#include "forms/fHome.h"                                                        // golden ckernel.cpp:20 -- RE-TARGET of uhome.h: fHome (forms/fHome.h:123) -- :372-:373
#include "acatchtray_shims.h"                                                   //AI(W906-W7-L2) 20260803: ADDED -- the OTHER half of golden :20 uhome.h.
// That include was re-targeted just above to forms/fHome.h for the FORM half
// (fHome), but ScanSystemSensor also calls the FREE FUNCTION
// InitProcessSingleMotorTask (golden uhome.h:89, body golden uhome.cpp:397) at
// ckernel.cpp:375.  This tree has no uhome.cpp; its offline home is
// acatchtray_shims.h:372 (no-op body acatchtray_shims.cpp:116).  Same include
// for the same symbol at ainarm9045_1x1_1.cpp:65.
#include "forms/fSetup.h"                                                       // golden ckernel.cpp:21 -- RE-TARGET of csetup.h: fSetup (forms/fSetup.h:76) -- :411
//AI(W906-W7-L2) 20260803: golden :22 cTowerLight.h dropped -- LED_*/RunState/fTowerLight are ShowRunLed-only (DEFERRED).
#include "atester_shims.h"                                                      // golden ckernel.cpp:23 -- RE-TARGET of cContact.h: fContact (atester_shims.h:251) -- :56,:62,:64,:74,:77,:89; ALSO the RE-TARGET of golden :30 rs232.h, COM2 (atester_shims.h:461) -- :445
#include "forms/fContact.h"   // AI(W906-W156) 20261007 (St02-E) (c): fContactForm for :237 / :306; on the old blank line
//AI(W906-W7-L2) 20260803: golden :25 ainarm2.h, :26 uLotInfo.h, :27 CCDInterface.h, :28 FTPClient.h dropped -- unused / deferred-only (see header block).
//AI(W906-W7-L2) 20260803: golden :29 cMyDB.h split across canary_support.h (MyDBIProcessNew, :2452) and aHotPlateSubstrate.h (MyDBIProcess, :463-:484) -- both already included above.
//AI(W906-W7-L2) 20260803: golden :30 rs232.h re-targeted to atester_shims.h (COM2 shim) -- already included above.
#include "cmydef.h"                                                             // golden ckernel.cpp:31 -- the global universe: bLampManualSetp/bButtonManualStep/bSTEP/FlushFlag/AccessLevel/ATC_SYSTEM(:3358)/bATCInitialFinish(:241)/K_* codes/C_* cylinder ids
#include "cpublic.h"                                                            // golden ckernel.cpp:32 -- RespondASECom (cpublic.h:41) -- :81, :123
#include "mycylin.h"                                                            // golden ckernel.cpp:33 -- Cylinder[] (ProcessAlarm's ALM_CYLINDER arm)
//AI(W906-W7-L2) 20260803: golden :34 ATCInterface.h, :35 ATC_Handler_Side.h dropped -- every ATC symbol used is reachable from cmydef.h / MachineType.h / cprod.h / atester_shims.h.
#include "forms/fShuttleMove.h"                                                 // golden ckernel.cpp:36 -- RE-TARGET of ShuttleMove.h: fShuttleMove (forms/fShuttleMove.h:139) -- :138,:140,:142,:145
#include "forms/fRotate.h"                                                      // golden ckernel.cpp:37 -- FrmRotate (forms/fRotate.h:123) -- :419,:421,:423,:425
//AI(W906-W7-L2) 20260803: golden :38 cShowBinSet.h, :39 common.h dropped -- ShowRunLed-only / zero uses.
//AI(W906-W7-L2) 20260803: golden :40 InOutArmZteach.h re-targeted -- Zteach (:622) is the facade at aHotPlateSubstrate.h:1134, already included above.
//AI(W906-W7-L2) 20260803: golden :41 AutoTeach.h dropped -- fAutoTeach is ShowRunLabel-only (DEFERRED).
//Austin 20190531.01 增加PLC_IO模組及安全相關IO.
//==>
//AI(W906-W7-L2) 20260803: golden :44 MyPLC_IO_Modbus.h, :45 ModbusTCPClient.h dropped -- Enable_PLCSafety_IO is ShowRunLed/ShowRunLabel-only (DEFERRED).
#include "forms/fAOI.h"                                                         // golden ckernel.cpp:46 -- FrmAOI (forms/fAOI.h:113)  //Eastsun 20260515 F023: E7 need FrmAOI pointer -- :502,:504
//<==
//Austin 20190531.01 增加PLC_IO模組及安全相關IO.
#include "acarry.h"                                                             // golden ckernel.cpp:49 -- //AI(ht9045-staterecord-analysis) 20260525 (RogerYang) : for ResetShtMoveTimeoutWatchdog (acarry.h:87)

#include "acarry_shims.h"                                                       //AI(W906-W7-L2) 20260803: ADDED -- SystemNG (:376, :679).
// Golden never declares SystemNG in the source tree at all: it comes from the
// SIBLING BCB6 COMPONENT PACKAGE, D:/HT9045/elec/Component/halarm.h:31
// (`extern bool PACKAGE SystemNG;`, storage HAlarm.cpp:14), which golden
// ckernel.cpp:1 reaches through MachineDefine.h:63 -- the same out-of-tree
// route as PopUpAlarm/ClearAllAlarm (see canary_support.h section 7).  This
// tree's offline home is acarry_shims.h:235, storage acarry_shims.cpp:159.
#include "Config.h"                                                             //AI(W906-W7-L2) 20260803: ADDED -- IniConfig (14 non-deferred uses, first at :261); golden reached it through the BCB6 header web
#include "CosFunction.h"                                                        //AI(W906-W7-L2) 20260803: ADDED -- CosFunction (:66, :108, :381); same reason
#include "myTimer.h"                                                            //AI(W906-W7-L2) 20260803: ADDED -- TQPF_Timer for the file-scope SLK1Delay/SLK2Delay at golden :1917-:1918
//---------------------------------------------------------------------------
//AI(W906-W7-L2) 20260803: golden :51 `#pragma package(smart_init)` dropped -- BCB6 package initialisation, no MinGW equivalent (same drop as acarry.cpp:62 / acatchtray.cpp:71).
//AI(W906-W7-L2) 20260803: golden :52 defines `bool bLockByServer=false;  // 2009.11.13 , Joye`.
// NOT defined here -- Automation/automation.cpp:55 already owns the definition
// (`bool bLockByServer = false;`, landed by AI(W906-Automation) 20260716 with the
// note at automation.cpp:44-49 recording that it was hoisted out of the then-
// untranslated golden ckernel.cpp:52).  Defining it again would be an ODR
// violation and a duplicate-symbol link error, so ckernel.cpp takes the extern
// and automation.cpp keeps the storage.  Initial value and semantics unchanged.
extern bool bLockByServer;                                                      // golden ckernel.cpp:52 -- storage: Automation/automation.cpp:55  // 2009.11.13 , Joye
// =============================================================================
//  FRAGMENT F1 -- golden ckernel.cpp:54-357 (304 golden lines).
//  Eight top-level entities in golden order, all translated in FULL -- no gate,
//  no stub, no reordering, no statement moved:
//      WaitManualStepKey    golden :54-94
//      WaitManualStartKey   golden :96-131
//      WaitManualRetryKey   golden :133-149
//      CheckThermo          golden :151-186
//      extern AccelateTask  golden :187
//      StopAllDestroy       golden :189-209
//      CheckBinSet          golden :211-250
//      ChangeUseSuckMode    golden :252-357  (:252 is its Chinese banner comment)
//  plus golden's own `//---` / `//------` separators at :95, :132, :150, :188,
//  :210 and :251, carried verbatim.  The fragment STOPS at golden :357 (the
//  closing brace of ChangeUseSuckMode); golden :358 (the separator) and :359
//  (ScanSystemSensor) belong to the next fragment.
//
//  Golden's Chinese trailing comments (:56, :81, :98, :123, :222, :238, :246,
//  :252, :261, :286, :289, :309) are decoded via cp950 and carried as real
//  UTF-8.  ZERO U+FFFD.
// =============================================================================

// -----------------------------------------------------------------------------
//AI(W906-W7-L2) 20260803: NO COLOUR SHIM IS NEEDED HERE -- and an earlier draft of
// this fragment wrongly added one.  Golden writes `clYellow` (:69, :111) and
// `(TColor)0x00804000` (:71, :113) unqualified, taking both from Graphics.hpp via
// the BCB6 <vcl.h> precompiled header.  In this tree both already arrive through
// the include block above, at global scope:
//   TColor   -- ported cmydef.h:16 `typedef int TColor;` (also acatchtray_shims.h:126)
//   clYellow -- ported acatchtray_shims.h:128 `const TColor clYellow = 0x0000FFFF;`
//               (guard HT9045_TCOLOR_SHIM, acatchtray_shims.h:124-129)
// acatchtray_shims.h is on this TU's include path because ScanSystemSensor needs
// InitProcessSingleMotorTask from it (see the prologue).  The draft shim used its
// own guard name, so it did NOT collapse against HT9045_TCOLOR_SHIM and the first
// real build failed with `redefinition of 'const TColor clYellow'`.  Removed
// rather than re-guarded: duplicating a definition the tree already owns is how
// two guard names for one concept get established.
// -----------------------------------------------------------------------------

// =============================================================================
//AI(W906-W7-L2) 20260803: WaitManualStepKey -- golden ckernel.cpp:54-94, in FULL.
// Declared in golden ckernel.h:22 (`bool WaitManualStepKey();`), so external
// linkage is unchanged.
//
// Shape: a per-tick POLL, not a blocking wait despite the name.  It (a) mirrors
// the global blink phase FlushFlag into the STEP lamp latch, (b) repaints the
// on-screen key, and (c) reports true EXACTLY ONCE on the tick a manual STEP
// arrives from any of three sources -- the on-screen button latch
// (fContact->bSetupStep), the rear-panel physical key (Sen[SnRKManualStep]) or
// the QualComm-US software button (bButtonManualStep).
//
// SUBSTRATE: fContact is the TfContactShim facade (atester_shims.h:251); the four
// members touched here are fShow (:157), cbOneTouchAutoContactHight (fContactForm, forms/fContact.h:1231 -- AI(W906-W156) 20261007 (St02-E) (c)),
// btnTStep (:206) and bSetupStep (:223).  fMain->BtnSTEP is forms/fMain.h:696.
// =============================================================================
bool WaitManualStepKey()
{
    //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- this early-out and the
    // structurally IDENTICAL one in WaitManualStartKey (golden :98-99) test the
    // very same condition and return OPPOSITE values: STEP returns true here,
    // START returns false there.  Read together, one-touch Auto Contact Height
    // auto-GRANTS every step request and auto-DENIES every start request, so the
    // contact-height sequence single-steps under its own control instead of
    // free-running.  Whether that asymmetry was intended or is a copy-paste slip
    // in golden, it is load-bearing: harmonising the two signs would let the
    // one-touch sequence free-run.  Both signs reproduced verbatim.
    if(W906_FormShowing("fContact", fContact->fShow) && fContactForm->cbOneTouchAutoContactHight->Checked==true)  //Ifor 20220803 add 一鍵完成Auto Contact Hight  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批1：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）   // AI(W906-W156) 20261007 (St02-E) (c): golden fContact = the real TfContact (fContactForm); the box the run-start single entry fills (FileRW/DeviceForm_File.cpp:1031); a tick made mid-run counts from the next run start (the web stand-in is cleared by every FormShow re-read)
        return true;

    bLampManualSetp=FlushFlag;

    if(bLampManualSetp)
        fContact->btnTStep->Caption="T.Step";
    else
        fContact->btnTStep->Caption="";

    if(CosFunction.bEnableSoftWareControlButton)                                //ChungHung 20150609 add only for TSMC
    {
        if(bLampManualSetp)
            fMain->BtnSTEP->Color=clYellow;
        else
            fMain->BtnSTEP->Color=(TColor)0x00804000;
    }

    if(fContact->bSetupStep || Sen[SnRKManualStep].IsOn() || bButtonManualStep) //ChungHung 20150526 add for QualComm US
    {
        bButtonManualStep=false;                                                //ChungHung 20150526 add for QualComm US
        //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED -- `fContact->bSetupStep=false;`
        // is written TWICE inside this one block, here at golden :77 and again at
        // golden :89, with no read of it in between.  The second store is dead.
        // Kept because deleting it is a tidy-up of golden, and because the two
        // stores straddle the IsSafeLockCheck() early-out below -- see the note
        // there for why that placement actually matters.
        fContact->bSetupStep=false;

        if(bSTEP)
        {
            RespondASECom("@e02115Done");                                       //kevin 20160304 回應 ase TrayFeed finish
            bSTEP=false;                                                        //kevin 20160304
        }

        //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- the safety-lock test
        // runs AFTER the key press has already been consumed: bButtonManualStep
        // (:76), fContact->bSetupStep (:77) and bSTEP (:82) are all cleared above.
        // So when the safety lock rejects the request the press is SWALLOWED, not
        // deferred -- the operator must press STEP again once the lock clears.
        // Note also that WaitManualStartKey (golden :96-131) has NO safety-lock
        // check at all: manual START is not gated by IsSafeLockCheck the way
        // manual STEP is.  Both behaviours reproduced as golden wrote them.
        if(IsSafeLockCheck())
            return false;
        bLampManualSetp=false;
        //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- the STEP handler also
        // clears the START lamp latch (golden :88).  WaitManualStartKey has no
        // symmetric `bLampManualSetp=false;`, so the cross-clear runs one way only.
        bLampManualStart=false;
        fContact->bSetupStep=false;

        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
// =============================================================================
//AI(W906-W7-L2) 20260803: WaitManualStartKey -- golden ckernel.cpp:96-131, in FULL.
// Declared in golden ckernel.h:21.  The START twin of WaitManualStepKey above:
// same blink-latch / repaint / three-source poll shape, against bSetupStart
// (atester_shims.h:222), Sen[SnRKManualTStart] and bButtonManualTStart, with the
// ASE hand-shake string "@e02116Done" instead of "@e02115Done".
// =============================================================================
bool WaitManualStartKey()
{
    //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- returns FALSE where the
    // identical test in WaitManualStepKey (golden :56-57) returns TRUE.  See the
    // full note there.  Do NOT harmonise.
    if(W906_FormShowing("fContact", fContact->fShow) && fContactForm->cbOneTouchAutoContactHight->Checked==true)  //Ifor 20220803 add 一鍵完成Auto Contact Hight  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批1：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）   // AI(W906-W156) 20261007 (St02-E) (c): as :237
        return false;

    bLampManualStart=FlushFlag;

    if(bLampManualStart)
        fContact->btnTStart->Caption="T.Start";
    else
        fContact->btnTStart->Caption="";

    if(CosFunction.bEnableSoftWareControlButton)                                //ChungHung 20150609 add only for TSMC
    {
        if(bLampManualStart)
            fMain->BtnT_Start->Color=clYellow;
        else
            fMain->BtnT_Start->Color=(TColor)0x00804000;
    }

    if(fContact->bSetupStart ||
       Sen[SnRKManualTStart].IsOn() ||
       bButtonManualTStart)                                                     //ChungHung 20150526 add for QualComm US
    {
        bButtonManualTStart=false;                                              //ChungHung 20150526 add for QualComm US
        if(bTSTART)
        {
            RespondASECom("@e02116Done");                                       //kevin 20160304 回應 ase TrayFeed finish
            bTSTART=false;                                                      //kevin 20160304
        }
        bLampManualStart=false;
        fContact->bSetupStart=false;
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
// =============================================================================
//AI(W906-W7-L2) 20260803: WaitManualRetryKey -- golden ckernel.cpp:133-149, in FULL.
// Declared in golden ckernel.h:23.  Same poll shape as the two above but on the
// SHUTTLE form, and with FOUR differences worth naming because they are golden's,
// not this translation's: (1) no one-touch early-out, (2) no
// CosFunction.bEnableSoftWareControlButton software-lamp block, (3) no
// IsSafeLockCheck gate, and (4) TWO physical keys are accepted, front
// Sen[SnFKRetry] and rear Sen[SnRKRetry], where STEP/START each accept only the
// rear one.  SUBSTRATE: fShuttleMove is forms/fShuttleMove.h:139; bShuttleRetry
// is :74 and btRetry is :104.
// =============================================================================
bool WaitManualRetryKey()                                                       //JerryYang 20160728
{
    bLampRetry=FlushFlag;

    if(bLampRetry)
        fShuttleMove->btRetry->Caption="Retry";
    else
        fShuttleMove->btRetry->Caption="";

    if(fShuttleMove->bShuttleRetry || Sen[SnFKRetry].IsOn() || Sen[SnRKRetry].IsOn())
    {
        bLampRetry=false;
        fShuttleMove->bShuttleRetry=false;
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
// =============================================================================
//AI(W906-W7-L2) 20260803: CheckThermo -- golden ckernel.cpp:151-186, in FULL.
// NOT declared in golden ckernel.h (see ckernel.h's "NOT DECLARED HERE" note) --
// file-local by convention, external linkage, no forward declaration added.
//
// GOLDEN BUG PRESERVED -- THE WHOLE LOOP IS DEAD.  Golden's for-loop at :161-183
// contains nothing but `continue` statements: there is no `return`, no `break`,
// no assignment, no call anywhere in its body.  Whatever per-zone temperature
// comparison it once guarded has been deleted, leaving only the zone-SKIP
// scaffolding behind.  Consequences, all reproduced verbatim:
//   * the loop cannot make the function return false, so CheckThermo's ONLY
//     false path is the `iHeaterCooling!=0` test at :159-160;
//   * `int i` (:156) exists solely to drive that dead loop;
//   * the iIndexHeatMode ladder handles HeadOnly / ChamberOnly / HeadChamber /
//     SocketChamber but NOT HeadSocket(=4) or HeadChamberSocket(=5)
//     (MachineType.h:538) -- a gap that is invisible today precisely because the
//     loop has no effect.
// Deleting the loop, or "restoring" a comparison into it, would both be rewrites.
// It is translated exactly as golden has it so that any regression test built
// against golden sees the same always-true-unless-cooling behaviour.
//
// The `#ifdef SOFT_SIMULTE` fork is golden's own (:153-185) and is kept as a real
// preprocessor fork, not collapsed: SOFT_SIMULTE is commented out at
// MachineType.h:48 in this tree, so the #else arm is what compiles -- the same
// arm the real machine builds.
// =============================================================================
bool CheckThermo()
{
    #ifdef SOFT_SIMULTE
        return true;
    #else
        int i;
        if(LastSet.iTemperature==Tempture_Hot)
            return true;
        if(iHeaterCooling!=0)
            return false;
        for(i=0; i<tcTotalCount; i++)
        {
            if(i==0 || i==1)  //Hotplate 1 & 2
                continue;
            //Steven 20090926 Start: Index Heating Mode
            if(Temperature.iIndexHeatMode==HeadOnly)       //Head Only (socket & chamber no use)
            {
                if(i==8 || i==9) continue;
            }
            else if(Temperature.iIndexHeatMode==ChamberOnly)  //Chamber Only (head 1 2 3 4 & socket no use)
            {
                if(i>=4 && i<=8) continue;
            }
            else if(Temperature.iIndexHeatMode==HeadChamber)  //Head + Chamber Only (socket no use)
            {
                if(i==8) continue;
            }
            else if(Temperature.iIndexHeatMode==SocketChamber)  //Socket + Chamber Only (Head 1234 no use)
            {
                if(i>=4 && i<=7) continue;
            }
            //Steven 20090926 End
        }
        return true;
    #endif
}
//AI(W906-W7-L2) 20260803: golden :187 -- a file-scope `extern` sitting BETWEEN two
// function bodies, exactly where golden put it (golden does not hoist it to the
// include block).  Kept in place because moving it would reorder the file.  Its
// only use in golden ckernel.cpp is `AccelateTask=1;` at golden :369, inside
// ScanSystemSensor -- a different fragment's range, so nothing in F1 reads it.
// SUBSTRATE GAP (reported, not worked around): golden defines the variable at
// golden csystem.cpp:16458 (`int AccelateTask=1;`), and the ported csystem.cpp has
// NO such definition -- tree-wide grep over .h/.cpp finds the identifier only in a
// comment at forms/fMain.h:621.  This declaration therefore compiles but will not
// LINK once ScanSystemSensor lands and references it.  The integrator owns the
// definition; F1 must not invent one, because defining it here would move storage
// that golden keeps in csystem.cpp.
extern int AccelateTask;
//------------------------------------------------------------------------------
// =============================================================================
//AI(W906-W7-L2) 20260803: StopAllDestroy -- golden ckernel.cpp:189-209, in FULL.
// Declared in golden ckernel.h:20.  Turns every destroy(blow) solenoid off across
// the in-arm, out-arm, both test heads and the tray arm.
//
// SUBSTRATE: the five TMyKitSuck grids are aHotPlateSubstrate.h:624 (InArmSuck),
// :627 (OutArmSuck), :636 (FTestSuck), :637 (BTestSuck) and :633 (CatchTraySuck);
// TMySucker::OffDestroy() is :129.  Offline there is no solenoid, so that seam
// counts calls (aHotPlateSubstrate.h:285-306, :348-358) instead of silently doing
// nothing -- which is what makes this function's grid coverage testable at all.
// =============================================================================
void StopAllDestroy()                                                           //Eliot 2009_12_24
{
    //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED -- the INNER loop bound is
    // `InArmSuck.iMaxRow`, where `InArmSuck.iMaxCol` is the obvious intent: `j` is
    // the COLUMN cursor and iMaxCol is a real, separate member
    // (aHotPlateSubstrate.h:402, beside iMaxRow at :377).  Confirmed by reading
    // golden :191-193 this pass -- golden really writes iMaxRow in both bounds.
    // Effect: the swept region is the iMaxRow x iMaxRow square, not the
    // iMaxRow x iMaxCol grid, so on any non-square picker layout some nozzles keep
    // their destroy solenoid ON after a "stop all".  On the 2x4 in-arm (iMaxRow=2,
    // iMaxCol=4) only columns 0-1 are reached and columns 2-3 are missed.
    // SECOND ODDITY, same two lines: OutArmSuck is swept with INArmSuck's bounds,
    // so an out-arm grid of a different shape is mis-covered as well.
    // Both are golden's; neither is repaired here.
    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxRow; j++)
        {
            InArmSuck.Suck[i][j].OffDestroy();
            OutArmSuck.Suck[i][j].OffDestroy();
        }
    }

    //AI(W906-W7-L2) 20260803: the test-head grids use the COMPILE-TIME row bound
    // MAX_Index_Row (MachineType.h:389, ==2) against the RUNTIME column bound
    // NEW_MAX_Index_Col (cmydef.h:3289) -- golden's own mix, kept as written.
    for(int i=0; i<MAX_Index_Row; i++)
    {
        for(int j=0; j<NEW_MAX_Index_Col; j++)
        {
            FTestSuck.Suck[i][j].OffDestroy();
            BTestSuck.Suck[i][j].OffDestroy();
        }
    }
    //AI(W906-W7-L2) 20260803: the tray arm gets ONE hard-coded nozzle [0][0] --
    // no loop, no bounds member consulted.  Golden's, preserved.
    CatchTraySuck.Suck[0][0].OffDestroy();
}
//------------------------------------------------------------------------------
// =============================================================================
//AI(W906-W7-L2) 20260803: CheckBinSet -- golden ckernel.cpp:211-250, in FULL.
// NOT declared in golden ckernel.h -- file-local by convention, like CheckThermo.
// Validates the bin->tray category map: every hardware tray must be either all
// pass or all fail, and the [interface error] tray must not be a pass tray.
//
// Despite the name it never touches fShowBinSet -- that form is ShowRunLed's, which
// is why golden :38 cShowBinSet.h is dropped from the ported include block.
// =============================================================================
bool CheckBinSet()
{
    int iTrayPassFail[eTrayCount];
    bool bIsPassT6[eTrayCount];

    for(int i=0; i<eTrayCount; i++)
    {
        iTrayPassFail[i]=-1;
        bIsPassT6[i]=false;
    }

    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
    {
        int j=Prod.iT6CatData[i];

        //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED -- this guard is ONE-SIDED
        // and MAGIC-NUMBERED.  It rejects j>=9 but never checks j<0, so a negative
        // Prod.iT6CatData[i] (the field is a plain `int`, cprod.h:511 in golden /
        // cprod.h:512 here, and -1 is the tree's usual "unassigned" marker) indexes
        // iTrayPassFail[] and bIsPassT6[] OUT OF BOUNDS at :229/:231/:232 below.
        // The literal 9 is also unrelated to the arrays' real extent eTrayCount
        // (MachineType.h:1104, ==33): it is the count of the ORIGINAL Auto/Fix tray
        // block, frozen in place while eTrayCount grew to include the magazines.
        // Preserved exactly -- widening the guard would change which category maps
        // this function accepts, and clamping it would mask whatever upstream
        // writes the negative.
        if(j>=9)
            continue;

        if(iTrayPassFail[j]<0)
        {
            iTrayPassFail[j]=Prod.bIsPassBin[i]?1:0;
            bIsPassT6[j]=Prod.bIsPassBin[i];
        }
        else
        {
            //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- `bool != int`.
            // bIsPassT6[] is bool but Prod.bIsPassBin[] is declared `int`, in BOTH
            // trees identically (golden cprod.h:515 `int bIsPassBin[TEST_MAX_BIN];
            // //Steven 20240118 : bIsPass --> bIsPassBin`, ported cprod.h:516, same
            // line).  The bool is promoted to int, so any bIsPassBin value other
            // than 0/1 compares unequal to the stored 0/1 and reports a spurious
            // pass/fail clash.  The types are the same on both sides, so this
            // translation is faithful; recorded because the `b` prefix on an `int`
            // is exactly the kind of thing a later reader would "fix".
            if(bIsPassT6[j]!=Prod.bIsPassBin[i])
            {
                ShowMyMessage("Category define has pass and fail at same field", "Pass Bin和Fail Bin不可放在同一個位置", "CheckBinSet");
                return false;
            }
        }
    }

    //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED -- Prod.iIfErrorT6 (cprod.h:520)
    // indexes iTrayPassFail[] with NO range check at all, not even the one-sided
    // j>=9 applied inside the loop above.  Golden's, unmodified.
    if(iTrayPassFail[Prod.iIfErrorT6]==0)
    {
        ShowMyMessage("Can not place to a pass tray of the category of [interface error]", "[interface error]不可以跟Pass Bin擺在同一個位置", "CheckBinSet");
        return false;
    }
    return true;
}
//------------------------------------------------------------------------------
// =============================================================================
//AI(W906-W7-L2) 20260803: ChangeUseSuckMode -- golden ckernel.cpp:252-357, in FULL.
// NOT declared in golden ckernel.h -- file-local by convention.  Its only golden
// call site is ScanSystemSensor at golden :370, which sits BELOW this definition
// in the same TU, so exactly as in golden no forward declaration is needed.
//
// What it does: at HOT temperature only, force TestIF/TestIF_File.iUseSuckMode
// down to the number of nozzles the hot path may actually use, per test mode.
// It writes iUseSuckMode into BOTH the live struct and the *_File mirror
// (cprod.h:2576/2577) on every arm that writes at all -- always as a pair, never
// one without the other.
//
// STRUCTURAL NOTE ON THE `break` STATEMENTS.  Four of the arms below reach a
// `break` from inside a nested `if` (golden :265, :293, :315, :330).  There is no
// enclosing loop, so each binds to the SWITCH -- i.e. it leaves iUseSuckMode
// untouched, which is that arm's entire purpose.  Reproduced verbatim; see the
// per-site notes.
// =============================================================================
//jou 2010-09-03 加熱暫時先關閉使用1x2-4,1x4-8,2x2-8
void ChangeUseSuckMode()
{
    if(LastSet.iTemperature==Tempture_Hot)
    {
        switch(TestIF.iTestMode)
        {
            case DualSite: //1x2
                //ChungHung 2011/02/10 start
                if(i8PickerHPMode==iHPWideHP && IniConfig.b1x2Use4Suck)    //JerryYang 20161007 簡化Hotplate判斷式 //Steven 20151117 : 2x2 8Picker at Hot mode
                {
                    if(HotPlateForm.XDivision==4 || HotPlateForm.XDivision==6 || HotPlateForm.XDivision==8 || HotPlateForm.XDivision==12)
                    {
                        //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- a BARE
                        // `break` arm that writes NOTHING.  On a wide hot plate with
                        // 1x2-uses-4-suckers enabled and an X division of 4/6/8/12,
                        // iUseSuckMode is deliberately left at whatever the recipe
                        // already set (the 4-nozzle hot mode), instead of being
                        // knocked down to 2 as every other path here does.  It
                        // binds to the switch, so it is equivalent to falling
                        // through to the arm's own `break` at golden :279 -- kept as
                        // an explicit early exit exactly as golden wrote it, because
                        // collapsing it would erase the intent marker.
                        break;
                    }
                    else
                    {
                        TestIF_File.iUseSuckMode=2;     //Steven 20161102 : 4 --> 2
                        TestIF.iUseSuckMode=2;
                    }
                }
                else
                {
                    TestIF_File.iUseSuckMode=2;
                    TestIF.iUseSuckMode=2;
                }
                //ChungHung 2011/02/10 end
                break;
            case QualSite2X2N:          //Frank 20200520 2X2NN Mode
//            case _6Site2X3N:            //Steven 20220425 : 2X3NN Mode
                TestIF_File.iUseSuckMode=2;
                TestIF.iUseSuckMode=2;
                break;
            case QualSite1X4: //1x4
            case _8Site1X4:   //ChungHung 20150528 add for 海思 _8Site1x4
            case _8Site2X4N:  //Wei 20231211 : 2X4NN Mode
                //ChungHung 2011/01/18 start
                if(i8PickerHPMode==iHPWideHP && IniConfig.b1x4Use8Suck)    //JerryYang 20161007 簡化Hotplate判斷式   //Steven 20151117 : 2x2 8Picker at Hot mode
                {
                    if(HotPlateForm.XDivision==4 || HotPlateForm.XDivision==6 || HotPlateForm.XDivision==8 || HotPlateForm.XDivision==12)
                    {
                        //AI(W906-W7-L2) 20260803: second bare-`break` arm, same shape
                        // as golden :265 -- leaves iUseSuckMode alone at 8.
                        break;
                    }
                    else
                    {
                        TestIF_File.iUseSuckMode=4;
                        TestIF.iUseSuckMode=4;
                    }
                }
                else
                {
                    TestIF_File.iUseSuckMode=4;
                    TestIF.iUseSuckMode=4;
                }
                //ChungHung 2011/01/18 end
                break;
            case QualSite2X2: //2x2
                //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- note this arm's
                // XDivision list is 6/8/12 while the two arms above test 4/6/8/12.
                // The `==4` case is MISSING here and only here.  Golden's; not
                // harmonised.
                if(i8PickerHPMode==iHPWideHP && IniConfig.b2x2Use8Suck)    //JerryYang 20161007 簡化Hotplate判斷式  //加熱模式八進四出 //Steven 20151117 : 2x2 8Picker at Hot mode
                {
                    if(HotPlateForm.XDivision==6 ||
                       HotPlateForm.XDivision==8 ||
                       HotPlateForm.XDivision==12)  //Steven 20180409 (Jou) : Add 2x2 support X=12
                    {
                        break;
                    }
                    else
                    {
                        TestIF_File.iUseSuckMode=4;
                        TestIF.iUseSuckMode=4;
                    }
                }
                else
                {
                    //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- this
                    // CUSTOMER_CODE test is ALMOST a no-op.  Its else arm (golden
                    // :338-342) and the ASE/SIGURD inner else (golden :332-336) are
                    // byte-identical (`iUseSuckMode=4` into both structs), so the
                    // customer check changes behaviour on ONE input only: an
                    // XDivision of 6 or 8, where ASE-Kaohsiung / SIGURD-PeiXing
                    // break out and leave iUseSuckMode alone while everyone else is
                    // knocked down to 4.  Preserved as four separate arms rather
                    // than folded, because folding would delete that customer seam.
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                       //kevin 20161006
                       CUSTOMER_CODE==CC_SIGURD_PeiXing)                        //Sam 20250415 : add SIGURD_PeiXing
                    {
                        if(HotPlateForm.XDivision==6 || HotPlateForm.XDivision==8)
                        {
                            break;
                        }
                        else
                        {
                            TestIF_File.iUseSuckMode=4;
                            TestIF.iUseSuckMode=4;
                        }
                    }
                    else
                    {
                        TestIF_File.iUseSuckMode=4;
                        TestIF.iUseSuckMode=4;
                    }
                }
                break;
            //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED -- the ENTIRE
            // _8Site2X4 / _16Site4X4 arm is a no-op.  Golden :347-353 tests
            // USE_IN_OUT_ARM_Y_PITCH against iXYPitch16Picker / iXYPitch16Bd_Be and
            // then supplies TWO EMPTY BRACE PAIRS -- the then-branch (:349-350) and
            // the else-branch (:351-353) are both `{ }` -- before falling to
            // `break`.  Nothing is read, nothing is written, and the 16-picker
            // distinction the test was written to make has no consequence.
            // Reproduced verbatim, empty braces and all: an 8-site 2x4 or 16-site
            // 4x4 machine at HOT temperature really does leave iUseSuckMode
            // untouched today, and any regression test built against golden expects
            // exactly that.  Collapsing this to a bare `break` would read as
            // "handled deliberately" and lose the abandoned-intent marker.
            case _8Site2X4:
            case _16Site4X4:
                if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                  //Steven for HT1032
                   USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                     //Ztex 2023.12.06 Add HT-1032
                {
                }
                else
                {
                }
                break;
        }
    }
}
//------------------------------------------------------------------------------
// ===========================================================================
//  ScanSystemSensor  -- golden ckernel.cpp:359-691 (333 lines).  TRANSLATED IN
//  FULL, statement for statement, with ONE gated call (golden :523,
//  DoInArm_SuckerMap -- the gate sits at its own line, with the reason).  //AI(W906-AMB-L2) 20260929: that gate is now retired (:1033) -- zero gated calls remain here
//
//  WHAT IT IS.  The START/PAUSE edge handler.  A single call does at most one
//  of three things:
//    * SoftStart edge (golden :365-533) -- the whole start-up admission
//      sequence: re-arm the work parameters, clear the alarm queue, then SIX
//      veto gates -- continue-fail bin selection (:381), host lock (:399), kit
//      temperature (:405), motor home (:411), ATC alarm sensor (:442) and safe
//      door (:526) -- any of which `return false` and leave SystemStart alone.
//    * SoftStop edge (:534-615) -- stop the motors, then snapshot every
//      nozzle's LEARNED vacuum on/off duration into LastSet and ReStart() the
//      nozzle, so the next start resumes from the measured timings.
//    * neither -- the tail (:617-690) runs on EVERY call regardless: the
//      tray-end lamp, the stopped-state index vacuum sweep, the SystemNG abort.
//
//  RETURN VALUE is `SystemStart` (:690), NOT "something happened".
//
//  CALLERS -- CORRECTED 20260803 BY INDEPENDENT AUDIT.  An earlier draft of this
//  banner said "Golden's only call site is csystem.cpp:16894" and that this
//  function "lands directly unit-testable and uncalled".  BOTH WERE FALSE.
//  Golden has TWO call sites, established by an exhaustive scan of 786 files over
//  BOTH the golden version tree AND the out-of-tree package D:\HT9045\elec\Component:
//    1. golden csystem.cpp:16894, inside MainProc's mode ladder.  Ported
//       csystem.cpp keeps that ladder as a three-line `#if 0` placeholder
//       (ported csystem.cpp:415-418), so THAT path is genuinely dormant here, and
//       hoisting it into MainProc's active spine would be an ordering deviation
//       invented by this wave.  csystem.cpp was NOT edited.
//    2. golden ckernel.cpp:2511, inside ProcessAlarm -- the SAME file, translated
//       by this very wave, and reproduced live and un-gated at ported :2852.  So
//       an in-file caller DOES exist and this function is reachable from any suite
//       that calls ProcessAlarm().
//  TEST AUTHORS NEED THIS: each PopUpAlarm iteration runs ScanSystemSensor with
//  SoftStop and SoftStart both forced false immediately beforehand (golden
//  :2509-2511 -> ported :2850-2852), i.e. neither edge branch is taken on that path.
//  Honest qualifier, stated as such: ProcessAlarm itself has no ported caller yet,
//  so the chain is dormant in the tree even though the call is live in the TU.
//
//  OFFLINE BRANCH SELECTION -- what actually executes with this tree's
//  defaults, so a test author knows what to override:
//    INDEX_SUCKER_TYPE = 0 (cmydef.cpp:3167)  -> every `INDEX_SUCKER_TYPE==1`
//        guard (:575, :594, :632, :651) and the :672 CloseSuckok arm is FALSE.
//    InArmSuck.iMaxRow / iMaxCol = 0 -- TMyKitSuck's constructor
//        (aHotPlateSubstrate.cpp:346-371) never assigns either, and the grids
//        are namespace-scope, so they stay zero-initialised: the :543-559
//        arm-nozzle snapshot loop executes ZERO times until a test sets them.
//    MAX_Index_Row = 2 (MachineType.h:389), NEW_MAX_Index_Col = 8
//        (cmydef.cpp:3513) -> the :561-612 index loop DOES run, 16 iterations,
//        which makes its :565-571 snapshot + ReStart the strongest offline hook
//        in the function (TMySucker's ctor seeds VacuumOnTime=120 /
//        VacuumOffTime=70, aHotPlateSubstrate.h, so copying them into an
//        all-zero LastSet is observable).  The two CatchTraySuck scalars at
//        :613-614 are unconditional and observable for the same reason.
//    FrmAOI = A LIVE INSTANCE (forms/fAOI.cpp:35 `TFrmAOI *FrmAOI = new
//        TFrmAOI();`, unconditional at static init) -> the :502 guard is TRUE
//        and :504 DOES run offline.  Corrected after audit: an earlier draft of
//        this table said NULL, copying a stale brief.  The substrate pass
//        deliberately did the opposite and says so at forms/fAOI.cpp:19-24 and
//        under the forms/fAOI.h banner "THE NULL QUESTION -- I DID NOT DO WHAT
//        THE BRIEF ASKED", because golden HT9045.cpp:233 creates the form at
//        application start, so on a real machine the guard is true by the time
//        SoftStart can be set.  A W7 test asserting bSimulateTopBtm stays false
//        would fail, and would look like a translation bug.
//    SOFT_SIMULTE NOT defined (MachineType.h:48 is commented out) -> all three
//        `#ifndef SOFT_SIMULTE` regions (:392-395, :509-514, :525-532) are live.
//
//  GOLDEN SPELLINGS KEPT: `bYpitchNeddHome` (Nedd) and
//  `bIndexCheckNoStopVaccum` (Vaccum) are golden's own misspellings and are the
//  tree-wide symbol names.  Not corrected.  CITATIONS CORRECTED AFTER AUDIT --
//  an earlier draft gave PORTED line numbers under a heading that says GOLDEN.
//  Golden declares them at golden cmydef.h:4688 (`extern bool bYpitchNeddHome;`
//  //kevin 20180823) and golden cmydef.h:3332 (`extern bool
//  bIndexCheckNoStopVaccum;` //jou 2012-05-09), both read cp950 this pass.
//
//  Big5: every Chinese comment below was decoded from golden via cp950 and is
//  stored here as real UTF-8.  ZERO U+FFFD.
// ===========================================================================
bool ScanSystemSensor()
{
    AnsiString Str2;
    bool CloseSuckok=true;                                                      //kevin 20110505
    bool bConsFail=false;

    if(SoftStart==true)                                                         //啟動檢查
    {
        fMain->MainFormChange();

        //AI(W906-W7-L2) 20260803: SUBSTRATE GAP (link-time, not compile-time).
        // This assignment resolves against `extern int AccelateTask;` at golden
        // ckernel.cpp:187, which is inside fragment F1's range.  Golden DOES
        // define the object -- csystem.cpp:16458 `int AccelateTask=1;` -- but the
        // ported csystem.cpp is a partial translation and no definition exists
        // anywhere in this tree.  Written exactly as golden writes it: gating an
        // assignment would silently alter the start-up sequence.  INTEGRATOR
        // ACTION -- land `int AccelateTask=1;` before ckernel.cpp is linked.
        AccelateTask=1;
        ChangeUseSuckMode();
        SetWorkParameter();
        fHome->fAbort=false;
        fHome->iHomeStep=1;
        for(int i=0; i<TOTAL_MOTOR; i++)
            InitProcessSingleMotorTask(i);
        SystemNG=false;
        ClearAllAlarm();
        SoftStart=false;
        SoftStop=false;

        if(CosFunction.bConFailFoolProofing==true &&                            //jou 2014-09-28 continue fail 防呆提醒,總開關開啟為On時,BIN別未勾選任何continue fail alarm
           (Prod.bContsFailBySocket==true || Prod.bContsFailByHead==true))
        {
            for(int i=0; i<iTestBinCount; i++)
            {
                if(Prod.bConsFail[i]==true)
                    bConsFail=true;
            }

            if(bConsFail==false)
            {
            //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- the SOFT_SIMULTE
            // guard wraps BOTH the operator message and the `return false`, so
            // under SOFT_SIMULTE this whole `if(bConsFail==false)` body is empty
            // and the continue-fail fool-proofing silently passes.  SOFT_SIMULTE
            // is not defined in this tree, so the veto is live.  Golden's
            // column-0 directives here (against the indented `#ifndef` style it
            // uses at :509 and :525 in this same function) are kept as written.
#ifndef SOFT_SIMULTE
                ShowMyMessage("Bin Select Form no click any continue fail error!","Bin Selset 頁面沒有選擇任何連續錯誤BIN別!");
                return false;
#endif
            }
        }

        if(bLockByServer)                                                       //Steven 20100820 : For OLP
        {
            ShowMyMessage("Lock by Host, need unlocked from HOST or restart program !!", "被網路控管中心鎖住必須由網路控管解除或重啟程式!!");
            return false;
        }

        if(iHome==0 && CheckThermo()==false)
        {
            ShowMyMessage("Kit tempture too high!!", "治具目前溫度尚未冷卻,不能進行測試!!", "ScanSystemSensor");
            return false;
        }

        if(iHome==0 && W906_FormShowing("fSetup", fSetup->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批1：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
        {
            if(fAllMotorHome)
            {
                if(bNeedArmZHome)
                {
                    // SUBSTRATE GAP (reported, not worked around) -- third member of
                    // the same family as AccelateTask (:816) and IsSafeLockCheck
                    // (:280/:2001).  Golden defines this at csystem.cpp:4858-4862
                    // (`iAllArmZHomeTask=1;` + `iAllArmZHomeCount=0;`, with the two
                    // globals at csystem.cpp:4856-4857); ported csystem.cpp has only
                    // the declaration (csystem.h:94) and NO definition, so this call
                    // compiles but will not LINK into any executable.  Confirmed by
                    // `nm -C --undefined-only` on ckernel.cpp.obj diffed against all
                    // 13 ht9045_*/vclcompat archives: AccelateTask, IsSafeLockCheck
                    // and InitDoArmZHome are the only three left unresolved.
                    // ACTION for the integrator -- land golden's REAL two-assignment
                    // body; an empty stub would silently leave iAllArmZHomeTask /
                    // iAllArmZHomeCount un-armed on a bNeedArmZHome START edge.
                    InitDoArmZHome();
                }
                else if(FrmRotate->bRotateInHome)                               //kevin 20130706 rotat single home
                {
                    FrmRotate->InitialInRotateHome();                           //Steven 20170329 (wei) : Add individual rotate motor
                }
                else if(FrmRotate->bRotateOutHome)                              //kevin 20130706 rotat single home
                {
                    FrmRotate->InitialOutRotateHome();                          //Steven 20170329 (wei) : Add individual rotate motor
                }
                //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED -- golden :427-430
                // tests `bYpitchNeddHome` and, when it is true, assigns `true` to
                // that same flag.  The assignment changes nothing; the branch's
                // ONLY real effect is that taking it SKIPS the final `else`
                // (:431-438), so the `CheckMotorHome()==false -> "Must home again"
                // -> return false` gate is bypassed whenever a Y-pitch home is
                // pending.  A clearing `=false`, or an actual home call, was
                // plainly meant.  Translated verbatim -- do NOT "fix" it.
                else if(bYpitchNeddHome)                                        //kevin 20180823
                {
                    bYpitchNeddHome=true;
                }
                else
                {
                    if(CheckMotorHome()==false)
                    {
                        ShowMyMessage("Must home again", "需要重新歸零", "ScanSystemSensor");
                        return false;
                    }
                }
            }
        }

        if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true &&
           Temperature.bATCHandlerStart==true && (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite))    //jou 2012-03-14 增加ATC控制方式
        {
            if(bATCInitialFinish==false || COM2->ATCAlarmSenCheck()==false)
            {
                ShowMyMessage("ATC Alarm Sensor Off,please check ATC system is OK!","ATC警報偵測,請確認ATC系統是否正常!");
                return false;
            }
        }

        if(IniConfig.bA36OpenDoorSetErrBin)                                     //JerryYang 20210901 : Microchip要求開安全門要分ERROR BIN
        {
            if(bIsOpenDoorNeedSetErrBin)
            {
                bIsOpenDoorNeedSetErrBin=false;
                int iCT=0;
                AnsiString sBuffer="";
                iCT=FRCarryKit.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Open safe door Out Shuttle1 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                FRCarryKit.SetAllRealIC2InterfaceBin();
                iCT=BRCarryKit.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Open safe door Out Shuttle2 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                BRCarryKit.SetAllRealIC2InterfaceBin();
                iCT=FTestSuck.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Open safe door Index Arm1 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                FTestSuck.SetAllRealIC2InterfaceBin();
                iCT=BTestSuck.CountRealIC();
                if(iCT>0)
                {
                    sBuffer.printf("Open safe door Index Arm2 Place to Error bin : Device=%d;",iCT);
                    MyDBIProcess("Message", sBuffer);
                }
                BTestSuck.SetAllRealIC2InterfaceBin();
                TestSocket.SetAllRealIC2InterfaceBin();
            }
        }
        else
        {
            bIsOpenDoorNeedSetErrBin=false;
        }

        SetMotorSpeed();
        SetSuckRetryCount();
        bLampSkip=false;
        bLampRetry=false;
        bLampTrayEnd=false;
        //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- golden :500 is a
        // commented-out `bLampAlarmReset=false;` starting at column 1, sitting
        // directly above the live one at :501.  Dead text, carried verbatim.
//        bLampAlarmReset=false;
        bLampAlarmReset=false;
        //AI(W906-W7-L2) 20260803: OFFLINE BRANCH, CORRECTED AFTER AUDIT.  This
        // tree's FrmAOI is NOT null: forms/fAOI.cpp:35 allocates a live instance
        // at static init (`TFrmAOI *FrmAOI = new TFrmAOI();`), unconditionally.
        // So golden's :502 guard is TRUE offline and :504 DOES execute.  The
        // substrate pass chose that deliberately against its own brief and
        // documents why at forms/fAOI.cpp:19-24 (golden HT9045.cpp:233 creates
        // the form at application start).  Test authors: bSimulateTopBtm WILL be
        // true after a SoftStart edge -- assert that, not the opposite.
        if(FrmAOI!=NULL)                                                        //Eastsun 20260515 F023: E7 lock AOI simulate at start
        {
            FrmAOI->bSimulateTopBtm=true;
        }

        if(SHUTTLE_FLOODGATE==1)                                                //Ifor 20260327 add:Tray Feed後START需先開啟Shuttle閘門,避免撞機
        {
            #ifndef SOFT_SIMULTE
            Cylinder[C_Shuttle1Floodgate].Off();
            Cylinder[C_OutShuttle1Floodgate].Off();
            Cylinder[C_Shuttle2Floodgate].Off();
            Cylinder[C_OutShuttle2Floodgate].Off();
            #endif
        }

        SystemStart=true;
        ResetShtMoveTimeoutWatchdog();                                          //AI(ht9045-staterecord-analysis) 20260525 (RogerYang) : PAUSE/START 邊界重置 watchdog, 避免暫停期間累積
        Str2="KS4,4,4,4;VT0.1,0.1,0.1,0.1;";                                    //Steven 20181231 : Galil平滑曲線改成4,4,4,4    // KS is pulse command driver use for  Smoothing  ,Large value of KS provide greater smoothing
        MOT[MTestY1].Gali_Command(Str2, __FUNC__);
        fMain->ChangeLevelAttr();

        //AI(W906-W7-L2) 20260803: [HISTORY -- superseded by AMB-L2 20260929 at :1033] GATED -- golden :523 is a live call to
        // DoInArm_SuckerMap() (golden declaration ainarm2.h:148), which rebuilds
        // the in-arm nozzle->site map after a site-mapping change.  It has NO
        // ported home: the identifier exists in this tree only as two FILE-LOCAL
        // no-op stubs -- AutoClean/AutoClean.cpp:127 `static void
        // DoInArm_SuckerMap() {}` and csystem.cpp:2730-2731 (macro-renamed to
        // W7C2_DoInArm_SuckerMap) -- neither of which is visible from here, and
        // golden's ainarm2.h was dropped from this TU's include block.  Copying
        // that stub pattern would put a silently-empty body behind a call that
        // reads as complete, so the call is GATED instead: the golden statement
        // stays visible in golden order and un-gating is a one-line edit.
        // INTEGRATOR ACTION.
        //#if 0 // TODO(W7-L2/INTEGRATOR): DoInArm_SuckerMap() has no ported definition -- only two file-local no-op stubs elsewhere in the tree.  Un-gate when the real ainarm2 body lands.   //AI(W906-AMB-L2) 20260929: gate retired -- the real body landed at ainarm2.cpp:4488 (golden ainarm2.cpp:965-972) and is declared extern at aHotPlateSubstrate.h:1096 (included at :114); golden ckernel.cpp:523 is a live call
        DoInArm_SuckerMap();                                                    //AI(W906-AMB-L2) 20260929: live again, golden ckernel.cpp:523
        //#endif //AI(W906-AMB-L2) 20260929: gate retired (see :1033)

        #ifndef SOFT_SIMULTE
        if(CheckSafeDoorIsClosed()==false)                                      //jou 20171030 (Steven) : 修正安全門未關按Start時IndexArm會先動作
        {
            SystemStart=false;
            StopAllMotor();
            return false;
        }
        #endif
        { extern void ResetShakeShuttleTimeOut(); ResetShakeShuttleTimeOut(); }   }   //AI(W906-W188) 20261010 (NB2-1): golden 913 ckernel.cpp:534-535 (ht9045-v899 20260831: reset the shake timeout only after START really succeeded, so pause / alarm time is not counted)
    else if(SoftStop==true)                                                     //暫停檢查
    {
        StopAllMotor();
        SoftStop=false;
        SystemStart=false;
        bLampAlarmReset=false;
        ResetShtMoveTimeoutWatchdog();                                          //AI(ht9045-staterecord-analysis) 20260525 (RogerYang) : PAUSE/START 邊界重置 watchdog, 避免暫停期間累積
        fMain->ChangeLevelAttr();

        //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED -- this loop snapshots and
        // ReStart()s BOTH arms, but both bounds come from the IN-arm grid only
        // (:543 `InArmSuck.iMaxRow`, :545 `InArmSuck.iMaxCol`).  OutArmSuck's own
        // extents are never consulted, so on any machine whose out-arm grid is
        // larger than its in-arm grid the surplus out-arm nozzles are neither
        // snapshotted into LastSet.iOutArmVacuumDummy*Time nor ReStart()ed nor
        // Normal()ed at PAUSE.  Same family as the `j<InArmSuck.iMaxRow` column
        // bound golden uses in StopAllDestroy at :193.  Bounds copied verbatim.
        // OFFLINE: iMaxRow/iMaxCol are both 0 (TMyKitSuck's ctor,
        // aHotPlateSubstrate.cpp:346-371, assigns neither), so this loop runs
        // ZERO times until a test sets them.
        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                LastSet.iInArmVacuumDummyOnTime[i][j]=InArmSuck.Suck[i][j].VacuumOnTime;
                LastSet.iOutArmVacuumDummyOnTime[i][j]=OutArmSuck.Suck[i][j].VacuumOnTime;
                LastSet.iInArmVacuumDummyOffTime[i][j]=InArmSuck.Suck[i][j].VacuumOffTime;
                LastSet.iOutArmVacuumDummyOffTime[i][j]=OutArmSuck.Suck[i][j].VacuumOffTime;

                InArmSuck.Suck[i][j].ReStart();
                OutArmSuck.Suck[i][j].ReStart();
                if(InArmSuck.Item[i][j]==NULL_IC)
                    InArmSuck.Suck[i][j].Normal();
                if(OutArmSuck.Item[i][j]==NULL_IC)
                    OutArmSuck.Suck[i][j].Normal();
            }
        }

        //AI(W906-W7-L2) 20260803: unlike the arm loop above, THIS loop is bounded
        // by config constants -- MAX_Index_Row (=2, MachineType.h:389) and
        // NEW_MAX_Index_Col (=8, cmydef.cpp:3513) -- so it runs 16 times with this
        // tree's defaults, which makes :565-571 (snapshot + ReStart) the main
        // observable of the entire PAUSE branch.  The DWORD -> int narrowing on
        // :565-568 is golden's own: TMySucker::VacuumOnTime/VacuumOffTime are
        // DWORD (aHotPlateSubstrate.h) while LastSet's arrays are int
        // (canary_support.h:232-239).  Reproduced, not "cleaned up".
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<NEW_MAX_Index_Col; j++)
            {
                LastSet.iFTestArmVacuumDummyOnTime[i][j]=FTestSuck.Suck[i][j].VacuumOnTime;
                LastSet.iBTestArmVacuumDummyOnTime[i][j]=BTestSuck.Suck[i][j].VacuumOnTime;
                LastSet.iFTestArmVacuumDummyOffTime[i][j]=FTestSuck.Suck[i][j].VacuumOffTime;
                LastSet.iBTestArmVacuumDummyOffTime[i][j]=BTestSuck.Suck[i][j].VacuumOffTime;

                FTestSuck.Suck[i][j].ReStart();
                BTestSuck.Suck[i][j].ReStart();

                if(IniConfig.bChangeKitNoHardStop==true)                        //jou 2015-12-08 Xilinx 驗證用
                {
                    if(INDEX_SUCKER_TYPE==1 && W906_FormShowing("fContact", fContact->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批1：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
                    {
                        if(bArm1NeedSuck==false &&                              //JerryYang 20240816 : 修正當index arm真空開啟，還沒建立真空的時候剛好發生ALARM，重新START會誤發Index arm pick up error
                           FTestSuck.Item[i][j]==NULL_IC && FTestSuck.Suck[i][j].GetOnBit())
                        {
                            FTestSuck.Suck[i][j].Normal();
                            CloseSuckok=false;
                        }

                        if(bArm2NeedSuck==false &&                              //JerryYang 20240816 : 修正當index arm真空開啟，還沒建立真空的時候剛好發生ALARM，重新START會誤發Index arm pick up error
                           BTestSuck.Item[i][j]==NULL_IC && BTestSuck.Suck[i][j].GetOnBit())
                        {
                            BTestSuck.Suck[i][j].Normal();
                            CloseSuckok=false;
                        }
                    }
                }
                else
                {
                    if(INDEX_SUCKER_TYPE==1 && bIndexCheckNoStopVaccum==false)  //Sam 20220902 : 修正 All Site 掉料
                    {
                        if(bArm1NeedSuck==false &&                              //JerryYang 20240816 : 修正當index arm真空開啟，還沒建立真空的時候剛好發生ALARM，重新START會誤發Index arm pick up error
                           FTestSuck.Item[i][j]==NULL_IC && FTestSuck.Suck[i][j].GetOnBit())
                        {
                            FTestSuck.Suck[i][j].Normal();
                            CloseSuckok=false;
                        }

                        if(bArm2NeedSuck==false &&                              //JerryYang 20240816 : 修正當index arm真空開啟，還沒建立真空的時候剛好發生ALARM，重新START會誤發Index arm pick up error
                           BTestSuck.Item[i][j]==NULL_IC && BTestSuck.Suck[i][j].GetOnBit())
                        {
                            BTestSuck.Suck[i][j].Normal();
                            CloseSuckok=false;
                        }
                    }
                }
            }
        }
        LastSet.iCatchArmVacuumDummyOnTime  =CatchTraySuck.Suck[0][0].VacuumOnTime;
        LastSet.iCatchArmVacuumDummyOffTime =CatchTraySuck.Suck[0][0].VacuumOffTime;
    }

    if(ArmSpeed_File[InArm].bAutoSKIP==1 && bASkStart==true)                    //kevin 20170606 (wei) tray end 燈亮
        bLampTrayEnd=true;

    if(SystemStart==false &&
       INDEX_SUCKER_TYPE==1 &&                                                  //jou 2010-05-24 start : 負壓
       W906_FormShowing("Zteach", Zteach->fShow)==false)                                                    //kevin 20210322 add  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批5：網頁沒有這個畫面（頁面表 noweb 列）⇒ 答案照舊是關，只是改走同一個函式 W906_FormShowing
    {
        CheckIndexAllSuckICFallDown(true, true);                                //Steven 20110725 : 修改負壓檢查方式

        //AI(W906-W7-L2) 20260803: golden :630-667 repeats golden :573-610 almost
        // verbatim -- the same two IniConfig.bChangeKitNoHardStop arms, the same
        // four `bArmNNeedSuck==false && Item==NULL_IC && GetOnBit()` tests, the
        // same Normal() + CloseSuckok=false bodies.  The ONLY textual difference
        // is that THIS copy carries a `//kevin 20110505` trailing comment on each
        // `CloseSuckok=false;` (:638, :645, :657, :664) and the first copy does
        // not (:581, :588, :600, :607).  What the two enclosing loops do NOT
        // share is the snapshot: :565-571 exists only in the PAUSE copy, so this
        // one is a pure vacuum sweep.  Both copies are reproduced as written and
        // deliberately NOT factored into a shared helper, which would be a
        // rewrite.  Note also the guard at :620-622 -- `SystemStart==false &&
        // INDEX_SUCKER_TYPE==1 && Zteach->fShow==false` -- so this sweep runs on
        // EVERY call while stopped, not only at the PAUSE edge.
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<NEW_MAX_Index_Col; j++)
            {
                if(IniConfig.bChangeKitNoHardStop==true)                        //jou 2015-12-08 Xilinx 驗證用
                {
                    if(INDEX_SUCKER_TYPE==1 && W906_FormShowing("fContact", fContact->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批1：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
                    {
                        if(bArm1NeedSuck==false &&                              //JerryYang 20240816 : 修正當index arm真空開啟，還沒建立真空的時候剛好發生ALARM，重新START會誤發Index arm pick up error
                           FTestSuck.Item[i][j]==NULL_IC && FTestSuck.Suck[i][j].GetOnBit())
                        {
                            FTestSuck.Suck[i][j].Normal();
                            CloseSuckok=false;                                  //kevin 20110505
                        }

                        if(bArm2NeedSuck==false &&                              //JerryYang 20240816 : 修正當index arm真空開啟，還沒建立真空的時候剛好發生ALARM，重新START會誤發Index arm pick up error
                           BTestSuck.Item[i][j]==NULL_IC && BTestSuck.Suck[i][j].GetOnBit())
                        {
                            BTestSuck.Suck[i][j].Normal();
                            CloseSuckok=false;                                  //kevin 20110505
                        }
                    }
                }
                else
                {
                    if(INDEX_SUCKER_TYPE==1 && bIndexCheckNoStopVaccum==false)  //Sam 20220902 : 修正 All Site 掉料
                    {
                        if(bArm1NeedSuck==false &&                              //JerryYang 20240816 : 修正當index arm真空開啟，還沒建立真空的時候剛好發生ALARM，重新START會誤發Index arm pick up error
                           FTestSuck.Item[i][j]==NULL_IC && FTestSuck.Suck[i][j].GetOnBit())
                        {
                            FTestSuck.Suck[i][j].Normal();
                            CloseSuckok=false;                                  //kevin 20110505
                        }

                        if(bArm2NeedSuck==false &&                              //JerryYang 20240816 : 修正當index arm真空開啟，還沒建立真空的時候剛好發生ALARM，重新START會誤發Index arm pick up error
                           BTestSuck.Item[i][j]==NULL_IC && BTestSuck.Suck[i][j].GetOnBit())
                        {
                            BTestSuck.Suck[i][j].Normal();
                            CloseSuckok=false;                                  //kevin 20110505
                        }
                    }
                }
            }
        }
    }

    //AI(W906-W7-L2) 20260803: the `&& INDEX_SUCKER_TYPE==1` re-test is redundant
    // in golden -- CloseSuckok can only have been cleared by one of the eight
    // GetOnBit() arms above, every one of which already required
    // INDEX_SUCKER_TYPE==1.  Kept as written.  Golden's 7-space indent on :674
    // (inside a 4-space body) is reproduced verbatim.
    if(CloseSuckok==false && INDEX_SUCKER_TYPE==1)                              //kevin 20110505
    {
       bIndexPlaceIcCheck=false;                                                //kevin 20110505 可以秀alarm
    }

    //AI(W906-W7-L2) 20260803: golden :677-684 and :686-689 are two consecutive
    // `if(SystemStart)` blocks rather than one -- and that is NOT redundancy: the
    // first block can clear SystemStart at :682, so the second correctly re-tests
    // it before latching CheckSystemPower.  Structure kept exactly as golden.
    // DEAD ABORT PATH -- DISCLOSED 20260803 BY INDEPENDENT AUDIT.  The code below is
    // byte-faithful to golden :677-684 and must NOT be changed.  But in THIS tree the
    // guard can never be true, so golden's abort never fires:
    //   * golden's SystemNG is not a plain flag, it is DERIVED from the alarm list, and
    //     it lives in the out-of-tree BCB6 package -- declared
    //     D:\HT9045\elec\Component\halarm.h:31 (`extern bool PACKAGE SystemNG;`),
    //     storage HAlarm.cpp:14.
    //   * golden's ONLY producer of SystemNG==true is HAlarm::Set's UpdateSystemNG()
    //     call at elec/Component/HAlarm.cpp:136.  That producer has no ported
    //     equivalent, so nothing in this tree can ever set it true.
    //   * ported ckernel.cpp only ever CLEARS it (:823, the golden :376 store).
    // CONSEQUENCE: a real machine would StopAllMotor() and drop SystemStart here when
    // the alarm list says the system is NG; this tree silently continues.  That is a
    // MISSING SAFETY ABORT, not a cosmetic gap.  ACTION for the integrator: port
    // UpdateSystemNG (or an equivalent alarm-list-derived producer) before this tree
    // is used to drive anything physical.  Tracked in docs/MIGRATION_ROADMAP.md DEFERRED.
    if(SystemStart)
    {
        if(SystemNG)
        {
            StopAllMotor();                                                     // alarm by alarm message
            SystemStart=false;
        }
    }

    if(SystemStart)
    {
        CheckSystemPower=true;
    }
    return SystemStart;
}
//AI(W906-W7-L2) 20260803: golden :692-698 -- six file-scope externs sitting in
// the seam between ScanSystemSensor (ends :691) and ShowRunLed (starts :704).
// Carried over verbatim and left UNDEFINED on purpose: their ONLY golden uses
// are the NINE lines :792, :797, :844, :847, :849, :854, :856, :861 and :863 --
// every one of them inside ShowRunLed (:704-932), which this wave DEFERS to
// W7-U -- so nothing in the ported tree references them and no link edge is
// created.  (Corrected this pass: the recon that preceded this wave listed only
// five of the nine, omitting :847, :849, :861 and :863.)  Dropping the externs
// would silently lose interface that W7-U needs.
// Golden's blank line at :694 and the banner at :699-703 -- including the two
// commented-out Amkor externs -- follow verbatim.
bool SECS_GEM_PPMUSIC_CONTROL_flag=false;  //AI(W906-MSTATE-P2b) 20260924: extern 改成定義（ShowRunLed 解閘後有使用者了；上面那段「故意不定義」的前提因此結束）——golden 的家是沒翻的 SECSGEM/UsecegemMainFrom.cpp:458，初值照原文；寫者（host 設 EC）在移植樹沒有連進來，所以值維持 golden 初值
int  iSECS_GEM_PPMUSIC_CONTROL_CLASS=0;  //AI(W906-MSTATE-P2b) 20260924: extern 改成定義（ShowRunLed 解閘後有使用者了；上面那段「故意不定義」的前提因此結束）——golden 的家是沒翻的 SECSGEM/UsecegemMainFrom.cpp:459，初值照原文；寫者（host 設 EC）在移植樹沒有連進來，所以值維持 golden 初值

bool SECS_GEM_PPSIGNALTOWER_CONTROL_flag=false;  //AI(W906-MSTATE-P2b) 20260924: extern 改成定義（ShowRunLed 解閘後有使用者了；上面那段「故意不定義」的前提因此結束）——golden 的家是沒翻的 SECSGEM/UsecegemMainFrom.cpp:461，初值照原文；寫者（host 設 EC）在移植樹沒有連進來，所以值維持 golden 初值
int  iSECS_GEM_PPSIGNALTOWER_CONTROL_RED=0;  //AI(W906-MSTATE-P2b) 20260924: extern 改成定義（ShowRunLed 解閘後有使用者了；上面那段「故意不定義」的前提因此結束）——golden 的家是沒翻的 SECSGEM/UsecegemMainFrom.cpp:462，初值照原文；寫者（host 設 EC）在移植樹沒有連進來，所以值維持 golden 初值
int  iSECS_GEM_PPSIGNALTOWER_CONTROL_GREEN=0;  //AI(W906-MSTATE-P2b) 20260924: extern 改成定義（ShowRunLed 解閘後有使用者了；上面那段「故意不定義」的前提因此結束）——golden 的家是沒翻的 SECSGEM/UsecegemMainFrom.cpp:463，初值照原文；寫者（host 設 EC）在移植樹沒有連進來，所以值維持 golden 初值
int  iSECS_GEM_PPSIGNALTOWER_CONTROL_YELLOW=0;  //AI(W906-MSTATE-P2b) 20260924: extern 改成定義（ShowRunLed 解閘後有使用者了；上面那段「故意不定義」的前提因此結束）——golden 的家是沒翻的 SECSGEM/UsecegemMainFrom.cpp:464，初值照原文；寫者（host 設 EC）在移植樹沒有連進來，所以值維持 golden 初值
//==============================================================================
// 處理三色燈
//==============================================================================
//extern AnsiString sAmkorAlarmMessage;
//extern AnsiString sAmkorAlarmBuzzer;
// =============================================================================
//  W7-L2 FRAGMENT F3 -- golden ckernel.cpp:704-1727
//  ShowRunLed (golden :704-932) + ShowRunLabel (golden :935-1726)
//  AI(W906-W7-L2) 20260803 -- BOTH DEFERRED TO W7-U.  NOT TRANSLATED.
//
//  READ THIS BEFORE ASSUMING ANY BEHAVIOUR.  Nothing in this fragment paints a
//  lamp, sets a caption, reads a sensor or evaluates a single golden condition.
//  The two functions below are linkable STUBS whose entire observable effect is
//  to increment a call counter.  They are labelled as such at every level: the
//  banner, the function comment, the counter name, and an `#if 0 // TODO(W7-U)`
//  gate that names the exact golden span that is missing.  Any caller that
//  behaves as though the tower light or the run label has been painted is
//  wrong; no painting happens on this front.
//
// -----------------------------------------------------------------------------
//  WHY DEFERRED -- MEASURED, NOT ASSERTED
// -----------------------------------------------------------------------------
//  Both functions are OVERWHELMINGLY VCL form painting -- but NOT "nothing else":
//  see the CORRECTED blast-radius list below, which catalogues the state and the
//  12 SW[] physical outputs they also write.  Counted this pass
//  over the cp950-decoded golden, LIVE lines only (a line whose first non-space
//  characters are `//` is excluded from the live counts):
//
//  ShowRunLed  golden :704-932  -- 229 lines total, 214 live, 15 comment-only
//      fMain->            31 derefs / 6 distinct members:
//                         ledRed(9) ledGreen(9) ledYellow(9) pnlSafePLC(2)
//                         ALed1(1) ledSafePLC(1)
//      fNote->            20 derefs / 3 distinct members:
//                         edErrorCode(18) fShow(1) AlarmType(1)
//      fTowerLight->fShow(1)   fShowBinSet->fShow(1)   fHome->fShow(1)
//      MyMessageBox->fShow(1)
//      CCDInterfaceForm->  bIdentificationFinish(1) iIdentificationStatus(1)
//      SW[] writes        12  (SwMusic1+0..3, SwTowerRed/Yellow/Green,
//                              SwFKAlarmReset / SwRKAlarmReset)
//      Sen[] reads         0
//      => 15 distinct object->member pairs across 7 objects.
//
//  ShowRunLabel golden :935-1726 -- 792 lines total, 759 live, 33 comment-only
//      fMain->           150 derefs / 7 distinct members:
//                        ShowNowStatus(88) labDelayStatus(40)
//                        EnabledSetupFile(7) ARTCombine(6) labAutoClean(5)
//                        labQAMode(3) CheckCanChangeRealDummy(1)
//      fLotInfo->         16 derefs / 3 members: btnFtpServer(8) btnFtpHD(6)
//                         tsFTP(2)
//      fContact->IsRun2DCheck(2)  fFTPClient->bShow(2)  fAutoTeach->GetState(1)
//      fHome->fShow(1)  fNote->fShow(1)  MyMessageBox->fShow(1)
//      Sen[] reads         6  (SnFrontLeftEMG SnFrontRightEMG SnRearLeftEMG
//                              SnRearRightEMG SnAllEMG SnMotorPower)
//      SW[] writes         0
//      => 16 distinct object->member pairs across 8 objects.
//
//  Translating these now would mean inventing widget stand-ins (a tri-state
//  `Value` LED, a `ShowNowStatus(TColor,AnsiString)` painter, a TTabSheet
//  `Enabled`, an ARTCombine caption panel ...).  That is still the right call,
//  but the ORIGINAL rationale here was WRONG and is corrected below.
//
//  CORRECTED 20260803 BY INDEPENDENT AUDIT.  This paragraph used to say the pair
//  had "ZERO state-machine behaviour" and that every state variable they read --
//  "RunState, LED_*, FlushFlag, iHeaterWaitTime, iSoakTimer, iBoostFuncStep,
//  bHALTing -- is written elsewhere; these two only render it."  NOT TRUE.  These
//  two functions WRITE state, and deferring them LOSES those writes:
//    ShowRunLed   -- RunState, bAlarmBuzzer (golden :740/:742/:774/:794/:846),
//                    bNeedMusicFinishLighAndAlarmOn (:775), bLampAlarmReset (:897),
//                    and 12 SW[] PHYSICAL OUTPUTS (SwMusic1+0..3, SwTowerRed/
//                    Yellow/Green, SwFKAlarmReset, SwRKAlarmReset).
//    ShowRunLabel -- bHALTing (golden :1533/:1608/:1621/:1627).  This is the
//                    SOLE WRITER OF bHALTing IN THE ENTIRE TREE (verified by
//                    grepping every assignment across the golden version dir;
//                    storage is cmydef.cpp:3534).  Its consumer is golden
//                    csystem.cpp:20534, which gates the chamber FAST-COOL
//                    hardware path (:20536-20538 fAirCon->PowerUp(5.5);
//                    SW[SwCoolingFan_Blower].On(); Cylinder[C_CoolingValve].Off()).
//                    So while this stub stands, bHALTing can never become true and
//                    that hardware branch is UNREACHABLE -- a state-machine
//                    consequence, not a painting one.
//                    Also lost: bSendChangeTempDelayAlarm (:957), RecordProcess
//                    (:958/:1389/:1402), bInitialTestDelayStatus[] cleared in a
//                    loop (:1324), iTemperatureOk (:1388), bSECSGEMAlarm (:1546),
//                    tUPH_PauseStartTime + bCalculatePauseTime (:1571-1572).
//  W7-U MUST treat this list as the deferral's real blast radius.
//
//  BRIEF FIGURES RE-MEASURED -- TWO CONFIRMED, ONE WRONG.
//    CONFIRMED  "ShowRunLed derefs fMain-> 31 times and fNote-> 20 times" --
//               exact for live code.  Raw text count for fMain-> is 32; the
//               extra is golden :889, a commented-out line
//               (`//if(RunState==LED_Running && fMain->ALed1->Value==false)`).
//    CONFIRMED  "ShowRunLabel derefs fMain-> 150 times" -- exact for live code.
//               Raw text count is 151; the extra is golden :1113, inside the
//               commented-out block golden :1111-1114.
//    WRONG      "across 14 widget families".  ShowRunLabel touches SEVEN
//               distinct fMain-> members, or SIXTEEN distinct object->member
//               pairs if every form object is counted.  Neither is 14.
//               Recorded here rather than argued with.
//
// -----------------------------------------------------------------------------
//  CHINESE COMMENTS -- WHERE THEY WENT
// -----------------------------------------------------------------------------
//  The deferred span golden :704-1726 carries 86 CJK-bearing lines (counted by
//  scanning the cp950-decoded lines for any codepoint above U+2E80).  Because
//  the BODIES are not emitted, those 86 comments travel with W7-U, not with
//  this fragment -- they are still in golden, unmodified, and W7-U must decode
//  them with cp950 and land them as real UTF-8.  They are NOT dropped and NOT
//  machine-retranslated.  The handful quoted in the GOLDEN ODDITIES section
//  below are reproduced verbatim as UTF-8 so the knowledge they carry survives
//  even if the golden tree moves.
//
// -----------------------------------------------------------------------------
//  GOLDEN ODDITIES IN THE DEFERRED SPAN -- W7-U MUST REPRODUCE ALL OF THESE
// -----------------------------------------------------------------------------
//  Every line number below was opened and read this pass with cp950.  These are
//  recorded, not fixed; this fragment preserves them by carrying them forward
//  in writing, since it emits no body to preserve them in.
//
//  [G1] golden :891 -- MIXED && / || WITH NO PARENTHESES.
//       `if(RunState==LED_Running && fMain->ALed1->Value==false || bTesterDuckingFinishLightYellowAndAlarmOn)`
//       C++ precedence makes this `(A && B) || C`, so the Ducking flag alone
//       forces the yellow blink regardless of RunState.  Golden :889 is the
//       commented-out predecessor -- `//if(RunState==LED_Running && fMain->ALed1->Value==false)`
//       -- which shows `|| C` was BOLTED ON to an existing conjunction.  Emit
//       it unparenthesised exactly as golden has it.
//
//  [G2] golden :1564-1567 -- THE SAME SHAPE, AND THE INDENTATION LIES.
//           else if(fMain->CheckCanChangeRealDummy()==false ||
//                   HasICUnderMachine() &&
//                   HasAnyICInMachine() &&
//                   CUSTOMER_CODE==CC_ASE_KaohSiung)
//       Parses as `A || (B && C && D)`, but B/C/D are indented flush under A as
//       though the whole thing were one conjunction gated by the ASE customer
//       code.  It is not: on every machine, `CheckCanChangeRealDummy()==false`
//       alone reaches the PAUSE arm.  Preserve the parse, keep the indentation.
//
//  [G3] golden :1239-1244 IS A DEAD GUARD.  It gates
//       `fMain->ShowNowStatus(clLime, AnsiString(iHeaterWaitTime))` behind four
//       terms (`iOneCycle==0 && bRunAutoClean==false &&
//       bIsAutoOneCycleAutoclean==false && iHeaterWaitTime!=0`).  Golden :1246
//       then re-runs the IDENTICAL ShowNowStatus call under `iHeaterWaitTime!=0`
//       alone -- a strict superset of that guard -- so :1239-1244 has no
//       observable effect on the status text.  Keep it; deleting it would be a
//       tidy-up of golden.  Golden :1239's own comment is
//       `//kevin 20130220 add iOneCycle 時會秀sacktime`  (see [G12]).
//
//  [G4] golden :941-942 -- HALF OF A TWO-SLOT ARRAY PAIR IS DEAD.
//       `static int iTempStart[2]={0,0};` / `static int iTempFinish[2]={0,0};`
//       Scanning all 2589 golden lines: iTempStart occurs ONLY at :941, :1361,
//       :1362, :1381, :1382, :1390, :1391, :1394, :1399 -- slot [1] is written
//       at :1362/:1382/:1391 and NEVER read.  iTempFinish occurs ONLY at :942,
//       :1396, :1399, :1401 -- slot [1] is never touched after the initialiser.
//       Only slot [0] participates in the "Working Temperature Arrived Done"
//       latch.  Keep both arrays at size 2 and keep the writes to [1].
//
//  [G5] golden :1597-1600 vs :1609 -- AN ACCESS-LEVEL GATE THAT IS IMMEDIATELY
//       OVERWRITTEN.  :1597-1600 sets `fLotInfo->btnFtpHD->Enabled` from
//       `IniConfig.iHDEnable<=AccessLevel`; nine lines later, OUTSIDE that
//       if/else and on every path through the branch, :1609 does
//       `fLotInfo->btnFtpHD->Enabled=true;` with the comment
//       `//wei 20160314 不需要onecycle or Cleanout就可上傳`.
//       Net effect: in this branch the HD button is always enabled and the
//       access level is ignored.  Reproduce both, in order.
//
//  [G6] golden :1200-1223 vs :1471-1509 -- TWO ART SWITCHES ON THE SAME
//       VARIABLE THAT DISAGREE.  Both switch on `LastSet.iAutoRetestCount_ART`.
//       The heater-OK path (:1200) handles cases 0-5 and falls through to
//       `default: fMain->ShowNowStatus(clLime, "Running");` (:1220-1222).
//       The not-hot path (:1471) handles cases 0-10 and falls through to
//       `default: fMain->ShowNowStatus(clLime, "ART Over");` (:1506-1508).
//       So iAutoRetestCount_ART==6 paints "Running" on one path and "RT6 ART"
//       on the other.  Do not harmonise them.
//
//  [G7] golden MISSPELLING `bTesterDucking` / `iTesterDucking` /
//       `bTesterDuckingFinishLightYellowAndAlarmOn` -- "Ducking" where
//       "Docking" is meant.  Proof, not inference: the declaration at golden
//       cmydef.h:3568 reads
//       `extern bool bTesterDucking;   //jou 2014-04-25 Tester Docking Wait delay time`
//       and golden ckernel.cpp:1268 uses the CORRECTLY spelled
//       `IniConfig.bA05UseAutoDocking` and `Temperature.bUseTesterDocking` on
//       the SAME LINE as the misspelled `bTesterDucking`.  These are global
//       names, i.e. API -- the typo is load-bearing and must be kept.
//       Occurrences in this span: :771, :891, :1268, :1270.  (:1270 was missing
//       from an earlier draft of this list and was added after audit -- it is
//       `fMain->ShowNowStatus((TColor) 0x000080FF, AnsiString(iTesterDucking));`,
//       the BODY of the very branch :1268 opens.  Since this catalogue is the
//       only artefact W7-U will work from, an incomplete list is how the typo
//       gets silently "corrected" on the line nobody listed.)
//
//  [G8] golden MISSPELLING `Temperature.bBoostFuncttion` (double t), declared
//       golden cprod.h:1495 (`bool bBoostFuncttion;`).  Used six times in this
//       span -- :1256, :1262, :1329, :1335, :1367, :1373 -- every one of them
//       next to the correctly spelled `bLBTempFunction`.  Keep the typo.
//
//  [G9] golden :1677-1679 AND :1687-1689 -- EMPTY BRANCHES THAT DO REAL WORK BY
//       BEING EMPTY.  `else if(TestIF_File.bRENESAS_EnableFTCT==true) { }`
//       (:1677-1679) exists solely to SUPPRESS the
//       `fMain->EnabledSetupFile(true)` in the trailing else (:1680-1683).
//       `if(AUTO3_IS_MAGAZINE==1 && iMagazineStatus!=0) { }` (:1687-1689) does
//       the same for the labDelayStatus clear at :1692-1693.  Deleting either
//       "dead" arm changes behaviour.
//
// [G10] golden :706 `static bool OldFlushFlag;` (no initialiser) vs golden :938
//       `static bool OldFlushFlag=false;` -- the same latch, the same purpose,
//       spelled two different ways in two adjacent functions.  Both are false
//       on first entry (function-local statics are zero-initialised), so this
//       is an inconsistency and not a defect -- and it must NOT be harmonised.
//
// [G11] golden :1009 -- the trailing comment on `EmgStr="Servo Off";` reads
//       `//kevin 20140121 偵測sevon 訊號`.  "sevon" is a typo for "servo",
//       inside a Chinese comment.  Carry the comment through byte-for-byte
//       (as UTF-8), typo included.
//
// [G12] golden :1239 -- `//kevin 20130220 add iOneCycle 時會秀sacktime`.
//       "sacktime" is a typo for "soaktime".  Same rule as [G11].
//
// [G13] golden :1041 and :1045 carry the IDENTICAL trailing comment
//       `//jou 2012-05-03 增加QA mode使用獨立的Loader Count`, although :1041
//       formats `iQAModePassCT` and only :1045 formats `iQAModeLoaderCT`.  The
//       comment is copy-pasted onto a branch it does not describe.  Keep both.
//
// [G14] golden :1720-1725 -- the fAutoTeach "Auto Alignment Mode" arm ends with
//       `return;` and :1726 is the function's closing brace, so the return is
//       unreachable-as-control-flow (it returns from the last statement).
//       Harmless; keep it.
//
// -----------------------------------------------------------------------------
//  WHAT W7-U MUST RESTORE ALONGSIDE THE BODIES
// -----------------------------------------------------------------------------
//  The F0 prologue DROPPED SEVEN include groups because their ONLY users are
//  these two deferred functions -- not because the symbols are unavailable.
//  W7-U must bring all seven back:
//      golden :22  cTowerLight.h        fTowerLight + RunState / LED_*
//      golden :26  uLotInfo.h           fLotInfo (all 16 derefs).  The three
//                                       widgets are golden uLotInfo.h:28
//                                       (`TTabSheet *tsFTP;`), :289
//                                       (`TButton *btnFtpServer;`) and :290
//                                       (`TButton *btnFtpHD;`).  NONE of the
//                                       three exists on the ported stand-in
//                                       forms/fLotInfo.h -- grepped this pass,
//                                       zero hits.  W7-U must land them.
//      golden :27  CCDInterface.h       CCDInterfaceForm (:828-829)
//      golden :28  FTPClient.h          fFTPClient (:1589, :1636)
//      golden :38  cShowBinSet.h        fShowBinSet (:729)
//      golden :41  AutoTeach.h          fAutoTeach (:1720)
//      golden :44/:45 MyPLC_IO_Modbus.h + ModbusTCPClient.h
//                                       Enable_PLCSafety_IO (:921, :1006)
//
//  PROLOGUE OFF-BY-ONE, RECORDED NOT FIXED: the F0 prologue's own closing recap
//  says "the six deferred-only groups above (fLotInfo, CCD, FTP, tower light,
//  bin-set form, PLC safety)" and omits AutoTeach -- even though the same file
//  drops golden :41 for exactly the deferred-only reason.  The restore list is
//  SEVEN groups.  F0_prologue.inc belongs to the prologue agent, so this
//  fragment does not edit it; flagged for the integrator instead.
//
// =============================================================================

// -----------------------------------------------------------------------------
//  W906 TEST SEAM -- STORAGE.  AI(W906-W7-L2) 20260803.  NOT IN GOLDEN.
//
//  Declared in ckernel.h:134-136 (already landed by the prologue agent; this
//  fragment only supplies the definitions, so there is NO substrate gap here).
//
//  WHY THEY EXIST.  DoSystemMessage (golden :1899-1915, a neighbour's range and
//  NOT deferred) is pure scheduling: `if(iMyCounter==0) { ShowRunLed(); (:1905)
//  ShowRunLabel(); (:1906) } else if(iMyCounter==3) { DoPanelLamp(); (:1910) }`
//  with a 0..5 wrap.  If the two stubs were silently empty, a faithful
//  DoSystemMessage and one that never dispatches at all would be
//  indistinguishable -- the exact unfalsifiable shape this project has already
//  paid for.  A counter per stub makes the 1-in-6 duty cycle testable while the
//  painting stays unwritten.
//
//  THESE ARE CALL COUNTERS, NOT DISPLAY STATE.  A nonzero count means "the
//  scheduler dispatched", never "the tower light was painted".
//
//  Naming follows this tree's seam idiom -- `W906_<subject>_<field>` plus one
//  reset -- as in canary_support.h:303-311 (W906_ShowErrorMessage_SimReturn /
//  _LastCode / _LastKCode / _Count / _Reset) and aHotPlateSubstrate.h:356-358
//  (W906_TMySucker_OffDestroyEpoch / _Total / _ResetAll, defined at
//  aHotPlateSubstrate.cpp:66-75).  ONE reset clears the pair, per the
//  _ResetAll precedent, so a test can isolate a single tick.
//
//  These three definitions retire when W7-U lands the real bodies.
// -----------------------------------------------------------------------------
int W906_ShowRunLed_Count   = 0;
int W906_ShowRunLabel_Count = 0;

void W906_ShowRunLedLabel_ResetCounts()
{
    W906_ShowRunLed_Count   = 0;
    W906_ShowRunLabel_Count = 0;
}

// ===========================================================================
//  GOLDEN VERBATIM PAIR -- ShowRunLed
//  --------------------------------
//  The block below is golden ckernel.cpp:704-932 (229 golden lines)
//  transcribed CHARACTER-FOR-CHARACTER (Big5/cp950 -> UTF-8 only) and GATED
//  OFF.  It is INERT: the ACTIVE ShowRunLed is the counter-bump DEFERRED stub
//  below (void ShowRunLed() at preimage :1579), which this pair does NOT touch.
//
//  WHY: the census scored ShowRunLed "translated" because a same-named live body
//  exists; it never compared SIZE.  That live body is a 38-line DEFERRED
//  STUB (W906-W7-L2, 20260803) whose own #if 0 held a PROSE SUMMARY of
//  golden, not golden.  These 229 lines of golden text existed NOWHERE in
//  this tree.  Now they do, auditable line-by-line, so the W7-U un-gate is
//  mechanical rather than a re-translation.  NET BEHAVIOUR CHANGE: ZERO.
//
//  NOTHING inside the gate is fixed, renamed, reflowed or reindented --
//  golden's own defects are preserved on purpose so a diff against golden
//  stays empty.  Preserved defects in this span:
//    golden :891  RunState==LED_Running && fMain->ALed1->Value==false || bTesterDucking...
//                 -- mixed &&/|| with NO parentheses, so it means (A&&B)||C.  NOT parenthesised.
//    golden :797  SW[SwMusic1+iSECS_GEM_PPMUSIC_CONTROL_CLASS-1].On() -- a CLASS of 0 indexes
//                 SwMusic1-1, one slot BELOW the group.  No range guard.  NOT added.
//    golden :838  the FlushFlag latch sits MID-BODY: the music/buzzer group above it runs
//                 every tick, the whole LED group below it only on a FlushFlag edge.
//
//  Same shape as this tree's existing pairs: csystem.cpp MainProc / DoAllProcess
//  / DoTrayFeedProcess / CheckContinusStartIsReady, plus the 26 pairs PT-W6b
//  landed in atester.cpp.  Being gated it needs NO callee to exist -- only
//  lexical validity.  Do NOT add stubs or declarations for its symbols; the
//  un-gate blockers are listed in the wave report, not papered over here.
// ===========================================================================
//AI(W906-MSTATE-P2b) 20260924: 解閘 —— 此行原本是 `#if 0 // GOLDEN VERBATIM -- golden ckernel.cpp:704-932 (229 lines).  GATE G-PTm1-ShowRunLed`。與 ShowRunLabel（:1940）同一個做法；對外行為變更（塔燈／蜂鳴器等 12 個 SW[] 實體輸出）列 docs/ChangeLog.md
void ShowRunLed()
{   ++W906_ShowRunLed_Count;   //AI(W906-MSTATE-P2b) 20260924: 原本空殼（:1841）唯一的效果，保留給 tests/test_w7_l2_ckernel.cpp 的 DoSystemMessage 排程斷言；接在同一行，不移動行號
    static bool OldFlushFlag;
    int is, i;
    if(W906_FormShowing("fNote", fNote->fShow) || IsEMGPressed())      //JerryYang 20260415 : 按EMG也要顯示紅燈  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
    {
        if(CosFunction.bTowerLightUseLD &&  //JerryYang 20230721 : LD/ULD區分不同的異常音樂
          (fNote->edErrorCode->Text=="MES0920" || fNote->edErrorCode->Text=="MES0921" || fNote->edErrorCode->Text=="MES0922" || fNote->edErrorCode->Text=="MES0923" ||
           fNote->edErrorCode->Text=="MES1021" || fNote->edErrorCode->Text=="MES1421" ||
           fNote->edErrorCode->Text=="MES1721" || fNote->edErrorCode->Text=="MES1821" || fNote->edErrorCode->Text=="MES1921" ||     //Steven 20230907 : For HT-9011UC
           fNote->edErrorCode->Text=="MES2821" || fNote->edErrorCode->Text=="MES2921" || fNote->edErrorCode->Text=="MES3021" ||
           fNote->edErrorCode->Text=="MES1120" || fNote->edErrorCode->Text=="MES1220" || fNote->edErrorCode->Text=="MES1320" ||
           fNote->edErrorCode->Text=="MES2520" || fNote->edErrorCode->Text=="MES2620" || fNote->edErrorCode->Text=="MES2720"))
        {
            RunState=LED_AutoRetest;
        }
        else if(fNote->AlarmType==3)    //Steven 20120203 : MES的Code用Message的音樂
        {
            RunState=LED_Message;
        }
        else
        {
            RunState=LED_ErrJam;
        }
    }
    else if(W906_FormShowing("MyMessageBox", MyMessageBox->fShow) || W906_FormShowing("fShowBinSet", fShowBinSet->fShow))  //JerryYang 20190523 Microchip要求跳出site map確認視窗時蜂鳴器要叫  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
    {
//        if(iUnLoaderCount)      //Jou 20150721 : 重新啟用功能    //JerryYang 20160811 Mark掉,使用Message的設定
//            RunState=LED_TrayCounter;
//        else
            RunState=LED_Message;
    }
    else if(bTesterSendPause)   //Steven 20201022 : For RFMD
    {
        RunState=LED_Message;  if(bPauseAlarmDelayActive && hPauseAlarmDelay.Off()) { bTesterPauseMusic=true; bPauseAlarmDelayActive=false; }   /* AI(W906-GB-P2c) 20260926: golden 912 ckernel.cpp:744-748 (RogerYang 20260626: Tester Pause sounds only after MaxTestTime -- set the buzzer when the delay expires, latch once); user todo "②", appended on this line so no line moves */
        if(bTesterPauseMusic)   //Steven 20220616 : Can select "Alarm Reset" when show "Tester Pause" for QORVO.
            bAlarmBuzzer=true;
        else
            bAlarmBuzzer=false;
    }
    else if(fHome->fShow)   //AI(W906-FSHOW-B2) 20260929: 維持讀程式狀態（R142＝A：golden 的 Home Monitor 開著＝正在回原點，網頁自己開的不算）
    {
        RunState=LED_Homeing;
    }
    else if(bReplyFTCTAlarm==true ||                                            //RogerYang 20251021 : 瑞薩FT-CT FTCT發送違法指令回傳Error需要報警
            bContinueMessageByReply71==true)                                    //RogerYang 20251107 : 瑞薩FT-CT Add Continue Form
    {
        RunState=LED_Message;
    }
    else if(SystemStart && (LastSet.iRunStartMode==rsmAutoRetest || bAutoRetestMusic))   //Steven 20140409 : Auto Retest
    {
        RunState=LED_AutoRetest;
    }
#ifndef DEBUG_SPIL
    else if(SystemStart && LastSet.iTester==OFF_LINE &&
            OFFLINE_ALARM)                                                      //Steven 20140805
    {
        RunState=LED_OfflineRun;
    }
#endif
    else if(SystemStart)
    {
        if(LastSet.iTemperature!=Tempture_Ambient &&
           fHeaterOK==false)
        {
            RunState=LED_Heating;
        }
        else if(bTesterDuckingFinishLightYellowAndAlarmOn || bNeedMusicAndAlarmOn) //kevin 20170817 (Steven) add music alarm
        {
            RunState=LED_Message;
            bAlarmBuzzer=true;
            bNeedMusicFinishLighAndAlarmOn=true;                                //kevin 20170817 (Steven) add music alarm
        }
        else if(bN07AlarmActive) { RunState=LED_Message; if(!bN07BuzzerSilenced) bAlarmBuzzer=true; }  else   //AI(W906-C15-N07) 20261003 (St02-E, claim): golden 906_0625_Steven ckernel.cpp:777-781 (Steven 20260603 : Secs_Gem disconnect alarm (JSCC NetworkMonitor) / respect Alarm Reset); flags SECSGEM/N07Alarm_St02.cpp; same line
        {
            RunState=LED_Running;
        }
    }
    else
    {
        if(IniConfig.bG14UseStartSoundAlarm && bStartMoveSpeed)                //kevin 20201116  Start 發出聲音 不動 5sec
            RunState=LED_Message;
        else
            RunState=LED_Pause;
    }
    // 處理音樂
    if(W906_FormShowing("fTowerLight", fTowerLight->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批1：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
    {
        if(IniConfig.bEnable_SECS_GEM==true && SECS_GEM_PPMUSIC_CONTROL_flag==true)       //Steven 20150605 : Add SECS GEM控制蜂鳴器
        {
            bAlarmBuzzer=true;
            for(i=0; i<4; i++)
                SW[SwMusic1+i].Off();
            SW[SwMusic1+iSECS_GEM_PPMUSIC_CONTROL_CLASS-1].On();
        }
        else
        {
            is=LastSet.MusicSelect[RunState];
            if(is)
            {
                is--;
                for(i=0; i<4; i++)
                {
                    if(is==i)
                    {
                        if((RunState==LED_Message || RunState==LED_ErrJam) && bAlarmBuzzer==false)  //Steven 20111030 : 少打一個等於
                            SW[SwMusic1+i].Off();
                        else
                            SW[SwMusic1+i].On();
                    }
                    else
                    {
                        SW[SwMusic1+i].Off();
                    }
                }
            }
            else
            {
                for(i=0; i<4; i++)
                    SW[SwMusic1+i].Off();
            }
#if 0 // SAFETY-GATE(W906-MSTATE-CCDID) golden ckernel.cpp:826-833（此行原本是 `//Eliot 2010_1206 start`）—— 缺相依：CCDInterfaceForm 在移植樹是 TCCDInterfaceFormShim（atester_shims.h:311），沒有 bIdentificationFinish／iIdentificationStatus（CCD TCP 辨識結果）；補欄位等於編造一個值。外層旗標 IniConfig.bEnableCCDUSETCPIP 屬於沒載入的 config.ini 半邊（T4 U70），今天恆 0，閘住與現況等價
            if(IniConfig.bEnableCCDUSETCPIP)
            {
                if(CCDInterfaceForm->bIdentificationFinish==true &&
                   CCDInterfaceForm->iIdentificationStatus==2)//0:未測試 1:Pass 2:Fail
                {
                    SW[SwMusic1].On();
                }
            }
#endif // SAFETY-GATE(W906-MSTATE-CCDID)（此行原本是 `//Eliot 2010_1206 end`）
        }
    }

    if(OldFlushFlag==FlushFlag)
    {
        return;
    }
    OldFlushFlag=FlushFlag;

    if(IniConfig.bEnable_SECS_GEM==true && SECS_GEM_PPSIGNALTOWER_CONTROL_flag==true)
    {
        bAlarmBuzzer=true;
        if(iSECS_GEM_PPSIGNALTOWER_CONTROL_RED==0)
            fMain->ledRed->Value=false;
        else if(iSECS_GEM_PPSIGNALTOWER_CONTROL_RED==1)
            fMain->ledRed->Value=true;
        else
            fMain->ledRed->Value=FlushFlag;

        if(iSECS_GEM_PPSIGNALTOWER_CONTROL_GREEN==0)
            fMain->ledGreen->Value=false;
        else if(iSECS_GEM_PPSIGNALTOWER_CONTROL_GREEN==1)
            fMain->ledGreen->Value=true;
        else
            fMain->ledGreen->Value=FlushFlag;

        if(iSECS_GEM_PPSIGNALTOWER_CONTROL_YELLOW==0)
            fMain->ledYellow->Value=false;
        else if(iSECS_GEM_PPSIGNALTOWER_CONTROL_YELLOW==1)
            fMain->ledYellow->Value=true;
        else
            fMain->ledYellow->Value=FlushFlag;
    }
    else
    {  if(bN07AlarmActive) { fMain->ledRed->Value=FlushFlag; fMain->ledGreen->Value=false; fMain->ledYellow->Value=false; } else {   //AI(W906-C15-N07) 20261003 (St02-E, claim): golden 906_0625_Steven ckernel.cpp:875-882 (Steven 20260605 : SECS disconnect -> blink red (JSCC)); this else closes on :1813 (golden :926); same line
        //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
        if(IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain && bLoaderNoTrayAutoCleanOut && RunState==LED_Running)
        {
            fMain->ledGreen->Value=FlushFlag;
        }
        else
        {
            if(LastSet.MessageLight[RunState][0]==1)        fMain->ledGreen->Value=true;
            else if(LastSet.MessageLight[RunState][0]==2)   fMain->ledGreen->Value=FlushFlag;
            else                                            fMain->ledGreen->Value=false;
        }

        if(LastSet.MessageLight[RunState][1]==1)        fMain->ledYellow->Value=true;
        else if(LastSet.MessageLight[RunState][1]==2)   fMain->ledYellow->Value=FlushFlag;
        else                                            fMain->ledYellow->Value=false;
        if(LastSet.MessageLight[RunState][2]==1)        fMain->ledRed->Value=true;
        else if(LastSet.MessageLight[RunState][2]==2)   fMain->ledRed->Value=FlushFlag;
        else                                            fMain->ledRed->Value=false;

        //if(RunState==LED_Running && fMain->ALed1->Value==false)
        //ChungHung 20141015 add for SCK When the handler start running (finished Start count function),Yellow light blinking & alarm on (press "alarm reset" then clear)
        if(RunState==LED_Running && fMain->ALed1->Value==false || bTesterDuckingFinishLightYellowAndAlarmOn)
            fMain->ledYellow->Value=FlushFlag;

        if(bNeedMusicAndAlarmOn)//kevin 20170816 (Steven) 發出音樂及閃燈
        {
            fMain->ledRed->Value=FlushFlag;
            bLampAlarmReset=true;
            if(bFrontPadActive) SW[SwFKAlarmReset].On();
            else                SW[SwRKAlarmReset].On();
        }

        //jou 2014-12-19 Temperature Less 30 deg. Show Light
        //jou 20180529 : Temperature Heater Ok Show Light
        if(CosFunction.bTempLess30degShowLight || CosFunction.bTempHeaterOkShowLight)
        {
            if(bTempLess30degShowLightFlag==true || bTempHeaterOkShowLightFlag==true)
            {
                fMain->ledGreen->Value=FlushFlag;
                fMain->ledYellow->Value=FlushFlag;
                fMain->ledRed->Value=FlushFlag;
            }
        }
    }  }   //AI(W906-C15-N07) 20261003 (St02-E, claim): the first brace closes the N07 else opened on :1769 (golden 906_0625_Steven ckernel.cpp:926 "bN07AlarmActive else"), the second is this line's own (golden :927 "bEnable_SECS_GEM PPSIGNALTOWER else"); same line

    SW[SwTowerRed].OnOff(fMain->ledRed->Value);
    SW[SwTowerYellow].OnOff(fMain->ledYellow->Value);
    SW[SwTowerGreen].OnOff(fMain->ledGreen->Value);

    //ben 20230913 add
    //==>
    if(IsSafePLCIOInstall()==true)   //AI(W906-W217) 20261010 (Ifor01): golden 913 ckernel.cpp:950
    {
        fMain->pnlSafePLC->Visible=true;
        { extern bool bSafePLCThread; fMain->ledSafePLC->Value=bSafePLCThread; }   // AI(W906-PLC-GATES) 20260926: golden ckernel.cpp:924 (display only, inside if(Enable_PLCSafety_IO==true)) -- the gate's reason below (sm does not link comms) is dissolved with csystem SEAM S4. Was: SAFETY-GATE(W906-MSTATE-SAFEPLC) golden ckernel.cpp:924 `fMain->ledSafePLC->Value=bSafePLCThread;` —— 缺相依：bSafePLCThread 定義在 MyPLC/MyPLC_IO_Modbus.cpp:93（ht9045_comms），ht9045_sm 刻意不連 comms（csystem.cpp 的 SEAM S4 同理）。純顯示（安全 PLC 心跳燈），同一個 if 裡的 pnlSafePLC->Visible 照翻
    }
    else
    {
        fMain->pnlSafePLC->Visible=false;
    }
    //<==
    //ben 20230913 add
}
//AI(W906-MSTATE-P2b) 20260924: 解閘結束 —— 此行原本是 `#endif // GOLDEN VERBATIM -- golden ckernel.cpp:704-932  (GATE G-PTm1-ShowRunLed, end)`
//AI(W906-W7-L2) 20260803: DEFERRED STUB for golden ckernel.cpp:704-932.
// This is NOT ShowRunLed.  It selects no RunState, drives no SW[SwMusic1+i],
// writes no fMain->led*, and returns without touching a single golden input.
// Its one observable effect is the counter bump below.  See the banner at the
// top of this fragment for the measured census and for the golden oddities
// catalogued [G1]-[G14], of which [G1], [G7] and [G10] land in this function.
// Scheduled for W7-U.
#if 0 // AI(W906-MSTATE-P2b) 20260924: 空殼退役 —— 真本體已在上方 :1604 解閘；以下到 :1878 保留原文當史料，不編譯
{
    ++W906_ShowRunLed_Count;

#if 0 // TODO(W7-U): golden ckernel.cpp:704-932 -- the ENTIRE body, untranslated. Near-pure VCL form painting: 31 fMain-> / 20 fNote-> derefs plus fTowerLight, fShowBinSet, MyMessageBox, fHome and CCDInterfaceForm, and it needs ~6 widget stand-ins (tri-state LED `Value`, panel `Visible`, TEdit `Text`) that this tree does not have. Deferred by W7-L2 charter, not by discovery.
    // Golden structure, for whoever picks this up (spans read this pass):
    //   :706-707   locals -- `static bool OldFlushFlag;` (see [G10]), `int is, i;`
    //   :708-788   RunState selection ladder, 9 arms in this order:
    //              :708  fNote->fShow || IsEMGPressed()   -> AutoRetest / Message / ErrJam
    //                    (:710-716 the bTowerLightUseLD MES-code list, 18 fNote->edErrorCode reads)
    //              :729  MyMessageBox->fShow || fShowBinSet->fShow -> LED_Message
    //              :736  bTesterSendPause                 -> LED_Message (+bAlarmBuzzer)
    //              :744  fHome->fShow                     -> LED_Homeing
    //              :748  bReplyFTCTAlarm || bContinueMessageByReply71 -> LED_Message
    //              :753  SystemStart && (rsmAutoRetest || bAutoRetestMusic) -> LED_AutoRetest
    //              :757-763  #ifndef DEBUG_SPIL: OFF_LINE && OFFLINE_ALARM -> LED_OfflineRun
    //              :764  SystemStart -> Heating / Message / Running
    //              :782  else       -> Message (bG14UseStartSoundAlarm) / Pause
    //   :789-836   buzzer + music group under `// 處理音樂`, gated on
    //              fTowerLight->fShow==false: SECS-GEM override :792-798
    //              (SW[SwMusic1+iSECS_GEM_PPMUSIC_CONTROL_CLASS-1]),
    //              LastSet.MusicSelect[RunState] :801-824, CCD override :826-833
    //   :838-842   the FlushFlag latch early-return (everything below is
    //              edge-triggered on FlushFlag; everything above is not)
    //   :844-867   SECS-GEM signal-tower override of ledRed/ledGreen/ledYellow
    //   :868-913   normal path: LastSet.MessageLight[RunState][0..2] tri-state
    //              (1=solid, 2=blink, else off), plus :871-874 SCK no-tray
    //              green blink, :891 the [G1] precedence line, :894-900
    //              bNeedMusicAndAlarmOn + SwFKAlarmReset/SwRKAlarmReset,
    //              :904-912 bTempLess30deg / bTempHeaterOk all-blink
    //   :915-917   SW[SwTowerRed/SwTowerYellow/SwTowerGreen].OnOff(led->Value)
    //   :921-929   Enable_PLCSafety_IO -> fMain->pnlSafePLC / ledSafePLC
    // The 14 CJK-bearing comment lines in this span -- :708 :710 :720 :729
    // :731 :748 :749 :784 :789 :792 :809 :829 :870 :894 -- stay in golden
    // until W7-U decodes them with cp950.  (14 + 72 in ShowRunLabel = the 86
    // counted for :704-1726 in the banner.)
#endif
#endif // AI(W906-MSTATE-P2b) 20260924: 空殼退役結束（此行原本是空殼的收尾 `}`）
//------------------------------------------------------------------------------
//AI(W906-W7-L2) 20260803: golden ckernel.cpp:934 verbatim.  Kept even though
// both functions that read these three cursors are deferred, because the line
// is inside this fragment's golden range and dropping it would be a silent
// deviation.  Ported homes: iArmTask -> aHotPlateSubstrate.h:932 (defined
// aHotPlateSubstrate.cpp:916); iTestHeadMotorTask -> atester.h:169 (defined
// atester.cpp:1470); OutArmTask -> defined aoutarm_shims.cpp:31, which IS in
// the build (CMakeLists.txt:1342), with 28 `extern int OutArmTask;`
// re-declarations across the aoutarm9045* variants.
// CORRECTED AFTER AUDIT: an earlier draft of this comment claimed OutArmTask had
// no ported declaration and no definition, and escalated "land int OutArmTask=1"
// to the integrator -- which would have produced a duplicate definition and a
// link error.  A re-declaration of an already-declared extern is legal and
// identical in type, so this line is safe either way.
// THE REAL GAP, which looking for absence missed: an INITIALISER MISMATCH.
// Golden aoutarm.cpp:42 is `int OutArmTask=1;`; ported aoutarm_shims.cpp:31 is
// `int  OutArmTask            = 0;`.  Golden's only reader of it in this whole
// file is :1576 -- `if(HasICUnderMachine() || iArmTask!=1 || OutArmTask!=1 ||
// iTestHeadMotorTask!=1)` -- which tests `!=1`, so the ported 0 inverts that
// predicate.  No live impact on this front (:1576 is inside ShowRunLabel, which
// is deferred below), and aoutarm_shims.cpp belongs to another wave with 28
// consumers, so it is NOT changed here.  W7-U must resolve it before it un-gates
// ShowRunLabel.
extern int iArmTask,OutArmTask,iTestHeadMotorTask;
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- ShowRunLabel
//  ----------------------------------
//  The block below is golden ckernel.cpp:935-1726 (792 golden lines)
//  transcribed CHARACTER-FOR-CHARACTER (Big5/cp950 -> UTF-8 only) and GATED
//  OFF.  It is INERT: the ACTIVE ShowRunLabel is the counter-bump DEFERRED stub
//  below (void ShowRunLabel() at preimage :1651), which this pair does NOT touch.
//
//  WHY: the census scored ShowRunLabel "translated" because a same-named live body
//  exists; it never compared SIZE.  That live body is a 73-line DEFERRED
//  STUB (W906-W7-L2, 20260803) whose own #if 0 held a PROSE SUMMARY of
//  golden, not golden.  These 792 lines of golden text existed NOWHERE in
//  this tree.  Now they do, auditable line-by-line, so the W7-U un-gate is
//  mechanical rather than a re-translation.  NET BEHAVIOUR CHANGE: ZERO.
//
//  NOTHING inside the gate is fixed, renamed, reflowed or reindented --
//  golden's own defects are preserved on purpose so a diff against golden
//  stays empty.  Preserved defects in this span:
//    golden :944-951   the ASYMMETRIC double latch.  OldFlushFlag is advanced at :947 BEFORE
//                      the second guard at :949 can return, so that return leaves
//                      OldiHeaterWaitTime stale while OldFlushFlag has already moved on.
//    golden :1564-1567 CheckCanChangeRealDummy()==false || HasICUnderMachine() &&
//                      HasAnyICInMachine() && CUSTOMER_CODE==CC_ASE_KaohSiung -- mixed &&/||
//                      with NO parentheses, so it means A||(B&&C&&D).  NOT corrected.
//    golden :1239-1244 dead block: the guarded ShowNowStatus(iHeaterWaitTime) is immediately
//                      overwritten by the unconditional :1246-1248 with the SAME value.
//    golden :1394-1403 index [1] of iTempStart/iTempFinish is WRITTEN (:1362 :1382 :1391) and
//                      never READ -- only [0] is ever tested.  Half-dead pair kept as-is.
//    golden :1677-1679 and :1687-1689  two deliberately EMPTY branch bodies.  Kept.
//
//  Same shape as this tree's existing pairs: csystem.cpp MainProc / DoAllProcess
//  / DoTrayFeedProcess / CheckContinusStartIsReady, plus the 26 pairs PT-W6b
//  landed in atester.cpp.  Being gated it needs NO callee to exist -- only
//  lexical validity.  Do NOT add stubs or declarations for its symbols; the
//  un-gate blockers are listed in the wave report, not papered over here.
// ===========================================================================
static const TColor clNavy = TColor(0x00800000), clFuchsia = TColor(0x00FF00FF), clTeal = TColor(0x00808000), clRed = TColor(0x000000FF), clLime = TColor(0x0000FF00), clBlack = TColor(0x00000000), clBlue = TColor(0x00FF0000);  //AI(W906-MSTATE-P2) 20260924: 解閘 —— 此行原本是 `#if 0 // GOLDEN VERBATIM ... GATE G-PTm1-ShowRunLabel`。七個顏色都是 golden Graphics.hpp 的值：clNavy/clFuchsia/clTeal 不在共用 9 色區塊（計畫書 §3 第 4 條），clRed/clLime/clBlack/clBlue 在但 ckernel.cpp 沒有 include 那些標頭（實測編譯錯誤），依 ATC/ATCInterface.h:113-118 的慣例在使用端就地補；寫在這一行是為了不移動其後行號
void ShowRunLabel()
{   ++W906_ShowRunLabel_Count;   //AI(W906-MSTATE-P2) 20260924: 原本空殼（:2743）唯一的效果，保留給 tests/test_w7_l2_ckernel.cpp 的 DoSystemMessage 排程斷言；接在同一行，不移動行號
    AnsiString str;
    static bool OldFlushFlag=false;
    static int OldiHeaterWaitTime=-1, CoolTime=0, iOldSec=-1;
    AnsiString EmgStr="";
    static int iTempStart[2]={0,0};                                             //kevin 20200829 add
    static int iTempFinish[2]={0,0};                                            //kevin 20200829 add

    if(OldFlushFlag==FlushFlag && OldiHeaterWaitTime==iHeaterWaitTime)
        return;

    OldFlushFlag=FlushFlag;

    if(FlushFlag==false && OldiHeaterWaitTime==iHeaterWaitTime)
        return;
    OldiHeaterWaitTime=iHeaterWaitTime;

    if(bSendChangeTempDelayAlarm)                                               //kevin 20151023 Auto Temp offset
    {
        if(tSendChangeTempDelay.Off())
        {
            bSendChangeTempDelayAlarm=false;
            RecordProcess("Auto Temp offset finish");
        }
    }

    if(IsTriTempFixDoorLockCheck())                                             //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        fMain->ShowNowStatus(clRed, "FixDoorOpen"); //檢查SafeDoor 6            //Ztex 2024.05.12 Change FixDoorOpen
        CoolTime=0;
        return;
    }

    if(((W906_FormShowing("MyMessageBox", MyMessageBox->fShow)==true &&  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
         iUnLoaderCount==0) ||                                                  //Steven 20140627 : Add for Secs Gem
         W906_FormShowing("fNote", fNote->fShow)==true) &&                                                 //JerryYang 20160830 考慮auto滿tray時show message但不停機的情況,機台不要show alarm  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
        bAlarmReset==false)                                                     //Steven 20140905 : 紀錄有被按下Alarm Reset
    {
        fMain->ShowNowStatus(clRed, "Alarm");
        return;
    }
    else if(IsSafeLockCheck())
    {
        fMain->ShowNowStatus(clBlack, "LOCK");
        CoolTime=0;
        return;
    }
    else if(IsEMGPressed())
    {
        //Ifor 20210810 add:Copy 7000
        //==>
//        if(bOutArmPlaceToUnloaderDestroy==true)
//        {
//            bOutArmPlaceToUnloaderDestroy_EMGAndPowerOff=true;
//        }
//        else
//        {
//            bOutArmPlaceToUnloaderDestroy_EMGAndPowerOff=false;
//        }
        //<==
        //Ifor 20210810 add:Copy 7000

        if(Sen[SnFrontLeftEMG].IsOff())
            EmgStr="EMG 1";
        else if(Sen[SnFrontRightEMG].IsOff())
            EmgStr="EMG 4";
        else if(Sen[SnRearLeftEMG].IsOff())
            EmgStr="EMG 2";
        else if(Sen[SnRearRightEMG].IsOff())                                    //kevin 20140121 add
            EmgStr="EMG 3";
        else if(IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff())                   //KenHsieh 20250212 : 新增PLC 斷線可瞬間判斷EMG及安全門  AI(W906-W217) 20261010 (Ifor01): golden 913 ckernel.cpp:1036
            EmgStr="EMG & PLC";
        else
            EmgStr="Servo Off";                                                 //kevin 20140121 偵測sevon 訊號

        fMain->ShowNowStatus(clRed, EmgStr);
        CoolTime=0;
        return;
    }
    else if(Sen[SnMotorPower].IsOff())
    {
        //Ifor 20210810 add:Copy 7000
        //==>
//        if(bOutArmPlaceToUnloaderDestroy==true)
//        {
//            bOutArmPlaceToUnloaderDestroy_EMGAndPowerOff=true;
//        }
//        else
//        {
//            bOutArmPlaceToUnloaderDestroy_EMGAndPowerOff=false;
//        }
        //<==
        //Ifor 20210810 add:Copy 7000

        fMain->ShowNowStatus(clRed, "Power Off");
        CoolTime=0;
        return;
    }

    if(IniConfig.bQAMode==true && LastSet.iRunStartMode==rsmQAMode &&           //Steven 20111128
       CUSTOMER_CODE!=CC_KYEC_LEE)                                              //Eastsun 20260526 #026-1.27 Ifor 20201109 add:KYEC 主畫面上不顯示QA Count
    {
        fMain->labQAMode->Visible=true;
        if(CosFunction.bQAModeUseUnloadCnt)                                     //JerryYang 20221004 : Maxim版本QA mode
        {
            str.sprintf("[Operation count %d / Setting Count %d]", iQAModePassCT, Prod.iQAModeCount);   //jou 2012-05-03 增加QA mode使用獨立的Loader Count
        }
        else
        {
            str.sprintf("[Operation count %d / Setting Count %d]", iQAModeLoaderCT, Prod.iQAModeCount);   //jou 2012-05-03 增加QA mode使用獨立的Loader Count
        }
        fMain->labQAMode->Caption=str;
    }
    else
    {
        fMain->labQAMode->Visible=false;
    }

    if(SystemStart)
    {
        fMain->EnabledSetupFile(false);
        fLotInfo->btnFtpServer->Enabled=false;
        fLotInfo->btnFtpHD->Enabled=false;

        //ChungHung 20140716 add if testing not finish can not homing
        if(bTesterSendPause)                                                    //Steven 20201022 : For RFMD
        {
            fMain->ShowNowStatus(clRed, "Tester PAUSE");
            CoolTime=0;
        }
        else if(IniConfig.bI01TesterFinishThenHome &&
                LastSet.iTester==ON_LINE &&
                bWaitTesterFinish)                                              //kevin 20150721 收到測試資料才能home
        {
            fMain->ShowNowStatus((TColor) 0x000080FF, "Wait Tester");
            CoolTime=0;
        }
        else if(fHome->fShow)   //AI(W906-FSHOW-B2) 20260929: 維持讀程式狀態（R142＝A：golden 的 Home Monitor 開著＝正在回原點，網頁自己開的不算）
        {
            fMain->ShowNowStatus((TColor) 0x000080FF, "Homing");
            CoolTime=0;
        }
        else if((ATC_SYSTEM==eATCHonPrecType ||
                 ATC_SYSTEM==eNewATCSystem) &&
                bNeedWaitATCRunSelfTestFinish)                                  //Ifor 20160720 add for ATC Safe Test start
        {
            fMain->ShowNowStatus((TColor) 0x000080FF, "ATC Self Test");
            CoolTime=0;
        }
        //==> Eastsun 20260526 #026-1.28 Ifor 20230608/20231107 add:KYEC 要求新增在主畫面顯示ATC Temp Wait秒數
        else if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false && bCheckATCTemp==true && iATCTempWaitTimer!=0)
        {
            fMain->ShowNowStatus(clFuchsia, AnsiString(iATCTempWaitTimer));
            fMain->labDelayStatus->Caption="ATC Temp Wait";
            fMain->labDelayStatus->Visible=true;
            CoolTime=0;
        }
        //<== Eastsun 20260526 #026-1.28
        else if(CUSTOMER_CODE!=CC_KYEC_LEE &&
                bDoIniStartAutoIonFanClean &&                                   //Isaac 20210609 : IO觸發IonFan清針
                bStartAutoIonFanClean==true)                                    //Ifor 20210720 add: IO觸發IonFan清針才顯示
        {
            fMain->ShowNowStatus((TColor) 0x000080FF, "Auto Clean Ion Fan");
        }
        else if(AUTO3_IS_MAGAZINE==1 && iMagazineStatus!=0)                     //Ifor 20231130 add: 0: Normal 1:Full Tray 2:Manual put tray
        {
            fMain->labDelayStatus->Caption="MAGAZINE Remove The Tray";
            fMain->labDelayStatus->Visible=true;
        }
        else
        {
            if(LastSet.iRunStartMode==rsmAutoRetest)                            //Steven 20140409 : Auto Retest
            {
                fMain->ShowNowStatus(clNavy, "Auto Retest");
            }
//            else if(fHeaterOK==false && LastSet.iTemperature==Tempture_Ambient && Temperature.bAmbientGuardbandCheck && IniConfig.bL20AbientGuardBand)  //Steven 20200427 : JCET凌中心說常溫溫度過低不能生產
//            {
//                fMain->ShowNowStatus((TColor) 0x000080FF, "Heater Wait");
//            }
            else if(LastSet.iTemperature==Tempture_Hot ||
                    LastSet.iTemperature==Tempture_AmbientHot ||                //kevin 2014091
                    (LastSet.iTemperature==Tempture_Ambient && CUSTOMER_CODE==CC_KYEC_LEE))     //KaiChen 20180201 (Steven) ：Use initial start delay in socket 增加常溫顯示
            {
                if(bSendChangeTempDelayAlarm)                                   //kevin 20151023 Auto Temp offset
                {
                    fMain->ShowNowStatus(clNavy, "Auto Temp offset");
                }
                else if(fHeaterOK ||
                        (LastSet.iTemperature==Tempture_Ambient &&
                         CUSTOMER_CODE==CC_KYEC_LEE))                           //KaiChen 20180201 (Steven) ：Use initial start delay in socket 增加常溫顯示
                {
                    if(bOneTimeHotPlateCheckAll && IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed && LastSet.iTemperature==Tempture_Hot)  //Steven : 20120315 主畫面狀態 --> HP Check
                    {
                        fMain->ShowNowStatus(clNavy, "HP Check");
                    }
                    else if(fContact->IsRun2DCheck()==true)                     //JerryYang 20250220 : 2DID硬體順序檢查功能
                    {
                        fMain->ShowNowStatus(clNavy, "2DID Map Checking");
                    }
                    else if(bRunAutoClean)                                      //Steven 20120208
                    {
                        fMain->ShowNowStatus(clNavy, "Cleaning");
                    }
                    else if(bRunOcrInsp)                                        //ChungHung 20120830 add OCR Function add
                    {
                        fMain->ShowNowStatus(clNavy, "OCR Insp");
                    }
                    else if(bDoEmptySocketCheck)                                //Steven 20201022 : For RFMD Empty Socket Check Funstion.
                    {
                        fMain->ShowNowStatus(clRed, "ESC");
                    }
                    else if(bResetMode || bResetModeAndCleanOut)                //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
                    {
                        if(bDoEmptySocketOneCycle)                              //Steven 20201022 : For RFMD Empty Socket Check Funstion.
                            fMain->ShowNowStatus(clRed, "ESC Reset");
                        else
                            fMain->ShowNowStatus(clRed, "Resetting");
                    }
                    else if(bCheckIndex)                                        //Steven 20110502
                    {
                        fMain->ShowNowStatus(clNavy, "Piggy Back");
                        SocketAirCoolingStart();                                //jou 2016-04-28 Socket Air Cooling contact count trun on
                    }
                    else if(IniConfig.bQAMode==true &&
                            LastSet.iRunStartMode==rsmQAMode)                   //Steven 20111128
                    {
                        fMain->ShowNowStatus(clLime, "QA Mode");
                    }
                    else if(bIsAutoOneCycleAutoclean)                           //kevin 20120710
                    {
                        fMain->ShowNowStatus(clNavy, "Onecycle Cleaning");
                    }
                    else if(IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain && bLoaderNoTrayAutoCleanOut) //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                    {
                        fMain->ShowNowStatus(clNavy, "No Tray");
                    }
                    else
                    {
                        if((CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_KYEC_LEE) &&
                           USE_AUTO_RETEST==eartInstall &&
                           (bAutoReTest_ART || (CUSTOMER_CODE==CC_KYEC_LEE && IniConfig.bA10_AutoReTest)) &&
                           (LastSet.iRunStartMode==rsmInitial_ART       ||
                            LastSet.iRunStartMode==rsmContinuStart_ART  ||
                            LastSet.iRunStartMode==rsmContinuRetest_ART))       //kevin 20150602     //Frank 20161017 KYEC_KW want add
                        {
                            if(LastSet.iRunStartMode==rsmAutoRetest)            //kevin 20150721 : Auto Retest
                            {
                                fMain->ShowNowStatus(clNavy, "Auto Retest");
                            }
                            else
                            {
                                if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                                {
                                    fMain->ARTCombine->Visible=true;
                                    if(bARTSeparate==false)                     //kevin 20170908 (wei) add art 分bin 狀態成立
                                    {
                                        fMain->ARTCombine->Caption="ART Combine Bin";
                                    }
                                    else
                                    {
                                        fMain->ARTCombine->Caption="ART Separate Bin";
                                    }
                                }

                                switch(LastSet.iAutoRetestCount_ART)            //Steven 20140409 : Auto Retest
                                {
                                    case 0:
                                        fMain->ShowNowStatus(clNavy, "FT ART");
                                        break;
                                    case 1:
                                        fMain->ShowNowStatus(clNavy, "RT1 ART");
                                        break;
                                    case 2:
                                        fMain->ShowNowStatus(clNavy, "RT2 ART");
                                        break;
                                    case 3:
                                        fMain->ShowNowStatus(clNavy, "RT3 ART");
                                        break;
                                    case 4:
                                        fMain->ShowNowStatus(clNavy, "RT4 ART");
                                        break;
                                    case 5:
                                        fMain->ShowNowStatus(clNavy, "RT5 ART");
                                        break;
                                    default:
                                        fMain->ShowNowStatus(clLime, "Running");
                                        break;
                                }
                            }
                        }
                        else if(LastSet.iRunStartMode==rsmContinuRetest_ART)    //kevin 20150715
                        {
                            if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                                fMain->ShowNowStatus(clNavy, "ART_Tray Move");  //kevin 20170908
                            else
                                fMain->ShowNowStatus(clNavy, "ART_ReTest");
                        }
                        else
                        {
                            fMain->ShowNowStatus(clLime, "Running");
                        }
                    }
                    CoolTime=0;
                    if(iOneCycle==0 && bRunAutoClean==false &&                  //kevin 20130220 add iOneCycle 時會秀sacktime
                       bIsAutoOneCycleAutoclean==false &&
                       iHeaterWaitTime!=0)                                      //kevin 20121023
                    {
                        fMain->ShowNowStatus(clLime, AnsiString(iHeaterWaitTime));
                    }

                    if(iHeaterWaitTime!=0)
                    {
                        fMain->ShowNowStatus(clLime, AnsiString(iHeaterWaitTime));
                        fMain->labDelayStatus->Caption="Heater Wait Time";      //wei 20171020 (jou) 延遲狀態顯示
                        fMain->labDelayStatus->Visible=true;                    //JerryYang 20171211 (Steven) 有用到再顯示避免擋到SECS GEM連線狀態
                    }
                    else if(Temperature.bLBTempFunction && iTriggerBoostFunction==Temperature.eBLBL)
                    {
                        fMain->ShowNowStatus(clFuchsia, AnsiString("Wait L/B"));
                    }
                    else if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) && iTriggerBoostFunction!=-1 && iBoostFuncStep==1)   //Steven 20180817 : Boost Function
                    {
                        fMain->ShowNowStatus(clFuchsia, AnsiString(iInitialCount));
                        fMain->labDelayStatus->Caption="Boost Duration";
                        fMain->labDelayStatus->Visible=true;
                    }
                    else if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) && iTriggerBoostFunction!=-1 && iBoostFuncStep==4)   //Steven 20180817 : Boost Function
                    {
                        fMain->ShowNowStatus(clFuchsia, AnsiString(iInitialCount));
                        fMain->labDelayStatus->Caption="Post Boost Duration";
                        fMain->labDelayStatus->Visible=true;
                    }
                    else if(((IniConfig.bA05UseAutoDocking==true && Temperature.bUseTesterDocking==true) || CUSTOMER_CODE==CC_SCK) && bTesterDucking==true && iTesterDucking>0) //ChungHung 20140714 only for SCK add CUSTOMER_CODE==CC_SCK
                    {
                        fMain->ShowNowStatus((TColor) 0x000080FF, AnsiString(iTesterDucking));
                    }
                    else if(bNeedInitialTestDelay && iInitialCount>0)           //ChungHung 20140801 add Korea Want to count down in main status
                    {
                        fMain->ShowNowStatus(clFuchsia, AnsiString(iInitialCount));
                        fMain->labDelayStatus->Caption=InitialTestDelayStatus();    //wei 20171020 (jou) 延遲狀態顯示
                        fMain->labDelayStatus->Visible=true;                        //JerryYang 20171211 (Steven) 有用到再顯示避免擋到SECS GEM連線狀態
                    }
                    else if(CosFunction.bEnableAfterTestedDelay && iAfterTestedCount>0) //ChungHung 20140730 add for ATK function after tested delay time
                    {
                        fMain->ShowNowStatus(clTeal, AnsiString(iAfterTestedCount));
                    }
                    else if(iSoakTimer>0)                                       //2013-11-27    Dell Add Index soak time
                    {
                        fMain->ShowNowStatus(clLime, AnsiString(iSoakTimer));
                        fMain->labDelayStatus->Caption="Soak Time";             //wei 20171020 (jou) 延遲狀態顯示
                        fMain->labDelayStatus->Visible=true;                    //JerryYang 20171211 (Steven) 有用到再顯示避免擋到SECS GEM連線狀態
                    }
                    else if(fHeaterOK && fHeaterStableOK==false && iStableTime>0)   //JerryYang 20210122 : ASE-CL新增待溫功能
                    {
                        fMain->ShowNowStatus(clLime, AnsiString(iStableTime));
                        fMain->labDelayStatus->Caption="Stable Time";
                        fMain->labDelayStatus->Visible=true;
                    }
                    else if(iInitialSoakTimer>0)                                //Steven 20140827
                    {
                        fMain->ShowNowStatus(clLime, AnsiString(iInitialSoakTimer));
                        fMain->labDelayStatus->Caption="Initial Soak Time";     //wei 20171020 (jou) 延遲狀態顯示
                        fMain->labDelayStatus->Visible=true;                    //JerryYang 20171211 (Steven) 有用到再顯示避免擋到SECS GEM連線狀態
                    }
                    else if(bInitStartDelayTime && iInitStartDelayCount>0)      //wei 20171020 (jou) InitStartDelayTime秒數倒數
                    {
                        fMain->ShowNowStatus(clBlue, AnsiString(iInitStartDelayCount));
                        fMain->labDelayStatus->Caption="Initial Start Delay Time";  //wei 20171020 (jou) 延遲狀態顯示
                        fMain->labDelayStatus->Visible=true;                    //JerryYang 20171211 (Steven) 有用到再顯示避免擋到SECS GEM連線狀態
                    }
                    else if(bInitialStart1Time && iInitialStart1Count>0)        //wei 20171020 (jou) InitialStart1 秒數倒數
                    {
                        fMain->ShowNowStatus(clBlue, AnsiString(iInitialStart1Count));
                        fMain->labDelayStatus->Caption="Initial Start 1 Time";  //wei 20171020 (jou) 延遲狀態顯示
                        fMain->labDelayStatus->Visible=true;                    //JerryYang 20171211 (Steven) 有用到再顯示避免擋到SECS GEM連線狀態
                    }
                    else if(bInitialStart2Time && iInitialStart2Count>0)        //kevin 20180905 InitialStart1 秒數倒數
                    {
                        fMain->ShowNowStatus(clBlue, AnsiString(iInitialStart2Count));
                        fMain->labDelayStatus->Caption="Shuttle Soak Time";
                        fMain->labDelayStatus->Visible=true;
                    }
                    else
                    {
                        fMain->labDelayStatus->Caption="";                      //wei 20171020 (jou) 延遲狀態顯示
                        fMain->labDelayStatus->Visible=false;                   //JerryYang 20171211 (Steven) 有用到再顯示避免擋到SECS GEM連線狀態
                        for(int i=0; i<15; i++)                                 //kevin 20200730 10->15
                        {
                            bInitialTestDelayStatus[i]=false;
                        }
                    }

                    bool bFlag=fMain->labDelayStatus->Visible;
                    if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) && iTriggerBoostFuncBack!=-1 && iBoostFuncStep==10)   //Steven 20180817 : Boost Function
                    {
                        str.sprintf("Boost Cooling : %d", iBoostCountDown);
                        fMain->labDelayStatus->Caption=str;
                        fMain->labDelayStatus->Visible=true;
                    }
                    else if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) && iTriggerBoostFuncBack!=-1 && iBoostFuncStep==11)   //Steven 20180817 : Boost Function
                    {
                        str.sprintf("L/B Boosting");
                        fMain->labDelayStatus->Caption=str;
                        fMain->labDelayStatus->Visible=true;
                    }
                    else
                    {
                        if(bFlag==false)
                            fMain->labDelayStatus->Visible=false;
                    }

                    if(IniConfig.bUseAutoSiteMapping)                           //jou 2011-03-24 start : Auto Site Mapping
                    {
                        if(LastSet.iTemperature==Tempture_Hot && LastSet.iRunStartMode==rsmAutoSiteMap && bSiteMappingCHKOK==false)
                        {
                            if(iShuttleHeaterWaitTime>0)
                            {
                                fMain->ShowNowStatus(clLime,AnsiString(iShuttleHeaterWaitTime));
                            }
                        }
                    }
                    //jou 2011-03-24 end
                }
                else
                {
                    iTempStart[0]=0;                                            //kevin 20200829 add
                    iTempStart[1]=0;                                            //kevin 20200829 add
                    if(Temperature.bLBTempFunction && iTriggerBoostFunction==Temperature.eBLBL)
                    {
                        fMain->ShowNowStatus(clFuchsia, AnsiString("Wait L/B"));
                    }
                    else if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) && iTriggerBoostFunction!=-1 && iBoostFuncStep==1)   //Steven 20180817 : Boost Function
                    {
                        fMain->ShowNowStatus(clFuchsia, AnsiString(iInitialCount));
                        fMain->labDelayStatus->Caption="Boost Duration";
                        fMain->labDelayStatus->Visible=true;
                    }
                    else if((Temperature.bBoostFuncttion || Temperature.bLBTempFunction) && iTriggerBoostFunction!=-1 && iBoostFuncStep==4)   //Steven 20180817 : Boost Function
                    {
                        fMain->ShowNowStatus(clFuchsia, AnsiString(iInitialCount));
                        fMain->labDelayStatus->Caption="Post Boost Duration";
                        fMain->labDelayStatus->Visible=true;
                    }
                    else
                    {
                        iTempStart[0]=1;                                        //kevin 20200829 add
                        iTempStart[1]=1;                                        //kevin 20200829 add
                        fMain->ShowNowStatus((TColor) 0x000080FF, "Heater Wait");
                    }
                    CoolTime=0;
                    if(iTemperatureOk!=2)
                    {
                        iTemperatureOk=2;                                       //kevin 20150914
                        NewRecordProcess("MES2130", "Temperature Wait");        //kevin 20150914
                        iTempStart[0]=1;                                        //kevin 20200829 add
                        iTempStart[1]=1;                                        //kevin 20200829 add
                    }

                    if(iTempStart[0]==1)                                        //kevin 20200829 add 加熱完成
                    {
                       iTempFinish[0]=2;
                    }

                    if(iTempStart[0]==0 && iTempFinish[0]==2)                   //kevin 20200829 add 加熱完成
                    {
                       iTempFinish[0]=0;                                        //kevin 20200829 add
                       RecordProcess("Working Temperature Arrived Done");       //kevin 20200829  加熱完成
                    }
                }
            }
            else
            {
                if(iHeaterCooling)
                {
                    fMain->ShowNowStatus((TColor) 0x000080FF, "Cooling Wait");
                    CoolTime=0;
                }
                else
                {
                    if(fContact->IsRun2DCheck())                                //JerryYang 20250220 : 2DID硬體順序檢查功能
                    {
                        fMain->ShowNowStatus(clNavy, "2DID Map Checking");
                    }
                    else if(bRunAutoClean)                                      //Steven 20120208
                    {
                        fMain->ShowNowStatus(clNavy, "Cleaning");
                    }
                    else if(bDoEmptySocketCheck)                                //Steven 20201022 : For RFMD Empty Socket Check Funstion.
                    {
                        fMain->ShowNowStatus(clRed, "ESC");
                    }
                    else if(bResetMode || bResetModeAndCleanOut)                //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
                    {
                        if(bDoEmptySocketOneCycle)                              //Steven 20201022 : For RFMD Empty Socket Check Funstion.
                            fMain->ShowNowStatus(clRed, "ESC Reset");
                        else
                            fMain->ShowNowStatus(clRed, "Resetting");
                    }
                    else if(bCheckIndex)                                        //Steven 20110502
                    {
                        fMain->ShowNowStatus(clNavy, "Piggy Back");
                        SocketAirCoolingStart();                                //jou 2016-04-28 Socket Air Cooling contact count trun on
                    }
                    else if(IniConfig.bQAMode==true && LastSet.iRunStartMode==rsmQAMode)    //Steven 20111128
                    {
                        fMain->ShowNowStatus(clLime, "QA Mode");
                    }
                    else if(bIsAutoOneCycleAutoclean)                           //kevin 20120710
                    {
                        fMain->ShowNowStatus(clNavy, "Onecycle Cleaning");
                    }
                    else if(IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain && bLoaderNoTrayAutoCleanOut) //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                    {
                        fMain->ShowNowStatus(clNavy, "No Tray");
                    }
                    else if((CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_KYEC_LEE) &&
                            USE_AUTO_RETEST==eartInstall &&
                            (bAutoReTest_ART || (CUSTOMER_CODE==CC_KYEC_LEE && IniConfig.bA10_AutoReTest)) &&
                            (LastSet.iRunStartMode==rsmInitial_ART      ||
                             LastSet.iRunStartMode==rsmContinuStart_ART ||
                             LastSet.iRunStartMode==rsmContinuRetest_ART))      //kevin 20150610       //Frank 20161017 KYEC_KW want add
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                        {
                            fMain->ARTCombine->Visible=true;                    //kevin 20170908 (wei) add art 分bin 狀態成立
                            if(bARTSeparate==false)
                            {
                                fMain->ARTCombine->Caption="ART Combine Bin";
                            }
                            else
                            {
                                fMain->ARTCombine->Caption="ART Separate Bin";
                            }
                        }

                        switch(LastSet.iAutoRetestCount_ART)                    //Steven 20140409 : Auto Retest
                        {
                            case 0:
                                fMain->ShowNowStatus(clNavy, "FT ART");
                                break;
                            case 1:
                                fMain->ShowNowStatus(clNavy, "RT1 ART");
                                break;
                            case 2:
                                fMain->ShowNowStatus(clNavy, "RT2 ART");
                                break;
                            case 3:
                                fMain->ShowNowStatus(clNavy, "RT3 ART");
                                break;
                            case 4:
                                fMain->ShowNowStatus(clNavy, "RT4 ART");
                                break;
                            case 5:
                                fMain->ShowNowStatus(clNavy, "RT5 ART");
                                break;
                            case 6:
                                fMain->ShowNowStatus(clNavy, "RT6 ART");
                                break;
                            case 7:
                                fMain->ShowNowStatus(clNavy, "RT7 ART");
                                break;
                            case 8:
                                fMain->ShowNowStatus(clNavy, "RT8 ART");
                                break;
                            case 9:
                                fMain->ShowNowStatus(clNavy, "RT9 ART");
                                break;
                            case 10:
                                fMain->ShowNowStatus(clNavy, "RT10 ART");
                                break;
                            default:
                                fMain->ShowNowStatus(clLime, "ART Over");
                                break;
                        }
                    }
                    else if(LastSet.iRunStartMode==rsmContinuRetest_ART)        //kevin 20150715
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
                            fMain->ShowNowStatus(clNavy, "ART_Tray Move");
                        else
                            fMain->ShowNowStatus(clNavy, "ART_ReTest");
                    }
                    else
                    {
                        if(IniConfig.bG12ContractModeManualMessage && bContractModeTest)    //kevin 20180222 (Steven) Arm 1 Arm2 吸取IC 做CONTRACT MODE
                        {                                                                   //kevin 20180222 (Steven) contract mode 秀手動送測試訊號
                            fMain->ShowNowStatus(clLime, "Manual T.Start");
                        }
                        else
                        {
                            fMain->ShowNowStatus(clLime, "Running");
                        }
                    }
                    CoolTime=0;
                }
            }
        }
        bHALTing=false;                                                         //jou 2012-01-30 常溫 & 機台沒在跑的時候chamber風扇不轉
    }
    else
    {
        if((IniConfig.bEnable_SECS_GEM==true && IniConfig.bRCMDStart==true && bPhysicalStart==true) ||          //Steven 20141006 : SECS GEM使用Remote Start功能
           (CUSTOMER_CODE==CC_ChipMos_ZHUBEI &&
            IniConfig.bN25_1_EnableStartControl && bPhysicalStart==true) ||     //JerryYang 20250120 : modify
           (IniConfig.bEnable_SECS_GEM==true && bSECSPause==true))              //Steven 20210413 : 南茂的自動Start功能
        {
            fMain->ShowNowStatus(clNavy, "RUN CHECK");
            if(IniConfig.bEnable_SECS_GEM==true &&
               ((IniConfig.bRCMDStart==true && bPhysicalStart==true) ||
                bSECSPause==true))                                              //JerryYang 20250120 : modify
                bSECSGEMAlarm=true;                                             //Ifor 20151210 鎖定面板
        }
        else if(bRunDecayTest==true)
        {
            fMain->ShowNowStatus((TColor) 0x000080FF, "Decay Test");
            CoolTime=0;
        }
        else if(CUSTOMER_CODE!=CC_KYEC_LEE &&
                bDoIniStartAutoIonFanClean &&                                   //Isaac 20210609 : IO觸發IonFan清針
                bStartAutoIonFanClean==true)                                    //Ifor 20210720 add: IO觸發IonFan清針才顯示
        {
            fMain->ShowNowStatus((TColor) 0x000080FF, "Auto Clean Ion Fan");
        }
        else if(AUTO3_IS_MAGAZINE==1 && iMagazineStatus!=0)                     //Ifor 20231130 add: 0: Normal 1:Full Tray 2:Manual put tray
        {
            fMain->labDelayStatus->Caption="MAGAZINE Remove The Tray";
            fMain->labDelayStatus->Visible=true;
        }
        else if(fMain->CheckCanChangeRealDummy()==false ||
                HasICUnderMachine() &&
                HasAnyICInMachine() &&
                CUSTOMER_CODE==CC_ASE_KaohSiung)                                //kevin 20151012
        {
            if(bCalculatePauseTime==false)
            {
                tUPH_PauseStartTime=Now();
                bCalculatePauseTime=true;
            }
            fMain->ShowNowStatus(clRed, "PAUSE");

            if(HasICUnderMachine() || iArmTask!=1 || OutArmTask!=1 || iTestHeadMotorTask!=1)
            {
                fMain->EnabledSetupFile(false);
                fLotInfo->btnFtpServer->Enabled=false;
            }
            else
            {
                if(CosFunction.bFTPFunction && IniConfig.bEnableFTP)
                {
                    fMain->EnabledSetupFile(true);                              //Steven 20250407 : 調整 cbSetupFileName 的 Enable判斷
                    if(IniConfig.iServerEnable<=AccessLevel)
                    {
#if 0 // SAFETY-GATE(W906-MSTATE-FTP) golden ckernel.cpp:1589 —— 缺相依：TfFTPClient 全樹沒有 port（BarcodeReader.h:44 的 B-F1 同一個缺口）。只閘條件行、保留本體：等同「FTP 視窗沒開」，與 W906_FShow（WebStart.cpp:210）在沒有瀏覽器總表時回「關著」的規則一致（使用者 Q8-B）
                        if(fFTPClient->bShow==false)
#endif // SAFETY-GATE(W906-MSTATE-FTP)
                            fLotInfo->btnFtpServer->Enabled=true;
                    }
                    else
                    {
                        fLotInfo->btnFtpServer->Enabled=false;
                    }
                    if(IniConfig.iHDEnable<=AccessLevel)
                        fLotInfo->btnFtpHD->Enabled=true;
                    else
                        fLotInfo->btnFtpHD->Enabled=false;
                }
                else
                {
                    fMain->EnabledSetupFile(true);
                }
            }
            CoolTime=0;
            bHALTing=false;                                                     //jou 2012-01-30 常溫 & 機台沒在跑的時候chamber風扇不轉
            fLotInfo->btnFtpHD->Enabled=true;                                   //wei 20160314 不需要onecycle or Cleanout就可上傳
        }
        else
        {
            if(iHome==0 && CheckThermo()==false)
            {
                if(iOldSec!=SystemSec)
                {
                    iOldSec=SystemSec;
                    CoolTime++;
                }
                fMain->ShowNowStatus(clBlack,AnsiString(CoolTime));
                bHALTing=false;                                                 //jou 2012-01-30 常溫 & 機台沒在跑的時候chamber風扇不轉
            }
            else
            {
                fMain->ShowNowStatus(clBlack, "HALT");
                CoolTime=0;
                bHALTing=true;                                                  //jou 2012-01-30 常溫 & 機台沒在跑的時候chamber風扇不轉
            }
            if(CosFunction.bFTPFunction && IniConfig.bEnableFTP)                // Landam 20110324 enable setup file selection
            {
                fMain->EnabledSetupFile(true);                                  //Steven 20250407 : 調整 cbSetupFileName 的 Enable判斷
                if(IniConfig.iServerEnable<=AccessLevel)                        //jou 2012-12-21 修正 FTP clean out 後，HD & server button 沒有 enabled 的問題。
                {
#if 0 // SAFETY-GATE(W906-MSTATE-FTP) golden ckernel.cpp:1636 —— 缺相依：TfFTPClient 全樹沒有 port（BarcodeReader.h:44 的 B-F1 同一個缺口）。只閘條件行、保留本體：等同「FTP 視窗沒開」，與 W906_FShow（WebStart.cpp:210）在沒有瀏覽器總表時回「關著」的規則一致（使用者 Q8-B）
                    if(fFTPClient->bShow==false)
#endif // SAFETY-GATE(W906-MSTATE-FTP)
                    {
                        if(HasICUnderMachine()==false &&
                           HasAnyICInMachine()==false)                          //JerryYang 20191212 機台內有IC不能download工作檔
                        {
                            if(CUSTOMER_CODE==CC_GIGAS &&
                               (LastSet.iRunStartMode!=rsmInitialStart &&
                                LastSet.iRunStartMode!=rsmCInitialRetest))      //Isaac 20200803 : 全智只有initial mode可以更換工作檔
                                fLotInfo->btnFtpServer->Enabled=false;
                            else
                                fLotInfo->btnFtpServer->Enabled=true;
                        }
                        else
                        {
                            fLotInfo->btnFtpServer->Enabled=false;
                        }
                    }
                }
                else
                {
                    fLotInfo->btnFtpServer->Enabled=false;
                }

                if(IniConfig.iHDEnable<=AccessLevel)
                    fLotInfo->btnFtpHD->Enabled=true;
                else
                    fLotInfo->btnFtpHD->Enabled=false;

                if(CUSTOMER_CODE==CC_Greatek)                                   //Sam 20171031 (wei) : OP 權限時隱藏 FTP
                {
                    if(AccessLevel==0)
                        fLotInfo->tsFTP->Enabled=false;                         //Sam 20190218 : 改用 Enable
                    else
                        fLotInfo->tsFTP->Enabled=true;                          //Sam 20190218 : 改用 Enable
                }
            }
            else if(CUSTOMER_CODE==CC_ASE_CL &&
                    CosFunction.bLastSetInSetUpFile)
            {
                fMain->EnabledSetupFile(OLPClientConnect);
            }
            else if(TestIF_File.bRENESAS_EnableFTCT==true)                      //RogerYang 20251020 : 瑞薩FT-CT
            {
            }
            else
            {
                fMain->EnabledSetupFile(true);
            }
            //-----------------------------------
        }

        if(AUTO3_IS_MAGAZINE==1 && iMagazineStatus!=0)
        {
        }
        else
        {
            fMain->labDelayStatus->Caption="";                                  //wei 20171020 (jou) 延遲狀態顯示
            fMain->labDelayStatus->Visible=false;                               //JerryYang 20171211 (Steven) 有用到再顯示避免擋到SECS GEM連線狀態
        }
    }

    if(IniConfig.bN09_LotCountAutoFunc && bWaitTSV)                             //Steven 20190521 : ATK lot count
    {
        fMain->ShowNowStatus(clNavy, "Waiting LCA");
    }

    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20210128 : 全智Auto Clean顯示狀態
    {
        fMain->labAutoClean->Visible=true;
        if(TestIF_File.iAutoClean_Function)
            fMain->labAutoClean->Caption="Auto clean ON";
        else
            fMain->labAutoClean->Caption="Auto clean OFF";
    }
    else
    {
        if(TestIF_File.iAutoClean_Function &&
           (IniConfig.bSIGURDFunction ||                                        //Alick 20160829 add for 矽格北興要求開啟AutoClean功能時，MAIN畫面要顯示告知Label
            CUSTOMER_CODE==CC_JCET))
            fMain->labAutoClean->Visible=true;
        else
            fMain->labAutoClean->Visible=false;
    }
#if 0 // SAFETY-GATE(W906-MSTATE-AUTOTEACH) golden ckernel.cpp:1720-1725 —— 缺相依：TfAutoTeach 全樹沒有 port（forms/fOffSet.h:154-155 明記；WebStart.cpp:1813 的 ST-W2-J 是同一個缺口）。閘住＝GetState() 恆為 0 的結果：Auto Alignment Mode 永不顯示
    if(fAutoTeach->GetState()>0 && CosFunction.bManualSteplAutoTeach)           //JimmyChiu 20211020 : Auto alignment mode
    {
        fMain->ShowNowStatus(clLime, "Auto Alignment Mode");
        CoolTime=0;
        return;
    }
#endif // SAFETY-GATE(W906-MSTATE-AUTOTEACH)
}   //AI(W906-MSTATE-P2) 20260924: 解閘結束 —— 此行原本是 `#endif // GOLDEN VERBATIM -- golden ckernel.cpp:935-1726  (GATE G-PTm1-ShowRunLabel, end)`
//AI(W906-W7-L2) 20260803: DEFERRED STUB for golden ckernel.cpp:935-1726.
// This is NOT ShowRunLabel.  It makes none of golden's 88 fMain->ShowNowStatus
// paint calls, none of its 40 labDelayStatus accesses (39 writes plus the one
// read at golden :1328), enables no FTP button, reads none of the 6 Sen[]
// EMG/power inputs, and takes none of its EIGHT returns (golden :945, :950,
// :966, :975, :981, :1013, :1032, :1724 -- counted this pass over live lines).
// Its one observable effect is the counter bump below.  The golden
// oddities the real body must carry are catalogued [G1]-[G14] in the banner at
// the top of this fragment; [G2]-[G14] land in this function ([G10] straddles both).
// Scheduled for W7-U.
#if 0 // AI(W906-MSTATE-P2) 20260924: 空殼退役 —— 真本體已在上方 :1940 解閘；以下到 :2803 保留原文當史料，不編譯
{
    ++W906_ShowRunLabel_Count;

#if 0 // TODO(W7-U): golden ckernel.cpp:935-1726 -- the ENTIRE body, untranslated. 792 lines, 150 fMain-> derefs over 7 members (ShowNowStatus 88x, labDelayStatus 40x) plus fLotInfo's three FTP widgets, fContact, fFTPClient, fAutoTeach, fHome, fNote and MyMessageBox. Needs a TColor-taking status painter, a caption/visible label, a TTabSheet Enabled and fMain->EnabledSetupFile -- none of which exist in this tree. Deferred by W7-L2 charter, not by discovery.
    // Golden structure, for whoever picks this up (spans read this pass):
    //   :937-942     locals: AnsiString str / EmgStr, static OldFlushFlag,
    //                OldiHeaterWaitTime, CoolTime, iOldSec, and the two
    //                half-dead [2] arrays of [G4]
    //   :944-951     the DOUBLE latch.  First guard returns when BOTH
    //                FlushFlag and iHeaterWaitTime are unchanged; OldFlushFlag
    //                is then updated; the SECOND guard (:949) can still return
    //                early, leaving OldiHeaterWaitTime stale while OldFlushFlag
    //                has already advanced.  Asymmetric on purpose-or-not --
    //                reproduce the order exactly.
    //   :953-960     tSendChangeTempDelay.Off() -> "Auto Temp offset finish"
    //   :962-967     IsTriTempFixDoorLockCheck() -> "FixDoorOpen", early return
    //   :969-1033    four hard early-returns, in order: "Alarm" (:974/:975),
    //                "LOCK" (:979/:981), the EMG ladder (:983-1014 -- FIVE
    //                Sen[] reads at :998 :1000 :1002 :1004 :1006, labelled
    //                EMG 1 / 4 / 2 / 3 in that physical order, then
    //                "EMG & PLC", then "Servo Off"; the SIXTH Sen[] read,
    //                SnMotorPower, is the next arm), "Power Off" (:1015-:1032)
    //   :1035-1052   labQAMode caption (see [G13])
    //   :1054-1534   SystemStart==true:
    //                :1056-1058 EnabledSetupFile(false), FTP buttons off
    //                :1061-1104 pre-empting statuses -- Tester PAUSE / Wait
    //                           Tester / Homing / ATC Self Test / KYEC ATC Temp
    //                           Wait / Auto Clean Ion Fan / MAGAZINE
    //                :1105-1531 the temperature tree:
    //                   :1107-1110 rsmAutoRetest -> "Auto Retest"
    //                   :1115-1405 Hot / AmbientHot / (Ambient && KYEC):
    //                      :1123-1358 fHeaterOK -- status ladder :1127-1237
    //                                 (incl. the [G6] ART switch at :1200),
    //                                 countdown ladder :1246-1326 (incl. the
    //                                 [G3] dead guard at :1239), boost overlay
    //                                 :1328-1345, auto-site-map :1347-1356
    //                      :1359-1404 not OK -- "Heater Wait" + MES2130 +
    //                                 the iTempStart/iTempFinish latch of [G4]
    //                   :1406-1531 else -- "Cooling Wait" (:1408-1412) or the
    //                                 second status ladder :1415-1528 (incl.
    //                                 the [G6] ART switch at :1471)
    //                :1533 bHALTing=false
    //   :1535-1695   SystemStart==false:
    //                :1537-1547 "RUN CHECK" (SECS remote start / bSECSPause)
    //                :1548-1552 "Decay Test"
    //                :1553-1563 Ion Fan / MAGAZINE
    //                :1564-1610 the [G2] PAUSE branch, its FTP-enable block
    //                           (:1583-1606) and the [G5] override at :1609
    //                :1611-1685 HALT / cool-down counter (:1613-1628) and the
    //                           big FTP-enable ladder (:1630-1683, incl. the
    //                           [G9] empty RENESAS arm at :1677)
    //                :1687-1694 labDelayStatus clear, behind the [G9] empty
    //                           MAGAZINE arm
    //   :1697-1700   "Waiting LCA" (bN09_LotCountAutoFunc && bWaitTSV)
    //   :1702-1718   labAutoClean caption/visibility
    //   :1720-1725   "Auto Alignment Mode" + the [G14] trailing return
    // The 72 CJK-bearing comment lines in this span stay in golden until W7-U
    // decodes them with cp950.  Nothing here has been machine-retranslated.
#endif
#endif // AI(W906-MSTATE-P2) 20260924: 空殼退役結束（此行原本是空殼的收尾 `}`）
//------------------------------------------------------------------------------
//AI(W906-W7-L2) 20260803: DoPanelLamp -- golden ckernel.cpp:1728-1897 (170 lines),
// translated in FULL: no gate, no stub, no reordering.  It is the panel-lamp output
// pass -- every statement is a global read plus an SW[] write, with exactly ONE
// VCL-facade read (fNote->fShow, golden :1749) which forms/fNote.h:133 now provides.
// Golden does NOT declare DoPanelLamp in ckernel.h (see ckernel.h:54-62), and its
// only golden call site -- DoSystemMessage at golden :1910 -- sits BELOW it in the
// same TU, so exactly as in golden there is no forward declaration and the external
// linkage is unchanged.
// Golden's trailing comments are Big5 and are carried over as real UTF-8, kept at
// golden's own column 80.
void DoPanelLamp()
{
    if(SystemInitialOK==false)
        return;

    if(IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain)                       //ChungHung 201320610 add No Tray AlArmReset light on
    {
        //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- golden :1735-1737 is an
        // EMPTY then-branch, written only to invert the test.  Behaviour: while the
        // loader has run out of tray AND the buzzer is sounding, DoPanelLamp leaves
        // BOTH AlarmReset lamps at whatever they already were.  Deliberately NOT
        // rewritten as `if(!(bLoaderNoTrayAutoCleanOut && bAlarmBuzzer))` -- that is
        // the same behaviour in a different shape, and this wave does not reshape
        // golden.
        if(bLoaderNoTrayAutoCleanOut && bAlarmBuzzer==true)
        {
        }
        else
        {
            if(bNeedMusicAndAlarmOn==false)                                     //kevin 20170817 (Steven) 不讓Alarm reset 燈滅
            {
                SW[SwFKAlarmReset].Off();
                SW[SwRKAlarmReset].Off();
            }
        }
    }
    else
    {
        //AI(W906-W7-L2) 20260803: golden :1749 -- the ONE VCL touch in this whole
        // function.  forms/fNote.h:112-133 pins the offline value of fShow to false
        // (golden note.cpp:164 ctor), and false OPENS this arm, so offline
        // DoPanelLamp DOES extinguish both AlarmReset lamps whenever
        // bNeedMusicAndAlarmOn is false.  fShow is a plain settable bool on the
        // facade, so a test can close the arm by writing true.
        if(W906_FormShowing("fNote", fNote->fShow)==false)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
        {
            if(bNeedMusicAndAlarmOn==false)                                     //kevin 20170817 (Steven) 不讓Alarm reset 燈滅
            {
                SW[SwFKAlarmReset].Off();
                SW[SwRKAlarmReset].Off();
            }
        }
    }

    //AI(W906-W7-L2) 20260803: golden :1759-1790 -- the LOCKOUT arm.  Either a S10F3
    // SECS alarm with no Note dialog behind it, or Employee-ID checking switched on,
    // and every operator key lamp except the two Power lamps is driven dark.  This
    // arm is symmetric: both Pause lamps are covered (SwFKPause :1768, SwRKPause
    // :1769) -- unlike the rear arm below, see the note at golden :1838.
    if((bSECSGEMAlarm && bSECSGEM_NoteAlarm==false) ||                          //wei 20150817 S10F3 按鍵不能按 //Ifor 20170616 (wei) add S10F3 後發生的Note Alarm 要恢復按鍵掃描解除Note後再鎖定按鍵
        bEnableEmployeeIDCheck==true)                                           //Ifor 20180911 (Steven) : Add 啟動 Employee ID Check
    {
        SW[SwFKPowerOff].OnOff(bLampPowerOff);
        SW[SwRKPowerOff].OnOff(bLampPowerOff);
        SW[SwFKPowerOn].OnOff(bLampPowerOn);
        SW[SwRKPowerOn].OnOff(bLampPowerOn);

        SW[SwFKReset].Off();
        SW[SwFKPause].Off();
        SW[SwRKPause].Off();
        SW[SwFKHome].Off();
        SW[SwFKStart].Off();
        SW[SwFKSkip].Off();
        SW[SwFKRetry].Off();
        SW[SwFKOneCycle].Off();
        SW[SwFKCleanOut].Off();
        SW[SwFKTrayFeed].Off();
        SW[SwFKTrayEnd].Off();
        SW[SwFKAlarmReset].Off();

        SW[SwRKReset].Off();
        SW[SwRKHome].Off();
        SW[SwRKStart].Off();
        SW[SwRKSkip].Off();
        SW[SwRKRetry].Off();
        SW[SwRKOneCycle].Off();
        SW[SwRKCleanOut].Off();
        SW[SwRKTrayFeed].Off();
        SW[SwRKTrayEnd].Off();
        SW[SwRKAlarmReset].Off();
    }
    //AI(W906-W7-L2) 20260803: golden :1791 -- FRONT panel active.  The front key
    // lamps follow their bLamp* flags only while the motor power is on AND the
    // power-on settle counter has expired; otherwise they are all driven dark.  The
    // rear key lamps (:1827-1836) are unconditionally dark either way.  Note the
    // deliberate cross-panel write at golden :1802/:1816: SwRKPause is driven from
    // the FRONT arm, because the rear Pause button stays live as a safety stop even
    // when the rear panel is not the active one.
    else if(bFrontPadActive)
    {
        SW[SwFKPowerOff].OnOff(bLampPowerOff);
        SW[SwRKPowerOff].OnOff(bLampPowerOff);
        SW[SwFKPowerOn].OnOff(bLampPowerOn);
        SW[SwRKPowerOn].OnOff(bLampPowerOn);

        if(bMotorPowerState && MotorPowerOnDelay==0)
        {
            SW[SwFKReset].OnOff(bLampReset);
            SW[SwFKPause].OnOff(bLampPause);
            SW[SwRKPause].OnOff(bLampPause);
            SW[SwFKHome].OnOff(bLampHome);
            SW[SwFKStart].OnOff(bLampStart);
            SW[SwFKSkip].OnOff(bLampSkip);
            SW[SwFKRetry].OnOff(bLampRetry);
            SW[SwFKOneCycle].OnOff(bLampOneCycle);
            SW[SwFKCleanOut].OnOff(bLampCleanOut);
            SW[SwFKTrayFeed].OnOff(bLampTrayFeed);
            SW[SwFKTrayEnd].OnOff(bLampTrayEnd);
        }
        else
        {
            SW[SwFKReset].Off();
            SW[SwFKPause].Off();
            SW[SwRKPause].Off();
            SW[SwFKHome].Off();
            SW[SwFKStart].Off();
            SW[SwFKSkip].Off();
            SW[SwFKRetry].Off();
            SW[SwFKOneCycle].Off();
            SW[SwFKCleanOut].Off();
            SW[SwFKTrayFeed].Off();
            SW[SwFKTrayEnd].Off();
            SW[SwFKAlarmReset].Off();
        }
        SW[SwRKReset].Off();
        SW[SwRKHome].Off();
        SW[SwRKStart].Off();
        SW[SwRKSkip].Off();
        SW[SwRKRetry].Off();
        SW[SwRKOneCycle].Off();
        SW[SwRKCleanOut].Off();
        SW[SwRKTrayFeed].Off();
        SW[SwRKTrayEnd].Off();
        SW[SwRKAlarmReset].Off();
    }
    //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED -- golden :1838-1883, the REAR
    // arm, is NOT the mirror image of the FRONT arm at :1791-1837, in two ways, and
    // both are reproduced exactly:
    //  (1) SwFKPause CAN BE LEFT STALE.  The front arm drives the cross-panel
    //      SwRKPause in BOTH of its inner branches (golden :1802 OnOff, :1816 Off).
    //      The rear arm drives the cross-panel SwFKPause in its inner IF only
    //      (golden :1849) -- there is no `SW[SwFKPause].Off();` in the inner else
    //      (:1860-1872) and none in the trailing front-clear block (:1873-1882)
    //      either, although that same block clears ten other front-key lamps.  So on
    //      the path bFrontPadActive==false AND NOT(bMotorPowerState && MotorPowerOnDelay
    //      ==0), DoPanelLamp never writes SwFKPause at all and the front Pause lamp
    //      keeps whatever state it last held.  By the symmetry of every other lamp
    //      this is an omission, not a design; it is kept because golden's behaviour
    //      is the specification this port is measured against.
    //  (2) The four Power-lamp writes are in a DIFFERENT ORDER here (golden
    //      :1840-1843: FKPowerOff, FKPowerOn, RKPowerOff, RKPowerOn) than in the two
    //      arms above (golden :1762-1765 and :1793-1796: FKPowerOff, RKPowerOff,
    //      FKPowerOn, RKPowerOn).  Semantically irrelevant -- four independent
    //      points -- but statement order is preserved per the wave rule.
    else
    {
        SW[SwFKPowerOff].OnOff(bLampPowerOff);
        SW[SwFKPowerOn].OnOff(bLampPowerOn);
        SW[SwRKPowerOff].OnOff(bLampPowerOff);
        SW[SwRKPowerOn].OnOff(bLampPowerOn);

        if(bMotorPowerState && MotorPowerOnDelay==0)
        {
            SW[SwRKReset].OnOff(bLampReset);
            SW[SwRKPause].OnOff(bLampPause);
            SW[SwFKPause].OnOff(bLampPause);
            SW[SwRKHome].OnOff(bLampHome);
            SW[SwRKStart].OnOff(bLampStart);
            SW[SwRKSkip].OnOff(bLampSkip);
            SW[SwRKRetry].OnOff(bLampRetry);
            SW[SwRKOneCycle].OnOff(bLampOneCycle);
            SW[SwRKCleanOut].OnOff(bLampCleanOut);
            SW[SwRKTrayFeed].OnOff(bLampTrayFeed);
            SW[SwRKTrayEnd].OnOff(bLampTrayEnd);
        }
        else
        {
            SW[SwRKReset].Off();
            SW[SwRKPause].Off();
            SW[SwRKHome].Off();
            SW[SwRKStart].Off();
            SW[SwRKSkip].Off();
            SW[SwRKRetry].Off();
            SW[SwRKOneCycle].Off();
            SW[SwRKCleanOut].Off();
            SW[SwRKTrayFeed].Off();
            SW[SwRKTrayEnd].Off();
            SW[SwRKAlarmReset].Off();
        }
        SW[SwFKReset].Off();
        SW[SwFKHome].Off();
        SW[SwFKStart].Off();
        SW[SwFKSkip].Off();
        SW[SwFKRetry].Off();
        SW[SwFKOneCycle].Off();
        SW[SwFKCleanOut].Off();
        SW[SwFKTrayFeed].Off();
        SW[SwFKTrayEnd].Off();
        SW[SwFKAlarmReset].Off();
    }

    //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- golden :1885-1893.  Both
    // arms of this if/else run the IDENTICAL statement
    // `SW[SwFrontActiveLed].OnOff(bFrontPadActive);` (golden :1887 and :1892); the
    // only thing iControlPanelMode==1 actually adds is the SwRearActiveLed write at
    // :1888.  Golden could have hoisted the common line and kept a one-statement
    // `if`, so the else arm's asymmetry is real and load-bearing for a test: with
    // iControlPanelMode!=1 the rear Enable LED is never written by DoPanelLamp and
    // holds its previous value.  Golden's 10-space body indent at :1887/:1888/:1892
    // (everything else in this function uses 8) is also kept verbatim.
    if(iControlPanelMode==1)                                                    //Ken 20210702 AddPadInterface
    {
          SW[SwFrontActiveLed].OnOff(bFrontPadActive);
          SW[SwRearActiveLed].OnOff(!bFrontPadActive);                          //KenHsieh 20211221 : Front新增Enable燈號
    }
    else
    {
          SW[SwFrontActiveLed].OnOff(bFrontPadActive);
    }

    //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- `bLampManualSetp` (golden
    // :1895) is golden's own misspelling of "Step"; it is the real global name --
    // golden cmydef.h:2561 and ported cmydef.h:2574 both read
    // `extern bool bLampManualSetp;` -- so the typo is load-bearing and is spelled
    // exactly as golden spells it.  Its sibling on the next line is
    // spelled correctly (bLampManualStart), which is what makes the pair look wrong.
    // Note also that these two rear-panel manual lamps are written UNCONDITIONALLY,
    // outside every arm above -- the SECS/Employee-ID lockout at :1759-1790 does not
    // extinguish them.
    SW[SwRKManualStep].OnOff(bLampManualSetp);
    SW[SwRKManualTStart].OnOff(bLampManualStart);
}
//------------------------------------------------------------------------------
//AI(W906-W7-L2) 20260803: DoSystemMessage -- golden ckernel.cpp:1899-1915, declared
// at ckernel.h:99, translated in FULL and NOT gated.  It is the per-tick display
// scheduler: a 6-phase round robin that fires ShowRunLed()+ShowRunLabel() on phase 0
// and DoPanelLamp() on phase 3, and does nothing on phases 1, 2, 4 and 5.  Golden's
// own trailing comments record why (`//10%CPU`, `//20%CPU`, and `降低CPU負載` on the
// counter): the two display passes are the expensive ones and are deliberately
// de-rated to one tick in six, with the panel-lamp pass offset three ticks away from
// them so the two costs never land on the same tick.
// ShowRunLed (golden :704-932) and ShowRunLabel (golden :935-1726) are this wave's
// DEFERRED stubs, landing ABOVE in this same TU; their W906_ShowRunLed_Count /
// W906_ShowRunLabel_Count counters (ckernel.h:134-136) exist precisely so that this
// scheduler stays falsifiable while the painting is unwritten.  DoPanelLamp is real.
// TESTABILITY -- REPORTED, NOT "FIXED": `iMyCounter` is a FUNCTION-LOCAL STATIC,
// exactly as golden has it (golden :1901), and golden provides NO way to reset it.
// No reset was added: that would be interface golden does not have, and this wave
// does not invent interface.  A test therefore cannot set the phase directly.  It
// can (a) rely on the process-start value 0 -- the very first DoSystemMessage() call
// in a process runs ShowRunLed+ShowRunLabel -- or (b) call
// W906_ShowRunLedLabel_ResetCounts() and then DoSystemMessage() six times and assert
// exactly one ShowRunLed and one ShowRunLabel per six ticks, which is
// phase-independent and therefore robust to any earlier test having ticked it.
void DoSystemMessage()
{
    static int iMyCounter=0;                                                    //Steven 20110608 : 降低CPU負載

    if(iMyCounter==0)
    {
        ShowRunLed();                                                           //10%CPU
        ShowRunLabel();
    }
    else if(iMyCounter==3)
    {
        DoPanelLamp();                                                          //20%CPU
    }
    iMyCounter++;
    if(iMyCounter>=6)
        iMyCounter=0;
}
//------------------------------------------------------------------------------
//AI(W906-W7-L2) 20260803: golden :1917-1918 -- two FILE-SCOPE TQPF_Timer objects.
// These are DEFINITIONS (storage), not declarations, and they sit between
// DoSystemMessage and ScanPannelKey in golden; they are kept in that exact position
// and order.  Their only users live in ScanPannelKey (golden :1919 onwards), which
// is a later fragment of this same TU, so -- as in golden -- no header declaration
// is needed.  TQPF_Timer comes from myTimer.h, added by the prologue block; it has a
// default constructor (`__fastcall TQPF_Timer();`, myTimer.h:24), so these two
// default-construct at static-init time just as they do under BCB6.
// Verified this pass: no other translation unit in the ported tree defines
// SLK1Delay or SLK2Delay, so this fragment owns the storage and there is no ODR
// clash to negotiate.
TQPF_Timer SLK1Delay;                                                           //JerryYang 20160524
TQPF_Timer SLK2Delay;                                                           //JerryYang 20160524
// ===========================================================================
//  ScanPannelKey  --  golden ckernel.cpp:1919-2406 (488 lines; the largest
//  single body in this wave).  The operator-panel key poller.  One call scans
//  the ACTIVE pad only, answers with exactly ONE key id (or -1), and keeps a
//  press latch so a held key fires once instead of on every tick.
//
//  SHAPE -- golden order, preserved statement for statement:
//    :1921-1923  three statics (flag, Task, Task2)
//    :1924-1925  SystemInitialOK guard
//    :1927       bFrontPadActive = Sen[SnRearPadActive].IsOff()  <- pad select
//    :1929-1930  IsSafeLockCheck guard        (ported csystem.h:119)
//    :1932-1933  bEnableEmployeeIDCheck guard (ported cmydef.h:3889)
//    :1935-1945  the bK[64] press-latch array + its one-shot zero init
//    :1947-2123  FRONT-pad arm  (15 top-level key blocks, counted this pass)
//    :2124-2299  REAR-pad arm   (15 top-level key blocks, counted this pass)
//    :2301-2377  SLK1 / SLK2 socket-clamp unclamp-button service -- runs on
//                every call, INDEPENDENT of the key scan, never touches `ret`
//    :2378       iRecordKey=ret
//    :2379-2394  press latch      <-- CARRIES THIS RANGE'S HEADLINE GOLDEN BUG
//    :2395-2404  release sweep over Sen[0..31]
//    :2405       return ret
//
//  FORM DEPENDENCY -- re-verified line by line this pass: EIGHT live `fNote->`
//  sites (fShow at :2007, :2038, :2184, :2215; IsTestSitICFallDown() at :2009,
//  :2040, :2186, :2217) and ZERO live `fMain->` sites.  The two `fMain->`
//  tokens inside this range, at golden :1981 and :2158, sit inside `//` comment
//  text ("Auto Decay 搬移 至 fMain->ScanKey 判斷"); the CODE on those two lines
//  is `if(Sen[SnFKStart].IsOn() || bAseStart)` and its SnRKStart twin.
//
//  SENSOR IDS USED BY THE LATCH ARITHMETIC BELOW -- read this pass from golden
//  cmydef.cpp:789-820 (ported cmydef.cpp carries the same values):
//    SnFKPowerOff 0   SnFKPowerOn 1   SnFKReset 2    SnFKPause 3   SnFKHome 4
//    SnFKStart    5   SnFKOneCycle 6  SnFKRetry 7    SnFKSkip  8
//    SnFKCleanOut 9   SnFKTrayFeed 10 SnFKTrayEnd 11 SnFKAlarmReset 12
//    SnFKCoverOpen 13 SnRKPowerOff 14 SnRKPowerOn 15 SnRKReset 16
//    SnRKPause    17  SnRKHome     18 SnRKStart   19 SnRKOneCycle 20
//    SnRKRetry    21  SnRKSkip     22 SnRKCleanOut 23 SnRKTrayFeed 24
//    SnRKTrayEnd  25  SnRKAlarmReset 26 SnRKCoverOpen 27
//    SnRKManualStep 28  SnRKManualTStart 29
//
//  NAME: golden misspells "Panel" as "Pannel" in the identifier itself (and in
//  golden ckernel.h:17 `int ScanPannelKey();`).  Kept -- it is the published
//  interface name.  CITATION CORRECTED AFTER AUDIT: an earlier draft said
//  ckernel.h:22, which is a real but DIFFERENT declaration
//  (`bool WaitManualStepKey();`) -- the kind of off-by-N that survives a casual
//  re-read because the line it lands on looks plausible.
// ===========================================================================
int ScanPannelKey()
{
    static bool flag=true;
    static int Task=1;                                                          //JerryYang 20160524
    static int Task2=1;                                                         //JerryYang 20160524
    if(SystemInitialOK==false)
        return -1;

    bFrontPadActive=Sen[SnRearPadActive].IsOff();

    if(IsSafeLockCheck())
        return -1;

    if(bEnableEmployeeIDCheck==true)                                            //Ifor 20180927 (Steven) : Add 工號查驗時按鍵不可操作
        return -1;

    static bool bK[64];
    static bool bFlag=true;                                                     //JerryYang 20160524
    static bool bFlag2=true;                                                    //JerryYang 20160524
    static int  iRecordKey=0;                                                   //kevin 20171024 (wei) record key use double push
    int ret, p;
    if(flag)
    {
        for(int i=0; i<64; i++)
            bK[i]=false;
        flag=false;
    }
    ret=-1;
    if(bFrontPadActive)
    {
        if(IniConfig.bDisibleResetButton==false)                                //ChungHung 20111208
        {
            //AI(W906-W7-L2) 20260803: GOLDEN TYPO PRESERVED -- the config field is
            // spelled `bDisibleResetButton` ("Disible", golden Config.h and ported
            // Config.h:154).  Left misspelled; renaming it would break every other
            // reader of the same struct.
            if(Sen[SnFKReset].IsOn() || bAseReset)
            {
                bAseReset=false;                                                //kevin 20150427
                ret=SnFKReset;
            }
        }

        if(Sen[SnFKPause].IsOn() || bAsePause)
        {
            bAsePause=false;
            ret=SnFKPause;
        }

        //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- golden :1964, inside
        // the FRONT-pad arm, polls the REAR pause sensor Sen[SnRKPause] and still
        // answers with the FRONT id SnFKPause.  It is deliberate cross-pad
        // acceptance for Pause, not a copy/paste slip: the REAR arm mirrors it at
        // golden :2141 by polling Sen[SnFKPause].  Note the second consequence --
        // both blocks also test the SAME bAsePause, and :1958 clears it, so :1964
        // can subsequently only fire on the physical rear key.  Preserved as-is.
        if(Sen[SnRKPause].IsOn() || bAsePause)
        {
            bAsePause=false;
            ret=SnFKPause;
        }

        if(Sen[SnFKHome].IsOn() || bAseHome)
        {
            if(bAseHome)
                iAseHome=2;                                                     //kevin 20150925
            else
                iAseHome=0;                                                     //kevin 20150925
            bAseHome=false;
            bPushHomeDetect=true;                                               //kevin 20171006
            ret=SnFKHome;
        }

        if(Sen[SnFKStart].IsOn() || bAseStart)                                  //Ifor 20160114 Auto Decay 搬移 至 fMain->ScanKey 判斷
        {
            bAseStart=false;                                                    //kevin 201050427
            ret=SnFKStart;
        }

        if(Sen[SnFKOneCycle].IsOn() || bAseOneCycle)
        {
            if(bAseOneCycle)
                iAseOneCycle=2;                                                 //kevin 20150925
            else
                iAseOneCycle=0;                                                 //kevin 20150925
            bAseOneCycle=false;
            ret=SnFKOneCycle;
        }

        //AI(W906-W7-L2) 20260803: the FRONT Power-Off / Power-On blocks answer with
        // the FRONT ids SnFKPowerOff(0) / SnFKPowerOn(1), and the front pad adds no
        // offset at golden :2382-2383, so bK[0] / bK[1] are swept against Sen[0] /
        // Sen[1] -- the right sensors.  ONE press yields ONE event here.  Contrast
        // the REAR twins at golden :2174-2182, which are the broken pair; see the
        // latch comment at golden :2379.
        if(Sen[SnFKPowerOff].IsOn())                                            //Alick 20160912 add for矽格北興要求紀錄按下Power Off按鈕
        {
            ret=SnFKPowerOff;
        }

        if(Sen[SnFKPowerOn].IsOn())                                             //kevin 20190328 要求紀錄按下Power On按鈕
        {
            ret=SnFKPowerOn;
        }

        if(W906_FormShowing("fNote", fNote->fShow))                                                        //jou 2011-12-20 修正曾經發生過掉料的話,note訊息沒辦法操作的問題  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
        {
            if(fNote->IsTestSitICFallDown()==false)                             //Steven 20111030
            {
                if(Sen[SnFKRetry].IsOn() || bAseRetry)
                {
                    ret=SnFKRetry;                                              //kevin 2015042
                    bAseRetry=false;                                            //kevin 20150427
                }
            }
            else
            {
                if(Sen[SnFKRetry].IsOn() || bAseRetry)
                {
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown==false)
                    {
                        ret=SnFKRetry;                                          //kevin 2015042
                        bAseRetry=false;                                        //kevin 20150427
                    }
                }
            }
        }
        else
        {
            if(Sen[SnFKRetry].IsOn() || bAseRetry)
            {
                ret=SnFKRetry;                                                  //kevin 2015042
                bAseRetry=false;                                                //kevin 20150427
            }
        }

        if(W906_FormShowing("fNote", fNote->fShow))                                                        //jou 2011-12-20 修正曾經發生過掉料的話,note訊息沒辦法操作的問題  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
        {
            if(fNote->IsTestSitICFallDown()==false)
            {
                if(Sen[SnFKSkip].IsOn() || bAseSKIP)                            //kevin 20150427
                {
                    bAseSKIP=false;                                             //kevin 20150427
                    ret=SnFKSkip;
                }
            }
            else
            {
                if(Sen[SnFKSkip].IsOn() || bAseSKIP)                            //kevin 20150427
                {
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown==false)
                    {
                        bAseSKIP=false;                                         //kevin 20150427
                        ret=SnFKSkip;
                    }
                }
            }
        }
        else
        {
            if(Sen[SnFKSkip].IsOn() || bAseSKIP)                                //kevin 20150427
            {
                bAseSKIP=false;                                                 //kevin 20150427
                ret=SnFKSkip;
            }
        }

        if(Sen[SnFKCleanOut].IsOn() || bAseCleanOut)                            //kevin 20150427
        {
            if(bAseCleanOut)                                                    //kevin 20150925
                iAseCleanOut=2;
            else
                iAseCleanOut=0;
            bAseCleanOut=false;
            ret=SnFKCleanOut;
        }

        if(Sen[SnFKTrayFeed].IsOn() || bAseTrayFeed)
        {
            if(bAseTrayFeed)                                                    //kevin 20150925
                iAseTrayFeed=2;
            else
                iAseTrayFeed=0;
            bAseTrayFeed=false;
            ret=SnFKTrayFeed;
        }

        if(Sen[SnFKTrayEnd].IsOn() || bAseTrayEnd)
        {
            if(bAseTrayEnd)
                iAseTrayEnd=2;                                                  //kevin 20150925
            else
                iAseTrayEnd=0;                                                  //kevin 20150925
            bAseTrayEnd=false;
            ret=SnFKTrayEnd;
            bManualTrayend=true;                                                //kevin 20171026 (wei) 手縱 tray end
            //AI(W906-W7-L2) 20260803: `iRecordKey!=ret` reads the value written at
            // golden :2378 on the PREVIOUS call, because :2378 runs AFTER this line
            // in the same call.  So the test means "this is not a repeat of the key
            // the last call answered with" -- golden's double-push suppression.  It
            // is NOT a self-comparison, even though the write is only 280 lines
            // below the read.  Offline CUSTOMER_CODE is 0 (ported cmydef.cpp:3412)
            // and CC_ASE_KaohSiung is 936 (ported MachineType.h:308), so this arm is
            // closed until a machine config sets the customer code.
            if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
               ArmSpeed[InArm].bAutoSKIP==true &&
               iRecordKey!=ret)                                                 //kevin 20171023 (Steven) auto tray end
            {
                bAutoTrayEndHasIC=false;                                        //kevin 20170906
                bNeedMusicAndAlarmOn=false;                                     //kevin 20170822 add alarm reset
                RecordProcess("OP Trigger_Manual_Trayend");
            }
        }

        //AI(W906-W7-L2) 20260803: GOLDEN ASYMMETRY PRESERVED -- these two FRONT-arm
        // Alarm-Reset blocks each clear FOUR things, the last being
        // `bTesterPauseMusic=false` (golden :2113 and :2121).  Their REAR-arm twins
        // at golden :2286-2298 clear only THREE -- they do NOT touch
        // bTesterPauseMusic.  Consequence: with the rear pad active, pressing Alarm
        // Reset while "Tester Pause" is showing leaves the QORVO tester-pause music
        // latched, which is exactly the case the Steven 20220616 comment says the
        // change was for.  Almost certainly an omission in the rear copy; preserved.
        if((Sen[SnRKAlarmReset].IsOn() && (ret==-1 || !bK[SnFKAlarmReset+(bFrontPadActive?0:SnRKPowerOff)])) || bAseAlarmReset)   //AI(W906-RSTHELD) 20261007: EastSun 1007 a held (stuck) ALARM RESET no longer replaces another key pressed meanwhile (golden: it always wins, so every other panel key is eaten); a fresh RESET press still wins
        {
            bAseAlarmReset=false;
            ret=SnFKAlarmReset;
            bNeedMusicAndAlarmOn=false;                                         //kevin 20170817 (Steven) add
            bTesterPauseMusic=false;  if(bTesterSendPause && TestIF.iMaxTime>0) { hPauseAlarmDelay.SetSecAndOn(TestIF.iMaxTime); bPauseAlarmDelayActive=true; }  if(bN07AlarmActive) bN07BuzzerSilenced=true;  /*AI(W906-C15-N07) 20261003 (St02-E, claim): golden 906_0625_Steven ckernel.cpp:2128 (Steven 20260603 : silence N07 buzzer on alarm reset); same line*/  /* AI(W906-GB-P2c) 20260926: golden 912 ckernel.cpp:2144-2148 / :2158-2162 -- Alarm Reset while still paused re-arms the delay (Timer ReStart) */                                            //Steven 20220616 : Can select "Alarm Reset" when show "Tester Pause" for QORVO.
        }

        if((Sen[SnFKAlarmReset].IsOn() && (ret==-1 || !bK[SnFKAlarmReset+(bFrontPadActive?0:SnRKPowerOff)])) || bAseAlarmReset)   //AI(W906-RSTHELD) 20261007: EastSun 1007 a held (stuck) ALARM RESET no longer replaces another key pressed meanwhile (golden: it always wins, so every other panel key is eaten); a fresh RESET press still wins
        {
            bAseAlarmReset=false;
            ret=SnFKAlarmReset;
            bNeedMusicAndAlarmOn=false;                                         //kevin 20170817 (Steven) add
            bTesterPauseMusic=false;  if(bTesterSendPause && TestIF.iMaxTime>0) { hPauseAlarmDelay.SetSecAndOn(TestIF.iMaxTime); bPauseAlarmDelayActive=true; }  if(bN07AlarmActive) bN07BuzzerSilenced=true;  /*AI(W906-C15-N07) 20261003 (St02-E, claim): golden 906_0625_Steven ckernel.cpp:2137 (Steven 20260603 : silence N07 buzzer on alarm reset); same line*/  /* AI(W906-GB-P2c) 20260926: golden 912 ckernel.cpp:2144-2148 / :2158-2162 -- Alarm Reset while still paused re-arms the delay (Timer ReStart) */                                            //Steven 20220616 : Can select "Alarm Reset" when show "Tester Pause" for QORVO.
        }
    }
    else
    {
        //AI(W906-W7-L2) 20260803: THE REAR ARM.  Read golden :2126-:2298 with the
        // latch at :2379-2394 in hand: THIRTEEN of the fifteen top-level blocks
        // below answer with a FRONT id even though they poll a REAR sensor -- and
        // between them they use exactly ELEVEN distinct front ids (:2131 SnFKReset,
        // :2138/:2144 SnFKPause, :2155 SnFKHome, :2161 SnFKStart, :2171
        // SnFKOneCycle, :2190/:2200/:2210 SnFKRetry, :2222/:2232/:2242 SnFKSkip,
        // :2253 SnFKCleanOut, :2264 SnFKTrayFeed, :2274 SnFKTrayEnd,
        // :2289/:2296 SnFKAlarmReset).  That is not sloppiness -- it is what makes
        // the `p+=SnRKPowerOff` (+14) at golden :2383 land on exactly the matching
        // REAR sensor index: 2+14=16=SnRKReset, 3+14=17=SnRKPause, 4+14=18=SnRKHome,
        // 5+14=19=SnRKStart, 6+14=20=SnRKOneCycle, 7+14=21=SnRKRetry,
        // 8+14=22=SnRKSkip, 9+14=23=SnRKCleanOut, 10+14=24=SnRKTrayFeed,
        // 11+14=25=SnRKTrayEnd, 12+14=26=SnRKAlarmReset.  The ONLY two blocks that
        // answer with an already-REAR id are Power-Off/Power-On at :2174-:2182, and
        // they are the ones that break.  Callers must know that a rear Start press
        // is reported as SnFKStart, never SnRKStart -- golden behaviour, preserved.
        if(IniConfig.bDisibleResetButton==false)                                //ChungHung 20111208
        {
            if(Sen[SnRKReset].IsOn() || bAseReset)
            {
                bAseReset=false;                                                //kevin 20150427
                ret=SnFKReset;
            }
        }

        if(Sen[SnRKPause].IsOn() || bAsePause)
        {
            bAsePause=false;
            ret=SnFKPause;
        }

        if(Sen[SnFKPause].IsOn() || bAsePause)
        {
            bAsePause=false;
            ret=SnFKPause;
        }

        if(Sen[SnRKHome].IsOn()||bAseHome)
        {
            //AI(W906-W7-L2) 20260803: GOLDEN ASYMMETRY PRESERVED -- the REAR Home
            // block sets iAseHome=1 (golden :2150) where the FRONT twin sets
            // iAseHome=2 (golden :1973).  Both are reached only when bAseHome is
            // true (an ASE remote Home), and the pad that happens to be active then
            // changes the code the consumer sees.  Same author, same day (kevin
            // 20150925).  Deliberate or not, it is golden; not normalised.
            if(bAseHome)
                iAseHome=1;                                                     //kevin 20150925
            else
                iAseHome=0;                                                     //kevin 20150925
            bAseHome=false;
            bPushHomeDetect=true;                                               //kevin 20171006
            ret=SnFKHome;
        }

        if(Sen[SnRKStart].IsOn() || bAseStart)                                  //Ifor 20160114 Auto Decay 搬移 至 fMain->ScanKey 判斷
        {
            bAseStart=false;                                                    //kevin 201050427
            ret=SnFKStart;
        }

        if(Sen[SnRKOneCycle].IsOn() || bAseOneCycle)
        {
            if(bAseOneCycle)
                iAseOneCycle=2;                                                 //kevin 20150925
            else
                iAseOneCycle=0;                                                 //kevin 20150925
            bAseOneCycle=false;
            ret=SnFKOneCycle;
        }

        //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED (setup half) -- these two are
        // the ONLY blocks in the REAR arm that answer with a REAR id
        // (SnRKPowerOff=14 / SnRKPowerOn=15) instead of the corresponding FRONT id.
        // The latch at golden :2382-2383 then adds SnRKPowerOff a SECOND time,
        // producing 28 (==SnRKManualStep) and 29 (==SnRKManualTStart).  Full
        // consequence written out at the latch below.  DO NOT change these to
        // SnFKPowerOff / SnFKPowerOn -- that would "fix" the bug and break the
        // regression test that pins the auto-repeat.
        if(Sen[SnRKPowerOff].IsOn())                                            //Alick 20160912 add for矽格北興要求紀錄按下Power Off按鈕
        {
            ret=SnRKPowerOff;
        }

        if(Sen[SnRKPowerOn].IsOn())                                             //kevin 20190328 要求紀錄按下Power On按鈕
        {
            ret=SnRKPowerOn;
        }

        if(W906_FormShowing("fNote", fNote->fShow))                                                        //jou 2011-12-20 修正曾經發生過掉料的話,note訊息沒辦法操作的問題  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
        {
            if(fNote->IsTestSitICFallDown()==false)                             //Steven 20111030   //jou 2011-12-23 解除不是Index alarm也需要按Z1的問題
            {
                if(Sen[SnRKRetry].IsOn() || bAseRetry)
                {
                    ret=SnFKRetry;                                              //kevin 2015042
                    bAseRetry=false;                                            //kevin 20150427
                }
            }
            else
            {
                if(Sen[SnRKRetry].IsOn() || bAseRetry)
                {
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown==false)
                    {
                        ret=SnFKRetry;                                          //kevin 2015042
                        bAseRetry=false;                                        //kevin 20150427
                    }
                }
            }
        }
        else
        {
            if(Sen[SnRKRetry].IsOn() || bAseRetry)                              // ret=SnFKRetry;
            {
                ret=SnFKRetry;                                                  //kevin 2015042
                bAseRetry=false;                                                //kevin 20150427
            }
        }

        if(W906_FormShowing("fNote", fNote->fShow))                                                        //jou 2011-12-20 修正曾經發生過掉料的話,note訊息沒辦法操作的問題  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
        {
            if(fNote->IsTestSitICFallDown()==false)                             //jou 2011-12-23 解除不是Index alarm也需要按Z1的問題
            {
                if(Sen[SnRKSkip].IsOn() || bAseSKIP)
                {
                    bAseSKIP=false;                                             //kevin 2015042
                    ret=SnFKSkip;                                               //kevin 2015042
                }
            }
            else
            {
                if(Sen[SnRKSkip].IsOn() || bAseSKIP)
                {
                    if(IniConfig.bD40IndexICFallDownMustPressFMotorDown==false)
                    {
                        //AI(W906-W7-L2) 20260803: golden :2231-:2232 are indented by
                        // 25 spaces where every sibling uses 24.  Whitespace slip in
                        // golden; reproduced exactly so a column-sensitive diff of
                        // this range against golden stays clean.
                         bAseSKIP=false;                                        //kevin 2015042
                         ret=SnFKSkip;                                          //kevin 20150429
                    }
                }
            }
        }
        else
        {
            if(Sen[SnRKSkip].IsOn() || bAseSKIP)
            {
                bAseSKIP=false;                                                 //kevin 2015042
                ret=SnFKSkip;                                                   //kevin 20150429
            }
        }

        if(Sen[SnRKCleanOut].IsOn() || bAseCleanOut)
        {
            if(bAseCleanOut)                                                    //kevin 20150925
                iAseCleanOut=2;
            else
                iAseCleanOut=0;
            bAseCleanOut=false;
            ret=SnFKCleanOut;
        }

        if(Sen[SnRKTrayFeed].IsOn() || bAseTrayFeed)
        {
            if(bAseTrayFeed)                                                    //kevin 20150925
                iAseTrayFeed=2;
            else
                iAseTrayFeed=0;

            bAseTrayFeed=false;
            ret=SnFKTrayFeed;
        }

        if(Sen[SnRKTrayEnd].IsOn() || bAseTrayEnd)
        {
            if(bAseTrayEnd)
                iAseTrayEnd=2;                                                  //kevin 20150925
            else
                iAseTrayEnd=0;                                                  //kevin 20150925
            bAseTrayEnd=false;
            ret=SnFKTrayEnd;
            bManualTrayend=true;                                                //kevin 20171026 (wei) 手縱 tray end
            if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
               ArmSpeed[InArm].bAutoSKIP==true &&
               iRecordKey!=ret)                                                 //kevin 20171023 (Steven) auto tray end
            {
                bAutoTrayEndHasIC=false;                                        //kevin 20170906
                bNeedMusicAndAlarmOn=false;                                     //kevin 20170822 add alarm reset
                RecordProcess("OP Trigger_Manual_Trayend");
            }
        }

        if((Sen[SnRKAlarmReset].IsOn() && (ret==-1 || !bK[SnFKAlarmReset+(bFrontPadActive?0:SnRKPowerOff)])) || bAseAlarmReset)   //AI(W906-RSTHELD) 20261007: EastSun 1007 a held (stuck) ALARM RESET no longer replaces another key pressed meanwhile (golden: it always wins, so every other panel key is eaten); a fresh RESET press still wins
        {
            bAseAlarmReset=false;
            ret=SnFKAlarmReset;
            bNeedMusicAndAlarmOn=false;                                         //kevin 20170817 (Steven) add
        }

        if((Sen[SnFKAlarmReset].IsOn() && (ret==-1 || !bK[SnFKAlarmReset+(bFrontPadActive?0:SnRKPowerOff)])) || bAseAlarmReset)   //AI(W906-RSTHELD) 20261007: EastSun 1007 a held (stuck) ALARM RESET no longer replaces another key pressed meanwhile (golden: it always wins, so every other panel key is eaten); a fresh RESET press still wins
        {
            bAseAlarmReset=false;
            ret=SnFKAlarmReset;
            bNeedMusicAndAlarmOn=false;                                         //kevin 20170817 (Steven) add
        }
    }

    //AI(W906-W7-L2) 20260803: golden :2301-:2377 -- the SLK socket-clamp service.
    // Structurally NOT part of the key scan: it sits after both pad arms, runs on
    // every call regardless of which pad is active, and never reads or writes
    // `ret`.  Its only outputs are the four C_SLK*_Clamp / C_SLK*_Unclamp
    // cylinders.  Kept in golden's position (before iRecordKey / the latch) --
    // moving it would be a source-order deviation even though nothing observable
    // depends on where it runs.
    if(INSTALL_SOCKET_CLAMP)                                                    //JerryYang 20160601 如果機台硬體支援分離測試才進來
    {
        if(Sen[SnHeaterDoor].IsOff()==true &&
           Sen[SnHeaterDoor2].IsOff()==true &&
           SystemStart==false)                                                  //JerryYang 20160601
        {
            //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED -- bFlag reads like a
            // press-edge latch but is not one.  When the button is held, the `if`
            // clears bFlag, and on the very next call the condition is false so the
            // `else` at golden :2337-:2340 puts bFlag straight back to true.  So
            // bFlag alternates every scan while the button is DOWN; it never blocks
            // a held button.  The real rate limit is the 3-second SLK1Delay driven
            // by the Task=1 -> Task=10 pair, which is what the Chinese comment on
            // golden :2314 promises ("每三秒才能控制一次").  Net effect: holding the
            // unclamp button toggles the clamp roughly every 3 s, forever.
            // Translated verbatim -- no debounce added.
            if(Sen[SnSLK1UnclampButton].IsOn() && bFlag)                        //JerryYang 20160601 偵測到Arm1 SLK夾持汽缸伸出/縮回按鈕被按下
            {
                bFlag=false;
                switch(Task)
                {
                    case 1:
                        {
                            SLK1Delay.SetSecAndOn(3);                           //JerryYang 20160601 每三秒才能控制一次ARM1 SLK夾持汽缸伸出/縮回
                            //AI(W906-W7-L2) 20260803: note the ASYMMETRIC test --
                            // the first arm fires when Clamp's sensor is ON *or*
                            // Unclamp's sensor is OFF (i.e. also on the "neither
                            // sensor made" ambiguous state), so an un-sensed cylinder
                            // defaults to being driven to UNCLAMP.  Only the fully
                            // unambiguous Unclamp==ON case takes the second arm.
                            // Golden's choice; preserved.
                            if(Cylinder[C_SLK1_Clamp].OnSensor()==true ||
                               Cylinder[C_SLK1_Unclamp].OnSensor()==false)
                            {
                                Cylinder[C_SLK1_Clamp].Off();
                                Cylinder[C_SLK1_Unclamp].On();
                            }
                            else if(Cylinder[C_SLK1_Unclamp].OnSensor()==true)
                            {
                                Cylinder[C_SLK1_Unclamp].Off();
                                Cylinder[C_SLK1_Clamp].On();
                            }
                            Task=10;
                        }
                        break;
                    case 10:
                        if(SLK1Delay.Off())
                        {
                            Task=1;
                        }
                        break;
                }
            }
            else
            {
                bFlag=true;
            }

            if(Sen[SnSLK2UnclampButton].IsOn() && bFlag2)                       //JerryYang 20160601 偵測到Arm2 SLK夾持汽缸伸出/縮回按鈕被按下
            {
                bFlag2=false;
                switch(Task2)
                {
                    case 1:
                        {
                            SLK2Delay.SetSecAndOn(3);                           //JerryYang 20160601 每三秒才能控制一次ARM2 SLK夾持汽缸伸出/縮回
                            if(Cylinder[C_SLK2_Clamp].OnSensor()==true ||
                               Cylinder[C_SLK2_Unclamp].OnSensor()==false)
                            {
                                Cylinder[C_SLK2_Clamp].Off();
                                Cylinder[C_SLK2_Unclamp].On();
                            }
                            else if(Cylinder[C_SLK2_Unclamp].OnSensor()==true)
                            {
                                Cylinder[C_SLK2_Unclamp].Off();
                                Cylinder[C_SLK2_Clamp].On();
                            }
                            Task2=10;
                        }
                        break;
                    case 10:
                        if(SLK2Delay.Off())
                        {
                            Task2=1;
                        }
                        break;
                }
            }
            else
            {
                bFlag2=true;
            }
        }
    }
    iRecordKey=ret;                                                             //kevin 20171024 (wei) record key use double push
    //AI(W906-W7-L2) 20260803: ***GOLDEN BUG PRESERVED -- THE HEADLINE ONE FOR THIS
    // RANGE.***  The rear Power-Off / Power-On keys DOUBLE-OFFSET their latch index
    // and therefore AUTO-REPEAT while held.
    //
    // golden :2382-:2383 turns a FRONT id into the matching REAR sensor index by
    // adding SnRKPowerOff (=14, golden cmydef.cpp:804) whenever the rear pad is the
    // active one.  Correct for the eleven distinct FRONT ids the rear arm answers
    // with (see the map in the rear-arm comment at golden :2126).
    // But golden :2176 answers `ret=SnRKPowerOff` (14) and golden :2181 answers
    // `ret=SnRKPowerOn` (15) -- ids that are ALREADY rear -- so:
    //       p = 14 + 14 = 28 == SnRKManualStep    (golden cmydef.cpp:819)
    //       p = 15 + 14 = 29 == SnRKManualTStart  (golden cmydef.cpp:820)
    // The press is latched in bK[28] / bK[29], but the release sweep at
    // golden :2395-:2404 asks `Sen[i].IsOn()` for the SAME index -- Sen[28] /
    // Sen[29] are the manual-step keys, NOT the key being held -- so the latch is
    // cleared on the very next call.  Observable sequence while the rear Power-Off
    // key is held down with no release in between:
    //       call 1 -> SnRKPowerOff  (bK[28] set; the early `return ret` below skips
    //                                the release sweep entirely on this call)
    //       call 2 -> -1            (bK[28] already true -> ret forced to -1; the
    //                                sweep then clears bK[28] because Sen[28] is off)
    //       call 3 -> SnRKPowerOff  (again) ... repeating on every odd call.
    // The FRONT Power-Off key is unaffected (golden :1999 answers SnFKPowerOff=0,
    // no offset, bK[0] swept against Sen[0]).
    //
    // TWO SMALLER ODDITIES IN THE SAME EIGHT LINES, also preserved:
    //   (a) the `return ret` at golden :2388 short-circuits the release sweep, so a
    //       latched-then-released key is only cleared on a later call in which NO
    //       key fires;
    //   (b) bK is 64 wide (golden :1935) but the sweep only walks 0..31 (golden
    //       :2395).  Harmless as written -- the largest index this function can
    //       ever produce is 29 -- but it means a future id above 31 would latch
    //       permanently.
    // DO NOT "CORRECT" ANY OF THIS.
    if(ret!=-1)
    {
        p=ret;
        if(bFrontPadActive==false)
            p+=SnRKPowerOff;

        if(bK[p]==false)
        {
            bK[p]=true;
            return ret;
        }
        else
        {
            ret=-1;
        }
    }
    for(int i=0; i<32; i++)
    {
        if(bK[i])
        {
            if(Sen[i].IsOn()==false)
            {
                bK[i]=false;
            }
        }
    }
    return ret;
}
//------------------------------------------------------------------------------
// ===========================================================================
//  DoScanLevelSensor  --  golden ckernel.cpp:2408-2430.  Reads the four
//  front-panel fingerprint / access-level sensors and returns the level index:
//    0 = Ope(rator), 1 = Eng(ineer), 2 = Sup(ervisor), 3 = Hon(prec), -1 = none.
//
//  PRIORITY IS BY SOURCE ORDER and the chain is `else if`, so the FIRST sensor
//  that is both Enable and IsOn() wins: Ope beats Eng beats Sup beats Hon.  If
//  two badges are somehow presented at once, the LOWER privilege level is what
//  comes back.  Order preserved exactly (golden :2413, :2417, :2421, :2425).
//  Both `Enable` and `IsOn()` are read, in that order, on every branch --
//  preserved, because `Enable` short-circuits `IsOn()` for unwired sensors.
// ===========================================================================
int DoScanLevelSensor()                                                         //Steven 20190503 : 指紋辨識權限
{
    int iRet=-1;
    //AI(W906-W7-L2) 20260803: GOLDEN DEAD CODE PRESERVED -- `bFlag` is initialised
    // false on golden :2411 and is NEVER assigned again anywhere in the function,
    // so the `bFlag==true ||` disjunct that opens all four branches (golden :2413,
    // :2417, :2421, :2425) can never be taken and every branch reduces to its
    // sensor test.  It is a left-in force-override switch: flipping the initialiser
    // to true would make the function always return 0 (top-priority Ope level).
    // Kept verbatim -- including the initialiser -- because deleting it would
    // silently remove that switch, and because g++ folds it away regardless.
    bool bFlag=false;

    if(bFlag==true || (Sen[SnFPLevelOpe].Enable && Sen[SnFPLevelOpe].IsOn()))
    {
        iRet=0;
    }
    else if(bFlag==true || (Sen[SnFPLevelEng].Enable && Sen[SnFPLevelEng].IsOn()))
    {
        iRet=1;
    }
    else if(bFlag==true || (Sen[SnFPLevelSup].Enable && Sen[SnFPLevelSup].IsOn()))
    {
        iRet=2;
    }
    else if(bFlag==true || (Sen[SnFPLevelHon].Enable && Sen[SnFPLevelHon].IsOn()))
    {
        iRet=3;
    }
    return iRet;
}
//------------------------------------------------------------------------------
// =============================================================================
//  GetMotorAlarmCode  --  golden ckernel.cpp:2432-2495
//
//  Per-motor alarm decoder.  Given the component that raised an alarm, find its
//  slot in MOT[], re-scan its status, turn the driver error index into a JAM/WAR
//  string and hand it to the note dialog.  Called from exactly ONE place:
//  ProcessAlarm's ALM_MOTOR_MOVE arm at golden :2514, immediately below.
//
//  SIGNATURE DEVIATION -- pinned by the integrator, and re-verified here rather
//  than taken on trust:
//      golden :2432   void GetMotorAlarmCode(TComponent *Comp)
//      ported         void GetMotorAlarmCode(HTMotor    *Comp)
//  The ONLY thing golden ever does with the parameter is the identity compare at
//  golden :2440, `if(MOT[i].Motor==Comp && MOT[i].Motor->Enable)`.  `MOT[i].Motor`
//  is `HTMotor *` (ported Motor/mymotor.h:139).  Golden's `class HTMotor` is
//  declared at golden Motor/HTMotor.h:31 -- read this pass: the head is a bare
//  `class HTMotor` with NO base-clause (:31-:33 are `class HTMotor` / `{` /
//  `private:`), and a scan of all 149 lines of that file for the token `operator`
//  returns ZERO hits, so there is no conversion operator either.  `HTMotor* ==
//  TComponent*` is therefore a comparison between
//  unrelated pointer types and does NOT type-check under standard C++.  Golden
//  gets away with it because BCB6 accepted it; the port cannot reproduce a line
//  that does not compile.  Retyping the parameter preserves the one semantic that
//  matters -- pointer identity of the alarming motor -- and lets golden :2440 be
//  written verbatim with no cast.  The already-landed alarm-queue seam uses the
//  identical retarget (`bool PopUpAlarm(HTMotor **ppComp, int &iCode)`,
//  canary_support.h:502, rationale at canary_support.h:452-473), so ProcessAlarm
//  below hands this function a pointer of exactly this type.
//
//  NOT DECLARED IN ckernel.h.  Golden ckernel.h does not declare it either (see
//  the ported ckernel.h banner, :54-62); the sole call site sits BELOW this
//  definition, so no forward declaration is required and adding one would invent
//  interface golden does not have.
// =============================================================================
void GetMotorAlarmCode(void *Comp)                                              // golden ckernel.cpp:2432 -- `TComponent *Comp`  //AI(W906-HALARM) 20260926: 改成 void*（halarm.h 檔頭）：真的 HAlarm 只有一個全域 Alarm，Parent 是 fMain 不是馬達，所以 :2440 的比對在 golden 裡本來就永遠不成立；HTMotor*==void* 可直接比
{
    //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED (golden :2434) -- `iCount` is a
    // WRITE-ONLY static.  It is initialised here and zeroed at golden :2459 and
    // golden :2464, but NOTHING ever reads it and NOTHING ever increments it: an
    // identifier scan over all 2589 cp950-decoded lines of golden ckernel.cpp
    // returns exactly three occurrences -- :2434, :2459, :2464 -- all of them
    // writes.  Both `else` arms that feed it are therefore pure no-ops.  It reads
    // like a repeat-suppression counter for the WAR240004 row that was never
    // finished.  Kept verbatim, dead writes and all.  (Expect a
    // -Wunused-but-set-variable style diagnostic; that is golden's shape.)
    static int iCount=0;
    int i, iRef;
    AnsiString str1;

    for(i=0; i<TOTAL_MOTOR; i++)
    {
        //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED (golden :2440) -- the guard
        // dereferences `MOT[i].Motor->Enable` with no null check.  It is safe only
        // because the left operand of `&&` already proved `MOT[i].Motor==Comp`; a
        // NULL `Comp` matching an unpopulated `MOT[i].Motor` would dereference
        // null.  Golden's queue only ever carries the object that alarmed, so the
        // case does not arise in practice.  The missing guard is golden's and is
        // NOT added here.
        if(MOT[i].Motor==Comp && MOT[i].Motor->Enable)     //pig 2016.01.30 MotorAlarm
        {
            MOT[i].ScanMotorStatus();
            iRef=MOT[i].GetErrorIndex();

            if(CUSTOMER_CODE==CC_JCET)  //Steven 20200312
            {
                if(i==0)
                {
                    if(iRef==3)
                    {
                        //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED (golden :2451)
                        // -- the format string reads `iCcwLed%d` while both of its
                        // neighbours are `name:%d` (`iAlarmLed:%d`, `Pos:%d`).  The
                        // missing colon is golden's and ships verbatim into the
                        // WAR240004 Motion row, so log parsing keyed on the emitted
                        // text keeps working.  Correcting it would change logged
                        // output.  (See the project memory note on WAR240004 having
                        // two emit paths -- this is the customer-independent one.)
                        str1.sprintf("In Arm X CCW alarm, iAlarmLed:%d, iCcwLed%d, Pos:%d",  MOT[i].Led[iAlarmLed]?1:0, MOT[i].Led[iCcwLed]?1:0, MOT[i].ReadEncoderPos());
                        MyDBIProcessNew("Motion", "WAR240004", str1, "GetMotorAlarmCode");
                        MOT[i].Led[iAlarmLed]=false;
                        MOT[i].Led[iCcwLed]=false;
                        MOT[i].Led[iSoftccwLed]=false;
                    }
                    else
                    {
                        iCount=0;
                    }
                }
                else
                {
                    iCount=0;
                }
            }

            //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED (golden :2468-2469) -- the
            // `iRef==9 -> iRef=7` remap.  MECHANISM CORRECTED AFTER AUDIT: an earlier
            // draft said 9 and 7 were the Led[] enum constants iServoOn and
            // iServoalarmLed.  They are not.  GetErrorIndex (golden
            // Motor/mymotor.cpp:1798-1819, read cp950 this pass) returns a DIAGNOSIS
            // code from its own value space, not a Led[] index: 0..5 are Alarm+X
            // combinations, 6 is `Led[iCwLed] || Led[iCcwLed]`, **7 is
            // `Led[iAlarmLed] && Motor->PServoAlarmOn`** (:1811-1812), and **9 is the
            // terminal catch-all `else return 9;`** (:1818) meaning "no recognised
            // error pattern".  So the remap relabels UNDIAGNOSED as SERVO ALARM.  The remap
            // is load-bearing for the code ShowMotorErrorMessage finally receives
            // (`iRef+1`), and it is reproduced exactly -- here and again in the
            // Galil arm below, where golden repeats it at :2488-2489.
            if(iRef==9)
                iRef=7;
            JamCode=MotorIndexToJamCode(i);
            ShowMotorErrorMessage(JamCode, iRef+1);
        }

        //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED (golden :2474) -- this second
        // arm is NOT gated on `MOT[i].Motor==Comp`.  It is a bare index-motor sweep
        // that runs on EVERY iteration of the loop regardless of which component
        // actually alarmed, so a call about any motor at all will also raise a Galil
        // index alarm if one is latched.  It also re-asserts `SoftStop=true` (:2476)
        // after ProcessAlarm has just cleared it at golden :2509.  Both behaviours
        // are golden's and are reproduced as-is.
        if(INDEX_MOTION_CARD==0 && (i==MTestY1 || i==MTestZ1 || i==MTestZ2 || i==MTestY2) && MOT[i].Gali_MotorAlarm && MotorPowerOnDelay==0)    //Steven 20210623 : Index使用Galil
        {
            SoftStop=true;
            MOT[i].MovFlag=false;
            MOT[MTestY1].bScanFlag=false;
            MOT[MTestZ1].bScanFlag=false;
            MOT[MTestZ2].bScanFlag=false;
            MOT[MTestY2].bScanFlag=false;
            MOT[MTestY1].GaliSofDelayCount=0;
            MOT[MTestZ1].GaliSofDelayCount=0;
            MOT[MTestZ2].GaliSofDelayCount=0;
            MOT[MTestY2].GaliSofDelayCount=0;
            MOT[i].Gali_ScanMotStatus();
            iRef = MOT[i].GetErrorIndex();
            if(iRef==9)
                iRef=7;
            JamCode = MotorIndexToJamCode(i);
            ShowMotorErrorMessage(JamCode, iRef+1);
        }
    }
    //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED (golden :2494) -- a commented
    // out `return 0;` left over from when this function returned int.  It is dead
    // text in a void function; carried across because it is part of golden's line.
    //return 0;
}
//==============================================================================
// only process Motor and Cylinder error
//==============================================================================
//AI(W906-W7-L2) 20260803: golden :2499 defines ALM_MOTOR_MOVE, and so does
// golden Motor/HTMotor.h:7 (`#define ALM_MOTOR_MOVE  55555`, read this pass) --
// which golden ckernel.cpp already pulls in through golden :12 `Motor/mymotor.h`.
// So GOLDEN ITSELF redefines the macro here.  The ported tree is in the same
// shape: Motor/HTMotor.h:29 has the identical `#define ALM_MOTOR_MOVE  55555`,
// reached through ported Motor/mymotor.h:66, which the prologue includes.  The
// line is kept because it is golden's; a redefinition whose replacement list is
// token-identical is legal C++ [cpp.replace]/2 and produces NO diagnostic --
// verified by the probe compile, which is warning-clean on this line.
// (canary_support.h:480-482 records the matching decision not to define it there.)
#define     ALM_MOTOR_MOVE  55555
//氣缸 Error is 自動加 10000
// =============================================================================
//  ProcessAlarm  --  golden ckernel.cpp:2501-2527.  Declared golden ckernel.h:19
//  / ported ckernel.h:94.  Drains the alarm FIFO and routes each entry to either
//  the MOTOR path (GetMotorAlarmCode, above) or the CYLINDER path
//  (CylinderIndexToJamCode -> ShowErrorMessage), then clears every registered
//  alarm object.
//
//  THE CONTRACT THIS CODES AGAINST -- both golden bodies read this pass, in the
//  sibling BCB6 component package that is not part of the translated tree:
//    D:/HT9045/elec/Component/HAlarm.cpp:264-283  `PopUpAlarm(TComponent **, int&)`.
//      Guarded on `ShowAlarmList->Count>0` (:266); on a hit it takes HALBusy +
//      ThMutex (:268-270), reads `ERR_MSG *P = ShowAlarmList->Items[0]` (:271),
//      writes `*Component = P->ObjPtr` (:273, // 指向錯誤物件) and
//      `iErrCode = P->iErrCode` (:274, // 傳回錯誤碼), `Delete(0)` + `delete P`
//      (:275-276), releases and returns true (:279).  On an empty list it falls
//      to `return false` (:282) WITHOUT touching either out-param.
//    D:/HT9045/elec/Component/HAlarm.cpp:234-242  `ClearAllAlarm()`.  Walks
//      HAlarmList calling `P->Clear()` on each entry (:236-240), then
//      `SystemNG=false` (:241).
//  The ported stand-ins are canary_support.h:502 and :505.  Their two documented
//  divergences -- NO dedup (golden's push side, HAlarm::Set, drops a code already
//  outstanding on that component, HAlarm.cpp:112) and NO HALBusy/ThMutex
//  serialisation -- are NOT compensated for in this function.  Compensating here
//  would move queue policy into ProcessAlarm, which is not where golden keeps it,
//  and would make the translation diverge from golden :2507 for a seam problem.
//
//  DEVIATION, forced by that seam (same root cause as GetMotorAlarmCode above):
//  golden :2503 declares `TComponent *Comp;` to match golden's PopUpAlarm
//  out-param type.  Ported PopUpAlarm takes `HTMotor **`, so the local is
//  `HTMotor *Comp;`.  Pointer LEVEL and every use of the variable are unchanged.
// =============================================================================
void ProcessAlarm()
{
    void *Comp;         // golden :2503 -- golden spells this `TComponent *Comp;`  //AI(W906-HALARM) 20260926: 跟著 PopUpAlarm(void**) 改型別（halarm.h:37）
    int iCode;
    //AI(W906-W7-L2) 20260803: `Pos` is left UNINITIALISED exactly as golden :2505.
    // Safe by contract, not by luck: CylinderIndexToJamCode ALWAYS writes through
    // the pointer -- golden note.cpp's final else (:4277-4280) defaults it to
    // MMSystem -- and the ported mapper preserves that (canary_support.h:358-360).
    int Pos;
    AnsiString sRef;
    while(PopUpAlarm(&Comp, iCode))
    {
        SoftStop=false;
        SoftStart=false;
        ScanSystemSensor();
        if(iCode==ALM_MOTOR_MOVE)
        {
            GetMotorAlarmCode(Comp);
            fAllMotorHome=false;        //發生馬達錯誤
            //Ifor 20151208 :矽品新增馬達異常 Show Alarm Message
            if(IniConfig.bSPILFunction==true || CUSTOMER_CODE==CC_SIGURD_PeiXing || CUSTOMER_CODE==CC_ASE_KaohSiung)//kevin 20180319 add //JerryYang 20160328 for 矽格北興,觸發回home的地方補上log  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                ShowMyMessage("Motor Move error, Need home"); //Ifor 20151208 :Add Motor Home Massage
        }
        else
        {
            sRef = CylinderIndexToJamCode(iCode, &Pos);
            ShowErrorMessage(sRef, K_RETRY, Pos);
        }
    }
    ClearAllAlarm();
}
//------------------------------------------------------------------------------
// =============================================================================
//  InitialTestDelayStatus  --  golden ckernel.cpp:2529-2587.  Declared golden
//  ckernel.h:25 / ported ckernel.h:100.  Returns the one-line reason string shown
//  on the main form while the first-device test start is being held off.
//
//  GOLDEN BUG PRESERVED -- THE "PRIORITY" IS PURE SOURCE ORDER.  These are ELEVEN
//  independent `if`s, not an `if/else if` ladder, and every arm assigns `str`
//  unconditionally.  So when several delay flags are set at once the LAST arm in
//  source order wins, and the effective precedence is the reverse of the reading
//  order: [10] SOT beats [8] EOT beats [9] OTD beats [7] no-fullSite beats [1]
//  first device beats [5] beats [4] beats [2] beats [3] beats [6] beats [11].
//  That puts golden :2532's [11] -- the newest arm, added by Steven 20221214 and
//  commented 主畫面顯示 -- at the very BOTTOM of the effective priority, which is
//  almost certainly not what adding it at the top was meant to achieve.  The
//  statement order is reproduced EXACTLY; converting it to an else-if ladder or
//  reordering it would silently change which message the operator sees.
//
//  Also preserved: golden tests only ELEVEN of the fifteen slots.  bInitialTestDelayStatus
//  is `bool[15]` (ported cmydef.h:4414, //Steven 20200730 : 10 --> 15) and indices
//  0, 12, 13 and 14 are never read here -- so a delay latched in one of those four
//  produces an EMPTY status string.  Golden's gap, kept.
// =============================================================================
AnsiString InitialTestDelayStatus()     //wei 20171020 (jou) 延遲狀態顯示
{
    AnsiString str="";
    if(bInitialTestDelayStatus[11])     //Steven 20221214 : Add 主畫面顯示
    {
        str="Initial delay--By KL";
    }

    if(bInitialTestDelayStatus[6])
    {
        str="Initial delay--Press stop time over";
    }

    if(bInitialTestDelayStatus[3])
    {
        str="Initial delay--Auto clean";
    }

    if(bInitialTestDelayStatus[2])
    {
        str="Initial delay--Show alarm message";
    }

    if(bInitialTestDelayStatus[4])
    {
        str="Initial delay--Open heat door";
    }

    if(bInitialTestDelayStatus[5])
    {
        str="Initial delay--Happen tested time blow";
    }

    if(bInitialTestDelayStatus[1])
    {
        //AI(W906-W7-L2) 20260803: GOLDEN BUG PRESERVED (golden :2564) -- "Fisrt" is
        // golden's misspelling of "First".  It is operator-visible text on the main
        // form; kept byte-for-byte because the regression tests are written against
        // golden's string and correcting it would fail them.
        str="Initial delay--Fisrt device delay";
    }

    if(bInitialTestDelayStatus[7])
    {
        str="Initial delay--No fullSite";
    }

    if(bInitialTestDelayStatus[9])
    {
        str="Initial delay--OTD unlock";
    }

    //AI(W906-W7-L2) 20260803: GOLDEN ODDITY PRESERVED (golden :2579, :2584) -- these
    // last two strings drop the "Initial delay--" prefix that the other nine carry.
    // The inconsistency is golden's; the strings are reproduced verbatim.
    if(bInitialTestDelayStatus[8])
    {
        str="EOT monitor time over delay";
    }

    if(bInitialTestDelayStatus[10])       //kevin 20181101 add SOT
    {
        str="SOT monitor time over delay";
    }
    return str;
}
//------------------------------------------------------------------------------
