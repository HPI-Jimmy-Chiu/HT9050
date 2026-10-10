# 宿主modal、answer與output-first

[回命令入口](../index.md)。V906 `tools/wb_serve.cpp`，同一hpi-uph-model樹；入口不另建技能鏡像。

| 要查的責任 | 入口 |
|---|---|
| 阻塞告警、notice、gate、ACK與query清除 | [告警回答](alarm-answer.md) |
| YES/NO、信箱退役與close side effects | [YES/NO](yes-no.md) |
| carry先取、餘項保存與巢狀modal | [carry順序](carry.md) |
| output分類、inert前綴與barrier | [output-first](output-first.md) |
| 安裝點、沿用依賴、Handler／版本／runtime界線 | [caller與版本](callers.md) |
| 固定來源、全部原文與metadata | [證據](evidence.md)／[manifest](source-manifest.json) |

11完整CPP／493函式原文行、21原文頁；9 context共179行credit0，含一處carry body重疊，不重複完成數。
MbWait／DialogMailboxRetire／W906_NoticeAckCommand沿用既有完整body，重核當前pin；舊頁鄰近comment未擴稱已更新。
原註解中的golden／端到端／RFC／裁決聲明照存，是歷史來源；本輪僅靜態查證與文件引用驗證。
尚未涵蓋完整main dispatch、browser實際消費、部署binary、硬體時序或UPH產能。
