// AI(W906-DOORLOCK / W906-BIGFAN) 20261001: MainTimerSegments.cpp. Part 1: ht9045::W906_SafeDoorLockTick = golden
//   TfMain::Timer1Timer main.cpp:3051-3076 -- the safe-door lock follows SystemStart (+ the magazine door locks). Walks golden's branch table: SAFE_DOOR_LOCK off -> untouched;
//   CE PLC non-safe mode -> locked; the IO page open -> untouched (the operator drives it there); Contact manual height ->
//   open; otherwise = SystemStart. "Page open" comes from the web page table through W906_FormFShowHook (here a test hook),
//   and fContact's own fShow member still counts (W906_FormShowing = member || hook). Plus the two kept early-return guards
//   (employee-ID wait, SECS/GEM alarm) and the PumpTick call site (WebBridgeTags.cpp). Memory only; no file is written.
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

namespace ht9045 { void W906_SafeDoorLockTick(); void W906_Timer2FanTick(); }
#include "LastSet.h"

static int g_fail = 0;
static void check(bool ok, const char* what)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static const char* g_openPage = "";
static bool TestHook(const char* form) { return std::strcmp(form, g_openPage) == 0; }

static void SensorOff(int idx) { Sen[idx].Enable = true; Sen[idx].Type = TYPE_A; Sen[idx].ISABase = eISABase; }   // IsOff()==true
static void SensorOn(int idx)  { Sen[idx].Enable = true; Sen[idx].Type = TYPE_B; Sen[idx].ISABase = eISABase; }   // IsOn()==true

static bool Lock() { return SW[SwSafeDoorLock].OutValue; }

static void Run(bool sentinel)
{
    SW[SwSafeDoorLock].OutValue = sentinel;
    ht9045::W906_SafeDoorLockTick();
}

