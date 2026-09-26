# 給機台端 Claude：更新包 8（GitLab main `a6b553ef`，相對更新包 7 `bc5add9e`）

> 筆電端 Claude 20260926 16:3x 產生。**先套更新包 3～7，再套這一包。要不要套由 Jimmy 決定。**
> 59 檔（其中 42 個是 `TesterComm/` 新檔），底稿 `base_bc5add9e\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：Steven02 的測試機通訊（GB 戰役 P1～P5＋P7＋P2c）

golden `H9046_32GPIB` 那支 GPIB 程式與 RS232Standard（TTL 板）翻成 C++，放在 `TesterComm/`（`namespace gpibbridge`），
**這一包起 wb_serve 第一次連上它**（`tools/wb_serve.cpp` 7 個一行 hook：H1 `/api/testercomm`、H2 `--allow-cmd`、H3 開機註冊引擎、
H4 主迴圈、H5 三個阻塞等待迴圈、H6 關站、H7）。

### ⚠ 行為變化（一定要知道）

* **wb_serve 開機約 1 秒會照 golden 自動啟動 GPIB 或 RS232Standard 引擎執行緒**（依工單 `TestIF_File.iTestType`）；找不到 GPIB 橋接（`gpib-32.dll`）時每 10 秒重試，
  不卡住主迴圈，通知送網頁。**不要它就設環境變數 `HT9045_TESTERCOMM=0`**。
* 共用標頭有新增行（Steven02 P2c，照 912 補）：`cmydef.h`／`cmydef.cpp` 3 個全域、`MessageDef.h` 2 個 MSG_CMD、`atester_shims.h` 一個成員、`forms/fLotInfo.cpp` 一行、
  `forms/fMain.*` 的 Tester 成員改經安裝座轉到 THandlerTesterSide ⇒ **要全量重編**。
* Qorvo「Tester Pause」蜂鳴器（ckernel.cpp）照 Jimmy 14:53 裁決**不改**，維持 906「馬上響」。
* `atester.cpp` 四段（P2b(b)）與主畫面 Tester 鈕（P2e）還**不在**這一包（Steven02 的分支上有，還沒代跑）。

### 合併時筆電修的 2 個連結錯（已在這一包裡）

`tools/wb_serve.cpp:2867`（ApiRoute）與 `:6759`（MbWait）在匿名 namespace 裡，函式內的 `extern` 會綁到匿名 namespace ⇒ 連結失敗；
改成 `:439` 檔案層級宣告＋`::` 呼叫。你們若在這兩個函式附近另外加過 hook，照同樣寫法。

## 在機台上要看的

1. 開機後其他功能照常、主迴圈不卡（引擎在自己的執行緒；沒有 GPIB 橋接時每 10 秒重試）。引擎狀態看網頁 `web/page/testercomm.html`（Steven02 P7，經 H1 `/api/testercomm`）；它沒有專屬的 console log（`TesterComm/Handler/TesterCommWiring.cpp:80-82` 只讀 `HT9045_TESTERCOMM`）。
2. 設 `HT9045_TESTERCOMM=0` 重開：引擎不啟動、不裝座（`TesterCommWiring.cpp:80-82`），`testercomm.html` 看得到沒有引擎。
3. 跳一個阻塞框（告警／是否）時，網頁照常；log 沒有因 H5 出錯。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證（兩組態全量 gate，含上面的修正）：出貨 192 項＝基準 3＋5 Disabled、模擬 192 項＝基準 18＋5 Disabled；
新 ctest `TesterComm_IPC`／`_GPIB`／`_RS232`／`_Handler` 兩組態通過；其他子檢查 0 差異；system\ config\ 586 檔 0 變動。
