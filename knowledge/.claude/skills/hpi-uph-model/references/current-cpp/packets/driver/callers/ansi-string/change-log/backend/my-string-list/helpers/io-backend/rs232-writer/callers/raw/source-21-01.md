# 原文 21／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Globals.cpp`；function／region `void GetTimeInfo()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `f76e283466c5610f2768b4f085f14f1641c948a416cf5cfb9e8dfbb18463454c`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void GetTimeInfo()
{
    static TDateTime dtPresent;
    //AI(W906-GB-P4) 20260926: program restart (g_rs232Life): no re-arm needed -- dtPresent is pure scratch, assigned
    //  from Now() on the next line before every read, so a value left by the previous program life is never seen.
    dtPresent=Now();
    DecodeDate(dtPresent, SystemYear, SystemMonth, SystemDate);
    DecodeTime(dtPresent, SystemHour, SystemMin, SystemSec, SystemMSec);
}

<!-- preserved-content:end -->
```
