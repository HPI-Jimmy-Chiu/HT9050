# 客戶代碼管理 Skill 使用指南

## 📋 概述

已為您建立了一套完整的**客戶代碼自動管理系統**，用於在 HT9045 和客戶需求單工作區中自動新增和維護客戶代碼。

---

## 🎯 核心功能

### HT9045 工作區 — `ht9045-customer-code-manager`

自動在以下檔案中插入客戶代碼定義：
- **MachineType.h**：新增 `#define CC_XXXXX YYY` 常數定義
- **CosFunction.cpp**：新增 `void FUNC_CC_XXXXX() {}` 功能函數
- **HandlerSys.dfm**：新增客戶選項到表單 UI

### 客戶需求單工作區 — `customer-code-manager`

自動在以下檔案中更新客戶配置：
- **customer-code-distributor SKILL**：更新客戶代碼索引表
- **CustomerReq.agent.md**：新增郵件路由規則（若需要）

---

## 🚀 快速開始

### 方式 1：直接告訴系統新增客戶代碼

在 **HT9045 工作區** 輸入：
```
新增客戶代碼：#define CC_AMD_SG 807 //AMD 新加坡
```

系統將：
1. 自動在 MachineType.h 中添加該定義（按代碼升序排列）
2. 在 CosFunction.cpp 中新增對應的函數骨架
3. 驗證編碼、格式、排序
4. 提示在客戶需求單工作區同步

然後在 **客戶需求單工作區** 輸入：
```
同步客戶代碼 807
或
新增 AMD_SG 807 Singapore
```

系統將：
1. 自動在 customer-code-distributor SKILL 的索引表中添加該客戶
2. 推定代理商為 HTS（新加坡地區）
3. 驗證無誤後完成同步

### 方式 2：使用簡化指令

```
@ht9045-customer-code-manager 新增 CC_AMD_SG 807 AMD_SG
```

或

```
@customer-code-manager 新增 AMD_SG 807 Singapore
```

### 方式 3：批量新增多個客戶代碼

```
新增以下客戶代碼：
1. #define CC_AI_SG 1000 //AI 新加坡
2. #define CC_NPU_JP 1001 //NPU 日本
3. #define CC_ML_US 1002 //ML 美國
```

系統將依序處理所有三個客戶代碼。

---

## 📝 新增客戶代碼的必填資訊

系統需要以下資訊才能完成新增：

| 資訊 | 格式 | 範例 | 說明 |
|------|------|------|------|
| **代碼符號** | `CC_XXXXX` | `CC_AMD_SG` | #define 常數名稱（必須大寫加底線） |
| **代碼號碼** | 3~5 位整數 | `807` | 客戶編號（通常按升序遞增） |
| **客戶名稱** | 英文名稱 | `AMD` | 客戶公司名稱（代碼符號取決於此） |
| **地區** | 具體地名 | `Singapore` | 客戶所在地區（用於推定代理商） |

---

## 📂 修改的檔案位置

### HT9045 端

```
d:\HT9045\
├── HT9011UC_Code_V3.33.XXX.Z_YYYYMMDD\
│   ├── MachineType.h              ← 客戶代碼定義
│   ├── CosFunction.cpp            ← 客戶功能函數
│   └── HandlerSys.dfm             ← 表單 UI
└── ...
```

### 客戶需求單端

```
u:\共用區\客戶需求單\
├── .github\skills\
│   └── customer-code-distributor\
│       └── SKILL.md               ← 客戶代碼索引表
└── .github\agents\
    └── CustomerReq.agent.md       ← 郵件路由規則
```

---

## ✅ 驗證與檢查

新增完成後系統自動執行：

1. **語法驗證**
   - ✓ #define 格式正確
   - ✓ 函數簽名完整
   - ✓ 括號、分號無誤

2. **編碼驗證**
   - ✓ 檔案保持 Big5（CP950）編碼
   - ✓ 無 UTF-8 亂碼

3. **排序驗證**
   - ✓ 客戶代碼按升序排列
   - ✓ 表格行序正確

4. **唯一性驗證**
   - ✓ 代碼編號無重複
   - ✓ 常數名稱無重複

5. **編譯驗證**（建議）
   - 手動在 BCB6 編譯驗證無誤

---

## 🔄 雙向同步流程

### 流程 1：先在 HT9045 端新增，再在 CustomerReq 端同步

