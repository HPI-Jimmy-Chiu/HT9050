# Timer 排程表計畫書（golden TTimer → C++ web）

> AI(W906-TIMER-TABLE) 20261001。裁決：`docs/RULINGS_20261001.md` 第 39 條。分支 `v906/jimmy-timer-table`（從 main `caae69bb` 開）。
> golden 全表（124 支）：`docs/TIMER_CENSUS.md`／`.tsv`（`tools/timer_census.py` 產生，**不要手改**）。
> 主畫面 14 支逐段現況：`docs/handoff/CENSUS129_20261001/census129_e.tsv` 的 `E-T1-*`／`E-T2-*`／`E-T3-*`／`E-TM-*` 列（整合 session 1001 的普查）。

## 0. 一句話

golden 的 TTimer 靠 Windows 計時訊息觸發，C++ 版沒有這個機制。現在改成**一張排程表**：
golden 的 `Timer3->Enabled=true;` 原樣照翻，排程表每一圈看誰到期就跑誰，跟 golden 一樣在主執行緒。
**核心已完成**：排程表本身、主畫面 14 支 Timer 的物件、開機與每圈的掛點、ctest。
**本體交給工作卡**（§5）：每張卡把一支 golden Timer 的本體逐行翻進來，接上排程表就會照 golden 的時間跑。

## 1. 裁決（使用者 20261001，第 39 條）

| # | 題目 | 裁決 |
|---|---|---|
| 1 | Timer3／Timer8／TimerESD 要不要照 BCB6 補 | **甲：三支整支照 BCB6 翻** |
| 2 | 用排程表，還是沿用「掛在主迴圈那一行尾巴」的舊接法 | **甲：排程表** |
| 3 | 告警框開著的時候這些 Timer 要不要繼續跑 | **甲（第一版）：框開著不跑**（⚠ 跟 9/30 St01 A01 的答覆不同，見 §8 Q2） |
| 4 | 工作量 | **核心寫好後交給其他人員，Kevin 也可以**；計畫書要把所有 Timer 都考慮進去 |
| 5 | 放主迴圈會不會影響生產 | **照建議**：golden 本來就同一條執行緒（MainProc 經 `Synchronize` 跑在主執行緒，golden `uruncontrol.cpp`），放主迴圈 |
| 6 | 「該不該跑」照 golden 每支的寫法，還是統一成「網頁開了才跑」 | **照建議：照 golden 每支的寫法**（排程表只看 Enabled；畫面開著沒由 golden 本體自己的 `if(!fShow) return;` 判斷，fShow 由頁面表給） |
| 7 | 慢的 I/O | **照建議：先全部在主迴圈跑並量時間**，超過 50 ms 的那幾行再搬背景 |

## 2. 現況（量測：main `caae69bb`，golden `HT9011UC_Code_V3.33.906.0_20260618`）

### 2.1 golden 有多少 Timer

**124 支 TTimer、60 張表單、本體合計 15,581 行**（`tools/timer_census.py`）。分類（判斷方法見工具檔頭）：

| 類 | 意思 | 支數 | 翻的時候要注意 |
|---|---|---|---|
| A | 開機是關的，由程式打開（多半是自己表單的 FormShow，也有 `fMain->Start()`） | 72 | **打開它的那一行也要翻**，不然排程表永遠不會跑它 |
| B | 一直開著，本體開頭 `if(!fShow) return;` | 19 | 本體照抄就好，fShow 由頁面表給（`W906_FormShowing`） |
| C | 一直開著、本體不看畫面 —— **程式活著就跑** | 30 | 不可以改成「網頁開了才跑」（例：LotInfo 的 Timer4 每秒跑 AMR 動作） |
| D | 開機是關的，也沒有任何程式打開它 | 1 | 死 Timer（`TfSocketCommunication.Timer1`） |
| ? | 表單不是開機就建（用到才 new），Enabled=True | 2 | 建好之後就跑，要看誰建它（`TfRFID`、`TframeProdInfo`） |

