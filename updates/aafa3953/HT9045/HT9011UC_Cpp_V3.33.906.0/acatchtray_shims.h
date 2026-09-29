// =============================================================================
//  acatchtray_shims.h  --  W6.3 offline cross-module shims for the TrayArm engine
//
//  Translation wave: W6.3 (TRAY-ARM ENGINE -- acatchtray.cpp)
//  Translator: AI(W6.3-CATCHTRAY) 20260626
//
//  WHY THIS FILE EXISTS
//  --------------------
//  The faithfully-transcribed TrayArm engine (acatchtray.cpp) reaches a handful
//  of cross-module subsystems that have NO translated home this wave.  Rather
//  than gate hundreds of call sites inside the verbatim switch(Task) SMs (which
//  would obscure the cursor flow we are trying to preserve), this header exposes
//  ONLY the surface the engine dereferences, on transparently-named non-VCL
//  stand-ins, with offline-safe bodies (false / 0 / no-op) in acatchtray_shims.cpp.
//  This mirrors the FormsFacade form-pointer cut already used by the in/out-arm
//  engine waves -- it is the "genuinely cleaner" shim the task brief permits.
//
//  Each symbol cites its golden home.  When the real subsystem lands (W7:
//  cTrayMapping.h / Magazine.h / OCR.h / AMR.h / uHGemHT9045 SECS / LoadCCDMap.h),
//  these shims are replaced and the call sites bind to the real bodies unchanged.
//
//  GATED (NOT shimmed here -- left as `#if 0` at the single call site, since the
//  offline guard already makes them dead):
//    * HSys.BinDisCtrl->FlashPro / ClearAutoChangingWarn  (database.h TMyBinDispCtrl
//      is forward-decl-only; HSys.BinDisCtrl==NULL offline so the calls never run).
//
//  Big5: Chinese comments decoded via cp950, preserved UTF-8.  ZERO U+FFFD.
// =============================================================================
// =============================================================================
//  AI(W906-W7-L1-Wave0) 20260801 -- SHIM RETIREMENT DEBT REGISTER (W7-L1)
//
//  Written into the TREE, not just into a report, because each entry below is a
//  DUPLICATE-SYMBOL link failure the moment its owning translated file lands, and
//  two of them are also return-type conflicts.  Wave 0 deliberately does NOT
//  perform any of these retirements: retiring a shim before its owner exists
//  turns a working link into undefined symbols.  Each retirement belongs to the
//  INTEGRATE step of the wave named against it.  All line numbers below were
//  re-derived by reading the files in this pass.
//
//  (Declarations in THIS header are given by NAME, not line number -- this banner
//  shifts every line below it, and a stale line citation is exactly the kind of
//  claim this project keeps having to correct.  The .cpp line numbers below were
//  re-derived AFTER this pass's own edits to acatchtray_shims.cpp.)
//
//  OWNED BY THE WAVE THAT LANDS asendic_Auto.cpp (W7-L1 Wave 2, "Auto"):
//    * acatchtray_shims.h <DoAutoReceiveBinTray decl> + acatchtray_shims.cpp:146
//      `bool DoAutoReceiveBinTray(int)` -- golden declares it VOID
//      (golden asendic_Auto.h:8).  Return type is not part of the mangled name,
//      so this is a duplicate symbol AND a conflicting declaration in any TU that
//      sees both.  All 4 ported callers discard the value, so switching to void
//      costs nothing.
//    * csystem_shims.h <Initial_Auto_BinTray_Task decl> + csystem_shims.cpp:120
//      `void Initial_Auto_BinTray_Task(int)` -- signature-identical duplicate.
//
//  [DONE 20260802] OWNED BY THE WAVE THAT LANDS asendic_Color.cpp
//  (W7-L1 Wave 1, "Color") -- ALL THREE RETIRED when asendic_Color.cpp landed:
//    * <DoAutoColor decl> -- body retired here; the DECLARATION was also
//      corrected `bool` -> `void`, which is golden (golden asendic_Color.h:20).
//      All 3 ported call sites discarded the value, so void cost them nothing.
//    * <InitAutoColorReceiveTask decl> -- body retired here.
//    * csystem_shims.h <InitAutoColorTask decl> -- body retired there.
//    In all three the DECLARATION IS KEPT: csystem.cpp and acatchtray.cpp bind
//    to these symbols through the shim headers, so deleting the declarations
//    would have required adding `#include "asendic_Color.h"` to both callers.
//
//  [DONE 20260802] OWNED BY THE WAVE THAT LANDS asendic_Loader.cpp (W7-L1
//  Wave 1, "Loader" bundle -- Loader + Loader_RT are ONE agent).  BOTH BODIES
//  RETIRED in csystem_shims.cpp, both declarations kept:
//    * <DoLoad decl>       -- body retired; csystem.cpp still calls it through
//      csystem_shims.h and the two signatures are identical.
//    * <InitLoadTask decl> -- body retired; behaviourally inert (the only
//      ported caller is asendic_Loader.cpp's own case 1400).
//      THE RE-BASELINE THIS NOTE PREDICTED DID NOT HAPPEN.  Measured, not
//      assumed: W7-L1 Wave 1 built both hub tests twice -- once against the
//      pristine archive, once with the bodies retired and the real Loader
//      objects added -- and test_w6_6_csystem_cycle stayed 20 PASS / 0 FAIL
//      and test_w6_6_hub stayed 9 PASS / 0 FAIL, exit 0, zero-line diff once
//      the new log lines are filtered.  Neither test file needed an edit.
//
//  [DONE 20260802] OWNED BY THE WAVE THAT LANDS asendic.cpp's AutoCylinder*
//  bodies (W7-L1 Wave 3 / L1a) -- RETIRED, and this one behaves differently from
//  the three above: BOTH the bodies AND the declarations are gone from this pair
//  of files.  The three `{ return true; }` no-op stubs that ARE the open HIGH
//  finding are replaced by golden's real state machines in asendic.cpp
//  (golden asendic.cpp:562-765 / :767-933 / :937-1160), and the declarations moved
//  to asendic.h where golden declares them (golden asendic.h:19-21).  The
//  declarations could NOT be kept here the way DoAutoColor's was, because golden
//  gives parameter 4 a default argument and the many TUs that include BOTH headers
//  would then see it twice.  The caller-argument-order warning that used to be
//  attached to those declarations moved with them, and is now also pinned by an
//  executable test.  AI(W906-W7-L1-W3fixB) 20260802: that test was cited here as
//  `tests/test_w7_l1_wave3_argorder.cpp`, which has never existed in this tree.
//  The real pin is tests/test_w7_l1_auto2.cpp SUB-TEST [12]
//  (`test_argument_order_pin`); swapping parameters 2 and 3 at golden
//  asendic_Auto2.cpp:85 takes that suite from 69/0 to 62 passed / 7 failed
//  (measured in this pass).
//
//  ALSO RETIRED BY WAVE 3, outside this file: csystem_shims.cpp's
//  `void NewDoAutoTrayEdgeCylinderLoop() {}` body (its declaration in
//  csystem_shims.h is kept, exactly like DoLoad / InitLoadTask above), because
//  asendic.cpp now carries golden's real body (golden asendic.cpp:1206-1233) and
//  the two definitions would otherwise be a duplicate symbol at link.
//
//  NOT A RETIREMENT AND NOT WAVE 0's: the six golden asendic.cpp free functions
//  (bARTUnloaderUseOneCylin / bARTUnloaderUseTwoCylin / PushLoaderTrayInAverageTime
//  / PushUnLoaderTrayInAverageTime / RecordAutoCleanOutStartEnd /
//  DoAutoTrayEdgeCylinderLoop, plus NewDoAutoTrayEdgeCylinderLoop) land as REAL
//  translated bodies in Wave 3, never as stubs -- so no shim for them exists here
//  to retire, and none should be added.
//
//  PRECEDENT for how a retirement is recorded once done: the DoAuto2 retirement
//  note in csystem_shims.h and the matching CMakeLists.txt comment.
// =============================================================================
#ifndef acatchtray_shimsH
#define acatchtray_shimsH

