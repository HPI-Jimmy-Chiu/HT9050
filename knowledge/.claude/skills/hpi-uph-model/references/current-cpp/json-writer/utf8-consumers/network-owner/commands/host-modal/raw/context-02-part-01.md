# 原文：modal globals and full adjacent rationale, carry definition overlaps separately counted body（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 38行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
// AI(W906-FW-W5b) 20260819: ShowErrorMessage -> browser query, ANSWER flows
// back. Golden blocks its UI thread in a modal loop until the operator picks
// RETRY / SKIP / CLEAN_OUT; the equivalent here is this pump: it blocks the
// tick thread, drains the queue itself, and refuses every command except the
// matching modal.answer with "modal-pending" -- the transport-level rendering
// of VCL modality (everything behind the dialog is inert until it is
// answered). No timeout, faithfully: golden waits forever. Tag publishing
// also freezes while pumping, exactly as golden's blocked UI thread would.
static webbridge::CommandQueue* g_pumpQueue = 0;
static unsigned long long       g_nextQid  = 1;
static std::deque<webbridge::WebCommand> g_carry; static bool g_carryRunnable = false; static bool g_outputsServed = false; static bool g_apiCacheDirty = false; static void W906_TakeCarry(std::vector<webbridge::WebCommand>& out) { for (std::size_t k = 0; k < g_carry.size(); ++k) out.push_back(g_carry[k]); g_carry.clear(); g_carryRunnable = false; }   // AI(W906-IOWEB-P25) 20260925: OUTPUT FIRST. g_carry = commands the output-first service (W906_ServiceOutputs, EOF) took off the queue but did NOT run (it runs outputs only); they keep their arrival order and the next drain -- the main loop's AND the modal wait's below -- takes them ahead of anything newer. g_apiCacheDirty replaces the three unconditional W906_ApiCacheRefresh() calls (20-160 ms each, measured by the click log): rebuilt once per loop, after the drain. On the old blank line, so no line below moves
// AI(W906-Q30-8) 20260922: 信箱目錄。由 main() 在決定 root 之後填。
//   ⚠ 空字串 = 沒接信箱（例如單元測試或還沒 init），此時只走 WebSocket，
//     行為與 20260921 相同 —— 不要讓「沒設定」變成「靜默不發警報」。
static std::string g_dialogMailboxDir;

// AI(W906-Q30-8) 20260922: 每個通道的 seq。契約 transport.ordering 要求
//   「seq must increase monotonically」，而 dialog-bridge.js:402 的條件是
//   `seq > channel.lastSeq`，且 lastSeq 初值 0 ⇒ **seq 必須 >= 1**。
//   樣板檔裡的 0 永遠不會大於 0 ⇒ 用 0 的話框永遠不開，而且不報錯。
//
// AI(W906-Q30-IDLE) 20260923: ⚠ 起點在 main() 改成「開機當下的 Unix 毫秒數」
//   （w906dlg::UnixMillisNow()），不再是 0。
//   理由：dialog-bridge.js 的 channel.lastSeq 是**頁面**的記憶體狀態
//   （dialog-bridge.js:7-10），只有重整頁面才歸 0；而量產是 file: 協定，頁面
//   直接讀檔、**不會因為 wb_serve 重開而重整**。舊寫法每次開機都從 1 數起
//   ⇒ 頁面沒重整、只有 wb_serve 重開時，新告警的 seq（1, 2, …）小於頁面記住的
//   lastSeq，:692 的 `seq > channel.lastSeq` 不成立 ⇒ **框不出來，C++ 在等**。
//   以時間當起點，後一個行程的 seq 一定大於前一個行程發過的任何 seq
//   （前提：時鐘不倒退，且每毫秒不超過一則）。0 仍是這裡的初值，只給
//   「main() 還沒跑到那一行」的極短窗口與單元測試用。
static unsigned long long g_dialogSeq = 0;  static const char* g_w906MotorNoteUnit = 0;  static const char* g_w906MotorNoteMsg = 0;   //AI(W906-JAM-STOP) 20260930: non-null only while ForwardShowMotorErrorMessage (EOF) posts golden ShowMotorErrorMessage's note through ForwardShowErrorMessage's kcode==0 branch -- the note's unit alias / message (golden note.cpp:1102 ErrShowToForm), and "skip the ShowErrorMessage-only stop + record" (the motor body did its own, golden :1054-1092).  Tick thread only
static w906dlg::AlarmSlot g_alarmSlot;  static std::set<unsigned long long> g_w906NoticeGatePassed;   //AI(W906-NOTICE-DEFER-5) 20261007: review of 0c22df0 -- keyed by the COMMAND id (unique, never reused), was the tag: a held ack consumed by another wait (ForwardShowErrorMessage / YesNo answer a carried notifyAck themselves) left the tag behind and the operator's NEXT PAUSE on the same notice skipped the gate; a second ack in the same wait overwrote the first's tag.  AI(W906-NOTICE-DEFER-4) 20261007: + the id of a notifyAck whose auth gate MbWait already ran (the main-loop arm then skips it -- the gate spends the one-shot pass).  AI(W906-J5-ACK) 20260930: what the Alarm-dialog-request mailbox holds now (tools/wb_dialog_mailbox.h AlarmSlot: idle / notice / blocking) -- written only by DialogMailboxPostAlarm / DialogMailboxRetire (through w906dlg::AlarmPost / AlarmRetire), read by W906_NoticeAckCommand (EOF).  Tick thread only.  On the old blank line, so no line below moves
// AI(W906-Q30-8) 20260922: 寫一份 ShowErrorMessage 的請求進信箱。
//   欄位逐字對照 web/JSON/Alarm-dialog-request.json（那是最權威的樣本），
//   arguments 五個鍵對上 golden
//   `ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool, AnsiString)`。
//   ⚠ `position` 是 cmydef 的 unit id，不是像素座標。

<!-- preserved-content:end -->
```
