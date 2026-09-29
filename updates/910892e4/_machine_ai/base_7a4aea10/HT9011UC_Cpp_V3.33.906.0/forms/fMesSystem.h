// =============================================================================
//  forms/fMesSystem.h  --  non-VCL facade for golden's TfMesSystem
//  (Mes/fVATMesFileSys.h / fVATMesFileSys.cpp) -- the VTEST MES integration
//  screen: downloads lot-open info from an MES HTTP server, cross-checks it
//  against the machine's own Setup/Security config, and periodically ships
//  production/OEE/jam-rate summaries back out (HTTP/SOAP/FTP/file).
//
//  AI(W906-FW3-MES1) 20260827: new file, FW wave FW3-MES1 (facade wave 1 of
//  fVATMesFileSys -- read-only direction, no write path). Golden source of
//  truth: HT9011UC_Code_V3.33.906.0_20260618/Mes/fVATMesFileSys.h (229 lines)
//  + Mes/fVATMesFileSys.cpp (3,455 lines). Decoded this wave with
//  `io.open(p, encoding='cp950')`, written out UTF-8, 0 U+FFFD.
//
//  ============================================================================
//  *** SAFETY / PRIVACY POSTURE -- READ BEFORE TOUCHING ANY GATE HERE ***
//  ============================================================================
//  TfMesSystem's entire reason to exist is to talk to an EXTERNAL MES server
//  (TIdHTTP GET/POST, a SOAP web-service client `fWebService->...`) and to
//  persist what it learns/produces to disk (WriteIniData, TStringList::
//  SaveToFile, raw fopen(...,"w"), TIniFile::WriteInteger, file DeleteFile).
//  Some of what it writes also feeds real production judgement downstream
//  (bin-routing cross-checks, MES lot-open validation gating whether the
//  Handler is allowed to go ON_LINE). Per this wave's task brief, ALL such
//  functions are GATED: declared here (so a future wave's un-gate binds to
//  the exact name+type this wave commits to) but NOT DEFINED in
//  forms/fMesSystem.cpp. Calling a gated member is a LINK ERROR by design
//  (`undefined reference to TfMesSystem::<method>`), same idiom as
//  forms/fIoSetView.h / forms/fContact.h / forms/fCleaning.h.
//
//  Of golden's 46 bodies (42 `TfMesSystem::` + 4 file-scope; census.py
//  `functions()` re-run 20260827, 0 short-fall / 0 overload collapse -- see
//  DENOMINATOR below), this wave delivers 17 and gates 29.
//
//  ============================================================================
//  *** THE GLOBALS `TfMesSystem` / `fMesSystem` ARE FREE -- THIS WAVE CLAIMS
//  THEM (unlike forms/fIoSetView.h's `fiosetview`, which stays undeclared) ***
//  ============================================================================
//  Re-verified 20260827 (comment-and-string-stripped scan, all .h/.hpp/.cpp
//  under this tree excluding build*/.git/.svn/.pti_frames/web):
//    `class TfMesSystem` (outside this file)      -- 0 hits.
//    `TfMesSystem *fMesSystem` (outside this file) -- 0 hits.
//    forms/fLotInfo.h:382-390 (WB-14, CheckNoRetestBinFlag) states outright:
//      "`fMesSystem` 有 NO PORT AT ALL in this tree ... among ~40 objects
//      that have no port at all" and gates its own two
//      `fMesSystem->NeedNoRTBinID(i)` call sites to a hardcoded conservative
//      `true` for exactly that reason. This wave's `fMesSystem` supplies the
//      real symbol that call site (and the other 10 below) is waiting for --
//      un-gating THAT call site is forms/fLotInfo.cpp's own follow-up work,
//      NOT done here (write boundary: this wave may only touch
//      forms/fMesSystem.{h,cpp}).
//  So: `.h` declares `extern TfMesSystem *fMesSystem;`, `.cpp` defines
//  `TfMesSystem *fMesSystem = new TfMesSystem();` -- same idiom as
//  forms/fCleaning.cpp:45 (`TfCleaning *fCleaning = new TfCleaning();`) and
//  forms/fLotInfo.cpp:4417.
//
//  ============================================================================
//  ELEVEN `#if 0`-GATED CALL SITES ELSEWHERE IN THE TREE -- NAMES ARE
//  PRE-COMMITTED, NOT THIS WAVE'S CHOICE
//  ============================================================================
//  `python tools/census/wave_preflight.py Mes/fVATMesFileSys.cpp` (20260827)
//  found 11 `fMesSystem->member` sites already sitting inside `#if 0` blocks,
//  waiting for a real facade to bind to:
//    AutoSiteMapPass(int)              atester.cpp:3271               ACTIVE
//    CleaOEEState12hList()             cinitial.cpp:9728              ACTIVE
//    DoInitailRecordReportByTime()     ainarm9045.cpp:5235,:5240      ACTIVE
//    DoRecordReportByTime(bool=false)  csystem.cpp:10154,:10553       GATE (L-1)
//    GetRcsCheckingResult(bool)        atester.cpp:1361,
//                                      aTester_Front.cpp:7661,
//                                      aTester_Rear.cpp:7688           GATE (S-1)
//    LabeledEditLotNo (TfLotInfoEdit*) cprod.cpp:1267                 field
//    NeedNoRTBinID(int)                forms/fLotInfo.cpp:2696,:2708  ACTIVE
//    asGetRcsCheckingResult (AnsiString) aTester_Front.cpp:7659,:8799,
//                                      aTester_Rear.cpp:7686...        field
//    bFormShowJustInitial (bool)       ainarm9045.cpp:5231,:5234,:5239 field
//    lbledtC1 (TfLotInfoEdit*)         cprod.cpp:1268                 field
//    lbledtCustLotNum (TfLotInfoEdit*) cprod.cpp:1266                 field
//  Every one of the 11 names+types below matches this list exactly (spot-
//  checked call-site argument shapes 20260827: `fMesSystem->AutoSiteMapPass
//  (iTesterBIN[i][j])` int arg, `fMesSystem->NeedNoRTBinID(i)` int arg,
//  `fMesSystem->CleaOEEState12hList()` / `->DoInitailRecordReportByTime()`
//  no args -- all agree with the signatures below).
//
//  ⚠ IMPORTANT: these 11 sites are STILL `#if 0` after this wave. Declaring
//  (and for the ACTIVE ones, DEFINING) the matching members does NOT un-gate
//  them -- that is a separate, deliberate edit to each of those OTHER files,
//  out of this wave's write boundary. See DISCLOSURE (trap #1) at the bottom.
//
//  ============================================================================
//  DENOMINATOR -- MEASURED THIS WAVE, NOT INHERITED
//  ============================================================================
//  `python tools/census/wave_preflight.py Mes/fVATMesFileSys.cpp` 20260827:
//  46 bodies = 42 `TfMesSystem::` + 4 file-scope (ValidNoRTBinInfo,
//  ShowInputDialog, MyGetTickCount64, mes_cmp_by_count), 3,349 golden span
//  lines, 0 brace-swallow short-fall, 0 overload collapse.
//  THIS WAVE: 17 of 46 bodies delivered (37.0%) -- 16 full translations + 1
//  ctor PORT SPLIT (fields + own-widget `new` + the one safe trailing call
//  golden's own ctor makes, `DoInitialLotInfor()`; the ctor's OTHER lines --
//  `LoadStringlRecordReport()` [file read+delete], `stringlRecordReport`
//  alloc, `dtOEETimeStart=Now()`, and several fields whose only consumer is
//  a GATED method -- are NOT translated, see DEVIATION D-2).
//  BY LINES: the 17 delivered bodies' golden spans total 337 of 3,349 lines
//  (10.1%) -- buttonDownloadLotInforClick alone (652L, GATE) is ~2x this
//  whole wave; CheckLotInfor (602L, GATE) and RecordByTimeOEE (505L, GATE)
//  together are another ~3.3x. Quote both numbers or neither.
//
//  ============================================================================
//  BATCH CRITERION FOR THIS WAVE (why these 17 and not others)
//  ============================================================================
//  A method is ACTIVE here only if ALL of the following hold, each checked by
//  reading the FULL golden body (never by name -- the task brief specifically
//  flagged DoRecordReportByTime/DoInitailRecordReportByTime as
//  name-suspicious ("聽起來就會寫報表檔"); both were read in full before any
//  decision):
//    1. It writes nothing to disk -- no WriteIniData, no TStringList::
//       SaveToFile/LoadFromFile, no raw fopen(...,"w"), no TIniFile::Write*,
//       no DeleteFile, no MyForceDirectories/b_Check_Dir_Exist_And_Creak_Dir
//       (that name means "check AND CREATE").
//    2. It sends/receives nothing outbound -- no TIdHTTP::Get/Post, no SOAP
//       (`fWebService->...`), no FTP (`fFTPClient->...`).
//    3. It issues no machine action / mode switch -- no `fMain->ModifyTester`,
//       no `SetRunStartMode`, no MOT[]/Cylinder[]/SW[] touch (none found
//       anywhere in this golden file, unlike forms/fIoSetView.h's iosetview).
//    4. Every symbol it needs already exists in the port tree AND is
//       reachable from ht9045_forms's actual link set (vclcompat +
//       ht9045_globals + ht9045_core only -- ht9045_forms does NOT link
//       ht9045_sm, ht9045_motor, ht9045_io, or ht9045_db). Traced this wave
//       (20260827) to each symbol's DEFINING .cpp and CMakeLists.txt
//       add_library block:
//         TestIF_File / BinSelect[8] / TrayForm  -> cprod.cpp:31/:34/:14,
//             `add_library(ht9045_globals ...)` (CMakeLists.txt:472-475).
//             REACHABLE -- used by AutoSiteMapPass / NeedNoRTBinID.
//         bNoRTBinFixFlag[3] / iTestRunMode / iFixRightHalf
//                                          -> cmydef.cpp:6150/:3339,
//             cmydef.cpp:3038, same ht9045_globals archive. REACHABLE.
//         LastSet (LAST_GENERAL_SET, ~390 fields) -> LastSet.cpp:39, also
//             inside `add_library(ht9045_globals ...)` (CMakeLists.txt:475,
//             moved there by GA1-B2-integrate specifically because cprod.cpp
//             in the SAME archive references it directly). REACHABLE --
//             used by DoInitailRecordReportByTime (iBinCTForAlways[4][20] /
//             iSiteTotalCTForAlways[4][8] / iSiteBinCTForAlways[4][8][256],
//             shapes cross-checked against LastSet.h:521-523, all three
//             match golden's own loop bounds exactly).
//         eAuto1 / MAX_SOCKET_ROW / MAX_SOCKET_COL / TEST_MAX_BIN / eTrayCount
//                                          -> MachineType.h (enum / #define /
//             `const int`), header-only, no link needed.
//         RecordProcess(AnsiString,AnsiString="") -> REAL body canary_support.cpp
//             (ht9045_sm), declared via TWO headers tree-wide
//             (canary_support.h AND cMyDB.h:130). *** THIS FILE INCLUDES
//             cMyDB.h FOR IT, NOT canary_support.h *** -- see LAST_GENERAL_SET
//             ODR TRAP below for why that choice is load-bearing, not
//             cosmetic. Reachability itself is the SAME documented exception
//             this task's own brief names: forms/fLotInfo.cpp (same
//             ht9045_forms archive) already calls RecordProcess 4x and the
//             tree is green (both archives link into the final exe).
//         cJSON_Parse/cJSON_GetObjectItem/cJSON_GetArraySize/cJSON_GetArrayItem
//                                          -> Public/cJSON.c,
//             `add_library(ht9045_public ...)` (CMakeLists.txt:377).
//             ht9045_forms links ht9045_core (CMakeLists.txt:769-776) which
//             itself `target_link_libraries(ht9045_core PUBLIC ht9045_public)`
//             (CMakeLists.txt:419) -- REACHABLE, a clean PUBLIC transitive
//             edge, not an exception.
//    5. Every symbol's definition line is NOT inside `#if 0` (checked with
//       census.py's gate_depth_map over cprod.cpp/cmydef.cpp/LastSet.cpp at
//       the exact definition lines above -- all live, none gated).
//
//  TRANSITIVE RULE, applied strictly: a method that CALLS a gated method is
//  itself gated. This is what removes btnSendJamCodeClick (-> SendJamCode,
//  GATE), btnQueryEocapStatusClick (-> QueryEocapStatus, GATE),
//  btnGetRcsCheckingResultClick (-> GetRcsCheckingResult, GATE),
//  WriteOEEState (-> AddOEEState, GATE) and SetOEEState (-> WriteOEEState,
//  transitively GATE).
//
//  ============================================================================
//  ⚠ LAST_GENERAL_SET ODR TRAP -- WHY THIS FILE INCLUDES cMyDB.h, NOT
//  canary_support.h, FOR RecordProcess
//  ============================================================================
//  LastSet.h's own banner (RELATIONSHIP TO canary_support.h's MINIMAL SHIM,
//  :19-29) states: canary_support.h carries ITS OWN, DIFFERENT
//  `struct LAST_GENERAL_SET` (a hand-picked 66-field subset) so 123 existing
//  TUs can include it without pulling in the full ~390-field struct.
//  "This header must NEVER be #included from canary_support.h or vice
//  versa" -- an ODR collision (two same-named, differently-shaped structs)
//  awaits any TU that includes BOTH. This file needs the REAL, FULL LastSet
//  (LastSet.h, for DoInitailRecordReportByTime's iBinCTForAlways/etc.) AND
//  RecordProcess (canary_support.cpp's real body, for CheckEocapStatus).
//  Resolution: RecordProcess has a SECOND, independent forward declaration in
//  cMyDB.h:130 ("see HOMECOMING NOTICE above -- real body lives in
//  canary_support.cpp") which does NOT itself declare LAST_GENERAL_SET and
//  does NOT include canary_support.h (verified 20260827: cMyDB.h's own
//  #include list is vclcompat/vcl_compat.h + vclcompat/StringGrid.h +
//  vclcompat/Controls.h + myTimer.h only; myTimer.h's is vclcompat/
//  vcl_compat.h + <windows.h> only -- neither pulls canary_support.h or
//  LAST_GENERAL_SET). forms/fMesSystem.cpp therefore `#include "cMyDB.h"`
//  (NOT "canary_support.h") alongside `#include "LastSet.h"` -- same real
//  linker symbol `RecordProcess(AnsiString,AnsiString)`, reached through the
//  collision-free header.
//
//  ============================================================================
//  GATED -- WRITES DISK (18)
//  ============================================================================
//   (W-01) buttonSaveClick        golden :166-210  10x WriteIniData (Server
//       section: URL/ACode/Action/Summary/ByTimeOEE/UPH report paths +
//       CreateManualEOCAP/QueryEocapStatus/GetRcsCheckingResult URLs).
//   (W-02) buttonDownloadLotInforClick golden :320-971 (652L) writes
//       listbMesFileLog->SaveToFile (MesFileLog_*.txt), WriteIniData
//       (Tray.Data "NoRTBin"), PLUS transmits (IdHTTPMESSystem->Get) PLUS
//       machine actions (fMain->ModifyTester(ON_LINE), SetRunStartMode,
//       fFTPClient->ShowFTPModal) -- excluded on every ground independently.
//   (W-03) DoEnableLotInforForMesFile golden :973-1035 -- see MISSING SYMBOL
//       below (NOT a write-gate; listed here only because it transitively
//       blocks FormShow, a write-adjacent method).
//   (W-04) VTestSummaryReport     golden :1640-1852 (213L) -- SAVEFILE +
//       SECS-adjacent hits confirmed by pattern scan; NOT read past the
//       confirming evidence, see DISCLOSURE.
//   (W-05) DoRecordReportByTime   golden :1902-2081 (180L) -- read IN FULL
//       (name-suspicious per task brief). Does NOT itself write a file (the
//       actual SaveToFile lives in the separate DoSaveReportByTime, W-06) --
//       it only appends strings to `stringlRecordReport` and reads globals.
//       GATED INSTEAD on LINK BOUNDARY (L-1 below): `TestSocket.iShtRow` /
//       `.iShtCol` (:1972,:1974,:2026,:2028) is aHotPlateSubstrate.cpp,
//       ht9045_sm, unreachable from ht9045_forms.
//   (W-06) DoSaveReportByTime     golden :2087-2107  MyForceDirectories +
//       stringlRecordReport->SaveToFile(*.sum) + DeleteStringlRecordReport.
//   (W-07) SaveStringlRecordReport golden :2112-2133 SaveToFile
//       (stringlRecordReport.sum, stOEEState12hList.csv).
//   (W-08) ~TfMesSystem (dtor)    golden :2135-2162 calls SaveStringlRecordReport
//       (W-07) as its first line -- transitively a write. A port-only dtor
//       that frees this wave's OWNED pointers (widgets/TStringLists) is
//       provided instead, NOT a golden translation -- see DEVIATION D-3.
//   (W-09) RunModeRW              golden :2165-2184 TIniFile::WriteInteger
//       (bRead==false arm) + b_Check_Dir_Exist_And_Creak_Dir; the bRead==true
//       arm also opens the same TIniFile, so the whole function is gated
//       rather than splitting a read-only shadow (task rule: "隱性寫檔陷阱
//       ...逐段確認用哪一支" -- here BOTH arms share one `new TIniFile(...)`
//       construction, not separable without inventing behaviour).
//   (W-10) LoadStringlRecordReport golden :2186-2199 DeleteFile (destructive)
//       after LoadFromFile.
//   (W-11) DeleteStringlRecordReport golden :2201-2212 DeleteFile.
//   (W-12) RecordByTimeOEE        golden :2248-2752 (505L) -- b_Check_Dir_
//       Exist_And_Creak_Dir(IniConfig.asByTimeOEEPath) confirmed at :2270
//       (create-dir); IniConfig.bN10_UploadSummaryToFTP guard at :2267
//       strongly implies an eventual FTP upload later in the body. Read only
//       the first ~120 of 505 lines (through the event-log regex/date-math
//       section) -- NOT fully read, see DISCLOSURE.
//   (W-13) VTestUPHReport         golden :2754-2794 slReport->SaveToFile.
//   (W-14) SendJamCode            golden :2796-2905 lstCreateManualEOCAP->
//       Items->SaveToFile (CreateManualEOCAP.txt) PLUS SOAP transmit
//       (fWebService->SIPServiceSoap_VTEST->CreateManualEOCAP).
//   (W-15) QueryEocapStatus       golden :2938-3055 lstQueryEocapStatus->
//       Items->SaveToFile (QueryEocapStatus.txt) PLUS HTTP transmit
//       (IdHTTPqueryEocapStatus->Get).
//   (W-16) Get2DIDFromServer      golden :3153-3176 MyForceDirectories +
//       uRDN.WriteFile (raw fopen "w") PLUS HTTP transmit (GetServerWebAPI).
//   (W-17) AddOEEState            golden :3262-3352 b_Check_Dir_Exist_And_
//       Creak_Dir(IniConfig.asByTimeOEEPath) (create-dir) PLUS
//       `TestSocket.iShtRow`/`.iShtCol` (:3292-:3293) -- ht9045_sm, ALSO a
//       LINK BOUNDARY gate independently.
//   (W-18) uReadDeviceNumer::WriteFile golden :120-125 raw
//       fopen(asLocationFile.c_str(),"w") -- unconditional write of caller-
//       supplied content to a caller-supplied path.
//
//  ============================================================================
//  GATED -- TRANSMITS OUTBOUND (5, all ALSO listed above under a write
//  reason except GetServerWebAPI/GetRcsCheckingResult which write nothing
//  themselves)
//  ============================================================================
//   (S-1) GetRcsCheckingResult    golden :3079-3146 (68L) IdHTTPMESSystem->
//       Get(asURL) (:3121) -- writes nothing itself (the SaveToFile pattern
//       hit on this file's earlier scan was QueryEocapStatus's, not this
//       one) but reads a live MES alarm-check endpoint and can call
//       SendJamCode (W-14) on FAIL -- excluded on transmission alone.
//   (S-2) GetServerWebAPI         golden :3199-3212 `new TIdHTTP(NULL)` +
//       ->Get(asURL) -- a fresh, local TIdHTTP instance, but still a real
//       network GET; "對外傳輸" excludes it regardless of which object
//       issues the call.
//
//  ============================================================================
//  GATED -- LINK BOUNDARY (ht9045_forms cannot reach ht9045_sm) (2)
//  ============================================================================
//   (L-1) DoRecordReportByTime    golden :1902-2081 `TestSocket.iShtRow`/
//       `.iShtCol` (:1972,:1974,:2026,:2028) -- TestSocket is `TMyKitSuck
//       TestSocket;` defined aHotPlateSubstrate.cpp:91, listed inside
//       `add_library(ht9045_sm ...)`. Same TestSocket/ht9045_sm shape as
//       forms/fIoSetView.h's own (L-01) precedent. Otherwise 100% pure
//       string-accumulation (no write, no transmit) -- would have been this
//       wave's SECOND-largest ACTIVE delivery (180L) had the boundary not
//       existed; recorded in enough detail for a future ht9045_sm-reaching
//       wave to un-gate directly.
//   (L-2) CheckLotInfor            golden :1037-1638 (602L) -- read IN FULL.
//       Otherwise ENTIRELY a pure validation/comparison function (compares
//       downloaded MES lot-open fields against the machine's own Setup/
//       Security config, `ShowMyMessage`+`return false` on every mismatch,
//       ZERO writes, ZERO transmission, ZERO machine action anywhere in the
//       602 lines). Gated SOLELY on `TestSocket.iShtRow`/`.iShtCol`
//       (:1576,:1582,:1584) -- same ht9045_sm boundary as L-1. Would have
//       been this wave's LARGEST ACTIVE delivery by a wide margin had this
//       one field not been ht9045_sm-only; a future ht9045_sm-reaching wave
//       should look here first. (Golden also declares an unused local
//       `sConfigPath=AuthPath+"Security_new.def";` at :1044 that is never
//       read again in the function -- a dead declaration, not a hidden
//       write; noted under GOLDEN NOTES.)
//
//  ============================================================================
//  GATED -- MISSING SYMBOL (2)
//  ============================================================================
//   (X-1) DoEnableLotInforForMesFile golden :973-1035 (63L) -- the ONLY
//       foreign-object touch in an otherwise all-own-widget `->Enabled=bFlag`
//       function is `fConfiguration->cbN05_CheckFile->Enabled=bFlag;`
//       (:982). `fConfiguration` IS a real, live global -- but it is ALREADY
//       BOUND to `W5SckArtRem_ConfigStub*` (Automation/SCK_ART_Remainder.h:
//       594-599, "Gate #14"), a minimal stand-in carrying EXACTLY ONE member
//       (`mmoN04_IP`). `cbN05_CheckFile` does not exist on that type
//       (verified 20260827: `grep -n cbN05_CheckFile Automation/
//       SCK_ART_Remainder.h` -- 0 hits). The `TfConfiguration` class that
//       DOES declare `cbN05_CheckFile` (forms/fConfiguration.h:1747) has NO
//       global pointer of its own -- the bare name `fConfiguration` is
//       already spoken for. Per task rule 4 ("缺符號比錯答案好...不要寫退化
//       版本"), this wave does not redeclare `fConfiguration` (hard
//       redefinition) nor silently drop the one line (a behaviour decision).
//       TRANSITIVELY GATES FormShow (golden :212-246, calls
//       DoEnableLotInforForMesFile unconditionally in both branches of its
//       `if(RunInfo.bLotStart)`) -- otherwise FormShow's own remaining
//       content (IniConfig field mirroring into widgets, AccessLevel check)
//       would have been safe on its own.
//   (X-2) ClearMesData             golden :3384-3444 (61L) -- every line but
//       the last is `own_widget->Text="";` (safe), but the FINAL line is
//       `fSortCT->btnClearCountClick(fSortCT);` (:3443). `fSortCT` IS real
//       and reachable (forms/fSortCT.h/.cpp, ht9045_forms) but
//       `btnClearCountClick` is NOT a member of the current `TfSortCT`
//       facade (verified 20260827: `grep -n btnClearCountClick forms/
//       fSortCT.h` -- 0 hits). Same rule 4 reasoning as X-1: not this
//       wave's write boundary to add a method to forms/fSortCT.h, and
//       dropping the line would silently change ClearMesData's behaviour.
//
//  ============================================================================
//  GATED -- MISSING TYPES (1)
//  ============================================================================
//   (T-1) ShowInputDialog (file-scope) golden :3446-3454 -- golden's OWN
//       comment marks it "(未完成)" (unfinished): constructs a bare `TForm
//       *dlg = new TForm(Application);`, sets 5 properties, never Shows it,
//       never frees it, never returns anything. Needs `TForm`, the global
//       `Application`, `poScreenCenter`, and `bsDialog` -- ALL FOUR absent
//       from vclcompat (verified 20260827: `grep -rn "class TForm\b"
//       vclcompat/*.h` / `\bApplication\b` / `poScreenCenter` / `bsDialog` --
//       0 hits each). Per rule 4, no facade-local TForm is invented here
//       (a real TForm type is a tree-wide design decision, not this file's
//       to make unilaterally) -- and even if it were, golden's own body is
//       inert (constructs and leaks an unused dialog), so there is no
//       behaviour this gate is withholding.
//
//  ============================================================================
//  GATED -- *Click / TRANSITIVE (4)
//  ============================================================================
//  btnSendJamCodeClick (golden :2907-2910, -> SendJamCode W-14)
//  btnQueryEocapStatusClick (golden :3057-3060, -> QueryEocapStatus W-15)
//  btnGetRcsCheckingResultClick (golden :3148-3151, -> GetRcsCheckingResult S-1)
//  WriteOEEState (golden :3214-3221, -> AddOEEState W-17) and its own caller
//    SetOEEState (golden :3223-3260, -> WriteOEEState, transitively W-17) --
//    5 named here (WriteOEEState + SetOEEState counted once each; the 3
//    *Click above are 3 more = arithmetic check below counts SetOEEState/
//    WriteOEEState in this bucket as 2, total bucket = 5).
//
//  Arithmetic check against the golden total: 17 ACTIVE (16 full + 1 ctor
//  port-split) + 18 write-gated + 2 transmit-only-gated (S-1/S-2, NOT
//  double-counted against the 18 write list -- GetRcsCheckingResult and
//  GetServerWebAPI write nothing themselves) + 2 link-boundary + 2 missing-
//  symbol + 1 missing-type + 5 *Click/transitive-only (btnSendJamCodeClick,
//  btnQueryEocapStatusClick, btnGetRcsCheckingResultClick, WriteOEEState,
//  SetOEEState) = 17+18+2+2+2+1+5 = 47. One over 46 because
//  DoEnableLotInforForMesFile (X-1) is counted BOTH as the reason FormShow
//  is gated (mentioned under X-1's own text) AND would otherwise double-
//  count itself -- it is its own single entry in the missing-symbol bucket
//  (X-1), and FormShow's write-adjacent bucket entry (W-03) is the SAME
//  function counted from FormShow's perspective, not a 47th body. Corrected
//  count: 46 golden bodies = 17 ACTIVE + 29 GATED (18 write + implied-in-W-03
//  DoEnableLotInforForMesFile itself is inside the 18, FormShow is inside
//  the 18 as W-03's target -- i.e. the 18-line "GATED -- WRITES DISK" list
//  above already contains BOTH DoEnableLotInforForMesFile's blocking note
//  AND is where FormShow's own W-03 entry lives) + 2 transmit + 2 link + 2
//  missing-symbol... this reconciliation is intentionally left showing its
//  own arithmetic seam rather than silently forced to balance: the
//  authoritative count is "17 ACTIVE, 29 NOT" (17+29=46), verified by
//  counting `TfMesSystem::` + file-scope definitions actually present in
//  forms/fMesSystem.cpp (16) + the ctor (1) = 17.
//
//  ============================================================================
//  GOLDEN NOTES (faithful; recorded, NOT "fixed")
//  ============================================================================
//  (G-1) golden ctor line :131 reads `bVATToChangeWorkFile-false;` -- a typo
//      (`-` where `=` was meant). This is a NO-OP expression statement (
//      `bool - bool` computed and discarded), so golden's own ctor NEVER
//      actually initialises `bVATToChangeWorkFile` at all. Not translated
//      either way here: the field is not declared in this wave's facade
//      (its only consumers -- buttonDownloadLotInforClick W-02, CheckLotInfor
//      L-2 -- are both gated), so the quirk is moot for THIS delivery, but
//      recorded so a future wave translating those consumers does not
//      "fix" the typo into a real initialisation without a deliberate call.
//  (G-2) CheckEocapStatus golden :2917-2918: `iStart=asString.Pos("eocapStatus");
//      asResult=asString.SubString(iStart+13,1);` -- if `Pos` returns 0 (not
//      found), `SubString(13,1)` still runs against whatever character
//      happens to sit at offset 13, with no guard. Translated verbatim.
//  (G-3) DoInitialLotInfor sets TWO fields to the literal string "Unknown"
//      (`lbledtInternalLot->Text="Unknown"` :273, `lbledtC1->Text="Unknown"`
//      :300) while every other of the ~50 fields it touches is reset to ""
//      -- an intentional asymmetry in golden, not a translation slip.
//      Preserved verbatim.
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention.
//  (D-2) CTOR is a PORT SPLIT, not a 1:1 translation (task rule: ctor fields
//      only + own-widget `new` only, no other global deref -- SIOF/
//      static-init trap). Golden's ctor body is kept for every line whose
//      target field this wave declares (see field list) PLUS the trailing
//      `bShow=false; DoInitialLotInfor();` (both safe: `DoInitialLotInfor`
//      touches only already-`new`'d own widgets, called AFTER they are all
//      allocated earlier in the same ctor body, exactly mirroring how real
//      VCL streams .dfm-owned components into existence before the user
//      ctor body runs). NOT translated: `bVATToChangeWorkFile-false;` (G-1,
//      moot -- field not declared), `LoadStringlRecordReport();` (file
//      read+DeleteFile, W-10), `stringlRecordReport=new TStringList;
//      ->Clear();` (field not declared -- only consumers are W-05/W-06/W-07/
//      W-10/W-11, all gated), `dtOEETimeStart=Now();` (field not declared --
//      only consumer is W-17 AddOEEState, gated), and the five scalar fields
//      whose only consumer is a gated method (`bVATToChangeWorkFile`,
//      `bCanShowMESReport`, `iRunSecondTemp`, `iUPHXCount`,
//      `bCreateManualEOCAP_OK`).
//  (D-3) `~TfMesSystem()` is a PORT-ONLY destructor (frees this wave's OWNED
//      raw pointers: the ~51 TfLotInfoEdit* widgets, `labEocapStatus`,
//      `listbMesFileLog`, the 4 TStringList* families, `stOEEState12hList`)
//      -- NOT a translation of golden's own dtor (golden's writes to disk
//      via SaveStringlRecordReport, W-08). Same idiom as forms/fCleaning.h
//      D-3 / forms/fIoSetView.h D-3. Copy/assign `= delete` for the same
//      double-free-safety reason.
//  (D-4) `TfMesSystem` has NO base class (matches forms/fCleaning.h /
//      forms/fIoSetView.h -- not `: public TForm`; no delivered method needs
//      a TForm base, and this file may not touch vclcompat to add one).
//  (D-5) `FormClose()` takes NO parameters -- golden signature is
//      `FormClose(TObject *Sender, TCloseAction &Action)`; `TCloseAction`
//      has no port anywhere in this tree, same already-established
//      precedent as ATC/ATCInterface.h:391 and forms/fIoSetView.h D-7 (not
//      re-litigated here).
//  (D-6) Scalar members with no explicit ctor initialiser in golden
//      (`bNoRTBinFlag[3]`) default via in-class `= {}` (all-false) -- BCB6/
//      VCL zero-fills a TForm-derived instance via GetMem before any field
//      initialiser runs, so this is the faithful default, not an invented
//      one (same reasoning as forms/fIoSetView.h D-6 / forms/fCleaning.h
//      precedent for un-initialised scalars).
//
//  ============================================================================
//  ZERO-WRITER FIELD
//  ============================================================================
//  `map2DIDFromServer` is populated ONLY by `Get2DIDFromServer` (W-16,
//  GATED). `IsMatchServerData` (ACTIVE, delivered) therefore always reads an
//  EMPTY map this wave and always returns `false`. This is consequence-free
//  for now: nothing in THIS delivery consumes `IsMatchServerData`'s return
//  value (no live caller exists inside this file, and no other TU calls it
//  either -- `fMesSystem->IsMatchServerData` is not among the 11 `#if 0`
//  sites). Flagged per task rule 7 so a future wave un-gating
//  Get2DIDFromServer knows this is the reason `IsMatchServerData` starts
//  "working" only once that other gate opens, not a bug introduced then.
//
//  ============================================================================
//  SYMBOLS THIS TU NEEDS (for the CMakeLists landing decision -- NOT made
//  here; this wave may not touch CMakeLists.txt)
//  ============================================================================
//    vclcompat      -- AnsiString / TStringList / TLabel / TListBox /
//                       TfLotInfoEdit (forms/FormWidgets.h -> vclcompat::TEdit)
//    ht9045_globals -- TestIF_File / BinSelect[8] / TrayForm (cprod.cpp),
//                       bNoRTBinFixFlag[3] / iTestRunMode / iFixRightHalf
//                       (cmydef.cpp), LastSet (LastSet.cpp)
//    ht9045_sm (via the RecordProcess exception ONLY, same precedent this
//                       task's own brief names) -- RecordProcess's real body
//                       (canary_support.cpp)
//    ht9045_public (via ht9045_core's PUBLIC link, a clean transitive edge,
//                       not an exception) -- cJSON.c
//  Expected landing: ht9045_forms (same layer as forms/fCleaning.cpp,
//  forms/fSortCT.cpp -- vclcompat + ht9045_globals + ht9045_core only).
//  Introduces NO new archive edge beyond what forms/fLotInfo.cpp already
//  proved safe (RecordProcess) and what ht9045_core already PUBLIC-links
//  (ht9045_public / cJSON).
//
//  ============================================================================
//  ⚠ ONE #define SEAM ELSEWHERE -- DOES NOT AFFECT THIS FILE, DOCUMENTED FOR
//  THE MAIN LOOP'S CMakeLists LANDING DECISION
//  ============================================================================
//  `asendic_Auto.cpp` carries `#define fMesSystem W7L1A_fMesSystem` (its own
//  TU-local redirect, feeding a `bFirstMaterialsQA` read at :2587/:2589 off
//  its own stand-in). That `#define` affects ONLY asendic_Auto.cpp's own
//  translation unit. This file (forms/fMesSystem.h) is NOT included by
//  asendic_Auto.cpp (this wave does not include this header from any
//  existing file -- write boundary), so the real `extern TfMesSystem
//  *fMesSystem;` declared below is never seen from inside that macro's
//  scope, and no name-mangling collision occurs. Recorded here only because
//  a FUTURE wave that DOES want to `#include "forms/fMesSystem.h"` from
//  asendic_Auto.cpp must do so BEFORE that `#define`, or the global name gets
//  silently rewritten to `W7L1A_fMesSystem` and fails to match this type.
//
//  ============================================================================
//  WHAT REMAINS / NOT DONE THIS WAVE (task rule 9 -- explicit so scope-
//  narrowing is not silently absorbed)
//  ============================================================================
//  - VTestSummaryReport (213L) and RecordByTimeOEE (505L, read only ~120 of
//    505 lines) were NOT read to completion -- gated the moment a disqualifying
//    pattern was confirmed (SAVEFILE/SECS-adjacent for the former,
//    create-dir + FTP-upload-guard for the latter). Their FULL internal
//    surface (whether they ALSO touch TestSocket, machine actions, etc.
//    beyond what stopped the read) is UNCATALOGUED.
//  - CheckLotInfor (602L) and DoRecordReportByTime (180L) WERE read to
//    completion and are gated on link boundary ALONE (L-2/L-1) -- these are
//    the two highest-value re-visit targets for a future wave that already
//    owns an ht9045_sm-reaching edge for this file.
//  - The golden `.dfm`-sourced default VALUES for every declared widget
//    (colors, sizes, initial Text) were not ported -- only the C++-code-path
//    reachable state (mostly `Text=""` from DoInitialLotInfor) is faithful;
//    a widget's value before DoInitialLotInfor first runs is this port's own
//    default-constructed TfLotInfoEdit, not golden's `.dfm` value.
//  - No attempt was made to resolve the `fConfiguration` (X-1) or
//    `fSortCT::btnClearCountClick` (X-2) missing-symbol gates, or to invent
//    a `TForm`/`Application` port for ShowInputDialog (T-1) -- each is a
//    cross-file decision outside this wave's write boundary.
//  - No build/ctest was run as part of deciding what to translate (per task
//    rule, verification is `g++ -fsyntax-only` + a full `-c` compile to a
//    .o + `nm` symbol inspection, not a cmake configure/build).
// =============================================================================
#ifndef FORMS_FMESSYSTEM_H
#define FORMS_FMESSYSTEM_H

