# 原文 11／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/forms/fNote_ShowError.cpp`；定位 `capture_banner_snapshot`；種類 `regions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `cdb907f898de72ba6a8c1d77c829825b6f0cc8346165bfd1badd253c31403d36`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
//  AI(W906-J5-ACK) 20260930: the operator's answer to a KeyCode==0 note -- INBOX 119 (Jerry J-5, FROM_JERRY 0929 18:24).
//  Golden (906 note.cpp): a KeyCode==0 note waits in ShowModal (ShowErrorMessage :991, ShowMotorErrorMessage :1133)
//  until the operator closes it, and it has ONE exit, PAUSE (AI(W906-I37A) 20261002: INBOX 122 / NB2 R122 -- not two, see START):
//    PAUSE  TfNote::BtnPauseClick (:3826), its KeyCode==0 arm (:4044-4153): SoftStop=true, ReturnCode=0, SoftStart=false,
//           EventReport(DoPause), SendCommand_ESD(ESD_SYSTEM_STOP), Close()
//    START  TfNote::Start (:3527), its KeyCode==0 arm (:3683-3803) is unreachable: FormShow hides BtnStart (:1542-1545),
//           the panel START key returns while it is hidden (:3024-3029); the port matches (dialog-page.js, wb_serve :7369)
//  and PAUSE ends in TfNote::FormClose (:2503-2762).  The port's notice does not wait (user ruling 20260923,
//  AI(W906-Q30-KZERO)).  This is the PAUSE exit, reached from the page's single 確認 button through WS dialog.notifyAck
//  (tools/wb_serve.cpp W906_NoticeAckCommand -> w906dlg::NotifyAckHandle, tools/wb_dialog_mailbox.h).  A web / remote
//  START (start.run, SECS RCMD) can restart the machine while the box is up; the ack then pauses it as golden does
//  (RULINGS_20261002 #5 = NIGHT_REPORT s0 #37 A; it used to be "pause 2", records only).  Nothing here starts motion.
//
//  W906_NoteNoticeCapture -- when a notice is posted (wb_serve ForwardShowErrorMessage, its kcode==0 branch): keeps what
//    FormClose reads from fNote.  Golden reads it at close, and nothing can change it while the note is modal (a new
//    alarm returns at :814-818); the port's notice does not hold the tick, so it is kept here.  Plus golden FormShow's
//    two lines this close depends on: :1526 tNoteTimer start (PassTime), :2130 Recovery="".
//  W906_NoteNoticeAckRefusal -- golden BtnPauseClick's early returns before it closes (the box stays up).
//  W906_NoteNoticeAckLikeGolden -- after the mailbox is idle: BtnPauseClick's KeyCode==0 arm, then FormClose, once.
//
//  WHAT RUNS (the pause code goes back in the ack, w906dlg::NotifyAckOkJson):
//    3 no-golden-note  the record half did not run (InitialOK false :538-542 / an alarm while fNote is up :814-818):
//                      golden shows no note at all, so nothing closes -- only the mailbox is retired.
//    2 skipped-machine-running  RETIRED by AI(W906-I37A) 20261002 (RULINGS_20261002 #5 = A).  It was: SystemStart or SoftStart
//                      true (restarted since the notice) -> records only, no PAUSE.  golden's KeyCode==0 arm has no such
//                      condition (:4146-4152): the machine is paused now and Alarm->Clear() drops what it raised meanwhile, as
//                      golden.  bPress below is always true; the code value 2 stays reserved in w906dlg::NotifyAckOkJson.
//
//    1 already-applied the motor note: ShowMotorErrorMessage's body applied :4146 / :4148 right after posting (this file,
//                      the AI(W906-ALARMSTOP) rule), so SoftStop / SoftStart are not set a second time; the rest as 0.
//    0 applied         the machine is still in the stop the notice put it in: everything below.
//  KNOWN GAPS (not dependency gates -- stated so the ack is not read as "golden's close is complete"):
//    * ShowMotorErrorMessage's EventLogTxt row (this file, after the hook) is written when the note is POSTED with
//      StopedTime (PassTime) 0; golden writes it after ShowModal returns (:1135-1163) with the seconds the note was up.
//      Moving it here would lose the row whenever a notice is never acknowledged (replaced by a newer alarm, wb_serve
//      restarted) -- a user decision, INBOX 119.
//    * ShowErrorMessage's post-ShowModal tail (golden :993-1047: SaveErrEventLog (N-7), the Unloader-full message, ASE,
//      the K_SKIP pause, HSForm) is not translated for any alarm, blocking or notice (this file's banner).
//    * HGem->ReportAlarm at close (:2551-2555) is INBOX 64, as for the blocking close (forms/fNote_JamCount.cpp banner).
//    * a notice replaced by a newer request before anyone acknowledged it never gets this close (golden cannot replace
//      an open note; the port's single-slot mailbox can).
//  ctest: tests/test_notice_ack.cpp (NoticeAck).
// =============================================================================
#include "Interface/InterfaceSYS.h"  // SendCommand_ESD / ESD_SYSTEM_STOP (golden :4151)
#include "SECSGEM/SecsEventType.h"   // SECS_EVENT.DoPause (golden :4150)
#include "SECSGEM/SecsEventReport.h" // EventReport (golden :4150)
#include "myTimer.h"                 // TQPF_Timer (golden note.h:438 tNoteTimer)
bool W906_CheckRecordJamType(AnsiString asJamCode, bool bCheckOnly);          // forms/fNote_JamCount.cpp (golden note.cpp:344 CheckRecordJamType)
void W906_NoteFormCloseAlarmClear();                                           // HAlarm.cpp:345 (golden FormClose :2531 Alarm->Clear())
extern TQPF_Timer hAutoCleanHangUp;                                            // csystem.h:291 (golden csystem.cpp:157)
extern TQPF_Timer tGalilTwoYMoveDelay;                                         // Motor/myGALILmotor.cpp:5432 (golden mymessbox.cpp:384 extern)
void W906_NoteNoticeCapture(const char* requestId, bool motorNote);
const char* W906_NoteNoticeAckRefusal(const char* requestId);
bool W906_NoteNoticeAckLikeGolden(const char* requestId, int* pause, bool* jamCounted, unsigned long* passTimeSec);
namespace {
struct W906NoticeNote                                                           // [W906] fNote as golden FormClose reads it, kept when the notice is posted
{
    bool       valid = false;
    AnsiString requestId;                                                       // the mailbox requestId it belongs to
    AnsiString code;                                                            // fNote->edErrorCode->Text (:2528-2529)
    int        alarmType = 0;                                                   // fNote->AlarmType (:2528)
    int        duplicateError = 0;                                              // iDuplicateError (:2528)
    int        eventId = 0;                                                     // iEventID (:2557)
    bool       goldenNote = false;                                              // golden reaches ShowModal for it (the record half ran)
    bool       answerApplied = false;                                           // :4146 / :4148 already applied (the motor note)
};
W906NoticeNote W906_Notice;
TQPF_Timer     W906_tNoteTimer;                                                 // [W906] golden note.h:438 TfNote::tNoteTimer (TfNote has none here)
}
//------------------------------------------------------------------------------

<!-- preserved-content:end -->
```
