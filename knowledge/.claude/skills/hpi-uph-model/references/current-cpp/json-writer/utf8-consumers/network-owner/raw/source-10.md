# 原文 10：Decoder original per-connection adapter context and PendingBytes

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `Decoder original per-connection adapter context and PendingBytes` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `d1965ec020c9b372b2468dd17b153de1d77f28a91360eb7e7f47d779430aa6ac`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// One per connection. WsDecoder is stateful -- it buffers partial frames and
// reassembles fragmented messages across TCP chunk boundaries -- so it cannot
// be a free function over the socket buffer the way this adapter first assumed.
struct Decoder {
    WsDecoder                dec;
    std::deque<WsMessage>    ready;

    // Server role: inbound frames MUST be masked, and the decoder enforces it.
    // The cap matches this file's own kMaxWsMessage so oversize is rejected by
    // the decoder (close 1009) rather than after reassembly.
    Decoder() : dec(/*requireMaskedInput=*/true, 64u * 1024u) {}
};

static size_t PendingBytes(const Decoder& d)
{
    return d.dec.PendingBytes() + d.dec.FragmentBytes();
}


<!-- preserved-content:end -->
```
