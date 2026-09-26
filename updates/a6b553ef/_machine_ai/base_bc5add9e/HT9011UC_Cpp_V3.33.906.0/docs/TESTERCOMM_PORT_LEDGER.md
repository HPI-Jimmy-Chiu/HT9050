# Tester 通訊（GPIB／DIO／RS232／TCPIP）移植帳本

計畫：`docs/GPIB_20260926_INTEGRATION_PROPOSAL.md`（skill `ht9045-gpib-bridge` §8）。本檔記錄翻譯基準、每個階段做了什麼、刻意偏離與待辦。

## 基準

| 來源 | 版本 | 用途 |
|---|---|---|
| GPIB 橋接程式 | `D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525`（H9046_32GPIB.exe） | GpibEngine（P1） |
| RS232／TTL 橋接程式 | `D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410`（RS232Standard.exe） | Rs232Engine（P4） |
| golden Handler | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy` | `OnMyCopyMsg`、atester 結果路徑、TesterTCP（P2、P5） |

BCB 線（BCB Handler ⇄ BCB 橋接 exe）照舊維護，與 C++ 線互不相接（裁決 5）。兩支橋接程式之後改版，都要回來這裡對一次差異。

## 裁決（使用者 20260926，全文在計畫 §7）

1 IO 通訊即時、畫面可略延遲；2 非 9045 機型全帶；3 補搬帳本；4 四種通訊一併整合、DIO 走 RS232Standard；5 BCB 接 BCB、C++ 接 C++；6 設定統一在 cTesterIF；7 `atester.cpp` 直接讀 DIO 卡的分支不翻；8 GPIB 內建的 RS232 是 AMD／ATC 輔助線，留在 GpibEngine；9 GPIB／RS232／TCPIP 共用一條執行緒；10 32 站 `makeDutPanel` 共用；11 通訊執行緒必須與機台執行緒分開、不能互相干擾。

## 階段紀錄

### P0 契約凍結＋骨架（20260926）

- `MessageDef.cpp`：`GPIBVersion`／`RS232Version` 由 `12.13.883` 改為 `12.13.905.0`（與 GPIB 橋接 V12.13.905.0 相同；`GPIBVersionCheck` 仍 12.13）。`MessageDef.h` 的封包結構三邊逐行相同（GPIB 與 RS232 的 `MessageDef.h` diff 為 0，V906 只多 CCD 結構），不動。全樹沒有其他地方比對舊字串。
- 新目錄 `TesterComm/`（只用 `WebBridge/Sync.h` 與 Win32，不連任何機台程式碼）：
  - `SyncMailbox`：`SendMessage(WM_COPYDATA)` 的行程內替身。送方阻塞到對方處理完；等待期間繼續處理送給自己的請求，所以雙向巢狀不會死結；處理函式只在擁有該邊的執行緒上跑。另加逾時（預設 5 秒）與 `Reset()`。
  - `TesterEngine`：引擎介面（`Start`／`RunOnce`／`OnHandlerMessage`／`Stop`／`IsUp`），以及對應 golden `TTL_MODE`／`GPIB_MODE`／`RS232_MODE`／`TCP_IP_MODE` 的列舉。
  - `TesterCommThread`：唯一的一條通訊執行緒（裁決 9）。迴圈是「處理信箱 → `RunOnce()` → 等信箱事件或 `RunOnce` 要的毫秒數」；引擎丟出的例外被接住並計數，執行緒繼續跑；心跳計數給 watchdog 用。
  - `TesterCommHub`：依 `iTestType` 只啟動一個引擎；換類型時照 golden `CloseGpibProgram()`→`RunTestProgram()` 的順序先停舊的（join）、清信箱、再起新的；沒註冊的類型就不起引擎。
- `CMakeLists.txt`：新 target `ht9045_testercomm`（尚未連進 wb_serve）。
- ctest `TesterComm_IPC`（`tests/test_testercomm_ipc.cpp`）：啟動、同步送、兩層雙向巢狀且順序與 SendMessage 相同、逾時後送方 250 ms 內放行且晚到的完成被釋放、引擎送回 Handler、兩種例外、執行緒身分隔離、換類型、關機。
- ⚠ **未編譯、未跑 ctest**：STEVEN-NB3 沒有 MinGW／CMake。請在有工具鏈的機器跑 `build.bat gate`，比對 ctest 失敗清單（常駐 5 個）加上 `TesterComm_IPC` 通過。

## 刻意偏離 golden（累積）

| 位置 | 偏離 | 理由 |
|---|---|---|
| `SyncMailbox` | 加了逾時與 `Reset()` | golden 的 SendMessage 會無限等；裁決 11 要求通訊端卡住時不能拖住機台執行緒 |
| `TesterCommThread` | 接住引擎例外並繼續跑 | 同上；golden 的 VCL 由 `Application->OnException` 接 |
| TcpEngine（P5 起） | 解碼結果經信箱交 tick 寫入 | golden 在主執行緒直接寫 `fMain->tTestResult`；一條通訊執行緒的規則要求改走信箱 |

## 待辦（P1）

- `GpibEngine`：`H9046_32GPIB` `Main.cpp` 的非 UI 部分去 VCL（`TestGPIB`、`ProcessStatusString*`、`SetParameter`、`MyGPIBWrite`、橋接端 `OnMyCopyMsg`、`ProcessAddress`、`ProcessMessage`），`static` 區域變數改成員。
- `IGpibDriver`：`LoadLibrary("gpib-32.dll")` 取 9 個 NI 函式與 `ThreadIbsta` 系列；`SimGpibDriver` 餵腳本給 ctest（腳本來源：`D:\GPIB9045\.github\skills\gpib-command-list`）。
