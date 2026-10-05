# Release Note 製作規則（Track B）

> 依據 `D:\HT9045\.claude\skills\make-report-skill\SKILL.md` 的 release-note 規範執行。
> Track A Step 4 進行中即可同步填入草稿；Track A Step 5 完成後補填 installer 資訊。

---

## 輸出路徑與 Template

| 版本層級 | 輸出路徑 | Template |
|---------|---------|---------|
| 廠內版（含 Pre-Release Scan 細節）| `<入口網站 repo>\public\Docs\customers\{代理商}\{客戶代碼}\release-notes\{YYYY}\` | honprec-red |
| 代理商版 | `<入口網站 repo>\public\Docs\customers\{代理商}\0000_{代理商}\release-notes\{YYYY}\` | honprec-red |
| 客戶版 | `<入口網站 repo>\public\Docs\customers\{代理商}\{客戶代碼}\release-notes\{YYYY}\` | honprec-red |

---

## 檔名規則

### Release Note

統一格式（不含作者名）：

```
HT-9xxx_Software_Release_Note_V{VERSION}_{YYYYMMDD}_{客戶縮寫}.md
HT-9xxx_Software_Release_Note_V{VERSION}_{YYYYMMDD}_{客戶縮寫}.html
```

| 版本層級 | 客戶縮寫欄位 |
|---------|------------|
| 廠內版 | `HonPrec`（廠內留存）或省略 |
| 代理商版 | 代理商縮寫（例：`TeraTech`） |
| 客戶版 | 客戶慣用縮寫（例：`ATK`、`FMSH`） |

範例：
- `HT-9xxx_Software_Release_Note_V3.33.903.3_20260427_ATK.md`
- `HT-9xxx_Software_Release_Note_V3.33.899.2_20260326_ATK.html`

⚠️ 禁止在檔名中加入作者名（例：`_Steven_`）。
⚠️ 版本號欄位直接用原版號，不簡化（例：`V3.33.903.3` 而非 `V903.3`）。
⚠️ 日期欄位固定 8 碼 YYYYMMDD（例：`20260427`）。
⚠️ 代理商版客戶縮寫**不可**用 `代理商版本` 或 `客戶版本` 等中文固定字樣。

### 軟體功能新增提案表 / 軟體異常修正紀錄表

```
{YYYYMMDD}_軟體功能新增提案表_{問題編號}.md
{YYYYMMDD}_軟體功能新增提案表_{問題編號}.html
{YYYYMMDD}_軟體異常修正紀錄表_{問題編號}.md
```

範例：
- `20260421_軟體功能新增提案表_ATK_ATC_MultiZone_NotWork.md`
- `20260427_軟體異常修正紀錄表_ATK_AutoCleanCount.md`

⚠️ 禁止在檔名中加入作者名（例：`_Steven_`）。

---

## 草稿階段內容（Track A Step 4 進行中即可填入）

1. Bug fix 詳情（提案表內容）
2. 既有客戶版 release note 內容
3. `Pre-Release Risk Check` 專章草稿：
   - 掃描範圍
   - 目前 findings 摘要
   - 待補的 Build / 語意分析欄位（標記 `[待補]`）
4. 編譯資訊（exe 大小 / 時間 / 0 errors / warnings 摘要）
5. `SOFT_SIMULTE` 已註解的驗證證據

---

## Track A Step 5 完成後補填

- Installer 檔名與大小（`.exe` + `.7z`）
- 依 `PreReleaseRiskCheck_*.md/html` 最終正式報告，補齊 release note 內 `Pre-Release Risk Check` 專章：
  - Build 結果
  - 語意分析結論
  - blocking issue 狀態

---

## Pre-Release Risk Check 併入規則

1. `PreReleaseRiskCheck_*.md/html` 必須**獨立保存**，不得省略。
2. release note 必須新增 **Pre-Release Risk Check 專章或附錄**，不可只寫一句「已執行 pre-release-check」帶過。
3. **廠內版**：完整摘要（掃描範圍、Critical/High/Low 統計、Build 結果、語意分析結論、待確認項目）；必要時附主要 findings 表。
4. **代理商版 / 客戶版**：去敏感摘要版（僅保留掃描已完成、風險等級統計、Build 通過與是否有 blocking issue）；不揭露內部檔名、行號、敏感實作細節，除非使用者另有要求。
5. Release note 完成條件：**兩份工件均已產出**：
   - 獨立的 `PreReleaseRiskCheck_*.md/html`
   - 已實際併入該報告內容的最終 release note `*.md/html`

---

## MD → HTML 轉換

```powershell
python "D:\HT9045\.claude\skills\make-report-skill\scripts\md_to_html.py" "<output.md>" --template red
```

在 Track A 與 Track B 均完成後執行。
