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
//  golden TFormHS::TimerAutoBackupTimer (1000 ms)       W906_TesterCommTick() / Poll() also run the hourly TimeData
//    :233-238 CheckClockTrigger(60) -> RecordTimeData      record (W906_TimeDataHourTick, AI(W906-ELA-W48B), below)
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

// ---- AI(W906-ELA-W48B) 20260928 (St02-E helper): ★W48-2 = B (Steven 0928 09:3x) -- the hourly TimeData record ----
// golden HS_Function.cpp:233-238 in TFormHS::TimerAutoBackupTimer (a 1000 ms VCL timer on the main thread, enabled in
// the TFormHS ctor :56; guard :106-111): at the top of every hour RecordTimeData(2) (iN10UploadProductMethod 1) or
// RecordTimeData(3) (906_0625_Steven cMyDB.cpp:339-498, V906 cMyDB.cpp:479: one TimeData row, file
// D:\HT9045_Log\TimeData\<yyyy>\TimeData_<yyyy>.csv, with the hour's PowerOn / StartTime / ... seconds, a HANDLER LOG
// row, the per-hour counters reset).
// V906 has no TimerAutoBackupTimer (forms/fHS.h: not ported; TFormHS cannot even be obtained), so the call sits in W906_TesterCommTick / W906_TesterCommPoll: the same thread as the counters it reads and resets
// (FileRW/MainRecord.cpp UpdateRecordScreen, wb_serve's main loop, golden Timer1 -- also a main-thread VCL timer), and
// Poll keeps it running inside the modal waits as the VCL timer did during ShowModal.  It runs before their
// HT9045_TESTERCOMM=0 opt-out check (TimeData is not tester communication).
// ⚠ One owner: if TimerAutoBackupTimer is ever ported, its :233-238 block must not be (or this tick must go) --
//   two callers = two rows an hour, the second with near-zero counters.
//   W906_TimeDataHourEdge   golden TFormHS::CheckClockTrigger(60), HS_Function.cpp:4312-4328, on *pbPerHours (golden:
//                           a function-local static shared by nobody else; here the caller's) -- true once per hour,
//                           when hh:mm:ss first reaches mm:ss >= 00:04 after having been in 00:00 .. 00:02
//   W906_TimeDataHourTickAt the timer body at the time iNowTime (hhmmss as an int, golden
//                           StrToInt(GetOnlyTimeInfoByString())) -- for the ctest (ELA_TimeData)
//   W906_TimeDataHourTick   W906_TimeDataHourTickAt(now)
bool W906_TimeDataHourEdge(int iNowTime, bool* pbPerHours);
void W906_TimeDataHourTickAt(int iNowTime);
void W906_TimeDataHourTick();

#endif
