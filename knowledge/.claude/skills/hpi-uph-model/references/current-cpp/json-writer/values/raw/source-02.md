# 原文 02：Null

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `JsonWriter& JsonWriter::Null() {` 定位，完整CPP正文。
來源 commit `97361abfd5d1a08b51caf707f928b56825f3f056`；摘錄 SHA256 `e19330d2e87f3a32fa5b76b03476b16a66c331f8f62d6c6f42d0a9d6872f87c6`。
原文、註解、縮排與空行完整保存；只做靜態核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::Null() {
    BeforeValue();
    buf_.append("null");
    return *this;
}


<!-- preserved-content:end -->
```
