# 原文 04／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `W906_NoticeAckCommand`；種類 `complete_cpp_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `7b60126dc47feb6061ec27247978b102434b5ebc81cd70b7ce79fed50581b8f0`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
void W906_NoticeAckCommand(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)
{
    const std::string tag = wc.hasTag ? wc.tag : std::string();
    std::string ack;
    const bool ok = w906dlg::NotifyAckHandle(g_alarmSlot, tag,
        [](const std::string& id) { const char* why = W906_NoteNoticeAckRefusal(id.c_str()); return std::string(why ? why : ""); },
        []() { return DialogMailboxRetire(); },
        [](const std::string& id, std::string* okJson) {
            int pause = 3; bool jam = false; unsigned long passSec = 0;
            W906_NoteNoticeAckLikeGolden(id.c_str(), &pause, &jam, &passSec);
            *okJson = w906dlg::NotifyAckOkJson(id, g_dialogSeq, pause, jam, passSec);
        },
        &ack);
    std::printf("dialog.notifyAck tag=%s -> %s %s\n", tag.c_str(), ok ? "ok" : "refused", ack.c_str());
    std::fflush(stdout);
    server.CompleteCommand((unsigned long long)wc.id, ok, ack);
}

<!-- preserved-content:end -->
```
