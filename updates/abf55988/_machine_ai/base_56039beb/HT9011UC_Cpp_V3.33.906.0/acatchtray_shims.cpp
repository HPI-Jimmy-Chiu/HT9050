// =============================================================================
//  acatchtray_shims.cpp  --  offline bodies for the W6.3 TrayArm cross-module shims
//
//  Translation wave: W6.3 (TRAY-ARM ENGINE -- acatchtray.cpp)
//  Translator: AI(W6.3-CATCHTRAY) 20260626
//
//  All bodies are CONSERVATIVE offline defaults over the Sim HAL.  See
//  acatchtray_shims.h for per-symbol rationale + golden provenance.
//    * Tray-Mapping / RFID / CCD predicates report "nothing to do" (false) so the
//      Tray-Mapping case arms fall through deterministically.
//    * AMR / Magazine feeds report "done / nothing" so the AMR dispatch (dormant
//      offline because TrayForm.bEnableAMR / fAGV->Use_AMR() are false) links.
//    * MyMessageBox is never visible; LoadCCD reports "no remain IC" (0).
//
//  Big5: Chinese comments preserved as UTF-8 (cp950).  ZERO U+FFFD.
// =============================================================================
#include "acatchtray_shims.h"
#include "MachineType.h"            // eTrayCount / ePortTotal / MAX_AUTO_TRAY (array dims)

// ---- BoolToStr (golden SysUtils.hpp) ---------------------------------------
AnsiString BoolToStr(bool B, bool /*UseBoolStrs*/)
{
    return B ? AnsiString("True") : AnsiString("False");
}

// ---- SECS_EVENT + EventReport -----------------------------------------------
// AI(W5-comms-Integrate) 20260710: now provided for real by ht9045_secsgem
// (SECSGEM/SecsEventType.cpp defines SECS_EVENT; SECSGEM/SecsEventReport.cpp
// defines EventReport()) -- see acatchtray_shims.h.  Removed the local
// placeholder definitions (would otherwise duplicate-define both symbols).

// ---- TfTrayMapping (offline: no CCD / laser / RFID) ------------------------
// AI(W906-W7-L1-Wave0) 20260801: channel-2 quartet added to the init list --
// same all-quiet offline posture as channel 1 (no RFID reader present).
TLdRFIDShim::TLdRFIDShim()
    : bCommConnect(false), bClearCmd1(false), bReadUID1(false),
      bClearCmd2(false), bReadUID2(false) {}   // sBlockID2 / sUID2 default-construct to ""
void TBtnShim::Click() {}
void TListTrayIDShim::Clear() { Text = ""; }
void TListTrayIDShim::Add(AnsiString s) { Text = Text + s + "\r\n"; }
void TYieldShim::AddYieldData(bool /*bPass*/) {}