另外兩個要先知道的數字：
- **Interval 小於 50 ms 的有 24 支**（例：主畫面 Timer1／TimerScanKey 30 ms、MotorTest 5 ms、雷射 1 ms、TCP 收資料 1 ms）。排程表一圈最長 50 ms，這些在一圈裡最多跑一次（§7.1）。
- **有重入旗標（`static bool bRun`）的 45 支**；其中「設了 true、中途 return 卻沒放回 false」的有 2 支（§4 第 6 條）：
  主畫面 Timer1 `main.cpp:2928`（N07 工號檢查，**V912 已修**：V912 main.cpp:3013 RogerYang 20260823）、Timer8 `main.cpp:32098`（`bNeedClearFile`，設 true 的唯一那行在 906／912 都被註解掉，目前走不到）。

### 2.2 移植樹已經接上的片段（舊接法，還在原處跑）

| golden | 移植樹 | 怎麼觸發 | 搬到排程表 |
|---|---|---|---|
| 主畫面 Timer1 的十幾段（FlushFlag、UpdateRecordScreen、安全門鎖、大風扇…） | `W906_FlushFlagTick`、`W906_MainRecordTimer1Tick`… | wb_serve 主迴圈／PumpTick，每段自己限速 | 工作卡 T-01（最後做） |
| 主畫面 Timer2 的幾段（測試秒數、State Record、RunInfo） | `forms/fMain_TestSeconds.cpp`、`cStateRecord.cpp`、`FileRW/MainRecord.cpp` | 同上 | 工作卡 T-02 |
| 主畫面 Timer3 的 [A01] 閒置登出 | St01 `FileRW/Main_A01AutoLogout.cpp` | `tools/wb_serve.cpp:5953`（主迴圈）＋ `:7621`（告警框等待中） | 工作卡 T-03 搬 |
| 主畫面 Timer3 的 `bNoNeedLoadSteupFile` | `WebRecipeChange.cpp:680`（換工單結尾做） | 換工單時 | T-03 決定留或搬（不要兩邊都做） |
| 主畫面 Timer8＋TemperatureStorageMinute | St02 MR !48 `v906/st02-s15`：`MainTimer8.cpp`＋派發器 `MainTimersSt02.cpp` | `WebBridgeTags.cpp:605`（PumpTick 裡） | **§8 Q1**：建議本體直接當排程表的 OnTimer、派發器退場 |
| SortCT 的 Timer1 | `W906_SortCTTimer1Tick` | 主迴圈 | 之後 |
| MotorTest 的 Timer1（jog／HOME／LoopMove） | 網頁馬達通道 `W906_MotorAccessTick` | 主迴圈 500 ms | 已由網頁通道取代，**不搬** |
| 告警框／訊息框自己的 Timer1 | `W906_ModalWaitTick`（`tools/wb_serve.cpp:7620`） | 三個等待迴圈 | 已取代，**不搬** |
| LotInfo 的 Timer3 | `TfLotInfo::Timer3Timer`（`forms/fLotInfo.cpp:2360`，本體閘 WB-6） | **沒人叫** | 工作卡（LotInfo） |

還有兩類替身要知道：
- **4 個檔各自定義了一個 `class TTimer`**：`ATC/ATCInterface.h:189`、`BinDisplay/MyBinDisp.h:231`、`OmronLaser/LaserSensor.h:174`、`Automation/uRENESAS_Server.h:194`。它們互不相容，同名不同義，是 ODR 風險（記憶 ht9045-v906-two-tmykitsuck-odr-trap 那一族）。工作卡 T-STANDIN 統一。
- **只有 Enabled 的替身**：`forms/fLotInfo.h` 的 `TfLotInfoTimer`（NetATCTime、TimerERMS、Timer3、LotKeyInTime）。程式會寫它的 Enabled，但沒有人去讀它來決定跑不跑。

## 3. 架構（已完成的核心）

### 3.1 檔案

