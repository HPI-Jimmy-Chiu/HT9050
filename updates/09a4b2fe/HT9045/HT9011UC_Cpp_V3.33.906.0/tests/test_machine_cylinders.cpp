// ===========================================================================
//  tests/test_machine_cylinders.cpp
//
//  AI(W906-W3-INITCYL) 20260924: 週末計畫 W3 第 7 項（InitialCylinderName／InitCylinder）的
//  「第三級：值有被維護」測試。
//
//  golden InitCylinder 的 if（IO_CARD_TYPE==NewIO_MN200 || PCI_P64C64）那半（移植 cinitial.cpp:4859 起），
//  對每個有名稱的氣缸查 IO 表三次：
//      <名稱>        -> Out*    （OutRingUse／OutRing／OutIP／OutPort／OutBit／OutType／OutISABase）
//      <名稱>_On     -> OnSen*  （同上 7 欄）
//      <名稱>_Off    -> OffSen* （同上 7 欄）
//  這支測試拿版控的 machines/HT9050/IO_Table.csv（argv[1]）餵進去，逐列逐欄比對，要求零不符。
//  Enable 系列另有 golden 的額外規則（位址全 0、Port／Bit 為 0 就關；尾段對 Empty／Color／Auto 強制開），
//  那不是「位址有沒有寫進去」的問題，這裡不比。
//
//  用法：test_machine_cylinders <IO_Table.csv>   [AI(W906-F9050-BD) 20261004: [<MachineTypeChoice>]]
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "database.h"
#include "common.h"
#include "cinitial.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "mycylin.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <set>
#include <vector>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_machine_cylinders.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static TIODATA* Row(const AnsiString& alias)
{
    HSys.mapIOTableIter = HSys.mapIOTable.find(alias);
    if (HSys.mapIOTableIter == HSys.mapIOTable.end()) return 0;
    return HSys.IOTable[HSys.mapIOTableIter->second.ToIntDef(0)];
}
static AnsiString UseOf(const TIODATA* r) { return (r->iPort == -1) ? AnsiString("") : AnsiString(r->iPort); }

// AI(W906-POOL5-4) 20261006 (Ifor01): POOL-5 #4 (Frank FR-PR1 2C, RULINGS_20261006 #16; TO_IFOR 18:4x = A) -- no more hard-coded
//   73/55/55/260 (94/75/75/285 as Type_HT9050).  (a) The set of names InitialCylinderName assigns must equal the oracle
//   tests/oracle/cylinder_names_906.txt: G = golden 906's literals, A = recorded addition for every type, H = recorded additions for
//   Type_HT9050 only.  (b) The rows bound must be every IO_Table row whose Alias is <name> / <name>_On / <name>_Off of a named
//   cylinder, counted from the CSV read line by line (not through HSys.LoadIoData).  A table update moves (b) by itself; only a
//   deliberate naming change moves (a), and then the oracle line says why.
static bool LoadNameOracle(const std::string& path, bool as9050, std::set<std::string>& out)
{
    FILE* in = std::fopen(path.c_str(), "rb");
    if (!in) return false;
    char line[512];
    while (std::fgets(line, sizeof line, in)) {
        if ((line[0] != 'G' && line[0] != 'A' && line[0] != 'H') || line[1] != ' ') continue;
        if (line[0] == 'H' && !as9050) continue;
        std::string n(line + 2);
        while (!n.empty() && (n.back() == '\r' || n.back() == '\n' || n.back() == ' ')) n.pop_back();
        if (!n.empty()) out.insert(n);
    }
    std::fclose(in);
    return !out.empty();
}
static std::set<std::string> CsvAliases(const char* path)
{
    std::set<std::string> out;
    FILE* in = std::fopen(path, "rb");
    if (!in) return out;
    char line[4096];
    while (std::fgets(line, sizeof line, in)) {
        const char* c1 = std::strchr(line, ',');            // column 2 = Alias
        if (!c1) continue;
        const char* c2 = std::strchr(c1 + 1, ',');
        std::string a(c1 + 1, c2 ? c2 : c1 + 1 + std::strlen(c1 + 1));
        while (!a.empty() && (a.back() == ' ' || a.back() == '\r' || a.back() == '\n')) a.pop_back();
        while (!a.empty() && a[0] == ' ') a.erase(0, 1);
        if (!a.empty()) out.insert(a);
    }
    std::fclose(in);
    return out;
}

