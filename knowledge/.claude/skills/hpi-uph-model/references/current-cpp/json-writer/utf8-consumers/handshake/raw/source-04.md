# 原文 04：IsTChar

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `IsTChar` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `84b762cc261e47190c4f83ce5fd4fd380061ff6be6ae1819823c637607b34022`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// RFC 7230 section 3.2.6:
//   tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
//           "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
// Everything else -- SP, HTAB, the separators, and every control or 8-bit byte
// -- is excluded. Note `c` is compared as unsigned so a high-bit byte cannot
// slip through on a signed-char platform.
static bool IsTChar(char c)
{
    const unsigned char u = static_cast<unsigned char>(c);
    if (u >= 'a' && u <= 'z') return true;
    if (u >= 'A' && u <= 'Z') return true;
    if (u >= '0' && u <= '9') return true;
    switch (u) {
        case '!': case '#': case '$': case '%': case '&': case '\'':
        case '*': case '+': case '-': case '.': case '^': case '_':
        case '`': case '|': case '~':
            return true;
        default:
            return false;
    }
}


<!-- preserved-content:end -->
```
