// ===========================================================================
//  tests/test_note_motorerror.cpp
//
//  AI(W906-JAM-STOP) 20260930: INBOX 118 -- golden ShowMotorErrorMessage (note.cpp:1052-1169), now the golden body at
//  forms/fNote_ShowError.cpp EOF (ht9045_sm); the record-only stand-in (canary_support.cpp:486-525) is retired.
//
//    [1] a motor JAM, InitialOK true, a note host installed (the stand-in for tools/wb_serve.cpp
//        ForwardShowMotorErrorMessage):
//        - the stop half (golden :1054-1059): StopAllMotor(true)'s Galil stop "VS0;SP0,0,0,0;" and then golden's "ST" reach
//          the Galil command channel (a fake Index route, Motor/GaliRoute.h, records the strings sent through
//          MOT[MTestY1].Gali_Command); golden ST's own clearing (MOT[MTestZ1].MovFlag); every other enabled
//          PServoAlarmOn motor DecStop()'d (StopAllMotor -> PCIL132_StopMotor); the front index brake output follows
//          IndexMotorBreakerOFF (csystem.cpp:19144); fAllMotorHome false
//        - the note reaches the display path: the host is called ONCE with Code+MotorAlarmNo upper-cased, KeyCode 0
//          (golden :1110), Pos = the motor's unit (golden :1096-1099), unit = MOT[UnitNo].Alias, message = MyDBIEvent's
//          text; fNote's fields as ErrShowToForm sets them; sJamArea / sJamCode (golden :1086-1091)
//        - the call RETURNS (the host does not wait) and the note's only answer is applied: SoftStop true, SoftStart false
//          (golden BtnPauseClick KeyCode==0 :4146 / :4148)
//        - the record: the EventTracker csv of the day and the EventLogTxt file carry the code; iHome 1 and
//          bIndexDropVacuumError false (golden :1167-1168)
//        - the recorder (canary_support.cpp:427-437) still counts
//    [2] InitialOK false: the same stop half, then golden :1061-1065 -- MyDBIProcess("Exception", ...) and return: no note,
//        SoftStop stays false, iHome untouched
//    [3] Code "WAR": ShowErrorMessage("WAR16101", 0, MMSystem, ...) (golden :1067-1071) and return: no note
//    [4] no host installed (every other ctest): the body runs and returns; no note, so no answer (SoftStop stays false)
//    [5] source pins on tools/wb_serve.cpp: the host is installed, it posts through ForwardShowErrorMessage with kcode 0
//        (the AI(W906-Q30-KZERO) branch returns before the wait loop), and ShowErrorMessage's own stop / record are
//        skipped for the motor note
//  Writes only through the common.cpp log seams (EventTracker / HANDLER LOG / EventLogTxt under as9045LogPath), which
//  ctest points into machine_log_scratch -- the guard below refuses to run anywhere else.
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "forms/fNote.h"
#include "forms/fMain.h"
#include "cMyDB.h"                  // AlarmUnit[] / iAlarmUnitTotal
#include "common.h"                 // as9045LogPath / asSaveEventLogPath / asProductionLogPath
#include "cpublic.h"                // GetTimeInfo
#include "cmydef.h"                 // InitialOK / SoftStop / SoftStart / fAllMotorHome / iHome / bIndexDropVacuumError / SW / Sen ids / MTest*
#include "Config.h"                 // IniConfig.bSPILFunction / bO06SaveLogTimePeriod / bD48PowerOffEmgCanNotUseZ1Z2
#include "CosFunction.h"            // CosFunction.bOLPFunction
#include "Motor/mymotor.h"          // MOT[]
#include "Motor/myGALILmotor.h"     // TMyGALILMotor (golden's index-axis class; Gali_Command needs MOT[MTestY1].Motor)
#include "Motor/mySimMotor.h"       // TMySimMotor
#include "Motor/GaliRoute.h"        // TGaliRoute / W906_SetGaliRoute -- the fake Galil channel
#include "myswitch.h"               // SW[]
#include "mysensor.h"               // Sen[]
#include "Public/MyStringList.h"    // TMyStringList (slEventLog)
#include "w906_ctest_guard.h"

