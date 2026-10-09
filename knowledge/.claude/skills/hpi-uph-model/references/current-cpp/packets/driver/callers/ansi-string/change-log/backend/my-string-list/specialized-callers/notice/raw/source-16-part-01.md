# 原文 16／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `ForwardShowErrorMessage_capture_call`；種類 `regions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `5581b03fb73a3d4670eaa32aa80e6daaa6aef877b681c2e5fc9b91b3dc38553d`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
        const bool noteOnly = DialogMailboxPostAlarm(qidStr, code, kcode, pos);  { extern void W906_NoteNoticeCapture(const char*, bool); W906_NoteNoticeCapture(qidStr, g_w906MotorNoteMsg != 0); }   //AI(W906-J5-ACK) 20260930: keep what golden FormClose reads from fNote (code / AlarmType / iDuplicateError / iEventID) for the ack -- forms/fNote_ShowError.cpp EOF

<!-- preserved-content:end -->
```
