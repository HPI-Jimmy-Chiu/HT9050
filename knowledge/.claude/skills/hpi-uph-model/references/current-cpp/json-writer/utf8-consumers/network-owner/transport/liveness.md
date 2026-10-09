# 控制權idle與WS idle／Ping

[上層](index.md)；[PumpLiveness](raw/source-08.md)；[NowMs context](raw/source-12.md)；[欄位與原comment](raw/source-13.md)。

NowMs選std::chrono::steady_clock、轉milliseconds的time_since_epoch().count()為unsigned long long。
這是此來源的時鐘選擇，不是獨立OS精度／實機時間量測；不當成IsoLocalNow、wall-clock班別或UPH時間來源。

| 狀態 | PumpLiveness判斷 |
|---|---|
| ctrlOwner_非0且controlIdleTimeoutMs>0 | now-ctrlLastCmdMs_ **>** timeout才store(0) |
| Conn非WS | 跳過WS idle／Ping |
| idleTimeoutMs>0 | now-lastRecvMs **>** timeout，CloseConn後continue |
| pingIntervalMs>0 | now-lastPingMs **>=** interval，enqueue Ping、更新lastPingMs／awaitingPong=true、pingsSent++ |

ctrlLastCmdMs_在哪個accepted command分支更新仍由HandleTextMessage／命令owner續查，不把原comment當完整權限驗證。
ReceiveInto成功recv即更新lastRecvMs；idle判斷只看這個時間，**沒有以awaitingPong為timeout predicate**。
awaitingPong在送Ping時設true，既有[ProcessWsBytes](../messages.md)收到Pong時清false；本body沒有依該bool阻止下一次Ping。
所以原「dead peer, or one that stopped ponging」註解是設計說明，實際判斷是WS接收idle；持續別種資料的情境尚未runtime驗證。
計數pingsSent記在Enqueue後，Flush可能保留／失敗；不能當peer已收到的證據。
timeout皆>0才啟用，相減與實際配置／thread排程需分開核；本次不修改runtime或機台參數。
