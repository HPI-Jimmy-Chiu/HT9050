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
#include <utility>   //AI(W906-MT-E1) 20260925: std::pair (W906_MotorTestVisibility)
#include <vector>

#include "WebBridge/JsonWriter.h"
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"     // AI(W906-IOWEB-P5) 20260924: SOFT_SIMULTE must be decided BEFORE the #ifdef below (tools/macro_order_gate.ps1)
#include "database.h"        // HSys.MotTable / TMOTDATA
#include "common.h"          // MotTablePath
#include "Motor/mymotor.h"   // MOT[MAX_TRAY_MOTOR] / TTrayMotor
#include "EtherCAT/Pci1203Monitor.h"   // AI(W906-IOWEB-P5) 20260924: PCI1203 rows read the card through the read-only monitor

std::vector<std::pair<int, bool> > W906_MotorTestVisibility();   //AI(W906-MT-E1) 20260925: forms/fMotorTest.cpp (global)
namespace ht9045 {
namespace sjson {

namespace {

unsigned long long g_mrtSeq = 0;
MotorRuntimeOverlayFn g_overlay = 0;   // AI(W906-W4-MOTOR) 20260925: W4-c 覆蓋掛鉤（ChanMotor.h 檔尾）
MotorTestPageStateFn  g_pageState = 0; //AI(W906-MT-E2) 20260925: Motor Test 整頁狀態掛鉤（ChanMotor.h 檔尾）
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
    //AI(W906-MT-E1) 20260925: golden Motor Test visibility per MOT index (forms/fMotorTest.cpp W906_MotorTestVisibility, the
    //  bView of golden uMotorTest.cpp:145-390). User EastSun 20260925: the motor list follows golden's rule.
    const std::vector<std::pair<int, bool> > mtVis = W906_MotorTestVisibility();
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
        bool mtShow = false;   //AI(W906-MT-E1) 20260925: same test as golden's layout loop (fMotorTest.cpp:572-577) -- by index, not by vector position (push order is not MotNo order, e.g. MLoad2Y)
        for (std::size_t v = 0; v < mtVis.size(); ++v) if (mtVis[v].first == mi && mtVis[v].second) { mtShow = true; break; }
        w.Key("motorTestVisible").Bool(mi >= 0 && mtShow);   //AI(W906-MT-E1) 20260925
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

//AI(W906-IOWEB-P5) 20260924: A PCI1203 ROW READS THE CARD, NOT MOT[].
//
//  Measured on the real machine's tree (read-only audit 20260924): MOT[i].Motor
//  is new'd for every one of TOTAL_MOTOR axes (cinitial.cpp InitialMotorParameter;
//  an unknown CardModel falls through to TMySMCMotor), so the old rule
//  `live = MOT[mi].Motor != 0` answered "good" for all 48 rows, with Position /
//  EncoderPosition values no card ever wrote -- in the shipping build the
//  engine's TMyEtherCatMotor never opens an axis (uiDevhand stays 0,
//  myEthercatmotor.cpp Open_Axis returns first). That breaks this file's own
//  null-not-0 rule (ChanMotor.h) in the worst way: a plausible number on a real
//  machine.
//
//  So a CardModel=="PCI1203" row now resolves through the read-only monitor --
//  the field-proven source (1203MON-12, CTL-32: axes opened BY ID, matching
//  production's Acm_AxOpenbyID(dev, BoardID, Port)):
//      Mot_Table BoardID == Pci1203AxisSample::station (ESC 0x0010 address)
//      Mot_Table Port    == Pci1203AxisSample::stationAxis (SubID; 0=A, 1=B)
//  and an axis flagged stationAmbiguous is REFUSED as a match: its station
//  number is shared by another drive, so a name keyed on it is not trustworthy
//  (Pci1203Monitor.h, 1203PHYS-1).
//
//  Other rows: SOFT_SIMULTE keeps THEIRS' MOT[] behaviour exactly (the sim
//  motors ARE the source there, and tests/test_motor_points [3] pins that); the
//  shipping build reports them nosource with a reason, because there MOT[] is
//  not connected to anything.
//  JSON shape unchanged; diag gains station/stationAxis/stateText/source/why.
namespace {
// values from EtherCAT/vendor/AdvMotDrv.h:792-807 / :2023-2037 -- copied, not
// included, so this TU does not pull the vendor header into non-armed targets
// (same practice as Pci1203Control.cpp:50-53).
const unsigned short kStaStopping = 2, kStaErrorStop = 3, kStaHoming = 4, kStaPtp = 5,
                     kStaConti = 6, kStaSync = 7, kStaExtJog = 8, kStaExtMpg = 9;
const unsigned long  kIoAlm = 0x00000002ul, kIoInp = 0x00002000ul, kIoSvon = 0x00004000ul;

const Pci1203AxisSample* FindCardAxis(TPci1203Monitor* mon, int station, int sub, const char** why)
{
    *why = "";
    const int n = mon ? mon->axisCount() : 0;
    if (n <= 0) { *why = "1203 監看器沒有開任何軸（沒有卡、卡沒開，或這顆 binary 沒有 HAVE_PCI1203）"; return 0; }
    if (station < 0 || sub < 0) { *why = "馬達表這一列沒有 BoardID/Port"; return 0; }
    bool ambiguous = false, invalid = false;
    for (int k = 0; k < n; ++k) {
        const Pci1203AxisSample& a = mon->axis(k);
        if (!a.opened || a.station != station || a.stationAxis != sub) continue;
        if (a.stationAmbiguous) { ambiguous = true; continue; }
        if (!a.valid) { invalid = true; continue; }
        return &a;
    }
    if (ambiguous) *why = "卡上這個站號不唯一（stationAmbiguous）：照站號對應的軸名不可信，拒絕對應";
    else if (invalid) *why = "卡上有這一軸，但這次取樣無效";
    else *why = "卡上沒有開到 (station=BoardID, stationAxis=Port) 這一軸 —— 檢查 Mot_Table 的 BoardID/Port";
    return 0;
}
}  // namespace
void SetMotorRuntimeOverlay(MotorRuntimeOverlayFn fn) { g_overlay = fn; }  MotorRuntimeOverlayFn W906_NativeMotorOverlay() { return g_overlay; }   /* AI(W906-NATIVE-PROTO) 20260928 [W906]: 原生 Main.MotorView（ui/native/NativeFormsWbServe.cpp，只在 -DW906_NATIVE_FORMS=ON 時連）讀網頁 /api/struct/motor/runtime 的**同一支**覆蓋掛鉤；宣告在呼叫端自己寫（沒有動 ChanMotor.h）。只讀、同一行插入，其後行號不動 */
void SetMotorTestPageState(MotorTestPageStateFn fn) { g_pageState = fn; }   //AI(W906-MT-E2) 20260925

std::string MotorRuntimeJson()
{
    const std::string now = NowIso();
    const unsigned long long seq = ++g_mrtSeq;
    int nGood = 0, nNoSrc = 0, nCard = 0;
    TPci1203Monitor* mon = Pci1203Monitor();
    MotorTestPageState pg;                                                      //AI(W906-MT-E2) 20260925: read once per body
    const bool hasPg = (g_pageState != 0) && g_pageState(pg);

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("schemaVersion").String("2.0.0");
    w.Key("motors").BeginArray();
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* r = HSys.MotTable[i];
        if (!r || r->Alias.Length() == 0) continue;
        const int mi = MotorIndexOf(r->No);
        //AI(W906-IOWEB-P25c) 20260925: MERGE of the machine's IOWEB-P5 and the laptop's W4-c.
        //  Values come from the laptop's overlay (W4-c: WebMotorAccessLive copies the monitor
        //  sample on the tick thread and converts cmd/enc to USER units with the golden gear
        //  ratio -- what golden Motor Test shows). The machine's P5 rule is kept on top: in
        //  the SHIPPING build MOT[] positions are not card values (MOT[i].Motor is new'd for
        //  every axis, the engine's TMyEtherCatMotor never opens one), so they are NOT shown
        //  as "good"; the engine's own flags (HomeFlag) are still read from MOT[]. `ax` is the
        //  P5 lookup, kept for diag.station/stationAxis/stateText and the reason when a row
        //  has no source. ⚠ Reading the monitor here is safe in this tree ONLY because the
        //  /api/struct/* bodies are built on the tick thread (IOWEB-P6 cache in wb_serve).
        const bool is1203 = (S(r->CardModel) == "PCI1203");
        const char* why = "";
        const Pci1203AxisSample* ax = 0;
        if (is1203) {
            if (r->iEnable != 1) why = "馬達表 Enable=0";
            else ax = FindCardAxis(mon, r->iBoardID, r->iPort, &why);
        }
        const bool motObj = (mi >= 0 && mi < MAX_TRAY_MOTOR && MOT[mi].Motor != 0);   // engine flags (HomeFlag) are real
#ifdef SOFT_SIMULTE
        const bool motLive = motObj;        // the sim motors ARE the source (tests/test_motor_points [3])
#else
        const bool motLive = false;         // P5: never show MOT[] positions as card values
        if (!ax && !is1203) why = "出貨組態：這一列不是 PCI1203，沒有接任何驅動後端（MOT[] 的值不是卡片來的）";
#endif
        MotorRuntimeOverlay ov;                                                 // W4-c：1203 軸改讀監看器
        const bool hasOv = (g_overlay != 0) && g_overlay(S(r->Alias), ov);
        const bool live = motLive || hasOv;
        if (hasOv) ++nCard;
        if (live) ++nGood; else ++nNoSrc;

        w.BeginObject();
        w.Key("motorId").String(S(r->Alias));
        w.Key("position").BeginObject();
        if (hasOv) {
            w.Key("cmdPos"); if (ov.posKnown) w.Number((wb_int64)ov.cmdPos); else w.Null();
            w.Key("encPos"); if (ov.posKnown) w.Number((wb_int64)ov.encPos); else w.Null();
        } else {
            w.Key("cmdPos");    if (motLive) w.Number((wb_int64)MOT[mi].Position);        else w.Null();
            w.Key("encPos");    if (motLive) w.Number((wb_int64)MOT[mi].EncoderPosition); else w.Null();
        }
        w.Key("targetPos"); if (motLive) w.Number((wb_int64)MOT[mi].TargetPosition);  else w.Null();
        w.EndObject();
        // 速度沒有快取欄位（要打驅動才知道）⇒ null；busy 只有監看器樣本知道
        w.Key("motion").BeginObject();
        w.Key("speed");    if (ax) w.Number(ax->cmdVel); else w.Null();
        w.Key("accel").Null();
        w.Key("decel").Null();
        w.Key("busy"); if (hasOv) w.Bool(ov.busy); else w.Null();
        w.Key("homeBusy"); if (hasOv) w.Bool((ov.state & 0xFFu) == 4u); else w.Null();   // STA_AX_HOMING
        //AI(W906-MT-E1) 20260925: Motor Test cross-tick jobs (WebMotorAccess MotorAccessJobs) -- the page pops btnHome /
        //  btnLoopMove up when the job is gone (golden Timer1Timer :962-966), and shows lblLoopCount (golden dwLoopCount).
        w.Key("homeJob");   if (hasOv) w.Bool(ov.homeJob); else w.Null();
        w.Key("loopJob");   if (hasOv) w.Bool(ov.loopJob); else w.Null();
        w.Key("loopCount"); if (hasOv && ov.loopJob) w.Number((wb_int64)ov.loopCount); else w.Null();
        //AI(W906-MT-E2) 20260925: golden lblJogPTime / lblJogNTime / lblAvgTime (uMotorTest.cpp:452 / :555 / :561) -- one set of
        //  labels for the whole form, so every row carries the same values; null = never measured (or no hook).
        w.Key("jogPTime"); if (hasPg && pg.hasJogPTime) w.Number((wb_int64)pg.jogPTime); else w.Null();
        w.Key("jogNTime"); if (hasPg && pg.hasJogNTime) w.Number((wb_int64)pg.jogNTime); else w.Null();
        w.Key("avgTime");  if (hasPg && pg.hasAvgTime)  w.Number(pg.avgTime);             else w.Null();
        //AI(W906-MT-E3c) 20260925: actual torque (EastSun R6: next to Real Speed) from the monitor sample through the overlay.
        //  Each value only when the monitor says it is valid this poll -- null, never 0 (a 0 % is a real reading).
        //  src: "pdo" (Acm_AxGetActTorque) | "sdo" (6077h/6877h, the focus axis) | "none". unitVerified: PDO and SDO agreed.
        w.Key("torque").BeginObject();
        w.Key("pct");  if (hasOv && ov.torquePctValid) w.Number(ov.torquePct); else w.Null();
        w.Key("nm");   if (hasOv && ov.torqueNmValid)  w.Number(ov.torqueNm);  else w.Null();
        w.Key("raw");  if (hasOv && ov.torqueRawValid) w.Number((wb_int64)ov.torqueRaw); else w.Null();
        w.Key("src").String(!hasOv ? "none" : ov.torqueSrc == 1 ? "pdo" : ov.torqueSrc == 2 ? "sdo" : "none");
        w.Key("unitVerified").Bool(hasOv && ov.torqueUnitVerified);
        w.EndObject();
        w.EndObject();
        w.Key("state").BeginObject();
        w.Key("servoOn"); if (hasOv) w.Bool(ov.servoOn); else w.Null();
        w.Key("alarm");   if (hasOv) w.Bool(ov.alarm);   else w.Null();
        w.Key("homeDone");  if (motObj) w.Bool(MOT[mi].HomeFlag == 1); else w.Null();   // the card does not know; only the engine does
        w.Key("inPos");   if (hasOv) w.Bool(ov.inPos);   else w.Null();
        w.Key("isOn").Null();
        w.Key("isOff").Null();
        //AI(W906-MT-E1) 20260925: golden Motor Test ALed1..10 = MOT[i].Led[0..9] (uMotorTest.cpp:626-642 UpdateMotorLed),
        //  decoded for a 1203 axis exactly as TMyEtherCatMotor::ScanMotorStatus does (Motor/myEthercatmotor.cpp:1166-1189):
        //  CW = LMT- (bit3), HOME = ORG (bit4), CCW = LMT+ (bit2), EM Stop = EMG (bit6), Alarm = ALM (bit1) or ERROR_STOP,
        //  Soft CW = SLMT_P (bit16), Soft CCW = SLMT_N (bit17), Servo On = SVON (bit14).
        //  S Alarm (Led[7]) is never set by golden for a 1203 axis, and InPos (Led[8]) is commented out there
        //  ("RogerYang 20250421 not work") -- so both stay false, as golden shows them. (state.inPos above is the raw
        //  INP bit, a different thing.) ⚠ HOME: golden relies on CFG_AxOrgLogic set at init by SensorType
        //  (myEthercatmotor.cpp:1174); here the axes are opened by the monitor, so this is the card's ORG bit as the
        //  card's current OrgLogic reports it. null = no sample for this row (lamp shows "unknown", not "off").
        w.Key("led");
        if (hasOv && ov.ioKnown) {
            const unsigned long io = ov.motionIO;
            w.BeginObject();
            w.Key("cw").Bool((io & 0x00000008ul) != 0);
            w.Key("home").Bool((io & 0x00000010ul) != 0);
            w.Key("ccw").Bool((io & 0x00000004ul) != 0);
            w.Key("emg").Bool((io & 0x00000040ul) != 0);
            w.Key("alarm").Bool((io & 0x00000002ul) != 0 || (ov.state & 0xFFu) == 3u);
            w.Key("softCw").Bool((io & 0x00010000ul) != 0);
            w.Key("softCcw").Bool((io & 0x00020000ul) != 0);
            w.Key("sAlarm").Bool(false);
            w.Key("inPos").Bool(false);
            w.Key("servoOn").Bool((io & 0x00004000ul) != 0);
            w.EndObject();
        } else {
            w.Null();
        }
        w.Key("updatedAt"); if (live) w.String(now); else w.Null();
        w.Key("quality").String(hasOv ? (ov.posKnown ? "good" : "partial") : (live ? "good" : "nosource"));
        w.EndObject();
        w.Key("diag").BeginObject();
        w.Key("errCode").Number((wb_int64)(hasOv ? ov.state : 0));
        std::string err;
        if (hasOv && !ov.posKnown) err = ov.why;
        if (hasOv && ov.alarm && !ov.driveErr.empty()) err += (err.empty() ? "" : "; ") + ov.driveErr;
        if (motObj && MOT[mi].HomeFlag == 2) err += (err.empty() ? "" : "; ") + std::string("home failed (HomeFlag=2)");
        if (!live && err.empty()) err = why;                                    // P5: say WHY there is no value
        w.Key("errText").String(err);
        w.Key("lastCommand").String("");
        w.Key("homeFlag"); if (motObj) w.Number((wb_int64)MOT[mi].HomeFlag); else w.Null();
        w.Key("driver").String(hasOv ? S(r->CardModel) + " (EastSun 1203 monitor)" : S(r->CardModel));
        w.Key("source").String(hasOv ? "pci1203-monitor" : (motLive ? "MOT[]" : "none"));   // P5 diag
        w.Key("station");     if (ax) w.Number((wb_int64)ax->station);     else w.Null();
        w.Key("stationAxis"); if (ax) w.Number((wb_int64)ax->stationAxis); else w.Null();
        w.Key("stateText");   if (ax) w.String(Pci1203AxisStateText(ax->state)); else w.Null();
        w.Key("why").String(hasOv ? ov.why : std::string(why));
        w.EndObject();
        //AI(W906-MT-E2) 20260925: "cur" = what golden Motor Test shows for the selected motor, from the golden object MOT[mi]
        //  (not Mot_Table, not the card): rows 1..10 of strngrdMotor are UpdateMotorParameter (uMotorTest.cpp:655-669) --
        //  initSpeed, jogHigh, jogLow, homeHigh, homeLow, softP, softN, acc (ReadAcc = the database value), dec, range;
        //  rate/range fill edtMotorRate/edtMotorRange on selection (:757-758); readSpeed = MOT.GetSpeed() = lblRealSpeed
        //  (:982, the last SetSpeed's value, NOT the card's running speed); lastHomePos = edtHomeOffset after a home (:965).
        //  Every one of them is a cached member (no driver call), and this body is built on the tick thread.
        //  null = no golden object for this row (MOT[mi].Motor is NULL).
        w.Key("cur");
        if (motObj) {
            HTMotor* M = MOT[mi].Motor;
            w.BeginObject();
            w.Key("enable").Bool(M->Enable);
            //AI(W906-MT-FIX1) 20260926: golden lM00Click :740-743 `#ifndef SOFT_SIMULTE if(Motor->Enable==false) return;` -- the SIM
            //  build sets every Motor->Enable false yet every motor is selectable; the page decides selection on this, not "enable".
#ifdef SOFT_SIMULTE
            w.Key("selectable").Bool(true);
#else
            w.Key("selectable").Bool(M->Enable);
#endif
            w.Key("initSpeed").Number((wb_int64)M->ReadInitSpeed());
            w.Key("jogHigh").Number((wb_int64)M->PJogHighSpeed);
            w.Key("jogLow").Number((wb_int64)M->PJogLowSpeed);
            w.Key("homeHigh").Number((wb_int64)M->PHomeHighSpeed);
            w.Key("homeLow").Number((wb_int64)M->PHomeLowSpeed);
            w.Key("softP").Number((wb_int64)M->PSoftLimitP);
            w.Key("softN").Number((wb_int64)M->PSoftLimitN);
            w.Key("acc").Number(M->ReadAcc());
            w.Key("dec").Number(M->ReadDec());
            w.Key("range").Number((wb_int64)M->ReadRange());
            w.Key("rate").Number((wb_int64)M->ReadRate());
            w.Key("readSpeed").Number((wb_int64)MOT[mi].GetSpeed());
            w.Key("lastHomePos").Number((wb_int64)M->LastHomePos);
            w.Key("gearRatio").Number(M->GearRatio);
            w.Key("homeFlag").Number((wb_int64)MOT[mi].HomeFlag);
            w.EndObject();
        } else {
            w.Null();
        }
        w.Key("seq").Number((wb_int64)seq);
        w.EndObject();
    }
    w.EndArray();

