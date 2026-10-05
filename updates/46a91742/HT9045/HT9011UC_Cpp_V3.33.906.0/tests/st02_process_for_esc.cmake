# --- test_st02_process_for_esc: AI(W906-ST02-ESC) 20261005 (St02-E) -- census 129 E-T1-007: golden 906 0618 TfMain::ProcessForESC
#   (main.cpp:7615-7706) = THandlerTesterSide::ProcessForESC (TesterComm/Handler/HandlerGpibMsg.cpp + HandlerEscSuck.cpp), run by
#   golden Timer1Timer :2858 = MainTimersSt02.cpp Timer1BinTick through ht9045::W906_ESCProcessHook.  Seeded by the real
#   ResetForESC, driven through the real Timer1 slice; the reset, bBin16HangUp, the waits, the destroys' order / short-circuit,
#   ResetForESC's refusals, golden bRunTimer1, no TesterComm, the hook-up lines.  Two TUs on purpose: the nozzle half needs the
#   full TMyKitSuck (mykitsuck.h), the main one the mirror (aHotPlateSubstrate.h) -- docs/KNOWLEDGE.md two-TMyKitSuck.
#   Containment first (st02_test_containment.h; the EOF ENVIRONMENT block covers it).  Control: W906_ESC_CONTROL=1 (hook not
#   installed) must go red.  Included from the end of tests/CMakeLists.txt (no line there moved).
add_executable(test_st02_process_for_esc test_st02_process_for_esc.cpp test_st02_process_for_esc_nozzle.cpp)
target_link_libraries(test_st02_process_for_esc PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_process_for_esc PRIVATE ws2_32)
endif()
add_test(NAME St02_ProcessForESC COMMAND test_st02_process_for_esc "${CMAKE_SOURCE_DIR}")   # argv[1]: section 8 reads the hook-up lines (read only)
set_tests_properties(St02_ProcessForESC PROPERTIES TIMEOUT 60)
