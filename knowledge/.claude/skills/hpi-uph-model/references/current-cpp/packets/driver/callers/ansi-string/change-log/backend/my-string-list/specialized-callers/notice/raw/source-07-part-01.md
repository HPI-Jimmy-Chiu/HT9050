# 原文 07／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `NotifyAckDecide`；種類 `complete_header_inline_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `3972d5768d3a6c1c9e1e4885dd3c6c3745cb50b964c176570e483beb3c72d553`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
inline NotifyAckVerdict NotifyAckDecide(const AlarmSlot& slot, const std::string& tag)
{
    if (slot.kind == AlarmSlot::kNotice && tag == slot.requestId) return kNotifyAckRetire;
    if (!tag.empty() && slot.WasSuperseded(tag)) return kNotifyAckSuperseded;  //AI(W906-J5-ACK-2) 20261001: an older notice this slot overwrote -- its box is stale, let the page close it
    //AI(W906-S17B) 20261003 (St01): the superseded list keeps kSupersededMax ids; an unattended loop (ShowLoadingIC: 8 notices
    //  in 4 s) passes 64 in ~30 s and the oldest boxes answered request-mismatch again (INBOX 137).  Ids only grow, so an id
    //  older than the one the slot holds -- and held by no wait loop -- is a box C++ has left behind: superseded, the page closes it.
    if (slot.kind != AlarmSlot::kIdle && AlarmIdOlder(tag, slot.requestId) && !TagWaiting(tag)) return kNotifyAckSuperseded;
    if (slot.kind == AlarmSlot::kBlocking) return kNotifyAckNotANotice;       // answered by the wait loop, never here
    if (slot.kind != AlarmSlot::kNotice)   return kNotifyAckNoPendingNotice;  // idle: already closed (idempotent)
    return kNotifyAckRequestMismatch;                                         // an id this slot never held
}

<!-- preserved-content:end -->
```
