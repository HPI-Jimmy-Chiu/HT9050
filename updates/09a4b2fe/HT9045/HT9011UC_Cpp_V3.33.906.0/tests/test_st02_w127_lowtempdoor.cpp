// =============================================================================
// test_st02_w127_lowtempdoor.cpp -- ctest St02_W127LowTempDoor.
// AI(W906-W127) 20261006 (St02-E): card W-127 (laptop TO_STEVEN s4 18:2x; RULINGS_20261006 #22 (4), Frank: needed) -- the
//   cold-temperature door check is wired as golden: TempCtrl/TriTemp.cpp calls LowTempIdleCheckSafeDoor() live at golden 0618
//   TempCtrl/TriTemp.cpp:77 (DoTriTempState) and :281 (fCheckDewPointStatus); csystem.cpp defines it live (golden 0618
//   csystem.cpp:3892) and csystem.h declares it.  The two `#if 0` gates (TriTemp.cpp:586 / :799, "no translated home") were
//   stale since PT-W5c 6977fc3a (20260810) put the body in csystem.cpp.
//  Why source pins and no behaviour test: the body is all `#ifndef SOFT_SIMULTE` (golden), so a SIM build cannot observe it;
//  [5]b pins that posture so the reader knows why.
//  CODE ONLY: comments stripped (a commented-out `//#if 0` disappears), then lines inside a dead `#if 0` arm are dropped
//  (`#if 0` dead / its `#else` live; `#if 1` live / its `#else` dead; any other #if / #ifdef / #ifndef live in both arms).
//  argv[1] = the tree root (read only).  No machine state, no writes.
//  REVERSE CHECKS (St02-E ran them, reverse_w127.py): restore `#if 0` at TriTemp.cpp:586 -> [1] [2]a [4] red; at :799 ->
//    [1] [2]b [4] red; move the :587 call above its `if(...iWorkTemp<25)` -> [3]a red; gate csystem.cpp's body -> [5] red.
// =============================================================================
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int g_total = 0, g_fail = 0;
#define CHECK(c, what) do { ++g_total; if (!(c)) { ++g_fail; std::printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, what); } \
                            else std::printf("  ok   %s\n", what); } while (0)

static std::string ReadSource(const std::string& root, const char* rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::ostringstream o;
    o << f.rdbuf();
    return o.str();
}

// Strip // and /* */ comments; keep string and char literals as they are (test_st02_keep912.cpp's CodeOnly).
static std::string CodeOnly(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ) {
        const char c = s[i];
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') {
            while (i < s.size() && s[i] != '\n') ++i;
        } else if (c == '/' && i + 1 < s.size() && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) { if (s[i] == '\n') out += '\n'; ++i; }
            i += 2;
        } else if (c == '"' || c == '\'') {
            out += c;
            ++i;
            while (i < s.size() && s[i] != c && s[i] != '\n') {
                if (s[i] == '\\' && i + 1 < s.size()) { out += s[i]; ++i; }
                out += s[i];
                ++i;
            }
            if (i < s.size()) { out += s[i]; ++i; }
        } else {
            out += c;
            ++i;
        }
    }
    return out;
}

static std::string Trim(const std::string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r')) --b;
    return s.substr(a, b - a);
}

// The code lines that the compiler sees (index = 0-based source line); dead lines become "".
static std::vector<std::string> LiveLines(const std::string& src)
{
    std::vector<std::string> lines;
    std::istringstream in(CodeOnly(src));
    std::string l;
    while (std::getline(in, l)) lines.push_back(l);
    struct Frame { bool ifLive, elseLive, inElse; };
    std::vector<Frame> st;
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string t = Trim(lines[i]);
        bool live = true;
        for (size_t k = 0; k < st.size(); ++k) live = live && (st[k].inElse ? st[k].elseLive : st[k].ifLive);
        if (t.compare(0, 1, "#") == 0) {
            const std::string d = Trim(t.substr(1));
            if (d.compare(0, 2, "if") == 0) {
                const bool zero = (d == "if 0" || d.compare(0, 5, "if 0 ") == 0 || d.compare(0, 5, "if 0\t") == 0);
                const bool one = (d == "if 1" || d.compare(0, 5, "if 1 ") == 0);
                st.push_back(Frame{ !zero, !one, false });
            } else if (d.compare(0, 4, "else") == 0 || d.compare(0, 4, "elif") == 0) {
                if (!st.empty()) st.back().inElse = true;
            } else if (d.compare(0, 5, "endif") == 0) {
                if (!st.empty()) st.pop_back();
            }
        }
        if (!live) lines[i].clear();
    }
    return lines;
}

static int FindLine(const std::vector<std::string>& L, const char* needle, size_t from = 0)
{
    for (size_t i = from; i < L.size(); ++i) if (L[i].find(needle) != std::string::npos) return (int)i;
    return -1;
}

static int PrevIf(const std::vector<std::string>& L, int from)
{
    for (int i = from - 1; i >= 0; --i) if (L[(size_t)i].find("if(") != std::string::npos) return i;
    return -1;
}

