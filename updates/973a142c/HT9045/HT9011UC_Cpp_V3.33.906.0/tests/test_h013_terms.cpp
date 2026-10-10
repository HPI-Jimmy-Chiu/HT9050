// =============================================================================
//  test_h013_terms.cpp -- H-013 items 1 + 2 (ruling 11 = B), both back to golden 906 and pinned that way here: golden 912's
//    IsSafePLCIOInstall() and fSecsAlarm terms are gone.
//
//  AI(W906-H013) 20261001 (St02-E).  Suite name (add_test): H013_Terms
//
//    1. (gone) AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 Command.cpp:7406 (RULINGS_20261002 #20 / #23-6):
//       IsSafePLCIOInstall() and SafePlcIOInstall.cpp are removed; MachineStatus reads Enable_PLCSafety_IO again (part 5);
//    2. (gone) AI(W906-ST02-C912) 20261003 (St02-E helper): back to golden 906_0625 Command.cpp:9605 / :9682 / :14066
//       (RULINGS_20261002 #20 / #23-6): fSecsAlarm, TSecsAlarmForm and SecsAlarmForm.{h,cpp} are removed (golden 906 has none);
//    3. TfMain::GetHandlerStatusByDll (Command.cpp:9889, golden 906_0625 Command.cpp:9605): no box shown -> not 4; fNote shown
//       -> 4 (SYSERROR); fNote shown with bAlarmReset -> the same as no box with bAlarmReset;
//    4. TfMain::RemoteControl(0) (Command.cpp:16336, golden 906_0625 :9682-9683): no box -> 0; fNote shown -> -1; hidden -> 0;
//       mode 0 = Reset, whose Pause is gated (#if 0), so every call returns without moving anything;
//    5. source pins (argv[1] = port root, read only; comments and dead #if 0 / #if 1-#else arms dropped): K1 is golden 906's
//       `(Enable_PLCSafety_IO && Sen[SnAllEMG].IsOff()))` inside TfMain::MachineStatus (906_0625 Command.cpp:7406);
//       IsSafePLCIOInstall is live nowhere in Command.cpp, cmydef.h (K6) or the port tree, and SafePlcIOInstall.cpp is gone
//       and in no add_library; K2-K4 are golden 906's expressions inside their functions (906_0625 Command.cpp:9605 /
//       :9682-9683 / :14066), fSecsAlarm is live nowhere in Command.cpp or the port tree, the SecsAlarmForm.h include (K5)
//       is gone, SecsAlarmForm.cpp is in no add_library and both SecsAlarmForm files are gone.
//  NOT COVERED here: HTGR,801 (Command.cpp:17935, golden 906_0625 :14066) runs only through the 7016 pump + framer; with no
//    box shown it is the existing TesterComm_TcpCmdServer row {"HTGR,801,", "HTSR,801,Normal,"}
//    (tests/test_tcp_cmd_server.cpp:474) -- this test pins its term in the source.  MachineStatus (K1) would need
//    Sen[SnAllEMG].IsOff()==true from a live IO read; the source pin covers it.
//  Memory only: no file is written; every global the test touches is restored.
//  Containment first (st02_test_containment.h).
// =============================================================================
#include "forms/fMain.h"
#include "forms/fNote.h"
#include "mymessbox_shim.h"        // MyMessageBox (TMyMessageBoxShim, the layout Command.cpp:297-305 re-declares)
#include "MachineType.h"
#include "cmydef.h"                // Enable_PLCSafety_IO / InitialOK / SystemStart / fAllMotorHome / iHome
#include "Motor/mymotor.h"         // MOT[] / Led[iServoOn]
#include "mysensor.h"              // Sen[]
#include "csystem.h"               // IsIndexMotorOutOfPower
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
static void Check(bool cond, const char* msg, int line)
{
    if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }
    else      { std::printf("  FAIL: %s  (line %d)\n", msg, line); ++g_fail; }
}
#define CHECK(cond, msg) Check((cond), (msg), __LINE__)

