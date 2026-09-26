// ---------------------------------------------------------------------------
//  test_testcategory_paint.cpp -- TfTestCategory: setters -> repaint -> capture
//  buffer (the data behind Data.TestCategory.html's tcat.* tags).
//
//  Steven 20260925 (Data.TestCategory) -- AI(W906-TCAT-WEB).
//  Golden: HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cTestCategory.cpp
//  (":N" below).  Port: ROOT cTestCategory.cpp (ht9045_sm) + forms/
//  fTestCategory.{h,cpp} (ht9045_forms).
//
//  WHY THIS EXISTS: the e2e probe (tools/webprobe/data_testcategory_probe.py)
//  can only see the boot/idle state -- nothing on the web page can start a
//  test cycle.  This suite drives the producer path golden uses
//  (atester_ProcessCount.cpp ProcessShowTestStatus / ProcessStartTestData ->
//  SetTestCateCellINT / SetTestCateCellAS / SetTestingCateCell ->
//  ShowTestCategory -> TStringGrid::Refresh -> OnDrawCell=sgArm1DrawCell) and
//  pins what lands in the capture buffer.
//
//  PARTS
//    1  ctor + dfm geometry (golden :20-29, cTestCategory.dfm)
//    2  boot = golden main.cpp:9882 + :10594 (InitCateCell, Show -> FormShow ->
//       SetShowCateMode + AdjFormData), 2x4_8 mode
//    3  idle paint: headers / blank white data cells / bold inheritance
//    4  producer path: bin, Error-bin alphabet, testing yellow, GetTestResult
//    5  bA09 "X" for a closed site
//    6  By Arm: Arm1/Arm2 headers, per-arm arrays, form Height 192
//    7  NN_1Row (2X4NN): RowCount 2, row letters B/A, row mapping, cols 5+ undrawn
//    8  InitialOK==false: golden :148 returns -> VCL default paint only
//  TOUCHES NO FILE.  Needs the god-stack link group (like test_auto9045).
// ---------------------------------------------------------------------------
#include "forms/fTestCategory.h"
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "vclcompat/LedCore.h"
#include <cstdio>
#include <cstring>

using vclcompat::clWhite;
using vclcompat::clGreen;
using vclcompat::clRed;
using vclcompat::clYellow;
using vclcompat::clBtnFace;

static int g_total = 0;
static int g_fail = 0;

static void check(bool ok, const char *what)
{
    ++g_total;
    if (!ok) ++g_fail;
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", what);
}

static const TColor kClWindow = static_cast<TColor>(0x80000005u);

static bool cellIs(TfTestCategoryGrid *g, int col, int row, const char *text, TColor bg, bool bold)
{
    TfTestCategoryGrid::W906_CellPaint *p = g->W906_Cell(col, row);
    if (p == 0) {
        std::printf("        cell (%d,%d) outside buffer %dx%d\n", col, row, g->W906_PaintCols, g->W906_PaintRows);
        return false;
    }
    const bool ok = std::strcmp(p->Text.c_str(), text) == 0 && p->Brush == bg && p->Bold == bold;
    if (!ok)
        std::printf("        cell (%d,%d) = (\"%s\", 0x%08X, %d)  want (\"%s\", 0x%08X, %d)\n",
                    col, row, p->Text.c_str(), (unsigned)p->Brush, (int)p->Bold, text, (unsigned)bg, (int)bold);
    return ok;
}

