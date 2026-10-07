// =============================================================================
//  test_w6_4_tester_core.cpp  --  W6.4a VERIFY: the RESULT-DECODE ANCHOR
//
//  Translation wave: W6.4a (TESTER/INDEX ENGINE salvage -- atester.cpp)
//  Author: AI(W6.4a-TESTER) 20260626
//
//  PURPOSE
//  -------
//  Companion to test_w6_4_tester.cpp (which pins the test-head SM cursors +
//  ArrayConvertSite/GetRowCol/GetIndexZSpeed geometry).  THIS TU pins the
//  high-value salvaged RESULT-DECODE ANCHOR -- the fMain-free TTL bin-decode
//  trio that is the whole reason the partial atester.cpp was salvaged:
//
//      GetBinaryData     (golden atester.cpp:670, translated :688)  -- ACTIVE
//      GetOneByOneData   (golden atester.cpp:621, translated :639)  -- ACTIVE
//      Test_GetTestResulSub (golden :759, translated :717)          -- ACTIVE
//      Test_GetTestResul (golden :789, translated :747)             -- ACTIVE
//      ArrayConvertSite  (golden :504, translated :516)             -- ACTIVE
//
//  None of these live inside a #if 0 gate; their real bodies are byte-faithful
//  to the golden (verified against D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618
//  \atester.cpp).  The decode reads ONLY pure config (Prod.DIOCfg.iCateBitLength
//  / iCateDataType + TestIF.iTestMode) + the file-scope ScanPort[] bit array --
//  no fMain, no MOT[], no Suck/Socket grid, no tester comms -- so it is fully
//  offline-deterministic.
//
//  CONCRETE ORACLE (from the W6.4a ReadPlan brief; hand-derived from the golden
//  formula, NOT from this implementation):
//    HexData[5]={0x01,0x02,0x04,0x08,0x10}; IOCT[]={4,8,4,9,5,10,4,9}.
//    _5Bit==4 -> IOCT[4]==5; CHBinary==1; CHOneByOne==0; iTestBinCount==16.
//    iTestMode=DualSite2x1 (1x2) -> ArrayConvertSite(0,Y)=Y, so X=0,Y=0 -> iSite=0,
//    Data=iSite*5=0.
//
//    [O1] Binary  _5Bit/CHBinary  iSite=0, ScanPort[0]=1 & ScanPort[2]=1:
//           Sum = 1*0x01 + 0*0x02 + 1*0x04 + 0*0x08 + 0*0x10 = 1+4 = 5
//           -> GetBinaryData(0,0) == 5.                                (golden 706)
//    [O2] OneByOne _5Bit/CHOneByOne iSite=0, ONLY ScanPort[2]=1:
//           exactly one set bit at i=2 -> iResult=2,sum=1 -> iResult+1 = 3
//           -> GetOneByOneData(0,0) == 3.                              (golden 679)
//    [O3] OneByOne _5Bit/CHOneByOne iSite=0, ScanPort[1]=1 & ScanPort[2]=1:
//           sum=2 (!=1) -> returns iTestBinCount == 16
//           -> GetOneByOneData(0,0) == 16.                             (golden 678)
//
//  PLUS the corroborating sub-oracles that lock the decode CONTRACT:
//    * Sum==0 -> GetBinaryData returns -1 (golden 707-708).
//    * iResult==-1 (no bit) -> GetOneByOneData returns -1 (golden 681).
//    * site OFFSET: iSite=1 (X=0,Y=1) -> Data=5; ScanPort[5]&ScanPort[7] set ->
//      Binary == 1+4 == 5 (proves Data=iSite*5 indexing, golden 702/706).
//    * Test_GetTestResulSub dispatch: CHBinary delegates to GetBinaryData,
//      CHOneByOne delegates to GetOneByOneData (golden 737-740) -- driven with a
//      pre-seeded ScanPort[] (SOFT_SIMULTE not defined, so the #ifndef Sen-read
//      block runs first and overwrites ScanPort from the Sim Sen[] HAL; we seed
//      the Sen[] sensors so the read reproduces the same window).
//    * ArrayConvertSite branch table: DualSite2x1 iSite=Y; _8Site2X4 X*4+Y; the
//      decode site-mapping precondition.
//
//  EQUIVALENCE NOTE: no Borland binary exists, so "equivalence" here ==
//  clean g++ compile/link + the decode anchor producing the values hand-derived
//  from the golden formula (the CONCRETE ORACLE above), with the anchor running
//  ACTIVE (not stubbed) over the real Prod/TestIF config + ScanPort[] state.
// =============================================================================
#include "atester.h"               // GetBinaryData / GetOneByOneData / Test_GetTestResul(Sub) / ArrayConvertSite / ScanPort
#include "cprod.h"                  // Prod (DIOCfg.iCateBitLength/iCateDataType) / TestIF (iTestMode)
#include "cmydef.h"                 // _5Bit / CHBinary / CHOneByOne / iTestBinCount / DualSite2x1 / _8Site2X4 / SenBit0..30
#include "cpublic.h"
#include "mysensor.h"              // Sen[] (TMySensor Sim HAL) for the Test_GetTestResulSub Sen-read path
#include "IOBackend.h"             // TSimIOBackend (set Sen[] bit states offline)
#include "MyLaneIo.h"              // MyLaneIO global + SetBackend()
#include <cstdio>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// Clear the whole ScanPort[] window the decode reads (MaxDIO==40 in atester.cpp).
static void ClearScanPort()
{
    for (int i = 0; i < 40; ++i)
        ScanPort[i] = false;
}

