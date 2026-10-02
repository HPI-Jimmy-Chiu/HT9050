// =============================================================================
//  test_pci1203_linkwatch.cpp -- EtherCAT/Pci1203LinkWatch.h, the 1203 link-lost
//  watch behind golden WAR16152 (tools/wb_serve.cpp EOF W906_Pci1203LinkWatchTick).
//
//  //AI(W906-1203-LINKLOST) 20261001: EastSun 1001「1203 如果斷線10秒 要跳出異常」.
//  A FAKE MONITOR (the five members the adapter reads) and a FAKE CLOCK (the test
//  hands Step() its milliseconds and its wall-clock text) -- so every rule is
//  walked without a card:
//    [1] never linked at boot (card never opens / ring never reaches OP) -> no alarm,
//        also with the REAL TPci1203Monitor of a build without the vendor SDK;
//    [2] lost 9.9 s -> no alarm; lost 10 s -> ONE alarm carrying the reason;
//        still lost for a minute -> no second alarm;
//    [3] restored -> the timer resets; lost again 10 s -> the alarm again;
//    [4] one good sample inside a loss restarts the 10 s;
//    [5] the card-level signals: monitor auto-disabled, card not open, empty scan,
//        a re-learnt ring with nothing in OP;
//    [6] which stations are watched: never-OP and unaddressable stations are not;
//        a scan that drops a station re-learns the ring;
//    [7] SAFEOP counts as down; the reason is CSV / quote safe and bounded;
//    [8] the tick counter wraps (GetTickCount after 49.7 days).
//  ⚠ WHAT IT DOES NOT PROVE: what Acm_DevGetSlaveStates actually returns for a
//  station behind a pulled cable -- built WITHOUT HAVE_PCI1203 like every ctest
//  target. That is the on-machine check (pull a ring-1 cable for > 10 s, watch the
//  oplog LINK lines and pci1203.slaveN.state). Reads and writes no file.
// =============================================================================
#include "EtherCAT/Pci1203LinkWatch.h"
#include "EtherCAT/Pci1203Monitor.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace ht9045;

static int g_total = 0;
static int g_fail  = 0;

static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static bool has(const std::string& s, const char* part) { return s.find(part) != std::string::npos; }

// ---------------------------------------------------------------------------
//  The fake monitor: the same five members Pci1203LinkInputFromMonitor reads on
//  TPci1203Monitor, holding the real sample structs.
// ---------------------------------------------------------------------------
struct FakeMon {
    Pci1203CardSample               c;
    bool                            dis;
    std::string                     disWhy;
    std::vector<Pci1203SlaveSample> s;
    FakeMon() : dis(false) { s.resize(8); }
    const Pci1203CardSample&  card() const { return c; }
    bool                      Disabled() const { return dis; }
    const std::string&        disabledReason() const { return disWhy; }
    int                       slaveCount() const { return (int)s.size(); }
    const Pci1203SlaveSample& slave(int i) const { return s[i]; }

    // A station in slot i that the scan found.
    void Station(int i, int ring, int addr, unsigned short state, bool readOk = true, unsigned long err = 0)
    {
        Pci1203SlaveSample& x = s[i];
        x.present = true; x.ring = ring; x.addr = addr; x.unaddressable = false;
        x.stateValid = readOk; x.state = readOk ? state : 0; x.lastError = readOk ? 0 : err;
        x.aliasValid = false; x.alias = 0;
    }
    void Up(int i)                       { s[i].stateValid = true;  s[i].state = 0x08; s[i].lastError = 0; }
    void Fail(int i, unsigned long err)  { s[i].stateValid = false; s[i].lastError = err; }
    void State(int i, unsigned short st) { s[i].stateValid = true;  s[i].state = st; s[i].lastError = 0; }
};

//  The "machine" most cases start from: card open, ring 0 two drives, ring 1 two IO modules, all in OP.
static FakeMon Healthy()
{
    FakeMon m;
    m.c.open = true;
    m.Station(0, 0, 0x00E, 0x08);
    m.Station(1, 0, 0x001, 0x08);
    m.Station(2, 1, 0x009, 0x08);
    m.Station(3, 1, 0x00A, 0x08);
    m.s[2].aliasValid = true; m.s[2].alias = 0x0050;
    return m;
}

