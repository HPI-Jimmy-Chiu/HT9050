# 原文 02：ToUtf8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/EventLogAnalysis/ElaHub.cpp`；以 `ToUtf8` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `0a4dc275d8c4df310da85b62eff82b5bf96649de7c8b16e22bfb808998d9c7bf`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
std::string ToUtf8(const std::string& s)
{
    if (IsValidUtf8(s)) return s;
    const int wn = ::MultiByteToWideChar(950, 0, s.data(), (int)s.size(), NULL, 0);
    if (wn <= 0) return std::string();
    std::vector<wchar_t> w((size_t)wn);
    ::MultiByteToWideChar(950, 0, s.data(), (int)s.size(), &w[0], wn);
    const int un = ::WideCharToMultiByte(CP_UTF8, 0, &w[0], wn, NULL, 0, NULL, NULL);
    if (un <= 0) return std::string();
    std::string out((size_t)un, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, &w[0], wn, &out[0], un, NULL, NULL);
    return out;
}


<!-- preserved-content:end -->
```
