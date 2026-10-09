# 原文 01：AcceptKey

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `AcceptKey` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `31b1d8e3c4f7c3fb2992b5d574b657f642f81c7ec7ebf09b1226ca4ca163347b`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// --- WsHandshake ------------------------------------------------------------
static std::string AcceptKey(const std::string& clientKey)
{
    return ComputeAcceptKey(clientKey);
}


<!-- preserved-content:end -->
```