//  Run the watch from t0 to t1 inclusive, one sample every `stepMs` (the 200 ms IO clock); count events.
struct Run { int armed, lost, alarm, restored; std::string lastAlarm, lastLost, lastRestored; Run() : armed(0), lost(0), alarm(0), restored(0) {} };
static void Feed(Pci1203LinkWatch& w, const FakeMon& m, unsigned long t0, unsigned long t1,
                 unsigned long stepMs, Run& run, const char* clock = "10:00:00")
{
    for (unsigned long t = t0; ; t += stepMs) {
        if (t - t0 > t1 - t0) break;   // wrap-safe "t > t1"
        const Pci1203LinkWatch::Result r = w.Step(Pci1203LinkInputFromMonitor(m), t, clock);
        if (r.event == Pci1203LinkWatch::kArmed)    { ++run.armed; }
        if (r.event == Pci1203LinkWatch::kLost)     { ++run.lost;     run.lastLost = r.text; }
        if (r.event == Pci1203LinkWatch::kAlarm)    { ++run.alarm;    run.lastAlarm = r.text; }
        if (r.event == Pci1203LinkWatch::kRestored) { ++run.restored; run.lastRestored = r.text; }
        if (t1 - t < stepMs) break;
    }
}

int main()
{
    std::printf("== test_pci1203_linkwatch (AI(W906-1203-LINKLOST) 20261001) ==\n");
    check(kPci1203LinkLostAlarmMs == 10000, "[0] the limit is 10 s, in one place (kPci1203LinkLostAlarmMs)");

    // -----------------------------------------------------------------------
    //  [1] never linked at boot
    // -----------------------------------------------------------------------
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m;                                    // card never opens, nothing scanned
        Feed(w, m, 1000, 61000, 200, run);
        check(!w.armed() && run.alarm == 0 && run.lost == 0 && run.armed == 0,
              "[1a] card never opened for 60 s: not armed, no lost event, no alarm");
        m.c.open = true; m.dis = true; m.disWhy = "not linked";   // the no-SDK Open(): disabled from the start
        Feed(w, m, 61200, 121200, 200, run);
        check(!w.armed() && run.alarm == 0, "[1b] disabled from the start (\"not linked\"): not armed, no alarm");
        m.dis = false; m.disWhy.clear();
        m.Station(0, 0, 0x00E, 0x02);                 // the ring came up to PREOP and stays there
        m.Station(1, 1, 0x009, 0x04);                 // SAFEOP
        m.Station(2, 1, 0x00A, 0, false, 0x8300000Ful);
        Feed(w, m, 121400, 181400, 200, run);
        check(!w.armed() && run.alarm == 0 && w.watched() == 0,
              "[1c] stations scanned but none ever in OP for 60 s: not armed, nothing watched, no alarm");
        m.Up(0);                                      // the first station reaches OP: armed now
        Feed(w, m, 181600, 181600, 200, run);
        check(w.armed() && run.armed == 1 && run.alarm == 0 && run.lost == 0 && w.watched() == 1,
              "[1d] first station in OP -> armed once, watching only that one, not lost");
    }
    {
        //  The REAL monitor of this build (no HAVE_PCI1203): constructed, Open() refused -- the adapter compiles
        //  against TPci1203Monitor and the watch never arms on it.
        TPci1203Monitor real;
        std::string why;
        const bool opened = real.Open(16, 32, why);
        Pci1203LinkWatch w;
        int events = 0;
        for (unsigned long t = 0; t <= 60000; t += 200) {
            real.Poll();
            if (w.Step(Pci1203LinkInputFromMonitor(real), t, "10:00:00").event != Pci1203LinkWatch::kNone) ++events;
        }
        check(!opened && !w.armed() && events == 0 && w.alarms() == 0,
              "[1e] real TPci1203Monitor without the vendor SDK: Open refused, 60 s of samples, never armed, no event");
    }

    // -----------------------------------------------------------------------
    //  [2] 9.9 s -> nothing; 10 s -> one alarm with the reason; still lost -> no second one
    // -----------------------------------------------------------------------
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 5000, 200, run);
        check(w.armed() && run.armed == 1 && w.watched() == 4 && run.lost == 0,
              "[2a] healthy ring: armed once, 4 stations watched, never lost");

        m.Fail(2, 0x8300000Ful);                      // ring 1 addr 0x009 stops answering at t=10000
        m.State(3, 0x01);                             // ring 1 addr 0x00A drops to INIT
        Run lostRun;
        Feed(w, m, 10000, 10000, 200, lostRun, "14:03:22");
        check(lostRun.lost == 1 && lostRun.alarm == 0 && w.lost() && !w.raised(),
              "[2b] first lost sample: one kLost event, timer started, no alarm");
        check(has(lostRun.lastLost, "r1 addr 0x009") && has(lostRun.lastLost, "WAR16152"),
              "[2b] the kLost log line names the station and the alarm to come");
        Feed(w, m, 10200, 19900, 100, lostRun, "14:03:31");   // up to 9.9 s after the first lost sample
        check(lostRun.alarm == 0 && w.alarms() == 0, "[2c] lost 9.9 s: NO alarm");
        Feed(w, m, 20000, 20000, 100, lostRun, "14:03:32");
        check(lostRun.alarm == 1 && w.alarms() == 1 && w.raised(), "[2d] lost 10.0 s: ONE alarm");
        const std::string& a = lostRun.lastAlarm;
        std::printf("      alarm text: %s\n", a.c_str());
        check(has(a, "1203 link lost 10.0 s since 14:03:22"), "[2d] reason: how long and SINCE WHEN (the first lost sample's clock)");
        check(has(a, "ring 1: 0/2 in OP") && has(a, "ring 0: 2/2 in OP"), "[2d] reason: per-ring tally of watched stations");
        check(has(a, "2 stations not in OP"), "[2d] reason: how many stations are down");
        check(has(a, "r1 addr 0x009 alias 0x0050 read failed 0x8300000F"), "[2d] reason: the failed read, with its alias and error");
        check(has(a, "r1 addr 0x00A state 0x01 INIT"), "[2d] reason: the station that left OP, with its state");
        Feed(w, m, 20200, 80000, 200, lostRun);
        check(lostRun.alarm == 1 && w.alarms() == 1 && lostRun.lost == 1,
              "[2e] still lost for another minute: no second alarm, no second kLost");

        // -------------------------------------------------------------------
        //  [3] restored -> the timer resets; a new 10 s loss alarms again
        // -------------------------------------------------------------------
        m.Up(2); m.Up(3);
        Run back;
        Feed(w, m, 80200, 80200, 200, back);
        check(back.restored == 1 && !w.lost() && !w.raised() && w.alarms() == 1,
              "[3a] every watched station in OP again: one kRestored, timer and raised mark cleared");
        check(has(back.lastRestored, "WAR16152 was raised"), "[3a] the restore line says the alarm had been raised");
        Feed(w, m, 80400, 90000, 200, back);
        check(back.restored == 1 && back.lost == 0 && back.alarm == 0, "[3b] healthy afterwards: no event");
        m.Fail(0, 0x80000001ul);                      // ring 0 addr 0x00E drops at t=100000
        Run again;
        Feed(w, m, 100000, 109999, 200, again);
        check(again.lost == 1 && again.alarm == 0, "[3c] lost again: kLost, and no alarm before 10 s");
        Feed(w, m, 110000, 110000, 200, again);
        check(again.alarm == 1 && w.alarms() == 2 && has(again.lastAlarm, "r0 addr 0x00E read failed 0x80000001")
              && has(again.lastAlarm, "ring 0: 1/2 in OP"),
              "[3d] lost again for 10 s: the alarm AGAIN (second one), naming the new station");
    }

    // -----------------------------------------------------------------------
    //  [4] one good sample inside a loss restarts the 10 s
    // -----------------------------------------------------------------------
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 1000, 200, run);
        m.Fail(1, 0x8300000Ful);
        Feed(w, m, 2000, 11000, 200, run);            // 9 s lost
        m.Up(1);
        Feed(w, m, 11200, 11200, 200, run);           // one good sample
        m.Fail(1, 0x8300000Ful);
        Feed(w, m, 11400, 20400, 200, run);           // 9 s lost again
        check(run.alarm == 0 && run.lost == 2 && run.restored == 1,
              "[4] 9 s lost + one good sample + 9 s lost: two short episodes, NO alarm");
        Feed(w, m, 20600, 21400, 200, run);           // the second episode reaches 10 s at 21400
        check(run.alarm == 1, "[4] ... and the second episode alarms once it has lasted 10 s by itself");
    }

    // -----------------------------------------------------------------------
    //  [5] the card-level signals
    // -----------------------------------------------------------------------
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 1000, 200, run);
        m.dis = true;
        m.disWhy = "auto-disabled after 10 consecutive polls in which every read failed (last error 0x8300000F)";
        Feed(w, m, 2000, 12000, 200, run);
        check(run.alarm == 1 && has(run.lastAlarm, "card not answering - monitor auto-disabled: auto-disabled after 10"),
              "[5a] monitor auto-disabled (card stopped answering): alarm after 10 s with the monitor's own reason");
    }
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 1000, 200, run);
        m.c.open = false; m.c.lastErrorText = "detached: production closed the card (uiDevhand went to 0)";
        Feed(w, m, 2000, 12000, 200, run);
        check(run.alarm == 1 && has(run.lastAlarm, "card not open - detached: production closed the card"),
              "[5b] card not open: alarm after 10 s naming why");
    }
    {
        //  A failed Refresh: Close() clears the slave list, the card stays closed.
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 1000, 200, run);
        FakeMon closed;                               // everything default: open=false, nothing present
        Feed(w, closed, 2000, 12000, 200, run);
        check(run.alarm == 1 && has(run.lastAlarm, "card not open"), "[5c] Refresh failed (monitor closed, list cleared): alarm");
        //  A later Refresh opens the card but sweeps an empty ring.
        FakeMon empty; empty.c.open = true;
        Run run2;
        Feed(w, empty, 12200, 40000, 200, run2);
        check(run2.alarm == 0 && run2.restored == 0 && w.lost(),
              "[5d] Refresh opens the card on an empty ring: still the SAME episode (no restore, no second alarm)");
        //  ... the ring comes back but every station is still PREOP.
        FakeMon pre; pre.c.open = true;
        pre.Station(0, 0, 0x00E, 0x02); pre.Station(1, 1, 0x009, 0x02);
        Feed(w, pre, 40200, 50000, 200, run2);
        check(run2.restored == 0 && w.lost(), "[5e] ring scanned again but nothing in OP yet: still lost, not restored");
        pre.Up(0); pre.Up(1);
        Feed(w, pre, 50200, 50200, 200, run2);
        check(run2.restored == 1 && !w.lost() && w.watched() == 2, "[5f] stations reach OP: restored, the new ring is watched");
    }
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 1000, 200, run);
        FakeMon empty; empty.c.open = true;           // an open card whose rescan found no station
        Feed(w, empty, 2000, 12000, 200, run);
        check(run.alarm == 1 && has(run.lastAlarm, "the scan finds no EtherCAT station"),
              "[5g] open card, empty scan: alarm after 10 s");
    }

    // -----------------------------------------------------------------------
    //  [6] which stations are watched
    // -----------------------------------------------------------------------
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        m.State(3, 0x02);                             // ring 1 0x00A never reaches OP (stuck PREOP)
        m.Station(4, 0, 0x00A, 0x08);                 // an unaddressable twin: its state is someone else's
        m.s[4].unaddressable = true;
        Feed(w, m, 0, 30000, 200, run);
        check(run.alarm == 0 && run.lost == 0 && w.watched() == 3,
              "[6a] a station stuck in PREOP since boot is not watched; an unaddressable one is skipped: no alarm");
        m.Fail(4, 0x8300000Ful);
        Feed(w, m, 30200, 60000, 200, run);
        check(run.alarm == 0 && run.lost == 0, "[6b] the unaddressable station's read failing does not count");
        m.Up(3);                                      // it reaches OP -> watched from now on
        Feed(w, m, 60200, 60200, 200, run);
        check(w.watched() == 4, "[6c] the late station reaches OP: watched from then on");
        m.State(3, 0x02);
        Feed(w, m, 60400, 70400, 200, run);
        check(run.alarm == 1 && has(run.lastAlarm, "r1 addr 0x00A state 0x02 PREOP"),
              "[6d] ... and leaving OP again now alarms after 10 s");
    }
    {
        //  A Refresh that no longer finds a module (removed on purpose): the ring is re-learnt, no alarm.
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 1000, 200, run);
        FakeMon after = Healthy();
        after.s[3] = Pci1203SlaveSample();            // ring 1 0x00A gone from the scan
        Feed(w, after, 1200, 30000, 200, run);
        check(run.alarm == 0 && run.lost == 0 && w.watched() == 3,
              "[6e] a scan without a module re-learns the ring (3 watched): no loss, no alarm");
    }

    // -----------------------------------------------------------------------
    //  [7] SAFEOP is down; the text is safe
    // -----------------------------------------------------------------------
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 1000, 200, run);
        m.State(0, 0x14);                             // SAFEOP + ERR
        Feed(w, m, 2000, 12000, 200, run);
        check(run.alarm == 1 && has(run.lastAlarm, "r0 addr 0x00E state 0x14 SAFEOP+ERR"),
              "[7a] a station that drops to SAFEOP(+ERR) is down (strict OP, golden MyEtherCAT.cpp:531)");
    }
    {
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0, 1000, 200, run);
        m.c.open = false;
        m.c.lastErrorText = std::string("vendor says: \"cable, unplugged\"; it's ") + std::string(600, 'x');
        Feed(w, m, 2000, 12000, 200, run);
        const std::string& a = run.lastAlarm;
        check(run.alarm == 1 && a.find(',') == std::string::npos && a.find('"') == std::string::npos &&
              a.find('\'') == std::string::npos, "[7b] the alarm text has no comma / double / single quote (csv + SQL errPart)");
        check(a.size() <= (std::size_t)kPci1203LinkLostTextMax && has(a, "..."), "[7b] ... and is clipped to kPci1203LinkLostTextMax");
    }
    {
        //  Many stations down: the first four are named, the rest counted.
        Pci1203LinkWatch w; Run run;
        FakeMon m; m.c.open = true;
        for (int i = 0; i < 8; ++i) m.Station(i, 1, 0x010 + i, 0x08);
        Feed(w, m, 0, 1000, 200, run);
        for (int i = 0; i < 7; ++i) m.Fail(i, 0x8300000Ful);
        Feed(w, m, 2000, 12000, 200, run);
        check(run.alarm == 1 && has(run.lastAlarm, "7 stations not in OP") && has(run.lastAlarm, "(+3 more)")
              && has(run.lastAlarm, "ring 1: 1/8 in OP"),
              "[7c] 7 of 8 down: the tally, four named, \"+3 more\"");
    }

    // -----------------------------------------------------------------------
    //  [8] the tick wraps
    // -----------------------------------------------------------------------
    {
        check(Pci1203LinkElapsedMs(0x00001710ul, 0xFFFFF000ul) == 10000ul, "[8a] elapsed across the 32-bit wrap = 10000 ms");
        Pci1203LinkWatch w; Run run;
        FakeMon m = Healthy();
        Feed(w, m, 0xFFFFE000ul, 0xFFFFEC80ul, 200, run);
        m.Fail(2, 0x8300000Ful);
        Feed(w, m, 0xFFFFF000ul, 0xFFFFFF00ul, 256, run);   // lost just before the wrap
        Feed(w, m, 0x00000000ul, 0x00001700ul, 256, run);   // 9.98 s after the first lost sample
        check(run.alarm == 0, "[8b] lost across the wrap, under 10 s: no alarm");
        Feed(w, m, 0x00001710ul, 0x00001710ul, 256, run);
        check(run.alarm == 1 && has(run.lastAlarm, "10.0 s"), "[8c] ... and at 10 s after the first lost sample: the alarm");
    }

    std::printf("== %d checks, %d failed ==\n", g_total, g_fail);
    if (g_fail == 0) std::printf("RESULT: ALL PASS\n");
    return g_fail == 0 ? 0 : 1;
}
