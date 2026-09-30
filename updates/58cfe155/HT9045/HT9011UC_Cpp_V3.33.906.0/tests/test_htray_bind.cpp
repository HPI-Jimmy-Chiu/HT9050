// ===========================================================================
//  tests/test_htray_bind.cpp
//
//  AI(W906-HTRAY) 20260930: INBOX 121 -- the ambient SIM stall (docs/AMBIENT_STALL_HTRAY_20260930.md).
//  golden TTrayMotor::SetHTrayPanel (Motor/mymotor.cpp:1500-1504) is `fHTary=true; pHTray=ptr;`, and golden
//  SetSimuScreenPara binds 50 tray motors once, inside `if(flag)` with no SOFT_SIMULTE guard (golden
//  cinitial.cpp:6430-6489).  TTrayMotor::SetTray fills the IC grid only `if(fHTary)` (golden mymotor.cpp:1506-1514).
//  The port had a no-op SetHTrayPanel and all 50 calls inside `#if 0 // GATE n5-G3`, so every supplied tray was
//  "fHasTray=true, 0 IC": the in-arm waited forever and CatchTray carried each tray off as tray end.
//
//    [1] unit, a fresh TTrayMotor: unbound, SetTray(HAS_IC) raises fHasTray and leaves the 4x4 grid empty (golden's
//        own behaviour for a motor it never binds); after SetHTrayPanel(NULL), SetTray(HAS_IC) gives HasIC() and
//        HowManyIC()==16, and SetTray(NULL_IC) empties the grid again while fHasTray stays true.
//    [2] the real one-shot SetSimuScreenPara() (cinitial.cpp): before it MOT[MMTrayY] does not fill; after it each of
//        golden's 50 motors fills a 2x3 grid on SetTray(HAS_IC); every OTHER MOT[] index (0..MAX_TRAY_MOTOR-1 minus the
//        50, MMTrayZ included -- golden never binds it) still only raises fHasTray.
//    [3] source pin on cinitial.cpp: the live NULL list in SetSimuScreenPara is outside every `#if 0`, the widget copy
//        (fMain-> / fAutoAlignment-> / fObserveMagazine->) is inside one, and both are, name for name and in order,
//        the list of [2] (which was taken from golden cinitial.cpp:6430-6489).
//  Writes: InitNewTray's MNetLog lines only (the HANDLER LOG under ctest's redirect roots; the guard below refuses to
//  run anywhere else).
// ===========================================================================
#include <cstdio>
#include <cstring>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "Motor/mymotor.h"      // TTrayMotor, MOT[MAX_TRAY_MOTOR]
#include "cmydef.h"             // HAS_IC / NULL_IC and the MM* tray-motor indices
#include "w906_ctest_guard.h"

void SetSimuScreenPara();       // cinitial.cpp -- golden declares it in no header (cinitial.cpp:7226 declares it block-local)

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_htray_bind.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// golden cinitial.cpp:6430-6489, in golden's order (6430-6452, 6454-6457, 6459-6461, 6463-6464, 6468-6471, 6476-6489).
struct Bound { const char* name; const int* idx; };
#define B(x) { #x, &x }
static const Bound kGolden[] = {
    B(MMPlate1), B(MMPlate2), B(MMTrayY), B(MMTrayY_Car), B(MMOCR),
    B(MManualTray1), B(MManualTray2), B(MManualTray3), B(MManualTray4), B(MManualTray5), B(MManualTray6),
    B(MMAuto1), B(MMAuto2), B(MMAuto3), B(MMAuto4), B(MMAuto5), B(MMAuto6),
    B(MMAuto1_Car), B(MMAuto2_Car), B(MMAuto3_Car), B(MMAuto4_Car), B(MMAuto5_Car), B(MMAuto6_Car),
    B(MMEmpty), B(MMEmpty_Car), B(MMColor), B(MMColor_Car),
    B(MMEmpty1), B(MMEmpty1_Car), B(MMAutoCleanKit),
    B(MInRotateKit), B(MOutRotateKit),
    B(MMAOASampleTray), B(MMAOASamplePlate), B(MMInArmAOATray), B(MMOutArmAOATray),
    B(MMMagazineTary1), B(MMMagazineTary2), B(MMMagazineTary3), B(MMMagazineTary4), B(MMMagazineTary5),
    B(MMMagazineTary6), B(MMMagazineTary7), B(MMMagazineTary8), B(MMMagazineTary9), B(MMMagazineTary10),
    B(MMMagazineTary11), B(MMMagazineTary12), B(MMMagazineTary13), B(MMMagazineTary14),
};
#undef B
static const int kGoldenN = (int)(sizeof(kGolden) / sizeof(kGolden[0]));

