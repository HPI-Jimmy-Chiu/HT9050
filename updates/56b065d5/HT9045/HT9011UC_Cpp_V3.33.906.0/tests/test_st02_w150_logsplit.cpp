// =============================================================================
//  test_st02_w150_logsplit.cpp -- W-150 LOG-SPLIT slice 1 (skill hpi-mnetlog-split s7): the golden TfMain log objects reached
//  through LogObjects.h accessors instead of fMain->slXxx outside main.cpp.
//
//  AI(W906-W150) 20261007 (St02-E).  Suite name (add_test): St02_W150LogSplit.  argv[1] = the port tree (read-only source pins).
//    [1] W906_CreateLogObjects() -> each of the 6 accessors is the very object fMain holds (slTimeData, slProdRecordLog, slTestLog,
//        slJamAlarmLog, slTorqueLog, slTorqueLogNew);
//    [2] cMyDB.cpp MyDBIProductionData (golden 0618 cMyDB.cpp:559-560) writes its row through W906_ProdRecordLogObj into the
//        redirected ProductRecord folder;
//    [3] the TestLog object writes through W906_TestLogObj; the W58 decision on cprod.cpp:3067: a network [N28] path is blocked in
//        the SIM build only (W906_SimNetPathBlocked), a local path never;
//    [4] source pins: cObserver.cpp:3193 / cprod.cpp:3067-3072 / rs232.cpp:868 live through the accessors (golden cObserver.cpp:2228,
//        cprod.cpp:2905-2912, rs232.cpp:1783), and no live `fMain->sl<one of the 6>` is left outside main.cpp / LogObjects.cpp /
//        forms / tests.  RecordEndTestTime (cObserver) and SetCustomerLimitationForConfig (cprod) are pinned, not run: the first
//        sends to HTTP first, the second rewrites the config edit lists.  rs232's >2000-byte Panasonic frame is pinned too.
//  Writes only ctest's sandbox (the log-root redirects); refuses a root under D:\HT9045 (incl. D:\HT9045_Log) that is not \obj\v906\.
// =============================================================================
#include "LogObjects.h"
#include "forms/fMain.h"
#include "Public/MyStringList.h"
#include "cprod.h"
#include "cmydef.h"
#include "Config.h"
#include "common.h"
#include "LastSet.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "cMyDB.h"           // MyDBIProductionData (golden cMyDB.cpp:500)
extern bool W906_SimNetPathBlocked(const AnsiString& path);   // common.h:341 / common.cpp:2785 (W58 Q5)
extern AnsiString asSaveEventLogPath, asProductionLogPath, sProductionInfoFilePath, asProductRecordPath, asTorqLogPath;

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
bool LineHas(const std::vector<std::string>& live, const std::string& needle)
{
    for (const std::string& l : live) if (l.find(needle) != std::string::npos) return true;
    return false;
}
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W150LogSplit -- W-150 slice 1: LogObjects.h accessors for the golden TfMain log objects\n");
    const std::string src = argc > 1 ? argv[1] : "";
    {
        const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath, &asProductRecordPath, &asTorqLogPath };
        const char* const names[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath", "asProductRecordPath", "asTorqLogPath" };
        const char* const envs[] = { "W906_HT9045LOG_ROOT", "W906_SAVEEVENTLOG_ROOT", "W906_PRODLOADER_ROOT", "W906_O19_ROOT" };
        bool ok = true;
        for (int i = 0; i < 6; ++i)
            if (roots[i]->IsEmpty() || UnderMachineTree(roots[i]->c_str())) { std::printf("  ABORT: %s = %s is not a sandbox\n", names[i], roots[i]->c_str()); ok = false; }
        for (const char* e : envs)
            if (!std::getenv(e)) { std::printf("  ABORT: %s is not set (run under ctest)\n", e); ok = false; }
        if (!ok) { std::printf("  nothing was called\n"); return 2; }
    }

    // ---------------------------------------------------------------- [1]
    std::printf("[1] the accessors are fMain's log objects\n");
    Check(fMain != nullptr, "[1] fMain exists");
    W906_CreateLogObjects();
    struct Row { const char* name; TMyStringList* got; TMyStringList* want; };
    const Row rows[] = {
        {"slTimeData",      W906_TimeDataLogObj(),   fMain->slTimeData},
        {"slProdRecordLog", W906_ProdRecordLogObj(), fMain->slProdRecordLog},
        {"slTestLog",       W906_TestLogObj(),       fMain->slTestLog},
        {"slJamAlarmLog",   W906_JamAlarmLogObj(),   fMain->slJamAlarmLog},
        {"slTorqueLog",     W906_TorqueLogObj(),     fMain->slTorqueLog},
        {"slTorqueLogNew",  W906_TorqueLogNewObj(),  fMain->slTorqueLogNew},
    };
    int same = 0;
    std::string bad;
    for (const Row& r : rows) { if (r.got != nullptr && r.got == r.want) ++same; else bad += std::string(" ") + r.name; }
    Check(same == 6, "[1] all 6 accessors return the object W906_CreateLogObjects put on fMain (" + std::to_string(same) + "/6)" + (bad.empty() ? "" : " wrong:" + bad));

    // ---------------------------------------------------------------- [2]
    std::printf("[2] cMyDB MyDBIProductionData writes its row through W906_ProdRecordLogObj\n");
    {
        TMyStringList* pr = W906_ProdRecordLogObj();
        const std::string file = pr ? std::string(pr->GetFileName().c_str()) : std::string();
        Check(pr != nullptr && !UnderMachineTree(file) && Lower(file).find(Lower(asProductRecordPath.c_str())) == 0,
              "[2] the ProductRecord file is in the redirected folder (" + file + ")");
        if (pr && !UnderMachineTree(file)) {
            IniConfig.sLotID = "W150LOT";
            MyDBIProductionData("W150ACT");
            std::string body;
            const bool r = ReadAll(std::string(pr->GetFileName().c_str()), &body);
            Check(r && body.find("W150LOT") != std::string::npos && body.find("W150ACT") != std::string::npos,
                  "[2] the row (lot W150LOT, action W150ACT) is in " + std::string(pr->GetFileName().c_str()) + " (golden cMyDB.cpp:559-560)");
        }
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] the TestLog object; the W58 network-path decision on cprod.cpp:3067\n");
    {
        TMyStringList* tl = W906_TestLogObj();
        const std::string base = std::string(as9045LogPath.c_str()) + "\\W150TestLog\\";
        ::CreateDirectoryA(base.c_str(), 0);
        if (tl) { tl->Path = base.c_str(); tl->FileName = "W150"; tl->AddText("W150 test row"); tl->MySaveToFile(); }
        std::string body;
        const std::string file = tl ? std::string(tl->GetFileName().c_str()) : std::string();
        Check(tl && !UnderMachineTree(file) && ReadAll(file, &body) && body.find("W150 test row") != std::string::npos,
              "[3] a row added through W906_TestLogObj lands in " + file);
#ifdef SOFT_SIMULTE
        Check(W906_SimNetPathBlocked("\\\\w150-nohost\\oee") && !W906_SimNetPathBlocked(base.c_str()),
              "[3] SIM: a UNC [N28] path is blocked (left unapplied, the TestLog stays local), a local path is not (W58 Q5, common.cpp:2785)");
#else
        Check(!W906_SimNetPathBlocked("\\\\w150-nohost\\oee") && !W906_SimNetPathBlocked(base.c_str()),
              "[3] ship: no path is blocked -- golden applies [N28] as it is (cprod.cpp:2909-2910)");
#endif
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] source pins + census\n");
    {
        std::string obs, prod, rs;
        const bool r = ReadAll(src + "/cObserver.cpp", &obs) && ReadAll(src + "/cprod.cpp", &prod) && ReadAll(src + "/rs232.cpp", &rs);
        const std::vector<std::string> lo = LiveCode(Lines(obs)), lp = LiveCode(Lines(prod)), lr = LiveCode(Lines(rs));
        Check(r && LineHas(lo, "if(W906_TestLogObj()) W906_TestLogObj()->AddText(str);"),
              "[4] cObserver.cpp RecordEndTestTime: the JSCK OEE row goes through W906_TestLogObj, live (golden cObserver.cpp:2228)");
        Check(r && LineHas(lp, "if(W906_TestLogObj()!=NULL && !W906_SimNetPathBlocked(IniConfig.sN28_Path))") &&
              LineHas(lp, "W906_TestLogObj()->Path=IniConfig.sN28_Path;") && LineHas(lp, "W906_TestLogObj()->FileName=IniConfig.sN28_IP;"),
              "[4] cprod.cpp: the [N28] TestLog path / file name applied live through the accessor, guarded by W58's W906_SimNetPathBlocked (golden cprod.cpp:2905-2912)");
        Check(r && LineHas(lr, "if(W906_TorqueLogObj()) W906_TorqueLogObj()->AddTextWithDateTime(S1);"),
              "[4] rs232.cpp: the >2000-byte frame goes to slTorqueLog through the accessor, live (golden rs232.cpp:1783)");
        std::vector<std::string> files;
        std::string root = src;
        for (std::size_t i = 0; i < root.size(); ++i) if (root[i] == '/') root[i] = '\\';
        Walk(root, files);
        const char* objs[] = {"slTimeData", "slProdRecordLog", "slTestLog", "slJamAlarmLog", "slTorqueLog", "slTorqueLogNew"};
        int live = 0;
        std::string where;
        for (const std::string& f : files) {
            const std::string lf = Lower(f);
            if (lf.find("\\main.cpp") != std::string::npos || lf.find("\\logobjects.cpp") != std::string::npos) continue;
            std::string text;
            if (!ReadAll(f, &text)) continue;
            const std::vector<std::string> L = LiveCode(Lines(text));
            for (std::size_t i = 0; i < L.size(); ++i)
                for (const char* o : objs) {
                    const std::string n1 = std::string("fMain->") + o;
                    for (std::size_t p = L[i].find(n1); p != std::string::npos; p = L[i].find(n1, p + 1)) {
                        const char c = p + n1.size() < L[i].size() ? L[i][p + n1.size()] : ' ';
                        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') continue;   // slTorqueLog vs slTorqueLogNew
                        ++live; where += " " + f.substr(root.size() + 1) + ":" + std::to_string(i + 1);
                    }
                }
        }
        Check(files.size() > 300 && live == 0, "[4] census: no live fMain->sl<the 6> outside main.cpp / LogObjects.cpp / forms / tests (" +
                                               std::to_string(files.size()) + " files)" + where);
    }

    std::printf("St02_W150LogSplit: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
