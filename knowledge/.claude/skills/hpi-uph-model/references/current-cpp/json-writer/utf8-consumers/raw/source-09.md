# 原文 09：WebSmartDiag-local U8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebSmartDiag.cpp`；以 `WebSmartDiag-local U8` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `c8685afa672ee99575ce90cd9679d298c5684564074335b03e4c9c581b46a81c`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// ---- text: literals in this tree are UTF-8, file contents are ANSI (cp950) ----
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