TfTrayMapping::TfTrayMapping()
    : eAOIType_TrayDeviceCheckLaser(0),
      bDoTrayDeviceCheckFromLoader(false),
      asErrorLaserScan(""),
      iAutoRetryCount(0),
      fShow(false)                               // AI(W906-ST-S2-fShow-B1) 20260917: offline, no form on screen; LAST in the list because it is declared last (-Wreorder)
{
    ldRFID              = new TLdRFIDShim();
    btnLdRFIDConn       = new TBtnShim();
    btnLdRFIDClear1     = new TBtnShim();
    btnLdRFIDReadBlock1 = new TBtnShim();
    btnReadUID1         = new TBtnShim();
    cbBarCodeSimulate   = new vclcompat::TCheckBox();   //AI(W906-ST-S3-B2b) 20260918: retyped, see header
    listTrayIDByLot     = new TListTrayIDShim();
    // AI(W906-W7-L1-Wave0) 20260801: channel-2 button triad (asendic_Color.cpp).
    btnLdRFIDClear2     = new TBtnShim();       // golden cTrayMapping.h:466
    btnReadUID2         = new TBtnShim();       // golden cTrayMapping.h:468
    btnLdRFIDReadBlock2 = new TBtnShim();       // golden cTrayMapping.h:470
    // AI(W906-W7-L1-Wave0) 20260801: eAOIType_TrayMapLaser MUST be 1, not 0.
    // In golden it is the SECOND enumerator of the in-class AOI-type enum
    // (golden cTrayMapping.h:699 eAOIType_TrayDeviceCheckLaser=0, :700
    // eAOIType_TrayMapLaser), so its ordinal is 1.  Mirroring it as an int that
    // defaulted to 0 would collapse the tray-map AOI type onto the device-remain
    // one and the distinction would disappear with no test able to see it.
    eAOIType_TrayMapLaser = 1;                  // golden cTrayMapping.h:700 (ordinal 1)
    // Tray-map grid: dimensions verbatim from golden cTrayMapping.h:617 [35][70].
    for(int iW0r=0; iW0r<35; iW0r++)
        for(int iW0c=0; iW0c<70; iW0c++)
            iTrayMappingDate[iW0r][iW0c] = 0;
}
void TfTrayMapping::WritePickMapLog() {}
bool TfTrayMapping::IsRunDeviceRemainLaser()        { return false; }
bool TfTrayMapping::GetTrayDeviceCheckIsRemainIC()  { return false; }
bool TfTrayMapping::DoLaserScanFromLoader(int, bool, bool) { return false; }
bool TfTrayMapping::DoAutoSkip(int)                 { return false; }
bool TfTrayMapping::CheckRunAutoRetry(int)          { return false; }
bool TfTrayMapping::DoCoverTrayID(int, bool)        { return false; }   // golden cTrayMapping.h:674 -- 2 params (see header)
bool TfTrayMapping::DoCoverTrayIDKeyence(int)       { return false; }
bool TfTrayMapping::DoTrayIDKeyence()               { return false; }
bool TfTrayMapping::DoTrayIDKeyence2(int)           { return false; }
bool TfTrayMapping::DoTrayID2CCD()                  { return false; }
bool TfTrayMapping::DoTrayIDCCD()                   { return false; }
bool TfTrayMapping::DoTrayMapCCD(int)               { return false; }
// -- AI(W906-W7-L1-Wave0) 20260801 ADD: Tray-Map-laser + tray-ID-check surface --
//    See acatchtray_shims.h for golden citations and, for DoTrayIDCheck, for why
//    its offline value is a decision rather than a default.
bool TfTrayMapping::IsRunTrayMapLaser()             { return false; }  // offline: no laser scanner installed
bool TfTrayMapping::DoSettingBeforeMotorMove(int, bool, AnsiString&)   { return true; }   // offline: setting complete
bool TfTrayMapping::DoGetValueAfterMotorArrival(int, bool, AnsiString&){ return true; }   // offline: value acquired
bool TfTrayMapping::GetTrayMapHasNullIC()           { return false; }  // offline: no mapped null IC (unreachable while IsRunTrayMapLaser is false)
bool TfTrayMapping::DoTrayIDCheck(AnsiString)       { return true;  }  // offline: no duplicate tray ID -- DELIBERATE, see header
TfTrayMapping *fTrayMapping = new TfTrayMapping();

// ---- TrayEditForm (golden uTrayEditForm.h) ---------------------------------
// AI(W906-ST-S2-fShow-B1) 20260917: see the header for why this is a two-member
// stand-in and why the type carries a _Facade suffix while the global does not.
void TTrayEditForm_Facade::Close() {}          // offline: there is no window to close
TTrayEditForm_Facade *TrayEditForm = new TTrayEditForm_Facade();

// ---- MyMessageBox (golden mymessbox.h) -------------------------------------
TMyMessageBoxShim::TMyMessageBoxShim() : Visible(false), fShow(false) {}
void TMyMessageBoxShim::Close() {}
TMyMessageBoxShim *MyMessageBox = new TMyMessageBoxShim();

// ---- LoadCCD (golden LoadCCDMap.h) -----------------------------------------
int  TLoadCCDShim::LoadCCDData()   { return 0; }    // offline: no remain IC -> proceed
void TLoadCCDShim::InitialLoadCCD(){}
TLoadCCDShim *LoadCCD = new TLoadCCDShim();

