---
name: hpi-motor-control
description: "Handler 馬達控制共用介面與機型差異，整合HT9045／HT9050、HTMotor／TMyMotor／TTrayMotor、MotorMove／JOG／servo、命令與encoder、Galil／SMC／MN200／MotionNet／EtherCAT PCI1203、Panasonic／Yaskawa驅動器資料及靜態Motor Layout／Teach統計。分析卡別開啟健康、類別狀態閘、安全與軟極限、Index單軸或3/4軸、扭力與空間干涉時使用。"
---

# Handler Motor Control

通用分層評估：[標頭瘦身與 Motor／IO 隔離](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ee4ed4df7a506badb6cb0ce72769eaeb8b2aebb5/.claude/skills/cpp_build/references/header-slimming-and-motor-io-isolation-20261006.md)（ST01-M，MR !262；評估提案，尚非本批實作）。

推送基準與後續裁決：[最新main整合](references/main-integration-20261006.md)。

同主題整合HT9050與其他機型；由馬達類別共用介面、控制卡／driver與機構差異選需要的reference。

## 必要限制

- 先讀 [共同與差異](references/common.md)。Steven Q119：新增動作閘看馬達類別的狀態，不以HT9050／1203名稱開關。
- 先從卡別設定、Mot_Table CardModel與類別辨識技術，再追開卡／健康；資料路徑分派與動作允許閘分開處理。
- Steven Q130：1軸Index走TMyMotor::MotorMove；3/4軸Index走Galil，與是否1203無關。裁決與目前類別建構缺口分開標記。
- 命令位置、encoder、servo／alarm、到位、樣本可用性與軟極限需按類別及版本確認；不由原例子的數值推定成功。
- Layout／Teach統計是靜態查找資料，不是實機安全教導值。原文與driver手冊保留版本，readonly定義檔不修改。

## 按問題選路

| 問題 | Reference |
|---|---|
| 共用介面與機型／卡別差異 | [共同與差異](references/common.md) |
| 開卡順序／失敗Alarm／執行期健康 | [開卡與健康](references/control/references/card-init-and-health.md) |
| HTMotor／TMyMotor／TTrayMotor結構 | [類別](references/control/references/motor-classes.md) |
| Galil／MotionNet／SMC／EtherCAT API | [控制卡樹](references/cards/index.md) |
| Panasonic／Yaskawa協定、物件／警報 | [Driver樹](references/drivers/index.md) |
| HT9045／HT9050 Index與當前案情 | [機型樹](references/machines/index.md) |
| 空間干涉、靜態Spec與歷史Teach群組 | [Layout／統計樹](references/layout/index.md) |
| Home／DS402回原點 | [Motor Home整合入口](../ht9045-motor-home/SKILL.md) |

## 查證與交付

核對目前source的類別建構、dispatcher與gate，再用對應reference追命令→資料回讀→到位／錯誤。保留Steven原裁決與歷史API例子，但不把未接入／舊架構當成當前實作。每批附Change Log與來源版本；此Skill未授權動機台或改執行期設定。
