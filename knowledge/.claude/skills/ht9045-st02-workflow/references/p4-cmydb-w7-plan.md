# cMyDB P4 (D1 A, D2 B, D3 A, D4 B, D5 / D8 done, D7 A) + W7 A — the plan

> Status 20260927: **GO**, and the laptop agrees to the whole shared-file list (one atomic P4 commit is fine).
> **DONE locally 20260927 (St02-E)**: `b4712e5f` (a) W7 + hook, `5c30e8cf` (a) optional hardening, `ec1b034c` (b) D2 + D4, then the P4 atomic commit (c) on
> `v906/steven-p4-wip`; both configs compiled, nothing run, not pushed.  (a) also re-closes the four cMyDB gates that WIP 20cd39f4
> had lifted early and gates the D2 parts of 240d37c3, so that every commit compiles; (c) lifts them again with the stand-in deletions.
> Not done (outside the list): ChanAction.cpp:210-248 diagnostic text, EventLog.cpp:104 / EventLog.h:69 comments, the ~31 files citing old lines.
> Paths are relative to `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\` unless absolute. Golden = 906_0625_Steven.
> Line numbers are from fbb91a3d; **re-check after every main merge** (the laptop's 5997abda touched cmydef.cpp :6185).
> WIP already written in `D:\AI_TempFile\st02-p4` on local branch `v906/steven-p4-wip`:
> - `d1dfbfa9` LogObjects.cpp: all golden log objects + the HeaterLog hook body + the slAutoSiteMapLog derive-and-swap;
> - `240d37c3` LogObjects.cpp: the ProductionLog body, the boot-time day-file load, ProductionLog("Close") at destroy;
> - `20cd39f4` cMyDB.cpp: the 4 homecoming gates lifted (line-neutral) + `#include "forms/fProductionInfo.h"` on blank :127.
> Commit plan: (a) W7 + HeaterLog hook + seams (seam hardening listed separately so the laptop can veto it); (b) D2 + D4 + seams;
> (c) P4 atomic + containment ctest + ledger + mydb skill + progress-st02 #24-#30. Compile both configs per commit.
> NEW (the laptop): wb_serve's MyMessageBox host calls MyDBIProcess("Message" / "Exception") at wb_serve.cpp :6143 / :6847 /
> :6890 / :6953 / :7083. After P4 those really write the event log: cover them in the containment ctest and the ledger.

## 1. Gates P4 lifts (cMyDB.cpp, St02's)
Each body matches golden after whitespace normalisation; the only difference is MyDBIProcessNew without __fastcall (D3 A).
- :926-994 MyDBIProcessNew (golden cMyDB.cpp:724-787). ProductionLog at :949 (D2). RespondASECom (CC_ASE_KaohSiung) resolves to canary_support.cpp:243's stand-in, which stays.
- :1002-1070 MyDBIProcess, 3-arg __fastcall (golden :789-855). ProductionLog at :1022.
- :1864-1883 NewRecordProcess (golden :1545-1562). Greatek `fProductionInfo->SaveMessageHistroy("", S, 0, 0)` at :1869 (D4).
- :1892-1903 RecordProcess (golden :1564-1573).
Left alone: :1986 / :2095 edtASECL_TesterID, :2162 UploadEventLogFile, XLS / TChart / fObserver gates (P5).

## 2. The five stand-ins
- (1) SECSGEM/uHGemEquipment.cpp:3480-3500 (banner :3452-3479): 3-arg MyDBIProcess forwarding to the 2-arg one, dropping S3. 32 live calls / 18 files. → DELETE; after :98 add `extern void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2);` (no default). :5815 / :5855 then bind to golden. tests/test_ga1_cmydb.cpp's copy (:243-248) must go.
- (2) aHotPlateSubstrate.cpp:1244-1249 (counter :1236-1243): the 2-arg stub; 228 live calls / 56 files / 18 declarations; MyProductionRecord.cpp:1154 and FTPClient_Transfer.cpp:87 anonymous 3-arg folders route to it. → D1 ADAPTER IN PLACE (same symbol; the counter stays for FTPClient_EventHandlers):
      extern void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2);   // golden cMyDB.h:20, body cMyDB.cpp:789-855
      void MyDBIProcess(AnsiString S1, AnsiString S2)
      { W906_MyDBIProcess_Count++; W906_MyDBIProcess_LastS1=S1; W906_MyDBIProcess_LastS2=S2;
        MyDBIProcess(S1, S2, AnsiString("")); }   // golden 906 has no 2-arg overload: a 2-arg call = (asTable, S1, "")
  Link: aHotPlateSubstrate.o (sm) → cMyDB.o (db); every sm-linking test also links db.