| 檔 | 內容 |
|---|---|
| `TimerTable.h/.cpp` | 排程表本體（`ht9045_globals`，最底層，不依賴機台碼） |
| `forms/fMain_Timers.h` | 主畫面 14 支 Timer 的成員巨集（Enabled／Interval 照 golden main.dfm）＋本體的宣告 |
| `forms/fMain_Timers.cpp` | 三支空殼本體（T-03／T-08／T-ESD）＋開機接線 `W906_TfMain_TimersBoot` |
| `forms/fMain.h` | `:71` include、`:1270` 放成員巨集（**都佔原本的空行，其他行號一行都沒動**） |
| `tools/wb_serve.cpp` | `:4493` 開機（同一行附加；`:4489` 的空行在 F5 契約探針 `tools/webprobe/f5_contract_probe.cjs` 的抽取範圍裡，放那裡 `WB_F5Contract` 會紅）、`:4661` 每圈、`:5955` 離開主迴圈印統計、`W906_NativeKeepaliveMain` 同步、檔尾 `W906_TimerWdMark`（都佔空行或同一行附加） |
| `tests/test_timer_table.cpp` | ctest `TimerTable`（§6） |
| `tools/timer_census.py` | golden 全表產生器 |

### 3.2 跟 VCL TTimer 一樣的地方（ctest 逐條驗）

| VCL | 排程表 |
|---|---|
| Enabled／Interval 的值真的改了 ⇒ 從那一刻重新數 | 屬性記「改過幾次」，下一圈看到就重新數 |
| 設成同一個值 ⇒ 什麼都不做 | 同 |
| Interval=0 或沒有 OnTimer ⇒ 不跑 | 同（還沒翻的 Timer 就是 OnTimer=0） |
| 程式忙了 30 秒，計時訊息也只來一次 | 同：到期只跑一次、不補跑，下一次排在原本的格子上（錯過整格就從現在起算） |
| OnTimer 丟例外 ⇒ 接住，Timer 照樣在 | 同，另外記次數、印一行 |
| ShowModal 裡同一支又被觸發 | 巢狀呼叫時，正在跑的那一支跳過（記 `reentrySkips`）；golden 本體自己的 `bRun` 旗標照抄，兩層並存 |

### 3.3 在哪裡跑、什麼時候不跑

- **主迴圈每一圈一次**（`tools/wb_serve.cpp:4661`）：在 1203 Poll 之後、處理網頁命令之前。拖曳原生視窗時的保活圈也跑。
- **告警框／是否框／訊息框的等待迴圈裡不跑**（第 3 題＝甲）。框關掉後的第一圈，到期的那幾支各補跑一次（不連發）。⚠ 見 §8 Q2。
- **只准主執行緒**：別的執行緒呼叫會印錯誤並直接 return。golden 的 Timer 全在主執行緒，翻過來的本體沒有任何鎖。

### 3.4 看得到它在做什麼

- 開機印一份排程表（每支一行：開／關、Interval、本體狀態 `no-body`／`stub`／`partial`／`full`）。
- **每 10 分鐘印一次統計**，**離開主迴圈時再印一次**：跑了幾次、平均／最長耗時、最晚晚了多久、超時次數、例外次數。
  例（20261001 筆電 `wb_serve --seconds 20` 實測）：`fMain.Timer3  on 1000 ms stub fires=19 avg=0us max=1us late<=63ms over=0`。
- **單次超過 50 ms** 印 `[TIMER] … 跑了 N ms` 警告（每支前 3 次，之後每 100 次一次）。這就是第 7 題的「先量」。
- 停擺看門狗：每跑一支之前記 `timer: fMain.Timer3`。卡在哪一支，看門狗的報告就印得出名字。
- `ht9045::TimerTableJson()` 已經寫好，網頁要顯示的話接 API 就好（工作卡 T-API）。

## 4. 翻譯規則（每張卡都照這個做）

