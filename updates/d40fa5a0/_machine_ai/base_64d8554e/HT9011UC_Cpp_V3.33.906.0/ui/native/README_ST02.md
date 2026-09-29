# St02 的原生視窗（E-016，v1 只顯示）

> 讀者：Steven、Jimmy、St01（代跑建置與 ctest）。
> 分支：`v906/steven-native-forms-st02`，底是 ST01-E3 的不閃版 `68a7a5f0`（`ui\native\README.md` §3.2 的做法）。
> 樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`。
> 撰寫：St02-E。**沒有在這台 PC 上執行過**，只有編譯；ctest 由 St01 代跑。

## 共通規則（每一頁都一樣）

- **只顯示，而且是構造上做到的：**
  - 視窗檔只 include `<windows.h>`、STL 與 `NativeHost.h`／`NativeGrid.h`。
  - ctest 的測試程式只連視窗檔。連得起來，就表示視窗沒有任何一條路通到運動、IO、卡片或檔案。
- **golden 的指令鈕：**
  - 照列，但全部 `WS_DISABLED`，視窗程序裡也沒有處理它們。
  - 只有 **Exit** 可以按，而且只關這個視窗，**不跑 golden 的 FormClose**。
- **資料：**
  - 跟網頁同一個 C++ 來源，只讀記憶體：監看器的覆蓋掛鉤、`MOT[]`、`Tech`／`Prod` 等。
  - 不讀卡、不經 JSON、不讀檔。
- **更新：**
  - 每 20 ms 比對一次，只重畫變了的格子；沒變就 0 格。每一頁的 ctest 都驗這一點。
  - 靜態欄位只建一次。
- **golden 看起來是顯示、其實會動機台的地方，v1 一律不做**（St02-E2 盤點 §0.1，`D:\AI_TempFile\st02-e2\native\ST02_NATIVE_INVENTORY_20260929.md`）：
  - 開窗送 Motor Power／Servo On；
  - 點選馬達時寫速度到卡片；
  - 燈號函式在警報時停馬達；
  - teach 的 Timer 歸零；
  - ShuttleMove 關窗時把 SystemStart 關掉。

## HW.MotorTest（`NativeMotorTest.*`，ctest `NativeMotorTest_Headless`，快速鍵 Ctrl+Alt+T）

- **左邊：** golden 會列的馬達，照 MotorTestClass 的順序。點一下只換畫面。
- **右上：** 十顆燈。
- **右邊明細：**
  - 命令位置、編碼器位置、Real Speed、Home Offset、各種狀態、Loop 次數；
  - golden 參數表的十項（InitSpeed～Range）；
  - 整頁欄位。
- **整頁欄位**（Jog／平均時間、Motor Power、Lock、C++ 選的軸）：經 `JsonBridge\ChanMotorPoints.cpp:210` 同一行加的取得函式 `W906_NativeMotorTestPage` 讀（筆電 0929 16:0x 同意）。
  - 掛鉤是 `WebMotorAccessLive.cpp` 的 W906_MotorTestPage：在它的鎖裡複製一份，只讀記憶體、不讀卡。
  - 掛鉤還沒裝、或還沒複製過時，照舊顯示「—」並寫原因。
- **留白：** Pressure 分頁（Galil 讀值未移植）、Light Scale、Motor Database。

## HW.ShuttleMove（`NativeShuttleMove.*`＋膠水 `NativeShuttleMoveGlue.cpp`，ctest `NativeShuttleMove_Headless`，快速鍵 Ctrl+Alt+S）

- **兩個編碼器：** 監看器的 encPos。golden 用 `ReadEncoderPos()`，那是讀卡，不叫。沒有值就「—」。
- **教導點：**
  - 8 個照 golden FormShow。
  - latch 機台多 4 個：值是記憶體裡現在的；golden 開窗時會先 `ReadData()` 讀檔，這裡不讀。
  - SPIL 時標「鎖定」。
- **四個格子：** 只有**版面**（欄 × 列）。格子數字 v1 不顯示，因為 golden 算數字之前先跑 `SetTechDataToProd()`，那是寫入。
- **golden FormShow 的顯示規則：** 列成一張表（條碼、latch、模擬版 Retry、Sensor Adj.、Sen. Latch、T.Step、SPIL、單邊 Shuttle）。
  - golden 不顯示的群組與按鈕，這裡也不顯示。
- **序列紀錄（meShuttleMaintain）：** 空白，因為移動／掃描序列還沒移植。
- **跟 golden 不一樣的地方（St02-E2 審查提醒）：**
  - golden 這一頁是開窗那一刻的快照（FormShow 讀一次；latch 那兩個值還先 `ReadData()` 讀檔）。原生視窗是每 20 ms 讀記憶體裡現在的值，所以別的地方改了教導點，這裡會馬上跟著變。
  - golden 只從選單開這一頁；原生 v1 在 wb_serve 啟動時就開（跟其他原生視窗一樣），之後要改成經 `ui.open` 開。
  - latch 機台上 golden 會把 gbScanOutShuttle 的標題改成「Detect In/Out Shuttle device」（:144），原生 v1 沒有顯示這個標題。

## HW.home（`NativeHome.*`，ctest `NativeHome_Headless`，快速鍵 Ctrl+Alt+H）

- **列：**
  - `fHome->HomeClass` 裡顯示的那些，照 golden 版面每 15 列一欄。
  - 名稱＝`labName` 的字（golden `MOT[].NumberAlias`）。
  - 位置＝`edPos` 的字。這就是 golden 那一格顯示的：開頭是 0，某一軸歸零完成時才寫進去。
- **燈號：** 顯示 golden 要給那一格的狀態（0 滅／1 綠／2 紅／3 黃）。
  - 移植樹的 `ShowLed` 現在把它存進 `W906_HomeLedState[]`（筆電 0929 16:0x 同意的 3 行：`forms\fHome.h:278`、`forms\fHome.cpp:487`／`:505`）。
  - 還沒叫過 ShowLed 的格子是 0，同 golden ledHome 一開始的 Value=false。
- **紀錄：** `ListBox1` 的內容，新的在上面。
- **狀態列：** fShow、iHomeStep、fAbort。「Reset OK」（golden Panel2）先顯示「—」，因為移植樹沒有 Panel2。
- **按鈕：**
  - Abort Home 停用。
  - Exit 只關這個視窗，不叫 golden 的 `Close()`：那會把 fShow 清掉，歸零流程就停了。

## HW.teach（`NativeTeach.*`，ctest `NativeTeach_Headless`，快速鍵 Ctrl+Alt+K）——表格版

- **教導點表：**
  - 每一個 TECH_PARA 一列（`forms\fTeachRegistry.cpp`），後面接雙軸的 TECH_TWOPARA。
  - 每一列：頁籤、Key、馬達、`Tech` 裡的值、那一軸現在的位置、teach.ini 的區段，以及畫出來的停用「Set／Go」。
  - 頁籤是從 dfm2rc 的 IR 對出來的，每一列都有對到；可以用頁籤名稱篩選。
- **不照 golden FormShow 決定頁籤／面板要不要顯示**：那一段約 660 行，移植樹是 `[DEP]`（`forms\fTeach.h:432`）。v1 列出全部。
- **列數：** 真的在跑的 TECH_PARA 是 **260**，不是 265。
  - 有 5 行 `TechPara.push_back` 在 `/* */` 註解裡：`forms\fTeachRegistry.cpp:352-358`，跟 golden 906_0625_Steven `uteach.cpp:619-625` 一樣。
  - 執行時還會因機台設定再少一些，開機時 wb_serve 會印實際數量。
  - Top&Bottom AOI 那幾點 golden 從來不呼叫，不列（St02-E2 查證）。
- **右邊的馬達面板（golden pnlMotion）：**
  - 跟著選到的那一列；還沒點之前跟著 C++ 的 ActiveMotorIndex。
  - 顯示：現在位置、編碼器（照 golden：MotorType 1／3 才用編碼器）、十顆燈、Reset Offset、HomeFlag。
  - Lock、速度、Move To 先「—」。
- **按鈕：**
  - 面板上的 13 顆 golden 按鈕都停用。
  - 點選一列只換畫面：golden 會寫速度到卡片，這裡不做。
  - 不跑 Timer1 的歸零。
  - Exit 只關這個視窗。
- **golden 建構時就藏起來的 7 列**（`Visible=false`：TopView Pick／Place／Kit Zup／Kit Z，雙軸 InArm Pick、OutArm Pick、TopView X）：灰字，說明欄註明「golden 建構時隱藏」（St02-E2 審查提醒）。
- **Reset Offset**：顯示 golden 那一格（`edtSetToOffset`）的字；golden 只有從 teach 歸零完成後才填，所以平常是空白（原本 v1 一直顯示 LastHomePos，已改）。
- **v1 沒有：** 吸嘴 Z 偏移表（TechSuckPara）、非馬達的設定（elTeach）、teach 頁籤上的 IO 燈號。