int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: test_st02_w127_lowtempdoor <tree root>\n"); return 2; }
    const std::string root = argv[1];
    const std::string tt = ReadSource(root, "TempCtrl/TriTemp.cpp"), cs = ReadSource(root, "csystem.cpp"), ch = ReadSource(root, "csystem.h");
    if (tt.empty() || cs.empty() || ch.empty()) { std::printf("  FAIL cannot read the three sources under %s\n", root.c_str()); return 1; }
    const std::vector<std::string> T = LiveLines(tt), C = LiveLines(cs), H = LiveLines(ch);

    std::printf("[1]-[4] TempCtrl/TriTemp.cpp\n");
    std::vector<int> calls;
    for (size_t i = 0; i < T.size(); ++i) if (T[i].find("LowTempIdleCheckSafeDoor();") != std::string::npos) calls.push_back((int)i);
    std::printf("  (live calls at :%d :%d)\n", calls.size() > 0 ? calls[0] + 1 : 0, calls.size() > 1 ? calls[1] + 1 : 0);
    CHECK(calls.size() == 2, "[1] LowTempIdleCheckSafeDoor(); is called live exactly twice (golden 0618 TriTemp.cpp:77 / :281)");
    const int fA = FindLine(T, "void DoTriTempState()"), fB = FindLine(T, "void fCheckDewPointStatus()"), fC = FindLine(T, "bool fCheckMotorMoveCount_Shuttle(");
    int inA = 0, inB = 0;
    for (size_t k = 0; k < calls.size(); ++k) {
        if (fA >= 0 && fB > fA && calls[k] > fA && calls[k] < fB) ++inA;
        if (fB >= 0 && fC > fB && calls[k] > fB && calls[k] < fC) ++inB;
    }
    CHECK(inA == 1, "[2]a one live call inside DoTriTempState() (golden 0618 TriTemp.cpp:77)");
    CHECK(inB == 1, "[2]b one live call inside fCheckDewPointStatus() (golden 0618 TriTemp.cpp:281)");
    if (calls.size() == 2) {
        const int i1 = PrevIf(T, calls[0]);
        CHECK(i1 >= 0 && T[(size_t)i1].find("W906_FormShowing(\"fTeach\", W7TT_FTeach->fShow)==false && iWorkTemp<25") != std::string::npos,
              "[3]a call 1 sits under golden :75 if(fTeach not shown && iWorkTemp<25)");
        const int i2 = PrevIf(T, calls[1]);
        const int i2b = i2 >= 0 ? PrevIf(T, i2) : -1;
        CHECK(i2 >= 0 && T[(size_t)i2].find("W906_FormShowing(\"fTeach\", W7TT_FTeach->fShow)==false") != std::string::npos &&
              i2b >= 0 && T[(size_t)i2b].find("if(bStartCheck==true)") != std::string::npos,
              "[3]b call 2 sits under golden :277 if(bStartCheck==true) / :279 if(fTeach not shown)");
    } else {
        CHECK(false, "[3]a call 1 sits under golden :75 if(fTeach not shown && iWorkTemp<25)");
        CHECK(false, "[3]b call 2 sits under golden :277 if(bStartCheck==true) / :279 if(fTeach not shown)");
    }
    int if0 = 0;
    for (size_t i = 0; i < T.size(); ++i) { const std::string t = Trim(T[i]); if (t.compare(0, 5, "#if 0") == 0) ++if0; }
    CHECK(if0 == 0, "[4] TriTemp.cpp has no live `#if 0` gate left");

    std::printf("[5]-[6] csystem.cpp / csystem.h\n");
    int defs = 0, defAt = -1;
    for (size_t i = 0; i < C.size(); ++i)
        if (C[i].find("bool LowTempIdleCheckSafeDoor(void)") != std::string::npos && C[i].find(';') == std::string::npos) { ++defs; defAt = (int)i; }
    CHECK(defs == 1, "[5]a csystem.cpp defines bool LowTempIdleCheckSafeDoor(void) live exactly once (golden 0618 csystem.cpp:3892)");
    bool order = false;
    if (defAt >= 0) {
        int end = defAt + 1;
        while ((size_t)end < C.size() && C[(size_t)end].compare(0, 1, "}") != 0) ++end;
        const int pSim = FindLine(C, "#ifndef SOFT_SIMULTE", (size_t)defAt);
        const int p25 = FindLine(C, "ShowErrorMessage(\"MES1625\", K_RETRY, MMSystem)", (size_t)defAt);
        const int p26 = FindLine(C, "ShowErrorMessage(\"MES1626\", K_RETRY, MMSystem)", (size_t)defAt);
        const int p27 = FindLine(C, "ShowErrorMessage(\"MES1627\", K_RETRY, MMSystem)", (size_t)defAt);
        const int p28 = FindLine(C, "ShowErrorMessage(\"MES1628\", K_RETRY, MMSystem)", (size_t)defAt);
        order = pSim > defAt && pSim < p25 && p25 < p26 && p26 < p27 && p27 < p28 && p28 < end;
    }
    CHECK(order, "[5]b its body: #ifndef SOFT_SIMULTE, then MES1625 / MES1626 / MES1627 / MES1628 with K_RETRY (golden; SIM is inert)");
    int decl = 0;
    for (size_t i = 0; i < H.size(); ++i) if (H[i].find("bool LowTempIdleCheckSafeDoor(void);") != std::string::npos) ++decl;
    CHECK(decl == 1, "[6] csystem.h declares bool LowTempIdleCheckSafeDoor(void); live exactly once");

    std::printf("St02_W127LowTempDoor: %d passed, %d failed\n", g_total - g_fail, g_fail);
    return g_fail ? 1 : 0;
}
