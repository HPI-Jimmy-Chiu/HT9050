// ===========================================================================
//  tests/test_vc4_group.cpp -- ctest Vc4Group
//
//  AI(W906-F03-QUADVAC) 20261004: F-03 ③ -- HT9050 picks ONE IC with the 4 nozzles of one ECAT-VC4.  The group lives in
//  TMySucker (mykitsuck.cpp, end of file); this test drives it on a fake 1203 backend (memory only: every output write
//  is recorded, every input is set by the test; no card, no machine file -- Gerneral.ini is ENV-ALL's per-test scratch).
//
//    1. switch OFF (no group) = golden: the master drives only its own two valves; all-on / any-on / held read
//       exactly GetStatus(); a companion is an ordinary sucker (drives its own valves, reads its own sensor).
//    2. the boot wiring: Gerneral.ini [System] IN_ARM_VC4_GROUP=1 -> InArmSuckA + C / E / G; the other two stations stay
//       ungrouped; a station whose Suck[0][0] is not the first name is NOT grouped.
//    3. fan-out: On / Off / Normal on the master drive all 4 make valves (odd DO 17..23) and break valves (even DO
//       16..22); a companion driven on its own sends nothing.
//    4. reads: all-on needs all 4 DI 64..67; any-on needs 1; a companion's own GetStatus() reads false; a row with
//       Enable=0 is not counted.
//    5. the carrying drop debounce: a loss reads "held" until it has lasted 100 ms; recovery restarts the window.
//    6. Suck(): 3 of 4 -> Error, never done; 4 of 4 -> done.  Destroy(): 1 still on -> Error; all off -> done.
//    7. source pins (argv[1] = the source root): the call sites R226 marked read the group the right way round.
//
//  REVERSE (run by hand, NB2-1 R228): W906_FanOut returning false at once -> section 3 red; Suck()'s all-on reads put
//  back to GetStatus() -> section 6 "3 of 4" red; the member line in GetStatus() removed -> section 4 "companion reads
//  false" red; kW906HoldLostMs = 0 -> section 5 red.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "vclcompat/IniFiles.h"
#include "mykitsuck.h"
#include "cmydef.h"
#include "LastSet.h"
#include "MyLaneIo.h"
#include "IOBackend.h"
#include "common.h"
#include "w906_ctest_guard.h"

#include <windows.h>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

extern void W906_InitVc4Groups();
extern int  W906_WireVc4Group(TMyKitSuck& kit, const char* const names[4], const char* station);

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- a fake PCIE-1203: outputs recorded by (station, channel), inputs set by the test --------------------------------
struct TFake1203 : public TIOBackend
{
    std::map<long, int> out;     // last value written
    int writes;
    std::set<long> on;           // DI channels that read 1
    TFake1203() : writes(0) {}
    static long Key(int ip, int port) { return (long)ip * 1000L + port; }
    int WriteBit(int Ring, int IP, int Port, int Bit, int Value) override
    {
        (void)Ring; (void)Bit;
        out[Key(IP, Port)] = Value;
        ++writes;
        return 0;
    }
    int ReadBit(int Ring, int IP, int Port, int Bit, unsigned char* Value) override
    {
        (void)Ring; (void)Bit;
        if (Value) *Value = on.count(Key(IP, Port)) ? 1 : 0;
        return 0;
    }
    int Out(int ip, int port) const
    {
        std::map<long, int>::const_iterator it = out.find(Key(ip, port));
        return it == out.end() ? -1 : it->second;
    }
};
static TFake1203 g_io;
static const int kRing = 1, kIp = 160;

