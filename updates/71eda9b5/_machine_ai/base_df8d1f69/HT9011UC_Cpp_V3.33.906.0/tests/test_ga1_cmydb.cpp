// ===========================================================================
//  tests/test_ga1_cmydb.cpp  -- GA-1-B4 verification (cMyDB.cpp sqlite wrapper)
//
//  Two independent proofs:
//
//  1) REAL-FILE ORACLE (no cMyDB.cpp dependency at all -- exercises the
//     vendored sqlite3 amalgamation directly, read-only, against an actual
//     production .DB under D:\HT9045\system\).  Confirms the engine can open
//     and read golden's real on-disk format.  RUNTIME-SKIP (prints SKIP,
//     exits 0) if D:\HT9045\system is absent, matching the established
//     tests/test_config_db.cpp convention (env override: HT9045_SYSTEM_DIR).
//
//  2) WRAPPER ROUND-TRIP (through cMyDB.cpp's REAL, compiled functions):
//     MyDBOpenDB/MyDBCloseDB, MyDBExecSQL (file-local helper, forward-
//     declared here -- same shape as golden's own internal use), and the
//     read-side wrappers MyDBQClearDT / MyDBQTotalLoader / MyDBQLotData /
//     RecordChangeLogProcess (which drives the ACTIVE MyDBIProcess seam).
//
//     NOTE ON ":memory:": MyDBOpenDB opens TWO independent sqlite3
//     connections (dbReadWrite, dbReadOnly) against the SAME `asDBPath`
//     string.  A literal ":memory:" filename allocates a SEPARATE anonymous
//     in-memory database PER CONNECTION (sqlite3 gives each `:memory:` open()
//     its own private DB unless SQLITE_OPEN_URI + "file::memory:?cache=
//     shared" is used, which MyDBOpenDB does not pass) -- so writes through
//     dbReadWrite would never be visible through dbReadOnly, and the round-
//     trip would silently read back nothing.  A temp FILE path is used
//     instead: both connections open the SAME on-disk file, which is both a
//     correct round-trip AND a more faithful stand-in for golden's real usage
//     (a single on-disk Handler.db3 shared by both connections).
//
//  GOD-STACK AVOIDANCE: this test does NOT link cprod.cpp/cmydef.cpp/
//  common.cpp/LastSet.cpp/mycylin.cpp/forms/fMain.cpp/forms/fLotInfo.cpp (the
//  real translated bodies behind cMyDB.cpp's `extern` surface) -- doing so
//  would pull in most of the ported tree.  Instead, following the established
//  precedent (test_config_loaders.cpp / test_MyCCLink.cpp: "TU-local stand-in
//  for CUSTOMER_CODE/IniConfig/ShowMyMessage/MyDBIProcess etc, avoiding
//  multiple-definition clashes with the real bodies"), every extern global
//  and free function cMyDB.cpp references is given a minimal, real-typed,
//  zero-behavior definition HERE.  None of these duplicate anything: the
//  real ht9045_globals/ht9045_sm/ht9045_forms/ht9045_io libraries are not
//  linked into this executable, so there is no ODR clash -- only a
//  standalone test binary that link-satisfies cMyDB.cpp's dependency
//  surface with inert stand-ins, then exercises the REAL cMyDB.cpp logic on
//  top of them.
// ===========================================================================
#include "cMyDB.h"
#include "third_party/sqlite3/sqlite3.h"
#include "cmydef.h"
#include "common.h"
#include "LastSet.h"
#include "mycylin.h"
#include "forms/fMain.h"
#include "forms/fLotInfo.h"

#include <cstdio>
#include <cstdlib>
#include <string>

// dbReadOnly/dbReadWrite are plain (non-static) globals defined in cMyDB.cpp,
// deliberately NOT declared in cMyDB.h (matches golden -- nothing outside
// cMyDB.cpp touches them there either). Redeclared here for the round-trip
// test's own health checks.
extern sqlite3 *dbReadOnly;
extern sqlite3 *dbReadWrite;

static int g_fail = 0;
static int g_total = 0;

