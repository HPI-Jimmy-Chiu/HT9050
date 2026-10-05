---
name: make-ht9045-installer
description: >-
  使用 NSIS 建置 HT9045 機台軟體安裝包（Installer .exe）。適用於：產生 HT9xxx 系列安裝程式、
  修改版本自訂旗標（CustomVersion / HandlerType / BetaVersion）、確認輸出檔案路徑與命名規則、
  完整 build → installer → release note 工作流。
  觸發關鍵字：NSIS, installer, 安裝包, 安裝程式, makensis, HT9045 installer,
  build installer, 打包安裝, 發版流程, 發版, release, 打包, 製作安裝程式.
---

# make-ht9045-installer

使用 NSIS 建置 HT9045 安裝包的完整發版流程，涵蓋 pre-release-check 整合、.res 版本 patch、Release Note 產出。

---

## 工作流程

| 步驟 | 動作 | 參照 |
|------|------|------|
| 0 | 確認版本來源路徑（未指定先詢問） | [workflow.md](references/workflow.md) §Step 0 |
| 1 | 確認 `SOFT_SIMULTE` 已註解 | [workflow.md](references/workflow.md) §Step 1 |
| 2 | **完整執行 pre-release-check**（含 Track A/B/C） | [workflow.md](references/workflow.md) §Step 2 |
| 2-d | Patch `.res` binary 版本號 | [res-version-patch.md](references/res-version-patch.md) |
| 3–5 | **Track A**：複製 EXE → NSIS 建置 → 7z 壓縮 | [workflow.md](references/workflow.md) §Track A |
| 6 | **Track B**：製作 Release Note（與 Track A 平行）| [release-note-rules.md](references/release-note-rules.md) |
| 7 | 輸出確認 | [workflow.md](references/workflow.md) §Step 7 |

⚠️ Step 3 前提：Step 2 整套 pre-release-check 已完成（含 Track B/C 確認）且 Build 通過。

---

## NSI 旗標快速索引

| 旗標 | 預設值 | 切換工具 | 詳細 |
|------|--------|---------|------|
| `CustomVersion` | `""` | `scripts\set_nsi_flags.py` | [nsi-flags.md](references/nsi-flags.md) §CustomVersion |
| `HandlerType` | `"HT9xxx"` | 同上 | [nsi-flags.md](references/nsi-flags.md) §HandlerType |
| `BetaVersion` | `""` | 同上 | [nsi-flags.md](references/nsi-flags.md) §BetaVersion |

```powershell
# 範例：切換為 AMD 客製 Beta；還原用 "" ""
python "D:\HT9045\.claude\skills\make-ht9045-installer\scripts\set_nsi_flags.py" "TF-AMD_" "_BETA"
```

---

## 環境需求

| 項目 | 路徑 |
|------|------|
| NSIS | `C:\Program Files (x86)\NSIS\makensis.exe` |
| NSI 腳本（預設）| `D:\HT9045_Updater_NSIS\NSIS_Script\HT9045_MUI.nsi` |
| NSI 腳本（Win10）| `D:\HT9045_Updater_NSIS\NSIS_Script\HT9045_MUI_Win10.nsi` |
| 版本來源 EXE | `D:\HT9045_Updater_NSIS\HT9045\EXE\HT9045.EXE` |
| 輸出目錄 | `D:\HT9045_Updater_NSIS\` |
| 7-Zip | `C:\Program Files\7-Zip\7z.exe` |
| BCB6 | `D:\ProgramFiles\Borland\CBuilder6\Bin\bcc32.exe` |

---

## 輸出命名規則

```
{CustomVersion}{HandlerType}_V{VERSION}_YYYY.MM.DD_HH.MM{BetaVersion}_Installer.exe
```

範例：
- `HT9xxx_V3.33.901.0_2026.04.09_16.22_Installer.exe`（標準版）
- `TF-AMD_HT9xxx_V3.33.901.0_2026.04.09_16.22_Installer.exe`（AMD 客製版）

---

## 核心禁令

- ❌ **不得用 PowerShell `|` 接 `make`**：BCB6 ilink32 需建 `MAKE0000.@@@`，pipe 接管 stdout 會 Fatal 並刪除 EXE。**必須 `cmd /c "make ... > log.txt 2>&1"`**。
- ❌ **不得跳過 Step 2 直接打包**：即使 EXE 已存在，也必須整套 pre-release-check 完成且確認後才可繼續。
- ❌ **不得在 `SOFT_SIMULTE` 未確認前繼續**：若偵測到未註解，停止並通知使用者，不得自行修改。
- ❌ **不得手動編輯 NSI 旗標**：一律透過 `set_nsi_flags.py` 切換，避免累積多餘注解行。
- ❌ **不得帶入 `/NOTIFYHWND` 參數**：此為 GUI 視窗控制代碼，命令列模式不需要。

---

## References 索引

| 檔案 | 內容 |
|------|------|
| [workflow.md](references/workflow.md) | Steps 0-7 完整指令、Track A/B 平行說明 |
| [nsi-flags.md](references/nsi-flags.md) | CustomVersion / HandlerType / BetaVersion 對照表 |
| [res-version-patch.md](references/res-version-patch.md) | Step 2-d：Patch 1 字串表 + Patch 2 DWORD 腳本 |
| [release-note-rules.md](references/release-note-rules.md) | Track B：路徑規則、檔名規則、Pre-Release 併入規則 |
