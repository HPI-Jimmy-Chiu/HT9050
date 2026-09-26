// =============================================================================
//  WebMotorAccess.cpp  --  `motor.access` 的解析與分派（純邏輯，只認 IMotorAccessBackend）
//
//  AI(W906-W4-MOTOR) 20260925.  設計與分層見 WebMotorAccess.h 檔頭，這裡不重複。
//  本檔刻意不 include 任何 god-stack 標頭：真實後端在 WebMotorAccessLive.cpp（只連進 wb_serve），
//  ctest（tests/test_web_motor_access.cpp）直接把本檔＋cJSON＋JsonWriter 編進去配假後端。
// =============================================================================
#include "WebMotorAccess.h"

#include <chrono>
#include <cmath>      // AI(W906-W5-b) 20260925: std::isfinite／floor（W5B-4 目標值驗證）
#include <cstdio>
#include <ctime>
#include <set>        // AI(W906-W5-b) 20260925: g_encBase（W5B-6）
#include <string>

#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"

namespace ht9045 {

namespace {

std::string NowIso()
{
    using namespace std::chrono;
    const system_clock::time_point now = system_clock::now();
    const std::time_t t = system_clock::to_time_t(now);
    const long ms = (long)(duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000);
    std::tm g = *std::gmtime(&t);    // 單執行緒（wb_serve 主迴圈）呼叫
    char buf[40];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                  g.tm_year + 1900, g.tm_mon + 1, g.tm_mday, g.tm_hour, g.tm_min, g.tm_sec, ms);
    return buf;
}

// ---------------------------------------------------------------------------
//  28 個 action（motor-access.json 38 個命令去重）。golden 欄 = 該按鈕事件在 golden 的位置，
//  讓每一列都能開 golden 對帳。status 見 MotorAccessActionStatus() 的說明。
// ---------------------------------------------------------------------------
struct ActionRow {
    const char* action;
    const char* status;     // "live" | "ui" | "queued"
    const char* wave;       // queued 時：哪一波接（寫進拒絕理由）
    const char* golden;
};

const ActionRow kActions[] = {
    { "jogP",                "live",   "",     "uMotorTest.cpp:862 sbMotorTest_JogPMouseDown（放開 :900）；uteach.cpp btnJogPMouseDown" },
    { "jogN",                "live",   "",     "uMotorTest.cpp:812 sbMotorTest_JogNMouseDown（.dfm 放開綁 JogPMouseUp）" },
    { "moveRelative",        "live",   "",     "uMotorTest.cpp:1224 MoveNClick／:1252 MovePClick" },
    { "moveAbsolute",        "live",   "",     "uMotorTest.cpp:1605 btnGoClick" },
    { "moveSoftLimitP",      "live",   "",     "uMotorTest.cpp:1097 btnGoSoftPClick" },
    { "moveSoftLimitN",      "live",   "",     "uMotorTest.cpp:1105 btnGoSoftNClick" },
    { "home",                "live",   "",     "uMotorTest.cpp:1113 btnHomeClick" },
    { "loopMove",            "live",   "",     "uMotorTest.cpp:1300 btnLoopMoveClick（DoLoopMove :393）" },
    { "stop",                "live",   "",     "uMotorTest.cpp:1647 btnStopClick；uteach.cpp btnStopClick" },
    { "servoToggle",         "live",   "",     "uMotorTest.cpp:1661 btnServoOffClick；uteach.cpp btnServoClick" },
    { "motorPowerToggle",    "blocked", "開電要 DoMotorPowerOn，它的三個剎車釋放在 csystem.cpp:14389 的閘 G9 裡仍是 #if 0（閘的理由「DEFINED NOWHERE」已過期：三支現在定義在 csystem.cpp:18964／:29240／:29274 —— NB2 R23 更正；解 G9 是剎車釋放，另案處理）；關電要 fHome->GaliMotorServoOff（uhome.cpp 沒翻）", "uMotorTest.cpp:1621 btnMotorPowerClick" },
    { "setPos1",             "ui",     "",     "uMotorTest.cpp:1079 btnSetPosPClick（只寫 edPos1）" },
    { "setPos2",             "ui",     "",     "uMotorTest.cpp:1088 btnSetPosNClick（只寫 edPos2）" },
    { "setJogHighSpeed",     "live",   "",     "uMotorTest.cpp:1403 btnHighSpeedClick" },
    { "setJogLowSpeed",      "live",   "",     "uMotorTest.cpp:1411 btnLowSpeedClick" },
    { "setHomeHighSpeed",    "live",   "",     "uMotorTest.cpp:1419 btnHomeHighClick" },
    { "setHomeLowSpeed",     "live",   "",     "uMotorTest.cpp:1427 btnHomeLowClick" },
    { "setSoftLimitP",       "live",   "",     "uMotorTest.cpp:1435 btnSoftPPosClick" },
    { "setSoftLimitN",       "live",   "",     "uMotorTest.cpp:1443 btnSoftNPosClick" },
    { "refreshParameter",    "ui",     "",     "uMotorTest.cpp:1451 btnRangeClick／:1458 btnRateClick（只呼叫 UpdateMotorParameter）" },
    { "setRangeAndInit",     "live",   "",     "uMotorTest.cpp:1374 btnSetRangeClick" },
    { "setRateAndInit",      "live",   "",     "uMotorTest.cpp:1364 btnSetRateClick" },
    { "reloadMotorData",     "blocked", "golden 是 InitialMotorParameter()＋每一軸 PCIL132_SetPos(0)：SetPos(0) 照 EastSun 不提供（落差清單 §3 第 7 條）；InitialMotorParameter 會對 1203 軸走 golden 開軸（InitMotor），與 EastSun 監看器的開軸規則衝突（W0 第 2 項待整合）", "uMotorTest.cpp:1695 btnReloadMotorDataClick" },
    { "resetMNet",           "live",   "",     "uMotorTest.cpp:1718 btResetMNetClick" },
    { "setTeachFromCurrent", "ui",     "",     "uteach.cpp:2098 btnSetToClick（只做 EditPtr->Text=edtNowPosition->Text，頁面自己做）" },
    { "setTeachFromOffset",  "ui",     "",     "uteach.cpp:2200 btnSetToOffsetClick（只做 EditPtr->Text=edtSetToOffset->Text，頁面自己做）" },
    { "teachSet",            "live",   "",     "uteach.cpp:3360 SetButton140Click／:3517 SetButton020Click／:3650 SetButton064Click（手動教導：關伺服→對話框→讀編碼器→開伺服）" },
    { "teachGo",             "live",   "",     "uteach.cpp:3401 GoButton140Click／:3559 GoButton020Click" },
};
const int kActionCount = (int)(sizeof(kActions) / sizeof(kActions[0]));

const ActionRow* FindAction(const std::string& a)
{
    for (int i = 0; i < kActionCount; ++i)
        if (a == kActions[i].action) return &kActions[i];
    return 0;
}

// ack 物件的共同欄位。Steven 的 motor-access-ack.json 形狀是
// {seq,id,state,result,message,motorId,position,completedAt}，這裡照那幾個名字，另加 layer 等對帳欄位。
void AckHead(webbridge::JsonWriter& w, const MotorAccessReq& r, const std::string& result,
             const std::string& layer, const std::string& message)
{
    w.Key("seq").Number((wb_int64)r.seq);
    // ⚠ 不可叫 "id"：WebBridgeServer::AckJson 把成功 ack 的物件「攤平」併進 {"type":"ack","id":<WS id>,"ok":true,…}，
    //   同名鍵在 JSON.parse 取後者 ⇒ 頁面用 m.id 找不到自己的 pending，每個成功命令都等到 15 秒逾時變錯誤。
    //   20260925 端對端探針（tools/webprobe/w4_motor_probe.py）量到才發現；type／id／ok／error 都是傳輸層保留字。
    w.Key("reqId").String(r.id);
    w.Key("state").String("done");
    w.Key("result").String(result);
    w.Key("message").String(message);
    w.Key("source").String(r.source);
    w.Key("button").String(r.button);
    w.Key("action").String(r.action);
    w.Key("motorId");
    if (r.motors.empty()) w.Null(); else w.String(r.motors[0]);
    w.Key("position").Null();
    w.Key("layer").String(layer);
    w.Key("completedAt").String(NowIso());
}

std::string Finish(webbridge::JsonWriter& w)
{
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

MotorAccessOutcome Refuse(const std::string& why)
{
    MotorAccessOutcome o;
    o.ok = false;
    o.ackJson = why;
    return o;
}

// NB2 R21 W4B-3：EastSun 自己判成功是 `issued && ret==0`（accepted 只代表通過驗證）。卡片回錯誤碼時 why 是廠商字串。
bool CmdFailed(const Pci1203CmdResult& r)
{
    return !r.accepted || (r.issued && r.ret != 0);
}
std::string CmdWhy(const Pci1203CmdResult& r)
{
    if (!r.accepted) return r.why;
    char b[48];
    std::snprintf(b, sizeof(b), "card returned 0x%08lX", r.ret);
    return r.why.empty() ? std::string(b) : std::string(b) + " " + r.why;
}

// W4B-5：每一軸最後一次下運動命令時的監看器輪詢序號。下一個運動命令要等至少一次新的輪詢（≤200 ms），
//   否則 READY／目前位置是命令之前的樣本。停止不受限。
std::map<int, unsigned long> g_issuedPoll;
void NoteIssued(IMotorAccessBackend& be, int axis) { g_issuedPoll[axis] = be.Pci1203PollCount(); }
bool SampleStale(IMotorAccessBackend& be, int axis)
{
    std::map<int, unsigned long>::const_iterator it = g_issuedPoll.find(axis);
    return it != g_issuedPoll.end() && be.Pci1203PollCount() <= it->second;
}
const char* kStaleWhy = "上一個命令之後監看器還沒更新（輪詢週期 200 ms），READY／位置還是命令前的樣本 —— 稍候再按";

//AI(W906-W5-b) 20260925: W5B-6 —— 手動教導（伺服關、手推）之後命令位置可能還停在推之前：golden TMyMotor::ServoOnOff(true) 在
//  PServoAlarmOn 的軸上 PCIL132_ResetPos（命令位置＝編碼器，mymotor.cpp:1936），EastSun 刻意不提供 Acm_AxSetCmdPosition
//  （Pci1203Control.h:222）⇒ 這些 1203 軸槽的「目前位置」（相對移動、jog 的軟體極限、setSoftLimit、畫面 cmdPos）改讀編碼器 actPos。
//  清單只增不減（這個行程結束才清）：之後的命令位置有沒有追上實際位置，要到機台量過才知道（docs/W5_PROGRESS.md §6（W5-b） 待決）。
std::set<int> g_encBase;

//AI(W906-W5-b) 20260925: 覆核 R-W5B-4 —— golden MOT[mi] 目前的 SetSpeed 百分比（只記 1203 軸；非 1203 軸 golden 的 MOT 物件自己記）。
//  golden 教導頁的 jog（TMyMotor::JogP(Speed) 不用 Speed，mymotor.cpp:1876-1888）與 MoveP／MoveN／MoveTo（uteach.cpp:2104-2131／
//  :2205-2232／:2391-2404）**都不設速度**，用的是這一軸上一次被設的速度；會設速度的是 ScrollBar1／edtSpeed 改變（:1312／:2442）、
//  選馬達（UpdateMotorTeachMonitor :1275 → 1%）、HOME 抬起（:2196 → 1%）、Go 鈕（GoButton140 → 1%、GoButton020 → 20%）、
//  以及 MotorTest 的每一次 SetSpeed（同一個 MOT 物件）。1203 軸在 EastSun 監看器開的 handle 上，卡上的 PTP 速度還會被 W4C-3 的
//  「歸零前把 PTP 速度設成歸零速度」蓋掉 ⇒ 教導頁每一次 jog／移動前，把這裡記的值重送一次（值不變，等於 golden 的「卡上是上次那個速度」）。
std::map<int, int> g_goldenPct;
int                g_teachEdt = -1;       // golden edtSpeed->Text 的鏡像（-1＝不知道；VCL Text 沒變不觸發 OnChange）
std::string        g_teachHomeUnknown;    // 最近一次 MotorAccessTeachHomeLed 回 -1 的原因（W5B-R2）

// W4B-4：正在 jog 的軸（放開／STOP 清掉；操作員連線消失時由 MotorAccessTick 停下）
struct JogRec { bool is1203; int axis; int mi; std::string motorId; };
std::vector<JogRec> g_jogs;
void ForgetJog(bool is1203, int axis, int mi)
{
    for (std::size_t i = 0; i < g_jogs.size(); )
        if (g_jogs[i].is1203 == is1203 && (is1203 ? g_jogs[i].axis == axis : g_jogs[i].mi == mi)) g_jogs.erase(g_jogs.begin() + i);
        else ++i;
}

// W4B-1：golden 真機組態下 Enable=0 的軸選不到（uMotorTest.cpp:740-743）
std::string NotSelectable(const std::string& who)
{
    return who + ": Enable=0 —— golden 真機組態下這一軸在畫面上選不到（uMotorTest.cpp:740-743 lM00Click），所有按鈕都碰不到它";
}
// NB2 R22：selectable 只是畫面層（模擬組態可選）。真正擋住 1203 軸的是 golden TMyEtherCatMotor::InitMotor
//   （myEthercatmotor.cpp:172）`if(!Enable) return true;` —— **無 #ifdef**，任何組態都不開這一軸，golden 的命令全部到不了卡。
//   EastSun 監看器開軸不看 Mot_Table 的 Enable ⇒ 1203 軸 Enable=0 一律拒絕（停止除外）。
//   ⚠ 看的是 Mot_Table 的 Enable 欄（MotorAccessAxis.tableEnable），不是 Motor->Enable：SOFT_SIMULTE 建置 golden 把每一軸的
//     Motor->Enable 都設 false（cinitial.cpp:4011-4034），看它會讓預設建置（模擬＋1203 實彈）的每一個 1203 軸都按不動 ——
//     W4-b1 的 Move1203／MoveBlock1203 原本就犯了這個錯（在模擬建置裡每個 1203 移動都回 Enable=0；筆電沒卡所以沒量到），一起改。
const char* kEnable0Why =
    "Enable=0（Mot_Table）—— 這台沒裝這一軸：golden TMyEtherCatMotor::InitMotor（myEthercatmotor.cpp:172）在任何組態都不開它，golden 的命令到不了卡（NB2 R22）";

// NB2 R23 W4C-6：golden 的 jog／相對移動／軟體極限移動鈕第一行都是
//   `if(ActiveIndex==-1 || btnHome->Down || btnLoopMove->Down) return;`（uMotorTest.cpp:815／:865／:1099／:1107／:1226／:1255）。
//   ⚠ btnGoClick（:1605）**沒有**這一行 ⇒ moveAbsolute 不擋（golden 同；軸在動時 READY 檢查會擋）。
std::string JobsBusyWhy(const std::string& who);   // 定義在 W4-b2 工作區塊

// ---------------------------------------------------------------------------
//  stop —— golden btnStopClick
//    uMotorTest.cpp:1647-1659
//        AllBtnUp();                      ← 畫面（頁面自己做）
//        StopAllMotor();                  ← GoldenStopAll
//        bSingleHome=false;               ← GoldenStopAll
//        if(ActiveIndex==-1) return;
//        ... MOT[ActiveIndex].PCIL132_StopMotor();
//    uteach.cpp btnStopClick：StopAllMotor(); Tech_Part=0; MOT[MTestY1].Gali_Command("ST"); AllBtnUp();
//
//  1203 軸（EastSun 層）：對監看器每一個開成功的軸送 kCmdAxStop（Acm_AxStopDec，EastSun 的減速停止）。
//  golden 的 StopAllMotor 是「全部馬達」，EastSun 的停止是逐軸 —— 逐軸全送就是兩者的交集，
//  不挑軸（挑軸會變成「停了畫面上選的那一軸，旁邊正在動的不停」）。
//  ⚠ 停止**不看**控制權、HomeFlag、互鎖 —— 任何情況下都要能停。唯一會讓 1203 那半不做的
//    是控制層不存在（這台筆電：沒有 SDK），ack 會照實寫出來。
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//  jog 放開 —— golden 的 MouseUp，**只停那一軸**（不是 btnStopClick 的全停）：
//    uMotorTest.cpp:900-910  sbMotorTest_JogPMouseUp（.dfm 兩顆 jog 的 OnMouseUp 都綁它）
//        if(ActiveIndex==-1 || btnHome->Down || btnLoopMove->Down) return;
//        Galil index 軸 → Gali_Command("ST")，其他 → MOT[ActiveIndex].PCIL132_StopMotor();
//    uteach.cpp btnJogPMouseUp：fTechAuto=false; 同上兩支
//  頁面照 Steven 的協定（catalog 的 release:"stop"）送 action "stop"，button 是那顆 jog 鍵 —— 用 button 分辨。
//  ⚠ golden 的 `btnHome->Down || btnLoopMove->Down` 早退不照搬：網頁端在 motion 執行中已鎖住其他按鈕，
//    而「放開卻不停」在網頁上的代價（連線延遲下軸繼續跑）比 golden 大得多。停止只會更安全。
// ---------------------------------------------------------------------------
bool IsJogButton(const std::string& b)
{
    const std::size_t n = b.size();
    return n >= 4 && (b.compare(n - 4, 4, "JogP") == 0 || b.compare(n - 4, 4, "JogN") == 0);
}

MotorAccessOutcome DoJogRelease(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (r.motors.empty()) {
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "no-motor", "none", "golden: ActiveIndex==-1 → return（沒有選馬達，不停任何軸）");
        MotorAccessOutcome o;
        o.ok = true;
        o.ackJson = Finish(w);
        return o;
    }
    MotorAccessAxis a;
    if (!be.Resolve(r.motors[0], a))
        return Refuse("jog release: 馬達表上沒有 " + r.motors[0]);
    webbridge::JsonWriter w;
    w.BeginObject();
    if (a.Is1203()) {
        std::string why;
        if (!be.Pci1203Ready(why))
            return Refuse("jog release " + r.motors[0] + "（PCI1203）: 1203 控制層不可用，無法送停止 —— " + why);
        if (a.axis < 0) {
            const std::string msg = "jog release " + r.motors[0] + "（PCI1203）: " + a.why;
            be.Pci1203NoteRefusal(wireId, "motor.access jog release", msg);
            return Refuse(msg);
        }
        Pci1203Cmd c;
        c.kind = kCmdAxStop;
        c.wireId = wireId;
        c.axis = a.axis;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        ForgetJog(true, a.axis, a.motIndex);
        if (CmdFailed(res))
            return Refuse("jog release " + r.motors[0] + ": 1203 停止失敗 —— " + CmdWhy(res));
        AckHead(w, r, "stopped", "pci1203", std::string(res.issued ? "sent: " : "accepted, NOT issued (dry): ") + res.wouldCall);
        w.Key("axis").Number((wb_int64)a.axis);
        w.Key("issued").Bool(res.issued);
    } else {
        if (!(a.motIndex >= 0 && a.motorLive))
            return Refuse("jog release " + r.motors[0] + "（" + a.cardModel + "）: 沒有 golden 馬達物件（MOT[].Motor 為 NULL）");
        be.GoldenStopMotor(a.motIndex);
        ForgetJog(false, -1, a.motIndex);
        AckHead(w, r, "stopped", "MOT", std::string("MOT[") + std::to_string(a.motIndex) + "] stop (golden JogPMouseUp)");
        w.Key("motIndex").Number((wb_int64)a.motIndex);
    }
    MotorAccessOutcome o;
    o.ok = true;
    o.ackJson = Finish(w);
    return o;
}

void CancelAllJobs(IMotorAccessBackend& be, const std::string& why);   // W4-b2（定義在下面的工作區塊）

// golden StopAllMotor 的 1203 那一半：對監看器每一個開成功的軸送 kCmdAxStop（DoStop 與教導頁 HOME 抬起共用；AI(W906-W5-b) 20260925 抽出來）
void Stop1203All(IMotorAccessBackend& be, long long wireId, int& sent, int& accepted, int& refused, std::string& firstRefusal)
{
    const int n = be.Pci1203AxisCount();
    for (int ax = 0; ax < n; ++ax) {
        if (!be.Pci1203AxisOpened(ax)) continue;
        Pci1203Cmd c;
        c.kind = kCmdAxStop;
        c.wireId = wireId;
        c.axis = ax;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        if (!CmdFailed(res)) { ++accepted; if (res.issued) ++sent; }
        else { ++refused; if (firstRefusal.empty()) firstRefusal = CmdWhy(res); }
    }
}

MotorAccessOutcome DoStop(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (IsJogButton(r.button)) return DoJogRelease(r, wireId, be);
    be.GoldenStopAll(r.source);
    CancelAllJobs(be, "STOP (golden AllBtnUp: btnHome/btnLoopMove up, bSingleHome=false)");
    g_jogs.clear();

    std::string why1203;
    const bool ready = be.Pci1203Ready(why1203);
    int sent = 0, accepted = 0, refused = 0;   // NB2 R19 RW4-1：sent＝真的呼叫了卡（issued），accepted 另計 —— dry 控制層只會 accepted
    std::string firstRefusal;
    if (ready) Stop1203All(be, wireId, sent, accepted, refused, firstRefusal);

    // 畫面上選的那一軸（golden 的 ActiveIndex 分支）。1203 軸上面已經停了；非 1203 軸走 golden 物件。
    std::string motorNote;
    MotorAccessAxis a;
    if (!r.motors.empty()) {
        if (!be.Resolve(r.motors[0], a)) {
            motorNote = "馬達表上沒有 " + r.motors[0];
        } else if (!a.Is1203()) {
            if (a.motIndex >= 0 && a.motorLive) be.GoldenStopMotor(a.motIndex);
            else motorNote = r.motors[0] + " 沒有 golden 馬達物件（MOT[].Motor 為 NULL）";
        }
    }

    char msg[256];
    if (ready)
        std::snprintf(msg, sizeof(msg), "StopAllMotor done; 1203: stop issued to %d axis(es), accepted %d%s%s",
                      sent, accepted, (accepted > sent) ? " (dry control: accepted but NOT issued)" : "",
                      refused ? "; SOME AXES REFUSED STOP" : "");
    else
        std::snprintf(msg, sizeof(msg), "StopAllMotor done; 1203 control not ready");

    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "stopped", ready ? "MOT+pci1203" : "MOT", msg);
    w.Key("pci1203").BeginObject();
    w.Key("ready").Bool(ready);
    if (!ready) w.Key("why").String(why1203);
    w.Key("stopSent").Number((wb_int64)sent);
    w.Key("stopAccepted").Number((wb_int64)accepted);
    w.Key("stopRefused").Number((wb_int64)refused);
    if (!firstRefusal.empty()) w.Key("firstRefusal").String(firstRefusal);
    w.EndObject();
    if (!motorNote.empty()) w.Key("note").String(motorNote);
    w.Key("partial").Bool(refused > 0);   // NB2 R19 RW4-2：有軸拒絕停止 ⇒ 頁面要顯示成錯誤，不是綠色 ok
    MotorAccessOutcome o;
    o.ok = true;
    o.ackJson = Finish(w);
    return o;
}

