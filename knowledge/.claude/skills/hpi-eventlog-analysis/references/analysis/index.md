# 資料與分析路由

- 單檔檢視與多日統計：[原正文第3／4節](../ela/original-entry.md#3-兩個同名的-geteventlogtext不要混淆)，原SPIL／欄位／10000列與日期來源保持。
- 切欄：main2db EventLogCsv.h的ela::SplitEventLogCsv及detail::AddEventLogCsvField以引號外逗號切欄、欄外Trim、尾端逗號保留空欄；只讀函式未跑parser。原BCB CommaText空白切欄、W15更正與VTEST選檔／去重、Options等見[原正文](../ela/original-entry.md)。
- MTBF／MTBA、Jam key、OEE：讀[原正文](../ela/original-entry.md)的來源與W45／W48／W61裁決；不得把舊random圖或缺Production_Log／TimeData的狀態補成實際可用OEE。
- 保存與變更：[原計畫](../ela/references/ela-web-conversion-plan.md)及[報表樹](../reports/index.md)；本次未重查全數學算法、Options構造預設值、檔案選取／consumer。

CSV產生者見[MyDB](../../../ht9045-mydb/SKILL.md)，State Record／hang不同來源見[State分析](../../../hpi-state-analysis/SKILL.md)。真檔路徑與原「只讀、複製fixture」規則保存；此批沒有讀runtime或建新fixture。
