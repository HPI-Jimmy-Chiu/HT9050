// ===========================================================================
//  WebTrayEdit.cpp -- the WS command act.trayEdit for the page HW.TrayEdit (golden uTrayEditForm, TrayEditForm.cpp).
//
//  AI(W906-S10) 20260929 (St02-E).  Only in wb_serve (CMakeLists.txt claim S-10 3.4); dispatched from
//  tools/wb_serve.cpp (claim 3.5), not from JsonBridge/ChanAction.cpp (the tests link that one, not this).
//  value = JSON string {"op": ...}:
//    rightClick {"panel":"pnlAuto1Yield"}  golden cSortCT right-click -> EditTray (every guard in TrayEditForm.cpp)
//    state                                 the form as the page draws it; runs golden Timer1Timer first
//    select {"x0","y0","x1","y1","moved"}  mtLoaderBuffer MouseDown / MouseMove (if the pointer moved on a cell) / MouseUp -> SetTray
//    binCount {"value":"n"} | noSave {"value":bool} | update | cancel | fill {"x":"…","y":"…"} | snapshot
//  Reply: {"executed":bool, "op":…, "guard":…, "detail":…, "state":{…}} -- "state" always, so the page redraws from
//  the server's truth.  ok = false only for a payload that is not JSON / has no op (a golden refusal is ok = true with
//  executed:false and the guard).  Holds FormLock (it writes MOT[].Tray, as WebSortCT.cpp holds it for lastdata.dat);
//  golden's ShowMyMessage runs after the lock is released (TrayEditForm.cpp QueueMyMessage, St02-E2 review note 1).
// ===========================================================================
#include <cstdio>
#include <cstdlib>
#include <string>

#include "TrayEditForm.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:39-40

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

std::string Str(const cJSON* o, const char* k)
{
    const cJSON* it = o ? cJSON_GetObjectItem(o, k) : 0;
    if (!it) return std::string();
    if (cJSON_IsString(it) && it->valuestring) return it->valuestring;
    if (cJSON_IsNumber(it)) { char b[32]; std::snprintf(b, sizeof(b), "%d", it->valueint); return b; }
    return std::string();
}

int Int(const cJSON* o, const char* k, int dflt)
{
    const cJSON* it = o ? cJSON_GetObjectItem(o, k) : 0;
    if (it && cJSON_IsNumber(it)) return it->valueint;
    if (it && cJSON_IsString(it) && it->valuestring) return std::atoi(it->valuestring);
    return dflt;
}

bool Bool(const cJSON* o, const char* k)
{
    const cJSON* it = o ? cJSON_GetObjectItem(o, k) : 0;
    return it && cJSON_IsTrue(it);
}

void WriteState(webbridge::JsonWriter& w, const w906trayedit::View& v)
{
    w.Key("state").BeginObject();
    w.Key("open").Bool(v.open);
    w.Key("motor").Number((wb_int64)v.motor);
    w.Key("motorName").String(v.motorName);
    w.Key("hasMap").Number((wb_int64)v.hasMap);
    w.Key("xItem").Number((wb_int64)v.xItem);
    w.Key("yItem").Number((wb_int64)v.yItem);
    w.Key("xbItem").Number((wb_int64)v.xbItem);
    w.Key("ybItem").Number((wb_int64)v.ybItem);
    w.Key("xbWidth").Number((wb_int64)v.xbWidth);
    w.Key("ybWidth").Number((wb_int64)v.ybWidth);
    w.Key("color").BeginArray();
    for (std::size_t i = 0; i < v.color.size(); ++i) w.Number((wb_int64)v.color[i]);
    w.EndArray();
    w.Key("number").BeginArray();
    for (std::size_t i = 0; i < v.number.size(); ++i) w.String(v.number[i]);
    w.EndArray();
    w.Key("colorMap").BeginObject();
    for (std::size_t i = 0; i < v.colorMap.size(); ++i) {
        char k[16];
        std::snprintf(k, sizeof(k), "%d", v.colorMap[i].first);
        w.Key(k).String(v.colorMap[i].second);
    }
    w.EndObject();
    w.Key("sel").BeginObject();
    w.Key("x0").Number((wb_int64)v.x0);
    w.Key("y0").Number((wb_int64)v.y0);
    w.Key("x1").Number((wb_int64)v.x1);
    w.Key("y1").Number((wb_int64)v.y1);
    w.EndObject();
    w.Key("caption").String(v.caption);
    w.Key("binCount").BeginObject();
    w.Key("visible").Bool(v.binVisible);
    w.Key("items").BeginArray();
    for (std::size_t i = 0; i < v.binItems.size(); ++i) w.String(v.binItems[i]);
    w.EndArray();
    w.Key("value").String(v.binValue);
    w.EndObject();
    w.Key("updateVisible").Bool(v.updateVisible);
    w.Key("noSave").Bool(v.noSave);
    w.EndObject();
}

}  // namespace

std::string W906_TrayEditCmd(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;
    cJSON* root = cJSON_Parse(payloadJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return "{\"executed\":false,\"guard\":\"bad-payload\",\"detail\":\"value must be a JSON object {\\\"op\\\":...}\"}";
    }
    const std::string op = Str(root, "op");
    w906trayedit::Result r;
    w906trayedit::View view;
    bool known = true;
    {
        FormLockGuard lock;
        if (op == "rightClick")    r = w906trayedit::RightClick(Str(root, "panel"));
        else if (op == "state")    w906trayedit::Timer1();
        else if (op == "select")   r = w906trayedit::Select(Int(root, "x0", -1), Int(root, "y0", -1), Int(root, "x1", -1), Int(root, "y1", -1),
                                                            Bool(root, "moved"));
        else if (op == "binCount") r = w906trayedit::BinCount(Str(root, "value"));
        else if (op == "noSave")   r = w906trayedit::NoSave(Bool(root, "value"));
        else if (op == "update")   r = w906trayedit::Update();
        else if (op == "cancel")   r = w906trayedit::Cancel();
        else if (op == "fill")     r = w906trayedit::Fill(Str(root, "x"), Str(root, "y"));
        else if (op == "snapshot") r = w906trayedit::Snapshot();
        else known = false;
        view = w906trayedit::GetView();   // under the lock: it reads MOT[].Tray (AI(W906-S10) 20260929 (St02-E, St02-E2 review note 1))
    }
    cJSON_Delete(root);
    if (!known) return "{\"executed\":false,\"guard\":\"unknown-op\"}";

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(r.executed);
    w.Key("op").String(op);
    w.Key("guard").String(r.guard);
    w.Key("detail").String(r.detail);
    WriteState(w, view);
    w.EndObject();
    if (op != "state") std::printf("act.trayEdit %s -> %s%s\n", op.c_str(), r.executed ? "done" : "refused ", r.guard.c_str());
    w906trayedit::ShowQueuedMessages();   // golden ShowMyMessage (the last thing that handler did), now outside FormLock
    if (!w.Ok()) return "{\"executed\":false,\"guard\":\"json-writer-misuse\"}";
    if (ok) *ok = true;
    return w.Str();
}