1. **認領**：照你那條交接管道的規則（TO_STEVEN §0／TO_KEVIN §0…），先在 FROM 檔寫「我接 T-xx、會動哪些檔」。
2. **讀 golden**：`docs/TIMER_CENSUS.tsv` 那一列有本體範圍、Enabled／Interval、所有打開／關掉它的地方（`writes` 欄）。golden 是 `HT9011UC_Code_V3.33.906.0_20260618`（Big5，cp950 解碼）。
3. **本體逐行照抄**成自由函式 `W906_<表單類別>_<Handler>()`，例 `W906_TfMain_Timer3Timer()`。只改這幾種地方：
   - 拿掉 `__fastcall` 與 `(TObject *Sender)`；
   - 本體裡 golden 的表單成員寫成 `fMain->X`／`fLotInfo->X`；
   - 缺相依的段落用 `#if 0 // GATE(W906-TIMER-<卡號>-Gn)：缺什麼` 擋住，並登記在檔頭的閘表。
4. **Timer 物件**：主畫面的已經在門面上（`fMain->Timer3`）。其他表單照 `forms/fMain_Timers.h` 的做法，在那個表單的門面加 `ht9045::TTimerEntry *X = new ht9045::TTimerEntry("fForm.X", <dfm Enabled>, <dfm Interval>, "<golden 範圍>");`。
5. **接本體**：golden 建表單時從 .dfm 綁 OnTimer。那個表單在移植樹建好的地方寫 `fForm->X->OnTimer = &W906_TfForm_XTimer; fForm->X->Status = "full";`（主畫面在 `W906_TfMain_TimersBoot`）。
6. **重入旗標**：golden 開頭的 `static bool bRun=false; if(...||bRun) return; bRun=true;` **照抄**。
   「沒放回 false 的 return」要逐一看：V912 已修的照 V912（例 Timer1 :2928 ⇒ V912 :3013），沒修的照抄，並在那一行註解寫出來。
7. **A 類**：打開／關掉它的那幾行（`writes` 欄）也要翻，否則排程表永遠不會跑它。golden 在 FormShow 打開的 ⇒ 接在那個表單「頁面打開」的邊緣（頁面表 `WebPageTable`）；golden 在 `Start()` 打開的 ⇒ 照翻進 `StartFromWeb`。
8. **畫面欄位**：碰門面上的畫面物件（`labStopTime` 這類）之前取 FormLock（同 `FileRW/MainRecord.cpp` 的 `FormLockGuard`）。網頁 API 的執行緒會同時讀這些物件。
9. **告警／訊息框**照 golden 呼叫（`ShowErrorMessage`／`ShowMyMessage`）。它們會等網頁回答，期間主迴圈停住（golden 也是）。
10. **寫檔、網路路徑**照 golden 寫在本體裡。排程表量到超過 50 ms 的，另開卡搬背景（照 `cStateRecord.cpp` 的背景執行緒做法：主執行緒準備好資料，交給背景寫）。
11. **舊接法的片段要一起搬走**（§2.2）：搬進本體之後，把原本 wb_serve／PumpTick 那一行的呼叫拿掉，不然同一段會跑兩次。
12. **測試**：每張卡一支 ctest，直接呼叫本體、塞假狀態看結果。排程表的時間行為不用再測（ctest `TimerTable` 已經測了）。
13. **收尾**：commit 訊息寫 golden 範圍＋閘表＋沒照 golden 的地方（沒有就寫「無」）；兩組態 gate；FROM 檔 §2 回報。

## 5. 工作卡

> 建議由誰做是筆電的提案；實際照各交接檔的認領為準。**有 MinGW／CMake 的人**才接得了 C++ 翻譯卡（Kevin 那台只有 BCB6，他的卡在 §5.6）。

### 5.1 主畫面三支（第 1 題：最先做）—— 就是 TO_STEVEN 的 S-13／S-14／S-15

