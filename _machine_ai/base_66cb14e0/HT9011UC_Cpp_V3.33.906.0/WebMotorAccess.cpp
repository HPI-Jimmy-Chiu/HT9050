// =============================================================================
//  WebMotorAccess.cpp  --  `motor.access` 的解析與分派（純邏輯，只認 IMotorAccessBackend）
//
//  AI(W906-W4-MOTOR) 20260925.  設計與分層見 WebMotorAccess.h 檔頭，這裡不重複。
//  本檔刻意不 include 任何 god-stack 標頭：真實後端在 WebMotorAccessLive.cpp（只連進 wb_serve），
//  ctest（tests/test_web_motor_access.cpp）直接把本檔＋cJSON＋JsonWriter 編進去配假後端。
// =============================================================================
#include "WebMotorAccess.h"

#include <chrono>
#include <cstdio>
#include <ctime>
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
    { "setTeachFromCurrent", "queued", "W5",   "uteach.cpp btnSetToClick" },
    { "setTeachFromOffset",  "queued", "W5",   "uteach.cpp btnSetToOffsetClick" },
    { "teachSet",            "queued", "W5",   "uteach.cpp SetButton*Click" },
    { "teachGo",             "queued", "W5",   "uteach.cpp GoButton*Click" },
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
    if (ready) {
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
    if (needPos && m.g.direction) { m.why = r.action + " " + r.motors[0] + ": " + kDirPending; return false; }
    return true;
}

// 目前位置（使用者單位）。golden 讀 MOT[i].ReadPos()；1203 軸改讀監看器的 cmdPos 再照 golden 換算。
bool CurrentUserPos(IMotorAccessBackend& be, const MotionCtx& m, int& pos)
{
    if (!m.a.Is1203()) { pos = be.GoldenReadPos(m.a.motIndex); return true; }
    double card = 0.0;
    if (!be.Pci1203CmdPos(m.a.axis, card)) return false;
    pos = MotorCardToUser(card, m.g.gearRatio);
    return true;
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
            if (m.g.direction) return Refuse(who + ": " + kDirPending);
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
    if (m.g.armZ && m.g.direction)
        return Refuse(who + ": arm Z 歸零後 golden 要去 ZSafePos（絕對移動）—— " + kDirPending);
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

bool IsTeachMotion(const std::string& a)
{
    return a == "jogP" || a == "jogN" || a == "moveRelative" || a == "moveAbsolute" || a == "home";
}

}  // namespace

// ---------------------------------------------------------------------------
void MotorAccessTick(IMotorAccessBackend& be, int elapsedMs, bool operatorConnected)
{
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
    return j;
}

void MotorAccessResetJobs()
{
    g_jogs.clear();
    g_issuedPoll.clear();
    g_homes.clear();
    g_loop = LoopJob();
    g_lastJobNote.clear();
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
                // 字串／陣列（teachSet 的 motorIds／edits／keys）留給 W5；null 不存 —— 「沒給」與 0 是兩回事
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
    if (r.action == "servoToggle") return DoServo(r, wireId, be);
    // uteach 的運動鈕有自己的互鎖（CheckCanMove／IsCanQuickJogMove／軟體極限吃 edtNowPosition…），跟教導頁一起在 W5 接
    if (r.source == "uteach" && IsTeachMotion(r.action))
        return Refuse("motor.access " + r.action + "（uteach）: 教導頁的運動鈕尚未接上（W5）—— golden uteach 另有 CheckCanMove／IsCanQuickJogMove 等互鎖");
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
