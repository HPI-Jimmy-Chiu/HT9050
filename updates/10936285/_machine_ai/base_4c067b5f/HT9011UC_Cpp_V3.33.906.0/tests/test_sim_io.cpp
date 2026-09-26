// =============================================================================
//  tests/test_sim_io.cpp  -- W4-IO INTERFACE-CUT acceptance proof (dedicated)
//
//  Purpose
//  -------
//  A self-contained, hardware-free proof that the IO interface-cut works
//  OFFLINE with ZERO vendor SDK.  Mirrors the W4 motor "test_sim_motor.cpp"
//  acceptance proof, but for the IO HAL.
//
//  The contract (task wording, verbatim):
//    * select TSimIOBackend (offline in-memory backend);
//    * via TLaneIO assert IOBitOn / IOBitOff then IOOutBitStatus reflects it;
//    * IOByteOut / IOInputByte round-trip;
//    * IOInputBit returns the sim value;
//    * range/error path returns the documented code.
//
//  WHY THIS IS A VALID INTERFACE-CUT PROOF
//  ---------------------------------------
//  The 4 real backends (TMN200Backend / TPci1203Backend / TMnetLegacyBackend /
//  TRawPortBackend) call vendor functions (mn_set_port_bit / Acm_DaqDo* /
//  _mnet_* / outportb).  Those are compiled with HAVE_MN200 / HAVE_PCI1203 /
//  HAVE_MNET / HAVE_RAWPORT all OFF (default), so the vendor headers are never
//  included and no vendor symbol is referenced.  TSimIOBackend substitutes for
//  any real backend behind the same TIOBackend* port, so TLaneIO compiles,
//  links and RUNS with no hardware -- exactly the property the migration needs.
//
//  FAITHFULNESS NOTES (cite MyLaneIo.cpp line numbers)
//  ---------------------------------------------------
//    * OutPortData cache write happens BEFORE the backend write
//      (IOBitOn .cpp:215 / IOBitOff .cpp:280 / IOByteOut .cpp:343); IOOutBitStatus
//      is a pure cache read (.cpp:406) -- no hardware read.
//    * CheckPortRangeErr returns 0 when InitialOK==false (.cpp:663) or
//      iISABase!=eMotionNet (.cpp:666); otherwise codes 1..5 (.cpp:671-690).
//    * IOInputByte IO_CARD_TYPE==0 quirk (.cpp:568): Value=byte(ret).  The sim
//      ReadByte returns ret=0, so when IO_CARD_TYPE==0 the byte value is taken
//      from ret (=> 0) NOT from *Value.  To round-trip the sim byte READ we set
//      IO_CARD_TYPE=1 (MN200 mn_get_di_byte path, .cpp:542) where *Value is used
//      directly.  This is faithful: the quirk is intentional BCB6 behaviour.
//    * IOByteOut always returns true (Sam 20250325, .cpp:370) -- the write path.
// =============================================================================
#include "MyLaneIo.h"
#include "IOBackend.h"
#include "MachineType.h"    // eMotionNet, ePCI1203, ePLCbase, eIOType
#include "cmydef.h"         // InitialOK, IO_CARD_TYPE, MyBitMask,
                            // DEV_INF_IO_32OUT_DEV / 32IN / 16IN_16OUT (via MyLaneIo.h)