| 卡 | Timer | golden | 現況 | 接排程表 | 建議 |
|---|---|---|---|---|---|
| **T-ESD**（= S-13） | TfMain.TimerESD（C 類，1000 ms） | main.cpp:30833-31095，263 行 | St02 已認領；`tESDError` 有 11 個生產者、沒人排空 ⇒ tester 要求停機的 MES0731、GPIB 流程錯 WAR07326／7／8 從不跳 | 本體寫進 `W906_TfMain_TimerESDTimer`（`forms/fMain_Timers.cpp` 的空殼，或你自己的新檔、刪掉空殼），**不用自己限速、不用掛 WebBridgeTags.cpp:605** | St02 |
| **T-03**（= S-14） | TfMain.Timer3（A 類，FormShow 打開，已接在 `W906_TfMain_TimersBoot`） | main.cpp:25106-25742，637 行 | [A01] St01 已做（舊接法）；其餘 8 段沒有驅動（census129_e E-T3-001～009） | 同上；St01 的 A01 改成在本體裡照 golden 的順序呼叫，再拿掉 wb_serve:5953／:7621 那兩個呼叫（跟 St01 對好） | St02（A01 那段跟 St01 對） |
| **T-08**（= S-15） | TfMain.Timer8＋TimerTemperatureStorageMinute（C 類，1000 ms） | main.cpp:32076-32168、:31097-31102 | **St02 MR !48 已寫好**（`MainTimer8.cpp`），用自己的派發器跑 | §8 Q1：MR !48 合了之後，`W906_TfMain_TimersBoot` 綁 `fMain->Timer8->OnTimer=&ht9045::W906_Timer8Timer`、`TimerTemperatureStorageMinute` 同；派發器與 :605 那一行退場 | St02 |

**TimerESD 的前置**：`tESDError` 現在有四份（TesterComm/Handler 的成員、`csystem.cpp:13850` 閘 G3 的 `fMain->tESDError`、`SCK_ART_Remainder.cpp:344` 空巨集、`AutoRetest.cpp:284` 的 `tGPIBMsg`）。先收成主畫面上的一份，`csystem.cpp` 的 G3（MES1712 每圈重報）與 SCK_ART 的 #10 兩個閘可以一起打開。

**TimerESD 的驗證**：golden 第一段是 `#ifdef SOFT_SIMULTE return; #endif`，模擬組態整支不跑（照抄）。要驗它真正的行為，得用真機組態（`-DW906_NO_SOFT_SIMULTE=ON`）建 ctest，或上機台。

### 5.2 主畫面其餘 11 支

| 卡 | Timer | 類 | golden | 要注意 |
|---|---|---|---|---|
| T-01 | Timer1（30 ms） | A（`TfNote` 建構子 note.cpp:220 打開） | :2696-3838，1143 行 | 十幾段已用舊接法在跑（E-T1-*）。最大、最危險（安全門鎖、離子風扇…會動 IO），**最後做**，一段一段搬 |
| T-02 | Timer2 | B（主畫面 fShow） | :20848-21682，835 行 | 幾段已用舊接法在跑（E-T2-*）；FormShow :10179 打開那行跟著搬 |
| T-04 | Timer4 | A（:10575 打開） | :28469-28502 | RTC 視覺程式看門狗 WAR0330（`REAL_TIME_CCD`） |
| T-05 | Timer5 | C | :31210-31297 | ATC 連線看門狗、Tj log |
| T-06 | Timer6 | A（`Start()` :6182／:6192／:6217、`DoHomeProcess` csystem.cpp:10373 打開） | :31299-31548，250 行 | SECS RCMD START 逾時 WAR16110；打開它的地方在 START 流程裡（`StartFromWeb`） |
| T-07 | Timer7 | C | :31550-31638 | VTEST OEE 等 |
| T-09 | Timer9（100 ms） | A | :32678-32691 | ⚠ `DoMoveShuttleSensor()` **會動 Shuttle**（AUTO_SENSOR_INSTALL） |
| T-10 | Timer10（100 ms） | C | :34156-34200 | 振動 LED 顯示；已有部分引用（WebShowBinSelect 等） |
| T-SK | TimerScanKey（30 ms） | A（:9617 打開） | :31994-32025 | 前面板按鍵；先查移植樹的面板鍵掃描（ScanPannelKey）是不是已經在別處做了 |
| T-DLL | TimerDLL（100 ms） | A（:11120 打開） | :33273-33431 | Epson／ATP DLL 遠端命令；`CosFunction.bDLLCommands` |
| （T-08） | TimerTemperatureStorageMinute | C | :31097-31102 | 併在 T-08 |

