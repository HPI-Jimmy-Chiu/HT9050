// ===========================================================================
//  MainTimersSt02.cpp -- the one PumpTick entry for the golden TfMain TTimers St02 ports.
//  AI(W906-ST02-MRB) 20261004 (St02-E): !70 MR-B (RULINGS_20261001 #39; Q84 = A, Q85 = A; design D2-D5, D1 rejected).  The five TfMain timers
//    below -- Timer8, TimerTemperatureStorageMinute, TimerESD, Timer3 and Timer2's N07 block -- are no longer slots of this
//    dispatcher: they are the OnTimers of the golden members fMain->Timer8 / ->TimerTemperatureStorageMinute / ->TimerESD /
//    ->Timer3 / ->Timer2 in the timer table (TimerTable.h), bound by W906_St02TimersBindTable() from W906_TfMain_TimersBoot
//    (forms/fMain_Timers.cpp) and run by the table on every main-loop pass (tools/wb_serve.cpp:4661).  What stays here is
//    what is not a TfMain timer: Timer1's type-2 bin pulse (the fast clock owns it; option A inside the waits) and the
//    HSys.BinDisCtrl Timer1 slot -- still from PumpTick (WebBridgeTags.cpp:605, unchanged) until T-01 / T-STANDIN.
//    * D2 (Q85 = A): the table also runs inside the three blocking waits -- W906_St02TimersTickFromModal (tools/wb_serve.cpp
//      :7621, unchanged) ticks it first; the table skips the entry whose box is open (its running_ flag).
//    * Re-entry is now the table's for all five: a timer is skipped while its own handler is on the stack.  golden VCL
//      re-enters Timer3 and TimerTemperatureStorageMinute (no running flag, R126 M4) -- accepted deviation (D1 rejected,
//      Q85 = A "skip the entry that raised the box"); HUMAN_REVIEW B.
//    * D3: after the port's FormClose (bSystemClose) each wrapper turns its own entry off (golden FormClose :11704-11708).
//    The notes below describe the dispatcher as it was before MR-B for the five; kept for the golden citations.
//  AI(W906-S15) 20261001 (St02-E).  Called from WebBridgeTags.cpp PumpTick (:605, the line the laptop reserved for
//  St02; PumpTick runs about every 500 ms).  Each golden TTimer fires at its own golden Interval, the VCL way:
//  the first call comes one Interval after the start, and a late tick does not catch up (one call, not several).
//  Deadline style (St02-E2 ST02_S15_CLAIMS correction 1, as St01's A01): the next deadline is the last one + Interval,
//  so a ~500 ms beat with jitter still averages 1000 ms (TemperatureStorageLog counts one call as one second).
//    Timer8                      main.dfm:17346, no Interval stored = 1000 ms      -> MainTimer8.cpp (S-15)
//    TimerTemperatureStorageMinute main.dfm:17318, no Interval stored = 1000 ms    -> MainTimer8.cpp (S-15)
//    TimerESD                    main.dfm:17313, no Interval stored = 1000 ms      -> MainTimerESD.cpp (S-13)
//    Timer1, only its :3439-3440 NUMBER_PANEL_TYPE==2 DoShowBinDigital (E-T1-022)  -> Timer1BinTick below   AI(W906-FASTCLK) 20261003: OPTION B now (RULINGS_20261002 #7 = s0 #35 A): in wb_serve the serve loop's 30 ms fast clock runs it (W906_St02Timer1BinFast, after W906_St02TimersTick); while it does, the PumpTick path below skips it and only the waiting boxes' path keeps option A
//      AI(W906-ET1022) 20261001 (St02-E), OPTION A (the laptop 20261001 20:0x: option B -- a 30 ms call on wb_serve.cpp:4575
//      -- is held for Jimmy, NIGHT_REPORT s0 #35): golden Timer1 is 30 ms (main.dfm:17272-17278); here it runs on every
//      dispatcher pass with golden's 30 ms deadline and guards, i.e. about once per 500 ms PumpTick pass (<= 100 ms passes
//      inside waiting boxes).  The type-2 pulse machine (cShowBinSelect.cpp:2729-2790, 0.1 s phases) then makes ~500 ms
//      pulses and a full 111-pulse train takes ~113 s instead of ~29 s -- HUMAN_REVIEW: the panel must count edges.
//      DoShowBinDigital never waits: a non-blocking state machine (iShowBinDigitalTask), one step per call, bounded loops.
//    Timer3                      main.dfm:17301, Enabled = False, no Interval = 1000 ms -> MainTimer3.cpp (S-14)
//    HSys.BinDisCtrl->Timer1     golden 906_0625 MyBinDisp.cpp:92-96, owner NULL, Interval 200 ms, Enabled in the ctor
//                                -> TMyBinDispCtrl::Timer1Timer (BinDisplay/BinDispBringUp_St02.cpp, golden :284-604)
//      AI(W906-ST02-C14) 20261002 (St02-E helper): a slot like the others, on the interval the TTimer object holds (golden
//      changes it: 50 while P66 flashes, 30 for TFT in case 1, back to 200 at the top of every tick); a change restarts the
//      deadline, as VCL TTimer::SetInterval does; Enabled false / no instance (NUMBER_PANEL_TYPE not 3 / 4) = no slot.
//      Running flag: golden Timer1Timer has one (static bRun, :287 / :306-309) -> Fire() (skipped while it is on the stack).
//      Before it, every pass: the InitialOK copy of golden FormShow :10483-10485 (once) and the receive delivery of the two
//      bin-display TComm (the reader thread only queues, BinDispBringUp_St02.cpp banner (1)).  bSystemClose: golden FormClose
//      :11567-11569 copies InitialOK (false by then) once, then nothing fires.  RATE: ~500 ms per PumpTick pass, ~200 ms inside
//      the modal waits (<= 100 ms passes) -- golden 200 ms; HUMAN REVIEW (the panel protocol has 2 s ack timeouts, so it only
//      slows the rotation).
//    Timer2, only its N07 block  main.dfm:17279, Enabled = False, no Interval = 1000 ms -> SECSGEM/N07Alarm_St02.cpp (C15)
//      AI(W906-C15-N07) 20261003 (St02-E helper): golden 906_0625_Steven main.cpp:20937-20999 (the SECS/GEM disconnect
//      alarm).  Latched on with InitialOK like Timer3 (golden FormShow :10179 Timer2->Enabled=true, the line before
//      Timer3's :10180).  Golden Timer2Timer's guards :20874-20882: fShow (TfMain is always shown here), InitialOK, and
//      bTimer2Run = this slot's g_inTick (a waiting box that comes back here skips it).  The port's other Timer2 segments
//      run from their own pumps (MainTimerSegments.cpp big fan, W906_Timer2HeaterTick, the StateRecord / ADAM / RunInfo
//      pumps in wb_serve) and do not share that flag: a waiting box opened by one of them does not hold this slot back,
//      as golden's single bTimer2Run would.  Stops with bSystemClose (golden FormClose :11704 Timer2->Enabled=false).
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
#include "BinDisplay/BinDispBringUp_St02.h"   // AI(W906-ST02-C14) 20261002 (St02-E helper): the bin display Timer1 slot (header)
#include "forms/fMain.h"      // AI(W906-ST02-MRB) 20261004 (St02-E): fMain->Timer8 / TimerTemperatureStorageMinute / TimerESD / Timer3 / Timer2 (forms/fMain_Timers.h)
#include "TimerTable.h"       // AI(W906-ST02-MRB) 20261004 (St02-E): ht9045::TimerTableTick / TTimerEntry; W906_TimerTableTickNow
#include <windows.h>   // GetTickCount

