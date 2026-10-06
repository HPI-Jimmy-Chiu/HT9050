// =============================================================================
//  tests/test_gear_teach_save.cpp
//
//  AI(W906-GEARRATIO2) 20261003 [W906]: NB2 R171 M2 -- the Motor Test "Gear Ratio" tab's REAL teach.ini write path, compared
//  byte for byte. Every [GEAR] save test in tests/test_web_motor_access.cpp uses the fake writer (FakeBackend
//  GearTeachSaveReload), and the runtime checks of a save (WebMotorAccess.cpp GearTeachDiffWhy + the reload) compare VALUES
//  only ("12" == "012"): a key order, comment, blank line or EOL change made by the writer would go unseen.
//
//  The path, as wb_serve runs it (the god-stack is linked, nothing in between is a stand-in):
//    GearLiveTeachRefs (GearRatioLive.cpp)  -> GearPlanBuild (GearCalc.cpp, the plan gearRatioPreview / gearRatioSave use)
//    -> GearLiveSetTeach -> GearLiveTeachSaveReload(true) = FileRW_Teach_SaveFile(true) -> IC_SaveFile (FileRW/Teach.cpp:
//       TECH_PARA / TECH_TWOPARA / TECH_SUCKPARA SaveToFile = WriteIniData in place; elTeach->SaveEditTextToFile = a TMemIniFile
//       whole-file rewrite, Public/HTEditList.cpp; the two rotator backlash keys) -> fTeach->ReadFile() -> the proxies mirrored
//       -> InitShuttleThreadParameter() (InitialOK is false here: it returns at once) -> fAllMotorHome=false.
//  It is the writer the Teach page's Save uses (FileRW/Teach.cpp IC_btnSaveClick :193) -- golden behaviour, NOT fixed here:
//  whatever it changes besides the rescaled values is reported and pinned as today's behaviour (see [3]).
//
//  argv[1] = the HT9050 snapshot teach.ini, READ ONLY (CMake: machines/HT9050/snapshot/machine_params/runcfg/system/teach.ini
//            = the file the machine's wb_serve reads and writes, SNAPSHOT_SOURCE.md; ASCII, CRLF)
//  argv[2] = the snapshot Gerneral.ini, READ ONLY (the twelve config keys the teach registry and ReadFile branch on are read
//            from it by this test's own parser; LoadMachineConfig is not run)
//  argv[3] = the scratch directory (CMake: <build>/tests/machine_config_scratch/GearTeachSave). asTeachPath = <argv[3]>\teach.ini.
//  Refused (exit 2) before any Handler code runs unless every ctest redirect variable AND that copy are in ctest's scratch
//  (tests/w906_ctest_guard.h), and when the copy would be argv[1] itself. Nothing outside argv[3] and the redirect roots is
//  written (RecordChangeLogProcess -> MyDBIProcess goes to machine_log_scratch).
//
//  Locks:
//   [1] the boot read (wb_serve SetWorkParameter -> ReadTechData -> fTeach->ReadFile; elTeach's TMemIniFile read ends with
//       UpdateFile) on the snapshot copy: bytes before == bytes after (the snapshot already went through the machine's boot), except
//       that it may ADD W906 extension-row keys (WebTeachButtons.gen.inc, IO_CARD_TYPE==PCI1203_IO) = 0 at their section's end -- a
//       snapshot older than a new teach point (POOL-5 #13, 20261006). [1b] checks that rule on made-up diffs.
//   [2] the gear save of MInArmX (spec §3.2 ratio 1 -> 0.9921524): every changed value is one of the plan's keys with its new
//       value, every plan key changed; no key removed, no key or section moved, both files are in TMemIniFile's own layout
//       ("[s]" / "k=v" / one blank line after every section, CRLF); after the reload every registry variable is what the plan
//       says (the runtime check, now on the real writer).
//   [3] what else the real writer changes -- today's behaviour, pinned, NOT fixed (the Teach page's Save does the same):
//       (a) rules, any snapshot: only elTeach keys the file lacks are added (at their section's end); a value changes only to
//           the same number in another text (elTeach doubles "%0.6f") or to its registry variable's value (two slots of one
//           variable that disagreed in the file);
//       (b) this snapshot, measured 20261003: 48 [ArmAlignment] keys added ({In,Out}Arm{PitchX,X,Y}Alignment{A,B}{a..d}=0),
//           [MInArmZE] SetEditAutoClean 0 -> -1220 (it and elTeach [InArm] AutoCleanPick share Teach.iAutoCleanPick), 8 CCD
//           doubles 0.0000 -> 0.000000; 10844 -> 11976 bytes, the bytes pinned by FNV-1a 64; order / CRLF / blank lines kept;
//       (c) so gearRatioSave's own verify (GearTeachDiffWhy: only the plan's keys may differ) sees 49 keys outside the plan
//           and refuses + restores the FIRST save on this file -- until one Teach-page Save has made the same 49 changes.
//   [4] a second save with nothing changed leaves the bytes alone (the writer is stable after its first rewrite).
// =============================================================================
#include "GearRatioBackend.h"
#include "GearCalc.h"
#include "forms/fTeach.h"
#include "forms/fTeachPara.h"
#include "Public/HTEditList.h"
#include "common.h"
#include "cmydef.h"
#include "MachineType.h"
#include "Motor/mymotor.h"
#include "w906_ctest_guard.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <direct.h>      // _mkdir

using namespace ht9045;

void InitialMotorName();                  // cinitial.cpp:3295 (golden declares it in no header; cinitial.cpp:3704 forward-declares it)
void FileRW_Teach_Boot();                 // FileRW/Teach.cpp (wb_serve boot: elTeach + golden InitialTeachEditList)
void FileRW_Teach_MirrorProxies();        // FileRW/Teach.cpp EOF

