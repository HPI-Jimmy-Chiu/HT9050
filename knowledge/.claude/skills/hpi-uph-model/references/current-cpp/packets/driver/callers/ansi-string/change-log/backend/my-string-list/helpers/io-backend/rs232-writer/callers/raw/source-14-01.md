# 原文 14／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Ui.cpp`；function／region `TfRS232Main::FormClose(`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `2aa678099b8ea15a42330b6687452a4f6e554d9a0f0e67e465b95e65a2eae57d`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void TfRS232Main::FormClose(TObject *Sender)
{
    CloseTesterComm();
    CloseTesterComm_TTL(0);                                                     //Isaac 20200903 :TTL RS232通訊
    CloseTesterComm_TTL(1);                                                     //Isaac 20210309 :TTL RS232兩塊板子

    SaveBinData();

    for(std::vector<TMyDutPanel *>::iterator iter=MY_DUT_PAL.begin(); iter!=MY_DUT_PAL.end(); ++iter)   //AI(W906-GB-P4) 20260926: vector -> std::vector
    {
        delete *iter;
        *iter=NULL;                                                             //AI(W906-GB-P4) 20260926: freed here, ~TfRS232Main skips it
    }
    sBarCode->Clear();                                                          //Ifor 20180917 (Steven) : Add TStringList 刪除前需先 Clean
    sBarCode_ASE_CL->Clear();                                                   //KaiChen 20191126 ：中壢日月光，2D 回傳格式
    delete sBarCode;
    sBarCode=NULL;                                                              //AI(W906-GB-P4) 20260926: the engine deletes the form after FormClose, and a later
                                                                                //   engine start re-creates the list; ~TfRS232Main frees only non-NULL
    delete sBarCode_ASE_CL;                                                     //KaiChen 20191126 ：中壢日月光，2D 回傳格式
    sBarCode_ASE_CL=NULL;                                                       //AI(W906-GB-P4) 20260926: see sBarCode above
    MY_DUT_PAL.clear();
}

<!-- preserved-content:end -->
```
