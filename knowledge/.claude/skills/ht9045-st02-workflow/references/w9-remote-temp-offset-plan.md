# W9 = A: remote temperature offset written into the recipe (SAFETY GATE S3 opened) — the plan

> Status 20260927: **GO**. The laptop verified every line on 5997abda (TO_STEVEN §4 12:3x):
> uTemp_Set.cpp :6306-6308 / :6345-6347 (S3), :6394 / :6397; SECSGEM/uHGemHT9045.cpp :7096 / :7118 / :7128 / :7131;
> forms/fTemp_Set.h :259-262 / :529-530. **Re-check the numbers after merging main.**
> Decisions (St02-M): 912 (a) the GPIB reload = YES; (b) iSiteToOfs = OUT (keep it in the ledger as pending, with the
> reason: iSiteToOfs is zero until the first START); SECS G34 = OPEN; FTP = OUT (not translated).
> Extra: change G34's :7106 comment ("夜間迴圈不會打開它") to past tense.
> The ctest is drafted, untracked: `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_w9_remote_temp_offset.cpp` (cases a-f,
> sandbox guard, real-file size/mtime check, never CloseIniFile). Add its CMake block in St02's section after
> TesterComm_W1ArtBrand (same link line as W1: testercomm_handler + testercomm + the RESCAN group + ws2_32).
> Ledger: put the 10-point 上機要看 list in docs\TESTERCOMM_PORT_LEDGER.md; commit message: points #1 / #5 / #6 / #10.
> Paths below are relative to `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`.

---

W9 survey (read-only; golden refs cite the tree name). No edits.

Naming correction first:
- ReadRemoteTempOffset only READS. The writer is SaveRemoteTempOffset. SaveRemoteTempOffsetFromGPIB is the GPIB parser that calls it.
- The "S3" in wb_serve.cpp:3795 / DUET3D doc is a different S3 (web START).

Golden 906_0625_Steven uTemp_Set.cpp:
- :5856-5973 SaveRemoteTempOffset(iArm,iSite,dOffset). File = DataPath + <SetUp.inf line 1> + "\Temperature.Data". It calls MyForceDirectories, and head = fMain->RefreshTempData(true,arm,site) (<0 → -1).
  - ATC branch (ATC_SYSTEM>eATC60 && !=eNonChamber): key [ATC] ATCTempOffset[<myTempPal[head]->iIndexTag>]. Limits are InputLimit.iTempLow/High, or N31 min/max (swapped if reversed). It writes only when eNewATCSystem, strictly inside the limits: WriteIniData(...,"ATC",S,now+offset) :5917.
  - Other branch: [User OffSet] CH<head+1>, strictly inside, WriteIniData :5954.
  - It ACCUMULATES (now + offset). It writes the file only: no memory update, no UI.
- :5975-6059 FromGPIB: "_"→"," CommaText. The arm comes from IndexStatus (0 = both); it calls Save once per non-zero value, sites 1..32. There is NO reload.
- :6061-6115 ReadRemoteTempOffset: [User OffSet] and [ATC] into Temperature.*, then fLotInfo->SetATCOffset(true) :6111, which pushes to the ATC.
- Golden callers:
  - GPIB main.cpp:16230-16248 (MSG_CMD_SetTJ);
  - SECS uHGemHT9045.cpp:3422-3431 (Save, then Read);
  - FTP uLotInfo.cpp:13910-14052 (Save x4, then Read at :14019).

912 vs 906 (uTemp_Set.cpp 912 :5934-6223), two RogerYang additions:
(a) THE W9 SUPPLEMENT, 912 :6142-6163 (20260823):
    - bNeedUpdateATC = any Save()==0;
    - after the loop: if set, ReadRemoteTempOffset() — "比照SECS TEMP_OFFSET, 重載offset並送ATC".
    GPIB was the only remote path that neither reloaded memory nor pushed to the ATC.
(b) NOT in your brief: 912 :5956-5974 + :6004 (20260824, "修正單排機型site2寫錯欄位"). It adds iOfsIndex via iSiteToOfs[iArm-1][r][c] from TestIF_File.iSiteMap, and changes the ATC key test to i==iOfsIndex.
    Unsafe in V906 today: iSiteToOfs is zero-initialised (fTemp_Set.h:547) and only filled by InitialAddrToATC, which runs at START (WebStart.cpp:1180) and is still gated at boot (cinitial.cpp:17551 N1-G5). So before the first START every site maps to ATCTempOffset[0].
    It needs its own decision. I propose: not now.

