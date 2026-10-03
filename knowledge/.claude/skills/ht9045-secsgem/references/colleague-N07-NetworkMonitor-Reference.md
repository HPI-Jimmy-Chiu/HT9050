# N07 SECS/GEM 連線監測警報 (NetworkMonitor)

> 客戶：943_JSCC（鴻勁興業）｜需求編號：JSCC-20260327-NetConn
> 釋出版本：V3.33.905.3（修正 905.1/905.2 燈不閃問題）
> 建立日期：2026-06-05｜作者：Steven

---

## 1. 功能概述

Handler 生產中即時監測與 MES 主機之間的 SECS/GEM（HSMS）連線。
連線中斷時發出**不停機**警示（閃爍紅框 + 閃爍塔燈紅燈 + 蜂鳴器 + 事件日誌）；
連線恢復後自動解除。預設關閉，由 INI `bN07_Alarm` 啟用。

---

## 2. 全域旗標（cmydef.cpp / cmydef.h）

| 旗標 | 定義位置 | 用途 |
|------|---------|------|
| `bN07AlarmActive` | `cmydef.cpp` ~4455 | 警報是否作用中（main.cpp 設定，ckernel 讀取驅動塔燈/蜂鳴） |
| `bN07BuzzerSilenced` | `cmydef.cpp` | Alarm Reset 是否已消音（蜂鳴靜音但視覺持續） |

> 兩者皆為**全域**，因為「狀態判定」在 main.cpp `Timer2Timer()`，
> 而「塔燈/蜂鳴實體輸出」在 ckernel.cpp `ShowRunLed()`，需跨檔共享。

---

## 3. 狀態機（main.cpp `TfMain::Timer2Timer()`）

### 啟用條件（缺一不可）
```cpp
bool bN07Enable = (IniConfig.bEnable_SECS_GEM==true &&
                   IniConfig.bN07_Alarm==true &&
                   LastSet.iTester==ON_LINE &&
                   SystemStart==true &&     //B2 : 生產中才偵測(與 ckernel else if(SystemStart) 一致)
                   HGem!=NULL);             //A  : 防 SECS 物件空指標當機
```

### 檢查頻率：首次即時 + 後續節流
| 狀態 | 頻率 | 行為 |
|------|------|------|
| 未警報（`bN07AlarmActive==false`） | **每 1 秒** | 一偵測到斷線**立即**觸發（零延遲） |
| 警報中（`bN07AlarmActive==true`） | **每 10 秒** | 查連線是否恢復（節流，避免日誌洗版） |

### 表現層：邊緣偵測（edge detection）
- `bN07AlarmActive==true` → 每輪維持閃爍紅框/label（防被其他邏輯蓋掉）。
- `bN07LastAlarm==true && bN07AlarmActive==false`（下降邊緣）→ 復原一次：
  `bAlarmBuzzer=false`、`bN07BuzzerSilenced=false`、`LoadTestModePicture()`、寫 Released log。
- 下降邊緣統一涵蓋「連線恢復」與「功能關閉/停機」兩種解除原因。

> **為何用下降邊緣**：解除有兩種原因（`bN07Enable=false` 或連線恢復），
> 分屬不同分支；綁任一分支都會漏掉另一個。用 `bN07LastAlarm` 記住上一輪狀態，
> 在 `bN07AlarmActive` 由 true→false 那一刻統一復原一次，兩種原因都涵蓋。

---

## 4. 實體輸出（ckernel.cpp `ShowRunLed()`）

### RunState + 蜂鳴器分支（在 `else if(SystemStart)` 內）
```cpp
else if(bN07AlarmActive)   //Steven 20260603
{
    RunState=LED_Message;
    if(!bN07BuzzerSilenced) bAlarmBuzzer=true;   //尊重 Alarm Reset 消音
}
```

### 塔燈紅燈閃爍（FlushFlag-gated LED 區塊，`else` 分支）
```cpp
if(bN07AlarmActive)   //Steven 20260605 : SECS disconnect -> blink red (JSCC)
{
    fMain->ledRed->Value    = FlushFlag;   //紅燈閃
    fMain->ledGreen->Value  = false;
    fMain->ledYellow->Value = false;       //⚠ 905.1/905.2 此處誤寫 ledRed=false 導致紅燈恆滅
}
else
{
    // 原 LastSet.MessageLight[RunState][x] 三段式
}
```

### Alarm Reset 消音（ScanKey SnRKAlarmReset / SnFKAlarmReset）
```cpp
if(bN07AlarmActive) bN07BuzzerSilenced=true;   //Steven 20260603
```

---

## 5. UI 紅框閃爍（main.cpp 表現層）
```cpp
Off_lineDisplay->BorderWidth = 20;
Off_lineDisplay->Color       = (FlushFlag)?clRed:clBtnFace;   //隨 FlushFlag 閃爍
labTesterMode->Caption       = "SECS DISCONNECTED";
```
> `Off_lineDisplay` 為**共用**面板（OFF-LINE 閃爍、`LoadTestModePicture()` 也用）。
> N07 要求 ON_LINE，與 OFF-LINE 閃爍狀態互斥。復原**交回** `LoadTestModePicture()`（共用擁有者）處理，不硬寫死值。

