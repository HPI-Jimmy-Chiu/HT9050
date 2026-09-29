// =============================================================================
//  canary_support.h  --  W6.1 canary-support scaffold (Empty-tray SM)
//
//  Translation wave: W6.0 (canary-support scaffold for asendic_Empty)
//  Translator: AI(W6.0-SCAFFOLD) 20260626
//
//  Collects the small substrate/scaffold gaps the W6.1 Empty-tray canary needs
//  that are NOT yet in the translated global headers:
//
//    1. __FUNC__ macro
//       BCB6 compiler builtin (current function name as a string).  Standard C++
//       spells it __func__.  asendic_Empty.cpp passes __FUNC__ to functions that
//       take AnsiString (which has a const char* ctor), so __func__ works.
//
//    2. LAST_GENERAL_SET / LastSet (MINIMAL shim)
//       The golden LastSet.h (1162 lines, LAST_GENERAL_SET) is gated out of the
//       target (MachineDefine.h #if 0 block).  The canary touches ONLY
//       LastSet.iRealDummy (23 refs, all comparisons vs DUMMY/REALLY).  This is
//       a MINIMAL shim with just iRealDummy.  TODO(W6.x): replace with the full
//       translated LastSet.h when the LastSet subsystem lands.
//
//    3. Free-function decls the canary calls that are not in a shared target
//       header:
//         ShowErrorMessage  (golden note.h)        -- drives Retry/Skip in the SM
//         RecordProcess     (golden cMyDB.h)
//         ShowUnloaderTrayMessage (golden mymessbox.h)
//         ReadWriteTrayID   (golden csystem.h -- already declared via csystem.h)
//         WhichAutoNeedTray (golden acatchtray.h)
//         ShowMyMessage     (golden mymessbox.h -- matches database.cpp local decl)
//       Sim bodies live in canary_support.cpp.
//
//  EventReport (SECS) is NOT declared here -- the single EventReport call in the
//  canary is gated #if 0 (W7-SECS).  MyMessageBox (VCL) is NOT declared here --
//  the pre-alarm block that uses it is gated #if 0 (W6.x).
// =============================================================================
#ifndef canary_supportH
#define canary_supportH

#include "vclcompat/vcl_compat.h"   // AnsiString

// ---------------------------------------------------------------------------
//  1. __FUNC__  (BCB6 builtin -> standard __func__)
// ---------------------------------------------------------------------------
#ifndef __FUNC__
#define __FUNC__ __func__
#endif

// ---------------------------------------------------------------------------
//  2. LAST_GENERAL_SET / LastSet -- full translation (GA-1 B1), shim RETIRED
// ---------------------------------------------------------------------------
// AI(W906-GA1-B1) 20260804: the MINIMAL 66-field shim that lived here since
// W6.2 is retired.  The full golden LastSet.h (855 fields across 6 structs +
// enum, token-stream-identical to golden) now provides LAST_GENERAL_SET and
// the extern LastSet/Tech/CmdData/AlignTeach objects; definitions live in
// LastSet.cpp (added to ht9045_sm, the same library that carried the shim's
// global, so ht9045_kyecftp's sm+core dual-link stays single-definition).
// Shim->full compatibility was verified field-by-field: all 66 shim fields
// match golden name+type+dims exactly (script-checked, _ga1_b1_report.md).
#include "LastSet.h"

// ---------------------------------------------------------------------------
//  3. Free-function decls (golden homes noted) + sim bodies in canary_support.cpp
// ---------------------------------------------------------------------------
//  ShowErrorMessage -- golden note.h:466.  In the sim it returns the Kxxx code
//  that drives the SM's Retry/Skip branch (default K_RETRY).  Signature VERBATIM.
int  ShowErrorMessage(AnsiString Code, int KCode, int Pos,
                      bool bDuplicateErr=false, AnsiString errPart=" ");         //Steven 20091231

//  RecordProcess -- golden cMyDB.h:63.  Body: the golden one in cMyDB.cpp (AI(W906-CMYDB-P4) 20260927 (St02-E); was a stdout sim).
void RecordProcess(AnsiString S, AnsiString S2="");

//  ShowUnloaderTrayMessage -- golden mymessbox.h:60.  Sim: logs (no UI).
void ShowUnloaderTrayMessage(AnsiString S1, AnsiString S2);                      //Jou 20150721

