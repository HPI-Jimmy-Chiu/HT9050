# 原文 12：HttpRequest::Clear

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `HttpRequest::Clear` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `96ff6d2bd9aa7957f3913111ecc7f4e87c3db9f0cf4f757347a2f13666e13e75`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// ---------------------------------------------------------------------------
//  HttpRequest members
// ---------------------------------------------------------------------------
void HttpRequest::Clear() {
    method.clear();
    target.clear();
    version.clear();
    rawPath.clear();
    path.clear();
    query.clear();
    headers.clear();
}


<!-- preserved-content:end -->
```