#include <cstdio>

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as test_sim_motor.cpp)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  Part 1 -- select TSimIOBackend; IOBitOn/IOBitOff -> IOOutBitStatus reflects it
//  CheckPortRangeErr returns 0 early (InitialOK==false) so the Sim path runs and
//  the OutPortData cache is the production observable.
// ---------------------------------------------------------------------------
static void test_bit_on_off_status()
{
    printf("\n[1] TSimIOBackend selected; IOBitOn/IOBitOff -> IOOutBitStatus\n");

    InitialOK = false;                       // no card init -> range check passes through

    TSimIOBackend* sim = new TSimIOBackend();   // SELECT the offline backend
    MyLaneIO.SetBackend(sim);
    MyLaneIO.InitialMyOutIOData();

    // IOBitOn -> OutPortData[Ring][IP][Port] |= MyBitMask[Bit]  (.cpp:215)
    MyLaneIO.IOBitOn(1, 0, 0, 4, eMotionNet, "Sw_R1IP0P0B4");
    CHECK(MyLaneIO.IOOutBitStatus(1, 0, 0, 4, eMotionNet) == true,
          "IOOutBitStatus==true after IOBitOn (OutPortData cache reflects it)");

    // A different bit in the same port must remain off (mask precision).
    CHECK(MyLaneIO.IOOutBitStatus(1, 0, 0, 5, eMotionNet) == false,
          "IOOutBitStatus(other bit)==false (IOBitOn touched only Bit 4)");

    // Turn a second bit on; both must read on.
    MyLaneIO.IOBitOn(1, 0, 0, 5, eMotionNet, "Sw_R1IP0P0B5");
    CHECK(MyLaneIO.IOOutBitStatus(1, 0, 0, 4, eMotionNet) == true &&
          MyLaneIO.IOOutBitStatus(1, 0, 0, 5, eMotionNet) == true,
          "both Bit4 and Bit5 read on after two IOBitOn calls");

    // IOBitOff -> OutPortData &= ~MyBitMask[Bit]  (.cpp:280); only Bit4 cleared.
    MyLaneIO.IOBitOff(1, 0, 0, 4, eMotionNet, "Sw_R1IP0P0B4");
    CHECK(MyLaneIO.IOOutBitStatus(1, 0, 0, 4, eMotionNet) == false,
          "IOOutBitStatus==false after IOBitOff (Bit4 cleared)");
    CHECK(MyLaneIO.IOOutBitStatus(1, 0, 0, 5, eMotionNet) == true,
          "IOBitOff(Bit4) leaves Bit5 on (mask precision)");
}

// ---------------------------------------------------------------------------
//  Part 2 -- IOByteOut / IOInputByte round-trip through the Sim
//  Write a byte (IOByteOut updates OutPortData AND the Sim port); read it back
//  via IOInputByte (Sim ReadByte fills *Value).
//
//  IO_CARD_TYPE=1 (MN200 path, .cpp:542): IOInputByte uses *Value from the
//  backend ReadByte directly, NOT the IO_CARD_TYPE==0 ret-quirk (.cpp:568).
// ---------------------------------------------------------------------------
static void test_byte_roundtrip()
{
    printf("\n[2] IOByteOut / IOInputByte round-trip (Sim port byte)\n");

    InitialOK = false;
    int savedCard = IO_CARD_TYPE;
    IO_CARD_TYPE  = 1;                        // MN200 read path: *Value used directly

    TSimIOBackend* sim = new TSimIOBackend();
    MyLaneIO.SetBackend(sim);
    MyLaneIO.InitialMyOutIOData();

    // IOByteOut returns true (Sam 20250325, .cpp:370) and writes the Sim port.
    bool wrote = MyLaneIO.IOByteOut(1, 2, 3, 0xA5u, eMotionNet);
    CHECK(wrote == true, "IOByteOut returns true (Sam 20250325)");

    // IOInputByte reads the SAME port back through the Sim (round-trip).
    byte rb = MyLaneIO.IOInputByte(1, 2, 3, eMotionNet);
    CHECK(rb == 0xA5u, "IOInputByte returns 0xA5 written by IOByteOut (Sim round-trip)");

    // Overwrite with a different value and confirm the read tracks it.
    MyLaneIO.IOByteOut(1, 2, 3, 0x3Cu, eMotionNet);
    byte rb2 = MyLaneIO.IOInputByte(1, 2, 3, eMotionNet);
    CHECK(rb2 == 0x3Cu, "IOInputByte tracks the overwritten byte 0x3C");

    // OutPortData cache also reflects the byte write (IOOutBitStatus per bit).
    // 0x3C = 0011 1100 -> bits 2,3,4,5 set.
    CHECK(MyLaneIO.IOOutBitStatus(1, 2, 3, 2, eMotionNet) == true  &&
          MyLaneIO.IOOutBitStatus(1, 2, 3, 5, eMotionNet) == true  &&
          MyLaneIO.IOOutBitStatus(1, 2, 3, 0, eMotionNet) == false &&
          MyLaneIO.IOOutBitStatus(1, 2, 3, 7, eMotionNet) == false,
          "OutPortData cache matches 0x3C bit pattern after IOByteOut");

    IO_CARD_TYPE = savedCard;                 // restore global
}

