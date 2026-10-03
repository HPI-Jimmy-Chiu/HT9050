// =============================================================================
//  test_pci1203_modulecheck.cpp -- EtherCAT/Pci1203ModuleCheck.h, the 1203 module
//  presence check behind WAR16154 (tools/wb_serve.cpp EOF W906_Pci1203ModuleCheckTick).
//
//  //AI(W906-MODCHECK) 20261003: EastSun 1003 (machine engineer's ruling).
//  A FAKE MONITOR (card() / Disabled() / slaveCount() / slave(i), holding the real
//  sample structs), a FAKE CLOCK and a FAKE INI (a Loader lambda over a string) --
//  so every rule is walked without a card and without a file:
//    [1] classification: 402 / SERVOPACK = motor, vacuum excluded (with the REAL
//        Pci1203Vc8IdentityOk), 401 / "Dig. In" / "Dig. Out" = IO, junctions excluded;
//    [2] the ini text round-trips; damaged inis are refused, decimal only;
//    [3] no ini -> one write event, no alarm; truncated / unaddressable / identity
//        missing -> no write, kWriteSkipped at 10 s (not 9.9); an empty scan starts nothing;
//    [4] everything present + OP -> early pass;
//    [5] missing -> nothing at 9.9 s, the alarm at 10 s;
//    [6] not OP -> nothing at 9.9 s, the alarm at 10 s; reaching OP inside the window -> pass;
//    [7] station changed / [8] extra -> the alarm at once (everything listed is in OP);
//    [9] several faults -> exactly ONE alarm, latched until the next generation
//        (OnLinkRestored); link lost -> silent / abandoned; the Loader runs once per generation;
//    [10] the alarm text is CSV / quote safe and bounded.
//  ⚠ WHAT IT DOES NOT PROVE: what the real scan reports for a module that is unplugged
//  and replugged WITHOUT a Refresh (present / position / name are scan-time facts),
//  and that W906_Pci1203ModuleIniCreate's no-replace rename behaves on this volume.
//  Both are on-machine checks. Reads and writes no file.
// =============================================================================
#include "EtherCAT/Pci1203ModuleCheck.h"
#include "EtherCAT/Pci1203Monitor.h"
#include "EtherCAT/Pci1203Vc8.h"

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
//  The fake monitor.
// ---------------------------------------------------------------------------
struct FakeMon {
    Pci1203CardSample               c;
    bool                            dis;
    std::vector<Pci1203SlaveSample> s;
    FakeMon() : dis(false) { s.resize(10); }
    const Pci1203CardSample&  card() const { return c; }
    bool                      Disabled() const { return dis; }
    int                       slaveCount() const { return (int)s.size(); }
    const Pci1203SlaveSample& slave(int i) const { return s[i]; }

    void Station(int i, int ring, int pos, int addr, const char* name, bool profileValid, unsigned short profile,
                 unsigned short state = 0x08)
    {
        Pci1203SlaveSample& x = s[i];
        x.present = true; x.ring = ring; x.position = pos; x.addr = addr; x.unaddressable = false;
        x.infoValid = true; x.name = name; x.profileValid = profileValid; x.profile = profile;
        x.stateValid = true; x.state = state; x.lastError = 0;
        x.vendorId = 0x539; x.productId = 0x100;
    }
    void Gone(int i)                     { s[i] = Pci1203SlaveSample(); }
    void State(int i, unsigned short st) { s[i].stateValid = true; s[i].state = st; }
};

