// =============================================================================
//  test_eboot005_customername.cpp -- FileRW_HSys_CustomerName (end of FileRW/HSys.cpp) = golden THandlerSystem::GetCustomerName.
//
//  //AI(W906-EBOOT005) 20261001 [W906] (St01): new file. Jimmy TO_STEVEN 20261001 12:3x, census 129 (e): golden TfMain::FormShow
//    (V912 main.cpp:11056-11057) RunInfo.Factory=HandlerSystem->GetCustomerName(); never ran in the port (no HandlerSystem global).
//    golden = HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp:1242-1272; list = V912 HandlerSys.dfm rgCustomerList (213 rows),
//    the proxy FileRW/HSys.gen.inc:1115 fills at boot.
//
//  Two executables from this one file:
//    test_eboot005_customername           HSys.cpp + _EditList.cpp + _EditPage.cpp compiled in, god-stack by RESCAN (as test_hsys_heater_mix):
//      [A] not booted: no proxy -> "HonPrec" (golden's empty-list path), and the call creates no proxy
//      [B] FileRW_HSys_Boot: 213 rows, DFM order pinned at the rows used below
//      [C] golden names per CUSTOMER_CODE / SPIL_FOR_QLE (first, middle, last row, the 3 fixed names, the "" quirk, codes not in the list)
//      [D] ItemIndex -1 / 0 / middle / last / out of range changes nothing (golden never reads it)
//      [E] cross-check with the port's THandlerSystem::GetCustomerName (HandlerSys.cpp:821-852).  //AI(W906-D044) 20261002 [W906] (St01):
//          Jimmy RULINGS_20261002 #9 -- 807 / 808 / 898 take the 906 list's names (FileRW/HSys.cpp W906_HSys906Row), so now:
//          its own 211-row list (906_0618 DFM) -> equal for every code; the V912 list in both -> differs exactly at 807, 808, 898
//          (was the other way round before D-044)
//      [F] the live list is what is walked (golden edtSearchCodeChange :1291-1309 filters it)
//      [G] FormLock taken and released on every call
//      [R] source ratchet (argv[1] = port root, read only; comments and '\r' dropped: the gate checkout is CRLF)
//      [H] //AI(W906-D043) 20261002: cObserver.cpp's labFactory note (W906_ObsFactoryNoteNeeded): golden's own "" (895 / 970) is no note
//      [R5] boot slot: wb_serve.cpp FileRW_HSys_Boot() then W906_Boot_RunInfoFactory(); tools/wb_boot_factory.cpp calls the lookup
//           (W906_EBOOT005_R5_ROOT = copies of those two files, control run); [R6] the cObserver.cpp guard and the HSys.cpp hook
//    test_eboot005_customername_fallback  (W906_EBOOT005_FALLBACK_ONLY) links ht9045_globals without HSys.cpp, so the
//      FileRW/_fallback.cpp line is the definition that runs: "HonPrec".
//  Writes nothing: the HSys proxies are memory only; the real Gerneral.ini / config.ini are compared before and after.
// =============================================================================
#ifdef W906_EBOOT005_FALLBACK_ONLY

#include "vclcompat/AnsiString.h"

#include <cstdio>
#include <string>

vclcompat::AnsiString FileRW_HSys_CustomerName();   // FileRW/_fallback.cpp (ht9045_globals): the only definition in this exe

int main()
{
    const std::string got = FileRW_HSys_CustomerName().c_str();
    const bool ok = (got == "HonPrec");
    std::printf("%s: FileRW/_fallback.cpp FileRW_HSys_CustomerName() = \"%s\" (want \"HonPrec\", golden V912 HandlerSys.cpp:1244/:1271)\n",
                ok ? "PASS" : "FAIL", got.c_str());
    std::printf("test_eboot005_customername_fallback: %d passed, %d failed\n", ok ? 1 : 0, ok ? 0 : 1);
    return ok ? 0 : 1;
}

#else  // !W906_EBOOT005_FALLBACK_ONLY

#include "FileRW/_EditList.h"
#include "forms/fHandlerSys.h"
#include "cmydef.h"
#include "common.h"
#include "MachineType.h"
#include "w906_ctest_guard.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

AnsiString FileRW_HSys_CustomerName();   // FileRW/HSys.cpp, end of file (the unit under test)
void FileRW_HSys_Boot();                 // FileRW/HSys.cpp (golden CreateForm(THandlerSystem) HT9045.cpp:210)
bool W906_ObsFactoryNoteNeeded(const AnsiString& caption);   //AI(W906-D043) 20261002 [W906] (St01): end of cObserver.cpp (ht9045_sm)
extern AnsiString (*W906_ObsGoldenFactoryHook)();             //AI(W906-D043): same place; FileRW/HSys.cpp installs FileRW_HSys_CustomerName

// Link only (not under test), as test_hsys_heater_mix.cpp:45-50: cprod.cpp (ht9045_globals) calls this one; the real one is
//   FileRW/IniConfig.cpp, not compiled here. Without it the linker pulls ht9045_globals' FileRW/_fallback.cpp member, which also
//   defines FileRW_ProxyChecked (real one: FileRW/_EditList.cpp, compiled here) and FileRW_HSys_CustomerName (real one: FileRW/HSys.cpp,
//   compiled here) -> multiple definition. Same body as FileRW/_fallback.cpp.
void FileRW_IniConfig_ChangeCBListProperty() {}