#include "vclcompat/vcl_compat.h"   // AnsiString
//AI(W906-ST-S3-B2b) 20260918: vclcompat::TCheckBox, for cbBarCodeSimulate below.
//  ⚠ I first "checked" for this include by counting matches of "Controls.h" in
//  this file and got 2 -- both were COMMENTS (:194, :196). Mention is not carry;
//  the compiler said so immediately.
#include "vclcompat/Controls.h"     // vclcompat::TCheckBox

// ===========================================================================
//  TColor + clWhite/clYellow  -- golden VCL Graphics.hpp.  The place-to-Auto SM
//  assigns fMain->mtAutoN->Color = cPtr[iTemp] where cPtr[2]={clWhite,clYellow}.
//  In the VCL TColor is an int RGB code; provide the int alias + the two consts.
// ===========================================================================
#ifndef HT9045_TCOLOR_SHIM
#define HT9045_TCOLOR_SHIM
typedef int TColor;                         // golden Graphics.hpp TColor (int)
const TColor clWhite  = 0x00FFFFFF;         // golden Graphics.hpp clWhite
const TColor clYellow = 0x0000FFFF;         // golden Graphics.hpp clYellow
#endif

// ===========================================================================
//  BoolToStr  -- golden SysUtils.hpp.  DoPlaceToBuffer's time-out log formats
//  several bool flags via BoolToStr.  vclcompat does not provide it; supply a
//  free function matching the VCL spelling ("-1"/"0" by default, but the engine
//  only uses it inside a log sprintf, so "True"/"False" is faithful enough).
// ===========================================================================
AnsiString BoolToStr(bool B, bool UseBoolStrs = false);   // golden SysUtils.hpp

