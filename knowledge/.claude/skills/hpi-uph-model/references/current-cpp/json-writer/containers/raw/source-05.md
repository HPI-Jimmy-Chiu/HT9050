# 原文 05：EndArray

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `EndArray` 定位，完整CPP正文。
來源 commit `94ff7c1981d1ac98f01376a574098c39e1c25275`；摘錄 SHA256 `f2ac9f9c7b7f2f136f5e46de5f377a8f33c6dc85ed9e7dde3a04dcbe843f2e67`。
原文與縮排完整保存；只有靜態來源核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::EndArray() {
    if (stack_.empty() || stack_.back() != kCtxArray || keyPending_) {
        ok_ = false;
        return *this;
    }
    stack_.pop_back();
    buf_.push_back(']');
    needComma_ = true;
    return *this;
}

<!-- preserved-content:end -->
```
