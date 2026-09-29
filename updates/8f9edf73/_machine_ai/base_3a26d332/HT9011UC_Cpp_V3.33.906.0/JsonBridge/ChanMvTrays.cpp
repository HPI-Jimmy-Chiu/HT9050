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
#include "database.h"               // HSys.MotTable / TMOTDATA (motionView.screenScale)   AI(W906-INBOX117) 20260929 (St02-E)
#include <cstdio>
#include <cstdlib>
#include <string>

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


// AI(W906-INBOX117) 20260929 (St02-E): INBOX 117 -- golden TMyMotor::SetScreenScale's two screen and two machine points per axis, as
//   SetSimuScreenPara set them (cinitial.cpp:13851-14018; golden 906 :5855-6493), so the web Main.MotionView can place
//   an axis on golden's straight line between its two teach points (golden mymotor.cpp:301-311 / :410-415).
//   JSON {"<alias>":[refStart,refEnd,factStart,factEnd],...}: the Mot_Table alias the page keys LIVE.motor by, and only
//   axes golden gave a machine pair (factStart != factEnd; for an equal pair golden GetScale returns Scale 1.0, :305-306,
//   which draws pulses as pixels -- golden binds no control to such an axis, and the page keeps its own placement).
//   Motor index from the Mot_Table "No" column, the same rule as ChanMotorPoints.cpp MotorIndexOf (:53; that one is in
//   an anonymous namespace): "M<digits>" -> digits.
static int ScreenScaleMotorIndex(const AnsiString& no)
{
    const std::string s = no.c_str();
    if (s.size() < 2 || (s[0] != 'M' && s[0] != 'm')) return -1;
    for (std::size_t i = 1; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9') return -1;
    return std::atoi(s.c_str() + 1);
}

static std::size_t StageScreenScale(webbridge::TagSnapshot& snap)
{
    static std::string last;
    static unsigned long ver = 0;
    std::string j = "{";
    bool first = true;
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* r = HSys.MotTable[i];
        if (!r || r->Alias.Length() == 0) continue;
        const int mi = ScreenScaleMotorIndex(r->No);
        if (mi < 0 || mi >= MAX_TRAY_MOTOR) continue;
        int s1 = 0, e1 = 0, s2 = 0, e2 = 0;
        MOT[mi].W906_GetScreenScale(s1, e1, s2, e2);                         // Motor/mymotor.h:156 (the laptop OK'd, 21:5x)
        if (s2 == e2) continue;
        char b[160];
        std::snprintf(b, sizeof(b), "%s\"%s\":[%d,%d,%d,%d]", first ? "" : ",", r->Alias.c_str(), s1, e1, s2, e2);
        j += b;
        first = false;
    }
    j += "}";
    if (j != last) { last = j; ++ver; }                                    // SetWorkParameter re-runs SetSimuScreenPara on save
    snap.stage("motionView.screenScale", webbridge::TagValue::makeString(last));
    snap.stage("motionView.screenScale.ver", webbridge::TagValue::makeInt((std::int64_t)ver));
    return 2;
}

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
    return n + 2 + StageScreenScale(snap);                                 // AI(W906-INBOX117) 20260929 (St02-E): INBOX 117
}
