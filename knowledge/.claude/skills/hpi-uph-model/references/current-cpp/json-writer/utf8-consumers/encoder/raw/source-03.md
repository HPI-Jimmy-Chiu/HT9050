# 原文 03：EncodeMaskedFrame

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `EncodeMaskedFrame` 定位。
固定來源 `16f1ddb588497d250b9eaad227bc26dce9294ce4`；SHA256 `5eaff87787594a6a7b900714d678f6062b7e65af0158a6657f833abe4a5e921c`。
完整函式與相鄰原註解；未執行程式／測試。

```cpp
<!-- preserved-content:start -->
std::string EncodeMaskedFrame(int opcode, const std::string& payload,
                              uint32_t maskKey, bool fin) {
    return EncodeFrame(opcode, payload, fin, true, maskKey);
}


<!-- preserved-content:end -->
```