### 5.3 其他表單：C 類（程式活著就跑，24 支）—— 主畫面之後的第一優先

| 表單 | Timer（Interval，本體行數） |
|---|---|
| TfLotInfo | Timer2（1000，258）、Timer4（1000，7：`CheckAMRAction`／`ReflashInfo`）、ATCTransferFileTime（1000，14） |
| TfLaserSensor | Timer1（1 ms，299）、TimerInArm（1 ms，257） |
| TfCCLink | tmrCanBusSearch（1000，105） |
| TfServerFrm | ProcTimer（100，124）、ShowTimer（1000，4） |
| TATC_InterfaceForm | CommFlagTimer（1000，43）、Timer（10，17） |
| TfNote | TimerFTP（100，56） |
| TACTForm | ACTConnectTimer（50，25） |
| TfGroundMan | Timer1（30，25） |
| TfBarCode | tmr1（100，24） |
| TfDefrostNote | Timer1（500，20） |
| TfAGV | Timer2（10，18）、Timer1（1000，本體沒找到） |
| TfSortCT | Timer2（5000，16） |
| TfFixAICCD | tmrLightControl（100，15） |
| TfAutoAlignment | Timer1（1000，12） |
| TfYieldMonitoring | Timer1（100，11） |
| TfDynamicTemp | Timer1（100，8） |
| TLoadCCD | Timer1（1000，8） |
| TCCDInterfaceForm | Timer2（200，本體沒找到） |

### 5.4 其他表單：B 類（畫面開著才跑，18 支）

很多已經被網頁或等待迴圈取代（MotorTest、告警框、訊息框、Home）。每張卡先回答「已取代／要翻」：

TFTool.Timer1（157 行）、TMyMessageBox.Timer1（220，已取代）、TTrayEditForm.Timer1（22）、TZteach.AutoTimer（95）、
TfCCLink.Timer1（671）、TfConfiguration.Timer1（26）、TfHome.Timer1（5）、TfMotorTest.Timer1（72，已取代）、
TfNote.Timer1（383，已取代）／tmrKeyIn（16）、TfObserver.Timer1（46）、TfOmron.Timer2（55）、TfPrecaution.tm_CheckEditEmpty（30）、
TfShowBinSet.Timer1（7）、TfTeach.Timer1（53）、TfTemperFrom.Timer1（66）、TfTowerLight.Timer1（6）、TfVacuumUnit.tmr1（48）。

### 5.5 其他表單：A 類（由程式打開，65 支，本體 8,530 行）

照表單分卡；**通訊類的 1 ms 輪詢**（TesterTCP、OLP、條碼 CCD、TrayMapping、FixAICCD、AutoAlignment 的 `*ProcessData`）在移植樹多半已改成 socket 執行緒或 TesterComm 泵。每張卡**先查有沒有現成的替代**再決定翻不翻。完整清單見 `docs/TIMER_CENSUS.md`。
大宗：TfBarCode 6 支（1,683 行）、TfTrayMapping 4 支（881）、TfLotInfo 6 支（1,260）、TFormHS 2 支（449）、TATCInterfaceForm 3 支（545）、THGem.Timer1（352）、TCOM2.TimerHPCard（355）、TACTForm.TimerACT（387）。

### 5.6 Kevin 的卡（BCB6，唯讀覆核）—— TO_KEVIN 的 K-04

Kevin 那台沒有 MinGW／CMake，所以他的卡是 **golden 端的覆核**，給 C++ 翻譯的人當依據：

