# 給機台端 Claude：更新包 28（GitLab main `95c2c26a`，相對更新包 27 `683fafa1`）

> 筆電端 Claude 20260927 01:4x 產生。**先套更新包 3～27，再套這一包。要不要套由 Jimmy 決定。**
> 11 檔（程式 7、文件 4），底稿 `base_683fafa1\`。一樣排除 OBJROOT 那 4 個檔（`.vscode/launch.json` 不在包裡，機台的 F5 設定不會被動到）。

## 這一包是什麼

1. **LOGSINK**（`forms/fMain.cpp`／`fMain.h`／`FormWidgets.h`）：主畫面 `AddShuttleMessage`／`AddAutoCleanMessage` 兩個空函式換成 golden 本體（main.cpp:30228-30261）。
   ⚠ 寫進去的 memo 還是 `TfMainMemo` 替身（不存任何行），所以**機台上沒有差別**，也不會寫 `d:\AutoCleanLogs\`。
2. **OBS-DFM**（`cObserver.cpp`）：觀察頁 Yield 圖帶 golden dfm 的預設值 —— Y 軸 -5～105、上下限框 100／0、32 條線標題 Site Aa～Site Dh。
3. **ENV-BANNER**（`tools/wb_serve.cpp`）：wb_serve 開機時，如果有設「把機台資料轉到別處」的 `W906_*` 環境變數，就逐一印出來（只印，不擋、不改行為）。
4. **ctest 的 log 根目錄**（`tests/CMakeLists.txt`，St02 D5 的 CMake 半邊）：只影響 ctest，每支測試的 `W906_HT9045LOG_ROOT`／`W906_SAVEEVENTLOG_ROOT` 指到 build 目錄裡。

## 你們機台上看得到的差別

* **用 F5「IOWEB(這台)」啟動 wb_serve 時，主控台開頭會多一段 `!!` 開頭的清單**，列出 launch.json 設的 12 個轉向變數與路徑（例如 `W906_IOTABLE_PATH = D:\HT9045\system\IO_Table.csv`）。這是預期的：它讓你一眼看到這一次 wb_serve 讀寫的是哪一份檔。
* 用 `HT9045_Web.cmd` 正式啟動、環境變數沒設時：**什麼都不多印**。如果這時候也看到 `!!` 清單，代表那個 cmd 視窗（或系統環境變數）殘留了轉向變數 —— 請停下來告訴 Jimmy，那一次 wb_serve 讀寫的不是機台正本。
* 觀察頁（Data.Observer）的 Yield 分頁：Y 軸範圍與兩個上下限框有值了。

## 在機台上要看的

1. 用 F5 啟動一次，確認 `!!` 清單列的正好是 launch.json 那 12 個（13 項裡 `W906_IOTIMING_LOG` 是診斷開關，不在清單裡）。
2. 用 `HT9045_Web.cmd` 啟動一次，確認**沒有** `!!` 清單。
3. 觀察頁 Yield 分頁：上限框 100、下限框 0。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查 201 支 0 差異，system＋config＋IniData 0 變動；
wb_serve 實跑兩次（A 設兩個轉向變數 ⇒ 清單剛好兩列；B 對照 ⇒ 不印），跑完照快照還原。
