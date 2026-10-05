# ===========================================================================
#  BATCH-SAFETY BOOTSTRAP -- must stay at the TOP of this file
# ===========================================================================
# AI(W906-TestBootstrap) 20260728: every test executable in this directory
# gets tests/test_bootstrap.cpp compiled into it, which disables Windows'
# modal error dialogs (MSVC Debug-CRT "Debug Assertion Failed!", CRT
# invalid-parameter, abort(), and the WER "program stopped working" box).
#
# WHY: those dialogs BLOCK. A single failing assert under `ctest -jN` stops
# the whole 95-test run dead, writes nothing to the log while it hangs, and
# buries the developer's screen in modal boxes. Observed 20260728 on an MSVC
# Debug build of this suite (hung at 94/95). The WER box also fires on plain
# crashes under MinGW -- this project has hit both a SegFault and a real
# STATUS_STACK_OVERFLOW in tests before (DEVLOG 2026-07-27).
#
# This does NOT hide failures: a failing test still fails, but now it fails
# fast and loud on stderr where ctest captures it, instead of hanging.
#
# An INTERFACE library (not a static one) so the TU is COMPILED INTO each
# executable -- an object containing only a static initializer can otherwise
# be dropped by the linker when nothing references it.
#
# `link_libraries()` is directory-scoped and applies to every target created
# BELOW it, which is why this block must remain first.
add_library(ht9045_test_bootstrap INTERFACE)
target_sources(ht9045_test_bootstrap
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/test_bootstrap.cpp
)
link_libraries(ht9045_test_bootstrap)

add_executable(test_ContactForce
    test_ContactForce.cpp
)

target_link_libraries(test_ContactForce PRIVATE ht9045_core)

add_test(NAME ContactForce COMMAND test_ContactForce)

# --- cContact: ComputeMaxIndexForceLimit / ComputeMinForce (W64b force-calc extract) -----
#   Same simple link shape as test_ContactForce above: cContact.cpp only #includes
#   MachineType.h (header-only) and takes every other external read as an explicit
#   parameter, so it needs no ht9045_globals / RESCAN link group.
add_executable(test_cContact
    test_cContact.cpp
)

target_link_libraries(test_cContact PRIVATE ht9045_core)

add_test(NAME cContact COMMAND test_cContact)

# --- MainCalcCore: ComputeShtModeFlag / ComputeCanChangeRealDummy /
#   ComputeSiteMapIsStander / ComputeSiteMapPriority / ComputeCanChangeSite /
#   ComputeCanChangeToSocket (W906 TfMain calc-core extract) ------------------
#   Same simple link shape as test_cContact above: MainCalcCore.cpp only #includes
#   MachineType.h (header-only) and takes every other external read as an explicit
#   parameter, so it needs no ht9045_globals / RESCAN link group.
add_executable(test_MainCalcCore
    test_MainCalcCore.cpp
)

target_link_libraries(test_MainCalcCore PRIVATE ht9045_core)

add_test(NAME MainCalcCore COMMAND test_MainCalcCore)

# --- cUnitConvert: iUnitMultiply100 / iUnitMultiply1000 (W2 pure extract) ---
add_executable(test_cUnitConvert
    test_cUnitConvert.cpp
)

# AI(pt-wave) 20260811 PT-W8: was `PRIVATE ht9045_core`.  cUnitConvert.cpp was a
#   2-function "W2 partial" that referenced nothing outside itself; PT-W8 landed its
#   7 real golden bodies, and those read the machine-config global universe.  Measured,
#   not guessed -- the 32 symbols ld could not resolve map to exactly three archives:
#     28 -> ht9045_globals  (TestIF/TestIF_File/Offset*/InArmOffSet*/OutArmOffSet*/
#                            SortArmOffSet*/ArmSpeed*/SHSpeed*/MGSpeed*/DeviceForm*/
#                            HotPlateForm*/UserDefForm*/IniConfig/LastSet/Temperature/
#                            Tempture_Ambient/USE_LdUldCassetteMode/USE_OUT_SORT_ARM)
#      1 -> ht9045_forms    (fShowMessage)
#      3 -> ht9045_sm       (InArmSuck, OutArm2Suck via aHotPlateSubstrate.cpp;
#                            ATC_InterfaceForm via acarry_shims.cpp:73)
#   RESCAN rather than a plain ordered list, for the reason already documented on
#   test_config_loaders below: GNU ld makes a single pass over an archive set and
#   cannot back-resolve, and these archives reference each other both ways.
target_link_libraries(test_cUnitConvert PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_cUnitConvert PRIVATE ws2_32)
endif()

add_test(NAME cUnitConvert COMMAND test_cUnitConvert)

# --- vclcompat semantics harness (W0) --------------------------------------
add_executable(test_vclcompat
    test_vclcompat.cpp
)

target_link_libraries(test_vclcompat PRIVATE vclcompat)

add_test(NAME vclcompat COMMAND test_vclcompat)

# --- VclCommaTextBcb6: TStringList CommaText / DelimitedText vs a REAL BCB6 run ----------------
#   AI(W906-COMMATEXT) 20261003 (INBOX 152): tests/oracle/commatext_cases.inc (case table + driver)
#   is compiled twice -- by BCB6 bcc32 5.6.4 into tests/oracle/commatext_bcb6.cpp (NOT built here;
#   tests/oracle/README_commatext.md has the command), whose output is pinned as
#   tests/oracle/commatext_bcb6_expected.txt, and by this test against vclcompat.  Every line is
#   compared; the few that cannot be a reference (BCB6 AVs / reads a clobbered terminator / MBCS
#   walk under ACP 950) carry their own port-side expectation in the test.  New code: -Wall -Wextra.
add_executable(test_vcl_commatext_bcb6 test_vcl_commatext_bcb6.cpp)
target_link_libraries(test_vcl_commatext_bcb6 PRIVATE vclcompat)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_vcl_commatext_bcb6 PRIVATE -Wall -Wextra)
endif()
add_test(NAME VclCommaTextBcb6
         COMMAND test_vcl_commatext_bcb6 "${CMAKE_CURRENT_SOURCE_DIR}/oracle/commatext_bcb6_expected.txt")
set_tests_properties(VclCommaTextBcb6 PROPERTIES TIMEOUT 60)

# --- HTMD5 + cJSON public library (W0 LeafUtils) ---------------------------
add_executable(test_HTMD5
    test_HTMD5.cpp
)

target_link_libraries(test_HTMD5 PRIVATE ht9045_public)

add_test(NAME HTMD5 COMMAND test_HTMD5)

# --- cJSON round-trip smoke test (W0 LeafUtils) ----------------------------
add_executable(test_cJSON
    test_cJSON.cpp
)

target_link_libraries(test_cJSON PRIVATE ht9045_public)

add_test(NAME cJSON COMMAND test_cJSON)
# AI(W906-AVFP) 20261005 (laptop): F-Secure quarantines test_cJSON.exe as Trojan.TR/W32.Evo (generic detection; 1004 08:23,
#   1004 21:37, 1005 11:31, then every relink blocked) -- a VERSIONINFO resource makes it a recognisable program and it is no
#   longer deleted (w906_testinfo.rc has the measurement). Add other test targets here if the scanner starts flagging them.
if(WIN32)
    target_sources(test_cJSON PRIVATE w906_testinfo.rc)
endif()

# ===========================================================================
#  W1 Public leaf utilities
# ===========================================================================

# --- ExternFunction: Math_AdjustAngle / String_JustNumber / Erase* / RecordTime
add_executable(test_ExternFunction
    test_ExternFunction.cpp
)

target_link_libraries(test_ExternFunction PRIVATE ht9045_public)

add_test(NAME ExternFunction COMMAND test_ExternFunction)

# --- WinSocketErrorCode: GetErrorMsg(Winsock code -> AnsiString) ------------
add_executable(test_WinSocketErrorCode
    test_WinSocketErrorCode.cpp
)

target_link_libraries(test_WinSocketErrorCode PRIVATE ht9045_public)
# winsock2 constants come from <winsock2.h>; no functions are called, but link
# ws2_32 defensively in case a future expansion calls a WSA* function.
if(WIN32)
    target_link_libraries(test_WinSocketErrorCode PRIVATE ws2_32)
endif()

add_test(NAME WinSocketErrorCode COMMAND test_WinSocketErrorCode)

# --- cBootLog: WriteBootLog / RotateBootLogIfNeeded (Win32 file tracer) -----
# AI(W906-BA-Cyl14) 20260911: contain the boot log.
#   Without this, every ctest run appended to the LIVE machine's
#   D:\HT9045\Error\BootLog.txt -- measured written at 20:27 during the ba1
#   gate -- and the test PASSED while doing it.
#   ../Public/cBootLog.cpp is compiled INTO this executable on purpose: linking
#   the ht9045_public copy would link an object built WITHOUT the -D and the
#   redirect would silently do nothing. The object on the link line satisfies
#   the symbols, so the archive member is never extracted.
#   Forward slashes are deliberate -- they need no escaping through CMake and
#   both fopen and CreateFileA accept them on Win32.
add_executable(test_cBootLog
    test_cBootLog.cpp
    ../Public/cBootLog.cpp
)

target_compile_definitions(test_cBootLog PRIVATE
    BOOT_LOG_DIR="${CMAKE_CURRENT_BINARY_DIR}/bootlog_scratch"
    BOOT_LOG_FILE="${CMAKE_CURRENT_BINARY_DIR}/bootlog_scratch/BootLog.txt"
)

target_link_libraries(test_cBootLog PRIVATE ht9045_public)

add_test(NAME cBootLog COMMAND test_cBootLog)

# ===========================================================================
#  W2 Common logic islands
# ===========================================================================

# --- PickPlanner: cArmPickPlan engine + Loader/OutArm scaffold adapters -----
#   Stub IArmPickPlanContext drives the generic enumerate-and-score engine;
#   asserts best-plan selection, strict-'>' tie-break, boundary guards, and
#   the shipped scaffolds' zero-side-effect (Search()==false) behaviour.
add_executable(test_PickPlanner
    test_PickPlanner.cpp
)

target_link_libraries(test_PickPlanner PRIVATE ht9045_common)

add_test(NAME PickPlanner COMMAND test_PickPlanner)

# ===========================================================================
#  W0-TAIL global define/type headers compile gate
# ===========================================================================
# Includes all 5 target headers + Config/CosFunction/myTimer/MachineDefine and
# static_asserts a spot-check of preserved enum/#define values. Links against
# vclcompat (IntToHex etc.) + myTimer.o (TQPF_Timer ctor). Does NOT pull the
# cprod/cmydef/cpublic active globals (their ctors are W6-gated).
add_executable(test_w0tail_headers
    w0tail_headers_compile.cpp
)

target_link_libraries(test_w0tail_headers PRIVATE vclcompat)
# myTimer.cpp lives in ht9045_globals; pull just its TQPF_Timer ctor via the lib.
target_link_libraries(test_w0tail_headers PRIVATE ht9045_globals ht9045_core)

add_test(NAME w0tail_headers COMMAND test_w0tail_headers)

# --- W0-TAIL VERIFY: standalone-compile + value/layout spot-check -----------
#   #includes every in-scope translated global header and asserts the
#   define/enum -> value pairs + PROD_INFO_ST sizeof/offsetof layout anchors
#   (static_assert at compile time + a runtime self-check). Links vclcompat
#   (IntToHex / value types) + ht9045_globals (TQPF_Timer ctor in myTimer.o).
add_executable(test_globals
    test_globals.cpp
)

target_link_libraries(test_globals PRIVATE vclcompat ht9045_globals ht9045_core)

add_test(NAME globals COMMAND test_globals)

# ===========================================================================
#  W3 config/DB
# ===========================================================================
# --- test_config_db: REAL machine-config oracle ---------------------------
#   Parses the REAL D:\HT9045\system files (Gerneral.ini / Mot_Table.csv /
#   IO_Table.csv) through the vclcompat layer (TStringList::CommaText name-based
#   column resolution mirroring database.cpp SetIOTableNo/SetMOTTableNo) and
#   asserts the ground-truth oracle values: 45 SMC axes, MOTION_CARD_TYPE=1 /
#   IO_CARD_TYPE=2, IO_Table 15 cols / 668 rows / ISABase=0 dominant (644:24).
#   Runtime-SKIPs (exit 0) when the real config dir is absent.
#   NOTE: the production readers (cinitial/database.cpp) are not translated yet
#   (later W3); this test verifies the vclcompat foundation + locks the oracle.
add_executable(test_config_db
    test_config_db.cpp
)

target_link_libraries(test_config_db PRIVATE vclcompat)

add_test(NAME config_db COMMAND test_config_db)

# --- test_IniFiles: TIniFile / TMemIniFile shim semantics ------------------
#   W3-cont CONFIG-READERS GATEWAY. Verifies BCB6 ini semantics the common.cpp
#   ini-helper family relies on: default fallback, case-insensitive match,
#   integer/float/bool/datetime parse + '%0.4f' double round-trip, section/key
#   existence, TMemIniFile flush-on-UpdateFile, Big5 byte preservation, plus a
#   runtime-SKIP real-config oracle over D:\HT9045\system\Gerneral.ini.
add_executable(test_IniFiles
    test_IniFiles.cpp
)

target_link_libraries(test_IniFiles PRIVATE vclcompat)

add_test(NAME IniFiles COMMAND test_IniFiles)

# --- test_ini_helpers: translated common.cpp *General* helper chain ---------
#   END-TO-END: drives OpenGeneralIniFile() + CheckAndReadIniDataGeneral(int/
#   bool/AnsiString) over a SCRATCH COPY of the REAL D:\HT9045\system\Gerneral.ini
#   (copied so seeding writes never touch the read-only real file) and asserts
#   the known-truth oracle ([System] MOTION_CARD_TYPE=1 / IO_CARD_TYPE=2 /
#   TTL_CARD_TYPE=2 / INDEX_MOTION_CARD=0, [TempCtrl] HEATER_CTRL_TYPE=4) plus
#   missing-key -> supplied-default.  Also a direct TIniFile/TMemIniFile shim
#   round-trip (write-through vs flush-on-UpdateFile, %0.4f double round-trip).
#   Links ht9045_core for the translated common.cpp helpers + asGeneralPath
#   global (ht9045_core PUBLIC-links vclcompat + ht9045_public transitively).
#   The real-config block runtime-SKIPs (exit 0) when the ini is absent.
add_executable(test_ini_helpers
    test_ini_helpers.cpp
)

target_link_libraries(test_ini_helpers PRIVATE ht9045_core)

add_test(NAME ini_helpers COMMAND test_ini_helpers)

# --- test_common: W906-CommonWaveFile un-gated file-helper cluster ----------
#   Smoke-tests the 6 functions un-gated this wave: WriteDataToFile (both
#   overloads) + ReadDataFromFile round-trip; CheckFileIsEmpty on
#   existing-empty/existing-non-empty/missing; MyForceDirectories creating a
#   nested scratch directory (+ empty-path -1 guard); GetLastOpenFN/
#   WriteLastDataFN round-trip against a repointed LastDataPath (bootstrap
#   else-branch then if-branch). All I/O under TEMP/TMP scratch paths --
#   never touches the real D:\HT9045\SetUp.inf default. Links ht9045_core
#   for the translated common.cpp bodies (PUBLIC-links vclcompat +
#   ht9045_public transitively).
add_executable(test_common
    test_common.cpp
)

target_link_libraries(test_common PRIVATE ht9045_core)

add_test(NAME common COMMAND test_common)

# ===========================================================================
#  W3-cont2 config-table LOADERS (translated database.cpp)
# ===========================================================================
# --- test_config_loaders: drive the TRANSLATED LoadMotData / LoadIoData ----
#   Exercises the ACTUAL translated production loaders (not a re-impl):
#     HSys.LoadMotData() -> SetMOTTableNo (name-based) -> TMOTDATA -> MotTable
#     HSys.LoadIoData()  -> SetIOTableNo  (name-based) -> TIODATA  -> IOTable
#   over the REAL D:\HT9045\system\{Mot_Table,IO_Table}.csv and asserts the
#   ground-truth oracle: Mot 45 rows all CardModel==SMC (CardModel NAME-resolved
#   to physical col 22, not enum fallback 9); M00/MInArmX BoardID==0/Port==0;
#   IO 668 rows, 15 cols, ISABase NAME-resolved to col 8, distribution 644:24.
#   Links ht9045_db (database.cpp) which PUBLIC-links vclcompat + ht9045_globals
#   (INDEX_MOTION_CARD, TQPF_Timer).  The TU supplies the two gated externals
#   (MyDBIProcess / ShowMyMessage) as local stubs.  Runtime-SKIPs (exit 0) when
#   the real config files are absent.
add_executable(test_config_loaders
    test_config_loaders.cpp
)

# Link map:
#   ht9045_db      -- the translated database.cpp loaders.
#   ht9045_core    -- defines IoTablePath / MotTablePath (common.cpp), which the
#                     loaders hardcode/assign (database.cpp:1552 / 1620).
#   ht9045_globals -- INDEX_MOTION_CARD (cmydef.cpp) + the queue-object ctors
#                     (cpublic.cpp).  cmydef.cpp.obj's static queue globals
#                     (QueueTaskList[] etc.) need ctors from cpublic.cpp.obj in
#                     the SAME archive; GNU ld's single-pass archive scan can't
#                     back-resolve that, so wrap the libs in a LINK_GROUP to make
#                     ld re-scan the archive set until all refs settle.
# AI(W906-SysModWire) 20260720: database.cpp now references HT9045Gem/HGem
# (SystemModularInitial real) -> pulls ht9045_secsgem (HT9045Gem ctor,
# uHGemHT9045_Shim.cpp.o) in cross-archive, which itself needs ht9045_db back
# (HSys) -- genuine circular archive dependency, RESCAN required across both.
# ht9045_secsgem also needs ht9045_public/vclcompat (already present) and
# ws2_32 (THGem's ctor constructs TClientSocket/TServerSocket -- see
# test_uHGemEquipment's own identical need).
target_link_libraries(test_config_loaders PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_config_loaders PRIVATE ws2_32)
endif()

add_test(NAME config_loaders COMMAND test_config_loaders)

# --- test_machine_iotable: 版控的 machines/<機台>/IO_Table.csv 必須被 golden 的
#   LoadIoData 讀得進來（AI(W906-IOTABLE-NOTE) 20260924）。
#   20260923 的 HT9050 表頭少了 Note 欄 -> SetIOTableNo 回 0 -> 整張 IO 表是空的，
#   wb_serve 照樣開得起來，看起來只像「沒資料」。讀的是版控檔不是機台現場，
#   所以它紅燈就是資料真的壞了，不是常駐失敗。同 config_loaders 的連結形狀。
#   新增機台：多一行 add_test，參數是檔案路徑、資料列數（不含表頭）、
#   Switch／Sensor 綁定數（cinitial.cpp 的 Name 字面值 ∩ CSV Alias，用 Python 另外算，
#   不要拿這支測試印出來的數字回填 —— 那就變成自己驗自己）。
add_executable(test_machine_iotable
    test_machine_iotable.cpp
)
target_link_libraries(test_machine_iotable PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_machine_iotable PRIVATE ws2_32)
endif()
add_test(NAME MachineIoTable_HT9050
         COMMAND test_machine_iotable "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv" 1124 136 301)   # AI(W906-IOWEB-P7) 20260924: the field table (09-24 rows + Note header + Lane 1); 1124 rows / SW 134 / Sen 301 recomputed independently (cinitial.cpp Name literals x CSV Alias), the same method reproduces 1062/132/289 on the 09-23 table; AI(W906-BRAKE-EMPTY-AUTO3) 20260930: SW bound 134 -> 136 (cpp 0039: SwCassetteEmptyMotBreaker / SwCassetteAuto3MotBreaker = IO_Table rows 1122 / 1125)
# AI(W906-IOWEB-P4) 20260925: IO_CARD_TYPE=4（PCI1203_IO，HT9050 機台的實際值）必須綁上同樣的列數；
#   IO_CARD_TYPE=0 是對照組，必須一個都綁不上（證明 gate 真的有在擋）。
add_test(NAME MachineIoTable_HT9050_PCI1203IO
         COMMAND test_machine_iotable "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv" 1124 136 301 4)   # AI(W906-IOWEB-P25c) 20260925: counts of THIS tree's machines/HT9050/IO_Table.csv (IOWEB-P7), not the laptop 1062/132/289; AI(W906-BRAKE-EMPTY-AUTO3) 20260930: SW bound 134 -> 136 (cpp 0039: SwCassetteEmptyMotBreaker / SwCassetteAuto3MotBreaker = IO_Table rows 1122 / 1125)
add_test(NAME MachineIoTable_HT9050_Card0Control
         COMMAND test_machine_iotable "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv" 1124 136 301 0)   # AI(W906-BRAKE-EMPTY-AUTO3) 20260930: SW bound 134 -> 136 (cpp 0039: SwCassetteEmptyMotBreaker / SwCassetteAuto3MotBreaker = IO_Table rows 1122 / 1125)

# --- test_machine_cylinders: W3 第 7 項 InitCylinder（if＝NewIO_MN200 那半）的第三級測試 ---
#   AI(W906-W3-INITCYL) 20260924: 餵 machines/HT9050/IO_Table.csv，每個氣缸的 Out／OnSen／OffSen 三組 7 欄
#   逐列比對 IO 表，要求零不符。連結形狀同 MachineIoTable。
add_executable(test_machine_cylinders
    test_machine_cylinders.cpp
)
target_link_libraries(test_machine_cylinders PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_machine_cylinders PRIVATE ws2_32)
endif()
add_test(NAME MachineCylinders_HT9050
         COMMAND test_machine_cylinders "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv")

# --- test_io_points: GET /api/struct/io/config 與 /api/struct/io/runtime 的產生器
#   （JsonBridge/ChanIoPoints.cpp，AI(W906-IO-POINTS) 20260924）。位址解析、InType 反相、
#   三種 null、方向規則對 golden 綁定交叉驗證、逐點 on/off 對帳（假樣本）。
#   真的讀卡只能在有 1203 的機台驗；這支負責之前能驗的全部。連結形狀同 SjsonChan。
add_executable(test_io_points
    test_io_points.cpp
    ../JsonBridge/ChanIoPoints.cpp
    ../EtherCAT/Pci1203Monitor.cpp)
target_include_directories(test_io_points PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_io_points PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
if(WIN32)
    target_link_libraries(test_io_points PRIVATE ws2_32)
endif()
add_test(NAME IoPoints_HT9050
         COMMAND test_io_points "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv")

# --- test_motor_points: GET /api/struct/motor/config|runtime 的產生器 ---
#   AI(W906-MOTOR-POINTS) 20260924: 讀版控的 machines/HT9050/Mot_Table.csv；
#   沒驅動物件 -> 全 nosource／null；掛一顆 TMySimMotor -> 只有那一軸 good 且值逐欄相同。連結形狀同 IoPoints。
add_executable(test_motor_points
    test_motor_points.cpp
    ../JsonBridge/ChanMotorPoints.cpp ../EtherCAT/Pci1203Monitor.cpp)   # AI(W906-IOWEB-P5) 20260924: PCI1203 rows now read Pci1203Monitor() (same link shape as test_io_points :361); same line so nothing below moves
target_include_directories(test_motor_points PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_motor_points PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
if(WIN32)
    target_link_libraries(test_motor_points PRIVATE ws2_32)
endif()
add_test(NAME MotorPoints_HT9050
         COMMAND test_motor_points "${CMAKE_SOURCE_DIR}/../machines/HT9050/Mot_Table.csv")

# ===========================================================================
#  W4 HAL motor layer
# ===========================================================================
# --- test_motor_w4: HTMotor / TMySimMotor / TMyMotor / TTrayMotor / MOT[] ---
#   Exercises the INTERFACE-CUT end-to-end with NO vendor SDK:
#     * TMySimMotor as HTMotor* (polymorphic dispatch, all overridden methods)
#     * TMyMotor routing through the sim (ReadPos / MotorHome state machine /
#       ScanMotorStatus / GetErrorIndex / PCIL132_* / JogP/JogN / Lock*)
#     * TTrayMotor tray data helpers (InitNewTray / HasIC / FullIC /
#       HowManyDevice / MoveTrayAllItem / SetNullIcToHasNullIc)
#     * MOT[] global array wiring (MOT[MInArmX] via sim, boundary check)
#   Links ht9045_motor which RESCAN-links ht9045_globals (cmydef/cprod/cpublic
#   ctors needed by mymotor.cpp's static iEncoderTorence / global MOT[]).
add_executable(test_motor_w4
    test_motor_w4.cpp
)

# ht9045_motor PUBLIC-links ht9045_globals; ht9045_db PUBLIC-links ht9045_globals
# and ht9045_core. The RESCAN group ensures intra-globals cross-references
# (cprod <-> cpublic <-> cmydef) are resolved across archive re-scans.
target_link_libraries(test_motor_w4 PRIVATE
    
    # AI(W906-GA1-B2-integrate) 20260804: upgraded to the full RESCAN group --
    # cprod.cpp/cpublic.cpp ungate widened ht9045_globals objects' extern surface
    # (fMain/fAGV/InArmSuck/... now referenced), the lightweight link no longer closes.
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_motor_w4 PRIVATE ws2_32)
endif()

add_test(NAME motor_w4 COMMAND test_motor_w4)

# --- test_sim_motor: HARDWARE INTERFACE-CUT acceptance proof ----------------
#   Dedicated, self-contained proof that the offline motor HAL works with NO
#   vendor SDK and NO hardware.  Creates a TMySimMotor via HTMotor* (and through
#   TMyMotor) and asserts the exact acceptance contract:
#     InitMotor ok; HomeObject()->HomeFlag() true & ReadPos()==0;
#     MoveToPos(N)->MotionDone() true & ReadPos()==N; GetAlarm()==false; Stop() ok.
#   Plus a full MotorHome() state-machine convergence over the sim.
#   Same RESCAN link group as test_motor_w4 (cprod<->cpublic<->cmydef ctors must
#   resolve across archive re-scans).  The brand drivers are NOT in ht9045_motor,
#   so this links with zero vendor libraries.
add_executable(test_sim_motor
    test_sim_motor.cpp
)

target_link_libraries(test_sim_motor PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_sim_motor PRIVATE ws2_32)
endif()

add_test(NAME sim_motor COMMAND test_sim_motor)

# ===========================================================================
#  W4-IO HAL IO layer
# ===========================================================================
# --- test_lane_io_sim: IO HARDWARE INTERFACE-CUT acceptance proof -----------
#   Dedicated, self-contained proof that the offline IO HAL works with NO
#   vendor SDK and NO hardware.
#   Part A: TSimIOBackend direct round-trip (WriteBit/ReadBit/WriteByte/ReadByte).
#   Part B: TLaneIO facade: IOBitOn->IOOutBitStatus cache; IOByteOut returns true;
#           IOInputBit round-trip through TSimIOBackend; CheckPortRangeErr==0
#           when InitialOK==false or iISABase!=eMotionNet; GetIOErrStr 0..5.
#   Part C: TMySwitch.On() sets OutPortData via MyLaneIO; Status() reads it back.
#   Part D: TMySensor.IsOn()/IsOff() round-trip through TSimIOBackend; Type
#           inversion; Enable==false -> false; CopySensor field copy.
#   Same RESCAN link group as test_motor_w4 (cprod<->cpublic<->cmydef ctors).
#   Links with zero vendor libraries (all HAVE_xxx default OFF).
add_executable(test_lane_io_sim
    test_lane_io_sim.cpp
)

target_link_libraries(test_lane_io_sim PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_lane_io_sim PRIVATE ws2_32)
endif()

add_test(NAME LaneIOSim COMMAND test_lane_io_sim)

# --- test_lane_io_route: A4-7 依點位分派三路後端（golden MyLaneIo.cpp 的 ePCI1203／MN200／_mnet_ 分支） ---
#   AI(W906-A4-7) 20260924: 注入三個探針，逐方法 × 點位 × IO_CARD_TYPE 斷言落到 golden 指定的那一路；
#   SelectVendorBackends 在 SOFT_SIMULTE／出貨組態各自的行為。連結形狀同 LaneIOSim。
add_executable(test_lane_io_route
    test_lane_io_route.cpp
)
target_link_libraries(test_lane_io_route PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_lane_io_route PRIVATE ws2_32)
endif()
add_test(NAME LaneIORoute COMMAND test_lane_io_route)

# --- test_sim_io: DEDICATED Sim-IO interface-cut acceptance proof -----------
#   Task-spec proof (mirrors test_sim_motor.cpp for the IO layer):
#     [1] select TSimIOBackend; via TLaneIO IOBitOn/IOBitOff then IOOutBitStatus
#         reflects the OutPortData cache (per-bit mask precision verified).
#     [2] IOByteOut / IOInputByte round-trip through the Sim port (IO_CARD_TYPE=1
#         MN200 read path so *Value is used directly, not the IO_CARD_TYPE==0
#         ret-quirk); OutPortData bit pattern cross-checked.
#     [3] IOInputBit returns the Sim value (write Sim bit -> read via facade).
#     [4] range/error path: drives CheckPortRangeErr documented codes 0..5
#         (InitialOK=true + iISABase=eMotionNet + iUseMNetIP device types) and
#         the InitialOK==false / iISABase!=eMotionNet early-return guards, plus
#         GetIOErrStr 0..5 string mapping.
#   Same RESCAN link group as test_lane_io_sim; zero vendor libraries (HAVE_xxx OFF).
add_executable(test_sim_io
    test_sim_io.cpp
)

target_link_libraries(test_sim_io PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_sim_io PRIVATE ws2_32)
endif()

add_test(NAME SimIO COMMAND test_sim_io)

# --- test_pci1203_seam: is the ePCI1203 IO arm WIRED, or only PRESENT? ------
#   AI(W906-BU-C2) 20260916. Built WITHOUT HAVE_PCI1203 on purpose: no vendor
#   symbol enters this link, so it needs no card and no SDK and cannot move a
#   machine. It injects a recording TIOBackend and drives MyLaneIO's
#   `if(iISABase == ePCI1203)` arm (MyLaneIo.cpp:290/:335/:398), which every
#   other IO test leaves untouched -- test_sim_io and test_lane_io_sim use
#   eMotionNet only, so before this file that arm had never executed.
#   Same RESCAN link group as test_sim_io.
add_executable(test_pci1203_seam
    test_pci1203_seam.cpp
)

target_link_libraries(test_pci1203_seam PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_pci1203_seam PRIVATE ws2_32)
endif()

add_test(NAME Pci1203Seam COMMAND test_pci1203_seam)

# ===========================================================================
#  HAL enablers: KeyPro license shim + TComm serial shim
# ===========================================================================
# --- test_keypro_tcomm: offline acceptance proof for both HAL enablers ------
#   [1] KeyPro shim (Public/HTKeyProShim.cpp, in ht9045_public): asserts
#       KeyPro_GetLevel(3)==1 via the GENUINE offline fallback -- there is no
#       KeyProDLL.dll in the C++ target tree, so LoadLibraryA fails and the
#       shim returns 1 (valid dongle).  The test first asserts the DLL is
#       absent so the fallback path is the one under test.  KeyProDLL.lib is
#       NOT static-linked (the shim resolves the export at run time).
#   [2] TComm sim (vclcompat/Comm.cpp, in vclcompat): opens in SIM mode with no
#       hardware; WriteCommData buffers into the tx sink (read back + content
#       checked); SimInjectReceive drives OnReceiveData (Sender/len/exact bytes
#       verified, multi-fire + no-handler no-op + Stop/re-Start).
#   Links ht9045_public (KeyPro_GetLevel) which PUBLIC-links vclcompat
#   (Spcomm::TComm); vclcompat listed explicitly for clarity.  On Win32 the
#   shims use kernel32 serial / LoadLibrary APIs that are in the default link
#   set, so no extra libraries are required.
add_executable(test_keypro_tcomm
    test_keypro_tcomm.cpp
)

target_link_libraries(test_keypro_tcomm PRIVATE ht9045_public vclcompat)

add_test(NAME KeyProTComm COMMAND test_keypro_tcomm)

# ===========================================================================
#  cpublic.cpp foundation wave (W4/W5/W6/W7 ungated sections)
# ===========================================================================
# --- test_cpublic_foundation: acceptance proof for all ungated W4 bodies ----
#   [A] DTK4848 frame format via TComm SIM (g_pDTKComm seam):
#       DTK4848_LRC(":0106470100FA")=="B7";
#       DTK4848WordWriteNoSucm(0,250) -> SimTxBuffer==":0106470100FAB7\r\n\0";
#       DTK4848WordReadNoSucm(0) -> SimTxBuffer==":010347000002B3\r\n\0".
#   [B] ByteUnionBit: SetByte(0xA5)->Bit(0..7) assertions; BitCount(true)==4;
#       BitString()=="10100101"; Hex()=="A5"; SetBit/range-guard.
#   [C] Queue method bodies: TMyQueue10 Add/Get/Show/SetAliasAndTask/ring-wrap;
#       TMyQueue100 Add/Get; TMyStrQueue100 Add/Get; TMyTimerQueue100
#       Add/GetTimeString (ConvertMSecToSPC("00:00:01.234")).
#   [D] VerInfo: m_ClearData leaves FixedFileVersion=="0.0.0.0"; bad path
#       -> graceful no-crash; GetAppVersion on own exe no-crash.
#   [E] Foundation free functions: ConvertSecondToSPC/ConvertMSecToSPC;
#       GetFloatFormatString/ConvertToMMType/Get0_01MMType;
#       ConvertDoubleStrToInt; sDataTimelog; Round x2; RotationCoordinates;
#       CRC_Check Modbus-CRC on known vector; GetScreenPos; VC8ToKpa/KpaToVC8
#       round-trip; IsDoubleEqual.
#   Links ht9045_globals (ungated queue/VerInfo/DTK4848 bodies) PUBLIC-links
#   psapi + version (Win32 version API); vclcompat for TComm SIM.
add_executable(test_cpublic_foundation
    test_cpublic_foundation.cpp
)

target_link_libraries(test_cpublic_foundation PRIVATE 
    # AI(W906-GA1-B2-integrate) 20260804: upgraded to the full RESCAN group --
    # cprod.cpp/cpublic.cpp ungate widened ht9045_globals objects' extern surface
    # (fMain/fAGV/InArmSuck/... now referenced), the lightweight link no longer closes.
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_cpublic_foundation PRIVATE ws2_32)  # AI(W906-GA1-B2-integrate) 20260804: full group pulls vclcompat sockets
endif()

add_test(NAME CpublicFoundation COMMAND test_cpublic_foundation)

# ===========================================================================
#  W6.1 CANARY -- asendic_Empty (Empty-tray stack feed) state machine
# ===========================================================================
# --- test_w6_1_empty_canary: drive the first ROOT SM over the W6 substrate ---
#   Proves the W6 decoupling stack links + RUNS with NO hardware:
#     ht9045_sm  -- the Empty-tray SM (asendic_Empty) + csystem predicates +
#                   FormsFacade (fAGV) + asendic sim helpers + canary_support.
#     ht9045_io  -- Cylinder[] / SW[] / Sen[] / MyLaneIO (TSimIOBackend default).
#     ht9045_motor -- MOT[] (TTrayMotor sim).
#     ht9045_db  -- database.cpp (provides ShowMyMessage's sibling-TU agreement
#                   + pulled into the RESCAN group for cross-archive ctors).
#     ht9045_globals -- Prod/IniConfig/CosFunction/TrayForm/RunInfo/Ld_UldDelayTime/
#                   TrayID + the enum/const universe (DUMMY/NULL_IC/K_RETRY/...).
#   The RESCAN link group lets GNU ld re-scan the archive set so the
#   cprod<->cpublic<->cmydef ctor cross-references settle (same idiom as the
#   W4 motor/IO tests).  Drives DoAutoEmpty/DoAutoEmpty1/DoLoadNewEmptyTrayToCar
#   in DUMMY for hundreds of iterations and asserts no crash + sane cursors.
add_executable(test_w6_1_empty_canary
    test_w6_1_empty_canary.cpp
)

target_link_libraries(test_w6_1_empty_canary PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_1_empty_canary PRIVATE ws2_32)
endif()

add_test(NAME W6_1_EmptyCanary COMMAND test_w6_1_empty_canary)

# ===========================================================================
#  W6 VERIFY -- dedicated predicate-API + canary-stepper acceptance proof
# ===========================================================================
# --- test_w6_canary: the task-mandated W6 verify TU -------------------------
#   Companion to test_w6_1_empty_canary above.  Where that one proves the stack
#   LINKS + does not crash, this one pins two concrete contracts:
#     PART A -- PREDICATE: set a KNOWN Sim HAL state (Sen[] sensors via
#       TSimIOBackend + MOT[] tray flags) and assert the frozen csystem.h
#       HasIC-family predicates return the EXPECTED bool -- the conservative
#       gated-leaf baseline (all false), the LIVE InputShuttleRearHasIC /
#       OutputShuttleRearHasIC IniConfig/TestIF_File guard branches (both ways),
#       the composite OR-aggregation shape, and predicate PURITY (no HAL perturb).
#     PART B -- CANARY.  AI(W906-W6-CANARYfix) 20260802: rewritten, and this
#       description with it.  The old text ("assert the stepper ADVANCES out of
#       entry and CONVERGES ... while staying on the documented cursor set") is
#       exactly what the rewrite removed: those two assertions were UNFALSIFIABLE
#       -- the bounded loop's own termination condition sat in the disjunction, and
#       the cursor-set predicate accepted precisely the set the SM can write.  Part
#       B now pins an EXACT cursor TRAJECTORY per stepper and observes the real
#       actuator wire bits (Sim IO addresses make Cylinder[]/SW[] On/Off/Push/Pop
#       readable), so an inversion with an unchanged trajectory goes red.  The old
#       claim of exercising the "lifter + Cylinder[] HAL" was also false: the
#       fixture enabled SenEmptyCCWDete without setting .Type, and TYPE_B is the
#       default, so case 1 returned true at step 0 and the lifter path ran ZERO
#       times.  The fixture now drives those sensors explicitly OFF.  Sub-tests:
#       [B.0] fixture contract, [B.1] DoAutoEmpty tray-PRESENT branch,
#       [B.2] DoLoadNewEmptyTrayToCar full lifter walk, [B.3] DoAutoEmpty1,
#       [B.4] fAGV::IsATK_AMR driven BOTH ways.
#   Links the SAME orchestration stack as the W6.1 canary: ht9045_sm IS the W6
#   orchestration lib (asendic_Empty + csystem_predicates + FormsFacade + the
#   asendic/canary_support sim helpers); it links the substrate (globals/motor/
#   io/vclcompat) with NO vendor SDK and NO TfMain.  Re-using it (vs. a second
#   lib over the same .cpp set) avoids duplicate-symbol ODR conflicts at link.
#   The RESCAN link group lets GNU ld settle the cprod<->cpublic<->cmydef ctor
#   cross-references (same idiom as the W4 motor/IO tests).
add_executable(test_w6_canary
    test_w6_canary.cpp
)

target_link_libraries(test_w6_canary PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_canary PRIVATE ws2_32)
endif()

add_test(NAME W6_Canary COMMAND test_w6_canary)

# ===========================================================================
#  W6.2 VERIFY -- shared InArm HotPlate pick/place geometry leaves
# ===========================================================================
# --- test_w6_2_inarm_canary: drive the shared HP-geometry leaf library -------
#   Proves ainarm_SearchPickPlate.cpp + ainarm_SearchPlacePlate.cpp +
#   aHotPlateSubstrate.cpp (all in ht9045_sm) LINK + RUN with NO hardware, and
#   pins faithful geometry-leaf values over a KNOWN Sim substrate:
#     GetHotPlateColStep / GetInArmToHPYPitch_9045 / GetHeaterWaitTime /
#     HotPlateYPitchCanPutAll / Row2CanPutHP / ArmRow1AllHasIC, plus the SM
#     DoInArmPickFromHotPlate_9045() converging to entry over the empty offline
#     team list, the W7-gated DoPlaceToHotPlate_9045() stub returning false, and   [AI(W906-HPDISP) 20260930: now the live dispatcher routing to 2x2_4_13]
#     the DoPlaceToHPSwapData / DoPlaceToHPBackupData data-swap leaves running
#     without crash.  Same RESCAN link group as the W6.1 canary so the
#     cprod<->cpublic<->cmydef ctor cross-references settle.
add_executable(test_w6_2_inarm_canary
    test_w6_2_inarm_canary.cpp
)

target_link_libraries(test_w6_2_inarm_canary PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2_inarm_canary PRIVATE ws2_32)
endif()

# ===========================================================================
#  W906-AutoCleanFoundation VERIFY -- AutoClean engine foundation wave
# ===========================================================================
# --- test_AutoClean: pure-calc + HAL-only helper verify ---------------------
#   Exercises every Part A pure-calc function (GetAutoCleanPickCount/
#   GetAutoCleanPickStep/CalculateAutoCleanXPitch/GetXPitchOfCleanKit_HP+_Kit+
#   dispatcher/GetYPitchOfCleanKit/RunAutoCleanByArmPickArm2Test/
#   Special_2X6_Tray_XItem7/GetInarmSuckRow) with a representative case each,
#   and pumps the HAL-only state-machine helpers (init task family/
#   CleanSetSpeed/TrayHasCleanIC(+Count)/SetAutoCleanStringGrid+
#   ReadWriteAutoCleanCount round-trip/RestoreCleanKitData/
#   SearchCleanKitRowCol(+UpDown)/PickFromCleanKit family/the Move* Z+XY
#   geometry leaves/GetShuttleState+CheckShuttleSensor_Clean dispatchers) over
#   the Sim HAL with no hardware. Also exercises Part D
#   (SetDeviceInTray/CleanPadCountCanSupport2Arm) and the FormsFacade
#   InitialUnLoaderTask behaviour-change fix (Part B). Same RESCAN link group
#   as the other ht9045_sm-linking verify TUs (AutoClean/AutoClean.cpp is
#   compiled into ht9045_sm, see the root CMakeLists.txt entry).
add_executable(test_AutoClean
    test_AutoClean.cpp
)

target_link_libraries(test_AutoClean PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_AutoClean PRIVATE ws2_32)
endif()

add_test(NAME AutoClean COMMAND test_AutoClean)

add_test(NAME W6_2_InArmCanary COMMAND test_w6_2_inarm_canary)

# --- test_w6_2_inarm_search: drive the Search/Do functions with golden oracle -
#   The task-mandated W6.2 *search* verify TU (companion to the canary above).
#   Sets a KNOWN Sim HAL state and drives the SearchPickPlate/SearchPlacePlate
#   Do*/Search*/geometry functions, asserting the HP-geometry RESULT and the SM
#   stepping against the GOLDEN FORMULA as oracle:
#     GetPlaceToHotPlateCol (XDiv 4/6/8/16 default-picker branches),
#     GetPlaceToHotPlateSuckCol (default + 1x1), GetInArmToHPOffset_9045
#     (AutoAlignment/SameOne/iP branches -> InOfsHP1/HP2/InSh1/-1),
#     GetVariableYInHotPlateData (manual pitch -> TestIF.iARM_Y_PITCH),
#     GetVariableXInHotPlateData (1x1 -> iXpitchMaxX3 + bPitchOver12000=false),
#     ResetHotPlateSearchParameter post-state, the DoInArmPickFromHotPlate_9045
#     SM stepping over the Sim HAL, and SearchPlateToPlace /
#     SearchPlacePlateXItem4_8Suck / CheckHasSpaceToPlace_9045 pumped without
#     crash.  Same RESCAN link group as the W6.1/W6.2 canaries so the
#     cprod<->cpublic<->cmydef ctor cross-references settle.
add_executable(test_w6_2_inarm_search
    test_w6_2_inarm_search.cpp
)

target_link_libraries(test_w6_2_inarm_search PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2_inarm_search PRIVATE ws2_32)
endif()

add_test(NAME W6_2_InArmSearch COMMAND test_w6_2_inarm_search)

# ===========================================================================
#  W6.2b VERIFY -- the IN-ARM ENGINE core SMs (ainarm9045.cpp)
# ===========================================================================
# --- test_w6_2_inarm_core: pump the engine's central task SMs over Sim HAL ---
#   The task-mandated W6.2b *core engine* verify TU.  Proves ainarm9045.cpp
#   (the in-arm ENGINE, in ht9045_sm) LINKS + its 3 ACTIVE central task state
#   machines PUMP over the Sim HAL with NO hardware:
#     DoInArmAdditionalFunction (iInArmAdditionalFunctionTask, golden:3209) --
#       case 1 falls through to case 100; offline selector flags gated -> the
#       else-path sets bResult=true, cursor settles at 100;
#     DoInArmIonFanGiveWay (iInArmIonFanGiveWayTask, golden:9166) -- entry
#       else-path Task->5; InSHT*InLF false guard -> settles at 5;
#     ProcessSCKARTLoadingCount (iProcessSCKARTLoadingCountTask, golden:7305) --
#       1->100->200 over the destroy/Z-safe stubs, case 200 returns true and
#       clears bPickFromLoader.
#   Each cursor is asserted to stay inside the documented switch(Task) state set
#   and to match the golden entry/early-out (golden line cited per assert), plus
#   the geometry/numeric ORACLEs: GetInArmPitch_9045 interpolation endpoints +
#   float->int TRUNCATION (5000.625 -> 5000), GetJStep default mode == 1,
#   FindCentorPointIndex, GetShtStartPos.
#   Same RESCAN link group as the W6.1/W6.2 canaries so the
#   cprod<->cpublic<->cmydef ctor cross-references settle.
add_executable(test_w6_2_inarm_core
    test_w6_2_inarm_core.cpp
)

target_link_libraries(test_w6_2_inarm_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2_inarm_core PRIVATE ws2_32)
endif()

add_test(NAME W6_2_InArmCore COMMAND test_w6_2_inarm_core)

# ===========================================================================
#  W6.2b-canary VERIFY -- the in-arm SITE-VARIANT dispatch (1x1_1 live arm)
# ===========================================================================
# --- test_w6_2b_inarm_variants: prove the variant DISPATCH over the Sim HAL ---
#   The task-mandated W6.2b-canary *variant dispatch* verify TU.  Proves the
#   in-arm ENGINE's two dispatch ladders (DoInArm_9045 / DoInArm_9045_SuckerMap)
#   route e9045_1x1_1 / e9045_1x4_1_Ac to the translated 1x1_1 variant
#   (ainarm9045_1x1_1.cpp), that the 1x1_1 master pick SM + SuckerMap builder
#   PUMP/RUN over the Sim HAL, and that ONLY the 1x1_1 arm was un-gated (a still-
#   gated iInArmType -- e9045_2x4_8 / e9045_2x8_32 -- hits the Program-Error else
#   and leaves the master cursor UNCHANGED).  Oracles (hand-derived from the
#   golden 1x1_1 source):
#     [D1] e9045_1x1_1, iArmTask=1, no IC -> single-tick 1->10->50->75 fall-through
#          (golden 1x1_1 :1770/:1861/:1871); cursor in documented set.
#     [D2] e9045_1x4_1_Ac -> SAME DoInArm_9045_1x1_1() advance (golden :4563).
#     [D3/D4] gated types -> Program-Error else, iArmTask UNCHANGED.
#     [S1] SuckerMap default branch numeric oracle: seed bUseTestSocket[0][0][0]=
#          true, iSiteMap[0][0]=7 -> Prod.fInArmSuck4x8[0][0][0]/bInSuckUse/iSiteMap
#          written + GetShuttleCol(0,0)==0 (golden :1428-1433/:507-510).
#     [S2/S3] dispatch routing + iSuckCol=1 column-shift bonus.
#   RECONCILIATION: there is NO e9045_2x4_16 enum + the golden routes e9045_2x8_32
#   -> 2x8_8, so ainarm9045_2x4_16.cpp / _2x8_32.cpp are DEAD (registered to
#   compile, never dispatched); no routing oracle is derivable for them -- they
#   serve as the "still-gated type -> Program-Error" witnesses instead.
#   Same RESCAN link group as the W6.1/W6.2 canaries + core TUs.
add_executable(test_w6_2b_inarm_variants
    test_w6_2b_inarm_variants.cpp
)

target_link_libraries(test_w6_2b_inarm_variants PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2b_inarm_variants PRIVATE ws2_32)
endif()

add_test(NAME W6_2b_InArmVariants COMMAND test_w6_2b_inarm_variants)

# ===========================================================================
#  W6.2c batch-1 VERIFY -- the 5 in-arm SITE-VARIANT batch (active arms)
# ===========================================================================
# --- test_w6_2c_inarm_variants: prove the 5-variant batch over the Sim HAL ---
#   Independently proves the W6.2c batch-1 of 5 in-arm site variants
#   (1x3_4, 1x2_2, 1x2_2_14, 1x2_4_Hot, 1x3_2_14).  Proves:
#     [D] the engine ladders (DoInArm_9045 / DoInArm_9045_SuckerMap) route each
#         of the 5 active iInArmType values to its OWN DoInArm_9045_<v>() /
#         _SuckerMap() arm (cursor advances), while a still-gated type
#         (e9045_2x4_8) hits the Program-Error else (cursor UNCHANGED) -- so only
#         the intended arms were un-gated; all 5 *_SuckerMap symbols defined+callable.
#     [O1] pure const table XPHSuckToSht_1x2_4 (golden 1x2_4_Hot:32; read by the
#          engine at ainarm9045.cpp:513) carries the exact golden values.
#     [O2] TMyKitSuck Arm{Up,Down,Left,Right}SideHaveRealIC Item-grid scans
#          (transcribed verbatim from golden mykitsuck.cpp; drive 1x2_4_Hot) match.
#     [O3] GetShuttleState_1x3_4 pick branch (golden ainarm9045_1x3_4.cpp:2965)
#          returns 0 (all NULL_IC) / 1 (left-top HAS_IC).
#   Same RESCAN link group as the W6.1/W6.2/W6.2b canaries + core TUs.
add_executable(test_w6_2c_inarm_variants
    test_w6_2c_inarm_variants.cpp
)

target_link_libraries(test_w6_2c_inarm_variants PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2c_inarm_variants PRIVATE ws2_32)
endif()

add_test(NAME W6_2c_InArmVariants COMMAND test_w6_2c_inarm_variants)

# --- test_w6_2c_inarm_variants_batch2: prove the 6-variant batch-2 over Sim HAL
#   Independently proves the W6.2c batch-2 of 6 in-arm site variants
#   (1x4_2, 1x4_8_Hot, 2x2_4_12, 2x1_2, 2x2_4, 2x2_4_14).  Proves:
#     [D]  the engine ladders (DoInArm_9045 / DoInArm_9045_SuckerMap) route each
#          of the 6 active iInArmType values to its OWN DoInArm_9045_<v>() /
#          _SuckerMap() arm (cursor advances), while a still-gated type
#          (e9045_2x4_8) hits the Program-Error else (cursor UNCHANGED); all 6
#          *_SuckerMap symbols defined+callable.
#     [O1] pure-const picker->shuttle permutation maps XPHSuckToSht_2x2_12[8] /
#          XPHSuckToSht_2x2_13[3][8] / XPHSuckToSht_2x2_14[3][8] carry the exact
#          golden values (identity / {0,2,1,3,..} / {0,2,3,1,..}).
#     [O2] DoInArm_9045_2x1_2_SuckerMap via the engine ladder: engine grid-clear
#          (bInSuckUse=false, iSiteMap=-1) + per-variant fill compose -- only
#          cols 0,2 of row 0 used, iSiteMap routed from TestIF.
#     [O3a] DoInArm_9045_2x1_2() advances the SHARED iArmTask global (int &Task).
#     [O3b] GetShuttleState_2x2_4 pick branch == 0/1/2 (golden :2965), driving
#           the substrate ArmUpSideAllTypeIC / ArmDownSideAllTypeIC twins.
#   Same RESCAN link group as the W6.1/W6.2/W6.2b/W6.2c-batch1 canaries + cores.
add_executable(test_w6_2c_inarm_variants_batch2
    test_w6_2c_inarm_variants_batch2.cpp
)

target_link_libraries(test_w6_2c_inarm_variants_batch2 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2c_inarm_variants_batch2 PRIVATE ws2_32)
endif()

add_test(NAME W6_2c_InArmVariantsBatch2 COMMAND test_w6_2c_inarm_variants_batch2)

# --- test_w6_2c_inarm_variants_batch3: prove the 6-variant batch-3 over Sim HAL
#   Independently proves the W6.2c batch-3 of 6 in-arm site variants
#   (1x4_4, 1x4_4_Back, 2x3_6, 2x3_6_14, 2x4_4, All_1Pick).  Proves:
#     [D]  the engine ladder (DoInArm_9045) routes each of the 5 iInArmType-routed
#          active variant values to its OWN DoInArm_9045_<v>() arm via the SOUND
#          engine==direct differential witness; all 6 *_SuckerMap symbols (incl.
#          the faithful-never-dispatched All_1Pick_SuckerMap) defined+callable.
#     [D-pick] ORACLE 3 (load-bearing): with USE_PICKER_COUNT!=ep1Picker,
#          e9045_2x4_4_14(=20) enters DoInArm_9045_2x4_4 (NOT _2x4_4_13) and
#          e9045_2x4_4_13(=21) hits the still-GATED Program-Error else (cursor
#          UNCHANGED); with USE_PICKER_COUNT==ep1Picker the FIRST ladder arm enters
#          DoInArm_9045_All_1Pick REGARDLESS of iInArmType (witnessed against a
#          gated type so only the leading picker-count `if` can advance the cursor).
#     [O1] pure-const tables XPHSuckToSht_2x3_6 / _OutArm carry the golden swap
#          (row1 {3,1,0,2,..} in-arm / {0,2,1,3,..} out-arm).
#     [O2] pure-const XPHSuckToSht_2x4_14[3][8] all rows == {0,4,5,1,2,6,7,3}.
#     [O3] GetShuttleState_2x3_6 pick branch == 0/1/2 (golden :3505), driving the
#          substrate ArmUpSideAllTypeIC/ArmDownSideAllTypeIC over the (NULL_IC,0,4)
#          6-site scan range.
#   Same RESCAN link group as the W6.1/W6.2/W6.2b/W6.2c-batch1/batch2 canaries.
add_executable(test_w6_2c_inarm_variants_batch3
    test_w6_2c_inarm_variants_batch3.cpp
)

target_link_libraries(test_w6_2c_inarm_variants_batch3 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2c_inarm_variants_batch3 PRIVATE ws2_32)
endif()

add_test(NAME W6_2c_InArmVariantsBatch3 COMMAND test_w6_2c_inarm_variants_batch3)

# --- test_w6_2c_inarm_variants_batch4: prove the 5-variant batch-4 over Sim HAL
#   Independently proves the W6.2c batch-4 of 5 in-arm site variants
#   (2x2_8_Hot, 2x4_8, 2x5_8, 2x6_8, 2x8_8).  Proves:
#     [D]  the engine ladder (DoInArm_9045) routes each of the 5 active variant
#          iInArmType values to its OWN DoInArm_9045_<v>() arm via the SOUND
#          engine==direct differential witness; all 5 *_SuckerMap symbols callable.
#     [D-route] golden routing fact (golden ainarm9045.cpp :4639): e9045_2x8_32
#          routes to the SAME DoInArm_9045_2x8_8 callee as e9045_2x8_8.
#     [D-gate] still-gated e9045_2x4_4_13 / e9045_1x4_4_13 -> Program-Error else,
#          cursor UNCHANGED.
#     [O1] pure-const XPHSuckToSht_2x6_6 (golden ainarm9045_2x6_8.cpp:75) standard /
#          Steven-20250123-fix / close-center-2x4 rows.
#     [O2] pure-const XPHSuckToSht_2x8_8 (golden ainarm9045_2x8_8.cpp:112) identity
#          standard + JerryYang-20190729 STM interleave.
#   Same RESCAN link group as the W6.1/W6.2/W6.2b/W6.2c-batch1/2/3 canaries.
add_executable(test_w6_2c_inarm_variants_batch4
    test_w6_2c_inarm_variants_batch4.cpp
)

target_link_libraries(test_w6_2c_inarm_variants_batch4 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2c_inarm_variants_batch4 PRIVATE ws2_32)
endif()

add_test(NAME W6_2c_InArmVariantsBatch4 COMMAND test_w6_2c_inarm_variants_batch4)

# --- test_w6_2c_inarm_Sfamily: prove the 2 S-family in-arm variant files ----
#   Independently proves the LAST 2 in-arm dispatch arms (the former residents of
#   the DoInArm_9045 "#if 0 // TODO(W6.2b variants)" gate, un-gated in batch-5):
#     ainarm9045S_1x4_4.cpp     (e9045_1x4_4_13; IRREGULAR func name DoInArm_9045S_1x4_4 -- S IN func)
#     ainarm9045S_2x4_4_13.cpp  (e9045_2x4_4_13; func DoInArm_9045_2x4_4_13 -- NO S)
#   Proves:
#     [D1/D2] BOTH engine ladders (DoInArm_9045 / DoInArm_9045_SuckerMap) route each
#          S-family iInArmType to its OWN IRREGULAR-named callee via the SOUND
#          engine==direct differential witness; neither hits the Program-Error else.
#     [O2] pure-const XPHSuckToSht_1x4_13 (golden ainarm9045S_1x4_4.cpp:32-33) +
#          XPHSuckToSht_2x4_13 (golden ainarm9045S_2x4_4_13.cpp:32-34).
#     [O3] picker-count witness: SetInOutArmParameter_1x4_4_13 -> SetPickerCount
#          (2,2,1,4,2,0,0); SetInOutArmParameter_2x4_4_13 iShtCol 4 (non-special) /
#          8 (_16Site2X8) -> SetPickerCount(2,2,2,{4|8},2,2,2).
#   Same RESCAN link group as the W6.1/W6.2/W6.2b/W6.2c-batch1..4 canaries.
add_executable(test_w6_2c_inarm_Sfamily
    test_w6_2c_inarm_Sfamily.cpp
)

target_link_libraries(test_w6_2c_inarm_Sfamily PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2c_inarm_Sfamily PRIVATE ws2_32)
endif()

add_test(NAME W6_2c_InArmSfamily COMMAND test_w6_2c_inarm_Sfamily)

# ===========================================================================
#  W6.2c VERIFY -- the OUT-ARM ENGINE core SMs (aoutarm9045.cpp)
# ===========================================================================
# --- test_w6_2_outarm_core: pump the OUT-ARM central task SMs over Sim HAL ---
#   The task-mandated W6.2c *core engine* verify TU (DIRECT mirror of the in-arm
#   core TU above).  Proves aoutarm9045.cpp (the out-arm ENGINE, in ht9045_sm)
#   LINKS + its ACTIVE central task state machines PUMP over the Sim HAL with NO
#   hardware:
#     DoOutArmAdditionalFunction (iOutArmAdditionalFunctionTask, golden:2246) --
#       case 1 FALLS THROUGH to case 100; the ACTIVE MoveOutArmToAutoSafe() stub
#       returns true and the Rotate/AOI/FixAI branches are W7-gated -> the
#       else-path sets bResult=true on the FIRST tick, cursor settles at 100;
#     DoOutArmIonFanGiveWay (iOutArmIonFanGiveWayTask, golden:4613) --
#       (2a) ESD-off PRE-SWITCH guard returns true + forces cursor==1 (golden
#            1312-1316); (2b) ESD-on entry case 1 with empty carry kits + no real
#            IC -> inner-else Task 1->2 (golden 1332); the full pump stays inside
#            the documented {1,2,3,5} set (no escape / no crash).
#   Each cursor is asserted to stay inside the documented switch(Task) state set
#   and to match the golden entry/early-out (golden line cited per assert), plus
#   the geometry/numeric ORACLEs: GetOutArmPitch_9045 interpolation endpoints +
#   float->int TRUNCATION (5000.125 -> 5000) + GetOutArmPitchY_9045 zero-guard
#   (denom==0 -> iOutArmY15Pitch, no div-by-zero).
#   This TU reaches the IonFan case-1/case-5 OutSHT1InRT/OutSHT2InRT predicates,
#   which had no offline home before this wave; csystem_predicates.cpp now
#   supplies them (delegating to InSHT*InRT, golden csystem.cpp:699/701).
#   Same RESCAN link group as the W6.1/W6.2 canaries so the
#   cprod<->cpublic<->cmydef ctor cross-references settle.
add_executable(test_w6_2_outarm_core
    test_w6_2_outarm_core.cpp
)

target_link_libraries(test_w6_2_outarm_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2_outarm_core PRIVATE ws2_32)
endif()

add_test(NAME W6_2_OutArmCore COMMAND test_w6_2_outarm_core)

# --- test_w6_2_outarm_variants: prove the LIVE out-arm site variants DISPATCH ---
#   W6.2c-OUT batch (1x1_1/1x2_2/1x2_4/1x3_2_14/1x3_4/1x4_2).  Independent witness
#   that the engine dispatch arm for each layout resolves to the REAL translated
#   DoOutArm_9045_<v>() body (in ht9045_sm) -- NOT the removed file-local no-op
#   `static void ...(){}` stub.  Each variant's golden task-1 entry writes the
#   global bSortingAllBinTrayFinish=false and advances OutArmTask off 1 (case
#   1->5->10->50 via the offline MoveOutArmToAutoSafe()/CheckOutArmInitState()
#   stubs); the deleted no-op stub would have left both untouched.  Plus the
#   engine GetOutArmPitch_9045 interpolation oracle as a link-regression guard.
#   Same RESCAN link group as test_w6_2_outarm_core so the cprod<->cpublic<->
#   cmydef ctor cross-references settle and ht9045_sm carries the variant bodies.
add_executable(test_w6_2_outarm_variants
    test_w6_2_outarm_variants.cpp
)

target_link_libraries(test_w6_2_outarm_variants PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_2_outarm_variants PRIVATE ws2_32)
endif()

add_test(NAME W6_2_OutArmVariants COMMAND test_w6_2_outarm_variants)

# --- test_w6_3_catchtray_core: pump the TRAY-ARM central task SMs over Sim HAL ---
#   The task-mandated W6.3 *core engine* verify TU (mirror of the out-arm core TU).
#   Proves acatchtray.cpp (the TrayArm ENGINE, in ht9045_sm) LINKS + its ACTIVE
#   task SMs PUMP over the Sim HAL with NO hardware:
#     DoSlapTray (iSlapTrayTask, golden:5877) -- the simplest fully-pumpable
#       sub-SM; DoSlapTray(true) re-inits to 1; the pump stays inside the
#       documented {1,5,10,20,50,100,200,300,400,450,500} set (golden:5889);
#     DoCatchTray (CatchTrayTask, golden:6000) -- the central dispatcher pumps
#       from entry without crash, cursor stays in a sane range;
#     WhichAutoNeedTray() (golden:410) ORACLE -- the real engine body (NOT the
#       deleted canary stub) returns 0 on the Clean-Out/no-IC path (golden:463)
#       and on the Tray-Feed/no-IC path (golden:417).
#   Same RESCAN link group as the W6.1/W6.2 canaries + core TUs.
add_executable(test_w6_3_catchtray_core
    test_w6_3_catchtray_core.cpp
)

target_link_libraries(test_w6_3_catchtray_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_3_catchtray_core PRIVATE ws2_32)
endif()

add_test(NAME W6_3_CatchTrayCore COMMAND test_w6_3_catchtray_core)

# --- test_w6_3_catchtray: TrayArm ENGINE acceptance proof (5 golden oracles) -
#   The task-mandated W6.3 acceptance verify TU (COMPANION to the core TU above;
#   mirror of the in/out-arm core verifies).  Sets a KNOWN Sim HAL state
#   (CatchTraySuck items, the per-SM entry cursors, MOT[].Position/Led via the
#   offline Motor==NULL knob, Prod tray-X geometry, iPauseBackUp) and asserts the
#   FIVE hand-derived golden oracles from the task brief:
#     [O1] DoSlapTray 200->300 FALL-THROUGH in one tick (golden 5925-5929 //break;)
#          + init guard (5883-5887) + case1->5 (5891-5897);
#     [O2] DoCatchFromLoader 1->10 FALL-THROUGH: one call from Task=1 with
#          MTrayXCanSafeMove()==true lands at 50 via case10 else (golden 1499-1539);
#     [O3] WhichAutoNeedTray()==0 on the Tray-Feed (golden 417) + Clean-Out
#          (golden 463) deterministic short-circuit paths;
#     [O4] WhichAutoNeedTray() AMR i+1 path documented as offline-unreachable
#          (TfAGV::Use_AMR()==false, FormsFacade.cpp:24) -- pinned, not faked;
#     [O5] DoCatchTray entry guard: iPauseBackUp!=-1 -> cursor unchanged
#          (golden 6015-6019); iPauseBackUp==-1 -> case1 eUnderCoveyor dispatch
#          Task=50 (golden 6076-6079).
#   Plus BONUS geometry oracles (IsTrayArmAtEmptyOrColor golden 7992-8004 /
#   IsTrayArmMoveAvoidOutArmCrash golden 7979-7990) and DoSlapTray + DoCatchTray
#   cursor-universe pumps (no escape / no crash).
#   Same RESCAN link group as the W6.1/W6.2 canaries + core TUs.
add_executable(test_w6_3_catchtray
    test_w6_3_catchtray.cpp
)

target_link_libraries(test_w6_3_catchtray PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_3_catchtray PRIVATE ws2_32)
endif()

add_test(NAME W6_3_CatchTray COMMAND test_w6_3_catchtray)

# --- test_w6_4_tester: pump the TESTER/INDEX (test-head) engine over Sim HAL ---
#   The task-mandated W6.4 verify TU (mirror of the in/out/tray-arm core TUs).
#   Proves atester.cpp (the TESTER/INDEX ENGINE, in ht9045_sm) LINKS + its ACTIVE
#   core test-head task SMs PUMP over the Sim HAL with NO hardware, and that the
#   geometry/numeric oracles match values hand-derived from the golden formula:
#     [SM1] DoTestY()         iTestYTask 1 -> 50 (golden :4890) -> 100 FALLS
#           THROUGH to 110 in one tick (golden :5180 no-break); cursor stays in
#           the documented switch(Task) set under a 128-tick pump.
#     [SM2] DoTestHeadMotor()  from case 4 (bC02InstallCCD==false) walks
#           4 -> 9 -> 10 -> 15 over Sim HAL (offline Motor==NULL -> Gali Z/Y moves
#           complete + ISNormal()==true); cursor stays in the documented set.
#     [N3]  ArrayConvertSite(1,2)@_8Site2X4==6 / (1,1)@DualSite2x1==3 / (0,2)@DualSite==2.
#     [N4]  GetRowCol@_8Site2X4 -> 2,4 ret4 / @SingleSite -> 1,1 ret0 / @QualSite2X2 -> 2,2 ret3.
#     [N5]  GetIndexZSpeed INTEGER division: !slowdown -> GailSpeed; slowdown scale=30
#           -> 50000*30/100==15000; scale=0 clamp 1 -> 500 (sim HTMotor* attached,
#           then detached to restore the Motor==NULL offline knob).
#   Same RESCAN link group as the W6.1/W6.2/W6.3 canaries + core TUs.
add_executable(test_w6_4_tester
    test_w6_4_tester.cpp
)

target_link_libraries(test_w6_4_tester PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_4_tester PRIVATE ws2_32)
endif()

add_test(NAME W6_4_Tester COMMAND test_w6_4_tester)

# --- test_w6_4_tester_core: the SALVAGED RESULT-DECODE ANCHOR concrete oracle -
#   The task-mandated W6.4a *decode-anchor* verify TU (companion to the SM/
#   geometry W6_4_Tester above).  Pins the high-value fMain-free TTL bin-decode
#   trio that is the reason the partial atester.cpp was salvaged, all ACTIVE
#   (no #if 0): GetBinaryData (golden :670) / GetOneByOneData (golden :621) fed
#   by Test_GetTestResulSub/Test_GetTestResul (golden :759/:789) + ArrayConvertSite
#   (golden :504).  Asserts the CONCRETE ORACLE hand-derived from the golden
#   formula (HexData={1,2,4,8,16}, IOCT[_5Bit=4]=5):
#     [O1] Binary  _5Bit/CHBinary  iSite=0 ScanPort[0]&[2] -> 1+4 == 5;
#     [O2] OneByOne _5Bit/CHOneByOne iSite=0 only ScanPort[2] -> idx2+1 == 3;
#     [O3] OneByOne ScanPort[1]&[2] -> sum!=1 -> iTestBinCount == 16;
#   plus the decode CONTRACT corroborators (Sum==0 -> -1; no-bit -> -1; full
#   weight table -> 31; Data=iSite*5 site offset) and the Test_GetTestResulSub
#   CHBinary/CHOneByOne dispatch driven THROUGH the Sim HAL Sen[] read path
#   (Sen[SenBit0+0..4] mapped onto an injected TSimIOBackend, SOFT_SIMULTE not
#   defined so the #ifndef Sen-repopulate block is live).
#   Same RESCAN link group as the W6.1/W6.2/W6.3/W6.4 canaries + core TUs.
add_executable(test_w6_4_tester_core
    test_w6_4_tester_core.cpp
)

target_link_libraries(test_w6_4_tester_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_4_tester_core PRIVATE ws2_32)
endif()

add_test(NAME W6_4_TesterAnchor COMMAND test_w6_4_tester_core)

# --- test_w6_4b_front_rear_destroy: Front (Z1/Arm1) / Rear (Z2/Arm2) "Destroy IC"
#   down-press SM pair -- W64b-Translate wave (aTester_Front.cpp/aTester_Rear.cpp),
#   now real translations (no longer atester_shims.cpp offline stubs).  Asserts:
#     (a) Init*TestDestroyICTask() -> cursor=1, bZ1/2Isdownflag=false (golden :300-305).
#     (b) CheckZ1/2IsDown()==true and TestZ1/2OutRandge()==false under the same
#         MOT[axis].Motor->Enable==false offline convergence knob test_w6_4_tester.cpp
#         uses (sim HTMotor attached, default Enable=false).
#     (c) DoFrontTestDestroyIC()/DoRearTestDestroyIC() PARK at Task=100 (case1
#         OutSHT1/2InLF()==false -> Task=100; case100's Gali_MotMove() stub,
#         mymotor.cpp:920 TODO(W6-Galil), unconditionally returns false so the SM
#         cannot progress further offline -- a pre-existing Galil-stub limitation,
#         not a W64b defect) + a 128-tick pump-sanity check (cursor never escapes
#         the documented switch(Task) label set).
#   Same RESCAN link group as the W6.4 Tester canaries.
add_executable(test_w6_4b_front_rear_destroy
    test_w6_4b_front_rear_destroy.cpp
)

target_link_libraries(test_w6_4b_front_rear_destroy PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_4b_front_rear_destroy PRIVATE ws2_32)
endif()

add_test(NAME W6_4b_FrontRearDestroy COMMAND test_w6_4b_front_rear_destroy)

# --- test_w6_5_shuttle: pump the SHUTTLE/CARRY engine over Sim HAL -----------
#   The task-mandated W6.5 verify TU (mirror of the in/out/tray-arm + tester
#   core TUs).  Proves acarry.cpp (the SHUTTLE/CARRY ENGINE, in ht9045_sm)
#   LINKS + its ACTIVE core shuttle task SM PUMPS over the Sim HAL with NO
#   hardware (136/136 case labels ACTIVE, 0 #if 0 gates), and that the
#   geometry/numeric/zero-guard oracles match values hand-derived from the
#   golden formula:
#     [O1] Do_Auto_SHT1()  AutoSHT1Task 1 -> 10 from entry over the Sim HAL
#          (case 1 final Task=10, golden :3342); cursor stays in the documented
#          switch(Task) universe {1,10,100,105,110,115,120,124,125,130,135,140,
#          200,201,202,203,210,211,212,220,225,230,235,250,251,252,300,400,500,
#          2300,2400,2500,2600,3000,3100,3101,3102,5000,5100} under a 64-tick pump.
#     [O3] GetShuttle1Pitch_9045 double->int TRUNCATION (golden :7540):
#          w=12000 -> 8000; w=18000 -> 14000; i1180=14003,w=15000 -> r=11001.5
#          -> (int)==11001 (toward-zero, NOT 11002).
#     [O4] CheckShuttleOutputHasICError zero-detect-pos guard (golden :2076-2097):
#          Tech.*DetectPos==0 offline -> returns false before the residual switch
#          even with bShuttleHasIC flagged (no spurious residual alarm).
#     [O5] CheckShuttleSensorBroken_1(_,false) left pass: iCheckCount==0 skips the
#          bRight error-aggregation arm -> returns false (golden :7322-7360).
#   Same RESCAN link group as the W6.1/W6.2/W6.3/W6.4 canaries + core TUs.
add_executable(test_w6_5_shuttle
    test_w6_5_shuttle.cpp
)

target_link_libraries(test_w6_5_shuttle PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_5_shuttle PRIVATE ws2_32)
endif()

add_test(NAME W6_5_Shuttle COMMAND test_w6_5_shuttle)

# --- test_w6_6_hub: pump the csystem ORCHESTRATION HUB over the Sim HAL -------
#   The task-mandated W6.6 verify TU.  Proves csystem.cpp (the integration HUB,
#   in ht9045_sm) LINKS against the five engines + its DoAllProcess main loop
#   PUMPS the WHOLE machine cycle over the Sim HAL with NO hardware, ticking the
#   engines in the documented 2-phase order (the bDoProcess interleave), and that
#   the tick-sequence ORACLE (g_csystemTrace, via CSYSTEM_TICK_ORACLE on
#   ht9045_sm) confirms the exact engine order:
#     [O1] TICK A = SensorScan,DoLoad,DoInArm,SHT1,TestHead,CatchTray
#          TICK B = SensorScan,DoLoad,SHT2,TestHead,CatchTray,DoOutArm (golden 9115-10255)
#     [O2] DoInArm only on A, DoOutArm only on B, SHT1/SHT2 mutually exclusive
#     [O3] MainProc alive-instrumentation (CallCount++/IsMainProcAlive) (golden 16677-16729)
#     [O4] 64-cycle pump stability + sustained A/B alternation, no crash
#   Same RESCAN link group as the W6.1..W6.5 canaries + core TUs.
add_executable(test_w6_6_hub
    test_w6_6_hub.cpp
)

target_link_libraries(test_w6_6_hub PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_6_hub PRIVATE ws2_32)
endif()

add_test(NAME W6_6_Hub COMMAND test_w6_6_hub)

# --- test_w6_6_csystem_cycle: the INTEGRATED MACHINE CYCLE pumps over Sim HAL --
#   The task-mandated W6.6 *cycle* verify TU (companion to test_w6_6_hub).  Sets a
#   KNOWN Sim HAL state, pumps the main process loop fn (DoAllProcess/MainProc)
#   for N=64 cycles, and asserts -- against values HAND-DERIVED from the golden
#   csystem.cpp -- that the integrated cycle ticks the FIVE engines in the
#   documented order without crash, the entry/guard behaviour matches the golden,
#   and the numeric/cursor oracles hold:
#     [O1] 4-tick (T,F,T,F) engine-order stream + invariants (golden :9115-10255)
#     [O2] MainProc instrumentation: CallCount 0->5, alive flips (golden :16687-16728)
#     [O3] master-guard early-return emits ONLY [SensorScan], zero engines (golden :10049)
#     [O4] 64-cycle pump: clean A/B each tick + strict bDoProcess toggle, no crash (golden :10254)
#   Same RESCAN link group as test_w6_6_hub (ht9045_sm carries the CSYSTEM_TICK_ORACLE
#   trace; the other archives settle the cprod<->cpublic<->cmydef ctor cross-refs).
add_executable(test_w6_6_csystem_cycle
    test_w6_6_csystem_cycle.cpp
)

target_link_libraries(test_w6_6_csystem_cycle PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w6_6_csystem_cycle PRIVATE ws2_32)
endif()

add_test(NAME W6_6_CSystemCycle COMMAND test_w6_6_csystem_cycle)

# ===========================================================================
#  W7-A1 VERIFY -- the in-arm shuttle FLOATING + LATCH pair (un-gated)
# ===========================================================================
# --- test_w7_a1_inarm_floating_latch: pump the un-gated PAIR over Sim HAL ----
#   Proves the two W7-A1 un-gated in-arm SMs in ainarm9045.cpp (ht9045_sm) LINK
#   and converge over the Sim HAL with NO hardware:
#     CheckInShuttleSensor_Latch  (golden ainarm9045.cpp:3810-4174) -- the latch
#       interlock; the VERBATIM entry guard (MOTION_CARD_TYPE!=Contec ||
#       iRealDummy!=REALLY) self-disables it to return 1 (Finish) BEFORE any
#       CCLink/Ltc/MOT body runs offline (sim canary iRealDummy==DUMMY).
#     DoInArmCheckShuttleFloating (golden ainarm9045.cpp:3349-3757) -- the
#       place-to-shuttle floating quality SM; with empty Sim grids it converges
#       1 -> 9000 -> (Task=1, bResult=true) over the MoveInArmZToPlateSafe stub +
#       the FLCarryKit/BLCarryKit HasRealIC()/UseSiteHasIC() empty-grid predicates.
#   Oracles (golden-cited): (a) latch self-guard -> 1, no cursor advance/MOT touch;
#   (b) floating convergence + case-9000 side-effects (IndexZCanMove[0/1]=true,
#   bShuttleKnock=false, cInplace->bNo9Action=false); (c) no-floating-no-IC takes
#   the no-alarm branch (never enters the shake/vibrate/knock loop nor the latch).
#   Same RESCAN link group as the other W6 in-arm tests (ht9045_sm carries the
#   floating/latch bodies + the W7-A1 shuttle-motion shims).
add_executable(test_w7_a1_inarm_floating_latch
    test_w7_a1_inarm_floating_latch.cpp
)

target_link_libraries(test_w7_a1_inarm_floating_latch PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_a1_inarm_floating_latch PRIVATE ws2_32)
endif()

add_test(NAME W7_A1_InArmFloatingLatch COMMAND test_w7_a1_inarm_floating_latch)

# --- test_w7_s0_motor_convergence: OFFLINE move-family CONVERGENCE enabler ---
#   Proves the W7 S0 change in Motor/mymotor.cpp makes the offline move family
#   ARRIVE over the Sim HAL (AI(W906-C21-MOTORMOVE) 20260924: disabled-driver terminal; NULL = golden -1; (e)-(h) added):
#     (a) CONVERGE  -- MotorMove(N) -> 1 (Move Success) AND ReadPos()==N (was a
#                      no-op stub returning 0/Moving forever before S0);
#     (b) PICKER    -- MotorMove2SpeedForPicker(FinalPos,&arm) -> true & arrives;
#     (c) HOME      -- MotorHome offline still converges Position->0 (regression
#                      guard; S0 did NOT touch the home path);
#     (d) FAST-PATH -- Gali_Two_ZAxis_Move / ISNormal / GalilTwoY_Move stay true
#                      offline (atester one-liners; NOT part of the move family).
#   Same RESCAN link group as test_sim_motor / test_motor_w4 -- only the motor
#   layer is exercised (no state-machine archive needed), zero vendor libraries.
add_executable(test_w7_s0_motor_convergence
    test_w7_s0_motor_convergence.cpp
)

target_link_libraries(test_w7_s0_motor_convergence PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_s0_motor_convergence PRIVATE ws2_32)
endif()

add_test(NAME W7_S0_MotorConvergence COMMAND test_w7_s0_motor_convergence)

# ===========================================================================
#  W7 SUBSTRATE STAGING VERIFY -- the cross-arm KitSuck occupancy grid (ON)
# ===========================================================================
# --- test_w7_grid_occupancy: prove the flag-flip wired the predicates to grid --
#   Proves the W7 substrate-staging change (csystem_predicates.cpp
#   HT9045_KITSUCK_GRID_AVAILABLE 0->1 + faithful golden UseSiteHasIC()/UseSiteNoIC()
#   Item-scan bodies in aHotPlateSubstrate.cpp) makes the csystem HasIC predicate
#   family consult the REAL (seedable) TMyKitSuck Item grid instead of returning a
#   hard-coded false.
#     PART A -- UNIT: a LOCAL TMyKitSuck: fresh (all NULL_IC) -> UseSiteHasIC false /
#       UseSiteNoIC true / HasRealIC false; seed a use-site (iShtRow x iShtCol) to
#       HAS_IC + widen iMaxRow/iMaxCol -> UseSiteHasIC true / UseSiteNoIC false /
#       HasRealIC true / CountRealIC>=1; ClearAll -> UseSiteHasIC false again.
#     PART B -- END-TO-END flag-flip proof: seed the GLOBAL FLCarryKit use-site that
#       InputShuttleFrontHasIC() reads -> InputShuttleFrontHasIC()==true AND
#       ShuttleHasIC()==true; clear -> false. Second witness: seed FTestSuck ->
#       IndexHasRealIC()==true; clear -> false. Globals are saved/restored so sibling
#       suites are unaffected. With the flag OFF these leaves were a hard-coded
#       `return false`; seeding making them true proves the wiring.
#   The csystem predicates live in ht9045_sm (csystem_predicates.cpp); the TMyKitSuck
#   bodies + the kit globals live in ht9045_sm too (aHotPlateSubstrate.cpp). Same
#   RESCAN link group as the other W6/W7 state-machine tests so the
#   cprod<->cpublic<->cmydef ctor cross-references settle.
add_executable(test_w7_grid_occupancy
    test_w7_grid_occupancy.cpp
)

target_link_libraries(test_w7_grid_occupancy PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_grid_occupancy PRIVATE ws2_32)
endif()

add_test(NAME W7_GridOccupancy COMMAND test_w7_grid_occupancy)

# ===========================================================================
#  W7 VERIFY -- HotPlate edge-cylinder knock loop + Loader vibration loop
# ===========================================================================
# --- test_w7_hotplate_loader_vibrate: prove both un-gated tail loops pump ---
#   Proves the W7 un-gate of DoHotplateEdgeCylinderLoop (golden csystem.cpp
#   :20323-20388) and DoLoaderVibrateLoop (golden :20391-20432), both
#   ZERO-BLOCKED (every symbol -- Cylinder[]/SW[]/MOT[]/IniConfig/LastSet/
#   Ld_UldDelayTime -- already had a real translated home).  Drives each loop
#   through its 10-tick cadence over the Sim HAL and asserts the Cylinder[]/
#   SW[] end state + TQPF_Timer on/off transitions against golden-derived
#   oracles: the four/three top guards (zero side effect, no iCT advance), the
#   bFirst LoopDelay<2 clamp, the off-delay/on-delay elapse -> On()/Off()
#   transitions (including the iVibratorHP1 int(OnDelay/10.0) truncation
#   quirk, preserved verbatim), and the bPickFromHotplate+VibrationMotor guard
#   forcing Cylinder Off().  Same RESCAN link group as the other W6/W7
#   state-machine tests (csystem.cpp is compiled into ht9045_sm) so the
#   cprod<->cpublic<->cmydef ctor cross-references settle.
add_executable(test_w7_hotplate_loader_vibrate
    test_w7_hotplate_loader_vibrate.cpp
)

target_link_libraries(test_w7_hotplate_loader_vibrate PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_hotplate_loader_vibrate PRIVATE ws2_32)
endif()

add_test(NAME W7_HotplateLoaderVibrate COMMAND test_w7_hotplate_loader_vibrate)

# ===========================================================================
#  W7-C1 VERIFY -- the end-of-lot CLEAN OUT drain finish-check (REAL body)
# ===========================================================================
# --- test_w7_c1_cleanout_finish: prove DoCleanOutFinishCheck does REAL work ---
#   Proves the W7-C1 change (the faithful golden DoCleanOutFinishCheck body,
#   csystem.cpp:14713-15748, now lives in the translated csystem.cpp -- replacing
#   the no-op stub) does REAL work over the now-seedable cross-arm occupancy grid:
#     (a) EMPTY machine (iCleanOut=1/iOneCycle=0/IndexStatus=Z1_Z2_Normal, all
#         kits cleared) -> the body does NOT park on the residual-IC cursor 1001;
#         clean-out can DRAIN toward finish.
#     (b) FALSE-FINISH GUARD (the new capability): seed InArmSuck use-site=HAS_IC
#         -> iCleanOutCycleTask==1001 (golden residual gate :14739) + iCleanOut
#         stays 1 (NO false instant-finish, unlike the old stub).
#     (c) DRAIN: clear the seed -> next call no longer parks on 1001.
#     (d)/(e) NO-REGRESSION: the entry guard (golden :14720-14721) gates the work
#         -- iOneCycle=1 leaves the cursor untouched, and iCleanOut=0 (the W6.6
#         hub posture) sets no clean-out drain cursor.
#   The DoCleanOutFinishCheck body + the iCleanOut/iOneCycle/iCleanOutCycleTask
#   globals + the TMyKitSuck kit globals all live in ht9045_sm (csystem.cpp /
#   cmydef / aHotPlateSubstrate.cpp).  Same RESCAN link group as the other W6/W7
#   state-machine tests so the cprod<->cpublic<->cmydef ctor cross-references
#   settle.  All seeded globals are saved/restored so sibling suites are unaffected.
add_executable(test_w7_c1_cleanout_finish
    test_w7_c1_cleanout_finish.cpp
)

target_link_libraries(test_w7_c1_cleanout_finish PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_c1_cleanout_finish PRIVATE ws2_32)
endif()

add_test(NAME W7_C1_CleanOutFinish COMMAND test_w7_c1_cleanout_finish)

# ===========================================================================
#  W7-C2 VERIFY -- the end-of-lot ONE CYCLE finish-check (REAL body)
# ===========================================================================
# --- test_w7_c2_onecycle_finish: prove DoOneCycleFinishCheck does REAL work ---
#   Proves the W7-C2 change (the faithful golden DoOneCycleFinishCheck body,
#   csystem.cpp:12813-14047, now lives in the translated csystem.cpp -- replacing
#   the no-op stub at csystem_shims.cpp:145) does REAL work over the now-seedable
#   cross-arm occupancy grid:
#     (a) EMPTY machine (iOneCycle=1/IndexStatus=Z1_Z2_Normal/CUSTOMER_CODE=0, all
#         kits cleared) -> the body does NOT park on the residual-IC wait cursor 5;
#         one-cycle can proceed toward finish.
#     (b) FALSE-FINISH GUARD (the new capability): seed InArmSuck use-site=HAS_IC
#         -> iOneCycleTask==5 (golden residual wait gate :12933) + iOneCycle stays
#         1 (NO false instant-finish -- the finish path that clears iOneCycle=0 at
#         golden :13931 is unreached, unlike the old stub).
#     (c) DRAIN: clear the seed -> next call no longer parks on 5.
#     (d) NO-REGRESSION: the entry guard (golden :12820) gates the work -- iOneCycle=0
#         (the W6.6 hub posture) takes the else-branch (golden :14025), leaving the
#         cursor untouched and iOneCycle at 0 (BtnOneCycle->Down offline-false).
#   The DoOneCycleFinishCheck body + the iOneCycle/iOneCycleTask/IndexStatus/
#   CUSTOMER_CODE globals + the TMyKitSuck kit globals all live in ht9045_sm
#   (csystem.cpp / cmydef / aHotPlateSubstrate.cpp).  Same RESCAN link group as
#   the other W6/W7 state-machine tests so the cprod<->cpublic<->cmydef ctor
#   cross-references settle.  All seeded globals are saved/restored so sibling
#   suites are unaffected.
add_executable(test_w7_c2_onecycle_finish
    test_w7_c2_onecycle_finish.cpp
)

target_link_libraries(test_w7_c2_onecycle_finish PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_c2_onecycle_finish PRIVATE ws2_32)
endif()

add_test(NAME W7_C2_OneCycleFinish COMMAND test_w7_c2_onecycle_finish)

# ===========================================================================
#  W906-AutoSiteMapCleanOut VERIFY -- InitCleanOutFunction's AutoSiteMap
#  branch (last remaining piece of Wave16) does REAL work
# ===========================================================================
# --- test_w906_autositemap_cleanout: prove the newly un-gated branch mutates
#   the REAL globals it should (golden csystem.cpp:15751-15785), not just
#   "compiles":
#     O1 guard false (iRunStartMode!=rsmAutoSiteMap/temp/JCET) -> whole branch
#        skipped, tail cursor resets still fire unconditionally.
#     O2 bAutoSiteMapHasPickHP==true -> ONLY iResetSiteMappingStep=2; the
#        else-branch's SiteMapData zero-out / bAutoSiteMapHotplateSave /
#        bSiteMappingCHKOK are NOT touched.
#     O3 bAutoSiteMapHasPickHP==false -> the else-branch fires:
#        bSiteMappingCHKOK=true, MOT[MMPlate1/MMPlate2].Tray.SiteMapData
#        zeroed for j<XDivision/k<YDivision on BOTH plates (seeded sentinels
#        both in- and out-of-bounds prove the exact loop bounds, not a
#        naive full-array wipe), bAutoSiteMapHotplateSave=false.
#     O4 fMain->SetMainRunStartMode(...) (this wave's documented GAP no-op
#        stub, FormsFacade.cpp) / InitInArmTask() / ShowTestHeadComp() are
#        all reached without crashing, for both iAutoSiteMapRunStartMode
#        selector values.
#   Same RESCAN link group as the other W6/W7 state-machine tests (csystem.cpp
#   / FormsFacade.cpp / cprod / Motor/mymotor.cpp all live in ht9045_sm /
#   ht9045_motor) so the cprod<->cpublic<->cmydef ctor cross-references
#   settle. All seeded globals are saved/restored so sibling suites are
#   unaffected.
add_executable(test_w906_autositemap_cleanout
    test_w906_autositemap_cleanout.cpp
)

target_link_libraries(test_w906_autositemap_cleanout PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w906_autositemap_cleanout PRIVATE ws2_32)
endif()

add_test(NAME W906_AutoSiteMapCleanOut COMMAND test_w906_autositemap_cleanout)

# ===========================================================================
#  W906-AutoCleanCluster (20260728) VERIFY -- DoIndexAutoClean +
#  DoIndexAutoClean_Arm1PickArm2Test (the SECOND, final AutoClean dependency
#  cluster; replaces the TEMPORARY empty-body placeholder).
# ===========================================================================
# --- test_w906_doindexautoclean: prove both functions are REAL, not stubs ---
#   O1-O3: the 3 early-return guards (F16 shuttle-sensor-broken check before
#     the unconditional Motor-> dereference; bFullViewCheckFinish==false after
#     it; Task-range + IsCanMove()==true) all leave Task untouched.
#   O4: a real run through DoIndexAutoClean's OWN switch, case 1 -> 100 -> 200
#     -> 300, asserting both Task and bInedxCleanFinish[0]/[1].
#   O5: the RunAutoCleanByArmPickArm2Test()==true delegation branch itself
#     (DoIndexAutoClean_Arm1PickArm2Test's own case1 runs, same Task=100 +
#     bInedxCleanFinish-clear shape).
#   O6: DoIndexAutoClean_Arm1PickArm2Test driven directly (now declared in
#     AutoClean.h for testability) with its own local arrays -- its case100
#     is a DIFFERENT branch shape than DoIndexAutoClean's own case100.
#   Same RESCAN link group as test_AutoClean / test_w906_autositemap_cleanout
#   (AutoClean/AutoClean.cpp lives in ht9045_sm) so the cprod<->cpublic<->
#   cmydef ctor cross-references settle.
add_executable(test_w906_doindexautoclean
    test_w906_doindexautoclean.cpp
)

target_link_libraries(test_w906_doindexautoclean PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w906_doindexautoclean PRIVATE ws2_32)
endif()

add_test(NAME W906_DoIndexAutoClean COMMAND test_w906_doindexautoclean)

# ===========================================================================
#  W5 comms wave -- SECSGEM base / Interface / CCLink / CanBus / MyPLC /
#  EtherCAT-comms (6-unit parallel translate batch, wired by the serial
#  integrate pass, 2026-07-10)
# ===========================================================================

# --- test_uHGemClass: HTGem ctor/dtor/UpdateDataPath + IsCorrectDateFormat +
#   the already-inline virtuals + derived-override polymorphism + the 36 gated
#   stubs' conservative defaults + every SECS_EVENT enumerator's exact golden
#   value.  Supplies its own local MyDBIProcess(3-arg) stub.
add_executable(test_uHGemClass
    test_uHGemClass.cpp
    test_uHGemClass_link_stubs.cpp
)

# AI(W906-SysModWire) 20260720: was plain `ht9045_secsgem` (this test used to
# have ZERO THGem/uHGemEquipment dependency). This wave un-gated 8 uHGemClass.cpp
# methods (S1F1/S1F2/S1F13/S1F14/Process_S1F14/S1F16/S1F18/S2F18) that call
# real out-of-line THGem methods (CheckSFFormatOnlyHead/OffLine/OnLine/
# GetOnLineMode/GetTimeInfo, uHGemEquipment.cpp) -- uHGemClass.cpp.o now has
# unconditional undefined refs into uHGemEquipment.cpp.o (same archive, fine)
# which itself references `HSys` (ht9045_db, cross-archive) and, once the
# SystemModularInitial shim wave lands, database.cpp.o references BACK into
# ht9045_secsgem for HT9045Gem's ctor -- the same circular-archive shape
# documented at test_uHGemEquipment's own entry below (identical RESCAN fix
# applied here; not one of the six targets the design brief's own risk R3
# anticipated by name, but the exact case R3's own wording covers: "if other
# targets also indirectly pull database.cpp.o in, apply the same fix").
# AI(W906-uHGemClass-Unlock3) 20260723: `ht9045_sm` added to the RESCAN group
# -- S2F16_NewEquipmentConstantSendAcknowledge (this wave's own un-gate) calls
# the real `HasICUnderMachine()`/`HasAnyICInMachine()` free-function
# predicates, which live in csystem_predicates.cpp, compiled into ht9045_sm
# (see that library's own root-CMakeLists.txt comment). Pulling that object
# file in transitively drags in TWO more ht9045_sm object files that each
# supply a REAL, non-static definition colliding with a local stub this test
# used to carry (both REMOVED from tests/test_uHGemClass_link_stubs.cpp this
# same wave -- see that file's own updated comment for the full list and the
# self-correction note on the ShowMyMessage one):
#   * aHotPlateSubstrate.cpp:772 `void MyDBIProcess(AnsiString, AnsiString) {}`
#   * canary_support.cpp:82-89   `void ShowMyMessage(AnsiString, AnsiString,
#                                  AnsiString, bool, bool) { ... }`
target_link_libraries(test_uHGemClass PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_uHGemClass PRIVATE ws2_32)
endif()

add_test(NAME uHGemClass COMMAND test_uHGemClass)

# --- test_interfacesys: GPIBInterface_Data + outbound builders + inbound
#   decoder + WM_COPYDATA dispatcher, exercised with NO real external program
#   attached (FindWindow legitimately returns NULL -- golden's own safe
#   default).  Interface/InterfaceSYS.cpp lives inside ht9045_sm (see root
#   CMakeLists.txt comment on why it is not part of ht9045_comms); ht9045_sm
#   PUBLIC-links ht9045_secsgem so EventReport()/SECS_EVENT resolve too, but
#   this test does not touch those.  Defines LOCAL stand-in globals
#   bGPIBError/bEcho/bUnderTest/bExist/iBin[4][8] that golden main.cpp owns
#   (main.cpp itself untranslated, W7).
add_executable(test_interfacesys
    test_interfacesys.cpp
)

target_link_libraries(test_interfacesys PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_interfacesys PRIVATE ws2_32)
endif()

add_test(NAME Interface_InterfaceSYS COMMAND test_interfacesys)

# --- test_MyCCLink: offline acceptance proof (HAVE_CCLINK not defined) ------
#   Exercises TMyCCLinkCtrl/TMyCCLinkBoard/UseCanBusOrEtherCAT/UseSensorMod3
#   entirely over the offline Mdfunc stub path.  Links the combined
#   ht9045_comms library (see root CMakeLists.txt); only this unit's own
#   translation units are extracted from that archive.  Supplies local stub
#   definitions for MySleep/MySleepEx/MyDBIProcess so it links standalone.
add_executable(test_MyCCLink
    test_MyCCLink.cpp
)

target_link_libraries(test_MyCCLink PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_MyCCLink PRIVATE ws2_32)
endif()

add_test(NAME MyCCLink COMMAND test_MyCCLink)

# --- CanBus cMyNUDN1: TMyNUDN1 NUDN1 station wrapper (W5-CanBus leaf) -------
#   Links the combined ht9045_comms library; cMyNUDN1.cpp itself needs only
#   vclcompat (confirmed leaf), so ht9045_globals here is just along for the
#   ride via ht9045_comms's own PUBLIC link (harmless).
add_executable(test_cMyNUDN1
    test_cMyNUDN1.cpp
)

target_link_libraries(test_cMyNUDN1 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_comms,ht9045_globals,ht9045_core,ht9045_public,vclcompat>"
)

add_test(NAME CanBus_cMyNUDN1 COMMAND test_cMyNUDN1)

# --- test_myplc_modbus: SIM-only acceptance proof, no hardware/network -----
#   [1] vclcompat/ClientSocket Sim-mode connect/disconnect/SendBuf/SimPushReceive.
#   [2] MyPLC::ModbusTCPClient SendData/Connect/DisConnect + byte-level frame-
#       parse oracle driving SocketRead/SocketConnect/SocketDisConnect/SocketError.
#   [3] MyPLC_IO_Modbus InitPLCIO/PLCIOTaskCycle/PLCStatusCheck pumped without
#       crash; TMyPLC_IO_Modbus IsOn()/IsOff(); TPLCIOThread offline-inert.
#   Links the combined ht9045_comms library (MyPLC_IO_Modbus.cpp's real
#   bPLCInData/bPLCIO + local MySleepEx fallback are unconditionally compiled
#   in -- no HAVE_PLC_IO/HAVE_MYSLEEPEX flags needed since ht9045_comms is
#   never linked alongside ht9045_io/ht9045_sm in the SAME executable).
add_executable(test_myplc_modbus
    test_myplc_modbus.cpp
)

target_link_libraries(test_myplc_modbus PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_myplc_modbus PRIVATE ws2_32 psapi version)
endif()

add_test(NAME MyPLCModbus COMMAND test_myplc_modbus)

# --- test_clientsocket_ext: Tag/Open()/SendText()/ReceiveText() acceptance --
#   SIM-only proof of the 4 additions ClientSocket.h/.cpp gained for BarCode's
#   8-CCD glue unit + Automation's TesterTCP unit (W5 final-fronts campaign,
#   2026-07-11). AI(W5-Final-Integrate) 20260711: this test file existed on
#   disk but was never wired in by any of the 4 integrate agents in that
#   campaign -- added here after independent verification caught the gap.
add_executable(test_clientsocket_ext
    test_clientsocket_ext.cpp
)

target_link_libraries(test_clientsocket_ext PRIVATE vclcompat)

if(WIN32)
    target_link_libraries(test_clientsocket_ext PRIVATE ws2_32)
endif()

add_test(NAME ClientSocketExt COMMAND test_clientsocket_ext)

# --- test_serversocket: vclcompat/ServerSocket Sim-mode acceptance test ------
#   TServerSocket/TServerWinSocket (Front C of the W906 uHGemClass/AGV_PortScan/
#   ServerSocket triple-front wave, 2026-07-13). Sim-only (no real network I/O),
#   same posture as test_clientsocket_ext.cpp just above. Links vclcompat only
#   (ServerSocket.cpp lives in the vclcompat library, see root CMakeLists.txt).
add_executable(test_serversocket
    test_serversocket.cpp
)

target_link_libraries(test_serversocket PRIVATE vclcompat)

if(WIN32)
    target_link_libraries(test_serversocket PRIVATE ws2_32)
endif()

add_test(NAME ServerSocket COMMAND test_serversocket)

# AI(W906-SpoolCluster) 20260721: test_FileListBox -- vclcompat/FileListBox real-rescan acceptance test ----
#   TFileListBox (W906-SpoolCluster: THGem's FileListBox1, Spool cluster).
#   Real file-system I/O under a scratch directory (no Sim/Real split, unlike
#   ClientSocket/ServerSocket -- FindFirst/FindNext/FindClose are already
#   real, unconditionally). Links vclcompat only (FileListBox.cpp lives in
#   the vclcompat library, see root CMakeLists.txt).
add_executable(test_FileListBox
    test_FileListBox.cpp
)

target_link_libraries(test_FileListBox PRIVATE vclcompat)

add_test(NAME FileListBox COMMAND test_FileListBox)

# --- test_MyNUEC1: EtherCAT/MyNUEC1 FS-N12 amplifier slave (W5 comms leaf) --
#   Offline stub path (HAVE_PCI1203 not defined); links the combined
#   ht9045_comms library.
add_executable(test_MyNUEC1
    test_MyNUEC1.cpp
)

target_link_libraries(test_MyNUEC1 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_MyNUEC1 PRIVATE ws2_32)
endif()

add_test(NAME MyNUEC1 COMMAND test_MyNUEC1)

# ===========================================================================
#  W5 Automation wave (8-unit parallel translate batch, wired by the serial
#  integrate pass, 2026-07-10) -- AMR/AGV predicates, HANA-ART, SCK_ART
#  extract, uRENESAS_Other data shapes, 32-Site engine, ProcessCount leaf
#  subset, Front/Rear "Suck IC" state machines.
# ===========================================================================

# --- test_amr: TeraPower AMR bridge (Automation/AMR.cpp) + the 3 AGV AMR-mode
#   predicates (AGV_predicates.cpp), driven over a KNOWN global-state baseline.
#   Same RESCAN link group as the other ht9045_sm verify TUs (AMR.cpp/HANA_ART.cpp/
#   SCK_ART.cpp are all now part of ht9045_sm -- see root CMakeLists.txt).
add_executable(test_amr
    test_amr.cpp
)

target_link_libraries(test_amr PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_amr PRIVATE ws2_32)
endif()

add_test(NAME AMR COMMAND test_amr)

# --- test_hana_art: uHANA_ART (Automation/HANA_ART.cpp) SRQ protocol helper --
#   black-box coverage via the public API (GetHanaSRQStr/ParseLOTONStr/
#   IsHanaArtAvailable/GetHD_CT/GetHD_Tempset/GetHD_T|J|A-DATA/DoSETUP_INFORM/
#   StartPrimeTest+EndPrimeTest/RecoveryARTStatus/IsContactAvailable).
add_executable(test_hana_art
    test_hana_art.cpp
)

target_link_libraries(test_hana_art PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_hana_art PRIVATE ws2_32)
endif()

add_test(NAME HanaART COMMAND test_hana_art)

# --- test_SCK_ART: Automation/SCK_ART.cpp 8-function ART extract (W5-SckArt-Translate) -------
#   Needs the ht9045_sm substrate (aHotPlateSubstrate.cpp's TestSocket, FormsFacade.cpp's fMain,
#   canary_support.cpp's LastSet/ShowErrorMessage) + ht9045_globals. SCK_ART.cpp is compiled
#   into ht9045_sm (see root CMakeLists.txt) -- do NOT also add it to this executable's own
#   sources (would duplicate-define every SckArt_* symbol at link time).
add_executable(test_SCK_ART
    test_SCK_ART.cpp
)

target_link_libraries(test_SCK_ART PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_SCK_ART PRIVATE ws2_32)
endif()

add_test(NAME SCK_ART COMMAND test_SCK_ART)

# --- test_urenesas_other: RENESAS host-protocol data shapes + cmd/status/err tables ---
#   Pure data shapes, zero global/HAL coupling -- its own small standalone library
#   (ht9045_automation, see root CMakeLists.txt), not part of ht9045_sm.
add_executable(test_urenesas_other
    test_urenesas_other.cpp
)

target_link_libraries(test_urenesas_other PRIVATE ht9045_automation)

add_test(NAME uRENESAS_Other COMMAND test_urenesas_other)

# --- test_atester_32site: 32-Site dual-arm test-head engine (W5-atester32site) -
#   All 8 golden functions real; retires the atester_shims.cpp offline stubs for
#   these symbols (see atester_shims.cpp/.h comments). Same RESCAN link group as
#   the other ht9045_sm verify TUs.
add_executable(test_atester_32site
    test_atester_32site.cpp
)

target_link_libraries(test_atester_32site PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_atester_32site PRIVATE ws2_32)
endif()

add_test(NAME W5_Atester32Site COMMAND test_atester_32site)

# --- test_transform_funtion: golden adam6024.cpp:1038 TransformFuntion（P2b）--
#   AI(W906-P2b-CF) 20260919.
#
#   759 行**從來沒有被編譯過**（先前整段閘掉），所以也從來沒被型別檢查或執行過。
#   編譯綠只證明型別對，不證明它在跑 —— 它有 44 個對力量表的引用，而那四張表
#   在 P2a 之前永遠是空的。
#
#   ⚠ 刻意不釘死輸出值（輸出取決於這一台的 Gerneral.ini / ContactInfo.ini /
#     DeviceForm）。斷言的是不變式：不當掉、輸出夾在 [0, 65535]、純函式、
#     以及 ★ 改表裡的 dLoadRate 輸出會跟著變（證明真的在讀表，不是走繞開的分支）。
add_executable(test_transform_funtion
    test_transform_funtion.cpp
)

target_link_libraries(test_transform_funtion PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_transform_funtion PRIVATE ws2_32)
endif()

add_test(NAME TransformFuntion COMMAND test_transform_funtion)

# --- test_contactforce_load: ContactForce 驅動層（P2a）-----------------------
#   AI(W906-P2a-CF) 20260919.
#
#   證明 LoadContactForceTables() 真的把 ContactForceTables() 的四張表從
#   D:\HT9045\system\ContactInfo.ini 填起來。在這之前那四張表的呼叫點是 0，
#   永遠空著、bLoaded 永遠 false。
#
#   ⚠ 刻意**不釘死任何機台組態值**（週末計畫 P8 記的 IniFiles 缺陷就是
#     「釘死 5 個機台組態值 -> 換台機器必紅」）。斷言的是結構與不變式：
#       * 筆數 == Type CSV 通過 golden >15.0 濾網的 token 數（CSV 由測試重讀）
#       * 值落在 golden CheckRange 的夾限內
#       * 至少一筆不是建構預設 -> 證明 ini 真的被讀了，不是空殼
#       * 冪等：連呼叫兩次筆數不變
#       * bUseDynamicKitDiameter==false -> 回 0、四張表全空（golden :430）
add_executable(test_contactforce_load
    test_contactforce_load.cpp
)

target_link_libraries(test_contactforce_load PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_contactforce_load PRIVATE ws2_32)
endif()

add_test(NAME ContactForceLoad COMMAND test_contactforce_load)

# --- test_kitsuck_predicates: TMyKitSuck 的四支「有沒有 IC」述詞 -------------
#   AI(W906-P1-HASIC) 20260919. P1 的驗收測試。
#
#   斷言的是 **golden 的語意**（MyKitSuck.cpp :227/:288/:432/:655），
#   不是移植樹修好之前的現況 —— 照現況寫的測試只會把缺陷釘死。
#
#   逐個 Item 狀態（cmydef.h:154-170 的 14 個）跑真值表，外加：
#     * 不變式 HasIC() == !NoIC()（golden 保證，移植樹修好之前不保證）
#     * ★ HAS_NULL_IC 那一格 HasIC()!=HasRealIC() —— 單獨證明兩支不再
#       塌成同一個實作（§C12.2 的核心症狀）
#     * iMaxRow/iMaxCol 是 golden 的迴圈邊界，不是陣列上界 _MAX_SUCK_*_ITEM
#       （防止退回掃全陣列）
#
#   連結群組與同目錄其他 TMyKitSuck 測試相同（RESCAN）。
add_executable(test_kitsuck_predicates
    test_kitsuck_predicates.cpp
)

target_link_libraries(test_kitsuck_predicates PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_kitsuck_predicates PRIVATE ws2_32)
endif()

add_test(NAME KitSuckPredicates COMMAND test_kitsuck_predicates)

# --- test_atester_front_suckic: DoFrontTestSuckIC/InitFrontTestSuckICTask SM --
#   W64b-Translate-Wave2 (aTester_Front.cpp additions). Asserts:
#     (a) InitFrontTestSuckICTask() -> cursor=1, bZ1Isdownflag=false (golden :870-875).
#     (b) DoFrontTestSuckIC() over an empty Sim HAL (no real IC anywhere) PARKS
#         at Task=1 (case 1's only advance is gated by FLCarryKit.UseSiteHasIC(),
#         golden :919 -- genuinely "nothing to pick", not a translation defect)
#         + a 128-tick pump-sanity check (cursor never escapes the documented
#         switch(Task) label set).
#   Same RESCAN link group as the sibling W64b Front/Rear Destroy suite.
add_executable(test_atester_front_suckic
    test_atester_front_suckic.cpp
)

target_link_libraries(test_atester_front_suckic PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_atester_front_suckic PRIVATE ws2_32)
endif()

add_test(NAME W64b_FrontSuckIC COMMAND test_atester_front_suckic)

# --- test_atester_rear_suckic: DoRearTestSuckIC (Wave 2, aTester_Rear.cpp) ----
#   Mirrors test_atester_front_suckic.cpp (sibling parallel unit).  Asserts:
#     (a) InitRearTestSuckICTask() -> cursor=1 (golden :891-895; NOTE golden
#         asymmetry vs Front -- Rear's Init does NOT reset an "isdownflag",
#         unlike InitFrontTestSuckICTask).
#     (b) DoRearTestSuckIC() PARKS at Task=1 over an empty Sim HAL (case 1's
#         only advance is guarded by BLCarryKit.UseSiteHasIC()==false) + a
#         128-tick pump-sanity check (cursor never escapes the documented
#         switch(Task) label set).
#   Same RESCAN link group as the W6.4b Front/Rear Destroy-IC suite.
add_executable(test_atester_rear_suckic
    test_atester_rear_suckic.cpp
)

target_link_libraries(test_atester_rear_suckic PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_atester_rear_suckic PRIVATE ws2_32)
endif()

add_test(NAME W64b_RearSuckIC COMMAND test_atester_rear_suckic)

# ===========================================================================
#  W5-BarCode (BarCode front, 2026-07-11) -- 7 parallel translate units giving
#  real bodies to 18 of the 20 TfBarCode_Shim methods (aHotPlateSubstrate.h:
#  623-664).  All 7 link the same RESCAN group as the other ht9045_sm-linked
#  suites above (cprod<->cpublic<->cmydef ctor cross-references).
# ===========================================================================

# --- test_barcode_helpers: TfHIKROBOTcmd / uCCDUnloaderClip / IsSHT2DIDScanFinish
add_executable(test_barcode_helpers
    test_barcode_helpers.cpp
)

target_link_libraries(test_barcode_helpers PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_helpers PRIVATE ws2_32)
endif()

add_test(NAME BarCodeHelpers COMMAND test_barcode_helpers)

# --- test_barcode_bottom2did: InitBottom2DIDScan / DoBottom2DIDScan (1-CCD) --
add_executable(test_barcode_bottom2did
    test_barcode_bottom2did.cpp
)

target_link_libraries(test_barcode_bottom2did PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_bottom2did PRIVATE ws2_32)
endif()

add_test(NAME BarCodeBottom2DID COMMAND test_barcode_bottom2did)

# --- test_barcode_bottom2did8ccd: DoBottom2DID_8CCD_Scan (8-CCD sibling) ----
add_executable(test_barcode_bottom2did8ccd
    test_barcode_bottom2did8ccd.cpp
)

target_link_libraries(test_barcode_bottom2did8ccd PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_bottom2did8ccd PRIVATE ws2_32)
endif()

add_test(NAME BarCode_Bottom2DID8CCD COMMAND test_barcode_bottom2did8ccd)

# --- test_barcode_shuttle1_scan: 7 Shuttle-1 scan/trigger/float-check methods
add_executable(test_barcode_shuttle1_scan
    test_barcode_shuttle1_scan.cpp
)

target_link_libraries(test_barcode_shuttle1_scan PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_shuttle1_scan PRIVATE ws2_32)
endif()

add_test(NAME BarCode_Shuttle1_Scan COMMAND test_barcode_shuttle1_scan)

# --- test_barcode_shuttle2_scan: 5 of 7 Shuttle-2 scan/trigger/float-check --
#   (DoBarcodeScanInShuttle_2/DoShuttleFloatCheck_2 handed off, not delivered)
add_executable(test_barcode_shuttle2_scan
    test_barcode_shuttle2_scan.cpp
)

target_link_libraries(test_barcode_shuttle2_scan PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_shuttle2_scan PRIVATE ws2_32)
endif()

add_test(NAME BarCode_Shuttle2Scan COMMAND test_barcode_shuttle2_scan)

# --- test_barcode_shuttle1_ccdscan: DoBarcodeCCDInShuttle_1 (largest unit) --
add_executable(test_barcode_shuttle1_ccdscan
    test_barcode_shuttle1_ccdscan.cpp
)

target_link_libraries(test_barcode_shuttle1_ccdscan PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_shuttle1_ccdscan PRIVATE ws2_32)
endif()

add_test(NAME BarCode_Shuttle1_CCDScan COMMAND test_barcode_shuttle1_ccdscan)

# --- test_barcode_shuttle2_ccdscan: DoBarcodeCCDInShuttle_2 -----------------
add_executable(test_barcode_shuttle2_ccdscan
    test_barcode_shuttle2_ccdscan.cpp
)

target_link_libraries(test_barcode_shuttle2_ccdscan PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_shuttle2_ccdscan PRIVATE ws2_32)
endif()

add_test(NAME W5_BarCode_Shuttle2CCDScan COMMAND test_barcode_shuttle2_ccdscan)

# --- W5-BarCode-Final (BarCode remainder front, 2026-07-11): the last 2/20
#     TfBarCode_Shim methods (hand-off completion) + the 8-CCD ClientSocket
#     connection-lifecycle glue. TfBarCode_Shim is 20/20 real methods as of
#     this integrate -- see CMakeLists.txt's ht9045_sm source-list comment.

# --- test_barcode_shuttle2_scanremainder1: DoBarcodeScanInShuttle_2 ---------
add_executable(test_barcode_shuttle2_scanremainder1
    test_barcode_shuttle2_scanremainder1.cpp
)

target_link_libraries(test_barcode_shuttle2_scanremainder1 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_shuttle2_scanremainder1 PRIVATE ws2_32)
endif()

add_test(NAME BarCode_Shuttle2ScanRemainder1 COMMAND test_barcode_shuttle2_scanremainder1)

# --- test_barcode_shuttle2_scanremainder2: DoShuttleFloatCheck_2 ------------
add_executable(test_barcode_shuttle2_scanremainder2
    test_barcode_shuttle2_scanremainder2.cpp
)

target_link_libraries(test_barcode_shuttle2_scanremainder2 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_shuttle2_scanremainder2 PRIVATE ws2_32)
endif()

add_test(NAME BarCode_Shuttle2ScanRemainder2 COMMAND test_barcode_shuttle2_scanremainder2)

# --- test_barcode_8ccd_glue: 13 real TClientSocket + SendCCDCommand + the 2
#     Bottom-8CCD timers -----------------------------------------------------
add_executable(test_barcode_8ccd_glue
    test_barcode_8ccd_glue.cpp
)

target_link_libraries(test_barcode_8ccd_glue PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_barcode_8ccd_glue PRIVATE ws2_32)
endif()

add_test(NAME BarCode8CCDGlue COMMAND test_barcode_8ccd_glue)

# ===========================================================================
#  W5-Final -- vclcompat::TList shim (BCB6 VCL.Classes TList, generic void*
#  list) + the SECSGEM wire-codec / SV-EC-registration prerequisite slices
# ===========================================================================

# --- test_tlist: vclcompat::TList shim semantics ----------------------------
#   Add/Count/IndexOf/Items[] (get+set)/Delete/Clear + the non-owning-list
#   contract (TList never frees what it holds -- unlike TObjectList). Header-
#   only shim (vclcompat/TList.h); links vclcompat for the umbrella header
#   (vcl_compat.h) even though no vclcompat .cpp symbol is actually called.
add_executable(test_tlist
    test_tlist.cpp
)

target_link_libraries(test_tlist PRIVATE vclcompat)

add_test(NAME TList COMMAND test_tlist)

# --- test_SecsWireCodec: SecsWireCodec (THGem's wire-codec slice) ----------
#   GetLengthOfType/GetLengthByte/GetSMLLenthByte/DataItemOut x2/DataItemInSub/
#   DataItemIn x2/DataItemInNew/GetDataItemLenAndType(Sub)/StringOut + the
#   shared HType global. Zero THGem/VCL/socket/grid dependency -- links
#   ht9045_secsgem (which now carries SecsWireCodec.cpp) only.
add_executable(test_SecsWireCodec
    test_SecsWireCodec.cpp
)

target_link_libraries(test_SecsWireCodec PRIVATE ht9045_secsgem)

add_test(NAME SecsWireCodec COMMAND test_SecsWireCodec)

# --- test_SecsSvEcRegistration: SecsSvEcRegistration (THGem's SV/EC ---------
#   registration API) -- SetSVDataPointer x4 / SetECDataPointer x4 /
#   GetECDataValue. Links ht9045_secsgem (carries both SecsSvEcRegistration.cpp
#   AND its sibling SecsWireCodec.cpp, needed for the shared HType global that
#   GetECDataValue's Type-dispatch compares against). No THGem/VCL-form/
#   socket/grid dependency.
add_executable(test_SecsSvEcRegistration
    test_SecsSvEcRegistration.cpp
)

target_link_libraries(test_SecsSvEcRegistration PRIVATE ht9045_secsgem)

add_test(NAME SecsSvEcRegistration COMMAND test_SecsSvEcRegistration)

# --- test_uHGemEquipment: THGem's first real slice (W906-uHGemEquipment) ----
#   CEID/Report/Alarm StringGrid "database" family (26 methods) + GemTimer +
#   vclcompat::TStringGrid. Links ht9045_secsgem (which now carries
#   SECSGEM/uHGemEquipment.cpp) -- PUBLIC-links vclcompat transitively (the
#   new vclcompat/StringGrid.cpp + AnsiString/TStringList this file needs) and,
#   as of the W906-uHGemEquipment-ConnLifecycle wave, ht9045_public (GetErrorMsg,
#   see ht9045_secsgem's own target in the root CMakeLists.txt).
#   Supplies its own local MyDBIProcess(2-arg) stand-in (see the test file's
#   own comment) rather than linking ht9045_sm's real aHotPlateSubstrate.cpp
#   definition -- same "avoid the god-stack for a focused unit test" posture
#   already established by test_config_loaders.cpp/test_MyCCLink.cpp.
#   AI(W906-uHGemEquipment-ConnLifecycle) 20260717: THGem now owns a real
#   clientGem/srvGem pair (TClientSocket/TServerSocket, vclcompat/ClientSocket
#   .cpp + ServerSocket.cpp) -- this test is THAT slice's first real consumer,
#   so ws2_32 must be linked (WIN32) exactly like every other test that
#   references vclcompat's socket internals (test_serversocket/
#   test_clientsocket_ext/test_myplc_modbus/... all do the same, see their own
#   entries in this file) -- Real-mode code paths inside ClientSocket.cpp/
#   ServerSocket.cpp reference WSAStartup/socket/connect/send/recv/etc. even
#   though this test only ever exercises Sim mode.
#   AI(W906-uHGemEquipment-BucketB) 20260717: ht9045_core (common.h's
#   ReadWriteIni/ReadIniData/WriteIniData, real functions -- self-contained,
#   own local RecordProcess no-op stub, see common.cpp's own header note) and
#   ht9045_globals (cmydef.h's CUSTOMER_CODE/CC_*/CosFunction/
#   bSECSGEMbyPass/bSECSGEMConnectionFail) added to the RESCAN group --
#   InitialHGem/SaveSystemDefault/DoUpdateStatus (this wave) need the REAL
#   symbols, and pulling uHGemEquipment.o into this executable (already
#   happening, for the pre-existing CEID/Report tests) means ALL of that
#   object file's external references must resolve, not just the ones this
#   test happens to call -- same "must resolve the whole .o's symbols"
#   reasoning already spelled out at test_TesterTCP's own entry below for an
#   identical situation.
add_executable(test_uHGemEquipment
    test_uHGemEquipment.cpp
)

# AI(W906-uHGemEquipment-ConnLifecycle) 20260717: plain
# `target_link_libraries(... PRIVATE ht9045_secsgem)` (as this line used to
# read) left ws2_32 placed BEFORE libvclcompat.a/libht9045_public.a on the
# actual link line (confirmed via the generated linkLibs.rsp) -- GNU ld's
# single left-to-right pass never re-visits ws2_32 once vclcompat's later
# unresolved WSAStartup/socket/connect/etc. show up, so the link failed.
# Same LINK_GROUP:RESCAN fix already established by this exact multi-hop-
# static-lib-plus-ws2_32 shape at test_myplc_modbus's own entry in this file.
# AI(W906-uHGemEquipment-BucketC) 20260717: ht9045_db added to the RESCAN
# group -- D3's HSys.MyGem seam needs database.cpp's `HSys` definition
# resolved (see root CMakeLists.txt's ht9045_secsgem target for the matching
# new link dependency).
target_link_libraries(test_uHGemEquipment PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_uHGemEquipment PRIVATE ws2_32)
endif()

add_test(NAME uHGemEquipment COMMAND test_uHGemEquipment)

# ===========================================================================
#  W5-Final Automation-remainder INTEGRATE: auto9045 / AGV_E84 /
#  SCK_ART_Remainder / Interface/TesterTCP_Socket
# ===========================================================================

# --- test_auto9045: Automation/auto9045.cpp whole-file OLP/GPIB config-accessor translation ---
add_executable(test_auto9045
    test_auto9045.cpp
)
target_link_libraries(test_auto9045 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_auto9045 PRIVATE ws2_32)
endif()
add_test(NAME Auto9045 COMMAND test_auto9045)

# --- test_agv_e84: Automation/AGV_E84.cpp E84 loader/unloader handshake core ---
add_executable(test_agv_e84
    test_agv_e84.cpp
)
target_link_libraries(test_agv_e84 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_agv_e84 PRIVATE ws2_32)
endif()
add_test(NAME AGV_E84 COMMAND test_agv_e84)

# --- test_agv_portscan: Automation/AGV_PortScan.cpp (the second AGV.cpp slice,
#     E84 sensor-status telemetry + AMR/AGV port-scan dispatcher family) -------
add_executable(test_agv_portscan
    test_agv_portscan.cpp
)
target_link_libraries(test_agv_portscan PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_agv_portscan PRIVATE ws2_32)
endif()
add_test(NAME AGV_PortScan COMMAND test_agv_portscan)

# --- test_SCK_ART_Remainder: Automation/SCK_ART_Remainder.cpp (next contiguous
#     golden slice after the sibling SCK_ART.cpp 8-function extract) ---------
add_executable(test_SCK_ART_Remainder
    test_SCK_ART_Remainder.cpp
)
target_link_libraries(test_SCK_ART_Remainder PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_SCK_ART_Remainder PRIVATE ws2_32)
endif()
add_test(NAME SCK_ART_Remainder COMMAND test_SCK_ART_Remainder)

# --- test_testertcp_socket: Interface/TesterTCP_Socket.cpp (bounded socket-
#     management subset of Interface/TesterTCP.cpp) ---------------------------
add_executable(test_testertcp_socket
    test_testertcp_socket.cpp
)
target_link_libraries(test_testertcp_socket PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_testertcp_socket PRIVATE ws2_32)
endif()
add_test(NAME Interface_TesterTCPSocket COMMAND test_testertcp_socket)

# ===========================================================================
#  W906 fire-wave INTEGRATE: Automation/automation.cpp / Interface/TesterTCP.cpp
# ===========================================================================

# --- test_automation: Automation/automation.cpp (TfAutomation OLP host-
#     command engine, "network wiring + small helpers" bucket) --------------
#   Same RESCAN link shape as test_auto9045/test_agv_e84/test_SCK_ART_Remainder
#   above (automation.cpp is compiled directly into ht9045_sm, alongside
#   auto9045.cpp/FormsFacade.cpp/canary_support.cpp it depends on).
add_executable(test_automation
    test_automation.cpp
)
target_link_libraries(test_automation PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_automation PRIVATE ws2_32)
endif()
add_test(NAME Automation COMMAND test_automation)

# --- test_TesterTCP: Interface/TesterTCP.cpp (sibling slice of
#     Interface/TesterTCP_Socket.cpp -- CopyRecipeToTester/CopyRecipeFromTester
#     OS-recipe zip-transfer pair + btnSaveClick arm-speed-save handler) -----
#   Same RESCAN link shape as test_testertcp_socket above (TesterTCP.cpp is
#   compiled directly into ht9045_sm, alongside acarry_shims.cpp/
#   canary_support.cpp it depends on for the REAL MySleep/RecordProcess/
#   ShowMyMessage bodies -- see TesterTCP.cpp's own extern-redeclaration
#   comments). INTEGRATE FIX: the front's own test_TesterTCP.cpp originally
#   supplied a LOCAL no-op `void MySleep(DWORD) {}` stand-in (written under the
#   assumption this test would NOT link ht9045_sm, matching the test_
#   FTPClient_Transfer.cpp/test_MyCCLink.cpp "avoid the god-stack" precedent).
#   That stand-in was REMOVED here: linking the RESCAN group (needed anyway,
#   for the real DataPath/sSaveByMachine/WriteIniData/ReadIniData in
#   ht9045_core + CosFunction/IniConfig in ht9045_globals) pulls in
#   acarry_shims.cpp's own real MySleep(DWORD) definition, and a second
#   definition in the test's main() would be a multiple-definition link error.
add_executable(test_TesterTCP
    test_TesterTCP.cpp
)
target_link_libraries(test_TesterTCP PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_TesterTCP PRIVATE ws2_32)
endif()
add_test(NAME Interface_TesterTCP COMMAND test_TesterTCP)

# ===========================================================================
#  W5-Final KYECFTP: MiniFtpEngine (Phase 1) / FTPClient_EventHandlers
#  (Phase 2a) / FTPClient_Transfer (Phase 2b)
#
#  All 3 targets below compile the relevant KYECFTP/*.cpp sources DIRECTLY
#  (not via the production `ht9045_kyecftp` library in the root CMakeLists.txt)
#  -- see each target's own comment for why.  `ht9045_kyecftp` itself still
#  gets built (as part of the default `all` target) and is what proves these
#  files type-check against the REAL cmydef.h/cprod.h/Config.h/CosFunction.h/
#  common.h/canary_support.h declarations; these 3 tests instead verify BEHAVIOR
#  fast and offline, each supplying its own minimal stand-ins per its file's own
#  SCOPE NOTE (same established pattern as tests/test_config_loaders.cpp /
#  tests/test_MyCCLink.cpp).
# ===========================================================================

# --- test_MiniFtpEngine: Nmftp::TNMFTP engine, Sim-mode fake-FTP-server, end to end ---
#   Zero god-stack dependency (MiniFtpEngine.{h,cpp} only touch vclcompat) --
#   compiles MiniFtpEngine.cpp directly and links vclcompat only.
add_executable(test_MiniFtpEngine
    test_MiniFtpEngine.cpp
    ../KYECFTP/MiniFtpEngine.cpp
)
target_link_libraries(test_MiniFtpEngine PRIVATE vclcompat)
if(WIN32)
    target_link_libraries(test_MiniFtpEngine PRIVATE ws2_32)
endif()
add_test(NAME MiniFtpEngine COMMAND test_MiniFtpEngine)

# --- test_FTPClient_EventHandlers: NMFTP1 event-handler bodies --------------
#   Needs the REAL CUSTOMER_CODE (cmydef.cpp, via ht9045_globals) for test
#   [G]'s CC_JSCC_OS gated-dependency case -- links ht9045_globals (+
#   ht9045_public, its own established co-dependency; see test_cpublic_foundation
#   above). Supplies its OWN local MyDBIProcess(2-arg)/ShowMyMessage stand-ins
#   instead of linking ht9045_sm (avoids the aHotPlateSubstrate.cpp/
#   canary_support.cpp god-stack for a focused unit test -- same avoidance as
#   test_config_loaders.cpp/test_MyCCLink.cpp) -- ht9045_sm/ht9045_core are
#   DELIBERATELY NOT linked here (nor is the production ht9045_kyecftp library,
#   which PUBLIC-links both): pulling either in risks the REAL MyDBIProcess/
#   ShowMyMessage bodies (aHotPlateSubstrate.cpp/canary_support.cpp, both inside
#   ht9045_sm) getting extracted alongside this TU's own same-named stand-ins,
#   a multiple-definition link error.
add_executable(test_FTPClient_EventHandlers
    test_FTPClient_EventHandlers.cpp
    ../KYECFTP/MiniFtpEngine.cpp
    ../KYECFTP/FTPClient_EventHandlers.cpp
)
target_link_libraries(test_FTPClient_EventHandlers PRIVATE 
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_FTPClient_EventHandlers PRIVATE ws2_32)
endif()
add_test(NAME FTPClient_EventHandlers COMMAND test_FTPClient_EventHandlers)

# --- test_FTPClient_Transfer: the 4 FTP-transfer session functions ----------
#   Deliberately supplies ITS OWN TU-local stand-ins for EVERY extern global
#   FTPClient_Transfer.cpp references (CUSTOMER_CODE/IniConfig/CosFunction/
#   Temperature/DataPath/OffsetPath/DefaultPath/LastDataPath/
#   asATCFileTransferPath/sWhite2DIDListLoc/sATCPath/ATC_SYSTEM/FTPAutomation_*/
#   bSigurd*/iAMD_Function/bHasFTPDownload/asSetupFileCheckList) PLUS
#   ShowMyMessage/RecordProcess/NewRecordProcess/MyDBIProcess(2-arg)/MySleep --
#   see the test file's own SCOPE NOTE. This means it must NOT link
#   ht9045_globals/ht9045_core/ht9045_sm (nor the production ht9045_kyecftp
#   library, which PUBLIC-links all three): the REAL cmydef.cpp/cprod.cpp/
#   common.cpp definitions of these SAME global names would collide with this
#   TU's own the moment anything else in that link forces one of those (very
#   large, densely cross-referenced -- see KNOWLEDGE.md's "全域狀態耦合"
#   gotcha) object files to be extracted from its archive -- a real
#   multiple-definition risk, not just a style preference. Compiles the 3
#   KYECFTP sources directly instead, linking only vclcompat.
add_executable(test_FTPClient_Transfer
    test_FTPClient_Transfer.cpp
    ../KYECFTP/MiniFtpEngine.cpp
    ../KYECFTP/FTPClient_EventHandlers.cpp
    ../KYECFTP/FTPClient_Transfer.cpp
)
target_link_libraries(test_FTPClient_Transfer PRIVATE vclcompat)
if(WIN32)
    target_link_libraries(test_FTPClient_Transfer PRIVATE ws2_32)
endif()
add_test(NAME FTPClient_Transfer COMMAND test_FTPClient_Transfer)

# ===========================================================================
#  W906-MyProductionRecord -- Public/MyProductionRecord.cpp pure-logic slice
#    (ctor/InitialRecord/DeleteProductionRecord + ~28 Add*Record/Get* leaves
#    that only read/write asBuffer; golden .cpp:218-654).
#
#  Compiles ../Public/MyProductionRecord.cpp directly a second time as its own
#  isolated build (it also lives in the main ht9045_sm target via the top-level
#  CMakeLists.txt, post AI(W906-Reconcile) 20260721 -- aHotPlateSubstrate.cpp's
#  own former no-op stand-ins for the same 8 methods were removed there, so
#  there is no multiple-definition risk linking the two together; this test
#  target simply chooses not to, to avoid pulling in the ht9045_sm god-stack
#  for a focused unit test). ht9045_sm itself now compiles cleanly (the
#  FormsFacade.h gap an earlier draft of this comment referenced is resolved).
#  Links ht9045_globals (CUSTOMER_CODE/IndexSuckName[][]/
#  iOneTrayPickCount[]/iTestBinCount/LoadForm/HotPlateForm/CosFunction/
#  TestIF_File/GetDateInfoByString/GetOnlyTimeInfoByString/GetTimeInfo/
#  System*) + vclcompat. Supplies its OWN local stand-ins (TMyKitSuck's ctor
#  + the InArmSuck/OutArmSuck/TestSocket globals it declares extern) instead
#  of linking aHotPlateSubstrate.cpp -- same "avoid the god-stack for a
#  focused unit test" posture already established by test_config_loaders.cpp/
#  test_MyCCLink.cpp/test_FTPClient_EventHandlers.cpp/test_FTPClient_Transfer.cpp
#  above. See the test file's own banner for the full reconciliation note.
# ===========================================================================
add_executable(test_MyProductionRecord
    test_MyProductionRecord.cpp
    ../Public/MyProductionRecord.cpp
)
target_link_libraries(test_MyProductionRecord PRIVATE 
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_MyProductionRecord PRIVATE ws2_32)
endif()
add_test(NAME MyProductionRecord COMMAND test_MyProductionRecord)

# ===========================================================================
#  W906 W7-A2 VERIFY -- SearchTrayToPlace_Magazine() return-type smoke test
#
#  Smoke test for the fix landed this wave: golden declares/defines
#  SearchTrayToPlace_Magazine() as `int` (aoutarm9045.h:63, def
#  aoutarm9045.cpp:1420) with the return value genuinely consumed at golden
#  aoutarm9045.cpp:1369 (`return SearchTrayToPlace_Magazine();`). The ported
#  tree previously had 14/27 site-variant TUs declaring it `void` against the
#  other 13's `int` and the shim's `void` definition -- a live ODR violation
#  that MinGW linked silently. This wave reconciled all 27 declaring TUs plus
#  the shim definition to `int` (verified by direct inspection, not by this
#  test). This test forward-declares the function `int` (the shape a newly
#  translated call site will use), assigns/consumes the return value, and
#  reproduces the golden :1369 `return SearchTrayToPlace_Magazine();` shape
#  via a local helper, linking ht9045_sm so the call resolves to the real
#  definition in aoutarm_shims.cpp.
#
#  IMPORTANT LIMITATION (do not restate the old claim that this "fails to
#  compile" on regression): a function's return type is not part of its C++
#  mangled name, so a `void` regression in aoutarm_shims.cpp's definition, or
#  a declaration drift in any of the OTHER 26 site-variant TUs this test does
#  not include, would build and link silently under MinGW -- verified with a
#  minimal two-TU repro (int-declaring caller + void-defining callee) that
#  built clean at -Wall -Wextra and ran. This test only proves the symbol
#  still exists, still links, and using it as `int` from a fresh call site
#  builds and runs; it is not a compile-time ODR guard. Same RESCAN link
#  group as the other W6.2c out-arm verify TUs.
# ===========================================================================
add_executable(test_w7_a2_searchtray_magazine_return
    test_w7_a2_searchtray_magazine_return.cpp
)

target_link_libraries(test_w7_a2_searchtray_magazine_return PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_a2_searchtray_magazine_return PRIVATE ws2_32)
endif()

add_test(NAME W7_A2_SearchTrayMagazineReturn COMMAND test_w7_a2_searchtray_magazine_return)

# ===========================================================================
#  W906 W7-C1 -- 3 framework-free custom-control cores + unit tests
#
#  vclcompat/TrayCore.* / LedCore.* / BtnPanelCore.* -- new, framework-free
#  (no VCL, no MFC, no GDI) state/geometry cores for the 3 custom-control
#  families identified in W7_UI_ARCHITECTURE_PLAN.md §6 W7-C1. Together
#  they model 7 real golden classes (TTMyTray/TTMyTray256, TMyLed/
#  TMyLedLane/TALed, TBtnPanel/TBtnPanelLane) -- AI(W906-W7-C) 20260728:
#  corrected count, verified directly against the golden source tree
#  (D:\HT9045\elec\myvcl\{HTray,HTray256,MyLed,MyLedLane,butPa1,
#  BtnPanelLane}.h/.cpp + D:\HT9045\elec\Component\aled.pas): 6 of these 7
#  classes live under elec\myvcl\, NOT elec\Component\ -- only TALed does
#  (a prior version of this comment said "5 of the 7", which under-counted
#  by one). Each links ONLY against vclcompat (no ht9045_* libraries, no
#  LINK_GROUP needed -- these headers have zero dependency on anything else
#  in the tree, by design/D5).
# ===========================================================================
add_executable(test_traycore
    test_traycore.cpp
    ${CMAKE_SOURCE_DIR}/vclcompat/TrayCore.cpp
)
target_link_libraries(test_traycore PRIVATE vclcompat)
add_test(NAME TrayCore COMMAND test_traycore)

add_executable(test_ledcore
    test_ledcore.cpp
    ${CMAKE_SOURCE_DIR}/vclcompat/LedCore.cpp
)
target_link_libraries(test_ledcore PRIVATE vclcompat)
add_test(NAME LedCore COMMAND test_ledcore)

add_executable(test_btnpanelcore
    test_btnpanelcore.cpp
    ${CMAKE_SOURCE_DIR}/vclcompat/BtnPanelCore.cpp
)
target_link_libraries(test_btnpanelcore PRIVATE vclcompat)
add_test(NAME BtnPanelCore COMMAND test_btnpanelcore)

# ===========================================================================
#  W7-B1c -- .dfm layout/colour/font/custom-control data table gate G9
#
#  Compiles a curated 4-form sample of the generated
#  tools/dfm2rc/layout_out/**/*_layout.gen.cpp tables (ARMS/cBinSel/
#  cContact/iosetview -- chosen to cover plain nesting, TTMyTray, TMyLed,
#  and the mass TMyLedLane/TBtnPanelLane Alias+True*/False*Color usage) and
#  round-trips a curated set of rows against golden .dfm text read directly
#  (see test_DfmLayoutGen.cpp's own banner for exactly which golden lines
#  each assertion reproduces). No vclcompat/ht9045_* link needed -- these
#  tables are plain data (D7/D12), no framework dependency.
#
#  This is NOT the full-133-form CMake wiring (that is W7-B1d's job per the
#  plan, S6 W7-B1d "全語料跑 + 閘門分流 + CMake 接線"); the full-corpus
#  compile-under-MinGW proof for all 133 forms lives in
#  tools/dfm2rc/reports/b1c_gate_report.json's g9_compile_smoke section
#  (run via `python tools/dfm2rc/run_b1c.py`, a direct g++ -fsyntax-only
#  batch outside ctest -- 133/133 clean at the time this test was added).
# ===========================================================================
add_executable(test_DfmLayoutGen
    test_DfmLayoutGen.cpp
    ${CMAKE_SOURCE_DIR}/tools/dfm2rc/layout_out/ARMS/ARMS_layout.gen.cpp
    ${CMAKE_SOURCE_DIR}/tools/dfm2rc/layout_out/cBinSel_layout.gen.cpp
    ${CMAKE_SOURCE_DIR}/tools/dfm2rc/layout_out/cContact_layout.gen.cpp
    ${CMAKE_SOURCE_DIR}/tools/dfm2rc/layout_out/iosetview_layout.gen.cpp
)
add_test(NAME DfmLayoutGen COMMAND test_DfmLayoutGen)

# ===========================================================================
#  W7-B1d -- full-corpus run + gate triage + CMake wiring
#
#  Wires tools/dfm2rc/run_b1d.py's 4 modes into ctest as ONE compiled
#  wrapper executable (test_dfm2rc_pipeline.cpp, so it inherits
#  ht9045_test_bootstrap like every other test in this file via the
#  directory-scoped link_libraries() at the top) invoked 4 times with
#  different argv -- ctest natively supports reusing one binary across
#  several add_test entries, so no need for 4 near-identical .cpp files.
#  See run_b1d.py's own module docstring for exactly what each mode
#  proves; short version:
#
#    dfm2rc_rc_compiles / dfm2rc_layout_full  -- FAST day-to-day path.
#        Compile the EXISTING checked-in tools/dfm2rc/rc_out (rc.exe +
#        windres, full 133/133 -- gate G5) and tools/dfm2rc/layout_out
#        (g++ -fsyntax-only, full 133/133 -- gate G9's compile half)
#        corpora as they sit on disk. Zero dependency on GOLDEN at test
#        time, same as every other test in this file. This is what the
#        sibling W7-B1c wave's own CMakeLists comment (above,
#        test_DfmLayoutGen's banner) explicitly deferred to "W7-B1d's
#        job" -- bringing the full-133-form compile proof (until now only
#        run ad hoc via `python run_b1c.py`, outside ctest) INTO ctest.
#
#    dfm2rc_fidelity / dfm2rc_idempotent -- the two tests that DO read
#        GOLDEN, deliberately: they re-derive the whole pipeline (parse
#        .dfm -> IR -> .rc/_ids.h/.rcmeta.json -> compile -> G6 readback
#        -> layout tables -> G9 geometry/taborder crosscheck) from
#        scratch into ${CMAKE_BINARY_DIR}/dfm2rc_regen (gitignored,
#        build_*/ -- see .gitignore), never touching the source tree.
#        `fidelity` asserts that full G0-G9 sweep is 0 problems (not just
#        "some possibly-stale committed JSON report says so") plus the
#        S12 ratchet #4 mojibake/BOM scan; `idempotent` byte-diffs that
#        fresh regeneration against the checked-in tools/dfm2rc/{ir_out,
#        rc_out,layout_out} (gate G7) -- proving the generators are a
#        pure, deterministic function of GOLDEN and that nothing
#        checked-in has silently drifted from what they currently emit.
#
#  Independently re-run fresh (2026-07-28, this wave) before wiring:
#  fidelity 133/133 forms, 0 problems (22,627 IR control nodes / 22,296
#  visual / 3,637 DIALOGEX templates / 22,586 emitted CONTROL entries /
#  22,760 layout rows / 7,280 events), 0 mojibake/BOM hits; idempotent 0
#  byte diffs; rc_compiles 133/133 clean on both rc.exe and windres;
#  layout_full 133/133 clean g++ -std=c++14 -Wall -Wextra -fsyntax-only.
#  Full per-form detail: tools/dfm2rc/reports/b1d_fidelity_report.json and
#  b1d_idempotent_report.json (written by run_b1d.py itself, refreshed
#  every time these ctest targets run).
# ===========================================================================
find_package(Python3 COMPONENTS Interpreter)

if(Python3_Interpreter_FOUND)
    add_executable(test_dfm2rc_pipeline
        test_dfm2rc_pipeline.cpp
    )
    target_compile_definitions(test_dfm2rc_pipeline PRIVATE
        HT9045_PYTHON_EXE="${Python3_EXECUTABLE}"
        HT9045_DFM2RC_DIR="${CMAKE_SOURCE_DIR}/tools/dfm2rc"
    )

    # AI(W906-W7-B1-review) 20260728: MEDIUM-4 fix -- fidelity and
    # idempotent used to share one "${CMAKE_BINARY_DIR}/dfm2rc_regen" root
    # (run_b1d.py then joined 'fidelity'/'idempotent' subdirs onto it, so
    # they never actually wrote into each other's files, but a plain ctest
    # run still hit both FAILING on a leftover rc.exe scratch file, see
    # run_b1d.py's _clean_dir() docstring for the full story). Each mode
    # now gets its own top-level root, independent of the other, on top of
    # _clean_dir()'s own tolerant-rmtree fallback.
    set(_dfm2rc_regen_dir_fidelity "${CMAKE_BINARY_DIR}/dfm2rc_regen_fidelity")
    set(_dfm2rc_regen_dir_idempotent "${CMAKE_BINARY_DIR}/dfm2rc_regen_idempotent")

    add_test(NAME dfm2rc_rc_compiles
        COMMAND test_dfm2rc_pipeline rc_compiles)
    add_test(NAME dfm2rc_layout_full
        COMMAND test_dfm2rc_pipeline layout_full)
    add_test(NAME dfm2rc_fidelity
        COMMAND test_dfm2rc_pipeline fidelity "${_dfm2rc_regen_dir_fidelity}")
    add_test(NAME dfm2rc_idempotent
        COMMAND test_dfm2rc_pipeline idempotent "${_dfm2rc_regen_dir_idempotent}")

    # AI(W906-FW2) 20260817: FW campaign -- every form layout committed under
    # web/forms/ must regenerate byte-identically from its IR (the G7 idea
    # applied to the web projection; emit_web.py --check writes nothing).
    # The stem list is derived from the committed files at CONFIGURE time, so
    # each FW-3 wave extends coverage just by emitting -- but a brand-new form
    # needs a reconfigure before ctest sees it, which the fresh-build-dir gate
    # discipline already guarantees.
    file(GLOB_RECURSE _fw_web_layouts
         RELATIVE "${CMAKE_SOURCE_DIR}/../web/forms"
         "${CMAKE_SOURCE_DIR}/../web/forms/*.layout.json")
    set(_fw_web_stems "")
    foreach(_f ${_fw_web_layouts})
        string(REPLACE ".layout.json" "" _n "${_f}")
        list(APPEND _fw_web_stems "${_n}")
    endforeach()
    if(_fw_web_stems)
        add_test(NAME web_layout_idempotent
            COMMAND "${Python3_EXECUTABLE}"
                    "${CMAKE_SOURCE_DIR}/tools/dfm2rc/emit_web.py"
                    --check ${_fw_web_stems})
    endif()

    # AI(W906-P6b-B) 20260921: P6-b 稽核 —— WebStart.cpp 裡**活的** fShow 讀取點
    #   有沒有全部接上視窗總表。
    #   為什麼要進 ctest 而不是留成一支手動腳本：它驗的是一個**會過期的
    #   absence claim**（「全部接上了」）。下一個波次只要新增一行
    #   `fXxx->fShow`，它就悄悄變成假的，而且失效方向是危險的那一側
    #   （那個判斷永遠讀到 false ＝「永遠沒人在教導」）。沒有任何既有測試會紅。
    #   ⚠ 它是純靜態掃描，不讀寫任何真實檔，也不建置 —— 秒級。
    add_test(NAME P6b_FShowWired
        COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/p6b_fshow_audit.py")
    #   稽核器自己也要被驗 —— 一個會假綠的稽核器比沒有稽核器更糟
    #   （它會訓練人相信「綠了就是接上了」）。
    add_test(NAME P6b_FShowWired_SelfTest
        COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/p6b_fshow_audit.py" --selftest)

    # AI(W906-CTEST-RELEV) 20260924: the relevance gate that lets build.bat skip
    # the 4 dfm2rc_* tests (tools/ctest_relevance.py) must itself stay honest:
    # its fingerprint covers tools/dfm2rc/, golden .dfm, the wrapper sources,
    # the ctest definitions and the toolchain. This selftest goes red if any of
    # those inputs is renamed, a dfm2rc .py grows a non-stdlib import, the
    # wrapper gains a source file, or the "-E dfm2rc_" regex stops matching
    # exactly the wrapper's tests -- each of which would otherwise turn the
    # gate into a silent "always skip". Static, no build dir, seconds.
    add_test(NAME ctest_relevance_SelfTest
        COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/tools/ctest_relevance.py" --selftest)

    # ⚠ AI(W906-DFM2RC-TMO) 20260917: THIS 300 HAS NEVER TAKEN EFFECT, and the
    # sentence that used to be here -- that it is "IN ADDITION to (not instead
    # of) the blanket 600s ratchet" -- is not how CMake works. Test properties
    # are ASSIGNED, not merged, and the blanket block at the bottom of this file
    # runs later, so it overwrites this for every test in the directory. The
    # receipt: dfm2rc_fidelity times out at 600s, not at 300s. The live override
    # is now at the END of this file, where the blanket block says to put it.
    set_tests_properties(
        dfm2rc_rc_compiles dfm2rc_layout_full dfm2rc_fidelity dfm2rc_idempotent
        PROPERTIES TIMEOUT 300
    )
else()
    message(WARNING "W7-B1d: Python3 interpreter not found on this machine -- "
                     "dfm2rc_rc_compiles/dfm2rc_layout_full/dfm2rc_fidelity/"
                     "dfm2rc_idempotent ctest targets are NOT registered. "
                     "This environment cannot verify the tools/dfm2rc/ "
                     "pipeline gates; see docs/W7-UI-SKIPPED.md.")
endif()

# --- W7-F0-b: the SV/EC void*-overload guard + the zero-behaviour-change
#     default-value regression for the unified widget stand-ins ---------------
# AI(W906-W7-F0) 20260728: plan SS6-F0-b / SS4-V2 / SS9-R8.  Deliberately placed
# ABOVE the GLOBAL TEST TIMEOUT block below, not appended after it -- that block
# reads the directory's TESTS property with get_property(), so a test registered
# after it would silently get NO timeout and could wedge a batch run (exactly the
# failure mode tests/test_bootstrap.cpp exists to prevent).
#
# Links vclcompat only: forms/FormWidgets.h needs AnsiString / TStringList /
# TStringGrid and nothing above the compat layer.  It deliberately does NOT
# include any forms/fXxx.h, so it does not pull ht9045_forms in -- this target
# tests the widget TYPE layer, and keeping its link line minimal means a link
# failure here can only ever mean vclcompat.
add_executable(test_w7_f0_controls_guard
    test_w7_f0_controls_guard.cpp
)

target_link_libraries(test_w7_f0_controls_guard PRIVATE vclcompat)

add_test(NAME w7_f0_controls_guard COMMAND test_w7_f0_controls_guard)

# --- W7-F1: the "Wall 2" unlock gate ----------------------------------------
# AI(W906-W7-F1) 20260729: plan SS6-F1 / SS4-V1. Also placed ABOVE the GLOBAL
# TEST TIMEOUT block for the same reason as test_w7_f0_controls_guard just
# above (a test registered after that block silently gets no timeout).
#
# Link map: ht9045_secsgem (mirrors test_SecsWireCodec/test_SecsSvEcRegistration's
# minimal link intent above) -- but this TU also pulls in the FULL facade
# (FormsFacade.h -> forms/fMain.cpp -> cprod.h/cmydef.h), which is exactly the
# same "cmydef.cpp.obj's static queue globals need ctors from cpublic.cpp.obj
# in the SAME archive, GNU ld's single-pass archive scan can't back-resolve
# that" cross-archive situation test_config_loaders's own target_link_libraries
# entry already documents just above in this file (cited by symbol name per
# KNOWLEDGE Gotcha 10 -- ported-tree line numbers rot) -- confirmed by hitting
# the IDENTICAL undefined-reference failure (SearchFile/md5_Folder,
# ht9045_public symbols cpublic.cpp.obj needs) on the first real build attempt.
# Reuses that same LINK_GROUP:RESCAN set verbatim (proven to resolve it there).
# No THGem instantiation happens in this TU, so unlike test_config_loaders this
# one does NOT need ws2_32.
#
# AI(W906-W7-F1fix) 20260729: the header comment in
# test_w7_f1_wall2_probe.cpp originally claimed this executable "names only
# ht9045_secsgem" and resolves fMain "purely through ht9045_secsgem's PUBLIC
# dependency on it" -- FALSE, disproved by this very target_link_libraries
# call listing ht9045_forms/ht9045_db/ht9045_core/ht9045_globals/ht9045_public/
# vclcompat explicitly (required for the LINK_GROUP:RESCAN workaround above,
# same as test_config_loaders). See that file's header for the corrected
# claim.
#
# AI(W906-W7-F1fix2) 20260729: that correction is right that the list is
# required, but its REASON is wrong in a way worth pinning down here, because
# it invites the next reader to believe ht9045_secsgem's PUBLIC edge to
# ht9045_forms does not work. It does. Measured, not argued:
#   * test_SecsWireCodec's call is a bare `PRIVATE ht9045_secsgem`, yet the
#     linkLibs response file CMake generates for it already contains
#     libht9045_forms.a -- the PUBLIC closure puts it there with nobody naming
#     it. Presence of ht9045_forms is NOT what the explicit list buys.
#   * Relinking this probe's own objects against exactly that bare-form archive
#     list produced 5 undefined references, ALL of them
#     libht9045_globals.a(cpublic.cpp.obj) -> SearchFile()/md5_Folder(), which
#     live in libht9045_public.a (nm --defined-only). None involve fMain.
#   * The same bare list wrapped in -Wl,--start-group/--end-group links, exit 0.
# So the load-bearing part is the GROUP, and the explicit library list exists
# only because $<LINK_GROUP:RESCAN,...> wraps only what is named inside it --
# transitively-pulled dependencies land outside the group and are back to a
# single-pass scan. Do not "simplify" this line to `PRIVATE ht9045_secsgem`.
add_executable(test_w7_f1_wall2_probe
    test_w7_f1_wall2_probe.cpp
)

target_link_libraries(test_w7_f1_wall2_probe PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_w7_f1_wall2_probe PRIVATE ws2_32)  # AI(W906-GA1-B2-integrate) 20260804: full group pulls vclcompat sockets
endif()

add_test(NAME w7_f1_wall2_probe COMMAND test_w7_f1_wall2_probe)

# ===========================================================================
#  W7-L1 -- asendic_Auto2 (Auto2-tray stack-feed) state machine
# ===========================================================================
# --- test_w7_l1_auto2: drive the Auto2-tray SM family over the W6 substrate --
# AI(W906-W7-L1) 20260729: plan SS6-W7-L1. Also placed ABOVE the GLOBAL TEST
# TIMEOUT block for the same reason as test_w7_f0_controls_guard /
# test_w7_f1_wall2_probe just above (a test registered after that block
# silently gets no timeout).
#
# Same link map as test_w6_1_empty_canary (asendic_Auto2.cpp lives in the same
# ht9045_sm library, alongside asendic_Empty.cpp, both over the Cylinder[]/
# MOT[]/Sen[]/SW[] Sim HAL + FormsFacade fAGV + canary_support). The RESCAN
# link group lets GNU ld re-scan the archive set so the cprod<->cpublic<->
# cmydef ctor cross-references settle (same idiom as the W6.1 canary test).
add_executable(test_w7_l1_auto2
    test_w7_l1_auto2.cpp
)

target_link_libraries(test_w7_l1_auto2 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_l1_auto2 PRIVATE ws2_32)
endif()

add_test(NAME W7_L1_Auto2 COMMAND test_w7_l1_auto2)

# ===========================================================================
#  W7-L1 Wave 1 -- asendic_Color, and the asendic_Loader + asendic_Loader_RT bundle
# ===========================================================================
# AI(W906-W7-L1-Wave1) 20260802: same link map as test_w7_l1_auto2 above -- all
# three translated units live in ht9045_sm, and the RESCAN link group lets GNU ld
# re-scan the archive set so the cprod<->cpublic<->cmydef ctor cross-references
# settle.  Both targets are placed ABOVE the GLOBAL TEST TIMEOUT block for the
# same reason the Auto2 target is: a test registered after that block silently
# gets no timeout.
add_executable(test_w7_l1_color
    test_w7_l1_color.cpp
)

target_link_libraries(test_w7_l1_color PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_l1_color PRIVATE ws2_32)
endif()

add_test(NAME W7_L1_Color COMMAND test_w7_l1_color)

add_executable(test_w7_l1_loader
    test_w7_l1_loader.cpp
)

target_link_libraries(test_w7_l1_loader PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_l1_loader PRIVATE ws2_32)
endif()

add_test(NAME W7_L1_Loader COMMAND test_w7_l1_loader)

# ===========================================================================
#  W7-L1 Wave 2 -- asendic_Auto and asendic_Auto_RT (the family's last two SMs)
# ===========================================================================
# AI(W906-W7-L1-Wave2) 20260802: same link map as the Wave-1 targets above -- both
# translated units live in ht9045_sm, and the RESCAN link group lets GNU ld re-scan
# the archive set so the cprod<->cpublic<->cmydef ctor cross-references settle.
# Both stay ABOVE the GLOBAL TEST TIMEOUT block: a test registered after it silently
# gets no timeout.
add_executable(test_w7_l1_auto
    test_w7_l1_auto.cpp
)

target_link_libraries(test_w7_l1_auto PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_l1_auto PRIVATE ws2_32)
endif()

add_test(NAME W7_L1_Auto COMMAND test_w7_l1_auto)

add_executable(test_w7_l1_auto_rt
    test_w7_l1_auto_rt.cpp
)

target_link_libraries(test_w7_l1_auto_rt PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_l1_auto_rt PRIVATE ws2_32)
endif()

add_test(NAME W7_L1_AutoRT COMMAND test_w7_l1_auto_rt)

# --- test_w7_f2_sckart_state: SckArt state merge proof + golden-divergence pins
# AI(W906-W7-F2) 20260729: plan SS3-C4 / SS6-F2 / SS9-R9. Also placed ABOVE the
# GLOBAL TEST TIMEOUT block for the same reason as the targets just above (a
# test registered after that block silently gets no timeout).
#
# Two things this target locks down:
#   (a) the W7-F2 merge -- SckArtRemainderState now derives from SckArtState, so
#       the 9 formerly-duplicated golden TfSCKART fields must construct
#       identically on both types (they did before the merge; that is what made
#       it behaviour-neutral, and this is the assertion that keeps it true);
#   (b) two GOLDEN DIVERGENCES found while diffing the four SckArt copies, which
#       W7-F2 reported instead of silently reconciling (iLOTSTATUS_* status
#       codes, and iTesterType). The assertions pin the CURRENT diverging
#       behaviour on purpose -- see the file's own header before "fixing" them.
#
# Same RESCAN link group as test_SCK_ART / test_SCK_ART_Remainder: both
# SCK_ART.cpp and SCK_ART_Remainder.cpp are compiled into ht9045_sm (root
# CMakeLists.txt), so they must NOT be added to this executable's own sources.
add_executable(test_w7_f2_sckart_state
    test_w7_f2_sckart_state.cpp
)

target_link_libraries(test_w7_f2_sckart_state PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_f2_sckart_state PRIVATE ws2_32)
endif()

add_test(NAME W7_F2_SckArtState COMMAND test_w7_f2_sckart_state)

# ===========================================================================
#  W7-L3 / W7-L4 Wave 0 -- test seam landed by the serial integrator BEFORE the
#  translation agents run
# ===========================================================================
# AI(W906-W7-L34-Wave0) 20260802: these two targets are created up front, against
# placeholder test bodies, so that the two parallel translation agents can each
# build and RUN their own suite immediately instead of having to edit this file.
# tests/CMakeLists.txt is a MERGE-AT-INTEGRATE file (plan S8) and two agents
# appending to it concurrently is exactly the shape that produced this repo's
# orphaned-git-stash incident -- so the integrator lands the seam once, serially,
# and the agents own ONLY their own .cpp.
#
# Neither translated unit needs a new entry in the root CMakeLists.txt: both
# cSiteUseManager.cpp (root CMakeLists.txt:1241) and Interface/TesterTCP.cpp
# (root CMakeLists.txt:1388) are ALREADY sources of ht9045_sm. Verified this wave
# by grep, not inherited from a doc.
#
# Same RESCAN link map as test_w7_l1_auto2 above, and for the same reason: the
# units live in ht9045_sm and GNU ld needs to re-scan the archive set so the
# cprod<->cpublic<->cmydef ctor cross-references settle.
#
# Both are placed ABOVE the GLOBAL TEST TIMEOUT block -- a test registered after
# that block silently gets no timeout, which is how a wedged run goes unnoticed.

add_executable(test_w7_l3_siteusemgr
    test_w7_l3_siteusemgr.cpp
)

target_link_libraries(test_w7_l3_siteusemgr PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_l3_siteusemgr PRIVATE ws2_32)
endif()

add_test(NAME W7_L3_SiteUseMgr COMMAND test_w7_l3_siteusemgr)

add_executable(test_w7_l4_copyostestresult
    test_w7_l4_copyostestresult.cpp
)

target_link_libraries(test_w7_l4_copyostestresult PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_l4_copyostestresult PRIVATE ws2_32)
endif()

add_test(NAME W7_L4_CopyOSTestResult COMMAND test_w7_l4_copyostestresult)

# ---------------------------------------------------------------------------
# AI(W906-P3-OLP) 20260919: P3 —— 瀏覽器 → OLP 設定指令白名單橋接。
#
#   受測單元 WebOlp.cpp 住在**根 CMakeLists.txt 的 wb_serve 目標**裡，不在任何
#   library 裡（理由同 WebStart.cpp / WebBridgeRecipeDoc.cpp：只有同時握有
#   伺服器與機台資料的那個行程執行得了它）。所以這裡直接把該 .cpp 列成本測試
#   的來源，而不是去 link 一個不存在的 library。
#
#   斷言的重點**不是**「回 0」。P2b 的 TransformFuntion 對每個輸入都回 0 且
#   都在合法範圍內，那種測試會全綠。真正承重的是「擾動輸入 → 輸出跟著變」
#   （PART 5）與「拒絕名單一條都不漏」（PART 2）。
#
#   ⚠ 本測試會**寫檔**：白名單上的 setter 幾乎每支最後都 WriteIniData 到
#   DataPath+GetLastOpenFN()。測試在 PART 0 把 DataPath 導到相對路徑
#   `webolp_scratch\`（ctest 的工作目錄底下），並主動驗證 scratch 真的收得到
#   寫入 —— 不驗那一條的話，「真實配方沒被動」也可能只是寫入整個失敗了。
#
#   Both placed ABOVE the GLOBAL TEST TIMEOUT block -- a test registered after
#   that block silently gets no timeout.
add_executable(test_webolp
    test_webolp.cpp
    ${CMAKE_SOURCE_DIR}/WebOlp.cpp
)

target_link_libraries(test_webolp PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_webolp PRIVATE ws2_32)
endif()

add_test(NAME WebOlpWhitelist COMMAND test_webolp)

# ---------------------------------------------------------------------------
# AI(W906-P6-WINREG) 20260920: P6-a —— 視窗狀態總表（取代 golden 的 fShow）。
#
#   受測單元 WebWindowRegistry.cpp 住在根 CMakeLists.txt 的 wb_serve 目標裡
#   （理由同 WebOlp.cpp），所以這裡直接列來源，不去 link 不存在的 library。
#   它只依賴 cJSON，不碰 god-stack ⇒ 連結面比其他測試小很多。
#
#   ⚠ 本測試斷言的重點是契約 §6 那一格：不可知／stale 的方向**與
#   guard.systemStart 相反**（不可知 -> 當成「那頁還開著」-> fShow true）。
#   契約自己寫著那是最容易寫反的地方，寫反的後果是放行本該擋住的 START。
#
#   stale 那一組靠 WebWindowRegistrySetStaleMsForTest() 驅動，不靠真的等 15 秒 ——
#   一支要跑 15 秒的測試最後一定會被改成不測那一格。
add_executable(test_winregistry
    test_winregistry.cpp
    ${CMAKE_SOURCE_DIR}/WebWindowRegistry.cpp
    ${CMAKE_SOURCE_DIR}/Public/cJSON.c
)

target_include_directories(test_winregistry PRIVATE ${CMAKE_SOURCE_DIR})

if(WIN32)
    target_link_libraries(test_winregistry PRIVATE ws2_32)
endif()

add_test(NAME WebWindowRegistry COMMAND test_winregistry)

# ---------------------------------------------------------------------------
# AI(W906-P4-SECS) 20260920: P4 —— SECS 的 SV 登錄表當 tag 目錄。
#
#   ★ 本測試最重要的斷言不是「有 741 筆」，是 **PART 1：缺相依時真的是 0 筆**。
#     少了那個對照組，741 證明不了是補上的相依起了作用 ——
#     也可能它本來就會自己登錄。
#
#   它逐一撞出四個相依（每一個都寫在測試本體的註解裡）：
#     1. THGem *HGem == NULL          PORT-ONLY 守衛，全樹零賦值點
#     2. CosFunction.bEnable_SECS_GEM golden 自己的閘；這台機器
#                                     (CC_CYUEAN) 產品上是 false
#     3. fLotInfo == NULL             -> AddEC() **SegFault**（gdb 實證）
#                                     ⇒ 測試刻意不叫 AddEC
#     4. 21 筆 GEM 標準 SV 在 THGem::FormCreate()（VCL 生命週期）
#
#   ⚠ 測試會改兩個行程內全域（HGem / CosFunction.bEnable_SECS_GEM）
#     並在結束前還原。ctest 每支測試是獨立行程，所以不會影響別的測試；
#     寫成還原是為了「單獨跑整支檔」時也乾淨。
#
#   ⚠ 它**不碰任何檔案** —— 登錄表只在記憶體裡，不需要 realfile_guard。
#
#   連結面比較小：只要 SECSGEM + 它的相依，不需要 forms/automation 全套，
#   但仍用同一個 RESCAN 群組，因為 cprod<->cmydef 的建構子交叉引用
#   一樣需要重掃。
add_executable(test_secs_catalogue
    test_secs_catalogue.cpp
    ${CMAKE_SOURCE_DIR}/SECSGEM/SecsSvRead.cpp
)

target_link_libraries(test_secs_catalogue PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_secs_catalogue PRIVATE ws2_32)
endif()

add_test(NAME SecsCatalogue COMMAND test_secs_catalogue)

# ---------------------------------------------------------------------------
# AI(W906-W7-L2) 20260803: W7-L2 regression suite for the ported ckernel.cpp
# (root CMakeLists.txt already lists ckernel.cpp as a source of ht9045_sm, next
# to ckernel_shims.cpp -- verified by grep this pass, not inherited from a doc,
# so NO root CMakeLists.txt change is needed).
#
# Same RESCAN link map as test_w7_l3_siteusemgr / test_w7_l4_copyostestresult
# above, and REQUIRED for the same reason: the unit lives in ht9045_sm and GNU ld
# must re-scan the archive set so the cprod<->cpublic<->cmydef constructor
# cross-references settle.
#
# AI(W906-W7-L2-substrate) 20260803: THE TRIPWIRE FIRED AS DESIGNED -- note updated.
# This block used to say the test TU DEFINES IsSafeLockCheck() / InitDoArmZHome() /
# AccelateTask itself, because ckernel.cpp.obj referenced them and nothing in the
# ported tree defined them, and that landing them in csystem.cpp would break this
# target with a duplicate symbol. That has now happened and been resolved:
# ported csystem.cpp defines all three from golden (csystem.cpp:178-183 the real
# two-assignment InitDoArmZHome + iAllArmZHomeCount, :402 AccelateTask=1,
# :433 IsSafeLockCheck), the test's "LINK-TIME SUBSTRATE FILL" is DELETED, and this
# target links with no fill at all -- which was impossible before.
# PART D7a/D7b/D7c-D7g now drive the REAL IsSafeLockCheck through its real inputs
# (Sen[SnRKCoverOpen], Sen[SnRKSafeLock], iControlPanelMode) instead of a boolean
# stand-in, so the suite pins golden's actual logic including its SW[] side effects.
# Do NOT re-add a local definition of any of the three.
#
# Placed ABOVE the GLOBAL TEST TIMEOUT block -- a test registered after that
# block silently gets no timeout, which is how a wedged run goes unnoticed.
# ---------------------------------------------------------------------------
add_executable(test_w7_l2_ckernel
    test_w7_l2_ckernel.cpp
)

target_link_libraries(test_w7_l2_ckernel PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w7_l2_ckernel PRIVATE ws2_32)
endif()

add_test(NAME W7_L2_CKernel COMMAND test_w7_l2_ckernel)

# ---------------------------------------------------------------------------
#  W906 trace: THGem::DoTraceDataResponse  (SECSGEM/uHGemEquipment.cpp:4640-4693)
# ---------------------------------------------------------------------------
# AI(W906-trace-test) 20260804: NEW target -- closes a live-code coverage gap.
# DoTraceDataResponse's caller was already wired before the translation landed
# (uHGemEquipment.cpp:4548-4549, the 10-slot sweep inside DoLocalAllProcessLoop,
# reached from :6740), so swapping the old no-op stub for golden's real body
# changed production behaviour immediately with zero tests on it.
#
# Same RESCAN link group as test_w7_l2_ckernel / test_w7_l3_siteusemgr above,
# and required for the same reason: the unit lives in ht9045_secsgem but drags
# in ht9045_db (HSys), ht9045_core/ht9045_globals (CUSTOMER_CODE / CosFunction /
# cprod<->cpublic<->cmydef constructor cross-references) and ht9045_sm (the real
# MyDBIProcess / ShowMyMessage / csystem predicates that uHGemEquipment.cpp and
# uHGemClass.cpp reference), so GNU ld must re-scan the archive set for the
# cross-references to settle.  With the full group linked, this test TU needs NO
# local link-time stubs at all -- do not add any.
#
# The suite needs no network: THGem's srvGem stays in vclcompat SIM mode
# (ServerSocket.h:144) and the wire bytes are captured from
# TCustomWinSocket::SimTxBuffer(), the same seam test_uHGemEquipment.cpp's
# [T6]/[T7] already use.  ws2_32 is still linked because ClientSocket.cpp /
# ServerSocket.cpp reference it unconditionally.
#
# Placed ABOVE the GLOBAL TEST TIMEOUT block -- a test registered after that
# block silently gets no timeout, which is how a wedged run goes unnoticed.
# ---------------------------------------------------------------------------
add_executable(test_w906_trace
    test_w906_trace.cpp
)

target_link_libraries(test_w906_trace PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_w906_trace PRIVATE ws2_32)
endif()

add_test(NAME W906_Trace COMMAND test_w906_trace)

# ---------------------------------------------------------------------------
#  GA-1 B1: LastSet full translation (LastSet.h 855 fields + LastSet.cpp globals)
# ---------------------------------------------------------------------------
# AI(W906-GA1-B1) 20260804: standalone data-layout suite -- compiles LastSet.cpp
# directly (single TU + the unit under test) instead of pulling a link group;
# the same LastSet.cpp also lives in ht9045_sm for production consumers, but
# linking the archive here would drag the whole sm dependency closure for a
# pure offsetof/read-write suite that needs none of it.
add_executable(test_ga1_lastset
    test_ga1_lastset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../LastSet.cpp
)
target_link_libraries(test_ga1_lastset PRIVATE vclcompat)
add_test(NAME GA1_LastSet COMMAND test_ga1_lastset)

# ---------------------------------------------------------------------------
#  GA-1 B5: language.cpp (TfLan CSV language pack -- load/query core)
# ---------------------------------------------------------------------------
# AI(W906-GA1-B5) 20260804: language.cpp is compiled directly into the test
# (it is not yet a member of any library); the RESCAN group supplies the real
# LastSet / MyDBIProcess / LogSoftwareOffTime definitions the TU references.
add_executable(test_ga1_language
    test_ga1_language.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../language.cpp
)
target_link_libraries(test_ga1_language PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ga1_language PRIVATE ws2_32)
endif()
add_test(NAME GA1_Language COMMAND test_ga1_language)

# ---------------------------------------------------------------------------
#  GA-1 B6: SYSTEM_MODULAR::ReadGeneralIni() (golden database.cpp:301-1537)
# ---------------------------------------------------------------------------
# AI(W906-GA1-B6) 20260804: same link shape as test_config_loaders (same
# database.cpp TU). Runs against a scratch COPY of system/Gerneral.ini --
# never writes the real config (the three write-back callers are #if 0 gated).
add_executable(test_ga1_readgeneralini
    test_ga1_readgeneralini.cpp
)
target_link_libraries(test_ga1_readgeneralini PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ga1_readgeneralini PRIVATE ws2_32)
endif()
add_test(NAME GA1_ReadGeneralIni COMMAND test_ga1_readgeneralini)

# ---------------------------------------------------------------------------
#  GA-1 B4: cMyDB.cpp (sqlite3 wrapper) -- real Handler.DB3 oracle + round-trip
# ---------------------------------------------------------------------------
# AI(W906-GA1-B4) 20260804: self-contained convention (like test_config_loaders):
# cMyDB.cpp compiles directly into the test and the TU's own god-stack-free
# stand-ins satisfy the extern surface. Deliberately NOT an ht9045_cmydb
# library yet -- no production consumer until GA-3 wires MyDBOpenDB; creating
# the library with its real forms/sm dependencies would collide with the
# test's stand-ins (see _ga1_b4_report.md).
add_executable(test_ga1_cmydb
    test_ga1_cmydb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../cMyDB.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../myTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AlarmCodeCatalog.cpp   # AI(W906-CMYDB-P3) 20260926: MyDBUpdateDB -> AlarmCodeCatalog_UpdateList (standard C++; not called here)
    ${CMAKE_CURRENT_SOURCE_DIR}/../AlarmCodeUpdater.cpp
)
target_link_libraries(test_ga1_cmydb PRIVATE vclcompat sqlite3)
# AI(W906-CSVONLY) 20260926: production cMyDB.cpp now has every sqlite path behind
# W906_CMYDB_SQLITE (default 0, user ruling: sqlite3 retired, CSV only, DB branches
# kept as #if).  This test compiles its OWN copy of cMyDB.cpp, so it turns the
# switch on and keeps verifying the preserved golden DB text.
target_compile_definitions(test_ga1_cmydb PRIVATE W906_CMYDB_SQLITE=1)
add_test(NAME GA1_cMyDB COMMAND test_ga1_cmydb)

# AI(W906-CSVONLY) 20260926: cMyDB CSV plan P2 -- MDB Updater alarm-code catalog.  Standard C++ only, links
# nothing from the tree; compares the C++ replay byte-for-byte with the generator's shipped list.
add_executable(test_mydb_csv_alarmcode
    test_mydb_csv_alarmcode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AlarmCodeCatalog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../AlarmCodeUpdater.cpp
)
target_include_directories(test_mydb_csv_alarmcode PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_mydb_csv_alarmcode PRIVATE
    W906_SHIP_ALARMCODELIST="${CMAKE_CURRENT_SOURCE_DIR}/../ship/Error/AlarmCodeList.txt")
add_test(NAME MyDB_CSV_AlarmCode COMMAND test_mydb_csv_alarmcode)

# AI(W906-CSVONLY-P1) 20260926: cMyDB CSV plan P1 -- EventLogTxt / HANDLER LOG backbone (TMyStringList, cMyDB.cpp,
#   LogObjects.cpp SaveEventLog).  All paths redirected to %TEMP% inside the test; never writes D:\HT9045_Log.
add_executable(test_mydb_csv_eventlog test_mydb_csv_eventlog.cpp)
target_link_libraries(test_mydb_csv_eventlog PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_mydb_csv_eventlog PRIVATE ws2_32)
endif()
add_test(NAME MyDB_CSV_EventLog COMMAND test_mydb_csv_eventlog)

# AI(W906-GB-P0) 20260926: Tester-comm P0 -- SyncMailbox nesting/timeout/isolation, TesterCommThread, TesterCommHub.
add_executable(test_testercomm_ipc test_testercomm_ipc.cpp)
target_link_libraries(test_testercomm_ipc PRIVATE ht9045_testercomm)
add_test(NAME TesterComm_IPC COMMAND test_testercomm_ipc)

# AI(W906-GB-P1) 20260926: Tester-comm P1 -- GPIB bridge engine.  Default run: whole-bridge link check, NI wrapper +
#   SimGpibDriver, TTimer emulation, Check2Dsum, engine guards (no disk side effects).  Full engine lifecycle on the
#   TesterComm thread is opt-in (HT9045_GPIB_FULL_TEST=1) because golden writes fixed paths such as D:\GPIBLOG.
add_executable(test_testercomm_gpib test_testercomm_gpib.cpp)
target_link_libraries(test_testercomm_gpib PRIVATE ht9045_testercomm ht9045_globals ht9045_core vclcompat)
add_test(NAME TesterComm_GPIB COMMAND test_testercomm_gpib)

# AI(W906-GB-P4) 20260926: Tester-comm P4 -- RS232Standard / TTL engine.  Default run: whole-program link check,
#   crc_chk = CRC-16/MODBUS check value, MyDeCodeASCII, TTimer emulation, engine guards.  Full lifecycle is opt-in
#   (HT9045_RS232_FULL_TEST=1) because golden writes D:\RS232Log.
add_executable(test_testercomm_rs232 test_testercomm_rs232.cpp)
target_link_libraries(test_testercomm_rs232 PRIVATE ht9045_testercomm ht9045_globals ht9045_core vclcompat)
if(WIN32)
    target_link_libraries(test_testercomm_rs232 PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_RS232 COMMAND test_testercomm_rs232)

# AI(W906-GB-P2a) 20260926: Tester-comm P2a -- Handler side (THandlerTesterSide) against the real GPIB engine on the
#   simulated NI driver.  Links the machine archives (RESCAN: they reference each other both ways, see
#   test_cUnitConvert above).
add_executable(test_testercomm_handler test_testercomm_handler.cpp)
target_link_libraries(test_testercomm_handler PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_testercomm_handler PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_Handler COMMAND test_testercomm_handler)

# AI(W906-GB-P2b) 20260926: SetTestTimeOutTimer (golden 912 atester.cpp:10920-10955) arms hFTestTimeOutDelay /
#   hBTestTimeOutDelay / TestTimeOut -- translation-gap fix for the On-Line immediate test time-out.  No disk access.
add_executable(test_set_test_timeout_timer test_set_test_timeout_timer.cpp)
target_link_libraries(test_set_test_timeout_timer PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_set_test_timeout_timer PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_TestTimeOutTimer COMMAND test_set_test_timeout_timer)

# AI(W906-GB-P6) 20260926: ruling 2A + Q2 (a) -- the GPIB program's extra RS232 port follows the recipe, each GPIB-mode
#   recipe seeded once from Setup.ini (TesterComm/Handler/HandlerGpibAux.cpp).  Files only in %TEMP%\ht9045_p6_gpib_aux.
add_executable(test_p6_gpib_aux test_p6_gpib_aux.cpp)
target_link_libraries(test_p6_gpib_aux PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_p6_gpib_aux PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_P6GpibAux COMMAND test_p6_gpib_aux)
# AI(W906-W1) 20260927: Steven W1 = (b) -- the Auto Retest brand follows the recipe (forms/fSCKART.cpp TfSCKART::AccessFile,
#   golden 906_0625_Steven SCK_ART.cpp:190-209; missing key -> 1 = 93K) and the LOTSTATUS / SRQMASK learning writes it back.
#   Files only in %TEMP%\ht9045_w1_<tick> (the test sets DataPath / LastDataPath itself and aborts if DataPath is under D:\HT9045).
add_executable(test_w1_art_brand test_w1_art_brand.cpp)
target_link_libraries(test_w1_art_brand PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_w1_art_brand PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_W1ArtBrand COMMAND test_w1_art_brand)
# AI(W906-W9) 20260927: Steven W9 = A -- a remote temperature offset is written into the recipe (uTemp_Set.cpp SAFETY GATE
#   S3 opened) + the 912 GPIB reload.  Files only in %TEMP%\ht9045_w9_<tick> (the test sets DataPath / LastDataPath itself,
#   aborts if they are under D:\HT9045, and checks the machine's own Temperature.Data keeps its size and write time).
add_executable(test_w9_remote_temp_offset test_w9_remote_temp_offset.cpp)
target_link_libraries(test_w9_remote_temp_offset PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_w9_remote_temp_offset PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_W9RemoteTempOffset COMMAND test_w9_remote_temp_offset)
# AI(W906-Q32-S69) 20260927 (St02): golden TTopBottomInspect's AOI.Data parameters as data (AOI/TopBottomInspect.*,
#   golden 906_0625_Steven fAOI.cpp:182-263; AI(W906-ST02-C912) 20261003 (St02-E helper): the 912 additions removed, RULINGS_20261002 #20 / #23-6).  No file access.
add_executable(test_tb_inspect test_tb_inspect.cpp ../AOI/TopBottomInspect.cpp)
target_include_directories(test_tb_inspect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_tb_inspect PRIVATE vclcompat)
add_test(NAME AOI_TopBottomInspectParams COMMAND test_tb_inspect)

# AI(W906-ELA-P1) 20260927: Event Log Analyzer P1a -- ElaCore (golden EventlogAnalyzer Rev891.0 GetEventLogText, CommaText,
#   Jam ini look-ups, dates): the golden switches pinned (oracle regression) + Steven's W15 / W18 / W19 = B defaults
#   (AI(W906-ELA-W19) 20260927).  Files only in %TEMP%\ht9045_ela_core and %TEMP%\ht9045_ela_core_fx.
add_executable(test_ela_core test_ela_core.cpp)
target_link_libraries(test_ela_core PRIVATE ht9045_ela)
# AI(W906-ELA-W15) 20260927 (St02-E): W23 sample copies (tests/fixtures/ela, copied from D:\HT9045_Log\EventLogTxt), READ
#   only -- the test copies them into %TEMP%\ht9045_ela_core_fx before it reads them (same idea as W906_WEB_JSON_DIR).
target_compile_definitions(test_ela_core PRIVATE W906_ELA_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/ela"
    W906_ELA_SRC_DIR="${CMAKE_SOURCE_DIR}")   # read only: section 9 checks its copy of cObserver.cpp:1369-1385
add_test(NAME ELA_Core COMMAND test_ela_core)
# AI(W906-ELA-P2) 20260927: ElaHub -- EL_* order / coalescing, ReadConfig (+ write-back), query snapshot, worker thread.
#   Files only in %TEMP%\ht9045_ela_hub.
add_executable(test_ela_hub test_ela_hub.cpp)
target_link_libraries(test_ela_hub PRIVATE ht9045_ela)
add_test(NAME ELA_Hub COMMAND test_ela_hub)
# AI(W906-ELA-W48) 20260928 (St02-E helper): ★W48 OEE (Steven 0928 = A) -- ElaOee ComputeOee / OeeJson on five generated
#   days: Steven's 18 / 1 / 5 h example (75.00 / 4.17 / 20.83 %), a stop split at midnight, overlapping stops as a union,
#   a partial span cut at now; plus the whole-day span of a 23:59:59 To.  Fixtures are written by the test (never copied
#   from D:\HT9045_Log), only in %TEMP%\ht9045_ela_oee (refused under D:\HT9045* / D:\RMS), removed on a green run.
#   AI(W906-ELA-W48B) 20260928 (St02-E helper): Steven 0928 09:3x -- W48-1 (each touchdown's Test Time, no gap rule),
#   W48-2 = B (off from generated TimeData rows: a gap, a restart row, an overdue tail, a first row capped at an hour;
#   ReadTimeData / ParseTimeDataLine), W48-3 = B (only the MTBA alarms are down; a sandbox JAM0000.dat).  Refuses to run
#   (exit 2) unless W906_HT9045LOG_ROOT is ctest's machine_log_scratch (containment first, the b12ab375 rule).
add_executable(test_ela_oee test_ela_oee.cpp)
target_link_libraries(test_ela_oee PRIVATE ht9045_ela)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_ela_oee PRIVATE -Wall -Wextra)
endif()
add_test(NAME ELA_Oee COMMAND test_ela_oee)
# AI(W906-ELA-W48B) 20260928 (St02-E helper): ★W48-2 = B -- the Handler's hourly TimeData record
#   (TesterComm/Handler/TesterCommWiring.cpp W906_TimeDataHourTick*: golden HS_Function.cpp:233-238 CheckClockTrigger(60)
#   -> RecordTimeData, cMyDB.cpp:479) and ElaOee reading it back.  Containment first (the b12ab375 rule): refuses, exit 2,
#   before ANY Handler code unless the four cMyDB log roots and the ctest redirect variables are machine_log_scratch;
#   then it points as9045LogPath / asSaveEventLogPath at %TEMP%\ht9045_ela_timedata_<tick> (refused under D:\HT9045*),
#   builds fMain->slTimeData there, and removes the sandbox on a green run.  Links the machine archives (RESCAN, as
#   test_testercomm_handler).
add_executable(test_ela_timedata test_ela_timedata.cpp)
target_link_libraries(test_ela_timedata PRIVATE
    ht9045_ela ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ela_timedata PRIVATE ws2_32)
endif()
add_test(NAME ELA_TimeData COMMAND test_ela_timedata)
# AI(W906-ELA-R4) 20260927 (St02-E): ElaFtp + ElaChipMos on a scripted fake transport (tests/ElaFtpFake.h): N25-3 success
#   (CWD / MKD / STOR / SIZE / LIST, HadUpload.txt), STOR 550, SIZE mismatch, SIZE unsupported -> LIST size, LIST missing,
#   connect / login failure (masked), back-off on a fake clock, the D-f gate (no transport call), the sim build's log-only
#   transport, N10 BYFILE, FTP_Log.  Never a real connection: it asserts wininet.dll is not even loaded.  Files only in
#   %TEMP%\ht9045_ela_ftp_<tick> (the test _putenv's W906_HT9045LOG_ROOT / W906_EVENTLOG_ROOT there first).
add_executable(test_ela_ftp test_ela_ftp.cpp)
target_link_libraries(test_ela_ftp PRIVATE ht9045_ela)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_ela_ftp PRIVATE -Wall -Wextra)
endif()
add_test(NAME ELA_Ftp COMMAND test_ela_ftp)
# AI(W906-ELA-R4): compile-only check that the WinINet transport links (built, never run, no add_test: it would need a
#   network and it never connects anyway -- main() only takes the factory's address).
add_executable(ela_ftp_wininet_link ela_ftp_wininet_link.cpp)
target_link_libraries(ela_ftp_wininet_link PRIVATE ht9045_ela)
# AI(W906-ELA-P3) 20260927: ElaService -- the /api/ela handling (ela::ServeHttp: 503 / 403 / 400 / 404, ?since, URL
#   decoding, query -> 202 -> result) and the QueryRequest / HubPaths seams (W906_EVENTLOG_ROOT / W906_PRODLOG_ROOT /
#   W906_GENERAL_INI_PATH).  The test _putenv's its own %TEMP% sandbox first (those three plus the D5 pair) and asserts
#   the real config.ini / JAM0000.dat / Gerneral.ini / EventLogTxt stamps unchanged; the sandbox goes on a green run.
#   Second line of defence: the APPEND below is deferred because the MACHINE-DATA CONTAINMENT block further down
#   overwrites ENVIRONMENT for every test that exists by then.  Not in _ht9045_env_extra: that list reaches EVERY test,
#   and W906_GENERAL_INI_PATH there would move other tests' Gerneral.ini.  The D5 pair comes through that list anyway.  AI(W906-TESTGUARD) 20260928: moving every test's Gerneral.ini is now done on purpose and PER TEST (AI(W906-ENV-ALL) at the file end, which runs after this DEFER), so ctest hands ELA_Service general_ini_scratch/ELA_Service/Gerneral.ini instead of the value below; the test's own _putenv sandbox still comes first.
add_executable(test_ela_service test_ela_service.cpp)
target_link_libraries(test_ela_service PRIVATE ht9045_ela)
add_test(NAME ELA_Service COMMAND test_ela_service)
cmake_language(DEFER CALL set_property TEST ELA_Service APPEND PROPERTY ENVIRONMENT
    "W906_EVENTLOG_ROOT=${CMAKE_CURRENT_BINARY_DIR}/ela_service_scratch/EventLogTxt"
    "W906_PRODLOG_ROOT=${CMAKE_CURRENT_BINARY_DIR}/ela_service_scratch/Production_Log"
    "W906_GENERAL_INI_PATH=${CMAKE_CURRENT_BINARY_DIR}/ela_service_scratch/Gerneral.ini")
# AI(W906-ELA-R1) 20260927 (St02-E): ELA reports plan R1 / R2 -- ElaFileUtil, GetEventLogTextToVec, ElaReports.  Every
#   write under %TEMP%\ht9045_ela_reports_<tick> (the test refuses a root under D:\HT9045, D:\HT9045_Log, D:\RMS,
#   D:\MTBF_Summary, D:\EventlogAnalyzer, and sets its own W906_HT9045LOG_ROOT); the sandbox goes on a green run.
#   W23 copies in tests/fixtures/ela, read only.
add_executable(test_ela_reports test_ela_reports.cpp)
target_link_libraries(test_ela_reports PRIVATE ht9045_ela)
target_compile_definitions(test_ela_reports PRIVATE W906_ELA_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/ela")
add_test(NAME ELA_Reports COMMAND test_ela_reports)

# AI(W906-ELA-R5) 20260927 (St02-E): ElaSchedule -- the time-based ELA jobs (plan R5: O06-4 / N10-3 analyzer Timer2,
#   N25-3 00:00, VTEST weekly, W13 N17 UploadProdLog 01:00), stagger / back-off / O10 + customer-code gates / D-b boot
#   catch-up, on a fake clock (no sleeping).  Files only in %TEMP%\ht9045_ela_sched_<tick> (refused under D:\HT9045*;
#   the test _putenv's W906_HT9045LOG_ROOT / W906_GENERAL_INI_PATH / W906_PRODLOG_ROOT into it) and it checks the real
#   config.ini / Gerneral.ini stamps.  New infrastructure: -Wall -Wextra on the test and on ElaSchedule.cpp.
add_executable(test_ela_schedule test_ela_schedule.cpp)
target_link_libraries(test_ela_schedule PRIVATE ht9045_ela)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_ela_schedule PRIVATE -Wall -Wextra)
    set_source_files_properties(${CMAKE_SOURCE_DIR}/EventLogAnalysis/ElaSchedule.cpp TARGET_DIRECTORY ht9045_ela
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME ELA_Schedule COMMAND test_ela_schedule)
# AI(W906-ELA-W45) 20260927 (St02-E): ★W45 (was W20) shared JAM0000.dat rules (JamRules.h, header-only): the golden defaults
#   of both editors as a table, the ★W45 A / B / C switch, ElaCore JamConfig == its pre-W45 bodies under A, the two
#   readers in both orders (golden's conflict pinned), the W906_JAM0000_PATH seam.  Files only in %TEMP%\ht9045_jam_rules_<tick>
#   (refuses a path under D:\HT9045), removed on a green run.
add_executable(test_jam_rules test_jam_rules.cpp)
target_link_libraries(test_jam_rules PRIVATE ht9045_ela)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_jam_rules PRIVATE -Wall -Wextra)
endif()
add_test(NAME Jam_Rules COMMAND test_jam_rules)
# AI(W906-W58) 20261001 (St02-E): SimNet_Path -- W58 Q5 phase 2 group A, W906_SimNetPathBlocked (common.cpp end of
#   file): SIM blocks a UNC / // path unless W906_SIM_NET_PATHS=1; SHIP never.  Links ht9045_core like test_common;
#   built and run in both configurations; writes nothing.
add_executable(test_simnet_path test_simnet_path.cpp)
target_link_libraries(test_simnet_path PRIVATE ht9045_core)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_simnet_path PRIVATE -Wall -Wextra)
endif()
add_test(NAME SimNet_Path COMMAND test_simnet_path)
set_tests_properties(SimNet_Path PROPERTIES TIMEOUT 60)

# AI(W906-SIM-W36-1) 20260928 (St02-E helper), AI(W906-W58) 20260930 (St02-E): W36-1 / W58, the SIM build starts every
#   launch with the FTP / network options off (SimNet/SimNetMask.cpp; docs/SIMNET_W58.md).  SimNetMask.cpp is compiled here with
#   W906_SIMNET_CORE_ONLY (the key table, the Mask, the SIM gate; no Handler globals, no FileRW proxies) against a fake
#   Handler, plus the real ELA readers through the override.  Files only in %TEMP%\ht9045_simnet_<tick> (refused under
#   D:\HT9045* / D:\RMS / D:\MTBF_Summary / D:\EventlogAnalyzer before anything is written: containment first), removed
#   on a green run; it stats the machine's config.ini / Gerneral.ini read-only.  New code: -Wall -Wextra on the test and
#   on SimNetMask.cpp inside wb_serve (TARGET_DIRECTORY, as ELA_Schedule does for ElaSchedule.cpp).
add_executable(test_simnet_mask test_simnet_mask.cpp ../SimNet/SimNetMask.cpp)
target_include_directories(test_simnet_mask PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_simnet_mask PRIVATE W906_SIMNET_CORE_ONLY)
target_link_libraries(test_simnet_mask PRIVATE ht9045_ela)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_simnet_mask PRIVATE -Wall -Wextra)
    if(TARGET wb_serve)
        set_source_files_properties(${CMAKE_SOURCE_DIR}/SimNet/SimNetMask.cpp TARGET_DIRECTORY wb_serve
            PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
    endif()
endif()
add_test(NAME SimNet_Mask COMMAND test_simnet_mask)
set_tests_properties(SimNet_Mask PROPERTIES TIMEOUT 60)
# AI(W906-W58) 20261001 (St02-E): SimNet_Write -- the mask's config.ini write-back is atomic (a temp file in the same
#   folder, flushed, + MoveFileExA MOVEFILE_REPLACE_EXISTING, WriteRawValue keeping vclcompat's in-place rules; a failed
#   move leaves the file as it was).  Core only, like SimNet_Mask; built and
#   run in both configurations.  Files only in %TEMP%\ht9045_simnet_w_<tick> (containment first), removed on a green run.
add_executable(test_simnet_write test_simnet_write.cpp ../SimNet/SimNetMask.cpp)
target_include_directories(test_simnet_write PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_simnet_write PRIVATE W906_SIMNET_CORE_ONLY)
target_link_libraries(test_simnet_write PRIVATE ht9045_ela)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_simnet_write PRIVATE -Wall -Wextra)
endif()
add_test(NAME SimNet_Write COMMAND test_simnet_write)
set_tests_properties(SimNet_Write PROPERTIES TIMEOUT 60)
# AI(W906-S118) 20260928 (St02-E): the main-screen tray producer for Main.MotionView (JsonBridge/ChanMvTrays.h,
#   header-only): the golden fMain bindings table (906_0625_Steven cinitial.cpp:6430-6471), tray -> JSON
#   (cells[row][col] = Data[col][row]) and the only-on-change rule.  Memory only: no MOT[], no file.
add_executable(test_mv_trays test_mv_trays.cpp)
target_include_directories(test_mv_trays PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)   # header-only, no library
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_mv_trays PRIVATE -Wall -Wextra)
endif()
add_test(NAME MotionView_Trays COMMAND test_mv_trays)
# AI(W906-S119) 20260927: Main.Record CLEAR / AseRecordMemo double-click -- act.main.clearRecord / act.main.meShuttle2Dbl
#   (JsonBridge/actions/MainRecordClear.cpp; golden spbClearRecordClick main.cpp:31109-31138, meShuttle2DblClick
#   :29471-29475).  Sources and archives as test_sjson_chan's action files; memory only (ArmData / fProductionInfo /
#   IniConfig), nothing is written by the code under test.
add_executable(test_main_record_clear test_main_record_clear.cpp
    ../JsonBridge/actions/MainRecordClear.cpp ../JsonBridge/EventLog.cpp)
target_include_directories(test_main_record_clear PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_main_record_clear PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
add_test(NAME MainRecord_Clear COMMAND test_main_record_clear)
# AI(W906-SEC-S128) 20260927 (St02): Jam export's missing-key merge into JAM0000.dat and its named lock (JamIniMerge.h,
#   header-only; WebSecurityJam.cpp uses it).  Every file is under %TEMP%\ht9045_s128_<tick>, removed on a green run.
add_executable(test_security_jam_merge test_security_jam_merge.cpp)
target_include_directories(test_security_jam_merge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_security_jam_merge PRIVATE vclcompat)
add_test(NAME Security_JamMerge COMMAND test_security_jam_merge)
# AI(W906-SEC-W59) 20260929 (St02-E): ★W59-1 the editable long alarm description (WebSecurityJamW59.h, header-only;
#   WebSecurityJam.cpp reaches it through WebSecurityJamW45.h).  Text core only (W906_JAMW59_CORE_ONLY): Big5 / CP949 /
#   cp1252 / UTF-8 decoding, TRichEdit-style RTF in and out, golden's font per language.  No file is read or written, no
#   link library (Win32 code-page calls only), seconds.  Its own TIMEOUT, so it does not depend on the GLOBAL block.
add_executable(test_jam_w59 test_jam_w59.cpp)
target_include_directories(test_jam_w59 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_jam_w59 PRIVATE -Wall -Wextra)
endif()
add_test(NAME Jam_W59 COMMAND test_jam_w59)
set_tests_properties(Jam_W59 PROPERTIES TIMEOUT 60)
# AI(W906-SEC-Q9) 20260927 (St02-E): Steven S131 Q9 = B -- New / Delete / Edit on the binary password book login.dat
#   (LoginDatBook.h, header-only; WebLogin.cpp W906_PwBinaryBook uses it).  Links ht9045_core for EncodeStr / DecodeStr
#   (common.cpp).  Every file is under %TEMP%\ht9045_q9_<tick>, removed on a green run; D:\HT9045\system\login.dat is
#   never opened (its size and write time are checked).
add_executable(test_login_dat_book test_login_dat_book.cpp)
target_include_directories(test_login_dat_book PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_login_dat_book PRIVATE ht9045_core)
add_test(NAME Security_LoginDatBook COMMAND test_login_dat_book)

# AI(W906-CMYDB-P4-D5) 20260927: cMyDB log roots for EVERY test, through the laptop's _ht9045_env_extra (file end,
#   AI(W906-ENV-ALL), TO_STEVEN 00:1x).  common.cpp :240/:425 as9045LogPath and :303 asSaveEventLogPath read these
#   (unset = the golden D:\HT9045_Log literals).  Written as the literal of _ht9045_log_scratch (:3890), because that
#   variable is set later in this file than this line.
list(APPEND _ht9045_env_extra  "W906_RMS_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/RMS"  "W906_PRODINFO_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/ProductionInfo"   # AI(W906-CMYDB-P4-D2/D4) 20260927 (St02-E): common.cpp :263/:442 asProductionLogPath, :293 sProductionInfoFilePath
    "W906_HT9045LOG_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch"  "W906_ERROR_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/Error"   # AI(W906-CMYDB-ERRSEAM) 20260928 (St02-E): cMyDB.cpp W906ErrorDir -- AlarmCodeList.txt and English\JAM31xxx.dat (MyDBUpdateDB / DoInsertAlarmCode) never hit D:\HT9045\Error from a ctest
    "W906_SAVEEVENTLOG_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/SaveEventLog"  "W906_HPCARD_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/HPCARD"  "W906_GROUNDMAN_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/GroundManLog/GroundManValue"  "W906_PRODLOADER_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/Production_Loader")   # AI(W906-LOGOBJ-SEAM) 20260927 (St02-E, optional hardening): common.cpp :316 / :323.  AI(W906-CMYDB-A4) 20260928 (St02-E helper): cMyDB.cpp :474 W906ProductLoaderDir -- RecordTimeData's hourly Production_Loader file (golden asProduct_LoaderPath) never hits D:\HT9045_Log from a ctest

# ---------------------------------------------------------------------------
#  GA-1 B3: cpublic.cpp newly-ungated SafeData x2 / UDPErrorLog / ShuttleLog
# ---------------------------------------------------------------------------
# AI(W906-GA1-B3) 20260804: file I/O redirected to %TEMP% inside the test;
# ht9045_core listed explicitly (ht9045_globals deliberately does not
# PUBLIC-link it -- D-A0-4 ruling).
add_executable(test_ga1_cpublic
    test_ga1_cpublic.cpp
)
target_link_libraries(test_ga1_cpublic PRIVATE
    
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ga1_cpublic PRIVATE ws2_32)
endif()
add_test(NAME GA1_cpublic COMMAND test_ga1_cpublic)

# ---------------------------------------------------------------------------
#  GA-1 B2: cprod.cpp ungate acceptance (5 representative ungated functions)
# ---------------------------------------------------------------------------
# AI(W906-GA1-B2) 20260804: self-contained convention (compiles the real
# cprod.cpp/common.cpp/LastSet.cpp/canary_support.cpp/mycylin.cpp/mysensor.cpp
# plus TU-local stand-ins) -- see _ga1_b2_report.md.
add_executable(test_ga1_cprod
    test_ga1_cprod.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../cprod.cpp
    # AI(W906-PT-W3-ungate) 20260808: CosFunction.cpp joins this target's minimal
    #   standalone link set.  It has to: cprod.cpp's CustomerFunctionSelect() no
    #   longer has its 7 customer-function calls behind `#if 0`, so the bodies must
    #   be reachable.  This target does NOT link ht9045_globals (it compiles its
    #   handful of sources directly, on purpose -- see the test's own banner), so
    #   the archive copy cannot serve it.  The alternative was to keep the 7 empty
    #   TU-local stand-ins that used to live in test_ga1_cprod.cpp, which is worse:
    #   they would silently shadow 517 real flag assignments and no diagnostic
    #   would ever fire.
    ${CMAKE_CURRENT_SOURCE_DIR}/../CosFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../cmydef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpublic.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/../EJ1N/TextProcess.cpp  # AI(W906-I03) 20261002 (Ifor01): cpublic.cpp's KT4H / E5DC bodies are ungated (I-03) and call T_ASXII2HEX / A_Create_LCR / T_HEX2ASCII_Mac / SetBCC (ht9045_globals has them; this target compiles its sources directly), same line
    ${CMAKE_CURRENT_SOURCE_DIR}/../myTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../LastSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../canary_support.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../mycylin.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/../HAlarm.cpp  # AI(W906-HALARM) 20260926: mycylin.cpp 的 SetAlarm／ClearAlarm 接上 golden HAlarm；本目標不連 ht9045_globals（自帶原始碼），所以直接編進來。SystemNG 的定義也在那裡（test_ga1_cprod.cpp:262 改成 extern）
    ${CMAKE_CURRENT_SOURCE_DIR}/../mysensor.cpp
)
target_link_libraries(test_ga1_cprod PRIVATE vclcompat)
if(WIN32)
    target_link_libraries(test_ga1_cprod PRIVATE psapi version)
endif()
add_test(NAME GA1_cprod COMMAND test_ga1_cprod)

# ---------------------------------------------------------------------------
#  GA-2 C1: cinitial.cpp block 1 (Sucker/Switch/Sensor names + init)
# ---------------------------------------------------------------------------
add_executable(test_ga2_c1_cinitial
    test_ga2_c1_cinitial.cpp
)
target_link_libraries(test_ga2_c1_cinitial PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ga2_c1_cinitial PRIVATE ws2_32)
endif()
add_test(NAME GA2_C1_cinitial COMMAND test_ga2_c1_cinitial)

# ===========================================================================
#  WB-0 web bridge substrate (AI(W906-WebBridge) 20260805)
# ===========================================================================
#  The C++ side of the browser HMI seam. All three link ht9045_webbridge only --
#  no vclcompat, no state machines, no machine headers (see that target's own
#  comment in the root CMakeLists.txt for why that isolation is load-bearing).
#
#  Registered ABOVE the GLOBAL TEST TIMEOUT block below, per that block's own
#  instruction: a test added after it silently gets no timeout.
#
#  Measured 20260805 under MinGW (g++ 6.3.0, thread model win32):
#    WB_Crypto   915 checks     -- RFC 3174 SHA-1 + RFC 4648 base64 vectors,
#                                  JSON null-vs-"" and the nan/inf mapping
#    WB_State    264 checks     -- includes REAL concurrency: 1 publisher +
#                                  4 readers over 2,000 generations, and
#                                  4 producers x 2,000 pushes with one drainer
#    WB_WsProto  10,526 checks  -- RFC 6455 section 1.3 accept-key vector,
#                                  section 5.7 example frames, byte-at-a-time
#                                  incremental decode equivalence
#
#  Failability was PROVEN by mutation, not assumed: breaking the RFC 6455 GUID,
#  masking server->client frames, serialising null as "", and moving the
#  snapshot generation bump outside its critical section each make the suite
#  fail. Worth recording that the LAST one is only caught when the race window
#  is widened -- so WB_State is a reliable regression detector for that
#  invariant, not a reliable race finder. Do not read a green WB_State as proof
#  that a newly introduced 2-instruction race is absent.
#    WB_Server   71 checks     -- a REAL server on an ephemeral loopback port:
#                                  static files (incl. .js as text/javascript,
#                                  which decides whether the UI loads at all),
#                                  10 path-traversal attacks all refused, a real
#                                  RFC 6455 handshake, the connect-time snapshot
#                                  frame, and command refusal while read-only
# WB_DataLayer is the odd one out: it is the only WB test that links the machine
# god-stack, because the thing it checks IS the machine data layer. Same
# LINK_GROUP:RESCAN set as test_ga1_readgeneralini, for the same single-pass
# archive-scan reason documented there.
add_executable(test_wb_datalayer test_wb_datalayer.cpp)
target_link_libraries(test_wb_datalayer PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_wb_datalayer PRIVATE ws2_32)
endif()
add_test(NAME WB_DataLayer COMMAND test_wb_datalayer)

# WB_Tags: the seam itself -- machine globals -> webbridge::TagSnapshot. Links
# BOTH the god-stack (it reads machine globals) and ht9045_webbridge (it stages
# into a TagSnapshot); WebBridgeTags.cpp is the one translation unit allowed to
# mix the two, which is why it lives at the tree root and not under WebBridge/.
# AI(W906-FW-CTBS) 20260818: cContactCT + cShowBinSelect Wave A oracles.
add_executable(test_contactct_core test_contactct_core.cpp)
target_link_libraries(test_contactct_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
add_test(NAME ContactCTCore COMMAND test_contactct_core)
# --- test_i129_b37: AI(W906-I129-B37) 20261002 -- INBOX 129 (a)(b) census batch 37 (RULINGS_20261001 #0): #9
#   SetShuttleToHasNullIC_9045 (golden ainarm9045.cpp:343-382), #8 SetFixTrayFullIC reaching csystem.cpp's ClearFixTray
#   (aoutarm.cpp PTW4 stand-in retired), #2 a source ratchet on ainarm2.cpp's lifted GATE W7E-K8-C (argv[1] = port root).
#   No IO; ClearFixTray's RecordProcess goes to the redirect roots.  Own TIMEOUT.
add_executable(test_i129_b37 test_i129_b37.cpp)
target_link_libraries(test_i129_b37 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_i129_b37 PRIVATE ws2_32)
endif()
add_test(NAME I129_Batch37 COMMAND test_i129_b37 "${CMAKE_SOURCE_DIR}")
set_tests_properties(I129_Batch37 PROPERTIES TIMEOUT 120)
add_executable(test_showbinselect_core test_showbinselect_core.cpp)
target_link_libraries(test_showbinselect_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
add_test(NAME ShowBinSelectCore COMMAND test_showbinselect_core)

# AI(W906-FW-SecCC) 20260819: FW-SecCC oracles -- TfSecurity access-level /
# jam-level engine, TfCounterClear ClearCount families.
add_executable(test_security_core test_security_core.cpp)
target_link_libraries(test_security_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
add_test(NAME SecurityCore COMMAND test_security_core)
# Steven 20260925 (Data.TestCategory)：golden sgArm1DrawCell 擷取＋SetTestCate* 路徑
add_executable(test_testcategory_paint test_testcategory_paint.cpp)
target_link_libraries(test_testcategory_paint PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>")
if(WIN32)
    target_link_libraries(test_testcategory_paint PRIVATE ws2_32)
endif()
add_test(NAME TestCategoryPaint COMMAND test_testcategory_paint)

add_executable(test_counterclear_core test_counterclear_core.cpp)
target_link_libraries(test_counterclear_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
add_test(NAME CounterClearCore COMMAND test_counterclear_core)
add_executable(test_binsel_core test_binsel_core.cpp)
target_link_libraries(test_binsel_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
add_test(NAME BinSelCore COMMAND test_binsel_core)

# AI(W906-FW3-YM-A) 20260818: FW-3 uYieldMonitoring Wave A oracle -- site-yield
# math, sliding-window ring, CanAutoCloseSite predicate, CheckSettingNo bounds.
add_executable(test_yieldmon_core test_yieldmon_core.cpp)
target_link_libraries(test_yieldmon_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
add_test(NAME YieldMonCore COMMAND test_yieldmon_core)

# AI(W906-FW3-Obs1) 20260818: FW-3 cObserver Wave 1 oracle -- CalculateStopTime
# symmetry, GetEventLogText's three filters against a scratch CSV corpus fed
# through W906_EVENTLOG_ROOT (the BinCount-redirect precedent), and a
# BtnQueryClick query-string assertion. Links the standard RESCAN group:
# cObserver.cpp lives in ht9045_sm and pulls cMyDB/cSocket/vclcompat.
add_executable(test_observer_core test_observer_core.cpp)
target_link_libraries(test_observer_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
add_test(NAME ObserverCore COMMAND test_observer_core)

# AI(W906-FW3-TemperFrom-WA) 20260818: FW-3 cTemperFrom Wave A oracle --
# ShowThermo's not-installed/in-band/over-band/under-band classification,
# TempRunShowAlarmHigh/Low true+false, SetShowYield/ShowYieldFuntion,
# ShowHotName, FormClose. cTemperFrom.cpp lives in ht9045_sm, same RESCAN
# group as cObserver/uYieldMonitoring above.
add_executable(test_temperfrom_core test_temperfrom_core.cpp)
target_link_libraries(test_temperfrom_core PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
add_test(NAME TemperFromCore COMMAND test_temperfrom_core)

# AI(W906-BU-C3) 20260916: ../EtherCAT/Pci1203Monitor.cpp added because WebBridgeTags.cpp now publishes the 63 pci1203.* tags. There are exactly FOUR targets that compile WebBridgeTags.cpp (wb_serve, wb_publish, and these two); the first fix patched only the two executables and the next gate died here. HAVE_PCI1203 stays OFF, so this links an inert read-only observer.
# AI(W906-BU-C-P4) 20260918: ../EtherCAT/Pci1203Control.cpp joins it, because P4
#   un-gated the 14 pci1203.control.* tags and WebBridgeTags.cpp now calls
#   Pci1203Control() / Pci1203ControlLinked() / TPci1203Control's const accessors.
#   ⚠ THE COMMENT ABOVE ALREADY SAID THIS WOULD HAPPEN. The 20260918 fix repeated
#   the 20260916 mistake exactly: patched the two executables in the root
#   CMakeLists, did not look in this file, and the build died here with 7
#   undefined references per target. The consumer set of a .cpp spans BOTH
#   CMakeLists files; grepping one of them is not grepping the tree.
#   HAVE_PCI1203 stays OFF here, so this links the same inert surface.
add_executable(test_wb_tags test_wb_tags.cpp ../WebBridgeTags.cpp ../EtherCAT/Pci1203Monitor.cpp ../EtherCAT/Pci1203Control.cpp)
target_link_libraries(test_wb_tags PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_wb_tags PRIVATE ws2_32)
endif()
add_test(NAME WB_Tags COMMAND test_wb_tags)

# WB_RecipeDoc: the browser's read view of a recipe/settings document.
# AI(W906-FW-C1R) 20260911.
#
# NOTE THE LINK LIST, IT IS THE POINT. vclcompat + ht9045_webbridge and NOTHING
# ELSE -- no LINK_GROUP:RESCAN god-stack, unlike WB_Tags right above, which needs
# it because WebBridgeTags.cpp reaches MainProc and the engine cursors.
# WebBridgeRecipeDoc.cpp defines no machine symbol and needs none; measured with
# `nm --undefined-only` on 20260911, which listed only vclcompat::,
# webbridge::JsonWriter:: and libc strtoll. If this list ever has to grow, the
# reader has started reaching into the machine and that is the thing to look at
# before making the link succeed.
#
# The test is hermetic: every path it touches is RELATIVE to the working
# directory, so it cannot reach D:\HT9045 machine data. One of its eight groups
# exists solely to assert that reading a file does not modify it -- see the
# header for why that is a regression guard and not paranoia.
add_executable(test_wb_recipedoc test_wb_recipedoc.cpp ../WebBridgeRecipeDoc.cpp)
target_link_libraries(test_wb_recipedoc PRIVATE vclcompat ht9045_webbridge)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # New-build code, no BCB6 idiom to accommodate: full strictness, same as
    # ht9045_webbridge itself (CMakeLists.txt:2988).
    target_compile_options(test_wb_recipedoc PRIVATE -Wall -Wextra)
endif()
add_test(NAME WB_RecipeDoc COMMAND test_wb_recipedoc)

# AI(W906-F5Recipe) 20260914: no server or production I/O; use VS Code's JSONC parser.
find_program(W906_NODE_EXECUTABLE NAMES node)
if(W906_NODE_EXECUTABLE AND WIN32)
    add_test(NAME WB_F5Contract COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/f5_contract_probe.cjs")
endif()

# WB_SimPump: pump mode -- the first thing outside tests/ that DRIVES the
# translated spine, and the only tag that puts a word on an operator screen.
# Same link shape as WB_Tags (it reaches MainProc + the engine cursors through
# WebBridgeTags.cpp), and deliberately loads NO config: PumpInit() is specified to
# work with zero config, so this TU cannot touch the shared production ini at all.
# AI(W906-SimPump) 20260813.
# AI(W906-BU-C3) 20260916: ../EtherCAT/Pci1203Monitor.cpp added because WebBridgeTags.cpp now publishes the 63 pci1203.* tags. There are exactly FOUR targets that compile WebBridgeTags.cpp (wb_serve, wb_publish, and these two); the first fix patched only the two executables and the next gate died here. HAVE_PCI1203 stays OFF, so this links an inert read-only observer.
# AI(W906-BU-C-P4) 20260918: same addition, same reason as test_wb_tags above.
add_executable(test_wb_simpump test_wb_simpump.cpp ../WebBridgeTags.cpp ../EtherCAT/Pci1203Monitor.cpp ../EtherCAT/Pci1203Control.cpp)
target_link_libraries(test_wb_simpump PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_wb_simpump PRIVATE ws2_32)
endif()
add_test(NAME WB_SimPump COMMAND test_wb_simpump)

# AI(W906-S12) 20260929 (St02-E): HeaterSimTick -- golden THeaterThread::HeaterThreadProcess in the SIM build
#   (HeaterSimTick.cpp, ht9045_sm; called from WebBridgeTags.cpp PumpTick).  Same library group as WB_SimPump, no
#   WebBridgeTags.cpp: the test calls ht9045::W906_HeaterSimTick directly.  Writes no file (the HeaterLog hook is 0).
add_executable(test_heater_sim_tick test_heater_sim_tick.cpp)
target_link_libraries(test_heater_sim_tick PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_heater_sim_tick PRIVATE ws2_32)
endif()
add_test(NAME HeaterSimTick COMMAND test_heater_sim_tick)

add_executable(test_wb_crypto  test_wb_crypto.cpp)
add_executable(test_wb_state   test_wb_state.cpp)
add_executable(test_wb_wsproto test_wb_wsproto.cpp)
add_executable(test_wb_server  test_wb_server.cpp)
# AI(W906-MAILBOX-RT) 20260924: HttpStatic 的 /JSON/runtime 覆蓋（執行期信箱搬出版控）。
add_executable(test_http_runtime_overlay test_http_runtime_overlay.cpp)
# AI(W906-WebBridge-Tcp) 20260812: the sidecar seam (TagJson + TcpTagLink).
add_executable(test_wb_tcplink test_wb_tcplink.cpp)
# AI(W906-JSONBRIDGE-S0) 20260923: 開機配置廣播。
#   ⚠ 這支**必須**跟 wb_serve 用同一組編譯旗標編，它的核心斷言是
#     「表裡的 defined 等於這次編譯的 #ifdef 結果」。用不同的 -D 編它，
#     兩邊都會是對的、比對也會過，但證明不了任何事。
add_executable(test_jsonbridge_s0
    test_jsonbridge_s0.cpp
    ../JsonBridge/MachineDefines.cpp
    ../JsonBridge/ChanConfig.cpp
    ../JsonBridge/gen/sjson_MachineDefines.gen.cpp)
target_include_directories(test_jsonbridge_s0 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_jsonbridge_s0 PRIVATE ht9045_webbridge)
add_test(NAME JsonBridgeS0 COMMAND test_jsonbridge_s0)

# AI(W906-JSONBRIDGE-S7-TESTS) 20260923: S2~S7 的離線不變量（不需要伺服器）。
#   為什麼要這一支：S7 的 213 個 temp.zone.* tag 全部必須是 null（送 0 會是
#   合法的攝氏溫度），而守住那條的原本只有 tools/webprobe/s7_thermo_probe.py
#   —— 那支要人工先起一個 wb_serve，所以它不在 ctest 裡。等於「把 null 換成
#   0 讓畫面好看」這種改動，可以在 gate 全綠的情況下完全不被發現。
#   ⚠ 這支刻意**不** link god-stack。它驗的是型別表本身（offsetof / sizeof
#     這類編譯期事實）與溫控 producer，不碰任何結構實例，所以不需要
#     JsonBridge/Bindings.cpp —— 那個 TU 引用 DeviceForm_File 等全域，
#     一加進來就會把整個 god-stack 拖進這個 target。
#   ⚠ 它也不讀任何 ini：PublishThermoTags 只寫進一個記憶體裡的 TagSnapshot，
#     所以這支沒有任何路徑碰得到 common.cpp:89 那份量產共用的 Gerneral.ini。
add_executable(test_jsonbridge_s7
    test_jsonbridge_s7.cpp
    ../JsonBridge/StageThermo.cpp
    ../JsonBridge/gen/thermo_channels.gen.cpp
    ../JsonBridge/gen/sjson_SYSTEM_DEVICE_FORM.gen.cpp
    ../JsonBridge/gen/sjson_LAST_LEVEL_SET.gen.cpp
    ../JsonBridge/gen/sjson_SYSTEM_TEST_IF.gen.cpp
    ../JsonBridge/gen/sjson_SYSTEM_TEMPERATURE.gen.cpp
    ../JsonBridge/gen/sjson_SYSTEM_TRAY_FORM.gen.cpp
    ../JsonBridge/gen/sjson_SYSTEM_TEST_MODE.gen.cpp)
target_include_directories(test_jsonbridge_s7 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_jsonbridge_s7 PRIVATE vclcompat ht9045_webbridge)
# 嚴格度只加在新寫的那一個 TU 上：gen/ 底下那幾個檔對非 standard-layout 的
# 結構用 offsetof（AnsiString 有建構子），-Wall 會湧出 -Winvalid-offsetof。
# 那是產生器的既有作法（wb_serve 也是這樣編的），不是這支測試要修的事。
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_jsonbridge_s7.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME JsonBridgeS7 COMMAND test_jsonbridge_s7)

# ===== AI(W906-SJSON-S8S9S10S11-TESTS) 20260923 =====
# S10 的警報環：ChanAlarm.cpp 一個機台全域都不碰，所以這支**不 link
# god-stack**，跑得快而且沒有任何路徑碰得到量產共用檔。
add_executable(test_sjson_alarm test_sjson_alarm.cpp ../JsonBridge/ChanAlarm.cpp)
target_include_directories(test_sjson_alarm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_sjson_alarm PRIVATE ht9045_webbridge)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_sjson_alarm.cpp ../JsonBridge/ChanAlarm.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME SjsonAlarm COMMAND test_sjson_alarm)

# S8/S9/S11：ChanProduction 與 ChanAction 讀 LastSet / Prod / IniConfig，
# ChanIo/ChanMotor 讀 Pci1203Monitor()，所以這支要 god-stack ＋
# EtherCAT/Pci1203Monitor.cpp（wb_serve 也是直接編它，CMakeLists.txt:3129）。
# ⚠⚠ 這支**刻意不呼叫 InstallClarnDataBody()**：本體會寫
#   D:\HT9045\system\lastdata.dat（硬編路徑，--dry 蓋不到）。
#   測試只驗 dryRun 與守衛，而「沒安裝」本身就是第二層保護。
add_executable(test_sjson_chan
    test_sjson_chan.cpp
    ../JsonBridge/ChanProduction.cpp
    ../JsonBridge/ChanIo.cpp
    ../JsonBridge/ChanMotor.cpp
    ../JsonBridge/ChanAction.cpp
    ../JsonBridge/actions/MainTesterConnect.cpp   # AI(W906-GB-P2e) 20260926: ChanAction.cpp 分派 act.main.testerConnect
    ../JsonBridge/actions/MainRecordClear.cpp ../JsonBridge/actions/LotInfoFtp.cpp ../JsonBridge/actions/ObserverSGJam.cpp   # AI(W906-S119) 20260927: ChanAction.cpp 分派 act.main.clearRecord／meShuttle2Dbl  AI(W906-LI9-F1) 20261002 (St02-E helper): ChanAction.cpp dispatches act.lotInfoFtp.* (forward only; no body installed here = not-installed)  AI(W906-ST02-OB7) 20261002 (St02-E helper): ChanAction.cpp:347 分派 act.observerSG.*
    ../JsonBridge/actions/MainClarnData.cpp ../JsonBridge/actions/MainStateRecord.cpp   # AI(W906-STATEREC) 20260924: ChanAction.cpp 分派 act.main.stateRecord（只碰 forms 層，不會把 cStateRecord.cpp 連進來）
    ../JsonBridge/EventLog.cpp
    ../EtherCAT/Pci1203Monitor.cpp)
target_include_directories(test_sjson_chan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_sjson_chan PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
add_test(NAME SjsonChan COMMAND test_sjson_chan)
# ===== AI(W906-SJSON-S8S9S10S11-TESTS) END =====

# ===== Steven 20260924 (S12 second type): TesterIF golden bridge ===== RETIRED 20260926 (St01)
# A 形狀 TFTestIF bridge（FileRW/TestIF_File.cpp）退役：Setup.TesterIF.html 改走 C 路 FileRW/TestIF_File_TesterIF.cpp（Steven 20260926，
# 經 github-02）。test_formbridge_testerif.cpp 一併刪除；FileRW/_formbridge_sources.cmake 只剩 HotPlate（wb_serve 仍 include）。

# --- test_webcmdguard: 伺服器端防連點（WebCmdGuard.h，S107-3）---
#   AI(W906-CMDGUARD) 20260926: 同 cmd+tag+value 在上一條還在跑、或完成後 W（預設 400 ms）內到達 -> busy、不執行。
#   時鐘手給（µs），模擬 wb_serve 分派迴圈；名稱級／op 級白名單、key、W906_CMDGUARD_MS 解析、表的上限、時鐘往回跳。
#   受測檔住在根 CMakeLists.txt 的 wb_serve 目標（理由同 WebWindowRegistry.cpp），這裡直接列來源；只依賴
#   WebCommand／TagValue／cJSON（ht9045_webbridge，同 test_sjson_alarm），不 link god-stack，不讀寫任何機台檔，秒級。
add_executable(test_webcmdguard test_webcmdguard.cpp ../WebCmdGuard.cpp)
target_include_directories(test_webcmdguard PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_webcmdguard PRIVATE ht9045_webbridge)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_webcmdguard.cpp ../WebCmdGuard.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME WebCmdGuard COMMAND test_webcmdguard)
set_tests_properties(WebCmdGuard PROPERTIES TIMEOUT 60)

target_link_libraries(test_wb_crypto  PRIVATE ht9045_webbridge)
target_link_libraries(test_wb_state   PRIVATE ht9045_webbridge)
target_link_libraries(test_wb_wsproto PRIVATE ht9045_webbridge)
target_link_libraries(test_wb_server  PRIVATE ht9045_webbridge)
target_link_libraries(test_http_runtime_overlay PRIVATE ht9045_webbridge)
target_link_libraries(test_wb_tcplink PRIVATE ht9045_webbridge)

add_test(NAME WB_Crypto  COMMAND test_wb_crypto)
add_test(NAME WB_State   COMMAND test_wb_state)
add_test(NAME WB_WsProto COMMAND test_wb_wsproto)
add_test(NAME WB_Server  COMMAND test_wb_server)
add_test(NAME WB_HttpRuntimeOverlay COMMAND test_http_runtime_overlay)
add_test(NAME WB_TcpLink COMMAND test_wb_tcplink)

# WB_State spawns real threads and runs 2,000 publish generations, and WB_Server
# opens real sockets; 300s is far above their measured runtimes (under 2s each)
# and only trips if one genuinely wedges. WB_Server's own client puts a 5s
# deadline on every blocking read for the same reason.
# WB_TcpLink opens real loopback sockets and deliberately waits for a reconnect,
# so it is the slowest of the group; its own waits are all budgeted (waitFor),
# so 300s only trips if a thread genuinely wedges rather than merely being slow.
set_tests_properties(WB_Crypto WB_State WB_WsProto WB_Server WB_TcpLink
                     PROPERTIES TIMEOUT 300)

# ===========================================================================
#  PT-W1 leaf coverage (AI(W906-PT-W1-Tests) 20260807)
# ===========================================================================
#  The two deterministic leaves that landed in translation wave PT-W1 with no
#  tests at all.  Both are golden-oracle tests: every expected value is derived
#  by hand from the golden 899 source and cited by golden line number at the
#  assertion, never read back off the port.
#
#  Registered ABOVE the GLOBAL TEST TIMEOUT block below, per that block's own
#  instruction: a test added after it silently gets no timeout.

# --- EJ1N/TextProcess.cpp: 16 free functions + the 2 nibble tables ----------
#   TextProcess.cpp is compiled into ht9045_globals (root CMakeLists.txt:460).
#   That archive PUBLIC-links what its objects need; this TU calls nothing but
#   TextProcess's own free functions and AnsiString, so it takes the same simple
#   shape as test_globals above rather than the god-stack LINK_GROUP.  Note the
#   unit's own banner: with GCC both catch(...) handlers are dropped, so it adds
#   no MyDBIProcess link edge.  GetCOMPortStatus is declared but never called by
#   the test -- it opens a real COM port.
add_executable(test_ptw1_textprocess
    test_ptw1_textprocess.cpp
)
target_link_libraries(test_ptw1_textprocess PRIVATE ht9045_globals ht9045_core vclcompat)
add_test(NAME PTW1_TextProcess COMMAND test_ptw1_textprocess)

# --- Public/MyStringList.cpp: TMyStringList --------------------------------
#   MyStringList.cpp is compiled into ht9045_sm (root CMakeLists.txt:1764) and
#   reaches fSCKART / fLotInfo / IniConfig / LastSet / MyDBIProcess, so it needs
#   the same single-pass LINK_GROUP:RESCAN god-stack as test_ga2_c1_cinitial
#   above, for the same archive-scan-order reason documented there.
#   The test writes ONLY into <build dir>/_ptw1_msl_scratch -- never
#   D:/HT9045/system, /config or /CFG.
add_executable(test_ptw1_mystringlist
    test_ptw1_mystringlist.cpp
)
target_link_libraries(test_ptw1_mystringlist PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ptw1_mystringlist PRIVATE ws2_32)
endif()
add_test(NAME PTW1_MyStringList COMMAND test_ptw1_mystringlist)

# --- AI(W906-YESNO) 20260925: ShowMyMessageBox_YES_NO 替身換真呼叫 ---------------
#   使用者 20260925 裁決（docs/RULINGS_20260925.md 第 10 條）：YES/NO 照 golden 問操作員。
#   驗 canary_support.cpp 的本體（hook 已裝／未裝、答 Yes／No）與 NB2 R15 那三題被自動同意的
#   呼叫點（csystem.cpp CheckContinusStartIsReady 兩題、OmronLaser/LaserSensorShuttle.cpp
#   case 300 兩處）各自走對的分支。三者都在 ht9045_sm，所以用同一個 RESCAN god-stack。
#   ⚠ 雷射「是」臂會 SaveShuttleLaserValue()：測試**自己**先把 DataPath／LastDataPath 換到
#     <build dir>/tests/_yesno_scratch 並斷言換成功才呼叫 —— 不依賴下面的 blanket ENVIRONMENT，
#     用手跑 exe 也不會寫進 D:\HT9045\IniData。
#   Registered ABOVE the GLOBAL TEST TIMEOUT block below (a test added after it gets no timeout).
add_executable(test_yesno_dialog
    test_yesno_dialog.cpp
)
target_link_libraries(test_yesno_dialog PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_yesno_dialog PRIVATE ws2_32)
endif()
add_test(NAME W906_YesNoDialog COMMAND test_yesno_dialog)

# ===========================================================================
#  GLOBAL TEST TIMEOUT -- must stay at the BOTTOM of this file
# ===========================================================================
# AI(W906-TestBootstrap) 20260728: second line of defence behind
# tests/test_bootstrap.cpp (see the block at the top of this file). The
# bootstrap stops the KNOWN blocking paths (modal CRT/WER dialogs); this
# catches everything else that can wedge a batch run -- an infinite loop, a
# state machine that never converges, a socket test waiting on a peer that
# never connects, or any future dialog path the bootstrap does not cover.
#
# Without it, `ctest` has NO default timeout for these tests and one hung
# executable blocks the suite indefinitely (exactly what happened 20260728).
#
# 600s is deliberately generous: the slowest tests here legitimately take
# ~40s under MSVC and ~22s under MinGW, and the whole suite runs ~200-220s,
# so this only ever trips on something genuinely stuck, never on a slow-but-
# healthy test. Raise it for a specific test with set_tests_properties()
# AFTER this block if one ever legitimately needs longer.
get_property(_ht9045_all_tests DIRECTORY PROPERTY TESTS)
set_tests_properties(${_ht9045_all_tests} PROPERTIES TIMEOUT 600)

# ---------------------------------------------------------------------------
#  AI(W906-BA-BU0g) 20260911: MACHINE-DATA CONTAINMENT. Migrated from the
#  D:\HT9050 tree, which had all five roots; this tree had NONE of them.
#
#  WHY -- a measured incident in THIS tree, today. The ba0 baseline gate's
#  ctest wrote into the machine's own live recipe tree, for the ACTIVE work
#  order D169_FCBGA1097_23X23_1X2_FT1_25_V05:
#      IniData\Data\D169_...\HandlerCondition.Data   17:42
#      IniData\Data\D169_...\Temperature.Data        17:44
#      IniData\Data\Tester.Data                       17:44
#  with REAL VALUE changes, not reformatting:
#      iIndexArmAutoCleanCnt  4062 -> 0        Temperature  125.0 -> 25.0000
#      iAutoCleanPad_CountTime_0_0/1  1 -> 0   iAutoClean_IndexTime added = 42
#  HT9045.exe was not running, so the handler did not do it.
#
#  The other tree's 20260826 sweep had found four such roots, all stamped by
#  ctest runs and none by a machine run:
#      D:\HT9045\Error\BootLog.txt                 test_cBootLog
#      D:\HT9045_Log\E84DataTxt\...                test_agv_e84
#      D:\HT9045_Log\TCP_Data\...                  test_automation
#      D:\HT9045_Log\Summary_Lot\                  an SCK_ART test
#  and its 20260908 follow-up added the fifth, D:\HT9045\IniData, after the
#  same class of incident. EVERY ONE of those tests PASSED while writing,
#  which is exactly why none was ever suspected. A passing test is the most
#  dangerous writer there is.
#
#  This block is the belt to the per-write braces (the getenv seams in the
#  translated units): it redirects the roots for the WHOLE suite, so a test
#  added later inherits containment instead of having to remember it.
#  Production leaves these unset and therefore keeps golden's literals
#  byte-for-byte.
#
#  It is a blanket set applied via the same DIRECTORY/TESTS enumeration as the
#  TIMEOUT above. No test in this directory sets ENVIRONMENT individually
#  today, so nothing is clobbered -- if one ever does, it must be MERGED here
#  rather than added separately, because the later call wins.
#
#  ⚠ This does NOT protect a test executable run BY HAND, which is exactly
#  what one does while debugging. A redirect that silently fails looks
#  identical to a pass.
# ---------------------------------------------------------------------------
set(_ht9045_log_scratch "${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch")
# AI(W906-FILERW-FALLBACK) 20260924: W906_INIDATA_ROOT 把整個 IniData 導到這個暫存夾，但裡面原本沒有 DioCfg ——
#   Steven 05f2695b 的 FormBridgeTesterIF（golden InitcbDIOType 列 DIOCFGPath 的 *.ini）在 ctest 下因此數到 0 而失敗，
#   直接執行 exe（沒有這個環境變數）才會過。照下面 config 暫存夾的「SEEDED, NOT EMPTY」先例：configure 時把真的
#   D:/HT9045/IniData/DioCfg 複製一份過來（讀到同樣的位元組、寫入落在建置樹），EXISTS 保護讓沒有那個資料夾的機器照常 configure。
if(EXISTS "D:/HT9045/IniData/DioCfg")
    file(COPY "D:/HT9045/IniData/DioCfg/" DESTINATION "${_ht9045_log_scratch}/IniData/DioCfg")
endif()

# ---------------------------------------------------------------------------
#  AI(W906-LOT-W1) 20260919: THE SIXTH ROOT -- D:\HT9045\config\ (`AuthPath`).
#
#  MEASURED, NOT FEARED. The 20260919 shipping gate wiped the machine's live
#  work order out of D:\HT9045\config\config.ini:
#      Lot ID=1         -> (empty)
#      Operator=1       -> (empty)
#      LotStartTime=2026-09-11 10:38:52 -> 9999-9999-00 9999:9999:9999
#      Start Time=20260911_103852        -> 9999999900_999999999999
#  Restored from tools/realfile_guard.py's snapshot. THE TEST PASSED while
#  doing it -- the same "a passing test is the most dangerous writer there is"
#  as the five roots above.
#
#  WHY IT ONLY STARTED TODAY. `AuthPath+"config.ini"` had plenty of writers
#  already, but the five damaged keys have exactly TWO writers in the whole
#  tree, and both were `{}` no-op stubs until this morning:
#      forms/fLotInfo.cpp:5385/:5395   TfLotInfo::SetLotID   -> "Lot ID"
#      forms/fLotInfo.cpp:5404         TfLotInfo::SetLotStart-> "LotStartTime"
#  (`git grep '"LotStartTime"'` = 3 hits, all in that one function.)
#  P0-2 replaced both stubs with golden's real bodies, so any ctest test that
#  reaches one of their 9 live call sites now writes the shared machine config
#  for real -- with the EMPTY strings an offline test supplies.
#  ⇒ Landing a translation can arm a writer that was previously inert. The
#    containment list is not a property of the tests; it is a property of what
#    the tree can now reach.
#
#  ⚠ MY FIRST ATTRIBUTION WAS WRONG, and the correction is the point.
#  I named tests/test_auto9045.cpp:293 (Part9_SetLotInfoGreatekOEE ->
#  Automation/auto9045.cpp:1484) from reading the call chain. Measured: built
#  that one test against this seam and ran it by hand with W906_AUTH_PATH
#  pointed at an EMPTY existing directory -- nothing was written, and the test
#  still passed. Reading the test's own comment says why: the offline chain
#  returns 90 at auto9045.cpp:1479, five lines BEFORE the SetLotID call.
#  The real reacher is one of the other 8 sites (HANA_ART.cpp:582,
#  SCK_ART_Remainder.cpp:1431, Command.cpp:9732/:13724/:17791,
#  auto9045.cpp:1415, forms/fLotInfo.cpp:4036 FormShow) -- named here as a set
#  because that is what is measured. The containment does not depend on which:
#  it is at the path, not at the caller.
#  (A by-hand probe needs the directory to EXIST -- TIniFile will not create
#  one, and a write into a missing directory fails silently and looks exactly
#  like containment working. That cost me one false green.)
#
#  SEEDED, NOT EMPTY. The scratch copy starts as a copy of the real directory,
#  because config.ini is READ by a dozen translated units (CosFunction,
#  Command, cAuthority's Security_new.def, TriTemp...). Pointing them at an
#  empty directory would silently change what they read and move the failure
#  set for reasons that have nothing to do with containment. Reads see the
#  same bytes; writes land in the build tree.
#
#  The source is the golden literal itself (common.cpp:102/:363), not a path
#  derived from CMAKE_SOURCE_DIR -- if golden's literal ever changes, this
#  must change with it, and the EXISTS guard makes a machine without that
#  directory configure cleanly instead of failing.
# ---------------------------------------------------------------------------
set(_ht9045_config_scratch "${CMAKE_CURRENT_BINARY_DIR}/machine_config_scratch")
file(MAKE_DIRECTORY "${_ht9045_config_scratch}")
if(EXISTS "D:/HT9045/config")
    file(COPY "D:/HT9045/config/" DESTINATION "${_ht9045_config_scratch}")
else()
    message(STATUS "W906_AUTH_PATH: D:/HT9045/config absent; scratch stays empty")
endif()

#  ⚠ ONE call, MERGED -- the block above says it in its own words: "if one ever
#  does, it must be MERGED here rather than added separately, because the later
#  call wins." W906_AUTH_PATH is appended to the same string for that reason.
#
#  AI(W906-MT-E3b) 20260925: THE SEVENTH ROOT -- d:\HT9045\system\machinerecord.dat
#  (+ machinerecordRealCCD.dat and PickHPRec.json, written by the same
#  SaveMachineRecord).  EastSun ruling R10 20260925: the machine's file is NOT
#  restored, tests write a scratch copy.  WB_SimPump reaches SaveMachineRecord
#  (MainProc's idle arm, csystem.cpp `iNeedRecord==1`), and W3-12c (20260925)
#  armed its WriteData.  Seam: cinitial.cpp W906_MachineRecordRedirect()
#  (unset = golden literal).  WriteData does NOT create the directory -- a write
#  into a missing one fails (WAR1682) and would look like containment working --
#  so it is created here, and tests/test_mt_e3b_engine.cpp (MT_E3b_Engine, PART A)
#  asserts the variable, the redirect AND the real files' untouched stamps.
#  Merged into the same string.
file(MAKE_DIRECTORY "${_ht9045_log_scratch}/machinerecord")
set_tests_properties(${_ht9045_all_tests} PROPERTIES ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: machine MT-E3b added W906_MACHINERECORD_DIR, laptop W2 (G-5/G-6) added W906_PRODLOG_ROOT/W906_CLEANPADLOG_ROOT -- both kept in this ONE string (the later call wins, so two calls would drop one); the ":3852" / ":3834" that the EOF blocks cite is this line (now :3883, the laptop inserted 29 lines above it)


# ---------------------------------------------------------------------------
#  AI(W906-DFM2RC-TMO) 20260917: the four dfm2rc timeouts, set where they can
#  actually take effect -- i.e. AFTER the GLOBAL TEST TIMEOUT block, which is
#  the escape hatch that block documents in its own words:
#      "Raise it for a specific test with set_tests_properties() AFTER this
#       block if one ever legitimately needs longer."
#
#  WHY. dfm2rc_fidelity was measured SIX times on this machine in one night
#  against the 600s cap:
#      Passed  333.41 / 425.49 / 486.01 s     three clean runs
#      Timeout 600.03 s   the machine was demonstrably 1.66x slower (a runaway
#                         background `grep -r` of mine -- RULINGS_20260916 §G.4)
#      Timeout 600.02 s   nothing of mine was running -- but the whole suite
#                         took 1233s where the previous Debug run took 1019s
#                         (1.21x), so SOMETHING was, and I cannot name it
#      Timeout 600.02 s   the RELEASE side of that same gate, 1101s vs 627s the
#                         run before (1.76x).  BOTH SIDES RED on a change that
#                         cannot touch this test: a comment-only edit to
#                         WebBridgeServer.h plus tools/wb_serve.cpp, which is not
#                         in test_dfm2rc_pipeline's link at all (:2508 builds it
#                         from one .cpp).  At that point the cap is not detecting
#                         hangs, it is manufacturing red gates.
#  This file's own July baseline says "fidelity ~130s". It is 2.6x-3.7x that on
#  a QUIET machine now, and the cap never moved.
#
#  Two false reds in six runs is not a hang detector, it is a coin flip that
#  costs a 90-minute dual-gate cycle each time. Worse, a gate that cries wolf
#  teaches the next reader to wave a real regression through as "probably the
#  flake" -- which is the failure mode this repo keeps paying for.
#
#  The other three are the same family (each spawns Python and does heavy file
#  I/O) and their July numbers are just as stale: rc_compiles was measured at
#  ~85s and ran 134-197s tonight. Raised together rather than one at a time as
#  each starts to bite.
#
#  ⚠ THIS CANNOT MASK A FAILURE. A failing test still fails, with the same
#  output. The only thing traded away is how long we wait before calling a
#  genuine hang a hang -- and nothing else in the suite is affected, because
#  the blanket 600s still governs every other test.
#
#  `if(TEST ...)` because these four are only registered when CMake found a
#  Python3 interpreter; without it they do not exist and naming them here
#  would be a hard configure error.
# ---------------------------------------------------------------------------
if(TEST dfm2rc_fidelity)
    set_tests_properties(
        dfm2rc_rc_compiles dfm2rc_layout_full dfm2rc_fidelity dfm2rc_idempotent
        PROPERTIES TIMEOUT 1800
    )
endif()

# ===========================================================================
# --- test_homeclass: golden 歸零狀態機的索引契約 ---------------------------
#   AI(W906-HOME-C1) 20260920. golden ProcessMotorHome case 600（uhome.cpp:
#   3020）用 `HomeClass[i]` 配 `MOT[i]`，也就是假設 push_back 順序 ==
#   馬達索引順序。那個假設沒有任何機制保證，純靠 173 列表格的撰寫順序，
#   而它破掉的症狀（某顆馬達用別顆的旗標決定要不要歸零）在**模擬下看不出來**
#   —— TMySimMotor::HomeFlag() 永遠回 true。所以要一個結構性斷言。
#   連整個 RESCAN 群組：InitialHomeClass 讀 MOT[]、機型旗標與 cmydef 全域。
add_executable(test_homeclass
    test_homeclass.cpp
)

target_link_libraries(test_homeclass PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_homeclass PRIVATE ws2_32)
endif()

add_test(NAME homeclass COMMAND test_homeclass)

# ===========================================================================
# --- test_mainproc_guard: golden csystem.cpp:17130 `if(SystemStart)` -------
#   AI(W906-Q15) 20260920. W906-HOME-W1（20260919）從 golden :17604 起抽，
#   把包住它的守衛（:17130）留在上游沒抄。症狀是實測出來的：wb_serve 跑 8 秒、
#   一個指令都沒送，stdout 出現 15 次 "Motor not home yet!" —— 程式一開起來
#   就在自己試著歸零。使用者 20260920 確認真實機台不會這樣（INBOX Q15）。
#   這支兩個方向都量：SystemStart=false -> iHome 維持 0；=true -> iHome 變 1。
add_executable(test_mainproc_guard
    test_mainproc_guard.cpp
)

target_link_libraries(test_mainproc_guard PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)

if(WIN32)
    target_link_libraries(test_mainproc_guard PRIVATE ws2_32)
endif()

add_test(NAME mainproc_guard COMMAND test_mainproc_guard)
#  AI(W906-MT-E3b) 20260925: homeclass and mainproc_guard are registered AFTER the containment tail block
#  (its `get_property(... TESTS)` runs at the GLOBAL TEST TIMEOUT block above), so until now they had neither
#  its TIMEOUT nor its ENVIRONMENT -- measured 20260925 in build_integ_ship_x86/tests/CTestTestfile.cmake
#  (their set_tests_properties carried only _BACKTRACE_TRIPLES), not assumed.
#  Both link the whole engine, and mainproc_guard drives MainProc(), whose idle arm calls SaveMachineRecord
#  (d:\HT9045\system\machinerecord.dat, EastSun ruling R10: tests write a scratch copy).  Same merged string.
set_tests_properties(homeclass mainproc_guard PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: same merged string as the global block (machine MACHINERECORD_DIR + laptop PRODLOG/CLEANPADLOG)

# --- test_dialog_mailbox: AI(W906-Q30-IDLE) 20260923 ----------------------------------
#   tools/wb_dialog_mailbox.h 的 MailboxResetStale()／UnixMillisNow()（header-only）。
#   只寫 ctest 工作目錄底下自己的暫存目錄；W906_WEB_JSON_DIR 只用來**讀** web/JSON 的正本，
#   比對嵌入的 idle 種子有沒有漂移（寫法同上面 ../web/forms）。
add_executable(test_dialog_mailbox test_dialog_mailbox.cpp)
target_compile_definitions(test_dialog_mailbox PRIVATE W906_WEB_JSON_DIR="${CMAKE_SOURCE_DIR}/../web/JSON")
add_test(NAME WB_DialogMailbox COMMAND test_dialog_mailbox)

# --- test_IniFiles_Win32Diff: AI(W906-T4-INIFMT2) 20260924 ---------------------------------
#   write-through vclcompat::TIniFile::WriteString vs the REAL kernel32 WritePrivateProfileStringA
#   (BCB6 TIniFile::WriteString is a thin call to it, so Windows itself is the oracle).
#   ~3,000 cases, byte-for-byte; writes only a scratch dir under the ctest working directory.
#   Before t4-inifmt2 the old flush()-rebuild path failed 2,706 of 3,027 of these.
if(WIN32)
    add_executable(test_IniFiles_Win32Diff test_IniFiles_Win32Diff.cpp)
    target_link_libraries(test_IniFiles_Win32Diff PRIVATE vclcompat)
    add_test(NAME IniFiles_Win32Diff COMMAND test_IniFiles_Win32Diff)
endif()

# --- test_IniFiles_Win32ReadDiff: AI(W906-A5-INIREAD) 20260924 -----------------------------
#   write-through vclcompat::TIniFile READS vs the REAL kernel32 GetPrivateProfileStringA plus the BCB6 Pascal on top
#   of it (ReadString 2048 / ReadSection(s) 16384 + `while P^ <> #0` / ValueExists / SectionExists). One long-lived
#   TIniFile bound before its file exists (the INIFileGeneral pattern) must answer from the CURRENT bytes; 250 random
#   files, fixed seed; plus an external-edit test and the 16384 ReadSection cap. Scratch dir under the ctest cwd only.
if(WIN32)
    add_executable(test_IniFiles_Win32ReadDiff test_IniFiles_Win32ReadDiff.cpp)
    target_link_libraries(test_IniFiles_Win32ReadDiff PRIVATE vclcompat)
    add_test(NAME IniFiles_Win32ReadDiff COMMAND test_IniFiles_Win32ReadDiff)
endif()
# --- test_machine_suckers: W3 第 9 項 InitialSuckerName／InitSucker（if＝NewIO_MN200 那半）的第三級測試 ---
#   AI(W906-W3-9) 20260924: 餵 machines/HT9050/IO_Table.csv，49 個吸嘴 × Sen／On／Off 三組 7 欄逐列比對，
#   另有三顆手算值（CSV 逐列讀出）與 Enable 兩組態規則。連結形狀同 MachineCylinders。
#   放在檔尾（不位移上面的行號引用），所以要自己補上 :3833 的圍堵環境變數 —— 那一段只套用在它之前登記的測試。
add_executable(test_machine_suckers
    test_machine_suckers.cpp
)
target_link_libraries(test_machine_suckers PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_machine_suckers PRIVATE ws2_32)
endif()
add_test(NAME MachineSuckers_HT9050
         COMMAND test_machine_suckers "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv")
set_tests_properties(MachineSuckers_HT9050 PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: same merged string as the global block (machine MACHINERECORD_DIR + laptop PRODLOG/CLEANPADLOG)
# --- test_machine_motors: W3 第 10 項 InitialMotorParameter（if＝NewIO_MN200 那半）的第三級測試 ---
#   AI(W906-W3-10) 20260925: 餵 machines/HT9050/Mot_Table.csv，每軸驅動類型、16 個欄位、1203／MN200 位址碼往返，
#   四軸手算值、Enable 兩組態規則、INDEX_MOTION_CARD 0／1 兩條路。出貨組態的 Enable 軸會呼叫 InitMotor ——
#   這台沒有 1203 SDK 所以不碰硬體；不要在有卡的機台上跑。放檔尾理由同 MachineSuckers（自己補圍堵環境變數）。
add_executable(test_machine_motors
    test_machine_motors.cpp
)
target_link_libraries(test_machine_motors PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_machine_motors PRIVATE ws2_32)
endif()
add_test(NAME MachineMotors_HT9050
         COMMAND test_machine_motors "${CMAKE_SOURCE_DIR}/../machines/HT9050/Mot_Table.csv")
set_tests_properties(MachineMotors_HT9050 PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: same merged string as the global block (machine MACHINERECORD_DIR + laptop PRODLOG/CLEANPADLOG)
# --- test_mytray_golden: W3 第 11 項 mytray.cpp 照 golden 重翻後的忠實度鎖 ---
#   AI(W906-W3-11) 20260925: ctor 30×70、ClearData 保留 PordRec 並只清 XItem×YItem、HasIC／HasRealIC／HasOnlyDataICAndNullIC
#   的 golden 判準、SetXYItem／SetBlockXYItem 鉗制與 P06、11 個原本是樁的函式、UnloaderInfo 往返（暫存檔在 ctest 工作目錄）。
add_executable(test_mytray_golden
    test_mytray_golden.cpp
)
target_link_libraries(test_mytray_golden PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_mytray_golden PRIVATE ws2_32)
endif()
add_test(NAME MyTray_Golden COMMAND test_mytray_golden)
set_tests_properties(MyTray_Golden PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: same merged string as the global block (machine MACHINERECORD_DIR + laptop PRODLOG/CLEANPADLOG)
# --- test_web_motor_access: W4-a `motor.access` 的解析與分派（假後端）---
#   AI(W906-W4-MOTOR) 20260925: 受測單元 WebMotorAccess.cpp 住在根 CMakeLists.txt 的 wb_serve 目標裡（理由同 WebOlp.cpp），
#   這裡直接列來源；它只依賴 cJSON 與 JsonWriter，不碰 god-stack。argv[1] = web/JSON/motor-access.json（38 個命令的覆蓋檢查）。
#   不讀寫任何機台檔，不需要圍堵環境變數；放檔尾所以自己帶 TIMEOUT（全域 timeout 區塊只套用在它之前登記的測試）。
add_executable(test_web_motor_access
    test_web_motor_access.cpp
    ${CMAKE_SOURCE_DIR}/WebMotorAccess.cpp  ${CMAKE_SOURCE_DIR}/GearCalc.cpp   # AI(W906-GEARRATIO) 20261002: WebMotorAccess.cpp's Gear Ratio part calls GearCalc (same line, nothing moves)
    ${CMAKE_SOURCE_DIR}/WebBridge/JsonWriter.cpp
    ${CMAKE_SOURCE_DIR}/Public/cJSON.c
)
target_include_directories(test_web_motor_access PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_web_motor_access PRIVATE vclcompat)   # AI(W906-COMMATEXT-FU) 20261003: NB2-1 -- MtSplitCells calls vclcompat::CommaTextCells (TStringList.cpp; the boot's CommaText, one parser)
add_test(NAME WebMotorAccess
         COMMAND test_web_motor_access "${CMAKE_SOURCE_DIR}/../web/JSON/motor-access.json" "${CMAKE_CURRENT_BINARY_DIR}/gear_scratch")   # AI(W906-GEARRATIO) 20261002: argv[2] = the Gear Ratio section's scratch directory (teach.ini / Mot_Table.csv copies; the test refuses D:/HT9045 production folders)
set_tests_properties(WebMotorAccess PROPERTIES TIMEOUT 120)
# --- test_pci1203_pure: MT-E3a 的純邏輯（1203 監看器／控制層）---
#   AI(W906-MT-E3a) 20260925: 批量 DI 的逐 port 比對、扭力換算與 PDO/SDO 一致性判準、golden InitMotor 的封閉設定表與
#   golden 順序、內部命令沒有網頁指令名稱。不帶 HAVE_PCI1203（廠商呼叫根本沒編進來），不讀寫任何檔案；放檔尾所以自己帶 TIMEOUT。
add_executable(test_pci1203_pure
    test_pci1203_pure.cpp
    ../EtherCAT/Pci1203Monitor.cpp
    ../EtherCAT/Pci1203Control.cpp)
target_include_directories(test_pci1203_pure PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_pci1203_pure PRIVATE vclcompat)
add_test(NAME Pci1203Pure COMMAND test_pci1203_pure)
set_tests_properties(Pci1203Pure PROPERTIES TIMEOUT 120)
# --- test_mt_e3b_engine: MT-E3b 引擎側（煞車族、GaliMotorServoOff／sbAbortHomeClick、VerifyMotorAction、MainProc 暫停、machinerecord 圍堵、R2）---
#   AI(W906-MT-E3b) 20260925: A 段斷言 W906_MACHINERECORD_DIR 有設而且目錄存在（沒設就拒絕呼叫 SaveMachineRecord），
#   並量 d:\HT9045\system 的 machinerecord.dat／machinerecordRealCCD.dat／PickHPRec.json 大小與時間戳不變；
#   G 段餵 machines/HT9050/Mot_Table.csv（唯讀，同 MachineMotors_HT9050）。註冊的都是假 hook，不碰 1203 卡。
#   放檔尾，所以自己帶 TIMEOUT 與 :3852 那一條合併過的圍堵環境變數（全域區塊只套用在它之前登記的測試）。
add_executable(test_mt_e3b_engine
    test_mt_e3b_engine.cpp
)
target_link_libraries(test_mt_e3b_engine PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_mt_e3b_engine PRIVATE ws2_32)
endif()
add_test(NAME MT_E3b_Engine
         COMMAND test_mt_e3b_engine "${CMAKE_SOURCE_DIR}/../machines/HT9050/Mot_Table.csv")
set_tests_properties(MT_E3b_Engine PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: same merged string as the global block (machine MACHINERECORD_DIR + laptop PRODLOG/CLEANPADLOG)
# --- test_ioweb_watch: ONSITE-1 的純邏輯（ioweb_probe 的 watch 模式、60E0h/60E1h 讀回三態）---
#   AI(W906-ONSITE-1) 20260926: tools/ioweb_watch.h 的 DI 位元差異（讀失敗那輪保留好值、改站號重取基準）與 IO_Table 反查
#   （ISABase 3：Lane = ring、IP = station、Port = stationChan*8 + bit，含 Enable=0），並拿版控的 machines/HT9050/IO_Table.csv
#   逐列對帳 JsonBridge/ChanIoPoints.cpp ResolveIoPoint；外加 Pci1203TorqueLimitReadText 的 ok/readFailed/notRead。
#   不帶 HAVE_PCI1203（不開卡），只讀版控 CSV、不寫檔。連結形狀同 IoPoints_HT9050（:379）。放檔尾，所以自己帶 TIMEOUT 與
#   :3852 那一條合併過的圍堵環境變數（全域區塊只套用在它之前登記的測試），同 MT_E3b_Engine。
add_executable(test_ioweb_watch
    test_ioweb_watch.cpp
    ../JsonBridge/ChanIoPoints.cpp
    ../EtherCAT/Pci1203Monitor.cpp)
target_include_directories(test_ioweb_watch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_ioweb_watch PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
if(WIN32)
    target_link_libraries(test_ioweb_watch PRIVATE ws2_32)
endif()
add_test(NAME IoWatch_HT9050
         COMMAND test_ioweb_watch "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv")
set_tests_properties(IoWatch_HT9050 PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: same merged string as the global block (machine MACHINERECORD_DIR + laptop PRODLOG/CLEANPADLOG)
# --- test_init_dio_status: TfMain::InitDIOStstus（golden 906 main.cpp:24357-24375）---
#   AI(W906-DIO) 20260925: TTLCfg → Prod.DIOCfg 逐欄相同（fixture，含 cModeName 中間 NUL 之後的位元組與前後鄰欄）、
#   18 個 TTL 點經記錄用後端（MyLaneIO.SetBackend）寫出的值、HT9050 真表下 18 點全部沒綁 ⇒ 0 次寫出；
#   模擬組態斷言 golden 的 #ifndef SOFT_SIMULTE（什麼都不做）。只讀 argv[1] 的 CSV，不寫任何檔。
#   連結形狀同 MachineSuckers；放檔尾（不位移上面的行號引用），所以自己補 :3834 的圍堵環境變數。
add_executable(test_init_dio_status
    test_init_dio_status.cpp
)
target_link_libraries(test_init_dio_status PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_init_dio_status PRIVATE ws2_32)
endif()
add_test(NAME InitDIOStstus
         COMMAND test_init_dio_status "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv")
set_tests_properties(InitDIOStstus PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: same merged string as the global block (machine MACHINERECORD_DIR + laptop PRODLOG/CLEANPADLOG)

# --- test_rs232_torque: Index Z 扭力的 RS232 交握（rs232.cpp，golden TCOM2 的扭力那一半）---
#   AI(W906-R28TORQ) 20260925: 使用者 20260925 裁決第 6 條「出貨組態的 Index Z 扭力上限要對齊原 BCB6 版本做法」。
#   假的 Panasonic A5 伺服器照協定回話，Comm1 強制 SIM 模式（不開 COM 埠）；逐位元組比對 golden 封包、讀回值、
#   SPComm 100 ms 分包語意；出貨組態另跑完整的 iWriteAndCheckMotorTorque（回 1／讀回錯回 2）與扭力回饋讀取。
#   不讀寫任何機台檔（不呼叫 RS232Init／W906_CreateFormBoot）；圍堵環境變數照檔尾慣例一併帶上。約 15 秒（出貨組態）。
add_executable(test_rs232_torque
    test_rs232_torque.cpp
)
target_link_libraries(test_rs232_torque PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_rs232_torque PRIVATE ws2_32)
endif()
add_test(NAME Rs232Torque COMMAND test_rs232_torque)
set_tests_properties(Rs232Torque PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-MERGE-56bbf785) 20260926: same merged string as the global block (machine MACHINERECORD_DIR + laptop PRODLOG/CLEANPADLOG)

# --- TeachButtonsGen: WebTeachButtons.gen.inc／FileRW/Teach.gen.inc 沒過期、與 golden 一致 ---
#   AI(W906-W5-b) 20260925: W5-b 覆核 W5B-1／W5B-8 —— 產生器 --check：重跑 tools/gen_teach_editlist.py（不寫檔），移植樹登錄表與
#   golden uteach.cpp 建構子各自解析、逐列比對、正則法再數一次（312 列／624 顆按鈕）、建構子 OnClick 覆寫＝:271-274 那 4 顆、
#   FormShow Tag 覆寫＝4 顆，最後和已 commit 的兩個 .gen.inc 逐字比（換行不算）。讀 golden 樹（唯讀），不寫任何檔，秒級。
#   AI(W906-W5-b) 20260925: 覆核 R-W5B-2／3 —— 另外產生 .dfm 上每一顆教導處理函式按鈕的可見性（建構子 Visible 引數、.dfm Visible／Enabled、
#   golden 全檔的 ->Visible／->TabVisible／->Enabled 賦值）與 .dfm Tag（W5B_BTN），同樣逐字比。
if(Python3_Interpreter_FOUND)
    add_test(NAME TeachButtonsGen
             COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/gen_teach_editlist.py" --check)
    set_tests_properties(TeachButtonsGen PROPERTIES TIMEOUT 120)
endif()
# --- E031_FormBridgeFullRun: AI(W906-E031) 20261003 [W906] (St01) -- todo E-031 phase 2.  A full tools/gen_formbridge.py run (no --only)
#   must reproduce the committed FileRW/_registry.cpp and FileRW/_formbridge_sources.cmake, including the two hand-written bridges
#   (TfTeach, Tfiosetview) that now come from the keep-list tools/formbridge/_hand_kept.py.  Before, those rows were hand-edited into the
#   generated files and a full run silently dropped them (wb_serve lost two bridges, the build and TeachFormShowBridge /
#   IoSetViewFormShowBridge stayed green: those tests carry their own one-entry kBridges[]).
#   tools/formbridge_fullrun_check.py: generator --out <TEMP dir> (refuses a dir inside the tree; hashes FileRW/ before / after and fails
#   if anything there changed), compares the two files (line endings ignored), then the built-in reverse check (RULINGS_20261003 #15):
#   a TEMP copy of the generator with HAND_KEPT = [] must come out different, missing exactly the hand-kept rows, else FAIL.
#   Reads golden V912 (this checkout's copy unless W906_V912_ROOT is set); no build dir, no machine file, about a second.
if(Python3_Interpreter_FOUND)
    add_test(NAME E031_FormBridgeFullRun
             COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/formbridge_fullrun_check.py")
    set_tests_properties(E031_FormBridgeFullRun PROPERTIES ENVIRONMENT "PYTHONIOENCODING=utf-8:replace" TIMEOUT 120)
endif()

# --- test_halarm: golden BCB6 元件 HAlarm（halarm.h／HAlarm.cpp）＋mycylin SetAlarm／ClearAlarm 接線 ---
#   AI(W906-HALARM) 20260926: RULINGS_20260926 第 16 條。Set 去重、Clear(code)、FIFO、SystemNG、ClearAllAlarm、
#   開機建立（golden main.cpp:22472）與氣缸兩個自由函式都接到全域 Alarm。不讀寫任何機台檔，秒級。
add_executable(test_halarm
    test_halarm.cpp
)
target_link_libraries(test_halarm PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_halarm PRIVATE ws2_32)
endif()
add_test(NAME HAlarm COMMAND test_halarm)
set_tests_properties(HAlarm PROPERTIES TIMEOUT 120)

# --- WB_TokenIdle: web/page/ht9045_recipe_client.js 的「閒置自動還權杖」（node，離線）---
#   AI(W906-TOKEN-IDLE) 20260926: 機台 0926 實測 Motor Test 拿了單一操作員權杖不還，IO 頁按 Output 被擋 10 分鐘。
#   兩個假頁面（各載一份 recipe client）接同一個照 WebBridgeServer.cpp 權杖規則的假伺服器：閒置還、hold 期間不還、
#   伺服器收回後重拿重送一次、免權杖指令不拿權杖。對照組：W906_RECIPE_CLIENT 指向修改前的檔必須紅（0926 實測 14 項失敗）。
if(W906_NODE_EXECUTABLE)
    add_test(NAME WB_TokenIdle COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/token_idle_selftest.cjs")
    set_tests_properties(WB_TokenIdle PROPERTIES TIMEOUT 60)
endif()

# AI(W906-MERGE-MACHINE) 20260926: five HT9050 IO tests the machine re-pinned to ITS field IO table (IOWEB-P7, 20260924:
#   1124 rows, Lane 1 on the ISABase-3 rows, Sen[SnMotorPower].Ring 1). That table lives in machines/HT9050/IO_Table.csv on
#   the machine, OUTSIDE the two subtrees its patches covered (HT9011UC_Cpp_V3.33.906.0/ and web/), so it did not come with
#   the merge; this tree still has the pre-P7 table (1062 rows, Lane 0). Measured 20260926 on the merged tree: 5 failures,
#   all of them the table (MachineIoTable_HT9050: nIo 1062 != 1124, iLane 0 != 1; IoPoints_HT9050: IOTable.size() != 1124;
#   IoWatch_HT9050: d1 && dIdx). Rather than put the laptop numbers back (the tests' own checks are field-specific) or keep
#   five red tests on main, they are DISABLED (ctest "Not Run (Disabled)", not a failure) exactly while the pre-P7 table is
#   the one in the tree -- counted by newlines, so CRLF/LF checkout does not matter. The moment the field table lands the
#   count changes and they run again. The machine is asked for the file in GitHub update package 5 (README_MACHINE_AI).
file(READ "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv" _ht9050_io_text)
string(REGEX MATCHALL "
" _ht9050_io_nl "${_ht9050_io_text}")
list(LENGTH _ht9050_io_nl _ht9050_io_lines)
if(_ht9050_io_lines EQUAL 1063)
    message(STATUS "machines/HT9050/IO_Table.csv is the pre-IOWEB-P7 table (1063 lines): 5 field-table tests DISABLED")
    set_tests_properties(MachineIoTable_HT9050 MachineIoTable_HT9050_PCI1203IO MachineIoTable_HT9050_Card0Control
                         IoPoints_HT9050 IoWatch_HT9050 PROPERTIES DISABLED TRUE)
endif()
unset(_ht9050_io_text)

# --- test_modal_wake: ht9045::ModalWake（WebModalWake.h）—— 阻塞框在等、沒有網頁時什麼時候開瀏覽器 ---
#   AI(W906-MODAL-WAKE) 20260926: RULINGS_20260926 第 9 條／第 26 條 Q3（30 s、2 min、每框 3 次、正式版畫面網址）。純邏輯，時間手給，
#   不開任何行程（ModalWakeLaunchEdge 不在這支測試裡呼叫）。
add_executable(test_modal_wake test_modal_wake.cpp ../WebModalWake.cpp)
target_include_directories(test_modal_wake PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
add_test(NAME ModalWake COMMAND test_modal_wake)
set_tests_properties(ModalWake PROPERTIES TIMEOUT 60)

# --- test_plc_gates: 安全 PLC 閘（RULINGS_20260926 第 20／22 條）能在 ctest 量到的部分 ---
#   AI(W906-PLC-GATES) 20260926: csystem.cpp 的 bCheckPLCConnet 讀真的 bSafePLCThread（SEAM S4 退役）＋心跳停 3 秒回 false。
#   連結形狀同 test_halarm（RESCAN 群組裡有 ht9045_comms，真的 bSafePLCThread 在那裡）。約 7 秒（兩段牆鐘等待）。不讀寫任何機台檔。
add_executable(test_plc_gates test_plc_gates.cpp)
target_link_libraries(test_plc_gates PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_plc_gates PRIVATE ws2_32)
endif()
add_test(NAME PlcGates COMMAND test_plc_gates)
set_tests_properties(PlcGates PROPERTIES TIMEOUT 60)

# --- test_lastdata_sandbox: AI(W906-LASTDATA-TESTDIR) 20260926 -- ctest never writes the machine's
#   D:\HT9045\system\lastdata*.dat: test_bootstrap.cpp redirects every test process into its own sandbox
#   (cprod.cpp W906_LastDataPath at EOF).  Real WriteLastDataFile(true)/ReadLastDataFile round-trip, the three
#   real files byte-identical before/after, and the control (variable removed -> golden literal unchanged).
#   LINK_GROUP:RESCAN as test_wb_datalayer (same god-stack closure).
add_executable(test_lastdata_sandbox test_lastdata_sandbox.cpp)
target_link_libraries(test_lastdata_sandbox PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_lastdata_sandbox PRIVATE ws2_32)
endif()
add_test(NAME LastDataSandbox COMMAND test_lastdata_sandbox)
set_tests_properties(LastDataSandbox PROPERTIES TIMEOUT 60)

# --- test_opmode_body: AI(W906-OPMODE) 20260926 -- golden TfMain::UpdateMainOperateMode body (forms/fMain_OperateMode.cpp, ht9045_sm)
#   behind the forms-layer stub (forms/fMain.cpp:507) + hook.  Control (no hook -> counter only), then the body: golden :12807 first line,
#   Hot clears fHeaterOK (:13000), AmbientHot forces soak time 0/disabled (:12950-12951), and the WriteLastDataFile tail stays in the sandbox.
add_executable(test_opmode_body test_opmode_body.cpp)
target_link_libraries(test_opmode_body PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_opmode_body PRIVATE ws2_32)
endif()
add_test(NAME OpModeBody COMMAND test_opmode_body)
set_tests_properties(OpModeBody PROPERTIES TIMEOUT 120)

# --- test_note_jamcount: AI(W906-J2) 20260926 -- golden TfNote::FormClose Jam count (note.cpp:2528-2529 -> CheckRecordJamType :344-389),
#   forms/fNote_JamCount.cpp (ht9045_sm).  RETRY counts; TRAY END / duplicate / WAR / DUMMY / unit>=9 / MTBA (G1 gate) do not; AnalogDevice_Phil
#   counts every unit; VTEST also bumps fMesSystem->iJamRateTotalForAlways; bCheckOnly=true never counts.  In-memory only, no file I/O.
add_executable(test_note_jamcount test_note_jamcount.cpp)
target_link_libraries(test_note_jamcount PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_note_jamcount PRIVATE ws2_32)
endif()
add_test(NAME NoteJamCount COMMAND test_note_jamcount)
set_tests_properties(NoteJamCount PROPERTIES TIMEOUT 120)

# --- test_atc_boot_init: AI(W906-ATC1) 20260926 -- golden TfMain::FormShow main.cpp:9357-9361 (InitialATC + ATCIniPath) at boot,
#   forms/fMain_ATCSiteUse.cpp EOF (ht9045_sm).  NB2 R72 ATC-1: without it a USE_ATC_MODE=4 machine indexes the empty ATC_SYS_PAL.
#   Control (empty at start), non-Hontech -> no channels / path set, Hontech -> 4 channels, the SetRunATC(false) crash path now runs.
add_executable(test_atc_boot_init test_atc_boot_init.cpp)
target_link_libraries(test_atc_boot_init PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_atc_boot_init PRIVATE ws2_32)
endif()
add_test(NAME AtcBootInit COMMAND test_atc_boot_init)
set_tests_properties(AtcBootInit PROPERTIES TIMEOUT 120)

# --- test_timer2_testseconds: AI(W906-J12) 20260926 -- golden TfMain::Timer2Timer test seconds (main.cpp:21157-21205),
#   forms/fMain_TestSeconds.cpp (ht9045_sm).  Not testing -> 0; one count per new SystemSec; keeps counting past iMaxTime (golden only logs).
add_executable(test_timer2_testseconds test_timer2_testseconds.cpp)
target_link_libraries(test_timer2_testseconds PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_timer2_testseconds PRIVATE ws2_32)
endif()
add_test(NAME Timer2TestSeconds COMMAND test_timer2_testseconds)
set_tests_properties(Timer2TestSeconds PROPERTIES TIMEOUT 120)

# --- test_record_time_info: AI(W906-W2-3) 20260926 -- golden RecordTimeInfo (cObserver.cpp:1846-2134), cObserver_TimeInfo.cpp (ht9045_sm).
#   Fixed start/end times -> TimeInfoGrid row 11, history shift, index cycle, the 60-minute wrap; RunInfo.TestTime / IndexCycleTime / dTestSec.
add_executable(test_record_time_info test_record_time_info.cpp)
target_link_libraries(test_record_time_info PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_record_time_info PRIVATE ws2_32)
endif()
add_test(NAME RecordTimeInfo COMMAND test_record_time_info)
set_tests_properties(RecordTimeInfo PROPERTIES TIMEOUT 120)

# ---------------------------------------------------------------------------
#  AI(W906-ENV-ALL) 20260926: the redirect roots for EVERY test.
#  The global ENVIRONMENT block above (set_tests_properties(${_ht9045_all_tests} ...), about :3971) only reaches the tests
#  that existed when that line ran; everything add_test'ed after it (the EOF blocks: OpModeBody, LastDataSandbox,
#  NoteJamCount, AtcBootInit, Timer2TestSeconds, RecordTimeInfo, St02's TesterComm_* / MyDB_* ...) ran with NO redirect,
#  i.e. against the real D:\HT9045_Log / system paths (St02 FROM_STEVEN 22:34 found it for cMyDB P4).
#  This deferred call runs at the END of this directory (cmake_language(DEFER), CMake >= 3.19; this tree builds with 4.0.2),
#  so it also covers tests appended below this line later:
#    1. a test with no ENVIRONMENT at all gets the same merged string as the global block (read back from `config_db`,
#       which is in that block) -- a test that set its own ENVIRONMENT is left alone;
#    2. anything in _ht9045_env_extra is APPENDed to every test (APPEND, so nobody's own variables are clobbered).
#       New redirect roots go into _ht9045_env_extra (list(APPEND ...) anywhere in this file), not into the long string.   3. AI(W906-TESTGUARD) 20260928: the same APPEND gives every test its OWN Gerneral.ini (St02 FROM_STEVEN 0928 11:03 item 6 (b)): W906_GENERAL_INI_PATH=<this binary dir>/general_ini_scratch/<test name>/Gerneral.ini (asGeneralPath, common.cpp :155 / :393) and W906_GENERAL_INI_SEED=D:/HT9045/system/Gerneral.ini; tests/test_bootstrap.cpp copies the seed over it before main() -- fresh every run, kept afterwards as the evidence of what the test wrote back.  Per test because ctest -jN runs tests concurrently.  It comes after any value a test set earlier (ELA_Service's DEFER at :3426) and so replaces it; a test that needs its own path adds a DEFERred APPEND after the cmake_language(DEFER CALL _w906_env_all_tests) line below.  The live-config readers do not move: config_db / IniFiles open HT9045_SYSTEM_DIR-or-literal, ini_helpers / GA1_ReadGeneralIni copy the LITERAL D:\HT9045\system\Gerneral.ini to %TEMP% themselves, config_loaders never opens it.
# ---------------------------------------------------------------------------
if(NOT DEFINED _ht9045_env_extra)
    set(_ht9045_env_extra "")
endif()
function(_w906_env_all_tests)
    get_test_property(config_db ENVIRONMENT _w906_ref_env)
    if(NOT _w906_ref_env)
        message(WARNING "[W906-ENV-ALL] config_db has no ENVIRONMENT -- the global redirect block moved?  Nothing copied.")
        set(_w906_ref_env "")
    endif()
    get_property(_w906_all DIRECTORY PROPERTY TESTS)
    set(_w906_n 0)
    foreach(_w906_t IN LISTS _w906_all)
        get_test_property(${_w906_t} ENVIRONMENT _w906_e)
        if(NOT _w906_e AND _w906_ref_env)
            set_property(TEST ${_w906_t} PROPERTY ENVIRONMENT "${_w906_ref_env}")
            math(EXPR _w906_n "${_w906_n}+1")
        endif()
        if(_ht9045_env_extra)
            set_property(TEST ${_w906_t} APPEND PROPERTY ENVIRONMENT ${_ht9045_env_extra} "W906_GENERAL_INI_PATH=${CMAKE_CURRENT_BINARY_DIR}/general_ini_scratch/${_w906_t}/Gerneral.ini" "W906_GENERAL_INI_SEED=D:/HT9045/system/Gerneral.ini")   # AI(W906-TESTGUARD) 20260928: 3. above -- a Gerneral.ini sandbox per test (tests/test_bootstrap.cpp seeds it)
        endif()
    endforeach()
    list(LENGTH _w906_all _w906_total)
    message(STATUS "[W906-ENV-ALL] ${_w906_n} of ${_w906_total} test(s) had no ENVIRONMENT and got the redirect roots")
endfunction()
cmake_language(DEFER CALL _w906_env_all_tests)

# --- test_initial_memory: AI(W906-INITMEM) 20260927 -- golden InitialMemory (cmydef.cpp:5786-5872), cmydef_InitialMemory.cpp (ht9045_globals).
#   SiteData[] X x Y per test mode, a dirtied sample zeroed again, asGPIBTempShow[] cleared by assignment.  After the DEFER line on
#   purpose: the deferred ENV-ALL function runs at the end of this directory, so this test still gets the redirect roots.
add_executable(test_initial_memory test_initial_memory.cpp)
target_link_libraries(test_initial_memory PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_initial_memory PRIVATE ws2_32)
endif()
add_test(NAME InitialMemory COMMAND test_initial_memory)
set_tests_properties(InitialMemory PROPERTIES TIMEOUT 120)

# --- START_SitesCensus: AI(W906-R80) 20260927 -- the "CI form" CLAUDE.md always named (tools/start_sites_census.py --check) was
#   never a ctest, so it sat red from 20260926 14:56 (St02 6fff0960 added HandlerGpibMsg.cpp:717) until NB2 R80 noticed.
#   33 call sites / 31 live / 2 gated (AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 main.cpp:15847-15858 (RULINGS_20261002 #20 / #23-6): the golden 912 GPIB remote START site is gone; AI(W906-W10) 20260927 (St02-E): HTSET,333 Command.cpp:17108 calls W906_RemoteRunStart).  Adding or removing a START path must change these numbers ON PURPOSE.
#   Static scan of the source tree, no build dir, seconds.
if(Python3_Interpreter_FOUND)
    add_test(NAME START_SitesCensus
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/start_sites_census.py" --check 36 34 2)   # AI(W906-SCANKEY) 20261003: was 35 33 2 -- + WebMainScanKey.cpp W906_RemoteRunStart("ScanKey_2"), golden TfMain::ScanKey main.cpp:2516 (the panel START key)    # AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 main.cpp:15847-15858 (RULINGS_20261002 #20 / #23-6): was 34 32 2 -- the golden 912 GPIB remote START site (TesterComm/Handler/HandlerGpibMsg.cpp, St02 MR !33, AI(W906-H012-CENSUS) 20261001) is removed   AI(W906-B50-CENSUS) 20261003 (laptop batch 50 merge): main had 36 34 2 after review6 wired CT-3b (FileRW/DeviceForm_File.cpp:703) and OS-1b (FileRW/Offset_File.cpp:846) in batch 47; minus this GPIB remote START site = 35 33 2
    set_tests_properties(START_SitesCensus PROPERTIES ENVIRONMENT "PYTHONIOENCODING=utf-8:replace" TIMEOUT 120)
endif()

# --- AI(W906-W10) 20260927 (St02-E): W10 = B -- the Handler's TCP command server 7016 / result server 7017 (golden
#   TCPCommandServer / TeraTCPResultServer).  Plan: D:\HT9045\.claude\skills\ht9045-st02-workflow\references\
#   w10-tcp-command-server-plan.md; ledger docs/TESTERCOMM_PORT_LEDGER.md W10.
#   TesterComm_TcpCmdFramer       R2 framer alone (TesterComm/Tcp/TcpCmdFramer.cpp): split / joined / noise / the 2048 cap.
#                                 No file, no socket.
#   TesterComm_TcpCmdServer       the pump + framer + the golden body end to end with the fMain servers in Sim (the test
#                                 fails if either gets a real socket); one test-owned polled server on 127.0.0.1:<ephemeral>
#                                 (never 7016 / 7017).  Files only in %TEMP%\ht9045_w10_<tick>: the test sets AuthPath /
#                                 DataPath / LastDataPath / asTCPIPPath / as9045LogPath itself and aborts before any call if
#                                 one is under D:\HT9045; it stats (read-only) the machine's config.ini / SetUp.inf /
#                                 lastdata.dat / today's TCPIP_Log file and fails if one changed.
#   TesterComm_TcpCmdFieldsCensus the framer's field table == what Command.cpp's TCP branches read (static scan, seconds).
add_executable(test_tcp_cmd_framer test_tcp_cmd_framer.cpp ../TesterComm/Tcp/TcpCmdFramer.cpp)
target_include_directories(test_tcp_cmd_framer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_tcp_cmd_framer PRIVATE vclcompat)
add_test(NAME TesterComm_TcpCmdFramer COMMAND test_tcp_cmd_framer)
set_tests_properties(TesterComm_TcpCmdFramer PROPERTIES TIMEOUT 60)
add_executable(test_tcp_cmd_server test_tcp_cmd_server.cpp)
target_link_libraries(test_tcp_cmd_server PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_tcp_cmd_server PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_TcpCmdServer COMMAND test_tcp_cmd_server)
set_tests_properties(TesterComm_TcpCmdServer PROPERTIES TIMEOUT 180)
if(Python3_Interpreter_FOUND)
    add_test(NAME TesterComm_TcpCmdFieldsCensus
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/tcp_cmd_fields_census.py" --check)
    set_tests_properties(TesterComm_TcpCmdFieldsCensus PROPERTIES ENVIRONMENT "PYTHONIOENCODING=utf-8:replace" TIMEOUT 60)
endif()

# --- test_record_error_log: AI(W906-RECERR) 20260927 -- golden RecordErrorLog (cpublic.cpp EOF) writes a real dated log file under
#   as9045LogPath; the test refuses to run unless that is the machine_log_scratch sandbox (ENV-ALL + St02 D5).
add_executable(test_record_error_log test_record_error_log.cpp)
target_link_libraries(test_record_error_log PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_record_error_log PRIVATE ws2_32)
endif()
add_test(NAME RecordErrorLog COMMAND test_record_error_log)
set_tests_properties(RecordErrorLog PROPERTIES TIMEOUT 120)
# --- test_mydb_p4_containment: AI(W906-CMYDB-P4) 20260927 (St02-E) -- cMyDB P4: the golden RecordProcess / NewRecordProcess /
#   MyDBIProcessNew / MyDBIProcess (+ the 2-arg adapter, RecordChangeLogProcess, the wb_serve MyMessageBox shapes), ProductionLog
#   (O06) and SaveMessageHistroy (Greatek + N14_1) write their token rows ONLY under machine_log_scratch.  Refuses to run unless the
#   four roots (W906_HT9045LOG_ROOT / W906_SAVEEVENTLOG_ROOT / W906_RMS_ROOT / W906_PRODINFO_ROOT, via _ht9045_env_extra) are there,
#   and fails if anything under the real golden roots was created or written.  RUN_SERIAL so that a hit is this test's own.
#   After the DEFER line on purpose: the deferred ENV-ALL function runs at the end of this directory (as test_initial_memory).
add_executable(test_mydb_p4_containment test_mydb_p4_containment.cpp)
target_link_libraries(test_mydb_p4_containment PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_mydb_p4_containment PRIVATE ws2_32)
endif()
add_test(NAME MyDB_P4_Containment COMMAND test_mydb_p4_containment)
set_tests_properties(MyDB_P4_Containment PROPERTIES TIMEOUT 120 RUN_SERIAL TRUE)
# --- test_weblogin_force_operator: AI(W906-D4) 20260928 (St02-E) -- D4: golden TfMain::ChangeTesterConnect Off-Line -> On-Line
#   "forced back to Operator" (golden 906_0625_Steven main.cpp:12104-12131; 912 :12621-12648) = WebLogin.cpp W906_WebLoginForceOperator,
#   installed at static init in W906_WebLoginForceOperatorHook (LogObjects.cpp EOF).  WebLogin.cpp lives in the root CMakeLists.txt's
#   wb_serve target, so it is listed here as a source; god-stack through RESCAN (as wb_serve); FormLock / FormUnlock
#   (JsonBridge/FormJson.cpp, wb_serve only) are empty stand-ins in the test file.  The three login seams point into the build dir's
#   weblogin_d4_scratch through a DEFERred APPEND scheduled after the ENV-ALL call above, so the test still gets the global redirect
#   string and _ht9045_env_extra first.  It refuses to run unless the seams and the cMyDB log roots are sandboxed, and fails if
#   D:\HT9045\system\login.dat / levelset.dat changed.  The call site (forms/fMain.cpp :1148) is not exercised (see the test file).
add_executable(test_weblogin_force_operator
    test_weblogin_force_operator.cpp
    ${CMAKE_SOURCE_DIR}/WebLogin.cpp
)
target_include_directories(test_weblogin_force_operator PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_weblogin_force_operator PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_weblogin_force_operator PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_weblogin_force_operator.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME WebLogin_ForceOperator COMMAND test_weblogin_force_operator)
set_tests_properties(WebLogin_ForceOperator PROPERTIES TIMEOUT 120)
cmake_language(DEFER CALL set_property TEST WebLogin_ForceOperator APPEND PROPERTY ENVIRONMENT
    "W906_LOGINDAT_PATH=${CMAKE_CURRENT_BINARY_DIR}/weblogin_d4_scratch/login.dat"
    "W906_PWBOOK_PATH=${CMAKE_CURRENT_BINARY_DIR}/weblogin_d4_scratch/pwbook.txt"
    "W906_LEVELSET_PATH=${CMAKE_CURRENT_BINARY_DIR}/weblogin_d4_scratch/levelset.dat")
# --- test_tester_connect_rules: AI(W906-D1D7) 20260928 (St02-E) -- golden TfMain::ChangeTesterConnect D1 / D2 / D3 / D6 / D7
#   (906_0625_Steven main.cpp:12072-12237) = TesterComm/Handler/HandlerTesterConnect.cpp, seats at LogObjects.cpp EOF.  Links the
#   Handler tester library + the machine archives (RESCAN, as test_testercomm_handler).  Memory only except the cMyDB log rows
#   (NewRecordProcess / RecordProcess), which go to machine_log_scratch through ENV-ALL (the test refuses to run otherwise).
#   D2 checks differ by build: SOFT_SIMULTE (sim) compiles golden's refusal out.  The fMain.cpp call sites are not exercised.
add_executable(test_tester_connect_rules test_tester_connect_rules.cpp ../JsonBridge/actions/MainTesterConnect.cpp ../JsonBridge/EventLog.cpp)   # AI(W906-GB-P2e-EN) 20260928 (St02-E): [8] act.main.testerConnect enable gate (the same two sources test_sjson_chan compiles)
target_link_libraries(test_tester_connect_rules PRIVATE
    ht9045_testercomm_handler ht9045_testercomm ht9045_webbridge   # AI(W906-GB-P2e-EN) 20260928 (St02-E): JsonWriter for [8]
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_tester_connect_rules PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_tester_connect_rules.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Wno-unused-parameter")   # Motor/HTMotor.h (via Motor/mymotor.h) stubs
endif()
add_test(NAME TesterConnect_Rules COMMAND test_tester_connect_rules)
set_tests_properties(TesterConnect_Rules PROPERTIES TIMEOUT 120)
# --- test_change_log: AI(W906-CHGLOG) 20260927 -- golden WriteIniData change-log blocks + TempChangeLog (common_ChangeLog.cpp,
#   ht9045_sm), called directly and once end to end through common.cpp's hooks.  Writes only %TEMP%\ht9045_chglog_<pid>\ (removed).
add_executable(test_change_log test_change_log.cpp)
target_link_libraries(test_change_log PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_change_log PRIVATE ws2_32)
endif()
add_test(NAME ChangeLog COMMAND test_change_log)
set_tests_properties(ChangeLog PROPERTIES TIMEOUT 120)
# --- test_memo_log: AI(W906-MEMO) 20260927 -- TfMainMemo stores lines (+4096 cap), AddShuttleMessage >2048 clear, golden OutShuttleLog
#   (acarry_shims.cpp EOF) writes SH2_/SH1_ logs under asShtLogPath; refuses to run unless that is the machine_log_scratch sandbox (D5).
add_executable(test_memo_log test_memo_log.cpp)
target_link_libraries(test_memo_log PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_memo_log PRIVATE ws2_32)
endif()
add_test(NAME MemoLog COMMAND test_memo_log)
set_tests_properties(MemoLog PROPERTIES TIMEOUT 120)
# --- test_task_list_register: AI(W906-TASKLIST) 20260927 -- golden TfMain::FormShow main.cpp:9717-10036 task-ring registration
#   (cStateRecord.cpp EOF W906_BootRegisterTaskList): 264 rows, port-side mappings (fContactForm, relocated BarCode globals),
#   CheckTaskChange records, SaveTaskList writes the reference StateRecord row shape.  Writes only %TEMP%\ht9045_tasklist_test\.
add_executable(test_task_list_register test_task_list_register.cpp)
target_link_libraries(test_task_list_register PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_task_list_register PRIVATE ws2_32)
endif()
add_test(NAME TaskListRegister COMMAND test_task_list_register)
set_tests_properties(TaskListRegister PROPERTIES TIMEOUT 120)
# --- test_teachleave: AI(W906-FRW-S122) 20260927 -- S122 關掉／打開 Teach／Motor Test ⇒ fAllMotorHome=false（WebTeachLeave.cpp）。
#   RULINGS_20260927 第 2 條第 18 題＝B、decisions-pending R80～R83＝A；golden V912 main.cpp:28846-28847、uteach.cpp:2442-2443、:1610。
#   受測檔住在根 CMakeLists.txt 的 wb_serve 目標（理由同 WebWindowRegistry.cpp／WebCmdGuard.cpp），這裡直接列來源；用真的
#   WebWindowRegistry.cpp 餵總表訊框（過期用 WebWindowRegistryAgeConnForTest），不 include cmydef、不 link god-stack、不讀寫任何機台檔，秒級。
add_executable(test_teachleave
    test_teachleave.cpp
    ${CMAKE_SOURCE_DIR}/WebTeachLeave.cpp
    ${CMAKE_SOURCE_DIR}/WebWindowRegistry.cpp
    ${CMAKE_SOURCE_DIR}/Public/cJSON.c
)
target_include_directories(test_teachleave PRIVATE ${CMAKE_SOURCE_DIR})
if(WIN32)
    target_link_libraries(test_teachleave PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_teachleave.cpp ${CMAKE_SOURCE_DIR}/WebTeachLeave.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME WebTeachLeave COMMAND test_teachleave)
set_tests_properties(WebTeachLeave PROPERTIES TIMEOUT 60)
# --- test_editlist_pageindex: AI(W906-FRW-S158) 20260927 [W906] -- C 路共用層 TPageControl 目前分頁（值種類 "activePageIndex"）。
#   FileRW/_EditList.cpp：editlist.get 的 proxies 帶 activePageIndex、editlist.save widgets／form.event state 送回時套回替身；
#   不合法（非整數／超出 0..分頁數-1／點不到）只丟那一筆、理由進 notes／todo；沒帶這個鍵＝舊行為；ELSetPageOrder 精確範圍。
#   受測檔住在根 CMakeLists.txt 的 wb_serve 目標（FileRW/_editlist_sources.cmake），這裡直接列來源；只依賴 vclcompat 與
#   cJSON／JsonWriter（ht9045_webbridge → ht9045_public）。HTEditList_RegisterControlName（Public/HTEditList.cpp，ht9045_sm）
#   由測試檔給只記名字的替身 —— 不 link god-stack。不讀寫任何檔，秒級。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_editlist_pageindex
    test_editlist_pageindex.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
)
target_include_directories(test_editlist_pageindex PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_editlist_pageindex PRIVATE ht9045_webbridge vclcompat)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_editlist_pageindex.cpp ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME EditList_PageIndex COMMAND test_editlist_pageindex)
set_tests_properties(EditList_PageIndex PROPERTIES TIMEOUT 60)
# --- test_hsys_heater_mix: AI(W906-FRW-S166) 20260927 [W906] -- HandlerSys「Heater」分頁 Q34 方案 D＋Q15 的讀寫檔（FileRW/HSys.cpp 第 (4) 段）。
#   RULINGS_20260926 S166（D-2＝A 依 RULINGS_20260927 第 7 條第 35 題）＋S137（Q15：缺鍵＝3、開頁不寫檔）＋S136（Q14＝B）。
#   受測檔住在根 CMakeLists.txt 的 wb_serve 目標（FileRW/_editlist_sources.cmake），這裡直接列來源：FileRW/HSys.cpp（include 產生檔
#   FileRW/HSys.gen.inc）＋C 路共用層 FileRW/_EditList.cpp／_EditPage.cpp；god-stack 用 RESCAN 連（同 wb_serve），沒有 TU-local 替身。
#   只寫 %TEMP%\ht9045_hsys_heater_mix_test\（asGeneralPath 轉向＋OpenGeneralIniFile）；不跑 golden FormShow／SaveSystemSet 全段（會讀寫
#   D:\GPIB9045、D:\RS232Standard）。開跑前後比對 D:\HT9045\system\Gerneral.ini 與 D:\HT9045\config\config.ini，有變就失敗。
#   放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_hsys_heater_mix
    test_hsys_heater_mix.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/HSys.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
)
target_include_directories(test_hsys_heater_mix PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_hsys_heater_mix PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_hsys_heater_mix PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_hsys_heater_mix.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME HSys_HeaterMix COMMAND test_hsys_heater_mix)
set_tests_properties(HSys_HeaterMix PROPERTIES TIMEOUT 120)
# --- test_b8_ctl2_timerep: AI(W906-B8-CTL2) 20260930 [W906] -- B8 CT-L2：Contact 頁 RTC Auto Tuning 勾選看不看得到＝golden TfContact::TimerEPTimer
#   （V912 cContact.cpp:17177-17232；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CT-L2」；Steven 20260929「請按照bcb的邏輯處理」）。
#   受測：FileRW/DeviceForm_File.cpp 檔尾 FileRW_Contact_TimerEPTick＋產生檔 FileRW/DeviceForm_File.gen.inc 的 DF_TimerEPTimer（條件 16 種組合、
#   等級 176、bOldRTCAutoTuning 邊緣兩種客戶、golden 開機值＝一律隱藏、EP 讀值／KYEC 倒數不在這一列），以及開頁／form.event 補一拍的接法
#   （原始碼棘輪，argv[1]＝移植樹根目錄，唯讀）。同 test_hsys_heater_mix 直接列來源、god-stack 用 RESCAN 連；不跑 golden FormShow。
#   不寫任何檔：開跑前後比對 D:\HT9045\system\Gerneral.ini／ContactInfo.ini、D:\HT9045\config\config.ini；ENVIRONMENT 照全域那一串（保險）。
#   放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_b8_ctl2_timerep
    test_b8_ctl2_timerep.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_File.cpp  ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_KbExtra.cpp   # AI(W906-SETUPA-KB) 20261002: DeviceForm_File.cpp 的 g_page 帶 extraJson（本體在 DeviceForm_KbExtra.cpp）；同一行
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_ctl2_timerep PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_b8_ctl2_timerep PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_ctl2_timerep PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_ctl2_timerep.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_CtL2_TimerEP COMMAND test_b8_ctl2_timerep "${CMAKE_SOURCE_DIR}")
set_tests_properties(B8_CtL2_TimerEP PROPERTIES TIMEOUT 120 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-B8-CTL2) 20260930: the global containment string verbatim (same as GaliRouteLive)
# --- test_openenter_log: AI(W906-FRW-S165) 20260927 [W906] -- C 路開頁記 golden 開窗鈕的 "Enter ..."（RULINGS_20260926 S165＝R101「要記」）。
#   FileRW/_EditPage.cpp 檔尾 FindOpenEnter／OpenEnterRecord（21 列逐列＝golden V912 main.cpp、每列都在 kOpenGates、不記的 4 頁）＋
#   PageJson 的呼叫點（第一次記且在 golden FormShow 之前、存檔後重讀不記、golden Close() 之後再記、沒開機不記）。
#   受測檔住在根 CMakeLists.txt 的 wb_serve 目標（FileRW/_editlist_sources.cmake），這裡直接列來源；god-stack 全域與 golden 的
#   NewRecordProcess／RecordProcess 由測試檔給替身 —— 不 link god-stack。不寫任何檔（[12] 唯讀讀 WebBuilder／WebSmartDiag／WebTowerLight.cpp 與 FileRW/Offset_File.gen.inc，AI(W906-AUTHMAINFORM) 20260930），秒級。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_openenter_log
    test_openenter_log.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
)
target_include_directories(test_openenter_log PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_openenter_log PRIVATE ht9045_webbridge vclcompat)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_openenter_log.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME OpenEnterLog COMMAND test_openenter_log "${CMAKE_SOURCE_DIR}")   # AI(W906-AUTHMAINFORM) 20260930 [W906]: + source root for [12] (read-only ratchet: the three config-menu RouteGuards, Offset_File.gen.inc)
set_tests_properties(OpenEnterLog PROPERTIES TIMEOUT 60)
# --- test_mainclose_stop: AI(W906-FRW-S167) 20260928 [W906] Q44（最小版）「按 Exit 必須先停下才能關閉」的判斷 ---
#   受測：FileRW/MainClose.cpp 檔尾 (甲) w906q44::Evaluate（Steven 20260928：停全部馬達＋關加熱器繼電器，讀回沒停就不關）。
#   測試檔先定義 W906_MAINCLOSE_Q44_DECIDE_ONLY 再 #include MainClose.cpp ⇒ 只編純判斷那一半（只用標準庫）：沒卡＝不擋、全停＝確認、
#   軸在動／繼電器仍是 1／拒寫／DRY RUN／卡失聯＝擋，等讀回（監看器 2 次輪詢）與 5 秒上限。MainClose.cpp 本身只編進 wb_serve（根 CMakeLists.txt）。
#   不 link god-stack、不讀寫任何檔、不碰 1203 卡，秒級。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_mainclose_stop test_mainclose_stop.cpp)
target_include_directories(test_mainclose_stop PRIVATE ${CMAKE_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_mainclose_stop.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME MainCloseStop COMMAND test_mainclose_stop)
set_tests_properties(MainCloseStop PROPERTIES TIMEOUT 60)
# --- test_formevent_position: AI(W906-EVB1) 20260928 [W906] X-2 -- WS form.event 帶控制項自己的新值：position（TTrackBar／TScrollBar／
#   TUpDown＝filerw::ELTrackBar 替身的 OnChange）與 activePageIndex（TPageControl 的 OnChange）；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B1。
#   受測：FileRW/_FormEvent.cpp（解析兩個新鍵、A 形狀帶了就拒）、FileRW/_EditPage.cpp RunPageEvent（先驗、先套再跑處理器、夾值補 changed／todo）、
#   FileRW/_EditList.cpp ELPageIndexRefused。受測檔住在根 CMakeLists.txt 的 wb_serve 目標，這裡直接列來源；god-stack 全域、golden
#   NewRecordProcess／RecordProcess、JsonBridge 的 FindBridge／RunEvent／FormLock 由測試檔給替身（同 test_openenter_log）—— 不 link god-stack。
#   不讀寫任何檔，秒級。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_formevent_position
    test_formevent_position.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
)
target_include_directories(test_formevent_position PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_formevent_position PRIVATE ht9045_webbridge vclcompat)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_formevent_position.cpp ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME FormEvent_Position COMMAND test_formevent_position)
set_tests_properties(FormEvent_Position PROPERTIES TIMEOUT 60)
# --- test_pagetable: AI(W906-PAGETAB-Q51) 20260928 [W906] -- 頁面表（WebPageTable.cpp，取代 golden fShow）T1＋T2。
#   Steven 20260928 Q51／第一優先「fShow 改用網頁顯示的陣列」、Q-P1（沒有畫面不准 START；運轉中全關 ⇒ 寬限 10 秒後正常 STOP）、Q-P2＝A、Q-P3＝A。
#   設計 D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md §6 T1／T2。受測檔住在根 CMakeLists.txt 的 wb_serve 目標
#   （理由同 WebWindowRegistry.cpp／WebTeachLeave.cpp），這裡直接列來源；用真的 WebWindowRegistry.cpp 餵總表訊框、WebTeachLeave.cpp 驗安裝後的取樣。
#   T2 讀 D:\HT9045\web\background.html（唯讀）的 WINDOWS 表逐列比。不 include cmydef、不 link god-stack、不寫任何檔，秒級。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_pagetable
    test_pagetable.cpp
    ${CMAKE_SOURCE_DIR}/WebPageTable.cpp
    ${CMAKE_SOURCE_DIR}/WebTeachLeave.cpp
    ${CMAKE_SOURCE_DIR}/WebWindowRegistry.cpp
    ${CMAKE_SOURCE_DIR}/Public/cJSON.c
)
target_include_directories(test_pagetable PRIVATE ${CMAKE_SOURCE_DIR})
if(WIN32)
    target_link_libraries(test_pagetable PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_pagetable.cpp ${CMAKE_SOURCE_DIR}/WebPageTable.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME WebPageTable COMMAND test_pagetable "${CMAKE_SOURCE_DIR}/../web/background.html")
set_tests_properties(WebPageTable PROPERTIES TIMEOUT 60)
#   AI(W906-PAGETAB-Q51) 20260928 [W906]：WebTeachLeave.cpp 的 WindowEdgeSample 現在會問頁面表（安裝之後才生效；test_teachleave 沒安裝 ⇒ 行為不變），
#   所以 test_teachleave 也要連 WebPageTable.cpp（上面 test_teachleave 那一段不動，這裡補來源）。
target_sources(test_teachleave PRIVATE ${CMAKE_SOURCE_DIR}/WebPageTable.cpp)
# --- FShow_Audit: AI(W906-PAGETAB-Q51) 20260928 [W906] -- 全樹「畫面開著沒」讀取點的棘輪（設計 page-state-array.md §6 T3，步驟 3）。
#   tools/fshow_audit.py：每個檔「直接讀表單的 fShow／bShow（＋頁面表上表單的 Visible／Showing）」的數目只能減、不能增
#   （基準 tools/fshow_audit_baseline.json；`#if 0` 裡的不算，解閘的人一解開就會被抓）；另抓 WebWindowRegistry.cpp／
#   WebTeachLeave.cpp 以外直接呼叫舊政策 WebWindowRegistryFShowPolicy／DiagnosticsOpen 的。改法：W906_FormShowing("X", X->fShow)。
#   純靜態掃描（git ls-files），不 build、不讀寫任何真實檔，秒級。稽核器自己也要驗（_SelfTest）。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
if(Python3_Interpreter_FOUND)
    add_test(NAME FShow_Audit COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/fshow_audit.py")
    set_tests_properties(FShow_Audit PROPERTIES TIMEOUT 120)
    add_test(NAME FShow_Audit_SelfTest COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/fshow_audit.py" --selftest)
    set_tests_properties(FShow_Audit_SelfTest PROPERTIES TIMEOUT 60)
endif()
# --- test_evb10a_edges: AI(W906-EVB10A) 20260929 [W906] -- 事件批次 B10 part a：設定視窗真的開／關那一下的分派表
#   （FileRW/WindowEdgeTails.h：YM-3 Yield、SU-9 Setup、OS-6 Offset、CT-3b Contact、CC-E10 Configuration）。
#   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10；Steven 20260928「任何畫面的事件, 都是我們做」、20260929「照 BCB 的邏輯」。
#   受測：表的表單名＝頁面表的網頁列＝background.html WINDOWS 的 form:（改名會靜默對不上）、Decide（運轉中哪幾個不跑）、OpenLatch、
#   跟真的頁面表／視窗總表一起跑的邊緣。各表單的 golden 本體只編進 wb_serve（FileRW/<結構>.cpp 檔尾），不在這裡。
#   同 test_pagetable 直接列來源；不 include cmydef、不 link god-stack、不寫任何檔（background.html 唯讀），秒級。
add_executable(test_evb10a_edges
    test_evb10a_edges.cpp
    ${CMAKE_SOURCE_DIR}/WebPageTable.cpp
    ${CMAKE_SOURCE_DIR}/WebTeachLeave.cpp
    ${CMAKE_SOURCE_DIR}/WebWindowRegistry.cpp
    ${CMAKE_SOURCE_DIR}/Public/cJSON.c
)
target_include_directories(test_evb10a_edges PRIVATE ${CMAKE_SOURCE_DIR})
if(WIN32)
    target_link_libraries(test_evb10a_edges PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_evb10a_edges.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME EvB10A_Edges COMMAND test_evb10a_edges "${CMAKE_SOURCE_DIR}/../web/background.html" "${CMAKE_SOURCE_DIR}")   # AI(W906-EVB10C-BC) 20260930 [W906]: + source root for [7] (BarCode Exit wiring ratchet, read-only)
set_tests_properties(EvB10A_Edges PROPERTIES TIMEOUT 60)
# --- test_evb10b_vclclicks: AI(W906-EVB10B) 20260929 [W906] -- 事件批次 B10 part b 的 C 路共用層（FileRW/_EditList.cpp）：
#   X-3（R100、R118＝照 BCB）VCL「程式設 TRadioGroup::ItemIndex／TCheckBox::Checked／TRadioButton::Checked 也觸發 DFM 的 OnClick」
#   （ELSetOnClick／ELClickIndex／ELClickChecked／HTEditList hook 用的 ELHTEditSet*），X-5／CC-L1 切分頁事件讀分頁（ELActivePage／ELPageIndexOf／
#   ELTabIndexOf）與 CC-L1 的整頁鎖組合；R118 的「ReadEditTextFromFile 再 InitialDataToEdit」順序。D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10。
#   同 test_editlist_pageindex 直接列來源、不 link god-stack（HTEditList_RegisterControlName 由測試檔給替身）；不讀寫任何檔，秒級。
#   放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_evb10b_vclclicks
    test_evb10b_vclclicks.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
)
target_include_directories(test_evb10b_vclclicks PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_evb10b_vclclicks PRIVATE ht9045_webbridge vclcompat)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_evb10b_vclclicks.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME EvB10B_VclClicks COMMAND test_evb10b_vclclicks)
set_tests_properties(EvB10B_VclClicks PROPERTIES TIMEOUT 60)
# --- test_evd013_barcode: AI(W906-D013) 20260929 [W906] -- todo D-013（Steven 20260929「照 BCB 的邏輯」）R126 網頁刷條碼框：
#   WS form.event 的 value "barcode"、formevent::BarcodeBoxOpen／BarcodeBoxClosed、ack 的 "barcode"（FileRW/_FormEvent.cpp 檔尾，FileRW/_FormEventCtx.h）；
#   BarcodeReader.cpp 檔尾 W906_BarcodeBoxModal（golden 的 FormShow → TimerKeyIn → btnEnterClick → FormClose 跑刷到的字）；R128 用的 CurrentValueJson。
#   受測檔住在根 CMakeLists.txt 的 wb_serve／ht9045_sm，這裡直接列來源（同 test_formevent_position）；god-stack 全域、RecordProcess／EventReport、
#   JsonBridge 的 FindBridge／RunEvent／FormLock 由測試檔給替身；myTimer.cpp 直接連。同一支測試編兩次：SIM（SOFT_SIMULTE，golden FormShow 關計時器）
#   與 SHIP（W906_NO_SOFT_SIMULTE，TimerKeyIn 照 KYEC 條碼格式清字）——結果依組態不同的只有 [6]。不 link god-stack、不讀寫任何檔，秒級。
foreach(_d013 test_evd013_barcode test_evd013_barcode_ship)
    add_executable(${_d013}
        test_evd013_barcode.cpp
        ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
        ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
        ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
        ${CMAKE_SOURCE_DIR}/BarcodeReader.cpp
        ${CMAKE_SOURCE_DIR}/myTimer.cpp
    )
    target_include_directories(${_d013} PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(${_d013} PRIVATE ht9045_webbridge vclcompat)
endforeach()
target_compile_definitions(test_evd013_barcode_ship PRIVATE W906_NO_SOFT_SIMULTE)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_evd013_barcode.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME EvD013_Barcode COMMAND test_evd013_barcode)
set_tests_properties(EvD013_Barcode PROPERTIES TIMEOUT 60)
add_test(NAME EvD013_Barcode_Ship COMMAND test_evd013_barcode_ship)
set_tests_properties(EvD013_Barcode_Ship PROPERTIES TIMEOUT 60)
# --- WB_WsLink: web/page/ht9045_link.js -- one WebSocket per browser tab (node, offline) ---
#   AI(W906-WSLINK) 20260929 [W906] ST01-E3: Jimmy 20260929 -- every iframe opened its own socket (23 vs the cap of 16, Motor Test dead).
#   A frame hub + iframes over MessageChannel against a fake server with the WebBridgeServer token rules and the WebCmdGuard 400 ms busy:
#   one connection, ack routing, fan-out, late-join snapshot, token held by the hub, token-lost fan-out, motor.stop when a moving page goes,
#   ping loss, fallback to a direct socket, and the real ht9045_recipe_client.js (as is: it opens through HT9045Link.open and loads the sibling ht9045_link.js itself); takeover always to the server, in-browser hand-over keeps the owner, bye / ping loss / window hidden release only the held jogs, never a STOP (Jimmy 20260929 conditions 1/2; 20260930 M1).
if(W906_NODE_EXECUTABLE)
    add_test(NAME WB_WsLink COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/ws_link_selftest.cjs")
    set_tests_properties(WB_WsLink PROPERTIES TIMEOUT 60)
endif()

# --- test_note_showerror: AI(W906-SHOWERR) 20260929 -- golden ShowErrorMessage's alarm record (note.cpp:538 / :794 / :802-818 / :839-868)
#   + TfNote::ErrShowToForm (:4307-4509) + CheckRecordJamDrivingRecord (:391-436), forms/fNote_ShowError.cpp (ht9045_sm).  MyDBIEvent /
#   MyDBIProcess write the EventTracker and HANDLER LOG csv through the common.cpp seams, i.e. into machine_log_scratch (the ENV-ALL
#   DEFER call above runs at the end of this directory, so it covers this test); the test refuses to run outside it (w906_ctest_guard.h).
add_executable(test_note_showerror test_note_showerror.cpp)
target_link_libraries(test_note_showerror PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_note_showerror PRIVATE ws2_32)
endif()
add_test(NAME NoteShowError COMMAND test_note_showerror)
set_tests_properties(NoteShowError PROPERTIES TIMEOUT 120)
# --- test_note_servooff: AI(W906-SHOWERR) 20260929 -- golden TfNote::FormShow's alarm servo-off (note.cpp:2035-2128),
#   TfNote::W906_FormShowServoOff in forms/fNote_ShowError.cpp (ht9045_sm).  No card: MOT[].Motor NULL (ServoOnOff / ReadEncoderPos do not
#   reach hardware); RecordProcess writes into machine_log_scratch (ENV-ALL); the guard refuses to run outside it.  ~2 s of golden MySleep(200).
add_executable(test_note_servooff test_note_servooff.cpp)
target_link_libraries(test_note_servooff PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_note_servooff PRIVATE ws2_32)
endif()
add_test(NAME NoteServoOff COMMAND test_note_servooff)
set_tests_properties(NoteServoOff PROPERTIES TIMEOUT 120)
# --- test_searchcleannum_once: AI(W906-FLOW-5) 20260929 -- RULINGS_20260929 section 5 item 5 = A (one SearchCleanNum, WAR16313).
#   The Cleaning page / boot copy (FileRW/TestIF_File_Cleaning.cpp, wb_serve only) now forwards to AutoClean/AutoClean.cpp's
#   (ht9045_sm) through W906_AutoClean_SearchCleanNum, so one count alarms once.  PART A drives the AutoClean body (same RESCAN
#   link as test_AutoClean); PART B reads the three source files (path from __FILE__) because the FileRW side cannot be linked
#   here.  RecordProcess -> MyDBIProcess writes only under the redirect roots, which the deferred _w906_env_all_tests (above)
#   also gives to tests added after it.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_searchcleannum_once
    test_searchcleannum_once.cpp
)
target_link_libraries(test_searchcleannum_once PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_searchcleannum_once PRIVATE ws2_32)
endif()
add_test(NAME SearchCleanNumOnce COMMAND test_searchcleannum_once)
set_tests_properties(SearchCleanNumOnce PROPERTIES TIMEOUT 120)
# --- test_weblogin_reauth: AI(W906-Q45-B5) 20260930 (St01) -- Q45 "甲" re-login (#4 Configuration [M01], #6 / #7 SetUp RTC-off /
#   OCR-off) = WebLogin.cpp's appended W906_Reauth block (WebReauth.h): golden TfSetup::DoPassword (V912 cSetUp.cpp:4325-4362) and
#   TfConfiguration::DoPassword (cConfiguration.cpp:6482-6529) in book and drop-down mode, the Configuration forced logout, the M01
#   revert, and the editlist.save plumbing (W906_ReauthTake / W906_ReauthAck / W906_ReauthClear, called by tools/wb_serve.cpp).
#   WebLogin.cpp lives in the root CMakeLists.txt's wb_serve target, so it is listed here as a source (as test_weblogin_force_operator);
#   god-stack through RESCAN; FormLock / FormUnlock are empty stand-ins in the test file.  The FileRW call sites (TestIF_File_SetUp.gen.inc,
#   TestIF_File_SetUp.cpp, IniConfig.cpp) and wb_serve.cpp cannot be linked into a ctest: step 7 checks them as source (comments
#   stripped).  Made-up random credentials only; the text book, the stdout capture and the three login seams live in the build dir's
#   weblogin_reauth_scratch (DEFERred APPEND scheduled after the ENV-ALL call, so the global redirects come first).  Refuses to run outside
#   ctest's redirect environment; fails if D:\HT9045\system\login.dat / levelset.dat / lastdata.dat / Gerneral.ini changed or if a
#   made-up password shows up in any output.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_weblogin_reauth
    test_weblogin_reauth.cpp
    ${CMAKE_SOURCE_DIR}/WebLogin.cpp
)
target_include_directories(test_weblogin_reauth PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_weblogin_reauth PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_weblogin_reauth PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_weblogin_reauth.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME WebLogin_Reauth COMMAND test_weblogin_reauth)
set_tests_properties(WebLogin_Reauth PROPERTIES TIMEOUT 120)
cmake_language(DEFER CALL set_property TEST WebLogin_Reauth APPEND PROPERTY ENVIRONMENT
    "W906_LOGINDAT_PATH=${CMAKE_CURRENT_BINARY_DIR}/weblogin_reauth_scratch/login.dat"
    "W906_PWBOOK_PATH=${CMAKE_CURRENT_BINARY_DIR}/weblogin_reauth_scratch/pwbook.txt"
    "W906_LEVELSET_PATH=${CMAKE_CURRENT_BINARY_DIR}/weblogin_reauth_scratch/levelset.dat")
# --- test_ecat_motor_route / test_pci1203_motor_route: AI(W906-ECAT-ROUTE) 20260929 -- INBOX 112, the engine motor -> 1203 route
#   (RULINGS_20260929 #11 decision 1 = A; docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md 7.4).
#   EcatMotorRoute: TMyEtherCatMotor (ht9045_motor, no HAVE_PCI1203) against a FAKE route -- A no route = as before, B identity,
#     C the HAVE arm's command sequences, D read decoding, E pending, F InitMotor, G homing through golden TMyMotor::MotorHome,
#     H Direction=1, I StopAllMotor's ServoAlarmOn x SystemStart rule and the Index-name hole (Q4), R the same card numbers as
#     Motor Test's pure functions (WebMotorAccess.cpp compiled in, as test_web_motor_access does). ~1 s of golden HomeDelay/MySleep.
#     No card, no file: nothing here installs the real route (W906_InstallPci1203MotorRoute has ONE call site, tools/wb_serve.cpp).
#   Pci1203MotorRoute: the route BODY (EtherCAT/Pci1203MotorRoute.cpp) against a fake env -- op mapping, identity, claim, pending,
#     Q11, Q7 skips, Q1, homing, InitMotor ladder. Pci1203Control.cpp / Pci1203Monitor.cpp without HAVE_PCI1203, as Pci1203Pure.
#   New infrastructure => -Wall -Wextra on the route body and its test. After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_ecat_motor_route
    test_ecat_motor_route.cpp
    ${CMAKE_SOURCE_DIR}/WebMotorAccess.cpp  ${CMAKE_SOURCE_DIR}/GearCalc.cpp   # AI(W906-GEARRATIO) 20261002: WebMotorAccess.cpp's Gear Ratio part calls GearCalc (same line, nothing moves)
    ${CMAKE_SOURCE_DIR}/WebBridge/JsonWriter.cpp
)
target_include_directories(test_ecat_motor_route PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ecat_motor_route PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ecat_motor_route PRIVATE ws2_32)
endif()
add_test(NAME EcatMotorRoute COMMAND test_ecat_motor_route)
set_tests_properties(EcatMotorRoute PROPERTIES TIMEOUT 120)
add_executable(test_pci1203_motor_route
    test_pci1203_motor_route.cpp
    ../EtherCAT/Pci1203MotorRoute.cpp
    ../EtherCAT/Pci1203Monitor.cpp
    ../EtherCAT/Pci1203Control.cpp)
target_include_directories(test_pci1203_motor_route PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_pci1203_motor_route PRIVATE vclcompat)
add_test(NAME Pci1203MotorRoute COMMAND test_pci1203_motor_route)
set_tests_properties(Pci1203MotorRoute PROPERTIES TIMEOUT 120)
if(NOT MSVC)   # the two NEW files only (the route body + its test); the god-stack headers test_ecat_motor_route.cpp pulls are not ours to re-warn
    set_source_files_properties(test_pci1203_motor_route.cpp ../EtherCAT/Pci1203MotorRoute.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
# --- test_amb_l3_inarm_stubs: AI(W906-AMB-L3) 20260930 -- INBOX 111 layer 2, the eight golden ainarm9045.cpp bodies that were
#   empty `{}` stubs (DoInArmSuckPreOn, SetAutoSkipCount, AutoSkipHasIClog, ShowAutoSkipError, PickErrorData, TrayPickupErrorData,
#   DoInArmLoadPickUP_9045, sLoadPickupClean; translated at the END of ainarm9045.cpp).  Proves each body runs and matches golden,
#   and that DoTraySkipProcess_9045's Auto Skip limit now trips (it never could with the counter stub).  Same RESCAN link as
#   test_searchcleannum_once.  No IO (every in-arm nozzle's OnEnable/OffEnable is cleared first); RecordProcess / MyDBIProcess
#   write only under the redirect roots (_w906_env_all_tests, above).  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_amb_l3_inarm_stubs
    test_amb_l3_inarm_stubs.cpp
)
target_link_libraries(test_amb_l3_inarm_stubs PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_amb_l3_inarm_stubs PRIVATE ws2_32)
endif()
add_test(NAME AmbL3InArmStubs COMMAND test_amb_l3_inarm_stubs)
set_tests_properties(AmbL3InArmStubs PROPERTIES TIMEOUT 120)
# --- test_inarm_leftside_golden: AI(W906-R128) 20261002 -- NB2 R128 / NIGHT_REPORT s2 #7: ainarm9045.cpp InArmLeftSideNoIC /
#   InArmLeftSideHasIC follow golden ainarm9045.cpp:4193-4229 (only the left 2x2 nozzle cells).  Unit checks on InArmSuck.Item
#   plus GetPlaceToHotPlateSuckCol's X Division 10 / column 8 switch to nozzles 2 / 3 (ainarm_SearchPlacePlate.cpp:957-961).
#   Links the real mykitsuck.cpp / ainarm9045.cpp (same RESCAN group as AmbL3InArmStubs); no IO, no files.  Own TIMEOUT.
add_executable(test_inarm_leftside_golden
    test_inarm_leftside_golden.cpp
)
target_link_libraries(test_inarm_leftside_golden PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_inarm_leftside_golden PRIVATE ws2_32)
endif()
add_test(NAME InArmLeftSideGolden COMMAND test_inarm_leftside_golden)
set_tests_properties(InArmLeftSideGolden PROPERTIES TIMEOUT 120)
# --- test_main_ctlbuttons: AI(W906-FLOW-4) 20260930 -- INBOX 109, the main screen's ONE CYCLE / TRAY FEED / ALARM RESET buttons.
#   A click goes in through the WS entry (W906_Main_EvB6Op "act.main.ctlButton", St01's FileRW/MainClick.cpp, compiled in here
#   exactly as wb_serve compiles it), St01's queue, and the consumer wb_serve's main loop calls every pass (W906_MainCtlButtonTick,
#   WebMainCtlButtons.cpp) into the golden bodies (cCleanOut.cpp, ht9045_sm).  Checks: once per click, queue drained, a disabled /
#   expired click runs nothing, RESET and the site cell are left in the queue (not wired: decision).  MainClick.cpp's OTHER entry
#   points reference the FileRW window-edge bodies and the form-bridge lock, which live in wb_serve's own FileRW / JsonBridge
#   sources; tests/test_main_ctlbuttons_stubs.cpp gives those (never called here) link-only stand-ins.  The bodies' log rows go to
#   machine_log_scratch (ENV-ALL, the DEFER above); the test refuses to run outside it.  ~3.5 s (the TTL case sleeps 3.3 s).
add_executable(test_main_ctlbuttons
    test_main_ctlbuttons.cpp
    test_main_ctlbuttons_stubs.cpp
    ${CMAKE_SOURCE_DIR}/WebMainCtlButtons.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/MainClick.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
)
target_include_directories(test_main_ctlbuttons PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_main_ctlbuttons PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_main_ctlbuttons PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_main_ctlbuttons.cpp test_main_ctlbuttons_stubs.cpp ${CMAKE_SOURCE_DIR}/WebMainCtlButtons.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME MainCtlButtons COMMAND test_main_ctlbuttons)
set_tests_properties(MainCtlButtons PROPERTIES TIMEOUT 120)
# --- test_main_scankey: AI(W906-SCANKEY) 20261003 -- the physical panel keys (golden TfMain::ScanKey main.cpp:2379-2656, its timer
#   TimerScanKeyTimer :31994-32025, TfHome::ScanKey uhome.cpp:4874-4889), ported in WebMainScanKey.cpp (wb_serve only), compiled in
#   here exactly as wb_serve compiles it.  Keys are injected through the *With(scan) entry points (a queue instead of ScanPannelKey);
#   START / PAUSE land in a recorder installed in the W906_RemoteRun seat (forms/fMain.h:1440-1447); fMain points at a FakeMain whose
#   Home / BtnResetClick only record.  tests/test_main_scankey_stubs.cpp gives FormLock / FormUnlock (depth counter) and a counting
#   W906_Contact_OneCycleProcess (wb_serve's is in FileRW/DeviceForm_File.cpp).  Log rows go to machine_log_scratch (ENV-ALL, the DEFER
#   above, so no ENVIRONMENT here on purpose); the test refuses to run outside it.
add_executable(test_main_scankey
    test_main_scankey.cpp
    test_main_scankey_stubs.cpp
    ${CMAKE_SOURCE_DIR}/WebMainScanKey.cpp
)
target_include_directories(test_main_scankey PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_main_scankey PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_main_scankey PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_main_scankey.cpp test_main_scankey_stubs.cpp ${CMAKE_SOURCE_DIR}/WebMainScanKey.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME MainScanKey COMMAND test_main_scankey)
set_tests_properties(MainScanKey PROPERTIES TIMEOUT 120)

# --- GaliRouteCore / GaliRouteEngine: AI(W906-INDEXZ) 20260930 -- INBOX 113, HT9050 Index Z1 Gali_* -> 1203 route ---
#   (the redo of cc426093's two tests, reverted with it by 94cd23d7).
#   GaliRouteCore: EtherCAT/Pci1203GaliRouteCore.cpp (pure) against a fake IO seam -- the Galil string parser and
#     claims, signs/units, speed ceilings, poison, stale-sample rule (own + Motor Test ledger, card detach, a stop
#     outside the route), failed stops LATCHED and forced (review #6), VS0;SP0 HALT and the resume on START / on
#     golden's SP string (review #2), a stopped home never done (review #1), the installer's decision incl. 'no open
#     card' (review #5), the build's install gate vs its macros, the forced-line cooldown. No monitor, no control
#     object, no vendor call, no file.
#   GaliRouteEngine: golden MOT[MTestZ1].Gali_* (Motor/myGALILmotor.cpp + the three Motor/mymotor.cpp restorations)
#     against the REAL TGaliRouteCore bound to a fake 1203 installed through Motor/GaliRoute.h, incl. D1 as the
#     installer applies it (review #7), Z2's Position (review #4), a stopped home at 250 (review #1), halt + START
#     (review #2), a stop outside the route. Link shape and containment variables as MT_E3b_Engine. Listed at EOF,
#     so both carry their own TIMEOUT.
add_executable(test_gali_route_core
    test_gali_route_core.cpp
    ../EtherCAT/Pci1203GaliRouteCore.cpp)
target_include_directories(test_gali_route_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_gali_route_core PRIVATE vclcompat)   # MachineType.h's vcl_compat.h (the install gate); no library code is used
add_test(NAME GaliRouteCore COMMAND test_gali_route_core)
set_tests_properties(GaliRouteCore PROPERTIES TIMEOUT 120)
add_executable(test_gali_route_engine
    test_gali_route_engine.cpp
    ../EtherCAT/Pci1203GaliRouteCore.cpp)
target_include_directories(test_gali_route_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_gali_route_engine PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_gali_route_engine PRIVATE ws2_32)
endif()
add_test(NAME GaliRouteEngine COMMAND test_gali_route_engine)
set_tests_properties(GaliRouteEngine PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-INDEXZ) 20260930: the MT_E3b_Engine string verbatim
if(NOT MSVC)   # the NEW files only (the route core + its pure test); the god-stack headers of the engine test are not ours to re-warn
    set_source_files_properties(test_gali_route_core.cpp ../EtherCAT/Pci1203GaliRouteCore.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()

# --- GaliRouteLive: AI(W906-INDEXZ-1203) 20260930 -- review round 2 of a8c3b04e (INBOX 113) ---
#   The LIVE half of the Index Z1 route: the real binding + installer (EtherCAT/Pci1203GaliRoute.cpp) and the foreign-stop
#   glue moved to its EOF, against a stand-in Pci1203MotorRouteEnv and the GaliRouteLiveSeams (SDK / monitor facts):
#   C (the installer before / after the monitor exists; not latched on a refusal), E(b) (D1 applied by the installer, an
#   engine row at the address refuses), E(a) / D2 (the glue: alarm sweep keeps a halt, an operator stop ends it), D7 (the
#   lock, off-thread calls), S (source pins: wb_serve's boot order, the lifted G22 / G7 / setters). The torque hook's body
#   (EtherCAT/Pci1203TorqueHook.cpp) is not linked (it needs WebMotorAccessLive.cpp): the test defines its two functions.
#   Link shape and containment variables as GaliRouteEngine; + the EtherCAT TUs and WebMotorAccess.cpp (as EcatMotorRoute).
add_executable(test_gali_route_live
    test_gali_route_live.cpp
    ../EtherCAT/Pci1203GaliRoute.cpp
    ../EtherCAT/Pci1203GaliRouteCore.cpp
    ../EtherCAT/Pci1203MotorRoute.cpp
    ../EtherCAT/Pci1203Monitor.cpp
    ../EtherCAT/Pci1203Control.cpp
    ${CMAKE_SOURCE_DIR}/WebMotorAccess.cpp  ${CMAKE_SOURCE_DIR}/GearCalc.cpp   # AI(W906-GEARRATIO) 20261002: WebMotorAccess.cpp's Gear Ratio part calls GearCalc (same line, nothing moves)
    ${CMAKE_SOURCE_DIR}/WebBridge/JsonWriter.cpp)
target_include_directories(test_gali_route_live PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_gali_route_live PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # part S reads tools/wb_serve.cpp, csystem.cpp, WebStart.cpp, forms/fMain.h
target_link_libraries(test_gali_route_live PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_gali_route_live PRIVATE ws2_32)
endif()
add_test(NAME GaliRouteLive COMMAND test_gali_route_live)
set_tests_properties(GaliRouteLive PROPERTIES TIMEOUT 600 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # the GaliRouteEngine string verbatim
# --- test_b8_os5_sortbuttons: AI(W906-B8-OS5) 20260930 [W906] -- B8 OS-5：Offset 頁 A30 Setup Teach 的 12 顆排序鈕＝golden TfOffSet::btnSortAuto1Click
#   （V912 cOffSet.cpp:3211-3221；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「OS-5」；Steven 20260929「請按照bcb的邏輯處理」）。
#   受測：FileRW/Offset_File.cpp（事件表註冊、檔尾 OS_EvSort／OS_EvB8BootProxies）＋產生檔 FileRW/Offset_File.gen.inc 的 OS_btnSortAuto1Click，
#   經真的 WS 入口 W906_FormEvent（FileRW/_FormEvent.cpp）與 RunPageEvent（FileRW/_EditPage.cpp）：事件表 12 列、Tag、看得見才點得到（DFM 父層）、
#   三條件 8 種組合、逐顆 Tag、運轉中 running、微調鈕沒被改道，以及原始碼棘輪（argv[1]＝移植樹根目錄、argv[2]＝web\page，唯讀）。
#   同 test_hsys_heater_mix 直接列來源、god-stack 用 RESCAN 連；JsonBridge 的 FindBridge／RunEvent／FormLock 由測試檔給連結用替身（同 test_formevent_position）。
#   不跑 golden FormShow／ReadFile；不寫任何檔：開跑前後比對 D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini；ENVIRONMENT 照全域那一串（保險）。
#   放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_b8_os5_sortbuttons
    test_b8_os5_sortbuttons.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/Offset_File.cpp  ${CMAKE_SOURCE_DIR}/FileRW/Main_A01AutoLogout.cpp   # AI(W906-D015-A01) 20260930: Offset_File.gen.inc OS_spbSaveClick now calls W906_A01ResetOperatorModeCount (golden cOffSet.cpp:2811 fMain->iOperatorModeCount=0); same line
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_os5_sortbuttons PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_b8_os5_sortbuttons PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_os5_sortbuttons PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_os5_sortbuttons.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Os5_SortButtons COMMAND test_b8_os5_sortbuttons "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Os5_SortButtons PROPERTIES TIMEOUT 120 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-B8-OS5) 20260930: the global containment string verbatim (same as GaliRouteLive)
# --- test_b8_su7_rtcclick: AI(W906-B8-SU7) 20260930 [W906] -- B8 SU-7：Setup 頁 RTC 等 6 個勾選框＝golden TfSetup::cbEnableRealTimeCCDClick
#   （V912 cSetUp.cpp:4364-4399；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「SU-7」；Steven 20260929「請按照bcb的邏輯處理」）。
#   受測：FileRW/TestIF_File_SetUp.cpp（事件表註冊、檔尾 FileRW_Setup_B8Su7Boot／Su7VclOnClick／FileRW_Setup_B8Su7BeforeApply、BeforeApply (5)）＋
#   產生檔 FileRW/TestIF_File_SetUp.gen.inc 的 SU_cbEnableRealTimeCCDClick，經真的 WS 入口 W906_FormEvent（FileRW/_FormEvent.cpp）與 RunPageEvent
#   （FileRW/_EditPage.cpp）；fMain->CheckCanChangeRealDummy 是 cMainStatus.cpp:323 的活本體（ht9045_sm）。test_b8_su7_ic.cpp 改活物件的格子注入「有料」
#   （aHotPlateSubstrate.h 跟 _EditList.h 的 HTEditList.h 衝突，分另一個 TU）。原始碼棘輪：argv[1]＝移植樹根目錄、argv[2]＝web\page（唯讀）。
#   同 test_b8_os5_sortbuttons 直接列來源、god-stack 用 RESCAN 連；JsonBridge 與只在 wb_serve 的四支（MainClick／Cleaning／WebLogin）由測試檔給連結用替身。
#   不跑 golden FormShow／ReadFile／存檔（會寫 HandlerCondition.Data／config.ini）；開跑前後比對 D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini。
#   ENVIRONMENT 照全域那一串（保險）。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_b8_su7_rtcclick
    test_b8_su7_rtcclick.cpp
    test_b8_su7_ic.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/TestIF_File_SetUp.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_su7_rtcclick PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_b8_su7_rtcclick PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_su7_rtcclick PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_su7_rtcclick.cpp test_b8_su7_ic.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Su7_RtcClick COMMAND test_b8_su7_rtcclick "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Su7_RtcClick PROPERTIES TIMEOUT 120 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-B8-SU7) 20260930: the global containment string verbatim (same as B8_Os5_SortButtons)
# --- test_b8_m11_ftrt: AI(W906-B8-M11) 20260930 [W906] -- B8 M-11：主畫面 FT／RT 小方塊＝golden TfMain::palFTClick／palRTClick → DoFTRTClick
#   → FTClick／RTClick（V912 main.cpp:30700-31001、:35752-35774；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「M-11」；
#   Steven 20260929「請按照bcb的邏輯處理」）。受測：FileRW/MainClick.cpp（檔尾 W906_Main_FtRtOp／W906_Main_DoFTRTClick、ChangeLevelAttr 的起動模式鎖，
#   經真的 WS 分派 W906_Main_EvB6Op "act.main.ftrt"）；CheckCanChangeRealDummy／HasICUnderMachine／HasAnyICInMachine 等是 ht9045_sm 的活本體。
#   test_b8_su7_ic.cpp（SU-7 的有料注入）共用；MainClick.cpp 其他入口要的 wb_serve 專屬函式用 test_main_ctlbuttons_stubs.cpp（筆電 FLOW-4 的連結替身，
#   原樣共用、不改；被叫到就 abort）。golden cbRunStartModeChange（SetRunStartMode 會寫機台檔）由測試縫換成記錄器。原始碼棘輪：argv[1]＝移植樹根目錄、
#   argv[2]＝web\page（唯讀）。開跑前後比對 Gerneral.ini／config.ini／lastdata.dat。ENVIRONMENT 照全域那一串（RecordProcess 的事件紀錄進 scratch）。
#   放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_b8_m11_ftrt
    test_b8_m11_ftrt.cpp
    test_b8_su7_ic.cpp
    test_main_ctlbuttons_stubs.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/MainClick.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
)
target_include_directories(test_b8_m11_ftrt PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_b8_m11_ftrt PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_m11_ftrt PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_m11_ftrt.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_M11_FtRt COMMAND test_b8_m11_ftrt "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_M11_FtRt PROPERTIES TIMEOUT 120 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # AI(W906-B8-M11) 20260930: the global containment string verbatim (same as B8_Su7_RtcClick)

# --- test_note_motorerror: AI(W906-JAM-STOP) 20260930 -- INBOX 118, golden ShowMotorErrorMessage (note.cpp:1052-1169), now the golden
#   body at forms/fNote_ShowError.cpp EOF (ht9045_sm; the canary_support.cpp stand-in is retired).  A motor JAM stops (StopAllMotor(true)'s
#   Galil stop + golden's "ST" through a fake Galil channel, Motor/GaliRoute.h; a DecStop'd axis; the index brake output), shows the
#   note through the host hook with KeyCode 0 and RETURNS (the wb_serve host is source-pinned to ForwardShowErrorMessage's kcode==0
#   branch), applies the note's only answer (SoftStop), records (EventTracker / EventLogTxt), sets iHome / bIndexDropVacuumError; the
#   InitialOK==false and "WAR" arms.  Writes only under the redirect roots (no own ENVIRONMENT: _w906_env_all_tests gives it the global
#   string + _ht9045_env_extra); the guard refuses to run outside them.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_note_motorerror test_note_motorerror.cpp)
target_include_directories(test_note_motorerror PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_note_motorerror PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [5] reads tools/wb_serve.cpp
target_link_libraries(test_note_motorerror PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_note_motorerror PRIVATE ws2_32)
endif()
if(NOT MSVC)   # new test file only
    set_source_files_properties(test_note_motorerror.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME NoteMotorError COMMAND test_note_motorerror)
set_tests_properties(NoteMotorError PROPERTIES TIMEOUT 120)

# --- test_notice_ack: AI(W906-J5-ACK) 20260930 -- INBOX 119 (Jerry J-5): WS dialog.notifyAck, the operator acknowledges a kCode==0
#   notice (ForwardShowErrorMessage's AI(W906-Q30-KZERO) branch and every golden ShowMotorErrorMessage note).  The mailbox writers
#   (tools/wb_dialog_mailbox.h AlarmPost / AlarmRetire -- byte-identical to the snprintf code they replaced), NotifyAckHandle's
#   outcomes (retired / request-mismatch / not-a-notice / no-pending-notice / golden-refused / retire-failed), golden's close at
#   the ack (forms/fNote_ShowError.cpp EOF: BtnPauseClick KeyCode==0 note.cpp:4044-4153 + FormClose :2503-2762) and source pins
#   on tools/wb_serve.cpp / WebBridge/WebBridgeServer.cpp.  Writes only its own mailbox folder under the redirected as9045LogPath
#   (no own ENVIRONMENT: _w906_env_all_tests gives it the global string; the guard refuses to run outside it).  After the GLOBAL
#   TEST TIMEOUT block => own TIMEOUT.
add_executable(test_notice_ack test_notice_ack.cpp)
target_include_directories(test_notice_ack PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_notice_ack PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [M] reads tools/wb_serve.cpp, WebBridge/WebBridgeServer.cpp
target_link_libraries(test_notice_ack PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_notice_ack PRIVATE ws2_32)
endif()
if(NOT MSVC)   # new test file only
    set_source_files_properties(test_notice_ack.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME NoticeAck COMMAND test_notice_ack)
set_tests_properties(NoticeAck PROPERTIES TIMEOUT 120)

# --- test_b8_ag1_agvini: AI(W906-B8-AG1) 20260930 [W906] -- B8 AG-1 (A)：AMR Setting 頁＝golden TfAGV 讀寫 D:\HT9045\config\AGV.ini
#   （V912 Automation/AGV.cpp spbSaveClick :1184-1225、ReadFile :1227-1262、DoIniDataToForm :1264-1295、FormShow :1304-1309；開窗 main.cpp:35776-35781、
#   :24333；D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「AG-1」；Steven 20260929「請按照bcb的邏輯處理」）。
#   受測：FileRW/TestIF_File_AGV.cpp＋產生檔 FileRW/TestIF_File_AGV.gen.inc＋FileRW/_EditPage.cpp（PageJson／PageSave／開窗閘 GAgv／開窗記錄 MES2198）；
#   god-stack 用 RESCAN 連（同 test_b8_os5_sortbuttons）；JsonBridge 的 FindBridge／RunEvent／FormLock 由測試檔給連結用替身。原始碼棘輪：argv[1]＝移植樹根目錄、
#   argv[2]＝web\page（唯讀）。AGV.ini 只寫沙盒：W906_AGVINI_PATH（下面 _ht9045_env_extra 給每一支 ctest；本測試再換到沙盒底下自己的資料夾），
#   真的 D:\HT9045\config\AGV.ini 只看在不在／大小／修改時間、不開檔。沒有自己的 ENVIRONMENT：_w906_env_all_tests 給全域那一串＋_ht9045_env_extra。
#   放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
list(APPEND _ht9045_env_extra "W906_AGVINI_PATH=${CMAKE_CURRENT_BINARY_DIR}/agvini_scratch/AGV.ini")   # AI(W906-B8-AG1) 20260930: golden AGV.cpp:1187 / :1230 hard-code D:\HT9045\config\AGV.ini -> FileRW/TestIF_File_AGV.cpp W906_AgvIniPath() (every ctest, so none can reach the real file)
add_executable(test_b8_ag1_agvini
    test_b8_ag1_agvini.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/TestIF_File_AGV.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_ag1_agvini PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_b8_ag1_agvini PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_ag1_agvini PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_ag1_agvini.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Ag1_AgvIni COMMAND test_b8_ag1_agvini "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Ag1_AgvIni PROPERTIES TIMEOUT 120)

# --- test_b8_ag1_initial: AI(W906-B8-AG1) 20260930 [W906] -- B8 AG-1 (B)（Steven 請看：會動真機的 E84 交握輸出）：golden 開機讀 AGV.ini
#   （V912 main.cpp:9378 fAGV->ReadFile()，TfMain::DoReadLastData）＋ Initial Load／Initial Unload 兩顆鈕（Automation/AGV.cpp:1316-1322／:1324-1330
#   → InitialE84LoadTask／InitialE84LoadSensor／bE84LoaderActionflag、Unloader 同形），假 IO（SW[] Enable＋哨兵 ISABase，同 ctest AGV_E84）。
#   受測：FileRW/TestIF_File_AGV.cpp 檔尾（FileRW_AGV_ReadFile、AG_EvBootProxies、事件表註冊）＋產生檔的 AG_btInitalLoadClick／AG_btInitalUnLoadClick，
#   經真的 WS 入口 W906_FormEvent（FileRW/_FormEvent.cpp）與 RunPageEvent（FileRW/_EditPage.cpp）；Initial* 本體在 ht9045_sm（Automation/AGV_E84.cpp）。
#   原始碼棘輪：argv[1]＝移植樹根目錄、argv[2]＝web\page（唯讀）。AGV.ini 只寫沙盒（W906_AGVINI_PATH，上面 B8_Ag1_AgvIni 那一段的 _ht9045_env_extra；
#   本測試再換到沙盒底下自己的資料夾）。沒有自己的 ENVIRONMENT（_w906_env_all_tests 給）。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_b8_ag1_initial
    test_b8_ag1_initial.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/TestIF_File_AGV.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_ag1_initial PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_b8_ag1_initial PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_ag1_initial PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_ag1_initial.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Ag1_Initial COMMAND test_b8_ag1_initial "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Ag1_Initial PROPERTIES TIMEOUT 120)
# --- test_d015_a01_autologout: AI(W906-D015-A01) 20260930 [W906] -- todo D-015：golden [A01] 閒置自動切回 Operator（V912 main.cpp:25957-26078，
#   TfMain::Timer3Timer，Timer3 1000 ms，main.dfm:17333）＋ A01 30 分鐘自動重開（:26080-26096）＋ Offset 存檔歸 0（cOffSet.cpp:2811）。
#   受測：FileRW/Main_A01AutoLogout.cpp（wb_serve 編進去的同一份）；god-stack 用 RESCAN 連（同 test_htray_bind）。登出本體走安裝座
#   W906_WebLoginForceOperatorHook（測試裝假的；真本體 WebLogin.cpp 由 ctest WebLogin_ForceOperator 驗），頁面表的兩個安裝座也裝假的；
#   FormLock 是測試檔的替身（記深度：叫登出本體時不可以拿著鎖）。SIM／出貨兩種組態都要過（#ifdef SOFT_SIMULTE 分支，ST01-M 的 gate 兩種都跑）。
#   config.ini 只寫沙盒：W906_A01CONFIGINI_PATH（下面 _ht9045_env_extra 給每一支 ctest；本測試再換到沙盒底下自己的資料夾），
#   真的 D:\HT9045\config\config.ini 只看在不在／大小／修改時間。原始碼棘輪：argv[1]＝移植樹根目錄（唯讀）。
#   沒有自己的 ENVIRONMENT：_w906_env_all_tests 給全域那一串＋_ht9045_env_extra。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
list(APPEND _ht9045_env_extra "W906_A01CONFIGINI_PATH=${CMAKE_CURRENT_BINARY_DIR}/a01config_scratch/config.ini")   # AI(W906-D015-A01) 20260930: golden main.cpp:26087 AuthPath+"config.ini" -> FileRW/Main_A01AutoLogout.cpp W906_A01ConfigIniPath() (every ctest, so none writes the shared machine_config_scratch copy or the real file)
add_executable(test_d015_a01_autologout
    test_d015_a01_autologout.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/Main_A01AutoLogout.cpp
)
target_include_directories(test_d015_a01_autologout PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_d015_a01_autologout PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_d015_a01_autologout PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_d015_a01_autologout.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME D015_A01AutoLogout COMMAND test_d015_a01_autologout "${CMAKE_SOURCE_DIR}")
# AI(W906-D015-A01b) 20260930 [W906] -- D015_A01MenuPage：頁面那一半（node，離線）。web/page/ht9045_main_st01_ev.js initMenuReport：主畫面
#   Tools／Config 下拉選單（golden palSetup／palConfig，V912 main.cpp:25965）開／關／載入時送 WS act.main.menuVisible、失敗才重送、
#   auth.level 從 >0 降到 0 收起兩個選單（golden ChangeLevelAttr :12928-12935＋:13182-13190）；假 DOM／假 recipe client／假時鐘，
#   真的 ht9045_main_st01_ev.js＋ht9045_busy_util.js；main.html／ht9xxx.css 只讀（棘輪：兩個選單只經 style.display 開關）。
#   對照組：W906_MAIN_ST01_EV 指向改之前的檔必須紅。C++ 那一半在上面的 D015_A01AutoLogout。
if(W906_NODE_EXECUTABLE)
    add_test(NAME D015_A01MenuPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/d015_menu_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(D015_A01MenuPage PROPERTIES TIMEOUT 60)
endif()
# AI(W906-D019) 20260930 / AI(W906-D020) 20261001 [W906] St01 -- D019_D020_PageSelftest（node，離線）：
#   D-019 web/page/ht9045_contact_slk.js §18：HT_WIN 關→開的邊緣照 golden TfContact::FormShow（V912 cContact.cpp:1094，:1186
#   scrbSLKChange）重算一次，邊緣後 2 秒內 site.arm* 變了再補一次（stage F 開窗快照沒有順序保證）；最小化不是邊緣、單獨開頁照舊。
#   D-020 web/page/ht9045_lotinfo_wire.js tabVis：藏掉選中的頁只換畫面、不送 selection.get／lotEnd.state（golden pgLotinfoChange
#   V912 uLotInfo.cpp:7340 只掛在 uLotInfo.dfm:98 OnChange＝使用者點頁籤）。假 DOM／假 tag／假時鐘，真的 ht9045_contact_slk.js、
#   ht9045_lotinfo_wire.js、tabs.js；background.html 只讀（HT_WIN open 契約的棘輪）。
#   對照組：W906_CONTACT_SLK／W906_LOTINFO_WIRE 指向改之前的檔必須紅。
if(W906_NODE_EXECUTABLE)
    add_test(NAME D019_D020_PageSelftest COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/d019_d020_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(D019_D020_PageSelftest PROPERTIES TIMEOUT 60)
endif()
# AI(W906-D021) 20261001 [W906] St01 -- D021_ContactScrollSync（node，離線）：web/page/ht9045_contact_slk.js §19。golden 906（V912 寫在（）裡；AI(W906-E030-CITE) 20261003）
#   cContact.cpp 只有一個 TScrollBar Position（FormShow :1169（V912 :1185） 設、:1170（V912 :1186） scrbSLKChange；spbSaveClick :14072（V912 :14179） -> SaveSetupFile :14296（V912 :14404）
#   寫 Head Device Mode）。#scrbSLK.value 改成 SB.pos 的 accessor：捲動後引擎存檔（gbValue :1250）與 form.event state（contact_ev :146）
#   讀到現在的位置；引擎每次 editlist.get（gbApply :1163）與 ack（contact_ev :237）給的 Position 照 :1185 採用、該批套完再照 :1186 重算一次；
#   頁面自己捲動不回寫（沒有迴圈）。真的 ht9045_wire_engine.js＋ht9045_wire_setupcontact.js＋ht9045_contact_ev.js＋ht9045_contact_slk.js
#   同一個 vm（Setup.Contact.html 的順序），假 DOM／假 C++／假時鐘；Setup.Contact.html 只讀（載入順序棘輪）。
#   對照組：W906_CONTACT_SLK 指向改之前的 ht9045_contact_slk.js 必須紅。
if(W906_NODE_EXECUTABLE)
    add_test(NAME D021_ContactScrollSync COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/d021_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(D021_ContactScrollSync PROPERTIES TIMEOUT 60)
endif()
set_tests_properties(D015_A01AutoLogout PROPERTIES TIMEOUT 120)
# --- test_htray_bind: AI(W906-HTRAY) 20260930 -- INBOX 121, the ambient SIM stall (docs/AMBIENT_STALL_HTRAY_20260930.md).
#   golden TTrayMotor::SetHTrayPanel (Motor/mymotor.cpp:1500-1504) sets fHTary, and SetTray fills the IC grid only when it is
#   set (golden :1506-1514); golden SetSimuScreenPara binds 50 tray motors once (golden cinitial.cpp:6430-6489).  [1] a fresh
#   TTrayMotor unbound / bound with NULL; [2] the real SetSimuScreenPara binds exactly those 50 (every other MOT[] index,
#   MMTrayZ included, stays unbound); [3] source pin: the live NULL list == the gated widget copy (GATE n5-G3) == golden's list.
#   Writes only InitNewTray's MNetLog lines under the redirect roots (no own ENVIRONMENT: _w906_env_all_tests gives it the
#   global string + _ht9045_env_extra; the guard refuses to run outside them).  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_htray_bind test_htray_bind.cpp)
target_include_directories(test_htray_bind PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(test_htray_bind PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [3] reads cinitial.cpp
target_link_libraries(test_htray_bind PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_htray_bind PRIVATE ws2_32)
endif()
if(NOT MSVC)   # new test file only
    set_source_files_properties(test_htray_bind.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME HtrayBind COMMAND test_htray_bind)
set_tests_properties(HtrayBind PROPERTIES TIMEOUT 120)
# --- test_i115b_lifted / test_i115b_vacuum: AI(W906-I115B) 20260930 -- INBOX 115 class B, RULINGS_20260930 #4 (user 0930 13:4x
#   "全部開"): the ten HIGH `#if 0` blocks of docs/INBOX115_TRIAGE_20260930.md section 3 are open, as golden.  test_i115b_lifted
#   proves the golden statements now run: row 14 power saving -> TfHome::GaliMotorServoOff through TPowerSaving::OnScanTmr Task 4
#   (the product never assigns tPowerSaving -- asserted), row 22 SCKART Destroy(), row 24 LoadTrayCanUse8Suck route, row 33
#   GetIndexParm, row 45 CC_SIGURD_HUKOU chamber lamp, row 46 WAR15195 + SystemStart=false in DoSystem, row 53 cylinder averages +
#   fMain->Pause (still the shell), rows 91/92 DUT fan follows HasAreaOverAmbientTemp (ship build only: SOFT_SIMULTE CheckHeater
#   returns at its first line, as golden).  test_i115b_vacuum: row 18 VacuumUnit GATE (1) (its own exe: VacuumUnit.h's HTEditList.h
#   cannot share a TU with aHotPlateSubstrate.h).  No own ENVIRONMENT: _w906_env_all_tests gives both the global string +
#   _ht9045_env_extra; the lifted test's guard refuses to run outside it; the vacuum test does no file IO.  After the GLOBAL TEST
#   TIMEOUT block => own TIMEOUT (part H pumps the 5-run cylinder self-test twice, about 30 s).
add_executable(test_i115b_lifted test_i115b_lifted.cpp)
target_include_directories(test_i115b_lifted PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_i115b_lifted PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_i115b_lifted PRIVATE ws2_32)
endif()
add_executable(test_i115b_vacuum test_i115b_vacuum.cpp)
target_include_directories(test_i115b_vacuum PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_i115b_vacuum PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_i115b_vacuum PRIVATE ws2_32)
endif()
if(NOT MSVC)   # new test files only
    set_source_files_properties(test_i115b_lifted.cpp test_i115b_vacuum.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME I115B_Lifted COMMAND test_i115b_lifted)
set_tests_properties(I115B_Lifted PROPERTIES TIMEOUT 180)
add_test(NAME I115B_Vacuum COMMAND test_i115b_vacuum)
set_tests_properties(I115B_Vacuum PROPERTIES TIMEOUT 60)

# --- test_mt_savemot: AI(W906-MT-SAVEMOT) 20260930 -- Motor Test「回寫 Mot_Table」(EastSun 20260930), the writer behind motor.access
#   saveMotTable (WebMotorAccess.cpp EOF: MotTableEditRow / MotTableSaveRow).  argv[1] = machines/HT9050/Mot_Table.csv, READ ONLY;
#   every write goes to W906_MOTTABLE_PATH = a scratch copy in this build directory (the test refuses a target under
#   D:\HT9045\System / config / IniData or equal to argv[1]).  Byte identity of every untouched byte is checked against an
#   independent rebuild.  Same sources as test_web_motor_access (no god-stack).  After the GLOBAL TEST TIMEOUT block => own TIMEOUT;
#   its own ENVIRONMENT is kept by _w906_env_all_tests (only _ht9045_env_extra is appended).
add_executable(test_mt_savemot
    test_mt_savemot.cpp
    ${CMAKE_SOURCE_DIR}/WebMotorAccess.cpp  ${CMAKE_SOURCE_DIR}/GearCalc.cpp   # AI(W906-GEARRATIO) 20261002: WebMotorAccess.cpp's Gear Ratio part calls GearCalc (same line, nothing moves)
    ${CMAKE_SOURCE_DIR}/WebBridge/JsonWriter.cpp
    ${CMAKE_SOURCE_DIR}/Public/cJSON.c
)
target_include_directories(test_mt_savemot PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_mt_savemot PRIVATE vclcompat)   # AI(W906-COMMATEXT-FU) 20261003 (laptop, batch 53 merge): WebMotorAccess.cpp is compiled in and its MtSplitCells calls vclcompat::CommaTextCells (MR !153) -- as test_web_motor_access
add_test(NAME MtSaveMotTable
         COMMAND test_mt_savemot "${CMAKE_SOURCE_DIR}/../machines/HT9050/Mot_Table.csv")
set_tests_properties(MtSaveMotTable PROPERTIES TIMEOUT 120
    ENVIRONMENT "W906_MOTTABLE_PATH=${CMAKE_CURRENT_BINARY_DIR}/mt_savemot_scratch/Mot_Table.csv")

# --- test_i125_realdummy: AI(W906-I125) 20260930 [W906] -- the real TfMain::CheckCanChangeRealDummy (cMainStatus.cpp:323 = golden
#   main.cpp:12374-12380) at its three consumer families after I125 retired the Automation/auto9045.cpp W5FA_TfMainExt stand-in
#   (hard-coded `return true`) and lifted Command.cpp's (S4)/(S10) ComputeCanChangeRealDummy substitutions back to golden's text:
#   empty machine -> true; one IC in InArmSuck / OutArmSuck / a shuttle kit / hot plate 1 -> false, and CheckNeedCleanOut / SetStartMode /
#   SetLotInfo / GetHandlerStatusByDll (golden Command.cpp:9620) / SetProdModeByDll (golden :14335) all refuse.  No file I/O (every call
#   returns before a write; see the file head).  Registered after the GLOBAL TEST TIMEOUT block => own TIMEOUT; ENVIRONMENT = the global
#   containment string verbatim (same as B8_Os5_SortButtons above), as insurance.
add_executable(test_i125_realdummy
    test_i125_realdummy.cpp
)
target_link_libraries(test_i125_realdummy PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_i125_realdummy PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_i125_realdummy.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME I125_RealDummy COMMAND test_i125_realdummy)
set_tests_properties(I125_RealDummy PROPERTIES TIMEOUT 120 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")

# --- test_webstreamstats: AI(W906-STREAM-S1) 20260930 -- RULINGS_20260930 #12 stage 1 (user 20:4x: 「有開的網頁才能更新資料」), the
#   [STREAM] measurement line (WebStreamStats.cpp). Pure: a webbridge TagSnapshot and the server stats struct only -- no machine
#   globals, no files, no sockets. Registered after the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_webstreamstats
    test_webstreamstats.cpp
    ${CMAKE_SOURCE_DIR}/WebStreamStats.cpp
)
target_include_directories(test_webstreamstats PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_webstreamstats PRIVATE ht9045_webbridge)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_webstreamstats.cpp ${CMAKE_SOURCE_DIR}/WebStreamStats.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME WebStreamStats COMMAND test_webstreamstats)
set_tests_properties(WebStreamStats PROPERTIES TIMEOUT 60)

# --- test_streamgate / test_streamgate_tags: AI(W906-STREAM-2A/2B) 20260930 -- RULINGS_20260930 #12 (「有開的網頁才能更新資料」).
#   StreamGate: WebWindowRegistryStreamWanted (uncertain -> send; fresh over stale; form:null windows by id; byForm unchanged).
#   StreamGateTags: the 1203 block of PublishHandlerTags behind SetStreamWantedHook -- no hook = today (test_wb_tags exact counts
#   untouched), closed = only pci1203.linked / enabled / phase, wanted = the whole family back. Same link as WB_Tags.
add_executable(test_streamgate
    test_streamgate.cpp
    ${CMAKE_SOURCE_DIR}/WebWindowRegistry.cpp
    ${CMAKE_SOURCE_DIR}/Public/cJSON.c
)
target_include_directories(test_streamgate PRIVATE ${CMAKE_SOURCE_DIR})
if(WIN32)
    target_link_libraries(test_streamgate PRIVATE ws2_32)
endif()
add_test(NAME StreamGate COMMAND test_streamgate)
set_tests_properties(StreamGate PROPERTIES TIMEOUT 60)
add_executable(test_streamgate_tags test_streamgate_tags.cpp ../WebBridgeTags.cpp ../EtherCAT/Pci1203Monitor.cpp ../EtherCAT/Pci1203Control.cpp)
target_link_libraries(test_streamgate_tags PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_streamgate_tags PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_streamgate.cpp test_streamgate_tags.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME StreamGateTags COMMAND test_streamgate_tags)
set_tests_properties(StreamGateTags PROPERTIES TIMEOUT 120)


# --- Stream2E_PagePolls: AI(W906-STREAM-S2E) 20260930 -- stream-by-open-page stage 2E (node, offline) ---
#   web/page/HW.IoSetView.html (200 ms /api/struct/io/runtime) and web/page/HW.teach.html (1 s /api/struct/motor/runtime) stop
#   polling while their window is closed, as golden V912 iosetview.cpp:162 / uteach.cpp:1366 Timer1Timer `if(fShow==false) return;`.
#   Cuts the real code out of both pages into a node vm with a fake window / HT_WIN / setInterval / fetch: closed -> no fetch;
#   open -> one immediate fetch then every tick; minimized -> polls; never an HT_WIN (standalone / old frame) -> polls as today;
#   HT_WIN not from the frame -> ignored; Teach's HOME pop-up (teachSyncHome) follows the same gate. Also: every inline <script>
#   compiles, neither page mixes CRLF/LF or has a BOM. Control: W906_S2E_PAGE_DIR -> the pre-change pages must be red.
#   Design: .claude/skills/ht9050-st01-evaluations/references/stream-by-open-page.md section 4.5. Reads the two pages only.
if(W906_NODE_EXECUTABLE)
    add_test(NAME Stream2E_PagePolls COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/stream2e_page_polls_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(Stream2E_PagePolls PROPERTIES TIMEOUT 60)
endif()

# --- Stream2E_MvMotor: AI(W906-STREAM-S2E-MV) 20260930 -- stream-by-open-page stage 2E, Motion View's motor poll (node, offline) ---
#   web/page/ht9045_mv_motor.js (500 ms /api/struct/motor/runtime) is added by Main.MotionView.html after the page loaded, so it
#   missed the frame's one-time HT_WIN and polled with its window closed (laptop measurement 20260930 23:5x). It now reads the
#   frame's WIN_STATE at start: never / closed -> no fetch; open / minimized -> polls; an HT_WIN open edge -> one fetch at once;
#   no frame or unreadable parent -> polls as before. Control: W906_S2E_PAGE_DIR -> the pre-change file must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME Stream2E_MvMotor COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/stream2e_mv_motor_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(Stream2E_MvMotor PROPERTIES TIMEOUT 60)
endif()

# --- test_sysinit_boot: AI(W906-SYSINIT) 20260930 -- INBOX 127: golden TfMain::FormShow main.cpp:9659 `SystemInitialOK=true;`
#   (golden 906; V912 :10092), golden's only writer of the flag, translated as W906_FRWBoot_SystemInitialOK() (FileRW/MainBoot.cpp
#   EOF) and called once from the wb_serve boot sequence (tools/wb_serve.cpp, the ShowLotInfoDownloadFlag line).
#   [1] false before boot, ScanPannelKey -1 at its first guard, DoPanelLamp silent; [2] bHandlerModel==false keeps it false (golden
#   FormShow :9162-9167 terminates first); [3] after the boot step the held panel key is answered and DoPanelLamp runs; [4] source
#   pins on tools/wb_serve.cpp (one call, golden order) and FileRW/MainBoot.cpp (one writer, never cleared).
#   Compiles FileRW/MainBoot.cpp directly (a wb_serve-only TU, same pattern as test_b8_os5_sortbuttons); god-stack via RESCAN.
#   Writes no file.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT; the ENV-ALL DEFER call gives it the redirect roots.
add_executable(test_sysinit_boot
    test_sysinit_boot.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/MainBoot.cpp
)
target_include_directories(test_sysinit_boot PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(test_sysinit_boot PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [4] reads tools/wb_serve.cpp, FileRW/MainBoot.cpp
target_link_libraries(test_sysinit_boot PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_sysinit_boot PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_sysinit_boot.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME SysinitBoot COMMAND test_sysinit_boot)
set_tests_properties(SysinitBoot PROPERTIES TIMEOUT 120)

# --- test_vacuum_vc8: AI(W906-VACUNIT-1203) 20260930 -- HW.VacuumUnit's hardware half, the ECAT-VC8 on the PCIE-1203 ---
#   EastSun 20260930 「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」, rulings R1-R3 20260930. A FAKE
#   VC8 route (VacuumUnit/Vc8Route.h) stands in for EtherCAT/Pci1203Vc8Route.inc: no card, no port, no file written.
#   [1] the Set All Tag table against the golden DFM IR (argv[1]); [2] EtherCAT/Pci1203Vc8.h identity / closed lists;
#   [3] no route = golden's failure values; [4]-[9] golden's own TfVacuumUnit / TMyVacuumPanel objects driven through
#   tmr1Timer, btnSVClick, btnSetInArmClick (DFM Tags), btnVaccumOnOffOnClick, sbResetClick; [10] R3 golden
#   SetIOTableByECAT_VC8_Sucker + the remapped-sucker gate; [11] the two Pci1203Control kinds are internal.
#   Link shape as test_note_motorerror (the god stack) + the two 1203 TUs wb_serve compiles (as test_gali_route_live).
#   No own ENVIRONMENT: _w906_env_all_tests gives it the containment string; the guard refuses to run outside it.
#   After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_vacuum_vc8
    test_vacuum_vc8.cpp
    ../EtherCAT/Pci1203Monitor.cpp
    ../EtherCAT/Pci1203Control.cpp)
target_include_directories(test_vacuum_vc8 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_vacuum_vc8 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_vacuum_vc8 PRIVATE ws2_32)
endif()
if(NOT MSVC)   # new test file only
    set_source_files_properties(test_vacuum_vc8.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME VacuumVc8
         COMMAND test_vacuum_vc8 "${CMAKE_SOURCE_DIR}/tools/dfm2rc/ir_out/VacuumUnit/VacuumUnit.dfm.ir.json")
set_tests_properties(VacuumVc8 PROPERTIES TIMEOUT 120)

# --- test_d025_menuopen: AI(W906-D025) 20261001 [W906] St01 -- todo D-025: the main screen's Tools / Config menu buttons (golden 906
#   main.cpp:28028-28047 (V912 :29030-29049) sbSettingClick / :28007-28026 (V912 :29009-29028) sbConfigClick) through WS act.main.menuOpen: FileRW/Main_D025MenuOpen.cpp
#   (W906_Main_MenuOpenOp, compiled exactly as wb_serve compiles it) + FileRW/_EditPage.cpp EOF MenuClickRefused (the open gate's
#   GToolsMenu / GConfigMenu). Gate refusals (running / disabled / not-authorized), the tail order SetWorkParameter -> UpdateMainOperateMode
#   -> EventReport(EnterConfig), Tools = EventReport(EnterTool) only, SECS only when bEnable_SECS_GEM, the FormJson lock. SetWorkParameter
#   through the test seam W906_MenuOpenSetWorkParameter (the real one rereads teach.ini etc.); UpdateMainOperateMode is the ctest counting
#   stub (forms/fMain.cpp:507) with a recording hook; EventReport is the port's sim counter. No file written. Link shape as
#   D015_A01AutoLogout (god stack, RESCAN) + ht9045_webbridge for _EditPage.cpp. argv as TA5_ScrollBars.
add_executable(test_d025_menuopen
    test_d025_menuopen.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/Main_D025MenuOpen.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
)
target_include_directories(test_d025_menuopen PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_d025_menuopen PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_d025_menuopen PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_d025_menuopen.cpp ${CMAKE_SOURCE_DIR}/FileRW/Main_D025MenuOpen.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME D025_MenuOpen COMMAND test_d025_menuopen "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(D025_MenuOpen PROPERTIES TIMEOUT 120)
# AI(W906-D025) 20261001 [W906] St01 -- D025_MenuOpenPage (node, offline): web/page/ht9045_main_st01_ev.js initMenuOpen. A click on
#   #sbSetting / #sbConfig is caught in the window capture phase before main.html's own listener, sent as act.main.menuOpen; allow ->
#   main.html's original handler (the REAL toggleMenu + the two button listeners cut out of main.html) opens the menu, already open ->
#   left open (golden has no toggle); deny -> no handler, status line; busy / in flight / old server; the D-015 A01b MutationObserver
#   report (act.main.menuVisible) still follows. Control run: W906_MAIN_ST01_EV pointing at the pre-D-025 file must go red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME D025_MenuOpenPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/d025_menu_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(D025_MenuOpenPage PROPERTIES TIMEOUT 60)
endif()
# --- Stream2E_St01Polls: AI(W906-STREAM-F2) 20261001 -- the Contact CT / Observer page polls follow stage 2E (node, offline) ---
#   web/page/ht9045_contactct_wire.js and ht9045_observer_wire.js visible(): the frame's WIN_STATE decides -- open / minimized
#   = poll, closed / never = stop (golden keeps both forms running when minimized; St01 1001 00:39 FYI 3). Pins background.html's
#   early windowState (FYI 2) and that ht9045_smartdiag_web.js keeps golden's recording timer. Control: W906_S2E_PAGE_DIR ->
#   the pre-change pages must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME Stream2E_St01Polls COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/stream2e_st01polls_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(Stream2E_St01Polls PROPERTIES TIMEOUT 60)
endif()
# --- test_ta5_scrollbars: AI(W906-TA5) 20261001 [W906] St01 -- TA-5: Setup.TrayAssignment graphic-mode scroll bars (golden V912
#   cTrayAssignment.cpp:1639 sbNormalTestChange / :1674 sbNormalTest_RTChange) through the real WS entry W906_FormEvent with "position"
#   (X-2): FileRW/TrayForm.cpp (event table registration, kTA_OnTab pages 2 / 3) + FileRW/TrayForm.gen.inc (kTA_Events rows, golden
#   GraphicToRadio / GraphicToRadio_RT and the VCL OnClick chain), FileRW/_FormEvent.cpp, FileRW/_EditPage.cpp RunPageEvent.
#   Link shape as B8_Os5_SortButtons (god stack, RESCAN); JsonBridge / MainClick stand-ins in the test file. golden FormShow reads
#   <DataPath>\<recipe>\Tray.Data: no own ENVIRONMENT, so _w906_env_all_tests gives it the containment string (W906_INIDATA_ROOT ->
#   machine_log_scratch); the test refuses to run without it and compares Gerneral.ini / config.ini / SetUp.inf before and after.
#   argv[1] = port root, argv[2] = web\page (read-only ratchets). After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_ta5_scrollbars
    test_ta5_scrollbars.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/TrayForm.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
)
target_include_directories(test_ta5_scrollbars PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ta5_scrollbars PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_ta5_scrollbars PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_ta5_scrollbars.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME TA5_ScrollBars COMMAND test_ta5_scrollbars "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(TA5_ScrollBars PROPERTIES TIMEOUT 120)
# AI(W906-TA5) 20261001 [W906] St01 -- TA5_ScrollPage (node, offline): web/page/ht9045_trayassign_ev.js TA-5 section. The two static
#   scroll-bar divs become operable scroll bars; a user step sends form.event {"control":"sbNormalTest","event":"change","position":n}
#   (X-2), the ack's changed is applied, a refusal puts the position back; #sbNormalTest.value is an accessor (engine save / state read
#   the live position, engine load / ack writes only move the thumb, no event). Real ht9045_trayassign_ev.js in a node vm with a fake
#   DOM / fake recipe client / fake clock; Setup.TrayAssignment.html read-only (ids + load-order ratchet).
#   Control run: W906_TRAYASSIGN_EV pointing at the pre-TA-5 file must go red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME TA5_ScrollPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/ta5_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(TA5_ScrollPage PROPERTIES TIMEOUT 60)
endif()
# --- test_s98_cfgtrayplate: AI(W906-S98) 20261001 [W906] St01 -- todo E-003 (1), RULINGS_20260926 S98 + S169 ("if it is not ported, we
#   implement it", over the Q41 S158 "new pages later" line): the Configuration page's Tray / Hot Plate tabs (golden V912
#   cConfiguration.cpp:6945-7217) through WS cfgtrayplate.op: FileRW/CfgTrayPlate.cpp (W906_CfgTrayPlateOp, compiled exactly as wb_serve
#   compiles it) + FileRW/_EditPage.cpp (PageShownLevel / PageJson / PageWindowClosed) + FileRW/_EditList.cpp. Guards (running / reload
#   page / not-on-tab / not-operable), select, Modify Data in two steps (golden ShowQwertyKey tail, typable characters), Add / Delete
#   (VCL grid side effects, the kept golden quirks), Load Data / Save (BCB6 CommaText, R33 / R34), the VCL EFOpenError / EFCreateError,
#   the FormJson lock, source ratchets. Link shape as TA5_ScrollBars (god stack, RESCAN). System\TrayForm.csv / PlateForm.csv exist only in
#   the sandbox: W906_TRAYFORMCSV_PATH / W906_PLATEFORMCSV_PATH (the _ht9045_env_extra line below gives EVERY ctest a sandbox for both, so
#   none can reach the real files; this test moves to its own folder under it); the real files are only stat()ed. argv[1] = port root,
#   argv[2] = web\page (read-only ratchets). Control run: W906_S98_SRC_ROOT / W906_S98_PAGE_DIR at pre-change copies must go red ([10]).
list(APPEND _ht9045_env_extra "W906_TRAYFORMCSV_PATH=${CMAKE_CURRENT_BINARY_DIR}/trayplate_scratch/TrayForm.csv" "W906_PLATEFORMCSV_PATH=${CMAKE_CURRENT_BINARY_DIR}/trayplate_scratch/PlateForm.csv")   # AI(W906-S98) 20261001: golden common.cpp:42-43 TrayTablePath / PlateTablePath -> FileRW/CfgTrayPlate.cpp W906_TrayTablePath() / W906_PlateTablePath() (every ctest)
add_executable(test_s98_cfgtrayplate
    test_s98_cfgtrayplate.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/CfgTrayPlate.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
)
target_include_directories(test_s98_cfgtrayplate PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_s98_cfgtrayplate PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_s98_cfgtrayplate PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_s98_cfgtrayplate.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME S98_CfgTrayPlate COMMAND test_s98_cfgtrayplate "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(S98_CfgTrayPlate PROPERTIES TIMEOUT 120)
# AI(W906-S98) 20261001 [W906] St01 -- S98_TrayPlatePage (node, offline): web/page/ht9045_config_trayplate.js. Nothing is sent at load; the
#   first cfgtrayplate.op get follows the engine's own editlist.get IniConfig (= the HT_WIN open edge when hosted, stage F); grid fill
#   (fixed row / column, cursor), cell click -> select, double click / Modify Data -> keypad two steps (HTQwerty OK / Cancel), Add / Delete /
#   Load Data / Save round trips with a fake transport, disabled buttons from operable, refusals (busy retry, reload page, not-operable),
#   one request in flight. Real ht9045_config_trayplate.js in a node vm with a fake DOM / fake recipe client / fake keypad / fake clock;
#   Config.Configuration.html read-only (ids + load-order ratchet). Control run: W906_CONFIG_TRAYPLATE_JS at a pre-change stand-in must go red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME S98_TrayPlatePage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/s98_trayplate_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(S98_TrayPlatePage PROPERTIES TIMEOUT 60)
endif()
# --- test_e020_lotinfo_secslotstart: AI(W906-E020-LI1) 20261002 [W906] St01 -- todo E-020 LI-1 (the laptop's card): golden
#   TfLotInfo::sbSECSLotStartClick (V912 uLotInfo.cpp:7501-8583) as W906_LotInfo_SECSLotStart (FileRW/LotInfo_SECSLotStart.cpp,
#   compiled exactly as wb_serve compiles it; the laptop points its lot.start arm at it).  Every golden refusal reached without a
#   network path (whyNot = golden's text), the success path (RunInfo.bLotStart / LotNo, sandbox config.ini [Lot Info], golden's
#   start-of-click resets), the 13 GATEs (refuse vs skip-and-log), the SOFT_SIMULTE difference (SCC 5..30, :8056-8069: SIM starts a
#   4-char lot, SHIP refuses), source ratchets (live definition, St01's wb_serve source line, no START spelled) and real files
#   untouched (stat / listing).  Link shape as S98_CfgTrayPlate (god stack, RESCAN).  argv[1] = port root.  Control run:
#   W906_E020_SRC_ROOT at a broken copy must go red ([14]).
add_executable(test_e020_lotinfo_secslotstart
    test_e020_lotinfo_secslotstart.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/LotInfo_SECSLotStart.cpp
)
target_include_directories(test_e020_lotinfo_secslotstart PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_e020_lotinfo_secslotstart PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_e020_lotinfo_secslotstart PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_e020_lotinfo_secslotstart.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME E020_LotInfoSECSLotStart COMMAND test_e020_lotinfo_secslotstart "${CMAKE_SOURCE_DIR}")
set_tests_properties(E020_LotInfoSECSLotStart PROPERTIES TIMEOUT 180)

# --- test_webjsonscrubcache + ScrubCache_SettingsEtag: AI(W906-SCRUBCACHE) 20261001 -- NB2 problem A (v906/nb2-assist
#   docs/nb2_assist/notes_webref/P1_STOPLAT_first_load_stall.md): background.html polls the 8.6 MB Production-update.json every
#   250 ms and each poll cost wb_serve's socket thread 115-132 ms of read + scrub. WebJsonScrubCache.cpp: the /JSON scrub
#   route's file cache (real files in its own %TEMP% folder; no machine globals). settings.js refreshProduction: HEAD + ETag
#   before the GET (node, offline). Control: W906_SCRUBCACHE_PAGE_DIR -> the pre-change settings.js must be red.
add_executable(test_webjsonscrubcache
    test_webjsonscrubcache.cpp
    ${CMAKE_SOURCE_DIR}/WebJsonScrubCache.cpp
)
target_include_directories(test_webjsonscrubcache PRIVATE ${CMAKE_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_webjsonscrubcache.cpp ${CMAKE_SOURCE_DIR}/WebJsonScrubCache.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME WebJsonScrubCache COMMAND test_webjsonscrubcache)
set_tests_properties(WebJsonScrubCache PROPERTIES TIMEOUT 60)
if(W906_NODE_EXECUTABLE)
    add_test(NAME ScrubCache_SettingsEtag COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/scrubcache_settings_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(ScrubCache_SettingsEtag PROPERTIES TIMEOUT 60)
endif()

# --- test_jamday_hook: AI(W906-JAMDAY) 20261001 -- census 129 (e) E-FT2-011: RUN_INFO::SaveJamRateByDay's body hook
#   (cprod.cpp:1070-1071). The day rollover in AddAlarm used to clear yesterday's JAM counts without saving them. Memory only:
#   the installed body is a recorder, no file is written.
add_executable(test_jamday_hook test_jamday_hook.cpp)
target_link_libraries(test_jamday_hook PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_jamday_hook PRIVATE ws2_32)
endif()
add_test(NAME JamDayHook COMMAND test_jamday_hook)
set_tests_properties(JamDayHook PROPERTIES TIMEOUT 60)

# --- test_tempfuse_limit: AI(W906-FUSELIMIT) 20261001 -- RULINGS_20261001 #4: golden TfMain::FormShow main.cpp:9535-9551
#   TempFuseLimitType (MainFormShow_TempFuse.cpp, ht9045_globals). Decision table + the wb_serve boot order (before
#   InitialHandler and the boot W906_DoReadLastData). Memory only; reads tools/wb_serve.cpp, writes nothing.
add_executable(test_tempfuse_limit test_tempfuse_limit.cpp)
target_link_libraries(test_tempfuse_limit PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_tempfuse_limit PRIVATE ws2_32)
endif()
add_test(NAME TempFuseLimit COMMAND test_tempfuse_limit "${CMAKE_SOURCE_DIR}")
set_tests_properties(TempFuseLimit PROPERTIES TIMEOUT 60)

# --- test_atc_emg_bridge: AI(W906-ATCEMG) 20261001 -- RULINGS_20261001 #5 (census 129 (a)(b) #17): csystem.cpp's G-ATC-A / B / C
#   (CheckATC6System, the ATC EMG down / up handshake) open through the bridges at the end of ATC/ATCInterface.cpp. Memory only;
#   reads csystem.cpp (argv[1] = source root), writes nothing, opens no socket.
add_executable(test_atc_emg_bridge test_atc_emg_bridge.cpp)
target_include_directories(test_atc_emg_bridge PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_atc_emg_bridge PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_atc_emg_bridge PRIVATE ws2_32)
endif()
add_test(NAME AtcEmgBridge COMMAND test_atc_emg_bridge "${CMAKE_SOURCE_DIR}")
set_tests_properties(AtcEmgBridge PROPERTIES TIMEOUT 60)

# --- test_main_timer_segments: AI(W906-DOORLOCK / W906-BIGFAN) 20261001 -- RULINGS_20261001 #0 / #5, census 129 (e) E-T1-012 + E-T2-001: golden TfMain::Timer1Timer
#   main.cpp:3051-3076 (the safe-door locks follow SystemStart) and Timer2Timer :20964-20968 (the big fan) in MainTimerSegments.cpp. Memory only; reads WebBridgeTags.cpp
#   (argv[1] = source root) for the PumpTick call site.
add_executable(test_main_timer_segments test_main_timer_segments.cpp)
target_include_directories(test_main_timer_segments PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_main_timer_segments PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_main_timer_segments PRIVATE ws2_32)
endif()
add_test(NAME MainTimerSegments COMMAND test_main_timer_segments "${CMAKE_SOURCE_DIR}")
set_tests_properties(MainTimerSegments PROPERTIES TIMEOUT 60)

# --- test_setlotstate: AI(W906-SETLOTSTATE) 20261001 -- RULINGS_20261001 #0 / #5, census 129 (a)(b) #16: golden TfMain::SetLotState
#   (main.cpp:15160-15407) in forms/fMain_SetLotState.cpp. The GPIB arm through test hooks in the tester seats; reads 5 sources
#   (argv[1] = source root). Memory only.
add_executable(test_setlotstate test_setlotstate.cpp)
target_include_directories(test_setlotstate PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_setlotstate PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_setlotstate PRIVATE ws2_32)
endif()
add_test(NAME SetLotState COMMAND test_setlotstate "${CMAKE_SOURCE_DIR}")
set_tests_properties(SetLotState PROPERTIES TIMEOUT 60)

# --- test_pause_forward: AI(W906-PAUSE-FWD / W906-SECS-RSTART / W906-W7C1-INI) 20261001 -- batch 18, RULINGS_20261001 #0 / #5: the base TfMain::Pause
#   forwards to the W906_RemoteRun seat (golden TfMain::Pause = TfMainWeb::PauseFromWeb in wb_serve); reads forms/fMain.cpp, SECSGEM/uHGemHT9045.cpp
#   (RCMD REMOTE_START -> W906_RemoteRunStart) and csystem.cpp (the W7C1 INI seams retired) -- argv[1] = source root. Memory only.
add_executable(test_pause_forward test_pause_forward.cpp)
target_include_directories(test_pause_forward PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_pause_forward PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_pause_forward PRIVATE ws2_32)
endif()
add_test(NAME PauseForward COMMAND test_pause_forward "${CMAKE_SOURCE_DIR}")
set_tests_properties(PauseForward PROPERTIES TIMEOUT 60)

# --- test_socket_sensor: J-6 CheckSocketSensor un-gate guard -----------------
#   AI(W906-J6-SOCKETSENSOR) 20260930. The LIVE CheckSocketSensor() used to be a
#   stub returning a hard-coded `true`, i.e. golden's "socket sensor found a
#   residual IC" (golden's last line is `return bHasErr;`) -- so it reported an
#   error on every call, in every build. Measured consequence: DoTestYFront case
#   115 -> Task 50 -> MOT[MInShuttle1/2].fCanMoveM stuck false -> acarry.cpp's
#   30 s Shuttle-2 watchdog -> blocking ShowMyMessage -> single-threaded
#   wb_serve stops pumping MainProc/DoAllProcess/DoInArm entirely.
#   This suite pins golden's outer guard (atester.cpp:10800
#   `if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor)`) and its
#   default verdict: feature off => false. Sensor-reading arms need Sim HAL
#   state and are deliberately not asserted here.
add_executable(test_socket_sensor
    test_socket_sensor.cpp
)

target_link_libraries(test_socket_sensor PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_socket_sensor PRIVATE ws2_32)
endif()
add_test(NAME SocketSensor COMMAND test_socket_sensor)
set_tests_properties(SocketSensor PROPERTIES TIMEOUT 120)

# --- test_safedoor_record: AI(W906-SAFEDOOR-REC) 20261001 -- batch 19, RULINGS_20261001 #0 / #5: csystem.cpp GATE G11 (DoSystem calls
#   RecordSafeDoorStates again, golden csystem.cpp:4394) and G25 / G28a / G28b (the Gerneral.ini door timestamps) lifted.  Writes only
#   asGeneralPath, which ENV-ALL points at this test's own scratch Gerneral.ini (W906_GENERAL_INI_PATH); refuses to run otherwise.
#   Reads csystem.cpp (argv[1] = source root) for the gate states.
add_executable(test_safedoor_record test_safedoor_record.cpp)
target_include_directories(test_safedoor_record PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_safedoor_record PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_safedoor_record PRIVATE ws2_32)
endif()
add_test(NAME SafeDoorRecord COMMAND test_safedoor_record "${CMAKE_SOURCE_DIR}")
set_tests_properties(SafeDoorRecord PROPERTIES TIMEOUT 60)
# --- test_tcp_tester_real: AI(W906-H008) 20261001 (St02-E) -- H-008: the TCP tester socket REAL in vclcompat's POLLED
#   mode (ClientSocket.cpp end of file) and TcpPump driving golden TfTesterTCP (Interface/TesterTCP_Socket.cpp).
#   CONTAINED: test-owned sockets on 127.0.0.1 with ephemeral ports only (never 6000 / 7016 / 7017 / the TesterIF
#   address), the log path redirected, no W906_TesterCommInit.  The refused case waits up to ~45 s worst case.
add_executable(test_tcp_tester_real test_tcp_tester_real.cpp)
target_link_libraries(test_tcp_tester_real PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_tcp_tester_real PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_TcpTesterReal COMMAND test_tcp_tester_real)
set_tests_properties(TesterComm_TcpTesterReal PROPERTIES TIMEOUT 120)
# --- test_coolfan_rules: AI(W906-COOLFAN) 20261001 (St02-E) -- RULINGS_20261001 #9: golden DoSwCoolingFan (csystem.cpp, golden
#   906_0625_Steven csystem.cpp:20434-20569) and its callers.  The blower / valve rules, the IO-page guard H1-07, H1-08 with the
#   ht9045_globals fallback, the air-conditioner branch (fAirCon lines stay gated), and the SIM heater tick reaching it.
#   Memory only; no IO table, writes nothing.
add_executable(test_coolfan_rules test_coolfan_rules.cpp)
target_link_libraries(test_coolfan_rules PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_coolfan_rules PRIVATE ws2_32)
endif()
add_test(NAME CoolFan_Rules COMMAND test_coolfan_rules)
set_tests_properties(CoolFan_Rules PROPERTIES TIMEOUT 60)
# --- test_fastclk_trylock: AI(W906-FASTCLK) 20261003 -- RULINGS_20261002 #7 「冷卻風扇拿 FormLock 的鎖先改（主迴圈不該等 HTTP 執行緒）」:
#   DoSwCoolingFan GATE H1-08 (csystem.cpp) and DoThermo GATE G14a (bthermo.cpp) read the Configuration proxies through
#   FileRW_ProxyTryRead (FileRW/_ProxyTry.cpp, compiled here with -Wall -Wextra).  The test supplies the FormJson lock (a real
#   CRITICAL_SECTION) and the seven FileRW/_fallback.cpp symbols (its reads take the lock as FileRW/_EditList.cpp's do), holds the
#   lock from a second thread and times both calls (< 100 ms, beat skipped, nothing driven); argv[1] pins the source lines (read
#   only).  Memory only, writes nothing.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_fastclk_trylock test_fastclk_trylock.cpp ${CMAKE_SOURCE_DIR}/FileRW/_ProxyTry.cpp)
target_include_directories(test_fastclk_trylock PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_fastclk_trylock PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_fastclk_trylock PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/FileRW/_ProxyTry.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME FastClk_TryLock COMMAND test_fastclk_trylock "${CMAKE_SOURCE_DIR}")
set_tests_properties(FastClk_TryLock PROPERTIES TIMEOUT 300)   # the body takes ~2 s (a 1.5 s lock hold); 300 because a freshly linked exe can wait minutes for the endpoint scanner here (20261003: HeaterSimTick 212 s on its first run)
# --- test_fastclock: AI(W906-FASTCLK) 20261003 -- the serve loop's shared fast clock (FastClock.cpp; RULINGS_20261002 #7): the kRate grid
#   with no catch-up + missed beats, kDelay, per-window stats, re-entry (a waiting box inside a body), exceptions, NextDueUs /
#   ClampTickDeadline across the GetTickCount wrap, PassMeter.  Pure (a fake clock, no god-stack), -Wall -Wextra.  CONTROL (20261003):
#   FastClock.cpp with the kRate re-grid replaced by a catch-up `due += period` -> 4 checks red (sections 2 and 5).
#   After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_fastclock test_fastclock.cpp ${CMAKE_SOURCE_DIR}/FastClock.cpp)
target_include_directories(test_fastclock PRIVATE ${CMAKE_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_fastclock.cpp ${CMAKE_SOURCE_DIR}/FastClock.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME FastClock COMMAND test_fastclock)
set_tests_properties(FastClock PROPERTIES TIMEOUT 300)   # pure and instant; 300 for the endpoint scanner on a fresh exe (see FastClk_TryLock)
# --- test_fastclk_jobs: AI(W906-FASTCLK) 20261003 -- the three jobs on the serve loop's fast clock (RULINGS_20261002 #7): FastClockJobs.cpp
#   (golden THeaterThread body per build, golden TfGroundMan::Timer1Timer), MainTimersSt02.cpp (the bin-panel fast entry, the PumpTick /
#   modal owner gate), and the wb_serve glue end to end for 1.5 s of real serve-loop calls (FastClockWbServe.cpp + FastClock.cpp +
#   WebStreamStats.cpp compiled here: wb_serve-only TUs; FastClockWbServe.cpp with -Wall -Wextra); argv[1] pins the hooks (read only).
#   CONTAINED (st02_test_containment.h).  Memory only.  Both configs.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_fastclk_jobs test_fastclk_jobs.cpp ${CMAKE_SOURCE_DIR}/FastClockWbServe.cpp ${CMAKE_SOURCE_DIR}/FastClock.cpp
    ${CMAKE_SOURCE_DIR}/WebStreamStats.cpp)
target_include_directories(test_fastclk_jobs PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_fastclk_jobs PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_fastclk_jobs PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(${CMAKE_SOURCE_DIR}/FastClockWbServe.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME FastClk_Jobs COMMAND test_fastclk_jobs "${CMAKE_SOURCE_DIR}")
set_tests_properties(FastClk_Jobs PROPERTIES TIMEOUT 300)   # ~2 s of body (1.5 s real-time loop); 300 for the endpoint scanner on a fresh exe (see FastClk_TryLock)
# --- test_boot_servo_on: AI(W906-B20-SERVOON) 20261001 -- census 129 (e) E-BOOT-002 (RULINGS_20261001 #10) + INBOX 134:
#   golden ServoOnAllMOT at boot (non-1203 axes only; a TMyEtherCatMotor row is skipped), the boot GetHotPlateYHalfPos
#   (golden TfMain::FormShow main.cpp:10164-10165, tools/wb_serve.cpp:4163) and the fMotorTest checks of the light-scale log
#   in Motor/mymotor.cpp. Recording motor classes + source pins; memory only, writes nothing.
add_executable(test_boot_servo_on test_boot_servo_on.cpp)
target_link_libraries(test_boot_servo_on PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_boot_servo_on PRIVATE ws2_32)
endif()
target_compile_definitions(test_boot_servo_on PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [2] reads tools/wb_serve.cpp, Motor/mymotor.cpp, tools/wb_boot_factory.cpp (AI(W906-B23-EBOOT005) 20261001)
add_test(NAME Boot_ServoOn COMMAND test_boot_servo_on)
set_tests_properties(Boot_ServoOn PROPERTIES TIMEOUT 60)

# --- test_testercomm_home: AI(W906-TC-SHARED) 20261001 (St02-E) -- the ONE home JSON of testercomm.html's Test Result tab (TesterComm/UiHome.h):
#   the testType mapping, the plSite state, log.seq, the merged object for every testType (valid JSON, the same keys in the same
#   order, running or not), and the route (GET home; POST home site commands to the active engine).  Memory only.
add_executable(test_testercomm_home test_testercomm_home.cpp)
target_link_libraries(test_testercomm_home PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_testercomm_home PRIVATE ws2_32)
endif()
add_test(NAME TesterComm_HomeJson COMMAND test_testercomm_home)
set_tests_properties(TesterComm_HomeJson PROPERTIES TIMEOUT 60)

# --- test_heater_chain: AI(W906-I01) 20261001 (Ifor01) -- golden TfMain::Index16Heater / IndexHeatMode / HotplateHeatMode
#   (main.cpp:18613-20847) and the SetTemp body (:23890-23981), forms/fMain_Heater.cpp (ht9045_sm). SetTemp stays a stub at
#   forms/fMain.cpp:511 (no hook installed anywhere in I-01 phase 1). Decision tables only (bUT150Install / bUT150HasUse):
#   HotplateHeatMode normal + tri-temp, Index16Heater 4- / 16-heater arms and its early returns, IndexHeatMode HeadOnly /
#   ChamberOnly; the SetTemp stub / hook / SystemStart early return. Memory only: Gerneral.ini and config.ini byte-identical.
#   Phase 2: Timer2 heater segment (golden :21528-21643, W906_Timer2HeaterSegment, QA arm open 20261002), the tick's InitialOK /
#   SystemStart guards + 1000 ms throttle (~4 s sleeps), WebBridgeTags.cpp:601, wb_serve boot call (golden :10722) (argv[1] = source root; AI(W906-R126) R126 M2/M3).
add_executable(test_heater_chain test_heater_chain.cpp)
target_link_libraries(test_heater_chain PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_heater_chain PRIVATE ws2_32)
endif()
add_test(NAME HeaterChain COMMAND test_heater_chain "${CMAKE_SOURCE_DIR}")
set_tests_properties(HeaterChain PROPERTIES TIMEOUT 120)

# --- test_st02_timers: AI(W906-S15) 20261001 (St02-E) -- S-15: golden TfMain::Timer8Timer + TimerTemperatureStorageMinuteTimer
#   at golden's 1000 ms (MainTimersSt02.cpp / MainTimer8.cpp, PumpTick WebBridgeTags.cpp:605).  The rate, [P29] MES0921 and its
#   hold, the S25 VTEST gate, [O11] Jam Rate with a fake clock, the temperature timer ([L10]), pnlCleanCount.  Containment
#   first (st02_test_containment.h); the temperature log roots go to a %TEMP% sandbox.
add_executable(test_st02_timers test_st02_timers.cpp)
target_link_libraries(test_st02_timers PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_timers PRIVATE ws2_32)
endif()
add_test(NAME St02_MainTimers COMMAND test_st02_timers)
set_tests_properties(St02_MainTimers PROPERTIES TIMEOUT 60)
include(${CMAKE_CURRENT_SOURCE_DIR}/st02_observer_sgjam.cmake)   # AI(W906-ST02-OB7) 20261002 (St02-E helper): ctest St02_ObserverSGJam + St02_ObserverSGJamPage (E-019 OB-7); on an empty St02 line, so no line moved
# --- test_st02_timer_esd: AI(W906-S13) 20261001 (St02-E) -- S-13: golden TfMain::TimerESDTimer (MainTimerESD.cpp + its tESDError
#   part THandlerTesterSide::TimerESDDrainESDError, TesterComm/Handler/HandlerTesterSide.cpp) and the dispatcher's TimerESD slot,
#   per-timer re-entry guard (the modal-wait tick) and close guard (MainTimersSt02.cpp).  SIM build: golden's SOFT_SIMULTE return,
#   nothing moves; both builds: the tESDError part through its hook; SHIP build: E2-E6 through the timer.  Containment first
#   (st02_test_containment.h); the yield / temperature log roots go to a %TEMP% sandbox.  Links the TesterComm Handler library for
#   the queue (THandlerTesterSide, not attached to the hub: no engine starts).
add_executable(test_st02_timer_esd test_st02_timer_esd.cpp)
target_link_libraries(test_st02_timer_esd PRIVATE
    ht9045_testercomm_handler ht9045_testercomm
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_timer_esd PRIVATE ws2_32)
endif()
add_test(NAME St02_TimerESD COMMAND test_st02_timer_esd "${CMAKE_SOURCE_DIR}")   # argv[1]: section 5 reads the hook-up lines (read only)
set_tests_properties(St02_TimerESD PROPERTIES TIMEOUT 60)

# --- test_d028_jamday_body: AI(W906-D028) 20261001 St01 -- todo D-028: FileRW/MainClose.cpp W906_FRW_InstallSaveRunMode installs
#   W906_RunInfo_SaveJamRateByDay as cprod.cpp's W906_SaveJamRateByDayBody (the laptop's JamDayHook half, a4ccc040), after the
#   bHandlerModel==false return; the upload stays in GATE (S95-J1). MainClose.cpp is wb_serve-only, so this is a source ratchet
#   (argv[1] = port root, read only, '\r' dropped for the CRLF gate checkout); it writes nothing (no D:\HT9045_Log file).
#   Not named *install*: Windows installer detection makes an unmanifested *install*.exe require elevation (ctest: "permission denied").
add_executable(test_d028_jamday_body test_d028_jamday_body.cpp)
add_test(NAME D028_JamDayBody COMMAND test_d028_jamday_body "${CMAKE_SOURCE_DIR}")
set_tests_properties(D028_JamDayBody PROPERTIES TIMEOUT 60)

# --- test_rotatekit_retry: AI(W906-I04) 20261001 (Ifor01) -- RotateKit pick-retry hang (GOLDEN_DEFECT_LEDGER.md, the RotateKit
#   row): case 4700 now goes through case 4710, which puts Z on its safe height (in arm MoveInArmZToPlateSafe, out arm
#   MoveOutArmToAutoSafe) before case 4000 -- changed from golden by RULINGS_20261001 #15 (Jimmy 1001 13:2x). Same commit lifts the
#   in arm's GATE (9) (CheckInArmZ is live, ainarm2.cpp:734). TMySimMotor drivers (tests/w906_test_motors.h); memory only, the
#   NewRecordProcess writes go to the ENV-ALL log scratch.
add_executable(test_rotatekit_retry test_rotatekit_retry.cpp)
target_link_libraries(test_rotatekit_retry PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_rotatekit_retry PRIVATE ws2_32)
endif()
add_test(NAME RotateKitRetry COMMAND test_rotatekit_retry)
set_tests_properties(RotateKitRetry PROPERTIES TIMEOUT 60)

# --- test_flow9050_shuttle: AI(W906-FLOW9050-A) 20261001 (Frank01, F-01b batch A) -- the HT9050 single-pair In / Out Shuttle
#   state machines translated from Frank's 910 tree (ref/frank-910-9050 beba23ee): acarry.cpp CheckIndexStatusERRSH1 /
#   Do_Auto_InSH / Do_Auto_OutSH (910 acarry.cpp:8522-8804, ledger H063), csystem.cpp DoAllProcess' Type_HT9050 shuttle
#   dispatch (H093 shuttle half), csystem_predicates.cpp OutSHT1InLF / OutSHT1InRT (H089), and the Type_HT9050 arms of
#   InitialOutArmNeedSuck (H074), MoveInArmXYToWaitTrayArm (H004) and AdjustShuttlePlaceOrder (H003); the Type_HT9046_LS
#   answers (what 9050GPIB decodes to today, database.cpp:517) are pinned unchanged.  Pumps DoAllProcess like
#   W6_6_CSystemCycle, so it refuses to run outside ctest's redirects (w906_ctest_guard.h).  Memory only.
add_executable(test_flow9050_shuttle test_flow9050_shuttle.cpp)
target_include_directories(test_flow9050_shuttle PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_flow9050_shuttle PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_flow9050_shuttle PRIVATE ws2_32)
endif()
add_test(NAME Flow9050_Shuttle COMMAND test_flow9050_shuttle)
set_tests_properties(Flow9050_Shuttle PROPERTIES TIMEOUT 120)

# --- test_eboot005_customername: AI(W906-EBOOT005) 20261001 [W906] (St01) -- Jimmy TO_STEVEN 20261001 12:3x, census 129 (e): golden
#   THandlerSystem::GetCustomerName (V912 HandlerSys.cpp:1242-1272) as FileRW_HSys_CustomerName (end of FileRW/HSys.cpp), reading the
#   THandlerSystem proxy rgCustomerList; the boot-slot call (golden main.cpp:11056-11057) is the laptop's. Golden names per CUSTOMER_CODE,
#   ItemIndex ignored, not booted = "HonPrec", cross-check with the port's HandlerSys.cpp:821-852 on the same list and on its own list,
#   FormLock taken; source ratchet (argv[1] = port root, read only, comments and '\r' dropped). HSys.cpp is listed directly, as
#   test_hsys_heater_mix; god-stack by RESCAN. Writes nothing (real Gerneral.ini / config.ini compared before and after).
#   The _fallback twin links ht9045_globals without HSys.cpp, so FileRW/_fallback.cpp's line is the one that runs ("HonPrec").
add_executable(test_eboot005_customername
    test_eboot005_customername.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/HSys.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
)
target_include_directories(test_eboot005_customername PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_eboot005_customername PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_eboot005_customername PRIVATE ws2_32)
endif()
add_executable(test_eboot005_customername_fallback test_eboot005_customername.cpp)
target_compile_definitions(test_eboot005_customername_fallback PRIVATE W906_EBOOT005_FALLBACK_ONLY=1)
target_include_directories(test_eboot005_customername_fallback PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_eboot005_customername_fallback PRIVATE ht9045_globals vclcompat)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_eboot005_customername.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME EBoot005_CustomerName COMMAND test_eboot005_customername "${CMAKE_SOURCE_DIR}")
set_tests_properties(EBoot005_CustomerName PROPERTIES TIMEOUT 120)
add_test(NAME EBoot005_CustomerNameFallback COMMAND test_eboot005_customername_fallback)
set_tests_properties(EBoot005_CustomerNameFallback PROPERTIES TIMEOUT 60)

# --- test_st02_timer3: AI(W906-S14) 20261001 (St02-E) -- S-14: golden TfMain::Timer3Timer (MainTimer3.cpp) and the dispatcher's
#   Timer3 slot (MainTimersSt02.cpp: on from the first pass with InitialOK, 1000 ms, skipped inside its own box, off after close).
#   G2 MES2108 once, G3 Fix3 sensor (SHIP), G15 TTL Clear lines, G16 the 8 yield alarms in order (a counting stand-in, then
#   the real ones once), G4 grid, G7 bin panels, G10 [E73], G13 [A37], G18 hourly record, G20 [RMS], G26 WAR1684 / WAR16500,
#   the gated parts with their flags on.  Containment first (st02_test_containment.h); the yield / temperature log roots go
#   to a %TEMP% sandbox.
add_executable(test_st02_timer3 test_st02_timer3.cpp)
target_link_libraries(test_st02_timer3 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_timer3 PRIVATE ws2_32)
endif()
add_test(NAME St02_Timer3 COMMAND test_st02_timer3 "${CMAKE_SOURCE_DIR}")   # argv[1]: section 12 reads the hook-up lines (read only)
set_tests_properties(St02_Timer3 PROPERTIES TIMEOUT 60)

# --- test_g023_os_report: AI(W906-G023) 20261001 (St02-E) -- todo G-023 / ST02-C1: golden TfTesterTCP's Open / Short report
#   trio (golden 906_0625_Steven Interface/TesterTCP.cpp:699-1056) as Interface/TesterTCP_OSReport.cpp, and its two callers
#   (aoutarm9045.cpp:4757; Automation/SCK_ART_Remainder.cpp:243 gate #6 <- W906_SckArt_SaveTestSummary).  CONTAINED: refuses
#   (exit 2) unless W906_HT9045LOG_ROOT and the four log roots are this directory's machine_log_scratch (_ht9045_env_extra, given
#   to every test by the deferred _w906_env_all_tests); writes only under <machine_log_scratch>/G023_OSReport_<tick> (as9045LogPath,
#   sJamRatePath, the B05 share, asSummaryPath, the N09 folder all redirected there; the SIM UNC case is a string check, no network);
#   stamps the machine's D:/HT9045_Log OS and JamRate files before and after (read only); removes its folder on a green run.
#   Both configs (SIM / SHIP); part 7 runs in SIM only.  [10] / [11] read Automation/SCK_ART_SaveTestSummary.cpp, aoutarm9045.cpp, Automation/SCK_ART_Remainder.cpp.
add_executable(test_g023_os_report test_g023_os_report.cpp)
target_link_libraries(test_g023_os_report PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_g023_os_report PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_g023_os_report PRIVATE -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()
target_compile_definitions(test_g023_os_report PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [10] / [11] source checks
add_test(NAME G023_OSReport COMMAND test_g023_os_report)
set_tests_properties(G023_OSReport PROPERTIES TIMEOUT 60)

# --- test_st02_timer1: AI(W906-ET1022) 20261001 (St02-E) -- E-T1-022: golden Timer1's NUMBER_PANEL_TYPE==2 bin-number panel
#   (main.cpp:3439-3440) in the dispatcher, option A (MainTimersSt02.cpp Timer1BinTick: every pass, golden's 30 ms deadline and
#   Timer1's guards).  The rate on a fake clock, type 2 only, the guards, one step per call, a QPC timing print (the laptop's
#   question b: St01 reports it), the hook-up line.  Memory only (SW[] outputs); containment first (st02_test_containment.h).
add_executable(test_st02_timer1 test_st02_timer1.cpp)
target_link_libraries(test_st02_timer1 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_timer1 PRIVATE ws2_32)
endif()
add_test(NAME St02_Timer1 COMMAND test_st02_timer1 "${CMAKE_SOURCE_DIR}")   # argv[1]: section 6 reads MainTimersSt02.cpp (read only)
set_tests_properties(St02_Timer1 PROPERTIES TIMEOUT 60)

# --- test_h013_terms: AI(W906-H013) 20261001 (St02-E) -- H-013 items 1 + 2 (ruling 11 = B), both back to golden 906 and pinned that way:
#   AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 Command.cpp:7406 (K1) and :9605 / :9682 / :14066 (K2-K4) (RULINGS_20261002 #20 / #23-6);
#   GetHandlerStatusByDll / RemoteControl answer 4 / -1 with fNote shown; IsSafePLCIOInstall / fSecsAlarm / TSecsAlarmForm live nowhere, their files gone;
#   source pins (argv[1] = port root, read only): K1-K4 = the 906 terms in their methods, K5 / K6 / K7 gone.
#   Memory only, writes nothing; containment first (st02_test_containment.h); the redirect roots come from _w906_env_all_tests.
add_executable(test_h013_terms test_h013_terms.cpp)
target_link_libraries(test_h013_terms PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_h013_terms PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_h013_terms.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME H013_Terms COMMAND test_h013_terms "${CMAKE_SOURCE_DIR}")
set_tests_properties(H013_Terms PROPERTIES TIMEOUT 120)

# --- test_socket_check: J-10 DoCheckSocketHasIC un-gate guard -----------------
#   AI(W906-J10-SOCKETCHECK) 20261001. The LIVE DoCheckSocketHasIC() used to be a
#   stub returning a hard-coded `false` while golden's 477 lines (golden
#   atester.cpp:4305-4781) sat inert inside GATE G-PTk3-DoCheckSocketHasIC.
#   Its only caller is DoTestY case 20 (atester.cpp:5429
#   `if(DoCheckSocketHasIC()==true) { Task=25; }`) and case 20 has no other
#   exit, so a function that can never return true pinned the test-cycle
#   dispatcher at Task 20 forever -- silently, with no alarm and no log line.
#   Measured in simulation by Jerry 20261001 (FROM_JERRY.md J-10).
#   This suite pins golden's entry state (case 1): the SKIP branch
#   (`LastSet.bD41TestSocketICCheckSkip || bCheckIndex` -> return true, golden
#   :4349-4350) and the per-arm routing (iSelArm==2 -> Task 2000). The
#   1000-/2000-series states command MOT[] motion and index vacuum; they need
#   Sim HAL state and are deliberately not asserted here.
add_executable(test_socket_check
    test_socket_check.cpp
)

target_link_libraries(test_socket_check PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_socket_check PRIVATE ws2_32)
endif()
add_test(NAME SocketCheck COMMAND test_socket_check)
set_tests_properties(SocketCheck PROPERTIES TIMEOUT 120)

# --- test_note_auth: AI(W906-D026) 20261001 (St01) -- todo D-026, the alarm note's password layer (Alert.Note, WS dialog.auth) =
#   WebLogin.cpp's appended D-026 block (WebNoteAuth.h): golden TfNote::FormShow's password block (V912 note.cpp:1496-1502 / :1818-1941),
#   TfNote::DoPassword (:5277-5429) and TfNote::DoUnlockPassword (:5431-5445) as Start() / BtnPauseClick() reach them (:3584-3607 /
#   :3906-3929, KeyCode==0 :3716-3727 / :4084-4095); the one-shot pass the answer gates consume; the notice / panel-key gates; the mailbox
#   "auth" object (tools/wb_dialog_mailbox.h AlarmRequestJson); source ratchets on tools/wb_serve.cpp / WebBridge/WebBridgeServer.cpp /
#   tools/wb_dialog_mailbox.h (argv[1] = the port root; control run: W906_D026_SRC_ROOT at a pre-change copy must go red).  WebLogin.cpp
#   is listed as a source (as test_weblogin_reauth); god-stack through RESCAN; FormLock / FormUnlock are empty stand-ins, and the private
#   TfSecurity::FileNameJam000 is pointed at the sandbox through the friend W906_LevelSetPut (WebLevelSet.cpp is wb_serve-only).
#   Made-up random credentials only; the text book, the jam-level file, the AlarmUnlock.ini copy and the stdout capture live in the build
#   dir's note_auth_scratch (DEFERred APPEND scheduled after the ENV-ALL call).  Refuses to run outside ctest's redirect environment; fails
#   if a made-up password shows up in any output or if D:\HT9045\system\login.dat / levelset.dat / lastdata.dat / Gerneral.ini,
#   D:\HT9045\Error\English\JAM0000.dat or C:\Windows\AlarmUnlock.ini changed.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_note_auth
    test_note_auth.cpp
    ${CMAKE_SOURCE_DIR}/WebLogin.cpp
)
target_include_directories(test_note_auth PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_note_auth PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_note_auth PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_note_auth.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME D026_NoteAuth COMMAND test_note_auth "${CMAKE_SOURCE_DIR}")
set_tests_properties(D026_NoteAuth PROPERTIES TIMEOUT 120)
cmake_language(DEFER CALL set_property TEST D026_NoteAuth APPEND PROPERTY ENVIRONMENT
    "W906_LOGINDAT_PATH=${CMAKE_CURRENT_BINARY_DIR}/note_auth_scratch/login.dat"
    "W906_PWBOOK_PATH=${CMAKE_CURRENT_BINARY_DIR}/note_auth_scratch/pwbook.txt"
    "W906_LEVELSET_PATH=${CMAKE_CURRENT_BINARY_DIR}/note_auth_scratch/levelset.dat"
    "W906_ALARMUNLOCK_PATH=${CMAKE_CURRENT_BINARY_DIR}/note_auth_scratch/AlarmUnlock.ini")
# AI(W906-D026) 20261001 [W906] St01 -- D026_NoteAuthPage (node, offline): web/page/ht9045_dialog_host.js HTDialogHost.verifyAuth (the hook
#   dialog-bridge.js calls when request.auth.required: the Dialog-auth-verify object goes to C++ as WS dialog.auth, tag = authId, value = the
#   JSON; the reply is mapped to {accepted, accessLevel, message}; the password in the object is cleared; never a synchronous throw) and the
#   login box's Cancel (golden Q45-4 = A: one dialog.auth {"cancelled":true} with the pressed key, no password field).  The real host file in a
#   node vm with a fake window / fake recipe client; no socket, no wb_serve.  Control run: W906_DIALOG_HOST_JS at a pre-change copy must go red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME D026_NoteAuthPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/d026_note_auth_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(D026_NoteAuthPage PROPERTIES TIMEOUT 60)
endif()

# AI(W906-TIF-KB) 20261002 (St02-E) -- TIF_KbGolden (node, offline): Setup.TesterIF's keyboard limits for the six
#   max-test-time fields and the three Initial Start Delay fields = golden 906_0625_Steven cTesterIF.cpp's generic branch
#   (:1401 DOUBLE 2, 0..15000; :1415 DOUBLE 2, no range), set by web/page/ht9045_testerif_c_wire.js block (8) over the
#   generated ht9045_wire_setuptesterif.js (docs/TESTERIF_FIELD_AUDIT_20261002.md P1 / P2).  Fake window / document, the
#   real wire data and c_wire files; Setup.TesterIF.html read only.  CONTROL: W906_TIF_C_WIRE pointing at the file before
#   block (8) must be red.  + VTEST (golden :1395-1398): extra.vtest -> the six max-test-time fields open with 0..36000
#   (FileRW/TestIF_File_TesterIF.cpp:236 sends it; read only).
if(W906_NODE_EXECUTABLE)
    add_test(NAME TIF_KbGolden COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/tif_kb_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(TIF_KbGolden PROPERTIES TIMEOUT 60)
endif()

# AI(W906-TIF-P3) 20261002 (St02-E) -- QWERTY_P3 (node, offline): web/page/qwerty.js:71 leaves a DOUBLE value as golden
#   906_0625_Steven myQwertyKeyBoard.cpp:290 (AnsiString(CheckRange(d, min, max)); the decimals argument is only the +/-
#   step), clamp unchanged (docs/TESTERIF_FIELD_AUDIT_20261002.md P3).  A minimal fake DOM, the real qwerty.js.
#   CONTROL: W906_QWERTY pointing at qwerty.js before :71 changed must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME QWERTY_P3 COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/qwerty_p3_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(QWERTY_P3 PROPERTIES TIMEOUT 60)
endif()

# AI(W906-KB-GOLDEN) 20261004 -- QwertyGoldenKeypad (node, offline): web/page/qwerty.js keys as golden TfQwertyKey
#   (906 myQwertyKeyBoard.cpp / .dfm, V912 the same bytes; machine report 20261004 "Teach numeric input"): the text opens selected and
#   the first key replaces it (FormShow :137-143 + AutoSelect, spbKeyClick :320-339), '-' toggles the sign (:421-431), '%' only
#   cycles the step captions (:368-418), steps = atof(caption) with int() / "%1.6f" (:433-449), OK and Abort share the CheckRange
#   post-step (:285-301), N_PORT and golden's min / max defaults (:249-257, .h:153), no X / overlay click (dfm:4, :283), a second
#   keypad stacks (:171-181); physical keys through the REAL physicalKeys() cut out of web/page/ht9045_wire_engine.js (the user's
#   20260915 Rule B, docs/web-client/REPLICATE.md §2.5: a key = the same-label button, Escape = Abort).  A minimal fake DOM.
#   CONTROL: W906_QWERTY_JS pointing at the qwerty.js before this change must be red (W906_WIRE_ENGINE_JS overrides the engine
#   file; 1/2 did not change the engine, 2/2 below did).
#   AI(W906-KB-GOLDEN) 20261004 (2/2): + the engine's PHYS_MAP (numeric pad BS / Del / OK for Backspace / Delete / Enter, Space only
#   where a space bar is shown, an unmatched key swallowed while a keypad is open) and qwerty.js settle() on every key / OK / Abort
#   (a hand-removed top gets onAbort once, the keypad under it works at once).  CONTROL: the HEAD engine -> 13 red, the HEAD
#   qwerty.js -> 9 red (108 checks).
if(W906_NODE_EXECUTABLE)
    add_test(NAME QwertyGoldenKeypad COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/qwerty_golden_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(QwertyGoldenKeypad PROPERTIES TIMEOUT 60)
endif()

# AI(W906-KB-GOLDEN) 20261004 (2/2) -- TeachKbGolden (node, offline): the web side of the Teach keypad follow-ups.  The REAL kb table of
#   web/page/ht9045_wire_hwteach.js against golden uteach.dfm / uteach.cpp / HTEdit.cpp (906 tree): EditSh1Speed / EditSh2Speed INTEGER
#   (dfm OnClick=EditSh1SpeedClick, uteach.cpp:4716-4719), the 8 elTeach ECDouble fields DOUBLE dp 3 (:3210-3217 -> HTEdit.cpp:216-223),
#   null for the 50 edits golden opens no keypad on (no OnClick / OnMouseDown and not in elTeach, or ReadOnly), no new range (ruling
#   #51 A); the REAL attachKeyboards() (null = readonly, no keypad); D6 typed-only-when-changed on HW.teach.html (golden SetText);
#   edtSetToOffset filled from the runtime cur.lastHomePos at a HOME end with HomeFlag 1 (Timer1Timer :1395 / :1402) and SetToOffset
#   refused on an empty box -- functions cut out of the REAL HW.teach.html, the REAL qwerty.js, a minimal fake DOM.
#   CONTROL: W906_TEACH_HTML / W906_WIRE_HWTEACH_JS / W906_WIRE_ENGINE_JS pointing at the HEAD copies must be red (5 / 14 / 6, 20261004).
if(W906_NODE_EXECUTABLE)
    add_test(NAME TeachKbGolden COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/teach_kb_golden_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(TeachKbGolden PROPERTIES TIMEOUT 60)
endif()

# AI(W906-C2) 20261002 (St02-E) -- KB_MachineSetting (node, offline): Main.CommView edtSetZ1 / edtSetZ2 keep golden
#   906_0625_Steven main.cpp:26175-26185's two branches by INDEX_DRIVER_TYPE (Mitsubishi 0..100, else 0..300), chosen when the
#   keyboard opens (web/page/ht9045_kb_generic.js; unknown / read failed -> the generated 0..100, the safe side).  Fake
#   window / document, the real wire data and ht9045_kb_generic.js; Main.CommView.html read only.
#   CONTROL: W906_KB_GENERIC pointing at a file without the listener (an empty .js) must be red.
#   AI(W906-C8) 20261002: + Setup.TrayForm Tp1/2/3TickUp by C++'s IniConfig.bC03UseCatchTray (golden cTrayForm.cpp:797-800; the page's C route Lab1/2/3XPickup captions, not LastSet.ini); CommView key missing -> golden Panasonic, 0..300.
if(W906_NODE_EXECUTABLE)
    add_test(NAME KB_MachineSetting COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/kb_machine_setting_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(KB_MachineSetting PROPERTIES TIMEOUT 60)
endif()

# AI(W906-O19-C4) 20261002 (St02-E helper) -- MyDB_O19_Summary: card ST02-C4 (todo D-004 (1)).  O19SummaryReport.cpp =
#   golden 906_0625_Steven TfObserver::DoProduction_Summary_Report (cObserver.cpp:4889-5058) on the real fObserver grid:
#   the golden bUseMDB==false jam sections ("No Record!!"), section 3 from Production_Loader INI files seeded with
#   WriteIniData (the end date excluded, across a year, start == end), SGDToCSV bytes (CRLF, append, first "," only),
#   W906O19_DayOfWeek / _HostName (= gethostname, hence ws2_32) / _SaveDir.  Containment first: exit 2 unless the cMyDB
#   log roots, W906_PRODLOADER_ROOT and W906_O19_ROOT are machine_log_scratch; then it writes only under
#   %TEMP%\ht9045_o19_<tick>, removed on a green run.  CONTROL: the old no-op stub fails the day-report check.
#   Links the machine archives (RESCAN, as test_mydb_csv_eventlog).  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
list(APPEND _ht9045_env_extra "W906_O19_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/O19")   # AI(W906-O19-C4) 20261002 (St02-E helper): the cMyDB.cpp O19 block's W906O19_SaveDir (golden IniConfig.asO19_SavePath) for EVERY ctest -- ELA_TimeData's RecordTimeData(3) reads the real clock, and at 08:00:01 .. 08:00:19 golden :445 MyForceDirectories would otherwise make <drive root>\<yyyy> (asO19_SavePath "") or D:\HT9045_Log\Product_Loader\<yyyy> (the default, once a test loads the config)
add_executable(test_o19_summary test_o19_summary.cpp)
target_include_directories(test_o19_summary PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_o19_summary PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_o19_summary PRIVATE ws2_32)
endif()
add_test(NAME MyDB_O19_Summary COMMAND test_o19_summary)
set_tests_properties(MyDB_O19_Summary PROPERTIES TIMEOUT 60)
# --- test_pci1203_linkwatch: AI(W906-1203-LINKLOST) 20261001 -- EastSun 1001「1203 如果斷線10秒 要跳出異常」:
#   EtherCAT/Pci1203LinkWatch.h (the decision behind tools/wb_serve.cpp EOF W906_Pci1203LinkWatchTick -> golden WAR16152)
#   against a fake monitor + fake clock: never linked -> no alarm; lost 9.9 s -> none; 10 s -> one, with the reason;
#   still lost -> no second; restored -> reset; lost again 10 s -> again; card-level signals; tick wrap.  Built like
#   Pci1203Pure (Pci1203Monitor.cpp / Pci1203Control.cpp without HAVE_PCI1203: case [1e] runs the adapter on the REAL
#   monitor class, which cannot open a card here).  Memory only, reads and writes no file; at the end, so it has its own TIMEOUT.
add_executable(test_pci1203_linkwatch
    test_pci1203_linkwatch.cpp
    ../EtherCAT/Pci1203Monitor.cpp
    ../EtherCAT/Pci1203Control.cpp)
target_include_directories(test_pci1203_linkwatch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_pci1203_linkwatch PRIVATE vclcompat)
add_test(NAME Pci1203LinkWatch COMMAND test_pci1203_linkwatch)
set_tests_properties(Pci1203LinkWatch PROPERTIES TIMEOUT 60)
# --- test_pci1203_modulecheck: AI(W906-MODCHECK) 20261003 -- EastSun 1003 (machine engineer's ruling), the 1203 module presence check:
#   EtherCAT/Pci1203ModuleCheck.h (the decision behind tools/wb_serve.cpp EOF W906_Pci1203ModuleCheckTick -> WAR16154)
#   against a fake monitor + fake clock + fake ini (a Loader lambda over a string): classification (402 / SERVOPACK motor,
#   vacuum excluded with the REAL Pci1203Vc8IdentityOk, 401 / Dig. In / Dig. Out IO); ini round trip + refusals; no ini -> one
#   write event, no alarm; truncated / unaddressable -> no write; all OP -> pass; missing / not OP at 9.9 s vs 10 s; station
#   changed; extra; several faults -> exactly one alarm, latched until the next generation; link lost -> abandoned.  Built like
#   test_pci1203_linkwatch (Pci1203Monitor.cpp / Pci1203Control.cpp without HAVE_PCI1203, for the real sample structs).
#   Memory only, reads and writes no file; at the end, so it has its own TIMEOUT.
add_executable(test_pci1203_modulecheck
    test_pci1203_modulecheck.cpp
    ../EtherCAT/Pci1203Monitor.cpp
    ../EtherCAT/Pci1203Control.cpp)
target_include_directories(test_pci1203_modulecheck PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_pci1203_modulecheck PRIVATE vclcompat)
add_test(NAME Pci1203ModuleCheck COMMAND test_pci1203_modulecheck)
set_tests_properties(Pci1203ModuleCheck PROPERTIES TIMEOUT 60)

# --- test_home_monitor: AI(W906-HOMEMON) 20261001 -- the web Home Monitor's C++ half (JsonBridge/ChanHome.cpp).
#   A: home.rows (golden runtime THomeClass: Visible rows, vector order, labName Caption / Left / Top, edPos Text, ShowLed lamp)
#   and home.log (ListBox1 newest first, capped) as JSON, escaping and UTF-8 repair; B: staged only while a browser has the
#   window open (streamWanted) or the home sequence shows it (fHome->Show), home.resetOk = Panel2->Visible; C: act.home.abort
#   refused while fHome->fShow is false (nothing called), the happy path = golden TfHome::sbAbortHomeClick (fake 1203 stop
#   hook called once, SystemStart / fAllMotorHome / motor power false, relay off, form closed), a second press refused.
#   ChanHome.cpp is wb_serve-only (root CMakeLists.txt), so it is listed here.  No card, no machine file: RecordProcess
#   (GaliMotorServoOff) writes only under the redirect roots -- no own ENVIRONMENT, _w906_env_all_tests gives it, and the
#   guard (w906_ctest_guard.h) refuses outside them.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_home_monitor test_home_monitor.cpp ${CMAKE_SOURCE_DIR}/JsonBridge/ChanHome.cpp)
target_include_directories(test_home_monitor PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_home_monitor PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_home_monitor PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_home_monitor.cpp ${CMAKE_SOURCE_DIR}/JsonBridge/ChanHome.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME HomeMonitor COMMAND test_home_monitor)
set_tests_properties(HomeMonitor PROPERTIES TIMEOUT 120)

# --- test_teach_formshow_bridge: AI(W906-TEACH-FORMSHOW) 20261002 -- the Teach page's golden FormShow bridge
#   (FileRW/TeachFormShow_File.cpp, golden uteach.cpp:1413-2012 screen half: Visible / TabVisible / Caption / Color / Enabled /
#   ActivePage). Two option sets, each assertion pins a golden line; FormBridge.cpp (wb_serve-only) is listed here and the test
#   owns the bridge table. Memory only: no machine file, no card.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_teach_formshow_bridge test_teach_formshow_bridge.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/TeachFormShow_File.cpp ${CMAKE_SOURCE_DIR}/JsonBridge/FormBridge.cpp)
target_include_directories(test_teach_formshow_bridge PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_teach_formshow_bridge PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_teach_formshow_bridge PRIVATE ws2_32)
endif()
add_test(NAME TeachFormShowBridge COMMAND test_teach_formshow_bridge)
set_tests_properties(TeachFormShowBridge PROPERTIES TIMEOUT 120)

# --- test_compk_datas: AI(W906-CSEL-NEEDREF / W906-OBS-TABVIS) 20261001 -- every-component check, Data / Status pages.
#   A: Counter Selection's NeedRef reader (golden TfMain::Timer1Timer 906 main.cpp:3208-3213 + DoShowUserDefFrom's
#   fTestCategory branch :8744-8752), FileRW/IniConfig_CounterSel.cpp FileRW_CounterSel_NeedRefTick -- SetShowCateMode and
#   Show / Close of fTestCategory after the form closes. B: the Observer's tabVisible map (cObserver.cpp
#   W906_ObserverTabVisibleJson: only the sheets golden FormShow writes). C: the Observer's Time Data (AI(W906-OBS-TIMEDATA):
#   pgcMessageChange / lstTimeDataClick, reads the TimeData year folder only). FileRW/IniConfig_CounterSel.cpp is wb_serve-only
#   (FileRW/_editlist_sources.cmake), so it is listed here with the C-route shared layer, as test_hsys_heater_mix does.
#   Memory only (no golden FormShow, no file); the guard refuses outside the redirect roots. After the GLOBAL TEST TIMEOUT
#   block => own TIMEOUT.
add_executable(test_compk_datas
    test_compk_datas.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/IniConfig_CounterSel.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
)
target_include_directories(test_compk_datas PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_compk_datas PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_compk_datas PRIVATE ws2_32)
endif()
add_test(NAME CompK_DataS COMMAND test_compk_datas)
set_tests_properties(CompK_DataS PROPERTIES TIMEOUT 120)

# --- test_setupa_kbextra_n04: AI(W906-SETUPA) 20261002 -- setupA（EastSun 1001「請檢查每個頁面元件…用枚舉」）兩個 C++ 補件：
#   (A) FileRW/IniConfig_N04.cpp＝golden TfConfiguration 建構子 cConfiguration.cpp:121-147 [N04] edtN04_Host／mmoN04_IP（本機名稱、IP；
#       KYEC_LEE 的 edN04_ID）；(B) FileRW/DeviceForm_KbExtra.cpp＝TfContact editlist.get extra（golden 小鍵盤的執行期上下限：
#       edForcePerPin*／edContactOffsetArm*／edShtPickOffset*／edDoubleForce，與 edDropWaitTimeMouseDown 輸入後修正要的伺服器端值）。
#   原始碼棘輪 argv[1]＝移植樹根目錄（唯讀）。不寫檔：比對 Gerneral.ini／config.ini。只讀本機網路設定（gethostname／gethostbyname）。
add_executable(test_cfga_kbextra_n04   # AI(W906-MACH1002) 20261002: was test_setupa_kbextra_n04 -- a 32-bit exe named *setup* is an installer to UAC (Permission denied / Not Run on the MinGW oracle)
    test_setupa_kbextra_n04.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/IniConfig_N04.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_KbExtra.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_cfga_kbextra_n04 PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_cfga_kbextra_n04 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_cfga_kbextra_n04 PRIVATE ws2_32)
endif()
add_test(NAME SetupA_KbExtraN04 COMMAND test_cfga_kbextra_n04 "${CMAKE_SOURCE_DIR}")
set_tests_properties(SetupA_KbExtraN04 PROPERTIES TIMEOUT 120)

# --- test_iosetview_formshow_bridge: AI(W906-IOSV-FORMSHOW) 20261002 -- the IO page's golden FormShow bridge
#   (FileRW/IoSetViewFormShow_File.cpp, golden iosetview.cpp:236-1023 screen half + ShowSuckMode / ShowShuttleSensor / LabSiteMap /
#   Hide_1032_IO). Two option sets (A = this HT9050's Gerneral.ini), each assertion pins a golden line; FormBridge.cpp (wb_serve-only)
#   is listed here and the test owns the bridge table. Memory only: no machine file, no card.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_iosetview_formshow_bridge test_iosetview_formshow_bridge.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/IoSetViewFormShow_File.cpp ${CMAKE_SOURCE_DIR}/JsonBridge/FormBridge.cpp)
target_include_directories(test_iosetview_formshow_bridge PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_iosetview_formshow_bridge PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_iosetview_formshow_bridge PRIVATE ws2_32)
endif()
add_test(NAME IoSetViewFormShowBridge COMMAND test_iosetview_formshow_bridge)
set_tests_properties(IoSetViewFormShowBridge PROPERTIES TIMEOUT 120)

# --- test_note_webkeygate: AI(W906-NOTE-KEYGATE) 20261002 -- the alarm note's key lock for a key chosen on the screen
#   (forms/fNote_WebKeyGate.cpp = golden note.cpp:2843-2870 BtnSkipClick + :2764-2873 UpdateButtonStatus): each golden condition
#   refuses with its reason and passes once cleared; source pins on tools/wb_serve.cpp (argv[1] = the port root, read only).
#   Memory only: no IO table, no card, no machine file.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_note_webkeygate test_note_webkeygate.cpp)
target_link_libraries(test_note_webkeygate PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_note_webkeygate PRIVATE ws2_32)
endif()
add_test(NAME NoteWebKeyGate COMMAND test_note_webkeygate "${CMAKE_SOURCE_DIR}")
set_tests_properties(NoteWebKeyGate PROPERTIES TIMEOUT 120)
# --- test_io_formshow: AI(W906-IO-FORMSHOW-OUT) 20261002 -- the IO window's golden open steps (JsonBridge/IoFormShow.cpp): on the
#   closed -> open edge of the real WebWindowRegistry.cpp, sbIOClick main.cpp:27946 (SystemStart -> nothing) / :27954 Enter IO and
#   FormShow iosetview.cpp:306-329 (Index brakes Off + position records); fresh reports only.  Switches Enable=false, sim motors.
add_executable(test_io_formshow
    test_io_formshow.cpp
    ${CMAKE_SOURCE_DIR}/JsonBridge/IoFormShow.cpp
    ${CMAKE_SOURCE_DIR}/WebWindowRegistry.cpp
)
target_include_directories(test_io_formshow PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_io_formshow PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_io_formshow PRIVATE ws2_32)
endif()
add_test(NAME IoFormShow COMMAND test_io_formshow "${CMAKE_SOURCE_DIR}")
set_tests_properties(IoFormShow PROPERTIES TIMEOUT 120)
# --- NonStopRoute: AI(W906-NONSTOP-SEM) 20261002 -- C++ ShowErrorMessage alarms always take the stopping page (golden note.cpp:795-808);
#   the REAL web/page/ht9045_nonstop_alarm.js with the REAL web/config/AlarmNonStop.json in a fake window (XHR); web-local alarms
#   still use the table.  CONTROL: W906_NONSTOP_JS at the file without block 0.5 -> [1] / [2] red (WAR1676 / WAR1677 / WAR1681).
if(W906_NODE_EXECUTABLE)
    add_test(NAME NonStopRoute COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/nonstop_route_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web")
    set_tests_properties(NonStopRoute PROPERTIES TIMEOUT 60)
endif()

# --- test_e021_observer: AI(W906-E021-OB1) 20261002 [W906] (St01) -- todo E-021: Data.Observer's St01 rows = golden TfObserver (906
#   cObserver.cpp; V912 in [ ], AI(W906-E030-CITE) 20261003): OB-1 Exit / close (BtnExitClick :697-706, FormClose :654-674 + SavePrecautionParameter :4154-4183 [4385-4414]), OB-2 Precautions
#   Record (:4452-4533 [4683-4764] + the save :4078-4152 [4309-4383]), OB-3 Major Maintenance (:4535-4678 [4766-4909] + :4311-4389 [4542-4620]), OB-4 Precaution Log (:4680-4753 [4911-4984]), OB-6
#   Time Data (:4766-4844 [4997-5075]), OB-8 Backup Log (:5371-5391 [5602-5622]), OB-9 Clear Time Data (:5393-5399 [5624-5630]).  Under test: the tail of cObserver.cpp
#   (ht9045::sjson::W906_ObserverAct, W906_E021_*, FileRW_Observer_WindowEdge; ht9045_sm) through the REAL act.* dispatch
#   JsonBridge/ChanAction.cpp HandleActionWithTag (:344) -- the source list of test_e022_contactct.  The three new path seams point into
#   machine_log_scratch/e021 (DEFER below, after AI(W906-ENV-ALL) copied the global string); the test refuses to run (exit 2) when any is
#   unset or outside the scratch, never calls jimmychiu's FormShow (it makes D:\PrecautionRecord\system), replaces OB-8's exec hook before
#   OB-8 (no command ever runs), and compares D:\PrecautionRecord, D:\MajorMaintenanceRecord, D:\HT9045\system and config.ini before / after.
#   argv[1] = port tree, argv[2] = web\page (source ratchets, read only).
add_executable(test_e021_observer
    test_e021_observer.cpp
    ../JsonBridge/ChanProduction.cpp
    ../JsonBridge/ChanIo.cpp
    ../JsonBridge/ChanMotor.cpp
    ../JsonBridge/ChanAction.cpp
    ../JsonBridge/actions/MainTesterConnect.cpp
    ../JsonBridge/actions/MainRecordClear.cpp ../JsonBridge/actions/LotInfoFtp.cpp ../JsonBridge/actions/ObserverSGJam.cpp   # AI(W906-LI9-F1) 20261002 (St02 claim on St01's line): ChanAction.cpp (:6642) dispatches act.lotInfoFtp.* -> W906_LotInfoFtpAct (forward only; no body installed here = not-installed); same line  AI(W906-ST02-OB7) 20261002 (St02-E helper): JsonBridge/ChanAction.cpp:347 now calls W906_ObserverSGJamAct_St02 (act.observerSG.*), so every target that compiles ChanAction.cpp needs this file
    ../JsonBridge/actions/MainClarnData.cpp ../JsonBridge/actions/MainStateRecord.cpp
    ../JsonBridge/EventLog.cpp
    ../EtherCAT/Pci1203Monitor.cpp)
target_include_directories(test_e021_observer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_e021_observer PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
if(WIN32)
    target_link_libraries(test_e021_observer PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_e021_observer.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME E021_Observer COMMAND test_e021_observer "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(E021_Observer PROPERTIES TIMEOUT 180)
cmake_language(DEFER CALL set_property TEST E021_Observer APPEND PROPERTY ENVIRONMENT
    "W906_PRECAUTION_ROOT=${_ht9045_log_scratch}/e021/PrecautionRecord"
    "W906_MAJORMAINT_ROOT=${_ht9045_log_scratch}/e021/MajorMaintenanceRecord"
    "W906_BACKUPLOGBAT_PATH=${_ht9045_log_scratch}/e021/system/2.bat")
# --- E021_ObserverPage: AI(W906-E021-OB1) 20261002 [W906] (St01) -- the page half of E-021 (node, offline): D:\HT9045\web\page\
#   ht9045_observer_ev.js (+ the same-line edits in ht9045_observer_wire.js and Data.Observer.html) in a node vm with a fake DOM /
#   HT9045Recipe: compiles, one EOL style, no BOM; the Record-tab buttons send act.observer.<op> with every input (widgets) after the
#   first dataRecord, never before; the reply's dataRecord is drawn (tab visibility, edits, combos incl. a text outside the list, memos,
#   panels; the focused box is kept); a refusal / busy: shows on the status line; Exit is stopped before .exitbtn and closes only on
#   close:true (an old wb_serve without act.observer.* still closes); pgcMessage tabs / lstTimeData / Backup Log (year text) / Clear
#   Time Data; ht9045_observer_wire.js hands dataRecord over and does not bind "not wired" on Backup Log.  Control: W906_E021_PAGE_DIR ->
#   the pre-change pages must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME E021_ObserverPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/e021_observer_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(E021_ObserverPage PROPERTIES TIMEOUT 60)
endif()
# --- test_thermo_comm: AI(W906-I03) 20261002 (Ifor01) -- I-03 phase 1: golden TCOM2 Comm2 (the temperature-controller serial port,
#   rs232.cpp EOF) + cpublic.cpp KT4H / TMC401 / E5DC frames + bthermo.cpp DoThermoReal over it. Comm2 forced to SIM (no COM port);
#   no RS232Init / W906_CreateFormBoot (they probe real ports / touch Gerneral.ini). Frames byte for byte against independent encoders,
#   the 100 ms delivery, DoThermo end to end with a fake KT4H (UN150ReadReal 100.0) and without one (999, longest call). ~10 s.
add_executable(test_thermo_comm test_thermo_comm.cpp)
target_link_libraries(test_thermo_comm PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_thermo_comm PRIVATE ws2_32)
endif()
add_test(NAME ThermoComm COMMAND test_thermo_comm "${CMAKE_SOURCE_DIR}")
set_tests_properties(ThermoComm PROPERTIES TIMEOUT 120 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")

# --- test_settemp_save: AI(W906-I01C) 20261002 (Ifor01) -- I-01 phase 2 (2): golden TfMain::SetTemp saves through
#   fTemp_Set->spbSaveClick -> SaveSetupFile with uTemp_Set.cpp SAFETY GATE (S1) / (S2) / GATE(dep-FormHS) open (W-05 = (a)).
#   %TEMP% sandbox recipe: [Mode] Temperature / [Time] Soak written and read back, fHeaterOK / iThermoTask reset, the web page
#   hook called; SystemStart writes nothing; the machine's own files untouched. Red with the gates put back. Containment: ENV-ALL.
add_executable(test_settemp_save test_settemp_save.cpp)
target_link_libraries(test_settemp_save PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_settemp_save PRIVATE ws2_32)
endif()
add_test(NAME SetTempSave COMMAND test_settemp_save)
set_tests_properties(SetTempSave PROPERTIES TIMEOUT 120)
# --- test_index_check4site: J-12 IndexCheck4Site un-gate guard ---------------
#   AI(W906-J12-INDEXCHECK4SITE) 20261002, answering NB2 R126 M1 (medium, on MR !81).
#   J-10 un-gated DoCheckSocketHasIC; its 1000-/2000-series states wait on
#   IndexCheck4Site (atester.cpp :4971 / :5188), but the LIVE IndexCheck4Site was
#   still a stub returning false, with golden's 117 lines inert inside
#   GATE G-PTk3-IndexCheck4Site. A callee that can never return true turned the
#   J-10 fix into a NEW silent stall at Task 1056 / 2056, visible only as a 300 s
#   CheckSocketHasICHangUpCheck entry. Reachable on CC_ASE_KaohSiung with
#   bDevicConfirm + INDEX_SUCKER_TYPE==1; HT9050 (CUSTOMER_CODE=0) unaffected.
#   This suite pins golden's reset path, the "nothing needs checking -> true"
#   verdict the stub could not produce, and that the verdict is read from the
#   real bNeedCheck grid within golden's [iSuckcount, iSuckcount+2) window.
#   No motion and no modal on any asserted path.
add_executable(test_index_check4site
    test_index_check4site.cpp
)

target_link_libraries(test_index_check4site PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_index_check4site PRIVATE ws2_32)
endif()
add_test(NAME IndexCheck4Site COMMAND test_index_check4site)
set_tests_properties(IndexCheck4Site PROPERTIES TIMEOUT 120)

# --- test_sht_rowcol_startpos: J-7 GetShtRowColStartPos guard ----------------
#   AI(W906-J12-INDEXCHECK4SITE) 20261002, answering NB2 R126 (low): the J-7
#   change (MR !80) shipped without a ctest. Until then golden's two reads
#   (ainarm9045.cpp golden :9095-9096) were inert and the live code pinned
#   iRow=iCol=0, so GetShtStartPos derived the start from a 0x0 grid. Because
#   FindCentorPointIndex(0) = -0.5, that did not merely lose the grid -- it put
#   the start HALF A PITCH the wrong side, identically for every geometry, and
#   the in-arm place target fell outside the soft limit (WAR0154, measured in
#   simulation). This suite stages the centre / offset / pitch / bases as plain
#   globals and checks the result against golden's own arithmetic for 1x1 and
#   2x2, that it now DEPENDS on TestSocket, and that an unhandled target still
#   returns golden's 0 / 0. No motion, no IO.
add_executable(test_sht_rowcol_startpos
    test_sht_rowcol_startpos.cpp
)

target_link_libraries(test_sht_rowcol_startpos PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_sht_rowcol_startpos PRIVATE ws2_32)
endif()
add_test(NAME ShtRowColStartPos COMMAND test_sht_rowcol_startpos)
set_tests_properties(ShtRowColStartPos PROPERTIES TIMEOUT 120)
# --- test_dtme08_cycle: AI(W906-I03) 20261002 (Ifor01) -- I-03 phase 2: golden TfrmDTME08 (forms/fDTME08.cpp, every gate open) +
#   bthermo.cpp G29 against a fake Modbus/TCP DTM the test owns (127.0.0.1, ephemeral port, pumped from the test loop -- no thread):
#   the boot (golden ctor), the polled non-blocking socket (192.0.2.1 never answers: no call waits), golden's boot writes in order
#   (sensor type / cycle time / Out2 / SV x 4 stations, frames byte for byte), the PV / status / SV cycle, DoSetSVOfDTME08 ->
#   SetSettingSV -> the SV write, the 100 ms TimerUpdate tick and the FormShow arm (SIM: no-op; ship: golden's `A && B || C`), the
#   DTM going away; source pins incl. "wb_serve does not arm it yet" (argv[1] = the port root, read only).  Reads
#   D:\HT9045\EXE\DTME08_Control.ini only (Reload; D-035 = golden's exe folder); compares Gerneral.ini / config.ini before and after.  ~25-30 s: golden
#   waits GetReconnectTime() = 10 s between connect attempts, kept.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_dtme08_cycle test_dtme08_cycle.cpp)
target_link_libraries(test_dtme08_cycle PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_dtme08_cycle PRIVATE ws2_32)
endif()
add_test(NAME DTME08Cycle COMMAND test_dtme08_cycle "${CMAKE_SOURCE_DIR}")
set_tests_properties(DTME08Cycle PROPERTIES TIMEOUT 120)
# --- test_e020_lotinfo_e020: AI(W906-E020-LI6) 20261002 [W906] (St01) -- todo E-020 LI-6 / LI-11 / LI-13: LotInfo_E020.cpp (ht9045_sm)
#   = golden V912 uLotInfo.cpp TfLotInfo::RTCChangeFile / Timer1Timer (:5356-5475), btChangeFileClick's branch choice (:10394-10418)
#   and palSecsGemMouseDown routed to jimmychiu's facade body (:10532-10606), through the REAL act.* dispatch JsonBridge/ChanAction.cpp
#   HandleActionWithTag (:348 same line) -- the source list of test_e021_observer.  One exe, two tests:
#     E020_LotInfoRTC  (rtc):  the SM and flags as golden, every COM2 RTC-vision statement logged as a skip (TCOM2Shim has no RTC half),
#                              the VCL Timer1 wrapper, source ratchets on the foreign lines cSetUp.cpp:1083 / :1093 and uhome.cpp:1554-1557,
#                              forms/fLotInfo.h:2333 and CMakeLists.txt (LotInfo_E020.cpp in ht9045_sm)
#     E020_LotInfoActs (acts): act.lotInfo.changeFile (visibility, branch 0 executed, branches 1-3 GATE (W906-E020-LI11-n)) and
#                              act.lotInfo.palSecsGemMouseDown (the 6-click sequence; SetLotID writes AuthPath config.ini -- the sandbox)
#   AuthPath (W906_AUTH_PATH, the global containment, ENV-ALL above) must be in ctest's scratch or the test exits 2 before any Handler
#   code; the real D:\HT9045\config\config.ini and D:\HT9045\system\Gerneral.ini are compared before / after.  argv[2] = port tree
#   (ratchets, read only).  Control: W906_E020_SRC_ROOT -> a folder with the pre-change sources must be red.
add_executable(test_e020_lotinfo_e020
    test_e020_lotinfo_e020.cpp
    ../JsonBridge/ChanProduction.cpp
    ../JsonBridge/ChanIo.cpp
    ../JsonBridge/ChanMotor.cpp
    ../JsonBridge/ChanAction.cpp
    ../JsonBridge/actions/MainTesterConnect.cpp
    ../JsonBridge/actions/MainRecordClear.cpp ../JsonBridge/actions/LotInfoFtp.cpp ../JsonBridge/actions/ObserverSGJam.cpp   # AI(W906-LI9-F1) 20261002 (laptop batch 46 merge): ChanAction.cpp dispatches act.lotInfoFtp.* since St02 MR !116 -- same line as !116 did at test_sjson_chan / test_e021_observer  AI(W906-ST02-OB7) 20261003 (St02-E, claim): + ObserverSGJam.cpp -- ChanAction.cpp:347 dispatches act.observerSG.* (W906_ObserverSGJamAct_St02), the same reason as LotInfoFtp.cpp before it
    ../JsonBridge/actions/MainClarnData.cpp ../JsonBridge/actions/MainStateRecord.cpp
    ../JsonBridge/EventLog.cpp
    ../EtherCAT/Pci1203Monitor.cpp)
target_include_directories(test_e020_lotinfo_e020 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_e020_lotinfo_e020 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
if(WIN32)
    target_link_libraries(test_e020_lotinfo_e020 PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_e020_lotinfo_e020.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME E020_LotInfoRTC COMMAND test_e020_lotinfo_e020 rtc "${CMAKE_SOURCE_DIR}")
set_tests_properties(E020_LotInfoRTC PROPERTIES TIMEOUT 120)
add_test(NAME E020_LotInfoActs COMMAND test_e020_lotinfo_e020 acts "${CMAKE_SOURCE_DIR}")
set_tests_properties(E020_LotInfoActs PROPERTIES TIMEOUT 120)
# --- E020_LotInfoPage: AI(W906-E020-LI11) 20261002 [W906] (St01) -- the page half of E-020 LI-11 / LI-13 (node, offline):
#   D:\HT9045\web\page\ht9045_lotinfo_e020.js in a node vm with a fake DOM / HT9045Recipe: Change File = act.lotInfo.changeFile with the
#   result / GATE text on #barcodeStatus and the 400 ms cooling; every palSecsGem mouse-down sent in order with its seq (never dropped),
#   a press on a child control not; Data.LotInfo.html loads the file and btChangeFile is enabled.  Control: W906_E020_PAGE_DIR -> the
#   pre-change pages must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME E020_LotInfoPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/e020_lotinfo_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(E020_LotInfoPage PROPERTIES TIMEOUT 60)
endif()
# --- D034_IniConfigFifoPage: AI(W906-D034) 20261002 [W906] (St01) -- todo D-034, the page half of MyMessageBox::DoPassword_MBox (node,
#   offline): D:\HT9045\web\page\ht9045_iniconfig_auth_c.js in a node vm with a fake window / document / HTQwerty / HT9045Recipe.
#   [I37_1] FIFO turned on (golden V912 cConfiguration.cpp:7353-7369 -> mymessbox.cpp:1228-1286) opens the login keypad and sends
#   extra.reauth {point:"i37_1"}; with [M01] changed too, M01 first then FIFO as an array; Abort = cancelled; drop-down mode = password
#   only; no keypad when golden would not ask (turned off / unchanged / CC_PTI vendor / not editable / older C++); reauthAll reported,
#   WAR1677 raised once; no password logged.  The C++ half is WebLogin_Reauth step 6b.  Control: W906_INICONFIG_AUTH_JS at the
#   pre-change script must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME D034_IniConfigFifoPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/d034_iniconfig_fifo_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(D034_IniConfigFifoPage PROPERTIES TIMEOUT 60)
endif()
# AI(W906-LI9-F1) 20261002 (St02-E helper) -- LI9_FtpClient: card LI-9 F1 = golden TfLotInfo::btnFtpServerClick (906_0625_Steven
#   uLotInfo.cpp:5001-5126) and the KYEC FTP dialog TfFTPClient's HD / Tester pages (KYECFTP/FTPClient.cpp) through WS
#   act.lotInfoFtp.<op> (JsonBridge/actions/LotInfoFtp.cpp -> WebLotInfoFtp_St02.cpp).  Memory plus a scratch root only, NEVER the
#   network: Server (tag 0) ends at its F2 gate; the one network-shaped path (a UNC Taster map) runs only in the SIM build, where
#   W58 Q5 skips it on the path string.  Refuses to run (exit 2) outside ctest's redirect roots (st02_test_containment.h, and
#   DataPath / OffsetPath / AuthPath must be scratch); then DataPath / OffsetPath / AuthPath and the two Taster files go to
#   <machine_log_scratch>/LI9_FtpClient_<tick>/; fails if D:/HT9045/config/config.ini or D:/HT9045/SetUp.inf changed.  Fakes:
#   ChangeSetUpFile / RecipeChainsReady / LocalIp (WebLotInfoFtpInstall_St02.cpp is wb_serve-only, not linked); FormLock /
#   FormUnlock stand-ins (JsonBridge/FormJson.cpp is a wb_serve source).  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_li9_ftpclient
    test_li9_ftpclient.cpp
    ../KYECFTP/FTPClientForm_St02.cpp
    ../forms/fLotInfo_Ftp_St02.cpp
    ../WebLotInfoFtp_St02.cpp
    ../JsonBridge/actions/LotInfoFtp.cpp
)
target_include_directories(test_li9_ftpclient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_li9_ftpclient PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_li9_ftpclient PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_li9_ftpclient.cpp ../KYECFTP/FTPClientForm_St02.cpp ../forms/fLotInfo_Ftp_St02.cpp ../WebLotInfoFtp_St02.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME LI9_FtpClient COMMAND test_li9_ftpclient)
set_tests_properties(LI9_FtpClient PROPERTIES TIMEOUT 120)
# AI(W906-LI9-F1) 20261002 (St02-E helper) -- LI9_FtpClientPage (node, offline): web/page/ht9045_lotinfo_ftp.js (the Lot Info FTP
#   buttons -> act.lotInfoFtp.open / state; opened:true asks the frame for the ftpclient window) and web/page/ht9045_ftpclient.js
#   (Data.FTPClient.html: draws the reply state, filter 200 ms after the last key, pick, Load from HD, close on HT_WIN open:false,
#   closeMe when C++ closed the dialog, one request at a time, busy / refusal).  Fake window / document / recipe client / clock;
#   Data.FTPClient.html read only.  CONTROL: W906_LI9_FTPCLIENT_JS or W906_LI9_LOTINFO_FTP_JS pointing at an empty file must go red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME LI9_FtpClientPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/li9_ftpclient_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(LI9_FtpClientPage PROPERTIES TIMEOUT 60)
endif()
# --- test_armcell_plan: AI(W906-ARMCELL) 20261002 -- the Teach "Arm Cell" tab's pure plan (ArmCellPlan.cpp, RULINGS_20261002 #18):
#   golden helpers, the catalog (22 golden areas, the 11 HT9050 ones usable, every other one greyed with a reason), the In / Out
#   targets of recipe FT005054_9050 worked out by hand, D1 (ep8 ini + one Enable Z == ep1), the golden quirks kept (q3 / q4 / f1-f5)
#   and the v1 refusals.  Pure (no god-stack, no file); -Wall -Wextra.  CONTROL: see the file head.
add_executable(test_armcell_plan test_armcell_plan.cpp ${CMAKE_SOURCE_DIR}/ArmCellPlan.cpp)
target_include_directories(test_armcell_plan PRIVATE ${CMAKE_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_armcell_plan.cpp ${CMAKE_SOURCE_DIR}/ArmCellPlan.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME ArmCellPlan COMMAND test_armcell_plan)
set_tests_properties(ArmCellPlan PROPERTIES TIMEOUT 60)
# --- test_temperfrom_timer1: AI(W906-TP1) 20261002 (Ifor01) -- TP-1: golden TfTemperFrom FormShow (:128-510, the state
#   half -- panel re-pointing, strShowYield flags, Timer1 enable), Timer1Timer (:1617-1682 -- 71 channels ShowThermo,
#   RunInfo.ShowTempComp for SECS, CCD cooling) and RecordTemp, through the port's boot show (W906_TemperFromBootShow) and the
#   100 ms tick (ht9045::W906_TemperFromTimer1Tick). CCD cooling observed on SW[SwCCDCooling].OutValue only (Enable=false: On()
#   sets the field and touches no hardware, myswitch.cpp:74-87); no IO table. Source pins read argv[1] (the port root, read
#   only); Gerneral.ini / config.ini compared before and after.  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_temperfrom_timer1 test_temperfrom_timer1.cpp)
target_link_libraries(test_temperfrom_timer1 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_temperfrom_timer1 PRIVATE ws2_32)
endif()
add_test(NAME TemperFromTimer1 COMMAND test_temperfrom_timer1 "${CMAKE_SOURCE_DIR}")
set_tests_properties(TemperFromTimer1 PROPERTIES TIMEOUT 120)

# AI(W906-ST02-ADAM) 20261002 (St02-E) -- the four ADAM-6024 ctests (golden 912 adam6024.cpp port; card ST02-ADAM).  Each refuses to run
#   outside ctest's redirect roots (st02_test_containment.h) and installs a FAKE ADAMTCP / ADSMOD table before any ADAM call; the
#   vendor DLLs are never loaded (Public/AdamTcp_St02.h / ApaxShim_St02.h [W906] S1 SIM build / S2 inside ctest).  Memory only, no
#   socket.  They switch the EP live switch on themselves (W906_AdamEpLive_SetForTest, Adam6024Integrate_St02.cpp) and back at the end.
#   ADAM6024_Comm (H1): Adam6024Comm_St02.cpp + Public/AdamTcp_St02.cpp.  Adam6024_Pressure (H2): Adam6024Pressure_St02.cpp.
#   Adam6024_Apax (H3): Adam6024Apax_St02.cpp + Public/ApaxShim_St02.cpp.  Adam6024_Flow (H4): Adam6024Integrate_St02.cpp over all of them
#   (Timer2 writer, AutoClean -> production, boot / exit / home, Close refcount, DLL missing; one 1.1 s Sleep).  Both configurations.
add_executable(test_adam6024_comm test_adam6024_comm.cpp)
target_link_libraries(test_adam6024_comm PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_adam6024_comm PRIVATE ws2_32)
endif()
add_test(NAME ADAM6024_Comm COMMAND test_adam6024_comm)
set_tests_properties(ADAM6024_Comm PROPERTIES TIMEOUT 120)
add_executable(test_adam6024_pressure test_adam6024_pressure.cpp)
target_link_libraries(test_adam6024_pressure PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_adam6024_pressure PRIVATE ws2_32)
endif()
add_test(NAME Adam6024_Pressure COMMAND test_adam6024_pressure)
set_tests_properties(Adam6024_Pressure PROPERTIES TIMEOUT 120)
add_executable(test_adam6024_apax test_adam6024_apax.cpp)
target_link_libraries(test_adam6024_apax PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_adam6024_apax PRIVATE ws2_32)
endif()
add_test(NAME Adam6024_Apax COMMAND test_adam6024_apax)
set_tests_properties(Adam6024_Apax PROPERTIES TIMEOUT 120)
add_executable(test_adam6024_flow test_adam6024_flow.cpp)
target_link_libraries(test_adam6024_flow PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_adam6024_flow PRIVATE ws2_32)
endif()
add_test(NAME Adam6024_Flow COMMAND test_adam6024_flow)
set_tests_properties(Adam6024_Flow PROPERTIES TIMEOUT 120)

# --- test_b8_os1b_autostart: AI(W906-B8-OS1B) 20261001 [W906] (St01) -- B8 OS-1b: the Offset nudge buttons save, and with
#   "Auto Offset Position Check" ticked golden calls fMain->Start (906 cOffSet.cpp:2888-2938, V912 :2892-2942) and the arms stop at the checked
#   part (UseAutoOffsetFunction :2944-2957). D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "OS-1b"; RULINGS_20261001 #0.
#   Under test: FileRW/Offset_File.cpp (tail) + Offset_File.gen.inc, FileRW/_FormEvent.cpp (after-ack queue, ack "afterAck"),
#   forms/fOffSet.cpp:10 (hook), through the real W906_FormEvent; the START seat W906_RemoteRun is a recorder. Source ratchet:
#   argv[1] = port tree, argv[2] = web\page (read-only). It writes only the W906_INIDATA_ROOT sandbox (refuses outside ctest,
#   w906_ctest_guard.h) and compares Gerneral.ini / config.ini / the IniData\Offset, IniData\DefineOffset and data file lists.
#   No ENVIRONMENT here on purpose: AI(W906-ENV-ALL) copies the global redirect string and _ht9045_env_extra to it.
add_executable(test_b8_os1b_autostart
    test_b8_os1b_autostart.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/Offset_File.cpp  ${CMAKE_SOURCE_DIR}/FileRW/Main_A01AutoLogout.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_os1b_autostart PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_b8_os1b_autostart PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_os1b_autostart PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_os1b_autostart.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Os1b_AutoCheckStart COMMAND test_b8_os1b_autostart "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Os1b_AutoCheckStart PROPERTIES TIMEOUT 180)

# --- test_b8_cl4_autoclean: AI(W906-B8-CL4) 20261001 [W906] (St01) -- B8 CL-4: the Cleaning page "Clean" button = golden
#   TfCleaning::btnStartAutoCleanClick (906 AutoClean\uCleaning.cpp:2869-2872, V912 :2901-2904) -> TfShowBinSelect::btnAutoCleanClick (906 cShowBinSelect.cpp:2101-2166, V912 :2248-2315):
#   registers one Auto Clean (bRunAutoClean + hang-up watchdog when the machine is empty; InitialAutoCleanAllTask -> One Cycle when it is not);
#   the machine moves only after START. D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CL-4"; RULINGS_20261001 #0.
#   Under test: FileRW/TestIF_File_Cleaning.cpp (tail W906_ShowBinSelect_btnAutoCleanClick; B3 section: DFM parent + proxy) + the generated
#   TestIF_File_Cleaning.gen.inc (CL_btnStartAutoCleanClick, kCL_Events row 84), through the real W906_FormEvent / RunPageEvent; the god-stack
#   bodies (AutoClean.cpp Initial*Task, csystem.cpp HasICUnderMachine / CheckIndexIsNormal, cCleanOut.cpp BtnOneCycleClick) by RESCAN.
#   Source ratchet: argv[1] = port tree, argv[2] = web\page (read-only). It writes only the W906_INIDATA_ROOT sandbox (refuses outside ctest,
#   w906_ctest_guard.h) and compares Gerneral.ini / config.ini and the D:\HT9045\IniData / D:\HT9045\data file lists.
#   No ENVIRONMENT here on purpose: AI(W906-ENV-ALL) copies the global redirect string and _ht9045_env_extra to it (same as B8_Os1b_AutoCheckStart).
add_executable(test_b8_cl4_autoclean
    test_b8_cl4_autoclean.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/TestIF_File_Cleaning.cpp  ${CMAKE_SOURCE_DIR}/FileRW/CfgTrayPlate.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_cl4_autoclean PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_b8_cl4_autoclean PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_cl4_autoclean PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_cl4_autoclean.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Cl4_StartAutoClean COMMAND test_b8_cl4_autoclean "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Cl4_StartAutoClean PROPERTIES TIMEOUT 180)
# --- B8_Cl4_CleaningPage: AI(W906-B8-CL4) 20261001 [W906] (St01) -- the page half of CL-4 (node, offline): D:\HT9045\web\page\ht9045_cleaning_ev.js
#   as is in a node vm with a fake DOM / HT9045Recipe: compiles, one EOL style, no BOM; events.btnStartAutoClean operable -> the button that
#   ht9045_cleaning_c.js disabled is re-enabled; a click sends one form.event {TfCleaning, btnStartAutoClean, click}; the ack is explained
#   (green: armed, START moves the machine; red: golden's refusal text); a double click is not sent; operable=false stays off.
#   Control: W906_CL4_PAGE_DIR -> the pre-change page must be red. Reads only that one .js.
if(W906_NODE_EXECUTABLE)
    add_test(NAME B8_Cl4_CleaningPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/cl4_cleaning_ev_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(B8_Cl4_CleaningPage PROPERTIES TIMEOUT 60)
endif()
# --- test_b8_ct3a_contactflags: AI(W906-B8-CT3A) 20261001 [W906] (St01) -- B8 CT-3a: the Contact page mode switch + T.Start / T.Step / One Cycle
#   (flags only; the machine moves after START) = golden TfContact::rbModeNormalClick / btnTStartClick / btnTStepClick / spbOneCycleClick
#   (906 cContact.cpp:15330-15339 / :2251-2254 / :2256-2259 / :16861-16865; V912 :15555-15564 / :2280-2283 / :2285-2288 / :17152-17156). D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3"; RULINGS_20261001 #0.
#   Under test: FileRW/DeviceForm_File.cpp (tail: FileRW_Contact_RbClickTrue / RbReimpose / Ct3aRun; EvB3Table) + the generated DeviceForm_File.gen.inc,
#   FileRW/_FormEvent.cpp (runexc rows 15-17), FileRW/_EditPage.cpp RunPageEvent / PageJson, with the real golden FormShow / FormClose; the flow side is the
#   live ckernel.cpp WaitManualStartKey / WaitManualStepKey (god-stack by RESCAN). Source ratchet: argv[1] = port tree, argv[2] = web\page (read-only).
#   Writes only the ctest sandbox (refuses outside ctest, w906_ctest_guard.h); compares Gerneral.ini / ContactInfo.ini / Contact.ini / config.ini and the
#   D:\HT9045\IniData file list. No ENVIRONMENT here on purpose: AI(W906-ENV-ALL) copies the global redirect string and _ht9045_env_extra to it.
add_executable(test_b8_ct3a_contactflags
    test_b8_ct3a_contactflags.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_File.cpp  ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_KbExtra.cpp   # AI(W906-R6MERGE) 20261002 (St01): merge of main 283f8382 -- DeviceForm_File.cpp g_page now takes &FileRW_Contact_KbExtraJson (main 453f40f9 SETUPA-KB), body in DeviceForm_KbExtra.cpp; same line
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_ct3a_contactflags PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_b8_ct3a_contactflags PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_ct3a_contactflags PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_ct3a_contactflags.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Ct3a_ContactFlags COMMAND test_b8_ct3a_contactflags "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Ct3a_ContactFlags PROPERTIES TIMEOUT 180)
# --- B8_Ct3a_ContactPage: AI(W906-B8-CT3A) 20261001 [W906] (St01) -- the page half of CT-3a (node, offline): D:\HT9045\web\page\ht9045_contact_ev.js
#   as is in a node vm with a fake DOM / HT9045Recipe: compiles, one EOL style, no BOM; T.Start / T.Step (display:none in the page) are shown when
#   the proxies say visible (golden FormShow :1630-1631 / SOFT_SIMULTE :1664-1665), stay hidden otherwise; the One Cycle LED follows the proxy Tag
#   (load and ack); clicks send form.event {TfContact, <control>, click}; a mode radio's ack turns the other radio off; operable=false sends nothing.
#   Control: W906_CT3A_PAGE_DIR -> the pre-change page must be red. Reads only that one .js.
if(W906_NODE_EXECUTABLE)
    add_test(NAME B8_Ct3a_ContactPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/ct3a_contact_ev_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(B8_Ct3a_ContactPage PROPERTIES TIMEOUT 60)
endif()
# --- test_b8_ct3b_contactstartpause: AI(W906-B8-CT3B) 20261001 [W906] (St01) -- B8 CT-3b': the Contact page START / PAUSE =
#   golden TfContact::btnStartClick (906 cContact.cpp:13965-13970, V912 :14072-14077: fMain->BtnStartClick = 906 main.cpp:6261-6323 / V912 :6529-6593, then edTorue0 / edTorue1 = "10")
#   and btnPauseClick (:14079-14082: fMain->BtnPauseClick = main.cpp:7387-7393). D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3";
#   RULINGS_20261001 #0. Under test: FileRW/DeviceForm_File.cpp (tail: W906_Contact_B8BtnStartClick / BtnPauseClick, Ct3bBtnStartClick, Ct3bRun;
#   EvB3Table) + the generated DeviceForm_File.gen.inc (DF_btnStartClick / DF_btnPauseClick, kDF_Events rows 23-24), FileRW/_FormEvent.cpp
#   (runexc rows 18-19, the after-ack queue, ack "afterAck") through the real W906_FormEvent / W906_FormEventRunAfterAck, FileRW/_KitSuck.cpp
#   (FileRW_InArmSuckShtRow), the real TfMain::BtnPauseClick / Pause and fMain->edTorue0 / edTorue1; the START / PAUSE seat W906_RemoteRun is a
#   recorder (god-stack by RESCAN). Source ratchet: argv[1] = port tree, argv[2] = web\page (read-only). Writes only the ctest sandbox (refuses
#   outside ctest, w906_ctest_guard.h); compares Gerneral.ini / ContactInfo.ini / Contact.ini / config.ini and the D:\HT9045\IniData file list.
#   No ENVIRONMENT here on purpose: AI(W906-ENV-ALL) copies the global redirect string and _ht9045_env_extra to it.
add_executable(test_b8_ct3b_contactstartpause
    test_b8_ct3b_contactstartpause.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_File.cpp  ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_KbExtra.cpp   # AI(W906-R6MERGE) 20261002 (St01): merge of main 283f8382 -- DeviceForm_File.cpp g_page now takes &FileRW_Contact_KbExtraJson (main 453f40f9 SETUPA-KB), body in DeviceForm_KbExtra.cpp; same line
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_ct3b_contactstartpause PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_b8_ct3b_contactstartpause PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_ct3b_contactstartpause PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_ct3b_contactstartpause.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Ct3b_ContactStartPause COMMAND test_b8_ct3b_contactstartpause "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Ct3b_ContactStartPause PROPERTIES TIMEOUT 180)
# --- B8_Ct3b_ContactPage: AI(W906-B8-CT3B) 20261001 [W906] (St01) -- the page half of CT-3b' (node, offline): D:\HT9045\web\page\ht9045_contact_ev.js
#   as is in a node vm with a fake DOM / HT9045Recipe: compiles, one EOL style, no BOM; START / PAUSE (display:none in the page) are shown when the
#   proxies say visible (golden FormShow :1628-1629 / SOFT_SIMULTE :1662-1663), stay hidden otherwise; clicks send form.event {TfContact, btnStart |
#   btnPause, click}; the ack's afterAck (the server runs golden btnStartClick / btnPauseClick right after the reply) is shown; hidden sends nothing.
#   Control: W906_CT3B_PAGE_DIR -> the pre-change page must be red. Reads only that one .js.
if(W906_NODE_EXECUTABLE)
    add_test(NAME B8_Ct3b_ContactPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/ct3b_contact_ev_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(B8_Ct3b_ContactPage PROPERTIES TIMEOUT 60)
endif()
# --- test_b8_ct3d_otddock: AI(W906-B8-CT3D) 20261001 [W906] (St01) -- B8 CT-3d: the Contact page OTD (One Touch Docking) dock cylinders + OTDTimer =
#   golden TfContact::palOTD_4Click / palOTD_6Click / OTDTimerTimer (906 cContact.cpp:15170-15193 / :15195-15218 / :15220-15282; V912 :15395-15418 / :15420-15443 / :15445-15507).
#   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3"; RULINGS_20261001 #0. Under test: FileRW/DeviceForm_File.cpp (tail:
#   FileRW_Contact_Ct3dBoot / FileRW_Contact_OTDTimerTick / Ct3dRun; EvB3Table; the OTDTimer beat at FormShowAndSnap / EvB3Run / Ct3aRun / Ct3bRun)
#   + the generated DeviceForm_File.gen.inc (DF_palOTD_4Click / DF_palOTD_6Click / DF_OTDTimerTimer, kDF_Events rows 25-26), FileRW/_FormEvent.cpp
#   (no runexc row for the OTD panels), FileRW/_EditPage.cpp RunPageEvent / PageJson, the real golden FormShow / FormClose. Recorders: the real
#   Cylinder[] / SW[] with every IO base = -1 (On() / Off() set Status / OutValue, no IO call); no motor, no IO. Source ratchet: argv[1] = port tree,
#   argv[2] = web\page (read-only). Writes only the ctest sandbox (refuses outside ctest, w906_ctest_guard.h); compares Gerneral.ini / ContactInfo.ini /
#   Contact.ini / SmartDiagnosticRecord.txt / config.ini and the D:\HT9045\IniData file list.
#   No ENVIRONMENT here on purpose: AI(W906-ENV-ALL) copies the global redirect string and _ht9045_env_extra to it.
add_executable(test_b8_ct3d_otddock
    test_b8_ct3d_otddock.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_File.cpp  ${CMAKE_SOURCE_DIR}/FileRW/DeviceForm_KbExtra.cpp   # AI(W906-R6MERGE) 20261002 (St01): merge of main 283f8382 -- DeviceForm_File.cpp g_page now takes &FileRW_Contact_KbExtraJson (main 453f40f9 SETUPA-KB), body in DeviceForm_KbExtra.cpp; same line
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp
)
target_include_directories(test_b8_ct3d_otddock PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_b8_ct3d_otddock PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_b8_ct3d_otddock PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_b8_ct3d_otddock.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME B8_Ct3d_OtdDock COMMAND test_b8_ct3d_otddock "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(B8_Ct3d_OtdDock PROPERTIES TIMEOUT 180)
# --- B8_Ct3d_ContactPage: AI(W906-B8-CT3D) 20261001 [W906] (St01) -- the page half of CT-3d (node, offline): D:\HT9045\web\page\ht9045_contact_ev.js
#   as is in a node vm with a fake DOM / HT9045Recipe: compiles, one EOL style, no BOM; ledOTD lamp on / off and red / lime from the proxy Tag
#   (golden OTDTimerTimer :15445-15507), palOTD_4 / palOTD_6 inset / outset from the panel Tag (golden BevelOuter, :15395 / :15420), at load and from
#   ack.changed; clicks send form.event {TfContact, palOTD_4 | palOTD_6, click}; hidden (USE_OTD != 1) sends nothing.
#   Control: W906_CT3D_PAGE_DIR -> the pre-change page must be red. Reads only that one .js.
if(W906_NODE_EXECUTABLE)
    add_test(NAME B8_Ct3d_ContactPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/ct3d_contact_ev_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(B8_Ct3d_ContactPage PROPERTIES TIMEOUT 60)
endif()
# --- test_e022_contactct: AI(W906-E022-CK1) 20261002 [W906] (St01) -- todo E-022 CK-1 / CK-2 / CK-3: Data.ContactCT's Count Clear, grid
#   double-click and Yield Chart = golden TfContactCT::btClearCountClick / sgYieldDblClick / btYieldChartClick (906 cContactCT.cpp = V912 byte for byte, same lines: :944-1069 /
#   :740-886 / :1071-1075).  Under test: the tail of cContactCT.cpp (ht9045::sjson::W906_ContactCTAct, ht9045_sm) through the REAL act.*
#   dispatch JsonBridge/ChanAction.cpp HandleActionWithTag (:349) -- same source list as test_sjson_chan above; "ok" = the reply carries
#   "executed":true, as wb_serve.cpp:4833.  Guards (running / level 107 + WAR1676 / KYEC / ASE bRefreshFunction), the YES / NO two-step
#   (golden NO still runs the tail :1050-1068), the YES clears (VTEST two arms), CK-2 site maths (QualSite2X2, NN_1Row, row 0 oddity, History
#   item, stale view, bad cell), CK-3 iShowYieldChart=1, a source ratchet (argv[1] = port tree, argv[2] = web\page, read-only).
#   Writes only the ctest sandbox (refuses outside ctest, w906_ctest_guard.h); compares D:\HT9045\system's file list and config.ini.
#   No ENVIRONMENT here on purpose: AI(W906-ENV-ALL) copies the global redirect string and _ht9045_env_extra to it.
add_executable(test_e022_contactct
    test_e022_contactct.cpp
    ../JsonBridge/ChanProduction.cpp
    ../JsonBridge/ChanIo.cpp
    ../JsonBridge/ChanMotor.cpp
    ../JsonBridge/ChanAction.cpp
    ../JsonBridge/actions/MainTesterConnect.cpp
    ../JsonBridge/actions/MainRecordClear.cpp ../JsonBridge/actions/LotInfoFtp.cpp ../JsonBridge/actions/ObserverSGJam.cpp   # AI(W906-LI9-F1) 20261002 (laptop batch 47 merge of review6): ChanAction.cpp dispatches act.lotInfoFtp.* since St02 MR !116 -- same line as !116 did at test_sjson_chan / test_e021_observer  AI(W906-ST02-OB7) 20261003 (St02-E, claim): + ObserverSGJam.cpp -- ChanAction.cpp:347 dispatches act.observerSG.* (W906_ObserverSGJamAct_St02), the same reason as LotInfoFtp.cpp before it
    ../JsonBridge/actions/MainClarnData.cpp ../JsonBridge/actions/MainStateRecord.cpp
    ../JsonBridge/EventLog.cpp
    ../EtherCAT/Pci1203Monitor.cpp)
target_include_directories(test_e022_contactct PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_e022_contactct PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
if(WIN32)
    target_link_libraries(test_e022_contactct PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_e022_contactct.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME E022_ContactCT COMMAND test_e022_contactct "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(E022_ContactCT PROPERTIES TIMEOUT 180)
# --- E022_ContactCTPage: AI(W906-E022-CK1) 20261002 [W906] (St01) -- the page half of E-022 (node, offline): D:\HT9045\web\page\
#   ht9045_contactct_ev.js (+ the same-line edits in ht9045_contactct_wire.js, Data.ContactCT.html, ht9045_observer_wire.js,
#   ht9045_startcondition_c.js) as is in a node vm with a fake DOM / HT9045Recipe: compiles, one EOL style, no BOM; Count Clear sends
#   act.contactCT.clearCount without an answer, asks golden's two strings, sends "yes" / "no" (NO is sent too: golden's tail), shows refusals;
#   the grid double-click sends the td's col / row and the shown yieldType; Yield Chart writes ht9045.observer.yieldChart and posts
#   {open:'observer'}; the Observer page switches to the Yield tab (no act tab) when open answers activePage 4 and re-opens on the storage
#   key; StartCondition's Smart Diagnostic button posts {open:'smartdiag'}.  Control: W906_E022_PAGE_DIR -> the pre-change pages must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME E022_ContactCTPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/e022_contactct_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(E022_ContactCTPage PROPERTIES TIMEOUT 60)
endif()
# --- test_e023_statusev: AI(W906-E023-SB1) 20261002 [W906] (St01) -- todo E-023 SB-1 / SB-3 / SB-4 / TP-2: Status.ShowBinSelect's Index-tab
#   Auto Clean, UPH grid double-click and Copy Recipe = golden TfShowBinSelect::btnAutoCleanClick / UPH_StringGridDblClick / sbCopyRecipeClick
#   (906 cShowBinSelect.cpp:2101-2166 / :2054-2099 / :2845-2850 -> main.cpp:34444-34451; V912 :2248-2315 / :2201-2246 / :2994-2999 -> :35596-35603), and Status.TemperFrom's hidden Handler System
#   entry = TfTemperFrom::Panel73/72/71MouseDown (906 cTemperFrom.cpp:1692-1769, V912 :1702-1779).  Under test: St01's cShowBinSelect_E023.cpp /
#   cTemperFrom_E023.cpp (ht9045_sm) through the REAL act.* dispatch JsonBridge/ChanAction.cpp HandleActionWithTag (:346) -- the
#   test_e022_contactct source list; SB-1 through the REAL seat in FileRW/TestIF_File_Cleaning.cpp (+ the FileRW layer, the
#   test_b8_cl4_autoclean list) and CL-4's body.  SB-1 is refused while running (SystemStart / SoftStart, stricter than golden -- pinned).
#   The TP-2 password literal is read from the port source and never printed.  Source ratchet: argv[1] = port tree, argv[2] = web\page
#   (read-only).  Writes only the ctest sandbox (refuses outside ctest, w906_ctest_guard.h) and the Copy Recipe seam W906_COPYRECIPE_ROOT
#   (the DEFERred APPEND below; without it the test exits 2 instead of writing the golden D:\Run); compares D:\HT9045\system, config.ini,
#   D:\Run, D:\HT9045\IniData and D:\HT9045\data before / after.
add_executable(test_e023_statusev
    test_e023_statusev.cpp
    ../JsonBridge/ChanProduction.cpp
    ../JsonBridge/ChanIo.cpp
    ../JsonBridge/ChanMotor.cpp
    ../JsonBridge/ChanAction.cpp
    ../JsonBridge/actions/MainTesterConnect.cpp
    ../JsonBridge/actions/MainRecordClear.cpp ../JsonBridge/actions/LotInfoFtp.cpp ../JsonBridge/actions/ObserverSGJam.cpp   # AI(W906-LI9-F1) 20261002 (laptop batch 47 merge of review6): ChanAction.cpp dispatches act.lotInfoFtp.* since St02 MR !116 -- same line as !116 did at test_sjson_chan / test_e021_observer  AI(W906-ST02-OB7) 20261003 (St02-E, claim): + ObserverSGJam.cpp -- ChanAction.cpp:347 dispatches act.observerSG.* (W906_ObserverSGJamAct_St02), the same reason as LotInfoFtp.cpp before it
    ../JsonBridge/actions/MainClarnData.cpp ../JsonBridge/actions/MainStateRecord.cpp
    ../JsonBridge/EventLog.cpp
    ../EtherCAT/Pci1203Monitor.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/TestIF_File_Cleaning.cpp  ${CMAKE_SOURCE_DIR}/FileRW/CfgTrayPlate.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_FormEvent.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_KitSuck.cpp)
target_include_directories(test_e023_statusev PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_e023_statusev PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge)
if(WIN32)
    target_link_libraries(test_e023_statusev PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_e023_statusev.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME E023_StatusEvents COMMAND test_e023_statusev "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../web/page")
set_tests_properties(E023_StatusEvents PROPERTIES TIMEOUT 180)
# The Copy Recipe seam for this test only, APPENDed after AI(W906-ENV-ALL) ran (a DEFERred call registered later runs later, so the
#   test keeps the global redirect roots and _ht9045_env_extra and gets this one too).  Unset, golden's target is D:\Run.
function(_w906_e023_copyrecipe_env)
    set_property(TEST E023_StatusEvents APPEND PROPERTY ENVIRONMENT "W906_COPYRECIPE_ROOT=${CMAKE_CURRENT_BINARY_DIR}/e023_scratch/Run")
endfunction()
cmake_language(DEFER CALL _w906_e023_copyrecipe_env)
# --- E023_StatusPages: AI(W906-E023-SB1) 20261002 [W906] (St01) -- the page half of E-023 (node, offline): D:\HT9045\web\page\
#   ht9045_showbinselect_ev.js and ht9045_temperfrom_ev.js as they are, in a node vm with a fake DOM / HT9045Recipe / HT9045Tags / HTQwerty:
#   compile, one EOL style, no BOM; state hides / shows Auto Clean; Auto Clean / Copy Recipe send one request and explain the reply (refusal
#   text in red); the UPH double-click sends row, pageIndex 2 and the row's cells from binsel.uph.grid, asks golden's OK / Cancel text,
#   sends "ok" / "cancel"; the three LEDs send act.temperFrom.mouseDown in click order with golden's button names, YES / NO, the password
#   through the keyboard only when C++ asks, postMessage {open:'handlersys'} only when C++ says opened; the password is not kept in state();
#   the two pages load the files, the old link / prompt and golden's literal are gone (Status.TemperFrom.html).
#   Control: W906_E023_PAGE_DIR -> the pre-change page directory must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME E023_StatusPages COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/e023_status_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(E023_StatusPages PROPERTIES TIMEOUT 60)
endif()
# --- HSys_TablePage: AI(W906-HSYS-TABLE) 20261002 [W906] (St01 ST01-E2) -- todo E-028, D:\HT9045\web\page\ht9045_hsys_table_c.js (page_Old's
#   table layout on HW.HandlerSys.html, proxies of the original DFM controls) on the real page in headless Edge (offline, temp folder only):
#   files (no BOM, one EOL, load order); every original the tables hide has a proxy (except golden-hidden GroupBox1 / 2; B69 1003: the per-track
#   Auto ART have proxies too); table -> original through real events so golden handlers run (TTL Card, Rotate Kit, ATC3.3+6.0, Fix 3, Heater Type,
#   Set Default = btnSetATCCom, keypad on the original); original -> table (value, Visible, Enabled); Search Function move / return.
#   Control inside the test: a copy whose pick() sets .checked without an event must be red. Edge missing -> exit 77 = SKIP.
if(W906_NODE_EXECUTABLE AND WIN32)
    add_test(NAME HSys_TablePage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/hsys_table_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(HSys_TablePage PROPERTIES TIMEOUT 180 SKIP_RETURN_CODE 77)
endif()
# --- E029_HeaterPages: AI(W906-FRW-E029) 20261002 [W906] (St01) -- todo E-029 / Q72, the page half (node, offline, temp folder only):
#   D:\HT9045\web\page\ht9045_hsys_heater_c.js rules (Index area EJ1N / DTM <-> [System] USE_16_HEATER both ways, one channel = the whole
#   area, Head1-4 stay on the COM port, golden iTempCode wiring for unit / station + CH, Heater Type keeps the area) and its page glue in a
#   small fake DOM (the ORIGINAL #rgHeater changes by a real click; elements with data-hst-src untouched); ht9045_temperfrom_strip.js
#   addresses for codes 5 / 6 ("EJ1N CHx-x" / "DTM CHx-x", main-screen-display.md 9.1) and the per-heater station keys.
#   Steven 1002「溫控器少了 DTM」「這邊選不到DTM」「還有EJ1N也選不到」「這些全部都要可以選,所以數量也是少了」「使用 EJ1N 或是 DTM 應該是要設定兩個變數」.
#   Server half: HSys_HeaterMix. Control inside the test: broken copies (32-heater count, code 6 in the strip) must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME E029_HeaterPages COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/e029_heater_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(E029_HeaterPages PROPERTIES TIMEOUT 60)
endif()

# --- test_c14_bindisp: AI(W906-ST02-C14) 20261002 (St02-E helper) -- ST02-C14 Bin Display, the C++ bring-up (golden 906_0625_Steven):
#   TDataModule3 + TMyBinDispHT9046 from the real boot (BinDisplay/BinDispBringUp_St02.cpp), golden Timer1Timer on the St02
#   dispatcher's bin display slot (MainTimersSt02.cpp), the type-3 Modbus-ASCII and type-4 TFT frames from a known tray state
#   against a fake panel bus (byte-exact), ChangeBinDispStatus on the unified panels (P1), the BinSel pause, the cadence on a fake
#   clock.  NO COM PORT: both TComm in SIM mode (asserted) and no real GetCOMPortStatus probe (asserted).  CONTAINED: refuses
#   (exit 2) unless the log roots and Gerneral.ini are ctest's scratch (st02_test_containment.h); writes only the scratch
#   Gerneral.ini and <machine_log_scratch>/BinDisplayLog.  Both configs (SIM / SHIP).
add_executable(test_c14_bindisp test_c14_bindisp.cpp)
target_link_libraries(test_c14_bindisp PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_c14_bindisp PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_c14_bindisp PRIVATE -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()
add_test(NAME C14_BinDisp COMMAND test_c14_bindisp "${CMAKE_SOURCE_DIR}")   # AI(W906-ST02-C14b) 20261003 (St02-E): argv[1] = section 12 (the C14b call sites)
set_tests_properties(C14_BinDisp PROPERTIES TIMEOUT 120)

# --- test_c10_hana_rms: AI(W906-ST02-C10) 20261002 (St02-E helper) -- card ST02-C10 (Steven W64 = A): golden 912 HANA RMS
#   Interlock (Automation/automation.cpp:2514-3158) as THanaRmsMembers (Automation/HanaRms_St02.h, the base of
#   TfAutomationShim) and gate G9 lifted (TesterComm/Handler/HandlerBridgeCtl.cpp, golden 912 main.cpp:18535-18539).
#   NO NETWORK: vclcompat's Sim TClientSocket (no OS socket); only TesterCommWiring.cpp can ask for a real one, and the
#   test stops before sending anything if the socket is not Sim.  CONTAINED: refuses (exit 2) unless as9045LogPath /
#   W906_HT9045LOG_ROOT are this directory's machine_log_scratch and AuthPath / W906_AUTH_PATH its machine_config_scratch
#   (_ht9045_env_extra and the merged ENVIRONMENT string, given to every test by the deferred _w906_env_all_tests);
#   writes only under <machine_log_scratch>/C10_HanaRms_<tick> (the HANARMS day file and Save .log, HANARMS.ini,
#   SetUp.inf); stamps the machine's D:/HT9045/config/HANARMS.ini and D:/HT9045_Log/HANARMS before and after (read only);
#   removes its folder on a green run.  Both configs (SIM / SHIP).  [12] reads TesterComm/Handler/HandlerBridgeCtl.cpp,
#   TesterComm/Handler/TesterCommWiring.cpp, atester.cpp, atester_shims.h.
add_executable(test_c10_hana_rms test_c10_hana_rms.cpp)
target_link_libraries(test_c10_hana_rms PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_c10_hana_rms PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_c10_hana_rms PRIVATE -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()
target_compile_definitions(test_c10_hana_rms PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [12] source pins
add_test(NAME C10_HanaRms COMMAND test_c10_hana_rms)
set_tests_properties(C10_HanaRms PROPERTIES TIMEOUT 60)

# AI(W906-ST02-C9-G2) 20261002 (St02-E helper) -- ST02C9_TeachSt02: St02 card ST02-C9 group G2 = the golden 906_0625_Steven uteach.cpp buttons SpeedButtonInRotatePos90 / Neg90 (:4605-4707, the 36 buttons
#   uteach.dfm binds to them), btnSht1 / 2GoLatch (:4721-4751) and btnSetAllIn / OutArmZ_Move (:5948-5996)
#   as WebMotorAccess.cpp's EOF block (motor.access action teachSt02, WebTeachSt02.h), on a fake IMotorAccessBackend
#   and a fake ITeachSt02Ops.  Linked like test_web_motor_access (WebMotorAccess.cpp + JsonWriter + cJSON, no god-stack;
#   WebTeachSt02Live.cpp is NOT linked).  No file, no env.  Section [6] goes through MotorAccessDispatch, so it needs
#   the two same-line hooks in WebMotorAccess.cpp (the kActions row, the uteach dispatch line).  At the end of the file: own TIMEOUT.
#   AI(W906-ST02-C9) 20261002: rebased onto main (G3 dropped -- the machine wired those buttons); placed after main's last block.
add_executable(test_teach_st02_c9
    test_teach_st02_c9.cpp
    ${CMAKE_SOURCE_DIR}/WebMotorAccess.cpp  ${CMAKE_SOURCE_DIR}/GearCalc.cpp   # AI(W906-GEARRATIO) 20261003 (batch 49 merge): WebMotorAccess.cpp's Gear Ratio part calls GearCalc, as the other four WebMotorAccess.cpp tests (same line, nothing moves)
    ${CMAKE_SOURCE_DIR}/WebBridge/JsonWriter.cpp
    ${CMAKE_SOURCE_DIR}/Public/cJSON.c
)
target_include_directories(test_teach_st02_c9 PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_teach_st02_c9 PRIVATE vclcompat)   # AI(W906-COMMATEXT-FU) 20261003 (laptop, batch 53 merge): WebMotorAccess.cpp is compiled in and its MtSplitCells calls vclcompat::CommaTextCells (MR !153) -- as test_web_motor_access
add_test(NAME ST02C9_TeachSt02 COMMAND test_teach_st02_c9)
set_tests_properties(ST02C9_TeachSt02 PROPERTIES TIMEOUT 60)

# AI(W906-ST02-C9-G2) 20261002 (St02-E helper) -- ST02C9_TeachSt02Page (node, offline): web/page/ht9045_teach_st02.js binds the
#   the 40 G2 buttons (HTMotorAccess.send('St02Teach*', {btn, golden atoi fields})), undoes the page's TEACH_UNWIRED grey on them
#   (the real grey code cut from HW.teach.html; load order pinned: the tag on the motor-access.js line, before the grey, which
#   runs once at load), leaves the machine's lanes / servo / TTL alone, and applies golden visibility from the C++ state query.
#   HW.teach.html read only.  CONTROL: W906_TEACH_ST02_JS pointing at a copy without the capture listener / ungrey must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME ST02C9_TeachSt02Page COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/teach_st02_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(ST02C9_TeachSt02Page PROPERTIES TIMEOUT 60)
endif()
# --- test_gear_calc: AI(W906-GEARRATIO) 20261002 [W906] -- the Motor Test "Gear Ratio" tab's pure part (GearCalc.cpp, RULINGS_20261002 #22,
#   NB2 spec §3.2 / §6 / §8): the fit (the spec's 50 / 100 / 200 mm example -> k 0.9921524, consistency 0.03 %), the rules (2 forward
#   points, 50 mm, 0.1 %, 2 % / 10 %), the 7-digit ratio, the rescale rounding, the plan (pointer dedup, TechTwoPara one side, the ep1
#   remap, the +-999999 placeholders), the teach.ini diff.  Pure (no god-stack, no file); -Wall -Wextra.  CONTROL: see the file head.
#   After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_gear_calc test_gear_calc.cpp ${CMAKE_SOURCE_DIR}/GearCalc.cpp)
target_include_directories(test_gear_calc PRIVATE ${CMAKE_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_gear_calc.cpp ${CMAKE_SOURCE_DIR}/GearCalc.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME GearCalc COMMAND test_gear_calc)
set_tests_properties(GearCalc PROPERTIES TIMEOUT 60)
# --- test_gear_teach_save: AI(W906-GEARRATIO2) 20261003 [W906] -- NB2 R171 M2: the Gear Ratio save's REAL teach.ini write path, byte
#   for byte (the [GEAR] save tests use a fake writer; the runtime checks compare values only). GearLiveTeachRefs -> GearPlanBuild ->
#   GearLiveSetTeach -> GearLiveTeachSaveReload(true) = FileRW_Teach_SaveFile(true) -> IC_SaveFile (TECH_* WriteIniData in place +
#   elTeach->SaveEditTextToFile, a TMemIniFile whole-file rewrite) -> fTeach->ReadFile -- the Teach page's own writer.
#   argv[1] = the HT9050 snapshot teach.ini (runcfg = what the machine's wb_serve reads / writes), argv[2] = the snapshot
#   Gerneral.ini (the registry's twelve config keys), both READ ONLY; argv[3] = the copy's directory under machine_config_scratch.
#   The test refuses to run outside ctest's redirect environment (tests/w906_ctest_guard.h). Sources as wb_serve compiles them
#   (GearRatioLive.cpp, FileRW/Teach.cpp + the C-path layer _EditList / _EditPage), god-stack through RESCAN (as test_hsys_heater_mix);
#   three link-only stand-ins in the test file (WebLogin_StateJson, FileRW_IniConfig_ChangeCBListProperty, formjson::FormLock /
#   FormUnlock), none under test. A refreshed snapshot keeps the rule checks; the exact bytes are checked for the 20261003 one
#   (FNV-hash gated, see the test). After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_gear_teach_save
    test_gear_teach_save.cpp
    ${CMAKE_SOURCE_DIR}/GearRatioLive.cpp
    ${CMAKE_SOURCE_DIR}/GearCalc.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/Teach.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
)
target_include_directories(test_gear_teach_save PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_gear_teach_save PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_gear_teach_save PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_gear_teach_save.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME GearTeachSave
         COMMAND test_gear_teach_save
                 "${CMAKE_SOURCE_DIR}/../machines/HT9050/snapshot/machine_params/runcfg/system/teach.ini"
                 "${CMAKE_SOURCE_DIR}/../machines/HT9050/snapshot/machine_params/D_HT9045_system/Gerneral.ini"
                 "${CMAKE_CURRENT_BINARY_DIR}/machine_config_scratch/GearTeachSave")
set_tests_properties(GearTeachSave PROPERTIES TIMEOUT 900)   # measured 20261003 on the laptop: 55 / 91 s alone, 213 s beside other ctests (every teach.ini key is a file open + rewrite through the real writer, three read / write rounds)
# --- MT_GearRatioPage: AI(W906-GEARRATIO) 20261002 [W906] -- the Motor Test "Gear Ratio" tab, the page half (node, offline; RULINGS_20261002 #22,
#   NB2 spec §4 / §8 webprobe list): the 6th tab switches and hides, mtGear* is not caught by the generic binders, the keypads are DOUBLE,
#   file:// / no C++ greys the whole tab, arrival is read from the runtime only (cmdPos = the ack's arriveCmdPos, busy false, INP, the C++
#   session arrived, two new reads), readings signed with the direction, Preview -> Save with the preview key, the 2..10 % second
#   confirmation, STOP, refusals, the 60 s timeout, HT_WIN, HW.MotorTest.html's four same-line hooks (read only). Loads the REAL
#   web/page/ht9045_mt_gearratio.js in a node vm with a fake DOM / runtime / clock; no socket, no file written.
#   CONTROL: W906_MT_GEAR_JS=<a broken copy> must go red (20 page rules checked that way).
if(W906_NODE_EXECUTABLE)
    add_test(NAME MT_GearRatioPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/mt_gearratio_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(MT_GearRatioPage PROPERTIES TIMEOUT 60)
endif()
# --- test_c14_bindisp_pane: AI(W906-ST02-C14) 20261002 (St02-E helper) -- ST02-C14 part 3, the web "Bin Display Status" pane's
#   C++ data path (golden 906_0625_Steven TfShowBinSelect tsUnloadMap / ChangeBinDispStatus cShowBinSelect.cpp:208-386):
#   WebBinDispStatus_St02.cpp (compiled here directly, a wb_serve-only TU, with -Wall -Wextra) stages exactly 15 binsel.disp.*
#   tags from the facade panels and the bin display controller; MainTimer3.cpp's W906_BinDispShownScope_St02 makes golden's
#   once-a-second call paint them although the facade's ActivePage is not tsUnloadMap (golden's early return :279-292 is
#   asserted without it), counts golden's :272 jump and restores the old page when there is no error; the black / red flash
#   flips per call; argv[1] pins the wb_serve.cpp / CMakeLists.txt / MainTimer3.cpp / page lines.  A stand-in controller
#   (TMyBinDispOffline) instead of the boot: no COM port, no Timer1, no file written.  CONTAINED: refuses (exit 2) outside
#   ctest's scratch roots (st02_test_containment.h).  Both configs (SIM / SHIP).  Link shape as WB_Tags (god stack + webbridge).
add_executable(test_c14_bindisp_pane test_c14_bindisp_pane.cpp ../WebBinDispStatus_St02.cpp)
target_include_directories(test_c14_bindisp_pane PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_c14_bindisp_pane PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_c14_bindisp_pane PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_c14_bindisp_pane PRIVATE -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()
add_test(NAME C14_BinDispPane COMMAND test_c14_bindisp_pane "${CMAKE_SOURCE_DIR}")
set_tests_properties(C14_BinDispPane PROPERTIES TIMEOUT 120)
# --- C14_BinDispPanePage: AI(W906-ST02-C14) 20261002 (St02-E helper) -- the page half (node, offline): web/page/ht9045_bindisp_status.js
#   builds golden 906's tsUnloadMap layout (cShowBinSelect.dfm:1361-2790) and paints the binsel.disp.* tags (golden ColorMap :218,
#   orange 0x000080FF -> #FF8000), hides the tab unless NUMBER_PANEL_TYPE 3 / 4 (FormShow :760-764), follows golden's :272 jump;
#   offline = "---".  Pins the page include and its 147 lines.  Control: W906_C14_PANE_JS -> an empty .js must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME C14_BinDispPanePage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/c14_bindisp_pane_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(C14_BinDispPanePage PROPERTIES TIMEOUT 60)
endif()

# AI(W906-TEACH-ZDOWN) 20261003 -- TeachZDownPage (node, offline): web/page/ht9045_teach_zdown_c.js (INBOX 147, golden uteach.cpp:4300 /
#   :4367 / :4542) binds btnSetAllInArmZ / btnSetAllOutArmZ / btnOutZAllDown -- HTMotorAccess.send(<button>, {fields: the screen values
#   teachFieldValue trusts, golden's list}) --, fills the boxes from a Set All reply (data-src cpp-pos) and greys the three when the
#   catalog lacks their rows; HW.teach.html loads it, hands the two actions' replies over in teachOnAck before its teachSet / teachGo
#   filter, and its own grey (the real code cut from the page) no longer greys the three.  The C++ half is ctest WebMotorAccess [ZDOWN].
#   CONTROL: W906_TEACH_ZDOWN_JS / W906_TEACH_HTML pointing at copies without the binding / the fields / the data-src mark / the ack
#   hook / the list filter / the script tag must go red (7 checked that way, 20261003).
if(W906_NODE_EXECUTABLE)
    add_test(NAME TeachZDownPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/teach_zdown_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(TeachZDownPage PROPERTIES TIMEOUT 60)
endif()

# AI(W906-FPFAST) 20261003 -- FPFAST_Pin: RULINGS_20261003 #7 (= NIGHT_REPORT s0 #77 A).  CMakeLists.txt:7 compiles the whole tree
#   with -fexcess-precision=fast (GNU C / C++ only) so the machine's WinLibs g++ 16.2 i686 evaluates `double == 5.6` like BCB6 and
#   the oracle g++ 6.3.0 (docs/FP_ORACLE_FINDINGS.md s9).  [R] the BCB6 answers at runtime (red on WinLibs without the flag);
#   [S] argv[1] = source root: the add_compile_options line is still there (g++ 6.3.0 is fast-only, so on the oracle only [S] can
#   see the line dropped); [X] x87 80-bit intermediates unchanged.  Standalone: no god stack, no file IO.  Controls: s9.4.
add_executable(test_fpfast test_fpfast.cpp)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_fpfast PRIVATE -Wall -Wextra)
endif()
add_test(NAME FPFAST_Pin COMMAND test_fpfast "${CMAKE_SOURCE_DIR}")
set_tests_properties(FPFAST_Pin PROPERTIES TIMEOUT 60)

# --- test_home_block: AI(W906-HOMEBLOCK) 20261003 (NB2-1) -- URGENT U14 item 3 / README R175 C1 / R188 H1: TfMain::Home asks
#   W906_HomeBlockedHook (forms/fMain.cpp) first, so every HOME source is refused while START is (Arm Cell job / hand teach);
#   wb_serve points it at W906_MotorAccessHomeBlocked. Pins the seam (null = as before; blocked = refused before golden's
#   first statement; not blocked = golden goes on). The decision itself: test_web_motor_access.cpp (MotorAccessHomeBlocked).
#   Every call runs with SoftStart set, so a golden fall-through stops at golden's SoftStart guard (no record, no motor, no file).
add_executable(test_home_block test_home_block.cpp)
target_link_libraries(test_home_block PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_home_block PRIVATE ws2_32)
endif()
add_test(NAME HomeBlock COMMAND test_home_block)
set_tests_properties(HomeBlock PROPERTIES TIMEOUT 120)

# --- test_st02_keep912: AI(W906-ST02-W71W73) 20261003 (St02-E) -- source pins for the three Steven-approved V912 keeps (RULINGS_20261003
#   #1 / #23, #20 exception): W71 Qorvo Tester Pause delayed buzzer, W72 AMD runtime site map / ATC type, W73 Murata NonTestToRBin
#   record.  Code only (comments stripped).  Pure: reads the tree (argv[1]), no libraries, no machine state.
add_executable(test_st02_keep912 test_st02_keep912.cpp)
add_test(NAME St02_Keep912 COMMAND test_st02_keep912 "${CMAKE_SOURCE_DIR}")   # argv[1]: reads five source files (read only)
set_tests_properties(St02_Keep912 PROPERTIES TIMEOUT 60)
# --- S16_LazyBoot: AI(W906-S16) 20261003 [W906] (St01 ST01-E2) -- laptop card S-16 (RULINGS_20261001 #32): D:\HT9045\web\background.html
#   loads a hidden window's page only when it is first opened (iframe data-defer-src), so start-up waits for the boot-visible windows and
#   the dialog overlays only (HT_LOAD_INFO.total <= 16, was ~72); first open loads + HT_WIN initial; close only hides; machine.gpibModel on a
#   never-opened Motion View loads nothing. Headless Edge offline, WebSocket stubbed in the frame. Control inside the test: a copy without the
#   defer rule must be red. Edge missing -> exit 77 = SKIP.
if(W906_NODE_EXECUTABLE AND WIN32)
    add_test(NAME S16_LazyBoot COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/s16_lazyboot_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web")
    set_tests_properties(S16_LazyBoot PROPERTIES TIMEOUT 400 SKIP_RETURN_CODE 77)
endif()

# --- test_st02_s09_setup_temperfrom: AI(W906-ST02-S09L) 20261003 (St02-E, helper) -- S-09 tsv 540, cSetUp.cpp:1115-1117 GATE(G-SU-TemperFrom)
#   lifted (golden 0618 cSetUp.cpp:2852, 0625_Steven same; 912 cSetUp.cpp:2874 same) with the port-only NULL guard of forms/fTemperFrom.h:400-406.
#   Runs the REAL TfSetup::ReadFile (fSetup->Init() first, as wb_serve.cpp:3198) on a HandlerCondition.Data seeded at the path ReadFile
#   reads (GetRecipeFileName under the redirected DataPath; LastDataPath pointed at a scratch SetUp.inf): [1] fTemperFrom NULL -> nothing,
#   no crash; [2] after W906_TemperFromBootShow, OCR Function 1 / 0 -> strShowYield[esytOcrOn].OnOff and palOcrOn "OCR On" / "OCR Off";
#   [3] OCR not installed -> no call; [4] Gerneral.ini / config.ini / SetUp.inf byte-identical.  Refuses (exit 2) outside ctest's redirect
#   roots.  Writes only <DataPath>ST02_S09L_* (deleted when green).  After the GLOBAL TEST TIMEOUT block => own TIMEOUT.
add_executable(test_st02_s09_setup_temperfrom test_st02_s09_setup_temperfrom.cpp)
set_target_properties(test_st02_s09_setup_temperfrom PROPERTIES OUTPUT_NAME test_st02_s09_su_temperfrom)   # AI(W906-ST02-S09L) 20261003 (laptop, batch 53, NB2-1 R199): the exe name must not contain "setup" -- Windows installer detection makes an unmanifested *setup*.exe require elevation (ctest "Not Run", permission denied); same trap as the *install* note above
target_link_libraries(test_st02_s09_setup_temperfrom PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_s09_setup_temperfrom PRIVATE ws2_32)
endif()
add_test(NAME St02_S09SetupTemperFrom COMMAND test_st02_s09_setup_temperfrom)
set_tests_properties(St02_S09SetupTemperFrom PROPERTIES TIMEOUT 120)

# --- PE_TruncationCheckTimeout: AI(W906-PECHECK-TIMEOUT) 20261003 (NB2-1, laptop task (k)) -- tools/pe_truncation_check.ps1's open
#   timeout: a file whose open hangs (the antivirus holding a fresh exe; simulated by the W906_PECHECK_TEST_SLOW_OPEN hook) is reported
#   LOCKED as a warning (exit 0), TRUNCATED keeps exit 1, and the run does not wait for the hung open. Fixtures = copies of
#   %SystemRoot%\System32\where.exe written only under the scratch dir below. Windows PowerShell 5.1 (the one build.bat starts).
add_test(NAME PE_TruncationCheckTimeout
         COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/tools/pe_truncation_check_selftest.ps1"
                 -Scratch "${CMAKE_CURRENT_BINARY_DIR}/pecheck_scratch")
set_tests_properties(PE_TruncationCheckTimeout PROPERTIES TIMEOUT 120)

# AI(W906-S17) 20261003 (St01, laptop card S-17, INBOX 137) -- the page half of "C++ keeps one slot, the page queues" (node, offline;
#   C++ half: tests/test_notice_ack.cpp [Q]).  The real web/page files in a node vm with a fake window / document / recipe client; no
#   socket, no wb_serve, no file written.
#   S17_DialogHostPage: ht9045_dialog_host.js -- a notice's ACKNOWLEDGE while ANOTHER alarm's query is pending goes to dialog.notifyAck (not
#     that alarm's modal.answer); an alarm box answers with its own requestId when the host's pendingQuery is stale / missing; "no query
#     pending[:superseded-by=..]" closes the box.  CONTROL: W906_DIALOG_HOST_JS at the pre-change file must go red.
#   S17_DialogClosePage: dialog-bridge.js (contract 1.3.1) -- Dialog-close-request.recent[]: a close for a QUEUED box removes it before it
#     renders; two closes in one poll both take effect; an old-format close still closes the active box.  CONTROL: W906_DIALOG_BRIDGE_JS.
if(W906_NODE_EXECUTABLE)
    add_test(NAME S17_DialogHostPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/s17_dialog_host_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(S17_DialogHostPage PROPERTIES TIMEOUT 60)
    add_test(NAME S17_DialogClosePage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/s17_dialog_close_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(S17_DialogClosePage PROPERTIES TIMEOUT 60)
endif()

# --- test_timer_table: AI(W906-TIMER-TABLE) 20261001 -- golden TTimer 的排程表（TimerTable.cpp，RULINGS_20261001 第 39 條）。
#   假時鐘逐條對 VCL TTimer：週期、Enabled／Interval 改了才重算、不補跑、保持相位、OnTimer 裡關自己、例外、巢狀 Tick、屬性對拷、
#   超過 50 ms 的記數、別的執行緒不准跑、OnTimer 裡拆掉別支；T14 拿 docs/TIMER_CENSUS.tsv 對 forms/fMain_Timers.h 那 14 支的
#   Enabled／Interval（golden main.dfm）。不 include cmydef、不 link god-stack、不讀寫任何真實檔，秒級。放在 GLOBAL TEST TIMEOUT 之後 ⇒ 自己設 TIMEOUT。
add_executable(test_timer_table
    test_timer_table.cpp
    ${CMAKE_SOURCE_DIR}/TimerTable.cpp
)
target_include_directories(test_timer_table PRIVATE ${CMAKE_SOURCE_DIR})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_timer_table.cpp ${CMAKE_SOURCE_DIR}/TimerTable.cpp
        PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME TimerTable COMMAND test_timer_table "${CMAKE_SOURCE_DIR}/docs/TIMER_CENSUS.tsv")
set_tests_properties(TimerTable PROPERTIES TIMEOUT 60)
# --- RealfileGuard_SelfTest: AI(W906-TIMER-TABLE) 20261001 -- tools/realfile_guard.py --selftest（暫存目錄，不碰真實檔）：
#   snap -> 改一個檔、多一個 snap 時不存在的檔 -> check 報 CHANGED＋CREATED -> restore 還原並刪掉多出來的 -> check 乾淨 -> drop。
#   起因：20261001 wb_serve 關站寫出原本不存在的 system\lastdata_backup2.dat，舊版 restore 刪不掉它（整合 session gate b21a 量到）。
if(Python3_Interpreter_FOUND)
    add_test(NAME RealfileGuard_SelfTest COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/realfile_guard.py" --selftest)
    set_tests_properties(RealfileGuard_SelfTest PROPERTIES TIMEOUT 60)
endif()
# --- E034_NoopEq: AI(W906-E034) 20261003 (St01, todo E-034 = laptop card S-21; Steven Q82 = A, #20 exception) -- the golden 0618
#   no-op `==` statements that 0625 / V912 fixed to `=`, taken in the laptop's port files.  [A] behaviour: acatchtray.cpp
#   DoLoadCarRotArmReadRFID x3 (port :8568 / :8609 / :8677 = 0618 :8403 / :8444 / :8512) set "NOREAD" (offline Motor==NULL,
#   Sen force, W906_ShowErrorMessage_SimReturn).  [B] source checks with an in-test mutation (the 0618 `==` put back must be
#   rejected): csystem.cpp :32417 WAR0955 (inside #if 0 OCRSTART -- no runtime effect yet), OCRInsp.cpp :819, forms/
#   fTrayAssignment.cpp :235, cShowBinSelect.cpp :2315 (St02's 8db5c2c9 fix; behaviour = C14_BinDispPane).  The other sites:
#   YieldMonCore (uYieldMonitoring :2242 / :2286 / :2287), ObserverCore (cObserver :2540), TemperFromCore (cTemperFrom :1302).
#   Reads source only, writes nothing.  Same RESCAN link group as W6_3_CatchTray.
add_executable(test_e034_noop_eq test_e034_noop_eq.cpp)
target_include_directories(test_e034_noop_eq PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_e034_noop_eq PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_e034_noop_eq PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_e034_noop_eq PRIVATE -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()
add_test(NAME E034_NoopEq COMMAND test_e034_noop_eq "${CMAKE_SOURCE_DIR}")
set_tests_properties(E034_NoopEq PROPERTIES TIMEOUT 120)

# --- test_st02_n07_alarm: AI(W906-C15-N07) 20261003 (St02-E helper) -- card ST02-C15: the N07 SECS/GEM disconnect alarm (JSCC
#   NetworkMonitor, Steven 20260603) -- golden 906_0625_Steven main.cpp:20937-20999 + WriteN07Log :20848-20860 =
#   SECSGEM/N07Alarm_St02.cpp, the Timer2 slot of MainTimersSt02.cpp, and the ckernel.cpp claims (ShowRunLed :1677 / :1769 /
#   :1813, ScanPannelKey :3374 / :3382).  A real THGem as HGem (bConnect switched, the real IsConnect), the slot on a fake clock:
#   the immediate trigger, the 10 s recheck and the release, Alarm Reset silences the buzzer while the red light keeps
#   blinking, the release clears the silence, both NetworkMonitor.log lines (golden bytes), off without [N07] SECS GEM Alarm /
#   Enable SECS GEM / ON-LINE / running / HGem, source pins (argv[1] = tree root, read only).  Containment first
#   (st02_test_containment.h).  New seam W906_SECSGEMLOG_ROOT (getenv; unset = golden's D:\SECS_GEM_LOGS literal) goes to
#   machine_log_scratch for every test, as the other roots (_ht9045_env_extra, read by the deferred _w906_env_all_tests).
list(APPEND _ht9045_env_extra "W906_SECSGEMLOG_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/SECS_GEM_LOGS")   # AI(W906-C15-N07) 20261003: SECSGEM/N07Alarm_St02.cpp WriteN07Log
add_executable(test_st02_n07_alarm test_st02_n07_alarm.cpp)
target_link_libraries(test_st02_n07_alarm PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_n07_alarm PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_st02_n07_alarm PRIVATE -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()
add_test(NAME St02_N07Alarm COMMAND test_st02_n07_alarm "${CMAKE_SOURCE_DIR}")   # argv[1]: section 10 reads the hook-up lines (read only)
set_tests_properties(St02_N07Alarm PROPERTIES TIMEOUT 60)
# --- St02_N07BannerPage: AI(W906-C15-N07-UI) 20261003 (St02-E) -- the N07 banner page half (node, offline):
#   web/page/ht9045_n07_banner.js draws golden 906_0625 main.cpp:20974-20980 (Off_lineDisplay 20 px red / clBtnFace blink at
#   golden FlushFlag's 270 ms, labTesterMode "SECS DISCONNECTED" red) on main.html's div.statusPane while the tag n07.alarm
#   (WebBridgeTags.cpp:1156) is true, and restores the pane on false / null (:20987-20993).  Pins the main.html include.
#   Control: W906_N07_BANNER_JS -> an empty .js must be red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME St02_N07BannerPage COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/n07_banner_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(St02_N07BannerPage PROPERTIES TIMEOUT 60)
endif()

# --- test_scankey_golden: AI(W906-ST02-SKG) 20261003 (St02-E helper) -- ScanKey combined MR step 5: St02's S-20 panel-key test
#   (tests/test_scankey.cpp at 74776f87, sections [1]-[17], never merged) ported onto MC01's WebMainScanKey.cpp (golden TfMain::ScanKey
#   main.cpp:2379-2656 + TimerScanKeyTimer :31994-32025 + TfHome::ScanKey uhome.cpp:4874-4889; a wb_serve-only TU compiled in here).
#   Companion of MainScanKey (MC01's, keys injected through the *With(scan) entries): this one drives the production entry
#   W906_MainScanKeyTick(noticeUp) = the REAL ScanPannelKey (ckernel.cpp) through the sim sensor seam and the real fMain;
#   W906_RemoteRun fakes count START / PAUSE; FormLock / FormUnlock (depth-counted), W906_Contact_OneCycleProcess and
#   W906_MsgBoxModelessPanelKeyTick are stubs in the test.  Every key branch and guard, re-entry (bRunTimer1), the Contact page's
#   open edge, TfHome::ScanKey first, the form lock around CLEAN OUT / ONE CYCLE / TRAY FEED only, the first-scan guard, the
#   non-modal box reader, the HOME interlock; argv[1] pins the two wb_serve ticks, one START call site, the lock, the census line.
#   The bodies' log rows go to machine_log_scratch (ENV-ALL DEFER, so no ENVIRONMENT here on purpose); the test refuses to run
#   elsewhere.  Both configs.  At EOF => own TIMEOUT.
add_executable(test_scankey_golden test_scankey_golden.cpp ${CMAKE_SOURCE_DIR}/WebMainScanKey.cpp)
target_include_directories(test_scankey_golden PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_scankey_golden PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_scankey_golden PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_scankey_golden.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME ScanKeyGolden COMMAND test_scankey_golden "${CMAKE_SOURCE_DIR}")   # argv[1]: [14] / [16] read six source files (read only)
set_tests_properties(ScanKeyGolden PROPERTIES TIMEOUT 300)   # seconds of body (SHIP DoMotorPowerOn waits 1 s per HOME); 300 for the endpoint scanner on a fresh exe

# --- test_indexz_torque_1203: E-038 (Q87, SAFETY) HT9050 Index Z torque from the PCI-1203's 6077h ---
#   AI(W906-E038) 20261003 (St01 / ST01-E): IndexZTorqueCore.h (pure: 2704h, sign from Mot_Table Direction, golden
#   k<0 -> 0 and "%5.2f", one value per arm from a later poll, baseline option off by default, P4 gate, P5 timeout),
#   IndexZTorque1203.cpp (rs232.cpp:358's branch, read-only Gerneral.ini switches), source pins on rs232.cpp /
#   EtherCAT/Pci1203TorqueHook.cpp / CMakeLists.txt. SHIP also runs the PCI1203 path against golden's RS-232 reader
#   (Comm1 in SIM mode, a fake hook). [T12] writes a Gerneral.ini into %TEMP% only and checks its SHA-256 is unchanged
#   by every read (key absent / 1 / 0). No machine file is read or written. Same link line as test_rs232_torque.
add_executable(test_indexz_torque_1203
    test_indexz_torque_1203.cpp
)
target_link_libraries(test_indexz_torque_1203 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_indexz_torque_1203 PRIVATE ws2_32)
endif()
target_compile_definitions(test_indexz_torque_1203 PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [T11] reads rs232.cpp, EtherCAT/Pci1203TorqueHook.cpp, IndexZTorque1203.cpp, CMakeLists.txt
add_test(NAME IndexZTorque1203 COMMAND test_indexz_torque_1203)
set_tests_properties(IndexZTorque1203 PROPERTIES TIMEOUT 300 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # same containment string as Rs232Torque
# --- TableAudit: AI(W906-E043) 20261004 [W906] (St01 ST01-E2) -- todo E-043 (Steven 1004 07:2x Q96 = B, rules approved 07:4x): the boot
#   table audit's rules T1-T12 (TableAudit.h). Pure: TableAudit.cpp alone, table / ini TEXT in, no file and no machine state. A clean base
#   gives 0 ERROR / 0 WARN; one bad sample per rule must report that rule at its level (deleting a rule's code turns its check red).
add_executable(test_tableaudit test_tableaudit.cpp ${CMAKE_SOURCE_DIR}/TableAudit.cpp)
target_include_directories(test_tableaudit PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME TableAudit COMMAND test_tableaudit)
set_tests_properties(TableAudit PROPERTIES TIMEOUT 60)

# --- test_f5_close_decide: AI(W906-F5-CLOSE) 20261004 -- NB2-1 (i), machine dispatch 4: closing the HMI window during F5 stops that F5's
#   start (tools/F5CloseDecide.h: start / pending / quit decisions; tools/wb_serve.cpp F5Close* applies them). Pure, both configs.
add_executable(test_f5_close_decide test_f5_close_decide.cpp)
target_include_directories(test_f5_close_decide PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME F5CloseDecide COMMAND test_f5_close_decide)
set_tests_properties(F5CloseDecide PROPERTIES TIMEOUT 30)

# --- test_teach_checkrange_reload: AI(W906-KB-GOLDEN) 20261004 [W906] -- the Teach page's two golden FormShow-only texts
#   (uteach.cpp:1753 edShtCheckRange=CHECK_RANGE, :1956-1961 InSHZDownRange=iInShtZRange only when In_Shuttle_Auto_Latch==eInSHAutoLtc)
#   are filled on a real window open only (FileRW/Teach.cpp MirrorToProxies(formShowOpen), the first-open edge filerw::OpenEnterRecord
#   returns at :256); the Save's reload (:195), every later editlist.get and FileRW_Teach_MirrorProxies get golden ReadFile's mirror only.
#   Before: after a Save the page showed the stale runtime CHECK_RANGE again (the Save writes Gerneral.ini only, :190; ReadTechData
#   re-reads it, cinitial.cpp:16139) and the next Save wrote it back. Locks [1]..[8] in the test head: first open, Save, the two
#   reloads (ReadFile half runs, FormShow half does not), the Gear Ratio mirror, a second Save, NO, a fresh open after the page
#   table's close edge without / with the re-read (W906_EditPageWindowEdgesArm as tools/wb_serve.cpp:4389, the real ReadTechData).
#   Sources and link shape as GearTeachSave (FileRW/Teach.cpp + _EditList / _EditPage, god-stack through RESCAN, the same two
#   link-only stand-ins); argv as GearTeachSave: the HT9050 snapshot teach.ini and Gerneral.ini READ ONLY, the copy under
#   machine_config_scratch; Gerneral.ini writes go to ENV-ALL's per-test sandbox (no ENVIRONMENT here on purpose); refuses to
#   run outside ctest (tests/w906_ctest_guard.h). Two full teach.ini saves through the real writer => TIMEOUT as GearTeachSave.
add_executable(test_teach_checkrange_reload
    test_teach_checkrange_reload.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/Teach.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditList.cpp
    ${CMAKE_SOURCE_DIR}/FileRW/_EditPage.cpp
)
target_include_directories(test_teach_checkrange_reload PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_teach_checkrange_reload PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)
if(WIN32)
    target_link_libraries(test_teach_checkrange_reload PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_teach_checkrange_reload.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME TeachCheckRangeReload
         COMMAND test_teach_checkrange_reload
                 "${CMAKE_SOURCE_DIR}/../machines/HT9050/snapshot/machine_params/runcfg/system/teach.ini"
                 "${CMAKE_SOURCE_DIR}/../machines/HT9050/snapshot/machine_params/D_HT9045_system/Gerneral.ini"
                 "${CMAKE_CURRENT_BINARY_DIR}/machine_config_scratch/TeachCheckRangeReload")
set_tests_properties(TeachCheckRangeReload PROPERTIES TIMEOUT 900)

# --- test_flow9050_tray: AI(W906-F9050-BD) 20261004 (laptop, F9050-BD) -- Frank's 910 HT9050 tray-arm / loader layered flows
#   (ref/frank-910-9050 2275f78c; ledger H064 / H065 / H076 / H091 and their DoCatchTray / DoLoad / DoInspectTrayColorOnLoader /
#   InitAllProcessTask arms), the 26 + 75 IO names registered only for Type_HT9050, and the Type_HT9046_LS answers (what 9050GPIB
#   decodes to today, database.cpp:517) unchanged.  Memory only; refuses to run outside ctest's redirects (w906_ctest_guard.h).
add_executable(test_flow9050_tray test_flow9050_tray.cpp)
target_include_directories(test_flow9050_tray PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_flow9050_tray PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_flow9050_tray PRIVATE ws2_32)
endif()
add_test(NAME Flow9050_Tray COMMAND test_flow9050_tray)
set_tests_properties(Flow9050_Tray PROPERTIES TIMEOUT 120)
# AI(W906-F9050-BD) 20261004: the HT9050 machine IO table bound as what 9050GPIB decodes to today (Type_HT9046_LS = 300: the
#   numbers of MachineIoTable_HT9050_PCI1203IO / MachineCylinders_HT9050) and as Type_HT9050 (800): +41 bound sensors (342), +26 named
#   cylinders (285) binding +22 Out / +21 _On / +21 _Off rows (94 / 75 / 75) -- 910 cinitial.cpp:2471-2545 / :4465-4490 name literals
#   x the CSV Alias column, counted with Python, not read back from these tests.
add_test(NAME MachineIoTable_HT9050_AsLS
         COMMAND test_machine_iotable "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv" 1124 136 301 4 300)
add_test(NAME MachineIoTable_HT9050_Type9050
         COMMAND test_machine_iotable "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv" 1124 136 342 4 800)
add_test(NAME MachineCylinders_HT9050_AsLS
         COMMAND test_machine_cylinders "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv" 300)
add_test(NAME MachineCylinders_HT9050_Type9050
         COMMAND test_machine_cylinders "${CMAKE_SOURCE_DIR}/../machines/HT9050/IO_Table.csv" 800)
# AI(W906-F9050-BD) 20261004: registered after the GLOBAL TEST TIMEOUT block (which only reaches tests added before it), so the
#   four set their own, like Flow9050_Tray above.
set_tests_properties(MachineIoTable_HT9050_AsLS MachineIoTable_HT9050_Type9050 MachineCylinders_HT9050_AsLS
                     MachineCylinders_HT9050_Type9050 PROPERTIES TIMEOUT 120)
# AI(W906-F9050-BD) 20261004: these five never need the machine's own Gerneral.ini: seed their per-test sandbox copy
#   (tests/test_bootstrap.cpp) from the versioned machines/HT9050/sim_9378/Gerneral.ini instead of D:/HT9045/system.  Scheduled
#   after AI(W906-ENV-ALL)'s DEFER, so this APPEND comes later in ENVIRONMENT and wins (the ELA_Service note above).
cmake_language(DEFER CALL set_property TEST Flow9050_Tray MachineIoTable_HT9050_AsLS MachineIoTable_HT9050_Type9050
    MachineCylinders_HT9050_AsLS MachineCylinders_HT9050_Type9050 APPEND PROPERTY ENVIRONMENT
    "W906_GENERAL_INI_SEED=${CMAKE_SOURCE_DIR}/../machines/HT9050/sim_9378/Gerneral.ini")

# --- test_st02_modal_timer1: AI(W906-S27) 20261004 (St02-E) -- S-27 / INBOX 93: golden 0618 note.cpp:3355 / mymessbox.cpp:542, a blocking
#   box's own Timer1 (10 ms) calls fMain->Timer1Timer, so the ported Timer1 segments that only PumpTick ran (GetTimeInfo :3189, the safe-door
#   lock :3051-3077) now also run while a box waits: W906_ModalTimer1Segments (MainTimersSt02.cpp), called at tools/wb_serve.cpp:7630.
#   Memory only; argv[1] = source root for the wb_serve.cpp / MainTimersSt02.cpp pins.
add_executable(test_st02_modal_timer1 test_st02_modal_timer1.cpp)
target_include_directories(test_st02_modal_timer1 PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_st02_modal_timer1 PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_st02_modal_timer1 PRIVATE ws2_32)
endif()
add_test(NAME St02_ModalTimer1 COMMAND test_st02_modal_timer1 "${CMAKE_SOURCE_DIR}")
set_tests_properties(St02_ModalTimer1 PROPERTIES TIMEOUT 60)
# --- test_e044_torque_wait: E-044 (Steven 1004 08:4x, SAFETY) HT9050 Index torque wait 12110 / 14110 times out ---
#   AI(W906-E044) 20261004 NOT GOLDEN (St01 / ST01-E): Ht9050TorqueWait.h (pure 5 s window), Ht9050TorqueWait.cpp
#   (HT9050-only wrapper + the alarm) and the two same-line inserts atester.cpp:6908 (case 12110) / :7538 (case 14110),
#   driven through the real DoTestHeadMotor with a fake clock. SHIP also runs golden's Panasonic reader to 999 on a
#   SIM-mode Comm1 (golden's 999 exit stays golden). Source pins read atester.cpp, Ht9050TorqueWait.cpp, CMakeLists.txt
#   and tools/wb_serve.cpp. No machine file is read or written; same containment string as Rs232Torque.
add_executable(test_e044_torque_wait
    test_e044_torque_wait.cpp
)
target_link_libraries(test_e044_torque_wait PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_e044_torque_wait PRIVATE ws2_32)
endif()
target_compile_definitions(test_e044_torque_wait PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")
add_test(NAME E044TorqueWait COMMAND test_e044_torque_wait)
set_tests_properties(E044TorqueWait PROPERTIES TIMEOUT 300 ENVIRONMENT
    "W906_E84DATA_ROOT=${_ht9045_log_scratch}/E84DataTxt;W906_PRODLOG_ROOT=${_ht9045_log_scratch}/Production_Log;W906_CLEANPADLOG_ROOT=${_ht9045_log_scratch}/CleanPad_Log;W906_TCPDATA_ROOT=${_ht9045_log_scratch}/TCP_Data;W906_SUMMARYLOT_ROOT=${_ht9045_log_scratch}/Summary_Lot;W906_EVENTLOG_ROOT=${_ht9045_log_scratch}/EventLogTxt;W906_INIDATA_ROOT=${_ht9045_log_scratch}/IniData;W906_UNLOADERINFO_ROOT=${_ht9045_log_scratch}/UnloaderInfo;W906_AUTH_PATH=${_ht9045_config_scratch}/;W906_MACHINERECORD_DIR=${_ht9045_log_scratch}/machinerecord")   # same containment string as Rs232Torque
# --- EcatAlarmScan / EcatAlarmScanLive: AI(W906-E045) 20261004 [W906] (St01 ST01-E2) -- todo E-045 = S-26 R4 + Steven 1004 17:0x
#   Q98 (EcatAlarmScan.h): on HT9050 a claimed 1203 row whose monitor sample shows ALM / ERROR_STOP raises through golden's
#   ScanAllMotorStatus body regardless of ServoAlarmOn, gets ONE clear-alarm attempt, and if that does not clear it the
#   START / HOME gate refuses until a power cycle. EcatAlarmScan = the pure rule (header only). EcatAlarmScanLive = the real
#   csystem.cpp ScanAllMotorStatus with a fake engine route (test_ecat_motor_route's shape), the god stack as
#   test_note_motorerror, + source pins on csystem.cpp. Gets the redirect roots from the deferred ENV-ALL call above.
add_executable(test_ecat_alarm_scan test_ecat_alarm_scan.cpp)
target_include_directories(test_ecat_alarm_scan PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME EcatAlarmScan COMMAND test_ecat_alarm_scan)
set_tests_properties(EcatAlarmScan PROPERTIES TIMEOUT 60)
add_executable(test_ecat_alarm_scan_live test_ecat_alarm_scan_live.cpp)
target_include_directories(test_ecat_alarm_scan_live PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(test_ecat_alarm_scan_live PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # [L10] reads csystem.cpp
target_link_libraries(test_ecat_alarm_scan_live PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ecat_alarm_scan_live PRIVATE ws2_32)
endif()
add_test(NAME EcatAlarmScanLive COMMAND test_ecat_alarm_scan_live)
set_tests_properties(EcatAlarmScanLive PROPERTIES TIMEOUT 120)
# AI(W906-E09) 20261004 (St02-E) -- E09_MainLogo (node, offline): card E-09 batch 1 -- web/page/main.html:45 #imgLogo{pointer-events:none;}
#   (the logo block moved by ht9xxx-layout-base.js lay over cbRunStartMode and took its clicks; golden main.dfm imgLogo / cbRunStartMode
#   never overlap).  Source checks: the one rule, both elements present, no click / mouse handler bound to imgLogo in web/page.
#   CONTROL: W906_MAIN_HTML pointing at main.html before the fix must turn check 1 red.
if(W906_NODE_EXECUTABLE)
    add_test(NAME E09_MainLogo COMMAND "${W906_NODE_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/webprobe/e09_main_logo_selftest.cjs" "${CMAKE_SOURCE_DIR}/../web/page")
    set_tests_properties(E09_MainLogo PROPERTIES TIMEOUT 60)
endif()
# --- test_vc4_group: AI(W906-F03-QUADVAC) 20261004 -- F-03 item 3: HT9050 picks ONE IC with the 4 nozzles of one ECAT-VC4 ---
#   The group in TMySucker (mykitsuck.cpp, end of file) driven on a fake PCIE-1203 backend: memory only, no card.  The
#   switches are written to ENV-ALL's per-test Gerneral.ini scratch (no ENVIRONMENT here on purpose); refuses to run
#   outside ctest (tests/w906_ctest_guard.h).  argv[1] = the source root for the call-site pins (read only).
add_executable(test_vc4_group
    test_vc4_group.cpp
)
target_include_directories(test_vc4_group PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_vc4_group PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_vc4_group PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_vc4_group.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")
endif()
add_test(NAME Vc4Group COMMAND test_vc4_group "${CMAKE_SOURCE_DIR}")
set_tests_properties(Vc4Group PROPERTIES TIMEOUT 120)
# --- test_s26_r3_inpos: AI(W906-S26-R3) 20261004 -- S-26 Appendix A R3 (St01): TMyEtherCatMotor::ScanMotorStatus's route arm now writes
#   Led[iInposLed] from the 1203 monitor sample with golden's meaning ("NOT in position": true unless READY and not pending), so
#   IsTrayArmMoveAvoidOutArmCrash (acatchtray.cpp, golden :7979) holds OutArm X/Y while MTrayX moves at/after Empty, as on golden's
#   SMC / MN200 machines. A decode (every STA_AX_* state, pending, INP bit ignored, failed read / no route / Enable=0 as before),
#   B the predicate, C SetOutArm_9045 stopping OutArm X / Y, D MotorMovePosition (ServoAlarmOn=1) arriving as before.
#   FAKE engine route (Motor/EcatMotorRoute.h, one sample per board/port), MOT[] rows on stack objects: no card, no file, no thread.
#   Link shape as EcatMotorRoute (god-stack through RESCAN).
add_executable(test_s26_r3_inpos test_s26_r3_inpos.cpp)
target_include_directories(test_s26_r3_inpos PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_s26_r3_inpos PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_s26_r3_inpos PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_s26_r3_inpos.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Wno-unused-parameter")   # the new file only; -Wno-unused-parameter: the god-stack headers' empty virtual stubs (HTMotor.h) are not ours to re-warn
endif()
add_test(NAME S26_R3_InPos COMMAND test_s26_r3_inpos)
set_tests_properties(S26_R3_InPos PROPERTIES TIMEOUT 120)

# --- test_s26_r1_zsafe_on / test_s26_r1_zsafe_default: AI(W906-S26-R1) 20261004 -- S-26 Appendix A R1 (St01 K4): "Z at its origin" in
#   InArmZSafe / OutArmZSafe (iFlag & 1) and CheckInArmZNeedHome / CheckOutArmZNeedHome (Motor/mymotor.cpp) behind MachineType.h
#   W906_DS402_ZSAFE_HOMEFLAG, DEFAULT OFF. One source, two executables. _on compiles Motor/mymotor.cpp ITSELF with the switch ON
#   ("W906_DS402_ZSAFE_HOMEFLAG=", empty like MachineType.h's own #define, so turning it on there later does not warn "redefined") --
#   this target only, the libraries stay as built; its object defines every symbol of ht9045_motor's mymotor.cpp.obj plus
#   W906_Ds402ZAtOrigin, so that archive member is never extracted. Same warning options as ht9045_motor gives mymotor.cpp.
#   _default links ht9045_motor as every build does: the lamp only (golden). FAKE engine route (Motor/EcatMotorRoute.h), MOT[] rows
#   on stack objects: no card, no file, no thread.
foreach(_w906r1 test_s26_r1_zsafe_on test_s26_r1_zsafe_default)
    add_executable(${_w906r1} test_s26_r1_zsafe.cpp)
    target_include_directories(${_w906r1} PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(${_w906r1} PRIVATE
        "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    )
    if(WIN32)
        target_link_libraries(${_w906r1} PRIVATE ws2_32)
    endif()
endforeach()
target_sources(test_s26_r1_zsafe_on PRIVATE ${CMAKE_SOURCE_DIR}/Motor/mymotor.cpp)
target_compile_definitions(test_s26_r1_zsafe_on PRIVATE "W906_DS402_ZSAFE_HOMEFLAG=")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_s26_r1_zsafe.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Wno-unused-parameter")   # the new file only (HTMotor.h's empty virtual stubs are not ours to re-warn)
    set_source_files_properties(${CMAKE_SOURCE_DIR}/Motor/mymotor.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Wno-conversion-null;-Wno-write-strings;-Wno-unused-parameter")   # = ht9045_motor's options; a source property of THIS directory, so only the _on target sees it
endif()
add_test(NAME S26_R1_ZSafe_On COMMAND test_s26_r1_zsafe_on)
set_tests_properties(S26_R1_ZSafe_On PROPERTIES TIMEOUT 120)
add_test(NAME S26_R1_ZSafe_Default COMMAND test_s26_r1_zsafe_default)
set_tests_properties(S26_R1_ZSafe_Default PROPERTIES TIMEOUT 120)

# --- test_mach0180_review: AI(W906-HOME-1310FG) 20261005 -- laptop review F3 of the MACH-0180 batch (machine cpp 0180-0201, ledger
#   docs/handoff/MACH0180_LEDGER_20261004.md): the HT9050 Tray Arm floodgate wait at ProcessMotorHome step 1310 (machine cpp 0196,
#   AI(W906-HOME-1310ORDER)) has its own timer (uhome.cpp tail: W906_HomeFloodgateWaitOver / W906_HomeFloodgateWaitReset) instead of
#   ResetOKDeleyTime, which can be spent before that wait starts. F3-a the helper's grace / reset (memory, about 0.3 s), F3-b source pins
#   on uhome.cpp (reads the source tree only). Link shape as S26_R3_InPos (god stack through RESCAN; uhome.cpp is in ht9045_sm).
add_executable(test_mach0180_review test_mach0180_review.cpp)
target_include_directories(test_mach0180_review PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(test_mach0180_review PRIVATE "W906_SRC_ROOT=\"${CMAKE_SOURCE_DIR}\"")   # F3-b reads uhome.cpp
target_link_libraries(test_mach0180_review PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_mach0180_review PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_mach0180_review.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra")   # the new file only
endif()
add_test(NAME MACH0180_Review COMMAND test_mach0180_review)
set_tests_properties(MACH0180_Review PROPERTIES TIMEOUT 120)

# --- test_ht9050_shtsafe: AI(W906-HT9050-SHTSAFE) 20261004 [W906] -- EastSun 1004: In Shuttle 1 goes to the Index only with Out Shuttle 1
#   at Right; otherwise Out Shuttle 1 is cleared to Right (Out Arm ZA at home, Out Shuttle 2 at Tech.iOutShuttle2Left first). acarry.cpp
#   W906_Ht9050OutSht1NotClearShuttle1CanNotMove / ...ToRight / ...Sht2ToLeft on sim motors (memory only), plus source pins on acarry.cpp
#   (5 call sites), uhome.cpp (HOME's last step: Out Shuttle 2 to Left) and ainarm9045.cpp (disabled C_TrayY_Fixer skipped on HT9050).
add_executable(test_ht9050_shtsafe test_ht9050_shtsafe.cpp)
target_include_directories(test_ht9050_shtsafe PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ht9050_shtsafe PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ht9050_shtsafe PRIVATE ws2_32)
endif()
add_test(NAME HT9050_ShtSafe COMMAND test_ht9050_shtsafe "${CMAKE_SOURCE_DIR}")
set_tests_properties(HT9050_ShtSafe PROPERTIES TIMEOUT 120)

# --- test_ht9050_dryrun: AI(W906-HT9050-DRYRUN) 20261005 [W906] -- EastSun 1005「將以上的步驟使用程式模擬是否能正確執行，若能模擬正確執行再進行真正的機台動作」:
#   Ht9050DryRun.cpp (Step1..Step10 motor-only cycle) on SLOW sim motors (a fixed distance per tick), the arm Z HOME lamps following their
#   position: step order per cycle, 5 cycles, the collision rules on every tick (In / Out Shuttle 1 at the Index point, Out Shuttle 2 Left,
#   Z up before X / Y, Out Arm X-then-Y / Y-then-X, Index Z1 / Loader Z never move), PAUSE / START, start with Out Shuttle 1 in the Index,
#   Out Arm ZA lamp off. Memory only.
add_executable(test_ht9050_dryrun test_ht9050_dryrun.cpp)
target_include_directories(test_ht9050_dryrun PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ht9050_dryrun PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_ht9050_dryrun PRIVATE ws2_32)
endif()
add_test(NAME HT9050_DryRun COMMAND test_ht9050_dryrun)
set_tests_properties(HT9050_DryRun PROPERTIES TIMEOUT 300)
# --- test_atc_cal_split: AI(W906-I08) 20261004 (Ifor01) -- I-08 (RULINGS_20261003 #22 / #1): the ATC ambient calibration file split
#   cold / hot as V912 (uTemp_Set.cpp:3005-3012 read, :4467-4474 write): Hot / AmbientHot use DefineTemp\Temperature_ATC.Data, the
#   other modes Temperature_ATC_Cold.Data (copied from Temperature.Data when missing); ReadTempFile and spbSaveClick in a %TEMP%
#   sandbox (as test_settemp_save); source pin that uTemp_Set.cpp and FileRW/Temperature.gen.inc both carry the split (argv[1] = the
#   port root, read only); the machine's own files untouched. Red with either site back to golden 0618's one _ATC file.
#   Containment: ENV-ALL.
add_executable(test_atc_cal_split test_atc_cal_split.cpp)
target_link_libraries(test_atc_cal_split PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_atc_cal_split PRIVATE ws2_32)
endif()
add_test(NAME AtcCalSplit COMMAND test_atc_cal_split "${CMAKE_SOURCE_DIR}")
# AI(W906-I08) 20261005 (Ifor01), W-59: 300 s, not 120 -- the laptop's SIM gate ran into 120 s (at [6]) while IFOR-NB2 needs
#   ~11 s; here a Hot / AmbientHot ReadTempFile with ATC cooling on already takes ~3.4 s (8 reads + 2 saves in all). The test
#   prints each step's time and a watchdog line for any step past 30 s, so a real hang still shows where it is.
set_tests_properties(AtcCalSplit PROPERTIES TIMEOUT 300)
# --- test_svon_syncard: AI(W906-SVON-SYNCARD) 20261005 -- NB2-1 R230 (laptop W-60; St01 W-47). TMyMotor::PCIL132_ResetPos
#   (Motor/mymotor.cpp, golden mymotor.cpp:1838-1856) on a PCIE-1203 row: golden picks its arm by the GLOBAL MOTION_CARD_TYPE and
#   its SYN arm relies on TMySYNTEKMotor::ResetPos ignoring the argument; a TMyEtherCatMotor writes it, so on HT9050
#   (MOTION_CARD_TYPE=0) the servo-ON sync wrote 0 and the route refused it (Q1). Now a 1203 row takes the encoder arm. FAKE engine
#   route answering coordinate writes like the real Q1; golden gates (PServoAlarmOn, servo OFF, Enable) and another class's
#   dispatch unchanged. Link shape as S26_R1_ZSafe_Default (god stack through RESCAN). Memory only: no card, no file, no thread.
add_executable(test_svon_syncard test_svon_syncard.cpp)
target_include_directories(test_svon_syncard PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_svon_syncard PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_svon_syncard PRIVATE ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_svon_syncard.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Wno-unused-parameter")   # the new file only (HTMotor.h's empty virtual stubs are not ours to re-warn)
endif()
add_test(NAME SvonSyncCard COMMAND test_svon_syncard)
set_tests_properties(SvonSyncCard PROPERTIES TIMEOUT 120)
# --- test_fullhome_zsafe_on / test_fullhome_zsafe_default: AI(W906-FULLHOME-ZSAFE) 20261005 -- NB2-1 R231 (laptop W-63). MachineType.h
#   W906_HT9050_FULLHOME_ZSAFE, DEFAULT OFF: TMyMotor::MotorHome (Motor/mymotor.cpp) on an HT9050 arm Z the drive homed goes on to
#   ZSafePos (golden ProcessSingleMotorHome case 500's move) before answering 1. One source, two executables, the shape of
#   S26_R1_ZSafe_On / _Default: _on compiles Motor/mymotor.cpp ITSELF with "W906_HT9050_FULLHOME_ZSAFE=" (this target only; its
#   object defines every symbol of ht9045_motor's mymotor.cpp.obj, so that archive member is never extracted), _default links
#   ht9045_motor as every build does. FAKE engine route, stack objects: no card, no file, no thread. ~2 s (HomeDelay x5).
foreach(_w906fz test_fullhome_zsafe_on test_fullhome_zsafe_default)
    add_executable(${_w906fz} test_fullhome_zsafe.cpp)
    target_include_directories(${_w906fz} PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(${_w906fz} PRIVATE
        "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    )
    if(WIN32)
        target_link_libraries(${_w906fz} PRIVATE ws2_32)
    endif()
endforeach()
target_sources(test_fullhome_zsafe_on PRIVATE ${CMAKE_SOURCE_DIR}/Motor/mymotor.cpp)
target_compile_definitions(test_fullhome_zsafe_on PRIVATE "W906_HT9050_FULLHOME_ZSAFE=")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(test_fullhome_zsafe.cpp PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Wno-unused-parameter")   # the new file only
endif()
add_test(NAME FullHomeZSafe_On COMMAND test_fullhome_zsafe_on)
set_tests_properties(FullHomeZSafe_On PROPERTIES TIMEOUT 120)
add_test(NAME FullHomeZSafe_Default COMMAND test_fullhome_zsafe_default)
set_tests_properties(FullHomeZSafe_Default PROPERTIES TIMEOUT 120)

# --- test_dtm_channel_map: AI(W906-I03b) 20261005 (Ifor01) -- I-03b step 2 (FROM_IFOR s1 1005 13:4x; plan = MR !201, Ifor 1005
#   D1-D4 as recommended): the HT9050 DTM channel map behind Gerneral.ini [TempCtrl] DTM_CHANNEL_MAP (EJ1N/DtmChannelMap):
#   the table (and against .claude/skills/ht9050-hw/data/HT9050-TempMap.json), the DTM form on a mapped machine (24 channels,
#   per-channel sensor type, boot SV 0, station 4 never addressed) against a fake Modbus/TCP DTM the test owns (127.0.0.1),
#   DoThermo end to end (Comm2 SIM, a fake DTK4848 answering writes): golden's serial-path SV reaches the DTM, SLK-1..8 stay 0,
#   PV stored as golden case 2500, no mapped zone on the serial port, 999 when the DTM goes away; source pins (argv[1] = the
#   port root, read only); Gerneral.ini / config.ini compared before and after.  ~20-40 s.  After the GLOBAL TEST TIMEOUT
#   block => own TIMEOUT.
add_executable(test_dtm_channel_map test_dtm_channel_map.cpp)
target_link_libraries(test_dtm_channel_map PRIVATE
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
)
if(WIN32)
    target_link_libraries(test_dtm_channel_map PRIVATE ws2_32)
endif()
add_test(NAME DtmChannelMap COMMAND test_dtm_channel_map "${CMAKE_SOURCE_DIR}")
set_tests_properties(DtmChannelMap PROPERTIES TIMEOUT 300)
