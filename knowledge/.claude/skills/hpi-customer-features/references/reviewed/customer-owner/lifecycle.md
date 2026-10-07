# 接收owner與控制權變更

來源：[WebBridgeServer.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp)的 `Impl::HandleTextMessage`／`Impl::CloseConn`／`Impl::PumpLiveness`／`Impl::CloseAllSockets`／`WebBridgeServer::ControlOwner`、`ctrlOwner_`／`ctrlLastCmdMs_`；設定介面見[WebBridgeServer.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.h)的controlIdleTimeoutMs。

## 接收時的直接守衛

HandleTextMessage檢查JSON、cmd／tag／value型別等基本格式，接著查readOnly與queue；通過後才處理control.*。acquire在owner為0或已是本連線時取得／保留；takeover的分支允許直接改為c.id。兩者更新ctrlLastCmdMs_並直接SendAck，不經一般命令queue。release只接受目前owner為c.id，否則回not-operator。

一般命令另有名稱／prefix例外表；editlist.save不在該owner例外中，`ctrlOwner_.load()!=c.id`拒絕，通過則更新ctrlLastCmdMs_，再建ticket／pending與QueuePush。接收owner、橋接readOnly、PageSave的AccessLevel與reauth是不同層，不能互相取代。這次未查完整HTTP／upgrade／auth.*策略或OpLogHook實作。

## 接收之後可能改變

| 選讀路徑 | 直接作用 | 不能據此推論 |
|---|---|---|
| control.takeover | ctrlOwner_改為發命令的c.id | 已排隊的舊owner命令必然被取消 |
| control.release | owner相同時設0 | queue自動撤銷或callee完全禁止 |
| CloseConn | 關閉的是目前owner時設0，再關socket／刪Conn | 完整queue／pending清理已查完 |
| PumpLiveness | 啟用controlIdleTimeoutMs且超過ctrlLastCmdMs_門檻時設0 | control idle與連線idle是相同計時器 |

PumpLiveness另以idleTimeoutMs／lastRecvMs判斷連線關閉，pingIntervalMs／lastPingMs處理WS ping；不要把這些條件當成control-owner續期的同一證據。沒有讀取或改動實際runtime設定，不推定現場timeout值。

CloseAllSockets直接關所有socket、clear conns_／liveWs_及listener；所選body沒有逐條走CloseConn，也沒有直接設ctrlOwner_。因此不能把bulk close說成等同CloseConn的owner清除；完整Stop／Start生命週期與後續效果未在本次追完。

ControlOwner getter只讀atomic owner值。接收通過與主迴圈真正執行之間仍須看[queue與分派](queue-dispatch.md)，不能由單次讀值宣稱整個操作期間owner未改。
