//---------------------------------------------------------------------------
//  Interface/TesterTCP.h -- bounded subset #2 of the golden TfTesterTCP form
//    (Interface/TesterTCP.cpp/.h, golden 1117 lines), a Tester "OS/Open-Short"
//    channel used only by CUSTOMER_CODE==CC_JSCC_OS.
//
//  AI(W906-D-Interface-TesterTCP) 20260716: new file. This is a DIFFERENT,
//  SIBLING file from the already-translated Interface/TesterTCP_Socket.h/.cpp
//  (that unit covers ClientSocket_TCPIP connect/disconnect/error/read +
//  TimerTCPIPConnectTimer + SendTCPIPCommand/AddTCPIPCommunicationLog +
//  btTCPIP_Trigger/Connect/DisconnectClick -- golden TesterTCP.cpp:166-345 +
//  615-633). This file does NOT touch that socket-management subset at all;
//  it covers a completely separate, zero-widget-coupling family: the OS
//  recipe zip-transfer-to/from-Tester pair, plus the per-machine
//  arm-speed-save click handler. Both files together still cover only a
//  fraction of the golden 1117-line TfTesterTCP form -- see the NOT
//  ATTEMPTED list below (this file) and TesterTCP_Socket.h's own list (that
//  file) for the remainder.
//
//  SCOPE (this wave, recon-selected -- 3 functions, all confirmed on a full
//  read to have NO VCL widget reads once the 4 identified inputs are
//  promoted to parameters):
//    CopyRecipeToTester    golden TesterTCP.cpp:1058-1077 (Steven 20250612 "for OS Tester.")
//    CopyRecipeFromTester  golden TesterTCP.cpp:1079-1097 (Steven 20250612 "for OS Tester.")
//    btnSaveClick          golden TesterTCP.cpp:1099-1115 (RogerYang 20260210)
//
//  AI(W906-W7-L4) 20260802: added a 4th function:
//    CopyOSTestResult      golden TesterTCP.cpp:660-697 (38 lines).
//  The historically documented blocker (a cross-file gap in
//  Public/MyProductionRecord.cpp's GetSiteNo/GetOrderOfContact) is STALE --
//  both exist with golden-matching semantics.
//  AI(W906-W7-L4-fix) 20260802: CITATION CORRECTED. The line numbers this
//  note used to carry ("Public/MyProductionRecord.cpp :666/:671, verified
//  against golden .cpp's own :666/:671") attributed PORTED line numbers to
//  golden. Re-verified this pass, both trees read directly:
//    GetSiteNo         PORTED Public/MyProductionRecord.cpp:666-669
//                      GOLDEN Public/MyProductionRecord.cpp:519-522
//    GetOrderOfContact PORTED Public/MyProductionRecord.cpp:671-674
//                      GOLDEN Public/MyProductionRecord.cpp:524-527
//  GOLDEN Public/MyProductionRecord.cpp:666-673 is NOT either of them -- it
//  is the unrelated TMyProductionRecord::SaveRecord (golden :660-...). The
//  bodies do match: golden `atoi(asBuffer->Strings[eSiteNO].c_str())` vs
//  ported `atoi(asBuffer->GetString(eSiteNO).c_str())` (same for
//  eOrderTest). Like the 3 functions
//  above, CopyOSTestResult has NO VCL widget reads at all (only
//  TestIF_File/LastSet/TestSocket globals + file I/O) so it needed no
//  parameter-promotion. CopyOSTestResult lands with NO production caller in
//  this tree (golden's own callers are themselves untranslated) -- see the
//  banner at the function body in TesterTCP.cpp for what would make it
//  reachable.
//
//  NOT attempted this wave (per this front's task brief, confirmed by recon's
//  dependency checks, not assumption):
//    - ctor / FormDestroy / FormShow / rgUnloaderClick (~140 widget-member
//      construction cluster, no FormsFacade home, near-zero unique logic).
//    - PlaceOSTestResultToTray (golden :699-739) -- EXPLICITLY OUT OF SCOPE
//      this wave: it dereferences two real on-screen VCL widgets (SourceMemo,
//      a TMemo*, and RichEdit[eTrayCount], despite its name an ARRAY of
//      dynamically created TMemo*). The ported Interface/TesterTCP.h has NO
//      TfTesterTCP class or facade at all, only free functions -- standing up
//      a whole form facade for two display-only memos is UI-axis work
//      (belongs to wave W7-U), not a small addition alongside a file-I/O-only
//      leaf like CopyOSTestResult.
//    - ProcessOSPrint / ProcessOSTrayData (heavy TRichEdit/TMemo + two-form
//      widget coupling, OS-report generation, not a live protocol path, low
//      urgency).
//    - [RESOLVED -- AI(W906-TesterTCPTimer) 20260720] TimerProcessTCPDataTimer
//      (the file's true protocol-decode core) and SimulateBin (previously
//      listed here as not-yet-attempted, alongside ProcessOSPrint/
//      ProcessOSTrayData above) are now both translated by the
//      W906-TesterTCPTimer wave, in the SIBLING file
//      Interface/TesterTCP_Socket.{h,cpp} (not this file) -- see that file's
//      own SCOPE list and DESIGN_TesterTCP_TimerProcessTCPDataTimer.md. The
//      "needs 5 new TfMain FormsFacade members" estimate below this note used
//      to carry turned out to be 3 members + 1 method once resolved (see that
//      design doc's own recon: cbSetupFileName was already added by the
//      2026-07-16 W906-Automation wave).
//    - ClientSocket_TCPIP* / TimerTCPIPConnectTimer / SendTCPIPCommand /
//      AddTCPIPCommunicationLog / btTCPIP_*Click (already covered by the
//      sibling Interface/TesterTCP_Socket.h/.cpp -- do not duplicate here).
//
//  extract-calc-core widget promotions (this wave):
//    btnSaveClick's golden body reads 3 TCheckBox->Checked (chkAuto1_OS/
//    chkAuto2_OS/chkAuto3_OS) and the form member LastFileName -- none of
//    those widgets/members exist yet in any translated form, and this file's
//    own write scope is Interface/TesterTCP.{h,cpp} + tests/test_TesterTCP.cpp
//    only (no FormsFacade edits permitted this wave) -- so, matching this
//    project's established extract-calc-core precedent (e.g.
//    TesterTCP_Socket.h's btTCPIP_TriggerClick promoting edTCPIP_CMD->Text to
//    a parameter), all 4 reads become explicit function parameters. A future
//    real TfTesterTCP UI wave should wire the 3 TCheckBox->Checked reads and
//    the LastFileName member straight through to these same parameters.
//---------------------------------------------------------------------------
#ifndef TesterTCP_MainH
#define TesterTCP_MainH

