// =============================================================================
//  test_st02_secsalarm_timer.cpp -- POOL-15 (SECS-S10F3-913): golden 913 TFSECS::TimerSecsAlarmTimer (SECSGEM/UsecegemMainFrom.cpp:
//  1023-1110), ShowSecsAlarmMessage (mymessbox.cpp:1565-1630), TSecsAlarmForm::btnOKClick / DoReleaseAndHide (:1455-1553) and L12
//  (:1415-1416 / :1560-1563) in SECSGEM/SecsAlarmTimer_St02.cpp, plus the four tools/wb_serve.cpp lines that wire the web box.
//
//  AI(W906-POOL15) 20261010 (St02).  Suite name (add_test): St02_SecsAlarmTimer.  argv[1] = port root (source pins, read only).
//  Containment first (st02_test_containment.h) and the four log roots must not be under D:\HT9045* (D:\HT9045_Log included) unless
//  under \obj\v906\.  Memory only: a local HTGem stands in for HSys.MyGem (wb_serve has none today), fake web-host hooks record what
//  would be posted; MyDBIProcess / RecordProcess write only ctest's redirect roots.  Every global touched is restored.
//    [T0]  the entry "FSECS.TimerSecsAlarm" (1000 ms) exists after the host install, on with a host, off with 0, 0, 0
//    [T1]  one queued message -> drained, posted, key lock on, machine stopped, buzzer on
//    [T2]  a MyMessageBox shown -> not drained (golden 913 :1039, RogerYang 0923)
//    [T3]  the window up (ordinary customer) -> the next message stays queued (:1058-1062)
//    [T4]  OK -> released: key lock off, CEID 73, mailbox retired, buzzer off; with SECS off golden keeps the lock (:1528)
//    [T5]  after OK the queued one is shown on the next tick
//    [T6]  bWaitSecsGemReply -> OK refused, window stays (:1457)
//    [T7]  a tag that is not this window's -> 0 (wb_serve's own routing)
//    [T8]  repost: MyMessageBox shown -> wait; mailbox lost -> new requestId; stale requestId refused; no retire when not ours;
//          a failed post is retried
//    [T9]  ALARM_RESET / YES / "" refused -- only OK closes (L12)
//    [T10] MAXIM_THAILAND: a new message hides the window (lock kept, no CEID 73), the next tick shows the new one (:1090-1097)
//    [T11] KYEC_LEE: empty queue -> nothing; iUnLoaderCount != 0 -> drained while up, memo appends, no repeat, 200-line cap
//    [T12] bAlarmAfterPreAlarm -> not drained (:1100)
//    [T13] InitialOK false / HSys.MyGem NULL -> nothing, no crash
//    [T14] source pins: tools/wb_serve.cpp (answer route before the Unloader check, host install / uninstall, the post helper),
//          CMakeLists.txt ht9045_sm line
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "Config.h"
#include "mysensor.h"
#include "mymessbox_shim.h"
#include "BarcodeReader.h"
#include "database.h"
#include "SECSGEM/uHGemClass.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"
#include "SECSGEM/SecsAlarmTimer_St02.h"
#include "TimerTable.h"
#include "st02_test_containment.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
std::string Num(long v) { char b[32]; std::snprintf(b, sizeof b, "%ld", v); return b; }

// ---- the fake web host -------------------------------------------------------------------------------------------
struct Posted { std::string s1, s2, qid; };
std::vector<Posted>      g_posts;
std::vector<std::string> g_retired;
int  g_postN    = 0;
bool g_failPost = false;
bool g_ours     = true;
std::string FakePost(const char* s1, const char* s2)
{
    if (g_failPost) return std::string();
    Posted p;
    p.s1 = s1 ? s1 : "";
    p.s2 = s2 ? s2 : "";
    p.qid = "msg-T" + Num(++g_postN);
    g_posts.push_back(p);
    return p.qid;
}
bool FakeIsOurs(const char*) { return g_ours; }
void FakeRetire(const char* qid) { g_retired.push_back(qid ? qid : ""); }

