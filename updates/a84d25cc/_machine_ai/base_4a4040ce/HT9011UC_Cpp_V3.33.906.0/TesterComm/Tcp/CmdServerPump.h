// ===========================================================================
//  TesterComm/Tcp/CmdServerPump.h -- W10 = B: drives the Handler's TCP command server 7016 and result server 7017
//  (golden TfMain::TCPCommandServer / TeraTCPResultServer) the way golden's VCL message loop did.
//  AI(W906-W10) 20260927 (St02-E).  Plan (folder D:\HT9045\.claude\skills\ht9045-st02-workflow\references):
//  w10-tcp-command-server-plan.md (items 1-9, Steven's rulings R1 / R3 / S-a / R2 of 20260927).
//
//  golden (906_0625_Steven)                                here
//  ------------------------------------------------------  ---------------------------------------------------------
//  main.dfm:17358-17377 two TServerSocket, stNonBlocking,  forms/fMain.cpp W906_TcpServersCreate: the two objects in
//    events on the VCL main thread                            Sim with the dfm bindings; Init switches them to POLLED
//                                                              real sockets (vclcompat ServerSocket.cpp end): every
//                                                              event fires inside Poll(), on the tick thread
//  FormShow main.cpp:10974-10977 `if(bEnableHandler...)    the first pump pass (the tick thread's first tick)
//    HanderTcpIp();` (and Start() :6036-6041 again)           (Start keeps its own call, WebStart.cpp:3473)
//  OnClientRead -> TCPCommandServerClientRead reads ALL     OnClientRead -> the framer (TcpCmdFramer.h, R2): bytes are
//    waiting bytes into char[100] = one command              appended per connection; every pass hands complete
//                                                              commands one by one to the unchanged golden body
//                                                              (W906_TcpCmdRunFrame, Command.cpp end), <= 10 each
//  ShowModal keeps the message loop running                W906_CmdServerPumpPoll() inside the three modal waits
//                                                              (wb_serve :537 / :806 / :6759 -> W906_TesterCommPoll),
//                                                              R3 = yes (HTGR,801 / 706 must answer during an alarm)
//  VCL Application->HandleException catches what a        each command runs inside try / catch here: an exception
//    handler throws (HTSET,702 StrToInt("abc"))               (vclcompat StrToInt throws std::runtime_error) is logged
//                                                              as #Exception# and never unwinds the tick thread
//                                                              (plan finding (d); W41 = A, Steven 0928: the handling is
//                                                              as golden -- golden showed a VCL error box and sent no reply;
//                                                              here: no reply, one log line)
//  HTSET,333 / 334 -> fMain->Start / fMain->Pause           W906_RemoteRun (forms/fMain.h end), installed by wb_serve:
//                                                              StartFromWeb / PauseFromWeb, called from the command,
//                                                              i.e. on the tick thread (condition (a))
//
//  Threads: Init / Tick / Poll / Shutdown on the Handler (tick) thread.  The first Tick records that thread; Poll from
//  any other thread does nothing (a ShowMyMessage wait on an HTTP thread must not run commands there).
// ===========================================================================
#ifndef TESTERCOMM_TCP_CMDSERVERPUMP_H
#define TESTERCOMM_TCP_CMDSERVERPUMP_H

// bRealSockets=false keeps both servers in Sim (the ctest, HT9045_TCPCMD_SIM=1): the pump, the framer and the golden
// body run exactly the same, only no OS socket is opened.
void W906_CmdServerPumpInit(bool bRealSockets);
void W906_CmdServerPumpTick();       // every tick (W906_TesterCommTick)
void W906_CmdServerPumpPoll();       // inside the modal waits (W906_TesterCommPoll); tick thread only
void W906_CmdServerPumpShutdown();
// AI(W906-W10fix) 20260928 (St02-E): creates fMain's two servers (Sim, golden's dfm bindings: W906_TcpServersCreate) when
// neither exists yet; a no-op once they do.  The ctor's own call (forms/fMain.cpp:235) sits behind that line's // comment
// and never runs until the line is claimed; W906_TesterCommInit and W906_CmdServerPumpInit call this first.
void W906_CmdServersEnsure();

struct W906CmdServerPumpStats
{
    unsigned long passes;       // pump passes that ran
    unsigned long frames;       // commands handed to the golden body
    unsigned long discards;     // #Discard# events (noise before a head)
    unsigned long incompletes;  // #Incomplete# events
    unsigned long overflows;    // #Overflow# events
    unsigned long caught;       // exceptions caught at the pump boundary
};
W906CmdServerPumpStats W906_CmdServerPumpGetStats();

#endif
