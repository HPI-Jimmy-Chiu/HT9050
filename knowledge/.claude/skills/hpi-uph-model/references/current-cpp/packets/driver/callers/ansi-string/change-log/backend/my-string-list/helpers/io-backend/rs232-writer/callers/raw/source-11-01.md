# 原文 11／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Log.cpp`；function／region `TfRS232Main::AddBinData(`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `629b7115b1b2969980f9e08624682d666cfc8bc3635188c966b6c79f760f0bbd`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void TfRS232Main::AddBinData(AnsiString sBinData)
{
    AnsiString Str;
    if(iUseRS232Mode>=InterfaceType_TTL)                                        //Isaac 20200903 :TTL RS232通訊
    {
        if(MemoBinData_TTL->Lines->Count>10000)
        {
            SaveBinData();
        }
        Str.sprintf("%04d-%02d-%02d,%02d:%02d:%02d.%03d, %s",
                     SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, sBinData);
        MemoBinData_TTL->Lines->Add(Str);
    }
    else if(iUseRS232Mode==2)
    {
        if(MemoBinData->Lines->Count>10000)
        {
            SaveBinData();
        }

        Str.sprintf("%04d-%02d-%02d,%02d:%02d:%02d.%03d, %s",
                     SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, sBinData);
        MemoBinData->Lines->Add(Str);
    }
    else //if(iUseRS232Mode==0) ==1
    {
        if(MemoBinData->Lines->Count>10000)
        {
            SaveBinData();
        }

        Str.sprintf("%04d-%02d-%02d, %02d:%02d:%02d, %s",
                     SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, sBinData);
        MemoBinData->Lines->Add(Str);
    }
}

<!-- preserved-content:end -->
```