// Link-only, NOT under test: GearRatioLive.cpp's GearLiveOperator (the calibration log's operator name) calls
//   WebLogin_StateJson, whose body (WebLogin.cpp) is compiled into wb_serve only. This test never calls GearLiveOperator.
std::string WebLogin_StateJson() { return "{}"; }
// Link-only, NOT under test (the same stand-in and reason as tests/test_hsys_heater_mix.cpp): cprod.cpp (ht9045_globals) calls
//   FileRW_IniConfig_ChangeCBListProperty, whose body is FileRW/IniConfig.cpp (not compiled here); without this the linker would
//   pull ht9045_globals' FileRW/_fallback.cpp member, which also defines FileRW_ProxyChecked -- a duplicate of the real
//   FileRW/_EditList.cpp this test compiles. Same empty body as FileRW/_fallback.cpp.
void FileRW_IniConfig_ChangeCBListProperty() {}
// Link-only (FileRW/_EditList.cpp:554 says so: "ctests that compile this file without JsonBridge give a test-local FormLock"):
//   the FormJson recursive lock (JsonBridge/FormJson.cpp, wb_serve only) guards the proxies against other threads; this test has one.
namespace ht9045 { namespace formjson { void FormLock() {} void FormUnlock() {} } }

namespace {

int g_pass = 0;
int g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", std::string(msg).c_str()); ++g_pass; } \
        else      { printf("  FAIL: %s  (line %d)\n", std::string(msg).c_str(), __LINE__); ++g_fail; } \
    } while (0)

std::string ReadAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
bool CanRead(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); return f.good(); }
bool WriteAll(const std::string& p, const std::string& d)
{
    std::ofstream f(p.c_str(), std::ios::binary | std::ios::trunc);
    f.write(d.data(), (std::streamsize)d.size());
    return f.good();
}
std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}
std::string Norm(std::string s)                                                // lower case, backslashes (path compare)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return Lower(s);
}
std::string I2S(long long v) { return std::to_string(v); }                    // not "%lld": MinGW 6.3's msvcrt printf has no ll

