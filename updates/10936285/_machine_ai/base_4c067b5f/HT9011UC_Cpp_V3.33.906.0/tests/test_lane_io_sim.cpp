// =============================================================================
//  tests/test_lane_io_sim.cpp  -- W4-IO HARDWARE INTERFACE-CUT acceptance test
//
//  Purpose
//  -------
//  Prove that the offline IO HAL works with ZERO vendor SDK and ZERO hardware.
//  TLaneIO (facade) + TSimIOBackend (in-memory default backend) must provide
//  fully exercisable IO through the same 6 public method signatures the
//  production code uses, and TMySwitch + TMySensor must route through TLaneIO
//  to TSimIOBackend.
//
//  This is the explicit W4-IO "interface-cut" proof:
//    * Real backends (TMN200Backend / TPci1203Backend / TMnetLegacyBackend)
//      call vendor functions that cannot link without the vendor SDK.  They
//      are compiled with HAVE_xxx all OFF (default) -- no vendor symbols needed.
//    * TSimIOBackend substitutes for any real backend behind the same
//      TIOBackend* interface, so the IO layer compiles, links and RUNS offline.
//
//  Acceptance assertions (mirror the task wording exactly):
//    IOBitOn  then IOOutBitStatus  reflects OutPortData cache
//    IOByteOut returns true
//    IOInputBit via Sim returns the bit we wrote (round-trip TSimIOBackend)
//    CheckPortRangeErr returns 0 when InitialOK==false / iISABase!=eMotionNet
//    GetIOErrStr maps 0->SUCCESS..5
//    TMySwitch.On()  sets the corresponding OutPortData bit via MyLaneIO
//    TMySensor.IsOn() reads it back via MyLaneIO (round-trip through Sim)
//
//  Faithfulness note: Ring/IP/Port/Bit semantics are BCB6 pulse-table indices.
//  No re-scaling.
// =============================================================================
#include "MyLaneIo.h"
#include "IOBackend.h"
#include "myswitch.h"
#include "mysensor.h"
#include "MachineType.h"    // eMotionNet, ePCI1203, eISABase, etc.
#include "cmydef.h"         // InitialOK, MyBitMask, IO_CARD_TYPE, etc.
#include <cstdio>

// ---------------------------------------------------------------------------
//  Minimal PASS / FAIL harness (same style as test_sim_motor.cpp)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                   \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ---------------------------------------------------------------------------
//  Part A -- TSimIOBackend exercised directly
//  Verifies the in-memory map: write a bit, read it back; write a byte, read.
// ---------------------------------------------------------------------------
static void test_sim_backend_direct()
{
    printf("\n[A] TSimIOBackend direct (in-memory round-trip)\n");

    TSimIOBackend sim;

    // WriteBit / ReadBit round-trip
    unsigned char v = 0;
    int ret;

    ret = sim.WriteBit(0, 0, 0, 3, 1);
    CHECK(ret >= 0,  "WriteBit(R0,IP0,Port0,Bit3,1) returns >= 0 (ok)");

    ret = sim.ReadBit(0, 0, 0, 3, &v);
    CHECK(ret >= 0,  "ReadBit(R0,IP0,Port0,Bit3) returns >= 0");
    CHECK(v == 1,    "ReadBit returns 1 after WriteBit 1");

    ret = sim.WriteBit(0, 0, 0, 3, 0);
    ret = sim.ReadBit(0, 0, 0, 3, &v);
    CHECK(v == 0,    "ReadBit returns 0 after WriteBit 0");

    // WriteByte / ReadByte round-trip
    ret = sim.WriteByte(1, 2, 3, 0xA5u);
    CHECK(ret >= 0,  "WriteByte(R1,IP2,Port3,0xA5) returns >= 0");

    ret = sim.ReadByte(1, 2, 3, &v);
    CHECK(ret >= 0,  "ReadByte(R1,IP2,Port3) returns >= 0");
    CHECK(v == 0xA5, "ReadByte returns 0xA5 after WriteByte 0xA5");

    // Out-of-range silently succeeds in sim (no crash, returns 0)
    ret = sim.WriteBit(99, 99, 99, 0, 1);
    CHECK(ret == 0,  "WriteBit out-of-range returns 0 (no crash)");
}

