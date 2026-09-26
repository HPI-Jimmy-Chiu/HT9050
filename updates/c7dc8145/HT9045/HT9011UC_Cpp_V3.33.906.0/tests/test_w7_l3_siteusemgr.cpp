// =============================================================================
//  test_w7_l3_siteusemgr.cpp -- cSiteUseManager (golden cSiteUseManager.cpp,
//  556 lines / cSiteUseManager.h, 72 lines -- both at the GOLDEN TREE ROOT)
//
//  Translation wave: W906-W7-L3
//  Author: AI(W906-W7-L3) 20260802
//
//  AI(W906-W7-L3) 20260802: REPLACES the integrator's 0-PASS/0-FAIL placeholder.
//
//  WHAT WAS THERE BEFORE, AND WHY IT MATTERS FOR THE ASSERTION DESIGN
//  ------------------------------------------------------------------
//  Until this wave the ported cSiteUseManager was a deliberate OFFLINE SHIM: 5 of
//  golden's 24 public members, with constant / no-op bodies --
//      UseCompactSearch()          -> false
//      CompactSearchPlateToPlace() -> true
//      CanAnyNozzleReachHP()       -> true
//      ShadowLogBeforeSearch(int)  -> {}
//      ValidateSearchResult(int)   -> {}
//  A CONSTANT-RETURNING SHIM IS THE OLD BEHAVIOUR.  So an assertion the shim would
//  also satisfy proves nothing.  Every assertion below is therefore written so the
//  shim's answer is WRONG for at least one fixture:
//    * UseCompactSearch()          is asserted TRUE  after SetUseCompactSearch(true)
//                                  and after Init() over an ini that says 1.
//    * CompactSearchPlateToPlace() is asserted FALSE on a full plate, and asserted
//                                  to WRITE a SPECIFIC (plate,x,y) into the
//                                  iPlacePlate/X/Y globals on a partial plate.
//    * CanAnyNozzleReachHP()       is asserted FALSE on a full plate.
//    * ShadowLogBeforeSearch()/ValidateSearchResult() are LOG-ONLY, so their only
//                                  observable is the RecordErrorLog text; this file
//                                  captures stdout and asserts the EXACT line.
//
//  PROOF OBJECTIVE
//  ---------------
//  Pin the REAL COMPUTED behaviour of all 24 members over hand-built HotPlate /
//  nozzle fixtures, including the ten golden quirks the translation preserves
//  (Q1..Q10, named in cSiteUseManager.cpp's banner).  Failability is proven by
//  mutation, not by argument -- see the agent report for the three runs.
//
//  TWO TEST SEAMS THIS FILE INTRODUCES (both fixture-only; zero production change)
//  ------------------------------------------------------------------------------
//  1. STDOUT CAPTURE.  RecordErrorLog's offline body (cpublic.cpp:1888-1896) does
//     `printf("  [RecordErrorLog] %s | %s\n", ...)`.  Several of the 24 members
//     (Init, ShadowLogBeforeSearch, ShadowLogAfterPlace, ValidateSearchResult,
//     SetUseCompactSearch) and the whole DumpCompactSearchFailure diagnostic have
//     NO other observable at all, so the suite re-points fd 1 at a scratch file
//     with _dup/_dup2 around each call and asserts on the captured text.  There is
//     no hook inside RecordErrorLog to override and none is added.
//  2. SIM MOTORS ON MInArmX / MInArmY.  DumpCompactSearchFailure derefs
//     MOT[MInArmX].Motor->PSoftLimitP (golden :472-473) with NO null guard, and
//     TMyMotor's ctor never assigns Motor, so in this tree those pointers are NULL
//     until someone installs one.  Golden would crash the same way if cinitial had
//     not installed them, so the CRASH IS FAITHFUL and is NOT patched in the
//     production file; instead the fixture installs two TMySimMotor instances,
//     exactly as tests/test_motor_w4.cpp:412 already does.
//
//  WHAT IT DRIVES -- all 24 golden public members:
//    ctor, Init, CanInNozzlePick, GetActiveInNozzleCount, CanOutNozzlePick,
//    GetActiveOutNozzleCount, GetHPOccupiedCount, GetHPAvailableCount,
//    GetHPCapacity, HPHasSpace, GetPlaceableCountPerCycle, WillExceedHPBoundary,
//    DumpInNozzleMap, DumpOutNozzleMap, DumpHPStatus, ShadowLogBeforeSearch,
//    ShadowLogAfterPlace, ValidateSearchResult, UseCompactSearch,
//    SetUseCompactSearch, CompactSearchPlateToPlace, WillExceedMotorLimit,
//    CanAnyNozzleReachHP, DumpCompactSearchFailure.
//
//  NOT COVERED -- stated plainly (see also the report):
//    * The 10 golden call sites in ainarm9045_1x4_4.cpp are NOT driven from here.
//      DoPlaceToHotPlate_9045_1x4_4 is only reachable through
//      DoPlaceToHotPlate_9045, whose dispatcher is `#if 0` in
//      ainarm_SearchPlacePlate.cpp:4859, and DoInArm_9045_1x4_4's SiteUseMgr arm
//      short-circuits on UseCompactSearch()==false.  This file tests the class,
//      not the in-arm integration.
//    * Init() is exercised over a SCRATCH ini this file writes; the real
//      D:\HT9045\system\Gerneral.ini is never read or written.
//    * No real HotPlate hardware, no motor motion: MOT[] tray data is set
//      directly and the sim motors are installed only so the diagnostic dump can
//      read PSoftLimit* without dereferencing NULL.
//    * Golden's own numeric geometry (XPitch/YPitch/base positions) is only
//      echoed by DumpCompactSearchFailure; this file asserts that the echo is
//      present and internally consistent, not that the geometry is right.
//    * HotPlateYPitchCanPutAll() / Row2CanPutHP(PlaceMode) / iYHalf are NEVER
//      CALLED by golden's CompactSearchPlateToPlace (see QUIRK Q1), so no
//      assertion here can reach them -- that is a property of golden, not a gap
//      the fixture could close.
// =============================================================================
#include "cSiteUseManager.h"
#include "aHotPlateSubstrate.h"      // iPlacePlate/X/Y, bPitchOver12000, InArmSuck
#include "ainarm_SearchPlacePlate.h" // bZFlgToHP, iPlaceHP
#include "Motor/mymotor.h"           // MOT[]
#include "Motor/mySimMotor.h"        // TMySimMotor (fixture-only)
#include "cprod.h"                   // Prod / HotPlateForm / TestIF_File
#include "cmydef.h"                  // NULL_IC / MMPlate1 / MInArmX / iInArmType / ...
#include "MachineType.h"             // MAX_ARM_Row / MAX_ARM_Col / eCKPos_HP2
#include "common.h"                  // asGeneralPath / OpenGeneralIniFile
#include "cpublic.h"                 // RecordErrorLog (the thing we capture)

