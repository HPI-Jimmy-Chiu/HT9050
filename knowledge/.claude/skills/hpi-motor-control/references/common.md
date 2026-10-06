# Motor Control 共同項與差異

基準main `eb7f47e7f`，20261006。先分清共用介面、裝置分派、機型差異與尚未實作的裁決。

## 共同設計與直接查證

- Steven 1005 Q119原裁決與Q130全文保存於 [原入口章節](control/original-entry.md)，是新的閘／分派設計依據；不宣稱E043等工作已全部完成。
- `HT9011UC_Cpp_V3.33.906.0/Motor/mymotor.cpp` 的 `ReadPos`／`ReadEncoderPos` 在Motor非NULL且Enable時讀底層，否則走fallback；數字可讀不等於硬體動過。
- 馬達共用`TMyMotor`封裝底層`HTMotor`，由各卡／driver實作；原文的成員與API例子按其版本保留，不作通用即時真值。
- 原入口的Home例子寫100完成，但目前V906 MotorHome成功回1；請讀 [Home整合路由](../../ht9045-motor-home/SKILL.md) 核對各層旗標。原錯例作歷史原稿保存，沒有提升為活規則。

| 項目 | 共用項／要求 | HT9045與其他機型 | HT9050差異／查證邊界 |
|---|---|---|---|
| 動作閘 | 依類別提供的狀態／讀值可用性判斷 | MotionNet／軸卡／EtherCAT都適用設計要求 | 不以Type_HT9050、W906_GpibModel或needs1203作閘開關；與資料路由不同 |
| Index接口 | 先辨識1軸或3/4軸的機構 | 3/4軸Index依Q130用Galil | 1軸Index依Q130用MotorMove；類別建構修正與待辦由活躍案情ref查證 |
| 位置與健康 | 區分cmd／encoder、Alarm／servo／inpos與可用性 | 各卡健康與恢復方式不同 | 1203 route／DS402只是其中一條；原S-26陷阱不可當成所有卡都有 |
| Driver／協定 | API表與單位需對照硬體 | Panasonic A4/A5 RS232；A6BN原文註明尚無機台在用 | Yaskawa Σ-X原資料為HT9050在用；量測與物件單位仍依driver資料 |
| Layout／Teach | 靜態座標與群組統計供查詢，不是自動可套的教導 | 原群組按model／HPP／GearRatio區分 | 未核對HT9050幾何，不把9045雙Index布局當9050單Z真值 |

[控制卡](cards/index.md)／[Driver](drivers/index.md)／[機型與案情](machines/index.md)／[Layout](layout/index.md)。

通用分層評估：[標頭瘦身與 Motor／IO 隔離](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ee4ed4df7a506badb6cb0ce72769eaeb8b2aebb5/.claude/skills/cpp_build/references/header-slimming-and-motor-io-isolation-20261006.md)（ST01-M，MR !262；評估提案，尚非本批實作）。
