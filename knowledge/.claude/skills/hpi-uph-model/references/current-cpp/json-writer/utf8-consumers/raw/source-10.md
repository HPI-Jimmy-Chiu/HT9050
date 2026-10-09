# 原文 10：MbU8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；以 `MbU8` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `534a59be3bb4ce2e451d058f488f9963de3506117efca3b907de48e32f5f6c89`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// 這棵樹的字串字面值是 UTF-8，從檔案讀進來的是 ANSI（cp950）—— 兩種都可能進到 S1/S2。
std::string MbU8(const char* p)
{
    const std::string s(p ? p : "");
    if (webbridge::IsValidUtf8(s)) return s;
    const int wn = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return webbridge::SanitizeToUtf8(s);
    std::wstring w((size_t)wn, L'\0');
    ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int un = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string u((size_t)(un > 0 ? un : 0), '\0');
    if (un > 0) ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, &u[0], un, NULL, NULL);
    return u;
}


<!-- preserved-content:end -->
```
