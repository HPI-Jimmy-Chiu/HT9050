# 給機台端 Claude：更新包 19（GitLab main `04c72d84`，相對更新包 18 `5459651f`）

> 筆電端 Claude 20260926 21:0x 產生。**先套更新包 3～18，再套這一包。要不要套由 Jimmy 決定。**
> 9 檔（測試通訊 7 支程式＋兩份文件），底稿 `base_5459651f\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：St02 的測試通訊 P2f／P7（筆電合進 main＋修一個編譯錯）

1. **P2f：Handler → 測試機橋接程式的設定同步**（golden `TfMain::Timer2Timer` main.cpp:22089-22234＋`SendMessageToGpibProg` :22667-22735）。
   以前移植樹從來沒把 `MSG_CMD_ChangeGpib` 送給橋接程式（GPIB9045／RS232Standard），所以橋接那邊的 Off-Line 模擬、GPIB 位址、bin 數一直是它自己 general.ini 的值。
   現在機台**停著**時（`InitialOK` 且沒在運轉），只要 GPIB 位址、On/Off-Line、bin 數、A10-6 有變、而且測區沒有 IC，就依序送 ChangeGpib → BarCode／Pin1 → 2DIDFormat → FTP；
   測試模式碼有變時送 TestMode，並照 golden 讀回橋接的 ini（`D:\GPIB9045\system\general.ini`、`D:\RS232Standard\system\Setup.ini` 的 `iTesterMode`；鍵不存在時 golden 會補寫預設值）。
   TCP/IP 的 OS 測試機（On-Line）換工單時照 golden 送 `WORKFILE,<檔名>,`、`GETOSSETUP`、`SET2DID,0/1`，中間各睡 100 ms（主迴圈約停 0.4 秒，只在換工單那一次）。
2. **P7：`POST /api/testercomm/<key>?cmd=` 同鍵 400 ms 防重送** —— 同一個 key、同一個 cmd 在 400 ms 內重來就丟掉，回 `{"queued":false,"reason":"repeat within 400 ms"}`（WebSocket 那條本來就有同樣的保護）。
3. **筆電的編譯修正**（`04c72d84`）：St02 那台沒有 MinGW，P2f 有一行 `fShowBinSelect->PageControl1Change(fShowBinSelect)` 編不過；改成傳 `nullptr`（本體不看 Sender，其他三個呼叫點都這樣寫）。

## 你們機台上看得到的差別

* 開著 GPIB 橋接程式（H9046_32GPIB.exe）時，**機台停著、測區沒有 IC** 的情況下改 On/Off-Line、改工單（bin 數或 GPIB 位址不同）：橋接程式那邊的模式／位址／bin 數會跟著變。以前不會。
* TCP/IP OS 測試機 On-Line 時換工單：測試機會收到 WORKFILE。
* 測試通訊頁連點同一顆鈕（400 ms 內）只算一次。

## 在機台上要看的

1. 這一包**不會讓馬達動**（只是通訊）。
2. 開橋接程式，機台停著、測區沒有 IC：切一次 Off-Line → On-Line，看橋接程式畫面的模式有沒有跟著變。
3. 如果是 TCP/IP OS 測試機：On-Line 換一次工單，看測試機端有沒有收到 WORKFILE。
4. 對照：機台運轉中（SystemStart）改設定 → 不應該送（golden 同樣只在停機時送）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份（**加備 `D:\GPIB9045\system\general.ini`**）→ Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），194 個測試子檢查 0 差異，system＋config＋IniData 0 變動，`D:\GPIB9045\system\general.ini` 前後 MD5 相同。
**筆電沒有接橋接程式、也沒有測試機**，上面第 2、3 步要機台端驗。
