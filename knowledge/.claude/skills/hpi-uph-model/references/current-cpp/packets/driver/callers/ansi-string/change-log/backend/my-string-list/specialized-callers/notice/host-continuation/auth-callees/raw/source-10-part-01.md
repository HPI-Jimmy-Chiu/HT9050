# 原文 10／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `Typed`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `2715e37ef405ff1dd71701f4d45458eb6fb7732ff9441532a0a5d0004a07e8bd`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
AnsiString Typed(const std::string& u)
{
    AnsiString a;
    if (pw::Acp(u, &a)) return a;
    return AnsiString(u.c_str());
}

<!-- preserved-content:end -->
```
