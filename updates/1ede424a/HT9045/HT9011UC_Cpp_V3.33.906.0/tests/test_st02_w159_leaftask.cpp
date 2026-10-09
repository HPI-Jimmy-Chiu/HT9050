// =============================================================================
//  test_st02_w159_leaftask.cpp -- W-159 E042-LEAFTASK: the residual-IC leaf stop in TfContact::DoTestContactFunction must stick.
//
//  AI(W906-W159) 20261009 (St02-E).  Suite name (add_test): St02_W159LeafTask.
//  forms/fContact_ContactSM.cpp:100-107 E042Leaf(): message + SystemStart=false + CarlibrationTask=1, returns false.  Golden 913
//  cContact.cpp:12884 / :12918 / :13283 `if(<GIGAS|JCET> && DoSocketSensorCheckRemainIC()==true) break;` (true = WAR0322, stay).
//  The port's leaf lines (:1234 case 320, :1268 case 335, :1633 case 799) compared the leaf's false with ==true and fell through:
//  EPSwitchOnOff + Task=330/340/800 in the same tick, so the next START resumed past the residual-IC check.  Now ==false: break.
//    [1] CC_GIGAS at case 320  [2] CC_JCET at case 335  [3] CC_JCET at case 799: one call -> CarlibrationTask stays 1, SystemStart
//        false, exactly one message naming DoSocketSensorCheckRemainIC.
//    [4] control, customer code 0 (no residual-IC check in golden): no message, Task advances as golden (320->330, 335->340, 799->800).
//  Containment first (the contact run logs through the machine log roots): W906TestInsideCtestRoots refuses to run (exit 2) unless
//  every log root and redirect variable points at ctest's own machine_log_scratch / machine_config_scratch under the build tree --
//  run by hand against the real D:\HT9045 / D:\HT9045_Log paths it stops before any contact code runs.
// =============================================================================
#include "forms/fContact.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "canary_support.h"          // W906_ShowMyMessage_Count / W906_ShowMyMessage_LastS1
#include "st02_test_containment.h"
#include <cstdio>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

struct Run { int task; bool start; int msgs; std::string last; };

static Run Tick(int customer, int task)
{
    CUSTOMER_CODE = customer;
    TestIF.iShuttleMode = 0;                 // both shuttles: 320 -> 330, 335 -> 340 in golden
    fContactForm->CarlibrationTask = task;
    SystemStart = true;
    SoftStop = false;
    const int c0 = W906_ShowMyMessage_Count;
    fContactForm->DoTestContactFunction();
    Run r;
    r.task = fContactForm->CarlibrationTask;
    r.start = SystemStart;
    r.msgs = W906_ShowMyMessage_Count - c0;
    r.last = W906_ShowMyMessage_LastS1.c_str();
    return r;
}

static std::string Desc(const Run& r)
{
    return "Task " + std::to_string(r.task) + ", SystemStart " + (r.start ? "true" : "false") + ", messages " + std::to_string(r.msgs);
}

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W159LeafTask -- DoTestContactFunction residual-IC leaf stop (golden 913 cContact.cpp:12884/:12918/:13283)\n");
    if (!W906TestInsideCtestRoots("St02_W159LeafTask"))
        return 2;
    if (fContactForm == 0) { std::printf("  ABORT: no fContactForm\n"); return 2; }
    const int savedCustomer = CUSTOMER_CODE;
    const int savedShuttleMode = TestIF.iShuttleMode;

    const struct { const char* tag; int customer; int task; } L[] = {
        { "[1] CC_GIGAS at case 320", CC_GIGAS, 320 },
        { "[2] CC_JCET at case 335",  CC_JCET,  335 },
        { "[3] CC_JCET at case 799",  CC_JCET,  799 },
    };
    for (size_t i = 0; i < sizeof(L) / sizeof(L[0]); ++i)
    {
        std::printf("%s\n", L[i].tag);
        const Run r = Tick(L[i].customer, L[i].task);
        Check(r.task == 1 && !r.start, std::string(L[i].tag) + ": the leaf's stop sticks -- CarlibrationTask 1, run stopped (" + Desc(r) + ")");
        Check(r.msgs == 1 && r.last.find("DoSocketSensorCheckRemainIC") != std::string::npos,
              std::string(L[i].tag) + ": exactly one message, naming DoSocketSensorCheckRemainIC (" + r.last + ")");
    }

    std::printf("[4] control: customer code 0 (golden has no residual-IC check there)\n");
    const struct { int task; int next; } C[] = { { 320, 330 }, { 335, 340 }, { 799, 800 } };
    for (size_t i = 0; i < sizeof(C) / sizeof(C[0]); ++i)
    {
        const Run r = Tick(0, C[i].task);
        Check(r.task == C[i].next && r.msgs == 0,
              "[4] case " + std::to_string(C[i].task) + " -> " + std::to_string(C[i].next) + ", no leaf (" + Desc(r) + ")");
    }

    CUSTOMER_CODE = savedCustomer;
    TestIF.iShuttleMode = savedShuttleMode;
    SystemStart = false;
    fContactForm->CarlibrationTask = 1;
    std::printf("\nSt02_W159LeafTask: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