// A fresh 2x3 grid, no tray, then SetTray(data): returns the IC count the grid ends with.
static int SupplyAndCount(TTrayMotor& m, int data, const char* func)
{
    m.Tray.SetXYItem(2, 3);
    m.Tray.ClearData();
    m.fHasTray = false;
    m.SetTray(data, func);
    return m.Tray.HowManyIC();
}

static std::string Trim(const std::string& s)
{
    size_t a = s.find_first_not_of(" \t\r");
    size_t b = s.find_last_not_of(" \t\r");
    return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}

int main()
{
    if (!W906TestRequireCtestRedirects("HtrayBind"))
        return 2;

    std::printf("[1] TTrayMotor::SetHTrayPanel sets fHTary (golden mymotor.cpp:1500-1504)\n");
    {
        TTrayMotor tm;
        tm.Tray.SetXYItem(4, 4);
        tm.SetTray(HAS_IC, "HtrayBind-1a");
        CHECK(tm.fHasTray == true);                     // SetTray always raises it (golden :1508)
        CHECK(tm.HasIC() == false);                     // unbound: the grid is not filled (golden :1509 `if(fHTary)`)
        CHECK(tm.Tray.HowManyIC() == 0);

        tm.SetHTrayPanel(NULL);                         // the port has no TTMyTray widget: NULL, as SetSimuScreenPara passes
        tm.SetTray(HAS_IC, "HtrayBind-1b");
        CHECK(tm.fHasTray == true);
        CHECK(tm.HasIC() == true);                      // HasIC: pHTray==NULL -> `else if(Tray.HasIC())` (golden :1073)
        CHECK(tm.Tray.HowManyIC() == 16);               // InitNewTray -> ClearData + SetData(HAS_IC) over 4x4

        tm.SetTray(NULL_IC, "HtrayBind-1c");
        CHECK(tm.fHasTray == true);
        CHECK(tm.HasIC() == false);
        CHECK(tm.Tray.HowManyIC() == 0);
    }

    std::printf("[2] SetSimuScreenPara binds exactly golden's %d motors (golden cinitial.cpp:6430-6489)\n", kGoldenN);
    CHECK(kGoldenN == 50);
    {
        CHECK(SupplyAndCount(MOT[MMTrayY], HAS_IC, "HtrayBind-2pre") == 0);   // before the one-shot: not bound
        CHECK(MOT[MMTrayY].fHasTray == true && MOT[MMTrayY].HasIC() == false);

        SetSimuScreenPara();

        std::set<int> bound;
        int filled = 0;
        for (int k = 0; k < kGoldenN; ++k)
        {
            const int i = *kGolden[k].idx;
            bound.insert(i);
            const int n = SupplyAndCount(MOT[i], HAS_IC, "HtrayBind-2");
            if (n == 6 && MOT[i].fHasTray && MOT[i].HasIC())
                ++filled;
            else
                std::printf("    NOT FILLED: MOT[%s] (index %d): HowManyIC()=%d fHasTray=%d\n",
                            kGolden[k].name, i, n, (int)MOT[i].fHasTray);
        }
        CHECK(filled == kGoldenN);
        CHECK((int)bound.size() == kGoldenN);           // 50 distinct indices: no two golden names alias one motor

        // MMTrayY after binding: SetTray(NULL_IC) empties the grid (golden InitNewTray(NULL_IC)).
        CHECK(SupplyAndCount(MOT[MMTrayY], HAS_IC, "HtrayBind-2y") == 6);
        MOT[MMTrayY].SetTray(NULL_IC, "HtrayBind-2y0");
        CHECK(MOT[MMTrayY].Tray.HowManyIC() == 0 && MOT[MMTrayY].fHasTray == true);

        // control: golden binds nothing else -- MMTrayZ (golden SetTray only raises fHasTray there) and every other index.
        CHECK(bound.count(MMTrayZ) == 0);
        CHECK(SupplyAndCount(MOT[MMTrayZ], HAS_IC, "HtrayBind-2z") == 0);
        CHECK(MOT[MMTrayZ].fHasTray == true && MOT[MMTrayZ].HasIC() == false);
        int others = 0, othersEmpty = 0;
        for (int i = 0; i < MAX_TRAY_MOTOR; ++i)
        {
            if (bound.count(i))
                continue;
            ++others;
            const int n = SupplyAndCount(MOT[i], HAS_IC, "HtrayBind-2c");
            if (n == 0 && MOT[i].fHasTray)
                ++othersEmpty;
            else
                std::printf("    UNEXPECTEDLY BOUND: MOT[%d]: HowManyIC()=%d\n", i, n);
        }
        CHECK(others == MAX_TRAY_MOTOR - kGoldenN);
        CHECK(othersEmpty == others);
    }

    std::printf("[3] source pin: cinitial.cpp SetSimuScreenPara, live NULL list == gated widget list == golden list\n");
    {
        const std::string path = std::string(W906_SRC_ROOT) + "/cinitial.cpp";
        std::ifstream f(path.c_str(), std::ios::binary);
        CHECK((bool)f);
        std::vector<std::string> lines;
        for (std::string l; std::getline(f, l); )
            lines.push_back(l);
        size_t beg = lines.size(), end = lines.size();
        for (size_t i = 0; i < lines.size(); ++i)
            if (lines[i].compare(0, 24, "void SetSimuScreenPara()") == 0) { beg = i; break; }
        for (size_t i = beg; i < lines.size(); ++i)
            if (!lines[i].empty() && lines[i][0] == '}') { end = i; break; }
        CHECK(beg < lines.size() && end < lines.size());

        std::vector<std::string> live, gated;
        std::vector<bool> ppStack;                      // true = an `#if 0`
        bool liveInsideIf0 = false, gatedOutsideIf0 = false;
        for (size_t i = beg; i < end && end < lines.size(); ++i)
        {
            const std::string t = Trim(lines[i]);
            if (t.compare(0, 3, "#if") == 0) { ppStack.push_back(t.compare(0, 5, "#if 0") == 0); continue; }
            if (t.compare(0, 6, "#endif") == 0) { if (!ppStack.empty()) ppStack.pop_back(); continue; }
            if (t.compare(0, 5, "#else") == 0 || t.compare(0, 5, "#elif") == 0) { if (!ppStack.empty()) ppStack.back() = false; continue; }
            if (t.compare(0, 2, "//") == 0)
                continue;
            bool inIf0 = false;
            for (size_t k = 0; k < ppStack.size(); ++k)
                inIf0 = inIf0 || ppStack[k];
            const std::string code = t.substr(0, t.find("//"));
            for (size_t at = code.find("MOT["); at != std::string::npos; at = code.find("MOT[", at + 4))
            {
                const size_t rb = code.find(']', at);
                const size_t call = code.find(".SetHTrayPanel(", at);
                if (rb == std::string::npos || call == std::string::npos || call != rb + 1)
                    continue;
                const std::string name = Trim(code.substr(at + 4, rb - at - 4));
                const size_t a0 = call + std::strlen(".SetHTrayPanel(");
                const std::string arg = Trim(code.substr(a0, code.find(')', a0) - a0));
                if (arg == "NULL") { live.push_back(name); liveInsideIf0 = liveInsideIf0 || inIf0; }
                else               { gated.push_back(name); gatedOutsideIf0 = gatedOutsideIf0 || !inIf0; }
            }
        }
        CHECK((int)live.size() == kGoldenN);
        CHECK((int)gated.size() == kGoldenN);
        CHECK(liveInsideIf0 == false);                  // the NULL list is compiled
        CHECK(gatedOutsideIf0 == false);                // the widget copy stays in GATE n5-G3
        int liveSame = 0, gatedSame = 0;
        for (int k = 0; k < kGoldenN; ++k)
        {
            if (k < (int)live.size() && live[k] == kGolden[k].name) ++liveSame;
            else std::printf("    live[%d] = %s, golden = %s\n", k, k < (int)live.size() ? live[k].c_str() : "(none)", kGolden[k].name);
            if (k < (int)gated.size() && gated[k] == kGolden[k].name) ++gatedSame;
            else std::printf("    gated[%d] = %s, golden = %s\n", k, k < (int)gated.size() ? gated[k].c_str() : "(none)", kGolden[k].name);
        }
        CHECK(liveSame == kGoldenN);
        CHECK(gatedSame == kGoldenN);
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAILED" : "OK", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