// ---- 3 ----------------------------------------------------------------------------------------------------------
static void Part3_GetHandlerStatusByDll()
{
    std::printf("\n[3] GetHandlerStatusByDll (Command.cpp:9889; golden 906_0625 Command.cpp:9605)\n");
    const bool sInit = InitialOK, sStart = SystemStart, sReset = bAlarmReset;
    const bool sMb = MyMessageBox->fShow, sNote = fNote->fShow, sPower = Sen[SnMotorPower].Enable;
    InitialOK = true;
    SystemStart = false;                 // else ret=2 before the term
    bAlarmReset = false;
    MyMessageBox->fShow = false;
    fNote->fShow = false;
    Sen[SnMotorPower].Enable = false;    // TMySensor::IsOff()==false when disabled (mysensor.cpp) -- no loaded IO table can change the path

    const int base = fMain->GetHandlerStatusByDll();
    char msg[256];
    std::snprintf(msg, sizeof msg, "no box shown -> %d, not 4", base);
    CHECK(base != 4, msg);

    fNote->fShow = true;                 // the 906 disjunct of the same term (golden 906_0625 :9605)
    CHECK(fMain->GetHandlerStatusByDll() == 4, "fNote shown -> 4 (SYSERROR, golden 906_0625 :9605-9608)");
    bAlarmReset = true;
    const int noteReset = fMain->GetHandlerStatusByDll();
    fNote->fShow = false;
    const int noneReset = fMain->GetHandlerStatusByDll();
    std::snprintf(msg, sizeof msg, "fNote shown + bAlarmReset -> %d == no box + bAlarmReset %d (the && bAlarmReset==false half)", noteReset, noneReset);
    CHECK(noteReset == noneReset && noteReset != 4, msg);

    Sen[SnMotorPower].Enable = sPower;
    fNote->fShow = sNote;
    MyMessageBox->fShow = sMb;
    bAlarmReset = sReset;
    SystemStart = sStart;
    InitialOK = sInit;
}

// ---- 4 ----------------------------------------------------------------------------------------------------------
static void Part4_RemoteControl()
{
    std::printf("\n[4] RemoteControl(0) (Command.cpp:16336; golden 906_0625 Command.cpp:9682-9686)\n");
    const int sensors[] = { SnFrontLeftEMG, SnFrontRightEMG, SnRearLeftEMG, SnRearRightEMG, SnServo, SnAllEMG, SnMotorPower };
    const int nSen = (int)(sizeof(sensors) / sizeof(sensors[0]));
    bool sEnable[sizeof(sensors) / sizeof(sensors[0])];
    for (int i = 0; i < nSen; ++i) sEnable[i] = Sen[sensors[i]].Enable;
    const bool sInit = InitialOK, sHome = fAllMotorHome, sPlc = Enable_PLCSafety_IO, sGemEmg = GEM_EMGPressed;  const ESafePLCIOType sPlcType = g_eSafePLCIOType;   // AI(W906-W217) 20261010 (Ifor01)
    const int sIHome = iHome;
    const bool sMb = MyMessageBox->fShow, sNote = fNote->fShow;
    const bool sLed1 = MOT[MTestZ1].Led[iServoOn], sLed2 = MOT[MTestZ2].Led[iServoOn];

    InitialOK = true;
    fAllMotorHome = true;
    iHome = 0;
    MyMessageBox->fShow = false;
    fNote->fShow = false;
    Enable_PLCSafety_IO = false;  g_eSafePLCIOType = eSafePLCIOType_Uninstall;   // AI(W906-W217) 20261010 (Ifor01) (St02-E 12:4x point 3): the type follows the bool
    // IsIndexMotorOutOfPower (csystem.cpp): SIM returns false; SHIP needs no EMG / power-off sensor and both Index Z servo LEDs on.
    for (int i = 0; i < nSen; ++i) Sen[sensors[i]].Enable = false;
    MOT[MTestZ1].Led[iServoOn] = true;
    MOT[MTestZ2].Led[iServoOn] = true;

    CHECK(fMain->SettingsIsWindowOpened() == false, "precondition: SettingsIsWindowOpened()==false");
    CHECK(IsIndexMotorOutOfPower() == false, "precondition: IsIndexMotorOutOfPower()==false");
    CHECK(fMain->RemoteControl(0) == 0, "no box shown -> RemoteControl(0)==0");
    fNote->fShow = true;                 // the 906 term's fNote disjunct (golden 906_0625 :9683)
    CHECK(fMain->RemoteControl(0) == -1, "fNote shown -> -1 (Operation not Allowed, golden 906_0625 :9682-9686)");
    fNote->fShow = false;
    CHECK(fMain->RemoteControl(0) == 0, "fNote hidden again -> 0");

    MOT[MTestZ2].Led[iServoOn] = sLed2;
    MOT[MTestZ1].Led[iServoOn] = sLed1;
    for (int i = 0; i < nSen; ++i) Sen[sensors[i]].Enable = sEnable[i];
    fNote->fShow = sNote;
    MyMessageBox->fShow = sMb;
    iHome = sIHome;
    GEM_EMGPressed = sGemEmg;            // SHIP: IsEMGPressed (inside IsIndexMotorOutOfPower) writes it
    Enable_PLCSafety_IO = sPlc;  g_eSafePLCIOType = sPlcType;   // AI(W906-W217) 20261010 (Ifor01): restored with the bool
    fAllMotorHome = sHome;
    InitialOK = sInit;
}

