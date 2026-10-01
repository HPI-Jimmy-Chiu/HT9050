// =============================================================================
//  test_inarm_leftside_golden.cpp -- AI(W906-R128) 20261002
//  ctest: InArmLeftSideGolden
//
//  NB2 R128 (NIGHT_REPORT s2 #7): ainarm9045.cpp InArmLeftSideNoIC / InArmLeftSideHasIC now follow golden
//  ainarm9045.cpp:4193-4229 -- only the LEFT 2x2 nozzle cells count (iRow==2, the default), or the left two cells
//  of row 0 (any other iRow).  The old port bodies asked the whole arm (InArmSuck.NoIC() / InArmSuck.HasRealIC());
//  every check marked [R] below is red with them -- the acceptance test for this change is "put the two old bodies
//  back and this test fails".
//
//    A  NoIC: left 2x2 empty, a right-side cell holds an IC          -> true    [R]
//    B  NoIC: one left cell holds an IC                              -> false
//    C  HasIC: left 2x2 all HAS_NULL_IC                              -> true    [R]  (golden: non-zero, HAS_NULL_IC counts)
//    D  HasIC: only Item[0][3] (right side) holds an IC              -> false   [R]; three of the four left cells -> false [R]
//    E  iRow=1: only Item[0][0] / Item[0][1] count                   NoIC true with Item[1][0] full [R];
//                                                                    HasIC false with only Item[0][0] [R]
//    F  GetPlaceToHotPlateSuckCol (ainarm_SearchPlacePlate.cpp:957-961): 2x5_8, X Division 10, column 8, left 2x2 empty
//       and the right nozzles full -> nozzles 2 / 3 [R]; left full -> 0 / 1; column 7 -> 0 / 1
//
//  The head is set to a real 2x4 (InArmSuck.SetItemAmount(2, 4)): the default TMyKitSuck is 1x1, and on it the old bodies
//  only saw Item[0][0], so several [R] checks passed by accident (1002 06:0x mutation run).
//  No IO and no files: only InArmSuck.Item[][] / its size and four globals are written, and all are restored at the end.
// =============================================================================
#include "MachineDefine.h"
#include "cmydef.h"
#include "cprod.h"
#include "mykitsuck.h"
#include "aHotPlateSubstrate.h"
#include "ainarm_SearchPlacePlate.h"
#include <cstdio>
#include <cstring>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static void ClearGrid()
{
    for (int r = 0; r < _MAX_SUCK_ROW_ITEM; ++r)
        for (int c = 0; c < _MAX_SUCK_COL_ITEM; ++c)
            InArmSuck.Item[r][c] = NULL_IC;
}

static void FillRightNozzles()                 // the right half of a 2x4 head: columns 2 and 3 of rows 0 and 1
{
    InArmSuck.Item[0][2] = HAS_IC; InArmSuck.Item[0][3] = HAS_IC;
    InArmSuck.Item[1][2] = HAS_IC; InArmSuck.Item[1][3] = HAS_IC;
}

