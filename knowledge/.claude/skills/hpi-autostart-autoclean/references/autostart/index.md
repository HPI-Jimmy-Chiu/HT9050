# AutoStart協定與原文

- [原完整主體](original-entry.md)：GTK Loader→Agent→Handler五階段、角色、Lot／Setup、Safety Interlock、Info Mismatch。
- [OLP協定](references/olp-protocol.md)：STX／SOH／ETX、命令／Data及socket角色。
- [指令規格](references/tcp-commands.md)：規格暫定HTSET對應與實際OLP字串分開。
- [階段與供應商需求](references/stage-details.md)：外部Handshake／3秒重試、未定義項保留原日期。
- [目前main](../runtime/start.md)：OLP的基底Start仍空，7016走W906_RemoteRun，不能把原文「Auto Start No Action」套到所有入口。

EndLot／StartLot與Recipe寫入相鄰，必要時另查LotInfo的目前實作；本批不發送命令或下載／套用Setup。
