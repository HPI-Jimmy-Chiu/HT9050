# 原文 07：Clear

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `Clear` 定位，完整CPP正文。
來源 commit `94ff7c1981d1ac98f01376a574098c39e1c25275`；摘錄 SHA256 `52b1a8998dabf9f70d0b4b26c30cbbff9cd187bb2d7527c12da0b91647f91322`。
原文與縮排完整保存；只有靜態來源核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
void JsonWriter::Clear() {
    buf_.clear();
    stack_.clear();
    needComma_  = false;
    keyPending_ = false;
    ok_         = true;
}

<!-- preserved-content:end -->
```