// ---------------------------------------------------------------------------
//  servoToggle —— golden
//    uMotorTest.cpp:1661-1671  btnServoOffClick：
//        MOT[ActiveIndex].ScanMotorStatus();
//        static bool bServoOn=true;  bServoOn=!bServoOn;          ← 函式內 static，所有軸共用
//        MOT[ActiveIndex].ServoOnOff(bServoOn);
//    uteach.cpp btnServoClick：MOT[ActiveMotorIndex].ServoOnOff(!MOT[ActiveMotorIndex].Led[iServoOn]);
//
//  目標狀態的決定順序：
//    1. 頁面明講（params.servoOn，Steven 的頁面用 runtime 的 servoOn 取反）
//    2. 這一軸真實的狀態取反（1203：motionIO 的 SVON 位元；其他：MOT[i].Led[iServoOn]）
//       —— uteach 的 golden 就是這樣；uMotorTest 的 static toggle 在「只有一軸」時等價
//    3. 只剩 uMotorTest 的 golden static toggle（狀態讀不到時）
//  uMotorTest 每次都把 static 設成這次的目標，所以第 3 條永遠是「跟上一次相反」，與 golden 的 static 同義。
// ---------------------------------------------------------------------------
bool g_motorTestServoStatic = true;     // golden uMotorTest.cpp:1666 `static bool bServoOn=true;`

MotorAccessOutcome DoServo(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (r.motors.empty())
        return Refuse("servoToggle: 沒有選馬達（golden: ActiveIndex==-1 → return）");
    MotorAccessAxis a;
    if (!be.Resolve(r.motors[0], a))
        return Refuse("servoToggle: 馬達表上沒有 " + r.motors[0]);
    {
        MotorGolden sg;
        const bool hasG = a.motIndex >= 0 && be.GoldenMotor(a.motIndex, sg);
        if (hasG && !sg.selectable) return Refuse(NotSelectable("servoToggle " + r.motors[0]));
        if (hasG && a.Is1203() && !a.tableEnable) return Refuse("servoToggle " + r.motors[0] + ": " + kEnable0Why);   // NB2 R22
    }
    if (r.source == "uMotorTest")                                               // NB2 R23：golden btnServoOffClick 第一行 AllBtnUp()（:1663）
        CancelAllJobs(be, "servo toggled (golden btnServoOffClick: AllBtnUp)");

    std::string why1203;
    const bool ready = a.Is1203() ? be.Pci1203Ready(why1203) : false;
    if (a.Is1203() && !ready)
        return Refuse("servoToggle " + r.motors[0] + "（PCI1203）: 1203 控制層不可用 —— " + why1203);
    if (a.Is1203() && a.axis < 0) {
        // EastSun 的規則：沒走到 Execute() 的拒絕也要記進 1203 的稽核（Pci1203Control.h NoteRefusal 的說明）
        const std::string why = "servoToggle " + r.motors[0] + "（PCI1203）: " + a.why;
        be.Pci1203NoteRefusal(wireId, "motor.access servoToggle", why);
        return Refuse(why);
    }
    if (!a.Is1203() && !(a.motIndex >= 0 && a.motorLive))
        return Refuse("servoToggle " + r.motors[0] + "（" + a.cardModel + "）: 沒有 golden 馬達物件（MOT[].Motor 為 NULL）");

    bool target = false;
    std::string basis;
    std::map<std::string, bool>::const_iterator it = r.flag.find("servoOn");
    if (it != r.flag.end()) {
        target = it->second;
        basis = "page";
    } else {
        bool known = false;
        const bool cur = a.Is1203() ? be.Pci1203ServoOn(a.axis, known)
                                    : be.GoldenServoOn(a.motIndex, known);
        if (known) {
            target = !cur;
            basis = "toggle-actual";
        } else if (r.source == "uMotorTest") {
            target = !g_motorTestServoStatic;
            basis = "golden-static";
        } else {
            return Refuse("servoToggle " + r.motors[0] + ": 讀不到目前的伺服狀態，也沒有指定 servoOn —— 不猜");
        }
    }
    if (r.source == "uMotorTest") g_motorTestServoStatic = target;

    webbridge::JsonWriter w;
    w.BeginObject();
    if (a.Is1203()) {
        Pci1203Cmd c;
        c.kind = kCmdAxSvOn;
        c.wireId = wireId;
        c.axis = a.axis;
        c.value = target ? 1.0 : 0.0;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        if (CmdFailed(res))
            return Refuse("servoToggle " + r.motors[0] + ": 1203 失敗 —— " + CmdWhy(res));
        AckHead(w, r, target ? "servoOn" : "servoOff", "pci1203",
                res.issued ? std::string("sent: ") + res.wouldCall
                           : std::string("accepted, NOT issued (1203 control is in dry mode): ") + res.wouldCall);
        w.Key("axis").Number((wb_int64)a.axis);
        w.Key("issued").Bool(res.issued);
        w.Key("wouldCall").String(res.wouldCall);
    } else {
        be.GoldenServoOnOff(a.motIndex, target);
        AckHead(w, r, target ? "servoOn" : "servoOff", "MOT",
                std::string("MOT[") + std::to_string(a.motIndex) + "].ServoOnOff(" + (target ? "true" : "false") + ")");
        w.Key("motIndex").Number((wb_int64)a.motIndex);
    }
    w.Key("servoOn").Bool(target);
    w.Key("basis").String(basis);
    MotorAccessOutcome o;
    o.ok = true;
    o.ackJson = Finish(w);
    return o;
}

// =============================================================================
//  W4-b1 運動命令（uMotorTest）—— golden 按鈕的互鎖照原順序，最後才落到 EastSun 的 1203 命令。
//  非 1203 軸（MN200 等）走 golden 的 MOT[] 方法（那是它唯一的路，互鎖在 golden 方法裡面）。
//  ⚠ 1203 軸只開放 Direction==0：golden 的 EtherCAT 馬達讀位置／極限／jog 依 Direction 翻號，絕對移動卻不翻
//    （Motor/myEthercatmotor.cpp ReadRealPos vs MoveToPos），兩種解法都不是忠於 golden ⇒ 夜間報告 §0 第 5 件等使用者。
// =============================================================================
const char* kDirPending =
    "Direction=1 的 1203 軸：golden 讀位置依 Direction 翻號、絕對移動卻不翻，方向慣例待使用者決定（夜間報告 §0 第 5 件）—— 決定前不下運動命令";

// 一軸的運動前提（motor 選了、解析得到、層可用）。成功回 true；失敗時 out.why 是拒絕理由。
struct MotionCtx {
    MotorAccessAxis a;
    MotorGolden     g;
    std::string     why;
};

bool MotionPrelude(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, MotionCtx& m, bool needPos)
{
    if (r.motors.empty()) { m.why = r.action + ": 沒有選馬達（golden: ActiveIndex==-1 → return）"; return false; }
    if (!be.Resolve(r.motors[0], m.a)) { m.why = r.action + ": 馬達表上沒有 " + r.motors[0]; return false; }
    if (m.a.motIndex < 0 || !be.GoldenMotor(m.a.motIndex, m.g)) {
        m.why = r.action + " " + r.motors[0] + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL）"; return false;
    }
    if (!m.g.selectable) { m.why = NotSelectable(r.action + " " + r.motors[0]); return false; }
    if (m.g.galilIndex) {
        m.why = r.action + " " + r.motors[0] + ": INDEX_MOTION_CARD==0 的 Index 軸走 golden 的 Galil 分支，motor.access 尚未接（HT9050 應設 INDEX_MOTION_CARD=1）";
        return false;
    }
    if (!m.a.Is1203()) return true;
    if (!m.a.tableEnable) { m.why = r.action + " " + r.motors[0] + ": " + kEnable0Why; return false; }   // NB2 R22
    std::string why;
    if (!be.Pci1203Ready(why)) { m.why = r.action + " " + r.motors[0] + "（PCI1203）: 1203 控制層不可用 —— " + why; return false; }
    if (m.a.axis < 0) {
        m.why = r.action + " " + r.motors[0] + "（PCI1203）: " + m.a.why;
        be.Pci1203NoteRefusal(wireId, "motor.access " + r.action, m.why);
        return false;
    }
    //AI(W906-DIR6B) 20260925: RULINGS_20260925 第 13 條（6B）「1203 軸不看 Direction，方向交給驅動器 Pn000」—— 不再以 Direction=1 拒絕
    return true;
}

// 目前位置（使用者單位）。golden 讀 MOT[i].ReadPos()；1203 軸改讀監看器的 cmdPos 再照 golden 換算。
bool CurrentUserPos(IMotorAccessBackend& be, const MotionCtx& m, int& pos)
{
    if (!m.a.Is1203()) { pos = be.GoldenReadPos(m.a.motIndex); return true; }
    double card = 0.0;
    if (g_encBase.count(m.a.axis)) { if (!be.Pci1203ActPos(m.a.axis, card)) return false; }   // AI(W906-W5-b) 20260925: W5B-6
    else if (!be.Pci1203CmdPos(m.a.axis, card)) return false;
    pos = MotorCardToUser(card, m.g.gearRatio);
    return true;
}

//AI(W906-W5-b) 20260925: W5B-9 —— golden MotorMovePosition（mymotor.cpp:590-593，軟體極限之後、MotionDone 之前）
//  `Position=ReadPos(); if(Position!=Tar) iLastRotatorDirP=(Tar>Position)?true:false;` —— 每一次 MotorMove 都記（沒真的下命令也記），
//  GetRotatorBacklash 拿它判斷補不補背隙。1203 軸的位置照 W4 讀監看器；讀不到就不記（golden 的 ReadPos 一定有值）。
void NoteRotatorDir(IMotorAccessBackend& be, const MotorAccessAxis& a, const MotorGolden& g, int target)
{
    MotionCtx m; m.a = a; m.g = g;
    int now = 0;
    if (CurrentUserPos(be, m, now) && now != target) be.GoldenSetRotatorLastDirP(a.motIndex, target > now);
}

bool ParamNum(const MotorAccessReq& r, const char* k, double& v)
{
    std::map<std::string, double>::const_iterator it = r.num.find(k);
    if (it == r.num.end()) return false;
    v = it->second;
    return true;
}

// golden SetSpeed(pct) 落到 1203：四個 kCmdAxSetSpeed。index 軸 golden 的 TMyMotor::SetSpeed 什麼都不做 ⇒ 不送。
bool Send1203Speed(IMotorAccessBackend& be, long long wireId, const MotionCtx& m, int pct,
                   std::string& why, std::string& note)
{
    const Motor1203Speed sp = MotorSpeedFromPct(pct, m.g);
    if (sp.skip) { note = "golden SetSpeed 不設速度（index 軸，或 PJogHighSpeed==0）"; return true; }
    const struct { Pci1203SpeedParam which; double v; } seq[4] = {
        { kSpeedInit, sp.velLow }, { kSpeedRun, sp.velHigh }, { kSpeedAcc, sp.acc }, { kSpeedDec, sp.dec } };
    for (int i = 0; i < 4; ++i) {
        Pci1203Cmd c;
        c.kind = kCmdAxSetSpeed; c.wireId = wireId; c.axis = m.a.axis; c.speed = seq[i].which; c.value = seq[i].v;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        if (CmdFailed(res)) { why = "1203 設定速度失敗 —— " + CmdWhy(res); return false; }
    }
    g_goldenPct[m.a.motIndex] = pct;                                            // AI(W906-W5-b) 20260925: R-W5B-4（golden MOT.SetSpeed 之後 MOT 記住的速度）
    char b[160];
    std::snprintf(b, sizeof(b), "speed %d%% -> s=%u velLow=%.1f velHigh=%.1f acc=%.1f dec=%.1f",
                  pct, sp.s, sp.velLow, sp.velHigh, sp.acc, sp.dec);
    note = b;
    return true;
}

MotorAccessOutcome OkAck(const MotorAccessReq& r, const std::string& result, const std::string& layer,
                         const std::string& msg, const MotionCtx& m, int target, bool hasTarget,
                         long long card, bool hasCard)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, result, layer, msg);
    if (m.a.Is1203()) w.Key("axis").Number((wb_int64)m.a.axis);
    w.Key("motIndex").Number((wb_int64)m.a.motIndex);
    if (hasTarget) w.Key("target").Number((wb_int64)target);
    if (hasCard)   w.Key("cardTarget").Number((wb_int64)card);
    MotorAccessOutcome o;
    o.ok = true;
    o.ackJson = Finish(w);
    return o;
}

// TMyMotor::MotorMove（mymotor.cpp:5906）→ MotorMovePosition（:5574）的互鎖，落到 EastSun kCmdAxMoveAbs。
MotorAccessOutcome Move1203(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be,
                            const MotionCtx& m, int target, bool hasPct, int pct)
{
    const std::string who = r.action + " " + r.motors[0];
    if (be.GoldenSafeDoorOpen(m.a.motIndex))                                    // MotorMove 第一行（-1）
        return Refuse(who + ": 安全門開著（golden CheckIsSafeDoorOpen → -1）");
    if (be.GoldenMoveLocked(m.a.motIndex)) {                                    // fCanMove*／mapLockList：golden 先停再不動
        Pci1203Cmd c; c.kind = kCmdAxStop; c.wireId = wireId; c.axis = m.a.axis;
        be.Pci1203Execute(c);
        return Refuse(who + ": 馬達被鎖（golden fCanMove*==false 或 mapLockList 非空 → PCIL132_StopMotor，不下移動）");
    }
    if (!m.a.tableEnable)                                                       // MotorMovePosition 只在 Enable 時下硬體命令（Mot_Table 的欄；見 kEnable0Why）
        return Refuse(who + ": Enable=0（Mot_Table；golden 當成沒裝這一軸，不下硬體命令）");
    char b[200];
    if (target >= m.g.softP) {                                                  // golden :5594 —— 等號也拒絕
        std::snprintf(b, sizeof(b), ": The target position of %s over positive soft limit ! 目標位置超過正向軟體極限! (%d > %d)",
                      r.motors[0].c_str(), target, m.g.softP);
        return Refuse(who + b);
    }
    if (target <= m.g.softN) {                                                  // golden :5602
        std::snprintf(b, sizeof(b), ": The target position of %s below negative soft limit ! 目標位置低於負向軟體極限! (%d <= %d)",
                      r.motors[0].c_str(), target, m.g.softN);
        return Refuse(who + b);
    }
    NoteRotatorDir(be, m.a, m.g, target);                                       // AI(W906-W5-b) 20260925: W5B-9（golden :590-593）
    if (SampleStale(be, m.a.axis)) return Refuse(who + ": " + kStaleWhy);    // NB2 R21 W4B-5
    if (!be.Pci1203AxisReady(m.a.axis))                                         // golden：MotionDone()==false 時不下命令
        return Refuse(who + ": 軸還在動或不在 READY（golden MotionDone()==false 時不下命令）");
    if (m.g.inShuttle && !be.GoldenShuttleFloodgateReady(m.g.inShuttle))       // NB2 R21 W4B-2：golden mymotor.cpp:652-697
        return Refuse(who + ": 飛梭閘門還沒開到位 —— 已下開閘命令（golden 同：這一次不動，閘門兩顆 OffSensor 到位後再按一次；超過 3 秒請查氣缸感測器）");
    std::string why, note;
    if (hasPct && !Send1203Speed(be, wireId, m, pct, why, note))
        return Refuse(who + ": " + why);
    const int card = MotorUserToCard(target, m.g.gearRatio);
    Pci1203Cmd c;
    c.kind = kCmdAxMoveAbs; c.wireId = wireId; c.axis = m.a.axis; c.value = (double)card;
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) return Refuse(who + ": 1203 移動失敗 —— " + CmdWhy(res));
    NoteIssued(be, m.a.axis);
    return OkAck(r, "moving", "pci1203",
                 std::string(res.issued ? "sent: " : "accepted, NOT issued (dry): ") + res.wouldCall +
                 (note.empty() ? "" : "; " + note),
                 m, target, true, card, true);
}

// 非 1203 軸：golden MOT.MotorMove 自己帶互鎖；回傳碼照 golden（1 到位、0 移動中、-1 安全門、-2／-3 軟體極限）。
MotorAccessOutcome MoveGolden(const MotorAccessReq& r, IMotorAccessBackend& be, const MotionCtx& m,
                              int target, bool hasPct, int pct)
{
    const int ret = be.GoldenMotorMove(m.a.motIndex, target, pct, hasPct);
    const std::string who = r.action + " " + r.motors[0];
    char b[96];
    std::snprintf(b, sizeof(b), "MOT[%d].MotorMove(%d) = %d", m.a.motIndex, target, ret);
    if (ret == -1) return Refuse(who + ": 安全門開著（" + b + "）");
    if (ret == -2) return Refuse(who + ": 目標超過正向軟體極限（" + b + "）");
    if (ret == -3) return Refuse(who + ": 目標低於負向軟體極限（" + b + "）");
    if (ret < 0)   return Refuse(who + ": golden MotorMove 回 " + std::to_string(ret) + "（" + b + "）");
    return OkAck(r, ret == 1 ? "inPosition" : "moving", "MOT", b, m, target, true, 0, false);
}

