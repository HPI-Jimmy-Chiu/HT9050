# main.dfm / main.cpp 拆分為獨立 HTML 畫面

> 原始碼為 **BCB6**（後續改用 VC++ 時須另行標注區隔）。

主視窗 `TfMain`（`main.dfm` / `main.cpp`）的 `pgMain` 只有兩個頁：**`tsMain`（實際操作主畫面）** 與
**`tsMotionView`（工程 / MotionView 畫面）**。本專案把 `tsMotionView` 拆解為多個**獨立 HTML 視窗**，
不再全部塞在單一主視窗內：

```
TfMain / pgMain (TPageControl)
├── tsMain          → page/Main.html                 （主操作畫面，手工）
└── tsMotionView    → 拆成以下獨立視窗（檔名統一加 `fMain.` 前綴）：
    ├── gbControlBtn (TGroupBox)              → page/Main.gbControlBtn.html
    └── pgMotionView (巢狀 TPageControl) 每個 TTabSheet 各一頁
```

> **檔名規則**：源自 `fMain`（main.dfm / TfMain）的拆分子頁一律以 **`fMain.` 前綴** 命名
> （`Main.MotorView.html`、`Main.Logs.html`…），以與其他表單（依 unit 命名的 Data.SortCT.html 等）區隴；
> 只有主視窗 `Main.html` 保留原名。data-open id（logs/motorview…）不含前綴，僅 background.html WINDOWS `src` 用實際檔名。

## gbControlBtn（控制按鈕欄）

`main.dfm:10594`，`tsMotionView` 內的垂直控制按鈕欄（144×620）。**release / debug 兩版均顯示**，
但兩版內容不同：

- **操作按鈕（`.opGroup`，HOME/RESET/… 等下表）**：debug 顯示、release 隱藏
  （CSS `html[data-mode="release"] .opGroup{display:none}`）。
- **跳轉按鈕（`.navGroup`，對應 pgMotionView 各 TabSheet）**：兩版均顯示，點擊 `postMessage({open:id})`
  請 background 開對應視窗（尚未建立者有 `if(win)` 容錯）。
- **視窗框**：background.html WINDOWS `ctrlbtn` 設 `noClose:true`（隱藏縮小/關閉鈕）＋`fixed:true`
  （無右下角縮放把手，不可手動調整大小）；標題列仍可拖曳移位。
  （`noClose` 機制：`(cfg.locked||cfg.noClose)?'':'…鈕…'`——`locked` 連拖曳一起禁，`noClose` 僅隱鈕）。
- **debug 高度**：debug 版按鈕多（操作＋跳轉），`ctrlbtn` 另設 `hDebug:890`；`winHeight()` 在
  `MODE==='debug' && cfg.hDebug` 時改用 hDebug（內容 ~866px），避免出現 scrollbar；release 仍用 `h:640`。

| 按鈕（TBtnPanel） | Caption | main.cpp 事件 |
|---|---|---|
| `BtnHome` | HOME | `BtnHomeClick` |
| `BtnReset` | RESET | `BtnResetClick` |
| `BtnOneCycle` | ONE CYCLE | `BtnOneCycleClick` |
| `BtnCleanOut` | CLEAN OUT | `BtnCleanOutClick` |
| `BtnTrayEnd` | TRAY FEED | `BtnTrayEndClick` |
| `BtnAlarmReset` | ALARM RESET | `BtnAlarmResetClick` |
| `BtnPause` | PAUSE | `BtnPauseClick`（TrueColor=clBlue／黃字） |
| `BtnStart` | START | `BtnStartClick`（TrueColor=clBlue／黃字） |
| `gbRemoteControl` → `BtnSTEP` / `BtnT_Start` / `BtnZUpDown` | STEP / T.Start / Z UpDown | `BtnSTEPClick` / `BtnT_StartClick` / `BtnZUpDownClick`（預設 `Enabled=False`→灰態） |

> 另有 `Button1/3/4/5`（`Visible=False` 開發測試鈕）與 `iWhichKitLabel`——不轉出。
> HTML 版顏色比照 dfm：FalseColor `8404992`=#004080、TrueColor `12936728`=#186DC5。

## pgMotionView 各 TabSheet → 獨立 HTML

