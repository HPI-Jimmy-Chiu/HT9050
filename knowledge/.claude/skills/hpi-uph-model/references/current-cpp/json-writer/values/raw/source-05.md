# 原文 05：Number(double)

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `JsonWriter& JsonWriter::Number(double v) {` 定位，完整CPP正文。
來源 commit `97361abfd5d1a08b51caf707f928b56825f3f056`；摘錄 SHA256 `0e24340a32bff9b89c78860c858c4a155c09b6e90782ebb07aa03b2582a44d11`。
原文、註解、縮排與空行完整保存；只做靜態核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::Number(double v) {
    BeforeValue();
    buf_.append(JsonNumber(v));
    return *this;
}


<!-- preserved-content:end -->
```
