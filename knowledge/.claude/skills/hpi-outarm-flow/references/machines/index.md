# OutArm機型分流

| 機型／配置 | 差異與查證入口 |
|---|---|
| HT9050 | 四嘴合吸一顆、教導A、間距0，In／Out接線名字順序有差異；dispatcher走通用單Picker。硬體有MOutArmY伺服與C_OutArmSmallY氣缸，不把舊概念圖的Y氣缸當現況 |
| HT9050飛梭 | 入、出獨立且沒有第二組入料飛梭；M18是出料飛梭Y。OutY取料位置與Shuttle互鎖去相鄰主題查原裁決／目前實作，不套HT9045配對馬達 |
| HT9050盤型 | Frank 20261006第18條：沒有Fix選項、Auto1→2→3由左到右且X左正右負。原Fix／Magazine／Fix3滿盤案例完整保留但不可直接當9050功能；不改機台工單與Teach |
| HT9045多吸嘴 | iInArmType分派、取料1／2與Suck映射按8／16／32-site配置；Out基準E／D／C及獨立USE_OUT_ARM_Y_PITCH查原文，不能從InArm公式反推 |
| HT9045S／LS等 | 特殊Pitch、單顆與盤型路線依原配置；整理提案提醒LS部分取料判斷用入料位置，本批未逐支重驗該特殊路徑，不能提升為common |

## 安全與來源

- [原硬體AR-1／2／5](../../../ht9050-hw/references/ht9050-vs-ht9045.md)：硬體差異；表內舊移植狀態不是今日main結論。
- [Frank最新盤型／教導裁決](../../../../../HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261006.md)：第18條；分類工單要機台端調整，文件整理不套值。
- [Shuttle與Index／InArm](../related.md)／[目前caller與SmallY旗標](../runtime/current-main.md)。
- [原基準軸與全部配置流程](../flow/original-entry.md)。
