// =============================================================================
//  MyPLC/SafePlcClock_St02.cpp  --  the safety PLC's serve-loop job (golden TPLCIOThread)
//
//  AI(W906-SAFEPLC) 20261006: EastSun 1006「都幫我補上」-- the safety-PLC connection mode (Gerneral.ini [System] SafePlcIO,
//  Enable_PLCSafety_IO; golden Jason 20230619) end to end. Before this, SafePlcIO=1 was not golden end to end
//  (cinitial.cpp GATE (W7a-I3) note): TPLCIOThread::Resume() is a no-op, so nothing pumped PLCIOProcess, and PlcComm
//  stayed a SIM socket.
//
//  GOLDEN: InitPLCIO (MyPLC_IO_Modbus.cpp :100, called from cinitial.cpp when Enable_PLCSafety_IO, IP fixed 172.16.8.120:502)
//  creates TPLCIOThread, whose Execute is `do { Synchronize(PLCIOProcess); MySleepEx(1,true); } while(!Terminated)` --
//  PLCIOProcess = PLCIOTaskCycle (FC4 read of 30001-30022) + PLCStatusCheck (reconnect every 5 s when down, the
//  bSafePLCThread heartbeat) + PlcComm.Cycle, all on the main thread; the socket's events arrive through the same
//  message loop. golden mymessbox / note also run it while a box waits (bPLCStatusCheck, kevin 20250407).
//
//  HERE: one FastClock kDelay job, 1 ms, registered only when SafePlcIO=1 at start (as golden creates the thread only then):
//    (1) PlcComm.W906_Poll() -- the socket events (connect / read / close / error) on this thread, as golden's message loop;
//    (2) MyPLCIOThread->PLCIOProcess() -- the golden body, verbatim, once InitPLCIO has created the thread object.
//  The FastClock also runs inside the blocking boxes (FastClockWbServe.cpp, E-FT1-001), which covers golden's box pump.
//  PlcComm is switched to POLLED-Real here; if InitPLCIO already opened it (SIM), it is re-opened so the switch applies.
//  Its own TU (like PadInterfaceClock_St02.cpp): only FastClockWbServe.cpp calls it, and every target that compiles that
//  file also compiles FastClock.cpp.
//  ⚠ golden ModbusTCPClient::SocketError sleeps 1000 ms (it runs on the main thread in golden too) -- with the PLC
//  unreachable, each 5 s reconnect attempt costs a 1 s stall, as in golden. Kept.
// =============================================================================
#include "MyPLC_IO_Modbus.h"
#include "cmydef.h"                 // Enable_PLCSafety_IO
#include "FastClock.h"
#include <string>
#include <cstdio>

namespace {
void W906_SafePlcTick()
{
    if(!Enable_PLCSafety_IO)
        return;
    PlcComm.W906_Poll();                                                        // golden: the VCL message loop's socket events
    if(MyPLCIOThread!=NULL)
        MyPLCIOThread->PLCIOProcess();                                          // golden TPLCIOThread::Execute: Synchronize(PLCIOProcess)
    static int s_conn=-1;  static bool s_effect=false;                          // port-only: one stdout line per connection change / first data
    const int conn=PlcComm.IsConnected() ? 1 : 0;
    if(conn!=s_conn)
    {
        std::printf("[SAFEPLC] %s 172.16.8.120:502\n", conn ? "connected" : "not connected");
        std::fflush(stdout);
        s_conn=conn;
    }
    if(bPLCIOEffect && !s_effect)
    {
        std::printf("[SAFEPLC] first input registers read (bPLCIOEffect)\n");
        std::fflush(stdout);
        s_effect=true;
    }
}
}  // namespace

void W906_SafePlcFastClockAdd(ht9045::fastclock::FastClock* clock, std::string& jobs)
{
    if(clock==0 || !Enable_PLCSafety_IO)                                        // golden: the thread exists only with SafePlcIO=1
        return;
    PlcComm.W906_UsePolledRealSocket();
    if(MyPLCIOThread!=NULL)                                                     // InitPLCIO ran first and opened it in SIM: re-open, now Real
    {
        PlcComm.SetScan(false);
        PlcComm.SetScan(true);
    }
    if(clock->Add("safeplc", 1, ht9045::fastclock::kDelay, &W906_SafePlcTick)>=0)   // golden MySleepEx(1,true)
        jobs += ", safeplc 1 ms (golden TPLCIOThread::PLCIOProcess, SafePlcIO=1, Modbus TCP 172.16.8.120:502)";
}
