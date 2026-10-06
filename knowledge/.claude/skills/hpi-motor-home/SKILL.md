---
name: hpi-motor-home
description: "Handler 馬達回原點共用與機型差異：HT9045、HT9050、MotorHome／MotorInitial／Home、HomeFlag／THomeFlag、HomeClass編排、單軸／全機HOME、SMC卡片式與PCI-1203 DS402驅動器式、步進SW3D／伺服SGDX、回原點完成與速度上限、MotorAccessStepperLeaveOrigin、RouteHomeStart／Done、Teach／LightScale及V906狀態。追HOME卡住、抖動、失敗與機型／驅動器分流時使用。"
---

# Handler 馬達 Home

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

HT9050與其他機型在同一主題內比較共用介面與差異；先確認版本、軸、控制卡與驅動器，再讀需要的分支。

## 必要限制

- 先讀 [共同與差異](references/common.md)，分清reset、底層home、單軸狀態機與全機編排。
- HomeFlag、MotorHome回傳值、THomeFlag及fAllMotorHome是不同層的狀態，不能互相代替。
- 卡片式和DS402驅動器式的結束證據、位置歸零與速度來源不同；不可由API同名推定相同。
- 未知驅動器、原點極性、home offset、步進離原點、命令／encoder與安全門條件先查證。
- 舊機台側臨時bypass和目前GitLab預設編譯旗標分開記錄；此Skill未授權HOME／JOG／清錯或改設定。

## 按問題選路

| 問題 | Reference |
|---|---|
| 共用介面、旗標、機型／驅動器對照 | [共同與差異](references/common.md) |
| HT9045／BCB／單軸與HomeClass | [HT9045與歷史流程](references/machines/ht9045.md) |
| HT9050／1203全機與單軸分流 | [HT9050流程](references/machines/ht9050.md) |
| SMC／卡片式與DS402／驅動器式 | [控制卡與驅動器樹](references/drivers/index.md) |
| V906 Teach／Home旗標與網頁生命週期 | [V906狀態原文](references/flow/generic/references/v906-port-status.md) |
| 客戶與配置條件 | [條件索引](references/customers.md) |

## 查證與交付

沿dispatcher→MotorHome→驅動器route追下命令、狀態樣本與完成條件；核對編排index與motor ID、序列防撞及失敗回報。保存版本、Task、軸／卡／drive身分與log證據；每批更新Change Log。舊名稱與原文保留供版本對照。
