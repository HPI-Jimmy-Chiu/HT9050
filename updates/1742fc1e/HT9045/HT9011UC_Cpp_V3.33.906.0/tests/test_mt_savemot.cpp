// =============================================================================
//  tests/test_mt_savemot.cpp
//
//  AI(W906-MT-SAVEMOT) 20260930: Motor Test「回寫 Mot_Table」-- the writer behind motor.access action saveMotTable
//  (WebMotorAccess.cpp EOF: MotTableFieldsOf / MotTableEditRow / MotTableSaveRow). EastSun 20260930.
//
//  argv[1] = the Mot_Table.csv to start from -- READ ONLY (CMake passes machines/HT9050/Mot_Table.csv; by hand:
//            D:\HT9045\System\Mot_Table.csv). This test never writes it.
//  The copy it writes = W906_MOTTABLE_PATH (the seam the boot's LoadMotData reads, database.cpp), else
//            %TEMP%\ht9045_savemot_<pid>\Mot_Table.csv. Refused (exit 2) when that path is argv[1] itself or lies under
//            D:\HT9045\System, \config or \IniData (production configuration, CLAUDE.md). Every other file it writes
//            (synthetic tables, backups) sits in the same directory as that copy.
//  Locks:
//   [1] every data row of the file: the ten values the boot puts in MOT (read here independently of the writer) -> "no change":
//       nothing written, no backup, bytes identical; both INDEX_MOTION_CARD arms.
//   [2] a change on a PCI1203 row through the file layer: backup == the old bytes; the new file == the old bytes with exactly
//       those cells replaced (compared with an independent rebuild: every other line, cell, separator, CR LF and the trailing
//       newline identical); integers stay integers ("12000", not "12000.000000"); a real is the shortest text ("0.125");
//       undo -> the original bytes again; a backup name that exists -> "_2".
//   [3] rules on synthetic text: MN200 styles, Range clamp, empty cells, the Index override, MC88X1, CardModel / Motorname /
//       Alias checks, missing / ambiguous column, non-finite / not integral / out of range, the unsigned wrap, quoted cells,
//       blanks around a cell, LF-only / lone CR / no trailing newline.
//   [4] file layer refusals: unreadable file, a read-only file (MoveFileEx refused) -> the bytes kept, no .tmp / .bak left.
// =============================================================================
#include "WebMotorAccess.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <direct.h>      // _mkdir
#include <process.h>     // _getpid
#include <sys/stat.h>    // _stat
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>     // SetFileAttributesA ([4] the read-only file)

using namespace ht9045;

static int g_pass = 0;
static int g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }
static std::string ReadAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static bool CanRead(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); return f.good(); }
static bool WriteAll(const std::string& p, const std::string& d)
{
    std::ofstream f(p.c_str(), std::ios::binary | std::ios::trunc);
    f.write(d.data(), (std::streamsize)d.size());
    return f.good();
}
static bool Exists(const std::string& p) { struct _stat st; return _stat(p.c_str(), &st) == 0; }
static std::string Norm(std::string s)   // lower case, backslashes (for the production-path guard)
{
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '/') s[i] = '\\';
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    }
    return s;
}
static void ForceDir(const std::string& d)
{
    for (std::size_t i = 3; i <= d.size(); ++i)
        if (i == d.size() || d[i] == '\\' || d[i] == '/') _mkdir(d.substr(0, i).c_str());
}

// The test's OWN reading of the file (plain comma split -- the source files have no quotes), independent of the writer's parser.
static std::vector<std::string> Split(const std::string& s)
{
    std::vector<std::string> v;
    std::string c;
    for (std::size_t i = 0; i < s.size(); ++i) { if (s[i] == ',') { v.push_back(c); c.clear(); } else c += s[i]; }
    v.push_back(c);
    return v;
}
static std::string Join(const std::vector<std::string>& v)
{
    std::string s;
    for (std::size_t i = 0; i < v.size(); ++i) { if (i) s += ','; s += v[i]; }
    return s;
}
struct Ln { std::string body, eol; };
static std::vector<Ln> Lines(const std::string& t)   // CR LF / LF files: body + its terminator
{
    std::vector<Ln> v;
    std::size_t b = 0;
    for (std::size_t i = 0; i < t.size(); ++i) {
        if (t[i] != '\n') continue;
        Ln l; l.body = t.substr(b, i - b); l.eol = "\n";
        if (!l.body.empty() && l.body[l.body.size() - 1] == '\r') { l.body.erase(l.body.size() - 1); l.eol = "\r\n"; }
        v.push_back(l); b = i + 1;
    }
    if (b < t.size()) { Ln l; l.body = t.substr(b); v.push_back(l); }
    return v;
}
static std::string Unlines(const std::vector<Ln>& v)
{
    std::string s;
    for (std::size_t i = 0; i < v.size(); ++i) s += v[i].body + v[i].eol;
    return s;
}
static int Col(const std::vector<std::string>& hdr, const char* name)
{
    for (std::size_t i = 0; i < hdr.size(); ++i) if (hdr[i] == name) return (int)i;
    return -1;
}
static std::string Fmt(const char* f, double v) { char b[64]; std::snprintf(b, sizeof(b), f, v); return b; }
static std::string Int(long long v) { return std::to_string(v); }

