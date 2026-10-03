// =============================================================================
//  forms/fHS.h  --  non-VCL facade for golden's TFormHS (HS_Function.h /
//  HS_Function.cpp) -- the "HS" (Hi-Silicon-lineage / KYEC) server-side glue
//  form: FTP/Server-socket log upload, ATC self-test and alarm-code display,
//  temperature-log header/value builders, RTM (JSCC remote monitor) protocol,
//  Power-Saving event logging, and the CheckCanRunStart_HS Start-button gate.
//
//  AI(W906-FW3-HS1) 20260827: new file, FW wave FW3-HS1 (facade wave 1 of
//  HS_Function -- read-only direction, no write path; task banner "FW3-HS1").
//  Golden source of truth: HT9011UC_Code_V3.33.906.0_20260618/HS_Function.cpp
//  (5,293 lines) + HS_Function.h (150 lines).  Decoded this wave with
//  `io.open(p, encoding='cp950')`, written out UTF-8, 0 U+FFFD (checked with
//  `data.count(chr(0xFFFD))` after decode, 20260827).
//
//  ============================================================================
//  *** SAFETY POSTURE -- READ BEFORE TOUCHING ANY GATE IN THIS FILE ***
//  ============================================================================
//  TFormHS is the handler's OUTBOUND-DATA-AND-ATC-CONTROL hub: it opens/closes
//  the ESD/RTM TServerSocket listeners and the HandlerClientSocket connection,
//  starts/stops the ATC chiller and RUN state (ATCInterfaceForm->
//  ATCChillerSwitch/SetRunATC, ATC_InterfaceForm->Run/SetSingleTemp/
//  SetAllTemp/ChannelFFCTrigger), sends the Lot-Start command to the ATC
//  subsystem (ATC_InterfaceForm->LotStart / ATCInterfaceForm->LotStart), is
//  the ONE class that raw-writes every rolling log file under
//  D:\HandlerLog\*_Log (EP/Temperature/ESD/ATCEven/ArmTestInfo/ChangeEvent/
//  GroundMan/PowerSaving/SiteContact/TFAMDLog/AutoMotive), and drives two
//  FTP paths (UpDataToServerByFTP against a generic FTP host, UpDataToServer_
//  KYEC's embedded upload).  Of golden's 66 method definitions, this wave
//  delivers 13 and gates 53 -- see WAVE SCOPE.
//
//  Every gated method is DECLARED here and DELIBERATELY NOT DEFINED in
//  forms/fHS.cpp (same idiom as forms/fIoSetView.h / forms/fContact.h).  The
//  LINKER is the interlock: the moment any future wave -- or the web write
//  path -- tries to CALL one, the build fails with `undefined reference to
//  TFormHS::<method>` naming the exact entry point.  No member here is
//  virtual, so an undefined member function costs nothing at link time until
//  somebody calls it.
//
//  UN-GATING PROTOCOL: translate the body in a wave that owns the interlock
//  story for whichever subsystem it touches (ATC control needs the ATC
//  ownership story; the *Log_HS writers need the shared-Gerneral.ini /
//  D:\HandlerLog write-path story; the socket handlers need a decision about
//  who is allowed to run a second live listener next to WebBridge's own
//  sockets).  Never as a side effect of "finishing the facade".
//
//  ============================================================================
//  DENOMINATOR -- MEASURED THIS WAVE, NOT INHERITED
//  ============================================================================
//  `python tools/census/wave_preflight.py HS_Function.cpp` (20260827):
//  golden has 66 `TFormHS::` method DEFINITIONS, 0 file-scope, 5,125 span
//  lines; span-sanity reports "census sees 66 bodies / stripped sees 66 /
//  swallowed 0" -- the denominator is clean, no comment-swallowed-brace risk.
//  THIS WAVE: 13 of 66 delivered (19.7% of golden method DEFINITIONS), 53
//  gated.  BY LINES: the 13 delivered bodies' golden spans total ~939 lines
//  (GetTempUseName 256 + CheckClockTrigger 184 + GetTitleName 109 +
//  CheckTempOffset 92 + ShowATCAlarmPosition 91 + CheckATCTempWait 83 +
//  CheckKeyPro 50 + NMFTP1Failure 25 + NMFTP1UnSupportedFunction 25 +
//  GetGPIBLogFilePath 5 + NMFTP1Error 5 + FormCreate 5 + ctor 9 (of which
//  only 5 lines are actually translated, see DEVIATION D-2)) out of golden's
//  5,125-line body (~18.3%).  CheckCanRunStart_HS alone (553 lines, GATED) is
//  more than half this wave's entire delivered span.  Quote both numbers or
//  neither.
//
//  ============================================================================
//  *** THE GLOBAL `FormHS` IS ALREADY TAKEN -- THIS FILE DOES NOT CLAIM IT ***
//  ============================================================================
//  Golden ends HS_Function.h:148 with `extern PACKAGE TFormHS *FormHS;`.
//  This header deliberately declares NO such global, because the port tree
//  already has a live, registered stand-in for that exact name:
//
//      Automation/SCK_ART_Remainder.h:625  struct W5SckArtRem_FormHSStub { ... };
//      Automation/SCK_ART_Remainder.h:629  extern W5SckArtRem_FormHSStub *FormHS;
//      Automation/SCK_ART_Remainder.cpp:382  W5SckArtRem_FormHSStub *FormHS = new W5SckArtRem_FormHSStub();
//
//  with 2 LIVE callers: Automation/SCK_ART_Remainder.cpp:2746 and :2750,
//  both `FormHS->UpDataToServerByFTP(...)`.  ⚠ TRAP: `UpDataToServerByFTP`
//  is ALSO one of golden TFormHS's own 66 methods (:2119-2169) -- SAME NAME,
//  TWO DIFFERENT OWNING CLASSES.  The stub's member is a DIFFERENT function
//  satisfying a DIFFERENT (SCK_ART) caller; it is NOT evidence that golden's
//  TFormHS::UpDataToServerByFTP has been translated, and this file's own
//  UpDataToServerByFTP declaration below is a plain GATE (FTP connect, task
//  brief's own exclusion list), unrelated to the stub.  The CLASS name
//  `TFormHS` is NOT occupied (`python tools/census/wave_preflight.py
//  HS_Function.cpp` section [3](a): "class name clear") -- only the bare
//  global pointer `FormHS` is spoken for.  Re-verified 20260827 (see
//  ABSENCE RE-RUN).
//
//  ============================================================================
//  BATCH CRITERION FOR THIS WAVE (why these 13 and not others)
//  ============================================================================
//  A method is ACTIVE here only if ALL of the following hold, each checked by
//  reading the FULL golden body (never by name):
//    1. It issues no machine action -- no MOT[]/Cylinder[]/SW[]/ATC
//       chiller-or-RUN-state/temperature-setpoint call.
//    2. It persists nothing -- no WriteIniData/WriteIniDataGeneral/
//       WriteDataToFile/SaveToFile/ForceDirectories/CreateDir/ExecZipCommand/
//       WinExec.
//    3. It sends nothing outbound -- no FTP connect (TfFTP/TNMFTP), no live
//       TServerSocket/TClientSocket traffic.
//    4. Every symbol it needs already exists in the port tree AND is
//       reachable from ht9045_forms's actual link set (vclcompat +
//       ht9045_globals + ht9045_core -- ht9045_core itself links
//       ht9045_public PUBLIC (CMakeLists.txt:420), so ht9045_public is ALSO
//       transitively reachable; ht9045_forms does NOT link ht9045_sm,
//       ht9045_motor, ht9045_io, ht9045_db, or ht9045_kyecftp).  Checked by
//       finding each global's ACTUAL DEFINITION line and cross-referencing
//       CMakeLists.txt's per-target source list, run 20260827:
//         RunInfo/Temperature/IniConfig                -> cprod.cpp,
//             ht9045_globals (CMakeLists.txt:472).
//         SystemYear/SystemMonth/CUSTOMER_CODE/ATC_SYSTEM/USE_16_HEATER/
//         iSocketBaseTempCount/BAR_CODE_INSTALL/iATC_Use_Heat_Count/
//         asHandlerVersion/asTempCtrl/UN150Read/bUT150Install/
//         iATCTempWaitTimer(_Start)/bCheckATCTemp/bEnable_KLT_Function/
//         Tri_Temp_Machine/Tri_Temperature_Max|MinDegree/TempFuseLimitType/
//         MachineTypeChoice   -> cmydef.cpp, ht9045_globals (:474).
//         GetTimeInfo/GetYesterdayInfo/GetOnlyTimeInfoByString/
//         GetDateInfoByString  -> cpublic.cpp, ht9045_globals (:473).
//         LastSet.iTemperature/Tempture_*  -> LastSet.cpp, ht9045_globals
//             (:475, moved there by GA1-B2-integrate).
//         MyTickCount   -> common.cpp, ht9045_core (:272; ht9045_forms
//             DECLARES this edge explicitly, CMakeLists.txt:730-735 --
//             "Unlike forms->sm (a documented cycle), core links only
//             vclcompat+ht9045_public and reaches nothing upward -- legal").
//         TQPF_Timer (ctor/SetSecAndOn/Off)  -> myTimer.cpp, ht9045_globals
//             (:471, "myTimer.cpp is fully translated this wave").
//         KeyPro_GetLevel   -> Public/HTKeyProShim.cpp, ht9045_public
//             (CMakeLists.txt:394) -- reachable via ht9045_core's PUBLIC
//             link to ht9045_public (see above).  This shim's OWN header
//             comment (Public/HTKeyProShim.h:33-36) explicitly anticipates
//             this exact translation: "W7 NOTE: the CheckKeyPro call site
//             ... is translated in a LATER batch. When it lands, the call
//             changes: KEYPRO_GET_LEVEL(3) -> KeyPro_GetLevel(3)" -- see
//             DEVIATION D-8.
//         RecordProcess / ShowMyMessage / ShowErrorMessage / MyDBIProcess
//             -> canary_support.cpp (RecordProcess/ShowMyMessage/
//             ShowErrorMessage) and aHotPlateSubstrate.cpp (MyDBIProcess,
//             2-arg overload), BOTH ht9045_sm -- technically UNREACHABLE by
//             the strict per-target link graph, but ALREADY an established,
//             load-bearing edge from ht9045_forms itself: forms/fLotInfo.cpp
//             (already merged, same archive) declares/calls all four
//             exactly this way (`extern void MyDBIProcess(AnsiString,
//             AnsiString);` at fLotInfo.cpp:79; `#include
//             "canary_support.h"` for the other three; ShowErrorMessage
//             called at fLotInfo.cpp:1375/1382/2698/4325).  fLotInfo.cpp's
//             OWN banner (:3134-3150, dated 20260826) states this
//             explicitly for RecordProcess: "RecordProcess in this tree
//             resolves to canary_support.cpp:113, a stdout printf
//             stand-in... 582 already-translated RecordProcess sites
//             carrying the identical exposure" -- i.e. this is a
//             pre-existing, documented, tree-wide accepted edge, not a new
//             one this wave introduces.  Reused here on that basis for all
//             four sink/message utilities; NOT re-litigated per call site.
//         FileInfo (::PathCombin / ::GetAllFileNamesInFolder)  -> DEFINED
//             ProductionInfo/FileInfo.cpp, ht9045_sm (CMakeLists.txt:2175,
//             inside the 1368-2304 ht9045_sm block).  UNREACHABLE, no
//             established forms-layer precedent for THIS symbol (unlike the
//             four sinks above) -- gates GetUploadServerByFTPPath and
//             GetGPIBLogFileName (see LINK BOUNDARY table).
//         ATCInterfaceForm / ATC_InterfaceForm / HSys  -> DEFINED
//             ATC/ATCInterface.cpp / acarry_shims.cpp / database.cpp, all
//             ht9045_sm or ht9045_db.  UNREACHABLE -- gates every method
//             touching them (see LINK BOUNDARY table).
//         IsMultiEPPressureRouteActive  -> NO PORT ANYWHERE (verified
//             20260827: `grep -rn IsMultiEPPressureRouteActive` over the
//             whole tree hits only csystem.cpp's own gate-comments, which
//             say so explicitly: "IsMultiEPPressureRouteActive() -- no
//             port ** ALARM PATH **", "IsMultiEPPressureRouteActive has NO
//             port. Verified 2026-08-09").  Gates ReadMultiEP outright --
//             this is a MISSING SYMBOL, not merely an out-of-reach one.
//    5. Every symbol's definition line is NOT inside `#if 0` (spot-checked
//       against the citations above; none were found gated).
//  Everything left is a machine action, a persistence call, an outbound
//  send, a symbol that fails criterion 4, or a live-network-socket handler
//  gated as a CATEGORY (see GATED -- SOCKET FAMILY below) even where its own
//  body looks momentarily inert.
//
//  ============================================================================
//  WAVE SCOPE -- ACTIVE (13).  Each was read line-by-line in full first.
//  ============================================================================
//    TFormHS ctor           golden :52-60   PORT SPLIT -- fields only
//                                            (bNeedUpload_N10/bHaltHandler/
//                                            bN10_3_1_Flag); the
//                                            TimerAutoBackup->Enabled=true /
//                                            TimerRTMMsg->Enabled=true lines
//                                            NOT translated (no TTimer
//                                            member this wave) -- DEVIATION D-2.
//    FormCreate              golden :62-66   ACTIVE (tHiSiRecord=new
//                                            TStringList(); own member only).
//    GetTempUseName          golden :2171-2426 ACTIVE (pure string builder
//                                            over config/enum state; zero
//                                            writes, zero ATC-subsystem
//                                            deref).  NAME RESERVED (gated-16
//                                            list: csystem.cpp:29440,:29461).
//    ShowATCAlarmPosition    golden :3567-3657 ACTIVE (pure iCode->position
//                                            classifier; touches only
//                                            MMATC_* extern-const-int
//                                            constants, cmydef.cpp,
//                                            ht9045_globals).
//    CheckClockTrigger       golden :4234-4417 ACTIVE (pure edge-detector
//                                            over GetOnlyTimeInfoByString()/
//                                            StrToInt; all state is its own
//                                            function-local statics).
//    GetTitleName            golden :4635-4743 ACTIVE (pure string builder;
//                                            calls this class's own
//                                            GetTempUseName for the Temp
//                                            case, translated below).
//    NMFTP1Failure           golden :3897-3921 ACTIVE (pure TCmdType switch
//                                            -> own sFTPErrMsg/bFTPError
//                                            fields + MyDBIProcess; no FTP
//                                            connect anywhere in the body).
//    NMFTP1UnSupportedFunction golden :3923-3947 ACTIVE (ditto).
//    NMFTP1Error             golden :3949-3953 ACTIVE (ShowMyMessage only).
//    CheckTempOffset         golden :3958-4049 ACTIVE (pure threshold
//                                            comparison + own statics;
//                                            ShowErrorMessage only in the
//                                            bSendAlarm branch).  NAME
//                                            RESERVED (gated-16 list:
//                                            bthermo.cpp:2628,:2632,:3941...).
//    CheckATCTempWait        golden :4745-4827 ACTIVE (pure UN150Read[]
//                                            threshold comparison + own
//                                            statics; RecordProcess only).
//    CheckKeyPro             golden :4898-4947 ACTIVE (KEYPRO_GET_LEVEL(3)
//                                            ADAPTED to KeyPro_GetLevel(3) --
//                                            DEVIATION D-8; graceful-offline
//                                            USB-dongle READ, no machine
//                                            action).
//    GetGPIBLogFilePath      golden :5027-5031 ACTIVE (pure sprintf over
//                                            SystemYear/SystemMonth).
//
//  ============================================================================
//  GATED -- CATEGORY A: EXPLICITLY EXCLUDED BY TASK BRIEF (11)
//  ============================================================================
//  Named in the dispatching agent's own exclusion list before this wave
//  opened a single golden line; content-verified on read, not merely quoted.
//   TimerAutoBackupTimer    golden :84-511    428L; FTP/log-upload timer.
//   RecordEPLog_HS          golden :897-1132  236L; EP log writer.
//   RecordESDLog_HS         golden :1247-1421 175L; ESD log writer + CopyFile.
//   RecordArmTestInfoLog_HS golden :1444-1483 WriteDataToFile.
//   RecordChangeLogByLot    golden :1485-1522 (name-excluded; not read this
//                                              wave -- see WHAT REMAINS).
//   SaveCloseOpenSiteEven   golden :3706-3744 NewRecordProcess site-log write.
//   UpDataToServer_KYEC     golden :1524-2043 520L; embedded server upload.
//   Check_FileFolderByFTP   golden :5053-5211 159L; TNMFTP object, live FTP.
//   UpDataToServerByFTP     golden :2119-2169 TfFTP::Connect/Upload/Close;
//                                              ALSO the live-name-collision
//                                              function (see FormHS note).
//   LoaderBufPreAlarm       golden :3469-3529 alarm display + Sen[]/MOT[]
//                                              (also LINK BOUNDARY).
//   FixTrayPreAlarm         golden :3530-3566 alarm display + MOT[]
//                                              (also LINK BOUNDARY).
//
//  ============================================================================
//  GATED -- CATEGORY B: WRITES A FILE OR DIRECTORY (21)
//  ============================================================================
//  WriteDataToFile / WriteIniData / WriteIniDataGeneral / SaveToFile /
//  ForceDirectories / CreateDir / ExecZipCommand, confirmed by reading the
//  body (not the name) in every case.
//   CalculateUploadToHostIntervalTimeFunction golden :513-582  calls
//       RecordESDLog_HS/RecordLog_HS (both write) + EventReport (SECS out).
//   RecordLog_HS            golden :584-645   calls RecordTemperatureLog_HS/
//       RecordESDLog_HS/RecordGroundManLog_HS/RecordATCEvenLog_HS (all
//       write) or UpDataToServer_KYEC (FTP, excluded).
//   ClearLastFileLogName_HS golden :647-706   tHiSiRecord->SaveToFile(...).
//   GetLastFileLogName_HS   golden :708-894   WriteIniData x6 branches.
//       NAME RESERVED (gated-16: atester.cpp:1604,:1861).
//   RecordTemperatureLog_HS golden :1206-1245 WriteDataToFile x2.
//   RecordATCEvenLog_HS     golden :1423-1442 Check_RecordFolder (mkdir) +
//       RecordAutoMotive (write), transitively.
//   CheckSetupNamelist_Hisi golden :2779-2811 WriteDataToFile if list file
//       missing.  NAME RESERVED (gated-16: csystem.cpp:1124,:1175).
//   AutoConnectATCNetwork   golden :3745-3776 TestList->SaveToFile(
//       "...ATC_NetworkConnect.bat") + ExecZipCommand (spawns a process) +
//       HSys.asATCSYSTEMIP (also LINK BOUNDARY).
//   RecordContact_SLTLog_HS golden :4056-4114 WriteDataToFile x2 +
//       MyForceDirectories.
//   RecordPowerSaveEvenLog_HS golden :4561-4583 WriteDataToFile.
//   RecordAutoMotive        golden :4587-4603 WriteDataToFile x2 (calls the
//       ACTIVE GetTitleName for the header row, but writes it to disk).
//   BackupGPIBLog           golden :4605-4633 TestList->SaveToFile(
//       "...GPIBLOG.bat") + ExecZipCommand.
//   RecordParameter_TFAMDLog golden :4949-5025 CreateDir + WriteDataToFile.
//   CheckMTKFileName        golden :5213-5244 WriteIniDataGeneral("Version",
//       "Ver", ...) -- writes the SHARED system\Gerneral.ini (see the
//       standing project-wide warning on common.cpp:89's asGeneralPath).
//   Check_RecordFolder      golden :5246-5263 ForceDirectories (mkdir),
//       called by 6+ of the writers above -- gating it independently also
//       stops any future accidental reuse from becoming a silent mkdir.
//   RecordGroundManLog_HS   golden :5265-5292 WriteDataToFile x2.
//   RecordRunState          golden :4186-4232 fMain->slRecordRunState->
//       AddTextWithDateTime(...) -- a Record*-named log sink; not proven
//       inert this wave (see WHAT REMAINS).
//  (HandlerClientSocketRead is cross-listed under CATEGORY F; it also calls
//   the gated RecordPowerSaveEvenLog_HS, counted once there.)
//
//  ============================================================================
//  GATED -- CATEGORY C: LINK BOUNDARY (ht9045_forms cannot reach ht9045_sm) (7)
//  ============================================================================
//   GetTempUseNamevalue     golden :2428-2777 349L; fLotInfo->ATCPtr[i]/
//       palATCWorkingTemp/pl_ATCChillerSV Captions are fine (ht9045_forms),
//       but ATCInterfaceForm->btnATCPower->Caption and
//       ATC_InterfaceForm->GetRunState() are ht9045_sm (ATC/ATCInterface.cpp,
//       acarry_shims.cpp) -- UNREACHABLE.
//   ShowATCSelfTestSatus    golden :3401-3468 ATC_InterfaceForm->
//       GetRunSelfTestResult/Status, ATCInterfaceForm-> ditto.
//   ShowATCAlarmMessage     golden :3659-3705 ATC_InterfaceForm->
//       GetAlarmMsg() + HSys.mapATCAlarmCode (database.cpp, ht9045_db).
//   GetUploadServerByFTPPath golden :2048-2117 FileInfo().PathCombin (x4) --
//       ProductionInfo/FileInfo.cpp, ht9045_sm.  Does NOT itself open an
//       FTP connection (verified by full read) -- gated purely on this
//       symbol, not re-classified under Category A.  NAME RESERVED
//       (gated-16: cConfiguration.cpp:5714).
//   GetGPIBLogFileName      golden :5034-5051 FileInfo().
//       GetAllFileNamesInFolder -- same ht9045_sm symbol.
//   ATC_FFCTrigger          golden :4478-4492 ATC_InterfaceForm->
//       ChannelFFCTrigger (also a machine action -- Category E).
//   TESTTEMPSETTING         golden :4829-4895 ATC_InterfaceForm->
//       SetSingleTemp/SetAllTemp (also a machine action -- Category E).
//
//  ============================================================================
//  GATED -- CATEGORY D: MISSING SYMBOL, NO PORT ANYWHERE (1)
//  ============================================================================
//   ReadMultiEP             golden :1144-1204 needs
//       IsMultiEPPressureRouteActive(), which csystem.cpp's own gate
//       comments state has "NO port" tree-wide (verified 20260827).  Not a
//       link-boundary issue -- the function does not exist to link against.
//       Note also the body reads an uninitialised `WORD buffer[36]` (the
//       real ADAMTCP_ReadReg call is commented out in GOLDEN ITSELF, "V896
//       left commented; enable after hardware verify") -- a pre-existing
//       golden defect this port would inherit verbatim if ever translated,
//       recorded here in case a future wave un-gates this on
//       IsMultiEPPressureRouteActive alone without re-reading the rest.
//
//  ============================================================================
//  GATED -- CATEGORY E: MACHINE ACTION / MODE-SWITCH SAFETY (6)
//  ============================================================================
//   CheckCanRunStart_HS     golden :2813-3365 553L, the single largest
//       golden body in this file.  ATCInterfaceForm/ATC_InterfaceForm
//       dereferenced within the FIRST 15 lines of the body (ATC_
//       InterfaceForm->GetRunSelfTestStatus at :2828); drives
//       ATCInterfaceForm->ATCChillerSwitch(true)/SetRunATC(true) and
//       ATC_InterfaceForm->Run()/LotStart(asATCEvenLotID) -- i.e. this is
//       the function that turns the ATC chiller and RUN state ON when the
//       operator presses Start.  Gated on SIZE + LINK BOUNDARY + MACHINE
//       ACTION independently; the remaining ~350 lines were not read
//       exhaustively once these three grounds were each independently
//       sufficient (see WHAT REMAINS) -- same "gate on size/risk before
//       full read" precedent as forms/fContact.h's FormShow / forms/
//       fIoSetView.h's own FormShow (S-03).
//   CheckSetupFileData      golden :3367-3400 sets `InitialOK=true;` (the
//       tree-wide "has initialisation completed" gate TimerAutoBackupTimer
//       itself checks first) and `fMain->iHasChangeFile=2;`.  Neither is a
//       file write, but both are global READINESS/MODE state a Start-path
//       function depends on; not translated as a "safe read" without a
//       wave that owns that story.
//   TimerRTMMsgTimer        golden :4163-4183 reads bHaltHandler and, when
//       true, sets `SystemStart=false;` -- i.e. this timer is HOW the RTM
//       "STOP" command (parsed elsewhere, TimerAutoBackupTimer's own gated
//       RTM branch) actually stops the handler.  Remote-control-adjacent;
//       gated even though its own body issues no direct hardware call.
//   CheckEPRange            golden :4428-4476 writes
//       `IniConfig.iD26EPEncoderRange` -- the EP/contact-force encoder
//       range used elsewhere to interpret force readings (safety-adjacent
//       config, not itself a file) -- and `fConfiguration->edD26->Text`
//       (fConfiguration widget reachability not verified this wave).  NAME
//       RESERVED (gated-16: cinitial.cpp:9144).
//   ATC_FFCTrigger          golden :4478-4492 (cross-listed under Category
//       C; the machine-action ground is independently sufficient).
//   TESTTEMPSETTING         golden :4829-4895 (cross-listed under Category
//       C; directly SETS ATC temperature setpoints per-arm/per-channel --
//       the clearest machine action in this file after CheckCanRunStart_HS).
//
//  ============================================================================
//  GATED -- CATEGORY F: SOCKET FAMILY, GATED AS A CATEGORY (10)
//  ============================================================================
//  vclcompat DOES port TServerSocket (vclcompat/ServerSocket.h) and
//  TCustomWinSocket/TClientSocket/TErrorEvent (vclcompat/ClientSocket.h), so
//  these are not blocked on a missing type.  Gated instead because they are
//  LIVE, remotely-reachable network I/O for THREE separate protocols (ESD
//  sensor console, JSCC RTM remote-stop/resume, and the handler's own
//  companion-app HandlerClientSocket), none of whose interlock/ownership
//  story this read-only wave is positioned to decide -- translating the
//  plumbing (accept/error/read) while its actual command interpretation
//  stays gated elsewhere in this same class (TimerAutoBackupTimer's RTM
//  branch, HandlerClientSocketRead's own Power-Saving/SECS effects) would
//  leave an inconsistent, easy-to-misread partial contract.  None of these
//  10 names appear in the gated-16 reserved-name list, so gating the whole
//  family costs nothing in future-un-gate compatibility.
//   ESDServerSocketClientConnect    golden :3780-3787
//   ESDServerSocketClientError      golden :3791-3804
//   ESDServerSocketClientRead       golden :3808-3874  parses live ESD/decay
//       sensor data into fLotInfo Captions + sESDLogData.
//   RTMServerSocketClientConnect    golden :4116-4125
//   RTMServerSocketClientDisconnect golden :4127-4131
//   RTMServerSocketClientError      golden :4133-4146
//   RTMServerSocketClientRead       golden :4148-4161  queues raw remote
//       text into fMain->RTMCommand (consumed by the gated RTM branch of
//       TimerAutoBackupTimer, which can set bHaltHandler).
//   HandlerClientSocketConnect      golden :4419-4426  body itself only
//       flips fMain->iResetCurrent/bPowersavingStart (no Socket-> touch at
//       all) -- kept gated for category consistency with its Error/Read
//       siblings rather than special-cased as the one "safe" member.
//   HandlerClientSocketError        golden :4494-4506
//   HandlerClientSocketRead         golden :4508-4557  parses "OFF"/other
//       into RunInfo.bSecsGem*EnergySavingState + EventReport (SECS out) +
//       calls the gated RecordPowerSaveEvenLog_HS (file write).
//
//  ============================================================================
//  GATED -- CATEGORY G: NULL-GLOBAL HAZARD (1)
//  ============================================================================
//   CheckIndependentPassWord golden :3878-3895 dereferences the GLOBAL
//       `fPassword` (`->ShowModal()`, `->edUserName->Text`,
//       `->edPassword->Text`).  forms/fPassword.h's OWN "NULL-GLOBAL NOTE"
//       (fPassword.h:160-170) states: "fQwertyKey/fPassword stay NULL until
//       a wiring wave constructs them ... no handler is wired today, so no
//       runtime path reaches the derefs."  Translating this function ACTIVE
//       would be the FIRST caller to actually dereference that null global
//       in a live, callable (if unwired) function -- exactly the
//       null-global-deref shape task rule 4 warns about (88-SEGFAULT
//       precedent).  EPuser/EPPass (cmydef.cpp, reachable) are not the
//       blocker; fPassword's construction state is.
//
//  ============================================================================
//  GATED -- CATEGORY H: EXTERNAL PROCESS / OS SIDE EFFECT (1)
//  ============================================================================
//   CloseWindowsKeyboard    golden :4051-4054 `WinExec("taskkill.exe /im
//       \"OSK.exe\" /f", SW_HIDE);` -- kills a real OS process.  Not a file
//       write and not machine hardware, but a real, non-reversible-from-
//       this-process side effect on the host OS; out of scope for a
//       read-only translation wave.
//
//  ============================================================================
//  GATED -- CATEGORY I: FORM DESTRUCTOR, UNREACHABLE SYMBOL (1)
//  ============================================================================
//   FormDestroy             golden :68-82 clears/deletes tHiSiRecord (both
//       fine, own member) but ALSO calls `LogSoftwareOffTime("TFormHS,
//       FormDestroy")`.  LogSoftwareOffTime IS a trivial offline no-op
//       (`void LogSoftwareOffTime(AnsiString) {}`) -- but its ONE definition
//       tree-wide is acarry_shims.cpp:255, inside the ht9045_sm block
//       (CMakeLists.txt:1368-2304).  Dropping the call to keep the rest of
//       FormDestroy ACTIVE would be exactly the "silently drop a golden
//       statement" degraded-translation task rule forbids; gated whole
//       instead.  (bFTPError/sFTPErrMsg/tHiSiRecord fields are still
//       declared below for the OTHER active methods that need them.)
//
//  ============================================================================
//  GOLDEN NOTES (faithful; recorded, NOT "fixed")
//  ============================================================================
//  (G-1) CheckClockTrigger's 30/90/120-minute branches (:4329-4415) compute
//      TWO independent edge-detectors per clock (a "_0min"/"_1min" pair,
//      offset by a magic literal -- +3000 for the 30-min case, +1300 for
//      90-min) whose exact arithmetic derivation is not self-evident from
//      the source; translated verbatim, not re-derived.
//  (G-2) GetTempUseName's ATC_SYSTEM==eNewATCSystem arm (:2375-2378) has an
//      empty `else { }` for iATC_Use_Heat_Count>8 (32-site) -- "預留 32Site
//      ATC Log", i.e. deliberately not yet implemented in golden itself.
//      Verbatim.
//  (G-3) CheckTempOffset's threshold ladder (:3966-4015) is a long
//      if/else-if chain with NO shared unit or scale across branches
//      (tcHeatGun*: literal 250; tcChamber: literal 165; Tri_Temp_Machine:
//      Tri_Temperature_Max/MinDegree; CC_KYEC_LEE/CC_PTI: literal 175;
//      Tri_Temp_Machine==1 && Type_HT1032 (UNREACHABLE -- the earlier
//      `else if(Tri_Temp_Machine==1)` arm already claims every
//      Tri_Temp_Machine==1 case, so this arm's own condition can never be
//      true; recorded as a defect, not fixed); else: TempFuseLimitType.
//      Translated verbatim including the dead arm.
//  (G-4) CheckKeyPro's `bStart` parameter is read ONLY in the
//      `SystemStart==false` branch (:4922-4928) -- when the handler is
//      already running, `bStart` is silently ignored and the periodic
//      60-second re-check path is used instead.  Verbatim.
//
//  ============================================================================
//  DEVIATIONS (port-only; each with why)
//  ============================================================================
//  (D-1) `__fastcall` dropped from every signature -- tree-wide convention.
//  (D-2) CTOR carries FIELDS ONLY (task rule 5).  Golden's ctor body also
//      sets `TimerAutoBackup->Enabled=true;` / `TimerRTMMsg->Enabled=true;`
//      -- NOT translated: no TTimer member exists on this facade (the two
//      timer-driven methods, TimerAutoBackupTimer and TimerRTMMsgTimer, are
//      both GATED, so a TTimer member would have no ACTIVE consumer this
//      wave).  Add the TTimer members in the wave that un-gates either
//      timer.
//  (D-3) `TComponent* Owner` / `: TForm(Owner)` base-init dropped -- no
//      facade-side TForm base (matches forms/fIoSetView.h D-4: no delivered
//      method needs one, and adding one only to satisfy an unused base pulls
//      in nothing useful).  `TFormHS` has NO base class here.
//  (D-4) golden `private:` members (bFTPError/sFTPErrMsg/bNeedUpload_N10/
//      bN10_3_1_Flag) collapsed to `public:` -- established convention
//      (forms/fPassword.h D-4, forms/fTemp_Set.h precedent it cites).
//  (D-5) `TObject *Sender` / event-glue parameters kept on every delivered
//      handler signature that golden gives one (FormCreate), even though
//      unused in the translated body -- golden signature, vclcompat::TObject
//      exists, and no handler is wired to anything (task rule 5).
//  (D-6) `String` (golden's VCL alias, `typedef vclcompat::AnsiString
//      String;` at vclcompat/vcl_compat.h:340) spelled `AnsiString`
//      throughout the translated bodies -- identical type, matches the
//      dominant spelling used elsewhere in this port.
//  (D-7) GATED declarations use `TObject*` / `TCustomWinSocket*` /
//      `TErrorEvent` / `TComponent*` / `WORD` / `TCmdType` exactly as golden
//      declares them (all five are real ports: vclcompat/vcl_compat.h,
//      vclcompat/ClientSocket.h, vclcompat/Comm.h, KYECFTP/MiniFtpEngine.h)
//      so a future un-gate's signature matches without re-deriving types.
//  (D-8) CheckKeyPro: `#include "HTKeyPro.h"` (golden, a Borland
//      `__declspec(dllimport)` driver header not portable to MinGW) ->
//      `#include "Public/HTKeyProShim.h"`, and the call
//      `KEYPRO_GET_LEVEL(3)` -> `KeyPro_GetLevel(3)`.  This is NOT a
//      discretionary rewrite: HTKeyProShim.h's own banner (Public/
//      HTKeyProShim.h:33-36) states this exact substitution is the intended
//      migration path for THIS SOLE call site, prepared by an earlier wave
//      in anticipation of CheckKeyPro's translation.  Semantics preserved:
//      KeyPro_GetLevel resolves the real KeyProDLL.dll at runtime via
//      LoadLibrary/GetProcAddress when present, and returns 1 ("valid
//      dongle") when absent -- the same "offline stays enabled" shape as
//      every other hardware-optional read in this port.
//  (D-9) CheckTempOffset: golden `abs(dTime-dAlarmTime)` (both double) ->
//      `std::fabs(...)`.  MinGW libstdc++ makes bare `abs(double)` an
//      AMBIGUOUS overload (int/long/long long candidates via <cstdlib>'s
//      <windows.h>-included C abs(int) plus <cstdlib>'s std::abs(long)/
//      std::abs(long long), none an exact double match) -- a genuine
//      compiler rejection (`g++ -std=c++17 -fsyntax-only`, 20260827), not a
//      style choice.  `std::fabs` is the unambiguous double overload with
//      identical semantics; `#include <cmath>` added.
//
//  ============================================================================
//  ABSENCE RE-RUN AT WAVE CLOSE -- 20260827
//  ============================================================================
//  Absence claims go stale (docs/KNOWLEDGE.md) and this repo runs parallel
//  waves.  Re-run immediately before writing this banner's final text:
//    global `FormHS` (outside this file) -- still exactly the
//        W5SckArtRem_FormHSStub owner (Automation/SCK_ART_Remainder.h:625/
//        :629, .cpp:382), still 2 live callers, both `->UpDataToServerByFTP`.
//    `class TFormHS` (outside this file) -- 0 hits.
//    `IsMultiEPPressureRouteActive` definition (any .cpp) -- still 0; only
//        the csystem.cpp gate-comments describing its absence.
//    `class FileInfo` definition -- still exactly 1
//        (ProductionInfo/FileInfo.cpp:314), still inside the ht9045_sm
//        add_library block (1368-2304).
//    `fPassword =` (an actual construction, not just the bare declaration)
//        -- still 0 hits tree-wide; forms/fPassword.cpp:29 is still a bare
//        `TfPassword *fPassword;` with no initializer.
//  Method: same grep-based re-check style as forms/fIoSetView.h's own
//  ABSENCE RE-RUN, run against the corpus excluding build*/.git/.svn/
//  .pti_frames/web.
//
//  ============================================================================
//  SYMBOLS THIS TU NEEDS (for the CMakeLists landing decision -- NOT made
//  here; this wave may not touch CMakeLists.txt)
//  ============================================================================
//    vclcompat      -- AnsiString/TObject (vcl_compat.h), TComponent
//                      (Comm.h), TCustomWinSocket/TErrorEvent
//                      (ClientSocket.h) -- the last three appear ONLY in
//                      GATED signatures, no ACTIVE body needs them.
//    ht9045_globals -- RunInfo/Temperature/IniConfig (cprod.cpp);
//                      SystemYear/SystemMonth/CUSTOMER_CODE/ATC_SYSTEM/
//                      USE_16_HEATER/iSocketBaseTempCount/BAR_CODE_INSTALL/
//                      iATC_Use_Heat_Count/asHandlerVersion/asTempCtrl/
//                      UN150Read/bUT150Install/iATCTempWaitTimer(_Start)/
//                      bCheckATCTemp/bEnable_KLT_Function/Tri_Temp_Machine/
//                      Tri_Temperature_Max|MinDegree/TempFuseLimitType/
//                      MachineTypeChoice (cmydef.cpp); GetTimeInfo/
//                      GetYesterdayInfo/GetOnlyTimeInfoByString/
//                      GetDateInfoByString (cpublic.cpp); LastSet
//                      (LastSet.cpp); TQPF_Timer (myTimer.cpp).
//    ht9045_core    -- MyTickCount (common.cpp) -- edge already declared at
//                      CMakeLists.txt:730-735 for a different consumer
//                      (forms/fSetup.cpp), reused here.
//    ht9045_public  -- KeyPro_GetLevel (Public/HTKeyProShim.cpp), reachable
//                      transitively via ht9045_core's PUBLIC link
//                      (CMakeLists.txt:420) -- introduces NO NEW archive
//                      edge at the ht9045_forms level (the edge already
//                      exists one hop down, in ht9045_core).
//    (established, pre-existing edge, reused not introduced) ht9045_sm --
//                      RecordProcess/ShowMyMessage/ShowErrorMessage
//                      (canary_support.cpp) + MyDBIProcess
//                      (aHotPlateSubstrate.cpp) -- see BATCH CRITERION 4's
//                      citation of forms/fLotInfo.cpp's own 20260826 banner
//                      for why this specific quartet is treated as accepted
//                      tree-wide convention rather than a new violation.
//    KYECFTP/MiniFtpEngine.h -- `enum TCmdType` ONLY (header-only; the
//                      surrounding MiniFtpEngine.cpp implementation lives in
//                      ht9045_kyecftp, CMakeLists.txt:2419, NOT linked and
//                      NOT needed -- no function from that TU is called).
//  Expected landing: ht9045_forms (same layer as forms/fContact.cpp,
//  forms/fIoSetView.cpp).  Introduces ONE new *declared* edge
//  (ht9045_core->ht9045_public was already declared for a different file;
//  this file is simply another consumer of it) and reuses, rather than
//  adds, the ht9045_sm RecordProcess/ShowMyMessage/ShowErrorMessage/
//  MyDBIProcess edge already exercised by forms/fLotInfo.cpp.
//
//  ============================================================================
//  WHAT REMAINS / NOT DONE THIS WAVE (explicit, so scope-narrowing is not
//  silently absorbed -- task rule 9)
//  ============================================================================
//  - CheckCanRunStart_HS (553L) was read only through roughly its first 200
//    lines before three independent gate grounds (size, link boundary,
//    machine action) were each already sufficient; the remaining ~350 lines
//    are UNCATALOGUED beyond "more of the same ATC/mode-switch shape" --
//    their machine-action surface, if any beyond what is already cited, has
//    NOT been enumerated.
//  - UpDataToServer_KYEC (520L) and RecordEPLog_HS (236L) were NOT read
//    body-by-body this wave (both were already named in the task's own
//    exclusion list before work started); their gate reasons above are
//    taken from that list plus the surrounding context read incidentally
//    (e.g. RecordEPLog_HS's opening lines, seen while reading
//    ClearLastFileLogName_HS's neighbourhood), not from a full read.
//  - RecordChangeLogByLot (golden :1485-1522) was NOT read at all this wave
//    -- name-excluded per the task brief before this file was opened. Its
//    GATE entry above is taken on that basis alone.
//  - RecordRunState's AddTextWithDateTime target (fMain->slRecordRunState)
//    was not traced to confirm whether it is itself persisted to disk
//    elsewhere or stays in-memory; gated on the conservative reading of its
//    own name and the Record* family pattern, not on a confirmed write.
//  - fConfiguration's widget surface (edD26, referenced by the gated
//    CheckEPRange) was not checked for existence/reachability -- CheckEPRange
//    is gated on the IniConfig.iD26EPEncoderRange safety-adjacent-config
//    ground regardless, so this was not pursued further.
//  - No build/ctest was run by this file's author beyond `g++ -std=c++17
//    -fsyntax-only` on both forms/fHS.h and forms/fHS.cpp and a full `-c`
//    compile to a .o with `nm --undefined-only -C` / `nm --defined-only -C`
//    inspection -- NOT a cmake configure/build (this wave's task rules
//    explicitly reserve CMakeLists.txt registration for the main loop).
//  - The golden __published widget block (TimerAutoBackup/pnlSlfeTestRun/
//    Label1-5/pl_ATCSelfTestSatus_*/ESDServerSocket/RTMServerSocket/
//    TimerRTMMsg/HandlerClientSocket) is NOT declared anywhere on this
//    facade -- zero ACTIVE method this wave touches a widget, so none were
//    added (see DEVIATION D-2 for the two TTimer members specifically).
// =============================================================================
#ifndef FORMS_FHS_H
#define FORMS_FHS_H