// FileRW/HSys.cpp and FileRW/_EditList.cpp take the FormJson lock (JsonBridge/FormJson.cpp, wb_serve only): count it.
namespace ht9045 { namespace formjson {
int g_lockCalls = 0, g_unlockCalls = 0, g_depth = 0;
void FormLock()   { ++g_lockCalls; ++g_depth; }
void FormUnlock() { ++g_unlockCalls; --g_depth; }
} }

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; return; }
    ++g_fail;
    std::printf("FAIL: %s\n", what.c_str());
}
void CheckEq(const std::string& got, const std::string& want, const std::string& what)
{
    if (got == want) { ++g_pass; return; }
    ++g_fail;
    std::printf("FAIL: %s\n  got : \"%s\"\n  want: \"%s\"\n", what.c_str(), got.c_str(), want.c_str());
}

std::string Name()
{
    return std::string(FileRW_HSys_CustomerName().c_str());
}
std::string Name(int code, int qle)
{
    CUSTOMER_CODE = code;
    SPIL_FOR_QLE = qle;
    return Name();
}
std::string Tag(int code, int qle)
{
    std::ostringstream o;
    o << "CUSTOMER_CODE=" << code << " SPIL_FOR_QLE=" << qle;
    return o.str();
}

TRadioGroup* Proxy()
{
    return dynamic_cast<TRadioGroup*>(filerw::ELFind("THandlerSystem", "rgCustomerList"));
}
std::vector<std::string> ItemsOf(TRadioGroup* rg)
{
    std::vector<std::string> v;
    for (int i = 0; rg && i < rg->Items->Count; ++i) {
        const AnsiString s = rg->Items->Strings[i];
        v.push_back(s.c_str());
    }
    return v;
}
void SetItems(TRadioGroup* rg, const std::vector<std::string>& v)
{
    rg->Items->Clear();
    for (const std::string& s : v) rg->Items->Add(s.c_str());
}
int CodeOf(const std::string& item)
{
    return std::atoi(item.substr(0, 3).c_str());   // golden :1248 atoi(SubString(1, 3))
}

bool ReadFile(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream o;
    o << f.rdbuf();
    *out = o.str();
    return true;
}

// ---- golden table: V912 HandlerSys.dfm rows, worked by hand from golden :1242-1272 (and cross-read from the DFM bytes, Big5) ----
struct Row { int code; int qle; const char* want; int index; };   // index = row position in the DFM list, -1 = not in the list
const Row kGolden[] = {
    {   0, 0, "HonPrec",        0 },   // first row "000 HonPrec ..." (else branch: the name happens to equal the default)
    { 731, 0, "CARSEM",         1 },   // golden :1259 fixed name (row text "731 Carsem_Thai ...")
    { 731, 1, "CARSEM",         1 },
    { 730, 0, "Infineon",       2 },   // golden :1255 fixed name (row text "730 IFXTH ...")
    { 730, 1, "Infineon",       2 },
    { 740, 0, "Ramos",          3 },
    { 764, 0, "Elmos",          9 },   // "764 Elmos Germany ..." -> up to the first space
    { 807, 0, "HonPrec",       38 },   // V912 row "807 TFAMD_M"; //AI(W906-D044) RULINGS_20261002 #9: 906 has no 807 row -> golden :1244 default
    { 807, 1, "HonPrec",       38 },
    { 808, 0, "HonPrec",       39 },   // V912 row "808 AMD_US";  //AI(W906-D044): 906 has no 808 row -> "HonPrec"
    { 865, 0, "HANA",          83 },   // "865 HANA Micron"
    { 889, 0, "CENTER",       106 },   // middle row (213 / 2)
    { 895, 0, "",             112 },   // "895 BARUN": no space after the name -> AnsiPos 0 -> SubString(1,-1) = "" (golden quirk, kept)
    { 898, 0, "AMD_SUZHOU",   114 },   // V912 text "898 TFAMD_SUZHOU"; //AI(W906-D044): 906's row (HandlerSys.cpp:682) "898 AMD_SUZHOU"
    { 898, 1, "AMD_SUZHOU",   114 },
    { 897, 0, "SILTERRA_CHINKIANG", 113 },   // //AI(W906-D044): the row before 898 is untouched (V912 name)
    { 806, 0, "STM",           37 },   // //AI(W906-D044): the row before 807 is untouched
    { 809, 0, "IMEC",          40 },   // //AI(W906-D044): the row after 808 is untouched
    { 910, 0, "SPIL",         126 },
    { 910, 1, "SPIL",         126 },   // QLE only for 912 (golden :1251)
    { 912, 0, "SPIL",         128 },
    { 912, 1, "QLE",          128 },   // golden :1251-1253 Steven 20230110 : For渠梁
    { 912, 2, "SPIL",         128 },   // SPIL_FOR_QLE==1 only
    { 937, 0, "ASE",          151 },   // "937 ASE Malaysia" + full-width spaces: the ASCII space after "ASE" wins
    { 970, 0, "",             183 },   // "970 GIGAS" + U+3000 only: no ASCII space -> "" (golden quirk, kept; Big5 A1 40 has no 0x20 either)
    { 985, 0, "MAXIM",        198 },   // "985 MAXIM " (trailing space)
    { 999, 0, "Qualcomm",     212 },   // last row
    {   1, 0, "HonPrec",       -1 },   // not in the list -> golden :1271 default
    { 896, 0, "HonPrec",       -1 },
    { 896, 1, "HonPrec",       -1 },
    {1000, 0, "HonPrec",       -1 },
    {  -1, 0, "HonPrec",       -1 },
};
const int kGoldenRows = 213;

