// tests/test_barcode_bottom2did.cpp
// Verification harness for the BarCode Bottom-2DID (1-CCD) scan translation
// (BarCode/BarCode_Bottom2DID.{h,cpp} -- BarCode_InitBottom2DIDScan /
// BarCode_DoBottom2DIDScan).
//
// No external test framework: a tiny check harness prints PASS/FAIL per case
// and a final summary, and returns non-zero on ANY failure (same style as
// tests/test_ContactForce.cpp).
//
// LIMITATION (stated explicitly): the real CCD hardware path (SendCCDCommand /
// ClientSocket) is gated offline -- there is no way to drive the SM past the
// case-500 "waiting for bGetSE9[]" timeout gate without a live CCD, so this
// harness verifies:
//   (1) the "no real IC" fast-finish path (case 1's early return true),
//   (2) InitBottom2DIDScan's map2DList bClear2DID erase-vs-skip behaviour,
//   (3) CheckWhichKitBottom2DID's exposure-flag computation (via the
//       observable extern globals) as the SM advances case 1->100->200->500
//       with one real IC present, and
//   (4) that the SM does NOT falsely finish while its own internal
//       Bottom2DPosDelay timer is still pending (the CCD-disconnected-offline
//       behaviour is a faithful "keep waiting", not a false positive).
#include "BarCode/BarCode_Bottom2DID.h"
#include "aHotPlateSubstrate.h"   // InArmSuck
#include "cmydef.h"               // HAS_IC / NULL_IC, iNeedBarcodeCount[] ... iBarcodeDuplicate[]
#include "cprod.h"                // TestIF_File
#include "forms/fLotInfo.h"       // fLotInfo->sgBarcode (W-163)
#include <cstdio>
#include <cstring>

static int g_pass = 0;
static int g_fail = 0;

static void check_b(const char* name, bool got, bool expected)
{
    if (got == expected) { printf("PASS  %-58s got=%d\n", name, (int)got); ++g_pass; }
    else { printf("FAIL  %-58s got=%d exp=%d\n", name, (int)got, (int)expected); ++g_fail; }
}

static void check_i(const char* name, int got, int expected)
{
    if (got == expected) { printf("PASS  %-58s got=%d\n", name, got); ++g_pass; }
    else { printf("FAIL  %-58s got=%d exp=%d\n", name, got, expected); ++g_fail; }
}

static void check_s(const char* name, const char* got, const char* expected)
{
    if (std::strcmp(got, expected) == 0) { printf("PASS  %-58s got=\"%s\"\n", name, got); ++g_pass; }
    else { printf("FAIL  %-58s got=\"%s\" exp=\"%s\"\n", name, got, expected); ++g_fail; }
}

// sgBarcode cell [col][row] as text (vclcompat TStringGrid: Cells[c][r] is an AnsiString&).
static const char* cell(int col, int row) { return fLotInfo->sgBarcode->Cells[col][row].c_str(); }

