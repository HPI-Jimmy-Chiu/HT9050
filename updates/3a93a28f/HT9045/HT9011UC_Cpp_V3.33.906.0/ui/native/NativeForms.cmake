# =============================================================================
# AI(W906-NATIVE-PROTO) 20260928 [W906]: 原生表單原型的建置開關（DEMO，不是定案）。
#
#   cmake -DW906_NATIVE_FORMS=ON ..   -> wb_serve 內建原生 Win32 視窗（HW.IoSetView、Main.MotorView，都是唯讀）
#   預設 OFF                           -> 與今天完全相同：沒有任何 ui/native 的碼進 exe
#
# Steven 20260928：「如果是使用define的方式隔開看是使用 cpp form或是 html form有，可以嗎？」
#   —— 名稱照 D:\HT9045\.claude\skills\ht9045-cpp-generated-pages\references\native-forms-plan.md §7.7
#   的提案（W906_NATIVE_FORMS）；St02 之後沿用同一個名稱。執行期逐頁選（system\NativeForms.ini）今晚不做。
#
# 形狀照根 CMakeLists.txt 的 W906_NO_SOFT_SIMULTE（option + add_compile_definitions + message）。
# 為什麼整段放在 include 檔：CMake 一行只能有一個指令，而根 CMakeLists.txt 的行號是
# 全樹註解的錨點（例 CMakeLists.txt:3368、:3430），所以根檔只佔用原本的一個空行（:67）放 include()。
# 掛到 wb_serve 與加 ctest 要等 wb_serve 目標與 enable_testing() 都存在，用 cmake_language(DEFER)
# 延到根目錄處理完才做，根檔就不必再動第二行。
# =============================================================================
option(W906_NATIVE_FORMS
       "Build the native Win32 forms prototype into wb_serve (HW.IoSetView + Main.MotorView, read-only). Default OFF = no native code at all."
       OFF)
if(NOT W906_NATIVE_FORMS)
    return()
endif()

add_compile_definitions(W906_NATIVE_FORMS)
message(STATUS "W906 native forms: ON (prototype ui/native/, HW.IoSetView + Main.MotorView read-only)")

