# 原文：SetNonBlocking

[上層](../index.md)。來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin `bebe1bf379f961899a85cc8376ebf1aacc9cf2df`；SHA256 `2738357ab747a5679dbf5d0b8298a5eb7dfa1121d02d8fc9f6e7fbedcb84d10d`。
完整函式計數1；原comment／metadata與正文保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
void SetNonBlocking(SOCKET s)
{
    u_long nb = 1;
    ioctlsocket(s, FIONBIO, &nb);
}


<!-- preserved-content:end -->
```
