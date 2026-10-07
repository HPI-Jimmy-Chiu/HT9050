// =============================================================================
//  test_wb_simpump.cpp -- WebBridgeTags PUMP MODE: does the state word tell the
//  truth, and does the spine actually advance?
//
//  AI(W906-SimPump) 20260813.
//
//  WHAT THIS GUARDS
//  Pump mode is the first thing outside tests/ that DRIVES the translated state
//  machine, and it puts a word on an operator-facing screen. Two ways that can go
//  wrong, and this TU exists for both:
//
//    (1) THE WORD LIES. machine.state must be null when we are not pumping (we are
//        not observing a machine, so we have nothing to report) and must track the
//        LIVE master guard when we are -- not a value latched at startup. O6 below
//        flips a guard term mid-run and requires the word to follow.
//
//    (2) THE PUMP DOES NOT ACTUALLY PUMP. A publisher can serve frames perfectly
//        while MainProc is never called, or called and throwing every time, and
//        from the browser those look identical to a healthy one. O3/O4 pin the
//        golden call-count and the exception count separately for that reason.
//
//  NO CONFIG IS LOADED, AND THAT IS PART OF THE TEST. PumpInit() is specified to
//  work with zero config (WebBridgeTags.h), the sim canary holds naturally at the
//  static-init defaults (MOTION_CARD_TYPE==0 != MotionCard_Contec==1, cmydef.cpp:3643
//  and cmydef.h:104; LastSet.iRealDummy==0==DUMMY != REALLY==2, cmydef.cpp:265-267),
//  and not loading it means this TU cannot write the shared production config the
//  way tests/test_wb_tags.cpp has to guard against.
// =============================================================================
#include "WebBridgeTags.h"
#include "w906_sim_build.h"   // AI(W906-W149) 20261007 (St02-E): W906_SIM_BUILD / W906_SIM_NOTE (card W-149 SIM-AWARE); on the old blank line
#include "cmydef.h"
#include "csystem.h"
#include "LastSet.h"   // LastSet.iRealDummy -- the sim canary's second term (O7)
#include "forms/fMain.h"   // AI(W906-MSTATE-P1) 20260923: fMain->palMainStatus->Caption (O5)

#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"

#include "wb_buildfact_tags.h"
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
#include <cstdio>
#include <string>
#include <vector>
#include "w906_test_motors.h"  // AI(W906-T6-MAINPROC) 20260923: MOT[].Motor 的開機不變式（見該檔檔頭）

// The tick oracle, to prove the trace stays BOUNDED across ticks -- csystemTraceClear()
// inside PumpTick() is the only thing that bounds it (csystem.cpp:198-200).
extern std::vector<int> g_csystemTrace;

static int g_total = 0;
static int g_fail  = 0;

static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

// Fetch one tag out of a freshly published generation.
static webbridge::TagValue publishAndGet(webbridge::TagSnapshot& snap, const char* tag)
{
    ht9045::PublishHandlerTags(snap);
    const webbridge::TagSnapshotView v = snap.read();
    webbridge::TagMap::const_iterator it = v.tags.find(tag);
    if (it == v.tags.end()) return webbridge::TagValue::makeNull();
    return it->second;
}

