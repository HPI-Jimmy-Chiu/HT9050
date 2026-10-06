# IO表格reference樹

- [V906讀表／綁定／覆寫／TableAudit](../io/references/table-loading-port.md)。
- [原Enable停用知識與PLC陷阱](../io/original-entry.md)：保留前提，不能用作本批修改安全IO表的指令。
- [吸嘴架構中的表格選配](../vacuum/original-entry.md)。

IO_CARD_TYPE決定讀表路徑，ISABase決定一列的backend；Cylinder輸出列／到位列、Sucker本體／_On／_Off、Sensor與SW各有不同Enable條件。改前核對InitialSwitch／InitialSensor／InitSucker／InitCylinder與AUTO_EMPTY_COLOR／PLC覆寫；本批只讀、不apply機台快照。
