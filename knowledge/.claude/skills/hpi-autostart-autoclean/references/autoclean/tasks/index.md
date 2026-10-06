# 清潔Task樹

先讀 [完整原狀態機](../references/state-machines.md)，依下列function／游標選分支：

| 階層 | Function／關鍵Task |
|---|---|
| 協調 | DoAutoCleanKit／iDoAutoCleanTask |
| 取Clean Pad | DoAutoCleanPickfromCleanKit／iAutoCleanPickFromCleanKitStageTask |
| 放回Kit | DoAutoCleanPlaceToCleanKit／iAutoCleanPlaceToCleanKitTask |
| Shuttle定位與互鎖 | DoShuttle1AutoClean、DoShuttle2AutoClean／iDoShuttle1AutoCleanTask、iDoShuttle2AutoCleanTask |
| Index接觸與吸放 | DoIndexAutoClean／iDoIndexAutoCleanTask、bInedxCleanFinish、iContactCount |

Task編號是該版本的控制游標，沒有對應原始碼就不能直接把同號套到另一機型。CleanAir不取放Kit，HP2／Fix3／CleanKit依配置選取；HT9050正常流程與乾跑的tick ownership另查 [機型](../../machines/index.md)。
