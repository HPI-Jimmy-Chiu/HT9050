# 原文 10／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `NotifyAckHandle`；種類 `complete_header_inline_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `cd7b9391606ac36deccf8ed2b9394fb7b233121b2fe9581fa53e07e17ba6da4c`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
template <class Refuse, class Retire, class Close>
inline bool NotifyAckHandle(const AlarmSlot& slot, const std::string& tag, Refuse refuse, Retire retire, Close close,
                            std::string* ack)
{
    const NotifyAckVerdict v = NotifyAckDecide(slot, tag);
    if (v != kNotifyAckRetire) { *ack = NotifyAckError(v, slot); return false; }
    const std::string id = slot.requestId;
    const std::string why = refuse(id);
    if (!why.empty()) { *ack = "golden-refused:" + why; return false; }
    if (!retire()) { *ack = "retire-failed:current=" + id; return false; }
    close(id, ack);
    return true;
}

<!-- preserved-content:end -->
```
