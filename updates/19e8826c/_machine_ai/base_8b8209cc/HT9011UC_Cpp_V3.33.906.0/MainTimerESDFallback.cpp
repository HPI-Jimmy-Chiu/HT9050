// ===========================================================================
//  MainTimerESDFallback.cpp -- AI(W906-S13) 20261001 (St02-E): the fallback of W906_MainClose_TimerRecordLoaderDate
//  (golden TfMain::TimerRecordLoaderDate; the real body is FileRW/MainClose.cpp:497 in that file's anonymous namespace,
//  reached through the global entry at MainClose.cpp:2879, St01; compiled into wb_serve only)
//  for every other program that links MainTimerESD.cpp (S-13 TimerESD E5 calls it; ht9045_sm).
//
//  ONE symbol in its own archive member, on purpose:
//    * wb_serve: its own MainClose.o defines the global entry before any archive is searched => this member is never
//      extracted there, wb_serve unchanged.
//    * the other programs: only this member is extracted.  The first try put it at the end of FileRW/_fallback.cpp
//      (the H1 line St01 agreed, 56b92e9b); extracting that member for this symbol also pulled its five FileRW
//      stand-ins into test_ga2_c1_cinitial / test_testcategory_paint, which link the real FileRW objects -> multiple
//      definition (exactly the warning in _fallback.cpp's header).  So St01's file is not touched.
//  Returns 0: the rowID wb_serve returns too (MyDBExecSQL, CSV only; MainClose.cpp:490-495).
// ===========================================================================
int W906_MainClose_TimerRecordLoaderDate() { return 0; }
