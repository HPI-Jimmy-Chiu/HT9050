# 原文 11：close reason validation

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `close reason validation` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `766ddf37d057e60866650e67875ab604562cd2c78e0a022057837451fb2c4e2c`。
context，新增完成函式計數0。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
                if (validateUtf8_ && !IsValidUtf8(m.closeReason)) {
                    Fail(kWsCloseInvalidPayload, "close reason is not valid UTF-8");
                    return -1;
                }

<!-- preserved-content:end -->
```