// ---- [A] ----------------------------------------------------------------------------------------------------------------------
void CaseNotBooted()
{
    std::printf("[A] not booted (FileRW_HSys_Boot not run yet)\n");
    Check(Proxy() == nullptr, "[A] no rgCustomerList proxy before FileRW_HSys_Boot");
    CheckEq(Name(999, 0), "HonPrec", "[A] " + Tag(999, 0) + ": no proxy = empty list -> golden :1271 default");
    CheckEq(Name(731, 0), "HonPrec", "[A] " + Tag(731, 0) + ": the fixed names are inside the loop, not reached");
    CheckEq(Name(912, 1), "HonPrec", "[A] " + Tag(912, 1));
    Check(Proxy() == nullptr, "[A] the call created no proxy (ELFind, not EL<>)");
}

// ---- [B] ----------------------------------------------------------------------------------------------------------------------
std::vector<std::string> CaseBoot()
{
    std::printf("[B] FileRW_HSys_Boot\n");
    FileRW_HSys_Boot();
    TRadioGroup* rg = Proxy();
    Check(rg != nullptr, "[B] rgCustomerList proxy exists after boot");
    const std::vector<std::string> items = ItemsOf(rg);
    Check((int)items.size() == kGoldenRows, "[B] 213 rows (V912 HandlerSys.dfm), got " + std::to_string(items.size()));
    for (const Row& r : kGolden) {
        if (r.index < 0) continue;
        const bool at = r.index < (int)items.size() && CodeOf(items[r.index]) == r.code;
        Check(at, "[B] row " + std::to_string(r.index) + " carries code " + std::to_string(r.code) + " (DFM order is load-bearing)");
    }
    std::set<int> codes;
    for (const std::string& s : items) codes.insert(CodeOf(s));
    Check(codes.size() == items.size(), "[B] no code appears twice (first equal row wins)");
    for (const Row& r : kGolden)
        if (r.index < 0) Check(codes.count(r.code) == 0, "[B] code " + std::to_string(r.code) + " is not in the list");
    return items;
}

// ---- [C] ----------------------------------------------------------------------------------------------------------------------
void CaseGolden()
{
    std::printf("[C] golden names\n");
    for (const Row& r : kGolden) CheckEq(Name(r.code, r.qle), r.want, "[C] " + Tag(r.code, r.qle));
}

//AI(W906-D043) 20261002 [W906] (St01): [H] cObserver.cpp's labFactory note (W906_ObserverJson :8108 -> W906_ObsFactoryNoteNeeded).
//  Golden has no note; an empty labFactory caption is golden's own value for 895 / 970, so only a caption golden would have filled
//  counts as "the boot step did not run".  Booted proxy here (after [B]).
void CaseObserverNote()
{
    std::printf("[H] Observer labFactory note\n");
    Check(W906_ObsGoldenFactoryHook == &FileRW_HSys_CustomerName, "[H] FileRW/HSys.cpp installed its lookup at static init");
    Name(895, 0);
    Check(!W906_ObsFactoryNoteNeeded(""), "[H] 895 BARUN: empty caption = golden's \"\" -> no note");
    Name(970, 0);
    Check(!W906_ObsFactoryNoteNeeded(""), "[H] 970 GIGAS: empty caption = golden's \"\" -> no note");
    Name(868, 0);
    Check(W906_ObsFactoryNoteNeeded(""), "[H] 868: golden gives a name, caption empty -> note (boot step missing)");
    Check(!W906_ObsFactoryNoteNeeded("CYUEAN"), "[H] a caption -> no note");
    Name(807, 0);   //AI(W906-D044) 20261002: 906 name "HonPrec" (not ""): an empty caption is a missing boot step
    Check(W906_ObsFactoryNoteNeeded(""), "[H] 807 (906 name HonPrec): caption empty -> note");
    Name(898, 0);
    Check(W906_ObsFactoryNoteNeeded(""), "[H] 898 (906 name AMD_SUZHOU): caption empty -> note");
    AnsiString (*const keep)() = W906_ObsGoldenFactoryHook;
    W906_ObsGoldenFactoryHook = 0;
    Name(895, 0);
    Check(W906_ObsFactoryNoteNeeded(""), "[H] no lookup installed (binaries without FileRW/HSys.cpp): empty caption -> note, as before");
    W906_ObsGoldenFactoryHook = keep;
}

