---
name: ht9045-customer-code-manager
description: >
  HT9045 客戶代碼自動管理技能。當使用者提供新客戶代碼定義（#define CC_xxx NNN //註解）時，
  自動在 MachineType.h、CosFunction.cpp（FUNC 定義 + DoCustomerFunction case 分支）、
  HandlerSys.dfm（rgCustomerList UI 選項）、Customer-Code-Distributor SKILL 中插入對應資料。
  支援自動格式化、代碼生成、地區→代理商推定、Big5 編碼保護、排序驗證、批量新增與回滾。
  關鍵字：客戶代碼, customer code, CC_, FUNC_CC_, MachineType.h, CosFunction.cpp,
  HandlerSys.dfm, DoCustomerFunction, rgCustomerList, 新增客戶, 客戶功能選擇,
  代理商, 鴻勁興業, TeraTech, HTS, JB-Elite, 代碼排序,
  新客戶配置, 批量新增, 回滾, 撤銷
---

# HT9045 客戶代碼自動管理技能

當使用者要求新增客戶代碼時，本技能自動在相關檔案中插入必要的定義和程式碼片段。

## 技能分工與雙向同步

客戶代碼由**兩個技能分工**維護，兩者**互為上下游、必須雙向同步**：

| 技能 | 工作區 | 負責範圍 |
|------|--------|---------|
| **ht9045-customer-code-manager**（本技能） | `d:\HT9045\.github` | **程式碼端**：`#define CC_xxx`（MachineType.h）、`FUNC_CC_xxx` + `case`（CosFunction.cpp）、`rgCustomerList`（HandlerSys.dfm） |
| **customer-code-manager**（報告端） | `d:\.github\skills\make-report-skill\customer-code\customer-code-manager\SKILL.md` | **報告/索引端**：客戶縮寫表（`customer-code-table.instructions.md`，唯一來源）、地區→代理商→語言映射、Release Note 客戶路由 |

> **同步規則**：
> - **本技能（程式碼端）為代碼數值與英文符號的來源**（Code Number / `CC_` Symbol）。新增 `CC_` 後，**必須**通知報告端在 `customer-code-table.instructions.md` 補上對應縮寫/代理商/語言。
> - **報告端為代理商與語言配置的來源**。撰寫報告或判定客戶代理商時，一律以報告端 instructions 表為準，不在本技能重複定義代理商清單。
> - 任一端新增客戶，缺少另一端對應 = 未完成。詳見報告端 SKILL 的「特殊情況處理 → 情況 3」。

## 觸發條件

以下任一格式：

```
新增客戶代碼：#define CC_AMD_SG               807 //AMD 新加坡
新增客戶 AMD (代碼=807, 地區=新加坡)
@ht9045-customer-code-manager 新增 CC_AMD_SG 807 AMD_SG
```

## 自動修改的檔案

| 檔案 | 修改內容 | 說明 |
|------|---------|------|
| **MachineType.h** | `#define CC_XXXXX YYY //註解` | 客戶代碼常數定義（按數值升序插入） |
| **CosFunction.cpp** | `void FUNC_CC_XXXXX() { }` + `case CC_XXXXX:` | 功能函數 + switch-case 分支 |
| **HandlerSys.dfm** | rgCustomerList Items.Strings 新增項目 | UI 客戶選擇列表（按代碼升序，含 Unicode 轉義） |
| **報告端縮寫表**（雙向同步） | `customer-code-table.instructions.md` 新增一行 | 客戶縮寫 + 代理商 + 語言對應（報告端 customer-code-manager 唯一來源） |

## 修改流程概覽

1. **提取客戶資訊** — 從 `#define CC_xxx NNN //註解` 解析 Symbol/Number/Comment
2. **定位最新版本** — 掃描 `d:\HT9045\` 找最新 `HT9011UC_Code_V3.33.*`
3. **MachineType.h** — 按數值升序插入 `#define CC_xxx` 行
4. **CosFunction.cpp FUNC** — 前一客戶 FUNC 結束行後插入空函數
5. **CosFunction.cpp case** — `DoCustomerFunction()` switch 末尾插入 case
6. **HandlerSys.dfm** — `rgCustomerList` Items.Strings 按數值升序插入（含 `dfm_escape()` Unicode 轉義）
7. **報告端縮寫表**（雙向同步）— 通知報告端 customer-code-manager 在 `customer-code-table.instructions.md` 補上縮寫（地區→代理商→語言自動推定）

> 詳細步驟、Python 一鍵腳本、使用範例、特殊情況處理、回滾流程：
> **[references/customer-code-workflow.md](references/customer-code-workflow.md)**

## 驗證步驟

