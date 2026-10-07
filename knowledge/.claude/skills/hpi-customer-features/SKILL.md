---
name: hpi-customer-features
description: "Handler客戶功能跨主題索引；查CUSTOMER_CODE、CC_與FUNC_CC_分支、V912／V906客戶碼差異、function候選與人工客戶表、孤兒／待補定位。整合HT9050及其他機型，分清程式符號、報告代理商／語言及機台runtime身分。"
---

# Handler客戶功能跨主題索引

同一入口整合HT9050與其他Handler；先分版本／客戶／機型，再讀所屬主題。

## 先確認

- [共同查證流程](references/common.md)：CC符號、比較條件與caller是不同層次；搜尋命中只作候選。
- [資料權威](references/authority.md)：程式數值、Factory顯示名稱、報告代理商／語言分開查，不由名字或機型推定。
- [機型分流](references/machines.md)：HT9050與其他機型共用方法；作用中CUSTOMER_CODE、MachineTypeChoice與runtime值另核對。
- [版本索引](references/versions/index.md)保留各版本符號／別名及source commit；沒有命中不能宣稱912-only。
- 活文件以版本／檔名＋function／關鍵變數定位；原文與裁決沿用各主題保存樹。

## 按用途選路

| 問題 | Reference |
|---|---|
| 已寫入主題知識的客戶差異 | [跨主題路由](references/topics/index.md)，逐列讀原表的查證界線 |
| 客戶碼讀寫／功能入口、ART／名稱局部查證 | [人工核對樹](references/reviewed/index.md)，局部靜態結論與候選分開 |
| 某個CC符號出現在哪些function | [客戶符號樹](references/customers/index.md)，候選與人工列分開 |
| V912與V906數值／符號是否相同 | [分版本定義](references/versions/index.md)，不把改名或別名合成同一客戶 |
| 重新產生候選或檢查未分類項 | [唯讀scanner](references/scanner.md)／[待補與孤兒](references/pending.md) |
| 新增客戶、名稱／報告雙向同步 | [Config客戶流程](../hpi-config/references/customer/index.md)／[權威](references/authority.md) |

來源更新：[最新main盤點](references/main-integration-20261007-114x.md)／[前次保存核對](references/main-integration-20261006-222x.md)，原候選、人工日期與新掃描摘要分開。

密碼簿算法說明的[公開發布界線](references/reviewed/publication/index.md)：固定排除公開同步，原文仍保留在公司私有 repo。