// ---- jogP／jogN：uMotorTest.cpp:862／:812（MouseDown） ----
MotorAccessOutcome DoJog(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    { const std::string busy = JobsBusyWhy(r.action); if (!busy.empty()) return Refuse(busy); }   // NB2 R23 W4C-6（:815／:865）
    MotionCtx m;
    if (!MotionPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string who = r.action + " " + r.motors[0];
    int now = 0;
    if (!CurrentUserPos(be, m, now)) return Refuse(who + ": 讀不到目前位置（golden 先看 edtCommandPos 的 999999 溢位保護）");
    if (now > 999999 || now < -999999)                                          // Steven 20100831 : 溢位保護
        return Refuse(who + ": Position over limitation! 移動位置超過限制!!");
    if (!be.GoldenSafeDoorClosed())                                             // CheckSafeDoorIsClosed()==false → return
        return Refuse(who + ": 安全門沒關（golden CheckSafeDoorIsClosed）");
    double pctD = 0.0;
    if (!ParamNum(r, "speed", pctD)) return Refuse(who + ": 缺 speed（golden SelMotSpeed=atoi(edtSpeed)）");
    const int pct = (int)pctD;
    const bool positive = (r.action == "jogP");
    if (!m.a.Is1203()) {
        be.GoldenJog(m.a.motIndex, positive, pct);                              // MOT[i].JogP/JogN（TMyMotor 內含安全門檢查）
        { JogRec j = { false, -1, m.a.motIndex, r.motors[0] }; ForgetJog(false, -1, m.a.motIndex); g_jogs.push_back(j); }
        return OkAck(r, "jogging", "MOT", std::string("MOT[") + std::to_string(m.a.motIndex) + "]." +
                     (positive ? "JogP" : "JogN") + "(" + std::to_string(pct) + ")", m, 0, false, 0, false);
    }
    if (be.GoldenSafeDoorOpen(m.a.motIndex))                                    // TMyMotor::JogP：CheckIsSafeDoorOpen() → return
        return Refuse(who + ": 安全門開著（golden TMyMotor::JogP 的 CheckIsSafeDoorOpen）");
    if (SampleStale(be, m.a.axis)) return Refuse(who + ": " + kStaleWhy);    // NB2 R21 W4B-5
    std::string why, note;
    if (!Send1203Speed(be, wireId, m, pct, why, note)) return Refuse(who + ": " + why);
    // EastSun：Acm_AxJog 在這張卡 SUCCESS 但 0 位移（web/js/pci1203/view.js 實測），改 moveVel（跑 PAR_AxVelHigh）＋放開送停止。
    Pci1203Cmd c;
    c.kind = kCmdAxMoveVel; c.wireId = wireId; c.axis = m.a.axis; c.dir = positive ? 1 : -1;   // Direction==0：JogP = 卡片正向
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) return Refuse(who + ": 1203 失敗 —— " + CmdWhy(res));
    NoteIssued(be, m.a.axis);
    { JogRec j = { true, m.a.axis, m.a.motIndex, r.motors[0] }; ForgetJog(true, m.a.axis, m.a.motIndex); g_jogs.push_back(j); }
    return OkAck(r, "jogging", "pci1203",
                 std::string(res.issued ? "sent: " : "accepted, NOT issued (dry): ") + res.wouldCall + "; " + note,
                 m, 0, false, 0, false);
}

// ---- moveRelative：MovePClick :1252／MoveNClick :1224 ----
MotorAccessOutcome DoMoveRelative(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    { const std::string busy = JobsBusyWhy(r.action); if (!busy.empty()) return Refuse(busy); }   // NB2 R23 W4C-6（:1226／:1255）
    MotionCtx m;
    if (!MotionPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string who = r.action + " " + r.motors[0];
    int now = 0;
    if (!CurrentUserPos(be, m, now)) return Refuse(who + ": 讀不到目前位置");
    if (now > 999999 || now < -999999) return Refuse(who + ": Position over limitation! 移動位置超過限制!!");
    double iv = 0.0, pctD = 0.0;
    if (!ParamNum(r, "interval", iv)) return Refuse(who + ": 缺 interval（golden cbbInterval）");
    const bool hasPct = ParamNum(r, "speed", pctD);
    // golden：MoveP → ReadPos()+Pos、MoveN → ReadPos()-Pos，Pos=cbbInterval（正數）。頁面 MoveN 送負的 interval ⇒ 依按鈕取絕對值。
    const int step = (int)(iv < 0 ? -iv : iv);
    const bool negative = (r.button.size() >= 5 && r.button.compare(r.button.size() - 5, 5, "MoveN") == 0);
    const int target = negative ? now - step : now + step;
    if (!m.a.Is1203()) return MoveGolden(r, be, m, target, hasPct, (int)pctD);
    return Move1203(r, wireId, be, m, target, hasPct, (int)pctD);
}

// ---- moveAbsolute：btnGoClick :1605 ----
MotorAccessOutcome DoMoveAbsolute(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    MotionCtx m;
    if (!MotionPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string who = r.action + " " + r.motors[0];
    if (!be.GoldenMotorCanRun()) return Refuse(who + ": EMG Stop（golden IsMotorCanRun(true)）");
    if (m.g.homeFlag == 0) return Refuse(who + ": Motor not home 馬達尚未歸零");
    double t = 0.0, pctD = 0.0;
    if (!ParamNum(r, "targetPos", t)) return Refuse(who + ": 缺 targetPos（golden edtHomeOffset）");
    const bool hasPct = ParamNum(r, "speed", pctD);
    if (!m.a.Is1203()) return MoveGolden(r, be, m, (int)t, hasPct, (int)pctD);
    return Move1203(r, wireId, be, m, (int)t, hasPct, (int)pctD);
}

// ---- moveSoftLimitP／N：btnGoSoftPClick :1097／btnGoSoftNClick :1105 —— 目標取 C++ 的 PSoftLimitP／N，不取頁面值 ----
//   ⚠ golden 的 MotorMovePosition 用 `Tar>=PSoftLimitP` 拒絕，所以「移到正向軟體極限」在 Enable 的軸上**一定被拒**（golden 同，照翻）。
MotorAccessOutcome DoMoveSoftLimit(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    { const std::string busy = JobsBusyWhy(r.action); if (!busy.empty()) return Refuse(busy); }   // NB2 R23 W4C-6（:1099／:1107）
    MotionCtx m;
    if (!MotionPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    double pctD = 0.0;
    const bool hasPct = ParamNum(r, "speed", pctD);
    const int target = (r.action == "moveSoftLimitP") ? m.g.softP : m.g.softN;
    if (!m.a.Is1203()) return MoveGolden(r, be, m, target, hasPct, (int)pctD);
    return Move1203(r, wireId, be, m, target, hasPct, (int)pctD);
}

// ---- setJogHighSpeed 等：btnHighSpeedClick :1403 … btnSoftNPosClick :1443 ----
//   golden：PJogHighSpeed=Motor->ReadSpeed()（上一次 SetSpeed 的 s）；PSoftLimitP=MOT.ReadPos()。只改記憶體（存檔走資料庫分頁）。
MotorAccessOutcome DoSetParam(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    MotionCtx m;
    const bool isSoft = (r.action == "setSoftLimitP" || r.action == "setSoftLimitN");
    // 速度參數不牽涉方向；軟體極限要讀位置 ⇒ Direction 慣例同樣要先決定
    if (r.motors.empty()) return Refuse(r.action + ": 沒有選馬達（golden: ActiveIndex==-1 → return）");
    if (!be.Resolve(r.motors[0], m.a)) return Refuse(r.action + ": 馬達表上沒有 " + r.motors[0]);
    if (m.a.motIndex < 0 || !be.GoldenMotor(m.a.motIndex, m.g))
        return Refuse(r.action + " " + r.motors[0] + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL）");
    const std::string who = r.action + " " + r.motors[0];
    if (!m.g.selectable) return Refuse(NotSelectable(who));
    MotorParamWhich which = kParamJogHigh;
    int value = 0;
    if (isSoft) {
        if (m.a.Is1203()) {
            std::string why;
            //AI(W906-DIR6B) 20260925: RULINGS_20260925 第 13 條（6B）「1203 軸不看 Direction，方向交給驅動器 Pn000」—— 不再以 Direction=1 拒絕
            if (!be.Pci1203Ready(why)) return Refuse(who + "（PCI1203）: 1203 控制層不可用，讀不到位置 —— " + why);
            if (m.a.axis < 0) return Refuse(who + "（PCI1203）: " + m.a.why);
        }
        if (!CurrentUserPos(be, m, value)) return Refuse(who + ": 讀不到目前位置（golden 取 MOT.ReadPos()）");
        which = (r.action == "setSoftLimitP") ? kParamSoftP : kParamSoftN;
    } else {
        double pctD = 0.0;
        if (!ParamNum(r, "speed", pctD)) return Refuse(who + ": 缺 speed（golden 取 Motor->ReadSpeed()＝上一次 SetSpeed(edtSpeed) 的值）");
        const Motor1203Speed sp = MotorSpeedFromPct((int)pctD, m.g);
        value = (int)sp.s;
        // NB2 R21 W4B-6：Index 四軸 golden TMyMotor::SetSpeed 整段空白，Motor->ReadSpeed() 回的是原本的速度（不是 0）
        if (m.g.indexMotor) value = (int)be.GoldenReadSpeed(m.a.motIndex);
        if      (r.action == "setJogHighSpeed")  which = kParamJogHigh;
        else if (r.action == "setJogLowSpeed")   which = kParamJogLow;
        else if (r.action == "setHomeHighSpeed") which = kParamHomeHigh;
        else                                     which = kParamHomeLow;
    }
    be.GoldenSetParam(m.a.motIndex, which, value);
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "set", "MOT", std::string("MOT[") + std::to_string(m.a.motIndex) + "].Motor 參數已改（只在記憶體；golden 同，存檔走資料庫分頁）");
    w.Key("motIndex").Number((wb_int64)m.a.motIndex);
    w.Key("value").Number((wb_int64)value);
    MotorAccessOutcome o;
    o.ok = true;
    o.ackJson = Finish(w);
    return o;
}

// =============================================================================
//  W4-b2（uMotorTest）：HOME、LoopMove、ResetMNet、Range／Rate —— 跨拍的工作由 MotorAccessTick() 推進
//  （wb_serve 每 500 ms 拍子呼叫一次，同 golden Timer1Timer 的角色）。
// =============================================================================
struct HomeJob {
    bool        active;
    std::string motorId;
    int         mi, axis;
    MotorGolden g;
    int         phase;        // 0 = 等 DS402 歸零完成；1 = arm Z 去 ZSafePos（golden ProcessSingleMotorHome case 500）
    bool        sawHoming;
    int         elapsedMs, readyMs, zTarget;
    bool        zIssued;      // case 500 的 MotorMove 已下過命令（golden fCMD）
    HomeJob() : active(false), mi(-1), axis(-1), phase(0), sawHoming(false), elapsedMs(0), readyMs(0), zTarget(0), zIssued(false) {}
};
struct LoopJob {
    bool        active;
    std::string motorId;
    MotorAccessAxis a;
    MotorGolden g;
    int         pos1, pos2, waitMs;
    int         task;         // 1 = 去 pos1、10 = 等、50 = 去 pos2、60 = 等（golden DoLoopMove 的 Task 值）
    bool        cmdIssued;
    int         legMs, waitLeft;
    unsigned long count;
    std::string lastWhy;
    LoopJob() : active(false), pos1(0), pos2(0), waitMs(0), task(1), cmdIssued(false), legMs(0), waitLeft(0), count(0) {}
};
std::vector<HomeJob> g_homes;
LoopJob              g_loop;
std::string          g_lastJobNote;

const int kHomeTimeoutMs      = 180000;   // DS402 歸零上限。⚠ NB2 R23 更正：golden 有逾時（MotorInitial 的 RESET_TIMES＝90 s，uhome.cpp:515-533 重試一次後停軸報錯），不是「沒有」；這裡 180 s ≈ 90 s×2
const int kHomeNoHomingMs     = 5000;     // 命令被接受卻一直沒進 HOMING ⇒ 當失敗（安全方向：HomeFlag 不會被誤設 1）
const int kZSafeTimeoutMs     = 60000;
const int kLoopLegTimeoutMs   = 120000;

bool IsHomingState(unsigned st) { return (st & 0xFFu) == 4u; }   // STA_AX_HOMING（AdvMotDrv.h:796）
bool IsReadyState(unsigned st)  { return (st & 0xFFu) == 1u; }   // STA_AX_READY
bool IsErrorState(unsigned st)  { return (st & 0xFFu) == 3u; }   // STA_AX_ERROR_STOP

void CancelHome(IMotorAccessBackend& be, HomeJob& h, bool sendStop, const std::string& why)
{
    if (!h.active) return;
    if (sendStop) { Pci1203Cmd c; c.kind = kCmdAxStop; c.axis = h.axis; be.Pci1203Execute(c); }
    h.active = false;
    g_lastJobNote = "home " + h.motorId + ": " + why;
}
void CancelLoop(IMotorAccessBackend& be, bool sendStop, const std::string& why)
{
    if (!g_loop.active) return;
    if (sendStop) {
        if (g_loop.a.Is1203()) { Pci1203Cmd c; c.kind = kCmdAxStop; c.axis = g_loop.a.axis; be.Pci1203Execute(c); }
        else be.GoldenStopMotor(g_loop.a.motIndex);
    }
    g_loop.active = false;
    g_loop.lastWhy = why;
    g_lastJobNote = "loop " + g_loop.motorId + ": " + why;
}
// golden btnStopClick 的 AllBtnUp()（btnHome／btnLoopMove 抬起）＋ bSingleHome=false：取消所有跨拍工作（軸已被 StopAllMotor 停下）
void CancelAllJobs(IMotorAccessBackend& be, const std::string& why)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) CancelHome(be, g_homes[i], false, why);
    CancelLoop(be, false, why);
}

HomeJob* FindHome(int axis)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i)
        if (g_homes[i].active && g_homes[i].axis == axis) return &g_homes[i];
    return 0;
}

bool AnyJobActive()
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (g_homes[i].active) return true;
    return g_loop.active;
}

std::string JobsBusyWhy(const std::string& who)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i)
        if (g_homes[i].active)
            return who + ": HOME 進行中（" + g_homes[i].motorId + "）—— golden 每個運動鈕第一行 `if(... btnHome->Down || btnLoopMove->Down) return;`；先按 HOME 停止或 STOP";
    if (g_loop.active)
        return who + ": LoopMove 進行中（" + g_loop.motorId + "）—— golden 每個運動鈕第一行 `if(... btnHome->Down || btnLoopMove->Down) return;`；先按 LoopMove 停止或 STOP";
    return std::string();
}

// 1203 的 MotorMove 互鎖（不回 ack，只回拒絕理由；空字串 = 可以下命令）。Move1203 與 LoopMove 共用。
std::string MoveBlock1203(IMotorAccessBackend& be, const MotorAccessAxis& a, const MotorGolden& g,
                          const std::string& who, int target)
{
    char b[200];
    if (be.GoldenSafeDoorOpen(a.motIndex)) return who + ": 安全門開著（golden CheckIsSafeDoorOpen → -1）";
    if (be.GoldenMoveLocked(a.motIndex))   return who + ": 馬達被鎖（golden fCanMove*==false 或 mapLockList 非空）";
    if (!a.tableEnable)                     return who + ": Enable=0（Mot_Table；golden 當成沒裝這一軸，不下硬體命令）";
    if (target >= g.softP) {
        std::snprintf(b, sizeof(b), ": over positive soft limit 目標位置超過正向軟體極限! (%d > %d)", target, g.softP);
        return who + b;
    }
    if (target <= g.softN) {
        std::snprintf(b, sizeof(b), ": below negative soft limit 目標位置低於負向軟體極限! (%d <= %d)", target, g.softN);
        return who + b;
    }
    if (!be.Pci1203AxisReady(a.axis))       return who + ": 軸不在 READY（golden MotionDone()==false 時不下命令）";
    if (g.inShuttle && !be.GoldenShuttleFloodgateReady(g.inShuttle))            // NB2 R21 W4B-2
        return who + ": 飛梭閘門還沒開到位（已下開閘命令）";
    return std::string();
}

// ---- home：btnHomeClick :1113（toggle）＋ golden ProcessSingleMotorHome（uhome.cpp:415）----
//   EastSun：Acm_AxHome(ax, 124／128, ±1)（DS402 方法 24／28 = /Home 開關、正／反向起跑）；卡片 mode 在這台回 0x8000510F。
//   方向：golden `if(HomeDirection) Acm_AxMoveHome(MODE12, 0 正向) else (…, 1 負向)` ⇒ HomeDirection=1 → 124／+1，=0 → 128／−1
//   （兩邊都是驅動器座標；EastSun 按鈕上的「往負／正方向」是操作員看機構的說法，web/js/pci1203/view.js 的 HOMEDIR-1 註解）。
//   ⚠ 照 EastSun 不做 golden 歸零完成後的 SetCommand(0)／SetPosition(0)：DS402 歸零由驅動器自己設原點。
MotorAccessOutcome DoHome(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    CancelLoop(be, false, "home pressed (golden btnHomeClick :1115: btnLoopMove->Down=false)");   // golden 第一行，在任何檢查之前
    // NB2 R23 W4C-5：golden 的 btnHome->Down **就是**狀態；網頁上頁面與伺服端會分岔（伺服端自己完成／失敗不通知頁面），
    //   「再按一次」就可能從「停」變成「開始」。⇒ 頁面明講要的狀態 start=true／false（= 按下後 btnHome->Down 的值），伺服端照做並回 homeActive。
    std::map<std::string, bool>::const_iterator sf = r.flag.find("start");
    if (sf == r.flag.end())
        return Refuse(r.action + ": 頁面版本太舊 —— home 要帶 start=true／false（按下後 btnHome->Down 的值；NB2 R23 W4C-5：切換式會讓「停」變「開始」）");
    const bool wantStart = sf->second;
    MotionCtx m;
    if (!MotionPrelude(r, wireId, be, m, false)) return Refuse(m.why);
    const std::string who = r.action + " " + r.motors[0];
    if (m.g.scaleMotor)
        return Refuse(who + ": 磁性尺軸 golden 直接 ResetPos(0)＋HomeFlag=1 —— EastSun 不提供重設座標（落差清單 §3 第 7 條），不做");
    if (!m.a.Is1203())
        return Refuse(who + "（" + m.a.cardModel + "）: 非 1203 軸的單軸歸零走 golden ProcessSingleMotorHome（uhome.cpp:415），移植樹那支是樁（acatchtray_shims.cpp:134 回 true）—— 缺相依");
    HomeJob* running = FindHome(m.a.axis);
    if (!wantStart) {                                                           // golden else 支（:1166）：bSingleHome=false; PCIL132_StopMotor();（無條件停）
        if (running) CancelHome(be, *running, true, "home released (golden: bSingleHome=false; PCIL132_StopMotor)");
        else { Pci1203Cmd c; c.kind = kCmdAxStop; c.wireId = wireId; c.axis = m.a.axis; be.Pci1203Execute(c); }
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "homeStopped", "pci1203", running ? "HOME 抬起：停止歸零（golden else 支）" : "HOME 抬起：沒有進行中的歸零，照 golden 送停止");
        w.Key("homeActive").Bool(false);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    if (running) {                                                              // 已在歸零：不重下命令
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "homing", "pci1203", "已在歸零中（不重下命令）");
        w.Key("homeActive").Bool(true);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    be.GoldenSetHomeFlag(m.a.motIndex, 0);                                      // golden :1158 一按就 HomeFlag=0（NB2 R23 §3：原本命令被接受後才清）
    // NB2 R23 W4C-1：golden TMyMotor::Home（mymotor.cpp:1638-1642）／MotorHome（:1661-1665）第一行
    //   `if(Motor==NULL || Motor->CheckIsSafeDoorOpen()) return …;` —— 門開著不歸零（golden 停在 case 300 等門關；網頁上改成拒絕，再按一次）
    if (be.GoldenSafeDoorOpen(m.a.motIndex))
        return Refuse(who + ": 安全門開著（golden TMyMotor::Home 的 CheckIsSafeDoorOpen）—— HomeFlag 已照 golden 清成 0，關門後再按 HOME");
    //AI(W906-DIR6B) 20260925: RULINGS_20260925 第 13 條（6B）「1203 軸不看 Direction，方向交給驅動器 Pn000」—— 不再以 Direction=1 拒絕
    //   （原本：arm Z 歸零後 golden 要去 ZSafePos 的絕對移動，Direction=1 時拒絕）
    if (SampleStale(be, m.a.axis)) return Refuse(who + ": " + kStaleWhy);    // NB2 R21 W4B-5
    // NB2 R23 W4C-3：golden TMyMotor::Home SetADCRate(100)（:1647「Debug Find Home…剎車距離太長而撞機」）→ HomeObject →
    //   TMyEtherCatMotor::SetHomeSpeed（myEthercatmotor.cpp:1703）：HomeVelLow／High＝PHomeLowSpeed／PHomeHighSpeed、HomeAcc／Dec＝dAcc／dDec。
    //   EastSun 量過：DS402 的 Acm_AxHome 會拿卡上**當下的 PTP** VelHigh／VelLow／Acc 去填 6099h:1／6099h:2／609Ah（EtherCAT/Pci1203Gear.h:503-506），
    //   不讀 PAR_AxHomeVel* ⇒ 同義做法是在 home 之前把 PTP 速度設成歸零速度（否則剛 jog 100% 的軸會用 jog 高速找原點）。
    if (m.g.homeHigh == 0)
        return Refuse(who + ": Home High Speed（PHomeHighSpeed）是 0 —— 不拿卡上殘留的速度去找原點（NB2 R23 W4C-3）");
    {
        const struct { Pci1203SpeedParam which; double v; } seq[4] = {
            { kSpeedInit, (double)m.g.homeLow }, { kSpeedRun, (double)m.g.homeHigh }, { kSpeedAcc, m.g.accDb }, { kSpeedDec, m.g.decDb } };
        for (int i = 0; i < 4; ++i) {
            Pci1203Cmd c;
            c.kind = kCmdAxSetSpeed; c.wireId = wireId; c.axis = m.a.axis; c.speed = seq[i].which; c.value = seq[i].v;
            const Pci1203CmdResult res = be.Pci1203Execute(c);
            if (CmdFailed(res)) return Refuse(who + ": 1203 設定歸零速度失敗 —— " + CmdWhy(res));
        }
    }
    Pci1203Cmd c;
    c.kind = kCmdAxHome; c.wireId = wireId; c.axis = m.a.axis;
    c.homeMode = m.g.homeDirection ? 124 : 128;
    c.dir      = m.g.homeDirection ? 1 : -1;
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) return Refuse(who + ": 1203 歸零失敗 —— " + CmdWhy(res));
    NoteIssued(be, m.a.axis);
    HomeJob h;
    h.active = true; h.motorId = r.motors[0]; h.mi = m.a.motIndex; h.axis = m.a.axis; h.g = m.g;
    h.zTarget = be.GoldenZSafePos();
    bool placed = false;
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (!g_homes[i].active) { g_homes[i] = h; placed = true; break; }
    if (!placed) g_homes.push_back(h);
    char sp[160];
    std::snprintf(sp, sizeof(sp), "home speed velLow=%u velHigh=%u acc=%.1f dec=%.1f", m.g.homeLow, m.g.homeHigh, m.g.accDb, m.g.decDb);
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "homing", "pci1203",
            std::string(res.issued ? "sent: " : "accepted, NOT issued (dry): ") + res.wouldCall +
            "; " + sp + "; HomeFlag=0，完成後由主迴圈設 1（失敗設 2）");
    w.Key("axis").Number((wb_int64)m.a.axis);
    w.Key("homeMode").Number((wb_int64)c.homeMode);
    w.Key("dir").Number((wb_int64)c.dir);
    w.Key("homeActive").Bool(true);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- loopMove：btnLoopMoveClick :1300（toggle）＋ DoLoopMove :393（Timer1 每拍）——單軸模式（select->ItemIndex==0）----