// ---------------------------------------------------------------------------
//  Part 3 -- IOInputBit returns the Sim value (input round-trip)
//  Write a bit directly into the Sim map, then read it via IOInputBit; the
//  facade routes pIO->ReadBit (eMotionNet, IO_CARD_TYPE-agnostic for the bit
//  path) and returns true/false based on the value (.cpp:481-487).
// ---------------------------------------------------------------------------
static void test_input_bit()
{
    printf("\n[3] IOInputBit returns the Sim value (input round-trip)\n");

    InitialOK = false;

    TSimIOBackend* sim = new TSimIOBackend();
    MyLaneIO.SetBackend(sim);
    MyLaneIO.InitialMyOutIOData();

    // Sim bit = 1 -> IOInputBit true
    sim->WriteBit(1, 0, 2, 5, 1);
    CHECK(MyLaneIO.IOInputBit(1, 0, 2, 5, eMotionNet, "Sen") == true,
          "IOInputBit==true after Sim WriteBit(...,1)");

    // Sim bit = 0 -> IOInputBit false
    sim->WriteBit(1, 0, 2, 5, 0);
    CHECK(MyLaneIO.IOInputBit(1, 0, 2, 5, eMotionNet, "Sen") == false,
          "IOInputBit==false after Sim WriteBit(...,0)");

    // Independent bit on a different port reads its own value.
    sim->WriteBit(2, 1, 1, 0, 1);
    CHECK(MyLaneIO.IOInputBit(2, 1, 1, 0, eMotionNet, "Sen2") == true,
          "IOInputBit reads an independent Sim bit on another port");
}

