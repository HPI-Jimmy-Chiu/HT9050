// ===========================================================================
//  TesterComm/Handler/TesterCommWiring.h -- the few calls wb_serve (the composition root) makes into tester
//  communication.  AI(W906-GB-P3) 20260926: Tester-comm plan P3.  Everything else stays inside TesterComm/.
//
//  golden (TfMain, VCL main thread)                        here (wb_serve)
//  ------------------------------------------------------  ---------------------------------------------------------
//  TfMain ctor / FormShow: bridge not running yet          W906_TesterCommInit()   once, after the machine is ready
//  WM_COPYDATA from the bridge, handled by the message     W906_TesterCommTick()   every tick-loop pass (<= 50 ms):
//    loop as soon as it arrives                              PollHandler() -- bridge -> Handler packets
//  Timer2 (1000 ms) -> ProcessHVisionConnect -> WakeupGPIB   + fTesterSide->ProcessHVisionConnect() every 1000 ms
//  TfTesterTCP socket events + its two timers (TCP/IP)    ... W906_TesterCommTick() also runs the P5 TCP pump
//  ShowModal keeps pumping WM_COPYDATA                     W906_TesterCommPoll()   inside each modal wait loop
//  program exit (bridge exe outlives / is closed)          W906_TesterCommShutdown()
//  the bridge's own form on screen                         W906_TesterCommHttp()   GET/POST /api/testercomm/...
//
//  Threads: Init / Tick / Poll / Shutdown on the Handler (tick) thread only.  Http on any thread (it only touches
//  testercomm::UiChannel, which has its own lock).
// ===========================================================================
#ifndef TESTERCOMM_HANDLER_TESTERCOMMWIRING_H
#define TESTERCOMM_HANDLER_TESTERCOMMWIRING_H

#include <string>

void W906_TesterCommInit();
void W906_TesterCommTick();
void W906_TesterCommPoll();
void W906_TesterCommShutdown();

// HTTP (wb_serve ApiRoute adapter copies status / contentType / body into its HttpResponse):
//   GET  /api/testercomm                 {"keys":["gpib","rs232"]}
//   GET  /api/testercomm/<key>           the engine's latest UiChannel snapshot (404 before the first one)
//   POST /api/testercomm/<key>?cmd=...   one page command (URL-encoded); 403 when commands are not allowed
// Returns false when `path` is not under /api/testercomm (the caller then tries its other routes).
bool W906_TesterCommHttp(const std::string& method, const std::string& path, const std::string& query,
                         bool allowCmd, int* status, std::string* contentType, std::string* body);

#endif
