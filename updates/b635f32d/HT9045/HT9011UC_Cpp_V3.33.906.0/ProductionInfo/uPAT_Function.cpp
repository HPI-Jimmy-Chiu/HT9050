// =============================================================================
//  ProductionInfo/uPAT_Function.cpp  --  the PANTHER "PAT" production-data
//                                       bridge + the machine-time stopwatch.
//
//  Faithful translation of golden ProductionInfo/uPAT_Function.cpp (1687 lines,
//  BCB6, Big5/cp950, MIXED line endings -- 1117 CRLF + 570 bare LF; the port is
//  LF-only, per this tree's rule for NEW files).
//  Translator: AI(W906-PT-W4-prodinfo) 20260808
//  Translation wave: W906-PT-W4 (last tier of genuinely-missing non-form units).
//
//  ROLE
//  ----
//  ONE customer's data bridge (PANTHER, CUSTOMER_CODE==CC_PANTHER==869,
//  鴻谷科技 -- MachineType.h:234), doing three unrelated jobs behind one class:
//
//   (1) PAT SETUP RECORD.  AnalyzePATSetUpText slices 88 named fields out of a
//       single flat >=5051-character ASCII record by ABSOLUTE 1-based column
//       offset (SubString(1,6), SubString(7,40), ...).  Every offset and every
//       width is reproduced EXACTLY -- they are a wire format, not arithmetic,
//       so nothing there may be "simplified".  LoadSetupFile feeds it from
//       FileInfo::DecodeASCII.
//
//   (2) INI PERSISTENCE, two files.  PATServer.ini (Path/Time/Type/Password
//       sections) and a per-recipe Job .ini carrying NINE machine-parameter
//       groups (Tray / Plate / Temperature / Contact / AutoClean / Tester /
//       Vacuum / Speed / Alarm3).  Each group has FOUR methods and the naming is
//       confusingly regular, so state it once:
//           Set<Group>Config(TIniFile*, const S&)  = struct  -> ini    (write)
//           Get<Group>Config(TIniFile*, S&)        = ini     -> struct (read)
//           Set<Group>ToSetting(S&)                = MACHINE -> struct (harvest
//                                                    the live VCL form values)
//           Get<Group>ToSetting(const S&)          = struct  -> MACHINE (push
//                                                    them back into the forms)
//       The 18 Config methods are pure ini/struct plumbing and are ALL ACTIVE.
//       The 18 ToSetting methods are the form-coupled half, and they are where
//       15 of this unit's 18 gates live -- see GATE REGISTER.
//
//   (3) FIXED-WIDTH PRODUCTION REPORTS.  GetRealTimeReport composes a
//       human-readable report out of GetBasicData + 4 tables + the jam log, and
//       GenerateRealTime/Hourly/EndLot Report write it to disk (<PC_NAME>.asc /
//       <lot>_<PC_NAME>_<start>_<end>.txt / .asc).  All of the counters it reads
//       (LastSet.BinCT/SendCT/iJamCount/iContactCT/SystemAccSecond,
//       TastCategory.iCountCategory, TestIF.iSiteMap, RunInfo) are REAL globals
//       in this tree, so the report bodies are ACTIVE, not stubbed.
//
//  TMachineTimeManager is a separate, self-contained 4-state stopwatch
//  (STATE_STOPPED / STATE_RUNNING / STATE_PAUSED / STATE_PAUSED_BEFORE_TEST)
//  over TDateTime.  All 10 of its bodies are ACTIVE and depend on NOTHING
//  outside vclcompat + MachineType.h's enum.  Golden's own instance is a
//  by-value member of TfMain (golden main.h:1713) driven from main.cpp:6112/
//  :6376, mymessbox.cpp:380, note.cpp:1044, uLotInfo.cpp:2014/:8370 -- none of
//  which is ported, which is what GATE [G1] is about.
//
//  GOLDEN DEFECTS DELIBERATELY PRESERVED (kept, NOT fixed -- this is a port):
//    * GetOutputCount (golden :487) prints the "Total" row using
//      `IntToStr(toutputCnt.iAuto1)` -- the Auto1 count, NOT iTotal, which was
//      just computed by CalculateTotal() one line above (:479).  Every other row
//      uses its own field.  Copy-paste slip in golden; reproduced verbatim.
//    * GetOutputSiteBinSummer (golden :551) loops `for(idata=0; idata<=MAX_BINS;
//      idata++)` i.e. 0..15 INCLUSIVE = 16 iterations, writing
//      `Tray[..].Bins[idata]` whose declared bound is Bins[MAX_BINS] == 15
//      (uPAT_Function.h:521).  Bins[15] is ONE PAST THE END.  The same
//      `<=MAX_BINS` shape recurs in GetOutputSiteBinYieldSummer (:582).  In
//      golden's own TTrayData layout the very next member is `int Total;`, so the
//      stray write lands on Total and is immediately overwritten by
//      CalculateTotal() (:556) -- which is presumably why it was never noticed.
//      This port keeps BOTH the loop bound AND the struct layout byte-identical,
//      so the behaviour (including which neighbouring field absorbs the write) is
//      the same.  DO NOT "fix" either half in isolation: fixing only the layout
//      would turn a benign overwrite into a real out-of-bounds write.
//    * GetDoublePercentStr (golden :1546) formats with `"%s%"` -- a trailing LONE
//      '%' with no conversion after it.  Kept verbatim.
//    * CheckMachineStationOnStart (golden :826) calls
//      `ShowMyMessage("...(%f)...(%f).",dNowTemp,dTarTemp)`.  ShowMyMessage is
//      NOT printf-like (golden mymessbox.h:58 -- five fixed AnsiString/bool
//      parameters), so the two doubles are converted by AnsiString(double) into
//      S2 and S3 and the "%f" tokens are shown to the operator literally.  Kept
//      verbatim; both trees' AnsiString has the same implicit double ctor
//      (vclcompat/AnsiString.h:71), so this compiles and misbehaves identically.
//    * GetAverageTestTime / GetAveragePauseTime / GetDoublePercentStr all pass
//      `double` through ChangeToFloatNonPcnt, whose return type is `float`
//      (MachineType.h:1601) -- precision is lost mid-expression.  Kept.
//    * SecondsToDateTime takes `int seconds` and is called with doubles (golden
//      :422/:429/:613/:707), so the value truncates at the call.  Kept.
//    * GetAveragePauseTime is labelled "Average Pure Test Time" by its caller
//      (golden :615) while actually averaging stPauseTime.  Golden's label; kept.
//
//  WAVE SCOPE -- 114 golden bodies, ALL 114 present.  102 fully ACTIVE, 12 carry
//  a gate (10 whole-body, 2 partial).  ZERO golden bodies omitted.
//    PAT_Function::PAT_Function                     golden :23   ACTIVE
//    PAT_Function::~PAT_Function                    golden :31   ACTIVE
//    DoIniDataToForm                                golden :35   ACTIVE
//    DoIniDatas                                     golden :43   ACTIVE
//    AnalyzePATSetUpText                            golden :52   ACTIVE
//    LoadSetupFile                                  golden :147  ACTIVE
//    SaveServerINI                                  golden :153  ACTIVE
//    LoadServerINI                                  golden :171  ACTIVE
//    SetPathConfig                                  golden :190  ACTIVE
//    GetPathConfig                                  golden :214  ACTIVE
//    SetTimeConfig                                  golden :239  ACTIVE
//    GetTimeConfig                                  golden :253  ACTIVE
//    SetTypeConfig                                  golden :268  ACTIVE
//    GetTypeConfig                                  golden :279  ACTIVE
//    SetPasswordConfig                              golden :291  ACTIVE
//    GetPasswordConfig                              golden :298  ACTIVE
//    ReadINIString                                  golden :306  ACTIVE
//    WriteINIString                                 golden :311  ACTIVE
//    GetRealTimeReport                              golden :316  ACTIVE
//    GetRealTimeRpt_FormatRow                       golden :344  ACTIVE
//    GetRealTimeRpt_OutCnt                          golden :354  ACTIVE
//    GetDoubleToStr                                 golden :368  ACTIVE
//    GetTotalContactCount                           golden :373  ACTIVE
//    GetCTbyChannel                                 golden :381  ACTIVE
//    ClearRptStruct                                 golden :408  ACTIVE
//    GetAverageTestTime                             golden :418  ACTIVE
//    GetAveragePauseTime                            golden :425  ACTIVE
//    GetOffSiteNumber                               golden :432  ACTIVE
//    GetJamRate                                     golden :440  ACTIVE
//    GetIndexCycleTime                              golden :457  ACTIVE
//    GetOutputCount                                 golden :471  ACTIVE
//    GetOutputTrayBinSummer                         golden :491  ACTIVE
//    GetOutputSiteBinSummer                         golden :524  ACTIVE
//    GetOutputSiteBinYieldSummer                    golden :566  ACTIVE
//    GetBasicData                                   golden :594  ACTIVE except GATE [G1] (3 of 35 rows)
//    GetJamLogList                                  golden :634  ACTIVE
//    GetTimeFomate                                  golden :645  ACTIVE (adaptation A1)
//    GetEndLotTime                                  golden :650  ACTIVE (adaptation A2)
//    GetStartLotTimeStr                             golden :680  ACTIVE
//    GetEndLotTimeStr                               golden :690  ACTIVE
//    GetTestTimeStr                                 golden :701  ACTIVE
//    GenerateRealTimeReport()                       golden :711  ACTIVE
//    GenerateRealTimeReport(const AnsiString&)      golden :719  ACTIVE
//    GenerateHourlyReport()                         golden :727  ACTIVE
//    GenerateHourlyReport(const AnsiString&)        golden :735  ACTIVE
//    GenerateEndLotReport()                         golden :746  ACTIVE
//    GenerateEndLotReport(const AnsiString&)        golden :754  ACTIVE
//    GetPATServerINI                                golden :765  ACTIVE
//    GetPATJobFileDownload                          golden :773  ACTIVE
//    GetPATJobFileUpload                            golden :780  ACTIVE
//    GetPATSetUpFile                                golden :787  ACTIVE
//    GetDayHourMinSecStr                            golden :794  ACTIVE
//    CheckMachineStationOnStart                     golden :807  ACTIVE
//    GetClosedSitesName                             golden :846  ACTIVE
//    GetProgramName                                 golden :874  ACTIVE
//    GetSoakTime                                    golden :900  ACTIVE
//    AnalysisTempAndTolerance                       golden :914  ACTIVE
//    SaveJobINI                                     golden :936  ACTIVE (GATE [G18] shim)
//    LoadJobINI                                     golden :975  ACTIVE
//    SetJobTrayConfig                               golden :1009 ACTIVE (GATE [G17] shim)
//    GetJobTrayConfig                               golden :1025 ACTIVE
//    SetJobTrayToSetting                            golden :1042 GATED-OUT-WHOLE [G2]
//    GetJobTrayToSetting                            golden :1056 GATED-OUT-WHOLE [G3]
//    SetJobPlateConfig                              golden :1074 ACTIVE (GATE [G17] shim)
//    GetJobPlateConfig                              golden :1088 ACTIVE
//    SetJobPlateToSetting                           golden :1103 GATED-OUT-WHOLE [G4]
//    GetJobPlateToSetting                           golden :1122 GATED-OUT-WHOLE [G5]
//    SetJobTemperatureConfig                        golden :1133 ACTIVE
//    GetJobTemperatureConfig                        golden :1142 ACTIVE
//    SetJobTemperatureToSetting                     golden :1152 ACTIVE (no form dep -- LastSet/Temperature only)
//    GetJobTemperatureToSetting                     golden :1159 ACTIVE except GATES [G6][G7]
//    SetJobContactConfig                            golden :1178 ACTIVE (GATE [G17] shim)
//    GetJobContactConfig                            golden :1186 ACTIVE
//    SetJobContactToSetting                         golden :1195 ACTIVE (no form dep -- DeviceForm_File only)
//    GetJobContactToSetting                         golden :1201 GATED-OUT-WHOLE [G8]
//    SetJobAutoCleanConfig                          golden :1208 ACTIVE (GATE [G17] shim)
//    GetJobAutoCleanConfig                          golden :1239 ACTIVE
//    SetJobAutoCleanToSetting                       golden :1271 GATED-OUT-WHOLE [G9]
//    GetJobAutoCleanToSetting                       golden :1297 GATED-OUT-WHOLE [G10]
//    SetJobTesterConfig                             golden :1325 ACTIVE
//    GetJobTesterConfig                             golden :1336 ACTIVE
//    SetJobTesterToSetting                          golden :1348 GATED-OUT-WHOLE [G11]
//    GetJobTesterToSetting                          golden :1355 GATED-OUT-WHOLE [G12]
//    SetJobVacuumConfig                             golden :1369 ACTIVE (GATE [G17] shim)
//    GetJobVacuumConfig                             golden :1383 ACTIVE
//    SetJobVacuumToSetting                          golden :1398 GATED-OUT-WHOLE [G13]
//    GetJobVacuumToSetting                          golden :1410 GATED-OUT-WHOLE [G14]
//    SetJobSpeedConfig                              golden :1424 ACTIVE
//    GetJobSpeedConfig                              golden :1436 ACTIVE
//    SetJobSpeedToSetting                           golden :1449 GATED-OUT-WHOLE [G15]
//    GetJobSpeedToSetting                           golden :1459 GATED-OUT-WHOLE [G16]
//    SetJobAlarm3Config                             golden :1480 ACTIVE
//    GetJobAlarm3Config                             golden :1496 ACTIVE
//    SetJobAlarm3ToSetting                          golden :1513 ACTIVE (golden body is EMPTY -- one `//`)
//    GetJobAlarm3ToSetting                          golden :1518 ACTIVE (golden body is EMPTY -- one `//`)
//    GetPAT_Folder                                  golden :1523 ACTIVE
//    GetJobName                                     golden :1530 ACTIVE
//    GetDoublePercentStr                            golden :1536 ACTIVE
//    SetStartLotTime                                golden :1551 ACTIVE
//    SetEndLotTime                                  golden :1556 ACTIVE
//    SetJamLog                                      golden :1561 ACTIVE
//    GetAreaByBin                                   golden :1572 ACTIVE
//    GetBinByArea                                   golden :1579 ACTIVE
//    SecondsToDateTime                              golden :1592 ACTIVE
//    TMachineTimeManager::TMachineTimeManager       golden :1598 ACTIVE
//    TMachineTimeManager::~TMachineTimeManager      golden :1606 ACTIVE
//    TMachineTimeManager::UpdateTime                golden :1610 ACTIVE
//    TMachineTimeManager::StartLot                  golden :1629 ACTIVE
//    TMachineTimeManager::Pause                     golden :1643 ACTIVE
//    TMachineTimeManager::Resume                    golden :1653 ACTIVE
//    TMachineTimeManager::EndLot                    golden :1664 ACTIVE
//    TMachineTimeManager::GetTotalRunningTime       golden :1673 ACTIVE
//    TMachineTimeManager::GetTotalPauseTime         golden :1678 ACTIVE
//    TMachineTimeManager::GetTotalPauseTimeBeforeTest golden :1683 ACTIVE
//
//  DECLARED-BUT-NEVER-DEFINED IN GOLDEN (so also not here -- this is golden's
//  own state, NOT an omission by this port).  Verified by grepping BOTH trees for
//  each name (see ABSENCE CLAIM A6):
//    TMachineTimeManager::ReadEdtListIni()   declared uPAT_Function.h:702
//    TMachineTimeManager::WriteEdtListIni()  declared uPAT_Function.h:703
//    TMachineTimeManager::InitEdtList()      declared uPAT_Function.h:705
//  Golden's own uPAT_Function.cpp defines none of the three and golden's whole
//  tree never calls them, so a golden link succeeds too.  They are reproduced as
//  DECLARATIONS in the ported header (verbatim) and left undefined.  Whoever adds
//  the first caller has to WRITE the bodies -- there is nothing to port.
//  (GetFileName() at header :704 IS defined, inline, in the header.)
//
//  SATISFIED-BY-SUBSTRATE (already real in this tree; each verified this wave with
//  the .cpp that actually holds the body AND the archive it compiles into, because
//  "a header declares it" is not link closure):
//    CUSTOMER_CODE, PC_NAME, bTestSiteUse[][][], Tempture_Hot/_AmbientHot,
//      GetTotalYield_Str()                    cmydef.h  -> cmydef_TotalYield.cpp (ht9045_globals)   AI(W906-TOTALYIELD) 20260927: this line said cmydef.cpp, whose copy is #if 0 -- the reference was unresolved until now (INBOX 81)
//    Prod, TestIF, TestIF_File, RunInfo, Temperature, DeviceForm_File,
//      dtStartLot, GetSiteCount(bool)         cprod.h   -> cprod.cpp    (ht9045_globals)
//    IniConfig (HT9045_CONFIG)                Config.h  -> Config.cpp   (ht9045_globals)
//    LastSet (LAST_GENERAL_SET)               LastSet.h -> LastSet.cpp  (ht9045_globals)
//    GetTimeInfo(), SystemYear/Month/Date/Hour/Min/Sec
//                                    cpublic.h/cmydef.h -> cpublic.cpp  (ht9045_globals)
//    uTimeTool (DiffTimeSec / TDatetmeToSavestring)
//                          PMAlarm/uTimeTool.h -> PMAlarm/uTimeTool.cpp (ht9045_globals)
//    FileInfo (DecodeASCII/IsFilePathExist/GetNewLine/EnsureDirectoriesExist/
//      PathCombin/SaveAsTxtFile), cMathTool (IsDoubleEqual)
//                  ProductionInfo/FileInfo.h -> ProductionInfo/FileInfo.cpp (ht9045_sm)
//    TastCategory (TEST_CATEGORY.iCountCategory)
//                                             cSocket.h -> cSocket.cpp  (ht9045_sm)
//    InArmSuck / TestSocket (TMyKitSuck) -- see TWO HEADERS note below
//                        aHotPlateSubstrate.h -> aHotPlateSubstrate.cpp (ht9045_sm)
//    ShowMyMessage(AnsiString,...)     canary_support.h -> canary_support.cpp (ht9045_sm)
//    fMain (TfMain: edWorkTemperBase / edSoakTime / SetTemp)
//                                    forms/fMain.h -> forms/fMain.cpp   (ht9045_forms)
//    ChangeToFloatNonPcnt<T>, MachineState, e3Auto1..e3Fix3, stPauseTime /
//      stProductTime, tcHotPlate1/2           MachineType.h  (header-only)
//    TIniFile, AnsiString, TDateTime, IntToStr/FloatToStr/StrToIntDef/
//      StrToFloatDef/Now/FormatDateTime/EncodeDate/EncodeTime/DecodeDate/
//      DecodeTime                             vclcompat      (vclcompat archive)
//
//  TWO HEADERS, ONE CLASS NAME -- the TMyKitSuck trap, checked BEFORE compiling
//  (campaign trap 5).  Golden includes "MyKitSuck.h" (golden :11) for `InArmSuck`
//  (golden MyKitSuck.h:357) and `TestSocket` (:368).  In THIS tree
//  `class TMyKitSuck` is defined TWICE with DIFFERENT LAYOUTS -- mykitsuck.h:274
//  and aHotPlateSubstrate.h:365 -- and both headers declare the SAME extern
//  globals.  The ones that actually get LINKED are aHotPlateSubstrate.cpp's
//  (mykitsuck.cpp is DELIBERATELY NOT in the build; CMakeLists.txt:2011-2022 says
//  so in full).  So this file includes "aHotPlateSubstrate.h", NOT "mykitsuck.h".
//  Compiling against the wrong one would have LINKED CLEANLY and then read every
//  field at the wrong offset.  The only members this file touches are
//  `TestSocket.iShtRow` / `.iShtCol` (aHotPlateSubstrate.h:406/:438) and
//  `InArmSuck.Suck[i][j].sName` (TMySucker::sName, aHotPlateSubstrate.h:111) --
//  all three present on the substrate layout, and `InArmSuck.Suck[][].sName` is
//  already the established tree-wide idiom (ainarm9045.cpp:1357 and ~30 more).
//
//  STATIC-INIT AUDIT (campaign trap 4): this unit defines NO global object and NO
//  file-scope object of any kind, so it contributes ZERO static initialisers.
//  Golden constructs PAT_Function from inside a function (`patFunc=new
//  PAT_Function();` at golden main.cpp:2263, a TfMain method) and holds
//  TMachineTimeManager by value inside TfMain -- neither is a static-init
//  construction even in golden.  The ONE static object introduced here is a
//  FUNCTION-LOCAL static inside GetBasicData (GATE [G1]), constructed on first
//  call rather than before main(), whose ctor touches nothing but its own members
//  (golden :1598-1604: three TDateTime = 0 and state = STATE_STOPPED).  Nothing in
//  this file is reachable from another TU's static initialiser.  Cross-checked
//  against docs/PT_CAMPAIGN_PLAN.md section 8's 18-NULL-pointer table: this unit
//  references NONE of those 18 globals (no HTEditList, no TMyStringList, no thread
//  object) -- grepped, not assumed.
//
//  GATE REGISTER -- 18 gates.  15 of them are one and the same finding: this
//  unit's job (2) pushes/pulls machine parameters through SEVEN different VCL
//  forms, and only two of those seven have any usable stand-in in this tree.
//  What exists, measured this wave (ABSENCE CLAIMS A1-A4 carry the commands):
//      fMain            forms/fMain.h            REAL object, but NO
//                                                `machineTime` member and NO
//                                                ChangeTempMode method
//      fTrayForm        forms/fTrayForm.h        REAL object, 2 members only
//                                                (asErrorMsg,
//                                                 IsEnableColorSensor)
//      fProductionInfo  forms/fProductionInfo.h  REAL object, 2 members only
//                                                (sLoadMO_TestFlow,
//                                                 CalTrayICCount)
//      fCleaning        forms/fCleaning.h        REAL object, 4 members, none of
//                                                the 22 this file reads
//      fContact         atester_shims.h          REAL object (TfContactShim),
//                                                none of the 3 this file reads
//      fHotPlate        -- NO `class TfHotPlate` and NO `fHotPlate` global
//                          anywhere in this tree (golden cHotPlate.h:72)
//      fTemp_Set        -- NO `class TfTemp_Set` and NO `fTemp_Set` global
//                          (golden uTemp_Set.h)
//      FTestIF          -- NO `class TFTestIF` and NO `FTestIF` global
//                          (golden cTesterIF.h)
//      fSpeed           -- NO `class TfSpeed` and NO `fSpeed` global
//  PRECISION ON THOSE LAST FOUR, because a loose grep for the bare identifiers
//  DOES return hits and it would be wrong to report "nothing exists".  What
//  exists is other units' own local workarounds for the SAME gap, in three
//  different shapes, and NONE of them is a linkable global this file could use:
//    * `#if 0` gating -- cprod.cpp:2647-2653 (`fSpeed->ReadFile()`, comment
//      "blocked by fSpeed@not declared anywhere in ported tree") and
//      cprod.cpp:3999-4013 (a 12-line fTemp_Set->rgIndexHeatMode block, same
//      comment shape).  bthermo.cpp gate G27 and uHeaterThread.cpp gate item 6
//      do the same for two other fTemp_Set members.
//    * TU-LOCAL stand-in object -- Automation/auto9045.cpp:398/:904/:925
//      (`W5FA_FTemp_Set`), MyTempPanel.h (`W5FA_TfTemp_SetExt`).
//    * macro no-op -- Automation/SCK_ART_Remainder.cpp:149
//      (`W5SCKARTREM_FTESTIF_READTESTIFFILE()` -> `do{}while(0)`).
//  THIS FILE FOLLOWS THE FIRST SHAPE (`#if 0`), which is cprod.cpp's own choice
//  for these exact two identifiers, and deliberately NOT the second: a TU-local
//  stand-in for a `Set<G>ToSetting` HARVESTER would read a private object nobody
//  ever writes and quietly return zeros dressed up as measured values, and for a
//  `Get<G>ToSetting` APPLIER it would write machine parameters into an object no
//  other TU can see.  Both are indistinguishable from working code at the call
//  site.  A `#if 0` cannot be mistaken for either.
//  ALSO CHECKED AND NOT CONFUSED WITH THE FORM: cprod.h:1361-1373 declares
//  `TRAY_TYPE_PARA HotPlateForm` / `_File` / `_NET` and cprod.h:376 a
//  `TRAY_DATA HotPlateForm[2]`.  Those are hot-plate GEOMETRY DATA structs whose
//  name merely resembles a form; they are not golden's `fHotPlate` TForm and are
//  NOT what [G4] needs.
//  A SHARED CONSEQUENCE, stated once instead of 15 times: `Set<G>ToSetting`
//  HARVESTS the live form values into the struct, and `Get<G>ToSetting` APPLIES
//  the struct to the machine.  Gating a `Set` one means SaveJobINI (golden :936)
//  persists whatever the struct already held -- which, on a fresh PAT_Function, is
//  each struct's own ctor-zeroed state (uPAT_Function.h's Clear() bodies), i.e.
//  the Job .ini gets ZEROS for that group instead of the operator's teach values.
//  Gating a `Get` one means a Job .ini downloaded from the PAT server is PARSED
//  correctly (the Config half is active, so the struct and any later Save
//  round-trip are right) but is NOT APPLIED to the machine.  NEITHER direction can
//  silently move an axis or change a force: every gated body is pure UI read/write
//  plus, in a few cases, a Save-button Click().  That is why "no-op" is the
//  faithful offline default here rather than a guess.
//
//  GATE NECESSITY PROOF (so that none of the 18 rests on an absence claim alone).
//  Every gate below was checked by OPENING it and compiling, not just by grepping:
//    * All 16 `#if 0` arms flipped to `#if 1` at once, same command line as the
//      clean build -> 81 errors, and every single one is a "has no member named" /
//      "was not declared in this scope" for exactly the symbol the gate names.  No
//      gate was silent, i.e. there is no gate here that was closed for nothing.
//    * The two macro gates ([G17],[G18]) checked the same way, by deleting the
//      `#define`s -> 26 errors: 24x "class vclcompat::TIniFile has no member named
//      WriteFloat" plus "FileCreate/FileClose was not declared in this scope".
//  Anyone retiring a gate should re-run that flip: it is the cheapest possible
//  check of whether the gap has since been filled by another wave.
//
//   [G1] golden :618-620 -- `fMain->machineTime.GetTotalPauseTimeBeforeTest()` /
//        `.GetTotalPauseTime()` and their sum: the "Stop Time(Before Test)" /
//        "Stop Time(After Test)" / "Total Stop Time" report rows.  Golden's
//        machineTime is a BY-VALUE TfMain member (golden main.h:1713); this tree's
//        forms/fMain.h TfMain has no such member and this wave may not edit that
//        pre-existing header.  DEFAULT CHOSEN: a function-local
//        `static TMachineTimeManager` -- literally the class defined lower down in
//        THIS file -- so the three rows are produced by the same code golden uses,
//        just from an instance nobody drives.  WHY THAT IS FAITHFUL AND NOT A
//        GUESS: every one of golden's six drivers of machineTime (main.cpp:6112/
//        :6376, mymessbox.cpp:380, note.cpp:1044, uLotInfo.cpp:2014/:8370) lives in
//        an UNPORTED unit, so even if the member existed on this tree's TfMain
//        nothing would ever call StartLot/Pause/Resume/EndLot and it would read 0.0
//        too -- the printed bytes are identical either way, which is a stronger
//        statement than "0 is a safe default".  BEHAVIOUR DELTA ON A REAL MACHINE:
//        golden prints real accumulated stop times; this build prints "0:0:0:0" for
//        all three rows until BOTH fMain gains the member AND main.cpp/note.cpp/
//        uLotInfo.cpp land.  RETIREMENT: delete the #else and restore the #if 0
//        arm the moment forms/fMain.h has `TMachineTimeManager machineTime;`.
//   [G2] golden :1044-1053 -- SetJobTrayToSetting whole body: 9 reads of fTrayForm
//        widgets (TrayName1/XST1/YST1/XPitch1/YPitch1/XCT1/YCT1/Tp1TickUp/
//        Tp1Thick).  forms/fTrayForm.h has none of them.  DEFAULT: no-op; `tray`
//        keeps its incoming value.  DELTA: SaveJobINI writes the [Tray] group from
//        the struct's prior (ctor-zeroed) content instead of the live tray teach
//        form.  Note golden :1052 hard-codes `tray.DimemsionY = 315;` -- that
//        constant is golden's, and it is preserved inside the #if 0 arm for whoever
//        retires this gate.
//        AI(W906-TRAY-READ) 20260923: 上面「forms/fTrayForm.h has none of them」**已過期** ——
//        T5 在 forms/fTrayForm.h 補上了這 9 個控制項（TrayName1…Tp1Thick，值由 cTrayForm.cpp 的
//        Init()/ReadFile() 經 HTEditList 填入）。本閘今天沒有解：解開要另開一顆行為變更 commit
//        （SaveJobINI 的 [Tray] 群組會開始寫真值），而且 golden :1052 的 DimemsionY=315 要一起帶。
//   [G3] golden :1058-1071 -- GetJobTrayToSetting whole body:
//        `fProductionInfo->sucTrayForm` (a SetTrayForm_Greatek, whose TYPE is
//        already real here -- ProductionInfo/uSetUpConfiguration.h -- but which is
//        not a member of this tree's TfProductionInfo) plus
//        `fProductionInfo->SettingTrayFormFromServerOnlyOne()`.  DEFAULT: no-op.
//        DELTA: a downloaded Job .ini's tray geometry is not pushed to the tray
//        form.  The type being real is exactly why this must be a gate and not an
//        "invent a local struct" shim: a local copy would compile, look right, and
//        update an object no other TU can see.
//   [G4] golden :1105-1119 -- SetJobPlateToSetting whole body:
//        `fProductionInfo->OverWritePlateFormData(17 args)` +
//        `SettingHotPlateFormFromServer()` + 6 reads of fHotPlate widgets +
//        `PlateTablePath` (common.h).  DEFAULT: no-op.  DELTA: [Plate] group
//        harvested as zeros; the hot-plate form is not rewritten.
//   [G5] golden :1124-1130 -- GetJobPlateToSetting whole body: the same two
//        fProductionInfo methods.  DEFAULT: no-op.  DELTA: a downloaded Job .ini's
//        plate geometry is not applied.
//   [G6] golden :1166-1173 -- the `if(atof(...)>25) ChangeTempMode(1,false,true)
//        else ChangeTempMode(0,false,true)` pair inside
//        GetJobTemperatureToSetting.  PARTIAL, deliberately: the surrounding lines
//        are ACTIVE because their dependencies are real --
//        fMain->edWorkTemperBase/edSoakTime ARE real members (forms/fMain.h:264/
//        :551, both `TfLotInfoEdit*` == vclcompat::TEdit, allocated by TfMain's
//        ctor at forms/fMain.cpp:85/:147), Temperature.fWorkTemperBase/fSoakTime
//        ARE real globals (cprod.h:1392/:1378), and fMain->SetTemp(bool,double,
//        double) IS a real virtual (forms/fMain.h:517).  ONLY ChangeTempMode is
//        missing from TfMain.  DEFAULT: skip the mode switch.  DELTA: the two edits
//        and the two globals ARE written and SetTemp IS called, but the
//        hot/ambient MODE is not switched, so a Job .ini that raises the target
//        above 25 degrees will not move the machine out of ambient mode.  This is
//        the one gate in this file with a real safety-adjacent flavour, so it is
//        called out rather than folded into the shared paragraph above.
//   [G7] golden :1175 -- `fTemp_Set->spbSave->Click();` at the end of
//        GetJobTemperatureToSetting.  fTemp_Set does not exist in this tree at all.
//        DEFAULT: skip.  DELTA: the temperature page's Save is not pressed, so the
//        values written to the two edits above are not persisted by that form's own
//        writer.  Golden :1161 is a commented-out `fTemp_Set->rgTemperatureMode->`
//        line -- golden's own comment, kept verbatim OUTSIDE the gate since it
//        compiles to nothing either way.
//   [G8] golden :1203-1205 -- GetJobContactToSetting whole body: fContact->
//        edForcePerPinG / edPinCount / spbSave->Click().  TWO independent reasons,
//        both measured: (i) this tree's fContact is TfContactShim
//        (atester_shims.h:154, `extern TfContactShim *fContact;` at :251) and has
//        NONE of those three members; (ii) this file does not include
//        atester_shims.h at all, so `fContact` is not even in scope here -- with
//        the gate opened the compiler reports `'fContact' was not declared in this
//        scope`, and adding the include would then expose reason (i).  Stating
//        both because reason (ii) alone would be a false "the shim is missing"
//        claim.  DEFAULT: no-op.  DELTA: a downloaded pin-count / gram-force is
//        not applied to the
//        contact page.  GOLDEN DEFECT INSIDE THE GATE, kept: golden writes
//        `edForcePerPinG->Text = IntToStr(contact.TheNoOfPins)` and
//        `edPinCount->Text = FloatToStr(contact.gf)` -- the two fields are CROSSED
//        relative to their names, and relative to SetJobContactToSetting
//        (:1197-1198, which reads them the right way round).  Do not un-cross it
//        while retiring the gate without deciding that separately.
//   [G9] golden :1273-1294 -- SetJobAutoCleanToSetting whole body: 22 reads of
//        fCleaning widgets.  forms/fCleaning.h TfCleaning has 4 members, none of
//        them.  DEFAULT: no-op.  DELTA: [AutoClean] group harvested as zeros.  NOTE
//        golden itself never harvests ContinuousFail / ContinuousFailCount /
//        AlarmCount (declared in the struct, written to the ini by
//        SetJobAutoCleanConfig, but absent from this harvester) -- golden's own gap,
//        visible in the #if 0 arm; not something this port introduced.
//  [G10] golden :1299-1322 -- GetJobAutoCleanToSetting whole body: the same 22
//        widgets plus `fCleaning->SaveAutoCleanData()`.  DEFAULT: no-op.  DELTA: a
//        downloaded auto-clean recipe is not applied and not saved.
//  [G11] golden :1350-1352 -- SetJobTesterToSetting whole body: FTestIF->
//        rgInterfaceType / cbGPIBType / edGPIBAddress.  No FTestIF anywhere.
//        DEFAULT: no-op.  DELTA: [Tester] group harvested as zeros.
//  [G12] golden :1357-1366 -- GetJobTesterToSetting whole body: 4 FTestIF widget
//        writes + `FTestIF->SaveSetupFile(szDir,S)` + GetLastOpenFN() + DataPath
//        (both common.h/common.cpp, in ht9045_core, which ht9045_sm does NOT link
//        -- CMakeLists.txt:2102 says so explicitly).  DEFAULT: no-op.  DELTA: a
//        downloaded tester interface / GPIB address is not applied and no setup
//        file is rewritten.  SECOND REASON THIS MUST STAY GATED FOR NOW: un-gating
//        it would drag ht9045_core into ht9045_sm's link line.
//  [G13] golden :1400-1407 -- SetJobVacuumToSetting whole body: 8 reads of fSpeed
//        widgets.  No fSpeed anywhere.  DEFAULT: no-op.  DELTA: [Vacuum] group
//        harvested as zeros.
//  [G14] golden :1412-1421 -- GetJobVacuumToSetting whole body: 8 fSpeed writes +
//        spbSave->Click().  DEFAULT: no-op.  DELTA: downloaded vacuum / destroy
//        wait times are not applied.
//  [G15] golden :1451-1456 -- SetJobSpeedToSetting whole body: 6 reads of fSpeed
//        widgets.  DEFAULT: no-op.  DELTA: [Speed] group harvested as zeros.
//  [G16] golden :1461-1477 -- GetJobSpeedToSetting whole body: 9 fSpeed writes +
//        spbSave->Click().  DEFAULT: no-op.  DELTA: downloaded axis speeds are not
//        applied.  This is the only gated body that would write real MOTION
//        parameters if un-gated against a real form -- flagged for whoever retires
//        it.
//  [G17] `iniFile->WriteFloat(section, ident, double)` -- 24 call sites (counted,
//        not estimated: golden lines 1013,1014,1015,1016,1019,1020,1021 in
//        SetJobTrayConfig; 1078,1079,1080,1081,1084,1085 in SetJobPlateConfig;
//        1183 in SetJobContactConfig; 1231,1232 in SetJobAutoCleanConfig;
//        1373-1380 in SetJobVacuumConfig).  This tree's vclcompat/IniFiles.h
//        TIniFile HAS ReadFloat (:154) but deliberately has NO WriteFloat -- its
//        own banner (:53-55) records why: "There is NO WriteFloat call anywhere in
//        common.cpp's ini family", which was true until THIS unit arrived.  This
//        wave may not edit that pre-existing header, so the call is satisfied by a
//        TU-local function-like macro (the established `#define`-a-local-shim idiom
//        of Automation/auto9045.cpp:73/:99):
//            #define WriteFloat(sec,key,val) WriteString(sec,key,FloatToStr(val))
//        WHY THAT IS THE FAITHFUL DEFAULT AND NOT AN APPROXIMATION: BCB6's
//        TCustomIniFile::WriteFloat IS `WriteString(Section, Ident,
//        FloatToStr(Value))` -- the macro is that body, expanded at the call site.
//        vclcompat's FloatToStr is 15-significant-digit, matching Delphi's, and
//        vclcompat's ReadFloat parses '.'-separated decimals, so the write/read
//        round-trip through Get<G>Config is exact.  BEHAVIOUR DELTA: none within
//        this tree.  The one theoretical difference is locale: Delphi's FloatToStr
//        honours the system decimal separator, so an ini written by a BCB6 build on
//        a comma-decimal machine would differ -- that is a pre-existing property of
//        vclcompat's locale-independent choice, not something this gate introduces.
//        RETIREMENT: add `void WriteFloat(const AnsiString&, const AnsiString&,
//        double);` to vclcompat/IniFiles.h + IniFiles.cpp and delete the macro.
//        See INTEGRATION REQUESTS in the wave report.
//  [G18] `FileCreate(FilePath)` / `FileClose(fileHandle)` -- golden :940/:943, the
//        "create the file if SaveJobINI was handed a path that does not exist yet"
//        bootstrap.  This BCB6 SysUtils int-handle trio has NO vclcompat shim
//        anywhere in this tree (checked; common.cpp:1305-1312 records hitting the
//        same wall and chose a TStringList substitution instead).  Satisfied here
//        by two TU-local static shims + `#define`, NOT by a substitution, because
//        golden's control flow inspects the handle and RETURNS FALSE when creation
//        fails (golden :941-948) -- a substitution that cannot fail would silently
//        delete that arm.  The shims are `_open(path,
//        _O_CREAT|_O_TRUNC|_O_RDWR|_O_BINARY, ...)` / `_close(h)`, matching BCB6's
//        contract on the only two properties golden depends on: create-or-truncate,
//        and a negative return on failure.  BEHAVIOUR DELTA: none observable
//        through SaveJobINI.  RETIREMENT: replace with a real vclcompat
//        FileCreate/FileClose if one is ever added.
//
//  MINIMAL NECESSARY ADAPTATIONS -- NOT gates, zero behaviour delta, listed so
//  that "no #if 0 here" is not mistaken for "unchanged":
//   (A1) golden :647 `return datetime.FormatString("yyyy/mm/dd hh:nn:ss");` ->
//        `return FormatDateTime("yyyy/mm/dd hh:nn:ss", datetime);`.  BCB6's
//        TDateTime::FormatString(fmt) is defined AS FormatDateTime(fmt,*this);
//        vclcompat/TDateTime.h has the free function (:67) but not the member, and
//        its documented token rules (:13-22) give MONTH for the `mm` after `yyyy/`
//        and MINUTE for `nn` -- the same reading BCB6 gives this exact format
//        string.
//   (A2) golden :662-664 / :667 / :672 -- the member-pointer decode plus the two
//        numeric TDateTime ctors:
//          datetime.DecodeDate(&year,&month,&day)     -> DecodeDate(datetime,year,month,day)
//          datetime.DecodeTime(&hour,&min,&sec,&msec) -> DecodeTime(datetime,hour,min,sec,msec)
//          TDateTime tdDate(year,month,day)           -> TDateTime tdDate = EncodeDate(year,month,day)
//          TDateTime tdTime(hour,0,0,0)               -> TDateTime tdTime = EncodeTime(hour,0,0,0)
//          TDateTime tdTime(hour,min,0,0)             -> TDateTime tdTime = EncodeTime(hour,min,0,0)
//        vclcompat/TDateTime.h provides the free forms (:61-64) and not the member
//        forms; BCB6's Word-taking TDateTime ctors are themselves defined as
//        EncodeDate/EncodeTime, so this is the same computation spelled with the
//        function that exists here.  Golden's own `unsigned short year=0,...`
//        declaration (:661) is UNCHANGED -- vclcompat's Word IS `unsigned short`
//        (TDateTime.h:31), so the reference-taking free functions bind to golden's
//        own locals with no cast and no temporary.
//
//  VCL/Borland conversions: `#pragma hdrstop` (golden :3) and `#pragma
//  package(smart_init)` (golden :20) dropped.  No __fastcall, no __property, no
//  __published, no PACKAGE anywhere in this unit (golden has none either).
//  SOFT_SIMULTE is not referenced by golden here and is not introduced.
//  ZERO vendor-DLL entry points: this unit calls no dmc_*, Acm_*, mn_*, smc_*,
//  _mnet_* or _l1xx_* function (grepped -- campaign trap 3 does not apply).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as real UTF-8 in
//  its original column.  Final gate: ZERO U+FFFD, ZERO CRLF.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "uPAT_Function.h" / "FileInfo.h" / "mymessbox.h" / "cprod.h" /
//     "cmydef.h" / "atester.h" / "MyKitSuck.h" / "common.h" / "uCleaning.h" /
//     "main.h" / "cSocket.h" / "uTimeTool.h" / "ProductionInfo.h"
//   ; #pragma package(smart_init)
//
//   NOT re-included here, and why (each checked against the CURRENT tree):
//     "mymessbox.h"    -- golden's modal-dialog header; not ported.  Its one
//                         symbol this file needs, ShowMyMessage, has its real
//                         compiled body behind canary_support.h/.cpp, which is
//                         this tree's registered home for it (the same
//                         substitution every ported consumer makes).
//     "atester.h"      -- EXISTS in this tree, and is still dropped: this file
//                         references ZERO symbols from it.  The only plausible
//                         reason golden lists it is GetSiteCount(bool), which is
//                         declared in cprod.h in BOTH trees (golden cprod.h:3298,
//                         port cprod.h:3299) and defined at port cprod.cpp:366.
//                         Verified by dropping it and compiling.
//     "MyKitSuck.h"    -- REPLACED by "aHotPlateSubstrate.h", and that is a
//                         correctness choice, not a path fix.  See the TWO HEADERS
//                         note above.
//     "common.h"       -- its three symbols reached from this file (PlateTablePath,
//                         DataPath, GetLastOpenFN) are ALL inside gated arms
//                         ([G4], [G12]).  Dropped so that the gates are the only
//                         thing holding back an ht9045_sm -> ht9045_core link edge;
//                         re-add it in the same commit that retires [G4]/[G12].
//     "uCleaning.h"    -- fCleaning's golden home.  Every use is inside [G9]/[G10].
//                         (This tree's stand-in lives in forms/fCleaning.h, not
//                         uCleaning.h, so re-adding it later means FormsFacade.h,
//                         which is already included below.)
//     "ProductionInfo.h" -- fProductionInfo's golden home (a TForm).  Every use is
//                         inside [G3]/[G4]/[G5].  The TYPE SetTrayForm_Greatek that
//                         [G3] needs is already real at
//                         ProductionInfo/uSetUpConfiguration.h, so retiring [G3]
//                         needs a TfProductionInfo member, not this header.
//     "main.h"         -- golden's giant form unit, not ported.  fMain arrives via
//                         "FormsFacade.h" below, which IS included, because [G6]'s
//                         ACTIVE lines really do touch fMain->edWorkTemperBase /
//                         edSoakTime / SetTemp.
//
//   ADDED here (not in golden's list, needed because golden reached these through
//   main.h / MachineDefine.h's VCL hub):
//     "Config.h"            -- IniConfig (HT9045_CONFIG), 7 fields used.  (Also
//                              arrives transitively via cprod.h:6; named
//                              explicitly because this file uses it directly.)
//     "cpublic.h"           -- GetTimeInfo() (:23), called by SetJamLog.
//     "canary_support.h"    -- ShowMyMessage (see above).
//     "FormsFacade.h"       -- forms/fMain.h's TfMain + the real `fMain` global.
//     "PMAlarm/uTimeTool.h" -- golden's "uTimeTool.h", this tree's real path.
//     <sys/stat.h>          -- _S_IREAD/_S_IWRITE for the [G18] FileCreate shim.
// =============================================================================
#include "MachineDefine.h"

