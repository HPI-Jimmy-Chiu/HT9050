# 原文 07：JsonNumber(wb_int64)

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `JsonNumber(wb_int64)`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `6ed074bdad2677bf026bdc4be1af936004032c84bb291a72e6153e3673f8f332`。
完整CPP函式，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
std::string JsonNumber(wb_int64 v) {
    char tmp[32];
    // No %lld under MSVC's older CRTs and no <inttypes.h> guarantee under MinGW
    // 6.3 -- format by hand, which is also locale-proof.
    bool neg = false;
    unsigned long long mag;
    if (v < 0) {
        neg = true;
        // -(min) overflows; go through unsigned.
        mag = static_cast<unsigned long long>(-(v + 1)) + 1ull;
    } else {
        mag = static_cast<unsigned long long>(v);
    }
    int pos = 31;
    tmp[pos] = '\0';
    if (mag == 0) {
        tmp[--pos] = '0';
    }
    while (mag > 0) {
        tmp[--pos] = static_cast<char>('0' + static_cast<int>(mag % 10ull));
        mag /= 10ull;
    }
    if (neg) {
        tmp[--pos] = '-';
    }
    return std::string(&tmp[pos]);
}


<!-- preserved-content:end -->
```
