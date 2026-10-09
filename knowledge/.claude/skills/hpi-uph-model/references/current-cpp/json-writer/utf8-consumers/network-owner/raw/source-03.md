# 原文 03：TryDecode

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `TryDecode` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `6a56c976aa2e01e986b8242477fc375be21acce095da990db6cb87b48ca5aff1`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// 1 = one message ready, 0 = need more bytes, -1 = protocol error.
//
// `in` is drained completely into the decoder; anything not yet a whole message
// stays inside the decoder, which is why Frame::consumed is always 0 and the
// caller must NOT erase from `in` itself.
static int TryDecode(Decoder& d, std::string& in, Frame& out)
{
    if (d.ready.empty()) {
        if (d.dec.Failed()) return -1;
        if (!in.empty()) {
            std::vector<WsMessage> got;
            const bool ok = d.dec.Feed(in, &got);
            in.clear();
            for (size_t i = 0; i < got.size(); ++i) d.ready.push_back(got[i]);
            if (!ok) {
                // Deliver whatever completed before the failure, then fail.
                if (d.ready.empty()) return -1;
            }
        }
        if (d.ready.empty()) return 0;
    }

    const WsMessage& m = d.ready.front();
    out.opcode  = m.opcode;
    // Data messages arrive fully reassembled, so a delivered message is always
    // a complete one; masking was enforced by the decoder before delivery.
    out.fin     = true;
    out.masked  = true;
    out.payload = m.payload;
    out.consumed = 0;
    d.ready.pop_front();
    return 1;
}


<!-- preserved-content:end -->
```
