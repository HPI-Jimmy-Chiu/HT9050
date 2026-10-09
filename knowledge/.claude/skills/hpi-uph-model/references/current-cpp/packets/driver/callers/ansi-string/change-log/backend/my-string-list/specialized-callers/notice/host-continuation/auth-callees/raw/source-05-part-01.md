# 原文 05／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `WebLogin_UsesBook`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `d51f38a15629709aa109e1c31a44fee24a2be0b0fa04a0dc5a48b71b564c854c`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
bool WebLogin_UsesBook()
{
    return FileExists(pwPath) || CosFunction.bUseLoginDatToSetLevel;
}

<!-- preserved-content:end -->
```
