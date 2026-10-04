// ===========================================================================
//  tests/test_mydb_csv_alarmcode.cpp -- cMyDB CSV plan P2 (MDB Updater -> V906 alarm-code catalog).
//  AI(W906-CSVONLY) 20260926.
//
//  1) Golden sequence shape (numbers from tools/gen_alarmcode_catalog.py on Rev902):
//     raw 2996 insertions, 2993 unique, duplicates exactly WAR0131 / MES1621 / JAM2339 (first kept).
//  2) Spot checks: first entry WAR000000, Unit 24 motor loop spelling (WAR24%03d%d, "name -- alarm").
//  3) Byte equality: AlarmCodeCatalog_UpdateList() output == ship/Error/AlarmCodeList.txt (the generator's
//     own output), so the C++ replay and the Python generator can never silently disagree.
//  4) Log file: version line first, golden "@@Error!!" / "Alarm Code: X is duplicate!!" wording.
//  Files are written under %TEMP%, never under D:\HT9045.
// ===========================================================================
#include "AlarmCodeCatalog.h"

#include <cstdio>
#include <cstdlib>
#include "w906_test_tmpname.h"   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
#include <string>

#ifndef W906_SHIP_ALARMCODELIST
#error "W906_SHIP_ALARMCODELIST must point at ship/Error/AlarmCodeList.txt"
#endif

static int g_fail = 0;
static int g_total = 0;
static void check(bool cond, const char* expr, int line)
{
    ++g_total;
    if (!cond)
    {
        ++g_fail;
        std::printf("FAIL line %d: %s\n", line, expr);
    }
}
#define CHECK(c) check((c), #c, __LINE__)

static std::string ReadAll(const std::string& path)
{
    std::string s;
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f)
        return s;
    char buf[65536];
    std::size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0)
        s.append(buf, n);
    std::fclose(f);
    return s;
}

static std::string TempDir()
{
    const char* t = std::getenv("TEMP");
    if (!t)
        t = std::getenv("TMP");
    std::string d = t ? t : ".";
    return d + "\\" + W906_TestTmpName("w906_alarmcode_test");   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h)
}

int main()
{
    // 1) shape
    AlarmCodeUpdateResult b = AlarmCodeCatalog_Build();
    CHECK(AlarmCodeCatalog_RawCount() == 2996);
    CHECK(b.rawCount == 2996);
    CHECK(b.list.size() == 2993);
    CHECK(b.duplicates.size() == 3);
    if (b.duplicates.size() == 3)
    {
        CHECK(b.duplicates[0] == "WAR0131");
        CHECK(b.duplicates[1] == "MES1621");
        CHECK(b.duplicates[2] == "JAM2339");
    }

    // 2) spot checks
    CHECK(!b.list.empty() && b.list[0].code == "WAR000000" && b.list[0].message == "Uknown alarm code!");
    bool sawFirstMotor = false, sawMotorAlarm8 = false;
    for (std::size_t i = 0; i < b.list.size(); ++i)
    {
        if (b.list[i].code == "WAR240000")
            sawFirstMotor = (b.list[i].message == "MInArmX -- Motor error.");
        if (b.list[i].code == "WAR240008")
            sawMotorAlarm8 = (b.list[i].message == "MInArmX -- Motor Alarm");
    }
    CHECK(sawFirstMotor);
    CHECK(sawMotorAlarm8);

    // 3) byte equality with the shipped file
    std::string dir = TempDir();
    std::string listPath = dir + "\\AlarmCodeList.txt";
    std::string logDir = dir + "\\MDB_UpdateLog";
    AlarmCodeUpdateResult u = AlarmCodeCatalog_UpdateList(listPath, logDir, "test-version");
    CHECK(u.listWritten);
    CHECK(u.logWritten);
    std::string ship = ReadAll(W906_SHIP_ALARMCODELIST);
    std::string mine = ReadAll(listPath);
    CHECK(!ship.empty());
    CHECK(mine == ship);
    if (mine != ship)
        std::printf("  size generated %u vs shipped %u\n", (unsigned)mine.size(), (unsigned)ship.size());

    // 4) log wording
    std::string log = ReadAll(u.logPath);
    CHECK(log.compare(0, 14, "test-version\r\n") == 0);
    CHECK(log.find("@@Error!!\r\nAlarm Code: WAR0131 is duplicate!!\r\n") != std::string::npos);
    CHECK(log.find("raw 2996, written 2993, duplicates 3") != std::string::npos);

    std::remove(listPath.c_str());
    std::remove(u.logPath.c_str());
    W906_TestTmpRemoveTree(dir);   // AI(W906-ST02-C17) 20261005 (St02-E): per-process scratch name (St01 E-039; tests/w906_test_tmpname.h): and the folder (MDB_UpdateLog included)

    std::printf("test_mydb_csv_alarmcode: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
