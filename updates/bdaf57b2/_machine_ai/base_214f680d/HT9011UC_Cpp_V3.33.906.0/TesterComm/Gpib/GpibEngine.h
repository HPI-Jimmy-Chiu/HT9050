// ===========================================================================
//  TesterComm/Gpib/GpibEngine.h -- runs the translated GPIB bridge (namespace gpibbridge) as a TesterEngine on
//  the one TesterComm thread.
//
//  AI(W906-GB-P1) 20260926: Tester-comm plan P1.  What golden H9046_32GPIB.exe got from its own process, this
//  adapter supplies in-process:
//
//    golden (own process)                              here (TesterComm thread)
//    ------------------------------------------------  --------------------------------------------------------
//    WinMain: CreateForm(SerialPoll) -> OnCreate;       Start(): new TSerialPoll -> FormCreate; new TfDummyART;
//      CreateForm(fDummyART); CreateForm(fRS232Main);     new TfRS232Main; FormShow  (same order)
//      Application->Run -> main form OnShow
//    TMyThread: SleepEx(1) + Synchronize(Process)       RunOnce(): Process (ProcessAddress + ProcessMessage when
//      (Unit2.cpp)                                        InitialOK) while bEnableThread; returns 1 ms
//    TTimer Timer1 (300 ms) / TimerTMode (10 ms) /      RunOnce(): each fires when Enabled and its Interval has
//      fDummyART->DummyARTTimer1                          elapsed; a false->true Enabled edge restarts the period
//                                                         (VCL TTimer semantics)
//    TComm OnReceiveData on the form thread             QueueRx() on the COM reader thread, DrainRx() here
//    FindWindow("TfMain", "HT-9045") -> HMountWnd       HMountWnd = HandlerWndToken() while the mailbox exists
//    SendMessage(HMountWnd, WM_COPYDATA) both ways      SyncMailbox (synchronous, pumps while waiting)
//    Close() -> OnClose (FormClose) -> process exits    RequestClose() flag; FormClose runs at the TOP of the next
//                                                         RunOnce (never inside a nested mailbox call), then the
//                                                         engine goes down (IsUp()==false) until the hub stops it
//
//  Isolation (ruling "不能互相干擾"): every gpibbridge call happens on the TesterComm thread.  The Handler side only
//  sees bytes through the mailbox and the two identity tokens below.
//
//  Driver: an injected IGpibDriver (ctest / SOFT_SIMULTE) wins; otherwise NiGpibDriver if gpib-32.dll loads;
//  otherwise no driver (every ib* call returns ERR and golden logs "Error Open GPIB0" as on a PC without a card).
// ===========================================================================
#ifndef TESTERCOMM_GPIB_GPIBENGINE_H
#define TESTERCOMM_GPIB_GPIBENGINE_H

#include "TesterComm/TesterEngine.h"
#include "TesterComm/Gpib/GpibDriver.h"

#include <atomic>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace gpibbridge {

class GpibEngine : public testercomm::TesterEngine
{
public:
    GpibEngine();
    ~GpibEngine();

    const char* Name() const { return "gpib"; }
    bool Start(testercomm::SyncMailbox* mailbox);
    unsigned RunOnce();
    int OnHandlerMessage(const std::string& payload);
    void Stop();
    bool IsUp() const { return up_.load(); }

    // ctest / SOFT_SIMULTE: use this driver instead of gpib-32.dll (not owned; must outlive the engine).
    // Call before the hub starts the engine.
    static void InjectDriver(IGpibDriver* driver);

    // ctest: point golden's two ini path variables (asGeneralPath / asHGeneralPath) elsewhere.  Applied at every
    // Start() after ResetBridgeGlobals(); empty = golden paths.  Call before the hub starts the engine.  Golden's
    // literal paths (D:\GPIBLOG, D:\RS232Standard\System\Setup.ini, ...) are not affected.
    static void OverrideIniPaths(const std::string& generalIni, const std::string& handlerGeneralIni);

    // Factory for TesterCommHub::RegisterFactory(kTestTypeGpib, ...).
    static testercomm::TesterEngine* Create();

    // Identity tokens for golden's window-handle checks (OnMyCopyMsg compares GHandler2Gpib->HandlerHwnd with
    // HMountWnd and GHandler2Gpib->GpibHwnd with this->Handle).  The Handler side (P2) fills MV.HandlerHwnd /
    // MV.GpibHwnd with these.  NULL while no bridge is running.  Readable from any thread.
    static HWND HandlerWndToken();
    static HWND BridgeWndToken();

    // Diagnostics (any thread).
    static unsigned long PostFailures();    // PostToHandler sends that timed out / had no receiver
    static unsigned long HandlerMessages(); // OnMyCopyMsg calls

    // VCL TTimer emulation (public for ctest).  Due() is true when the timer should fire now: Enabled, and
    // Interval ms elapsed since the last fire or since the false->true Enabled edge.  Interval 0 never fires; missed
    // periods are not caught up (WM_TIMER coalesces).
    struct TimerSlot
    {
        bool wasEnabled;
        unsigned long long lastMs;
        TimerSlot() : wasEnabled(false), lastMs(0) {}
    };
    static bool Due(TimerSlot& slot, bool enabled, unsigned interval, unsigned long long nowMs);

private:
    GpibEngine(const GpibEngine&);
    GpibEngine& operator=(const GpibEngine&);

    void DoClose();              // golden OnClose, once
    void Teardown();             // delete the forms, release the driver

    testercomm::SyncMailbox* mailbox_;
    IGpibDriver* ownedDriver_;   // NiGpibDriver we created (deleted in Teardown)
    std::atomic<bool> up_;
    bool started_;
    bool closed_;                // FormClose done
    int depth_;                  // nesting of OnHandlerMessage (mailbox pump); DoClose only at depth 0
    TimerSlot timer1_, timerTMode_, timerDummyArt_;
    unsigned long long lastUiMs_;   // P7: last UiChannel snapshot (every kUiPeriodMs)
};

}  // namespace gpibbridge

#endif
