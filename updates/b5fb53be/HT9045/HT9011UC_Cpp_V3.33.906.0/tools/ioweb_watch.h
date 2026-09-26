// ===========================================================================
//  tools/ioweb_watch.h -- the PURE half of `ioweb_probe <outDir> <seconds> watch`.
//
//  AI(W906-ONSITE-1) 20260926.  NOT in golden.  NOT part of the runtime.
//
//  WHY IT EXISTS.  EastSun opens the machine's safety doors one at a time and
//  the probe has to say which PCIE-1203 input each door is wired to. That is two
//  small decisions, and both are easy to get subtly wrong in a way that still
//  prints plausible output:
//    1. WHICH BITS CHANGED between two polls of the monitor's DI bytes -- a read
//       that failed for one poll, or a byte the card map re-attributed to a
//       different station, must not be reported as a door opening.
//    2. WHICH IO_Table ROWS NAME THAT BIT -- by the SAME address rule the IO web
//       page uses, JsonBridge/ChanIoPoints.cpp ResolveIoPoint():
//           ISABase 3 (ePCI1203) rows:  Lane = ring, IP = station,
//                                       Port = stationChan * 8 + bit
//       (confirmed against ResolveIoPoint :119-:129 and IoConfigJson :198-:202,
//       which publishes station = IP, stationChan = Port / 8, stationBit =
//       Port % 8). The Bit COLUMN is not part of that rule; it is shown, and a
//       row whose Bit column disagrees with Port % 8 says so.
//  So both live here: no vendor call, no HSys, no file, no clock -- the probe
//  feeds them and tests/test_ioweb_watch.cpp checks them, including against
//  ResolveIoPoint itself on the versioned HT9050 IO_Table.
//
//  ⚠ ROWS ARE LISTED MORE GENEROUSLY THAN THE PAGE RESOLVES THEM, on purpose.
//  Enable=0 rows are listed (a door row that is disabled is exactly what an
//  on-site check should find). A row at the same IP/Port whose Lane is empty,
//  or whose IOType is not an input, is listed WITH A CAVEAT saying the page
//  does not read it from this DI bit -- a mistyped row is a finding, not noise.
//  A row whose Lane names the OTHER ring is not listed: station numbers are
//  unique only within a ring, so that row is a different device.
//
//  AI(W906-ONSITE-1) 20260926 (adversarial review): three more pure pieces.
//    * NoRowText is "(no ePCI1203 row)", not "(no IO_Table row)": the lookup
//      only ever considers ISABase 3 rows, and the HT9050 table HAS door rows --
//      on MotionNet addresses (ISABase 0). DoorRowsOffPci1203 lists those at the
//      start of a watch, so "no row" is never read as "no door in the table".
//    * BitColumnCandidates: 97 of the versioned HT9050 table's 425 ISABase 3
//      rows are written in the older "Port = byte, Bit = bit" form. The page
//      does NOT read them that way; they are a SEPARATE, labelled list and are
//      never mixed into the primary one (which stays = the page's rule).
//    * TorqueLimitLine: one console line for one axis's 60E0h / 60E1h read-back.
// ===========================================================================
#ifndef TOOLS_IOWEB_WATCH_H
#define TOOLS_IOWEB_WATCH_H

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