1. **語法檢查** — #define 格式、函數簽名、case 語法
2. **編碼檢查** — 三個檔案均保持 Big5（CP950）編碼
3. **排序檢查** — MachineType.h / CosFunction.cpp / HandlerSys.dfm 按代碼升序
4. **重複檢查** — 新客戶代碼未重複定義
5. **編譯驗證** — BCB6 專案編譯確認

## 執行限制

- 所有檔案中的客戶代碼**必須按升序排列**
- 必須保持 Big5（CP950）編碼，禁止 UTF-8
- 新增後建議立即提交 SVN 並編譯驗證

## V906 移植樹（C++）的客戶名稱（20261001，St01，todo D-037／E-BOOT-005）

- 移植樹 D:\HT9045\HT9011UC_Cpp_V3.33.906.0 沒有 `HandlerSystem` 全域；客戶清單是具名替身 `EL<TRadioGroup>("THandlerSystem","rgCustomerList")`，
  內容由產生器 `tools/gen_editlist.py`（設定 `tools/editlist/HSys.py`）從 golden V912 `HandlerSys.dfm` 轉成 `FileRW/HSys.gen.inc`（213 列，DFM 順序），
  開機時 `FileRW_HSys_Boot()` → `HS_DfmItems()` 填好（不用開 HandlerSys 頁）。
- golden `THandlerSystem::GetCustomerName`（V912 `HandlerSys.cpp:1242-1272`）的移植＝`FileRW/HSys.cpp` 檔尾 `AnsiString FileRW_HSys_CustomerName()`：
  逐列比前三碼＝CUSTOMER_CODE，第一個相同的列；912＋SPIL_FOR_QLE＝"QLE"、730＝"Infineon"、731＝"CARSEM"；其他取第 5 字到第一個半形空格；找不到＝"HonPrec"。
  **不看 ItemIndex**。golden 怪癖照留：895 BARUN／970 GIGAS 的 DFM 列名字後面是全形空格 ⇒ 回 ""。沒有 HSys.cpp 的程式用 `FileRW/_fallback.cpp` 檔尾那一行（"HonPrec"）。
- 用途：golden TfMain::FormShow（V912 main.cpp:11056-11057）`RunInfo.Factory`＋Observer `labFactory`；SECS SV 1005。開機那一行由筆電接（wb_serve 開機段）。
- **807／808／898 用 906 的名字**（20261002，St01，D-044；Jimmy `docs/RULINGS_20261002.md` 第 9 條「42 用 906 的對照表」）：
  `FileRW/HSys.cpp` 的 `W906_HSys906Row` 在逐列比對前把這三列換成 906 清單（移植樹 `HandlerSys.cpp:569` 那張 211 列，906_0618 DFM）的樣子——
  807（V912 `TFAMD_M`）、808（V912 `AMD_US`）906 沒有這兩列 ⇒ 跳過 ⇒ 找不到 ⇒ `"HonPrec"`；898 用 906 的列字 `898 AMD_SUZHOU ...` ⇒ `"AMD_SUZHOU"`（V912 是 `TFAMD_SUZHOU`）。
  只改這個查名字的函式；HandlerSys 頁面的清單（`FileRW/HSys.gen.inc`，V912 213 列）不動。之後 V912 的 DFM 再改名或加列時，要先問 Jimmy 用哪一張表。
  ctest `EBoot005_CustomerName`：在移植樹自己的 906 清單上，跟 `HandlerSys.cpp:821-852` 的翻譯每一個客戶碼都相同；在 V912 清單上只差 807／808／898。
- **新增客戶時**：golden 的 `HandlerSys.dfm` 加了列之後，移植樹要重跑 `tools/gen_editlist.py --only HSys` 更新 `FileRW/HSys.gen.inc`，否則 V906 回不出新客戶的名字；
  ctest `EBoot005_CustomerName` 釘了 213 列的順序，列數變了要一起改測試。移植樹檔案是 UTF-8（golden 是 Big5）。

## 依賴技能與資源

| 資源 | 位置 | 用途 |
|------|------|------|
| customer-code-manager（報告端） | `d:\.github\skills\make-report-skill\customer-code\customer-code-manager\SKILL.md` | 報告/索引端雙向同步對應技能（代理商、語言、客戶路由） |
| 客戶縮寫表（唯一來源） | `d:\.github\instructions\customer-code-table.instructions.md` | 客戶縮寫 + 代理商 + 語言對照表 |
| Big5 Encoding | `.copilot\instructions\big5-files.instructions.md` | 確保編碼一致性 |
| source-map | [references/source-map.md](references/source-map.md) | 檔案位置對照 |

---

**最後更新**：2026-06-04 | **版本**：1.2.0 | **維護者**：Steven Chou
