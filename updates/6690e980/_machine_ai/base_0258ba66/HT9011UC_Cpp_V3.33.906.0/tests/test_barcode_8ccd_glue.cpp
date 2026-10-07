// tests/test_barcode_8ccd_glue.cpp
// Verification harness for BarCode/BarCode_8CCD_Glue.{h,cpp} -- the 8-CCD
// ClientSocket connection-lifecycle glue (13 real TClientSocket instances +
// SendCCDCommand + TimerBotton8CCDConnectTimer + TimerBottom8CCDInitialTimer).
//
// No external test framework: a tiny check harness prints PASS/FAIL per case
// and a final summary, and returns non-zero on ANY failure (same style as
// tests/test_barcode_bottom2did8ccd.cpp).
//
// SCOPE of this harness (all exercised against the REAL vclcompat Sim-mode
// ClientSocket -- no live CCD hardware, matching every sibling BarCode test):
//   Part 0: SendCCDCommand dispatch-by-index correctness (Bottom family,
//           Shuttle family, OCR text-vs-"\r\n" variant, empty-Msg2 no-op,
//           InitialOK guard, "LF" substring side effect on
//           bChangeSetupFile/OCRChangeFileTimeOut).
//   Part 1: TimerBotton8CCDConnectTimer -- fresh-disconnected-socket connects
//           on the very first tick (Count starts at 90, already past both
//           thresholds); the reconnect-throttle boundary (Count>20 for
//           Bottom_1) is exact (still down at 19 ticks post-disconnect, up
//           at the 20th); the InitialOK / bEnableBarCode+bEnableShtFloatChk
//           early-return guard.
//   Part 2: TimerBottom8CCDInitialTimer's Task (Bottom_1..4) cursor -- full
//           walk from case 1 through 3000 (non-Shuttle-Float-Check path),
//           verifying BOTH the Task value at each step AND the exact
//           SendCCDCommand payload text sent at each step.
//   Part 3: TimerBottom8CCDInitialTimer's Task2 (Bottom_5..8) cursor,
//           continuing in the SAME process (Task is left at 3000 by Part 2,
//           which is what lets Task2's own half start being evaluated at
//           all) -- this time WITH bEnableShtFloatChk=true, so it also
//           walks the Shuttle-Float-Check branch (cases 2100-2350),
//           including BOTH the "GF already matches" shortcut (case 2250's
//           true-equal branch) and the "GF mismatch -> re-send LF" branch
//           (case 2250's else -> 2300 -> 2350).
//
// LIMITATION (stated explicitly, same shape as every sibling BarCode test):
// the golden ClientSocket_Shuttle1_AConnect/Disconnect/Error/Read shared
// event-handler set (which would normally flip bSFCGetVR/bGetSE9[]/bGetGF/
// bGetLF/etc. true from a REAL CCD reply) is NOT part of this unit's scope
// (see BarCode_8CCD_Glue.h banner) -- this harness therefore drives those
// flags directly (white-box), simulating "the CCD just replied", exactly
// the same testing shape the DoBottom2DID_8CCD_Scan test uses for its own
// bGetSE9[] gate.
//
// NOT tested here (documented, not a coverage gap this harness can close):
// golden BarCode.cpp:9264's genuine copy-paste bug (the iBottom_2 reconnect
// success branch resets `Count1=0;` instead of `Count2=0;`) touches a
// function-LOCAL `static` counter with no external linkage -- there is no
// way to observe it from a black-box test. It is preserved verbatim and
// documented with an inline comment at the exact spot in
// BarCode_8CCD_Glue.cpp (see translate report item 6 for the full
// tick-by-tick consequence analysis).
#include "BarCode/BarCode_8CCD_Glue.h"
#include "cprod.h"               // TestIF_File
#include "cmydef.h"              // BAR_CODE_INSTALL, BOTTOM_2DID, BOTTOM_2DID_CCD, InitialOK
#include "MachineType.h"         // ebcUseOCR, ebctUseCCDMode
#include "aHotPlateSubstrate.h"  // InArmSuck

