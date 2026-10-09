# 原文 03：Bool

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `JsonWriter& JsonWriter::Bool(bool v) {` 定位，完整CPP正文。
來源 commit `97361abfd5d1a08b51caf707f928b56825f3f056`；摘錄 SHA256 `2a3063194bf3f93a69b7ba281d5b378751f04f26dc57323a61c3363e5391cca9`。
原文、註解、縮排與空行完整保存；只做靜態核對，未執行writer或機台。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::Bool(bool v) {
    BeforeValue();
    buf_.append(v ? "true" : "false");
    return *this;
}


<!-- preserved-content:end -->
```