// ---- AMR (golden AMR.h) -- AI(W5-Automation-Integrate) 20260710: TAMRShim/AMR
//      stopgap REMOVED (ODR collision with the real Automation/AMR.cpp `AMR`
//      global now that it exists) -- see acatchtray_shims.h comment.

// ---- missing free functions (offline-safe) ---------------------------------
#if 0   // PT-W7e-part2 RETIRED (MoveInArm2XYToWait): real translated body now in ainarm2.cpp
bool MoveInArm2XYToWait()                       { return true;  }
#endif
#if 0   // PT-W7e-part2 RETIRED (MoveInArm2XYToShuttle2Wait): real translated body now in ainarm2.cpp
bool MoveInArm2XYToShuttle2Wait()               { return true;  }
#endif
#if 0   // PT-W7e-part2 RETIRED (CheckInArmFinishAllPickerAction): real translated body now in ainarm2.cpp
bool CheckInArmFinishAllPickerAction()          { return true;  }
#endif
#if 0   // PT-W7e-part2 RETIRED (IsMoveInArm2XYToWait): real translated body now in ainarm2.cpp
bool IsMoveInArm2XYToWait()                     { return true;  }
#endif
#if 0   // PT-W7e-part2 RETIRED (IsMoveInArm2XYToShuttle2Wait): real translated body now in ainarm2.cpp
bool IsMoveInArm2XYToShuttle2Wait()             { return false; }  // golden ainarm2.h:142 (Sam 20250522) -- offline: NOT at wait
#endif
bool CheckOutArmAutoAlignmentTrayModeBeUse(int) { return false; }
void InitProcessSingleMotorTask(int)            {}
bool ProcessSingleMotorHome(int)                { return true;  }
int  SearchNeedAMRUnload(bool)                  { return -1;    }
int  SearchNeedAMRUnloadCount()                 { return 0;     }
AnsiString AMRUnloadBin(int)                    { return "";    }
bool DoMagazineAMRTrayFeed(int)                 { return true;  }
void InitialDoMagazineAMRTrayFeed()             {}
// InitAutoColorReceiveTask: RETIRED BODY (W7-L1 Wave 1 "Color") -- real body now
// in asendic_Color.cpp (golden asendic_Color.h:16).  Declaration kept in the
// header because csystem.cpp / acatchtray.cpp bind to it through there.
// InitAutoEmptyReceiveTask: REAL home asendic_Empty.cpp (W6.1 canary) -> NOT redefined here (ODR)
// DoAutoColor: RETIRED BODY (W7-L1 Wave 1 "Color") -- real body now in
// asendic_Color.cpp, and it is VOID (golden asendic_Color.h:20); this stand-in
// wrongly returned bool.  All 3 ported call sites discard the value.
#if 0   // PT-W7a RETIRED (SetUnloaderInfoFile)
//AI(ht9045-v906) 20260810: PT-W7a -- RETIRED. cinitial.cpp now holds golden's real body (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0810_w7a.
void SetUnloaderInfoFile(int)                   {}
#endif
void InitDoOutArmTeachAlignmentProcessTask()    {}
void NewRecordProcess(AnsiString, AnsiString, AnsiString) {}

// ---- missing file-scope globals --------------------------------------------
//  iCoverTrayIDTask is a per-port cursor array; size to the cover-tray-ID port
//  count.  iKeyenceCoverTrayID_LoaderCar etc. index it (MachineType.h enum);
//  64 is comfortably above the port count.  iReadCIDAction = ePortTotal (idle).
int iCoverTrayIDTask[64] = {0};                 // golden cmydef / cTrayMapping
int iReadCIDAction       = 0;                   // golden -- ePortTotal at rest (set in SM)
//  NOTE: bPlaceToHotplate / bPlaceShuttle / iInArmWaitPosition / iInRotateFinish
//  are DEFINED in acatchtray.cpp (it owns them this wave) -- not here (ODR).
//  NOTE: the TrayArm-subsystem globals (bEject / bPurgeOutAllDevice / iThisPortNo
//  / iThisPortStatus / iPortStatus / ASE_OutTrayNum / iReceiveAutoTrayTask /
//  bOldAutoHasTray / bBoatChangeCasset / bPortIsBusy / bAskStopPort /
//  iLastPortStatus / bForceSendLoaderIsEmpty) + the Tray-Mapping inits + the
//  out-arm MoveOutArmXY_ToFix_Tray_Full(bool) are DEFINED below in the
//  consolidated W7 block -- not duplicated here.

