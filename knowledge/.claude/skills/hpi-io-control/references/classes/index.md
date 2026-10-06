# IO物件reference樹

- [完整原類別與方法](../io/references/io-classes.md)：Cylinder／Sensor／Switch／Sucker／Kit／LaneIO。
- [disabled與缺列時的各層回值](../io/references/cylinder-sensor-layer.md)。
- [原IO入口與Task流程](../io/original-entry.md)。
- [目前實作補充](../runtime/current-main.md)：到位true、普通sensor false、Pad先分流、OutValue與OutPortData快取。

Push／Pop／Suck／Destroy是逐tick重呼叫的Task流程，原文的「阻塞式」不得直接理解成caller一次就完成；延遲、retry、alarm與實際感測分開查。
