# 原文 20：LooksLikeWebSocketUpgrade

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `LooksLikeWebSocketUpgrade` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `ca00afc5240df3c6d5857cf76a8a2cb0c9b6c23c397cc1f44c98cdc16b7fd170`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
bool LooksLikeWebSocketUpgrade(const HttpRequest& req) {
    return ListContainsToken(req.Header("upgrade"), "websocket");
}


<!-- preserved-content:end -->
```
