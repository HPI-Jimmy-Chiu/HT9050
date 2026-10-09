# 原文 05／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Support.cpp`；function／region `TMyStringList::AddTextWithHex(`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `c0d218109943b3af61394515bd07da2a1b344fb47246391d64287581054c632f`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
AnsiString TMyStringList::AddTextWithHex(AnsiString sUnitName, std::vector<Byte>bHex)
{
    AnsiString Str;
    GetTimeInfo();

    int iSize=bHex.size();
    AnsiString sDataHex="";
    AnsiString sDataAscii="";
    for(int i=0; i<iSize; i++)
    {
        sDataHex+=IntToHex((int)bHex[i], 2)+" ";
        sDataAscii+=MyDeCodeASCII((int)bHex[i]);
    }

    if(iSize==0)
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"\",\"\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName);
    }
    else
    {
        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, \"%s\", \"%s\"",
                     SystemYear, SystemMonth, SystemDate,
                     SystemHour, SystemMin, SystemSec, SystemMSec,
                     sUnitName, sDataAscii, sDataHex);
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