// ===========================================================================
//  SECS_EVENT (ETypeStruct) + EventReport -- AI(W5-comms-Integrate) 20260710:
//  this used to be a MINIMAL local mirror (only the dozen CEID names acatchtray
//  .cpp references, values hand-copied from golden uHGemHT9045.h:313).  The W5
//  SECSGEM-base translate unit has since landed the REAL, complete extraction
//  (SECSGEM/SecsEventType.h -- every CEID, golden line-for-line) plus a real
//  Sim-first EventReport() entry point (SECSGEM/SecsEventReport.h), both linked
//  via ht9045_secsgem (see root CMakeLists.txt).  Verified every CEID name this
//  file used to declare locally exists in the real enum with the IDENTICAL
//  value (TrayTestFinish=67 / AGVSupplement=272 / UnloaderTrayIDReadOK=285 /
//  LoaderTrayIDReadFail=287 / AGVLdID=291 / BundleCompleteIDRead=242 /
//  BundleEnd_IDREAD_Auto1..6=246/248/252/254/256/258) -- switching to the real
//  header is behavior-identical for every acatchtray.cpp call site, and
//  resolves what would otherwise be a duplicate `struct ETypeStruct`/SECS_EVENT/
//  EventReport definition now that the real translation exists.
// ===========================================================================
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"

// ===========================================================================
//  TfTrayMapping (fTrayMapping)  -- golden cTrayMapping.h.  The TrayArm engine
//  derefs the Tray-Mapping / device-remain / cover-tray-ID / RFID surface.
//  MINIMAL mirror of exactly the members the engine touches; every predicate is
//  false / no-op offline (no CCD / no laser / no RFID reader present), so the
//  Tray-Mapping case arms compile and take their deterministic "nothing to map"
//  fall-through.  TODO(W7): replace with the real translated cTrayMapping.h.
// ===========================================================================
//  ldRFID nested object (golden cTrayMapping.h TfTrayMapping::ldRFID)
class TLdRFIDShim
{
public:
    bool       bCommConnect;        // RFID reader link up?  (offline false)
    bool       bClearCmd1;          // clear-cmd ack
    bool       bReadUID1;           // UID read done?
    AnsiString sBlockID1;           // read block-ID
    AnsiString sUID1;               // read UID
    // AI(W906-W7-L1-Wave0) 20260801: the CHANNEL-2 quartet -- symmetric completion
    // of the channel-1 quartet above, which only exists because acatchtray.cpp
    // happened to need channel 1 first.  golden cTrayMapping.h:118 (bClearCmd2),
    // :119 (bReadUID2), :108 (sUID2), :109 (sBlockID2) -- all four re-read from
    // the cp950-decoded golden this pass.  asendic_Color.cpp drives the whole
    // channel-2 read sequence: :423-425 / :595-597 / :1503-1505 clear them,
    // :593 tests bClearCmd2, :1511 tests bReadUID2, :1534 tests sBlockID2!="" and
    // :1536 tests sBlockID2.Length()!=9, :571 assigns sBlockID2 a formatted stamp.
    bool       bClearCmd2;          // golden cTrayMapping.h:118 -- clear-cmd ack (channel 2)
    bool       bReadUID2;           // golden cTrayMapping.h:119 -- UID read done? (channel 2)
    AnsiString sBlockID2;           // golden cTrayMapping.h:109 -- read block-ID (channel 2)
    AnsiString sUID2;               // golden cTrayMapping.h:108 -- read UID (channel 2)
    TLdRFIDShim();
};
//  a clickable-button stand-in (golden TButton*; engine only calls ->Click())
class TBtnShim { public: void Click(); };
//  a TStringList-ish stand-in for listTrayIDByLot (Clear/Add/Text)
//
//  AI(W906-W7-F2) 20260729 -- DELIBERATELY *NOT* RETIRED.  W7-F2 retired the TU-local
//  duplicate widget value-holders tree-wide in favour of vclcompat/Controls.h, and this
//  type was on that list, but it FAILS the equivalence check and aliasing it would
//  change bytes written to disk.  It is not a stock widget (Controls.h has no
//  counterpart); the only candidate is vclcompat::TStringList, and the two disagree on
//  what ->Text produces:
//    * this shim:            Add(s) appends s + "\r\n", so Text == "<id>\r\n"
//    * vclcompat TStringList: GetText() joins items with '\n' and appends NO trailing
//                             break, so Text would become "<id>"
//  acatchtray.cpp's only two consumers write that value straight to a file
//  (`WriteDataToFile(asTrayIDByLot, fTrayMapping->listTrayIDByLot->Text)`, both call
//  sites), so swapping the type would silently change the on-disk tray-ID-by-lot
//  record.  Noted for the record: of the two, THIS shim is the closer match to real
//  VCL (Delphi's TStrings::GetTextStr emits sLineBreak == CRLF after EVERY line,
//  including the last); the divergence is in vclcompat::TStringList, which is outside
//  this wave's write scope.  Reported as a W7-F2 finding rather than "fixed" here.
class TListTrayIDShim
{
public:
    AnsiString Text;
    void Clear();
    void Add(AnsiString s);
};
//  a yield-recorder stand-in for yieldRemainIC (AddYieldData(bool))
class TYieldShim { public: void AddYieldData(bool bPass); };