#include "vclcompat/vcl_compat.h"    // AnsiString / TObject / WORD
#include "vclcompat/Comm.h"          // TComponent
#include "vclcompat/ClientSocket.h"  // TCustomWinSocket / TErrorEvent
#include "KYECFTP/MiniFtpEngine.h"   // enum TCmdType (header-only; see banner)

#include <vector>

// ===========================================================================
//  eRecordType -- golden HS_Function.h:23-32, reproduced verbatim (this
//  facade's own copy; golden's enum is a plain top-level declaration, not a
//  TFormHS member, so it is not "occupied" by anything else in the tree --
//  re-verified 20260827, `enum eRecordType` -- 0 hits outside this file).
// ===========================================================================
enum eRecordType
{
    eReocrd_EP        =0,
    eReocrd_Temp      =1,
    eReocrd_ESD       =2,
    eReocrd_ATCEven   =3,
    eReocrd_Space     =4,
    eReocrd_TestArm   =5,
    eReocrd_ChangEven =6,
    eRecordTypeTotal
};

// ===========================================================================
//  TFormHS -- non-VCL facade (golden HS_Function.h:33-146,
//  `class TFormHS : public TForm`; NO base class here, see DEVIATION D-3).
// ===========================================================================
class TFormHS
{
public:
    TFormHS();