---

## 6. 精簡日誌（main.cpp `WriteN07Log()`）
```cpp
static void WriteN07Log(const char *pszEvent)
{
    NewRecordProcess("", pszEvent);   //事件日誌（僅邊緣，不洗版）
    // 另寫 D:\SECS_GEM_LOGS\YYYY\MM_DD\NetworkMonitor.log（與 SECS log 同目錄,沿用既有打包）
}
```
事件字串：`SECS/GEM Connection Lost` / `SECS/GEM Connection Restored` / `SECS/GEM Connection Alarm Released`。

---

## 7. 組態（Config.h / cConfiguration.cpp）

| 項目 | 值 |
|------|----|
| INI 欄位 | `IniConfig.bN07_Alarm` |
| UI 控制 | `fConfiguration->chkN07_3_2`（SECS GEM 群組「SECS GEM Alarm」） |
| 預設 | 關閉 |

---

## 8. 已知陷阱（開發踩過）

| 陷阱 | 後果 | 正解 |
|------|------|------|
| 在 main.cpp 設 `ledRed->Value` 想驅動塔燈 | 被 ckernel `ShowRunLed()` 每輪重算蓋掉，塔燈不動 | 塔燈/蜂鳴一律在 ckernel `ShowRunLed()` 內依 `bN07AlarmActive` 強制 |
| ckernel N07 分支 `ledRed=false`（誤打，應 `ledYellow=false`） | 紅燈恆滅、塔燈不閃（905.1/905.2 bug） | 改 `ledYellow=false` |
| 釋放條件含 `bN07AlarmActive==false` | 狀態機死鎖、計數歸零、log 洗版、永不觸發 | 釋放只看 `bN07Enable`；解除用下降邊緣 |
| 每秒無條件 `bAlarmBuzzer=false` | 踩全域共用蜂鳴旗標、誤關其他警報 | 只在下降邊緣設一次 |
| main.cpp 顯示不檢查 SystemStart、ckernel 在 `else if(SystemStart)` 內 | 停機斷線時 UI 紅、塔燈/蜂鳴不動（不一致） | 採 B2：`bN07Enable` 加 `SystemStart`，兩邊一致 |

---

## 9. 驗收測試

| # | 步驟 | 預期 |
|:-:|------|------|
| 1 | 生產中拔網路線 | 立即閃紅框 + 塔燈紅燈閃 + 蜂鳴 + Lost log |
| 2 | 按 Alarm Reset | 蜂鳴靜音，紅框/塔燈持續閃 |
| 3 | 插回網路線 | 自動解除、復原、Restored/Released log |
| 4 | 斷網期間 | 生產不停機 |
| 5 | INI 開關關閉 | 完全靜默，與舊版一致 |
| 6 | 檢視 `D:\SECS_GEM_LOGS\YYYY\MM_DD\NetworkMonitor.log` | 中斷/解除均有時間戳 |

---

## 10. V906 C++ 移植現況（20261003，St02 卡 ST02-C15；**兩張 MR 都還沒進 main**）