class TfTrayMapping
{
public:
    // -- AOI-type selector the device-remain laser scan passes (golden enum) ----
    int  eAOIType_TrayDeviceCheckLaser;         // golden cTrayMapping.h
    bool bDoTrayDeviceCheckFromLoader;          // golden -- device-remain-from-loader flag
    AnsiString asErrorLaserScan;                // golden -- laser-scan error string
    int  iAutoRetryCount;                       // golden -- auto-retry counter
    TLdRFIDShim     *ldRFID;                     // golden -- loader-car RFID reader
    TBtnShim        *btnLdRFIDConn;              // golden -- RFID connect button
    TBtnShim        *btnLdRFIDClear1;            // golden -- RFID clear button
    TBtnShim        *btnLdRFIDReadBlock1;        // golden -- RFID read-block button
    TBtnShim        *btnReadUID1;                // golden -- RFID read-UID button
    //AI(W906-ST-S3-B2b) 20260918: was TBtnShim, which is wrong and the old
    //  comment already said so ("sim checkbox"). TBtnShim is declared as "a
    //  clickable-button stand-in (golden TButton*; engine only calls ->Click())"
    //  and has no Checked, so acatchtray.cpp:7762's `fTrayMapping->
    //  cbBarCodeSimulate->Checked` -- inside `#ifdef SOFT_SIMULTE` -- could never
    //  compile. It never had to: the tree's convention was that SOFT_SIMULTE is
    //  never defined. The user turned it on 20260918.
    //  Measured before retyping: ->Checked is the ONLY member any call site
    //  touches tree-wide, and nothing calls ->Click() on it, so nothing is lost.
    vclcompat::TCheckBox *cbBarCodeSimulate;     // golden BarCode.h:539 (TCheckBox*) -- offline Checked=false
    TListTrayIDShim *listTrayIDByLot;            // golden -- per-lot tray-ID list
    TYieldShim       yieldRemainIC;              // golden -- device-remain yield recorder
    // AI(W906-W7-L1-Wave0) 20260801: the CHANNEL-2 button triad -- symmetric
    // completion of the channel-1 triad above (which exists only because
    // acatchtray.cpp needed channel 1).  golden types are TButton* and the SMs
    // only ever call ->Click(), so they reuse the existing TBtnShim.  Driven by
    // asendic_Color.cpp:426 / :598 / :1506 (btnReadUID2), :581 (btnLdRFIDClear2),
    // :1528 (btnLdRFIDReadBlock2).
    TBtnShim        *btnLdRFIDClear2;            // golden cTrayMapping.h:466 (TButton*)
    TBtnShim        *btnReadUID2;                // golden cTrayMapping.h:468 (TButton*)
    TBtnShim        *btnLdRFIDReadBlock2;        // golden cTrayMapping.h:470 (TButton*)
    // AI(W906-W7-L1-Wave0) 20260801: Tray-Map-laser surface asendic_Loader.cpp
    // needs.  eAOIType_TrayMapLaser is, in golden, an ENUMERATOR inside the class
    // (golden cTrayMapping.h:699-700: eAOIType_TrayDeviceCheckLaser=0 then
    // eAOIType_TrayMapLaser, i.e. ordinal 1) -- mirrored as a plain int here, the
    // same shape its sibling eAOIType_TrayDeviceCheckLaser already uses a few
    // lines up.  THE CTOR MUST SET IT TO 1: if both AOI-type ints defaulted to 0
    // the device-remain and tray-map scans would collapse onto one value and the
    // distinction would silently vanish (see acatchtray_shims.cpp).
    int  eAOIType_TrayMapLaser;                  // golden cTrayMapping.h:700 (enumerator, ordinal 1)
    // Dimensions VERBATIM from golden cTrayMapping.h:617 `int
    // iTrayMappingDate[35][70];`.  golden asendic_Loader.cpp:1057 indexes it
    // [j][i] == [row][col], so Tray.YItem must stay < 35 and XItem < 70; do not
    // "tidy" the bounds or transpose the subscripts.
    int  iTrayMappingDate[35][70];               // golden cTrayMapping.h:617

