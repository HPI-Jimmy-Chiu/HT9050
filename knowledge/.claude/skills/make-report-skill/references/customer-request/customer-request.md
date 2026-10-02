# Customer Request（軟體功能新增提案表）

## 三版本定義（必須明確後再產出）

| 版本 | 簡稱 | 受眾 | 可包含內容 | 禁止內容 |
|------|------|------|------------|----------|
| **對內詳細版** | `internal` | HonPrec 內部工程師/PM | 程式碼 diff、函式名稱、完整技術說明、P1~P6 風險標記 | — |
| **代理商版** | `distributor` | 代理商工程師 | 流程圖、功能描述、對應客戶資訊、操作步驟 | 原始程式碼 |
| **客戶版** | `customer` | 終端客戶 | 流程圖、使用者可見功能說明 | 原始碼、內部技術細節；**資訊不足時須向客戶提問**（見下方提問規則）|

### 客戶版提問規則
若以下資訊缺失，**必須先向使用者（或確認轉給客戶）提問，取得回答後再產出**：
1. 客戶現有操作流程描述（When / What / How）
2. 期望的改變結果
3. 客戶端硬體/軟體版本限制
4. 是否有特殊的安規或驗收標準

## 適用時機
- 使用者要求依 `.msg` 郵件生成提案表
- 使用者要求套用「軟體功能新增提案表」格式
- 輸出 `.md` + `.html` 雙格式

## 輸出路徑
- 三版本（廠內版 / 代理商版 / 客戶版）皆放在該客戶資料夾下：
  `<repo>\public\Docs\customers\{代理商}\{客戶代碼}_{客戶名稱}\proposals\{YYYY}\`
- 僅以檔名尾碼 `{版本別}` 區分版本。
- 備份：`U:\共用區\客戶需求單\{對應路徑}` (生成後 robocopy)

## 輸出格式
- HTML：使用 **honprec-red-template**（主色 `#c0392b`，含 Logo）
- Markdown：純文字 metadata 表格，**不放 Logo**

## 客戶路由規則

| 寄件信箱網域 | 輸出目錄 |
|---|---|
| `@honprec.com` | `0000_HonPrec\proposals\` |
| `@teratechkorea.com` | `TeraTech\{客戶代碼}\proposals\`（主旨判斷：ATK/SCK/Hana/TESNA）|
| `@jb-elite.com` | `JB-Elite\{客戶代碼}\proposals\` |
| `@spandnix.co.jp` | `Spandnix\{客戶代碼}\proposals\` |
| `@htsepl.com` | `HTS\{客戶代碼}\proposals\` |
| 其他 | `HPI-TW\` or `HPI-USA\` or `HPI-EURO\` or `ETC\` |

## 多語言支援

| 代理商 | 語言 |
|---|---|
| TeraTech | 韓文 |
| JB-Elite | 英文（泰文/菲律賓文） |
| Spandnix | 中文 + 日文 |
| HTS | 繁體中文 + 英文 |
| 廠內 | 繁體中文 |

## 提案資訊表（必填）

```
| 項目 | 內容 |
| 問題編號 | #P260427-ATK-H9-01（格式見 issue-number-format.md）|
| 提案版本 | V3.33.xxx.x |
| 提案日期 | YYYY-MM-DD |
| 客戶代碼 | {CODE}_{NAME} |
| 機台型號 | HT9045 / GPIB9045 |
| 工程師 | 姓名 |
| 修改主題 | 5-10 字簡短說明 |
```

## 命名規則
```
{YYYYMMDD}_{專案名稱}_{提案主旨}_廠內版.md/.html   ← 對內詳細版
{YYYYMMDD}_{專案名稱}_{提案主旨}_代理商版.md/.html ← 代理商版
{YYYYMMDD}_{專案名稱}_{提案主旨}_客戶版.md/.html    ← 客戶版
```

- `{版本別}` 為固定字樣：`廠內版` / `代理商版` / `客戶版`。
- `{專案名稱}` 例：`HT9045`、`HT9046LS`、`GPIB9045`。
- 範例：`20260318_HT9045L_AutoClean_InarmPickerDown_廠內版.html`
- 完整輸出路徑與命名規則統一參考 [../report-version-naming/report-version-naming.md](../report-version-naming/report-version-naming.md)。

## 完整規格
詳見：`../../customer-code/customer-code-manager/SKILL.md`（客戶縮寫/代理商唯一來源：`d:\.github\instructions\customer-code-table.instructions.md`）
HTML 樣式詳見：`../../templates/honprec-red-template/SKILL.md`

## 相關參考
- 問題編號格式：[`../issue-number-format/issue-number-format.md`](../issue-number-format/issue-number-format.md)
- 全域問題索引：`<repo>\public\Docs\customers\issue-index.md`（20260929 之前的在 `D:\docs\customers\issue-index.md`，內容相同）
