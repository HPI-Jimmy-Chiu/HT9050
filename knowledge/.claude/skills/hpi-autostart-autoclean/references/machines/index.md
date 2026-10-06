# HT9050與其他機台的共同／差異

| 面向 | 共通 | HT9050／配置差異 |
|---|---|---|
| 啟動 | 依Start入口與守衛判斷 | OLP／HTSET／Web與build profile分開，不由顯示名稱決定 |
| 清潔 | Trigger→Task→清潔與OneCycle協調 | CleanKit／HP2／Fix3／CleanAir、機構／教點／客戶配置不同 |
| 正常流程 | 讀bRunAutoClean及各Task，並追csystem正常階梯 | [目前乾跑](#目前main的乾跑)若接管tick，正常階梯本拍不執行 |
| 版本 | 0618／0625／V899／V912原文標原日期 | E-030保留V912守衛；[0210review](mach0210.md)是當次機台版本，不當目前main結論 |

HT9050所有共同資訊與機型差異都留在此Skill；不另建立只含9050的AutoStart／AutoClean副本。馬達／機構配置要另讀 [HT9050硬體](../../../ht9050-hw/SKILL.md)、[Index](../../../hpi-index-flow/SKILL.md)、[Shuttle](../../../hpi-shuttle-flow/SKILL.md)。

## 目前main的乾跑

基準main `f57d93f15` 的HT9011UC_Cpp_V3.33.906.0/MachineType.h仍定義W906_HT9050_DRYRUN；Ht9050DryRun.cpp::W906_Ht9050DryRunOn只在此key且非SOFT_SIMULTE時判W906_Ht9050OrgHome(MTrayX)!=-2，SIM直接false。不要由MACHINE.id或一個宏推所有build都乾跑。

csystem.cpp::DoAllProcess在On為true時，呼叫W906_Ht9050DryRunTick，傳SoftStop／SystemStart／fAllMotorHome作paused及iHandlerStartCount==0作firstRunTick，隨後return，因此跳過本拍的正常階梯與AutoClean。Tick在firstRunTick且未paused時重置命令／位置到位cache及速度狀態；paused時目前有逐軸Stop與Z1Stop。這些是現在函式內容，不能拿0210當次缺口列表直接說本版本仍沒有這些步驟。

目前旗標與來源碼查證不等於現場軟體／參數／教點已同步、也不代表乾跑所有安全問題已結案。此批不開／關key、不更改runtime、不動機台；[舊review](mach0210.md)保留版本來源供對照。
