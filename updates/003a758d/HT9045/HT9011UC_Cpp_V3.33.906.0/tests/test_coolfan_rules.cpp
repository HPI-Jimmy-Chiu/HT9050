// =============================================================================
//  tests/test_coolfan_rules.cpp -- golden DoSwCoolingFan (csystem.cpp:24787 = golden 906_0625_Steven csystem.cpp:20434-20569)
//  and its callers, lifted by the RULINGS_20261001 #9 work card.  AI(W906-COOLFAN) 20261001 (St02-E).  Suite: CoolFan_Rules
//
//  The two outputs it drives: SW[SwCoolingFan_Blower] (OutValue) and Cylinder[C_CoolingValve] (Status; golden "CoolingValve
//  Off 是吹風進去").  No IO table is loaded, so both points are Enable=false: On() / Off() only set those fields, nothing
//  reaches a card (myswitch.cpp:74-76, mycylin.cpp:344-362).
//    1. A normal machine (not HT9046_LS, USE_AIR_CONDITIONER 0), golden :20548-20567: chamber over fChamberCoolTemp-1 in
//       Ambient, or in Hot with Head / Head+Socket heating -> blower on, valve off; otherwise (Chamber heating, cool chamber,
//       no reading 999) -> blower off, valve on.
//    2. H1-07 (golden :20437-20440): the IO Set View page open -> return false and touch nothing, both through
//       fiosetview->fShow and through the web page table (W906_FormFShowHook "fiosetview").  H1-08 (golden :20442-20446) in a
//       ctest: FileRW_ProxyChecked is the ht9045_globals fallback (false), so an open "fConfiguration" page never refuses --
//       the same answer whether or not H1-08's port line is in (it reads the chkHeater proxy, which only wb_serve has).
//    3. An air-conditioner machine (USE_AIR_CONDITIONER 1), golden :20448-20546: CCD over temperature -> blower on, valve on;
//       Hot + Head mode -> by the chamber temperature; Ambient, not ASE Kaohsiung: bHALTing and a warm chamber -> blower on,
//       valve off, else blower off (valve untouched).  The fAirCon->PowerUp / PowerDown lines stay gated (H1-09a-f).
//    4. The callers: SIM -- W906_HeaterSimTick -> DoHeaterOn (G12b) reaches it.  CheckHeater's GATE 5 (golden
//       uHeaterThread.cpp:173) sits after golden's SOFT_SIMULTE early return (uHeaterThread.cpp:505-511): SHIP-only.
//       SHIP -- the tick is a no-op, so the outputs stay as set (no caller yet: THeaterThread is never created, MainClose.cpp:986).
//  Every global it changes is saved and restored.
// =============================================================================
#include "MachineType.h"      // SOFT_SIMULTE, Type_HT9046_LS, HeadOnly / ChamberOnly / HeadSocket, tcChamber, CC_*
#include "cmydef.h"           // InitialOK, MachineTypeChoice, USE_AIR_CONDITIONER, bCCDOverTemp, bHALTing, UN150Read, CUSTOMER_CODE,
                              // fHeaterOK / fHeaterStableOK / iHeaterWait / iATCOnLine
#include "cprod.h"            // Temperature.iIndexHeatMode / fChamberCoolTemp
#include "csystem.h"          // DoSwCoolingFan, W906_FormFShowHook, bHeaterDoorIsOpen[]
#include "LastSet.h"          // LastSet.iTemperature
#include "forms/fMain.h"      // fMain->chkHeaterOk (the SIM tick's CheckHeater reads it)
#include "atester_shims.h"    // fiosetview (TfiosetviewShim::fShow)
#include "myswitch.h"         // SW[]
#include "mycylin.h"          // Cylinder[]

#include <cstdio>
#include <cstring>

namespace ht9045 { void W906_HeaterSimTick(); unsigned long W906_HeaterSimTickExceptions(); }   // HeaterSimTick.cpp

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

