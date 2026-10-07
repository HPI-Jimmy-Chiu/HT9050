# SendToEngine 與同步 mailbox 局部流程

pin `37cef908a42adaaa8df8a0d3fc5959e52446e802`；[manifest](source-manifest.json)保存V906四source、七完整函式文字／body hash與七宣告片段。這是[Command傳送](../index.md)的下游局部，沒有驗證engine／測試機收到或完整UPH計數鏈。

- [本地返回、反向pump與逾時](flow.md)：`SendToEngine`、`SyncMailbox::Send／Process／Poll／Reset`。
- [版本、機型與未查界線](limits.md)：HT9050與其他Handler共用路由，V912出口另列。
- 固定source：[Hub](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37cef908a42adaaa8df8a0d3fc5959e52446e802/HT9011UC_Cpp_V3.33.906.0/TesterComm/TesterCommHub.cpp)、[Hub宣告](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37cef908a42adaaa8df8a0d3fc5959e52446e802/HT9011UC_Cpp_V3.33.906.0/TesterComm/TesterCommHub.h)、[Mailbox](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37cef908a42adaaa8df8a0d3fc5959e52446e802/HT9011UC_Cpp_V3.33.906.0/TesterComm/SyncMailbox.cpp)、[Mailbox宣告](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37cef908a42adaaa8df8a0d3fc5959e52446e802/HT9011UC_Cpp_V3.33.906.0/TesterComm/SyncMailbox.h)。

活正文用function與欄位定位；原碼golden行號／當時的設計註解只作歷史，未重驗golden或真機。
