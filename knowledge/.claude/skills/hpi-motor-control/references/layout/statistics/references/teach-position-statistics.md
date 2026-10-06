> 保存來源：`.claude/skills/ht9045-motor-spatial-layout/references/teach-position-statistics.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# HT9045 Teach Position Statistics — 索引

> **自動產生**：來源 `D:\Users\steven\Desktop\Hang Up\` 各機台 state record。
> **產生腳本**：`AI_TempFile/_collect_teach_stats.py`
> **產生日期**：2026-04-20  &nbsp;&nbsp; **樣本數**：401 筆 state record / **群組數**：41

## 分組依據

| 維度 | 角色 | 說明 |
|------|------|------|
| 機台型號 (HT Model) | **主分組 Key** | 不同型號結構不同，teach 値不可混用 |
| 機台群組 (A~E)      | **頂層分類**   | 相同群組機器結構相近，可互為參考 |
| HP 位置型 (HPP)     | **次分組 Key** | 0=窄HP（HP1Y≈-42000~-46000），1=宬 HP（HP1Y≈-82000） |
| GearRatio 世代      | **次分組 Key** | ~0.194=舊 SYNTEK卡；~0.350=新 SMC卡；A系列差異<100 unit故合並 |
| GPIB 12-site        | **次分組 Key** | GPIB Model=`9045GPIB_12Site` → 升級為 B12 群 |
| Serial No           | **機台識別**   | `[Version] Serial No` — 相同 S/N 視為同一台機器 |

## 機台群組分類表

| 群組 | 說明 | 成員機型 |
|------|------|----------|
| **A** | 大機型系列 (32-site GPIB) | `HT-9046AP`, `HT-9046AT`, `HT-9046AT+`, `HT-9046LS`, `HT-9132`, `HT9046LS` |
| **B** | HT-9045 標準機 (8-site GPIB) | `HT-9045`, `HT-9045HA`, `HT-9045HW`, `HT-9045L`, `HT-9045W`, `HT-9045WA`, `HT-9045WH`, `HT-9046WA`, `HT9045`, `HT9045HA`, `HT9045WA`, `HT9046HA` |
| **B12** | HT-9045 12-site GPIB 變體 (9045GPIB_12Site) | *(B 群機台在執行時升級，GPIB=9045GPIB_12Site)* |
| **B*** | HT-9045F (ATK 客製獨立) | `HT-9045F` |
| **C** | HT-9046 系列 (16-site 真空) | `HT-9046A`, `HT9046`, `HT9046-02` |
| **D** | HT-9045 12-site 獨立 | `HT-9045S`, `HT9045_12Site` |
| **E** | 獨立機種 | `HT-9011UC`, `HT-9016C`, `HT-9132LS`, `HT9046C` |

> **B12 判斷依據**：`GPIB9045/system/general.ini [Version] Model = 9045GPIB_12Site`。
> 這類機台的 HT9045 側 Model 仍為 HT-9045WA/WH/HW/HA 等，但配置了 12-site GPIB 程式頭，teach 位置與標準 8-site 有所差異。

### B12 群機台清單（判斷依據：GPIB=9045GPIB_12Site）

| Model | Serial No | Factory | 樣本數 |
|-------|-----------|---------|--------|
| `HT-9045HA` | `ILC483` | KYEC | 1 |
| `HT-9045HA` | `ILD357` | KYEC | 1 |
| `HT-9045HW` | `HLD842` | KYEC | 1 |
| `HT-9045HW` | `ILD084` | KYEC | 3 |
| `HT-9045WA` | `ELC617` | KYEC | 7 |
| `HT-9045WA` | `ELC742` | KYEC | 1 |
| `HT-9045WA` | `ELC752` | KYEC | 1 |
| `HT-9045WA` | `ILD352` | KYEC | 3 |
| `HT-9045WA` | `JLD615` | KYEC | 2 |
| `HT-9045WA` | `JLD617` | KYEC | 1 |
| `HT-9045WA` | `JLD650` | KYEC | 1 |
| `HT-9045WH` | `ELD105` | KYEC | 1 |
| `HT-9045WH` | `ELD106` | KYEC | 1 |
| `HT-9045WH` | `ELD799` | KYEC | 1 |
| `HT-9045WH` | `ELD802` | KYEC | 1 |

## 群組總覽

| # | 群組 | HT Model | HPP | GearRatio世代 | 機台數 | 樣本數 | Picker | Y Pitch | 群組詳細 |
|---|------|----------|-----|---------------|--------|--------|--------|---------|----------|
| G01 | **A** | `HT-9046AP` | HPP=0 | (合並) | 3 | 4 | 1×4 | 2×4 | [G01](groups/G01-A-HT-9046AP-HPP0-ALL.md) |
| G02 | **A** | `HT-9046AT` | HPP=0 | (合並) | 6 | 10 | 1×10 | 2×9, 1×1 | [G02](groups/G02-A-HT-9046AT-HPP0-ALL.md) |
| G03 | **A** | `HT-9046AT+` | HPP=0 | (合並) | 12 | 26 | 1×26 | 2×25, 1×1 | [G03](groups/G03-A-HT-9046AT_-HPP0-ALL.md) |
| G04 | **A** | `HT-9046LS` | HPP=0 | (合並) | 36 | 58 | 1×58 | 2×39, 1×17 | [G04](groups/G04-A-HT-9046LS-HPP0-ALL.md) |
| G05 | **A** | `HT-9132` | HPP=0 | (合並) | 30 | 73 | 1×73 | 2×72, 0×1 | [G05](groups/G05-A-HT-9132-HPP0-ALL.md) |
| G06 | **A** | `HT9046LS` | HPP=0 | (合並) | 9 | 15 | 1×15 | 0×8, 1×7 | [G06](groups/G06-A-HT9046LS-HPP0-ALL.md) |
| G07 | **B** | `HT-9045` | HPP=1 | 0.350(M) | 1 | 2 | 1×2 | 0×2 | [G07](groups/G07-B-HT-9045-HPP1-SMC.md) |
| G08 | **B** | `HT-9045HA` | HPP=0 | 0.194(S) | 2 | 7 | 1×7 | 0×7 | [G08](groups/G08-B-HT-9045HA-HPP0-SYNTEK.md) |
| G09 | **B** | `HT-9045HA` | HPP=0 | 0.350(M) | 8 | 35 | 1×35 | 0×35 | [G09](groups/G09-B-HT-9045HA-HPP0-SMC.md) |
| G10 | **B** | `HT-9045HA` | HPP=1 | 0.350(M) | 5 | 9 | 1×9 | 0×9 | [G10](groups/G10-B-HT-9045HA-HPP1-SMC.md) |
| G11 | **B** | `HT-9045HW` | HPP=0 | 0.350(M) | 3 | 6 | 1×6 | 0×5, 1×1 | [G11](groups/G11-B-HT-9045HW-HPP0-SMC.md) |
| G12 | **B** | `HT-9045HW` | HPP=1 | 0.350(M) | 3 | 7 | 1×7 | 0×7 | [G12](groups/G12-B-HT-9045HW-HPP1-SMC.md) |
| G13 | **B** | `HT-9045L` | HPP=0 | 0.350(M) | 5 | 5 | 1×5 | 1×5 | [G13](groups/G13-B-HT-9045L-HPP0-SMC.md) |
| G14 | **B** | `HT-9045W` | HPP=0 | 0.350(M) | 2 | 2 | 1×2 | 0×2 | [G14](groups/G14-B-HT-9045W-HPP0-SMC.md) |
| G15 | **B** | `HT-9045WA` | HPP=0 | 0.194(S) | 1 | 1 | 1×1 | 0×1 | [G15](groups/G15-B-HT-9045WA-HPP0-SYNTEK.md) |
| G16 | **B** | `HT-9045WA` | HPP=1 | GearY=? | 1 | 1 | 1×1 | 0×1 | [G16](groups/G16-B-HT-9045WA-HPP1-GearY=.md) |
| G17 | **B** | `HT-9045WA` | HPP=1 | 0.350(M) | 7 | 13 | 1×13 | 0×10, 1×3 | [G17](groups/G17-B-HT-9045WA-HPP1-SMC.md) |
| G18 | **B** | `HT-9045WH` | HPP=0 | 0.350(M) | 4 | 7 | 1×7 | 0×7 | [G18](groups/G18-B-HT-9045WH-HPP0-SMC.md) |
| G19 | **B** | `HT-9046WA` | HPP=0 | 0.350(M) | 1 | 1 | 1×1 | 0×1 | [G19](groups/G19-B-HT-9046WA-HPP0-SMC.md) |
| G20 | **B** | `HT9045` | HPP=0 | 0.350(M) | 3 | 3 | 1×3 | 1×2, 0×1 | [G20](groups/G20-B-HT9045-HPP0-SMC.md) |
| G21 | **B** | `HT9045` | HPP=1 | 0.350(M) | 1 | 2 | 1×2 | 0×2 | [G21](groups/G21-B-HT9045-HPP1-SMC.md) |
| G22 | **B** | `HT9045HA` | HPP=0 | 0.350(M) | 8 | 15 | 1×15 | 0×12, 1×3 | [G22](groups/G22-B-HT9045HA-HPP0-SMC.md) |
| G23 | **B** | `HT9045HA` | HPP=1 | 0.350(M) | 1 | 1 | 1×1 | 0×1 | [G23](groups/G23-B-HT9045HA-HPP1-SMC.md) |
| G24 | **B** | `HT9045WA` | HPP=0 | 0.350(M) | 2 | 6 | 1×6 | 0×6 | [G24](groups/G24-B-HT9045WA-HPP0-SMC.md) |
| G25 | **B** | `HT9046HA` | HPP=0 | 0.350(M) | 3 | 5 | 1×5 | 0×5 | [G25](groups/G25-B-HT9046HA-HPP0-SMC.md) |
| G26 | **B12** | `HT-9045HA` | HPP=0 | 0.350(M) | 2 | 2 | 1×2 | 0×2 | [G26](groups/G26-B12-HT-9045HA-HPP0-SMC.md) |
| G27 | **B12** | `HT-9045HW` | HPP=0 | 0.350(M) | 1 | 3 | 1×3 | 0×3 | [G27](groups/G27-B12-HT-9045HW-HPP0-SMC.md) |
| G28 | **B12** | `HT-9045HW` | HPP=1 | 0.350(M) | 1 | 1 | 1×1 | 0×1 | [G28](groups/G28-B12-HT-9045HW-HPP1-SMC.md) |
| G29 | **B12** | `HT-9045WA` | HPP=1 | 0.350(M) | 7 | 16 | 1×16 | 1×9, 0×7 | [G29](groups/G29-B12-HT-9045WA-HPP1-SMC.md) |
| G30 | **B12** | `HT-9045WH` | HPP=0 | 0.350(M) | 4 | 4 | 1×4 | 0×4 | [G30](groups/G30-B12-HT-9045WH-HPP0-SMC.md) |
| G31 | **B*** | `HT-9045F` | HPP=0 | 0.194(S) | 1 | 1 | 1×1 | 0×1 | [G31](groups/G31-B_-HT-9045F-HPP0-SYNTEK.md) |
| G32 | **C** | `HT-9046A` | HPP=1 | 0.350(M) | 1 | 1 | 1×1 | 0×1 | [G32](groups/G32-C-HT-9046A-HPP1-SMC.md) |
| G33 | **C** | `HT9046` | HPP=0 | 0.194(S) | 4 | 13 | 1×13 | 2×12, 1×1 | [G33](groups/G33-C-HT9046-HPP0-SYNTEK.md) |
| G34 | **C** | `HT9046` | HPP=0 | 0.350(M) | 10 | 21 | 1×21 | 1×15, 2×5 | [G34](groups/G34-C-HT9046-HPP0-SMC.md) |
| G35 | **C** | `HT9046-02` | HPP=0 | 0.350(M) | 1 | 1 | 1×1 | 2×1 | [G35](groups/G35-C-HT9046-02-HPP0-SMC.md) |
| G36 | **D** | `HT-9045S` | HPP=1 | 0.350(M) | 1 | 1 | 1×1 | 1×1 | [G36](groups/G36-D-HT-9045S-HPP1-SMC.md) |
| G37 | **D** | `HT9045_12Site` | HPP=0 | 0.350(M) | 1 | 4 | 1×4 | 1×4 | [G37](groups/G37-D-HT9045_12Site-HPP0-SMC.md) |
| G38 | **E** | `HT-9011UC` | HPP=0 | 0.194(S) | 1 | 3 | 1×3 | 0×3 | [G38](groups/G38-E-HT-9011UC-HPP0-SYNTEK.md) |
| G39 | **E** | `HT-9016C` | HPP=1 | 0.6(other) | 1 | 12 | 1×12 | 2×12 | [G39](groups/G39-E-HT-9016C-HPP1-GearY~.md) |
| G40 | **E** | `HT-9132LS` | HPP=0 | 0.194(S) | 1 | 2 | 1×2 | 2×2 | [G40](groups/G40-E-HT-9132LS-HPP0-SYNTEK.md) |
| G41 | **E** | `HT9046C` | HPP=0 | 0.350(M) | 1 | 2 | 1×2 | 2×2 | [G41](groups/G41-E-HT9046C-HPP0-SMC.md) |

<!-- preserved-content:end -->
