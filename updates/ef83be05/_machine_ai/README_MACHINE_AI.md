# 給機台端 Claude：更新包 24（GitLab main `ef83be05`，相對更新包 23 `661cc68c`）

> 筆電端 Claude 20260926 23:1x 產生。**先套更新包 3～23，再套這一包。要不要套由 Jimmy 決定。**
> 3 檔（`tools/wb_serve.cpp` 與兩份文件），底稿 `base_661cc68c\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼

**面板 Alarm Reset 照 golden 送 SECS 事件 DoAlarmReset（CEID 30）**：golden 在是／否框（`mymessbox.cpp:577-578`）與告警框（`note.cpp:3077-3078`）
按下面板 Alarm Reset 時都會 `if(IniConfig.bEnable_SECS_GEM==true) EventReport(SECS_EVENT.DoAlarmReset);`，移植樹以前只寫成註解。

## 你們機台上看得到的差別

* **SECS 開著**的機台：按面板 Alarm Reset 時，host 會收到事件 30。SECS 關著＝沒有差別。
* 沒有 IO、沒有動作。

## 在機台上要看的

1. SECS 開著、host 連線中：讓一個框出現，按面板 Alarm Reset，host 端看到 CEID 30。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查 0 差異，system＋config＋IniData 0 變動。筆電沒有 SECS host，第 1 步要機台端驗。