int main()
{
    printf("==== W6.4a RESULT-DECODE ANCHOR verify ====\n");
    printf("  HexData={1,2,4,8,16}; IOCT[_5Bit=4]=5; CHBinary=%d CHOneByOne=%d iTestBinCount=%d\n",
           CHBinary, CHOneByOne, iTestBinCount);

    // -----------------------------------------------------------------------
    //  PART 0 -- ArrayConvertSite site-mapping precondition (1x2 -> iSite=Y).
    //  The decode functions key Data off iSite=ArrayConvertSite(X,Y), so pin it.
    // -----------------------------------------------------------------------
    printf("[0] ArrayConvertSite site-map precondition\n");
    TestIF.iTestMode = DualSite2x1;                       // ==4 (1x2): Site=X*2+Y+1, ret X*2+Y
    CHECK(ArrayConvertSite(0, 0) == 0, "P0 ArrayConvertSite(0,0)@1x2 -> iSite=0 (golden 526)");
    CHECK(ArrayConvertSite(0, 1) == 1, "P0 ArrayConvertSite(0,1)@1x2 -> iSite=1 (golden 526)");
    TestIF.iTestMode = _8Site2X4;                         // ==9 (>=_6Site2X3): Site=X*4+Y+1
    CHECK(ArrayConvertSite(1, 2) == 6, "P0 ArrayConvertSite(1,2)@8Site -> X*4+Y=6 (golden 522)");

    // -----------------------------------------------------------------------
    //  Lock the decode config for the CONCRETE ORACLE: _5Bit, iSite=Y (1x2).
    // -----------------------------------------------------------------------
    Prod.DIOCfg.iCateBitLength = _5Bit;                   // ==4 -> IOCT[4]=5, Data=iSite*5
    TestIF.iTestMode           = DualSite2x1;             // 1x2 -> ArrayConvertSite(0,0)=iSite 0

    // =======================================================================
    //  PART A -- GetBinaryData CONCRETE ORACLE  [O1]
    // =======================================================================
    printf("[A] GetBinaryData _5Bit/CHBinary\n");
    Prod.DIOCfg.iCateDataType = CHBinary;                 // ==1 (Test_GetTestResulSub -> GetBinaryData)

    //  [O1] iSite=0, ScanPort[0]=1 & ScanPort[2]=1 -> Sum = 1 + 4 = 5
    ClearScanPort();
    ScanPort[0] = true;
    ScanPort[2] = true;
    {
        int v = GetBinaryData(0, 0);
        printf("    observed GetBinaryData(0,0) [bit0,bit2] = %d (expect 5)\n", v);
        CHECK(v == 5, "O1 GetBinaryData _5Bit iSite0 bit0&bit2 -> 1+4 == 5 (golden 706)");
    }

    //  corroborate: Sum==0 -> -1
    ClearScanPort();
    {
        int v = GetBinaryData(0, 0);
        CHECK(v == -1, "A2 GetBinaryData no bits -> Sum==0 -> -1 (golden 707)");
    }

    //  corroborate FULL weight table: all 5 bits set -> 1+2+4+8+16 == 31
    ClearScanPort();
    for (int i = 0; i < 5; ++i) ScanPort[i] = true;
    {
        int v = GetBinaryData(0, 0);
        CHECK(v == 31, "A3 GetBinaryData all 5 bits -> 1+2+4+8+16 == 31 (golden 706)");
    }

    //  corroborate SITE OFFSET: iSite=1 (X=0,Y=1) -> Data=iSite*5=5; bits at 5&7
    //  -> 0x01+0x04 == 5.  Proves Data=iSite*5 windowing, not a fixed base.
    ClearScanPort();
    ScanPort[5] = true;                                   // window bit 0 for site 1
    ScanPort[7] = true;                                   // window bit 2 for site 1
    {
        int v = GetBinaryData(0, 1);                      // iSite=1
        printf("    observed GetBinaryData(0,1) [bit5,bit7] = %d (expect 5)\n", v);
        CHECK(v == 5, "A4 GetBinaryData iSite1 Data=5 win bit0&bit2 -> 5 (site offset, golden 702/706)");
    }

    // =======================================================================
    //  PART B -- GetOneByOneData CONCRETE ORACLE  [O2] + [O3]
    // =======================================================================
    printf("[B] GetOneByOneData _5Bit/CHOneByOne\n");
    Prod.DIOCfg.iCateDataType = CHOneByOne;               // ==0 (Test_GetTestResulSub -> GetOneByOneData)
    TestIF.iTestMode          = DualSite2x1;              // iSite=0 for (0,0)

    //  [O2] iSite=0, ONLY ScanPort[2]=1 -> exactly one bit at i=2 -> 2+1 == 3
    ClearScanPort();
    ScanPort[2] = true;
    {
        int v = GetOneByOneData(0, 0);
        printf("    observed GetOneByOneData(0,0) [only bit2] = %d (expect 3)\n", v);
        CHECK(v == 3, "O2 GetOneByOneData only bit2 -> idx2+1 == 3 (golden 679)");
    }

    //  [O3] iSite=0, ScanPort[1]=1 & ScanPort[2]=1 -> sum==2 (!=1) -> iTestBinCount
    ClearScanPort();
    ScanPort[1] = true;
    ScanPort[2] = true;
    {
        int v = GetOneByOneData(0, 0);
        printf("    observed GetOneByOneData(0,0) [bit1,bit2] = %d (expect 16)\n", v);
        CHECK(v == iTestBinCount && v == 16,
              "O3 GetOneByOneData bit1&bit2 -> sum!=1 -> iTestBinCount == 16 (golden 678)");
    }

    //  corroborate: no bits set -> iResult==-1 -> -1
    ClearScanPort();
    {
        int v = GetOneByOneData(0, 0);
        CHECK(v == -1, "B3 GetOneByOneData no bits -> -1 (golden 681)");
    }

    //  corroborate: only bit0 -> idx0+1 == 1 (low-edge of the window)
    ClearScanPort();
    ScanPort[0] = true;
    {
        int v = GetOneByOneData(0, 0);
        CHECK(v == 1, "B4 GetOneByOneData only bit0 -> idx0+1 == 1 (golden 679)");
    }

    // =======================================================================
    //  PART C -- Test_GetTestResulSub DISPATCH over the Sim HAL (golden 717-740).
    //  SOFT_SIMULTE is NOT defined, so the #ifndef block REPOPULATES ScanPort[]
    //  from the Sim HAL Sen[] (Sen[SenBit0+i].Status(), i<10) BEFORE dispatching.
    //  We map the 5 window sensors Sen[SenBit0+0..4] to known (Ring,IP,Port,Bit)
    //  addresses on an injected TSimIOBackend (Type=1 active-high so Status()
    //  returns the raw bit), DISABLE every other sensor the loop reads (i=5..9
    //  of SenBit0 + all of SenBit10/20/30 -> Status()==false, Enable==false path
    //  mysensor.cpp:82-86), drive the bits, then assert the dispatch routes
    //  CHBinary->GetBinaryData and CHOneByOne->GetOneByOneData.
    // =======================================================================
    printf("[C] Test_GetTestResulSub dispatch over Sim HAL (Sen[] repopulate, golden 722/737)\n");
    {
        // Inject a fresh offline Sim IO backend; TLaneIO reads Sen[] through it.
        static TSimIOBackend simIO;
        MyLaneIO.SetBackend(&simIO);

        // The bit blocks the Test_GetTestResulSub loop reads (i=0..9 each).
        const int kBlocks[4] = { SenBit0, SenBit10, SenBit20, SenBit30 };

        // (a) DISABLE every sensor the loop touches -> Status()==false (no pollution).
        for (int b = 0; b < 4; ++b)
            for (int i = 0; i < 10; ++i)
                Sen[kBlocks[b] + i].Enable = false;

        // (b) Map the 5 active window sensors Sen[SenBit0+0..4] to mBit[0][1][1] bits.
        //     Ring=0,IP=1,Port=1 keeps (Ring<=0&&IP<=0&&Port<=0&&Bit<=0) FALSE
        //     (MyLaneIo.cpp:425 guard) and CheckPortRangeErr==0 (InitialOK==false).
        for (int i = 0; i < 5; ++i) {
            TMySensor &s = Sen[SenBit0 + i];
            s.Enable  = true;
            s.Type    = 1;            // active-high: Status() returns the raw read
            s.ISABase = eMotionNet;   // == 0 -> MyLaneIO.IOInputBit path
            s.Ring = 0; s.IP = 1; s.Port = 1; s.Bit = i;
        }

        TestIF.iTestMode           = DualSite2x1;   // iSite=0 for (0,0)
        Prod.DIOCfg.iCateBitLength = _5Bit;

        // Helper: drive window bit i high/low through the SAME injected backend.
        // (lambda-free: a local struct is BCB6-style, but this is the C++17 test TU)
        // ---- C1: CHBinary dispatch: window bit0 & bit2 high -> 1+4 == 5 ----
        Prod.DIOCfg.iCateDataType = CHBinary;
        simIO.WriteBit(0, 1, 1, 0, 1);
        simIO.WriteBit(0, 1, 1, 1, 0);
        simIO.WriteBit(0, 1, 1, 2, 1);
        simIO.WriteBit(0, 1, 1, 3, 0);
        simIO.WriteBit(0, 1, 1, 4, 0);
        // sanity: the Sen-read window the loop will latch into ScanPort[0..4]
        CHECK(Sen[SenBit0 + 0].Status() == true  && Sen[SenBit0 + 1].Status() == false &&
              Sen[SenBit0 + 2].Status() == true  && Sen[SenBit0 + 3].Status() == false &&
              Sen[SenBit0 + 4].Status() == false,
              "C0 Sim HAL Sen[SenBit0+0..4] reflect {1,0,1,0,0} (Type=1 active-high)");
        {
            int v = Test_GetTestResulSub(0, 0);
            printf("    observed Test_GetTestResulSub(0,0) CHBinary = %d (expect 5)\n", v);
            CHECK(v == 5, "C1 Test_GetTestResulSub CHBinary -> GetBinaryData == 5 (Sen-read dispatch, golden 740)");
        }

        // ---- C2: CHOneByOne dispatch: only window bit2 high -> idx2+1 == 3 -
        Prod.DIOCfg.iCateDataType = CHOneByOne;
        simIO.WriteBit(0, 1, 1, 0, 0);
        simIO.WriteBit(0, 1, 1, 1, 0);
        simIO.WriteBit(0, 1, 1, 2, 1);
        simIO.WriteBit(0, 1, 1, 3, 0);
        simIO.WriteBit(0, 1, 1, 4, 0);
        {
            int v = Test_GetTestResulSub(0, 0);
            printf("    observed Test_GetTestResulSub(0,0) CHOneByOne = %d (expect 3)\n", v);
            CHECK(v == 3, "C2 Test_GetTestResulSub CHOneByOne -> GetOneByOneData == 3 (Sen-read dispatch, golden 738)");
        }

        // ---- C3: end-to-end Test_GetTestResul (50x stability confirm) -----
        //  Same steady window -> the 50-iteration re-read agrees -> returns iOld.
        {
            int v = Test_GetTestResul(0, 0);
            printf("    observed Test_GetTestResul(0,0) CHOneByOne stable = %d (expect 3)\n", v);
            CHECK(v == 3, "C3 Test_GetTestResul stable 50x re-read -> 3 (golden 747-762)");
        }
    }

    // =======================================================================
    printf("\n==== W6.4a decode-anchor summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
