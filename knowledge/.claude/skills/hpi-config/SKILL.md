---
name: hpi-config
description: "HT9050與其他Handler的機台設定共同入口。處理IniConfig／HT9045_CONFIG、Config.h／elConfig／configByRecipe、Lock by File、功能群組、Gerneral.ini規格同步、Model／CUSTOMER_CODE、客戶碼與名稱／報告索引，以及教點升版與runtime owner／設定來源差異。"
---

# Handler機台設定／規格／客戶碼

同一Skill按資料層、版本、機型與客戶分流；HT9050與其他Handler共用查證方式，設定值另依機台核對。

## 先確認

- 讀 [共同資料層](references/common.md)，分清config.ini、Gerneral.ini、Recipe與機台鏡像，不能互相當預設。
- 活文件用版本／檔名＋function／變數；[原Config](references/config/index.md)保存舊版本完整知識，歷史行號不作目前主定位。
- [目前Web寫者](references/runtime/writers.md)：--allow-system-write已不選行為；config／Gerneral／teach等Cowner仍擋B路apply。
- [機型與快照](references/machines/index.md)先核對Model、來源時間及作用中檔案；舊Mot_Table_9050與客戶碼957不代表現在每台相同。
- [客戶與版本](references/customer/index.md)分清程式CC數值、RunInfo.Factory名稱與報告代理商／語言；預設修正目標依AGENTS的V912，不自動掃版本改檔。
- scripts、YAML／i18n、xlsx與產生器輸入欄位表留在原路徑；[資源與缺件](references/resources/index.md)標清Git未收錄項，不執行產生器。

## 按用途選路

| 問題 | Reference |
|---|---|
| IniConfig欄位、UI綁定、Lock／Recipe覆蓋 | [Config樹](references/config/index.md)／[目前資料層](references/runtime/config.md) |
| A～P功能、ECID、說明／多語手冊 | [欄位群組](references/config/fields/index.md)／[資源](references/resources/index.md) |
| 規格指示書、Gerneral／Model／ATC等 | [規格同步](references/general/index.md)／[靜態Schema](../../../.github/specs/gerneral-ini-schema.md) |
| 客戶新增、顯示名稱、報告雙向同步 | [客戶樹](references/customer/index.md)／[目前名稱](references/runtime/customer-name.md) |
| 教點升版／版本差異 | [teach.ini沿革](references/config/references/teach-ini-migration-trap.md)／[機型](references/machines/index.md) |
| 完整舊主體 | [Config](references/config/original-entry.md)／[General](references/general/original-entry.md)／[Customer](references/customer/original-entry.md) |

20261006推前更新：[HT9050／其他機型與caller](../hpi-customer-features/references/main-integration-20261006-2022.md)，舊原文與查證日期保持。
