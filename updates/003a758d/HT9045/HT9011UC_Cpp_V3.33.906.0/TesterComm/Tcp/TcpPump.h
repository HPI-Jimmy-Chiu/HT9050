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
//    ClientSocket_TCPIP OnConnect/OnDisconnect/OnError/   vclcompat TClientSocket in POLLED mode (H-008) fires them
//      OnRead on the main thread (ctNonBlocking:            from Poll() on this thread; W906_TcpPumpInit() still
//      golden 906_0625_Steven TesterTCP.dfm:2936-2943)      re-routes the four events into a queue, and
//                                                           W906_TcpPumpTick() replays the golden handlers
//                                                           (TesterTCPSocket_On*) on the Handler thread
//    TimerTCPIPConnect / TimerProcessTCPData Enabled when  every tick: the same condition golden evaluates in
//      TestIF_File.iTestType==TCP_IP_MODE && ON_LINE        TfMain::FormShow (906_0625_Steven main.cpp:10879-10884) and
//      (main.cpp:10879-10884, cTesterIF.cpp:580-617,        when the mode or ON/OFF-LINE changes (cTesterIF.cpp:580-617,
//      main.cpp:28804-28819)                                main.cpp:28804-28819); on a true->false edge the socket
//                                                           is closed as golden does
//    TTimer firing                                        Enabled + Interval (TimerTCPIPConnect: VCL default 1000;
//                                                           TimerProcessTCPData: 1 -> every tick pass)
//
//  AI(W906-H008) 20261001 (St02-E): REAL socket in the SHIP build (H-008; RULINGS_20261001 #0).  W906_TcpPumpInit(true)
//  puts the socket in vclcompat's POLLED mode (SetPolled + SetSimMode(false)): TimerTCPIPConnectTimer's Open()
//  starts a non-blocking connect and returns at once, and Poll() at the top of every tick finishes it -- the
//  Handler thread never waits for the tester (ruling 10 = A's aim; ruling 11 "不能互相干擾").  The SIM build stays
//  SIM, and HT9045_TCPCMD_SIM=1 keeps SHIP SIM too (W906_TcpTesterRealSocket).  SHIP + TCP_IP_MODE + ON-LINE really
//  dials TesterIF [Tester TCPIP] Address:Port (golden default 172.16.8.150:6000, cTesterIF.cpp:778-779).
//  Known gap: inside a modal wait (ShowMyMessage -> MbWait) this pump does not run; golden's ShowModal kept
//  processing socket messages.
//
//  Threads: Init / Tick on the Handler thread only.  The queue callbacks run on the shim's thread.
// ===========================================================================
#ifndef TESTERCOMM_TCP_TCPPUMP_H
#define TESTERCOMM_TCP_TCPPUMP_H

void W906_TcpPumpInit(bool bRealSocket = false);   // AI(W906-H008) 20261001 (St02-E): true = REAL socket, POLLED (H-008)
bool W906_TcpTesterRealSocket();                   // AI(W906-H008) 20261001 (St02-E): SIM build false; SHIP true unless HT9045_TCPCMD_SIM=1
void W906_TcpPumpTick();
unsigned long W906_TcpPumpEvents();   // replayed socket events (diagnostics / ctest)

#endif
