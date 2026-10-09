# 原文 05：U8Text

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；以 `U8Text` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `b5cc0b7dfa997e3afa8d62af1ba90b2a6318cf63eba4a358a5555af367e32fa5`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// AI(W906-D034) 20261002: an AnsiString that came from a file (cp950) or a UTF-8 literal -> UTF-8 for the page (as tools/wb_serve.cpp MbU8)
std::string U8Text(const AnsiString& a)
{
    const std::string s(a.c_str());
    if (webbridge::IsValidUtf8(s)) return s;
    const int wn = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return webbridge::SanitizeToUtf8(s);
    std::wstring w((std::size_t)wn, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int un = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string u((std::size_t)(un > 0 ? un : 0), '\0');
    if (un > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, &u[0], un, NULL, NULL);
    return u;
}


<!-- preserved-content:end -->
```