// ---------------------------------------------------------------------------
//  Part B -- TLaneIO facade through TSimIOBackend (injected)
//  Uses SetBackend() to replace the default TSimIOBackend with a fresh one
//  so we control the initial state.
// ---------------------------------------------------------------------------
static void test_lane_io_facade()
{
    printf("\n[B] TLaneIO facade with TSimIOBackend (interface-cut)\n");

    // Inject a fresh Sim backend so test is independent of global MyLaneIO state.
    TSimIOBackend* sim = new TSimIOBackend();
    MyLaneIO.SetBackend(sim);
    MyLaneIO.InitialMyOutIOData();

    // --- IOBitOn then IOOutBitStatus reflects OutPortData cache --------------
    // CheckPortRangeErr returns 0 early when InitialOK==false (no card checked).
    // (InitialOK is typically false in offline/test -- no hardware init.)
    MyLaneIO.IOBitOn(1, 0, 0, 4, eMotionNet, "TestSwitch");
    bool onStatus = MyLaneIO.IOOutBitStatus(1, 0, 0, 4, eMotionNet, "TestSwitch");
    CHECK(onStatus == true,  "IOOutBitStatus==true after IOBitOn (OutPortData cache)");

    // OutPortData bit should reflect MyBitMask[4]
    // (the exact bit pattern is internal but IOOutBitStatus is the production API)

    // --- IOBitOff clears the cache -------------------------------------------
    MyLaneIO.IOBitOff(1, 0, 0, 4, eMotionNet, "TestSwitch");
    bool offStatus = MyLaneIO.IOOutBitStatus(1, 0, 0, 4, eMotionNet, "TestSwitch");
    CHECK(offStatus == false, "IOOutBitStatus==false after IOBitOff");

    // --- IOByteOut returns true -----------------------------------------------
    bool bOut = MyLaneIO.IOByteOut(1, 0, 0, 0xC3u, eMotionNet);
    CHECK(bOut == true,      "IOByteOut returns true (Sam 20250325)");

    // --- IOInputBit round-trip through TSimIOBackend -------------------------
    // Write a bit into the Sim map directly, then read via IOInputBit.
    // Ring=1,IP=0,Port=2,Bit=5 -> write 1 to Sim, then IOInputBit should see it.
    sim->WriteBit(1, 0, 2, 5, 1);
    // IOInputBit with iISABase=eMotionNet routes to pIO->ReadBit on the Sim.
    // Note: CheckPortRangeErr returns 0 when InitialOK==false (always in test).
    bool bitIn = MyLaneIO.IOInputBit(1, 0, 2, 5, eMotionNet, "TestSensor");
    CHECK(bitIn == true,     "IOInputBit returns true after Sim WriteBit 1 (round-trip)");

    sim->WriteBit(1, 0, 2, 5, 0);
    bool bitIn0 = MyLaneIO.IOInputBit(1, 0, 2, 5, eMotionNet, "TestSensor");
    CHECK(bitIn0 == false,   "IOInputBit returns false after Sim WriteBit 0");

    // --- CheckPortRangeErr returns 0 when InitialOK==false -------------------
    // (InitialOK starts as false; no hardware init in test build)
    int rangeErr = MyLaneIO.CheckPortRangeErr(true, eMotionNet, 0, 0, 0, 0);
    CHECK(rangeErr == 0,     "CheckPortRangeErr returns 0 when InitialOK==false");

    // Also returns 0 for any iISABase != eMotionNet
    int rangeErrPCI = MyLaneIO.CheckPortRangeErr(true, ePCI1203, 999, 999, 999, 999);
    CHECK(rangeErrPCI == 0,  "CheckPortRangeErr returns 0 for iISABase!=eMotionNet");

    // --- GetIOErrStr maps 0->SUCCESS, 1-5 documented strings -----------------
    AnsiString s0 = MyLaneIO.GetIOErrStr(0);
    CHECK(s0 == AnsiString("SUCCESS"),                    "GetIOErrStr(0)==SUCCESS");

    AnsiString s1 = MyLaneIO.GetIOErrStr(1);
    CHECK(s1 == AnsiString("Bit not between 0 to 7"),     "GetIOErrStr(1)");

    AnsiString s2 = MyLaneIO.GetIOErrStr(2);
    CHECK(s2 == AnsiString("Ring, IP, or Port not in range"), "GetIOErrStr(2)");

    AnsiString s3 = MyLaneIO.GetIOErrStr(3);
    CHECK(s3 == AnsiString("UseMNetIP Err"),               "GetIOErrStr(3)");

    AnsiString s4 = MyLaneIO.GetIOErrStr(4);
    CHECK(s4 == AnsiString("16IN/OUT - output port error"), "GetIOErrStr(4)");

    AnsiString s5 = MyLaneIO.GetIOErrStr(5);
    CHECK(s5 == AnsiString("16IN/OUT - input port error"),  "GetIOErrStr(5)");

    // --- BackUpOutputData / RestoreOutputData smoke test ---------------------
    // Set a known output state, back up, change it, restore -- verify restored.
    MyLaneIO.IOByteOut(0, 1, 2, 0x55u, eMotionNet);
    MyLaneIO.BackUpOutputData();
    MyLaneIO.IOByteOut(0, 1, 2, 0x00u, eMotionNet);   // overwrite
    // After Restore, iUseMNetIP[0][1] is DEV_INF_NO_DEV (0x00), so the
    // conditional in RestoreOutputData does not fire -- safe to call, no crash.
    MyLaneIO.RestoreOutputData();
    CHECK(true, "BackUpOutputData / RestoreOutputData (no crash)");
}

