# 原文 02：EncodeFrame

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `EncodeFrame` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `62ec03a8865af49f4ef20450d1e6702645fd0c67305284c527cc0a66306674dc`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
std::string EncodeFrame(int opcode, const std::string& payload,
                        bool fin, bool mask, uint32_t maskKey) {
    std::string f;
    f.reserve(payload.size() + 14);

    // FIN + RSV1..3 (always zero: no extension is negotiated in the
    // handshake, so sending a non-zero RSV bit would be a protocol error) +
    // 4-bit opcode.
    unsigned char b0 = static_cast<unsigned char>((fin ? 0x80 : 0x00) |
                                                  (opcode & 0x0F));
    f.push_back(static_cast<char>(b0));

    AppendLength(f, payload.size(), mask ? 0x80 : 0x00);

    if (mask) {
        // Client -> server direction only. The key is written big-endian and
        // the payload is XORed with key[i % 4] (RFC 6455 section 5.3).
        unsigned char k[4];
        k[0] = static_cast<unsigned char>((maskKey >> 24) & 0xFF);
        k[1] = static_cast<unsigned char>((maskKey >> 16) & 0xFF);
        k[2] = static_cast<unsigned char>((maskKey >> 8) & 0xFF);
        k[3] = static_cast<unsigned char>(maskKey & 0xFF);
        f.append(reinterpret_cast<const char*>(k), 4);
        for (std::size_t i = 0; i < payload.size(); ++i) {
            f.push_back(static_cast<char>(
                static_cast<unsigned char>(payload[i]) ^ k[i & 3]));
        }
    } else {
        // Server -> client: NEVER masked (RFC 6455 section 5.1). A masked
        // frame from the server is a protocol error the browser will drop the
        // connection over, so the default of mask=false is the safe one.
        f.append(payload);
    }
    return f;
}


<!-- preserved-content:end -->
```
