# 原文 01：ElaHub-local IsValidUtf8

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/EventLogAnalysis/ElaHub.cpp`；以 `ElaHub-local IsValidUtf8` 定位。
固定來源 `b1bbe408a035595c81615cbc22543a326fba12b2`；SHA256 `ade3f1e3aae805dcb9b5e1396019b949a7ecf2e451311a00ea0ce8b393e65dfa`。
完整函式，保留相鄰原註解。未執行程式／測試。

```cpp
<!-- preserved-content:start -->
// ---------------------------------------------------------------------------
//  UTF-8 for the snapshot
// ---------------------------------------------------------------------------
static bool IsValidUtf8(const std::string& s)
{
    size_t i = 0;
    while (i < s.size())
    {
        const unsigned char c = (unsigned char)s[i];
        int n = 0;
        if (c < 0x80) n = 0;
        else if ((c & 0xE0) == 0xC0 && c >= 0xC2) n = 1;
        else if ((c & 0xF0) == 0xE0) n = 2;
        else if ((c & 0xF8) == 0xF0 && c <= 0xF4) n = 3;
        else return false;
        for (int k = 1; k <= n; ++k)
            if (i + k >= s.size() || ((unsigned char)s[i + k] & 0xC0) != 0x80) return false;
        i += n + 1;
    }
    return true;
}


<!-- preserved-content:end -->
```
