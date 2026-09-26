# 給機台端 Claude：更新包 26（GitLab main `dcd37592`，相對更新包 25 `44ddf2c6`）

> 筆電端 Claude 20260927 00:1x 產生。**先套更新包 3～25，再套這一包。要不要套由 Jimmy 決定。**
> 3 檔（`tests/CMakeLists.txt`、`cprod.cpp` 一行、`docs/NIGHT_REPORT.md`），底稿 `base_44ddf2c6\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼（只影響 ctest，不影響 wb_serve 的行為）

1. **每一支 ctest 都拿到路徑轉向變數**：`tests/CMakeLists.txt` 的全域 ENVIRONMENT 以前只給到那一行之前加的測試，檔尾十幾支（OpModeBody、LastDataSandbox、
   NoteJamCount、AtcBootInit、Timer2TestSeconds、RecordTimeInfo、St02 的 TesterComm_*／MyDB_* …）完全沒有轉向，走到就寫真的 `D:\HT9045_Log`／system。
   現在檔尾用 `cmake_language(DEFER …)` 在目錄結尾統一補上（**需要 CMake 3.19 以上**；筆電是 4.4.3）。
2. `cprod.cpp` 的 ctest 沙盒 helper（`W906_LastDataPath`）找檔名時也認 `/`：以前只認 `\`，遇到正斜線的 `W906_AUTH_PATH` 會算錯檔名（寫不出檔）。正式 wb_serve 不受影響（只有測試行程會被轉向）。

## 你們機台上看得到的差別

* wb_serve 本身：**沒有差別**。
* ctest：檔尾那些測試改寫到 build 目錄下的 scratch，不再碰 `D:\HT9045_Log`／`system`。

## 在機台上要看的

1. 先確認機台的 CMake 版本（`cmake --version`）≥ 3.19；低於的話 configure 會報錯，請告訴筆電改寫法。
2. configure 後看 `<build>\tests\CTestTestfile.cmake`：每一支 `add_test` 後面都有 `ENVIRONMENT`。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新 configure＋建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查跟改之前 0 差異；201 支測試全部帶 ENVIRONMENT；system＋config＋IniData 0 變動。
