# --- test_st02_l42_staterecord913: AI(W906-L42) 20261010 (St02) -- card L42 (ST02_V912_VS_V913 s3 L42): golden 913 State Record
#   diagnostics (RogerYang 20260914 / 20260915 / 20260922, HHT-17), bodies in StateRecord_L42_St02.cpp (ht9045_sm), called on
#   single lines of cStateRecord.cpp: W906_L42_SaveMachineMaterial (golden 913 main.cpp:27706-27727) writes MachineMaterial.txt
#   -- the five golden lines, idle and seeded; SaveTaskList's new
#   rows (golden 913 :6835-6863 Pattern #31, :7079-7116 FLCarryKit / BLCarryKit / TestSocket, :7140-7166 ATK on a seeded ATK
#   machine) and SaveDecisionVariables' (:7310-7427 section 5b ASM, :7474-7484 hang-up watchdog) with seeded values, in golden
#   order; source pins for every call (cStateRecord.cpp, which defines no W906_L42_* body), the citations, the CMake line and
#   the three GATE(W906-L42-n).  argv[1] = the port tree (read only).
#   Writes only a fresh l42_<tick> folder in ctest's log-root sandbox (removed when green); refuses roots under
#   D:\HT9045 / D:\HT9045_Log that are not a build dir (\obj\v906\).  No ENVIRONMENT here on purpose: AI(W906-ENV-ALL)
#   copies the global redirect string.  Link line as test_task_list_register.  Included from St02's State Record block of
#   tests/CMakeLists.txt (a former blank line; no line there moved).  The exe name has no setup / install / update / patch.
add_executable(test_st02_l42_staterecord913 test_st02_l42_staterecord913.cpp)
target_link_libraries(test_st02_l42_staterecord913 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_l42_staterecord913 PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_st02_l42_staterecord913.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME St02_L42StateRecord913 COMMAND test_st02_l42_staterecord913 "${CMAKE_SOURCE_DIR}")   # argv[1]: [S] reads cStateRecord.cpp (read only)
set_tests_properties(St02_L42StateRecord913 PROPERTIES TIMEOUT 120)
