---
name: ht9045-shuttle-flow
description: HT9045 IC Test Handler Shuttle（InShuttle/OutShuttle）流程知識庫。當使用者詢問 Shuttle 移動、Do_Auto_SHT1、Do_Auto_SHT2、Shuttle 左右移動、Shuttle sensor broken、Shuttle 浮料（Floating）、Shuttle 殘料（Null IC / Lose IC）、Shuttle 2D Barcode、Rotate Shuttle 檢查、Shuttle retry/skip/home、Fix3 氣缸保護、Shuttle 與 Index/InArm/OutArm 安全互鎖等相關問題時，應先載入此技能以理解 Shuttle 完整處理流程。關鍵字：Do_Auto_SHT1, Do_Auto_SHT2, AutoSHT1Task, AutoSHT2Task, CheckShuttleOutputHasICError, CheckShuttleSensorBroken_1, CheckShuttleSensorBroken_2, CheckNullICShuttle1_9045, CheckNullICShuttle2_9045, InSHT1, InSHT2, Shuttle。
---

# HT9045 Shuttle 舊名相容入口

此主題改由 [hpi-shuttle-flow](../hpi-shuttle-flow/SKILL.md) 管理。
先讀新入口，依 HT9045／HT9050／客戶條件分流；原觸發詞保留，舊 reference 路徑仍提供直達正文的指標。

原正文完整保存在 [歷史正文](../hpi-shuttle-flow/references/ht9045/legacy-body-20261006.md)，
來源版別、案例與相對連結保留／修正；不要把原 V897/V904 示例根路徑當成當前執行基準。
本批不刪舊入口；撤除時由整合者確認部署、引用與原擁有者通知。
