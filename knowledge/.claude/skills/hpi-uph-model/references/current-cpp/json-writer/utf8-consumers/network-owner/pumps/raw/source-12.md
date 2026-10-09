# 原文：original TagJson and JSON quote forwarding rationale and adapters; context only（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `888556ff727904de2d687af657191d1d5c661a56c9510c734768104268280bb4`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// --- JsonWriter -------------------------------------------------------------
// AI(W906-WebBridge-Tcp) 20260812: the TagValue -> JsonWriter mapping that used
// to live here as sib::WriteValue now lives in WebBridge/TagJson.cpp, which is
// also where the reverse direction lives. The Null vs "" distinction it
// preserves is unchanged and still load-bearing: the browser renders null as
// "---" and "" as blank, and collapsing them misreports an uninstalled device
// as a real zero.

// {"tag":value,"tag2":value2}
//
// AI(W906-WebBridge-Tcp) 20260812: delegated to WebBridge/TagJson.h. This used
// to be the only tag encoder in the tree; the TCP sidecar link now needs the
// identical bytes (and the decode direction, which has no counterpart here), so
// the definition moved to TagJson and this became a forwarder. Keeping a second
// hand-maintained copy is how the two wires would silently drift apart.
static std::string ObjectFrom(const std::map<std::string, TagValue>& m)
{
    return EncodeTagObject(m);
}

// A JSON string literal, quotes included.
static std::string QuoteString(const std::string& s)
{
    return JsonQuote(s);
}


<!-- preserved-content:end -->
```
