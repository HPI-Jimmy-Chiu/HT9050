# 原文 02／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/forms/fNote_ShowError.cpp`；定位 `W906_NoteNoticeAckRefusal`；種類 `complete_cpp_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `016f9d4adf4092c6b4ec5ebb709168f8913c9eae41f0fb6620420d57262be669`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
const char* W906_NoteNoticeAckRefusal(const char* requestId)
{
    if (!W906_Notice.valid || W906_Notice.requestId != AnsiString(requestId ? requestId : "")) return 0;   // [W906] not this notice: nothing to refuse (the close below does nothing either)
    if (!W906_Notice.goldenNote) return 0;                                      // [W906] pause 3: golden has no note, no BtnPauseClick
    if (false) return 0;                                                        // [W906] was pause 2 (SystemStart||SoftStart -> no refusal checks); AI(W906-I37A) 20261002: A -- golden refuses whatever the machine state
#if 0 // GATE(W906-J5-ACK) B1: golden :3828-3831 -- fNote->bNeedTCPAlarm (TfNote has none; Command.cpp's readers are comments / #if 0) and bErrPan_err / Pwd (0 definitions in this tree, git grep 20260930): the HandlerResultServer panel lock and the SpecialPanel password are not ported, so neither refuses here
    if(CosFunction.bEnableHandlerResultServer && bNeedTCPAlarm)                 //Sam 20230620 : 面板已經鎖定需要 IT 下命令解鎖
        return;
    if(bErrPan_err==true && Pwd!="")
        return;                                                                 //Richard 2011/2/22 SpecialPanel
#endif // GATE(W906-J5-ACK) B1
     if(CUSTOMER_CODE==CC_ASE_SG && bTesterSendPause==true)
        return "ASE_SG: the tester sent PAUSE (golden note.cpp:3833-3834)";    // golden :3834 return;
    return 0;
}

<!-- preserved-content:end -->
```
