# 原文 07／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Support.cpp`；function／region `TMyStringList::GetFileName()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `0ddb9b005e5ce26554eefc210c1e4592c47e9058128fb6c3bc5377f0c50b1e78`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
AnsiString TMyStringList::GetFileName()
{
    AnsiString sPathName;
    AnsiString sFileName="";
    AnsiString Str;
    int iHour;
    GetTimeInfo();

    if(HTPath=="")
        Path="D:\\HandlerLog";

    if(FileName=="")
        FileName="";

    if(bFilePathWithDate)                                                       //Steven 20210623 : 預設存檔要有日期當資料夾
    {
        if(HTSaveType>=TByMonth)                                                //使用年月存檔的話,就By年分類
        {
            sPathName.sprintf("%s\\%04d", HTPath, SystemYear);
        }
        else //if(HTSaveType>=TByDay)                                           //使用每日存檔的話,就By月分類
        {
            sPathName.sprintf("%s\\%04d\\%02d", HTPath, SystemYear, SystemMonth);
        }
    }
    else
    {
        sPathName=HTPath;
    }

    MyForceDirectories(sPathName);
    if(HTSaveType==TByMaxLineCount)
    {
        sFileName.sprintf("%s\\%s_%04d%02d%02d %02d%02d%02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    }
    else
    {
        if(HTSaveType==TByHour)
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour);
        }
        else if(HTSaveType==TBy2Hour)
        {
            iHour=SystemHour-SystemHour%2;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy4Hour)
        {
            iHour=SystemHour-SystemHour%4;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy6Hour)
        {
            iHour=SystemHour-SystemHour%6;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy8Hour)
        {
            iHour=SystemHour-SystemHour%8;
            sFileName.sprintf("%s\\%s_%04d%02d%02d %02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, iHour);
        }
        else if(HTSaveType==TBy12Hour)
        {
            if(SystemHour<8)
            {
                GetYesterdayInfo();
                sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.log", sPathName, HTFileName, SystemYearYesterday, SystemMonthYesterday, SystemDateYesterday, 20);
            }
            else if(SystemHour>=20)
            {
                sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, 20);
            }
            else
            {
                sFileName.sprintf("%s\\%s_%04d%02d%02d%02d00.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, 8);
            }
        }
        else if(HTSaveType==TByDay)
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate);
        }
        else if(HTSaveType==TByMonth)
        {
            sFileName.sprintf("%s\\%s_%04d%02d.log", sPathName, HTFileName, SystemYear, SystemMonth);
        }
        else if(HTSaveType==TByMin)
        {
            sFileName.sprintf("%s\\%s_%04d%02d%02d_%02d%02d.log", sPathName, HTFileName, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        }
        else //if(HTSaveType==TByYear)
        {
            sFileName.sprintf("%s\\%s_%04d.log", sPathName, HTFileName, SystemYear);
        }
    }
    return sFileName;
}

<!-- preserved-content:end -->
```