// ---- [D] ----------------------------------------------------------------------------------------------------------------------
void CaseItemIndex()
{
    std::printf("[D] ItemIndex is not read\n");
    TRadioGroup* rg = Proxy();
    if (!rg) { Check(false, "[D] no proxy"); return; }
    const int keep = rg->ItemIndex;
    Check(keep == 0, "[D] boot ItemIndex = 0 (DFM), got " + std::to_string(keep));
    const int idx[] = { -1, 0, 106, 212, 213, 100000 };
    const Row probe[] = { {0, 0, "HonPrec", 0}, {889, 0, "CENTER", 106}, {999, 0, "Qualcomm", 212}, {896, 0, "HonPrec", -1} };
    for (int ix : idx) {
        rg->ItemIndex = ix;
        for (const Row& r : probe)
            CheckEq(Name(r.code, r.qle), r.want, "[D] ItemIndex=" + std::to_string(ix) + " " + Tag(r.code, r.qle));
    }
    rg->ItemIndex = keep;
}

// ---- [E] ----------------------------------------------------------------------------------------------------------------------
void CaseCrossCheck(const std::vector<std::string>& proxyItems)
{
    std::printf("[E] cross-check with the port's THandlerSystem::GetCustomerName (HandlerSys.cpp:821-852)\n");
    std::vector<int> codes;
    for (const std::string& s : proxyItems) codes.push_back(CodeOf(s));
    const int extra[] = { -999, -1, 1, 896, 1000 };
    for (int c : extra) codes.push_back(c);

    //AI(W906-D044) 20261002 [W906] (St01): E1 / E2 swapped their expectation (RULINGS_20261002 #9, 906 names for 807 / 808 / 898).
    // E1: the V912 list in both -> FileRW (906 names for the three) differs from the port's plain walk exactly at 807, 808, 898
    THandlerSystem* hs = new THandlerSystem();   // ht9045_sm; ctor hydrates its own 211 rows (HandlerSys.cpp:794)
    SetItems(hs->rgCustomerList, proxyItems);
    int n = 0;
    std::set<int> diff1;
    for (int qle = 0; qle <= 2; ++qle) {
        for (int c : codes) {
            const std::string a = Name(c, qle);
            const std::string b = hs->GetCustomerName().c_str();
            ++n;
            if (a != b) { diff1.insert(c); std::printf("  E1 %s: FileRW=\"%s\" port (V912 list)=\"%s\"\n", Tag(c, qle).c_str(), a.c_str(), b.c_str()); }
        }
    }
    const std::set<int> want = { 807, 808, 898 };
    Check(n == 3 * (kGoldenRows + 5), "[E1] compared " + std::to_string(n) + " (code, QLE) pairs");
    Check(diff1 == want, "[E1] V912 list: FileRW differs from the plain walk exactly at 807, 808, 898 (" + std::to_string(diff1.size()) + " codes differ)");

    // E2: the port's own list (906_0618 DFM) -> FileRW (on the V912 proxy) gives 906's answer for every code
    THandlerSystem* hs2 = new THandlerSystem();
    const std::vector<std::string> portItems = ItemsOf(hs2->rgCustomerList);
    Check(portItems.size() == 211, "[E2] the port's own list has 211 rows (HandlerSys.cpp:569), got " + std::to_string(portItems.size()));
    std::set<int> all(codes.begin(), codes.end());
    for (const std::string& s : portItems) all.insert(CodeOf(s));
    std::set<int> diff;
    int n2 = 0;
    for (int qle = 0; qle <= 2; ++qle) {
        for (int c : all) {
            const std::string a = Name(c, qle);
            const std::string b = hs2->GetCustomerName().c_str();
            ++n2;
            if (a != b) {
                diff.insert(c);
                std::printf("  E2 %s: FileRW (V912 list + 906 rows) \"%s\", port (906_0618 list) \"%s\"\n", Tag(c, qle).c_str(), a.c_str(), b.c_str());
            }
        }
    }
    Check(n2 >= 3 * 211, "[E2] compared " + std::to_string(n2) + " (code, QLE) pairs");
    Check(diff.empty(), "[E2] FileRW == the port's 906 translation on its own list for every code (" + std::to_string(diff.size()) + " codes differ)");
}

// ---- [F] ----------------------------------------------------------------------------------------------------------------------
void CaseLiveList(const std::vector<std::string>& proxyItems)
{
    std::printf("[F] the live list is walked (golden edtSearchCodeChange :1300-1308 filter, search text \"ASE\")\n");
    TRadioGroup* rg = Proxy();
    if (!rg) { Check(false, "[F] no proxy"); return; }
    std::vector<std::string> filtered;
    for (const std::string& s : proxyItems) {
        const AnsiString up = AnsiString(s.c_str()).UpperCase();
        if (up.AnsiPos("ASE") != 0) filtered.push_back(s);
    }
    SetItems(rg, filtered);
    CheckEq(Name(999, 0), "HonPrec", "[F] filtered list without 999 -> default");
    CheckEq(Name(937, 0), "ASE", "[F] filtered list with 937");
    //AI(W906-D044) 20261002: search "AMD" keeps the V912 rows 807 / 808 / 898 (and 982): the 906 names still come out
    filtered.clear();
    for (const std::string& s : proxyItems)
        if (AnsiString(s.c_str()).UpperCase().AnsiPos("AMD") != 0) filtered.push_back(s);
    SetItems(rg, filtered);
    Check(filtered.size() >= 3, "[F] search AMD keeps at least the three V912 rows, got " + std::to_string(filtered.size()));
    CheckEq(Name(807, 0), "HonPrec", "[F] filtered (AMD) 807 -> 906: no row -> default");
    CheckEq(Name(808, 0), "HonPrec", "[F] filtered (AMD) 808 -> default");
    CheckEq(Name(898, 0), "AMD_SUZHOU", "[F] filtered (AMD) 898 -> 906 row text");
    SetItems(rg, proxyItems);
    CheckEq(Name(999, 0), "Qualcomm", "[F] full list back");
}