// ---- an INI file as this test reads it (independent of the writer's own parser) ----
struct Item { std::string key, val; };
struct Sec  { std::string name; std::vector<Item> items; };
struct Ini  { std::vector<Sec> secs; int odd = 0; bool lfOnly = false; };
Ini ParseIni(const std::string& t)
{
    Ini x;
    std::size_t b = 0;
    while (b < t.size()) {
        std::size_t e = t.find('\n', b);
        if (e == std::string::npos) e = t.size();
        std::string line = t.substr(b, e - b);
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1); else if (e < t.size()) x.lfOnly = true;
        b = e + 1;
        if (line.empty()) continue;
        if (line[0] == '[') {
            const std::size_t c = line.find(']');
            Sec s; s.name = line.substr(1, c == std::string::npos ? std::string::npos : c - 1);
            x.secs.push_back(s);
            continue;
        }
        const std::size_t q = line.find('=');
        if (q == std::string::npos || x.secs.empty()) { ++x.odd; continue; }   // a comment, junk, or a key before any section
        Item it; it.key = line.substr(0, q); it.val = line.substr(q + 1);
        x.secs.back().items.push_back(it);
    }
    return x;
}
// TMemIniFile's layout (vclcompat/IniFiles.cpp TIniStore::SaveToFile with blankLineAfterSection = BCB6 GetStrings)
std::string Canon(const Ini& x)
{
    std::string t;
    for (std::size_t s = 0; s < x.secs.size(); ++s) {
        t += "[" + x.secs[s].name + "]\r\n";
        for (std::size_t k = 0; k < x.secs[s].items.size(); ++k) t += x.secs[s].items[k].key + "=" + x.secs[s].items[k].val + "\r\n";
        t += "\r\n";
    }
    return t;
}
typedef std::pair<std::string, std::string> SK;                                // (lower section, lower key)
struct Change { std::string sec, key, oldV, newV; };
struct Diff {
    std::vector<Change> changed;                                               // the same key, another value text
    std::vector<Change> added;                                                 // newV = the value; oldV = "end" / "inside" (where in its section)
    std::vector<Change> removed;
    std::vector<std::string> moved;                                            // a key or a section out of its old order
    std::vector<std::string> secAdded, secRemoved;
};
Diff DiffIni(const Ini& a, const Ini& b)
{
    Diff d;
    std::map<std::string, std::size_t> bi;
    for (std::size_t s = 0; s < b.secs.size(); ++s) bi.insert(std::make_pair(Lower(b.secs[s].name), s));
    std::set<std::string> seenA;
    long lastSec = -1;
    for (std::size_t s = 0; s < a.secs.size(); ++s) {
        const std::string ln = Lower(a.secs[s].name);
        seenA.insert(ln);
        std::map<std::string, std::size_t>::const_iterator it = bi.find(ln);
        if (it == bi.end()) { d.secRemoved.push_back(a.secs[s].name); continue; }
        if ((long)it->second < lastSec) d.moved.push_back("[" + a.secs[s].name + "] (section)");
        lastSec = (long)it->second;
        const Sec& sa = a.secs[s];
        const Sec& sb = b.secs[it->second];
        std::map<std::string, std::size_t> ki;
        for (std::size_t k = 0; k < sb.items.size(); ++k) ki.insert(std::make_pair(Lower(sb.items[k].key), k));
        std::set<std::string> seenK;
        long lastK = -1;
        for (std::size_t k = 0; k < sa.items.size(); ++k) {
            const std::string kl = Lower(sa.items[k].key);
            seenK.insert(kl);
            std::map<std::string, std::size_t>::const_iterator jt = ki.find(kl);
            if (jt == ki.end()) { Change c; c.sec = sa.name; c.key = sa.items[k].key; c.oldV = sa.items[k].val; d.removed.push_back(c); continue; }
            if ((long)jt->second < lastK) d.moved.push_back("[" + sa.name + "] " + sa.items[k].key);
            lastK = (long)jt->second;
            if (sb.items[jt->second].val != sa.items[k].val || sb.items[jt->second].key != sa.items[k].key) {
                Change c; c.sec = sa.name; c.key = sa.items[k].key; c.oldV = sa.items[k].val; c.newV = sb.items[jt->second].val;
                if (sb.items[jt->second].key != sa.items[k].key) c.newV += " (key spelled " + sb.items[jt->second].key + ")";
                d.changed.push_back(c);
            }
        }
        for (std::size_t k = 0; k < sb.items.size(); ++k) {
            if (seenK.count(Lower(sb.items[k].key))) continue;
            Change c; c.sec = sb.name; c.key = sb.items[k].key; c.newV = sb.items[k].val;
            c.oldV = ((long)k > lastK) ? "end" : "inside";
            d.added.push_back(c);
        }
    }
    for (std::size_t s = 0; s < b.secs.size(); ++s) if (!seenA.count(Lower(b.secs[s].name))) d.secAdded.push_back(b.secs[s].name);
    return d;
}
void PrintDiff(const char* what, const Diff& d)
{
    printf("    %s: %u changed, %u added, %u removed, %u moved, %u sections added, %u sections removed\n", what,
           (unsigned)d.changed.size(), (unsigned)d.added.size(), (unsigned)d.removed.size(), (unsigned)d.moved.size(),
           (unsigned)d.secAdded.size(), (unsigned)d.secRemoved.size());
    for (std::size_t i = 0; i < d.changed.size(); ++i) printf("      changed [%s] %s: %s -> %s\n", d.changed[i].sec.c_str(), d.changed[i].key.c_str(), d.changed[i].oldV.c_str(), d.changed[i].newV.c_str());
    for (std::size_t i = 0; i < d.added.size(); ++i)   printf("      added   [%s] %s=%s (%s of its section)\n", d.added[i].sec.c_str(), d.added[i].key.c_str(), d.added[i].newV.c_str(), d.added[i].oldV.c_str());
    for (std::size_t i = 0; i < d.removed.size(); ++i) printf("      removed [%s] %s=%s\n", d.removed[i].sec.c_str(), d.removed[i].key.c_str(), d.removed[i].oldV.c_str());
    for (std::size_t i = 0; i < d.moved.size(); ++i)   printf("      moved   %s\n", d.moved[i].c_str());
    for (std::size_t i = 0; i < d.secAdded.size(); ++i)   printf("      section added   [%s]\n", d.secAdded[i].c_str());
    for (std::size_t i = 0; i < d.secRemoved.size(); ++i) printf("      section removed [%s]\n", d.secRemoved[i].c_str());
}
// the first byte where two texts differ, with the line around it (the report's "exactly which bytes")
void PrintFirstByteDiff(const std::string& a, const std::string& b)
{
    std::size_t i = 0;
    while (i < a.size() && i < b.size() && a[i] == b[i]) ++i;
    if (i == a.size() && i == b.size()) { printf("    bytes identical (%u)\n", (unsigned)a.size()); return; }
    const std::size_t ls = a.rfind('\n', i == 0 ? 0 : i - 1);
    const std::size_t from = (ls == std::string::npos) ? 0 : ls + 1;
    printf("    first differing byte at offset %u (sizes %u -> %u); line there: \"%s\" -> \"%s\"\n", (unsigned)i, (unsigned)a.size(), (unsigned)b.size(),
           a.substr(from, a.find('\r', from) == std::string::npos ? std::string::npos : a.find('\r', from) - from).c_str(),
           b.substr(from, b.find('\r', from) == std::string::npos ? std::string::npos : b.find('\r', from) - from).c_str());
}

// the snapshot Gerneral.ini, read by this test (never through asGeneralPath, never written)
std::map<SK, std::string> ReadCfg(const std::string& t)
{
    std::map<SK, std::string> m;
    const Ini x = ParseIni(t);
    for (std::size_t s = 0; s < x.secs.size(); ++s)
        for (std::size_t k = 0; k < x.secs[s].items.size(); ++k) {
            std::string v = x.secs[s].items[k].val;
            while (!v.empty() && (v[v.size() - 1] == ' ' || v[v.size() - 1] == '\t')) v.erase(v.size() - 1);
            m.insert(std::make_pair(SK(Lower(x.secs[s].name), Lower(x.secs[s].items[k].key)), v));
        }
    return m;
}

