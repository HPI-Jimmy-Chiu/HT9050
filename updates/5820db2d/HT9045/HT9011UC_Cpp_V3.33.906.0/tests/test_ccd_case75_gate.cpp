// ===========================================================================
//  tests/test_ccd_case75_gate.cpp   (ctest: CcdCase75Gate)
//
//  AI(W906-W120) 20261006 (Frank01; TO_FRANK 1006 18:0x W-120, NB2-GPT NBG-Q10): the CCD identification wait, case 75 of
//  DoTestYFront (aTester_Front.cpp) and DoTestYRear (aTester_Rear.cpp), must behave the same on both sides. Offline the
//  golden test `bCCDProgramExistence==false` is gated (K1F7 / K1G7). The front's offline default was corrected to `false`
//  on 20260811 (PT-W7c); the rear kept `true`, which re-armed the 20 s timer every tick and parked in case 75 for ever
//  (golden's own CCD-timeout exit unreachable, no message). W-120 sets the rear to `false` too.
//  ORACLES (golden's own case-75 logic once the program-existence test is false):
//    1. timers expired (never armed / 0 ms): one tick -> iCCDTimeOutCount + 1, Task 73   (front and rear alike)
//    2. the 200 ms settle timer (Delay2) still running: the tick returns false, Task stays 75, the 20 s timer is not
//       re-armed (the settle wait is golden's)                                         (front and rear alike)
//    3. Delay2 over, the 20 s timer still running: Task stays 75 (waiting for the CCD)   (front and rear alike)
//    4. the 11th timeout in a row (> 100 / CCDTimeOutSec = 10): "CCD Time out" shown once, count back to 0, Task 73
//    5. source pins: both offline gates read `false)`, no `true)` left in either K1F7 / K1G7 case-75 gate
//  Reverse check (by hand for the MR): the rear gate back to `true` -> rear 1 and 4 and the rear source pin go red (3 cannot
//  tell the two apart: both wait). Memory only; argv[1] = the port root (read only).
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cmydef.h"
#include "myTimer.h"
#include "atester_shims.h"          // iTestYFrontTask / iTestYRearTask / DoTestYFront / DoTestYRear
#include "atester.h"                // iCCDTimeOutCount
#include "canary_support.h"         // W906_ShowMyMessage_*

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

extern TQPF_Timer DoTestYFrontDelay, DoTestYFrontDelay2;   // aTester_Front.cpp:157
extern TQPF_Timer DoTestYRearDelay,  DoTestYRearDelay2;    // aTester_Rear.cpp:340

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, e); }
    else    std::printf("  PASS: %s\n", e);
}
#define CHECK(c, msg) check((c), msg, __LINE__)

struct Side {
    const char* name;
    int* task;
    bool (*run)();
    TQPF_Timer* delay;      // the 20 s CCD wait
    TQPF_Timer* delay2;     // the 200 ms settle
};

static void Expire(Side& s) { s.delay->SetMSAndOn(0); s.delay2->SetMSAndOn(0); }

static void RunSide(Side& s)
{
    char msg[256];
    std::printf(" -- %s\n", s.name);

    Expire(s); iCCDTimeOutCount = 0; W906_ShowMyMessage_Reset();
    *s.task = 75; s.run();
    std::snprintf(msg, sizeof msg, "%s 1: timers expired -> one tick: Task 75 -> 73, iCCDTimeOutCount 0 -> 1", s.name);
    CHECK(*s.task == 73 && iCCDTimeOutCount == 1 && W906_ShowMyMessage_Count == 0, msg);

    Expire(s); s.delay2->SetMSAndOn(60000); iCCDTimeOutCount = 0;
    *s.task = 75; const bool r = s.run();
    std::snprintf(msg, sizeof msg, "%s 2: settle timer running -> returns false, Task stays 75, no timeout counted", s.name);
    CHECK(r == false && *s.task == 75 && iCCDTimeOutCount == 0, msg);

    Expire(s); s.delay->SetMSAndOn(60000); iCCDTimeOutCount = 0;
    *s.task = 75; for (int i = 0; i < 5; ++i) s.run();
    std::snprintf(msg, sizeof msg, "%s 3: settle over, 20 s wait running -> 5 ticks: Task stays 75, no timeout counted", s.name);
    CHECK(*s.task == 75 && iCCDTimeOutCount == 0, msg);

    Expire(s); iCCDTimeOutCount = 10; W906_ShowMyMessage_Reset();
    *s.task = 75; s.run();
    std::snprintf(msg, sizeof msg, "%s 4: 11th timeout -> \"CCD Time out\" once, count back to 0, Task 73", s.name);
    CHECK(*s.task == 73 && iCCDTimeOutCount == 0 && W906_ShowMyMessage_Count == 1 &&
          W906_ShowMyMessage_LastS1 == AnsiString("CCD Time out"), msg);
}

