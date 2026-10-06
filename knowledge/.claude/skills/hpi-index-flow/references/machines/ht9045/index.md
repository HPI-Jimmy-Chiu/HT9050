# HT9045 與相關歷史 Index 流程

## 差異

原版涵蓋有 Index Y、Z1／Z2、前後臂及 32 Site 的配置；每個分支仍須核對機型與版本。與 HT9050 的共同項／差異見 [共用表](../../common.md)。

## 入口與流程

- [原版入口與主 Task／DoTestY 分支](../../history/ht9045-index-flow/index.md)
- [Auto Height 與 Contact Test](autoheight-contact-test-ht9045.md)
- [主狀態機詳細流程](../../flow/DoTestHeadMotor_ProcessFlow.md)
- [子狀態機](../../flow/child-state-machines.md)
- [Z1Up／Z2Down 時序](../../flow/Z1UpZ2Down_MotionSequence.md)

## 安全

- [跨臂 Z 安全互鎖與卡死案例](case-crossarm-z-safe-interlock-deadlock.md)
- [Contact/Pick 互鎖原案例](../../history/ht9045-contact-pick-interlock/index.md) 是 V899 紀錄，不是 V906 實作聲明。
- [EP 版本差異](../../ep/index.md)

## 查證來源

本分支保留原文件記載的版本、行號與日期。修正預設交付 V912；分析 V899 機台仍讀 V899，不得修改唯讀版本。
