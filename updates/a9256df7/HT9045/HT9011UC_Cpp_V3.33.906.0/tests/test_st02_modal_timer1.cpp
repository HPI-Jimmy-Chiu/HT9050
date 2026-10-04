// AI(W906-S27) 20261004 (St02-E): ctest St02_ModalTimer1 -- S-27 / INBOX 93.  golden 0618 note.cpp:3355 / mymessbox.cpp:542: a
//   blocking box's own Timer1 (10 ms) calls fMain->Timer1Timer, so golden Timer1's segments keep running while the box is up.
//   W906_ModalTimer1Segments (MainTimersSt02.cpp), called from W906_ModalWaitTick (tools/wb_serve.cpp:7630), runs the ported
//   Timer1 segments that only PumpTick ran: GetTimeInfo (golden :3189) and the safe-door lock (golden :3051-3077,
//   ht9045::W906_SafeDoorLockTick).  Memory only (the SW / Sen tables, the clock globals).
//   1. the door lock follows SystemStart through it: an alarm stop (SystemStart false) -> unlocked while the box is up; running -> locked
//   2. golden Timer1Timer's guards: InitialOK false (:2719) -> lock untouched; the N07 employee-ID wait (:2928) -> untouched
//   3. the clock: SystemYear sentinel 9999 -> refreshed (golden :3189), also with InitialOK false (as PumpTick, WebBridgeTags.cpp:600)
//   4. argv[1] = source root: tools/wb_serve.cpp calls it exactly once, as code (before the // of its line), inside
//      W906_ModalWaitTick; MainTimersSt02.cpp defines it once.
//   CONTROL (reverse, scratchpad s27/REVERSE_S27.md): 4 is red against the wb_serve.cpp before S-27 (no call); 1 and 3 are red
//   against a W906_ModalTimer1Segments whose body is emptied.
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include "cmydef.h"
#include "MachineType.h"
#include "Config.h"
#include "mysensor.h"
#include "myswitch.h"
#include "csystem.h"
#include "atester_shims.h"
#include "cContact.h"
#include "LastSet.h"

void W906_ModalTimer1Segments();   // MainTimersSt02.cpp

static int g_fail = 0;
static void check(bool ok, const char* what)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static bool NoPage(const char*) { return false; }   // no web page open (W906_FormShowing -> false)

static bool Lock() { return SW[SwSafeDoorLock].OutValue; }

static void Run(bool sentinel)
{
    SW[SwSafeDoorLock].OutValue = sentinel;
    W906_ModalTimer1Segments();
}

static std::string ReadAll(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::size_t Count(const std::string& s, const std::string& what)
{
    std::size_t n = 0;
    for (std::size_t p = s.find(what); p != std::string::npos; p = s.find(what, p + what.size()))
        ++n;
    return n;
}

int main(int argc, char** argv)
{
    W906_FormFShowHook = &NoPage;
    SAFE_DOOR_LOCK = true;
    Enable_PLCSafety_IO = false;
    IniConfig.bN07_EnableEmployeeIdCheak = false;
    bEnableEmployeeIDCheck = false;
    bSECSGEMAlarm = false;
    bSECSGEM_NoteAlarm = false;
    fContact->fShow = false;
    iContactMode = CONTACT_NORMAL;
    const bool oldInit = InitialOK;
    InitialOK = true;

    std::printf("1. the door lock follows SystemStart while a box waits (golden :3051-3064 through fMain->Timer1Timer)\n");
    SystemStart = false; Run(true);
    check(Lock() == false, "alarm stop (SystemStart false) -> SW[SwSafeDoorLock] Off while the box is up");
    SystemStart = true;  Run(false);
    check(Lock() == true, "running (SystemStart true) -> On");

    std::printf("2. golden Timer1Timer's guards\n");
    InitialOK = false; SystemStart = false; Run(true);
    check(Lock() == true, "InitialOK false (golden :2719) -> lock untouched");
    InitialOK = true;
    IniConfig.bN07_EnableEmployeeIdCheak = true; bEnableEmployeeIDCheck = true; SystemStart = false; Run(true);
    check(Lock() == true, "N07 employee-ID wait (golden :2928, inside W906_SafeDoorLockTick) -> lock untouched");
    IniConfig.bN07_EnableEmployeeIdCheak = false; bEnableEmployeeIDCheck = false;
    SystemStart = false; Run(true);
    check(Lock() == false, "wait over -> follows SystemStart again");

    std::printf("3. the clock keeps running while a box waits (golden :3189 ProcessTimeUpdate -> GetTimeInfo)\n");
    InitialOK = false;
    SystemYear = 9999; SystemMonth = 9999;
    W906_ModalTimer1Segments();
    check(SystemYear >= 2020 && SystemYear < 2200 && SystemMonth >= 1 && SystemMonth <= 12,
          "SystemYear / SystemMonth refreshed from the 9999 sentinel, also with InitialOK false (as PumpTick)");
    InitialOK = true;

    std::printf("4. the hook-up (argv[1] = source root)\n");
    if (argc > 1) {
        const std::string root = argv[1];
        const std::string wb = ReadAll(root + "/tools/wb_serve.cpp");
        const std::string call = "W906_ModalTimer1Segments(); }";
        const std::size_t def = wb.find("void W906_ModalWaitTick(int kind, int kcode)");
        const std::size_t end = def == std::string::npos ? def : wb.find("\n}", def);
        const std::size_t at = wb.find(call);
        check(!wb.empty() && Count(wb, call) == 1 && Count(wb, "{ extern void W906_ModalTimer1Segments(); ") == 1,
              "tools/wb_serve.cpp: one call site, with its extern declaration");
        check(def != std::string::npos && end != std::string::npos && at != std::string::npos && def < at && at < end,
              "tools/wb_serve.cpp: the call is inside W906_ModalWaitTick");
        bool code = false;
        if (at != std::string::npos) {
            const std::size_t bol = wb.rfind('\n', at) + 1;
            const std::size_t eol = wb.find('\n', at);
            const std::string line = wb.substr(bol, eol - bol);
            const std::size_t cmt = line.find("//");
            code = cmt != std::string::npos && line.find(call) < cmt
                   && line.find("W906_MainRecordTimer1Tick(); }") < line.find(call);
        }
        check(code, "tools/wb_serve.cpp: the call is code (before the // of its line), right after W906_MainRecordTimer1Tick");
        const std::string mt = ReadAll(root + "/MainTimersSt02.cpp");
        check(Count(mt, "void W906_ModalTimer1Segments()") == 1 && Count(mt, "ht9045::W906_SafeDoorLockTick();") == 1
              && mt.find("if (InitialOK == false || g_s27Running)") != std::string::npos,
              "MainTimersSt02.cpp: defined once, with golden's InitialOK / re-entry guard, one door-lock call");
    } else {
        std::printf("  (skipped: no source root given)\n");
    }

    W906_FormFShowHook = 0;
    SW[SwSafeDoorLock].OutValue = false;
    SystemStart = false;
    InitialOK = oldInit;
    std::printf("%s St02_ModalTimer1 (%d failure(s))\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