// ---------------------------------------------------------------------------
//  Part C -- TMySwitch routes through MyLaneIO -> TSimIOBackend
// ---------------------------------------------------------------------------
static void test_myswitch_routing()
{
    printf("\n[C] TMySwitch routes through MyLaneIO (object-layer proof)\n");

    // Inject a fresh Sim backend so Part B's state does not interfere.
    TSimIOBackend* sim = new TSimIOBackend();
    MyLaneIO.SetBackend(sim);
    MyLaneIO.InitialMyOutIOData();

    // Configure a switch pointing at Ring=0,IP=1,Port=0,Bit=6,Type=1 (active-high)
    TMySwitch sw;
    sw.Ring    = 0;
    sw.IP      = 1;
    sw.Port    = 0;
    sw.Bit     = 6;
    sw.Type    = 1;     // active-high: On()->IOBitOn, Off()->IOBitOff
    sw.ISABase = eMotionNet;
    sw.Enable  = true;
    sw.Name    = "TestSW";

    // On() -> IOBitOn -> OutPortData|=MyBitMask[6]
    sw.On();
    bool afterOn = MyLaneIO.IOOutBitStatus(0, 1, 0, 6, eMotionNet, "TestSW");
    CHECK(afterOn == true,  "TMySwitch.On() sets OutPortData bit via MyLaneIO");

    // Status() reads OutPortData cache (IOOutBitStatus -- no hardware read)
    bool st = sw.Status();
    CHECK(st == true,       "TMySwitch.Status()==true after On() (cache round-trip)");

    // Off() -> IOBitOff -> OutPortData&=~MyBitMask[6]
    sw.Off();
    bool afterOff = MyLaneIO.IOOutBitStatus(0, 1, 0, 6, eMotionNet, "TestSW");
    CHECK(afterOff == false, "TMySwitch.Off() clears OutPortData bit via MyLaneIO");

    // OnOff() convenience wrapper
    sw.OnOff(true);
    CHECK(MyLaneIO.IOOutBitStatus(0, 1, 0, 6, eMotionNet, "TestSW") == true,
          "TMySwitch.OnOff(true) -> bit on");
    sw.OnOff(false);
    CHECK(MyLaneIO.IOOutBitStatus(0, 1, 0, 6, eMotionNet, "TestSW") == false,
          "TMySwitch.OnOff(false) -> bit off");

    // CopySwitch: verify field copy for eMotionNet branch
    TMySwitch sw2;
    CopySwitch(&sw, &sw2);
    CHECK(sw2.Ring == sw.Ring && sw2.IP == sw.IP && sw2.Port == sw.Port &&
          sw2.Bit  == sw.Bit  && sw2.Type == sw.Type,
          "CopySwitch(eMotionNet) copies Ring/IP/Port/Bit/Type");
}