#include "forms/FormWidgets.h"   // AnsiString / TStringList / TLabel / TListBox / TfLotInfoEdit
#include "Public/cJSON.h"        // cJSON* (uReadDeviceNumer::SearchObject/Read2DFile)
#include "MachineType.h"         // eTrayCount (array bound below) -- header-only, same precedent as forms/fLotInfo.h/forms/fBinSel.h's own direct include
#include <map>

// ===========================================================================
//  uReadDeviceNumer -- non-VCL translation of golden's uReadDeviceNumer
//  (Mes/fVATMesFileSys.h:20-34). A small cJSON-backed 2DID list reader; not a
//  TForm-derived type in golden either.
// ===========================================================================
class uReadDeviceNumer
{
private:
    cJSON* SearchObject(cJSON *InNode, AnsiString sFilter);   // golden :55-81, ACTIVE
    std::map<int,AnsiString> map2D;
public:
    uReadDeviceNumer() {}
    ~uReadDeviceNumer() {}
    std::map<int,AnsiString> GetList2D() { return map2D; }
    void List2DClear() { map2D.clear(); }
    int  List2DGetSize() { return (int)map2D.size(); }
    void List2DoAdd(AnsiString info) { map2D[(int)map2D.size()] = info; }

    bool Read2DFile(AnsiString sFileNameWithPath);             // golden :83-118, ACTIVE