- (3) canary_support.cpp:117-123 RecordProcess (printf): 485 live calls / 98 files. → DELETE. tests/test_ga1_cprod.cpp compiles ../canary_support.cpp directly ⇒ add `void RecordProcess(AnsiString, AnsiString) {}` next to its :168. common.cpp:98's file-local no-op stays (it also stops a recursion).
- (4) canary_support.cpp:537-543 MyDBIProcessNew + unused recorder :529-535 (canary_support.h :282-307). 28 live calls. → DELETE.
- (5) acatchtray_shims.cpp:152 NewRecordProcess {}. 159 live calls. → DELETE (.h:438-439 comment).
Total after P4: 947 live call sites reach golden bodies. P4 must be ONE atomic commit.

## 3. D2 ProductionLog (golden cpublic.cpp:594-622)
- Runs only when IniConfig.bO06SaveLogTimePeriod. File `<asProductionLogPath>\<SocketHandlerID>_yyyymmdd.logs`; lines go to fMain->MemoProductionLog, saved WHOLE at >32768 lines / "Close" / bSaveToFile. No MyForceDirectories.
- **DATA-LOSS RISK**: SaveToFile overwrites ⇒ golden FormShow's day-file load (main.cpp:9411-9416) MUST ship with D2 (done in WIP 240d37c3).
- Body lives in LogObjects.cpp (ht9045_db), NOT cpublic.cpp (globals has no fMain). The gated copy cpublic.cpp:778-806 stays as reference.
- Member (Jimmy's area, line-neutral on St02's blank line) forms/fMain.h:1301:
  `vclcompat::TMemo *MemoProductionLog = new vclcompat::TMemo();   // golden main.h:533 -- real TStringList; NOT TfMainMemo (4096 cap)`
- Seam: common.cpp:263 and :442 `asProductionLogPath = W906EnvPathOr("W906_RMS_ROOT", "D:\\RMS")`; tests/CMakeLists.txt St02 block (~:3428-3430): `"W906_RMS_ROOT=${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch/RMS"` + `file(MAKE_DIRECTORY …)`.
- Deviation: VCL SaveToFile throws EFCreateError on a missing dir; vclcompat fails silently. ProductionLog("Close") runs at destroy, not FormClose.

## 4. D4 SaveMessageHistroy (golden ProductionInfo/ProductionInfo.cpp:1136-1173)
- CSV `sProductionInfoFilePath\<sLoadMO_MO>\<_sOEE_DirectoryName>\<_sOEE_MO>_History.csv`, header `AlarmCode,AlarmDate,AlarmTime,ErrMessage,AlarmType,SkipLevel`; load, append, save. Only CC_Greatek 956 && bN14_1.
- forms/fProductionInfo.h: `AnsiString _sOEE_DirectoryName;` next to :129 (golden .h:344; stays ""); method decl after :280. .cpp: body at EOF + `#include "common.h"`; TStringList::Append → Add.
- Seam: common.cpp:293 `sProductionInfoFilePath = W906EnvPathOr("W906_PRODINFO_ROOT", "D:\\HT9045_log\\ProductionInfo")` (single site) + CMake APPEND. tests/test_automation.cpp:856 sets CC_Greatek (one flag away).
- tests/test_ga1_cmydb.cpp: include + `TfProductionInfo *fProductionInfo = NULL;` + a SaveMessageHistroy stub.