static const char* g_openPage = 0;   // the fake web page table: this one golden form is "open"
static bool FakePageTable(const char* goldenForm) { return g_openPage != 0 && std::strcmp(goldenForm, g_openPage) == 0; }

static bool Blower() { return SW[SwCoolingFan_Blower].OutValue; }
static bool Valve()  { return Cylinder[C_CoolingValve].Status; }
static void Preset(bool blower, bool valve)
{
    SW[SwCoolingFan_Blower].OutValue = blower;
    Cylinder[C_CoolingValve].Status  = valve;
}

// one normal-machine case: preset the opposite of the expected result, run, compare
static void Normal(const char* name, int temp, int heatMode, double chamber, bool expBlower, bool expValve)
{
    LastSet.iTemperature        = temp;
    Temperature.iIndexHeatMode  = heatMode;
    UN150Read[tcChamber]        = chamber;
    Preset(!expBlower, !expValve);
    const bool r = DoSwCoolingFan(false);
    std::printf("  -- %s\n", name);
    CHECK(r == true);
    CHECK(Blower() == expBlower && Valve() == expValve);
}

int main()
{
    std::printf("CoolFan_Rules\n");
    const int    sMachine = MachineTypeChoice, sAirCon = USE_AIR_CONDITIONER, sCust = CUSTOMER_CODE;
    const int    sTemp = LastSet.iTemperature, sMode = Temperature.iIndexHeatMode;
    const double sCool = Temperature.fChamberCoolTemp, sChamber = UN150Read[tcChamber];
    const bool   sCCD = bCCDOverTemp, sHalt = bHALTing, sInit = InitialOK;
    const bool   sBlower = Blower(), sValve = Valve();
    bool (*const sHook)(const char*) = W906_FormFShowHook;
    CHECK(fiosetview != 0);
    if (fiosetview == 0) return 1;
    const bool sIoShow = fiosetview->fShow;

    if (MachineTypeChoice == Type_HT9046_LS)
        MachineTypeChoice = 0;
    USE_AIR_CONDITIONER = 0;
    CUSTOMER_CODE = 0;
    bCCDOverTemp = false;
    bHALTing = false;
    fiosetview->fShow = false;
    W906_FormFShowHook = 0;
    Temperature.fChamberCoolTemp = 35.0;

    std::printf(" 1. normal machine (golden :20548-20567)\n");
    Normal("Ambient, chamber 40 > 34",                 Tempture_Ambient, ChamberOnly, 40.0, true,  false);
    Normal("Ambient, chamber 30 <= 34",                Tempture_Ambient, ChamberOnly, 30.0, false, true);
    Normal("Ambient, no reading (999)",                Tempture_Ambient, ChamberOnly, 999.0, false, true);
    Normal("Hot + HeadOnly, chamber 40",               Tempture_Hot,     HeadOnly,    40.0, true,  false);
    Normal("AmbientHot + HeadSocket, chamber 40",      Tempture_AmbientHot, HeadSocket, 40.0, true, false);
    Normal("Hot + ChamberOnly, chamber 40",            Tempture_Hot,     ChamberOnly, 40.0, false, true);

    std::printf(" 2. H1-07 the IO page / H1-08 in a ctest\n");
    LastSet.iTemperature = Tempture_Ambient;  Temperature.iIndexHeatMode = ChamberOnly;  UN150Read[tcChamber] = 40.0;
    Preset(false, true);
    fiosetview->fShow = true;
    CHECK(DoSwCoolingFan(false) == false);
    CHECK(Blower() == false && Valve() == true);             // untouched (else blower on, valve off)
    fiosetview->fShow = false;
    W906_FormFShowHook = &FakePageTable;
    g_openPage = "fiosetview";
    CHECK(DoSwCoolingFan(false) == false);
    CHECK(Blower() == false && Valve() == true);
    g_openPage = "fConfiguration";                           // chkHeater proxy: the fallback answers false -> not refused
    CHECK(DoSwCoolingFan(false) == true);
    CHECK(Blower() == true && Valve() == false);
    g_openPage = 0;
    W906_FormFShowHook = 0;

    std::printf(" 3. air-conditioner machine (golden :20448-20546; fAirCon lines stay gated)\n");
    USE_AIR_CONDITIONER = 1;
    bCCDOverTemp = true;
    Preset(false, false);
    CHECK(DoSwCoolingFan(false) == true);
    CHECK(Blower() == true && Valve() == true);
    bCCDOverTemp = false;
    LastSet.iTemperature = Tempture_Hot;  Temperature.iIndexHeatMode = HeadOnly;
    UN150Read[tcChamber] = 40.0;  Preset(false, true);
    CHECK(DoSwCoolingFan(false) == true);
    CHECK(Blower() == true && Valve() == false);              // Head mode, chamber over fChamberCoolTemp
    UN150Read[tcChamber] = 30.0;  Preset(true, true);
    CHECK(DoSwCoolingFan(false) == true);
    CHECK(Blower() == false && Valve() == false);
    LastSet.iTemperature = Tempture_Ambient;  UN150Read[tcChamber] = 40.0;
    bHALTing = true;  Preset(false, true);
    CHECK(DoSwCoolingFan(false) == true);
    CHECK(Blower() == true && Valve() == false);              // halting + warm chamber: fast cool
    bHALTing = false;  Preset(true, true);
    CHECK(DoSwCoolingFan(false) == true);
    CHECK(Blower() == false && Valve() == true);              // idle: blower off, valve untouched
    USE_AIR_CONDITIONER = 0;

    std::printf(" 4. the callers (W906_HeaterSimTick -> DoHeaterOn G12b; GATE 5 is SHIP-only)\n");
    CHECK(fMain != 0 && fMain->chkHeaterOk != 0);
    if (fMain != 0 && fMain->chkHeaterOk != 0)
    {
        const bool sChk = fMain->chkHeaterOk->Checked;
        const bool sOK = fHeaterOK, sStable = fHeaterStableOK, sATC = iATCOnLine;   // the tick's CheckHeater writes these
        const int  sWait = iHeaterWait;
        bool sDoor[4];
        for (int i = 0; i < 4; ++i) sDoor[i] = bHeaterDoorIsOpen[i];
        const bool sRelay = SW[SwHeaterRelay].OutValue, sFan = SW[SwHeaterFan].OutValue;   // DoHeaterOn sets them
        LastSet.iTemperature = Tempture_Ambient;  Temperature.iIndexHeatMode = ChamberOnly;  UN150Read[tcChamber] = 999.0;
        InitialOK = true;
        fMain->chkHeaterOk->Checked = true;
        Preset(true, false);
        ht9045::W906_HeaterSimTick();
#ifdef SOFT_SIMULTE
        CHECK(Blower() == false && Valve() == true);          // reached: no chamber reading -> blower off, valve on
#else
        CHECK(Blower() == true && Valve() == false);          // SHIP: the tick is a no-op (no caller yet)
#endif
        CHECK(ht9045::W906_HeaterSimTickExceptions() == 0);
        fMain->chkHeaterOk->Checked = sChk;
        fHeaterOK = sOK;  fHeaterStableOK = sStable;  iATCOnLine = sATC;  iHeaterWait = sWait;
        for (int i = 0; i < 4; ++i) bHeaterDoorIsOpen[i] = sDoor[i];
        SW[SwHeaterRelay].OutValue = sRelay;  SW[SwHeaterFan].OutValue = sFan;
    }

    MachineTypeChoice = sMachine;  USE_AIR_CONDITIONER = sAirCon;  CUSTOMER_CODE = sCust;
    LastSet.iTemperature = sTemp;  Temperature.iIndexHeatMode = sMode;  Temperature.fChamberCoolTemp = sCool;
    UN150Read[tcChamber] = sChamber;  bCCDOverTemp = sCCD;  bHALTing = sHalt;  InitialOK = sInit;
    fiosetview->fShow = sIoShow;  W906_FormFShowHook = sHook;
    Preset(sBlower, sValve);
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
