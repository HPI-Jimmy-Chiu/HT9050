# 原文：Outgoing and PendingAck structs（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`d95fc5fc504d4e2b93a68f754874a72cf1166471`；本頁payload SHA256 `975af746fb645d71a4fb6f19e32d8fe12fd49163f9ab5a65d32457745e2a00b4`。
完整函式的完成數記於manifest；拆頁與context均不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
    struct Outgoing {
        unsigned long long connId;   // 0 = broadcast to every WS connection
        std::string        frame;
    };

    struct PendingAck {
        unsigned long long connId;
        double             browserId;
    };

<!-- preserved-content:end -->
```
