# 原文：DialogMailboxRetireMessage（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 12行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
static void DialogMailboxRetireMessage()
{
    if (g_dialogMailboxDir.empty()) return;
    const std::string json = w906dlg::MessageIdleJson(++g_dialogSeq);
    if (json.empty()) {
        std::printf("  ⚠ Message 信箱退役：種子裡找不到 \"seq\":0 —— 不寫出去，寧可不退役也不要寫壞檔\n");
        return;
    }
    if (!w906dlg::MailboxPut(g_dialogMailboxDir, "Message-dialog-request", json))
        std::printf("  ⚠ Message 信箱退役失敗 -- 下次重整會再彈一次\n");
}


<!-- preserved-content:end -->
```