int main(int argc, char** argv)
{
    // AI(W906-POOL5-4): positional <IO_Table.csv> [<MachineTypeChoice>], plus --names=<oracle> anywhere
    std::string namesPath;
    std::vector<char*> pos;
    pos.push_back(argv[0]);
    for (int k = 1; k < argc; ++k) {
        if (std::strncmp(argv[k], "--names=", 8) == 0) namesPath = argv[k] + 8;
        else pos.push_back(argv[k]);
    }
    argc = (int)pos.size();
    argv = pos.data();
    if (argc < 2) { std::printf("usage: test_machine_cylinders <IO_Table.csv> [<MachineTypeChoice>] --names=<cylinder_names_906.txt>\n"); return 2; }
    const std::string env = std::string("W906_IOTABLE_PATH=") + argv[1];
    HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));
    HSys.LoadIoData();
    std::printf("IO_Table rows: %d\n", (int)HSys.IOTable.size());
    CHECK(HSys.IOTable.size() > 0);

    IO_CARD_TYPE = NewIO_MN200;          // 走 golden 的 if 那半（筆電 Gerneral.ini 的實際值）
    //AI(W906-F9050-BD) 20261004: argv[2] = MachineTypeChoice (not given = the static default, as before).  800 = Type_HT9050:
    //  InitialCylinderName then also names the 26 HT9050 cylinders 295-320 (only for that type, cinitial.cpp).
    const bool bMachineTypeGiven = (argc >= 3);
    if (bMachineTypeGiven) MachineTypeChoice = std::atoi(argv[2]);
    const bool bAs9050 = (MachineTypeChoice == Type_HT9050);
    iControlPanelMode = 0;
    Enable_PLCSafety_IO = false;
    InitCylinder();                      // 內部先呼叫 InitialCylinderName()（golden :4103）

    int named = 0, outRows = 0, outBad = 0, onRows = 0, onBad = 0, offRows = 0, offBad = 0;
    for (int i = 0; i < MaxCylinderItem; ++i) {
        TMyCylinder& C = Cylinder[i];
        if (C.CylinderName == AnsiString("")) continue;
        ++named;
        CHECK(C.OnSensorName  == C.CylinderName + "_On");
        CHECK(C.OffSensorName == C.CylinderName + "_Off");

        if (const TIODATA* r = Row(C.CylinderName)) {
            ++outRows;
            const bool ok = C.OutRingUse == UseOf(r) && C.OutRing == r->iLane && C.OutIP == r->iIP &&
                            C.OutPort == r->iPort && C.OutBit == r->iBit && C.OutType == r->iInType &&
                            C.OutISABase == r->iISABase;
            if (!ok) { if (outBad < 5) std::printf("  Out mismatch: %s\n", C.CylinderName.c_str()); ++outBad; }
        }
        if (const TIODATA* r = Row(C.OnSensorName)) {
            ++onRows;
            const bool ok = C.OnSenRingUse == UseOf(r) && C.OnSenRing == r->iLane && C.OnSenIP == r->iIP &&
                            C.OnSenPort == r->iPort && C.OnSenBit == r->iBit && C.OnSenType == r->iInType &&
                            C.OnSenISABase == r->iISABase;
            if (!ok) { if (onBad < 5) std::printf("  OnSen mismatch: %s\n", C.OnSensorName.c_str()); ++onBad; }
        }
        if (const TIODATA* r = Row(C.OffSensorName)) {
            ++offRows;
            const bool ok = C.OffSenRingUse == UseOf(r) && C.OffSenRing == r->iLane && C.OffSenIP == r->iIP &&
                            C.OffSenPort == r->iPort && C.OffSenBit == r->iBit && C.OffSenType == r->iInType &&
                            C.OffSenISABase == r->iISABase;
            if (!ok) { if (offBad < 5) std::printf("  OffSen mismatch: %s\n", C.OffSensorName.c_str()); ++offBad; }
        }
    }
    std::printf("cylinders named %d | Out %d rows %d bad | OnSen %d rows %d bad | OffSen %d rows %d bad\n",
                named, outRows, outBad, onRows, onBad, offRows, offBad);
    CHECK(named > 0); CHECK(C_StackedTrayLockOff != C_LoadRobotX); CHECK(Cylinder[C_StackedTrayLockOff].CylinderName == AnsiString("C_StackedTrayLockOff")); CHECK(Cylinder[C_LoadRobotX].CylinderName == AnsiString("C_LoadRobotX"));   // AI(W906-CYL75) 20260926: 撞號修正後兩名各有自己的槽（RULINGS_20260926 第 11 條）
    CHECK(outRows > 0 && outBad == 0);
    CHECK(onRows > 0 && onBad == 0);
    CHECK(offRows > 0 && offBad == 0);
    // AI(W906-W3-AUDIT) 20260925: 把數量釘住（W3 稽核：原本只要求 >0，宣稱的 72／54／54 沒被驗）。
    //   期望值由 Python 獨立算：InitialCylinderName（cinitial.cpp:7694 起）的名稱字面值 ∩ machines/HT9050/IO_Table.csv
    //   的 Alias，逐索引計；不是拿這支測試的輸出回填。
    //   named＝259（原 258）：golden cmydef.cpp:442/:444 的 C_StackedTrayLockOff 與 C_LoadRobotX 都是 75（906 與 V912 相同，golden 缺陷），後指派的 C_LoadRobotX 蓋掉前者。
    //   AI(W906-CYL75) 20260926 使用者裁決修正（RULINGS_20260926 第 11 條）：移植樹 cmydef.cpp:446 改 153 ⇒ 259 名稱綁 259 槽；有人改回 75 這裡會變回 258。72／54／54 不變（HT9050 表沒有這兩個名稱的列）。
    //   AI(W906-HT9050-RULE10) 20261005: 新增 C_OutArmSmallY（cmydef.cpp :690 =152，EastSun 規則 10）。HT9050 IO_Table.csv 有它的
    //   本體／_On／_Off 三列 ⇒ 72／54／54／259 各 +1 = 73／55／55／260（三種列 0 不符）。拿掉該汽缸這裡會變回舊值。
    //   AI(W906-MACH0210) 20261005: 整合進 main：C_OutArmSmallY 用 main 的 303（F9050-BD，cmydef.cpp:719，Eastsun 20260811；機台的 :690 =152 沒有收），
    //   名稱照機台的做法「每個機種都登記」（cinitial.cpp InitialCylinderName，C_OutShuttle2Floodgate 下一行）⇒ 非 Type_HT9050＝73／55／55／260（機台的數字）；
    //   Type_HT9050＝94／75／75／285 不變（303 本來就在 F9050-BD 的 26 個裡，同一槽登記兩次）。兩組都由 Python 獨立重算（名稱字面值 ∩ CSV Alias）。
    //   AI(W906-POOL5-4) 20261006 (Ifor01): the four numbers above (73/55/55/260, 94/75/75/285) are no longer pinned -- see
    //   LoadNameOracle / CsvAliases.  What they guarded stays guarded: a cylinder name dropped or misspelled breaks (a); the CYL75
    //   fix undone (C_StackedTrayLockOff and C_LoadRobotX back on one slot) loses C_StackedTrayLockOff and breaks (a) too; a
    //   lookup that stops binding rows breaks (b).
    {
        std::set<std::string> oracle, prog;
        const bool haveOracle = !namesPath.empty() && LoadNameOracle(namesPath, bAs9050, oracle);
        CHECK(haveOracle);                                               // --names=tests/oracle/cylinder_names_906.txt
        for (int i = 0; i < MaxCylinderItem; ++i)
            if (Cylinder[i].CylinderName != AnsiString("")) prog.insert(std::string(Cylinder[i].CylinderName.c_str()));
        int missing = 0, extra = 0;
        for (const std::string& n : oracle) if (!prog.count(n)) { if (missing++ < 8) std::printf("  name in the oracle but not assigned: %s\n", n.c_str()); }
        for (const std::string& n : prog) if (!oracle.count(n)) { if (extra++ < 8) std::printf("  name assigned but not in the oracle: %s\n", n.c_str()); }
        std::printf("names: assigned %d (slots %d), oracle %d (%s)\n", (int)prog.size(), named, (int)oracle.size(), bAs9050 ? "G+A+H" : "G+A");
        CHECK(missing == 0 && extra == 0);                               // (a) the name set = golden 906 + recorded additions
        CHECK(named == (int)prog.size());                                // one slot per name (CYL75: no two names on one slot)

        const std::set<std::string> alias = CsvAliases(argv[1]);
        int expOut = 0, expOn = 0, expOff = 0;
        for (const std::string& n : prog) {
            if (alias.count(n)) ++expOut;
            if (alias.count(n + "_On")) ++expOn;
            if (alias.count(n + "_Off")) ++expOff;
        }
        std::printf("rows bound: Out %d / On %d / Off %d, expected from the CSV %d / %d / %d\n", outRows, onRows, offRows, expOut, expOn, expOff);
        CHECK(expOut > 0 && outRows == expOut);                          // (b) every row of a named cylinder is bound
        CHECK(onRows == expOn && offRows == expOff);
    }
    CHECK(Cylinder[C_OutArmSmallY].CylinderName == AnsiString("C_OutArmSmallY"));   //AI(W906-MACH0210) 20261005: named for every type (machine cpp 0210 rule 10 / dry run)
    if (bMachineTypeGiven && !bAs9050) {                                 //AI(W906-F9050-BD) 20261004: any other type: slots 295-320 unnamed and disabled   //AI(W906-MACH0210) 20261005: except C_OutArmSmallY (303), named for every type
        int named9050 = 0, enabled9050 = 0;
        for (int i = 295; i < MaxCylinderItem; ++i) {
            if (i == C_OutArmSmallY) continue;                           //AI(W906-MACH0210) 20261005
            if (Cylinder[i].CylinderName != AnsiString("")) ++named9050;
            if (Cylinder[i].Enable) ++enabled9050;
        }
        CHECK(named9050 == 0); CHECK(enabled9050 == 0);
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