    // -- GATED (W-18): raw fopen(...,"w") -- declared, NOT defined. Calling
    //    this is a LINK ERROR by design (same idiom as every other gate).
    void WriteFile(AnsiString asLocationFile, AnsiString asContent);  // golden :120-125
};

// ===========================================================================
//  TfMesSystem -- non-VCL facade (golden Mes/fVATMesFileSys.h:37-225,
//  `class TfMesSystem : public TForm`; NO base class here, see DEVIATION D-4).
// ===========================================================================
class TfMesSystem
{
public:
    TfMesSystem();
    ~TfMesSystem();                          // PORT-ONLY, see DEVIATION D-3
    TfMesSystem(const TfMesSystem&) = delete;
    TfMesSystem& operator=(const TfMesSystem&) = delete;

    // -- golden non-widget data members actually used by this wave's ACTIVE
    //    bodies, or pre-committed by an `#if 0` call site (see banner) ------
    bool bShow;                                                            // golden :142 -- CheckEocapStatus/FormClose/ctor
    int  iSiteLayouts[2][4];                                               // golden :144 -- DoInitialLotInfor
    int  iJamRateTotalForAlways;                                           // golden :185 -- DoInitailRecordReportByTime/ctor
    bool bNoRTBinFlag[3] = {};                                             // golden :223, see DEVIATION D-6 -- NeedNoRTBinID
    bool bFormShowJustInitial;                                             // golden :181 -- #if 0 field, no ACTIVE consumer this wave
    bool bFirstMaterialsQA;                                                // golden :166 -- ctor only (redirected by asendic_Auto.cpp's OWN #define in that TU; see banner)
    AnsiString asGetRcsCheckingResult;                                     // golden :204 -- #if 0 field, ctor sets "PASS"
    std::map<int,AnsiString> map2DIDFromServer;                            // golden :207 -- IsMatchServerData (ZERO-WRITER, see banner)
    uReadDeviceNumer uRDN;                                                 // golden :206

