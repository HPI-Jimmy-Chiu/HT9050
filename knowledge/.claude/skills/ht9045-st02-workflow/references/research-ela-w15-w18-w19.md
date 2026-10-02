# 調查：ELA W15／W18／W19 要改哪裡（St02-M 20260927，唯讀調查）

> 裁決（Steven 20260927）：W15＝B（只用逗號分隔、引號包住的算一欄，兩頁共用 parser）；W16＝A（SPIL 照 golden）；W17＝A（寫回照 golden）；W18＝B（修明顯的 golden bug、記偏離）；W19＝B（去重、接受其他存檔週期的檔名）；W23：樣本在 `D:\HT9045_Log\EventLogTxt`（**唯讀**，fixture 要複製進 repo 或 %TEMP%）。
> 行號：V906＝`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`（EventLogAnalysis\ElaCore.*、ElaTables.cpp、ElaHub.*、ElaService.*；tests\test_ela_*.cpp）；golden 分析器＝`D:\HT9045_SVN_TempFile\EventlogAnalyzer\Code\`（Analyzer.cpp 等）。

## 1. Options 開關與分支點

- `struct Options` 在 ElaCore.h:76-84；預設在 ElaCore.cpp:66-70（`bcbCommaText(true)`、`keepGoldenLeaks(true)`）；正式環境用預設建（ElaService.cpp:151）⇒ 改建構子就會改到 wb_serve。
- **#15 `bcbCommaText`**（ElaCore.h:78，預設 true）：只有一個分支 `Analyzer::Split`（ElaCore.cpp:560-563，用在 :653）。現有 `CommaOnlyText`（ElaCore.cpp:152-177）不夠用（見 §3）。ElaTables 不看它；它的 `BcbCommaText` 呼叫（:333、:338、:350、:357、:396）是解 ElaCore 用 `BcbGetCommaText`（:673）寫的列，來回完全一致，不用動。ElaTables.cpp:495（Production_Log）用 golden 的空白→'_' 技巧，只有含引號的列會不同，改不改都可。
- **#18 `keepGoldenLeaks`**（ElaCore.h:80，預設 true）：分支在 `Summary::InitData`（ElaCore.cpp:50-51，:580／:596／:606 呼叫）與清單清除 :607-611。ElaTables 沒有開關，所有 golden 怪行為都無條件（ElaTables.cpp:4-6 註解）。
- **#19**：沒有欄位；檔案規則寫死在 ElaCore.cpp:628-648，VTEST 分支 :631 用 `custCode`（ElaCore.h:82，`SetCustCode` :191，ElaHub.cpp:326 設）。
- **建議開關**：`bcbCommaText` 預設改 false；`keepGoldenLeaks` 改名 `keepGoldenBugs`（預設 false），ElaTables 每個修正都看它；新增 `goldenFileRule`（預設 false）。這樣仍保留一個 golden／oracle 模式給 #23 對照用。

**#18 各項：修或留**

| 項目 | 移植位置（golden） | 判斷 | 理由 |
|---|---|---|---|
| iFailCount 從不歸零 | ElaCore.cpp:34-52（Analyzer.cpp:111-126） | 修 | Fail Count／MTBF／MUBF 每次查詢（含開機那次）都往上加；Hub 整個生命週期只有一個 Analyzer（ElaHub.h:125） |
| By-Unit／By-Function 清單不清 | ElaCore.cpp:607-611 | 修 | By Filter 會顯示之前區間的列 |
| by-hour 測試時間 ×OneHourMS | ElaTables.cpp:573／:582（:1899） | 修 | `dtTestTime` 是「天」的分數，每小時合計小 24 倍；測試時間改用 OneDayMS，每小時 MTBF／MTBA 的期間仍用 OneHourMS（:578-589，golden :1889-1927） |
| 單筆被 "No Record!!" 蓋掉 | ElaTables.cpp:359-360（:2050） | 修 | golden 的 `RowCount==2` 原意是「沒有列」，結果把唯一一筆的時間蓋掉；只有表格真的空時才顯示 |
| Top5Filter 留舊列 | ElaTables.cpp:374-413 | 修 | 沒重查就換篩選會留著舊篩選的代碼；"Top n" 標籤寫在每一列事件上（:393-397）⇒ 清表格、只標彙總列、重算 `iTop5FilterRowCount` |
| Top5 前綴比對 | ElaTables.cpp:423（:2218）＋頁面自己那份 `D:\HT9045\web\page\eventlog.html:131-134` | 修（完全比對） | 真的代碼會撞：WAR1520 對 WAR15206、WAR1519 對 WAR15194 |
| 功能勾選框交叉 | golden 建構子 :257-263 對照 Analyzer.dfm:1161-1210；頁面 eventlog.html:16／:90 | 在頁面修 | 「2D barcode」顯示 Temp、「Temperature」顯示 ESD、「OCR」顯示 2DID、「ESD」顯示 OCR；正確對應：RTC→0、2D barcode→3、Temperature→1、OCR→4、ESD→2、Auto clean→5、By Yield→6 |
| LoadTime 中止 | ElaTables.cpp:474-506 | 修 | 最後一行被截斷（寫到一半當機）會讓畫面留著上一次查詢的 summary（ElaHub.cpp:371 報「Production_Log aborted」）⇒ 跳過壞行或壞檔、記 log，UpdateSgProduction／SetTotalSummary 照跑 |
| ":"+訊息 | ElaTables.cpp:549-554 | 留 | 只影響例外 memo 文字；移植版沒有指標 UB |
| Top5 尾端空白列；"31 Cylinder" 沒有勾選框 | — | 留 | 純畫面／缺功能，不是算錯 |
| G3 O06 每分鐘存；G9 空資料當掉 | ElaReports（HOLD） | 移植時修 | SKILL.md:103 已列 G3 |
| G5 UPH、G7 包含 vs 前綴、G12 `%d%d%d`、G13 hostname | — | 留 | 是規則或客戶檔名 |

## 2. #19：檔名

**golden**：Analyzer.cpp:813-821 路徑要同時含 `yyyymmdd.csv` 與 `EventLogTxt_`；VTEST（915／919）:808-810 要 `yyyymmdd` 與 `_EventLogTxt_`；對每天 × 每個檔跑迴圈（GetEventLogTextToVec :1211-1217 同，但沒有 VTEST 規則）。

**Handler 怎麼命名**：基本名 cprod.cpp:2530-2535：`EventLogTxt`、`EventLogTxt_<ID>`（O15）、`<MT>_<ID>_EventLogTxt`（N10）；SaveType cprod.cpp:2538-2566（還是 `#if 0` GA1-B2）；檔名由 MyStringList.cpp:731-903 產生；資料夾（:749-778）：月檔放年資料夾、日檔 `yyyy\mm`、更短週期 `yyyy\mm\dd`。