namespace ht9045 {

void W906_Timer8Timer();                          // MainTimer8.cpp
void W906_TimerTemperatureStorageMinuteTimer();   // MainTimer8.cpp
void W906_TimerESDTimer();                        // MainTimerESD.cpp (S-13)
void W906_Timer3Timer();  bool W906_FastClockOwnsBin();   // MainTimer3.cpp (S-14)   AI(W906-FASTCLK) 20261003: + W906_FastClockOwnsBin (FastClockJobs.cpp) -- true while the serve loop's 30 ms fast clock runs Timer1BinTick (FastClockWbServe.cpp sets it; every ctest: false); same line
void W906_N07Timer2Tick_St02();                   // SECSGEM/N07Alarm_St02.cpp (C15)   AI(W906-C15-N07) 20261003

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
enum { kT8 = 0, kTS = 1, kESD = 2, kT3 = 3, kT1 = 4, kBD = 5, kT2 = 6, kSlots = 7 };   // AI(W906-S14): + kT3; AI(W906-ET1022) 20261001 (St02-E): + kT1; AI(W906-ST02-C14) 20261002: + kBD   AI(W906-C15-N07) 20261003: + kT2
unsigned long g_t8 = 0, g_tTS = 0, g_tESD = 0, g_t3 = 0;
unsigned long g_n8 = 0, g_nTS = 0, g_nESD = 0, g_n3 = 0;   // calls, for the ctest
unsigned long g_t1 = 0, g_n1 = 0, g_t1Old = 0;             // AI(W906-ET1022) 20261001 (St02-E): deadline / calls / golden dwOldTime (:2700)
unsigned long g_tBD = 0, g_nBD = 0, g_tBDms = 0;          // AI(W906-ST02-C14) 20261002: bin display Timer1 deadline / calls / the interval the deadline was set for
unsigned long g_t2 = 0, g_n2 = 0;                          // AI(W906-C15-N07) 20261003: Timer2 deadline / calls (for the ctest)
bool g_t2On = false;                                       // AI(W906-C15-N07) 20261003: golden Timer2->Enabled (FormShow :10179), latched
bool g_inTick[kSlots] = { false, false, false, false, false, false, false };  // AI(W906-S13): this timer's handler is on the stack (header; kTS / kT3 unused, R126)
bool g_t3On = false;                              // AI(W906-S14): golden Timer3->Enabled (FormShow :10180), latched (header)
unsigned long g_reentries = 0;  int g_modalDepth = 0;   // nested calls that skipped a busy timer, for the ctest.  AI(W906-FASTCLK) 20261003: g_modalDepth > 0 while a blocking box's wait loop runs the dispatcher (W906_St02TimersTickModalAt); same line

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
}  bool FireFast(int slot) { if (g_inTick[slot]) { ++g_reentries; return false; } return true; }   // AI(W906-FASTCLK) 20261003: Fire() without the deadline -- the caller's clock is the Interval (the serve loop's fast clock); the running flag still holds; same line