// What the boot puts into MOT[mi] from one row (TMOTDATA's reads and defaults, then cinitial InitialMotorParameter's MN200 /100,
//   MC88X1 Rate -> Acc/Dec, SetRange's >1000 -> 1000) -- written from database.cpp / cinitial.cpp, not from the writer.
struct BootRow { std::string no, alias, card; int mi; MotorGolden g; };
static bool BootOf(const std::vector<std::string>& hdr, const std::vector<std::string>& c, bool indexMc0, BootRow& out)
{
    if (c.size() < 29) return false;
    struct Cell {
        const std::vector<std::string>& h; const std::vector<std::string>& c;
        std::string operator()(const char* n) const { const int k = Col(h, n); return (k >= 0 && k < (int)c.size()) ? c[(std::size_t)k] : std::string(); }
    } cell = { hdr, c };
    out.no = cell("Motorname"); out.alias = cell("Alias"); out.card = cell("CardModel");
    if (out.no.size() < 2 || (out.no[0] != 'M' && out.no[0] != 'm')) return false;
    for (std::size_t i = 1; i < out.no.size(); ++i) if (out.no[i] < '0' || out.no[i] > '9') return false;
    out.mi = std::atoi(out.no.c_str() + 1);
    const bool indexRow = out.alias == "MTestY1" || out.alias == "MTestZ1" || out.alias == "MTestZ2" || out.alias == "MTestY2";
    const bool forced = indexMc0 && indexRow && out.card != "PCI1203";
    if (forced) out.card = "SMC";
    struct IntOf { const Cell& cell; int operator()(const char* n, int d) const { const std::string s = cell(n); return s.empty() ? d : std::atoi(s.c_str()); } } I = { cell };
    MotorGolden g;
    g.initSpeed = (unsigned)I("InitSpeed", 100);
    g.jogHigh   = (unsigned)I("JogHighSpeed", 100);
    g.jogLow    = (unsigned)I("JogLowSpeed", 100);
    g.homeHigh  = (unsigned)I("HomeHighSpeed", 100);
    g.homeLow   = (unsigned)I("HomeLowSpeed", 100);
    g.softP     = I("SoftLimitP", 999999);
    g.softN     = I("SoftLimitN", -999999);
    for (int k = 0; k < 2; ++k) {
        double a;
        if (forced) a = 1.0;
        else if (out.card == "MC88X1") a = (double)I("Rate", 1);
        else {
            const std::string s = cell(k == 0 ? "Acc" : "Dec");
            a = s.empty() ? 1.0 : std::atof(s.c_str());
            if (out.card == "MN200" && a > 1) { const double d = a / 100.0; a = d; }
        }
        (k == 0 ? g.accDb : g.decDb) = a;
    }
    unsigned r = (out.card == "MC88X1") ? 10u : (unsigned)I("Range", 1);
    if (r > 1000u) r = 1000u;
    g.range = r;
    out.g = g;
    return true;
}

static const MotTableCellChange* CellOf(const MotTableEditResult& e, const char* col)
{
    for (std::size_t i = 0; i < e.cells.size(); ++i) if (e.cells[i].column == col) return &e.cells[i];
    return 0;
}