// ---- 5: source pins -----------------------------------------------------------------------------------------------
static bool ReadFile(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}

// Comments out (string / char literals kept), '\r' dropped, newlines kept.
static std::string StripComments(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    size_t i = 0;
    const size_t n = s.size();
    while (i < n)
    {
        const char c = s[i];
        if (c == '\r') { ++i; continue; }
        if (c == '/' && i + 1 < n && s[i + 1] == '/')
        {
            while (i < n && s[i] != '\n') ++i;
        }
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
            o += c;
            ++i;
            while (i < n && s[i] != q && s[i] != '\n')
            {
                if (s[i] == '\\' && i + 1 < n) { o += s[i]; ++i; }
                o += s[i];
                ++i;
            }
            if (i < n && s[i] == q) { o += s[i]; ++i; }
        }
        else
        {
            o += c;
            ++i;
        }
    }
    return o;
}

static std::string Trim(const std::string& s)
{
    const size_t a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return std::string();
    const size_t b = s.find_last_not_of(" \t");
    return s.substr(a, b - a + 1);
}

// Live lines only: `#if 0` arms and the #else / #elif arms of `#if 1` are blanked (line numbers kept); any other condition
// (#ifdef SOFT_SIMULTE ...) keeps every arm (unknown = live).  Input = StripComments output.
static std::vector<std::string> LiveLines(const std::string& code)
{
    struct Lvl { int cur; bool taken; };   // cur: 1 live, 0 dead, -1 unknown; taken: an earlier arm is known live
    std::vector<Lvl> st;
    std::vector<std::string> out;
    std::istringstream in(code);
    std::string line;
    while (std::getline(in, line))
    {
        const std::string t = Trim(line);
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
            }
            else if (d.compare(0, 4, "elif") == 0 && !st.empty())
            {
                Lvl& l = st.back();
                const std::string e = Trim(d.substr(4));
                if (l.taken) l.cur = 0;
                else if (e == "0" || e == "(0)") l.cur = (l.cur == -1) ? -1 : 0;
                else if ((e == "1" || e == "(1)") && l.cur == 0) { l.cur = 1; l.taken = true; }
                else l.cur = -1;
            }
            else if (d.compare(0, 4, "else") == 0 && !st.empty())
            {
                Lvl& l = st.back();
                if (l.taken) l.cur = 0;
                else if (l.cur == 0) l.cur = 1;
                else l.cur = -1;
            }
            else if (d.compare(0, 5, "endif") == 0 && !st.empty())
            {
                st.pop_back();
                out.push_back(std::string());
                continue;
            }
        }
        bool dead = false;
        for (size_t k = 0; k < st.size(); ++k) if (st[k].cur == 0) dead = true;
        const bool directive = !t.empty() && t[0] == '#' && (Trim(t.substr(1)).compare(0, 2, "if") == 0 ||
                               Trim(t.substr(1)).compare(0, 4, "else") == 0 || Trim(t.substr(1)).compare(0, 4, "elif") == 0);
        out.push_back(dead || directive ? std::string() : line);
    }
    return out;
}