MotorAccessOutcome DoLoopMove(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    // NB2 R23 W4C-5：頁面明講要的狀態（按下後 btnLoopMove->Down 的值），伺服端照做並回 loopActive —— 不再切換式
    std::map<std::string, bool>::const_iterator sf = r.flag.find("start");
    if (sf == r.flag.end())
        return Refuse(r.action + ": 頁面版本太舊 —— loopMove 要帶 start=true／false（按下後 btnLoopMove->Down 的值；NB2 R23 W4C-5：切換式會讓「停」變「開始」）");
    if (!sf->second) {                                                          // golden 抬起 → PCIL132_StopMotor
        const bool was = g_loop.active;
        const std::string id = g_loop.motorId;
        if (was) CancelLoop(be, true, "loop released (golden btnLoopMoveClick: PCIL132_StopMotor)");
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "loopStopped", g_loop.a.Is1203() ? "pci1203" : "MOT",
                was ? "LoopMove 抬起：停止（" + id + "）" : std::string("LoopMove 抬起：伺服端沒有進行中的來回（可能已自己停下：") + g_loop.lastWhy + "）");
        w.Key("loopCount").Number((wb_int64)g_loop.count);
        w.Key("loopActive").Bool(false);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    if (g_loop.active) {                                                        // 已在來回：不重開
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "looping", g_loop.a.Is1203() ? "pci1203" : "MOT", "LoopMove 已在進行（" + g_loop.motorId + "，不重開）");
        w.Key("loopActive").Bool(true);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    { std::map<std::string, double>::const_iterator md = r.num.find("mode");       // golden select->ItemIndex：0＝單軸、1＝All
      if (md != r.num.end() && md->second != 0.0)
          return Refuse(r.action + ": 「All」模式（golden DoLoopMove select->ItemIndex!=0，全部軸輪流）還沒接 —— 只接單軸（NB2 R23 §3：原本靜默當單軸還回成功）"); }
    MotionCtx m;
    if (!MotionPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string who = r.action + " " + r.motors[0];
    double p1 = 0, p2 = 0, wt = 0, pct = 0;
    if (!ParamNum(r, "pos1", p1) || !ParamNum(r, "pos2", p2)) return Refuse(who + ": 缺 pos1／pos2（golden edPos1／edPos2）");
    ParamNum(r, "waitTime", wt);
    const bool hasPct = ParamNum(r, "speed", pct);
    std::map<std::string, bool>::const_iterator cf = r.flag.find("confirmNotHomed");
    if (m.g.homeFlag == 0 && (cf == r.flag.end() || !cf->second))            // golden :1317 MessageDlg YES/NO
        return Refuse(who + ": Motor not home yet, sure to loop test? (馬達尚未歸零，確定要執行？) —— 頁面確認後帶 confirmNotHomed 再送");
    for (std::size_t i = 0; i < g_homes.size(); ++i)                            // golden：btnLoopMoveClick 先 btnHome->Down=false
        if (g_homes[i].active) CancelHome(be, g_homes[i], true, "loop pressed (golden btnHome->Down=false)");
    if (m.a.Is1203() && hasPct) {
        std::string why, note;
        if (!Send1203Speed(be, wireId, m, (int)pct, why, note)) return Refuse(who + ": " + why);
    }
    g_loop = LoopJob();
    g_loop.active = true; g_loop.motorId = r.motors[0]; g_loop.a = m.a; g_loop.g = m.g;
    g_loop.pos1 = (int)p1; g_loop.pos2 = (int)p2;
    const int w01 = (int)wt;
    g_loop.waitMs = (w01 > 100 || w01 <= 0) ? 0 : w01 * 100;                  // golden Set0_1SecAndOn(Wait)；>100 或 <=0 不等
    g_loop.task = 1;
    if (!m.a.Is1203()) {                                                        // golden 第一拍：MOT.MotorMove(pos1)（速度＝edtSpeed）
        const int ret0 = be.GoldenMotorMove(m.a.motIndex, g_loop.pos1, (int)pct, hasPct);
        if (ret0 < 0) { g_loop.active = false; return Refuse(who + ": golden MotorMove(pos1) 回 " + std::to_string(ret0) + "（安全門／軟體極限）"); }
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "looping", m.a.Is1203() ? "pci1203" : "MOT",
            "LoopMove 開始：pos1→pos2 由主迴圈推進（再按一次或 STOP 停止）");
    w.Key("pos1").Number((wb_int64)g_loop.pos1);
    w.Key("pos2").Number((wb_int64)g_loop.pos2);
    w.Key("waitMs").Number((wb_int64)g_loop.waitMs);
    w.Key("loopActive").Bool(true);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- resetMNet：btResetMNetClick :1718 —— YES/NO（頁面 confirm）且 SystemStart==false 才 ResetMNet(0,…) ----
MotorAccessOutcome DoResetMNet(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    std::map<std::string, bool>::const_iterator cf = r.flag.find("confirm");
    if (cf == r.flag.end() || !cf->second)
        return Refuse("resetMNet: 需要確認（golden ShowMyMessageBox_YES_NO \"Do you want to Reset MNet?\"）");
    if (be.GoldenSystemStart())
        return Refuse("resetMNet: SystemStart==true，golden 不重置");
    be.GoldenResetMNet();
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "reset", "MOT", "ResetMNet(0, \"MNet斷電\", \"Power Off\", false)（golden :1723）");
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- setRangeAndInit／setRateAndInit：btnSetRangeClick :1374／btnSetRateClick :1364 ----
//   golden：SetRange/SetRate → Motor->InitMotor(Address) → HomeFlag=0。1203 軸的 InitMotor 走 golden 開軸
//   （Acm_AxOpenbyID，m_Axishand[999]），與 EastSun 監看器的開軸規則衝突（W0 第 2 項待整合）⇒ 1203 軸拒絕。
MotorAccessOutcome DoRangeRate(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    if (r.motors.empty()) return Refuse(r.action + ": 沒有選馬達（golden: ActiveIndex==-1 → return）");
    MotionCtx m;
    if (!be.Resolve(r.motors[0], m.a)) return Refuse(r.action + ": 馬達表上沒有 " + r.motors[0]);
    if (m.a.motIndex < 0 || !be.GoldenMotor(m.a.motIndex, m.g))
        return Refuse(r.action + " " + r.motors[0] + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL）");
    const std::string who = r.action + " " + r.motors[0];
    CancelLoop(be, false, "range/rate set (golden :1366／:1376 第一行 btnLoopMove->Down=false)");   // NB2 R23：golden 在任何檢查之前
    if (!m.g.selectable) return Refuse(NotSelectable(who));
    if (m.a.Is1203())
        return Refuse(who + "（PCI1203）: golden 接著呼叫 InitMotor（Acm_AxOpenbyID 重新開軸），與 EastSun 監看器的開軸規則衝突（W0 第 2 項待整合）—— 不做");
    double v = 0;
    if (!ParamNum(r, "value", v)) return Refuse(who + ": 缺 value（golden edtMotorRange／edtMotorRate）");
    be.GoldenSetRangeRate(m.a.motIndex, r.action == "setRangeAndInit", (int)v);
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "set", "MOT", std::string("MOT[") + std::to_string(m.a.motIndex) + "] " +
            (r.action == "setRangeAndInit" ? "SetRange" : "SetRate") + "(" + std::to_string((int)v) + ")＋InitMotor＋HomeFlag=0");
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

void TickHomes(IMotorAccessBackend& be, int ms)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) {
        HomeJob& h = g_homes[i];
        if (!h.active) continue;
        h.elapsedMs += ms;
        unsigned st = 0;
        if (!be.Pci1203AxisState(h.axis, st)) {
            if (h.elapsedMs > kHomeNoHomingMs) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, true, "axis state unreadable -> HomeFlag=2"); }
            continue;
        }
        if (IsErrorState(st)) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, false, "ERROR_STOP -> HomeFlag=2"); continue; }
        if (h.phase == 0) {
            if (IsHomingState(st)) { h.sawHoming = true; continue; }
            if (IsReadyState(st)) {
                if (!h.sawHoming) {
                    h.readyMs += ms;
                    if (h.readyMs >= kHomeNoHomingMs) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, false, "never entered HOMING -> HomeFlag=2"); }
                    continue;
                }
                //AI(W906-W5-b) 20260925: 覆核 R-W5B-5 —— golden TMyMotor::MotorHome case 20 歸零完成（mymotor.cpp:1721）`iLastRotatorDirP=true;`
                //  —— 在 ProcessSingleMotorHome 的 case 500（arm Z 去 ZSafePos）之前，所以每一軸都設（旋轉站的背隙補償讀它，:2012-2045）
                be.GoldenSetRotatorLastDirP(h.mi, true);
                if (h.g.armZ) {                                                 // golden ProcessSingleMotorHome case 500：MotorMove(ZSafePos)（下一段）
                    h.phase = 1; h.elapsedMs = 0; h.zIssued = false;
                    continue;
                }
                be.GoldenSetHomeFlag(h.mi, 1);
                CancelHome(be, h, false, "home done -> HomeFlag=1");
                continue;
            }
            if (h.elapsedMs > kHomeTimeoutMs) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, true, "home timeout -> HomeFlag=2"); }
        } else {
            // NB2 R23 W4C-2：golden uhome.cpp:597-603 case 500 `if(MOT[Index].MotorMove(ZSafePos)) { HomeFlag=true; … }`，每拍重呼叫。
            //   MotorMove 的互鎖（mymotor.cpp:5906 起）與它的回傳值照翻：
            //     安全門開 → -1、軟體極限 → -2／-3：`if(非 0)` 成立 ⇒ **不移動、HomeFlag=true**（golden 自己的寫法，照翻；Z 停在原點，已歸零）
            //     被鎖（fCanMove*／mapLockList）→ 停軸，到位回 1 否則 0 ⇒ 等；MotionDone()==false → 0 ⇒ 等；到位 → 1 ⇒ HomeFlag=true
            MotorGolden g;
            if (be.GoldenMotor(h.mi, g)) h.g = g;                               // 互鎖用即時值
            if (be.GoldenSafeDoorOpen(h.mi) || h.zTarget >= h.g.softP || h.zTarget <= h.g.softN) {
                be.GoldenSetHomeFlag(h.mi, 1);
                CancelHome(be, h, false, "case 500: golden MotorMove(ZSafePos) returned -1/-2/-3 (door open or soft limit) -> no move, HomeFlag=1 (golden `if(MotorMove())` treats it as done)");
                continue;
            }
            if (be.GoldenMoveLocked(h.mi)) {                                    // golden：PCIL132_StopMotor 然後回 0（除非剛好在目標）
                Pci1203Cmd sc; sc.kind = kCmdAxStop; sc.axis = h.axis; be.Pci1203Execute(sc);
                h.zIssued = false;
            } else if (!h.zIssued) {
                if (IsReadyState(st) && !SampleStale(be, h.axis)) {             // golden MotionDone()==false → 回 0 等
                    { MotorAccessAxis za; za.motIndex = h.mi; za.cardModel = "PCI1203"; za.axis = h.axis; NoteRotatorDir(be, za, h.g, h.zTarget); }   // AI(W906-W5-b) 20260925: W5B-9
                    Pci1203Cmd c; c.kind = kCmdAxMoveAbs; c.axis = h.axis; c.value = (double)MotorUserToCard(h.zTarget, h.g.gearRatio);
                    const Pci1203CmdResult res = be.Pci1203Execute(c);
                    if (CmdFailed(res)) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, false, "ZSafePos move failed: " + CmdWhy(res)); continue; }
                    NoteIssued(be, h.axis);
                    h.zIssued = true;
                }
                if (h.elapsedMs > kZSafeTimeoutMs) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, true, "ZSafePos timeout -> HomeFlag=2"); }
                continue;
            }
            double card = 0;
            if (IsReadyState(st) && !SampleStale(be, h.axis) && be.Pci1203CmdPos(h.axis, card)) {
                const int pos = MotorCardToUser(card, h.g.gearRatio);
                if (pos - h.zTarget <= 2 && h.zTarget - pos <= 2) {
                    be.GoldenSetHomeFlag(h.mi, 1);                              // golden case 500：HomeFlag=true
                    CancelHome(be, h, false, "home done, Z at ZSafePos -> HomeFlag=1");
                    continue;
                }
            }
            if (h.elapsedMs > kZSafeTimeoutMs) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, true, "ZSafePos timeout -> HomeFlag=2"); }
        }
    }
}

void TickLoop(IMotorAccessBackend& be, int ms)
{
    LoopJob& L = g_loop;
    if (!L.active) return;
    if (L.task == 10 || L.task == 60) {
        L.waitLeft -= ms;
        if (L.waitLeft <= 0) { L.task = (L.task == 10) ? 50 : 1; L.cmdIssued = false; }
        return;
    }
    const int target = (L.task == 1) ? L.pos1 : L.pos2;
    const std::string who = "loopMove " + L.motorId;
    bool arrived = false;
    if (!L.a.Is1203()) {
        const int ret = be.GoldenMotorMove(L.a.motIndex, target, 0, false);    // golden：`if(MOT[ActiveIndex].MotorMove(p))`
        if (ret < 0) { CancelLoop(be, false, "golden MotorMove returned " + std::to_string(ret) + " (refused) -> loop stopped"); return; }
        arrived = (ret == 1);
    } else {
        if (!L.cmdIssued) {
            if (SampleStale(be, L.a.axis)) return;                              // NB2 R23 §2：每一腿也要等一次新的輪詢（W4B-5）
            MotorGolden g;
            if (be.GoldenMotor(L.a.motIndex, g)) L.g = g;                      // 互鎖用即時值（軟體極限可能剛被改）
            const std::string why = MoveBlock1203(be, L.a, L.g, who, target);
            if (!why.empty()) { CancelLoop(be, false, why + " -> loop stopped"); return; }
            NoteRotatorDir(be, L.a, L.g, target);                               // AI(W906-W5-b) 20260925: W5B-9（golden MotorMove → MotorMovePosition :590-593）
            Pci1203Cmd c; c.kind = kCmdAxMoveAbs; c.axis = L.a.axis; c.value = (double)MotorUserToCard(target, L.g.gearRatio);
            const Pci1203CmdResult res = be.Pci1203Execute(c);
            if (CmdFailed(res)) { CancelLoop(be, false, "1203 move failed: " + CmdWhy(res)); return; }
            NoteIssued(be, L.a.axis);
            L.cmdIssued = true; L.legMs = 0;
            return;
        }
        L.legMs += ms;
        unsigned st = 0;
        if (be.Pci1203AxisState(L.a.axis, st) && IsErrorState(st)) { CancelLoop(be, false, "ERROR_STOP -> loop stopped"); return; }
        double card = 0;
        if (IsReadyState(st) && !SampleStale(be, L.a.axis) && be.Pci1203CmdPos(L.a.axis, card)) {
            const int pos = MotorCardToUser(card, L.g.gearRatio);
            if (pos - target <= 2 && target - pos <= 2) arrived = true;        // golden CompareCommandPos 的 iGap=2
            else if (L.legMs > ms) { CancelLoop(be, false, "stopped before target -> loop stopped"); return; }
        }
        if (!arrived && L.legMs > kLoopLegTimeoutMs) { CancelLoop(be, true, "leg timeout -> loop stopped"); return; }
    }
    if (!arrived) return;
    L.cmdIssued = false;
    if (L.task == 50) ++L.count;                                                // golden dwLoopCount++
    if (L.waitMs > 0) { L.waitLeft = L.waitMs; L.task = (L.task == 1) ? 10 : 60; }
    else L.task = (L.task == 1) ? 50 : 1;
}

// =============================================================================
//  W5-b（uteach 教導頁的運動鈕）—— golden uteach.cpp 的按鈕事件逐段照翻，1203 軸的底層照 EastSun（同 W4）。
//  golden 教導頁每個運動鈕的第一道是 `CheckCanMove()==false || ActiveMotorIndex==-1 || IsCanQuickJogMove()==false → return`
//  （forms/fTeach.cpp 已翻；兩支自己會跳訊息）；教導頁**沒有** uMotorTest 那種 btnHome／btnLoopMove 互斥（照 golden 不加）。
//  AI(W906-W5-b) 20260925: W5-b 覆核 14 條（W5B-1…14）的修正。偏離 golden 的每一處與待決事項：docs/W5_PROGRESS.md §6（W5-b）。
// =============================================================================
const char* kW5bDoc = "docs/W5_PROGRESS.md §6（W5-b）";

// 手動教導（golden SetButton*Click 的「關伺服 → fTeachShow->ShowModal() → 開伺服」，網頁上拆成開始／結束兩個請求）
struct TeachSetJob {
    bool                   active;
    std::string            btn, handler;
    TeachHandlerKind       kind;
    std::string            edit0, edit1;       // golden EditPtr／EditPtr1（GetTechPos 寫進哪兩個欄位）
    std::vector<MotionCtx> m;
    std::vector<bool>      servoOff;           // 開始時關了伺服的軸
    std::vector<bool>      servoOnAtEnd;       // golden 在 ShowModal 之後 ServoOnOff(true) 的軸
    //AI(W906-W5-b) 20260925: 覆核 R-W5B-8 —— golden GetTechPos 第一行 `if(ActiveMotorIndex==-1) return;`（uteach.cpp:3476）對兩軸分支也生效，
    //  而 SetButton020／064 不設 ActiveMotorIndex ⇒ 頁面當時沒有選馬達時，按確定什麼都不寫。
    bool                   amiValid;
    TeachSetJob() : active(false), kind(kTkNone), amiValid(true) {}
};
TeachSetJob g_teachSet;

