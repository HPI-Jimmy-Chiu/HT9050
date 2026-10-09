# 原文 01／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `DialogMailboxRetire`；種類 `complete_cpp_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `1e10f4af8c43e5ee5f5ddf8dbbaf39cdebe7660e47d7848a25f4e938d6704708`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
static bool DialogMailboxRetire()   //AI(W906-J5-ACK) 20260930: returns true = both files are idle (was void; only W906_NoticeAckCommand reads it)
{
    if (g_dialogMailboxDir.empty()) return false;
    //AI(W906-J5-ACK) 20260930: `++g_dialogSeq` and the JSON are w906dlg::AlarmRetire's now (tools/wb_dialog_mailbox.h AlarmIdleJson)
    // ⚠⚠ 20260922 端到端實測抓到：這裡原本是 `char json[256]`，而下面那串
    //   格式化出來約 500 字元 ⇒ `snprintf` **靜默截斷**，寫出一個少了後半段
    //   的壞 JSON。症狀不是「退役失敗」而是
    //   `Invalid control character at column 256` —— 而且只有在**解析**
    //   那個檔的時候才看得到，wb_serve 自己完全不會報錯。
    //   ⇒ 開大到 1024 並在下面斷言沒被截斷。
    //   [AI(W906-J5-ACK) 20260930: now one std::string (w906dlg::AlarmIdleJson) -- no buffer, nothing to truncate; the same
    //    text, byte for byte (tests/test_notice_ack.cpp [A] against a verbatim copy of the 1024-byte snprintf).]
    // ⚠ snprintf 截斷是**靜默**的 —— 它回「本來要寫幾個字元」而不是實際寫的。
    //   不檢查就會寫出半截 JSON，而那只有在對方解析時才炸。   [AI(W906-J5-ACK) 20260930: history -- the check went with the buffer]

    // AI(W906-J5-ACK) 20260930: w906dlg::AlarmRetire also sets g_alarmSlot idle -- only when BOTH files were written, so
    //   a failed retire leaves the request acknowledgeable (a second dialog.notifyAck retries it) instead of reporting
    //   "no-pending-notice" over a box the page still shows.
    const bool ok = w906dlg::AlarmRetire(g_alarmSlot, g_dialogMailboxDir, g_dialogSeq);
    if (!ok)
        std::printf("  ⚠ dialog mailbox retire FAILED -- 下次重整會再彈一次\n");
    return ok;
}

<!-- preserved-content:end -->
```