static std::string ReadAll(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    std::string s = ss.str(), t;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] != 13) t += s[i];
    return t;
}
// the offline arm of the case-75 gate: from "#else\n            if(<delay2>.Off()==false ||\n" to the next "#endif"
static std::string Gate(const std::string& src, const std::string& delay2)
{
    const std::string head = "#else\n            if(" + delay2 + ".Off()==false ||\n";
    const size_t b = src.find(head);
    if (b == std::string::npos) return std::string();
    const size_t e = src.find("#endif", b);
    if (e == std::string::npos) return std::string();
    const std::string blk = src.substr(b + head.size(), e - b - head.size());
    std::string code;                                       // the code only: each line cut at "//" (the front's note says `...=true)`)
    std::istringstream in(blk);
    for (std::string line; std::getline(in, line); ) {
        const size_t c = line.find("//");
        code += (c == std::string::npos ? line : line.substr(0, c)) + "\n";
    }
    return code;
}
static bool HasNote(const std::string& src, const std::string& delay2, const char* note)   // the AI note on the gate block
{
    const size_t b = src.find("#else\n            if(" + delay2 + ".Off()==false ||\n");
    const size_t e = (b == std::string::npos) ? b : src.find("#endif", b);
    return b != std::string::npos && e != std::string::npos && src.substr(b, e - b).find(note) != std::string::npos;
}

int main(int argc, char** argv)
{
#ifdef SOFT_SIMULTE
    std::printf("CcdCase75Gate (SIM)\n");
#else
    std::printf("CcdCase75Gate (SHIP)\n");
#endif
    Side front = { "front DoTestYFront", &iTestYFrontTask, &DoTestYFront, &DoTestYFrontDelay, &DoTestYFrontDelay2 };
    Side rear  = { "rear  DoTestYRear",  &iTestYRearTask,  &DoTestYRear,  &DoTestYRearDelay,  &DoTestYRearDelay2 };
    std::printf(" behaviour of case 75 (golden's CCD wait once the program-existence test is false)\n");
    RunSide(front);
    RunSide(rear);
    iTestYFrontTask = 1; iTestYRearTask = 1; iCCDTimeOutCount = 0;

    std::printf(" source pins\n");
    if (argc > 1) {
        const std::string root = argv[1];
        const std::string sf = ReadAll(root + "/aTester_Front.cpp"), sr = ReadAll(root + "/aTester_Rear.cpp");
        const std::string gf = Gate(sf, "DoTestYFrontDelay2");
        const std::string gr = Gate(sr, "DoTestYRearDelay2");
        CHECK(!gf.empty() && gf.find("false)") != std::string::npos && gf.find("true)") == std::string::npos,
              "aTester_Front.cpp case 75 K1F7 offline gate: `false)` (PT-W7c)");
        CHECK(!gr.empty() && gr.find("false)") != std::string::npos && gr.find("true)") == std::string::npos,
              "aTester_Rear.cpp case 75 K1G7 offline gate: `false)` (W-120)");
        CHECK(HasNote(sr, "DoTestYRearDelay2", "AI(W906-W120)") && HasNote(sf, "DoTestYFrontDelay2", "PT-W7c"),
              "the notes sit on the gates: rear AI(W906-W120), front PT-W7c");
    } else {
        std::printf("  (no port root given -- source pins skipped)\n");
        CHECK(false, "argv[1] = port root");
    }

    std::printf("CcdCase75Gate: %d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
