# 原文 06：String

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `JsonWriter& JsonWriter::String(const std::string& v) {` 定位，完整CPP正文。
來源 commit `97361abfd5d1a08b51caf707f928b56825f3f056`；摘錄 SHA256 `7d95fe28ee3624700b65df1811539645e88e38c0b4d3f395a48c8550a02fdb04`。
原文、註解、縮排與空行完整保存；只做靜態核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::String(const std::string& v) {
    BeforeValue();
    buf_.append(JsonQuote(v));
    return *this;
}


<!-- preserved-content:end -->
```
