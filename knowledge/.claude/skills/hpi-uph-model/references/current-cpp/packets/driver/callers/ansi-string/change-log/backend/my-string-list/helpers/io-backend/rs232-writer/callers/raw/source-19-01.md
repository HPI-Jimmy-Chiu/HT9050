# 原文 19／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Engine.cpp`；function／region `Rs232Engine::Stop()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `62a2d0138007c8cdd1f30b1a4daef5f7094a28d1786a0af14a2d270e8a8e297b`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void Rs232Engine::Stop()
{
    if (!started_)
        return;
    if (!closed_)
        DoClose();   // golden: Handler's CloseGpibProgram -> MSG_CMD_CloseGpib -> Close()
    Teardown();
}

<!-- preserved-content:end -->
```