HTGem* g_gem = 0;
void Tick()
{
    ht9045::TTimerEntry* e = W906_SecsAlarmTimerEntry_St02();
    if (e && e->OnTimer) e->OnTimer();
}
int Queued() { return g_gem ? g_gem->SecsAlarmMessage->Count : -1; }
void Queue(const char* s) { g_gem->SecsAlarmMessage->Add(AnsiString(s)); }
std::string Qid() { return W906_SecsAlarmQid_St02(); }
int Answer(const std::string& tag, const char* action, std::string* reply) { return W906_SecsAlarmAnswer_St02(tag.c_str(), action, reply); }
void Fresh()                                     // every case starts from a closed window, an empty queue and an empty host log
{
    W906_SecsAlarmReset_St02();
    while (g_gem->SecsAlarmMessage->Count) g_gem->SecsAlarmMessage->Delete(0);
    g_posts.clear(); g_retired.clear();
    g_failPost = false; g_ours = true;
    MyMessageBox->fShow = false;
    bSECSGEMAlarm = false; bAlarmAfterPreAlarm = false; bWaitSecsGemReply = false;
    CUSTOMER_CODE = 0; iUnLoaderCount = 0;
    SystemStart = true; SoftStart = true; bAlarmBuzzer = false; bHandlerPause = false; iHandlerStartCount = 5;
    InitialOK = true;
    ResetSimEventReport();
}
std::string State()
{
    return "queued=" + Num(Queued()) + " posts=" + Num((long)g_posts.size()) + " retired=" + Num((long)g_retired.size()) +
           " shown=" + (W906_SecsAlarmShown_St02() ? "1" : "0") + " qid=" + Qid() + " lock=" + (bSECSGEMAlarm ? "1" : "0") +
           " ceid=" + Num((long)g_SimLastEventReportCeid);
}

// ---- source pins ---------------------------------------------------------------------------------------------------
bool ReadLines(const std::string& path, std::vector<std::string>* out)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::string line;
    while (std::getline(f, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        out->push_back(line);
    }
    return true;
}
size_t FindLine(const std::vector<std::string>& L, const std::string& needle, size_t from = 0)
{
    for (size_t i = from; i < L.size(); ++i) if (L[i].find(needle) != std::string::npos) return i;
    return std::string::npos;
}
// the definition line itself (exact): the same text also appears in declarations / calls elsewhere in wb_serve.cpp
size_t FindExact(const std::vector<std::string>& L, const std::string& line)
{
    for (size_t i = 0; i < L.size(); ++i) if (L[i] == line) return i;
    return std::string::npos;
}
// the code part of a line (before the first // that is not inside a string literal)
std::string CodeOf(const std::string& l)
{
    bool inStr = false;
    for (size_t i = 0; i + 1 < l.size(); ++i)
    {
        if (l[i] == '\\' && inStr) { ++i; continue; }
        if (l[i] == '"') inStr = !inStr;
        if (!inStr && l[i] == '/' && l[i + 1] == '/') return l.substr(0, i);
    }
    return l;
}

