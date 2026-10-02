// ===========================================================================
//  JsonBridge/ChanHome.cpp -- the web Home Monitor: home.* tags + act.home.abort.
//  AI(W906-HOMEMON) 20261001.  The shape, the golden bindings and the gates are in ChanHome.h.
//
//  Tick thread only (PublishExtraTags inside PublishHandlerTags; the command drain and the blocking-dialog waits of
//  tools/wb_serve.cpp) -- the thread that runs ProcessMotorHome, which writes everything read here, so no lock.
//  Pure reads + stage on the tag side: no I/O, no wait.  Global functions (wb_serve declares them at file scope,
//  outside its anonymous namespaces).
// ===========================================================================
#include "JsonBridge/ChanHome.h"

#include "forms/fHome.h"                // TfHome / fHome / THomeClass / W906_HomeLedState
#include "vclcompat/Controls.h"         // TLabel / TEdit / TListBox / TPanel (full types)
#include "cmydef.h"                     // SystemStart / fAllMotorHome / SoftStop / bMotorPowerState
#include "Motor/mymotor.h"              // MAX_TRAY_MOTOR (the size of W906_HomeLedState, forms/fHome.cpp:505)
#include "WebBridge/TagSnapshot.h"
#include "WebBridge/TagValue.h"
#include "WebBridge/JsonWriter.h"       // escapes + UTF-8 repair (a bad byte must never drop the frame)
#include "WebBridge/WebBridgeServer.h"
#include "WebBridge/CommandQueue.h"     // WebCommand

#include <cstdio>
#include <cstdint>
#include <string>

namespace ht9045 { namespace homemon {

bool HomeShown() { return fHome != 0 && fHome->fShow; }   //AI(W906-HOMEMON) 20261001: stays DIRECT on purpose (R142 = A, as csystem.cpp's MainProc stop arm `if(fHome->fShow) fHome->sbAbortHomeClick(fHome);`): golden's Home Monitor on screen = the home sequence showed it (ProcessMotorHome case 20 Show()); a page the operator opened on the web is not "shown" for the abort, and its stream is the streamWanted argument.  FShow_Audit baseline raised for this one line in the same commit

std::string RowsJson(const TfHome& h, const int* lamp, int lampCount)
{
    webbridge::JsonWriter w;
    w.BeginArray();
    for (std::size_t i = 0; i < h.HomeClass.size(); ++i) {
        const THomeClass* c = h.HomeClass[i];
        if (c == 0 || !c->Visible) continue;                                   // golden layout loop :357-375 places Visible rows only
        w.BeginObject();
        w.Key("slot").Number((wb_int64)i);
        w.Key("motor").Number((wb_int64)c->index);
        if (c->labName != 0) {
            w.Key("name").String(c->labName->Caption.c_str());                // golden :82 MOT[MotNo].NumberAlias
            w.Key("x").Number((wb_int64)c->labName->Left);                     // golden :361-362 (forms/fHome.cpp:430-431)
            w.Key("y").Number((wb_int64)c->labName->Top);
        } else {
            w.Key("name").Null(); w.Key("x").Null(); w.Key("y").Null();
        }
        if (c->edPos != 0) w.Key("pos").String(c->edPos->Text.c_str());        // golden :90 "0", then ShowMotorHomePos :694/:696
        else               w.Key("pos").Null();
        if (lamp != 0 && (int)i < lampCount) w.Key("lamp").Number((wb_int64)lamp[i]);   // ShowLed's attr (forms/fHome.cpp:487)
        else                                 w.Key("lamp").Null();
        w.EndObject();
    }
    w.EndArray();
    return w.Str();
}

std::string LogJson(const TfHome& h, int maxLines)
{
    webbridge::JsonWriter w;
    w.BeginArray();
    const vclcompat::TStringList* items = h.ListBox1 ? h.ListBox1->Items : 0;
    const int n = items ? items->GetCount() : 0;
    for (int k = 0; k < n && k < maxLines; ++k)                               // Items[0] = newest (golden Insert(0, ...))
        w.String(items->GetString(k).c_str());
    w.EndArray();
    return w.Str();
}

} }   // namespace ht9045::homemon

