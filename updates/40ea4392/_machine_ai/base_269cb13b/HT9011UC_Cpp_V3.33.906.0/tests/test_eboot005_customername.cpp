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
//      [E] cross-check with the port's THandlerSystem::GetCustomerName (HandlerSys.cpp:821-852): same list -> equal for every row's code;
//          its own 211-row list (906_0618 DFM) -> differs exactly at 807, 808, 898
//      [F] the live list is what is walked (golden edtSearchCodeChange :1291-1309 filters it)
//      [G] FormLock taken and released on every call
//      [R] source ratchet (argv[1] = port root, read only; comments and '\r' dropped: the gate checkout is CRLF)
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
    { 807, 0, "TFAMD_M",       38 },   // V912 row (not in the port's 906_0618 list)
    { 808, 0, "AMD_US",        39 },   // V912 row (not in the port's 906_0618 list)
    { 865, 0, "HANA",          83 },   // "865 HANA Micron"
    { 889, 0, "CENTER",       106 },   // middle row (213 / 2)
    { 895, 0, "",             112 },   // "895 BARUN": no space after the name -> AnsiPos 0 -> SubString(1,-1) = "" (golden quirk, kept)
    { 898, 0, "TFAMD_SUZHOU", 114 },   // V912 text (the port's 906_0618 list says AMD_SUZHOU)
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

    // E1: same list -> same answer for every code, every QLE switch
    THandlerSystem* hs = new THandlerSystem();   // ht9045_sm; ctor hydrates its own 211 rows (HandlerSys.cpp:794)
    SetItems(hs->rgCustomerList, proxyItems);
    int n = 0, bad = 0;
    for (int qle = 0; qle <= 2; ++qle) {
        for (int c : codes) {
            const std::string a = Name(c, qle);
            const std::string b = hs->GetCustomerName().c_str();
            ++n;
            if (a != b) { ++bad; std::printf("  E1 differs: %s FileRW=\"%s\" port=\"%s\"\n", Tag(c, qle).c_str(), a.c_str(), b.c_str()); }
        }
    }
    Check(n == 3 * (kGoldenRows + 5), "[E1] compared " + std::to_string(n) + " (code, QLE) pairs");
    Check(bad == 0, "[E1] same list: FileRW == port for every pair (" + std::to_string(bad) + " differ)");

    // E2: the port's own list (906_0618 DFM) -> the only differences are the V912 list changes
    THandlerSystem* hs2 = new THandlerSystem();
    const std::vector<std::string> portItems = ItemsOf(hs2->rgCustomerList);
    Check(portItems.size() == 211, "[E2] the port's own list has 211 rows (HandlerSys.cpp:569), got " + std::to_string(portItems.size()));
    std::set<int> all(codes.begin(), codes.end());
    for (const std::string& s : portItems) all.insert(CodeOf(s));
    std::set<int> diff;
    for (int qle = 0; qle <= 1; ++qle) {
        for (int c : all) {
            const std::string a = Name(c, qle);
            const std::string b = hs2->GetCustomerName().c_str();
            if (a != b) {
                diff.insert(c);
                std::printf("  E2 %s: FileRW (V912 list) \"%s\", port (906_0618 list) \"%s\"\n", Tag(c, qle).c_str(), a.c_str(), b.c_str());
            }
        }
    }
    const std::set<int> want = { 807, 808, 898 };
    Check(diff == want, "[E2] own lists differ exactly at 807, 808, 898 (" + std::to_string(diff.size()) + " codes differ)");
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

    // R5: tools/wb_serve.cpp (the laptop's boot slot; read only). Not wired yet = a note, not a failure. Once wired: every call comes
    //   after the FileRW_HSys_Boot() call (before it there is no proxy -> "HonPrec"), and every declaration returns AnsiString.
    if (Load(root, "tools/wb_serve.cpp", &ws)) {
        const std::string t = StripCpp(ws);
        size_t boot = std::string::npos;
        for (size_t p = t.find("FileRW_HSys_Boot()"); p != std::string::npos; p = t.find("FileRW_HSys_Boot()", p + 1)) {
            if (p >= 5 && t.compare(p - 5, 5, "void ") == 0) continue;   // the extern declaration
            if (boot == std::string::npos) boot = p;
        }
        Check(boot != std::string::npos, "[R5] wb_serve.cpp calls FileRW_HSys_Boot()");
        int calls = 0, decls = 0;
        for (size_t p = t.find("FileRW_HSys_CustomerName("); p != std::string::npos; p = t.find("FileRW_HSys_CustomerName(", p + 1)) {
            size_t e = p;
            while (e > 0 && (t[e - 1] == ' ' || t[e - 1] == '\t')) --e;
            size_t b = e;
            while (b > 0 && (std::isalnum((unsigned char)t[b - 1]) || t[b - 1] == '_' || t[b - 1] == ':' || t[b - 1] == '*' || t[b - 1] == '&')) --b;
            const std::string prev = t.substr(b, e - b);
            if (prev.empty() || prev == "return") {
                ++calls;
                Check(boot != std::string::npos && p > boot, "[R5] the FileRW_HSys_CustomerName() call comes after FileRW_HSys_Boot()");
            } else {
                ++decls;
                Check(prev.size() >= 10 && prev.compare(prev.size() - 10, 10, "AnsiString") == 0,
                      "[R5] declared as returning AnsiString (found \"" + prev + "\")");
            }
        }
        if (calls == 0) std::printf("  note: wb_serve.cpp does not call FileRW_HSys_CustomerName() yet (the laptop's boot-slot line)\n");
        else std::printf("  wb_serve.cpp: %d call(s), %d declaration(s)\n", calls, decls);
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
