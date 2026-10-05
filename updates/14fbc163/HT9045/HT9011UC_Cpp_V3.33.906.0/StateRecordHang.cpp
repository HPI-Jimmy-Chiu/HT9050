// =============================================================================
//  StateRecordHang.cpp  --  State Record: the hang route.  NOT golden.
//
//  AI(W906-S24-HANG) 20261004 (St02-E helper), card S-24 claim ② (laptop OK on handoff 587a830c).
//
//  WHY.  The only way to a State Record is WS act.main.stateRecord, which the TICK thread drains (tools/wb_serve.cpp
//  :4672 -> JsonBridge/ChanAction.cpp -> MainStateRecord.cpp -> cStateRecord.cpp).  When the tick loop is stuck the
//  command waits in the queue, the page says "15 s no answer" and nothing is written (S24_GAP_ANALYSIS.md section 4).
//  This route is served on the SOCKET thread (GET /api/struct/staterecord.hang, tools/wb_serve.cpp:2522) and starts a
//  worker that writes <root><yyyymmdd_hhmmss>_hang\ with the nine W906_* files (+ W906_Mailbox\) from the snapshot the
//  tick thread published LAST and from disk files only -- it never touches a machine global (StateRecordDiag.h, THREADS).
//
//  NOT here: golden's DoStateRecord (Task_ListWithTime.csv, DecisionVariables.csv, ... read tick-owned state), the golden
//  log copies and the 7z zip (the job lives in cStateRecord.cpp's anonymous namespace and starts from the tick).  The
//  folder stays a plain folder.
//  AI(W906-S24-Q92) 20261004 (St02-E; Steven Q92 = A, 07:2x): the AUTOMATIC record (end of this file) -- one per tick-loop stall of >= 60 s,
//  through the same route (StateRecordDiag.h, W906_StateRecordAutoMonitorStart).
//
//  One record at a time (busy guard), at most one per minGapMs (default 5 s) so a page cannot flood the disk.
// =============================================================================
#include "StateRecordDiag.h"

#include <windows.h>
#include <cstdio>
#include <string>

namespace {

volatile LONG g_hangBusy = 0;
// touched only while g_hangBusy is held by the requester
DWORD g_hangLastTick = 0;
bool  g_hangHaveLast = false;

struct HangJob
{
    std::string   folder, trigger;
    int           files;
    std::string   why;
    volatile LONG refs;          // the requester and the worker; the last one out deletes
    HANDLE        done;          // manual-reset, set when the worker finished
    HangJob() : files(0), refs(2), done(0) {}
};

void Release(HangJob* j)
{
    if (::InterlockedDecrement(&j->refs) == 0) {
        if (j->done) ::CloseHandle(j->done);
        delete j;
    }
}

DWORD WINAPI HangWorker(LPVOID p)
{
    HangJob* j = static_cast<HangJob*>(p);
    W906SrDiagWriteOptions o;
    o.trigger = j->trigger;
    try {
        j->files = W906_StateRecordDiagWriteFilesEx(j->folder, o, &j->why);
    } catch (...) {
        j->files = -2;
        j->why = "the writer threw";
    }
    ::InterlockedExchange(&g_hangBusy, 0);   // before the event: a caller that saw "done" may start the next one at once
    ::SetEvent(j->done);
    Release(j);
    return 0;
}

std::string Num(unsigned long long v)
{
    char b[24];
    int i = 23;
    b[i] = '\0';
    do {
        b[--i] = (char)('0' + (int)(v % 10u));
        v /= 10u;
    } while (v != 0 && i > 0);
    return std::string(b + i);
}

std::string SNum(long long v)
{
    return v < 0 ? "-" + Num((unsigned long long)(-(v + 1)) + 1u) : Num((unsigned long long)v);
}

std::string JStr(const std::string& s)
{
    const std::string t = W906SrDiagSafe(s);    // no control characters, valid UTF-8
    std::string o = "\"";
    for (std::size_t i = 0; i < t.size(); ++i) {
        if (t[i] == '"' || t[i] == '\\') o += '\\';
        o += t[i];
    }
    o += '"';
    return o;
}

std::string Refuse(const char* guard, const std::string& detail)
{
    return std::string("{\"ok\":false,\"guard\":") + JStr(guard) + ",\"detail\":" + JStr(detail) + "}";
}

// ?trigger=<a-z A-Z 0-9 . _ -, at most 40>; anything else is dropped
std::string TriggerFromQuery(const std::string& q)
{
    std::string t;
    const std::string::size_type p = q.find("trigger=");
    if (p != std::string::npos) {
        for (std::size_t i = p + 8; i < q.size() && q[i] != '&' && t.size() < 40; ++i) {
            const char ch = q[i];
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '.' || ch == '_' ||
                ch == '-')
                t += ch;
        }
    }
    return t.empty() ? std::string("http") : t;
}

}  // namespace

