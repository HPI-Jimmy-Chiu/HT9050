# 給機台端 Claude：更新包 12（GitLab main `dfe09efb`，相對更新包 11 `10936285`）

> 筆電端 Claude 20260926 18:2x 產生。**先套更新包 3～11，再套這一包。要不要套由 Jimmy 決定。**
> 6 檔，底稿 `base_10936285\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：ctest 不再寫到機台的 `system\lastdata*.dat`

**正式程式（wb_serve）的行為不變。** 只有測試執行檔會被轉向。

| 檔 | 改了什麼 |
|---|---|
| `cprod.cpp` | `ReadLastDataFile`／`WriteLastDataFile` 裡 15 處寫死的 `D:\HT9045\system\lastdata*.dat` 同一行改成 `W906_LastDataPath("<原字串>")`；檔尾 20 行是它的本體。**沒有被轉向時原封回傳 golden 的字串**（連 `d:`／`D:` 大小寫都不變） |
| `tests/test_bootstrap.cpp` | 只連進測試執行檔：`main()` 之前在測試的工作目錄（build dir）建 `w906_ctest_lastdata_<PID>\`，複製 `system` 的三個 lastdata 檔當種子，設 `W906_CTEST_LASTDATA_DIR_<PID>`；正常結束時刪掉 |
| `tests/test_lastdata_sandbox.cpp`、`tests/CMakeLists.txt` | 新 ctest `LastDataSandbox` |
| 文件 | NIGHT_REPORT、INBOX |

變數名綁行程 PID，所以外面設不到正式的 wb_serve（跟 `--dry` 退場的理由一樣：正式程式不能有「看起來存成功、其實寫到別處」的開關）。

### ⚠ 你們跑 ctest 前還是要知道

`WriteLastDataFile()` 除了 lastdata，**一定會寫 `config\config.ini`**（Vibrate_Time、P65_QAMode、SocketContact；預設參數再加 O_Count 接觸壽命計數）。
這一包**沒有**圍住那一條，現在也沒有別的 ctest 走到 `WriteLastDataFile`。筆電接下來要翻的 `UpdateMainOperateMode` 會走到它，
那一包之前會先把 config.ini 那條也圍好。

## 在機台上要看的（輕量）

1. 全量重編後 ctest：`LastDataSandbox` 通過（20 項）；跑完 build dir 的 `tests\` 底下沒有 `w906_ctest_lastdata_*` 殘留。
2. 跑 ctest 前後 `system\lastdata.dat`／`lastdata_backup.dat` 與 `config\config.ini` 的 MD5 不變。
3. wb_serve 開機照常讀到上次的工單（正式程式沒有轉向）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新 configure＋全量建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled）；`LastDataSandbox` 兩組態 20／20（含對照組：沒有那個變數就回 golden 字串原樣）。
