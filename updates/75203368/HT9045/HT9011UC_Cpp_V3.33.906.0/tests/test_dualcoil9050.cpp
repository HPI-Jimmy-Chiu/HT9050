// =============================================================================
//  test_dualcoil9050.cpp -- AI(W906-DUALCOIL9050) 20261008
//
//  EastSun 1008: "ht9050 的汽缸都是用五口三位的 ... Cylinder[C_LoaderEdgePush].Pop() 會等於 Cylinder[C_LoaderEdgePush].Off() &&
//  Cylinder[C_LoaderEdgePushOff].On()，Push() 則與 Pop 相反". Drives the REAL TMyCylinder (mycylin.cpp) and the REAL hook
//  (JsonBridge/W906DualCoil9050.cpp) and reads both coils back from MyLaneIO's output cache (1203 addresses, as on the machine:
//  C_LoaderEdgePush = lane 1 / IP 178 / port 1 / bit 1, C_LoaderEdgePushOff = lane 1 / IP 178 / port 0 / bit 0, InType 1).
//
//  [1] HT9050: Pop / Off -> On coil off, Off coil on; Push / On -> On coil on, Off coil off
//  [2] the Off coil is released BEFORE the On coil is energised (Push), and energised AFTER the On coil is released (Pop)
//  [3] not HT9050 -> the Off coil is never written (golden)
//  [4] no write when the "<name>Off" row is Enable=0, is not IOType "Cylinder", or is itself an engine Cylinder[]
//  [5] a disabled cylinder writes neither coil
// =============================================================================
#include <cstdio>
#include <string>
#include <vector>

#include "cmydef.h"
#include "MachineType.h"
#include "database.h"
#include "MyLaneIo.h"
#include "mycylin.h"

extern AnsiString W906_GpibModel;
extern void (*g_W906PairedCoilHook)(const AnsiString&, bool);

static int g_pass = 0, g_fail = 0;
#define CHECK(c) do { if (c) ++g_pass; else { ++g_fail; std::printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

namespace {
const int kLane = 1, kIP = 178;

void AddRow(const char* type, const char* alias, int port, int bit, int enable)
{
    TIODATA* row = new TIODATA(AnsiString(""));
    row->Type = type; row->Alias = alias;
    row->iEnable = enable; row->iISABase = ePCI1203; row->iLane = kLane; row->iIP = kIP;
    row->iPort = port; row->iBit = bit; row->iInType = 1;
    const int idx = (int)HSys.IOTable.size();
    HSys.IOTable.push_back(row);
    HSys.mapIOTable[AnsiString(alias)] = AnsiString(idx);
}

void SetupCyl(int c, const char* name, int port, int bit)
{
    TMyCylinder& y = Cylinder[c];
    y.Enable = true; y.CylinderName = name;
    y.OutRing = kLane; y.OutIP = kIP; y.OutPort = port; y.OutBit = bit; y.OutType = TYPE_A; y.OutISABase = ePCI1203;
    y.OnSenEnable = false; y.OffSenEnable = false; y.OnDelayTime = 0; y.OffDelayTime = 0;
    y.Reset();
}

bool Bit(int port, int bit) { return MyLaneIO.IOOutBitStatus(kLane, kIP, port, bit, ePCI1203, "t"); }
void Clear(int port, int bit) { MyLaneIO.IOBitOff(kLane, kIP, port, bit, ePCI1203, "t"); }

// [2]: wrap the installed hook and record the On coil's state at the moment the Off coil is written.
void (*s_real)(const AnsiString&, bool) = 0;
std::vector<std::string> s_log;
int s_onPort = 1, s_onBit = 1;
void Spy(const AnsiString& name, bool energise)
{
    s_log.push_back(std::string(energise ? "off-coil ON  with on-coil=" : "off-coil OFF with on-coil=") + (Bit(s_onPort, s_onBit) ? "1" : "0"));
    s_real(name, energise);
}
}  // namespace

