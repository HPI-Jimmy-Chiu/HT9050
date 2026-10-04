// ===========================================================================
//  tests/test_notice_ack.cpp
//
//  AI(W906-J5-ACK) 20260930: INBOX 119 (Jerry J-5) -- WS dialog.notifyAck, the operator's acknowledgement of a kCode==0
//  notice.  Before it, tools/wb_serve.cpp ForwardShowErrorMessage's AI(W906-Q30-KZERO) branch (and, since INBOX 118,
//  every golden ShowMotorErrorMessage note) left Alarm-dialog-request.json "pending" for good: nothing retired it, the
//  page's dialog.response was refused, a reload showed it again, only a wb_serve restart cleared it.
//
//  The handler logic is w906dlg::NotifyAckHandle (tools/wb_dialog_mailbox.h); wb_serve.cpp W906_NoticeAckCommand binds
//  it to DialogMailboxRetire() (= w906dlg::AlarmRetire on its mailbox) and to golden's close in forms/fNote_ShowError.cpp
//  (W906_NoteNoticeAckRefusal / W906_NoteNoticeAckLikeGolden).  This test runs the SAME NotifyAckHandle with the SAME two
//  golden functions and AlarmRetire on a scratch mailbox, then pins the wb_serve binding in source ([M]).
//
//    [A] the two mailbox writers (AlarmRequestJson / AlarmIdleJson) are byte-identical to the snprintf code they replaced
//        in wb_serve.cpp (a verbatim copy is below) and parse as JSON
//    [B] a kcode==0 notice posts the mailbox pending (.json and shim), slot kNotice
//    [C] a wrong tag -> ok:false "request-mismatch:current=<id>"; both files byte-identical; nothing applied
//    [D] the right tag -> ok:true: both files idle (content == AlarmIdleJson(seq)), the ack payload, and golden's close
//        (BtnPauseClick KeyCode==0 note.cpp:4044-4153 + FormClose :2503-2762): SoftStop / SoftStart, EventReport(DoPause),
//        the jam count, PassTime, Recovery, the FormClose flags
//    [E] a second ack of the same id -> "no-pending-notice"; the jam count does not move again
//    [F] a blocking alarm (K_RETRY) pending -> "not-a-notice:current=<id>"; nothing retired, slot kBlocking
//    [G] restarted since the notice (SystemStart) -> retired and PAUSED as golden: pause "applied" (AI(W906-I37A) 20261002, RULINGS_20261002 #5 = A; it was "skipped-machine-running": no PAUSE, the records only)
//    [H] the motor note (its body applied the answer at post) -> pause "already-applied": SoftStop / SoftStart untouched
//    [I] golden BtnPauseClick's refusal (CC_ASE_SG + bTesterSendPause, note.cpp:3833-3834) -> "golden-refused:..."; still
//        pending; accepted once the refusal is gone
//    [J] no golden note (the record half did not run, note.cpp:541 / :817) -> retired, pause "no-golden-note", no effect
//    [K] a failed retire -> "retire-failed:current=<id>"; the slot stays kNotice and nothing is applied (a retry works)
//    [L] the handler never blocks: every call returns within AtomicWrite's own retry bound (2 s; measured 47-312 ms)
//    [M] source pins: the wb_serve.cpp dispatch, handler, blocking-wait answer and capture; the token exemption   [N] AI(W906-I37A) 20261002: INBOX 123 W906_NoteBlockingPauseLikeGolden (behaviour + the two wb_serve.cpp callers)   [O] AI(W906-I124) 20261003: INBOX 124 the panel PAUSE closes a notice (wb_serve.cpp W906_NoticePanelKeyTick, source pins)   [P] AI(W906-ARMCELL-NOTICE) 20261003: INBOX 151 w906dlg::NoticePendingWhy + its wb_serve.cpp / ArmCellLive.cpp binding (source pins)
//    [Q] AI(W906-S17) 20261003 (St01, S-17): boxes C++ has left behind close -- B older ids, E the alarm pump's notifyAck,
//        A stale answers inside a wait, D Dialog-close-request recent[]; source pins for the three waits and the close writer
//  Writes only its own mailbox folder under the redirected as9045LogPath (the guard refuses to run anywhere else);
//  MyDBUEventRecover is the CSV-only no-op, EventReport is the Sim recorder, Alarm is NULL in ctest.
// ===========================================================================
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include "forms/fNote.h"
#include "cmydef.h"                 // SoftStop / SoftStart / SystemStart / bOpenAllDoor / sAlarmMes / bAlarmReset / lamps / CUSTOMER_CODE / K_RETRY
#include "LastSet.h"                // LastSet.iRealDummy / iJamCount
#include "Config.h"                 // IniConfig.bEnable_SECS_GEM
#include "CosFunction.h"            // CosFunction.bIncludeMTBA
#include "MachineType.h"            // CC_ASE_SG
#include "common.h"                 // as9045LogPath
#include "SECSGEM/SecsEventType.h"  // SECS_EVENT.DoPause
#include "SECSGEM/SecsEventReport.h"// g_SimLastEventReportCeid / ResetSimEventReport
#include "Public/cJSON.h"
#include "../tools/wb_dialog_mailbox.h"
#include "w906_ctest_guard.h"