int main(int argc, char** argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc < 2 || !CanRead(argv[1])) {
        printf("FAIL: argv[1] = a readable Mot_Table.csv to start from (read only)\n");
        return 1;
    }
    const std::string src = argv[1];
    const char* envp = std::getenv("W906_MOTTABLE_PATH");
    const bool fromEnv = envp && *envp;
    std::string copy;
    if (fromEnv) copy = envp;
    else {
        const char* t = std::getenv("TEMP");
        copy = std::string(t ? t : ".") + "\\ht9045_savemot_" + std::to_string((long long)_getpid()) + "\\Mot_Table.csv";
    }
    for (std::size_t i = 0; i < copy.size(); ++i) if (copy[i] == '/') copy[i] = '\\';
    {
        const std::string n = Norm(copy);
        const char* prod[] = { "d:\\ht9045\\system", "d:\\ht9045\\config", "d:\\ht9045\\inidata" };
        for (int k = 0; k < 3; ++k)
            if (n.compare(0, std::strlen(prod[k]), prod[k]) == 0) {
                printf("REFUSED: the copy %s is under %s (production configuration) -- point W906_MOTTABLE_PATH at a scratch copy\n", copy.c_str(), prod[k]);
                return 2;
            }
        if (n == Norm(src)) { printf("REFUSED: the copy %s is argv[1] itself (the source stays read only)\n", copy.c_str()); return 2; }
    }
    const std::size_t sl = copy.find_last_of('\\');
    const std::string dir = (sl == std::string::npos) ? std::string(".") : copy.substr(0, sl);
    ForceDir(dir);
    const std::string orig = ReadAll(src);
    printf("source (read only) %s  %u bytes\ncopy (written)     %s\n", src.c_str(), (unsigned)orig.size(), copy.c_str());
    // leftovers of an earlier run in the same scratch directory
    const char* stamps[] = { "t1", "t1b", "20260930_000001", "20260930_000001_2", "20260930_000002", "20260930_000003", "t4" };
    for (int k = 0; k < 7; ++k) std::remove((copy + ".bak_" + stamps[k]).c_str());
    std::remove((copy + ".tmp_savemot").c_str());

    // ---------------------------------------------------------------- [1]
    printf("[1] every row of the file: the boot's values -> no change, nothing written\n");
    {
        CHECK(WriteAll(copy, orig) && ReadAll(copy) == orig, "scratch copy made (byte for byte)");
        const std::vector<Ln> L = Lines(orig);
        CHECK(L.size() >= 2, "the file has a header and data rows");
        const std::vector<std::string> hdr = Split(L.empty() ? std::string() : L[0].body);
        for (int arm = 0; arm < 2; ++arm) {
            const bool indexMc0 = (arm == 0);
            int rows = 0, bad = 0;
            for (std::size_t k = 1; k < L.size(); ++k) {
                BootRow b;
                if (!BootOf(hdr, Split(L[k].body), indexMc0, b) || b.alias.empty()) continue;
                int same = 0;
                for (std::size_t j = 1; j < L.size(); ++j) { const std::vector<std::string> c = Split(L[j].body); if ((int)c.size() > Col(hdr, "Alias") && c[(std::size_t)Col(hdr, "Alias")] == b.alias) ++same; }
                if (same != 1) continue;
                ++rows;
                const MotTableSaveResult s = MotTableSaveRow(copy, indexMc0 ? "t1" : "t1b", b.alias, b.mi, MotTableFieldsOf(b.g), indexMc0, b.card);
                if (!s.ok || s.wrote || s.edit.changedCount != 0 || s.edit.lineNo != (int)k + 1 || s.edit.newRowText != L[k].body) {
                    if (++bad <= 5) printf("    row %u %s: ok=%d wrote=%d changed=%d line=%d why=%s\n", (unsigned)(k + 1), b.alias.c_str(),
                                           (int)s.ok, (int)s.wrote, s.edit.changedCount, s.edit.lineNo, s.why.c_str());
                }
            }
            printf("    INDEX_MOTION_CARD==0 %s: %d rows checked\n", indexMc0 ? "true" : "false", rows);
            CHECK(rows >= 40 && bad == 0, indexMc0 ? "every row (Index override on): the boot's values -> ok, no change, right line, row text as is"
                                                  : "every row (Index override off): the boot's values -> ok, no change, right line, row text as is");
        }
        CHECK(ReadAll(copy) == orig, "the copy is byte-identical after all the no-change saves");
        CHECK(!Exists(copy + ".bak_t1") && !Exists(copy + ".bak_t1b") && !Exists(copy + ".tmp_savemot"), "no change -> no backup, no temp file");
    }

    // ---------------------------------------------------------------- [2]
    printf("[2] a change on a PCI1203 row through the file layer: only those cells, backup, undo\n");
    {
        CHECK(WriteAll(copy, orig), "scratch copy reset");
        const std::vector<Ln> L = Lines(orig);
        const std::vector<std::string> hdr = Split(L[0].body);
        std::size_t k = 0;
        BootRow b;
        for (std::size_t j = 1; j < L.size() && k == 0; ++j) {
            const std::vector<std::string> c = Split(L[j].body);
            if (!BootOf(hdr, c, true, b) || b.card != "PCI1203") continue;
            const char* need[] = { "InitSpeed", "JogHighSpeed", "JogLowSpeed", "HomeHighSpeed", "HomeLowSpeed", "SoftLimitP", "SoftLimitN", "Acc", "Dec", "Range" };
            bool full = true;
            for (int q = 0; q < 10; ++q) if (c[(std::size_t)Col(hdr, need[q])].empty()) full = false;
            if (full && b.g.range <= 1000u && std::atof(c[(std::size_t)Col(hdr, "Acc")].c_str()) == b.g.accDb) k = j;
        }
        CHECK(k != 0, "a PCI1203 row with all ten cells filled");
        if (k != 0) {
            printf("    row %u: %s\n", (unsigned)(k + 1), L[k].body.c_str());
            MotorGolden g = b.g;
            g.jogHigh += 100;
            g.softP -= 1;
            g.accDb += 4000.0;                                                  // an integral double
            std::vector<std::string> c = Split(L[k].body);
            c[(std::size_t)Col(hdr, "JogHighSpeed")] = Int(g.jogHigh);
            c[(std::size_t)Col(hdr, "SoftLimitP")]   = Int(g.softP);
            c[(std::size_t)Col(hdr, "Acc")]          = Fmt("%.0f", g.accDb);
            std::vector<Ln> E = L; E[k].body = Join(c);
            const std::string expect = Unlines(E);
            const MotTableSaveResult s = MotTableSaveRow(copy, "20260930_000001", b.alias, b.mi, MotTableFieldsOf(g), true, b.card);
            printf("    why=%s backup=%s\n", s.why.c_str(), s.backup.c_str());
            CHECK(s.ok && s.wrote && s.edit.changedCount == 3, "3 cells changed (JogHighSpeed, SoftLimitP, Acc), written");
            CHECK(s.backup == copy + ".bak_20260930_000001" && ReadAll(s.backup) == orig, "backup = path.bak_<stamp>, byte-identical to the old file");
            const std::string now = ReadAll(copy);
            CHECK(now == expect, "the new file == the old bytes with exactly those three cells replaced (independent rebuild)");
            const std::vector<Ln> N = Lines(now);
            int diffLines = 0; bool eolSame = N.size() == L.size();
            for (std::size_t j = 0; j < N.size() && j < L.size(); ++j) { if (N[j].body != L[j].body) ++diffLines; if (N[j].eol != L[j].eol) eolSame = false; }
            CHECK(diffLines == 1 && eolSame && now.size() >= 2 && orig.size() >= 2 && now.substr(now.size() - 2) == orig.substr(orig.size() - 2),
                  "one line differs; same line count, every line ending and the trailing newline unchanged");
            const MotTableCellChange* jh = CellOf(s.edit, "JogHighSpeed");
            const MotTableCellChange* ac = CellOf(s.edit, "Acc");
            const MotTableCellChange* dc = CellOf(s.edit, "Dec");
            CHECK(jh && jh->changed && jh->newText == Int(g.jogHigh) && ac && ac->changed && ac->newText.find('.') == std::string::npos &&
                  ac->newText == Fmt("%.0f", g.accDb) && dc && !dc->changed, "integers stay integers (Acc \"" "12000\"-style, not \"12000.000000\"); Dec untouched");
            CHECK(s.edit.newRowText == E[k].body && s.edit.lineNo == (int)k + 1, "newRowText (the in-memory _CommaText) = the new line; lineNo 1-based");
            CHECK(!Exists(copy + ".tmp_savemot"), "the temp file is gone after the replace");

            g.accDb = 0.125;
            const MotTableSaveResult s2 = MotTableSaveRow(copy, "20260930_000002", b.alias, b.mi, MotTableFieldsOf(g), true, b.card);
            const MotTableCellChange* a2 = CellOf(s2.edit, "Acc");
            CHECK(s2.ok && s2.wrote && s2.edit.changedCount == 1 && a2 && a2->newText == "0.125" && a2->oldText == Fmt("%.0f", b.g.accDb + 4000.0),
                  "a real value -> the shortest text \"0.125\"; only that cell changes this time");
            CHECK(ReadAll(s2.backup) == expect, "the second backup holds the first result");
            c[(std::size_t)Col(hdr, "Acc")] = "0.125";
            std::vector<Ln> E2 = L; E2[k].body = Join(c);
            CHECK(ReadAll(copy) == Unlines(E2), "file == rebuild after the second save");

            const MotTableSaveResult s3 = MotTableSaveRow(copy, "20260930_000001", b.alias, b.mi, MotTableFieldsOf(b.g), true, b.card);   // undo; stamp taken
            CHECK(s3.ok && s3.wrote && s3.edit.changedCount == 3 && s3.backup == copy + ".bak_20260930_000001_2" && ReadAll(s3.backup) == Unlines(E2),
                  "undo with an existing backup name -> \"_2\"; that backup = the state before the undo");
            CHECK(ReadAll(copy) == orig, "undo -> the file is byte-identical to the original again");
            CHECK(ReadAll(copy + ".bak_20260930_000001") == orig, "the first backup was not overwritten");
        }
    }

    // ---------------------------------------------------------------- [3]
    printf("[3] rules on synthetic text\n");
    {
        const std::string H = "Motorname,Alias,SoftLimitN,SoftLimitP,BoardID,Port,IP,Direction,GearRatio,HomeDirectior,HomeHighSpeed,HomeLowSpeed,"
                              "InitSpeed,JogHighSpeed,JogLowSpeed,Rate,Enable,ServoAlarmOn,Range,1P2P,SensorType,SimulateSpeed,CardModel,Acc,Dec,"
                              "EncodeType,PickLimit,LimitLogic,In1Logic";
        const std::string R1203 = "M03,MInArmZA,-999999,999999,3,0,3,0,1,0,10000,1000,100,800,100,60,1,0,75,1,0,10000,PCI1203,8000,8000,0,,0,1";
        const std::string RMnRaw = "M02,MInArmPitch,-999999,999999,2,18,,0,2.5,1,50,1000,50,400,100,50,0,0,30,0,0,10000,MN200,0.1,0.1,0,,0,1";
        const std::string RMnX = "M04,MInArmZB,-999999,999999,2,20,,0,0.85,1,10000,1000,100,800,100,60,0,0,1500,0,0,10000,MN200,70,70,0,,0,1";
        const std::string RY1 = "M13,MTestY1,-999999,999999,,,,0,0.2,0,10000,50000,100,900000,100,20,0,1,70,1,0,10000,SMC,,,2,,,";
        const std::string RZ1 = "M14,MTestZ1,-999999,999999,14,0,14,0,0.1,1,6000,3000,10,90000,100,10,1,1,70,1,0,10000,PCI1203,9000000,9000000,2,-99999,0,1";
        const std::string RMc = "M50,MC88Axis,-999999,999999,0,A,,0,1,0,100,100,100,100,100,77,1,0,10,0,0,1000,MC88X1,1,1,0,,0,0";
        const std::string RDup1 = "M60,MDup,-999999,999999,3,0,3,0,1,0,10000,1000,100,800,100,60,1,0,75,1,0,10000,PCI1203,8000,8000,0,,0,1";
        const std::string RDup2 = "M61,MDup,-999999,999999,3,0,3,0,1,0,10000,1000,100,800,100,60,1,0,75,1,0,10000,PCI1203,8000,8000,0,,0,1";
        const std::string RShort = "M62,MShort,1,2,3";
        std::vector<std::string> q = Split(R1203);
        q[0] = "M70"; q[1] = "MQuoted";
        std::string RQ;
        for (std::size_t i = 0; i < q.size(); ++i) RQ += (i ? "," : "") + std::string("\"") + q[i] + "\"";
        const std::string RSp = "M71,MSpace,-999999,999999,3,0,3,0,1,0,10000,1000,100, 800 ,100,60,1,0,75,1,0,10000,PCI1203,8000,8000,0,,0,1";
        const std::string rows[] = { R1203, RMnRaw, RMnX, RY1, RZ1, RMc, RDup1, RDup2, RShort, RQ, RSp };
        std::string T = H + "\r\n";
        for (int i = 0; i < 11; ++i) T += rows[i] + "\r\n";
        const std::vector<std::string> hdr = Split(H);
        struct Ed {
            const std::string& T;
            MotTableEditResult operator()(const std::string& alias, int mi, const std::vector<MotTableSaveField>& f, bool indexMc0,
                                          const std::string& card, std::string& out) const { return MotTableEditRow(T, alias, mi, f, indexMc0, card, out); }
        } edit = { T };
        std::string out;
        BootRow b;

        // MN200, raw style (Acc 0.1)
        BootOf(hdr, Split(RMnRaw), true, b);
        MotorGolden g = b.g;
        MotTableEditResult e = edit("MInArmPitch", 2, MotTableFieldsOf(g), true, "MN200", out);
        CHECK(e.ok && e.changedCount == 0 && out == T, "MN200 raw style: the boot's values -> no change, text identical");
        g.accDb = 0.2;  e = edit("MInArmPitch", 2, MotTableFieldsOf(g), true, "MN200", out);
        CHECK(e.ok && CellOf(e, "Acc")->newText == "0.2", "MN200 raw: 0.2 -> \"0.2\"");
        g.accDb = 5.0;  e = edit("MInArmPitch", 2, MotTableFieldsOf(g), true, "MN200", out);
        CHECK(e.ok && CellOf(e, "Acc")->newText == "500", "MN200 raw: 5 (> 1) -> \"500\" (the boot's /100 gives 5 back)");
        g.accDb = 0.005; e = edit("MInArmPitch", 2, MotTableFieldsOf(g), true, "MN200", out);
        CHECK(e.ok && CellOf(e, "Acc")->newText == "0.005", "MN200 raw: 0.005 -> \"0.005\"");

        // MN200, x100 style (Acc 70 = 0.7) + Range 1500 (SetRange clamps to 1000)
        BootOf(hdr, Split(RMnX), true, b);
        g = b.g;
        const double k07 = 0.7;                                                 // a double variable: on x87 with -fexcess-precision=standard a bare
        CHECK(g.accDb == k07 && g.range == 1000u, "(model) MN200 70 -> 0.7, Range 1500 -> 1000");   //   literal compares in long double
        e = edit("MInArmZB", 4, MotTableFieldsOf(g), true, "MN200", out);
        CHECK(e.ok && e.changedCount == 0 && out == T && !CellOf(e, "Acc")->note.empty() && !CellOf(e, "Range")->note.empty(),
              "MN200 70 shown as 0.7 and Range 1500 shown as 1000 -> no change, kept with a note");
        g.accDb = 0.8;  e = edit("MInArmZB", 4, MotTableFieldsOf(g), true, "MN200", out);
        CHECK(e.ok && CellOf(e, "Acc")->newText == "80" && !CellOf(e, "Dec")->changed, "MN200 x100 style kept: 0.8 -> \"80\"");
        g.accDb = 0.005; e = edit("MInArmZB", 4, MotTableFieldsOf(g), true, "MN200", out);
        CHECK(e.ok && CellOf(e, "Acc")->newText == "0.005", "MN200 x100 style, 0.005 (x100 = 0.5 would boot as 0.5) -> \"0.005\"");
        g = b.g; g.range = 999; e = edit("MInArmZB", 4, MotTableFieldsOf(g), true, "MN200", out);
        CHECK(e.ok && CellOf(e, "Range")->newText == "999" && e.changedCount == 1, "Range 999 -> \"999\"");

        // MTestY1 (SMC, Acc/Dec empty): the Index override / an empty cell
        BootOf(hdr, Split(RY1), true, b);
        g = b.g;
        e = edit("MTestY1", 13, MotTableFieldsOf(g), true, "SMC", out);
        CHECK(e.ok && e.changedCount == 0 && e.cardModel == "SMC", "Index row, override on: Acc/Dec 1.0 -> no change");
        g.accDb = 2.0; e = edit("MTestY1", 13, MotTableFieldsOf(g), true, "SMC", out);
        CHECK(!e.ok && Has(e.why, "INDEX_MOTION_CARD") && out == T, "Index row, override on: Acc 2 -> refused (the boot never reads it), nothing changed");
        BootOf(hdr, Split(RY1), false, b);
        g = b.g;
        e = edit("MTestY1", 13, MotTableFieldsOf(g), false, "SMC", out);
        CHECK(e.ok && e.changedCount == 0, "Index row, override off: empty Acc = TMOTDATA's 1.0 -> no change");
        g.accDb = 2.0; e = edit("MTestY1", 13, MotTableFieldsOf(g), false, "SMC", out);
        CHECK(!e.ok && Has(e.why, "空的"), "an EMPTY cell with a different value -> refused (not filled)");
        g = b.g; e = edit("MTestY1", 13, MotTableFieldsOf(g), true, "PCI1203", out);
        CHECK(!e.ok && Has(e.why, "CardModel"), "the CardModel this run loaded differs -> refused");

        // MTestZ1 on a PCI1203 row: EastSun R2, no override
        BootOf(hdr, Split(RZ1), true, b);
        g = b.g; g.accDb = 8000000.0;
        e = edit("MTestZ1", 14, MotTableFieldsOf(g), true, "PCI1203", out);
        CHECK(e.ok && CellOf(e, "Acc")->newText == "8000000" && e.cardModel == "PCI1203", "MTestZ1 PCI1203 (R2): Acc written \"8000000\"");

        // MC88X1: Acc/Dec from Rate, Range 10
        BootOf(hdr, Split(RMc), true, b);
        g = b.g;
        CHECK(g.accDb == 77.0 && g.range == 10u, "(model) MC88X1 Acc/Dec = Rate 77, Range 10");
        e = edit("MC88Axis", 50, MotTableFieldsOf(g), true, "MC88X1", out);
        CHECK(e.ok && e.changedCount == 0, "MC88X1 at the boot's values -> no change");
        g.accDb = 5.0; e = edit("MC88Axis", 50, MotTableFieldsOf(g), true, "MC88X1", out);
        CHECK(!e.ok && Has(e.why, "MC88X1"), "MC88X1 Acc 5 -> refused (Acc comes from Rate)");
        g = b.g; g.range = 11; e = edit("MC88Axis", 50, MotTableFieldsOf(g), true, "MC88X1", out);
        CHECK(!e.ok && Has(e.why, "MC88X1"), "MC88X1 Range 11 -> refused (always 10)");

        // row lookup
        BootOf(hdr, Split(R1203), true, b);
        g = b.g;
        e = edit("MDup", 60, MotTableFieldsOf(g), true, "", out);
        CHECK(!e.ok && Has(e.why, "2 列"), "an Alias on two rows -> refused");
        e = edit("MNoSuch", 99, MotTableFieldsOf(g), true, "", out);
        CHECK(!e.ok && Has(e.why, "找不到"), "no such Alias -> refused");
        e = edit("MShort", 62, MotTableFieldsOf(g), true, "", out);
        CHECK(!e.ok && Has(e.why, "29"), "a row with < 29 cells -> refused");
        e = edit("MInArmZA", 4, MotTableFieldsOf(g), true, "", out);
        CHECK(!e.ok && Has(e.why, "Motorname"), "Motorname M03 is not MOT[4] -> refused");
        e = edit("MInArmZA", 3, MotTableFieldsOf(g), true, "PCI1203", out);
        CHECK(e.ok && e.changedCount == 0 && e.lineNo == 2 && e.motorName == "M03", "the right row: line 2, M03");

        // header
        {
            std::string H2 = H; H2.replace(H2.find(",Dec,"), 5, ",Dxc,");
            std::string o2;
            MotTableEditResult e2 = MotTableEditRow(H2 + "\r\n" + R1203 + "\r\n", "MInArmZA", 3, MotTableFieldsOf(g), true, "", o2);
            CHECK(!e2.ok && Has(e2.why, "Dec"), "a column missing from the header -> refused");
            std::string H3 = H; H3.replace(H3.find(",Acc,"), 5, ",AccDec,"); H3.replace(H3.find(",Dec,"), 5, ",Dxc,");
            e2 = MotTableEditRow(H3 + "\r\n" + R1203 + "\r\n", "MInArmZA", 3, MotTableFieldsOf(g), true, "", o2);
            CHECK(!e2.ok && Has(e2.why, "同時"), "two tokens on one header cell (SetMOTTableNo's substring rule) -> refused");
        }

        // values
        {
            std::vector<MotTableSaveField> f = MotTableFieldsOf(g);
            f[7].value = std::numeric_limits<double>::quiet_NaN();
            e = edit("MInArmZA", 3, f, true, "", out);
            CHECK(!e.ok && Has(e.why, "Acc") && out == T, "Acc NaN -> refused");
            f = MotTableFieldsOf(g); f[8].value = std::numeric_limits<double>::infinity();
            e = edit("MInArmZA", 3, f, true, "", out);
            CHECK(!e.ok && Has(e.why, "Dec"), "Dec inf -> refused");
            f = MotTableFieldsOf(g); f[1].value = 800.5;
            e = edit("MInArmZA", 3, f, true, "", out);
            CHECK(!e.ok && Has(e.why, "JogHighSpeed"), "JogHighSpeed 800.5 -> refused (not an integer)");
            f = MotTableFieldsOf(g); f[1].value = -1.0;
            e = edit("MInArmZA", 3, f, true, "", out);
            CHECK(!e.ok, "an unsigned field -1 -> refused (out of range)");
            f = MotTableFieldsOf(g); f[5].value = 3e9;
            e = edit("MInArmZA", 3, f, true, "", out);
            CHECK(!e.ok && Has(e.why, "SoftLimitP"), "SoftLimitP 3e9 -> refused (out of int range)");
            MotorGolden g2 = g; g2.jogHigh = 4294967196u; g2.softN = -5;
            e = edit("MInArmZA", 3, MotTableFieldsOf(g2), true, "", out);
            CHECK(e.ok && CellOf(e, "JogHighSpeed")->newText == "-100" && (unsigned)std::atoi("-100") == 4294967196u && CellOf(e, "SoftLimitN")->newText == "-5",
                  "unsigned 4294967196 -> \"-100\" (the boot's atoi -> unsigned gives it back); SoftLimitN -5 -> \"-5\"");
            std::vector<std::string> c = Split(R1203);
            c[13] = "-100"; c[2] = "-5";
            std::string expect = T; expect.replace(expect.find(R1203), R1203.size(), Join(c));
            CHECK(out == expect, "... and the file text = the old text with those two cells replaced, nothing else");
        }

        // quoting and blanks
        {
            MotorGolden gq = g; gq.jogHigh = 900;
            e = edit("MQuoted", 70, MotTableFieldsOf(gq), true, "", out);
            std::string RQ2 = RQ; RQ2.replace(RQ2.find("\"800\""), 5, "\"900\"");
            std::string expect = T; expect.replace(expect.find(RQ), RQ.size(), RQ2);
            CHECK(e.ok && e.changedCount == 1 && out == expect, "a quoted cell stays quoted (\"800\" -> \"900\"), every other byte identical");
            e = edit("MSpace", 71, MotTableFieldsOf(gq), true, "", out);
            std::string RSp2 = RSp; RSp2.replace(RSp2.find(" 800 "), 5, " 900 ");
            expect = T; expect.replace(expect.find(RSp), RSp.size(), RSp2);
            CHECK(e.ok && e.changedCount == 1 && out == expect, "blanks around a cell stay (\" 800 \" -> \" 900 \")");
        }

        // line endings
        {
            MotorGolden gz; BootOf(hdr, Split(RZ1), true, b); gz = b.g; gz.jogHigh = 90001;
            std::vector<std::string> cz = Split(RZ1); cz[13] = "90001";
            const std::string lf = H + "\n" + R1203 + "\n" + RZ1;                // LF only, no trailing newline
            std::string o2;
            MotTableEditResult e2 = MotTableEditRow(lf, "MTestZ1", 14, MotTableFieldsOf(gz), true, "", o2);
            CHECK(e2.ok && o2 == H + "\n" + R1203 + "\n" + Join(cz), "LF-only file, last line without a newline: kept exactly");
            const std::string cr = H + "\r" + RZ1 + "\r" + R1203 + "\r";          // lone CR
            e2 = MotTableEditRow(cr, "MTestZ1", 14, MotTableFieldsOf(gz), true, "", o2);
            CHECK(e2.ok && o2 == H + "\r" + Join(cz) + "\r" + R1203 + "\r" && e2.lineNo == 2, "lone-CR file: kept exactly");
        }
    }

    // ---------------------------------------------------------------- [4]
    printf("[4] file layer refusals\n");
    {
        const std::string none = dir + "\\no_such_mot_table.csv";
        std::remove(none.c_str());
        MotorGolden g;
        MotTableSaveResult s = MotTableSaveRow(none, "t4", "MInArmZA", 3, MotTableFieldsOf(g), true, "");
        CHECK(!s.ok && !s.wrote && Has(s.why, "讀不到"), "unreadable file -> refused");

        const std::string ro = dir + "\\readonly_mot_table.csv";
        ::SetFileAttributesA(ro.c_str(), FILE_ATTRIBUTE_NORMAL);
        std::remove(ro.c_str()); std::remove((ro + ".bak_t4").c_str()); std::remove((ro + ".bak_t4_2").c_str()); std::remove((ro + ".tmp_savemot").c_str());
        const std::string H = "Motorname,Alias,SoftLimitN,SoftLimitP,BoardID,Port,IP,Direction,GearRatio,HomeDirectior,HomeHighSpeed,HomeLowSpeed,"
                              "InitSpeed,JogHighSpeed,JogLowSpeed,Rate,Enable,ServoAlarmOn,Range,1P2P,SensorType,SimulateSpeed,CardModel,Acc,Dec,"
                              "EncodeType,PickLimit,LimitLogic,In1Logic";
        const std::string R1203 = "M03,MInArmZA,-999999,999999,3,0,3,0,1,0,10000,1000,100,800,100,60,1,0,75,1,0,10000,PCI1203,8000,8000,0,,0,1";
        const std::string T = H + "\r\n" + R1203 + "\r\n";
        CHECK(WriteAll(ro, T), "synthetic file written");
        BootRow b;
        BootOf(Split(H), Split(R1203), true, b);
        g = b.g;
        s = MotTableSaveRow(ro, "t4", "MInArmZA", 3, MotTableFieldsOf(g), true, "PCI1203");
        CHECK(s.ok && !s.wrote && !Exists(ro + ".bak_t4"), "no change -> ok, nothing written, no backup");
        ::SetFileAttributesA(ro.c_str(), FILE_ATTRIBUTE_READONLY);
        g.jogHigh = 801;
        s = MotTableSaveRow(ro, "t4", "MInArmZA", 3, MotTableFieldsOf(g), true, "PCI1203");
        printf("    read-only: ok=%d wrote=%d why=%s\n", (int)s.ok, (int)s.wrote, s.why.c_str());
        CHECK(!s.ok && !s.wrote && ReadAll(ro) == T && !Exists(ro + ".tmp_savemot") && !Exists(ro + ".bak_t4"),
              "read-only file: the replace is refused -> the bytes kept, no .tmp / .bak left");
        ::SetFileAttributesA(ro.c_str(), FILE_ATTRIBUTE_NORMAL);
        s = MotTableSaveRow(ro, "t4", "MInArmZA", 3, MotTableFieldsOf(g), true, "PCI1203");
        std::string expect = T; expect.replace(expect.find(",800,"), 5, ",801,");
        CHECK(s.ok && s.wrote && ReadAll(ro) == expect && ReadAll(ro + ".bak_t4") == T, "writable again -> written, backup = the old bytes");
        std::remove(ro.c_str()); std::remove((ro + ".bak_t4").c_str());
    }

    CHECK(ReadAll(src) == orig, "argv[1] (the source) is byte-identical at the end -- never written");
    if (!fromEnv) {                                                           // the %TEMP% default: leave nothing behind
        for (int k = 0; k < 7; ++k) std::remove((copy + ".bak_" + stamps[k]).c_str());
        std::remove(copy.c_str());
        _rmdir(dir.c_str());
    }
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
