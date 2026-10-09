# 原文 23：BuildHandshakeResponse

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `BuildHandshakeResponse` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `0db8acf3fe7f7a29465aeabdeab75162f821cac6be22e7a71207310867179d7e`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
std::string BuildHandshakeResponse(const std::string& secWebSocketKey,
                                   const std::string& subprotocol) {
    std::string r;
    r += "HTTP/1.1 101 Switching Protocols\r\n";
    r += "Upgrade: websocket\r\n";
    r += "Connection: Upgrade\r\n";
    r += "Sec-WebSocket-Accept: ";
    r += ComputeAcceptKey(secWebSocketKey);
    r += "\r\n";
    if (!subprotocol.empty()) {
        r += "Sec-WebSocket-Protocol: ";
        r += subprotocol;
        r += "\r\n";
    }
    // Deliberately NO Sec-WebSocket-Extensions: nothing is negotiated, which
    // is what entitles WsFrame's decoder to reject non-zero RSV bits.
    r += "\r\n";
    return r;
}


<!-- preserved-content:end -->
```