#include <cstdio>
#include <cstdlib>
#include <io.h>                      // _dup / _dup2 / _fileno / _close (MinGW)

// ---------------------------------------------------------------------------
//  CHECK harness (same shape as tests/test_ini_helpers.cpp / test_w7_l1_auto2.cpp)
// ---------------------------------------------------------------------------
static int g_pass = 0;
static int g_fail = 0;

static void check(bool cond, const char* expr, const char* what, int line)
{
    if (cond)
    {
        ++g_pass;
    }
    else
    {
        ++g_fail;
        std::printf("FAIL [line %d]  %s\n    -- %s\n", line, what, expr);
    }
}
#define CHECK(cond, what) check((cond), #cond, (what), __LINE__)

// ---------------------------------------------------------------------------
//  SEAM 1 -- stdout capture around a RecordErrorLog-emitting call
// ---------------------------------------------------------------------------
static AnsiString capPath()
{
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    return AnsiString(t) + AnsiString("\\ht9045_w7l3_siteusemgr_cap.txt");
}

static int g_savedFd = -1;

// _fileno() is not visible under this toolchain's strict-ANSI mode (only _dup /
// _dup2 / _close are), so use the POSIX/Windows constant for stdout directly.
static const int kStdoutFd = 1;

// AI(W906-RECERR) 20260927: RecordErrorLog (cpublic.cpp EOF) now writes golden's dated file instead of printing, so the
//   capture also collects what was APPENDED to <as9045LogPath>\SiteUseMgr\YYYY\MM\YYYYMMDDHH.txt between capBegin and
//   capEnd (both files if the hour rolled over).  The sandbox is ctest's machine_log_scratch (ENV-ALL + St02 D5).
static AnsiString g_logPathAtBegin;
static long       g_logSizeAtBegin = 0;

static AnsiString siteUseLogPath()
{
    GetTimeInfo();
    AnsiString p;
    p.sprintf("%s\\SiteUseMgr\\%04d\\%02d\\%04d%02d%02d%02d.txt", as9045LogPath.c_str(), SystemYear, SystemMonth,
              SystemYear, SystemMonth, SystemDate, SystemHour);
    return p;
}

static long fileSize(const AnsiString& path)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return 0;
    std::fseek(f, 0, SEEK_END);
    long n = std::ftell(f);
    std::fclose(f);
    return n < 0 ? 0 : n;
}

static AnsiString readFrom(const AnsiString& path, long offset)
{
    AnsiString out = "";
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return out;
    std::fseek(f, offset, SEEK_SET);
    char buf[2048];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf) - 1, f)) > 0)
    {
        buf[n] = '\0';
        out += AnsiString(buf);
    }
    std::fclose(f);
    return out;
}

static void capBegin()
{
    g_logPathAtBegin = siteUseLogPath();                 // AI(W906-RECERR) 20260927
    g_logSizeAtBegin = fileSize(g_logPathAtBegin);
    std::fflush(stdout);
    g_savedFd = _dup(kStdoutFd);
    if (std::freopen(capPath().c_str(), "w", stdout) == NULL)
    {
        // Capture unavailable -- restore immediately so the run stays visible.
        _dup2(g_savedFd, kStdoutFd);
        _close(g_savedFd);
        g_savedFd = -1;
    }
}

static AnsiString capEnd()
{
    std::fflush(stdout);
    if (g_savedFd >= 0)
    {
        _dup2(g_savedFd, kStdoutFd);
        _close(g_savedFd);
        g_savedFd = -1;
    }
    std::clearerr(stdout);

    AnsiString out = "";
    FILE* f = std::fopen(capPath().c_str(), "rb");
    if (f)
    {
        char buf[2048];
        size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf) - 1, f)) > 0)
        {
            buf[n] = '\0';
            out += AnsiString(buf);
        }
        std::fclose(f);
    }
    // AI(W906-RECERR) 20260927: plus what RecordErrorLog appended to the dated log file(s) meanwhile.
    AnsiString nowPath = siteUseLogPath();
    out += readFrom(g_logPathAtBegin, g_logSizeAtBegin);
    if (nowPath != g_logPathAtBegin)
        out += readFrom(nowPath, 0);
    return out;
}

// AnsiString::Pos is 1-based VCL semantics (vclcompat) -> >0 means "found".
static bool has(const AnsiString& hay, const char* needle)
{
    return hay.Pos(AnsiString(needle)) > 0;
}

// ---------------------------------------------------------------------------
//  FIXTURE helpers
// ---------------------------------------------------------------------------
// Golden indexes MOT[MMPlate1 + p].Tray.Data[x][y] with x in [0,XDivision) and
// y in [0,YDivision).  TMyTray::Data is [_MAX_COL_ITEM=30][_MAX_ROW_ITEM=70], so
// every fixture below keeps XDivision <= 30 and YDivision <= 70.
static void setHP(int xdiv, int ydiv, int plateSelect)
{
    HotPlateForm.XDivision    = xdiv;
    HotPlateForm.YDivision    = ydiv;
    HotPlateForm.iPlateSelect = plateSelect;
}

static void fillPlate(int p, int value)
{
    for (int x = 0; x < 30; ++x)
        for (int y = 0; y < 70; ++y)
            MOT[MMPlate1 + p].Tray.Data[x][y] = value;
}

static void clearNozzles()
{
    for (int s = 0; s < 2; ++s)
        for (int r = 0; r < MAX_ARM_Row; ++r)
            for (int c = 0; c < MAX_ARM_Col; ++c)
                Prod.fInArmSuckUse[s][r][c] = false;
    for (int r = 0; r < MAX_ARM_Row; ++r)
        for (int c = 0; c < MAX_ARM_Col; ++c)
        {
            Prod.fOutArmSuckUse[r][c] = false;
            bZFlgToHP[r][c]           = false;
        }
}

