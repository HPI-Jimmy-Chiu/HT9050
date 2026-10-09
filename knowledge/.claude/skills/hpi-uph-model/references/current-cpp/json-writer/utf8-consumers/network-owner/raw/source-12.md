# 原文 12：original HTTP/WS size constants only

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `original HTTP/WS size constants only` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `6625c765beeb87dc37b91b7043391c3232eb8b59749c49fe6e34b8c212bac09f`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
const size_t kMaxHttpHead    = 32u * 1024u;   // request head before we give up
const size_t kMaxWsMessage   = 64u * 1024u;   // reassembled text message cap

<!-- preserved-content:end -->
```
