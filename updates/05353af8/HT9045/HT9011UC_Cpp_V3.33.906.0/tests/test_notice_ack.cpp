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
//    [G] restarted since the notice (SystemStart) -> retired, pause "skipped-machine-running": no PAUSE, the records only
//    [H] the motor note (its body applied the answer at post) -> pause "already-applied": SoftStop / SoftStart untouched
//    [I] golden BtnPauseClick's refusal (CC_ASE_SG + bTesterSendPause, note.cpp:3833-3834) -> "golden-refused:..."; still
//        pending; accepted once the refusal is gone
//    [J] no golden note (the record half did not run, note.cpp:541 / :817) -> retired, pause "no-golden-note", no effect
//    [K] a failed retire -> "retire-failed:current=<id>"; the slot stays kNotice and nothing is applied (a retry works)
//    [L] the handler never blocks: every call returns within AtomicWrite's own retry bound (2 s; measured 47-312 ms)
//    [M] source pins: the wb_serve.cpp dispatch, handler, blocking-wait answer and capture; the token exemption
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
bool        W906_NoteNoticeAckLikeGolden(const char* requestId, int* pause, bool* jamCounted, unsigned long* passTimeSec);
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
        CHECK(Ack("16", &ack, g_dir) == false);
        CHECK(ack == "request-mismatch:current=17");
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
        CHECK(Ack("17", &ack, g_dir) == false && ack == "not-a-notice:current=18");
        CHECK(Slurp(aJson) == jb && Slurp(aShim) == sb && !w906dlg::TextSaysIdle(jb));
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kBlocking && g_slot.requestId == "18");
        CHECK(w906dlg::AlarmRetire(g_slot, g_dir, g_seq));                    // the wait loop's own retire (DialogMailboxRetire)
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle);
        CHECK(Ack("18", &ack, g_dir) == false && ack == "no-pending-notice");
    }

    // ---- [G] ------------------------------------------------------------------------------------------------
    std::printf("[G] restarted since the notice: retired, no PAUSE, the records only\n");
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
        CHECK(ack.find("\"pause\":\"skipped-machine-running\"") != std::string::npos);
        CHECK(SoftStop == false && SoftStart == false && SystemStart == true);  // the running machine is not paused
        CHECK(g_SimEventReportCount == 0);                                    // no DoPause report
        CHECK(bOpenAllDoor == false && bInArmNeedToSafePos == true);          // FormClose's flags left alone
        CHECK(JamCount() == jam0 + 1);                                        // the record still counts (golden's START exit closes too)
        CHECK(g_slot.kind == w906dlg::AlarmSlot::kIdle && w906dlg::TextSaysIdle(Slurp(aJson)));
        SystemStart = false;
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
                                  "std::string(\"not-a-notice:current=\") + qidStr); } else if (wc.cmd == \"motor.stop\")");
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

    CUSTOMER_CODE = saveCust; LastSet.iRealDummy = saveReal; IniConfig.bEnable_SECS_GEM = saveSecs;
    CosFunction.bIncludeMTBA = saveMtba; bTesterSendPause = saveTsp;
    if (g_fail == 0) RemoveMailbox(g_dir);                                    // kept on a failure, for the diagnosis
    std::printf("\n%d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
