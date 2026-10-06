# IO共同項與差異

只讀基準main `372b91908`，20261006。共用物件介面不代表各機台有相同硬體、拓樸或Enable行為。

| 層 | 共同項 | 差異／界線 |
|---|---|---|
| 表格 | Alias綁定、地址、Enable、Type需分開追 | IO_CARD_TYPE與ISABase不是同一列舉；初始化可能覆寫表值 |
| 氣缸 | 輸出、On／Off感測、Push／Pop Task分層 | Enable=false的OnSensor／OffSensor回true，不能說實體已到位 |
| Sensor | IsOn／IsOff依Type及讀取來源 | 普通disabled回false；iControlPanelMode的Pad鍵先分流，不能一概而論 |
| Switch | On／Off與快取／回讀不同 | 普通disabled不輸出但OutValue已更新；Pad按鈕走通訊面板 |
| 1203 | 以Ring／station／channel路由DI、DO | Port帶channel，Bit在RouteReadBit／RouteWriteBit不参與；byte API又不同 |
| 其他IO卡 | 同一TLaneIO介面與表格驅動 | MotionNet／MN200與ISA／PLC各自地址語意，不能照1203忽略Bit |
| 吸嘴 | 真空／破壞、感測、Task與Item矩陣分開 | HT9050四嘴合吸一IC；其他配置的多IC／Pitch／iPickStep不能互換 |
| 畫面／退出 | 追caller、gate、回覆與硬體結果 | VCL modal案例≠wb_serve HTTP；Q44裁決≠早期全DO清零方案 |

[目前main](runtime/current-main.md)／[機型](machines/index.md)／[類別](classes/index.md)。

通用分層評估：[標頭瘦身與 Motor／IO 隔離](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ee4ed4df7a506badb6cb0ce72769eaeb8b2aebb5/.claude/skills/cpp_build/references/header-slimming-and-motor-io-isolation-20261006.md)（ST01-M，MR !262；評估提案，尚非本批實作）。
