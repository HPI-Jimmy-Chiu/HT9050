# 原文：WebBridgeServer::Impl::PostQueryOptions

[上層](../index.md)。固定pin `0e99afbd9adde80d06aaf6b9fd706799512811b2`，來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；SHA256 `dc89814bcf70060c21e60bf209f7657bd61e5878ceed3a1a57afcc85d0bf483e`。
完整函式含鄰接原comment／metadata，僅靜態保存，未執行程式或機台。

```cpp
<!-- preserved-content:start -->
// AI(W906-YESNO) 20260925: PostQuery 的兄弟 —— 選項名字由呼叫端直接給（見標頭）。
//   與 PostQuery 共用同一個廣播佇列、同一份補發槽（pendingQueryFrame_／pendingQueryQid_），
//   所以同一時間仍只有一個待答的 query —— 單執行緒的 tick 本來就不可能同時問兩題。
//   ⚠ `kcode` 固定寫 0：這一題沒有 K_* 遮罩。ht9045_dialog_host.js 只拿它印錯誤訊息，
//     答案對不對由 wb_serve 的等待迴圈照 options 判。
void WebBridgeServer::Impl::PostQueryOptions(unsigned long long qid, const std::string& kind,
                                             const std::string& text,
                                             const std::vector<std::string>& options,
                                             const std::string& at)
{
    std::ostringstream os;
    os << "{\"type\":\"query\",\"qid\":" << qid
       << ",\"code\":\"\",\"kcode\":0"
       << ",\"kind\":" << sib::QuoteString(kind)
       << ",\"text\":" << sib::QuoteString(text)
       << ",\"options\":[";
    for (std::size_t i = 0; i < options.size(); ++i)
        os << (i ? "," : "") << sib::QuoteString(options[i]);
    os << "],\"at\":" << sib::QuoteString(at.empty() ? IsoLocalNow() : at)
       << "}";
    {
        WbGuard ol(outMx_);
        Outgoing o;
        o.connId = 0;              // broadcast
        o.frame  = os.str();
        outQ_.push_back(o);
        pendingQueryFrame_ = o.frame;   // 補發給之後才連上的瀏覽器（同 PostQuery）
        pendingQueryQid_   = qid;
    }
    {
        WbGuard sl(statsMx);
        ++stats.queriesSent;
    }
    Wake();  if (g_W906OpLogHook) g_W906OpLogHook("POST", 0, (double)qid, true, "query", os.str());   //AI(W906-OPLOG) 20260929
}


<!-- preserved-content:end -->
```