std::size_t W906_StageHomeMonitor(webbridge::TagSnapshot& snap, bool streamWanted)
{
    using namespace ht9045::homemon;
    using webbridge::TagValue;
    if (fHome == 0) return 0;
    const bool shown = HomeShown();
    if (!shown && !streamWanted) return 0;                                     // no window anywhere: the family leaves the snapshot

    static std::string rows;                                                   // tick thread only
    rows = RowsJson(*fHome, W906_HomeLedState, MAX_TRAY_MOTOR);

    // log: rebuilt only when the count, the newest or the oldest line changed (else two AnsiString copies per tick)
    static std::string log = "[]", logFirst, logLast;
    static int logCount = -1;
    const vclcompat::TStringList* items = fHome->ListBox1 ? fHome->ListBox1->Items : 0;
    const int cnt = items ? items->GetCount() : 0;
    const std::string first = cnt > 0 ? std::string(items->GetString(0).c_str()) : std::string();
    const std::string last  = cnt > 0 ? std::string(items->GetString(cnt - 1).c_str()) : std::string();
    if (cnt != logCount || first != logFirst || last != logLast) {
        log = LogJson(*fHome, kHomeLogMax);
        logCount = cnt; logFirst = first; logLast = last;
    }

    snap.stage("home.fShow",     TagValue::makeBool(shown));
    snap.stage("home.fAbort",    TagValue::makeBool(fHome->fAbort));
    snap.stage("home.step",      TagValue::makeInt((std::int64_t)fHome->iHomeStep));
    snap.stage("home.resetOk",   fHome->Panel2 ? TagValue::makeBool(fHome->Panel2->Visible) : TagValue::makeNull());
    snap.stage("home.rows",      TagValue::makeString(rows));
    snap.stage("home.log",       TagValue::makeString(log));
    snap.stage("home.log.count", TagValue::makeInt((std::int64_t)cnt));
    return 7;
}

static std::string HomeAbortAck()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("accepted").Bool(true);
    w.Key("fShow").Bool(ht9045::homemon::HomeShown());                          // false after golden Close()
    w.Key("fAbort").Bool(fHome != 0 && fHome->fAbort);
    w.Key("homeStep").Number((wb_int64)(fHome ? fHome->iHomeStep : -1));
    w.Key("systemStart").Bool(SystemStart != 0);
    w.Key("fAllMotorHome").Bool(fAllMotorHome != 0);
    w.Key("motorPower").Bool(bMotorPowerState != 0);                            // GaliMotorServoOff cut it (golden)
    w.EndObject();
    return w.Str();
}

bool W906_HomeAbortWire(const std::string& valueJson, std::string& ack)
{
    (void)valueJson;   // golden sbAbortHomeClick takes nothing from the page; the request itself is in the oplog (CMD line)
    if (fHome == 0) {
        ack = "act.home.abort: fHome is null -- no Home Monitor in this binary";
        return false;
    }
    if (!ht9045::homemon::HomeShown()) {
        ack = "not-open: the Home Monitor is not shown by the home sequence (fHome->fShow is false) -- golden's Abort Home "
              "button is on that form only; nothing was done（回原點畫面沒有在回原點中，沒有執行）";
        std::printf("act.home.abort REFUSED: fHome->fShow is false (iHomeStep=%d)\n", fHome->iHomeStep);
        std::fflush(stdout);
        return false;
    }
    std::printf("act.home.abort: golden TfHome::sbAbortHomeClick (uhome.cpp:4980-4986)  iHomeStep=%d SystemStart=%d SoftStart=%d\n",
                fHome->iHomeStep, (int)SystemStart, (int)SoftStart);
    try {
        fHome->sbAbortHomeClick(fHome);                                        // GaliMotorServoOff("sbAbortHomeClick") + fAbort=true + Close()
    } catch (...) {
        ack = "exception in golden TfHome::sbAbortHomeClick";
        std::printf("act.home.abort: %s\n", ack.c_str());
        std::fflush(stdout);
        return false;
    }
    ack = HomeAbortAck();
    std::printf("act.home.abort -> %s\n", ack.c_str());
    std::fflush(stdout);
    return true;
}

void W906_HomeAbortCommand(webbridge::WebBridgeServer& server, const webbridge::WebCommand& wc)
{
    std::string ack;
    const std::string v = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
    const bool ok = W906_HomeAbortWire(v, ack);
    server.CompleteCommand((unsigned long long)wc.id, ok, ack);
}