// AI(W906-ET1022) 20261001 (St02-E): golden TfMain::Timer1Timer's guards up to :3439, then its only St02 segment (header, option A).
void Timer1BinTick(unsigned long now, bool fast = false)   // AI(W906-FASTCLK) 20261003: fast = called by the serve loop's 30 ms fast clock (W906_St02Timer1BinFastAt), which IS golden Timer1's Interval; same line
{
    if (NUMBER_PANEL_TYPE != 2)                   // :3439; types 1 / 3 / 4 = Timer3 G7 (MainTimer3.cpp)
        return;
    if (fast ? !FireFast(kT1) : !Fire(kT1, g_t1, now, 30))   // main.dfm:17274 Interval = 30; g_inTick = golden bRunTimer1 (:2727-2731).  AI(W906-FASTCLK) 20261003: fast -> no deadline here, the running flag only
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
// AI(W906-ST02-C14) 20261002 (St02-E helper): golden TMyBinDispCtrl::Timer1 (header).
void BinDispTimer1Slot(unsigned long now)
{
    W906_BinDispFormShowAt_St02();                // golden FormShow main.cpp:10483-10485, once InitialOK is true
    W906_BinDispRxPumpAt_St02(now);               // SPComm's receive delivery (BinDispBringUp_St02.cpp banner (1))
    const unsigned long ms = W906_BinDispTimer1Interval_St02();   // 0 = no Timer1 to run
    if (ms != g_tBDms)                            // VCL: a new Interval (or Enabled) restarts the timer
    {
        g_tBDms = ms;
        g_tBD = 0;
    }
    if (ms == 0 || !Fire(kBD, g_tBD, now, ms))     // g_inTick = golden's static bRun (MyBinDisp.cpp:287 / :306-309)
        return;
    ++g_nBD;
    InTick busy(kBD);
    W906_BinDispTimer1Fire_St02();                // Timer1->OnTimer -> TMyBinDispCtrl::Timer1Timer
}
// AI(W906-ST02-MRB) 20261004 (St02-E): the five TfMain timers as table OnTimers (header).  g_e[] = the entries W906_St02TimersBindTable bound.
enum { kE8 = 0, kETS = 1, kEESD = 2, kE3 = 3, kE2 = 4, kEntries = 5 };
TTimerEntry* g_e[kEntries] = { 0, 0, 0, 0, 0 };
unsigned long long g_skip0 = 0;                           // the entries' reentrySkips at the last W906_St02TimersReset (ctest)
unsigned long long Skips() { unsigned long long n = 0; for (int k = 0; k < kEntries; ++k) if (g_e[k]) n += g_e[k]->Stats.reentrySkips; return n; }
// D3: golden FormClose main.cpp:11704-11708 (Timer2 / Timer3 / TimerESD off; Timer8 / TimerTemperatureStorageMinute end
// with the form) -> after the port's FormClose (bSystemClose, FileRW/MainClose.cpp:952) the entry turns itself off.
bool Closed(int k) { if (!bSystemClose) return false; if (g_e[k]) g_e[k]->Enabled = false; return true; }
void St02OnTimer8()   { if (Closed(kE8)) return;   ++g_n8;   W906_Timer8Timer(); }                         // golden main.dfm:17346 Timer8 (1000); bTimerRunning = the table's running flag
void St02OnTimerTS()  { if (Closed(kETS)) return;  ++g_nTS;  W906_TimerTemperatureStorageMinuteTimer(); }   // golden main.dfm:17318 (1000); golden re-enters it (R126 M4) -- header
void St02OnTimerESD() { if (Closed(kEESD)) return; ++g_nESD; W906_TimerESDTimer(); }                       // golden main.dfm:17313 TimerESD (1000); bRun :30913
void St02OnTimer3()   { if (Closed(kE3)) return;   if (InitialOK) ++g_n3; W906_Timer3Timer(); }            // golden main.dfm:17301 Timer3 (1000); its body returns at once while InitialOK is false (:25194)
// golden TfMain::Timer2Timer (906_0625_Steven main.cpp:20862-21760): its guards up to :20882, then its only St02 segment,
// the N07 block :20937-20999.  :20874-20875 fShow: TfMain is always shown while wb_serve runs; bTimer2Run (:20880-20882)
// = the table's running flag.  The port's other Timer2 segments still run from their own pumps (header) -> "partial".
void St02OnTimer2N07()
{
    if (Closed(kE2))
        return;
    if (InitialOK == false)                       // :20877-20878
        return;
    ++g_n2;
    W906_N07Timer2Tick_St02();                    // :20937-20999 (SECSGEM/N07Alarm_St02.cpp)
}

// What stays on PumpTick (header): no TfMain timer any more.
void St02PassAt(unsigned long now)
{
    if (bSystemClose)                             // AI(W906-S13): after the port's FormClose nothing fires (header)
    {
        W906_BinDispFormClose_St02();             // AI(W906-ST02-C14) 20261002: golden FormClose main.cpp:11567-11569, once
        return;
    }
    if (!W906_FastClockOwnsBin()) Timer1BinTick(now);   // AI(W906-ET1022) 20261001 (St02-E): every pass, golden's 30 ms deadline (header, option A).  AI(W906-FASTCLK) 20261003: while the serve loop's fast clock owns it (option B), only the blocking boxes' pass runs it here.  AI(W906-FASTCLK-MODAL) 20261003: E-FT1-001 -- the fast clock now runs in the boxes too (FastClockWbServe.cpp), so while it owns the bin panel neither path here runs it (one runner); `|| g_modalDepth > 0` dropped
    BinDispTimer1Slot(now);                       // AI(W906-ST02-C14) 20261002 (St02-E helper): golden TMyBinDispCtrl::Timer1 (header)
}
}  // namespace