    w.Key("runtime").BeginObject();
    w.Key("connected").Bool(nGood > 0);
    w.Key("lastPollAt").String(now);
    w.Key("pollIntervalMs").Number((wb_int64)kMotorPollMs);
#ifdef SOFT_SIMULTE
    w.Key("provider").String(nCard > 0 ? "wb_serve:pci1203-monitor+MOT[]" : "wb_serve:MOT[]");
#else
    w.Key("provider").String("wb_serve:pci1203-monitor");
#endif
    w.Key("seq").Number((wb_int64)seq);
    //AI(W906-MT-E2) 20260925: the motor C++ believes the Motor Test page has selected (golden ActiveIndex; selectMotor)
    w.Key("selectedMotor"); if (hasPg && !pg.selectedMotor.empty()) w.String(pg.selectedMotor); else w.Null();
    w.Key("counts").BeginObject();
    w.Key("good").Number((wb_int64)nGood);
    w.Key("nosource").Number((wb_int64)nNoSrc);
    w.Key("card").Number((wb_int64)nCard);      // AI(W906-IOWEB-P5) 20260924: rows whose values came from the 1203 card
    w.EndObject();
    if (nGood == 0) {
#ifdef SOFT_SIMULTE
        w.Key("why").String("沒有任何一軸建立了驅動物件（MOT[i].Motor 全為 NULL）：馬達表沒載入，或 InitialMotorParameter 還沒跑。");
#else
        w.Key("why").String("沒有任何 PCI1203 列對到 1203 監看器開著的軸（見每一列 diag.why）；出貨組態不拿 MOT[] 當來源。");
#endif
    }
    w.EndObject();

