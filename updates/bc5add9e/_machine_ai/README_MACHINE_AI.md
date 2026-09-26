# 給機台端 Claude：更新包 7（GitLab main `bc5add9e`，相對更新包 6 `1e15c4f9`）

> 筆電端 Claude 20260926 16:1x 產生。**先套更新包 3～6，再套這一包。要不要套由 Jimmy 決定。**
> 10 檔，底稿 `base_1e15c4f9\`。一樣排除 OBJROOT 那 4 個檔。

## 內容

| 類 | 檔 | 對機台的影響 |
|---|---|---|
| **安全 PLC 閘**（RULINGS_20260926 第 20／22 條，筆電那一半） | `csystem.cpp`、`cinitial.cpp`、`ckernel.cpp`（都是同一行改寫，行數不變）、`tests/test_plc_gates.cpp`＋`tests/CMakeLists.txt`（新 ctest `PlcGates`） | **SafePlcIO=0（你們現在的值）行為不變**：G12／G14／G23／G-PLC-A／G-PLC-B 都先判 `Enable_PLCSafety_IO`。`bSafePLCThread` 改用 `MyPLC/MyPLC_IO_Modbus.cpp:93` 真的那個（不再是 csystem.cpp 裡恆 false 的替身）；W7a-I3 開機 `InitPLCIO` 照 golden（在 `if(Enable_PLCSafety_IO)` 裡）；ckernel 的 PLC 心跳燈。連結面：MyPLC 的物件現在會進 wb_serve（靜態物件 `PlcComm`，Sim 模式的 socket，不連線、不開執行緒） |
| **網頁權杖**（NB2 R68／R69） | `web/page/ht9045_recipe_client.js:691`、`web/page/motor-access.js:311`、`tools/webprobe/token_idle_selftest.cjs` | ① `motor.access` 被拒（not-operator）**不再自動重送**：重送的 jog 可能落在免權杖的 stop 之後；下一次按鍵會先重拿權杖。你們 `motor-access.js:273` 的 late-ack 補停止還在。② **按住中的 jog 也持有權杖**：原本按住超過 30 秒權杖被還、deadman 停下 jog；golden 是一直 jog 到放開為止 |
| 文件 | INBOX（第 41 列筆電那一半結案、第 54 列 SafePlcIO=1 還差的）、NIGHT_REPORT | — |

## ⚠ SafePlcIO 還不要改 1（INBOX 第 54 列）

程式碼照 golden 了，但端到端還不是 golden：**沒有東西在跑 PLC 輪詢**（`TPLCIOThread::Resume` 是空函式、`bPLCStatusCheck` 沒有呼叫者），
而且 `PlcComm` 的 socket 是 Sim 模式。改 1 的話機台會**永遠處在 EMG**（G-PLC-A）、門永遠不掃描（G-PLC-B）—— 方向是停機，但不是接一台活的 PLC。
另外你們那一半（IO 表的 ePLCbase 列 SnAllSafeDoor／SnAllEMG／SnSafeMode、172.16.8.x 網卡、PLC IP 與暫存器對照）沒有這些，G14 也會每次拒絕 START。

## 在機台上要看的

1. Motor Test 按住 jog 超過 30 秒不放：軸要一直動到放開為止（以前 30 秒左右會自己停）。
2. 開機 log 沒有新的錯誤；`SafePlcIO=0` 時畫面上不會出現 PLC 相關訊息。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證（兩組態全量 gate）：出貨 189 項＝基準 3＋5 Disabled；模擬＝基準 18＋5 Disabled；`PlcGates` 兩組態通過；子檢查 0 差異；
nm：兩組態 wb_serve.exe 有 `B _bSafePLCThread／_bPLCIOEffect／_bIOPowered／_PlcComm`、替身 `PTW6h4` 消失。權杖 selftest 33／33（對照組紅 4 條）。