// AI(W906-ST02-MRB) 20261004 (St02-E): golden main.dfm OnTimer = Timer8Timer / TimerTemperatureStorageMinuteTimer / TimerESDTimer / Timer3Timer /
//   Timer2Timer, and golden FormShow :10179-10180 (Timer2->Enabled=true; Timer3->Enabled=true).  Called by
//   W906_TfMain_TimersBoot (forms/fMain_Timers.cpp) once fMain is the live facade, and by W906_St02TimersReset (ctest).
void W906_St02TimersBindTable()
{
    if (fMain == 0)
        return;
    TTimerEntry* const e[kEntries] = { fMain->Timer8, fMain->TimerTemperatureStorageMinute, fMain->TimerESD, fMain->Timer3, fMain->Timer2 };
    void (*const f[kEntries])() = { &St02OnTimer8, &St02OnTimerTS, &St02OnTimerESD, &St02OnTimer3, &St02OnTimer2N07 };
    const char* const st[kEntries] = { "partial", "full", "partial", "partial", "partial" };   // gates: MainTimer8.cpp / MainTimerESD.cpp / MainTimer3.cpp; Timer2 = its N07 block only
    for (int k = 0; k < kEntries; ++k)
    {
        g_e[k] = e[k];
        e[k]->OnTimer = f[k];
        e[k]->Status = st[k];
    }
    fMain->Timer2->Enabled = true;                // golden FormShow :10179
    fMain->Timer3->Enabled = true;                // golden FormShow :10180 (also W906_TfMain_TimersBoot's own line)
}

