# 原文 02：BeginObject

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `BeginObject` 定位，完整CPP正文。
來源 commit `94ff7c1981d1ac98f01376a574098c39e1c25275`；摘錄 SHA256 `3dbd0f6c6b1b3f096fd879d11ebc608d31ea1a8a14b55dd068de47cfd338036f`。
原文與縮排完整保存；只有靜態來源核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::BeginObject() {
    BeforeValue();
    buf_.push_back('{');
    stack_.push_back(static_cast<char>(kCtxObject));
    needComma_ = false;
    return *this;
}

<!-- preserved-content:end -->
```
