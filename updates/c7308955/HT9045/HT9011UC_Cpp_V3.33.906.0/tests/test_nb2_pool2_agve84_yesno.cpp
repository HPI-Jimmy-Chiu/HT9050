// =============================================================================
//  tests/test_nb2_pool2_agve84_yesno.cpp -- AI(W906-POOL2-NB2) 20261008 (NB2-1, W-167, README R263)
//
//  POOL-2: Automation/AGV_E84.cpp :564 (DoE84Loader case 2000) and :931 (DoE84Unloader case 2000) opened -- golden 0618
//  Automation/AGV.cpp:565 / :910 `ret=ShowMyMessageBox_YES_NO(asStr, "是否要修改Port狀態?");` instead of the old `ret=2` stand-in.
//  Review: MainNB-GPT RD5軟體_IF0_AGVE84候選複核_20261007_225648.md (v906/mainnb-gpt-cli-handoff), "minimum verification".
//
//  HOW THE SM REACHES case 2000 (no hand-set iCount -- it is a function-local static):
//    case 500 with BUSY on reads CS0 / CS1 and sets iCount (0 = CS0 only, 1 = CS1 only, 2 = both) -> Task 600 (golden :409-433);
//    the test then puts the cursor on 900 (cases 600-800 do not touch iCount once it is not 100) with READY off and
//    VALID / COMPT / CS0 / CS1 off -> iCount != iPlaceWhichBuffer[x] -> Task 2000; the next call asks.
//  The dialog answer is W906_ShowMyMessageBoxYesNo_Hook (canary_support.h:215): 1 Yes, 2 No, 3 box already open; 0 = no host
//  (the stand-in then returns its SimReturn, 0). Only ret==1 changes the buffer (golden :566 / :911).
//
//  ORACLES (golden 0618 Automation/AGV.cpp:549-572 / :908-915 / :893-907 / :916-919):
//   [L] Loader, iCount 0 (CS0), start buffer 1:
//       question asked once, S1 "AGV Place To Loader,Start Buffer is Empty", S2 "是否要修改Port狀態?";
//       Yes -> iPlaceWhichBuffer[0]=0 and flags[0] cleared in case 2000 itself; 2100 then clears flags[0]; flags[1] untouched;
//       No / 3 / 0 -> buffer stays 1, flags[0] untouched, 2100 clears flags[1]; all -> 2100 -> 1000 -> 1.
//   [LM] Loader, iCount == start buffer -> case 900 goes straight to 1000, no question.
//   [U] Unloader, iCount 2 (CS0+CS1), start buffer 0: S1 "AGV Pick Form Auto 3,Start Buffer is Auto 1";
//       Yes -> iPlaceWhichBuffer[1]=2 -> case 1000 clears MOT[iMMAuto[2]] only; No / 3 / 0 -> clears MOT[iMMAuto[0]] only.
//  REVERSE (done when this landed): `#if 1` back to `#if 0` (the old ret=2) -> every Yes oracle and every "asked once" is red.
//  CONTAINMENT: the E84 log root is redirected (W906_E84DATA_ROOT, same as tests/test_agv_e84.cpp). No real IO, no motor moves:
//  sensors / switches use the Enable + sentinel ISABase technique of tests/test_agv_e84.cpp; ClearTray is a software reset.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"                   // Sn*/Sw* indices, bE84Loader(Action)flag / bE84Unloader(Action)flag, iMMAuto[]
#include "cprod.h"                    // TestIF_File (E84 timeouts)
#include "mysensor.h"                 // Sen[]
#include "myswitch.h"                 // SW[]
#include "mycylin.h"
#include "Motor/mymotor.h"            // MOT[] (TTrayMotor: fHasTray / sTrayID / ClearTray)
#include "Automation/AGV_E84.h"       // DoE84Loader / DoE84Unloader / iE84LoadTask / iE84UnloadTask / iPlaceWhichBuffer
#include "canary_support.h"           // ShowMyMessageBox_YES_NO capture + hook
#include "w906_test_tmpname.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define NB2_TEST_PUTENV _putenv
#else
#define NB2_TEST_PUTENV putenv
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { if (cond) { ++g_pass; } else { ++g_fail; std::printf("  FAIL [test_nb2_pool2_agve84_yesno.cpp:%d] %s\n", __LINE__, msg); } } while (0)