V906 today (uTemp_Set.cpp is jimmychiu's; all target lines blame c60e9f43 jimmychiu 0820; CRLF):
- Save :6245-6366, FromGPIB :6371-6403, Read :6407-6468. They match golden 906 apart from the gates.
- S3 = the two #if 0 blocks around the WriteIniData calls: :6306-6308 (ATC) and :6345-6347 (User OffSet). Banners :6241-6243, :6368-6369; header table :71-73; register forms/fTemp_Set.h:247-262 ("reserved for explicit user sign-off"). W9 = A is that sign-off.
- LIVE now:
  1. Tester SETTESTOFFSET_… / DEVICETEMP… → GpibCommands.cpp:597-603 sends MSG_CMD_SetTJ and replies "ECHO";
  2. HandlerGpibMsg.cpp:1144-1166 → FromGPIB → Save;
  3. Save calls MyForceDirectories (:6256, outside S3, live), reads the real file and checks the bounds;
  4. it skips the write, BUT it logs "Remote Control Temp. Offset a to b" and returns 0.
  The log claims success, and nothing changes.
- SECS: uHGemHT9045.cpp:7096-7131 GATE G34 (answers HCACK=1), dead. FTP: not translated. No test and no web action calls any of the three.
- Still gated after W9: GATE(dep-fLotInfo-SetATCOffset) :6462-6464 (SetATCOffset has 0 definitions). ATC machines get file + memory only; the ATC controller keeps its old offset.

Proposed exact edits (all same-line, 0 net lines, CRLF kept):
- uTemp_Set.cpp :6306 / :6345 `#if 0 // SAFETY GATE (S3)` → `// AI(W906-W9) 20260927: SAFETY GATE (S3) opened (Steven W9=A) -- golden 906 :5917` / `:5954`.
- :6308 / :6347 `#endif // SAFETY GATE (S3)` → `// (end of former SAFETY GATE (S3))`.
- 912 (a), in FromGPIB:
  - :6387 `        {` → `        {   bool bNeedUpdateATC=false;   //AI(W906-W9) 20260927: 912 uTemp_Set.cpp:6142 (RogerYang 20260823) GPIB下的溫度offset需回送ATC`
  - :6394 `SaveRemoteTempOffset(1, i, dTempureOfs);` → `{ if(SaveRemoteTempOffset(1, i, dTempureOfs)==0) bNeedUpdateATC=true; }   // 912 :6149-6152`
  - :6397 the same with (2, …)   // 912 :6155-6158
  - :6398 `            }` → `            }   if(bNeedUpdateATC==true) ReadRemoteTempOffset();   // 912 :6161-6162 (ATC push stays GATE(dep-fLotInfo-SetATCOffset))`
- Banner / table comment lines, same line: :71-73, :6241-6243, :6368-6369, :6405 (":6405 says 'Read-only' but :6414 calls MyForceDirectories").
- forms/fTemp_Set.h :259-262 register text, :529-530 comments.
- No new dependency (same class; ht9045_sm).
- Laptop-side scripts that register S3 "Expect=PRESENT" (tools/stale_premise_scan.ps1:724-748, wave_wrapup_gate.ps1:1050-1060, jimmychiu) keep passing, but their wording goes stale.
- OPTIONAL, needs your / Steven's confirmation (W9's text doesn't name SECS): open G34 too. uHGemHT9045.cpp :7096/:7106 comments; :7118 `if(fTemp_Set && fTemp_Set->SaveRemoteTempOffset(iArm, iHead, dOffset)==0)`; :7128 `#else` → `#if 0`; :7131. jimmychiu's file.

Ownership:
- TO_STEVEN §1 on main: no claim on uTemp_Set.cpp / fTemp_Set.h / uHGemHT9045.cpp.
- St01's branch (855a6a83) touches none of them.
- So claim in FROM_STEVEN §1, then confirm line numbers with the laptop (per the ruling).

Containment:
- The path is built inline (DataPath + GetLastOpenFN() + "\Temperature.Data"), and DataPath is under W906_INIDATA_ROOT, so ctest writes land in scratch.
- W906_SETUPINF_PATH is not in the blanket, so ctest reads the real SetUp.inf name (read-only).
- wb_serve refuses to start with W906_INIDATA_ROOT set, so on a machine it writes the real recipe.

ctest proposal: tests/test_w9_remote_temp_offset.cpp, TesterComm_W9RemoteTempOffset. W1's link line, placed right after W1.
- Setup:
  - sandbox %TEMP%\ht9045_w9_<tick>, with a SetUp.inf naming "W9R";
  - DataPath / LastDataPath set in main(); abort before any call if a path is under D:\HT9045, plus a size/mtime check that the real Temperature.Data is unchanged;
  - fTemp_Set = new TfTemp_Set(); Init();
  - a TfMain subclass overrides RefreshTempData (the test_testercomm_handler idiom).
- Cases:
  a. User OffSet written as "%0.4f", and it accumulates;
  b. the strict bounds; head -1 gives no file;
  c. eNewATCSystem ATCTempOffset[tag], the N31 swapped limits, bUseOldATCTempOffset → -1, eATC30 no write;
  d. the 912 reload: FromGPIB("SETTESTOFFSET_1.5") updates file AND memory (a sentinel 99 becomes 1.5), where golden 906 would leave 99;
  e. _0 / _70 → no write and no reload;
  f. iArm=0 → both arms;
  g. optional: end to end through THandlerTesterSide::Sink(MSG_CMD_SetTJ).
- Never call CloseIniFile() (freed-INIFile bug).

上機要看 (rule 4):
1. Back up the recipe's Temperature.Data first, and check that SetUp.inf names the running recipe (else it writes Data\Fail Open\).
2. One small SETTESTOFFSET_0.5 on one site: the log line, the key and its "%0.4f" value, and the Temp_Set page shows the reloaded value.
3. Does the heater SV actually move? Memory feeds bthermo.cpp:282 ConvertTempOffset; whether the new SV is re-sent is not traced.
4. On ATC machines the ATC does not change (SetATCOffset gap).
5. It accumulates per command up to ±60: does the customer's tester send increments or absolute values, and does it retry?
6. DEVICETEMP… is treated as an offset: a tester sending absolute Tj within ±60 would write it into the recipe.
7. The target arm follows IndexStatus when the message arrives.
8. The tester always gets "ECHO", so no failure feedback. The out-of-range log prints doubles with %d (a golden quirk).
9. The change is persistent for every later lot on that recipe, with a ChangeLog entry via the CHGLOG hook. A recipe-MD5 check may flag it.
10. On any PC running wb_serve against a real IniData, a GPIB SETTESTOFFSET_ now edits that PC's recipe.
Say go (and G34 yes/no) after you've talked to the laptop, and I'll do it in St02's section. (b) stays out unless you say otherwise.