//AI(W906-POOL5-13) 20261006: POOL-5 #13 (Frank FR-PR1 2C, RULINGS_20261006 #16; NB2-1 R242) -- [1] used to pin the boot read byte for byte
//  against the snapshot, with a one-off exception for [MTestZ1] setEditIndex1ToOutSht1Z (10/05); every new W906 teach point turned it red
//  again. The structural rule instead: the boot read may only ADD keys, and every added key is a W906 extension row of the teach
//  registry (WebTeachButtons.gen.inc, the generator's EXT_ROWS = the rows whose condition is IO_CARD_TYPE==PCI1203_IO) in its
//  motor's section, at that section's end, with the default value 0 -- golden ReadFromFile writes a missing key with the variable's
//  value, 0 on a first boot. No value change, removal, move or section change; a new section only if it is an extension row's
//  motor section holding nothing but such keys. The table comes from the generator, so a new extension row needs no test edit.
struct ExtKey { std::string sec, key; };
std::vector<ExtKey> ExtRowsFromTable()                                       // after InitialMotorName(): MOT[].Alias = the sections
{
    struct Row { const char* atoms; int mot; const char* motName; const char* key; };
#define W5B_COND(e)  0
#define W5B_PTR(x)   0
#define W5B_NOPTR    0
#define W5B_MOT(x)   (int)(x), #x
#define W5B_NOMOT    -1, ""
#define W5B_ROW(o, live, atoms, p0, p1, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis) { atoms, m0, k0 },
#define W5B_TAGOVR(b, a, m)
#define W5B_BTN(b, h, tag, dv, so, hw, lr)
    const Row rows[] = {
#include "WebTeachButtons.gen.inc"
        { "", -1, "", "" }                                                      // sentinel
    };
    std::vector<ExtKey> out;
    for (std::size_t i = 0; i + 1 < sizeof(rows) / sizeof(rows[0]); ++i) {
        if (std::string(rows[i].atoms) != "IO_CARD_TYPE==PCI1203_IO" || rows[i].mot < 0) continue;
        ExtKey e; e.sec = MOT[rows[i].mot].Alias.c_str(); e.key = rows[i].key;
        if (e.sec.empty()) e.sec = rows[i].motName;
        out.push_back(e);
    }
    return out;
}
const ExtKey* FindExt(const std::vector<ExtKey>& ext, const std::string& sec, const std::string& key)
{
    for (std::size_t i = 0; i < ext.size(); ++i)
        if (Lower(ext[i].sec) == Lower(sec) && Lower(ext[i].key) == Lower(key)) return &ext[i];
    return 0;
}
// true when a -> b only adds extension-row keys with the default 0 (b = the file after the boot read); why = the first violation
bool OnlyExtDefaultsAdded(const Diff& d, const Ini& b, const std::vector<ExtKey>& ext, std::string& why)
{
    if (!d.changed.empty()) { why = "a value changed: [" + d.changed[0].sec + "] " + d.changed[0].key; return false; }
    if (!d.removed.empty()) { why = "a key removed: [" + d.removed[0].sec + "] " + d.removed[0].key; return false; }
    if (!d.moved.empty())   { why = "moved: " + d.moved[0]; return false; }
    if (!d.secRemoved.empty()) { why = "a section removed: [" + d.secRemoved[0] + "]"; return false; }
    for (std::size_t i = 0; i < d.added.size(); ++i) {
        const Change& a = d.added[i];
        if (!FindExt(ext, a.sec, a.key)) { why = "[" + a.sec + "] " + a.key + " is not a W906 extension row of this section"; return false; }
        if (a.newV != "0")   { why = "[" + a.sec + "] " + a.key + "=" + a.newV + " is not the default 0"; return false; }
        if (a.oldV != "end") { why = "[" + a.sec + "] " + a.key + " was added inside its section, not at its end"; return false; }
    }
    for (std::size_t i = 0; i < d.secAdded.size(); ++i) {
        const Sec* s = 0;
        for (std::size_t k = 0; k < b.secs.size(); ++k) if (Lower(b.secs[k].name) == Lower(d.secAdded[i])) s = &b.secs[k];
        if (!s) { why = "section [" + d.secAdded[i] + "] not found"; return false; }
        for (std::size_t k = 0; k < s->items.size(); ++k) {
            if (!FindExt(ext, s->name, s->items[k].key)) { why = "new section [" + s->name + "] holds " + s->items[k].key + ", not a W906 extension row"; return false; }
            if (s->items[k].val != "0") { why = "new section [" + s->name + "] " + s->items[k].key + "=" + s->items[k].val + " is not the default 0"; return false; }
        }
    }
    return true;
}

}  // namespace

