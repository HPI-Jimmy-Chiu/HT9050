# Track A — 編譯驗證（BCB6 全重建）

> 與 Track B（MD 草稿）、Track C（語意變動分析）平行啟動。
> 也可使用 `bcb_build` 技能取代以下手動步驟。

---

## 全重建腳本

```powershell
$BCB = "D:\ProgramFiles\Borland\CBuilder6"
$env:BCB  = $BCB
$env:PATH = "$BCB\Bin;" + $env:PATH

# ⚠ 必須先清除 Obj 目錄，避免舊 .obj 遮蔽新的編譯錯誤（增量編譯可能隱藏真實錯誤）
Remove-Item "D:\HT9045\Obj\*" -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "Obj directory cleared."

cd "<PROJECT_DIR>"
# ⚠ 必須使用 cmd /c 重導向，避免 PowerShell pipe 干擾 ilink32 建立 MAKE0000.@@@ 暫存檔
cmd /c "make -f <project>.mak -B > build_log.txt 2>&1"
```

> **🚨 重要禁令**：切勿使用 PowerShell 的 `|` 或 `| Tee-Object` 連接 `make`。
>
> BCB6 的連結器 `ilink32` 需要在工作目錄建立 `MAKE0000.@@@` 暫存回應檔；若 stdout 被 PowerShell pipe 接管，該暫存檔建立會失敗，導致 `Fatal: Unable to open file 'MAKE0000.@@@'` 並刪除輸出 EXE。

---

## Build 結果分析

```powershell
$log    = Get-Content "build_log.txt" -Encoding Default
$errors = $log | Select-String -Pattern " Error " -CaseSensitive
$fatals = $log | Select-String -Pattern "Fatal:"  -CaseSensitive
$warns  = $log | Select-String -Pattern "Warning" -CaseSensitive
Write-Host "Errors: $($errors.Count)  Fatals: $($fatals.Count)  Warnings: $($warns.Count)"
```

---

## 報告填欄

報告中請至少包含：

| Build Item | Value |
|------------|-------|
| Build Mode | Full Rebuild (`-B`) |
| Exit Code | 0 代表成功 |
| Compile Errors | 預期為 0 |
| Linker Warnings | Arm 變體間的 Public symbol duplicates 屬預期（inline 函式重複） |

> **註記**：若 linker 出現 `Public symbol defined in both module`（Arm 變體 `.obj`），通常為預期行為，不影響最終執行檔。

---

*最後更新：2026-04-24*
