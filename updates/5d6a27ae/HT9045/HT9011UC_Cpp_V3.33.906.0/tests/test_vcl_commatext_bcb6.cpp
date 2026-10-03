// ===========================================================================
//  tests/test_vcl_commatext_bcb6.cpp -- ctest VclCommaTextBcb6
//  AI(W906-COMMATEXT) 20261003 (INBOX 152)
//
//  vclcompat::TStringList CommaText / DelimitedText (setter AND getter) must behave
//  exactly like BCB6.  The reference is not an opinion: tests/oracle/commatext_bcb6.cpp
//  was compiled with BCB6 bcc32 5.6.4 against the real VCL (rtl.lib) and its output was
//  pinned as tests/oracle/commatext_bcb6_expected.txt.  This test runs the SAME case table
//  and driver (tests/oracle/commatext_cases.inc) against vclcompat and compares every line.
//
//  usage: test_vcl_commatext_bcb6 <path to commatext_bcb6_expected.txt>
// ===========================================================================
#include "vclcompat/TStringList.h"
#include "vclcompat/Controls.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using vclcompat::AnsiString;
using vclcompat::TObject;
using vclcompat::TStringList;

#include "oracle/commatext_cases.inc"

namespace {

int g_fail = 0;

#define CHECK(c) do { if (!(c)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)

// Oracle lines that are NOT a reference for the port, with the port's own expectation.
// An entry without a blank names a whole case id (all its oracle lines are skipped).
struct PortOnly { const char* key; const char* why; };
const PortOnly kPortOnly[] = {
    // AnsiExtractQuotedStr, unterminated content ending in a doubled quote: SetLength is one
    // short (sysutils.pas:3925) and the Move of the pair loop (:3932) writes the doubled quote
    // over the item's NUL terminator.  The ITEM is the same in both ("a" / one quote char), but
    // BCB6's GetDelimitedText then scans PChar(S) past Length into that clobbered byte and
    // whatever heap bytes follow -- undefined, so the oracle's "get" line is not a reference.
    { "S26 get", "BCB6 reads the clobbered NUL terminator of item \"a\"" },
    { "S29 get", "BCB6 reads the clobbered NUL terminator of item '\"'" },
    // CharNext under ACP 950 hides a Big5 trail byte 0x7C from the '|' delimiter; the port's
    // strings are UTF-8 and it steps bytes (TStringList.cpp, "CharNext").
    { "V01 set", "Big5 trail byte under an ASCII delimiter in 0x40..0x7E" },
    // AnsiStrScan follows SysLocale.FarEast (thread locale), so the oracle line moves with the
    // oracle box's locale; on the pinned run (FarEast=0) it equals the port's anyway.
    { "V02",     "UTF-8 bytes under an MBCS-aware AnsiStrScan" },
    // BCB6 raises EAccessViolation (SetLength(Result, 0) then Move through PChar('') =
    // System's read-only @@zeroByte); the port returns '' for that item and carries on.
    { "R01",     "BCB6: EAccessViolation" },
    { "R02",     "BCB6: EAccessViolation (after adding \"a\")" }
};

// The port's lines for the kPortOnly keys, exactly.
const char* const kPortOnlyExpected[] = {
    "S26 get [a]",
    "S29 get [\"\"\"\"]",
    "V01 set n=3 [x\\xA4] [y] [z]",
    "V02 set n=2 [\\xE5\\x85\\xA7] [x]",
    "V02 get [\\xE5\\x85\\xA7,x]",
    "R01 set n=1 []",
    "R01 get [\"\"]",
    "R02 set n=2 [a] []",
    "R02 get [a,]"
};

std::string IdOf(const std::string& line)  { return line.substr(0, line.find(' ')); }
std::string KeyOf(const std::string& line)
{
    const size_t a = line.find(' ');
    if (a == std::string::npos) return line;
    const size_t b = line.find(' ', a + 1);
    return line.substr(0, b);
}

bool IsPortOnly(const std::string& line)
{
    for (size_t i = 0; i < sizeof(kPortOnly) / sizeof(kPortOnly[0]); ++i) {
        const std::string k = kPortOnly[i].key;
        if (k.find(' ') == std::string::npos ? IdOf(line) == k : KeyOf(line) == k) return true;
    }
    return false;
}

std::vector<std::string> SplitLines(const std::string& text)
{
    std::vector<std::string> out;
    std::string cur;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') { out.push_back(cur); cur.clear(); }
        else if (text[i] != '\r') cur += text[i];      // the pinned file may come back CRLF (core.autocrlf)
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

// Everything the oracle printed, in the oracle's order: table, then R01, then R02.
std::string RunPort()
{
    AnsiString out;
    int i;
    for (i = 0; i < CtNumSet(); ++i)   { TStringList* sl = new TStringList; CtRunSetCase(kCtSet[i], sl, out); delete sl; }
    for (i = 0; i < CtNumGet(); ++i)   { TStringList* sl = new TStringList; CtRunGetCase(kCtGet[i], sl, out); delete sl; }
    for (i = 0; i < CtNumBehav(); ++i) { TStringList* sl = new TStringList; CtRunBehav(i, sl, out);        delete sl; }
    for (i = 0; i < CtNumRisky(); ++i) { TStringList* sl = new TStringList; CtRunSetCase(kCtRisky[i], sl, out); delete sl; }
    return out.str();
}

void CompareWithOracle(const char* expectedPath)
{
    std::ifstream in(expectedPath, std::ios::binary);
    CHECK(static_cast<bool>(in));
    if (!in) { std::printf("cannot open %s\n", expectedPath); return; }
    std::ostringstream ss;
    ss << in.rdbuf();
    std::vector<std::string> oracleAll = SplitLines(ss.str());
    std::vector<std::string> portAll   = SplitLines(RunPort());

    std::vector<std::string> oracle, port, portOnly;
    for (size_t i = 0; i < oracleAll.size(); ++i)
        if (!oracleAll[i].empty() && oracleAll[i][0] != '#' && !IsPortOnly(oracleAll[i])) oracle.push_back(oracleAll[i]);
    for (size_t i = 0; i < portAll.size(); ++i)
        (IsPortOnly(portAll[i]) ? portOnly : port).push_back(portAll[i]);

    // 1. every compared line, in order
    int mismatches = 0;
    const size_t n = oracle.size() > port.size() ? oracle.size() : port.size();
    for (size_t i = 0; i < n; ++i) {
        const std::string o = i < oracle.size() ? oracle[i] : std::string("<missing>");
        const std::string p = i < port.size()   ? port[i]   : std::string("<missing>");
        if (o != p) {
            if (mismatches < 40) std::printf("MISMATCH\n  bcb6: %s\n  port: %s\n", o.c_str(), p.c_str());
            ++mismatches;
        }
    }
    CHECK(mismatches == 0);
    CHECK(oracle.size() == port.size());
    // a pinned file with almost nothing in it would make the comparison vacuous
    CHECK(oracle.size() >= 200);

    // 2. the port-only lines, exactly
    const size_t ne = sizeof(kPortOnlyExpected) / sizeof(kPortOnlyExpected[0]);
    CHECK(portOnly.size() == ne);
    for (size_t i = 0; i < ne && i < portOnly.size(); ++i) {
        if (portOnly[i] != kPortOnlyExpected[i]) {
            std::printf("PORT-ONLY MISMATCH\n  want: %s\n  got:  %s\n", kPortOnlyExpected[i], portOnly[i].c_str());
            ++g_fail;
        }
    }
    // 3. every port-only key really occurs in the oracle file (no stale entries)
    for (size_t k = 0; k < sizeof(kPortOnly) / sizeof(kPortOnly[0]); ++k) {
        bool seen = false;
        for (size_t i = 0; i < oracleAll.size() && !seen; ++i)
            if (!oracleAll[i].empty() && oracleAll[i][0] != '#') {
                const std::string key = kPortOnly[k].key;
                seen = key.find(' ') == std::string::npos ? IdOf(oracleAll[i]) == key : KeyOf(oracleAll[i]) == key;
            }
        if (!seen) { std::printf("stale kPortOnly entry: %s\n", kPortOnly[k].key); ++g_fail; }
    }
    std::printf("compared %d lines with the BCB6 oracle, %d mismatches; %d port-only lines checked\n",
                static_cast<int>(oracle.size()), mismatches, static_cast<int>(portOnly.size()));
}

std::vector<std::string> Items(TStringList* sl)
{
    std::vector<std::string> v;
    for (int i = 0; i < sl->Count; ++i) v.push_back(AnsiString(sl->Strings[i]).str());
    return v;
}

std::vector<std::string> SetComma(const char* s)
{
    TStringList* sl = new TStringList;
    sl->CommaText = s;
    std::vector<std::string> v = Items(sl);
    delete sl;
    return v;
}

std::string GetComma(const std::vector<std::string>& items)
{
    TStringList* sl = new TStringList;
    for (size_t i = 0; i < items.size(); ++i) sl->Add(AnsiString(items[i]));
    const std::string t = AnsiString(sl->CommaText).str();
    delete sl;
    return t;
}

// The brief's examples, spelled out (the table above covers them too; these read as documentation).
void ReadableExamples()
{
    typedef std::vector<std::string> V;
    CHECK(SetComma("07 Tester I/F") == (V{"07", "Tester", "I/F"}));
    CHECK(SetComma("2025-08-13,10:20:30.123,07 Tester I/F,JAM0701")
          == (V{"2025-08-13", "10:20:30.123", "07", "Tester", "I/F", "JAM0701"}));
    CHECK(SetComma("a,b,") == (V{"a", "b", ""}));          // classes.pas:4412-4421 CharNext(P1)^ = #0
    CHECK(SetComma("a, ")  == (V{"a"}));
    CHECK(SetComma("a,,b") == (V{"a", "", "b"}));
    CHECK(SetComma("")     == (V{}));
    CHECK(SetComma("\"ab\"cd,e") == (V{"ab", "cd", "e"}));
    CHECK(SetComma("\"abc") == (V{"ab"}));                 // sysutils.pas:3919-3922
    CHECK(GetComma(V{"a", "", "b"}) == "a,,b");            // classes.pas:4103-4109
    CHECK(GetComma(V{""}) == "\"\"");                      // classes.pas:4094-4095
    CHECK(GetComma(V{}) == "");
    CHECK(GetComma(V{"a b", "c"}) == "\"a b\",c");
    CHECK(GetComma(V{"x\"y"}) == "\"x\"\"y\"");

    // SetCommaText leaves Delimiter / QuoteChar at ',' / '"' (classes.pas:4306-4307) ...
    TStringList* sl = new TStringList;
    sl->Delimiter = ';';
    sl->QuoteChar = '\'';
    sl->CommaText = "a;b,c";
    CHECK(sl->Count == 2);
    CHECK(sl->Delimiter == ',');
    CHECK(sl->QuoteChar == '"');
    // ... while GetCommaText restores them (classes.pas:4073-4083).
    sl->Delimiter = ';';
    const std::string t = AnsiString(sl->CommaText).str();
    CHECK(t == "a;b,c");
    CHECK(sl->Delimiter == ';');
    delete sl;

    // Round trip of a production-log style row with empty fields: written bare, read back the same.
    const V row{"PMLD1019", "", "3", "", "", "7 8", "x\"y", ""};
    TStringList* r = new TStringList;
    r->CommaText = AnsiString(GetComma(row));
    CHECK(Items(r) == row);
    CHECK(GetComma(row) == "PMLD1019,,3,,,\"7 8\",\"x\"\"y\",");
    delete r;
}

// SetDelimitedText's Clear is the non-virtual TStringList::Clear: TRadioGroupItems' override
// imitates VCL's ItemsChange, which VCL runs at EndUpdate against the FINAL Count, so it must
// not fire before the Adds (it would clamp ItemIndex against an empty list).
void RadioGroupItemsNotClampedEarly()
{
    vclcompat::TRadioGroup rg;
    rg.Items->Add("a");
    rg.Items->Add("b");
    rg.Items->Add("c");
    rg.ItemIndex = 2;
    rg.Items->CommaText = "x,y,z";
    CHECK(rg.ItemIndex == 2);
    CHECK(rg.Items->Count == 3);
    rg.Items->Clear();                                     // the explicit Clear() still clamps (W906-D024)
    CHECK(rg.ItemIndex == -1);
}

// AI(W906-COMMATEXT-FU) 20261003: NB2-1 (INBOX 152 follow-up) -- vclcompat::CommaTextCells (the same CtSetDelimited run as
// SetCommaText, plus each item's bytes; used by WebMotorAccess.cpp MtSplitCells) and TStrings.Assign copying Delimiter /
// QuoteChar (classes.pas:3964-3965).
void CellsAndAssign()
{
    const std::string s = "a,b c,\"x,y\"z,";              // 0 a | 1 , | 2 b | 3 ' ' | 4 c | 5 , | 6 "x,y" .. 10 | 11 z | 12 ,
    std::vector<vclcompat::CommaTextCell> cc;
    vclcompat::CommaTextCells(s, cc);
    vclcompat::TStringList sl;
    sl.CommaText = AnsiString(s);
    CHECK(cc.size() == 6 && sl.Count == 6);
    bool same = cc.size() == static_cast<size_t>(sl.Count);
    for (size_t k = 0; same && k < cc.size(); ++k) same = (cc[k].v == AnsiString(sl.Strings[static_cast<int>(k)]).str());
    CHECK(same);                                           // the values ARE CommaText's items
    if (cc.size() == 6) {
        CHECK(cc[0].b == 0 && cc[0].e == 1 && !cc[0].quoted && cc[0].v == "a");
        CHECK(cc[1].b == 2 && cc[1].e == 3 && cc[1].v == "b");          // a blank ends an unquoted item (BCB6)
        CHECK(cc[2].b == 4 && cc[2].e == 5 && cc[2].v == "c");
        CHECK(cc[3].b == 6 && cc[3].e == 11 && cc[3].quoted && cc[3].v == "x,y");   // quotes included in the span
        CHECK(cc[4].b == 11 && cc[4].e == 12 && !cc[4].quoted && cc[4].v == "z");  // text after the closing quote = the next item
        CHECK(cc[5].b == 13 && cc[5].e == 13 && cc[5].v.empty() && !cc[5].quoted); // the trailing "" item at the end, unquoted
    }
    vclcompat::CommaTextCells("", cc);
    CHECK(cc.empty());
    vclcompat::CommaTextCells("   ", cc);
    CHECK(cc.empty());

    vclcompat::TStringList a, b;
    a.Delimiter = ';';
    a.QuoteChar = '\'';
    a.Add("x");
    b.Assign(&a);
    CHECK(b.Delimiter == ';' && b.QuoteChar == '\'' && b.Count == 1);
    b.Add("p q");
    CHECK(AnsiString(b.DelimitedText).str() == "x;'p q'");  // b uses a's Delimiter / QuoteChar after Assign, as VCL
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: %s <commatext_bcb6_expected.txt>\n", argv[0]); return 2; }
    CompareWithOracle(argv[1]);
    ReadableExamples();
    RadioGroupItemsNotClampedEarly();
    CellsAndAssign();                                      // AI(W906-COMMATEXT-FU) 20261003
    if (g_fail) { std::printf("VclCommaTextBcb6: %d FAILED\n", g_fail); return 1; }
    std::printf("VclCommaTextBcb6: all OK\n");
    return 0;
}