int main(int argc, char** argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc < 4 || !CanRead(argv[1]) || !CanRead(argv[2])) {
        printf("FAIL: argv[1] = the snapshot teach.ini, argv[2] = the snapshot Gerneral.ini (both read only), argv[3] = the scratch directory\n");
        return 1;
    }
    const std::string src = argv[1], cfgPath = argv[2];
    std::string dir = argv[3];
    for (std::size_t i = 0; i < dir.size(); ++i) if (dir[i] == '/') dir[i] = '\\';
    while (!dir.empty() && dir[dir.size() - 1] == '\\') dir.erase(dir.size() - 1);
    const std::string copy = dir + "\\teach.ini";
    {
        const char* const rt[] = { "asTeachPath (the copy)", copy.c_str(), 0 };
        if (!W906TestRequireCtestRedirects("GearTeachSave", rt)) return 2;
        if (Norm(copy) == Norm(src)) { printf("REFUSED: the copy %s is argv[1] itself (the snapshot stays read only)\n", copy.c_str()); return 2; }
    }
    for (std::size_t i = 3; i <= dir.size(); ++i) if (i == dir.size() || dir[i] == '\\') _mkdir(dir.substr(0, i).c_str());
    std::remove(copy.c_str());

    const std::string bytes0 = ReadAll(src);
    printf("source (read only) %s  %u bytes\ncopy (written)     %s\n", src.c_str(), (unsigned)bytes0.size(), copy.c_str());
    CHECK(!bytes0.empty() && WriteAll(copy, bytes0) && ReadAll(copy) == bytes0, "setup: the snapshot copied into the scratch directory, byte for byte");
    {
        const Ini x0 = ParseIni(bytes0);
        bool upd = false, sh1 = false, sh2 = false;
        for (std::size_t s = 0; s < x0.secs.size(); ++s) {
            const std::string n = Lower(x0.secs[s].name);
            if (n == "minshuttle1") sh1 = true;
            if (n == "minshuttle2") sh2 = true;
            if (n == "teach ini") for (std::size_t k = 0; k < x0.secs[s].items.size(); ++k) if (Lower(x0.secs[s].items[k].key) == "update2" && x0.secs[s].items[k].val == "1") upd = true;
        }
        // forms/fTeachPara.cpp TfTeach::ReadFile: without [Teach INI] Update2=1 or [MInShuttle1] / [MInShuttle2] it reads the
        //   machine's d:\HT9045\system\tech.dat (golden's literal) and rewrites teach.ini from it -- this test refuses that input.
        if (!(upd && sh1 && sh2)) { printf("REFUSED: %s lacks [Teach INI] Update2=1 or [MInShuttle1] / [MInShuttle2] (ReadFile would read d:\\HT9045\\system\\tech.dat)\n", src.c_str()); return 2; }
    }
    asTeachPath = AnsiString(copy.c_str());
    CHECK(std::string(asTeachPath.c_str()) == copy && GearLiveTeachIniPath() == copy, "asTeachPath = the scratch copy (GearLiveTeachIniPath reads it back)");
    CHECK(InitialOK == false, "InitialOK is false: InitShuttleThreadParameter (the save's tail) returns at once");

    // ---- the machine's configuration (only the keys the teach registry / ReadFile / SaveFile branch on) ----
    {
        const std::map<SK, std::string> cfg = ReadCfg(ReadAll(cfgPath));
        struct K { const char* sec; const char* key; int* dst; };
        const K keys[] = {
            { "system", "IO_CARD_TYPE", &IO_CARD_TYPE },                       // fTeachRegistry.cpp:767 (the PCI1203 rows)
            { "outsortarm", "USE_OUT_SORT_ARM", &USE_OUT_SORT_ARM },           // :95 / :439 / :548 / :750, SaveFile / ReadFile
            { "system", "USE_PICKER_COUNT", &USE_PICKER_COUNT },               // :136 / :236 / :417 ..., the ep1Picker remap
            { "system", "AUTO_EMPTY_COLOR", &AUTO_EMPTY_COLOR },               // :246 / :252
            { "system", "INOUT_ARM_PICKER_USE_MOTOR", &InOutArmPickerUseMotor },   // :459 / :523
            { "indexdriver", "USE_INDEX_ARM_AXES", &USE_INDEX_ARM_AXES },      // :660
            { "system", "INSTALL_OCR_YMot", &INSTALL_OCR_YMot },               // :697
            { "system", "USE_IN_OUT_ARM_Y_PITCH", &USE_IN_OUT_ARM_Y_PITCH },   // :712 / :727 / :738
            { "system", "USE_OUT_ARM_Y_PITCH", &USE_OUT_ARM_Y_PITCH },         // :718
            { "system", "In_Shuttle_Auto_Latch", &In_Shuttle_Auto_Latch },     // FileRW/Teach.cpp MirrorToProxies
            { "system", "CUSTOMER_CODE", &CUSTOMER_CODE },
        };
        int missing = 0;
        std::string got;
        for (std::size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
            std::map<SK, std::string>::const_iterator it = cfg.find(SK(keys[i].sec, Lower(keys[i].key)));
            if (it == cfg.end()) { ++missing; printf("    config key [%s] %s missing\n", keys[i].sec, keys[i].key); continue; }
            *keys[i].dst = std::atoi(it->second.c_str());
            got += std::string(got.empty() ? "" : ", ") + keys[i].key + "=" + it->second;
        }
        std::map<SK, std::string>::const_iterator aoa = cfg.find(SK("system", "machine_has_auto_alignment_ccd"));
        if (aoa == cfg.end()) ++missing; else MACHINE_HAS_AUTO_ALIGNMENT_CCD = std::atoi(aoa->second.c_str()) != 0;   // TfTeach::ReadFile :485
        printf("    config: %s, MACHINE_HAS_AUTO_ALIGNMENT_CCD=%d\n", got.c_str(), (int)MACHINE_HAS_AUTO_ALIGNMENT_CCD);
        CHECK(missing == 0, "the twelve config keys read from the snapshot Gerneral.ini (the registry of THIS machine, not the defaults)");
    }

    // ---- wb_serve's boot order: MOT names, the TfTeach facade (its ctor builds the registry), elTeach + InitialTeachEditList ----
    InitialMotorName();                                                         // MOT[].Alias = the teach.ini section names
    fTeach = new TfTeach();
    FileRW_Teach_Boot();
    CHECK(fTeach != 0 && !fTeach->TechPara.empty() && elTeach != 0 && elTeach->FEditList->Count > 0,
          "the facade's registry and elTeach exist (" + I2S((long long)fTeach->TechPara.size()) + " TechPara, " +
          I2S((long long)fTeach->TechTwoPara.size()) + " TechTwoPara, " + I2S(elTeach ? elTeach->FEditList->Count : 0) + " elTeach)");

    // ---- [1] the boot read ----
    printf("[1] the boot read (ReadTechData -> fTeach->ReadFile) on the snapshot copy\n");
    fTeach->ReadFile();
    FileRW_Teach_MirrorProxies();
    const std::string bytes1 = ReadAll(copy);
    const Ini x0 = ParseIni(bytes0), x1 = ParseIni(bytes1);
    {
        const Diff d01 = DiffIni(x0, x1);
        PrintDiff("snapshot -> after the boot read", d01);
        PrintFirstByteDiff(bytes0, bytes1);
        CHECK(Canon(x0) == bytes0 && x0.odd == 0 && !x0.lfOnly, "the snapshot is in TMemIniFile's own layout already (no comment, CRLF, one blank line after every section)");
        //AI(W906-POOL5-13) 20261006: was AI(W906-TEACH-INDEXZ-OUTSHT) 20261005's one-off exception for [MTestZ1] setEditIndex1ToOutSht1Z
        //  (the snapshot then predated that point; it carries it now). Generalized per Frank FR-PR1 2C #13 -- see OnlyExtDefaultsAdded.
        const std::vector<ExtKey> ext = ExtRowsFromTable();
        std::string why;
        const bool extOnly = OnlyExtDefaultsAdded(d01, x1, ext, why);
        CHECK(!ext.empty(), "the W906 extension rows read from WebTeachButtons.gen.inc (" + I2S((long long)ext.size()) + " rows on IO_CARD_TYPE==PCI1203_IO)");
        CHECK(Canon(x1) == bytes1 && x1.odd == 0 && !x1.lfOnly, "[1] after the boot read the file is still in TMemIniFile's own layout");
        CHECK(bytes1 == bytes0 || extOnly, std::string("[1] the boot read leaves the snapshot alone, except that it may add W906 extension-row keys with the default 0 at their section's end (") +
              (bytes1 == bytes0 ? "byte for byte" : I2S((long long)d01.added.size()) + " key(s) added") + ")" + (extOnly ? std::string() : " -- " + why));

        // the rule itself, on made-up diffs (no file): what it must accept and what it must refuse
        printf("[1b] the boot-read rule on made-up diffs\n");
        if (!ext.empty()) {
            Ini b; Sec s; s.name = ext[0].sec; Item it; it.key = ext[0].key; it.val = "0"; s.items.push_back(it); b.secs.push_back(s);
            Change add; add.sec = ext[0].sec; add.key = ext[0].key; add.newV = "0"; add.oldV = "end";
            std::string w;
            Diff ok;      ok.added.push_back(add);
            CHECK(OnlyExtDefaultsAdded(ok, b, ext, w), "[1b] accepts an extension-row key added with 0 at its section's end ([" + add.sec + "] " + add.key + ")");
            Diff nz = ok; nz.added[0].newV = "5";
            CHECK(!OnlyExtDefaultsAdded(nz, b, ext, w), "[1b] refuses an extension-row key added with a non-default value (" + w + ")");
            Diff gold = ok; gold.added[0].key = "setEditInZSafeHeight";
            CHECK(!OnlyExtDefaultsAdded(gold, b, ext, w), "[1b] refuses a golden key added by the boot read (" + w + ")");
            Diff wrongSec = ok; wrongSec.added[0].sec = "NoSuchMotorSection";
            CHECK(!OnlyExtDefaultsAdded(wrongSec, b, ext, w), "[1b] refuses an extension-row key in another section (" + w + ")");
            Diff inside = ok; inside.added[0].oldV = "inside";
            CHECK(!OnlyExtDefaultsAdded(inside, b, ext, w), "[1b] refuses a key added inside its section (" + w + ")");
            Diff chg; Change c; c.sec = ext[0].sec; c.key = ext[0].key; c.oldV = "1"; c.newV = "0"; chg.changed.push_back(c);
            CHECK(!OnlyExtDefaultsAdded(chg, b, ext, w), "[1b] refuses a changed value (" + w + ")");
            Diff rem; rem.removed.push_back(c);
            CHECK(!OnlyExtDefaultsAdded(rem, b, ext, w), "[1b] refuses a removed key (" + w + ")");
            Diff newSec; newSec.secAdded.push_back(ext[0].sec);
            CHECK(OnlyExtDefaultsAdded(newSec, b, ext, w), "[1b] accepts a new section that holds only extension-row keys = 0 ([" + ext[0].sec + "])");
            Ini bJunk = b; Item junk; junk.key = "setEditInZSafeHeight"; junk.val = "0"; bJunk.secs[0].items.push_back(junk);
            CHECK(!OnlyExtDefaultsAdded(newSec, bJunk, ext, w), "[1b] refuses a new section holding a golden key (" + w + ")");
        }
    }

    // ---- [2] the gear save of MInArmX ----
    printf("[2] gearRatioSave's teach.ini step on MInArmX: ratio 1 -> 0.9921524 through the real writer\n");
    std::vector<GearTeachRef> refs;
    std::string why;
    CHECK(GearLiveTeachRefs(refs, why) && !refs.empty(), "GearLiveTeachRefs: the live registry (" + I2S((long long)refs.size()) + " slots)" + (why.empty() ? std::string() : " -- " + why));
    GearPlanInput in;
    in.alias = "MInArmX"; in.mi = MInArmX; in.oldRatio = 1.0; in.newRatio = std::strtod("0.9921524", 0); in.softP = 999999; in.softN = -999999;
    const GearPlan plan = GearPlanBuild(in, refs, [](int mi) { return (USE_PICKER_COUNT == ep1Picker && (mi == 7 || mi == 26)) ? mi - 4 : mi; });   // WebMotorAccessLive.cpp GoldenTeachRemap
    std::vector<std::pair<const int*, int> > values;                            // WebMotorAccess.cpp GearTeachValues(plan, true)
    std::map<SK, std::string> expect;                                           // every slot of every changed variable -> its new text
    std::map<const int*, int> want;                                             // the reload: every registry variable
    for (std::size_t i = 0; i < refs.size(); ++i) want[refs[i].ptr] = refs[i].value;
    for (std::size_t i = 0; i < plan.teach.size(); ++i) {
        const GearTeachChange& c = plan.teach[i];
        want[c.ptr] = c.newV;
        if (c.newV == c.oldV) continue;
        values.push_back(std::make_pair(c.ptr, c.newV));
        for (std::size_t r = 0; r < refs.size(); ++r)
            if (refs[r].ptr == c.ptr) expect[SK(Lower(refs[r].section), Lower(refs[r].key))] = I2S(c.newV);
        printf("    plan: %s %d -> %d (%u slot(s))\n", c.where.c_str(), c.oldV, c.newV, (unsigned)c.slots.size());
    }
    CHECK(plan.ok && plan.changedCount > 0 && plan.changedCount == (int)values.size() && !expect.empty(),
          "the plan (GearPlanBuild, as the dispatcher builds it): " + I2S(plan.changedCount) + " MInArmX teach values change, " + I2S((long long)expect.size()) + " teach.ini keys");
    std::string note;
    const bool saved = GearLiveSetTeach(values, why) && GearLiveTeachSaveReload(true, why, note);
    CHECK(saved, "GearLiveSetTeach + GearLiveTeachSaveReload(true) (FileRW_Teach_SaveFile(true) -> IC_SaveFile -> elTeach->SaveEditTextToFile, ReadFile)" + (why.empty() ? std::string() : " -- " + why));
    printf("    writer note: %.400s\n", note.c_str());
    const std::string bytes2 = ReadAll(copy);
    const Ini x2 = ParseIni(bytes2);
    const Diff d12 = DiffIni(x1, x2);
    PrintDiff("before -> after the gear save", d12);
    PrintFirstByteDiff(bytes1, bytes2);
    std::vector<Change> otherChanged;
    std::set<SK> hit;
    for (std::size_t i = 0; i < d12.changed.size(); ++i) {
        const SK k(Lower(d12.changed[i].sec), Lower(d12.changed[i].key));
        std::map<SK, std::string>::const_iterator e = expect.find(k);
        if (e != expect.end() && e->second == d12.changed[i].newV) hit.insert(k);
        else otherChanged.push_back(d12.changed[i]);
    }
    CHECK(hit.size() == expect.size(), "[2] every key of the plan changed to its new value (" + I2S((long long)hit.size()) + " of " + I2S((long long)expect.size()) + ")");
    CHECK(Canon(x2) == bytes2 && x2.odd == 0 && !x2.lfOnly, "[2] the saved file is in TMemIniFile's own layout: no comment / junk line, CRLF only, one blank line after every section");
    CHECK(d12.removed.empty() && d12.moved.empty() && d12.secRemoved.empty(), "[2] no key or section removed, none moved out of its order");
    std::vector<GearTeachRef> after;                                            // the registry after the save + ReadFile
    {
        int bad = 0;
        std::string first;
        if (!GearLiveTeachRefs(after, why)) ++bad;
        for (std::size_t i = 0; i < after.size(); ++i) {
            std::map<const int*, int>::const_iterator it = want.find(after[i].ptr);
            if (it == want.end() || it->second == after[i].value) continue;
            if (++bad == 1) first = after[i].list + " [" + after[i].section + "] " + after[i].key + "=" + I2S(after[i].value) + " (want " + I2S(it->second) + ")";
        }
        CHECK(bad == 0, "[2] after the save + ReadFile every registry variable is what the plan says (new for MInArmX, unchanged elsewhere)" + (first.empty() ? std::string() : " -- first: " + first));
    }

    // ---- [3] what else the real writer changes: today's behaviour, pinned (golden's writer -- reported, not fixed) ----
    printf("[3] the writer's other changes (golden behaviour, pinned)\n");
    for (std::size_t i = 0; i < otherChanged.size(); ++i) printf("    other value text: [%s] %s %s -> %s\n", otherChanged[i].sec.c_str(), otherChanged[i].key.c_str(), otherChanged[i].oldV.c_str(), otherChanged[i].newV.c_str());
    printf("    keys added: %u, sections added: %u, other values changed: %u\n", (unsigned)d12.added.size(), (unsigned)d12.secAdded.size(), (unsigned)otherChanged.size());
    // (a) the rules, for any snapshot: the writer only ADDS keys of elTeach (golden InitialTeachEditList) the file lacks, at the end
    //     of their section; and changes a value only (i) to the same number in another text -- an elTeach double rewritten with
    //     "%0.6f" (HTEditList iDecimalPoint 6: 0.0000 -> 0.000000) -- or (ii) to the value of the registry variable behind that key
    //     (two slots of one variable that disagreed in the file: the read leaves the last reader's value -- elTeach -- in the
    //     variable, the write puts it in both).
    {
        std::set<std::string> elKeys;
        for (int k = 0; elTeach && k < elTeach->FEditList->Count; ++k) {
            const THTEdit* it = static_cast<const THTEdit*>(elTeach->FEditList->Items[k]);
            if (it) elKeys.insert(Lower(it->IniGroupName.c_str()) + "|" + Lower(it->IniKeyName.c_str()));
        }
        int badAdd = 0, badChg = 0;
        for (std::size_t i = 0; i < d12.added.size(); ++i)
            if (!elKeys.count(Lower(d12.added[i].sec) + "|" + Lower(d12.added[i].key)) || d12.added[i].oldV != "end") {
                if (++badAdd <= 3) printf("    added but not an elTeach key at its section's end: [%s] %s\n", d12.added[i].sec.c_str(), d12.added[i].key.c_str());
            }
        for (std::size_t i = 0; i < otherChanged.size(); ++i) {
            const Change& c = otherChanged[i];
            char* e1 = 0;
            char* e2 = 0;
            const double v1 = std::strtod(c.oldV.c_str(), &e1), v2 = std::strtod(c.newV.c_str(), &e2);
            const bool sameNumber = !c.oldV.empty() && !c.newV.empty() && e1 && *e1 == '\0' && e2 && *e2 == '\0' && v1 == v2;
            bool synced = false;
            for (std::size_t r = 0; r < after.size() && !synced; ++r)
                synced = Lower(after[r].section) == Lower(c.sec) && Lower(after[r].key) == Lower(c.key) && I2S(after[r].value) == c.newV;
            if (sameNumber || synced) continue;
            if (++badChg <= 3) printf("    changed, neither a reformat nor a shared-variable sync: [%s] %s %s -> %s\n", c.sec.c_str(), c.key.c_str(), c.oldV.c_str(), c.newV.c_str());
        }
        CHECK(badAdd == 0 && badChg == 0 && d12.secAdded.empty(),
              "[3] (rules) every other change is an elTeach key added at its section's end, a double rewritten as the same number, or a key synced to its registry variable; no section added");
    }
    // (b) the bytes for THIS snapshot (FNV-1a 64 c72a04f3266400f6, 10844 bytes = machines/HT9050/snapshot c84209ad, git blob
    //     9e9c6dec), measured 20261003: besides the 7 rescaled keys, 48 keys added to [ArmAlignment] --
    //     {In,Out}Arm{PitchX,X,Y}Alignment{A,B}{a..d}=0, elTeach entries this file never had --, [MInArmZE] SetEditAutoClean 0 -> -1220
    //     (TechPara MInArmZE "SetEditAutoClean" and elTeach [InArm] AutoCleanPick share Teach.iAutoCleanPick; the file had 0 / -1220),
    //     the 8 [ArmAlignment] In/OutArmCCDX/Y Resolution / Radian 0.0000 -> 0.000000; 10844 -> 11976 bytes, FNV-1a 64 246ae79b728e69f9.
    //     Layout untouched: same order, CRLF, the blank line after every section, the trailing blank line.
    // (c) what it means for gearRatioSave: its verify step (WebMotorAccess.cpp GearTeachDiffWhy = GearIniDiff, then only the plan's
    //     keys with their new values may differ; "0.0000" == "0.000000") finds 49 keys outside the plan here (48 added + the synced
    //     one) -> the first Gear Ratio save on this file is refused and everything restored, until the file has been through one
    //     Teach-page Save (FileRW/Teach.cpp IC_btnSaveClick, the same writer, makes the same 49 changes; [4] shows the second save
    //     changes nothing). Reported to the laptop (NB2 R171 M2), not changed here.
    {
        unsigned long long h0 = 14695981039346656037ULL, h2 = 14695981039346656037ULL;
        for (std::size_t i = 0; i < bytes0.size(); ++i) { h0 ^= (unsigned char)bytes0[i]; h0 *= 1099511628211ULL; }
        for (std::size_t i = 0; i < bytes2.size(); ++i) { h2 ^= (unsigned char)bytes2[i]; h2 *= 1099511628211ULL; }
        int outsidePlan = 0;
        const std::vector<GearIniDelta> gd = GearIniDiff(bytes1, bytes2);
        for (std::size_t i = 0; i < gd.size(); ++i) {
            std::map<SK, std::string>::const_iterator e = expect.find(SK(Lower(gd[i].section), Lower(gd[i].key)));
            if (!(e != expect.end() && gd[i].hasNew && e->second == gd[i].newV)) ++outsidePlan;
        }
        printf("    snapshot FNV-1a 64 %08x%08x, after the save %08x%08x (%u bytes); GearIniDiff deltas outside the plan: %d\n",
               (unsigned)(h0 >> 32), (unsigned)h0, (unsigned)(h2 >> 32), (unsigned)h2, (unsigned)bytes2.size(), outsidePlan);
        if (h0 == 0xc72a04f3266400f6ULL && bytes0.size() == 10844) {
            std::set<std::string> want48, got48;
            const char* sides[2] = { "In", "Out" };
            const char* kinds[3] = { "PitchX", "X", "Y" };
            const char* cols = "abcd";
            for (int s = 0; s < 2; ++s)
                for (int k = 0; k < 3; ++k)
                    for (int r = 0; r < 2; ++r)
                        for (int c = 0; c < 4; ++c) want48.insert(std::string(sides[s]) + "Arm" + kinds[k] + "Alignment" + (r == 0 ? "A" : "B") + cols[c]);
            bool addOk = true;
            for (std::size_t i = 0; i < d12.added.size(); ++i) {
                got48.insert(d12.added[i].key);
                if (Lower(d12.added[i].sec) != "armalignment" || d12.added[i].newV != "0") addOk = false;
            }
            int syncOk = 0, fmtOk = 0;
            for (std::size_t i = 0; i < otherChanged.size(); ++i) {
                const Change& c = otherChanged[i];
                if (Lower(c.sec) == "minarmze" && c.key == "SetEditAutoClean" && c.oldV == "0" && c.newV == "-1220") ++syncOk;
                if (Lower(c.sec) == "armalignment" && c.key.find("CCD") != std::string::npos && c.oldV == "0.0000" && c.newV == "0.000000") ++fmtOk;
            }
            CHECK(addOk && got48 == want48 && d12.added.size() == 48, "[3] (this snapshot) 48 keys added, exactly [ArmAlignment] {In,Out}Arm{PitchX,X,Y}Alignment{A,B}{a..d}=0");
            CHECK(otherChanged.size() == 9 && syncOk == 1 && fmtOk == 8, "[3] (this snapshot) 9 other values: [MInArmZE] SetEditAutoClean 0 -> -1220 and the 8 CCD doubles 0.0000 -> 0.000000");
            CHECK(bytes2.size() == 11976 && h2 == 0x246ae79b728e69f9ULL, "[3] (this snapshot) the saved file is exactly today's bytes: 11976, FNV-1a 64 246ae79b728e69f9");
            CHECK(outsidePlan == 49, "[3] (this snapshot) GearTeachDiffWhy's rule sees 49 keys outside the plan -> gearRatioSave's verify would refuse and restore this first save (reported, not changed)");
        } else {
            printf("    (another snapshot than the one measured 20261003: the exact counts of (b) / (c) are not checked; the rules of (a) are)\n");
        }
    }

    // ---- [4] a second save, nothing changed ----
    printf("[4] a second save with nothing changed\n");
    note.clear();
    CHECK(GearLiveTeachSaveReload(true, why, note) && ReadAll(copy) == bytes2, "[4] the writer is stable: saving again leaves the bytes as they are");

    // the states stay in the scratch directory for a look (the next run starts by deleting the copy)
    WriteAll(dir + "\\teach.1_after_boot_read.ini", bytes1);
    WriteAll(dir + "\\teach.2_after_gear_save.ini", bytes2);
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