static void check(bool cond, const char* expr, const char* file, int line) {
    ++g_total;
    if (!cond) {
        ++g_fail;
        std::printf("FAIL [%s:%d]  %s\n", file, line, expr);
    }
}
#define CHECK(cond) check((cond), #cond, __FILE__, __LINE__)

// ===========================================================================
//  PART 1 -- real-file oracle (sqlite3 engine only, no cMyDB.cpp needed)
//
//  AI(GA1-B4) 20260804: the task brief pointed at D:\HT9045\system\*.DB
//  (cylinder/Sensor/Motor-named files) as "real SQLite .DB" oracles.
//  VERIFIED, NOT ASSUMED: every one of those files (checked cylinder.DB,
//  motor.DB, Sensor.DB, alarm.DB, message.db, user.db, note.db, Yield.DB,
//  switch.DB, sucker.DB, Tray.DB, delay.db, db.DB, function.DB,
//  JamErrorInfo.DB) is REJECTED by the vendored sqlite3 engine with
//  "file is encrypted or is not a database" (SQLITE_NOTADB), and none of
//  their first 16 bytes match the "SQLite format 3\0" magic header -- they
//  are NOT sqlite3 files at all (almost certainly legacy BDE/Paradox .DB
//  tables, consistent with database.h's own gated `TDataModule1` / BDE
//  `TTable` note elsewhere in this tree). cMyDB.cpp's *actual* sqlite3
//  database is a DIFFERENT file in a DIFFERENT directory: `asDBPath`
//  (common.cpp) defaults to "D:\HT9045\MDB\Handler.db3", and THAT file DOES
//  carry the real "SQLite format 3\0" header and opens/queries cleanly --
//  used below instead.
// ===========================================================================
static AnsiString mdbDir() {
    const char* env = std::getenv("HT9045_MDB_DIR");
    if (env && *env) return AnsiString(env);
    return AnsiString("D:\\HT9045\\MDB");
}

static void test_real_db_oracle() {
    AnsiString dbPath = mdbDir() + AnsiString("\\Handler.DB3");

    if (!FileExists(dbPath)) {
        std::printf("SKIP test_real_db_oracle: \"%s\" not found "
                    "(set HT9045_MDB_DIR to override). Exiting this part.\n",
                    dbPath.c_str());
        return;
    }

    sqlite3 *db = NULL;
    int rc = sqlite3_open_v2(dbPath.c_str(), &db, SQLITE_OPEN_READONLY, NULL);
    CHECK(rc == SQLITE_OK);
    CHECK(db != NULL);
    if (rc != SQLITE_OK || db == NULL) { if (db) sqlite3_close(db); return; }

    // List tables -- expect the exact table set cMyDB.cpp's own SQL strings
    // reference (AlarmList/EventLog/UnitName/AxleName/MotorAlarmList/Process/
    // Message/Motion/Production/TimeData/TotalLoader/ClearDateTime/LotInfo).
    char **result = NULL; int rows = 0, cols = 0; char *errMsg = NULL;
    rc = sqlite3_get_table(db, "SELECT name FROM sqlite_master WHERE type='table'",
                           &result, &rows, &cols, &errMsg);
    CHECK(rc == SQLITE_OK);
    CHECK(rows >= 1);
    std::printf("Handler.DB3: %d table(s):", rows);
    bool sawAlarmList = false;
    for (int i = 1; i <= rows; ++i) {
        std::printf(" %s", result[i*cols]);
        if (AnsiString(result[i*cols]) == "AlarmList") sawAlarmList = true;
    }
    std::printf("\n");
    if (errMsg) sqlite3_free(errMsg);
    sqlite3_free_table(result);
    CHECK(sawAlarmList);

    // Read >=3 rows from AlarmList -- the exact table + column shape
    // MyDBIEvent/GetMyDBIMessage's SQL strings target
    // ("ID_AlarmList, AlarmCode, UnitNo, ID_Axle, Type, Message").
    errMsg = NULL;
    rc = sqlite3_get_table(db, "SELECT * FROM AlarmList LIMIT 5", &result, &rows, &cols, &errMsg);
    CHECK(rc == SQLITE_OK);
    CHECK(cols == 6);
    CHECK(rows >= 3);
    std::printf("  AlarmList: %d cols, %d row(s) read; row1 AlarmCode=\"%s\" Message=\"%s\"\n",
                cols, rows, rows >= 1 ? result[1*cols+1] : "", rows >= 1 ? result[1*cols+5] : "");
    if (errMsg) sqlite3_free(errMsg);
    sqlite3_free_table(result);

    sqlite3_close(db);
}