static std::string NoSpace(const std::string& s)
{
    std::string o;
    o.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r') o += s[i];
    return o;
}

static int Count(const std::string& hay, const std::string& needle)
{
    int n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + 1)) ++n;
    return n;
}

static std::string Join(const std::vector<std::string>& v, size_t a, size_t b)
{
    std::string o;
    for (size_t i = a; i < b && i < v.size(); ++i) { o += v[i]; o += '\n'; }
    return o;
}

// The live body of `<ret> TfMain::<name>(` : from its header line to the next column-0 TfMain:: definition header.
static std::string Method(const std::vector<std::string>& live, const std::string& header)
{
    size_t a = live.size();
    for (size_t i = 0; i < live.size(); ++i)
        if (live[i].compare(0, header.size(), header) == 0) { a = i; break; }
    if (a == live.size()) return std::string();
    size_t b = live.size();
    for (size_t i = a + 1; i < live.size(); ++i)
    {
        const std::string& l = live[i];
        if (!l.empty() && l[0] != ' ' && l[0] != '{' && l[0] != '}' && l[0] != '#' &&
            l.find("TfMain::") != std::string::npos && l.find('(') != std::string::npos)
        { b = i; break; }
    }
    return NoSpace(Join(live, a, b));
}

static std::string LiveFile(const std::string& path, bool* ok)
{
    std::string raw;
    *ok = ReadFile(path, &raw);
    return NoSpace(Join(LiveLines(StripComments(raw)), 0, (size_t)-1));
}