//  WhichAutoNeedTray -- golden acatchtray.h:8.  Sim: 0 (no auto needs a tray).
int  WhichAutoNeedTray();

//  ShowMyMessage -- golden mymessbox.h:58.  Signature matches the database.cpp
//  local forward-decl (S3 defaulted) so the two TUs agree.  Sim: logs (no UI).
void ShowMyMessage(AnsiString S1, AnsiString S2="", AnsiString S3="",
                   bool Ok=false, bool bServoOff=false);

// ---------------------------------------------------------------------------
//  4. AI(W906-W7-L1-Wave0) 20260801 -- OBSERVABILITY SEAM for the two operator
//     dialogs above.  ONE seam set, landed once, in the serialized pass.
//
//  WHY IT CANNOT BE DEFERRED.  canary_support.cpp's ShowErrorMessage returned
//  K_RETRY UNCONDITIONALLY.  Every K_SKIP / K_CLEAN_OUT recovery arm reached
//  through it is therefore structurally unreachable, so any "the skip path is
//  covered" claim about those arms is unfalsifiable -- precisely the tautology
//  class this project has already paid for on this file family.  With
//  W906_ShowErrorMessage_SimReturn a test can feed K_SKIP and actually walk the
//  arm; with the count/last-code fields it can also prove the alarm was raised at
//  all, and with WHICH code.  (The W7-L1 recons put the affected arm count in the
//  low tens across the asendic_* family -- that figure is THEIRS and was not
//  re-counted in this pass, so treat it as an estimate, not a measurement.  The
//  structural argument above does not depend on the number.)
//
//  WHY IT IS ONE SEAM AND NOT SIX.  Five separate recon agents each proposed
//  their own version under three different names.  123 files in this tree carry
//  an `#include "canary_support.h"` line (counted this pass; a looser grep for
//  the mere string reports 141, which is why the exact figure is stated here), so
//  six competing seams would be six collisions on one very widely included
//  header.  Naming follows the established `W906_..._Sim` idiom named in the
//  forms/fMain.h facade contract (rule 4).
//
//  HEADER COST: deliberately zero new includes.  These are `extern` declarations
//  only; K_RETRY (cmydef.h:276, `extern const int`, value 0x0001 at cmydef.cpp:337)
//  is applied in canary_support.cpp, which already includes cmydef.h.
//
//  NOTE ON THE ShowMyMessage HALF: golden's ShowMyMessage returns void, so there
//  is no "next return" to make settable -- it gets the other three (last-argument
//  capture, call count, reset) and nothing more.  That asymmetry is golden's, not
//  an omission.
// ---------------------------------------------------------------------------
//  ShowErrorMessage seam
extern int        W906_ShowErrorMessage_SimReturn;  // next return value; RESET VALUE IS K_RETRY (the pre-seam behaviour)
extern AnsiString W906_ShowErrorMessage_LastCode;  extern AnsiString W906_ShowErrorMessage_LastErrPart;   // last `Code` argument seen; "" when never called  //AI(W906-SHOWERR) 20260929: + LastErrPart = the last errPart (golden ShowErrorMessage 5th parameter, note.cpp:532), "" when never called; tools/wb_serve.cpp ForwardShowErrorMessage passes it to W906_ShowErrorMessageRecordLikeGolden (forms/fNote_ShowError.cpp) for MyDBIEvent / ErrShowToForm
extern int        W906_ShowErrorMessage_LastKCode;  extern bool W906_ShowErrorMessage_LastDuplicate;  /* AI(W906-J2) 20260926: last bDuplicateErr (golden note.cpp:802-803 iDuplicateError=1); read by the FormClose Jam count (forms/fNote_JamCount.cpp) */  // last `KCode` mask seen (which buttons golden offered); 0 when never called
extern int        W906_ShowErrorMessage_Count;      // call count -- distinguishes "not called" from "called and returned K_RETRY"
void W906_ShowErrorMessage_Reset();                 // restore SimReturn to K_RETRY, clear capture + count
//  AI(W906-FW-W5b) 20260819: optional ANSWER hook -- lets a host process
//  (today: tools/wb_serve) put the K_RETRY/K_SKIP/K_CLEAN_OUT question to a
//  real operator (browser modal) and return the chosen K code.  Returning 0
//  means "no answer taken" and falls back to W906_ShowErrorMessage_SimReturn,
//  so every existing test keeps its settable-seam behaviour even with a hook
//  installed.  Raw char* + ints: zero new includes (same rule as the
//  ShowMyMessage hook above).  Null (the default) = unattended sim unchanged.
extern int (*W906_ShowErrorMessage_Hook)(const char* Code, int KCode, int Pos);
// ---------------------------------------------------------------------------
//  AI(W906-P6b-C) 20260921: 視窗狀態總表 -> `Bit4_HandlerDiagnostics` 的 seam
//
//  golden `Command.cpp:7348-7360`（本樹 `Command.cpp:15027` 的 `GATE(FW3-WF)`）
//  用 ~26 個 `fXxx->fShow` 判斷「有沒有診斷/設定畫面開著」，再把結果送進 GPIB
//  的 `Bit4_HandlerDiagnostics`。移植樹的 UI 是網頁，那些 `fShow` 沒人維護。
//  P6-b 的視窗狀態總表（`WebWindowRegistry.h`）是它的替代訊號源。
//
//  ## 為什麼是 hook，不是把 WebWindowRegistry.cpp 搬進 library
//
//  `Command.cpp` 在 `ht9045_sm`；`WebWindowRegistry.cpp` 只在 `wb_serve` 目標裡
//  （`CMakeLists.txt:3168`），連不到。兩條路：
//    (a) 把 `WebWindowRegistry.cpp` 移進 library
//    (b) 在 `ht9045_sm` 開一個函式指標 seam，由宿主行程安裝
//
//  選 (b)，理由不是「比較小」而是**方向**：(a) 會讓狀態機這一層開始相依
//  cJSON 與瀏覽器契約，那與這個專案的架構（web UI 在外、C++ 控制在內）相反。
//  狀態機不該知道有瀏覽器這回事，它只該知道「有人告訴我診斷畫面開著沒」。
//  ⇒ 與本檔上面兩個 `W906_*_Hook` 完全同一個慣例（`tools/wb_serve.cpp:2412/:2417`
//     安裝、`:3309` 卸載），沒有新增任何 include。
//
//  ## 語意
//
//  回傳 true = 「現在有診斷/設定畫面開著」= golden 那個 `if` 為真。
//  **NULL（預設）= 沒有人安裝 = 維持今天的行為**（`Bit4` 恆 0）。
//  ⇒ 任何沒有安裝它的宿主（所有測試、未來的 MFC harness）行為完全不變。
//
//  ## ⚠ 執行緒
//
//  總表沒有鎖。今天安全，因為在 `wb_serve` 裡**寫**（`ui.windows.put`）與
//  **讀**（這個 hook）都發生在同一條 tick 執行緒上：指令是從 socket 執行緒
//  「入列」，但**分派**在 tick 上（`tools/wb_serve.cpp:2386-2389` 的設計註解，
//  `:2454` 的 `cmdQueue.drain`）。
//  ⚠ 哪天 `MachineStatus()` 改成在別的執行緒上跑，這條就不成立了，
//    那時要在 `WebWindowRegistry.cpp` 裡加鎖，不是在這裡。
extern bool (*W906_DiagnosticsWindowOpen_Hook)(bool systemStart, int contactMode);
//  ShowMyMessage seam
extern AnsiString W906_ShowMyMessage_LastS1;        // last `S1` argument seen; "" when never called
extern int        W906_ShowMyMessage_Count;         // call count
void W906_ShowMyMessage_Reset();                    // clear capture + count
//  AI(W906-FW-W5a) 20260819: optional forward hook -- lets a host process
//  (today: tools/wb_serve) SEE each ShowMyMessage as it happens, e.g. to push
//  it to a browser as an info modal (design doc section 4; golden returns
//  void, so this stays display-only -- no answer flows back).  Raw char*
//  on purpose: zero new includes on a header 123 TUs carry.  Null (the
//  default) = recording behaviour unchanged.
extern void (*W906_ShowMyMessage_Hook)(const char* S1, const char* S2);