// ---------------------------------------------------------------------------
//  Part 4 -- range/error path returns the documented codes
//  CheckPortRangeErr (.cpp:661-693).  Bypass the two early returns:
//    InitialOK==true  AND  iISABase==eMotionNet
//  then drive each documented code 0..5.  iUseMNetIP[][] is a PUBLIC member, so
//  we set the device type to make the per-code branches deterministic.
//  GetIOErrStr (.cpp:800-816) must map each code to its documented string.
// ---------------------------------------------------------------------------
static void test_range_error_codes()
{
    printf("\n[4] CheckPortRangeErr documented codes + GetIOErrStr strings\n");

    bool savedInit = InitialOK;
    InitialOK = true;                         // force the real range check to run

    // Make Ring=0,IP=0 a valid 32-out device so code 3 does NOT trip there.
    MyLaneIO.iUseMNetIP[0][0] = DEV_INF_IO_32OUT_DEV;

    // --- code 0: valid args on a valid 32-out device -------------------------
    int c0 = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 0, 0, 0, 3);
    CHECK(c0 == 0, "CheckPortRangeErr code 0 (valid Ring0/IP0/Port0/Bit3, 32OUT dev)");

    // --- code 1: Bit out of [0,7] (Bit!=999) ---------------------------------
    int c1 = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 0, 0, 0, 8);
    CHECK(c1 == 1, "CheckPortRangeErr code 1 (Bit 8 not in 0..7)");

    // --- code 2: Ring/IP/Port out of range -----------------------------------
    int c2 = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 999, 0, 0, 0);
    CHECK(c2 == 2, "CheckPortRangeErr code 2 (Ring 999 out of range)");

    // --- code 3: iUseMNetIP not a valid IO device type -----------------------
    // Ring=0,IP=1 left as DEV_INF_NO_DEV by InitialMyUseIP() -> code 3.
    MyLaneIO.iUseMNetIP[0][1] = DEV_INF_NO_DEV;
    int c3 = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 0, 1, 0, 0);
    CHECK(c3 == 3, "CheckPortRangeErr code 3 (iUseMNetIP==NO_DEV)");

    // --- code 4: 16IN/16OUT, DO_Type=true, Port not 2 or 3 -------------------
    MyLaneIO.iUseMNetIP[0][2] = DEV_INF_IO_16IN_16OUT_DEV;
    int c4 = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 0, 2, 0, 0);   // DO, Port 0
    CHECK(c4 == 4, "CheckPortRangeErr code 4 (16IN/16OUT DO port not 2/3)");
    // And a DO write on the VALID port 2 of the same device is code 0.
    int c4ok = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 0, 2, 2, 0);
    CHECK(c4ok == 0, "CheckPortRangeErr code 0 (16IN/16OUT DO on valid port 2)");

    // --- code 5: 16IN/16OUT, DO_Type=false, Port not 0 or 1 ------------------
    int c5 = MyLaneIO.CheckPortRangeErr(false, eMotionNet, 0, 2, 2, 0);   // DI, Port 2
    CHECK(c5 == 5, "CheckPortRangeErr code 5 (16IN/16OUT DI port not 0/1)");

    // --- Bit==999 sentinel skips the bit check (byte ops pass it) ------------
    int cByte = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 0, 0, 0 /*Bit defaults 999*/);
    CHECK(cByte == 0, "CheckPortRangeErr Bit==999 sentinel skips bit range check");

    // --- early-return guards: InitialOK==false / iISABase!=eMotionNet --------
    InitialOK = false;
    int eInit = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 999, 999, 999, 999);
    CHECK(eInit == 0, "CheckPortRangeErr returns 0 when InitialOK==false");
    InitialOK = true;
    int ePci = MyLaneIO.CheckPortRangeErr(true, ePCI1203, 999, 999, 999, 999);
    CHECK(ePci == 0, "CheckPortRangeErr returns 0 when iISABase!=eMotionNet");

    // --- GetIOErrStr maps each documented code to its string -----------------
    CHECK(MyLaneIO.GetIOErrStr(0) == AnsiString("SUCCESS"),
          "GetIOErrStr(0)==SUCCESS");
    CHECK(MyLaneIO.GetIOErrStr(1) == AnsiString("Bit not between 0 to 7"),
          "GetIOErrStr(1)==Bit not between 0 to 7");
    CHECK(MyLaneIO.GetIOErrStr(2) == AnsiString("Ring, IP, or Port not in range"),
          "GetIOErrStr(2)==Ring, IP, or Port not in range");
    CHECK(MyLaneIO.GetIOErrStr(3) == AnsiString("UseMNetIP Err"),
          "GetIOErrStr(3)==UseMNetIP Err");
    CHECK(MyLaneIO.GetIOErrStr(4) == AnsiString("16IN/OUT - output port error"),
          "GetIOErrStr(4)==16IN/OUT - output port error");
    CHECK(MyLaneIO.GetIOErrStr(5) == AnsiString("16IN/OUT - input port error"),
          "GetIOErrStr(5)==16IN/OUT - input port error");

    InitialOK = savedInit;                    // restore
}

// ---------------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------------
int main()
{
    printf("=================================================================\n");
    printf("  W4-IO Sim IO -- dedicated interface-cut acceptance proof\n");
    printf("  (TSimIOBackend via TLaneIO; zero vendor SDK, zero hardware)\n");
    printf("=================================================================\n");

    test_bit_on_off_status();
    test_byte_roundtrip();
    test_input_bit();
    test_range_error_codes();

    printf("\n-----------------------------------------------------------------\n");
    printf("  Results: %d passed, %d failed\n", g_pass, g_fail);
    printf("-----------------------------------------------------------------\n");

    return (g_fail == 0) ? 0 : 1;
}
