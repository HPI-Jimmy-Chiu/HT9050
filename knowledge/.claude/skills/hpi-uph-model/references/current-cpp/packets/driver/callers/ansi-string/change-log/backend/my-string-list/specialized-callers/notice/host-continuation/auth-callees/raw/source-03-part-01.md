# 原文 03／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `DoUnlockPassword`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `ac8a97925acb09117ff1df4acac37479c9ee10e6c32e49f0e56352bbb1c60383`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
bool DoUnlockPassword(Note& n, const Cred* cred, Out* o)
{
    bool bFlag=true;
    if(CosFunction.bUseAlarmUnlockPassWord==true && n.bAlarmUnlockPassWord==true)
    {
        o->unlockAsks=true;
        if(!cred) return false;                                                 // [W906] asked only "would golden stop here"
        LoadUnlockPassword();
        AnsiString typed = cred->cancelled ? AnsiString("") : reauth::Typed(cred->password);  reauth::WipeOnExit wipeTyped(&typed);   // fPassword->edPassword->Text="" then the keypad
        if(!s_unlockMissing && asUnlockPassword==typed)                         // [W906] !s_unlockMissing: banner
        {
            n.bAlarmUnlockPassWord=false;
            o->unlockPassed=true;
        }
    }

    if(n.bAlarmUnlockPassWord==true)
        bFlag=false;

    return  bFlag;
}

<!-- preserved-content:end -->
```
