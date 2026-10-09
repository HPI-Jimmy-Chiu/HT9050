# 原文 09：Frame original per-connection adapter context

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `Frame original per-connection adapter context` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `4827ad88a88685360f95bede8a6e2524fe19b79ec3944ee3f984fb239868c7d3`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
struct Frame {
    int         opcode;
    bool        fin;
    bool        masked;
    std::string payload;
    size_t      consumed;   // always 0: the decoder owns consumption now
    Frame() : opcode(0), fin(false), masked(false), consumed(0) {}
};

<!-- preserved-content:end -->
```