// ---- W7 ADD: per-Auto receive / stack-cylinder helpers (offline-safe) ------
// DoAutoReceiveBinTray: RETIRED BODY (W7-L1 Wave 2 "Auto") -- real body now in
// asendic_Auto.cpp, and it is VOID (golden asendic_Auto.h:8); this stand-in
// wrongly returned bool.  All 4 ported call sites discard the value.
// AutoCylinderUp / AutoCylinderMiddle / AutoCylinderLower: RETIRED BODIES
// (W7-L1 Wave 3 / L1a) -- the three `{ return true; }` no-op stubs that were
// THE open HIGH finding of this sub-project are gone.  The real golden state
// machines now live in asendic.cpp (golden asendic.cpp:562-765, :767-933,
// :937-1160) and the DECLARATIONS moved to asendic.h, which is where golden
// declares them (golden asendic.h:19-21).  Unlike the Color / Loader / Auto
// retirements above, the declarations are NOT kept here: golden carries default
// arguments on parameter 4, and every TU that calls these already includes
// asendic.h, so keeping a second defaulted declaration in this header would give
// parameter 4 two default arguments in the many TUs that include both.
// Verified in this pass -- all four production callers include asendic.h:
// acatchtray.cpp:90, asendic_Auto.cpp:122, asendic_Auto2.cpp:100,
// asendic_Auto_RT.cpp:201.
#if 0   // PT-W7a RETIRED (SetMotorScaleSpeed)
//AI(ht9045-v906) 20260810: PT-W7a -- RETIRED. cinitial.cpp now holds golden's real body (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0810_w7a.
void SetMotorScaleSpeed(int, int)                    {}                  // offline: no-op
#endif
#if 0   // PT-W7a RETIRED (SetMotorAccelSpeed)
//AI(ht9045-v906) 20260810: PT-W7a -- RETIRED. cinitial.cpp now holds golden's real body (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0810_w7a.
void SetMotorAccelSpeed(int, int)                    {}                  // AI(W906-AutoCleanFoundation) 20260721: golden cinitial.h:51 sibling -- offline no-op
#endif
void ClearAutoChangingWarn(int)                      {}                  // offline: BinDisp warn-clear no-op

// =============================================================================
//  (W7) TrayArm-subsystem GLOBALS + functions whose golden definitions live in
//  not-yet-translated modules / gated cmydef.cpp/cprod.cpp blocks (KYEC-AMR /
//  Boat-Carrier / Tray-Mapping / cover-tray / port-status).  The engine reads
//  them; their extern decls are in cmydef.h / csystem.h / acatchtray.h.  Provide
//  offline-safe DEFINITIONS here so the engine LINKS.  Golden homes noted.
//  TODO(W7): remove as cmydef.cpp / the owning subsystems are ungated/translated.
// =============================================================================
#include "cmydef.h"     // MAX_AUTO_TRAY, eTrayCount, ePortTotal

