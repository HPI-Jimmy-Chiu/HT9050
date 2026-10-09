# 原文 03：Cp950ToUtf8

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `Cp950ToUtf8`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `5b05d12f00c09a90ba693dd99661c2bd15cbc61000d414c035a27bcf04aa27c4`。
完整CPP函式，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
bool Cp950ToUtf8(const std::string& s, std::string* out) {
    const int wideLen = ::MultiByteToWideChar(
        950, MB_ERR_INVALID_CHARS, s.data(), static_cast<int>(s.size()), NULL, 0);
    if (wideLen <= 0) {
        return false;
    }
    std::wstring wide;
    wide.resize(static_cast<std::size_t>(wideLen));
    if (::MultiByteToWideChar(950, MB_ERR_INVALID_CHARS, s.data(),
                              static_cast<int>(s.size()), &wide[0], wideLen) != wideLen) {
        return false;
    }
    const int utf8Len = ::WideCharToMultiByte(
        CP_UTF8, 0, wide.data(), wideLen, NULL, 0, NULL, NULL);
    if (utf8Len <= 0) {
        return false;
    }
    std::string utf8;
    utf8.resize(static_cast<std::size_t>(utf8Len));
    if (::WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLen, &utf8[0], utf8Len,
                              NULL, NULL) != utf8Len) {
        return false;
    }
    out->swap(utf8);
    return true;
}

<!-- preserved-content:end -->
```