function(w906_native_forms_attach)
    if(NOT TARGET wb_serve)
        message(FATAL_ERROR "W906_NATIVE_FORMS=ON but there is no wb_serve target to attach to")
    endif()
    set(_nf_dir ${CMAKE_SOURCE_DIR}/ui/native)
    # 視窗本體與主機（純 Win32，不含任何機台碼）。NativeGrid.cpp（20260929）＝不閃、只畫變了的格子的表格＋雙緩衝標籤，取代 ListView。
    set(_nf_ui ${_nf_dir}/NativeHost.cpp ${_nf_dir}/NativeGrid.cpp ${_nf_dir}/NativeIoView.cpp ${_nf_dir}/NativeMotorView.cpp
               ${_nf_dir}/NativeMotorTest.cpp   # AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.MotorTest v1, display only
               ${_nf_dir}/NativeShuttleMove.cpp    # AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1, display only
               ${_nf_dir}/NativeHome.cpp         # AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.home v1, display only
               ${_nf_dir}/NativeTeach.cpp)      # AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.teach v1 (a table), display only
    # wb_serve：視窗本體 ＋ 膠水（HSys.IOTable／HSys.MotTable／MOT[]／1203 監看器樣本／EastSun 覆蓋掛鉤，唯讀）
    target_sources(wb_serve PRIVATE ${_nf_ui} ${_nf_dir}/NativeFormsWbServe.cpp
        ${_nf_dir}/NativeShuttleMoveGlue.cpp)   # AI(W906-NATIVE-ST02) 20260929: HW.ShuttleMove glue (Tech / Prod / TestIF / IniConfig, read only)
    target_link_libraries(wb_serve PRIVATE comctl32 gdi32 user32)

    # 無顯示的 ctest：建視窗（不顯示）→ 灌假資料 → 讀回 → 關掉；拖曳保活用模擬的內部迴圈驗。
    # 只連視窗本體與主機，不連 god-stack、不讀寫任何機台檔。
    add_executable(test_native_forms ${CMAKE_SOURCE_DIR}/tests/test_native_forms.cpp ${_nf_ui})
    target_include_directories(test_native_forms PRIVATE ${CMAKE_SOURCE_DIR})
    # winmm：展示模式用 timeBeginPeriod(1) 排出真的 20 ms（只連在測試程式；wb_serve 不連、不改計時器解析度）
    target_link_libraries(test_native_forms PRIVATE comctl32 gdi32 user32 winmm)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # 新寫的基礎建設開足嚴格度（agent 準則 3）—— 只對不含機台標頭的檔（視窗本體、主機、測試）。
        # NativeFormsWbServe.cpp 不列：MotorView 那一版 include 了 Motor/mymotor.h → HTMotor.h，那些標頭在 -Wextra 下
        # 本來就有既有警告（例 HTMotor.h:143 unused parameter，20260929 ON 建置實測），開了只會把別人的警告算到這裡。
        set_source_files_properties(${_nf_ui} ${CMAKE_SOURCE_DIR}/tests/test_native_forms.cpp
            PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
    endif()
    add_test(NAME NativeIoView_Headless    COMMAND test_native_forms --io)
    add_test(NAME NativeMotorView_Headless COMMAND test_native_forms --motor)
    add_test(NAME NativeHost_Keepalive     COMMAND test_native_forms --keepalive)
    # 20260929：不閃／效率（沒變 0 格、一顆燈 2 格、看不到的列 0 格、背景圖顏色、1000 次更新平均成本）
    add_test(NAME NativeGrid_Efficiency    COMMAND test_native_forms --perf)
    set_tests_properties(NativeIoView_Headless NativeMotorView_Headless NativeHost_Keepalive NativeGrid_Efficiency PROPERTIES TIMEOUT 60)
    # AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.MotorTest v1 -- its own test exe (window files only, no machine code):
    #   only Exit is enabled, null is —, selection is screen state, no-change = 0 cells, one value = one cell, 1000-update cost.
    add_executable(test_native_motortest ${CMAKE_SOURCE_DIR}/tests/test_native_motortest.cpp ${_nf_ui})
    target_include_directories(test_native_motortest PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(test_native_motortest PRIVATE comctl32 gdi32 user32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(${CMAKE_SOURCE_DIR}/tests/test_native_motortest.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
    endif()
    add_test(NAME NativeMotorTest_Headless COMMAND test_native_motortest)
    set_tests_properties(NativeMotorTest_Headless PROPERTIES TIMEOUT 60)
    # AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1 -- window files only: only Exit enabled, golden
    #   FormShow visibility (bar code / latch / SIM / SPIL), encoder null = —, no-change = 0 cells, one value = one cell.
    add_executable(test_native_shuttlemove ${CMAKE_SOURCE_DIR}/tests/test_native_shuttlemove.cpp ${_nf_ui})
    target_include_directories(test_native_shuttlemove PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(test_native_shuttlemove PRIVATE comctl32 gdi32 user32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(${CMAKE_SOURCE_DIR}/tests/test_native_shuttlemove.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
    endif()
    add_test(NAME NativeShuttleMove_Headless COMMAND test_native_shuttlemove)
    set_tests_properties(NativeShuttleMove_Headless PROPERTIES TIMEOUT 60)
    # AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.home v1 -- its own test exe (window files only, no machine code):
    #   only Exit is enabled, null is —, golden 15-row layout, no-change = 0 cells, one value = one cell, 1000-update cost.
    add_executable(test_native_home ${CMAKE_SOURCE_DIR}/tests/test_native_home.cpp ${_nf_ui})
    target_include_directories(test_native_home PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(test_native_home PRIVATE comctl32 gdi32 user32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(${CMAKE_SOURCE_DIR}/tests/test_native_home.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
    endif()
    add_test(NAME NativeHome_Headless COMMAND test_native_home)
    set_tests_properties(NativeHome_Headless PROPERTIES TIMEOUT 60)
    # AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.teach v1 -- its own test exe (window files only, no machine code):
    #   only Exit is enabled, null is —, the tab filter, selection is screen state, no-change = 0 cells,
    #   one axis = only its rows' cells, 1000-update cost with 265 + 52 rows.
    add_executable(test_native_teach ${CMAKE_SOURCE_DIR}/tests/test_native_teach.cpp ${_nf_ui})
    target_include_directories(test_native_teach PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(test_native_teach PRIVATE comctl32 gdi32 user32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties(${CMAKE_SOURCE_DIR}/tests/test_native_teach.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
    endif()
    add_test(NAME NativeTeach_Headless COMMAND test_native_teach)
    set_tests_properties(NativeTeach_Headless PROPERTIES TIMEOUT 60)
endfunction()
cmake_language(DEFER CALL w906_native_forms_attach)
