// =============================================================================
//  WebModalWake.h -- a blocking dialog is waiting and no web page is connected:
//                    open the browser (RULINGS_20260926 #9, details #26 Q3)
//
//  AI(W906-MODAL-WAKE) 20260926
//
//  WHY
//    golden shows its dialogs (TfNote alarm box, TMyMessageBox YES/NO and ShowMyMessage)
//    on the machine's own screen with ShowModal: the operator cannot miss them.  In the
//    port the box is in the browser; with no page open wb_serve waits forever and nobody
//    sees anything.  User ruling #9 (3A+B): keep waiting like golden, open the browser
//    when there is no connection, and remind with the tower light / buzzer like an alarm.
//    #26 Q3 fixed the numbers: first launch after 30 s with no web page, then at most one
//    launch per 2 minutes, at most 3 launches per dialog, open the release screen.
//
//  WHAT THIS FILE IS
//    The decision only (pure, no clock, no process, no socket), so ctest can drive it
//    with hand-made times.  tools/wb_serve.cpp owns the clock (GetTickCount64), the live
//    WebSocket count (WebBridgeServer::LiveWebSocketCount) and the launcher
//    (ModalWakeLaunchEdge below).
//
//  RULES (each one is a check in tests/test_modal_wake.cpp)
//    * "no web page" = zero live WebSocket connections (HTTP requests do not count).
//    * the 30 s window restarts every time a page connects and goes away again: it is
//      30 s of CONTINUOUS absence, not 30 s since the box appeared.
//    * after a launch the next one needs another 2 minutes (the browser may take a few
//      seconds to connect; a launch that never connects is retried at most twice).
//    * at most 3 launches per dialog; the count restarts with the next dialog.
//    * nothing happens between End() and the next Begin().
// =============================================================================
#ifndef W906_WEB_MODAL_WAKE_H
#define W906_WEB_MODAL_WAKE_H

#include <string>

namespace ht9045 {

class ModalWake {
public:
    static const unsigned long long kFirstMs  = 30000ULL;    // #26 Q3: 30 s with no web page
    static const unsigned long long kRepeatMs = 120000ULL;   // #26 Q3: then at most once per 2 minutes
    static const int                kMaxPerDialog = 3;       // #26 Q3: at most 3 per dialog

    ModalWake() : active_(false), noWeb_(false), noWebSince_(0), lastLaunch_(0), launches_(0) {}

    // a blocking dialog starts waiting
    void Begin(unsigned long long nowMs, int liveWs);
    // every pass of the wait loop; true = launch the browser now (the launch is counted)
    bool Tick(unsigned long long nowMs, int liveWs);
    // the dialog was answered / closed
    void End() { active_ = false; }

    bool active() const   { return active_; }
    int  launches() const { return launches_; }

private:
    bool active_;
    bool noWeb_;                      // currently no live WebSocket
    unsigned long long noWebSince_;   // when the current absence started
    unsigned long long lastLaunch_;
    int launches_;
};

// The release screen (#26 Q3 "開正式版畫面"): background.html without mode=debug, the same URL
// HT9045_Web.cmd:39 opens -- only the port follows the running server.
std::string ModalWakeUrl(int port);

// Launch Edge the way HT9045_Web.cmd:40-42 / :81-82 does (same exe search, same profile
// %LOCALAPPDATA%\HT9045_Edge_Web, same window options).  Does not wait for it.
// false + why: Edge not found / CreateProcess failed (the caller logs it; the dialog keeps waiting).
bool ModalWakeLaunchEdge(const std::string& url, std::string& why);

}  // namespace ht9045

#endif  // W906_WEB_MODAL_WAKE_H