int main()
{
    TfTestCategory *f = fTestCategory;
    std::printf("-- 1. ctor + dfm geometry\n");
    check(f != 0 && f->MyStringGD[0] == f->sgArm1 && f->MyStringGD[1] == f->sgArm2 &&
          f->sgArm1->Tag == 0 && f->sgArm2->Tag == 1,
          "golden :20-25 MyStringGD[i]=sgArm1/sgArm2, Tag=i");
    check(f->EdgeWidth == 80 && f->EdgeHeight == 24 && !f->bCateByArm && !f->bShow, "golden :26-29");
    check(f->sgArm1->RowCount == 3 && f->sgArm1->ColCount == 3 && f->sgArm1->FixedRows == 1 &&
          f->sgArm1->FixedCols == 1 && f->sgArm1->DefaultColWidth == 80 && f->sgArm1->Height == 80,
          "dfm: RowCount=3 ColCount=3 DefaultColWidth=80 Height=80; VCL FixedRows/FixedCols=1");
    check(W906_TestCategoryInstall() && (bool)f->sgArm1->OnDrawCell && (bool)f->sgArm2->OnDrawCell,
          "dfm OnDrawCell=sgArm1DrawCell bound on both grids");
    check(!W906_TestCategoryBooted(), "not booted before W906_BootTestCategory()");

    std::printf("-- 2. boot (golden main.cpp:9882 + DoShowUserDefFrom :9170-9178), 2X4_8\n");
    InitialOK = true;
    iTestBinCount = 16;
    IniConfig.bShowTestCate = true;
    IniConfig.iShowCateByArm = 0;
    IniConfig.bA09_ByArmCloseSite = false;
    IniConfig.iI20ErrorBinAlphabet = 2;                      // "E"
    TestIF.iTestMode = _8Site2X4;
    TestIF_File.iTestMode = _8Site2X4;
    TestIF.iShuttleMode = 0;
    W906_BootTestCategory();
    check(W906_TestCategoryBooted() && f->bShow, "booted; Show() -> FormShow -> bShow=true");
    check(f->Height == 105 && f->Width == 269, "SetShowCateMode Height=105 (not by arm); AdjFormData Width=269");
    check(f->sgArm1->RowCount == 3 && f->sgArm1->ColCount == 5 && f->sgArm2->ColCount == 5,
          "AdjFormData 2X4_8: RowCount=3, ColCount=5 (golden :57-70 / :87-98)");
    check(f->sgArm1->ColWidths[0] == 80 && f->sgArm1->ColWidths[4] == 40 && f->sgArm1->RowHeights[2] == 24,
          "AdjFormData ColWidths[0]=EdgeWidth 80, [1..4]=40, RowHeights=24");

    std::printf("-- 3. idle paint\n");
    f->sgArm1->Refresh();
    TfTestCategoryGrid *g1 = f->sgArm1;
    check(cellIs(g1, 0, 0, "Socket 0", clBtnFace, false), "corner = \"Socket 0\" (golden :160, zero-based Tag), not bold");
    check(cellIs(g1, 1, 0, "a", clBtnFace, true) && cellIs(g1, 4, 0, "d", clBtnFace, true), "column headers a..d bold (:203-209)");
    check(cellIs(g1, 0, 1, "A", clBtnFace, true) && cellIs(g1, 0, 2, "B", clBtnFace, true), "row headers A/B bold (:192-197)");
    check(cellIs(g1, 1, 1, "", clWhite, true) && cellIs(g1, 4, 2, "", clWhite, true),
          "data cells after InitCateCell: -1 -> \"\" on clWhite, bold inherited from the row header");

    std::printf("-- 4. producer path (ProcessShowTestStatus / ProcessStartTestData shapes)\n");
    f->SetTestingCateCell(0, 0, 1, clYellow);                // golden atester_ProcessCount.cpp:2012
    f->SetTestCateCellINT(0, 0, 0, 3, clGreen);              // :242 pass, bin 3
    f->SetTestCateCellINT(0, 1, 3, 20, clRed);               // :246 fail, bin >= iTestBinCount
    f->SetTestCateCellAS(0, 1, 2, "-1", clWhite);            // :2016 no IC
    f->ShowTestCategory(0);                                  // :255 -> Refresh
    check(cellIs(g1, 1, 1, "3", clGreen, true), "bin 3 green at row A col a");
    check(cellIs(g1, 2, 1, "", clYellow, true), "testing: colour only, TestResult stays -1 -> \"\" on yellow");
    check(cellIs(g1, 4, 2, "E", clRed, true), "bin 20 >= iTestBinCount 16 -> I20 alphabet 2 = \"E\" (:333-341)");
    check(cellIs(g1, 3, 2, "", clWhite, true), "SetTestCateCellAS \"-1\" -> atoi -1 -> blank white");
    TColor col = 0;
    const int b = f->GetTestResult(0, 0, 0, &col);
    check(b == 3 && col == clGreen, "GetTestResult(0,0,0) = 3 / clGreen (RecordHistroy's HistroyBin source)");
    IniConfig.iI20ErrorBinAlphabet = 1;
    g1->Refresh();
    check(cellIs(g1, 4, 2, "16", clRed, true), "I20 alphabet 1 -> AnsiString(iTestBinCount) = \"16\"");

    std::printf("-- 5. bA09 closed site\n");
    IniConfig.bA09_ByArmCloseSite = true;
    for (int i = 0; i < 2; ++i) for (int r = 0; r < 4; ++r) for (int c = 0; c < 8; ++c) LastSet.bUseTestSocket[i][r][c] = true;
    LastSet.bUseTestSocket[0][0][2] = false;                 // Tag 0, row A, col c
    g1->Refresh();
    check(cellIs(g1, 3, 1, "X", clWhite, true), "closed site with TestResult<0 -> \"X\" (:322-327)");
    check(cellIs(g1, 1, 1, "3", clGreen, true), "a site with a result is not overwritten by X");
    IniConfig.bA09_ByArmCloseSite = false;

    std::printf("-- 6. By Arm\n");
    IniConfig.iShowCateByArm = 1;
    f->SetShowCateMode();
    f->SetTestCateCellINT(1, 0, 0, 5, clGreen);              // arm 2 -> TestResult[1][0][0]
    f->ShowTestCategory(1);
    f->sgArm1->Refresh();
    f->sgArm2->Refresh();
    check(f->bCateByArm && f->Height == 192, "SetShowCateMode: bCateByArm, Height=192");
    check(cellIs(f->sgArm2, 0, 0, "Arm2", clBtnFace, false) && cellIs(g1, 0, 0, "Arm1", clBtnFace, false),
          "corner = ArmStr[Tag] (golden :15 / :162)");
    check(cellIs(f->sgArm2, 1, 1, "5", clGreen, true), "sgArm2 paints TestResult[1] (by arm)");
    check(cellIs(g1, 1, 1, "3", clGreen, true), "sgArm1 still paints TestResult[0]");
    IniConfig.iShowCateByArm = 0;
    f->SetShowCateMode();

    std::printf("-- 7. NN_1Row (2X4NN)\n");
    TestIF.iTestMode = _8Site2X4N;
    TestIF_File.iTestMode = _8Site2X4N;
    f->InitCateCell();
    f->AdjFormData();
    check(g1->RowCount == 2 && g1->ColCount == 5, "AdjFormData: IsNNMode()==NN_1Row -> RowCount 2, 2X4NN ColCount 5");
    f->SetTestCateCellINT(0, 1, 0, 7, clGreen);              // NN_1Row: Arm forced 0; not by arm -> [0][1][0]
    f->SetTestCateCellINT(1, 0, 1, 9, clRed);                //                               -> [0][0][1]
    g1->Refresh();
    f->sgArm2->Refresh();
    check(cellIs(g1, 0, 1, "B", clBtnFace, true) && cellIs(f->sgArm2, 0, 1, "A", clBtnFace, true),
          "row header: sgArm1 'B', sgArm2 'A' (:178-189)");
    check(cellIs(g1, 1, 1, "7", clGreen, true), "sgArm1 (Tag 0) paints TestResult[0][1][*] (:263-268)");
    check(cellIs(f->sgArm2, 2, 1, "9", clRed, true), "sgArm2 (Tag 1) paints TestResult[0][0][*]");
    TestIF.iTestMode = _10Site2X5;
    TestIF_File.iTestMode = _10Site2X5;
    f->AdjFormData();
    TestIF_File.iTestMode = _8Site2X4N;                      // painter asks IsNNMode() (file), layout came from 2X5
    g1->Refresh();
    check(g1->ColCount == 6 && cellIs(g1, 5, 1, "", kClWindow, false),
          "NN_1Row paints only cols 1..4 (:261); col 5 keeps the VCL default (\"\", clWindow, not bold)");
    check(f->Width == 309, "AdjFormData 2X5: Width=309");

    std::printf("-- 8. InitialOK==false\n");
    InitialOK = false;
    g1->Refresh();
    check(cellIs(g1, 0, 0, "", clBtnFace, false) && cellIs(g1, 1, 1, "", kClWindow, false) && !g1->W906_Cell(1, 1)->Drawn,
          "golden :148 returns before drawing -> VCL default paint only");
    InitialOK = true;

    std::printf("\ntest_testcategory_paint: %d checks, %d failure(s)\n", g_total, g_fail);
    return g_fail == 0 ? 0 : 1;
}