// AI(W906-S24-Q92) 20261004 (St02-E; Steven Q92 = A, 07:2x): the automatic record's state -- touched only by the monitor thread (or a test
// calling the step function with the monitor not started).
namespace {
volatile LONG g_autoStarted = 0;
bool          g_autoDone = false;      // this stall (g_autoForPass) already has its record
DWORD         g_autoForPass = 0;
volatile LONG g_autoCount = 0;

bool IsDialogWait(const std::string& phase)   // tools/wb_serve.cpp WdMark2 at the three blocking waits (:534 / :803 / :6757)
{
    return phase.compare(0, 10, "modal wait") == 0 || phase.compare(0, 12, "yes/no wait ") == 0;
}

DWORD WINAPI AutoMonitor(LPVOID)
{
    for (;;) {
        ::Sleep(1000);
        const W906SrDiagModuleInfo m = W906_StateRecordDiagInfo();
        const W906SrDiagWatchdogInfo wd = W906_StateRecordDiagWatchdog();
        W906_StateRecordAutoStallStepAt("D:\\HT9045_StateRecord\\", ::GetTickCount(), m.havePass, m.lastPassTick,
                                        wd.ok ? wd.phase : std::string(), 60000u, 0u, 0);
    }
}
}  // namespace

void W906_StateRecordAutoMonitorStart()
{
    if (!kW906StateRecordAutoOnHang || ::InterlockedCompareExchange(&g_autoStarted, 1, 0) != 0)
        return;
    DWORD tid = 0;
    HANDLE h = ::CreateThread(NULL, 0, &AutoMonitor, NULL, 0, &tid);   // Win32, as HangWorker; runs until the process ends
    if (h != 0) ::CloseHandle(h);
    std::printf("[STATEREC] automatic hang record on: one per tick-loop stall of >= 60 s (Q92 = A) -> D:\\HT9045_StateRecord\\<time>_hang\n");
}

bool W906_StateRecordAutoStallStepAt(const std::string& root, unsigned long nowTick, bool havePass, unsigned long lastPassTick,
                                     const std::string& wdPhase, unsigned int thresholdMs, unsigned int waitMs,
                                     std::string* folderOut)
{
    if (!havePass)                                                     // nothing published yet: no tick loop to call stuck
        return false;
    if ((DWORD)((DWORD)nowTick - (DWORD)lastPassTick) < (DWORD)thresholdMs)
        return false;
    if (IsDialogWait(wdPhase))                                         // a blocking box waiting for the operator is not a stall
        return false;
    if (g_autoDone && g_autoForPass == (DWORD)lastPassTick)            // one record per stall
        return false;
    std::string folder;
    const std::string js = W906_StateRecordHangRequestAt(root, "auto-stall", 0u, waitMs, &folder);
    if (js.find("\"started\":true") == std::string::npos)              // busy (a manual hang record): try again next step
        return false;
    g_autoDone = true;
    g_autoForPass = (DWORD)lastPassTick;
    ::InterlockedIncrement(&g_autoCount);
    if (folderOut) *folderOut = folder;
    std::printf("[STATEREC] tick loop silent for %lu ms (watchdog: %s) -> automatic hang record %s\n",
                (unsigned long)((DWORD)nowTick - (DWORD)lastPassTick), wdPhase.empty() ? "?" : wdPhase.c_str(), folder.c_str());
    std::fflush(stdout);
    return true;
}

unsigned long W906_StateRecordAutoCount() { return (unsigned long)g_autoCount; }

// AI(W906-S24-Q93) 20261004 (St02-E; Steven Q93 = A, St01 OK 11:27): exactly one command is served inside a blocking box (StateRecordDiag.h)
bool W906_StateRecordDialogTakes(const std::string& cmd) { return cmd == "act.main.stateRecord"; }
void W906_StateRecordAutoTestReset() { g_autoDone = false; g_autoForPass = 0; ::InterlockedExchange(&g_autoCount, 0); }