    // -- golden private: fields, collapsed to public (DEVIATION D-4) -------
    bool bFTPError = false;                 // golden :79 -- NMFTP1* fields
    AnsiString sFTPErrMsg;                  // golden :80
    bool bNeedUpload_N10 = false;           // golden :81 -- ctor-only this wave
    bool bN10_3_1_Flag = false;             // golden :82 -- ctor-only this wave

    // -- golden public: fields this wave's ACTIVE methods touch ------------
    bool bHaltHandler = false;              // golden :120
    TStringList *tHiSiRecord = nullptr;     // golden :100 (FormCreate owns it)

    // -- ACTIVE this wave (defined in forms/fHS.cpp) ------------------------
    void FormCreate(TObject *Sender);                                          // golden :62-66
    AnsiString GetTempUseName(int source);                                     // golden :2171-2426
    int ShowATCAlarmPosition(int iCode);                                       // golden :3567-3657
    bool CheckClockTrigger(int iClock);                                        // golden :4234-4417
    void GetTitleName(AnsiString &S, eRecordType iType);                       // golden :4635-4743
    void NMFTP1Failure(bool &Handled, TCmdType Trans_Type);                    // golden :3897-3921
    void NMFTP1UnSupportedFunction(TCmdType Trans_Type);                       // golden :3923-3947
    void NMFTP1Error(TComponent *Sender, WORD Errno, AnsiString Errmsg);       // golden :3949-3953
    bool CheckTempOffset(int Addr, double SetTemp, bool bSendAlarm = false);   // golden :3958-4049
    bool CheckATCTempWait();                                                   // golden :4745-4827
    bool CheckKeyPro(bool bStart = false);                                     // golden :4898-4947 (DEVIATION D-8)
    AnsiString GetGPIBLogFilePath();                                           // golden :5027-5031

