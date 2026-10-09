# 原文 04／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Support.cpp`；function／region `TMyStringList::AddText(`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `be682569fb35754ce11ab5f0cebc9141ba2166bef6aba3b2f0b5b4a760222739`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
AnsiString TMyStringList::AddText(AnsiString sUnitName, AnsiString sMsg1, AnsiString sMsg2)
{
    AnsiString Str;
    GetTimeInfo();
    if(sMsg2=="" && sMsg1=="")
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"\",\"\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName);
    }
    else if(sMsg2=="")
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"%s\",\"\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName, sMsg1);
    }
    else
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"%s\",\"%s\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName, sMsg1, sMsg2);
    }

    if(FileName=="")
        FileName="";

    if(MyList->Count>HTMaxLineCount)
    {
        MySaveToFile();
        MyList->Clear();
    }
    MyList->Add(Str);
    return Str;
}

<!-- preserved-content:end -->
```
