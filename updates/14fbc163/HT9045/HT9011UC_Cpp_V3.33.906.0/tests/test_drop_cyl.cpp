// ===========================================================================
//  tests/test_drop_cyl.cpp   (ctest: DropCyl)
//
//  AI(W906-DROPCYL) 20261005: NB2-1 R229 (laptop W-64; Steven W-56; St01 ST01_W_ANSWERS_20261004.md W-56) -- HT9050's In / Out
//  Arm anti-drop cylinders (C_InPnPDrop1-4 / C_OutPnPDrop1-4) follow the nozzle's has-IC state, each arm behind its own
//  Gerneral.ini [System] switch (IN_ARM_DROP_CYL / OUT_ARM_DROP_CYL, default 0). Helpers at the end of mykitsuck.cpp.
//    1  switches absent (= 0): W906_DropCylNames names nothing; Open / Close answer true and write no IO, IC or not
//    2  IN_ARM_DROP_CYL=1 only: the four In names, not the Out ones, never C_OutArmSmallY; both = 1: all eight
//    3  Open: a closed cylinder is popped (output off), false until its _Off sensor is on; an open one is not driven again
//    4  Close: no IC -> true, nothing written; IC held -> pushed (output on), false until its _On sensor is on; closed -> not
//       driven again
//    5  a cylinder the IO table does not have (Enable=false) is skipped
//    6  HOME: the eight are in CynNeedHome, CynForHome == 82
//    7  source pins: the four Z-down hooks, the three close sites, the InitCylinder call
//  Memory + a scratch Gerneral.ini (ctest redirects asGeneralPath); a fake PCIE-1203 behind MyLaneIO (test_vc4_group's shape).
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "vclcompat/IniFiles.h"
#include "mykitsuck.h"
#include "mycylin.h"
#include "cmydef.h"
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

extern void W906_DropCylNames();
extern bool W906_DropCylOpen(int arm);
extern bool W906_DropCylClose(int arm);
extern void W906_DropCylReread();
extern TMyKitSuck InArmSuck, OutArmSuck;

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- a fake PCIE-1203: outputs recorded by (station, port), inputs set by the test --------------------------------------
struct TFake1203 : public TIOBackend
{
    std::map<long, int> out;
    int writes;
    std::set<long> on;
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
static const int kRing = 1, kIp = 18;

// cylinder k of an arm: output port 10*arm+k, _On sensor 200+10*arm+k, _Off sensor 300+10*arm+k (station 18). A 1203 output port must be
// < 32 (MyLaneIo.cpp CheckPortRangeErr, the OutPortData cache) or the write is dropped silently; inputs are not range-checked.
static int OutPort(int arm, int k) { return 10 * arm + k; }
static int OnPort(int arm, int k)  { return 200 + 10 * arm + k; }
static int OffPort(int arm, int k) { return 300 + 10 * arm + k; }
static int Idx(int arm, int k)
{
    if (arm == 0) return k == 0 ? C_InPnPDrop1  : k == 1 ? C_InPnPDrop2  : k == 2 ? C_InPnPDrop3  : C_InPnPDrop4;
    return              k == 0 ? C_OutPnPDrop1 : k == 1 ? C_OutPnPDrop2 : k == 2 ? C_OutPnPDrop3 : C_OutPnPDrop4;
}
static void Bind(int arm, int k, bool enable)
{
    TMyCylinder& c = Cylinder[Idx(arm, k)];
    c.Enable = enable;
    c.OutISABase = ePCI1203; c.OutType = TYPE_A; c.OutRing = kRing; c.OutIP = kIp; c.OutPort = OutPort(arm, k); c.OutBit = 0;
    c.OnSenEnable = true;  c.OnSenISABase = ePCI1203;  c.OnSenType = TYPE_A;  c.OnSenRing = kRing;  c.OnSenIP = kIp;  c.OnSenPort = OnPort(arm, k);   c.OnSenBit = 0;
    c.OffSenEnable = true; c.OffSenISABase = ePCI1203; c.OffSenType = TYPE_A; c.OffSenRing = kRing; c.OffSenIP = kIp; c.OffSenPort = OffPort(arm, k); c.OffSenBit = 0;
    c.OnAlarmTime = 3; c.OffAlarmTime = 3; c.OnDelayTime = 0; c.OffDelayTime = 0;
    c.OnSensorName = c.CylinderName + "_On"; c.OffSensorName = c.CylinderName + "_Off";
}
static void Sensors(int arm, bool closed)                                        // the vane at closed (On) or open (Off)
{
    for (int k = 0; k < 4; ++k) {
        g_io.on.erase(TFake1203::Key(kIp, OnPort(arm, k)));
        g_io.on.erase(TFake1203::Key(kIp, OffPort(arm, k)));
        g_io.on.insert(TFake1203::Key(kIp, closed ? OnPort(arm, k) : OffPort(arm, k)));
    }
}
static bool AllOut(int arm, int v) { for (int k = 0; k < 4; ++k) if (g_io.Out(kIp, OutPort(arm, k)) != v) return false; return true; }
template <class F> static int Drive(F f)                                         // call until true, at most 2 s
{
    const DWORD t0 = ::GetTickCount();
    while (::GetTickCount() - t0 < 2000) { if (f()) return 1; ::Sleep(1); }
    return 0;
}
static void SetSwitches(int in, int out)
{
    TIniFile ini(asGeneralPath);
    ini.WriteInteger("System", "IN_ARM_DROP_CYL", in);
    ini.WriteInteger("System", "OUT_ARM_DROP_CYL", out);
    W906_DropCylReread();
}
static void DropSwitchKeys()                                                     // vclcompat TIniFile has no DeleteKey: strip the lines
{
    std::ifstream in(asGeneralPath.c_str(), std::ios::binary);
    std::string line, out;
    while (std::getline(in, line)) {
        std::string k = line.substr(0, line.find('='));
        for (size_t i = 0; i < k.size(); ++i) if (k[i] >= 'a' && k[i] <= 'z') k[i] = char(k[i] - 32);
        while (!k.empty() && (k[k.size() - 1] == ' ' || k[k.size() - 1] == '\t')) k.erase(k.size() - 1);
        if (k == "IN_ARM_DROP_CYL" || k == "OUT_ARM_DROP_CYL") continue;
        out += line + "\n";
    }
    in.close();
    std::ofstream o(asGeneralPath.c_str(), std::ios::binary | std::ios::trunc);
    o << out;
}
static void ClearNames()
{
    for (int arm = 0; arm < 2; ++arm) for (int k = 0; k < 4; ++k) Cylinder[Idx(arm, k)].CylinderName = "";
    Cylinder[C_OutArmSmallY].CylinderName = "";
}
static std::string ReadAll(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    std::string s = ss.str(), t;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] != 13) t += s[i];   // EOL-agnostic pins
    return t;
}
static int Count(const std::string& s, const std::string& what)
{
    int n = 0;
    for (size_t p = s.find(what); p != std::string::npos; p = s.find(what, p + what.size())) ++n;
    return n;
}