// one VC4 channel n: make DO 17+2n, break DO 16+2n, vacuum-OK DI 64+n (machines/HT9050/IO_Table.csv, station 160)
static void Bind(TMySucker& s, const char* name, int n, bool enable)
{
    s.SuckerName = name;
    s.SensorName = name;
    s.Enable = enable;
    s.OnEnable = enable;
    s.OffEnable = enable;
    s.OnISABase = ePCI1203; s.OffISABase = ePCI1203; s.SenISABase = ePCI1203;
    s.OnType = TYPE_A; s.OffType = TYPE_A; s.SenType = TYPE_A;
    s.OnRing = kRing; s.OffRing = kRing; s.SenRing = kRing;
    s.OnIP = kIp; s.OffIP = kIp; s.SenIP = kIp;
    s.OnPort = 17 + 2 * n; s.OffPort = 16 + 2 * n; s.SenPort = 64 + n;
    s.OnBit = n; s.OffBit = n; s.SenBit = n;
    s.OnAlarmTime = 3; s.OffAlarmTime = 3;          // x10 ms in Suck()/Destroy()
    s.OnDelayTime = 0; s.OffDelayTime = 0;
    s.DestroyAgainCount = 0; s.DestroyAgainTime = 0;
    s.SetRetryCount(1);
}
static void InputsOn(int mask)                       // bit n = DI 64+n
{
    g_io.on.clear();
    for (int n = 0; n < 4; ++n)
        if (mask & (1 << n))
            g_io.on.insert(TFake1203::Key(kIp, 64 + n));
}
static bool AllMake(int v)  { for (int n = 0; n < 4; ++n) if (g_io.Out(kIp, 17 + 2 * n) != v) return false; return true; }
static bool AllBreak(int v) { for (int n = 0; n < 4; ++n) if (g_io.Out(kIp, 16 + 2 * n) != v) return false; return true; }

// drive a state-machine call until it returns true, sets Error, or 2 s pass
template <class F> static int Drive(TMySucker& s, F f)
{
    const DWORD t0 = ::GetTickCount();
    s.ReStart();
    while (::GetTickCount() - t0 < 2000)
    {
        if (f(s)) return 1;
        if (s.Error) return -1;
        ::Sleep(0);
    }
    return 0;
}
static bool DoSuck(TMySucker& s)    { return s.Suck(); }
static bool DoDestroy(TMySucker& s) { return s.Destroy(); }

