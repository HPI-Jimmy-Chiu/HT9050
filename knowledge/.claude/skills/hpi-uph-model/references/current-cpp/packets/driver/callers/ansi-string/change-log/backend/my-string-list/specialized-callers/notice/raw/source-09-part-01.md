# 原文 09／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `NotifyAckOkJson`；種類 `complete_header_inline_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `6656dfd5d929097190f6f4c723071d31f5dd86fc6629116acbe7e8c5b97ba593`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
inline std::string NotifyAckOkJson(const std::string& requestId, unsigned long long retireSeq, int pause,
                                   bool jamCounted, unsigned long passTimeSec)
{
    static const char* const kPause[] = { "applied", "already-applied", "skipped-machine-running", "no-golden-note" };
    const char* p = (pause >= 0 && pause <= 3) ? kPause[pause] : "unknown";
    char tail[96];
    std::snprintf(tail, sizeof(tail), ",\"jamCounted\":%s,\"passTime\":%lu}", jamCounted ? "true" : "false", passTimeSec);
    return "{\"notice\":\"retired\",\"requestId\":\"" + JsonEscape(requestId) + "\",\"seq\":" + AlarmU64(retireSeq) +
           ",\"pause\":\"" + p + "\"" + tail;
}

<!-- preserved-content:end -->
```