// AI(W906-ST02-MRB) 20261004 (St02-E): ctest only now -- one main-loop pass on the test clock: the table (as tools/wb_serve.cpp:4661) and the
//   St02 pass (as PumpTick).  wb_serve itself never calls this (PumpTick = W906_St02TimersTick below, the table = :4661).
void W906_St02TimersTickAt(unsigned long now)
{
    ht9045::TimerTableTick(now);
    St02PassAt(now);
}

void W906_St02TimersReset()                       // ctest only
{
    g_t8 = 0; g_tTS = 0; g_tESD = 0; g_t3 = 0;
    g_n8 = 0; g_nTS = 0; g_nESD = 0; g_n3 = 0;
    g_t3On = false;
    W906_St02TimersBindTable();                   // AI(W906-ST02-MRB) 20261004 (St02-E): the five entries bound and enabled (W906_TfMain_TimersBoot)
    for (int k = 0; k < kEntries; ++k)            // restart them: the next TimerTableTick counts one Interval from its now
    {
        if (g_e[k] == 0) continue;
        g_e[k]->Enabled = true;
        const int iv = g_e[k]->Interval;
        g_e[k]->Interval = 0;
        g_e[k]->Interval = iv;
    }
    g_skip0 = Skips();
    g_t1 = 0; g_n1 = 0; g_t1Old = 0;
    g_tBD = 0; g_nBD = 0; g_tBDms = 0;               // AI(W906-ST02-C14) 20261002
    g_t2 = 0; g_n2 = 0; g_t2On = false;           // AI(W906-C15-N07) 20261003
    g_reentries = 0;
}
void W906_St02TimersCounts(unsigned long* t8, unsigned long* ts) { if (t8) *t8 = g_n8; if (ts) *ts = g_nTS; }   // ctest only
void W906_St02TimersCountsESD(unsigned long* esd, unsigned long* reentries) { if (esd) *esd = g_nESD; if (reentries) *reentries = g_reentries + (unsigned long)(Skips() - g_skip0); }   // ctest only (S-13)
void W906_St02TimersCountsT3(unsigned long* t3, bool* on) { if (t3) *t3 = g_n3; if (on) *on = g_e[kE3] != 0 && (bool)g_e[kE3]->Enabled; }   // ctest only (S-14)
void W906_St02TimersCountsT1(unsigned long* t1) { if (t1) *t1 = g_n1; }   // ctest only (E-T1-022)
void W906_St02Timer1BinTickAt(unsigned long now) { if (bSystemClose) return; Timer1BinTick(now); }   // ctest only (E-T1-022): the Timer1 slot alone, the dispatcher's order
void W906_St02TimersCountsBD(unsigned long* bd) { if (bd) *bd = g_nBD; }   // ctest only (ST02-C14)
void W906_St02TimersCountsT2(unsigned long* t2, bool* on) { if (t2) *t2 = g_n2; if (on) *on = g_e[kE2] != 0 && (bool)g_e[kE2]->Enabled; }   // ctest only (C15)
void W906_St02Timer2N07TickAt(unsigned long now) { if (bSystemClose) return; for (int k = 0; k < kEntries; ++k) if (k != kE2 && g_e[k]) g_e[k]->Enabled = false; ht9045::TimerTableTick(now); }   // AI(W906-ST02-MRB) 20261004 (St02-E): the Timer2 entry alone -- the other four bound entries are switched off for it   // ctest only (C15): the Timer2 slot alone, the dispatcher's order
void W906_St02BinDispSlotAt(unsigned long now) { if (bSystemClose) { W906_BinDispFormClose_St02(); return; } BinDispTimer1Slot(now); }   // ctest only (ST02-C14): the bin display slot alone, the dispatcher's order

