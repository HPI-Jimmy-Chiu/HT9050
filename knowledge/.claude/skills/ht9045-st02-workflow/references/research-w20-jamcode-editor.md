# W20 調查：分析器與 Handler 的 Jam Code 編輯器合成一個

> St02-M 開的唯讀調查（20260927）。**只是調查與提案，沒有改任何程式。**
> **20260928 狀態**：Steven 0928 第 18 項裁決（現在叫 ★W45）——18a W20-1＝A、18b W20-3＝B、18c W20-5＝A；其他題**未明確裁決，照 golden**（§6「狀態」欄）。
> 實作：St02-E helper，分支 `v906/steven-w45-jam` `6f1f8024`（兩組態只編譯）；帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md`「★W45（原 W20）Steven 0928 裁決後」。
> 讀的版本：
> - golden Handler 906＝`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`（下稱 906_0625_Steven）；golden 912＝`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`（下稱 912）。都是 cp950。
> - golden 分析器＝`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\`（SVN r891，下稱 Rev891），cp950。
> - V906（git 唯讀）：安全頁那一半用 `origin/main` `8b5a91b5`；`origin/v906/steven-cbridge-review6` `db1b7638`、`origin/v906/steven-gpib-widget` `c0ea3021` 的 `JamIniMerge.h`／`WebSecurityJam.cpp/.h`／`cSecurity.cpp`／`forms/fSecurity.h`／`tests/test_security_jam_merge.cpp`／`Status.Security.html` 的 blob 跟 main **完全相同**。ELA 那一半用 `origin/v906/steven-gpib-widget` `c0ea3021`（W15／W18／W19 之後，比 main 新；main 的行號另外標）。
> - 本機真檔 `D:\HT9045\Error\English\JAM0000.dat`（2026-02-12，39,197 bytes）**只讀**做格式確認。
> - 行號寫法：`檔:行`；V906 的檔寫 `（ref）`。

---

## 一句話結論

兩個編輯器改的是**同一個檔** `D:\HT9045\Error\English\JAM0000.dat`（cp950 INI，section＝區名，key＝`<碼>` 或 `<碼> <後綴>`），真正的衝突只有一個：**CC_ASE_CL（933）與 CC_TERAPOWER（967）這兩家，Handler 把它的「Include MTBA」勾選存進 `IncludeMTBF` 鍵、缺鍵當 true，而分析器同一個鍵缺鍵只對 `24 Motor` 與 24 個碼當 true**；加上兩邊的讀取都會「缺鍵就寫回預設」，所以誰先讀誰決定檔案內容。
建議：合一的編輯器放在 **Status.Security 的 Jam 分頁**（已經有 golden 權限、鎖、匯入匯出），補一個「每個鍵一個勾選框」的 `IncludeMTBF`（分析器的 `cbIncludeMTBF`），eventlog.html 只放一顆打開它的按鈕；缺鍵預設抽成一個 header-only 的共用規則檔，兩家別名客戶以 Handler 為準（只改 ELA，St02 的檔）。

---

## §1 檔案格式與缺鍵預設

### 1.1 相關檔案

| 檔 | 內容 | 誰讀／寫 | 依據 |
|---|---|---|---|
| `D:\HT9045\Error\English\JAM0000.dat` | Jam 設定 INI。section＝區名（`[01 Input Arm]`…`[31 Cylinder]`，分析器另外會產生 `[Process]`、`[Motion]` 等 CSV UnitName 的 section）；key＝`<碼>`（等級）或 `<碼> <後綴>` | Handler 讀寫；分析器讀寫；KYEC FTP（CC_AMD_M）會整檔下載覆蓋 | Handler 路徑 906_0625_Steven `cSecurity.cpp:216`；分析器 Rev891 `Analyzer.cpp:296`；FTP 906_0625_Steven `KYECFTP\FTPClient.cpp:238-250`、`:614-633` |
| `D:\HT9045\Error\{English,Chinese,Korea,Singapore}\<碼>.dat` | 訊息說明（RichEdit 內容，可能是 RTF） | 兩個編輯器都讀、都寫回 | Handler 906_0625_Steven `cSecurity.cpp:988-1012`、`:1183-1201`；分析器 Rev891 `Analyzer.cpp:2725-2749`、`:2775-2796` |
| `D:\HT9045\Error\AlarmCodeList.txt` | `碼=訊息`，碼的第 4-5 字元＝區號 → 下拉清單 | 兩邊都只讀；Handler 開機時寫出 | 分析器 Rev891 `Common.cpp:318-343`；Handler 906_0625_Steven `cMyDB.cpp:1586-1611`（寫出 `:184`） |

**編碼**：TIniFile（Win32 profile API，ANSI）→ cp950、沒有 BOM（本機真檔第一行就是 `[01 Input Arm]`，開頭位元組 `5B 30 31`）。bool 寫成 `0`／`1`（TIniFile `WriteBool`）。

**本機真檔觀察（只讀）**：24 個 section（含 `[Process]`、`[Motion]`）；key 分布：等級 631、`Silent` 628、`Red` 600、`Bit8` 312、`IncludeMTBA` 129、`IncludeMTBF` 123、`O17ContiAlarm` 9、`AlarmAfterUnloaderFull` 4。`IncludeMTBF` 幾乎全是 `0` 且集中在 `[Process]`（65）、`[16 System]`（13）——符合「分析器查詢時補寫」的樣子（推論）。

### 1.2 缺鍵就寫回預設（兩邊都一樣）

- 分析器 Rev891 `Common.cpp:124-139`（bool）、`:107-122`（int）：`if(!INIFile->ValueExists(...)) INIFile->WriteBool(...default)`。
- Handler 906_0625_Steven `common.cpp:449-464`（bool）、`:432-447`（int）、`:466-491`（字串；空字串也會補寫 `:484-488`）。
- ⇒ **第一個讀到缺鍵的人把自己的預設寫進檔**，之後兩邊都讀那個值。兩邊預設不同時，檔案內容取決於讀取順序。
- 差別：分析器每個 getter 先檢查 `FileExists(FileNameJam000)`，檔案不在就不讀不寫、回 false／0（Rev891 `Analyzer.cpp:2805`、`:2818`、`:2842`；存檔 `:2768`、`:2792`）。Handler 不檢查；檔案不在時建構子會把 31 區 × 所有碼跑一遍把整個檔建出來（906_0625_Steven `cSecurity.cpp:216-237`）。
- Handler 的 `WriteIniData` 值有變時會寫「設定變更紀錄」（906_0625_Steven `common.cpp:636` 起）；分析器的 `WriteIniData` 沒有（Rev891 `Common.cpp:185-205`）。

### 1.3 每個鍵：誰讀、誰寫、缺鍵預設

Handler 行號＝906_0625_Steven `cSecurity.cpp`（912 同一段往後 +1 左右，見 §7）；分析器行號＝Rev891 `Analyzer.cpp`。

| key（section＝區名） | Handler 讀（缺鍵預設） | Handler 寫 | 分析器讀（缺鍵預設） | 分析器寫 |
|---|---|---|---|---|
| `<碼>`（等級） | `:1211-1212` 讀字串 `"0"`→`ToIntDef(0)`，再套客戶鉗制 `:1213-1293`（CC_GIGAS JAM0201→0；ASE_KH／AMKOR／QUALCOMM／SCC／預設分支的碼提高到 `LevelSet.AccessLevel[35]`；ASE_CL 的 WAR16126／WAR1685／WAR24xx ≥1；912 多 WAR16330 ≥1，912 `cSecurity.cpp:1294-1300`） | `:1129`（存前先鉗 `:1064-1122`） | `:2806` int 預設 0，**不鉗** | `:2770` |
| `<碼> Red` | `:1425` false；AMKOR／QUALCOMM／SCC 的清單強制 true `:1431-1457` | `:1127` | — | — |
| `<碼> Silent` | `:1313` false | `:1128` | — | — |
| `<碼> Bit8` | `:1302` int 0 | `:1130` | — | — |
| `<碼> UnlockPassWord` | `:1324` false（`bUseAlarmUnlockPassWord` 才讀，`:1019-1020`） | `:1131-1134` | — | — |
| `<碼> IncludeMTBA` | **非 ASE_CL／TERAPOWER**：碼含 JAM 且區在 01～05 → true，否則 false（`:1342-1354`） | `:1145` | **同一條規則**（`:2820-2832`） | `:2771` |
| `<碼> IncludeMTBF` | **只有 ASE_CL（933）／TERAPOWER（967）**：當成「Include MTBA」讀，**缺鍵 true**（`:1335-1338`） | 這兩家 `cbIncludeMTBA` 寫到這個鍵 `:1138-1141` | 區＝`24 Motor` 或碼在 24 個清單內（WAR01300、WAR01301、WAR0348、JAM0407、JAM0408、WAR1635/1636/1638/1639、WAR1690～1696、WAR16109、WAR2201～2203、WAR16150～16152、MES16119）→ true，否則 false（`:2846-2878`） | `:2772` |
| `<碼> bN27AddBoard`／`<碼> bN27AlarmSel`／區層級 `bN27AlarmSel` | `:1739` false／`:1728` true／`:1726` true | `:1151-1153` | — | — |
| `<碼> ContAlarmNotUpload` | `:1749` false | `:1158` | — | — |
| `<碼> ContiAlarm` | `:1364-1366` true（只有 JAM 碼） | `:1161-1164` | — | — |
| `<碼> O17ContiAlarm` | `:1377` false | `:1168` | — | — |
| `<碼> GetAddAlarmLog` | `:1388` false（CC_PTI） | `:1171-1174` | — | — |
| `<碼> TCPAlarm` | `:1398` false | `:1176-1179` | — | — |
| `<碼> AlarmAfterUnloaderFull` | `:1406-1408` false（只有 `02 Output Arm`） | `:1181`（每次都寫） | — | — |

- 客戶碼：`CC_ASE_CL 933`、`CC_TERAPOWER 967`（906_0625_Steven `MachineType.h:300`、`:337`）。
- `CosFunction.bIncludeMTBA` 在所有客戶都是 true（906_0625_Steven `CosFunction.cpp:4221` 預設；`FUNC_CC_ASE_CL` `:628`、`FUNC_CC_TERAPOWER` `:1785`、`FUNC_CC_TeraProbe` `:1833`、`FUNC_CC_SANAN` `:3289` 也設 true；沒有任何地方設 false）；912 同（912 `CosFunction.cpp:4357` 等）。⇒ Handler 的 `cbIncludeMTBA` 一直看得到（906_0625_Steven `cSecurity.cpp:289`）。
- 分析器分析時用的 section＝**CSV 的 UnitName 欄**（Rev891 `Analyzer.cpp:882`、`:901`，`tsRow->Strings[elUnit]`）。BCB Handler 寫的是 `"04 Input Shuttle"` 這種區名（樣本 `D:\HT9045_Log\EventLogTxt\2026\…`），跟編輯器的區名一致；但 `Process`／`Motion` 列也會被查，就在 JAM0000.dat 產生 `[Process]`、`[Motion]`（帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md` M9，gw 版 :203）。兩個編輯器都只列 31 區，看不到這些 section。

