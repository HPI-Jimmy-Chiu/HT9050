# AutoClean流程與原文

- [原完整主體](original-entry.md)：trigger bits、eCKPos、資料狀態、Clean Pad、計數、OneCycle與客戶flags。
- [Task分層](tasks/index.md)：主協調器、Pick／Place、Shuttle與Index接觸循環。
- [完整函式清冊](references/function-registry.md)：原先各版本行號是歷史，查目前用function名／Task／變數。
- [目前入口](../runtime/cleaning.md)：Cleaning／Status、GPIB／SECS、E-030與iOneCycle守衛。
- [HT9050乾跑](../machines/index.md)：On接管tick時不跑正常AutoClean階梯，不由bRunAutoClean一個flag判斷動作已執行。

保留原文的V899定位方便唯讀對照；修改／交付源碼仍依AGENTS與目前版本邊界。此Skill整理只改文件。