static void RedirectE84LogRoot()                                   // same as tests/test_agv_e84.cpp (CRT putenv: getenv must see it)
{
    static char buf[512];
    const char* tmp = getenv("TEMP");
    if (tmp == 0) tmp = getenv("TMP");
    if (tmp == 0) tmp = ".";
    std::snprintf(buf, sizeof(buf), "W906_E84DATA_ROOT=%s\\ht9045_e84_nb2_scratch%s", tmp, W906_TestTmpTag().c_str());
    NB2_TEST_PUTENV(buf);
}

static const int kSentinelISABase = -1;                            // matches no IO routing enum -> the backend is never read
static void SetSensor(TMySensor& s, bool on) { s.Enable = true; s.ISABase = kSentinelISABase; s.Type = on ? 0 : 1; }
static void SwitchOff(TMySwitch& sw) { sw.Enable = true; sw.ISABase = kSentinelISABase; sw.Type = 1; sw.Off(); }

static int g_answer = 0;
static int AnswerHook(const char*, const char*, const char*) { return g_answer; }

static const char* AnswerName(int a) { return a == 1 ? "Yes" : a == 2 ? "No" : a == 3 ? "box already open" : "no host (0)"; }

// ---------------------------------------------------------------- Loader ----------------------------------------------------
static void LoaderCase(bool cs0, bool cs1, int startBuf, int answer, bool expectAsk)
{
    const int iCount = cs0 && cs1 ? 2 : cs0 ? 0 : 1;
    char tag[160];
    std::snprintf(tag, sizeof(tag), "[L] Loader iCount %d, start %d, answer %s", iCount, startBuf, AnswerName(answer));
    std::printf("%s\n", tag);
    W906_ShowMyMessageBoxYesNo_Reset();
    g_answer = answer;
    for (int i = 0; i < 3; i++) { bE84LoaderActionflag[i] = true; bE84Loaderflag[i] = true; }
    iPlaceWhichBuffer[0] = startBuf;

    // case 500: BUSY on, CS0/CS1 -> iCount, Task 600
    InitialE84LoadTask();
    iE84LoadTask = 500;
    SetSensor(Sen[SnE84_1_BUSY], true);
    SetSensor(Sen[SnE84_1_CS0], cs0);
    SetSensor(Sen[SnE84_1_CS1], cs1);
    DoE84Loader();
    CHECK(iE84LoadTask == 600, "case 500 read CS0/CS1 and moved to 600");

    // case 900: READY off, VALID/COMPT/CS0/CS1 off
    iE84LoadTask = 900;
    SwitchOff(SW[SwE84_1_READY]);
    SetSensor(Sen[SnE84_1_VALID], false);
    SetSensor(Sen[SnE84_1_COMPT], false);
    SetSensor(Sen[SnE84_1_CS0], false);
    SetSensor(Sen[SnE84_1_CS1], false);
    DoE84Loader();
    if (!expectAsk)
    {
        CHECK(iE84LoadTask == 1000, "[LM] iCount == start buffer -> case 900 finishes to 1000");
        CHECK(W906_ShowMyMessageBoxYesNo_Count == 0, "[LM] no question when the buffer already matches");
        CHECK(bE84LoaderActionflag[startBuf] == false && bE84Loaderflag[startBuf] == false, "[LM] case 900 cleared the start buffer's flags");
        return;
    }
    CHECK(iE84LoadTask == 2000, "case 900 found iCount != buffer -> 2000");

    DoE84Loader();                                                  // case 2000
    CHECK(W906_ShowMyMessageBoxYesNo_Count == 1, "case 2000 asked the operator once (golden AGV.cpp:565)");
    char want[96];
    const char* name[3] = { "Loader", "Empty", "" };                // golden :552-561: the repeated ==1 test leaves "Color" unreachable
    std::snprintf(want, sizeof(want), "AGV Place To %s,Start Buffer is %s", name[iCount], name[startBuf]);
    CHECK(std::strcmp(W906_ShowMyMessageBoxYesNo_LastS1.c_str(), want) == 0, "S1 = golden AGV Place To ... text");
    CHECK(std::strcmp(W906_ShowMyMessageBoxYesNo_LastS2.c_str(), "是否要修改Port狀態?") == 0, "S2 = golden question");
    CHECK(iE84LoadTask == 2100, "case 2000 -> 2100 whatever the answer");
    const int buf = answer == 1 ? iCount : startBuf;
    CHECK(iPlaceWhichBuffer[0] == buf, answer == 1 ? "Yes -> iPlaceWhichBuffer[0] = iCount" : "not Yes -> iPlaceWhichBuffer[0] unchanged");
    if (answer == 1)
        CHECK(bE84LoaderActionflag[iCount] == false && bE84Loaderflag[iCount] == false, "Yes -> case 2000 clears the new buffer's flags");
    else
        CHECK(bE84LoaderActionflag[iCount] == true && bE84Loaderflag[iCount] == true, "not Yes -> iCount's flags untouched in case 2000");

    DoE84Loader();                                                  // case 2100
    CHECK(iE84LoadTask == 1000, "2100 -> 1000");
    CHECK(bE84LoaderActionflag[buf] == false && bE84Loaderflag[buf] == false, "2100 clears the chosen buffer's flags");
    const int other = answer == 1 ? startBuf : iCount;
    CHECK(bE84LoaderActionflag[other] == true && bE84Loaderflag[other] == true, "the buffer not chosen keeps its flags");
    DoE84Loader();                                                  // case 1000
    CHECK(iE84LoadTask == 1, "1000 -> 1");
}