| 樣式（MyStringList.cpp） | golden | 樣本 |
|---|---|---|
| `<n>_yyyymmdd.csv`（:873） | 讀 | 336 個 `EventLogTxt_*`＋N10／O15 名 |
| `_RT`／`_FT`／`_TesterOffline`（:859-869） | 跳過 | — |
| `<n>_yyyymmdd HH.csv`（小時、2/4/6/8 h、12 h 方法 0；:801-835） | 跳過 | `2023\09\01\EventLogTxt_20230901 00.csv`、`… 12.csv` |
| `<n>_yyyymmddHH00.csv`，HH＝08/20（:842-850）；2000 那個檔跨到隔天 08:00 | 跳過（VTEST 會讀） | 119 個 HHT-55 檔；`2024\07\01\…202407012000.csv` 到 2024-07-02 07:50 |
| `<n>_yyyymm.csv` 月檔（:879；iO15=8） | 跳過 | `2025\11\EventLogTxt_202511.csv` |
| `AllEventLog\<n>_yyyymmdd.csv`（:586-592；一律日名、只有 LF :599） | **讀 → 算兩次** | `AllEventLog\HT9046LS_JFTH013_EventLogTxt_20250825.csv`＝`2025\08\` 那份去掉 CR（各 284 行） |
| `ByLotID\…_ByLotEventLog.csv`（:378-380）、`JamStat_*`、`SGJamCount\*_RawData.csv`、`JAM_Log\Processed_JAM.csv`（cprod.cpp:4243） | 不會對上 | 維持排除 |

**建議規則**
1. **哪些檔**：維持 golden「路徑含 `EventLogTxt_`」；檔名結尾是 `_yyyymmdd[_RT|_FT|_TesterOffline]`、`_yyyymmdd HH`、`_yyyymmdd HHNNSS`、`_yyyymmddHH00`、`_yyyymm`、`_yyyy` 之一＋`.csv`（大小寫不拘）。
2. **每個檔涵蓋的時間**：日檔 [D, D+1)；`D HH`：[D+HH, D+1)；0800：[D 08, D 20)；2000：[D 20, D+1 08)；依行數上限切的檔：[D, D+2)；月／年檔：整月／整年。跟 [dtStart, dtEnd] 有重疊就選；現有的逐列檢查（ElaCore.cpp:662）照樣精確切。
3. **每次查詢每個檔只讀一次**：取代「每天 × 每檔」迴圈（不然月檔每天算一次）；順序依涵蓋起點穩定排序，主資料夾在 AllEventLog 前。
4. **列去重**：只有會被計入的列需要 key，用 `ct`（ElaCore.cpp:673）；同一個 key 已經從**另一個**檔計過就跳過；同一個檔內的重複照算（golden 行為；Duplicate 欄才是操作員重複的過濾）；提供 `dupRowsSkipped` 計數。這也涵蓋「主資料夾是小時檔、AllEventLog 是日檔」的情況（:589-592）。
5. VTEST 分支（ElaCore.cpp:631-640）變成新規則的子集，拿掉並記偏離。

## 3. #15：兩頁共用一個 parser

**簽名**：header-only `EventLogAnalysis/EventLogCsv.h`：
```cpp
namespace ela { inline std::vector<std::string> SplitEventLogCsv(const std::string& line); }
```
**為什麼 header-only**：`ht9045_ela` 只連 vclcompat 與 nmftp（CMakeLists.txt:1365-1372）；cObserver.cpp 在 `ht9045_sm`（:1924、:2476），sm 從不連 `ht9045_ela`（wb_serve 把 ela 放在群組前面、機台庫不回頭參照 :3434；test_observer_core 連 sm 不連 ela）；放 ela 的 .cpp 會多一條 sm→ela 相依；vclcompat 的定位是「鏡像 BCB6」（:155-158），也不該放那裡；cObserver.cpp 本來就 include 從根目錄算的標頭（:90）。

**規則**：跟 V912 `SplitEventLogCsvLine`／`AddEventLogCsvField`（V906 cObserver.cpp:1351-1385）一樣，檢視頁行為不變：
- 在引號外的逗號切；每個 `"` 切換「在引號內」狀態。
- 原始欄位兩端去掉 <= ' ' 的字元；之後若以 `"` 開頭就去掉那個引號，若最後一個字元是 `"` 也去掉。
- 然後把 `""` 變成 `"`（支援雙引號；真實的 XCOPY 列需要）。
- 引號內的文字原樣保留：`"\t"` 佔位仍是 TAB；`" 08:45:33.712"` 保留前導空白（驗證時會 trim，ElaCore.cpp:376／394）。
- 沒關閉的引號延伸到行尾；結尾逗號產生一個空欄。
- 逐 byte 切對 cp950 與 UTF-8 都安全。

