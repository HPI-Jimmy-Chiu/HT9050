# 原文 08／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `NotifyAckError`；種類 `complete_header_inline_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `394304cea822f027a90679e9aa85454832fe421a2ba82cd46bd54464e56c8912`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
inline std::string NotifyAckError(NotifyAckVerdict v, const AlarmSlot& slot)
{
    switch (v) {
        case kNotifyAckNoPendingNotice: return "no-pending-notice";
        case kNotifyAckSuperseded:      return "no-pending-notice:superseded-by=" + slot.requestId;   //AI(W906-J5-ACK-2) 20261001: same code as idle -- the page closes the box (ht9045_dialog_host.js tests the prefix)
        case kNotifyAckRequestMismatch: return "request-mismatch:current=" + slot.requestId;
        case kNotifyAckNotANotice:      return "not-a-notice:current=" + slot.requestId;
        default:                        return std::string();
    }
}

<!-- preserved-content:end -->
```
