# 原文 05：IsToken

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `IsToken` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `7026de6afc73663c49e91f319e054ecca267fdcfa7e30e68c085484de43877fc`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// token = 1*tchar  (so the empty string is NOT a token)
static bool IsToken(const std::string& s)
{
    if (s.empty()) return false;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (!IsTChar(s[i])) return false;
    }
    return true;
}


<!-- preserved-content:end -->
```