```
HT9045 端
↓
新增客戶代碼到 MachineType.h + CosFunction.cpp
↓
生成「同步號令」
↓
CustomerReq 端
↓
執行同步（自動更新 customer-code-distributor）
↓
完成
```

### 流程 2：先在 CustomerReq 端申請，再在 HT9045 端實現

```
CustomerReq 端
↓
申請客戶代碼（系統推薦編號、名稱、代理商）
↓
生成「建議值」給 HT9045 端
↓
HT9045 端
↓
按建議新增到 MachineType.h + CosFunction.cpp
↓
自動同步到 CustomerReq 端
↓
完成
```

---

## ⚠️ 常見問題與解決

### Q1：新增客戶代碼時出現「代碼已存在」

**原因**：該客戶代碼號已在檔案中定義過

**解決**：
- 查詢 MachineType.h 確認現有代碼
- 選擇新的代碼號（如 808、809 等）
- 或確認是否需要覆蓋現有定義

### Q2：新增後編譯出錯

**原因**：
- 大小寫不一致（#define 常數應為大寫）
- 編碼轉換為 UTF-8
- 函數簽名錯誤

**解決**：
- 確認 MachineType.h 中的常數名稱全為大寫
- 手動檢查編碼（應為 Big5）
- 查看 CosFunction.cpp 函數簽名是否完整

### Q3：客戶代碼排序錯亂

**原因**：新增代碼時未按升序排列

**解決**：
- 系統自動檢查排序
- 若有不一致，會提示並自動重新排列

### Q4：無法自動推定代理商

**原因**：客戶所在地區不在已知清單中

**解決**：
- 手動指定代理商（如 TeraTech、HTS 等）
- 或將該客戶歸為 ETC（未分類）
- 後續可更新到 customer-code-distributor 中

---

## 🔧 進階用法

### 批量新增

```
@ht9045-customer-code-manager 批量新增
CC_AI_SG 1000 AI Singapore
CC_NPU_JP 1001 NPU Japan
CC_ML_US 1002 ML USA
```

### 指定目標版本

```
@ht9045-customer-code-manager 新增 CC_AMD_SG 807 --version V3.33.899.0 --customer AMD_SG
```

### 撤銷客戶代碼

```
@ht9045-customer-code-manager 撤銷 CC_AMD_SG
```

將刪除所有相關定義和配置。

### 同步檢查

```
@customer-code-manager 檢查同步狀態
```

驗證 HT9045 端和 CustomerReq 端的客戶代碼是否一致。

---

## 📚 相關技能與資源

| 技能 | 位置 | 用途 |
|------|------|------|
| **ht9045-customer-code-manager** | `d:\HT9045\.github\skills\ht9045-customer-code-manager\SKILL.md` | HT9045 端客戶代碼管理 |
| **customer-code-manager** | `u:\共用區\客戶需求單\.github\skills\customer-code-manager\SKILL.md` | CustomerReq 端客戶代碼管理 |
| **customer-code-distributor** | `u:\共用區\客戶需求單\.github\skills\customer-code-distributor\SKILL.md` | 客戶代碼索引資料庫 |
| **AGENTS.md** | `d:\HT9045\.github\AGENTS.md` | HT9045 專案總覽 |

---

## 🎓 學習路徑

### 級別 1：基礎使用
1. 閱讀本指南的「快速開始」部分
2. 嘗試新增一個簡單的客戶代碼
3. 驗證修改是否正確

### 級別 2：進階操作
1. 了解修改流程詳解
2. 嘗試批量新增
3. 学習撤銷與回滾

### 級別 3：深度理解
1. 研讀 SKILL.md 完整文檔
2. 理解代理商地區映射規則
3. 掌握編碼與格式規範

---

## 📞 獲取幫助

### 查看技能詳細文檔

HT9045 工作區：
```
@Ask ht9045-customer-code-manager 的完整功能有哪些？
```

客戶需求單工作區：
```
@Ask customer-code-manager 的使用方式
```

### 特定問題解答

```
@Ask 如何在新加坡新增一個 AMD 客戶代碼？
@Ask 客戶代碼定義錯誤了，怎麼撤銷？
@Ask HT9045 和 CustomerReq 的客戶代碼如何同步？
```

---

**建立日期**：2026-03-30  
**版本**：1.0.0  
**作者**：Steven Chou
