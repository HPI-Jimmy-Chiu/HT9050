// AI(W906-SAFEDOOR-REC) 20261001: batch 19, RULINGS_20261001 #0 / #5 -- csystem.cpp GATE G11 / G25 / G28a / G28b lifted.
//   RecordSafeDoorStates (csystem.cpp:21388 = golden csystem.cpp:2833-3042), the call DoSystem() makes every pass again.
//   Part 1 (behaviour, only through the function itself):
//     SystemStart true -> returns at once (golden :2845-2856), nothing written;
//     SystemStart false, [O18] on, safe door 1 opens (SnSafeDoor1 reads off) -> Gerneral.ini [LastSafeDoorOpen]
//       <sensor name>=YYYY-MM-DD-HH (golden :2874-2875, GATE G25) in asGeneralPath;
//     the latch: a second pass with the door still open writes nothing new; the door closes -> latch cleared;
//     [O18] off -> the door opens again and nothing is written.
//   The test refuses to run unless asGeneralPath is the per-test scratch copy (tests/CMakeLists.txt ENV-ALL
//   W906_GENERAL_INI_PATH) -- never the machine's D:\HT9045\system\Gerneral.ini.  CloseIniFile() is never called.
//   Part 2 (source, argv[1] = source root, // comments stripped): G11 / G25 / G28a / G28b are #if 1, G26 / G27 still #if 0.
//   NOT COVERED: the hatchway timestamps (G28a / G28b need a Tri-Temp hatchway sensor table), the MyDBIProcess rows
//   (they go to the redirected log roots), the SECS events (EventReport is a counter in this port).
#include <cstdio>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include "cmydef.h"
#include "Config.h"
#include "csystem.h"
#include "common.h"
#include "mysensor.h"

static int g_fail = 0;
static void check(bool ok, const char* what)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static void SensorOff(int idx) { Sen[idx].Enable = true; Sen[idx].Type = TYPE_A; Sen[idx].ISABase = eISABase; }   // IsOff()==true
static void SensorOn(int idx)  { Sen[idx].Enable = true; Sen[idx].Type = TYPE_B; Sen[idx].ISABase = eISABase; }   // IsOn()==true

static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string Code(const std::string& text)
{
    std::string code;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        const std::size_t c = line.find("//");
        code += (c == std::string::npos ? line : line.substr(0, c)) + "\n";
    }
    return code;
}

static std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) s[i] = (char)std::tolower((unsigned char)s[i]);
    return s;
}

