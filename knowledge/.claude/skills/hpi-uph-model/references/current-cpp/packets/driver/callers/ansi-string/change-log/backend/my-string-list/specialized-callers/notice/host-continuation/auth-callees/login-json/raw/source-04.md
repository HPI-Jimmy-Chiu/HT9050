# 原文 04：BeforeValue

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；定位 `BeforeValue`，完整CPP正文。
來源 commit `367d9d85792fa756db6e898c950d6d79931cbf74`；摘錄 SHA256 `f1562ad7165e86c12c9f18dde724344c5b8a767acb20f70680a71aa6ac87b7ac`。
歷史註解、客戶條件、裁決與常數保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
void JsonWriter::BeforeValue() {
    if (!stack_.empty() && stack_.back() == kCtxObject && !keyPending_) {
        ok_ = false;              // object members need a Key() first
    }
    if (needComma_) {
        buf_.push_back(',');
    }
    keyPending_ = false;
    needComma_  = true;
}

<!-- preserved-content:end -->
```
