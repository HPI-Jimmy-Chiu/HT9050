# 原文 02：EncodeServerFrame

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`；以 `EncodeServerFrame` 定位。
pin `5eaf921a326691d45eccfcd5f00b6fdda26a9340`；分頁payload SHA256 `a8f4864c42aeb6d09e48eac33958a20d7664388325a3b125897afcc7dfaf1b54`。
完整函式／相鄰註解或有限context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// Server-to-client frames are never masked (RFC 6455 section 5.1).
static std::string EncodeServerFrame(int opcode, const std::string& payload)
{
    return EncodeFrame(opcode, payload, /*fin=*/true, /*mask=*/false, /*maskKey=*/0);
}


<!-- preserved-content:end -->
```
