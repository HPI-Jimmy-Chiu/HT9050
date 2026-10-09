# 原文 13／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Ui.cpp`；function／region `TfRS232Main::FormDestroy(`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `66e30d9b3c4d481102dadc85ad4256cc00cc083dd591fb269ea2dd87154fc072`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
void TfRS232Main::FormDestroy(TObject *Sender)
{
    InitialOK=false;
    slRS232Log->Clear();
    delete slRS232Log;
    slRS232Log=NULL;                                                            //AI(W906-GB-P4) 20260926: the engine deletes the form after FormDestroy
                                                                                //   (Rs232Engine::Teardown); ~TfRS232Main frees only non-NULL
}

<!-- preserved-content:end -->
```
