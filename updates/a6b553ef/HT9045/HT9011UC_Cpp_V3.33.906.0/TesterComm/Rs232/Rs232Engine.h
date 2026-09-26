// ===========================================================================
//  TesterComm/Rs232/Rs232Engine.h -- runs the translated RS232Standard program (namespace rs232std) as a
//  TesterEngine on the one TesterComm thread.  Serves BOTH TestIF_File.iTestType RS232_MODE (2) and TTL_MODE (0):
//  user ruling 20260926 "DIO 走 RS232Standard" -- golden RS232Standard picks Standard / SLT / TTL itself from
//  Setup.ini [SystemSetup] iTesterMode (MainForm.cpp:420-431).
//
//  AI(W906-GB-P4) 20260926: Tester-comm plan P4.  Same adapter design as gpibbridge::GpibEngine (read
//  TesterComm/Gpib/GpibEngine.h); the differences:
//    golden (own process)                              here (TesterComm thread)
//    ------------------------------------------------  --------------------------------------------------------
//    WinMain: CreateForm(fRS232Main) -> OnCreate;       Start(): new TfRS232Main -> FormCreate -> FormShow
//      CreateForm(fMyPal) (design template);            (fMyPal has no runtime role)
//      Application->Run -> OnShow
//    no worker thread; TTimer Timer1 (300 ms)           RunOnce(): Timer1 by Enabled/Interval; returns 10 ms
//    TComm x3 + uSocketServer events on the form thread QueueRx() on their threads, DrainRx() here
//    Close() -> OnClose -> OnDestroy -> exit            RequestClose flag -> FormClose at the next top-level
//                                                         RunOnce; FormDestroy + delete in Stop()
//    every launch: fresh globals                         ResetRs232Globals() + ++g_rs232Life in Start()
// ===========================================================================
#ifndef TESTERCOMM_RS232_RS232ENGINE_H
#define TESTERCOMM_RS232_RS232ENGINE_H

#include "TesterComm/TesterEngine.h"

#include <atomic>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace rs232std {

class Rs232Engine : public testercomm::TesterEngine
{
public:
    Rs232Engine();
    ~Rs232Engine();

    const char* Name() const { return "rs232std"; }
    bool Start(testercomm::SyncMailbox* mailbox);
    unsigned RunOnce();
    int OnHandlerMessage(const std::string& payload);
    void Stop();
    bool IsUp() const { return up_.load(); }

    // Factory for TesterCommHub::RegisterFactory(kTestTypeRs232 / kTestTypeTtl, ...).
    static testercomm::TesterEngine* Create();

    // ctest: point golden's two ini path variables (IniFileName = Setup.ini, asHGeneralPath = Handler Gerneral.ini)
    // elsewhere.  Applied at every Start() after ResetRs232Globals(); empty = golden paths.  Call before the hub
    // starts the engine.  Golden literal paths (D:\RS232Log ...) are not affected.
    static void OverrideIniPaths(const std::string& setupIni, const std::string& handlerGeneralIni);

    // Identity tokens for golden's window-handle checks (see GpibEngine::HandlerWndToken).  NULL while no program
    // life is running.  Readable from any thread.
    static HWND HandlerWndToken();
    static HWND BridgeWndToken();

    static unsigned long PostFailures();
    static unsigned long HandlerMessages();

    // VCL TTimer emulation (public for ctest); same rules as GpibEngine::Due.
    struct TimerSlot
    {
        bool wasEnabled;
        unsigned long long lastMs;
        TimerSlot() : wasEnabled(false), lastMs(0) {}
    };
    static bool Due(TimerSlot& slot, bool enabled, unsigned interval, unsigned long long nowMs);

private:
    Rs232Engine(const Rs232Engine&);
    Rs232Engine& operator=(const Rs232Engine&);

    void DoClose();
    void Teardown();

    testercomm::SyncMailbox* mailbox_;
    std::atomic<bool> up_;
    bool started_;
    bool closed_;
    int depth_;
    TimerSlot timer1_;
    unsigned long long lastUiMs_;   // P7 snapshot cadence
};

}  // namespace rs232std

#endif
