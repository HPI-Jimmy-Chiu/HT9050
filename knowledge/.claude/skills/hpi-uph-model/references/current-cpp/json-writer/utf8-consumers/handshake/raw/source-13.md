# 原文 13：HttpRequest::HasHeader

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `HttpRequest::HasHeader` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `4de4a6e50d69f1e2d6eb578987655905498c395400040072d992d900cdcd993e`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
bool HttpRequest::HasHeader(const std::string& name) const {
    std::string want = ToLowerAscii(name);
    for (std::size_t i = 0; i < headers.size(); ++i) {
        if (headers[i].first == want) return true;
    }
    return false;
}


<!-- preserved-content:end -->
```
