---
name: customer-code-manager
description: >
  客戶代碼管理技能（CustomerReq 工作區）。支援：客戶代碼編號申請、
  代理商對應關係配置、區域語言設定、Release Note 客戶映射。
  與 HT9045 端的 ht9045-customer-code-manager 技能雙向同步。
keywords:
  - 客戶代碼
  - customer code
  - 代理商
  - distributor
  - 客戶名稱
  - 地區配置
  - 代碼申請
  - 新客戶
applyTo: "**/*"
---

# 客戶代碼管理技能（CustomerReq 工作區）

本技能負責客戶需求單工作區的客戶代碼配置、代理商映射、區域語言設定。

---

## 技能分工與雙向同步

客戶代碼由**兩個技能分工**維護，兩者**互為上下游、必須雙向同步**：

| 技能 | 工作區 | 負責範圍 |
|------|--------|---------|
| **ht9045-customer-code-manager**（程式碼端） | `d:\HT9045\.github\skills\ht9045-customer-code-manager\SKILL.md` | `#define CC_xxx`（MachineType.h）、`FUNC_CC_xxx` + `case`（CosFunction.cpp）、`rgCustomerList`（HandlerSys.dfm） |
| **customer-code-manager**（本技能，報告/索引端） | `d:\.github\skills\make-report-skill\customer-code\customer-code-manager` | 客戶縮寫表（`customer-code-table.instructions.md`，唯一來源）、地區→代理商→語言映射、Release Note 客戶路由 |

> **同步規則**：
> - **程式碼端為代碼數值與英文符號的來源**（Code Number / `CC_` Symbol）。
> - **本技能為代理商與語言配置的來源**；撰寫報告或判定客戶代理商時一律以本端的 `customer-code-table.instructions.md` 為準。
> - 任一端新增客戶，缺少另一端對應 = 未完成（見下方「特殊情況處理 → 情況 3」）。

---

## 觸發條件

當使用者在客戶需求單工作區要求執行以下操作時，應載入本技能：

### 觸發關鍵字

- 新增客戶代碼
- 客戶代碼申請
- 代理商對應
- 區域語言配置
- 客戶索引
- 客戶代碼映射
- `CC_XXXXX` (代碼符號)

---

## 問題編號客戶縮寫表（Issue Number CUSTOMER 欄位）

> 縮寫對照表已移至 **instructions**，讓 AI 在所有場合自動套用，不需手動觸發技能：
> → [`instructions/customer-code-table.instructions.md`](d:\.github\instructions\customer-code-table.instructions.md)

新客戶加入時，請在上面的 instructions 檔的「客戶縮寫表」中插入一行，再依下方步驟更新其他檔案。

> 問題編號格式規則詳見：[`references/issue-number-format/issue-number-format.md`](../../references/issue-number-format/issue-number-format.md)

---

## 自動修改的檔案

當使用者新增客戶代碼時，本技能將在以下檔案中進行配置：

| 檔案 | 位置 | 修改內容 | 說明 |
|------|------|---------|------|
| **customer-code-table.instructions.md** | `.github/instructions/customer-code-table.instructions.md` | 客戶縮寫表 | 新增縮寫 + 代理商對應（**唯一來源**） |
| **CustomerReq.agent.md** | `.github/agents/CustomerReq.agent.md` | 更新郵件路由規則 | 若為新代理商則添加 |

---

## 修改流程詳解

### 步驟 1：客戶信息驗證

從 HT9045 端傳遞的客戶代碼資訊中提取：

| 欄位 | 來源 | 說明 |
|------|------|------|
| **Code Number** | MachineType.h | 客戶代碼（3~5位數字） |
| **Customer Name** | 註解 | 客戶英文名稱 |
| **Region** | 註解 | 地區（新加坡、日本等） |
| **Distributor** | （推定） | 代理商（基於地區自動推定） |

### 步驟 2：地區→代理商映射

根據地區自動推定代理商：

| 地區 | 代理商 | 語言配置 |
|------|--------|----------|
| Korea | TeraTech | 英文 + 韓文 |
| Singapore / Malaysia | HTS | 繁體中文 + 英文 |
| Philippines / Thailand | JB-Elite | 英文 |
| Japan | Spandnix | 繁體中文 + 日文 |
| China | 鴻勁興業 | 繁體中文 |
| Taiwan | HPI-TW | 繁體中文 |
| USA / Canada | HPI-USA | 英文 |
| Europe | HPI-Euro | 英文 |
| — (無法判定) | ETC | — |

### 步驟 3：更新客戶縮寫表（唯一來源）

**檔案**：`.github/instructions/customer-code-table.instructions.md`

**查找錨點**：「客戶縮寫表」表格

**位置確認**：表格按客戶代碼 / 縮寫排列

**插入格式**：
```markdown
| `AMD_SG` | AMD 新加坡（807_AMD_SG） | Singapore | HTS |
```

**規則**：
- 若地區為空，填入 `—`
- 若代理商為空，填入 `—`
- 客戶名稱使用 SnakeCase 或 CamelCase（與 MachineType.h 保持一致）
- 必須與程式碼端 `CC_` Symbol 保持一致

### 步驟 4：郵件路由規則更新（若需要）

若新增代理商信箱域名，需更新 `CustomerReq.agent.md` 中的路由規則：

**檔案**：`.github/agents/CustomerReq.agent.md`

