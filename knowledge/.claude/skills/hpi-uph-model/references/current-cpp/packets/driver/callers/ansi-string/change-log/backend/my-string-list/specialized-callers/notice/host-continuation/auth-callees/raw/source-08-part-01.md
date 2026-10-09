# 原文 08／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `BookPath`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `0e0961d7752abeec0ce1af4a7af3e0723322757e397f21bea1d1e8ce6d0132c6`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
AnsiString BookPath()                                                           // golden pwPath；測試縫同 wb_serve auth.login
{
    const char* e = std::getenv("W906_PWBOOK_PATH");
    return (e && *e) ? AnsiString(e) : pwPath;
}

<!-- preserved-content:end -->
```