void        W906_NoteNoticeCapture(const char* requestId, bool motorNote);    // forms/fNote_ShowError.cpp EOF
const char* W906_NoteNoticeAckRefusal(const char* requestId);
bool        W906_NoteNoticeAckLikeGolden(const char* requestId, int* pause, bool* jamCounted, unsigned long* passTimeSec);  void W906_NoteBlockingPauseLikeGolden(int k);   // AI(W906-I37A) 20261002
extern int        iDuplicateError;                                            // forms/fNote_ShowError.cpp (golden note.cpp:84)
extern int        iEventID;                                                   // (golden note.cpp:82)
extern bool       W906_ShowErrorMessage_Recorded;
extern AnsiString Recovery;                                                   // (golden note.cpp:118)
extern bool       bTesterSendPause;                                           // cmydef.h:4970

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_notice_ack.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---------------------------------------------------------------------------
//  [A] reference: the two writers exactly as tools/wb_serve.cpp had them before AI(W906-J5-ACK) (verbatim)
// ---------------------------------------------------------------------------
#if defined(__GNUC__)   // the verbatim copies keep their %llu, which MinGW 6.3's -Wformat does not know (msvcrt); the runtime does ([B] checks the seq)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif
static std::string OldPostAlarmJson(unsigned long long seq, const std::string& requestId, const char* code, int kcode,
                                    int pos, const char* g_w906MotorNoteUnit, const char* g_w906MotorNoteMsg)
{
    const bool blocking = true;   // ShowErrorMessage 專用；NonStop 那條要傳 false
    char head[256];
    std::snprintf(head, sizeof(head),
        "{\"schemaVersion\":\"1.0.0\",\"channel\":\"show-error-message\","
        "\"seq\":%llu,\"requestId\":\"%s\",\"state\":\"pending\","
        "\"function\":\"ShowErrorMessage\",\"blocking\":%s,",
        seq, w906dlg::JsonEscape(requestId).c_str(),
        blocking ? "true" : "false");

    char args[192];
    std::snprintf(args, sizeof(args),
        "\"arguments\":{\"code\":\"%s\",\"kCode\":%d,\"position\":%d,"
        "\"duplicateError\":false,\"errorPart\":\"\"},",
        w906dlg::JsonEscape(code ? code : "").c_str(), kcode, pos);

    std::string json = std::string(head) + args +
        "\"display\":{\"alarmType\":null,\"unitName\":\"" + w906dlg::JsonEscape(g_w906MotorNoteUnit ? g_w906MotorNoteUnit : "") + "\",\"message\":\""
        + w906dlg::JsonEscape(g_w906MotorNoteMsg ? g_w906MotorNoteMsg : (code ? code : "")) +
        "\",\"jamArea\":\"\",\"description\":\"\",\"flushPanel\":null},"
        "\"auth\":{\"required\":false,\"kind\":\"none\",\"level\":null,"
        "\"title\":\"Password\",\"prompt\":\"\",\"userIdRequired\":true,"
        "\"defaultUserId\":null},"
        "\"buttons\":[],\"closePolicy\":\"acknowledge-only\",\"error\":null}";
    return json;
}
static std::string OldRetireJson(unsigned long long seq)
{
    char json[1024];
    const int wrote = std::snprintf(json, sizeof(json),
        "{\"schemaVersion\":\"1.0.0\",\"channel\":\"show-error-message\","
        "\"seq\":%llu,\"requestId\":\"\",\"state\":\"idle\","
        "\"function\":\"ShowErrorMessage\",\"blocking\":true,"
        "\"arguments\":{\"code\":\"\",\"kCode\":0,\"position\":0,"
        "\"duplicateError\":false,\"errorPart\":\"\"},"
        "\"display\":{\"alarmType\":null,\"unitName\":\"\",\"message\":\"\","
        "\"jamArea\":\"\",\"description\":\"\",\"flushPanel\":null},"
        "\"auth\":{\"required\":false,\"kind\":\"none\",\"level\":null,"
        "\"title\":\"Password\",\"prompt\":\"\",\"userIdRequired\":true,"
        "\"defaultUserId\":null},"
        "\"buttons\":[],\"closePolicy\":\"acknowledge-only\",\"error\":null}",
        seq);
    if (wrote < 0 || (size_t)wrote >= sizeof(json)) return std::string();
    return json;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// ---------------------------------------------------------------------------
static std::string Slurp(const std::string& path)
{
    std::string s;
    w906dlg::ReadWholeFile(path, s);
    return s;
}
static std::string ReadSource(const char* rel)
{
    std::ifstream f((std::string(W906_SRC_ROOT) + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static bool MakeDirs(const std::string& path)
{
    for (size_t i = 3; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '\\' || path[i] == '/') {
            const std::string p = path.substr(0, i);
            if (::GetFileAttributesA(p.c_str()) == INVALID_FILE_ATTRIBUTES && !::CreateDirectoryA(p.c_str(), NULL))
                return false;
        }
    }
    return true;
}
static void RemoveMailbox(const std::string& dir)
{
    const std::string j = dir + "\\Alarm-dialog-request.json", s = dir + "\\js\\Alarm-dialog-request.js";
    ::DeleteFileA(j.c_str()); ::DeleteFileA((j + ".tmp_wb").c_str());
    ::DeleteFileA(s.c_str()); ::DeleteFileA((s + ".tmp_wb").c_str());
    ::RemoveDirectoryA((dir + "\\js").c_str());
    ::RemoveDirectoryA(dir.c_str());
}
static std::string JsonStr(const cJSON* o, const char* k)
{
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k);
    return (v && cJSON_IsString(v) && v->valuestring) ? std::string(v->valuestring) : std::string("<none>");
}
static double JsonNum(const cJSON* o, const char* k)
{
    const cJSON* v = cJSON_GetObjectItemCaseSensitive(o, k);
    return (v && cJSON_IsNumber(v)) ? v->valuedouble : -1.0;
}
// the shim's value: `window.__HT9045_DATA__["Alarm-dialog-request"]=<json>;` on its second line
static std::string ShimValue(const std::string& shim)
{
    const std::string key = "window.__HT9045_DATA__[\"Alarm-dialog-request\"]=";
    const size_t p = shim.find(key);
    if (p == std::string::npos) return std::string();
    const size_t b = p + key.size();
    const size_t e = shim.find(";\r\n", b);
    return (e == std::string::npos) ? std::string() : shim.substr(b, e - b);
}

// ---------------------------------------------------------------------------
//  the handler under test: w906dlg::NotifyAckHandle with the same refuse / close functions as tools/wb_serve.cpp
//  W906_NoticeAckCommand; the retire is w906dlg::AlarmRetire, which is all DialogMailboxRetire() does.
// ---------------------------------------------------------------------------
static w906dlg::AlarmSlot   g_slot;
static unsigned long long   g_seq = 1000;
static std::string          g_dir;
static DWORD                g_slowestMs = 0;
static bool Ack(const std::string& tag, std::string* ack, const std::string& dir)
{
    const DWORD t0 = ::GetTickCount();
    const bool ok = w906dlg::NotifyAckHandle(g_slot, tag,
        [](const std::string& id) { const char* why = W906_NoteNoticeAckRefusal(id.c_str()); return std::string(why ? why : ""); },
        [&dir]() { return w906dlg::AlarmRetire(g_slot, dir, g_seq); },
        [](const std::string& id, std::string* okJson) {
            int pause = 3; bool jam = false; unsigned long passSec = 0;
            W906_NoteNoticeAckLikeGolden(id.c_str(), &pause, &jam, &passSec);
            *okJson = w906dlg::NotifyAckOkJson(id, g_seq, pause, jam, passSec);
        },
        ack);
    const DWORD dt = ::GetTickCount() - t0;
    if (dt > g_slowestMs) g_slowestMs = dt;
    std::printf("    ack tag=%s -> %s %s  (%lu ms)\n", tag.c_str(), ok ? "ok" : "refused", ack->c_str(), (unsigned long)dt);
    return ok;
}

// what ForwardShowErrorMessage's record half leaves in fNote before the kcode==0 branch posts (forms/fNote_ShowError.cpp)
static void RecordLike(const char* code, int alarmType, bool recorded)
{
    fNote->edErrorCode->Text = AnsiString(code);
    fNote->AlarmType = alarmType;
    iDuplicateError = 0;
    iEventID = 7;
    W906_ShowErrorMessage_Recorded = recorded;
}
// the stop the notice put the machine in (W906_AlarmStopLikeGolden, golden note.cpp:799-801)
static void StoppedLikeNotice()
{
    SoftStop = false; SoftStart = false; SystemStart = false;
}
static void PostNotice(const char* qid, const char* code, bool motorNote)
{
    CHECK(w906dlg::AlarmPost(g_slot, g_dir, g_seq, qid, code, 0, 3, motorNote ? "MTestZ1" : "", motorNote ? "Test Z1 servo alarm" : code, true));
    W906_NoteNoticeCapture(qid, motorNote);
}
static int JamCount() { return LastSet.iJamCount[0]; }

int main()
{
    const char* const rt0[] = { "as9045LogPath", as9045LogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NoticeAck", rt0))
        return 2;
    char stamp[64];
    std::snprintf(stamp, sizeof(stamp), "\\NoticeAck_%lu", (unsigned long)::GetTickCount());
    g_dir = std::string(as9045LogPath.c_str()) + stamp;          // this run's own mailbox (ctest -j: others share the scratch)
    const char* const rt[] = { "mailbox", g_dir.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NoticeAck", rt))
        return 2;
    CHECK(MakeDirs(g_dir + "\\js"));
    std::printf("=== test_notice_ack (AI(W906-J5-ACK), INBOX 119) -- mailbox %s ===\n", g_dir.c_str());
    CHECK(fNote != 0);
    if (fNote == 0) return 1;
    const std::string aJson = g_dir + "\\Alarm-dialog-request.json";
    const std::string aShim = g_dir + "\\js\\Alarm-dialog-request.js";

    const int  saveCust = CUSTOMER_CODE, saveReal = LastSet.iRealDummy;
    const bool saveSecs = IniConfig.bEnable_SECS_GEM, saveMtba = CosFunction.bIncludeMTBA, saveTsp = bTesterSendPause;
    LastSet.iRealDummy = REALLY;
    CosFunction.bIncludeMTBA = false;
    IniConfig.bEnable_SECS_GEM = true;
    CUSTOMER_CODE = 0;
    bTesterSendPause = false;

    // ---- [A] ------------------------------------------------------------------------------------------------
    std::printf("[A] the writers are byte-identical to the snprintf code they replaced, and parse\n");
    {
        struct S { unsigned long long seq; const char* id; const char* code; int k; int pos; const char* unit; const char* msg; };
        const S samples[] = {
            { 1, "17", "JAM0508", 0, 3, 0, 0 },
            { 1790000000123ULL, "42", "WAR240147", 0, 14, "MTestZ1", "Test Z1 servo alarm" },
            { 9, "5", "MES0920", K_RETRY | K_SKIP, 0, 0, 0 },
            { 10, "6", "A\"B\\C", 0, -1, "u\tx", "m\nq" },
        };
        for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
            const S& s = samples[i];
            const std::string neu = w906dlg::AlarmRequestJson(s.seq, s.id, s.code, s.k, s.pos, s.unit ? s.unit : "",
                                                              s.msg ? s.msg : s.code, true);
            CHECK(neu == OldPostAlarmJson(s.seq, s.id, s.code, s.k, s.pos, s.unit, s.msg));
            cJSON* r = cJSON_Parse(neu.c_str());
            CHECK(r != 0);
            if (r) { CHECK(JsonStr(r, "state") == "pending"); CHECK(JsonStr(r, "requestId") == s.id); cJSON_Delete(r); }
        }
        const unsigned long long seqs[] = { 0, 1, 1790000000999ULL };
        for (int i = 0; i < 3; ++i) {
            CHECK(w906dlg::AlarmIdleJson(seqs[i]) == OldRetireJson(seqs[i]));
            cJSON* r = cJSON_Parse(w906dlg::AlarmIdleJson(seqs[i]).c_str());
            CHECK(r != 0);
            if (r) { CHECK(JsonStr(r, "state") == "idle"); cJSON_Delete(r); }
        }
    }

    // ---- [B] ------------------------------------------------------------------------------------------------
    std::printf("[B] a kcode==0 notice posts the mailbox pending\n");
    RecordLike("JAM0508", 1, true);
    StoppedLikeNotice();
    PostNotice("17", "JAM0508", false);
    CHECK(g_slot.kind == w906dlg::AlarmSlot::kNotice && g_slot.requestId == "17");
    const unsigned long long postSeq = g_slot.seq;
    const std::string j1 = Slurp(aJson), s1 = Slurp(aShim);
    {
        cJSON* r = cJSON_Parse(j1.c_str());
        CHECK(r != 0);
        if (r) {
            CHECK(JsonStr(r, "state") == "pending");
            CHECK(JsonStr(r, "requestId") == "17");
            CHECK(JsonStr(r, "closePolicy") == "acknowledge-only");
            const cJSON* a = cJSON_GetObjectItemCaseSensitive(r, "arguments");
            CHECK(a != 0 && JsonNum(a, "kCode") == 0.0);                     // what identifies a notice
            CHECK(JsonNum(r, "seq") == (double)postSeq);
            cJSON_Delete(r);
        }
        CHECK(ShimValue(s1) + "\n" == j1);                                    // the shim carries the same request
        CHECK(w906dlg::TextSaysIdle(s1) == false);
    }

    // ---- [C] ------------------------------------------------------------------------------------------------
    std::printf("[C] a wrong tag: request-mismatch, nothing written, nothing applied\n");
    {
        std::string ack;
        ResetSimEventReport();
        const int jam0 = JamCount();
        CHECK(Ack("99", &ack, g_dir) == false);                               //AI(W906-S17B) 20261003: was "16" -- an OLDER id is now a
        CHECK(ack == "request-mismatch:current=17");                          //  left-behind box (superseded, below); a never-issued newer one still mismatches
        CHECK(Ack("16", &ack, g_dir) == false && ack == "no-pending-notice:superseded-by=17");   //AI(W906-S17B): older than the slot, no wait holds it
        CHECK(Ack("", &ack, g_dir) == false && ack == "request-mismatch:current=17");   // no tag at all
        CHECK(Slurp(aJson) == j1 && Slurp(aShim) == s1);
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kNotice && g_slot.requestId == "17" && g_slot.seq == postSeq);
        CHECK(SoftStop == false && JamCount() == jam0 && g_SimEventReportCount == 0);
    }

    // ---- [D] ------------------------------------------------------------------------------------------------
    std::printf("[D] the right tag: retired, and golden's close (BtnPauseClick KeyCode==0 + FormClose)\n");
    {
        bOpenAllDoor = false; sAlarmMes = "left over"; bAlarmReset = true; bLampSkip = true; bLampTrayEnd = true;
        bInArmNeedToSafePos = true; bAutoRetestJam = true; iJamSkipICCount = 3; bShowNoteCleanSocket = true;
        bSECSGEM_NoteAlarm = true; bAlarmAfterPreAlarm = true; bEnterTestIF = false; bStartMoveSpeed = true;
        Recovery = "RETRY";
        ResetSimEventReport();
        const int jam0 = JamCount();
        ::Sleep(1100);                                                        // PassTime counts whole seconds (golden :2545)
        std::string ack;
        CHECK(Ack("17", &ack, g_dir) == true);
        const std::string j2 = Slurp(aJson), s2 = Slurp(aShim);
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle && g_slot.requestId.empty());
        CHECK(j2 == w906dlg::AlarmIdleJson(g_seq) + "\n");                    // the file content, not just the ack
        CHECK(ShimValue(s2) == w906dlg::AlarmIdleJson(g_seq));
        CHECK(w906dlg::TextSaysIdle(j2) && w906dlg::TextSaysIdle(s2));
        CHECK(j2.find("\"requestId\":\"17\"") == std::string::npos);
        cJSON* r = cJSON_Parse(("{\"type\":\"ack\",\"id\":1,\"ok\":true," + ack.substr(1)).c_str());   // as WebBridgeServer AckJson splices it
        CHECK(r != 0);
        if (r) {
            CHECK(JsonStr(r, "notice") == "retired");
            CHECK(JsonStr(r, "requestId") == "17");
            CHECK(JsonStr(r, "pause") == "applied");
            CHECK(JsonNum(r, "seq") == (double)g_seq);
            CHECK(cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(r, "jamCounted")));
            CHECK(JsonNum(r, "passTime") >= 1.0);
            CHECK(JsonStr(r, "type") == "ack" && JsonNum(r, "id") == 1.0);   // no transport key overwritten
            cJSON_Delete(r);
        }
        // BtnPauseClick (golden :3840, :4146-4151)
        CHECK(bStartMoveSpeed == false);
        CHECK(SoftStop == true && SoftStart == false);
        CHECK(g_SimLastEventReportCeid == (unsigned)SECS_EVENT.DoPause);
        // FormClose (golden :2513 ... :2760)
        CHECK(bShowNoteCleanSocket == false && bAlarmReset == false && bSECSGEM_NoteAlarm == false && bAlarmAfterPreAlarm == false);
        CHECK(JamCount() == jam0 + 1);                                        // :2528-2529 -- JAM0508, AlarmType 1, REALLY
        CHECK(bLampSkip == false && bLampTrayEnd == false);
        CHECK(std::strcmp(Recovery.c_str(), "") == 0);                                                // :2548-2549 KeyCode==0
        CHECK(bEnterTestIF == true);
        CHECK(iJamSkipICCount == 0 && std::strcmp(sAlarmMes.c_str(), "") == 0 && bAutoRetestJam == false && bOpenAllDoor == true);
        CHECK(bInArmNeedToSafePos == false);
        CHECK(std::strcmp(fNote->edErrorCode->Text.c_str(), "") == 0);                                // :2673
    }

    // ---- [E] ------------------------------------------------------------------------------------------------
    std::printf("[E] the same id again: no-pending-notice, nothing applied twice\n");
    {
        SoftStop = false;
        ResetSimEventReport();
        const int jam0 = JamCount();
        const std::string j2 = Slurp(aJson);
        std::string ack;
        CHECK(Ack("17", &ack, g_dir) == false && ack == "no-pending-notice");
        CHECK(JamCount() == jam0 && SoftStop == false && g_SimEventReportCount == 0);
        CHECK(Slurp(aJson) == j2);
        CHECK(W906_NoteNoticeAckLikeGolden("17", 0, 0, 0) == false);          // the close itself is once-only too
    }

    // ---- [F] ------------------------------------------------------------------------------------------------
    std::printf("[F] a blocking alarm pending: not-a-notice, nothing retired\n");
    {
        CHECK(w906dlg::AlarmPost(g_slot, g_dir, g_seq, "18", "JAM0301", K_RETRY, 3, "", "JAM0301", true));
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kBlocking);
        const std::string jb = Slurp(aJson), sb = Slurp(aShim);
        std::string ack;
        CHECK(Ack("18", &ack, g_dir) == false && ack == "not-a-notice:current=18");
        CHECK(Ack("17", &ack, g_dir) == false && ack == "no-pending-notice:superseded-by=18");   //AI(W906-S17B) 20261003: was not-a-notice -- 17 (retired in [D]) is older than the blocking 18: its box is stale, the page closes it
        CHECK(Ack("99", &ack, g_dir) == false && ack == "not-a-notice:current=18");             //  a newer, never-issued id: unchanged
        CHECK(Slurp(aJson) == jb && Slurp(aShim) == sb && !w906dlg::TextSaysIdle(jb));
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kBlocking && g_slot.requestId == "18");
        CHECK(w906dlg::AlarmRetire(g_slot, g_dir, g_seq));                    // the wait loop's own retire (DialogMailboxRetire)
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle);
        CHECK(Ack("18", &ack, g_dir) == false && ack == "no-pending-notice");
    }

    // ---- [G] ------------------------------------------------------------------------------------------------
    std::printf("[G] restarted since the notice: retired, and PAUSED as golden (RULINGS_20261002 #5 = A; was pause 2)\n");
    {
        RecordLike("JAM0508", 1, true);
        StoppedLikeNotice();
        PostNotice("19", "JAM0508", false);
        SystemStart = true;                                                   // START pressed while the box was up
        bOpenAllDoor = false; bInArmNeedToSafePos = true;
        ResetSimEventReport();
        const int jam0 = JamCount();
        std::string ack;
        CHECK(Ack("19", &ack, g_dir) == true);
        CHECK(ack.find("\"pause\":\"applied\"") != std::string::npos);   // AI(W906-I37A) 20261002: was "skipped-machine-running"
        CHECK(SoftStop == true && SoftStart == false && SystemStart == true);    // golden :4146-4148: SoftStop -- MainProc pauses the running machine
        CHECK(g_SimLastEventReportCeid == (unsigned)SECS_EVENT.DoPause);      // golden :4149-4150
        CHECK(bOpenAllDoor == true && bInArmNeedToSafePos == false);          // FormClose runs (golden :2503-2762), as for any PAUSE
        CHECK(JamCount() == jam0 + 1);                                        // :2528-2529
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle && w906dlg::TextSaysIdle(Slurp(aJson)));
        SystemStart = false; SoftStop = false;
    }

    // ---- [H] ------------------------------------------------------------------------------------------------
    std::printf("[H] the motor note: its body applied the answer at post -- not again\n");
    {
        RecordLike("WAR240147", 2, false);                                    // the motor body writes fNote itself; Recorded is stale
        SoftStop = false; SoftStart = false; SystemStart = false;
        PostNotice("20", "WAR240147", true);
        SoftStop = false;                                                     // the kernel consumed the body's SoftStop=true (ckernel.cpp:1046)
        ResetSimEventReport();
        const int jam0 = JamCount();
        std::string ack;
        CHECK(Ack("20", &ack, g_dir) == true);
        CHECK(ack.find("\"pause\":\"already-applied\"") != std::string::npos);
        CHECK(SoftStop == false && SoftStart == false);                       // not set a second time
        CHECK(g_SimLastEventReportCeid == (unsigned)SECS_EVENT.DoPause);      // the PAUSE press is still reported (golden :4150)
        CHECK(JamCount() == jam0);                                            // WAR: AlarmType 2
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle);
    }

    // ---- [I] ------------------------------------------------------------------------------------------------
    std::printf("[I] golden BtnPauseClick refuses (CC_ASE_SG + bTesterSendPause): the box stays\n");
    {
        RecordLike("JAM0508", 1, true);
        StoppedLikeNotice();
        PostNotice("21", "JAM0508", false);
        CUSTOMER_CODE = CC_ASE_SG; bTesterSendPause = true;
        const std::string ji = Slurp(aJson);
        const int jam0 = JamCount();
        std::string ack;
        CHECK(Ack("21", &ack, g_dir) == false);
        CHECK(ack.compare(0, 15, "golden-refused:") == 0);
        CHECK(Slurp(aJson) == ji && g_slot.kind == w906dlg::AlarmSlot::kNotice && SoftStop == false && JamCount() == jam0);
        bTesterSendPause = false;
        CHECK(Ack("21", &ack, g_dir) == true && JamCount() == jam0 + 1 && SoftStop == true);
        CUSTOMER_CODE = 0;
    }

    // ---- [J] ------------------------------------------------------------------------------------------------
    std::printf("[J] no golden note (the record half did not run): retired, nothing applied\n");
    {
        RecordLike("JAM0508", 1, false);
        StoppedLikeNotice();
        PostNotice("22", "JAM0508", false);
        ResetSimEventReport();
        const int jam0 = JamCount();
        bOpenAllDoor = false;
        std::string ack;
        CHECK(Ack("22", &ack, g_dir) == true);
        CHECK(ack.find("\"pause\":\"no-golden-note\"") != std::string::npos);
        CHECK(SoftStop == false && g_SimEventReportCount == 0 && JamCount() == jam0 && bOpenAllDoor == false);
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle);
    }

    // ---- [K] ------------------------------------------------------------------------------------------------
    std::printf("[K] a failed retire: retire-failed, the slot stays, nothing applied; the retry works\n");
    {
        RecordLike("JAM0508", 1, true);
        StoppedLikeNotice();
        PostNotice("23", "JAM0508", false);
        const std::string gone = g_dir + "\\no_such_dir";                     // fopen fails at once (no retry loop)
        ResetSimEventReport();
        const int jam0 = JamCount();
        std::string ack;
        CHECK(Ack("23", &ack, gone) == false && ack == "retire-failed:current=23");
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kNotice && g_slot.requestId == "23");
        CHECK(SoftStop == false && JamCount() == jam0 && g_SimEventReportCount == 0);
        CHECK(!w906dlg::TextSaysIdle(Slurp(aJson)));
        CHECK(Ack("23", &ack, g_dir) == true && JamCount() == jam0 + 1);
        CHECK(w906dlg::TextSaysIdle(Slurp(aJson)));
    }

    // ---- [K2] -----------------------------------------------------------------------------------------------
    //AI(W906-J5-ACK-2) 20261001: measured on the laptop 20261001 09:59:54-58 -- ShowLoadingIC posted WAR07324 eight times in
    //   four seconds (requestId 1..8).  The page queues every request (dialog-bridge.js queueStop, FIFO, never dropped) and
    //   showed box 1 first; its dialog.notifyAck tag=1 answered request-mismatch:current=8, the host keeps the box on that
    //   error, so box 8 waited behind seven boxes that could never close.  A superseded notice now answers
    //   no-pending-notice:superseded-by=<current> (the host closes on the no-pending-notice prefix), nothing written, nothing applied.
    std::printf("[K2] a notice overwritten by a newer one: the stale box closes, the current one still acks\n");
    {
        RecordLike("WAR07324", 2, true);
        StoppedLikeNotice();
        PostNotice("24", "WAR07324", false);
        PostNotice("25", "WAR07324", false);                                  // the single slot now holds 25; 24 was overwritten
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kNotice && g_slot.requestId == "25" && g_slot.WasSuperseded("24"));
        const std::string jk = Slurp(aJson), sk = Slurp(aShim);
        ResetSimEventReport();
        const int jam0 = JamCount();
        std::string ack;
        CHECK(Ack("24", &ack, g_dir) == false && ack == "no-pending-notice:superseded-by=25");
        CHECK(ack.compare(0, 17, "no-pending-notice") == 0);                  // the prefix ht9045_dialog_host.js closes on
        CHECK(Slurp(aJson) == jk && Slurp(aShim) == sk);                      // nothing written
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kNotice && g_slot.requestId == "25");
        CHECK(SoftStop == false && JamCount() == jam0 && g_SimEventReportCount == 0);   // nothing applied for the stale one
        CHECK(Ack("99", &ack, g_dir) == false && ack == "request-mismatch:current=25");  // an id never posted: unchanged (AI(W906-S17B) 20261003: a NEWER one -- an older one is superseded now)
        CHECK(Ack("25", &ack, g_dir) == true);                                // the current one retires as before
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle && g_slot.superseded.empty());
        CHECK(Ack("24", &ack, g_dir) == false && ack == "no-pending-notice");   // idle answers for everything, as before
        // overwritten by a BLOCKING alarm: the stale notice box still closes; the blocking one is not-a-notice as before
        RecordLike("WAR07324", 2, true);
        StoppedLikeNotice();
        PostNotice("26", "WAR07324", false);
        CHECK(w906dlg::AlarmPost(g_slot, g_dir, g_seq, "27", "JAM0301", K_RETRY, 3, "", "JAM0301", true));
        CHECK(Ack("26", &ack, g_dir) == false && ack == "no-pending-notice:superseded-by=27");
        CHECK(Ack("27", &ack, g_dir) == false && ack == "not-a-notice:current=27");
        CHECK(Ack("99", &ack, g_dir) == false && ack == "not-a-notice:current=27");   //AI(W906-S17B) 20261003: was "16" (older -> superseded now, see [Q])
        CHECK(w906dlg::AlarmRetire(g_slot, g_dir, g_seq));                    // the wait loop's own retire
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle && g_slot.superseded.empty());
    }

    // ---- [L] ------------------------------------------------------------------------------------------------
    std::printf("[L] never blocks: slowest call %lu ms\n", (unsigned long)g_slowestMs);
    // No wait for anybody: the only delay is w906dlg::AtomicWrite's own sharing-violation retry, at most 20 x 50 ms per file,
    // two files (tools/wb_dialog_mailbox.h) -- so 2 s bounds it even when the virus scanner holds the new file (measured
    // 20260930: 47-203 ms build_ship, up to 312 ms build_sim, on a laptop running another full gate).
    CHECK(g_slowestMs < 2500);

    // ---- [M] ------------------------------------------------------------------------------------------------
    std::printf("[M] source pins: tools/wb_serve.cpp, WebBridge/WebBridgeServer.cpp\n");
    {
        const std::string s = ReadSource("tools/wb_serve.cpp");
        CHECK(!s.empty());
        CHECK(s.find("} else if (wc.cmd == \"dialog.notifyAck\") { extern void W906_NoticeAckCommand(webbridge::WebBridgeServer&, "
                     "const webbridge::WebCommand&); W906_NoticeAckCommand(server, wc);") != std::string::npos);
        const size_t h = s.find("void W906_NoticeAckCommand(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)\r\n{");
        const size_t h2 = (h == std::string::npos) ? h : s.find("void W906_NoticeAckCommand(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)\n{");
        const size_t hb = (h != std::string::npos) ? h : h2;
        CHECK(hb != std::string::npos);
        if (hb != std::string::npos) {
            size_t e = s.find("\n}", hb);
            const std::string body = s.substr(hb, (e == std::string::npos ? s.size() : e) - hb);
            CHECK(body.find("w906dlg::NotifyAckHandle(g_alarmSlot, tag,") != std::string::npos);
            CHECK(body.find("W906_NoteNoticeAckRefusal(id.c_str())") != std::string::npos);
            CHECK(body.find("[]() { return DialogMailboxRetire(); }") != std::string::npos);
            CHECK(body.find("W906_NoteNoticeAckLikeGolden(id.c_str(), &pause, &jam, &passSec);") != std::string::npos);
            CHECK(body.find("w906dlg::NotifyAckOkJson(id, g_dialogSeq, pause, jam, passSec)") != std::string::npos);
            CHECK(body.find("server.CompleteCommand((unsigned long long)wc.id, ok, ack);") != std::string::npos);
            CHECK(body.find("PostQuery") == std::string::npos && body.find("ClearQuery") == std::string::npos);
            CHECK(body.find("waitForPush") == std::string::npos && body.find("for (;;)") == std::string::npos && body.find("Sleep(") == std::string::npos);
        }
        const size_t fwd = s.find("static int ForwardShowErrorMessage(const char* code, int kcode, int pos)");
        const size_t yn  = s.find("static int ForwardShowMyMessageBoxYesNo(");
        const size_t na  = s.find("if (wc.cmd == \"dialog.notifyAck\") { g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "
                                  "w906dlg::WaitNotifyAckReply(g_alarmSlot, wc.hasTag ? wc.tag : std::string(), qidStr)); } else if (wc.cmd == \"motor.stop\")");   //AI(W906-S17E) 20261003: was not-a-notice for every tag ([Q])
        CHECK(fwd != std::string::npos && yn != std::string::npos && na != std::string::npos && fwd < na && na < yn);   // inside the blocking wait
        const size_t kz  = (fwd == std::string::npos) ? fwd : s.find("if (kcode == 0) {", fwd);
        const size_t cap = (kz == std::string::npos) ? kz : s.find("W906_NoteNoticeCapture(qidStr, g_w906MotorNoteMsg != 0);", kz);
        const size_t ret = (kz == std::string::npos) ? kz : s.find("return 0;", kz);
        CHECK(kz != std::string::npos && cap != std::string::npos && ret != std::string::npos && cap < ret);   // captured before the notice returns
        const size_t pa = s.find("static bool DialogMailboxPostAlarm(");
        const size_t dr = s.find("static bool DialogMailboxRetire()");
        CHECK(pa != std::string::npos && s.find("w906dlg::AlarmPost(g_alarmSlot, g_dialogMailboxDir, g_dialogSeq, requestId,", pa) != std::string::npos);
        CHECK(dr != std::string::npos && s.find("w906dlg::AlarmRetire(g_alarmSlot, g_dialogMailboxDir, g_dialogSeq);", dr) != std::string::npos);
        const std::string w = ReadSource("WebBridge/WebBridgeServer.cpp");
        CHECK(w.find("cmdName != \"dialog.response\" && cmdName != \"dialog.notifyAck\"") != std::string::npos);   // token-exempt
    }

    // ---- [N] AI(W906-I37A) 20261002 -----------------------------------------------------------------------------------
    std::printf("[N] INBOX 123: a blocking alarm answered PAUSE -- golden BtnPauseClick's Select[] arm (906 note.cpp:3840, :3983-4038)\n");
    {
        IniConfig.bEnable_SECS_GEM = true;
        bStartMoveSpeed = true;
        ResetSimEventReport();
        W906_NoteBlockingPauseLikeGolden(K_SKIP);
        CHECK(bStartMoveSpeed == false);                                       // :3840
#ifdef SOFT_SIMULTE
        CHECK(g_SimEventReportCount == 2);                                     // :3983-3989 DoSkip (SOFT_SIMULTE only), then :4036-4037 DoPause
#else
        CHECK(g_SimEventReportCount == 1);                                     // :4036-4037 DoPause (the key records are SOFT_SIMULTE only)
#endif
        CHECK(g_SimLastEventReportCeid == (unsigned)SECS_EVENT.DoPause);
        ResetSimEventReport();
        W906_NoteBlockingPauseLikeGolden(K_TRAIN);                             // i==7: a record, no event (golden :4023-4026)
        CHECK(g_SimEventReportCount == 1 && g_SimLastEventReportCeid == (unsigned)SECS_EVENT.DoPause);
        IniConfig.bEnable_SECS_GEM = false;
        ResetSimEventReport();
        W906_NoteBlockingPauseLikeGolden(K_RETRY);
        CHECK(g_SimEventReportCount == 0);                                     // every EventReport sits behind bEnable_SECS_GEM
        IniConfig.bEnable_SECS_GEM = true;
        const std::string s = ReadSource("tools/wb_serve.cpp");
        const size_t p1 = s.find("if (pressed == \"BtnPause\") {");
        const size_t p2 = s.find("if (ioPressed == \"BtnPause\") {");
        CHECK(p1 != std::string::npos && s.find("W906_NoteBlockingPauseLikeGolden(k);", p1) < s.find("W906_NoteJamCountOnClose(k);", p1));
        CHECK(p2 != std::string::npos && s.find("W906_NoteBlockingPauseLikeGolden(kio);", p2) < s.find("W906_NoteJamCountOnClose(kio);", p2));
        const std::string f = ReadSource("forms/fNote_ShowError.cpp");
        const size_t b = f.find("void W906_NoteBlockingPauseLikeGolden(int k)");
        CHECK(b != std::string::npos && f.find("EventReport(SECS_EVENT.DoPause);", b) < f.find("SendCommand_ESD(ESD_SYSTEM_STOP);", b));
    }

    // ---- [O] AI(W906-I124) 20261003 (NB2, INBOX 124) ------------------------------------------------------------------
    //  The panel PAUSE key closes a notice: golden TfNote::Timer1Timer -> ScanKey (906 note.cpp:3380 -> :3062-3066,
    //  MES2111 + BtnPauseClick) -> BtnPauseClick's KeyCode==0 arm.  W906_NoticePanelKeyTick (wb_serve.cpp EOF) cannot link
    //  here (wb_serve.cpp is the exe), so -- as [M] does for W906_NoticeAckCommand -- its binding is pinned in source; the exit
    //  it takes is the SAME NotifyAckHandle with the SAME refuse / retire / close functions the sections above run.
    std::printf("[O] INBOX 124: the panel PAUSE key takes the notice's 確認 exit (golden ScanKey -> BtnPauseClick KeyCode==0)\n");
    {
        const std::string s = ReadSource("tools/wb_serve.cpp");
        // (1) ScanKey's PAUSE branch reports a KeyCode==0 press (no selection), after golden's MES2111 record; START stays kcode != 0 only
        const size_t pb = s.find("NewRecordProcess(\"MES2111\", \"PAUSE pressed\", \"Note_ScanKey\");");
        const size_t pk = (pb == std::string::npos) ? pb : s.find("else if (kcode == 0) { *ans = \"PAUSE\"; *pressed = \"BtnPause\"; *input = \"SnFKPause\"; }", pb);
        CHECK(pb != std::string::npos && pk != std::string::npos && pk - pb < 400);
        CHECK(s.find("if (kcode != 0 &&                                                // :3069 BtnStart->Visible") != std::string::npos);
        // (2) called on EVERY tick-loop pass (next to the IO window ticks), not on the 500 ms pump beat
        CHECK(s.find("W906_IoFormShowTick(); }  { extern void W906_NoticePanelKeyTick(); W906_NoticePanelKeyTick(); }") != std::string::npos);
        // (3) the body: notices only, ScanKey with kcode 0, the auth gate first, then the 確認 exit, then the page's close
        size_t h = s.find("void W906_NoticePanelKeyTick()\r\n{");
        if (h == std::string::npos) h = s.find("void W906_NoticePanelKeyTick()\n{");
        CHECK(h != std::string::npos);
        if (h != std::string::npos) {
            const size_t e = s.find("\n}", h);
            const std::string body = s.substr(h, (e == std::string::npos ? s.size() : e) - h);
            const size_t k0 = body.find("if (g_alarmSlot.kind != w906dlg::AlarmSlot::kNotice || g_alarmSlot.requestId.empty()) return;");
            const size_t k1 = body.find("W906_AlarmIoAnswer(id.c_str(), 0, &ans, &pressed, &input);");
            const size_t k2 = body.find("if (pressed != \"BtnPause\") return;");
            const size_t k3 = body.find("if (!W906_NoteAuthNoticeGate(id, &why)) {");
            const size_t k4 = body.find("w906dlg::NotifyAckHandle(g_alarmSlot, id,");
            const size_t k5 = body.find("if (ok) W906_DialogCloseRequest(\"show-error-message\", id, reqSeq, \"PAUSE\", 0, \"SnFKPause\");");
            CHECK(k0 != std::string::npos && k1 != std::string::npos && k2 != std::string::npos && k3 != std::string::npos &&
                  k4 != std::string::npos && k5 != std::string::npos);
            CHECK(k0 < k1 && k1 < k2 && k2 < k3 && k3 < k4 && k4 < k5);
            CHECK(body.find("W906_NoteNoticeAckRefusal(rid.c_str())") != std::string::npos);
            CHECK(body.find("[]() { return DialogMailboxRetire(); }") != std::string::npos);
            CHECK(body.find("W906_NoteNoticeAckLikeGolden(rid.c_str(), &pause, &jam, &passSec);") != std::string::npos);
            CHECK(body.find("const unsigned long long reqSeq = g_alarmSlot.seq;") != std::string::npos && body.find("reqSeq = g_alarmSlot.seq;") < k4);
            CHECK(body.find("PostQuery") == std::string::npos && body.find("ClearQuery") == std::string::npos);
            CHECK(body.find("waitForPush") == std::string::npos && body.find("for (;;)") == std::string::npos && body.find("Sleep(") == std::string::npos);
        }
    }

    //AI(W906-ARMCELL-NOTICE) 20261003: NB2-1 -- INBOX 151 (RULINGS_20261003 #22 = s0 #70 (3)): the Teach Arm Cell job asks "is a kcode==0
    //  notice still open" (GO refused, a running job cancelled) -- w906dlg::NoticePendingWhy on wb_serve's g_alarmSlot.
    std::printf("[P] INBOX 151: a kcode==0 notice not yet acknowledged counts as a box up for the Teach Arm Cell job\n");
    {
        w906dlg::AlarmSlot sl;
        std::string what = "stale";
        CHECK(!w906dlg::NoticePendingWhy(sl, what) && what.empty());                                         // idle (boot / retired)
        sl.kind = w906dlg::AlarmSlot::kNotice; sl.requestId = "31";
        CHECK(w906dlg::NoticePendingWhy(sl, what) && what.find("通知框") != std::string::npos && what.find("31") != std::string::npos);
        sl.requestId.clear();
        CHECK(!w906dlg::NoticePendingWhy(sl, what));                                                           // a notice slot with no id is not one
        sl.kind = w906dlg::AlarmSlot::kBlocking; sl.requestId = "32";
        CHECK(!w906dlg::NoticePendingWhy(sl, what));                                                           // blocking = fNote->fShow, asked first
        CHECK(!w906dlg::NoticePendingWhy(g_slot, what) == (g_slot.kind != w906dlg::AlarmSlot::kNotice || g_slot.requestId.empty()));   // the slot [B]..[K2] left
        // in source: wb_serve's binding to g_alarmSlot, and ArmCellPopupUpLive asking it after golden's fNote / MyMessageBox flags
        const std::string s = ReadSource("tools/wb_serve.cpp");
        size_t h = s.find("bool W906_NoticeBoxPending(std::string& what)\r\n{");
        if (h == std::string::npos) h = s.find("bool W906_NoticeBoxPending(std::string& what)\n{");
        CHECK(h != std::string::npos && s.find("return w906dlg::NoticePendingWhy(g_alarmSlot, what);", h) != std::string::npos &&
              s.find("return w906dlg::NoticePendingWhy(g_alarmSlot, what);", h) - h < 120);
        const std::string a = ReadSource("ArmCellLive.cpp");
        const size_t f0 = a.find("bool ArmCellPopupUpLive(std::string& what)");
        const size_t f1 = (f0 == std::string::npos) ? f0 : a.find("MyMessageBox->fShow)) { what = ", f0);
        const size_t f2 = (f1 == std::string::npos) ? f1 : a.find("{ if (::W906_NoticeBoxPending(what)) return true; }", f1);
        CHECK(a.find("bool W906_NoticeBoxPending(std::string& what);   namespace ht9045 {") != std::string::npos);   // the global one, declared before the namespace
        const size_t f3 = (f2 == std::string::npos) ? f2 : a.find("what.clear();", f2);
        CHECK(f0 != std::string::npos && f1 != std::string::npos && f2 != std::string::npos && f3 != std::string::npos && f3 - f0 < 1500);
    }

    //AI(W906-S17) 20261003 (St01, laptop card S-17 = INBOX 137, step 2): C++ keeps ONE slot per mailbox, the page QUEUES every
    //  request (dialog-bridge.js queueStop, FIFO, never dropped).  A box C++ has left behind must close on its answer, or the
    //  box C++ waits on now -- queued behind it -- never shows.  B: an id older than the slot's (ids only grow) is superseded
    //  even when the 64-entry list dropped it.  E: the blocking alarm's own pump answers dialog.notifyAck (WaitNotifyAckReply);
    //  it said not-a-notice for every tag, so "notice, then blocking alarm" hung.  A: a modal.answer / dialog.response a wait
    //  does not hold -> "no query pending:superseded-by=<own>" (was modal-pending).  D: Dialog-close-request carries recent[].
    std::printf("[Q] S-17: a box C++ has left behind closes on its answer (B older id, E the alarm pump's notifyAck, A stale answers, D recent closes)\n");
    {
        std::string ack;
        // ---- Q1 (B): more overwrites than the superseded list keeps -> the oldest box still closes
        RecordLike("WAR07324", 2, true);
        StoppedLikeNotice();
        char id[16];
        for (int n = 1000; n < 1070; ++n) { std::snprintf(id, sizeof(id), "%d", n); PostNotice(id, "WAR07324", false); }
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kNotice && g_slot.requestId == "1069");
        CHECK(g_slot.superseded.size() == 64 && !g_slot.WasSuperseded("1000") && !g_slot.WasSuperseded("1004"));   // 1000..1004 fell off the list
        const std::string jq = Slurp(aJson), sq = Slurp(aShim);
        const int jam0 = JamCount();
        ResetSimEventReport();
        CHECK(Ack("1000", &ack, g_dir) == false && ack == "no-pending-notice:superseded-by=1069");   // was request-mismatch:current=1069 -> stuck
        CHECK(Ack("1004", &ack, g_dir) == false && ack == "no-pending-notice:superseded-by=1069");
        CHECK(Ack("1068", &ack, g_dir) == false && ack == "no-pending-notice:superseded-by=1069");   // still in the list: as before
        CHECK(Ack("2000", &ack, g_dir) == false && ack == "request-mismatch:current=1069");          // newer, never issued: unchanged
        {
            w906dlg::WaitTagScope outer("1000");                                                      // an id a wait loop still holds is never stale
            CHECK(Ack("1000", &ack, g_dir) == false && ack == "request-mismatch:current=1069");
        }
        CHECK(Slurp(aJson) == jq && Slurp(aShim) == sq);                                             // nothing written
        CHECK(SoftStop == false && JamCount() == jam0 && g_SimEventReportCount == 0);               // nothing applied
        CHECK(Ack("1069", &ack, g_dir) == true && g_slot.kind == w906dlg::AlarmSlot::kIdle);         // the current one retires as before
        CHECK(w906dlg::WaitingTags().empty());

        // ---- Q2 (E): notice 1100, then blocking alarm 1101; the page acks 1100 while the alarm's pump waits on 1101.
        //   The pump is ForwardShowErrorMessage's wait loop inside wb_serve.exe (static, live socket; no ctest drives it, see
        //   tests/test_yesno_dialog.cpp).  It registers its qid (WaitTagScope) and answers dialog.notifyAck with exactly
        //   w906dlg::WaitNotifyAckReply(g_alarmSlot, tag, qidStr) -- both lines are pinned in Q5 and [M].  Same state here,
        //   built by the same calls in the same order (PostNotice = DialogMailboxPostAlarm + capture; AlarmPost K_RETRY).
        RecordLike("WAR07324", 2, true);
        StoppedLikeNotice();
        PostNotice("1100", "WAR07324", false);
        CHECK(w906dlg::AlarmPost(g_slot, g_dir, g_seq, "1101", "JAM0301", K_RETRY, 3, "", "JAM0301", true));
        {
            w906dlg::WaitTagScope s17Wait("1101");                                                    // the pump's own line
            const std::string je = Slurp(aJson), se = Slurp(aShim);
            CHECK(w906dlg::WaitNotifyAckReply(g_slot, "1100", "1101") == "no-pending-notice:superseded-by=1101");   // box 1100 closes (was not-a-notice)
            CHECK(w906dlg::WaitNotifyAckReply(g_slot, "1101", "1101") == "not-a-notice:current=1101");             // the waiting alarm's own box stays
            CHECK(w906dlg::WaitNotifyAckReply(g_slot, "", "1101") == "not-a-notice:current=1101");
            CHECK(w906dlg::WaitNotifyAckReply(g_slot, "5000", "1101") == "not-a-notice:current=1101");             // never issued
            {
                w906dlg::WaitTagScope inner("1102");                                                  // a nested wait: the outer one is not stale
                CHECK(w906dlg::WaitNotifyAckReply(g_slot, "1101", "1102") == "not-a-notice:current=1102");
            }
            CHECK(g_slot.kind == w906dlg::AlarmSlot::kBlocking && g_slot.requestId == "1101");        // 1101 still pending
            CHECK(Slurp(aJson) == je && Slurp(aShim) == se);                                          // nothing written, nothing retired

            // ---- Q3 (A): answers to boxes no wait holds, while this wait runs
            CHECK(w906dlg::WaitOtherReply("dialog.response", "msg-3", "1101") == "no query pending:superseded-by=1101");   // a replaced / panel-closed modeless box
            CHECK(w906dlg::WaitOtherReply("modal.answer", "1099", "1101") == "no query pending:superseded-by=1101");       // an alarm answered / closed elsewhere
            CHECK(w906dlg::WaitOtherReply("dialog.response", "1101", "1101") == "modal-pending");                          // its own tag (served before this): kept
            CHECK(w906dlg::WaitOtherReply("dialog.response", "", "1101") == "modal-pending");
            CHECK(w906dlg::WaitOtherReply("sys.ping", "msg-3", "1101") == "modal-pending");                               // other commands: unchanged
            {
                w906dlg::WaitTagScope outerMsg("msg-2");                                              // held by another (outer) wait
                CHECK(w906dlg::WaitOtherReply("dialog.response", "msg-2", "1101") == "modal-pending");
            }
            CHECK(w906dlg::WaitOtherReply("dialog.response", "msg-3", "1101").compare(0, 16, "no query pending") == 0);   // what the pages close on
        }
        CHECK(w906dlg::WaitingTags().empty());
        CHECK(w906dlg::AlarmRetire(g_slot, g_dir, g_seq));                                           // the wait loop's own retire

        // ---- Q4 (D): Dialog-close-request keeps the last kCloseRecentMax closes, oldest first; top level = the newest
        {
            std::vector<std::string> recent;
            std::string last;
            for (unsigned long long q = 1; q <= 9; ++q) {
                const std::string top = "{\"schemaVersion\":\"1.0.0\",\"channel\":\"dialog-close\",\"seq\":" + w906dlg::AlarmU64(q) +
                                        ",\"closeRequestId\":\"close-" + w906dlg::AlarmU64(q) + "\",\"state\":\"pending\",\"error\":null}";
                last = w906dlg::CloseRequestWithRecent(top, recent, w906dlg::CloseEntryJson(q, "show-my-message", "msg-" + w906dlg::AlarmU64(q),
                                                                                           100 + q, "SnFKPause", "2026-10-03T23:00:00.000", "PAUSE", 0));
            }
            cJSON* r = cJSON_Parse(last.c_str());
            CHECK(r != 0);
            if (r) {
                cJSON* rc = cJSON_GetObjectItem(r, "recent");
                CHECK(rc != 0 && cJSON_GetArraySize(rc) == 8);
                cJSON* top = cJSON_GetObjectItem(r, "closeRequestId");
                CHECK(top && top->valuestring && std::string(top->valuestring) == "close-9");
                if (rc && cJSON_GetArraySize(rc) == 8) {
                    cJSON* first = cJSON_GetArrayItem(rc, 0);
                    cJSON* newest = cJSON_GetArrayItem(rc, 7);
                    cJSON* fs = first ? cJSON_GetObjectItem(first, "seq") : 0;
                    cJSON* nt = newest ? cJSON_GetObjectItem(newest, "target") : 0;
                    cJSON* nr = nt ? cJSON_GetObjectItem(nt, "requestId") : 0;
                    cJSON* ns = nt ? cJSON_GetObjectItem(nt, "requestSeq") : 0;
                    cJSON* na = newest ? cJSON_GetObjectItem(newest, "resolvedAction") : 0;
                    cJSON* nn = na ? cJSON_GetObjectItem(na, "name") : 0;
                    CHECK(fs && fs->valueint == 2);                                                   // close 1 dropped (8 kept)
                    CHECK(nr && nr->valuestring && std::string(nr->valuestring) == "msg-9" && ns && ns->valueint == 109);
                    CHECK(nn && nn->valuestring && std::string(nn->valuestring) == "PAUSE");
                }
                cJSON_Delete(r);
            }
        }

        // ---- Q5: source pins -- the three waits register their tag and reply through the helpers; the close writer keeps recent
        const std::string s = ReadSource("tools/wb_serve.cpp");
        const size_t fwd = s.find("static int ForwardShowErrorMessage(const char* code, int kcode, int pos)");
        const size_t yn  = s.find("static int ForwardShowMyMessageBoxYesNo(");
        const size_t mbw = s.find("std::string MbWait(const std::string& qid, const char* const* opts, int nOpts)");
        CHECK(fwd != std::string::npos && yn != std::string::npos && mbw != std::string::npos && fwd < yn && yn < mbw);
        if (fwd != std::string::npos && yn != std::string::npos && mbw != std::string::npos) {
            const size_t w0 = s.find("W906ModalWaitScope wakeScope(0, code);  w906dlg::WaitTagScope s17Wait(qidStr);", fwd);
            const size_t w1 = s.find("W906ModalWaitScope wakeScope(1);  w906dlg::WaitTagScope s17Wait(qidStr);", yn);
            const size_t w2 = s.find("W906ModalWaitScope wakeScope(2);  w906dlg::WaitTagScope s17Wait(qid);", mbw);
            CHECK(w0 != std::string::npos && w0 < yn);
            CHECK(w1 != std::string::npos && w1 < mbw);
            CHECK(w2 != std::string::npos && w2 - mbw < 1500);
            const size_t c0 = s.find("if (!W906_MsgBoxModelessAnswer(wc) && !W906_SimDiCommand(wc)) g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "
                                     "w906dlg::WaitOtherReply(wc.cmd, wc.hasTag ? wc.tag : std::string(), qidStr)); }", fwd);
            const size_t c1 = s.find("if (wc.cmd == \"dialog.notifyAck\") g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "
                                     "w906dlg::WaitNotifyAckReply(g_alarmSlot, wc.hasTag ? wc.tag : std::string(), qidStr)); else if (!W906_MsgBoxModelessAnswer(wc)) "
                                     "g_modalServer->CompleteCommand((unsigned long long)wc.id, false, w906dlg::WaitOtherReply(wc.cmd, wc.hasTag ? wc.tag : std::string(), qidStr)); }", yn);
            const size_t c2 = s.find("g_modalServer->CompleteCommand((unsigned long long)wc.id, false, wc.cmd == \"dialog.notifyAck\" ? "
                                     "w906dlg::WaitNotifyAckReply(g_alarmSlot, wc.hasTag ? wc.tag : std::string(), qid) : "
                                     "w906dlg::WaitOtherReply(wc.cmd, wc.hasTag ? wc.tag : std::string(), qid));", mbw);
            CHECK(c0 != std::string::npos && w0 < c0 && c0 < yn);                                   // inside the alarm pump, after its registration
            CHECK(c1 != std::string::npos && w1 < c1 && c1 < mbw);
            CHECK(c2 != std::string::npos && w2 < c2 && c2 < s.find("const std::string io = W906MbIoDismiss();", mbw));
            //AI(W906-MACH0180-F2) 20261005: laptop review F2 -- machine cpp 0190 (199d3534, AI(W906-NOTICE-DEFER)) + 999ac071 (NOTICE-DEFER-2)
            //  put `} else if (wc.cmd == "dialog.notifyAck") {` into MbWait BEFORE its final else, so c2's WaitNotifyAckReply arm is no
            //  longer reached by a notifyAck there (c2 still finds the text -- alone it was a false green).  MbWait HOLDS every notifyAck
            //  (w906NoticeHeld) and gives it to g_carry, at the front, when the wait returns (W906NoticeRelease); the main loop answers it.
            //  These pins assert that, so they go red when it changes (ledger docs/handoff/MACH0180_LEDGER_20261004.md 待決 5: keep the
            //  deferral, or restore S-17's reply -- then the hold branch and these three pins go together).
            const size_t loop = s.find("for (;;) {", mbw);
            const size_t h0 = s.find("std::vector<webbridge::WebCommand> w906NoticeHeld;  struct W906NoticeRelease { std::vector<webbridge::WebCommand>& held; "
                                     "~W906NoticeRelease() { if (!held.empty()) { g_carry.insert(g_carry.begin(), held.begin(), held.end()); g_carryRunnable = true; } } } "
                                     "w906NoticeRelease = { w906NoticeHeld };", mbw);
            const size_t h1 = s.find("} else if (wc.cmd == \"dialog.notifyAck\") {", mbw);
            const size_t h2 = s.find("w906NoticeHeld.push_back(wc);", mbw);
            CHECK(h0 != std::string::npos && w2 < h0 && h0 < loop);                 // the holder lives for the whole wait; released on every return
            CHECK(h1 != std::string::npos && h2 != std::string::npos && loop < h1 && h1 < h2 && h2 < c2);   // every notifyAck is held before the final else
            const size_t rc = s.find("g_carry.push_back(wc)", mbw);
            CHECK(rc == std::string::npos || rc > c2);                              // not 0190's push into g_carry (MbWait took it back every pass and spun)
        }
        CHECK(s.find("static std::vector<std::string> s17Recent;  if (!w906dlg::MailboxPut(g_dialogMailboxDir, \"Dialog-close-request\", "
                     "w906dlg::CloseRequestWithRecent(b, s17Recent, w906dlg::CloseEntryJson(seq, channel, requestId, requestSeq, "
                     "inputName ? inputName : \"\", at, actionName, actionCode))))") != std::string::npos);
    }

    CUSTOMER_CODE = saveCust; LastSet.iRealDummy = saveReal; IniConfig.bEnable_SECS_GEM = saveSecs;
    CosFunction.bIncludeMTBA = saveMtba; bTesterSendPause = saveTsp;
    if (g_fail == 0) RemoveMailbox(g_dir);                                    // kept on a failure, for the diagnosis
    std::printf("\n%d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