int main()
{
    int saved[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];
    std::memcpy(saved, InArmSuck.Item, sizeof(saved));
    const int  savedType = iInArmType, savedXDiv = HotPlateForm.XDivision, savedX0 = iPlacePlateX[0];
    const int  savedClose = iCloseSiteModeFor2x8;
    const bool savedAuto = bRunAutoClean;
    const int  savedRows = InArmSuck.iMaxRow, savedCols = InArmSuck.iMaxCol;
    InArmSuck.SetItemAmount(2, 4);                                            // a 2x4 head (8 nozzles), as 2x5_8

    std::printf("PART A/B -- InArmLeftSideNoIC (golden ainarm9045.cpp:4193-4210)\n");
    ClearGrid(); FillRightNozzles();
    CHECK(InArmLeftSideNoIC() == true, "A [R] left 2x2 empty, right nozzles full -> NoIC true (old body: whole arm -> false)");
    ClearGrid(); InArmSuck.Item[1][1] = HAS_IC;
    CHECK(InArmLeftSideNoIC() == false, "B one left cell Item[1][1] holds an IC -> NoIC false");
    ClearGrid();
    CHECK(InArmLeftSideNoIC() == true, "B2 empty arm -> NoIC true");

    std::printf("PART C/D -- InArmLeftSideHasIC (golden ainarm9045.cpp:4212-4229)\n");
    ClearGrid();
    InArmSuck.Item[0][0] = HAS_NULL_IC; InArmSuck.Item[0][1] = HAS_NULL_IC;
    InArmSuck.Item[1][0] = HAS_NULL_IC; InArmSuck.Item[1][1] = HAS_NULL_IC;
    CHECK(InArmLeftSideHasIC() == true, "C [R] left 2x2 all HAS_NULL_IC -> HasIC true (golden tests non-zero; old body: no real IC -> false)");
    ClearGrid(); InArmSuck.Item[0][3] = HAS_IC;
    CHECK(InArmLeftSideHasIC() == false, "D [R] only Item[0][3] (right side) holds an IC -> HasIC false (old body: any real IC -> true)");
    ClearGrid();
    InArmSuck.Item[0][0] = HAS_IC; InArmSuck.Item[0][1] = HAS_IC; InArmSuck.Item[1][0] = HAS_IC;
    CHECK(InArmLeftSideHasIC() == false, "D2 [R] three of the four left cells -> HasIC false (old body: any real IC -> true)");
    InArmSuck.Item[1][1] = HAS_IC;
    CHECK(InArmLeftSideHasIC() == true, "D3 all four left cells -> HasIC true");

    std::printf("PART E -- iRow != 2: only Item[0][0] / Item[0][1] count\n");
    ClearGrid(); InArmSuck.Item[1][0] = HAS_IC; InArmSuck.Item[1][1] = HAS_IC;
    CHECK(InArmLeftSideNoIC(1) == true, "E1 [R] iRow=1, row 1 full, row 0 left empty -> NoIC true");
    ClearGrid(); InArmSuck.Item[0][0] = HAS_IC; InArmSuck.Item[0][1] = HAS_NULL_IC;
    CHECK(InArmLeftSideHasIC(1) == true, "E2 iRow=1, Item[0][0]=HAS_IC, Item[0][1]=HAS_NULL_IC, row 1 empty -> HasIC true");
    ClearGrid(); InArmSuck.Item[0][0] = HAS_IC;
    CHECK(InArmLeftSideHasIC(1) == false, "E3 [R] iRow=1, only Item[0][0] -> HasIC false (old body: any real IC -> true)");

    std::printf("PART F -- GetPlaceToHotPlateSuckCol, X Division 10 (ainarm_SearchPlacePlate.cpp:957-961)\n");
    iInArmType = e9045_2x5_8;                  // not 1x1 / AxEx / AxxG -> the plain branch
    iCloseSiteModeFor2x8 = -1;                 // neither e2x8Run2x2_13 nor _14
    bRunAutoClean = false;
    HotPlateForm.XDivision = 10;
    iPlacePlateX[0] = 8;
    ClearGrid(); FillRightNozzles();
    CHECK(GetPlaceToHotPlateSuckCol(0) == 2 && GetPlaceToHotPlateSuckCol(1) == 3,
          "F1 [R] column 8, left empty, right full -> nozzles 2 / 3 (old body: 0 / 1, both empty -> nothing placed)");
    ClearGrid();
    InArmSuck.Item[0][0] = HAS_IC; InArmSuck.Item[0][1] = HAS_IC;
    InArmSuck.Item[1][0] = HAS_IC; InArmSuck.Item[1][1] = HAS_IC;
    CHECK(GetPlaceToHotPlateSuckCol(0) == 0 && GetPlaceToHotPlateSuckCol(1) == 1, "F2 column 8, left full -> nozzles 0 / 1");
    iPlacePlateX[0] = 7;
    ClearGrid(); FillRightNozzles();
    CHECK(GetPlaceToHotPlateSuckCol(0) == 0 && GetPlaceToHotPlateSuckCol(1) == 1, "F3 column 7 -> nozzles 0 / 1 (the switch is only at column 8)");

    std::memcpy(InArmSuck.Item, saved, sizeof(saved));
    iInArmType = savedType; HotPlateForm.XDivision = savedXDiv; iPlacePlateX[0] = savedX0;
    iCloseSiteModeFor2x8 = savedClose; bRunAutoClean = savedAuto;
    InArmSuck.SetItemAmount(savedRows, savedCols);

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
