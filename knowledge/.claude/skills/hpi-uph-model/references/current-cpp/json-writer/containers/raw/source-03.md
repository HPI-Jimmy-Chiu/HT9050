# 原文 03：EndObject

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `EndObject` 定位，完整CPP正文。
來源 commit `94ff7c1981d1ac98f01376a574098c39e1c25275`；摘錄 SHA256 `2668965e769764f3f60a5209078c62a76c1bdc0cfc2cd7560925be5e823ce9f1`。
原文與縮排完整保存；只有靜態來源核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::EndObject() {
    if (stack_.empty() || stack_.back() != kCtxObject || keyPending_) {
        ok_ = false;
        return *this;
    }
    stack_.pop_back();
    buf_.push_back('}');
    needComma_ = true;
    return *this;
}

<!-- preserved-content:end -->
```
