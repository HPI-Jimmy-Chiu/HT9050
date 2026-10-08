// =============================================================================
//  test_st02_w150_logsplit2.cpp -- W-150 LOG-SPLIT slice 2 (skill hpi-mnetlog-split s7 / s8 L5): golden TTLLog and the Index
//  position log (LogIndexMaxMinPos + TfMain::AddIndexPosLog) as their own modules (TTLLog.cpp / IndexPosLog.cpp), reaching
//  fMain->slTTLLog / slIndexYMaxMinShift through LogObjects.h accessors; slQtyLog (MainClarnData.cpp) through W906_QtyLogObj.
//
//  AI(W906-W150) 20261008 (St02-E).  Suite name (add_test): St02_W150LogSplit2.  argv[1] = the port tree (read-only source pins).
//    [1] before W906_CreateLogObjects: the 3 new accessors are null and TTLLog / LogIndexMaxMinPos write no log-object file;
//    [2] after it: each accessor is the object fMain holds;
//    [3] TTLLog (golden 0618 cpublic.cpp:489-512): TTL_MODE -> "<msg>, " + 12 switch states in TTL_Signal_LOG; another test type
//        -> nothing; "Close" -> written whatever the test type;
//    [4] LogIndexMaxMinPos (golden 0618 cpublic.cpp:1582-1599): both "Index Y1:" / "Index Y2:" lines with the seeded values in the
//        IndexMaxMin file, and the same two lines in the AddIndexPosLog buffer;
//    [5] W906_AddIndexPosLog (golden 0618 main.cpp:33634-33664): bSave writes the buffer to IndexPos\YYYY\MM_IndexPosLog\Z1UpZ2Down_<stamp>.logs
//        and clears it; without bSave the 1026th line first saves the 1025 buffered ones (Count>1024);
//    [6] source pins: the three retired gates (cDIOStatus.cpp :71, MainTimer3.cpp G15, cStateRecord.cpp G9) call live; cpublic.cpp
//        keeps its two reference copies gated (one live definition each); MainClarnData.cpp uses W906_QtyLogObj; IndexPosLog.cpp
//        carries TODO(W906-LOGVIEW) L5; the CMake line; no live fMain->slTTLLog / slIndexYMaxMinShift / slQtyLog outside
//        main.cpp / LogObjects.cpp / forms / tests.
//  Writes only a fresh w150b_<tick> folder in ctest's log-root sandbox (W906_HT9045LOG_ROOT; removed when green); refuses a root under D:\HT9045 (incl. D:\HT9045_Log).
// =============================================================================
#include "LogObjects.h"
#include "IndexPosLog.h"
#include "forms/fMain.h"
#include "Public/MyStringList.h"
#include "cprod.h"
#include "cmydef.h"
#include "cpublic.h"
#include "common.h"
#include "myswitch.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0;   // D:\HT9045 and D:\HT9045_Log
}
bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}
bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); v.push_back(l); }
    return v;
}
// live code of a line: no `#if 0 ... #endif` block (nesting tracked), no // comment (outside strings)
std::vector<std::string> LiveCode(const std::vector<std::string>& v)
{
    std::vector<std::string> out(v.size());
    std::vector<int> st;   // 1 = a dead #if 0 frame
    for (std::size_t i = 0; i < v.size(); ++i) {
        std::string t = v[i];
        std::size_t a = t.find_first_not_of(" \t");
        const std::string d = a == std::string::npos ? "" : t.substr(a);
        const bool dead = !st.empty() && st.back() == 1;
        if (d.compare(0, 3, "#if") == 0) { st.push_back(dead || d.compare(0, 5, "#if 0") == 0 ? 1 : 0); continue; }
        if (d.compare(0, 6, "#endif") == 0) { if (!st.empty()) st.pop_back(); continue; }
        if (d.compare(0, 5, "#else") == 0 || d.compare(0, 5, "#elif") == 0) {
            if (!st.empty()) { const bool parentDead = st.size() > 1 && st[st.size() - 2] == 1; st.back() = parentDead ? 1 : (st.back() == 1 ? 0 : 1); }
            continue;
        }
        if (dead) continue;
        bool inStr = false;
        for (std::size_t k = 0; k + 1 < t.size(); ++k) {
            if (t[k] == '"' && (k == 0 || t[k - 1] != '\\')) inStr = !inStr;
            if (!inStr && t[k] == '/' && t[k + 1] == '/') { t = t.substr(0, k); break; }
        }
        out[i] = t;
    }
    return out;
}
int LiveCount(const std::vector<std::string>& live, const std::string& needle)
{
    int n = 0;
    for (const std::string& l : live) if (l.find(needle) != std::string::npos) ++n;
    return n;
}
void Walk(const std::string& dir, std::vector<std::string>& files)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string n = fd.cFileName;
        if (n == "." || n == "..") continue;
        const std::string p = dir + "\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (n == "tests" || n == "docs" || n == "forms" || n == "third_party" || n == ".git" || n.compare(0, 5, "build") == 0) continue;
            Walk(p, files);
        } else {
            const std::string l = Lower(n);
            if (l.size() > 4 && (l.compare(l.size() - 4, 4, ".cpp") == 0 || l.compare(l.size() - 2, 2, ".h") == 0 || l.compare(l.size() - 4, 4, ".inc") == 0)) files.push_back(p);
        }
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}
void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p); else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}
std::string FileOf(TMyStringList* sl) { return sl ? std::string(sl->GetFileName().c_str()) : std::string(); }
std::string Flushed(TMyStringList* sl)   // MySaveToFile, then the file's text
{
    std::string body;
    if (sl == nullptr) return body;
    sl->MySaveToFile();
    ReadAll(FileOf(sl), &body);
    return body;
}
std::string SwitchStates()   // golden cpublic.cpp:497-508, written out independently of TTLLog.cpp
{
    const int sw[12] = {SwClear0, SwClear1, SwClear2, SwClear3, SwStart0, SwStart1, SwStart2, SwStart3, SwDut0, SwDut1, SwDut2, SwDut3};
    std::string s;
    for (int i = 0; i < 12; ++i) s += std::string(SW[sw[i]].Status() ? "1" : "0") + (i < 11 ? ", " : " ");
    return s;
}
void FixedTime(int sec)   // AddIndexPosLog stamps with the caller's last GetTimeInfo -- pin it so the file names are known
{
    SystemYear = 2026; SystemMonth = 10; SystemDate = 8; SystemHour = 1; SystemMin = 2; SystemSec = (Word)sec; SystemMSec = 4;
}
std::string IndexPosDir()
{
    char b[64];
    std::snprintf(b, sizeof b, "\\IndexPos\\%04d\\%02d_IndexPosLog\\", (int)SystemYear, (int)SystemMonth);
    return std::string(as9045LogPath.c_str()) + b;
}
std::string Z1Name(int sec)
{
    char b[64];
    std::snprintf(b, sizeof b, "Z1UpZ2Down_%04d%02d%02d%02d%02d%02d.logs", 2026, 10, 8, 1, 2, sec);
    return IndexPosDir() + b;
}
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W150LogSplit2 -- W-150 slice 2: TTLLog.cpp / IndexPosLog.cpp and the slTTLLog / slIndexYMaxMinShift / slQtyLog accessors\n");
    const std::string src = argc > 1 ? argv[1] : "";
    if (!std::getenv("W906_HT9045LOG_ROOT") || as9045LogPath.IsEmpty() || UnderMachineTree(as9045LogPath.c_str())) {
        std::printf("  ABORT: as9045LogPath = %s is not a sandbox (W906_HT9045LOG_ROOT %s) -- nothing was called\n",
                    as9045LogPath.c_str(), std::getenv("W906_HT9045LOG_ROOT") ? "set" : "not set (run under ctest)");
        return 2;
    }
    // a fresh folder per run under ctest's log root: a re-run (or the reverse round) must not see the last run's files
    const std::string root = std::string(as9045LogPath.c_str()) + "\\w150b_" + std::to_string((unsigned long)::GetTickCount());
    ::CreateDirectoryA(root.c_str(), 0);
    as9045LogPath = root.c_str();   // W906_CreateLogObjects / W906_AddIndexPosLog build their paths from it

    // ---------------------------------------------------------------- [1]
    std::printf("[1] before W906_CreateLogObjects\n");
    Check(fMain != nullptr, "[1] fMain exists");
    Check(W906_TTLLogObj() == nullptr && W906_IndexYMaxMinShiftLogObj() == nullptr && W906_QtyLogObj() == nullptr,
          "[1] the three accessors are null before the log objects exist");
    TestIF.iTestType = TTL_MODE;
    TTLLog("W150PRE");
    LogIndexMaxMinPos("W150PRE");
    Check(!Exists(root + "\\TTL_Signal_LOG"), "[1] TTLLog / LogIndexMaxMinPos with no log objects: no crash, no TTL_Signal_LOG folder");
    Check(W906_IndexPosLogLineCount() == 2, "[1] AddIndexPosLog does not need the log objects (golden memo): 2 lines buffered (" +
                                            std::to_string(W906_IndexPosLogLineCount()) + ")");
    FixedTime(1);
    W906_AddIndexPosLog("W150 flush", true);   // start [4] / [5] from an empty buffer

    // ---------------------------------------------------------------- [2]
    std::printf("[2] the accessors are fMain's log objects\n");
    W906_CreateLogObjects();
    Check(W906_TTLLogObj() != nullptr && W906_TTLLogObj() == fMain->slTTLLog, "[2] W906_TTLLogObj() == fMain->slTTLLog");
    Check(W906_IndexYMaxMinShiftLogObj() != nullptr && W906_IndexYMaxMinShiftLogObj() == fMain->slIndexYMaxMinShift,
          "[2] W906_IndexYMaxMinShiftLogObj() == fMain->slIndexYMaxMinShift");
    Check(W906_QtyLogObj() != nullptr && W906_QtyLogObj() == fMain->slQtyLog, "[2] W906_QtyLogObj() == fMain->slQtyLog");
    Check(!UnderMachineTree(FileOf(W906_TTLLogObj())) && !UnderMachineTree(FileOf(W906_IndexYMaxMinShiftLogObj())),
          "[2] TTL_Signal_LOG / IndexMaxMin resolve into the sandbox (" + FileOf(W906_TTLLogObj()) + ")");
    Check(!Exists(FileOf(W906_TTLLogObj())) && !Exists(FileOf(W906_IndexYMaxMinShiftLogObj())),
          "[2] ... and neither file exists yet: [1] wrote nothing through a null accessor");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] TTLLog (golden 0618 cpublic.cpp:489-512)\n");
    {
        TMyStringList* const sl = W906_TTLLogObj();
        const int saved = TestIF.iTestType;
        TestIF.iTestType = TTL_MODE;
        TTLLog("W150T1");
        std::string body = Flushed(sl);
        Check(!UnderMachineTree(FileOf(sl)) && body.find("W150T1, " + SwitchStates()) != std::string::npos,
              "[3] TTL_MODE: \"W150T1, \" + the 12 switch states (" + SwitchStates() + ") in " + FileOf(sl));
        TestIF.iTestType = TTL_MODE + 1;
        TTLLog("W150T2");
        TTLLog("Close");
        body = Flushed(sl);
        Check(body.find("W150T2") == std::string::npos, "[3] another test type: no W150T2 line");
        Check(body.find("Close, " + SwitchStates()) != std::string::npos, "[3] \"Close\" is written whatever the test type");
        TestIF.iTestType = saved;
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] LogIndexMaxMinPos (golden 0618 cpublic.cpp:1582-1599)\n");
    {
        TMyStringList* const sl = W906_IndexYMaxMinShiftLogObj();
        iMaxCommandY1 = 111; iMinCommandY1 = -11; iMaxTeachY1F = 12; iMinTeachY1F = -12; iMaxTeachY1M = 13; iMinTeachY1M = -13;
        iMaxCommandY2 = 222; iMinCommandY2 = -22; iMaxTeachY2M = 23; iMinTeachY2M = -23; iMaxTeachY2R = 24; iMinTeachY2R = -24;
        const int before = W906_IndexPosLogLineCount();
        LogIndexMaxMinPos("W150");
        std::string body;
        const bool r = ReadAll(FileOf(sl), &body);   // LogIndexMaxMinPos saves itself (golden :1596)
        const std::string y1 = "Index Y1: MaxCMDY1:111,MinCMDY1:-11,MaxY1F:12,MinY1F:-12,MaxY1M:13,MinY1M:-13";
        const std::string y2 = "Index Y2: MaxCMDY2:222,MinCMDY2:-22,MaxY2M:23,MinY2M:-23,MaxY2R:24,MinY2R:-24";
        Check(r && !UnderMachineTree(FileOf(sl)) && body.find(y1) != std::string::npos && body.find(y2) != std::string::npos,
              "[4] both lines in the IndexMaxMin file " + FileOf(sl));
        Check(W906_IndexPosLogLineCount() == before + 2, "[4] the same two lines went to the AddIndexPosLog buffer (" +
                                                        std::to_string(before) + " -> " + std::to_string(W906_IndexPosLogLineCount()) + ")");
    }

    // ---------------------------------------------------------------- [5]
    std::printf("[5] W906_AddIndexPosLog (golden 0618 main.cpp:33634-33664)\n");
    {
        FixedTime(3);
        W906_AddIndexPosLog("W150 save", true);
        std::string body;
        const bool r = ReadAll(Z1Name(3), &body);
        Check(r && body.find("2026-10-08, 01:02:03.004 : W150 save") != std::string::npos && body.find("Index Y1: MaxCMDY1:111") != std::string::npos,
              "[5] bSave: " + Z1Name(3) + " holds the buffered [4] lines and \"2026-10-08, 01:02:03.004 : W150 save\"");
        Check(W906_IndexPosLogLineCount() == 0, "[5] bSave clears the buffer");
        FixedTime(5);
        for (int i = 0; i < 1025; ++i) W906_AddIndexPosLog(AnsiString(("W150L" + std::to_string(i)).c_str()));
        Check(W906_IndexPosLogLineCount() == 1025 && !Exists(Z1Name(5)), "[5] 1025 lines buffered, nothing saved yet (" +
                                                                         std::to_string(W906_IndexPosLogLineCount()) + ")");
        W906_AddIndexPosLog("W150LAST");
        body.clear();
        const bool r2 = ReadAll(Z1Name(5), &body);
        const std::vector<std::string> L = Lines(body);
        Check(r2 && L.size() == 1025 && L.front().find(" : W150L0") != std::string::npos && L.back().find(" : W150L1024") != std::string::npos &&
              body.find("W150LAST") == std::string::npos,
              "[5] the 1026th line first saves the 1025 buffered ones (golden Count>1024) to " + Z1Name(5) + " (" + std::to_string(L.size()) + " lines)");
        Check(W906_IndexPosLogLineCount() == 1, "[5] ... and then buffers itself (1 line)");
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] source pins + census\n");
    {
        std::string dio, mt3, sr, cp, mcd, ttl, ipl, cm;
        const bool r = ReadAll(src + "/cDIOStatus.cpp", &dio) && ReadAll(src + "/MainTimer3.cpp", &mt3) && ReadAll(src + "/cStateRecord.cpp", &sr) &&
                       ReadAll(src + "/cpublic.cpp", &cp) && ReadAll(src + "/JsonBridge/actions/MainClarnData.cpp", &mcd) &&
                       ReadAll(src + "/TTLLog.cpp", &ttl) && ReadAll(src + "/IndexPosLog.cpp", &ipl) && ReadAll(src + "/CMakeLists.txt", &cm);
        Check(r, "[6] the eight source files read");
        const std::vector<std::string> ldio = LiveCode(Lines(dio)), lmt3 = LiveCode(Lines(mt3)), lsr = LiveCode(Lines(sr)), lcp = LiveCode(Lines(cp)),
                                       lmcd = LiveCode(Lines(mcd)), lttl = LiveCode(Lines(ttl)), lipl = LiveCode(Lines(ipl));
        Check(LiveCount(ldio, "TTLLog(\"InitDIOStstus\");") == 1, "[6] cDIOStatus.cpp: TTLLog(\"InitDIOStstus\") live (golden 0618 main.cpp:24373)");
        Check(LiveCount(lmt3, "TTLLog(\"Initial TTL 1\");") == 1, "[6] MainTimer3.cpp: G15 retired, TTLLog(\"Initial TTL 1\") live (golden 0618 main.cpp:25493)");
        Check(LiveCount(lsr, "LogIndexMaxMinPos(\"StateRecord\");") == 1, "[6] cStateRecord.cpp: G9 retired, LogIndexMaxMinPos(\"StateRecord\") live (golden 0618 main.cpp:26675)");
        Check(LiveCount(lcp, "void TTLLog(AnsiString Message)") == 0 && LiveCount(lcp, "void LogIndexMaxMinPos(AnsiString str)") == 0 &&
              LiveCount(lttl, "void TTLLog(AnsiString Message)") == 1 && LiveCount(lipl, "void LogIndexMaxMinPos(AnsiString str)") == 1,
              "[6] one live definition each: TTLLog.cpp / IndexPosLog.cpp; cpublic.cpp keeps its reference copies gated");
        Check(LiveCount(lmcd, "if (W906_QtyLogObj() != 0) return W906_QtyLogObj();") == 1 && LiveCount(lmcd, "fMain->slQtyLog") == 0,
              "[6] MainClarnData.cpp QtyLog() takes fMain->slQtyLog through W906_QtyLogObj");
        Check(ipl.find("TODO(W906-LOGVIEW)") != std::string::npos && ipl.find("MemoIndexPosLog") != std::string::npos && ipl.find("L5") != std::string::npos,
              "[6] IndexPosLog.cpp marks the dropped display: TODO(W906-LOGVIEW) MemoIndexPosLog (skill hpi-mnetlog-split s8 L5)");
        bool cmOk = false;
        for (const std::string& l : Lines(cm)) {
            const std::size_t m = l.find("MNetLog.cpp"), t = l.find("TTLLog.cpp"), i = l.find("IndexPosLog.cpp"), h = l.find('#');
            if (m != std::string::npos && t != std::string::npos && i != std::string::npos && h != std::string::npos && m < t && t < i && i < h) cmOk = true;
        }
        Check(cmOk, "[6] CMakeLists.txt: TTLLog.cpp and IndexPosLog.cpp on the MNetLog.cpp line, before its comment (ht9045_sm)");
        std::vector<std::string> files;
        std::string tree = src;
        for (std::size_t i = 0; i < tree.size(); ++i) if (tree[i] == '/') tree[i] = '\\';
        Walk(tree, files);
        const char* objs[] = {"slTTLLog", "slIndexYMaxMinShift", "slQtyLog"};
        int live = 0;
        std::string where;
        for (const std::string& f : files) {
            const std::string lf = Lower(f);
            if (lf.find("\\main.cpp") != std::string::npos || lf.find("\\logobjects.cpp") != std::string::npos) continue;
            std::string text;
            if (!ReadAll(f, &text)) continue;
            const std::vector<std::string> L = LiveCode(Lines(text));
            for (std::size_t i = 0; i < L.size(); ++i)
                for (const char* o : objs)
                    if (L[i].find(std::string("fMain->") + o) != std::string::npos) { ++live; where += " " + f.substr(tree.size() + 1) + ":" + std::to_string(i + 1); }
        }
        Check(files.size() > 300 && live == 0, "[6] census: no live fMain->slTTLLog / slIndexYMaxMinShift / slQtyLog outside main.cpp / LogObjects.cpp / forms / tests (" +
                                               std::to_string(files.size()) + " files)" + where);
    }

    W906_DestroyLogObjects();
    Check(W906_TTLLogObj() == nullptr && W906_IndexYMaxMinShiftLogObj() == nullptr && W906_QtyLogObj() == nullptr,
          "[6] after W906_DestroyLogObjects the accessors are null again");
    if (g_fail == 0 && root.find("\\w150b_") != std::string::npos && !UnderMachineTree(root)) RemoveTree(root);   // green: drop the sandbox
    std::printf("St02_W150LogSplit2: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
