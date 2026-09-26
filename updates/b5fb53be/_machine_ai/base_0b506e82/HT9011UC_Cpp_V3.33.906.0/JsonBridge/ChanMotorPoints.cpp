// ===========================================================================
//  JsonBridge/ChanMotorPoints.cpp
//
//  AI(W906-MOTOR-POINTS) 20260924.  NOT in golden.
//  每一軸（馬達表 Alias）的設定與即時狀態，給 HW.MotorTest.html／HW.teach.html。
//  設計、資料來源、null-vs-0 規則寫在 JsonBridge/ChanMotor.h 檔尾，這裡不重複。
// ===========================================================================
#include "JsonBridge/ChanMotor.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

#include "WebBridge/JsonWriter.h"
#include "vclcompat/vcl_compat.h"
#include "database.h"        // HSys.MotTable / TMOTDATA
#include "common.h"          // MotTablePath
#include "Motor/mymotor.h"   // MOT[MAX_TRAY_MOTOR] / TTrayMotor

namespace ht9045 {
namespace sjson {

namespace {

unsigned long long g_mrtSeq = 0;
MotorRuntimeOverlayFn g_overlay = 0;   // AI(W906-W4-MOTOR) 20260925: W4-c 覆蓋掛鉤（ChanMotor.h 檔尾）
const int kMotorPollMs = 1000;       // HW.MotorTest.html 原本就是每秒重讀一次（loadRuntimeOnly）

std::string NowIso()
{
    using namespace std::chrono;
    const system_clock::time_point now = system_clock::now();
    const std::time_t t = system_clock::to_time_t(now);
    const long ms = (long)(duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000);
    std::tm g = *std::gmtime(&t);    // 單執行緒（UI tick）呼叫
    char buf[40];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                  g.tm_year + 1900, g.tm_mon + 1, g.tm_mday, g.tm_hour, g.tm_min, g.tm_sec, ms);
    return buf;
}

std::string S(const AnsiString& a) { return std::string(a.c_str()); }

// "M07" -> 7；不是 M+數字 -> -1（cinitial.cpp 以 "M%02d" 建 MOT[i] 與表列的對應）
int MotorIndexOf(const AnsiString& no)
{
    const std::string s = S(no);
    if (s.size() < 2 || (s[0] != 'M' && s[0] != 'm')) return -1;
    for (std::size_t i = 1; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9') return -1;
    return std::atoi(s.c_str() + 1);
}

void IntOrNull(webbridge::JsonWriter& w, const char* k, int v, int nullValue)
{
    w.Key(k);
    if (v == nullValue) w.Null(); else w.Number((wb_int64)v);
}

}  // namespace

std::string MotorConfigJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("schemaVersion").String("2.0.0");
    w.Key("generatedAt").String(NowIso());
    w.Key("source").BeginObject();
    w.Key("motTable").String(S(MotTablePath));
    w.Key("producer").String("wb_serve: HSys.MotTable (the table C++ loaded at boot)");
    w.Key("rows").Number((wb_int64)HSys.MotTable.size());
    w.EndObject();

    w.Key("motors").BeginArray();
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* r = HSys.MotTable[i];
        if (!r || r->Alias.Length() == 0) continue;
        w.BeginObject();
        w.Key("motorId").String(S(r->Alias));
        w.Key("motorIndex");
        const int mi = MotorIndexOf(r->No);
        if (mi < 0) w.Null(); else w.Number((wb_int64)mi);
        w.Key("alias").String(S(r->Alias));
        w.Key("no").String(S(r->No));
        w.Key("group").String("");
        w.Key("axis").String("");
        w.Key("driverType").String(S(r->CardModel));
        w.Key("isaBase").Null();
        w.Key("enabledByDefault").Bool(r->iEnable == 1);
        w.Key("enableCondition").String("");
        w.Key("limits").BeginObject();
        w.Key("softP").Number((wb_int64)r->iSoftLimitP);
        w.Key("softN").Number((wb_int64)r->iSoftLimitN);
        w.Key("zSafePos").Null();
        w.Key("zLimitPos").Null();
        w.EndObject();
        w.Key("encoder").BeginObject();
        w.Key("tolerance").Null();
        w.EndObject();
        w.Key("hw").BeginObject();
        IntOrNull(w, "boardId", r->iBoardID, -1);
        IntOrNull(w, "port", r->iPort, -1);
        IntOrNull(w, "ip", r->iIP, -1);
        w.Key("gearRatio").Number(r->dGearRatio);
        w.Key("direction").Number((wb_int64)r->iDirection);
        w.Key("homeDirection").Number((wb_int64)r->iHomeDirectior);
        w.EndObject();
        w.Key("ui").BeginObject();
        w.Key("pages").BeginArray().EndArray();
        w.Key("componentNames").BeginArray().String(S(r->Alias)).EndArray();
        w.EndObject();
        w.Key("note").String("");
        w.Key("params").BeginObject();
        w.Key("initSpeed").Number((wb_int64)r->iInitSpeed);
        w.Key("jogHighSpeed").Number((wb_int64)r->iJogHighSpeed);
        w.Key("jogLowSpeed").Number((wb_int64)r->iJogLowSpeed);
        w.Key("homeHighSpeed").Number((wb_int64)r->iHomeHighSpeed);
        w.Key("homeLowSpeed").Number((wb_int64)r->iHomeLowSpeed);
        w.Key("softLimitP").Number((wb_int64)r->iSoftLimitP);
        w.Key("softLimitN").Number((wb_int64)r->iSoftLimitN);
        w.Key("acc").Number(r->dAcc);
        w.Key("dec").Number(r->dDec);
        w.Key("range").Number((wb_int64)r->iRange);
        w.EndObject();
        w.EndObject();
    }
    w.EndArray();