    void WritePickMapLog();                      // golden -- offline no-op
    bool IsRunDeviceRemainLaser();               // golden -- offline false
    bool GetTrayDeviceCheckIsRemainIC();         // golden -- offline false
    bool DoLaserScanFromLoader(int eType, bool bStart, bool bCheck=false);  // golden
    bool DoAutoSkip(int eType);                  // golden -- offline false
    bool CheckRunAutoRetry(int eType);           // golden -- offline false
    // AI(W906-W7-L1-Wave2) 20260802: SIGNATURE CORRECTED -- golden cTrayMapping.h:674
    // is `bool DoCoverTrayID(int iFunction, bool bAlarm=false);`, TWO parameters.
    // The one-parameter stand-in was a shim bug: asendic_Auto.cpp passes the second
    // argument, so case 1415 could not compile against it.  The default keeps all
    // four pre-existing one-argument call sites source-compatible.  NOTE the mangled
    // name changes (...Ei -> ...Eib), so this needs a FULL rebuild, not incremental.
    bool DoCoverTrayID(int iPos, bool bAlarm=false);  // golden cTrayMapping.h:674 -- offline false
    bool DoCoverTrayIDKeyence(int iPos);         // golden -- offline false
    bool DoTrayIDKeyence();                      // golden -- offline false (case 4200, 0-arg)
    bool DoTrayIDKeyence2(int i);                // golden -- offline false
    bool DoTrayID2CCD();                         // golden -- offline false
    bool DoTrayIDCCD();                          // golden -- offline false
    bool DoTrayMapCCD(int iMap);                 // golden -- offline false (case 5100/6030, 1-arg)
    // AI(W906-W7-L1-Wave0) 20260801: Tray-Map-laser methods asendic_Loader.cpp
    // needs, plus the tray-ID duplicate check asendic_Color.cpp needs.  All
    // golden lines re-read from the cp950-decoded golden cTrayMapping.h this pass.
    //
    // IsRunTrayMapLaser() is the MASTER GATE for the whole Tray-Map-laser branch
    // family -- 6 golden sites in asendic_Loader.cpp (:185, :362, :891, :903,
    // :917, :1124).  Offline FALSE (no laser scanner), which is why
    // GetTrayMapHasNullIC below is unreachable offline yet still has to compile.
    bool IsRunTrayMapLaser();                    // golden cTrayMapping.h:710 -- offline: false
    // Third parameter is a NON-CONST AnsiString& out-param and golden passes this
    // object's OWN asErrorLaserScan member into it (asendic_Loader.cpp:187, :364,
    // :369, :893, :905) -- so it must stay a reference, not a value or a const&.
    bool DoSettingBeforeMotorMove(int iMode, bool bIsFirst, AnsiString &asError);   // golden cTrayMapping.h:722 -- offline: true
    bool DoGetValueAfterMotorArrival(int iMode, bool bIsFirst, AnsiString &asError);// golden cTrayMapping.h:720 -- offline: true
    bool GetTrayMapHasNullIC();                  // golden cTrayMapping.h:721 -- offline: false
    // DoTrayIDCheck's offline return is a REAL SEMANTIC DECISION, not a default,
    // and it is TRUE.  Both Keyence tray-ID arms test it as
    // `else if(fTrayMapping->DoTrayIDCheck(asTrayIDData2)==false)`
    // (golden asendic_Color.cpp:747 and :1073).  Returning false would steer both
    // arms into golden's "Tray ID duplicate -- operator must remove the tray"
    // dead end, making that whole sub-flow offline-unreachable and untestable;
    // returning true steers them to the success path.  Offline there is no
    // tray-ID history to collide with, so "no duplicate detected" is also the
    // faithful reading.  Decision recorded here at the declaration on purpose --
    // it silently determines whether two whole sub-flows are reachable offline.
    bool DoTrayIDCheck(AnsiString Str);          // golden cTrayMapping.h:747 -- offline: TRUE (see above)
    // AI(W906-W7-L1-Wave0) 20260801: DELIBERATELY NOT ADDED -- `TTMyTray
    // *mtTrayMap` (golden cTrayMapping.h:388).  Its only reference in the whole
    // W7-L1 family is golden asendic_Loader.cpp:933, which sits inside the
    // `//`-commented-out block :929-936 (verified by reading those lines this
    // pass).  Adding it would be inventing surface nothing dereferences -- the
    // same rule that kept PPID / bNeedClearFile out in W7-F1.  Listed so it is not
    // later mistaken for an omission.
    // AI(W906-ST-S2-fShow-B1) 20260917: golden cTrayMapping.h `bool fShow;`.
    // Offline false, same reasoning as forms/fShuttleMove.h's fShow (read that
    // one for the full argument): there is no VCL form on screen in this target,
    // and the tree's two LIVE siblings (atester_shims.h:157 fContact->fShow,
    // forms/fTemp_Set.h:505 fTemp_Set->fShow) already hold exactly this value.
    //
    // NOTE THE CLASS: this is the SHIM TfTrayMapping that the GLOBAL
    // `fTrayMapping` is bound to (:313 below).  forms/fTrayMapping.h holds a
    // DIFFERENT class (TfTrayMappingForm) which was renamed precisely because
    // this shim took the name first -- see that header's banner.  Adding fShow
    // there instead would compile and never be read.
    bool fShow;

