// ===========================================================================
//  MainTimersSt02.cpp -- the one PumpTick entry for the golden TfMain TTimers St02 ports.
//  AI(W906-S15) 20261001 (St02-E).  Called from WebBridgeTags.cpp PumpTick (:605, the line the laptop reserved for
//  St02; PumpTick runs about every 500 ms).  Each golden TTimer fires at its own golden Interval, the VCL way:
//  the first call comes one Interval after the start, and a late tick does not catch up (one call, not several).
//  Deadline style (St02-E2 ST02_S15_CLAIMS correction 1, as St01's A01): the next deadline is the last one + Interval,
//  so a ~500 ms beat with jitter still averages 1000 ms (TemperatureStorageLog counts one call as one second).
//    Timer8                      main.dfm:17346, no Interval stored = 1000 ms      -> MainTimer8.cpp (S-15)
//    TimerTemperatureStorageMinute main.dfm:17318, no Interval stored = 1000 ms    -> MainTimer8.cpp (S-15)
//    TimerESD                    main.dfm:17313, no Interval stored = 1000 ms      -> MainTimerESD.cpp (S-13)
//    Timer1, only its :3439-3440 NUMBER_PANEL_TYPE==2 DoShowBinDigital (E-T1-022)  -> Timer1BinTick below
//      AI(W906-ET1022) 20261001 (St02-E), OPTION A (the laptop 20261001 20:0x: option B -- a 30 ms call on wb_serve.cpp:4575
//      -- is held for Jimmy, NIGHT_REPORT s0 #35): golden Timer1 is 30 ms (main.dfm:17272-17278); here it runs on every
//      dispatcher pass with golden's 30 ms deadline and guards, i.e. about once per 500 ms PumpTick pass (<= 100 ms passes
//      inside waiting boxes).  The type-2 pulse machine (cShowBinSelect.cpp:2729-2790, 0.1 s phases) then makes ~500 ms
//      pulses and a full 111-pulse train takes ~113 s instead of ~29 s -- HUMAN_REVIEW: the panel must count edges.
//      DoShowBinDigital never waits: a non-blocking state machine (iShowBinDigitalTask), one step per call, bounded loops.
//    Timer3                      main.dfm:17301, Enabled = False, no Interval = 1000 ms -> MainTimer3.cpp (S-14)
//  AI(W906-S14) 20261001 (St02-E): TIMER3 ENABLE.  golden Timer3 is off at design time; FormShow :10180 sets
//    Timer3->Enabled=true and InitialOK=true follows at :10464 in the same FormShow.  Here the slot is latched on (g_t3On) by
//    the first pass that sees InitialOK true (WebBridgeTags.cpp:563 PumpInit = the port's FormShow :10464; a modal-wait tick
//    during boot does not start it), so its first call comes one Interval after that, as golden's.  It stays on: golden
//    disables Timer3 only in FormClose (:11705 -> bSystemClose below) and in the two self-close paths (:25675 / :25686,
//    gated in MainTimer3.cpp G21 / G22).
//
//  AI(W906-S13) 20261001 (St02-E), St02-E2 ST02_S15_CLAIMS correction 2 / ST02_S13_CLAIMS section 2:
//  * MODAL WAITS.  golden VCL TTimers keep firing inside every ShowModal (the nested message loop).  The port's three
//    blocking boxes (alarm / yes-no / ShowMyMessage) loop in tools/wb_serve.cpp, where PumpTick does not run; their
//    W906_ModalWaitTick (:7621, St01's A01 segment, same line -- the laptop and St01 agreed) calls
//    W906_St02TimersTickFromModal (end of this file) every pass (<= 100 ms).  The deadlines are the same ones.
//  * RE-ENTRY GUARD g_inTick, PER TIMER, ONLY WHERE GOLDEN HAS ONE.  When one of these handlers opens a waiting box
//    (TimerESD's MES0731, Timer8's MES0732 ...), the modal-wait tick comes back here.  Timer8 (bTimerRunning,
//    main.cpp:32156-32162), TimerESD (bRun, :30913) and Timer1 (bRunTimer1, :2698) return at once when re-entered, so
//    while their handler is still on the stack they are skipped, deadline untouched (St01's A01 guard does the same).
//    Timer3 (:25184-25195) and TimerTemperatureStorageMinute (:31175-31180) have NO running flag: VCL re-enters them
//    inside their own box, so they fire on their deadline even then (AI(W906-R126) 20261002, NB2 R126 M4 -- the guard
//    had been copied from Timer8 / TimerESD; a nested ShowMyMessage returns at once, golden mymessbox.cpp:785).  The
//    OTHER timers always fire, as VCL timers do while another one's ShowModal waits.
//  * CLOSE.  golden FormClose disables TimerESD (main.cpp:11708, with Timer1-4 / TimerScanKey); Timer8 and
//    TimerTemperatureStorageMinute end with the form.  The port's FormClose is FileRW/MainClose.cpp, which sets
//    bSystemClose (:952, golden :12060) => nothing here fires after that.
// ===========================================================================
#include "MachineType.h"
#include "cmydef.h"    // bSystemClose, InitialOK, SystemInitialOK, NUMBER_PANEL_TYPE, bSECSGEMAlarm, bSECSGEM_NoteAlarm
#include "forms/fShowBinSelect.h"   // fShowBinSelect->DoShowBinDigital (cShowBinSelect.cpp:2405), AI(W906-ET1022) 20261001 (St02-E)
#include <windows.h>   // GetTickCount

