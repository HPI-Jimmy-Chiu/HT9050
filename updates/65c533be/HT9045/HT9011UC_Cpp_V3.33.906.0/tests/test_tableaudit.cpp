// test_tableaudit.cpp -- AI(W906-E043) 20261004 [W906] (St01 ST01-E2): ctest TableAudit -- the boot table audit's rules (TableAudit.h,
//   Steven 1004 07:2x Q96 = B, rule table approved 07:4x). Pure: links TableAudit.cpp only, feeds it table / ini TEXT, reads no file
//   and touches no machine state.
//   [0] a clean base (IO + Mot + Pci1203Axis.ini + Pci1203Modules.ini) gives 0 ERROR / 0 WARN
//   [1..12] one bad sample per rule: the rule's finding appears at its level (deleting that rule's code turns its check red)
//   [13] the summary line, the first-error text and the finding-line format
//   (EastSun's live dispatch-7 tables are not in the repo; they were run once by hand with this audit: see the commit message.)
#include "TableAudit.h"
#include "EtherCAT/Pci1203ModuleCheck.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace ht9045;

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); }
    else    { ++g_fail; std::printf("  FAIL: %s\n", what.c_str()); }
}

static const char* const kIoHead  = "IOType,Alias,Lane,ModuleType,IP,Port,Bit,InType,ISABase,Enable,OnAlarmTime,OffAlarmTime,OnDelayTime,OffDelayTime,Note\r\n";
static const char* const kMotHead = "Motorname,Alias,SoftLimitN,SoftLimitP,BoardID,Port,IP,Direction,GearRatio,HomeDirectior,HomeHighSpeed,HomeLowSpeed,"
                                    "InitSpeed,JogHighSpeed,JogLowSpeed,Rate,Enable,ServoAlarmOn,Range,1P2P,SensorType,SimulateSpeed,CardModel,Acc,Dec,"
                                    "EncodeType,PickLimit,LimitLogic,In1Logic\r\n";

static std::string IoRow(const std::string& type, const std::string& alias, int ip, int port, int bit, int enable, const std::string& times = ",,,")
{
    char b[256];
    std::snprintf(b, sizeof(b), "%s,%s,1,1,%d,%d,%d,1,3,%d,%s,\r\n", type.c_str(), alias.c_str(), ip, port, bit, enable, times.c_str());
    return b;
}
static std::string MotRow(const std::string& no, const std::string& alias, const std::string& board, const std::string& port,
                          const std::string& gear, int enable, const std::string& card)
{
    return no + "," + alias + ",-999999,999999," + board + "," + port + ",,0," + gear + ",0,200000,50000,1000,10000,100,90," + std::to_string(enable)
           + ",1,100,1,0,10000," + card + ",100000,100000,2,,0,0\r\n";
}

static std::string BaseIo()
{
    return std::string(kIoHead)
        + IoRow("Cylinder", "C_A", 16, 7, 7, 1, "50,10,1,1") + IoRow("Cylinder_On", "C_A_On", 18, 15, 7, 1) + IoRow("Cylinder_Off", "C_A_Off", 18, 14, 6, 1)
        + IoRow("Sensor", "SnX", 17, 0, 1, 1) + IoRow("Switch", "SwY", 16, 8, 0, 1) + IoRow("Sucker", "SuckA", 160, 64, 0, 1)
        + IoRow("Sucker_On", "SuckA_On", 160, 64, 1, 1) + IoRow("Sucker_Off", "SuckA_Off", 160, 64, 2, 1);
}
static std::string BaseMot()
{
    return std::string(kMotHead) + MotRow("M00", "MInArmX", "0", "0", "1", 1, "PCI1203") + MotRow("M01", "MInArmY", "1", "0", "1", 1, "PCI1203")
           + MotRow("M37", "MColorZ", "", "", "0.9", 0, "SMC");
}
static std::string ModulesIni(const std::vector<int>& ioStations)
{
    std::vector<Pci1203ModuleEntry> e;
    for (std::size_t i = 0; i < ioStations.size(); ++i) {
        Pci1203ModuleEntry m;
        m.kind = kPci1203ModIo; m.ring = 1; m.position = (int)i; m.station = ioStations[i]; m.name = "IO";
        e.push_back(m);
    }
    return Pci1203ModuleIniSerialize(e, "test");
}
static TableAuditInput Base()
{
    TableAuditInput in;
    in.ioFound = true;  in.ioCsv = BaseIo();
    in.motFound = true; in.motCsv = BaseMot();
    in.ht9050 = true; in.ioCardType = 4; in.autoEmptyColor = 0;
    in.axisIniFound = true; in.axisIni = "[station0.axis0]\r\npelLogic=0\r\nmelLogic=0\r\n";
    in.modulesIniFound = true; in.modulesIni = ModulesIni(std::vector<int>{ 16, 17, 18, 160 });
    return in;
}
static int Count(const TableAuditResult& r, const std::string& rule, int level)
{
    int n = 0;
    for (std::size_t i = 0; i < r.findings.size(); ++i) if (r.findings[i].rule == rule && r.findings[i].level == level) ++n;
    return n;
}
static std::string Dump(const TableAuditResult& r)
{
    std::string s;
    for (std::size_t i = 0; i < r.findings.size(); ++i) s += "\n      " + TableAuditFindingLine(r.findings[i]);
    return s;
}
static void Expect(const char* tag, const TableAuditInput& in, const std::string& rule, int level, const std::string& why)
{
    const TableAuditResult r = TableAuditRunPure(in);
    Check(Count(r, rule, level) >= 1, std::string(tag) + " " + rule + " " + (level == kAuditError ? "ERROR" : level == kAuditWarn ? "WARN" : "INFO") + ": " + why
          + (Count(r, rule, level) >= 1 ? std::string("") : Dump(r)));
}