int main(int argc, char** argv)
{
    const std::string gen = Lower(asGeneralPath.c_str());
    if (gen.empty() || gen.find("d:" "\x5c" "ht9045" "\x5c" "system") == 0 || gen.find("d:/ht9045/system") == 0) {
        std::printf("FAIL asGeneralPath is the machine's file (%s) -- refusing to run (ENV-ALL W906_GENERAL_INI_PATH missing)\n", asGeneralPath.c_str());
        return 1;
    }
    std::printf("asGeneralPath = %s\n", asGeneralPath.c_str());

    // every listed safe door closed, the hatchways not installed, door 1 = the one under test
    int door1 = -1;
    for (int i = 0; i < MAX_SAFE_DOOR_CNT; i++) {
        SensorOn(iSafeDoor[i]);
        if (iSafeDoor[i] == SnSafeDoor1 && door1 < 0) door1 = i;
    }
    for (int i = 0; i < MAX_HATCH_DOOR_CNT; i++) Sen[iSafeDoorHatchway[i]].Enable = false;
    check(door1 >= 0, "SnSafeDoor1 is in the golden iSafeDoor[] list");
    if (door1 < 0) return 1;
    const AnsiString key = "W906TestSafeDoor1";
    Sen[SnSafeDoor1].Name = key;
    IniConfig.bA36OpenDoorSetErrBin = false;
    IniConfig.bO18SafeDoorOnOffDurationDetect = true;
    SystemYear = 2026; SystemMonth = 10; SystemDate = 1; SystemHour = 12;
    WriteIniData(asGeneralPath, "LastSafeDoorOpen", key, AnsiString("unset"));

    // running: golden returns before looking at any door
    SystemStart = true;
    SensorOff(SnSafeDoor1);
    RecordSafeDoorStates();
    check(ReadIniData(asGeneralPath, "LastSafeDoorOpen", key, AnsiString("")) == AnsiString("unset"),
          "SystemStart true -> returns at once, nothing written (golden :2845-2856)");

    // stopped, [O18] on, door 1 open -> the timestamp (GATE G25)
    SystemStart = false;
    RecordSafeDoorStates();
    check(ReadIniData(asGeneralPath, "LastSafeDoorOpen", key, AnsiString("")) == AnsiString("2026-10-01-12"),
          "door 1 opens while stopped, [O18] on -> [LastSafeDoorOpen] W906TestSafeDoor1=2026-10-01-12 (golden :2874-2875)");

    // still open next pass: the latch keeps it from writing again
    SystemHour = 13;
    RecordSafeDoorStates();
    check(ReadIniData(asGeneralPath, "LastSafeDoorOpen", key, AnsiString("")) == AnsiString("2026-10-01-12"),
          "door still open on the next pass -> no second write (bSafeDoorOpen latch, golden :2862 / :2866)");

    // closes -> latch cleared; opens again with [O18] off -> no write
    SensorOn(SnSafeDoor1);
    RecordSafeDoorStates();
    IniConfig.bO18SafeDoorOnOffDurationDetect = false;
    SensorOff(SnSafeDoor1);
    RecordSafeDoorStates();
    check(ReadIniData(asGeneralPath, "LastSafeDoorOpen", key, AnsiString("")) == AnsiString("2026-10-01-12"),
          "closed then opened again with [O18] off -> nothing written (the option gates the timestamp, golden :2868)");

    // and with [O18] back on, the next opening writes the new hour
    SensorOn(SnSafeDoor1);
    RecordSafeDoorStates();
    IniConfig.bO18SafeDoorOnOffDurationDetect = true;
    SensorOff(SnSafeDoor1);
    RecordSafeDoorStates();
    check(ReadIniData(asGeneralPath, "LastSafeDoorOpen", key, AnsiString("")) == AnsiString("2026-10-01-13"),
          "closed, [O18] on again, opened at hour 13 -> the timestamp follows");
    SensorOn(SnSafeDoor1);
    RecordSafeDoorStates();

    // Part 2: the source
    const std::string src = argc > 1 ? argv[1] : ".";
    const std::string cs = Code(Read(src + "/csystem.cpp"));
    check(cs.size() > 1000000, "csystem.cpp read (comments stripped)");
    struct G { const char* tag; bool live; } gates[] = {
        {"GATE G11 -- golden csystem.cpp:4394", true}, {"GATE G25 -- golden csystem.cpp:2874-2875", true},
        {"GATE G28a -- golden csystem.cpp:3028-3029", true}, {"GATE G28b -- golden csystem.cpp:3036-3037", true},
        {"GATE G26 -- golden csystem.cpp:2966-2971", false}, {"GATE G27 -- golden csystem.cpp:2973-3016", false},
    };
    const std::string raw = Read(src + "/csystem.cpp");
    for (unsigned k = 0; k < sizeof(gates) / sizeof(gates[0]); ++k) {
        const std::string want = std::string(gates[k].live ? "#if 1 // " : "#if 0 // ") + gates[k].tag;
        const std::string what = std::string("csystem.cpp: ") + gates[k].tag + (gates[k].live ? " is #if 1" : " stays #if 0");
        check(raw.find(want) != std::string::npos, what.c_str());
    }

    std::printf("%s SafeDoorRecord (%d failure(s))\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
