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
#include "cmydef.h"               // HAS_IC / NULL_IC
#include <cstdio>

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

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