    TfTrayMapping();
};
extern TfTrayMapping *fTrayMapping;             // golden cTrayMapping.h: extern TfTrayMapping *fTrayMapping

// AI(W906-ST-S2-fShow-B1) 20260917: minimal TrayEditForm stand-in.
//
// golden's uTrayEditForm.h owns a real form; the whole tree references exactly
// TWO of its members -- `fShow` and `Close()` -- from one place (golden
// main.cpp:4716-4717, ported WebStart.cpp): "if the edit-tray window is open,
// close it when START is pressed".  Nothing reads a return value and nothing
// else on the START path touches this object.  So the honest facade is two
// members, not a form.
//
// CLASS NAME CARRIES A _Facade SUFFIX ON PURPOSE.  asendic_Auto.cpp:251 and
// asendic_Loader.cpp:189 both record `EditTray(int,int)` as "RETIRED BY: the
// wave that lands uTrayEditForm" -- i.e. a future wave brings in golden's real
// class.  Taking the bare name now would hand that wave the same class-name
// collision this file already caused once for TfTrayMapping (see
// forms/fTrayMapping.h's banner).  The GLOBAL keeps golden's spelling, because
// that is what the call site reads; only the type is marked as a stand-in.
//
// Close() is an offline no-op: there is no window to close.  It is a real
// out-of-line function rather than `{}` so that the day it must do something
// (tell the browser to leave that page) there is one place to put it.
struct TTrayEditForm_Facade
{
    bool fShow;                                  // golden uTrayEditForm.h -- form visible? (offline false)
    void Close();                                // golden -- offline no-op
    TTrayEditForm_Facade() : fShow(false) {}
};
extern TTrayEditForm_Facade *TrayEditForm;       // golden uTrayEditForm.h: extern PACKAGE TTrayEditForm *TrayEditForm

// ===========================================================================
//  MyMessageBox  -- golden mymessbox.h (TMyMessageBox*).  The buffer-place SM
//  closes it before raising a pre-alarm.  Offline: never visible -> Close() no-op.
// ===========================================================================
// AI(W906-FW-G24) 20260824: class body EXTRACTED to mymessbox_shim.h so
// SECSGEM/uHGemHT9045.cpp can reach MyMessageBox without this header's
// TColor consts / NewRecordProcess defaults (probe-verified conflicts).
// Same single definition, included back -- consumers unchanged.
#include "mymessbox_shim.h"

