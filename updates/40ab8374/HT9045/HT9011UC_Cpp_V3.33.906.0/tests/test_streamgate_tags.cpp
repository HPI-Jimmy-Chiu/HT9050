// test_streamgate_tags.cpp -- the 1203 block of PublishHandlerTags behind the stream gate.
// AI(W906-STREAM-2B) 20260930: RULINGS_20260930 #12 (user 20:4x 「有開的網頁才能更新資料」).
// Pins: (1) no hook installed (every other ctest, test_wb_tags' exact counts) = today's output, byte for byte the same key
// set; (2) hook says "pci1203 not wanted" = only pci1203.linked / enabled / phase remain of the family, nothing else of the
// snapshot changes; (3) hook says wanted = the full family again (so opening the page brings every tag back on the next
// publish); (4) the hook is asked with the background.html id "pci1203".
#include "WebBridgeTags.h"

#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"

#include <cstdio>
#include <set>
#include <string>

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("  %s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static bool g_wanted = true;
static std::string g_askedFor;
static int g_asks = 0;
static bool Hook(const char* webId) { ++g_asks; g_askedFor = webId ? webId : ""; return g_wanted; }

static std::set<std::string> Keys(webbridge::TagSnapshot& snap)
{
    ht9045::PublishHandlerTags(snap);
    std::set<std::string> k;
    const webbridge::TagSnapshotView v = snap.read();
    for (webbridge::TagMap::const_iterator it = v.tags.begin(); it != v.tags.end(); ++it) k.insert(it->first);
    return k;
}
static std::size_t Pci(const std::set<std::string>& k)
{
    std::size_t n = 0;
    for (std::set<std::string>::const_iterator it = k.begin(); it != k.end(); ++it) if (it->compare(0, 8, "pci1203.") == 0) ++n;
    return n;
}

int main()
{
    webbridge::TagSnapshot snap;

    // (1) no hook: today's behaviour
    ht9045::SetStreamWantedHook(0);
    const std::set<std::string> base = Keys(snap);
    const std::size_t basePci = Pci(base);
    std::printf("no hook: %u tags, %u pci1203.*\n", (unsigned)base.size(), (unsigned)basePci);
    check(basePci > 3, "without the gate the whole pci1203 family is staged (more than the three card-layer tags)");

    // (2) not wanted
    ht9045::SetStreamWantedHook(&Hook);
    g_wanted = false;
    const std::set<std::string> closed = Keys(snap);
    std::printf("closed: %u tags, %u pci1203.*\n", (unsigned)closed.size(), (unsigned)Pci(closed));
    check(g_asks >= 1 && g_askedFor == "pci1203", "the hook is asked with the background.html id \"pci1203\"");
    check(Pci(closed) == 3, "closed: exactly three pci1203.* tags remain");
    check(closed.count("pci1203.linked") && closed.count("pci1203.enabled") && closed.count("pci1203.phase"),
          "closed: they are linked / enabled / phase (always published, St01 s4.3)");
    bool restSame = true;
    for (std::set<std::string>::const_iterator it = base.begin(); it != base.end(); ++it)
        if (it->compare(0, 8, "pci1203.") != 0 && !closed.count(*it)) restSame = false;
    for (std::set<std::string>::const_iterator it = closed.begin(); it != closed.end(); ++it)
        if (it->compare(0, 8, "pci1203.") != 0 && !base.count(*it)) restSame = false;
    check(restSame, "closed: every tag outside pci1203.* is exactly the same set as without the gate");

    // (3) wanted again
    g_wanted = true;
    const std::set<std::string> open = Keys(snap);
    check(open == base, "wanted again: the key set is back to the ungated one");

    ht9045::SetStreamWantedHook(0);
    const std::set<std::string> unhooked = Keys(snap);
    check(unhooked == base, "hook removed: back to today's output");

    std::printf("test_streamgate_tags: %d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