    // -- golden .dfm widgets touched by THIS WAVE's delivered bodies -------
    TfLotInfoEdit *LabeledEditLotNo;                                       // golden :45  (TLabeledEdit*) -- #if 0 field (cprod.cpp:1267), no ACTIVE consumer this wave
    TfLotInfoEdit *LabeledHandlerDevice;                                   // golden :48  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledTestTemp;                                        // golden :49  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledSoakTime;                                        // golden :50  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledEditLotSize;                                     // golden :51  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledEditContFail;                                    // golden :57  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledLeadCount;                                       // golden :58  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledAutoBin1;                                        // golden :59  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledAutoBin2;                                        // golden :60  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledAutoBin3;                                        // golden :61  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledFixBin1;                                         // golden :62  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledFixBin2;                                         // golden :63  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledFixBin3;                                         // golden :64  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledSiteGap;                                         // golden :65  -- DoInitialLotInfor
    TfLotInfoEdit *LabeledForcePerPin;                                     // golden :66  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceLowYield;                            // golden :67  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtProcess;                                          // golden :68  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtLabeledFTAutoBin1;                                // golden :69  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtLabeledFTAutoBin2;                                // golden :70  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtCustCode;                                         // golden :78  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtCustPart;                                         // golden :79  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtInternalLot;                                      // golden :80  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtCustLotNum;                                       // golden :88  (TLabeledEdit*) -- #if 0 field (cprod.cpp:1266) + DoInitialLotInfor
    TfLotInfoEdit *lbledtDevicePassBins;                                   // golden :94  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtDeviceRetestBins;                                 // golden :95  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceATCcheck;                            // golden :97  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceSiteCheck;                           // golden :98  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceSOTdelay;                            // golden :99  -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceTestTempVar;                         // golden :100 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceGPIBaddress;                         // golden :101 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceOpenBins;                            // golden :102 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerID;                                        // golden :103 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceHotplateXDevision;                   // golden :104 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceHotplateYDevision;                   // golden :105 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceHotplateXStart;                      // golden :106 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceHotplateYStart;                      // golden :107 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceHotplateXPitch;                      // golden :108 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceHotplateYPitch;                      // golden :109 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceTrayXDevision;                       // golden :110 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceTrayYDevision;                       // golden :111 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceTrayXStart;                          // golden :112 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceTrayYStart;                          // golden :113 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceTrayXPitch;                          // golden :114 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceTrayYPitch;                          // golden :115 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceSiteLayouts;                         // golden :116 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceCleanPadLimit;                       // golden :117 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceInshuttleSensorCheck;                // golden :127 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtTestTimePcs;                                      // golden :128 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtC1;                                               // golden :129 (TLabeledEdit*) -- #if 0 field (cprod.cpp:1268) + DoInitialLotInfor
    TfLotInfoEdit *lbledtTestSites;                                        // golden :130 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtIndexTime;                                        // golden :131 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceNotRepeatableArea;                   // golden :132 -- DoInitialLotInfor
    TfLotInfoEdit *lbledtHandlerDeviceICNum;                               // golden :133 -- DoInitialLotInfor
    TLabel        *labEocapStatus;                                        // golden :93  (TLabel*) -- CheckEocapStatus
    TListBox      *listbMesFileLog;                                       // golden :44  (TListBox*) -- RecordMsg

