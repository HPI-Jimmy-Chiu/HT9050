# 原文 05／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `W906_NoticePanelKeyTick`；種類 `complete_cpp_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `4b3a274e1a4ec8e27703655cf0ee521f75cf9a63a39fb450f2ebca9d502ef741`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
void W906_NoticePanelKeyTick()
{
    if (g_alarmSlot.kind != w906dlg::AlarmSlot::kNotice || g_alarmSlot.requestId.empty()) return;
    const std::string id = g_alarmSlot.requestId;
    const unsigned long long reqSeq = g_alarmSlot.seq;                          // the notice's request seq (AlarmPost), read before the retire clears the slot
    std::string ans, pressed, input;
    W906_AlarmIoAnswer(id.c_str(), 0, &ans, &pressed, &input);                  // golden Timer1Timer -> ScanKey (keeps its own per-note state)
    if (pressed != "BtnPause") return;
    extern bool W906_NoteAuthNoticeGate(const std::string&, std::string*);     // WebLogin.cpp (the gate in front of dialog.notifyAck, :4889)
    std::string why;
    if (!W906_NoteAuthNoticeGate(id, &why)) {
        std::printf("notice %s: panel PAUSE -- %s (the box stays; answer it on the screen)\n", id.c_str(), why.c_str());
        std::fflush(stdout);
        return;
    }
    std::string ack;
    const bool ok = w906dlg::NotifyAckHandle(g_alarmSlot, id,
        [](const std::string& rid) { const char* w = W906_NoteNoticeAckRefusal(rid.c_str()); return std::string(w ? w : ""); },
        []() { return DialogMailboxRetire(); },
        [](const std::string& rid, std::string* okJson) {
            int pause = 3; bool jam = false; unsigned long passSec = 0;
            W906_NoteNoticeAckLikeGolden(rid.c_str(), &pause, &jam, &passSec);
            *okJson = w906dlg::NotifyAckOkJson(rid, g_dialogSeq, pause, jam, passSec);
        },
        &ack);
    if (ok) W906_DialogCloseRequest("show-error-message", id, reqSeq, "PAUSE", 0, "SnFKPause");
    std::printf("notice %s: panel PAUSE -> %s %s\n", id.c_str(), ok ? "closed" : "refused", ack.c_str());
    std::fflush(stdout);
}

<!-- preserved-content:end -->
```
