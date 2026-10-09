# 原文 17：HttpRequest::HasQueryParam

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `HttpRequest::HasQueryParam` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `9bf91ccf2492744117d9c2e264489ef1343d6095486f96b2dd29454e8877edf6`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
bool HttpRequest::HasQueryParam(const std::string& name) const {
    return FindQueryRaw(query, name, 0);
}


<!-- preserved-content:end -->
```
