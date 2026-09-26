// ===========================================================================
//  TesterComm/TesterEngine.h -- the interface every tester-communication engine implements.
//
//  AI(W906-GB-P0) 20260926: Tester-comm plan P0.  Engines planned (later phases): GpibEngine (H9046_32GPIB
//  TSerialPoll, P1), Rs232Engine incl. the TTL board = DIO (RS232Standard TfRS232Main, P4), TcpEngine
//  (Interface/TesterTCP_Socket pump, P5).  User ruling 20260926: the three share ONE TesterComm thread because a
//  machine uses only one of them in production; TesterCommHub starts exactly one engine per TestIF_File.iTestType.
//
//  Threading contract (ruling "不能互相干擾"):
//    * Start(), RunOnce(), OnHandlerMessage() and Stop() are called ONLY on the TesterComm thread.
//    * An engine must not touch machine objects (IO, motors, fMain, MainProc, tags); it talks to the Handler
//      only through the SyncMailbox it is given in Start().
//    * RunOnce() may block (golden MyGPIBWrite waits up to 2 s) -- that only delays this thread.
// ===========================================================================
#ifndef TESTERCOMM_TESTERENGINE_H
#define TESTERCOMM_TESTERENGINE_H

#include "TesterComm/SyncMailbox.h"

#include <string>

namespace testercomm {

// Mirrors golden cmydef.h TTL_MODE / GPIB_MODE / RS232_MODE / TCP_IP_MODE (TestIF_File.iTestType).
enum TesterTestType
{
    kTestTypeTtl   = 0,   // DIO: in V906 this is the RS232Standard TTL board mode (ruling 20260926)
    kTestTypeGpib  = 1,
    kTestTypeRs232 = 2,
    kTestTypeTcpIp = 3
};

class TesterEngine
{
public:
    virtual ~TesterEngine() {}

    virtual const char* Name() const = 0;

    // Called once on the TesterComm thread before the first RunOnce().  Return false to refuse to run.
    virtual bool Start(SyncMailbox* mailbox) = 0;

    // One iteration of the engine's own loop (golden: GPIB TMyThread 1 ms loop, RS232 Timer1 300 ms).
    // Returns how long the thread may sleep before the next call, in ms (a mailbox arrival wakes it earlier).
    virtual unsigned RunOnce() = 0;

    // A request from the Handler side (golden: the bridge's OnMyCopyMsg).  Runs on the TesterComm thread.
    virtual int OnHandlerMessage(const std::string& payload) = 0;

    // Called once on the TesterComm thread after the last RunOnce().
    virtual void Stop() = 0;

    // Link state (golden bFind / bConnectOK).  Read from any thread, so implementations keep it atomic.
    virtual bool IsUp() const = 0;
};

// Factory registered per test type.  Returns a new engine or NULL.
typedef TesterEngine* (*TesterEngineFactory)();

}  // namespace testercomm

#endif