// The port tree's live IsSafePLCIOInstall / fSecsAlarm / TSecsAlarmForm tokens -- golden 906 has none (AI(W906-ST02-C912) 20261003
// (St02-E helper): back to golden 906_0625 Command.cpp:7406 and :9605 / :9682 / :14066 (RULINGS_20261002 #20 / #23-6)).
// tests/, docs/, third_party/, scratchpad/, build* skipped.
struct Defs { int safe = 0, alarm = 0, cls = 0; std::string safeAt, alarmAt, clsAt; int files = 0; };
static void ScanTree(const std::string& root, const std::string& rel, Defs* d)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((root + "\\" + rel + (rel.empty() ? "" : "\\") + "*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do
    {
        const std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        const std::string r = rel.empty() ? name : rel + "\\" + name;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (rel.empty() && (name == "tests" || name == "docs" || name == "third_party" || name == "scratchpad" ||
                                name.compare(0, 5, "build") == 0 || name[0] == '.'))
                continue;
            ScanTree(root, r, d);
            continue;
        }
        const size_t dot = name.rfind('.');
        const std::string ext = dot == std::string::npos ? std::string() : name.substr(dot);
        if (ext != ".cpp" && ext != ".h" && ext != ".hpp" && ext != ".inc" && ext != ".c") continue;
        ++d->files;
        std::string raw;
        if (!ReadFile(root + "\\" + r, &raw)) continue;
        if (raw.find("IsSafePLCIOInstall") == std::string::npos && raw.find("SecsAlarm") == std::string::npos) continue;
        const std::string live = NoSpace(Join(LiveLines(StripComments(raw)), 0, (size_t)-1));
        const int s = Count(live, "IsSafePLCIOInstall");
        const int a = Count(live, "fSecsAlarm");       // every live token (golden 906: none)
        const int c = Count(live, "TSecsAlarmForm");
        if (s) { d->safe += s; d->safeAt += r + " "; }
        if (a) { d->alarm += a; d->alarmAt += r + " "; }
        if (c) { d->cls += c; d->clsAt += r + " "; }
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

// ht9045_globals' add_library(...) source tokens and every add_library's tokens (CMake '#' comments dropped).
static void CmakeLists(const std::string& raw, std::vector<std::string>* globals, int* safeAll, int* alarmAll)
{
    std::istringstream in(raw);
    std::string line, lib;
    bool inLib = false;
    *safeAll = *alarmAll = 0;
    while (std::getline(in, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        const size_t hash = line.find('#');
        std::string code = hash == std::string::npos ? line : line.substr(0, hash);
        const std::string t = Trim(code);
        if (!inLib && t.compare(0, 12, "add_library(") == 0)
        {
            inLib = true;
            lib = t.substr(12);
            const size_t sp = lib.find_first_of(" )");
            lib = sp == std::string::npos ? lib : lib.substr(0, sp);
            code = t.substr(12);
        }
        if (!inLib) continue;
        std::istringstream w(code);
        std::string tok;
        bool close = false;
        while (w >> tok)
        {
            if (!tok.empty() && tok[tok.size() - 1] == ')') { close = true; tok.erase(tok.size() - 1); }
            if (tok == "SafePlcIOInstall.cpp") ++*safeAll;
            if (tok == "SecsAlarmForm.cpp") ++*alarmAll;
            if (lib == "ht9045_globals" && !tok.empty()) globals->push_back(tok);
            if (close) break;
        }
        if (close) inLib = false;
    }
}

static void Part5_SourcePins(const std::string& root)
{
    std::printf("\n[5] source pins under %s (read only)\n", root.c_str());
    bool ok = false;
    std::string raw;
    ok = ReadFile(root + "\\Command.cpp", &raw);
    CHECK(ok && !raw.empty(), "Command.cpp read");
    const std::vector<std::string> live = LiveLines(StripComments(raw));
    const std::string all = NoSpace(Join(live, 0, live.size()));

    const std::string ms = Method(live, "void TfMain::MachineStatus(");
    CHECK(!ms.empty() && ms.find(NoSpace("(Enable_PLCSafety_IO && Sen[SnAllEMG].IsOff()))")) != std::string::npos,
          "K1 live in TfMain::MachineStatus: (Enable_PLCSafety_IO && Sen[SnAllEMG].IsOff())) (golden 906_0625 Command.cpp:7406)");
    CHECK(!ms.empty() && all.find("IsSafePLCIOInstall") == std::string::npos,
          "K1: golden 912's IsSafePLCIOInstall is not live anywhere in Command.cpp (RULINGS_20261002 #20 / #23-6)");

    const std::string gh = Method(live, "int TfMain::GetHandlerStatusByDll(");
    CHECK(!gh.empty() && gh.find(NoSpace("if(((W906_FormShowing(\"MyMessageBox\", MyMessageBox->fShow)==true && iUnLoaderCount==0) || "
                                         "W906_FormShowing(\"fNote\", fNote->fShow)==true) && bAlarmReset==false)")) != std::string::npos,
          "K2 live in TfMain::GetHandlerStatusByDll: if(((MyMessageBox ... && iUnLoaderCount==0) || fNote ...) && bAlarmReset==false) (golden 906_0625 :9605)");

    const std::string rc = Method(live, "int TfMain::RemoteControl(");
    CHECK(!rc.empty() && rc.find(NoSpace("if(SettingsIsWindowOpened() || IsIndexMotorOutOfPower() || "
                                         "W906_FormShowing(\"MyMessageBox\", MyMessageBox->fShow)==true || "
                                         "W906_FormShowing(\"fNote\", fNote->fShow)==true || fAllMotorHome==false || iHome==1)")) != std::string::npos,
          "K3 live in TfMain::RemoteControl: ... MyMessageBox ... || fNote ... || fAllMotorHome==false || iHome==1) (golden 906_0625 :9682-9683)");

    const std::string tc = Method(live, "void TfMain::TCPCommandServerClientRead(");
    CHECK(!tc.empty() && tc.find(NoSpace("else if(W906_FormShowing(\"MyMessageBox\", MyMessageBox->fShow))"
                                         " { sData1=\"Down\"; }")) != std::string::npos,
          "K4 live in TfMain::TCPCommandServerClientRead (HTGR,801): else if(... MyMessageBox ...) -> Down (golden 906_0625 :14066-14069)");

    char msg[256];
    const int reads = Count(all, "fSecsAlarm");
    std::snprintf(msg, sizeof msg, "Command.cpp: %d live fSecsAlarm tokens (golden 906_0625 Command.cpp: 0)", reads);
    CHECK(reads == 0, msg);
    CHECK(Count(all, "SecsAlarmForm") == 0, "K5 gone: no live SecsAlarmForm.h include / TSecsAlarmForm in Command.cpp");

    const std::string cmydef = LiveFile(root + "\\cmydef.h", &ok);
    CHECK(ok && !cmydef.empty() && Count(cmydef, "IsSafePLCIOInstall") == 0, "K6 gone: no live IsSafePLCIOInstall in cmydef.h (golden 906 has none)");

    std::string gone;
    CHECK(!ReadFile(root + "\\SafePlcIOInstall.cpp", &gone), "SafePlcIOInstall.cpp is gone (golden 906 has no IsSafePLCIOInstall)");
    std::string goneAlarm;
    CHECK(!ReadFile(root + "\\SecsAlarmForm.cpp", &goneAlarm) && !ReadFile(root + "\\SecsAlarmForm.h", &goneAlarm),
          "SecsAlarmForm.cpp / SecsAlarmForm.h are gone (golden 906 has no fSecsAlarm / TSecsAlarmForm)");

    std::string cm;
    ok = ReadFile(root + "\\CMakeLists.txt", &cm);
    std::vector<std::string> globals;
    int safeAll = 0, alarmAll = 0;
    CmakeLists(cm, &globals, &safeAll, &alarmAll);
    bool gSafe = false, gAlarm = false;
    for (size_t i = 0; i < globals.size(); ++i) { if (globals[i] == "SafePlcIOInstall.cpp") gSafe = true; if (globals[i] == "SecsAlarmForm.cpp") gAlarm = true; }
    std::snprintf(msg, sizeof msg, "K7: SafePlcIOInstall.cpp and SecsAlarmForm.cpp in no add_library (ht9045_globals: %d source tokens; counts %d / %d)",
                  (int)globals.size(), safeAll, alarmAll);
    CHECK(ok && !gSafe && !gAlarm && safeAll == 0 && alarmAll == 0, msg);

    Defs d;
    ScanTree(root, "", &d);
    std::snprintf(msg, sizeof msg, "port tree (%d source files): IsSafePLCIOInstall live %d time(s) [%s] -- W-217 (Ifor 1010 B, RULINGS_20261008 #3): golden 913 form is back, defined once in cmydef.cpp", d.files, d.safe, d.safeAt.c_str());
    { std::string cd; CHECK(d.files > 0 && d.safe > 0 && ReadFile(root + "\\cmydef.cpp", &cd) && Count(cd, "bool IsSafePLCIOInstall()") == 1, msg); }   // AI(W906-W217) 20261010 (Ifor01): was d.safe == 0 (St02-E nodded, W-220)
    std::snprintf(msg, sizeof msg, "port tree: fSecsAlarm live %d time(s) [%s] (golden 906: none)", d.alarm, d.alarmAt.c_str());
    CHECK(d.files > 0 && d.alarm == 0, msg);
    std::snprintf(msg, sizeof msg, "port tree: TSecsAlarmForm live %d time(s) [%s] (golden 906: none)", d.cls, d.clsAt.c_str());
    CHECK(d.files > 0 && d.cls == 0, msg);
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("H013_Terms\n");
    if (!W906TestInsideCtestRoots("H013_Terms"))
        return 2;
    const std::string root = argc > 1 ? argv[1] : ".";   // CMAKE_SOURCE_DIR (add_test passes it)

    std::printf("\n[2] facade objects (the fSecsAlarm-is-NULL check went with fSecsAlarm: golden 906 has none)\n");
    CHECK(fMain != 0 && fNote != 0 && MyMessageBox != 0, "fMain / fNote / MyMessageBox exist");
    if (fMain == 0 || fNote == 0 || MyMessageBox == 0)
    {
        std::printf("FAILED H013_Terms (no facade objects)\n");
        return 1;
    }

    Part3_GetHandlerStatusByDll();
    Part4_RemoteControl();
    Part5_SourcePins(root);

    std::printf("\n%s H013_Terms: %d passed, %d failed\n", g_fail ? "FAILED" : "ALL PASS", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
