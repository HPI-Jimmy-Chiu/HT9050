// ===========================================================================
//  tests/test_mach0180_review.cpp   (ctest: MACH0180_Review)
//
//  AI(W906-HOME-1310FG) 20261005: laptop review of the MACH-0180 batch (machine cpp 0180-0201 on main, ledger
//  docs/handoff/MACH0180_LEDGER_20261004.md), finding F3. Machine cpp 0196 (AI(W906-HOME-1310ORDER)) stopped HOME on HT9050
//  when a Tray Arm floodgate did not pop up at ProcessMotorHome step 1310, timed with ResetOKDeleyTime -- the 30 s timer armed
//  when step 1250 / 1270 ends. 1310ORDER lets "every axis in place" win over that timer once it has run out, and only then are
//  the floodgates asked to Pop() for the first time, so with the timer spent HOME stopped on the first look. The wait now has
//  its own timer (uhome.cpp tail: W906_HomeFloodgateWaitOver / W906_HomeFloodgateWaitReset).
//    F3-a  the helper: the first pass after a reset starts a fresh grace (false); it runs out after the given time (true) and
//          stays out while the same wait goes on; the next reset (step 1300 -> 1310) gives the next wait its full grace again,
//          however long ago the last one ran out -- the case 0196 got wrong
//    F3-b  source pins on uhome.cpp: the stop tests W906_IsHT9050() BEFORE the helper (no other machine ever arms it), it no
//          longer reads ResetOKDeleyTime, and case 1300 resets the wait on the line that falls into case 1310
//  AI(W906-POOL7-W79B) 20261006: NB2-1 R244, POOL-7 = W-79 (b) (Frank 1006 10:02, RULINGS_20261006): at step 1310 HT9050's
//  Out Shuttle 2 no longer waits for MLoaderZ (flag19) before its move to Left -- they may move at the same time and do not
//  collide; the machine has not waited since 10/05 18:29. MLoaderZ still has to reach Prod.TrayZ_Home before HOME is done.
//    W79b  source pins on uhome.cpp case 1310: the Out Shuttle 2 condition (code, comments stripped) has no flag19 but keeps
//          the other flags; flag19 is still set by MLoaderZ's move to Prod.TrayZ_Home and is still in both all-in-place
//          checks (1310ORDER's timeout guard and golden's HOME-complete test)
//  Memory only: no card, no thread; the one file read is the source tree's uhome.cpp.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include <windows.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