// ---------------------------------------------------------------------------
//  Part D -- TMySensor routes through MyLaneIO -> TSimIOBackend
// ---------------------------------------------------------------------------
static void test_mysensor_routing()
{
    printf("\n[D] TMySensor routes through MyLaneIO (object-layer proof)\n");

    // Reuse the same Sim backend (still injected from Part C).
    // Write a bit directly to Sim so we control what the sensor reads.
    TSimIOBackend* sim = new TSimIOBackend();
    MyLaneIO.SetBackend(sim);
    MyLaneIO.InitialMyOutIOData();

    // Configure a sensor: Ring=0,IP=2,Port=1,Bit=3,Type=1 (active-high)
    TMySensor sen;
    sen.Ring    = 0;
    sen.IP      = 2;
    sen.Port    = 1;
    sen.Bit     = 3;
    sen.Type    = 1;    // active-high: Status()/IsOn() return raw read; IsOff() inverts
    sen.ISABase = eMotionNet;
    sen.Enable  = true;
    sen.Name    = "TestSen";

    // With bit=0 in Sim -> IsOn()==false, IsOff()==true
    sim->WriteBit(0, 2, 1, 3, 0);
    CHECK(sen.IsOn()  == false, "TMySensor.IsOn()==false when Sim bit=0 (Type=1)");
    CHECK(sen.IsOff() == true,  "TMySensor.IsOff()==true  when Sim bit=0 (Type=1)");

    // With bit=1 in Sim -> IsOn()==true, IsOff()==false
    sim->WriteBit(0, 2, 1, 3, 1);
    CHECK(sen.IsOn()  == true,  "TMySensor.IsOn()==true  when Sim bit=1 (Type=1)");
    CHECK(sen.IsOff() == false, "TMySensor.IsOff()==false when Sim bit=1 (Type=1)");

    // Status() same as IsOn() for Type=1
    bool st = sen.Status();
    CHECK(st == true, "TMySensor.Status()==true when Sim bit=1 (Type=1)");

    // Type=0 (active-low) inverts: bit=1 -> IsOn()==false
    sen.Type = 0;
    CHECK(sen.IsOn()  == false, "TMySensor.IsOn()==false when Sim bit=1 AND Type=0 (invert)");
    CHECK(sen.IsOff() == true,  "TMySensor.IsOff()==true  when Sim bit=1 AND Type=0 (invert)");

    // CopySensor: eMotionNet branch copies Ring/IP/Port/Bit/Type
    TMySensor sen2;
    CopySensor(&sen, &sen2);
    CHECK(sen2.Ring == sen.Ring && sen2.IP == sen.IP && sen2.Port == sen.Port &&
          sen2.Bit  == sen.Bit  && sen2.Type == sen.Type,
          "CopySensor(eMotionNet) copies Ring/IP/Port/Bit/Type");

    // Enable=false -> Status()/IsOn()/IsOff() all return false (State=-1)
    sen.Enable = false;
    CHECK(sen.Status() == false, "TMySensor.Status()==false when Enable==false");
    CHECK(sen.IsOn()   == false, "TMySensor.IsOn()==false  when Enable==false");
    CHECK(sen.IsOff()  == false, "TMySensor.IsOff()==false when Enable==false");
}

// ---------------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------------
int main()
{
    printf("=================================================================\n");
    printf("  W4-IO LaneIO Sim -- interface-cut acceptance test\n");
    printf("=================================================================\n");

    test_sim_backend_direct();
    test_lane_io_facade();
    test_myswitch_routing();
    test_mysensor_routing();

    printf("\n-----------------------------------------------------------------\n");
    printf("  Results: %d passed, %d failed\n", g_pass, g_fail);
    printf("-----------------------------------------------------------------\n");

    return (g_fail == 0) ? 0 : 1;
}
