# Release Note（發版說明）

## 適用時機
- 使用者要求產生 HT9045 / GPIB9045 發版說明
- 輸出三層級版本：廠內版、代理商版、客戶版

## 三層級結構

| 版本 | 語言 | 受眾 | 技術詳細度 |
|---|---|---|---|
| 廠內版 | 繁體中文 | HonPrec 內部工程/PM/QC | 完整（含代碼/函式名）|
| 代理商版 | 代理商語言 | 代理商工程師 | 中等（功能描述，不含代碼）|
| 客戶版 | 客戶語言 | 終端用戶 | 精簡（操作影響說明）|

## 輸出路徑
- 三版本（廠內版 / 代理商版 / 客戶版）皆放在該客戶資料夾下：
  `<repo>\public\Docs\customers\{代理商}\{客戶代碼}_{客戶名稱}\release-notes\{YYYY}\`
- 僅以檔名尾碼 `{版本別}` 區分；不再分散到 `D:\docs\release-notes\`。
- 備份：生成後 robocopy 至 `U:\共用區\客戶需求單\` 對應路徑。

## 輸出格式
- HTML：使用 **honprec-red-template**（主色 `#c0392b`，含 Logo）
- Markdown：純文字 metadata 表格，**不放 Logo**

## 多語言支援

| 代理商 | 語言 |
|---|---|
| TeraTech | 韓文 |
| JB-Elite | 泰文 / 菲律賓文 |
| Spandnix | 繁體中文+日文 |
| HTS | 簡體中文 |
| 廠內 | 繁體中文 |

## 版本號格式
```
V{Major}.{Minor}.{Revision}.{Build}_{YYYYMMDD}
例：V3.33.900.0_20260331
```

## 命名規則
```
{YYYYMMDD}_{專案名稱}_Software_Release_Note_V{版本號4碼}_{版本別}.md/.html
```

- `{版本別}` 為固定字樣：`廠內版` / `代理商版` / `客戶版`。
- `{專案名稱}` 例：`HT-9xxx`、`HT-9045`、`GPIB9045`；無法判定時預設 `HT-9xxx`。
- 多語版本在 `{版本別}` 後附語言/標記 token，例 `_客戶版_ENG_KOR`、`_客戶版_BETA2`。

範例：
```
廠內版：20260504_HT-9xxx_Software_Release_Note_V3.33.904.0_廠內版.html
客戶版：20260325_HT-9xxx_Software_Release_Note_V3.33.899.1_客戶版.html
客戶版(多語)：20260327_HT-9xxx_Software_Release_Note_V3.33.899.2_客戶版_ENG_KOR.md
```

- 完整 mapping、輸出路徑與更多範例，統一參考 [../report-version-naming/report-version-naming.md](../report-version-naming/report-version-naming.md)。
- 檔名尾碼一律使用固定字樣 `廠內版`/`代理商版`/`客戶版`，不再使用 `{代理商名稱}`/`{客戶名稱}`（如 `_TeraTech`、`_ATK`）。
- 代理商 / 客戶識別改由目錄 `{客戶代碼}_{客戶名稱}` 承載。

## 必填內容
1. 版本資訊（版本號、日期、EXE 路徑）
2. 新增功能清單
3. 修正問題清單
4. 相關客戶需求單連結
5. 廠內版額外包含：修改檔案、函式名稱、測試驗證結果

## 完整規格
HTML 樣式詳見：`../../templates/honprec-red-template/SKILL.md`
腳本工具：`../../scripts/generate_release_note.py`
