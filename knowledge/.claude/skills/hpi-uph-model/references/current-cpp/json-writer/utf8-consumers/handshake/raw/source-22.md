# 原文 22：ComputeAcceptKey

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `ComputeAcceptKey` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `21c8333d4ae0d77f9380eed0f72119f572f81e03508a536016bcb719a7715baa`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// ---------------------------------------------------------------------------
//  Response construction
// ---------------------------------------------------------------------------
std::string ComputeAcceptKey(const std::string& secWebSocketKey) {
    // RFC 6455 section 4.2.2 step 5: base64(SHA1(key + GUID)). The key is
    // concatenated exactly as received (trimmed by the caller), NOT decoded.
    std::string toHash = secWebSocketKey;
    toHash += kWebSocketGuid;
    std::string digest = WB_SHA1_RAW(toHash);      // 20 raw bytes
    return WB_BASE64_ENCODE(digest);
}


<!-- preserved-content:end -->
```