#include "vclcompat/vcl_compat.h"     // AnsiString

//---------------------------------------------------------------------------
//  golden TfTesterTCP::CopyRecipeToTester(AnsiString FileName), TesterTCP.cpp:1058-1077.
//  Steven 20250612 "for OS Tester."; Steven 20250327/20230710: OS Tester's own
//  work-file must also be uploaded (zips DataPath+FileName into OS_Setting.zip
//  under IniConfig.asN06_TesterPath, gated by IniConfig.bN06_CopyTesterFile).
//---------------------------------------------------------------------------
void TesterTCP_CopyRecipeToTester(AnsiString FileName);

//---------------------------------------------------------------------------
//  golden TfTesterTCP::CopyRecipeFromTester(AnsiString FileName), TesterTCP.cpp:1079-1097.
//  Steven 20250612 "for OS Tester."; the inverse direction of the pair above
//  (re-zips DataPath+FileName's OS_Setting.zip to append the Tester-side
//  <FileName>.ini found under IniConfig.asN06_TesterPath).
//---------------------------------------------------------------------------
void TesterTCP_CopyRecipeFromTester(AnsiString FileName);

//---------------------------------------------------------------------------
//  golden TfTesterTCP::btnSaveClick(TObject *Sender), TesterTCP.cpp:1099-1115.
//  RogerYang 20260210: per-machine arm-speed save request -- if enabled the
//  target directory becomes sSaveByMachine (per-machine shared save), else
//  DataPath+LastFileName (per-recipe save); writes 3 JSCC_OS_Use AutoN flags
//  into <dir>\ArmCondition.Data. extract-calc-core: the 3 golden
//  chkAutoN_OS->Checked widget reads and the golden LastFileName form-member
//  read are promoted to explicit parameters (see file banner).
//---------------------------------------------------------------------------
void TesterTCP_btnSaveClick(bool bChkAuto1_OSChecked,
                             bool bChkAuto2_OSChecked,
                             bool bChkAuto3_OSChecked,
                             AnsiString LastFileName);