// ---------------------------------------------------------------------------
//  AI(W906-YESNO) 20260925: ShowMyMessageBox_YES_NO -- golden mymessbox.h:55,
//  body mymessbox.cpp:1009-1062.  使用者 20260925 裁決（RULINGS_20260925.md
//  第 10 條，3A）：「YES/NO 對話框被替身自動回答 → 照 golden 跳網頁對話框，
//  等操作員回答」。
//
//  ## 在這之前
//  全樹沒有本體，五個 TU 各自放一對 `static int W?_ShowMyMessageBox_YES_NO(...)
//  { return 0; }` ＋ `#define`（OmronLaser/LaserSensorShuttle.cpp、atester.cpp、
//  cinitial.cpp、csystem.cpp、PowerSavingMode.cpp）。0 不是 golden 的任何答案
//  （golden 只回 1＝Yes／2＝No／3＝框已開著），於是判 `ret==2` 才算拒絕的呼叫點
//  （例：「確定要儲存測距數值？」）**不問就當作同意**。那五對替身已拿掉，
//  全部改呼叫這一個。
//
//  ## 回傳值（golden 語意，逐字）
//    1 = Yes（pnlYes Tag=1，mymessbox.dfm:160）
//    2 = No （pnlNo  Tag=2，mymessbox.dfm:141）
//    3 = 已有 MyMessageBox 開著且 iUnLoaderCount==0（mymessbox.cpp:1012-1015）
//
//  ## 誰來回答
//  與 ShowErrorMessage 同一個慣例：宿主行程（今天是 tools/wb_serve）安裝
//  W906_ShowMyMessageBoxYesNo_Hook，把問題送到網頁、在 tick 執行緒上等操作員按
//  是／否，回 1 或 2。hook 回 0 ＝「沒有人能回答」（沒裝 hook、或 wb_serve 的
//  網頁伺服器還沒起來 —— 例如開機時 cinitial.cpp 那四個呼叫），此時回
//  W906_ShowMyMessageBoxYesNo_SimReturn。
//  ⚠ SimReturn 的重設值刻意是 **0**（＝拿掉的那五個替身原本回的值），不是 1 也
//    不是 2：沒有操作員的宿主（單元測試、開機前段）行為與 20260925 之前逐位元相同。
//    golden 沒有「預設鈕」可以照抄（MyMessageBox 的 Yes/No 是 TPanel，不是
//    ModalResult 按鈕；golden 的 iValue 是全域、不在呼叫前歸零）。
//
//  Raw char* hook：本標頭零新增 include（與上面兩個 hook 同一條規則）。
int  ShowMyMessageBox_YES_NO(AnsiString S1, AnsiString S2, AnsiString S3="");   // golden mymessbox.h:55（S3=NULL）
extern int        W906_ShowMyMessageBoxYesNo_SimReturn;  // 沒人回答時的回傳；RESET VALUE IS 0（替身時代的值）
extern AnsiString W906_ShowMyMessageBoxYesNo_LastS1;     // 最後一次的 S1；從沒呼叫過是 ""
extern AnsiString W906_ShowMyMessageBoxYesNo_LastS2;     // 最後一次的 S2
extern int        W906_ShowMyMessageBoxYesNo_Count;      // 呼叫次數
void W906_ShowMyMessageBoxYesNo_Reset();                 // SimReturn 回 0、清 capture 與計數
extern int (*W906_ShowMyMessageBoxYesNo_Hook)(const char* S1, const char* S2, const char* S3);