---

## §2 兩個 golden 編輯器

### 2.1 Handler：TfSecurity「Jam Code」分頁（906_0625_Steven `cSecurity.cpp`／`cSecurity.dfm`）

| 項目 | 內容 | 依據 |
|---|---|---|
| 位置 | `fSecurity`（「Password and Security」）的 `PageControl1.tsJamCode`；另一頁 `tsStatisticsJam` | `cSecurity.dfm:347-1012`、`:1014` |
| 欄位 | Area `cbJamArea`（31 區）、Code `cbJamCode`（AlarmCodeList 依區號）、Level `rgJamLevel`（Operator／Engineer／Supervisor／HonPrec；`bSecurityHave5Level` 時插第 0 格「Open」，KYEC_LEE 插「Operator」）、Language `cbJamLang`（English／Chinese／Korean／Singapore）、`RichEditJamCode`、`rgMachineStatusBit8`（0／1，SCK 隱藏）、勾選 13 個：`cbJamNeedRed`、`cbSilentMode`、`cbUnlockPassWord`、`cbIncludeMTBA`、`chkCheckContAlarm`、`chkO17`、`cbAddAlarmLog`、`cbN27AddBoard`、`cbN27AlarmSel`、`cbN27AlarmSelByArea`、`chkTCPAlarm`、`cbContAlarmNotUpload`、`chkAlarmAfterFullTray`；標籤 `labMustCheck_35` | dfm `:692-711`、`:712-760`、`:761-796`、`:797`、`:812-1012`；FormShow `:289-310`、`:416`、`:428-431` |
| 按鈕 | `spbImport`（CSV→每列寫 5 鍵：等級、Silent、Red、Bit8（非 SCK）、UnlockPassWord（有開才寫）；**不含 IncludeMTBA／MTBF**）、`spbExport`（逐區逐碼 `ChangeJamMessage(false)` 輸出 CSV：`JamArea, JamCode, JamLevel, JamNeedRed, SilentMode, UnlockPassWord, [MachineStatusBit8,] Message`；**不含 IncludeMTBA／MTBF**）、`SecurityExit` | `:1509-1577`、`:1579-1657`；dfm `:402`、`:540` |
| 驗證 | 區／碼不可空（`:1065`、`:1125`）；客戶鉗制：指定碼的等級不能低於 `[35] Alarm - Trouble Shooting`，某些客戶強制紅底（`:1064-1122`、`:1213-1293`、`:1418-1464`） | 同左 |
| 存檔時機 | 換區／換碼／換語言時先存上一筆（`ChangeJamMessage(bSave=true)` → `SaveJamLevel`，`:951-974`、`:980-981`）；關視窗（`FormClose` `:462`）；訊息檔只要 RichEdit 有行就寫（會建檔，`:1200-1201`） | 同左 |
| 誰能開 | 主畫面 `sbPassword` 要過 `Insufficient(29)`「[29] Config - Password」（906_0625_Steven `main.cpp:27677-27685`；912 `main.cpp:28589-28597`）；Jam 分頁所在的 `PageControl1` 只在 HonPrec，或 Supervisor 且非 SPIL／SCS 時看得到（`cSecurity.cpp:301-380`） | 同左 |
| 何時重讀 | FormShow 選第 0 區第 0 碼（`:417-426`）；每次換選重讀；Import 後重讀（`:1571`）；getter 沒有快取，執行時每次直接讀檔 | 同左 |
| 執行時用到的地方 | `note.cpp:344-380` `CheckRecordJamType`（IncludeMTBA→Jam 計數）、`note.cpp:1808`／`:5259`（等級＝解除權限）、`:1480`（Silent）、`:1488`（UnlockPassWord）、`:1761`（Red）、`:940`（Bit8）、`cMyDB.cpp:1373-1381`（MTBA 筆數）、`cObserver.cpp:2671-2673` 等 | 906_0625_Steven |