// ---------------------------------------------------------------------------
//  AI(W906-W5-b) 20260925: 覆核 R-W5B-1 —— golden ActiveMotorIndex 與 EditPtr 由處理函式決定；頁面照 C++ 回的改選馬達（SetTo／jog／移動才會
//  作用在 golden 的那一軸）。成功的 ack 帶 editPtr／editPtr1／activeMotor 鍵；拒絕是純文字，尾巴加 " [editPtr=…,editPtr1=…,active=…]"，
//  只列這次 golden 真的改了的（active= 後面空白＝golden 的 ActiveMotorIndex 變成 -1）。
// ---------------------------------------------------------------------------
struct TeachEcho {
    std::string e0, e1;       // golden EditPtr／EditPtr1（空＝這次沒改）
    bool        act;          // golden ActiveMotorIndex 這次被改了
    std::string actAlias;     // 改成哪一軸（Mot_Table 別名；""＝-1）
    TeachEcho() : act(false) {}
};
void EchoActive(IMotorAccessBackend& be, TeachEcho& e, int mi)
{
    e.act = true;
    e.actAlias.clear();
    if (mi >= 0 && !be.AliasOfMotIndex(mi, e.actAlias)) e.actAlias.clear();
}
std::string EchoTail(const TeachEcho& e)
{
    std::string s;
    if (!e.e0.empty()) s += "editPtr=" + e.e0;
    if (!e.e1.empty()) s += std::string(s.empty() ? "" : ",") + "editPtr1=" + e.e1;
    if (e.act) s += std::string(s.empty() ? "" : ",") + "active=" + e.actAlias;
    return s.empty() ? s : " [" + s + "]";
}
MotorAccessOutcome RefuseT(const std::string& why, const TeachEcho& e) { return Refuse(why + EchoTail(e)); }

// 頁面目前選的馬達（golden ActiveMotorIndex 在按這顆按鈕之前的值）；-1＝沒選或馬達表上沒有
int PageAmi(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    MotorAccessAxis a;
    if (r.motors.empty() || !be.Resolve(r.motors[0], a)) return -1;
    return a.motIndex;
}

// ---- golden 的速度狀態（R-W5B-4；說明在 g_goldenPct）----
// golden MOT[mi].SetSpeed(pct)：1203 軸記下來（下一次教導頁的 jog／移動前送到卡上）；非 1203 軸直接叫 golden 物件（golden 當場寫卡）
void TeachSpeedSet(IMotorAccessBackend& be, int mi, int pct)
{
    if (mi < 0) return;
    std::string alias;
    MotorAccessAxis a;
    if (!be.AliasOfMotIndex(mi, alias) || !be.Resolve(alias, a)) return;
    if (a.Is1203()) { g_goldenPct[mi] = pct; return; }
    if (a.motIndex >= 0 && a.motorLive) be.GoldenSetSpeed(mi, pct);
}
// golden edtSpeed->Text（或 ScrollBar1->Position）改成 v：edtSpeedChange（uteach.cpp:2442）→ MOT[ActiveMotorIndex].SetSpeed(v)。
//   VCL 的 Text 沒變就不觸發 OnChange ⇒ 值一樣時什麼都不做。
void TeachEdtSpeed(IMotorAccessBackend& be, int ami, int v)
{
    if (g_teachEdt == v) return;
    g_teachEdt = v;
    TeachSpeedSet(be, ami, v);
}
// golden UpdateMotorTeachMonitor(Index)（:1275-1292）：ScrollBar1->Position=1、edtSpeed->Text="1"（→ ActiveMotorIndex 的 SetSpeed(1)）、MOT[Index].SetSpeed(1)
void TeachMonitor(IMotorAccessBackend& be, int ami, int index)
{
    if (index == -1) return;
    TeachEdtSpeed(be, ami, 1);
    TeachSpeedSet(be, index, 1);
}
// golden UpdateMotorTeachTwoMonitor(OldIndex, Index)（:1294-1310）：同上，但 ActiveMotorIndex／OldIndex／Index 任一為 -1 就整段不做
void TeachTwoMonitor(IMotorAccessBackend& be, int ami, int oldIndex, int index)
{
    if (ami == -1 || oldIndex == -1 || index == -1) return;
    TeachEdtSpeed(be, ami, 1);
    TeachSpeedSet(be, index, 1);
}
// 教導頁 jog／移動用的速度：golden 這一軸目前的 SetSpeed 百分比；這個行程還沒設過就 1%（golden 在教導頁選到一軸一定先經過
//   UpdateMotorTeachMonitor 的 SetSpeed(1)；網頁上選馬達會帶 speedEvent，照理不會走到這個預設）
int TeachCurPct(int mi)
{
    std::map<int, int>::const_iterator it = g_goldenPct.find(mi);
    return it == g_goldenPct.end() ? 1 : it->second;
}

// 教導頁共用前置：解析、golden 馬達、1203 層（MotionPrelude），再過 golden CheckCanMove＋IsCanQuickJogMove。
//   ⚠ 真機組態（或 SOFT_SIMULTE＋真的會動的 1203 軸，W5B-3）下 IsCanQuickJogMove → CheckShuttleCanMove 會
//     ShowErrorMessage("WAR16435"／"WAR16436", K_RETRY) —— 照 golden 阻塞到操作員回答（W5-b 裁決 1；答案怎麼回來見 docs）。
bool TeachPrelude(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, MotionCtx& m, bool needPos)
{
    if (!MotionPrelude(r, wireId, be, m, needPos)) return false;
    g_teachHomeUnknown.clear();
    if (!be.GoldenTeachCanMove(m.a.motIndex)) {
        m.why = r.action + "（uteach）" + r.motors[0] + ": golden CheckCanMove()／IsCanQuickJogMove() 不允許（急停、臂的 Z 不在原點、閘門等；訊息框已顯示原因）";
        //AI(W906-W5-b) 20260925: 覆核 W5B-R2 —— 1203 軸的「在原點」不明（ORG 極性未量、沒樣本）時一律當「不在原點」，golden 的訊息會說
        //  「請先讓 Z 軸在 home 的位置上」，這裡把真正的原因接在後面，免得操作員以為 Z 真的沒回原點
        if (!g_teachHomeUnknown.empty()) m.why += "；其中 1203 軸的原點狀態不明（當成不在原點，fail-closed）：" + g_teachHomeUnknown;
        return false;
    }
    return true;
}

// golden 以 MOT 下標取馬達（TechPara[Tag]->MotorSelect）→ 找回 Alias 再走同一個前置
bool TeachCtxOfIndex(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, int mi, bool needPos,
                     bool canMove, MotionCtx& m)
{
    std::string alias;
    if (!be.AliasOfMotIndex(mi, alias)) { m.why = r.action + ": golden 馬達 MOT[" + std::to_string(mi) + "] 在 Mot_Table 沒有 Alias"; return false; }
    MotorAccessReq r2 = r;
    r2.motors.assign(1, alias);
    return canMove ? TeachPrelude(r2, wireId, be, m, needPos) : MotionPrelude(r2, wireId, be, m, needPos);
}

TeachHandlerKind TeachKindOf(const std::string& h)
{
    if (h == "SetButton140Click") return kTkSet140;
    if (h == "GoButton140Click")  return kTkGo140;
    if (h == "SetButton020Click") return kTkSet020;
    if (h == "GoButton020Click")  return kTkGo020;
    if (h == "SetButton064Click") return kTkSet064;
    if (h == "MotorTrayXClick")   return kTkSelectOnly;
    return kTkOther;
}
char TeachListOf(TeachHandlerKind k)             // 處理函式讀哪張清單
{
    if (k == kTkSet140 || k == kTkGo140) return 'P';                                 // TechPara[Tag]（uteach.cpp:3371／:3408）
    if (k == kTkSet020 || k == kTkGo020 || k == kTkSet064) return 'T';               // TechTwoPara[Tag]（:3528／:3571／:3662）
    return 0;
}
std::string TeachRowText(const TeachRegRow& w)
{
    std::string s = std::string(w.owner == 'P' ? "TECH_PARA" : "TECH_TWOPARA") + "（馬達 " + w.name0;
    if (w.owner == 'T') s += "／" + w.name1;
    s += "、欄位 " + w.edit0;
    if (w.owner == 'T') s += "／" + w.edit1;
    return s + "）";
}

// W5B-13：golden 的 EditPtr／EditPtr1（btnSetTo／btnSetToOffset 寫進哪個欄位）在處理函式哪一步設、就在那一步之後的拒絕也要帶回頁面。
//   AI(W906-W5-b) 20260925: 覆核 R-W5B-1 —— 連同 golden ActiveMotorIndex 一起帶（TeachEcho／EchoTail，格式在上面）。
std::string AddEcho(const std::string& ack, const TeachEcho& e, const int* backlash = 0)
{
    if ((e.e0.empty() && !e.act && !backlash) || ack.size() < 2 || ack[ack.size() - 1] != '}') return ack;
    webbridge::JsonWriter w;
    w.BeginObject();
    if (!e.e0.empty()) w.Key("editPtr").String(e.e0);
    if (!e.e1.empty()) w.Key("editPtr1").String(e.e1);
    if (e.act) w.Key("activeMotor").String(e.actAlias);
    if (backlash) w.Key("backlash").Number((wb_int64)*backlash);
    w.EndObject();
    if (!w.Ok()) return ack;
    std::string out = ack;
    out.insert(out.size() - 1, std::string(ack.size() > 2 ? "," : "") + w.Str().substr(1, w.Str().size() - 2));
    return out;
}

MotorAccessOutcome TeachOk(const MotorAccessReq& r, const std::string& result, const std::string& msg,
                           const std::vector<int>* positions, bool hasTeach, bool teachActive,
                           const TeachEcho* echo = 0, const TeachSetJob* job = 0)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, result, "uteach", msg);
    if (positions) {
        w.Key("positions").BeginArray();
        for (std::size_t i = 0; i < positions->size(); ++i) w.Number((wb_int64)(*positions)[i]);
        w.EndArray();
    }
    if (hasTeach) w.Key("teachActive").Bool(teachActive);
    if (echo && !echo->e0.empty()) w.Key("editPtr").String(echo->e0);
    if (echo && !echo->e1.empty()) w.Key("editPtr1").String(echo->e1);
    if (echo && echo->act) w.Key("activeMotor").String(echo->actAlias);
    if (job) { w.Key("teachBtn").String(job->btn); w.Key("teachHandler").String(job->handler); }
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// W5B-4：golden `iNewPos=atoi(EditPtr->Text.c_str())`（uteach.cpp:3431／:3597）—— 目標值就是這一列畫面欄位的字。
//   頁面只送「從 C++ 載入過、非空白、是整數」的欄位（fields={欄位 id: 值}）；C++ 再驗一次：這一列要的欄位要在、要是有限整數。
//   ⚠ 偏離 golden：golden 的 atoi 對空白／非數字回 0 照走（走到 0）；這裡拒絕（W5-b 覆核 W5B-4、NB2 W4W5 盤點 :134／:263）。
bool TeachFieldValue(const MotorAccessReq& r, const std::string& edit, int& v, std::string& why)
{
    if (edit.empty()) { why = "golden 這一列沒有 SetEdit 欄位"; return false; }
    std::map<std::string, std::map<std::string, double> >::const_iterator f = r.obj.find("fields");
    if (f == r.obj.end()) { why = "缺 fields（頁面要送這顆按鈕的教導欄位值 {欄位 id: 值}；頁面版本太舊？）"; return false; }
    std::map<std::string, double>::const_iterator it = f->second.find(edit);
    if (it == f->second.end()) {
        why = "頁面沒送欄位 " + edit + " 的值 —— 欄位沒有從 C++ 載入、空白或不是整數時頁面不送（golden 的 atoi 會當 0 走，這裡不走）";
        return false;
    }
    const double d = it->second;
    if (!std::isfinite(d) || d != std::floor(d) || d > 2147483647.0 || d < -2147483648.0) {
        why = "欄位 " + edit + " 的值不是有限的整數"; return false;
    }
    v = (int)d;
    return true;
}

// 伺服開／關：1203 → EastSun kCmdAxSvOn（CmdFailed＝R21 W4B-3 的判準）；其他 → golden MOT.ServoOnOff
bool TeachServo(IMotorAccessBackend& be, long long wireId, const MotionCtx& m, bool on, std::string& why)
{
    if (!m.a.Is1203()) { be.GoldenServoOnOff(m.a.motIndex, on); return true; }
    Pci1203Cmd c; c.kind = kCmdAxSvOn; c.wireId = wireId; c.axis = m.a.axis; c.value = on ? 1.0 : 0.0;
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) { why = "1203 伺服" + std::string(on ? "開" : "關") + "失敗（" + m.a.cardModel + " MOT[" + std::to_string(m.a.motIndex) + "]）—— " + CmdWhy(res); return false; }
    return true;
}

// golden GetTechPos（uteach.cpp:3474-3512）：單軸（ActiveFlag[0]）Contec 且 MotorType==0 → ReadPos()，否則 ReadEncoderPos()；
//   兩軸（ActiveFlag[1]）→ ReadEncoderPos()。1203：ReadPos＝W4 的目前位置（CurrentUserPos），ReadEncoderPos＝監看器 actPos。
bool TeachTechPos(IMotorAccessBackend& be, const MotionCtx& m, bool single, int& pos)
{
    if (single && be.GoldenTechPosUsesReadPos(m.a.motIndex)) return CurrentUserPos(be, m, pos);   // W5B-14（:3487-3490）
    if (!m.a.Is1203()) { pos = be.GoldenReadEncoderPos(m.a.motIndex); return true; }
    double card = 0.0;
    if (!be.Pci1203ActPos(m.a.axis, card)) return false;
    pos = MotorCardToUser(card, m.g.gearRatio);
    return true;
}

// 結束手動教導：golden ShowModal 之後那一段 ServoOnOff(true)。W5B-6：golden ServoOnOff(true) 在 PServoAlarmOn 的軸上
//   MySleep(200)＋PCIL132_ResetPos（命令位置＝編碼器）；EastSun 不提供 Acm_AxSetCmdPosition（Pci1203Control.h:222 刻意不做）⇒
//   不發明驅動呼叫，改成記下這一軸，之後相對移動／jog／目前位置顯示以編碼器位置為基準（CurrentUserPos）。回傳錯誤（空＝全部成功）。
std::string EndTeachJob(IMotorAccessBackend& be, long long wireId)
{
    std::string err;
    for (std::size_t i = 0; i < g_teachSet.m.size(); ++i) {
        const MotionCtx& m = g_teachSet.m[i];
        if (g_teachSet.servoOnAtEnd[i]) {
            std::string why;
            if (!TeachServo(be, wireId, m, true, why)) err += " " + why + ";";
        }
        if (m.a.Is1203() && g_teachSet.servoOff[i] && m.g.servoAlarmOn) g_encBase.insert(m.a.axis);
    }
    g_teachSet = TeachSetJob();
    return err;
}
//AI(W906-W5-b) 20260925: 覆核 W5B-R5 —— 上一版在這裡有「死人開關」（操作員連線消失／告警／START → 結束手動教導並自動開伺服）。
//  golden 的 fTeachShow->ShowModal() 不會自己關，伺服只在操作員按確定／取消後才開（uteach.cpp:3393-3396、:3548-3556、:3689-3694）；
//  而自動開伺服的那一刻操作員的手可能還推著軸，開伺服會不會把軸拉回舊的命令位置（Q3）也還沒量 ⇒ 拿掉。取而代之：
//  連線斷掉／重新整理 → 手動教導保持（伺服保持關，golden 對話框開著的狀態）；頁面重新載入後 teachSet {query:true} 把對話框叫回來；
//  告警 → 照 golden 只停機；START → 手動教導中一律拒絕啟動（MotorAccessStartBlocked）。

// ---- jogP／jogN：uteach.cpp:997 btnJogPMouseDown／:1223 btnJogNMouseDown（放開 :1262 → DoJogRelease）----
//   ⚠ 偏離（W5-b 覆核 W5B-10 列出，docs）：golden 的 999999 保護與軟體極限比的是畫面 edtNowPosition（:1006／:2111），這裡用 C++ 讀的位置
//   AI(W906-W5-b) 20260925: 覆核 R-W5B-4 —— golden `SelMotSpeed=atoi(edtSpeed->Text)` 之後 MOT.JogP(SelMotSpeed)，但 TMyMotor::JogP(Speed)
//     根本不用 Speed（mymotor.cpp:1876-1888，直接 Motor->JogP()）⇒ jog 跑的是這一軸目前被設的速度（g_goldenPct），不是 edtSpeed。
MotorAccessOutcome DoTeachJog(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    MotionCtx m;
    if (!TeachPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string who = r.action + "（uteach）" + r.motors[0];
    int now = 0;
    if (!CurrentUserPos(be, m, now)) return Refuse(who + ": 讀不到目前位置（golden edtNowPosition）");
    if (now > 999999 || now < -999999) return Refuse(who + ": Position over limitation! 移動位置超過限制!!");   // Steven 20100831
    if (!be.GoldenSafeDoorClosed()) return Refuse(who + ": 安全門沒關（golden CheckSafeDoorIsClosed）");
    const bool positive = (r.action == "jogP");
    char b[220];
    if (positive && now >= m.g.softP) {                                         // Ztex 2024.08.18 Add Jog Ckeck SoftLimit
        std::snprintf(b, sizeof(b), ": The target position of %s over positive soft limit ! 的目標位置超過正向軟體極限! (%d > %d)", r.motors[0].c_str(), now, m.g.softP);
        return Refuse(who + b);
    }
    if (!positive && now <= m.g.softN) {
        std::snprintf(b, sizeof(b), ": The target position of %s below negative soft limit ! 的目標位置低於負向軟體極限! (%d <= %d)", r.motors[0].c_str(), now, m.g.softN);
        return Refuse(who + b);
    }
    if (!m.a.Is1203()) {
        be.GoldenJog(m.a.motIndex, positive, -1);                               // golden MOT[i].JogP／JogN(SelMotSpeed)：Speed 不用（-1＝不設速度）
        { JogRec j = { false, -1, m.a.motIndex, r.motors[0] }; ForgetJog(false, -1, m.a.motIndex); g_jogs.push_back(j); }
        return OkAck(r, "jogging", "MOT", std::string("MOT[") + std::to_string(m.a.motIndex) + "]." + (positive ? "JogP" : "JogN"),
                     m, 0, false, 0, false);
    }
    if (be.GoldenSafeDoorOpen(m.a.motIndex)) return Refuse(who + ": 安全門開著（golden TMyMotor::JogP 的 CheckIsSafeDoorOpen）");
    if (SampleStale(be, m.a.axis)) return Refuse(who + ": " + kStaleWhy);
    std::string why, note;
    if (!Send1203Speed(be, wireId, m, TeachCurPct(m.a.motIndex), why, note)) return Refuse(who + ": " + why);   // golden 這一軸目前的速度（R-W5B-4）
    Pci1203Cmd c; c.kind = kCmdAxMoveVel; c.wireId = wireId; c.axis = m.a.axis; c.dir = positive ? 1 : -1;   // EastSun：moveVel＋放開停止
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) return Refuse(who + ": 1203 失敗 —— " + CmdWhy(res));
    NoteIssued(be, m.a.axis);
    { JogRec j = { true, m.a.axis, m.a.motIndex, r.motors[0] }; ForgetJog(true, m.a.axis, m.a.motIndex); g_jogs.push_back(j); }
    return OkAck(r, "jogging", "pci1203", std::string(res.issued ? "sent: " : "accepted, NOT issued (dry): ") + res.wouldCall + "; " + note,
                 m, 0, false, 0, false);
}