`main.dfm:11913` 的巢狀 `pgMotionView`（`ActivePage=tsActionView`）共 11 個 TTabSheet：

| # | TabSheet | Caption | HTML 檔 | 主要 dfm 元件 / main.cpp |
|---|---|---|---|---|
| 1 | `tsMotorView` | Motor View | **page/Main.MotorView.html**（已轉） | `Panel17`「Motor Position and Limit Sensor View」＋`cbCheckEncoderEveryTime`；`StringGrid1`(Motor/Current/Target/Speed/Can/L/M/R)＋`StringGrid3`(11 顆狀態 LED：Ready/CW/Home/CCW/EM Stop/Alarm/SoftCW/SoftCCW/S Alarm/In Pos/ServoOn)。**列＝`fMotorTest->MotorTestClass` 中 `Visible==true` 的馬達**（依機台選配動態增減，來源 `uMotorTest.cpp` push_back 清單）；LED 由 `main.cpp UpdateMotorScreen()` 依 `MOT[i].Led[]` 更新。HTML 版資料驅動（`MOTORS[]`＋`en` 旗標），另附「顯示未啟用馬達」toggle 示範 enable 驅動列數 |
| 2 | `tsActionView` | Motion View | **page/Main.MotionView.html**（已轉） | 動作流程視圖（357 個物件）。由產生器 `SUBTREE_JOBS` 從 `main.dfm` 切出該 object 區塊輸出；執行期掛 `motionview-sim.js`，依 BCB6 `cinitial.cpp SetSimuScreenPara()` 的對照（`Sim-scale.json`）讀 `Motor-runtime.json`／`IO-runtime.json` 變更位置與 LED 狀態——詳見 [motion-view.md](motion-view.md) |
| 3 | `tsCommView` | Comm View | **page/Main.CommView.html**（已轉） | `PanelMain7`/`pcCommView`（Style=tsButtons）8 子分頁以 `TPageControl` 還原：Torque（`pal_RS232`＋巢狀 `pgcTorque`：`tsTorqueCommData`(ListBox14/Panel192「Send Data」)／`tsTorqueChart`(`chtTorque` Series1=Arm1紅/Series2=Arm2綠 → 佔位)＋Z1/Z2 讀寫控制 `chkReadTorque1/2`・`edTorue0/1`・`edtReadZ1/Z2`・`btnReadZ1/Z2`・`edtSetZ1/Z2`・`btnSetZ1/Z2`；`grpTorqueFunction` Visible=False 不轉出)、`tsInArm`(`StringGrid2` 8×60)、`TrayMap`(`StringGrid8`/`StringGrid10`)、`Tj`(`MmoTj`/`Memo1`)、`TCPIPLOG`(`Memo2`)、`tsTrayStepMotor`(`mmoTrayStepMotor`)、`tsVibrationMotor`(`mmoVibrationMotor`)、`IndexArmYPos`(`MemoIndexPosLog`＋`palIndexShiftFunction`：`cbArm1/cbArm2/cbBoth`＋`btnRecordSHT/RecordSocket/Save/Clear/SaveMaxMin`) |
| 4 | `tsHeaterView` | Heater View | **page/Main.HeaterView.html**（已轉） | `PanelMain8`；`palHP2View`「HOTPLATE 2」＋`StringGrid4`(4×12)、`palHP1View`「HOTPLATE 1」＋`StringGrid5`(4×12)、`rgHotplateShowMessage`(7 選項：Hot Time/Which Shuttle/Which Kit/Count/Which Row/Which Site/Table HP)；`SaveHPPick`/`btnSaveHPTable` Visible=False 不轉出 |
| 5 | `tsLogs` | Logs | **page/Main.Logs.html**（已轉） | 原 `ScrollBox2` 的 6 個 GroupBox→`TPageControl` 分頁，各一個 Memo：`mmo1`(GPIB Monitor，含 `lblGPIBWND`)、`MemoHome`(Home Action)、`MemoHeaterLog`(Heater On Off)、`EPMemo`(EP Change)、`MemoProductionLog`(Production)、`memoTSD`(TSD time data, TMyMemo Path=D:\HandlerLog\TSD)；**並整合 `tsMNetLog` 的 `mmoMNet` 為第 7 分頁** |
| 6 | `tsShuttleSensor` | Shuttle Sensor | **page/Main.ShuttleSensor.html**（已轉） | `palShtSensor`；`meShuttle2`(Out Shuttle 2)／`meShuttle1`(Out Shuttle 1) 兩 Memo（雙擊 `meShuttle2DblClick`）＋ `cbTestOutShuttleSensor`／`cbShowShuttleSensor`(checked)／`cbShowInShuttleSensor` |
| 7 | `tsRecord` | Record | **page/Main.Record.html**（已轉） | `PanelMain2`；`palCounter`「Counter」＋`sgDebugRecord`(5×9)、`spbClearRecord`「CLEAR」、`memoAutoClean`(Auto Clean)、`btSavelog`「Save Log」、`AseRecordMemo`(ASE 記錄) |
| 8 | `tsTaskList` | Task List | **page/Main.TaskList.html**（已轉） | `sgTaskList`(TStringGrid ColCount=110 / RowCount=62，FixedCols/Rows=1)，可捲動、sticky 首列首行；State Record（`Task_ListWithTime.csv`） |
| 9 | `tsUnloaderInfo` | Unloader Info | **page/Main.UnloaderInfo.html**（已轉） | `sgUnloaderInfo`(TStringGrid 4×12)＋`palUnloaderInfo1`("Unloader")＋`rgUnloaderInfo`([1]Bin/[2]Which Index/[3]Which Site)＋`cbUnloaderinfo`(Auto1~Fix6) |
| 10 | `tsMNetLog` | MNetLog | **已整合至 page/Main.Logs.html**（MNetLog 分頁） | `mmoMNet`(TMemo)——不再單獨成窗 |
| 11 | `ts1` | AOA Info | **page/Main.AOAInfo.html**（已轉） | `Memo3_AOA_IN`/`Memo3_AOA_OUT`(TMemo)＋`Panel14` 各站 AOA X/Y 位置表（`lab{Loader,Hotplate1/2,InShuttle1/2,OutShuttle1/2,Auto1~3,Fix1~3}_X/_Y`） |