// ---------------------------------------------------------------------------
//  5. AI(W906-W7-L2) 20260803 -- the three golden note.h FREE FUNCTIONS that
//     ckernel.cpp's motor/cylinder alarm path calls.  note.cpp is NOT ported
//     (307 KB, TfNote form unit), so these have no home anywhere in the tree --
//     verified this pass: `grep -rn` for all three over the whole ported tree
//     returned ZERO hits before this edit.
//
//  Header line numbers verified by re-reading the cp950-decoded golden note.h:
//     :467  void       ShowMotorErrorMessage(AnsiString, int, AnsiString="")
//     :468  AnsiString MotorIndexToJamCode(int)
//     :470  AnsiString CylinderIndexToJamCode(int, int*)
//
//  ROUTE TAKEN PER FUNCTION (each decided by reading the golden BODY, not the
//  declaration):
//
//   * MotorIndexToJamCode  -> REAL TRANSLATION.  Golden note.cpp:4291-4296 is
//     four lines, VCL-free and side-effect-free: `S.sprintf("WAR24%03d", MotNo)`.
//     A real body beats a sim body and is directly unit-testable.
//
//   * CylinderIndexToJamCode -> REAL TRANSLATION (of the mapper), with ONE
//     documented omission.  Golden note.cpp:4156-4284 is a pure 20-branch
//     cylinder-index -> owning-unit table plus `S.sprintf("JAM%d", Code)`; all
//     71 constants it needs (C_* cylinder ids, MM*/M* unit ids) already exist in
//     this tree as `extern const int` in cmydef.h -- each of the 71 was checked
//     individually this pass, zero missing.  The omission is golden's opening
//     `for(int i=0;i<6;i++) fNote->Select[i]=false;` (note.cpp:4161-4162): the
//     ported TfNote (forms/fNote.h) has no `Select[]` member, that file is NOT
//     owned by this front, and the field is a pure note-DIALOG button pre-clear
//     (golden note.h:396 `bool Select[10];` //ChungHung HT9045 2011/12/13
//     //Input pickup device error時,按"retry"鍵,機台都會自動home add BtnHome).
//     Nothing in ProcessAlarm reads it -- the function's entire contract with
//     golden ckernel.cpp:2522-2523 is the returned JAM string and *Pos, and BOTH
//     are translated faithfully.  See the report's integrator notes.
//
//   * ShowMotorErrorMessage -> RECORDING SIM (mandated, and independently
//     correct: golden note.cpp:1052+ drives StopAllMotor, Galil "ST", the note
//     form, MyDBIEvent, fFTPClient and fAutomation).  It is NOT a bare no-op --
//     see canary_support.cpp for the three global side effects and the two
//     control-flow arms that ARE reproduced faithfully.
// ---------------------------------------------------------------------------
//  golden note.h:468 -- REAL body (golden note.cpp:4291).  "WAR24%03d" % MotNo.
AnsiString MotorIndexToJamCode(int MotNo);

