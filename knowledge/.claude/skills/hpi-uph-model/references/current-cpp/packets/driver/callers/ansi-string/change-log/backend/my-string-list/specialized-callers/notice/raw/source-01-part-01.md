# 原文 01／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/forms/fNote_ShowError.cpp`；定位 `W906_NoteNoticeCapture`；種類 `complete_cpp_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `e65c045566b6ea2b65781e741495ce8dcb17f1726c60ecced1864969a8a50b74`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
void W906_NoteNoticeCapture(const char* requestId, bool motorNote)             // [W906] see the banner
{
    W906_Notice.valid          = true;
    W906_Notice.requestId      = AnsiString(requestId ? requestId : "");
    W906_Notice.code           = (fNote != 0) ? fNote->edErrorCode->Text : AnsiString("");
    W906_Notice.alarmType      = (fNote != 0) ? fNote->AlarmType : 0;
    W906_Notice.duplicateError = iDuplicateError;
    W906_Notice.eventId        = iEventID;
    W906_Notice.goldenNote     = motorNote ? true : W906_ShowErrorMessage_Recorded;   // the motor body returns before the host when InitialOK is false (:1061-1065)
    W906_Notice.answerApplied  = motorNote;
    if (W906_Notice.goldenNote)
    {
    W906_tNoteTimer.LatchCycleTimeSec(true);                                    // golden TfNote::FormShow :1526
    Recovery="";                                                                //Steven 20120209 : 每次進來都要初始化   [golden FormShow :2130]
    }
}

<!-- preserved-content:end -->
```
