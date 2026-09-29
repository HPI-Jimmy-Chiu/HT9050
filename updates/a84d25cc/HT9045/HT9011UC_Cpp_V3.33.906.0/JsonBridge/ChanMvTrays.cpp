// ===========================================================================
//  JsonBridge/ChanMvTrays.cpp -- S118: stage the main-screen trays (MOT[m].Tray) as motionView.trays.* tags.
//  AI(W906-S118) 20260928 (St02-E).  See ChanMvTrays.h for the shape, the golden bindings and the change rule.
//
//  Call: W906_StageMotionViewTrays(snap) from PublishExtraTags (tools/wb_serve.cpp), i.e. inside PublishHandlerTags
//  before commitPublish(), on the tick thread -- the same thread that runs MainProc and so writes MOT[].Tray, so the
//  read needs no lock.  A tag not staged in a tick drops out of the next generation (WebBridge/TagSnapshot.h), so
//  every tray tag is staged every tick; its JSON string is rebuilt only when the tray changed (UpdateTray).
//  Global (not in a namespace): wb_serve declares it at file scope, outside its anonymous namespace.
//  Pure reads + stage: no I/O, no lock, no wait.
// ===========================================================================
#include "JsonBridge/ChanMvTrays.h"
#include "Motor/mymotor.h"          // TTrayMotor MOT[MAX_TRAY_MOTOR] and its TMyTray Tray
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"

#include <cstddef>
#include <cstdint>

static_assert(ht9045::mvtray::kMaxCol == _MAX_COL_ITEM && ht9045::mvtray::kMaxRow == _MAX_ROW_ITEM,
              "ChanMvTrays.h array bounds must equal mytray.h _MAX_COL_ITEM / _MAX_ROW_ITEM");

// the MOT[] index of every tray, from the same list as TrayBindings() (so the same order and count)
static const int* const kTrayMotors[] = {
#define HT9045_MV_ADDR(n, m, p) &m,
    HT9045_MV_TRAY_LIST(HT9045_MV_ADDR)
#undef HT9045_MV_ADDR
};
static_assert(sizeof(kTrayMotors) / sizeof(kTrayMotors[0]) == (size_t)ht9045::mvtray::kTrayCount,
              "one motor per tray binding");

std::size_t W906_StageMotionViewTrays(webbridge::TagSnapshot& snap)
{
    using namespace ht9045::mvtray;
    const std::vector<TrayBinding>& t = TrayBindings();
    static std::vector<TrayState> st(t.size());   // tick thread only
    static unsigned long allVer = 0;
    static const std::string keys = TrayKeysJson();
    static std::vector<std::string> tagName;       // AI(W906-S118-R3) 20260928 (St02-E, St02-E2 review R3): the 34 tag names, built once
    if (tagName.size() != t.size())
    {
        tagName.clear();
        for (size_t i = 0; i < t.size(); ++i)
            tagName.push_back(std::string("motionView.trays.") + t[i].name);
    }
    std::size_t n = 0;
    for (size_t i = 0; i < t.size(); ++i)
    {
        const int m = *kTrayMotors[i];
        if (m < 0 || m >= MAX_TRAY_MOTOR)
            continue;
        const TMyTray& tray = MOT[m].Tray;
        if (UpdateTray(st[i], t[i].name, tray.XItem, tray.YItem, tray.Data))
            ++allVer;
        snap.stage(tagName[i], webbridge::TagValue::makeString(st[i].json));
        ++n;
    }
    snap.stage("motionView.trays.ver", webbridge::TagValue::makeInt((std::int64_t)allVer));
    snap.stage("motionView.trays.keys", webbridge::TagValue::makeString(keys));
    return n + 2;
}