// ---------------------------------------------------------------- Unloader --------------------------------------------------
static void SeedTrays()
{
    for (int k = 0; k < 3; k++)
    {
        MOT[iMMAuto[k]].fHasTray = true;
        char id[16]; std::snprintf(id, sizeof(id), "T%d", k);
        MOT[iMMAuto[k]].sTrayID = id;
    }
}

static void UnloaderCase(bool cs0, bool cs1, int startBuf, int answer)
{
    const int iCount = cs0 && cs1 ? 2 : cs0 ? 0 : 1;
    std::printf("[U] Unloader iCount %d, start %d, answer %s\n", iCount, startBuf, AnswerName(answer));
    W906_ShowMyMessageBoxYesNo_Reset();
    g_answer = answer;
    for (int i = 0; i < 3; i++) { bE84UnloaderActionflag[i] = true; bE84Unloaderflag[i] = true; }
    iPlaceWhichBuffer[1] = startBuf;
    SeedTrays();

    InitialE84UnLoaderTask();
    iE84UnloadTask = 500;
    SetSensor(Sen[SnE84_2_BUSY], true);
    SetSensor(Sen[SnE84_2_CS0], cs0);
    SetSensor(Sen[SnE84_2_CS1], cs1);
    DoE84Unloader();
    CHECK(iE84UnloadTask == 600, "[U] case 500 read CS0/CS1 and moved to 600");

    iE84UnloadTask = 900;
    SwitchOff(SW[SwE84_2_READY]);
    SetSensor(Sen[SnE84_2_VALID], false);
    SetSensor(Sen[SnE84_2_COMPT], false);
    SetSensor(Sen[SnE84_2_CS0], false);
    SetSensor(Sen[SnE84_2_CS1], false);
    DoE84Unloader();
    CHECK(iE84UnloadTask == 2000, "[U] case 900 found iCount != buffer -> 2000");

    DoE84Unloader();                                                // case 2000
    CHECK(W906_ShowMyMessageBoxYesNo_Count == 1, "[U] case 2000 asked the operator once (golden AGV.cpp:910)");
    char want[96];
    std::snprintf(want, sizeof(want), "AGV Pick Form Auto %d,Start Buffer is Auto %d", iCount + 1, startBuf + 1);
    CHECK(std::strcmp(W906_ShowMyMessageBoxYesNo_LastS1.c_str(), want) == 0, "[U] S1 = golden AGV Pick Form Auto ... text");
    const int buf = answer == 1 ? iCount : startBuf;
    CHECK(iPlaceWhichBuffer[1] == buf, answer == 1 ? "[U] Yes -> iPlaceWhichBuffer[1] = iCount" : "[U] not Yes -> iPlaceWhichBuffer[1] unchanged");
    CHECK(iE84UnloadTask == 2100, "[U] 2000 -> 2100");
    DoE84Unloader();                                                // 2100
    CHECK(iE84UnloadTask == 1000, "[U] 2100 -> 1000");
    DoE84Unloader();                                                // 1000: flags + ClearTray of the chosen Auto
    CHECK(iE84UnloadTask == 1, "[U] 1000 -> 1");
    CHECK(bE84UnloaderActionflag[buf] == false && bE84Unloaderflag[buf] == false, "[U] case 1000 clears the chosen buffer's flags");
    for (int k = 0; k < 3; k++)
    {
        char msg[120];
        if (k == buf)
        {
            std::snprintf(msg, sizeof(msg), "[U] Auto %d (chosen) cleared: fHasTray false, sTrayID empty", k + 1);
            CHECK(MOT[iMMAuto[k]].fHasTray == false && MOT[iMMAuto[k]].sTrayID == "", msg);
        }
        else
        {
            std::snprintf(msg, sizeof(msg), "[U] Auto %d (not chosen) keeps its tray", k + 1);
            CHECK(MOT[iMMAuto[k]].fHasTray == true && MOT[iMMAuto[k]].sTrayID != "", msg);
        }
    }
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    RedirectE84LogRoot();
    {
        const char* got = getenv("W906_E84DATA_ROOT");
        std::printf("E84 log root override: %s\n", got ? got : "(NOT SET)");
        CHECK(got != 0 && std::strstr(got, "HT9045_Log") == 0, "E84 data-log root redirected away from the live machine tree");
        if (got == 0 || std::strstr(got, "HT9045_Log") != 0) { std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail); return 2; }
    }
    std::printf("=== test_nb2_pool2_agve84_yesno: AGV_E84 case 2000 asks (golden AGV.cpp:565 / :910) ===\n");
    W906_ShowMyMessageBoxYesNo_Hook = &AnswerHook;

    LoaderCase(true, false, 1, 1, true);       // Yes
    LoaderCase(true, false, 1, 2, true);       // No
    LoaderCase(true, false, 1, 3, true);       // box already open
    LoaderCase(true, false, 1, 0, true);       // no host
    LoaderCase(false, true, 0, 1, true);       // iCount 1 (CS1), start 0, Yes
    LoaderCase(true, false, 0, 1, false);      // [LM] match -> no question

    UnloaderCase(true, true, 0, 1);            // iCount 2, start 0, Yes  -> Auto 3 cleared
    UnloaderCase(true, true, 0, 2);            // No                     -> Auto 1 cleared
    UnloaderCase(true, true, 0, 3);            // box already open
    UnloaderCase(true, true, 0, 0);            // no host
    UnloaderCase(false, true, 2, 1);           // iCount 1, start 2, Yes  -> Auto 2 cleared

    W906_ShowMyMessageBoxYesNo_Hook = 0;
    W906_ShowMyMessageBoxYesNo_Reset();
    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
