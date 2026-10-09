# 原文 25：BuildHandshakeErrorResponse

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `BuildHandshakeErrorResponse` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `996c09cf4934712c64e6db0fcfc3e1d6d9bcaa5331aac8158cdb6019bce60e8d`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
std::string BuildHandshakeErrorResponse(HandshakeResult why) {
    switch (why) {
        case kHsUpgradeRequired:
            return SimpleResponse("426 Upgrade Required", "Sec-WebSocket-Version: 13");
        case kHsMethodNotAllowed:
            return SimpleResponse("405 Method Not Allowed", "Allow: GET");
        case kHsOk:            // caller error -- never send a 200 for a handshake
        case kHsNotWebSocket:
        case kHsBadRequest:
        default:
            return SimpleResponse("400 Bad Request", 0);
    }
}


<!-- preserved-content:end -->
```
