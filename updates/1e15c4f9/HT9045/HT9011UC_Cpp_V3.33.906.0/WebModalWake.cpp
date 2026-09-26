// =============================================================================
//  WebModalWake.cpp -- see WebModalWake.h
//  AI(W906-MODAL-WAKE) 20260926: RULINGS_20260926 #9 / #26 Q3
// =============================================================================
#include "WebModalWake.h"

#include <cstdio>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

namespace ht9045 {

void ModalWake::Begin(unsigned long long nowMs, int liveWs)
{
    active_     = true;
    launches_   = 0;
    lastLaunch_ = 0;
    noWeb_      = (liveWs <= 0);
    noWebSince_ = nowMs;              // the 30 s count starts no earlier than the box
}

bool ModalWake::Tick(unsigned long long nowMs, int liveWs)
{
    if (!active_) return false;
    if (liveWs > 0) { noWeb_ = false; return false; }          // a page is there: nothing to do
    if (!noWeb_)    { noWeb_ = true; noWebSince_ = nowMs; }     // the page just went away: the 30 s restart
    if (launches_ >= kMaxPerDialog) return false;
    if (nowMs < noWebSince_ || nowMs - noWebSince_ < kFirstMs) return false;
    if (launches_ > 0 && (nowMs < lastLaunch_ || nowMs - lastLaunch_ < kRepeatMs)) return false;
    ++launches_;
    lastLaunch_ = nowMs;
    return true;
}

std::string ModalWakeUrl(int port)
{
    char b[96];
    std::snprintf(b, sizeof(b), "http://127.0.0.1:%d/background.html", port > 0 ? port : 8045);
    return b;
}

bool ModalWakeLaunchEdge(const std::string& url, std::string& why)
{
#ifdef _WIN32
    // HT9045_Web.cmd:41-42: %ProgramFiles(x86)% first, then %ProgramFiles% -- but that cmd runs 64-bit; this exe is 32-bit, and
    // under WOW64 its ProgramFiles IS "Program Files (x86)", so the real "Program Files" is ProgramW6432 (review wf_ea672f01-f4b)
    const char* roots[3] = { std::getenv("ProgramFiles(x86)"), std::getenv("ProgramW6432"), std::getenv("ProgramFiles") };
    std::string edge;
    for (int i = 0; i < 3 && edge.empty(); ++i) {
        if (!roots[i] || !*roots[i]) continue;
        const std::string p = std::string(roots[i]) + "\\Microsoft\\Edge\\Application\\msedge.exe";
        const DWORD a = ::GetFileAttributesA(p.c_str());
        if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY)) edge = p;
    }
    if (edge.empty()) { why = "msedge.exe not found under ProgramFiles(x86) / ProgramFiles"; return false; }
    const char* lad = std::getenv("LOCALAPPDATA");
    const std::string profile = std::string(lad && *lad ? lad : ".") + "\\HT9045_Edge_Web";   // HT9045_Web.cmd:40
    // HT9045_Web.cmd:81-82
    std::string cmd = "\"" + edge + "\" --new-window --window-size=1920,1032 \"" + url +
                      "\" --user-data-dir=\"" + profile + "\" --no-first-run --no-default-browser-check";
    STARTUPINFOA si; ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    PROCESS_INFORMATION pi; ZeroMemory(&pi, sizeof(pi));
    std::string buf = cmd;   // CreateProcessA may write into the command line
    if (!::CreateProcessA(NULL, &buf[0], NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
        char e[64]; std::snprintf(e, sizeof(e), "CreateProcess failed, GetLastError=%lu", (unsigned long)::GetLastError());
        why = e;
        return false;
    }
    ::CloseHandle(pi.hThread);
    ::CloseHandle(pi.hProcess);
    why = cmd;
    return true;
#else
    (void)url;
    why = "not Windows";
    return false;
#endif
}

}  // namespace ht9045
