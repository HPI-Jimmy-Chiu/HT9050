# 原文 14／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `main_dispatch_notice_auth`；種類 `regions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `9939c811cf752a99f3eb205cd108568da7d66c848e02e2e6ed17d1690d11261a`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
            } else if (wc.cmd == "dialog.notifyAck" && [&]() -> bool { if (g_w906NoticeGatePassed.erase((unsigned long long)wc.id) != 0) return false;  /*AI(W906-NOTICE-DEFER-4) 20261007: MbWait already ran this gate at the press (wb_serve.cpp MbWait notifyAck arm)*/  extern bool W906_NoteAuthNoticeGate(const std::string&, std::string*); std::string nw; if (W906_NoteAuthNoticeGate(wc.hasTag ? wc.tag : std::string(), &nw)) return false; server.CompleteCommand((unsigned long long)wc.id, false, nw); return true; }()) {   /*AI(W906-D026) 20261001 St01: golden TfNote::BtnPauseClick KeyCode==0 arm (note.cpp:4084-4095) -> DoPassword: a notice golden guards with a password is refused (auth-required) until dialog.auth passed; answered inside the lambda. Same line, no line moves*/ } else if (wc.cmd == "dialog.notifyAck") { extern void W906_NoticeAckCommand(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_NoticeAckCommand(server, wc);   /*AI(W906-J5-ACK) 20260930: INBOX 119 -- the operator acknowledges a kCode==0 notice (body at EOF; never waits, never posts a query). Same line, so no line below moves*/ } else if (wc.cmd == "dialog.auth") {

<!-- preserved-content:end -->
```
