# 原文 10／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Log.cpp`；function／region `TfRS232Main::SaveBinData()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `8638f22634a5d4cca2f020175be6d55ef922bda231287e08e706a3f88b06bb2a`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void TfRS232Main::SaveBinData()
{
    if(iUseRS232Mode==0 && MemoBinData->Lines->Count==0)                        //Isaac 20200903 :TTL RS232通訊
        return;
    if(iUseRS232Mode>=InterfaceType_TTL && MemoBinData_TTL->Lines->Count==0)
        return;
    AnsiString FileName;

    if(iUseRS232Mode>=InterfaceType_TTL)                                        //Isaac 20200903 :TTL RS232通訊
    {
        FileName.sprintf("D:\\RS232Log\\BinLog_TTL\\%04d_%02d", SystemYear, SystemMonth);
        MyForceDirectories(FileName);

        FileName.sprintf("D:\\RS232Log\\BinLog_TTL\\%04d_%02d\\%04d_%02d_%02d_%02d %02d %02d_TTL_Rs232_BinData.log",
                         SystemYear, SystemMonth,
                         SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        MemoBinData_TTL->Lines->SaveToFile(FileName);
        MemoBinData_TTL->Clear();
    }
    else //if(iUseRS232Mode==0)
    {
        FileName.sprintf("D:\\RS232Log\\BinLog\\%04d_%02d", SystemYear, SystemMonth);
        MyForceDirectories(FileName);

        FileName.sprintf("D:\\RS232Log\\BinLog\\%04d_%02d\\%04d_%02d_%02d_%02d %02d %02d_Rs232_BinData.log",
                         SystemYear, SystemMonth,
                         SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        MemoBinData->Lines->SaveToFile(FileName);
        MemoBinData->Clear();
    }
}

<!-- preserved-content:end -->
```
