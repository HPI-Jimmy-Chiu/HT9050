# 原文 04：BeginArray

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `BeginArray` 定位，完整CPP正文。
來源 commit `94ff7c1981d1ac98f01376a574098c39e1c25275`；摘錄 SHA256 `2aa68db79779430e525dbc3ab0055d739732e683d4e8ca47919144dcd9648379`。
原文與縮排完整保存；只有靜態來源核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::BeginArray() {
    BeforeValue();
    buf_.push_back('[');
    stack_.push_back(static_cast<char>(kCtxArray));
    needComma_ = false;
    return *this;
}

<!-- preserved-content:end -->
```