//---------------------------------------------------------------------------
//  golden TfTesterTCP::CopyOSTestResult(int iSuckRow, int iSuckCol), golden
//  TesterTCP.cpp:660-697 (38 lines). Steven 20230116 "OS測試機準備送出工作檔名"
//  (prepares the work-file name the OS Tester is about to send)/Steven
//  20231113 "修正TCP/IP Test為Off Line" (fix for TCP/IP Test being Off Line).
//
//  Gated on TestIF_File.iTestType==TCP_IP_MODE and (outside SOFT_SIMULTE)
//  LastSet.iTester==ON_LINE; on pass-through it force-creates
//  D:\HT9045_Log\OSTestResult and, for each of the Device<N>.TXT /
//  FailPin_Device<N>.TXT source files (Z:\ normally, D:\RMS\ under
//  SOFT_SIMULTE) that a real, openable-for-read source is confirmed for
//  (golden's own CheckFileExist, gated dead in this ported tree -- see
//  TesterTCP.cpp's Gated_CheckFileExist), CopyFile()s it to
//  Device<contact index, %06d>_<tester channel, %02d>.TXT under that folder.
//  iTesterCh = TestSocket.PordRec[iSuckRow][iSuckCol].GetSiteNo()-1 (1-based
//  site number -> 0-based channel, golden :671); iContactIndex =
//  ...GetOrderOfContact() (golden :672).
//
//  AI(W906-W7-L4) 20260802: NO production caller in this tree -- golden's own
//  callers of CopyOSTestResult are themselves not yet translated. See the
//  reachability banner at this function's body (TesterTCP.cpp) for what would
//  make it reachable.
//---------------------------------------------------------------------------
void TesterTCP_CopyOSTestResult(int iSuckRow, int iSuckCol);

//===========================================================================
//  AI(W906-W7-L4-fix) 20260802: TEST-OBSERVABLE SEAMS around the three
//  environment-touching calls TesterTCP_CopyOSTestResult makes
//  (MyForceDirectories / CheckFileExist / CopyFile).
//
//  WHY THIS EXISTS. The first version of tests/test_w7_l4_copyostestresult.cpp
//  could only observe "did D:\HT9045_Log\OSTestResult appear on this disk".
//  That is nearly content-free: this machine has no Z: drive, so
//  CheckFileExist("Z:\Device<N>.TXT") is false whatever <N> is, CopyFile never
//  runs, and every assertion about the copy was vacuously true. Two real
//  mutations survived it -- dropping the golden `-1` from
//  `iTesterCh = GetSiteNo()-1` (golden Interface/TesterTCP.cpp:671), and
//  deleting the `if(CheckFileExist(...))` guard so CopyFile runs
//  unconditionally (golden :681). The observable that distinguishes those
//  mutants is the TARGET FILENAME (`Device%06d_%02d.TXT`, golden :683/:694),
//  which only exists as an argument to CopyFile. So the three calls now route
//  through recording seams, and a test can install a hook to drive the copy
//  branch with no Z: drive and no filesystem writes at all.
//
//  IDIOM. Same shape as Automation/SCK_ART_Remainder.{h,cpp}'s
//  W5SckArtRem_LastShellExecuteOpenPath / W5SckArtRem_LastFTPUpload_* captures
//  (a stand-in that records what it was asked to do into a file-scope variable
//  a test can read), with one deliberate difference: SCK_ART's gates NEVER do
//  the real thing, whereas these seams ALWAYS do the real thing unless a test
//  explicitly installs a hook.
//
//  PRODUCTION BEHAVIOUR (hooks all NULL, which is their static-init state and
//  the only state production ever sees): each seam calls exactly the same real
//  function with exactly the same arguments and returns exactly its result --
//  real Gated_MyForceDirectories, real Gated_CheckFileExist, real CopyFile.
//  The ONLY added effect is the recording itself: a few assignments to the
//  file-scope variables below. No I/O, no allocation-visible behaviour change,
//  no control-flow change. tests/test_w7_l4_copyostestresult.cpp PART 7 proves
//  the pass-through empirically (not by assertion) through the three
//  W7L4TesterTCP_Prove*Seam entry points below.
//
//  Nothing else in this file routes through the seams: TesterTCP_btnSaveClick
//  still calls Gated_MyForceDirectories directly, exactly as before, so the
//  recordings below describe CopyOSTestResult and nothing else.
//===========================================================================