**為什麼要 trim**：真實資料長這樣 `2024-07-01, 20:00:00.093, Process,"\t",…`。現在的 `CommaOnlyText` 會留下 " Process"，破壞 `!= "Process"` 判斷（:763）與 `mapUnitName` 查表（:758），還會在 JAM0000.dat 寫出一個 " Process" 區段 ⇒ `CommaOnlyText` 要被取代，不是只打開。

**ElaCore 改**：`Split` → `SplitEventLogCsv`；:67 預設改 false；`CommaOnlyText`（ElaCore.h:90）拿掉或改成別名；更新 :19-20 的說明。
**cObserver 改**（共用檔，先認領）：:1351-1385 那兩個 helper 改呼叫 `ela::SplitEventLogCsv(asLine.c_str())`，每欄加進 `tsRow`；`ParseEventLogLine`（:1387-1393）保留 SPIL 分支（W16＝A）；:1444／:1488／:1523 呼叫處不動。golden 906_0625 仍用 `CommaText`（:3848／:3892／:3927），這只是把已移植的 V912 切法搬過去。選配：:3395（JamRawData 讀取）也可用。帳本 M4 寫錯了：V912 檢視頁會保留 TAB。

## 4. ctest

**預設翻轉後會失敗的斷言**（test_ela_core.cpp:127-175 與 :202-297 用預設 Options）：