namespace ht9045 {
namespace iowatch {

enum { kIsaPci1203 = 3 };                               // MachineType.h eIOType: ePCI1203 = 3 (ChanIoPoints.cpp kIsaPci1203)
enum { kDirUnknown = 0, kDirIn = 1, kDirOut = 2 };      // == sjson::IoPointDir (JsonBridge/ChanIo.h)

// One DI byte as the monitor published it this poll.
struct DiByte {
    bool          valid;
    int           ring;          // -1 = flat read: ring is not compared (same as IoRuntimeJson)
    int           station;       // the card map's owner; -1 = the map had nothing for this port
    int           stationChan;   // byte within that station; -1 = unknown
    int           slot;          // the monitor's DI index (= its flat port)
    unsigned char byteData;
    DiByte() : valid(false), ring(-1), station(-1), stationChan(-1), slot(-1), byteData(0) {}
};

// One bit that differs between the baseline and this poll.
struct BitChange {
    int slot;
    int ring;
    int station;
    int stationChan;
    int bit;          // 0..7 within the byte
    int oldV;         // 0 / 1
    int newV;         // 0 / 1
};

// What one diff did with every slot, so the probe can SAY why a slot was silent.
struct DiffStats {
    int compared;     // valid now AND a same-address baseline existed -> compared bit by bit
    int invalid;      // this poll's read failed: nothing reported, the baseline is KEPT
    int adopted;      // first valid reading of the slot: it becomes the baseline, nothing reported
    int readdressed;  // ring / station / stationChan changed (card map re-read): baseline replaced, nothing reported
};

inline bool SameAddress(const DiByte& a, const DiByte& b)
{
    return a.ring == b.ring && a.station == b.station &&
           a.stationChan == b.stationChan && a.slot == b.slot;
}

// Compares `cur` against `base` -- the LAST VALID reading of each slot -- and
// advances `base`. Only a slot valid in both, at the same address, can report a
// change. Why "last valid" and not "last poll": a door opened while one read
// failed would otherwise be lost (invalid -> valid would only be "adopted").
// `base` grows to cur.size(); entries past a shrunk `cur` are kept untouched.
inline std::vector<BitChange> DiffAndAdvance(std::vector<DiByte>& base,
                                             const std::vector<DiByte>& cur,
                                             DiffStats* stats)
{
    DiffStats st = { 0, 0, 0, 0 };
    std::vector<BitChange> out;
    if (base.size() < cur.size()) base.resize(cur.size());
    for (std::size_t i = 0; i < cur.size(); ++i) {
        const DiByte& c = cur[i];
        DiByte& b = base[i];
        if (!c.valid)            { ++st.invalid; continue; }
        if (!b.valid)            { ++st.adopted; b = c; continue; }
        if (!SameAddress(b, c))  { ++st.readdressed; b = c; continue; }
        ++st.compared;
        const unsigned x = static_cast<unsigned>(b.byteData ^ c.byteData);
        for (int bit = 0; bit < 8; ++bit) {
            if ((x & (1u << bit)) == 0u) continue;
            BitChange ch = { c.slot, c.ring, c.station, c.stationChan, bit,
                             (b.byteData >> bit) & 1, (c.byteData >> bit) & 1 };
            out.push_back(ch);
        }
        b = c;
    }
    if (stats) *stats = st;
    return out;
}

// One IO_Table row, in the fields the lookup needs (the probe copies them out of
// HSys.IOTable; the test builds them by hand or from the same table).
struct IoRow {
    int         row;       // 0-based index into HSys.IOTable (= IoConfigJson "row")
    std::string alias;
    std::string type;      // IOType column
    int         dir;       // kDirIn / kDirOut / kDirUnknown (sjson::IoDirectionOfType(type))
    int         isaBase;
    int         lane;      // -1 = empty
    int         ip;        // -1 = empty
    int         port;      // -1 = empty
    int         bitCol;    // the Bit column, -1 = empty -- NOT part of the address rule
    int         enable;
    int         inType;
    IoRow() : row(-1), dir(kDirUnknown), isaBase(-1), lane(-1), ip(-1), port(-1),
              bitCol(-1), enable(0), inType(0) {}
};

// Does this row name DI bit (ring, station, stationChan, bit)? ResolveIoPoint's
// address rule, plus the two generous cases the banner explains: an empty Lane
// matches (with a caveat), and direction is NOT checked here (RowCaveat says).
inline bool AddressMatches(const IoRow& r, int ring, int station, int stationChan, int bit)
{
    if (r.isaBase != kIsaPci1203) return false;
    if (r.ip < 0 || r.port < 0) return false;
    if (station < 0 || stationChan < 0 || bit < 0 || bit > 7) return false;
    if (r.ip != station || r.port / 8 != stationChan || r.port % 8 != bit) return false;
    if (ring >= 0 && r.lane >= 0 && r.lane != ring) return false;   // the other ring = another device
    return true;
}

// Indices into `rows`, in table order, of every row that names this DI bit.
inline std::vector<int> RowsForDiBit(const std::vector<IoRow>& rows,
                                     int ring, int station, int stationChan, int bit)
{
    std::vector<int> hit;
    for (std::size_t i = 0; i < rows.size(); ++i)
        if (AddressMatches(rows[i], ring, station, stationChan, bit)) hit.push_back(static_cast<int>(i));
    return hit;
}

// Would the IO page itself read this (address-matching) row from this DI bit?
// ResolveIoPoint's remaining conditions: an input row with a Lane.
inline bool PageReadsHere(const IoRow& r)
{
    return r.dir == kDirIn && r.lane >= 0;
}

// Why the page would NOT read this matching row from this bit; "" when it would.
inline const char* RowCaveat(const IoRow& r)
{
    if (r.lane < 0)       return "Lane empty: the IO page resolves nothing for this row";
    if (r.dir == kDirOut) return "output-typed: the IO page reads this row from DO, not from this DI bit";
    if (r.dir != kDirIn)  return "IOType is neither input nor output: the IO page resolves nothing for this row";
    return "";
}

// ⚠ Says "ePCI1203", not "IO_Table": only ISABase 3 rows are looked at, and a
// door can have a row on another bus (DoorRowsOffPci1203 lists those).
inline const char* NoRowText() { return "(no ePCI1203 row)"; }

// THE LEGACY READING of an ISABase 3 row: Port = the station's byte, Bit = the
// bit. NOT the page's rule (ResolveIoPoint reads Port = byte * 8 + bit and
// ignores the Bit column) -- used only for the separately labelled candidate
// list. Same ring / station / generous-Lane handling as AddressMatches.
inline bool LegacyAddressMatches(const IoRow& r, int ring, int station, int stationChan, int bit)
{
    if (r.isaBase != kIsaPci1203) return false;
    if (r.ip < 0 || r.port < 0 || r.bitCol < 0) return false;
    if (station < 0 || stationChan < 0 || bit < 0 || bit > 7) return false;
    if (r.bitCol == r.port % 8) return false;   //AI(W906-ONSITE-1) 20260926: only the legacy-form rows (Bit column != Port%8, the ones counted at start-up); a page-form row is already where the page reads it
    if (r.ip != station || r.port != stationChan || r.bitCol != bit) return false;
    if (ring >= 0 && r.lane >= 0 && r.lane != ring) return false;   // the other ring = another device
    return true;
}

// Rows the legacy reading puts on this DI bit, in table order, EXCLUDING rows
// already in the primary list (RowsForDiBit) -- a row is never shown twice.
inline std::vector<int> BitColumnCandidates(const std::vector<IoRow>& rows,
                                            int ring, int station, int stationChan, int bit)
{
    std::vector<int> hit;
    for (std::size_t i = 0; i < rows.size(); ++i)
        if (LegacyAddressMatches(rows[i], ring, station, stationChan, bit) &&
            !AddressMatches(rows[i], ring, station, stationChan, bit))
            hit.push_back(static_cast<int>(i));
    return hit;
}

inline const char* BitColumnCandidatesLabel()
{
    return "Bit-column candidates (legacy reading Port = byte, Bit = bit -- NOT how the web page reads them; "
           "the page uses Port = byte*8 + bit):";
}

// Case-insensitive "door" anywhere in the alias (SnSafeDoor1, SnHeaterDoor, ...).
inline bool AliasHasDoor(const std::string& alias)
{
    static const char kDoor[] = "door";
    if (alias.size() < 4) return false;
    for (std::size_t i = 0; i + 4 <= alias.size(); ++i) {
        std::size_t k = 0;
        for (; k < 4; ++k) {
            char c = alias[i + k];
            if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
            if (c != kDoor[k]) break;
        }
        if (k == 4) return true;
    }
    return false;
}

// Door-named rows that are NOT ISABase 3, in table order -- the doors whose
// input is not on the 1203 table yet, so a watch cannot name them.
inline std::vector<int> DoorRowsOffPci1203(const std::vector<IoRow>& rows)
{
    std::vector<int> hit;
    for (std::size_t i = 0; i < rows.size(); ++i)
        if (rows[i].isaBase != kIsaPci1203 && AliasHasDoor(rows[i].alias)) hit.push_back(static_cast<int>(i));
    return hit;
}

// "row 12 SnSafeDoor2 [Sensor] ISABase=0 Lane=1 IP=4 Port=0 Bit=1 Enable=0"
inline std::string DescribeOffRow(const IoRow& r)
{
    char b[192];
    std::snprintf(b, sizeof(b), "row %d %s [%s] ISABase=%d Lane=%d IP=%d Port=%d Bit=%d Enable=%d",
                  r.row, r.alias.empty() ? "(no alias)" : r.alias.c_str(),
                  r.type.empty() ? "?" : r.type.c_str(),
                  r.isaBase, r.lane, r.ip, r.port, r.bitCol, r.enable);
    return b;
}

// "row 623 SnSafeDoor1 [Sensor] Lane=1 IP=2 Port=25 Bit=1 Enable=1 InType=1"
// + " -- <caveat>" and a Bit-column note when they apply.
inline std::string DescribeRow(const IoRow& r)
{
    char b[160];
    std::snprintf(b, sizeof(b), "row %d %s [%s] Lane=%d IP=%d Port=%d Bit=%d Enable=%d InType=%d",
                  r.row, r.alias.empty() ? "(no alias)" : r.alias.c_str(),
                  r.type.empty() ? "?" : r.type.c_str(),
                  r.lane, r.ip, r.port, r.bitCol, r.enable, r.inType);
    std::string s = b;
    if (r.enable != 1) s += " (Enable=0: the page shows it as disabled)";
    if (r.bitCol >= 0 && r.port >= 0 && r.bitCol != r.port % 8) {
        std::snprintf(b, sizeof(b), " (Bit column %d differs from Port%%8 = %d; the page uses Port)",
                      r.bitCol, r.port % 8);
        s += b;
    }
    const char* cav = RowCaveat(r);
    if (*cav) { s += " -- "; s += cav; }
    return s;
}

// Every matching row, " | "-joined, or "(no ePCI1203 row)".
inline std::string RowsText(const std::vector<IoRow>& rows, const std::vector<int>& hit)
{
    if (hit.empty()) return NoRowText();
    std::string s;
    for (std::size_t k = 0; k < hit.size(); ++k) {
        if (k) s += " | ";
        s += DescribeRow(rows[static_cast<std::size_t>(hit[k])]);
    }
    return s;
}

// The Bit-column candidates, " | "-joined; "" when there are none (the CSV
// column is then an empty quoted field, never "(no ... row)").
inline std::string CandidatesText(const std::vector<IoRow>& rows, const std::vector<int>& cand)
{
    return cand.empty() ? std::string() : RowsText(rows, cand);
}

// RFC 4180 field: always quoted, embedded quotes doubled.
inline std::string CsvQuote(const std::string& v)
{
    std::string s = "\"";
    for (std::size_t i = 0; i < v.size(); ++i) { if (v[i] == '"') s += '"'; s += v[i]; }
    s += '"';
    return s;
}

// ioTableRows = the page's rule (RowsText); bitColumnCandidates = the legacy
// reading (CandidatesText), a separate column so the two cannot be confused.
inline const char* ChangeCsvHeader()
{
    return "session,tSec,poll,ring,station,stationChan,bit,slot,old,new,ioTableRows,bitColumnCandidates\r\n";
}

// Is `firstLine` (as read, with or without its CR / LF) this version's header?
// The probe appends to an existing log only when it is -- a log written by an
// earlier build has one column fewer, and mixing the two in one file would
// shift every later reader's columns.
inline bool IsChangeCsvHeader(std::string firstLine)
{
    while (!firstLine.empty() && (firstLine[firstLine.size() - 1] == '\n' || firstLine[firstLine.size() - 1] == '\r'))
        firstLine.erase(firstLine.size() - 1);
    std::string h = ChangeCsvHeader();
    h.erase(h.size() - 2);   // the CRLF
    return firstLine == h;
}

// One CSV line (CRLF), in ChangeCsvHeader() order.
inline std::string ChangeCsvLine(const std::string& session, double tSec, unsigned long poll,
                                 const BitChange& c, const std::string& rowsText,
                                 const std::string& candidatesText)
{
    char b[160];
    std::snprintf(b, sizeof(b), ",%.1f,%lu,%d,%d,%d,%d,%d,%d,%d,",
                  tSec, poll, c.ring, c.station, c.stationChan, c.bit, c.slot, c.oldV, c.newV);
    return CsvQuote(session) + b + CsvQuote(rowsText) + "," + CsvQuote(candidatesText) + "\r\n";
}

// One half (60E0h or 60E1h) of an axis's torque-limit read-back, as the probe
// copies it out of Pci1203AxisSample (trqLimValid / trqLimVal / trqLimRet /
// trqLimRetText, readText = Pci1203TorqueLimitReadText).
struct TrqLimHalf {
    bool           valid;
    unsigned short val;        // raw, 0.1 % of rated torque; meaningful only when valid
    unsigned long  ret;        // the vendor return of the last attempt (0 = SUCCESS)
    std::string    readText;   // "ok" / "readFailed" / "notRead"
    std::string    retText;    // the vendor's own words for ret, from the monitor; "" = none
    TrqLimHalf() : valid(false), val(0), ret(0ul) {}
};

// "ax3 station 2 axis B: 68E0h pos=3000 (300.0 %) ok ret=0x00000000 | 68E1h neg=- readFailed ret=0x80000009 \"...\""
// `base` is Pci1203GearAxisBase(stationAxis) (0x000 / 0x800), passed in so this
// file does not carry a second copy of that rule. ret is printed only when a
// read was attempted (valid, or ret != 0); "-" otherwise.
inline std::string TorqueLimitLine(int axis, int station, int stationAxis, unsigned base, const TrqLimHalf h[2])
{
    char b[160];
    std::snprintf(b, sizeof(b), "ax%d station %d axis %s:", axis, station,
                  stationAxis == 0 ? "A" : (stationAxis == 1 ? "B" : "?"));
    std::string s = b;
    for (int k = 0; k < 2; ++k) {
        const unsigned idx = (k == 0 ? 0x60E0u : 0x60E1u) + base;
        if (h[k].valid)
            std::snprintf(b, sizeof(b), "%s %04Xh %s=%u (%.1f %%) %s", k ? " |" : "", idx, k ? "neg" : "pos",
                          (unsigned)h[k].val, (double)h[k].val / 10.0, h[k].readText.c_str());
        else
            std::snprintf(b, sizeof(b), "%s %04Xh %s=- %s", k ? " |" : "", idx, k ? "neg" : "pos",
                          h[k].readText.c_str());
        s += b;
        if (h[k].valid || h[k].ret != 0ul) {
            std::snprintf(b, sizeof(b), " ret=0x%08lX", h[k].ret);
            s += b;
            if (!h[k].retText.empty()) { s += " \""; s += h[k].retText; s += "\""; }
        } else {
            s += " ret=-";
        }
    }
    return s;
}

// "[   12.4 s] DI ring 1 station 2 chan 3 bit 1 (slot 25): 0 -> 1"
inline std::string ChangeHeadline(double tSec, const BitChange& c)
{
    char b[160];
    std::snprintf(b, sizeof(b), "[%7.1f s] DI ring %d station %d chan %d bit %d (slot %d): %d -> %d",
                  tSec, c.ring, c.station, c.stationChan, c.bit, c.slot, c.oldV, c.newV);
    return b;
}

}  // namespace iowatch
}  // namespace ht9045

#endif  // TOOLS_IOWEB_WATCH_H
