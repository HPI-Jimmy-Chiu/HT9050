// =============================================================================
//  tests/test_heater_sim_tick.cpp -- S-12: golden THeaterThread::HeaterThreadProcess in the SIM build (HeaterSimTick.cpp).
//  AI(W906-S12) 20260929 (St02-E).  Suite name (add_test): HeaterSimTick
//
//  Golden 906_0625_Steven uHeaterThread.cpp :56-66 (the body) and :138-144 (CheckHeater's SIM branch: fHeaterOK /
//  fHeaterStableOK / iHeaterWait = fMain->chkHeaterOk->Checked, iATCOnLine = true).
//    1. InitialOK false: nothing changes (golden :58).
//    2. SIM, InitialOK, chkHeaterOk checked: fHeaterOK / fHeaterStableOK true, iHeaterWait 1, iATCOnLine true, the four
//       door flags false, and CheckHeaterOK() true in Hot mode (the InArm case 50 gate, ainarm9045_2x2_4.cpp:1867-1872).
//    3. SIM, chkHeaterOk unchecked: fHeaterOK false -> CheckHeaterOK() false in Hot mode, true in Ambient.
//    4. SHIP: the tick is a no-op (the flags keep the sentinel values).
//  No IO table is loaded, so every sensor is Enable=false: IsOff() is false (no EMG pressed, mysensor.cpp:176-181) and the
//  heater-fan alarm path is skipped (Sen[SnIndexHeaterFan].Enable false).  The HeaterLog hook is 0 here: no file is written.
//  Every global it changes is saved and restored, including SW[SwHeaterRelay] / SW[SwHeaterFan].OutValue, which
//  DoHeaterOn sets through On() / Off() even with Enable false (myswitch.cpp).  One thing is not restored: HeaterLog's
//  function-local static OldMessage (cpublic.cpp:697), which only de-duplicates log lines and has no hook here.
//  (St02-E2 review m1, ST02_S12_REVIEW_20260929.md.)
// =============================================================================
#include "MachineType.h"      // SOFT_SIMULTE
#include "cmydef.h"           // InitialOK, fHeaterOK, fHeaterStableOK, iHeaterWait, iATCOnLine, Tempture_*
#include "csystem.h"          // bHeaterDoorIsOpen[]
#include "LastSet.h"          // LastSet.iTemperature
#include "forms/fMain.h"      // fMain->chkHeaterOk
#include "myswitch.h"         // SW[] (DoHeaterOn writes SwHeaterRelay / SwHeaterFan)

#include <cstdio>

namespace ht9045 { void W906_HeaterSimTick(); unsigned long W906_HeaterSimTickExceptions(); }   // HeaterSimTick.cpp
bool CheckHeaterOK();                                                                             // uHeaterThread.cpp:478

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

static void Sentinel()
{
    fHeaterOK = false; fHeaterStableOK = false; iHeaterWait = 7; iATCOnLine = false;
    for (int i = 0; i < 4; ++i) bHeaterDoorIsOpen[i] = true;
}

int main()
{
    std::printf("HeaterSimTick\n");
    const bool savedInit = InitialOK, savedOK = fHeaterOK, savedStable = fHeaterStableOK, savedATC = iATCOnLine;
    const int savedWait = iHeaterWait, savedTemp = LastSet.iTemperature;
    bool savedDoor[4];
    for (int i = 0; i < 4; ++i) savedDoor[i] = bHeaterDoorIsOpen[i];
    CHECK(fMain != 0 && fMain->chkHeaterOk != 0);
    if (fMain == 0 || fMain->chkHeaterOk == 0) return 1;
    const bool savedChk = fMain->chkHeaterOk->Checked;
    const bool savedRelay = SW[SwHeaterRelay].OutValue, savedFan = SW[SwHeaterFan].OutValue;

    std::printf("  -- 1. InitialOK false\n");
    Sentinel();
    InitialOK = false;
    fMain->chkHeaterOk->Checked = true;
    ht9045::W906_HeaterSimTick();
    CHECK(fHeaterOK == false && fHeaterStableOK == false && iHeaterWait == 7 && iATCOnLine == false && bHeaterDoorIsOpen[0]);

#ifdef SOFT_SIMULTE
    std::printf("  -- 2. SIM, chkHeaterOk checked\n");
    Sentinel();
    InitialOK = true;
    LastSet.iTemperature = Tempture_Hot;
    fMain->chkHeaterOk->Checked = true;
    ht9045::W906_HeaterSimTick();
    CHECK(fHeaterOK == true && fHeaterStableOK == true && iHeaterWait == 1 && iATCOnLine == true);
    CHECK(!bHeaterDoorIsOpen[0] && !bHeaterDoorIsOpen[1] && !bHeaterDoorIsOpen[2] && !bHeaterDoorIsOpen[3]);
    CHECK(CheckHeaterOK() == true);

    std::printf("  -- 3. SIM, chkHeaterOk unchecked\n");
    fMain->chkHeaterOk->Checked = false;
    ht9045::W906_HeaterSimTick();
    CHECK(fHeaterOK == false && fHeaterStableOK == false && iHeaterWait == 0);
    CHECK(CheckHeaterOK() == false);
    LastSet.iTemperature = Tempture_Ambient;
    CHECK(CheckHeaterOK() == true);
#else
    std::printf("  -- 4. SHIP: the tick is a no-op\n");
    Sentinel();
    InitialOK = true;
    fMain->chkHeaterOk->Checked = true;
    ht9045::W906_HeaterSimTick();
    CHECK(fHeaterOK == false && fHeaterStableOK == false && iHeaterWait == 7 && iATCOnLine == false && bHeaterDoorIsOpen[0]);
#endif
    CHECK(ht9045::W906_HeaterSimTickExceptions() == 0);

    InitialOK = savedInit; fHeaterOK = savedOK; fHeaterStableOK = savedStable; iATCOnLine = savedATC;
    iHeaterWait = savedWait; LastSet.iTemperature = savedTemp; fMain->chkHeaterOk->Checked = savedChk;
    for (int i = 0; i < 4; ++i) bHeaterDoorIsOpen[i] = savedDoor[i];
    SW[SwHeaterRelay].OutValue = savedRelay; SW[SwHeaterFan].OutValue = savedFan;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
