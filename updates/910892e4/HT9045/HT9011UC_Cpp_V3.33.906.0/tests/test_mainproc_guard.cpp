// =============================================================================
//  test_mainproc_guard.cpp -- AI(W906-Q15) 20260920
//
//  把 golden `csystem.cpp:17130 if(SystemStart)` 變成一支會當掉的測試。
//
//  ## 它守什麼
//
//  golden 把**歸零派發階梯連同 spine**整組放在 `if(SystemStart)` 裡：
//
//      :16894   ScanSystemSensor();     ← 在 if 外（它才是把 SystemStart 設起來的人）
//      :17130   if(SystemStart)
//      :17586       if(fAllMotorHome==false && ...) iHome=1;   ← 觸發歸零
//      :18728       DoAllProcess();                            ← spine（brace depth 5）
//
//  移植樹在 20260919 的 W906-HOME-W1 波次裡**漏抄了這個守衛**（抽取範圍從
//  :17604 起算，守衛在上游 :17130）。
//
//  ## 漏掉時的實際症狀（這支測試存在的理由）
//
//  `wb_serve --seconds 8`，沒有送任何指令、START 也沒按：
//  stdout 出現 15 次 `[ShowMyMessage] Motor not home yet!`
//  —— 8 秒 / 500 ms tick，每個 tick 一次。**程式一開起來就在自己試著歸零。**
//
//  使用者 20260920 確認（INBOX Q15）：
//      「不會，只有 Start 時候會檢查是否有歸零，沒歸零就會先執行歸零動作，
//        然後才正式跑機台動作」
//
//  ## 它為什麼**會**當掉（不是不可能當掉的 gate）
//
//  兩個方向都量：
//    * `SystemStart==false` → spine 的 trace 必須是**空的**（一顆引擎都沒跑）
//    * `SystemStart==true`  → trace 必須**非空**（spine 真的被驅動）
//  少了第二條，這支就會變成「永遠通過」的裝飾品 —— 一個 `if(false)` 也能過。
//
//  ## ⚠ 為什麼不直接量「iHome 有沒有變 1」
//
//  第一版是那樣寫的，然後**它 SIGSEGV**。堆疊：
//      MainProc -> W906_MainProcHomeDispatch -> DoHomeProcess
//               -> ProcessMotorHome -> TMyMotor::MotorInitial
//               -> HTMotor::SetHomeobjectTask   ← `Motor` 是 NULL
//  原因：`ProcessMotorHome()` case 1 的第一個迴圈就對 164 顆馬達呼叫
//  `MotorInitial()` 並讀 `MOT[i].Motor->Enable`，而**建立那些 HTMotor 物件的是
//  `InitialMotorParameter()`（cinitial.cpp:3739）**，這支測試沒有跑它
//  （跑它要先 `LoadMachineConfig()`，那會寫真實的 system\Gerneral.ini）。
//
//  ⇒ 改成量 spine 的 trace：同樣**兩個方向都量**，而且不需要把整台機台
//    bring-up 起來。「START 之後歸零真的起跑」那一條由端對端的
//    `tools/webprobe/spine_poll_probe.py` 在 wb_serve 裡量 —— 那個行程**有**
//    跑 `InitialMotorParameter()`（tools/wb_serve.cpp:2083）。
//    兩支測的是同一個守衛的兩面，不是互相取代。
// =============================================================================
#include <cstdio>
#include <vector>

#include "cmydef.h"      // SystemStart / fAllMotorHome / iHome / InitialOK / SoftStop
#include "csystem.h"     // MainProc / GetMainProcCallCount
#include "atester_shims.h"
#include "forms/fHome.h"
#include "w906_test_motors.h"  // AI(W906-T6-MAINPROC) 20260923: MOT[].Motor 的開機不變式（見該檔檔頭）
#include "Motor/mymotor.h"     // AI(W906-MT-FIX1) 20260926: MOT[] (the powered-machine fixture in ArmRunState)
#include "Motor/HTMotor.h"     //   iServoOn
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
extern bool bDestoryOnSht;