void W906_St02TimersTick() { St02PassAt(::GetTickCount()); }   // AI(W906-ST02-MRB) 20261004 (St02-E): PumpTick (WebBridgeTags.cpp:605): the St02 pass only; the table runs on :4661

// AI(W906-FASTCLK) 20261003: RULINGS_20261002 #7 (s0 #35 = A) = option B of the header's E-T1-022 note.  The serve loop's shared
//   fast clock (FastClockWbServe.cpp) calls this every 30 ms = golden Timer1's Interval (main.dfm:17274), so Timer1BinTick runs
//   with fast=true: no deadline of its own, golden's guards unchanged -- InitialOK, the S10F3 alarm, the 20 ms dwOldTime rule
//   (:3154-3167; it also keeps this path and the modal path below from both firing within 20 ms), SystemInitialOK and
//   bRunTimer1 (FireFast).  The same GetTickCount the dispatcher uses (golden MyTickCount).
void W906_St02Timer1BinFastAt(unsigned long now) { if (bSystemClose) return; Timer1BinTick(now, true); }
void W906_St02Timer1BinFast() { W906_St02Timer1BinFastAt(::GetTickCount()); }
// AI(W906-FASTCLK) 20261003: the blocking boxes' pass (W906_St02TimersTickFromModal below): a depth mark so the dispatcher still
//   runs the bin panel while the fast clock owns it -- the fast clock runs on the main loop only (FastClockWbServe.cpp header).
//   A depth, not a flag: a timer's own box nests a second pass.
void W906_St02TimersTickModalAt(unsigned long now)   // ctest: one waiting-box pass on the test clock
{
    struct Depth { Depth() { ++g_modalDepth; } ~Depth() { --g_modalDepth; } } depth;
    ht9045::TimerTableTick(now);                  // AI(W906-ST02-MRB) 20261004 (St02-E): D2 (Q85 = A) -- the table inside the waits
    St02PassAt(now);
}
void W906_St02TimersTickModalNow()                // AI(W906-ST02-MRB) 20261004 (St02-E): the waiting boxes' pass in wb_serve (below)
{
    struct Depth { Depth() { ++g_modalDepth; } ~Depth() { --g_modalDepth; } } depth;
    W906_TimerTableTickNow();                     // the same 64-bit clock as the main loop's :4661 call
    St02PassAt(::GetTickCount());
}

}  // namespace ht9045