    // -- GATED this wave -- declared, NOT defined.  Calling one of these is
    //    a LINK ERROR by design (see safety posture banner above). ---------
    void FormDestroy(TObject *Sender);                                                     // golden :68-82    GATE (Cat I)
    void TimerAutoBackupTimer(TObject *Sender);                                             // golden :84-511   GATE (Cat A)
    void CalculateUploadToHostIntervalTimeFunction();                                       // golden :513-582  GATE (Cat B)
    void RecordLog_HS(bool bAnyWaySend = false, bool bRealTime = false);                    // golden :584-645  GATE (Cat B)
    void ClearLastFileLogName_HS(int iType);                                                // golden :647-706  GATE (Cat B)
    AnsiString GetLastFileLogName_HS(int iType);                                            // golden :708-894  GATE (Cat B)
    void RecordEPLog_HS(AnsiString aFileName);                                              // golden :897-1132 GATE (Cat A)
    bool ReadMultiEP();                                                                     // golden :1144-1204 GATE (Cat D)
    void RecordTemperatureLog_HS(AnsiString aFileName);                                     // golden :1206-1245 GATE (Cat B)
    void RecordESDLog_HS(AnsiString aFileName);                                             // golden :1247-1421 GATE (Cat A)
    void RecordATCEvenLog_HS(AnsiString aFileName);                                         // golden :1423-1442 GATE (Cat B)
    void RecordArmTestInfoLog_HS(AnsiString aFileName);                                     // golden :1444-1483 GATE (Cat A)
    void RecordChangeLogByLot(AnsiString S, AnsiString S2);                                 // golden :1485-1522 GATE (Cat A)
    int UpDataToServer_KYEC(int iLog, bool bForceSend = false);                             // golden :1524-2043 GATE (Cat A)
    AnsiString GetUploadServerByFTPPath(AnsiString asFileType, bool bDailyReport = false);  // golden :2048-2117 GATE (Cat C)
    int UpDataToServerByFTP(AnsiString asDirPath, AnsiString sFileName,
                             AnsiString asFileType, bool bDailyReport = false);              // golden :2119-2169 GATE (Cat A)
    AnsiString GetTempUseNamevalue(int source);                                             // golden :2428-2777 GATE (Cat C)
    bool CheckSetupNamelist_Hisi();                                                         // golden :2779-2811 GATE (Cat B)
    bool CheckCanRunStart_HS();                                                             // golden :2813-3365 GATE (Cat E)
    bool CheckSetupFileData(AnsiString strSetupFile);                                       // golden :3367-3400 GATE (Cat E)
    void ShowATCSelfTestSatus(bool bFlash, bool bHasAlarm);                                 // golden :3401-3468 GATE (Cat C)
    void LoaderBufPreAlarm();                                                               // golden :3469-3529 GATE (Cat A)
    void FixTrayPreAlarm();                                                                 // golden :3530-3566 GATE (Cat A)
    void ShowATCAlarmMessage();                                                             // golden :3659-3705 GATE (Cat C)
    void SaveCloseOpenSiteEven(int iZ, int iRow, int iCol, bool bUse);                      // golden :3706-3744 GATE (Cat A)
    void AutoConnectATCNetwork();                                                           // golden :3745-3776 GATE (Cat B)
    void ESDServerSocketClientConnect(TObject *Sender, TCustomWinSocket *Socket);           // golden :3780-3787 GATE (Cat F)
    void ESDServerSocketClientError(TObject *Sender, TCustomWinSocket *Socket,
                                     TErrorEvent ErrorEvent, int &ErrorCode);                // golden :3791-3804 GATE (Cat F)
    void ESDServerSocketClientRead(TObject *Sender, TCustomWinSocket *Socket);              // golden :3808-3874 GATE (Cat F)
    bool CheckIndependentPassWord();                                                        // golden :3878-3895 GATE (Cat G)
    void CloseWindowsKeyboard();                                                            // golden :4051-4054 GATE (Cat H)
    void RecordContact_SLTLog_HS();                                                         // golden :4056-4114 GATE (Cat B)
    void RTMServerSocketClientConnect(TObject *Sender, TCustomWinSocket *Socket);           // golden :4116-4125 GATE (Cat F)
    void RTMServerSocketClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);        // golden :4127-4131 GATE (Cat F)
    void RTMServerSocketClientError(TObject *Sender, TCustomWinSocket *Socket,
                                     TErrorEvent ErrorEvent, int &ErrorCode);                // golden :4133-4146 GATE (Cat F)
    void RTMServerSocketClientRead(TObject *Sender, TCustomWinSocket *Socket);              // golden :4148-4161 GATE (Cat F)
    void TimerRTMMsgTimer(TObject *Sender);                                                 // golden :4163-4183 GATE (Cat E)
    void RecordRunState();                                                                  // golden :4186-4232 GATE (Cat B)
    void HandlerClientSocketConnect(TObject *Sender, TCustomWinSocket *Socket);             // golden :4419-4426 GATE (Cat F)
    void CheckEPRange();                                                                    // golden :4428-4476 GATE (Cat E)
    void ATC_FFCTrigger(bool bOnOff);                                                       // golden :4478-4492 GATE (Cat C/E)
    void HandlerClientSocketError(TObject *Sender, TCustomWinSocket *Socket,
                                   TErrorEvent ErrorEvent, int &ErrorCode);                  // golden :4494-4506 GATE (Cat F)
    void HandlerClientSocketRead(TObject *Sender, TCustomWinSocket *Socket);                // golden :4508-4557 GATE (Cat F)
    void RecordPowerSaveEvenLog_HS(AnsiString aDataSRecordData, bool bIsATC = false);        // golden :4561-4583 GATE (Cat B)
    void RecordAutoMotive(AnsiString sDirPath, AnsiString aFileName,
                           AnsiString sD1, eRecordType iType);                                // golden :4587-4603 GATE (Cat B)
    void BackupGPIBLog();                                                                   // golden :4605-4633 GATE (Cat B)
    void TESTTEMPSETTING(int iTemp);                                                        // golden :4829-4895 GATE (Cat C/E)
    void RecordParameter_TFAMDLog();                                                        // golden :4949-5025 GATE (Cat B)
    std::vector<AnsiString> GetGPIBLogFileName();                                           // golden :5034-5051 GATE (Cat C)
    bool Check_FileFolderByFTP();                                                           // golden :5053-5211 GATE (Cat A)
    bool CheckMTKFileName(bool bInitial = false);                                           // golden :5213-5244 GATE (Cat B)
    void Check_RecordFolder(AnsiString asDirPath, bool bNeedTime = false);                  // golden :5246-5263 GATE (Cat B)
    void RecordGroundManLog_HS(AnsiString aFileName);                                       // golden :5265-5292 GATE (Cat B)
};

#endif // FORMS_FHS_H
