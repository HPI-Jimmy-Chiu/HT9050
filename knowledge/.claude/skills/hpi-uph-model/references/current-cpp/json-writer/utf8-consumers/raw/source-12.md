# 原文 12：assembled continuation validation

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `assembled continuation validation`／TryOneFrame資料派送定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `1d00b013e279821f57c8d1d1aea08d667d939752d4795c3b16d6619a6a4e981c`。
context，新增完成函式計數0。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
    if (opcode == kWsContinuation) {
        frag_ += payload;
        if (!fin) return 1;                 // still mid-message
        WsMessage m;
        m.opcode = fragOpcode_;
        m.payload = frag_;
        frag_.clear();
        fragOpcode_ = 0;
        if (validateUtf8_ && m.opcode == kWsText && !IsValidUtf8(m.payload)) {
            Fail(kWsCloseInvalidPayload, "text message is not valid UTF-8");
            return -1;
        }
        out->push_back(m);
        return 1;
    }

<!-- preserved-content:end -->
```