// -- data globals (offline-safe init) -----------------------------------------
bool bBoatChangeCasset[3]            = {false,false,false};   // golden cmydef.cpp:5874 (Ifor 20251220 Boat Change)
bool bEject                          = false;                 // golden cmydef.cpp:5937 (JerryYang 20251020 half-clean)
bool bPurgeOutAllDevice              = false;                 // golden cmydef.cpp:5939 (JerryYang 20251020 half-clean)
bool bTrayArmIsPickColor             = false;                 // golden cmydef.cpp:5912 (JerryYang 20250113 Color TrayID)
bool bOldAutoHasTray[MAX_AUTO_TRAY]  = {false,false,false,false,false,false}; // golden cmydef.cpp (acatchtray.h:53 extern)
int  ASE_OutTrayNum[eTrayCount]      = {0};                   // golden cmydef.cpp:5795 (KYEC AMR output tray count)
int  iThisPortNo                     = 0;                     // golden cmydef.cpp:5843
int  iThisPortStatus                 = 0;                     // golden cmydef.cpp:5844
int  iPortStatus[ePortTotal]         = {0};                   // golden cmydef.cpp:5845
#if 0   // PT-W5c RETIRED (iReceiveAutoTrayTask)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
int  iReceiveAutoTrayTask[MAX_AUTO_TRAY] = {0};               // golden csystem.h:67 (auto-receive cursors)
#endif
// AI(W906-AGV-PortScan-Integrate) 20260713: same-neighborhood siblings of the
// iThisPortNo/iThisPortStatus/iPortStatus trio just above (golden cmydef.cpp
// :5916-5924, all still inside this tree's cmydef.cpp "#if 0 // TODO(W6)"
// tail gate -- declared `extern` in cmydef.h so any TU compiles, but with NO
// active definition anywhere) -- needed by Automation/AGV_PortScan.cpp
// (bIsStackBusy/bScanLoadPortState_SPIL/bScanUnLoadPortState_SPIL/
// DoE84LoaderScan/DoE84UnloaderScan etc., all in this same ht9045_sm
// library), which a fresh from-scratch link caught as undefined references.
// Offline-safe zero-init, same idiom as every other stand-in in this block.
bool bAskStopPort[ePortTotal]        = {false};                // golden cmydef.cpp:5916 (JerryYang 20250521 : For AMR)
bool bPortIsBusy[ePortTotal]         = {false};                 // golden cmydef.cpp:5917
int  iLastPortStatus[ePortTotal]     = {0};                    // golden cmydef.cpp:5921
bool bForceSendLoaderIsEmpty         = false;                   // golden cmydef.cpp:5924 (JerryYang 20250618 : add)

// -- functions (offline-safe) -------------------------------------------------
#if 0   // PT-W5f RETIRED (DoLockUnloader)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165. *** BEHAVIOUR DELTA, NOT NEUTRAL: the stub answered "tray clamped OK" unconditionally, so retiring it lets the real check actually run and it can now REFUSE. ***
bool DoLockUnloader(int /*iAuto*/)         { return true; }   // golden csystem.h:259 -- offline: unloader locked OK
#endif
void InitialTrayIDTask()                   {}                 // golden cTrayMapping -- reset tray-ID SM (no-op)
void InitialTrayID2Task()                  {}                 // golden cTrayMapping -- reset tray-ID-2 SM (no-op)
void InitialTrayMapTask(int /*iTrayMap*/)  {}                 // golden cTrayMapping -- reset tray-map SM (no-op)
void InitialCoverTrayIDTask(int /*iPos*/)  {}                 // golden cTrayMapping -- reset cover-tray-ID SM (no-op)
//  MoveOutArmXY_ToFix_Tray_Full(bool): golden aoutarm side has a 0-arg file-local
//  static; the TrayArm engine calls the 1-arg (bMoveY) form (golden decl :68).
//  Offline: out-arm reaches the fix-tray-full XY immediately.
// AI(W906-PT-W4-integrate) 20260809: 1 STAND-IN DEFINITION(S) RETIRED FROM HERE.
//   aoutarm.cpp (golden's own home for all of them) landed in wave PT-W4 and is
//   registered in ht9045_sm, so both definitions were in libht9045_sm.a and every
//   executable linking it failed with `multiple definition of ...`. The linker named
//   each one, which is also the proof the signatures match exactly -- a decorated-name
//   collision cannot happen otherwise.
//   Retired here: MoveOutArmXY_ToFix_Tray_Full
//   BEHAVIOUR: these were offline defaults (return true/false/0/no-op); the real bodies
//   run golden's actual logic, so out-arm paths that used to short-circuit now execute.
//   That is the point of the wave, and it is why this wave was measured on its own.