1. **906 對 912 的 Timer 差異**：主畫面 14 支＋§5.3 的 24 支，逐支比 golden 906（0618）與 V912 的本體，列出 V912 修過的地方（像 Timer1 :2928 那種）。翻的人照 §4 第 6 條決定跟不跟。
2. **工具分類的覆核**：`docs/TIMER_CENSUS.md` 的 A／B／C 類，用你對 golden 的了解抽查（特別是 C 類：真的程式活著就一直跑嗎？）。
3. **9050 用不到的段落**：每支 Timer 裡哪幾段只給某些客戶碼或硬體（翻的人可以排後面）。

交付：只寫 FROM_KEVIN.md（或一份 md 放 `v906/kevin-handoff`），附 golden 檔名行號。

### 5.7 基礎建設卡

| 卡 | 內容 |
|---|---|
| T-API | `TimerTableJson()` 接成網頁 API，給除錯頁看每支 Timer 的狀態（St01 的 JSON 通道） |
| T-STANDIN | 4 個 TU 內的 `class TTimer` 替身與 `TfLotInfoTimer` 換成 `ht9045::TTimerEntry` |
| T-FAST | Interval < 50 ms 的 24 支：逐支決定（一圈一次夠不夠；不夠的話主迴圈要不要依「下一個到期時間」提早醒 —— 排程表已經算得出來，但還沒接進主迴圈的睡眠） |
| T-IO | 排程表報告裡超過 50 ms 的那幾行，搬到背景寫檔（第 7 題） |

## 6. 驗收

- **ctest `TimerTable`**（`tests/test_timer_table.cpp`，假時鐘，秒級）：週期、Enabled／Interval 改了才重算、設成同一個值不重算、不補跑、保持相位、OnTimer 裡關掉自己、例外、巢狀呼叫、屬性對拷（vclcompat 拷貝賦值陷阱）、超時記數、別的執行緒不准跑、OnTimer 裡拆掉別支、Find／JSON；
  T14 拿 `docs/TIMER_CENSUS.tsv` 對 `forms/fMain_Timers.h` 那 14 支的 Enabled／Interval —— 有人改了值而 golden 沒變，這裡就紅。
- **每張工作卡自己的 ctest**（§4 第 12 條）。
- **執行時**：wb_serve 主控台的 `[TIMER]` 報告（開機、每 10 分鐘）。驗收看「該跑的有在跑（fires 有在加）、沒有超時」。
- **兩組態 gate**，比對失敗清單（不看數字）。
- **要跑真實檔驗證（例：短跑 wb_serve）**：照 `tools/realfile_guard.py` snap → 跑 → check → restore → drop。⚠ 先確認沒有別的 worktree 的 gate 在跑（真實檔是共用的；20261001 18:41～19:25 筆電這邊的兩次短跑就把整合 session gate b21a 的 ELA_Schedule 弄紅）。`snap` 現在偵測到 ctest.exe 會拒絕，要硬跑加 `--ignore-ctest`，並先跟跑 gate 的 session（ht9045-b0）講好。

## 7. 已知差異與風險

1. **解析度**：排程表一圈最長 50 ms（有網頁命令時提早醒）。Interval 小於一圈的 24 支，一圈最多跑一次。golden Windows 計時器的最小解析度約 10～16 ms，所以 30 ms 的 Timer1 在 golden 大約每 31 ms 一次，這裡大約每 50 ms 一次。T-FAST 卡處理。
2. **告警框開著時不跑**（第 3 題＝甲）。golden 在 ShowModal 期間 Timer 照跑。差異例：框開著 20 分鐘，A01 在框開著時不會登出。⚠ St01 的 A01 現在是框開著照跑（9/30 答覆）—— 見 §8 Q2。
3. **同一圈裡關了又開**：VCL 會從第二次設定那一刻重新數；排程表看到「改過」也重新數，從下一圈起算，最多差一圈。
4. **靜態庫的登記陷阱**：Timer 物件若寫成某個 .cpp 的靜態物件，而那個 .o 沒被任何人引用，連結器會整個丟掉，Timer 就不存在。所以主畫面的 Timer 放在門面成員（`fMain` 建好就有），本體由開機函式接上（wb_serve 有呼叫 ⇒ 一定連進來）。
5. **主畫面門面被建了兩份**：`forms/fMain.cpp:554` 程式啟動時先建一份 TfMain，`tools/wb_serve.cpp:3794-3795` 再建 `TfMainWeb` 並把 `fMain` 指過去（golden 只有一份）。
   兩份各帶 14 支 Timer ⇒ 實測排程表 28 支。`W906_TfMain_TimersBoot` 開頭把不屬於現在 `fMain` 的那 14 支拿下表（只拿下、不刪物件）。
   **其他表單如果也有「建兩份」的情況，接線時照這個做**。
