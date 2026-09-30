// =============================================================================
//  WebMainCtlButtons.cpp -- runs the golden OnClick bodies of the main screen's control buttons the browser clicked.
//
//  AI(W906-FLOW-4) 20260930: INBOX 109 "FLOW-4".  NOT in golden as a file.  wb_serve only (CMakeLists.txt,
//    add_executable(wb_serve ...)): it takes St01's events from FileRW/MainClick.cpp, which is compiled into wb_serve
//    only (that file's head: do not move it into any archive).
//
//  WHAT HAPPENS ON A CLICK
//    browser -> WS act.main.ctlButton {"op":"click","button":"oneCycle"|"trayFeed"|"alarmReset"|"reset"}
//    drain   -> FileRW/MainClick.cpp W906_Main_CtlButtonOp (St01, S169): a click on a disabled button is refused there
//               (its CtlBtnEnabled = golden TfMain::ProcessKeyFlush, 906 main.cpp:4098), otherwise ONE event per click is
//               queued (kept 3 s) and the ack ("eventOnly") goes back at once.
//    next pass of wb_serve's main loop (the loop's wait is capped at 50 ms) -> W906_MainCtlButtonTick() below takes every
//               pending event and calls the golden OnClick body once per click, under FormLock (the lock St01's
//               act.main.cleanOut holds around fMain->CleanOut).  The bodies keep their own golden guards
//               (fAllMotorHome / iOneCycle / bSECSGEMAlarm / bEnableEmployeeIDCheck / SystemStart ...); nothing here
//               re-decides them and nothing here trusts the browser beyond "this button was clicked".
//
//  WHY EVERY PASS AND NOT ONLY ON THE 500 ms BEAT
//    golden runs an OnClick handler as soon as the VCL loop dispatches the click; nothing in golden defers it to a timer.
//    The queue only exists because St01's side registers and Jimmy's side runs (S169).  Taking the event on the next pass
//    keeps the delay to one pass (<= ~50 ms) instead of up to one beat, and the call sits at the top of the pass, before
//    PumpTick, so the MainProc of that beat already sees what the body set.  (St01's note says "in the tick (500 ms)"; the
//    3 s TTL is sized for that and is only more comfortable at 50 ms.)
//
//  WIRED / NOT WIRED   (golden 906 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618, cp950)
//    oneCycle   -> fMain->BtnOneCycleClick    main.cpp:4332-4380                               body cCleanOut.cpp  WIRED
//    trayFeed   -> fMain->BtnTrayEndClick     main.cpp:13944-13947 -> InitialTrayFeedTask :2317-2377  cCleanOut.cpp  WIRED
//    alarmReset -> fMain->BtnAlarmResetClick  main.cpp:22159-22166                             body cCleanOut.cpp  WIRED
//    None of the three waits (see the block above them in cCleanOut.cpp): flags, log rows, a SECS event counter.
//
//    reset      -> NOT WIRED -- a decision for the user.  golden TfMain::Reset (main.cpp:7235-7601, BtnResetClick :7230)
//                  can stop on a modal dialog:
//                    :7394  ShowMyMessage("Please remove the devices on in arm picker!", "", "", false, true)
//                           when !bResetPutUntestToErrorBin && bO01_ResetNeedClearAndCheckHP && bCanUseHotPlateCheck &&
//                           !bResetClearArmIC (golden ShowMyMessage always ends in ShowModal, mymessbox.cpp:860)
//                    :7486  ShowErrorMessage("MES1652", K_ONECYCLE|K_CLEAN_OUT, ...) when the final else runs with
//                           CosFunction.bResetModeIncludeCleanOut && fNote->fShow==false && bResetNotMsg==false
//                  In this port both wait ON THE TICK THREAD for the browser's answer (tools/wb_serve.cpp
//                  ForwardShowErrorMessage / W906MbShowMyMessage), so MainProc stops until the operator answers.  In golden
//                  MainProc runs on a thread through Synchronize (uruncontrol.cpp:34-50), which the dialog's own message
//                  loop keeps servicing -- golden keeps polling while that dialog is open.  Besides, ResetServoOff
//                  (:7146-7226) sleeps MySleep(200) up to twice and switches the in/out arm X/Y servos off.
//                  TfMain::Reset / BtnResetClick stay the offline no-ops (forms/fMain.cpp:403 / :405) and the event is left
//                  in St01's queue, where it expires after 3 s -- exactly the behaviour before this file.
//    siteClick  -> NOT WIRED -- a decision for the user.  golden TfMain::mtDutOnOffMouseUp (main.cpp:28888-29572) can stop
//                  on a modal: :28917 ShowMyMessage (CC_KYEC_JCTHIU + rsmFIFOMode) and :29154-29158 / :29208-29212
//                  `do { ret=ShowMyMessagePWD(...); } while(ret==1);` (IniConfig.bShowCloseSiteAlarmWhenStart, closing a
//                  site); and the web grid's (x,y) is the 8x4 tag contract, not golden's mtDutOnOff X/Y (St01's note at
//                  W906_Main_TakeSiteClickEvent).  W906_Main_TakeSiteClickEvent is not called; its events expire as before.
//
//  ORDER INSIDE ONE PASS: St01 keeps one queue per button, so two different buttons clicked within the same pass lose their
//    relative order; this runs oneCycle, then trayFeed, then alarmReset (St01's kCtlBtns order).  Clicks of one button
//    keep theirs (one body run per click, oldest first).
// =============================================================================
#include "forms/fMain.h"            // fMain; TfMain::BtnOneCycleClick / BtnTrayEndClick / BtnAlarmResetClick (bodies cCleanOut.cpp)
#include "cmydef.h"                 // the globals printed below: fAllMotorHome / iOneCycle / iCleanOut / iTrayFeed / iTrayFeedTask /
                                    //   bManualOneCycle / SystemStart
