# 原文 23／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Engine.cpp`；function／region `Rs232Engine::DoClose()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `6e7351dff0874b888ec1e5f03e2253121b9a2c6fc9a816fff90b2d1f143029a9`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void Rs232Engine::DoClose()
{
    if (closed_ || fRS232Main == 0)
        return;
    closed_ = true;
    up_.store(false);
    fRS232Main->FormClose(0);   // golden Close() -> OnClose
    testercomm::UiChannel::Instance().Publish(kUiKey, BuildUiSnapshot(false));
    testercomm::UiChannel::Instance().Publish("rs232.home", BuildHomeFragment(false));   // AI(W906-TC-SHARED) 20261001 (St02-E)
}

<!-- preserved-content:end -->
```
