# 原文：WsaErrText

[上層](../index.md)。來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `bebe1bf379f961899a85cc8376ebf1aacc9cf2df`；SHA256 `fb54135875fa30f3bf9a16cba19f0557b367fe4daec0a3dd8a4aec82bf21cfc5`。
完整函式計數1；原comment／metadata與正文保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
std::string WsaErrText(const char* what, int code)
{
    std::ostringstream os;
    os << what << " failed, WSAGetLastError=" << code;
    return os.str();
}


<!-- preserved-content:end -->
```
