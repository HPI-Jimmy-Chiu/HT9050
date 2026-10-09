# 原文 15／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Ui.cpp`；function／region `TfRS232Main constructor: slRS232Log creation and SaveType`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `f4dbe9df81f3ce0ab206b82015778dd36500170a589ddaf8460745f1609d8157`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
    slRS232Log=new TMyStringList("D:\\RS232Log\\LOG",
                                 "RS232_Log",
                                 "Date, Time, Action, Message, Hex");           //Steven 20240913 : 變更RS232 log記錄方式
    slRS232Log->SaveType=TBy2Hour;
<!-- preserved-content:end -->
```