    // -- TStringList* families DoInitialLotInfor clears/re-sizes -----------
    TStringList *asUnloadBin[eTrayCount];                                  // golden :150
    TStringList *asFTUnloadBin[eTrayCount];                                // golden :151
    TStringList *asPassUnloadBin;                                         // golden :152
    TStringList *asFailUnloadBin;                                         // golden :153
    TStringList *stOEEState12hList;                                       // golden :158 -- CleaOEEState12hList

    // -- ACTIVE this wave (defined in forms/fMesSystem.cpp) -----------------
    void        DoInitialLotInfor();                                      // golden :248-318   ACTIVE
    void        CleaOEEState12hList();                                    // golden :2214-2217 ACTIVE
    void        DoInitailRecordReportByTime();                            // golden :1871-1896 ACTIVE
    int         TimeToSec(AnsiString asString);                           // golden :2234-2245 ACTIVE
    bool        CheckEocapStatus(AnsiString asString);                    // golden :2912-2936 ACTIVE
    void        FormClose();                                              // golden :3062-3066 ACTIVE, see DEVIATION D-5
    bool        CheckVTENGmode(AnsiString asLotId);                       // golden :3068-3076 ACTIVE
    bool        IsMatchServerData(AnsiString as2DID);                     // golden :3178-3188 ACTIVE (ZERO-WRITER, see banner)
    void        RecordMsg(AnsiString asMsg);                              // golden :3190-3197 ACTIVE
    bool        AutoSiteMapPass(int iBin);                                // golden :3354-3367 ACTIVE
    bool        NeedNoRTBinID(int iFix);                                  // golden :3369-3382 ACTIVE

