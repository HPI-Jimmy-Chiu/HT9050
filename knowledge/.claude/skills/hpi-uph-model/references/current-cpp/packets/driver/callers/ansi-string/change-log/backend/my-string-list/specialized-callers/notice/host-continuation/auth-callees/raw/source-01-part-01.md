# 原文 01／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `WebLogin_StateJson`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `1bf0ee5f4e7bba4b0d61b30e2fa7c314ecdc1a3f0393c196f1cda9d445e66cc8`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
std::string WebLogin_StateJson()
{
    std::string j = "{\"mode\":\"";
    j += WebLogin_UsesBook() ? "book" : "select";
    j += "\",\"level\":" + std::to_string(AccessLevel);
    j += ",\"itemIndex\":" + std::to_string(s_itemIndex);
    j += ",\"levelName\":\"" + std::string(fMain->cbUserSelect->Text.c_str()) + "\"";
    j += ",\"userCaption\":\"" + std::string(s_userCaption.c_str()) + "\"";   // golden spbUserName->Caption
    j += ",\"items\":[\"Operator\",\"Engineer\",\"Supervisor\",\"HonPrec\"]";
    j += ",\"btLogin\":\"";
    j += s_btLoginIsLogout ? "Logout" : "Login";
    j += "\",\"systemStart\":";
    j += SystemStart ? "true" : "false";
    j += "}";
    return j;
}

<!-- preserved-content:end -->
```