int main()
{
    std::printf("[0] clean base\n");
    {
        const TableAuditResult r = TableAuditRunPure(Base());
        Check(r.errors == 0 && r.warns == 0, "[0] base tables: 0 ERROR, 0 WARN (got " + std::to_string(r.errors) + " / " + std::to_string(r.warns) + ")" + Dump(r));
        Check(TableAuditFirstError(r).empty(), "[0] no first error");
    }

    std::printf("[1] T1\n");
    { TableAuditInput in = Base(); in.ioCardType = 2; Expect("[1a]", in, "T1", kAuditError, "HT9050 with IO_CARD_TYPE 2"); }
    { TableAuditInput in = Base(); in.ioFound = false; in.ioCsv.clear(); Expect("[1b]", in, "T1", kAuditError, "IO_Table.csv missing"); }
    { TableAuditInput in = Base(); in.ioCsv = "IOType,Alias,Lane\r\nSensor,SnX,1\r\n"; Expect("[1c]", in, "T1", kAuditError, "IO header with 3 cells"); }
    { TableAuditInput in = Base(); in.motCsv = std::string(kMotHead).substr(0, std::string(kMotHead).size() - 10) + "\r\n"; Expect("[1d]", in, "T1", kAuditError, "Mot header cut"); }
    { TableAuditInput in = Base(); in.ht9050 = false; in.ioCardType = 2;
      const TableAuditResult r = TableAuditRunPure(in); Check(Count(r, "T1", kAuditError) == 0, "[1e] not HT9050, IO_CARD_TYPE 2: no T1"); }

    std::printf("[2] T2a / T2b\n");
    { TableAuditInput in = Base(); in.motCsv = std::string(kMotHead) + MotRow("M00", "MInArmX", "0", "0", "0", 1, "PCI1203");
      Expect("[2a]", in, "T2a", kAuditError, "Enable-1 GearRatio 0"); }
    { TableAuditInput in = Base(); in.motCsv = std::string(kMotHead) + MotRow("M00", "MInArmX", "0", "0", "0.0.071425", 1, "PCI1203");
      Expect("[2b]", in, "T2a", kAuditError, "Enable-1 GearRatio 0.0.071425"); }
    { TableAuditInput in = Base(); in.motCsv = std::string(kMotHead) + MotRow("M37", "MColorZ", "37", "0", "0.0.071425", 0, "SMC");
      Expect("[2c]", in, "T2b", kAuditWarn, "Enable-0 GearRatio 0.0.071425 (M37 today)"); }
    { TableAuditInput in = Base(); in.ioCsv = std::string(kIoHead) + "Sensor,SnX,1,1,17,+-,1,1,3,1,,,,,\r\n";
      Expect("[2d]", in, "T2a", kAuditError, "Enable-1 IO Port '+-'"); }
    { TableAuditInput in = Base(); in.ioCsv = std::string(kIoHead) + IoRow("Sensor", "SnX", 17, 0, 1, 1, "5,x,,");
      Expect("[2e]", in, "T2b", kAuditWarn, "alarm time 'x'"); }

    std::printf("[3] T3\n");
    { TableAuditInput in = Base(); in.ioCsv = std::string(kIoHead) + "Sensor,SnX,1,1,17,,1,1,3,1,,,,,\r\n";
      Expect("[3a]", in, "T3", kAuditWarn, "IO Enable 1 with Port empty"); }
    { TableAuditInput in = Base(); in.motCsv = std::string(kMotHead) + MotRow("M13", "MTestY1", "", "", "1", 1, "PCI1203");
      Expect("[3b]", in, "T3", kAuditWarn, "Mot Enable 1 with BoardID / Port empty"); }

    std::printf("[4] T4\n");
    { TableAuditInput in = Base(); in.motCsv = std::string(kMotHead) + MotRow("M30", "MTrayX", "30", "1", "1", 1, "PCI1203") + MotRow("M18", "MOutShuttle2", "30", "1", "1", 1, "PCI1203");
      Expect("[4]", in, "T4", kAuditError, "two PCI1203 axes at 30/1"); }

    std::printf("[5] T5a / T5b\n");
    { TableAuditInput in = Base(); in.ioCsv = BaseIo() + IoRow("Cylinder", "C_B", 16, 7, 7, 1);
      Expect("[5a]", in, "T5a", kAuditError, "two Enable-1 outputs at 3/16/7/7"); }
    { TableAuditInput in = Base(); in.ioCsv = BaseIo() + IoRow("Sucker_Off", "SuckB_Off", 160, 64, 2, 1);
      Expect("[5b]", in, "T5a", kAuditError, "Sucker_Off is an output"); }
    { TableAuditInput in = Base(); in.ioCsv = BaseIo() + IoRow("Sensor", "SnSafeDoorIndex", 17, 0, 1, 1);
      Expect("[5c]", in, "T5b", kAuditWarn, "two Enable-1 inputs at 3/17/0/1 (SnSafeDoor1 = SnSafeDoorIndex)"); }

    std::printf("[6] T6\n");
    { TableAuditInput in = Base(); in.ioCsv = std::string(kIoHead) + IoRow("Cylinder", "C_A", 16, 7, 7, 1) + IoRow("Cylinder_On", "C_A_On", 18, 15, 7, 0) + IoRow("Cylinder_Off", "C_A_Off", 18, 14, 6, 0);
      Expect("[6a]", in, "T6", kAuditWarn, "output 1, sensors 0"); }
    { TableAuditInput in = Base(); in.ioCsv = std::string(kIoHead) + IoRow("Cylinder", "C_A", 16, 7, 7, 0) + IoRow("Cylinder_On", "C_A_On", 18, 15, 7, 1) + IoRow("Cylinder_Off", "C_A_Off", 18, 14, 6, 1);
      Expect("[6b]", in, "T6", kAuditWarn, "output 0, sensors 1 (Floodgate case)"); }

    std::printf("[7] T7\n");
    { TableAuditInput in = Base(); in.ioCsv = BaseIo() + IoRow("Cylinder", "C_Empty_Up", 80, 1, 0, 1);
      Expect("[7a]", in, "T7", kAuditWarn, "AUTO_EMPTY_COLOR 0 disables C_Empty_Up (Enable 1 in the table)"); }
    { TableAuditInput in = Base(); in.autoEmptyColor = 1;
      Expect("[7b]", in, "T7", kAuditWarn, "AUTO_EMPTY_COLOR 1 enables the Empty / Color cylinders the table lacks"); }
    { TableAuditInput in = Base(); in.autoEmptyColor = 1; in.loadZMotorEmpty = true;
      in.ioCsv = BaseIo() + IoRow("Cylinder", "C_EmptyLoaderZ_Select", 80, 2, 0, 1) + IoRow("Cylinder", "C_ColorLoaderZ_Select", 80, 2, 1, 1)
               + IoRow("Cylinder", "C_Empty_Fix", 80, 2, 2, 1) + IoRow("Cylinder", "C_Color_Fix", 80, 2, 3, 1) + IoRow("Cylinder", "C_Color_Up", 80, 2, 4, 1)
               + IoRow("Cylinder", "C_Color_Middle", 80, 2, 5, 1) + IoRow("Cylinder", "C_Empty_Up", 80, 2, 6, 1);
      const TableAuditResult r = TableAuditRunPure(in);
      Check(Count(r, "T7", kAuditWarn) == 0, "[7c] AUTO_EMPTY_COLOR 1 + LOAD_Z_USE_MOTOR[1]: 7 rows Enable 1, C_Empty_Middle absent and forced off -> no T7"
            + (Count(r, "T7", kAuditWarn) == 0 ? std::string("") : Dump(r))); }

    std::printf("[8] T8\n");
    { TableAuditInput in = Base(); in.ioCsv = BaseIo() + IoRow("Sensor", "SnX", 17, 3, 3, 1);
      Expect("[8a]", in, "T8", kAuditWarn, "duplicate Alias SnX"); }
    { TableAuditInput in = Base(); in.ioCsv = BaseIo() + IoRow("Sensor", "", 4, 0, 4, 0);
      Expect("[8b]", in, "T8", kAuditWarn, "typed row without Alias"); }
    { TableAuditInput in = Base(); in.ioCsv = BaseIo() + "Sensor,Sn Z,1,1,17,1,1,1,3,1,,,,,\r\n";
      Expect("[8c]", in, "T8", kAuditWarn, "a space inside a cell"); }
    { TableAuditInput in = Base(); in.motCsv = BaseMot() + MotRow("M00", "MInArmX2", "5", "0", "1", 0, "SMC");
      Expect("[8d]", in, "T8", kAuditWarn, "duplicate Motorname M00"); }

    std::printf("[9] T9\n");
    { TableAuditInput in = Base(); in.motCsv = BaseMot() + MotRow("M13", "MTestY1", "13", "0", "1", 1, "SMC");
      Expect("[9a]", in, "T9", kAuditError, "HT9050, Enable-1 SMC axis"); }
    { TableAuditInput in = Base(); in.ht9050 = false; in.motCsv = BaseMot() + MotRow("M13", "MTestY1", "13", "0", "1", 1, "SMC");
      const TableAuditResult r = TableAuditRunPure(in); Check(Count(r, "T9", kAuditError) == 0, "[9b] not HT9050: SMC is fine"); }

    std::printf("[10] T10\n");
    { TableAuditInput in = Base(); in.codeMotors.push_back("M00"); in.codeMotors.push_back("M99"); in.codeIoNames.push_back("SnX"); in.codeIoNames.push_back("SnNotThere");
      const TableAuditResult r = TableAuditRunPure(in);
      const bool ok10 = Count(r, "T10", kAuditInfo) == 2 && r.errors == 0 && r.warns == 0;
      Check(ok10, "[10] M99 and SnNotThere reported as INFO only" + (ok10 ? std::string("") : Dump(r))); }

    std::printf("[11] T11\n");
    { TableAuditInput in = Base(); in.axisIni = "[station0.axis0]\r\nmelLogic=0\r\n"; Expect("[11a]", in, "T11", kAuditWarn, "melLogic only (station0 today)"); }
    { TableAuditInput in = Base(); in.axisIni = "[station9.axis0]\r\npelLogic=1\r\nmelLogic=1\r\n"; Expect("[11b]", in, "T11", kAuditWarn, "station 9 maps to no axis"); }
    { TableAuditInput in = Base(); in.axisIni = "[station0.axis0]\r\npelLogic=2\r\nmelLogic=0\r\n"; Expect("[11c]", in, "T11", kAuditWarn, "pelLogic=2"); }

    std::printf("[12] T12\n");
    { TableAuditInput in = Base(); in.modulesIni = ModulesIni(std::vector<int>{ 16, 17 }); Expect("[12a]", in, "T12", kAuditWarn, "stations 18 / 160 not listed"); }
    { TableAuditInput in = Base(); in.modulesIniFound = false; Expect("[12b]", in, "T12", kAuditInfo, "no Pci1203Modules.ini -> INFO"); }

    std::printf("[13] summary / first error\n");
    {
        TableAuditInput in = Base(); in.ioCardType = 2;
        const TableAuditResult r = TableAuditRunPure(in);
        const std::string s = TableAuditSummaryLine(r);
        Check(s.find("[BOOT] 驗表：ERROR 1") == 0, "[13a] summary line starts with [BOOT] 驗表：ERROR 1 (" + s + ")");
        Check(TableAuditFirstError(r).compare(0, 3, "T1 ") == 0, "[13b] first error is T1 (" + TableAuditFirstError(r) + ")");
        Check(TableAuditFindingLine(r.findings[0]).compare(0, 21, "TABLEAUDIT ERROR T1 [") == 0, "[13c] finding line format");
    }

    std::printf("%d / %d passed\n", g_pass, g_pass + g_fail);
    return g_fail ? 1 : 0;
}