### 2.2 分析器：TfrmELA「Jam Code Setting」分頁（Rev891 `Analyzer.cpp`／`Analyzer.dfm`）

| 項目 | 內容 | 依據 |
|---|---|---|
| 位置 | `PageControl2.tsJamCode`（Caption「Jam Code Setting」） | `Analyzer.dfm:1840-2043` |
| 欄位 | Area `cbJamArea`（同樣 31 區）、Code `cbJamCode`、Level `rgJamLevel`（固定 4 格：Operator／Engineer／Supervisor／**Hontech**，沒有 5 級）、Language `cbJamLang`、`RichEditJamCode`、`cbIncludeMTBA`、**`cbIncludeMTBF`** | dfm `:1895-1910`、`:1911-1930`、`:1931-1978`、`:1979-2014`、`:2015-2042` |
| 按鈕 | 沒有（沒有匯入匯出、沒有存檔鈕） | dfm `:1840-2043` |
| 驗證 | 區／碼不可空（`:2759-2766`）；JAM0000.dat 不在就不寫（`:2768`）；訊息檔不在就不寫（`:2792`）；**不套任何客戶鉗制** | 同左 |
| 存檔時機 | **只有**換區／換碼／換語言（`ChangeJamMessage` 預設 `bSave=true`，`Analyzer.h:533`；`Analyzer.cpp:2690-2710`、`:2716-2717`）。`SaveJamLevel` 只在 `:2717` 被呼叫，`FormClose`（`:309` 起）不存 ⇒ 最後改的那一筆若沒換選就關視窗會丟掉（由程式推得） | 同左 |
| 寫哪些鍵 | 3 個：`<碼>`＝radio 的 ItemIndex（原始值）、`<碼> IncludeMTBA`、`<碼> IncludeMTBF`（`:2770-2772`） | 同左 |
| 誰能開 | **沒有密碼**：分頁一直看得到（FormShow 只藏 OEE／ProdLog，`:413-418`） | 同左 |
| 何時重讀 | Timer1 第一次（開機約 1 秒）選第 0 區第 0 碼（`:466-475`）；之後換選才讀 | 同左 |
| 分析用到的地方 | 等級：**只有編輯器顯示用**（`GetJamLevel` 只在 `:2751` 被呼叫）。IncludeMTBA → alarm 次數／時間（MTBA，`:882`）；IncludeMTBF → fail 次數／時間（MTBF，`:901`），每一筆停機列都查一次 | 同左 |

### 2.3 並排對照（差異）

| | Handler | 分析器 |
|---|---|---|
| 可改的鍵 | 14 種（含 `IncludeMTBA`，別名客戶是 `IncludeMTBF`） | 3 種（等級、IncludeMTBA、IncludeMTBF） |
| 等級 | 4 或 5 格、套客戶鉗制 | 固定 4 格、原始值、不鉗 |
| 存檔 | 換選＋關視窗 | 只有換選 |
| 權限 | [29]＋Supervisor／HonPrec | 無 |
| 匯入匯出 | 有（不含 MTBA／MTBF） | 無 |
| 檔案不在 | 建構子建整個檔 | 什麼都不讀不寫 |

---

## §3 V906 現況與缺口

### 3.1 已經有的

**Handler 編輯器已經上 web**（St01 `0609a14f` 20260926；S54 背景匯出 `d082c0be`；S127／S128 進度條＋缺鍵合回 St02 `4fbaf7e9`，已在 main）：
- 頁面：`D:\HT9045\web\page\Status.Security.html:56`（DFM 產生的單行頁；頁籤 `data-t="10"`「Jam Code」；有 `cbIncludeMTBA`，**沒有 `cbIncludeMTBF`**）（origin/main）；視窗 id `security`（`D:\HT9045\web\background.html:450`，origin/main）。
- 頁面 JS：`D:\HT9045\web\page\ht9045_wire_statussecurity.js:113-126`（說明）、`:130-131`（`BOXES` 13 個，沒有 MTBF）、`:200-235` render、`:236-245` values、`:257-262` open／select／save、`:388-411` wire（Exit 在 `:396` 送 save）（origin/main；review6 的 blob 不同但這幾段相同性未逐行比，推論相同）。
- WS 指令 `security.jam`：分派 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\wb_serve.cpp:5556`，關站 `:5965`；sysfile 表擋直接寫 `jam0000.dat` 並指向 security.jam `:1317`（origin/main）。
- C++ 本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp`（origin/main）：op 說明 `:12-48`；勾選表 `kBox` `:82-96`（13 個，都綁 `TfSecurity` 成員）；權限 `JamTabAllowed` `:102-109`（＝golden FormShow `:301-380`）；`ApplyValues` `:137-165`；`WriteState` `:172-216`；open `:793-845`（照 golden FormShow 的 Jam 那一半）；非 open 的寫入指令權限閘 `:913-917`；select／save／import 拿鎖 `:990-995`；select／save `:1001-1028`（呼叫 golden `SaveJamLevel`／`cbJam*Change`）；背景匯出 `:251-771`（getter 逐行複本 `:361-580`，`SnapGetJemIncludeMTBA` `:472-500`）；S128 合回 `:701-735`。
- golden 翻譯本體 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp`（origin/main）：`ChangeJamMessage` `:1120`、`SaveJamLevel` `:1204`（別名客戶寫 `IncludeMTBF` `:1283`、其他 `:1287`）、`GetJamLevel` `:1346`、`GetJemIncludeMTBA` `:1474-1504`（別名 `:1483`）、`GetJamArea` 閘住回空字串 `:1558-1570`、`W906_SecurityJamBoot` `:2047-2104`（路徑字面值 `:2085`）。成員 `cbIncludeMTBA` `forms\fSecurity.h:528`、private＋friend `:557-567`。
- 鎖：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamIniMerge.h:17-23`（理由）、`:37`（`Local\HT9045_JAM0000_dat`）、`:40-61`（`Lock`）、`:116-185`（備份／合回／驗證）（origin/main，blob `2438f728`）。ctest `Security_JamMerge`（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt:3443-3446`，全部在 `%TEMP%\ht9045_s128_<tick>`）。

**ELA 端**（St02；gw `c0ea3021`）：
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.h:91-94`（`Options.jamWriteBack`／`jamIniPath`）、`:163-176`（`JamConfig`：三個 getter，**沒有 setter**）；`ElaCore.cpp:69-70`（預設路徑字面值）、`:551-564`（`JamFileLock`，同一把具名鎖，等 5 秒）、`:567-597`（查＋補寫在鎖內；拿不到鎖就只讀不寫）、`:599-652`（getter，照 Rev891 `:2799-2882`）、`:916`／`:932`（每筆停機列查 MTBA／MTBF）。main 版行號：`ElaCore.cpp:68`、`:445`、`:461-525`。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp:77-141`（只有 `GET /api/ela`、`POST /api/ela/query`，**沒有 Jam API**）、`:151`（`ela::Options o;` 用預設，沒帶客戶碼、沒有路徑 env）；`ElaHub.cpp:36-53`（env seam 只有 `W906_EVENTLOG_ROOT`／`W906_PRODLOG_ROOT`／`W906_GENERAL_INI_PATH`）。
- `D:\HT9045\web\page\eventlog.html`（gw）：沒有 Jam 分頁；`:84` 起是 31 區名（By-Area 過濾框用）；`:19` 寫明報表／上傳不在這頁。
- ctest `ELA_Core` 第 5 節（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_ela_core.cpp:347-440`，gw）：JAM0000.dat 在 `%TEMP%`，驗補寫與 `jamWriteBack=false`。
- 帳本 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md`（gw）`:171`（JAM0000.dat 列）、`:237`（G10 寫 Handler 檔）、`:264`（§8 第 6 題：編輯器要不要搬——就是 W20）、`:340-347`（`cbJamAreaChange`／`ChangeJamMessage`／`SaveJamLevel`／getter 在函式帳本）。

**Handler 執行時的讀者（V906，origin/main）**：
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:1883-1885`（GetJamLevel／Silent／Red，主執行緒，不拿鎖）。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\Command.cpp:15131`（GetBit8，主執行緒，不拿鎖）。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cMyDB.cpp:1668-1670`（MTBA 筆數；`GetJamArea` 閘住回 ""，所以 `GetJemIncludeMTBA` 直接回 false；golden 自己也有 bug：碼那一格也呼叫 `GetJamArea`，906_0625_Steven `cMyDB.cpp:1376`）。
- `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fNote_JamCount.cpp:59-62`：`CheckRecordJamType` 的 MTBA 分支 **GATE(W906-J2) G1 閘住**（fNote 沒有 sJamArea／sJamCode）；該檔 `:24` 註明「MTBA 那一支閘著」；V906 `CosFunction.cpp:4438` 預設 `bIncludeMTBA=true`，所以推論目前所有客戶都走被閘住的那一支、Jam 計數不加（St02-E 動手前再確認）。
- ⇒ **V906 今天真正在執行時用 IncludeMTBA／MTBF 的只有 ELA**；Handler 端只有 security.jam 頁（開／選／存／匯出）會讀寫它們。

