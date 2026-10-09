# 原文 04：IsValidUtf8

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `IsValidUtf8`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `e3cb6f0829861768a85a6e85e0e365175852e43fb3ee3f881c10b72b39105e65`。
完整CPP函式，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
bool IsValidUtf8(const std::string& s) {
    std::size_t i = 0;
    while (i < s.size()) {
        const std::size_t len = Utf8SequenceLen(s, i);
        if (len == 0) {
            return false;
        }
        i += len;
    }
    return true;
}


<!-- preserved-content:end -->
```