namespace ht9045 {

void W906_Timer8Timer();                          // MainTimer8.cpp
void W906_TimerTemperatureStorageMinuteTimer();   // MainTimer8.cpp
void W906_TimerESDTimer();                        // MainTimerESD.cpp (S-13)
void W906_Timer3Timer();                          // MainTimer3.cpp (S-14)

namespace {
// VCL TTimer: the first fire one Interval after the start; then once per Interval on a fixed deadline grid; more
// than one Interval late -> one call and a new grid from now (no catch-up).  next == 0: not started.
bool Due(unsigned long& next, unsigned long now, unsigned long ms)
{
    if (next == 0)
    {
        next = (now + ms) ? now + ms : 1;
        return false;
    }
    if ((long)(now - next) < 0)
        return false;
    next = (now - next >= ms) ? now + ms : next + ms;
    return true;
}
enum { kT8 = 0, kTS = 1, kESD = 2, kT3 = 3, kT1 = 4, kSlots = 5 };   // AI(W906-S14): + kT3; AI(W906-ET1022) 20261001 (St02-E): + kT1
unsigned long g_t8 = 0, g_tTS = 0, g_tESD = 0, g_t3 = 0;
unsigned long g_n8 = 0, g_nTS = 0, g_nESD = 0, g_n3 = 0;   // calls, for the ctest
unsigned long g_t1 = 0, g_n1 = 0, g_t1Old = 0;             // AI(W906-ET1022) 20261001 (St02-E): deadline / calls / golden dwOldTime (:2700)
bool g_inTick[kSlots] = { false, false, false, false, false };  // AI(W906-S13): this timer's handler is on the stack (header; kTS / kT3 unused, R126)
bool g_t3On = false;                              // AI(W906-S14): golden Timer3->Enabled (FormShow :10180), latched (header)
unsigned long g_reentries = 0;                    // nested calls that skipped a busy timer, for the ctest

struct InTick
{
    int s;
    explicit InTick(int slot) : s(slot) { g_inTick[s] = true; }
    ~InTick() { g_inTick[s] = false; }
};

// AI(W906-S13): Due() unless this timer's handler is still running (a waiting box inside it): then skip, deadline untouched.
// Only for the timers whose golden handler has a running flag (header); the others call Due() directly.
bool Fire(int slot, unsigned long& next, unsigned long now, unsigned long ms)
{
    if (g_inTick[slot])
    {
        ++g_reentries;
        return false;
    }
    return Due(next, now, ms);
}

// AI(W906-ET1022) 20261001 (St02-E): golden TfMain::Timer1Timer's guards up to :3439, then its only St02 segment (header, option A).
void Timer1BinTick(unsigned long now)
{
    if (NUMBER_PANEL_TYPE != 2)                   // :3439; types 1 / 3 / 4 = Timer3 G7 (MainTimer3.cpp)
        return;
    if (!Fire(kT1, g_t1, now, 30))                // main.dfm:17274 Interval = 30; g_inTick = golden bRunTimer1 (:2727-2731)
        return;
    if (InitialOK == false)                       // :2719
        return;
    if (bSECSGEMAlarm && bSECSGEM_NoteAlarm == false)   // :2938-2959
        return;
    if (g_t1Old > now)                            // :3154-3159
    {
        g_t1Old = now;
        return;
    }
    if (now - g_t1Old < 20)                       // :3163-3167
        return;
    g_t1Old = now;                                // :3171
    if (SystemInitialOK == false)                 // :3172-3176
        return;
    if (fShowBinSelect == 0)                      // [W906] golden's form always exists; a program without it does nothing
        return;
    ++g_n1;
    InTick busy(kT1);
    fShowBinSelect->DoShowBinDigital();           // :3439-3440
}
}  // namespace

void W906_St02TimersTickAt(unsigned long now)    // ctest drives this with its own clock
{
    if (bSystemClose)                             // AI(W906-S13): after the port's FormClose nothing fires (header)
        return;
    if (Fire(kT8, g_t8, now, 1000))
    {
        ++g_n8;
        InTick busy(kT8);
        W906_Timer8Timer();                       // golden main.dfm:17346 Timer8 (1000)
    }
    if (Due(g_tTS, now, 1000))                    // AI(W906-R126) 20261002: golden :31175-31180 has no running flag (header)
    {
        ++g_nTS;
        W906_TimerTemperatureStorageMinuteTimer();   // golden main.dfm:17318 (1000)
    }
    if (Fire(kESD, g_tESD, now, 1000))
    {
        ++g_nESD;
        InTick busy(kESD);
        W906_TimerESDTimer();                     // golden main.dfm:17313 TimerESD (1000)   AI(W906-S13) 20261001
    }
    if (!g_t3On && InitialOK)                     // AI(W906-S14): golden FormShow :10180 Timer3->Enabled=true (header)
        g_t3On = true;
    if (g_t3On && Due(g_t3, now, 1000))           // AI(W906-R126) 20261002: golden :25184-25195 has no running flag (header, NB2 R126 M4)
    {
        ++g_n3;
        W906_Timer3Timer();                       // golden main.dfm:17301 Timer3 (1000)   AI(W906-S14) 20261001
    }
    Timer1BinTick(now);                           // AI(W906-ET1022) 20261001 (St02-E): every pass, golden's 30 ms deadline (header, option A)
}

void W906_St02TimersReset()                       // ctest only
{
    g_t8 = 0; g_tTS = 0; g_tESD = 0; g_t3 = 0;
    g_n8 = 0; g_nTS = 0; g_nESD = 0; g_n3 = 0;
    g_t3On = false;
    g_t1 = 0; g_n1 = 0; g_t1Old = 0;
    g_reentries = 0;
}
void W906_St02TimersCounts(unsigned long* t8, unsigned long* ts) { if (t8) *t8 = g_n8; if (ts) *ts = g_nTS; }   // ctest only
void W906_St02TimersCountsESD(unsigned long* esd, unsigned long* reentries) { if (esd) *esd = g_nESD; if (reentries) *reentries = g_reentries; }   // ctest only (S-13)
void W906_St02TimersCountsT3(unsigned long* t3, bool* on) { if (t3) *t3 = g_n3; if (on) *on = g_t3On; }   // ctest only (S-14)
void W906_St02TimersCountsT1(unsigned long* t1) { if (t1) *t1 = g_n1; }   // ctest only (E-T1-022)
void W906_St02Timer1BinTickAt(unsigned long now) { if (bSystemClose) return; Timer1BinTick(now); }   // ctest only (E-T1-022): the Timer1 slot alone, the dispatcher's order

void W906_St02TimersTick() { W906_St02TimersTickAt(::GetTickCount()); }

}  // namespace ht9045

// AI(W906-S13) 20261001 (St02-E, claim tools/wb_serve.cpp:7621): the modal-wait tick (header).  Global namespace, like
//   wb_serve's W906_ModalWaitTick and St01's W906_A01AutoLogoutTick on the same line.
void W906_St02TimersTickFromModal() { ht9045::W906_St02TimersTick(); }