bool ExtraContainment()
{
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
    for (int i = 0; i < 4; ++i)
    {
        const std::string s = W906TestSafeLower(roots[i]->c_str());
        if (s.find("d:\\ht9045") == 0 && s.find("\\obj\\v906\\") == std::string::npos)
        {
            std::printf("  REFUSED: log root %s is under D:\\HT9045* and not under \\obj\\v906\\\n", roots[i]->c_str());
            return false;
        }
    }
    return true;
}
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_SecsAlarmTimer -- golden 913 TFSECS::TimerSecsAlarmTimer (UsecegemMainFrom.cpp:1023-1110) + the web box (POOL-15)\n");
    if (!W906TestInsideCtestRoots("St02_SecsAlarmTimer") || !ExtraContainment())
        return 2;
    const std::string root = argc > 1 ? argv[1] : ".";

    // saved globals
    HTGem* const sGem = HSys.MyGem;
    const bool sInit = InitialOK, sSysInit = SystemInitialOK, sStart = SystemStart, sSoft = SoftStart, sLock = bSECSGEMAlarm;
    const bool sPre = bAlarmAfterPreAlarm, sWait = bWaitSecsGemReply, sSecs = IniConfig.bEnable_SECS_GEM, sMb = MyMessageBox->fShow;
    const bool sBuzz = bAlarmBuzzer, sPause = bHandlerPause, sFall = bIsTestSitICFallDown, sDoor3 = Sen[SnSafeDoor3].Enable;
    const int  sCc = CUSTOMER_CODE, sUnl = iUnLoaderCount, sHsc = iHandlerStartCount;

    HTGem gem;
    g_gem = &gem;
    HSys.MyGem = &gem;
    SystemInitialOK = false;                      // ShowSecsAlarmMessage then skips StopAllMotor (golden :1579 guard)
    IniConfig.bEnable_SECS_GEM = true;
    Sen[SnSafeDoor3].Enable = false;
    if (FormBarcodeReader == 0) FormBarcodeReader = new TFormBarcodeReader();   // the KYEC arm reads it (golden :1081); wb_serve builds it at boot
    FormBarcodeReader->bShow = false;

    // ---------------------------------------------------------------- [T0]
    std::printf("[T0] the timer table entry\n");
    W906_SecsAlarmHostInstall_St02(&FakePost, &FakeIsOurs, &FakeRetire);
    ht9045::TTimerEntry* const e = W906_SecsAlarmTimerEntry_St02();
    Check(e != 0 && std::strcmp(e->Name, "FSECS.TimerSecsAlarm") == 0 && (int)e->Interval == 1000 && (bool)e->Enabled && e->OnTimer != 0,
          "[T0] host installed -> \"FSECS.TimerSecsAlarm\", 1000 ms, on, OnTimer set (golden 913 UsecegemMainFrom.dfm:10977-10982, FormCreate :150)");
    W906_SecsAlarmHostInstall_St02(0, 0, 0);
    Check(e != 0 && !(bool)e->Enabled && W906_SecsAlarmTimerEntry_St02() == e, "[T0] 0, 0, 0 -> the same entry, off (FormDestroy :967)");
    W906_SecsAlarmHostInstall_St02(&FakePost, &FakeIsOurs, &FakeRetire);
    Check(e != 0 && (bool)e->Enabled && W906_SecsAlarmTimerEntry_St02() == e, "[T0] installed again -> the same entry, on");

    // ---------------------------------------------------------------- [T1]
    std::printf("[T1] one queued host message\n");
    Fresh();
    Queue("HELLO");
    Tick();
    Check(Queued() == 0 && g_posts.size() == 1 && g_posts[0].s1 == "HELLO" && g_posts[0].s2 == "" && W906_SecsAlarmShown_St02() &&
          Qid() == g_posts[0].qid && bSECSGEMAlarm, "[T1] drained, posted once (s1=HELLO), shown, key lock on (" + State() + ")");
    Check(!SystemStart && !SoftStart && bHandlerPause && iHandlerStartCount == 0 && bAlarmBuzzer,
          "[T1] machine state on show: SystemStart / SoftStart off, bHandlerPause, iHandlerStartCount 0, buzzer on (golden 913 mymessbox.cpp:1570-1588)");

    // ---------------------------------------------------------------- [T2]
    std::printf("[T2] a MyMessageBox is shown\n");
    Fresh();
    MyMessageBox->fShow = true;
    Queue("WAIT FOR THE BOX");
    Tick();
    Check(Queued() == 1 && g_posts.empty() && !W906_SecsAlarmShown_St02() && !bSECSGEMAlarm,
          "[T2] not drained while MyMessageBox->fShow (golden 913 :1039-1043) (" + State() + ")");
    MyMessageBox->fShow = false;
    Tick();
    Check(Queued() == 0 && g_posts.size() == 1, "[T2] drained once the box is gone (" + State() + ")");

    // ---------------------------------------------------------------- [T3] [T4] [T5]
    std::printf("[T3] the window is up (ordinary customer)\n");
    Fresh();
    Queue("FIRST");
    Tick();
    Queue("SECOND");
    Tick();
    Check(Queued() == 1 && g_posts.size() == 1 && W906_SecsAlarmText_St02() == "FIRST",
          "[T3] the second stays queued while the first is up (golden 913 :1058-1062) (" + State() + ")");

    std::printf("[T4] OK releases the window\n");
    const std::string q1 = Qid();
    std::string reply = "x";
    bIsTestSitICFallDown = true;
    const int k4 = Answer(q1, "OK", &reply);
    Check(k4 == 1 && reply.empty() && !bSECSGEMAlarm && g_SimLastEventReportCeid == (unsigned)SECS_EVENT.MymessboxOK &&
          g_retired.size() == 1 && g_retired[0] == q1 && !bAlarmBuzzer && !W906_SecsAlarmShown_St02() && Qid().empty(),
          "[T4] OK -> 1, key lock off, CEID 73, mailbox retired once, buzzer off, window closed (golden 913 :1504 / :1528-1551) (" + State() + ")");
    Check(!bIsTestSitICFallDown, "[T4] FormClose part: safe door 3 not enabled -> bIsTestSitICFallDown cleared (golden 913 :1514-1521)");

    std::printf("[T5] the queued one comes next\n");
    Tick();
    Check(Queued() == 0 && g_posts.size() == 2 && g_posts[1].s1 == "SECOND" && W906_SecsAlarmShown_St02() && Qid() == g_posts[1].qid && bSECSGEMAlarm,
          "[T5] next tick: SECOND drained and shown with a new requestId (" + State() + ")");

    std::printf("[T4b] SECS off at OK: golden keeps the key lock\n");
    IniConfig.bEnable_SECS_GEM = false;
    ResetSimEventReport();
    const int k4b = Answer(Qid(), "OK", &reply);
    Check(k4b == 1 && bSECSGEMAlarm && g_SimEventReportCount == 0 && !W906_SecsAlarmShown_St02(),
          "[T4b] bEnable_SECS_GEM false -> released but bSECSGEMAlarm kept, no EventReport (golden 913 :1528 guard) (" + State() + ")");
    IniConfig.bEnable_SECS_GEM = true;

    // ---------------------------------------------------------------- [T6]
    std::printf("[T6] waiting for EAP\n");
    Fresh();
    Queue("EAP");
    Tick();
    bWaitSecsGemReply = true;
    const int k6 = Answer(Qid(), "OK", &reply);
    Check(k6 == -1 && reply.find("bWaitSecsGemReply") != std::string::npos && bSECSGEMAlarm && W906_SecsAlarmShown_St02() && g_retired.empty(),
          "[T6] bWaitSecsGemReply -> refused, window and lock stay (golden 913 :1457) (" + State() + " reply=" + reply + ")");
    bWaitSecsGemReply = false;

    // ---------------------------------------------------------------- [T7]
    std::printf("[T7] somebody else's request\n");
    const int k7a = Answer("msg-999", "OK", &reply);
    const int k7b = Answer("", "OK", &reply);
    const int k7c = Answer("1234", "PAUSE", &reply);
    Check(k7a == 0 && k7b == 0 && k7c == 0 && W906_SecsAlarmShown_St02() && bSECSGEMAlarm,
          "[T7] unknown tags -> 0 (wb_serve's own routing), window untouched (" + State() + ")");

    // ---------------------------------------------------------------- [T8]
    std::printf("[T8] repost when the mailbox lost the box\n");
    const std::string q8 = Qid();
    g_ours = false;
    MyMessageBox->fShow = true;
    Tick();
    Check(g_posts.size() == 1 && Qid() == q8, "[T8a] a MyMessageBox owns the mailbox -> no repost yet (" + State() + ")");
    MyMessageBox->fShow = false;
    Tick();
    Check(g_posts.size() == 2 && Qid() == g_posts[1].qid && Qid() != q8 && g_posts[1].s1 == "EAP" && W906_SecsAlarmShown_St02(),
          "[T8b] box gone, mailbox not ours -> posted again with a new requestId, same text (" + State() + ")");
    const int k8 = Answer(q8, "OK", &reply);
    Check(k8 == -1 && reply.find("no query pending") != std::string::npos && W906_SecsAlarmShown_St02() && bSECSGEMAlarm,
          "[T8c] the stale requestId -> refused with \"no query pending\" (the page drops it), window stays (reply=" + reply + ")");
    g_failPost = true;
    g_ours = false;
    Tick();
    Check(g_posts.size() == 2 && Qid() == g_posts[1].qid, "[T8d] a failed post keeps the last requestId (" + State() + ")");
    g_failPost = false;
    Tick();
    Check(g_posts.size() == 3 && Qid() == g_posts[2].qid, "[T8d] ... and is retried on the next tick (" + State() + ")");
    g_ours = false;
    const int k8e = Answer(Qid(), "OK", &reply);
    Check(k8e == 1 && g_retired.empty() && !bSECSGEMAlarm, "[T8e] OK while the mailbox holds another box -> released, mailbox NOT retired (" + State() + ")");

    // ---------------------------------------------------------------- [T9]
    std::printf("[T9] only OK closes the window (L12)\n");
    Fresh();
    Queue("L12");
    Tick();
    const int k9a = Answer(Qid(), "ALARM_RESET", &reply);
    const int k9b = Answer(Qid(), "YES", &reply);
    const int k9c = Answer(Qid(), "", &reply);
    Check(k9a == -1 && k9b == -1 && k9c == -1 && W906_SecsAlarmShown_St02() && bSECSGEMAlarm && g_retired.empty(),
          "[T9] ALARM_RESET / YES / \"\" -> refused, window and lock stay (golden 913 :1415-1416 / :1560-1563) (" + State() + ")");
    const int k9d = Answer(Qid(), "PAUSE", &reply);
    Check(k9d == 1 && !bSECSGEMAlarm, "[T9] PAUSE (the page's pnlPause) releases like OK");

    // ---------------------------------------------------------------- [T10]
    std::printf("[T10] MAXIM_THAILAND\n");
    Fresh();
    CUSTOMER_CODE = CC_MAXIM_THAILAND;
    Queue("MAXIM 1");
    Tick();
    const std::string q10 = Qid();
    Queue("MAXIM 2");
    ResetSimEventReport();
    Tick();
    Check(!W906_SecsAlarmShown_St02() && Queued() == 1 && bSECSGEMAlarm && g_SimEventReportCount == 0 && g_retired.size() == 1 && g_retired[0] == q10,
          "[T10] a new message while up -> hidden (not released: lock kept, no CEID), mailbox retired (golden 913 :1090-1097) (" + State() + ")");
    Tick();
    Check(W906_SecsAlarmShown_St02() && Queued() == 0 && g_posts.size() == 2 && g_posts[1].s1 == "MAXIM 2",
          "[T10] next tick -> the new message shown fresh (" + State() + ")");

    // ---------------------------------------------------------------- [T11]
    std::printf("[T11] KYEC_LEE\n");
    Fresh();
    CUSTOMER_CODE = CC_KYEC_LEE;
    Tick();
    Check(g_posts.empty() && !W906_SecsAlarmShown_St02(), "[T11] empty queue -> nothing (golden 913 :1065-1072)");
    FormBarcodeReader->bShow = true;
    Queue("K0");
    Tick();
    Check(!FormBarcodeReader->bShow && Queued() == 1 && g_posts.empty(), "[T11] the barcode box open -> closed first, nothing shown this tick (golden 913 :1081-1086)");
    Tick();
    Check(Queued() == 0 && W906_SecsAlarmShown_St02() && W906_SecsAlarmLineCount_St02() == 1, "[T11] next tick -> K0 shown (" + State() + ")");
    Queue("K1");
    Tick();
    Check(Queued() == 1 && W906_SecsAlarmLineCount_St02() == 1, "[T11] window up and iUnLoaderCount == 0 -> K1 stays queued (:1052-1056)");
    iUnLoaderCount = 1;
    Tick();
    Check(Queued() == 0 && W906_SecsAlarmLineCount_St02() == 2 && W906_SecsAlarmText_St02() == "K0\r\nK1" && g_posts.size() == 2 &&
          g_posts[1].s1 == "K0\r\nK1", "[T11] iUnLoaderCount != 0 -> drained while up, memo appended, posted again (BringToFront) (" + State() + ")");
    Queue("K1");
    Tick();
    Check(W906_SecsAlarmLineCount_St02() == 2, "[T11] the same line twice in a row is not added again (golden 913 :1614-1615)");
    for (int i = 0; i < 205; ++i) { char b[16]; std::snprintf(b, sizeof b, "L%03d", i); Queue(b); }
    for (int i = 0; i < 205; ++i) Tick();
    const std::string memo = W906_SecsAlarmText_St02();
    Check(Queued() == 0 && W906_SecsAlarmLineCount_St02() == 200 && memo.find("L204") != std::string::npos && memo.find("K0") == std::string::npos,
          "[T11] 200-line cap, oldest dropped (golden 913 :1617-1618) (lines=" + Num(W906_SecsAlarmLineCount_St02()) + ")");

    // ---------------------------------------------------------------- [T12]
    std::printf("[T12] PreAlarm -> Alarm in progress\n");
    Fresh();
    bAlarmAfterPreAlarm = true;
    Queue("PRE");
    Tick();
    Check(Queued() == 1 && g_posts.empty() && !bSECSGEMAlarm, "[T12] bAlarmAfterPreAlarm -> not drained (golden 913 :1100) (" + State() + ")");

    // ---------------------------------------------------------------- [T13]
    std::printf("[T13] InitialOK false / no HT9045Gem\n");
    Fresh();
    InitialOK = false;
    Queue("EARLY");
    Tick();
    Check(Queued() == 1 && g_posts.empty(), "[T13] InitialOK false -> nothing (golden 913 :1028-1031)");
    InitialOK = true;
    HSys.MyGem = 0;
    Tick();
    HSys.MyGem = &gem;
    Check(Queued() == 1 && g_posts.empty() && !bSECSGEMAlarm, "[T13] HSys.MyGem NULL -> nothing, no crash ([W906] P1: wb_serve has no HT9045Gem today)");

    // ---------------------------------------------------------------- [T14]
    std::printf("[T14] source pins under %s (read only)\n", root.c_str());
    std::vector<std::string> wb, cm;
    const bool okWb = ReadLines(root + "\\tools\\wb_serve.cpp", &wb);
    const bool okCm = ReadLines(root + "\\CMakeLists.txt", &cm);
    Check(okWb && okCm, "[T14] tools/wb_serve.cpp and CMakeLists.txt read");
    const size_t a0 = FindExact(wb, "bool W906_MsgBoxModelessAnswer(const webbridge::WebCommand& wc)");
    const size_t aR = a0 == std::string::npos ? a0 : FindLine(wb, "W906_SecsAlarmAnswer_St02(wc.tag.c_str(), MbAnswerOf(wc).c_str(), &r)", a0);
    const size_t aU = a0 == std::string::npos ? a0 : FindLine(wb, "if (s_mbModeless.qid.empty()", a0);
    Check(aR != std::string::npos && aU != std::string::npos && aR < aU && CodeOf(wb[aR]).find("W906_SecsAlarmAnswer_St02(") != std::string::npos,
          "[T14] W906_MsgBoxModelessAnswer asks W906_SecsAlarmAnswer_St02 (in code, not a comment) before the Unloader box's check");
    const size_t i0 = FindExact(wb, "void W906_MsgBoxHostInstall()");
    const size_t iI = i0 == std::string::npos ? i0 : FindLine(wb, "W906_SecsAlarmHostInstall_St02(&W906MbSecsPostSt02, &W906MbSecsIsOursSt02, &W906MbSecsRetireSt02)", i0);
    const size_t u0 = FindExact(wb, "void W906_MsgBoxHostUninstall()");
    const size_t uU = u0 == std::string::npos ? u0 : FindLine(wb, "W906_SecsAlarmHostInstall_St02(0, 0, 0)", u0);
    Check(iI != std::string::npos && iI < i0 + 5 && CodeOf(wb[iI]).find("W906_SecsAlarmHostInstall_St02(") != std::string::npos &&
          uU != std::string::npos && uU < u0 + 5 && CodeOf(wb[uU]).find("W906_SecsAlarmHostInstall_St02(") != std::string::npos,
          "[T14] W906_MsgBoxHostInstall installs the SECS host, W906_MsgBoxHostUninstall passes 0, 0, 0 (both in code)");
    const size_t p0 = FindLine(wb, "static std::string W906MbSecsPostSt02(const char* s1, const char* s2)");
    const std::string pc = p0 == std::string::npos ? std::string() : CodeOf(wb[p0]);
    Check(pc.find("v.function = \"ShowSecsAlarmMessage\"") != std::string::npos && pc.find("v.blocking = false") != std::string::npos &&
          pc.find("v.stopAllMotor = true") != std::string::npos && pc.find("v.pause.caption = \"OK\"") != std::string::npos &&
          pc.find("v.alarmReset.visible = true") != std::string::npos && pc.find("MbPost(v, qid)") != std::string::npos,
          "[T14] the post helper: ShowSecsAlarmMessage, non-blocking, machine-stopping, OK + Alarm Reset, through MbPost");
    const size_t mp = FindLine(wb, "MbEsc(qid) + \"\\\",\\\"state\\\":\\\"pending\\\"");
    Check(mp != std::string::npos && pc.find("\"\\\",\\\"state\\\":\\\"pending\\\"\"") != std::string::npos,
          "[T14] isOurs looks for MbPost's own \"requestId\":\"<qid>\",\"state\":\"pending\" text");
    size_t cL = std::string::npos;
    for (size_t i = 0; i < cm.size(); ++i)
    {
        const std::string code = cm[i].substr(0, cm[i].find('#'));
        const size_t n = code.find("SECSGEM/N07Alarm_St02.cpp"), s = code.find("SECSGEM/SecsAlarmTimer_St02.cpp");
        if (n != std::string::npos && s != std::string::npos && s > n) { cL = i; break; }
    }
    size_t lib = std::string::npos;
    for (size_t i = cL; cL != std::string::npos && i != (size_t)-1; --i)
        if (cm[i].compare(0, 12, "add_library(") == 0) { lib = i; break; }
    Check(cL != std::string::npos && lib != std::string::npos && cm[lib].find("add_library(ht9045_sm") == 0,
          "[T14] CMakeLists.txt: SECSGEM/SecsAlarmTimer_St02.cpp after SECSGEM/N07Alarm_St02.cpp on the same line, before '#', in ht9045_sm");

    // restore
    W906_SecsAlarmReset_St02();
    W906_SecsAlarmHostInstall_St02(0, 0, 0);
    while (gem.SecsAlarmMessage->Count) gem.SecsAlarmMessage->Delete(0);
    HSys.MyGem = sGem;
    g_gem = 0;
    InitialOK = sInit; SystemInitialOK = sSysInit; SystemStart = sStart; SoftStart = sSoft; bSECSGEMAlarm = sLock;
    bAlarmAfterPreAlarm = sPre; bWaitSecsGemReply = sWait; IniConfig.bEnable_SECS_GEM = sSecs; MyMessageBox->fShow = sMb;
    bAlarmBuzzer = sBuzz; bHandlerPause = sPause; bIsTestSitICFallDown = sFall; Sen[SnSafeDoor3].Enable = sDoor3;
    CUSTOMER_CODE = sCc; iUnLoaderCount = sUnl; iHandlerStartCount = sHsc;
    FormBarcodeReader->bShow = false;

    std::printf("\n%s St02_SecsAlarmTimer: %d passed, %d failed\n", g_fail ? "FAILED" : "ALL PASS", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
