# 原文 06：TrimOws

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `TrimOws` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `4845fc9ff19bd79b1bb97a019a5a420dfe9236168a3776f6ea38617866b5b77a`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
static std::string TrimOws(const std::string& s) {
    std::size_t b = 0, e = s.size();
    while (b < e && IsOws(s[b])) ++b;
    while (e > b && IsOws(s[e - 1])) --e;
    return s.substr(b, e - b);
}


<!-- preserved-content:end -->
```
