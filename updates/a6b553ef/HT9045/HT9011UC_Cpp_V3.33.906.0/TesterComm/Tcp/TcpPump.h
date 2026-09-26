// ===========================================================================
//  TesterComm/Tcp/TcpPump.h -- plan P5: drives the Handler's built-in TCP/IP tester channel (golden TfTesterTCP,
//  already translated as Interface/TesterTCP_Socket.*) the way golden's form did.
//
//  AI(W906-GB-P5) 20260926.  TCP/IP differs from GPIB / RS232: golden has NO bridge program for it.  TfTesterTCP is
//  a Handler form; its socket events and its two timers (TimerTCPIPConnect, TimerProcessTCPData Interval=1) all run
//  on the Handler's main thread, and TimerProcessTCPDataTimer writes the Handler's test results directly.  So the
//  faithful home is the Handler (tick) thread, not the TesterComm thread:
//
//    golden                                               here
//    ---------------------------------------------------  ------------------------------------------------------
//    ClientSocket_TCPIP OnConnect/OnDisconnect/OnError/   vclcompat TClientSocket fires them on ITS reader thread;
//      OnRead on the main thread                            W906_TcpPumpInit() re-routes the four events into a
//                                                           queue, W906_TcpPumpTick() replays the golden handlers
//                                                           (TesterTCPSocket_On*) on the Handler thread
//    TimerTCPIPConnect / TimerProcessTCPData Enabled when  every tick: the same condition golden evaluates in
//      TestIF_File.iTestType==TCP_IP_MODE && ON_LINE        TfMain::FormShow (main.cpp:11320-11325) and when the mode
//      (main.cpp:11320, cTesterIF.cpp:581-618,              or ON/OFF-LINE changes (cTesterIF.cpp:581-618,
//      main.cpp:29770-29781)                                main.cpp:29770-29781); on a true->false edge the socket
//                                                           is closed as golden does
//    TTimer firing                                        Enabled + Interval (TimerTCPIPConnect: VCL default 1000;
//                                                           TimerProcessTCPData: 1 -> every tick pass)
//
//  ⚠ KNOWN LIMIT (P8): vclcompat TClientSocket in REAL mode connects synchronously (golden's VCL socket was
//  non-blocking).  TimerTCPIPConnectTimer's Open() would then block the Handler thread while the tester is
//  unreachable -- ruling 11 ("不能互相干擾").  The socket is in SIM mode today (nothing calls SetSimMode(false)),
//  so this cannot happen yet; switching to real mode at bring-up must move that Open() off the Handler thread
//  (ledger P5).
//
//  Threads: Init / Tick on the Handler thread only.  The queue callbacks run on the shim's thread.
// ===========================================================================
#ifndef TESTERCOMM_TCP_TCPPUMP_H
#define TESTERCOMM_TCP_TCPPUMP_H

void W906_TcpPumpInit();
void W906_TcpPumpTick();
unsigned long W906_TcpPumpEvents();   // replayed socket events (diagnostics / ctest)

#endif