// ---- moveRelative：uteach.cpp:2104 btnMovePClick／:2205 btnMoveNClick —— fCMD=false; MotorMove(P1±P2)，P1=目前位置、P2=ComboBox1 ----
//   AI(W906-W5-b) 20260925: 覆核 R-W5B-4 —— golden 這兩顆**不設速度**（上一版每次拿頁面 edtSpeed 設一次，蓋掉了 HOME 抬起的 SetSpeed(1)
//     保護與 GoButton020 之後的 20%）⇒ 1203 軸重送 golden 這一軸目前的速度（g_goldenPct，值不變），非 1203 軸不動速度。
MotorAccessOutcome DoTeachMoveRel(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    MotionCtx m;
    if (!TeachPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string who = r.action + "（uteach）" + r.motors[0];
    int now = 0;
    if (!CurrentUserPos(be, m, now)) return Refuse(who + ": 讀不到目前位置");
    if (now > 999999 || now < -999999) return Refuse(who + ": Position over limitation! 移動位置超過限制!!");
    double iv = 0.0;
    if (!ParamNum(r, "interval", iv)) return Refuse(who + ": 缺 interval（golden ComboBox1）");
    const int step = (int)(iv < 0 ? -iv : iv);
    const bool negative = (r.button.size() >= 5 && r.button.compare(r.button.size() - 5, 5, "MoveN") == 0);   // btnMoveN
    const int target = negative ? now - step : now + step;
    if (!m.a.Is1203()) return MoveGolden(r, be, m, target, false, 0);
    return Move1203(r, wireId, be, m, target, true, TeachCurPct(m.a.motIndex));
}

// ---- moveAbsolute：uteach.cpp:2391 btnMoveToClick —— fCMD=false; MotorMove(edtMoveTo)，golden 這裡不看 HomeFlag、不設速度（R-W5B-4）----
MotorAccessOutcome DoTeachMoveTo(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    MotionCtx m;
    if (!TeachPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string who = r.action + "（uteach）" + r.motors[0];
    double t = 0.0;
    if (!ParamNum(r, "targetPos", t)) return Refuse(who + ": 缺 targetPos（golden edtMoveTo）");
    if (!m.a.Is1203()) return MoveGolden(r, be, m, (int)t, false, 0);
    return Move1203(r, wireId, be, m, (int)t, true, TeachCurPct(m.a.motIndex));
}

MotorAccessOutcome DoHome(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be);   // 上面 W4-b2

// ---- HOME 抬起：golden btnHomeClick 的 else 支（uteach.cpp:2188-2197）----
//   golden：INDEX_MOTION_CARD==0 的 Index 四軸 → MOT[ActiveMotorIndex].Gali_Command("ST")；其他 → StopAllMotor()；最後 MOT[ActiveMotorIndex].SetSpeed(1)
//   （jou 2012-05-21「Home沒做完就暫停，再按Jog就飛出去撞機」）。
//   AI(W906-W5-b) 20260925: 覆核 R-W5B-6 —— 上一版直接呼叫 DoStop（golden btnStopClick 的語意：多送 MOT[MTestY1] 的 Galil "ST"、Galil Index 軸也 StopAllMotor）。
//     改成照 golden 抬起分支。StopAllMotor 的網頁譯法同 DoStop：EastSun 監看器開的 1203 軸 golden 的 StopAllMotor 碰不到 ⇒ 逐軸補送停止；
//     歸零工作一併取消（btnHome 抬起、軸也停了）。
//   ⚠ 仍然偏離（docs D2）：golden 抬起時也先過 CheckCanMove／IsCanQuickJogMove、ActiveMotorIndex==-1、磁性尺軸（擋住就不停）；這裡一律停。
MotorAccessOutcome TeachHomeRelease(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    MotorAccessAxis a;
    MotorGolden g;
    const bool hasM = !r.motors.empty() && be.Resolve(r.motors[0], a) && a.motIndex >= 0 && be.GoldenMotor(a.motIndex, g);
    bool ready = false;
    std::string why1203, firstRefusal, msg;
    int sent = 0, accepted = 0, refused = 0;
    CancelAllJobs(be, "HOME released (golden btnHomeClick else: btnHome up, motors stopped)");
    if (hasM && g.galilIndex) {                                                 // golden :2190-2192
        be.GoldenStopMotor(a.motIndex);                                         // INDEX_MOTION_CARD==0 的 Index 軸 → Gali_Command("ST")
        msg = "HOME 抬起：golden Galil 分支 MOT[" + std::to_string(a.motIndex) + "].Gali_Command(\"ST\")（:2191）";
    } else {                                                                    // golden :2193-2194
        be.GoldenStopAll("uteach.home");                                        // StopAllMotor()（抬起分支沒有 MTestY1 的 Galil "ST"）
        g_jogs.clear();
        ready = be.Pci1203Ready(why1203);
        if (ready) Stop1203All(be, wireId, sent, accepted, refused, firstRefusal);
        char b[200];
        std::snprintf(b, sizeof(b), "HOME 抬起：StopAllMotor（:2194）; 1203: stop issued to %d axis(es), accepted %d%s", sent, accepted,
                      refused ? "; SOME AXES REFUSED STOP" : "");
        msg = ready ? std::string(b) : std::string("HOME 抬起：StopAllMotor（:2194）; 1203 control not ready");
    }
    if (hasM) {                                                                 // golden :2196 MOT[ActiveMotorIndex].SetSpeed(1)
        if (!a.Is1203()) { if (a.motorLive) be.GoldenSetSpeed(a.motIndex, 1); }
        else if (!g.galilIndex) {
            std::string why, note, w2;
            MotionCtx m; m.a = a; m.g = g;
            //AI(W906-W5-b) 20260925: W5B-12 —— 回傳碼照 R21 W4B-3 判（Send1203Speed 內用 CmdFailed）；失敗就誠實說（停止已送）
            if (be.Pci1203Ready(w2) && a.axis >= 0 && a.tableEnable) {
                if (!Send1203Speed(be, wireId, m, 1, why, note))
                    return Refuse(r.action + "（uteach）" + r.motors[0] + ": HOME 抬起：停止已送（StopAllMotor＋1203 全軸停），但 golden MOT.SetSpeed(1) 失敗 —— " + why +
                                  "（卡上還是原本的速度；下一次 jog／移動前請先確認速度）");
            } else {
                g_goldenPct[a.motIndex] = 1;                                    // 卡不可用：記下 golden 的速度狀態（下一次教導頁的 jog／移動會送）
            }
        }
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "homeStopped", (hasM && g.galilIndex) ? "MOT" : (ready ? "MOT+pci1203" : "MOT"), msg);
    w.Key("homeActive").Bool(false);
    if (!firstRefusal.empty()) w.Key("firstRefusal").String(firstRefusal);
    w.Key("partial").Bool(refused > 0);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- home：uteach.cpp:2133 btnHomeClick ----
//   golden：馬達電源關 → 訊息、return（按下與抬起都先過這一道）；CheckCanMove／IsCanQuickJogMove；按下 → 單軸歸零
//   （1203 走 W4-b2 的 DS402＋W4-d 的門檢查與歸零速度）；抬起 → TeachHomeRelease。
MotorAccessOutcome DoTeachHome(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    std::map<std::string, bool>::const_iterator sf = r.flag.find("start");
    if (sf == r.flag.end())
        return Refuse(r.action + "（uteach）: 頁面版本太舊 —— home 要帶 start=true／false（按下後 btnHome->Down 的值）");
    //AI(W906-W5-b) 20260925: W5-b 覆核額外觀察 —— golden :2137-2142 的電源檢查在 `if(btnHome->Down==true)` 之前，抬起也先擋（電源關時軸不會動，照 golden 不送停止）
    if (be.GoldenMotorPowerOff()) return Refuse(r.action + "（uteach）: Motor power is OFF!! 電源被關閉!!（golden :2137）");
    if (!sf->second) return TeachHomeRelease(r, wireId, be);
    MotionCtx m;
    if (!TeachPrelude(r, wireId, be, m, false)) return Refuse(m.why);
    return DoHome(r, wireId, be);
}

// 背隙補償（golden TMyMotor::GetRotatorBacklash，mymotor.cpp:2012-2045）。W5B-9：ReadPos 照 W4 讀監看器（CurrentUserPos），
//   iLastRotatorDirP 由 Move1203／LoopMove／case 500 照 golden MotorMovePosition 記（NoteRotatorDir），歸零完成設 true（R-W5B-5）。
bool TeachRotatorBacklash(IMotorAccessBackend& be, const MotionCtx& m, int goal, bool inRot, int set, int& bl)
{
    const int prod = (set == 0) ? be.GoldenProdRotatorBacklash(inRot) : set;   // golden :2017／:2031 `(iTechData==0)?Prod.i*_iRotateA_Backlash:iTechData`
    int pos = 0;
    if (!CurrentUserPos(be, m, pos)) return false;                              // golden MOT[MIn/OutRotateKit].ReadPos()
    const bool lastP = be.GoldenRotatorLastDirP(m.a.motIndex);
    bl = 0;
    if (goal > pos && !lastP) bl = prod;
    else if (goal < pos && lastP) bl = -prod;
    return true;
}

// 有限的整數而且在 int 範圍（golden atoi 讀畫面字串的結果一定是 int）
bool IsIntValue(double d)
{
    return std::isfinite(d) && d == std::floor(d) && d <= 2147483647.0 && d >= -2147483648.0;
}

// ---- GoButton140Click（uteach.cpp:3401-3470）：單軸，TechPara[Tag] ----
MotorAccessOutcome TeachGo140(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, const TeachTarget& t)
{
    const std::string who0 = r.action + " " + t.btn + "（golden " + t.handler + "）";
    TeachEcho ec;
    //AI(W906-W5-b) 20260925: 覆核 R-W5B-1 —— golden :3408-3412 一進來就改 ActiveMotorIndex（含 ep1Picker 重映射），之後的 return 都已經改了
    const int mi = (t.row.mot0 < 0) ? -1 : be.GoldenTeachRemap(t.row.mot0);    // Ifor 20260109 fix 單一吸嘴模組顯示異常問題（:3409）
    EchoActive(be, ec, mi);
    if (mi < 0) return RefuseT(who0 + ": golden ActiveMotorIndex==-1 → return", ec);
    MotionCtx m;
    if (!TeachCtxOfIndex(r, wireId, be, mi, true, true, m)) return RefuseT(m.why, ec);   // CheckCanMove／IsCanQuickJogMove（:3414）＋Enable（:3419）
    const std::string who = who0 + " " + m.a.cardModel + " MOT[" + std::to_string(mi) + "]";
    if (m.g.homeFlag == 0) return RefuseT(who + ": motor need home 馬達需要歸零", ec);   // :3422（EditPtr 還沒設）
    ec.e0 = t.row.edit0;                                                        // :3428 EditPtr=TechPara[Tag]->SetEdit
    TeachMonitor(be, mi, mi);                                                   // :3429-3430 SetSpeed(ScrollBar1)＋UpdateMotorTeachMonitor → 1%（R-W5B-4）
    int target = 0;
    std::string why;
    if (!TeachFieldValue(r, ec.e0, target, why)) return RefuseT(who + ": " + why, ec);   // :3431 iNewPos=atoi(EditPtr->Text)
    char b[200];
    if (target > m.g.softP) { std::snprintf(b, sizeof(b), ": Over max soft limit, abort process! 超過軟體正極限 (%d > %d)", target, m.g.softP); return RefuseT(who + b, ec); }
    if (target < m.g.softN) { std::snprintf(b, sizeof(b), ": Bellow min soft limit, abort process! 小於軟體負極限 (%d < %d)", target, m.g.softN); return RefuseT(who + b, ec); }
    int bl = 0;
    if (m.g.rotateKit) {                                                        // RogerYang 20260113 : Rotator新增背隙補償（:3455-3470）
        double set = 0.0;
        if (!ParamNum(r, m.g.rotateKit == 1 ? "backlashIn" : "backlashOut", set))
            return RefuseT(who + ": 缺背隙設定（golden 讀 edtEditRotate" + std::string(m.g.rotateKit == 1 ? "In" : "Out") + "Backlash 的畫面值）", ec);
        //AI(W906-W5-b) 20260925: 覆核 W5B-R9 —— 背隙畫面值也要是有限整數（golden atoi 的結果是 int；上一版直接 (int)set，超大的值是未定義行為）
        if (!IsIntValue(set)) return RefuseT(who + ": 背隙設定不是有限的整數", ec);
        if (!TeachRotatorBacklash(be, m, target, m.g.rotateKit == 1, (int)set, bl)) return RefuseT(who + ": 讀不到旋轉站目前位置（背隙補償要用）", ec);
        const long long t2 = (long long)target + (long long)bl;
        if (t2 > 2147483647LL || t2 < -2147483647LL - 1) return RefuseT(who + ": 目標＋背隙超出 int 範圍", ec);
        target = (int)t2;
    }
    // golden：SetSpeed(ScrollBar1) 之後 InitMOTParameter(); SetSpeed(1); MotorMove(iNewPos) ⇒ 實際用 1% 走
    std::string alias; be.AliasOfMotIndex(mi, alias);
    MotorAccessReq r2 = r; r2.motors.assign(1, alias);
    MotorAccessOutcome o = m.a.Is1203() ? Move1203(r2, wireId, be, m, target, true, 1) : MoveGolden(r2, be, m, target, true, 1);
    if (!o.ok) return RefuseT(o.ackJson, ec);
    o.ackJson = AddEcho(o.ackJson, ec, m.g.rotateKit ? &bl : 0);
    return o;
}

// ---- GoButton020Click（uteach.cpp:3559-3600）：兩軸，TechTwoPara[Tag]；golden 這裡不看 HomeFlag ----
MotorAccessOutcome TeachGo020(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, const TeachTarget& t)
{
    const std::string who0 = r.action + " " + t.btn + "（golden " + t.handler + "）";
    const int mots[2] = { t.row.mot0, t.row.mot1 };
    const std::string edits[2] = { t.row.edit0, t.row.edit1 };
    TeachEcho ec;
    // 第一個迴圈（:3569-3574）：ActiveMotorIndex=MotorSelect[i]，兩軸都過 CheckCanMove／ActiveMotorIndex!=-1／IsCanQuickJogMove
    //   ⚠ 偏離（docs D6）：golden 的 Enable 檢查在第二個迴圈、第 1 軸已經在走之後；這裡解析／1203 層／Enable 兩軸先過才動
    MotionCtx mm[2];
    for (int i = 0; i < 2; ++i) {
        EchoActive(be, ec, mots[i]);                                            // AI(W906-W5-b) 20260925: R-W5B-1（golden 這一圈改 ActiveMotorIndex）
        if (mots[i] < 0) return RefuseT(who0 + ": golden ActiveMotorIndex==-1（第 " + std::to_string(i + 1) + " 軸）→ return", ec);
        if (!TeachCtxOfIndex(r, wireId, be, mots[i], true, true, mm[i])) return RefuseT(mm[i].why, ec);
    }
    const int ami = mots[1];                                                    // 第一個迴圈跑完，ActiveMotorIndex＝MotorSelect[1]
    // golden :3578-3584 看的是第一個迴圈留下來的 ActiveMotorIndex（＝第 2 軸），照翻
    const bool skip2 = be.GoldenIndexArm3Axis() && (mm[1].g.indexZ || mm[1].g.indexY);
    //AI(W906-W5-b) 20260925: 覆核 W5B-R8 —— 欄位值的檢查（docs D5，golden 的 atoi 沒有這一道）在第 1 軸動之前兩軸一起做，不再留下「只動了一軸」
    int targets[2] = { 0, 0 };
    for (int i = 0; i < 2; ++i) {
        if (i > 0 && skip2) continue;
        std::string why;
        if (!TeachFieldValue(r, edits[i], targets[i], why))
            return RefuseT(who0 + " 第 " + std::to_string(i + 1) + " 軸: " + why + "（兩軸都還沒動）", ec);
    }
    std::string done;
    char b[200];
    for (int i = 0; i < 2; ++i) {
        if (i > 0 && skip2) { done += "; axis 2 skipped (IndexArm_3_Axis)"; continue; }
        const std::string who = who0 + " 第 " + std::to_string(i + 1) + " 軸";
        ec.e0 = edits[i];                                                       // :3593 EditPtr=TechTwoPara[Tag]->SetEdit[i]
        // :3594-3596 ScrollBar1->Position=20（→ ActiveMotorIndex 的 SetSpeed）、MOT[Two].SetSpeed(20)、UpdateMotorTeachTwoMonitor（→ 1%）（R-W5B-4）
        TeachEdtSpeed(be, ami, 20);
        TeachSpeedSet(be, mots[i], 20);
        TeachTwoMonitor(be, ami, mots[0], mots[i]);
        const std::string moving = done.empty() ? std::string() : "（第 1 軸已經在走" + done + "）";
        const int target = targets[i];
        if (target > mm[i].g.softP) { std::snprintf(b, sizeof(b), ": Over max soft limit, abort process! 超過軟體正極限 (%d > %d)", target, mm[i].g.softP); return RefuseT(who + b + moving, ec); }
        if (target < mm[i].g.softN) { std::snprintf(b, sizeof(b), ": Bellow min soft limit, abort process! 小於軟體負極限 (%d < %d)", target, mm[i].g.softN); return RefuseT(who + b + moving, ec); }
        std::string alias; be.AliasOfMotIndex(mots[i], alias);
        MotorAccessReq r2 = r; r2.motors.assign(1, alias);
        MotorAccessOutcome o = mm[i].a.Is1203() ? Move1203(r2, wireId, be, mm[i], target, true, 20)   // Steven 20210723 : Go按鈕速度提升到20%
                                                : MoveGolden(r2, be, mm[i], target, true, 20);
        if (!o.ok) return RefuseT(who + ": " + o.ackJson + moving, ec);
        done += "; axis " + std::to_string(i + 1) + " -> " + std::to_string(target);
    }
    return TeachOk(r, "moving", "GoButton020Click" + done, 0, false, false, &ec);
}

// ---- 手動教導開始：SetButton140Click（:3360）／SetButton020Click（:3517）／SetButton064Click（:3650）----
//   golden：關伺服 → fTeachShow->ShowModal()（操作員用手推到位；OK = GetTechPos：EditPtr->Text＝編碼器；Cancel 不寫）→ 開伺服。
//   網頁上拆成兩個請求：start=true（關伺服、回 teachActive）→ 頁面顯示對話框 → start=false＋accept（開伺服；accept 才回 positions）。
//   ⚠ 偏離一處（修 golden 的坑，docs D1）：SetButton020／064 在 golden 是「逐軸：檢查 HomeFlag（或馬達 -1）→ 關伺服」，第 2 軸擋下時
//     第 1 軸已經被關伺服、函式直接 return（沒有對話框、也不開回來）⇒ 這裡先檢查全部軸，才開始關伺服。
MotorAccessOutcome TeachSetBegin(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, const TeachTarget& t)
{
    if (g_teachSet.active) return Refuse("teachSet: 已有一個手動教導在進行（" + g_teachSet.btn + "）—— 先按確定或取消");
    const TeachRegRow& w = t.row;
    const bool single = (t.kind == kTkSet140);
    // golden：EditPtr（與兩軸的 EditPtr1）在讀完 Tag 之後立刻設（:3376／:3528-3529／:3663-3664），之後的 return 都已經改過 EditPtr
    TeachEcho ec;
    ec.e0 = w.edit0;
    if (!single) ec.e1 = w.edit1;
    const int pageAmi = PageAmi(r, be);                                         // golden ActiveMotorIndex（020／064 不改它）
    const std::string who = r.action + " " + t.btn + "（golden " + t.handler + "）";
    TeachSetJob job;
    job.btn = t.btn; job.handler = t.handler; job.kind = t.kind; job.edit0 = ec.e0; job.edit1 = ec.e1;
    job.amiValid = single || pageAmi >= 0;                                      // AI(W906-W5-b) 20260925: R-W5B-8（golden GetTechPos :3476）
    if (single) EchoActive(be, ec, (w.mot0 < 0) ? -1 : be.GoldenTeachRemap(w.mot0));   // AI(W906-W5-b) 20260925: R-W5B-1（:3371-3375）
    const int n = single ? 1 : 2;
    for (int i = 0; i < n; ++i) {
        const int mot = (i == 0) ? w.mot0 : w.mot1;
        if (mot < 0) return RefuseT(who + ": golden " + std::string(single ? "ActiveMotorIndex" : "TwoActiveMotorIndex") + "==-1 → return", ec);
        const int mi = single ? be.GoldenTeachRemap(mot) : mot;                 // Ifor 20260109（:3372；兩軸那兩支沒有）
        // AI(W906-W5-b) 20260925: R-W5B-4 —— golden UpdateMotorTeachMonitor（140 :3381／020 :3538，都在 HomeFlag 檢查之前）→ SetSpeed(1)
        if (t.kind != kTkSet064) TeachMonitor(be, single ? mi : pageAmi, mi);
        MotionCtx m;
        if (!TeachCtxOfIndex(r, wireId, be, mi, true, false, m)) return RefuseT(m.why, ec);   // golden Set 鈕不過 CheckCanMove
        if (single && m.g.indexZ)                                               // golden :3384 Index Z 不做手動教導
            return TeachOk(r, "noHandTeach", "golden：Index Z 軸（MTestZ1／Z2）不做手動教導（沒有關伺服、沒有對話框）", 0, true, false, &ec);
        if (m.g.homeFlag == 0) return RefuseT(who + ": motor need home 馬達需要歸零（MOT[" + std::to_string(mi) + "]）", ec);
        const bool keepOn = single ? m.g.teachNoServoOff : (t.kind == kTkSet064 && m.g.indexY);   // :3393／:3683
        job.m.push_back(m);
        job.servoOff.push_back(!keepOn);
        // golden ShowModal 之後：140 無條件 ServoOnOff(true)（:3396，連沒關的 MTrayBracketZ／MMagazine 也送 —— W5B-14）；
        //   020 每一軸（:3551-3556）；064 非 Index Y（:3690-3694）
        job.servoOnAtEnd.push_back(t.kind == kTkSet064 ? !m.g.indexY : true);
    }
    if (t.kind == kTkSet064) TeachTwoMonitor(be, pageAmi, w.mot0, w.mot1);     // golden :3686（迴圈之後）UpdateMotorTeachTwoMonitor（R-W5B-4）
    for (std::size_t i = 0; i < job.m.size(); ++i) {
        std::string why;
        if (!job.servoOff[i] || TeachServo(be, wireId, job.m[i], false, why)) continue;
        // W5B-12：關伺服失敗 → 把前面關掉的開回來，而且每一個回滾都看回傳碼（R21 W4B-3），訊息照實說
        std::string rb;
        bool any = false;
        for (std::size_t k = 0; k < i; ++k) {
            if (!job.servoOff[k]) continue;
            any = true;
            std::string w2;
            if (!TeachServo(be, wireId, job.m[k], true, w2)) rb += " 第 " + std::to_string(k + 1) + " 軸開回失敗 —— " + w2 + ";";
        }
        return RefuseT(who + ": " + why + (!any ? std::string() : rb.empty() ? std::string("（已把前面關掉的伺服開回）")
                                                                  : "（前面關掉的伺服開回失敗:" + rb + " 請到機台確認伺服狀態）"), ec);
    }
    job.active = true;
    g_teachSet = job;
    return TeachOk(r, "teaching", "伺服已關閉 —— 用手把軸推到位，再按確定（讀編碼器）或取消（golden " + t.handler + " → fTeachShow）",
                   0, true, true, &ec, &g_teachSet);
}

// ---- 手動教導結束：golden fTeachShow 的確定（GetTechPos）／取消，然後 ServoOnOff(true) ----
MotorAccessOutcome TeachSetEnd(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, bool accept)
{
    if (!g_teachSet.active) return TeachOk(r, "teachIdle", "沒有進行中的手動教導", 0, true, false);
    const TeachSetJob job = g_teachSet;
    std::vector<int> pos;
    std::string err;
    //AI(W906-W5-b) 20260925: 覆核 R-W5B-8 —— golden GetTechPos 第一行 `if(ActiveMotorIndex==-1) return;`（:3476）在兩個分支之前
    const bool write = accept && job.amiValid;
    if (write)                                                                  // 伺服還關著時讀（golden 在 ShowModal 裡按 OK）
        for (std::size_t i = 0; i < job.m.size(); ++i) {
            int p = 0;
            if (!TeachTechPos(be, job.m[i], job.kind == kTkSet140, p)) err += " 第 " + std::to_string(i + 1) + " 軸位置讀不到;";
            pos.push_back(p);
        }
    const std::string se = EndTeachJob(be, wireId);
    if (!err.empty() || !se.empty())
        return Refuse("teachSet 結束（" + job.btn + "）:" + err + se + "（伺服已嘗試開回；請確認每一軸的伺服狀態；位置沒有寫進欄位）");
    TeachEcho ec;
    ec.e0 = job.edit0; ec.e1 = job.edit1;
    return TeachOk(r, write ? "taught" : accept ? "taughtNothing" : "teachCancelled",
                   write ? "GetTechPos：讀編碼器位置（golden fTeachShow OK）；伺服已開回"
                         : accept ? "golden GetTechPos：ActiveMotorIndex==-1 → return（SetButton020／064 不設 ActiveMotorIndex，按的時候頁面沒有選馬達）—— 什麼都不寫；伺服已開回"
                                  : "取消（golden fTeachShow Cancel）；伺服已開回",
                   write ? &pos : 0, true, false, &ec);
}

// ---- teachSet／teachGo 的按鈕：依 golden Tag 語意找出處理函式與那一列，照處理函式做（W5B-1／W5B-2）----
//   ⚠ 照處理函式、不照頁面把它放在 Set 還是 Go 欄：例 SetBtnPreciserOpen／Close（golden .dfm OnClick = GoButton140Click）按下去是移動。
MotorAccessOutcome DoTeachButton(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    std::map<std::string, std::string>::const_iterator b = r.str.find("btn");
    if (b == r.str.end() || b->second.empty()) return Refuse(r.action + ": 缺 btn（教導點的 Set／Go 按鈕名，頁面版本太舊？）");
    TeachRegistry reg;
    std::string why;
    if (!be.GoldenTeachRegistry(reg, why))
        return Refuse(r.action + " " + b->second + ": golden 登錄表對不上 —— " + why + "（教導頁的 Set／Go 全部不做，fail-closed）");
    TeachTarget t;
    const std::string bad = MotorAccessResolveTeachButton(reg, b->second, kTeachQuirkPolicy, t);
    if (!bad.empty()) return Refuse(r.action + " " + bad);
    if (t.kind == kTkSelectOnly) {                                              // golden MotorTrayXClick（:3630-3648 TechMotorAxle[Tag]）：只選馬達
        TeachMonitor(be, t.selectMot, t.selectMot);                             // :3647 UpdateMotorTeachMonitor → SetSpeed(1)（R-W5B-4）
        TeachEcho ec;
        EchoActive(be, ec, t.selectMot);                                        // R-W5B-1
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "selectOnly", "ui", "golden " + t.btn + " 的處理函式是 MotorTrayXClick：只選馬達 " + t.selectName +
                                          "（ActiveMotorIndex＝TechMotorAxle[Tag]，:3636；Tag 由 FormShow :1518-1521 設），不動、不關伺服");
        w.Key("selectMotor").String(t.selectName);
        w.Key("activeMotor").String(ec.actAlias);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    if (t.kind == kTkGo140) return TeachGo140(r, wireId, be, t);
    if (t.kind == kTkGo020) return TeachGo020(r, wireId, be, t);
    return TeachSetBegin(r, wireId, be, t);                                     // Set140／020／064
}

// ---- teachSet：query（頁面重新整理後看得到還在進行的手動教導，W5B-5／W5B-R6）／start=false 結束／start=true 開始 ----
MotorAccessOutcome DoTeachSet(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    std::map<std::string, bool>::const_iterator qf = r.flag.find("query");
    if (qf != r.flag.end() && qf->second) {
        const TeachSetJob& j = g_teachSet;
        TeachEcho ec;
        if (j.active) { ec.e0 = j.edit0; ec.e1 = j.edit1; }
        return TeachOk(r, j.active ? "teaching" : "teachIdle",
                       j.active ? "手動教導進行中（" + j.btn + "，伺服是關的）—— 按確定讀位置或取消，都會開回伺服" : std::string("沒有進行中的手動教導"),
                       0, true, j.active, &ec, j.active ? &j : 0);
    }
    std::map<std::string, bool>::const_iterator sf = r.flag.find("start");
    if (sf == r.flag.end()) return Refuse("teachSet: 缺 start（true＝開始手動教導、false＝結束）");
    if (!sf->second) {
        std::map<std::string, bool>::const_iterator ac = r.flag.find("accept");
        return TeachSetEnd(r, wireId, be, ac != r.flag.end() && ac->second);
    }
    if (g_teachSet.active) return Refuse("teachSet: 已有一個手動教導在進行（" + g_teachSet.btn + "）—— 先按確定或取消");
    return DoTeachButton(r, wireId, be);
}

// W5B-5：手動教導中，uMotorTest 對同一軸的任何按鈕（停止除外，停止在前面就分派掉了）擋下 —— golden 的 fTeachShow 是 ShowModal
//   AI(W906-W5-b) 20260925: 覆核 W5B-R1 —— 只擋 uMotorTest。教導頁自己的請求在分派層另一道處理（只放行 teachSet）；上一版不分 source，
//   教導頁送的「確定／取消」（motors＝教導軸本身）被當成「同軸」拒絕，手動教導結束不了、伺服一直關著。
std::string TeachAxisBusyWhy(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    if (!g_teachSet.active || r.motors.empty() || r.source == "uteach") return std::string();
    MotorAccessAxis a;
    if (!be.Resolve(r.motors[0], a)) return std::string();
    for (std::size_t i = 0; i < g_teachSet.m.size(); ++i) {
        const MotorAccessAxis& t = g_teachSet.m[i].a;
        if ((a.motIndex >= 0 && a.motIndex == t.motIndex) || (a.Is1203() && t.Is1203() && a.axis >= 0 && a.axis == t.axis))
            return "motor.access " + r.action + "（" + r.source + "）" + r.motors[0] + ": 教導頁手動教導中（" + g_teachSet.btn +
                   "，這一軸伺服是關的）—— golden fTeachShow 是 ShowModal，開著時 MotorTest 按不到；先在教導頁按確定或取消";
    }
    return std::string();
}

// AI(W906-W5-b) 20260925: 覆核 R-W5B-4 —— 頁面報的 golden 速度事件：操作員改了 edtSpeed（golden ScrollBar1Change／edtSpeedChange :1312／:2442
//   → MOT[ActiveMotorIndex].SetSpeed(v)）或操作員選了馬達（UpdateMotorTeachMonitor → SetSpeed(1)，頁面這時 edtSpeed 顯示 1）。
//   params {speedEvent:true, speed:v}；v 照 golden edtSpeedChange 夾在 ScrollBar1 的 Min 1／Max 100（uteach.dfm ScrollBar1 Min=1，Max 預設 100）。
void TeachSpeedEvent(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    std::map<std::string, bool>::const_iterator ev = r.flag.find("speedEvent");
    double v = 0.0;
    if (ev == r.flag.end() || !ev->second || !ParamNum(r, "speed", v) || !std::isfinite(v)) return;
    const int ami = PageAmi(r, be);
    if (ami < 0) return;                                                        // golden edtSpeedChange：ActiveMotorIndex==-1 → return
    int s = (v > 100.0) ? 100 : (v < 1.0) ? 1 : (int)v;
    g_teachEdt = s;
    TeachSpeedSet(be, ami, s);
}

}  // namespace

// ---------------------------------------------------------------------------
//  AI(W906-W5-b) 20260925: W5-b 裁決 2 ＝ 2C（RULINGS_20260925 第 9 條「逐顆驗證，確定是 golden 綁錯的改成動它自己那一列；驗不確定的維持停用」）：§6.4 A 類 6 顆（PADView_Z／BGAView_Z／ScannerAOI_Z 的 Set／Go）在 golden 128 種組合下全部讀到 TechTwoPara 之外（確定綁錯）⇒ 選項 C；B／C 類沒登錄、沒有自己的教導點，兩種政策都拒絕（＝維持停用）
//  （這顆按鈕改動它自己那一列：P 列照 SetButton140／GoButton140、T 列照 SetButton020／GoButton020）。只有這一個開關。
// ---------------------------------------------------------------------------
const int kTeachQuirkPolicy = kTeachQuirkOwnRow;

// golden 的 Tag 語意（uteach.cpp FormShow :1473-1505）：TechPara 迴圈先、TechTwoPara 迴圈後，每一列 `funButton->Tag=i; btGo->Tag=i;`
//   ⇒ 同一顆按鈕最後登錄的那一列勝出；處理函式再讀 TechPara[Tag]（140）或 TechTwoPara[Tag]（020／064）。
//   FormShow :1518-1521 另把四顆 SetButtonIn*LtcSen* 的 Tag 改成軸控鈕的 Tag（處理函式 MotorTrayXClick 讀 TechMotorAxle[Tag]）。
//   AI(W906-W5-b) 20260925: 覆核 R-W5B-2 —— 先判 golden 按不按得到（靜態確定按不到的一律不做）；R-W5B-3 —— 這台沒登錄、但 .dfm 上處理函式是
//   教導處理函式的按鈕，golden 用 .dfm 的 Tag 讀清單 ⇒ 也是怪按鈕（沒有自己的教導點，選項 C 不適用），照實說 golden 會讀哪一列。
std::string MotorAccessResolveTeachButton(const TeachRegistry& reg, const std::string& btn, int policy, TeachTarget& t)
{
    t = TeachTarget();
    t.btn = btn;
    const TeachRegRow* own = 0;
    int lastSeq = -1;
    bool regVis = true;
    for (std::size_t i = 0; i < reg.P.size(); ++i)
        if (reg.P[i].setBtn == btn || reg.P[i].goBtn == btn) {
            own = &reg.P[i]; t.ownList = 'P'; t.ownIndex = (int)i;
            if (reg.P[i].seq > lastSeq) { lastSeq = reg.P[i].seq; regVis = reg.P[i].vis; }
        }
    for (std::size_t i = 0; i < reg.T.size(); ++i)
        if (reg.T[i].setBtn == btn || reg.T[i].goBtn == btn) {
            own = &reg.T[i]; t.ownList = 'T'; t.ownIndex = (int)i;
            if (reg.T[i].seq > lastSeq) { lastSeq = reg.T[i].seq; regVis = reg.T[i].vis; }
        }
    std::map<std::string, TeachBtnInfo>::const_iterator bi = reg.btns.find(btn);
    const TeachBtnInfo* info = (bi != reg.btns.end()) ? &bi->second : 0;
    if (info) {
        // golden 看得見嗎：起始＝.dfm Visible；建構子裡最後執行的那一列登錄把它設成那一列的 Visible 引數（TECH_PARA 建構子 :61-74）；
        //   再套登錄以外的賦值（產生器已分類）與父物件（產生器已算好靜態確定看不見的那一層）
        const bool vis = (lastSeq >= 0) ? regVis : info->dfmVisible;
        std::string why;
        if (!info->hiddenWhy.empty()) why = info->hiddenWhy;
        else if (info->selfOther == "hide") why = btn + "->Visible 被 golden 無條件設成 false，之後沒有任何地方設回 true";
        else if (!vis && info->selfOther != "dyn")
            why = std::string(lastSeq >= 0 ? "golden 建構子登錄這顆按鈕時 Visible=false（TECH_PARA 建構子把按鈕藏起來，uteach.cpp:61-74）"
                                           : ".dfm 的 Visible=False") + "，之後沒有任何地方設回 true";
        if (!why.empty()) {
            t.handler = info->handler;
            t.hiddenWhy = why;
            return btn + ": golden 按不到這顆按鈕 —— " + why + "。golden 裡按不到＝什麼都不會發生，這裡同樣不做";
        }
    }
    std::map<std::string, std::pair<int, std::string> >::const_iterator so = reg.selectOnly.find(btn);
    if (so != reg.selectOnly.end()) {                                           // :1518-1521 的 Tag 覆寫在兩個迴圈之後，最後勝出
        t.handler = "MotorTrayXClick"; t.kind = kTkSelectOnly;
        t.selectMot = so->second.first; t.selectName = so->second.second;
        if (own) t.row = *own;
        return std::string();
    }
    if (!own) {
        if (!info)
            return btn + ": golden uteach.dfm 沒有這顆教導按鈕（不是 TSpeedButton，或處理函式不是 Set／Go 教導處理函式）—— 不做";
        t.unregistered = true;
        t.handler = info->handler;
        t.kind = TeachKindOf(t.handler);
        t.readList = TeachListOf(t.kind);
        t.ownIndex = info->dfmTag;
        t.quirk = true;
        const std::vector<TeachRegRow>& rl = (t.readList == 'P') ? reg.P : reg.T;
        const std::string rn = (t.readList == 'P') ? "TechPara" : "TechTwoPara";
        t.outOfRange = t.ownIndex < 0 || t.ownIndex >= (int)rl.size();
        if (!t.outOfRange) { t.goldenRow = rl[t.ownIndex]; t.goldenRowKnown = true; }
        const std::string what = t.outOfRange
            ? rn + "[" + std::to_string(t.ownIndex) + "] —— 超出 " + rn + "（這台 " + std::to_string(rl.size()) + " 列），golden 讀到清單外的記憶體（未定義行為，多半是存取違規）"
            : rn + "[" + std::to_string(t.ownIndex) + "] = " + TeachRowText(rl[t.ownIndex]);
        return btn + ": 未登錄的教導鈕（怪按鈕）—— 這台機台的 golden 登錄表沒有它" +
               std::string(info->lateReg ? "（golden 只在 InitialFormOncetime 登錄它：USE_Scanner_AOI_Inspection==TopBottomInstall，uteach.cpp:5998-6014；移植樹沒有 FrmAOI，缺相依）" : "") +
               "，golden FormShow 不設它的 Tag（仍是 .dfm 的 Tag=" + std::to_string(t.ownIndex) + "），處理函式 " + t.handler + " 照樣讀 " + what +
               "。不做 —— 待使用者決定（" + kW5bDoc + " 裁決 2：暫定拒絕；它沒有自己的教導點，選項 C 不適用）";
    }
    t.handler = (own->setBtn == btn) ? own->setHandler : own->goHandler;
    t.kind = TeachKindOf(t.handler);
    t.readList = TeachListOf(t.kind);
    if (!t.readList)
        return btn + ": golden 處理函式 " + t.handler + " 不是教導點的 Set／Go 處理函式（" + TeachRowText(*own) + "）—— 這裡沒有翻它，不做";
    const std::vector<TeachRegRow>& rl = (t.readList == 'P') ? reg.P : reg.T;
    t.quirk = (t.readList != t.ownList);
    t.outOfRange = t.ownIndex >= (int)rl.size();
    if (!t.quirk) { t.row = *own; return std::string(); }
    if (!t.outOfRange) { t.goldenRow = rl[t.ownIndex]; t.goldenRowKnown = true; }
    const std::string rn = (t.readList == 'P') ? "TechPara" : "TechTwoPara";
    const std::string what = t.outOfRange
        ? rn + "[" + std::to_string(t.ownIndex) + "] —— 超出 " + rn + "（這台 " + std::to_string(rl.size()) + " 列），golden 讀到清單外的記憶體（未定義行為，多半是存取違規）"
        : rn + "[" + std::to_string(t.ownIndex) + "] = " + TeachRowText(rl[t.ownIndex]);
    if (policy == kTeachQuirkOwnRow) {                                          // 選項 C：動這顆按鈕自己那一列
        t.row = *own;
        const bool isSet = (own->setBtn == btn);
        t.kind = (t.ownList == 'P') ? (isSet ? kTkSet140 : kTkGo140) : (isSet ? kTkSet020 : kTkGo020);
        t.readList = t.ownList;
        return std::string();
    }
    return btn + ": 怪按鈕 —— golden 處理函式 " + t.handler + " 讀 " + rn + "[Tag]，而這顆按鈕的 Tag=" + std::to_string(t.ownIndex) +
           " 是 " + (t.ownList == 'P' ? "TechPara" : "TechTwoPara") + " 的索引（它自己的教導點是 " + TeachRowText(*own) + "）；golden 實際會讀 " +
           what + "。不做 —— 待使用者決定（" + kW5bDoc + " 裁決 2：暫定拒絕，選項 C＝改成動自己那一列）";
}

// AI(W906-W5-b) 20260925: 覆核 W5B-R2 —— 在機台上量到的卡片 CFG_AxOrgLogic（-1＝還沒量）。量法：Z 軸停在原點、離開原點，各讀一次
//   監看器 motionIO bit 4（1203 監看頁或 /api），再讀一次卡片的 CFG_AxOrgLogic（EastSun 監看器目前不讀這個屬性）。量到之前 1203 軸的「在原點」一律不明。
const int kPci1203CardOrgLogic = -1;

int MotorAccessOrgHomeLed(bool orgBit, bool sensorType, int cardOrgLogic)
{
    if (cardOrgLogic != 0 && cardOrgLogic != 1) return -1;                      // 卡片的 ORG 邏輯不知道 ⇒ 位元的意思不知道
    const int goldenLogic = sensorType ? 0 : 1;                                 // golden InitMotor :252 `uOrgLogic=bSensorType ? 0 : 1`
    return ((cardOrgLogic == goldenLogic) ? orgBit : !orgBit) ? 1 : 0;          // 卡片的邏輯跟 golden 設的不同 ⇒ 位元反相
}

int MotorAccessTeachHomeLed(IMotorAccessBackend& be, int mi)
{
    std::string alias;
    MotorAccessAxis a;
    if (!be.AliasOfMotIndex(mi, alias) || !be.Resolve(alias, a) || !a.Is1203()) return -2;
    if (!a.tableEnable) return 1;                                               // golden Enable=false → Led[iHomeLed]=true（golden myEthercatmotor.cpp:888；移植樹 :1207）
    if (a.axis < 0) { g_teachHomeUnknown = alias + "：" + a.why; return -1; }
    unsigned long io = 0;
    if (!be.Pci1203MotionIO(a.axis, io)) { g_teachHomeUnknown = alias + "：監看器沒有這一軸的樣本"; return -1; }
    if (SampleStale(be, a.axis)) { g_teachHomeUnknown = alias + "：命令之後監看器還沒有新樣本"; return -1; }   // 命令之後還沒新樣本
    MotorGolden g;
    if (!be.GoldenMotor(mi, g)) { g_teachHomeUnknown = alias + "：沒有 golden 馬達物件（讀不到 SensorType）"; return -1; }
    // AX_MOTION_IO_ORG（golden myEthercatmotor.cpp:860 ScanMotorStatus `(Status>>4)&0x1`）；極性見 kPci1203CardOrgLogic（W5B-R2）
    const int led = MotorAccessOrgHomeLed((io & 0x00000010ul) != 0, g.sensorType, kPci1203CardOrgLogic);
    if (led < 0)
        g_teachHomeUnknown = alias + "：EastSun 監看器開軸沒有設 CFG_AxOrgLogic（golden InitMotor myEthercatmotor.cpp:252 依 SensorType 設），"
                                     "ORG 位元的極性還沒在機台上量（kPci1203CardOrgLogic=-1）";
    return led;
}

std::string MotorAccessTeachHomeUnknownWhy() { return g_teachHomeUnknown; }

bool MotorAccessTeachLive1203(IMotorAccessBackend& be, int mi)
{
    std::string alias, why;
    MotorAccessAxis a;
    if (!be.AliasOfMotIndex(mi, alias) || !be.Resolve(alias, a) || !a.Is1203()) return false;
    return be.Pci1203Ready(why);
}

bool MotorAccessStartBlocked(std::string& why)
{
    if (!g_teachSet.active) return false;
    why = "START 拒絕：教導頁手動教導中（" + g_teachSet.btn + "，伺服是關的）—— golden 的 fTeachShow 是 ShowModal，開著時畫面上的 START 按不到；"
          "先在教導頁按確定或取消（" + std::string(kW5bDoc) + " D9）";
    return true;
}

bool MotorAccessEncoderBase(int axis1203) { return g_encBase.count(axis1203) != 0; }

int MotorAccessGoldenSpeedPct(int mi)
{
    std::map<int, int>::const_iterator it = g_goldenPct.find(mi);
    return it == g_goldenPct.end() ? -1 : it->second;
}

// ---------------------------------------------------------------------------
void MotorAccessTick(IMotorAccessBackend& be, int elapsedMs, bool operatorConnected)
{
    // AI(W906-W5-b) 20260925: 覆核 W5B-R5／R6 —— 手動教導不在這裡結束（golden 的對話框不會自己關；見 DoTeachJog 前的說明）
    if (!operatorConnected && !g_jogs.empty()) {                                // NB2 R21 W4B-4：jog 的死人開關
        for (std::size_t i = 0; i < g_jogs.size(); ++i) {
            if (g_jogs[i].is1203) { Pci1203Cmd c; c.kind = kCmdAxStop; c.axis = g_jogs[i].axis; be.Pci1203Execute(c); }
            else be.GoldenStopMotor(g_jogs[i].mi);
        }
        g_lastJobNote = "jog stopped: operator connection gone (deadman, " + std::to_string(g_jogs.size()) + " axis)";
        g_jogs.clear();
    }
    // NB2 R23 W4C-4：golden Timer1Timer（uMotorTest.cpp:914-919）
    //   `if(fShow==false) return;`（畫面關了就不再推進）→ 網頁：操作員連線消失
    //   `if(IsSafeLockCheck()) Close();`（關畫面 ⇒ 同上）
    //   golden 只是不再推進（重開畫面會接著走）；網頁上不自動恢復 ⇒ 取消工作（HomeFlag 維持 0，不假裝完成）。
    //   golden 不送停止（進行中的那一段走完），這裡同樣不送。
    if (AnyJobActive()) {
        if (!operatorConnected)
            CancelAllJobs(be, "operator connection gone (golden Timer1Timer: fShow==false -> no further steps)");
        else if (be.GoldenSafeLockActive())
            CancelAllJobs(be, "safe lock active (golden Timer1Timer: IsSafeLockCheck() -> Close())");
    }
    TickHomes(be, elapsedMs);
    TickLoop(be, elapsedMs);
}

void MotorAccessOnAlarm(IMotorAccessBackend& be, const std::string& what)
{
    const bool had = AnyJobActive() || !g_jogs.empty();
    CancelAllJobs(be, "alarm " + what + " (golden Timer1Timer: fNote->fShow -> btnLoopMove/btnHome up)");
    g_jogs.clear();
    // AI(W906-W5-b) 20260925: 覆核 W5B-R5 —— 手動教導不動：golden 在 fTeachShow 開著時出告警只停機（note.cpp:795 StopAllMotor），伺服保持關，
    //   對話框還在；伺服只在操作員按確定／取消後開。上一版在這裡自動開伺服（含 kcode==0 的通知），拿掉。
    std::string why;
    if (!be.Pci1203Ready(why)) return;
    int n = 0;
    for (int ax = 0; ax < be.Pci1203AxisCount(); ++ax) {                        // golden note.cpp:795 StopAllMotor＝全部馬達（見 DoStop 的說明）
        if (!be.Pci1203AxisOpened(ax)) continue;
        Pci1203Cmd c; c.kind = kCmdAxStop; c.axis = ax; be.Pci1203Execute(c);
        ++n;
    }
    if (had || n) g_lastJobNote = "alarm " + what + ": jobs cancelled, 1203 stop sent to " + std::to_string(n) + " axis(es)";
}

MotorAccessJobState MotorAccessJobs()
{
    MotorAccessJobState j;
    j.homesActive = 0;
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (g_homes[i].active) ++j.homesActive;
    j.loopActive = g_loop.active;
    j.loopMotor  = g_loop.motorId;
    j.loopTask   = g_loop.task;
    j.loopCount  = g_loop.count;
    j.lastNote   = g_lastJobNote;
    j.jogsActive = (int)g_jogs.size();
    j.teachActive = g_teachSet.active;                                          // AI(W906-W5-b) 20260925
    j.teachBtn    = g_teachSet.btn;
    j.encBaseAxes = (int)g_encBase.size();
    return j;
}

void MotorAccessResetJobs()
{
    g_jogs.clear();
    g_issuedPoll.clear();
    g_homes.clear();
    g_loop = LoopJob();
    g_lastJobNote.clear();
    g_teachSet = TeachSetJob();
    g_encBase.clear();                                                          // AI(W906-W5-b) 20260925
    g_goldenPct.clear();                                                        // AI(W906-W5-b) 20260925: R-W5B-4
    g_teachEdt = -1;
    g_teachHomeUnknown.clear();
}

// ---------------------------------------------------------------------------
int MotorUserToCard(int p, double r)
{
    if (r == 0.0) r = 1.0;                                                      // golden 會把 Motor->GearRatio 改成 1.0（這裡不寫回）
    int p1 = (int)(float)((double)p / r);                                       // ChangeToFloatNonPcnt 回 float，再賦給 int
    int p2 = (int)((double)p1 * r);
    if (p2 < p) {
        while (1) { p1++; p2 = (int)((double)p1 * r); if (p2 >= p) break; }
    } else if (p2 > p) {
        while (1) { p1--; p2 = (int)((double)p1 * r); if (p2 <= p) break; }
    }
    return p1;
}

int MotorCardToUser(double card, double r)
{
    const int realPos = (int)card;                                              // ReadRealPos 回 int（Direction==0 不翻）
    return (int)((double)realPos * r);                                          // ReadPos 回 int
}

Motor1203Speed MotorSpeedFromPct(int pct, const MotorGolden& g)
{
    Motor1203Speed sp;
    sp.s = 0; sp.velLow = sp.velHigh = sp.acc = sp.dec = 0.0; sp.skip = false;
    if (g.indexMotor) { sp.skip = true; return sp; }                            // TMyMotor::SetSpeed：Index 四軸整段空白
    double p = pct;
    if (p >= 100) p = 100;
    int s;
    if (g.zStack) s = (int)((g.jogHigh - g.jogLow) * (unsigned)(int)p / 100 + g.jogLow);
    else          s = (int)(g.jogHigh * (unsigned)(int)p / 100);
    sp.s = (unsigned)s;
    if (g.jogHigh == 0) { sp.skip = true; return sp; }                          // TMyEtherCatMotor::SetSpeed：PJogHighSpeed==0 → return
    double persent = (double)sp.s / g.jogHigh * 1.0;
    if (persent > 1.0) persent = 1.0;
    else if (persent < 0.01) persent = 0.01;
    sp.velLow = g.initSpeed * persent;
    sp.velHigh = g.jogHigh * persent;
    if (sp.velHigh < sp.velLow) sp.velHigh = sp.velLow;
    sp.acc = g.acc;
    sp.dec = g.dec;
    return sp;
}

// ---------------------------------------------------------------------------
bool MotorAccessParse(const std::string& json, MotorAccessReq& out, std::string& why)
{
    out = MotorAccessReq();
    cJSON* root = cJSON_Parse(json.c_str());
    if (root == 0 || !cJSON_IsObject(root)) {
        cJSON_Delete(root);
        why = "motor.access: value 不是 JSON 物件（要送 motor-access.js 組好的 request）";
        return false;
    }
    bool ok = true;
    const cJSON* it = 0;

    it = cJSON_GetObjectItemCaseSensitive(root, "seq");
    if (cJSON_IsNumber(it)) out.seq = (long long)it->valuedouble;
    struct { const char* k; std::string* dst; } strs[] = {
        { "id", &out.id }, { "source", &out.source }, { "button", &out.button },
        { "action", &out.action }, { "kind", &out.kind },
    };
    for (unsigned i = 0; i < sizeof(strs) / sizeof(strs[0]); ++i) {
        it = cJSON_GetObjectItemCaseSensitive(root, strs[i].k);
        if (cJSON_IsString(it) && it->valuestring) *strs[i].dst = it->valuestring;
    }

    it = cJSON_GetObjectItemCaseSensitive(root, "motors");
    if (it != 0 && !cJSON_IsNull(it)) {
        if (!cJSON_IsArray(it)) { ok = false; why = "motor.access: motors 必須是陣列"; }
        else {
            const cJSON* m = 0;
            cJSON_ArrayForEach(m, it) {
                if (!cJSON_IsString(m) || m->valuestring == 0) {
                    ok = false; why = "motor.access: motors 的元素必須是字串（Mot_Table 的 Alias）"; break;
                }
                out.motors.push_back(m->valuestring);
            }
        }
    }

    it = cJSON_GetObjectItemCaseSensitive(root, "params");
    if (ok && it != 0 && !cJSON_IsNull(it)) {
        if (!cJSON_IsObject(it)) { ok = false; why = "motor.access: params 必須是物件"; }
        else {
            const cJSON* p = 0;
            cJSON_ArrayForEach(p, it) {
                if (p->string == 0) continue;
                if (cJSON_IsNumber(p))    out.num[p->string]  = p->valuedouble;
                else if (cJSON_IsBool(p)) out.flag[p->string] = cJSON_IsTrue(p) != 0;
                else if (cJSON_IsString(p) && p->valuestring) out.str[p->string] = p->valuestring;   // W5-b：btn
                else if (cJSON_IsArray(p)) {                                                          // W5-b：targets（只收全是數字的陣列）
                    std::vector<double> v;
                    bool allNum = true;
                    const cJSON* e = 0;
                    cJSON_ArrayForEach(e, p) { if (cJSON_IsNumber(e)) v.push_back(e->valuedouble); else { allNum = false; break; } }
                    if (allNum) out.arr[p->string] = v;
                }
                else if (cJSON_IsObject(p)) {                                                         // AI(W906-W5-b) 20260925: fields＝{欄位 id: 值}
                    std::map<std::string, double> o;
                    const cJSON* e = 0;
                    cJSON_ArrayForEach(e, p) { if (e->string && cJSON_IsNumber(e)) o[e->string] = e->valuedouble; }   // 非數字（含 null）＝沒給
                    out.obj[p->string] = o;
                }
                // null 不存 —— 「沒給」與 0 是兩回事；字串陣列（edits／keys）C++ 不用
            }
        }
    }

    if (ok && out.action.empty()) { ok = false; why = "motor.access: 缺 action"; }
    if (ok && out.source.empty()) { ok = false; why = "motor.access: 缺 source"; }
    cJSON_Delete(root);
    return ok;
}

const char* MotorAccessActionStatus(const std::string& action)  // "live" | "ui" | "queued" | "blocked"（相依不存在或規則已裁決不做）| "unknown"
{
    const ActionRow* row = FindAction(action);
    return row ? row->status : "unknown";
}

MotorAccessOutcome MotorAccessDispatch(const MotorAccessReq& r, long long wireId,
                                       IMotorAccessBackend& be)
{
    if (r.source != "uMotorTest" && r.source != "uteach")
        return Refuse("motor.access: 不認得的 source '" + r.source + "'（只認 uMotorTest／uteach）");
    const ActionRow* row = FindAction(r.action);
    if (row == 0)
        return Refuse("motor.access: 不認得的 action '" + r.action + "'");

    //AI(W906-W5-b) 20260925: 覆核 W5B-R3 —— 這兩頁開著＝golden 的教導頁（或它帶出的 MotorTest）開著：golden 一進教導頁就 fAllMotorHome=false
    //  （FormShow uteach.cpp:1606；離開 main.cpp:27845、MotorTest 關閉 uteach.cpp:2439 也清）⇒ 下一次 START 先做全部歸零。網頁上看不到「開頁」，
    //  每一個教導頁／MotorTest 的命令都清一次（教導頁的 C 路開頁 FileRW_Teach_Page 也清）。
    be.GoldenClearAllMotorHome();
    const std::string status = row->status;
    if (status == "blocked")
        return Refuse(std::string("motor.access ") + r.action + ": 不做 —— " + row->wave + "。golden：" + row->golden);
    if (status == "queued") {
        std::string why = std::string("motor.access ") + r.action + ": 尚未接上（" + row->wave + "）。golden：" + row->golden;
        if (std::string(row->wave) == "W4-b")
            why += "。要先照 golden 補互鎖（IsMotorCanRun／安全門／HomeFlag／軟體極限），再落到 EastSun 的 1203 命令，不回假成功。";
        return Refuse(why);
    }
    if (status == "ui") {
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "ui-only", "ui", std::string("golden 這顆按鈕只動畫面，C++ 無動作：") + row->golden);
        MotorAccessOutcome o;
        o.ok = true;
        o.ackJson = Finish(w);
        return o;
    }
    if (r.action == "stop")        return DoStop(r, wireId, be);
    //AI(W906-W5-b) 20260925: 覆核 W5B-R4 —— golden 只在進頁時擋一次（main.cpp:27827-27828 sbTeachingClick `if(SystemStart) return;`；教導頁／MotorTest
    //  的按鈕本身不再看）⇒ golden 裡「運轉中按教導頁的鈕」不會發生。網頁的頁面在運轉中照樣開著（別的分頁或告警框按了 START），所以把入口那道
    //  搬到每一個命令：運轉中一律不做（停止在上面、ui 更上面，永遠放行；teachSet 的查詢與結束也放行 —— 手動教導中 START 本來就被擋）。
    const bool teachEndOrQuery = r.action == "teachSet" &&
        ((r.flag.count("query") && r.flag.find("query")->second) || (r.flag.count("start") && !r.flag.find("start")->second));
    if (be.GoldenSystemStart() && !teachEndOrQuery)
        return Refuse("motor.access " + r.action + "（" + r.source + "）: SystemStart==true（機台運轉中）—— golden 的教導頁／MotorTest 在運轉中進不去"
                      "（main.cpp:27827 sbTeachingClick `if(SystemStart) return;`），開著的頁面在運轉中一律不動；只有 STOP 放行");
    //AI(W906-W5-b) 20260925: W5B-5 —— 手動教導中（golden fTeachShow->ShowModal 開著）：教導頁除了 teachSet（結束／查詢）都擋；
    //  MotorTest 對同一軸的按鈕也擋（含伺服鈕）。停止在上面已經分派，永遠不擋。
    //  覆核 W5B-R1：教導頁的 teachSet 不再過「同軸」那一道（TeachAxisBusyWhy 只看 uMotorTest）。
    if (g_teachSet.active) {
        if (r.source == "uteach" && r.action != "teachSet")
            return Refuse("motor.access " + r.action + "（uteach）: 手動教導中（" + g_teachSet.btn + "）—— golden 的 fTeachShow 是對話框，開著時其他按鈕按不到；先按確定或取消");
        const std::string busy = TeachAxisBusyWhy(r, be);
        if (!busy.empty()) return Refuse(busy);
    }
    if (r.source == "uteach") TeachSpeedEvent(r, be);                           // AI(W906-W5-b) 20260925: R-W5B-4（頁面報的 golden 速度事件）
    if (r.action == "servoToggle") return DoServo(r, wireId, be);
    // W5-b：uteach 的運動鈕走 golden uteach 的互鎖（CheckCanMove／IsCanQuickJogMove／軟體極限吃目前位置…）
    if (r.source == "uteach") {
        if (r.action == "jogP" || r.action == "jogN") return DoTeachJog(r, wireId, be);
        if (r.action == "moveRelative")                 return DoTeachMoveRel(r, wireId, be);
        if (r.action == "moveAbsolute")                 return DoTeachMoveTo(r, wireId, be);
        if (r.action == "home")                         return DoTeachHome(r, wireId, be);
        if (r.action == "teachGo")                      return DoTeachButton(r, wireId, be);   // golden 按鈕的處理函式決定做什麼（W5B-2）
        if (r.action == "teachSet")                     return DoTeachSet(r, wireId, be);
    }
    if (r.action == "jogP" || r.action == "jogN") return DoJog(r, wireId, be);
    if (r.action == "moveRelative")                 return DoMoveRelative(r, wireId, be);
    if (r.action == "moveAbsolute")                 return DoMoveAbsolute(r, wireId, be);
    if (r.action == "moveSoftLimitP" || r.action == "moveSoftLimitN") return DoMoveSoftLimit(r, wireId, be);
    if (r.action == "home")                         return DoHome(r, wireId, be);
    if (r.action == "loopMove")                     return DoLoopMove(r, wireId, be);
    if (r.action == "resetMNet")                    return DoResetMNet(r, be);
    if (r.action == "setRangeAndInit" || r.action == "setRateAndInit") return DoRangeRate(r, be);
    if (r.action.compare(0, 3, "set") == 0 &&
        (r.action.find("Speed") != std::string::npos || r.action.compare(0, 12, "setSoftLimit") == 0))
        return DoSetParam(r, be);
    return Refuse("motor.access " + r.action + ": 表上標 live 但沒有實作（程式錯誤）");
}

}  // namespace ht9045

// =============================================================================
//  落差清單（golden／Steven 的頁面 vs EastSun 的實機做法；「衝突時跟 EastSun，落差寫下來」）
//  W4-a 範圍內的兩條：
//   1. servoToggle：golden uMotorTest 用函式內 static 輪流切（所有軸共用一個 static），
//      EastSun 的 SvOn 帶明確的 0／1。這裡以「頁面明講 > 這一軸實際狀態取反 > golden static」決定，
//      只有狀態讀不到時才退回 golden 的 static。
//   2. stop：golden 停「全部馬達」＋畫面上那一軸；EastSun 的停止是逐軸 —— 對每一個開成功的 1203 軸都送，
//      並照樣呼叫 golden StopAllMotor（非 1203 軸只有那條路）。停止不看控制權與互鎖。
//  W4-b 會碰到的四條（NB2 R1 §3，照通則跟 EastSun）：JOG 改 moveVel＋stop（Acm_AxJog 在這張卡 0 位移）、
//  HOME 用 DS402 124／128（golden 的卡片 mode 在這台回 0x8000510F）、SetPos(0) 不提供、SetExtDrive 不提供。
// =============================================================================