int main(int argc, char** argv)
{
    const char* const rt[] = { "asGeneralPath", asGeneralPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("DropCyl", rt))
        return 2;
    MyLaneIO.SetBackends(&g_io, 0, 0);
    const int itemIn = InArmSuck.Item[0][0], itemOut = OutArmSuck.Item[0][0];
    InArmSuck.Item[0][0] = NULL_IC; OutArmSuck.Item[0][0] = NULL_IC;

    std::printf(" 1. switches absent (= 0)\n");
    DropSwitchKeys();
    W906_DropCylReread();
    ClearNames();
    W906_DropCylNames();
    bool anyName = false;
    for (int arm = 0; arm < 2; ++arm) for (int k = 0; k < 4; ++k) if (Cylinder[Idx(arm, k)].CylinderName != "") anyName = true;
    CHECK(!anyName);
    for (int arm = 0; arm < 2; ++arm) for (int k = 0; k < 4; ++k) Bind(arm, k, true);   // even bound, the switch off touches nothing
    Sensors(0, true); Sensors(1, true);
    g_io.writes = 0;
    OutArmSuck.Item[0][0] = HAS_IC;
    CHECK(W906_DropCylOpen(0) && W906_DropCylOpen(1) && W906_DropCylClose(0) && W906_DropCylClose(1));
    CHECK(g_io.writes == 0);
    OutArmSuck.Item[0][0] = NULL_IC;

    std::printf(" 2. the names\n");
    SetSwitches(1, 0);
    ClearNames();
    W906_DropCylNames();
    CHECK(Cylinder[C_InPnPDrop1].CylinderName == "C_InPnPDrop1" && Cylinder[C_InPnPDrop4].CylinderName == "C_InPnPDrop4");
    CHECK(Cylinder[C_OutPnPDrop1].CylinderName == "" && Cylinder[C_OutArmSmallY].CylinderName == "");
    SetSwitches(1, 1);
    ClearNames();
    W906_DropCylNames();
    CHECK(Cylinder[C_OutPnPDrop1].CylinderName == "C_OutPnPDrop1" && Cylinder[C_OutPnPDrop4].CylinderName == "C_OutPnPDrop4");
    CHECK(Cylinder[C_InPnPDrop2].CylinderName == "C_InPnPDrop2" && Cylinder[C_OutArmSmallY].CylinderName == "");

    std::printf(" 3. Open\n");
    for (int arm = 0; arm < 2; ++arm) for (int k = 0; k < 4; ++k) Bind(arm, k, true);
    for (int k = 0; k < 4; ++k) MyLaneIO.IOBitOn(kRing, kIp, OutPort(1, k), 0, ePCI1203, "t");   // Out vanes closed (output on)
    Sensors(1, true);
    CHECK(!W906_DropCylOpen(1));                                                // popped, _Off not yet
    CHECK(AllOut(1, 0));                                                        // output off = Pop
    CHECK(!W906_DropCylOpen(1));
    Sensors(1, false);
    CHECK(Drive([] { return W906_DropCylOpen(1); }) == 1);
    g_io.writes = 0;
    CHECK(W906_DropCylOpen(1) && g_io.writes == 0);                             // already open: not driven again

    std::printf(" 4. Close\n");
    OutArmSuck.Item[0][0] = NULL_IC;
    g_io.writes = 0;
    CHECK(W906_DropCylClose(1) && g_io.writes == 0);                            // no IC: stays open
    OutArmSuck.Item[0][0] = HAS_IC;
    CHECK(!W906_DropCylClose(1));                                               // pushed, _On not yet
    CHECK(AllOut(1, 1));
    Sensors(1, true);
    CHECK(Drive([] { return W906_DropCylClose(1); }) == 1);
    g_io.writes = 0;
    CHECK(W906_DropCylClose(1) && g_io.writes == 0);                            // already closed
    // the In Arm the same way (its own switch, its own kit)
    InArmSuck.Item[0][0] = HAS_IC;
    Sensors(0, false);
    for (int k = 0; k < 4; ++k) MyLaneIO.IOBitOff(kRing, kIp, OutPort(0, k), 0, ePCI1203, "t");
    CHECK(!W906_DropCylClose(0) && AllOut(0, 1));
    Sensors(0, true);
    CHECK(Drive([] { return W906_DropCylClose(0); }) == 1);
    CHECK(!W906_DropCylOpen(0) && AllOut(0, 0));                                // the next Z-down opens it again
    Sensors(0, false);
    CHECK(Drive([] { return W906_DropCylOpen(0); }) == 1);
    InArmSuck.Item[0][0] = NULL_IC;

    std::printf(" 5. a cylinder the IO table does not have\n");
    Bind(1, 3, false);
    for (int k = 0; k < 3; ++k) MyLaneIO.IOBitOn(kRing, kIp, OutPort(1, k), 0, ePCI1203, "t");
    Sensors(1, true);
    const int before = g_io.Out(kIp, OutPort(1, 3));
    CHECK(!W906_DropCylOpen(1));
    Sensors(1, false);
    CHECK(Drive([] { return W906_DropCylOpen(1); }) == 1);
    CHECK(g_io.Out(kIp, OutPort(1, 3)) == before);                              // never driven
    Bind(1, 3, true);

    std::printf(" 6. HOME\n");
    CHECK(CynForHome == 82);
    int found = 0;
    for (int arm = 0; arm < 2; ++arm) for (int k = 0; k < 4; ++k)
        for (int i = 0; i < CynForHome; ++i) if (CynNeedHome[i] == Idx(arm, k)) { ++found; break; }
    CHECK(found == 8);
    CHECK(CynNeedHome[73] == C_Auto6Separate);                                  // the golden 74 unchanged in front

    std::printf(" 7. source pins\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::string mm = ReadAll(root + "/Motor/mymotor.cpp");
    const std::string a9 = ReadAll(root + "/ainarm9045.cpp");
    const std::string sp = ReadAll(root + "/ainarm_SearchPickPlate.cpp");
    const std::string ao = ReadAll(root + "/aoutarm9045_All_1Picker.cpp");
    const std::string ci = ReadAll(root + "/cinitial.cpp");
    CHECK(!mm.empty() && !a9.empty() && !sp.empty() && !ao.empty() && !ci.empty());
    CHECK(Count(mm, "bool OutArmZMoveDown(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool bLoader, bool bPreOn)//Steven for HT1032\n{   extern bool W906_DropCylOpen(int); if(!W906_DropCylOpen(1)) return false;") == 1);
    CHECK(Count(mm, "bool InArmZMoveDown(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool bLoader, bool bPreOn)//Steven for HT1032\n{   extern bool W906_DropCylOpen(int); if(!W906_DropCylOpen(0)) return false;") == 1);
    CHECK(Count(a9, "{   extern bool W906_DropCylOpen(int); if(!W906_DropCylOpen(0)) return false;") == 1);       // MoveInArmZToLoaderPick
    CHECK(Count(sp, "{   extern bool W906_DropCylOpen(int); if(!W906_DropCylOpen(0)) return false;") == 1);       // MoveInArmZToHotPlatePick
    CHECK(Count(a9, "case 2100:\n            extern bool W906_DropCylClose(int); if(W906_DropCylClose(0) && ArmFinishForLoader())") == 1);
    CHECK(Count(sp, "case 340:\n            extern bool W906_DropCylClose(int); if(MoveInArmZToPlateSafe(Task) && W906_DropCylClose(0))") == 1);
    CHECK(Count(ao, "{ extern bool W906_DropCylClose(int); if(!W906_DropCylClose(1)) break; }  iOutRotateFinish=3;") == 1);
    CHECK(Count(ci, "InitialCylinderName();  { extern void W906_DropCylNames(); W906_DropCylNames(); }") == 1);
    CHECK(Count(mm + a9 + sp + ao, "W906_DropCylClose(") == 3 + 3);             // three calls + three declarations, nowhere else

    MyLaneIO.SetBackends(0, 0, 0);
    InArmSuck.Item[0][0] = itemIn; OutArmSuck.Item[0][0] = itemOut;
    SetSwitches(0, 0);
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
