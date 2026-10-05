# 客戶代碼管理 Skill 使用指南（CustomerReq 工作區）

## 📋 概述

本 Skill 負責客戶需求單工作區的客戶代碼配置與代理商管理。與 HT9045 端的 `ht9045-customer-code-manager` 技能協同工作。

---

## 🎯 核心職責

1. **客戶代碼索引管理**
   - 維護 `customer-code-table.instructions.md`（唯一來源）中的客戶縮寫表
   - 按客戶代碼 / 縮寫管理

2. **代理商映射**
   - 根據客戶地區自動推定代理商
   - 維護區域語言配置關係

3. **郵件路由配置**
   - 管理 CustomerReq.agent.md 中的代理商郵件路由規則
   - 支援多代理商自動分發

---

## 🚀 快速開始

### 方式 1：同步 HT9045 端新增的客戶代碼

HT9045 端新增了客戶代碼後，在本工作區執行：

```
同步客戶代碼 807
或
@customer-code-manager 同步 807
```

系統將自動：
1. 讀取 HT9045 端的相關定義
2. 從 MachineType.h 中提取客戶信息
3. 在 `customer-code-table.instructions.md` 中添加縮寫項
4. 驗證代理商與地區匹配

### 方式 2：手動新增客戶代碼

```
新增客戶代碼：AMD_SG 807 新加坡
或
@customer-code-manager 新增 807 AMD_SG Singapore
```

系統將：
1. 驗證客戶代碼唯一性
2. 推定代理商為 HTS（基於新加坡地區）
3. 更新客戶索引表
4. 完成配置

### 方式 3：查詢現有客戶代碼

```
查詢客戶代碼 807
或
@customer-code-manager 信息 AMD_SG
```

系統將返回：
- 客戶名稱、代碼、地區
- 對應代理商、語言配置
- HT9045 端的定義位置

---

## 📝 支援的客戶資訊格式

系統接受以下多種輸入格式：

### 格式 1：簡化版本
```
新增 AMD_SG 807 Singapore
```

### 格式 2：詳細版本
```
新增客戶代碼 CC_AMD_SG 807 AMD_SG Singapore HTS
```

### 格式 3：表格版本
```
| 807 | AMD_SG | Singapore | HTS |
```

### 格式 4：從 HT9045 同步
```
同步 d:\HT9045\HT9011UC_Code_V3.33.898.1_RogerYang_20260317
```

---

## ✅ 自動驗證規則

新增完成後系統執行：

1. **唯一性檢查**
   - ✓ 客戶代碼編號無重複
   - ✓ 客戶名稱無重複

2. **格式檢查**
   - ✓ 代碼為 3~5 位數字
   - ✓ 客戶名稱為英文名稱
   - ✓ 地區為有效地點

3. **排序檢查**
   - ✓ 表格按客戶代碼升序排列

4. **代理商驗證**
   - ✓ 代理商名稱合法
   - ✓ 地區與代理商匹配

---

## 🌍 地區→代理商自動映射

系統根據客戶地區自動推定代理商：

| 地區關鍵字 | 對應代理商 | 語言配置 | 郵件域名 |
|----------|---------|--------|--------|
| Korea, 韓國 | TeraTech | 英文 + 韓文 | @teratechkorea.com |
| Singapore, Malaysia, 新加坡, 馬來西亞 | HTS | 繁體中文 + 英文 | @htsepl.com |
| Philippines, Thailand, 菲律賓, 泰國 | JB-Elite | 英文 | @jb-elite.com |
| Japan, 日本 | Spandnix | 繁體中文 + 日文 | @spandnix.co.jp |
| China, Shanghai, Beijing, 中國, 上海, 北京 | 鴻勁興業 | 繁體中文 | — |
| Taiwan, 台灣 | HPI-TW | 繁體中文 | — |
| USA, Canada, 美國, 加拿大 | HPI-USA | 英文 | — |
| Europe, Germany, France, 歐洲, 德國, 法國 | HPI-Euro | 英文 | — |

---

## 📂 修改的檔案

### customer-code-table.instructions.md（唯一來源）

**檔案**：`D:\HT9045\.claude\skills\make-report-skill\references\customer-code-table\customer-code-table.instructions.md`

**修改區域**：「客戶縮寫表」

**插入位置**：按客戶代碼 / 縮寫排列

