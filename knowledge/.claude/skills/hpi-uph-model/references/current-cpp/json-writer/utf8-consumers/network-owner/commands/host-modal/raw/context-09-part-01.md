# 原文：finite main hook installation context only; not complete main function（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 5行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
    g_pumpQueue = &cmdQueue;
    W906_ShowErrorMessage_Hook = &ForwardShowErrorMessage;  { extern void (*W906_ShowMotorErrorMessage_Hook)(const char*, int, int, const char*, const char*); extern void ForwardShowMotorErrorMessage(const char*, int, int, const char*, const char*); W906_ShowMotorErrorMessage_Hook = &ForwardShowMotorErrorMessage; }   //AI(W906-JAM-STOP) 20260930: golden ShowMotorErrorMessage's fNote->ShowModal() (note.cpp:1133) -- host at EOF; same line, no line moves
    // AI(W906-YESNO) 20260925: 從這裡起，每個 ShowMyMessageBox_YES_NO 都送到網頁、等操作員按是／否
    //   （使用者 20260925 裁決第 10 條）。在這一行之前（開機序列）呼叫的仍回 0，見 ForwardShowMyMessageBoxYesNo。
    W906_ShowMyMessageBoxYesNo_Hook = &ForwardShowMyMessageBoxYesNo;  { extern bool (*W906_HomeBlockedHook)(const char*, std::string&); extern bool W906_MotorAccessHomeBlocked(const char*, std::string&); W906_HomeBlockedHook = &W906_MotorAccessHomeBlocked; }   //AI(W906-HOMEBLOCK) 20261003: NB2-1 (URGENT U14 item 3) -- from here on TfMain::Home refuses while START is refused (Arm Cell job / hand teach), for every HOME source (forms/fMain.cpp Home's first statement; rules WebMotorAccess.cpp EOF). Same line, no line moves

<!-- preserved-content:end -->
```