## 5. W7 = A: all golden log objects (golden main.cpp:1500-1675; FormDestroy :11991-12042)
- 27 `new TMyStringList` lines = 24 names = 58 objects (slHanaTrayMap ×33, slDewPointLog ×3). fMain.h already declares every member (:1277-1295).
- Real writers in wb_serve today: slHeaterLog (17 HeaterLog sites), slAutoSiteMapLog (5), sl2DMappingLog (2D sort; also fixes a latent NULL deref asortarm.cpp:3915), slQtyLog. No ctest calls W906_CreateLogObjects.
- slAutoSiteMapLog: derive-and-swap in LogObjects.cpp (done in WIP).
- HeaterLog: hook, line-neutral in cpublic.cpp (the laptop handed it to St02, TO_STEVEN :117):
    :704  extern void (*W906_HeaterLogHook)(AnsiString);   //AI(W906-LOGOBJ-W7): was GATE (PT-W5c); golden cpublic.cpp:523 runs via the hook
    :705  if (W906_HeaterLogHook) W906_HeaterLogHook(NewMess);   // golden: fMain->slHeaterLog->AddTextWithDateTime(NewMess);
    :706  //AI(W906-LOGOBJ-W7): (old #endif)
    EOF   void (*W906_HeaterLogHook)(AnsiString) = 0;
  HeaterSVLog :711 stays gated.
- slHanaTrayMap: line-neutral swap cmydef.cpp :6151 (definition) / :6152 (`#if 0` resumes) — re-check lines (St01 +4 above; laptop touched :6185).
- slQtyLog: JsonBridge/actions/MainClarnData.cpp:45 same line: prefer fMain->slQtyLog when non-NULL, then the lazy g_slQtyLog.
- Optional seam hardening (list separately for veto): LogObjects literals → as9045LogPath+"\\X"; common.cpp :246 / :430 asTorqLogPath, :266 asProductRecordPath, :268 asQtyDataPath derived from as9045LogPath; :316 W906_HPCARD_ROOT, :323 W906_GROUNDMAN_ROOT (golden lowercase `_log` kept) + CMake APPEND.

## 6. ctests
- test_ga1_cmydb: its two D5 roots are "" — point them at %TEMP% before :325 (else RecordChangeLogProcess fopens at the drive root); delete :238-248; rewrite :325-327.
- New St02 test tests/test_mydb_p4_containment.cpp (ELA_Service pattern): refuse unless all roots contain machine_log_scratch; start FILETIME; act with a unique token (RecordProcess / NewRecordProcess / MyDBIProcessNew / the adapter / RecordChangeLogProcess / ProductionLog with O06 / SaveMessageHistroy with Greatek+N14_1 / the MyMessageBox paths); expect token rows in scratch and NO file with mtime ≥ start under any real root (SaveEventLog, ASE log, EventLogTxt, D:\RMS, ProductionInfo + the 23 W7 roots). A running wb_serve gives a false failure.
- nm check: 3-arg MyDBIProcess / MyDBIProcessNew / NewRecordProcess / RecordProcess defined only in cMyDB.o (+ test copies); 2-arg only in aHotPlateSubstrate.o.

## 7. Shared lines (all cleared by the laptop)
aHotPlateSubstrate.cpp:1236-1249; SECSGEM/uHGemEquipment.cpp:88-98 + delete :3452-3500; canary_support.cpp:114-123 + :523-543 and .h :69-70 / :282-307; acatchtray_shims.cpp:152 (+ .h comment); cpublic.cpp:704-706 + EOF; cmydef.cpp:6151-6152; common.cpp :263 / :442 / :293 (+ optional hardening); forms/fMain.h:1301; forms/fProductionInfo.h :129 / :280 + .cpp EOF; JsonBridge/actions/MainClarnData.cpp:45; tests/test_ga1_cprod.cpp ~:168; tests/test_ga1_cmydb.cpp; tests/CMakeLists.txt St02 block. Comment-only: ChanAction.cpp:243, EventLog.cpp:93-99 / :113 (kSinkStdout false), HandlerBridgeCtl.cpp:88-101, ~31 files citing old stand-in lines.
Deviations for the ledger: the anonymous-namespace S3→S2 folding continues; silent SaveToFile failure; ProductionLog("Close") at destroy; the slAutoSiteMapLog swap.
