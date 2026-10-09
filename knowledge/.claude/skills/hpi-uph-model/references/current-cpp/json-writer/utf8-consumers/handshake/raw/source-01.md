# 原文 01：LowerCh

[證據](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/WsHandshake.cpp`；以 `LowerCh` 定位。
pin `7bf6ff28e23bbfeb6f94c1751d63ae62533313f4`；SHA256 `901fcfa979a34f0a9b2703d4b3c1dd708e1cf5d7d71d481ed8bc277f6ca6cb8c`。
原函式／相鄰註解或header context保存，未執行程式。

```cpp
<!-- preserved-content:start -->
// ---------------------------------------------------------------------------
//  small ASCII helpers (locale-independent on purpose -- ::tolower is
//  locale-sensitive and this is wire protocol, not user text)
// ---------------------------------------------------------------------------
static char LowerCh(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}


<!-- preserved-content:end -->
```