    w.Key("index").BeginObject();
    w.Key("byId").BeginObject();
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i)
        if (HSys.MotTable[i] && HSys.MotTable[i]->Alias.Length())
            w.Key(S(HSys.MotTable[i]->Alias)).Number((wb_int64)i);
    w.EndObject();
    w.EndObject();

    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

void SetMotorRuntimeOverlay(MotorRuntimeOverlayFn fn) { g_overlay = fn; }

std::string MotorRuntimeJson()
{
    const std::string now = NowIso();
    const unsigned long long seq = ++g_mrtSeq;
    int nGood = 0, nNoSrc = 0;

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("schemaVersion").String("2.0.0");
    w.Key("motors").BeginArray();
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* r = HSys.MotTable[i];
        if (!r || r->Alias.Length() == 0) continue;
        const int mi = MotorIndexOf(r->No);
        const bool motLive = (mi >= 0 && mi < MAX_TRAY_MOTOR && MOT[mi].Motor != 0);
        MotorRuntimeOverlay ov;                                                 // W4-c：1203 軸改讀監看器
        const bool hasOv = (g_overlay != 0) && g_overlay(S(r->Alias), ov);
        const bool live = motLive || hasOv;
        if (live) ++nGood; else ++nNoSrc;

        w.BeginObject();
        w.Key("motorId").String(S(r->Alias));
        w.Key("position").BeginObject();
        if (hasOv) {
            w.Key("cmdPos"); if (ov.posKnown) w.Number((wb_int64)ov.cmdPos); else w.Null();
            w.Key("encPos"); if (ov.posKnown) w.Number((wb_int64)ov.encPos); else w.Null();
        } else {
            w.Key("cmdPos");    if (live) w.Number((wb_int64)MOT[mi].Position);        else w.Null();
            w.Key("encPos");    if (live) w.Number((wb_int64)MOT[mi].EncoderPosition); else w.Null();
        }
        w.Key("targetPos"); if (motLive) w.Number((wb_int64)MOT[mi].TargetPosition);  else w.Null();
        w.EndObject();
        // 速度沒有快取欄位（要打驅動才知道）⇒ null；busy 只有監看器樣本知道
        w.Key("motion").BeginObject();
        w.Key("speed").Null();
        w.Key("accel").Null();
        w.Key("decel").Null();
        w.Key("busy"); if (hasOv) w.Bool(ov.busy); else w.Null();
        w.Key("homeBusy"); if (hasOv) w.Bool((ov.state & 0xFFu) == 4u); else w.Null();   // STA_AX_HOMING
        w.EndObject();
        w.Key("state").BeginObject();
        w.Key("servoOn"); if (hasOv) w.Bool(ov.servoOn); else w.Null();
        w.Key("alarm");   if (hasOv) w.Bool(ov.alarm);   else w.Null();
        w.Key("homeDone");  if (motLive) w.Bool(MOT[mi].HomeFlag == 1); else w.Null();
        w.Key("inPos");   if (hasOv) w.Bool(ov.inPos);   else w.Null();
        w.Key("isOn").Null();
        w.Key("isOff").Null();
        w.Key("updatedAt"); if (live) w.String(now); else w.Null();
        w.Key("quality").String(hasOv ? (ov.posKnown ? "good" : "partial") : (live ? "good" : "nosource"));
        w.EndObject();
        w.Key("diag").BeginObject();
        w.Key("errCode").Number((wb_int64)(hasOv ? ov.state : 0));
        std::string err;
        if (hasOv && !ov.posKnown) err = ov.why;
        if (hasOv && ov.alarm && !ov.driveErr.empty()) err += (err.empty() ? "" : "; ") + ov.driveErr;
        if (motLive && MOT[mi].HomeFlag == 2) err += (err.empty() ? "" : "; ") + std::string("home failed (HomeFlag=2)");
        w.Key("errText").String(err);
        w.Key("lastCommand").String("");
        w.Key("homeFlag"); if (motLive) w.Number((wb_int64)MOT[mi].HomeFlag); else w.Null();
        w.Key("driver").String(hasOv ? S(r->CardModel) + " (EastSun 1203 monitor)" : S(r->CardModel));
        w.EndObject();
        w.Key("seq").Number((wb_int64)seq);
        w.EndObject();
    }
    w.EndArray();

    w.Key("runtime").BeginObject();
    w.Key("connected").Bool(nGood > 0);
    w.Key("lastPollAt").String(now);
    w.Key("pollIntervalMs").Number((wb_int64)kMotorPollMs);
    w.Key("provider").String("wb_serve:MOT[]");
    w.Key("seq").Number((wb_int64)seq);
    w.Key("counts").BeginObject();
    w.Key("good").Number((wb_int64)nGood);
    w.Key("nosource").Number((wb_int64)nNoSrc);
    w.EndObject();
    if (nGood == 0)
        w.Key("why").String("沒有任何一軸建立了驅動物件（MOT[i].Motor 全為 NULL）：馬達表沒載入，或 InitialMotorParameter 還沒跑。");
    w.EndObject();

    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

}  // namespace sjson
}  // namespace ht9045