// ===========================================================================
//  LoadCCD  -- golden LoadCCDMap.h (TLoadCCD*).  DoCatchTray case 150/160 reads
//  the loader tray-map CCD.  Offline: LoadCCDData() returns 0 ("no remain IC,
//  proceed"); InitialLoadCCD() no-op.
// ===========================================================================
class TLoadCCDShim
{
public:
    int  LoadCCDData();     // golden LoadCCDMap.h -- offline: 0 (OK, no remain)
    void InitialLoadCCD();  // golden LoadCCDMap.h -- offline no-op
};
extern TLoadCCDShim *LoadCCD;                   // golden LoadCCDMap.h

// ===========================================================================
//  AMR  -- golden AMR.h (TAMR AMR).  DoPlaceTrayToAuto case 510 calls
//  AMR.SupplyCover(AutoTarget).
//  AI(W5-Automation-Integrate) 20260710: the REAL TTeraPowerAMR/AMR (Automation/
//  AMR.cpp) now exists and defines a global `AMR` object -- the TAMRShim stopgap
//  that used to live here has been REMOVED (it declared the SAME global name
//  `AMR` with a DIFFERENT type, which would ODR-collide/duplicate-symbol at link
//  once Automation/AMR.cpp joined this library). acatchtray.cpp now
//  `#include "Automation/AMR.h"` directly and calls the real AMR.SupplyCover(int)
//  (identical signature, so the one call site is unchanged).
// ===========================================================================

// ===========================================================================
//  Missing free functions the engine calls (no translated home this wave).
//  All offline-safe.  Golden homes noted.  These keep the verbatim SM bodies
//  linkable; their real bodies live in main.cpp / ainarm2.cpp / Magazine.cpp /
//  cSortCT.cpp / Automation (W7).
// ===========================================================================
//  in-arm position / picker helpers (golden ainarm9045.h / ainarm2.h / main.h)
bool MoveInArm2XYToWait();                       // golden -- offline: reached (true)
bool MoveInArm2XYToShuttle2Wait();               // golden -- offline: reached (true)
bool CheckInArmFinishAllPickerAction();          // golden -- offline: finished (true)
bool IsMoveInArm2XYToWait();                     // golden -- offline: at wait (true)
// W6.2c batch-4 (2x8_8): golden ainarm2.h:142 (Sam 20250522).  Offline: return FALSE
// so the golden guard `if(IsMoveInArm2XYToShuttle2Wait()==false) MoveInArm2XYToShuttle2Wait();`
// runs the offline-available move (the faithful not-already-at-wait path).
bool IsMoveInArm2XYToShuttle2Wait();             // golden ainarm2.h:142 -- offline: NOT at wait (false)
//  out-arm auto-alignment selector (golden aoutarm*.h)
bool CheckOutArmAutoAlignmentTrayModeBeUse(int iMode);  // golden -- offline false
//  single-motor home helpers for under-conveyor Z (golden mymotor/main)
void InitProcessSingleMotorTask(int iMot);       // golden uhome.cpp:397 -- AI(W906-SMHOME) 20260927: 已照翻（uhome.cpp 檔尾），不再是 no-op
bool ProcessSingleMotorHome(int iMot);           // golden uhome.cpp:415 -- AI(W906-SMHOME) 20260927: 已照翻（uhome.cpp 檔尾），不再一律回 true
//  AMR / Magazine cover-tray feed (golden AMR.h / Magazine.h)
int  SearchNeedAMRUnload(bool bAlarm);           // golden -- offline: -1 (nothing)
int  SearchNeedAMRUnloadCount();                 // golden -- offline: 0
AnsiString AMRUnloadBin(int iBinPos);            // golden -- offline: ""
bool DoMagazineAMRTrayFeed(int iWhichAuto);      // golden -- offline: done (true)
void InitialDoMagazineAMRTrayFeed();             // golden -- offline no-op
//  receive-tray task inits (golden asendic / main)
void InitAutoColorReceiveTask();                 // RETIRED BODY (W7-L1 Wave 1 "Color"): real body in asendic_Color.cpp
void InitAutoEmptyReceiveTask();                 // golden -- offline no-op
void DoAutoColor();                              // RETIRED BODY (W7-L1 Wave 1 "Color"): real VOID body in asendic_Color.cpp (golden asendic_Color.h:20)
//  unloader-info / teach-alignment (golden main / cSortCT)
void SetUnloaderInfoFile(int iUnloader);         // golden -- offline no-op  (also extern in acatchtray.cpp)
void InitDoOutArmTeachAlignmentProcessTask();    // golden -- offline no-op
//  production-log sink (golden cMyDB.h:62) -- body: the golden one in cMyDB.cpp (AI(W906-CMYDB-P4) 20260927 (St02-E); the empty stand-in in acatchtray_shims.cpp is gone)
void NewRecordProcess(AnsiString S1, AnsiString S2="", AnsiString S3="");  // golden cMyDB.h:62 is (AlarmCode, S, Debug=" "); the body turns Debug "" into " ", so a 2-arg call behaves the same

