// ===========================================================================
//  JsonBridge/actions/ObserverSGJam.h -- Data.Observer > System Message > SG_JamCount (E-019 OB-7)
//
//  AI(W906-ST02-OB7) 20261002 (St02-E helper).  Card ST02-C7 / plan
//    D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\C7_FTPSAVE_SGJAM_PLAN_20261002.md section 2.4.
//  golden 0618 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618 (cp950; 906_0625 has the same lines); 912 lines in brackets
//    (D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy):
//    act.observerSG.queryNow        <- cObserver.cpp:5361-5364 btnSG_QueryNowClick       -> StatisticalJamCount(false)  [912 :5592]
//    act.observerSG.queryYesterday  <- cObserver.cpp:5366-5369 btnSG_QueryYesterdayClick -> StatisticalJamCount(true)   [912 :5597]
//    (dfm cObserver.dfm:1879-1896, OnClick :1886 / :1895; StatisticalJamCount :5060-5276 [912 :5291])
//    act.observerSG.state           <- no golden handler: the VCL grid strngrdJamLog and labLoaderCount keep their
//                                      contents on the form; the page reads them back when the SG_JamCount sheet is
//                                      shown (read only, nothing written, no log line).
//  The bodies are jimmychiu's translation (cObserver.cpp:3320-3613, FW-Q5 58c66991); this file only calls the two
//    TfObserver members and reports what golden did.  Nothing in cObserver.cpp / forms/fObserver.h is changed.
//
//  Route: WS act.observerSG.<op> -> tools/wb_serve.cpp act.* catch-all (IsActionCommand) -> JsonBridge/ChanAction.cpp:347
//    (same-line dispatch, before that line's //) -> W906_ObserverSGJamAct_St02 below.  The prefix is 15 characters; its
//    13th is 'S', so E-021's `act.observer.` test on :344 does not take it.  act.* needs the operator token
//    (WebBridgeServer) and passes the WebCmdGuard double-click guard (same cmd + value inside 400 ms -> busy:).  It runs on
//    the tick thread (golden's main thread) without FormLock, like E-021's act.observer.*.
//
//  Port-only refusals (golden never gets there; none of them changes what golden does once it runs):
//    no-form                 fObserver / strngrdJamLog / labLoaderCount NULL
//    not-open                the Observer page is not open (W906_FormShowing("fObserver", bShow), as E-021)
//    bad-payload             value is not a JSON object
//    log-objects-not-created slEventLog is NULL (golden builds it in the TfMain constructor, main.cpp:1501-1512; in the
//                            port wb_serve builds it at boot, tests/ctest builds its own) -- golden's first statement
//                            after the InitialOK test dereferences it (:5075)
//  {"dryRun":true} runs the guards only (RULINGS_20260926 #12: absent or false = execute, as the other act.* here).
//
//  Reply (executed:true when golden's handler ran -- also when golden itself returned early, as E-021's golden refusals):
//    op, goldenLine, dryRun, initialOk, eventLogCsv {path, exists} (golden :5102-5105, computed with the same two clock
//    calls first), early ("" | "initial-not-ok" golden :5062-5065 | "eventlog-missing" golden :5106-5111: it only writes
//    the process log "JamRawData is error. EventLog is not exist. <file>" and returns, the grid keeps its old cells),
//    loaderCountBefore, sgJamCountCsv (MyStringList.cpp MySaveSGJamCountToFile path, when the body ran), grid {rows, cols,
//    fixedRows, fixedCols, truncated, cells[row][col]} (every row, incl. golden's trailing blank one, :5160), labLoaderCount,
//    iOneDayLoaderCount, skipped (the [N26] FTP upload tail, gated Q5a and S25).
//  Golden shows no message box on this sheet; the page writes one status line.
//
//  Files golden writes when the body runs (W906_EVENTLOG_ROOT redirects all of them; unset = golden's literal):
//    D:\HT9045_Log\EventLogTxt\SGJamCount\JamCountEnable.ini            (missing keys 01..19 seeded = 1)
//    D:\HT9045_Log\EventLogTxt\SGJamCount\YYYY\MM\<HandlerID>_YYYYMMDD_RawData.csv   (deleted, then rewritten)
//    D:\HT9045_Log\EventLogTxt\SGJamCount\YYYY\MM\  and  <slEventLog->Path>\YYYY\MM\   (MyForceDirectories)
//    the process log (RecordProcess) when the event-log CSV is missing.
//    It reads <slEventLog->Path>\YYYY\MM\<slEventLog->FileName>_YYYYMMDD.csv (golden :5102-5117).
//    queryYesterday sets iOneDayLoaderCount = 0 (golden :5262-5263, memory only).
//
//  ctest: St02_ObserverSGJam (tests/test_st02_observer_sgjam.cpp) and St02_ObserverSGJamPage (node, offline).
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_ACTIONS_OBSERVERSGJAM_H
#define HT9045_JSONBRIDGE_ACTIONS_OBSERVERSGJAM_H

#include <string>

namespace ht9045 {
namespace sjson {

// JsonBridge/ChanAction.cpp:347 declares this at block scope (inside ht9045::sjson::HandleActionWithTag, so the same
// namespace) and calls it for every act.observerSG.*.  Keep the two signatures identical.
std::string W906_ObserverSGJamAct_St02(const std::string& cmd, const std::string& payloadJson);

}  // namespace sjson
}  // namespace ht9045

#endif  // HT9045_JSONBRIDGE_ACTIONS_OBSERVERSGJAM_H