**查找錨點**：「Step 2：判定提案來源與路由目標資料夾」之「信箱網域表」

**插入格式**（若為新代理商）：
```markdown
| `@distributor.com` | ☑ 客戶端 | `Distributor\` | 新代理商郵件自動路由 |
```

> ⚠️ 通常只新增客戶代碼不需修改此表，除非客戶代碼對應全新的代理商。

---

## 使用範例

### 範例 1：新增新加坡 AMD 客戶代碼

**HT9045 端執行**（先執行）：
```
新增客戶代碼 #define CC_AMD_SG 807 //AMD 新加坡
```

**CustomerReq 端執行**（後執行）：
```
@customer-code-manager 同步客戶代碼 807
或
@customer-code-manager 新增 AMD_SG 807 Singapore
```

**自動修改**：

**customer-code-table.instructions.md**（插入到客戶縮寫表）:
```markdown
| `AMD_SG` | AMD 新加坡（807_AMD_SG） | Singapore | HTS |
```

---

## 客戶代碼編號申請流程

若使用者要求申請新客戶代碼編號（尚未在 HT9045 中定義），本技能應：

1. **掃描可用編號**
   - 查詢 `customer-code-table.instructions.md` 中已用編號（並與程式碼端 MachineType.h 對照）
   - 提議下一個可用編號（通常遞增 +1 或指定特定號段）

2. **建議代理商與語言**
   - 基於客戶所在地區自動推薦代理商
   - 提議區域語言配置

3. **生成申請建議**
   - 為 HT9045 端提供建議的 #define 語句
   - 為 CustomerReq 端提供建議的索引表項目

### 範例：客戶代碼申請

**使用者輸入**：
```
申請新客戶代碼：AI 技術（新加坡地區）
```

**系統建議**：
```
推薦客戶代碼：807（下一個可用編號）
推薦客戶名稱：AI_SG
推薦代理商：HTS
推薦語言：繁體中文 + 英文

建議 HT9045 端添加：
#define CC_AI_SG                807 //AI 新加坡

建議 CustomerReq 端添加：
| 807 | AI_SG | Singapore | HTS |
```

---

## 驗證規則

新增客戶代碼後執行自動驗證：

1. **唯一性檢查**
   - 確認客戶代碼編號未重複
   - 確認客戶名稱未重複（允許不同地區的同一公司）

2. **排序檢查**
   - 驗證表格按客戶代碼升序排列

3. **代理商驗證**
   - 確認代理商名稱與現有清單相符
   - 若為新代理商，建議先更新代理商清單

4. **語言檢查**
   - 驗證區域語言配置與代理商規則一致

5. **格式檢查**
   - 確認 Markdown 表格格式正確
   - 確認無多餘空格或特殊字元

---

## 特殊情況處理

### 情況 1：客戶代碼已存在但代理商不同

**場景**：同一客戶服務多個代理商

**處理**：
- 建議使用新的客戶代碼（如 808A、808B）
- 或在客戶名稱中區分地區（如 AMD_SG、AMD_JP）

### 情況 2：地區無法自動推定代理商

**場景**：客戶所在地區為罕見國家或未列表

**處理**：
- 提示使用者手動指定代理商
- 或提示將此客戶歸為 `ETC` 類別

### 情況 3：新增客戶代碼時 HT9045 端尚未同步

**場景**：使用者只在 CustomerReq 端新增，MachineType.h 尚未更新

**處理**：
- 提示：「檢測到 HT9045 端尚未新增此客戶代碼」
- 建議先在 HT9045 端執行 @ht9045-customer-code-manager 技能

---

## 依賴技能與資源

| 技能/資源 | 位置 | 用途 |
|----------|------|------|
| **ht9045-customer-code-manager** | `d:\HT9045\.github\skills\ht9045-customer-code-manager\SKILL.md` | 程式碼端雙向同步對應技能（`CC_` 定義、FUNC、UI） |
| **customer-code-table.instructions.md** | `.github\instructions\customer-code-table.instructions.md` | 客戶縮寫/代理商/語言唯一來源 |
| **CustomerReq Agent** | `.github\agents\CustomerReq.agent.md` | 路由規則與提案表流程 |

---

## 檔案編輯規則

- **格式**：Markdown
- **編碼**：UTF-8
- **行尾**：LF（Unix style）
- **表格對齐**：Markdown 標準格式（`|` 分隔）

---

## 回滾與版本控制

若新增客戶代碼後需撤銷：

```
@customer-code-manager 撤銷客戶代碼 807
```

**回滾流程**：
1. 從 `customer-code-table.instructions.md` 的客戶縮寫表中刪除該行
2. 若有郵件路由規則變更，恢復原配置
3. 同步提示 HT9045 端撤銷對應的 #define 和函數

---

## 相關檔案位置

| 檔案 | 路徑 | 說明 |
|------|------|------|
| 客戶縮寫表（唯一來源） | `.github/instructions/customer-code-table.instructions.md` | 客戶縮寫/代理商/語言索引 |
| CustomerReq Agent | `.github/agents/CustomerReq.agent.md` | 代理商路由規則 |
| 客戶代碼來源 | `d:\HT9045\.github\skills\ht9045-customer-code-manager\SKILL.md` | HT9045 程式碼端雙向同步技能 |

---

**最後更新**：2026-04-27  
**版本**：1.1.0  
**維護者**：Steven Chou
