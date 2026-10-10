# 原文：DialogMailboxPostYesNo（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 12行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
static bool DialogMailboxPostYesNo(const std::string& requestId,
                                   const char* s1, const char* s2, const char* s3)
{
    if (g_dialogMailboxDir.empty()) return false;
    const unsigned long long seq = ++g_dialogSeq;
    const std::string json = w906dlg::YesNoRequestJson(seq, requestId,
        s1 ? s1 : "", s2 ? s2 : "", s3 ? s3 : "",
        SystemInitialOK == true,                 // runtime.systemInitialOK（樣本欄位，照填）
        iUnLoaderCount == 0);                    // requestedSideEffects.pauseHandler：golden FormShow :302 `if(!iUnLoaderCount)` 那一臂
    return w906dlg::MailboxPut(g_dialogMailboxDir, "Message-dialog-request", json);
}


<!-- preserved-content:end -->
```
