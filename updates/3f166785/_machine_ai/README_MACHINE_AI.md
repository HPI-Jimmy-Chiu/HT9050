# 給機台端 Claude：更新包 29（GitLab main `3f166785`，相對更新包 28 `95c2c26a`）

> 筆電端 Claude 20260927 02:4x 產生。**先套更新包 3～28，再套這一包。要不要套由 Jimmy 決定。**
> 底稿 `base_95c2c26a\`。一樣排除 OBJROOT 那 4 個檔（`.vscode/launch.json` 不在包裡）。

## 這一包是什麼

1. **INITMEM**（`cmydef_InitialMemory.cpp` 新檔、`CMakeLists.txt`、`cmydef.cpp`、`tools/wb_serve.cpp:3858`）：wb_serve 開機時照 golden 呼叫 `InitialMemory()`（golden 在 HSys 建構子裡、ReadGeneralIni 之前），放在 `LoadMachineConfig()` 之前。
   以前沒人呼叫。站點行列表 `SiteData[]` 開機時其實已由 St01 的 `SeedSiteData()`（`wb_serve.cpp:4158` 那一段）補上，所以大多數情況看不出差別；
   差別在：表更早就有值、**沒有有效配方路徑時也有值**（那時 St01 那段不會跑），以及開機就啟動「Handler 停機時間」計時（golden 同）。
2. **START 呼叫點普查**（`tools/start_sites_census.py`、新 ctest `START_SitesCensus`）：數字更正成 34／活 30／閘 4（`HandlerGpibMsg.cpp:717` 的 GPIB START 其實被 `#define W906_REMOTE_START_WIRED 0` 閘住）。**活的啟動路徑數沒變**。
3. **觀察頁**（`cObserver.cpp`）：mtRowA～D 照 dfm 的大小（420×180、11×9）；一段錯的註解改正。
4. **Event Log Analyzer 核心**（St02，`EventLogAnalysis/*` 新檔、新 library `ht9045_ela`、ctest `ELA_Core`）：**沒有連進 wb_serve**，`/api/ela` 仍暫停 —— 機台上跑的程式不受影響，只多一支測試。
5. 文件（NIGHT_REPORT、INBOX、DEVLOG、DUET3D 分析的普查數字；repo 根的 CLAUDE.md 不在包裡）。共 19 檔。

## 你們機台上看得到的差別

* **開機時 wb_serve 主控台不再印** `FileRW TestIF_File_SetUp: SiteData[] seeded from golden InitialMemory ...` 那一行（St01 的補值守衛看到表已經有值就不動了）。還看得到那一行 ⇒ 代表開機沒走到 `InitialMemory()`，請告訴 Jimmy。
* 其他沒有差別。

## 在機台上要看的

1. 開機主控台沒有 `SiteData[] seeded` 那一行；LotInfo 頁、Setup 頁的站格數量符合機台配置（跟套之前一樣）。
2. ctest：`InitialMemory`、`START_SitesCensus`、`ELA_Core` 三支新測試都要過（`START_SitesCensus` 需要機台上有 Python3；沒有的話 CMake 不會註冊它，不算失敗）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查只多三支新測試，system＋config＋IniData 0 變動、`D:\HT9045_Log` 0 變動。
