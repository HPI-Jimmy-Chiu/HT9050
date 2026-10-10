# 原文：WebBridgeServer::Impl::ClearQuery

[上層](../index.md)。固定pin `0e99afbd9adde80d06aaf6b9fd706799512811b2`，來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；SHA256 `f74213cf0c23b88b703d3e351c0b30d31db32f70f0bf11435d9e1871263a6314`。
完整函式含鄰接原comment／metadata，僅靜態保存，未執行程式或機台。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
//  AI(W906-Q30-REPLAY) 20260921: 宿主回答完（或放棄）之後清掉待答的 query。
//
//  ⚠ 一定要呼叫。不清的話，**下一個**連上來的瀏覽器會收到一個早就被回答過的
//    警報框，而且它送回來的 `modal.answer` 會被判成 `no query pending` ——
//    操作員會看到一個關不掉的框。
//
//  ⚠ 用 `qid` 比對而不是無條件清：若在極短時間內第一個被答完、第二個已經
//    `PostQuery` 上來，無條件清會把**新的**那一個抹掉。
//    （今天的同步模型不會發生，但這個保護是免費的，而且它讓這支函式可以
//     在不確定狀態下安全地被呼叫。）
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::ClearQuery(unsigned long long qid)
{
    if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, (double)qid, true, "clear", std::string());  WbGuard ol(outMx_);   //AI(W906-OPLOG) 20260929: the dialog was answered or given up (logged before taking outMx_)
    if (pendingQueryQid_ == qid) {
        pendingQueryQid_ = 0;
        pendingQueryFrame_.clear();
    }
}


<!-- preserved-content:end -->
```
