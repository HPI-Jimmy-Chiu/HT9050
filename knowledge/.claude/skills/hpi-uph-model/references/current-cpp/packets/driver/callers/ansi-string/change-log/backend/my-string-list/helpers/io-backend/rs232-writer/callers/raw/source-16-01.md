# 原文 16／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Engine.cpp`；function／region `Rs232Engine::Teardown()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `fc97490d4bcff75745d643add25233735edda1d97cf0bd53f24815c5175fd2e2`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void Rs232Engine::Teardown()
{
    up_.store(false);
    if (fRS232Main)
    {
        try
        {
            fRS232Main->FormDestroy(0);   // golden OnDestroy after OnClose (MainForm.dfm)
        }
        catch (...)
        {
        }
    }
    g_bridgeTok.store(0);
    delete fRS232Main;   // stops the three TComm reader threads and uServer first (Rs232Ui.cpp dtor)
    fRS232Main = 0;
    testercomm::UiChannel::Instance().Publish(kUiKey, "{\"up\":false}");
    g_handlerTok.store(0);
    HMountWnd = 0;
    GHandler2Gpib = 0;
    mailbox_ = 0;
    started_ = false;
    g_live.store(0);
}

<!-- preserved-content:end -->
```
