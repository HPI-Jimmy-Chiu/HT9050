// test_boot_servo_on.cpp -- AI(W906-B20-SERVOON) 20261001: census 129 (e) E-BOOT-002 (RULINGS_20261001 #10) + INBOX 134
//
//   [1] ServoOnAllMOT (golden Motor/mymotor.cpp:4609-4621, body at the end of Motor/mymotor.cpp): an enabled non-1203 motor gets
//       SetServoOn(true) exactly once; a disabled one, the golden MTestY1..MTestY2 range and a PCIE-1203 axis (TMyEtherCatMotor,
//       RULINGS_20261001 #10) never; NULL rows are skipped; the first id after the range is not.
//   [2] source pins: tools/wb_serve.cpp calls GetHotPlateYHalfPos then ServoOnAllMOT right after fCounterClear->ReadCTInfo()
//       (golden TfMain::FormShow main.cpp:10163-10165), on that line's code part (before its first //); the empty stub is gone;
//       every fMotorTest->mmoN->Lines->Add in Motor/mymotor.cpp is behind if(fMotorTest!=0) (INBOX 134).
//   Memory and source reads only; writes nothing.
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "Motor/mymotor.h"
#include "Motor/myEthercatmotor.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_boot_servo_on.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// any non-1203 card class (SMC / MN200 / SYNTEK ...): records what ServoOnAllMOT asks for
struct RecMotor : public HTMotor {
    int on, off;
    RecMotor() : on(0), off(0) {}
    void SetServoOn(bool IsOn) override { if (IsOn) ++on; else ++off; }
};
// a PCIE-1203 axis: cinitial.cpp:3998 builds TMyEtherCatMotor for CardModel "PCI1203"
struct RecEcat : public TMyEtherCatMotor {
    int on;
    RecEcat() : TMyEtherCatMotor(-1), on(0) {}
    void SetServoOn(bool IsOn) override { if (IsOn) ++on; }
};

static std::vector<std::string> ReadLines(const char* rel)
{
    std::vector<std::string> out;
    std::ifstream f((std::string(W906_SRC_ROOT) + "/" + rel).c_str(), std::ios::binary);
    std::string s;
    while (std::getline(f, s)) {
        if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
        out.push_back(s);
    }
    return out;
}

// the part of a line the compiler sees: /* ... */ removed, then everything from the first // cut
static std::string CodePart(std::string s)
{
    for (;;) {
        const std::size_t a = s.find("/*");
        if (a == std::string::npos) break;
        const std::size_t b = s.find("*/", a + 2);
        if (b == std::string::npos) { s.erase(a); break; }
        s.erase(a, b + 2 - a);
    }
    const std::size_t c = s.find("//");
    if (c != std::string::npos) s.erase(c);
    return s;
}

static int Count(const std::string& s, const std::string& what)
{
    int n = 0;
    for (std::size_t p = s.find(what); p != std::string::npos; p = s.find(what, p + what.size())) ++n;
    return n;
}

int main()
{
    std::printf("Boot_ServoOn\n");

    // ==================== [1] ServoOnAllMOT ====================
    for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Motor = NULL;
    CHECK(MTestY1 > 3 && MTestY2 >= MTestY1 && MTestY2 + 1 < TOTAL_MOTOR);
    RecMotor a, b, ty1, tmid, ty2, after;
    RecEcat e;
    a.Enable = true; b.Enable = false; e.Enable = true;
    ty1.Enable = true; tmid.Enable = true; ty2.Enable = true; after.Enable = true;
    MOT[0].Motor = &a;
    MOT[1].Motor = &b;
    MOT[2].Motor = &e;
    MOT[MTestY1].Motor = &ty1;
    if (MTestY2 > MTestY1 + 1) MOT[MTestY1 + 1].Motor = &tmid;
    MOT[MTestY2].Motor = &ty2;
    MOT[MTestY2 + 1].Motor = &after;
    ServoOnAllMOT();
    CHECK(a.on == 1 && a.off == 0);                       // enabled, not 1203: golden servo on, once
    CHECK(b.on == 0 && b.off == 0);                       // Enable==false: golden skip
    CHECK(e.on == 0);                                     // PCIE-1203: RULINGS_20261001 #10 (EastSun's Motor Power On keeps it)
    CHECK(ty1.on == 0 && tmid.on == 0 && ty2.on == 0);   // golden skips MTestY1..MTestY2
    CHECK(after.on == 1);                                 // the range ends at MTestY2
    for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Motor = NULL;
    ServoOnAllMOT();                                      // every row NULL: nothing to do, no crash (port-only guard)
    CHECK(a.on == 1 && after.on == 1);

    // ==================== [2] source pins ====================
    {
        const std::vector<std::string> L = ReadLines("tools/wb_serve.cpp");
        CHECK(!L.empty());
        int hits = 0;
        for (std::size_t i = 0; i < L.size(); ++i) {
            const std::string code = CodePart(L[i]);
            const std::size_t r = code.find("fCounterClear->ReadCTInfo();");
            if (r == std::string::npos) continue;
            ++hits;
            const std::size_t g = code.find("GetHotPlateYHalfPos();", r);
            const std::size_t s = code.find("ServoOnAllMOT();", r);
            CHECK(g != std::string::npos && s != std::string::npos && r < g && g < s);   // golden :10163 -> :10164 -> :10165
        }
        CHECK(hits == 1);
    }
    {
        const std::vector<std::string> L = ReadLines("Motor/mymotor.cpp");
        CHECK(!L.empty());
        int defs = 0, stubs = 0, logs = 0, guarded = 0;
        for (std::size_t i = 0; i < L.size(); ++i) {
            const std::string code = CodePart(L[i]);
            if (code.find("void ServoOnAllMOT()") != std::string::npos) {
                ++defs;
                if (code.find("{}") != std::string::npos) ++stubs;
            }
            logs += Count(code, "fMotorTest->mmo");
            guarded += Count(code, "if(fMotorTest!=0) fMotorTest->mmo");
        }
        CHECK(defs == 1 && stubs == 0);                   // one real body, the `{}` stub retired
        CHECK(logs == 8 && guarded == 8);                 // OutArm / InArm ContinuousMove_9045 light-scale log, all checked
    }

    std::printf("test_boot_servo_on: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