## 背景桌面掛載（background.html WINDOWS）

- 現有：`motorview`(Main.MotorView.html——已轉)、`motionview`(Main.MotionView.html——**已轉＋模擬層**)。
- `ctrlbtn`(Main.gbControlBtn.html)：**非** debugOnly（兩版均建立）；`fixed:true`＋`noClose:true`＋`hDebug:890`。
- 已轉（皆 `hidden:true`，由 gbControlBtn 跳轉按鈕開啟）：`logs`(Main.Logs.html)、`shuttlesensor`(Main.ShuttleSensor.html)、`tasklist`(Main.TaskList.html)、`commview`(Main.CommView.html)、`heaterview`(Main.HeaterView.html)、`record`(Main.Record.html)、`unloaderinfo`(Main.UnloaderInfo.html)、`aoainfo`(Main.AOAInfo.html)。
- pgMotionView 全部 TabSheet（含 Motor View、Motion View）已轉為 HTML，不再有實機截圖暫代頁。

## 拆分原則

1. **一個 TabSheet = 一個 HTML 視窗**：`pgMotionView` 的巢狀分頁不在單一頁內切換，改為桌面各自的視窗，貼合 main.cpp 各視圖獨立更新（各自 `OnDrawCell`/`OnTimer`/Memo 附加）。
2. **release/debug 分流**：gbControlBtn 兩版皆建立；操作按鈕（`.opGroup`）僅 debug、跳轉按鈕（`.navGroup`）兩版皆有。純工程/除錯用途的個別 View 可用 `debugOnly:true` 只在 debug 版建立。
3. **命名對齊**：HTML 檔名取 TabSheet Caption 去空白（Motor View→MotorView.html）；元件 id/title 與 dfm 一致並登錄 ComponentMap。
4. **入口**：Main.html Config ▾ 原「Motion View / Monitor View」已移除；改由 gbControlBtn 跳轉按鈕開啟各 pgMotionView 視窗（Motor/Motion/Logs/Shuttle Sensor/Task List…）。
5. **Log 類整合**：原 tsLogs `ScrollBox2` 六段 GroupBox＋tsMNetLog 合併為單一 `Logs.html`，以 `TPageControl`（tabs.js）分頁、各分頁一個 Memo。