//  per-Auto receive-bin-tray flow (golden asendic / cSortCT) -- offline: false
void DoAutoReceiveBinTray(int iWhichAuto);       // RETIRED BODY (W7-L1 Wave 2 "Auto"): real VOID body in asendic_Auto.cpp (golden asendic_Auto.h:8)
//  per-Auto stack cylinder up/middle/lower helpers -- DECLARATIONS RETIRED
// AI(W906-W7-L1-Wave3) 20260802: the three AutoCylinderUp/Middle/Lower
// declarations that used to sit here (and the long caller-argument-order
// warning attached to them) have MOVED to asendic.h, together with the real
// golden bodies which now live in asendic.cpp -- the `{ return true; }` stubs
// in acatchtray_shims.cpp are retired.  They are not merely deleted from here:
// golden declares them in asendic.h (golden asendic.h:19-21) WITH default
// arguments on parameter 4, and every TU that calls them also includes
// asendic.h (acatchtray.cpp:90, asendic_Auto.cpp:122, asendic_Auto2.cpp:100,
// asendic_Auto_RT.cpp:201 -- all four re-verified in this pass), so a second
// defaulted declaration here would be a hard error ("default argument given for
// parameter 4 after previous specification") in each of those TUs.
// THE ARGUMENT-ORDER WARNING IS NOT LOST -- it is now in asendic.h next to the
// declarations, and tests/test_w7_l1_auto2.cpp SUB-TEST [12]
// (`test_argument_order_pin`) turns it into an executable assertion.
// AI(W906-W7-L1-W3fixB) 20260802: corrected -- this line used to cite
// `tests/test_w7_l1_wave3_argorder.cpp`, a file that does not exist anywhere in
// the tree.  The coverage does exist; only the pointer was wrong.
//  motor scale-speed setter + BinDisp warn-clear (golden main / MyBinDisp) -- no-op
void SetMotorScaleSpeed(int iMot, int iSp);      // golden -- no-op
// AI(W906-AutoCleanFoundation) 20260721: golden cinitial.h:51 sibling of
// SetMotorScaleSpeed just above (same golden module, cinitial.cpp -- not yet
// translated anywhere in this tree). AutoClean/AutoClean.cpp's CleanSetSpeed
// (in-scope this wave) calls both; mirrors the exact no-op idiom already
// established for its sibling one line up.
void SetMotorAccelSpeed(int iMot, int iSp);      // golden cinitial.h:51 -- no-op
void ClearAutoChangingWarn(int iAuto);           // golden MyBinDisp -- no-op

// ===========================================================================
//  eATkTrayFeed enum members the count helper reads (golden Automation/AGV.h:212).
//  AddTrayCount compares LastSet.iUnloadFixTray against these two; AGV.h is not
//  in scope this wave, so mirror the two ordinals verbatim.
// ===========================================================================
//  eAtkTfFeedFix / eAtkTfFixToAMR / eAtkTfPutIDTray / eAtkTfPutCover /
//  eAtkTfPutEmptyTray (golden Automation/AGV.h:215-220) are defined directly in
//  acatchtray.cpp (it owns those mirrors this wave) -- NOT here, to avoid a
//  duplicate-enumerator clash.

// ===========================================================================
//  Missing file-scope globals the RFID SM reads (golden cmydef / cTrayMapping).
// ===========================================================================
extern int iCoverTrayIDTask[];                   // golden -- per-port cover-tray-ID SM cursors
extern int iReadCIDAction;                       // golden -- which-port-reading-CID action
//  NOTE: bPlaceToHotplate / bPlaceShuttle / iInArmWaitPosition / iInRotateFinish
//  are defined directly in acatchtray.cpp (it owns them this wave).

#endif // acatchtray_shimsH