// ---- [G] ----------------------------------------------------------------------------------------------------------------------
void CaseLock()
{
    std::printf("[G] FormLock\n");
    using namespace ht9045::formjson;
    const int l0 = g_lockCalls, u0 = g_unlockCalls;
    (void)Name(999, 0);
    Check(g_lockCalls > l0, "[G] FormLock taken by the call");
    Check(g_lockCalls - l0 == g_unlockCalls - u0, "[G] every FormLock released");
    Check(g_depth == 0, "[G] lock depth back to 0");
}

// ---- [R] ----------------------------------------------------------------------------------------------------------------------
std::string DropCR(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    for (char c : s) if (c != '\r') o += c;
    return o;
}
// C/C++: drop // and /* */ comments, keep string and char literals.
std::string StripCpp(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    size_t i = 0;
    const size_t n = s.size();
    while (i < n) {
        const char c = s[i];
        if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            while (i < n && s[i] != '\n') ++i;
        } else if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(s[i] == '*' && s[i + 1] == '/')) { if (s[i] == '\n') o += '\n'; ++i; }
            i += 2;
            o += ' ';
        } else if (c == '"' || c == '\'') {
            const char q = c;
            o += c; ++i;
            while (i < n && s[i] != q && s[i] != '\n') {
                if (s[i] == '\\' && i + 1 < n) { o += s[i]; ++i; }
                o += s[i]; ++i;
            }
            if (i < n) { o += s[i]; ++i; }
        } else {
            o += c; ++i;
        }
    }
    return o;
}
// CMake: drop # comments outside "..." strings.
std::string StripCMake(const std::string& s)
{
    std::string o;
    bool inStr = false;
    for (size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (inStr) {
            o += c;
            if (c == '\\' && i + 1 < s.size()) { o += s[++i]; continue; }
            if (c == '"') inStr = false;
        } else if (c == '"') {
            inStr = true; o += c;
        } else if (c == '#') {
            while (i < s.size() && s[i] != '\n') ++i;
            o += '\n';
        } else {
            o += c;
        }
    }
    return o;
}
std::string Squash(const std::string& s)
{
    std::string o;
    bool sp = false;
    for (char c : s) {
        if (c == ' ' || c == '\t' || c == '\n') { sp = true; continue; }
        if (sp && !o.empty()) o += ' ';
        sp = false;
        o += c;
    }
    return o;
}
size_t Count(const std::string& hay, const std::string& needle)
{
    size_t k = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + 1)) ++k;
    return k;
}
// From the '{' at or after `from`, the text up to the matching '}' (literals skipped).
std::string BlockFrom(const std::string& s, size_t from)
{
    const size_t open = s.find('{', from);
    if (open == std::string::npos) return "";
    int depth = 0;
    for (size_t i = open; i < s.size(); ++i) {
        const char c = s[i];
        if (c == '"' || c == '\'') {
            const char q = c;
            for (++i; i < s.size() && s[i] != q; ++i) if (s[i] == '\\') ++i;
            continue;
        }
        if (c == '{') ++depth;
        if (c == '}' && --depth == 0) return s.substr(open, i - open + 1);
    }
    return "";
}
// The text of a CMake command `head(...)` (parentheses balanced), or "".
std::string CMakeCall(const std::string& s, size_t at)
{
    int depth = 0;
    for (size_t i = s.find('(', at); i != std::string::npos && i < s.size(); ++i) {
        if (s[i] == '(') ++depth;
        if (s[i] == ')' && --depth == 0) return s.substr(at, i - at + 1);
    }
    return "";
}
bool Load(const std::string& root, const char* rel, std::string* out)
{
    std::string raw;
    if (!ReadFile(root + "/" + rel, &raw)) { Check(false, std::string("[R] cannot read ") + rel); return false; }
    *out = DropCR(raw);
    return true;
}

