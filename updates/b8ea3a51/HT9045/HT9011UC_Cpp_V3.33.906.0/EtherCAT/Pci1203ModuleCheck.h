// =============================================================================
//  EtherCAT/Pci1203ModuleCheck.h -- "1203 模組在位檢查": every IO / motor module
//  recorded on the first connect must be there, in OP, at the same station, on
//  every later connect.
//
//  //AI(W906-MODCHECK) 20261003: rules decided by the machine engineer (EastSun 1003):
//    * FIRST CONNECT: record every IO module and every motor (drive) module --
//      name, ring, position, station -- into D:\HT9045\system\Pci1203Modules.ini.
//      Create it ONLY when it does not exist; never overwrite. Do not write when the
//      scan is truncated, a station is unaddressable, or the scan found nothing.
//    * AFTERWARDS ONLY READ IT. On every connect (the boot connect and every link
//      restore), within 10 s:
//        listed module not found                         -> missing
//        found but not in OP within 10 s                  -> not OP
//        same ring + position + name, different station   -> station changed
//        an IO / motor module the ini does not list       -> extra
//      ONE alarm per connect (the host raises golden-shaped WAR16154 with the text).
//      Nothing while the link watch (EtherCAT/Pci1203LinkWatch.h) reports link lost.
//    This header is the DECISION (pure, header-only, no vendor header, no
//    <windows.h>, no clock, no file I/O). The host (tools/wb_serve.cpp EOF,
//    W906_Pci1203ModuleCheckTick) feeds it one sample after every monitor Poll(),
//    loads the ini when asked, writes the ini when asked, and raises the alarm.
//
//  ## CLASSIFICATION (Pci1203ModuleClassify -- takes name / profile / isVacuum so it stays pure)
//    motor  = CiA profile 402, else the name contains "SERVOPACK"
//             (the same rule as EtherCAT/Pci1203IoRoute.cpp StationIsDrive)
//    excluded = the station passes Pci1203Vc8IdentityOk (EtherCAT/Pci1203Vc8.h, ECAT-VC8 / VC4
//             vacuum units) -- the host computes isVacuum, this header never sees the table
//    IO     = CiA profile 401, else the name contains "Dig. In" / "Dig. Out"
//    anything else (ECAT-2515 junctions, unknown modules) is excluded: not recorded, not checked.
//
//  ## "STATION"
//    = Pci1203SlaveSample::addr, the SlaveIP every address-keyed call uses (StationIsDrive, the
//    VC8 route and the 1203 page's messages all call that "站"). Measured 20260910 it tracks the
//    module's dial (alias), so turning a dial shows up here as "station changed".
//    Unaddressable stations are left out of the sample (as the link watch does): their addr is
//    their twin's. A recorded module that became unaddressable therefore reads as "missing".
//
//  ## CONNECT GENERATIONS
//    A generation starts (a) the first time the card is open, not disabled, the scan found
//    stations and the link is not lost, and (b) after every OnLinkRestored() (the host calls it
//    when the link watch reports kRestored), at the first sample that satisfies the same test.
//    At the start it asks the host to load the ini once (the Loader functor), then:
//      no ini      -> as soon as the scan is writable: kWriteIni (the text to create); if it does
//                     not become writable within 10 s: kWriteSkipped (the reason). No alarm.
//      bad ini     -> kIniBad (the reason) once. No alarm (see the host: open question).
//      ini read    -> every sample: early kPass when every listed module is found, at its
//                     station, in OP, and nothing extra; early kAlarm when every listed module is
//                     found in OP but a station changed / an extra module exists; otherwise at
//                     kPci1203ModCheckMs: kPass if no problem, else kAlarm.
//    A generation produces at most ONE terminal event (kWriteIni / kWriteSkipped / kIniBad /
//    kPass / kAlarm / kAbandoned) and then latches until the next generation.
//    The link lost while a generation runs -> kAbandoned (no alarm; the link watch owns that
//    fault, WAR16152); the restore starts a new one.
// =============================================================================
#ifndef HT9045_ETHERCAT_PCI1203MODULECHECK_H
#define HT9045_ETHERCAT_PCI1203MODULECHECK_H

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "EtherCAT/Pci1203LinkWatch.h"   // Pci1203LinkSafeText / Pci1203LinkElapsedMs / Pci1203LinkStateName (pure)