int main()
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("WB_SimPump", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    // AI(W906-T6-MAINPROC) 20260923: T6 讓 MainProc 照 golden 每拍呼叫 DoSystem -> ScanAllMotorStatus，
    //   它直接讀 MOT[i].Motor->Enable；golden 開機後 MOT[].Motor 必非 NULL，本測試沒跑 InitialMotorParameter，
    //   所以先補模擬馬達物件（tests/w906_test_motors.h）。
    W906_TestEnsureSimMotors();
    std::setvbuf(stdout, 0, _IONBF, 0);
    std::printf("==== WebBridgeTags pump mode (machine.state from palMainStatus + spine advance) ====\n");

    webbridge::TagSnapshot snap;

    // -------------------------------------------------------------------------
    //  O1 -- BEFORE PumpInit: the pump publishes nothing, and says so with null.
    // -------------------------------------------------------------------------
    std::printf("\n-- O1 pre-init: no pump, therefore no state to report\n");
    check(ht9045::PumpActive() == false, "O1 PumpActive() is false before PumpInit()");
    check(publishAndGet(snap, "machine.state").isNull(),
          "O1 machine.state is NULL before pumping (not \"HALT\" -- we are not "
          "observing a machine, so \"HALT\" would be an unsupported claim)");
    check(publishAndGet(snap, "clock.text").isNull(),
          "O1 clock.text is NULL before pumping");
    check(publishAndGet(snap, "pump.task.load").isNull(),
          "O1 pump.task.load is NULL before pumping");
    {
        const ht9045::PumpStats st = ht9045::PumpTelemetry();
        check(st.ticks == 0 && st.mainProcCalls == 0 && st.exceptions == 0 &&
              st.alive == false,
              "O1 telemetry is all-zero before pumping");
    }

    // The invariant test_wb_tags.cpp asserts in its step 1, re-asserted here so a
    // future pump tag that forgets to gate itself fails in THIS file too, next to
    // the code that would have caused it.
    //AI(W906-BU-C3) 20260916: narrowed in step with that file -- the blanket
    // "nothing is non-null" form died when BU-C3 merged 14 deliberately
    // compile-time/module-level tags.  The reference above was a line citation
    // (":68-75") that this very edit would have invalidated; it now names the
    // step instead.  Exemption list and reasoning: tests/wb_buildfact_tags.h.
    {
        webbridge::TagSnapshot s2;
        ht9045::PublishHandlerTags(s2);
        const webbridge::TagSnapshotView v = s2.read();
        std::size_t nonNull = 0;
        std::size_t factsSeen = 0;
        std::string leaked;
        for (webbridge::TagMap::const_iterator it = v.tags.begin();
             it != v.tags.end(); ++it) {
            if (it->second.isNull()) continue;
            ++nonNull;
            if (wbtest::IsBuildFactTag(it->first.c_str())) { ++factsSeen; continue; }
            if (!leaked.empty()) leaked += ", ";
            leaked += it->first;
        }
        std::printf("   pre-init snapshot: %u tags, %u non-null "
                    "(%u exempt build/module facts, %u leaked)\n",
                    (unsigned)v.tags.size(), (unsigned)nonNull,
                    (unsigned)factsSeen, (unsigned)(nonNull - factsSeen));
        if (!leaked.empty())
            std::printf("   LEAKED: %s\n", leaked.c_str());
        check(leaked.empty(),
              "O1 every tag whose source is the data layer is null pre-init "
              "(no process tag leaks a value into the pre-load control state)");
        check(factsSeen == wbtest::BuildFactTagCount(),
              "O1 every exempt build/module fact really IS published pre-init "
              "(the list in wb_buildfact_tags.h has not gone stale)");
        const ht9045::TagCoverage c = ht9045::HandlerTagCoverage();
        check(c.live == 0,
              "O1 machine-source coverage is still 0 live (process tags are "
              "excluded from the coverage denominator on purpose)");
    }

    // -------------------------------------------------------------------------
    //  O2 -- PumpInit arms the fixture. The canary holds at static-init defaults.
    // -------------------------------------------------------------------------
    std::printf("\n-- O2 PumpInit\n");
    std::string whyNot("(untouched)");
    const bool armed = ht9045::PumpInit(whyNot);
    if (!armed) std::printf("   PumpInit refused: %s\n", whyNot.c_str());
    check(armed, "O2 PumpInit() succeeds with no config loaded (sim canary holds "
                 "at the static-init defaults)");
    check(whyNot.empty(), "O2 PumpInit() clears whyNot on success");
    check(ht9045::PumpActive(), "O2 PumpActive() is true after PumpInit()");

    //AI(W906-IdlePump) 20260817: THESE TWO ASSERTIONS ARE INVERTED, deliberately.
    // They used to require SystemStart==true and fAllMotorHome==true, i.e. that
    // PumpInit had STARTED the machine. The user rejected that behaviour -- a
    // freshly opened BCB6 HT9045 is idle until the operator presses HOME then
    // START -- so PumpInit no longer writes either term (WebBridgeTags.cpp, the
    // "OPENED, NOT STARTED" fixture). Asserting the opposite turns this block into
    // a REGRESSION GUARD: if anyone reintroduces the force, this fails first.
    //
    // NOT COVERED by this change, and covered elsewhere on purpose:
    //   * "the word reads SIM RUN when all three guard terms hold" -- still tested,
    //     in O5, with the TEST setting the terms instead of production code.
    //   * "the word is re-derived per publish, not latched" -- still tested in O6.
    //   * nothing tests a real START, because nothing can raise SoftStart offline
    //     yet; that arrives with the browser write path (SCOPE.md section 2.6) and
    //     needs its own test then.
    check(SoftStop == false,      "O2 fixture set SoftStop=false (a fresh boot is "
                                  "not soft-stopped)");
    check(SystemStart == false,   "O2 fixture did NOT start the machine "
                                  "(SystemStart stays false -- opened, not started)");
    check(fAllMotorHome == false, "O2 fixture did NOT claim the motors homed "
                                  "(fAllMotorHome stays false -- HOME comes first "
                                  "on a real machine, then START)");
    check(InitialOK == true,      "O2 fixture set InitialOK=true (MainProc head "
                                  "guard, csystem.cpp:3005 -- a real opened "
                                  "program does finish initialisation)");

    // -------------------------------------------------------------------------
    //  O3 -- the spine ADVANCES: golden's own counter, +1 per tick.
    // -------------------------------------------------------------------------
    std::printf("\n-- O3 spine advance (golden's MainProc counter, csystem.h:52)\n");
    const unsigned int before = ht9045::PumpTelemetry().mainProcCalls;
    const int kTicks = 8;
    for (int i = 0; i < kTicks; ++i) ht9045::PumpTick();
    const ht9045::PumpStats after = ht9045::PumpTelemetry();

    std::printf("   mainProcCalls %u -> %u over %d ticks; exceptions=%lu\n",
                before, after.mainProcCalls, kTicks,
                (unsigned long)after.exceptions);
    check(after.mainProcCalls == before + (unsigned)kTicks,
          "O3 GetMainProcCallCount() advanced exactly once per PumpTick()");
    check(after.ticks == (unsigned long long)kTicks,
          "O3 telemetry tick count matches the PumpTick() calls");
    check(after.exceptions == 0,
          "O3 ZERO ticks threw out of MainProc() (a pump that throws every tick "
          "looks healthy from the browser)");
    check(after.alive, "O3 IsMainProcAlive(60) is true after pumping");

    // -------------------------------------------------------------------------
    //  O4 -- the tick oracle trace stays BOUNDED. Without csystemTraceClear() in
    //  PumpTick() this grows for the life of the process (csystem.cpp:200), which
    //  is a leak in a publisher meant to run for hours.
    // -------------------------------------------------------------------------
    std::printf("\n-- O4 trace is bounded\n");
    const std::size_t sizeAfter8 = g_csystemTrace.size();
    for (int i = 0; i < 40; ++i) ht9045::PumpTick();
    const std::size_t sizeAfter48 = g_csystemTrace.size();
    std::printf("   g_csystemTrace: %u tokens after 8 ticks, %u after 48\n",
                (unsigned)sizeAfter8, (unsigned)sizeAfter48);
    check(sizeAfter48 <= sizeAfter8 + 4,
          "O4 trace does NOT grow with tick count (PumpTick clears it each tick)");

    //AI(W906-HOME-W1) 20260919: 這裡原本只有一條
    //     check(sizeAfter48 > 0, "O4 trace is non-empty, ...engines really are being called")
    // 而它斷言的是**接上歸零階梯之前**的 MainProc —— 那時 MainProc 是 slim body
    // （ScanSystemSensor + spine），所以不管有沒有歸零都會呼叫到 engines。
    //
    // golden 不是這樣。golden csystem.cpp:17586-17602（本樹 csystem.cpp:1480-1496
    // 的 #if 0 副本可逐字對照）在鏈之前就寫著：
    //     if(fAllMotorHome==false && fContact->fShow==false && bDestoryOnSht==false)
    //         iHome=1;                    <- 沒歸零就自己排歸零
    // 然後鏈走到 `else if(iHome==1)` -> DoHomeProcess() -> 這個 tick 結束。
    // ⇒ **沒歸零的機台不會走到 spine。** 那是 golden 的設計。
    //
    // 而這支測試的 fixture **刻意**讓 fAllMotorHome 維持 false（O2 自己的訊息：
    // "HOME comes first on a real machine, then START"）。兩件事放在一起，
    // 舊斷言就變成「宣稱沒歸零的機台也在跑 engines」—— 那正是要避免的說法。
    //
    // 改成兩段，是**加強**不是放寬：先釘住 golden 的「沒歸零就不跑」，
    // 再明確宣告已歸零、證明 engines 真的會被呼叫。
    check(sizeAfter48 == 0,
          "O4a NOT homed -> trace is EMPTY: golden's ladder routes an un-homed "
          "machine to DoHomeProcess and returns before the spine "
          "(csystem.cpp:17586-17602 + the iHome==1 arm)");

    {
        //  這一行是**測試夾具的宣告**，不是對機台狀態的主張：它說的是
        //  「現在假設這台已經歸零過了」，這樣才測得到 spine 那一段。
        const bool savedHome = fAllMotorHome;
        const int  savedIHome = iHome;
        const bool savedStart = SystemStart;
        fAllMotorHome = true;
        //AI(W906-Q15) 20260920: **SystemStart 也要宣告。**
        //  20260920 補上了 golden csystem.cpp:17130 的 `if(SystemStart)` ——
        //  它把歸零階梯連同 spine 整組包住（:18728 的 DoAllProcess 實測
        //  brace depth = 5，在同一個 if 裡）。使用者 Q15 確認真實機台就是這樣：
        //  「只有 Start 時候會檢查是否有歸零」。
        //  ⇒ 這個 fixture 的 O2 刻意讓 SystemStart 維持 false（「opened, not
        //    started」），所以 O4b 想測 spine 就必須在這裡明講「現在假設按過
        //    START 了」—— 與上一行對 fAllMotorHome 的宣告同一個性質。
        //  ⚠ 少了這一行，O4b 會紅，而那個紅**是守衛在正確工作**，不是回歸。
        SystemStart = true;
        //AI(W906-HOME-W1) 20260919: iHome 也要清。
        //  上面那 48 個 tick 已經讓 golden 的
        //  `if(fAllMotorHome==false ...) iHome=1;`（csystem.cpp:17586）踩下去，
        //  而**清掉 iHome 的唯一一行在 DoHomeProcess() 的本體裡**。
        //  ⚠ AI(W906-HOME-C2) 20260920 更正：原文說那個本體「包在恆假的 seam
        //    `W906G4_ProcessMotorHome` 裡」—— **那個 seam 20260920 已經退休**，
        //    DoHomeProcess 現在呼叫的是 uhome.cpp 的真 `ProcessMotorHome()`。
        //    這一行仍然要留，但理由換了：這個行程沒有跑過
        //    `InitialMotorParameter()`，所以 ProcessMotorHome 的 bring-up 守衛
        //    會拒絕執行（印 "refusing -- machine not brought up"），
        //    iHome 一樣不會被清。
        //  → 只設 fAllMotorHome=true 不夠，`iHome==1` 那一臂還是會贏。
        iHome = 0;
        for (int i = 0; i < 8; ++i) ht9045::PumpTick();
        const std::size_t sizeHomed = g_csystemTrace.size();
        std::printf("   g_csystemTrace with fAllMotorHome=true: %u tokens\n",
                    (unsigned)sizeHomed);
        //AI(W906-T6-MAINPROC) 20260923: 重新校準（pt-wave-loop 規則二：期望值是照鷹架校準的）。
        //  原本斷言 sizeHomed > 0。T6 把 MainProc 照 golden 翻完之後，每一拍都先跑 DoSystem()，
        //  而 golden DoSystem() 在 `if(SystemStart)` 裡逐項檢查安全門／靜電風扇／壓縮空氣／乾燥空氣……，
        //  任何一項不對就 StopAllMotor + 告警 + SystemStart=false（golden csystem.cpp:4404-4530 一帶）。
        //  這支測試刻意不載入任何設定（檔頭「NO CONFIG IS LOADED, AND THAT IS PART OF THE TEST」），
        //  出貨組態下沒有 IO 卡，感測器讀起來全是「沒有」—— 所以上面夾具宣告的「已歸零、已按 START」
        //  在第一拍就被 golden 撤銷，spine 走不到。test_mainproc_guard.cpp PART 2 用 gdb 觀察點量過
        //  是壓縮空氣不足那一臂（golden :4448-4452，WAR1603）；哪一臂先觸發取決於感測器預設值，這裡不綁定。
        //  ⇒ 改成斷言 golden 在這個環境真正會做的事：
        check(W906_SIM_BUILD ? (sizeHomed == 0 && SystemStart == true) : (sizeHomed == 0 && SystemStart == false),
              "O4b fixture declares homed+started, but golden DoSystem's safety interlocks "
              "(csystem.cpp:4404-4530; measured: air-pressure WAR1603 at :4448-4452) revoke it "
              "on the first tick -> spine not reached" W906_SIM_NOTE(" -- SIM: the arm that revokes START here (WAR1603 air, golden 0618 csystem.cpp:4448-4452) sits in DoSystem's #ifndef SOFT_SIMULTE block :4441-4493 (port csystem.cpp:16792), compiled out in SIM, so START stays; the spine is still not reached (measured; that stop is not a SOFT_SIMULTE arm)"));
        //  NOT COVERED（移交）：「已歸零、已按 START、機台就緒 -> engines 真的被呼叫」。
        //    改由 wb_serve 端到端量（START 前後各數一次 DoInArm_9045，gdb 計數中斷點）：
        //    20260923 T6 實測 START 前 0 次、START→歸零→Initial Start 之後 21 次（DoAllProcess 44 次）。
        fAllMotorHome = savedHome;
        iHome = savedIHome;
        SystemStart = savedStart;
    }

    // -------------------------------------------------------------------------
    //  O5 -- the published word and the cursors.
    // -------------------------------------------------------------------------
    std::printf("\n-- O5 published state\n");
    {
        ht9045::PublishHandlerTags(snap);
        const webbridge::TagSnapshotView v = snap.read();

        //AI(W906-MSTATE-P1) 20260923: **這一條斷言反過來了，而且是刻意的。**
        //
        //  舊版要求「pumping 時 machine.state 一定要有值」，而且那個值是
        //  "SIM IDLE"。那只有在發布端**自己編一個字**的前提下才成立 ——
        //  SIM RUN/HALT/IDLE 描述的是這個行程的守衛，不是機台狀態。把它放進一個
        //  標著「機台狀態」的欄位，操作員會把它讀成機台的狀態。
        //
        //  machine.state 現在發的是 golden 的真值 fMain->palMainStatus->Caption。
        //  寫它的 TfMain::ShowNowStatus（golden main.cpp:22190-22318）與餵它的
        //  ckernel ShowRunLabel（golden ckernel.cpp:935-1726）都還沒翻，所以
        //  caption 是空的 ⇒ 正確答案是 **null**（不可知），不是一個編出來的字。
        //  守衛沒有遺失：pump.guard.softStop/systemStart/allMotorHome 照舊發送。
        //
        //  ⚠ 只斷言 null 的話，這會變成一個**不可能失敗的 gate** —— 接線整條斷掉
        //    也照樣綠。所以下面第二段自己塞一個 caption 再發一次，證明值真的會
        //    **逐字**流到 tag。兩面都測，才是 gate。
        //AI(W906-MSTATE-P2) 20260924: 上面那段說明的前提（「寫入者都還沒翻」）今天不成立了 ——
        //  ShowNowStatus 已翻（cMainStatus.cpp，golden main.cpp:22190-22317），ShowRunLabel 已解閘
        //  （ckernel.cpp:1940，golden ckernel.cpp:935-1726），而 T6 之後 DoSystem 結尾的 DoSystemMessage()
        //  每個 tick 都會叫到它。所以抽過 tick 之後 caption 有值是**正確**的，斷言改成：
        //  machine.state 必須等於 ShowRunLabel 剛寫進 palMainStatus->Caption 的那個字（逐字）。
        //  本夾具實測是 "HALT"（沒有啟動的機台）；不把 "HALT" 寫死 —— 出貨組態下互鎖可能讓它走別的臂，
        //  這條要釘的是「發的是 golden 寫的那個字」，不是「一定是 HALT」。
        //  「null 表示不可知」的性質沒有丟：O6 最後一條仍然釘著「caption 清空 -> null」。
        webbridge::TagMap::const_iterator st = v.tags.find("machine.state");
        const bool haveState = (st != v.tags.end()) && !st->second.isNull();
        const AnsiString capAfterTick = (fMain != 0 && fMain->palMainStatus != 0)
                                        ? fMain->palMainStatus->Caption : AnsiString("");
        std::printf("   after pumping: palMainStatus->Caption = \"%s\"\n", capAfterTick.c_str());
        check(haveState && capAfterTick.Length() > 0 &&
              st->second.asString() == std::string(capAfterTick.c_str()),
              "O5 after pumping, machine.state is the word golden ShowRunLabel just wrote "
              "into palMainStatus->Caption, verbatim");

        webbridge::TagMap::const_iterator sc = v.tags.find("machine.stateCode");
        check(sc == v.tags.end() || sc->second.isNull(),
              "O5 machine.stateCode is NULL -- iSECSGEMMachineState's only writer "
              "(ShowNowStatus, golden main.cpp:22277-22287) runs only under "
              "IniConfig.bEnable_SECS_GEM, which is off in this fixture; 0 would read "
              "as a real 1-based code");

        //AI(W906-MSTATE-P1) 20260923: 第二面 —— 給 caption 一個值，tag 必須逐字送出。
        //  逐字很重要：golden 有 20+ 處拿 palMainStatus->Caption 跟字串字面比對
        //  （Command.cpp / automation.cpp / aseTest.cpp），任何「順手改漂亮」都會
        //  讓那些比對靜默失效。這條斷言把「逐字」釘住。
        if (fMain != 0 && fMain->palMainStatus != 0) {
            const AnsiString saved = fMain->palMainStatus->Caption;
            fMain->palMainStatus->Caption = AnsiString("Running");
            ht9045::PublishHandlerTags(snap);
            const webbridge::TagSnapshotView v2 = snap.read();
            webbridge::TagMap::const_iterator st2 = v2.tags.find("machine.state");
            const bool got = (st2 != v2.tags.end()) && !st2->second.isNull();
            check(got, "O5 machine.state carries a value once palMainStatus has one");
            if (got) {
                std::printf("   machine.state = \"%s\"\n",
                            st2->second.asString().c_str());
                check(st2->second.asString() == std::string("Running"),
                      "O5 machine.state is palMainStatus->Caption VERBATIM "
                      "(no re-wording -- golden compares this string literally)");
            }
            fMain->palMainStatus->Caption = saved;
            ht9045::PublishHandlerTags(snap);
        } else {
            check(false, "O5 fMain->palMainStatus must exist (forms/fMain.h:287)");
        }

        webbridge::TagMap::const_iterator src = v.tags.find("machine.stateSource");
        check(src != v.tags.end() && !src->second.isNull() &&
              src->second.asString().size() > 0,
              "O5 machine.stateSource states the provenance beside the word");

        check(v.tags.find("clock.text") != v.tags.end() &&
              !v.tags.find("clock.text")->second.isNull(),
              "O5 clock.text is live while pumping");

        // Raw guard terms, so the derived word can be checked against its inputs.
        //AI(W906-IdlePump) 20260817: assert the published term TRACKS the global
        // rather than asserting a fixed true. That is the property that actually
        // matters -- "nothing derived is published without its inputs" -- and unlike
        // a hardcoded expectation it keeps holding whatever the machine state is.
        check(v.tags.find("pump.guard.systemStart") != v.tags.end() &&
              v.tags.find("pump.guard.systemStart")->second.asBool() == SystemStart,
              "O5 pump.guard.systemStart publishes the raw term, matching the global");
        check(v.tags.find("pump.guard.softStop") != v.tags.end() &&
              v.tags.find("pump.guard.softStop")->second.asBool() == false,
              "O5 pump.guard.softStop publishes the raw term");

        // Cursors: the evidence the engines are cycling rather than merely linked.
        const char* cursors[] = {
            "pump.task.load", "pump.task.inArm", "pump.task.outArm",
            "pump.task.sht1", "pump.task.sht2", "pump.task.testHead",
            "pump.task.catchTray"
        };
        std::size_t present = 0;
        for (std::size_t i = 0; i < sizeof(cursors) / sizeof(cursors[0]); ++i) {
            webbridge::TagMap::const_iterator c = v.tags.find(cursors[i]);
            if (c != v.tags.end() && !c->second.isNull()) ++present;
        }
        std::printf("   engine cursors carrying a value: %u of 7\n", (unsigned)present);
        check(present == 7, "O5 all seven engine cursors publish a value while pumping");
    }

    //AI(W906-IdlePump) 20260817: from here on the TEST holds the guard open, so the
    // "word says RUN" and anti-latch properties stay covered after production code
    // stopped forcing them. A test may set machine globals; production may not, and
    // that asymmetry is the entire point of the change.
    // Only PublishHandlerTags is called below -- never PumpTick -- so raising these
    // does NOT let an engine run, and DoLoad never reaches its no-tray retry.
    SystemStart   = true;                  // csystem.cpp:4239 term 2
    fAllMotorHome = true;                  // csystem.cpp:4239 term 3
    //AI(W906-MSTATE-P1b) 20260923: 這兩段原本斷言 machine.state 會隨守衛在
    //  "SIM RUN"/"SIM HALT" 之間跳。machine.state 已改成發 golden 的真值
    //  （palMainStatus->Caption），所以那個斷言掛錯對象了。
    //  **但它測的性質是對的、而且值得保留**：
    //    (1) 守衛必須被發布出去、且隨著改變 -> 移到 pump.guard.*（它們本來就在發）
    //    (2) 值必須每次發布重新推導、不是 PumpInit 時 latch 一次 -> 移到 caption
    //  兩個性質都留著，只是各自掛回正確的來源。
    {
        const webbridge::TagValue g1 = publishAndGet(snap, "pump.guard.systemStart");
        const webbridge::TagValue g2 = publishAndGet(snap, "pump.guard.allMotorHome");
        check(!g1.isNull() && g1.asBool() == true,
              "O5 pump.guard.systemStart follows the term the test just raised");
        check(!g2.isNull() && g2.asBool() == true,
              "O5 pump.guard.allMotorHome follows the term the test just raised");
        //AI(W906-MSTATE-P2) 20260924: 原本斷言「守衛拉起來 machine.state 仍是 null」—— 前提是 caption
        //  沒有寫入者。現在有了，所以改測同一個性質的正確形狀：只拉守衛、不跑 tick（這一段只呼叫
        //  PublishHandlerTags），machine.state 必須**不變** —— 守衛不是機台狀態，發布端不得從守衛推導出一個字。
        const AnsiString capBefore = (fMain != 0 && fMain->palMainStatus != 0)
                                     ? fMain->palMainStatus->Caption : AnsiString("");
        const webbridge::TagValue s = publishAndGet(snap, "machine.state");
        std::printf("   guard raised, no tick -> \"%s\" (caption \"%s\")\n",
                    s.isNull() ? "<null>" : s.asString().c_str(), capBefore.c_str());
        check(capBefore.Length() == 0 ? s.isNull()
                                      : (!s.isNull() && s.asString() == std::string(capBefore.c_str())),
              "O5 raising the guard alone leaves machine.state equal to the caption -- the guard "
              "is not the machine state, and conflating them is what MSTATE-P1 fixed");
    }

    // -------------------------------------------------------------------------
    //  O6 -- THE ANTI-LATCH CHECK. 值必須每次發布重新推導，不是 PumpInit 時抓一次。
    //  AI(W906-MSTATE-P1b) 20260923: 對象從守衛字改成 caption（machine.state 的
    //  新來源）＋ pump.guard.softStop（守衛的新家）。兩條路都要證明沒有 latch。
    // -------------------------------------------------------------------------
    std::printf("\n-- O6 the published values track LIVE inputs, nothing is latched\n");
    SoftStop = true;                       // one of the three terms, csystem.cpp:4239
    {
        const webbridge::TagValue g = publishAndGet(snap, "pump.guard.softStop");
        std::printf("   with SoftStop=true  -> guard %s\n",
                    g.isNull() ? "<null>" : (g.asBool() ? "true" : "false"));
        check(!g.isNull() && g.asBool() == true,
              "O6 pump.guard.softStop follows the term the moment it blocks the engines");
    }
    SoftStop = false;
    {
        const webbridge::TagValue g = publishAndGet(snap, "pump.guard.softStop");
        check(!g.isNull() && g.asBool() == false,
              "O6 pump.guard.softStop returns when the term clears (not latched)");
    }
    if (fMain != 0 && fMain->palMainStatus != 0) {
        const AnsiString saved = fMain->palMainStatus->Caption;
        fMain->palMainStatus->Caption = AnsiString("Running");
        const webbridge::TagValue a = publishAndGet(snap, "machine.state");
        fMain->palMainStatus->Caption = AnsiString("HALT");
        const webbridge::TagValue b = publishAndGet(snap, "machine.state");
        fMain->palMainStatus->Caption = AnsiString("");
        const webbridge::TagValue c = publishAndGet(snap, "machine.state");
        std::printf("   caption Running/HALT/\"\" -> \"%s\" / \"%s\" / %s\n",
                    a.isNull() ? "<null>" : a.asString().c_str(),
                    b.isNull() ? "<null>" : b.asString().c_str(),
                    c.isNull() ? "<null>" : c.asString().c_str());
        check(!a.isNull() && a.asString() == std::string("Running"),
              "O6 machine.state follows the caption (Running)");
        check(!b.isNull() && b.asString() == std::string("HALT"),
              "O6 machine.state follows the caption when it CHANGES (HALT) -- "
              "re-derived every publish, not latched");
        check(c.isNull(),
              "O6 machine.state returns to NULL when the caption is cleared "
              "(no stale last-known value left on the screen)");
        fMain->palMainStatus->Caption = saved;
    }

    // -------------------------------------------------------------------------
    //  O7 -- THE SAFETY GATE. PumpInit must REFUSE when the sim canary says the
    //  hardware interlock is live.
    //
    //  Why this is worth a test rather than a code read: the refusal condition is
    //  the logical NEGATION of the interlock's own self-disable guard
    //  (ainarm9045.cpp:1891-1893 returns early when
    //  `MOTION_CARD_TYPE!=Contec || iRealDummy!=REALLY`, so the interlock is LIVE
    //  exactly when `==Contec && ==REALLY`). An && flipped to || here would still
    //  compile, still pass every other check in this file, and would let a
    //  config-loaded pump be the first thing ever to execute CCLink/Ltc/MOT bodies
    //  offline. That is the one failure in this feature with a physical-machine
    //  consequence, so it gets its own oracle.
    // -------------------------------------------------------------------------
    std::printf("\n-- O7 PumpInit REFUSES when the interlock is live\n");
    {
        const int savedCard  = MOTION_CARD_TYPE;
        const int savedDummy = LastSet.iRealDummy;

        MOTION_CARD_TYPE     = MotionCard_Contec;   // cmydef.h:104 == 1
        LastSet.iRealDummy   = REALLY;              // cmydef.cpp:265 == 2

        std::string why("(untouched)");
        const bool ok = ht9045::PumpInit(why);
        std::printf("   PumpInit -> %s; whyNot=\"%s\"\n",
                    ok ? "true" : "false", why.c_str());
        check(W906_SIM_BUILD ? ok : !ok, "O7 PumpInit() REFUSES with MOTION_CARD_TYPE==Contec AND "
                   "iRealDummy==REALLY (the interlock at ainarm9045.cpp:1891 is live)" W906_SIM_NOTE(" -- SIM: PumpInit's SOFT_SIMULTE arm reports the canary and continues (WebBridgeTags.cpp:471-510, AI(W906-ST-S3-B2a); V906-only, no golden)"));
        check(W906_SIM_BUILD ? why.empty() : !why.empty(), "O7 the refusal explains itself in whyNot" W906_SIM_NOTE(" -- SIM: no refusal, whyNot cleared (WebBridgeTags.cpp:510)"));

        // Only ONE of the two terms live must NOT refuse -- the interlock
        // self-disables on either, so refusing there would be a false positive that
        // makes the pump unusable on a box with a Contec card.
        LastSet.iRealDummy = savedDummy;            // back to DUMMY, card still Contec
        std::string why2("(untouched)");
        const bool ok2 = ht9045::PumpInit(why2);
        check(ok2, "O7 PumpInit() ALLOWS a Contec card when iRealDummy!=REALLY "
                   "(one term is enough to self-disable the interlock)");

        MOTION_CARD_TYPE   = savedCard;
        LastSet.iRealDummy = savedDummy;
    }

    { void W906_FlushEdgeEveryWindow(); W906_FlushEdgeEveryWindow(); }   // AI(W906-FLUSH-100) 20261007: O9, end of file
    std::printf("\ntest_wb_simpump: %d checks, %d failure(s)\n", g_total, g_fail);
    return g_fail == 0 ? 0 : 1;
}

