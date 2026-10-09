# 原文 06：ForceJsonDecimalPoint

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebBridge/JsonWriter.cpp`；以 `ForceJsonDecimalPoint`／原header policy定位。
來源 commit `8edc9bdb86d2ece2a83244d9b71f9dfe5837ce86`；摘錄 SHA256 `7de2910b8d8d73e9621ac88cf533d186b66bd1d763826c821f085cb036839ba8`。
完整CPP函式，全文、註解、縮排與原空行保存；未執行函式或歷史測試。

```cpp
<!-- preserved-content:start -->
void ForceJsonDecimalPoint(std::string* text) {
    const struct lconv* lc = ::localeconv();
    if (lc == 0 || lc->decimal_point == 0) {
        return;
    }
    const char sep = lc->decimal_point[0];
    if (sep == '.' || sep == '\0') {
        return;
    }
    for (std::size_t i = 0; i < text->size(); ++i) {
        if ((*text)[i] == sep) {
            (*text)[i] = '.';
        }
    }
}


<!-- preserved-content:end -->
```
