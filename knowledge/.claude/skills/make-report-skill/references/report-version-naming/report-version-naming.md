# Report Version Naming（報告版本命名規則）

## 適用範圍

適用於所有由 make-report-skill 產生、或引用其規則的報告類型，只要檔名、標題、輸出版本涉及以下 audience 層級：

- 廠內版
- 代理商版
- 客戶版

典型場景包括：

- Release Note（發版說明）
- 客戶提案報告（軟體功能新增提案表 / 軟體異常修正紀錄表）
- 客戶手冊 / 代理商手冊 / 使用說明書
- 任何需要區分 HonPrec / 代理商 / 客戶受眾的對外報告

## 核心原則

1. `廠內版`、`代理商版`、`客戶版` 是 audience 分類，**並且直接作為檔名尾碼的固定字樣**。
2. 檔名一律以 `{YYYYMMDD}` 為前綴（8 碼日期）。
3. 目錄與檔名是兩套變數：
   - 目錄使用 `{代理商}`、`{客戶代碼}_{客戶名稱}` 等結構化路徑變數。
   - 檔名尾碼一律使用固定字樣 `廠內版` / `代理商版` / `客戶版`。

## audience → 檔名尾碼對照

| audience 詞 | 檔名尾碼（版本別） | 說明 |
|---|---|---|
| 廠內版 | `廠內版` | 固定字樣 |
| 代理商版 | `代理商版` | 固定字樣 |
| 客戶版 | `客戶版` | 固定字樣 |

> 注意：此規則已於 2026-05 更新。先前版本要求以 `{代理商名稱}` / `{客戶名稱}`（如 `TeraTech`、`ATK`）作為尾碼，現已改為固定字樣 `代理商版` / `客戶版`。代理商 / 客戶的識別改由「目錄」承載（`{客戶代碼}_{客戶名稱}`）。

## 輸出路徑

| 報告類型 | 輸出路徑 |
|---|---|
| Release Note | `<repo>\public\Docs\customers\{代理商}\{客戶代碼}_{客戶名稱}\release-notes\{YYYY}\` |
| 提案報告 | `<repo>\public\Docs\customers\{代理商}\{客戶代碼}_{客戶名稱}\proposals\{YYYY}\` |
| 相關手冊 / 說明書 | `<repo>\public\Docs\manual\` |

- 同一份報告的廠內版 / 代理商版 / 客戶版，皆放在「該客戶」對應的 `release-notes\{YYYY}` 或 `proposals\{YYYY}` 之下，僅以檔名尾碼區分版本別。
- 手冊不分客戶，統一放在 `<repo>\public\Docs\manual\`（可依主題建子資料夾，如 `GPIB_Manual\`）。

## 檔名格式

```text
Release Note：{YYYYMMDD}_{專案名稱}_Software_Release_Note_V{版本號4碼}_{版本別}.html / .md
提案報告：    {YYYYMMDD}_{專案名稱}_{提案主旨}_{版本別}.html / .md
相關手冊：    {YYYYMMDD}_{手冊主旨}_{版本別}.html / .md
```

- `{版本號4碼}`：四碼版本，例 `3.33.904.0`。
- `{版本別}`：`廠內版` / `代理商版` / `客戶版`。
- `{專案名稱}`：例 `HT-9xxx`、`HT-9045`、`GPIB9045`；無法判定時預設 `HT-9xxx`。
- 多語版本可在 `{版本別}` 後再附語言/標記 token，例 `_客戶版_ENG_KOR`、`_客戶版_BETA2`。

## 範例

```text
Release Note 廠內版：20260504_HT-9xxx_Software_Release_Note_V3.33.904.0_廠內版.html
Release Note 客戶版：20260325_HT-9xxx_Software_Release_Note_V3.33.899.1_客戶版.html
Release Note 客戶版(多語)：20260327_HT-9xxx_Software_Release_Note_V3.33.899.2_客戶版_ENG_KOR.md
提案報告 廠內版：20260318_HT9045L_AutoClean_InarmPickerDown_廠內版.html
提案報告 客戶版：20260316_HT9046LS_Offset_Z_Reset_客戶版.md
手冊 客戶版：20260401_GPIB_Standard_Command_Manual_客戶版_zh-TW.html
```

## 判斷順序

1. 先判斷報告 audience：廠內版 / 代理商版 / 客戶版 → 決定檔名尾碼 `{版本別}`。
2. 判斷報告類型（Release Note / 提案報告 / 手冊）→ 決定檔名格式與輸出路徑。
3. 判斷目錄變數：`{代理商}`、`{客戶代碼}_{客戶名稱}`。
   - 客戶代碼 / 客戶名稱 / 代理商，優先參考 `D:\HT9045\.claude\skills\make-report-skill\references\customer-code-table\customer-code-table.instructions.md`，正式來源為 `D:\HT9045\.claude\skills\make-report-skill\customer-code\customer-code-manager\SKILL.md`。
   - `Distributor` 未明示但 `Region` 可判定時，依「地區 → 代理商」映射推定。
4. 組合：`{路徑}\{YYYYMMDD}_..._{版本別}.{ext}`。

## 常見錯誤

- 檔名尾碼仍寫成 `{代理商名稱}` / `{客戶名稱}`（如 `_TeraTech`、`_ATK`）→ 應改為固定字樣 `代理商版` / `客戶版`。
- 忘記 `{YYYYMMDD}` 前綴。
- Release Note 漏掉 `Software_Release_Note_V` 與四碼版本號。
- 把廠內版 Release Note 放到 `D:\docs\release-notes\` 而非客戶資料夾下（現行規則一律放客戶資料夾的 `release-notes\{YYYY}`）。
- 手冊放進客戶資料夾，而非統一的 `<repo>\public\Docs\manual\`。