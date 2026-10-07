> 保存來源：`.claude/skills/bcb_build/references/powershell-cleanup.md`，main `2db43115d`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# PowerShell 視窗清理策略（bcb_build）

## 目的

避免 build 期間產生的 PowerShell 視窗殘留，造成工作環境混亂或資源占用。

## 模式 1：一般模式（建議預設）

只清理有視窗的 PowerShell 程序，保留目前執行中的 shell。

```powershell
$selfPid = $PID
Get-Process powershell -ErrorAction SilentlyContinue |
  Where-Object { $_.Id -ne $selfPid -and $_.MainWindowHandle -ne 0 } |
  Stop-Process -Force -ErrorAction SilentlyContinue
```

## 模式 2：強制模式（進階）

清理所有非當前 PID 的 PowerShell 程序（包含背景程序）。

```powershell
$selfPid = $PID
Get-Process powershell -ErrorAction SilentlyContinue |
  Where-Object { $_.Id -ne $selfPid } |
  Stop-Process -Force -ErrorAction SilentlyContinue
```

## 風險說明（強制模式）

- 可能中斷其他自動化工作
- 可能中斷背景監控或長時間任務
- 建議先用一般模式，再視需要升級至強制模式

## 建議順序

1. 先執行一般模式
2. 確認仍有殘留才執行強制模式
3. build 腳本預設只用一般模式

## 已整合腳本

- `scripts/build_bcb_safe.ps1`

此腳本已內建 `try/finally` 清理流程，build 成功或失敗都會執行視窗清理。
<!-- preserved-content:end -->
