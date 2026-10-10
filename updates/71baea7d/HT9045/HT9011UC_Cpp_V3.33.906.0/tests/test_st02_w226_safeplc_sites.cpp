// =============================================================================
//  test_st02_w226_safeplc_sites.cpp -- W-226 (SAFEPLC-913 part 2): the csystem.cpp (13) / Command.cpp (1) safe-PLC sites read
//  golden 913's IsSafePLCIOInstall() (cmydef.cpp, W-217) instead of the bool Enable_PLCSafety_IO (RULINGS_20261008 #3).
//
//  AI(W906-W226) 20261010 (St02-E).  Suite name (add_test): St02_W226SafePlcSites.  argv[1] = port root (read only).  Memory only.
//  database.cpp reads the bool and the type from the same [System] SafePlcIO (W217_SafePlcType pins that), so the two always agree:
//  behaviour is unchanged and this test pins the source form, live code only (comments and dead #if 0 / #if 1-#else arms dropped):
//    [1] csystem.cpp: the 13 golden 913 expressions, each the expected number of times, IsSafePLCIOInstall live 13 times and no live
//        Enable_PLCSafety_IO left (golden 913 csystem.cpp :1174 :1433 :2665 :2725 :2731 :2748 :2752 :2795 :4562 :4580 :4584 :4588
//        :17796 = port :19485 :19954 :21216 :21291 :21297 :21325 :21331 :21387 :16730 :16764 :16768 :16780 :30521 at main e22b9f34).
//    [2] Command.cpp: TfMain::MachineStatus reads (IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff())) (golden 913 :7456), once in the
//        file, no live Enable_PLCSafety_IO.
//    [3] the two golden 913 spellings that differ from 0618 are kept: `IsSafePLCIOInstall()==true && bPLCIOEffect==false` (913 :2665,
//        0618 `==1`) and `IsSafePLCIOInstall()==true && Sen[SnAllSafeDoor].IsOff() && bReturnFlag` (913 :2752, 0618 bare).
// =============================================================================
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
bool ReadFile(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}
// Comments out (string / char literals kept), CR dropped, newlines kept -- tests/test_h013_terms.cpp:145 StripComments.
std::string StripComments(const std::string& s)
{
    std::string o;
    size_t i = 0;
    const size_t n = s.size();
    while (i < n)
    {
        const char c = s[i];
        if (c == '\r') { ++i; continue; }
        if (c == '/' && i + 1 < n && s[i + 1] == '/') { while (i < n && s[i] != '\n') ++i; }
        else if (c == '/' && i + 1 < n && s[i + 1] == '*')
        {
            i += 2;
            while (i + 1 < n && !(s[i] == '*' && s[i + 1] == '/')) { if (s[i] == '\n') o += '\n'; ++i; }
            i += 2;
            o += ' ';
        }
        else if (c == '"' || c == '\'')
        {
            const char q = c;
            o += c; ++i;
            while (i < n && s[i] != q && s[i] != '\n')
            {
                if (s[i] == '\\' && i + 1 < n) { o += s[i]; ++i; }
                o += s[i]; ++i;
            }
            if (i < n && s[i] == q) { o += s[i]; ++i; }
        }
        else { o += c; ++i; }
    }
    return o;
}
std::string Trim(const std::string& s)
{
    const size_t a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return std::string();
    return s.substr(a, s.find_last_not_of(" \t") - a + 1);
}
// Live lines: `#if 0` arms and the #else / #elif arms of `#if 1` blanked; other conditions keep every arm -- test_h013_terms.cpp:198.
std::vector<std::string> LiveLines(const std::string& code)
{
    struct Lvl { int cur; bool taken; };
    std::vector<Lvl> st;
    std::vector<std::string> out;
    std::istringstream in(code);
    std::string line;
    while (std::getline(in, line))
    {
        const std::string t = Trim(line);
        bool directive = false;
        if (!t.empty() && t[0] == '#')
        {
            const std::string d = Trim(t.substr(1));
            if (d.compare(0, 2, "if") == 0)
            {
                Lvl l = { -1, false };
                if (d.compare(0, 5, "ifdef") != 0 && d.compare(0, 6, "ifndef") != 0)
                {
                    const std::string e = Trim(d.substr(2));
                    if (e == "0" || e == "(0)") l.cur = 0;
                    else if (e == "1" || e == "(1)") { l.cur = 1; l.taken = true; }
                }
                st.push_back(l);
                directive = true;
            }
            else if ((d.compare(0, 4, "else") == 0 || d.compare(0, 4, "elif") == 0) && !st.empty())
            {
                Lvl& l = st.back();
                if (l.taken) l.cur = 0;
                else if (l.cur == 0 && d.compare(0, 4, "else") == 0) l.cur = 1;
                else l.cur = -1;
                directive = true;
            }
            else if (d.compare(0, 5, "endif") == 0 && !st.empty()) { st.pop_back(); out.push_back(std::string()); continue; }
        }
        bool dead = false;
        for (size_t k = 0; k < st.size(); ++k) if (st[k].cur == 0) dead = true;
        out.push_back(dead || directive ? std::string() : line);
    }
    return out;
}
std::string NoSpace(const std::string& s)
{
    std::string o;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r') o += s[i];
    return o;
}
int Count(const std::string& hay, const std::string& needle)
{
    int n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + 1)) ++n;
    return n;
}
std::string Live(const std::string& path, bool* ok)
{
    std::string raw;
    *ok = ReadFile(path, &raw);
    std::string o;
    const std::vector<std::string> v = LiveLines(StripComments(raw));
    for (size_t i = 0; i < v.size(); ++i) { o += v[i]; o += '\n'; }
    return NoSpace(o);
}
// The live body of TfMain::MachineStatus: from its header to the next column-0 `TfMain::` definition header.
std::string MachineStatus(const std::string& path)
{
    std::string raw;
    if (!ReadFile(path, &raw)) return std::string();
    const std::vector<std::string> v = LiveLines(StripComments(raw));
    const std::string h = "void TfMain::MachineStatus(";
    size_t a = v.size();
    for (size_t i = 0; i < v.size(); ++i) if (v[i].compare(0, h.size(), h) == 0) { a = i; break; }
    std::string o;
    for (size_t i = a; i < v.size(); ++i)
    {
        const std::string& l = v[i];
        if (i > a && !l.empty() && l[0] != ' ' && l[0] != '{' && l[0] != '}' && l[0] != '#' && l.find("TfMain::") != std::string::npos &&
            l.find('(') != std::string::npos)
            break;
        o += l; o += '\n';
    }
    return NoSpace(o);
}
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W226SafePlcSites -- csystem.cpp / Command.cpp read golden 913's IsSafePLCIOInstall() (W-226)\n");
    const std::string root = argc > 1 ? argv[1] : std::string();

    std::printf("[1] csystem.cpp\n");
    {
        bool ok = false;
        const std::string cs = Live(root + "/csystem.cpp", &ok);
        Check(ok && cs.size() > 100000, "[1] csystem.cpp read (" + std::to_string(cs.size()) + " live bytes)");
        const struct { std::string expr; int want; const char* golden; } r[] = {
            { NoSpace("(IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff()))"), 1, "913 :1174 DoSystem EMG" },
            { NoSpace("if(IsSafePLCIOInstall())"), 2, "913 :1433 / :17796" },
            { NoSpace("if(IsSafePLCIOInstall()==true && bPLCIOEffect==false)"), 1, "913 :2665 CheckSafeDoorIsClosed" },
            { NoSpace("if(IsSafePLCIOInstall()==1)"), 3, "913 :2725 / :2731 / :2795" },
            { NoSpace("if(IsSafePLCIOInstall() && bCheckPLCConnet()==false)"), 2, "913 :2748 / :4562" },
            { NoSpace("if(IsSafePLCIOInstall()==true && Sen[SnAllSafeDoor].IsOff() && bReturnFlag)"), 1, "913 :2752" },
            { NoSpace("else if(IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff()) ShowErrorMessage(\"WAR16140\", K_RETRY, MMSystem);"), 1, "913 :4580" },
            { NoSpace("if(IsSafePLCIOInstall() && bPLCDisconnet)"), 1, "913 :4584" },
            { NoSpace("if(IsSafePLCIOInstall() && bCheckPLCAllSafedoorAndEMGEnable()==false)"), 1, "913 :4588-4589 (two lines)" },
        };
        for (size_t i = 0; i < sizeof(r) / sizeof(r[0]); ++i)
        {
            const int n = Count(cs, r[i].expr);
            Check(n == r[i].want, "[1] " + r[i].expr + " live " + std::to_string(n) + " time(s), want " + std::to_string(r[i].want) +
                                      " (golden " + r[i].golden + ")");
        }
        const int sites = Count(cs, "IsSafePLCIOInstall()");
        const int bools = Count(cs, "Enable_PLCSafety_IO");
        Check(sites == 13, "[1] IsSafePLCIOInstall() live " + std::to_string(sites) + " time(s) in csystem.cpp, want 13 (golden 913's 13 sites)");
        Check(bools == 0, "[1] the bool Enable_PLCSafety_IO live " + std::to_string(bools) + " time(s) in csystem.cpp, want 0");
    }

    std::printf("[2] Command.cpp\n");
    {
        bool ok = false;
        const std::string cm = Live(root + "/Command.cpp", &ok);
        const std::string ms = MachineStatus(root + "/Command.cpp");
        Check(ok && !ms.empty() && ms.find(NoSpace("(IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff()))")) != std::string::npos,
              "[2] TfMain::MachineStatus reads (IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff())) (golden 913 Command.cpp:7456)");
        Check(Count(cm, "IsSafePLCIOInstall()") == 1 && Count(cm, "Enable_PLCSafety_IO") == 0,
              "[2] Command.cpp: IsSafePLCIOInstall() live once, the bool Enable_PLCSafety_IO not at all");
    }

    std::printf("[3] golden 913's own spelling kept where 0618 differed\n");
    {
        bool ok = false;
        const std::string cs = Live(root + "/csystem.cpp", &ok);
        Check(Count(cs, NoSpace("IsSafePLCIOInstall()==1 && bPLCIOEffect==false")) == 0 &&
                  Count(cs, NoSpace("if(IsSafePLCIOInstall() && Sen[SnAllSafeDoor].IsOff() && bReturnFlag)")) == 0,
              "[3] not 0618's `==1 && bPLCIOEffect` / bare `&& Sen[SnAllSafeDoor]` forms (913 :2665 / :2752 say ==true)");
    }

    std::printf("St02_W226SafePlcSites: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