### 3.2 誰擁有哪個檔

| 檔（絕對路徑） | 擁有者 | 依據 |
|---|---|---|
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp`、`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\forms\fSecurity.h` | St01（原始落地是 Jimmy FW-SecCC `4626bd1b`；Jam 分頁 `0609a14f`、S64 `8c5ea501`／`855a6a83`／`2e70279d` 都是 St01） | `git log origin/main` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp/.h` | St01 寫，**已交出給 St02** | `C:\Users\steven\.claude\skills\ops-st02-manager\SKILL.md:64` |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamIniMerge.h`、`tests\test_security_jam_merge.cpp` | St02（`4fbaf7e9`） | `git log` |
| `D:\HT9045\web\page\Status.Security.html`、`D:\HT9045\web\page\ht9045_wire_statussecurity.js` | St01（St02 在 `4fbaf7e9` 改過 JS） | `git log`（`62f064bf` 標「St01 page JS」） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\*`、`D:\HT9045\web\page\eventlog.html`、`tests\test_ela_*.cpp`、`docs\ELA_PORT_LEDGER.md` | St02 | `git log`、ELA skill §5 |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\common.cpp`、`Command.cpp`、`KYECFTP\*`、`CMakeLists.txt`、`tests\CMakeLists.txt`、`tools\wb_serve.cpp` | 筆電（Jimmy）／共用 | ST02-M 手冊 §1、`git log`（common.cpp 最近是 jimmychiu `42ea830b`／`752130a5`） |
| `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebCmdGuard.cpp`（`security.jam` 的 op 白名單 `:28`、`:118`） | St01 | `git log` |

### 3.3 缺口（跟兩個 golden 比）

1. **沒有 `cbIncludeMTBF`**（分析器的勾選）：`WebSecurityJam.cpp:82-96`、`ht9045_wire_statussecurity.js:130-131`、`Status.Security.html:56` 都沒有；V906 目前沒有任何 UI 能改 `IncludeMTBF`（非別名客戶）或 `IncludeMTBA`（別名客戶）。
2. **eventlog.html 沒有進入點**（分析器使用者本來在分析器視窗裡就能改）。
3. **訊息說明唯讀**：兩個 golden 都能改 RichEdit 並存回 `<碼>.dat`；V906 網頁唯讀，存檔時把載入的行寫回（`WebSecurityJam.cpp:43`、JS `:124`）。已是既有偏離。
4. **912 的 WAR16330 鉗制沒進移植樹的 `GetJamLevel`**（`cSecurity.cpp:1346` 起是 906 版），但背景匯出的複本 `SnapGetJamLevel` 有（`WebSecurityJam.cpp:445-451`，註解說「交 Jimmy」）⇒ 同一個 WAR16330，頁面顯示的等級跟匯出 CSV 可能不同（由程式推得）。
5. **ELA 不知道客戶碼**：`ElaService.cpp:151` 用預設 `Options`，`custCode` 空；要做別名客戶規則得把客戶碼帶進去（Hub 的設定快照有 `sCustCode`，`ElaHub.cpp:390`，推論可用）。
6. **JAM0000.dat 路徑沒有 env seam**：Handler 端字面值 `cSecurity.cpp:2085`；ELA 端只有 `Options.jamIniPath`（ctest 直接設），沒有 env（`ElaHub.cpp:36-53`）。security.jam 的端到端測試目前只能碰真檔。
7. **鎖沒有涵蓋所有寫者**：拿鎖的只有 `WebSecurityJam.cpp`（open／select／save／import、合回）與 `ElaCore.cpp` `JamConfig`；主執行緒上其他 getter（`cObserver.cpp:1883-1885`、`Command.cpp:15131`）的補寫、KYEC FTP 的整檔下載（`KYECFTP\FTPClient_Transfer.cpp:496-497`，上傳 `:865-866`）都不拿（origin/main）。
8. **文件過時**：`D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\SKILL.md` §5 說「`JamIniMerge.h` 還沒合進 St01 分支／main」——實際已在 origin/main（blob `2438f728`）。St02-E 更新 skill 時順手改。

---

## §4 衝突

### 4.1 不一致的地方

| # | 項目 | Handler | 分析器 | 會不會真的衝突 |
|---|---|---|---|---|
| C1 | `IncludeMTBF` 缺鍵預設（別名客戶 933／967） | 全部 true（`cSecurity.cpp:1338`） | 24 Motor＋24 碼 true，其他 false（`Analyzer.cpp:2846-2878`） | **會**：先讀者寫進檔 |
| C2 | `IncludeMTBF` 的意義（別名客戶） | ＝Handler 的「Include MTBA」（Jam 計數／MTBA 筆數） | ＝MTBF fail 次數 | **會**：一個鍵兩個意思 |
| C3 | `IncludeMTBA` 的意義（別名客戶） | Handler 不讀 | 分析器的 MTBA | 不衝突，但 Handler 編輯器改不到它 |
| C4 | `IncludeMTBA` 缺鍵預設（非別名客戶） | JAM＋01～05 區 true | 同 | 不衝突（`cSecurity.cpp:1342-1354`＝`Analyzer.cpp:2820-2832`） |
| C5 | `IncludeMTBF`（非別名客戶） | Handler 不讀不寫 | 分析器讀寫 | 不衝突 |
| C6 | 等級 | 5 級時第 0 格是「Open」，存的值整格位移；存與讀都套鉗制 | 固定 4 格、原始值、不鉗；分析器不拿等級做分析 | 顯示會錯位；分析器存檔可繞過鉗制（Handler 讀時仍會再鉗，執行時影響有限） |
| C7 | 權限 | [29]＋Supervisor／HonPrec | 無 | 合一後要選一個 |
| C8 | 檔案不在 | 建構子建整個檔 | 不讀不寫 | 小 |
| C9 | 存檔紀錄 | 值變了寫設定變更紀錄 | 不寫 | 小 |
| C10 | section | 只有 31 區 | 另外產生 `[Process]`／`[Motion]`… | 兩邊編輯器都看不到後者 |

### 4.2 具體例子

1. **（別名客戶，分析器先讀）** CC_ASE_CL 機台，`[03 Index Unit]` 有 `JAM0305=0`，沒有 `JAM0305 IncludeMTBF`。ELA 查到一筆 JAM0305 停機 → `GetJemIncludeMTBF("03 Index Unit","JAM0305")` 不在清單 → 預設 false → 寫入 `JAM0305 IncludeMTBF=0`（Rev891 `Analyzer.cpp:2877`；V906 gw `ElaCore.cpp:648-649`）。之後 Handler 關掉 JAM0305 的警報 → `CheckRecordJamType` → `GetJemIncludeMTBA` 在 ASE_CL 讀 `JAM0305 IncludeMTBF`＝0 → 不加 Jam 計數（906_0625_Steven `note.cpp:355-358` → `cSecurity.cpp:1335-1338`）；Security 頁的「Include MTBA」也顯示沒勾。**若 Handler 先讀**，寫的是 `=1`：Handler 計數，分析器也把 JAM0305 當 MTBF fail。
   - 補充（推論）：golden 機台第一次開機、JAM0000.dat 不在時，Handler 建構子會把 31 區所有碼跑一遍（`cSecurity.cpp:216-237`），在 ASE_CL 就把每個碼的 `IncludeMTBF` 寫成 1——所以實際的 ASE_CL 機台上，31 區的碼多半已經是 Handler 的值；分析器的預設只落在 Handler 沒碰過的 section。
2. **（別名客戶，Handler 編輯器）** 工程師在 Security 頁把 `24 Motor` 的 WAR2401「Include MTBA」取消勾選 → 寫 `WAR2401 IncludeMTBF=0`（`cSecurity.cpp:1138-1141`）→ 分析器不再把 WAR2401 算 MTBF fail（分析器對 24 Motor 預設是 true）。畫面寫 MTBA，改到的是分析器的 MTBF。
3. **（別名客戶，分析器編輯器）** 在分析器勾「Include MTBA」→ 寫 `IncludeMTBA`（`Analyzer.cpp:2771`），Handler 在 ASE_CL 從不讀這個鍵，沒效果；勾「Include MTBF」→ 寫的正是 Handler 當成 MTBA 讀的鍵（`:2772`），Handler 的 Jam 計數跟著變。兩個框的效果在 Handler 看來是反的。
4. **（5 級機台，等級）** `bSecurityHave5Level=true`：Handler 頁 JAM0126 選「Operator」＝第 1 格 → 檔案 `JAM0126=1`（`cSecurity.cpp:303-310`、`:1129`）。分析器 4 格固定，第 1 格顯示「Engineer」（`Analyzer.dfm:1923-1927`）。分析器存檔也不鉗：例如預設分支的 JAM0203 在分析器可以存成 0（`Analyzer.cpp:2770`），但 Handler 存與讀都會拉到 `[35]`（`cSecurity.cpp:1109-1119`、`:1281-1291`）——檔案值與 Handler 畫面不一致。
5. **（非別名客戶）** IncludeMTBA 兩邊預設相同、IncludeMTBF 只有分析器讀 ⇒ 讀取順序不影響結果。**所以 C1／C2 只發生在 933／967 兩家。**

---

## §5 合併方案（提案，未實作）

### 5.1 建議的形狀

- **一個編輯器，放在 Status.Security 的 Jam 分頁**（`D:\HT9045\web\page\Status.Security.html` 頁籤 `data-t="10"`）。理由：golden Handler 的權限、客戶鉗制、匯入匯出、S54 背景匯出、S128 合回、鎖都已經在那裡；分析器那一半只多一個鍵。
- **eventlog.html 放一顆「Jam Code Setting…」按鈕**，打開 `security` 視窗（`D:\HT9045\web\background.html:450`）並切到 Jam 分頁；權限照 Handler（看不到分頁的等級會看到 guard `not-authorized`）。怎麼從 iframe 叫開另一個視窗要找現成的呼叫（推論：其他頁有類似的做法，St02-E 找）。
- **「每個鍵一個勾選框」**（W20-5 的建議 A）：
  - 框 1＝golden Handler 的 `cbIncludeMTBA`（不改）：一般客戶綁 `IncludeMTBA`，933／967 綁 `IncludeMTBF`。
  - 框 2＝分析器那一格（新，id `cbIncludeMTBF`）：**綁「另一個鍵」**——一般客戶綁 `IncludeMTBF`（標「Include MTBF」＝分析器原樣），933／967 綁 `IncludeMTBA`（標「Include MTBA（分析器）」）。這樣任一個 golden 編輯器能改的鍵，合一後都還改得到，也不會兩框寫同一個鍵。
  - 框 2 不是 `TfSecurity` 的成員（golden Handler 沒有這個元件），做成 `WebSecurityJam.cpp` 自己的「虛擬勾選」：讀／寫都在主執行緒、在已經拿著的鎖裡（`:990-995`），用 Handler 的 `CheckAndReadIniData`／`WriteIniData`（`common.cpp`，跟同分頁其他鍵同一個 INIFile 單例、也會寫設定變更紀錄），預設值從共用規則檔取。**不動 `cSecurity.cpp`／`forms/fSecurity.h`。**
  - 存檔順序照兩個 golden：先 golden `SaveJamLevel`（Handler 的鍵），再寫框 2（分析器 `SaveJamLevel` 的最後一行 `Analyzer.cpp:2772`）；select 時先存 from 那筆的框 2，再觸發 golden `cbJam*Change`；新選的碼在 `WriteState` 讀框 2。
- **缺鍵預設抽成一個 header-only 規則檔**（新 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h`，St02；header-only 的理由同 `JamIniMerge.h:6-7`，wb_serve 的來源清單不變）：
  - `IsMtbfAlias(cust)`＝933／967（golden `cSecurity.cpp:1335-1336`、`:1138-1139`）。
  - `DefaultIncludeMTBA(area, code)`＝兩邊相同的那條（`Analyzer.cpp:2820-2832`）。
  - `DefaultIncludeMTBF(area, code, cust)`＝分析器清單（`Analyzer.cpp:2846-2871`）；**W20-3＝B 時**別名客戶回 true（＝Handler `cSecurity.cpp:1338`）。
  - `SecondBoxKey(cust)`＝框 2 綁的鍵後綴。
  - 使用者：`WebSecurityJam.cpp`（框 2）與 `ElaCore.cpp` `JamConfig`（ELA 分析）。golden Handler getter 不改（W20-3＝B 只讓 ELA 偏離）。