void ShowMotorErrorMessage(AnsiString Code, int MotorAlarmNo, AnsiString errPart);   // canary_support.h:268 (that header cannot share a TU with cMyDB.h)
extern void (*W906_ShowMotorErrorMessage_Hook)(const char*, int, int, const char*, const char*);   // forms/fNote_ShowError.cpp
extern int        W906_ShowMotorErrorMessage_Count;                         // canary_support.cpp:430
extern AnsiString W906_ShowMotorErrorMessage_LastCode;
extern int        W906_ShowMotorErrorMessage_LastMotorAlarmNo;
extern AnsiString W906_ShowMotorErrorMessage_LastErrPart;
extern AnsiString W906_ShowErrorMessage_LastCode;                           // canary_support.cpp (the ShowErrorMessage recorder)
extern int        W906_ShowErrorMessage_LastKCode;
void W906_ShowErrorMessage_Reset();
extern bool bGalilTwoYMoveFlag;                                             // Motor/myGALILmotor.cpp (StopAllMotor(true) clears it, golden :4717)

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_note_motorerror.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)
static bool Eq(const AnsiString& a, const char* b) { return std::strcmp(a.c_str(), b) == 0; }

// --- the fake Galil command channel (every string MOT[MTestY1].Gali_Command sends with no card) ---
static std::vector<std::string> g_galil;
static bool FakeCommand(int, const char* data, long* reply) { g_galil.push_back(data ? data : ""); if (reply) *reply = 0; return true; }
static TGaliRoute g_route = { MTestZ1, FakeCommand, 0, 0 };

// --- the note host stand-in (what tools/wb_serve.cpp ForwardShowMotorErrorMessage receives) ---
struct NoteCall { std::string code, unit, message; int keyCode, pos; };
static std::vector<NoteCall> g_notes;
static bool g_softStopDuringHost = true;
static void FakeHost(const char* code, int keyCode, int pos, const char* unit, const char* message)
{
    NoteCall n;
    n.code = code ? code : ""; n.unit = unit ? unit : ""; n.message = message ? message : ""; n.keyCode = keyCode; n.pos = pos;
    g_notes.push_back(n);
    g_softStopDuringHost = SoftStop;                                        // the answer is applied AFTER the host returns
}

// --- a motor whose DecStop is observable (StopAllMotor -> PCIL132_StopMotor -> Motor->DecStop) ---
struct CountingSimMotor : public TMySimMotor {
    int decStops;
    CountingSimMotor() : decStops(0) {}
    void DecStop() override { ++decStops; TMySimMotor::DecStop(); }
};

