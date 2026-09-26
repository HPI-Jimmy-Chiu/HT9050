# 給機台端 Claude：更新包 23（GitLab main `661cc68c`，相對更新包 22 `d1bd26ad`）

> 筆電端 Claude 20260926 23:0x 產生。**先套更新包 3～22，再套這一包。要不要套由 Jimmy 決定。**
> 25 檔（St02 的測試通訊與 cMyDB 第四批、`web/page/testercomm.html`、文件），底稿 `base_d1bd26ad\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼（St02 第四批，筆電合進 main，這次 0 個編譯錯）

1. **P6 Q2(a)（Jimmy 裁決）**：GPIB 程式的額外 RS232 port 跟配方走 —— 每份 GPIB 配方第一次跑時，把 `Setup.ini` 的 RS232 值抄進配方的 `Rs232_Data`，之後以配方為準。
2. **P2f**：TTL 卡 2／3 重送測試模式的條件（golden `FTestIF->fShow==false`）改讀網頁視窗總表（以前在網頁架構下永遠成立）。
3. **cMyDB P3**：wb_serve 開機照 golden 跑 `MyDBUpdateDB`，讀 `D:\HT9045\Error\AlarmCodeList.txt` 建 AlarmCode 快取；**檔不存在時會照 golden 重建並存檔**。
4. `web/page/testercomm.html` 更新；Event Log Analyzer 轉 web 的帳本（只有文件）；幾處過時註解。

## 你們機台上看得到的差別

* 開機後 `D:\HT9045\Error\AlarmCodeList.txt` 會被讀（沒有就建一份）。
* GPIB 配方第一次跑時，配方的 `Rs232_Data` 會被寫進 `Setup.ini` 的 RS232 值。

## 在機台上要看的

1. **不會讓馬達動、不改 IO**。會寫的檔：`D:\HT9045\Error\AlarmCodeList.txt`、GPIB 配方的 `Rs232_Data` 那幾個鍵。
2. 套之前**加備** `D:\HT9045\Error\`、使用中 GPIB 配方的資料夾。
3. 開機後看 `AlarmCodeList.txt` 內容合理（警報碼清單）；跑一份 GPIB 配方，看 `Rs232_Data` 抄進去的值跟 `Setup.ini` 一致。

## 步驟

同前幾包：Check → EastSun 同意 → 備份（上面第 2 點）→ Apply → **重新建置** → ctest 與筆電比（新測試 `TesterComm_P6GpibAux`）→ commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查只有新測試不同；system＋config＋IniData、`D:\GPIB9045\system`、`D:\HT9045\Error`、`D:\HT9045_Log` 都 0 變動（ctest 沒寫進去）。
**筆電沒有 GPIB 橋接、沒有 TTL 卡**，第 3 步要機台端驗。
