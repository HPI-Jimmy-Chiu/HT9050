# connId、排隊與選讀存檔分支

來源：[CommandQueue.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebBridge/CommandQueue.h)的 `WebCommand`；[CommandQueue.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebBridge/CommandQueue.cpp)的 `tryPush`／`drain`；[WebBridgeServer.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp)的 `sib::QueuePush`／`Impl::CompleteCommand`／`Impl::PumpOutgoing`；[wb_serve.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp)的 `W906_TakeCarry`／`main`／editlist.save；[WebCmdGuard.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebCmdGuard.cpp)的Check／CheckKey／KeyOf／Exempt／W906CmdGuardScope::Begin，另見[WebCmdGuard.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/de7c2bf77127d19f23c6790fc9af1c42c911afe5/HT9011UC_Cpp_V3.33.906.0/WebCmdGuard.h)的歷史註解與介面。

## 身分與時間各自保留

QueuePush將server內部ticket放入WebCommand.id，另填c.connId=來源連線；browser原id留在PendingAck。connId、queue ticket與browser id是不同資料，不能互換。WebCommand constructor的connId初始0不代表這條接收路徑永遠是0；WebCmdGuard.h的舊註解稱QueuePush未填connId，與目前所讀實作不同，現況依QueuePush而非該歷史註解。

tryPush在容量容許時複製整個WebCommand並記pushedUs，否則返回false；HandleTextMessage對失敗回command queue full並移除pending ticket。drain先以鎖內swap取出queue，再依序append到out。這兩個選讀body沒有ControlOwner重比；不能把排入queue當成真正執行／落盤成功。

main先W906_TakeCarry再drain；TakeCarry複製g_carry命令進out後清carry。本次只核對這個搬移body，不宣稱所有輸出優先／modal／carry寫者的順序與清除政策已查完。

## 共用guard與owner不同

選讀分派前段先W906_Q44CmdRefused，再W906CmdGuardScope；busy則拒絕並跳過該命令。WebCmdGuard::Check／CheckKey處理重複指令時間窗；KeyOf使用cmd、tag與value型別／內容，所讀body不把connId加進key，也不直接查server owner。Exempt是防連點的分類，不等於接收owner豁免；相關helper／其他分派路徑仍未全量查證。

editlist.save選讀arm重查tag／FindPage、SystemStart／SoftStart、OpenGateRefused、value、reauth，然後FormLock及對應PageSave／其他Save caller，最後CompleteCommand。這個arm沒有直接 `server.ControlOwner()` 對 `wc.connId` 的比較；相對地，所讀act.observerSG分支對非Exempt命令有這項比較。這只能支持兩個選取分支的直接程式差異，不能證明所有callee、HTTP入口、owner併發或政策整體有／沒有保護，也不在本次改程式或宣稱實機缺陷成立。

## ack與執行效果

CompleteCommand以ticket找PendingAck，找不到就return；找到才取原connId／browserId，移除pending並將ack排入outQ_。PumpOutgoing只向仍存在、isWs且未closeAfterFlush、connId符合的連線送出。沒有收到ack不能單憑此判斷命令未執行；ack是否生成／能送達與[INI落盤](../customer-storage/ini.md)分開查。

後續仍需核對所有carry／modal與關閉分派、實際callee／reauth、權限寫者、owner競態及最後consumer／磁碟效果；原候選與未決保持。HT9050／其他機種的runtime、版本、權限與啟用方式另分流，不由這段共用Web結構推定現場狀態相同。