// =============================================================================
//  O9 AI(W906-FLUSH-100) 20261007: EastSun「回完home 畫面怎還是都一樣?」-- after HOME the main page kept "Homing". ShowRunLabel /
//  ShowRunLed (golden edge check, ckernel.cpp:1949 / :1738) repaint only when FlushFlag differs from their previous call, and
//  DoSystemMessage calls them on 1 beat in 6. W906_FlushFlagTick ran on wall clock (>= 250 ms): at the 100 ms beat that is 2 flips per
//  6 beats (even) -> the same value at every call -> frozen. O5 above only checks that SOME word is published, so it stayed green.
//  This drives W906_FlushFlagTick exactly as PumpTick does (once per beat) and samples FlushFlag on every 6th beat like
//  DoSystemMessage: every sample must differ from the one before (the old wall-clock code, called back to back, never flipped at all).
// =============================================================================
namespace ht9045 { void W906_FlushFlagTick(); }
void W906_FlushEdgeEveryWindow()
{
    std::printf("\n-- O9 FlushFlag shows an edge to every 6-beat ShowRunLabel window (any beat length)\n");
    extern bool bSECSGEMAlarm;
    const bool savedAlarm = bSECSGEMAlarm;
    bSECSGEMAlarm = false;
    for (int k = 0; k < 6; ++k) ht9045::W906_FlushFlagTick();                  // settle into a window
    bool prev = FlushFlag;
    int edges = 0, windows = 20;
    for (int w = 0; w < windows; ++w)
    {
        for (int k = 0; k < 6; ++k) ht9045::W906_FlushFlagTick();
        if (FlushFlag != prev) ++edges;
        prev = FlushFlag;
    }
    char buf[160];
    std::snprintf(buf, sizeof(buf), "O9 FlushFlag changed in %d of %d six-beat windows (must be every one, or the status word / tower lamps freeze)", edges, windows);
    check(edges == windows, buf);
    bSECSGEMAlarm = savedAlarm;
}
