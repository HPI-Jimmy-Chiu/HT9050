# 原文 05／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `AlarmRetire`；種類 `complete_header_inline_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `65e0ef914b7ea0b07b83b3488a521d02f01534af57f80eaa12352f95dce27eff`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
inline bool AlarmRetire(AlarmSlot& slot, const std::string& dir, unsigned long long& seqCounter)
{
    if (dir.empty()) return false;
    const unsigned long long seq = ++seqCounter;
    if (!MailboxPut(dir, "Alarm-dialog-request", AlarmIdleJson(seq))) return false;
    slot = AlarmSlot();
    return true;
}

<!-- preserved-content:end -->
```
