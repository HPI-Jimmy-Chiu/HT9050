// =============================================================================
//  test_st02_w150_logsplit4.cpp -- W-150 LOG-SPLIT slice 4 (W-178 part 1; skill hpi-mnetlog-split s7): TempCtrl/TriTemp.cpp's
//  W7TT_TMyStringList stand-in forwards to the real fMain->slTriTempDoorlog / slDewPointLog[3] (LogObjects.h), and forms/fLotInfo.cpp's
//  W906_slLotInfolog() prefers fMain->slLotInfolog (its lazy copy now under as9045LogPath).
//
//  AI(W906-W150) 20261008 (St02-E).  Suite name (add_test): St02_W150LogSplit4.  argv[1] = the port tree (read-only source pins).
//    [1] before W906_CreateLogObjects: the 3 accessors are null; DoorOpenAlarmForTriTemp writes no CheckInputDoor file (no crash).
//    [2] after it: W906_TriTempDoorLogObj / W906_DewPointLogObj(0..2) / W906_LotInfoLogObj are fMain's objects; an index outside 0..2 -> null.
//    [3] DoorOpenAlarmForTriTemp (golden 0618 TriTemp.cpp:1870 ...; the door-log write at :1950) -> "<TaskNow>, <TaskNext>, ..." in
//        TriTemp\CheckInputDoor (golden 0618 main.cpp:1621-1624).
//    [4] fCheckDewPointStatus (golden 0618 TriTemp.cpp:149 / :163) with a dew-point meter on a tri-temp machine -> one line in each of
//        InArm_ / Index_ / OutArm_DewPoint_Log (golden 0618 main.cpp:1626-1640): the stand-in's `which` is the golden index.
//    [5] LotInfo: a row through W906_LotInfoLogObj -> W906_LotInfoLogFileName() (fLotInfo.cpp, golden uLotInfo.cpp:1996-1997) names
//        that same object's file, inside the sandbox.
//    [6] source pins: TriTemp.cpp :330 / :342, fLotInfo.cpp :6639 / :6642.
//  Writes only a fresh w150d_<tick> folder in ctest's log-root sandbox (W906_HT9045LOG_ROOT; removed when green).
// =============================================================================
#include "LogObjects.h"
#include "forms/fMain.h"
#include "Public/MyStringList.h"
#include "cmydef.h"
#include "common.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void fCheckDewPointStatus();                                                   // TempCtrl/TriTemp.h:88
int  DoorOpenAlarmForTriTemp(bool bBigDoor, int iTempMode, int iTaskNow, int iTaskNext, int iCountTime);   // TempCtrl/TriTemp.cpp:2390
AnsiString W906_LotInfoLogFileName();                                           // forms/fLotInfo.h:2375

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
std::string FileOf(TMyStringList* sl) { return sl ? std::string(sl->GetFileName().c_str()) : std::string(); }
std::string Flushed(TMyStringList* sl)
{
    std::string body;
    if (sl == nullptr) return body;
    sl->MySaveToFile();
    ReadAll(FileOf(sl), &body);
    return body;
}
std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); v.push_back(l); }
    return v;
}
std::string LineOf(const std::vector<std::string>& v, std::size_t n) { return n >= 1 && n <= v.size() ? v[n - 1] : std::string(); }
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
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W150LogSplit4 -- W-150 slice 4: TriTemp door / dew-point logs and the LotInfo log through LogObjects.h\n");
    const std::string src = argc > 1 ? argv[1] : "";
    if (!std::getenv("W906_HT9045LOG_ROOT") || as9045LogPath.IsEmpty() || UnderMachineTree(as9045LogPath.c_str())) {
        std::printf("  ABORT: as9045LogPath = %s is not a sandbox -- nothing was called\n", as9045LogPath.c_str());
        return 2;
    }
    // a fresh folder per run under ctest's log root: a re-run (or the reverse round) must not see the last run's files
    const std::string root = std::string(as9045LogPath.c_str()) + "\\w150d_" + std::to_string((unsigned long)::GetTickCount());
    ::CreateDirectoryA(root.c_str(), 0);
    as9045LogPath = root.c_str();

    // ---------------------------------------------------------------- [1]
    std::printf("[1] before W906_CreateLogObjects\n");
    Check(W906_TriTempDoorLogObj() == nullptr && W906_DewPointLogObj(0) == nullptr && W906_LotInfoLogObj() == nullptr,
          "[1] the accessors are null before the log objects exist");
    DoorOpenAlarmForTriTemp(true, 0, 910, 1, 0);
    Check(!Exists(root + "\\TriTemp"), "[1] DoorOpenAlarmForTriTemp with no log objects: no crash, no TriTemp folder");

    // ---------------------------------------------------------------- [2]
    std::printf("[2] the accessors are fMain's objects\n");
    W906_CreateLogObjects();
    Check(W906_TriTempDoorLogObj() != nullptr && W906_TriTempDoorLogObj() == fMain->slTriTempDoorlog, "[2] W906_TriTempDoorLogObj() == fMain->slTriTempDoorlog");
    Check(W906_DewPointLogObj(0) == fMain->slDewPointLog[0] && W906_DewPointLogObj(1) == fMain->slDewPointLog[1] && W906_DewPointLogObj(2) == fMain->slDewPointLog[2] &&
          W906_DewPointLogObj(0) != nullptr && W906_DewPointLogObj(3) == nullptr && W906_DewPointLogObj(-1) == nullptr,
          "[2] W906_DewPointLogObj(0..2) == fMain->slDewPointLog[0..2]; outside 0..2 -> null");
    Check(W906_LotInfoLogObj() != nullptr && W906_LotInfoLogObj() == fMain->slLotInfolog, "[2] W906_LotInfoLogObj() == fMain->slLotInfolog");
    Check(!UnderMachineTree(FileOf(W906_TriTempDoorLogObj())) && !UnderMachineTree(FileOf(W906_LotInfoLogObj())),
          "[2] CheckInputDoor / LotInfo resolve into the sandbox (" + FileOf(W906_TriTempDoorLogObj()) + ")");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] DoorOpenAlarmForTriTemp -> CheckInputDoor (golden 0618 main.cpp:1621-1624)\n");
    DoorOpenAlarmForTriTemp(true, 0, 910, 1, 0);
    {
        const std::string door = Flushed(W906_TriTempDoorLogObj());
        Check(door.find("910, 1, ") != std::string::npos && Lower(FileOf(W906_TriTempDoorLogObj())).find("checkinputdoor") != std::string::npos,
              "[3] \"910, 1, ...\" in " + FileOf(W906_TriTempDoorLogObj()));
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] fCheckDewPointStatus -> InArm_ / Index_ / OutArm_DewPoint_Log (golden 0618 main.cpp:1626-1640)\n");
    {
        const int saveHw = DewPoint_Hardware_Install, saveTri = Tri_Temp_Machine;
        const double saveDeg = dAdamValue_Degree, saveMa = dAdamValue_mA;
        DewPoint_Hardware_Install = 1;
        Tri_Temp_Machine = 1;
        dAdamValue_Degree = 12.5;                             // the ADAM read is a TU-local no-op stand-in (TriTemp.cpp:518): it keeps these
        dAdamValue_mA = 7.25;
        fCheckDewPointStatus();                              // starts its 1 s TimerCheckInterval (golden :1-second log)
        ::Sleep(1150);
        fCheckDewPointStatus();                              // the timer is off -> reads, the value changed (0 -> 12.5) -> one line per meter
        const char* names[3] = {"inarm_dewpoint_log", "index_dewpoint_log", "outarm_dewpoint_log"};
        int good = 0;
        std::string where;
        for (int i = 0; i < 3; ++i) {
            const std::string body = Flushed(W906_DewPointLogObj(i));
            const std::string f = FileOf(W906_DewPointLogObj(i));
            if (body.find("12.500000, 7.250000") != std::string::npos && Lower(f).find(names[i]) != std::string::npos && !UnderMachineTree(f)) ++good;
            else where += " [" + std::to_string(i) + "] " + f;
        }
        Check(good == 3, "[4] each dew-point meter's line in its own log (" + std::to_string(good) + "/3)" + where);
        const std::string door = Flushed(W906_TriTempDoorLogObj());
        Check(door.find("12.500000") == std::string::npos, "[4] ... and none in the door log (the stand-in's which picks the object)");
        DewPoint_Hardware_Install = saveHw;
        Tri_Temp_Machine = saveTri;
        dAdamValue_Degree = saveDeg;
        dAdamValue_mA = saveMa;
    }

    // ---------------------------------------------------------------- [5]
    std::printf("[5] LotInfo through fMain->slLotInfolog (golden 0618 uLotInfo.cpp:1996-1997)\n");
    {
        TMyStringList* const li = W906_LotInfoLogObj();
        li->AddText("W150D,lot,row");
        li->MySaveToFile();
        const std::string viaLot = W906_LotInfoLogFileName().c_str();
        std::string body;
        Check(!viaLot.empty() && viaLot == std::string(li->sLastFileName.c_str()) && !UnderMachineTree(viaLot) && ReadAll(viaLot, &body) &&
              body.find("W150D,lot,row") != std::string::npos,
              "[5] W906_LotInfoLogFileName() names fMain->slLotInfolog's file, in the sandbox (" + viaLot + ")");
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] source pins\n");
    {
        std::string tri, lot;
        const bool r = ReadAll(src + "/TempCtrl/TriTemp.cpp", &tri) && ReadAll(src + "/forms/fLotInfo.cpp", &lot);
        const std::vector<std::string> lt = Lines(tri), ll = Lines(lot);
        Check(r && LineOf(lt, 330).find("which < 3 ? W906_DewPointLogObj(which) : W906_TriTempDoorLogObj()") != std::string::npos &&
              LineOf(lt, 342).find("slDewPointLog[i]->which=i;") != std::string::npos,
              "[6] TriTemp.cpp:330 forwards to the real objects, :342 sets which = the golden index");
        Check(r && LineOf(ll, 6639).find("if (TMyStringList* const q = W906_LotInfoLogObj()) return q;") != std::string::npos &&
              LineOf(ll, 6642).find("p=new TMyStringList(as9045LogPath+") != std::string::npos,
              "[6] fLotInfo.cpp:6639 prefers fMain->slLotInfolog, :6642's lazy copy is under as9045LogPath");
    }

    W906_DestroyLogObjects();
    Check(W906_TriTempDoorLogObj() == nullptr && W906_DewPointLogObj(0) == nullptr && W906_LotInfoLogObj() != nullptr,   // golden never deletes slLotInfolog (LogObjects.cpp destroy note)
          "[6] after W906_DestroyLogObjects: door / dew-point accessors null again; slLotInfolog kept, as golden (never deleted)");
    if (g_fail == 0 && root.find("\\w150d_") != std::string::npos && !UnderMachineTree(root)) RemoveTree(root);   // green: drop the sandbox
    std::printf("St02_W150LogSplit4: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
