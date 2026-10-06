# Gerneral.ini規格同步

- [原完整工作流程](original-entry.md)：規格指示書、Model／Serial／Factory／CUSTOMER_CODE、ATC／Contact／sensor選項及欄位驗證。
- [原database mapping](references/database-mapping.md)／[HandlerSys mapping](references/handlersys-mapping.md)／[main mapping](references/main-mapping.md)／[cross reference](references/cross-reference.md)。
- [靜態Schema](../../../../../.github/specs/gerneral-ini-schema.md)：保留現有權威，不複製另一份。
- [目前資料層](../runtime/config.md)／[目前owner](../runtime/writers.md)／[機型與快照](../machines/index.md)。

原流程提到rev897_base或掃最新版本是原年代步驟，現行修正目標與唯讀邊界依AGENTS／write-boundary-policy；分析其他版本仍可讀。機台專屬檔版控走machines，不放system／config。修改規格文件與安裝runtime是兩個動作，本批只重整Skill。