//  golden note.h:470 -- REAL mapper (golden note.cpp:4156).  //Steven 20231127 :
//  氣缸Alarm改成自動生成, 分類31.  Writes the owning unit id through *Pos and
//  returns "JAM<Code>".  *Pos is ALWAYS written (golden's final else, :4277-4280,
//  defaults it to MMSystem), so callers may leave it uninitialised, as golden
//  ckernel.cpp:2505 does.
AnsiString CylinderIndexToJamCode(int Code, int *Pos);

//  golden note.h:467 -- RECORDING SIM.  Signature VERBATIM (errPart defaulted).
void ShowMotorErrorMessage(AnsiString Code, int MotorAlarmNo, AnsiString errPart="");

//  ShowMotorErrorMessage recorder.  The W7-L2 ckernel test plan asserts this
//  fired EXACTLY ONCE with a specific JamCode, so the count and the last Code
//  are both required; MotorAlarmNo is captured too because golden ckernel passes
//  a *derived* value (`iRef+1`, golden ckernel.cpp:2471/:2491) that is worth
//  pinning independently of the code string.
extern AnsiString W906_ShowMotorErrorMessage_LastCode;         // last `Code`; "" when never called
extern int        W906_ShowMotorErrorMessage_LastMotorAlarmNo; // last `MotorAlarmNo`; 0 when never called
extern AnsiString W906_ShowMotorErrorMessage_LastErrPart;      // last `errPart`; "" when never called
extern int        W906_ShowMotorErrorMessage_Count;            // call count
void W906_ShowMotorErrorMessage_Reset();                       // clear capture + count

// ---------------------------------------------------------------------------
//  6. AI(W906-W7-L2) 20260803 -- MyDBIProcessNew (golden cMyDB.h:21, //Steven
//     20161220 : Process加上Alarm Code).  golden ckernel.cpp:2452 calls it from
//     GetMotorAlarmCode, so the W7-L2 translation wave needs a real declaration.  (Body: golden, cMyDB.cpp, since P4.)
//
//  COLLISION ANALYSIS (done before landing this, because the tree already had a
//  TU-local stand-in).  atester_32Site.cpp:410 defines
//  `static void W5_32S_MyDBIProcessNew(...)` and :414 does
//  `#define MyDBIProcessNew W5_32S_MyDBIProcessNew`.  That is SAFE against this
//  declaration, and the reason is ORDERING, not luck:
//    - atester_32Site.cpp includes canary_support.h at its line 175;
//    - the #define lands at line 414, i.e. AFTER every #include in that TU
//      (its last #include is at :175 -- checked, there are no later ones).
//  So the preprocessor sees this prototype with its real spelling, and only the
//  CALL SITE at atester_32Site.cpp:1857 is rewritten to the static stand-in.
//  There is also no link-time clash: the stand-in has internal linkage, and that
//  TU never references the external symbol.  NOTHING in atester_32Site.cpp needs
//  to change -- which is just as well, since this front does not own it.
//  KEEP THIS DECLARATION ABOVE any future #include added to that file.
void MyDBIProcessNew(AnsiString asTable, AnsiString AlarmCode, AnsiString S1, AnsiString S2=" ");