    //AI(W906-MT-E3c) 20260925: the page-wide golden state, top level (contract: beside "motors" and "runtime"; the page also
    //  looks inside "runtime"). null = no hook / not filled (ctest) -- never a block of defaults.
    w.Key("motorPower");
    if (hasPg && pg.hasPower) {
        w.BeginObject();
        w.Key("relayOn").Bool(pg.relayOn);                                      // SW[SwMotorRelay].OutValue (after the card sync)
        w.Key("relayCard"); if (pg.relayCardKnown) w.Bool(pg.relayCard); else w.Null();
        w.Key("motorPowerState").Bool(pg.motorPowerState);                      // bMotorPowerState
        w.Key("pending").Bool(pg.powerPending);                                 // DoMotorPowerOn's 1 s still running
        w.EndObject();
    } else {
        w.Null();
    }
    w.Key("lock");
    if (hasPg && pg.hasLock) {
        w.BeginObject();
        w.Key("locked").Bool(pg.locked);                                        // golden LockAllButton(true): MoveN/MoveP/LoopMove disabled
        w.Key("text").String(pg.labLockVisible ? pg.lockText : std::string());  // golden labLock caption ("*Lock by Homeing" / "*Lock by M03 moveing")
        w.Key("pnlStop").String(pg.locked ? "#FFFF00" : "#CCD9DF");             // clYellow / (TColor)0x00DFD9CC (golden :2363, :1076)
        w.Key("unlockCount").Number((wb_int64)pg.unlockCount);
        w.Key("lastUnlockWhy").String(pg.lastUnlockWhy);
        w.EndObject();
    } else {
        w.Null();
    }
    w.Key("lightScale");
    if (hasPg && pg.hasLightScale) {
        w.BeginObject();
        w.Key("active").Bool(pg.lsActive);                                      // golden Timer2->Enabled
        w.Key("task").Number((wb_int64)pg.lsTask);                              // golden LightScale's Task
        w.Key("editsEnabled").Bool(pg.lsEditsEnabled);                          // edPitech / edDelayTime ->Enabled
        w.Key("homePending").Bool(pg.lsHomePending);                            // its bSingleHome
        w.Key("useAxis").Number((wb_int64)pg.lsUseAxis);                        // golden iUseAxis (MOT index)
        w.Key("axisItem").Number((wb_int64)pg.lsAxisItem);
        w.Key("moveType").Number((wb_int64)pg.lsMoveType);
        w.Key("needMovePos").Number((wb_int64)pg.lsNeedMovePos);
        w.Key("memoCount").Number((wb_int64)pg.lsMemoCount);                    // Memo1->Lines->Count
        w.Key("memoTail").BeginArray();                                         // the last lines of Memo1, golden text
        for (std::size_t i = 0; i < pg.lsMemoTail.size(); ++i) w.String(pg.lsMemoTail[i]);
        w.EndArray();
        w.Key("data").BeginArray();                                             // mmo1..mmo8
        for (int k = 0; k < 8; ++k) {
            w.BeginArray();
            for (std::size_t i = 0; i < pg.lsData[k].size(); ++i) w.String(pg.lsData[k][i]);
            w.EndArray();
        }
        w.EndArray();
        w.Key("dataCounts").BeginArray();                                       // iLogLightScaleCount_InArmX1 .. OutArmY2
        for (int k = 0; k < 8; ++k) w.Number((wb_int64)pg.lsDataCounts[k]);
        w.EndArray();
        w.Key("lastSaved").String(pg.lsLastSaved);
        w.Key("lastNote").String(pg.lsLastNote);
        w.Key("encoderSrc").String(pg.lsEncoderSrc);
        w.EndObject();
    } else {
        w.Null();
    }

    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

}  // namespace sjson
}  // namespace ht9045