namespace ht9045 {

//  ★ THE ONE PLACE the 10 s lives (EastSun 1003「10 秒內」).
enum { kPci1203ModCheckMs = 10000 };
//  How many problems the alarm text names one by one; the rest are counted.
enum { kPci1203ModCheckListMax = 4 };
//  errPart of golden ShowErrorMessage (event log / HANDLER LOG csv): short, CSV / quote safe.
enum { kPci1203ModCheckTextMax = 400 };
//  A module name inside the alarm text is clipped to this many bytes.
enum { kPci1203ModCheckNameMax = 28 };

enum Pci1203ModuleKind { kPci1203ModNone = 0, kPci1203ModIo = 1, kPci1203ModMotor = 2 };

inline const char* Pci1203ModuleKindName(int kind)
{
    return kind == kPci1203ModMotor ? "Motor" : kind == kPci1203ModIo ? "IO" : "None";
}

//  Case-insensitive substring (ASCII), as Pci1203IoRoute.cpp NameSaysServopack / Pci1203Vc8NameHas.
inline bool Pci1203ModNameHas(const std::string& name, const char* has)
{
    if (!has || !*has) return false;
    std::size_t m = 0;
    while (has[m]) ++m;
    for (std::size_t i = 0; i + m <= name.size(); ++i) {
        std::size_t k = 0;
        for (; k < m; ++k) {
            char a = name[i + k], b = has[k];
            if (a >= 'a' && a <= 'z') a = (char)(a - 'a' + 'A');
            if (b >= 'a' && b <= 'z') b = (char)(b - 'a' + 'A');
            if (a != b) break;
        }
        if (k == m) return true;
    }
    return false;
}

//  Trim spaces / tabs / CR / LF at both ends (the ini stores and compares the trimmed name).
inline std::string Pci1203ModTrim(const std::string& s)
{
    std::size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) --b;
    return s.substr(a, b - a);
}

//  The engineer's rule, in its order: drive first (never argued with), vacuum excluded, then IO.
inline int Pci1203ModuleClassify(const std::string& name, bool profileValid, unsigned short profile, bool isVacuum)
{
    if ((profileValid && profile == 402) || Pci1203ModNameHas(name, "SERVOPACK")) return kPci1203ModMotor;
    if (isVacuum) return kPci1203ModNone;
    if ((profileValid && profile == 401) || Pci1203ModNameHas(name, "Dig. In") || Pci1203ModNameHas(name, "Dig. Out"))
        return kPci1203ModIo;
    return kPci1203ModNone;
}

// One module as the ini records it.
struct Pci1203ModuleEntry {
    int           kind;        // kPci1203ModIo / kPci1203ModMotor
    int           ring;
    int           position;    // ADV_SLAVE_INFO.Position
    int           station;     // Pci1203SlaveSample::addr
    std::string   name;        // trimmed ADV_SLAVE_INFO.Name
    unsigned long vendorId;    // informational (not compared)
    unsigned long productId;   // informational (not compared)
    Pci1203ModuleEntry() : kind(kPci1203ModNone), ring(-1), position(-1), station(-1), vendorId(0), productId(0) {}
};

// One scanned (present, addressable) station as this check sees it.
struct Pci1203ModuleStation {
    int            ring;
    int            position;
    int            station;
    bool           infoValid;  // ADV_SLAVE_INFO came back (name / position / IDs are real)
    std::string    name;       // trimmed
    unsigned long  vendorId;
    unsigned long  productId;
    bool           readOk;     // this Poll's state read succeeded
    unsigned short state;      // raw EC_SLAVE_STATE_*
    int            kind;       // Pci1203ModuleClassify
    Pci1203ModuleStation()
        : ring(-1), position(-1), station(-1), infoValid(false), vendorId(0), productId(0)
        , readOk(false), state(0), kind(kPci1203ModNone) {}
};