#include <cstdio>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

static void check_b(const char* name, bool got, bool expected)
{
    if (got == expected) { printf("PASS  %-72s got=%d\n", name, (int)got); ++g_pass; }
    else { printf("FAIL  %-72s got=%d exp=%d\n", name, (int)got, (int)expected); ++g_fail; }
}

static void check_i(const char* name, int got, int expected)
{
    if (got == expected) { printf("PASS  %-72s got=%d\n", name, got); ++g_pass; }
    else { printf("FAIL  %-72s got=%d exp=%d\n", name, got, expected); ++g_fail; }
}

static void check_s(const char* name, const std::string& got, const std::string& expected)
{
    if (got == expected) { printf("PASS  %-72s got=\"%s\"\n", name, got.c_str()); ++g_pass; }
    else { printf("FAIL  %-72s got=\"%s\" exp=\"%s\"\n", name, got.c_str(), expected.c_str()); ++g_fail; }
}

static std::string TxOf(Scktcomp::TClientSocket* s)
{
    const std::vector<char>& v = s->Socket->SimTxBuffer();
    return std::string(v.begin(), v.end());
}

int main()
{
    printf("=== BarCode_8CCD_Glue (ClientSocket connection-lifecycle glue) verification ===\n\n");

    // -------------------------------------------------------------------
    // Part 0: SendCCDCommand dispatch-by-index correctness
    // -------------------------------------------------------------------
    printf("-- Part 0: SendCCDCommand --\n");
    {
        InitialOK = false;
        ClientSocket_Bottom_3->Open();
        ClientSocket_Bottom_3->Socket->SimClearTx();
        SendCCDCommand(iBottom_3, "should be a no-op", "NOPE");
        check_s("InitialOK==false -> SendCCDCommand is a total no-op",
                 TxOf(ClientSocket_Bottom_3), "");

        InitialOK = true;
        BOTTOM_2DID_CCD = 1;
        BOTTOM_2DID = 1;
        TestIF_File.bEnableBottom2D = true;
        BAR_CODE_INSTALL = 0;   // not ebcUseOCR

        ClientSocket_Bottom_1->Socket->SimClearTx();
        ClientSocket_Bottom_3->Socket->SimClearTx();
        ClientSocket_Bottom_4->Socket->SimClearTx();
        ClientSocket_Bottom_1->Open();
        ClientSocket_Bottom_4->Open();
        SendCCDCommand(iBottom_3, "dispatch test", "HELLO");
        check_s("Bottom-family dispatch: index==iBottom_3 sends only to Bottom_3",
                 TxOf(ClientSocket_Bottom_3), "HELLO\r\n");
        check_s("Bottom-family dispatch: Bottom_1 untouched by an iBottom_3 call",
                 TxOf(ClientSocket_Bottom_1), "");
        check_s("Bottom-family dispatch: Bottom_4 untouched by an iBottom_3 call",
                 TxOf(ClientSocket_Bottom_4), "");

        ClientSocket_Bottom_3->Socket->SimClearTx();
        SendCCDCommand(iBottom_3, "empty Msg2 is a pure log-only action", "");
        check_s("Msg2==\"\" sends nothing (golden's empty then-branch)",
                 TxOf(ClientSocket_Bottom_3), "");

        // Shuttle-family branch (BOTTOM_2DID_CCD/BOTTOM_2DID/bEnableBottom2D
        // gate false -> golden's `else` branch, dispatched by
        // iBarCode1_1/iBarCode1_2/iBarCode2_1/iBarCode2_2 instead).
        TestIF_File.bEnableBottom2D = false;
        ClientSocket_Shuttle1_A->Socket->SimClearTx();
        ClientSocket_Shuttle1_B->Socket->SimClearTx();
        ClientSocket_Shuttle2_A->Socket->SimClearTx();
        ClientSocket_Shuttle1_A->Open();
        ClientSocket_Shuttle1_B->Open();
        ClientSocket_Shuttle2_A->Open();
        SendCCDCommand(iBarCode2_1, "shuttle dispatch test", "WORLD");
        check_s("Shuttle-family dispatch: index==iBarCode2_1 sends only to Shuttle2_A",
                 TxOf(ClientSocket_Shuttle2_A), "WORLD\r\n");
        check_s("Shuttle-family dispatch: Shuttle1_A untouched by an iBarCode2_1 call",
                 TxOf(ClientSocket_Shuttle1_A), "");
        check_s("Shuttle-family dispatch: Shuttle1_B untouched by an iBarCode2_1 call",
                 TxOf(ClientSocket_Shuttle1_B), "");

        // OCR install variant: plain text, no "\r\n" terminator appended.
        BAR_CODE_INSTALL = ebcUseOCR;
        ClientSocket_Shuttle1_A->Socket->SimClearTx();
        SendCCDCommand(iBarCode1_1, "OCR variant", "OCRMSG");
        check_s("BAR_CODE_INSTALL==ebcUseOCR sends Msg2 verbatim (no \\r\\n)",
                 TxOf(ClientSocket_Shuttle1_A), "OCRMSG");
        BAR_CODE_INSTALL = 0;   // restore for the rest of this test

        // "LF" substring side effect (recipe-change timer arm).
        bChangeSetupFile = false;
        ClientSocket_Shuttle1_A->Socket->SimClearTx();
        SendCCDCommand(iBarCode1_1, "recipe change", "LF,0,SOMEFILE,0");
        check_b("Msg2 containing \"LF\" sets bChangeSetupFile=true",
                 bChangeSetupFile, true);
        check_b("OCRChangeFileTimeOut freshly armed (5s) is NOT yet Off()",
                 OCRChangeFileTimeOut.Off(), false);

        ClientSocket_Shuttle1_A->Close();
        ClientSocket_Shuttle1_B->Close();
        ClientSocket_Shuttle2_A->Close();
        ClientSocket_Bottom_1->Close();
        ClientSocket_Bottom_3->Close();
        ClientSocket_Bottom_4->Close();
    }

    // -------------------------------------------------------------------
    // Part 1: TimerBotton8CCDConnectTimer
    // -------------------------------------------------------------------
    printf("\n-- Part 1: TimerBotton8CCDConnectTimer --\n");
    {
        InitialOK = false;
        TestIF_File.bEnableBarCode = true;
        TimerBotton8CCDConnectTimer();
        check_b("InitialOK==false -> early return, Bottom_1 stays disconnected",
                 (bool)ClientSocket_Bottom_1->Active, false);

        InitialOK = true;
        TestIF_File.bEnableBarCode = false;
        TestIF_File.bEnableShtFloatChk = false;
        bEnableCCDChannelConnect[iBottom_1] = true;
        TimerBotton8CCDConnectTimer();
        check_b("bEnableBarCode==false && bEnableShtFloatChk==false -> early return",
                 (bool)ClientSocket_Bottom_1->Active, false);

        TestIF_File.bEnableBarCode = true;
        TimerBotton8CCDConnectTimer();
        check_b("Fresh disconnected + enabled socket connects on the FIRST tick "
                "(golden's static Count starts at 90, already > both thresholds)",
                (bool)ClientSocket_Bottom_1->Active, true);
        check_s("Bottom_1 Address seeded to the golden literal",
                 std::string(ClientSocket_Bottom_1->Address.c_str()), "172.16.8.200");
        check_i("Bottom_1 Port seeded to the golden literal (5011)",
                 ClientSocket_Bottom_1->Port, 5011);

        // Force a disconnect (Address mismatch) so the throttle-wait path
        // (Count>20) is genuinely exercised from a freshly-reset Count.
        ClientSocket_Bottom_1->Address = "0.0.0.0";
        TimerBotton8CCDConnectTimer();   // golden's disconnect branch: Close() + Count1=0, then Count1++ -> 1
        check_b("Address mismatch forces a Close() (disconnect branch)",
                 (bool)ClientSocket_Bottom_1->Active, false);

        for (int i = 0; i < 19; ++i)
            TimerBotton8CCDConnectTimer();
        check_b("19 more ticks after the forced disconnect: Count1 only reached 20 "
                "(threshold is `Count1>20`, strictly greater) -> still disconnected",
                (bool)ClientSocket_Bottom_1->Active, false);

        TimerBotton8CCDConnectTimer();   // the 20th tick: Count1 21>20 -> reconnect
        check_b("The 20th subsequent tick pushes Count1 to 21 (>20) -> reconnects",
                (bool)ClientSocket_Bottom_1->Active, true);
        check_s("Reconnect restores the golden literal Address",
                 std::string(ClientSocket_Bottom_1->Address.c_str()), "172.16.8.200");

        // Bottom_4 (a different index/port, 30-tick threshold) also connects
        // from a fresh disconnected state on the very first tick (same
        // "Count starts at 90" mechanism) -- lighter check, no boundary.
        bEnableCCDChannelConnect[iBottom_4] = true;
        TimerBotton8CCDConnectTimer();
        check_b("Bottom_4 (30-tick-threshold slot) also connects on the first tick",
                (bool)ClientSocket_Bottom_4->Active, true);
        check_i("Bottom_4 Port seeded to its own golden literal (5014)",
                 ClientSocket_Bottom_4->Port, 5014);

        ClientSocket_Bottom_1->Close();
        ClientSocket_Bottom_4->Close();
        bEnableCCDChannelConnect[iBottom_1] = false;
        bEnableCCDChannelConnect[iBottom_4] = false;
    }

    // -------------------------------------------------------------------
    // Part 2: TimerBottom8CCDInitialTimer -- Task (Bottom_1..4)
    // -------------------------------------------------------------------
    printf("\n-- Part 2: TimerBottom8CCDInitialTimer -- Task (Bottom_1..4) --\n");
    {
        InitialOK = true;
        TestIF_File.bEnableBarCode = true;
        TestIF_File.bEnableShtFloatChk = false;    // exercise the non-SFC path here
        TestIF_File.bEnableBottom2D = true;        // matches this test's BOTTOM_2DID_CCD/BOTTOM_2DID==1 SendCCDCommand routing
        TestIF_File.bBarCodeMultiRecipe = false;
        BOTTOM_2DID_CCD = 1;
        BOTTOM_2DID = 1;
        BAR_CODE_INSTALL = 0;
        InArmSuck.iShtCol = 4;
        TestIF_File.bSFCUse2Photo = false;

        ClientSocket_Bottom_1->Address = "172.16.8.200"; ClientSocket_Bottom_1->Port = 5011; ClientSocket_Bottom_1->Open();
        ClientSocket_Bottom_2->Address = "172.16.8.200"; ClientSocket_Bottom_2->Port = 5012; ClientSocket_Bottom_2->Open();
        ClientSocket_Bottom_3->Address = "172.16.8.200"; ClientSocket_Bottom_3->Port = 5013; ClientSocket_Bottom_3->Open();
        ClientSocket_Bottom_4->Address = "172.16.8.200"; ClientSocket_Bottom_4->Port = 5014; ClientSocket_Bottom_4->Open();
        ClientSocket_Bottom_1->Socket->SimClearTx();

        // Simulates Change2DSetupFile() (golden BarCode.cpp:5531, out of this
        // unit's scope) having already kicked the cursor to 1 once.
        iBottomConntectionOkTask1 = 1;

        TimerBottom8CCDInitialTimer();   // call 1: case 1 -> case 100
        check_i("call1: Task 1 -> 100 (fallthrough into the just-armed case 100 check)",
                 iBottomConntectionOkTask1, 100);
        check_s("call1: case 1 sent \"VR\" to Bottom_1",
                 TxOf(ClientSocket_Bottom_1), "VR\r\n");
        check_i("call1: SetSFCCheckStepCount() seeded iSFCTotalMoveStep from InArmSuck.iShtCol",
                 iSFCTotalMoveStep, 4);
        ClientSocket_Bottom_1->Socket->SimClearTx();

        bSFCGetVR = true;   // simulate the CCD's "VR" reply having arrived
        TimerBottom8CCDInitialTimer();   // call 2: case 100 -> Task=1000
        check_i("call2: Task 100 -> 1000 (bSFCGetVR==true, bEnableBarCode==true)",
                 iBottomConntectionOkTask1, 1000);

        TimerBottom8CCDInitialTimer();   // call 3: case 1000 -> case 1050
        check_i("call3: Task 1000 -> 1050 (fallthrough)", iBottomConntectionOkTask1, 1050);
        check_s("call3: case 1000 sent \"SE0,9,4\" (dVisionVer<2 format) to Bottom_1",
                 TxOf(ClientSocket_Bottom_1), "SE0,9,4\r\n");
        check_b("call3: bGetSE9[iBottom_1] freshly reset to false", bGetSE9[iBottom_1], false);
        ClientSocket_Bottom_1->Socket->SimClearTx();

        bGetSE9[iBottom_1] = true; bGetSE9[iBottom_2] = true;
        bGetSE9[iBottom_3] = true; bGetSE9[iBottom_4] = true;
        TimerBottom8CCDInitialTimer();   // call 4: case 1050 -> Task=1100
        check_i("call4: Task 1050 -> 1100 (all 4 bGetSE9[] true)", iBottomConntectionOkTask1, 1100);

        TimerBottom8CCDInitialTimer();   // call 5: case 1100 -> case 1150
        check_i("call5: Task 1100 -> 1150 (fallthrough)", iBottomConntectionOkTask1, 1150);
        check_s("call5: case 1100 sent bare \"GF\" (dVisionVer<2) to Bottom_1",
                 TxOf(ClientSocket_Bottom_1), "GF\r\n");
        ClientSocket_Bottom_1->Socket->SimClearTx();

        bGetGF = true;
        TimerBottom8CCDInitialTimer();   // call 6: case 1150 -> Task=1200
        check_i("call6: Task 1150 -> 1200 (bGetGF==true)", iBottomConntectionOkTask1, 1200);

        bChangeSetupFile = false;
        TimerBottom8CCDInitialTimer();   // call 7: case 1200 -> case 1250
        check_i("call7: Task 1200 -> 1250 (fallthrough)", iBottomConntectionOkTask1, 1250);
        check_s("call7: case 1200 sent \"LF,1\" (bEnableBottom2D branch, empty recipe name) to Bottom_1",
                 TxOf(ClientSocket_Bottom_1), "LF,1\r\n");
        check_b("call7: the \"LF,1\" payload's \"LF\" substring set bChangeSetupFile==true",
                 bChangeSetupFile, true);
        ClientSocket_Bottom_1->Socket->SimClearTx();

        bGetLF = true;
        TimerBottom8CCDInitialTimer();   // call 8: case 1250 -> Task=2000 (real break, no further fallthrough)
        check_i("call8: Task 1250 -> 2000 (bGetLF==true; case 1250 has a real break)",
                 iBottomConntectionOkTask1, 2000);

        TimerBottom8CCDInitialTimer();   // call 9: case 2000 -> Task=3000 (bEnableShtFloatChk==false)
        check_i("call9: Task 2000 -> 3000 (bEnableShtFloatChk==false skips the SFC branch)",
                 iBottomConntectionOkTask1, 3000);
    }

    // -------------------------------------------------------------------
    // Part 3: TimerBottom8CCDInitialTimer -- Task2 (Bottom_5..8), WITH the
    // Shuttle-Float-Check branch this time (bEnableShtFloatChk=true).
    // Relies on Part 2 having left Task==3000 (the outer `if(Task!=3000)
    // return;` guard is what actually lets this half run at all).
    // -------------------------------------------------------------------
    printf("\n-- Part 3: TimerBottom8CCDInitialTimer -- Task2 (Bottom_5..8), SFC branch --\n");
    {
        check_i("precondition: Task is still 3000 (Part 2's end state)",
                 iBottomConntectionOkTask1, 3000);

        TestIF_File.bEnableShtFloatChk = true;

        ClientSocket_Bottom_5->Address = "172.16.8.200"; ClientSocket_Bottom_5->Port = 5015; ClientSocket_Bottom_5->Open();
        ClientSocket_Bottom_6->Address = "172.16.8.200"; ClientSocket_Bottom_6->Port = 5016; ClientSocket_Bottom_6->Open();
        ClientSocket_Bottom_7->Address = "172.16.8.200"; ClientSocket_Bottom_7->Port = 5017; ClientSocket_Bottom_7->Open();
        ClientSocket_Bottom_8->Address = "172.16.8.200"; ClientSocket_Bottom_8->Port = 5018; ClientSocket_Bottom_8->Open();
        ClientSocket_Bottom_5->Socket->SimClearTx();

        iBottomConntectionOkTask2 = 1;   // simulates Change2DSetupFile()'s Task2 kick

        TimerBottom8CCDInitialTimer();   // case 1 -> case 100
        check_i("Task2 1 -> 100", iBottomConntectionOkTask2, 100);
        check_s("case 1 sent \"VR\" to Bottom_5", TxOf(ClientSocket_Bottom_5), "VR\r\n");
        ClientSocket_Bottom_5->Socket->SimClearTx();

        bSFCGetVR = true;
        TimerBottom8CCDInitialTimer();   // case 100 -> Task2=1000
        check_i("Task2 100 -> 1000", iBottomConntectionOkTask2, 1000);

        TimerBottom8CCDInitialTimer();   // case 1000 -> case 1050
        check_i("Task2 1000 -> 1050", iBottomConntectionOkTask2, 1050);
        check_s("case 1000 sent \"SE0,9,4\" to Bottom_5", TxOf(ClientSocket_Bottom_5), "SE0,9,4\r\n");
        ClientSocket_Bottom_5->Socket->SimClearTx();

        bGetSE9[iBottom_5] = true; bGetSE9[iBottom_6] = true;
        bGetSE9[iBottom_7] = true; bGetSE9[iBottom_8] = true;
        TimerBottom8CCDInitialTimer();   // case 1050 -> Task2=1100
        check_i("Task2 1050 -> 1100", iBottomConntectionOkTask2, 1100);

        TimerBottom8CCDInitialTimer();   // case 1100 -> case 1150 (bGetGF self-resets to false here)
        check_i("Task2 1100 -> 1150", iBottomConntectionOkTask2, 1150);
        check_s("case 1100 sent bare \"GF\" to Bottom_5 (bGetGF freshly reset by THIS case, "
                "not left over from Part 2)", TxOf(ClientSocket_Bottom_5), "GF\r\n");
        ClientSocket_Bottom_5->Socket->SimClearTx();

        bGetGF = true;
        TimerBottom8CCDInitialTimer();   // case 1150 -> Task2=1200
        check_i("Task2 1150 -> 1200", iBottomConntectionOkTask2, 1200);

        TimerBottom8CCDInitialTimer();   // case 1200 -> case 1250 (bGetLF self-resets here too)
        check_i("Task2 1200 -> 1250", iBottomConntectionOkTask2, 1250);
        check_s("case 1200 sent \"LF,1\" to Bottom_5", TxOf(ClientSocket_Bottom_5), "LF,1\r\n");
        ClientSocket_Bottom_5->Socket->SimClearTx();

        bGetLF = true;
        TimerBottom8CCDInitialTimer();   // case 1250 -> Task2=2000 (real break)
        check_i("Task2 1250 -> 2000", iBottomConntectionOkTask2, 2000);

        TimerBottom8CCDInitialTimer();   // case 2000 -> Task2=2100 (bEnableShtFloatChk==true this time)
        check_i("Task2 2000 -> 2100 (bEnableShtFloatChk==true takes the SFC branch)",
                 iBottomConntectionOkTask2, 2100);

        TimerBottom8CCDInitialTimer();   // case 2100 -> case 2150
        check_i("Task2 2100 -> 2150", iBottomConntectionOkTask2, 2150);
        check_s("case 2100 sent \"E9,1,4\" (iSFCTotalMoveStep) to Bottom_5",
                 TxOf(ClientSocket_Bottom_5), "E9,1,4\r\n");
        check_b("case 2100 freshly reset bSFCGetSE9[iBottom_5] to false", bSFCGetSE9[iBottom_5], false);
        ClientSocket_Bottom_5->Socket->SimClearTx();

        bSFCGetSE9[iBottom_5] = true; bSFCGetSE9[iBottom_6] = true;
        bSFCGetSE9[iBottom_7] = true; bSFCGetSE9[iBottom_8] = true;
        TimerBottom8CCDInitialTimer();   // case 2150 -> Task2=2200
        check_i("Task2 2150 -> 2200", iBottomConntectionOkTask2, 2200);

        TimerBottom8CCDInitialTimer();   // case 2200 -> case 2250
        check_i("Task2 2200 -> 2250", iBottomConntectionOkTask2, 2250);
        check_s("case 2200 sent \"GF,1\" to Bottom_5", TxOf(ClientSocket_Bottom_5), "GF,1\r\n");
        ClientSocket_Bottom_5->Socket->SimClearTx();

        // Branch A: GF reply ALREADY matches the (empty) recipe name ->
        // shortcut straight to 3000, skipping the re-Change-File round trip.
        sSFCCurrentGF = "";
        bSFCGetGF = true;
        TimerBottom8CCDInitialTimer();   // case 2250 (match) -> Task2=3000
        check_i("case 2250 \"GF already matches\" branch -> Task2=3000 directly",
                 iBottomConntectionOkTask2, 3000);
        check_b("case 2250 match branch also sets bSFCGetLF=true (golden verbatim)",
                 bSFCGetLF, true);
        check_s("case 2250 match branch sends nothing further to Bottom_5",
                 TxOf(ClientSocket_Bottom_5), "");

        // Branch B (bonus coverage, white-box): force case 2250 back to a
        // GF-MISMATCH so the case 2300 -> 2350 re-send-LF path is exercised.
        iBottomConntectionOkTask2 = 2200;
        TimerBottom8CCDInitialTimer();   // case 2200 -> case 2250 again
        ClientSocket_Bottom_5->Socket->SimClearTx();
        sSFCCurrentGF = "SOME-OTHER-RECIPE";
        bSFCGetGF = true;
        TimerBottom8CCDInitialTimer();   // case 2250 (mismatch) -> Task2=2300
        check_i("case 2250 \"GF mismatch\" branch -> Task2=2300 (re-send LF)",
                 iBottomConntectionOkTask2, 2300);

        bSFCGetLF = false;
        TimerBottom8CCDInitialTimer();   // case 2300 -> case 2350
        check_i("Task2 2300 -> 2350", iBottomConntectionOkTask2, 2350);
        check_s("case 2300 sent \"LF,1,\" (SFC Change File, empty recipe name) to Bottom_5",
                 TxOf(ClientSocket_Bottom_5), "LF,1,\r\n");

        bSFCGetLF = true;
        TimerBottom8CCDInitialTimer();   // case 2350 -> Task2=3000
        check_i("Task2 2350 -> 3000 (bSFCGetLF==true)", iBottomConntectionOkTask2, 3000);

        ClientSocket_Bottom_5->Close(); ClientSocket_Bottom_6->Close();
        ClientSocket_Bottom_7->Close(); ClientSocket_Bottom_8->Close();
    }

    ClientSocket_Bottom_1->Close(); ClientSocket_Bottom_2->Close();
    ClientSocket_Bottom_3->Close(); ClientSocket_Bottom_4->Close();

    printf("\n=== SUMMARY: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
