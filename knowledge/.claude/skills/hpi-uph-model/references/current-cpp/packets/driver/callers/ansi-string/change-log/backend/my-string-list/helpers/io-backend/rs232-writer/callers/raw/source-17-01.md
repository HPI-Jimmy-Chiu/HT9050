# 原文 17／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Engine.cpp`；function／region `Rs232Engine::OverrideIniPaths(`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `aae5c9f4e6530d1ba439a8fee771103aabd6b87837cb9c433f014ac956bc1367`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void Rs232Engine::OverrideIniPaths(const std::string& setupIni, const std::string& handlerGeneralIni)
{
    g_setupIniOverride = setupIni;
    g_hgenIniOverride = handlerGeneralIni;
}

<!-- preserved-content:end -->
```
