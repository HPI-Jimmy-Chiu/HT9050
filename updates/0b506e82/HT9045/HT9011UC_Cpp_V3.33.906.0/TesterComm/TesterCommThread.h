// ===========================================================================
//  TesterComm/TesterCommThread.h -- the single TesterComm thread (user ruling 20260926: GPIB／RS232／TCPIP share
//  one thread; it must be separate from the machine thread and must not interfere with it).
//  AI(W906-GB-P0) 20260926.
//
//  Loop (golden references: H9046_32GPIB Unit2.cpp TMyThread::Execute -- SleepEx(1) + ProcessAddress/ProcessMessage;
//  RS232Standard Timer1 300 ms):
//      engine->Start(mailbox)
//      while (!stop) { heartbeat++; mailbox->Poll(engine side); ms = engine->RunOnce(); wait(mailbox wake, ms) }
//      engine->Stop()
//  Exceptions thrown by the engine are caught and counted so a faulty engine cannot take the process down;
//  the thread keeps running (the machine thread is never affected either way).
// ===========================================================================
#ifndef TESTERCOMM_TESTERCOMMTHREAD_H
#define TESTERCOMM_TESTERCOMMTHREAD_H

#include "TesterComm/TesterEngine.h"

#include <atomic>

namespace testercomm {

class TesterCommThread
{
public:
    TesterCommThread();
    ~TesterCommThread();

    // Starts the thread running `engine` (not owned).  Returns false if already running or the OS refused.
    bool Start(TesterEngine* engine, SyncMailbox* mailbox);

    // Signals the loop to finish, waits for engine->Stop() and joins.  Safe to call when not running.
    void Stop();

    bool Running() const { return running_.load(); }
    bool StartFailed() const { return startFailed_.load(); }
    unsigned long Heartbeat() const { return heartbeat_.load(); }   // loop iterations (watchdog breadcrumb)
    unsigned long EngineErrors() const { return errors_.load(); }   // exceptions caught from the engine

private:
    TesterCommThread(const TesterCommThread&);
    TesterCommThread& operator=(const TesterCommThread&);

    static void Entry(void* self);
    void Loop();

    webbridge::WbThread thread_;
    TesterEngine* engine_;
    SyncMailbox* mailbox_;
    std::atomic<bool> stop_;
    std::atomic<bool> running_;
    std::atomic<bool> startFailed_;
    std::atomic<unsigned long> heartbeat_;
    std::atomic<unsigned long> errors_;
};

}  // namespace testercomm

#endif
