# 給機台端 Claude：更新包 13（GitLab main `19844f8e`，相對更新包 12 `dfe09efb`）

> 筆電端 Claude 20260926 19:0x 產生。**先套更新包 3～12，再套這一包。要不要套由 Jimmy 決定。**
> 7 檔，底稿 `base_dfe09efb\`。一樣排除 OBJROOT 那 4 個檔。

## 內容

| 類 | 檔 | 對機台的影響 |
|---|---|---|
| **多分頁的視窗總表不再互相蓋掉**（St01 18:35 報） | `WebBridge/WebBridgeServer.cpp`、`tests/test_wb_server.cpp` | 以前送進 wb_serve 的每個網頁指令 `connId` 都是 0，`ui.windows.put` 把每個瀏覽器分頁都登記成同一條連線。現在每個分頁各自一份；總表本來就是「新鮮的回報蓋過過期的」（你們的 MT-FIX1），所以關掉的分頁過一陣子就不算數，**不會讓 START 卡住**。可能看得到的差別：兩個分頁一個開 Motor Test、一個是主畫面時，以前主畫面的訊框可能把「Motor Test 開著」蓋掉，現在不會 |
| ctest 不寫真實 `config\config.ini` | `cprod.cpp:2085`（一行）、`tests/test_bootstrap.cpp`、`tests/test_lastdata_sandbox.cpp` | `WriteLastDataFile()` 一定會寫 config.ini 的四節（Vibrate_Time、P65_QAMode、SocketContact、O_Count 接觸壽命計數）；只有測試行程會轉進自己的沙盒，**正式 wb_serve 行為不變** |
| 文件 | NIGHT_REPORT、INBOX | — |

## 在機台上要看的（輕量）

1. 全量重編後 ctest：`WB_Server`（84 項）、`LastDataSandbox`（20 項）通過；跑 ctest 前後 `config\config.ini` 與 `system\lastdata*.dat` 的 MD5 不變。
2. 只開一個瀏覽器分頁時行為不變（Motor Test 視窗開／關跟以前一樣）。開兩個以上分頁時，各分頁的視窗狀態現在會分開記；
   若看到「關掉 Motor Test 之後 MainProc 還是暫停」這類情形，把 `ui.windows` 的診斷（`WebWindowRegistryStats`）與各分頁送的訊框回報給筆電 —— 筆電沒有在多分頁下實測過。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled）；`WB_Server` 第 6 節的對照組（把 connId 改回 0）紅 3 條；
system＋config＋IniData 2,372 檔 0 變動。
