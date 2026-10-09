# 原文 01：Key

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `Key` 定位，完整CPP正文。
來源 commit `94ff7c1981d1ac98f01376a574098c39e1c25275`；摘錄 SHA256 `2db940bd4765c4ea0bcec88d2178d06e3c1d8ac6e0a28ec4ee58104c25a17aff`。
原文與縮排完整保存；只有靜態來源核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::Key(const std::string& name) {
    if (stack_.empty() || stack_.back() != kCtxObject || keyPending_) {
        ok_ = false;
    }
    if (needComma_) {
        buf_.push_back(',');
        needComma_ = false;
    }
    buf_.append(JsonQuote(name));
    buf_.push_back(':');
    keyPending_ = true;
    return *this;
}

<!-- preserved-content:end -->
```
