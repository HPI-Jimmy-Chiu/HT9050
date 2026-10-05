# W10 = B: TCP command server 7016 / 7017 opened, 100-byte receive overflow fixed — the plan

> **Status 20260927 late (St02-E): DONE on `v906/steven-w10-wip` (D:\AI_TempFile\st02-w10), compiled both configs, NOT run
> (St01 runs the ctest); not merged.**  What was built and every deviation / 上機要看: `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\
> TESTERCOMM_PORT_LEDGER.md` "W10＝B"; the protocol + V906 shape: `D:\HT9045\.claude\skills\hpi-gpib\references\ht9045-gpib-bridge\references\tcp-command-server-7016.md`.
> Differences from the plan below: item 4 (">99 -> drop") is REPLACED by R2 (St02-M 20260927: the RS232-style framer,
> TesterComm/Tcp/TcpCmdFramer.*, 2048-byte cap, no NG; 322 / 323 guard only the write, golden reply);
> item 2's ctor creation is one call on fMain.cpp:235; item 3 also rewrote the stale B7 banner; S-a opened 16 of the 18 S2
> writes -- HTSET,354's two need fContact (A4) and S3 (701) needs fSCKART->iCurrentStatus / iLOTSTATUS_A (A11, a facade
> change outside the plan: reported, not done); the 702 catch sits at the pump boundary as TODO(W10-702) (no reply, one
> #Exception# line); census `--check 34 31 3` (the tool counts W906_RemoteRunStart).

> Status 20260927: **GO; all of (1)-(9) below are St02's** (the laptop is busy with the out-arm chain). Conditions:
> (a) HTSET,333 / 334 ONLY via StartFromWeb / PauseFromWeb, on the tick thread from the Poll in the tick; never base
>     TfMain::Start / Pause.
> (b) The census numbers come from tools\start_sites_census.py, not by hand.
> (c) Post the WebStart.cpp / Command.cpp diff line numbers in FROM_STEVEN §3 before it goes to main.
> **Steven's rulings 20260927 (via St02-M):** R1 = fix (send the reply with its real length, no cBuffer[500] copy; recorded
> as a deviation). R3 = yes (poll the pump inside the three dialog waits, wb_serve :537 / :806 / :6759). S-a = yes (the 18 S2
> config / recipe writes + S3 (701) as golden). **R2: do NOT answer NG** -- Steven: 「在 rs232 的 bcb 程式裡面，有做把命令接起來的
> 動作，你要參考處理」: study (read-only) how the golden BCB RS232 program (D:\RS232Standard, the version the rs232 skills use;
> cite the tree name) joins partial commands, propose the same for the 7016 receive path (likely replaces ">99 bytes -> drop":
> accumulate, then split into complete commands) and say what HTSET,322 / 323 do with an out-of-range index without NG; send
> the proposal before coding. Still open: the HTSET,702 StrToInt catch.
> Paths below are relative to `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\`; golden = 906_0625_Steven unless marked.

---

W10 = B survey: TCP command server 7016 / 7017. Read-only; golden refs = 906_0625_Steven unless marked. No edits.

HEADLINE: W10=B is bigger than "open + fix the 100-byte overflow". It needs three of your rulings (R1-R3) and one of Steven's (S-a). Everything below is in jimmychiu's files.

What you listed, re-verified:
- Command.cpp :13510 / :13524: two #if 0 in HanderTcpIp (from :13504).
- :13553-13618: HandlerTCPIPResultSendProcess, gated whole.
- :16391 `char EthernetBuffer[100];`, :16401 `ReceiveBuf(EthernetBuffer, iBufferLenght)`.
- WebStart.cpp:3473 calls HanderTcpIp().
- fMain.h has no TServerSocket (only a comment at :1006).

Gated sites your list missed:
- Command.cpp :13476 (Close), :13494 (Memo2), :13626-13660 (HandlerTeraTResultSendProcess, the 7017 push);
- WebStart.cpp:3482-3488 (the else branch that closes both servers);
- atester_ProcessCount.cpp:2549-2576 GATE J (the HTSR,501 test-result push; it also needs fObserver->TimeInfoGrid, which is absent).

Golden:
- main.h:121-122 declares TeraTCPResultServer / TCPCommandServer (dfm: stNonBlocking, Port 0, Active False). Only TCPCommandServer has OnClientRead.
- HanderTcpIp (Command.cpp:12623-12655) hard-codes 7016 / 7017 (no ini keys) and does Close / Port / Open; on failure: "Socket Server Open Error!!".
- Callers:
  - FormShow main.cpp:10974 at BOOT (not ported in V906; V906 only reaches it through START);
  - Start() :6036-6041;
  - :6042-6046 closes both servers when the flag is off.
- The only switch is CosFunction.bEnableHandlerResultServer: Greatek 956 / TeraPower 967 / TeraProbe 804.
- The receive overflow: ReceiveLength() is every waiting byte, and ReceiveBuf copies ALL of it into char[100].
  - ≥101 bytes: a stack overwrite; any LAN host can crash the handler with one send.
  - exactly 100: no NUL.
  - V906's vclcompat queue is unbounded, so the overflow is just as reachable in V906.
- 98 active commands (HTSET,502 is inside /* */). Stateful ones:
  - 310 / 311: AccessLevel Supervisor / Operator with NO password;
  - 318 / 711: SetRunStartMode, no SystemStart check;
  - 333 Start (TERAPOWER or bRemoteLotStart, HALT, !SystemStart) / 334 Pause (TERAPOWER, SystemStart); the reply is OK even when Start refuses;
  - 403: a blocking ShowMyMessage dialog;
  - 404: open / close sites;
  - 700 / 702: Clarn_Data(1) (702 also ART lot start + StrToInt; it replies twice when iTesterType==1);
  - 720 (Greatek): SetLotID + SetLotStart. LIVE in V906 and writes config.ini [Lot Info];
  - 721: SetLotEnd is gated but the reply is still OK;
  - 812: sets bAMRRequestSupplyTray=true and answers OK;
  - 18 S2 config / recipe writes (314-316, 331 / 332, 350 / 354, 461-470, 710-713) + S3 WriteLastDataFile (701).
- Reply: ASCII `HTSR,<id>,<fields>,` with no CR/LF, sent to EVERY 7016 client (up to 3 retries).

New findings:
- (a) B7 is fixed in 0625 and 912 (Command.cpp:12626 / 912 :12634 `Active=false`). Only V899 had it, so the B7 banner (V906 :13199) and GOLDEN_DEFECT_LEDGER row 8 are stale.
- (b) A SECOND overflow on the reply side: `char cBuffer[500]; strcpy(cBuffer, sMessage.c_str())` (V906 :13557/:13559, :13630/:13632). HTGR,255 / 256 with 33+ bins is already about 504 bytes. W10=B doesn't cover it.
- (c) A network-controlled OOB write: HTSET,322 / 323 `BinSelect[..].iDBContact[atoi(sData[2])]`, no bound (golden too).
- (d) HTSET,702 StrToInt throws std::runtime_error in vclcompat, which would unwind the tick thread (VCL caught it in the message loop). The pump must catch.
- (e) vclcompat ServerSocket Real mode: accept + per-connection reader threads fire OnClientRead on THOSE threads and edit Connections[] under a private lock, while the golden send loop walks Connections[] on the tick thread unlocked. It's a real race; the header says it's unresolved.
- (f) ServerSocket binds INADDR_ANY with SO_REUSEADDR, so a second process (the BCB6 exe, or another wb_serve) can silently share 7016.
- (g) No code anywhere calls SetSimMode(false); every V906 socket is Sim today.
- (h) asTCPIPPath = fixed "D:\HT9045_Log\TCPIP_Log" (common.cpp:295), with no ctest seam.
- (i) START_SitesCensus (tests/CMakeLists.txt:4527 `--check 34 30 4`) counts the gated fMain->Start at :17109. Un-gating changes it to 34 31 3.

Proposed plan (needs claims; all jimmychiu's, except vclcompat also jimmychiu, c5379a06):
1. vclcompat ServerSocket / ClientSocket, additive and off by default (the 7 Sim users unchanged):
   - SetPolled(bool) + Poll(): non-blocking accept / recv / send, with events fired on the CALLING (tick) thread, like VCL stNonBlocking. No new thread, and it removes race (e);
   - SetBindAddress, BoundPort(), SetExclusiveAddr (SO_EXCLUSIVEADDRUSE), a 64 KiB per-connection queue cap, SimFailNextOpen;
   - extend test_serversocket.
2. forms/fMain.h ~:1004: add the two TServerSocket* members (golden main.h:121-122). The fMain.cpp ctor creates them in Sim with the dfm event bindings.
3. Command.cpp: lift :13476, :13494 (Memo2 may stay gated), :13510 (use the 0625 / 912 `Active=false`), :13524, :13553, :13626; after Open() add `if(!…->Active) throw 0;` so a port conflict reaches golden's catch.
4. The overflow (deviation 1, W10=B), at :16390-16402:
   - if ReceiveLength > 99, drain everything, log "#Overflow# N bytes dropped (limit 99)", and treat it as an unknown command (golden's tail sends 0 bytes). The connection stays open. 100 bytes is also dropped.
   - 99 = golden's own safe maximum. (Option: HTSET,403's `<1023` check hints at ~1 KiB messages, but that's more than golden can safely take.)
5. Start / Pause seam: `W906_RemoteRunTable{Start,Pause}` in fMain.h/.cpp, zero by default.
   - Do NOT forward the base TfMain::Start / Pause (that would arm about 30 Start and 40 Pause sites).
   - wb_serve installs it after :3794: Start → g_webMain->StartFromWeb, Pause → PauseFromWeb (tick thread, same as start.run), refused during manual teach (W906_MotorAccessStartBlocked). Not installed → NG.
   - Replace :17108-17110 / :17131-17133.
6. WebStart.cpp:3482-3488: lift once the members exist.
7. common.cpp:295: asTCPIPPath = as9045LogPath+"\\TCPIP_Log" (the :244 pattern, so ctest goes to scratch via D5).
8. A new TesterComm/Tcp/CmdServerPump.cpp/.h (St02) in ht9045_testercomm_handler:
   - Init: polled + real + exclusive on the two fMain servers, then golden FormShow :10974 `if(bEnableHandlerResultServer) HanderTcpIp();`;
   - Tick: Poll() both in try/catch(...) (the VCL HandleException equivalent);
   - hooked at TesterCommWiring.cpp:112 / :127 (no new wb_serve hook); HT9045_TESTERCOMM=0 keeps everything Sim.
9. The census tool + tests/CMakeLists.txt:4527 → `--check 34 31 3`; WebStart.h:36/:42 notes.
10. Docs: DEVLOG / KNOWLEDGE / TESTERCOMM ledger / GOLDEN_DEFECT row 8.
GATE J (the 501 push) stays gated (TimeInfoGrid is missing): 7017 will listen but not push yet.

Needs rulings:
- R1: deviation 2, the cBuffer[500] reply overflow → send sMessage with its real length (no copy)?  **Ruled: fix (deviation).**
- R2: deviation 3, HTSET,322 / 323 bounds check → NG?  **Ruled: no NG; follow the RS232 BCB command joining (proposal first).**
- R3: poll the pump inside the three dialog wait loops (wb_serve :537 / :806 / :6759)? Golden VCL keeps answering during ShowModal, and HTGR,801 / 706 must answer "Down" during an alarm (the AMR depends on it). I recommend yes.  **Ruled: yes.**
- S-a (Steven): enable the 18 S2 config / recipe writes + S3 (701)? W10=B doesn't say. All go through AuthPath / DataPath / W906_LastDataPath, so ctest is safe.  **Ruled: yes, as golden.**

ctest (compile here, run by St01): test_tcp_cmd_server, full RESCAN + testercomm_handler + ws2_32.
- Guard: refuse unless asTCPIPPath / AuthPath / DataPath are all in scratch. The fMain servers stay Sim, and the test fails if any real socket is on 7016 / 7017.
- Cases:
  1. HanderTcpIp in Sim;
  2. a table of all 98 command replies (quirks included: HTSET,317 reply prefix, 701 → `HTSR,701,,`, 519 / 520 NG, 702 twice);
  3. broadcast to 2 connections;
  4. overflow: 99 parsed; 100 / 4096 / 65536 dropped; the next command is still answered;
  5. the Start / Pause stub: exact strings, NG paths, and the base Pause counter unchanged;
  6. HTSET,702,abc does not crash;
  7. file writes only in scratch;
  8. (if R1) the 255-bin reply is sent in full;
  9. loopback, the FIRST real-mode socket test in the tree: a test-owned polled server on 127.0.0.1:0 (never 7016 / 7017), with a Winsock client, callbacks on the test thread, and an exclusive-bind conflict;
  10. the census passes with the new numbers.

Safety: once open, any LAN host (only on CC 956 / 967 / 804; the dev box is 868, so it stays closed) can:
- start / pause the machine;
- start a lot with any LotID;
- get OK for a lot end that never happens;
- get Supervisor with no password;
- clear counters and write lastdata;
- switch run mode while running;
- pop a blocking dialog;
- tell the AMR it may load;
- open / close sites.
Replies go to all clients. When a 7017 client disconnects, the next START re-runs HanderTcpIp and drops every 7016 connection (golden).

上機要看 (rule 4):
1. netstat shows 7016 LISTENING only by wb_serve and only on 956 / 967 / 804.
2. The "[7016]/[7017] Server Listen" log lines.
3. HTGR,109 / 205 / 801 / 802 are byte-equal to BCB6 in the same state.
4. A 200-byte send logs #Overflow#, the process stays up, and the next command is answered.
5. HTSET,333 at HALT WITH THE E-STOP IN HAND behaves like web START; 334 pauses; 333 during teach → NG.
6. With BCB6 or a second wb_serve on the port → "Socket Server Open Error!!".
7. Firewall 7016 / 7017 to the MES / AMR hosts.
8. HTSET,812 / 811 with the real AMR.
9. Back up config.ini / lastdata before 720 / 700 / 702 (tools/realfile_guard.py).
10. TCPIP_Log gets one file per hour.

Owners / claims:
- TO_STEVEN §1 has no live claim on these files, but S-04 says claim first in FROM_STEVEN before touching Command.cpp or wb_serve.cpp. The laptop's hw.access row owns "the one wb_serve command-dispatch line", which is a different line from ours; tell them.
- St01 (855a6a83) doesn't touch Command.cpp / WebStart / vclcompat / common / TesterComm / atester_ProcessCount / the census tool. Its fMain.h/.cpp edits are on other lines. Both sides append at the end of CMakeLists / tests/CMakeLists.
My suggestion: it's big, and most of it is Jimmy's files. Maybe split it: St02 does 1 + 4 + 8 + the ctest after the claims; Jimmy / the laptop do 2 / 3 / 5 / 6. Your call.
