# 原文 02／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/WebLogin.cpp`；定位 `Press`；種類 `complete_cpp_functions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `d49f2ad581dd573f891c66f73404764ad006f58a40f4bc408501cf10d00fc88a`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
bool Press(Note& n, int sel, const Cred* cred, Out* o)
{
    if(IniConfig.bF15OutShuttleLoseICNeedPWD)                                   //ChungHung 20120912 Amkor 需求Shuttle lose ic need password Only Skip
    {
        if((n.tempCode=="JAM0508" || n.tempCode=="JAM0509") && sel!=0)          // Select[0]!=true
            n.bNeedPassWord=false;
    }
    if(sel>=0)
    {
        // golden :3592-3596 / :3914-3918 KYEC_LEE MES0923 (bAutoRetestJam / bNeedKeyInSkipIC): not the password, not here
        if(DoUnlockPassword(n, cred, o)==false)                                 //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
        {
            if(o->reason.empty()) o->reason = o->unlockAsks ? "the alarm unlock password did not match (note.cpp:5438)"
                                                            : "bAlarmUnlockPassWord is still set (note.cpp:5442-5443)";
            return false;
        }
        if(cred && o->unlockPassed)                                             // [W906] banner: the second box of the same press
        {
            Note probe = n;
            Out po;
            if(DoPassword(probe, sel, nullptr, &po)==false && po.asks)
            {
                o->loginNext = true;
                o->mode = po.mode;
                o->required = po.required;
                o->reason = "alarm unlock password accepted (note.cpp:5438-5439); golden now opens the login box (DoPassword)";
                return false;
            }
        }
    }
    if(DoPassword(n, sel, cred, o)==false)                                      //Steven 20101124
        return false;
    return true;
}

<!-- preserved-content:end -->
```
