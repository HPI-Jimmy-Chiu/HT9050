# 原文 19：IsValidWebSocketKey

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `IsValidWebSocketKey` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `8b555deebeaa4a537c1bbf8c9eddc834bf1cc5395a489d66c0d5d2e98b371a73`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// ---------------------------------------------------------------------------
//  Handshake validation
// ---------------------------------------------------------------------------
bool IsValidWebSocketKey(const std::string& key) {
    // RFC 6455 section 4.1: the key is "a base64-encoded (see Section 4 of
    // [RFC4648]) value that, when decoded, is 16 bytes in length". That means
    // exactly 24 characters, the last two being the '=' padding for a 16 % 3 == 1
    // remainder.
    //
    // The shape check is done here rather than delegated entirely to
    // Base64Decode because that decoder deliberately skips ASCII whitespace
    // (legal for line-wrapped MIME base64), so on its own it would accept a
    // wrapped or padded key of the wrong length.
    if (key.size() != 24) return false;
    if (key[22] != '=' || key[23] != '=') return false;
    for (std::size_t i = 0; i < 22; ++i) {
        char c = key[i];
        bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') || c == '+' || c == '/';
        if (!ok) return false;
    }
    // Then confirm it really decodes, and to exactly 16 bytes.
    std::string raw;
    if (!Base64Decode(key, &raw)) return false;
    return raw.size() == 16;
}


<!-- preserved-content:end -->
```