#include "ProductionInfo/uPAT_Function.h"
#include "ProductionInfo/FileInfo.h"     // FileInfo (:129) + cMathTool (:211)
#include "cprod.h"                       // Prod / TestIF / TestIF_File / RunInfo / Temperature / DeviceForm_File / dtStartLot / GetSiteCount
#include "cmydef.h"                      // CUSTOMER_CODE / PC_NAME / bTestSiteUse / Tempture_Hot,_AmbientHot / GetTotalYield_Str / SystemYear..Sec
#include "Config.h"                      // IniConfig (HT9045_CONFIG)
#include "cpublic.h"                     // GetTimeInfo()
#include "aHotPlateSubstrate.h"          // golden "MyKitSuck.h" AS LINKED: TMyKitSuck InArmSuck (:624) / TestSocket (:635), TMySucker::sName (:111)
#include "cSocket.h"                     // TEST_CATEGORY TastCategory (:250)
#include "PMAlarm/uTimeTool.h"           // golden "uTimeTool.h": DiffTimeSec / TDatetmeToSavestring
#include "canary_support.h"              // golden "mymessbox.h" surface AS LINKED: ShowMyMessage
#include "FormsFacade.h"                 // golden "main.h" surface AS LINKED: TfMain + fMain (edWorkTemperBase/edSoakTime/SetTemp)

