# 原文 20／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Engine.cpp`；function／region `Rs232Engine::Start(`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `6c65aaceca2d606a89b5513449c025b672c0608dfcf420e507364240cc55b159`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
bool Rs232Engine::Start(testercomm::SyncMailbox* mailbox)
{
    if (mailbox == 0 || started_)
        return false;
    int expected = 0;
    if (!g_live.compare_exchange_strong(expected, 1))
        return false;   // golden: CreateMutex "MyRS232Standard" refused a second instance

    mailbox_ = mailbox;
    started_ = true;
    closed_ = false;
    depth_ = 0;

    // golden: every launch is a fresh process (Rs232Bridge.h "V906 program life")
    ResetRs232Globals();
    ++g_rs232Life;
    if (!g_setupIniOverride.empty())
        IniFileName = g_setupIniOverride.c_str();
    if (!g_hgenIniOverride.empty())
        asHGeneralPath = g_hgenIniOverride.c_str();

    // golden FindWindow("TfMain", ...) -> HMountWnd: the Handler side is "found" while the mailbox exists.
    HMountWnd = reinterpret_cast<HWND>(mailbox);
    g_handlerTok.store(static_cast<void*>(mailbox));

    try
    {
        // golden WinMain (RS232Standard.cpp): CreateForm(fRS232Main) [OnCreate], CreateForm(fMyPal), Run [OnShow].
        VclCreateForm(fRS232Main);
        fRS232Main->mailbox = mailbox;
        g_bridgeTok.store(static_cast<void*>(fRS232Main));
        fRS232Main->FormCreate(0);
        fRS232Main->FormShow(0);
    }
    catch (...)
    {
        Teardown();
        return false;
    }

    timer1_ = TimerSlot();
    up_.store(!fRS232Main->closeRequested);
    return true;
}

<!-- preserved-content:end -->
```
