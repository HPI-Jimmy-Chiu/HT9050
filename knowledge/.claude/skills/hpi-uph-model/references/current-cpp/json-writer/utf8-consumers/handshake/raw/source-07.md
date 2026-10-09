# 原文 07：EqualsIgnoreCase

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `EqualsIgnoreCase` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `d0a034b3eb4577c25330ad837071af5e3cdbec623108f6e4da487e54c7dad389`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
static bool EqualsIgnoreCase(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (LowerCh(a[i]) != LowerCh(b[i])) return false;
    }
    return true;
}


<!-- preserved-content:end -->
```
