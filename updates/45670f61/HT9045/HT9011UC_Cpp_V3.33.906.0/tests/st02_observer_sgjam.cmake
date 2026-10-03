# =============================================================================
#  tests/st02_observer_sgjam.cmake -- AI(W906-ST02-OB7) 20261002 (St02-E helper)
#  Included from tests/CMakeLists.txt:6175 (an empty St02 line between St02_MainTimers and St02_TimerESD, so no line of
#  tests/CMakeLists.txt moved; CMake takes one command per line, hence the whole block lives here -- the
#  CMakeLists.txt:67 NativeForms.cmake precedent).  Same directory scope: ENV-ALL (the deferred _w906_env_all_tests at the
#  end of tests/CMakeLists.txt) gives both tests the redirect roots; neither sets its own ENVIRONMENT.
#
#  E-019 OB-7: Data.Observer > System Message > SG_JamCount = golden 906_0625 (D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven)
#  cObserver.cpp:5361-5369 btnSG_QueryNowClick / btnSG_QueryYesterdayClick -> StatisticalJamCount :5060-5276.
# =============================================================================

# --- test_st02_observer_sgjam (ctest St02_ObserverSGJam): JsonBridge/actions/ObserverSGJam.cpp through the REAL act.*
#   dispatch JsonBridge/ChanAction.cpp HandleActionWithTag (its :347 same-line dispatch), on the static fObserver
#   (cObserver.cpp:3289, built at static init; the grid header of golden's constructor :305-317 is checked first).
#   Seeds the event-log CSV where StatisticalJamCount really reads it (<slEventLog->Path>\YYYY\MM\EventLogTxt_YYYYMMDD.csv,
#   slEventLog built by the test) and JamCountEnable.ini where StatisticalJamCountEnable reads it.  Cases: refusals
#   (not-open, unknown op, act.nope / act.observer.* not taken, bad payload, slEventLog NULL, dryRun), Query Now
#   (JAM0301 x2 / JAM0302 counted, WAR / JAM00xx / JAM20xx not, Rate 2.00 / 1.00, golden's trailing blank row, RawData.csv),
#   state (read only), Query Yesterday (counts yesterday, iOneDayLoaderCount -> 0 golden :5262-5263), the [N26] FTP tail
#   reported as skipped (gated Q5a, S25; no socket anywhere), yesterday's CSV missing (golden :5106-5111 early return,
#   counter and grid untouched), InitialOK false (nothing written), JamCountEnable.ini 03=0 (JAM03xx not counted), source
#   ratchets (argv[1] port tree, argv[2] web\page), and D:\HT9045_Log\EventLogTxt\SGJamCount unchanged before / after.
#   Containment: exit 2 unless the ctest redirect roots are in machine_log_scratch (w906_ctest_guard.h); every file it
#   writes is under <W906_EVENTLOG_ROOT>\st02_ob7 (removed on a green run).  Memory and scratch files only, no network.
add_executable(test_st02_observer_sgjam
    test_st02_observer_sgjam.cpp
    ../JsonBridge/ChanProduction.cpp
    ../JsonBridge/ChanIo.cpp
    ../JsonBridge/ChanMotor.cpp
    ../JsonBridge/ChanAction.cpp
    ../JsonBridge/actions/MainTesterConnect.cpp
    ../JsonBridge/actions/MainRecordClear.cpp
    ../JsonBridge/actions/MainClarnData.cpp ../JsonBridge/actions/MainStateRecord.cpp
    ../JsonBridge/actions/LotInfoFtp.cpp   # AI(W906-ST02-OB7) 20261003 (St02-E): main's ChanAction.cpp (LI-9 !116) dispatches act.lotInfoFtp.* -> W906_LotInfoFtpAct, as tests/CMakeLists.txt:3969 / :6644
    ../JsonBridge/actions/ObserverSGJam.cpp
    ../JsonBridge/EventLog.cpp
    ../EtherCAT/Pci1203Monitor.cpp)
target_include_directories(test_st02_observer_sgjam PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_st02_observer_sgjam PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
if(WIN32)
    target_link_libraries(test_st02_observer_sgjam PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_st02_observer_sgjam.cpp ../JsonBridge/actions/ObserverSGJam.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME St02_ObserverSGJam COMMAND test_st02_observer_sgjam "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(St02_ObserverSGJam PROPERTIES TIMEOUT 120)

# --- St02_ObserverSGJamPage (node, offline): D:\HT9045\web\page\ht9045_observer_sgjam.js in a node vm with a fake DOM /
#   HT9045Recipe (+ the real ht9045_busy_util.js): compiles, one EOL style, no BOM; at load the grid shows golden's
#   constructor shape (header, 6 columns, 4 empty rows); Query Now / Query Yesterday send control.acquire ->
#   act.observerSG.<op> -> control.release; the reply's grid and Loader Count are drawn (an empty Caption keeps the dfm
#   text); eventlog-missing / initial-not-ok / a refusal / busy: / an old wb_serve without act.observerSG.* on the status
#   line; a second click inside the cool-down is not sent; the SG_JamCount tab sends a quiet state after STATE_DELAY_MS; a
#   click while that state is in flight is sent after it; Data.Observer.html loads the file after ht9045_observer_ev.js.
#   Control: W906_ST02_OB7_PAGE_DIR -> a page directory without the file must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME St02_ObserverSGJamPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/st02_observer_sgjam_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(St02_ObserverSGJamPage PROPERTIES TIMEOUT 60)
endif()