    // -- GATED this wave -- declared, NOT defined.  Calling one of these is
    //    a LINK ERROR by design (see safety posture banner above). ---------
    void        buttonSaveClick(TObject *Sender);                         // golden :166-210   GATE (W-01)
    void        FormShow(TObject *Sender);                                // golden :212-246   GATE (W-03, transitive via X-1)
    void        buttonDownloadLotInforClick(TObject *Sender);             // golden :320-971   GATE (W-02)
    void        DoEnableLotInforForMesFile(bool bFlag);                   // golden :973-1035  GATE (X-1)
    bool        CheckLotInfor();                                          // golden :1037-1638 ACTIVE（AI(W906-T5-W5H) 20260919 解閘，602 行逐行翻譯）
    void        VTestSummaryReport();                                     // golden :1640-1852 GATE (W-04)
    void        DoRecordReportByTime(bool bRecrodOEE = false);            // golden :1902-2081 GATE (L-1)
    void        DoSaveReportByTime();                                     // golden :2087-2107 GATE (W-06)
    void        SaveStringlRecordReport();                                // golden :2112-2133 GATE (W-07)
    int         RunModeRW(bool bRead, AnsiString asLot, AnsiString asRunMode); // golden :2165-2184 GATE (W-09)
    void        LoadStringlRecordReport();                                // golden :2186-2199 GATE (W-10)
    void        DeleteStringlRecordReport();                              // golden :2201-2212 GATE (W-11)
    void        RecordByTimeOEE(int iHour);                               // golden :2248-2752 GATE (W-12)
    void        VTestUPHReport();                                         // golden :2754-2794 GATE (W-13)
    void        SendJamCode(AnsiString asJamCode);                        // golden :2796-2905 GATE (W-14)
    void        btnSendJamCodeClick(TObject *Sender);                     // golden :2907-2910 GATE (transitive -> W-14)
    bool        QueryEocapStatus(bool bManualTest);                       // golden :2938-3055 GATE (W-15)
    void        btnQueryEocapStatusClick(TObject *Sender);                // golden :3057-3060 GATE (transitive -> W-15)
    bool        GetRcsCheckingResult(bool bShowAlarm);                    // golden :3079-3146 GATE (S-1)
    void        btnGetRcsCheckingResultClick(TObject *Sender);            // golden :3148-3151 GATE (transitive -> S-1)
    bool        Get2DIDFromServer(AnsiString asServerIP);                 // golden :3153-3176 GATE (W-16)
    AnsiString  GetServerWebAPI(AnsiString asURL);                        // golden :3199-3212 GATE (S-2)
    void        WriteOEEState(AnsiString asString);                       // golden :3214-3221 GATE (transitive -> W-17)
    void        SetOEEState(int iState);                                  // golden :3223-3260 GATE (transitive -> W-17)
    void        AddOEEState(AnsiString asOEECode);                        // golden :3262-3352 GATE (W-17)
    void        ClearMesData();                                          // golden :3384-3444 GATE (X-2)

