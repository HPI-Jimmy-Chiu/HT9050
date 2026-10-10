# 原文：IsSaneName and IsoLocalNow complete helper context; not new completion credit（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`34c8e2e164014a93699f0b0a712366f5da01ebd2`；本頁payload SHA256 `0c297a4d8431e0f6ea0f2ae5169637312a5a2d0c5cc48c0fe468ec118ea56244`。
完整函式完成數見manifest；context／helper／adapter與拆頁不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// A command / tag name we are willing to hand to the UI thread. Deliberately
// narrow: these strings come off a socket and end up selecting machine actions.
bool IsSaneName(const std::string& s, size_t maxLen)
{
    if (s.size() > maxLen) return false;
    for (size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') ||
                        c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

std::string IsoLocalNow()
{
    std::time_t t = std::time(0);
    std::tm tmv;
#if defined(_MSC_VER)
    localtime_s(&tmv, &t);
#else
    std::tm* p = std::localtime(&t);
    if (p) tmv = *p; else std::memset(&tmv, 0, sizeof(tmv));
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d",
                  tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
                  tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
    return std::string(buf);
}


<!-- preserved-content:end -->
```