//  The "machine": ring 0 two drives + a junction, ring 1 two DI modules + a VC4 vacuum unit, all in OP.
static FakeMon Healthy()
{
    FakeMon m;
    m.c.open = true;
    m.Station(0, 0, 0, 0x00E, "SGDXS-2R8A30A0", true, 402);
    m.Station(1, 0, 1, 0x000, "SGDXW-2R8AA0A1000 SERVOPACK", false, 0);
    m.Station(2, 1, 0, 0x001, "ECAT-2515 junction", false, 0);
    m.Station(3, 1, 8, 0x050, "ECx-P32-HON 32DI 32 Ch. Dig. In.", false, 0);
    m.Station(4, 1, 9, 0x051, "ECx-P32-HON 32DO 32 Ch. Dig. Out", true, 401);
    m.Station(5, 1, 10, 0x0A0, "ECAT-VC4-ODM1", true, 401);
    m.s[5].vendorId = 0x00494350ul; m.s[5].productId = 0x00A20401ul;
    m.c.slavesFound = 6;
    return m;
}

static bool IsVac(const Pci1203SlaveSample& s) { std::string why; return Pci1203Vc8IdentityOk(s, why); }

static Pci1203ModuleInput In(const FakeMon& m, bool linkLost = false)
{
    return Pci1203ModuleInputFromMonitor(m, IsVac, linkLost);
}

//  A fake ini file: `exists` + `text`, and how many times it was loaded.
struct FakeIni {
    bool        exists;
    std::string text;
    int         loads;
    FakeIni() : exists(false), loads(0) {}
    Pci1203ModuleIniLoad operator()()
    {
        ++loads;
        Pci1203ModuleIniLoad L;
        if (!exists) return L;
        std::string why;
        if (Pci1203ModuleIniParse(text, L.entries, why)) L.status = Pci1203ModuleIniLoad::kOk;
        else { L.status = Pci1203ModuleIniLoad::kBad; L.why = why; }
        return L;
    }
};

struct Run {
    int write, skip, bad, pass, alarm, abandon, start;
    std::string lastAlarm, lastSkip, lastWrite;
    std::vector<Pci1203ModuleProblem> problems;
    unsigned long firstEventAt;
    Run() : write(0), skip(0), bad(0), pass(0), alarm(0), abandon(0), start(0), firstEventAt(0) {}
    int events() const { return write + skip + bad + pass + alarm + abandon; }
};

static void One(Pci1203ModuleCheck& c, const FakeMon& m, FakeIni& ini, unsigned long t, Run& run, bool linkLost = false)
{
    const Pci1203ModuleCheck::Result r = c.Step(In(m, linkLost), t, "2026-10-03 08:00:00", [&ini]() { return ini(); });
    if (!r.startText.empty()) ++run.start;
    if (r.event != Pci1203ModuleCheck::kNone && run.events() == 0) run.firstEventAt = t;
    switch (r.event) {
    case Pci1203ModuleCheck::kWriteIni:     ++run.write; run.lastWrite = r.iniText; break;
    case Pci1203ModuleCheck::kWriteSkipped: ++run.skip;  run.lastSkip = r.text;     break;
    case Pci1203ModuleCheck::kIniBad:       ++run.bad;                              break;
    case Pci1203ModuleCheck::kPass:         ++run.pass;                             break;
    case Pci1203ModuleCheck::kAlarm:        ++run.alarm; run.lastAlarm = r.text; run.problems = r.problems; break;
    case Pci1203ModuleCheck::kAbandoned:    ++run.abandon;                          break;
    default: break;
    }
}

//  t0..t1 inclusive, one sample every stepMs.
static void Feed(Pci1203ModuleCheck& c, const FakeMon& m, FakeIni& ini, unsigned long t0, unsigned long t1,
                 unsigned long stepMs, Run& run, bool linkLost = false)
{
    for (unsigned long t = t0; t <= t1; t += stepMs) One(c, m, ini, t, run, linkLost);
}

//  The ini the healthy machine would record.
static FakeIni HealthyIni()
{
    FakeIni ini;
    ini.exists = true;
    ini.text = Pci1203ModuleIniSerialize(Pci1203ModuleCollect(In(Healthy())), "2026-10-03 07:00:00");
    return ini;
}

