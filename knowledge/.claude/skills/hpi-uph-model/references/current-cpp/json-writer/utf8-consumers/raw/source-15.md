# 原文 15：WsDecoder constructor defaults

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsFrame.cpp`；以 `WsDecoder constructor defaults` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `77fbfb92aedd9f24d64726a2e264be544e92d6c5c6e39528697c7b616248ccd4`。
context，新增完成函式計數0。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
WsDecoder::WsDecoder(bool requireMaskedInput, std::size_t maxMessageBytes)
    : fragOpcode_(0),
      requireMask_(requireMaskedInput),
      validateUtf8_(true),
      failed_(false),
      sawClose_(false),
      closeCode_(kWsCloseNormal),
      maxMessage_(maxMessageBytes ? maxMessageBytes : kDefaultMaxMessageBytes) {}

<!-- preserved-content:end -->
```