// The canonical nozzle fixture reused by several sections:
//   SHT0 active = {[0][0], [0][1]}          -> GetActiveInNozzleCount(0) == 2
//   SHT1 active = {[0][0]}                  -> GetActiveInNozzleCount(1) == 1
//   bZFlgToHP   = {[0][0]}                  -> GetPlaceableCountPerCycle(0) == 1
// GetPlaceableCountPerCycle(0) is deliberately DIFFERENT from
// GetActiveInNozzleCount(0) so an implementation that forgot the bZFlgToHP AND
// (golden :117) cannot pass.
static void setCanonicalNozzles()
{
    clearNozzles();
    Prod.fInArmSuckUse[0][0][0] = true;
    Prod.fInArmSuckUse[0][0][1] = true;
    Prod.fInArmSuckUse[1][0][0] = true;
    Prod.fOutArmSuckUse[1][3]   = true;
    bZFlgToHP[0][0]             = true;
}

static void clearAutoCleanSkip()
{
    TestIF_File.iAutoClean_Tray     = -1;
    TestIF_File.iAutoClean_Function = 0;
}

static void writeIni(const AnsiString& path, const char* body)
{
    FILE* f = std::fopen(path.c_str(), "wb");
    if (f) { std::fputs(body, f); std::fclose(f); }
}

