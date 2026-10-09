# 原文 13：ReceiveInto dispatch tail only; not full receive loop completion

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `ReceiveInto dispatch tail only; not full receive loop completion` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `d486efaf78ea792c23ad4a6d8f95deda3e28fcc57c46d01e31003804e2dadb9e`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
    if (!c.isWs) {
        if (c.in.size() > kMaxHttpHead) return false;    // head never terminated
        return ProcessHttpHead(c);
    }
    return ProcessWsBytes(c);
}

<!-- preserved-content:end -->
```