void W906_HomeFloodgateWaitReset();          // uhome.cpp tail, AI(W906-HOME-1310FG)
bool W906_HomeFloodgateWaitOver(int ms);

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_mach0180_review.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static std::string ReadSource(const char* rel)
{
    std::ifstream f((std::string(W906_SRC_ROOT) + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

// AI(W906-POOL7-W79B) 20261006: the code of [from, to) with every // comment cut off (the comments there name flag19 on purpose)
static std::string CodeOnly(const std::string& u, size_t from, size_t to)
{
    std::string out;
    bool comment = false;
    for (size_t i = from; i < to && i < u.size(); ++i) {
        if (u[i] == '\n') { comment = false; out += '\n'; continue; }
        if (!comment && u[i] == '/' && i + 1 < u.size() && u[i + 1] == '/') comment = true;
        if (!comment) out += u[i];
    }
    return out;
}

int main()
{
    std::printf("MACH-0180 review: F3 (HT9050 step-1310 floodgate wait has its own timer)\n");

    // ---- F3-a the helper ------------------------------------------------------------------------------------------------
    W906_HomeFloodgateWaitReset();
    CHECK(W906_HomeFloodgateWaitOver(100) == false);          // first pass of a wait: a fresh grace
    CHECK(W906_HomeFloodgateWaitOver(100) == false);          // still inside it
    ::Sleep(250);
    CHECK(W906_HomeFloodgateWaitOver(100) == true);           // not up within the grace -> over (the caller stops HOME)
    CHECK(W906_HomeFloodgateWaitOver(100) == true);           // and it stays over while the same wait goes on (no silent re-arm)
    W906_HomeFloodgateWaitReset();                             // the next step 1300 -> 1310
    CHECK(W906_HomeFloodgateWaitOver(100) == false);          // a full grace again, although the last one ran out long ago
    W906_HomeFloodgateWaitReset();
    CHECK(W906_HomeFloodgateWaitOver(30000) == false);        // the production value: never over at its first pass
    CHECK(W906_HomeFloodgateWaitOver(30000) == false);

    // ---- F3-b source pins on uhome.cpp ----------------------------------------------------------------------------------
    const std::string u = ReadSource("uhome.cpp");
    CHECK(!u.empty());
    const size_t stop = u.find("if(W906_IsHT9050() && W906_HomeFloodgateWaitOver(30000))");
    CHECK(stop != std::string::npos);                          // HT9050 tested first: every other machine never arms the wait
    CHECK(u.find("if(W906_IsHT9050() && ResetOKDeleyTime.Off())") == std::string::npos);   // 0196's timer is gone from the stop
    if (stop != std::string::npos) {
        const size_t gate = u.rfind("if(!(bCyflag[0] && bCyflag[1] && bCyflag[2] && bCyflag[3]))", stop);
        CHECK(gate != std::string::npos && stop - gate < 600);   // inside the floodgate wait of the all-in-place block
        CHECK(u.find("Tray Arm floodgate not up", stop) != std::string::npos && u.find("Tray Arm floodgate not up", stop) - stop < 900);
    }
    const size_t c1300 = u.find("fHome->iHomeStep=1310;   { extern void W906_HomeFloodgateWaitReset(); W906_HomeFloodgateWaitReset(); }");
    CHECK(c1300 != std::string::npos);
    if (c1300 != std::string::npos) {
        const size_t c1310 = u.find("case 1310:", c1300);
        CHECK(c1310 != std::string::npos && c1310 - c1300 < 400);    // the line that falls into case 1310
        CHECK(u.rfind("case 1300:", c1300) != std::string::npos && c1300 - u.rfind("case 1300:", c1300) < 1500);
    }
    CHECK(u.find("bool W906_HomeFloodgateWaitOver(int ms)") != std::string::npos);
    CHECK(u.find("void W906_HomeFloodgateWaitReset()") != std::string::npos);

    // ---- W79b HT9050 step 1310: Out Shuttle 2 does not wait for MLoaderZ (flag19) -- AI(W906-POOL7-W79B) 20261006 ---------
    std::printf("W-79 (b): HT9050 step 1310, Out Shuttle 2 goes to Left without waiting for MLoaderZ (flag19)\n");
    const size_t c1310 = u.find("case 1310:");
    size_t cNext = (c1310 == std::string::npos) ? std::string::npos : u.find("case ", c1310 + 10);   // the next numbered case
    while (cNext != std::string::npos && !(cNext + 5 < u.size() && u[cNext + 5] >= '0' && u[cNext + 5] <= '9'))
        cNext = u.find("case ", cNext + 5);
    CHECK(c1310 != std::string::npos && cNext != std::string::npos);
    const size_t mv = u.find("flag6=(MOT[MOutShuttle2].MotorMove(Prod.OutSHT[1].iLeft)==1);", c1310);
    CHECK(mv != std::string::npos && mv < cNext);              // the Out Shuttle 2 move is in case 1310
    if (mv != std::string::npos && c1310 != std::string::npos) {
        const size_t blk  = u.rfind("if(W906_IsHT9050())", mv);
        const size_t cond = u.rfind("else if(flag1 && flag2", mv);
        CHECK(blk != std::string::npos && blk > c1310 && cond != std::string::npos && cond > blk);   // HT9050 only
        if (cond != std::string::npos && cond < mv) {
            const std::string code = CodeOnly(u, cond, mv);
            CHECK(code.find("flag19") == std::string::npos);    // W-79 (b): no wait for MLoaderZ
            CHECK(code.find("flag10") != std::string::npos && code.find("flag14[3]") != std::string::npos &&
                  code.find("flag18[2]") != std::string::npos);   // the other flags still gate the move
        }
    }
    CHECK(u.find("flag19=(W906_HomeMove(MLoaderZ, W906_TrayZ9050Home(0))!=0); }", c1310) != std::string::npos);   // MLoaderZ still moves -- AI(W906-TRAYZ-PITCH) 20261007 laptop: to the taught Loader Home since machine cpp 0250 (was Prod.TrayZ_Home, always 0 on HT9050)
    {
        const size_t g = u.find("!(W906_IsHT9050() && flag1 && flag2 && flag3 && flag4 && flag5 && flag6", mv);
        CHECK(g != std::string::npos && g < cNext);
        if (g != std::string::npos)
            CHECK(CodeOnly(u, g, u.find("{", g)).find("flag19") != std::string::npos);   // 1310ORDER: all in place includes it
        const size_t done = u.find("if(flag1 && flag2 && flag3 && flag4 &&", g == std::string::npos ? c1310 : g);
        CHECK(done != std::string::npos && done < cNext);
        if (done != std::string::npos)
            CHECK(CodeOnly(u, done, u.find("{", done)).find("flag19") != std::string::npos);   // HOME complete still needs it
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
