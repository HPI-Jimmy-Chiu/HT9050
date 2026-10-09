# 原文 08／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_dialog_mailbox.h`；定位 `AlarmIdleJson`；種類 `complete_header_inline_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `714aa5b5acee1095b442d62b8cba509f8fc65b30a1849b444e4be18ad70e01f2`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
inline std::string AlarmIdleJson(unsigned long long seq)
{
    return "{\"schemaVersion\":\"1.0.0\",\"channel\":\"show-error-message\","
        "\"seq\":" + AlarmU64(seq) + ",\"requestId\":\"\",\"state\":\"idle\","
        "\"function\":\"ShowErrorMessage\",\"blocking\":true,"
        "\"arguments\":{\"code\":\"\",\"kCode\":0,\"position\":0,"
        "\"duplicateError\":false,\"errorPart\":\"\"},"
        "\"display\":{\"alarmType\":null,\"unitName\":\"\",\"message\":\"\","
        "\"jamArea\":\"\",\"description\":\"\",\"flushPanel\":null},"
        "\"auth\":{\"required\":false,\"kind\":\"none\",\"level\":null,"
        "\"title\":\"Password\",\"prompt\":\"\",\"userIdRequired\":true,"
        "\"defaultUserId\":null},"
        "\"buttons\":[],\"closePolicy\":\"acknowledge-only\",\"error\":null}";
}

<!-- preserved-content:end -->
```