// ===========================================================================
//  PART 2 -- wrapper round-trip (real cMyDB.cpp bodies, god-stack-free stand-
//  ins for everything it derefs).  Definitions in dependency order.
// ===========================================================================

// ---- CosFunction / IniConfig / RunInfo / SystemYear.../LastSet -----------
// (real types via the headers cMyDB.h transitively pulls in; instances given
//  HERE instead of linking the real cprod.cpp/cmydef.cpp/Config.h-def TUs)
HT9045_COUSTOMER_FUNCTION CosFunction;
HT9045_CONFIG IniConfig;
int CUSTOMER_CODE = 0;   // cmydef.h:3181 extern int -- 0 matches no CC_* branch in cMyDB.cpp, an honest "no customer profile" offline default
RUN_INFO::RUN_INFO() {}
RUN_INFO::~RUN_INFO() {}
RUN_INFO RunInfo;
LAST_GENERAL_SET LastSet;
Word SystemYear=2026, SystemMonth=8, SystemDate=4, SystemHour=0, SystemMin=0, SystemSec=0, SystemMSec=0;
Word SystemYearYesterday=2026, SystemMonthYesterday=8, SystemDateYesterday=3;
AnsiString asDBPath = "";
AnsiString asSummaryPath = "";
AnsiString AuthPath = "";
TMyStringList *slEventLog = NULL;   // never dereferenced on the ACTIVE paths this test exercises (opaque type; NULL is a valid, honest offline state)
class TMyStringList; // (already fwd-declared by cmydef.h; kept here only as a documentation anchor -- harmless redeclaration)

// ---- mycylin.h : TMyCylinder / Cylinder[] / asTempCtrl[] -------------------
// MyDBUpdateDB (compiled, not called by this test) references these at file
// scope; the linker still needs them resolved even though this test never
// invokes MyDBUpdateDB.  TMyCylinder's real ctor lives in mycylin.cpp (not
// linked here) -- give the array trivial storage via a minimal same-named
// class is not an option (ODR), so instead we call MyDBUpdateDB from NO test
// path and provide the exact real extern storage using the real class list-
// initialized default-constructed array. TMyCylinder has a declared (non-
// inline) ctor, so we must define it once, here, to stand in for mycylin.cpp.
TMyCylinder::TMyCylinder() : PTempWinCtrl(NULL), NeedFinishOnFunction(false), NeedFinishOffFunction(false),
    Enable(false), Status(false), Change(false), AlarmEnable(false), OnAlarmCode(0), OffAlarmCode(0),
    OnAlarmTime(0), OffAlarmTime(0), OnDelayTime(0), OffDelayTime(0), OutRing(0), OutIP(0), OutPort(0), OutBit(0),
    OutType(0), OutISABase(0), OnSenRing(0), OnSenIP(0), OnSenPort(0), OnSenBit(0), OnSenType(0), OnSenISABase(0),
    OnSenEnable(false), OffSenRing(0), OffSenIP(0), OffSenPort(0), OffSenBit(0), OffSenType(0), OffSenISABase(0),
    OffSenEnable(false), bCylinderOn(false), ISABase(0), bCheckSafeDoor(false), bfirst(false), bOnOffState(false),
    iOnOffCount(0), dOnTime(0), dOffTime(0), iTimeOutCount(0), bCylPreAlarmByPassT(false), sListOnTime(NULL),
    sListOffTime(NULL), iOnOffCountAlarm(0), iOnCountAlarm(0), iOffCountAlarm(0), dOnTimeAlarm(0), dOffTimeAlarm(0),
    iTimeOutCountAlarm(0), iResetCount(0)
{}
TMyCylinder Cylinder[MaxCylinderItem];
AnsiString asTempCtrl[tcTotalCount];