void CaseRatchet(const std::string& root)
{
    std::printf("[R] source ratchet (%s)\n", root.c_str());
    std::string hs, fb, cm, el, ws;

    // R1/R2: FileRW/HSys.cpp -- one definition, at the end, golden's statements in golden's order
    if (Load(root, "FileRW/HSys.cpp", &hs)) {
        const std::string t = StripCpp(hs);
        const std::string sig = "AnsiString FileRW_HSys_CustomerName()";
        Check(Count(t, sig) == 1, "[R1] HSys.cpp defines FileRW_HSys_CustomerName once");
        const size_t at = t.find(sig);
        const size_t edge = t.find("const char* FileRW_HSys_WindowEdge(bool open)");
        Check(at != std::string::npos && edge != std::string::npos && at > edge, "[R1] it is appended after FileRW_HSys_WindowEdge (end of file)");
        const std::string body = Squash(at == std::string::npos ? std::string() : BlockFrom(t, at));
        const char* const seq[] = {
            "AnsiString Str=\"HonPrec\", tmp;",                                 // golden :1244
            "int iCustomerCode;",                                               // :1245
            "for(int i=0; i<rgCustomerList->Items->Count; i++)",               // :1246
            "iCustomerCode=atoi(item.SubString(1, 3).c_str());",               // :1248
            "if(iCustomerCode==CUSTOMER_CODE)",                                // :1249
            "if(CUSTOMER_CODE==CC_SPIL_CHINA_SUZHOU && SPIL_FOR_QLE==1)",      // :1251
            "Str=\"QLE\";",                                                    // :1253
            "else if(CUSTOMER_CODE==CC_IFXTH_Thai)",                           // :1255
            "Str=\"Infineon\";",                                               // :1257
            "else if(CUSTOMER_CODE==CC_Carsem_Thai)",                          // :1259
            "Str=\"CARSEM\";",                                                 // :1261
            "tmp=item.SubString(5, item.Length());",                           // :1265
            "Str=tmp.SubString(1, tmp.AnsiPos(\" \")-1);",                     // :1266
            "return Str;",                                                     // :1268
            "return Str;",                                                     // :1271
        };
        size_t p = 0;
        for (const char* s : seq) {
            const size_t q = body.find(s, p);
            Check(q != std::string::npos, std::string("[R2] golden statement in order: ") + s);
            if (q != std::string::npos) p = q + 1;
        }
        //AI(W906-D044) 20261002 [W906] (St01): the 906 rows (RULINGS_20261002 #9) are applied between golden :1248 and :1249, once
        {
            const std::string ov = "if(!W906_HSys906Row(iCustomerCode, &item)) continue;";
            const size_t a1248 = body.find("iCustomerCode=atoi(item.SubString(1, 3).c_str());");
            const size_t a1249 = body.find("if(iCustomerCode==CUSTOMER_CODE)");
            const size_t o = body.find(ov);
            Check(Count(body, ov) == 1 && o != std::string::npos && a1248 != std::string::npos && a1249 != std::string::npos && a1248 < o && o < a1249,
                  "[R2] D-044: the 906-row override sits between golden :1248 and :1249, once");
            const size_t hdef = t.find("static bool W906_HSys906Row(int iCode, AnsiString* item)");
            const std::string hb = Squash(hdef == std::string::npos ? std::string() : BlockFrom(t, hdef));
            Check(hdef != std::string::npos && hdef < at, "[R2] D-044: W906_HSys906Row is defined before FileRW_HSys_CustomerName");
            const char* const rows[] = { "case 807: return false;", "case 808: return false;",
                                         "case CC_AMD_SUZHOU: *item=\"898 AMD_SUZHOU AMD 蘇州\"; return true;", "default: return true;" };
            for (const char* r : rows) Check(hb.find(r) != std::string::npos, std::string("[R2] D-044: W906_HSys906Row has ") + r);
            Check(Count(hb, "case ") == 3, "[R2] D-044: exactly three 906 rows (807, 808, 898), got " + std::to_string(Count(hb, "case ")));
        }
        Check(body.find("filerw::ELFind(\"THandlerSystem\", \"rgCustomerList\")") != std::string::npos, "[R2] reads the proxy with ELFind");
        Check(body.find("EL<") == std::string::npos, "[R2] no EL<> (would create an empty proxy before boot)");
        Check(body.find("ItemIndex") == std::string::npos, "[R2] does not read ItemIndex (golden does not)");
        Check(body.find("ht9045::formjson::FormLock()") != std::string::npos && body.find("ht9045::formjson::FormUnlock()") != std::string::npos,
              "[R2] takes and releases FormLock");
    }

    // R3: FileRW/_fallback.cpp -- one line, after the S09-Q3 fallbacks, returns golden's default
    if (Load(root, "FileRW/_fallback.cpp", &fb)) {
        const std::string t = Squash(StripCpp(fb));
        Check(Count(t, "FileRW_HSys_CustomerName()") == 1, "[R3] _fallback.cpp defines FileRW_HSys_CustomerName once");
        Check(t.find("vclcompat::AnsiString FileRW_HSys_CustomerName() { return vclcompat::AnsiString(\"HonPrec\"); }") != std::string::npos,
              "[R3] the fallback returns \"HonPrec\" (golden :1244/:1271)");
        const size_t a = t.find("FileRW_ProxySetItemIndex"), b = t.find("FileRW_HSys_CustomerName");
        Check(a != std::string::npos && b != std::string::npos && b > a, "[R3] appended after the existing fallbacks");
    }

    // R4: the fallback's precondition (FileRW/_fallback.cpp header): FileRW/_fallback.cpp is an ht9045_globals member, and
    //   FileRW/HSys.cpp is compiled straight into wb_serve (FileRW/_editlist_sources.cmake), never into an archive -- else the two
    //   definitions would be archive members and whichever is pulled first would win silently.
    if (Load(root, "CMakeLists.txt", &cm) && Load(root, "FileRW/_editlist_sources.cmake", &el)) {
        const std::string t = StripCMake(cm);
        const size_t g = t.find("add_library(ht9045_globals");
        Check(g != std::string::npos && CMakeCall(t, g).find("FileRW/_fallback.cpp") != std::string::npos,
              "[R4] FileRW/_fallback.cpp is in add_library(ht9045_globals ...)");
        int libs = 0, bad = 0;
        for (size_t p = t.find("add_library("); p != std::string::npos; p = t.find("add_library(", p + 1)) {
            ++libs;
            const std::string call = CMakeCall(t, p);
            if (call.find("FileRW/HSys.cpp") != std::string::npos || call.find("W906_EDITLIST_SRC") != std::string::npos ||
                call.find("W906_FILERW_SRC") != std::string::npos) {
                ++bad;
                std::printf("  R4: %s\n", call.substr(0, 80).c_str());
            }
        }
        Check(libs > 0 && bad == 0, "[R4] no add_library carries FileRW/HSys.cpp (" + std::to_string(libs) + " add_library calls)");
        const size_t w = t.find("add_executable(wb_serve");
        Check(w != std::string::npos && CMakeCall(t, w).find("${W906_FILERW_SRC}") != std::string::npos, "[R4] wb_serve compiles ${W906_FILERW_SRC}");
        Check(StripCMake(el).find("FileRW/HSys.cpp") != std::string::npos, "[R4] FileRW/_editlist_sources.cmake lists FileRW/HSys.cpp");
    }

    // R5: the boot slot (read only).  //AI(W906-D043) 20261002 [W906] (St01): since the laptop's batch 23 (AI(W906-B23-EBOOT005)) the
    //   call is in tools/wb_boot_factory.cpp W906_Boot_RunInfoFactory (golden TfMain::FormShow V912 main.cpp:11056-11057), which
    //   tools/wb_serve.cpp calls at boot; the old R5 scanned only wb_serve.cpp for FileRW_HSys_CustomerName and so could never go red.
    //   Now: wb_serve.cpp calls FileRW_HSys_Boot() and, AFTER it (before it there is no proxy -> "HonPrec"), W906_Boot_RunInfoFactory();
    //   wb_boot_factory.cpp calls FileRW_HSys_CustomerName() inside W906_Boot_RunInfoFactory; every declaration returns AnsiString; a
    //   direct call left in wb_serve.cpp must come after the boot too.  Comments and '\r' dropped (Load + StripCpp).
    //   W906_EBOOT005_R5_ROOT = a folder holding tools/wb_serve.cpp + tools/wb_boot_factory.cpp copies (control run only).
    {
        const char* r5env = std::getenv("W906_EBOOT005_R5_ROOT");
        const std::string r5root = (r5env && *r5env) ? std::string(r5env) : root;
        std::string bf;
        if (Load(r5root, "tools/wb_serve.cpp", &ws) && Load(r5root, "tools/wb_boot_factory.cpp", &bf)) {
            // first CALL of `name()` (an `void name()` declaration / definition is skipped)
            auto firstCall = [](const std::string& t, const std::string& name) {
                for (size_t p = t.find(name + "()"); p != std::string::npos; p = t.find(name + "()", p + 1)) {
                    if (p >= 5 && t.compare(p - 5, 5, "void ") == 0) continue;
                    if (p > 0 && (std::isalnum((unsigned char)t[p - 1]) || t[p - 1] == '_')) continue;   // a longer name
                    return p;
                }
                return std::string::npos;
            };
            // FileRW_HSys_CustomerName( occurrences: calls (nothing or `return` / `=` before) and declarations (a type before)
            auto scan = [](const std::string& t, const char* where, std::vector<size_t>* calls, int* decls) {
                for (size_t p = t.find("FileRW_HSys_CustomerName("); p != std::string::npos; p = t.find("FileRW_HSys_CustomerName(", p + 1)) {
                    size_t e = p;
                    while (e > 0 && (t[e - 1] == ' ' || t[e - 1] == '\t')) --e;
                    size_t b = e;
                    while (b > 0 && (std::isalnum((unsigned char)t[b - 1]) || t[b - 1] == '_' || t[b - 1] == ':' || t[b - 1] == '*' || t[b - 1] == '&')) --b;
                    const std::string prev = t.substr(b, e - b);
                    if (prev.empty() || prev == "return") {
                        calls->push_back(p);
                    } else {
                        ++*decls;
                        Check(prev.size() >= 10 && prev.compare(prev.size() - 10, 10, "AnsiString") == 0,
                              std::string("[R5] ") + where + " declares it as returning AnsiString (found \"" + prev + "\")");
                    }
                }
            };
            const std::string t = StripCpp(ws);
            const size_t boot = firstCall(t, "FileRW_HSys_Boot");
            const size_t fac = firstCall(t, "W906_Boot_RunInfoFactory");
            Check(boot != std::string::npos, "[R5] wb_serve.cpp calls FileRW_HSys_Boot()");
            Check(fac != std::string::npos, "[R5] wb_serve.cpp calls W906_Boot_RunInfoFactory() (tools/wb_boot_factory.cpp)");
            Check(boot != std::string::npos && fac != std::string::npos && fac > boot,
                  "[R5] the W906_Boot_RunInfoFactory() call comes after FileRW_HSys_Boot() (golden order: proxy first)");
            std::vector<size_t> wsCalls;
            int wsDecls = 0;
            scan(t, "wb_serve.cpp", &wsCalls, &wsDecls);
            for (size_t p : wsCalls)
                Check(boot != std::string::npos && p > boot, "[R5] a direct FileRW_HSys_CustomerName() call in wb_serve.cpp comes after FileRW_HSys_Boot()");

            const std::string f = StripCpp(bf);
            std::vector<size_t> fCalls;
            int fDecls = 0;
            scan(f, "wb_boot_factory.cpp", &fCalls, &fDecls);
            const size_t def = f.find("void W906_Boot_RunInfoFactory()");
            const std::string body = def == std::string::npos ? std::string() : BlockFrom(f, def);
            const size_t open = def == std::string::npos ? std::string::npos : f.find('{', def);
            int inBody = 0;
            for (size_t p : fCalls) if (open != std::string::npos && p > open && p < open + body.size()) ++inBody;
            Check(def != std::string::npos, "[R5] wb_boot_factory.cpp defines W906_Boot_RunInfoFactory()");
            Check(inBody >= 1, "[R5] wb_boot_factory.cpp calls FileRW_HSys_CustomerName() inside W906_Boot_RunInfoFactory()");
            Check(Squash(body).find("RunInfo.Factory=FileRW_HSys_CustomerName();") != std::string::npos,
                  "[R5] ... as golden main.cpp:11056 RunInfo.Factory=HandlerSystem->GetCustomerName();");
            std::printf("  wb_serve.cpp: boot call at %d, factory call at %d, %d direct call(s); wb_boot_factory.cpp: %d call(s) (%d in the body), %d declaration(s)\n",
                        (int)(boot == std::string::npos ? -1 : (long)boot), (int)(fac == std::string::npos ? -1 : (long)fac), (int)wsCalls.size(),
                        (int)fCalls.size(), inBody, fDecls);
        }
    }

    // R6: //AI(W906-D043) 20261002 [W906] (St01): cObserver.cpp's labFactory note asks W906_ObsFactoryNoteNeeded (end of that file),
    //   not the bare Caption.IsEmpty() (that called golden's own "" for 895 / 970 a missing boot step); FileRW/HSys.cpp installs the lookup.
    {
        std::string ob;
        if (Load(root, "cObserver.cpp", &ob) && Load(root, "FileRW/HSys.cpp", &hs)) {
            const std::string t = StripCpp(ob);
            const size_t note = t.find(",\\\"labFactory\\\":\" + W906Obs_QS(kW906ObsNoFac)");
            const size_t cond = t.rfind("if (", note);
            const std::string ifLine = (note == std::string::npos || cond == std::string::npos) ? std::string() : t.substr(cond, note - cond);
            Check(ifLine.find("W906_ObsFactoryNoteNeeded(f->labFactory->Caption)") != std::string::npos &&
                  ifLine.find("Caption.IsEmpty()") == std::string::npos,
                  "[R6] cObserver.cpp: the labFactory note is guarded by W906_ObsFactoryNoteNeeded(f->labFactory->Caption)");
            Check(Count(t, "bool W906_ObsFactoryNoteNeeded(const AnsiString& caption)\n{") == 1, "[R6] cObserver.cpp defines W906_ObsFactoryNoteNeeded once");
            Check(Squash(StripCpp(hs)).find("W906_ObsGoldenFactoryHook = &FileRW_HSys_CustomerName;") != std::string::npos,
                  "[R6] FileRW/HSys.cpp installs FileRW_HSys_CustomerName as the lookup");
        }
    }
}

