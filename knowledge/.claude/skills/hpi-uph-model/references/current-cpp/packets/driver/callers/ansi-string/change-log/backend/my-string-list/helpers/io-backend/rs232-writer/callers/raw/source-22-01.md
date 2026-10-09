# 原文 22／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Globals.cpp`；function／region `int MyForceDirectories(`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `0b24668a2ae25c8b1623398b5de49cd699800ba497c7bdcada48a80fad1876e5`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
int MyForceDirectories(AnsiString Directory, AnsiString Function)               //Steven 20210112 : 針對資料夾加上保護
{
    AnsiString Str1="", Str2="";
    if(Directory=="")
    {
        RecordProcess("Directory value is NULL!", Function);
        return -1;
    }
    else
    {
        try
        {
            if(DirectoryExists(Directory)==false)
            {
                ForceDirectories(Directory);
            }
        }
#if 0 // TODO(W906-GB-P4): vclcompat has no VCL Exception class (E.HelpContext / E.Message); golden cmydef.cpp:598-606.
      //   Anything thrown now lands in the catch(...) below: same "Force Directory FAIL!" line and the same -1, minus
      //   golden's two detail lines (Exception Code / Exception Message).  Tree precedent: common.cpp:2005-2015.
        catch(Exception& E)                                                     //Steven 20140505 : 試著抓出連線異常的訊息
        {
            Str1.sprintf("Function: %s, Directory:%s", Function, Directory);
            fRS232Main->ShowCommData("Exception", "Force Directory FAIL!", Str1);
            Str1.sprintf("Exception Code : %d", E.HelpContext);
            Str2.sprintf("Exception Message : %s", E.Message);
            fRS232Main->ShowCommData("[Exception]",  Str1, Str2);
            return -1;
        }
#endif
        catch(...)
        {
            Str1.sprintf("Function: %s, Directory:%s", Function, Directory);
            fRS232Main->ShowCommData("Exception", "Force Directory FAIL!", Str1);
            return -1;
        }
    }

    return 1;
}

<!-- preserved-content:end -->
```
