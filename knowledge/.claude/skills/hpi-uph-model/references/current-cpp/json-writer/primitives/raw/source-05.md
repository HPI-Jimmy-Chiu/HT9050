# 原文 05：SanitizeToUtf8

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `SanitizeToUtf8`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `2144c28417897707039a01a91f6b9e432d421f20ddbbc0eb941ddfe24b039747`。
完整CPP函式，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
std::string SanitizeToUtf8(const std::string& raw) {
    if (raw.empty()) {
        return raw;
    }
    // (a) already valid UTF-8 -- pass through untouched.
    if (IsValidUtf8(raw)) {
        return raw;
    }
#if defined(_WIN32)
    // (b) assume legacy Big5 / CP950 (what the V899 tree and machine config
    //     files actually contain) and transcode.
    std::string converted;
    if (Cp950ToUtf8(raw, &converted) && IsValidUtf8(converted)) {
        return converted;
    }
#endif
    // (c) last resort: byte-wise U+FFFD replacement. Never emits invalid UTF-8.
    return RepairUtf8(raw);
}


<!-- preserved-content:end -->
```