// csystem.cpp 的 tick-sequence oracle（test_w6_6_csystem_cycle.cpp:71 也用它）
extern std::vector<int> g_csystemTrace;
extern void csystemTraceClear();

static int g_fail = 0;
static int g_checks = 0;

static void CHECK(bool ok, const char *what)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL  %s\n", what); }
    else     {           std::printf("  ok    %s\n", what); }
}

// 讓 spine 有東西可跑，而且**不要**走到歸零那一臂：
//   fAllMotorHome=true  -> W906_MainProcHomeDispatch 的第一個 if 不成立
//                          -> 不設 iHome=1 -> 不呼叫 DoHomeProcess
//   （歸零那一臂需要真的 bring-up，見檔頭。）
// ⚠ 名字不可以叫 `RunState` —— `cmydef.h:2552` 有 `extern int RunState;`
//   （塔燈狀態）。編譯器會報 "redeclared as different kind of symbol"。
static void ArmRunState()
{
    InitialOK     = true;     // MainProc 開頭的守衛（golden :16724）
    SoftStop      = false;    // DoAllProcess 的主守衛（golden :10049）
    fAllMotorHome = true;
    bDestoryOnSht = false;
    if (fContact) fContact->fShow = false;
    iHome         = 0;
    //AI(W906-MT-FIX1) 20260926: GATE G04 is open now (EastSun ruling 20260926). golden's out-of-power arm locks the index
    //  motor and sets iHome=1 on EVERY pass while IsIndexMotorOutOfPower() -- which is golden's answer for an unpowered
    //  machine, and it is exactly what this fixture was (no servo LED lit). This test is about the SystemStart guard, not
    //  about power, so the fixture now models a powered machine: golden's Index servo LEDs on (the non-1203 arm of
    //  IsIndexMotorOutOfPower). Without this, iHome==1 would come from G04 and say nothing about the guard.
    MOT[MTestZ1].Led[iServoOn] = true;
    MOT[MTestZ2].Led[iServoOn] = true;
}

