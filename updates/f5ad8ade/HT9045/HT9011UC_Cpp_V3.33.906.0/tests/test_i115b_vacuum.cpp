// =============================================================================
//  tests/test_i115b_vacuum.cpp -- AI(W906-I115B) 20260930
//
//  INBOX 115 class B row 18 (RULINGS_20260930 #4): VacuumUnit/VacuumUnit.cpp
//  GATE (1) lifted -- SetIOTableByECAT_VC8_Sucker / SetSuckISABase /
//  VaccumCopyFormSuck / VaccumCopyToSuck are the golden bodies (golden
//  VacuumUnit.cpp:431-548) and the four port stubs are commented out.
//  Its own executable: VacuumUnit.h pulls Public/HTEditList.h, which cannot
//  share a TU with aHotPlateSubstrate.h (tests/test_i115b_lifted.cpp).
//
//    V1  VCCU_UNIT_TYPE!=1: nothing is copied (golden :433).
//    V2  VCCU_UNIT_TYPE==1: every F/B test-arm and In/Out-arm nozzle gets
//        ISABase=ePCI1203 on all four fields (golden :482-488), and the panel
//        receives the nozzle's On/Off/Sen Ring/IP/Port/Bit/Type/ISABase and name
//        (golden :490-522; the three ->Hint writes stay gated -- no widget).
//    V3  VaccumCopyToSuck keeps golden's slip (golden :534): OnRing gets the
//        panel's OffRing and the nozzle's OffRing is never written.
//
//  No caller in the port runs SetIOTableByECAT_VC8_Sucker yet (golden calls it
//  from TfMain::FormShow main.cpp:10934 and iosetview.cpp:2090), so this test is
//  the only thing that executes it.  No file IO.
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include "VacuumUnit/VacuumUnit.h"
#include "VacuumUnit/MyVacuumPanel.h"
#include "mykitsuck.h"              // TMySucker, FTestSuck / BTestSuck / InArmSuck / OutArmSuck
#include "cmydef.h"                 // VCCU_UNIT_TYPE
#include "MachineType.h"            // ePCI1203

#include <cstdio>

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_i115b_vacuum.cpp:%d]  %s\n", line, what); }
    else     {           std::printf("  ok    %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

static void Seed(TMySucker& s, int base)
{
    s.SuckerName = "SeedSuck";
    s.OnRing  = base + 1; s.OnIP  = base + 2; s.OnPort  = base + 3; s.OnBit  = base + 4; s.OnType  = 0; s.OnISABase  = 0;
    s.OffRing = base + 5; s.OffIP = base + 6; s.OffPort = base + 7; s.OffBit = base + 8; s.OffType = 0; s.OffISABase = 0;
    s.SenRing = base + 9; s.SenIP = base + 10; s.SenPort = base + 11; s.SenBit = base + 12; s.SenType = 0; s.SenISABase = 0;
    s.ISABase = 0;
}

int main()
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    fVacuumUnit = new TfVacuumUnit(0);
    // TMyVacuumPanel's ctor reads three of the form's own widgets (VacuumUnit/MyVacuumPanel.cpp:246 / :266 / :283:
    // pnlCurectVal->Color, edSV->Text, btnSV->Caption); golden has them from the .dfm, the port's TfVacuumUnit ctor
    // leaves them unset (Initial() is not run in the port either -- FileRW/TestIF_File_VacuumUnit.cpp:29), so the
    // fixture supplies them.
    fVacuumUnit->pnlCurectVal = new TPanel();
    fVacuumUnit->edSV         = new TEdit();
    fVacuumUnit->btnSV        = new TSpeedButton();
    fVacuumUnit->iIndexColMax = 1;
    fVacuumUnit->iInOutColMax = 1;
    for (int r = 0; r < 2; ++r)
    {
        fVacuumUnit->myPalArm1[0][r]  = new TMyVacuumPanel(fVacuumUnit, 4, 0, r);
        fVacuumUnit->myPalArm2[0][r]  = new TMyVacuumPanel(fVacuumUnit, 5, 0, r);
        fVacuumUnit->myPalInArm[0][r] = new TMyVacuumPanel(fVacuumUnit, 2, 0, r);
        fVacuumUnit->myPalOutArm[0][r]= new TMyVacuumPanel(fVacuumUnit, 3, 0, r);
        Seed(FTestSuck.Suck[r][0], 100 + r * 20);
        Seed(BTestSuck.Suck[r][0], 200 + r * 20);
        Seed(InArmSuck.Suck[r][0], 300 + r * 20);
        Seed(OutArmSuck.Suck[r][0], 400 + r * 20);
    }
    TMyVacuumPanel& p = *fVacuumUnit->myPalArm1[0][1];
    const int ringBefore = p.OnRing;

    std::printf("-- V1: VCCU_UNIT_TYPE=0 -- golden :433 skips everything --\n");
    VCCU_UNIT_TYPE = 0;
    SetIOTableByECAT_VC8_Sucker();
    CHECK(p.OnRing == ringBefore);
    CHECK(FTestSuck.Suck[1][0].OnISABase == 0 && FTestSuck.Suck[1][0].ISABase == 0);

    std::printf("-- V2: VCCU_UNIT_TYPE=1 -- golden :456-478 --\n");
    VCCU_UNIT_TYPE = 1;
    SetIOTableByECAT_VC8_Sucker();
    const TMySucker& f = FTestSuck.Suck[1][0];
    CHECK(f.OnISABase == ePCI1203 && f.OffISABase == ePCI1203 && f.SenISABase == ePCI1203 && f.ISABase == ePCI1203);
    CHECK(p.SuckerName == "SeedSuck");
    CHECK(p.OnRing == 121 && p.OnIP == 122 && p.OnPort == 123 && p.OnVCNo == 124 && p.OnType == 0 && p.OnISABase == ePCI1203);
    CHECK(p.OffRing == 125 && p.OffIP == 126 && p.OffPort == 127 && p.OffVCNo == 128 && p.OffISABase == ePCI1203);
    CHECK(p.SenRing == 129 && p.SenIP == 130 && p.SenPort == 131 && p.SenVCNo == 132 && p.SenISABase == ePCI1203);
    CHECK(fVacuumUnit->myPalArm2[0][0]->OnRing == 201 && BTestSuck.Suck[0][0].ISABase == ePCI1203);
    CHECK(fVacuumUnit->myPalInArm[0][1]->SenVCNo == 332 && InArmSuck.Suck[1][0].SenISABase == ePCI1203);
    CHECK(fVacuumUnit->myPalOutArm[0][0]->OffPort == 407 && OutArmSuck.Suck[0][0].OffISABase == ePCI1203);

    std::printf("-- V3: VaccumCopyToSuck keeps golden :534 OnRing=OffRing --\n");
    TMySucker t;
    t.OnRing = -1; t.OffRing = -2; t.ISABase = 0;
    TMyVacuumPanel q(fVacuumUnit, 2, 0, 0);
    q.OnRing = 11; q.OffRing = 19; q.OnIP = 12; q.OffIP = 13; q.SenPort = 14; q.OffVCNo = 15;
    VaccumCopyToSuck(q, t);
    CHECK(t.OnRing == 19);                                                    // golden overwrites OnRing with the panel OffRing
    CHECK(t.OffRing == -2);                                                   // and never writes OffRing
    CHECK(t.OnIP == 12 && t.OffIP == 13 && t.SenPort == 14 && t.OffBit == 15);
    CHECK(t.ISABase == ePCI1203);

    std::printf("\n%d checks, %d failed\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
