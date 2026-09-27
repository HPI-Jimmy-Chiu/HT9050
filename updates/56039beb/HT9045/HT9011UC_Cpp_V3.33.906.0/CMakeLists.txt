cmake_minimum_required(VERSION 3.10)
project(HT9045_Cpp C CXX)   # C needed for Public/cJSON.c
set(HT9045_CXX_STANDARD 17 CACHE STRING "C++ standard. 17 = oracle/default. 14 ONLY for the WinLibs g++ 16 lanes (std::byte vs w32api byte clash, cmydef.h:127)")  # AI(W906-IOWEB-P1) 20260924: ported from the field tree (AI(W906-MachineImport) 20260825); build_nonoracle.bat/build_x64.bat already pass -DHT9045_CXX_STANDARD=14, which this file silently ignored. Default 17 => build.bat (6.3.0) unchanged. On the old blank line so no line below moves.
set(CMAKE_CXX_STANDARD ${HT9045_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# AI(W906-W7-L1-Wave0) 20260801: pin the Win32 API floor for BOTH oracles, so the
# two of them stop seeing DIFFERENT API surfaces.  This lands decision DA2 (of
# decision set D2) from docs/W7_UI_ARCHITECTURE_PLAN.md SS2, which was already
# taken and recorded there but explicitly left un-landed with "owner = main loop";
# the reasoning below is that section's, not re-derived here.
#
#   * The PROBLEM being fixed is oracle divergence, not a missing feature.  With
#     nothing pinned, MinGW defaults to _WIN32_WINNT=0x0500 (Windows 2000) while
#     MSVC auto-selects the newest (it prints "_WIN32_WINNT not defined.
#     Defaulting to _WIN32_WINNT_MAXVER").  An API declared on only one of those
#     two surfaces therefore slips past BOTH oracles.  Pinning one value is what
#     makes "declared on one side only" a build error somewhere.
#   * WHY 0x0601 (Windows 7) AND NOT 0x0A00.  The primary oracle is MinGW.org
#     MinGW32 gcc 6.3.0, whose C:\MinGW\include\sdkddkver.h tops out at
#     _WIN32_WINNT_WINBLUE (0x0603) and has no _WIN32_WINNT_WIN10 at all.  Worse
#     than failing, the bare number 0x0A00 compiles SILENTLY there while being
#     completely inert (that w32api carries no Win10-only declarations), which
#     would manufacture a false "we are pinned to Win10" impression.  0x0601 is
#     the highest value that means anything on this oracle.
#   * A COMPILE-TIME FLOOR IS NOT A RUNTIME TARGET.  The shipped runtime target
#     stays Windows 10/11; this only fixes which declarations the headers expose.
#   * WINVER is set to the same value deliberately -- the two are independent
#     macros in the Windows headers and leaving WINVER unpinned would reintroduce
#     exactly the inconsistency this exists to remove.
# VERIFIED BY REBUILDING AFTER ADDING IT, and stated precisely rather than as
# "warning-clean" (this build is NOT warning-free and never was): a full MinGW
# rebuild produced 277 warning lines before the pin and 284 after, and the entire
# +7 delta is -Wmissing-field-initializers from the 7 LAST_GENERAL_SET fields this
# same pass added to canary_support.h under its pre-existing `LastSet = {0}`
# aggregate initialiser.  The set of NON-missing-field-initializer warning texts
# is byte-identical before and after, i.e. the pin itself introduced ZERO new
# warnings.  common.cpp:2313's ERROR_SHARING_VIOLATION / ERROR_LOCK_VIOLATION also
# still resolve (they are plain winerror.h codes, not version-gated).
# =============================================================================
add_compile_definitions(_WIN32_WINNT=0x0601 WINVER=0x0601)

# =============================================================================
# AI(W906-GATE-SIMCFG) 20260918: 選組態。**預設 OFF = 沿用 MachineType.h:48**，
# 也就是使用者 20260918 打開的 SOFT_SIMULTE 模擬組態，F5 行為零變化。
#
#   cmake -DW906_NO_SOFT_SIMULTE=ON ..   -> 出貨組態（SOFT_SIMULTE 關）
#
# 為什麼要有這個開關：ctest 那套 fidelity harness 是對「旗標關閉」推導的，
# 旗標開著跑會有 14 支必然失敗（全部是 #ifdef SOFT_SIMULTE 臂的行為差異，
# 非缺陷；逐條追溯見 MachineType.h 的註解）。若改成把那 14 支跳過，
# gate 就少掉 14 支的鑑別力——那是 docs 裡「不可能當掉的 gate 不是 gate」那條。
# 所以 gate 改建出貨組態（tools/dualgate.sh 會帶這個旗標），
# 模擬組態則由 tools/simbuild.sh 做「建得起來」的把關。
# =============================================================================
option(W906_NO_SOFT_SIMULTE
       "Build the SHIPPING configuration (SOFT_SIMULTE undefined). Default OFF = simulation, matching MachineType.h:48."
       OFF)
if(W906_NO_SOFT_SIMULTE)
    add_compile_definitions(W906_NO_SOFT_SIMULTE)
    message(STATUS "W906 config: SHIPPING (SOFT_SIMULTE undefined)")
else()
    message(STATUS "W906 config: SIMULATION (SOFT_SIMULTE defined, per MachineType.h:48)")
endif()

# =============================================================================
# AI(W906-PT-W3-integrate) 20260808: MN200DLL_EXPORTS -- a LINKAGE switch, not a
# behaviour switch, and it must be GLOBAL for the same reason _WIN32_WINNT above
# is: every TU that sees Motor/vendor/mn200.h has to agree on the decorated name.
#
# WHY IT IS NEEDED: mn200.h:1-5 makes MOTIONNETDLL_API `__declspec(dllimport)`
# unless MN200DLL_EXPORTS is defined, so every mn_* call site emits an
# `_imp__mn_xxx@N` thunk reference that only a real vendor import library can
# satisfy -- and there is none for MinGW (the shipped MN200DLL.lib is 32-bit OMF).
# Defining MN200DLL_EXPORTS makes it `__declspec(dllexport)` instead: callers then
# emit a plain `_mn_xxx@N` reference, which Motor/vendor_offline_motionnet.cpp
# satisfies statically.  It touches linkage only; no declaration changes shape.
#
# NOT MN200DLL_VB, which was tried first and is WRONG: mn200.h mirrors its API
# three times with DIFFERENT signatures (mn_fix_move takes `SPEED_PAR*` at :307,
# `double*,BYTE*` at :426, and `SPEED_PAR` BY VALUE at :545), and golden targets
# the :545 MOTIONNETDLL_API block.  MN200DLL_VB switches the active block and so
# breaks 4 signatures -- 10 compile errors in myMN200motor.cpp.  See that file's
# banner in Motor/vendor_offline_motionnet.cpp for the full account.
#
# Scoped globally rather than per-target ON PURPOSE: if one target saw dllexport
# and another dllimport, the same function would be referenced under two
# different symbol names and the link would break for a reason invisible at the
# call site.  mn200.h is the ONLY file tree-wide that reads either macro
# (grepped), so there is no other blast radius.
# =============================================================================
add_compile_definitions(MN200DLL_EXPORTS)

# =============================================================================
# AI(W906-PT-W5a-integrate) 20260809: DLLDIR_EX -- the SAME LEVER SHAPE as
# MN200DLL_EXPORTS above, and global for the same reason: every TU must agree on the
# decorated name.
#
# Motor/Hontech_M4.h:61-64 makes DLLDIR `__declspec(dllimport)` unless DLLDIR_EX is
# defined, so a caller that includes it emits `_imp___Hon_m4_xxx@N` thunk references.
# But Motor/Hontech_M4.cpp -- a TRANSLATED unit already in this archive -- DEFINES the
# plain `__Hon_m4_xxx@N` symbols (:157 onwards).  The two never matched.
#
# NOTHING BROKE UNTIL NOW, and the reason is the archive-extraction trap again: no TU
# referenced Motor/mySYNTEKmotor.cpp, so its object was never pulled and its 26
# `_imp___Hon_m4_*` references were never looked at.  cinitial.cpp's newly translated
# InitialMotorParameter() has `new TMySYNTEKMotor(iAdder)` in golden's driver ladder,
# which extracts the member and turns those 26 into hard errors.
#
# Probed before adopting, not assumed:
#   without DLLDIR_EX  -> caller emits _imp___Hon_m4_initial@8   (nothing defines it)
#   with    DLLDIR_EX  -> caller emits   __Hon_m4_initial@8
#   Hontech_M4.cpp.obj defines            __Hon_m4_initial@8     <-- match
# So this does not add a stub; it CONNECTS ~2,000 already-translated lines that had
# been unreachable.  Motor/Hontech_M4.h is the only file in this tree that reads the
# macro (grepped), so there is no other blast radius.
# =============================================================================
add_compile_definitions(DLLDIR_EX)


# ---------------------------------------------------------------------------
#  GNU/MinGW: static-link the compiler runtimes
# ---------------------------------------------------------------------------
# AI(W906-W7-A1fix) 20260729: link libgcc/libstdc++ statically so every
# executable is self-contained.
#
# WHY, and it is not cosmetic: without this every test .exe imports
# libgcc_s_dw2-1.dll and libstdc++-6.dll (confirmed with objdump -p) and
# resolves them from PATH at LOAD time. Launch one from any environment whose
# PATH lacks C:\MinGW\bin -- a plain PowerShell, a vcvarsall shell that
# rewrote PATH, a scheduler, another tool's subprocess -- and the loader fails
# with STATUS_DLL_INIT_FAILED (0xc0000142), which Windows reports as a MODAL
# "application was unable to start correctly" dialog. That happened to
# test_testertcp_socket.exe on 2026-07-29 and interrupted the user.
#
# This failure mode is INVISIBLE to tests/test_bootstrap.cpp's dialog guards:
# a loader failure occurs before ANY user code -- including static
# initializers -- has run, so SetErrorMode has not been called yet and cannot
# suppress it. The earlier claim that the bootstrap covers batch runs was
# therefore overstated; it covers crashes and asserts DURING execution, not
# failures to start. Removing the external dependency is the only fix that
# addresses the cause rather than the symptom, and it makes the exes runnable
# from any working directory or shell.
#
# Cost: a few hundred KB per executable, and no behaviour change -- the same
# libstdc++/libgcc code, bound at link time instead of load time.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_link_options(-static-libgcc -static-libstdc++)
endif()

# ---------------------------------------------------------------------------
# Library: vclcompat  (W0 keystone)
#   BCB6 / VCL compatibility layer: recreate ONLY the API surface the V906
#   source actually uses (AnsiString / TStringList / TDateTime / SysUtils),
#   with semantics that mirror BCB6 exactly (1-based AnsiString indexing, etc.).
#   Translated files #include "vclcompat/vcl_compat.h" instead of <vcl.h>.
# ---------------------------------------------------------------------------
add_library(vclcompat
    vclcompat/AnsiString.cpp
    vclcompat/TStringList.cpp
    vclcompat/TDateTime.cpp
    vclcompat/SysUtils.cpp
    vclcompat/IniFiles.cpp
    # --- HAL enabler: SPComm TComm serial shim (Spcomm::TComm) --------------
    #   Faithful stand-in for the SPComm VCL serial component
    #   (D:/HT9045/elec/Component/spcomm.hpp:154-277) used by the temp /
    #   barcode / EJ1N / RS232-tester code.  REAL mode drives \\.\COMx via
    #   Win32 CreateFile/SetCommState/ReadFile/WriteFile + a Win32 reader
    #   thread; SIM mode (auto fallback / SetSimMode) needs no hardware so the
    #   compat layer compiles/links/runs offline.  Lives in the vclcompat lib
    #   because it derives from vclcompat::TComponent and is pulled in via the
    #   vcl_compat.h umbrella.  On Win32 the .cpp uses the kernel32 serial API
    #   (already in the default MinGW link set -- no extra target_link needed).
    vclcompat/Comm.cpp
    # --- W5 comms enabler: Scktcomp TClientSocket/TCustomWinSocket shim -------
    #   Faithful stand-in for the VCL Scktcomp::TClientSocket (Winsock2 TCP
    #   client) used by MyPLC/ModbusTCPClient.cpp.  Default is SIM mode (no
    #   real connect attempted); Real mode is an explicit composition-root
    #   opt-in (SetSimMode(false)) -- see the file header for why this does
    #   NOT auto-fall-back to Sim on a failed Real connect (Enable_PLCSafety_IO
    #   is a safety-interlock read; masking a real connect failure would be
    #   unsafe).  Lives here (not in MyPLC/ or a new library) because it is
    #   general vclcompat infra, matching Comm.cpp's precedent.
    vclcompat/ClientSocket.cpp
    # --- W906-ServerSocket enabler: Scktcomp TServerSocket/TServerWinSocket
    #   shim (multi-connection server sibling of ClientSocket.cpp above).
    #   Faithful stand-in for the VCL ScktComp::TServerSocket used by
    #   SECSGEM/uHGemEquipment.cpp's srvGem (passive-HSMS role) and
    #   Automation/automation.cpp's OLPServer -- see ServerSocket.h's own
    #   file-head for both golden call-shape citations.  No production
    #   consumer wired yet (prerequisite infra, same shape as the earlier
    #   SecsWireCodec/SecsSvEcRegistration slices); reuses ClientSocket.h's
    #   TCustomWinSocket for its per-connection objects (see that header's
    #   own EXTENSION note), so it must link/compile alongside ClientSocket.cpp
    #   in the same library, not standalone.  Default is Sim mode; Real mode
    #   is an explicit SetSimMode(false) opt-in (WinSock2 listen/accept +
    #   a background acceptor thread + per-connection reader threads).
    vclcompat/ServerSocket.cpp
    # --- W906-uHGemEquipment enabler: headless TStringGrid shim (Grids.hpp) --
    #   Faithful stand-in for the real VCL TStringGrid used by SECSGEM/
    #   uHGemEquipment.h's CEID/Report/Alarm "StringGrid database" family
    #   (strGrdCEID/stdGridReportID/strGrdAlarm/strGrdAlarmOld -- see that
    #   header's own file-head note). Headless: Cells[ACol][ARow] (0-based,
    #   unlike AnsiString's 1-based indexing) + growable RowCount/ColCount
    #   property proxies; no rendering/FixedRows/Options. Zero dependency
    #   beyond vclcompat/AnsiString.h + <vector>. NOT brought into the global
    #   namespace by vcl_compat.h (consumers add their own `using
    #   vclcompat::TStringGrid;` -- see that header's own note on why).
    vclcompat/StringGrid.cpp
    # --- W906-uHGemEquipment-BucketC enabler: headless TMemoryStream shim
    #   (Classes.hpp). Faithful stand-in for the real VCL TMemoryStream used
    #   by SECSGEM/uHGemEquipment.cpp's RecvMemoryBuffer/ProcBuffer/
    #   TempProcBuffer (clientGemRead/ProcessSocketReceiveData) -- see
    #   vclcompat/MemoryStream.h's own file-head note for the exact API
    #   surface + semantics. Zero dependency beyond <vector>/<cstring>.
    #   vclcompat/SyncObjs.h (TCriticalSection) is header-only (inline) --
    #   no matching .cpp, no separate CMake source entry needed for it.
    vclcompat/MemoryStream.cpp
    # AI(W906-SpoolCluster) 20260721: headless TFileListBox shim (FileCtrl.hpp) -
    #   Faithful stand-in for the real VCL TFileListBox used by SECSGEM/
    #   uHGemEquipment.h's THGem::FileListBox1 (Spool cluster: WriteToSpoolFile/
    #   DoSpool/SetSpoolActive/GetSpoolActive) -- see vclcompat/FileListBox.h's
    #   own file-head note for the exact scan-logic/sort-order/Refresh()==
    #   Update() rationale. Zero dependency beyond vclcompat/AnsiString.h +
    #   vclcompat/TStringList.h + vclcompat/SysUtils.h's FindFirst/FindNext/
    #   FindClose/TSearchRec family (already real). NOT brought into the
    #   global namespace by vcl_compat.h (consumers add their own `using
    #   vclcompat::TFileListBox;` -- same established precedent as
    #   vclcompat::TStringGrid above).
    vclcompat/FileListBox.cpp
    # AI(W906-PT-W3-integrate) 20260808: TrayCore JOINS THE LIBRARY -- and this
    #   reverses a decision recorded further down this file (see the
    #   HT9045_UI_CTRL_SOURCES note, "the vclcompat/*Core.cpp files are listed
    #   here rather than linked from a library ON PURPOSE ... adding them to that
    #   library would change the MinGW oracle's link graph from inside a UI-only
    #   wave").  That reasoning was about SCOPE, not principle, and the case it
    #   was waiting for has now arrived: OmronLaser/LaserSensor.h declares
    #   mtPlate1/mtPlate2/mtPlate1_Golden/mtPlate2_Golden as
    #   `vclcompat::TrayCore*` (the tree's registered stand-in for golden's
    #   TTMyTray, elec\myvcl\HTray), and three PT-W3 units in ht9045_sm --
    #   OmronLaser/{LaserSensor,LaserSensorInArm,LaserSensorShuttle}.cpp -- call
    #   its methods.  A NON-UI archive now needs the body, so "compiled into the
    #   UI targets only" no longer covers the tree.
    #   SAFE because TrayCore.cpp is zero-dependency by design (it includes only
    #   vclcompat/TrayCore.h and <cstdio> -- verified this pass), so vclcompat's
    #   own dependency graph does not widen by one edge.
    #   AI(W906-DropMSVC) 20260817: this note used to also cover render/*.cpp and
    #   cite HT9045_UI_CTRL_SOURCES. Both are GONE -- vclcompat/render/ and the
    #   whole MFC UI were deleted when the product UI became web. Rewritten to
    #   say what is true now, because the old wording was actively dangerous.
    #
    #   LedCore.cpp / BtnPanelCore.cpp stay OUT of this library. Their only
    #   out-of-line consumers are their own ctests (tests/CMakeLists.txt compiles
    #   each straight into test_ledcore / test_btnpanelcore). PROVEN by the
    #   20260817 build: with every UI target deleted, the god-stack still links,
    #   so nothing on the machine side needs those bodies.
    #
    #   *** DO NOT DELETE THE FILES. *** The old wording, "nothing outside the UI
    #   targets references them", is true of the .cpp BODIES and false of the
    #   HEADERS, and someone skimming it could bin all three. Measured 20260817:
    #     vclcompat/LedCore.h      included by 9 machine TUs (ATC/ATCInterface,
    #       aTester_Front, aTester_Rear, csystem.cpp, mykitsuck.h,
    #       EJ1N/MyOmronPanel.h, VacuumUnit/MyVacuumPanel.*)
    #     vclcompat/BtnPanelCore.h included by 8 (forms/fMain.*, TempCtrl/TriTemp,
    #       atester_ProcessCount, EJ1N/MyOmronPanel.h, mykitsuck.h, VacuumUnit/*)
    #     vclcompat/TrayCore.h     included by 3 (OmronLaser/LaserSensor.*,
    #       EJ1N/MyOmronPanel.h)
    #   They are machine-side TYPE stand-ins that happen to have UI-sounding
    #   names. Only the renderers and the MFC CWnd shells were ever UI.
    #
    #   The duplicate direct compile (tests/CMakeLists.txt's test_traycore) is
    #   left alone ON PURPOSE: a static archive member is only extracted to
    #   resolve a symbol that is still undefined, so an object compiled straight
    #   into a target always wins and never collides with the archive copy. That
    #   target keeps the "self-contained, links only vclcompat" property its own
    #   comment claims.
    vclcompat/TrayCore.cpp
)

target_include_directories(vclcompat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---------------------------------------------------------------------------
# Library: ht9045_core
#   Collects all ROOT-level translated source files.
#   Mirror the BCB6 source tree: translated .cpp/.h files live at the project
#   root (same relative path as their BCB6 originals).
# ---------------------------------------------------------------------------
add_library(ht9045_core
    ContactForce.cpp
    # AI(W906-P2a-CF) 20260919: ContactForce 的驅動層（讀 ContactInfo.ini、
    #   填那四張表）。分成獨立 TU 的理由見 ContactForceLoad.h 的橫幅：
    #   ContactForce.cpp 的範圍被它自己的橫幅鎖成「CALC CORE ONLY，
    #   不碰任何 global-config」，而驅動必然要碰 CosFunction /
    #   CUSTOMER_CODE / EP_Install / INSTALL_DOUBLE_EP 與磁碟 IO。
    ContactForceLoad.cpp
    RunStartMode.cpp
    # AI(W906-P2b-CF) 20260919: golden adam6024.cpp:1038 TransformFuntion
    #   （759 行）。只有那一支；ADAM 模組的讀寫與 TfAdam6024 表單不在這一波。
    adam6024.cpp
    cUnitConvert.cpp      # W2 partial: iUnitMultiply100/iUnitMultiply1000
    common.cpp            # W3 partial: ini-helper family only; other regions gated #if 0
    # --- W64b (TESTER/INDEX force-calc extract): TfContact::GetMaxIndexForceLimit /
    #   GetMinForce turned into free functions (ComputeMaxIndexForceLimit / ComputeMinForce).
    #   Only #includes MachineType.h (header-only enum/macro header per KNOWLEDGE.md W0); every
    #   other external read (dDutCount, TestIF.iTestMode, IniConfig.*, fContactForce->SLKClass[i]->
    #   dContactOffset*) is passed in as an explicit scalar parameter, so this TU needs NO link to
    #   ht9045_globals (confirmed empirically: the clean build below links fine without it).
    cContact.cpp
    # --- W906-MainCalcCore (TfMain calc-core extract): 6 pure calc-core TfMain member
    #   functions turned into free functions (ComputeShtModeFlag / ComputeCanChangeRealDummy /
    #   ComputeSiteMapIsStander / ComputeSiteMapPriority / ComputeCanChangeSite /
    #   ComputeCanChangeToSocket). Only #includes MachineType.h (header-only); every other
    #   external read (TestIF.*, MOT[].HasIC(), InArmSuck/OutArmSuck.HasIC(), ShuttleHasIC()/
    #   IndexHasIC()/InputShuttleHasIC(), IniConfig.*, bCanAutoCloseSite, bPickFromLoader,
    #   iPickFromLoadStageTask, SystemStart, iSiteTotal) is passed in as an explicit scalar
    #   parameter -- same "no ht9045_globals link needed" shape as cContact.cpp above.
    MainCalcCore.cpp
)

target_include_directories(ht9045_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ht9045_core links the compat layer (translated logic will depend on it).
target_link_libraries(ht9045_core PUBLIC vclcompat)

# Strict warnings help catch porting errors early.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(HT9045_WARNFLAGS -Wall -Wextra -Wshadow -Wconversion -Wpedantic)
    target_compile_options(vclcompat   PRIVATE ${HT9045_WARNFLAGS})
    target_compile_options(ht9045_core PRIVATE ${HT9045_WARNFLAGS})

    # AI(W64b-ForceCalc-Translate) 20260710: cContact.cpp is the first ht9045_core TU to
    # #include MachineType.h under full -Wpedantic.  MachineType.h (already-accepted W0
    # keystone header) carries several BCB6-preserved `};` stray-semicolon idioms after
    # free-function template bodies (e.g. ChangeToFloatNonPcnt) that ISO C++ pedantically
    # flags as "extra ';'" at namespace scope -- harmless, but not this wave's file to fix.
    # Narrowly suppress -Wpedantic on this ONE translation unit only (same precedent as
    # ht9045_public's HTMD5.cpp / ht9045_globals' idiom carve-outs); every other warning
    # class (-Wall -Wextra -Wshadow -Wconversion) still applies in full to cContact.cpp.
    set_source_files_properties(cContact.cpp PROPERTIES COMPILE_FLAGS "-Wno-pedantic")

    # AI(ht9045-v899) 20260722: MainCalcCore.cpp also #includes MachineType.h (via
    # MainCalcCore.h) and hits the SAME stray-';' -Wpedantic idiom noted above for
    # cContact.cpp. Same narrow suppression, same reasoning.
    set_source_files_properties(MainCalcCore.cpp PROPERTIES COMPILE_FLAGS "-Wno-pedantic")

    # AI(W906-P2b-CF) 20260919: adam6024.cpp 的 TransformFuntion 是 golden
    #   adam6024.cpp:1038-1796 的逐字翻譯，裡面有 7 處 golden 自己的隱式
    #   double->int 轉換（例 `int iMax = fMaxUnit;`，golden :1793）。
    #   加 (int) 轉型可以消掉警告，但那會讓這個檔不再逐字等於 golden ——
    #   而逐字是這一波的價值所在（759 行的力量換算，改錯一個運算就壓錯力道）。
    #   ⇒ 用既有的 per-TU 抑制機制（同 HTMD5.cpp / cContact.cpp 的先例），
    #     只關 -Wfloat-conversion 這一類，其餘（-Wall -Wextra -Wshadow
    #     -Wconversion 的其他子類）對本檔照樣全開。
    set_source_files_properties(adam6024.cpp PROPERTIES COMPILE_FLAGS "-Wno-float-conversion")
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_public
#   Portable utility modules translated from BCB6 Public/*.
#
#   cJSON.c  -- portable C (Dave Gamble, MIT); compiled as C via set_source_files_properties.
#              No VCL dependency.  #include "Public/cJSON.h" from any C++ TU.
#
#   HTMD5.cpp -- MD5 class + file/folder helpers.  Depends on vclcompat for
#               AnsiString, TStrings/TStringList, FileExists, DirectoryExists.
#               VCL TMask replaced by simple_wildcard_match(); auto_ptr -> unique_ptr.
# ---------------------------------------------------------------------------
# Compile cJSON.c as C (it must not see C++ name mangling on its own symbols).
set_source_files_properties(Public/cJSON.c PROPERTIES LANGUAGE C)

#  AI(W906-WebBridge) 20260805: cJSON.c was briefly extracted into its own
#  target (so ht9045_webbridge could reach cJSON without inheriting this
#  target's PUBLIC vclcompat dependency) and then put back, because two tests
#  started failing at the same time and the extraction was the obvious suspect.
#
#  IT WAS NOT THE CAUSE, and the real cause was worth knowing:
#      MyPLCModbus        passed unoptimised, FAILED under -O3
#                         ("client3 starts disconnected")
#      BarCodeBottom2DID  passed unoptimised, SEGFAULTED under -O3 before
#                         main() printed anything
#  Proven at the time by building the same source both ways: build_nobt (no
#  CMAKE_BUILD_TYPE) passed both; a -DCMAKE_BUILD_TYPE=Release build failed
#  both, with or without the extraction.
#
#  AI(W906-IdlePump) 20260817: *** THIS NOTE IS NOW STALE, AND THE TENSES
#  ABOVE ARE PAST TENSE FOR THAT REASON. BOTH TESTS PASS UNDER -O3 TODAY. ***
#  Measured in a FRESH -DCMAKE_BUILD_TYPE=Release dir (build_idle_rel):
#      133 tests, 4 failed -- config_db, config_loaders, dfm2rc_idempotent,
#      GA1_ReadGeneralIni -- an IDENTICAL list to the Debug run beside it.
#      MyPLCModbus and BarCodeBottom2DID re-run alone: 2/2 Passed.
#  Some later translation wave fixed them; nobody noticed because nothing
#  re-ran Release after this note was written. Left in place rather than
#  deleted, because the REASONING is still the lesson worth keeping (a link
#  change was blamed for two real latent optimisation defects), and because
#  a reader who remembers the warning needs to see it retracted, not gone.
#  CONSEQUENCE: Release is a viable build type again. That matters for the
#  eventual 64-bit product build -- see install/README.md section 6b -- so do
#  not cite this paragraph as a reason to avoid it. Re-measure before
#  trusting either way; that is the actual lesson.
#
#  The reason nobody had seen this: every recorded baseline in this tree
#  (including the 122/126 of 20260804) was measured with CMAKE_BUILD_TYPE
#  EMPTY -- i.e. no -O3, no -DNDEBUG. These are two real latent defects, not
#  build-system damage, and they are the kind that only ever show up in the
#  build that ships.
#
#  The extraction itself is therefore harmless and could be redone. It is left
#  undone only because the simpler structure works and the link-level
#  independence it buys is worth less than the source-level independence
#  WebBridge already has. If it is wanted, the cleaner route is to drop
#  WebBridge's cJSON dependency entirely -- it uses cJSON in exactly one
#  function (WebBridgeServer.cpp's inbound-command parser, 21 references, a
#  flat object with five known keys).
add_library(ht9045_public
    Public/cJSON.c
    Public/HTMD5.cpp
    # --- W1 Public leaf utilities (BCB6 -> standard C++17) ------------------
    Public/ExternFunction.cpp      # Math_AdjustAngle / String_JustNumber /
                                   # String_Erase[LR]Space / RecordTime  (pure logic)
    Public/WinSocketErrorCode.cpp  # GetErrorMsg(Winsock code -> AnsiString); needs ws2_32 headers
    Public/cBootLog.cpp            # Win32 file-based boot tracer (WriteBootLog / Rotate)
    # --- HAL enabler: KeyPro license interface-cut --------------------------
    #   KeyPro_GetLevel(unsigned) -- portable stand-in for the SOLE KeyPro entry
    #   the V906 source calls (KEYPRO_GET_LEVEL(3) at HS_Function.cpp:4933).
    #   Resolves the export from KeyProDLL.dll at RUN TIME (LoadLibrary/
    #   GetProcAddress) and returns 1 (valid dongle) when the DLL is absent
    #   (offline).  Deliberately does NOT static-link KeyProDLL.lib (32-bit OMF
    #   Borland import lib, unusable by MinGW) -- no extra target_link_libraries
    #   entry is added for it.  The std::string-returning KeyPro exports are
    #   NEVER called in 906 and are NOT ported.
    Public/HTKeyProShim.cpp
)

# cJSON.c uses WIN32 FindFirstFile indirectly (via cJSON.h guards); HTMD5.cpp
# uses FindFirstFileA directly.  Both need windows.h available under MinGW --
# it is, but we silence the dllexport chatter by hiding symbols for the static build.
target_compile_definitions(ht9045_public PRIVATE
    CJSON_HIDE_SYMBOLS          # suppress __declspec(dllexport) in cJSON.h
)

target_include_directories(ht9045_public PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_public PUBLIC vclcompat)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_public PRIVATE ${HT9045_WARNFLAGS})
    # HTMD5.cpp contains the original RFC 1321 MD5 algorithm bit-operations and
    # streampos casts that were never ported (intentional faithful translation).
    # Suppress inherited warnings on that TU only; all other TUs keep full strictness.
    set_source_files_properties(Public/HTMD5.cpp PROPERTIES COMPILE_FLAGS
        "-Wno-conversion -Wno-parentheses")
endif()

# ht9045_core also benefits from public utilities.
target_link_libraries(ht9045_core PUBLIC ht9045_public)

# ---------------------------------------------------------------------------
# Library: ht9045_common
#   Translated logic islands under Common/ that are interface-driven and free
#   of Prod/cmydef/MachineType/hardware/VCL-form coupling.
#
#   PickPlanner -- generic enumerate-and-score Arm pick/place planner
#     (cArmPickPlan) plus two production adapters that are interface-driven
#     via IArmPickPlanContext:
#       cInArmLoaderPickPlanner -- Phase 3 scaffold (Search() returns false)
#       cOutArmPlacePlanner     -- Phase 4 scaffold (Search() returns false)
#     The adapters ship as scaffolds: the Phase 3.1 / 4.1 bodies that would
#     pull MOT[]/InArmSuck/OutArmSuck globals are deferred to W6 and remain
#     inline TODOs.  Nothing here reaches an untranslated global, so the whole
#     subtree compiles standalone against vclcompat only.
# ---------------------------------------------------------------------------
add_library(ht9045_common
    Common/PickPlanner/cArmPickPlan.cpp
    Common/PickPlanner/cInArmLoaderPickPlanner.cpp
    Common/PickPlanner/cOutArmPlacePlanner.cpp
)

target_include_directories(ht9045_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/PickPlanner
)

# PickPlanner .cpp files swap <vcl.h> for the vclcompat umbrella header, so the
# compat layer must be on the include path even though no VCL type is used.
target_link_libraries(ht9045_common PUBLIC vclcompat)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_common PRIVATE ${HT9045_WARNFLAGS})
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_globals  (W0-TAIL keystone)
#   The de-VCL'd GLOBAL define/type headers + the .cpp regions that define the
#   contract globals. These headers (MachineType / Config / CosFunction / cprod /
#   cpublic / cmydef / myTimer / MachineDefine) gate most downstream logic.
#
#   Each .cpp keeps its active global var DEFINITIONS (Prod / IniConfig /
#   CosFunction / Offset / ARM_OFFSET* arrays / name-tables / TQPF_Timer) while
#   the function bodies that reach untranslated state machines / app headers are
#   gated behind '#if 0 // TODO(Wx)'.  Built as a static library: it must COMPILE
#   standalone (verified), but is NOT linked into an executable here because the
#   active globals construct objects whose ctors live in the gated regions
#   (those resolve in W3/W6/W7).  myTimer.cpp is fully translated this wave.
# ---------------------------------------------------------------------------
add_library(ht9045_globals STATIC
    myTimer.cpp
    cprod.cpp  FileRW/_fallback.cpp  # AI(W906-FILERW-FALLBACK) 20260924: cprod.cpp 引用的兩個 FileRW 函式在非 wb_serve 程式的後備（wb_serve 自己的 FileRW 物件先解，不會抽出本成員）；見檔頭
    cpublic.cpp
    cmydef.cpp  cmydef_InitialMemory.cpp  cmydef_TotalYield.cpp   # AI(W906-TOTALYIELD) 20260927: golden GetTotalYield_double/_Str (cmydef.cpp:5885-5901), live copies (INBOX 81).  AI(W906-INITMEM) 20260927: golden InitialMemory (cmydef.cpp:5786-5872), live copy; called by wb_serve before LoadMachineConfig
    LastSet.cpp    # AI(W906-GA1-B2-integrate) 20260804: moved here from ht9045_sm -- the
                   # ungated cprod.cpp (same archive) references LastSet directly, and
                   # globals is the lowest common archive every consumer already links.
    # AI(W906-PT-W1) 20260807: EJ1N/TextProcess.cpp -- FIRST file of golden EJ1N/ to
    # land.  Leaf text/protocol conversions (ASCII<->dec, hex<->binary text, BCC/LRC
    # checksums, Omron analogue scaling, COM-port probe).  Placed in globals, not
    # ht9045_sm, because it is the same class of leaf as cpublic.cpp/cmydef.cpp and
    # its real consumers are cpublic.cpp:307-346 (THIS archive) and
    # CCLink/MyCCLink.cpp:80-83 (ht9045_comms, which already PUBLIC-links globals at
    # :1073).  Substrate is vclcompat + CRT + kernel32 only -- no new link edge.
    EJ1N/TextProcess.cpp
    # AI(W906-PT-W2) 20260807: pure-data / pure-leaf units from translation wave
    # PT-W2.  All four are the same class as EJ1N/TextProcess.cpp above: they
    # define constants and globals or compute over nothing but the CRT, so they
    # belong in the lowest archive every consumer already links.
    #   Config.cpp       -- struct HT9045_CONFIG + the `IniConfig` global whose
    #                       header (Config.h) was already ported and included
    #                       tree-wide; the .cpp was the missing definition.
    #   MessageDef.cpp   -- message-id constants only, zero functions.
    #   SgdToXLS.cpp     -- one free function.
    #   PMAlarm/uTimeTool.cpp -- 10 date/time helpers over vclcompat TDateTime.
    Config.cpp
    MessageDef.cpp
    SgdToXLS.cpp
    PMAlarm/uTimeTool.cpp
    # AI(W906-PT-W3-integrate) 20260808: CosFunction.cpp -- the customer-code
    # configuration layer (176 bodies: DoCustomerFunction + InitialCosFunction +
    # ~90 per-customer CC_* appliers).  Same class as Config.cpp above: it defines the
    # `CosFunction` global whose header was already ported and included tree-wide, and
    # computes over nothing but other globals.  Its only caller, cprod.cpp's
    # CustomerFunctionSelect(), is in THIS archive.
    #   Until this landed, `CosFunction` was 488 bytes of BSS zero at runtime -- the
    #   measured cause of "the whole config layer reads zero" recorded in WB-1.
    CosFunction.cpp  HAlarm.cpp  # AI(W906-HALARM) 20260926: golden BCB6 component D:\HT9045\elec\Component\HAlarm.cpp（RULINGS_20260926 第 16 條）。放 globals：使用者是 mycylin.cpp（ht9045_io）、myGALILmotor.cpp（ht9045_motor）、ckernel.cpp（ht9045_sm），三者都連 globals；本體只用 CRT＋kernel32，沒有新的連結邊。SystemNG 的定義跟著搬來（acarry_shims.cpp 改成宣告）
)

target_include_directories(ht9045_globals PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_globals PUBLIC vclcompat)

# AI(ht9045-v899) 20260626: cpublic.cpp now ungates FindAndKillProcess (psapi) and
# VerInfo::GetAppVersion/m_GetVerInfo (version.lib).  Both ship with MinGW but
# must be listed explicitly -- the linker does not auto-pull them.
if(WIN32)
    target_link_libraries(ht9045_globals PUBLIC psapi version)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # The translated globals carry the original BCB6 idioms (e.g. AnsiString
    # array brace-init with NULL).  Keep them compiling without the strict
    # -Werror-style flags used on freshly-written modules.
    target_compile_options(ht9045_globals PRIVATE
        -Wno-conversion-null -Wno-write-strings)
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_forms  (W7-F0 keystone -- the FormsFacade sink)
#
# AI(W906-W7-F0) 20260728: NEW library, created by the W7-F0 FormsFacade
# refactor (docs/W7_UI_ARCHITECTURE_PLAN.md SS2-D6 / SS6-F0-d).
#
# WHAT IT IS
#   The 9 non-VCL form-pointer stand-ins the state machines dereference
#   (fMain / fLotInfo / fSCKART / fSortCT / fAGV / fNote / fOffSet / fCleaning /
#   fShowMessage), previously all in a single root-level FormsFacade.cpp that
#   was compiled into ht9045_sm.  FormsFacade.cpp is gone; FormsFacade.h is now
#   an umbrella that just #includes forms/*.h, so all 113 existing
#   `#include "FormsFacade.h"` lines are untouched.
#
# WHY IT EXISTS -- IT BREAKS A REAL CMake CYCLE ("Wall 2", plan SS4-V1/SS3-C7)
#   Before this wave, verified directly in THIS file:
#     * FormsFacade.cpp was in ht9045_sm's source list;
#     * `target_link_libraries(ht9045_secsgem PUBLIC vclcompat ht9045_public
#        ht9045_core ht9045_globals ht9045_db)` -- ht9045_secsgem does NOT link
#        ht9045_sm;
#     * `target_link_libraries(ht9045_sm PUBLIC ht9045_io ht9045_motor
#        ht9045_globals vclcompat ht9045_secsgem)` -- ht9045_sm DOES link
#        ht9045_secsgem.
#   So SECSGEM/uHGemHT9045.cpp could never reach fMain: adding ht9045_sm to
#   ht9045_secsgem would have created a genuine CMake target cycle.  That, not
#   the "~19 missing FormsFacade stubs" MIGRATION_ROADMAP.md:133 blames, was the
#   structural blocker.  Sinking the facade to a BOTTOM layer removes it:
#   ht9045_secsgem now links ht9045_forms (see its own link line below), and
#   ht9045_forms links nothing above itself.
#
#   THE EXACT MECHANISM -- AI(W906-W7-F0-fix) 20260728, measured, because the
#   plain phrase "CMake target cycle" is NOT self-evidently an error:
#     * A cycle between two STATIC libraries is LEGAL in CMake.  Measured with a
#       2-target throwaway probe (A STATIC <-> B STATIC): configure exit 0,
#       build exit 0, and the generated linkLibs.rsp reads
#       `libA.a libB.a libA.a libB.a` -- CMake just repeats the archives.  Every
#       library in this file is STATIC (no add_library(... SHARED), and
#       BUILD_SHARED_LIBS is never set), so "they are all static libs, therefore
#       a cycle is fine" would have been a reasonable and WRONG conclusion.
#     * What makes it a hard error HERE is that the cycle runs through a
#       $<LINK_GROUP:RESCAN,...> node, and a group is not a STATIC_LIBRARY
#       target -- CMake's rule is "cyclic dependencies are allowed only among
#       static libraries".  Measured on THIS project without editing it, by
#       re-injecting the pre-F0 edge via
#         cmake -DCMAKE_PROJECT_INCLUDE=<file containing>
#           cmake_language(DEFER ... CALL target_link_libraries
#                          ht9045_secsgem PUBLIC ht9045_sm)
#       -> configure/generate FAILED (exit 1) with
#         "The inter-target dependency graph, for the target
#          test_config_loaders, contains the following strongly connected
#          component (cycle): group RESCAN:{ht9045_secsgem,ht9045_forms,...}
#          depends on ht9045_sm; ht9045_sm depends on group ..."
#       and the same error for test_uHGemEquipment -- and ONLY those two, because
#       they are the only two targets whose RESCAN group contains
#       ht9045_secsgem but NOT ht9045_sm (tests/CMakeLists.txt:291 and :2027).
#       Both groups already had that shape before this wave (verified against
#       git HEAD), so the blocker was real pre-F0 too.
#     * The same injection with `target_link_libraries(ht9045_forms PUBLIC
#       ht9045_sm)` instead ALSO fails (exit 1, same two targets).  So "just
#       declare the ht9045_forms -> ht9045_sm dependency honestly and drop the
#       bottom-layer claim" is not an available option -- it does not configure.
#       That is why the fix below moves the source instead.
#
# DEPENDENCIES -- and why they are acyclic
#   * vclcompat       -- AnsiString / TStringList / TStringGrid / the unified
#                        widget stand-ins in vclcompat/Controls.h.
#   * ht9045_globals  -- forms/fMain.cpp's faithfully-translated
#                        WritePERSITETemperature body reads TestIF.iTestType
#                        (cprod.cpp) and writes asTCPIPTemperature (cmydef.cpp),
#                        and the ctor uses MAX_SOCKET_TOTAL (MachineType.h via
#                        cprod.h).  ht9045_globals itself links no project
#                        library except vclcompat (line ~359 in this file; the
#                        only other entries are the psapi/version Win32 import
#                        libs at ~365), so this edge introduces no cycle.
#   * Automation/AGV_predicates.cpp is a SOURCE of this target (not a link
#     dependency) -- see "NO UNDECLARED BACK-EDGE" below.
#   The plan's wording is "depends only on vclcompat"; ht9045_globals is added
#   because it is a REAL, already-existing dependency of the moved code (it was
#   satisfied implicitly while the facade sat inside ht9045_sm) and declaring it
#   keeps the layering honest without touching the property that matters -- that
#   ht9045_forms depends on nothing that depends on ht9045_secsgem or ht9045_sm.
#
# NO UNDECLARED BACK-EDGE -- AI(W906-W7-F0-fix) 20260728
#   As first landed, this library had a 4th, UNDECLARED dependency: forms/
#   fAGV.cpp's 3 predicates delegate to the free functions in
#   Automation/AGV_predicates.cpp, which was compiled into ht9045_sm.  So
#   libht9045_forms.a carried 3 undefined references it declared no dependency
#   for -- measured on the built archive with
#     nm --undefined-only libht9045_forms.a
#   which attributed all 3 (AGV_IsSPIL_AMR / AGV_IsATK_AMR / AGV_Use_AMR) to
#   fAGV.cpp.obj.  In archive reality ht9045_forms depended on ht9045_sm, i.e.
#   the "bottom layer" property that justifies the ht9045_secsgem edge below did
#   not actually hold; it only LOOKED like it held because GNU ld extracts
#   fAGV.cpp.obj lazily, so targets that never touch fAGV (e.g.
#   test_config_loaders, tests/CMakeLists.txt:291, which links ht9045_forms
#   WITHOUT ht9045_sm) linked anyway.
#   FIX: Automation/AGV_predicates.cpp is compiled HERE (and REMOVED from
#   ht9045_sm's source list, see that target).  This is sound because:
#     * Its only external symbols are IniConfig (cprod.cpp:50),
#       USE_COVER_TRAYID and CUSTOMER_CODE (cmydef.cpp) -- all in
#       ht9045_globals, which is already a declared dependency above.  It pulls
#       in nothing upward, so no cycle.
#     * It matches GOLDEN's own grouping: golden defines all 3 as TfAGV MEMBER
#       functions inside the fAGV form's own unit (Automation/AGV.cpp:1315 /
#       1327 / 1339, declared Automation/AGV.h:204-206) -- they are form code,
#       not state-machine code.  W5 extracted them to free functions so the SMs
#       could call them without the VCL form; the extraction chose the
#       Automation/ directory, not a different layer.
#     * ht9045_sm PUBLIC-links this target (see its link line), so its own
#       consumer Automation/AGV_PortScan.cpp still resolves them.  No test target
#       is stranded either: grep over tests/CMakeLists.txt finds ZERO
#       $<LINK_GROUP:...> containing ht9045_sm without ht9045_forms.  (The
#       converse DOES occur -- :291 and :2027 have ht9045_forms without
#       ht9045_sm -- and that is exactly the direction the layering permits, and
#       is now genuinely safe rather than accidentally safe.)
#   ht9045_forms remains listed INSIDE the existing $<LINK_GROUP:RESCAN,...>
#   groups because its members reference ht9045_globals, which is listed AFTER
#   it in those groups -- GNU ld is single-pass, so the group's re-scan is what
#   settles that.  MSVC's link.exe re-scans archives on its own, which is why
#   the RESCAN group was a declared no-op there (the if(MSVC) block was removed
#   top of this file).
# ---------------------------------------------------------------------------
add_library(ht9045_forms STATIC
    forms/fMain.cpp
    forms/fLotInfo.cpp
    forms/fSCKART.cpp
    forms/fSortCT.cpp
    forms/fAGV.cpp
    forms/fNote.cpp
    forms/fOffSet.cpp
    forms/fCleaning.cpp
    forms/fShowMessage.cpp
    # AI(W906-W7-L1-Wave0) 20260801: 4 new form stand-ins for the asendic_* tray SM
    # family, landed by the W7-L1 Wave-0 serialized facade pass.  fTrayForm and
    # fOCR are genuinely first homes; forms/fProductionInfo.cpp supplies the first
    # DEFINITION of a global two out-arm variants only ever extern-declared; and
    # forms/fFixAICCD.cpp takes over the fFixAICCD global that aoutarm_shims.cpp
    # used to define as a NULL pointer (see FormsFacade.h for the exact status of
    # each).  Same bottom-layer dependency shape as the 9 above (vclcompat +
    # ht9045_globals only).
    forms/fTrayForm.cpp
    forms/fOCR.cpp
    # AI(W906-FW3-PIMOVE) 20260828: `forms/fProductionInfo.cpp` 已移到
    # **ht9045_sm** 的來源清單（見該處註記與量測）。
    # 上面那段 W7-L1-Wave0 的敘述提到它時是當時的實情，**不改寫**；
    # 它現在的家在 ht9045_sm。
    forms/fFixAICCD.cpp
    # AI(W906-W7-L2) 20260803: 5 new form stand-ins landed by the W7-L2 substrate
    # pass, in front of the golden ckernel.cpp translation.  All five are FIRST
    # homes -- none of these form types existed anywhere in this tree before.
    #   fShuttleMove -- WaitManualRetryKey (golden ckernel.cpp:133-149) only.
    #   fHome / fSetup / fRotate / fAOI -- ScanSystemSensor (golden :359-691),
    #   which derefs 7 distinct form globals across 15 live sites.
    #   AI(W906-W7-L2) 20260803 CORRECTED: this comment first said fAOI's global is
    #   deliberately NULL.  It is not.  forms/fAOI.cpp:35 allocates a live instance
    #   at static init, deliberately and against the wave brief, because golden
    #   HT9045.cpp:233 creates the form at application start -- so golden's guard
    #   at ckernel.cpp:502 (`if(FrmAOI!=NULL)`) is TRUE on a real machine and the
    #   faithful offline state is a live object.  See forms/fAOI.cpp:19-24.
    # Same layer as their siblings above (plain-data facades over ht9045_globals,
    # no upward edge), so no new link dependency is introduced.
    forms/fShuttleMove.cpp
    forms/fHome.cpp
    forms/fSetup.cpp
    # AI(W906-FW-QWKEY1-integ) 20260824: fQwertyKey/fPassword facades -- the
    # four globals (fQwertyKey/fQwertyKey2/fPassword/fPassword2) must live in
    # the forms layer so queued ht9045_forms consumers can un-gate without
    # re-creating the undeclared forms->sm archive edge (:618-626). Their
    # sm-reaching methods live in root myQwertyKeyBoard.cpp/Password.cpp
    # (same split as cSetUp.cpp/cTemperFrom.cpp).
    forms/fQwertyKey.cpp
    forms/fPassword.cpp
    forms/fRotate.cpp
    forms/fAOI.cpp
    # AI(W906-FW3-IOSV1) 20260827: iosetview facade (golden iosetview.cpp, class
    # Tfiosetview -- 5 of 65 methods, the read-only display slice).  Belongs in
    # ht9045_forms, measured with nm --undefined-only on the compiled .o: the
    # ONLY undefined non-runtime symbols are TTLCfg / TestIF_File (cprod.cpp:44/:31)
    # and _8Bit/_10Bit/_10BitPE/_10BitPO (cmydef.cpp:217-223) -- all six are live
    # (gate_depth_map: not inside any #if 0) and all six are ht9045_globals, which
    # ht9045_forms already links.  ZERO ht9045_sm / ht9045_motor symbols, so this
    # adds no archive edge.
    #
    # NAME NOTE: the class name Tfiosetview is free, but the GLOBAL `fiosetview`
    # is owned by atester_shims.h:354 (extern TfiosetviewShim *fiosetview, defined
    # atester_shims.cpp:380) with 5 live callers.  The facade therefore declares
    # NO global at all -- same resolution as W32's TfTrayMappingForm.  Verified
    # comment-stripped: zero standalone `fiosetview` identifiers in either file.
    forms/fIoSetView.cpp
    # AI(W906-FW3-HS1) 20260827: golden HS_Function.cpp's TFormHS, 13 of 66 bodies.
    # NAME NOTE: same shape as fIoSetView above -- the class name TFormHS is free,
    # but the GLOBAL `FormHS` is owned by Automation/SCK_ART_Remainder.h:629
    # (extern W5SckArtRem_FormHSStub *FormHS, defined SCK_ART_Remainder.cpp:382)
    # with 2 live callers at SCK_ART_Remainder.cpp:2746 and :2750.  The facade
    # declares NO global; verified comment-stripped that neither delivered file
    # writes that identifier.
    # LINK NOTE: this TU references RecordProcess / ShowMyMessage /
    # ShowErrorMessage (canary_support.cpp) and MyDBIProcess
    # (aHotPlateSubstrate.cpp), all four in ht9045_sm, which ht9045_forms does not
    # link.  That is NOT a new exposure: forms/fLotInfo.cpp -- already in this same
    # source list -- calls them 4 / 14 / 4 / 2 times respectively and the tree
    # links green, because the final executables link both archives.  Measured
    # 20260827, comment-stripped, not taken from the wave agent's report.
    # NOT CONNECTED: with the global name taken, nothing can obtain a TFormHS*,
    # so this object will not be extracted from the archive by any current link.
    # Registering it keeps the file compiled and honest rather than rotting
    # outside the build; it does not make it reachable.
    forms/fHS.cpp
    # AI(W906-FW3-ATC1) 20260827: golden ATC/ATC_Handler_Side.cpp's
    # TATC_InterfaceForm, 28 of 169 bodies (1 ctor + 27 methods).
    # NAME NOTE: three near-identical names, and picking the wrong one links
    # cleanly while reading every field at the wrong offset:
    #   TATC_InterfaceForm      golden ATC_Handler_Side.h -- THIS one, class name
    #                           free in the port, so the facade owns it
    #   TATC_InterfaceFormShim  acarry_shims.h:109 -- owns the GLOBAL
    #                           ATC_InterfaceForm (:115) with live readers at
    #                           acarry.cpp:8349 and aTester_Front.cpp:479, and
    #                           carries exactly one field, iATC_MODE_TYPE
    #   TATCInterfaceForm       ATC/ATCInterface.h:301 -- NO underscore, a
    #                           DIFFERENT golden file, already translated, in
    #                           ht9045_sm. Unrelated to this wave.
    # The facade declares no global, same shape (c) resolution as fIoSetView and
    # fHS above. Verified comment-stripped: neither delivered file names
    # ATC_InterfaceForm or ATC_Data outside comments.
    # NOT CONNECTED: with the global taken, nothing can obtain a
    # TATC_InterfaceForm*, so this object is not extracted by any current link.
    # Only external symbol is Total_Compressor (cmydef.cpp:5679, ht9045_globals,
    # not inside any #if 0) -- measured with nm on the compiled .o.
    forms/fATCHandlerSide.cpp
    # AI(W906-FW3-MES1) 20260827: golden Mes/fVATMesFileSys.cpp's TfMesSystem,
    # 17 of 46 bodies (13 TfMesSystem:: incl. ctor and a port-only dtor, 2
    # uReadDeviceNumer::, 3 file-scope).
    # DIFFERENT FROM THE THREE WAVES ABOVE: both the class name TfMesSystem and
    # the global fMesSystem are free in this tree, so this facade DOES declare
    # the global (forms/fMesSystem.h:704 / .cpp:597), the same idiom as
    # forms/fCleaning.cpp:56.  fIoSetView / fHS / fATCHandlerSide could not,
    # because a shim already owned each of their globals.
    # STILL NOT REACHED TODAY, but for a different reason: the 11 call sites that
    # name fMesSystem-> are all inside #if 0 (atester.cpp:3271, cinitial.cpp:9728,
    # ainarm9045.cpp:5231-5240, csystem.cpp:10154/10553, aTester_Front.cpp:7659-,
    # aTester_Rear.cpp:7686-, forms/fLotInfo.cpp:2696/2708, cprod.cpp:1266-1268).
    # When those are un-gated they will bind to THIS class -- signatures were
    # checked against the actual call-site argument shapes.  That is the material
    # difference from the shim-owned cases, where un-gating would bind to the
    # shim instead and fail.
    # LINK NOTE: RecordProcess (canary_support.cpp, ht9045_sm) rides the existing
    # forms/fLotInfo.cpp exception; cJSON_* (Public/cJSON.c, ht9045_public) is
    # reachable through the declared chain ht9045_forms -> ht9045_core (:769) ->
    # ht9045_public (:420) -- a clean transitive link, not a new edge.  Everything
    # else is ht9045_globals.  Measured with nm on the compiled .o.
    # ODR NOTE: LastSet.h and canary_support.h each define struct
    # LAST_GENERAL_SET with different layouts (see LastSet.h:20).  This TU takes
    # RecordProcess's declaration from cMyDB.h instead of canary_support.h so it
    # can include the full LastSet.h without the collision.
    forms/fMesSystem.cpp
    # AI(W906-FW3-SD1) 20260827: golden SmartDiagnostic.cpp's TfSmartDiagnostic,
    # 24 of 36 bodies (23 TfSmartDiagnostic:: plus one file-scope function),
    # 711 of 966 golden span lines -- the thickest wave of this campaign so far,
    # and the first picked by tools/census/wave_candidates.py.
    # Class name and global fSmartDiagnostic were both free, so this facade
    # declares the global (fSmartDiagnostic.h:285 / .cpp:37).  All 74
    # fSmartDiagnostic-> sites tree-wide are still inside #if 0, so there is no
    # live caller today; what changes is that csystem.cpp's own H5-G4/G5/G6
    # banners name "the wave that lands a TfSmartDiagnostic facade" as their
    # retirement condition, and this is that wave.  UN-GATING IS STILL A
    # SEPARATE DECISION -- H5-G6's own delta note says un-gating makes a
    # fMain->Pause(...) reachable, which is safety-relevant and stays queued.
    # LINK NOTE: no ht9045_sm, no ht9045_io, no motor/db symbols at all --
    # only IniConfig / CosFunction / N_INTEGER / TQPF_Timer (ht9045_globals),
    # fQwertyKey (ht9045_forms) and vclcompat.  Measured with nm on the .o.
    # Cylinder[] is deliberately never referenced: its definition is in
    # mycylin.cpp, which sits in ht9045_io (add_library at :1044) and is
    # unreachable from here.  mycylin.h is included only for the
    # MaxCylinderItem macro.  That boundary is why
    # sb_SmarDiagnostic_CreateCyliderNameClick is excluded.
    # NAME NOTE: golden's file-scope SplitStrByDotSpaceOnly collides with a
    # DIFFERENT-BODIED global of the same name already in cpublic.h:25 /
    # cpublic.cpp:465 (that one treats space as a delimiter; golden's copy does
    # not).  The facade's copy is renamed SD_SplitStrByDotSpaceOnly and kept
    # static.  Same name, different semantics -- a function-level cousin of the
    # two-headers trap.
    forms/fSmartDiagnostic.cpp
    # AI(W906-FW3-LGM1) 20260828: golden login.cpp's TfLogin, 4 of 4 bodies --
    # the first golden file this campaign has finished. 88 span lines. Class name
    # and global fLogin were both free, so the facade declares the global
    # (fLogin.h:196 / .cpp:23). The 7 reserved names (ShowModal, cbLoginUserName,
    # edLoginNewPassword, edLoginOldPassword, edUserName, labUserName,
    # rgLoginOption) are all consumed by cSecurity.cpp inside its own #if 0; when
    # that opens it binds here. One gate, LG-1: FormShow's
    # fProductionInfo->ScreenkeyboardShow() branch, that method having no port.
    forms/fLogin.cpp
    # AI(W906-FW3-LGM1) 20260828: golden GroundMan/GroundMan.cpp's TfGroundMan,
    # 9 of 18 bodies (10 C++ methods -- the 114-line ctor splits into ctor plus a
    # fully-gated Init(), the forms/fMesSystem.cpp precedent), 307 of 1,605 span
    # lines. Global fGroundMan free and declared (fGroundMan.h:567 / .cpp:30).
    # LINK NOTE, and the reason to read this one: the wave agent first wrote
    # FormShow and Init() translating HSys.iGroundManScanPoint live, then traced
    # HSys to database.cpp -- CMakeLists.txt:871, inside add_library(ht9045_db)
    # at :870 -- which ht9045_forms does not link, and corrected itself by gating
    # those branches (GM-0/GM-4) and dropping the 16 fields whose only consumer
    # was inside them. Main loop verified rather than accepted: all five HSys
    # references in the delivered .cpp are inside #if 0 by gate-depth measurement
    # on the comment-stripped text, and nm on the compiled .o shows no HSys and
    # no ht9045_db/io/sm symbol at all. Without that catch this would have been
    # the first undeclared forms -> ht9045_db edge in the tree.
    # EXCLUSIONS are comms and writes, decided by reading bodies: RS232 open /
    # send / receive (Init_GM_RS232, SetGroundMaster, comGMReceiveData,
    # spbStart/StopComClick), WriteIniData/WriteDataToFile (spbSaveClick,
    # ShowGroundManLog), CheckAndReadIniData's write-back-on-miss
    # (ReadGroundOffset), and DoGroundMasterMonitor -- 840 lines, an RS232 state
    # machine that also calls StopAllMotor() at golden :1295, found by reading.
    # ZERO-WRITER WARNING: the 28 labValue_0_2..labValue_3_5 fields are declared
    # but nothing writes them this wave -- their only writer, comGMReceiveData,
    # is excluded. Unlike previous zero-writer findings this consequence LEAVES
    # the module: when SECSGEM/uHGemHT9045_SV.cpp's GATE [G17] opens, host SV
    # polls of SVID 43300-43327 read back empty strings for every "Ground man CHn
    # Value" -- not a crash, silent empty telemetry. Whoever opens [G17] must
    # land comGMReceiveData first.
    forms/fGroundMan.cpp
    # AI(W906-FW3-OWM1) 20260828: three small greenfield forms in one wave, all
    # three declaring their globals because all three names were free.
    #
    # golden ObserveMagazine.cpp -> TfObserveMagazine, 3 of 3 bodies, 14 of 14
    # span lines, no gate at all -- the second golden file this campaign has
    # finished outright (after login.cpp). All 15 reserved field names declared
    # (labSimMagazineTrayNo, mtMagazineTray1..14, consumed by csystem.cpp:625 and
    # cinitial.cpp:14347-14360). mtMagazineTray* use vclcompat::TrayCore, whose
    # body is TrayCore.cpp at CMakeLists.txt:256 inside add_library(vclcompat) at
    # :137 -- already linked, not a new edge (checked here, not assumed).
    #
    # golden ATC/WinWaySetting.cpp -> TfWinway, 4 of 17 bodies carrying live
    # logic plus 2 declared-but-fully-gated for the reserved-name contract,
    # 26 of 209 span lines. DENOMINATOR IS 17, NOT THE 16 census reports:
    # OpenCommPort is overloaded, `bool OpenCommPort(int)` at golden :156 and
    # `void OpenCommPort()` at :164, and census keys its dict on the name. The
    # overload check added to wave_preflight.py after ATC1 (KNOWLEDGE #23) caught
    # this one, which is the second real instance.
    # Nearly every method here touches arrATC_Site[n] (ATC_WinWay*), whose body
    # is ATC/ATC_WinWay.cpp at CMakeLists.txt:1432 inside add_library(ht9045_comms)
    # at :1408 -- a target ht9045_forms does not link and which is not one of the
    # four documented forms->sm exceptions. The ctor sets arrATC_Site[i] to 0, so
    # even a read-only translation would null-deref the moment it ran; that is why
    # the field-touching methods are not declared at all rather than shipped as
    # degraded stubs. Only a forward declaration of ATC_WinWay appears, creating
    # no link edge -- confirmed with nm, zero ATC_WinWay symbols.
    #
    # golden Monitor/MonitorInterface.cpp -> TfMonitor, 11 of 23 bodies live plus
    # 2 declared-but-gated, 92 of 391 span lines. Same shape: MVCtrl is a
    # MonitorTCPIP*, body at CMakeLists.txt:2334 inside add_library(ht9045_sm) at
    # :1498, unreachable, so the ctor leaves it permanently NULL and every method
    # built around it is undeclared -- including MonitorTimerTimer, the 185-line
    # main loop. MyDBIProcess in FormDestroy rides the documented fLotInfo
    # exception; LogSoftwareOffTime, which is NOT on that list, was gated instead.
    # JUDGEMENT WORTH READING: golden's ctor calls LoadTCPIPParament() at :32.
    # The wave translated that method live (it only reads the ini and writes its
    # own widgets) but gated the CTOR'S CALL to it, because fMonitor is a
    # file-scope `new`, so every executable linking ht9045_forms -- ctest included
    # -- would touch D:\HT9045\system\MVData.ini before main(). Not a null-deref,
    # but the same family as the static-init hazard in KNOWLEDGE.
    forms/fObserveMagazine.cpp
    forms/fWinway.cpp
    forms/fMonitor.cpp
    # AI(W906-FW3-HSP1) 20260828: three more greenfield forms, all three
    # declaring their globals.
    #
    # golden cHotPlate.cpp -> TfHotPlate, 7 of 19 bodies, 149 of 718 span lines.
    # NAME NOTE that turned out to be a false alarm, recorded so nobody re-raises
    # it: this file is cHotPlate, and aHotPlateSubstrate.cpp sits in ht9045_sm,
    # so the obvious worry was a cross-diet edge. Measured instead: HotPlateForm
    # and HotPlateForm_File are defined at cprod.cpp:15/16 in ht9045_globals, and
    # all 14 external symbols this TU needs resolve into ht9045_globals or
    # ht9045_core, none gated. The name similarity is a coincidence.
    # Exclusions from reading bodies: ReadFile uses CheckAndReadIniData, which
    # writes defaults back on a missing key; SaveSetupFile and spbSaveClick write;
    # XST1MouseDown / XCT1MouseDown / cbSelectHPFromDBChange reach Barcode_Reader
    # and SetArmHotPlateYPitch reaches GetHotPlateYHalfPos, all in ht9045_sm;
    # LoadImage needs TImage; FormShortCut needs TWMKey, which has no port.
    #
    # golden cShowBinSet.cpp -> TfShowBinSet, 5 of 7 bodies (4 members plus the
    # file-scope ShowSiteMapping_YES_NO), 83 of 187 span lines.
    # NAME NOTE, and this one is real: TfShowBinSet is NOT TfShowBinSelect.
    # cShowBinSelect.cpp is a different golden file, already translated, living in
    # ht9045_sm with forms/fShowBinSelect.h. The two differ by "elect" versus
    # "et". Verified disjoint before dispatch and again after: TfShowBinSet
    # appears only in dfm2rc generator output, TfShowBinSelect only in its own
    # pair. FormShow is gated precisely because it derefs fShowBinSelect, which is
    # real but unreachable from here.
    # This TU needs ZERO project symbols -- nm shows only C++ runtime vtables.
    #
    # golden Precaution.cpp -> TfPrecaution, 3 of 6 bodies, 21 of 73 span lines.
    # The other three all deref fObserver, whose body is cObserver.cpp in
    # ht9045_sm. Also zero project symbols.
    forms/fHotPlate.cpp
    forms/fShowBinSet.cpp
    forms/fPrecaution.cpp
    # AI(W906-FW3-TIF1) 20260828: three greenfield tester/utility forms.
    #
    # golden cTesterIF.cpp -> TFTestIF, 21 of 38 members ACTIVE, 115 of 1488 span
    # lines.  Member percentage and line percentage disagree sharply here and
    # neither should be quoted alone: every large body in this form is a
    # persistence or link-boundary body, every small one is a keypad handler.
    # All 10 reserved names honoured verbatim (DoIniDataToForm, ReadTestIFFile,
    # SaveSetupFile, cbGPIBType, cbRs232Type, edGPIBAddress, edInitialMaxTest,
    # edMaxTestTime, fShow, rgInterfaceType).
    # NAME NOTE, and this one is real: TFTestIF / FTestIF is the FORM.  It is NOT
    # the TestIF / TestIF_File DATA globals at cprod.cpp:30 (ht9045_globals).
    # Same family as fShowBinSet vs fShowBinSelect one wave earlier.
    # nm --undefined-only measured on the object: CUSTOMER_CODE, CosFunction,
    # IniConfig, N_DOUBLE, N_INTEGER, N_PORT, TestIF (all ht9045_globals),
    # OnlyNumberInPut / OnlyNumberAndDotInPut (common.cpp, ht9045_core),
    # fQwertyKey + TfQwertyKey::ShowQwertyKey (forms/fQwertyKey.cpp, this target)
    # and fTesterTCP (this target, registered below).  ZERO unreachable symbols.
    # Gates are three kinds: safety policy (SaveSetupFile and friends write, so
    # they stay dark), link boundary (Barcode_Reader / CheckTTLBoardBitMode /
    # fShowBinSelect / fSpeed / fYieldMonitoring / SetWorkParameter in ht9045_sm,
    # EventReport in ht9045_secsgem), and members vclcompat has no port for.
    #
    # golden Interface/TesterTCP.cpp -> TfTesterTCP, only 1 of 23 members ACTIVE,
    # and that low number is the CORRECT answer, not a shortfall.  16 of golden's
    # bodies were ALREADY translated -- as free functions, in another file:
    # 12 x TesterTCPSocket_* in Interface/TesterTCP_Socket.cpp and 4 x TesterTCP_*
    # in Interface/TesterTCP.cpp, both ht9045_sm.  16 + 1 ACTIVE + 6 GATED = 23,
    # which is the check that this adds up.  CORRECTION, mine: the first version
    # of this comment said 17 and 13.  There are indeed 13 TesterTCPSocket_*
    # functions, but TesterTCPSocket_Init (:185) is NOT a translated golden body --
    # golden's TfTesterTCP has no member named Init; that function reproduces the
    # .dfm's declarative OnConnect/OnDisconnect/OnError/OnRead bindings.  Counting
    # it inflated golden-member coverage by one.  The wave agent caught this.
    # Spot-measured:
    # TesterTCPSocket_SendTCPIPCommand at TesterTCP_Socket.cpp:331,
    # TesterTCP_CopyOSTestResult at TesterTCP.cpp:455.  Four were RENAMED on the
    # way over (ClientSocket_TCPIPConnect/Disconnect/Error/Read -> On*), so a
    # name-only search under-reports them -- pre-flight shapes (a)/(c)/(d) all
    # read "clear" here and that is true but misleading, because the existing port
    # deliberately did not reproduce golden's class.  Re-translating those 16
    # would have been duplicate work.  This TU needs ZERO project symbols.
    #
    # golden cAirCon.cpp -> TfAirCon, 5 of 16 members, 36 of 164 span lines.
    # Reserved names PowerUp / PowerDown honoured.  SW[] (SwAirConditioner) is
    # myswitch.cpp = ht9045_io, unreachable.  The serial methods are gated by
    # SAFETY POLICY, not by reach: vclcompat::TComm IS linkable from here, so an
    # un-gated StartComm/WriteCommData would really open the port and transmit.
    #
    # Measured with -Wall -Wextra (stricter than this target's flags), real -c
    # compiles, rc captured off-pipeline: all three rc=0, 0 errors, 0 warnings.
    forms/fTesterIF.cpp
    forms/fTesterTCP.cpp
    forms/fAirCon.cpp
    # AI(W906-FW3-DTL1) 20260828: four more greenfield forms.  Line numbers into
    # CMakeLists are deliberately NOT cited here -- they go stale the moment any
    # wave adds a source, and they are comments so nothing catches it.  Target
    # names are the durable fact; those are what is quoted.
    #
    # golden cTowerLight.cpp -> TfTowerLight, ALL 8 members, 101 of 133 span
    # lines (75.9%) -- the highest line coverage of any form so far.  Gates:
    # RGB00Click writes disk (WriteLastDataFile, golden :78, hard-coded path that
    # --dry cannot redirect); rgMusicTestClick is BOTH a link boundary (SW[] ->
    # myswitch.cpp -> ht9045_io) AND a physical buzzer output.  A third gate is
    # line-level, not whole-body: the two trailing myLog.Do_Log(...) calls only
    # (handlerlog.cpp -> ht9045_sm, and it writes disk).  That follows the
    # existing forms/fHandlerSys.h GATE (H22-1) precedent.
    #
    # golden cCounterSel.cpp -> TfCounterSel, ALL 5 members, 10 of 123 lines.
    # Thin by nature: this form's whole job is persisting settings.
    # The gate worth knowing about is a HIDDEN WRITE on a read-sounding path:
    # FormShow -> ProcessLastSetIni_Visible(bReadFile) -> ReadWriteIni(bool)
    # -> CheckAndReadIniData, which WriteBool()s a default back into
    # D:\HT9045\config\config.ini when a key is missing.  Same function-pair
    # trap as fHotPlate's G-1.  This TU needs ZERO project symbols (nm).
    #
    # golden cLd_ULd.cpp -> TfLd_ULd, 15 of 16 members + 1 omitted, 41 of 222
    # lines.  FormShortCut is omitted outright: TWMKey has no port, and its whole
    # body just calls the ACTIVE SetDefaultPos().  The ctor is gated for two
    # independent reasons: elUdUld's body is Public/HTEditList.cpp (ht9045_sm),
    # and a file-scope `new` runs before main() (trap #4).
    #
    # golden EJ1N/fDTME08.cpp -> TfrmDTME08, ALL 55 members, 173 of 693 lines.
    # 19 gates are the dtme08 / uDTME08Control link boundary (ht9045_comms), 6
    # are TMyOmronPanel's OUT-OF-LINE members (its inline ones are fine), and the
    # two hardware ones are SW[SwHeaterRelay] (ht9045_io, and a heater relay is a
    # physical output) and ATC_InterfaceForm (ht9045_sm).  The single biggest
    # gated body, DoDTME08Cycle, is 202 lines = 29% of the file on its own: the
    # whole autonomous poll-and-setpoint loop.
    #
    # NOT RE-TRANSLATED, and this is the point of the new pre-dispatch clause:
    # EJ1N/uDTME08Control.{h,cpp} and EJ1N/MyOmronPanel.{h,cpp} were ALREADY
    # fully ported in the PT-W2 era.  They are used by declaration only.  Missing
    # that would have meant re-translating two whole collaborator classes --
    # exactly the waste that nearly happened to TesterTCP one wave earlier.
    #
    # A REACHABILITY TRAP worth recording: HeaterSVLog LOOKS reachable and is
    # not.  It is declared in cpublic.h and defined in cpublic.cpp, which is in
    # ht9045_globals -- which forms does link.  But that definition sits inside
    # that file's own `#if 0`.  A definition inside `#if 0` is not a definition.
    #
    # ONE NON-MECHANICAL EDIT, flagged in-file: golden cLd_ULd.cpp:189 ends a
    # trailing `//` comment with a backslash, which line-splices into the next
    # (blank) line, so g++ -Wall reports -Wcomment EVEN INSIDE `#if 0` (the lexer
    # runs regardless) and `#pragma GCC diagnostic ignored` does not suppress it.
    # Only the comment DELIMITER changed to /* */; the text is byte-identical to
    # golden and the code line is untouched.  Verified against golden by reading
    # the cp950 source: the line does end in a backslash and the next line is blank.
    #
    # Measured, real -c compiles with -Wall -Wextra, rc taken off-pipeline:
    # all four rc=0, 0 errors, 0 warnings.  nm --undefined-only over the four
    # objects yields 12 distinct project symbols, ALL reachable
    # (ht9045_globals / ht9045_core / ht9045_forms / vclcompat); ZERO from
    # ht9045_sm, _io, _comms, _db, _secsgem, _motor.
    forms/fTowerLight.cpp
    forms/fCounterSel.cpp
    forms/fLd_ULd.cpp
    forms/fDTME08.cpp
    # AI(W906-FW3-BTQ1) 20260828: five more greenfield forms.  Target names are
    # quoted, not CMakeLists line numbers -- those go stale on the next wave and
    # nothing in the build catches it (it happened twice in one morning).
    #
    # golden cBuilder.cpp -> TfBuilder, 13 of 22 members, 123 of 531 span lines.
    # Worth noting WHY the 9 gates exist: NOT ONE of them is a link gate.
    # DataPath / OffsetPath / MyForceDirectories / ShowMyMessage are all reachable
    # from here today -- which is exactly why un-gating would really copy and
    # delete recipe folders.  These are read-only-direction SAFETY gates, and the
    # distinction matters: a link gate opens when the link graph changes, a safety
    # gate opens only when a human decides the write is acceptable.
    #
    # golden cTestCategory.cpp -> TfTestCategory, 4 ACTIVE + 1 partial of 13,
    # 54 of 518 span lines.  FIVE of its gates rest on ONE symbol, IsNNMode,
    # whose only definitions are atester_shims.cpp (a `return 0;` STUB -- trap #1
    # shape 2, the kind nm --undefined-only cannot see) and cinitial.cpp (the real
    # body), both ht9045_sm.  Also: TfTestCategory::Active is a PORT-ONLY field.
    # golden has no such member -- it is inherited TForm::Active, and the
    # no-base-class deviation removes it, so the facade must supply it or the
    # pre-committed PowerSavingMode.cpp site cannot bind.
    #
    # golden QAMode.cpp -> TfQAMode, 5 ACTIVE + 1 partial of 12 members plus the
    # file-scope QABackupStatus, 40 of 302 span lines.
    #
    # golden DIOInterFaceCFG.cpp -> TfDIOFrom, 8 of 14 members, 66 of 236 span
    # lines (28.0%, the best ratio of the five).
    #
    # golden RPDefault.cpp -> TfRPDefault, 4 of 11 members, 30 of 386 span lines.
    # One body dominates the whole wave: CompareRPDefaultAndValue is 228 lines,
    # 59.1% of that golden file on its own.
    # GOLDEN-SIDE FINDING: SetReciepeParameterDefault is declared at golden
    # RPDefault.h:59 and DEFINED NOWHERE IN GOLDEN'S OWN TREE.  That is why
    # pre-flight counts 11 bodies while the golden header lists 12 methods -- the
    # denominator gap is golden's, not ours.
    #
    # RESERVED NAMES: 19, not 18.  The dispatch brief listed 18 because the main
    # loop had read pre-flight through `head -14` and silently truncated
    # fDIOFrom's list; rgStartLogic (pre-committed at forms/fTesterIF.cpp:1477)
    # was the one that fell off.  The wave agent caught it by re-running pre-flight
    # itself, which is why the prompt tells it to.  All 19 are declared and their
    # signatures were checked against the actual call sites -- e.g.
    # CompareRPDefaultAndValue takes (Name, Default, Value) in golden's argument
    # order, which is what forms/fTesterIF.cpp passes.
    #
    # nm --undefined-only over the five objects: 15 distinct project symbols, ALL
    # reachable.  ht9045_core: DataPath, OnlyNumberInPut, OnlyNumberAndDotInPut,
    # OnlyMakeFileDataInPut.  ht9045_globals: IniConfig, CosFunction, TestIF_File,
    # TTLCfg, N_INTEGER, N_NO_SYMBOL, bUseTwoArm32Site, AccessLevel,
    # iDefHonPrecLevel.  ht9045_forms: fQwertyKey + TfQwertyKey::ShowQwertyKey.
    # ht9045_sm: ShowMyMessage ONLY -- canary_support.cpp, one of the four
    # documented forms->sm exceptions, verified not inside #if 0.
    # ZERO from _motor, _io, _db, _comms, _secsgem.
    # Measured with real -c compiles, -Wall -Wextra, rc taken off-pipeline:
    # all five rc=0, 0 errors, 0 warnings.
    #
    # CORRECTION to the standing four-exception list, found by this wave:
    # MyDBIProcess has TWO overloads.  The 2-arg one is aHotPlateSubstrate.cpp
    # (ht9045_sm, the sanctioned exception, and the one these forms call); a 3-arg
    # __fastcall version documented at cMyDB.h:81 has its body in
    # SECSGEM/uHGemEquipment.cpp = ht9045_secsgem, which forms canNOT reach.
    # "MyDBIProcess is an exception" is true of the overload, not of the name.
    #
    # UN-GATE BLOCKER, recorded so it is not discovered late: forms/fTesterIF.cpp
    # reads six fDIOFrom widgets as ->Items->Strings[i].  Those lists are EMPTY in
    # this port because nothing loads a .dfm layout.  Un-gating that site without
    # first supplying the strings indexes an empty list -- the same "check where
    # the value comes from" shape as the fTowerLight LED Tag one wave earlier.
    forms/fBuilder.cpp
    forms/fTestCategory.cpp
    forms/fQAMode.cpp
    forms/fDIOFrom.cpp
    forms/fRPDefault.cpp
    # AI(W906-W7-F0-fix) 20260728: MOVED here from ht9045_sm -- the 3 AMR-mode
    # predicates forms/fAGV.cpp delegates to.  See the "NO UNDECLARED BACK-EDGE"
    # block above.  File content unchanged (zero behaviour change: same code,
    # same callers, different archive).
    Automation/AGV_predicates.cpp
)

target_include_directories(ht9045_forms PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_forms PUBLIC vclcompat ht9045_globals
    # AI(W906-FW-SETUP-C2) 20260821: ht9045_core DECLARED -- forms/fSetup.cpp's
    # XPitchKeyPress (Wave A) calls OnlyNumberAndDotInPut, whose body is
    # common.cpp (ht9045_core). The edge existed undeclared and linked only by
    # GNU ld's lazy extraction (the exact AGV_predicates shape :614-646 fixed
    # once before). Unlike forms->sm (a documented cycle), core links only
    # vclcompat+ht9045_public (:296/:420) and reaches nothing upward -- legal.
    ht9045_core)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Same warning posture as ht9045_globals / ht9045_sm: the facade sees the
    # BCB6 idioms inherited from the global headers (AnsiString brace-init etc.)
    # via cprod.h / cmydef.h.
    target_compile_options(ht9045_forms PRIVATE
        -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()

# ---------------------------------------------------------------------------
# third_party: sqlite3  (W3 external dependency)
#   The golden ref links a prebuilt sqlite3.lib and ships only sqlite3.h (no
#   sqlite3.c).  Per the migration plan, sqlite3 is an EXTERNAL portable
#   dependency, NOT hand-translated -- cMyDB (the WRAPPER) is what we translate
#   (scheduled W3).  This subdir creates an `sqlite3` target resolved as:
#   vendored amalgamation  ->  system find  ->  header-only stub.
#   No executable links it yet (cMyDB is not translated this wave), so the
#   stub path keeps the build green while documenting the dependency.
# ---------------------------------------------------------------------------
add_subdirectory(third_party/sqlite3)
message(STATUS "sqlite3 resolution: ${SQLITE3_RESOLUTION}")

# ---------------------------------------------------------------------------
# Library: ht9045_db  (W3-cont2 config-table loaders)
#   PARTIAL translation of database.cpp: the Mot_Table.csv / IO_Table.csv
#   parsers (SYSTEM_MODULAR::LoadMotData / LoadIoData + the TMOTDATA / TIODATA
#   row ctors + the name-based column resolvers TMOTNO::SetMOTTableNo /
#   TIOTABLENO::SetIOTableNo).  All other database.cpp regions (TDataModule1,
#   SYSTEM_MODULAR ctor/dtor, ReadGeneralIni, SystemModularInitial body,
#   InstallColorBinDisplay) are gated #if 0 // TODO(wave) inside the .cpp.
#
#   Depends on:
#     * vclcompat       -- AnsiString / TStringList / FileExists / HexStrToInt
#     * ht9045_globals  -- INDEX_MOTION_CARD (cmydef.cpp), the cmydef/MachineType
#                          headers, and TQPF_Timer (myTimer.o) for HSys.SysTimer.
#
#   NOT linked into an executable directly here; the test_config_loaders target
#   links it and supplies the two stubbed externals (MyDBIProcess / ShowMyMessage)
#   that the gated UI/DB waves will provide for real.
# ---------------------------------------------------------------------------
add_library(ht9045_db
    database.cpp
    # AI(W906-PT-W2-integrate) 20260807: cMyDB.cpp was translated in an earlier
    # wave and then NEVER REGISTERED IN ANY ARCHIVE -- it has been sitting in the
    # tree compiling for nobody.  Found because PT-W2's Public/HTEditList.cpp is
    # the first ported unit to call RecordChangeLogProcess(AnsiString,AnsiString),
    # whose only body in the tree is cMyDB.cpp:1850, and three test executables
    # link-failed on it.  (cMyDB.h:59 and cMyDB.cpp:1841 both state that symbol
    # "has NO existing stand-in anywhere in the tree, so it is translated
    # ACTIVELY" -- correct, and it still could not be linked.)
    # Verified before adding: `g++ -std=c++17 -fsyntax-only` on cMyDB.cpp is clean.
    # This is deliberately NOT solved by gating the call in HTEditList.cpp: a gate
    # is for a symbol with no body, and this one has a real body -- gating it
    # would have written a false claim into a GATE REGISTER, which is the exact
    # defect class this wave's audit caught twice elsewhere.
    cMyDB.cpp
    # AI(W906-CSVONLY) 20260926: MDB Updater -> V906 (cMyDB plan P2). AlarmCodeCatalog.cpp is generated by
    # tools/gen_alarmcode_catalog.py; AlarmCodeUpdater.cpp is the CSV-only UpdateAlarmList. Standard C++ only.
    AlarmCodeCatalog.cpp
    AlarmCodeUpdater.cpp
    LogObjects.cpp   # AI(W906-CSVONLY-P1) 20260926: golden TfMain TMyStringList log objects (cMyDB plan P1)
)

target_include_directories(ht9045_db PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_db PUBLIC vclcompat ht9045_globals)

# AI(W906-PT-W2-integrate) 20260807: cMyDB.cpp (added to this archive just above)
# is the SQLite wrapper, so this archive now genuinely needs the sqlite3 target
# that CMakeLists.txt:692 already builds.  Until cMyDB.cpp was registered, nothing
# in ht9045_db referenced a single sqlite3 symbol, which is why the edge was
# missing rather than wrong.
target_link_libraries(ht9045_db PUBLIC sqlite3)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # database.cpp carries the original BCB6 idioms inherited from the global
    # headers (AnsiString brace-init etc. pulled via cmydef/cprod).  Match the
    # ht9045_globals warning posture so the translated loaders compile without
    # tripping on idioms that live in the (already-accepted) global headers,
    # while still flagging genuine porting mistakes in the loader bodies.
    target_compile_options(ht9045_db PRIVATE
        -Wall -Wextra -Wno-conversion-null -Wno-write-strings)
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_testercomm  (Tester-comm plan P0 skeleton)
#
# AI(W906-GB-P0) 20260926: SyncMailbox (in-process SendMessage(WM_COPYDATA) stand-in), the single
#   TesterComm thread shared by GPIB/RS232/TCPIP (user ruling 20260926), and TesterCommHub (one engine per
#   TestIF_File.iTestType).  No engine yet; nothing links it into wb_serve yet (P3/P7).  Depends only on the
#   header-only WebBridge/Sync.h and Win32.
# ---------------------------------------------------------------------------
add_library(ht9045_testercomm STATIC
    TesterComm/SyncMailbox.cpp
    TesterComm/TesterCommThread.cpp
    TesterComm/TesterCommHub.cpp
    TesterComm/UiChannel.cpp        # AI(W906-GB-P7) 20260926: engine snapshot / page command channel
    # AI(W906-GB-P1) 20260926: GPIB engine = golden H9046_32GPIB (V12.13.905.0 + HT9050) in namespace gpibbridge.
    #   GpibBridge.h is the fixed header; TesterComm/Gpib/TRANSLATION_RULES.md is how the bodies were translated;
    #   GpibEngine runs it on the TesterComm thread.  NI-488.2 is loaded at run time (gpib-32.dll), no import lib.
    TesterComm/Gpib/GpibDriver.cpp
    TesterComm/Gpib/GpibGlobals.cpp
    TesterComm/Gpib/GpibUi.cpp
    TesterComm/Gpib/GpibTestGpib.cpp
    TesterComm/Gpib/GpibCore.cpp
    TesterComm/Gpib/GpibHandlerMsg.cpp
    TesterComm/Gpib/GpibCommands.cpp
    TesterComm/Gpib/GpibCastleAmdHana.cpp
    TesterComm/Gpib/GpibAux.cpp
    TesterComm/Gpib/GpibEngine.cpp
    TesterComm/Gpib/GpibUiSnapshot.cpp   # AI(W906-GB-P7) 20260926: GPIB tab snapshot + page commands
    # AI(W906-GB-P4) 20260926: RS232Standard (tester RS232 + TTL board = DIO) V12.13.902.0 in namespace rs232std;
    #   Rs232Bridge.h is the fixed header, TesterComm/Rs232/TRANSLATION_RULES.md the rules, Rs232Engine the adapter.
    TesterComm/Rs232/Rs232Globals.cpp
    TesterComm/Rs232/Rs232Ui.cpp
    TesterComm/Rs232/Rs232HandlerMsg.cpp
    TesterComm/Rs232/Rs232Comm.cpp
    TesterComm/Rs232/Rs232Log.cpp
    TesterComm/Rs232/Rs232Parse.cpp
    TesterComm/Rs232/Rs232Support.cpp
    TesterComm/Rs232/Rs232Engine.cpp
    TesterComm/Rs232/Rs232UiSnapshot.cpp  # AI(W906-GB-P7) 20260926: RS232 / TTL tab snapshot + page commands
)
target_include_directories(ht9045_testercomm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# AI(W906-GB-P1) 20260926: the bridge bodies use vclcompat (AnsiString / TStringList / TIniFile / TComm / widgets) and
#   two ht9045_globals objects: MessageDef.o (MSG_CMD_* constants, shared contract) and myTimer.o (TQPF_Timer).
target_link_libraries(ht9045_testercomm PUBLIC vclcompat ht9045_globals)
if(WIN32)
    # AI(W906-GB-P4) 20260926: rs232std uSocketServer / Ping (winsock) on vclcompat ServerSocket / ClientSocket.
    target_link_libraries(ht9045_testercomm PUBLIC ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_testercomm PRIVATE -Wall -Wextra)
    # Golden text kept verbatim carries unused locals/parameters, signed/unsigned compares and char* literals;
    # silence exactly those classes on the translated files so real porting warnings stay visible.
    set_source_files_properties(
        TesterComm/Gpib/GpibGlobals.cpp
        TesterComm/Gpib/GpibUi.cpp
        TesterComm/Gpib/GpibTestGpib.cpp
        TesterComm/Gpib/GpibCore.cpp
        TesterComm/Gpib/GpibHandlerMsg.cpp
        TesterComm/Gpib/GpibCommands.cpp
        TesterComm/Gpib/GpibCastleAmdHana.cpp
        TesterComm/Gpib/GpibAux.cpp
        TesterComm/Rs232/Rs232Globals.cpp
        TesterComm/Rs232/Rs232Ui.cpp
        TesterComm/Rs232/Rs232HandlerMsg.cpp
        TesterComm/Rs232/Rs232Comm.cpp
        TesterComm/Rs232/Rs232Log.cpp
        TesterComm/Rs232/Rs232Parse.cpp
        TesterComm/Rs232/Rs232Support.cpp
        PROPERTIES COMPILE_FLAGS
        "-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-sign-compare -Wno-write-strings -Wno-conversion-null")
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_testercomm_handler  (Tester-comm plan P2a: the Handler side)
#
# AI(W906-GB-P2a) 20260926: golden TfMain bridge-facing code (OnMyCopyMsg case WM_GPIB_Program, SendMSG_CMD,
#   SendMSG_TestMode, RunTestProgram, CloseGpibProgram, ProcessHVisionConnect, WakeupGPIB ...) as class
#   THandlerTesterSide on TesterCommHub.  Separate from ht9045_testercomm on purpose: this is MACHINE code (fMain,
#   Command.cpp replies, TestIF ...), the engines must stay free of it (ruling 11).  Consumers link it together
#   with the machine archives (RESCAN group, see tests/CMakeLists.txt test_testercomm_handler).
# ---------------------------------------------------------------------------
add_library(ht9045_testercomm_handler STATIC
    TesterComm/Handler/HandlerTesterSide.cpp
    TesterComm/Handler/HandlerGpibMsg.cpp
    TesterComm/Handler/HandlerBridgeCtl.cpp
    TesterComm/Handler/TesterCommWiring.cpp   # AI(W906-GB-P3) 20260926: wb_serve entry points (Init/Tick/Poll/Shutdown/Http)
    TesterComm/Handler/HandlerGpibAux.cpp     # AI(W906-GB-P6) 20260926: 2A + Q2(a) GPIB extra RS232 port follows the recipe
    TesterComm/Tcp/TcpPump.cpp                # AI(W906-GB-P5) 20260926: TCP/IP channel pump (Handler thread, golden TfTesterTCP)
)
target_include_directories(ht9045_testercomm_handler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ht9045_testercomm_handler PUBLIC ht9045_testercomm)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_testercomm_handler PRIVATE -Wall -Wextra
        -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-sign-compare
        -Wno-write-strings -Wno-conversion-null)
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_ela  (Event Log Analyzer plan P1: ElaCore, no UI)
#   AI(W906-ELA-P1) 20260927 (St02).  golden EventlogAnalyzer Rev891.0; ledger docs/ELA_PORT_LEDGER.md.
#   Standard C++ + vclcompat TIniFile (JAM0000.dat).  Not linked into wb_serve yet (the /api/ela route is held).
# ---------------------------------------------------------------------------
add_library(ht9045_ela STATIC
    EventLogAnalysis/ElaCore.cpp
    EventLogAnalysis/ElaTables.cpp   # P1b: grids / summary / Production_Log / file scan
    EventLogAnalysis/ElaHub.cpp      # P2 / P3: EL_* commands, worker thread, JSON snapshot (AI(W906-ELA-P2) 20260927)
    EventLogAnalysis/ElaService.cpp  # P3: the production hub + wb_serve entry points (W906_ElaStart / Stop / Post / Http)
)
target_include_directories(ht9045_ela PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ht9045_ela PUBLIC vclcompat ht9045_nmftp)   # AI(W906-ELA-R0) 20260927: TNMFTP for the R4 uploads (no symbol used yet; static archive -> nothing pulled)

# ---------------------------------------------------------------------------
# Library: ht9045_motor  (W4 HAL motor layer)
#
#   HTMotor.{h,cpp}    -- abstract port (virtual base, no vendor SDK)
#   mySimMotor.{h,cpp} -- TMySimMotor : offline sim adapter (default brand)
#   mymotor.{h,cpp}    -- TMyMotor / TTrayMotor (PARTIAL: motion-routing core)
#
#   Sim path (default): compiles + links with NO vendor SDK; no HAVE_xxx macros
#   needed.  The brand drivers (SMC/Galil/MN200/SYNTEK/EtherCAT/MC88X1) are
#   deferred to W4-part2 -- their .cpp files are NOT translated yet because they
#   pull the same W6 god-stack (main.h / cinitial.h / mycylin.h / etc.) as
#   mymotor.cpp.  They will be translated with #if HAVE_xxx vendor guards once
#   the W6 sensor layer lands.
#
#   mymotor.cpp pulls sIC_Type[] (cmydef.h), iYRegNum (cmydef.h), bTestSiteUse[]
#   (cmydef.h), HAS_IC/NULL_IC/HAS_NULL_IC (cprod/cmydef), INDEX_MOTION_CARD
#   (cmydef), MOTION_CARD_TYPE (cmydef), ChangeToFloatNonPcnt (MachineType.h),
#   Prod.* (cprod) -- all resolved by ht9045_globals.
#   mytray.h (TMyTray): NOT yet in Motor/ -- include path is project root.
# ---------------------------------------------------------------------------
add_library(ht9045_motor STATIC
    Motor/HTMotor.cpp
    Motor/mySimMotor.cpp
    Motor/mymotor.cpp
    mytray.cpp          # TMyTray: required by TTrayMotor in mymotor.cpp (W4 partial)
    # AI(W906-PT-W2) 20260807: three more motor-family drivers from wave PT-W2.
    # They join the archive their abstract base already lives in (HTMotor.cpp),
    # alongside mymotor.cpp, and they reach nothing above it.  Hontech_M4 and
    # mySMCmotor both include a FORBIDDEN-TO-MODIFY vendor header out of
    # Motor/vendor/ (CMNet/CMNETError and CSmc respectively) -- included, never
    # edited, and the real vendor calls stay behind the tree's existing
    # #if HAVE_xxx posture so the Sim path still links with no SDK present.
    Motor/Hontech_M4.cpp
    Motor/mySMCmotor.cpp
    EtherCAT/MyEtherCAT.cpp   # reaches MOT[]/TTrayMotor/TOTAL_MOTOR -- motor, not comms
    # AI(W906-PT-W3-integrate) 20260808: MOVED HERE from ht9045_comms.  It was
    #   never wrong to have it there while nothing pulled it; PT-W3's
    #   Motor/myEthercatmotor.cpp is what made MyEtherCAT.cpp.obj get extracted,
    #   and MyEtherCAT.cpp:190 does `new TMyNUEC1(...)` plus 7 more members, so
    #   the definitions have to be reachable from THIS archive.  Consumers of
    #   TMyNUEC1 tree-wide: EtherCAT/MyEtherCAT.{h,cpp} and tests/test_MyNUEC1.cpp
    #   (whose LINK_GROUP lists both archives, so the move is invisible to it).
    #   CanBus/cMyDNM100UD.h only NAMES it in prose -- checked, not a consumer.
    #   Following the MyEtherCAT.cpp precedent one line up rather than adding a
    #   ht9045_motor -> ht9045_comms edge for a single class.
    EtherCAT/MyNUEC1.cpp
    # AI(W906-PT-W3-integrate) 20260808: wave PT-W3's three remaining motor drivers.
    # Same placement rationale as the PT-W2 three above (concrete HTMotor subclasses,
    # vendor headers under Motor/vendor/ included-never-edited).
    #   CORRECTION, 20260808 second pass: an earlier version of this comment also
    #   claimed "real vendor calls behind the tree's #if HAVE_xxx posture", copied
    #   from the PT-W2 three.  That is FALSE for myMN200motor.cpp -- its 124 mn_*
    #   calls are ACTIVE, and the link proved it the moment mymotor.cpp's MNetLog
    #   stub retirement caused the object to be extracted.  That is what
    #   Motor/vendor_offline_motionnet.cpp below exists for.
    # Two stub retirements were forced by these landing -- see Motor/mymotor.cpp:
    #   * `OpenPCI132Card(bool)` had an empty stub at Motor/mymotor.cpp:1378; the real
    #     295-line body is Motor/myMN200motor.cpp:1154 (golden myMN200motor.cpp:855).
    #   * `MyMNetLine myLine[MAXRing]` is DECLARED by Motor/myMN200motor.h and read/written
    #     at myMN200motor.cpp:1208/1209/2449/2451/2453, but golden defines it in
    #     Motor/mymotor.cpp:43 -- a region this tree's partial mymotor.cpp had not reached.
    Motor/myMN200motor.cpp
    Motor/mySYNTEKmotor.cpp
    Motor/myEthercatmotor.cpp
    # AI(W906-PT-W3-integrate) 20260808: the offline vendor LINK layer -- 173
    # MotionNet/MN200/PCI-L1xx entry points, every one of them the exact set some
    # registered TU in this build references (measured with nm --undefined-only,
    # not guessed).  NOT a translation of any golden unit; read its file banner
    # for the return-value semantics ("no card found", never SUCCESS) and for why
    # this only surfaced now.  Requires the global MN200DLL_VB defined at the top
    # of this file.  Delete this entry when a real MotionNet SDK is linked.
    Motor/vendor_offline_motionnet.cpp
    # AI(W906-PT-W4-integrate) 20260809: wave PT-W4's one motor unit, plus its own
    # offline vendor layer.  myGALILmotor.cpp is the ONLY unit in the tree that talks
    # to a Galil DMC-series controller; it funnels every access through 7 DMC32 entry
    # points which it keeps ACTIVE (most sit under `#ifndef SOFT_SIMULTE`, and this
    # build does not define SOFT_SIMULTE).  There is no MinGW-usable Galil import
    # library, so the 7 symbols get an offline layer for exactly the reason the
    # MotionNet one exists -- see vendor_offline_galil.cpp's banner.  Unlike MN200 it
    # needs NO global macro: DMCCOM.H declares them extern "C" __stdcall with no
    # dllimport, so callers already emit direct references.
    Motor/myGALILmotor.cpp
    Motor/vendor_offline_galil.cpp
    # AI(W906-PT-W5a-integrate) 20260809: the THIRD offline vendor layer -- CONTEC SMC.
    # Motor/mySMCmotor.cpp has been in this archive for waves carrying 46 unresolved
    # SmcW* references, and nothing broke because nothing referenced TMySMCMotor, so the
    # object was never extracted.  cinitial.cpp's newly translated
    # InitialMotorParameter() contains golden's driver ladder, one arm of which is
    # `new TMySMCMotor(iAdder)` -- the first consumer -- so the member now gets pulled
    # and those 46 become hard errors.  Measured with nm on the real archive before
    # writing the layer: exactly 46 distinct `_SmcW*@N`, zero defined in any of the 16
    # archives.  See vendor_offline_smc.cpp's banner for the return-value semantics
    # (non-zero == "the call failed", read off mySMCmotor.cpp:395-398's own `ret!=0`
    # test, because CSmc.h ships no error-code table to quote).
    Motor/vendor_offline_smc.cpp
)

target_include_directories(ht9045_motor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    # AI(W906-PT-W2) 20260807: the read-only vendor SDK headers.  This tree keeps
    # them in a vendor/ subdirectory rather than loose next to the ported sources
    # (golden has them loose in Motor/ and EtherCAT/) so that "never modify this"
    # is visible from the path alone -- AGENTS.md lists every one of these files
    # as forbidden-to-modify.  Putting the two directories on the include path is
    # what lets the ported sources keep golden's include lines BYTE-IDENTICAL
    # (`#include "CSmc.h"`, `#include "AdvMotApi.h"`) instead of carrying a
    # vendor/ prefix that golden does not have.
    # PUBLIC, not PRIVATE: Motor/Hontech_M4.h and Motor/mySMCmotor.h are public
    # headers of this archive and they are the ones doing the including.
    ${CMAKE_CURRENT_SOURCE_DIR}/Motor/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/EtherCAT/vendor
)

target_link_libraries(ht9045_motor PUBLIC vclcompat ht9045_globals)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_motor PRIVATE
        -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_io  (W4-IO HAL IO layer)
#
#   IOBackend.{h,cpp}   -- abstract port TIOBackend (WriteBit/WriteByte/ReadBit/
#                          ReadByte, int<0=fail) + TSimIOBackend (offline in-memory
#                          default, always active) + real backends TMN200Backend /
#                          TPci1203Backend / TMnetLegacyBackend / TRawPortBackend
#                          (bodies behind #if HAVE_MN200 / HAVE_PCI1203 / HAVE_MNET /
#                          (HAVE_RAWPORT && x86), all OFF by default).
#   MyLaneIo.{h,cpp}    -- TLaneIO facade: keeps the 6 BCB6 method names + signatures
#                          + OutPortData cache + CheckPortRangeErr + ret<0 / SOFT_SIMULTE
#                          bookkeeping; routes vendor calls through TIOBackend* pIO.
#                          Default: pIO = new TSimIOBackend() (offline).
#   myswitch.{h,cpp}    -- TMySwitch On/Off/Status/OnOff/CopySwitch; eMotionNet||
#                          ePCI1203 -> MyLaneIO (active); pad/raw branches gated
#                          #if 0 // TODO(W6).
#   mysensor.{h,cpp}    -- TMySensor Status/IsOn/IsOff/CopySensor via MyLaneIO;
#                          same pad gate; raw branch stub.
#
#   Sim path (default): compiles + links with NO vendor SDK; no HAVE_xxx macros
#   needed.  The real backends compile as stubs (each method {return 0;}) and
#   never link to vendor symbols.
#
#   Depends on:
#     * vclcompat       -- AnsiString / windows.h (byte)
#     * ht9045_globals  -- InitialOK, IO_CARD_TYPE, MyBitMask, MachineTypeChoice,
#                          uiDevhand, iPLCSafetyVer, eMotionNet/ePCI1203/etc.
#                          (cmydef.h / MachineType.h / cprod.h resolved here)
#
#   DEFER to W4-IO-part2 / W6 (NOT compiled now):
#     * TMyCylinder Push/Pop/On/Off/SetSimulateCompoment (Alarm/SystemStart/
#       bHandlerPause/TControl god-stack)
#     * TMySucker Suck()/Destroy() state machines (TQPF_Timer/alarm)
#     * Entire TMyKitSuck data grid (TALed/pLed/TMyProductionRecord/TTrayMotor/
#       cprod/MyDBIProcess)
#     * myio.cpp raw-port free funcs (outportb/IOSetOutport/TTL_CARD_TYPE +
#       2-arg IdleCheckSafeDoorByCylinder)
#     * Real backend wiring/selection in cinitial by IO_CARD_TYPE
#     * MyPLC_IO_Modbus/ModbusTCPClient PLC input state
# ---------------------------------------------------------------------------
add_library(ht9045_io STATIC
    IOBackend.cpp
    MyLaneIo.cpp
    myswitch.cpp
    mysensor.cpp
    # --- W6.0: TMyCylinder HAL piece (the one real substrate gap the W6.1
    #   Empty-tray canary needs).  The W4-IO note above DEFERRED TMyCylinder
    #   Push/Pop/On/Off to "W4-IO-part2 / W6"; this is that wave.  Routes over
    #   the Sim IO HAL (MyLaneIO) exactly like myswitch/mysensor; the alarm /
    #   SmartDiagnostic / VCL-widget god-stack is stubbed/gated in mycylin.cpp.
    mycylin.cpp
    # AI(W906-PT-W2) 20260807: golden myio.cpp -- the low-level port read/write
    # layer, translated whole in wave PT-W2.  It is NOT a duplicate of
    # MyLaneIo.cpp in this same list: MyLaneIo is TLaneIO (the multi-ring/IP/port
    # class), myio.cpp is the 12 free functions + globals that sit under it and
    # read TTL_CARD_TYPE / MyBitMask / bPLCIO directly.  See the myio.cpp banner
    # for how the two relate.
    myio.cpp
)

target_include_directories(ht9045_io PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    # AI(W906-PCI1203-IncludeFix) 20260818: IOBackend.cpp/MyLaneIo.cpp both
    # have had `#if HAVE_PCI1203 #include "AdvMotApi.h"` since the W4-IO wave,
    # but this target never had EtherCAT/vendor on its include path -- so the
    # bare include could never resolve the moment HAVE_PCI1203 was turned on.
    # Mirrors ht9045_motor's identical entry below (same vendor headers, same
    # uiDevhand master handle). Caught via a HAVE_PCI1203=1 probe build.
    ${CMAKE_CURRENT_SOURCE_DIR}/EtherCAT/vendor
)

target_link_libraries(ht9045_io PUBLIC vclcompat ht9045_globals)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_io PRIVATE
        -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_secsgem  (W5 SECSGEM base layer + Sim EventReport seam)
#
#   uHGemClass.{h,cpp}   -- HTGem: abstract SECS/GEM S,F-handler base (~90
#                           virtuals; the ~33 golden already left fully inline
#                           carry verbatim, the 57 that need THGem/uHGemEquipment
#                           (out of scope) are #if 0-gated with a conservative
#                           ACTIVE stub -- link-complete today).
#   SecsEventType.{h,cpp}-- SECS_EVENT (ETypeStruct): the CEID enum extracted
#                           standalone from uHGemHT9045.h, verbatim values.
#   SecsEventReport.{h,cpp} -- NEW (integrate-agent addition, not a golden
#                           translation): a minimal Sim-first free function
#                           `EventReport(unsigned Ceid)` that gives csystem.cpp's
#                           W7C1_EVENTREPORT_*/W7C2_EVENTREPORT_* seam macros a
#                           REAL, offline-safe call target (records last-CEID +
#                           a running count) instead of a no-op.  Deliberately
#                           does NOT instantiate HTGem -- see its file header:
#                           HTGem::~HTGem() needs a 3-arg MyDBIProcess overload
#                           that only tests/test_uHGemClass.cpp stubs today, and
#                           forcing every ht9045_sm-linked executable to resolve
#                           that too is not worth it for a Sim counter.
#
#   ht9045_sm links this library so csystem.cpp can call EventReport()/read
#   SECS_EVENT directly; only SecsEventType.o + SecsEventReport.o are ever
#   extracted from this archive for that link (uHGemClass.o stays a dormant,
#   unreferenced archive member there -- same "compiled but not pulled in"
#   shape as other inert files elsewhere in this tree). tests/test_uHGemClass
#   links this library directly to exercise HTGem itself.
#
#   SecsWireCodec.{h,cpp} -- W5-Final: THGem's pure byte-level SECS-II item
#     codec slice (GetLengthOfType/GetLengthByte/GetSMLLenthByte/DataItemOut
#     x2/DataItemInSub/DataItemIn x2/DataItemInNew/GetDataItemLenAndType(Sub)/
#     StringOut), plus the shared `HType`/HTypeStruct global (the canonical
#     home for HTypeStruct/STypeStruct/HSMS_Head_Struct -- see its own file
#     header). Zero dependency beyond vclcompat.
#   SecsSvEcRegistration.{h,cpp} -- W5-Final: THGem's SV/EC registration API
#     (SetSVDataPointer x4 / SetECDataPointer x4 / GetECDataValue). A
#     deliberately SEPARATE class from SecsWireCodec (reuses its `HType`
#     global via #include, does not extend it -- see that file's own
#     "DESIGN: SEPARATE CLASS" note); depends on SecsWireCodec.h + vclcompat/
#     TList.h (vclcompat::TList, used qualified -- never brought into the
#     global namespace, per that shim's own ODR note re aHotPlateSubstrate.h).
#   Both slices are prerequisites -- not yet consumers -- of a FUTURE full
#   THGem/uHGemEquipment.cpp translation wave (out of scope here); see each
#   file's own "INTEGRATE-AGENT WIRING POINT" note.
#
#   SECSGEM/uHGemEquipment.{h,cpp} -- W906-uHGemEquipment INTEGRATE: THGem's
#     FIRST real slice (a deliberately partial class -- see that header's own
#     file-head note for exactly what is/isn't modeled yet). This wave's slice
#     is the CEID/Report/Alarm StringGrid "database" family (SetCEIDContent/
#     GetCEIDContent/DeleteReportID/.../CopyStringGridAsTabFormat/SetAlamData/
#     ReadAlamData/WriteAlamData, 26 methods) + the standalone GemTimer utility.
#     Depends on vclcompat + the new vclcompat/StringGrid.h shim (see above) +
#     SECSGEM/SecsEventType.h (SECS_EVENT.TotalEvent array bound). Deliberately
#     does NOT yet embed SecsWireCodec/SecsSvEcRegistration (InitLocalHead/
#     DataItemOut/GetDataItemLenAndType/SetSVDataPointer/... are still OUT of
#     THGem's scope this wave -- a future wave should add them as by-value
#     members, matching HTGem's own already-proven WireCodec/SvEcReg pattern;
#     see uHGemEquipment.h's own "MEMBERS DELIBERATELY NOT PRESENT YET" note).
#     INTEGRATE CHECK (this wave): confirmed zero ODR overlap with
#     uHGemClass.cpp/SecsWireCodec.cpp's own GetDataItemLenAndTypeAndDelete(Sub)
#     -- uHGemEquipment.cpp does not define or call either; THGem does not yet
#     model that method at all. No production consumer wired yet (HTGem's
#     `THGem *HGemPtr` in uHGemClass.h stays a forward declaration -- see that
#     header's own comment on why an #include swap was evaluated and declined
#     this wave: every currently-gated HTGem method needs at least one THGem
#     member/method still absent from this partial slice, so switching to a
#     full #include would add a large new dependency for zero un-gating
#     benefit today).
#
#   AI(W906-uHGemEquipment-BucketB) 20260717: THIRD wave on this same file --
#     ~15 small widget stand-ins (THGemRadioGroup/THGemEdit/THGemCheckBox/
#     THGemComboBox/THGemPanel/THGemSpeedButton/THGemMemo, uHGemEquipment.h) +
#     InitialHGem/SaveSystemDefault/DoUpdateStatus/ProcessShow/FormClose/
#     ManualCreatergRoleClick/FormShow/StringOut(2-arg). DEPENDENCY WIDENING:
#     this slice needs CUSTOMER_CODE/CC_KYEC_LEE/CC_SIGURD_ChungXing/
#     CC_MAXIM_THAILAND/CosFunction/bSECSGEMbyPass/bSECSGEMConnectionFail
#     (cmydef.h -> ht9045_globals) and ReadWriteIni/ReadIniData/WriteIniData
#     (common.h -> ht9045_core) -- see uHGemEquipment.cpp's own top-of-file
#     note for the full reasoning (common.h confirmed to stay lightweight:
#     ini-helper family only, does not itself drag in the rest of golden's
#     common.h). Same "translated unit needs the global/ini-helper families"
#     shape already established by KYECFTP/FTPClient_Transfer.cpp's own
#     ht9045_core+ht9045_globals link (see that library's own comment above).
#     No circular dependency: ht9045_core/ht9045_globals each only link
#     vclcompat (+psapi/version for ht9045_globals on WIN32), neither links
#     back to ht9045_secsgem or ht9045_public.
# ---------------------------------------------------------------------------
add_library(ht9045_secsgem
    SECSGEM/uHGemClass.cpp
    SECSGEM/SecsEventType.cpp
    SECSGEM/SecsEventReport.cpp
    SECSGEM/SecsWireCodec.cpp
    SECSGEM/SecsSvEcRegistration.cpp
    SECSGEM/uHGemEquipment.cpp
    # AI(W906-SvEcDataItem) 20260720: TasmInfo -- THGem::FormCreate's SV10-19
    # (CPU/disk/memory) source functions. New, standalone (vclcompat only),
    # zero new link edge (same archive as uHGemEquipment.cpp, which calls it).
    SECSGEM/TasmInfo.cpp
    # AI(W906-uHGemHT9045-Bucket0) 20260721: HT9045Gem's permanent home
    # (supersedes the deleted uHGemHT9045_Shim.cpp, W906-SysModWire) -- the
    # ONLY definition of `HT9045Gem`, needed by database.cpp's now-real
    # SystemModularInitial (`new HT9045Gem(...)`). See that header's own
    # file-head note for scope (still zero of golden's 22 overrides).
    SECSGEM/uHGemHT9045.cpp
    # AI(W906-PT-W4-integrate) 20260809 CORRECTION, second pass: PT-W4's two SECS/GEM
    # registration tables (uHGemHT9045_SV.cpp / _EC.cpp) were briefly registered HERE
    # on the reasoning that they are "the data half of the same object" as
    # uHGemHT9045.cpp.  THAT WAS WRONG, and uHGemHT9045_SV.cpp:333-345 says so in its
    # own title: "CMakeLists PLACEMENT -- READ BEFORE REGISTERING THIS FILE. It must go
    # in ht9045_sm, NOT in ht9045_secsgem."  Both units take the address of objects
    # defined by ht9045_sm members, and ht9045_secsgem deliberately does NOT link
    # ht9045_sm (that edge is the documented CMake cycle, see :579-616).  Measured, not
    # taken on trust -- nm --undefined-only on each object against every archive:
    #     _SV.o : 5 symbols resolved only by libht9045_sm.a (ArmData/ArmHistory 226
    #             references, iLoadStateATK, sOutputBinCode, iPortStatus)
    #     _EC.o : 4 symbols resolved only by libht9045_sm.a (iThisPortNo, ...)
    # Registered in ht9045_sm instead -- see the PT-W4 block in that library.
    # WHY IT LOOKED FINE: it configured, compiled, and the whole build stayed rc=0,
    # because nothing referenced either object so neither was ever extracted from the
    # archive.  The archive-extraction trap again; a placement error here is silent.
)

target_include_directories(ht9045_secsgem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/SECSGEM
)

# AI(W906-uHGemEquipment-ConnLifecycle) 20260717: THGem::GetSocketErrorMsg
# (uHGemEquipment.cpp, golden :6842-6845) is a thin forward to the already-real
# GetErrorMsg(TObject*, int) -- its one implementation is
# Public/WinSocketErrorCode.cpp, which lives in the ht9045_public library (see
# that library's own target above). GetErrorMsg only reads <winsock2.h>
# constants (no WSA*/socket function calls -- see that .cpp's own header
# note), so this adds no further transitive link requirement (no ws2_32 etc.).
# AI(W906-uHGemEquipment-BucketB) 20260717: ht9045_core (common.h's
# ReadWriteIni/ReadIniData/WriteIniData) + ht9045_globals (cmydef.h's
# CUSTOMER_CODE/CC_*/CosFunction/bSECSGEMbyPass/bSECSGEMConnectionFail) added
# -- see this target's own comment above for the full reasoning.
# AI(W906-uHGemEquipment-BucketC) 20260717: ht9045_db added -- D3's HSys.MyGem
# seam (`#include "database.h"`, `extern SYSTEM_MODULAR HSys;`) needs
# database.cpp's one definition of `HSys` linked in. ht9045_db's OWN
# target_link_libraries declaration only lists vclcompat+ht9045_globals (see
# that library's own target above) -- no CMake-target-graph cycle.
#
# AI(W906-SysModWire) 20260720: a real (object-file-level, not CMake-target-
# level) circular SYMBOL dependency now exists as of this wave: database.cpp's
# SystemModularInitial calls `new HT9045Gem(...)`, a symbol this library
# (ht9045_secsgem) defines (uHGemHT9045.cpp.o as of W906-uHGemHT9045-Bucket0,
# 20260721; was uHGemHT9045_Shim.cpp.o before the swap-back) -- so ht9045_db's
# database.cpp.o now has an undefined reference back INTO this archive, even
# though ht9045_db's CMake target never declares ht9045_secsgem as a
# dependency. Any executable that ends up needing BOTH archives (this one,
# for HSys via ht9045_db) must link them through a `$<LINK_GROUP:RESCAN,...>`
# (or equivalent --start-group/--end-group) wrapper, not a plain sequential
# link -- see tests/CMakeLists.txt's test_uHGemEquipment/test_uHGemClass/
# test_config_loaders (the three targets that actually pull database.cpp.o;
# independent review verified via nm that only these executables contain
# _HSys -- the motor/io targets linking ht9045_db never pull database.cpp.o,
# so they need no ht9045_secsgem edge) for the applied fix.
# Deliberately NOT making ht9045_db declare
# `target_link_libraries(ht9045_db PUBLIC ht9045_secsgem)` here -- that would
# invert this library's intended layering (ht9045_db is a lower layer than
# SECS/GEM) for a dependency that is a database.cpp implementation detail, not
# ht9045_db's own public interface.
# AI(W906-W7-F0) 20260728: ht9045_forms ADDED -- this single edit is what
# actually unblocks "Wall 2".  SECSGEM/uHGemHT9045.cpp touches 28 distinct fMain
# members in golden; it could not reach ANY of them before, because the facade
# lived in ht9045_sm and adding ht9045_sm here would have made a CMake cycle
# (ht9045_sm already links this target -- see target_link_libraries(ht9045_sm
# PUBLIC ...) below).  ht9045_forms is a bottom layer (vclcompat +
# ht9045_globals only), so this edge is acyclic.
# AI(W906-W7-F0-fix) 20260728: that acyclicity now has a POSITIVE control, not
# just "the configure succeeded" -- re-injecting the ht9045_secsgem->ht9045_sm
# edge into this exact tree makes configure FAIL with a strongly-connected-
# component error on test_config_loaders and test_uHGemEquipment, while the tree
# without it configures clean.  Both runs are recorded in the ht9045_forms block
# above ("THE EXACT MECHANISM"), including why a cycle among STATIC libraries is
# normally legal and why THIS one is not.
target_link_libraries(ht9045_secsgem PUBLIC vclcompat ht9045_public ht9045_core ht9045_globals ht9045_db ht9045_forms)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_secsgem PRIVATE ${HT9045_WARNFLAGS})

    # AI(W5-SECSGEM-Integrate) 20260711: uHGemClass.h/.cpp carry 2 golden BCB6
    # idioms (an intentionally-unused virtual-override parameter; a trailing
    # ';' after a ctor body) -- same class of carve-out already accepted for
    # cContact.cpp. Narrowly suppress just these two warning classes on this
    # file; every other warning class (-Wall -Wextra -Wshadow -Wconversion)
    # still applies in full.
    set_source_files_properties(SECSGEM/uHGemClass.cpp PROPERTIES COMPILE_FLAGS
        "-Wno-pedantic -Wno-unused-parameter")
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_automation  (W5 Automation wave -- RENESAS host-protocol data shapes)
#
#   Automation/uRENESAS_Other.{h,cpp} -- ~22 plain-old-data record/reply classes
#     (TRENESAS_OnLine/_ProdInfo/_ModeStart/_HandlerStart/_ModeEnd/_LotEnd/_TestEnd/
#     _SendAlarm/_SendCleanOut + their _Reply counterparts + the aggregate
#     TRENESAS_RECV_CMD_DATA/_Send_CMD_DATA containers), the TRENESAS_CMD /
#     TRENESAS_Status_Code / TRENESAS_Err_Index command/status/error-code constant
#     tables, and TRENESAS_EndFileData (the only class with real logic). Zero
#     hardware/VCL/global coupling -- pure data shapes prerequisite for the FUTURE
#     Automation/uRENESAS_Server.cpp wave (not yet scheduled, not yet a consumer),
#     so this gets its own small standalone library (matching the unit's own
#     translate report) rather than being folded into ht9045_sm's larger link
#     surface for no reason.
# ---------------------------------------------------------------------------
add_library(ht9045_automation STATIC
    Automation/uRENESAS_Other.cpp
)

target_include_directories(ht9045_automation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_automation PUBLIC vclcompat)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_automation PRIVATE ${HT9045_WARNFLAGS})
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_comms  (W5 comms wave -- CCLink / CanBus / MyPLC / EtherCAT)
#
#   CCLink/MyCCLink.{h,cpp}       -- TMyCCLinkCtrl (top-level comm state machine)
#   CCLink/MyCCLinkBoard.{h,cpp}  -- TMyCCLinkBoard (Mdfunc vendor board wrapper;
#                                    real vendor calls behind #if HAVE_CCLINK,
#                                    default OFF -- mirrors IOBackend.cpp's
#                                    HAVE_MN200/HAVE_PCI1203/HAVE_MNET).
#   CCLink/MyCCLinkSensor_predicates.{h,cpp} -- UseCanBusOrEtherCAT/UseSensorMod3.
#   CanBus/cMyNUDN1.{h,cpp}       -- TMyNUDN1 (NUDN1 DeviceNet amplifier station
#                                    wrapper); real vendor calls behind
#                                    #if HAVE_DNM100, default OFF.
#   MyPLC/ModbusTCPClient.{h,cpp} -- Modbus-TCP-ish framing client over
#                                    vclcompat/ClientSocket.h (above).
#   MyPLC/MyPLC_IO_Modbus.{h,cpp} -- PLC safety-IO polling (InitPLCIO/
#                                    PLCIOTaskCycle/PLCStatusCheck) + TPLCIOThread
#                                    (offline-inert driver, no real OS thread).
#   EtherCAT/MyNUEC1.{h,cpp}      -- TMyNUEC1 (FS-N12 EtherCAT current-sense
#                                    amplifier slave); real vendor calls behind
#                                    #if HAVE_PCI1203 (REUSES the flag
#                                    IOBackend.cpp/MyLaneIo.cpp already
#                                    established for the same Advantech API
#                                    family), default OFF.
#
#   Interface/InterfaceSYS.cpp is DELIBERATELY NOT part of this library: its
#   own translate report found it both depends on (TestSocket/LastSet/
#   FormsFacade) and unblocks (atester.cpp's GetTesterResult/ProcessTesterTimeOut)
#   symbols that live in ht9045_sm -- making it a separate library here would
#   create a circular ht9045_comms<->ht9045_sm CMake target dependency (CMake
#   rejects cycles in the target link graph, unlike the linker's own archive
#   re-scanning). It is instead compiled directly into ht9045_sm's own source
#   list (see that library below) -- the integrate agent's own resolution of
#   this unit's flagged conflict.
#
#   No cross-unit archive-selection conflicts: the four units here never
#   reference each other's symbols (verified by grep during integrate); each
#   unit's own dedicated test below links this whole library, and only that
#   unit's own translation units are ever extracted from the archive.
# ---------------------------------------------------------------------------
add_library(ht9045_comms STATIC
    CCLink/MyCCLink.cpp
    CCLink/MyCCLinkBoard.cpp
    CCLink/MyCCLinkSensor_predicates.cpp
    CanBus/cMyNUDN1.cpp
    MyPLC/ModbusTCPClient.cpp
    MyPLC/MyPLC_IO_Modbus.cpp
    # AI(W906-PT-W3-integrate) 20260808: EtherCAT/MyNUEC1.cpp MOVED OUT of this
    #   archive to ht9045_motor -- see the note beside it there.  Short version:
    #   its only consumer tree-wide is EtherCAT/MyEtherCAT.cpp, which already
    #   sits in ht9045_motor for exactly this reason, and ht9045_motor has no
    #   link edge to ht9045_comms (nor should it gain one -- ht9045_sm links
    #   both, so the edge would only exist to reach one class).
    # AI(W906-PT-W2) 20260807: wave PT-W2's protocol/fieldbus units.  Each one
    # talks to a device over a wire and reaches nothing above this archive.
    #   CanBus/cMyDNM100UD.cpp  -- 泓格 DNM100UD CAN SDK wrapper; joins its
    #       sibling CanBus/cMyNUDN1.cpp already in this list.
    #   ATC/TCPData.cpp, ATC/ATC_WinWay.cpp -- ATC handler-side wire structs.
    #       Member order and sizes are protocol-bearing, kept exactly as golden.
    #   EJ1N/* -- the Omron EJ1N temperature-controller family.  They build on
    #       EJ1N/TextProcess.cpp, which landed in ht9045_globals last wave and
    #       which this archive already PUBLIC-links.
    CanBus/cMyDNM100UD.cpp
    ATC/TCPData.cpp
    ATC/ATC_WinWay.cpp
    EJ1N/uSocketServerClient.cpp
    EJ1N/uModbusCommand.cpp
    EJ1N/uDTME08Control.cpp
    EJ1N/MyOmronPanel.cpp
    # AI(W906-PT-W3-integrate) 20260808: ATC/ATCSystem.cpp joins its own family --
    # ATC/TCPData.cpp (the wire structs it sends) and ATC/ATC_WinWay.cpp are already
    # in this list.  It is the ATC 3.0/6.0 TCP protocol engine (command assembly,
    # reply dispatch, per-channel timeout ladder); it talks to a device over a wire
    # and reaches nothing above this archive.  The ATC UI form that DRIVES it
    # (ATC/ATCInterface.cpp) does reach higher and lands in ht9045_sm instead.
    ATC/ATCSystem.cpp
)

target_include_directories(ht9045_comms PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_comms PUBLIC vclcompat ht9045_globals)

if(WIN32)
    target_link_libraries(ht9045_comms PUBLIC ws2_32)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # BCB6-idiom-heavy new comms code (narrowing int->short/byte at vendor call
    # boundaries, empty golden event handlers, etc.) -- match the ht9045_io /
    # ht9045_motor relaxed posture rather than ht9045_core's strict one (none of
    # these files is the "genuinely idiom-free" case that earned cContact.cpp
    # its stricter flags).
    target_compile_options(ht9045_comms PRIVATE
        -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_sm  (W6 state-machine wave -- W6.1 Empty-tray CANARY)
#
#   The first ROOT state machine translated over the W6 substrate, plus the
#   W6 decoupling scaffold that lets it stand alone:
#
#     asendic_Empty.cpp     -- the Empty-tray stack-feed SM (CANARY).  Faithful
#                              switch(Task) steppers; each Do* owns its cursor int.
#     csystem_predicates.cpp-- the frozen csystem.h HasIC predicate interface
#                              (implemented over the Sim HAL; canary uses none,
#                              but they link the contract for later sub-waves).
#     (FormsFacade.cpp)     -- WAS here: the non-VCL fAGV satellite stub (the
#                              canary's only form-pointer deref,
#                              fAGV->IsATK_AMR()).  AI(W906-W7-F0) 20260728
#                              moved it to forms/fAGV.cpp in the new
#                              ht9045_forms library (see its block above);
#                              this target links ht9045_forms, so the canary is
#                              unaffected.
#     asendic.cpp           -- SIM bodies for CylinderUp/Middle/Lower + TrayMove*
#                              tray-move/lifter helpers (manipulate Cylinder[]/
#                              MOT[]; full lifter SM deferred W6.x).
#     canary_support.cpp    -- minimal LastSet shim + sim bodies for the free
#                              functions the canary calls that have no shared
#                              target header (ShowErrorMessage/RecordProcess/
#                              ShowUnloaderTrayMessage/WhichAutoNeedTray/
#                              ShowMyMessage/ReadWriteTrayID).
#
#   Depends on the full substrate: ht9045_io (Cylinder[]/SW[]/Sen[]/MyLaneIO),
#   ht9045_motor (MOT[]), ht9045_globals (Prod/IniConfig/CosFunction/TrayForm/
#   RunInfo/Ld_UldDelayTime/TrayID/enums), ht9045_db (database.cpp -- pulled by
#   the RESCAN group for cross-archive ctor resolution), vclcompat.
# ---------------------------------------------------------------------------
add_library(ht9045_sm STATIC
    asendic_Empty.cpp
    # AI(W906-W7-L1) 20260729: asendic_Auto2.cpp -- the Auto2-tray stack-feed SM,
    # first landing of the W7-L1 asendic_* feed-SM family (see plan SS6 W7-L1).
    # Verified zero outgoing cross-file calls into the other 6 asendic_* siblings
    # (Loader/Loader_RT/Scanner/Auto/Auto_RT/Color), so it is safe to land
    # standalone in the same library asendic_Empty.cpp already occupies (same
    # substrate: Cylinder[]/MOT[]/Sen[]/SW[]/canary_support/FormsFacade fAGV).
    asendic_Auto2.cpp
    # AI(W906-W7-L1-Wave1) 20260802: the W7-L1 Wave-1 landings.  All three join the
    # same library and the same Sim HAL substrate asendic_Empty.cpp / asendic_Auto2.cpp
    # already occupy (Cylinder[]/MOT[]/Sen[]/SW[] + canary_support + FormsFacade +
    # acatchtray_shims); NO new link dependency was introduced by either.
    #
    #   asendic_Color.cpp      -- the Color-tray stack-feed SM.  Verified by two
    #     independent recons and re-checked by its own wave: ZERO outgoing calls into
    #     any asendic_* sibling, so it is the family's only true leaf.  It is also the
    #     PREREQUISITE for Wave 2's asendic_Auto.cpp, which calls this file's
    #     ForTERAPOWERCheckColorSensor (golden asendic_Auto.cpp:363/:2029/:2074).
    #
    #   asendic_Loader.cpp + asendic_Loader_RT.cpp -- the Loader bundle.  These two
    #     MUST land together: they are a genuine bidirectional cycle (golden
    #     asendic_Loader.cpp:1973/:1987 call into _RT, and _RT.cpp:190 calls back into
    #     asendic_Loader.cpp:3167 CheckLoaderICFloating), so neither compiles as a
    #     standalone unit of work.
    #
    # LANDED ATOMICALLY WITH FIVE SHIM-BODY RETIREMENTS (see the SHIM RETIREMENT DEBT
    # REGISTER banners in acatchtray_shims.h and csystem_shims.h, where these are now
    # marked DONE): acatchtray_shims DoAutoColor + InitAutoColorReceiveTask, and
    # csystem_shims InitAutoColorTask + DoLoad + InitLoadTask.  Earlier would have
    # given undefined symbols, later duplicate symbols.  DoAutoColor's DECLARATION was
    # additionally corrected bool -> void, which is what golden says
    # (golden asendic_Color.h:20); all ported call sites discarded the value.
    asendic_Color.cpp
    asendic_Loader.cpp
    asendic_Loader_RT.cpp
    # AI(W906-W7-L1-Wave2) 20260802: the W7-L1 Wave-2 landings -- the LAST two SM
    # files of the asendic_* family.  Same library and Sim HAL substrate as the five
    # units above; no new link dependency.
    #
    #   asendic_Auto.cpp     -- the Auto-tray (automatic sort-output) stack-feed SM
    #     for Auto1..Auto6.  2,561 golden lines, of which ONE function
    #     (DoAutoReceiveBinTray) is 2,189 lines across 56 cases.  ORDER MATTERS: it
    #     must follow asendic_Color.cpp, whose ForTERAPOWERCheckColorSensor it calls
    #     (golden asendic_Auto.cpp:363/:2029/:2074).
    #
    #   asendic_Auto_RT.cpp  -- the Auto1..Auto6 unloader-tray ROUND-TRIP
    #     (auto-retest) variant.  Lands standalone: zero facade members and zero shim
    #     retirements of its own (proven with `nm -g --defined-only` over all ten
    #     archives).  It also SUPPLIES the two definitions that asendic_Auto2.h and
    #     asendic_Auto.h already extern-declare as "owned by asendic_Auto_RT.cpp"
    #     (InitUnLoadNewAutoTrayTask / DoUnLoadNewAutoToStack), so those stop being
    #     orphan declarations.
    #
    # LANDED ATOMICALLY WITH the Auto track's two shim-body retirements
    # (acatchtray_shims DoAutoReceiveBinTray, csystem_shims Initial_Auto_BinTray_Task),
    # its bool -> void declaration correction, and the TfTrayMapping::DoCoverTrayID
    # signature fix -- see the DEBT REGISTER banners.  That last one changes a mangled
    # name, so this landing needs a FULL rebuild, not an incremental one.
    asendic_Auto.cpp
    asendic_Auto_RT.cpp
    csystem_predicates.cpp
    # AI(W906-W7-F0) 20260728: FormsFacade.cpp REMOVED from this library -- the
    # facade now compiles into the bottom-layer ht9045_forms target (see its
    # block above for why: having it here was the actual "Wall 2" CMake cycle).
    # ht9045_sm links ht9045_forms below, so nothing in this library loses access
    # to fMain/fLotInfo/... .
    asendic.cpp
    canary_support.cpp
    cinitial.cpp          # AI(W906-GA2-C1) 20260804: block 1 (golden :77-3060) -- Sucker/Switch/Sensor names+init
    # --- W6.2 (IN-ARM foundation): shared HotPlate pick/place geometry leaves ---
    #   ainarm_SearchPickPlate.cpp  -- HP-pick geometry + DoInArmPickFromHotPlate_9045 SM
    #   ainarm_SearchPlacePlate.cpp -- HP-place geometry + search-planner + data-swap leaves
    #   aHotPlateSubstrate.cpp      -- the W6.2 in-arm substrate the leaves pump
    #                                  (InArmSuck/FLCarryKit/.. TMyKitSuck grid +
    #                                   PickFromHPList uPlateInfo + ainarm2 cursors +
    #                                   in-arm engine helper stubs + tRotate/Laser)
    ainarm_SearchPickPlate.cpp
    ainarm_SearchPlacePlate.cpp
    aHotPlateSubstrate.cpp  mykitsuck.cpp  # AI(W906-A4-6) 20260924: 吸嘴類別合一 —— golden 佈局的 TMySucker／TMyKitSuck（mykitsuck.h／.cpp）取代 aHotPlateSubstrate 的精簡鏡像；見 :2723
    # AI(W906-Reconcile) 20260721: Public/MyProductionRecord.cpp's real pure-logic
    # bodies (ctor + ~28 leaves) supersede the offline no-op stand-ins
    # aHotPlateSubstrate.cpp used to carry for the same 8 methods (removed there,
    # see that file's own comment) -- lives in ht9045_sm because that's what
    # aHotPlateSubstrate.cpp/AutoClean.cpp (both in this target) call into.
    Public/MyProductionRecord.cpp
    # --- W5-comms INTEGRATE: Interface/InterfaceSYS.cpp (WM_COPYDATA IPC bridge
    #     to the GPIB/Tester bridge / ESD / Auto-Update / Event-Log-Analyzer
    #     helper programs).  Lives HERE, not in ht9045_comms, per that unit's own
    #     translate report: it depends on TestSocket/LastSet (aHotPlateSubstrate.
    #     cpp/canary_support.cpp, just above) + fMain/fBarCode (was
    #     FormsFacade.cpp here; AI(W906-W7-F0) 20260728 moved it to
    #     forms/fMain.cpp in ht9045_forms, which this target links)
    #     and in turn unblocks atester.cpp's still-gated GetTesterResult/
    #     ProcessTesterTimeOut (both also in this library) -- a standalone
    #     library here would create a circular ht9045_comms<->ht9045_sm target
    #     dependency.  See CMakeLists.txt's ht9045_comms comment block.
    Interface/InterfaceSYS.cpp
    # --- W5-Final-TesterTCPSocket INTEGRATE: Interface/TesterTCP_Socket.cpp
    #     (bounded socket-management subset of Interface/TesterTCP.cpp --
    #     Connect/Disconnect/Error/Read handlers, the 30-tick reconnect timer,
    #     SendTCPIPCommand/AddTCPIPCommunicationLog). Same rationale as
    #     InterfaceSYS.cpp just above: compiled directly into ht9045_sm (not a
    #     separate library) since it depends on FormsFacade/canary_support/
    #     common already carried here. Depends on vclcompat/ClientSocket.h's
    #     TClientSocket/TCustomWinSocket/TErrorEvent shim (already present on
    #     disk from an earlier front in this integrate sequence) + the
    #     StringReplace/TReplaceFlags addition this unit made to
    #     vclcompat/SysUtils.h/.cpp (additive only, no hard-boundary file touched).
    Interface/TesterTCP_Socket.cpp
    # --- W6.2b (IN-ARM engine): the central pick SM + iInArmType dispatch
    #     ladders + pure pitch/offset geometry + 3 pumpable task SMs.  Variant
    #     dispatch bodies + dense MOT[]/sensor/UI SMs are gated (#if 0) with
    #     active stubs; see the file-head gate documentation.
    ainarm9045.cpp
    # --- W7-A1 INTEGRATE: offline link stubs for the shuttle-motion engine seam
    #     symbols the un-gated in-arm floating/latch PAIR references (golden
    #     ainarm2.h: DoShakeShuttle/DoKnockShuttle/DoKnockShuttleFirst/
    #     DoVibrateShuttle/RecordShtSuperflous + IndexZCanMove[2]/bShuttleKnock).
    #     IsTestZ1/Z2NotSafe... are NOT here (defined in acarry.cpp).
    ainarm9045_w7_shims.cpp
    # --- W6.2b-canary (IN-ARM site variants): 3 per-site-config variant files
    #     translated faithfully from the golden BCB6 tree ROOT (mirror name/path).
    #       ainarm9045_1x1_1.cpp -- single-site 1x1_1 (THE buildable, live arm;
    #                               defines DoInArm_9045_1x1_1 + _SuckerMap, called
    #                               by the un-gated dispatch arm in ainarm9045.cpp).
    #       ainarm9045_2x4_16.cpp + _shims -- 16-site variant; faithful translation
    #                               but DEAD by the golden engine design (no
    #                               e9045_2x4_16 enum, no dispatch arm; references a
    #                               file-local NN-site map -- golden member absent by
    #                               design).  Registered so it compiles/links; never
    #                               dispatched.
    #       ainarm9045_2x8_32.cpp -- 32-site variant; whole golden body GATED (needs
    #                               Prod.fInArmSuckUse_TwoArm32Site, absent from
    #                               golden cprod.h by design) + the golden engine
    #                               routes e9045_2x8_32 -> 2x8_8 (DEAD).  Active
    #                               offline stubs make the TU link; never dispatched.
    ainarm9045_1x1_1.cpp
    ainarm9045_2x4_16.cpp
    ainarm9045_2x4_16_shims.cpp
    ainarm9045_2x8_32.cpp
    # --- W6.2c batch-1 (5 IN-ARM site variants made ACTIVE; dispatch arms pulled
    #     out of the "#if 0 // TODO(W6.2b variants)" gate in ainarm9045.cpp):
    ainarm9045_1x3_4.cpp
    ainarm9045_1x2_2.cpp
    ainarm9045_1x2_2_14.cpp
    ainarm9045_1x2_4_Hot.cpp
    ainarm9045_1x3_2_14.cpp
    # --- W6.2c batch-2 (6 IN-ARM site variants made ACTIVE; dispatch arms pulled
    #     out of the "#if 0 // TODO(W6.2b variants)" gate in ainarm9045.cpp).
    #     2x2_4 / 2x2_4_14 also drove the substrate add of TMyKitSuck::
    #     ArmDownSideAllTypeIC (golden MyKitSuck.h:335 -> aHotPlateSubstrate).
    ainarm9045_1x4_2.cpp
    ainarm9045_1x4_8_Hot.cpp
    ainarm9045_2x2_4_12.cpp
    ainarm9045_2x1_2.cpp
    ainarm9045_2x2_4.cpp
    ainarm9045_2x2_4_14.cpp
    # --- W6.2c batch-3 (6 IN-ARM site variants made ACTIVE; dispatch arms pulled
    #     out of the "#if 0 // TODO(W6.2b variants)" gate in ainarm9045.cpp).
    #     1x4_4 also drove the substrate add of cSiteUseManager (SiteUseMgr) +
    #     RecordErrorLog (see substrate note).  All_1Pick's DoInArm arm is the
    #     FIRST ladder arm (USE_PICKER_COUNT==ep1Picker), and its *_SuckerMap()
    #     is declared-but-NEVER-dispatched by the golden engine (faithful).
    ainarm9045_1x4_4.cpp
    ainarm9045_1x4_4_Back.cpp
    ainarm9045_2x3_6.cpp
    ainarm9045_2x3_6_14.cpp
    ainarm9045_2x4_4.cpp
    ainarm9045_All_1Pick.cpp
    cSiteUseManager.cpp
    # --- W6.2c batch-4 (5 IN-ARM site variants made ACTIVE; dispatch arms pulled
    #     out of the "#if 0 // TODO(W6.2b variants)" gate in ainarm9045.cpp).
    #     2x2_8_Hot drove the substrate add of TMyKitSuck::SetNullIcToHasNullIc
    #     (golden MyKitSuck.h:280/.cpp:340) + the engine predicate RowCanDualSite
    #     (golden ainarm2.cpp:1554, offline false).  2x8_8 drove the offline homes
    #     of IsMoveInArm2XYToShuttle2Wait (golden ainarm2.h:142 -> acatchtray_shims)
    #     + XPHSuckToSht_2x8_8_OutArm (golden aoutarm9045_2x8_8.h:7 -> substrate).
    #     The golden engine routes e9045_2x8_32 -> DoInArm_9045_2x8_8 (the LIVE
    #     family), so the DEAD ainarm9045_2x8_32.cpp (line 419) stays compiled-but-
    #     never-dispatched -- do NOT remove it.  iCloseSiteModeFor2x6/2x8 +
    #     iCloseSiteStep_2x8 placeholder definitions in aHotPlateSubstrate.cpp /
    #     acatchtray_shims.cpp were removed (the variant .cpp own the real defs).
    ainarm9045_2x2_8_Hot.cpp
    ainarm9045_2x4_8.cpp
    ainarm9045_2x5_8.cpp
    ainarm9045_2x6_8.cpp
    ainarm9045_2x8_8.cpp
    # --- W6.2c batch-5 (2 S-family HT-9045S in-arm variants made ACTIVE; dispatch
    #     arms un-gated in BOTH ladders of ainarm9045.cpp).  IRREGULAR func names:
    #     ainarm9045S_1x4_4.cpp defines DoInArm_9045S_1x4_4(+_SuckerMap) (S in func);
    #     ainarm9045S_2x4_4_13.cpp defines DoInArm_9045_2x4_4_13(+_SuckerMap) (NO S).
    ainarm9045S_1x4_4.cpp
    ainarm9045S_2x4_4_13.cpp
    # --- W6.2b-canary (IN-ARM shared helpers): the two ainarm2.cpp leaves the
    #     site-variant SMs call but that had no body yet -- SetInArmUseSuckToHas
    #     NullIC (golden ainarm2.cpp:201) + InitInArmPickFromHotPlateTask (golden
    #     :623), FAITHFUL pure-grid/cursor logic over the Sim HAL.  Also hosts the
    #     single ACTIVE definition of bDestoryOnSht (cmydef.cpp's is gated #if 0).
    ainarm2.cpp
    # --- W6.2c (OUT-ARM engine): the central place-to-Auto SM + iInArmType
    #     dispatch ladders (DoOutArm_9045 / DoPickFromShuttle_9045) + pure
    #     pitch/offset geometry + 4 pumpable task SMs (DoOutArmAdditionalFunction /
    #     DoOutArmPlaceToAuto_9045 / DoOutArmAfterPlaceToAuto / DoOutArmIonFanGiveWay).
    #     Variant dispatch bodies + dense MOT[]/sensor/Magazine/AOI/Rotate SMs are
    #     gated (#if 0) with active stubs; see the file-head gate documentation.
    #     Pumps the same aHotPlateSubstrate (OutArmSuck/FRCarryKit/BRCarryKit/
    #     OutArm2Suck + out-arm cursor ints) as the in-arm engine.
    aoutarm9045.cpp
    # --- W6.2c-OUT batch (6 OUT-ARM site variants made ACTIVE; dispatch arms
    #     pulled out of the "#if 0 // TODO(W6.2b variants)" gate in aoutarm9045.cpp
    #     + their 2 static no-op stubs removed).  All 6 route to a LIVE callee per
    #     the golden ENUM->CALLEE map.  These drove the substrate adds of
    #     TMySucker::GetNeedSuckStatus/SetNeedSuck/bNeedSuck (golden MyKitSuck.h:
    #     119/121/123) + TMyKitSuck::SetUnuseAndHasNullICToNullIC (golden
    #     MyKitSuck.cpp:369) in aHotPlateSubstrate.{h,cpp}.
    aoutarm9045_1x1_1.cpp
    aoutarm9045_1x2_2.cpp
    aoutarm9045_1x2_4.cpp
    aoutarm9045_1x3_2_14.cpp
    aoutarm9045_1x3_4.cpp
    aoutarm9045_1x4_2.cpp
    # --- W6.2c-OUT batch-2 (7 more OUT-ARM site variants made ACTIVE; LIVE mode):
    #     1x4_4S/1x4_4/1x4_8/2x1_2/2x2_4/2x2_8/2x3_6_14.  Their dispatch arms were
    #     already active in aoutarm9045.cpp; here their 2 static no-op stubs are
    #     removed + their 2 extern decls un-gated.  All route to a LIVE callee per
    #     the golden ENUM->CALLEE map.  Drove the substrate add of CheckDuplicateBarCode
    #     (golden aoutarm.h:130, offline true) in aoutarm_shims.cpp -- used by 1x4_4.
    aoutarm9045_1x4_4S.cpp
    aoutarm9045_1x4_4.cpp
    aoutarm9045_1x4_8.cpp
    aoutarm9045_2x1_2.cpp
    aoutarm9045_2x2_4.cpp
    aoutarm9045_2x2_8.cpp
    aoutarm9045_2x3_6_14.cpp
    # --- W6.2c-OUT batch-3 (FINAL 7 OUT-ARM site variants made ACTIVE; LIVE mode):
    #     2x3_6/2x4_4/2x4_8/2x5_8/2x6_8/2x8_8/All_1Picker.  This empties BOTH the
    #     gated-extern and static-stub blocks in aoutarm9045.cpp -- all out-arm
    #     variants now resolve to real translated funcs.  Drove the substrate add of
    #     TMyKitSuck::HasRealIC_Left/Right (golden MyKitSuck.h:294-295) used by
    #     2x5_8/2x6_8/2x8_8, and the REMOVAL of the temp MoveOutArmToShuttleIncludeZ_
    #     9045_2x4_4 stub (now defined for real in aoutarm9045_2x4_4.cpp).
    #     All_1Picker is DoOutArm-ONLY (picker-gated; no DoPickFromShuttle callee).
    aoutarm9045_2x3_6.cpp
    aoutarm9045_2x4_4.cpp
    aoutarm9045_2x4_8.cpp
    aoutarm9045_2x5_8.cpp
    aoutarm9045_2x6_8.cpp
    aoutarm9045_2x8_8.cpp
    aoutarm9045_All_1Picker.cpp
    # --- W6.2c-OUT cross-module offline shims: the out-arm ENGINE free-functions
    #     + SM-cursor globals the 6 variants call but whose golden bodies (aoutarm.cpp
    #     / aoutarm9045.cpp engine helpers / Magazine / aRotateKIT_Out / AutoTeach /
    #     FixAICCD) are not translated yet.  Offline-safe non-static defs so an exe
    #     linking ht9045_sm resolves them; the variants forward-declare each.
    aoutarm_shims.cpp
    # --- W6.2c-OUT batch-4 (DEAD): 6 out-arm site variants that are NEVER
    #     dispatched -- compiled as dead weight only (like in-arm 2x4_16/2x8_32).
    #     1x4_4_Back (enum present but DoOutArm ladder arm is an EMPTY //-body and
    #     DoPickFromShuttle routes it to LIVE 1x4_4); 2x2_4_14 (enum present, BOTH
    #     ladders route to LIVE 2x2_4); 2x2_4_23 / 2x4_16 / 2x8_16 (enums ABSENT
    #     from MachineType.h -> no dispatch arm); 2x8_32 (enum present, BOTH ladders
    #     route to LIVE 2x8_8).  Each carries the full _<deadv> suffix on its
    #     DoOutArm_9045_<deadv> + helpers so all symbols are UNIQUE never-called
    #     globals.  CMake-only: NO static-stub removal, NO extern un-gate, NO
    #     dispatch-ladder edit to aoutarm9045.cpp (the ladders are already 100% live).
    aoutarm9045_1x4_4_Back.cpp
    aoutarm9045_2x2_4_14.cpp
    aoutarm9045_2x2_4_23.cpp
    aoutarm9045_2x4_16.cpp
    aoutarm9045_2x8_16.cpp
    aoutarm9045_2x8_32.cpp
    # --- W6.2c-OUT B5 (S-prefix, dead-but-compiled): the LAST out-arm variant
    #     file.  aoutarm9045S_1x4_4.cpp carries the IRREGULAR _9045S_1x4_4 infix
    #     on its top-level export DoOutArm_9045S_1x4_4() AND all helpers.  VERIFIED
    #     DEAD: zero external callers in the golden tree (out-arm dispatch ladders
    #     route to the SUFFIX-S DoOutArm_9045_1x4_4S, defined in the DIFFERENT
    #     already-committed aoutarm9045_1x4_4S.cpp).  Compiled-only, like in-arm
    #     2x8_32: NO static-stub removal, NO extern un-gate, NO dispatch-ladder
    #     edit to aoutarm9045.cpp (dispatch unchanged -- file never dispatched).
    aoutarm9045S_1x4_4.cpp
    # --- W906-Automation INTEGRATE: Automation/automation.cpp -- TfAutomation,
    #     the OLP host-command engine's "network wiring + small helpers" slice
    #     (~34 methods + 6 free functions: CommandProcess/SendClient/SendServer,
    #     OLPServerClient*/OLPClient* socket event handlers, tmrOLPTimer,
    #     DoCommandBuffer, ShowCharHex/ShowRecord/SaveRecord/ShowMSG,
    #     GetHTMLJSONDatas/GetMachineStatus/GetWorkOrder/GetMainTemp/
    #     GetTemperatures, ...). Lives HERE (not a standalone library) because
    #     it depends on FormsFacade.h (fMain->palMainStatus/cbSetupFileName/
    #     edWorkTemperBase) + canary_support.h (RecordProcess) + auto9045.h
    #     (GetHandlerType/GetDataPath/GetMainStatus/...), all already carried
    #     by this library -- same rationale as Interface/InterfaceSYS.cpp /
    #     Interface/TesterTCP_Socket.cpp above.
    #     AI(W906-P3-OLP) 20260920 STALE-CLAIM FIX: this note used to say
    #     "ProcessBuffer/ProcessBuffer1/SendReportRequest stay safe no-op
    #     stubs (deferred to a dedicated future wave)". That is NO LONGER
    #     TRUE -- the W906-AutoPB wave translated ProcessBuffer in full
    #     (automation.cpp:1288, 67 *_REQUEST + 104 *_INQUIRE branches).
    #     Leaving the claim up is the expensive kind of stale note: the
    #     next reader concludes the path is dead and stops measuring.
    #     NOTE the dispatcher is translated but DELIBERATELY NOT REACHED
    #     from wb_serve -- see WebOlp.h's header for why the browser must
    #     not go through it (SendClient forces iConnect=0 at :1034-1039,
    #     pushing the reply into a real factory host's OLP socket).
    #     INTEGRATE CHECK: golden's
    #     bare `fAutomation` global is already claimed by atester_shims.h's
    #     TfAutomationShim (consumed by atester.cpp/atester_32Site.cpp) -- this
    #     translation names its own real singleton `fAutomationEngine` instead
    #     (see automation.h's own "GLOBAL-NAME COLLISION AVOIDED" note); no ODR
    #     conflict confirmed by grep across the whole tree.
    Automation/automation.cpp
    # --- W906-D-Interface-TesterTCP INTEGRATE: Interface/TesterTCP.cpp -- a
    #     SIBLING slice of Interface/TesterTCP_Socket.cpp above (same golden
    #     TfTesterTCP form, a DISJOINT function subset: CopyRecipeToTester/
    #     CopyRecipeFromTester OS-recipe zip-transfer pair + the per-machine
    #     btnSaveClick arm-speed-save handler; TesterTCP_Socket.cpp keeps sole
    #     ownership of the ClientSocket_TCPIP connect/timer/read/error family --
    #     zero overlap, confirmed by both units' own translate reports). Lives
    #     HERE for the identical reason as its sibling: depends on common.h
    #     (DataPath/sSaveByMachine/WriteIniData, ht9045_core, transitively
    #     needed at final-link time) + canary_support.h (RecordProcess/
    #     ShowMyMessage) + the REAL offline MySleep(DWORD) already defined in
    #     acarry_shims.cpp below (this file's own extern redeclaration now
    #     resolves for real once compiled into this same library, matching the
    #     precedent already established by Automation/auto9045.cpp/
    #     CCLink/MyCCLink.cpp/KYECFTP/FTPClient_Transfer.cpp's identical
    #     extern-redeclaration idiom for this exact symbol).
    Interface/TesterTCP.cpp
    # --- W5 Automation wave (Automation network unit): TeraPower AMR tray-count
    #     bridge (TTeraPowerAMR / global `AMR`) + the HANA-ART SRQ protocol
    #     helper (uHANA_ART) + the 8-function SCK/ATK "Auto Retest" extract
    #     (Automation/SCK_ART.cpp, wired into csystem.cpp's W7C1/W7C2 SCKART
    #     seams -- see csystem.cpp).  Same footprint already carried by
    #     ht9045_sm: ht9045_globals/ht9045_motor/FormsFacade.h/canary_support.h.
    #     NOTE: Automation/AMR.cpp's global `AMR` retires acatchtray_shims.cpp's
    #     TAMRShim/AMR stopgap (ODR: same name, different type) -- see that
    #     file's comment.
    # AI(W906-W7-F0-fix) 20260728: this wave's 4th member,
    # Automation/AGV_predicates.cpp (the 3 AGV AMR-mode predicates
    # AGV_IsSPIL_AMR/AGV_IsATK_AMR/AGV_Use_AMR), was MOVED OUT of this library
    # into ht9045_forms -- forms/fAGV.cpp delegates to them, and leaving them
    # here gave libht9045_forms.a 3 undefined refs into ht9045_sm that
    # ht9045_forms declared no dependency for, silently voiding its bottom-layer
    # property.  See the "NO UNDECLARED BACK-EDGE" block in the ht9045_forms
    # target above.  This library's own consumer, Automation/AGV_PortScan.cpp
    # (below), still resolves them: ht9045_sm PUBLIC-links ht9045_forms.
    Automation/AMR.cpp
    Automation/HANA_ART.cpp
    Automation/SCK_ART.cpp
    # --- W5-Final-AGV_E84 INTEGRATE: Automation/AGV_E84.cpp -- the E84
    #     loader/unloader handshake core (DoE84Loader/DoE84Unloader, 15/15
    #     case labels each, faithful) + ShowE84Log/E84StatusLog file logging.
    #     Same footprint as the AMR/HANA_ART/SCK_ART cluster just above
    #     (ht9045_globals/ht9045_motor/FormsFacade.h/
    #     canary_support.h) -- that cluster's 4th member, AGV_predicates.cpp,
    #     moved to ht9045_forms, see the AI(W906-W7-F0-fix) note above;
    #     additionally now uses fAGV->mmE84Log (see
    #     FormsFacade.h/.cpp TfAGV::mmE84Log, added this integrate).
    Automation/AGV_E84.cpp
    # --- W5-Automation-cont INTEGRATE: Automation/AGV_PortScan.cpp -- the
    #     second AGV.cpp slice (after AGV_E84.cpp's E84 handshake core above and
    #     AGV_predicates.cpp's 3 AMR predicates, which AI(W906-W7-F0-fix)
    #     20260728 moved to ht9045_forms -- this file calls them and still
    #     resolves them via ht9045_sm's PUBLIC link to that target): E84
    #     sensor-status
    #     telemetry (NowE84Status/E84StatusChange) + the AMR/AGV port-scan
    #     dispatcher family (ScanLoadPort + 8 bScan*/bIsStackBusy free functions),
    #     13 functions total.  Same footprint as the AGV_E84.cpp/AGV_predicates.cpp
    #     cluster (ht9045_globals/ht9045_motor/FormsFacade.h/canary_support.h),
    #     plus 3 new FormsFacade stand-in members this unit added (TfMain::ALed1/
    #     labAutomation, TfLotInfo::ALedLoader/aLedAuto[3]/palRemoveTray -- see
    #     FormsFacade.h/.cpp "W5-Automation ADD" blocks) and #includes AGV_E84.h
    #     (reuses its iLoaderScanTask[]/iUnloaderScanTask[]/bE84Loaderflag[]/
    #     bE84Unloaderflag[] externs, does not redeclare) + AGV_predicates.h.
    Automation/AGV_PortScan.cpp
    # --- W5-Final-SckArtRemainder INTEGRATE: Automation/SCK_ART_Remainder.cpp --
    #     the next contiguous golden slice (SetSetupFilePath/AccessFile/
    #     SetGPIBVersion/UpdateCount/AddAlarmCode/ClearLotInfo/AddOutputJamCnt/
    #     SaveTestSummary-dispatcher) continuing the sibling SCK_ART.cpp 8-
    #     function extract just above.
    #     AI(W906-W7-F2) 20260729: the "flagged for a future TfSCKART/FormsFacade
    #     unification wave" TODO that used to end this entry is REPLACED by what
    #     that wave actually did, because it landed PARTIALLY and the difference
    #     matters to whoever picks this up next:
    #       DONE  -- SckArtRemainderState now `: public SckArtState`, so
    #                dCurrYield / iFTRTCount / iNeedRT / iInputCount /
    #                iManualRejectCnt / iTesterType / sLOTSTATUS / iCurrentStatus
    #                (+ iCurrent93KARTStep) are declared ONCE instead of twice.
    #                Provably behaviour-neutral: both ctors were compared field by
    #                field and already produced identical values.  Locked by
    #                tests/test_w7_f2_sckart_state.cpp.
    #       NOT DONE -- forms/fSCKART.h's TfSCKART still carries its own
    #                iInputJamCnt/iFTRTCount/iInputCount/iCurrent93KARTStep, and
    #                csystem.cpp's W7C1_TfSCKARTSeam / W7C2_TfSCKARTSeam each still
    #                embed a private SckArtState plus shadow fields.  Golden has
    #                ONE fSCKART object; this tree has several, so collapsing them
    #                CHANGES BEHAVIOUR (a write through one instance would become
    #                visible to readers of another) -- that is a semantic decision,
    #                not a refactor, and W7-F2 deliberately did not take it.
    #       FOUND    -- two real golden divergences surfaced by that diff
    #                (iLOTSTATUS_L/_W/_R/_A constructed 0 vs golden 3/1/4/6;
    #                iTesterType constructed 0 vs golden 1 for every CUSTOMER_CODE
    #                except CC_SCK).  Reported and pinned by test, NOT reconciled,
    #                per plan SS6-F2 / SS9-R9.  See the divergence blocks in
    #                csystem.cpp (W7C1_TfSCKARTSeam) and Automation/SCK_ART.cpp
    #                (SckArtState ctor).
    Automation/SCK_ART_Remainder.cpp
    # --- W5-Final-Auto9045 INTEGRATE: Automation/auto9045.cpp -- the whole OLP/
    #     GPIB host-command config-accessor layer (148 golden free functions,
    #     confirmed NOT a VCL TForm). Same footprint as the Automation cluster
    #     above; all VCL-form gaps this file reaches (fBinSel/fShowBinSelect/
    #     fTrayAssignment/fTestCategory/fContactCT/fCounterClear/fMonitor/
    #     fProductionInfo/fTemp_Set + extra fMain/fObserver/fLotInfo/fContact/
    #     fAutomation members) stay as this file's own TU-local `W5FA_`
    #     companion objects -- NOT folded into FormsFacade.h/atester_shims.h by
    #     this integrate (would each need a brand-new shared form class with no
    #     other consumer yet; deferred, see this integrate's own report).
    Automation/auto9045.cpp
    # --- W7 (TRAY-ARM engine): the top dispatcher DoCatchTray (flat switch) + 8
    #     pumpable task SM skeletons (DoCatchFromLoader / CatchNewTrayFromBuffer /
    #     DoPlaceTrayToAuto / DoPlaceToBuffer / DoSlapTray / DoCatchUnderTray /
    #     DoSupportUnderTray / DoPlaceBufferTray) + cover-tray/RFID/cassette SMs +
    #     the pure decision helpers (WhichAutoNeedTray/IsOutArmSafe/...).  No
    #     site-variant dispatch in this file.  Hardware behind the Sim HAL
    #     (MOT[]/Cylinder[]/Sen[]/SW[]/CatchTraySuck); cross-module/UI surface
    #     (fTrayMapping/LoadCCD/MyMessageBox/HSys.BinDisCtrl) behind offline
    #     shims + ACTIVE stubs; AMR is REAL now (Automation/AMR.cpp above); see
    #     the file-head gate documentation.  OWNS the real WhichAutoNeedTray()
    #     (the canary_support.cpp sim stub was removed).
    acatchtray.cpp
    # --- W6.3 TRAY-ARM cross-module offline shims (fTrayMapping / MyMessageBox /
    #     LoadCCD / SECS_EVENT+EventReport / Tray-Mapping-laser / AMR-feed /
    #     NewRecordProcess / BoolToStr / iCoverTrayIDTask / iReadCIDAction).
    acatchtray_shims.cpp
    # --- W6.4 (TESTER/INDEX engine): the test-result-DECODE anchor (GetBinaryData /
    #     GetOneByOneData fed by Test_GetTestResulSub/Test_GetTestResul) + its
    #     consumers (Check_TTL_Status / CheckTestSocketIsError / ProcessTestResult)
    #     + the central index SMs DoTestY / DoTestHeadMotor (entry faithful, deep
    #     MOT[]/EP/torque/Front-Rear/32Site bodies gated) + pure site geometry
    #     (ArrayConvertSite / GetRowCol / GetRTCSiteMap).  Pumps the same Sim HAL +
    #     aHotPlateSubstrate (TestSocket/FTestSuck/BTestSuck/InArmSuck/FRCarryKit/
    #     BRCarryKit + iTestYTask/iTestHeadMotorTask cursors).  See the file-head
    #     gate documentation.
    atester.cpp
    # --- W64b (TESTER/INDEX engine): the Front (Z1/Arm1) and Rear (Z2/Arm2)
    #     "Destroy IC" + "Suck IC" down-press state machines (DoFrontTestDestroyIC /
    #     DoRearTestDestroyIC / DoFrontTestSuckIC / DoRearTestSuckIC +
    #     CheckZ1IsDown/CheckZ2IsDown/TestZ1OutRandge/TestZ2OutRandge/
    #     TestZ1SetPos/TestZ2SetPos leaf deps), faithfully translated -- no
    #     longer offline shims for these symbols (see atester_shims.cpp note
    #     below for what of Front/Rear is still shimmed).
    aTester_Front.cpp
    aTester_Rear.cpp
    # --- W5-atester32site / W5-ProcessCount-Translate: the 32-Site dual-arm
    #     test-head engine (all 8 golden functions, 2 vision islands internally
    #     gated -- see the file's own header) + the ProcessCount/low-yield-alarm
    #     leaf subset (14 golden functions: RecordPiggyBackStartEnd/
    #     ProcessPiggyBackFunction/InitialPiggyBackFunction/ProcessSocketPurgeCount/
    #     DoLowYieldAlarm/DoLowYieldAlarmFirst/CheckContinuoussFail/
    #     ProcessContinuoussFailForATC/GetMultiplierNum/CheckContinuoussPassBySocket/
    #     CheckContinuoussPass/ProcessHeadContactCount/ProcessSocketCount/
    #     ProcessAutoloadcellMeasureCount).  Both retire their corresponding
    #     stub entries from atester_shims.cpp/.h (see that file's remaining
    #     scope note below).
    atester_32Site.cpp
    atester_ProcessCount.cpp
    # --- W6.4 TESTER/INDEX cross-module offline shims (fContact / ADAM_* /
    #     CCDInterfaceForm / fAutomation / fObserver / fiosetview / COM2 / IsNNMode /
    #     CheckHeaterOK; aTester_Front/Rear's Destroy+Suck pairs and the whole of
    #     atester_32Site/atester_ProcessCount are real now -- see above -- every
    #     OTHER Front/Rear export, e.g. DoTestYFront/TestZ1OutRandge2, plus the
    #     ProcessCount exports still out of this wave's scope [ProcessShowTestStatus/
    #     ProcessCount/ProcessStartTestData/RecordHistroy/ProcessQASampling], stay
    #     offline shims here).
    atester_shims.cpp
    # AI(W906-R28TORQ) 20260925: golden TCOM2 的 Index Z 扭力那一半（rs232.cpp；使用者 20260925 裁決第 6 條）。
    #   TCOM2Shim（COM2）的建構子與扭力成員都在這裡；放 ht9045_sm 是因為它要 fMain（ht9045_forms）、
    #   fContactForm（forms/fContact.cpp，本 target）、SW[]（ht9045_io）、MOT[]（ht9045_motor）、HSys（ht9045_db），
    #   而且跟 atester_shims.cpp（定義 `COM2 = new TCOM2Shim()` 的那個 TU）必須在同一個 archive —— 建構子在這裡，
    #   沒有它 atester_shims.cpp.obj 連不起來（pt-wave-loop 陷阱 #1 第 3 種形狀「註冊在錯的 archive」）。
    rs232.cpp
    # --- W6.5 (SHUTTLE/CARRY engine): the two input-shuttle master SMs
    #     Do_Auto_SHT1 / Do_Auto_SHT2 (39 cases each, ALL ACTIVE; case 200->201
    #     fall-through preserved) + the 9046AU sort-shuttle Do_Auto_SHT3 + the
    #     residual/sensor-broken/null-IC/floating checks (switch TestIF.iTestMode)
    #     + the In/Out-arm-vs-shuttle move-safe interlocks + pure pitch geometry +
    #     the uShuttleThread sensor-poll LOGIC (verbatim; the Win32 wrapper is a
    #     thin offline driver).  Pumps the same Sim HAL + aHotPlateSubstrate
    #     (FLCarryKit/BLCarryKit/FRCarryKit/BRCarryKit + AutoSHT1/2/3Task cursors).
    #     NO #if 0 gates -- every golden case body is ACTIVE.  See the file head.
    acarry.cpp
    # --- W6.5 SHUTTLE/CARRY cross-module offline shims (Tech / fLtcSensor /
    #     OutSht3Kit / ATC_InterfaceForm+ATC_Data / CheckShtFloating /
    #     InShtInLF-RT(int) / SortSHTInLF-RT / DoStepShuttleCheck / shuttle
    #     Empty/MustHas/Prominent checks / Check32site* / MoveOutArmXY_To* /
    #     SendHTTPRequest / MoveSortArmToAutoSafe / Win32 thread tokens).
    acarry_shims.cpp
    # --- W906-AutoCleanFoundation (~30 pure-calc/HAL-only helpers) + W906-
    #     AutoCleanCluster (9 small helpers + the 4 named core pick/place
    #     engines -- DoAutoCleanPickfromCleanKit/DoPlaceToShuttle/
    #     DoPickFromShuttle/DoAutoCleanPlaceToCleanKit -- + the 3 shuttle-clean
    #     state machines + the DoAutoCleanKit master orchestrator), out of
    #     golden AutoClean/AutoClean.cpp (9,137 lines). Still explicitly OUT OF
    #     SCOPE (a separate follow-on wave, the SECOND dependency cluster):
    #     DoIndexAutoClean(+_Arm1PickArm2Test variant) -- see the TEMPORARY
    #     placeholder stub for DoIndexAutoClean near the top of the .cpp.
    #     Mirrors golden's own AutoClean/ subfolder (new in the target tree).
    AutoClean/AutoClean.cpp
    # --- W6.6 (csystem ORCHESTRATION HUB): the integration SPINE that ties the
    #     five already-translated engines together.  csystem.cpp realizes the
    #     frozen csystem.h spine free-functions: MainProc (slim) + DoAllProcess
    #     (THE per-tick engine driver -- bDoProcess 2-tick alternation + master
    #     guards + the 5 engine calls in documented order + DoLoad/DoCatchTray/
    #     DoAutoReceiveBinTray loop + tail loops) + the alive-instrumentation
    #     (GetMainProc*/IsMainProcAlive) + InitAllProcessTask + CheckMotorHome +
    #     SetInitialICCheck + InitCleanOutFunction/InitOneCycle/InitTrayEndFunction.
    #     The ~24K-line god-file bulk (AutoRetest/OCR/AutoClean/mode-ladder/
    #     DoOneCycleFinishCheck/DoCleanOutFinishCheck/...) is GATED #if 0
    #     // TODO(W7).  Predicates (HasIC family) stay in csystem_predicates.cpp
    #     (ODR-skipped here).  See the file-head documentation.
    csystem.cpp
    # AI(W906-HOME-C2) 20260920: uhome.cpp -- golden uhome.cpp:719-4842 的移植，
    #   其中 :1180-4842 是 ProcessMotorHome()（3,663 行、73 個 case）。
    #   放在 ht9045_sm 是因為它唯一的呼叫端是同一個 library 裡的
    #   csystem.cpp DoHomeProcess()，而它消費的是同一批機台全域
    #   （MOT[] / Cylinder[] / Sen[] / SW[] / Prod / TestIF...）。
    #
    #   ⚠ 它的資料層在別處：`THomeClass` 與 `TfHome::InitialHomeClass()` 在
    #     forms/fHome.{h,cpp}（ht9045_forms），因為 TfHome 門面早就住在那裡。
    #     `TfHome::Show/Close/RotateCheckClear` 則在 uhome.cpp ——
    #     golden 自己也把那三個放在 uhome.cpp。
    #
    #   ⚠ 它**不** include Motor/myGALILmotor.h。原因與 forms/fHome.cpp 檔頭寫的
    #     同一個：那會逼出 609 個 `TMyMotor::Gali_*` 重複符號
    #     （mymotor.cpp 的樁 vs myGALILmotor.cpp 的真本體）。
    uhome.cpp
    # AI(W906-FW3-WA) 20260817: Command.cpp -- FW-3 Wave A, 63 golden TfMain::
    # PURE methods (the GPIB/TCP reply surface). Lives in ht9045_sm, NOT
    # ht9045_forms: the bodies are heavy machine-global consumers
    # (TestIF/LastSet/BinSelect/IndexStatus...) and ht9045_forms links nothing
    # above itself by design (see the ht9045_forms block); TfMain member
    # definitions may legally live in any TU. Declarations: forms/fMain.h
    # FW3-WA block. The 3 pre-existing fMain.cpp stub bodies (ArmStatusStrings/
    # WritePERSITETemperature/PERSITETemperatureStrings) were retired in the
    # same commit -- multiple definition otherwise.
    Command.cpp
    # AI(W906-FW3-Obs1) 20260818: cObserver.cpp -- FW-3 cObserver Wave 1, the
    # read-only display core (EventLog CSV parse/filter, MDB query strings,
    # contact-kind stats) against the new forms/fObserver.h facade. The class
    # is TfObserver -- a DIFFERENT name from atester_shims' TfObserverShim, so
    # the two coexist without ODR contact; the shim<->facade swap of the
    # global fObserver is a separate, deliberate wave.
    cObserver.cpp  cObserver_TimeInfo.cpp   # AI(W906-W2-3) 20260926: golden cObserver.cpp:1846-2134 RecordTimeInfo (same library as cObserver.cpp)
    # AI(W906-FW3-YM-A) 20260818: uYieldMonitoring.cpp -- FW-3 yield-engine
    # Wave A (17 PURE methods vs forms/fYieldMonitoring.h facade; the class
    # coexists with the TfYieldMonitoring_2x4_16 stand-in by name -- the
    # fYieldMonitoring global swap is a queued, deliberate wave; the safety
    # ACTION methods DoAutoCloseSite/DoRTAutoSocketOff are NOT translated).
    uYieldMonitoring.cpp
    # AI(W906-FW-CTBS) 20260818: cContactCT.cpp (full, 21 methods) +
    # cShowBinSelect.cpp (Wave A, 18/29 -- widget-heavy Wave B and the
    # machine-action safety set are queued, see forms/fShowBinSelect.h).
    # Their globals are DEFINED nullptr in their own TUs: every consumer
    # dereference is still gated; real instances land with the yield-engine
    # 39-gate enablement wave.
    cContactCT.cpp
    cShowBinSelect.cpp
    SmartDiagnostic.cpp  # Steven 20260925 (Data.SmartDiagnostic): CreateCyliderNameClick body (reads Cylinder[] in ht9045_io); forms/fSmartDiagnostic.cpp facade
    cTestCategory.cpp  # Steven 20260925 (Data.TestCategory): golden TfTestCategory FormShow/AdjFormData/sgArm1DrawCell/SetTestCate* bodies (IsNNMode -> ht9045_sm); forms/fTestCategory.cpp facade
    cSortCT.cpp  # Steven 20260925 (Data.SortCT): golden TfSortCT ShowLoadingIC/ShowLoadingIC_ART/ShowSortIC/btnClearCountClick bodies (reach fShowBinSelect/ArmData/fProductionInfo -> ht9045_sm); forms/fSortCT.cpp trampolines to them via g_W906_SortCTBodies
    # AI(W906-FW-SecCC) 20260819: cSecurity.cpp (access-level engine +
    # jam-level tables) + cCounterClear.cpp (counter-clear engine; its
    # csystem.cpp W7C1 seam retired same commit). Globals are REAL instances
    # (SIOF-audited: TfCounterClear ctor = one bool; TfSecurity ctor's
    # cross-TU work sits behind if(INIFileGeneral!=0)).
    cSecurity.cpp
    cCounterClear.cpp
    # AI(W906-FW-BinSel-WA) 20260819: cBinSel.cpp Wave A (data core +
    # the 21+ gate-consumer members; UI bulk = Wave B stubs). Global is a
    # REAL instance (ctor = own-member allocations + zero-read bools,
    # SIOF-audited at integration).
    cBinSel.cpp
    # AI(W906-FW3-TemperFrom-WA) 20260818: cTemperFrom.cpp -- FW-3 cTemperFrom
    # Wave A (temperature-display data core: ctor channel/panel maps, the
    # 885-line ShowThermo classifier, TempRunShowAlarmHigh/Low, the small
    # function-status strip) against the new forms/fTemperFrom.h facade. No
    # existing stand-in shares the class name TfTemperFrom (only the
    # differently-named TempCtrl/TriTemp.cpp W7TT_TfTemperFrom, no ODR
    # contact); the live fTemperFrom global is not declared this wave --
    # both known consumers (csystem.cpp GATE G02, PowerSavingMode.cpp
    # GATE (4)) stay gated, integration is a separate, deliberate wave.
    cTemperFrom.cpp
    # AI(W906-FW3-TempSet-WA) 20260820: uTemp_Set.cpp + DynamicTemp.cpp --
    # FW-3 batch-3 close-out (temperature-set dialog: 106/108 methods vs
    # golden's 108 definition points, ctor emptied into explicit Init(); and
    # the temp-IC calibration dialog: 26/26) against new forms/fTemp_Set.h /
    # forms/fDynamicTemp.h facades. Globals are golden-faithful ZERO-INIT
    # pointers (TfTemp_Set *fTemp_Set; TfDynamicTemp *fDynamicTemp; -- no
    # static-init ctor, SIOF-safe by construction). All hardware/file writes
    # sit behind SAFETY gates (S1-S16 in the .cpp banner). Known waiting
    # consumers stay gated this commit: uHeaterThread.cpp GATE 6
    # (ControlATC60AirFlow x4), cprod.cpp:3989-4003, and auto9045.cpp's
    # TU-local W5FA_FTemp_Set stand-in (differently named, no symbol clash;
    # convergence is a separate behaviour-change wave).
    uTemp_Set.cpp
    DynamicTemp.cpp
    # AI(W906-FW-SETUP-B) 20260821: cSetUp.cpp -- TfSetup's sm-reaching trio
    # (ReadUseSuckModeFile / CheckSTMMode / cbI21Click). NOT forms/fSetup.cpp
    # material: these bodies need InArmSuck/MOT[]/IndexHasIC/per-arm dispatch,
    # all ht9045_sm residents, and ht9045_forms cannot link sm (documented
    # cycle at :589-593). Same root-file split as cTemperFrom/MainTempMode.
    cSetUp.cpp
    # AI(W906-TRAY-READ) 20260923: cTrayForm.cpp -- TfTrayForm::Init()/ReadFile()（golden cTrayForm.cpp:28-139 /
    # :364-453）。要 HTEditList（Public/HTEditList.cpp，本目標），所以同 cSetUp.cpp 的根目錄分家：
    # facade（建構子／IsEnableColorSensor／全域）留在 forms/fTrayForm.cpp（ht9045_forms）。
    cTrayForm.cpp
    # AI(W906-MSTATE-P2) 20260924: cMainStatus.cpp -- TfMain::ShowNowStatus／EnabledSetupFile／
    # CheckCanChangeRealDummy（golden main.cpp:22190-22317／28310-28425／12374-12380），ShowRunLabel 解閘的地基。
    # 要 HasICUnderMachine／ShuttleHasIC／EventReport（sm／secsgem），同 cSetUp.cpp 的根目錄分家。
    cMainStatus.cpp
    # AI(W906-T4-U35) 20260924: cLd_ULd.cpp -- TfLd_ULd::Init()/ReadFile()（golden cLd_ULd.cpp:26-84 / :143-154），
    # 要 HTEditList（本目標），同 cTrayForm.cpp 的根目錄分家。
    cLd_ULd.cpp
    cSensorScan.cpp       # AI(W906-SENSORSCAN) 20260924: TfMain::ProcessSensorScan（golden main.cpp:13949-14305）＋ W906_BootSimLoaderCheckBox；要 Sen[]／RecordProcess（本目標），同 cMainStatus.cpp 的根目錄分家
    cCleanOut.cpp  cDIOStatus.cpp  BootSummary.cpp  # AI(W906-BOOTSUM) 20260925: BootSummary.cpp = 開機 [BOOT] 摘要（純觀測）；AI(W906-CLEANOUT) 20260924: TfMain::CleanOut／BtnCleanOutClick（golden main.cpp:4264-4330），要 InitCleanOutFunction／MOT[]／EventReport，同 cMainStatus.cpp 的根目錄分家   AI(W906-DIO) 20260925: cDIOStatus.cpp = TfMain::InitDIOStstus（golden main.cpp:24357-24375）＋ W906_BootInitDIOStstus（FormShow :10106-10108），要 SW[]（ht9045_io），同上的根目錄分家；接在同一行，不移動行號
    cStateRecord.cpp  common_ChangeLog.cpp   # AI(W906-CHGLOG) 20260927: common_ChangeLog.cpp = golden WriteIniData change-log bodies + TempChangeLog (hooks from common.cpp) | AI(W906-STATEREC) 20260924: TfMain::DoStateRecord 本體＋SaveTaskList／SaveDecisionVariables／ProcessTimeUpdate／StateRecordImage／DumpMainFormSnapshot（golden main.cpp:6402-6963／7806-7818／26109-26678），要 SaveMachineRecord／Del_Tree／MOT[]（本目標）。只經 wb_serve 的 W906_InstallStateRecordBody() 被抽進來，測試執行檔不會連到它
    # AI(W906-OFS-READFILE) 20260923: cOffSet.cpp -- TfOffSet::ReadFile and the
    # three bodies it calls (GetOffsetPath / ReadInvisibleFile /
    # LoadLoaderScaleValues) plus the CapStrInput/Output/Sortput tables.
    # Here and not forms/fOffSet.cpp for the same reason as cSetUp.cpp above:
    # GetOffsetPath needs FileInfo().PathCombin (ProductionInfo/FileInfo.cpp,
    # this library, :2641) and ht9045_forms cannot link ht9045_sm.  That was
    # the whole content of forms/fOffSet.h's GATE (O-4), now retired.
    cOffSet.cpp
    # AI(W906-AA-READFILE) 20260923: first file of the AutoAlignment module in
    # this tree -- TfAutoAlignment::ReadFile only (golden :1373-1431).  Here
    # and not ht9045_forms because it reaches fMain->cbSetupFileName and
    # GetRecipeFileName, same reasoning as cOffSet.cpp above.
    AutoAlignment/AutoAlignment.cpp
    # AI(W906-MG-READFILE) 20260923: first file of the Magazine module here --
    # TfMagazine::ReadFile only (golden :3607-3646).
    Magazine.cpp
    # AI(W906-FW-QWKEY1-integ) 20260824: the sm-reaching halves of the
    # fQwertyKey/fPassword pair (TfQwertyKey::FormDestroy needs MyDBIProcess/
    # LogSoftwareOffTime; TfPassword::sbPasswordModifyOKClick needs
    # ShowMyMessage) -- same root-file split as cSetUp.cpp above.
    myQwertyKeyBoard.cpp
    Password.cpp
    # AI(W906-FW-BARCODE1) 20260824: TFormBarcodeReader + Barcode_Reader/
    # InputBarcodeNumber -- the last shared blocker behind every remaining
    # gated keyboard/barcode site (entity wave; gate openings are the next
    # wave's separate commit).
    BarcodeReader.cpp
    # AI(W906-FW-CMD-E) 20260821: MainTempMode.cpp -- TfMain::ChangeTempMode
    # (golden main.cpp:21749-21925), the temperature-mode switcher extracted
    # from main.cpp. NOT MainCalcCore material (that file's charter is pure
    # computation; this method has machine-facing side effects, all behind
    # SAFETY gates: fSetup/fOffSet/COM2 sites gated on real absences,
    # SendCommToATC7 and heater actions inert). Declaration in forms/fMain.h.
    MainTempMode.cpp
    # AI(W906-FW3-Speed-WA) 20260819: cSpeed.cpp -- batch-5 cSpeed display
    # side (ctor, FormShow, ReadFile, ReadWriteFile read-arm with the write
    # arm forced off at entry, DoIniDataToForm, FormClose's re-read-refresh)
    # against the new forms/fSpeed.h facade. 47 write-path methods and the
    # 3 TWinControl-recursive RPDefault methods are not declared at all;
    # the live fSpeed global is not declared -- cprod.cpp's GA1-B2 gate
    # stays gated, integration is a separate, deliberate wave (the
    # fTemperFrom posture).
    cSpeed.cpp
    # AI(W906-FW3-StartCond-WA) 20260819: cStartCondition.cpp + HandlerSys.cpp
    # -- batch-5 display sides (8/35 and 3/44 methods; write paths undeclared,
    # the fTemperFrom posture). New findings live in their facade banners:
    # FormShow's two hidden LastSet.iStartMode writes gated (golden :145/:171),
    # UpdateCylinderScreen's clear-and-save self-heal gated (golden :1565-1572),
    # and the pointer-typed TPageControl::ActivePage comparison class of gate
    # (vclcompat only has ActivePageIndex). Live globals not declared --
    # integration is a separate wave.
    cStartCondition.cpp
    HandlerSys.cpp
    # AI(W906-FW3-Config-WA) 20260820: cConfiguration.cpp -- batch-5 Wave A
    # (28 display-side methods, 289 golden lines -- the wave corrected the
    # recon's own 354 sum; the 4,131-line InitConfigEdtList policy cluster
    # and every (b)/(c) method stay undeclared). New TfConfigurationGrid
    # facade subclass per the TfObserverGrid convention; fConfiguration
    # global not declared -- ctor is write-path, integration deferred.
    cConfiguration.cpp
    # AI(W906-BinDisp-WA) 20260820: BinDisplay/MyBinDisp.cpp -- the
    # TMyBinDispCtrl data-layer base (recon docs/RECON_BinDisCtrl.md path 1),
    # plus TMyBinDispOffline no-op concrete child. Hardware protocol subclass
    # (~2,500 lines) deliberately unscheduled; the 9 existing BinDisCtrl
    # #if 0 gates stay gated until a separate integrator wave wires
    # database.h and flips them one by one.
    BinDisplay/MyBinDisp.cpp
    # --- W6.6 HUB cross-module offline shims (DoInArm->DoInArm_9045 /
    #     DoOutArm->DoOutArm_9045 / DoLoad / DoSortArm / bShuttleShake +
    #     InitLoadTask / InitOutArmTask / InitialDoLockUnloader /
    #     Initial_Auto_BinTray_Task / InitAutoColorTask / Magazine inits /
    #     DoAuto3Magazine / NewDoAutoTrayEdgeCylinderLoop / DoAutoEmpty1 /
    #     DoInArm/OutArmTeachAlignmentProcess / SetFixTrayMiddleDtata /
    #     DoOneCycleFinishCheck / DoCleanOutFinishCheck).
    #     AI(W906-W7-L1) 20260729: DoAuto2 REMOVED from this list -- it is now a
    #     real body in asendic_Auto2.cpp (added just above), not a shim here.
    csystem_shims.cpp
    # --- W5-BarCode (BarCode front, 2026-07-11): real bodies for 18 of the 20
    #     TfBarCode_Shim methods (aHotPlateSubstrate.h:623-664).  7 parallel
    #     translate units extracting SPECIFIC functions from the still-largely-
    #     untranslated ~24,500-line BarCode/{BarCode,BarCode_Sh1,BarCode_Sh2,
    #     BarcodeXML}.cpp VCL-form core (see MIGRATION_ROADMAP.md DEFERRED
    #     table).  DoBarcodeScanInShuttle_2/DoShuttleFloatCheck_2 were NOT
    #     delivered this wave (clean hand-off, BarCode_Shuttle2_Scan's own
    #     report) -- those 2 TfBarCode_Shim methods keep their pre-existing
    #     offline stub.  BarCode_Bottom2DID8CCD.{h,cpp} intentionally live at
    #     project ROOT (not under BarCode/), per that unit's exact task-brief
    #     path -- see its own file banner.
    #     INTEGRATE FIX: resolved 2 real cross-unit ODR conflicts before wiring
    #     (BarCode_Shuttle2_Scan.cpp's own bCCDcheckShuttle[2] vs. the existing
    #     cmydef.cpp global; BarCode_Shuttle2_CCDScan.cpp's independent
    #     redefinition of 8 golden TfBarCode-member globals already owned by
    #     BarCode_Bottom2DID.cpp) -- both units' own translate reports flagged
    #     these; see aHotPlateSubstrate.cpp's include-block comment for the
    #     resolution.
    # AI(W906-BC-READFILE) 20260923: TfBarCode's own bodies -- the 21 forwarders
    # moved out of aHotPlateSubstrate.cpp plus the new ReadFile (golden :663-1041).
    # Same library as before the move, so no call site's link path changed.
    BarCode/BarCode.cpp
    BarCode/BarCode_Helpers.cpp
    BarCode/BarCode_Bottom2DID.cpp
    BarCode_Bottom2DID8CCD.cpp
    BarCode/BarCode_Shuttle1_Scan.cpp
    BarCode/BarCode_Shuttle2_Scan.cpp
    BarCode/BarCode_Shuttle1_CCDScan.cpp
    BarCode/BarCode_Shuttle2_CCDScan.cpp
    # --- W5-BarCode-Final (BarCode remainder front, 2026-07-11): the last 2/20
    #     TfBarCode_Shim methods (DoBarcodeScanInShuttle_2/DoShuttleFloatCheck_2,
    #     the exact "REMAINING / HANDED OFF" leftover flagged above) + the
    #     8-CCD ClientSocket connection-lifecycle glue (SendCCDCommand's REAL
    #     body + the 13 real TClientSocket globals + the Bottom-8CCD Connect/
    #     Init timer pair).  TfBarCode_Shim is 20/20 real methods as of this
    #     integrate.  See aHotPlateSubstrate.cpp's include-block comment.
    #     INTEGRATE DECISION (BarCode-8CCD-Glue's own flagged question):
    #     the 6 pre-existing per-unit gated Gated_SendCCDCommand-style no-op
    #     stand-ins (BarCode_Bottom2DID.cpp, BarCode_Shuttle1_CCDScan.cpp,
    #     BarCode_Shuttle1_Scan.cpp, BarCode_Shuttle2_CCDScan.cpp,
    #     BarCode_Shuttle2_Scan.cpp, BarCode_Bottom2DID8CCD.cpp) are LEFT
    #     SEPARATE, NOT retargeted onto the new real ::SendCCDCommand -- (1)
    #     those 6 files are already-integrated production code outside this
    #     front's 3-unit scope, and this campaign's whole premise is that
    #     serial integrates avoid touching files another front owns; (2) each
    #     already has a passing oracle test asserting its own no-op/log
    #     behaviour -- retargeting risks a silent behavioural change (real
    #     SendCCDCommand's own log format + Socket->Active/SendText branches)
    #     for zero required benefit, since every gated stand-in is already
    #     faithful to a disconnected-CCD machine; (3) consolidating call sites
    #     is a cleanup, not a correctness fix -- better done as its own
    #     deliberate follow-up wave with full re-verification of all 6 affected
    #     test suites, not a side effect of this hand-off completion.
    BarCode/BarCode_Shuttle2_ScanRemainder1.cpp
    BarCode/BarCode_Shuttle2_ScanRemainder2.cpp
    BarCode/BarCode_8CCD_Glue.cpp
    # --- PT-W8 (2026-08-11): the last 3 golden TfBarCode methods of each of
    #     BarCode_Sh1.cpp / BarCode_Sh2.cpp (InitialSFCAutoTune / DoSFCAutoTune /
    #     Do2DIDCheck) -- the trio BarCode_Shuttle1_Scan.h:32 recorded as
    #     "deferred, not touched at all by this unit".  Same target as their 7
    #     already-landed siblings above, which is what they share a substrate
    #     with (FLCarryKit/InArmSuck/MOT[]/Offset/TestIF_File + the ckernel
    #     seams SendCCDCommand/WaitManualStepKey), so no new link dependency.
    #     NOTE, stated rather than implied: nothing in this tree CALLS these six
    #     yet -- golden's callers are cContact.cpp:12289+, uhome.cpp:4311 and
    #     BarCode/BarCode.cpp:6839, all still untranslated.  They will compile,
    #     enter libht9045_sm.a, and not be extracted by the link until one of
    #     those lands.  That is expected, not a defect (archive-extraction
    #     shape (a)); it is recorded here so a later reader does not "fix" it.
    BarCode/BarCode_Shuttle1_SFCAutoTune.cpp
    BarCode/BarCode_Shuttle2_SFCAutoTune.cpp
    # --- W7-L2 substrate (2026-08-03): free-function seams that golden's
    #     ScanSystemSensor (ckernel.cpp:359-691) calls and that have NO ported
    #     home of their own -- SetWorkParameter (called golden ckernel.cpp:371,
    #     declared golden cinitial.h:17) and SetSuckRetryCount (called :496).
    #     Correct target: ckernel's substrate (Sen[]/SW[]/MOT[]/Cylinder[]/
    #     canary_support/FormsFacade/csystem.h) is exactly what ht9045_sm
    #     already carries, so no new link dependency is introduced.
    ckernel_shims.cpp
    # --- W7-L2 (2026-08-03): golden ckernel.cpp in full -- the system-sensor /
    #     panel-key / panel-lamp / motor-alarm layer.  15 of golden's 17
    #     functions are real bodies; ShowRunLed (golden :704-932) and
    #     ShowRunLabel (:935-1726) are explicitly-labelled DEFERRED stubs with
    #     call counters, because between them they make 181 fMain-> widget
    #     dereferences and belong to W7-U, not here.  One ported file per golden
    #     file per KNOWLEDGE.md:14, so this is assembled from six translation
    #     fragments in golden line order rather than split by function.
    #     ScanSystemSensor -- CORRECTED 20260803 BY INDEPENDENT AUDIT.  This comment
    #     used to claim it "has NO live caller yet" and that csystem.cpp:16894 was
    #     golden's only call site.  Both were false.  Golden has TWO call sites
    #     (exhaustive scan of 786 files over the golden tree AND the out-of-tree
    #     package D:\HT9045\elec\Component): csystem.cpp:16894, whose ported MainProc
    #     ladder really is still the three-line `#if 0` at ported csystem.cpp:415-418
    #     and so really is dormant here; and golden ckernel.cpp:2511 inside
    #     ProcessAlarm, which THIS wave translates and which calls it live at ported
    #     ckernel.cpp:2852.  Wiring the MainProc one would still be an ordering
    #     deviation, so csystem.cpp is untouched -- but the function is NOT uncalled.
    ckernel.cpp
    # =========================================================================
    # AI(W906-PT-W1) 20260807: PURE-TRANSLATION CAMPAIGN, WAVE 1.
    # Ten net-new golden units, none of which had ANY port file before this wave.
    # Measured before landing: 0 missing golden functions, 18,525/18,692 golden
    # body lines live (99.1%), mojibake 0, and all ten pass `g++ -std=c++17
    # -fsyntax-only` standalone.  They join ht9045_sm because every one of them
    # pumps the same Sim HAL substrate this list already carries (Cylinder[]/MOT[]/
    # Sen[]/SW[] + canary_support + FormsFacade + aHotPlateSubstrate) -- no new
    # link edge is introduced by any of them.
    #
    #   RotateKit/aRotateKIT.cpp      -- shared rotate-kit helpers.  MUST precede
    #     nothing (order-free) but its InitialInArmRotateKIT gate is retired by
    #     aRotateKIT_In.cpp landing in the same commit.
    #   RotateKit/aRotateKIT_In.cpp   -- in-side rotate-kit motion SM.
    #   RotateKit/aRotateKIT_Out.cpp  -- out-side mirror (translated from the Out
    #     golden file, NOT copied from In -- the two genuinely differ).
    #   TempCtrl/TriTemp.cpp          -- tri-temperature control SM.
    #   bthermo.cpp                   -- thermo/chamber control (largest of the ten).
    #   AutoRetest.cpp                -- auto-retest flow.
    #   OCRInsp.cpp                   -- OCR inspection SM.
    #   SortingBinTray/SortingBinTray.cpp -- unloader bin-tray sorting.
    #   uHeaterThread.cpp             -- heater supervisor; the VCL TThread subclass
    #     becomes the same OFFLINE-INERT standalone driver idiom acarry.cpp:7006-7290
    #     already uses for uShuttleThread.
    #   Public/MyStringList.cpp       -- TMyStringList, the type behind cmydef.h:15's
    #     tree-wide opaque forward declaration.  NOT ht9045_public: that archive
    #     links vclcompat only (:382) and this TU reads cprod/cmydef/Config/LastSet,
    #     fSCKART/fLotInfo and MyDBIProcess.  Landing it makes TMyStringList a
    #     COMPLETE type, which unblocks five existing gates elsewhere (cprod.cpp:2496,
    #     cMyDB.cpp:344/400/448/745, cmydef.cpp:129, HANA_ART.cpp:1006) -- those
    #     retirements are deliberately a SEPARATE step, not this commit.
    # =========================================================================
    RotateKit/aRotateKIT.cpp
    RotateKit/aRotateKIT_In.cpp
    RotateKit/aRotateKIT_Out.cpp
    # AI(W906-T6-MAINPROC) 20260923: RotateKit/fRotate.cpp -- TFrmRotate::DoInRotateHome/DoOutRotateHome
    #   (golden RotateKit/fRotate.cpp:1170/:1203). Needs ProcessSingleMotorHome (acatchtray_shims.cpp,
    #   this target), so it lives here, not in forms/fRotate.cpp (ht9045_forms) -- same split as cTrayForm.cpp.
    RotateKit/fRotate.cpp
    TempCtrl/TriTemp.cpp  forms/fMain_OperateMode.cpp  forms/fMain_ATCSiteUse.cpp  forms/fNote_JamCount.cpp  forms/fMain_TestSeconds.cpp   # AI(W906-J12) 20260926: fMain_TestSeconds.cpp = golden Timer2Timer test seconds (uses RecordProcess, sm).  # AI(W906-J2) 20260926: fNote_JamCount.cpp = golden note.cpp CheckRecordJamType + the FormClose Jam count; in ht9045_sm because it reads canary_support's bDuplicateErr.  # AI(W906-OPMODE) 20260926: golden TfMain::UpdateMainOperateMode／TemperatureEditDisable／SetNormalOrPrime／ChangeATCSiteUse 的本體。放 ht9045_sm 不放 ht9045_forms：它們用 ATCInterfaceForm／fTemp_Set／IsNNMode 等 sm 符號，放 forms 會造出 :683-690 禁止的 forms->sm 反向相依
    bthermo.cpp
    AutoRetest.cpp
    OCRInsp.cpp
    SortingBinTray/SortingBinTray.cpp
    uHeaterThread.cpp
    Public/MyStringList.cpp
    # =========================================================================
    # AI(W906-PT-W2) 20260807: PURE-TRANSLATION CAMPAIGN, WAVE 2 (the remainder).
    # Wave PT-W2 cleared the whole "non-form unit whose unported dependencies
    # number 0 or 1" tier -- 28 golden units.  The pure leaves went to
    # ht9045_globals, the motor drivers to ht9045_motor, myio.cpp to ht9045_io
    # and the fieldbus/protocol units to ht9045_comms (all above); these are the
    # ones that reach the machine globals, the Sim HAL or the forms facade, so
    # they land here for the same reason every other unit in this list does.
    #
    #   cSocket.cpp        -- TMySocket/TArm/TLotSummary + 8 globals.  Note the
    #       test-complete counter array at golden cSocket.cpp:108 is NOT the
    #       ProductData counter and is kept exactly where golden has it.
    #   cAuthority.cpp     -- access-level / authority gating.
    #   uruncontrol.cpp    -- calls MainProc() (csystem.cpp, THIS archive), so it
    #       cannot sit lower however leaf-like it looks.
    #   HThreadCtrl/HThreadCtrl.cpp -- thread-control helper.  MinGW here is the
    #       win32 thread model with no std::thread; follows WebBridge/Sync.h.
    #   PMAlarm/PMAlarmSystem.cpp   -- PM alarm system (its uTimeTool sibling is
    #       a pure leaf and went to ht9045_globals).
    #   ProductionInfo/{TfFTP,FileInfo,uSetUpConfiguration}.cpp
    #   Automation/{SCK_TUTS,MesWebService}.cpp
    #   Public/{HTEditList,HTEdit}.cpp -- HTEdit was not in the wave's original
    #       28-unit assignment (it sits one dependency tier up); it was
    #       translated alongside HTEditList because the two share a contract.
    #   Monitor/MonitorTCPIP.cpp -- reaches MyDBIProcess, so sm rather than comms.
    # =========================================================================
    cSocket.cpp
    cAuthority.cpp
    uruncontrol.cpp
    HThreadCtrl/HThreadCtrl.cpp
    PMAlarm/PMAlarmSystem.cpp
    ProductionInfo/TfFTP.cpp
    ProductionInfo/FileInfo.cpp
    ProductionInfo/uSetUpConfiguration.cpp
    # AI(W906-PT-W4-integrate) 20260809: wave PT-W4's four state-machine / feature
    # units.  All four reach MOT[]/Sen[]/Cylinder[]/LastSet/IniConfig and nothing
    # above this archive, same placement test the rest of this list uses.
    #   aoutarm.cpp / asortarm.cpp -- the OUT-ARM and SORT-ARM engines (63 and 64 free
    #     functions respectively). Siblings of aoutarm9045.cpp already in this list.
    #   ProductionInfo/uPAT_Function.cpp -- PAT_Function (102 methods) +
    #     TMachineTimeManager (8). Joins its ProductionInfo/ siblings three lines up.
    #   PowerSavingMode.cpp -- TPowerSaving + the TModule family.
    aoutarm.cpp
    asortarm.cpp
    ProductionInfo/uPAT_Function.cpp
    PowerSavingMode.cpp
    #   SECSGEM/uHGemHT9045_{SV,EC}.cpp -- MOVED HERE from ht9045_secsgem on the second
    #     integrate pass.  Each is one enormous HT9045Gem:: method holding golden's SV /
    #     EC dictionary, and each takes the address of objects that live in THIS archive
    #     (cSocket.cpp's ArmData/ArmHistory, Automation/AGV_PortScan.cpp's
    #     iLoadStateATK/sOutputBinCode, acatchtray_shims.cpp:189's iPortStatus -- note
    #     cmydef.cpp:5950 also defines iPortStatus but inside its `#if 0`, so the shim
    #     holds the only live one).  ht9045_secsgem cannot see any of that by design.
    #     ht9045_sm links ht9045_secsgem, so HGem/SecsSvEcRegistration still resolve.
    #     uHGemHT9045_SV.cpp:333-345 documented this before either file was registered.
    SECSGEM/uHGemHT9045_SV.cpp
    SECSGEM/uHGemHT9045_EC.cpp
    Automation/SCK_TUTS.cpp
    Automation/MesWebService.cpp
    Public/HTEditList.cpp
    Public/HTEdit.cpp
    Monitor/MonitorTCPIP.cpp
    # -----------------------------------------------------------------------
    # AI(W906-PT-W3-integrate) 20260808: wave PT-W3.  These 13 units all reach
    # ABOVE the narrow archives (cprod/cmydef globals, MOT[]/Cylinder[]/SW[]/Sen[],
    # FormsFacade, database.h's HSys, SCK_ART_Remainder's SckArt* state), so they
    # land here rather than widening ht9045_io / ht9045_automation / ht9045_comms
    # link surfaces for one file each.  PT-W3's three motor drivers went to
    # ht9045_motor and CosFunction.cpp to ht9045_globals -- see those blocks.
    #
    # Two stub retirements were FORCED by this batch (same shape as PT-W2's
    # uPlateInfo: a stub was hiding a real hole, and the real body arriving turns
    # it into a `multiple definition`):
    #   * Motor/mymotor.cpp OpenPCI132Card       -> Motor/myMN200motor.cpp:1154
    #   * aHotPlateSubstrate.cpp CheckInArmFloating -> OmronLaser/LaserSensorInArm.cpp
    #
    # *** AI(W906-A4-6) 20260924: mykitsuck.cpp 已註冊（ht9045_sm，:1888）—— 下面這段是註冊前的史料。原標題：DELIBERATELY NOT REGISTERED IN THIS WAVE: mykitsuck.cpp ***
    # It is translated, on disk, and syntax-clean -- but registering it is a
    # SUBSTRATE HOMECOMING, not a file addition, and it is scheduled as its own wave:
    #   * mykitsuck.h:274 defines `class TMyKitSuck` and aHotPlateSubstrate.h:365
    #     ALSO defines `class TMyKitSuck`, with a DIFFERENT layout (substrate starts
    #     `TMySucker Suck[][]`; golden starts `TALed *pLed[][]` + `bool bLed[][]`).
    #   * Both headers declare the same extern globals, and mykitsuck.cpp:205-234
    #     defines 24 of them against aHotPlateSubstrate.cpp:80-93/:1067-1070's 14.
    #     Fourteen collide outright (InArmSuck / FLCarryKit / BLCarryKit / OutArmSuck /
    #     OutArm2Suck / FRCarryKit / BRCarryKit / CatchTraySuck / TestSocket /
    #     FTestSuck / BTestSuck / ptrInSHT / ptrInSHTBackup / OutArmSuckBackup).
    #   * aHotPlateSubstrate.h is #included by 177 TUs, so retiring its TMyKitSuck is
    #     a tree-wide change that must be measured on its own.
    # mykitsuck.h's own banner already said this pair "is NOT #included by anything
    # yet, and NOT added to CMakeLists.txt" -- that is still the right call today.
    cInArmPlacement.cpp
    handlerlog.cpp
    MyTempPanel.cpp
    ScanBtnThread.cpp
    TfAOILaserScan.cpp
    ATC/ATCInterface.cpp
    OmronLaser/LaserSensor.cpp
    OmronLaser/LaserSensorInArm.cpp
    OmronLaser/LaserSensorShuttle.cpp
    VacuumUnit/VacuumUnit.cpp
    VacuumUnit/MyVacuumPanel.cpp
    Automation/uRENESAS_Server.cpp
    # AI(W906-FW-W23/24/25) 20260826: 三個全新的表單 facade（FW-TRAYASSIGN-W23 /
    # FW-MOTORTEST-W24 / FW-TEACH-W25）。**這是本 list 裡的頭三個 forms/*.cpp。**
    #
    # 為什麼放這裡而不是 ht9045_forms —— 落點是用 nm 量的，不是採信 agent 的建議
    # （兩個 agent 對此給了相反答案，其中一個建議 ht9045_forms 是錯的）：
    # ht9045_forms 只 link vclcompat + ht9045_globals + ht9045_core（:714-721），
    # 而這三個 TU 的未定義符號各自需要 forms 拿不到的東西：
    #   fTeach.cpp          sm 5（IsEMGPressed / ShowMyMessage / ShowErrorMessage /
    #                       fLtcSensor / InArmSuck）、motor 7（MOT / TMyMotor::ReadPos /
    #                       ScanMotorStatus …）、io 4（Sen / Cylinder / TMySensor::IsOn …）
    #   fMotorTest.cpp      sm 1（ShowMyMessage）、motor 6（MOT / HTMotor::ReadAcc /
    #                       ReadDec / ReadRange …）、io 2（Sen / TMySensor::IsOff）、
    #                       core 1（MotTablePath）
    #   fTrayAssignment.cpp motor（MOT / TTrayMotor）、sm（HasICUnderMachine /
    #                       HasAnyICInMachine / fShowBinSelect）
    # 那些 motor/io 符號**全部是讀取類**（ReadPos / ScanMotorStatus / IsOn / IsOff），
    # 與三個波宣稱的「唯讀方向」一致，可當作對該宣稱的獨立佐證。
    # 放進 ht9045_forms 會靠 GNU ld 的 lazy extraction 偷連上，複製 :612-646 記錄過的
    # forms->sm undeclared back-edge。
    #
    # 目前沒有任何已編譯的碼參照這三個類別（三個波各自收工重跑過 absence claim），
    # 所以不會產生新的 forms->sm 反向需求。
    # AI(W906-FW-CONTACT-W26) 20260826: 同樣用 nm 量過落點。forms/fContact.cpp 的
    # 79 個未定義符號裡有 8 個只有 ht9045_sm 提供：IndexHasIC / ShuttleHasIC /
    # ShowMyMessage / Barcode_Reader / TMyKitSuck::HasIC / iCloseSiteModeFor2x8 /
    # InArmSuck / OutArmSuck，另有 MOT（ht9045_motor）。
    # 波次 agent 擔心「若落在 ht9045_forms，IndexHasIC/ShuttleHasIC 這條邊會撐破
    # 該 target 的 link diet」，並建議退而 gate 掉 timerContactTimer。
    # **不採用那個備案**：落在 ht9045_sm 就沒有那條邊要撐，那支方法保持完整。
    forms/fContact.cpp
    # AI(W906-FW-TRAYMAP-W32) 20260826: 落點同樣用 nm 量的（agent 也建議 ht9045_sm，
    # 但這是量出來的不是採信）。72 個未定義符號裡有 10 個 ht9045_forms 拿不到：
    #   sm    ShowMyMessage / TfAOI::~TfAOI
    #   motor MOT / TMyMotor::ReadPos / TMyMotor::GetSpeed
    #   io    Cylinder / TMyCylinder::OnSensor / OnStatus / OffSensor / OffStatus
    #
    # ⚠ 這個 facade 的類別叫 **TfTrayMappingForm** 不是 TfTrayMapping——
    # 後者的名字已被 acatchtray_shims.h:219 的 shim 類別佔用（:313 還有
    # `extern TfTrayMapping *fTrayMapping`），比 fContact 那次的衝突更深
    # （那次 shim 叫 TfContactShim，golden 的類別名是空的）。
    # 因此本 facade **完全不宣告全域指標**，fTrayMapping 仍指向 shim，全樹行為零改變。
    #
    # 與 acatchtray_shims.cpp（:1776，同 archive）共存是安全的：類別名不同、無共用符號。
    forms/fTrayMapping.cpp
    # AI(W906-FW-BOOTSTRAP-W35) 20260827: 表單 bootstrap。落點用 nm 量的：
    # 12 個未定義符號裡 **6 個只有 ht9045_sm 提供**（TFormBarcodeReader/TfDynamicTemp/
    # TfVacuumUnit 三個 ctor + 三個同名全域），另 2 個來自 ht9045_forms（TfPassword 那組）。
    # 因為 ht9045_sm 已 link ht9045_forms（:2284-2295）而**反向會 configure 失敗**
    # （target cycle，CMakeLists.txt:589-591 白紙黑字記過），所以只能落 ht9045_sm。
    # 放樹根而非 forms/ 也是這個理由——forms/ 對應的是 ht9045_forms。
    #
    # ⚠ 本 TU 目前**沒有任何呼叫者**，所以它不改變任何執行期行為、也不可觀察。
    # 行為變更會在「決定 InitForms() 的呼叫點」那一波才落地，屆時要重讀
    # FormsBootstrap.cpp 的 MEASURED BEHAVIOUR DELTA 段。
    FormsBootstrap.cpp
    forms/fTrayAssignment.cpp
    forms/fMotorTest.cpp
    forms/fTeach.cpp
    # AI(W906-TEACH-W1) 20260919: teach 參數登錄表 + 讀檔。落在 ht9045_sm 而不是
    # ht9045_forms，理由與上面 forms/fTeach.cpp 同一條（:2710-2717 的 target cycle）。
    #
    #  * fTeachPara.cpp     —— TECH_PARA / TECH_TWOPARA / TECH_MotorAxle /
    #                          TECH_SUCKPARA 的本體，加 TfTeach::ReadFile()。
    #  * fTeachRegistry.cpp —— **產生的**（tools/gen_teach_registry.py），
    #                          440 個 push_back，是 golden uteach.cpp:342-994
    #                          的可重跑變換。不要手改。
    #
    # 為什麼現在要它們：START 的唯一阻擋是 Tech.* 全 0
    # （WebStart.cpp:1125 CompareTechData 否決），而填 Tech.* 的整條鏈
    # （ReadTechData -> fTeach->ReadFile -> TechPara[i]->ReadFromFile）
    # 在這棵樹沒有本體。計畫：docs/PLAN_START_TO_RUN.md P0-1。
    forms/fTeachPara.cpp
    forms/fTeachRegistry.cpp
    # AI(W906-FW3-PIMOVE) 20260828: 從 ht9045_forms 搬過來（原在 :674）。
    # 純連結圖變更，**沒有任何 .cpp/.h 內容被改**。
    # 落點是用 nm 量的（btq2 建出來的 archive），兩個方向都量了：
    #
    #  (a) 搬走會不會擱淺別人：它定義 199 個外部符號，
    #      **ht9045_forms 裡其他所有 object 對這 199 個的引用數 = 0**；
    #      九個 archive 裡只有 **ht9045_sm 需要 1 個（`_fProductionInfo`）**，
    #      secsgem/db/core/public/comms/automation/kyecftp/webbridge 全是 0。
    #      所以這一搬把一個跨 archive 相依（sm -> forms）變成同 archive 解析，
    #      **不新增任何邊，反而少一條**。
    #  (b) 搬過來會不會自己連不到：它需要 55 個未定義符號，
    #      globals 满足 9、vclcompat 满足 10、**只有 ht9045_forms 能满足的只有 1 個：`_fMain`**。
    #      而 ht9045_sm **顯式** link ht9045_forms（本檔 :2716），所以 `_fMain` 仍然連得到。
    #
    # 目的：`TfProductionInfo` 還有 10 支方法卡在「`ht9045_forms` 連不到
    # `ht9045_sm`」這道邊上（其中 `CalculateOEEReport` 三次解參考 `fObserver`，
    # golden :805/:806/:813）。搬完之後它們才可能被翻譯成 ACTIVE。
    # ⚠ **本 commit 只搬來源，不翻任何方法、不解任何 gate。**
    # ⚠ 安全軸仍然獨立：`LoadMOInformation`（寫檔/寫 ini）與
    #   `CheckOEE_WhenStart`（送命令/上傳）**即使連結問題解掉也不能翻**（見 DEVLOG 20260828 IX/XII）。
    forms/fProductionInfo.cpp
)

target_include_directories(ht9045_sm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_sm PUBLIC
    ht9045_io ht9045_motor ht9045_globals vclcompat
    # W5-comms INTEGRATE: EventReport()/SECS_EVENT (csystem.cpp) + link-
    # completeness for Interface/InterfaceSYS.cpp (see source-list comment above).
    ht9045_secsgem
    # AI(W906-W7-F0) 20260728: the ~60 TUs in this library that #include
    # "FormsFacade.h" used to get fMain/fLotInfo/... from FormsFacade.cpp inside
    # this same archive; it now lives in ht9045_forms.  (Strictly redundant with
    # the transitive edge via ht9045_secsgem above, but declared explicitly
    # because ht9045_sm's own sources are the direct consumers -- if the secsgem
    # edge is ever narrowed, this must not silently break.)
    ht9045_forms)
    # AI(W906-FW-TEMP3) 20260820: an sm->comms PUBLIC edge was TRIED here and
    # REVERTED same evening -- uTemp_Set.cpp (this archive) calls ATC60System::*
    # whose bodies live in ht9045_comms (ATC/ATCSystem.cpp, :1283 above), but a
    # direct edge makes CMake's LINK_GROUP graphs cyclic (comms itself depends
    # on group members like ht9045_globals, so group->sm->comms->group). The
    # repo's convention for this is the one test_lane_io_sim already uses:
    # ht9045_comms goes INSIDE each consuming test's RESCAN group
    # (tests/CMakeLists.txt, five groups updated this wave).

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # The SM carries BCB6 idioms inherited from the global headers (AnsiString
    # brace-init etc.).  Match the ht9045_io / ht9045_globals warning posture so
    # the translated stepper compiles without tripping on already-accepted
    # global-header idioms, while still flagging genuine porting mistakes.
    target_compile_options(ht9045_sm PRIVATE
        -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)

    # AI(W906-PT-W4-integrate) 20260809: SECSGEM/uHGemHT9045_EC.cpp is capped at -O1.
    # NOT a style choice and NOT a warning carve-out -- without it the RELEASE build
    # FAILS OUTRIGHT:
    #     cc1plus.exe: out of memory allocating 65536 bytes
    # The file is golden's EC (equipment-constant) dictionary: ~1,817 statements in ONE
    # function (HT9045Gem_AddEC), essentially one straight-line basic block of
    # SetECDataPointer calls.  GCC's -O2/-O3 pipeline is superlinear in basic-block
    # size, and MinGW g++ 6.3.0's cc1plus is a 32-bit process, so it exhausts its own
    # address space on this one translation unit.
    # MEASURED, not guessed -- the file compiled ALONE, no parallel-build memory
    # pressure, so this is a genuine per-TU limit rather than a -j artefact:
    #     -O3  OOM after 50s     -O2  OOM after 45s
    #     -O1  OK   after 35s    -O0  OK   after  4s
    # -O1 is therefore the HIGHEST level that works and is what is pinned: the
    # deviation from the project's Release posture is the smallest one available.
    # SCOPE IS ONE FILE, also measured: the sibling SECSGEM/uHGemHT9045_SV.cpp (the SV
    # dictionary, 923 statements) compiles clean at -O3 in 8s, so it is deliberately
    # NOT capped.  If a future wave splits AddEC into per-section functions, delete
    # this and let it take the archive's normal flags.
    # It appends, so ht9045_sm's warning flags above still apply to this file.
    set_source_files_properties(SECSGEM/uHGemHT9045_EC.cpp PROPERTIES COMPILE_FLAGS "-O1")
endif()

# W6.6: enable the csystem.cpp tick-sequence ORACLE hook (compiles a tiny
# std::vector trace into DoAllProcess so the W6.6 verify TU can confirm the
# documented per-tick engine ORDER).  The CSYS_TICK macro is a NO-OP unless this
# is defined, so the faithful golden body is untouched in spirit; the trace is
# write-only instrumentation read only by the integration test.
target_compile_definitions(ht9045_sm PRIVATE CSYSTEM_TICK_ORACLE)

# ---------------------------------------------------------------------------
# Library: ht9045_kyecftp  (W5-Final KYECFTP wave -- FTP client engine +
#   TfFTPClient session logic)
#
#   KYECFTP/MiniFtpEngine.{h,cpp}            -- Nmftp::TNMFTP: a from-scratch FTP
#     protocol engine standing in for the closed BCB6 TNMFTP component (no vendor
#     source exists anywhere in this repo -- see the file's own banner for every
#     inferred wire-behavior judgment call: async->sync, PASV-only, Vendor/
#     ParseList inert, CurrentDir's live PWD round-trip, Mode()'s MODE_BYTE->
#     "TYPE L 8" mapping, 502->OnUnSupportedFunction classification). Built on
#     vclcompat/ClientSocket.h (one control-channel connection + a fresh PASV
#     data-channel connection per transfer). Zero dependency beyond vclcompat.
#   KYECFTP/FTPClient_EventHandlers.{h,cpp}  -- TfFTPClient's NMFTP1 event-handler
#     bodies (status/success/failure logging), as free functions matching
#     TNMFTP's std::function callback-slot signatures. Needs CUSTOMER_CODE
#     (cmydef.cpp) + MyDBIProcess (aHotPlateSubstrate.cpp) + ShowMyMessage
#     (canary_support.cpp).
#   KYECFTP/FTPClient_Transfer.{h,cpp}       -- the 4 real FTP-transfer session
#     functions (LoadFileFormServer2/UploadFileToServer2/Download_2DSortingList/
#     Download_2DID_WhiteList), built against both siblings above. Needs
#     CUSTOMER_CODE/IniConfig/CosFunction/Temperature (cprod.cpp/cmydef.cpp) +
#     DataPath/OffsetPath/DefaultPath/LastDataPath/asATCFileTransferPath/
#     sWhite2DIDListLoc (common.cpp) + MyDBIProcess/NewRecordProcess/MySleep/
#     ShowMyMessage/RecordProcess (aHotPlateSubstrate.cpp/acatchtray_shims.cpp/
#     acarry_shims.cpp/canary_support.cpp, all in ht9045_sm).
#
#   PRODUCTION link surface: vclcompat (MiniFtpEngine's transport) + ht9045_sm
#   (CUSTOMER_CODE/IniConfig/CosFunction/Temperature transitively via
#   ht9045_globals, + the 5 real free-function bodies above) + ht9045_core
#   (common.cpp's DataPath family -- ht9045_sm does NOT itself link ht9045_core,
#   see that library's own link line above, so it must be named explicitly here
#   too). NOT exercised end-to-end by this library's own unit tests below --
#   see those targets' own comments in tests/CMakeLists.txt for why: each test
#   supplies TU-local stand-in globals/functions instead, deliberately avoiding
#   the cmydef/cprod/common/canary_support/aHotPlateSubstrate "god-stack" for a
#   fast, focused unit test (same avoidance already established by
#   tests/test_config_loaders.cpp / tests/test_MyCCLink.cpp) -- ht9045_kyecftp
#   itself compiling clean here is what proves these 3 files type-check against
#   the REAL header declarations (cmydef.h/cprod.h/Config.h/CosFunction.h/
#   common.h/canary_support.h), which the units' own narrower manual
#   verification could not check.
#
#   No circular dependency: grepped the whole tree (production + tests) for any
#   existing caller of LoadFileFormServer2/UploadFileToServer2/
#   Download_2DSortingList/Download_2DID_WhiteList/Nmftp:: outside KYECFTP/
#   itself -- zero hits. ht9045_sm/ht9045_core need nothing from ht9045_kyecftp,
#   so this is a clean one-way dependency (unlike Interface/InterfaceSYS.cpp,
#   which had a genuine two-way need and was folded directly into ht9045_sm's
#   own source list instead -- see that library's comment above).
# ---------------------------------------------------------------------------
# AI(W906-ELA-R0) 20260927 (St02; the laptop's OK, TO_STEVEN §4 11:4x): the Nmftp::TNMFTP engine gets its own archive,
#   so ht9045_kyecftp (FTP client, god-stack) and ht9045_ela (the ELA reports / uploads plan, R4 -- no symbol used yet)
#   share ONE copy.  MiniFtpEngine.cpp depends on vclcompat only (its own unit's report); ws2_32 used to reach it through
#   ht9045_kyecftp's PUBLIC link and is now named here.  Acceptance: `nm --defined-only` shows the Nmftp::TNMFTP symbols
#   in exactly one archive (libht9045_nmftp.a).  The unit tests that compile ../KYECFTP/MiniFtpEngine.cpp directly
#   (tests/CMakeLists.txt MiniFtpEngine / FTPClient_EventHandlers / FTPClient_Transfer) are unchanged.
add_library(ht9045_nmftp STATIC
    KYECFTP/MiniFtpEngine.cpp
)
target_include_directories(ht9045_nmftp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ht9045_nmftp PUBLIC vclcompat)
if(WIN32)
    target_link_libraries(ht9045_nmftp PUBLIC ws2_32)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ht9045_nmftp PRIVATE -Wall -Wextra)
endif()

add_library(ht9045_kyecftp
    KYECFTP/FTPClient_EventHandlers.cpp          # AI(W906-ELA-R0) 20260927: MiniFtpEngine.cpp moved to ht9045_nmftp (linked PUBLIC below)
    KYECFTP/FTPClient_Transfer.cpp
)

target_include_directories(ht9045_kyecftp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_kyecftp PUBLIC ht9045_nmftp vclcompat ht9045_sm ht9045_core)   # AI(W906-ELA-R0): ht9045_nmftp PUBLIC, so every user still gets TNMFTP

if(WIN32)
    target_link_libraries(ht9045_kyecftp PUBLIC ws2_32)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Same relaxed posture as ht9045_sm/ht9045_comms: FTPClient_EventHandlers.cpp/
    # FTPClient_Transfer.cpp #include the BCB6-idiom-heavy global headers
    # (cmydef.h/cprod.h/Config.h/CosFunction.h/common.h). MiniFtpEngine.cpp itself
    # (zero god-stack dependency) is independently clean under the full strict
    # set per its own unit's report; no per-file carve-out is needed since the
    # relaxed flags here are a strict subset of warnings, not additions.
    target_compile_options(ht9045_kyecftp PRIVATE
        -Wall -Wextra -Wno-conversion-null -Wno-write-strings -Wno-unused-parameter)
endif()


# ---------------------------------------------------------------------------
# Probe library: ht9045_pci1203_probe
#   AI(W906-1203HAL-2) 20260820: compile-only regression guard for the
#   user-ruled PCIE-1203 direction. 1203HAL-1 made -DHAVE_PCI1203=1 compile
#   for the first time (MOTION_IO rename wrap + EtherCAT/AdvMotCompat.h's
#   ADVCMNAPI/#undef-Direct shims); this target recompiles the six
#   vendor-consumer TUs under that flag on every build so a future edit
#   cannot silently regress it back to "explodes on contact".
#
#   AI(W906-BU-C1) 20260916: the banner said "six" while the list held SEVEN --
#   a stale count.  It is EIGHT now: EtherCAT/Pci1203Monitor.cpp joined, the
#   first source here that is NOT a duplicate of an ht9045_sm member.  It is
#   the colleague's PCIE-1203 read-only monitor, landed by BU-C1 (see
#   docs/BU_C_CAMPAIGN_PLAN.md).  Landing it HERE and nowhere else is the
#   point: compile-proven, linked into nothing, therefore NOT armed.
#   KNOWN GAP, stated rather than hidden: because this target is the file's
#   only registration, the build only ever compiles it WITH HAVE_PCI1203=1.
#   The flag-off arm was checked by hand this wave (g++ -fsyntax-only, 0
#   errors both ways) but nothing re-checks it per build.  That is BU-C2.
#   AI(W906-BU-C4) 20260918: BU-C4 ran, so the line that used to say
#   "Pci1203Control.{h,cpp} -- the WRITE surface -- is deliberately NOT here"
#   is now void. It IS here, and it is also in wb_serve (see that target).
#   Keeping it in BOTH is the point, and it closes half of the BU-C2 gap stated
#   just above for this one file: this target compiles it with HAVE_PCI1203=1,
#   wb_serve compiles it with the flag OFF, so EVERY build now type-checks BOTH
#   arms of the write surface. That gap was not theoretical -- the flag-off arm
#   did NOT compile when it landed (three vendor spellings used outside the
#   guards; see the file's own header note and RULINGS_20260917.md).
#
#   STATIC on purpose and linked into NOTHING: an archive does not resolve
#   symbols, so the unresolved Acm_* vendor imports are fine -- linking the
#   real SDK .lib is 1203HAL-3's problem, compilability is this target's.
#   The object files are duplicates of ht9045_sm members under a different
#   flag; because no executable links this archive, no ODR/duplicate-symbol
#   contact exists.
# ---------------------------------------------------------------------------
add_library(ht9045_pci1203_probe STATIC
    cinitial.cpp
    IOBackend.cpp
    MyLaneIo.cpp
    Motor/myEthercatmotor.cpp
    EtherCAT/MyEtherCAT.cpp
    EtherCAT/MyNUEC1.cpp
    VacuumUnit/MyVacuumPanel.cpp
    EtherCAT/Pci1203Monitor.cpp      # AI(W906-BU-C1) 20260916 -- not a duplicate; see banner
    EtherCAT/Pci1203Control.cpp      # AI(W906-BU-C4) 20260918 -- write surface, flag-ON arm
)
# AI(W906-BU-C-P5) 20260918: ⚠ 同事 20260916b 的 CMakeLists 在這一段還多了一組
#   `find_library(ADVMOT_APP_LIB ...)` + `foreach(_t ht9045_io ht9045_motor
#   ht9045_sm) target_compile_definitions(${_t} PRIVATE HAVE_PCI1203=1)`，
#   也就是把 1203 武裝到三個**生產**函式庫上。
#
#   P5 **刻意沒有取它**。A 樹的姿態是「只武裝探針」：下面這個 target 開
#   HAVE_PCI1203=1 是為了證明那條路徑編得過，不是為了讓它會動。取了那一組，
#   任何裝了 Advantech SDK 的機器一建置就會把真實廠商呼叫連進 io/motor/sm。
#
#   逐條裁決與理由在 docs/PCI1203_20260918_INTEGRATION_PLAN.md §12。
#   要改變這個姿態請從那一節開始讀，不要只是把同事的那幾行貼回來。
#   AI(W906-Q34-ARM) 20260923: 使用者裁決改成「找得到 SDK 就武裝 **wb_serve**」（§12 第 4 列被推翻），
#   但**這三個生產函式庫仍然不武裝** —— 本段的理由對它們照舊成立。見 wb_serve target 後的 Q34-ARM 區塊。
target_compile_definitions(ht9045_pci1203_probe PRIVATE HAVE_PCI1203=1)
target_include_directories(ht9045_pci1203_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/EtherCAT/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/Motor/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/SECSGEM
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite3
)

# ---------------------------------------------------------------------------
# Link probe: pci1203_linkprobe
#   AI(W906-1203HAL-3) 20260820: the x86 ADVMOT.lib is a standard COFF import
#   library MinGW links directly (docs/RECON_1203_SDK_linkability.md). This
#   tiny exe proves it on every build of a machine that has the SDK: real
#   Acm_* imports resolved by the real vendor .lib. BUILD-ONLY -- never a
#   ctest (without a card the enumerate call blocks 15+ s, and running needs
#   ADVMOT.dll on PATH). Machines without the SDK skip the target entirely.
#   Beware the decoy: Public/AdvMotAPI.dll is a DIFFERENT component;
#   the header-matching library is ADVMOT.lib / ADVMOT.dll.
# ---------------------------------------------------------------------------
find_library(ADVMOT_X86_LIB
    NAMES ADVMOT
    PATHS "C:/Program Files (x86)/Advantech/Common Motion/Public"
    NO_DEFAULT_PATH
)
if(ADVMOT_X86_LIB)
    add_executable(pci1203_linkprobe tools/pci1203_linkprobe.cpp)
    target_compile_definitions(pci1203_linkprobe PRIVATE HAVE_PCI1203=1)
    target_include_directories(pci1203_linkprobe PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/EtherCAT/vendor
    )
    target_link_libraries(pci1203_linkprobe PRIVATE ${ADVMOT_X86_LIB})
    message(STATUS "PCIE-1203 link probe enabled: ${ADVMOT_X86_LIB}")
else()
    message(STATUS "PCIE-1203 SDK not found -- pci1203_linkprobe skipped (compile probe still active)")
endif()

# ---------------------------------------------------------------------------
# Library: ht9045_webbridge
#   AI(W906-WebBridge) 20260805: the C++ side of the browser HMI seam --
#   D:\HT9045\web talks to the handler through this. Design and wire protocol:
#   D:\HT9045\web\docs\ARCHITECTURE.md (option A, sections 4 and 5).
#
#   WHY IT LINKS ALMOST NOTHING
#   This layer is deliberately independent of vclcompat, of the state machines,
#   and of every machine header. It knows exactly two things about the machine:
#   a TagSnapshot the UI thread publishes into, and a CommandQueue the UI thread
#   drains. That is the whole seam. Keeping the dependency list this short is
#   what lets the socket thread be reasoned about in isolation -- and the socket
#   thread must never block the UI thread, because the SECS/GEM layer on that
#   thread has a reply budget (ARCHITECTURE.md section 5).
#
#   It DOES link ht9045_public (for the vendored cJSON its inbound-command
#   parser uses), which transitively brings in vclcompat. That is a link-level
#   dependency only -- no WebBridge source includes a vclcompat or machine
#   header, and that source-level independence is the property that matters for
#   the migration. See the note at ht9045_public above for why cJSON was not
#   split out to remove even the link-level dependency.
#
#   Threading note: this tree's MinGW oracle is thread model win32, which has no
#   working <mutex>/<thread>. WebBridge/Sync.h supplies CRITICAL_SECTION-based
#   equivalents. std::atomic IS available and is used directly.
# ---------------------------------------------------------------------------
add_library(ht9045_webbridge
    WebBridge/Sha1.cpp
    WebBridge/Base64.cpp
    WebBridge/JsonWriter.cpp
    WebBridge/TagValue.cpp
    WebBridge/TagSnapshot.cpp
    WebBridge/CommandQueue.cpp
    WebBridge/WsHandshake.cpp
    WebBridge/WsFrame.cpp
    WebBridge/HttpStatic.cpp
    WebBridge/WebBridgeServer.cpp
    # AI(W906-WebBridge-Tcp) 20260812: the sidecar seam. TagJson is the single
    # tag<->JSON definition (WebBridgeServer.cpp forwards to it); TcpTagLink is
    # the loopback, read-only TCP link that lets the web server run in a
    # DIFFERENT process from the machine data. See WebBridge/TcpTagLink.h for
    # why SECS was rejected as that transport (uHGemEquipment.cpp:2005-2013).
    WebBridge/TagJson.cpp
    WebBridge/TcpTagLink.cpp
)

target_include_directories(ht9045_webbridge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ht9045_webbridge PUBLIC ht9045_public)

if(WIN32)
    target_link_libraries(ht9045_webbridge PUBLIC ws2_32)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Full strictness: this is new-build code with no BCB6 idiom to accommodate,
    # so unlike the translated targets it gets no relaxations.
    target_compile_options(ht9045_webbridge PRIVATE -Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# Tool: wb_serve
#   AI(W906-WebBridge) 20260806: runs the browser HMI against the REAL machine
#   data layer -- LoadMachineConfig() + WebBridgeTags + the bridge, serving
#   D:\HT9045\web. This is the WB-2 milestone made runnable; the handler itself
#   will own the server once GA-3 puts the god-stack in HT9045.exe.
#
#   It links BOTH the god-stack (via WebBridgeTags.cpp, the one TU allowed to
#   mix the two worlds) and ht9045_webbridge. LINK_GROUP:RESCAN for the same
#   single-pass archive-scan reason the god-stack tests use.
# ---------------------------------------------------------------------------
# Steven 20260924 (S12)：FileRW/ 一個結構一支 cpp。審查第 8 輪 M-2：不用 GLOB（半成品一落地就會被編進來），
# 只收兩個產生器寫的正面清單：gen_editlist.py → _editlist_sources.cmake（C 形狀：共用層＋各結構入口）、
# gen_formbridge.py（不帶 --only）→ _formbridge_sources.cmake（A 形狀：已整合的 golden 表單 bridge）。
include(${CMAKE_CURRENT_SOURCE_DIR}/FileRW/_editlist_sources.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/FileRW/_formbridge_sources.cmake)
set(W906_FILERW_SRC)
foreach(_f ${W906_EDITLIST_SRC} ${W906_FORMBRIDGE_SRC})
    list(APPEND W906_FILERW_SRC ${CMAKE_CURRENT_SOURCE_DIR}/${_f})
endforeach()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/FileRW/_editlist_sources.cmake ${CMAKE_CURRENT_SOURCE_DIR}/FileRW/_formbridge_sources.cmake)
if(WIN32)
    enable_language(RC)   # Steven 20260925：tools/wb_serve.rc 版本資源（golden asHandlerVersion 讀 exe 版本，main.cpp:9759-9761）
endif()
add_executable(wb_serve
    tools/wb_serve.cpp
    $<$<PLATFORM_ID:Windows>:tools/wb_serve.rc>   # Steven 20260925：版本資源 3.33.906.0
    WebBridgeTags.cpp
    # AI(W906-BU-C3) 20260916: WebBridgeTags.cpp now publishes the 63 pci1203.*
    #   tags, so this binary needs the monitor's definitions.  Registering it in
    #   ht9045_pci1203_probe alone (BU-C1) proved it COMPILES and nothing more --
    #   that archive is STATIC and linked into nothing, so the first buc3 gate died
    #   with 55 undefined references at link.  -fsyntax-only cannot see that.
    #   HAVE_PCI1203 stays OFF here: every vendor call, and the AdvMotCompat.h
    #   include itself, sits inside `#if HAVE_PCI1203` (Pci1203Monitor.cpp:55-58),
    #   so this adds an inert read-only observer, not an armed one.
    EtherCAT/Pci1203Monitor.cpp
    # AI(W906-BU-C4) 20260918: the WRITE surface. Landed here and NOT in a
    #   sidecar for the same reason as WebBridgeRecipeDoc.cpp below -- only the
    #   process that holds both the server and the machine data can execute a
    #   browser-initiated command. With the 20260918 one-executable ruling that
    #   process is wb_serve, and the BU-C4 prerequisite "get the colleague's
    #   wb_publish.cpp / TcpTagLink diff" dissolves with it: his file only
    #   MENTIONS them in prose, it has no code dependency on either (measured).
    #   HAVE_PCI1203 stays OFF here, as for the monitor above, so on this laptop
    #   Pci1203ControlEnable() refuses with "not linked" -- an honest refusal the
    #   browser can see, not a fake success. The colleague's machine builds the
    #   same file with the flag on.
    #   AI(W906-Q34-ARM) 20260923: 同上 —— 有 SDK 的機器上本 target 自己就會帶旗標，
    #   不再需要同事那邊另外開；那台機器上網頁一次點擊就會真的送到卡片。
    EtherCAT/Pci1203Control.cpp EtherCAT/Pci1203IoRoute.cpp JsonBridge/IoBtnPanelClick.cpp   # AI(W906-IOWEB-P17) 20260925: A4-7 engine IO -> 1203 route (MachineType.h EOF WB_ENGINE_IO_1203) + HW.IoSetView's io.btnPanelClick (golden BtnPanelClick); no vendor call of their own -- Execute() above makes it. Same line, no line below moves
    WebAuth.cpp  WebModalWake.cpp   # AI(W906-FW-W2) 20260819: auth.login verification core.  AI(W906-MODAL-WAKE) 20260926: WebModalWake.cpp = RULINGS_20260926 #9 (blocking dialog with no web page -> open the browser)
    WebLogin.cpp  # Steven 20260924: golden TfMain cbUserSelectChange / stOperatorClick / btLoginClick
    WebSmartDiag.cpp  # Steven 20260925 (Data.SmartDiagnostic): WS smartdiag.op JSON wrapper
    WebRecipeChange.cpp  WebReadChainCalls.cpp   # AI(W906-RCHG) 20260925: WS recipe.change (golden cbSetupFileNameChange/ChangeSetUpFile) + DoReadLastData call shims
    WebBuilder.cpp    # Steven 20260925 (Data.Builder): WS builder.op JSON wrapper (create/delete/import/export recipe, Steven approved web create/delete)
    WebSortCT.cpp  WebLotInfo.cpp  WebTowerLight.cpp  WebSecurityJam.cpp  # Steven 20260925 (Data.SortCT): Clear Count button JSON wrapper (JsonWriter/cJSON) around cSortCT.cpp btnClearCountClick; dispatch still to be added by the integrator  //  Steven 20260925 (Data.LotInfo): WebLotInfo.cpp = WS lotinfo.op JSON wrapper around forms/fLotInfo.cpp (golden TfLotInfo), same line so nothing moves  //  AI(W906-TOWERLIGHT) 20260925 (Steven 團隊): WebTowerLight.cpp = WS towerlight.op JSON wrapper around forms/fTowerLight.cpp (golden V912 TfTowerLight), same line so nothing moves  //  Steven 團隊 20260926 (Status.Security Jam): WebSecurityJam.cpp = WS security.jam JSON wrapper around cSecurity.cpp (golden V912 TfSecurity Jam tab), same line so nothing moves
    # AI(W906-FW-C1WIRE) 20260911: recipe read/write, wb_serve ONLY.
    # The sidecar cannot carry this: TcpTagLink.h:58 states there is no inbound
    # command path at all, and wb_gateway keeps readOnly on purpose
    # (wb_gateway.cpp:104). So a browser-initiated write can only be executed by
    # the single process that holds both the server and the machine data. Adding
    # it to wb_publish would compile and link and never be called.
    WebBridgeRecipeDoc.cpp
    # AI(W906-ST-W0) 20260915: 瀏覽器 START 的 C++ 落點（TfMainWeb）。
    # 同一個理由的第四個檔：只有同時握有伺服器與機台資料的那個行程執行得了它。
    # 放在這裡而不是 ht9045_webbridge，因為那個 library 只 link ht9045_public
    # （上面 :2979），刻意不碰 god-stack —— 那是「零機台碼」的承重牆。
    # ST-W0 現況：類別存在、編得過、連得上，**沒有任何人建構或呼叫它**。
    # 它刻意不 override TfMain::Start()：全樹有約 19 個 fMain->Start() 呼叫點
    # （SECS/GEM 遠端 START、clean-out 自動重啟、InArm 取料錯誤重啟…），
    # override 一次就把它們全部武裝。理由詳見 WebStart.h 檔頭。
    WebStart.cpp
    # AI(W906-P3-OLP) 20260919: OLP 設定指令的白名單橋接。
    #   同上面四個檔的理由：只有同時握有伺服器與機台資料的行程執行得了它。
    #   它刻意「不」走 TfAutomation::ProcessBuffer() —— 那條路的
    #   共用回覆函式 SendClient() 在 handle 對不上時會強制 iConnect=0
    #   （automation.cpp:1034-1039），把瀏覽器的回覆推進真實工廠主機的 OLP 連線。
    #   詳見 WebOlp.h 檔頭。
    WebOlp.cpp
    # AI(W906-P6-WINREG) 20260920: 視窗狀態總表（取代 golden 的 fShow）。
    #   同上面四個檔的理由：只有握有伺服器的行程收得下瀏覽器推上來的總表。
    WebWindowRegistry.cpp  WebMotorAccess.cpp  WebMotorAccessLive.cpp   # AI(W906-W4-MOTOR) 20260925: motor.access（馬達按鈕 → golden 互鎖 → EastSun 1203 命令）；同上面四個檔的理由，只有 wb_serve 執行得了
    # AI(W906-P4-SECS) 20260920: SECS 的 SV 登錄表當 tag 目錄。
    #   借一張現成的「這台機台能報什麼」清冊，不必完成整個 SECS。
    SecsTagPublish.cpp
    SECSGEM/SecsSvRead.cpp
    # AI(W906-JSONBRIDGE-S0) 20260923: 開機配置廣播。
    #   規格 .claude/skills/ht9045-json-bridge/SKILL.md 4.6。
    #   gen/ 底下那個檔是 tools/gen_machine_defines.py 產生的，不要手改 ——
    #   它把 MachineType.h 的每個 #define 包成一組 #ifdef/#else，讓**編譯器**
    #   決定 defined 與值。這是唯一正確的作法：MachineType.h:63-64 的
    #   SOFT_SIMULTE 相依於 CMake 傳進來的 W906_NO_SOFT_SIMULTE，任何照檔案
    #   文字判斷的作法都會在出貨組態上判錯。
    JsonBridge/MachineDefines.cpp
    JsonBridge/ChanConfig.cpp
    JsonBridge/gen/sjson_MachineDefines.gen.cpp
    # AI(W906-JSONBRIDGE-S1) 20260923: 留痕 ring ＋ log.event。
    #   它呼叫 golden 的 RecordProcess / NewRecordProcess /
    #   RecordChangeLogProcess（cMyDB.h），那三個符號在 wb_serve 已經連得到
    #   （nm 實測），所以不需要多加任何 lib。
    JsonBridge/EventLog.cpp
    JsonBridge/actions/MainTesterConnect.cpp   # AI(W906-GB-P2e) 20260926: act.main.testerConnect（golden imgTesterClick main.cpp:29732-29794）
    JsonBridge/actions/MainRecordClear.cpp   # AI(W906-S119) 20260927: act.main.clearRecord／meShuttle2Dbl（golden spbClearRecordClick／meShuttle2DblClick）
    # AI(W906-JSONBRIDGE-S2/S3) 20260923: 結構欄位表（產生）＋ 實例綁定（手寫）
    #   ＋ 依表讀成 JSON。gen/ 底下的檔由 tools/gen_sjson.py 產生，不要手改。
    JsonBridge/StructJson.cpp
    JsonBridge/Bindings.cpp
    JsonBridge/StructApply.cpp   # AI(W906-JSONBRIDGE-S6) 20260923: JSON -> 結構（dryRun）
    JsonBridge/gen/sjson_SYSTEM_DEVICE_FORM.gen.cpp
    JsonBridge/gen/sjson_LAST_LEVEL_SET.gen.cpp
    JsonBridge/gen/sjson_SYSTEM_TEST_IF.gen.cpp
    JsonBridge/gen/sjson_SYSTEM_TEMPERATURE.gen.cpp
    JsonBridge/gen/sjson_SYSTEM_TRAY_FORM.gen.cpp
    JsonBridge/gen/sjson_SYSTEM_TEST_MODE.gen.cpp
    # AI(W906-JSONBRIDGE-S7) 20260923: 溫控通道 producer。
    # thermo_channels.gen.cpp 由 tools/gen_tempchan.py 產生，裡面有一條
    # static_assert 把通道數釘在 MachineType.h 的 tcTotalCount 上 ——
    # MachineType.h 改了而沒重產，會在編譯期爆而不是在畫面上少一個通道。
    JsonBridge/StageThermo.cpp
    JsonBridge/gen/thermo_channels.gen.cpp
    # AI(W906-SJSON-S8S9S10S11) 20260923: 四條新通道。
    #   ⚠ 這些檔跟 S0～S7 一樣**直接進 wb_serve 的來源清單**，不是一個
    #   `ht9045_jsonbridge` library。SKILL.md §4.9 寫的是後者，但 S1～S7 實際
    #   做成前者，而且那是對的：這些 TU 讀機台全域（LastSet / Prod / IniConfig /
    #   fMain / fCounterClear），做成 library 就得宣告一整排向上的相依，
    #   跟 WebBridgeTags.cpp「把混合限制在一個 TU」的理由相衝。沿用既有作法。
    JsonBridge/ChanProduction.cpp          # S8  prod.*（含差量與靜止量測）
    JsonBridge/ChanIo.cpp JsonBridge/ChanIoPoints.cpp  # S9  io.di / io.do 位元打包；AI(W906-IO-POINTS) 20260924: ChanIoPoints = 每個 Alias 的 io/config、io/runtime（接在同一行，不移動其後行號）
    JsonBridge/ChanMotor.cpp JsonBridge/ChanMotorPoints.cpp  # S9  motor.axes 定序陣列；AI(W906-MOTOR-POINTS) 20260924: ChanMotorPoints = /api/struct/motor/config|runtime（每軸，來源 HSys.MotTable＋MOT[]）
    JsonBridge/ChanAlarm.cpp               # S10 alarm.* 事件環（非阻塞入隊）
    JsonBridge/ChanAction.cpp              # S11 act.* 分派 ＋ counter.clear 收編
    # ⚠ 本體與分派分檔：分派只做守衛與 JSON，本體是 golden 的逐字翻譯。
    #   它經 forms/fMain.h 的 W906_ClarnDataBody 指標掛進 TfMain::Clarn_Data
    #   —— 不能直接寫在 forms/fMain.cpp，那會讓 ht9045_forms 長出
    #   forms->sm / forms->db 的 archive 邊（見該檔頭的長註解）。
    JsonBridge/actions/MainClarnData.cpp   # S11 golden V912 main.cpp:15458-15648
    # Steven 20260924 (S12)：/api/form 的外殼＋FormLock。AI(W906-Q4-S126) 20260927：第一型（gen/form_*.gen.cpp，tools/gen_formjson.py）退役，只剩第二型
    JsonBridge/FormJson.cpp
    # Steven 20260924 (S12 第二型)：golden 原檔產生的表單 bridge（tools/gen_formbridge.py）
    JsonBridge/FormBridge.cpp
    ${W906_FILERW_SRC}   # FileRW：兩份產生的清單（見 add_executable 上方）
    FileRW/Teach.cpp   # AI(W906-W5-TEACH) 20260925：golden TfTeach（IniConfig 樣式的手寫入口，不在 _editlist_sources.cmake）
    JsonBridge/actions/MainStateRecord.cpp)   # AI(W906-STATEREC) 20260924: act.main.stateRecord 的分派（golden sbStateRecordClick main.cpp:26209-26212）；接在原本的收尾括號同一行

target_link_libraries(wb_serve PRIVATE ht9045_testercomm_handler ht9045_testercomm ht9045_ela   # AI(W906-GB-P3) 20260926: H7 -- Tester comm (Handler side + engines); ahead of the RESCAN group, the machine libraries never reference them back (fMain.h W906_TesterForward)
    "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
    ht9045_webbridge
)

if(WIN32)
    target_link_libraries(wb_serve PRIVATE ws2_32)
endif()

# ---------------------------------------------------------------------------
# AI(W906-Q34-ARM) 20260923: 找得到 Advantech SDK 的機器，wb_serve 帶 HAVE_PCI1203=1 並連 ADVMOT。
#
#   使用者 20260923 裁決（Q34 第 1 條）：「找得到 SDK 才對 wb_serve 開」。
#   ⇒ **推翻** docs/PCI1203_20260918_INTEGRATION_PLAN.md §12 第 4 列「只武裝探針」——
#     但**只推翻 wb_serve 這一個 target**。同事包的 foreach(ht9045_io ht9045_motor ht9045_sm)
#     仍然不取（上面 BU-C-P5 的理由照舊成立）：IOBackend 的 TPci1203Backend 不直接呼叫 SDK。⚠ AI(W906-IOWEB-P17) 20260925：
#     但 WB_ENGINE_IO_1203（MachineType.h 檔尾）開著時，wb_serve 會把它接到 EtherCAT/Pci1203IoRoute.cpp ⇒ **機台狀態機會經卡片驅動 IO**
#     （讀＝Pci1203Monitor.cpp 的取樣，寫＝Pci1203Control.cpp 的 Execute；網頁命令面也是同一個 Execute）。
#
#   ⚠⚠ 它實際的意思：在**有 SDK 且有卡**的機器上，網頁上一次點擊可以讓線圈通電、讓伺服轉動
#     （命令面 WB_PUMP_1203_CONTROL＋_LIVE 已在 MachineType.h 開著），而且開機時會對 ring 0
#     呼叫 Acm_MasStartRing（WB_PUMP_1203_START_RING，4cd981b）。20260923 nm 實證：
#     帶 HAVE_PCI1203=1 編出的 Pci1203Monitor.cpp.obj 引用 _Acm_MasStartRing@8，共 27 個 Acm_ 符號；
#     不帶旗標的是 0 個。
#
#   ⚠⚠ 「找得到 SDK」≠「卡在位」：筆電裝了 Advantech SDK 也會命中這裡，wb_serve 就被武裝。
#     沒有卡時 Acm_DevOpen 會失敗；若回 0x83000002（從站未就緒），COLD-1（Q34-8）會在開機時
#     每秒重試、**最多 90 秒**，那段時間 tick 迴圈不動（使用者裁決第 3 條已知並接受）。
#     本 session 沒有「有 SDK、沒卡」的環境，這個延遲**未實測**。
#
#   ⚠ 位元數：照同事包（PKG CMakeLists.txt:2592-2600）依指標大小找，x64 走 Public/X64。
#     用**新變數** ADVMOT_APP_LIB，不蓋掉上面 pci1203_linkprobe 用的 ADVMOT_X86_LIB
#     （那個變數沒有 PATH_SUFFIXES，x64 configure 會快取到 i386 的 lib —— 已知缺陷，
#     docs/HT9050_PCI1203_DIRECTION.md §5.2 記載，另案處理）。
#
#   這台筆電（JIMMYCHIU-NB，20260923 實測）沒有 C:/Program Files (x86)/Advantech ⇒ 走 else，
#   wb_serve 與改動前完全相同（Pci1203ControlEnable() 誠實回 "not linked"）。
#
#   要反轉：把下面整個 if 換成 else 那一支的 message 即可；**同時**注意 MachineType.h 的
#   WB_PUMP_1203_START_RING／_CONTROL_LIVE／WB_ENGINE_IO_1203（檔尾，IOWEB-P17）與 tools/pci1203_control_gate.ps1 的 $expectActive
#   是另外幾道開關，反轉武裝要四處一起看（CLAUDE.md「要反轉」那段；AI(W906-IOWEB-P17) 20260925 加上引擎 IO 路由那一道）。
# ---------------------------------------------------------------------------
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(ADVMOT_APP_LIB NAMES ADVMOT
        PATHS "C:/Program Files (x86)/Advantech/Common Motion/Public/X64"
        NO_DEFAULT_PATH)
else()
    find_library(ADVMOT_APP_LIB NAMES ADVMOT
        PATHS "C:/Program Files (x86)/Advantech/Common Motion/Public"
        NO_DEFAULT_PATH)
endif()
if(ADVMOT_APP_LIB)
    target_compile_definitions(wb_serve PRIVATE HAVE_PCI1203=1)
    target_include_directories(wb_serve PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/EtherCAT/vendor)
    target_link_libraries(wb_serve PRIVATE ${ADVMOT_APP_LIB})
    message(STATUS "PCIE-1203 ARMED on wb_serve: ${ADVMOT_APP_LIB}")
else()
    message(STATUS "PCIE-1203 SDK not found -- wb_serve NOT armed (not linked)")
endif()

# ---------------------------------------------------------------------------
# AI(W906-T4-RETIRE) 20260919: wb_publish / wb_gateway 的 target 區塊在這裡，
#   已依使用者 20260918 裁決「退役」移除（計畫書 §11.4 的 R2）。
#
#   那兩支是 wb_serve 的 sidecar 分家版：wb_publish = 資料層 + TcpTagPublisher，
#   wb_gateway = TcpTagClient + WebBridgeServer，中間走 loopback TCP tag feed。
#   20260918 實測的兩個理由：
#     * wb_publish 的 link 清單是 wb_serve 的**真子集** —— 它能做的事 wb_serve
#       都能做，而且 wb_serve 還多了 HTTP 與命令面。
#     * wb_gateway 宣稱的「可以 front V899」**零實作**：WebSnapPort 只存在於
#       docs/parked/ 的 .txt，V899 樹 grep 回 0，而 V899 自 20260909 起唯讀。
#
#   ⚠ 退役有一個前置，而且先做了：R1 把金線 pump（PumpInit/PumpTick）搬進
#   wb_serve（20260918）。順序反了就是先把能力拆掉再想辦法補。
#   史料與完整理由在 docs/PCI1203_20260918_INTEGRATION_PLAN.md §11。
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
# =============================================================================
# AI(W906-IOWEB-P12) 20260924: READ-ONLY IO-web verification probe (tools/ioweb_probe.cpp).
#   Not part of the runtime; EXCLUDE_FROM_ALL so no default build changes. It links
#   the read-only monitor (EtherCAT/Pci1203Monitor.cpp) and NOT the write surface
#   (Pci1203Control.cpp), plus the same JSON producers the IO / motor pages use.
#   HAVE_PCI1203 only when the vendor import library was found (same rule as wb_serve).
#   Appended at EOF so no line of this file moves.
# =============================================================================
if(WIN32 AND ADVMOT_APP_LIB)
    add_executable(ioweb_probe EXCLUDE_FROM_ALL
        tools/ioweb_probe.cpp
        EtherCAT/Pci1203Monitor.cpp
        JsonBridge/ChanIoPoints.cpp
        JsonBridge/ChanMotorPoints.cpp
        JsonBridge/ChanMotor.cpp)
    target_include_directories(ioweb_probe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/EtherCAT/vendor)
    target_compile_definitions(ioweb_probe PRIVATE HAVE_PCI1203=1)
    target_link_libraries(ioweb_probe PRIVATE
        "$<LINK_GROUP:RESCAN,ht9045_sm,ht9045_secsgem,ht9045_forms,ht9045_automation,ht9045_comms,ht9045_io,ht9045_motor,ht9045_db,ht9045_core,ht9045_globals,ht9045_public,vclcompat>"
        ht9045_webbridge ${ADVMOT_APP_LIB} ws2_32)
    message(STATUS "ioweb_probe: read-only 1203 probe available (EXCLUDE_FROM_ALL)")
endif()