int main(int argc, char** argv)
{
    W906_FormFShowHook = &TestHook;
    SAFE_DOOR_LOCK = true;
    Enable_PLCSafety_IO = false;
    IniConfig.bN07_EnableEmployeeIdCheak = false;
    bEnableEmployeeIDCheck = false;
    bSECSGEMAlarm = false;
    bSECSGEM_NoteAlarm = false;
    fContact->fShow = false;
    iContactMode = CONTACT_NORMAL;
    g_openPage = "";

    SystemStart = true;  Run(false);
    check(Lock() == true, "SAFE_DOOR_LOCK, running -> SW[SwSafeDoorLock] On (golden :3062 OnOff(SystemStart))");
    SystemStart = false; Run(true);
    check(Lock() == false, "not running -> Off");

    SystemStart = true; g_openPage = "fiosetview"; Run(false);
    check(Lock() == false, "IO page open (page table \"io\") -> untouched even while running (golden :3056 fiosetview->fShow)");
    SystemStart = false; Run(true);
    check(Lock() == true, "IO page open, stopped -> untouched as well");
    g_openPage = "";

    SystemStart = true; iContactMode = CONTACT_MANUAL_GET_HEIGHT; g_openPage = "fContact"; Run(true);
    check(Lock() == false, "Contact page open in manual-height mode -> Off while running (golden :3058-3059)");
    g_openPage = ""; fContact->fShow = true; Run(true);
    check(Lock() == false, "the same through fContact's own fShow member (W906_FormShowing = member || page table)");
    fContact->fShow = false; Run(false);
    check(Lock() == true, "Contact page closed -> back to SystemStart");
    iContactMode = CONTACT_NORMAL; g_openPage = "fContact"; Run(false);
    check(Lock() == true, "Contact page open but not manual height -> SystemStart");
    g_openPage = "";

    Enable_PLCSafety_IO = true; SensorOff(SnSafeMode); SystemStart = false; Run(false);
    check(Lock() == true, "CE PLC (Enable_PLCSafety_IO) not in safe mode -> On even when stopped (golden :3053-3055)");
    g_openPage = "fiosetview"; Run(false);
    check(Lock() == true, "... and before the IO-page test (golden order)");
    g_openPage = "";
    SensorOn(SnSafeMode); Run(true);
    check(Lock() == false, "CE PLC in safe mode -> back to SystemStart (stopped -> Off)");
    Enable_PLCSafety_IO = false;

    SystemStart = true;
    SAFE_DOOR_LOCK = false; Run(false);
    check(Lock() == false, "SAFE_DOOR_LOCK off -> untouched");
    SAFE_DOOR_LOCK = true;

    IniConfig.bN07_EnableEmployeeIdCheak = true; bEnableEmployeeIDCheck = true; Run(false);
    check(Lock() == false, "employee-ID wait ([N07] + bEnableEmployeeIDCheck) -> untouched (golden Timer1Timer returned at :2928)");
    bEnableEmployeeIDCheck = false; Run(false);
    check(Lock() == true, "[N07] on but no check pending -> runs");
    IniConfig.bN07_EnableEmployeeIdCheak = false;

    bSECSGEMAlarm = true; Run(false);
    check(Lock() == false, "SECS/GEM alarm not yet noted -> untouched (golden :2958)");
    bSECSGEM_NoteAlarm = true; Run(false);
    check(Lock() == true, "SECS/GEM alarm already noted -> runs");
    bSECSGEMAlarm = false; bSECSGEM_NoteAlarm = false;

    // magazine door locks (golden :3065-3076)
    const int oldMag = AUTO3_IS_MAGAZINE, oldMagSt = iMagazineStatus;
    AUTO3_IS_MAGAZINE = 1; iMagazineStatus = 0; SystemStart = true;
    SW[SwMagazineSafeDoorLock].OutValue = false; SW[SwMagazineSafeDoor2LockOn].OutValue = false; SW[SwMagazineSafeDoor2LockOff].OutValue = true;
    ht9045::W906_SafeDoorLockTick();
    check(SW[SwMagazineSafeDoorLock].OutValue == true && SW[SwMagazineSafeDoor2LockOn].OutValue == true &&
          SW[SwMagazineSafeDoor2LockOff].OutValue == false, "magazine, running, status 0 -> door lock On, door-2 lock On / unlock Off (golden :3069-3074)");
    iMagazineStatus = 1;
    ht9045::W906_SafeDoorLockTick();
    check(SW[SwMagazineSafeDoorLock].OutValue == false && SW[SwMagazineSafeDoor2LockOn].OutValue == true,
          "magazine returning a tray (status != 0) -> its door opens (golden :3071-3072), door 2 still follows SystemStart");
    SystemStart = false; iMagazineStatus = 0;
    ht9045::W906_SafeDoorLockTick();
    check(SW[SwMagazineSafeDoorLock].OutValue == false && SW[SwMagazineSafeDoor2LockOn].OutValue == false &&
          SW[SwMagazineSafeDoor2LockOff].OutValue == true, "magazine, stopped -> unlocked");
    SystemStart = true; g_openPage = "fiosetview";
    ht9045::W906_SafeDoorLockTick();
    check(SW[SwMagazineSafeDoorLock].OutValue == false && SW[SwMagazineSafeDoor2LockOff].OutValue == true,
          "magazine with the IO page open -> untouched (golden :3067)");
    g_openPage = ""; AUTO3_IS_MAGAZINE = oldMag; iMagazineStatus = oldMagSt;
    SW[SwMagazineSafeDoorLock].OutValue = false; SW[SwMagazineSafeDoor2LockOn].OutValue = false; SW[SwMagazineSafeDoor2LockOff].OutValue = false;

    // Part 2: the big fan (golden Timer2Timer :20964-20968)
    const int oldCC = CUSTOMER_CODE;
    CUSTOMER_CODE = CC_KYEC_LEE; LastSet.bBigFan = true; IniConfig.bC01_FanDirection = true;
    SW[SwBigFan].OutValue = false; SW[SwFanDirection].OutValue = false;
    ht9045::W906_Timer2FanTick();
    check(SW[SwBigFan].OutValue == true && SW[SwFanDirection].OutValue == false,
          "big fan follows LastSet.bBigFan; another customer -> the fan direction is not driven (golden :20964-20968)");
    LastSet.bBigFan = false;
    ht9045::W906_Timer2FanTick();
    check(SW[SwBigFan].OutValue == false, "LastSet.bBigFan false -> the big fan Off");
    CUSTOMER_CODE = CC_ASE_CL;
    ht9045::W906_Timer2FanTick();
    check(SW[SwFanDirection].OutValue == true, "ASE_CL -> the fan direction follows [C01] FanDirection");
    CUSTOMER_CODE = CC_HONPREC_QC; IniConfig.bC01_FanDirection = false;
    ht9045::W906_Timer2FanTick();
    check(SW[SwFanDirection].OutValue == false, "HONPREC QC (code 0) -> follows it too (false)");
    CUSTOMER_CODE = oldCC; SW[SwBigFan].OutValue = false; SW[SwFanDirection].OutValue = false;

    // PumpTick calls them (WebBridgeTags.cpp), exactly once
    const std::string src = argc > 1 ? argv[1] : ".";
    std::ifstream f(src + "/WebBridgeTags.cpp", std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    const std::string s = ss.str();
    const std::size_t a = s.find("W906_SafeDoorLockTick(); }");
    const std::size_t b = a == std::string::npos ? a : s.find("W906_SafeDoorLockTick(); }", a + 1);
    const std::size_t p = s.find("void PumpTick(");
    check(!s.empty() && a != std::string::npos && b == std::string::npos && p != std::string::npos && p < a,
          "WebBridgeTags.cpp: PumpTick calls W906_SafeDoorLockTick, one call site");
    const std::size_t c = s.find("W906_Timer2FanTick(); }");
    const std::size_t d = c == std::string::npos ? c : s.find("W906_Timer2FanTick(); }", c + 1);
    check(c != std::string::npos && d == std::string::npos && p < c, "WebBridgeTags.cpp: PumpTick calls W906_Timer2FanTick, one call site");

    W906_FormFShowHook = 0;
    SW[SwSafeDoorLock].OutValue = false;
    SystemStart = false;
    std::printf("%s MainTimerSegments (%d failure(s))\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