static bool FileHas(const std::string& path, const char* needle)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::stringstream ss; ss << f.rdbuf();
    return ss.str().find(needle) != std::string::npos;
}
static int CountIn(const std::string& path, const char* needle)   // the EventTracker csv is shared by every run: count, do not just find
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return 0;
    std::stringstream ss; ss << f.rdbuf();
    const std::string s = ss.str();
    int n = 0;
    for (size_t at = s.find(needle); at != std::string::npos; at = s.find(needle, at + 1)) ++n;
    return n;
}
static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p); else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}
static bool TreeHas(const std::string& dir, const char* needle, std::string* where)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return false;
    bool found = false;
    do {
        const std::string n = fd.cFileName;
        if (n == "." || n == "..") continue;
        const std::string p = dir + "\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { if (TreeHas(p, needle, where)) { found = true; break; } }
        else if (FileHas(p, needle)) { if (where) *where = p; found = true; break; }
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return found;
}
static std::string ReadSource(const char* rel)
{
    std::ifstream f((std::string(W906_SRC_ROOT) + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static size_t LastIndexOf(const std::vector<std::string>& v, const char* s)
{
    for (size_t i = v.size(); i > 0; --i) if (v[i - 1] == s) return i - 1;
    return (size_t)-1;
}

static void ArmState()
{
    SoftStop = true;  SoftStart = true;  fAllMotorHome = true;  iHome = 0;  bIndexDropVacuumError = true;
    bGalilTwoYMoveFlag = true;
    MOT[MTestZ1].MovFlag = true;
    SW[SwFMotorBreaker].OutValue = true;
    g_galil.clear();
    g_notes.clear();
    g_softStopDuringHost = true;
}

int main()
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asProductionLogPath", asProductionLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NoteMotorError", rt))
        return 2;
    std::printf("=== test_note_motorerror (AI(W906-JAM-STOP), INBOX 118) ===\n");
    CHECK(fNote != 0);
    CHECK(fMain != 0);
    if (fNote == 0 || fMain == 0) { std::printf("FAIL: no fNote / fMain\n"); return 1; }

    // --- fixture: the boot state the body relies on (golden: TfMain ctor / FormShow / InitialMotorParameter) ---
    fMain->AlarmCodeMap["WAR240147"] = "Test Z1 servo alarm";               // MyDBUpdateDB's AlarmCodeList row (cMyDB.cpp:2190-2225)
    if (fMain->UnitNameMap == 0) fMain->UnitNameMap = new TStringList();
    fMain->UnitNameMap->Clear();
    for (int i = 0; i < iAlarmUnitTotal; i++) fMain->UnitNameMap->Add(AlarmUnit[i]);
    TMyStringList* const savedEv = slEventLog;                              // golden TfMain ctor (LogObjects.cpp W906_CreateLogObjects)
    char evStamp[64];  std::snprintf(evStamp, sizeof(evStamp), "\\NoteMotorError_%lu", (unsigned long)::GetTickCount());
    const std::string evDir = std::string(as9045LogPath.c_str()) + evStamp;   // this run's own folder under the scratch root (ctest -j: others share the scratch)
    slEventLog = new TMyStringList(AnsiString(evDir.c_str()), "EventLogTxt",
                                   "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe");
    // The day file exists before the first alarm on a running machine (boot rows).  It matters: golden
    // TMyStringList::MyInsertToFile (Public/MyStringList.cpp:985-988) only NAMES the file when it does not exist yet and
    // writes nothing -- measured here 20260930: without this row, [1]'s alarm line was not written.
    slEventLog->AddTextWithDateTime("fixture, day file exists");
    slEventLog->MySaveToFile();
    const char* names[4] = { "MTestY1", "MTestZ1", "MTestZ2", "MTestY2" };
    for (int k = 0; k < 4; ++k) {                                           // cinitial.cpp's Galil arm (ports 0..3), as test_gali_route_engine
        const int i = MTestY1 + k;
        if (MOT[i].Motor == 0) MOT[i].Motor = new TMyGALILMotor(k);
        MOT[i].SetAlias(i, names[k]);
        MOT[i].Motor->Enable = true;
    }
    CountingSimMotor* const inArmX = new CountingSimMotor();                // one ordinary axis: StopAllMotor stops it
    inArmX->Enable = true;  inArmX->PServoAlarmOn = true;
    HTMotor* const savedInArmX = MOT[MInArmX].Motor;
    MOT[MInArmX].Motor = inArmX;
    W906_SetGaliRoute(&g_route);

    const bool saveInit = InitialOK, saveSpil = IniConfig.bSPILFunction, saveO06 = IniConfig.bO06SaveLogTimePeriod, saveOlp = CosFunction.bOLPFunction;
    IniConfig.bSPILFunction = false;  IniConfig.bO06SaveLogTimePeriod = false;  CosFunction.bOLPFunction = false;
    fNote->fShow = false;
    const bool zDown = Sen[SnFMotorDown].IsOn() && IniConfig.bD48PowerOffEmgCanNotUseZ1Z2 == false;   // golden IndexMotorBreakerOFF's own test (csystem.cpp:19146)
    CHECK(zDown == false);                                                  // the fixture is the "brake off" arm

    // ---- [1] ----------------------------------------------------------------------------------------------------
    std::printf("[1] motor JAM, InitialOK true, note host installed\n");
    InitialOK = true;
    W906_ShowMotorErrorMessage_Hook = &FakeHost;
    ArmState();
    const int n0 = W906_ShowMotorErrorMessage_Count;
    std::string tracker;
    {
        GetTimeInfo();                                                      // SystemYear / Month / Date (cpublic.h:23) -- MyDBIEvent's path uses the same clock
        char day[64], name[64];
        std::snprintf(day, sizeof(day), "\\ASE log\\%04d\\%02d\\%02d\\", (int)SystemYear, (int)SystemMonth, (int)SystemDate);
        std::snprintf(name, sizeof(name), "@%04d_%02d_%02d_EventTracker.csv", (int)SystemYear, (int)SystemMonth, (int)SystemDate);
        tracker = std::string(as9045LogPath.c_str()) + day + IniConfig.SocketHandlerID.c_str() + name;
    }
    const int tracked0 = CountIn(tracker, "Test Z1 servo alarm");
    ShowMotorErrorMessage("war24014", 7, "Gali_MotMove_");
    std::printf("    galil strings:");
    for (size_t i = 0; i < g_galil.size(); ++i) std::printf(" [%s]", g_galil[i].c_str());
    std::printf("\n");
    // stop half
    const size_t iVs = LastIndexOf(g_galil, "VS0;SP0,0,0,0;"), iSt = LastIndexOf(g_galil, "ST");
    CHECK(iVs != (size_t)-1);                                               // StopAllMotor(true) (golden :1056 -> Motor/myGALILmotor.cpp StopAllMotor)
    CHECK(iSt != (size_t)-1 && iSt == g_galil.size() - 1);                  // golden :1057 MOT[MTestY1].Gali_Command("ST", errPart) -- the last Galil string
    CHECK(iVs != (size_t)-1 && iSt != (size_t)-1 && iVs < iSt);
    CHECK(MOT[MTestZ1].MovFlag == false);                                   // golden ST's own clearing (no-card branch)
    CHECK(bGalilTwoYMoveFlag == false);                                     // StopAllMotor(true)
    CHECK(inArmX->decStops >= 1);                                           // StopAllMotor: PCIL132_StopMotor on an enabled PServoAlarmOn axis
    CHECK(SW[SwFMotorBreaker].OutValue == zDown);                           // golden :1058 IndexMotorBreakerOFF
    CHECK(fAllMotorHome == false);                                          // golden :1059
    // display path
    CHECK(g_notes.size() == 1);
    if (g_notes.size() == 1) {
        std::printf("    note: code=%s keyCode=%d pos=%d unit=%s message=%s\n", g_notes[0].code.c_str(), g_notes[0].keyCode,
                    g_notes[0].pos, g_notes[0].unit.c_str(), g_notes[0].message.c_str());
        CHECK(g_notes[0].code == "WAR240147");                              // golden :1077-1078
        CHECK(g_notes[0].keyCode == 0);                                     // golden :1110 "PAUSE only"
        CHECK(g_notes[0].pos == MTestZ1);                                   // golden :1094-1099: "014" -> 14 <= MTrayX -> Pos = 14
        CHECK(g_notes[0].unit == "MTestZ1");                                // golden :1102 MOT[UnitNo].Alias
        CHECK(g_notes[0].message == "Test Z1 servo alarm");                 // MyDBIEvent's Message (golden :1080)
    }
    CHECK(g_softStopDuringHost == false);                                   // golden :1054 SoftStop=false still in force while the note is posted
    CHECK(SoftStop == true && SoftStart == false);                          // the note's only answer (golden BtnPauseClick KeyCode==0 :4146 / :4148)
    CHECK(Eq(fNote->edErrorCode->Text, "WAR240147"));                       // ErrShowToForm
    CHECK(Eq(fNote->Edit3->Text, "24"));
    CHECK(Eq(fNote->edUnitName->Text, "MTestZ1"));
    CHECK(Eq(fNote->sJamCode, "WAR240147"));                                // golden :1091
    CHECK(Eq(fNote->sJamArea, "Motor"));                                    // golden :1087 JamArea[24-1]
    CHECK(fNote->KeyCode == 0);
    CHECK(iHome == 1);                                                      // golden :1168
    CHECK(bIndexDropVacuumError == false);                                  // golden :1167
    CHECK(W906_ShowMotorErrorMessage_Count == n0 + 1);
    CHECK(Eq(W906_ShowMotorErrorMessage_LastCode, "war24014"));             // the recorder takes the argument as passed
    CHECK(W906_ShowMotorErrorMessage_LastMotorAlarmNo == 7);
    CHECK(Eq(W906_ShowMotorErrorMessage_LastErrPart, "Gali_MotMove_"));
    {
        CHECK(CountIn(tracker, "Test Z1 servo alarm") == tracked0 + 1);    // MyDBIEvent's row -- one new, from this call
        std::string evFile;
        CHECK(TreeHas(evDir, "WAR240147", &evFile));   // golden :1126-1163 the event-log line
        std::printf("    EventTracker: %s\n    EventLogTxt: %s\n", tracker.c_str(), evFile.c_str());
        std::string row;
        {
            std::ifstream f(evFile.c_str(), std::ios::binary);
            std::string l;
            while (std::getline(f, l)) if (l.find("WAR240147") != std::string::npos) { row = l; break; }
        }
        std::printf("    row: %s\n", row.c_str());
        CHECK(row.find(",Motor,WAR240147,") != std::string::npos);          // golden :1148-1149 sJamArea, sJamCode
        CHECK(row.find("Test Z1 servo alarm") != std::string::npos);        // :1153 Message
        CHECK(row.find(",7,") != std::string::npos);                        // :1154 MotorAlarmNo
    }

    // ---- [2] ----------------------------------------------------------------------------------------------------
    std::printf("[2] InitialOK false: stop half, then the Exception record and return\n");
    InitialOK = false;
    ArmState();
    SoftStop = false;
    ShowMotorErrorMessage("WAR24014", 1, "");
    CHECK(LastIndexOf(g_galil, "VS0;SP0,0,0,0;") != (size_t)-1 && LastIndexOf(g_galil, "ST") == g_galil.size() - 1);
    CHECK(SW[SwFMotorBreaker].OutValue == zDown);
    CHECK(fAllMotorHome == false && SoftStart == false);
    CHECK(g_notes.empty());                                                 // golden :1064 returns before the note
    CHECK(SoftStop == false);                                               // no note, no answer
    CHECK(iHome == 0 && bIndexDropVacuumError == true);                     // :1167-1168 not reached

    // ---- [3] ----------------------------------------------------------------------------------------------------
    std::printf("[3] Code \"WAR\": WAR16101 through ShowErrorMessage, no note\n");
    InitialOK = true;
    ArmState();
    SoftStop = false;
    W906_ShowErrorMessage_Reset();
    ShowMotorErrorMessage("WAR", 3, "x");
    CHECK(Eq(W906_ShowErrorMessage_LastCode, "WAR16101") && W906_ShowErrorMessage_LastKCode == 0);   // golden :1069
    CHECK(g_notes.empty());
    CHECK(LastIndexOf(g_galil, "ST") != (size_t)-1);                        // the stop half ran first
    CHECK(iHome == 0);

    // ---- [4] ----------------------------------------------------------------------------------------------------
    std::printf("[4] no note host installed: the body returns, no answer applied\n");
    W906_ShowMotorErrorMessage_Hook = 0;
    ArmState();
    SoftStop = false;
    ShowMotorErrorMessage("WAR24014", 7, "");
    CHECK(g_notes.empty());
    CHECK(SoftStop == false && SoftStart == false);
    CHECK(fAllMotorHome == false && iHome == 1);

    // ---- [5] ----------------------------------------------------------------------------------------------------
    std::printf("[5] source pins: tools/wb_serve.cpp\n");
    {
        const std::string s = ReadSource("tools/wb_serve.cpp");
        CHECK(!s.empty());
        CHECK(s.find("W906_ShowMotorErrorMessage_Hook = &ForwardShowMotorErrorMessage;") != std::string::npos);
        const size_t host = s.find("void ForwardShowMotorErrorMessage(const char* code, int keyCode, int pos, const char* unitName, const char* message)");
        CHECK(host != std::string::npos);
        const size_t call = (host == std::string::npos) ? host : s.find("ForwardShowErrorMessage(code, 0, pos);", host);
        CHECK(call != std::string::npos);                                   // kcode 0: never the blocking wait
        const size_t fwd = s.find("static int ForwardShowErrorMessage(const char* code, int kcode, int pos)");
        const size_t kz  = (fwd == std::string::npos) ? fwd : s.find("if (kcode == 0) {", fwd);
        const size_t pq  = (fwd == std::string::npos) ? fwd : s.find("g_modalServer->PostQuery(qid", fwd);
        CHECK(fwd != std::string::npos && kz != std::string::npos && pq != std::string::npos && kz < pq);   // the notice branch comes first ...
        const size_t ret = (kz == std::string::npos) ? kz : s.find("return 0;", kz);
        CHECK(ret != std::string::npos && pq != std::string::npos && ret < pq);                              // ... and returns before the wait
        CHECK(s.find("if (!g_w906MotorNoteMsg) { extern void W906_AlarmStopLikeGolden(const char*);") != std::string::npos);
    }

    W906_SetGaliRoute(0);
    MOT[MInArmX].Motor = savedInArmX;
    delete inArmX;
    delete slEventLog;
    slEventLog = savedEv;
    if (g_fail == 0) RemoveTree(evDir);                                     // this run's own event-log folder (kept on a failure, for the diagnosis)
    InitialOK = saveInit;  IniConfig.bSPILFunction = saveSpil;  IniConfig.bO06SaveLogTimePeriod = saveO06;  CosFunction.bOLPFunction = saveOlp;
    std::printf("\n%d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