**範例**：
```markdown
| `STM` | STMicroelectronics（806_STM） | China | 鴻勁興業 |
| `AMD_SG` | AMD 新加坡（807_AMD_SG） | Singapore | HTS |
```

### CustomerReq.agent.md

**檔案**：`.github/agents/CustomerReq.agent.md`

**修改區域**：郵件路由表（若新增新代理商時）

**規則**：僅在引入全新代理商域名時才需修改

---

## 🔄 與 HT9045 端的同步

### 自動同步流程

```
HT9045 端執行
↓
新增 #define CC_AMD_SG 807 到 MachineType.h
↓
系統自動檢測到新增
↓
CustomerReq 工作區
↓
@customer-code-manager 自動同步
↓
customer-code-table.instructions.md 已更新
↓
同步完成 ✓
```

### 手動同步

若自動同步失敗，手動觸發：

```
@customer-code-manager 同步 HT9045 端新增
```

### 衝突檢測

若 HT9045 端和 CustomerReq 端定義不一致，系統提示：

```
⚠️  檢測到衝突：
  HT9045 端：CC_AMD_SG = 807
  CustomerReq 端：CC_AMD_SG = 808 (索引表有誤)

建議：
1. 手動驗證 HT9045 端的 MachineType.h（正確來源）
2. 執行修復：@customer-code-manager 修復 807
3. 確認 CustomerReq 端已更新
```

---

## 🎓 使用範例

### 範例 1：同步新加坡 AMD 客戶（代碼 807）

**步驟 1**：HT9045 端新增（已完成）
```
新增客戶代碼 #define CC_AMD_SG 807 //AMD 新加坡
```

**步驟 2**：CustomerReq 端同步
```
同步客戶代碼 807
```

**系統執行**：
1. 讀取 HT9045 端的 MachineType.h
2. 提取：CC_AMD_SG = 807, "AMD 新加坡"
3. 推定代理商：HTS（新加坡地區）
4. 推定語言：繁體中文 + 英文
5. 在 `customer-code-table.instructions.md` 插入：`| `AMD_SG` | AMD 新加坡（807_AMD_SG） | Singapore | HTS |`

**驗證**：✓ 完成

### 範例 2：批量同步多個客戶代碼

**HT9045 端新增**（已完成）：
```
1. #define CC_AI_SG 1000 //AI 新加坡
2. #define CC_NPU_JP 1001 //NPU 日本
3. #define CC_ML_US 1002 //ML 美國
```

**CustomerReq 端同步**：
```
同步客戶代碼 1000 1001 1002
或
@customer-code-manager 批量同步 1000-1002
```

**結果**：
```
| 1000 | AI_SG | Singapore | HTS |
| 1001 | NPU_JP | Japan | Spandnix |
| 1002 | ML_US | USA | HPI-USA |
```

---

## ⚠️ 常見問題

### Q：新增客戶時提示「無法推定代理商」

**A**：地區資訊不在已知清單中

**解決**：
```
新增 AMD_SG 807 Singapore --distributor HTS
```

手動指定代理商。

### Q：修改後客戶縮寫表中的表格格式亂掉了

**A**：可能是 Markdown 表格格式錯誤

**解決**：
```
@customer-code-manager 修復格式
```

系統自動重新排版。

### Q：想撤銷之前新增的客戶代碼

**A**：執行撤銷指令

```
@customer-code-manager 撤銷 807
```

系統將：
1. 從 `customer-code-table.instructions.md` 中刪除該行
2. 提示 HT9045 端也進行撤銷
3. 確認雙端同步完成

---

## 📚 相關資源

| 名稱 | 位置 | 說明 |
|------|------|------|
| **customer-code-table.instructions.md** | `D:\HT9045\.claude\skills\make-report-skill\references\customer-code-table\customer-code-table.instructions.md` | 客戶縮寫/代理商/語言唯一來源 |
| **customer-code-manager SKILL** | `.github/skills/make-report-skill/customer-code/customer-code-manager/SKILL.md` | 本技能詳細文檔 |
| **ht9045-customer-code-manager SKILL** | `d:\HT9045\.github\skills\ht9045-customer-code-manager\SKILL.md` | HT9045 程式碼端雙向同步技能 |
| **CustomerReq Agent** | `.github/agents/CustomerReq.agent.md` | 代理商路由定義 |

---

**建立日期**：2026-03-30  
**最後更新**：2026-06-04  
**版本**：1.1.0  
**作者**：Steven Chou
