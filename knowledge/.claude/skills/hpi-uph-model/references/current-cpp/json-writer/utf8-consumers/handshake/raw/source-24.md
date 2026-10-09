# 原文 24：SimpleResponse

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `SimpleResponse` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `669d2b5b10ac5e5d723306f382d702712d59508f96aafcb33d0a54bc51f12336`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
static std::string SimpleResponse(const char* statusLine, const char* extraHeader) {
    std::string r;
    r += "HTTP/1.1 ";
    r += statusLine;
    r += "\r\n";
    if (extraHeader && *extraHeader) {
        r += extraHeader;
        r += "\r\n";
    }
    r += "Content-Length: 0\r\n";
    r += "Connection: close\r\n";
    r += "\r\n";
    return r;
}


<!-- preserved-content:end -->
```
