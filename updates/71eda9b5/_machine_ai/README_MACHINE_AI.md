# 給機台端 Claude：更新包 21（GitLab main `71eda9b5`，相對更新包 20 `df8d1f69`）

> 筆電端 Claude 20260926 22:1x 產生。**先套更新包 3～20，再套這一包。要不要套由 Jimmy 決定。**
> 32 檔（大多是 St02 的測試通訊與 cMyDB），底稿 `base_df8d1f69\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼

St02（Steven 的測試介面那台）第三批，筆電合進 main 並修了三處編不過的地方（St02 那台沒有編譯器）：

1. **cMyDB P1**：wb_serve 開機照 golden 建 log 物件（`LogObjects.cpp`），關機收尾；cMyDB 的 CSV／SaveEventLog 寫入閘解開 ⇒
   **會照 golden 寫 `D:\HT9045_Log\` 底下的 log**（以前不寫）。
2. **On-Line 測試逾時**：`SetTestTimeOutTimer` 照 golden 翻（`a54633b4`），修掉「On-Line 一送 SOT 就判測試逾時」；
   32-site 雙臂的 `hFTestTimeOutDelay` 換成真的全域（`df1a4fca`）。
3. **Qorvo 的 Tester Pause**：照 golden 912，`MaxTestTime>0` 時等逾時才響，Alarm Reset 之後仍在 Pause 就重新計時（`ckernel.cpp` 三行行尾）。
4. 測試結果處理的 R06（fMesSystem）／R09（SECS EventReport）／F2 放開；P6 設定暫定子集（RS232 新選項、Handler-ID 格式等，**St02 標「待使用者確認」**）。
5. 筆電這邊同一輪：觀察頁的 bin 歷史矩陣每顆照 golden 更新；bin 顏色表還原成 golden 的值（以前全黑）。

## 你們機台上看得到的差別

* `D:\HT9045_Log\` 開始有 golden 的 log 檔（SaveEventLog 等）。
* 接真 tester On-Line 跑：不會一送 SOT 就跳測試逾時。
* Qorvo 配方：Tester Pause 不會馬上響，等 `MaxTestTime` 到了才響。
* 觀察頁 bin 歷史、Unload 面板顏色有顏色了。

## 在機台上要看的

1. **這一包不會讓馬達多動**；會改變的是測試逾時的判定與告警響的時機。
2. 套之前**加備** `D:\HT9045_Log\`（整個目錄）。
3. On-Line 接 tester 跑幾顆：不應該一送 SOT 就逾時；真的沒回應時仍要照 `MaxTestTime` 逾時。
4. Qorvo 機台：tester 送 Pause，確認到 `MaxTestTime` 才響、按 Alarm Reset 後重新計時。
5. 開機後看 `D:\HT9045_Log\` 有沒有新檔、內容格式對不對。

## 步驟

同前幾包：Check → EastSun 同意 → 備份（上面第 2 點）→ Apply → **重新建置** → ctest 與筆電比（新測試 `MyDB_CSV_EventLog`、`TesterComm_TestTimeOutTimer`）→ commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查只有兩支新測試不同，
system＋config＋IniData 0 變動，`D:\HT9045_Log` 前後檔案清單相同（ctest 沒寫進去），`D:\GPIB9045\system\general.ini` MD5 不變。
**筆電沒有 tester、沒有 Qorvo 配方**，上面第 3～5 步要機台端驗。