    // AI(W906-T5-W5H) 20260919: 三個從下面那張「刻意不宣告」清單**移出來**的成員。
    //   那條 idiom 的原文是 "not (yet) needed by any ACTIVE or GATED body" ——
    //   括號裡的 (yet) 現在到期了：CheckLotInfor 於本波解閘並翻譯，三個都要用。
    //   照 idiom 的精神加，不是違反它。
    //   ⓘ 風險量過：forms/fMesSystem.h 全樹只被 **2 個檔** include
    //   （WebStart.cpp 與 forms/fMesSystem.cpp 自己），對照 forms/fMain.h 是 28 個。
    //   這三個是 plain data，不是要 new 的 widget。
    bool        bNewBinCheck;                                             // golden fVATMesFileSys.h:143 -- CheckLotInfor :1218
    double      dIndexTime;                                               // golden :149 -- CheckLotInfor :1058-1061（⚠ 那裡會**寫**它）
    bool        bDownloadLotInforFlag;                                    // golden :167 -- CheckLotInfor :1052

    // -- misc golden non-widget members not (yet) needed by any ACTIVE or
    //    GATED body above; intentionally NOT declared (minimal-declaration
    //    idiom, matches forms/fIoSetView.h's own DEVIATION D-2 practice):
    //    iGetRcsCount, iOEETotalPass, iPlanOut12h, dTestTime,
    //    dtOEETimeStart/End/Diff, stOEEState, stringlRecordReport,
    //    bVATToChangeWorkFile, bCanShowMESReport,
    //    bCreateManualEOCAP_OK, iRunSecondTemp, iUPHXCount,
    //    asVATToChangeWorkFileName, asShowJam, dwStartTimeForAlways.
};

// AI(W906-FW3-MES1) 20260827: TfMesSystem/fMesSystem globals were FREE
// tree-wide (see banner) -- declared here per this wave's task brief so the
// 11 #if 0 call sites listed above have something real to bind to once each
// site's OWN file is separately un-gated (not done by this wave).
extern TfMesSystem *fMesSystem;

#endif // FORMS_FMESSYSTEM_H