#include "JsonBridge/FormJson.h"    // ht9045::formjson::FormLock / FormUnlock
#include "FileRW/MainClickTail.h"   // W906_Main_TakeCtlButtonEvent (St01, FileRW/MainClick.cpp)

#include <cstdio>

namespace {
struct W906CtlFormLock {
    W906CtlFormLock()  { ht9045::formjson::FormLock(); }
    ~W906CtlFormLock() { ht9045::formjson::FormUnlock(); }
};

int W906CtlRunOneCycle()
{
    int n = 0;
    while (W906_Main_TakeCtlButtonEvent("oneCycle")) {
        W906CtlFormLock lock;
        const int down0 = fMain->BtnOneCycle ? (int)fMain->BtnOneCycle->Down : -1;
        const int manual0 = (int)bManualOneCycle;
        fMain->BtnOneCycleClick(fMain);                                         // golden OnClick (main.dfm:10694), Sender unused
        ++n;
        std::printf("act.main.ctlButton oneCycle -> golden BtnOneCycleClick ran (906 main.cpp:4332): BtnOneCycle->Down %d->%d "
                    "bManualOneCycle %d->%d  fAllMotorHome=%d iOneCycle=%d SystemStart=%d\n",
                    down0, fMain->BtnOneCycle ? (int)fMain->BtnOneCycle->Down : -1, manual0, (int)bManualOneCycle,
                    (int)fAllMotorHome, iOneCycle, (int)SystemStart);
    }
    return n;
}

int W906CtlRunTrayFeed()
{
    int n = 0;
    while (W906_Main_TakeCtlButtonEvent("trayFeed")) {
        W906CtlFormLock lock;
        const int tf0 = iTrayFeed, tft0 = iTrayFeedTask;
        fMain->BtnTrayEndClick(fMain);                                          // golden OnClick (main.dfm:10759)
        ++n;
        std::printf("act.main.ctlButton trayFeed -> golden BtnTrayEndClick ran (906 main.cpp:13944 -> InitialTrayFeedTask :2317): "
                    "iTrayFeed %d->%d iTrayFeedTask %d->%d  iCleanOut=%d iOneCycle=%d fAllMotorHome=%d SystemStart=%d\n",
                    tf0, iTrayFeed, tft0, iTrayFeedTask, iCleanOut, iOneCycle, (int)fAllMotorHome, (int)SystemStart);
    }
    return n;
}

int W906CtlRunAlarmReset()
{
    int n = 0;
    while (W906_Main_TakeCtlButtonEvent("alarmReset")) {
        W906CtlFormLock lock;
        fMain->BtnAlarmResetClick(fMain);                                       // golden OnClick (main.dfm:10737)
        ++n;
        std::printf("act.main.ctlButton alarmReset -> golden BtnAlarmResetClick ran (906 main.cpp:22159): MES2116 row, "
                    "SECS DoAlarmReset if bEnable_SECS_GEM, RespondASECom\n");
    }
    return n;
}
}  // namespace

// Called once per pass of wb_serve's main loop (tools/wb_serve.cpp, the line after the loop's wait) and from the native
// window drag keepalive's copy of that line.  Returns how many golden bodies ran (the ctest reads it).
int W906_MainCtlButtonTick()
{
    if (fMain == 0) return 0;
    int n = 0;
    n += W906CtlRunOneCycle();
    n += W906CtlRunTrayFeed();
    n += W906CtlRunAlarmReset();
    // "reset" and the site cell: NOT taken on purpose (file head) -- they expire in St01's queue as before.
    if (n) std::fflush(stdout);
    return n;
}