// ---------------------------------------------------------------------------
int main()
{
    std::printf("=== W7-L3 cSiteUseManager (golden cSiteUseManager.cpp, 556 lines) ===\n");

    // Fixture-only: install sim motors so DumpCompactSearchFailure's UNGUARDED
    // MOT[MInArmX].Motor->PSoftLimitP (golden :472-473) has something to read.
    static TMySimMotor simX;
    static TMySimMotor simY;
    simX.SetSoftLimit(120000, -120000);
    simY.SetSoftLimit( 90000,  -90000);
    MOT[MInArmX].Motor = &simX;
    MOT[MInArmY].Motor = &simY;

    // Deterministic scalars the diagnostic dump echoes.
    iInArmType      = 7;
    iPlaceHP        = 0;
    iYHalf          = 3;
    iInArmXBase     = 0;
    iInArmYBase     = 0;
    iXpitchMaxX3    = 12000;
    bPitchOver12000 = false;
    InArmSuck.iPickRow = 1;
    InArmSuck.iPickCol = 4;
    for (int r = 0; r < MAX_ARM_Row; ++r)
        for (int c = 0; c < MAX_ARM_Col; ++c)
            InArmSuck.Item[r][c] = (r == 0 && c == 2) ? 4 : NULL_IC;
    Prod.HotPlateForm[0].iXPitch  = 5000;
    Prod.HotPlateForm[0].iYPitch  = 4000;
    Prod.XInArm_Plate2_Pick[0][0] = 11000;
    Prod.XInArm_Plate1_Pick[0][0] = 12000;
    Prod.YInArm_Plate2_Pick[0][0] = 21000;
    Prod.YInArm_Plate1_Pick[0][0] = 22000;

    clearAutoCleanSkip();

    // =======================================================================
    // [1] ctor default + the UseCompactSearch/SetUseCompactSearch pair
    //     (golden :20-24 / :263-280)
    // =======================================================================
    {
        cSiteUseManager local;                       // exercises golden :20 ctor
        CHECK(local.UseCompactSearch() == false,
              "[1] fresh cSiteUseManager: m_bUseCompactSearch defaults false (golden :23)");

        capBegin();
        local.SetUseCompactSearch(true);             // golden :274
        AnsiString log = capEnd();
        CHECK(local.UseCompactSearch() == true,
              "[1] SetUseCompactSearch(true) -> UseCompactSearch()==true "
              "(the OLD SHIM answered false unconditionally)");
        CHECK(has(log, "CompactSearch ENABLED"),
              "[1] SetUseCompactSearch(true) logs 'CompactSearch ENABLED' (golden :278)");

        capBegin();
        local.SetUseCompactSearch(false);
        log = capEnd();
        CHECK(local.UseCompactSearch() == false,
              "[1] SetUseCompactSearch(false) -> UseCompactSearch()==false");
        CHECK(has(log, "CompactSearch DISABLED"),
              "[1] SetUseCompactSearch(false) logs 'CompactSearch DISABLED' (golden :278)");
    }

    // The process-wide instance golden declares (golden :17) must exist and start
    // OFF -- this is what keeps the two ainarm9045_1x4_4.cpp compact-search
    // branches dead in the current tree (Init() has no caller there).
    CHECK(SiteUseMgr.UseCompactSearch() == false,
          "[1] global SiteUseMgr starts with compact search OFF (golden :17 + :23)");

    // =======================================================================
    // [2] Init() reads [SiteUseMgr]/bUseCompactSearch from the General ini
    //     (golden :27-35).  Driven over a SCRATCH ini, never the real one.
    // =======================================================================
    {
        const char* t = std::getenv("TEMP");
        if (!t || !*t) t = std::getenv("TMP");
        if (!t || !*t) t = ".";
        AnsiString iniOn  = AnsiString(t) + AnsiString("\\ht9045_w7l3_siteusemgr_on.ini");
        AnsiString iniOff = AnsiString(t) + AnsiString("\\ht9045_w7l3_siteusemgr_off.ini");
        writeIni(iniOn,  "[SiteUseMgr]\r\nbUseCompactSearch=1\r\n");
        writeIni(iniOff, "[SiteUseMgr]\r\nbUseCompactSearch=0\r\n");

        cSiteUseManager local;
        AnsiString savedPath = asGeneralPath;

        // ON: start from the OPPOSITE value so a no-op Init() cannot pass.
        local.SetUseCompactSearch(false);
        asGeneralPath = iniOn;
        OpenGeneralIniFile();
        capBegin();
        local.Init();
        AnsiString log = capEnd();
        CHECK(local.UseCompactSearch() == true,
              "[2] Init() over an ini saying 1 flips compact search ON (golden :29)");
        CHECK(has(log, "Init CompactSearch=ON"),
              "[2] Init() logs 'Init CompactSearch=ON' (golden :33)");

        // OFF: again start from the OPPOSITE value.
        local.SetUseCompactSearch(true);
        asGeneralPath = iniOff;
        OpenGeneralIniFile();
        capBegin();
        local.Init();
        log = capEnd();
        CHECK(local.UseCompactSearch() == false,
              "[2] Init() over an ini saying 0 flips compact search OFF (golden :29)");
        CHECK(has(log, "Init CompactSearch=OFF"),
              "[2] Init() logs 'Init CompactSearch=OFF' (golden :33)");

        asGeneralPath = savedPath;
        DeleteFile(iniOn);
        DeleteFile(iniOff);
    }

    // =======================================================================
    // [3] InArm nozzle queries (golden :39-56)
    // =======================================================================
    setCanonicalNozzles();
    CHECK(SiteUseMgr.CanInNozzlePick(0, 0, 0) == true,
          "[3] CanInNozzlePick(0,0,0) reads Prod.fInArmSuckUse[0][0][0]==true (golden :44)");
    CHECK(SiteUseMgr.CanInNozzlePick(0, 0, 2) == false,
          "[3] CanInNozzlePick(0,0,2) reads a false cell");
    CHECK(SiteUseMgr.CanInNozzlePick(1, 0, 0) == true,
          "[3] CanInNozzlePick(1,0,0): SHT1 plane is a DIFFERENT slice");
    CHECK(SiteUseMgr.CanInNozzlePick(1, 0, 1) == false,
          "[3] CanInNozzlePick(1,0,1): SHT1 is NOT a copy of SHT0");
    CHECK(SiteUseMgr.CanInNozzlePick(2, 0, 0) == false,
          "[3] CanInNozzlePick rejects iSht>1 (golden :41)");
    CHECK(SiteUseMgr.CanInNozzlePick(-1, 0, 0) == false,
          "[3] CanInNozzlePick rejects iSht<0 (golden :41)");
    CHECK(SiteUseMgr.CanInNozzlePick(0, MAX_ARM_Row, 0) == false,
          "[3] CanInNozzlePick rejects iRow>=MAX_ARM_Row (golden :42)");
    CHECK(SiteUseMgr.CanInNozzlePick(0, 0, MAX_ARM_Col) == false,
          "[3] CanInNozzlePick rejects iCol>=MAX_ARM_Col (golden :43)");
    CHECK(SiteUseMgr.GetActiveInNozzleCount(0) == 2,
          "[3] GetActiveInNozzleCount(0)==2 over the canonical fixture (golden :47)");
    CHECK(SiteUseMgr.GetActiveInNozzleCount(1) == 1,
          "[3] GetActiveInNozzleCount(1)==1 -- per-shuttle, not a shared total");
    CHECK(SiteUseMgr.GetActiveInNozzleCount(9) == 0,
          "[3] GetActiveInNozzleCount rejects iSht>1 with 0 (golden :49)");

    // =======================================================================
    // [4] OutArm nozzle queries (golden :60-75)
    // =======================================================================
    CHECK(SiteUseMgr.CanOutNozzlePick(1, 3) == true,
          "[4] CanOutNozzlePick(1,3) reads Prod.fOutArmSuckUse[1][3] (golden :64)");
    CHECK(SiteUseMgr.CanOutNozzlePick(0, 0) == false,
          "[4] CanOutNozzlePick(0,0) reads a false cell -- OutArm map is NOT the InArm one");
    CHECK(SiteUseMgr.CanOutNozzlePick(-1, 0) == false,
          "[4] CanOutNozzlePick rejects iRow<0 (golden :62)");
    CHECK(SiteUseMgr.CanOutNozzlePick(0, MAX_ARM_Col) == false,
          "[4] CanOutNozzlePick rejects iCol>=MAX_ARM_Col (golden :63)");
    CHECK(SiteUseMgr.GetActiveOutNozzleCount() == 1,
          "[4] GetActiveOutNozzleCount()==1 (golden :67)");

    // =======================================================================
    // [5] HotPlate capacity / occupancy (golden :79-126) + QUIRK Q8
    // =======================================================================
    setHP(4, 4, 0x01);
    fillPlate(0, NULL_IC);
    fillPlate(1, NULL_IC);
    MOT[MMPlate1 + 0].Tray.Data[0][0] = 4;
    MOT[MMPlate1 + 0].Tray.Data[1][0] = 4;
    MOT[MMPlate1 + 0].Tray.Data[3][2] = 4;
    // A cell OUTSIDE the 4x4 division must NOT be counted -- pins that the loop
    // bound is XDivision/YDivision and not the raw array size.
    MOT[MMPlate1 + 0].Tray.Data[7][7] = 4;

    CHECK(SiteUseMgr.GetHPCapacity() == 16,
          "[5] GetHPCapacity()==XDivision*YDivision==16 (golden :100)");
    CHECK(SiteUseMgr.GetHPOccupiedCount(0) == 3,
          "[5] GetHPOccupiedCount(0)==3; the [7][7] cell outside the division is ignored (golden :85-88)");
    CHECK(SiteUseMgr.GetHPOccupiedCount(1) == 0,
          "[5] GetHPOccupiedCount(1)==0 -- plate 1 is MOT[MMPlate1+1], a different tray");
    CHECK(SiteUseMgr.GetHPOccupiedCount(2) == 0,
          "[5] GetHPOccupiedCount rejects iPlate>1 with 0 (golden :81)");
    CHECK(SiteUseMgr.GetHPAvailableCount(0) == 13,
          "[5] GetHPAvailableCount(0)==16-3 (golden :95)");
    CHECK(SiteUseMgr.GetHPAvailableCount(-1) == 0,
          "[5] GetHPAvailableCount rejects iPlate<0 with 0 (golden :94)");
    CHECK(SiteUseMgr.HPHasSpace(0, 13) == true,
          "[5] HPHasSpace(0,13)==true (>= is inclusive, golden :106)");
    CHECK(SiteUseMgr.HPHasSpace(0, 14) == false,
          "[5] HPHasSpace(0,14)==false");
    CHECK(SiteUseMgr.HPHasSpace(99, 0) == true,
          "[5] GOLDEN QUIRK Q8: the iNeedCells<=0 early-out fires BEFORE the plate "
          "range check, so a bogus plate answers true (golden :105)");
    CHECK(SiteUseMgr.HPHasSpace(99, 1) == false,
          "[5] Q8 counterpart: with iNeedCells>0 the bogus plate answers false");
    CHECK(SiteUseMgr.WillExceedHPBoundary(0, 13) == false,
          "[5] WillExceedHPBoundary(0,13)==false (13 < 13 is false, golden :125)");
    CHECK(SiteUseMgr.WillExceedHPBoundary(0, 14) == true,
          "[5] WillExceedHPBoundary(0,14)==true");
    CHECK(SiteUseMgr.WillExceedHPBoundary(5, 0) == true,
          "[5] WillExceedHPBoundary fails SAFE (true) on a bogus plate (golden :124)");

    // =======================================================================
    // [6] Combined nozzle+HP (golden :111-119)
    // =======================================================================
    CHECK(SiteUseMgr.GetPlaceableCountPerCycle(0) == 1,
          "[6] GetPlaceableCountPerCycle(0)==1: it is fInArmSuckUse AND bZFlgToHP, "
          "NOT the nozzle count (which is 2 here) -- golden :117");
    CHECK(SiteUseMgr.GetPlaceableCountPerCycle(1) == 1,
          "[6] GetPlaceableCountPerCycle(1)==1 over SHT1's own slice");
    bZFlgToHP[0][0] = false;
    CHECK(SiteUseMgr.GetPlaceableCountPerCycle(0) == 0,
          "[6] clearing bZFlgToHP[0][0] drops the placeable count to 0 while the "
          "nozzle count stays 2 -- the AND is load-bearing");
    bZFlgToHP[0][0] = true;
    CHECK(SiteUseMgr.GetPlaceableCountPerCycle(7) == 0,
          "[6] GetPlaceableCountPerCycle rejects iSht>1 with 0 (golden :113)");

    // =======================================================================
    // [7] Debug dumps -- EXACT strings (golden :130-170)
    // =======================================================================
    CHECK(SiteUseMgr.DumpInNozzleMap(0) == AnsiString("InArm SuckUse[0]: R0[1,1,0,0] R1[0,0,0,0] "),
          "[7] DumpInNozzleMap(0) exact text incl. the trailing space (golden :133-144)");
    CHECK(SiteUseMgr.DumpInNozzleMap(1) == AnsiString("InArm SuckUse[1]: R0[1,0,0,0] R1[0,0,0,0] "),
          "[7] DumpInNozzleMap(1) renders SHT1's own slice");
    CHECK(SiteUseMgr.DumpInNozzleMap(2) == AnsiString("Invalid sht"),
          "[7] DumpInNozzleMap rejects iSht>1 with 'Invalid sht' (golden :132)");
    CHECK(SiteUseMgr.DumpOutNozzleMap() == AnsiString("OutArm SuckUse: R0[0,0,0,0] R1[0,0,0,1] "),
          "[7] DumpOutNozzleMap exact text (golden :149-160)");
    CHECK(SiteUseMgr.DumpHPStatus(0) == AnsiString("HP1: 3/16 (avail=13)"),
          "[7] DumpHPStatus(0) exact text -- plate index 0 prints as HP1 (golden :166-168)");
    CHECK(SiteUseMgr.DumpHPStatus(1) == AnsiString("HP2: 0/16 (avail=16)"),
          "[7] DumpHPStatus(1) exact text");
    CHECK(SiteUseMgr.DumpHPStatus(4) == AnsiString("Invalid plate"),
          "[7] DumpHPStatus rejects iPlate>1 (golden :165)");

    // =======================================================================
    // [8] ShadowLogBeforeSearch -- LOG IS THE ONLY OBSERVABLE (golden :173-186)
    //     Fixture: plate0 occ=3/16, plate1 occ=16/16, nz0=2 nz1=1 placeable=1.
    // =======================================================================
    fillPlate(1, 4);                 // plate 1 completely occupied
    {
        capBegin();
        SiteUseMgr.ShadowLogBeforeSearch(0);
        AnsiString log = capEnd();
        CHECK(has(log, "BeforeSearch HP1 occ=3 avail=13 cap=16 nz0=2 nz1=1 placeable=1"),
              "[8] ShadowLogBeforeSearch(0) emits the EXACT golden line (golden :182-183); "
              "the OLD SHIM emitted nothing at all");

        capBegin();
        SiteUseMgr.ShadowLogBeforeSearch(1);
        log = capEnd();
        CHECK(has(log, "BeforeSearch HP2 occ=16 avail=0 cap=16 nz0=2 nz1=1 placeable=1"),
              "[8] ShadowLogBeforeSearch(1) reports plate 1's own occupancy");

        capBegin();
        SiteUseMgr.ShadowLogBeforeSearch(7);
        log = capEnd();
        CHECK(has(log, "BeforeSearch HP8 occ=0 avail=0 cap=16"),
              "[8] GOLDEN QUIRK Q9: no plate-range check -- a bogus plate logs the "
              "self-contradictory occ=0 avail=0 cap=16 (golden :173-185)");
    }

    // =======================================================================
    // [9] ShadowLogAfterPlace (golden :189-198)
    // =======================================================================
    {
        capBegin();
        SiteUseMgr.ShadowLogAfterPlace(1, 2, 3, 9);
        AnsiString log = capEnd();
        CHECK(has(log, "AfterPlace HP2 [2][3] cellState=9 avail=0"),
              "[9] ShadowLogAfterPlace echoes its args and re-reads the plate's "
              "availability (golden :194-195)");

        capBegin();
        SiteUseMgr.ShadowLogAfterPlace(0, 0, 1, 4);
        log = capEnd();
        CHECK(has(log, "AfterPlace HP1 [0][1] cellState=4 avail=13"),
              "[9] ShadowLogAfterPlace on plate 0 reports avail=13, not plate 1's 0");
    }

    // =======================================================================
    // [10] ValidateSearchResult (golden :201-260) + QUIRKS Q3 / Q4
    //      plate0 is still 3/16 occupied with cells (0,0),(1,0),(3,2) taken.
    // =======================================================================
    {
        // (a) plate index out of range -> single WARN, EARLY RETURN, no summary.
        iPlacePlate[0] = 5; iPlacePlateX[0] = 0; iPlacePlateY[0] = 0;
        capBegin();
        SiteUseMgr.ValidateSearchResult(0);
        AnsiString log = capEnd();
        CHECK(has(log, "WARN plate=5 out of range [0,1] idx=0"),
              "[10a] ValidateSearchResult warns on a bad plate (golden :214)");
        CHECK(!has(log, "Validate idx=0"),
              "[10a] ...and RETURNS immediately, so NO summary line is emitted (golden :216)");

        // (b) both boundaries violated, idx=1 -> two WARNs + HAS_WARN summary.
        //     Q4: nozzle= is ALWAYS SHT0's count (2), never SHT1's (1).
        iPlacePlate[1] = 0; iPlacePlateX[1] = 99; iPlacePlateY[1] = -1;
        capBegin();
        SiteUseMgr.ValidateSearchResult(1);
        log = capEnd();
        CHECK(has(log, "WARN HP1 X=99 out of [0,4) idx=1"),
              "[10b] X-boundary WARN with the real XDivision in the message (golden :225)");
        CHECK(has(log, "WARN HP1 Y=-1 out of [0,4) idx=1"),
              "[10b] Y-boundary WARN (golden :233)");
        CHECK(has(log, "Validate idx=1 HP1 pos=[99][-1] avail=13 nozzle=2 HAS_WARN"),
              "[10b] GOLDEN QUIRK Q4: the summary's nozzle= is GetActiveInNozzleCount(0) "
              "(==2) even for idx=1 whose own shuttle has 1 (golden :253)");

        // (c) in-range but the target cell is OCCUPIED -> 'not empty' WARN.
        iPlacePlate[0] = 0; iPlacePlateX[0] = 1; iPlacePlateY[0] = 0;   // cell == 4
        capBegin();
        SiteUseMgr.ValidateSearchResult(0);
        log = capEnd();
        CHECK(has(log, "WARN HP1 [1][0] not empty cellState=4 idx=0"),
              "[10c] occupied-target WARN reports the actual cell value (golden :244-245)");
        CHECK(has(log, "Validate idx=0 HP1 pos=[1][0] avail=13 nozzle=2 HAS_WARN"),
              "[10c] ...and the summary says HAS_WARN");

        // (d) fully clean -> OK, and the summary is the ONLY line.
        iPlacePlate[0] = 0; iPlacePlateX[0] = 2; iPlacePlateY[0] = 2;   // empty cell
        capBegin();
        SiteUseMgr.ValidateSearchResult(0);
        log = capEnd();
        CHECK(has(log, "Validate idx=0 HP1 pos=[2][2] avail=13 nozzle=2 OK"),
              "[10d] a clean result summarises OK (golden :256-258)");
        CHECK(!has(log, "WARN"),
              "[10d] ...with no WARN line at all");

        // (e) Q3: 'Check 5' compares NOTHING.  Plate 0 is nearly full (avail=1) while
        //     the nozzle count is 2 -- an over-subscription the comment implies would
        //     be caught.  Golden still says OK.  Preserved, and pinned here.
        fillPlate(0, 4);
        MOT[MMPlate1 + 0].Tray.Data[2][2] = NULL_IC;   // keep the target cell legal
        iPlacePlate[0] = 0; iPlacePlateX[0] = 2; iPlacePlateY[0] = 2;
        capBegin();
        SiteUseMgr.ValidateSearchResult(0);
        log = capEnd();
        CHECK(has(log, "Validate idx=0 HP1 pos=[2][2] avail=1 nozzle=2 OK"),
              "[10e] GOLDEN QUIRK Q3: avail(1) < nozzle(2) and golden STILL says OK -- "
              "'Check 5' computes both values and compares neither (golden :251-258)");

        CHECK(SiteUseMgr.GetHPOccupiedCount(0) == 15,
              "[10e] sanity: the fixture really did leave plate 0 at 15/16");
    }

    // =======================================================================
    // [11] WillExceedMotorLimit -- QUIRK Q5 (golden :410-413)
    // =======================================================================
    CHECK(SiteUseMgr.WillExceedMotorLimit(0, 0, 0) == false,
          "[11] WillExceedMotorLimit(0,0,0)==false");
    CHECK(SiteUseMgr.WillExceedMotorLimit(-7, 9999, -9999) == false,
          "[11] GOLDEN QUIRK Q5: WillExceedMotorLimit ignores ALL THREE arguments and "
          "returns a constant false -- golden's deliberate disable (golden :412)");

    // =======================================================================
    // [12] CanAnyNozzleReachHP (golden :416-437) + QUIRK Q10
    // =======================================================================
    setHP(4, 4, 0x01);
    fillPlate(0, NULL_IC);
    fillPlate(1, NULL_IC);
    CHECK(SiteUseMgr.CanAnyNozzleReachHP() == true,
          "[12] empty plate 0 -> true");

    fillPlate(0, 4);
    CHECK(SiteUseMgr.CanAnyNozzleReachHP() == false,
          "[12] FULL plate 0, single-plate select -> false; the OLD SHIM answered "
          "true unconditionally (golden :436)");

    // plate 1 is empty, but iPlateSelect != 0x03 so plate 1 is never scanned.
    CHECK(SiteUseMgr.CanAnyNozzleReachHP() == false,
          "[12] GOLDEN QUIRK Q10: plate 1 is wide open yet unreachable, because "
          "'dual plate' is recognised ONLY for iPlateSelect==0x03 (golden :420)");

    setHP(4, 4, 0x03);
    CHECK(SiteUseMgr.CanAnyNozzleReachHP() == true,
          "[12] with iPlateSelect==0x03 the empty plate 1 IS found");

    MOT[MMPlate1 + 0].Tray.Data[3][3] = NULL_IC;
    fillPlate(1, 4);
    CHECK(SiteUseMgr.CanAnyNozzleReachHP() == true,
          "[12] a SINGLE empty cell anywhere on plate 0 is enough");
    MOT[MMPlate1 + 0].Tray.Data[3][3] = 4;
    CHECK(SiteUseMgr.CanAnyNozzleReachHP() == false,
          "[12] ...and re-filling that one cell flips it back to false");

    // =======================================================================
    // [13] CompactSearchPlateToPlace -- the real scan (golden :284-401)
    //      Every case asserts the (plate,x,y) WRITTEN INTO iPlacePlate/X/Y, which a
    //      constant `return true` cannot produce.
    // =======================================================================
    clearAutoCleanSkip();

    // (a) SCAN ORDER: Y outer, X inner (golden :340-342).  Plate 0 has row y=0
    //     full and (0,1),(1,1) taken, so the first free cell in Y-outer order is
    //     (x=2,y=1).  An X-outer scan would have answered (x=0,y=2) instead.
    setHP(4, 4, 0x01);
    fillPlate(0, NULL_IC);
    fillPlate(1, NULL_IC);
    {
        int x;
        for (x = 0; x < 4; ++x) MOT[MMPlate1 + 0].Tray.Data[x][0] = 4;
    }
    MOT[MMPlate1 + 0].Tray.Data[0][1] = 4;
    MOT[MMPlate1 + 0].Tray.Data[1][1] = 4;
    iPlacePlate[0] = 0; iPlacePlateX[0] = -1; iPlacePlateY[0] = -1;
    {
        capBegin();
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        AnsiString log = capEnd();
        CHECK(ok == true, "[13a] partial plate -> true");
        CHECK(iPlacePlate[0] == 0 && iPlacePlateX[0] == 2 && iPlacePlateY[0] == 1,
              "[13a] writes (plate=0,x=2,y=1): Y-OUTER/X-INNER scan order "
              "(an X-outer scan would have written x=0,y=2) -- golden :340-342/:371-373");
        CHECK(has(log, "CompactSearch OK HP1 [2][1] need=1x1"),
              "[13a] logs the hit with need=1x1 -- QUIRK Q1: the block size is "
              "hard-pinned to 1x1 (golden :301-302/:376-377)");
    }

    // (b) FULL single plate -> false, and DumpCompactSearchFailure runs.
    fillPlate(0, 4);
    fillPlate(1, NULL_IC);
    iPlacePlate[0] = 0; iPlacePlateX[0] = 7; iPlacePlateY[0] = 7;
    {
        capBegin();
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        AnsiString log = capEnd();
        CHECK(ok == false,
              "[13b] FULL plate, single-plate select -> false; the OLD SHIM answered "
              "true unconditionally (golden :400)");
        CHECK(iPlacePlate[0] == 0 && iPlacePlateX[0] == 7 && iPlacePlateY[0] == 7,
              "[13b] on failure the globals are left UNTOUCHED (golden only writes "
              "them inside the bFit arm, :371-373)");
        CHECK(has(log, "CompactSearch FULL no space need=1x1"),
              "[13b] logs the FULL line (golden :391)");
        // -- and the diagnostic dump that golden chains off it (golden :395) --
        CHECK(has(log, "FAIL PARAMS iInArmType=7 iPickRow=1 iPickCol=4"),
              "[13b] DumpCompactSearchFailure section 1 echoes the arm topology (golden :449)");
        CHECK(has(log, "XDiv=4 YDiv=4 ProdXPitch=5000 ProdYPitch=4000 plateSelect=0x01 dual=0 startPlate=0 cleanSkip=0 pitchOver=0"),
              "[13b] section 2 echoes the HP config from Prod.HotPlateForm[0], "
              "NOT the double-typed global (golden :456-459)");
        CHECK(has(log, "FAIL MOTOR baseX0=11000 baseX1=12000 baseY0=21000 baseY1=22000 pitchSpan=12000"),
              "[13b] section 2b echoes the InArm pick bases + iXpitchMaxX3 (golden :468-469)");
        CHECK(has(log, "FAIL MOTOR limXP=120000 limXN=-120000 limYP=90000 limYN=-90000"),
              "[13b] section 2b derefs MOT[MInArmX/Y].Motor->PSoftLimit* unguarded (golden :471-473)");
        CHECK(has(log, "FAIL MOTOR P0 col=3 tgtX=26000 tgtY=21000 exceed=0"),
              "[13b] GOLDEN QUIRKS Q6+Q7: the loop dumps EVERY column (not just 0 and 7 "
              "as its comment claims) and tgtY subtracts iYPitch*0, i.e. nothing, so "
              "tgtY==baseY0 (golden :475-482)");
        CHECK(has(log, "FAIL POS plate=0 ix=7 iy=7 iPlaceHP=0"),
              "[13b] section 3 echoes the untouched search globals (golden :487-488)");
        CHECK(has(log, "FAIL HP1 occ=16 avail=0 empty=0 motorSkip=0 cap=16"),
              "[13b] section 4: motorSkip is structurally 0 because Q5 disabled the "
              "limit check (golden :506-512)");
        CHECK(has(log, "FAIL GRID HP1 Y00 [XXXX]"),
              "[13b] section 5 renders a full row of occupied cells as XXXX (golden :538)");
        CHECK(!has(log, "FAIL GRID HP2"),
              "[13b] ...and only plate 0 is dumped, because dual==0 (golden :518)");
        CHECK(has(log, "FAIL SUCK Item[0,0,4,0|0,0,0,0]"),
              "[13b] section 6 renders InArmSuck.Item row-major with '|' between rows "
              "(golden :544-554)");
    }

    // (c) DUAL PLATE: plate 0 full, plate 1 has exactly one hole at (3,2).
    setHP(4, 4, 0x03);
    fillPlate(0, 4);
    fillPlate(1, 4);
    MOT[MMPlate1 + 1].Tray.Data[3][2] = NULL_IC;
    iPlacePlate[0] = 0; iPlacePlateX[0] = -1; iPlacePlateY[0] = -1;
    {
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        CHECK(ok == true, "[13c] dual plate: the hole on plate 1 is found");
        CHECK(iPlacePlate[0] == 1 && iPlacePlateX[0] == 3 && iPlacePlateY[0] == 2,
              "[13c] writes (plate=1,x=3,y=2) -- the second plate really is scanned "
              "when iPlateSelect==0x03 (golden :333-337)");
    }

    // (d) QUIRK Q2: the START plate is read from the *previous* iPlacePlate[0], and
    //     the next try wraps with `% 2`.  plate 1 full, plate 0 has a hole at (1,0);
    //     starting at 1 must try 1 first (miss) then (1+1)%2==0 (hit).
    fillPlate(0, 4);
    fillPlate(1, 4);
    MOT[MMPlate1 + 0].Tray.Data[1][0] = NULL_IC;
    iPlacePlate[0] = 1; iPlacePlateX[0] = -1; iPlacePlateY[0] = -1;
    {
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        CHECK(ok == true, "[13d] dual plate starting at 1 still finds plate 0's hole");
        CHECK(iPlacePlate[0] == 0 && iPlacePlateX[0] == 1 && iPlacePlateY[0] == 0,
              "[13d] GOLDEN QUIRK Q2: iStartPlate is the leftover global and the try "
              "index wraps modulo 2 (golden :332/:337)");
    }

    // (e) QUIRK Q10 on the search side: plate 0 full, plate 1 wide open, but
    //     iPlateSelect==0x02, so only plate 0 is tried and the search FAILS.
    setHP(4, 4, 0x02);
    fillPlate(0, 4);
    fillPlate(1, NULL_IC);
    iPlacePlate[0] = 0; iPlacePlateX[0] = -1; iPlacePlateY[0] = -1;
    {
        capBegin();
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        AnsiString log = capEnd();
        CHECK(ok == false,
              "[13e] GOLDEN QUIRK Q10: iPlateSelect==0x02 is NOT treated as dual, so a "
              "wide-open plate 1 is never tried and the search fails (golden :333)");
        CHECK(has(log, "plateSelect=0x02 dual=0"),
              "[13e] ...and the diagnostic dump records exactly that");
    }

    // (f) AUTOCLEAN reserved zone: on plate 0, rows y<4 are skipped when
    //     iAutoClean_Tray==eCKPos_HP2 AND iAutoClean_Function==1 (golden :325-328/:345).
    setHP(4, 8, 0x01);
    fillPlate(0, NULL_IC);
    fillPlate(1, NULL_IC);
    TestIF_File.iAutoClean_Tray     = eCKPos_HP2;
    TestIF_File.iAutoClean_Function = 1;
    iPlacePlate[0] = 0; iPlacePlateX[0] = -1; iPlacePlateY[0] = -1;
    {
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        CHECK(ok == true, "[13f] autoclean skip still finds space below the reserved rows");
        CHECK(iPlacePlate[0] == 0 && iPlacePlateX[0] == 0 && iPlacePlateY[0] == 4,
              "[13f] the first FOUR y rows of plate 0 are reserved, so the hit is y=4 "
              "and not y=0 (golden :328/:345)");
    }

    // (g) the same reserved zone does NOT apply to plate 1 (golden :345 tests iP==0).
    iPlacePlate[0] = 1; iPlacePlateX[0] = -1; iPlacePlateY[0] = -1;
    {
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        CHECK(ok == true, "[13g] plate 1 search succeeds");
        CHECK(iPlacePlate[0] == 1 && iPlacePlateX[0] == 0 && iPlacePlateY[0] == 0,
              "[13g] on plate 1 the AutoClean skip is NOT applied, so y=0 wins "
              "(golden :345 guards on iP==0)");
    }

    // (h) and with the AutoClean gate switched off, plate 0 goes back to y=0 --
    //     which proves (f) measured the gate and not the board.
    clearAutoCleanSkip();
    iPlacePlate[0] = 0; iPlacePlateX[0] = -1; iPlacePlateY[0] = -1;
    {
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        CHECK(ok == true, "[13h] gate off: search succeeds");
        CHECK(iPlacePlate[0] == 0 && iPlacePlateX[0] == 0 && iPlacePlateY[0] == 0,
              "[13h] gate off -> the hit moves back to y=0 (control for [13f])");
    }

    // (i) QUIRK Q1 again, from the other side: bPitchOver12000 changes NOTHING,
    //     because iNeedRow/iNeedCol were already pinned to 1 (golden :304-309).
    bPitchOver12000 = true;
    iPlacePlate[0] = 0; iPlacePlateX[0] = -1; iPlacePlateY[0] = -1;
    {
        capBegin();
        bool ok = SiteUseMgr.CompactSearchPlateToPlace();
        AnsiString log = capEnd();
        CHECK(ok == true && iPlacePlate[0] == 0 && iPlacePlateX[0] == 0 && iPlacePlateY[0] == 0,
              "[13i] GOLDEN QUIRK Q1: bPitchOver12000 is DEAD CODE -- identical result to [13h]");
        CHECK(has(log, "need=1x1"),
              "[13i] ...and the block size is still 1x1");
    }
    bPitchOver12000 = false;

    // =======================================================================
    // [14] DumpCompactSearchFailure called DIRECTLY (golden :440-556), so the
    //      'C' (AutoClean-reserved) grid glyph gets covered -- CompactSearch can
    //      never reach the dump with cleanSkip>0 on a board that also has a hole.
    // =======================================================================
    setHP(4, 6, 0x03);
    fillPlate(0, NULL_IC);
    fillPlate(1, 4);
    {
        capBegin();
        SiteUseMgr.DumpCompactSearchFailure(1, 1, 1, /*iAutoCleanYSkip=*/2,
                                            /*bDualPlate=*/true, /*iStartPlate=*/1);
        AnsiString log = capEnd();
        CHECK(has(log, "FAIL GRID HP1 Y00 [CCCC]"),
              "[14] rows below iAutoCleanYSkip on plate 0 render as 'C' (golden :530-531)");
        CHECK(has(log, "FAIL GRID HP1 Y02 [....]"),
              "[14] rows at/after the skip render as '.' (golden :533)");
        CHECK(has(log, "FAIL GRID HP2 Y00 [XXXX]"),
              "[14] with bDualPlate=true the SECOND plate is dumped too (golden :518)");
        CHECK(!has(log, "M]") && !has(log, "[M"),
              "[14] GOLDEN QUIRK Q5 corollary: the 'M' (motor-limited) glyph is "
              "structurally unreachable (golden :528-529)");
        CHECK(has(log, "dual=1 startPlate=1 cleanSkip=2"),
              "[14] the caller-supplied dump arguments are echoed verbatim (golden :456-459)");
        CHECK(has(log, "FAIL HP1 occ=0 avail=24 empty=24 motorSkip=0 cap=24"),
              "[14] section 4 for the empty plate 0 of a 4x6 board");
        CHECK(has(log, "FAIL HP2 occ=24 avail=0 empty=0 motorSkip=0 cap=24"),
              "[14] section 4 for the full plate 1");
    }

    // -----------------------------------------------------------------------
    MOT[MInArmX].Motor = NULL;      // un-install the fixture sim motors
    MOT[MInArmY].Motor = NULL;
    DeleteFile(capPath());

    std::printf("=== %d PASS, %d FAIL (of %d) ===\n", g_pass, g_fail, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
