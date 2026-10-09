# 原文 09／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `BookOverride`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `1418261f2ff9329b3f46e12a577eb7811fce2f3c213407a635e7b8fa1e2d6e9a`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
AnsiString BookOverride()
{
    const char* e = std::getenv("W906_PWBOOK_PATH");
    return (e && *e) ? AnsiString(e) : AnsiString("");
}

<!-- preserved-content:end -->
```
