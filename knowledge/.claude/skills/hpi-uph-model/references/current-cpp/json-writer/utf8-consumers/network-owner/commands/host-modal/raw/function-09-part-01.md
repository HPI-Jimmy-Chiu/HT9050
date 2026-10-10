# 原文：W906_IsInertCmd（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 5行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
static bool W906_IsInertCmd(const std::string& c)
{
    return c == "sys.ping" || c == "log.event" || c == "ui.windows.put" || c == "cfg.resync";
}


<!-- preserved-content:end -->
```
