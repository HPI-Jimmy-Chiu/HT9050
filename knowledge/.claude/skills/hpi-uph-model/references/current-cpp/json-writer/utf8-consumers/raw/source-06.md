# 原文 06：WebLotInfoFtp-local U8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLotInfoFtp_St02.cpp`；以 `WebLotInfoFtp-local U8` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `987b5de29f49c0838595d0d44763d6a35e7c9fd664b0fa77d88b5cc790d94529`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// ---- names on disk are ANSI (cp950), JSON is UTF-8 (same as WebRecipeChange.cpp U8 / FromU8) ----
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
