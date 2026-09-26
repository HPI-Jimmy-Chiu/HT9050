# 給機台端 Claude：更新包 17（GitLab main `d4897dbd`，相對更新包 16 `39bd8f1e`）

> 筆電端 Claude 20260926 20:2x 產生。**先套更新包 3～16，再套這一包。要不要套由 Jimmy 決定。**
> 3 檔（`tools/wb_serve.cpp` 與兩份文件），底稿 `base_39bd8f1e\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：NB2 R70 的 MW-F 與 YN-4

`tools/wb_serve.cpp`，同一行改寫（行數不變）：

| 件 | 改了什麼 | 你們機台上看得到的差別 |
|---|---|---|
| **MW-F** | 警報框／是否框開著時，照 golden `fMain->Timer1Timer`（main.cpp:3125-3128）每一拍跑 `CheckIndexAllSuckICFallDown(true, true)`（`bIndexCheckNoStopVaccum==false` 時） | **框開著等待時也會照 golden 檢查 Index 吸嘴**：REALLY 模式下，吸嘴上應該有 IC 卻讀不到真空，而且 `INDEX_SUCKER_TYPE==1` 時，照 golden 呼叫那個吸嘴的 `Normal()`（會動到真空輸出）。以前框開著時完全不檢查 |
| YN-4 | 面板 Alarm Reset 照 golden 清 `SECS_GEM_PPMUSIC_CONTROL_flag`／`SECS_GEM_PPSIGNALTOWER_CONTROL_flag` | 今天兩者恆 false，沒有差別 |

## 在機台上要看的

1. **確認機台周圍有沒有人**。全量重編、開機照常。
2. 叫出一個警報框、框開著時，若 Index 上有 IC、真空正常，不應該有任何輸出變化。
3. ctest 與筆電比（這個路徑沒有 ctest；SOFT_SIMULTE 下本體是空的）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：兩組態語法檢查 0 錯、全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查 0 差異；system＋config＋IniData 0 變動。
