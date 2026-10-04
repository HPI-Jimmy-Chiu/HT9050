// ===========================================================================
//  TesterComm/Handler/TimeDataRecordRefresh.cpp -- AI(W906-ELA-W48B-A3) 20260928 (St02-E, St02-E2 review A3).
//
//  wb_serve only (CMakeLists.txt wb_serve sources, St02's line).  Installs the record-counter refresh that
//  W906_TimeDataHourTickAt (TesterComm/Handler/TesterCommWiring.cpp) calls right before the hourly RecordTimeData:
//  FileRW/MainRecord.cpp W906_MainRecordTimer1Tick = golden Timer1Timer's UpdateRecordScreen (with its InitialOK check
//  and first-tick latch).  Golden's dialogs keep calling fMain->Timer1Timer (note.cpp:3355, mymessbox.cpp:542); V906's
//  W906_ModalWaitTick does too since 0928 (tools/wb_serve.cpp:7630), so this hourly refresh is now a harmless extra.
//  Installed at static init, like WebLogin.cpp's D4 seat, because MainRecord.cpp is linked only into wb_serve.
// ===========================================================================
extern void (*W906_TimeDataRecordRefreshHook)();
void W906_MainRecordTimer1Tick();

namespace {
struct W906TimeDataRecordRefreshInstall
{
    W906TimeDataRecordRefreshInstall() { W906_TimeDataRecordRefreshHook = &W906_MainRecordTimer1Tick; }
};
W906TimeDataRecordRefreshInstall g_w906TimeDataRecordRefreshInstall;
}  // namespace