- **ELA 端**：`JamConfig::GetJemIncludeMTBF` 的預設改從 `JamRules.h` 取（要 `Options.custCode`）；W20-3＝A 時照舊。ELA **不做**編輯 API；ELA 每一筆停機列都重新讀檔（`ElaCore.cpp:570`，沒有快取），編輯器存完下一次查詢就看得到，不用通知 Hub。

### 5.2 兩邊怎麼吃同一份資料

```
 Status.Security Jam 分頁 ──WS security.jam──► WebSecurityJam.cpp（主執行緒，拿 Local\HT9045_JAM0000_dat）
        ▲ 按鈕打開                                 ├─ golden TfSecurity：ChangeJamMessage／SaveJamLevel／getter（14 種鍵，框 1）
 eventlog.html                                   └─ 框 2：common.cpp CheckAndReadIniData／WriteIniData＋JamRules.h 預設
                                                        │
                            D:\HT9045\Error\English\JAM0000.dat（cp950 INI，格式不變）
                                                        │
 Handler 執行時：cObserver／Command／cMyDB／fNote_JamCount（直接讀檔，沒有快取）
 ELA worker：ElaCore JamConfig（每筆停機列查 IncludeMTBA／IncludeMTBF，同一把鎖，預設從 JamRules.h）
```