int main()
{
    printf("=== BarCode Bottom2DID (1-CCD scan) translation verification ===\n\n");

    // -------------------------------------------------------------------
    // Test 1: no real IC anywhere -> case 1's `if(InArmSuck.HasRealIC()==
    // false) return true;` fast path.  Fresh process -> InArmSuck.Item[][]
    // is zero-initialised (NULL_IC) by default (static storage duration).
    // -------------------------------------------------------------------
    printf("-- Test 1: no real IC -> instant finish --\n");
    {
        BarCode_InitBottom2DIDScan();
        check_b("first call returns true (nothing to scan)", BarCode_DoBottom2DIDScan(), true);
        check_b("second call still true (Task never advanced past 1)", BarCode_DoBottom2DIDScan(), true);
    }

    // -------------------------------------------------------------------
    // Test 2: InitBottom2DIDScan(bClear2DID) map2DList erase-vs-skip.
    // -------------------------------------------------------------------
    printf("\n-- Test 2: InitBottom2DIDScan bClear2DID map2DList behaviour --\n");
    {
        InArmSuck.iMaxRow = 2;
        InArmSuck.iMaxCol = 4;

        InArmSuck.Item[0][0] = HAS_IC;
        InArmSuck.cDeviceInf[0][0] = "CODE0001";
        map2DList["CODE0001"] = "x";

        BarCode_InitBottom2DIDScan(true);
        check_b("bClear2DID=true erases matching map2DList entry",
                 map2DList.find("CODE0001") == map2DList.end(), true);

        InArmSuck.cDeviceInf[0][0] = "CODE0002";
        map2DList["CODE0002"] = "x";

        BarCode_InitBottom2DIDScan(false);
        check_b("bClear2DID=false leaves map2DList entry untouched",
                 map2DList.find("CODE0002") != map2DList.end(), true);

        // cleanup -- do not leak state into Test 3.
        map2DList.erase("CODE0002");
        InArmSuck.Item[0][0] = NULL_IC;
        InArmSuck.cDeviceInf[0][0] = "";
    }

    // -------------------------------------------------------------------
    // Test 3: one real IC at [0][0] -> SM advances case 1->100->200->500,
    // CheckWhichKitBottom2DID computes the exposure flags correctly, and
    // the SM does not falsely finish while Bottom2DPosDelay is pending.
    // -------------------------------------------------------------------
    printf("\n-- Test 3: real IC at [0][0] drives the SM forward --\n");
    {
        InArmSuck.iMaxRow = 2;
        InArmSuck.iMaxCol = 4;
        InArmSuck.Item[0][0] = HAS_IC;          // only [0][0] has a real IC
        InArmSuck.cDeviceInf[0][0] = "";        // not yet decoded

        BarCode_InitBottom2DIDScan(true);       // Task cursor starts fresh at 1

        bool r1 = BarCode_DoBottom2DIDScan();   // case 1->100->200->500
        check_b("first tick: not finished yet (still scanning)", r1, false);
        check_i("iBottomKit advanced from -1 to 0 (case 100)", iBottomKit, 0);
        check_b("bBottom2DNeedMoveInArm==true (kit 0 has a pending cell)",
                 bBottom2DNeedMoveInArm, true);
        check_b("bCCDBarcodeExposureOK[0]==false ([0][0] real IC, no code yet)",
                 bCCDBarcodeExposureOK[0], false);
        check_b("bCCDBarcodeExposureOK[1]==true ([0][2] empty)",
                 bCCDBarcodeExposureOK[1], true);
        check_b("bCCDBarcodeExposureOK[2]==true ([1][0] empty)",
                 bCCDBarcodeExposureOK[2], true);
        check_b("bCCDBarcodeExposureOK[3]==true ([1][2] empty)",
                 bCCDBarcodeExposureOK[3], true);

        bool r2 = BarCode_DoBottom2DIDScan();   // case 500 again, timer just set
        check_b("second tick: still not finished (Bottom2DPosDelay pending, no false-positive finish)",
                 r2, false);

        // cleanup
        InArmSuck.Item[0][0] = NULL_IC;
        InArmSuck.cDeviceInf[0][0] = "";
    }

    // -------------------------------------------------------------------
    // Test 4 (W-163, POOL-2 BC-A): the single-CCD DoBarcodeCount writes all
    // of golden BarCode.cpp:5854-5858 + :5875 + :5887-5892 into
    // fLotInfo->sgBarcode. The production caller is case 5000 (:1587), which
    // this offline harness cannot reach (case 500 waits for the CCD), so it
    // goes through BarCode_Bottom2DID_DoBarcodeCountForTest(). This is NOT
    // the 8-CCD BarCode_DoBarcodeCount that test_barcode_bottom2did8ccd covers.
    // Every one of the 20 per-station cells gets a distinct non-zero value,
    // so a swapped row/column or an unwritten cell cannot pass by accident.
    // -------------------------------------------------------------------
    printf("\n-- Test 4: single-CCD DoBarcodeCount -> sgBarcode (W-163) --\n");
    {
        check_b("fLotInfo->sgBarcode exists", fLotInfo != 0 && fLotInfo->sgBarcode != 0, true);
        for (int c = 0; c < 6; c++)                    // sentinel: anything not written stays "-"
            for (int r = 0; r < 7; r++)
                fLotInfo->sgBarcode->Cells[c][r] = "-";

        //                     station:   1    2    3    4
        const int need[4]  = {          101, 102, 103, 104 };
        const int pass[4]  = {           91,  92,  93,  94 };
        const int err[4]   = {           10,  11,  12,  13 };
        const int retry[4] = {           31,  32,  33,  34 };
        const int dup[4]   = {           41,  42,  43,  44 };
        for (int i = 0; i < 4; i++)
        {
            iNeedBarcodeCount[i]  = need[i];
            iBarcodePassCount[i]  = pass[i];
            iBarcodeErrorCount[i] = err[i];
            iBarcodeAutoRetry[i]  = retry[i];
            iBarcodeDuplicate[i]  = dup[i];
        }
        TestIF_File.b2DIDYield = true;
        TestIF_File.i2DYieldIgnoreCnt = 5;           // Count1 410 > 5
        TestIF_File.d2DIDYield = 95.0;               // 90.24 < 95 -> alarm
        bool alarm = BarCode_Bottom2DID_DoBarcodeCountForTest();

        char name[96], want[16];
        const int  rowOf[5] = { 1, 2, 3, 5, 6 };      // golden :5854-5858 rows
        const int* valOf[5] = { need, pass, err, retry, dup };
        const char* what[5] = { "Need", "Pass", "Error", "AutoRetry", "Duplicate" };
        for (int i = 0; i < 4; i++)
            for (int k = 0; k < 5; k++)
            {
                std::snprintf(name, sizeof name, "station %d %s -> Cells[%d][%d]", i + 1, what[k], 1 + i, rowOf[k]);
                std::snprintf(want, sizeof want, "%d", valOf[k][i]);
                check_s(name, cell(1 + i, rowOf[k]), want);
            }

        // row 4 (per-station %, golden :5875) and column 5 (totals, :5887-5892)
        check_s("station 1 rate -> Cells[1][4]", cell(1, 4), "90.10");   //  9100/101
        check_s("station 2 rate -> Cells[2][4]", cell(2, 4), "90.20");   //  9200/102
        check_s("station 3 rate -> Cells[3][4]", cell(3, 4), "90.29");   //  9300/103
        check_s("station 4 rate -> Cells[4][4]", cell(4, 4), "90.38");   //  9400/104
        check_s("total Need -> Cells[5][1]",      cell(5, 1), "410");
        check_s("total Pass -> Cells[5][2]",      cell(5, 2), "370");
        check_s("total Error -> Cells[5][3]",     cell(5, 3), "46");
        check_s("total yield -> Cells[5][4]",     cell(5, 4), "90.24");  // 37000/410
        check_s("total AutoRetry -> Cells[5][5]", cell(5, 5), "130");
        check_s("total Duplicate -> Cells[5][6]", cell(5, 6), "170");
        check_s("s2DIDYield == total yield cell", s2DIDYield.c_str(), cell(5, 4));
        check_b("yield alarm: 90.24 < 95 -> true", alarm, true);
        TestIF_File.d2DIDYield = 90.0;
        check_b("yield alarm: 90.24 >= 90 -> false", BarCode_Bottom2DID_DoBarcodeCountForTest(), false);
        check_s("header row 0 / label column 0 untouched", cell(0, 0), "-");

        // all zero: the divide-by-zero branches give 0.00, not NaN, and the
        // per-station cells really become "0" (they were non-zero above).
        for (int i = 0; i < 4; i++)
        {
            iNeedBarcodeCount[i] = 0; iBarcodePassCount[i] = 0; iBarcodeErrorCount[i] = 0;
            iBarcodeAutoRetry[i] = 0; iBarcodeDuplicate[i] = 0;
        }
        TestIF_File.d2DIDYield = 95.0;
        TestIF_File.i2DYieldIgnoreCnt = 5;
        check_b("all zero, Count1 0 <= ignore 5 -> no alarm", BarCode_Bottom2DID_DoBarcodeCountForTest(), false);
        int zeroBad = 0;
        for (int i = 0; i < 4; i++)
            for (int k = 0; k < 5; k++)
                if (std::strcmp(cell(1 + i, rowOf[k]), "0") != 0) ++zeroBad;
        check_i("all zero: the 20 per-station cells are \"0\"", zeroBad, 0);
        for (int i = 0; i < 4; i++)
        {
            std::snprintf(name, sizeof name, "all zero: station %d rate -> Cells[%d][4]", i + 1, 1 + i);
            check_s(name, cell(1 + i, 4), "0.00");
        }
        check_s("all zero: total yield -> Cells[5][4]", cell(5, 4), "0.00");
        check_s("all zero: total Need -> Cells[5][1]", cell(5, 1), "0");
        TestIF_File.i2DYieldIgnoreCnt = -1;          // 0 > -1, and rate1 0.00 < 95
        check_b("all zero, ignore -1 -> alarm (rate1 is 0, not NaN)", BarCode_Bottom2DID_DoBarcodeCountForTest(), true);

        // cleanup
        TestIF_File.b2DIDYield = false;
        TestIF_File.i2DYieldIgnoreCnt = 0;
        TestIF_File.d2DIDYield = 0.0;
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
