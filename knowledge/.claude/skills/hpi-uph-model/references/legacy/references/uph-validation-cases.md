> 保存來源：`.claude/skills/ht9045-uph-model/references/uph-validation-cases.md`，main `76dd45f37`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# UPH 公式驗證對照

## 驗證原則

從 `t:\研五_SourceCode\Function\UPH\` 既有 xlsx 收集已知 (條件, 實測 UPH) pair，
套公式重算，誤差需 **< ±3%** 才能視為公式可用。誤差 > 5% 視為公式漏項。

## 已知資料來源

| 檔案 | 內容 |
|------|------|
| `UPH for HT-9xxx series_3(更新版)_New113.04.10.xlsx` | 最新版各機種 UPH 規格 |
| `UPH for HT-9xxx series_3(更新版)_New113.01.31.xlsx` | 對照組 |
| `UPH 詳解.xlsx` | 各段時間拆解 |
| `2X12 UPH估算.xlsx` | 2x12 模式估算 |
| `HT9132 高速版 240Kg 測試資料.xlsx` | 高速版實測 |
| `HT-9046LS_20230224_w_UPH_chart_V2.1.pdf` | 規格 chart |
| `UPH for HT9046LS_Barcode_Cylinder.xls` | 帶 Barcode/Cylinder 影響 |
| `UPH_SOP/UPH SOP教學-2.pptx` | SOP 教學 |
| `UPH詳解/UPH詳解 V2.0.xlsx` / `UPH詳解_V3.0.xlsx` | 早期版本對照 |

## 驗證表（待填）

| Case# | 來源檔案 | 機型 | Package | Tray | Site | TT | 2D | Soak | 實測 UPH | 公式 UPH | 誤差 % | 備註 |
|-------|---------|------|---------|------|------|----|----|----|---------|---------|--------|------|
| V01 | UPH ...4.10.xlsx | HT9046LS | QFP 7×7 | 10×25 | 4 | 10 | OFF | 16 | — | — | — | |
| V02 | UPH ...4.10.xlsx | HT9046LS | QFN 3×3 | 14×35 | 8 | 5 | ON | 0 | — | — | — | |
| V03 | UPH 詳解.xlsx | HT9046LS | QFN 5×5 | 14×35 | 4 | 20 | OFF | 0 | — | — | — | |
| V04 | 2X12 UPH估算 | HT9046 | QFP 14×14 | 6×15 | 24 | 15 | OFF | 0 | — | — | — | |
| V05 | HT9132 高速版 | HT9132 | QFN 4×4 | 14×35 | 16 | 5 | OFF | 0 | — | — | — | |

> Phase 1 第一次跑出 MotorTime CSV 後即可填回此表，作為公式 commit 前的把關依據。

## 公式調整紀錄

| 日期 | 調整 | 原因 |
|------|------|------|
| 2026-05-29 | 初版 | 建立 |
| — | — | — |

## 已知公式缺口（持續記錄）

- Auto Clean 中斷攤提（每 N 顆 IC）
- ATC 升降溫等候（多溫場景）
- Loader empty / Unloader full idle 時間
- Pick Error 重試攤提（與 Vacuum 狀況高度相關）
- Qorvo CONFIGURE 流程的 Sht 延遲

<!-- preserved-content:end -->