// One sample: what the monitor says right after a Poll(), plus the link watch's verdict.
struct Pci1203ModuleInput {
    bool        open;            // card().open
    bool        disabled;        // Disabled()
    int         slavesFound;     // card().slavesFound
    bool        scanTruncated;   // card().scanTruncated
    int         unaddressable;   // present stations no address reaches (left out of `stations`)
    bool        linkLost;        // the link watch says lost right now (Pci1203LinkWatch::lost())
    std::vector<Pci1203ModuleStation> stations;   // present + addressable, scan order
    Pci1203ModuleInput() : open(false), disabled(false), slavesFound(0), scanTruncated(false), unaddressable(0), linkLost(false) {}
};

inline bool Pci1203ModStationOp(const Pci1203ModuleStation& s)
{
    return s.readOk && (s.state & 0x0Fu) == 0x08u;           // EC_SLAVE_STATE_OP, as Pci1203LinkStationUp
}

//  The monitor -> sample adapter. A template, as Pci1203LinkInputFromMonitor, so the test can hand
//  it a fake with card() / Disabled() / slaveCount() / slave(i). `isVacuum(sample)` is the host's
//  Pci1203Vc8IdentityOk wrapper. Reads only what Poll() / the scan already stored.
template <class Mon, class IsVac>
inline Pci1203ModuleInput Pci1203ModuleInputFromMonitor(const Mon& mon, IsVac isVacuum, bool linkLost)
{
    Pci1203ModuleInput in;
    in.open          = mon.card().open;
    in.disabled      = mon.Disabled();
    in.slavesFound   = mon.card().slavesFound;
    in.scanTruncated = mon.card().scanTruncated;
    in.linkLost      = linkLost;
    const int n = mon.slaveCount();
    for (int i = 0; i < n; ++i) {
        const auto& s = mon.slave(i);
        if (!s.present) continue;
        if (s.unaddressable) { ++in.unaddressable; continue; }
        Pci1203ModuleStation st;
        st.ring      = s.ring;
        st.position  = s.position;
        st.station   = s.addr;
        st.infoValid = s.infoValid;
        st.name      = Pci1203ModTrim(s.name);
        st.vendorId  = s.vendorId;
        st.productId = s.productId;
        st.readOk    = s.stateValid;
        st.state     = s.state;
        st.kind      = Pci1203ModuleClassify(st.name, s.profileValid, s.profile, isVacuum(s));
        in.stations.push_back(st);
    }
    return in;
}

// ---------------------------------------------------------------------------
//  The ini (text). Plain golden-style sections, one per module; the host writes it once.
// ---------------------------------------------------------------------------
inline std::string Pci1203ModuleIniSerialize(const std::vector<Pci1203ModuleEntry>& e, const std::string& created)
{
    std::string t;
    t += "; Pci1203Modules.ini -- PCI-1203 IO / motor modules recorded on the first connect.\r\n";
    t += "; AI(W906-MODCHECK) 20261003: written once by wb_serve when this file did not exist; never overwritten.\r\n";
    t += "; Every later connect compares the ring against this list (missing / not OP / station changed / extra -> WAR16154).\r\n";
    t += "; To re-learn the machine after a deliberate hardware change: delete this file and reconnect.\r\n";
    t += "[Pci1203Modules]\r\n";
    t += "Version=1\r\n";
    char b[96];
    std::snprintf(b, sizeof(b), "Count=%d\r\n", (int)e.size());
    t += b;
    t += "Created=" + Pci1203ModTrim(created) + "\r\n";
    for (std::size_t i = 0; i < e.size(); ++i) {
        std::snprintf(b, sizeof(b), "[Module%d]\r\n", (int)(i + 1));                     t += b;
        t += std::string("Kind=") + Pci1203ModuleKindName(e[i].kind) + "\r\n";
        std::snprintf(b, sizeof(b), "Ring=%d\r\n", e[i].ring);                            t += b;
        std::snprintf(b, sizeof(b), "Position=%d\r\n", e[i].position);                    t += b;
        std::snprintf(b, sizeof(b), "Station=%d\r\n", e[i].station);                      t += b;
        std::string nm = e[i].name;                                                       // one line, always
        for (std::size_t k = 0; k < nm.size(); ++k) if (nm[k] == '\r' || nm[k] == '\n') nm[k] = ' ';
        t += "Name=" + Pci1203ModTrim(nm) + "\r\n";
        std::snprintf(b, sizeof(b), "VendorID=0x%08lX\r\n", e[i].vendorId);               t += b;
        std::snprintf(b, sizeof(b), "ProductID=0x%08lX\r\n", e[i].productId);             t += b;
    }
    return t;
}