> 本節是 St02 加的；§1～§9 是 Steven 20260605 的 golden 說明，沒動。下面的 golden 行號是本機的 906 對照樹 `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\`（St02 照它翻）；移植樹＝repo `HT9011UC_Cpp_V3.33.906.0\`，行號是各 MR 的 tip。上機清單在 `v906/steven-handoff` 分支的 `docs\handoff\ST02_HUMAN_REVIEW_20260930.md`。
>
> ⛔ **1003 08:2x 更新：!137／!138 先不收進筆電第 50 批**（`HT9011UC_Cpp_V3.33.906.0\docs\NIGHT_REPORT.md` §0 第 79 項等 Jimmy，跟第 78 項一起定；MR 開著、St02 不用重做；`docs\handoff\CHAT_JIMMY.md` 1003 08:2x）。NB2 R177：①**真的 golden 906_0618 沒有這個警報**（筆電重查 0618：`bN07AlarmActive`／`bN07_Alarm`／`WriteN07Log`／`chkN07_3_2`／`NetworkMonitor` 程式 0 筆，只有 0625_Steven 與 V912 有），而 `RULINGS_20261002.md` 第 20 條說 golden 一律是 0618、0625 只供對照；②移植樹主畫面的 `TfMain::ScanKey` 還沒翻（INBOX 86），**按主畫面的 Alarm Reset 消不了這個蜂鳴器**，加上下面說的 `IsConnect()` 永遠 false ⇒ 勾了的機台一按 START 就整批響。NB2 也覆核：N07 沒勾的機台完全不受影響，HT9050 的設定沒開 SECS。

- **main 上沒有 N07**：origin/main `577c41c5` 全樹 `bN07AlarmActive`／`bN07BuzzerSilenced` 0 筆（只有 `Config.h:1512` 的 `bN07_Alarm` 設定鍵）。

**MR !137 `v906/st02-c15-n07`（tip `9b9b18ff`；`fd3ca49b` 是本體，`9b9b18ff` 只多 St02 workflow skill 的文件）**——照 0625_Steven 的狀態機、塔燈、蜂鳴、Alarm Reset、兩種紀錄：
- 新檔 `SECSGEM\N07Alarm_St02.cpp/.h`：兩個全域（0625_Steven `cmydef.cpp:4481-4482`，宣告照 golden 放在 `cmydef.h:4343`，同一行追加）、`WriteN07Log`（golden `main.cpp:20848-20860`，檔名／資料夾／格式／fopen 模式照 golden，含 `"\r\r\n"` 怪癖 G1）、`W906_N07Timer2Tick_St02()`＝golden `main.cpp:20937-20999` 逐行。
- 節拍：St02 的 `MainTimersSt02.cpp` 新增 Timer2 槽（1000 ms，`:136-151`、`:183`；InitialOK 之後才開，同 golden FormShow `:10179`）。
- `ckernel.cpp`（筆電的檔，1003 07:1x 同意的同一行認領）：`:1677` ShowRunLed 的 N07 分支（golden `ckernel.cpp:777-781`，仍在 `else if(SystemStart)` 裡）、`:1769`／`:1813` 斷線紅燈閃（golden `:875-882`、`:926`）、`:3374`／`:3382` Alarm Reset 只消音（golden `:2128`／`:2137`；這兩行 MR !132 也改，後合的重合，N07 那句接在 `bTesterPauseMusic=false;` 後面）。
- 主畫面紅框那一段（golden `main.cpp:20976-20982`、`:20991-20992`）在 `N07Alarm_St02.cpp` 裡照原文留在 `#if 0 // SAFETY-GATE(W906-C15-N07-UI)`（門面沒有 `Off_lineDisplay`／`labTesterMode`）；網頁的紅框改由 MR !138 用 tag 畫。
- ctest `St02_N07Alarm`（`tests\test_st02_n07_alarm.cpp`；紀錄寫到 `W906_SECSGEMLOG_ROOT` 指的 scratch，不碰 `D:\SECS_GEM_LOGS`）。人工審核 B36。

**MR !138 `v906/st02-c15-banner`（tip `dd2dd2d6`，疊在 !137 上、!137 合了才合）**——主畫面紅框＋「SECS DISCONNECTED」：
- C++：`WebBridgeTags.cpp:1156`（machine.state 那一行，筆電同意的同一行追加）`stageBool(snap, "n07.alarm", IniConfig.bEnable_SECS_GEM && IniConfig.bN07_Alarm, bN07AlarmActive);`——[N07] 的 SECS GEM 或 SECS GEM Alarm 沒開＝null。
- 網頁：St02 新檔 `web\page\ht9045_n07_banner.js`（0625_Steven `main.cpp:20974-20993`）：`n07.alarm`＝true 時 `div.statusPane` 加 20 px 框、每 270 ms 紅／底色交替（[W906] 頁面自己的計時器＝golden FlushFlag 30 ms×9）、加自己的紅字元素 `#labTesterMode`「SECS DISCONNECTED」；St01 的 `#labFailAlarmCnt`／`#palMainStatus` 不動、看得到；false／null 完全還原、離線不畫；palMainStatus 的移位縮放不做（[W906] 版面）。`web\page\main.html:626` 同一行追加 include（St01 1003 07:32 同意）。
- ctest `St02_N07BannerPage`（node，含對照組：`W906_N07_BANNER_JS` 指到空檔要紅）。人工審核 B37。

**⚠ 移植樹還沒有真的 HSMS 連線**（筆電 1003 07:1x 同意照 golden）：wb_serve 的 `HGem` 是 `SecsTagPublish.cpp` 那個目錄用的 THGem，socket 從沒開（SECSGEM 以外沒人呼叫 `THGem::DoOpenCommuncation`／`Connect`，St02 1003 `git grep`），所以 `IsConnect()` 永遠是 false。**HSMS 接上之前不要勾 [N07]「SECS GEM Alarm」（`chkN07_3_2`）**：勾了的話，ON-LINE 而且運轉中，第一個運轉 tick 就會報警、而且不會解除（這是實際的連線狀態，不是移植的假象）；這句也要寫進給 EastSun 的上機卡（B36／B37）。第一次發布之前 `HGem` 是 NULL，golden 自己的 `HGem!=NULL`（`main.cpp:20946`）讓警報保持關。

**還開著的題目 Q3（等 Steven，經 ST01-M；JSCC 客戶行為）**：912 不一樣，**沒有翻**——912 `main.cpp:21560` 拿掉 `SystemStart==true`（RogerYang 20260625：不在運轉也會報），912 `ckernel.cpp:800-808` 把蜂鳴分支移出 `else if(SystemStart)`（除了卡料／原點中以外都響）。照 Steven 1003 的常設規則算不算「912 比較好」要他定；常設規則本身在 NIGHT_REPORT §0 第 78 項等 Jimmy，**他回之前不往 912 改**。本節 §3 的啟用條件（含 `SystemStart==true`，B2）就是 0625_Steven 的寫法。整個功能收不收是第 79 項（上面的 ⛔）。
