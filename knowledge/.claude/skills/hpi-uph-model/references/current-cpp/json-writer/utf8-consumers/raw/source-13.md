# 原文 13：unfragmented text validation and delivery

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `unfragmented text validation and delivery` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `a4d07dec3d01147837b7d103f3dc7c51910396336b444b16af4c529221a330eb`。
context，新增完成函式計數0。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
    // Unfragmented single-frame message.
    WsMessage m;
    m.opcode = opcode;
    m.payload = payload;
    if (validateUtf8_ && opcode == kWsText && !IsValidUtf8(m.payload)) {
        // RFC 6455 section 8.1: a text message whose payload is not valid
        // UTF-8 is a 1007 failure, not something to hand upstairs.
        Fail(kWsCloseInvalidPayload, "text message is not valid UTF-8");
        return -1;
    }
    out->push_back(m);
    return 1;
<!-- preserved-content:end -->
```