// ---- forms/fMain.h , forms/fLotInfo.h : NULL is a valid offline state ------
TfMain *fMain = NULL;
TfLotInfo *fLotInfo = NULL;

// ---- common.h free functions (god-stack avoidance stand-ins) --------------
char *ConvertSecondToTime(long s) { static char buf[32]; std::snprintf(buf, sizeof(buf), "%ld", s); return buf; }
void GetTimeInfo() {}
bool RespondASECom(AnsiString) { return true; }
void ProductionLog(AnsiString, bool, AnsiString) {}
AnsiString GetOnlyTimeInfoByString(AnsiString) { return AnsiString("000000"); }
int MyForceDirectories(AnsiString, AnsiString) { return 1; }
bool ReadIniData(AnsiString, AnsiString, AnsiString, bool bValue) { return bValue; }
int ReadIniData(AnsiString, AnsiString, AnsiString, int Value) { return Value; }
double ReadIniData(AnsiString, AnsiString, AnsiString, double Value) { return Value; }
AnsiString ReadIniData(AnsiString, AnsiString, AnsiString, AnsiString Value) { return Value; }
void WriteIniData(AnsiString, AnsiString, AnsiString, bool) {}
void WriteIniData(AnsiString, AnsiString, AnsiString, int) {}
void WriteIniData(AnsiString, AnsiString, AnsiString, double) {}
void WriteIniData(AnsiString, AnsiString, AnsiString, unsigned long) {}
void WriteIniData(AnsiString, AnsiString, AnsiString, AnsiString) {}
void WriteIniData(AnsiString, AnsiString, AnsiString, TDateTime) {}
void SGDToCSV(vclcompat::TStringGrid*, AnsiString, AnsiString, AnsiString) {}

// ---- cmydef.h : SaveEventLog ----------------------------------------------
void SaveEventLog() {}

// ---- MyDBIProcess (3-arg __fastcall) : cMyDB.cpp's own real definition is
// gated (homecoming); RecordChangeLogProcess below still calls it.  Same
// role SECSGEM/uHGemEquipment.cpp's stand-in plays in the real tree -- a
// minimal, honest, non-golden-faithful recorder so the ACTIVE call site has
// something to link against.
static int g_MyDBIProcess_Count = 0;
void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2)
{
    ++g_MyDBIProcess_Count;
    std::printf("  [test-stand-in MyDBIProcess] %s | %s | %s\n", asTable.c_str(), S1.c_str(), S2.c_str());
}

// MyDBExecSQL is a file-local (non-static, external-linkage) helper in
// cMyDB.cpp, deliberately NOT re-declared in cMyDB.h (matches golden, which
// never exposes it either).  Forward-declared here to drive table setup
// through the SAME sqlite3_exec path the rest of the wrapper uses.
extern int MyDBExecSQL(AnsiString str);

static AnsiString tempDbPath() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp || !*tmp) tmp = ".";
    AnsiString p; p.sprintf("%s\\ga1_b4_roundtrip.db3", tmp);
    return p;
}