static std::string ReadAll(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static int Count(const std::string& s, const std::string& what)
{
    int n = 0;
    for (size_t p = s.find(what); p != std::string::npos; p = s.find(what, p + what.size()))
        ++n;
    return n;
}

int main(int argc, char** argv)
{
    const char* const rt[] = { "asGeneralPath", asGeneralPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("Vc4Group", rt))
        return 2;
    std::printf("Vc4Group (%s)\n",
#ifdef SOFT_SIMULTE
                "SIM"
#else
                "SHIP"
#endif
                );
    MyLaneIO.SetBackends(&g_io, 0, 0);
    LastSet.iRealDummy = REALLY;
    static const char* const names[4] = { "InArmSuckA", "InArmSuckC", "InArmSuckE", "InArmSuckG" };
    TMySucker* s[4] = { &InArmSuck.Suck[0][0], &InArmSuck.Suck[0][1], &InArmSuck.Suck[0][2], &InArmSuck.Suck[0][3] };
    for (int n = 0; n < 4; ++n)
        Bind(*s[n], names[n], n, true);
    TMySucker& A = *s[0];

    std::printf(" 1. switch OFF = golden\n");
    {
        TIniFile ini(asGeneralPath);
        ini.WriteInteger("System", "IN_ARM_VC4_GROUP", 0);
        ini.WriteInteger("System", "OUT_ARM_VC4_GROUP", 0);
        ini.WriteInteger("System", "INDEX_VC4_GROUP", 0);
    }
    W906_InitVc4Groups();
    CHECK(A.W906_iGroupCount == 0 && s[1]->W906_pGroupMaster == 0);
    g_io.out.clear();
    A.On();
    CHECK(g_io.Out(kIp, 17) == 1 && g_io.Out(kIp, 16) == 0);
    CHECK(g_io.Out(kIp, 19) == -1 && g_io.Out(kIp, 21) == -1 && g_io.Out(kIp, 23) == -1);
    s[1]->On();
    CHECK(g_io.Out(kIp, 19) == 1);                               // a companion is an ordinary sucker
    for (int mask = 0; mask < 16; ++mask)
    {
        InputsOn(mask);
        const bool g = A.GetStatus();
        CHECK(A.W906_GetStatusAllOn() == g && A.W906_GetStatusAnyOn() == g && A.W906_GetStatusHeld() == g);
        CHECK(s[1]->GetStatus() == ((mask & 2) != 0));
    }

    std::printf(" 2. the boot wiring (Gerneral.ini [System] IN_ARM_VC4_GROUP=1)\n");
    {
        TIniFile ini(asGeneralPath);
        ini.WriteInteger("System", "IN_ARM_VC4_GROUP", 1);
    }
    W906_InitVc4Groups();
    CHECK(A.W906_iGroupCount == 4);
    CHECK(A.W906_pGroup[0] == &A && A.W906_pGroup[1] == s[1] && A.W906_pGroup[2] == s[2] && A.W906_pGroup[3] == s[3]);
    CHECK(s[1]->W906_pGroupMaster == &A && s[2]->W906_pGroupMaster == &A && s[3]->W906_pGroupMaster == &A);
    CHECK(A.W906_pGroupMaster == 0);
    CHECK(OutArmSuck.Suck[0][0].W906_iGroupCount == 0 && FTestSuck.Suck[0][0].W906_iGroupCount == 0);
    {
        TMyKitSuck kit;                                           // Suck[0][0] is not the first name -> not grouped
        kit.Suck[0][0].SuckerName = "InArmSuckE";
        kit.Suck[0][1].SuckerName = "InArmSuckC";
        kit.Suck[0][2].SuckerName = "InArmSuckA";
        kit.Suck[0][3].SuckerName = "InArmSuckG";
        CHECK(W906_WireVc4Group(kit, names, "test") == 0 && kit.Suck[0][0].W906_iGroupCount == 0);
        kit.Suck[0][3].SuckerName = "nothing";                    // a missing name -> not grouped
        kit.Suck[0][0].SuckerName = "InArmSuckA"; kit.Suck[0][2].SuckerName = "InArmSuckE";
        CHECK(W906_WireVc4Group(kit, names, "test") == 0 && kit.Suck[0][0].W906_iGroupCount == 0);
    }

    std::printf(" 3. fan-out\n");
    g_io.out.clear();
    A.On();
    CHECK(AllMake(1) && AllBreak(0));
    A.Off();
    CHECK(AllMake(0) && AllBreak(1));
    A.Normal();
    CHECK(AllMake(0) && AllBreak(0));
    {
        const int w = g_io.writes;
        s[2]->On();                                               // a companion on its own: nothing is sent
        s[3]->Off();
        s[1]->OnDestroy();
        CHECK(g_io.writes == w);
    }

    std::printf(" 4. reads\n");
    InputsOn(0x7);                                                // A, C, E on; G off
    CHECK(A.W906_GetStatusAllOn() == false && A.W906_GetStatusAnyOn() == true);
    CHECK(A.GetStatus() == true);                                 // the master's own sensor (A)
    CHECK(s[1]->GetStatus() == false && s[2]->GetStatus() == false);   // a companion reads false while grouped
    CHECK(s[1]->Sensor() == false);
    InputsOn(0xF);
    CHECK(A.W906_GetStatusAllOn() == true);
    InputsOn(0x8);                                                // only G
    CHECK(A.W906_GetStatusAllOn() == false && A.W906_GetStatusAnyOn() == true && A.GetStatus() == false);
    InputsOn(0x0);
    CHECK(A.W906_GetStatusAnyOn() == false);
    s[3]->Enable = false;                                         // G's row disabled -> 3 of 3 is "all on"
    InputsOn(0x7);
    CHECK(A.W906_GetStatusAllOn() == true);
    s[3]->Enable = true;

    std::printf(" 5. the carrying drop debounce (100 ms)\n");
    InputsOn(0xF);
    CHECK(A.W906_GetStatusHeld() == true);
    InputsOn(0xB);                                                // E lost
    const DWORD tLost = ::GetTickCount();
    CHECK(A.W906_GetStatusHeld() == true);                        // the window starts
    while (::GetTickCount() - tLost < 50) ::Sleep(0);
    CHECK(A.W906_GetStatusHeld() == true);                        // ~50 ms: still held
    InputsOn(0xF);
    CHECK(A.W906_GetStatusHeld() == true);                        // recovered: the window restarts
    InputsOn(0xB);
    const DWORD tLost2 = ::GetTickCount();
    CHECK(A.W906_GetStatusHeld() == true);
    while (::GetTickCount() - tLost2 < 70) ::Sleep(0);
    CHECK(A.W906_GetStatusHeld() == true);                        // 70 ms after the second loss: the first one does not count
    while (::GetTickCount() - tLost2 < 130) ::Sleep(0);
    CHECK(A.W906_GetStatusHeld() == false);                       // > 100 ms: dropped
    A.On();                                                       // a new hold restarts the window
    CHECK(A.W906_dwHoldLostStart == 0);

    std::printf(" 6. Suck() / Destroy()\n");
    InputsOn(0x7);                                                // 3 of 4
    CHECK(Drive(A, DoSuck) == -1);
    InputsOn(0xF);
    CHECK(Drive(A, DoSuck) == 1);
    CHECK(AllMake(1) && AllBreak(0));
    InputsOn(0x4);                                                // E still holds after the blow
    CHECK(Drive(A, DoDestroy) == -1);
    InputsOn(0x0);
    CHECK(Drive(A, DoDestroy) == 1);
    CHECK(AllMake(0) && AllBreak(0));

    std::printf(" 7. source pins\n");
    if (argc > 1)
    {
        const std::string root = argv[1];
        const std::string cs = ReadAll(root + "/csystem.cpp");
        const std::string oa = ReadAll(root + "/aoutarm.cpp");
        const std::string tf = ReadAll(root + "/aTester_Front.cpp");
        const std::string ci = ReadAll(root + "/cinitial.cpp");
        const std::string mk = ReadAll(root + "/mykitsuck.cpp");
        CHECK(!cs.empty() && !oa.empty() && !tf.empty() && !ci.empty() && !mk.empty());
        CHECK(Count(cs, "InArmSuck.Suck[i][j].W906_GetStatusHeld()==false") == 3);    // In Arm drops
        CHECK(Count(cs, "OutArmSuck.Suck[i][j].W906_GetStatusHeld()==false") == 1);   // Out Arm drop
        CHECK(Count(cs, "FTestSuck.Suck[i][j].W906_GetStatusHeld()==false") == 2);    // Index drops
        CHECK(Count(oa, "OutArmSuck.Suck[i][j].W906_GetStatusHeld()==false") == 4);
        CHECK(Count(tf, "FTestSuck.Suck[i][j].W906_GetStatusHeld()==false") == 7);
        CHECK(Count(cs, "InArmSuck.Suck[i][j].W906_GetStatusAnyOn())") == 2);         // the JAM0127 residue check + InArmSuckState
        CHECK(Count(cs, "? Ptr.Suck[iR][iC].W906_GetStatusAllOn() : Ptr.Suck[iR][iC].W906_GetStatusAnyOn();") == 1);
        CHECK(Count(ci, "InitSucker();  { extern void W906_InitVc4Groups(); W906_InitVc4Groups(); }") == 1);
        CHECK(Count(mk, "InRet=W906_GetStatusAllOn();") == 3 && Count(mk, "InRet=W906_GetStatusAnyOn();") == 2);
        CHECK(Count(mk, "{   if(W906_FanOut(true, bOn)) return;") == 1 && Count(mk, "{   if(W906_FanOut(false, bOn)) return;") == 1);
    }
    else
        std::printf("  (skipped: no source root given)\n");

    std::printf(" 8. HT9050: In / Out Arm forced on (Jimmy 1005, NB2-1 R232), the Index still by its switch\n");
    {
        extern AnsiString W906_GpibModel;
        const AnsiString gm = W906_GpibModel;
        const int mt = MachineTypeChoice;
        static const char* const outN[4] = { "OutArmSuckA", "OutArmSuckC", "OutArmSuckE", "OutArmSuckG" };
        static const char* const idxN[4] = { "FTestSuckAA", "FTestSuckBA", "FTestSuckAB", "FTestSuckBB" };
        for (int n = 0; n < 4; ++n) { OutArmSuck.Suck[0][n].SuckerName = outN[n]; FTestSuck.Suck[0][n].SuckerName = idxN[n]; }
        {
            TIniFile ini(asGeneralPath);
            ini.WriteInteger("System", "IN_ARM_VC4_GROUP", 0);
            ini.WriteInteger("System", "OUT_ARM_VC4_GROUP", 0);
            ini.WriteInteger("System", "INDEX_VC4_GROUP", 0);
        }
        W906_GpibModel = "HT-9045"; MachineTypeChoice = Type_HT9045;
        W906_InitVc4Groups();
        CHECK(A.W906_iGroupCount == 0 && OutArmSuck.Suck[0][0].W906_iGroupCount == 0 && FTestSuck.Suck[0][0].W906_iGroupCount == 0);   // not HT9050 + switches 0 = nothing
        W906_GpibModel = "9050GPIB";
        W906_InitVc4Groups();
        CHECK(A.W906_iGroupCount == 4 && OutArmSuck.Suck[0][0].W906_iGroupCount == 4);   // 9050GPIB: In / Out grouped although the file says 0
        CHECK(OutArmSuck.Suck[0][1].W906_pGroupMaster == &OutArmSuck.Suck[0][0] && s[3]->W906_pGroupMaster == &A);
        CHECK(FTestSuck.Suck[0][0].W906_iGroupCount == 0);                               // the Index is not forced
        W906_GpibModel = "HT-9045"; MachineTypeChoice = Type_HT9050;
        W906_InitVc4Groups();
        CHECK(A.W906_iGroupCount == 4 && OutArmSuck.Suck[0][0].W906_iGroupCount == 4 && FTestSuck.Suck[0][0].W906_iGroupCount == 0);   // Type_HT9050 alone: the same
        { TIniFile ini(asGeneralPath); ini.WriteInteger("System", "INDEX_VC4_GROUP", 1); }
        W906_InitVc4Groups();
        CHECK(FTestSuck.Suck[0][0].W906_iGroupCount == 4);                               // the Index still follows its own switch
        { TIniFile ini(asGeneralPath); ini.WriteInteger("System", "INDEX_VC4_GROUP", 0); }
        W906_GpibModel = gm; MachineTypeChoice = mt;
        W906_InitVc4Groups();
        CHECK(A.W906_iGroupCount == 0 && OutArmSuck.Suck[0][0].W906_iGroupCount == 0);   // back to a non-HT9050 machine: nothing
        if (argc > 1)
        {
            const std::string mk = ReadAll(std::string(argv[1]) + "/mykitsuck.cpp");
            CHECK(Count(mk, "if(W906_GpibModel==\"9050GPIB\" || MachineTypeChoice==Type_HT9050) { if(swIn!=1 || swOut!=1)") == 1);
            CHECK(Count(mk, "swIn=1; swOut=1; } }") == 1 && Count(mk, "swIdx=1") == 0);
        }
    }
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
