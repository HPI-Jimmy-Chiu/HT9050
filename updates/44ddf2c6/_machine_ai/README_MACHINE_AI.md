# 給機台端 Claude：更新包 25（GitLab main `44ddf2c6`，相對更新包 24 `ef83be05`）

> 筆電端 Claude 20260926 23:4x 產生。**先套更新包 3～24，再套這一包。要不要套由 Jimmy 決定。**
> 9 檔（新檔 `cObserver_TimeInfo.cpp`、`cObserver.cpp`／`forms/fObserver.h`／`forms/fGroundMan.*` 同行修改、CMake 兩支、新 ctest、文件），底稿 `base_ef83be05\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼

**觀察頁的測試／Index 時間表照 golden 每次測完更新**（golden `RecordTimeInfo`，cObserver.cpp:1846-2134，290 行）。以前是空樁，所以：
* 觀察頁的時間表一直是空的；遠端 HTGR 209 IndexTime 查詢讀到空字串；
* SECS 的 `TestTime`／`IndexCycleTime`、`RunInfo.dTestTimeSec`、`dTestSec` 都沒更新；
* 每顆 IC 生產紀錄裡的測試時間、SOT／EOT、Ground 欄是空的。

## 你們機台上看得到的差別

* 跑料之後，觀察頁的 Test Time／Index Cycle Time 表有數字了；SECS host 讀得到 TestTime／IndexCycleTime。
* ⚠ 照 golden：`dTestSec` 從此每測完一顆就更新 —— 開 ATC 的機台，ATC 的「測試時間補償」（`ATCInterface.cpp:2241`）會開始用真的測試秒數。
* 若 `Gerneral.ini` 的 D70（Index Cycle Time Record）開著：`D:\HT9045_log\IndexCycleTimeRecord\` 每 10 顆寫一個 csv（golden 同）。

## 在機台上要看的

1. **不會讓馬達動、不改 IO**。
2. 跑幾顆料：觀察頁的時間表每測完一顆更新一次，第 11 列是最新的；遠端查 IndexTime 有值。
3. 開 ATC 的機台：留意 ATC 測試時間補償的行為（以前 `dTestSec` 只有初始值）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份（D70 開著的話加備 `D:\HT9045_log\IndexCycleTimeRecord\`）→ Apply → **重新建置** → ctest 與筆電比（新測試 `RecordTimeInfo`）→ commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查只有新測試不同，system＋config＋IniData 0 變動，`D:\HT9045_Log` 沒被寫。
新測試用固定的開始／結束時間驗算：2.5 秒、index cycle 1.2 秒、跨小時加 60 分鐘都對。**筆電沒有實跑料**，第 2、3 步要機台端驗。