static int Count(const std::vector<Pci1203ModuleProblem>& v, int kind)
{
    int n = 0;
    for (std::size_t i = 0; i < v.size(); ++i) if (v[i].kind == kind) ++n;
    return n;
}

int main()
{
    std::printf("== test_pci1203_modulecheck (AI(W906-MODCHECK) 20261003) ==\n");
    check(kPci1203ModCheckMs == 10000, "[0] the window is 10 s, in one place (kPci1203ModCheckMs)");

    // -----------------------------------------------------------------------
    //  [1] classification
    // -----------------------------------------------------------------------
    check(Pci1203ModuleClassify("SGDXS-2R8A30A0", true, 402, false) == kPci1203ModMotor, "[1a] CiA 402 -> motor");
    check(Pci1203ModuleClassify("SGDXW servopack", false, 0, false) == kPci1203ModMotor, "[1b] name SERVOPACK (any case), no profile -> motor");
    check(Pci1203ModuleClassify("odd drive", true, 402, true) == kPci1203ModMotor, "[1c] 402 wins over isVacuum (drive first)");
    check(Pci1203ModuleClassify("ECAT-VC4-ODM1", true, 401, true) == kPci1203ModNone, "[1d] vacuum unit excluded even with profile 401");
    check(Pci1203ModuleClassify("whatever", true, 401, false) == kPci1203ModIo, "[1e] CiA 401 -> IO");
    check(Pci1203ModuleClassify("ECx-P32-HON 32DI 32 Ch. Dig. In.", false, 0, false) == kPci1203ModIo, "[1f] name Dig. In -> IO");
    check(Pci1203ModuleClassify("ECx 32DO Dig. Out", false, 0, false) == kPci1203ModIo, "[1g] name Dig. Out -> IO");
    check(Pci1203ModuleClassify("ECAT-2515 junction", false, 0, false) == kPci1203ModNone, "[1h] junction / unknown -> excluded");
    {
        const Pci1203ModuleInput in = In(Healthy());
        int io = 0, mo = 0, none = 0;
        for (std::size_t i = 0; i < in.stations.size(); ++i)
            (in.stations[i].kind == kPci1203ModIo ? io : in.stations[i].kind == kPci1203ModMotor ? mo : none)++;
        check(in.stations.size() == 6 && mo == 2 && io == 2 && none == 2,
              "[1i] adapter over the fake monitor: 2 motor, 2 IO, junction + VC4 excluded (REAL Pci1203Vc8IdentityOk)");
        check(Pci1203ModuleCollect(in).size() == 4, "[1j] Collect records the 4 IO / motor modules only");
    }

    // -----------------------------------------------------------------------
    //  [2] ini text
    // -----------------------------------------------------------------------
    {
        const std::vector<Pci1203ModuleEntry> a = Pci1203ModuleCollect(In(Healthy()));
        const std::string t = Pci1203ModuleIniSerialize(a, "2026-10-03 07:00:00");
        std::vector<Pci1203ModuleEntry> b; std::string why;
        bool same = Pci1203ModuleIniParse(t, b, why) && b.size() == a.size();
        for (std::size_t i = 0; same && i < a.size(); ++i)
            same = a[i].kind == b[i].kind && a[i].ring == b[i].ring && a[i].position == b[i].position &&
                   a[i].station == b[i].station && a[i].name == b[i].name &&
                   a[i].vendorId == b[i].vendorId && a[i].productId == b[i].productId;
        check(same, "[2a] serialize -> parse round-trips every field");
        check(has(t, "Count=4") && has(t, "Station=80") && has(t, "Kind=Motor"), "[2b] decimal station, Count, Kind in the text");
        check(Pci1203ModuleIniParse("\xEF\xBB\xBF" + t, b, why) && b.size() == 4, "[2c] a UTF-8 BOM (an editor's save) is accepted");

        std::string bad1 = t; bad1.replace(bad1.find("Count=4"), 7, "Count=5");
        check(!Pci1203ModuleIniParse(bad1, b, why) && has(why, "Count") && b.empty(), "[2d] Count not matching the sections -> refused");
        std::string bad2 = t; { const std::size_t k = bad2.find("Name="); bad2.replace(k, bad2.find('\r', k) - k, "Name="); }
        check(!Pci1203ModuleIniParse(bad2, b, why), "[2e] an empty Name -> refused");
        check(!Pci1203ModuleIniParse("", b, why) && !Pci1203ModuleIniParse("[Pci1203Modules]\r\nCount=0\r\n", b, why),
              "[2f] empty file / no module -> refused");
        std::string oct = t; oct.replace(oct.find("Station=80"), 10, "Station=080");
        check(Pci1203ModuleIniParse(oct, b, why) && b[2].station == 80, "[2g] a hand-edited 080 reads as decimal 80, not octal");
        std::string bad3 = t; bad3.replace(bad3.find("Ring=0"), 6, "Ring=x");
        check(!Pci1203ModuleIniParse(bad3, b, why), "[2h] a non-number Ring -> refused");
    }

    // -----------------------------------------------------------------------
    //  [3] first connect: write / skip
    // -----------------------------------------------------------------------
    {
        Pci1203ModuleCheck c; FakeIni ini; Run run;
        FakeMon m = Healthy();
        Feed(c, m, ini, 1000, 31000, 200, run);
        check(run.write == 1 && run.alarm == 0 && run.events() == 1 && run.firstEventAt == 1000,
              "[3a] no ini: ONE write event at the first sample, no alarm, nothing after");
        check(has(run.lastWrite, "Count=4") && ini.loads == 1, "[3b] the write carries the 4 modules; the ini was loaded once");
    }
    {
        Pci1203ModuleCheck c; FakeIni ini; Run run;
        FakeMon m = Healthy(); m.c.scanTruncated = true;
        Feed(c, m, ini, 1000, 10900, 100, run);                // 9.9 s after the start
        check(run.events() == 0, "[3c] truncated scan: nothing written, nothing said at 9.9 s");
        One(c, m, ini, 11000, run);
        check(run.skip == 1 && run.write == 0 && run.alarm == 0 && has(run.lastSkip, "truncated"),
              "[3d] truncated scan: kWriteSkipped at 10 s, with the reason, no alarm");
        Feed(c, m, ini, 11200, 40000, 200, run);
        check(run.events() == 1, "[3e] ... and latched (no retry until the next connect)");
        c.OnLinkRestored(); m.c.scanTruncated = false;
        One(c, m, ini, 41000, run);
        check(run.write == 1, "[3f] the next connect with a complete scan writes");
    }
    {
        Pci1203ModuleCheck c; FakeIni ini; Run run;
        FakeMon m = Healthy(); m.s[6] = m.s[3]; m.s[6].unaddressable = true;
        Feed(c, m, ini, 0, 12000, 200, run);
        check(run.write == 1 && run.skip == 0, "[3g] AI(W906-MODCHECK-2) 20261003: an unaddressable station (a ring junction, EastSun) no longer blocks the write");
    }
    {
        Pci1203ModuleCheck c; FakeIni ini; Run run;
        FakeMon m = Healthy(); m.s[3].infoValid = false;
        Feed(c, m, ini, 0, 12000, 200, run);
        check(run.write == 0 && run.skip == 1 && has(run.lastSkip, "identity"), "[3h] a station without its identity -> not written");
    }
    {
        Pci1203ModuleCheck c; FakeIni ini; Run run;
        FakeMon m; m.c.open = true; m.c.slavesFound = 0;
        Feed(c, m, ini, 0, 30000, 200, run);
        check(run.events() == 0 && run.start == 0 && ini.loads == 0 && c.gen() == 0,
              "[3i] the scan found 0: no generation, no write, the ini not even read");
        FakeMon dis = Healthy(); dis.dis = true;
        Feed(c, dis, ini, 30200, 60000, 200, run);
        check(run.events() == 0 && c.gen() == 0, "[3j] monitor disabled: no generation");
    }

    // -----------------------------------------------------------------------
    //  [4] all present + OP -> early pass
    // -----------------------------------------------------------------------
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        Feed(c, Healthy(), ini, 5000, 65000, 200, run);
        check(run.pass == 1 && run.alarm == 0 && run.events() == 1 && run.firstEventAt == 5000,
              "[4a] everything present, at its station, in OP: pass at the first sample, then nothing");
        check(ini.loads == 1 && c.gen() == 1, "[4b] one generation, the ini loaded once");
    }

    // -----------------------------------------------------------------------
    //  [5] missing
    // -----------------------------------------------------------------------
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        FakeMon m = Healthy(); m.Gone(3); m.c.slavesFound = 5;
        Feed(c, m, ini, 1000, 10900, 100, run);
        check(run.events() == 0, "[5a] a listed module missing: nothing at 9.9 s");
        One(c, m, ini, 11000, run);
        check(run.alarm == 1 && Count(run.problems, Pci1203ModuleProblem::kMissing) == 1 && has(run.lastAlarm, "missing r1 pos 8 st 80"),
              "[5b] ... the alarm at 10 s, naming ring / position / station");
    }

    // -----------------------------------------------------------------------
    //  [6] not OP
    // -----------------------------------------------------------------------
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        FakeMon m = Healthy(); m.State(0, 0x14);              // SAFEOP + ERR
        Feed(c, m, ini, 1000, 10900, 100, run);
        check(run.events() == 0, "[6a] found but SAFEOP: nothing at 9.9 s");
        One(c, m, ini, 11000, run);
        check(run.alarm == 1 && Count(run.problems, Pci1203ModuleProblem::kNotOp) == 1 && has(run.lastAlarm, "SAFEOP+ERR"),
              "[6b] ... the alarm at 10 s with the state");
    }
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        FakeMon m = Healthy(); m.State(4, 0x02);              // PREOP while the ring comes up
        Feed(c, m, ini, 1000, 5000, 200, run);
        m.State(4, 0x08);
        Feed(c, m, ini, 5200, 30000, 200, run);
        check(run.pass == 1 && run.alarm == 0 && run.firstEventAt == 5200, "[6c] reaching OP inside the window -> pass, no alarm");
    }

    // -----------------------------------------------------------------------
    //  [7] station changed / [8] extra -> at once
    // -----------------------------------------------------------------------
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        FakeMon m = Healthy(); m.s[3].addr = 0x055;           // the dial turned
        Feed(c, m, ini, 1000, 30000, 200, run);
        check(run.alarm == 1 && run.firstEventAt == 1000 && Count(run.problems, Pci1203ModuleProblem::kStationChanged) == 1 &&
              Count(run.problems, Pci1203ModuleProblem::kMissing) == 0 && has(run.lastAlarm, "st 80 -> 85"),
              "[7] same ring + position + name, other station: station changed, alarm at once (not missing + extra)");
    }
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        FakeMon m = Healthy(); m.Station(6, 1, 11, 0x052, "ECx-P32-HON 32DI 32 Ch. Dig. In.", false, 0); m.c.slavesFound = 7;
        m.Station(7, 1, 12, 0x0A1, "ECAT-VC4-ODM1", true, 401); m.s[7].vendorId = 0x00494350ul;   // a vacuum unit: never extra
        Feed(c, m, ini, 1000, 30000, 200, run);
        check(run.alarm == 1 && run.firstEventAt == 1000 && run.problems.size() == 1 &&
              Count(run.problems, Pci1203ModuleProblem::kExtra) == 1 && has(run.lastAlarm, "extra IO r1 pos 11 st 82"),
              "[8] an IO module not in the ini: extra, alarm at once; the added vacuum unit is not counted");
    }

    // -----------------------------------------------------------------------
    //  [9] several faults -> one alarm; latch; generations; link lost
    // -----------------------------------------------------------------------
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        FakeMon m = Healthy();
        m.Gone(0); m.State(1, 0x04); m.s[3].addr = 0x057; m.Station(6, 1, 11, 0x052, "x Dig. Out", false, 0);
        Feed(c, m, ini, 1000, 61000, 200, run);
        check(run.alarm == 1 && run.events() == 1 && run.problems.size() == 4,
              "[9a] missing + not OP + station changed + extra: exactly ONE alarm in 60 s, carrying all four");
        check(has(run.lastAlarm, "4 problems"), "[9b] the alarm text counts them");
        c.OnLinkRestored();
        Feed(c, m, ini, 61200, 80000, 200, run);
        check(run.alarm == 2 && ini.loads == 2 && c.gen() == 2, "[9c] link restored -> a new generation -> the alarm once more");

        c.OnLinkRestored();
        FakeMon good = Healthy();
        Feed(c, good, ini, 80200, 82000, 200, run, true);      // restore reported but the watch says lost again
        check(run.start == 2 && c.gen() == 2, "[9d] link lost: no generation starts");
        Feed(c, good, ini, 82200, 90000, 200, run);
        check(run.pass == 1 && c.gen() == 3, "[9e] link back (not lost): the pending restore starts generation 3 -> pass");
    }
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        FakeMon m = Healthy(); m.Gone(4);
        Feed(c, m, ini, 1000, 5000, 200, run);
        Feed(c, m, ini, 5200, 30000, 200, run, true);           // lost in the middle of the window
        check(run.abandon == 1 && run.alarm == 0, "[9f] the link lost while checking: abandoned, no alarm (the link watch owns it)");
        Feed(c, m, ini, 30200, 60000, 200, run);
        check(run.alarm == 0 && c.gen() == 1, "[9g] ... and nothing until the restore is reported");
        c.OnLinkRestored();
        Feed(c, m, ini, 60200, 75000, 200, run);
        check(run.alarm == 1, "[9h] restore -> re-checked -> the missing module alarms");
    }
    {
        Pci1203ModuleCheck c; FakeIni ini; ini.exists = true; ini.text = "garbage"; Run run;
        Feed(c, Healthy(), ini, 0, 30000, 200, run);
        check(run.bad == 1 && run.write == 0 && run.alarm == 0 && run.events() == 1, "[9i] an unreadable ini: kIniBad once, never rewritten, no alarm");
    }

    // -----------------------------------------------------------------------
    //  [10] safe text
    // -----------------------------------------------------------------------
    {
        Pci1203ModuleCheck c; FakeIni ini = HealthyIni(); Run run;
        FakeMon m = Healthy();
        for (int i = 0; i < 4; ++i) {
            char nm[96]; std::snprintf(nm, sizeof(nm), "Weird, \"quoted\" 'name' %d Dig. In with a very long tail text", i);
            m.Station(6 + i, 1, 20 + i, 0x060 + i, nm, false, 0);
        }
        m.Gone(0);
        Feed(c, m, ini, 0, 12000, 200, run);
        check(run.alarm == 1 && run.lastAlarm.find(',') == std::string::npos && run.lastAlarm.find('"') == std::string::npos &&
              run.lastAlarm.find('\'') == std::string::npos && run.lastAlarm.size() <= (std::size_t)kPci1203ModCheckTextMax &&
              has(run.lastAlarm, "(+1 more)"),
              "[10] 5 problems: no comma / quote, <= 400 bytes, the first 4 named and the rest counted");
    }

    std::printf("== %d checks, %d failed ==\n", g_total, g_fail);
    std::printf(g_fail == 0 ? "RESULT: ALL PASS\n" : "RESULT: FAIL\n");
    return g_fail == 0 ? 0 : 1;
}