// Capacity of the per-call recording arrays. CopyOSTestResult makes at most 2
// existence probes and at most 2 copies; 4 leaves headroom without hiding an
// overflow (the *Count variables below keep counting past the array, so a test
// asserting on the count still sees the truth).
#define W7L4TESTERTCP_MAX_RECORDED 4

// --- what the seams recorded, last call of TesterTCP_CopyOSTestResult -------
extern AnsiString W7L4TesterTCP_LastForceDirectoriesPath;      // golden :674 1st argument
extern AnsiString W7L4TesterTCP_LastForceDirectoriesFunction;  // golden :674 2nd argument
extern int        W7L4TesterTCP_ForceDirectoriesCount;

extern AnsiString W7L4TesterTCP_CheckFileExistPath[W7L4TESTERTCP_MAX_RECORDED];  // golden :681/:692 probes, in order
extern int        W7L4TesterTCP_CheckFileExistCount;

extern AnsiString W7L4TesterTCP_CopyFileSource[W7L4TESTERTCP_MAX_RECORDED];      // golden :684/:695 1st argument
extern AnsiString W7L4TesterTCP_CopyFileTarget[W7L4TESTERTCP_MAX_RECORDED];      // golden :684/:695 2nd argument
extern bool       W7L4TesterTCP_CopyFileFailIfExists[W7L4TESTERTCP_MAX_RECORDED];// golden :684/:695 3rd argument
extern int        W7L4TesterTCP_CopyFileCount;

// Clears every recording variable above back to its "<unset>"/0 state.
void W7L4TesterTCP_ResetSeamRecording();

// --- test-installable hooks (all NULL == production) ------------------------
// A NULL hook means "do the real thing"; a non-NULL hook fully REPLACES the
// real call (the real function is not invoked at all). A test that installs a
// hook is responsible for setting it back to NULL.
typedef int  (*W7L4TesterTCP_ForceDirectoriesHookFn)(AnsiString Directory, AnsiString Function);
typedef bool (*W7L4TesterTCP_CheckFileExistHookFn)(AnsiString cFName);
typedef bool (*W7L4TesterTCP_CopyFileHookFn)(AnsiString sSource, AnsiString sTarget, bool bFailIfExists);

extern W7L4TesterTCP_ForceDirectoriesHookFn W7L4TesterTCP_ForceDirectoriesHook;
extern W7L4TesterTCP_CheckFileExistHookFn   W7L4TesterTCP_CheckFileExistHook;
extern W7L4TesterTCP_CopyFileHookFn         W7L4TesterTCP_CopyFileHook;

// --- pass-through provers ---------------------------------------------------
// Thin exported wrappers that invoke THE SAME seam functions the production
// body of TesterTCP_CopyOSTestResult invokes (the seams themselves are
// TU-local, per this project's gate idiom, so a test cannot reach them
// otherwise). They exist so the "hook NULL -> the real function really runs"
// claim can be demonstrated on test-chosen scratch paths instead of merely
// asserted -- CopyOSTestResult's own paths are golden-hardcoded to
// D:\HT9045_Log\OSTestResult and Z:\, neither of which a test may write to.
// NOT called by any production path.
int  W7L4TesterTCP_ProveForceDirectoriesSeam(AnsiString Directory, AnsiString Function);
bool W7L4TesterTCP_ProveCheckFileExistSeam(AnsiString cFName);
bool W7L4TesterTCP_ProveCopyFileSeam(AnsiString sSource, AnsiString sTarget, bool bFailIfExists);

#endif // TesterTCP_MainH
