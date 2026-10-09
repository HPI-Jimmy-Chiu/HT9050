# Notice capture／ack：計時、退役與回應

[上層警報 caller](../index.md)。承接 [警報寫入時機](../handler.md)，
本輪保存 6 完整 CPP、4 header inline 與 6 選定區段，來源 `c90d8d22bb2d34c532443386469de717aef2e671`。
444 行 AckLikeGolden 分頁後可逐字重組；沒有把父函式的區段算成完整函式。

| 問題 | 入口 |
|---|---|
| snapshot、Recovery、PassTime 及覆蓋 | [Capture 與時間](capture.md) |
| 拒絕、pause 值及 close 副作用 | [Ack 的條件](ack.md) |
| mailbox、auth、panel、held 與 retired | [Host 與回應](host.md) |
| 機型／客戶／版本、普查與仍待查項 | [證據界線](evidence.md) |
| 原文、來源 hash、種類及 lossless 分頁 | [來源清單](source-manifest.json) |
| 2051 份來源的 10 符號字面命中 | [普查資料](symbol-census.json) |

本段局部 reference 完成；canonical Skill 與完整 notice／UPH 部署鏈仍未完成。
以下都是來源碼靜態讀取，沒有執行告警、硬體、資料庫、build 或機台測試。

## Host continuation（20261009）

[退役writer、完整MbWait與AuthVerify](host-continuation/index.md)：補齊雙檔分開替換、序號先增、held解構搬回及處理成功／accepted／pass的界線；完整部署與其他wait／timer仍待續。