6. **改 `TimerTable.h` 會重編上百個檔**：它經 `forms/fMain_Timers.h` 被 `forms/fMain.h` 引用（主畫面門面要有 Timer 成員）。
   工作卡只改自己的本體檔與 `forms/fMain_Timers.cpp`，**不要動 `TimerTable.h`**；真的要改介面，先在交接檔講、一次改完。
7. **St02 的派發器**（MR !48）跟排程表是兩套排程。兩套同時存在就沒有單一的地方看「誰在跑」—— §8 Q1。

## 8. 待使用者裁決（衝突，筆電 1001 17:xx 整理）

**Q1：St02 的派發器要不要併進排程表？**
- 白話：St02 今天已經把 Timer8 寫好（MR !48），用的是他自己寫的小派發器，在 PumpTick 裡每 1000 ms 叫一次。排程表做的是同一件事，而且多了開關、統計、看門狗。
- 例子：MR !48 合了以後，Timer8 由派發器在跑；Timer3 由排程表在跑。查「現在哪幾支 Timer 在跑」要看兩個地方。
- 選項：
  - **A（建議）**：MR !48 照原樣合（本體不用改）；合了之後筆電把 Timer8／溫度紀錄的本體接上排程表，派發器與 `WebBridgeTags.cpp:605` 那一行退場（先在 TO_STEVEN §4 跟 St02 講好）。S-13／S-14 改寫進排程表的空殼，不再掛 :605。
  - B：兩套並存。St02 那幾支走派發器，其他走排程表。
- 暫行：排程表的分支先不碰 St02 的檔，等你回答。

**Q2：告警框開著時，這些 Timer 要不要跑？**（今天第 3 題選甲，跟 9/30 的答覆不同）
- 白話：9/30 你答 St01「A01 閒置計數在告警框開著時照數、時間到照樣登出」（照 golden），已經做在 `tools/wb_serve.cpp:7621`；St02 的 S-13／S-15 也照這個設計（框開著時 TimerESD、Timer8 照跑）。今天第 3 題你選的是甲「框開著不跑」。
- 例子：框開著 20 分鐘。照 9/30 的做法，A01 在框開著時就會登出；照今天的甲，要等框關了才開始數。
- 選項：
  - **A（建議）照 golden，框開著也跑**：在告警框的等待迴圈（`tools/wb_serve.cpp:7621`，同一行）也呼叫排程表。排程表已經會跳過「正在跑的那一支」（就是跳出這個框的那一支），其他到期的照跑。跟 9/30 的答覆、St02 的設計、第 0 條「照 golden」一致。風險：框裡又跳框（golden 也會）；移植樹的訊息框已有「已經開著就不開第二個」的守衛（`tools/wb_serve.cpp:778`）。
  - B 維持今天的甲：框開著都不跑（A01 也要改回不跑，跟 9/30 的答覆不一致）。
  - C 逐支決定：計數／記錄類照跑，會跳框的不跑（要逐行分類，工作量大）。
- 暫行：排程表現在照甲（等待迴圈不呼叫）；St01 的 A01 照 9/30 的做法不動。
