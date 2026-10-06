> 保存來源：`.claude/skills/ht9045-general-ini/SKILL.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../common.md)。

<!-- preserved-content:start -->

# HT9045 Gerneral.ini — 規格指示書同步 Skill

> **INI 結構靜態規格**（Section 定義、模組角色、資料流、Key 對應表）已移至：
> **[`d:\HT9045\.github\specs\gerneral-ini-schema.md`](../../../../../.github/specs/gerneral-ini-schema.md)**
>
> 本 Skill 聚焦於**工作程序**：依規格指示書修正特定機台的 `Gerneral_*.ini` 檔案。

---

## 規格指示書轉 Gerneral.ini（Spec Sync 工作流程）

> **適用場景**：接到「依規格指示書修正 `Gerneral_機台序號.ini`」工作時使用。目標是最少修改、先驗證再寫入。

### 讀取順序

1. 讀目標 INI：`d:\HT9045\規格指示書\Gerneral_*.ini`
2. 讀對應規格書：`d:\HT9045\規格指示書\機器製造規格指示書_*.html`
3. 需要數值或 enum 時，讀 `d:\HT9045\HT9045_SVN_TempFile\rev897_base\MachineType.h`

### 核心欄位對應

| Gerneral.ini | 規格指示書欄位 | 寫法規則 |
|------|------|------|
| `[System] CUSTOMER_CODE` | 終端客戶 | 使用 `MachineType.h` 數字 |
| `[Version] Model` | 機型標籤 | 文字 |
| `[Version] Serial No` | 機號 | 文字 |
| `[Version] Machine ID` | 機號 | 文字 |
| `[Version] Factory` | 終端客戶 | 使用英文，禁止中文或數字 |

### 規格選項對應表

#### 001 取放裝置

| 規格選項 | USE_PICKER_COUNT | USE_IN_OUT_ARM_Y_PITCH | IN_OUT_ARM_Y_PITCH_MIN | IN_OUT_ARM_Y_PITCH_MAX |
|------|------|------|------|------|
| 2x4 Z軸馬達 | 1 | 0 | 0 | 0 |
| 2x4 Z軸馬達+Y軸手動 | 1 | 1 | 0 | 0 |
| 2x4 Z軸馬達+快拆吸嘴 | 1 | 0 | 0 | 0 |
| 2x4 Z軸馬達+快拆吸嘴+Y軸手動 | 1 | 1 | 0 | 0 |
| 2x4 Z軸馬達+X Y Auto+快拆吸嘴 | 1 | 2 | 1500 | 6500 |
| 2x4 Z軸馬達+X Y Auto+快拆吸嘴+In/Out Arm 光學尺 | 1 | 2 | 1500 | 6500 |
| 2x4 Z軸馬達+XY Auto | 1 | 2 | 1500 | 6500 |
| 2x2 Z軸馬達 | 0 | — | — | — |
| 2x2 Z軸馬達+Y軸手動 | 0 | — | — | — |
| 2x2 Z軸馬達+快拆吸嘴 | 0 | — | — | — |

#### 005 空 Tray 收送

| 規格選項 | AUTO_EMPTY_COLOR |
|------|------|
| Empty Manual/Color Manual | 0 |
| Empty Auto/Color Auto | 1 |

#### 009 加熱模組

| 規格選項 | USE_ATC_MODE | ATC_SYSTEM_USEHEAT | iSocketBaseTempCount | INSTALL_HEAT_GUN | USE_16_HEATER |
|------|------|------|------|------|------|
| 標準加熱模組 | eATCUninstall | — | eDut1ea | — | eht4Heater |
| DUT加熱(Socket Base) | — | — | eDut4ea | — | — |
| Index多組式加熱(8 site) | — | — | — | — | eht16HeaterDTME08 |
| Index多組式加熱(16 site) | — | — | — | — | eht32HeaterDTME08 |
| Socket Hot Air | — | — | — | 1 | — |
| ATC 2.X系統 Dual Site實裝 | eATCHonPrecType | 4 | — | — | — |
| ATC 2.X系統 Quad Site實裝 | eATCHonPrecType | 8 | — | — | — |
| ATC 3.X系統 Single Site實裝 | eNewATCSystem | 2 | — | — | — |
| ATC 3.X系統 Dual Site實裝 | eNewATCSystem | 4 | — | — | — |
| ATC 3.X系統 Quad Site實裝 | eNewATCSystem | 8 | — | — | — |
| ATC 3.X系統 16 Site實裝 | eNewATCSystem | 32 | — | — | — |
| ATC 6.X系統 Quad Site實裝 | eNewATCSystem | 8 | — | — | — |
| ATC 6.X系統 8 Site實裝 | eNewATCSystem | 16 | — | — | — |
| ATC 6.X系統 16 Site實裝 | eNewATCSystem | 32 | — | — | — |
| ATC 7.X系統 Dual Site實裝 | eATCHonPrecType | 4 | — | — | — |

#### 014 Tray 取放機構

| 規格選項 | USE_CATCH_TRAY_MODEL |
|------|------|
| 吸Tray方式 | 0 |
| 左右夾正Tray方式 | 0 |
| 左右夾正反面Tray方式 | 0 |
| 前後夾正Tray方式 | 1 |
| ART前後夾正Tray方式 | 2 |
| ART前後夾正Tray + 旋轉 | 2 |
| 寬版吸Tray方式 | 0 |
| 旋轉吸Tray方式 | 0 |
| 直壓式吸TRAY方式 | 0 |
| ART左右夾正Tray方式 | 3 |

#### 022 Show Bin 顯示器

| 規格選項 | NUMBER_PANEL_TYPE |
|------|------|
| 點矩陣 | 3 |
| TFT | 4 |
| 無 | 0 |

#### 037 Contact Force

| 規格選項 | INDEX_PRESS_TYPE |
|------|------|
| 85Kg | e85KG |
| 120Kg | e120KG |
| 240Kg | e240KG |
| 360Kg | e360KG |
| 480Kg | e500KG |
| 640Kg | e640KG |

#### 059 Socket sensor 疊料偵測

| 規格選項 | USE_SOCKET_SENSOR | SocketSenAmpQty |
|------|------|------|
| 無 | 0 | 0 |
| 4組 | 1 | 4 |
| 3組 | 1 | 3 |
| 2組 | 1 | 2 |
| 其他 | 1 | 依規格填入 |

#### 075 175度加熱

| 規格選項 | HighTempLimit |
|------|------|
| 無 | tTemp130 |
| 支援175℃ | tTemp175 |

### 欄位驗證備查

- `INDEX_PRESS_TYPE`：先查 `enum eIndexPressType`
- `USE_16_HEATER`：先查 `enum eHeaterType`
- `SocketBasedAdd4Temp`：先查 `enum eSocketTempControll`
- `HighTempLimit`：先查 `enum eTempType`
- `ATC_SYSTEM_USEHEAT`：需與 `USE_ATC_MODE` 一起判讀

### 執行流程

1. 從規格書找出機型、機號、終端客戶、加熱模組、Contact Force、Socket sensor、高溫支援。
2. 以 `MachineType.h` 驗證所有數字或 enum 對應，不直接猜值。
3. 修改 INI 時只改必要欄位，保留其他模板值。
4. `Factory` 一律寫英文客戶名，不寫中文、不寫數字。
5. 若 `USE_ATC_MODE=0`，通常 `ATC_SYSTEM_USEHEAT` 應同步檢查是否要設為 `0`。
6. 修改後至少回讀一次目標區塊確認。

### Factory 寫法規則

- 參照終端客戶英文名寫入（如 `AMD_SUZHOU`）
- 不要把 `Factory` 寫成客戶代碼數字，除非使用者明確要求

### 輸出要求

- 回報「修改前 → 修改後」
- 若有歧義欄位，先列出判斷依據再決定
- 若使用者要求留存結果，產生 md 報告並記錄來源、對應依據、最終值

<!-- preserved-content:end -->
