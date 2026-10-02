# make-ht9045-installer — 完整發版工作流

> 前提：使用者必須明確指定本次原始碼版本路徑（如 `D:\HT9045\HT9011UC_Code_V3.33.903.0_20260417`）。若未指定，**先詢問**，不得自行猜測。

---

## Step 0 — 確認版本來源

詢問範本：
> 請提供本次要打包的：
> 1. HT9045 原始碼版本路徑（例：`D:\HT9045\HT9011UC_Code_V3.33.903.0_20260417`）
> 2. 是否同時更新 GPIB9045 / RS232Standard？若是，請告知版本路徑

---

## Step 1 — 確認 `SOFT_SIMULTE` 已註解（防呆）

```powershell
Select-String -Path "<原始碼路徑>\MachineType.h" -Pattern 'SOFT_SIMULTE'
# 預期：全部行首為 `//`（已被註解）
```

若發現未註解 → **停止流程並通知使用者**，不得自行修改。

---

## Step 2 — 完整執行 pre-release-check

### 2-a. 刪除舊 EXE 與 Obj（防止殘留遮蔽錯誤）

```powershell
Remove-Item "D:\HT9045\EXE\HT9045.exe"   -Force -ErrorAction SilentlyContinue
Remove-Item "D:\HT9045\Obj\*"             -Recurse -Force -ErrorAction SilentlyContinue
```

### 2-b. BCB6 全重建

```powershell
$BCB = "D:\ProgramFiles\Borland\CBuilder6"
$env:BCB  = $BCB
$env:PATH = "$BCB\Bin;" + $env:PATH
cd "<原始碼路徑>"
cmd /c "make -f HT9045.mak -B > build_log.txt 2>&1"
```

> ❌ 禁止使用 PowerShell `|` / `Tee-Object` 包住 `make`。BCB6 ilink32 需建立 `MAKE0000.@@@` 暫存回應檔；pipe 接管 stdout 會導致 Fatal 並刪除 EXE。

Step 2 **完整範圍**（完成條件）：
1. P1–P13 風險掃描 + DFM 解析度檢查（Pattern 範圍以 [pre-release-check/SKILL.md](../../pre-release-check/SKILL.md) §風險模式索引為準）
2. F1–F11 格式化檢查
3. 彙整 findings，標記 clean 或列出風險表
4. 平行啟動 Track A（BCB6 全重建）/ Track B（Pre-Release MD 草稿）/ Track C（語意分析）
5. Track A 完成後確認 Track B / C → 產出完整 Pre-Release Check 報告

### 2-c. 驗證 Build 通過

```powershell
# 確認無真實錯誤
Select-String -Path "build_log.txt" -Pattern "Error E|Fatal" |
  Where-Object { $_.Line -notmatch "WinSocketErrorCode" }

# 確認 EXE 已重新產生
Get-Item "D:\HT9045\EXE\HT9045.exe" |
  Format-Table Name, LastWriteTime, @{N='MB';E={[math]::Round($_.Length/1MB,2)}} -AutoSize
```

**通過條件**：
- `Select-String` 無輸出（0 真實錯誤）
- `LastWriteTime` 為本次執行時間，大小約 28–30 MB
- Pre-Release Risk Check 所有 Track 均已完成確認

若 EXE 不存在 → 連結失敗，**禁止繼續**。
若 Track B / C 尚未確認 → **即使 EXE 已產生，禁止繼續打包**。

### 2-d. Patch `.res` Binary 版本號

詳見 [res-version-patch.md](res-version-patch.md)。

---

## Step 2-d 完成後：Track A 與 Track B 平行執行

| Track | 工作 | 前提 |
|-------|------|------|
| **Track A** | Step 3 → Step 4 → Step 5（依序） | Step 2-d 完成 |
| **Track B** | Release Note 草稿 → 補填 installer 資訊 | Step 2-d 完成；installer 資訊於 Track A Step 5 後補填 |

---

## Track A — Step 3：複製 EXE

```powershell
Copy-Item "D:\HT9045\EXE\HT9045.exe" "D:\HT9045_Updater_NSIS\HT9045\EXE\HT9045.exe" -Force
```

若同時更新 GPIB9045 / RS232Standard：
```powershell
Copy-Item "<GPIB版本>\H9046_32GPIB.exe"   "D:\HT9045_Updater_NSIS\GPIB9045\H9046_32GPIB.exe" -Force
Copy-Item "<RS232版本>\RS232Standard.exe" "D:\HT9045_Updater_NSIS\RS232Standard\RS232Standard.exe" -Force
```

---

## Track A — Step 4：NSIS 建置

```powershell
& "C:\Program Files (x86)\NSIS\makensis.exe" "D:\HT9045_Updater_NSIS\NSIS_Script\HT9045_MUI.nsi"
```

> Win10 版改用 `HT9045_MUI_Win10.nsi`。
> **不要**帶入 `/NOTIFYHWND` 參數。

**輸出位置**：`D:\HT9045_Updater_NSIS\{CustomVersion}{HandlerType}_V{VERSION}_YYYY.MM.DD_HH.MM{BetaVersion}_Installer.exe`

---

## Track A — Step 5：7z 壓縮

```powershell
$exe = (Get-ChildItem "D:\HT9045_Updater_NSIS\*Installer.exe" |
  Sort-Object LastWriteTime -Descending | Select-Object -First 1).FullName
& "C:\Program Files\7-Zip\7z.exe" a "$exe.7z" $exe
```

---

## Track B — Step 6：Release Note

詳見 [release-note-rules.md](release-note-rules.md)。

---

## Step 7 — 輸出確認（Track A + B 均完成後）

```powershell
Get-ChildItem D:\HT9045_Updater_NSIS\*Installer.exe*, D:\HT9045_Updater_NSIS\*Installer.exe.7z |
  Sort-Object LastWriteTime -Descending | Select-Object -First 6 |
  Format-Table Name, LastWriteTime, @{N='MB';E={[math]::Round($_.Length/1MB,1)}} -AutoSize
```