### 5.3 並行

- 保留現在的具名鎖 `Local\HT9045_JAM0000_dat`（`JamIniMerge.h:37`、`ElaCore.cpp:551-564`）：頁面每個 op 最多等 2 秒、回 `busy`；ELA 每個鍵最多等 5 秒、拿不到就只讀不寫（#17＝A 的例外，下次查詢再補）。框 2 的讀寫在頁面已經拿著的鎖裡做（Win32 mutex 同執行緒可重入），不另外拿。
- 沒拿鎖的寫者（§3.3 第 7 點）：主執行緒的其他 getter 補寫的鍵，ELA 都不碰（Bit8、Red、Silent、等級）或預設相同（IncludeMTBA）⇒ 無害；W20-3＝B 之後 `IncludeMTBF` 兩邊也補同樣的值 ⇒ 同時補寫也寫同樣的位元組（推論）。剩下 KYEC FTP 整檔覆蓋（CC_AMD_M、`iAMD_Function==1`），見 W20-9。

### 5.4 ctest（全部只寫 `%TEMP%`，絕不碰 `D:\HT9045\Error`、`D:\HT9045_Log`；這台只編譯，請 ST01-M 代跑）

1. **新 `Jam_Rules`**（`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_jam_rules.cpp`，St02）：純函式表格測試——兩個 golden 的預設當 oracle（`Analyzer.cpp:2812-2882`、906_0625_Steven `cSecurity.cpp:1329-1358` 逐條：`01 Input Arm`/JAM0109→MTBA true、`16 System`/JAM1601→false、`24 Motor`/WAR2401→MTBF true、`07 Tester I/F`/WAR0701→MTBF false、清單內 MES16119→true…）、別名對應（933／967／其他）、`SecondBoxKey`、W20-3 的裁決值。
2. **一個檔兩個讀者、兩種順序**：在 `%TEMP%` 放一份小 JAM0000.dat，用 ELA `JamConfig`（真的程式）與「照 Handler 規則的讀者」（測試裡用 `vclcompat::TIniFile`＋`JamRules.h` 組）先後讀同一個缺鍵，順序對調再跑一次：W20-3＝B 時兩種順序最後的檔案位元組相同；`Options` 切 golden 模式時斷言兩種順序**不同**（把 golden 的衝突釘住當證據）。可以放在 `Jam_Rules` 或 `ELA_Core` 第 5 節後面（`tests\test_ela_core.cpp:347-440` 的做法）。
3. **`ELA_Core` 第 5 節擴充**：`Options.custCode="933"` 時 `GetJemIncludeMTBF` 補寫的值；另一條執行緒拿著鎖時不補寫（照 `test_security_jam_merge.cpp` 第 5 節的寫法）。
4. **security.jam 端到端**（沒有 ctest 能直接呼叫 `W906_SecurityJamOp`，它在 wb_serve 的來源裡）：加 env seam `W906_JAM0000_PATH`（Handler `cSecurity.cpp:2085` 同一行換成函式；ELA `ElaService.cpp:151` 設 `o.jamIniPath`），ST01-M 的 SIM 實跑把兩邊指到 `%TEMP%`。⚠ golden `SaveJamLevel` 也會寫 `D:\HT9045\Error\<語言>\<碼>.dat`（移植樹 `cSecurity.cpp:1204` 起的訊息段），所以端到端要照 #33 的「備份→驗證→還原」，或再加一個 Error 根目錄的 seam（W20-4 以外的選項，St01 的檔）。
5. 新測試的註冊在 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt`（共用檔，仿 `:3443-3446`），要先請筆電點頭。

### 5.5 要動的檔與擁有者（給 St02-M 認領）

| 順序 | 檔（絕對路徑） | 擁有者 | 改什麼 | 認領 |
|---|---|---|---|---|
| 1 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\JamRules.h`（新） | St02 | 預設規則＋別名＋框 2 的鍵 | 新檔 |
| 2 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.h`、`ElaCore.cpp`（`:599-652`） | St02 | `GetJemIncludeMTBF` 預設走 `JamRules.h`（W20-3＝B 才偏離）；帳本記偏離 | 自己的 |
| 3 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaService.cpp`（`:151`） | St02 | `o.custCode`＝機台客戶碼；`o.jamIniPath`＝`W906_JAM0000_PATH`（沒設＝字面值） | 自己的 |
| 4 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp`（`:82-96`、`:137-165`、`:172-216`、`:793-845`、`:1001-1028`） | St01 寫、已交給 St02 | 框 2（虛擬勾選）：state 帶 checked／visible／caption／key，select／save 先存 from 再觸發 golden 事件 | 在 gpib-widget 做＋FROM_STEVEN §1 認領 |
| 5 | `D:\HT9045\web\page\ht9045_wire_statussecurity.js`（`:130-131`） | St01 | `BOXES` 加 `cbIncludeMTBF`；標籤文字用 C++ 回的 caption | §4 請 ST01-M 同意 |
| 6 | `D:\HT9045\web\page\Status.Security.html`（`:56`，單行） | St01 | 在 `cbIncludeMTBA` 旁加一個 `<label id="cbIncludeMTBF">`（位置照分析器 `Analyzer.dfm:2029-2042` 相對 `cbIncludeMTBA` 的下方一格），標 `AI(W906-ELA-W20)`；行數不變 | §4 請 ST01-M 同意 |
| 7 | `D:\HT9045\web\page\eventlog.html` | St02 | 「Jam Code Setting…」按鈕打開 `security` 視窗的 Jam 分頁 | 自己的 |
| 8 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\test_jam_rules.cpp`（新）、`tests\test_ela_core.cpp` | St02 | §5.4 第 1～3 項 | 自己的 |
| 9 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tests\CMakeLists.txt` | 筆電（共用） | 加 `Jam_Rules`（仿 `:3443-3446`） | §1 貼行號＋§3 請筆電 |
| 10 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp`（`:2085`） | St01 | （選做，端到端用）同一行改成 `W906_Jam0000Path()`（仿檔尾 `W906_LevelSetPath`） | §4 |
| 11 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP\FTPClient_Transfer.cpp`（`:496-497`、`:865-866`） | 筆電（Jimmy） | W20-9＝B 才做：下載／上傳 JAM0000.dat 包在 `jamini::Lock` 裡 | §3 請筆電 |
| 12 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md`、`D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\SKILL.md`（§5 #20、JamIniMerge 已在 main）、`D:\HT9045\.claude\skills\ht9050-construction\references\progress-st02.md` | St02 | 記裁決與偏離 | 自己的 |
| 另案 | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cSecurity.cpp:1346` 起 `GetJamLevel` 補 912 的 WAR16330（912 `cSecurity.cpp:1294-1300`） | St01／Jimmy | 不屬 W20；§3.3 第 4 點，頁面與匯出不一致 | 轉告 |

- 第 4～6 項 St01 的頁面與 JS：若 St01 分支上還有沒進 main 的改動，照 ST02-M 手冊 §6 先問。
- 工作量（推論）：JamRules.h＋ELA 約 120 行、WebSecurityJam 約 80 行、JS／HTML 約 20 行、ctest 約 250 行。

---

## §6 要 Steven 裁決的題

| 題 | 問題（背景一句） | 選項 | 建議 | 在哪看 | 狀態（20260928） |
|---|---|---|---|---|---|
| **W20-1** | 合一的編輯器放哪一頁？ | **A** Status.Security 的 Jam 分頁，eventlog.html 放一顆打開它的按鈕；**B** eventlog.html 自己一個 Jam 分頁（同一個 `security.jam` 後端、同一個權限）；**C** 兩頁都放（同一支 JS） | **A**（一個入口；權限、鎖、匯入匯出都已在那裡） | `D:\HT9045\web\page\Status.Security.html:56`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp:12-48`；`D:\HT9045\web\page\eventlog.html:19` | **18a＝A**（Steven 0928）。已做：eventlog.html 工具列「Jam Code Setting」鈕打開 security 視窗的 Jam Code 分頁 |
| **W20-2** | 誰能改？分析器沒有密碼，Handler 要 [29]＋Supervisor／HonPrec | **A** 照 Handler；**B** 照分析器（任何人） | **A** | `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:413-418`；`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\main.cpp:27677-27685`、`cSecurity.cpp:301-380`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp:102-109` | 未明確裁決，照 golden：兩個 golden 不同→Handler 擁有的資料照 Handler（＝A；沿用 `JamTabAllowed`，第二框同一個閘） |
| **W20-3** | `IncludeMTBF` 缺鍵預設（只有 ASE_CL 933／TERAPOWER 967 會衝突：Handler 把「Include MTBA」存在這個鍵、缺鍵 true；分析器缺鍵只有 24 Motor＋24 碼 true） | **A** 照 golden 各自（誰先讀誰寫進檔）；**B** 兩邊同一份規則、以 Handler 為準（這兩家缺鍵＝true，ELA 偏離）；**C** 同一份規則、以分析器為準（Handler 偏離） | **B**（golden 機台開機建檔時 Handler 本來就把 31 區的碼寫成 1，推論；只改 St02 的 ELA，不動 golden 翻譯檔） | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:1329-1358`、`:1136-1147`、`:216-237`；`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2838-2882`；本檔 §4.2 例 1 | **18b＝B**（Steven 0928）。已做：`JamRules.h` 開關＝B，只有 ELA 偏離；Handler getter 不動 |
| **W20-4** | 「讀到缺鍵就寫回預設」要不要保留？ | **A** 保留（#17＝A 現況）；**B** 改成開機時在鎖內一次補齊 IncludeMTBA／IncludeMTBF（先備份、補完驗證），之後讀的時候不寫 | **A**（W20-3＝B 之後兩邊補同樣的值，先後順序不再影響結果） | `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Common.cpp:124-139`；`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\common.cpp:449-464`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.cpp:567-597`（gw） | 未明確裁決，照 golden（＝A，不變；第二框也是缺鍵寫預設） |
| **W20-5** | 分析器多的那一格怎麼放？ | **A** 每個鍵一個框：Handler 原本的框不動，第二框綁「另一個鍵」（一般客戶＝IncludeMTBF「Include MTBF」；933／967＝IncludeMTBA「Include MTBA（分析器）」）；**B** 933／967 只顯示 Handler 那一框（IncludeMTBA 鍵沒有 UI，照預設）；**C** 933／967 兩框連動、寫同值到兩個鍵（兩個 golden 都沒有） | **A**（任一個 golden 能改的鍵都還改得到，也不會兩框寫同一個鍵） | `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.dfm:2015-2042`、`Analyzer.cpp:2770-2772`；`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:1136-1147`；本檔 §4.2 例 2、例 3 | **18c＝A**（Steven 0928）。已做 St02 的部分（`JamRules.h` 規則＋`jamrules::SecondBox`、`WebSecurityJamW45.h`）；`WebSecurityJam.cpp`／頁面 JS／HTML 的呼叫點待認領 |
| **W20-6** | 等級用哪一套？（分析器 4 格原始值不鉗，而且分析器不拿等級做分析） | **A** 只用 Handler 的（分析器那組拿掉）；**B** 另外唯讀顯示檔案裡的原始值 | **A** | `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2751`、`:2799-2810`、`Analyzer.dfm:1911-1930`；`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:301-310`、`:1204-1297` | 未明確裁決，照 golden：兩個 golden 不同→Handler 的（＝A；分析器那組等級不加） |
| **W20-7** | 訊息說明（`Error\<語言>\<碼>.dat`）要不要能改？兩個 golden 都能改，V906 網頁現在唯讀 | **A** 維持唯讀（記偏離）；**B** 開放改（要處理 Big5／韓文／RTF 內容） | **A**（W20 先合鍵；要開放另案） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\WebSecurityJam.cpp:43`；`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:1183-1201`；`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\Analyzer.cpp:2775-2796` | 未明確裁決：兩個 golden 都能改，V906 在 W45 以前就唯讀（St01 的既有偏離）；**W45 沒動**。「照舊」＝唯讀、「照 golden」＝可改，兩者不同 → 待 St02-E／Steven 定 |
| **W20-8** | 匯入／匯出 CSV 要不要加 IncludeMTBA／IncludeMTBF 欄？ | **A** 照 golden 不加；**B** 加欄（舊機台／Excel 的檔會對不上） | **A**（S129：改格式要換檔名） | `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cSecurity.cpp:1509-1657`；`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\RULINGS_20260926.md:353` | 未明確裁決，照 golden（＝A：CSV 不加欄） |
| **W20-9**（次要） | KYEC FTP（CC_AMD_M、`iAMD_Function==1`）會整檔下載覆蓋 JAM0000.dat、沒拿鎖 | **A** 不動；**B** 請筆電把下載／上傳包進同一把鎖 | **B**（改動小，筆電的檔） | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\KYECFTP\FTPClient_Transfer.cpp:496-497`、`:865-866`；`D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\KYECFTP\FTPClient.cpp:238-250` | 未明確裁決，照 golden（＝A：KYEC FTP 不動；建議的 B 沒做、沒認領） |
| **W20-10**（次要） | 分析器產生的 `[Process]`／`[Motion]` 等 section 要不要在編輯器裡列出？ | **A** 不列（兩個 golden 都只有 31 區）；**B** 列出（新功能） | **A** | `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\ELA_PORT_LEDGER.md:203`（gw，M9）；本機 `D:\HT9045\Error\English\JAM0000.dat`（只讀） | 未明確裁決，照 golden（＝A：只列 31 區） |

