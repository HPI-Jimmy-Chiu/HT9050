# 原文 02：ToLowerAscii

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `ToLowerAscii` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `8f3ef2a58a8e8200281c999204a127417f268e8252716e9945ac95f62fc9837e`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
std::string ToLowerAscii(const std::string& s) {
    std::string r(s);
    for (std::size_t i = 0; i < r.size(); ++i) r[i] = LowerCh(r[i]);
    return r;
}


<!-- preserved-content:end -->
```