// AI(W906-S13) 20261001 (St02-E, claim tools/wb_serve.cpp:7621): the modal-wait tick (header).  Global namespace, like
//   wb_serve's W906_ModalWaitTick and St01's W906_A01AutoLogoutTick on the same line.
void W906_St02TimersTickFromModal() { ht9045::W906_St02TimersTickModalNow(); }   // AI(W906-FASTCLK) 20261003: through the modal depth mark (W906_St02TimersTickModalAt above); same line

// AI(W906-S27) 20261004 (St02-E): S-27 / INBOX 93 -- golden 0618 note.cpp:3355 / mymessbox.cpp:542: the two blocking boxes' own
//   Timer1 (Interval 10 ms, note.dfm:22686 / mymessbox.dfm:211) call fMain->Timer1Timer on every tick, so golden Timer1's segments keep
//   running while a box is up.  W906_ModalWaitTick (tools/wb_serve.cpp:7630) already runs FlushFlag, UpdateRecordScreen,
//   CheckIndexAllSuckICFallDown and St02's dispatcher; these are the ported Timer1 segments that only PumpTick ran
//   (WebBridgeTags.cpp:600 / :626), so they stopped while a box waited:
//   - golden :3189 ProcessTimeUpdate -> GetTimeInfo: the SystemHour / Min / Sec globals.  First, before the guard, as PumpTick does.
//   - golden :3051-3077 the safe-door / magazine locks follow SystemStart (ht9045::W906_SafeDoorLockTick, MainTimerSegments.cpp:31;
//     its N07 / SECS-alarm guards = golden :2928 / :2949 / :2957 are inside).  An alarm stops the machine before its box shows
//     (golden note.cpp:801 SystemStart=false), so golden unlocks the door while the alarm is up; the port kept it locked until the
//     box closed.
//   Guards as at the top of golden Timer1Timer: :2719 InitialOK, :2698 / :2727 bRunTimer1 (re-entry).  Called once per box pass
//   (<= 100 ms; golden 10 ms); both calls are idempotent.  ctest St02_ModalTimer1.
//   Why a flag local to this function is enough: golden bRunTimer1 is Timer1Timer's own static, so it only blocks a box raised from
//   inside Timer1Timer.  The port runs those Timer1 segments in PumpTick (W906_FlushFlagTick, W906_SafeDoorLockTick,
//   W906_Timer2FanTick) and none of them can raise a box: they flip a flag or write SW[] outputs, and TMySwitch::On / Off
//   (myswitch.cpp:74 / :124) -> IOBitOn / IOBitOff (myio.cpp:282 / :316, IdleCheckSafeDoorByCylinder csystem.cpp:21333) show no
//   box (the pad SendSwitchStatus branch is #if 0); GetTimeInfo only reads the clock.  So every box that reaches this call was
//   raised outside golden Timer1 (MainProc etc. -- golden runs MainProc on the run-control thread, uruncontrol.cpp:38) and
//   golden would run these segments in it; the flag only stops this function's own recursion.
void GetTimeInfo();                                             // cpublic.h:23
namespace ht9045 { void W906_SafeDoorLockTick(); }               // MainTimerSegments.cpp:31
namespace { bool g_s27Running = false; }                        // golden :2698 static bool bRunTimer1
void W906_ModalTimer1Segments()
{
    GetTimeInfo();                                              // golden :3189 (PumpTick: WebBridgeTags.cpp:600, before its guard)
    if (InitialOK == false || g_s27Running)                     // golden :2719 / :2727
        return;
    struct Run { Run() { g_s27Running = true; } ~Run() { g_s27Running = false; } } run;   // cleared on every way out
    ht9045::W906_SafeDoorLockTick();                            // golden :3051-3077
}
