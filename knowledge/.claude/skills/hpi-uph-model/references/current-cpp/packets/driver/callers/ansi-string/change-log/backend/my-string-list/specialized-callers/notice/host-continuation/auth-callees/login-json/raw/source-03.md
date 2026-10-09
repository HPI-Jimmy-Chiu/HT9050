# 原文 03：RawValue

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；定位 `RawValue`，完整CPP正文。
來源 commit `367d9d85792fa756db6e898c950d6d79931cbf74`；摘錄 SHA256 `c707620534f39df51fce87be46fa5698cf98c594a5251ac8b54ba6cd285e6abf`。
歷史註解、客戶條件、裁決與常數保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
JsonWriter& JsonWriter::RawValue(const std::string& jsonText) {
    BeforeValue();
    buf_.append(jsonText);
    return *this;
}

<!-- preserved-content:end -->
```
