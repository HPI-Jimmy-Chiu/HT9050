# 原文 12／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `main_notice_dialog_auth_dispatch`；種類 `regions`。
來源 commit `144eba7f7b6c3b0e9a5fe1e77912c40ea4ee9b2c`；完整摘錄 SHA256 `d3bcac61637eac9511af1e8335883ec357c388eaf81957093108aeaacfb3f398`。
原註解、裁決、gate與歷史測試敘述保留；不是本輪實機驗證。

```cpp
<!-- preserved-content:start -->
            } else if (wc.cmd == "dialog.notifyAck" && [&]() -> bool { if (g_w906NoticeGatePassed.erase((unsigned long long)wc.id) != 0) return false;  /*AI(W906-NOTICE-DEFER-4) 20261007: MbWait already ran this gate at the press (wb_serve.cpp MbWait notifyAck arm)*/  extern bool W906_NoteAuthNoticeGate(const std::string&, std::string*); std::string nw; if (W906_NoteAuthNoticeGate(wc.hasTag ? wc.tag : std::string(), &nw)) return false; server.CompleteCommand((unsigned long long)wc.id, false, nw); return true; }()) {   /*AI(W906-D026) 20261001 St01: golden TfNote::BtnPauseClick KeyCode==0 arm (note.cpp:4084-4095) -> DoPassword: a notice golden guards with a password is refused (auth-required) until dialog.auth passed; answered inside the lambda. Same line, no line moves*/ } else if (wc.cmd == "dialog.notifyAck") { extern void W906_NoticeAckCommand(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_NoticeAckCommand(server, wc);   /*AI(W906-J5-ACK) 20260930: INBOX 119 -- the operator acknowledges a kCode==0 notice (body at EOF; never waits, never posts a query). Same line, so no line below moves*/ } else if (wc.cmd == "dialog.auth") {
                // AI(W906-Q30-8) 20260922: 密碼驗證 —— 契約 dialogAuth.verifier「C++ only. HTML never compares passwords or access levels.」。
                // AI(W906-D026) 20261001 St01: 接上了（原本在這裡誠實回 "dialog auth not wired"）。這一支是主迴圈的：
                //   現在沒有阻塞告警在等（等待迴圈自己收它那一則，ForwardShowErrorMessage 的 else 鏈同一條指令），只剩 kCode==0 的通知 ——
                //   golden TfNote::BtnPauseClick 的 KeyCode==0 臂（V912 note.cpp:4084-4095）→ TfNote::DoPassword（:5277-5429）。
                //   本體 WebLogin.cpp 檔尾 W906_NoteAuthVerify：比對只在 C++（WebLogin_BookCompare／stOperatorClick），
                //   回應不含密碼（S41／S55），這裡不印 value。通過＝留一張一次性通行給緊接著的 dialog.notifyAck（上一行的閘）。
                //   權杖：比照 dialog.response 免（WebBridge/WebBridgeServer.cpp 的豁免那一行）；防連點：WebCmdGuard 白名單（本來就有）。
                //   SpecialPanel 密碼（D-034，契約 kind special-note）也走這一支 W906_NoteAuthVerify；MyMessageBox::DoPassword_MBox（Configuration [I37_1] 走 editlist.save reauth）、SECS 工號檢查不在這裡。
                const std::string naCurrent = (g_alarmSlot.kind == w906dlg::AlarmSlot::kNotice) ? g_alarmSlot.requestId : std::string();
                extern bool W906_NoteAuthVerify(const std::string&, const std::string&, const std::string&, bool, std::string*);
                std::string naReply;
                //   （空行的位置：行數與原本的 15 行相同，下面的行號不動）
                const bool naOk = W906_NoteAuthVerify(wc.hasTag ? wc.tag : std::string(),
                                                      (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(),
                                                      naCurrent, false, &naReply);  server.CompleteCommand((unsigned long long)wc.id, naOk, naReply);

<!-- preserved-content:end -->
```
