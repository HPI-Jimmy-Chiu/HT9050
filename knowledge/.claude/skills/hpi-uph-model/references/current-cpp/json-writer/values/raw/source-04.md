# 原文 04：Number(wb_int64)

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `JsonWriter& JsonWriter::Number(wb_int64 v) {` 定位，完整CPP正文。
來源 commit `97361abfd5d1a08b51caf707f928b56825f3f056`；摘錄 SHA256 `a51f0f97efd17f367e5c48795b91dd4b6365b86b8924404283dc7395d82b7bb0`。
原文、註解、縮排與空行完整保存；只做靜態核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::Number(wb_int64 v) {
    BeforeValue();
    buf_.append(JsonNumber(v));
    return *this;
}


<!-- preserved-content:end -->
```
