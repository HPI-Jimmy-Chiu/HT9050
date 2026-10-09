# 原文 08／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Log.cpp`；function／region `TfRS232Main::ShowCommData(AnsiString sUnitName, vector<Byte>`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `ffe66cfcbd08c6b7f43904963be910d6dd8307180e01996a3690b1ac5d4f56a8`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void TfRS232Main::ShowCommData(AnsiString sUnitName, vector<Byte>bHex)          //Steven 20240913 : 變更RS232 log記錄方式
{
    if(MemoLog->Lines->Count>10240)
    {
        MemoLog->Clear();
    }

    //AI(W906-GB-P4) 20260926: VCL TStrings::Append(S) is `Add(S);` minus the returned index (Classes.pas); vclcompat
    //   TStringList has no Append, so the Add it forwards to is called directly (same line appended).
    MemoLog->Lines->Add(slRS232Log->AddTextWithHex(sUnitName, bHex));
}

<!-- preserved-content:end -->
```