| 行 | 現在 | 翻轉後 |
|---|---|---|
| :145 | stop 5 / 98 s | 6 / 105 s |
| :148 | WAR 1 | 2 |
| :149 | by-day 04/01 stop 58 s | 65 s |
| :156-157 | War／Alarm map 1／3 | 2／4 |
| :161 | slStopList 5 | 6 |
| :164、:172 | writes 10 | 12 |
| :173 | iFailCount 4（stop 5） | 2（stop 6） |
| :174 | By-Area [0] 2 | 1 |
| :233-234 | 第二列舊資料留著 | [0]＝WAR1601、[1]＝WAR2401 |
| :241 | By Area 5 列 | 6 |
| :244-245 | "No Record!!" 蓋掉時間 | `sgByArea[0][1] == "08:10:00"` |
| :277-280 | TotalStopTime `00:00:58` | `00:01:05` |
| :286／:289-290 | 每小時預期 ×3600000 | ×86400000 |
| :291-294 | mmoSummary[17] `00:01:38.000` | `00:01:45.000` |

:83 只有 `CommaOnlyText` 改名才要改。test_ela_hub.cpp（:49 的 `""A""` 列解出一樣）、test_ela_service.cpp（:236／:245 成立；:241 的註解過期）、test_observer_core.cpp:192-226 都不用改。**建議**：那兩個區塊明確釘在 golden 模式（三個開關都設），當 oracle 回歸；新預設另外加區塊。

**新測項**：① parser 用真實列形狀：`, Process,"\t"…,`、XCOPY `""D:\…""`、`,16 System,MES1640,,0,0,"One cycle finish", ,…`、`," 08:45:33.712","16 System",WAR16100,…`、沒關閉的引號；② 來回：每列 `BcbCommaText(BcbGetCommaText(r)) == r`；③ #19：月檔只讀一次、12 h 檔跨午夜、AllEventLog 去重、ByLot 檔從不讀、JamStat／RawData 排除。

**Fixture**：複製進 `tests/fixtures/ela/`，用 compile definition 指過去（同 tests/CMakeLists.txt:4157 的 `W906_WEB_JSON_DIR`）；執行時複製到 `%TEMP%\...\EventLogTxt\yyyy\mm\…`（資料夾名要真實）；repo 有 `core.autocrlf=true`，不要 commit LF 版本——AllEventLog 那份在測試裡去 CR 產生（同 MyStringList.cpp:599）。

**候選檔**（整檔計數，用 Python 模擬兩種切法、不過濾日期）：
- **a.** `2025\09\HT-9016C_PMLD1019_EventLogTxt_20250910.csv`（48 KB、490 行）：有 8 列沒加引號的 `16 System,MES1640`；golden 123 stop 列／0 s／MES 115，新 parser 131／309 s／MES 123。
- **b.** `2025\08\HT9046LS_JFTH013_EventLogTxt_20250825.csv`（31 KB）＋它的 AllEventLog 複本＋`ByLotID\2025\08\25\HT9046LS_JFTH013_600000_20250825130115_ByLotEventLog.csv`：golden 算 80 stop 列（40 × 2），新 40、`dupRowsSkipped`＝40。
- **c.** `2024\07\01\HT-9045W_HHT-55_EventLogTxt_202407010800.csv`（35 KB）＋`…2000.csv`（248 KB，裁成 20:00 前後與午夜到 07:50）：golden 什麼都不讀；新 162＋1314 stop 列、69＋6866 s、JAM 31＋83。另外 `2023\09\01\EventLogTxt_20230901 00.csv`／`… 12.csv`（17＋49 MES 列，golden 0）、`2025\11\EventLogTxt_202511.csv`（11 列；查 2 天還是 11）。

**也要更新**：ElaCore.h:18-27、ElaTables.cpp:4-6、帳本 P1a／P1b「保留的 golden 行為」清單（變成偏離清單）、SKILL.md:100-104 與 :122。
