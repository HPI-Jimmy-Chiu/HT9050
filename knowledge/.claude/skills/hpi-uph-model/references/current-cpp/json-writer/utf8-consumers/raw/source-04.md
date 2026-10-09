# 原文 04：WebBuilder-local U8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBuilder.cpp`；以 `WebBuilder-local U8` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `7bc8698b4fc98fa8287e3a468276c51e978be4011c925c7c0ed76d82db545c6c`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// ---- text: file-system names are ANSI (cp950); JSON is UTF-8 ----
std::string U8(const AnsiString& a)
{
    const std::string s = a.c_str();
    if (webbridge::IsValidUtf8(s)) return s;
    const int wn = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return webbridge::SanitizeToUtf8(s);
    std::wstring w((size_t)wn, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int un = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string u((size_t)(un > 0 ? un : 0), '\0');
    if (un > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, &u[0], un, NULL, NULL);
    return u;
}

<!-- preserved-content:end -->
```