inline bool Pci1203ModParseInt(const std::string& v, int* out)
{
    const std::string s = Pci1203ModTrim(v);
    if (s.empty()) return false;
    char* end = 0;
    const long x = std::strtol(s.c_str(), &end, 10);   // decimal only: a hand-edited "080" must not read as octal
    if (!end || *end != '\0') return false;
    *out = (int)x;
    return true;
}

//  Text -> entries. False (with `why`) for anything the check could misread: no [Pci1203Modules],
//  Count missing / not matching the module sections, a module without Ring / Position / Station /
//  Name / a known Kind, or no module at all.
inline bool Pci1203ModuleIniParse(const std::string& text, std::vector<Pci1203ModuleEntry>& out, std::string& why)
{
    out.clear();
    why.clear();
    bool head = false, inHead = false;
    int count = -1;
    struct Got { bool kind, ring, pos, st, name; Got() : kind(false), ring(false), pos(false), st(false), name(false) {} };
    std::vector<Got> got;
    std::size_t p = 0;
    int line = 0;
    std::size_t start = 0;
    if (text.size() >= 3 && (unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF) start = 3;
    p = start;
    while (p <= text.size()) {
        std::size_t q = text.find('\n', p);
        if (q == std::string::npos) q = text.size();
        const std::string ln = Pci1203ModTrim(text.substr(p, q - p));
        ++line;
        p = q + 1;
        if (ln.empty() || ln[0] == ';' || ln[0] == '#') { if (q == text.size()) break; continue; }
        if (ln[0] == '[') {
            const std::size_t e = ln.find(']');
            const std::string sec = e == std::string::npos ? std::string() : ln.substr(1, e - 1);
            inHead = false;
            if (sec == "Pci1203Modules") { head = true; inHead = true; }
            else if (sec.compare(0, 6, "Module") == 0) { out.push_back(Pci1203ModuleEntry()); got.push_back(Got()); }
            else { char b[64]; std::snprintf(b, sizeof(b), "line %d: unknown section", line); why = b; out.clear(); return false; }
            if (q == text.size()) break;
            continue;
        }
        const std::size_t eq = ln.find('=');
        if (eq == std::string::npos) { char b[64]; std::snprintf(b, sizeof(b), "line %d: no '='", line); why = b; out.clear(); return false; }
        const std::string k = Pci1203ModTrim(ln.substr(0, eq));
        const std::string v = Pci1203ModTrim(ln.substr(eq + 1));
        bool ok = true;
        if (inHead) {
            if (k == "Count") ok = Pci1203ModParseInt(v, &count);
        } else if (!out.empty()) {
            Pci1203ModuleEntry& m = out.back();
            Got& g = got.back();
            if      (k == "Kind")      { m.kind = v == "Motor" ? kPci1203ModMotor : v == "IO" ? kPci1203ModIo : kPci1203ModNone; g.kind = m.kind != kPci1203ModNone; ok = g.kind; }
            else if (k == "Ring")      { ok = Pci1203ModParseInt(v, &m.ring);     g.ring = ok; }
            else if (k == "Position")  { ok = Pci1203ModParseInt(v, &m.position); g.pos  = ok; }
            else if (k == "Station")   { ok = Pci1203ModParseInt(v, &m.station);  g.st   = ok; }
            else if (k == "Name")      { m.name = v; g.name = !v.empty(); ok = g.name; }
            else if (k == "VendorID")  { m.vendorId  = std::strtoul(v.c_str(), 0, 0); }   // informational: never fails the parse
            else if (k == "ProductID") { m.productId = std::strtoul(v.c_str(), 0, 0); }
        }
        if (!ok) { char b[96]; std::snprintf(b, sizeof(b), "line %d: bad value for %s", line, k.c_str()); why = b; out.clear(); return false; }
        if (q == text.size()) break;
    }
    if (!head)              { why = "no [Pci1203Modules] section"; out.clear(); return false; }
    if (out.empty())        { why = "no module listed"; return false; }
    if (count != (int)out.size()) {
        char b[80]; std::snprintf(b, sizeof(b), "Count=%d but %d module sections", count, (int)out.size());
        why = b; out.clear(); return false;
    }
    for (std::size_t i = 0; i < got.size(); ++i) {
        if (!(got[i].kind && got[i].ring && got[i].pos && got[i].st && got[i].name)) {
            char b[80]; std::snprintf(b, sizeof(b), "[Module%d] lacks Kind / Ring / Position / Station / Name", (int)(i + 1));
            why = b; out.clear(); return false;
        }
    }
    return true;
}

//  The entries the first connect records: every IO / motor station of the sample, scan order.
inline std::vector<Pci1203ModuleEntry> Pci1203ModuleCollect(const Pci1203ModuleInput& in)
{
    std::vector<Pci1203ModuleEntry> v;
    for (std::size_t i = 0; i < in.stations.size(); ++i) {
        const Pci1203ModuleStation& s = in.stations[i];
        if (s.kind == kPci1203ModNone) continue;
        Pci1203ModuleEntry e;
        e.kind = s.kind; e.ring = s.ring; e.position = s.position; e.station = s.station;
        e.name = s.name; e.vendorId = s.vendorId; e.productId = s.productId;
        v.push_back(e);
    }
    return v;
}

//  Why the first-connect write must NOT happen now ("" = writable).
inline std::string Pci1203ModuleWriteBlock(const Pci1203ModuleInput& in)
{
    char b[96];
    if (!in.open || in.disabled)  return "card not open / monitor disabled";
    if (in.scanTruncated)         return "the scan was truncated (wall-clock budget)";
    if (in.slavesFound <= 0 || in.stations.empty()) return "the scan found no station";
    //AI(W906-MODCHECK-2) 20261003: EastSun「站號一定不衝突 不然你不會讀得到 你看到衝突的 是中繼站 你單純看馬達就好」-- the unaddressable
    //  stations are the ring's junctions (no IO / motor module), so they no longer block the first-connect write (17:19 it was refused for
    //  "3 station(s) unaddressable"). They were never listed anyway (InputFromMonitor skips them).
    (void)b;
    for (std::size_t i = 0; i < in.stations.size(); ++i) {
        const Pci1203ModuleStation& s = in.stations[i];
        if (!s.infoValid || s.position < 0) {
            std::snprintf(b, sizeof(b), "ring %d station %d: identity (ADV_SLAVE_INFO) not read", s.ring, s.station);
            return b;
        }
    }
    if (Pci1203ModuleCollect(in).empty()) return "the scan found no IO / motor module";
    return std::string();
}

struct Pci1203ModuleProblem {
    enum Kind { kMissing = 0, kNotOp, kStationChanged, kExtra };
    int                 kind;
    Pci1203ModuleEntry  module;      // kExtra: what was found; others: the ini entry
    int                 foundStation; // kStationChanged / kNotOp: the station found now
    bool                readOk;      // kNotOp
    unsigned short      state;       // kNotOp
    Pci1203ModuleProblem() : kind(kMissing), foundStation(-1), readOk(false), state(0) {}
};

//  ini vs sample. `allFoundOp` = every listed module was found (ring + position + name) and is in OP.
inline std::vector<Pci1203ModuleProblem> Pci1203ModuleDiff(const std::vector<Pci1203ModuleEntry>& ini,
                                                           const Pci1203ModuleInput& in, bool* allFoundOp)
{
    std::vector<Pci1203ModuleProblem> out;
    std::vector<bool> used(in.stations.size(), false);
    bool all = true;
    for (std::size_t i = 0; i < ini.size(); ++i) {
        const Pci1203ModuleEntry& e = ini[i];
        int hit = -1;
        for (std::size_t k = 0; k < in.stations.size(); ++k) {
            const Pci1203ModuleStation& s = in.stations[k];
            if (used[k] || !s.infoValid) continue;
            if (s.ring == e.ring && s.position == e.position && s.name == e.name) {
                if (hit < 0 || s.station == e.station) hit = (int)k;      // prefer the exact station
                if (s.station == e.station) break;
            }
        }
        if (hit < 0) {
            Pci1203ModuleProblem p; p.kind = Pci1203ModuleProblem::kMissing; p.module = e;
            out.push_back(p); all = false; continue;
        }
        used[hit] = true;
        const Pci1203ModuleStation& s = in.stations[hit];
        if (s.station != e.station) {
            Pci1203ModuleProblem p; p.kind = Pci1203ModuleProblem::kStationChanged; p.module = e; p.foundStation = s.station;
            out.push_back(p);
        }
        if (!Pci1203ModStationOp(s)) {
            Pci1203ModuleProblem p; p.kind = Pci1203ModuleProblem::kNotOp; p.module = e; p.foundStation = s.station;
            p.readOk = s.readOk; p.state = s.state;
            out.push_back(p); all = false;
        }
    }
    for (std::size_t k = 0; k < in.stations.size(); ++k) {
        const Pci1203ModuleStation& s = in.stations[k];
        if (used[k] || s.kind == kPci1203ModNone) continue;
        Pci1203ModuleProblem p; p.kind = Pci1203ModuleProblem::kExtra;
        p.module.kind = s.kind; p.module.ring = s.ring; p.module.position = s.position; p.module.station = s.station;
        p.module.name = s.name; p.module.vendorId = s.vendorId; p.module.productId = s.productId;
        out.push_back(p);
    }
    if (allFoundOp) *allFoundOp = all;
    return out;
}

inline std::string Pci1203ModuleProblemText(const Pci1203ModuleProblem& p)
{
    const std::string nm = Pci1203LinkSafeText(p.module.name, (std::size_t)kPci1203ModCheckNameMax);
    char b[192];
    switch (p.kind) {
    case Pci1203ModuleProblem::kMissing:
        std::snprintf(b, sizeof(b), "missing r%d pos %d st %d %s", p.module.ring, p.module.position, p.module.station, nm.c_str());
        break;
    case Pci1203ModuleProblem::kNotOp:
        if (p.readOk)
            std::snprintf(b, sizeof(b), "not OP r%d pos %d st %d %s state 0x%02X %s%s", p.module.ring, p.module.position,
                          p.foundStation, nm.c_str(), (unsigned)p.state, Pci1203LinkStateName(p.state), (p.state & 0x10u) ? "+ERR" : "");
        else
            std::snprintf(b, sizeof(b), "not OP r%d pos %d st %d %s state read failed", p.module.ring, p.module.position,
                          p.foundStation, nm.c_str());
        break;
    case Pci1203ModuleProblem::kStationChanged:
        std::snprintf(b, sizeof(b), "station changed r%d pos %d %s st %d -> %d", p.module.ring, p.module.position, nm.c_str(),
                      p.module.station, p.foundStation);
        break;
    default:
        std::snprintf(b, sizeof(b), "extra %s r%d pos %d st %d %s", Pci1203ModuleKindName(p.module.kind), p.module.ring,
                      p.module.position, p.module.station, nm.c_str());
        break;
    }
    return b;
}

// What the host's Loader returns (it is asked once per generation).
struct Pci1203ModuleIniLoad {
    enum Status { kMissing = 0, kOk = 1, kBad = 2 };
    int                             status;
    std::vector<Pci1203ModuleEntry> entries;   // kOk
    std::string                     why;       // kBad
    Pci1203ModuleIniLoad() : status(kMissing) {}
};

class Pci1203ModuleCheck {
public:
    enum Event {
        kNone = 0,
        kWriteIni,      // first connect, scan writable: create the ini with `iniText` (CREATE_NEW; never overwrite)
        kWriteSkipped,  // first connect, scan never writable within the window: `text` = why; nothing written
        kIniBad,        // the ini exists but cannot be read safely: `text` = why
        kPass,          // every listed module present, at its station, in OP; nothing extra
        kAlarm,         // raise WAR16154 with `text` NOW (once per generation)
        kAbandoned      // the link was lost while this generation ran: no verdict
    };
    struct Result {
        Event         event;
        unsigned long gen;          // the generation this result belongs to (0 = none yet)
        std::string   startText;    // non-empty on the sample that started a generation (a log line)
        std::string   text;         // the event's text (kAlarm: the alarm's errPart)
        std::string   iniText;      // kWriteIni: the file's content
        std::vector<Pci1203ModuleEntry>   entries;    // kWriteIni: what is recorded
        std::vector<Pci1203ModuleProblem> problems;   // kAlarm
        Result() : event(kNone), gen(0) {}
    };

    Pci1203ModuleCheck() : started_(false), running_(false), restorePending_(false), mode_(kModeCheck), gen_(0), since_(0), alarms_(0) {}

    // The link watch says the link came back (its kRestored): the next good sample starts a generation.
    void OnLinkRestored() { restorePending_ = true; }

    //  One sample. `load()` -> Pci1203ModuleIniLoad, called once when a generation starts.
    //  `clock` = the same instant as wall-clock text (the start time named in the alarm).
    template <class Loader>
    Result Step(const Pci1203ModuleInput& in, unsigned long nowMs, const std::string& clock, Loader load)
    {
        Result r;
        r.gen = gen_;

        // 1. The link watch owns a lost link: abandon a running generation, start none.
        if (in.linkLost) {
            if (running_) {
                running_ = false;
                r.event = kAbandoned;
                r.text  = Pci1203LinkSafeText(GenHead_() + "1203 link lost while checking - no verdict; checked again when the link is restored",
                                              kPci1203ModCheckTextMax);
            }
            return r;
        }

        // 2. Start a generation?
        const bool cardOk = in.open && !in.disabled && in.slavesFound > 0;
        if (!running_) {
            const bool start = cardOk && (!started_ || restorePending_);
            if (!start) return r;
            const bool restore = started_;
            started_ = true; restorePending_ = false; running_ = true;
            ++gen_; r.gen = gen_;
            since_ = nowMs; sinceClock_ = clock;
            const Pci1203ModuleIniLoad L = load();
            entries_.clear(); badWhy_.clear();
            if (L.status == Pci1203ModuleIniLoad::kOk)       { mode_ = kModeCheck; entries_ = L.entries; }
            else if (L.status == Pci1203ModuleIniLoad::kBad) { mode_ = kModeBad;   badWhy_ = L.why; }
            else                                             { mode_ = kModeWrite; }
            char b[160];
            std::snprintf(b, sizeof(b), "1203 module check #%lu started (%s, %s) - %s", gen_, restore ? "link restored" : "first connect",
                          clock.c_str(),
                          mode_ == kModeCheck ? "comparing with the ini" : mode_ == kModeBad ? "ini unreadable" : "no ini yet: recording");
            r.startText = Pci1203LinkSafeText(b, kPci1203ModCheckTextMax);
            if (mode_ == kModeCheck) {
                std::snprintf(b, sizeof(b), " (%d modules listed; alarm WAR16154 if a problem remains after %d s)",
                              (int)entries_.size(), (int)(kPci1203ModCheckMs / 1000));
                r.startText = Pci1203LinkSafeText(r.startText + b, kPci1203ModCheckTextMax);
            }
        }

        // 3. Decide.
        const unsigned long el = Pci1203LinkElapsedMs(nowMs, since_);
        const bool timeUp = el >= (unsigned long)kPci1203ModCheckMs;
        if (mode_ == kModeBad) {
            running_ = false;
            r.event = kIniBad;
            r.text  = Pci1203LinkSafeText(GenHead_() + "Pci1203Modules.ini unreadable - " + badWhy_ + " - not checked (never rewritten)",
                                          kPci1203ModCheckTextMax);
            return r;
        }
        if (mode_ == kModeWrite) {
            const std::string block = Pci1203ModuleWriteBlock(in);
            if (block.empty()) {
                running_ = false;
                r.event   = kWriteIni;
                r.entries = Pci1203ModuleCollect(in);
                r.iniText = Pci1203ModuleIniSerialize(r.entries, sinceClock_);
                int io = 0, mo = 0;
                for (std::size_t i = 0; i < r.entries.size(); ++i) (r.entries[i].kind == kPci1203ModMotor ? mo : io)++;
                char b[128];
                std::snprintf(b, sizeof(b), "recording %d modules (%d IO, %d motor) into Pci1203Modules.ini", (int)r.entries.size(), io, mo);
                r.text = Pci1203LinkSafeText(GenHead_() + b, kPci1203ModCheckTextMax);
            } else if (timeUp) {
                running_ = false;
                r.event = kWriteSkipped;
                r.text  = Pci1203LinkSafeText(GenHead_() + "Pci1203Modules.ini NOT written - " + block +
                                              " (tried again on the next connect)", kPci1203ModCheckTextMax);
            }
            return r;
        }

        // mode_ == kModeCheck
        bool allFoundOp = false;
        std::vector<Pci1203ModuleProblem> pr = Pci1203ModuleDiff(entries_, in, &allFoundOp);
        if (!allFoundOp && !timeUp) return r;      // something listed is missing / not OP: wait for the window
        running_ = false;
        if (pr.empty()) {
            char b[160];
            std::snprintf(b, sizeof(b), "all %d listed modules present, at their stations, in OP after %lu ms",
                          (int)entries_.size(), el);
            r.event = kPass;
            r.text  = Pci1203LinkSafeText(GenHead_() + b, kPci1203ModCheckTextMax);
            return r;
        }
        ++alarms_;
        r.event    = kAlarm;
        r.problems = pr;
        r.text     = AlarmText_(pr, el);
        return r;
    }

    bool          running() const { return running_; }   bool idle() const { return started_ && !running_ && !restorePending_; }   //AI(W906-PASSPROF) 20261006: idle = Step() would return at once (a generation ran, none running, no restore pending) -- the caller may skip building the input; same line
    unsigned long gen()     const { return gen_; }
    unsigned long alarms()  const { return alarms_; }   // alarms asked for since construction

private:
    enum Mode { kModeCheck = 0, kModeWrite, kModeBad };

    std::string GenHead_() const
    {
        char b[48];
        std::snprintf(b, sizeof(b), "1203 module check #%lu: ", gen_);
        return b;
    }

    // "1203 modules: 3 problems 10.0 s after connect 08:01:02 - missing ... / not OP ... (+1 more)"
    std::string AlarmText_(const std::vector<Pci1203ModuleProblem>& pr, unsigned long el) const
    {
        char head[128];
        std::snprintf(head, sizeof(head), "1203 modules: %d problem%s %lu.%lu s after connect %s - ", (int)pr.size(),
                      pr.size() == 1 ? "" : "s", el / 1000ul, (el % 1000ul) / 100ul, sinceClock_.c_str());
        std::string list;
        for (std::size_t i = 0; i < pr.size() && i < (std::size_t)kPci1203ModCheckListMax; ++i) {
            if (!list.empty()) list += " / ";
            list += Pci1203ModuleProblemText(pr[i]);
        }
        if (pr.size() > (std::size_t)kPci1203ModCheckListMax) {
            char b[32];
            std::snprintf(b, sizeof(b), " (+%d more)", (int)pr.size() - (int)kPci1203ModCheckListMax);
            list += b;
        }
        return Pci1203LinkSafeText(head + list, kPci1203ModCheckTextMax);
    }

    bool          started_;          // a generation has started since boot
    bool          running_;          // a generation is waiting for its verdict
    bool          restorePending_;   // OnLinkRestored() since the last start
    Mode          mode_;
    unsigned long gen_;
    unsigned long since_;
    std::string   sinceClock_;
    unsigned long alarms_;
    std::vector<Pci1203ModuleEntry> entries_;   // this generation's ini (kModeCheck)
    std::string   badWhy_;                      // kModeBad
};

}  // namespace ht9045

#endif  // HT9045_ETHERCAT_PCI1203MODULECHECK_H