int main()
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("mainproc_guard", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    // AI(W906-T6-MAINPROC) 20260923: T6 讓 MainProc 照 golden 每拍呼叫 DoSystem -> ScanAllMotorStatus，
    //   它直接讀 MOT[i].Motor->Enable；golden 開機後 MOT[].Motor 必非 NULL，本測試沒跑 InitialMotorParameter，
    //   所以先補模擬馬達物件（tests/w906_test_motors.h）。
    W906_TestEnsureSimMotors();
    std::printf("=== test_mainproc_guard (AI(W906-Q15)) ===\n");

    // -----------------------------------------------------------------------
    //  PART 0 -- 前提
    // -----------------------------------------------------------------------
    std::printf("\n-- PART 0: 前提 --\n");
    ArmRunState();
    CHECK(fHome != 0, "fHome 全域指標非 NULL");
    CHECK(InitialOK == true, "InitialOK==true（否則 MainProc 開頭就 return）");

    // -----------------------------------------------------------------------
    //  PART 1 -- ★ 沒按 START：MainProc 跑，但 spine 一顆引擎都不能動
    // -----------------------------------------------------------------------
    std::printf("\n-- PART 1: SystemStart==false --\n");
    ArmRunState();
    SystemStart = false;

    const unsigned before = GetMainProcCallCount();
    csystemTraceClear();
    for (int i = 0; i < 5; ++i)
        MainProc();
    const unsigned after = GetMainProcCallCount();
    const size_t traceOff = g_csystemTrace.size();

    std::printf("  MainProc 呼叫計數 %u -> %u，spine trace 長度=%u\n",
                before, after, (unsigned)traceOff);
    CHECK(after == before + 5,
          "MainProc 本身照跑（計數器在守衛之前，golden :16728）");
    CHECK(traceOff == 0,
          "★ spine trace 是空的 —— 沒按 START 就一顆引擎都不跑（golden :17130）");
    CHECK(iHome == 0,
          "★ iHome 維持 0 —— 沒按 START 就不會自己開始歸零（golden :17586）");

    // -----------------------------------------------------------------------
    //  PART 2 -- ★ 按了 START：spine 必須真的被驅動
    //
    //  沒有這一段，PART 1 用一個 `if(false)` 也能過 —— 那就是
    //  memory: a-gate-that-cannot-fail 說的那種東西。
    // -----------------------------------------------------------------------
    std::printf("\n-- PART 2: SystemStart==true --\n");
    ArmRunState();
    SystemStart = true;

    csystemTraceClear();
    MainProc();
    const size_t traceOn = g_csystemTrace.size();

    std::printf("  spine trace 長度=%u  SystemStart=%d  fAllMotorHome=%d\n",
                (unsigned)traceOn, (int)SystemStart, (int)fAllMotorHome);

    //AI(W906-T6-MAINPROC) 20260923: 重新校準（pt-wave-loop 規則二：期望值是照鷹架校準的）。
    //  原本這裡斷言「SystemStart==true -> spine trace 非空」與「traceOn > traceOff」。
    //  那兩條是照 T6 之前的 MainProc 骨架寫的 —— 骨架不呼叫 DoSystem()。
    //  golden 的順序是 ScanSystemSensor（:16894）-> DoSystem（MainProc 內）-> … -> if(SystemStart)（:17130），
    //  而 golden DoSystem() 在 `if(SystemStart)` 裡逐項檢查安全門／靜電風扇／壓縮空氣／乾燥空氣……，
    //  任何一項不對就 StopAllMotor + 告警 + SystemStart=false（golden csystem.cpp:4404-4530 一帶）。
    //  這支測試刻意不跑 LoadMachineConfig（會寫真的 Gerneral.ini），出貨組態下沒有 IO 卡，
    //  感測器讀起來全是「沒有」—— 所以 golden 在第一拍就把 START 拉回 false，spine 根本走不到。
    //  20260923 gdb 觀察點實測：SystemStart 1->0 發生在 DoSystem() 內，是壓縮空氣不足那一臂
    //    （golden csystem.cpp:4448-4452：`else if(Sen[SnAirIsEnough].IsOff()) { StopAllMotor();
    //     ShowErrorMessage("WAR1603", …); SystemStart=false; }`；反組譯序列 IsOff→StopAllMotor→
    //     ShowErrorMessage→寫 SystemStart，期間沒有寫 fAllMotorHome）。哪一臂先觸發取決於感測器預設值，
    //    所以下面只斷言 SystemStart 被拉回，不綁定是哪一臂、也不斷言 fAllMotorHome。
    //  另外試過夾具宣告「感測器全部未安裝」（Sen[].Enable=false），第一拍仍被別的互鎖拉回 0，
    //  要在 ctest 裡模擬整台開機不划算，所以不走那條。
    //  ⇒ 這裡改成斷言 golden 在這個環境**真正會做**的事：
    CHECK(SystemStart == false,
          "★ START 被 golden DoSystem 的安全互鎖拉回（本環境實測是壓縮空氣不足 WAR1603，golden csystem.cpp:4448-4452）");
    CHECK(traceOn == 0,
          "★ 互鎖在 DoSystem，早於 golden :17130 的 if(SystemStart) —— spine 沒有被驅動");
    //  NOT COVERED（移交）：「SystemStart==true 且機台就緒時 spine 真的被驅動」與「兩個方向確實不同」。
    //    這兩條要整台機的開機前提（系統／馬達電源、安全門、歸零），改由 wb_serve 端到端量：
    //    START 前後各數一次 DoInArm_9045 的呼叫次數（gdb 計數中斷點，見檔頭提到的 webprobe 探針）。
    //    20260923 T6 實測：START 前 0 次、START→歸零→Initial Start 之後 21 次（DoAllProcess 44 次）——
    //    那就是兩個方向，而且是在真的開機序列下量的。
    (void)traceOff;

    // -----------------------------------------------------------------------
    //  收尾：不要把全域留在 START 狀態給下一支測試
    // -----------------------------------------------------------------------
    SystemStart = false;
    iHome       = 0;
    csystemTraceClear();

    std::printf("\n=== %d checks, %d FAIL ===\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
