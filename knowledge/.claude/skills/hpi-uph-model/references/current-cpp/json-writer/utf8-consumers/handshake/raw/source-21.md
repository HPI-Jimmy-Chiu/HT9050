# 原文 21：CheckWebSocketUpgrade

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `CheckWebSocketUpgrade` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `fc098ddf242d77ce56fddbdece34b4032e37ee4fe47a5e7027d6586af75d4fae`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
HandshakeResult CheckWebSocketUpgrade(const HttpRequest& req, std::string* whyNot) {
    if (whyNot) whyNot->clear();

    if (!LooksLikeWebSocketUpgrade(req)) {
        if (whyNot) *whyNot = "no Upgrade: websocket header";
        return kHsNotWebSocket;
    }
    // From here on the client is definitely attempting a WebSocket handshake,
    // so every remaining failure is an error response, not a static file.
    if (req.method != "GET") {
        if (whyNot) *whyNot = "method is not GET";
        return kHsMethodNotAllowed;
    }
    if (req.version != "HTTP/1.1") {
        if (whyNot) *whyNot = "HTTP version is not 1.1";
        return kHsBadRequest;
    }
    if (!ListContainsToken(req.Header("connection"), "upgrade")) {
        if (whyNot) *whyNot = "Connection header does not contain the upgrade token";
        return kHsBadRequest;
    }
    if (!req.HasHeader("sec-websocket-version")) {
        if (whyNot) *whyNot = "missing Sec-WebSocket-Version";
        return kHsBadRequest;
    }
    if (TrimOws(req.Header("sec-websocket-version")) != "13") {
        // RFC 6455 section 4.4: the right answer is 426 plus the version we do
        // speak, so the client can retry instead of guessing.
        if (whyNot) *whyNot = "Sec-WebSocket-Version is not 13";
        return kHsUpgradeRequired;
    }
    std::string key = TrimOws(req.Header("sec-websocket-key"));
    if (key.empty()) {
        if (whyNot) *whyNot = "missing Sec-WebSocket-Key";
        return kHsBadRequest;
    }
    if (!IsValidWebSocketKey(key)) {
        if (whyNot) *whyNot = "Sec-WebSocket-Key is not base64 of 16 bytes";
        return kHsBadRequest;
    }
    return kHsOk;
}


<!-- preserved-content:end -->
```