int main()
{
    std::printf("test_dualcoil9050 -- HT9050 five-port three-position cylinders\n");
    CHECK(g_W906PairedCoilHook != 0);                                   // W906DualCoil9050.cpp registered itself

    // machine addresses (D:\HT9045\system\IO_Table.csv 20261008)
    AddRow("Cylinder", "C_LoaderEdgePush",    1, 1, 1);
    AddRow("Cylinder", "C_LoaderEdgePushOff", 0, 0, 1);
    AddRow("Cylinder", "C_LoaderEdgeClip",    3, 3, 1);
    AddRow("Cylinder", "C_LoaderEdgeClipOff", 2, 2, 0);                // [4] Enable=0
    AddRow("Cylinder", "C_TrayZ_Selector",    5, 5, 1);
    AddRow("Sensor",   "C_TrayZ_SelectorOff", 4, 4, 1);                // [4] not an output row
    AddRow("Cylinder", "C_Load_Up",           7, 7, 1);
    AddRow("Cylinder", "C_Load_UpOff",        6, 6, 1);                // [4] owned by an engine Cylinder[] below
    SetupCyl(C_LoaderEdgePush, "C_LoaderEdgePush", 1, 1);
    SetupCyl(C_LoaderEdgeClip, "C_LoaderEdgeClip", 3, 3);
    SetupCyl(C_TrayZ_Selector, "C_TrayZ_Selector", 5, 5);
    SetupCyl(C_Load_Up,        "C_Load_Up",        7, 7);
    Cylinder[C_Empty_Up].CylinderName = "C_Load_UpOff";                // stand-in for golden's C_DockXAxisOff style engine pair

    std::printf("[1] HT9050: Pop / Off energise <name>Off, Push / On release it\n");
    W906_GpibModel = "9050GPIB";
    CHECK(Cylinder[C_LoaderEdgePush].Pop());
    CHECK(Bit(1, 1) == false); CHECK(Bit(0, 0) == true);
    CHECK(Cylinder[C_LoaderEdgePush].Push());
    CHECK(Bit(1, 1) == true);  CHECK(Bit(0, 0) == false);
    Cylinder[C_LoaderEdgePush].Off();
    CHECK(Bit(1, 1) == false); CHECK(Bit(0, 0) == true);
    Cylinder[C_LoaderEdgePush].On();
    CHECK(Bit(1, 1) == true);  CHECK(Bit(0, 0) == false);

    std::printf("[2] order: Off coil released before the On coil goes on, energised after it goes off\n");
    s_real = g_W906PairedCoilHook; g_W906PairedCoilHook = &Spy; s_log.clear();
    Cylinder[C_LoaderEdgePush].Pop();                                   // On coil was on
    Cylinder[C_LoaderEdgePush].Push();                                  // On coil was off
    g_W906PairedCoilHook = s_real;
    CHECK(s_log.size() == 2);
    if (s_log.size() == 2) {
        CHECK(s_log[0] == "off-coil ON  with on-coil=0");                // Pop: On coil already off
        CHECK(s_log[1] == "off-coil OFF with on-coil=0");                // Push: On coil not yet on
        for (size_t i = 0; i < s_log.size(); ++i) std::printf("    %s\n", s_log[i].c_str());
    }

    std::printf("[3] not HT9050: golden, the Off coil is not touched\n");
    W906_GpibModel = "";
    const int savedType = MachineTypeChoice;
    if (MachineTypeChoice == Type_HT9050) MachineTypeChoice = 0;
    Clear(0, 0);
    Cylinder[C_LoaderEdgePush].Pop();
    CHECK(Bit(1, 1) == false); CHECK(Bit(0, 0) == false);
    MachineTypeChoice = savedType;
    W906_GpibModel = "9050GPIB";

    std::printf("[4] no write: Enable=0 row / non-Cylinder row / engine-owned name\n");
    Clear(2, 2); Clear(4, 4); Clear(6, 6);
    Cylinder[C_LoaderEdgeClip].Pop(); CHECK(Bit(2, 2) == false);
    Cylinder[C_TrayZ_Selector].Pop(); CHECK(Bit(4, 4) == false);
    Cylinder[C_Load_Up].Pop();        CHECK(Bit(6, 6) == false);
    CHECK(Bit(3, 3) == false); CHECK(Bit(5, 5) == false); CHECK(Bit(7, 7) == false);   // the On coils still follow golden

    std::printf("[5] a disabled cylinder writes neither coil\n");
    Cylinder[C_LoaderEdgePush].Push();
    CHECK(Bit(1, 1) == true); CHECK(Bit(0, 0) == false);
    Cylinder[C_LoaderEdgePush].Enable = false;
    Cylinder[C_LoaderEdgePush].Pop();
    CHECK(Bit(1, 1) == true); CHECK(Bit(0, 0) == false);

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    std::printf(g_fail ? "FAIL\n" : "ALL PASS\n");
    return g_fail ? 1 : 0;
}
