# Alarm 顯示與資料來源

- [位置輸入表](../maps/index.md)：arguments.position → panel／unit → 各機型mv；Code單元與位置另做一致性核對。
- [目前main](../runtime/current-main.md)：machineId、resolve、deriveFromCode／fillFromCode的分流。
- [原文](../source/original-entry.md)：ErrShowToForm、AlarmType、Code.SubString、AlarmCodeMap、reDescription的完整歷史與Golden查證；舊flushPanel永遠null等描述有日期，不能脫離當時版本套用。
- [MotionView共同入口](../../../hpi-motionview/SKILL.md)：配置／幾何／Teach與狀態來源；告警overlay不代表馬達位置已量測。

Code文本、unitNo、position紅框、語系.dat與AlarmCode手冊分別確認。不要以手冊全文冒充程式實際讀取的說明檔。
