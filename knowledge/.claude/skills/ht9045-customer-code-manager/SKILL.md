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

# ht9045-customer-code-manager 相容入口

同主題已整合到 [hpi-config](../hpi-config/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-config/references/customer/original-entry.md)

## 技能分工與雙向同步

[讀取此節](../hpi-config/references/customer/original-entry.md#技能分工與雙向同步)

## 觸發條件

[讀取此節](../hpi-config/references/customer/original-entry.md#觸發條件)

## 自動修改的檔案

[讀取此節](../hpi-config/references/customer/original-entry.md#自動修改的檔案)

## 修改流程概覽

[讀取此節](../hpi-config/references/customer/original-entry.md#修改流程概覽)

## 驗證步驟

[讀取此節](../hpi-config/references/customer/original-entry.md#驗證步驟)

## 執行限制

[讀取此節](../hpi-config/references/customer/original-entry.md#執行限制)

## V906 移植樹（C++）的客戶名稱（20261001，St01，todo D-037／E-BOOT-005）

[讀取此節](../hpi-config/references/customer/original-entry.md#v906-移植樹c的客戶名稱20261001st01todo-d-037e-boot-005)

## 依賴技能與資源

[讀取此節](../hpi-config/references/customer/original-entry.md#依賴技能與資源)