static void test_wrapper_roundtrip() {
    AnsiString dbPath = tempDbPath();
    // Start from a clean file each run.
    std::remove(dbPath.c_str());

    CosFunction.bUseMDB = true;
    asDBPath = dbPath;

    MyDBOpenDB();
    CHECK(dbReadWrite != NULL);
    CHECK(dbReadOnly != NULL);

    // ---- schema the wrapper's own query functions expect -------------
    // NOTE: MyDBExecSQL returns sqlite3_last_insert_rowid(), which is NOT
    // reset by a CREATE TABLE (DDL) statement -- it keeps reporting whatever
    // the most recent INSERT produced. So CREATE TABLE return values are
    // deliberately not asserted against 0 here.
    MyDBExecSQL("CREATE TABLE ClearDateTime (ClearDateTime TEXT)");
    CHECK(MyDBExecSQL("INSERT INTO ClearDateTime VALUES ('2026-08-01 00:00:00')") >= 1);

    MyDBExecSQL("CREATE TABLE TotalLoader (ID_TotalLoader INTEGER PRIMARY KEY AUTOINCREMENT, "
                "LoadCount INTEGER, OccurDateTime TEXT)");
    CHECK(MyDBExecSQL("INSERT INTO TotalLoader (LoadCount, OccurDateTime) VALUES (100, '2026-08-01 08:00:00')") >= 1);
    CHECK(MyDBExecSQL("INSERT INTO TotalLoader (LoadCount, OccurDateTime) VALUES (150, '2026-08-02 08:00:00')") >= 1);

    MyDBExecSQL("CREATE TABLE LotData (ID_LotData INTEGER PRIMARY KEY AUTOINCREMENT, LotID TEXT)");
    CHECK(MyDBExecSQL("INSERT INTO LotData (LotID) VALUES ('LOTX01')") >= 1);
    CHECK(MyDBExecSQL("INSERT INTO LotData (LotID) VALUES ('LOTX02')") >= 1);
    CHECK(MyDBExecSQL("INSERT INTO LotData (LotID) VALUES ('LOTX03')") >= 1);

    // ---- read path : real header-declared query wrappers -------------
    AnsiString clearDt = MyDBQClearDT();
    std::printf("MyDBQClearDT() -> \"%s\"\n", clearDt.c_str());
    CHECK(clearDt == "2026-08-01 00:00:00");

    // MyDBQTotalLoader takes the most-recent-row-at-or-before each boundary
    // and subtracts: StartDateTime landing BEFORE the 08:00:00 row means
    // iStartCount=100 (the row at-or-before 09:00:00 on day 1); EndDateTime
    // landing after day 2's row means iEndCount=150. delta = 150-100 = 50.
    AnsiString delta = MyDBQTotalLoader("2026-08-01 09:00:00", "2026-08-02 09:00:00");
    std::printf("MyDBQTotalLoader(...) -> \"%s\" (expect 50 = 150-100)\n", delta.c_str());
    CHECK(delta == "50");

    TStringList *lotList = new TStringList();
    MyDBQLotData("SELECT LotID FROM LotData", lotList);
    std::printf("MyDBQLotData rows: %d\n", lotList->Count);
    // AI(GA1-B4) 20260804: golden quirk (see cMyDB.cpp's MyDBQLotData note) --
    // the loop starts at sqlite3_get_table row index 2, silently skipping
    // row index 1 (the FIRST data row, LOTX01). Preserved verbatim: expect
    // only 2 entries (LOTX02, LOTX03) back from 3 inserted rows.
    CHECK(lotList->Count == 2);
    if (lotList->Count == 2) {
        CHECK(lotList->Strings[0] == "LOTX02");
        CHECK(lotList->Strings[1] == "LOTX03");
    }
    lotList->Clear();
    delete lotList;

    // ---- write path through a real, ACTIVE, non-gated wrapper function:
    // RecordChangeLogProcess -> MyDBIProcess("ChangeLog", S, S2) -- proves
    // the cMyDB.h-declared MyDBIProcess seam correctly links to whichever
    // definition is available (here, this test's own stand-in above).
    int before = g_MyDBIProcess_Count;
    RecordChangeLogProcess("unit-test change", "detail");
    CHECK(g_MyDBIProcess_Count == before + 1);

    // MyDBUpdateDB's ALTER/DELETE/VACUUM tail (ACTIVE code) against tables
    // that DO exist in this schema-light test DB will simply fail silently
    // per MyDBExecSQL's own error-swallowing contract (matches golden) --
    // not exercised here to keep this test's schema minimal and legible.

    MyDBCloseDB();
    std::remove(dbPath.c_str());
}

int main() {
    test_real_db_oracle();
    test_wrapper_roundtrip();

    std::printf("\n%d/%d checks passed.\n", g_total - g_fail, g_total);
    return g_fail != 0 ? 1 : 0;
}