#include <sys/stat.h>                    // _S_IREAD / _S_IWRITE   (GATE [G18])

//---------------------------------------------------------------------------
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G17] -- TIniFile::WriteFloat.
//   vclcompat/IniFiles.h's TIniFile has ReadFloat but no WriteFloat; this
//   function-like macro IS BCB6's own WriteFloat body (WriteString + FloatToStr)
//   expanded at the call site, so golden's 24 `iniFile->WriteFloat(...)` lines
//   below stay VERBATIM and the retirement is a one-line deletion here.
//   Defined AFTER every #include on purpose, so that no header's declaration of a
//   member called WriteFloat can be mangled by it.  See file-head [G17].
//---------------------------------------------------------------------------
#define WriteFloat(_sec_,_key_,_val_) WriteString(_sec_,_key_,FloatToStr(_val_))

//---------------------------------------------------------------------------
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G18] -- BCB6 SysUtils FileCreate /
//   FileClose (golden SaveJobINI :940/:943).  No vclcompat shim exists for the
//   int-handle file trio (common.cpp:1305-1312 records the same gap).  These two
//   reproduce the only two properties golden's control flow depends on:
//   create-or-truncate, and a NEGATIVE return when that fails (golden tests
//   `fileHandle>=0` and returns false otherwise).  Same TU-local static-shim +
//   #define idiom as Automation/auto9045.cpp:73/:99.  See file-head [G18].
//---------------------------------------------------------------------------
static int W4PAT_FileCreate(const AnsiString& asFileName)
{
    return ::_open(asFileName.c_str(),
                   _O_CREAT | _O_TRUNC | _O_RDWR | _O_BINARY,
                   _S_IREAD | _S_IWRITE);
}
static int W4PAT_FileClose(int iHandle)
{
    return ::_close(iHandle);
}
#define FileCreate W4PAT_FileCreate
#define FileClose  W4PAT_FileClose
//---------------------------------------------------------------------------
PAT_Function::PAT_Function()
{
    if(CUSTOMER_CODE==CC_PANTHER)
    {
        DoIniDatas();
    }
}
//---------------------------------------------------------------------------
PAT_Function::~PAT_Function()
{
}
//---------------------------------------------------------------------------
void PAT_Function::DoIniDataToForm()
{
    if(CUSTOMER_CODE==CC_PANTHER && IniConfig.bB11UsePATServerFile)
    {
        LoadServerINI(GetPATServerINI());
    }
}
//---------------------------------------------------------------------------
void PAT_Function::DoIniDatas()
{
    patSetup.Clear();
    dStartLotTime=0.0;
    dEndLotTime=0.0;
    sPATMode="";
    iPATModeInd=0;
}
//---------------------------------------------------------------------------
bool PAT_Function::AnalyzePATSetUpText(const AnsiString& text)
{
    int iLen=text.Length();
    if (iLen>=5051)
    {
        patSetup.customerCode = text.SubString(1, 6).Trim();
        patSetup.deviceNo = text.SubString(7, 40).Trim();
        patSetup.cLotNo = text.SubString(47, 25).Trim();
        patSetup.lotNo = text.SubString(72, 24).Trim();
        patSetup.accessoryNo1 = text.SubString(96, 25).Trim();
        patSetup.testQty = text.SubString(121, 10).Trim();
        patSetup.waferId = text.SubString(131, 52).Trim();
        patSetup.temp = text.SubString(183, 12).Trim();
        patSetup.employeeNo = text.SubString(195, 15).Trim();
        patSetup.testerNo = text.SubString(210, 15).Trim();
        patSetup.entityNo = text.SubString(225, 10).Trim();
        patSetup.step = text.SubString(235, 30).Trim();
        patSetup.programName = text.SubString(265, 100).Trim();
        patSetup.pgmPath = text.SubString(365, 120).Trim();
        patSetup.recipe = text.SubString(485, 50).Trim();
        patSetup.programVer = text.SubString(535, 10).Trim();
        patSetup.prtProgram = text.SubString(545, 100).Trim();
        patSetup.rtProgram = text.SubString(645, 100).Trim();
        patSetup.gdProgram = text.SubString(745, 100).Trim();
        patSetup.eqcProgram = text.SubString(845, 100).Trim();
        patSetup.cshProgram = text.SubString(945, 100).Trim();
        patSetup.batchFileName = text.SubString(1045, 50).Trim();
        patSetup.codeName = text.SubString(1095, 30).Trim();
        patSetup.codeVersion = text.SubString(1125, 30).Trim();
        patSetup.reloadProgram = text.SubString(1155, 3).Trim();
        patSetup.writeReadCodeDevice = text.SubString(1158, 3).Trim();
        patSetup.eqcCshProgram = text.SubString(1161, 100).Trim();
        patSetup.cshBatchFileExecution = text.SubString(1261, 3).Trim();
        patSetup.scriptFileName = text.SubString(1264, 100).Trim();
        patSetup.scriptFilePath = text.SubString(1364, 120).Trim();
        patSetup.notchDirection = text.SubString(1484, 6).Trim();
        patSetup.pretestFileName = text.SubString(1490, 100).Trim();
        patSetup.retestFileName = text.SubString(1590, 100).Trim();
        patSetup.goldenFileName = text.SubString(1690, 100).Trim();
        patSetup.eqcFileName = text.SubString(1790, 100).Trim();
        patSetup.dutBoardNo = text.SubString(1890, 25).Trim();
        patSetup.cfgServerPath = text.SubString(1990, 120).Trim();
        patSetup.cfgLocalPath = text.SubString(2110, 120).Trim();
        patSetup.cfgProgram = text.SubString(2230, 100).Trim();
        patSetup.siteNumber = text.SubString(2330, 10).Trim();
        patSetup.waferType = text.SubString(2340, 10).Trim();
        patSetup.cpAutoReTest = text.SubString(2350, 3).Trim();
        patSetup.notRtBin = text.SubString(2353, 220).Trim();
        patSetup.failSiteNumber = text.SubString(2573, 100).Trim();
        patSetup.proberCardCurrentTouchDown = text.SubString(2673, 10).Trim();
        patSetup.proberCardLimitTouchDown = text.SubString(2683, 10).Trim();
        patSetup.curiVersion = text.SubString(2693, 30).Trim();
        patSetup.osVersion = text.SubString(2723, 30).Trim();
        patSetup.waferVersion = text.SubString(2753, 20).Trim();
        patSetup.igExcelVersion = text.SubString(2773, 200).Trim();
        patSetup.downloadPath = text.SubString(2973, 200).Trim();
        patSetup.accessoryNo2 = text.SubString(3173, 25).Trim();
        patSetup.accessoryNo3 = text.SubString(3198, 25).Trim();
        patSetup.accessoryNo4 = text.SubString(3223, 25).Trim();
        patSetup.accessoryNo5 = text.SubString(3248, 25).Trim();
        patSetup.s100OIVersion = text.SubString(3273, 20).Trim();
        patSetup.step2 = text.SubString(3293, 20).Trim();
        patSetup.codeName2 = text.SubString(3313, 20).Trim();
        patSetup.registerBatExecution = text.SubString(3333, 3).Trim();
        patSetup.modelFile = text.SubString(3336, 50).Trim();
        patSetup.type = text.SubString(3386, 30).Trim();
        patSetup.dateCode = text.SubString(3416, 30).Trim();
        patSetup.socketNumber = text.SubString(3446, 120).Trim();
        patSetup.auxDataFile = text.SubString(3566, 60).Trim();
        patSetup.driverPath = text.SubString(3626, 80).Trim();
        patSetup.driverSize1 = text.SubString(3706, 20).Trim();
        patSetup.driverSize2 = text.SubString(3726, 20).Trim();
        patSetup.driverNameDisplayPath = text.SubString(3746, 60).Trim();
        patSetup.cLotNoForSLT = text.SubString(3806, 100).Trim();
        patSetup.passBin = text.SubString(3906, 30).Trim();
        patSetup.checkSun = text.SubString(3936, 20).Trim();
        patSetup.releaseCode = text.SubString(3956, 20).Trim();
        patSetup.pgmVersion = text.SubString(3976, 20).Trim();
        patSetup.autoZOSProgram = text.SubString(3996, 100).Trim();
        patSetup.accessoryNo6 = text.SubString(4096, 300).Trim();
        patSetup.pgmPath2 = text.SubString(4396, 300).Trim();
        patSetup.downloadPath2 = text.SubString(4696, 300).Trim();
        patSetup.autoFixtureCal = text.SubString(4996, 3).Trim();
        patSetup.fdEngineMonitor = text.SubString(4999, 3).Trim();
        patSetup.ecnNo = text.SubString(5002, 30).Trim();
        patSetup.nvtProcess = text.SubString(5032, 10).Trim();
        patSetup.nvtFlowStep = text.SubString(5042, 10).Trim();
        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
bool PAT_Function::LoadSetupFile(const AnsiString FilePath)
{
    DoIniDatas();
    return AnalyzePATSetUpText(FileInfo(FilePath).DecodeASCII(FilePath,0));
}
//---------------------------------------------------------------------------
bool PAT_Function::SaveServerINI(const AnsiString FilePath)
{
    if(FileInfo().IsFilePathExist(FilePath))
    {
        TIniFile* iniFile=new TIniFile(FilePath);
        SetPathConfig(iniFile,pathConf);
        SetTimeConfig(iniFile,timeConf);
        SetTypeConfig(iniFile,typeConf);
        SetPasswordConfig(iniFile,sPATFtpPwd);
        delete iniFile;
        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
bool PAT_Function::LoadServerINI(const AnsiString FilePath)
{
    if(FileInfo().IsFilePathExist(FilePath))
    {
        TIniFile* iniFile=new TIniFile(FilePath);
        GetPathConfig(iniFile,pathConf);
        GetTimeConfig(iniFile,timeConf);
        GetTypeConfig(iniFile,typeConf);
        GetPasswordConfig(iniFile,sPATFtpPwd);
        delete iniFile;
        return true;
    }
    else
    {
        ShowMyMessage(AnsiString().sprintf("PATServer.ini file is missing. Path:%s",FilePath));
        return false;
    }
}
//---------------------------------------------------------------------------
void PAT_Function::SetPathConfig(TIniFile* iniFile,const PathConfig &pathConfig)
{
    if(iniFile==NULL)
        return;
    WriteINIString(iniFile,"Path", "IntervalTime", IntToStr(pathConfig.iIntervalTime));
    WriteINIString(iniFile,"Path", "RealTimePath", pathConfig.sRealTimePath);
    WriteINIString(iniFile,"Path", "LoadProduction", pathConfig.sLoadProduction);
    WriteINIString(iniFile,"Path", "BackupToServer", pathConfig.sBackupToServer);
    WriteINIString(iniFile,"Path", "SockLifeTime", pathConfig.sSockLifeTime);
    WriteINIString(iniFile,"Path", "CleanSocketPath", pathConfig.sCleanSocketPath);
    WriteINIString(iniFile,"Path", "SpecialOriginPath", pathConfig.sSpecialOriginPath);
    WriteINIString(iniFile,"Path", "SpecialSaveToPath", pathConfig.sSpecialSaveToPath);
    WriteINIString(iniFile,"Path", "JobFileUpDownloadPath", pathConfig.sJobFileUpDownloadPath);
    WriteINIString(iniFile,"Path", "RealTimeSubTestPath", pathConfig.sRealTimeSubTestPath);
    WriteINIString(iniFile,"Path", "ServerJobFilePath", pathConfig.sServerJobFilePath);
    WriteINIString(iniFile,"Path", "SendJamLogFilePath", pathConfig.sSendJamLogFilePath);
    WriteINIString(iniFile,"Path", "sRunCorrelationIniPath", pathConfig.sRunCorrelationIniPath);
    WriteINIString(iniFile,"Path", "sConfirmSiteMapIniPath", pathConfig.sConfirmSiteMapIniPath);
    WriteINIString(iniFile,"Path", "EngJobFileUploadPath", pathConfig.sEngJobFileUploadPath);
    WriteINIString(iniFile,"Path", "sRunInfoTesterPath", pathConfig.sRunInfoTesterPath);
    WriteINIString(iniFile,"Path", "sOneCycleReportPath", pathConfig.sOneCycleReportPath);
    WriteINIString(iniFile,"Path", "sRunInfoBackupPath", pathConfig.sRunInfoBackupPath);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetPathConfig(TIniFile* iniFile,PathConfig &pathConfig)
{
    if(iniFile==NULL)
        return false;
    pathConfig.iIntervalTime = atoi(ReadINIString(iniFile,"Path", "IntervalTime").c_str());
    pathConfig.sRealTimePath = ReadINIString(iniFile,"Path", "RealTimePath");
    pathConfig.sLoadProduction = ReadINIString(iniFile,"Path", "LoadProduction");
    pathConfig.sBackupToServer = ReadINIString(iniFile,"Path", "BackupToServer");
    pathConfig.sSockLifeTime = ReadINIString(iniFile,"Path", "SockLifeTime");
    pathConfig.sCleanSocketPath = ReadINIString(iniFile,"Path", "CleanSocketPath");
    pathConfig.sSpecialOriginPath = ReadINIString(iniFile,"Path", "SpecialOriginPath");
    pathConfig.sSpecialSaveToPath = ReadINIString(iniFile,"Path", "SpecialSaveToPath");
    pathConfig.sJobFileUpDownloadPath = ReadINIString(iniFile,"Path", "JobFileUpDownloadPath");
    pathConfig.sRealTimeSubTestPath = ReadINIString(iniFile,"Path", "RealTimeSubTestPath");
    pathConfig.sServerJobFilePath = ReadINIString(iniFile,"Path", "ServerJobFilePath");
    pathConfig.sSendJamLogFilePath = ReadINIString(iniFile,"Path", "SendJamLogFilePath");
    pathConfig.sRunCorrelationIniPath = ReadINIString(iniFile,"Path", "sRunCorrelationIniPath");
    pathConfig.sConfirmSiteMapIniPath = ReadINIString(iniFile,"Path", "sConfirmSiteMapIniPath");
    pathConfig.sEngJobFileUploadPath = ReadINIString(iniFile,"Path", "EngJobFileUploadPath");
    pathConfig.sRunInfoTesterPath = ReadINIString(iniFile,"Path", "sRunInfoTesterPath");
    pathConfig.sOneCycleReportPath = ReadINIString(iniFile,"Path", "sOneCycleReportPath");
    pathConfig.sRunInfoBackupPath = ReadINIString(iniFile,"Path", "sRunInfoBackupPath");
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetTimeConfig(TIniFile* iniFile,const TimeConfig &timeConfig)
{
    if(iniFile==NULL)
        return;
    WriteINIString(iniFile,"Time", "A", timeConfig.A);
    WriteINIString(iniFile,"Time", "B", timeConfig.B);
    WriteINIString(iniFile,"Time", "C", timeConfig.C);
    WriteINIString(iniFile,"Time", "D", timeConfig.D);
    WriteINIString(iniFile,"Time", "E", timeConfig.E);
    WriteINIString(iniFile,"Time", "F", timeConfig.F);
    WriteINIString(iniFile,"Time", "G", timeConfig.G);
    WriteINIString(iniFile,"Time", "H", timeConfig.H);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetTimeConfig(TIniFile* iniFile,TimeConfig &timeConfig)
{
    if(iniFile==NULL)
        return false;
    timeConfig.A=ReadINIString(iniFile,"Time", "A");
    timeConfig.B=ReadINIString(iniFile,"Time", "B");
    timeConfig.C=ReadINIString(iniFile,"Time", "C");
    timeConfig.D=ReadINIString(iniFile,"Time", "D");
    timeConfig.E=ReadINIString(iniFile,"Time", "E");
    timeConfig.F=ReadINIString(iniFile,"Time", "F");
    timeConfig.G=ReadINIString(iniFile,"Time", "G");
    timeConfig.H=ReadINIString(iniFile,"Time", "H");
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetTypeConfig(TIniFile* iniFile,const TypeConfig &typeConfig)
{
    if(iniFile==NULL)
        return;
    WriteINIString(iniFile,"Type", "JamType1", typeConfig.JamType1);
    WriteINIString(iniFile,"Type", "JamType2", typeConfig.JamType2);
    WriteINIString(iniFile,"Type", "JamType3", typeConfig.JamType3);
    WriteINIString(iniFile,"Type", "JamType4", typeConfig.JamType4);
    WriteINIString(iniFile,"Type", "JamType5", typeConfig.JamType5);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetTypeConfig(TIniFile* iniFile,TypeConfig &typeConfig)
{
    if(iniFile==NULL)
        return false;
    typeConfig.JamType1=ReadINIString(iniFile,"Type", "JamType1");
    typeConfig.JamType2=ReadINIString(iniFile,"Type", "JamType2");
    typeConfig.JamType3=ReadINIString(iniFile,"Type", "JamType3");
    typeConfig.JamType4=ReadINIString(iniFile,"Type", "JamType4");
    typeConfig.JamType5=ReadINIString(iniFile,"Type", "JamType5");
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetPasswordConfig(TIniFile* iniFile,const AnsiString &ftppwd)
{
    if(iniFile==NULL)
        return;
    WriteINIString(iniFile,"Password", "PATFtpPwd", ftppwd);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetPasswordConfig(TIniFile* iniFile,AnsiString &ftppwd)
{
    if(iniFile==NULL)
        return false;
    ftppwd=ReadINIString(iniFile,"Password", "PATFtpPwd");
    return true;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::ReadINIString(TIniFile* iniFile,const AnsiString& section, const AnsiString& key, const AnsiString& defaultValue)
{
    return iniFile->ReadString(section, key, defaultValue);
}
//---------------------------------------------------------------------------
void PAT_Function::WriteINIString(TIniFile* iniFile,const AnsiString& section, const AnsiString& key, const AnsiString& value)
{
    iniFile->WriteString(section, key, value);
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetRealTimeReport(const int &iReportType)
{
    ClearRptStruct();
    AnsiString srpt="";
    srpt+=GetBasicData(iReportType);
    //
    srpt+=FileInfo().GetNewLine();
    srpt+=GetRealTimeRpt_FormatRow("-----------------Output Count-----------------------", "");
    srpt+=GetOutputCount();

    srpt+=FileInfo().GetNewLine();
    srpt+=GetRealTimeRpt_FormatRow("-----------------Output Tray Bin Summer-----------------------", "");
    srpt+=GetOutputTrayBinSummer();

    srpt+=FileInfo().GetNewLine();
    srpt+=GetRealTimeRpt_FormatRow("-----------------Output Site Bin Summer-----------------------", "");
    srpt+=GetOutputSiteBinSummer();

    srpt+=FileInfo().GetNewLine();
    srpt+=GetRealTimeRpt_FormatRow("-----------------Output Site Bin Yield Summer-----------------------", "");
    srpt+=GetOutputSiteBinYieldSummer();

    srpt+=FileInfo().GetNewLine();
    srpt+=GetRealTimeRpt_FormatRow("--------------------------------Alarm log----------------------------", "");
    srpt+=GetJamLogList();
    return srpt;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetRealTimeRpt_FormatRow(const AnsiString& sLabel, const AnsiString& sValue)
{
    if(sValue=="")
    {
        return AnsiString().sprintf("%s\n", sLabel.c_str());
    }
    else
        return AnsiString().sprintf("%-26s: %s\n", sLabel.c_str(), sValue.Trim().c_str());
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetRealTimeRpt_OutCnt(const AnsiString& sLabel, const AnsiString& sNum, const AnsiString& sPercent)
{
    int labelWidth=5;
    int numWidth=6;
    int percentWidth=6;
    return AnsiString().sprintf(
            "%-*s: %-*s: %-*s %s",
            labelWidth,   sLabel.c_str(),
            numWidth,     sNum,
            percentWidth, sPercent,
            FileInfo().GetNewLine()
        );
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetDoubleToStr(double dValue)
{
    return AnsiString().sprintf("%02.2f",dValue);
}
//---------------------------------------------------------------------------
int PAT_Function::GetTotalContactCount()
{
    int iret=(LastSet.iContactCT[0]+LastSet.iContactCT[1]);
    if(iret==0)
        iret=1;
    return iret;
}
//---------------------------------------------------------------------------
void PAT_Function::GetCTbyChannel(int iCh,int iRetDatas[16])
{
    int iRowCounts=TestSocket.iShtRow;
    int iColCounts=TestSocket.iShtCol;
    int iret=0,itotal=0;
    for(int iRow=0; iRow<iRowCounts; iRow++)
    {
        for(int iCol=0; iCol<iColCounts; iCol++)
        {
            if(TestIF.iSiteMap[iRow][iCol]>0 && TestIF.iSiteMap[iRow][iCol]==(iCh+1))
            {
                itotal=0;
                for(int iCat=1; iCat<16; iCat++)
                {
                    iret=0;
                    iret+=TastCategory.iCountCategory[0][iRow][iCol][iCat];
                    iret+=TastCategory.iCountCategory[1][iRow][iCol][iCat];
                    iRetDatas[iCat-1]=iret;
                    itotal+=iret;
                }
                iRetDatas[15]=itotal;
                return;
            }
        }
    }
}
//---------------------------------------------------------------------------
void PAT_Function::ClearRptStruct()
{
    toutputCnt.Clear();
    iTotalSites=0;
    for (int i = 0; i < MAX_SITES; i++)
    {
        Tray[i].Clear();
    }
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetAverageTestTime()
{
    double dret=0.0;
    dret=ChangeToFloatNonPcnt((double)(RunInfo.dTestTimeSec), (double)(GetTotalContactCount()));
    return GetDayHourMinSecStr(SecondsToDateTime(dret));
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetAveragePauseTime()
{
    double dret=0.0;
    dret=ChangeToFloatNonPcnt((double)(LastSet.SystemAccSecond[0][stPauseTime]), (double)(GetTotalContactCount()));
    return GetDayHourMinSecStr(SecondsToDateTime(dret));
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetOffSiteNumber()
{
    int iret=0;
    bool bIncludeCloseSite=true;
    iret=GetSiteCount(bIncludeCloseSite)-GetSiteCount(!bIncludeCloseSite);
    return IntToStr(iret);
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetJamRate()
{
    AnsiString str="";
    int sendCT  =LastSet.SendCT[1];
    int jamCount=LastSet.iJamCount[1];
    if(sendCT==0 || jamCount==0)
    {
        str.sprintf("0 / 10000");
    }
    else
    {
        int jamRate=(jamCount*10000)/sendCT;
        str.sprintf("%d / 10000", jamRate);
    }
    return str;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetIndexCycleTime()
{
    AnsiString str="";
    if(RunInfo.IndexTime=="")
    {
        str.sprintf("0");
    }
    else
    {
        str.sprintf("%s", RunInfo.IndexTime);
    }
    return str;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetOutputCount()
{
    toutputCnt.iAuto1=LastSet.BinCT[0][e3Auto1];
    toutputCnt.iAuto2=LastSet.BinCT[0][e3Auto2];
    toutputCnt.iAuto3=LastSet.BinCT[0][e3Auto3];
    toutputCnt.iFix1= LastSet.BinCT[0][e3Fix1];
    toutputCnt.iFix2= LastSet.BinCT[0][e3Fix2];
    toutputCnt.iFix3= LastSet.BinCT[0][e3Fix3];
    toutputCnt.CalculateTotal();
    AnsiString srpt="";
    srpt+=GetRealTimeRpt_OutCnt("Auto1", IntToStr(toutputCnt.iAuto1),GetDoublePercentStr((double)toutputCnt.iAuto1,(double)toutputCnt.iTotal));
    srpt+=GetRealTimeRpt_OutCnt("Auto2", IntToStr(toutputCnt.iAuto2),GetDoublePercentStr((double)toutputCnt.iAuto2,(double)toutputCnt.iTotal));
    srpt+=GetRealTimeRpt_OutCnt("Auto3", IntToStr(toutputCnt.iAuto3),GetDoublePercentStr((double)toutputCnt.iAuto3,(double)toutputCnt.iTotal));
    srpt+=GetRealTimeRpt_OutCnt("Fix1",  IntToStr(toutputCnt.iFix1),GetDoublePercentStr((double)toutputCnt.iFix1,(double)toutputCnt.iTotal));
    srpt+=GetRealTimeRpt_OutCnt("Fix2",  IntToStr(toutputCnt.iFix2),GetDoublePercentStr((double)toutputCnt.iFix2,(double)toutputCnt.iTotal));
    srpt+=GetRealTimeRpt_OutCnt("Fix3",  IntToStr(toutputCnt.iFix3),GetDoublePercentStr((double)toutputCnt.iFix3,(double)toutputCnt.iTotal));
    srpt+=GetRealTimeRpt_OutCnt("Total", IntToStr(toutputCnt.iAuto1)," ");
    return srpt;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetOutputTrayBinSummer()
{
    int labelWidth=6;
    int numBins=15;
    int colWidth=10;
    int numRows=6;
    AnsiString header="Tray :";
    for(int i=0;i<numBins;i++)
    {
        header+=AnsiString().sprintf("%-*s",colWidth,("Bin"+IntToStr(i+1)).c_str());
    }
    header+=FileInfo().GetNewLine();
    AnsiString labels[6]={"Auto1:", "Auto2:", "Auto3:", "Fix1 :", "Fix2 :", "Fix3 :"};
    int data[6][15]={0};
    int iBin=0;
    for(int i=0;i<numRows;i++) //6
    {
        iBin=GetBinByArea(i);
        data[i][iBin-1]=LastSet.BinCT[0][i];
    }
    AnsiString stable=header;
    for(int i=0;i<numRows;i++)
    {
        stable+=AnsiString().sprintf("%-*s",labelWidth,labels[i].c_str());
        for(int j=0;j<numBins;j++)
        {
            stable+=AnsiString().sprintf("%-*d", colWidth, data[i][j]);
        }
        stable+=FileInfo().GetNewLine();
    }
    return stable;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetOutputSiteBinSummer()
{
    int labelWidth=7;
    int colWidth=10;
    AnsiString header="Tray : ";
    for(int i=0;i<MAX_BINS;i++)
    {
        header+=AnsiString().sprintf("%-*s",colWidth,("Bin"+IntToStr(i+1)).c_str());
    }
    header+=AnsiString().sprintf("%-*s",colWidth,AnsiString("Total").c_str());
    header+=FileInfo().GetNewLine();
    int data[16]={0};
    int iRowCounts=TestSocket.iShtRow;
    int iColCounts=TestSocket.iShtCol;
    AnsiString stable=header;
    iTotalSites=0;
    for(int iRow=0; iRow<iRowCounts; iRow++)
    {
        for(int iCol=0; iCol<iColCounts; iCol++)
        {
            if(TestIF.iSiteMap[iRow][iCol]>0)
            {
                if(iTotalSites<MAX_SITES)
                {
                    ZeroMemory(data , sizeof(data));
                    stable+=AnsiString().sprintf("%-*s",labelWidth,("Site"+IntToStr(iTotalSites+1)+":").c_str());//head
                    GetCTbyChannel(iTotalSites,data);
                    for(int idata=0;idata<=MAX_BINS;idata++)
                    {
                        Tray[iTotalSites].Bins[idata]=data[idata];
                        stable+=AnsiString().sprintf("%-*d", colWidth, data[idata]);//datas
                    }
                    Tray[iTotalSites].CalculateTotal();
                    stable+=FileInfo().GetNewLine();
                }
                iTotalSites++;
            }
        }
    }
    return stable;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetOutputSiteBinYieldSummer()
{
    int labelWidth=9;
    int numBins=15;
    int colWidth=10;
    AnsiString header="Tray :   ";
    for(int i=0;i<numBins;i++)
    {
        header+=AnsiString().sprintf("%-*s",colWidth,("Bin"+IntToStr(i+1)).c_str());
    }
    header+=AnsiString().sprintf("%-*s",colWidth,AnsiString("Total").c_str());
    header+=FileInfo().GetNewLine();
    AnsiString stable=header, sRowData="";
    for(int iRow=0; iRow<iTotalSites; iRow++)
    {
        stable+=AnsiString().sprintf("%-*s",labelWidth,("Site"+IntToStr(iRow+1)+":").c_str());//head
        for(int iCol=0; iCol<=MAX_BINS; iCol++)
        {
            sRowData=GetDoublePercentStr((double)Tray[iRow].Bins[iCol],(double)toutputCnt.iTotal);
            stable+=AnsiString().sprintf("%-*s", colWidth, sRowData);
        }
        sRowData=GetDoublePercentStr((double)Tray[iRow].Total,(double)toutputCnt.iTotal);
        stable+=AnsiString().sprintf("%-*s", colWidth, sRowData);//datas
        stable+=FileInfo().GetNewLine();
    }
    return stable;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetBasicData(const int &iReportType)
{
    AnsiString srpt="";
    srpt+=GetRealTimeRpt_FormatRow("Runcard Number",patSetup.lotNo);
    srpt+=GetRealTimeRpt_FormatRow("Cus.Lot Number",patSetup.cLotNo);
    srpt+=GetRealTimeRpt_FormatRow("Part Number",patSetup.deviceNo);
    srpt+=GetRealTimeRpt_FormatRow("Program",GetProgramName(iPATModeInd));
    srpt+=GetRealTimeRpt_FormatRow("Job Name",patSetup.recipe);
    srpt+=GetRealTimeRpt_FormatRow("Op. Name",patSetup.employeeNo);
    srpt+=GetRealTimeRpt_FormatRow("Entity No",patSetup.entityNo);
    srpt+=GetRealTimeRpt_FormatRow("LoadBoard",patSetup.accessoryNo1);
    srpt+=GetRealTimeRpt_FormatRow("Start Time",GetStartLotTimeStr());
    srpt+=GetRealTimeRpt_FormatRow("End Time",GetEndLotTimeStr(iReportType));
    srpt+=GetRealTimeRpt_FormatRow("Test Temperature",patSetup.temp);
    srpt+=GetRealTimeRpt_FormatRow("Soak Time",GetDoubleToStr(Temperature.fSoakTime));
    srpt+=GetRealTimeRpt_FormatRow("Act.Temperature(Plate1)",RunInfo.ShowTempComp[tcHotPlate1]);
    srpt+=GetRealTimeRpt_FormatRow("Act.Temperature(Plate2)",RunInfo.ShowTempComp[tcHotPlate2]);
    srpt+=GetRealTimeRpt_FormatRow("Test Yield", GetTotalYield_Str());
    srpt+=GetRealTimeRpt_FormatRow("Test Time", GetTestTimeStr(iReportType));
    srpt+=GetRealTimeRpt_FormatRow("Pure Test Time", GetDayHourMinSecStr(SecondsToDateTime(LastSet.SystemAccSecond[0][stProductTime]*0.001)));
    srpt+=GetRealTimeRpt_FormatRow("Average Test Time", GetAverageTestTime());
    srpt+=GetRealTimeRpt_FormatRow("Average Pure Test Time", GetAveragePauseTime());
    srpt+=GetRealTimeRpt_FormatRow("Index Time", RunInfo.IndexTime);
    srpt+=GetRealTimeRpt_FormatRow("Idle Time", GetDayHourMinSecStr(SecondsToDateTime(LastSet.SystemAccSecond[0][stPauseTime]*0.001)));
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G1] -- fMain->machineTime.
//   forms/fMain.h TfMain has no `machineTime` member (golden main.h:1713 does);
//   this wave may not edit that pre-existing header.  The #else arm uses a
//   FUNCTION-LOCAL static of the very class defined lower down in this file, so
//   the three rows are produced by golden's own code from an undriven instance.
//   Function-local (not file-scope) on purpose: constructed on first call, never
//   before main() -- campaign trap 4.  See file-head [G1] for why the printed
//   bytes are identical to a widened fMain today.
#if 0   // ---- golden text VERBATIM (golden :618-620) ----
    srpt+=GetRealTimeRpt_FormatRow("Stop Time(Before Test)", GetDayHourMinSecStr(fMain->machineTime.GetTotalPauseTimeBeforeTest()));
    srpt+=GetRealTimeRpt_FormatRow("Stop Time(After Test)", GetDayHourMinSecStr(fMain->machineTime.GetTotalPauseTime()));
    srpt+=GetRealTimeRpt_FormatRow("Total Stop Time", GetDayHourMinSecStr(fMain->machineTime.GetTotalPauseTimeBeforeTest()+fMain->machineTime.GetTotalPauseTime()));
#else
    static TMachineTimeManager W4PAT_machineTime;                                //stands in for golden fMain->machineTime (main.h:1713)
    srpt+=GetRealTimeRpt_FormatRow("Stop Time(Before Test)", GetDayHourMinSecStr(W4PAT_machineTime.GetTotalPauseTimeBeforeTest()));
    srpt+=GetRealTimeRpt_FormatRow("Stop Time(After Test)", GetDayHourMinSecStr(W4PAT_machineTime.GetTotalPauseTime()));
    srpt+=GetRealTimeRpt_FormatRow("Total Stop Time", GetDayHourMinSecStr(W4PAT_machineTime.GetTotalPauseTimeBeforeTest()+W4PAT_machineTime.GetTotalPauseTime()));
#endif
    srpt+=GetRealTimeRpt_FormatRow("Test Frequencey", IntToStr(GetTotalContactCount()));
    srpt+=GetRealTimeRpt_FormatRow("Site Number", IntToStr(GetSiteCount(true)));
    srpt+=GetRealTimeRpt_FormatRow("Off Site Number", GetOffSiteNumber());
    srpt+=GetRealTimeRpt_FormatRow("Off site status", GetClosedSitesName());
    srpt+=GetRealTimeRpt_FormatRow("Oper", patSetup.step);
    srpt+=GetRealTimeRpt_FormatRow("Step", sPATMode);
    srpt+=GetRealTimeRpt_FormatRow("Index Cycle Time", GetIndexCycleTime());
    srpt+=GetRealTimeRpt_FormatRow("UPH", IntToStr(RunInfo.iUPH));
    srpt+=GetRealTimeRpt_FormatRow("Jam Qty", IntToStr(LastSet.iJamCount[1]));
    srpt+=GetRealTimeRpt_FormatRow("Jam rate", GetJamRate());
    return srpt;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetJamLogList()
{
    AnsiString srpt="";
    unsigned int iSize=JamLogs.size();
    for(unsigned int i=0;i<iSize;i++)
    {
        srpt+=AnsiString().sprintf("%s\n", JamLogs[i].c_str());
    }
    return srpt;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetTimeFomate(const TDateTime &datetime)
{
    // AI(W906-PT-W4-prodinfo) 20260808: ADAPTATION (A1) -- golden :647 is
    //   `return datetime.FormatString("yyyy/mm/dd hh:nn:ss");`.  BCB6's
    //   TDateTime::FormatString(fmt) IS FormatDateTime(fmt,*this); vclcompat has
    //   the free function (TDateTime.h:67) and not the member.  Same format
    //   string, same tokens, zero behaviour delta.
    return FormatDateTime("yyyy/mm/dd hh:nn:ss", datetime);
}
//---------------------------------------------------------------------------
TDateTime PAT_Function::GetEndLotTime(const int &iReportType)
{
    double dretEndLotTime=0.0;
    if(iReportType==(int)REPORT_END_LOT)
    {
        dretEndLotTime=dEndLotTime;
    }
    else
    {
        TDateTime datetime=Now();
        TDateTime updatedDateTime;
        unsigned short year=0, month=0, day=0, hour=0, min=0, sec=0, msec=0;
        // AI(W906-PT-W4-prodinfo) 20260808: ADAPTATION (A2) -- golden :662-664 are
        //   `datetime.DecodeDate(&year,&month,&day);`,
        //   `datetime.DecodeTime(&hour,&min,&sec,&msec);` and
        //   `TDateTime tdDate(year,month,day);`.  vclcompat/TDateTime.h supplies
        //   the FREE DecodeDate/DecodeTime (:61-62, reference out-params, Word ==
        //   unsigned short so golden's own locals bind directly) and EncodeDate
        //   (:63), which is exactly what BCB6's Word-taking TDateTime ctor does.
        //   Zero behaviour delta.  Same reasoning for :667 / :672 below.
        DecodeDate(datetime, year, month, day);
        DecodeTime(datetime, hour, min, sec, msec);
        TDateTime tdDate = EncodeDate(year,month,day);
        if(iReportType==(int)REPORT_HOURLY)
        {
            TDateTime tdTime = EncodeTime(hour,0,0,0);                              //ADAPTATION (A2): golden `TDateTime tdTime(hour,0,0,0);`
            updatedDateTime=tdDate+tdTime;
        }
        else//REPORT_REALTIME
        {
            TDateTime tdTime = EncodeTime(hour,min,0,0);                            //ADAPTATION (A2): golden `TDateTime tdTime(hour,min,0,0);`
            updatedDateTime=tdDate+tdTime;
        }
        dretEndLotTime=updatedDateTime;
    }
    return dretEndLotTime;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetStartLotTimeStr()
{
    AnsiString sRetTime="";
    if(dStartLotTime>0.0)
    {
        sRetTime=GetTimeFomate(dStartLotTime);
    }
    return sRetTime;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetEndLotTimeStr(const int &iReportType)
{
    AnsiString sEndLotTime="";
    double dGetEndLotTime=GetEndLotTime(iReportType);
    if(dGetEndLotTime>0.0)
    {
        sEndLotTime=GetTimeFomate(dGetEndLotTime);
    }
    return sEndLotTime;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetTestTimeStr(const int &iReportType)
{
    AnsiString str="";
    double dGetEndLotTime=GetEndLotTime(iReportType);
    double dGetStartLotTime=dStartLotTime;
    double difftimeSec=uTimeTool().DiffTimeSec(dGetEndLotTime,dGetStartLotTime);
    str=GetDayHourMinSecStr(SecondsToDateTime(difftimeSec));
    return str;
}
//---------------------------------------------------------------------------
void PAT_Function::GenerateRealTimeReport()
{
    if(IniConfig.bB12UsePATSetup==true)
    {
        GenerateRealTimeReport(IniConfig.sB14RealTimePath);
    }
}
//---------------------------------------------------------------------------
void PAT_Function::GenerateRealTimeReport(const AnsiString &sPath)
{
    FileInfo().EnsureDirectoriesExist(sPath);
    AnsiString sFileName=AnsiString().sprintf("%s.asc",PC_NAME);
    AnsiString sFileNameWithPath=FileInfo().PathCombin(sPath,sFileName);
    FileInfo().SaveAsTxtFile(GetRealTimeReport(REPORT_REALTIME),sFileNameWithPath);
}
//---------------------------------------------------------------------------
void PAT_Function::GenerateHourlyReport()
{
    if(IniConfig.bB12UsePATSetup==true)
    {
        GenerateHourlyReport(IniConfig.sB14RealTimePath);
    }
}
//---------------------------------------------------------------------------
void PAT_Function::GenerateHourlyReport(const AnsiString &sPath)
{
    FileInfo().EnsureDirectoriesExist(sPath);
    AnsiString sRuncardNumber=patSetup.lotNo;
    AnsiString sStartLotTime=uTimeTool().TDatetmeToSavestring(dtStartLot);
    AnsiString sEndLotTime=uTimeTool().TDatetmeToSavestring(GetEndLotTime(REPORT_HOURLY));
    AnsiString sFileName=AnsiString().sprintf("%s_%s_%s_%s.txt",sRuncardNumber,PC_NAME,sStartLotTime,sEndLotTime);
    AnsiString sFileNameWithPath=FileInfo().PathCombin(sPath,sFileName);
    FileInfo().SaveAsTxtFile(GetRealTimeReport(REPORT_HOURLY),sFileNameWithPath);
}
//---------------------------------------------------------------------------
void PAT_Function::GenerateEndLotReport()
{
    if(IniConfig.bB12UsePATSetup==true)
    {
        GenerateEndLotReport(IniConfig.sB14RealTimePath);
    }
}
//---------------------------------------------------------------------------
void PAT_Function::GenerateEndLotReport(const AnsiString &sPath)
{
    FileInfo().EnsureDirectoriesExist(sPath);
    AnsiString sRuncardNumber=patSetup.lotNo;
    AnsiString sStartLotTime=uTimeTool().TDatetmeToSavestring(dtStartLot);
    AnsiString sEndLotTime=uTimeTool().TDatetmeToSavestring(GetEndLotTime(REPORT_END_LOT));
    AnsiString sFileName=AnsiString().sprintf("%s_%s_%s_%s.asc",sRuncardNumber,PC_NAME,sStartLotTime,sEndLotTime);
    AnsiString sFileNameWithPath=FileInfo().PathCombin(sPath,sFileName);
    FileInfo().SaveAsTxtFile(GetRealTimeReport(REPORT_END_LOT),sFileNameWithPath);
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetPATServerINI()
{
    AnsiString sFileName=AnsiString().sprintf("PATServer.ini");
    FileInfo().EnsureDirectoriesExist(IniConfig.sB11PATServerPath);
    AnsiString sJobFullFileName=FileInfo().PathCombin(IniConfig.sB11PATServerPath,sFileName);
    return sJobFullFileName;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetPATJobFileDownload()
{
    AnsiString sFileName=GetJobName();
    AnsiString sret=FileInfo().PathCombin(IniConfig.sB13PATJobDownloadPath,sFileName);
    return sret;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetPATJobFileUpload()
{
    AnsiString sFileName=GetJobName();
    AnsiString sret=FileInfo().PathCombin(IniConfig.sB13PATJobUploadPath,sFileName);
    return sret;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetPATSetUpFile()
{
    AnsiString sFileName=AnsiString().sprintf("%s.asc",PC_NAME);
    AnsiString sret=FileInfo().PathCombin(IniConfig.sB12PATSetupPath,sFileName);
    return sret;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetDayHourMinSecStr(const double &dtTime)
{
    double ddata=(dtTime<=0)?0:dtTime;
    int days = static_cast<int>(ddata); // Integer part is the number of days
    double fractionalDay = ddata - days; // Fractional part is the time of day
    int hours = static_cast<int>(fractionalDay * 24);
    double fractionalHour = fractionalDay * 24 - hours;
    int minutes = static_cast<int>(fractionalHour * 60);
    double fractionalMinute = fractionalHour * 60 - minutes;
    int seconds = static_cast<int>(fractionalMinute * 60);
    return AnsiString().sprintf("%d:%d:%d:%d",days,hours,minutes,seconds);
}
//---------------------------------------------------------------------------
bool PAT_Function::CheckMachineStationOnStart()
{
    bool bret=true;
    if(IniConfig.bB12UsePATSetup==true)
    {
        if(patSetup.cLotNo=="")
        {
            ShowMyMessage("【B12】Barcode scanning is required to retrieve data first.");
            return false;
        }
        //temp check
        if(LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot)
        {
            double dNowTemp=Temperature.fWorkTemperBase;
            int iSetTemp=0,iTolerance=0;
            AnalysisTempAndTolerance(patSetup.temp,iSetTemp,iTolerance);
            double dTarTemp=double(iSetTemp);
            if(cMathTool().IsDoubleEqual(dNowTemp,dTarTemp)==false)
            {
                ShowMyMessage("【B12】The actual temperature setting(%f) differs from the target temperature(%f).",dNowTemp,dTarTemp);
                return false;
            }
        }
        //Lot start check
        if(cMathTool().IsDoubleEqual(dStartLotTime,0.0))
        {
            ShowMyMessage("【B12】Not Yet Click Lot Start.");
            return false;
        }
        //Lot end reset
        if(cMathTool().IsDoubleEqual(dEndLotTime,0.0)==false)
        {
            ShowMyMessage("【B12】Not Yet Reset to Clear Lot End.");
            return false;
        }
    }
    return bret;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetClosedSitesName()
{
    AnsiString sret="";
    bool bfirst=true;
    int iRowCounts=TestSocket.iShtRow;
    int iColCounts=TestSocket.iShtCol;
    for(int i=0; i<iRowCounts; i++)
    {
        for(int j=0; j<iColCounts; j++)
        {
            if(TestIF_File.iSiteMap[i][j]==0 ||                                 //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
               bTestSiteUse[0][i][j]==false)
            {
                if(bfirst)
                {
                    bfirst=false;
                }
                else
                {
                    sret+=",";
                }
                sret+=InArmSuck.Suck[i][j].sName;
            }
        }
    }
    return sret;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetProgramName(int iTestIdx)
{
    AnsiString sret="";
    if(iTestIdx==0)     //Normal Test
    {
        sret=patSetup.programName;
    }
    else if(iTestIdx==1)//Pre-Test
    {
        sret=patSetup.prtProgram;
    }
    else if(iTestIdx==2)//Re-Test
    {
        sret=patSetup.rtProgram;
    }
    else if(iTestIdx==3)//GD
    {
        sret=patSetup.gdProgram;
    }
    else//4             //EQC
    {
        sret=patSetup.eqcProgram;
    }
    return sret;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetSoakTime()
{
    AnsiString sret="";
    if((LastSet.iTemperature==Tempture_Hot || LastSet.iTemperature==Tempture_AmbientHot))
    {
        sret=GetDoubleToStr(Temperature.fSoakTime);
    }
    else
    {
        sret="0";
    }
    return sret;
}
//---------------------------------------------------------------------------
void PAT_Function::AnalysisTempAndTolerance(const AnsiString &str,int &iTemp,int &iTolerance)
{
    AnsiString sTemp="", sTolerance="";
    int i=1;
    while(i<=str.Length() && (str[i] >= '0' && str[i] <= '9'))
    {
        sTemp+=str[i];
        i++;
    }
    while (i<=str.Length() && !(str[i] >= '0' && str[i] <= '9'))
    {
        i++;
    }
    while (i<=str.Length() && (str[i] >= '0' && str[i] <= '9'))
    {
        sTolerance+=str[i];
        i++;
    }
    iTemp=atoi(sTemp.c_str());
    iTolerance=atoi(sTolerance.c_str());
}
//---------------------------------------------------------------------------
bool PAT_Function::SaveJobINI(const AnsiString FilePath)
{
    if(FileInfo().IsFilePathExist(FilePath)==false)
    {
        int fileHandle=FileCreate(FilePath);
        if (fileHandle>=0)
        {
            FileClose(fileHandle); // 關閉檔案句柄
        }
        else
        {
            return false;
        }
    }
    //setting
    SetJobTrayToSetting(trayconfig);
    SetJobPlateToSetting(plateconfig);
    SetJobTemperatureToSetting(tempconfig);
    SetJobContactToSetting(contactconfig);
    SetJobAutoCleanToSetting(autocleanconfig);
    SetJobTesterToSetting(testerconfig);
    SetJobVacuumToSetting(vacuumconfig);
    SetJobSpeedToSetting(speedConfig);
    SetJobAlarm3ToSetting(alarm3Config);
    //to ini
    TIniFile* iniFile=new TIniFile(FilePath);
    SetJobTrayConfig(iniFile,trayconfig);
    SetJobPlateConfig(iniFile,plateconfig);
    SetJobTemperatureConfig(iniFile,tempconfig);
    SetJobContactConfig(iniFile,contactconfig);
    SetJobAutoCleanConfig(iniFile,autocleanconfig);
    SetJobTesterConfig(iniFile,testerconfig);
    SetJobVacuumConfig(iniFile,vacuumconfig);
    SetJobSpeedConfig(iniFile,speedConfig);
    SetJobAlarm3Config(iniFile,alarm3Config);
    delete iniFile;
    return true;
}
//---------------------------------------------------------------------------
bool PAT_Function::LoadJobINI(const AnsiString FilePath)
{
    if(FileInfo().IsFilePathExist(FilePath))
    {
        TIniFile* iniFile=new TIniFile(FilePath);
        GetJobTrayConfig(iniFile,trayconfig);
        GetJobPlateConfig(iniFile,plateconfig);
        GetJobTemperatureConfig(iniFile,tempconfig);
        GetJobContactConfig(iniFile,contactconfig);
        GetJobAutoCleanConfig(iniFile,autocleanconfig);
        GetJobTesterConfig(iniFile,testerconfig);
        GetJobVacuumConfig(iniFile,vacuumconfig);
        GetJobSpeedConfig(iniFile,speedConfig);
        GetJobAlarm3Config(iniFile,alarm3Config);
        delete iniFile;
        //setting
        GetJobTrayToSetting(trayconfig);
        GetJobPlateToSetting(plateconfig);
        GetJobTemperatureToSetting(tempconfig);
        GetJobContactToSetting(contactconfig);
        GetJobAutoCleanToSetting(autocleanconfig);
        GetJobTesterToSetting(testerconfig);
        GetJobVacuumToSetting(vacuumconfig);
        GetJobSpeedToSetting(speedConfig);
        GetJobAlarm3ToSetting(alarm3Config);
        return true;
    }
    else
    {
        ShowMyMessage(AnsiString().sprintf("Job file is missing. Path:%s",FilePath));
        return false;
    }
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobTrayConfig(TIniFile* iniFile, const TTrayConfig &tray)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteFloat("Tray", "PitchX",         tray.PitchX);
    iniFile->WriteFloat("Tray", "PitchY",         tray.PitchY);
    iniFile->WriteFloat("Tray", "StartPosX",      tray.StartPosX);
    iniFile->WriteFloat("Tray", "StartPosY",      tray.StartPosY);
    iniFile->WriteInteger("Tray", "DivisionX",      tray.DivisionX);
    iniFile->WriteInteger("Tray", "DivisionY",      tray.DivisionY);
    iniFile->WriteFloat("Tray", "DimemsionX",     tray.DimemsionX);
    iniFile->WriteFloat("Tray", "DimemsionY",     tray.DimemsionY);
    iniFile->WriteFloat("Tray", "ZThickness",     tray.ZThickness);
    iniFile->WriteString("Tray", "UserDefineName", tray.UserDefineName);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobTrayConfig(TIniFile* iniFile, TTrayConfig &tray)
{
    if(iniFile==NULL)
        return false;
    tray.PitchX         = iniFile->ReadFloat("Tray", "PitchX", 0.0);
    tray.PitchY         = iniFile->ReadFloat("Tray", "PitchY", 0.0);
    tray.StartPosX      = iniFile->ReadFloat("Tray", "StartPosX", 0.0);
    tray.StartPosY      = iniFile->ReadFloat("Tray", "StartPosY", 0.0);
    tray.DivisionX      = iniFile->ReadInteger("Tray", "DivisionX", 0);
    tray.DivisionY      = iniFile->ReadInteger("Tray", "DivisionY", 0);
    tray.DimemsionX     = iniFile->ReadFloat("Tray", "DimemsionX", 0.0);
    tray.DimemsionY     = iniFile->ReadFloat("Tray", "DimemsionY", 0.0);
    tray.ZThickness     = iniFile->ReadFloat("Tray", "ZThickness", 0.0);
    tray.UserDefineName = iniFile->ReadString("Tray", "UserDefineName", "");
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobTrayToSetting(TTrayConfig &tray)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G2] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1044-1053) ----
    tray.UserDefineName.sprintf("%s",fTrayForm->TrayName1->Text);
    tray.StartPosX      = atof(fTrayForm->XST1->Text.c_str());
    tray.StartPosY      = atof(fTrayForm->YST1->Text.c_str());
    tray.PitchX         = atof(fTrayForm->XPitch1->Text.c_str());
    tray.PitchY         = atof(fTrayForm->YPitch1->Text.c_str());
    tray.DivisionX      = atof(fTrayForm->XCT1->Text.c_str());
    tray.DivisionY      = atof(fTrayForm->YCT1->Text.c_str());
    tray.DimemsionX     = atof(fTrayForm->Tp1TickUp->Text.c_str());
    tray.DimemsionY     = 315;
    tray.ZThickness     = atof(fTrayForm->Tp1Thick->Text.c_str());
#else
    (void)tray;   // deliberately unused while [G2] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobTrayToSetting(const TTrayConfig &tray)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G3] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1058-1071) ----
    SetTrayForm_Greatek *stf=&fProductionInfo->sucTrayForm;
    stf->Clear();
    stf->sTRAYPN = tray.UserDefineName;
    stf->sXStartPos = tray.StartPosX;
    stf->sYStartPos = tray.StartPosY;
    stf->sXPitch = tray.PitchX;
    stf->sYPitch = tray.PitchY;
    stf->sColumnsX = tray.DivisionX;
    stf->sRowsY = tray.DivisionY;
    stf->sXWidth = tray.DimemsionX;
    stf->sYHeight = tray.DimemsionY;
    stf->sZThickness = tray.ZThickness;
    stf->Group = "";
    fProductionInfo->SettingTrayFormFromServerOnlyOne();
#else
    (void)tray;   // deliberately unused while [G3] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobPlateConfig(TIniFile* iniFile, const TPlateConfig &plate)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteFloat("Plate", "PitchX",         plate.PitchX);
    iniFile->WriteFloat("Plate", "PitchY",         plate.PitchY);
    iniFile->WriteFloat("Plate", "StartPositionX", plate.StartPositionX);
    iniFile->WriteFloat("Plate", "StartPositionY", plate.StartPositionY);
    iniFile->WriteInteger("Plate", "DivisionX",      plate.DivisionX);
    iniFile->WriteInteger("Plate", "DivisionY",      plate.DivisionY);
    iniFile->WriteFloat("Plate", "WidthX",         plate.WidthX);
    iniFile->WriteFloat("Plate", "WidthY",         plate.WidthY);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobPlateConfig(TIniFile* iniFile, TPlateConfig &plate)
{
    if(iniFile==NULL)
        return false;
    plate.PitchX         = iniFile->ReadFloat("Plate", "PitchX", 0.0);
    plate.PitchY         = iniFile->ReadFloat("Plate", "PitchY", 0.0);
    plate.StartPositionX = iniFile->ReadFloat("Plate", "StartPositionX", 0.0);
    plate.StartPositionY = iniFile->ReadFloat("Plate", "StartPositionY", 0.0);
    plate.DivisionX      = iniFile->ReadInteger("Plate", "DivisionX", 0);
    plate.DivisionY      = iniFile->ReadInteger("Plate", "DivisionY", 0);
    plate.WidthX         = iniFile->ReadFloat("Plate", "WidthX", 0.0);
    plate.WidthY         = iniFile->ReadFloat("Plate", "WidthY", 0.0);
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobPlateToSetting(TPlateConfig &plate)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G4] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1105-1119) ----
    fProductionInfo->OverWritePlateFormData(PlateTablePath,"",plate.StartPositionX,
                                            plate.StartPositionY,plate.PitchX,plate.PitchY,
                                            plate.DivisionX,plate.DivisionY,plate.WidthX,
                                            plate.WidthY,"0","",
                                            "","0","0",
                                            "0","0");
    fProductionInfo->SettingHotPlateFormFromServer();
    plate.StartPositionX=atof(fHotPlate->XST1->Text.c_str());
    plate.StartPositionY=atof(fHotPlate->YST1->Text.c_str());
    plate.PitchX=atof(fHotPlate->XPitch1->Text.c_str());
    plate.PitchY=atof(fHotPlate->YPitch1->Text.c_str());
    plate.DivisionX=atoi(fHotPlate->XCT1->Text.c_str());
    plate.DivisionY=atoi(fHotPlate->YCT1->Text.c_str());
    plate.WidthX=0.0;
    plate.WidthY=0.0;
#else
    (void)plate;   // deliberately unused while [G4] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobPlateToSetting(const TPlateConfig &plate)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G5] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1124-1130) ----
    fProductionInfo->OverWritePlateFormData(PlateTablePath,"",plate.StartPositionX,
                                            plate.StartPositionY,plate.PitchX,plate.PitchY,
                                            plate.DivisionX,plate.DivisionY,plate.WidthX,
                                            plate.WidthY,"0","",
                                            "","0","0",
                                            "0","0");
    fProductionInfo->SettingHotPlateFormFromServer();
#else
    (void)plate;   // deliberately unused while [G5] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobTemperatureConfig(TIniFile* iniFile, const TTemperatureConfig &temp)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteInteger("Temperature", "TemptureMode", temp.TemptureMode);
    iniFile->WriteInteger("Temperature", "Default",      temp.DefaultValue);
    iniFile->WriteInteger("Temperature", "SoakTime",     temp.SoakTime);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobTemperatureConfig(TIniFile* iniFile, TTemperatureConfig &temp)
{
    if(iniFile==NULL)
        return false;
    temp.TemptureMode = iniFile->ReadInteger("Temperature", "TemptureMode", 0);
    temp.DefaultValue = iniFile->ReadInteger("Temperature", "Default",      0);
    temp.SoakTime     = iniFile->ReadInteger("Temperature", "SoakTime",     0);
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobTemperatureToSetting(TTemperatureConfig &temp)
{
    temp.TemptureMode=LastSet.iTemperature;
    temp.DefaultValue=Temperature.fWorkTemperBase;
    temp.SoakTime=Temperature.fSoakTime;
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobTemperatureToSetting(const TTemperatureConfig &temp)
{
//    fTemp_Set->rgTemperatureMode->ItemIndex=temp.TemptureMode;
    fMain->edWorkTemperBase->Text  =IntToStr(temp.DefaultValue);
    fMain->edSoakTime->Text        =IntToStr(temp.SoakTime);
    Temperature.fWorkTemperBase    =temp.DefaultValue;
    Temperature.fSoakTime          =temp.SoakTime;
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G6] -- see file-head GATE REGISTER.
//   PARTIAL gate: only the ChangeTempMode mode-switch pair is closed.  The
//   edits, the two Temperature globals and SetTemp above/below are ACTIVE --
//   all three ARE real here.  Only TfMain::ChangeTempMode is missing.
#if 0   // ---- golden text VERBATIM (golden :1166-1173) ----
    if(atof(fMain->edWorkTemperBase->Text.c_str())>25)
    {
        fMain->ChangeTempMode(1, false, true);
    }
    else
    {
        fMain->ChangeTempMode(0, false, true);
    }
#else
    // [G6] closed: nothing to substitute -- the gated lines are pure UI
    //   side effects with no non-UI half to keep.
#endif
    fMain->SetTemp(false, atof(fMain->edWorkTemperBase->Text.c_str()), atof(fMain->edSoakTime->Text.c_str()));
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G7] -- see file-head GATE REGISTER.
//   fTemp_Set has no class and no global anywhere in this tree.
#if 0   // ---- golden text VERBATIM (golden :1175-1175) ----
    fTemp_Set->spbSave->Click();
#else
    // [G7] closed: nothing to substitute -- the gated lines are pure UI
    //   side effects with no non-UI half to keep.
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobContactConfig(TIniFile* iniFile, const TContactConfig &contact)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteInteger("Contact", "TheNoOfPins", contact.TheNoOfPins);
    iniFile->WriteFloat("Contact", "gf",            contact.gf);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobContactConfig(TIniFile* iniFile, TContactConfig &contact)
{
    if(iniFile==NULL)
        return false;
    contact.TheNoOfPins = iniFile->ReadInteger("Contact", "TheNoOfPins", 0);
    contact.gf          = iniFile->ReadFloat("Contact", "gf", 0.0);
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobContactToSetting(TContactConfig &contact)
{
    contact.TheNoOfPins=DeviceForm_File.iPinOfDie;
    contact.gf=DeviceForm_File.ForcePerPinG;
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobContactToSetting(const TContactConfig &contact)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G8] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1203-1205) ----
    fContact->edForcePerPinG->Text=IntToStr(contact.TheNoOfPins);
    fContact->edPinCount->Text=FloatToStr(contact.gf);
    fContact->spbSave->Click();
#else
    (void)contact;   // deliberately unused while [G8] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobAutoCleanConfig(TIniFile* iniFile, const TAutoCleanConfig &autoClean)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteInteger("AutoClean", "UseAutoClean",         autoClean.UseAutoClean);
    iniFile->WriteInteger("AutoClean", "InitialStart",         autoClean.InitialStart);
    iniFile->WriteInteger("AutoClean", "InitialRetestStart",   autoClean.InitialRetestStart);
    iniFile->WriteInteger("AutoClean", "Finish",               autoClean.Finish);
    iniFile->WriteInteger("AutoClean", "Manual",               autoClean.Manual);
    iniFile->WriteInteger("AutoClean", "SocketAlarmFail",      autoClean.SocketAlarmFail);
    iniFile->WriteInteger("AutoClean", "SocketAlarmFailCount", autoClean.SocketAlarmFailCount);
    iniFile->WriteInteger("AutoClean", "Interval",             autoClean.Interval);
    iniFile->WriteInteger("AutoClean", "IntervalCount",        autoClean.IntervalCount);
    iniFile->WriteInteger("AutoClean", "ContinuousFail",       autoClean.ContinuousFail);
    iniFile->WriteInteger("AutoClean", "ContinuousFailCount",  autoClean.ContinuousFailCount);
    iniFile->WriteInteger("AutoClean", "NumberPices",          autoClean.NumberPices);
    iniFile->WriteInteger("AutoClean", "Tray",                 autoClean.Tray);
    iniFile->WriteInteger("AutoClean", "AlarmCount",           autoClean.AlarmCount);
    iniFile->WriteInteger("AutoClean", "CleanPadDeviation",    autoClean.CleanPadDeviation);
    iniFile->WriteInteger("AutoClean", "ContactMode",          autoClean.ContactMode);
    iniFile->WriteInteger("AutoClean", "ContactTime",          autoClean.ContactTime);
    iniFile->WriteInteger("AutoClean", "ContactCount",         autoClean.ContactCount);
    iniFile->WriteInteger("AutoClean", "PinCount",             autoClean.PinCount);
    iniFile->WriteFloat("AutoClean",   "PinForceN",            autoClean.PinForceN);
    iniFile->WriteFloat("AutoClean",   "PinForceGf",           autoClean.PinForceGf);
    iniFile->WriteInteger("AutoClean", "OutArmSpeed",          autoClean.OutArmSpeed);
    iniFile->WriteInteger("AutoClean", "ShuttleSpeed",         autoClean.ShuttleSpeed);
    iniFile->WriteInteger("AutoClean", "IndexArmSpeed",        autoClean.IndexArmSpeed);
    iniFile->WriteInteger("AutoClean", "InArmSpeed",           autoClean.InArmSpeed);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobAutoCleanConfig(TIniFile* iniFile, TAutoCleanConfig &autoClean)
{
    if(iniFile==NULL)
        return false;
    autoClean.UseAutoClean         = iniFile->ReadInteger("AutoClean", "UseAutoClean",         0);
    autoClean.InitialStart         = iniFile->ReadInteger("AutoClean", "InitialStart",         0);
    autoClean.InitialRetestStart   = iniFile->ReadInteger("AutoClean", "InitialRetestStart",   0);
    autoClean.Finish               = iniFile->ReadInteger("AutoClean", "Finish",               0);
    autoClean.Manual               = iniFile->ReadInteger("AutoClean", "Manual",               0);
    autoClean.SocketAlarmFail      = iniFile->ReadInteger("AutoClean", "SocketAlarmFail",      0);
    autoClean.SocketAlarmFailCount = iniFile->ReadInteger("AutoClean", "SocketAlarmFailCount", 0);
    autoClean.Interval             = iniFile->ReadInteger("AutoClean", "Interval",             0);
    autoClean.IntervalCount        = iniFile->ReadInteger("AutoClean", "IntervalCount",        0);
    autoClean.ContinuousFail       = iniFile->ReadInteger("AutoClean", "ContinuousFail",       0);
    autoClean.ContinuousFailCount  = iniFile->ReadInteger("AutoClean", "ContinuousFailCount",  0);
    autoClean.NumberPices          = iniFile->ReadInteger("AutoClean", "NumberPices",          0);
    autoClean.Tray                 = iniFile->ReadInteger("AutoClean", "Tray",                 0);
    autoClean.AlarmCount           = iniFile->ReadInteger("AutoClean", "AlarmCount",           0);
    autoClean.CleanPadDeviation    = iniFile->ReadInteger("AutoClean", "CleanPadDeviation",    0);
    autoClean.ContactMode          = iniFile->ReadInteger("AutoClean", "ContactMode",          0);
    autoClean.ContactTime          = iniFile->ReadInteger("AutoClean", "ContactTime",          0);
    autoClean.ContactCount         = iniFile->ReadInteger("AutoClean", "ContactCount",         0);
    autoClean.PinCount             = iniFile->ReadInteger("AutoClean", "PinCount",             0);
    autoClean.PinForceN            = iniFile->ReadFloat("AutoClean",   "PinForceN",            0.0);
    autoClean.PinForceGf           = iniFile->ReadFloat("AutoClean",   "PinForceGf",           0.0);
    autoClean.OutArmSpeed          = iniFile->ReadInteger("AutoClean", "OutArmSpeed",          0);
    autoClean.ShuttleSpeed         = iniFile->ReadInteger("AutoClean", "ShuttleSpeed",         0);
    autoClean.IndexArmSpeed        = iniFile->ReadInteger("AutoClean", "IndexArmSpeed",        0);
    autoClean.InArmSpeed           = iniFile->ReadInteger("AutoClean", "InArmSpeed",           0);
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobAutoCleanToSetting(TAutoCleanConfig &autoClean)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G9] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1273-1294) ----
    autoClean.UseAutoClean = fCleaning->rgAutoCleanOnOff->ItemIndex;
    autoClean.InitialStart = fCleaning->chkAutoCleanMode1->Checked ? 1 : 0;
    autoClean.InitialRetestStart = fCleaning->chkAutoCleanMode2->Checked ? 1 : 0;
    autoClean.Finish = fCleaning->chkAutoCleanMode3->Checked ? 1 : 0;
    autoClean.Manual = fCleaning->chkAutoCleanMode4->Checked ? 1 : 0;
    autoClean.SocketAlarmFail = fCleaning->chkAutoCleanMode5->Checked ? 1 : 0;
    autoClean.SocketAlarmFailCount = StrToIntDef(fCleaning->edAlarmCount->Text, 0);
    autoClean.Interval = fCleaning->chkAutoCleanMode6->Checked ? 1 : 0;
    autoClean.IntervalCount = StrToIntDef(fCleaning->edIntervalContact->Text, 0);
    autoClean.NumberPices = StrToIntDef(fCleaning->edPinsCount->Text, 0);
    autoClean.Tray = fCleaning->rgCleanKitType->ItemIndex;
    autoClean.CleanPadDeviation = StrToIntDef(fCleaning->edCleanPadDeviation->Text, 0);
    autoClean.ContactMode = fCleaning->ContactMode->ItemIndex;
    autoClean.ContactTime = StrToIntDef(fCleaning->edContactTime->Text, 0);
    autoClean.ContactCount = StrToIntDef(fCleaning->edACContactCount->Text, 0);
    autoClean.PinCount = StrToIntDef(fCleaning->edPinsCount->Text, 0);
    autoClean.PinForceN = StrToFloatDef(fCleaning->edPinSingleN->Text, 0.0);
    autoClean.PinForceGf = StrToFloatDef(fCleaning->edPinSingleGf->Text, 0.0);
    autoClean.OutArmSpeed = StrToIntDef(fCleaning->OutArmSpeed->Text, 0);
    autoClean.ShuttleSpeed = StrToIntDef(fCleaning->ShuttleSpeed->Text, 0);
    autoClean.IndexArmSpeed = StrToIntDef(fCleaning->IndexArmSpeed->Text, 0);
    autoClean.InArmSpeed = StrToIntDef(fCleaning->edtInArmZSpeed->Text, 0);
#else
    (void)autoClean;   // deliberately unused while [G9] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobAutoCleanToSetting(const TAutoCleanConfig &autoClean)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G10] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1299-1322) ----
    fCleaning->rgAutoCleanOnOff->ItemIndex=autoClean.UseAutoClean;
    fCleaning->chkAutoCleanMode1->Checked=(autoClean.InitialStart==1)?true:false;
    fCleaning->chkAutoCleanMode2->Checked=(autoClean.InitialRetestStart==1)?true:false;
    fCleaning->chkAutoCleanMode3->Checked=(autoClean.Finish==1)?true:false;
    fCleaning->chkAutoCleanMode4->Checked=(autoClean.Manual==1)?true:false;
    fCleaning->chkAutoCleanMode5->Checked=(autoClean.SocketAlarmFail==1)?true:false;
    fCleaning->edAlarmCount->Text=IntToStr(autoClean.SocketAlarmFailCount);
    fCleaning->chkAutoCleanMode6->Checked=(autoClean.Interval==1)?true:false;
    fCleaning->edIntervalContact->Text=IntToStr(autoClean.IntervalCount);
    fCleaning->edPinsCount->Text=IntToStr(autoClean.NumberPices);
    fCleaning->rgCleanKitType->ItemIndex=autoClean.Tray;
    fCleaning->edCleanPadDeviation->Text=IntToStr(autoClean.CleanPadDeviation);
    fCleaning->ContactMode->ItemIndex=autoClean.ContactMode;
    fCleaning->edContactTime->Text=IntToStr(autoClean.ContactTime);
    fCleaning->edACContactCount->Text=IntToStr(autoClean.ContactCount);
    fCleaning->edPinsCount->Text=IntToStr(autoClean.PinCount);
    fCleaning->edPinSingleN->Text=FloatToStr(autoClean.PinForceN);
    fCleaning->edPinSingleGf->Text=FloatToStr(autoClean.PinForceGf);
    fCleaning->OutArmSpeed->Text=IntToStr(autoClean.OutArmSpeed);
    fCleaning->ShuttleSpeed->Text=IntToStr(autoClean.ShuttleSpeed);
    fCleaning->IndexArmSpeed->Text=IntToStr(autoClean.IndexArmSpeed);
    fCleaning->edtInArmZSpeed->Text=IntToStr(autoClean.InArmSpeed);
    //
    fCleaning->SaveAutoCleanData();
#else
    (void)autoClean;   // deliberately unused while [G10] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobTesterConfig(TIniFile* iniFile, const TTesterConfig &tester)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteInteger("Tester", "TestMode", tester.TestMode);
    iniFile->WriteInteger("Tester", "Rs232_Interface", tester.Rs232_Interface);
    iniFile->WriteInteger("Tester", "AntiSignal", tester.AntiSignal);
    iniFile->WriteInteger("Tester", "Gpib_Interface", tester.Gpib_Interface);
    iniFile->WriteInteger("Tester", "GpibAddress", tester.GpibAddress);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobTesterConfig(TIniFile* iniFile, TTesterConfig &tester)
{
    if(iniFile==NULL)
        return false;
    tester.TestMode = iniFile->ReadInteger("Tester", "TestMode", 0);
    tester.Rs232_Interface = iniFile->ReadInteger("Tester", "Rs232_Interface", 0);
    tester.AntiSignal = iniFile->ReadInteger("Tester", "AntiSignal", 0);
    tester.Gpib_Interface = iniFile->ReadInteger("Tester", "Gpib_Interface", 0);
    tester.GpibAddress = iniFile->ReadInteger("Tester", "GpibAddress", 0);
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobTesterToSetting(TTesterConfig &tester)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G11] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1350-1352) ----
    tester.TestMode = FTestIF->rgInterfaceType->ItemIndex;
    tester.Gpib_Interface = FTestIF->cbGPIBType->ItemIndex;
    tester.GpibAddress = StrToIntDef(FTestIF->edGPIBAddress->Text, 0);
#else
    (void)tester;   // deliberately unused while [G11] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobTesterToSetting(const TTesterConfig &tester)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G12] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1357-1366) ----
    FTestIF->rgInterfaceType->ItemIndex=tester.TestMode;
    FTestIF->cbRs232Type->ItemIndex=tester.TestMode;
    FTestIF->cbGPIBType->ItemIndex=tester.Gpib_Interface;
    FTestIF->edGPIBAddress->Text=IntToStr(tester.GpibAddress);
    //
    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    szDir=DataPath+S;
    FTestIF->SaveSetupFile(szDir,S);
#else
    (void)tester;   // deliberately unused while [G12] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobVacuumConfig(TIniFile* iniFile, const TVacuumConfig &vacuum)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteFloat("Vacuum", "Arm1VacuumWait", vacuum.Arm1VacuumWait);
    iniFile->WriteFloat("Vacuum", "IndexVacuumWait", vacuum.IndexVacuumWait);
    iniFile->WriteFloat("Vacuum", "OutVacuumWait", vacuum.OutVacuumWait);
    iniFile->WriteFloat("Vacuum", "TrayVacuumWait", vacuum.TrayVacuumWait);
    iniFile->WriteFloat("Vacuum", "Arm1DestroyWait", vacuum.Arm1DestroyWait);
    iniFile->WriteFloat("Vacuum", "IndexDestroyWait", vacuum.IndexDestroyWait);
    iniFile->WriteFloat("Vacuum", "OutDestroyWait", vacuum.OutDestroyWait);
    iniFile->WriteFloat("Vacuum", "TrayDestroyWait", vacuum.TrayDestroyWait);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobVacuumConfig(TIniFile* iniFile, TVacuumConfig &vacuum)
{
    if(iniFile==NULL)
        return false;
    vacuum.Arm1VacuumWait   = iniFile->ReadFloat("Vacuum", "Arm1VacuumWait", 0.0);
    vacuum.IndexVacuumWait  = iniFile->ReadFloat("Vacuum", "IndexVacuumWait", 0.0);
    vacuum.OutVacuumWait    = iniFile->ReadFloat("Vacuum", "OutVacuumWait", 0.0);
    vacuum.TrayVacuumWait   = iniFile->ReadFloat("Vacuum", "TrayVacuumWait", 0.0);
    vacuum.Arm1DestroyWait  = iniFile->ReadFloat("Vacuum", "Arm1DestroyWait", 0.0);
    vacuum.IndexDestroyWait = iniFile->ReadFloat("Vacuum", "IndexDestroyWait", 0.0);
    vacuum.OutDestroyWait   = iniFile->ReadFloat("Vacuum", "OutDestroyWait", 0.0);
    vacuum.TrayDestroyWait  = iniFile->ReadFloat("Vacuum", "TrayDestroyWait", 0.0);
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobVacuumToSetting(TVacuumConfig &vacuum)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G13] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1400-1407) ----
    vacuum.Arm1VacuumWait = StrToFloatDef(fSpeed->edInVacumCheckTime->Text, 0.0);
    vacuum.IndexVacuumWait = StrToFloatDef(fSpeed->edIndexVacumCheckTime->Text, 0.0);
    vacuum.OutVacuumWait = StrToFloatDef(fSpeed->edOutVacumCheckTime->Text, 0.0);
    vacuum.TrayVacuumWait = StrToFloatDef(fSpeed->edTrayVacumCheckTime->Text, 0.0);
    vacuum.Arm1DestroyWait = StrToFloatDef(fSpeed->edInArmAirOnTime->Text, 0.0);
    vacuum.IndexDestroyWait = StrToFloatDef(fSpeed->edIndexAirOnTime->Text, 0.0);
    vacuum.OutDestroyWait = StrToFloatDef(fSpeed->edOutArmAirOnTime->Text, 0.0);
    vacuum.TrayDestroyWait = StrToFloatDef(fSpeed->edTrayArmAirOnTime->Text, 0.0);
#else
    (void)vacuum;   // deliberately unused while [G13] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobVacuumToSetting(const TVacuumConfig &vacuum)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G14] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1412-1421) ----
    fSpeed->edInVacumCheckTime->Text=FloatToStr(vacuum.Arm1VacuumWait);
    fSpeed->edIndexVacumCheckTime->Text=FloatToStr(vacuum.IndexVacuumWait);
    fSpeed->edOutVacumCheckTime->Text=FloatToStr(vacuum.OutVacuumWait);
    fSpeed->edTrayVacumCheckTime->Text=FloatToStr(vacuum.TrayVacuumWait);
    fSpeed->edInArmAirOnTime->Text=FloatToStr(vacuum.Arm1DestroyWait);
    fSpeed->edIndexAirOnTime->Text=FloatToStr(vacuum.IndexDestroyWait);
    fSpeed->edOutArmAirOnTime->Text=FloatToStr(vacuum.OutDestroyWait);
    fSpeed->edTrayArmAirOnTime->Text=FloatToStr(vacuum.TrayDestroyWait);
    //
    fSpeed->spbSave->Click();
#else
    (void)vacuum;   // deliberately unused while [G14] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobSpeedConfig(TIniFile* iniFile, const TSpeedConfig &speed)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteInteger("Speed", "InArm2Speed", speed.InArm2Speed);
    iniFile->WriteInteger("Speed", "ShuttleSpeed", speed.ShuttleSpeed);
    iniFile->WriteInteger("Speed", "IndexSpeed", speed.IndexSpeed);
    iniFile->WriteInteger("Speed", "OutArmSpeed", speed.OutArmSpeed);
    iniFile->WriteInteger("Speed", "CatchTraySpeed", speed.CatchTraySpeed);
    iniFile->WriteInteger("Speed", "TrayLoaderSpeed", speed.TrayLoaderSpeed);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobSpeedConfig(TIniFile* iniFile, TSpeedConfig &speed)
{
    if(iniFile==NULL)
        return false;
    speed.InArm2Speed    = iniFile->ReadInteger("Speed", "InArm2Speed", 0);
    speed.ShuttleSpeed   = iniFile->ReadInteger("Speed", "ShuttleSpeed", 0);
    speed.IndexSpeed     = iniFile->ReadInteger("Speed", "IndexSpeed", 0);
    speed.OutArmSpeed    = iniFile->ReadInteger("Speed", "OutArmSpeed", 0);
    speed.CatchTraySpeed = iniFile->ReadInteger("Speed", "CatchTraySpeed", 0);
    speed.TrayLoaderSpeed= iniFile->ReadInteger("Speed", "TrayLoaderSpeed", 0);
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobSpeedToSetting(TSpeedConfig &speed)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G15] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1451-1456) ----
    speed.InArm2Speed = StrToIntDef(fSpeed->edInXYSpd->Text, 0);
    speed.ShuttleSpeed = StrToIntDef(fSpeed->edSht1Spd->Text, 0);
    speed.IndexSpeed = StrToIntDef(fSpeed->edIndexSpeed->Text, 0);
    speed.OutArmSpeed = StrToIntDef(fSpeed->edOutXSpd->Text, 0);
    speed.CatchTraySpeed = StrToIntDef(fSpeed->edTrayXSpd->Text, 0);
    speed.TrayLoaderSpeed = StrToIntDef(fSpeed->edtLoaderSpeed1->Text, 0);
#else
    (void)speed;   // deliberately unused while [G15] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobSpeedToSetting(const TSpeedConfig &speed)
{
// AI(W906-PT-W4-prodinfo) 20260808: GATE [G16] -- see file-head GATE REGISTER.
#if 0   // ---- golden text VERBATIM (golden :1461-1477) ----
    //inarm
    fSpeed->edInXYSpd->Text=IntToStr(speed.InArm2Speed);
    fSpeed->edInPitchSpd->Text=IntToStr(speed.InArm2Speed);
    //sht
    fSpeed->edSht1Spd->Text=IntToStr(speed.ShuttleSpeed);
    fSpeed->edSht2Spd->Text=IntToStr(speed.ShuttleSpeed);
    //index
    fSpeed->edIndexSpeed->Text=IntToStr(speed.IndexSpeed);
    //outarm
    fSpeed->edOutXSpd->Text=IntToStr(speed.OutArmSpeed);
    fSpeed->edOutPitchSpd->Text=IntToStr(speed.OutArmSpeed);
    //catch tray
    fSpeed->edTrayXSpd->Text=IntToStr(speed.CatchTraySpeed);
    //tray loader
    fSpeed->edtLoaderSpeed1->Text=IntToStr(speed.TrayLoaderSpeed);
    //
    fSpeed->spbSave->Click();
#else
    (void)speed;   // deliberately unused while [G16] is closed
#endif
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobAlarm3Config(TIniFile* iniFile, const TAlarm3Config &alarm3)
{
    if(iniFile==NULL)
        return;
    iniFile->WriteInteger("Alarm3", "OpenFunction", alarm3.OpenFunction);
    iniFile->WriteInteger("Alarm3", "ViewMode", alarm3.ViewMode);
    iniFile->WriteInteger("Alarm3", "SiteABinContinuouCount", alarm3.SiteABinContinuouCount);
    iniFile->WriteInteger("Alarm3", "SiteBBinContinuouCount", alarm3.SiteBBinContinuouCount);
    iniFile->WriteInteger("Alarm3", "SiteCBinContinuouCount", alarm3.SiteCBinContinuouCount);
    iniFile->WriteInteger("Alarm3", "SiteDBinContinuouCount", alarm3.SiteDBinContinuouCount);
    iniFile->WriteInteger("Alarm3", "SiteEBinContinuouCount", alarm3.SiteEBinContinuouCount);
    iniFile->WriteInteger("Alarm3", "SiteFBinContinuouCount", alarm3.SiteFBinContinuouCount);
    iniFile->WriteInteger("Alarm3", "SiteGBinContinuouCount", alarm3.SiteGBinContinuouCount);
    iniFile->WriteInteger("Alarm3", "SiteHBinContinuouCount", alarm3.SiteHBinContinuouCount);
}
//---------------------------------------------------------------------------
bool PAT_Function::GetJobAlarm3Config(TIniFile* iniFile, TAlarm3Config &alarm3)
{
    if(iniFile==NULL)
        return false;
    alarm3.OpenFunction              = iniFile->ReadInteger("Alarm3", "OpenFunction", 0);
    alarm3.ViewMode                  = iniFile->ReadInteger("Alarm3", "ViewMode", 0);
    alarm3.SiteABinContinuouCount    = iniFile->ReadInteger("Alarm3", "SiteABinContinuouCount", 0);
    alarm3.SiteBBinContinuouCount    = iniFile->ReadInteger("Alarm3", "SiteBBinContinuouCount", 0);
    alarm3.SiteCBinContinuouCount    = iniFile->ReadInteger("Alarm3", "SiteCBinContinuouCount", 0);
    alarm3.SiteDBinContinuouCount    = iniFile->ReadInteger("Alarm3", "SiteDBinContinuouCount", 0);
    alarm3.SiteEBinContinuouCount    = iniFile->ReadInteger("Alarm3", "SiteEBinContinuouCount", 0);
    alarm3.SiteFBinContinuouCount    = iniFile->ReadInteger("Alarm3", "SiteFBinContinuouCount", 0);
    alarm3.SiteGBinContinuouCount    = iniFile->ReadInteger("Alarm3", "SiteGBinContinuouCount", 0);
    alarm3.SiteHBinContinuouCount    = iniFile->ReadInteger("Alarm3", "SiteHBinContinuouCount", 0);
    return true;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJobAlarm3ToSetting(TAlarm3Config &alarm3)
{
    //
}
//---------------------------------------------------------------------------
void PAT_Function::GetJobAlarm3ToSetting(const TAlarm3Config &alarm3)
{
    //
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetPAT_Folder()
{
    AnsiString sRet=AnsiString("D:\\HT9045\\system\\PATFolder");
    FileInfo().EnsureDirectoriesExist(sRet);
    return sRet;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetJobName()
{
    AnsiString sRet=AnsiString().sprintf("%s.ini",patSetup.recipe);
    return sRet;
}
//---------------------------------------------------------------------------
AnsiString PAT_Function::GetDoublePercentStr(const double &dMolecular,const double &dDenominator)
{
    AnsiString sRet="0.00%";
    if(cMathTool().IsDoubleEqual(dDenominator,0.0,0.00999))
    {
        return sRet;
    }
    else
    {
        double dret=(ChangeToFloatNonPcnt((double)(dMolecular), (double)(dDenominator)))*100.0;
        sRet=AnsiString().sprintf("%s%",GetDoubleToStr(dret));
        return sRet;
    }
}
//---------------------------------------------------------------------------
void PAT_Function::SetStartLotTime(const double &dValue)
{
    dStartLotTime=dValue;
}
//---------------------------------------------------------------------------
void PAT_Function::SetEndLotTime(const double &dValue)
{
    dEndLotTime=dValue;
}
//---------------------------------------------------------------------------
void PAT_Function::SetJamLog(const AnsiString& sJamArea,const AnsiString& sMsg)
{
    GetTimeInfo();
    AnsiString sDate=AnsiString().sprintf("%04d/%02d/%02d", SystemYear,SystemMonth,SystemDate);
    AnsiString sTime=AnsiString().sprintf("%02d:%02d:%02d", SystemHour,SystemMin,SystemSec);
    AnsiString sPosition=sJamArea;
    //msg
    AnsiString sSaveMsg=AnsiString().sprintf("%s %s %s : %s",sDate,sTime,sPosition,sMsg);
    JamLogs.push_back(sSaveMsg);
}
//---------------------------------------------------------------------------
int PAT_Function::GetAreaByBin(int iBin)                                        //Bin1=1 ; Bin2=2
{
    int iArea=Prod.iT6PosCate[iBin];
    iArea=iArea-1;
    return iArea;//eLastSetBinCTArea
}
//---------------------------------------------------------------------------
int PAT_Function::GetBinByArea(int iArea)                                       //Bin1=1 ; Bin2=2
{
    iArea=iArea+1;
    for(int i=1;i<=15;i++)//15
    {
        if(Prod.iT6PosCate[i]==iArea)
        {
            return i;
        }
    }
    return 0;
}
//---------------------------------------------------------------------------
TDateTime PAT_Function::SecondsToDateTime(int seconds)
{
    double dnum=(double)seconds/86400.0;
    return dnum;
}
//---------------------------------------------------------------------------
TMachineTimeManager::TMachineTimeManager()
{
    totalRunning = 0;
    totalPause = 0;
    totalPauseBeforeTest=0;
    state = STATE_STOPPED;
}
//---------------------------------------------------------------------------
TMachineTimeManager::~TMachineTimeManager()
{
}
//---------------------------------------------------------------------------
void TMachineTimeManager::UpdateTime()
{
    TDateTime currentTime = Now();
    TDateTime duration = currentTime - lastSwitchTime;
    if (state == STATE_RUNNING)
    {
        totalRunning += duration;
    }
    else if (state == STATE_PAUSED)
    {
        totalPause += duration;
    }
    else if (state == STATE_PAUSED_BEFORE_TEST)
    {
        totalPauseBeforeTest += duration;
    }
    lastSwitchTime = currentTime;
}
//---------------------------------------------------------------------------
void TMachineTimeManager::StartLot()
{
    if (state != STATE_STOPPED)
    {
        EndLot();  // 如果已在運行，先結束上一個 Lot
    }
    totalRunning = 0;
    totalPause = 0;
    totalPauseBeforeTest=0;
    startTime = Now();
    lastSwitchTime = startTime;
    state = STATE_PAUSED_BEFORE_TEST;  // 預設從運作停止
}
//---------------------------------------------------------------------------
void TMachineTimeManager::Pause()
{
    if (state == STATE_RUNNING)
    {
        UpdateTime();  // 記錄運作時間
        state = STATE_PAUSED;
    }
    // 如果已在暫停或停止，無效
}
//---------------------------------------------------------------------------
void TMachineTimeManager::Resume()
{
    if (state == STATE_PAUSED ||
        state == STATE_PAUSED_BEFORE_TEST)
    {
        UpdateTime();  // 記錄暫停時間
        state = STATE_RUNNING;
    }
    // 如果已在運作或停止，無效
}
//---------------------------------------------------------------------------
void TMachineTimeManager::EndLot()
{
    if (state != STATE_STOPPED)
    {
        UpdateTime();  // 記錄最後時間
        state = STATE_STOPPED;
    }
}
//---------------------------------------------------------------------------
double TMachineTimeManager::GetTotalRunningTime() const
{
    return totalRunning;
}
//---------------------------------------------------------------------------
double TMachineTimeManager::GetTotalPauseTime() const
{
    return totalPause;
}
//---------------------------------------------------------------------------
double TMachineTimeManager::GetTotalPauseTimeBeforeTest() const
{
    return totalPauseBeforeTest;
}
//---------------------------------------------------------------------------
