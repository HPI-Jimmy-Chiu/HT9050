# Yield／Low Yield 告警

| 要查的問題 | 文件與函式 |
|---|---|
| count、limit、window 與 WAR07xx 分群 | [保存原入口](original-entry.md)／[九核心函式原文](references/YieldMonitoring_Functions_Explanation.md) |
| 目前呼叫者、OneCycle／Retry／Smart Auto Clean | [目前main](current-main.md)的 W906_Timer3Timer、DoLowYieldAlarm、PC_CHECKSMARTAUTOCLEAN |
| RT 切換後低良率 site 關閉 | 同上 DoRTAutoSocketOff／DoAutoCloseSite，區分 0618、0625／V912 與 E-034 |
| 為何突然不報／延後報 | ClearYieldCount、計數窗口與 Host／SECS/GEM 來源；不能只看 Alarm Reset |
| 權限、解除、畫面與告警停止 | [共同規則](../common.md)／[解除](../dismissal/index.md)／[目前告警宿主](../runtime/current-main.md) |

原函式說明與行號帶原日期及版本，完整保存為歷史。需要現在的結論先讀目前main，再以指定版本／檔名、function／特定變數查證；不修改良率門檻或操作實機。