struct RealGuard { std::string path, bytes; bool had; };

}  // namespace

int main(int argc, char** argv)
{
    const char* const rt[] = { "asGeneralPath", asGeneralPath.c_str(), 0, 0 };
    if (!W906TestRequireCtestRedirects("EBoot005_CustomerName", rt))
        return 2;
    if (argc < 2) { std::printf("usage: test_eboot005_customername <port root>\n"); return 2; }
    std::vector<RealGuard> guard = { { "D:\\HT9045\\system\\Gerneral.ini", "", false }, { "D:\\HT9045\\config\\config.ini", "", false } };
    for (RealGuard& g : guard) g.had = ReadFile(g.path, &g.bytes);
    const int keepCode = CUSTOMER_CODE, keepQle = SPIL_FOR_QLE;

    CaseNotBooted();
    const std::vector<std::string> items = CaseBoot();
    CaseGolden();
    CaseObserverNote();   //AI(W906-D043) 20261002 [W906] (St01)
    CaseItemIndex();
    CaseCrossCheck(items);
    CaseLiveList(items);
    CaseLock();
    CaseRatchet(argv[1]);

    CUSTOMER_CODE = keepCode;
    SPIL_FOR_QLE = keepQle;
    for (const RealGuard& g : guard) {
        std::string now;
        const bool had = ReadFile(g.path, &now);
        Check(had == g.had && now == g.bytes, "real machine file unchanged: " + g.path);
    }
    std::printf("test_eboot005_customername: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

#endif  // W906_EBOOT005_FALLBACK_ONLY
