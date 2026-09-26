# 給機台端 Claude：更新包 27（GitLab main `683fafa1`，相對更新包 26 `dcd37592`）

> 筆電端 Claude 20260927 00:3x 產生。**先套更新包 3～26，再套這一包。要不要套由 Jimmy 決定。**
> 5 檔（`common.cpp`、`handlerlog.cpp`、三份文件），底稿 `base_dcd37592\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼（St02 第五批，0 個編譯錯）

1. **cMyDB P4-D5**：`common.cpp` 三處 log 根目錄（`as9045LogPath`、`asSaveEventLogPath`，含 `InitCommonString` 執行期那一句）改成先讀環境變數
   `W906_HT9045LOG_ROOT`／`W906_SAVEEVENTLOG_ROOT`，**沒設就是 golden 原字面 `D:\HT9045_Log`**。只給 ctest 用；機台上不要設這兩個變數。
2. **S93**：`handlerlog.cpp` 檔尾多一支轉接函式 `W906_SaveSiteStatusLog()`（呼叫點在 St01 那支分支，main 還沒有）。

## 你們機台上看得到的差別

* wb_serve：**沒有差別**（沒設環境變數＝golden 路徑）。

## 在機台上要看的

1. 確認機台的系統環境變數**沒有** `W906_HT9045LOG_ROOT`／`W906_SAVEEVENTLOG_ROOT`（有的話 log 會寫到別處）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查 0 差異，system＋config＋IniData 0 變動。
