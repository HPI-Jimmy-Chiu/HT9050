# Issue Number Format（問題編號規則）

## 格式定義

```
#[T][YYMMDD]-[CUSTOMER]-[MC]-[NN]
```

| 欄位 | 長度 | 說明 | 範例 |
|------|------|------|------|
| `#` | 1 | 固定前綴符號 | `#` |
| `T` | 1 | 類型代碼（見下方） | `P` |
| `YYMMDD` | 6 | 建立日期（年2碼+月2碼+日2碼） | `260427` |
| `-` | — | 分隔符 | — |
| `CUSTOMER` | 3~6 | 客戶代碼縮寫（見下方） | `ATK` |
| `-` | — | 分隔符 | — |
| `MC` | 2~8 | 機台系列代碼（見下方） | `H9` |
| `-` | — | 分隔符 | — |
| `NN` | 2 | 序號，每天同客戶同類型從 `01` 重置 | `01` |

---

## 類型代碼（T）

| 代碼 | 意義 | 說明 |
|------|------|------|
| `P` | Problem | 問題回報，客戶反映的 Bug 或異常 |
| `R` | Request | 功能需求，客戶要求新增或修改功能 |

---

## 機台系列代碼（MC）

| 代碼 | 機台型號 | 說明 |
|------|----------|------|
| `H9` | HT9xxx（通用） | HT9045 / HT9046 等 9000 系列，不限特定型號 |
| `H1` | HT10xx（通用） | HT1032 等 10000 系列 |


---

## 客戶代碼（CUSTOMER）

> 客戶代碼縮寫的**完整定義與維護**統一由以下 Instructions 負責，本文件不重複維護：
> → [`references/customer-code-table/customer-code-table.instructions.md`](D:\HT9045\.claude\skills\make-report-skill\references\customer-code-table\customer-code-table.instructions.md)

新客戶加入時，請至 `customer-code-table.instructions.md` 登記縮寫，並依 [customer-code-manager SKILL](../../customer-code/customer-code-manager/SKILL.md) 步驟執行。


---

## 序號規則（NN）

- 每天、同客戶、同類型（P/R）各自從 `01` 重置
- 同一天同客戶若有 2 筆 Problem → `P260427-ATK-H9-01`、`P260427-ATK-H9-02`
- `P` 和 `R` 序號**獨立計算**，不互相影響

---

## 完整行格式

```
#[問題編號], [第2欄], [說明]
```

### 第2欄規則

| 情況 | 填寫內容 | 格式 | 範例 |
|------|----------|------|------|
| 軟體相關 | 發生時的軟體版本號 | `Vx.xx.xxx.x` | `V3.21.895.5` |
| 硬體相關 | 影響範圍 | 自由文字 | `All handlers` |
| 不確定 | 省略 | — | — |

---

## 範例

```
#P260427-ATK-H9-01, V3.21.895.5, Auto Clean count issue.
#R251127-ATK-H9-01, All handlers, Request to exchange Picker for In/OutArm
```

---

## 索引登記

所有問題編號應同步登記至：
→ `<repo>\public\Docs\customers\issue-index.md`（20260929 之前的在 `D:\docs\customers\issue-index.md`，內容相同）
