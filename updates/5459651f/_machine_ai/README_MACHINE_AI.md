# 給機台端 Claude：更新包 18（GitLab main `5459651f`，相對更新包 17 `d4897dbd`）

> 筆電端 Claude 20260926 20:4x 產生。**先套更新包 3～17，再套這一包。要不要套由 Jimmy 決定。**
> 3 檔（`tools/wb_serve.cpp` 與兩份文件），底稿 `base_d4897dbd\`。一樣排除 OBJROOT 那 4 個檔。

## 這一包是什麼：警報框照 golden 設暫停標記（NB2 R71 C1）

golden `TfNote::FormShow`（警報框一出現）無條件設 `bHandlerPause`、`iHandlerStartCount=0`、`bPauseInMotor`／`bPauseOutMotor`／`bPauseSortMotor`、
`bHangTimePause`、`bSupplyNewICTrayPause`、`bLifterPause[]`／`bAuto2Pause[]`、`lHandlerStopTime`；`FormClose` 設 `bEnterTestIF`、`lHandlerStopTime`、`bInArmNeedToSafePos=false`。
以前移植樹一個都沒設。

### 你們機台上看得到的差別

**用 START（面板或網頁）答掉警報框、機台恢復運轉時**：

* 手臂會照 golden **先回 Z 安全位**再接著走（`bPauseInMotor`，`Motor/mymotor.cpp:5080-5094`；A62 開著時先回原點）。以前是從 `StopAllMotor` 停下的地方直接接著走。
* tester 逾時與 Hang-up 計時照 golden 重算（`bHandlerPause`），框開著的時間不再算進 tester 逾時。

答 PAUSE／RETRY 而不按 START 的情況，以前就正常（MainProc 的暫停分支會補設）。

## 在機台上要看的

1. **確認機台周圍有沒有人**（恢復運轉時手臂會先回 Z 安全位）。
2. 運轉中讓一個警報框出現，用 START 答掉：手臂先抬到 Z 安全位再繼續；不應該馬上跳 tester 逾時。
3. 對照：用 PAUSE 答掉，再按 START，行為跟上面一樣（以前就是這樣）。

## 步驟

同前幾包：Check → EastSun 同意 → 備份 → Apply → **重新建置** → ctest 與筆電比 → commit 回報。
`powershell -NoProfile -ExecutionPolicy Bypass -File <本包>\_machine_ai\check_and_copy.ps1 -Mode Check -Target D:\HT9045\_integ_ioweb`

筆電驗證：golden 7 處逐行對過（都在函式最外層）；兩組態語法檢查 0 錯、全量 gate＝基準（出貨 3＋5 Disabled、模擬 18＋5 Disabled），子檢查 0 差異。
**這條路徑筆電沒有實跑**（要 wb_serve 運轉中、用 START 答掉真的警報框），請機台端照上面第 2、3 步驗。
