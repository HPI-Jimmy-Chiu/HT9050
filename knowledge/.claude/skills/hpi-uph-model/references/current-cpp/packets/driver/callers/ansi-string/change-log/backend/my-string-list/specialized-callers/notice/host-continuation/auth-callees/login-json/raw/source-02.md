# 原文 02：WebLogin_BookLogin

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `WebLogin_BookLogin`，完整CPP正文。
來源 commit `367d9d85792fa756db6e898c950d6d79931cbf74`；摘錄 SHA256 `b2a1a3d7e421f9221d02dbc765db93cc210d1dbb14305a53b42175fa51b1e5c3`。
歷史註解、客戶條件、裁決與常數保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
int WebLogin_BookLogin(const AnsiString& user, const AnsiString& password, const AnsiString& bookOverride,
                       std::string* msg)
{
    if(s_btLoginIsLogout)
    {
        // 審查 20260924 M4：golden btLoginClick :28082 只在 Caption=="Login" 時登入；Caption 是 Logout 時按下是登出
        *msg = "already logged in (btLogin shows Logout): log out first";
        return WEBLOGIN_ALREADY_IN;
    }
    const int r = WebLogin_BookCompare(user, password, bookOverride, msg);
    if(r==WEBLOGIN_OK)
        s_btLoginIsLogout=true;                                                 // golden :28091 btLogin->Caption="Logout"
    return r;
}

<!-- preserved-content:end -->
```
