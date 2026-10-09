# 原文 09／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Log.cpp`；function／region `TfRS232Main::ShowCommData(AnsiString sUnitName, AnsiString`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `466e477716b7e06e4fb49a9c38f1d6da29c73ae52b23142fe6cd302ea4dc6cb4`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void TfRS232Main::ShowCommData(AnsiString sUnitName, AnsiString sMsg1, AnsiString sMsg2)    //Steven 20240913 : 變更RS232 log記錄方式
{
    if(MemoLog->Lines->Count>10240)
    {
        MemoLog->Clear();
    }

    //AI(W906-GB-P4) 20260926: Append -> Add, as above.
    MemoLog->Lines->Add(slRS232Log->AddText(sUnitName, sMsg1, sMsg2));
}

<!-- preserved-content:end -->
```
