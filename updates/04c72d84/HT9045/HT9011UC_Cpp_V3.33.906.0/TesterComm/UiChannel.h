// ===========================================================================
//  TesterComm/UiChannel.h -- the web page's window onto the tester-comm engines (plan P7).
//
//  AI(W906-GB-P7) 20260926.  User ruling 1: "GPIB 的 IO 通訊必須是即時的，HTML 畫面可以有一點點延遲".  So the page
//  never touches an engine directly:
//    * snapshots: the engine, on the TesterComm thread, copies what golden showed on its form (LEDs, status bar,
//      32 site panels, check boxes, combos, log tails, notices) into a JSON text every ~200 ms and Publish()es it.
//      Readers (the wb_serve tick, any thread) take the latest copy with Read().  The golden bodies write their
//      widgets without locks, exactly as golden; only this copy is shared, under the channel's own mutex.
//    * commands: the page's clicks / edits are Post()ed as short text commands ("click btnManualStart",
//      "check chkUpperCase 1", "combo cbbGPIBTimo 2", "site 3 on 1", ...) and the engine drains them on the
//      TesterComm thread in RunOnce, sets the widget and calls the golden OnClick/OnChange handler.
//  One channel per engine family (key "gpib" / "rs232" / "tcpip"), so the page's four tabs stay independent.
//  Header-only dependencies: WebBridge/Sync.h + std.
// ===========================================================================
#ifndef TESTERCOMM_UICHANNEL_H
#define TESTERCOMM_UICHANNEL_H

#include "WebBridge/Sync.h"

#include <deque>
#include <map>
#include <string>
#include <vector>

namespace testercomm {

class UiChannel
{
public:
    static UiChannel& Instance();

    // ---- snapshots (engine -> page) ----
    void Publish(const std::string& key, const std::string& json);           // TesterComm thread
    bool Read(const std::string& key, std::string* json, unsigned long* seq) const;   // any thread; false if none yet
    void Clear(const std::string& key);                                      // engine stopped

    // ---- commands (page -> engine) ----
    // any thread; bounded (oldest dropped).  AI(W906-GB-P7) 20260926: windowMs > 0 drops an EXACT repeat of the
    // same key + command posted within windowMs (a double click / double POST); returns false when dropped.
    bool Post(const std::string& key, const std::string& command, unsigned windowMs = 0);
    std::vector<std::string> Take(const std::string& key);                   // TesterComm thread
    unsigned long Dropped() const;

    static const size_t kMaxQueued = 64;

private:
    UiChannel();
    UiChannel(const UiChannel&);
    UiChannel& operator=(const UiChannel&);

    struct Snap
    {
        std::string json;
        unsigned long seq;
        Snap() : seq(0) {}
    };
    mutable webbridge::WbMutex mu_;
    std::map<std::string, Snap> snaps_;
    std::map<std::string, std::deque<std::string> > cmds_;
    struct LastPost
    {
        std::string command;
        unsigned long tick;
        LastPost() : tick(0) {}
    };
    std::map<std::string, LastPost> last_;
    unsigned long dropped_;
};

// Minimal JSON text helpers for the snapshot builders (no allocation-heavy library in the engines).
std::string JsonEscape(const std::string& s);                 // quotes included: "\"...\""

}  // namespace testercomm

#endif
