# 原文 08：WebRecipeChange-local U8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebRecipeChange.cpp`；以 `WebRecipeChange-local U8` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `732bf6bec19f0aeabf32b7bf202211cbc1283bfa2cefd8d016e5bef33929240d`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// ---- 檔名：磁碟上是 ANSI（cp950），JSON 是 UTF-8 ----
std::string U8(const AnsiString& a)
{
    const std::string s = a.c_str();
    if (webbridge::IsValidUtf8(s)) return s;
    const int wn = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return std::string();
    std::wstring w((size_t)wn, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string out((size_t)(n > 0 ? n : 0), '\0');
    if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, &out[0], n, NULL, NULL);
    return out;
}

<!-- preserved-content:end -->
```