std::string W906_StateRecordHangRequest(const std::string& query)
{
    return W906_StateRecordHangRequestAt("D:\\HT9045_StateRecord\\", TriggerFromQuery(query), 5000u, 1500u, 0);
}

std::string W906_StateRecordHangRequestAt(const std::string& root, const std::string& trigger, unsigned int minGapMs,
                                          unsigned int waitMs, std::string* folderOut)
{
    if (::InterlockedCompareExchange(&g_hangBusy, 1, 0) != 0)
        return Refuse("busy", "another hang record is being written");
    const DWORD now = ::GetTickCount();
    if (g_hangHaveLast && (DWORD)(now - g_hangLastTick) < (DWORD)minGapMs) {
        ::InterlockedExchange(&g_hangBusy, 0);
        return Refuse("too-soon", "one hang record per " + Num(minGapMs) + " ms");
    }
    g_hangHaveLast = true;
    g_hangLastTick = now;

    SYSTEMTIME lt;
    ::GetLocalTime(&lt);
    char name[48];
    std::snprintf(name, sizeof(name), "%04u%02u%02u_%02u%02u%02u_hang", (unsigned)lt.wYear, (unsigned)lt.wMonth,
                  (unsigned)lt.wDay, (unsigned)lt.wHour, (unsigned)lt.wMinute, (unsigned)lt.wSecond);
    std::string base = root;
    if (!base.empty() && base[base.size() - 1] != '\\' && base[base.size() - 1] != '/') base += "\\";
    std::string folder = base + name;
    for (int k = 2; k < 100 && ::GetFileAttributesA(folder.c_str()) != INVALID_FILE_ATTRIBUTES; ++k)
        folder = base + name + "_" + Num((unsigned long long)k);
    if (folderOut) *folderOut = folder;

    HangJob* j = new HangJob;
    j->folder = folder;
    j->trigger = "hang-route:" + trigger;
    j->done = ::CreateEventA(NULL, TRUE, FALSE, NULL);
    if (j->done == 0) {
        delete j;
        ::InterlockedExchange(&g_hangBusy, 0);
        return Refuse("event", "CreateEvent failed");
    }
    DWORD tid = 0;
    HANDLE h = ::CreateThread(NULL, 0, &HangWorker, j, 0, &tid);   // Win32, not std::thread: MinGW 6.3 win32 thread model
    if (h == 0) {
        ::CloseHandle(j->done);
        delete j;
        ::InterlockedExchange(&g_hangBusy, 0);
        return Refuse("thread", "CreateThread failed");
    }
    ::CloseHandle(h);                                              // detached; it releases the busy guard itself
    const bool finished = (::WaitForSingleObject(j->done, waitMs) == WAIT_OBJECT_0);
    const int files = finished ? j->files : 0;
    const std::string why = finished ? j->why : std::string();
    Release(j);

    const W906SrDiagModuleInfo m = W906_StateRecordDiagInfo();
    const W906SrDiagSnapshot s = W906_StateRecordDiagRead();
    const DWORD nowTick = ::GetTickCount();
    const bool ok = !finished || files == kW906SrDiagFileCount;   // AI(W906-S24-S2) 20261005 (St02-E): was 7 (+ W906_IO.csv, W906_Recent.csv)
    std::string o = "{\"ok\":";
    o += ok ? "true" : "false";
    o += ",\"guard\":" + JStr(ok ? "" : "write-failed");
    o += ",\"folder\":" + JStr(folder);
    o += ",\"started\":true,\"finished\":";
    o += finished ? "true" : "false";
    o += ",\"files\":" + SNum(files);
    o += ",\"snapshotSeq\":" + Num(s.seq);
    o += ",\"snapshotAgeMs\":" + (s.seq ? Num((unsigned long)(nowTick - s.builtTick)) : std::string("null"));
    o += ",\"lastPassAgeMs\":" + (m.havePass ? Num((unsigned long)(nowTick - m.lastPassTick)) : std::string("null"));
    o += ",\"tickThread\":" + Num(m.tickThread);
    o += ",\"requestThread\":" + Num(::GetCurrentThreadId());
    o += ",\"detail\":" + JStr(finished ? (files == kW906SrDiagFileCount ? std::string("W906_* written from the last published snapshot") : why)
                                        : std::string("the worker is still writing; the folder fills in the background"));
    o += ",\"note\":" + JStr("port-only (not golden): no golden State Record files, no log copies, no zip -- those need the "
                             "tick loop");
    o += "}";
    return o;
}