---

## §7 來源清單

**golden 分析器（Rev891，cp950）** `D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\`
- `Analyzer.cpp`：`:296`（路徑）、`:413-418`（分頁可見）、`:466-475`（Timer1 初始化）、`:857-918`（停機／MTBA／MTBF 計數，`:882`、`:901`）、`:2690-2710`（換選事件）、`:2712-2754`（ChangeJamMessage）、`:2756-2797`（SaveJamLevel）、`:2799-2810`、`:2812-2836`、`:2838-2882`（三個 getter）
- `Analyzer.h:533`（`ChangeJamMessage(bool bSave=true)`）、`:536-538`
- `Analyzer.dfm:1840-2043`（tsJamCode 全部元件）
- `Common.cpp:50-70`（OpenIniFile／CloseIniFile）、`:107-139`（CheckAndReadIniData int／bool）、`:185-205`（WriteIniData bool）、`:318-343`（GetJameCodeOfAxis）

**golden Handler 906（906_0625_Steven，cp950）** `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`
- `cSecurity.cpp`：`:216-237`（建構子建檔）、`:260-436`（FormShow）、`:438-468`（FormClose，`:462`）、`:951-974`、`:976-1060`、`:1062-1202`、`:1204-1297`、`:1299-1411`、`:1418-1464`、`:1509-1577`、`:1579-1657`、`:1719-1752`
- `cSecurity.dfm:347-1012`、`:1014`
- `common.cpp:323-343`、`:432-491`、`:622` 起
- `main.cpp:27677-27685`；`note.cpp:344-380`；`cMyDB.cpp:184`、`:1338-1409`、`:1586-1611`；`cObserver.cpp:2671-2673`
- `CosFunction.cpp:600`／`:628`、`:1773`／`:1785`、`:1821`／`:1833`、`:3278`／`:3289`、`:4221`；`MachineType.h:300`、`:337`
- `KYECFTP\FTPClient.cpp:238-250`、`:614-633`

**golden Handler 912（cp950）** `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\`
- `cSecurity.cpp`：跟 906 只差兩處——`:207` 多 `[179] Yield - [Bin] Count Setting`、`GetJamLevel` 預設分支多 WAR16330 `:1294-1300`；對應行號 FormShow `:261`、`cbIncludeMTBA->Visible` `:290`、FormClose `:439`（`SaveJamLevel` `:463`）、ChangeJamMessage `:977`、SaveJamLevel `:1063`（IncludeMTBF `:1142`）、GetJamLevel `:1205`、GetJemIncludeMTBA `:1337`（`:1346`）、Import `:1517`、Export `:1587`。`cSecurity.dfm`／`cSecurity.h` 與 906 相同。
- `main.cpp:28589-28597`；`note.cpp:344-360`；`CosFunction.cpp:629`、`:1799`、`:1847`、`:3379`、`:4357`

**V906（git 唯讀，`D:\HT9045`）**
- origin/main `8b5a91b5`：`HT9011UC_Cpp_V3.33.906.0\JamIniMerge.h`、`WebSecurityJam.cpp`、`WebSecurityJam.h:13-25`、`cSecurity.cpp`、`forms\fSecurity.h`、`forms\fNote_JamCount.cpp:1-62`、`cMyDB.cpp:1668-1670`、`cObserver.cpp:1883-1885`、`Command.cpp:15131`、`KYECFTP\FTPClient_Transfer.cpp:496-497`／`:865-866`、`WebCmdGuard.cpp:28`／`:118`、`tools\wb_serve.cpp:1317`／`:5556`／`:5965`、`tests\CMakeLists.txt:3443-3446`、`tests\test_security_jam_merge.cpp`、`tests\test_security_core.cpp:1-40`、`CosFunction.cpp:811`／`:4438`、`docs\RULINGS_20260926.md:171`／`:351-353`；`D:\HT9045\web\page\Status.Security.html:56`、`D:\HT9045\web\page\ht9045_wire_statussecurity.js`、`D:\HT9045\web\background.html:450`／`:511`
- origin/v906/steven-gpib-widget `c0ea3021`：`HT9011UC_Cpp_V3.33.906.0\EventLogAnalysis\ElaCore.h`、`ElaCore.cpp`、`ElaHub.cpp:36-53`／`:390`、`ElaService.cpp:77-168`、`tests\test_ela_core.cpp:347-440`、`docs\ELA_PORT_LEDGER.md:171`／`:203`／`:237`／`:255-266`／`:340-347`；`D:\HT9045\web\page\eventlog.html`
- blob 比對（ls-tree）：安全頁那一組在 main／review6／gpib-widget 相同；`ht9045_wire_statussecurity.js` 在 review6 是 `87e7516f`（main／gw 是 `4f255e15`）；ELA 那一組 gw 比 main 新。

**計畫與手冊**
- `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\SKILL.md` §5（#20）
- `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-web-conversion-plan.md`
- `D:\HT9045\.claude\skills\ht9045-eventlog-analyzer\references\ela-reports-upload-plan.md`
- `C:\Users\steven\.claude\skills\ops-st02-manager\SKILL.md` §1、§6

**本機資料（只讀）**
- `D:\HT9045\Error\English\JAM0000.dat`（格式、section、鍵分布）
- `D:\HT9045\Error\AlarmCodeList.txt`（3,074 行）
- `D:\HT9045_Log\EventLogTxt\`（UnitName 格式）
