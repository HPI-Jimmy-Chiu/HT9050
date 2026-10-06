# 溫度機型與通道差異

## 差異

HT9050與HT9045使用同一溫度Skill；分流依實際裝配、控制器系列、USE_16_HEATER與通道映射，不能只靠型號推定硬體可用。

## 入口與流程

- [畫面／enum／實體位址與HT9050歷史對照](../core/references/main-screen-display.md)。
- [HT9050硬體DTM表](../../../ht9050-hw/references/temp-dtm-map.md)：仍留在原硬體知識位置，本Skill導向；3站24CH、SLK推導及主機／IP疑點保留。
- [原71通道表](../controllers/configuration/references/golden-v912-channels.md)／[DTM系列](../core/references/controllers/delta-dtm.md)。

## 安全與查證

配線表、golden映射、V906設定與當前機台快照分開；沒有同步的本機參數不能用來回報機台問題。DTM第二主機與感測器型別限制須由硬體／底層確認；本批純文件未套參數。
