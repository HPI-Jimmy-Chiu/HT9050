# ProcessHttpHead：一次HTTP請求與upgrade分流

[上層](index.md)；[WebBridgeServer::Impl::ProcessHttpHead原文](raw/source-04.md)；[ReceiveInto dispatch tail only; not full receive loop completion原文](raw/source-13.md)；[original HTTP/WS size constants only原文](raw/source-12.md)。

`WebBridgeServer.cpp`的`ProcessHttpHead(Conn& c)`先找`c.in`的CRLFCRLF；找不到回true等待。
找到後取出header並erase到分隔符尾，後方bytes仍在`c.in`。
`ReceiveInto`的已保存dispatch尾段會在非WS狀態先檢`c.in.size() > kMaxHttpHead`而回false，常數32 KiB。
這不是`ProcessHttpHead`本體的431回覆，也不是recv迴圈的記憶體硬上限；完整接收／drop流程另續。

request line用`istringstream >> method >> target >> version`；本體只判method／target是否空，空時enqueue 400並設`closeAfterFlush`。
不能把已讀version字串當作已驗HTTP version，也不能套用[獨立ParseHttpRequest](../handshake/parser.md)的cap／token／obs-fold規則。
header按第一個colon拆分，key走`Lower(Trim(...))`，value走`Trim`；同key用逗號加空白合併，沒有colon的行在此被略過。
這些callee以本檔function定位；本次五檔bytes與有限詞法查讀，未完成全樹parser／caller驗證。

path從target去除最先出現的`?`或`#`；wantsUpgrade取Upgrade／Connection兩值交`ContainsCI`。
同檔`ContainsCI`本體是`Lower(hay).find(needle)`；這是substring比對，與獨立helper的token-list契約不同。
要upgrade時，method非GET或path不等`cfg.wsPath`會加`wsRejected`、enqueue 404並設flush後關閉；通過才交`DoWebSocketUpgrade`。

其他請求加`httpRequests`；`routeFn`存在且`routePrefix`非空、path字面前綴相同時先呼叫dynamic route。
query自target的`?`之後取出，再去掉`#`之後；route回false就落到`files.Serve(method,target)`。
route回true時，`!dyn.headOnly`會無條件依body大小補Content-Length；HEAD保留handler所設值。
dynamic成功與static回覆均enqueue並設`closeAfterFlush`，此body採每連線一個HTTP request。
`Enqueue`／旗標／回true不等於已送到client或已關socket；Flush／CloseConn與server loop完整生命週期尚待保存。