//  AI(W906-CMYDB-P4) 20260927 (St02-E): the MyDBIProcessNew recorder (W906_MyDBIProcessNew_LastAlarmCode / _Count /
//  _Reset) is DELETED with the stand-in -- nothing read it (grep 20260927).  ckernel.cpp:2452's JCET "WAR240004"
//  motion row now lands in the real EventLogTxt / HANDLER LOG / EventTracker files, as golden.




// ---------------------------------------------------------------------------
//  7. AI(W906-W7-L2) 20260803 -- THE ALARM-QUEUE SEAM (PopUpAlarm/ClearAllAlarm)
//
//  ####################################################################
//  #  READ THIS BEFORE CHANGING THE SIGNATURES.  The brief for this    #
//  #  item stated that an exhaustive scan of the golden tree found     #
//  #  ZERO declarations and ZERO definitions of PopUpAlarm and         #
//  #  ClearAllAlarm, and that they therefore had to be DESIGNED from   #
//  #  scratch.  That premise is WRONG, and this pass disproved it.     #
//  ####################################################################
//
//  The scan was right about the SOURCE TREE and wrong about the PROJECT.  Both
//  functions live in a sibling BCB6 *component package* that is not part of the
//  translated source tree:
//
//      D:\HT9045\elec\Component\halarm.h    :36  void PACKAGE ClearAllAlarm();
//                                                     // 清除所有物件的錯誤碼
//                                           :37  bool PACKAGE PopUpAlarm(
//                                                     TComponent **Component,
//                                                     int &iErrCode);
//                                                     // 取出要顯示的錯誤訊息
//      D:\HT9045\elec\Component\HAlarm.cpp  :234 ClearAllAlarm() body
//                                           :264 PopUpAlarm() body
//
//  The trail was left in this very tree: Motor/HTMotor.h:9-11 records that the
//  W4 wave dropped `#include "halarm.h"` and identified it as "VCL TComponent
//  class".  Golden ckernel.cpp compiles against halarm.h through its own first
//  line -- golden ckernel.cpp:1 is `#include "MachineDefine.h"` and golden
//  MachineDefine.h:63 is `#include "halarm.h"`; golden Motor/HTMotor.h:5 carries
//  the same live include (both re-read this pass).
//  AI(W906-W7-L2) 20260803: an earlier draft of this paragraph ALSO listed
//  mycylin.cpp as an includer.  IT IS NOT ONE -- golden mycylin.cpp:6 is
//  `//#include "halarm.h"`, commented out; that TU only carries
//  `extern HAlarm  *Alarm;` (mycylin.cpp:83).  Corrected in place; the two live
//  includes above carry the conclusion on their own.
//  So these are TRANSLATED below from real bodies, not invented.
//
//  GOLDEN SEMANTICS (HAlarm.cpp:264-283, read in full):
//    PopUpAlarm pops the FRONT of a FIFO of `ERR_MSG { TComponent *ObjPtr;
//    int iErrCode; }` (HAlarm.cpp:16-19) held in `ShowAlarmList` (:22,
//    // 顯示錯誤串列).  On a non-empty list it writes *Component = P->ObjPtr
//    (:273, // 指向錯誤物件), iErrCode = P->iErrCode (:274, // 傳回錯誤碼),
//    Delete(0)s the entry, frees it, and returns true.  On an empty list it
//    falls through to `return false` (:282) WITHOUT touching either out-param.
//    ClearAllAlarm (:234-242) walks every registered HAlarm clearing its code
//    list, then sets SystemNG=false (:241).
//
//  POINTER TYPE -- READ THIS, IT DIVERGES FROM GOLDEN ON PURPOSE.
//  Golden spells the out-param `TComponent **` (halarm.h:37), and golden
//  ckernel.cpp:2503 declares `TComponent *Comp;` to match.  This tree types it
//  `HTMotor **` instead.  Why:
//    - The ONLY thing golden ever does with the popped pointer is an IDENTITY
//      comparison against a motor: `if(MOT[i].Motor==Comp && ...)` at golden
//      ckernel.cpp:2440, inside GetMotorAlarmCode(TComponent *Comp) (:2432).
//    - `MOT[i].Motor` is `HTMotor *` -- golden Motor/mymotor.h:56, and likewise
//      ported Motor/mymotor.h:139 (MOT[] itself is `TTrayMotor MOT[]`, ported
//      Motor/mymotor.h:385, whose base TMyMotor owns that member).
//    - Golden's `class HTMotor` (golden Motor/HTMotor.h:31) has NO base class
//      and no conversion operator -- checked explicitly this pass, and the same
//      is true of the ported one (Motor/HTMotor.h:79).  So golden's own
//      `HTMotor* == TComponent*` comparison at :2440 is between unrelated
//      pointer types and does not type-check under standard C++.  That is an
//      anomaly IN GOLDEN (flagged in this front's report); it is not something
//      the port can faithfully reproduce, because it does not compile.
//    - Typing the seam on `HTMotor **` keeps the ONE semantic that matters --
//      pointer identity of the alarming motor -- and lets the ckernel
//      translation write golden's comparison verbatim with no cast.  Typing it
//      on vclcompat::TComponent would force a cast at :2440 and buy nothing.
//  The POINTER LEVEL is unchanged from golden: pointer-to-pointer out-param.
//
//  HEADER COST: still zero new includes.  `class HTMotor;` is a forward
//  declaration -- pointer-to-incomplete is all the prototype needs, and it is
//  compatible with the real definition in Motor/HTMotor.h:79.  This matters:
//  123 TUs include this header.
//
//  ALM_MOTOR_MOVE is deliberately NOT defined here.  Motor/HTMotor.h:29 already
//  has `#define ALM_MOTOR_MOVE 55555`, identical to golden ckernel.cpp:2499, and
//  the ckernel translation will include that header anyway for MOT[].
//
//  OFFLINE DEFAULT AND WHICH BRANCH IT SELECTS.  With an empty queue PopUpAlarm
//  returns false, so golden ProcessAlarm's `while(PopUpAlarm(&Comp, iCode))`
//  (golden ckernel.cpp:2507) runs ZERO iterations and the function falls
//  straight through to ClearAllAlarm() at :2526.  This is not "false is safe" --
//  it is golden's own empty-list arm (HAlarm.cpp:266 guards on
//  `ShowAlarmList->Count>0`, :282 returns false otherwise), and it is the
//  CORRECT arm offline because the list is only ever populated by HAlarm::Set
//  (HAlarm.cpp:110-130) on an HAlarm object, and HAlarm objects only exist once
//  something constructs one (HAlarm.cpp:49 `HAlarmList->Add(this)`).  An offline
//  facade constructs none, so "nothing has alarmed" is the faithful state.  Use
//  W906_PopUpAlarm_Push() to leave that arm.
// ---------------------------------------------------------------------------
//  AI(W906-HALARM) 20260926: THIS SEAM IS RETIRED -- the real golden component
//  (D:\HT9045\elec\Component\HAlarm.cpp / halarm.h) is now translated whole in
//  halarm.h / HAlarm.cpp (ht9045_globals), RULINGS_20260926 #16.  What changed
//  against the notes above:
//    * POINTER TYPE: HTMotor* -> void*.  With the real component the only
//      HAlarm object is golden's global `Alarm` whose Parent is fMain -- not a
//      motor -- so typing the queue on HTMotor* would force a lie-cast.  void* is
//      the honest identity type; ckernel's `MOT[i].Motor==Comp` (golden :2440)
//      still compiles verbatim (HTMotor* == void* is standard C++).
//    * W906_PopUpAlarm_Push / W906_Alarm_* test counters: gone (zero users
//      tree-wide, measured with the Grep tool 20260926).  Tests construct a real
//      HAlarm and call Set() instead -- that path also DEDUPLICATES like golden
//      (HAlarm.cpp:112), which the push stand-in could not.
//  The two prototypes stay here (123 TUs include this header and ckernel.cpp
//  reaches them through it); they must stay token-identical to halarm.h:36-37.
// ---------------------------------------------------------------------------
bool PopUpAlarm(void **Component, int &iErrCode);   // golden halarm.h:37 (TComponent** -> void**, see above)
void ClearAllAlarm();                               // golden halarm.h:36

#endif // canary_supportH
