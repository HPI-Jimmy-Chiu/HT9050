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
#include <direct.h>      //AI(W906-MT-E3c) 20260925: _mkdir -- the Light Scale saves (golden MyForceDirectories), the only files this file writes
#include <sys/stat.h>    //AI(W906-MT-E3c) 20260925: _stat (golden DirectoryExists)

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
//  28 個 action（motor-access.json 38 個命令去重）；AI(W906-MT-E2) 20260925：+4（43 個命令、32 個 action）；AI(W906-MT-E3c) 20260925：+5（48 個命令、37 個 action）。golden 欄 = 該按鈕事件在 golden 的位置，
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
    //AI(W906-MT-E3c) 20260925: was "blocked" (G9 closed, GaliMotorServoOff not translated) -- both landed in MT-E3b (0d253a0,
    //  EastSun R1: the brake family back to golden). Live now: DoMotorPower.
    { "motorPowerToggle",    "live",   "",     "uMotorTest.cpp:1621 btnMotorPowerClick" },
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
    { "reloadMotorData",     "live",   "",     "uMotorTest.cpp:1695 btnReloadMotorDataClick（AI(W906-MT-E1) 20260925：EastSun 開放 SetCmd/ActualPosition ⇒ 做 PCIL132_SetPos(0)；AI(W906-MT-E2) 20260925：InitialMotorParameter 的表值就地套回 MOT[]（不重建物件、不開軸），見 DoReloadMotorData）" },
    { "resetMNet",           "live",   "",     "uMotorTest.cpp:1718 btResetMNetClick" },
    //AI(W906-MT-E2) 20260925: the Motor Test functions that needed no ruling (contract with the web side: motor-access.json
    //  buttons labName / strngrdMotor / BitBtn1 / scrlbrMotorSpeed / edtSpeed, source uMotorTest, none of kind motion).
    { "selectMotor",         "live",   "",     "uMotorTest.cpp:734 lM00Click／:762 lpA00Click" },
    { "setParamCell",        "live",   "",     "uMotorTest.cpp:1176 strngrdMotorSelectCell" },
    { "copyFrom",            "live",   "",     "uMotorTest.cpp:1384 BitBtn1Click（來源 = cbbMotorName 的第 2、3 個字元）" },
    { "setSpeed",            "live",   "",     "uMotorTest.cpp:791 scrlbrMotorSpeedScroll（jog=true）／:1587 edtSpeedChange（jog=false）" },
    //AI(W906-MT-E3c) 20260925: the contract with the web side (motor-access.json FormShow / FormClose / BitBtn2 / BitBtn3 /
    //  btnSaveLogLightScaleData, source uMotorTest; only lightScale is kind motion).
    { "formShow",            "live",   "",     "uMotorTest.cpp:985 FormShow（C++ 那一半：bSingleHome=false、按鈕抬起、Motor Power 同步、ActiveIndex=-1）" },
    { "formClose",           "live",   "",     "uMotorTest.cpp:1347 FormClose（C++ 那一半：fShow=false、PauseUT150Polling=false；MotorTest.ini 由頁面寫，R5）" },
    { "lightScale",          "live",   "",     "uMotorTest.cpp:2096 BitBtn2Click（LightScale :1795、Timer2Timer :2031）" },
    { "lightScaleSave",      "live",   "",     "uMotorTest.cpp:2102 BitBtn3Click（SaveAsCSV :1749）" },
    { "lightScaleDataSave",  "live",   "",     "uMotorTest.cpp:2051 btnSaveLogLightScaleDataClick（SaveAsCSV_Kaichen :1982）" },
    //AI(W906-MERGE-56bbf785) 20260926: machine rows above (MT-E2 / MT-E3c) + the laptop's W5-b teach rows below (were "queued" W5 on
    //  the machine) -- 48 commands, 37 actions, no queued / blocked row left.
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
//AI(W906-MERGE-56bbf785) 20260926: 1203 軸的教導頁 jog 不再用這裡（改用 DoJog 的 g_jogSpd／JogPctOf，見 DoTeachJog）；這裡只剩移動（Move1203）用。
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

//AI(W906-MT-E2) 20260925: golden TfMotorTest page state that lives across buttons.
//  g_selMotor           = golden ActiveIndex, as the Alias (set by lM00Click / lpA00Click; golden FormShow :1053 starts it at -1 = "").
//  g_jogPct[mi]         = the scroll-bar position of the last SetSpeed(pos, true) (scrlbrMotorSpeedScroll :801) for that
//                         motor since it was selected. golden JogP/JogN drop their speed argument (Motor/mymotor.cpp:1322-1335),
//                         so a jog runs at whatever the last SetSpeed(x, true) put into CFG_AxJog*. User ruling 20260925
//                         「Jog = golden」. Nothing set since the selection = 1 (golden lM00Click :755 scrlbrMotorSpeed->Position=1)
//                         -- never a speed left over from before, never an unknown one.
std::string        g_selMotor;
std::map<int, int> g_jogPct;
int JogPctOf(int mi)
{
    std::map<int, int>::const_iterator it = g_jogPct.find(mi);
    return it == g_jogPct.end() ? 1 : it->second;
}
//AI(W906-MT-FIX1) 20260926: g_jogSpd[mi] = the jog-family VALUES written with that position (review medium: "jog speed is
//  recalculated at every press"). golden fixes CFG_AxJog* at the moment of the scroll (PJogHighSpeed*pct, InitSpeed*pct,
//  dAcc/dDec as they were then); a later cell edit / Copy From / Reload / btnHighSpeed only touches memory, so golden's next jog
//  still runs at the old card values. The port used to re-derive them from the CURRENT MOT[] parameters at every press -- e.g.
//  10x faster right after Copy From from a faster motor, or tens of Mpps after a row-2 keypad slip, without the bar moving.
//  Set at selection (1%) and on every successful setSpeed(jog=true); DoJog re-sends exactly these.
std::map<int, Motor1203Speed> g_jogSpd;
//AI(W906-MT-E3c) 20260925:
//  g_ptpPct[mi] = the percentage of the last golden SetSpeed(x) this file sent for that motor's PTP family (selection = 1,
//                 setSpeed, a MoveP/MoveN/Loop start that carried a speed). An All-mode loop starts every OTHER motor at it
//                 (lead default 20260925: "other axes start with their last golden speed pct (1 if never set)") -- golden
//                 leaves each motor at its last SetSpeed; re-sending it puts back what a DS402 HOME or EastSun's 1203 page
//                 may have left in the card's PTP family since.
//  g_mtShown    = golden TfMotorTest::fShow as C++ last heard it: formShow / selectMotor -> true, formClose -> false.
//                 Used to keep the torque SDO focus only while the page is open (WebMotorAccessLive.cpp).
std::map<int, int> g_ptpPct;
int PtpPctOf(int mi)
{
    std::map<int, int>::const_iterator it = g_ptpPct.find(mi);
    return it == g_ptpPct.end() ? 1 : it->second;
}
bool g_mtShown = false;

// AI(W906-MT-E1) 20260925: golden TMyEtherCatMotor::DecStop (golden Motor/myEthercatmotor.cpp:415-437) for a 1203
//   axis -- Acm_AxStopDec, then Acm_AxSetExtDrive(ax, 0) (leave jog mode; golden then sets bFirstClickJog=true, so
//   the next jog sends ExtDrive(1) again). golden sends the ExtDrive(0) even when StopDec failed (it only pops
//   WAR16122). EVERY stop of a 1203 axis in this file goes through here, so no path -- jog release, STOP, the
//   dead-man, an alarm, a home/loop cancel -- can leave an axis in ext-drive (jog) mode.
//   User EastSun 20260925: jog the golden way (Acm_AxSetExtDrive + Acm_AxJog) -- 「開放，照舊版做法」.
Pci1203CmdResult Stop1203(IMotorAccessBackend& be, int axis, long long wireId)
{
    Pci1203Cmd c;
    c.kind = kCmdAxStop; c.wireId = wireId; c.axis = axis;
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    Pci1203Cmd x;
    x.kind = kCmdAxSetExtDrive; x.wireId = wireId; x.axis = axis; x.value = 0.0;
    be.Pci1203Execute(x);
    return res;
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
        const Pci1203CmdResult res = Stop1203(be, a.axis, wireId);   // AI(W906-MT-E1): golden PCIL132_StopMotor -> DecStop (StopDec + ExtDrive 0)
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
void EndSingleHome(IMotorAccessBackend& be, const std::string& why);   //AI(W906-MT-E3c) 20260925: golden bSingleHome=false (defined with the jobs)

// golden StopAllMotor 的 1203 那一半：對監看器每一個開成功的軸送 kCmdAxStop（DoStop 與教導頁 HOME 抬起共用；AI(W906-W5-b) 20260925 抽出來）
//AI(W906-MERGE-56bbf785) 20260926: machine DoStop = per-axis Stop1203 (StopDec + ExtDrive 0, MT-E1: every 1203 stop in this file
//  leaves jog mode), laptop = the same loop moved into this helper with a bare kCmdAxStop. Kept the laptop's helper (the teach
//  page's HOME release uses it too) with the machine's stop inside, so neither path can leave an axis in ext-drive mode.
void Stop1203All(IMotorAccessBackend& be, long long wireId, int& sent, int& accepted, int& refused, std::string& firstRefusal)
{
    const int n = be.Pci1203AxisCount();
    for (int ax = 0; ax < n; ++ax) {
        if (!be.Pci1203AxisOpened(ax)) continue;
        const Pci1203CmdResult res = Stop1203(be, ax, wireId);          // AI(W906-MT-E1): StopDec + ExtDrive 0 (golden DecStop)
        if (!CmdFailed(res)) { ++accepted; if (res.issued) ++sent; }
        else { ++refused; if (firstRefusal.empty()) firstRefusal = CmdWhy(res); }
    }
}

MotorAccessOutcome DoStop(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (IsJogButton(r.button)) return DoJogRelease(r, wireId, be);
    be.GoldenStopAll(r.source);
    CancelAllJobs(be, "STOP (golden AllBtnUp: btnHome/btnLoopMove up, bSingleHome=false)");
    //AI(W906-MT-E3c) 20260925: golden btnStopClick :1651 `bSingleHome=false;` -- also releases Light Scale's single home
    //  (its case 1 then goes on to the scan, golden). The Light Scale timer itself is NOT stopped (R7: golden STOP does not).
    if (r.source == "uMotorTest") EndSingleHome(be, "STOP (golden btnStopClick :1651 bSingleHome=false)");
    g_jogs.clear();

    std::string why1203;
    const bool ready = be.Pci1203Ready(why1203);
    int sent = 0, accepted = 0, refused = 0;   // NB2 R19 RW4-1：sent＝真的呼叫了卡（issued），accepted 另計 —— dry 控制層只會 accepted
    std::string firstRefusal;
    if (ready) Stop1203All(be, wireId, sent, accepted, refused, firstRefusal);   // AI(W906-MERGE-56bbf785): the machine's per-axis Stop1203 loop, now in the helper

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

//AI(W906-MT-E3c) 20260925: the body of MotionPrelude, for one named motor (the All-mode loop and Light Scale have no
//  r.motors[0] of their own).
bool MotionPreludeFor(const std::string& action, const std::string& motorId, long long wireId, IMotorAccessBackend& be,
                      MotionCtx& m, bool needPos)
{
    if (!be.Resolve(motorId, m.a)) { m.why = action + ": 馬達表上沒有 " + motorId; return false; }
    if (m.a.motIndex < 0 || !be.GoldenMotor(m.a.motIndex, m.g)) {
        m.why = action + " " + motorId + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL）"; return false;
    }
    if (!m.g.selectable) { m.why = NotSelectable(action + " " + motorId); return false; }
    //AI(W906-MT-E3c) 20260925: EASTSUN R2 -- a Mot_Table row whose CardModel is PCI1203 is a 1203 axis, whatever
    //  INDEX_MOTION_CARD says (M14 MTestZ1 on HT9050: INDEX_MOTION_CARD stays 0, Gerneral.ini untouched). So the Galil
    //  refusal applies to a NON-1203 Index row only; the live backend already reports galilIndex=false for a 1203 row, the
    //  Is1203() test here is the second lock (was: tested before Is1203, which refused M14 -- MT-E3b api_for_next 12).
    if (m.g.galilIndex && !m.a.Is1203()) {
        m.why = action + " " + motorId + ": INDEX_MOTION_CARD==0 的 Index 軸走 golden 的 Galil 分支，motor.access 尚未接（HT9050 應設 INDEX_MOTION_CARD=1）";
        return false;
    }
    if (!m.a.Is1203()) return true;
    if (!m.a.tableEnable) { m.why = action + " " + motorId + ": " + kEnable0Why; return false; }   // NB2 R22
    std::string why;
    if (!be.Pci1203Ready(why)) { m.why = action + " " + motorId + "（PCI1203）: 1203 控制層不可用 —— " + why; return false; }
    if (m.a.axis < 0) {
        m.why = action + " " + motorId + "（PCI1203）: " + m.a.why;
        be.Pci1203NoteRefusal(wireId, "motor.access " + action, m.why);
        return false;
    }
    //AI(W906-DIR6B) 20260925: RULINGS_20260925 第 13 條（6B）「1203 軸不看 Direction，方向交給驅動器 Pn000」—— 不再以 Direction=1 拒絕
    //AI(W906-MERGE-56bbf785) 20260926: machine had only moved this test into MotionPreludeFor (MT-E3c refactor, no rule change);
    //  the laptop removed it by EastSun's ruling 6B -- the ruling is taken, here and in the machine's other Direction=1 holds
    //  (StartHome1203, GoldenMove1203, DoLoopMoveAll, RecordLastHomePos, Live GoldenLightScaleEncoder). HT9050 Mot_Table: every
    //  PCI1203 row is Direction=0 (read 20260926), so no behaviour changes on this machine today. (needPos is kept in the signature.)
    (void)needPos;
    return true;
}

bool MotionPrelude(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, MotionCtx& m, bool needPos)
{
    if (r.motors.empty()) { m.why = r.action + ": 沒有選馬達（golden: ActiveIndex==-1 → return）"; return false; }
    return MotionPreludeFor(r.action, r.motors[0], wireId, be, m, needPos);
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
// AI(W906-MT-E1) 20260925: setJog = golden TMyEtherCatMotor::SetSpeed(x, bSetJog=true) (golden
//   Motor/myEthercatmotor.cpp:618-726): the same four values ALSO go to the jog family CFG_AxJogVelLow / High /
//   Acc / Dec, which is what Acm_AxJog runs on (the PTP family alone would leave the jog at its old speed).
//   golden sets them from the speed scroll bar (uMotorTest.cpp:801 SetSpeed(pos, true)); the web page has no
//   scroll-bar event, so the jog sends them with the current percentage right before each jog.
//   [AI(W906-MT-E2) 20260925: superseded -- the page now sends the scroll bar (setSpeed jog=true, DoSetSpeed), and a jog
//   re-sends only the jog family at that position (DoJog).]
//   ⓘ golden also writes CFG_AxJogVLTime = 0 (an I32 property); Pci1203Control has no I32 setter, so it is not
//   written (gap, noted in the ack).
//AI(W906-MT-E2) 20260925: `families` picks which half goes to the card -- kFamPtp = golden SetSpeed(x) (edtSpeedChange,
//   MoveP/MoveN, LoopMove start), kFamPtp|kFamJog = golden SetSpeed(x, true) (the speed scroll bar, :801), kFamJog alone =
//   the jog press re-sending the jog family of the last scroll position (user ruling 20260925 「Jog = golden」: golden
//   JogP/JogN ignore their speed argument, so the PTP family must not be touched by a jog -- btnGo runs on it).
const int kFamPtp = 1, kFamJog = 2;
//AI(W906-MT-FIX1) 20260926: split so a jog can re-send the EXACT values the scroll bar wrote (see g_jogSpd below).
bool Send1203SpeedValues(IMotorAccessBackend& be, long long wireId, const MotionCtx& m, int pct, const Motor1203Speed& sp,
                         std::string& why, std::string& note, int families)
{
    if (sp.skip) { note = "golden SetSpeed 不設速度（index 軸，或 PJogHighSpeed==0）"; return true; }
    const struct { Pci1203SpeedParam which; double v; } seq[8] = {
        { kSpeedInit, sp.velLow }, { kSpeedRun, sp.velHigh }, { kSpeedAcc, sp.acc }, { kSpeedDec, sp.dec },
        { kSpeedJogInit, sp.velLow }, { kSpeedJogRun, sp.velHigh }, { kSpeedJogAcc, sp.acc }, { kSpeedJogDec, sp.dec } };
    for (int i = 0; i < 8; ++i) {
        if (!(families & (i < 4 ? kFamPtp : kFamJog))) continue;
        Pci1203Cmd c;
        c.kind = kCmdAxSetSpeed; c.wireId = wireId; c.axis = m.a.axis; c.speed = seq[i].which; c.value = seq[i].v;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        if (CmdFailed(res)) { why = "1203 設定速度失敗 —— " + CmdWhy(res); return false; }
    }
    //AI(W906-W5-b) 20260925: R-W5B-4（golden MOT.SetSpeed 之後 MOT 記住的速度）
    //AI(W906-MERGE-56bbf785) 20260926: laptop recorded every Send1203Speed (then PTP only); the machine split the families -- a
    //  golden SetSpeed(x[, true]) always writes the PTP family, a jog-family-only re-send is not a SetSpeed, so only kFamPtp records.
    if (families & kFamPtp) g_goldenPct[m.a.motIndex] = pct;
    char b[200];
    std::snprintf(b, sizeof(b), "speed %d%% -> s=%u velLow=%.1f velHigh=%.1f acc=%.1f dec=%.1f (%s)",
                  pct, sp.s, sp.velLow, sp.velHigh, sp.acc, sp.dec,
                  families == (kFamPtp | kFamJog) ? "PTP + jog" : (families == kFamJog ? "jog family only" : "PTP"));
    note = b;
    return true;
}
bool Send1203Speed(IMotorAccessBackend& be, long long wireId, const MotionCtx& m, int pct,
                   std::string& why, std::string& note, int families = kFamPtp)
{
    return Send1203SpeedValues(be, wireId, m, pct, MotorSpeedFromPct(pct, m.g), why, note, families);
}

//AI(W906-MT-E2) 20260925: an ack's "cur" = the same keys as /api/struct/motor/runtime "cur" (JsonBridge/ChanMotorPoints.cpp),
//  read back from the golden object AFTER the action -- what C++ actually holds, not what the page computed.
//  Rows 1..10 of golden strngrdMotor are UpdateMotorParameter (uMotorTest.cpp:655-669): initSpeed, jogHigh, jogLow, homeHigh,
//  homeLow, softP, softN, acc (ReadAcc = the database value), dec, range.
void WriteCur(webbridge::JsonWriter& w, const MotorGolden& g)
{
    w.Key("cur").BeginObject();
    w.Key("enable").Bool(g.enable);
    w.Key("selectable").Bool(g.selectable);                                     //AI(W906-MT-FIX1) 20260926: golden lM00Click :740-743 per build
    w.Key("initSpeed").Number((wb_int64)g.initSpeed);
    w.Key("jogHigh").Number((wb_int64)g.jogHigh);
    w.Key("jogLow").Number((wb_int64)g.jogLow);
    w.Key("homeHigh").Number((wb_int64)g.homeHigh);
    w.Key("homeLow").Number((wb_int64)g.homeLow);
    w.Key("softP").Number((wb_int64)g.softP);
    w.Key("softN").Number((wb_int64)g.softN);
    w.Key("acc").Number(g.accDb);
    w.Key("dec").Number(g.decDb);
    w.Key("range").Number((wb_int64)g.range);
    w.Key("rate").Number((wb_int64)g.rate);
    w.Key("readSpeed").Number((wb_int64)g.readSpeed);
    w.Key("lastHomePos").Number((wb_int64)g.lastHomePos);
    w.Key("gearRatio").Number(g.gearRatio);
    w.Key("homeFlag").Number((wb_int64)g.homeFlag);
    w.EndObject();
}
void WriteCurOf(webbridge::JsonWriter& w, IMotorAccessBackend& be, int mi)
{
    MotorGolden g;
    if (mi >= 0 && be.GoldenMotor(mi, g)) WriteCur(w, g);
    else w.Key("cur").Null();
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
        Stop1203(be, m.a.axis, wireId);                                         // AI(W906-MT-E1): golden PCIL132_StopMotor = DecStop
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
    if (hasPct) g_ptpPct[m.a.motIndex] = pct;                                   //AI(W906-MT-E3c) 20260925: golden SetSpeed(pct) -> this motor's last PTP pct
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
    if (hasPct) g_ptpPct[m.a.motIndex] = pct;                                   //AI(W906-MT-E3c) 20260925: golden MOT.SetSpeed(pct)
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
    //AI(W906-MT-E2) 20260925: JOG SPEED = GOLDEN (user ruling 20260925). golden reads SelMotSpeed=atoi(edtSpeed) (:832/:882)
    //  and passes it to MOT.JogP/JogN, which DROP it (Motor/mymotor.cpp:1322-1335): the jog runs at the last
    //  SetSpeed(x, true), i.e. the speed scroll bar (:801). So the page's `speed` is no longer used (nor required); the
    //  pct is the last setSpeed(jog=true) of this motor since it was selected, 1 when there was none (JogPctOf).
    //  (Was: the jog family was set from edtSpeed right before each jog -- a stand-in while the page had no scroll bar.)
    const int pct = JogPctOf(m.a.motIndex);
    //AI(W906-MT-FIX1) 20260926: the values the bar wrote (g_jogSpd), not a fresh MotorSpeedFromPct of today's parameters.
    std::map<int, Motor1203Speed>::const_iterator js = g_jogSpd.find(m.a.motIndex);
    const Motor1203Speed jogSp = (js != g_jogSpd.end()) ? js->second : MotorSpeedFromPct(pct, m.g);
    const bool positive = (r.action == "jogP");
    if (!m.a.Is1203()) {
        be.GoldenJog(m.a.motIndex, positive, pct);                              // MOT[i].JogP/JogN（TMyMotor 內含安全門檢查）
        { JogRec j = { false, -1, m.a.motIndex, r.motors[0] }; ForgetJog(false, -1, m.a.motIndex); g_jogs.push_back(j); }
        return OkAck(r, "jogging", "MOT", std::string("MOT[") + std::to_string(m.a.motIndex) + "]." +
                     (positive ? "JogP" : "JogN") + "(" + std::to_string(pct) + ") -- jog speed = last scroll position (golden)", m, 0, false, 0, false);
    }
    if (be.GoldenSafeDoorOpen(m.a.motIndex))                                    // TMyMotor::JogP：CheckIsSafeDoorOpen() → return
        return Refuse(who + ": 安全門開著（golden TMyMotor::JogP 的 CheckIsSafeDoorOpen）");
    if (SampleStale(be, m.a.axis)) return Refuse(who + ": " + kStaleWhy);    // NB2 R21 W4B-5
    std::string why, note;
    //AI(W906-MT-E2) 20260925: only the jog family (CFG_AxJog*), at the last scroll position -- what golden's card holds at
    //  this moment. The PTP family is left alone (golden's jog never touches it; btnGo runs on it).
    if (!Send1203SpeedValues(be, wireId, m, pct, jogSp, why, note, kFamJog)) return Refuse(who + ": " + why);
    // AI(W906-MT-E1) 20260925: THE GOLDEN JOG (user EastSun 20260925 「開放，照舊版做法」), replacing main's
    //   kCmdAxMoveVel. golden TMyEtherCatMotor::JogP/JogN (golden Motor/myEthercatmotor.cpp:441-535): first click
    //   Acm_AxSetExtDrive(ax, 1) ("Setting 1 is jog mode"), then Acm_AxJog(ax, Direction ? 1 : 0) for JogP and
    //   (Direction ? 0 : 1) for JogN; the release is DecStop = StopDec + ExtDrive(0) (Stop1203 above), which also
    //   resets golden's bFirstClickJog -- so every press sends ExtDrive(1) again, exactly like here.
    //   Direction: 1203 axes with Direction=1 are refused earlier (kDirPending); with Direction=0 JogP is
    //   DIRECTION_POS = wire +1 (Pci1203Control WireDirToVendor), JogN is DIRECTION_NEG = wire -1.
    //   [AI(W906-MERGE-56bbf785) 20260926: ruling 6B (RULINGS_20260925 #13) -- a 1203 axis ignores Direction (the drive's Pn000
    //   owns it), so Direction=1 is no longer refused and JogP is always wire +1.]
    //   ⚠ main had switched to MoveVel because "Acm_AxJog returns SUCCESS with 0 displacement on this card"
    //   (web/js/pci1203/view.js) -- that measurement was a jog WITHOUT ExtDrive(1). Verify on the machine.
    {
        Pci1203Cmd x;
        x.kind = kCmdAxSetExtDrive; x.wireId = wireId; x.axis = m.a.axis; x.value = 1.0;
        const Pci1203CmdResult xr = be.Pci1203Execute(x);
        if (CmdFailed(xr)) return Refuse(who + ": 1203 進入 jog 模式（Acm_AxSetExtDrive 1）失敗 —— " + CmdWhy(xr));
    }
    Pci1203Cmd c;
    c.kind = kCmdAxJogStart; c.wireId = wireId; c.axis = m.a.axis; c.dir = positive ? 1 : -1;   // Direction==0：JogP = DIRECTION_POS
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) { Stop1203(be, m.a.axis, wireId); return Refuse(who + ": 1203 失敗 —— " + CmdWhy(res)); }   // leave jog mode again
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
    WriteCurOf(w, be, m.a.motIndex);   //AI(W906-MT-E2) 20260925: golden UpdateMotorParameter() after the button -- the page redraws from what C++ stored
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
    // AI(W906-MT-E1) 20260925: card-side home (golden EtherCatMotHome, non-DS402 drives only): after READY wait
    //   golden's HomeDelay 0.3 s, then SetCommand(0) + SetPosition(0) (golden case 20). -1 = READY not seen yet.
    bool        cardSide, zeroed;
    int         zeroWaitMs;
    //AI(W906-MT-E3c) 20260925: started by Light Scale's case 0 (golden LightScale :1836-1840 sets iSingleHomeIndex /
    //  bSingleHome WITHOUT btnHome->Down). Such a job is golden's bSingleHome only: an AllBtnUp (btnHome->Down=false) does not
    //  end it, VerifyMotorAction's "Homeing" lock does not see it (golden sees it through MotionDone instead), and the motion
    //  buttons' `btnHome->Down` guard does not refuse for it.
    bool        fromLS;
    HomeJob() : active(false), mi(-1), axis(-1), phase(0), sawHoming(false), elapsedMs(0), readyMs(0), zTarget(0), zIssued(false),
                cardSide(false), zeroed(false), zeroWaitMs(-1), fromLS(false) {}
};
//AI(W906-MT-E3c) 20260925: golden TMyMotor's MotorMove state (iOldPos, fCMD -- members of MOT[i], golden mymotor.cpp:871-962),
//  for a 1203 axis driven the golden way by the All-mode loop and Light Scale (GoldenMove1203). One per motor, like golden's
//  members (MOT[] is static, so both start at 0/false); golden btnLoopMoveClick's InitMOTParameter clears fCMD.
struct GoldenMoveState {
    bool fCMD;
    int  oldPos;
    GoldenMoveState() : fCMD(false), oldPos(0) {}
};
std::map<int, GoldenMoveState> g_gm;
//AI(W906-MT-E3c) 20260925: one motor of an All-mode loop (golden MotorTestClass[i] with cbUsing->Checked).
struct LoopAxis {
    std::string     motorId;
    MotorAccessAxis a;
    MotorGolden     g;
    int             pos1, pos2;       // golden atoi(MotorTestClass[i]->edPos1/edPos2->Text), sent as pos1.<Alias> / pos2.<Alias>
    bool            flag;             // golden MotorTestClass[i]->AllMotMoveFlag of this tick
    int             lastRet;          // the last golden MotorMove return (for the ack / note)
    LoopAxis() : pos1(0), pos2(0), flag(false), lastRet(0) {}
};
struct LoopJob {
    bool        active;
    std::string motorId;
    MotorAccessAxis a;
    MotorGolden g;
    int         pos1, pos2, waitMs;
    int         task;         // 1 = 去 pos1、10 = 等、50 = 去 pos2、60 = 等（golden DoLoopMove 的 Task 值）
    bool        cmdIssued;
    //AI(W906-MT-E2) 20260925: absolute ms on LoopNow() (was legMs/waitLeft accumulated from the beat's elapsed ms), so the
    //  500 ms beat and the post-Poll tick (MotorAccessPollTick) can both advance the same job.
    double      legStartMs, waitUntilMs;
    int         wrongReady;   // fresh READY samples away from the target; the 2nd one = "stopped before target" (was: 2nd beat)
    unsigned long wrongPoll;  // poll count of the last sample counted -- one sample is counted once, whoever looks at it
    unsigned long count;
    std::string lastWhy;
    //AI(W906-MT-E3c) 20260925: golden select->ItemIndex==1 (All). motorId/a/g are then the SELECTED motor (golden ActiveIndex,
    //  whose HomeFlag alone was asked at the start, :1316); the motors that move are `axes` (cbUsing-checked, may be none).
    bool        all;
    std::vector<LoopAxis> axes;
    LoopJob() : active(false), pos1(0), pos2(0), waitMs(0), task(1), cmdIssued(false), legStartMs(0.0), waitUntilMs(0.0),
                wrongReady(0), wrongPoll(0), count(0), all(false) {}
};
std::vector<HomeJob> g_homes;
LoopJob              g_loop;
std::string          g_lastJobNote;
bool                 g_lastSafeLock = false;   //AI(W906-MT-FIX1) 20260926: the beat's IsSafeLockCheck() result, for the post-Poll gate

//AI(W906-MT-E3c) 20260925: golden TfMotorTest's Light Scale state (uMotorTest.cpp:1789-1978). The statics of golden
//  LightScale() are members here and, like golden's, survive a reset except the ones LightScale(true) assigns
//  (iPitch, iDelayTime, iMinLim, iMaxLim, iNeedMovePos, Task, bStopflag). `timer` is Timer2->Enabled (dfm :3696-3698:
//  Enabled=False, Interval=10). `memo` is Memo1->Lines. `homePending` is golden bSingleHome as case 0 set it -- case 1
//  waits for it to drop, and it drops exactly where golden's bSingleHome drops (a single home that ended with HomeFlag==1,
//  golden MainProc :30409-30413 of the port; STOP; VerifyMotorAction's unlock; an alarm; FormShow; btnHome released).
struct LightScaleJob {
    bool   timer, stopFlag, editsEnabled, homePending;
    int    task;
    int    pitch, delayMs, moveItem, movePitch, needMoveLim, maxLim, minLim, needMovePos, lightScalePos, position, useAxisItem;
    double delayUntil;                // golden DelayTime.SetMSAndOn(iDelayTime) (a TQPF_Timer, uMotorTest.cpp:1789)
    bool   delayOver;                 // the delay has run out on the clock ...
    unsigned long delayPoll;          // ... at this monitor poll count: a 1203 reading must come from a LATER poll
    std::vector<std::string> memo;
    std::string lastSaved, lastNote, encoderSrc;
    LightScaleJob() : timer(false), stopFlag(false), editsEnabled(true), homePending(false), task(-1), pitch(0), delayMs(0),
                      moveItem(0), movePitch(0), needMoveLim(0), maxLim(0), minLim(0), needMovePos(0), lightScalePos(0),
                      position(0), useAxisItem(-1), delayUntil(0.0), delayOver(false), delayPoll(0) {}
};
LightScaleJob g_ls;
int  g_lsUseAxis  = 0;      // golden TfMotorTest::iUseAxis (uMotorTest.h:372): never initialised by golden -> 0 (VCL zero-fills a form)
int  g_lsAxisItem = -1;     // golden rgAxis->ItemIndex (dfm writes none -> -1)
int  g_lsMoveType = -1;     // golden rgMoveType->ItemIndex
//AI(W906-MT-E3c) 20260925: Motor Power On's cross-tick DoMotorPowerOn (lead default: the golden 1 s busy wait must not
//  freeze the tick thread). true between W906_DoMotorPowerOnBegin() and the Step() that returns true; then
//  SW[SwServerON].On() -- golden's next line after DoMotorPowerOn() (:1635 / :1042).
bool g_powerPending = false;
std::string g_powerFrom;

//AI(W906-MT-E2) 20260925: golden Loop timing -- TQPF_Timer tLoopMoveTimer + DWORD Average (golden uMotorTest.cpp:393-610;
//  TQPF_Timer::LatchCycleTime, myTimer.cpp:123-138: QueryPerformanceCounter, int ms of a float):
//    every tick of a leg : `if(flag==false){ flag=true; tLoopMoveTimer.LatchCycleTime(true); }` then MotorMove(p)
//    pos1 reached        : flag=false; lblJogPTime = LatchCycleTime();                                     (:451-452)
//    pos2 reached        : flag=false; lblJogNTime = LatchCycleTime(); dwLoopCount++; Average += LatchCycleTime();
//                          lblAvgTime = ChangeToFloatNonPcnt((double)Average, (double)dwLoopCount)  -- pos2 legs ONLY (:554-561)
//    btnLoopMoveClick    : dwLoopCount=0 on both branches; Average=0 and lblAvgTime=0 ONLY when HomeFlag!=0 (:1326 / :1332-1334)
//  ⚠ golden's `flag` is a function `static` (:399) that only an arrival clears: a loop stopped in the middle of a leg
//    leaves it set, and the NEXT loop's first leg is then timed from that old latch. Kept as golden has it (it shows only as
//    a long first-leg time after a mid-leg stop); MotorAccessResetJobs (tests) is the only other reset.
bool          g_latchArmed  = false;
double        g_latchMs     = 0.0;
unsigned long g_loopAverage = 0;        // golden `DWORD Average;` (uMotorTest.h:377)
bool          g_hasJogP = false, g_hasJogN = false, g_hasAvg = false;
int           g_jogPTime = 0, g_jogNTime = 0;
double        g_avgTime  = 0.0;
double        g_beatClockMs = 0.0;      // the clock when the backend has none (MonotonicMs() < 0): the sum of the beats' elapsedMs

double LoopNow(IMotorAccessBackend& be)
{
    const double t = be.MonotonicMs();
    return t >= 0.0 ? t : g_beatClockMs;
}
int LoopLatchMs(double nowMs) { return (int)(float)(nowMs - g_latchMs); }     // golden: `int itmp=ChangeToFloatNonPcnt(...)` (a float)
void LoopLegTimed(LoopJob& L, double nowMs)                                    // the arrival half of the table above
{
    g_latchArmed = false;
    const int t = LoopLatchMs(nowMs);
    if (L.task == 1) { g_jogPTime = t; g_hasJogP = true; return; }
    g_jogNTime = t; g_hasJogN = true;
    ++L.count;                                                                  // golden dwLoopCount++
    g_loopAverage += (unsigned long)t;                                          // golden Average+=LatchCycleTime() (its 2nd call: the same instant here)
    g_avgTime = (double)(float)((double)g_loopAverage / (double)L.count);
    g_hasAvg = true;
}

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
    if (sendStop) Stop1203(be, h.axis, -1);                                    // AI(W906-MT-E1): StopDec + ExtDrive 0
    h.active = false;
    g_lastJobNote = "home " + h.motorId + ": " + why;
}
void CancelLoop(IMotorAccessBackend& be, bool sendStop, const std::string& why)
{
    if (!g_loop.active) return;
    if (sendStop) {
        if (g_loop.a.Is1203()) Stop1203(be, g_loop.a.axis, -1);                // AI(W906-MT-E1): StopDec + ExtDrive 0
        else be.GoldenStopMotor(g_loop.a.motIndex);
    }
    g_loop.active = false;
    g_loop.lastWhy = why;
    g_lastJobNote = "loop " + g_loop.motorId + ": " + why;
}
// golden btnStopClick 的 AllBtnUp()（btnHome／btnLoopMove 抬起）＋ bSingleHome=false：取消所有跨拍工作（軸已被 StopAllMotor 停下）
//AI(W906-MT-E3c) 20260925: split in two, because golden's AllBtnUp (buttons up) and bSingleHome=false are two statements
//  and Light Scale tells them apart: CancelAllJobs = the buttons (btnHome's HOME jobs -- the file's documented safe-direction
//  reading of btnHome->Down=false -- and the loop); a HOME job Light Scale started is golden bSingleHome only and is left
//  running. EndSingleHome = golden `bSingleHome=false` (every single home, and Light Scale's case 1 is released).
void CancelAllJobs(IMotorAccessBackend& be, const std::string& why)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (!g_homes[i].fromLS) CancelHome(be, g_homes[i], false, why);
    CancelLoop(be, false, why);
}
void EndSingleHome(IMotorAccessBackend& be, const std::string& why)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) CancelHome(be, g_homes[i], false, why);
    if (g_ls.homePending) { g_ls.homePending = false; g_ls.lastNote = "bSingleHome=false (" + why + ") -> Light Scale case 1 goes on"; }
}
//AI(W906-MT-E3c) 20260925: golden MainProc `if(ProcessSingleMotorHome(idx) && MOT[idx].HomeFlag==1) bSingleHome=false;`
//  (port csystem.cpp:30407-30413): a single home that ENDED WITH HomeFlag==1 drops bSingleHome. One flag in golden, so any
//  home's success releases Light Scale's case 1 (a failure -- HomeFlag 2 -- does not: golden keeps waiting).
void SingleHomeSucceeded()
{
    if (g_ls.homePending) { g_ls.homePending = false; g_ls.lastNote = "single home done (HomeFlag=1) -> Light Scale case 1 goes on"; }
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
        if (g_homes[i].active && !g_homes[i].fromLS)                            //AI(W906-MT-E3c) 20260925: btnHome->Down only (not Light Scale's single home)
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
// AI(W906-MT-E1) 20260925: golden TMyEtherCatMotor::EtherCatMotHome (golden Motor/myEthercatmotor.cpp:1289-1370) +
//   SetHomeSpeed (:1703-1740) for a NON-DS402 drive on the 1203. golden case 1: if(!MotionDone()) DecStop, wait;
//   else DecStop, InitMotor, SetHomeSpeed (PAR_AxHomeVelLow/High = PHomeLow/HighSpeed, PAR_AxHomeAcc/Dec = dAcc/dDec,
//   PAR_AxHomeJerk = 0), PAR_AxHomeCrossDistance = 100/GearRatio, SetSoftLimit(999999,-999999),
//   Acm_AxMoveHome(ax, MODE12_AbsSearchReFind (=11), HomeDirection ? 0 : 1); case 10: READY; case 20: 0.3 s, then
//   SetCommand(0), SetPosition(0), SetSpeed(OldSpeed), SetSoftLimit(PSoftLimitP, PSoftLimitN) (TickHomes).
//   ⚠ GAPS (Pci1203Control has no entry for them; this branch cannot run on this machine -- all 19 axes are DS402):
//     InitMotor, PAR_AxHomeJerk, PAR_AxHomeCrossDistance, the soft-limit widening/restore, SetSpeed(OldSpeed).
//AI(W906-MT-E3c) 20260925: the start of a 1203 single home, shared by btnHome (DoHome) and Light Scale's case 0 -- was the
//  second half of DoHome plus HomeCardSide(), moved here unchanged (texts too) so both starts run the same checks and send
//  the same commands. Returns "" when the job was pushed (info says what went out), else the refusal reason.
//  Precondition: MotionPreludeFor passed, a 1203 axis, no job running on it.
struct HomeStartInfo {
    bool        cardSide;
    bool        issued;
    std::string wouldCall;
    int         homeMode, dir;
    std::string speedNote;
    HomeStartInfo() : cardSide(false), issued(false), homeMode(-1), dir(0) {}
};
void PushHomeJob(const HomeJob& h)
{
    bool placed = false;
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (!g_homes[i].active) { g_homes[i] = h; placed = true; break; }
    if (!placed) g_homes.push_back(h);
}
std::string StartHomeCardSide(const std::string& motorId, long long wireId, IMotorAccessBackend& be,
                              const MotionCtx& m, const std::string& who, bool fromLS, HomeStartInfo& info)
{
    if (!be.Pci1203AxisReady(m.a.axis)) {                                       // golden case 1: !MotionDone() -> DecStop; break (wait)
        Stop1203(be, m.a.axis, wireId);
        return who + ": 軸還在動（golden EtherCatMotHome case 1：先 DecStop，停好後再按一次 HOME）";
    }
    Stop1203(be, m.a.axis, wireId);                                             // golden: DecStop() before SetHomeSpeed
    const struct { int which; double v; } hs[4] = {
        { kHomeVelLow, (double)m.g.homeLow }, { kHomeVelHigh, (double)m.g.homeHigh }, { kHomeAcc, m.g.accDb }, { kHomeDec, m.g.decDb } };
    for (int i = 0; i < 4; ++i) {
        Pci1203Cmd c;
        c.kind = kCmdAxSetHome; c.wireId = wireId; c.axis = m.a.axis; c.home = hs[i].which; c.value = hs[i].v;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        if (CmdFailed(res)) return who + ": 1203 設定卡片歸零速度失敗（golden SetHomeSpeed）—— " + CmdWhy(res);
    }
    if (m.g.gearRatio == 0.0) return who + ": GearRatio=0（golden `if(GearRatio==0) break;`：不歸零）";
    Pci1203Cmd c;
    c.kind = kCmdAxMoveHome; c.wireId = wireId; c.axis = m.a.axis;
    c.homeMode = 11;                                                            // MODE12_AbsSearchReFind (AdvMotDrv.h)
    c.dir      = m.g.homeDirection ? 1 : -1;                                    // golden HomeDirection ? 0 (POS) : 1 (NEG)
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) return who + ": 1203 卡片式歸零失敗（golden WAR16122）—— " + CmdWhy(res);
    NoteIssued(be, m.a.axis);
    HomeJob h;
    h.active = true; h.motorId = motorId; h.mi = m.a.motIndex; h.axis = m.a.axis; h.g = m.g; h.cardSide = true; h.fromLS = fromLS;
    h.zTarget = be.GoldenZSafePos();
    PushHomeJob(h);
    info.cardSide = true; info.issued = res.issued; info.wouldCall = res.wouldCall; info.homeMode = c.homeMode; info.dir = c.dir;
    return std::string();
}
std::string StartHome1203(const std::string& motorId, long long wireId, IMotorAccessBackend& be, const MotionCtx& m,
                          const std::string& who, bool fromLS, HomeStartInfo& info)
{
    be.GoldenSetHomeFlag(m.a.motIndex, 0);                                      // golden :1158 一按就 HomeFlag=0（NB2 R23 §3：原本命令被接受後才清）；Light Scale :1840 同
    // NB2 R23 W4C-1：golden TMyMotor::Home（mymotor.cpp:1638-1642）／MotorHome（:1661-1665）第一行
    //   `if(Motor==NULL || Motor->CheckIsSafeDoorOpen()) return …;` —— 門開著不歸零（golden 停在 case 300 等門關；網頁上改成拒絕，再按一次）
    if (be.GoldenSafeDoorOpen(m.a.motIndex))
        return who + ": 安全門開著（golden TMyMotor::Home 的 CheckIsSafeDoorOpen）—— HomeFlag 已照 golden 清成 0，關門後再按 HOME";
    //AI(W906-DIR6B) 20260925: RULINGS_20260925 第 13 條（6B）「1203 軸不看 Direction，方向交給驅動器 Pn000」—— 不再以 Direction=1 拒絕
    //   （原本：arm Z 歸零後 golden 要去 ZSafePos 的絕對移動，Direction=1 時拒絕）
    //AI(W906-MERGE-56bbf785) 20260926: the laptop removed this refusal in DoHome; the machine had moved it here (MT-E3c) -- removed here.
    if (SampleStale(be, m.a.axis)) return who + ": " + kStaleWhy;              // NB2 R21 W4B-5
    // AI(W906-MT-E1) 20260925: HOME BRANCHES ON THE DRIVE (user EastSun 20260925: 「歸原點要有分支 預設值ethercat是用啥方式
    //   然後分支如果是1203才用現在的方式」 -> 「更正是看驅動器」):
    //     DS402 servo (profile 402 / SERVOPACK / Sigma-X)  -> the current Acm_AxHome(124|128) below -- the only way any
    //                                                        home ever succeeded on this machine; card-side modes are
    //                                                        refused here (0x8000510F);
    //     another drive                                    -> golden's EtherCAT default, TMyEtherCatMotor::EtherCatMotHome
    //                                                        (Acm_AxMoveHome MODE12 + zero after), StartHomeCardSide();
    //     unknown                                          -> refuse. Never guess: MODE12 on a DS402 drive fails, and
    //                                                        golden then zeroes the position anyway = a fake "homed".
    {
        const int driveKind = be.Pci1203DriveKind(m.a.axis);
        if (driveKind < 0)
            return who + ": 判斷不出這一軸的驅動器類型（監看器沒有這一站的 DS402／SERVOPACK 身分資料）—— 不猜：DS402 軸走卡片式 MODE12 會失敗，"
                         "golden 接著仍會把座標歸零，變成「沒動卻顯示已回原點」。請先重新掃描 1203。";
        if (driveKind == 0) return StartHomeCardSide(motorId, wireId, be, m, who, fromLS, info);
    }
    // NB2 R23 W4C-3：golden TMyMotor::Home SetADCRate(100)（:1647「Debug Find Home…剎車距離太長而撞機」）→ HomeObject →
    //   TMyEtherCatMotor::SetHomeSpeed（myEthercatmotor.cpp:1703）：HomeVelLow／High＝PHomeLowSpeed／PHomeHighSpeed、HomeAcc／Dec＝dAcc／dDec。
    //   EastSun 量過：DS402 的 Acm_AxHome 會拿卡上**當下的 PTP** VelHigh／VelLow／Acc 去填 6099h:1／6099h:2／609Ah（EtherCAT/Pci1203Gear.h:503-506），
    //   不讀 PAR_AxHomeVel* ⇒ 同義做法是在 home 之前把 PTP 速度設成歸零速度（否則剛 jog 100% 的軸會用 jog 高速找原點）。
    if (m.g.homeHigh == 0)
        return who + ": Home High Speed（PHomeHighSpeed）是 0 —— 不拿卡上殘留的速度去找原點（NB2 R23 W4C-3）";
    {
        const struct { Pci1203SpeedParam which; double v; } seq[4] = {
            { kSpeedInit, (double)m.g.homeLow }, { kSpeedRun, (double)m.g.homeHigh }, { kSpeedAcc, m.g.accDb }, { kSpeedDec, m.g.decDb } };
        for (int i = 0; i < 4; ++i) {
            Pci1203Cmd c;
            c.kind = kCmdAxSetSpeed; c.wireId = wireId; c.axis = m.a.axis; c.speed = seq[i].which; c.value = seq[i].v;
            const Pci1203CmdResult res = be.Pci1203Execute(c);
            if (CmdFailed(res)) return who + ": 1203 設定歸零速度失敗 —— " + CmdWhy(res);
        }
    }
    Pci1203Cmd c;
    c.kind = kCmdAxHome; c.wireId = wireId; c.axis = m.a.axis;
    c.homeMode = m.g.homeDirection ? 124 : 128;
    c.dir      = m.g.homeDirection ? 1 : -1;
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) return who + ": 1203 歸零失敗 —— " + CmdWhy(res);
    NoteIssued(be, m.a.axis);
    HomeJob h;
    h.active = true; h.motorId = motorId; h.mi = m.a.motIndex; h.axis = m.a.axis; h.g = m.g; h.fromLS = fromLS;
    h.zTarget = be.GoldenZSafePos();
    PushHomeJob(h);
    char sp[160];
    std::snprintf(sp, sizeof(sp), "home speed velLow=%u velHigh=%u acc=%.1f dec=%.1f", m.g.homeLow, m.g.homeHigh, m.g.accDb, m.g.decDb);
    info.issued = res.issued; info.wouldCall = res.wouldCall; info.homeMode = c.homeMode; info.dir = c.dir; info.speedNote = sp;
    return std::string();
}

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
        else Stop1203(be, m.a.axis, wireId);                                   // AI(W906-MT-E1): golden PCIL132_StopMotor = DecStop
        EndSingleHome(be, "HOME released (golden btnHomeClick :1170 bSingleHome=false)");   //AI(W906-MT-E3c) 20260925: the shared flag -- also ends Light Scale's single home
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
    //AI(W906-MT-E3c) 20260925: the start itself moved to StartHome1203 (shared with Light Scale's case 0), unchanged.
    //AI(W906-MERGE-56bbf785) 20260926: laptop's change here was only the DIR6B removal of the armZ && Direction refusal; it is
    //  applied inside StartHome1203 (the machine's structure), the rest of the laptop's copy of the start is the same code.
    HomeStartInfo info;
    const std::string refused = StartHome1203(r.motors[0], wireId, be, m, who, false, info);
    if (!refused.empty()) return Refuse(refused);
    webbridge::JsonWriter w;
    w.BeginObject();
    if (info.cardSide) {
        AckHead(w, r, "homing", "pci1203",
                std::string(info.issued ? "sent: " : "accepted, NOT issued (dry): ") + info.wouldCall +
                "; card-side home (drive is not DS402): after READY + 0.3 s SetCommand(0)+SetPosition(0), then HomeFlag=1");
        w.Key("homeActive").Bool(true);
        w.Key("axis").Number((wb_int64)m.a.axis);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    AckHead(w, r, "homing", "pci1203",
            std::string(info.issued ? "sent: " : "accepted, NOT issued (dry): ") + info.wouldCall +
            "; " + info.speedNote + "; HomeFlag=0，完成後由主迴圈設 1（失敗設 2）");
    w.Key("axis").Number((wb_int64)m.a.axis);
    w.Key("homeMode").Number((wb_int64)info.homeMode);
    w.Key("dir").Number((wb_int64)info.dir);
    w.Key("homeActive").Bool(true);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

//AI(W906-MT-E3c) 20260925: golden TMyMotor::MotorMove(p) (golden Motor/mymotor.cpp:871-962, with MotorMovePosition
//  :549-860) for a 1203 axis, called once per golden tick by the All-mode loop and Light Scale -- the two callers whose
//  golden semantics hang on MotorMove's RETURN VALUE (All: `AllMotMoveFlag=MOT[i].MotorMove(p)`, Light Scale:
//  `if(MOT[iUseAxis].MotorMove(iPosition))`), including its quirks, which both callers inherit (R7 / lead item 7):
//    * -1 (safe door), -2 / -3 (soft limit) are TRUTHY: "arrived" -- nothing moves, the caller goes on;
//    * after a command (fCMD), MotionDone() alone is "arrived" (:718-815): an axis stopped short -- STOP, an alarm's
//      StopAllMotor, VerifyMotorAction's 2 s StopAllMotor -- counts as arrived at the target it never reached;
//    * a new target clears fCMD (:883-889); locked -> PCIL132_StopMotor, 1 only when already there (:895-931);
//    * Enable=0 (Mot_Table) -> 1 at once, no command (:821-823, non-SIM);
//    * not fCMD: MotionDone()==false -> 0 (wait); at the target within CompareCommandPos's iGap -> 1; else MoveToPos, fCMD.
//  Card half (EastSun's layer): MotionDone() = the monitor sample's STA_AX_READY, ReadPos() = its cmdPos in user units,
//  MoveToPos = kCmdAxMoveAbs. Port-only waits (return 0, the caller simply asks again next tick): a sample older than this
//  file's last command on the axis (W4B-5), no 1203 control / unresolved axis / Direction=1 (kDirPending) -- none of them
//  can report "arrived". [AI(W906-MERGE-56bbf785) 20260926: Direction=1 is no longer a wait -- ruling 6B, see MotionPreludeFor.] NOT modelled (golden side effects of MotorMovePosition that never change the return's truthiness
//  here): the -2/-3 ShowMyMessage popups (kept in `note`), PServoAlarmOn's InPos-lamp wait (a 1203 axis never lights it,
//  Led[8] is commented out in golden ScanMotorStatus), the every-100th-arrival WAR1639 encoder check and the Position<=-500
//  home-sensor check (-5 / -6, both truthy too).
int GoldenMove1203(IMotorAccessBackend& be, const MotorAccessAxis& a, const MotorGolden& g, int target, long long wireId,
                   std::string& note)
{
    GoldenMoveState& s = g_gm[a.motIndex];
    if (be.GoldenSafeDoorOpen(a.motIndex)) { note = "MotorMove -1 (safe door open)"; return -1; }        // :876-880
    if (target != s.oldPos) { s.fCMD = false; s.oldPos = target; }                                      // :883-889
    std::string why;
    const bool canCmd = be.Pci1203Ready(why) && a.axis >= 0;
    if (be.GoldenMoveLocked(a.motIndex)) {                                                               // :895-931
        if (canCmd) Stop1203(be, a.axis, wireId);
        double card = 0.0;
        if (canCmd && !SampleStale(be, a.axis) && be.Pci1203CmdPos(a.axis, card) &&   //AI(W906-MERGE-56bbf785) 20260926: `!g.direction &&` dropped (ruling 6B)
            MotorCardToUser(card, g.gearRatio) == target) { s.fCMD = false; return 1; }
        note = "MotorMove 0 (motor locked: fCanMove* / mapLockList -> PCIL132_StopMotor)";
        return 0;
    }
    if (!a.tableEnable) { s.fCMD = false; return 1; }                                                   // :821-823 (+ :953-956)
    char b[160];
    if (target >= g.softP) { std::snprintf(b, sizeof(b), "MotorMove -2: target %d over positive soft limit %d (golden popup)", target, g.softP); note = b; return -2; }
    if (target <= g.softN) { std::snprintf(b, sizeof(b), "MotorMove -3: target %d below negative soft limit %d (golden popup)", target, g.softN); note = b; return -3; }
    if (!canCmd) { note = "MotorMove 0: 1203 control / axis not available -- " + (a.axis < 0 ? a.why : why); return 0; }
    //AI(W906-MERGE-56bbf785) 20260926: `if (g.direction) { note = kDirPending; return 0; }` removed -- ruling 6B (see MotionPreludeFor).
    if (SampleStale(be, a.axis)) return 0;
    double card = 0.0;
    if (!be.Pci1203CmdPos(a.axis, card)) { note = "MotorMove 0: position unreadable"; return 0; }
    const int pos = MotorCardToUser(card, g.gearRatio);                                                 // :586 Position=ReadPos()
    //AI(W906-MERGE-56bbf785) 20260926: the laptop's W5B-9 (golden :590-593 `if(Position!=Tar) iLastRotatorDirP=(Tar>Position);`, every
    //  MotorMove) added to this machine-only MotorMove too (All-mode loop / Light Scale), with this function's own ReadPos.
    if (pos != target) be.GoldenSetRotatorLastDirP(a.motIndex, target > pos);
    const bool done = be.Pci1203AxisReady(a.axis);                                                      // Motor->MotionDone()
    if (!s.fCMD && !done) return 0;                                                                     // :593-596
    if (!s.fCMD) {
        const int gap = (g.armZ && g.gearRatio > 2 && g.gearRatio <= 5) ? 5 : 2;                        // :603-608 (USE_CompareCommandPos, MachineType.h)
        if (target - gap <= pos && pos <= target + gap) { s.fCMD = false; return 1; }                   // CompareCommandPos(Tar, iGap)==1 -> InitMOTParameter; 1
        if (g.inShuttle && !be.GoldenShuttleFloodgateReady(g.inShuttle)) { note = "MotorMove -1 (floodgate not open, golden :652-698)"; return -1; }
        Pci1203Cmd c; c.kind = kCmdAxMoveAbs; c.wireId = wireId; c.axis = a.axis; c.value = (double)MotorUserToCard(target, g.gearRatio);
        const Pci1203CmdResult res = be.Pci1203Execute(c);                                              // Motor->MoveToPos(Pos)
        if (CmdFailed(res)) note = "MoveToPos failed (golden WAR16122 popup, fCMD set anyway): " + CmdWhy(res);
        NoteIssued(be, a.axis);
        s.fCMD = true;                                                                                  // :714
        return 0;
    }
    if (done) { s.fCMD = false; return 1; }                                                             // :718-815 MotionDone() -> Position=Tar; 1
    return 0;
}

// ---- loopMove, select=All：golden btnLoopMoveClick :1300-1336 ＋ DoLoopMove :468-501 / :577-617 ----
//AI(W906-MT-E3c) 20260925: golden semantics, each kept:
//  * only the SELECTED motor (golden ActiveIndex = C++ g_selMotor) is asked about HomeFlag (:1316) -- the checked motors
//    are not; ActiveIndex==-1 -> refused like golden's `btnLoopMove->Down=false; return;` (:1303-1307);
//  * every tick every checked motor gets MotorMove(its edPos1 / edPos2) (GoldenMove1203 above, or the golden object for a
//    non-1203 motor), and the leg ends only when ALL of them returned non-zero -- a refused move (-1/-2/-3) counts as
//    arrived (golden `AllMotMoveFlag=MOT[i].MotorMove(p)` converts int to bool);
//  * dwLoopCount++ and Average+=LatchCycleTime() on the pos2 leg only; All mode never latches the leg timer
//    (`flag`/LatchCycleTime(true) are in the single-axis branch), so Average adds the ms since the LAST single-axis latch
//    (0 if there never was one -- golden's zero-initialised timer), lblJogPTime/lblJogNTime are not touched;
//  * the same Wait / Task 10 / 60 as single mode; no leg timeout (golden has none).
//  Lead default 20260925: the motors start at their last golden speed pct (g_ptpPct, 1 if never set) -- 1203 axes only
//  (the golden object of any other card keeps its own last SetSpeed). The page sends motors = the checked visible rows
//  and pos1.<Alias> / pos2.<Alias> per motor (the parser keeps any numeric key).
MotorAccessOutcome DoLoopMoveAll(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (g_selMotor.empty())
        return Refuse(r.action + "（All）: 沒有選馬達 —— golden btnLoopMoveClick :1303-1307 `if(ActiveIndex==-1){ btnLoopMove->Down=false; return; }`");
    MotionCtx sel;
    if (!be.Resolve(g_selMotor, sel.a) || sel.a.motIndex < 0 || !be.GoldenMotor(sel.a.motIndex, sel.g))
        return Refuse(r.action + "（All）: 選取的馬達 " + g_selMotor + " 沒有 golden 馬達物件");
    double wt = 0;
    ParamNum(r, "waitTime", wt);
    std::map<std::string, bool>::const_iterator cf = r.flag.find("confirmNotHomed");
    if (sel.g.homeFlag == 0 && (cf == r.flag.end() || !cf->second))           // golden :1316-1328 -- the selected motor only
        return Refuse(r.action + "（All）" + g_selMotor + ": Motor not home yet, sure to loop test? (馬達尚未歸零，確定要執行？) —— 頁面確認後帶 confirmNotHomed 再送");
    std::vector<LoopAxis> axes;
    for (std::size_t k = 0; k < r.motors.size(); ++k) {
        LoopAxis ax;
        ax.motorId = r.motors[k];
        const std::string who = r.action + "（All）" + ax.motorId;
        if (!be.Resolve(ax.motorId, ax.a)) return Refuse(who + ": 馬達表上沒有這個 Alias");
        if (ax.a.motIndex < 0 || !be.GoldenMotor(ax.a.motIndex, ax.g)) return Refuse(who + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL）");
        if (ax.g.galilIndex && !ax.a.Is1203())
            return Refuse(who + ": INDEX_MOTION_CARD==0 的 Index 軸走 golden 的 Galil 分支（golden ck00Click :1673-1685 本來就讓 Index 四軸勾不起來）");
        if (ax.a.Is1203() && ax.a.tableEnable) {                                // Enable=0: golden MotorMove answers 1 without hardware (kept)
            std::string why;
            if (!be.Pci1203Ready(why)) return Refuse(who + "（PCI1203）: 1203 控制層不可用 —— " + why);
            if (ax.a.axis < 0) { be.Pci1203NoteRefusal(wireId, "motor.access loopMove", who + ": " + ax.a.why); return Refuse(who + "（PCI1203）: " + ax.a.why); }
            //AI(W906-MERGE-56bbf785) 20260926: the Direction=1 refusal (kDirPending) removed -- ruling 6B (see MotionPreludeFor).
        }
        double p1 = 0, p2 = 0;
        if (!ParamNum(r, ("pos1." + ax.motorId).c_str(), p1) || !ParamNum(r, ("pos2." + ax.motorId).c_str(), p2))
            return Refuse(who + ": 缺 pos1." + ax.motorId + "／pos2." + ax.motorId + "（golden MotorTestClass[i]->edPos1／edPos2）");
        ax.pos1 = (int)p1; ax.pos2 = (int)p2;
        axes.push_back(ax);
    }
    for (std::size_t i = 0; i < g_homes.size(); ++i)                            // golden :1309 btnHome->Down=false (same as single mode)
        if (g_homes[i].active && !g_homes[i].fromLS) CancelHome(be, g_homes[i], true, "loop pressed (golden btnHome->Down=false)");
    std::string speedNote;
    for (std::size_t k = 0; k < axes.size(); ++k) {                             // lead default: the last golden speed pct
        LoopAxis& ax = axes[k];
        if (!ax.a.Is1203() || !ax.a.tableEnable) continue;
        MotionCtx m; m.a = ax.a; m.g = ax.g;
        std::string why, note;
        if (!Send1203Speed(be, wireId, m, PtpPctOf(ax.a.motIndex), why, note)) return Refuse(r.action + "（All）" + ax.motorId + ": " + why);
        speedNote += (speedNote.empty() ? "" : "; ") + ax.motorId + " " + std::to_string(PtpPctOf(ax.a.motIndex)) + "%";
    }
    be.GoldenInitMOTParameterAll();                                             // golden :1314-1315
    for (std::map<int, GoldenMoveState>::iterator gi = g_gm.begin(); gi != g_gm.end(); ++gi) gi->second.fCMD = false;
    g_loop = LoopJob();                                                         // golden dwLoopCount=0 (:1326 / :1332)
    g_loop.active = true; g_loop.all = true; g_loop.motorId = g_selMotor; g_loop.a = sel.a; g_loop.g = sel.g;
    g_loop.axes = axes;
    const int w01 = (int)wt;
    g_loop.waitMs = (w01 > 100 || w01 <= 0) ? 0 : w01 * 100;                  // golden Set0_1SecAndOn(Wait)
    g_loop.task = 1;
    if (sel.g.homeFlag != 0) { g_loopAverage = 0; g_avgTime = 0.0; g_hasAvg = true; }   // golden :1333-1334 (homed branch only)
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "looping", "MOT+pci1203",
            "LoopMove（All）開始：" + std::to_string(axes.size()) + " 軸同時去 pos1，全部到位才換段（golden DoLoopMove :468-617）" +
            (speedNote.empty() ? std::string() : "; speed " + speedNote));
    w.Key("all").Bool(true);
    w.Key("selectedMotor").String(g_selMotor);
    w.Key("motors").BeginArray();
    for (std::size_t k = 0; k < axes.size(); ++k) {
        w.BeginObject();
        w.Key("motorId").String(axes[k].motorId);
        w.Key("pos1").Number((wb_int64)axes[k].pos1);
        w.Key("pos2").Number((wb_int64)axes[k].pos2);
        w.EndObject();
    }
    w.EndArray();
    w.Key("waitMs").Number((wb_int64)g_loop.waitMs);
    w.Key("loopActive").Bool(true);
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
      //AI(W906-MT-E3c) 20260925: All mode is wired now (was refused, NB2 R23 §3) -- DoLoopMoveAll.
      if (md != r.num.end() && md->second != 0.0) return DoLoopMoveAll(r, wireId, be); }
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
        if (g_homes[i].active && !g_homes[i].fromLS) CancelHome(be, g_homes[i], true, "loop pressed (golden btnHome->Down=false)");   //AI(W906-MT-E3c) 20260925: not Light Scale's single home (bSingleHome, no button)
    if (m.a.Is1203() && hasPct) {
        std::string why, note;
        if (!Send1203Speed(be, wireId, m, (int)pct, why, note)) return Refuse(who + ": " + why);
        g_ptpPct[m.a.motIndex] = (int)pct;                                      //AI(W906-MT-E3c) 20260925
    }
    be.GoldenInitMOTParameterAll();                                             //AI(W906-MT-E3c) 20260925: golden :1314-1315 `for(i<TOTAL_MOTOR) MOT[i].InitMOTParameter();`
    for (std::map<int, GoldenMoveState>::iterator gi = g_gm.begin(); gi != g_gm.end(); ++gi) gi->second.fCMD = false;   //   (the 1203 half of the same fCMD)
    g_loop = LoopJob();                                                         // golden dwLoopCount=0 (:1326 / :1332)
    g_loop.active = true; g_loop.motorId = r.motors[0]; g_loop.a = m.a; g_loop.g = m.g;
    g_loop.pos1 = (int)p1; g_loop.pos2 = (int)p2;
    const int w01 = (int)wt;
    g_loop.waitMs = (w01 > 100 || w01 <= 0) ? 0 : w01 * 100;                  // golden Set0_1SecAndOn(Wait)；>100 或 <=0 不等
    g_loop.task = 1;
    if (m.g.homeFlag != 0) { g_loopAverage = 0; g_avgTime = 0.0; g_hasAvg = true; }   //AI(W906-MT-E2) 20260925: golden :1333-1334 Average=0; lblAvgTime->Caption=0 -- the homed branch only (the confirmed not-homed branch keeps both)
    if (!m.a.Is1203()) {                                                        // golden 第一拍：MOT.MotorMove(pos1)（速度＝edtSpeed）
        if (!g_latchArmed) { g_latchArmed = true; g_latchMs = LoopNow(be); }   //AI(W906-MT-E2) 20260925: that first tick latches the leg timer too (golden :443-447)
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

//AI(W906-MT-E3c) 20260925: golden TMyEtherCatMotor::InitMotor (golden Motor/myEthercatmotor.cpp:166-396) for a 1203 axis,
//  EastSun ruling R4 20260925 "Test Range = FULL golden InitMotor ... through EastSun's TPci1203Control on the monitor's
//  handles, never a second axis open". Every golden call becomes one Pci1203Execute on the monitor's axis slot, in golden
//  order; golden's error handling is kept per step (a failing write pops WAR16122 and golden carries on -- here the step is
//  listed as failed in the ack and the sequence carries on the same way).
//    :172-173  `if(!Enable) return true;`                        -> Mot_Table Enable=0: nothing written
//    :175-178  Open_Axis(); `if(bAxisOpen==false) return false;`  -> no 1203 control / no monitor axis: nothing written
//    :180-185  the four EMG sensors IsOff -> ShowMyMessage("Please Unlock EMG And Restart The Software!!") + return false
//    :187-217  ResetMotorError: ResetError; GetState; not READY -> ResetError, its failure -> WAR16123 + `goto ResetMotorError`
//              (GetState failing -> WAR16121 + goto). ⚠ DEVIATION: golden's goto has no bound (it hangs the UI thread while
//              ResetError keeps failing); bounded here to kInitResetRounds rounds, then InitMotor stops (nothing more written)
//              and the ack says so. The state read is the monitor's last sample (taken before this click; the card is not
//              re-read in between), so an axis that was not READY gets golden's second ResetError.
//    :219-361  the configuration table, via kCmdAxSetInitCfg (Pci1203GoldenInitCfgPlan: PPU, ElReact, AlmEnable, AlmReact,
//              OrgLogic, Jerk, SetEtherCatInType's arm, pulse modes -- EastSun's MT-E3a). Success = issued && ret==0 (a U32
//              Dsp_PropertyIDNotSupport comes back as ret 0 with a note, golden's excuse); dry = accepted, not issued.
//    :363-382  CFG_AxMaxVel = PJogHighSpeed, CFG_AxMaxAcc = dAcc, CFG_AxMaxDec = dDec (kSpeedMaxVel/MaxAcc/MaxDec)
//    :384-390  ResetError
//    :392      SetServoOn(true) = MotOutputOn (:1392-1412): `if(bAlarm)` ResetError + MySleep(100); then SvOn 1. golden's bAlarm is
//              the file-global lamp of the LAST ScanMotorStatus -- on Motor Test that is the selected motor (Timer1 scans it
//              every 5 ms), i.e. this axis: its ALM bit or ERROR_STOP in the monitor sample.
//    :393-394  SetCommand(0), SetPosition(0)
const int kInitResetRounds = 3;
struct InitStep {
    std::string name, call, note;
    bool        accepted, issued, ok;
    unsigned long ret;
    InitStep() : accepted(false), issued(false), ok(false), ret(0) {}
};
InitStep InitExec(IMotorAccessBackend& be, const Pci1203Cmd& c, const std::string& name)
{
    InitStep s;
    s.name = name;
    const Pci1203CmdResult r = be.Pci1203Execute(c);
    s.call = r.wouldCall; s.accepted = r.accepted; s.issued = r.issued; s.ret = r.ret;
    s.ok = !CmdFailed(r);
    if (!s.ok) s.note = "golden WAR16122 popup, InitMotor carries on -- " + CmdWhy(r);
    else if (!r.issued) s.note = "dry: accepted, NOT issued";
    else if (!r.why.empty()) s.note = r.why;                                    // e.g. the Dsp_PropertyIDNotSupport excuse
    return s;
}
InitStep InitNote(const std::string& name, bool ok, const std::string& note)
{
    InitStep s; s.name = name; s.ok = ok; s.note = note; return s;
}
// Returns true when golden's InitMotor ran to its end (every step reached; some may have failed -- see steps).
bool InitMotor1203(IMotorAccessBackend& be, long long wireId, const MotionCtx& m, std::vector<InitStep>& steps, std::string& stopWhy)
{
    if (!m.a.tableEnable) { stopWhy = "Enable=0 (Mot_Table): golden InitMotor :172 `if(!Enable) return true;` -- nothing written"; return false; }
    std::string why;
    if (!be.Pci1203Ready(why)) { stopWhy = "golden InitMotor :177 `if(bAxisOpen==false) return false;` -- 1203 control not available: " + why; return false; }
    if (m.a.axis < 0) { stopWhy = "golden InitMotor :177 `if(bAxisOpen==false) return false;` -- " + m.a.why; return false; }
    if (be.GoldenInitMotorEmgOff()) {                                           // :180-185
        stopWhy = "Please Unlock EMG And Restart The Software!! 請解開EMG並重新啟動軟體!! (golden InitMotor :183 ShowMyMessage, return false)";
        return false;
    }
    Pci1203Cmd c;
    c.wireId = wireId; c.axis = m.a.axis;
    bool reset = false;
    for (int round = 1; round <= kInitResetRounds && !reset; ++round) {        // :187 ResetMotorError:
        c.kind = kCmdAxResetError;
        steps.push_back(InitExec(be, c, "ResetError (:189)"));
        unsigned st = 0;
        if (!be.Pci1203AxisState(m.a.axis, st)) {                               // :197 / :212-217
            steps.push_back(InitNote("GetState (:197)", false, "unreadable -> golden WAR16121 + goto ResetMotorError"));
            continue;
        }
        if (IsReadyState(st)) { reset = true; break; }                          // :200
        c.kind = kCmdAxResetError;
        InitStep s2 = InitExec(be, c, "ResetError, state not READY (:203)");
        const bool ok2 = s2.ok;
        if (!ok2) s2.note = "golden WAR16123 + goto ResetMotorError -- " + s2.note;
        steps.push_back(s2);
        if (ok2) reset = true;                                                  // golden goes on (even if still not READY)
    }
    if (!reset) {
        char b[200];
        std::snprintf(b, sizeof(b), "golden `goto ResetMotorError` would retry for ever; bounded to %d rounds here -- InitMotor stopped, nothing more written", kInitResetRounds);
        stopWhy = b;
        return false;
    }
    const std::vector<IMotorAccessBackend::InitCfg> plan = be.GoldenInitCfgPlan(m.g);   // EastSun's Pci1203GoldenInitCfgPlan
    for (std::size_t i = 0; i < plan.size(); ++i) {                             // :219-361
        Pci1203Cmd k;
        k.kind = kCmdAxSetInitCfg; k.wireId = wireId; k.axis = m.a.axis; k.initCfg = plan[i].which; k.value = plan[i].value;
        char nm[96];
        std::snprintf(nm, sizeof(nm), "%s=%g", plan[i].name.c_str(), plan[i].value);
        steps.push_back(InitExec(be, k, nm));
    }
    const struct { Pci1203SpeedParam which; double v; const char* nm; } mx[3] = {   // :363-382
        { kSpeedMaxVel, (double)m.g.jogHigh, "CFG_AxMaxVel=PJogHighSpeed" }, { kSpeedMaxAcc, m.g.acc, "CFG_AxMaxAcc=dAcc" },
        { kSpeedMaxDec, m.g.dec, "CFG_AxMaxDec=dDec" } };
    for (int i = 0; i < 3; ++i) {
        Pci1203Cmd k;
        k.kind = kCmdAxSetSpeed; k.wireId = wireId; k.axis = m.a.axis; k.speed = mx[i].which; k.value = mx[i].v;
        char nm[96];
        std::snprintf(nm, sizeof(nm), "%s (%g)", mx[i].nm, mx[i].v);
        steps.push_back(InitExec(be, k, nm));
    }
    c.kind = kCmdAxResetError;
    steps.push_back(InitExec(be, c, "ResetError (:385)"));
    {                                                                           // :392 SetServoOn(true) -> MotOutputOn(1)
        unsigned long io = 0; unsigned st = 0;
        const bool alarm = (be.Pci1203MotionIO(m.a.axis, io) && (io & 0x00000002ul) != 0) ||
                           (be.Pci1203AxisState(m.a.axis, st) && IsErrorState(st));
        if (alarm) {                                                            // :1398-1405 bAlarm
            c.kind = kCmdAxResetError;
            InitStep s = InitExec(be, c, "ResetError before SvOn (bAlarm, :1400)");
            if (!s.ok) s.note = "golden ignores this result (:1402-1404) -- " + s.note;
            s.ok = true;
            steps.push_back(s);
            be.SleepMs(100);                                                    // :1401 MySleep(100)
        }
        Pci1203Cmd k;
        k.kind = kCmdAxSvOn; k.wireId = wireId; k.axis = m.a.axis; k.value = 1.0;
        steps.push_back(InitExec(be, k, "SvOn 1 (:1406)"));
    }
    Pci1203Cmd z;
    z.kind = kCmdAxSetCmdPos; z.wireId = wireId; z.axis = m.a.axis; z.value = 0.0;
    steps.push_back(InitExec(be, z, "SetCommand(0) (:393)"));
    z.kind = kCmdAxSetActPos;
    steps.push_back(InitExec(be, z, "SetPosition(0) (:394)"));
    NoteIssued(be, m.a.axis);
    return true;
}

// ---- setRangeAndInit／setRateAndInit：btnSetRangeClick :1374／btnSetRateClick :1364 ----
//   golden：SetRange/SetRate → Motor->InitMotor(Address) → HomeFlag=0。1203 軸的 InitMotor 走 golden 開軸
//   （Acm_AxOpenbyID，m_Axishand[999]），與 EastSun 監看器的開軸規則衝突（W0 第 2 項待整合）⇒ 1203 軸拒絕。
//   [AI(W906-MT-E3c) 20260925: superseded for 1203 axes by EastSun R4 -- the FULL golden InitMotor on the monitor's handles
//   (InitMotor1203 above), after golden's SetRange / SetRate:
//     SetRange (myEthercatmotor.cpp:567-572): Range=min(a,1000) -- memory only.
//     SetRate  (:576-616): dAcc=(PJogHighSpeed-InitSpeed)*a/8e6 (memory); dAcc==0 -> return; else the local Rate
//              (MotorRateFromGolden, unsigned 32-bit like golden) goes to PAR_AxAcc and PAR_AxDec (kSpeedAcc / kSpeedDec).
//              golden has no bAxisOpen guard there -- with no axis it would call the SDK on an unopened handle and pop
//              WAR16122; here that half is skipped with the same note. Test Rate stays hidden on the page like golden
//              (dfm Visible=False, R4), its backend is this.
//   then HomeFlag=0 (:1371 / :1381). Nothing here refuses what golden would do: the ack lists every step and its result.]
MotorAccessOutcome DoRangeRate(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    CancelLoop(be, false, "range/rate set (golden :1366／:1376 第一行 btnLoopMove->Down=false)");   // NB2 R23：golden 在任何檢查之前 (AI(W906-MT-E3c): now before the ActiveIndex test too, golden order)
    if (r.motors.empty()) return Refuse(r.action + ": 沒有選馬達（golden: ActiveIndex==-1 → return）");
    MotionCtx m;
    if (!be.Resolve(r.motors[0], m.a)) return Refuse(r.action + ": 馬達表上沒有 " + r.motors[0]);
    if (m.a.motIndex < 0 || !be.GoldenMotor(m.a.motIndex, m.g))
        return Refuse(r.action + " " + r.motors[0] + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL）");
    const std::string who = r.action + " " + r.motors[0];
    if (!m.g.selectable) return Refuse(NotSelectable(who));
    double v = 0;
    if (!ParamNum(r, "value", v)) return Refuse(who + ": 缺 value（golden edtMotorRange／edtMotorRate）");
    if (m.a.Is1203()) {                                                         //AI(W906-MT-E3c) 20260925: R4 (was refused)
        if (!(v > -2147483649.0 && v < 2147483648.0)) return Refuse(who + ": value 超出 int 範圍 —— golden atoi 的結果不可預期");
        const bool isRange = (r.action == "setRangeAndInit");
        const unsigned uv = (unsigned)(int)v;                                   // golden SetRange/SetRate(unsigned int a) of atoi(...)
        std::vector<InitStep> steps;
        const int mi = m.a.motIndex;
        if (isRange) {
            be.GoldenSetRangeMemory(mi, uv);                                    // golden :1379 SetRange
            steps.push_back(InitNote("SetRange(" + std::to_string(uv) + ") (:567-572)", true, "memory: Range=min(a,1000)"));
        } else {
            const MotorGoldenRate gr = MotorRateFromGolden(uv, m.g.jogHigh, m.g.initSpeed, be.GoldenReadSpeed(mi), m.g.range);
            be.GoldenSetAccMemory(mi, gr.dAcc);                                 // golden :583 dAcc=...
            char b[200];
            std::snprintf(b, sizeof(b), "memory: dAcc=(PJogHighSpeed %u - InitSpeed %u)*%u/8e6 = %.9g", m.g.jogHigh, m.g.initSpeed, uv, gr.dAcc);
            steps.push_back(InitNote("SetRate(" + std::to_string(uv) + ") (:576-583)", true, b));
            if (gr.skip) {
                steps.push_back(InitNote("SetRate (:584-585)", true, "dAcc==0 -> golden returns: PAR_AxAcc / PAR_AxDec not written"));
            } else {
                std::string why;
                if (!be.Pci1203Ready(why) || m.a.axis < 0) {
                    steps.push_back(InitNote("PAR_AxAcc / PAR_AxDec (:604-615)", false,
                                             "no monitor axis -- golden has no bAxisOpen guard here and would pop WAR16122 on an unopened handle; not sent"));
                } else {
                    for (int k = 0; k < 2; ++k) {
                        Pci1203Cmd c;
                        c.kind = kCmdAxSetSpeed; c.wireId = wireId; c.axis = m.a.axis; c.speed = k ? kSpeedDec : kSpeedAcc; c.value = gr.rate;
                        std::snprintf(b, sizeof(b), "%s=Rate %.9g (:%d, clamp %u..%u)", k ? "PAR_AxDec" : "PAR_AxAcc", gr.rate, k ? 610 : 604, gr.accMin, gr.accMax);
                        steps.push_back(InitExec(be, c, b));
                    }
                }
            }
        }
        MotionCtx m2 = m;                                                       // InitMotor reads dAcc AFTER SetRate changed it (CFG_AxMaxAcc=dAcc)
        be.GoldenMotor(mi, m2.g);
        std::string stopWhy;
        const bool ran = InitMotor1203(be, wireId, m2, steps, stopWhy);         // golden :1370 / :1380
        be.GoldenSetHomeFlag(mi, 0);                                            // golden :1371 / :1381
        int failed = 0;
        for (std::size_t i = 0; i < steps.size(); ++i) if (!steps[i].ok) ++failed;
        webbridge::JsonWriter w;
        w.BeginObject();
        std::string msg = std::string(isRange ? "SetRange" : "SetRate") + "(" + std::to_string(uv) + ") + InitMotor + HomeFlag=0: ";
        msg += ran ? (failed ? std::to_string(failed) + " step(s) failed (golden WAR popups), sequence carried on like golden" : "every step done")
                   : "InitMotor did not run -- " + stopWhy;
        AckHead(w, r, !ran ? "initMotorNotRun" : (failed ? "partial" : "initialized"), "pci1203", msg);
        w.Key("motIndex").Number((wb_int64)mi);
        if (m.a.axis >= 0) w.Key("axis").Number((wb_int64)m.a.axis);
        w.Key("value").Number((wb_int64)uv);
        w.Key("initMotor").BeginObject();
        w.Key("ran").Bool(ran);
        if (!ran) w.Key("why").String(stopWhy);
        w.Key("failed").Number((wb_int64)failed);
        w.Key("steps").BeginArray();
        for (std::size_t i = 0; i < steps.size(); ++i) {
            w.BeginObject();
            w.Key("step").String(steps[i].name);
            w.Key("ok").Bool(steps[i].ok);
            if (!steps[i].call.empty()) { w.Key("call").String(steps[i].call); w.Key("issued").Bool(steps[i].issued); w.Key("ret").Number((wb_int64)steps[i].ret); }
            if (!steps[i].note.empty()) w.Key("note").String(steps[i].note);
            w.EndObject();
        }
        w.EndArray();
        w.EndObject();
        w.Key("partial").Bool(!ran || failed > 0);
        WriteCurOf(w, be, mi);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    be.GoldenSetRangeRate(m.a.motIndex, r.action == "setRangeAndInit", (int)v);
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "set", "MOT", std::string("MOT[") + std::to_string(m.a.motIndex) + "] " +
            (r.action == "setRangeAndInit" ? "SetRange" : "SetRate") + "(" + std::to_string((int)v) + ")＋InitMotor＋HomeFlag=0");
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// AI(W906-MT-E1) 20260925: golden btnReloadMotorDataClick (uMotorTest.cpp:1695-1711): InitialMotorParameter(); then
//   for every motor MOT[i].PCIL132_SetPos(0) = Motor->SetCommand(0) + Motor->SetPosition(0) (golden mymotor.cpp
//   PCIL132_SetPos; an Enable=0 motor only clears its memory copy). golden has NO guard on this button (no
//   SystemStart / motion check) -- user ruling: golden guards exactly, so none here either.
//   Done for every axis the 1203 monitor opened (every PCI1203 row of Mot_Table on this machine).
//   ⚠ GAP: InitialMotorParameter() is NOT re-run -- it re-creates the MOT[] objects that the Motor Test's golden
//   interlocks and the runtime overlay read, and it opens axes the golden way, which conflicts with the monitor's
//   single axis owner (W0 item 2). The Mot_Table values themselves are re-read by the next wb_serve start.
//   [AI(W906-MT-E2) 20260925: gap narrowed -- the TABLE VALUES of InitialMotorParameter() are now put back
//   (GoldenReloadMotorParams: Mot_Table.csv re-read, speeds / soft limits / acc-dec / range / gear / direction ... applied
//   to the existing objects in place). Still not done: re-creating objects, InitMotor (opening axes), a changed row
//   identity (that row is skipped, the ack says "restart wb_serve").]
MotorAccessOutcome DoReloadMotorData(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    std::string why;
    if (!be.Pci1203Ready(why)) return Refuse(r.action + ": 1203 控制層不可用 —— " + why);
    // AI(W906-MT-E1) 20260925: golden order -- InitialMotorParameter() (HomeFlag=0 for every motor) BEFORE SetPos(0).
    //   Cleared first so a partial zeroing failure still leaves nothing marked homed. Every axis must be homed again.
    be.GoldenClearAllHomeFlags();
    const MotorReloadResult rr = be.GoldenReloadMotorParams();                  //AI(W906-MT-E2) 20260925: the rest of golden InitialMotorParameter() -- before the zeroing, golden order
    int done = 0, failed = 0;
    std::string firstFail;
    const int n = be.Pci1203AxisCount();
    for (int ax = 0; ax < n; ++ax) {
        if (!be.Pci1203AxisOpened(ax)) continue;
        Pci1203Cmd c;
        c.kind = kCmdAxSetCmdPos; c.wireId = wireId; c.axis = ax; c.value = 0.0;   // golden SetCommand(0)
        const Pci1203CmdResult rc = be.Pci1203Execute(c);
        Pci1203Cmd a;
        a.kind = kCmdAxSetActPos; a.wireId = wireId; a.axis = ax; a.value = 0.0;   // golden SetPosition(0)
        const Pci1203CmdResult ra = be.Pci1203Execute(a);
        if (CmdFailed(rc) || CmdFailed(ra)) { ++failed; if (firstFail.empty()) firstFail = "ax" + std::to_string(ax) + ": " + CmdWhy(CmdFailed(rc) ? rc : ra); }
        else ++done;
        NoteIssued(be, ax);
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    char b[320];
    //AI(W906-MT-E2) 20260925: the message and the "reload" block say what came back from Mot_Table.csv.
    std::snprintf(b, sizeof(b), "HomeFlag=0 on every motor; Mot_Table values re-applied to %d motor(s)%s; PCIL132_SetPos(0) on %d opened 1203 axis(es), %d failed; home every axis again",
                  rr.applied, rr.identityChanged ? " (some rows changed identity -- restart wb_serve for those)" : "", done + failed, failed);
    std::string msg(b);
    if (!rr.read) msg += "; Mot_Table NOT re-read: " + rr.why;
    if (!firstFail.empty()) msg += "; first failure " + firstFail;
    AckHead(w, r, (failed || !rr.read || rr.identityChanged) ? "partial" : "reloaded", "pci1203", msg);
    w.Key("axesZeroed").Number((wb_int64)done);
    w.Key("axesFailed").Number((wb_int64)failed);
    w.Key("reload").BeginObject();
    w.Key("read").Bool(rr.read);
    if (!rr.read) w.Key("why").String(rr.why);
    w.Key("applied").Number((wb_int64)rr.applied);
    w.Key("identityChanged").Number((wb_int64)rr.identityChanged);
    if (!rr.firstChanged.empty()) w.Key("firstChanged").String(rr.firstChanged);
    w.Key("restartNeeded").Bool(rr.identityChanged > 0);
    w.EndObject();
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// =============================================================================
//  AI(W906-MT-E2) 20260925: the Motor Test functions that needed no ruling -- motor selection, the parameter grid,
//  Copy From, the speed scroll bar / edtSpeed. Each follows its golden handler; the 1203 card is written only where
//  golden's own SetSpeed would write it on an opened axis.
// =============================================================================
bool AnyHomeActive()                                                            // golden bSingleHome
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (g_homes[i].active) return true;
    return g_ls.homePending;                                                    //AI(W906-MT-E3c) 20260925: Light Scale's case 0 set bSingleHome too
}

// the motor of a Motor Test page action (golden ActiveIndex): resolved, golden object present, selectable.
bool PageMotor(const MotorAccessReq& r, IMotorAccessBackend& be, MotionCtx& m, std::string& who, std::string& why)
{
    if (r.motors.empty()) { why = r.action + ": 沒有選馬達（golden: ActiveIndex==-1 → return）"; return false; }
    who = r.action + " " + r.motors[0];
    if (!be.Resolve(r.motors[0], m.a)) { why = r.action + ": 馬達表上沒有 " + r.motors[0]; return false; }
    if (m.a.motIndex < 0 || !be.GoldenMotor(m.a.motIndex, m.g)) {
        why = who + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL；golden 在這裡會解參考 NULL）"; return false;
    }
    if (!m.g.selectable) { why = NotSelectable(who); return false; }
    return true;
}

// ---- selectMotor：lM00Click :734 ／ lpA00Click :762 ----
//   golden, in this order:
//     #ifndef SOFT_SIMULTE  if(MOT[Tag].Motor->Enable==false) return;    <- BEFORE anything else (nothing is released)
//     btnLoopMove->Down=false; btnHome->Down=false;                       <- CancelAllJobs, no stop (Down=false fires no OnClick)
//     ShowMotorSelect(old,0); ActiveIndex=Tag; ShowMotorSelect(new,1)     <- page; C++ keeps the Alias (runtime.selectedMotor)
//     scrlbrMotorSpeed->Position=1;                                       <- g_jogPct=1: the jog runs at 1% until the bar moves
//     MOT[Tag].SetSpeed(1); ... edtSpeed->Text=1 (-> edtSpeedChange -> SetSpeed(1) again: the same values)
//     edtMotorRate->Text=ReadRate(); edtMotorRange->Text=ReadRange();     <- the ack's cur (rate, range)
//   golden's SetSpeed(1) WRITES THE CARD on an opened axis (TMyEtherCatMotor::SetSpeed -> PAR_AxVelLow/VelHigh/Acc/Dec), so a
//   1203 axis gets those four values here (Send1203Speed, PTP family), and the golden object gets SetSpeed(1) for its iSpeed.
//   ⚠ golden leaves bSingleHome alone (the single home runs on in MainProc with its button up); here the HOME job is dropped
//     like every other AllBtnUp in this file (HomeFlag stays 0 -- home that axis again). The safe direction; noted.
MotorAccessOutcome DoSelectMotor(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (r.motors.empty()) return Refuse("selectMotor: 沒有指定馬達（golden lM00Click 的 Sender->Tag）");
    MotionCtx m;
    std::string who, why;
    if (!PageMotor(r, be, m, who, why)) return Refuse(why);                    // golden :740-743 returns before anything
    CancelAllJobs(be, "motor selected (golden lM00Click :744-745: btnLoopMove/btnHome->Down=false, no stop)");
    const std::string prev = g_selMotor;
    g_selMotor = r.motors[0];
    g_mtShown = true;                                                           //AI(W906-MT-E3c) 20260925: a selection comes from an open page
    g_jogPct[m.a.motIndex] = 1;                                                // golden :755 scrlbrMotorSpeed->Position=1
    g_jogSpd[m.a.motIndex] = MotorSpeedFromPct(1, m.g);                        //AI(W906-MT-FIX1) 20260926: the 1% jog values, fixed now
    g_ptpPct[m.a.motIndex] = 1;                                                //AI(W906-MT-E3c) 20260925: golden :756 SetSpeed(1) -> the PTP pct
    be.GoldenSetSpeed(m.a.motIndex, 1, false);                                  // golden :756 MOT[Tag].SetSpeed(1)
    std::string note, layer = "MOT";
    bool sent = false;
    if (m.a.Is1203()) {
        std::string w1;
        if (MotorSpeedFromPct(1, m.g).skip) note = "golden SetSpeed sets no speed (Index axis, or PJogHighSpeed==0)";
        else if (!m.a.tableEnable)          note = "Enable=0 (Mot_Table): golden never opened this axis, its SetSpeed stops at the object";
        else if (!be.Pci1203Ready(w1))      note = "card speed not sent -- 1203 control not available: " + w1;
        else if (m.a.axis < 0)              note = "card speed not sent -- " + m.a.why;
        else if (!Send1203Speed(be, wireId, m, 1, w1, note)) note = "card speed NOT set -- " + w1;   // golden: a WAR16122 popup
        else { sent = true; layer = "MOT+pci1203"; }
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "selected", layer, "selected (golden lM00Click): SetSpeed(1)" + (note.empty() ? std::string() : "; " + note));
    w.Key("selectedMotor").String(g_selMotor);
    w.Key("previousMotor"); if (prev.empty()) w.Null(); else w.String(prev);
    w.Key("motIndex").Number((wb_int64)m.a.motIndex);
    w.Key("speedPct").Number((wb_int64)1);
    w.Key("cardSpeedSent").Bool(sent);
    WriteCurOf(w, be, m.a.motIndex);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- setParamCell：strngrdMotorSelectCell :1176-1222 ----
//   golden: `if(ACol!=1 || ARow==0) return; if(ActiveIndex==-1) return;` -> keypad (rows 8/9 N_DOUBLE + atof, the others
//   N_INTEGER + atoi, no range check) -> the row's setter (memory only, golden same: TMyEtherCatMotor::SetInitSpeed /
//   SetRange and HTMotor::Set*DataBase touch no card) -> btnLoopMove->Down=false -> HomeFlag=0 -> UpdateMotorParameter.
//   ⚠ golden writes the cell even when the keypad is cancelled (Edit2 still holds the old text) -- the page sends that too.
MotorAccessOutcome DoSetParamCell(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    MotionCtx m;
    std::string who, why;
    if (!PageMotor(r, be, m, who, why)) return Refuse(why);
    double rowD = 0.0, v = 0.0;
    if (!ParamNum(r, "row", rowD)) return Refuse(who + ": 缺 row（golden strngrdMotor 的 ARow）");
    if (!ParamNum(r, "value", v))  return Refuse(who + ": 缺 value（golden Edit2 經鍵盤後的值）");
    if (!(rowD >= 1.0 && rowD <= 10.0) || rowD != (double)(int)rowD)           // range first: (int) of a huge double is undefined
        return Refuse(who + ": row 必須是 1..10 —— golden strngrdMotorSelectCell :1179 `if(ACol!=1 || ARow==0) return;`，表只有第 1..10 列");
    const int row = (int)rowD;
    double ret = v;                                                             // rows 8/9: golden atof (N_DOUBLE keypad)
    if (row != 8 && row != 9) {
        if (!(v > -2147483649.0 && v < 2147483648.0))
            return Refuse(who + ": 值超出 int 範圍 —— golden atoi 的結果不可預期，不寫");
        ret = (double)(int)v;                                                   // golden atoi (N_INTEGER keypad): truncated toward 0
    } else if (v != v) {
        return Refuse(who + ": value 不是數字");
    }
    be.GoldenSetCell(m.a.motIndex, row, ret);                                   // golden :1198-1218
    CancelLoop(be, false, "parameter cell edited (golden SelectCell :1219 btnLoopMove->Down=false, no stop)");
    be.GoldenSetHomeFlag(m.a.motIndex, 0);                                      // golden :1220 MOT[ActiveIndex].HomeFlag=0
    static const char* const kRowName[11] = { "", "InitialSpeed", "JogHighSpeed", "JogLowSpeed", "HomeHighSpeed", "HomeLowSpeed",
                                              "SoftLimitP", "SoftLimitN", "Acc", "Dec", "Range" };   // golden FormShow :1011-1020
    webbridge::JsonWriter w;
    w.BeginObject();
    char b[160];
    std::snprintf(b, sizeof(b), "MOT[%d] row %d (%s) = %g (memory only, golden same); LoopMove up, HomeFlag=0 (golden :1219-1220)",
                  m.a.motIndex, row, kRowName[row], ret);
    AckHead(w, r, "set", "MOT", b);
    w.Key("motIndex").Number((wb_int64)m.a.motIndex);
    w.Key("row").Number((wb_int64)row);
    w.Key("value").Number(ret);
    WriteCurOf(w, be, m.a.motIndex);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- copyFrom：BitBtn1Click :1384-1401 ----
//   golden: Source = (str[1]-'0')*10 + (str[2]-'0') of cbbMotorName's text (the page sends that integer, parsed the golden
//   way -- so "M108" gives 10); `if(ActiveIndex==-1 || Source>24 || Source<0) return;` then six copies (memory only) and
//   UpdateMotorParameter. HomeFlag and LoopMove are NOT touched (golden).
//   ⚠ golden returns silently on a bad Source; here the page is told why (ok=false). Nothing is copied either way.
MotorAccessOutcome DoCopyFrom(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    MotionCtx m;
    std::string who, why;
    if (!PageMotor(r, be, m, who, why)) return Refuse(why);
    double sD = 0.0;
    if (!ParamNum(r, "source", sD)) return Refuse(who + ": 缺 source（golden cbbMotorName 第 2、3 個字元算出的編號）");
    char b[200];
    if (!(sD >= 0.0 && sD <= 24.0) || sD != (double)(int)sD) {                 // range first: (int) of a huge double is undefined
        std::snprintf(b, sizeof(b), ": Source=%g —— golden BitBtn1Click :1392 `if(ActiveIndex==-1 || Source>24 || Source<0) return;`（只抄得到 M00..M24）", sD);
        return Refuse(who + b);
    }
    const int src = (int)sD;
    MotorGolden gs;
    if (!be.GoldenMotor(src, gs)) {
        std::snprintf(b, sizeof(b), ": MOT[%d].Motor 為 NULL —— golden 在這裡會解參考 NULL，不抄", src);
        return Refuse(who + b);
    }
    const int mi = m.a.motIndex;
    be.GoldenSetParam(mi, kParamJogHigh,  (int)gs.jogHigh);                     // golden :1394
    be.GoldenSetParam(mi, kParamJogLow,   (int)gs.jogLow);                      // :1395
    be.GoldenSetParam(mi, kParamHomeHigh, (int)gs.homeHigh);                    // :1396
    be.GoldenSetParam(mi, kParamHomeLow,  (int)gs.homeLow);                     // :1397
    be.GoldenSetParam(mi, kParamSoftP,    gs.softP);                            // :1398
    be.GoldenSetParam(mi, kParamSoftN,    gs.softN);                            // :1399
    webbridge::JsonWriter w;
    w.BeginObject();
    std::snprintf(b, sizeof(b), "MOT[%d] <- MOT[%d]: JogHigh/JogLow/HomeHigh/HomeLow/SoftLimitP/SoftLimitN (memory only; HomeFlag untouched, golden same)", mi, src);
    AckHead(w, r, "copied", "MOT", b);
    w.Key("motIndex").Number((wb_int64)mi);
    w.Key("source").Number((wb_int64)src);
    WriteCurOf(w, be, mi);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- setSpeed：scrlbrMotorSpeedScroll :791-810（jog=true）／edtSpeedChange :1587-1603（jog=false）----
//   scroll bar: `if(ActiveIndex==-1) return; if(bSingleHome==true) return;` -> MOT.SetSpeed(pos, true) (PTP + jog family)
//               -> edtSpeed->Text=pos (-> edtSpeedChange -> SetSpeed(pos): the same PTP values again, not re-sent here)
//               -> `if(btnLoopMove->Down)` SetSpeed(pos, true) for every visible cbUsing-checked motor.
//   edtSpeed:   `if(ActiveIndex==-1) return;` -> SetSpeed(atoi(edtSpeed)) (PTP family only); no bSingleHome check (golden).
//   pct must be 1..100: the scroll bar's Min=1 / Max=100 (dfm :1609, :799) and edtSpeed's keypad bounds (edtSpeedClick :1715).
//   Index axes (MTestY1/Z1/Z2/Y2): golden TMyMotor::SetSpeed is empty for them (Motor/mymotor.cpp:325-329) -> nothing.
//   ⚠ GAP: the LoopMove branch's "every cbUsing-checked motor" -- the page sends no cbUsing (All mode is not wired), so only
//     this motor is set (the single-axis loop runs on this motor anyway).
MotorAccessOutcome DoSetSpeed(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    MotionCtx m;
    std::string who, why;
    if (!PageMotor(r, be, m, who, why)) return Refuse(why);
    std::map<std::string, bool>::const_iterator jf = r.flag.find("jog");
    if (jf == r.flag.end()) return Refuse(who + ": 缺 jog（true = 速度捲軸 scrlbrMotorSpeed，false = edtSpeed）");
    const bool jog = jf->second;
    double pD = 0.0;
    if (!ParamNum(r, "pct", pD)) return Refuse(who + ": 缺 pct");
    if (!(pD >= 1.0 && pD <= 100.0) || pD != (double)(int)pD)                  // range first: (int) of a huge double is undefined
        return Refuse(who + ": pct 必須是 1..100 的整數（golden 捲軸 Min=1／Max=100、edtSpeed 鍵盤上下限 100／1）");
    const int pct = (int)pD;
    const int mi = m.a.motIndex;
    webbridge::JsonWriter w;
    w.BeginObject();
    if ((jog && AnyHomeActive()) || m.g.indexMotor) {                           // golden returns / does nothing
        AckHead(w, r, "ignored", "none", m.g.indexMotor
                ? "Index 軸（MTestY1/Z1/Z2/Y2）：golden TMyMotor::SetSpeed 對這四軸整段空白 —— 不設速度"
                : "HOME 進行中：golden scrlbrMotorSpeedScroll :796 `if(bSingleHome==true) return;` —— 速度不變");
        w.Key("pct").Number((wb_int64)pct);
        w.Key("jog").Bool(jog);
        WriteCurOf(w, be, mi);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    std::string note, layer = "MOT";
    if (m.a.Is1203()) {
        if (!m.a.tableEnable) return Refuse(who + ": " + kEnable0Why);
        std::string w1;
        if (!be.Pci1203Ready(w1)) return Refuse(who + "（PCI1203）: 1203 控制層不可用 —— " + w1);
        if (m.a.axis < 0) {
            const std::string msg = who + "（PCI1203）: " + m.a.why;
            be.Pci1203NoteRefusal(wireId, "motor.access setSpeed", msg);
            return Refuse(msg);
        }
        const Motor1203Speed spNew = MotorSpeedFromPct(pct, m.g);
        if (!Send1203SpeedValues(be, wireId, m, pct, spNew, w1, note, jog ? (kFamPtp | kFamJog) : kFamPtp)) {
            //AI(W906-MT-FIX1) 20260926: a failed scroll-bar write must not leave the jog at an OLD, possibly higher speed while the
            //  page shows the new position (review low). Keep the slower of the two: the next jog re-sends it (DoJog).
            std::string jogNote;
            if (jog) {
                std::map<int, Motor1203Speed>::const_iterator old = g_jogSpd.find(mi);
                if (old == g_jogSpd.end() || spNew.velHigh < old->second.velHigh) { g_jogSpd[mi] = spNew; g_jogPct[mi] = pct; }
                jogNote = "; jog 之後用 " + std::to_string(JogPctOf(mi)) + "%（取新舊兩者較慢的）";
            }
            return Refuse(who + ": " + w1 + jogNote);
        }
        if (jog) g_jogSpd[mi] = spNew;                                          //AI(W906-MT-FIX1) 20260926: the values just written
        be.GoldenSetSpeed(mi, pct, false);                                      // the golden object's iSpeed (ReadSpeed / lblRealSpeed); no card
        layer = "pci1203";
    } else {
        be.GoldenSetSpeed(mi, pct, jog);                                        // golden MOT.SetSpeed(pos, true) / SetSpeed(edtSpeed)
        if (jog) be.GoldenSetSpeed(mi, pct, false);                             // the scroll's edtSpeed->Text=pos -> edtSpeedChange
        note = "MOT[" + std::to_string(mi) + "].SetSpeed(" + std::to_string(pct) + (jog ? ", true)" : ")");
    }
    if (jog) g_jogPct[mi] = pct;                                                // the jog now runs at this position (DoJog)
    g_ptpPct[mi] = pct;                                                         //AI(W906-MT-E3c) 20260925: golden SetSpeed(pct[, true]) -> this motor's last PTP pct
    //AI(W906-MT-E3c) 20260925: golden :803-809 `if(btnLoopMove->Down)` -> MOT[i].SetSpeed(Position, true) for every visible
    //  cbUsing-checked motor. The page sends the checked motors with an All-mode loop (lead contract), so a running All loop
    //  gets them here; the selected motor was just set above (golden sets it a second time, same values -- not re-sent).
    //  A single-mode loop has no checked list from the page: that half stays a gap there.
    if (jog && g_loop.active && g_loop.all) {
        int nSet = 0;
        std::string firstFail;
        for (std::size_t k = 0; k < g_loop.axes.size(); ++k) {
            LoopAxis& ax = g_loop.axes[k];
            const int ai = ax.a.motIndex;
            if (ai == mi) continue;
            MotionCtx lm; lm.a = ax.a;
            if (!be.GoldenMotor(ai, lm.g) || lm.g.indexMotor) continue;         // golden TMyMotor::SetSpeed: empty for the Index four
            if (ax.a.Is1203()) {
                std::string w1, n1;
                if (!ax.a.tableEnable || !be.Pci1203Ready(w1) || ax.a.axis < 0) continue;   // golden: an unopened axis' SetSpeed stops at the object
                const Motor1203Speed spAx = MotorSpeedFromPct(pct, lm.g);
                if (!Send1203SpeedValues(be, wireId, lm, pct, spAx, w1, n1, kFamPtp | kFamJog)) { if (firstFail.empty()) firstFail = ax.motorId + ": " + w1; continue; }
                g_jogSpd[ai] = spAx;                                            //AI(W906-MT-FIX1) 20260926
                be.GoldenSetSpeed(ai, pct, false);
            } else {
                be.GoldenSetSpeed(ai, pct, true);
            }
            g_jogPct[ai] = pct; g_ptpPct[ai] = pct;
            ++nSet;
        }
        note += "; LoopMove(All): SetSpeed(" + std::to_string(pct) + ", true) also on " + std::to_string(nSet) + " checked motor(s) (golden :803-809)" +
                (firstFail.empty() ? std::string() : "; FAILED " + firstFail);
    } else if (jog && g_loop.active) {
        note += "; GAP: golden also sets every cbUsing-checked motor during LoopMove -- a single-mode loop brings no checked list from the page";
    }
    AckHead(w, r, "speedSet", layer, note);
    w.Key("pct").Number((wb_int64)pct);
    w.Key("jog").Bool(jog);
    w.Key("jogPct").Number((wb_int64)JogPctOf(mi));
    WriteCurOf(w, be, mi);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

//AI(W906-MT-E2) 20260925: golden LastHomePos -- TMyEtherCatMotor::EtherCatMotHome case 20 (golden Motor/myEthercatmotor.cpp
//  :1357-1370; port :1744) `LastHomePos=-ReadPos();` right BEFORE SetCommand(0)/SetPosition(0): where the OLD zero lies in
//  the new coordinates. golden Timer1Timer shows it in edtHomeOffset when the home ends (uMotorTest.cpp:965). Read here from
//  a FRESH monitor sample (cmdPos = golden ReadRealPos's Acm_AxGetCmdPosition), in user units (ReadPos = ReadRealPos x
//  GearRatio). Direction=1 axes record nothing: their positions are withheld until the direction convention is ruled (§0 #5).
//  [AI(W906-MERGE-56bbf785) 20260926: ruled -- 6B (RULINGS_20260925 #13, a 1203 axis ignores Direction): recorded like any axis.]
void RecordLastHomePos(IMotorAccessBackend& be, const HomeJob& h)
{
    double card = 0.0;
    if (SampleStale(be, h.axis) || !be.Pci1203CmdPos(h.axis, card)) return;
    be.GoldenSetLastHomePos(h.mi, -MotorCardToUser(card, h.g.gearRatio));
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
                // AI(W906-MT-E1) 20260925: card-side home only (golden EtherCatMotHome case 10 -> 20): READY ->
                //   HomeDelay 0.3 s -> SetCommand(0), SetPosition(0). DS402 homes never do this (the drive sets
                //   its own origin; zeroing there is what makes a failed home look like a successful one).
                if (h.cardSide && !h.zeroed) {
                    if (h.zeroWaitMs < 0) { h.zeroWaitMs = 0; continue; }       // first READY tick: start the 0.3 s
                    h.zeroWaitMs += ms;
                    if (h.zeroWaitMs < 300) continue;
                    RecordLastHomePos(be, h);                                   //AI(W906-MT-E2) 20260925: golden `LastHomePos=-ReadPos();` BEFORE the zeroing
                    Pci1203Cmd zc; zc.kind = kCmdAxSetCmdPos; zc.axis = h.axis; zc.value = 0.0;
                    const Pci1203CmdResult zr = be.Pci1203Execute(zc);
                    Pci1203Cmd za; za.kind = kCmdAxSetActPos; za.axis = h.axis; za.value = 0.0;
                    const Pci1203CmdResult ar = be.Pci1203Execute(za);
                    h.zeroed = true;
                    NoteIssued(be, h.axis);
                    //  golden SetCommand/SetPosition only pop WAR16122 on failure and EtherCatMotHome still returns
                    //  true (= homed). User ruling: golden guards exactly -- so the same outcome, with the failure
                    //  kept in the job note instead of a popup.
                    const std::string zfail = (CmdFailed(zr) || CmdFailed(ar))
                        ? ("; WAR16122 (golden popup): zeroing failed -- " + CmdWhy(CmdFailed(zr) ? zr : ar)) : std::string();
                    //AI(W906-MERGE-56bbf785) 20260926: + the laptop's iLastRotatorDirP=true (R-W5B-5, golden MotorHome case 20) on this
                    //  machine-only exit too (the armZ card-side home reaches the common line below on its next tick).
                    if (!h.g.armZ) { be.GoldenSetRotatorLastDirP(h.mi, true); be.GoldenSetHomeFlag(h.mi, 1); SingleHomeSucceeded(); CancelHome(be, h, false, "card-side home done, SetCommand(0)+SetPosition(0) -> HomeFlag=1" + zfail); continue; }
                    if (!zfail.empty()) g_lastJobNote = "home " + h.motorId + zfail;
                    continue;                                                   // armZ: next tick goes to ZSafePos (fresh sample)
                }
                //AI(W906-MT-E2) 20260925: DS402 home: the same golden formula at completion. ⚠ Not the same meaning: the
                //  drive defines the origin itself (nothing is zeroed here), so this is -(position when the drive says done)
                //  and will usually read about 0 -- golden never homed a DS402 drive, there is no golden value to match.
                if (!h.cardSide) RecordLastHomePos(be, h);
                //AI(W906-W5-b) 20260925: 覆核 R-W5B-5 —— golden TMyMotor::MotorHome case 20 歸零完成（mymotor.cpp:1721）`iLastRotatorDirP=true;`
                //  —— 在 ProcessSingleMotorHome 的 case 500（arm Z 去 ZSafePos）之前，所以每一軸都設（旋轉站的背隙補償讀它，:2012-2045）
                //AI(W906-MERGE-56bbf785) 20260926: machine's card-side block + LastHomePos above, then the laptop's line (both kept).
                be.GoldenSetRotatorLastDirP(h.mi, true);
                if (h.g.armZ) {                                                 // golden ProcessSingleMotorHome case 500：MotorMove(ZSafePos)（下一段）
                    h.phase = 1; h.elapsedMs = 0; h.zIssued = false;
                    continue;
                }
                be.GoldenSetHomeFlag(h.mi, 1); SingleHomeSucceeded();
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
                be.GoldenSetHomeFlag(h.mi, 1); SingleHomeSucceeded();
                CancelHome(be, h, false, "case 500: golden MotorMove(ZSafePos) returned -1/-2/-3 (door open or soft limit) -> no move, HomeFlag=1 (golden `if(MotorMove())` treats it as done)");
                continue;
            }
            if (be.GoldenMoveLocked(h.mi)) {                                    // golden：PCIL132_StopMotor 然後回 0（除非剛好在目標）
                Stop1203(be, h.axis, -1);                                       // AI(W906-MT-E1): golden PCIL132_StopMotor = DecStop
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
                    be.GoldenSetHomeFlag(h.mi, 1); SingleHomeSucceeded();                              // golden case 500：HomeFlag=true
                    CancelHome(be, h, false, "home done, Z at ZSafePos -> HomeFlag=1");
                    continue;
                }
            }
            if (h.elapsedMs > kZSafeTimeoutMs) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, true, "ZSafePos timeout -> HomeFlag=2"); }
        }
    }
}

//AI(W906-MT-E2) 20260925: one step of golden DoLoopMove per call (golden: one step per 5 ms Timer1 tick). Called by the
//  500 ms beat (every loop) AND right after each 1203 Poll (MotorAccessPollTick, 1203 loops only): the arrival is seen on
//  the fresh sample and the leg is timed there. Times are LoopNow() (the backend's QPC ms; the beat sum without one).
//AI(W906-MT-E3c) 20260925: one golden Timer1 tick of DoLoopMove's All branch (golden uMotorTest.cpp:468-501 / :577-617),
//  see DoLoopMoveAll. Refreshes each motor's golden values first (the interlocks read the live soft limits, as golden's
//  MotorMove does).
void TickLoopAll(IMotorAccessBackend& be)
{
    LoopJob& L = g_loop;
    const double now = LoopNow(be);
    if (L.task == 10 || L.task == 60) {
        if (now >= L.waitUntilMs) L.task = (L.task == 10) ? 50 : 1;             // golden LoopWait.Off()
        return;
    }
    bool allMotFlag = true;                                                     // golden AllMot_Flag
    for (std::size_t k = 0; k < L.axes.size(); ++k) {
        LoopAxis& ax = L.axes[k];
        const int target = (L.task == 1) ? ax.pos1 : ax.pos2;                   // golden atoi(edPos1 / edPos2)
        MotorGolden g;
        if (be.GoldenMotor(ax.a.motIndex, g)) ax.g = g;
        std::string note;
        ax.lastRet = ax.a.Is1203() ? GoldenMove1203(be, ax.a, ax.g, target, -1, note)
                                   : be.GoldenMotorMove(ax.a.motIndex, target, 0, false);
        ax.flag = (ax.lastRet != 0);                                            // golden `AllMotMoveFlag=MOT[i].MotorMove(p);` (int -> bool)
        if (!note.empty() && ax.lastRet < 0) g_lastJobNote = "loop (All) " + ax.motorId + ": " + note + " -> counts as arrived (golden)";
        if (!ax.flag) allMotFlag = false;
    }
    if (!allMotFlag) return;
    if (L.task == 50) {                                                         // golden :600-603
        ++L.count;
        g_loopAverage += (unsigned long)LoopLatchMs(now);
        g_avgTime = (double)(float)((double)g_loopAverage / (double)L.count);
        g_hasAvg = true;
    }
    if (L.waitMs > 0) { L.waitUntilMs = now + L.waitMs; L.task = (L.task == 1) ? 10 : 60; }   // golden LoopWait.Set0_1SecAndOn(Wait)
    else L.task = (L.task == 1) ? 50 : 1;
}

void TickLoop(IMotorAccessBackend& be)
{
    LoopJob& L = g_loop;
    if (!L.active) return;
    if (L.all) { TickLoopAll(be); return; }                                     //AI(W906-MT-E3c) 20260925
    const double now = LoopNow(be);
    if (L.task == 10 || L.task == 60) {
        if (now >= L.waitUntilMs) { L.task = (L.task == 10) ? 50 : 1; L.cmdIssued = false; }   // golden LoopWait.Off()
        return;
    }
    const int target = (L.task == 1) ? L.pos1 : L.pos2;
    const std::string who = "loopMove " + L.motorId;
    if (!g_latchArmed) { g_latchArmed = true; g_latchMs = now; }               // golden `if(flag==false){flag=true; tLoopMoveTimer.LatchCycleTime(true);}`
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
            L.cmdIssued = true; L.legStartMs = now; L.wrongReady = 0; L.wrongPoll = 0;
            return;
        }
        unsigned st = 0;
        if (be.Pci1203AxisState(L.a.axis, st) && IsErrorState(st)) { CancelLoop(be, false, "ERROR_STOP -> loop stopped"); return; }
        double card = 0;
        if (IsReadyState(st) && !SampleStale(be, L.a.axis) && be.Pci1203CmdPos(L.a.axis, card)) {
            const int pos = MotorCardToUser(card, L.g.gearRatio);
            if (pos - target <= 2 && target - pos <= 2) arrived = true;        // golden CompareCommandPos 的 iGap=2
            else {
                const unsigned long pc = be.Pci1203PollCount();
                if (pc != L.wrongPoll) { L.wrongPoll = pc; ++L.wrongReady; }
                if (L.wrongReady >= 2) { CancelLoop(be, false, "stopped before target -> loop stopped"); return; }
            }
        }
        if (!arrived && now - L.legStartMs > kLoopLegTimeoutMs) { CancelLoop(be, true, "leg timeout -> loop stopped"); return; }
    }
    if (!arrived) return;
    LoopLegTimed(L, now);                                                       // lblJogPTime / lblJogNTime, dwLoopCount++, Average
    L.cmdIssued = false;
    if (L.waitMs > 0) { L.waitUntilMs = now + L.waitMs; L.task = (L.task == 1) ? 10 : 60; }   // golden LoopWait.Set0_1SecAndOn(Wait)
    else L.task = (L.task == 1) ? 50 : 1;
}

// =============================================================================
//  AI(W906-MT-E3c) 20260925: Motor Power, FormShow / FormClose, Light Scale -- golden uMotorTest.cpp, EastSun rulings
//  R1/R5/R7/R9 20260925 and the lead's defaults (see WebMotorAccess.h). Each handler cites the golden lines it follows.
// =============================================================================
const char* kMotorPowerOffQ = "Sure to turn off motor power? (確定要關掉馬達電源？)";   // golden :1628, verbatim

int StopAllOpened1203(IMotorAccessBackend& be, long long wireId)
{
    std::string why;
    int n = 0;
    if (!be.Pci1203Ready(why)) return 0;
    for (int ax = 0; ax < be.Pci1203AxisCount(); ++ax) {
        if (!be.Pci1203AxisOpened(ax)) continue;
        Stop1203(be, ax, wireId);                                               // AI(W906-MT-E1): StopDec + ExtDrive 0 (golden DecStop)
        ++n;
    }
    g_jogs.clear();
    return n;
}

// golden's post-DoMotorPowerOn line (btnMotorPowerClick :1635 / FormShow :1042), once the cross-tick 1 s is over.
void TickPowerOn(IMotorAccessBackend& be)
{
    if (!g_powerPending) return;
    if (!be.GoldenMotorPowerOnStep()) return;                                   // golden `if(MotorPowerOnDelay.Off()) break;`
    be.GoldenServerOn();                                                        // SW[SwServerON].On()
    g_powerPending = false;
    g_lastJobNote = "Motor Power On (" + g_powerFrom + "): DoMotorPowerOn's 1 s is over -> SW[SwServerON].On()";
}
// golden DoMotorPowerOn() as a cross-tick job (lead default 20260925; W906_DoMotorPowerOnBegin/Step, csystem.h). Begin runs
//  golden's first loop pass; one Step() right away covers the SIM arm (golden returns at once there).
void StartPowerOn(IMotorAccessBackend& be, const std::string& from)
{
    be.GoldenMotorPowerOnBegin();
    g_powerPending = true;
    g_powerFrom = from;
    TickPowerOn(be);
}

void WritePowerFields(webbridge::JsonWriter& w, IMotorAccessBackend& be, bool on)
{
    w.Key("motorPowerOn").Bool(on);
    w.Key("caption").String(on ? "Motor Power On" : "Motor Power Off");         // golden :1636 / :1642 (:1043 / :1049)
    w.Key("pending").Bool(g_powerPending);
    const MotorPowerState ps = be.GoldenMotorPower(false);
    w.Key("relayOn").Bool(ps.relayOn);
    w.Key("relayCard"); if (ps.cardKnown) w.Bool(ps.card); else w.Null();
    w.Key("motorPowerState").Bool(ps.motorPowerState);
    w.Key("routeLastWrite").String(be.GoldenRouteLastWrite());
}

// ---- motorPowerToggle：btnMotorPowerClick :1621-1645 ----
//   golden:  bool bRelayOn=SW[SwMotorRelay].OutValue;  AllBtnUp();
//            if(bRelayOn) { if(mrNo==MessageDlg(kMotorPowerOffQ)) return; }
//            bRelayOn=!bRelayOn;
//            ON : DoMotorPowerOn(); SW[SwServerON].On(); Caption="Motor Power On";
//            OFF: fHome->GaliMotorServoOff("TfMotorTest::btnMotorPowerClick"); Caption="Motor Power Off";
//   No SystemStart / door guard (golden has none; R9: not blocked). The question is the page's (confirmOff=true = "Yes"):
//   without it, with the relay ON, C++ refuses with golden's text and `needConfirm:true` -- AllBtnUp has already run, as in
//   golden (it precedes the dialog). OFF also sends Stop1203 to every axis the monitor opened (golden StopAllMotor cannot
//   reach them; GaliMotorServoOff's own hook does it again, harmless). Power off keeps HomeFlag (golden does not clear it).
MotorAccessOutcome DoMotorPower(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (g_powerPending)
        return Refuse("motorPowerToggle: Motor Power On 還在進行（golden DoMotorPowerOn 的 1 秒等待，這裡跨拍進行、不凍結主執行緒）—— 一秒後再按");
    const MotorPowerState ps = be.GoldenMotorPower(true);                       // lead default: OutValue synced from the card read-back first
    const bool bRelayOn = ps.relayOn;                                           // golden :1623
    CancelAllJobs(be, "Motor Power (golden btnMotorPowerClick :1624 AllBtnUp)");
    std::map<std::string, bool>::const_iterator cf = r.flag.find("confirmOff");
    //AI(W906-MT-FIX1) 20260926: 方向要跟操作員確認的一致（審查 medium）。golden 在同一條執行緒上讀 OutValue → 問 → 照讀到的值切，
    //   按了「Yes, turn off」一定是關電；網頁的判斷來自上一次 runtime，問的期間繼電器可能被 IO 頁／EMG 改掉。
    //   (a) 頁面帶了 expectRelayOn（按下時以為的狀態）而現值不同 ⇒ 不切、回 relay-changed，頁面照 golden 重新判斷再問；
    //   (b) confirmOff=true（操作員答了「確定要關掉馬達電源？」）而現值是關的 ⇒ 絕不開電（舊頁面沒有 expectRelayOn 也一樣）。
    std::map<std::string, bool>::const_iterator ex = r.flag.find("expectRelayOn");
    const bool confirmedOff = (cf != r.flag.end() && cf->second);
    if ((ex != r.flag.end() && ex->second != bRelayOn) || (confirmedOff && !bRelayOn))
        return Refuse(std::string("motorPowerToggle: relay-changed -- 按下時以為 SwMotorRelay ") +
                      ((ex != r.flag.end() ? ex->second : confirmedOff) ? "開著" : "關著") + "，現在是" + (bRelayOn ? "開著" : "關著") +
                      "（" + (ps.synced ? "照卡片回讀" : "OutValue") + "）—— 沒有切換；請看目前狀態再按一次" +
                      (confirmedOff && !bRelayOn ? "（已確認關電的命令不會開電）" : ""));
    if (bRelayOn && (cf == r.flag.end() || !cf->second))
        return Refuse(std::string("needConfirm:true -- ") + kMotorPowerOffQ +
                      " —— golden :1628 MessageDlg YES/NO：繼電器 SwMotorRelay 是開的（" + (ps.synced ? "OutValue 剛照卡片回讀更新" : "OutValue") +
                      "），頁面問過「Yes」後帶 confirmOff=true 再送");
    webbridge::JsonWriter w;
    w.BeginObject();
    if (!bRelayOn) {                                                            // golden :1632-1638
        StartPowerOn(be, "TfMotorTest::btnMotorPowerClick");
        AckHead(w, r, g_powerPending ? "powerOnPending" : "powerOn", "MOT",
                std::string("DoMotorPowerOn: SwMotorRelay On + IndexMotorBreakerOFF/MagazineBreakerOFF/CassetteBreakerOFF") +
                (g_powerPending ? "; golden's 1 s wait runs across ticks, then SW[SwServerON].On()" : "; SW[SwServerON].On()"));
        WritePowerFields(w, be, true);
    } else {                                                                    // golden :1639-1644
        const int n = StopAllOpened1203(be, wireId);
        be.GoldenMotorServoOff("TfMotorTest::btnMotorPowerClick");
        AckHead(w, r, "powerOff", "MOT",
                "fHome->GaliMotorServoOff: StopAllMotor, relay/ServerON off, bMotorPowerState=false, SystemStart=false, fAllMotorHome=false, "
                "brakes held; HomeFlag kept (golden); 1203 stop sent to " + std::to_string(n) + " axis(es)");
        w.Key("stopSent").Number((wb_int64)n);
        WritePowerFields(w, be, false);
    }
    w.Key("synced").Bool(ps.synced);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- formShow：FormShow :985-1077，C++ 那一半 ----
//   golden, in this order: fShow=true; bSingleHome=false; (edPos read -- page); btnHome/btnLoopMove Down=false; (grid captions
//   -- page); Motor Power sync `f=SW[SwMotorRelay].OutValue; f ? DoMotorPowerOn()+SW[SwServerON].On() :
//   fHome->GaliMotorServoOff("TfMotorTest::FormShow")` with the caption; select->ItemIndex=0 (page); ActiveIndex=-1;
//   (page: first tab, cbbMotorName, tsMotorDatabase -- KEPT, lead default -- and pnlStop 0x00DFD9CC).
//   The sync reads the card first (lead default, as DoMotorPower). ⚠ golden really runs GaliMotorServoOff on every open with
//   the power off: StopAllMotor, SystemStart=false, fAllMotorHome=false, brakes held (kept).
MotorAccessOutcome DoFormShow(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    g_mtShown = true;                                                           // golden :990 fShow=true (the engine reads the window registry)
    EndSingleHome(be, "FormShow (golden :994 bSingleHome=false)");
    CancelAllJobs(be, "FormShow (golden :1003-1004 btnHome/btnLoopMove Down=false)");
    const MotorPowerState ps = be.GoldenMotorPower(true);
    std::string msg;
    int n = 0;
    if (ps.relayOn) {                                                           // golden :1039-1045
        if (!g_powerPending) StartPowerOn(be, "TfMotorTest::FormShow");
        msg = "Motor Power sync: relay ON -> DoMotorPowerOn + SW[SwServerON].On()";
    } else {                                                                    // golden :1046-1051
        n = StopAllOpened1203(be, wireId);
        be.GoldenMotorServoOff("TfMotorTest::FormShow");
        msg = "Motor Power sync: relay OFF -> fHome->GaliMotorServoOff(\"TfMotorTest::FormShow\") (golden; 1203 stop sent to " + std::to_string(n) + " axis(es))";
    }
    const std::string prev = g_selMotor;
    g_selMotor.clear();                                                         // golden :1053 ActiveIndex=-1
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "shown", "MOT", msg + "; bSingleHome=false, HOME/LoopMove up, ActiveIndex=-1");
    WritePowerFields(w, be, ps.relayOn);
    w.Key("synced").Bool(ps.synced);
    w.Key("selectedMotor").Null();
    w.Key("previousMotor"); if (prev.empty()) w.Null(); else w.String(prev);
    w.Key("lightScaleActive").Bool(g_ls.timer);                                 // golden FormShow leaves Timer2 alone
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- formClose：FormClose :1347-1362，C++ 那一半 ----
//   golden: fShow=false; (WriteIniData MotorTest.ini -- the page, R5); PauseUT150Polling=false.
//   With fShow false golden's Timer1Timer returns at its first line, so a running LoopMove is never stepped again and the
//   next FormShow raises it -- ended here, without a stop (golden sends none; the leg in flight completes).
//   Not ended (golden keeps them going): Light Scale (Timer2Timer has no fShow test) and a HOME job (golden's single home
//   runs in MainProc while Motor Test OR Teach is shown; here it keeps completing -- the safe side: HomeFlag becomes 1 only
//   when the drive really finished).
MotorAccessOutcome DoFormClose(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    g_mtShown = false;
    CancelLoop(be, false, "FormClose (golden fShow=false: Timer1Timer no longer steps DoLoopMove)");
    be.GoldenFormClose();
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "closed", "MOT", std::string("fShow=false, PauseUT150Polling=false; LoopMove ended (no stop)") +
            (g_ls.timer ? "; Light Scale keeps running (golden Timer2 has no fShow test)" : ""));
    w.Key("lightScaleActive").Bool(g_ls.timer);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- Light Scale：BitBtn2Click :2096 / LightScale :1795-1978 / Timer2Timer :2031 / BitBtn3Click :2102 / SaveAsCSV :1749 /
//      btnSaveLogLightScaleDataClick :2051 / SaveAsCSV_Kaichen :1982 / rgAxisClick :2121 ----
//   EastSun R7 20260925: COMPLETELY golden, including what golden cannot stop -- the user was told the risks (the arms'
//   soft limits are +-999999 on HT9050, so the first target is PSoftLimitP-100; there is no MLightScale axis row, so the
//   scale reads 0). Kept exactly:
//     * STOP does not stop the scan (golden btnStopClick never touches Timer2); only its single home is released
//       (bSingleHome=false) and the axis is stopped -- MotorMove then counts the stopped axis as arrived and the scan goes on;
//     * pressing BitBtn2 again only toggles the timer, and LightScale(true) resets Task to 0 each press (re-home on restart);
//     * closing the page does not stop it (Timer2Timer has no fShow test); neither does an alarm, VerifyMotorAction or
//       Motor Power;
//     * the axis is golden iUseAxis, read on every tick (rgAxisClick changes it -- here when the page next sends the radio
//       group, at BitBtn2 / BitBtn3: the contract has no rgAxisClick command, see api_for_next);
//     * a refused move (door -1, soft limit -2/-3) is "arrived": the point is recorded and the scan goes on;
//     * Pitch 0 (the dfm text 'Pitch' -> atoi 0) repeats the same point for ever; rgMoveType -1 waits at case 10 for ever;
//     * no axis selected (rgAxis -1, never clicked) scans MOT[0] (golden iUseAxis is 0 until rgAxisClick runs);
//     * the save overwrites D:\LightScale\Motor{N}Positive.csv without a backup (golden SaveToFile).
//   Stepping: golden Timer2 is 10 ms, one task per tick. Here a step runs right after every 1203 Poll and on every 500 ms
//   beat, and each step runs golden ticks back to back until the task has to wait (home, move, delay) -- so no transition
//   costs a poll period. Port-only waits, all in the direction of "later, never different": a 1203 move waits for a sample
//   newer than its command (W4B-5), and a monitor encoder reading waits for a poll AFTER the delay ended.
//   Cases 100 and 200-240 (golden's round-trip mode, rgMoveType ItemIndex 2) are not ported: rgMoveType has two items
//   (dfm :2795-2805) and nothing sets Task=100, so golden cannot reach them; the contract bounds moveType to -1..1.
int LsUseAxisOf(int item)                                                        // golden rgAxisClick :2121-2133
{
    if (item == 1) return 0;                                                    // MInArmX
    if (item == 2) return 1;                                                    // MInArmY
    if (item == 3) return 19;                                                   // MOutArmX
    if (item == 4) return 20;                                                   // MOutArmY
    return -1;
}
// The page sends the radio groups' ItemIndex. rgAxisClick ran for every value the group can show (-1 = never clicked).
void LsApplyRadios(int axisItem, int moveType)
{
    if (axisItem >= 0) g_lsUseAxis = LsUseAxisOf(axisItem);
    g_lsAxisItem = axisItem;
    g_lsMoveType = moveType;
}
bool LsRadiosFrom(const MotorAccessReq& r, bool needPitch, int& axisItem, int& moveType, int& pitch, int& delayMs, std::string& why)
{
    double a = 0, t = 0, p = 0, d = 0;
    if (!ParamNum(r, "axisItem", a) || !ParamNum(r, "moveType", t)) { why = r.action + ": 缺 axisItem／moveType（golden rgAxis／rgMoveType->ItemIndex）"; return false; }
    if (!(a >= -1 && a <= 4) || a != (double)(int)a) { why = r.action + ": axisItem 必須是 -1..4（rgAxis: No Use/InArm X/InArm Y/OutArm X/OutArm Y）"; return false; }
    if (!(t >= -1 && t <= 1) || t != (double)(int)t) { why = r.action + ": moveType 必須是 -1..1（rgMoveType: 去(單趟)/返(單趟)）"; return false; }
    axisItem = (int)a; moveType = (int)t;
    if (needPitch) {
        if (!ParamNum(r, "pitch", p) || !ParamNum(r, "delayMs", d)) { why = r.action + ": 缺 pitch／delayMs（golden atoi(edPitech／edDelayTime->Text)）"; return false; }
        if (!(p > -2147483649.0 && p < 2147483648.0) || !(d > -2147483649.0 && d < 2147483648.0)) { why = r.action + ": pitch／delayMs 超出 int"; return false; }
        pitch = (int)p; delayMs = (int)d;
    }
    return true;
}

// golden MOT[mi].MotorMove(p) for the Light Scale arm (1203: GoldenMove1203; another card: the golden object).
int LsMotorMove(IMotorAccessBackend& be, int mi, int target, std::string& note)
{
    const std::string alias = be.AliasOfMotor(mi);
    MotorAccessAxis a;
    MotorGolden g;
    if (alias.empty() || !be.Resolve(alias, a) || !be.GoldenMotor(mi, g)) { note = "MOT[" + std::to_string(mi) + "] has no Mot_Table row / object -> golden MotorMove -1"; return -1; }
    if (a.Is1203()) return GoldenMove1203(be, a, g, target, -1, note);
    if (!a.motorLive) { note = "Motor==NULL -> golden MotorMove -1"; return -1; }
    return be.GoldenMotorMove(mi, target, 0, false);
}

// golden LightScale case 0 (:1835-1841): fHome->iHomeStep=1; iSingleHomeIndex=iUseAxisItem;
//   InitProcessSingleMotorTask(iSingleHomeIndex); bSingleHome=true; MOT[iUseAxisItem].HomeFlag=0; -> the port's single home
//   is this file's HOME job (the drive-branched home of btnHome, StartHome1203), marked fromLS.
//  Returns false only to retry on the next tick (a 1203 sample older than this file's last command on the axis, W4B-5 --
//  transient, so case 0 is simply run again); every other outcome is final and case 1 follows.
bool LsStartHome(IMotorAccessBackend& be, int mi)
{
    const std::string alias = be.AliasOfMotor(mi);
    MotionCtx m;
    const bool prelude = !alias.empty() && MotionPreludeFor("lightScale home", alias, -1, be, m, false);
    if (prelude && m.a.Is1203() && m.a.axis >= 0 && !FindHome(m.a.axis) && SampleStale(be, m.a.axis)) {
        g_ls.lastNote = "lightScale home " + alias + ": " + kStaleWhy;
        return false;
    }
    g_ls.homePending = true;                                                    // golden bSingleHome=true
    be.GoldenSetHomeFlag(mi, 0);                                                // golden :1840
    const std::string stuck = " -- golden's single home would wait here too: the scan waits at case 1 until bSingleHome drops "
                              "(STOP / HOME released / FormShow / VerifyMotorAction's unlock / an alarm release it, like golden)";
    if (alias.empty()) { g_ls.lastNote = "Light Scale home: MOT[" + std::to_string(mi) + "] has no Mot_Table row" + stuck; return true; }
    if (!prelude) { g_ls.lastNote = m.why + stuck; return true; }
    if (!m.a.Is1203()) {
        g_ls.lastNote = "lightScale home " + alias + "（" + m.a.cardModel + "）: golden ProcessSingleMotorHome is a stub in this tree (acatchtray_shims.cpp:134)" + stuck;
        return true;
    }
    if (FindHome(m.a.axis)) { g_ls.lastNote = "lightScale home " + alias + ": a single home already runs on this axis (it goes on)"; return true; }
    HomeStartInfo info;
    const std::string refused = StartHome1203(alias, -1, be, m, "lightScale home " + alias, true, info);
    g_ls.lastNote = refused.empty() ? "lightScale home " + alias + ": " + (info.issued ? "sent " : "accepted, NOT issued (dry) ") + info.wouldCall
                                    : refused + stuck;
    return true;
}

// golden LightScale(false) (:1814-1977): one golden tick.
void LightScaleStep(IMotorAccessBackend& be)
{
    if (g_lsUseAxis == -1 || g_lsAxisItem == 0) return;                         // :1814-1815
    g_ls.useAxisItem = g_lsUseAxis;                                             // :1817
    const int ua = g_ls.useAxisItem;
    switch (g_ls.task) {
        case 0:
            if (LsStartHome(be, ua)) g_ls.task = 1;                             // (false: a stale sample -- case 0 again next tick)
            break;
        case 1:
            if (!g_ls.homePending) g_ls.task = 10;                              // :1844 if(bSingleHome==false)
            break;
        case 10: {
            g_ls.moveItem = g_lsMoveType;                                       // :1850 rgMoveType->ItemIndex
            MotorGolden g;
            if (!be.GoldenMotor(ua, g)) { g_ls.lastNote = "case 10: MOT[" + std::to_string(ua) + "].Motor is NULL (golden would dereference it)"; break; }
            if (g_ls.moveItem == 0) {                                           // :1851-1857 去(單趟)
                g_ls.needMovePos = g.softP - 100;
                g_ls.movePitch   = g_ls.pitch;
                g_ls.needMoveLim = g.softN + 100;
                g_ls.task = 150;
            } else if (g_ls.moveItem == 1) {                                    // :1858-1864 返(單趟)
                g_ls.needMovePos = g.softN + 100;
                g_ls.movePitch   = -g_ls.pitch;
                g_ls.needMoveLim = g.softP - 100;
                g_ls.task = 150;
            }                                                                   // -1: stays at 10 (golden); 2 unreachable (see above)
            break;
        }
        case 150: {
            g_ls.position = g_ls.needMovePos;                                   // :1876
            std::string note;
            const int ret = LsMotorMove(be, g_lsUseAxis, g_ls.position, note);  // :1878 MOT[iUseAxis].MotorMove(iPosition)
            if (!note.empty()) g_ls.lastNote = "case 150 target " + std::to_string(g_ls.position) + ": " + note;
            if (ret != 0) {                                                     // golden `if(...)`: -1/-2/-3 are "arrived" too
                g_ls.delayUntil = LoopNow(be) + g_ls.delayMs;                   // :1880 DelayTime.SetMSAndOn(iDelayTime)
                g_ls.delayOver = false;
                g_ls.task = 160;
            }
            break;
        }
        case 160: {
            if (LoopNow(be) < g_ls.delayUntil) break;                           // :1885 DelayTime.Off()
            if (!g_ls.delayOver) { g_ls.delayOver = true; g_ls.delayPoll = be.Pci1203PollCount(); }
            std::string src;
            bool fromMonitor = false;
            const int enc = be.GoldenLightScaleEncoder(src, fromMonitor);
            if (fromMonitor && be.Pci1203PollCount() <= g_ls.delayPoll) break; // port: a sample taken after the delay
            g_ls.encoderSrc = src;
            g_ls.lightScalePos = enc * (-1);                                    // :1887 (MOT[MLightScale].ReadEncoderPos())*(-1)
            g_ls.task = 170;
            break;
        }
        case 170:
            g_ls.memo.push_back(MotorLightScaleLine(g_ls.needMovePos, g_ls.lightScalePos));   // :1892-1893 Memo1->Lines->Add(S1)
            g_ls.needMovePos -= g_ls.movePitch;                                 // :1894
            g_ls.task = 180;
            break;
        case 180:
            if (g_ls.moveItem == 0 && g_ls.needMovePos > g_ls.needMoveLim)      g_ls.task = 150;   // :1898-1901
            else if (g_ls.moveItem == 1 && g_ls.needMoveLim > g_ls.needMovePos) g_ls.task = 150;   // :1902-1905
            else {                                                              // :1906-1912
                g_ls.editsEnabled = true;
                g_ls.timer = false;
                g_ls.stopFlag = true;
                g_ls.lastNote = "scan done: " + std::to_string(g_ls.memo.size()) + " line(s) in Memo1";
            }
            break;
        default:
            break;                                                              // -1 (never reset) / 100 / 200-240: see above
    }
}
// golden Timer2Timer (:2031-2037) = one tick; then as many more as need no wait (see the banner).
void LightScaleTick(IMotorAccessBackend& be)
{
    for (int n = 0; n < 16 && g_ls.timer; ++n) {
        const int before = g_ls.task;
        g_ls.timer = false;                                                     // :2033
        LightScaleStep(be);                                                     // :2034
        if (g_ls.stopFlag == false) g_ls.timer = true;                          // :2035-2036
        if (g_ls.task == before) break;                                         // waiting: the next Poll / beat asks again
    }
}

// golden LightScale(true) (:1819-1831); false = its guard returned first (:1814-1815, nothing reset).
bool LightScaleReset(IMotorAccessBackend& be, int pitch, int delayMs, std::string& why)
{
    if (g_lsUseAxis == -1 || g_lsAxisItem == 0) return false;
    MotorGolden g;
    if (!be.GoldenMotor(g_lsUseAxis, g)) { why = "MOT[" + std::to_string(g_lsUseAxis) + "].Motor is NULL -- golden would dereference it at :1823"; return false; }
    g_ls.useAxisItem  = g_lsUseAxis;
    g_ls.pitch        = pitch;                                                  // :1821 atoi(edPitech->Text)
    g_ls.delayMs      = delayMs;                                                // :1822 atoi(edDelayTime->Text)
    g_ls.minLim       = g.softN;                                                // :1823
    g_ls.maxLim       = g.softP;                                                // :1824
    g_ls.editsEnabled = false;                                                  // :1825-1826
    g_ls.needMovePos  = 0;                                                      // :1827
    g_ls.task         = 0;                                                      // :1828
    g_ls.stopFlag     = false;                                                  // :1829
    return true;
}

// ---- lightScale：BitBtn2Click :2096-2100 `LightScale(true); Timer2->Enabled=!Timer2->Enabled;` ----
MotorAccessOutcome DoLightScale(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    int axisItem = -1, moveType = -1, pitch = 0, delayMs = 0;
    std::string why;
    if (!LsRadiosFrom(r, true, axisItem, moveType, pitch, delayMs, why)) return Refuse(why);
    LsApplyRadios(axisItem, moveType);
    std::string nullWhy;
    const bool reset = LightScaleReset(be, pitch, delayMs, nullWhy);
    if (!nullWhy.empty()) return Refuse(r.action + ": " + nullWhy);
    g_ls.timer = !g_ls.timer;                                                   // :2099
    const std::string alias = (g_lsUseAxis >= 0) ? be.AliasOfMotor(g_lsUseAxis) : std::string();
    char b[320];
    std::snprintf(b, sizeof(b), "golden BitBtn2Click: LightScale(true) %s; Timer2 %s. iUseAxis=%d (%s), rgAxis=%d, rgMoveType=%d, pitch=%d, delay=%d ms",
                  reset ? "reset (Task 0: home first, then the scan)" : "returned at its guard (iUseAxis==-1 or rgAxis No Use) -- nothing reset",
                  g_ls.timer ? "ON" : "OFF (the axis in flight is not stopped -- golden)", g_lsUseAxis, alias.empty() ? "-" : alias.c_str(),
                  g_lsAxisItem, g_lsMoveType, pitch, delayMs);
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, g_ls.timer ? "lightScaleOn" : "lightScaleOff", "MOT+pci1203", b);
    w.Key("active").Bool(g_ls.timer);
    w.Key("reset").Bool(reset);
    w.Key("task").Number((wb_int64)g_ls.task);
    w.Key("useAxis").Number((wb_int64)g_lsUseAxis);
    w.Key("useMotor"); if (alias.empty()) w.Null(); else w.String(alias);
    w.Key("editsEnabled").Bool(g_ls.editsEnabled);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// golden MyForceDirectories / DirectoryExists for the two save buttons (the only files this file writes).
bool LsDirExists(const std::string& p)
{
    struct _stat st;
    return _stat(p.c_str(), &st) == 0 && (st.st_mode & _S_IFDIR) != 0;
}
bool LsForceDir(const std::string& p)
{
    if (p.empty() || LsDirExists(p)) return !p.empty();
    const std::size_t k = p.find_last_of("\\/");
    if (k != std::string::npos && k > 2) LsForceDir(p.substr(0, k));
    _mkdir(p.c_str());
    return LsDirExists(p);
}
bool LsWriteLines(const std::string& path, const std::vector<std::string>& lines, std::string& why)   // golden Lines->SaveToFile
{
    const std::string text = MotorStringsFileText(lines);
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) { why = "cannot create " + path; return false; }
    const bool ok = std::fwrite(text.data(), 1, text.size(), f) == text.size();
    const bool closed = std::fclose(f) == 0;
    if (!ok || !closed) { why = "write failed: " + path; return false; }
    return true;
}

// ---- lightScaleSave：BitBtn3Click :2102-2119 ＋ SaveAsCSV :1749-1787 ----
MotorAccessOutcome DoLightScaleSave(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    int axisItem = -1, moveType = -1, pitch = 0, delayMs = 0;
    std::string why;
    if (!LsRadiosFrom(r, false, axisItem, moveType, pitch, delayMs, why)) return Refuse(why);
    LsApplyRadios(axisItem, moveType);
    if (moveType == -1) return Refuse("往返動作選擇異常! (golden BitBtn3Click :2104-2107 ShowMessage)");
    if (axisItem == -1 || axisItem == 0) return Refuse("量測軸選擇異常! (golden BitBtn3Click :2109-2113 ShowMessage)");
    const std::string root = be.LightScaleRoot();                               // golden :1752 "D:\\LightScale"
    if (!LsForceDir(root))                                                      // :1753-1759
        return Refuse("lightScaleSave: " + root + " could not be created -- golden SaveAsCSV returns silently here and BitBtn3 still "
                      "shows \"Save successfully!\" (golden defect); here the page is told nothing was saved");
    static const int kN[2][5] = { { 0, 1, 3, 5, 7 }, { 0, 2, 4, 6, 8 } };       // :1761-1782 Motor1/3/5/7 (去), 2/4/6/8 (返)
    const std::string path = root + "\\Motor" + std::to_string(kN[moveType][axisItem]) + "Positive.csv";   // :1784 tmp+".csv"
    const std::size_t n = g_ls.memo.size();
    if (!LsWriteLines(path, g_ls.memo, why))                                    // :1785 SaveToFile (golden: an EFCreateError dialog, Memo1 kept)
        return Refuse("lightScaleSave: " + why + " -- Memo1 not cleared (golden: the SaveToFile exception skips Memo->Clear())");
    g_ls.memo.clear();                                                          // :1786 Memo->Clear()
    g_ls.lastSaved = path;
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "saved", "file", "Save successfully! Path:" + root);          // golden :2118
    w.Key("path").String(path);
    w.Key("lines").Number((wb_int64)n);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- lightScaleDataSave：btnSaveLogLightScaleDataClick :2051-2094 ＋ SaveAsCSV_Kaichen :1982-2027 ----
MotorAccessOutcome DoLightScaleDataSave(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    const std::string root = be.LightScaleRoot();
    if (!LsForceDir(root)) return Refuse("lightScaleDataSave: " + root + " could not be created (golden :2057-2063 returns silently)");
    const std::string dir = root + "\\LightScaleData_" + be.LocalStampYmdhm(); // :2065-2066 FormatDateTime("yyyymmddhhmm", Now())
    if (!LsForceDir(dir)) return Refuse("lightScaleDataSave: " + dir + " could not be created (golden :2067-2073 returns silently)");
    static const char* const kName[9] = { "", "InArmX_Motor1Positive", "InArmX_Motor2Positive", "InArmY_Motor3Positive",
                                          "InArmY_Motor4Positive", "OutArmX_Motor5Positive", "OutArmX_Motor6Positive",
                                          "OutArmY_Motor7Positive", "OutArmY_Motor8Positive" };   // :1988-2019
    std::vector<std::string> files;
    for (int k = 1; k <= 8; ++k) {                                              // :2075-2082 mmo1..mmo8
        const std::string path = dir + "\\" + kName[k] + ".csv";
        std::string why;
        if (!LsWriteLines(path, be.GoldenLightScaleData(k), why))
            return Refuse("lightScaleDataSave: " + why + " (golden: the SaveToFile exception stops here; mmo1..mmo" + std::to_string(k - 1) +
                          " were already saved and cleared, the counters are NOT reset)");
        be.GoldenLightScaleDataClear(k);                                        // :2026 Memo->Clear()
        files.push_back(path);
    }
    be.GoldenLightScaleCountsReset();                                           // :2084-2091
    g_ls.lastSaved = dir;
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "saved", "file", "Save successfully!\nPath:" + dir);          // golden :2093 ShowMessage
    w.Key("path").String(dir);
    w.Key("paths").BeginArray();
    for (std::size_t i = 0; i < files.size(); ++i) w.String(files[i]);
    w.EndArray();
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// =============================================================================
//  AI(W906-MERGE-56bbf785) 20260926: the machine's MT-E3c block above + the laptop's W5-b block below. The machine still had
//  base's IsTeachMotion() (it refused every uteach motion as "W5"); W5-b replaced it and its only call site, so it is gone.
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
    if (a.motIndex >= 0 && a.motorLive) be.GoldenSetSpeed(mi, pct, false);     // AI(W906-MERGE-56bbf785): golden SetSpeed(pct) = (pct, bSetJog=false)
}
// golden edtSpeed->Text（或 ScrollBar1->Position）改成 v：edtSpeedChange（uteach.cpp:2442）→ MOT[ActiveMotorIndex].SetSpeed(v)。
//   VCL 的 Text 沒變就不觸發 OnChange ⇒ 值一樣時什麼都不做。
//AI(W906-MERGE-56bbf785) 20260926: second review -- golden ScrollBar1Change (uteach.cpp:1318, RogerYang 20250729) is
//  `MOT[ActiveMotorIndex].SetSpeed(Speed/3, true)`: every change of the teach page's speed (the bar itself, edtSpeedChange's
//  `ScrollBar1->Position=Speed` :2452, UpdateMotorTeach(Two)Monitor's `Position=1` :1283/:1301 -- VCL fires OnChange on a
//  programmatic Position change) also writes the JOG family, at a third of the value; TMyEtherCatMotor::SetSpeed clamps below 1%
//  to 1% (myEthercatmotor.cpp:631-632). DoTeachJog re-sends g_jogSpd, so recording it here is enough (no card write now).
void TeachJogFamily(IMotorAccessBackend& be, int mi, int v)
{
    if (mi < 0) return;
    std::string alias;
    MotorAccessAxis a;
    if (!be.AliasOfMotIndex(mi, alias) || !be.Resolve(alias, a)) return;
    const int jp = (v / 3 < 1) ? 1 : v / 3;
    if (a.Is1203()) {
        MotorGolden g;
        if (!be.GoldenMotor(mi, g)) return;
        g_jogPct[mi] = jp;
        g_jogSpd[mi] = MotorSpeedFromPct(jp, g);
        return;
    }
    if (a.motIndex >= 0 && a.motorLive) be.GoldenSetSpeed(mi, jp, true);
}
void TeachEdtSpeed(IMotorAccessBackend& be, int ami, int v)
{
    if (g_teachEdt == v) return;
    g_teachEdt = v;
    TeachSpeedSet(be, ami, v);
    TeachJogFamily(be, ami, v);                                                 //AI(W906-MERGE-56bbf785) 20260926: golden ScrollBar1Change's SetSpeed(Speed/3, true)
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
//   AI(W906-MERGE-56bbf785) 20260926: 1203 軸的 jog 已不用它（DoTeachJog 改用 DoJog 的來源）；現在只給 MoveP／MoveN／MoveTo（Move1203）。
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
//   AI(W906-MERGE-56bbf785) 20260926: 1203 軸改成 DoJog 的速度來源（g_jogSpd／JogPctOf：上一次 SetSpeed(x,true) 寫進 jog 家族的值），
//     不再用 g_goldenPct（教導頁的 PTP 百分比）。理由與落差（golden ScrollBar1Change 的 SetSpeed(Speed/3,true) 沒有照做）在 1203 分支裡；EastSun 之後可能另裁。
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
    //AI(W906-MERGE-56bbf785) 20260926: laptop = Send1203Speed(PTP family, TeachCurPct) + kCmdAxMoveVel (EastSun's monitor-page jog, which
    //  was also the W4 Motor Test jog when the laptop forked). Machine MT-E1 = the golden TMyEtherCatMotor::JogP/JogN for every 1203 jog in
    //  this file (user EastSun 20260925 「開放，照舊版做法」): Acm_AxSetExtDrive(ax, 1) + Acm_AxJog, released by Stop1203 (StopDec + ExtDrive 0,
    //  DoJogRelease). The laptop's own rule for this page is "1203 軸的底層照 EastSun（同 W4）", so the teach jog takes the machine's golden
    //  jog; the SPEED stays the laptop's (R-W5B-4: this axis' current golden pct, TeachCurPct), written to the jog family CFG_AxJog* --
    //  what Acm_AxJog runs on -- right before the jog; the PTP family is not touched (the next teach move re-sends it, Move1203).
    //AI(W906-MERGE-56bbf785) 20260926: review finding -- the jog SPEED is now DoJog's source, not TeachCurPct recomputed at each press.
    //  golden TMyEtherCatMotor::JogP/JogN ignore their Speed argument and run on the card's jog family CFG_AxJog*, which keeps the
    //  values of the last SetSpeed(x, bSetJog=true) (golden Motor/myEthercatmotor.cpp:698-729). The port tracks that per motor in
    //  g_jogSpd (the exact values written then) / JogPctOf (1% when nothing was written since the selection) -- what DoJog re-sends.
    //  The teach page's edtSpeedChange / UpdateMotorTeachMonitor / HOME release / Go buttons (TeachSpeedSet) are SetSpeed(v) with
    //  bSetJog=false: PTP family only. The old code sent MotorSpeedFromPct(TeachCurPct) -- the teach page's PTP pct, recomputed from
    //  today's parameters at every press -- to the jog family.
    //  Second review (same day): golden uteach.cpp:1318 ScrollBar1Change is `SetSpeed(Speed/3, true)` (RogerYang 20250729), so the teach
    //    page's own speed changes DO write the jog family, at a third of the value (min 1%) -- now mirrored by TeachJogFamily (called from
    //    TeachEdtSpeed / TeachSpeedEvent on a change), which updates g_jogPct / g_jogSpd; this jog then re-sends them.
    const int pct = JogPctOf(m.a.motIndex);                                     // DoJog 的速度來源：Motor Test 捲軸上一次的位置（沒動過＝1%）
    std::map<int, Motor1203Speed>::const_iterator jsp = g_jogSpd.find(m.a.motIndex);
    const Motor1203Speed sp = (jsp != g_jogSpd.end()) ? jsp->second : MotorSpeedFromPct(pct, m.g);
    if (!Send1203SpeedValues(be, wireId, m, pct, sp, why, note, kFamJog)) return Refuse(who + ": " + why);
    {
        Pci1203Cmd x;
        x.kind = kCmdAxSetExtDrive; x.wireId = wireId; x.axis = m.a.axis; x.value = 1.0;
        const Pci1203CmdResult xr = be.Pci1203Execute(x);
        if (CmdFailed(xr)) return Refuse(who + ": 1203 進入 jog 模式（Acm_AxSetExtDrive 1）失敗 —— " + CmdWhy(xr));
    }
    Pci1203Cmd c; c.kind = kCmdAxJogStart; c.wireId = wireId; c.axis = m.a.axis; c.dir = positive ? 1 : -1;   // ruling 6B: JogP = wire +1 (DIRECTION_POS)
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) { Stop1203(be, m.a.axis, wireId); return Refuse(who + ": 1203 失敗 —— " + CmdWhy(res)); }   // leave jog mode again (as DoJog)
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
        if (!a.Is1203()) { if (a.motorLive) be.GoldenSetSpeed(a.motIndex, 1, false); }   // AI(W906-MERGE-56bbf785): golden SetSpeed(1)
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
    //AI(W906-MERGE-56bbf785) 20260926: review finding (manual-teach guard bypass) -- EVERY motor of the request, not only motors[0]:
    //  an All-mode loop carries the checked rows in motors, and the taught axis may be any of them (it would then be driven with
    //  its servo off). An alias that does not resolve is skipped here (the handler refuses it later, as before).
    for (std::size_t k = 0; k < r.motors.size(); ++k) {
        MotorAccessAxis a;
        if (!be.Resolve(r.motors[k], a)) continue;
        for (std::size_t i = 0; i < g_teachSet.m.size(); ++i) {
            const MotorAccessAxis& t = g_teachSet.m[i].a;
            if ((a.motIndex >= 0 && a.motIndex == t.motIndex) || (a.Is1203() && t.Is1203() && a.axis >= 0 && a.axis == t.axis))
                return "motor.access " + r.action + "（" + r.source + "）" + r.motors[k] + ": 教導頁手動教導中（" + g_teachSet.btn +
                       "，這一軸伺服是關的）—— golden fTeachShow 是 ShowModal，開著時 MotorTest 按不到；先在教導頁按確定或取消";
        }
    }
    return std::string();
}

//AI(W906-MERGE-56bbf785) 20260926: review finding (manual-teach guard bypass) -- the Motor Test actions that reach the taught axis
//  WITHOUT naming it in motors, so TeachAxisBusyWhy cannot see them. golden fTeachShow is ShowModal: while it is open nothing on
//  Motor Test can be pressed; the port keeps W5B-5's "other axes still allowed" for the per-axis buttons, but refuses these:
//    reloadMotorData  -- re-applies the table values to every motor, the taught one included;
//    lightScale       -- only when it would START a scan (it homes and moves the arm by itself); turning a running scan off passes;
//    motorPowerToggle -- relay / ServerON / GaliMotorServoOff act on every motor (power ON = servo power under the operator's hand);
//    formShow         -- golden FormShow syncs Motor Power the same way (DoMotorPowerOn or GaliMotorServoOff) and raises HOME / Loop;
//    loopMove, mode!=0 (All) -- moves every checked motor; only a start (a release, start=false, is a stop and passes).
//  Same wording as the per-axis refusal ("教導頁手動教導中").
std::string TeachPageWideBusyWhy(const MotorAccessReq& r)
{
    if (!g_teachSet.active || r.source != "uMotorTest") return std::string();
    const char* what = 0;
    if (r.action == "reloadMotorData")       what = "Reload Motor Data 會把表值套回所有馬達，含教導軸";
    else if (r.action == "lightScale")       { if (!g_ls.timer) what = "Light Scale 開始掃描會自己歸零、移動手臂"; }
    else if (r.action == "motorPowerToggle") what = "Motor Power 會切所有馬達的電源／伺服";
    else if (r.action == "formShow")         what = "FormShow 會照繼電器同步 Motor Power（DoMotorPowerOn 或 GaliMotorServoOff）";
    else if (r.action == "loopMove") {
        std::map<std::string, double>::const_iterator md = r.num.find("mode");
        std::map<std::string, bool>::const_iterator sf = r.flag.find("start");
        if (md != r.num.end() && md->second != 0.0 && !(sf != r.flag.end() && !sf->second))
            what = "All 模式的來回會動所有勾選的軸";
    }
    if (what == 0) return std::string();
    return "motor.access " + r.action + "（" + r.source + "）: 教導頁手動教導中（" + g_teachSet.btn + "，教導軸伺服是關的；" + what +
           "）—— golden fTeachShow 是 ShowModal，開著時 MotorTest 按不到；先在教導頁按確定或取消";
}

//AI(W906-MERGE-56bbf785) 20260926: review finding (run gate blocks STOP-direction requests) -- the Motor Test requests that only
//  stop something: lightScale while Timer2 is on (golden BitBtn2Click toggles it off; LightScale(true) only resets the task), and
//  loopMove / home released (start=false: golden's else branches, PCIL132_StopMotor / bSingleHome=false). Like STOP they pass the
//  SystemStart gate and the manual-teach guard (a release names its axis, which may be the taught one; stopping it is harmless).
bool MotorTestStopDirection(const MotorAccessReq& r)
{
    if (r.source != "uMotorTest") return false;
    if (r.action == "lightScale") return g_ls.timer;
    if (r.action == "loopMove" || r.action == "home") {
        std::map<std::string, bool>::const_iterator sf = r.flag.find("start");
        if (sf == r.flag.end() || sf->second) return false;
        //AI(W906-MERGE-56bbf785) 20260926: second review -- a release passes only when there IS something of Motor Test's to stop.
        //  With no job, DoHome's release still sends golden's unconditional Stop1203 to that axis, so while running (or while a
        //  hand-teach is open) it could stop an axis the engine / the operator is using -- a path golden never had (the page cannot
        //  be opened while SystemStart, fTeachShow is modal). No job => not a stop-direction request => refused by the gates as before.
        if (r.action == "loopMove") return g_loop.active;
        if (r.motors.empty()) return false;
        for (std::size_t i = 0; i < g_homes.size(); ++i)
            if (g_homes[i].active && g_homes[i].motorId == r.motors[0]) return true;
        return false;
    }
    return false;
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
    const bool changed = (g_teachEdt != s);                                     //AI(W906-MERGE-56bbf785) 20260926: golden OnChange fires only on a change
    g_teachEdt = s;
    TeachSpeedSet(be, ami, s);
    if (changed) TeachJogFamily(be, ami, s);                                    //AI(W906-MERGE-56bbf785) 20260926: golden ScrollBar1Change's SetSpeed(Speed/3, true)
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
    g_beatClockMs += elapsedMs;                                                 //AI(W906-MT-E2) 20260925: LoopNow() when the backend has no clock
    // AI(W906-W5-b) 20260925: 覆核 W5B-R5／R6 —— 手動教導不在這裡結束（golden 的對話框不會自己關；見 DoTeachJog 前的說明）
    if (!operatorConnected && !g_jogs.empty()) {                                // NB2 R21 W4B-4：jog 的死人開關
        for (std::size_t i = 0; i < g_jogs.size(); ++i) {
            if (g_jogs[i].is1203) Stop1203(be, g_jogs[i].axis, -1);            // AI(W906-MT-E1): dead-man stop also leaves jog mode
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
        else if ((g_lastSafeLock = be.GoldenSafeLockActive()))                //AI(W906-MT-FIX1) 20260926: remembered for the post-Poll step
            CancelAllJobs(be, "safe lock active (golden Timer1Timer: IsSafeLockCheck() -> Close())");
    } else {
        g_lastSafeLock = false;
    }
    TickHomes(be, elapsedMs);
    TickLoop(be);                                                               //AI(W906-MT-E2) 20260925: times from LoopNow(), not elapsedMs
    TickPowerOn(be);                                                            //AI(W906-MT-E3c) 20260925: Motor Power On's 1 s (cross-tick DoMotorPowerOn)
    LightScaleTick(be);                                                         //AI(W906-MT-E3c) 20260925: golden Timer2 (not gated by the operator / page -- golden Timer2Timer has no fShow test)
}

//AI(W906-MT-E2) 20260925: see WebMotorAccess.h. Only a 1203 loop needs the fresh sample; the golden-object loop (non-1203)
//  asks MOT.MotorMove itself and stays on the beat, like everything else.
void MotorAccessPollTick(IMotorAccessBackend& be, bool operatorConnected)
{
    //AI(W906-MT-FIX1) 20260926: the same gate as the beat (MotorAccessTick), BEFORE any step -- otherwise a LoopMove leg could be
    //  issued up to 500 ms after the operator's connection is gone or the safe lock tripped (the beat would only cancel it after
    //  the leg was already on its way). The safe lock is the last beat's result: IsSafeLockCheck() switches an output, so it is
    //  not called a second time here.
    if (AnyJobActive()) {
        if (!operatorConnected)
            CancelAllJobs(be, "operator connection gone (seen after a Poll; golden Timer1Timer: fShow==false -> no further steps)");
        else if (g_lastSafeLock)
            CancelAllJobs(be, "safe lock active (last beat; golden Timer1Timer: IsSafeLockCheck() -> Close())");
    }
    bool loop1203 = g_loop.active && g_loop.a.Is1203();
    //AI(W906-MT-E3c) 20260925: an All-mode loop with any 1203 axis, Light Scale and the power-on job step here too (a 1203
    //  arrival is seen on the fresh sample; golden Timer1 is 5 ms, Timer2 10 ms).
    for (std::size_t k = 0; !loop1203 && g_loop.active && g_loop.all && k < g_loop.axes.size(); ++k) loop1203 = g_loop.axes[k].a.Is1203();
    if (loop1203) TickLoop(be);
    TickPowerOn(be);
    LightScaleTick(be);
}

// ---- AI(W906-MT-E3c) 20260925: the engine's VerifyMotorAction hooks (WebMotorAccess.h) ----
int MotorAccessMovingOf(IMotorAccessBackend& be, int motIndex)
{
    const std::string alias = be.AliasOfMotor(motIndex);
    if (alias.empty()) return -1;
    MotorAccessAxis a;
    if (!be.Resolve(alias, a) || !a.Is1203()) return -1;                        // not a 1203 row: golden MotionDone decides
    if (a.axis < 0) return 0;                                                   // the monitor did not open it: no command of this tree reaches it
    for (std::size_t i = 0; i < g_jogs.size(); ++i) if (g_jogs[i].is1203 && g_jogs[i].axis == a.axis) return 1;
    if (SampleStale(be, a.axis)) return 1;                                      // a command went out, no fresh sample yet
    unsigned st = 0;
    if (!be.Pci1203AxisState(a.axis, st)) return 0;                             // no valid sample: unknown -> not locking on a monitor hiccup
    return IsReadyState(st) ? 0 : 1;                                            // golden TMyEtherCatMotor::MotionDone: state==STA_AX_READY
}
bool MotorAccessHomingActive()
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (g_homes[i].active && !g_homes[i].fromLS) return true;
    return false;
}
void MotorAccessAllBtnUp(IMotorAccessBackend& be, const std::string& why)
{
    CancelAllJobs(be, why + " (golden fMotorTest->AllBtnUp)");                  // the axes were just stopped by the engine (StopAllMotor + Stop1203All)
    EndSingleHome(be, why + " (golden fMotorTest->bSingleHome=false)");
    g_lastJobNote = "AllBtnUp: " + why + (g_ls.timer ? " -- Light Scale keeps running (golden AllBtnUp does not touch Timer2)" : "");
}
void MotorAccessStop1203All(IMotorAccessBackend& be, const std::string& why)
{
    const int n = StopAllOpened1203(be, -1);
    g_lastJobNote = "Stop1203 on " + std::to_string(n) + " axis(es): " + why;
}

MotorLightScaleState MotorAccessLightScale(std::size_t tail)
{
    MotorLightScaleState s;
    s.active = g_ls.timer; s.task = g_ls.task; s.editsEnabled = g_ls.editsEnabled; s.homePending = g_ls.homePending;
    s.useAxis = g_lsUseAxis; s.axisItem = g_lsAxisItem; s.moveType = g_lsMoveType; s.needMovePos = g_ls.needMovePos;
    s.memoCount = (unsigned long)g_ls.memo.size();
    const std::size_t from = (g_ls.memo.size() > tail) ? g_ls.memo.size() - tail : 0;
    s.memoTail.assign(g_ls.memo.begin() + (std::ptrdiff_t)from, g_ls.memo.end());
    s.lastSaved = g_ls.lastSaved; s.lastNote = g_ls.lastNote; s.encoderSrc = g_ls.encoderSrc;
    return s;
}

MotorGoldenRate MotorRateFromGolden(unsigned a, unsigned jogHigh, unsigned initSpeed, unsigned iSpeed, unsigned range)
{
    MotorGoldenRate g;
    g.dAcc = 0.0; g.skip = false; g.rate = 0.0; g.accPersent = g.accMin = g.accMax = 0u;
    double Rate = a;                                                            // :582 Rate=a;
    g.dAcc = (double)(unsigned)(jogHigh - initSpeed) * Rate / 8000000.;         // :583 (unsigned int arithmetic, then double)
    if (g.dAcc == 0) { g.skip = true; return g; }                               // :584-585
    Rate = (double)(unsigned)(iSpeed * range - initSpeed * range) / g.dAcc;     // :587 (unsigned 32-bit, may wrap -- golden)
    unsigned iAccPersent = (iSpeed * range) / 65535u;                           // :591
    if (iAccPersent == 0) iAccPersent = 1;                                      // :592-593
    g.accPersent = iAccPersent;
    g.accMin = iAccPersent * 2001u;                                             // :595
    g.accMax = iAccPersent * 8192000u;                                          // :596 (wraps above 524 -- golden)
    if (Rate < g.accMin) Rate = g.accMin;                                       // :598-599
    if (Rate > g.accMax) Rate = g.accMax;                                       // :600-601
    g.rate = Rate;
    return g;
}
std::string MotorLightScaleLine(int needMovePos, int lightScalePos)
{
    char b[128];
    std::snprintf(b, sizeof(b), "ArmPosition, %d, LightScalePos, %d ,[ %d ]", needMovePos, lightScalePos, needMovePos - lightScalePos);
    return b;
}
std::string MotorStringsFileText(const std::vector<std::string>& lines)
{
    std::string t;
    for (std::size_t i = 0; i < lines.size(); ++i) { t += lines[i]; t += "\r\n"; }
    return t;
}

namespace {
//AI(W906-MT-E2) 20260925: golden UpdateMotorLed (uMotorTest.cpp:644-652), run by Timer1Timer for the SELECTED motor:
//      if(MOT[ActiveIndex].Led[iAlarmLed] && fNote->fShow)
//      { MOT[ActiveIndex].PCIL132_StopMotor(); MOT[ActiveIndex].HomeFlag=0; btnLoopMove->Down=false; btnHome->Down=false; }
//  The port's fNote->fShow: an alarm is up exactly from ForwardShowErrorMessage (wb_serve), which calls
//  MotorAccessOnAlarm first and, for a blocking alarm, then holds the tick thread in its modal wait (no beat, no Poll)
//  until it is answered -- so the moment it is raised is the only moment this thread can look, and it looks here.
//  (golden looks every 5 ms while the note is up, so an alarm lamp that comes on only AFTER the note appeared is missed
//  here; a kcode==0 note does not block and is not remembered as "shown".) Alarm lamp as golden decodes it:
//  1203 = ALM (motionIO bit 1) or STA_AX_ERROR_STOP (TMyEtherCatMotor::ScanMotorStatus, myEthercatmotor.cpp:1179/:1188);
//  other cards = MOT.Led[iAlarmLed] after ScanMotorStatus. Returns true when it acted.
bool AlarmRuleSelected(IMotorAccessBackend& be)
{
    if (g_selMotor.empty()) return false;
    MotorAccessAxis a;
    if (!be.Resolve(g_selMotor, a) || a.motIndex < 0) return false;
    bool alarm = false;
    if (a.Is1203()) {
        unsigned long io = 0;
        unsigned st = 0;
        const bool hasIo = a.axis >= 0 && be.Pci1203MotionIO(a.axis, io);
        const bool hasSt = a.axis >= 0 && be.Pci1203AxisState(a.axis, st);
        alarm = (hasIo && (io & 0x00000002ul) != 0) || (hasSt && IsErrorState(st));
    } else if (a.motorLive) {
        bool known = false;
        alarm = be.GoldenAlarmLed(a.motIndex, known) && known;
    }
    if (!alarm) return false;
    if (!a.Is1203()) be.GoldenStopMotor(a.motIndex);                           // golden PCIL132_StopMotor (1203 axes: all stopped by the caller, DecStop)
    be.GoldenSetHomeFlag(a.motIndex, 0);                                        // golden MOT[ActiveIndex].HomeFlag=0
    return true;
}
}  // namespace

void MotorAccessOnAlarm(IMotorAccessBackend& be, const std::string& what)
{
    const bool had = AnyJobActive() || !g_jogs.empty();
    CancelAllJobs(be, "alarm " + what + " (golden Timer1Timer: fNote->fShow -> btnLoopMove/btnHome up)");
    //AI(W906-MT-E3c) 20260925: the same golden block's `bSingleHome=false;` (uMotorTest.cpp:925) -- Timer1Timer runs it only
    //  while Motor Test is shown (:914 `if(fShow==false) return;`). The Light Scale timer is not touched (golden).
    if (g_mtShown) EndSingleHome(be, "alarm " + what + " (golden Timer1Timer :925 bSingleHome=false)");
    g_jogs.clear();
    // AI(W906-W5-b) 20260925: 覆核 W5B-R5 —— 手動教導不動：golden 在 fTeachShow 開著時出告警只停機（note.cpp:795 StopAllMotor），伺服保持關，
    //   對話框還在；伺服只在操作員按確定／取消後開。上一版在這裡自動開伺服（含 kcode==0 的通知），拿掉。
    std::string why;
    int n = 0;
    if (be.Pci1203Ready(why)) {
        for (int ax = 0; ax < be.Pci1203AxisCount(); ++ax) {                    // golden note.cpp:795 StopAllMotor＝全部馬達（見 DoStop 的說明）
            if (!be.Pci1203AxisOpened(ax)) continue;
            Stop1203(be, ax, -1);                                               // AI(W906-MT-E1): alarm stop = golden DecStop (StopDec + ExtDrive 0)
            ++n;
        }
    }
    const bool sel = AlarmRuleSelected(be);                                     //AI(W906-MT-E2) 20260925: after the stops -- was: returned first when the 1203 control was missing
    if (had || n || sel)
        g_lastJobNote = "alarm " + what + ": jobs cancelled, 1203 stop sent to " + std::to_string(n) + " axis(es)" +
                        (sel ? "; selected motor " + g_selMotor + " has its alarm lamp on -> HomeFlag=0 (golden UpdateMotorLed :644-652)" : "");
}

MotorAccessJobState MotorAccessJobs()
{
    MotorAccessJobState j;
    j.homesActive = 0;
    for (std::size_t i = 0; i < g_homes.size(); ++i)
        if (g_homes[i].active) { ++j.homesActive; j.homeMotors.push_back(g_homes[i].motorId); }   // AI(W906-MT-E1) 20260925
    j.loopActive = g_loop.active;
    j.loopMotor  = g_loop.motorId;
    j.loopTask   = g_loop.task;
    j.loopCount  = g_loop.count;
    j.lastNote   = g_lastJobNote;
    j.jogsActive = (int)g_jogs.size();
    j.selectedMotor = g_selMotor;                                               //AI(W906-MT-E2) 20260925
    j.hasJogPTime = g_hasJogP;  j.jogPTime = g_jogPTime;
    j.hasJogNTime = g_hasJogN;  j.jogNTime = g_jogNTime;
    j.hasAvgTime  = g_hasAvg;   j.avgTime  = g_avgTime;
    //AI(W906-MT-E3c) 20260925
    j.loopAll = g_loop.active && g_loop.all;
    if (g_loop.active) {
        if (g_loop.all) for (std::size_t k = 0; k < g_loop.axes.size(); ++k) j.loopMotors.push_back(g_loop.axes[k].motorId);
        else j.loopMotors.push_back(g_loop.motorId);
    }
    j.pageShown    = g_mtShown;
    j.powerPending = g_powerPending;
    j.singleHome   = AnyHomeActive();
    j.teachActive = g_teachSet.active;                                          // AI(W906-W5-b) 20260925 (AI(W906-MERGE-56bbf785): machine + laptop fields, both kept)
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
    g_lastSafeLock = false;                                      //AI(W906-MT-FIX1) 20260926
    g_jogSpd.clear();                                            //AI(W906-MT-FIX1) 20260926
    g_selMotor.clear(); g_jogPct.clear();                        //AI(W906-MT-E2) 20260925
    g_latchArmed = false; g_latchMs = 0.0; g_loopAverage = 0;
    g_hasJogP = g_hasJogN = g_hasAvg = false; g_jogPTime = g_jogNTime = 0; g_avgTime = 0.0;
    g_beatClockMs = 0.0;
    g_ls = LightScaleJob(); g_lsUseAxis = 0; g_lsAxisItem = -1; g_lsMoveType = -1;   //AI(W906-MT-E3c) 20260925
    g_powerPending = false; g_powerFrom.clear(); g_ptpPct.clear(); g_mtShown = false; g_gm.clear();
    g_teachSet = TeachSetJob();                                  // AI(W906-MERGE-56bbf785): + the laptop's W5-b state
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
    //AI(W906-MERGE-56bbf785) 20260926: only while SystemStart==false. golden sbTeachingClick returns on SystemStart (main.cpp:27827)
    //  BEFORE the teach page's FormShow can clear fAllMotorHome, so golden never clears it during a run. Here it matters because the
    //  machine's Motor Test page re-sends formShow when refused (every 1.5-10 s, MT-E3 web) and the SystemStart gate below refuses it:
    //  clearing first would make every such retry drop fAllMotorHome under a running machine (DoAllProcess then returns at its main
    //  guard every tick). Laptop behaviour with the machine idle is unchanged.
    if (!be.GoldenSystemStart()) be.GoldenClearAllMotorHome();
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
    //AI(W906-MERGE-56bbf785) 20260926: two machine actions pass this gate too --
    //  motorPowerToggle: EastSun ruling R9 20260925 "Motor Power must not be blocked" (DoMotorPower: no SystemStart / door guard);
    //    power OFF is GaliMotorServoOff = StopAllMotor + SystemStart=false, i.e. a stop.
    //  formClose: golden FormClose (fShow=false, PauseUT150Polling=false) moves nothing; refusing it would leave C++'s fShow record
    //    (torque focus, alarm rule) stuck "open" after the page is gone -- same reason the teachSet end passes.
    //AI(W906-MERGE-56bbf785) 20260926: review finding (run gate blocked STOP-direction requests) -- + MotorTestStopDirection: a
    //  lightScale that turns a running scan off, a loopMove / home release (start=false). Only uMotorTest; a lightScale that would
    //  START a scan (Timer2 off) and every start stay refused while running.
    const bool mtStopDir = MotorTestStopDirection(r);
    const bool machineNoBlock = ((r.action == "motorPowerToggle" || r.action == "formClose") && r.source == "uMotorTest") || mtStopDir;
    if (be.GoldenSystemStart() && !teachEndOrQuery && !machineNoBlock)
        return Refuse("motor.access " + r.action + "（" + r.source + "）: SystemStart==true（機台運轉中）—— golden 的教導頁／MotorTest 在運轉中進不去"
                      "（main.cpp:27827 sbTeachingClick `if(SystemStart) return;`），開著的頁面在運轉中一律不動；只有 STOP 放行"
                      "（以及 MotorTest 的 HOME／LoopMove 抬起、關掉進行中的 Light Scale）");   //AI(W906-MERGE-56bbf785) 20260926: the message names the stop-direction passes
    //AI(W906-W5-b) 20260925: W5B-5 —— 手動教導中（golden fTeachShow->ShowModal 開著）：教導頁除了 teachSet（結束／查詢）都擋；
    //  MotorTest 對同一軸的按鈕也擋（含伺服鈕）。停止在上面已經分派，永遠不擋。
    //  覆核 W5B-R1：教導頁的 teachSet 不再過「同軸」那一道（TeachAxisBusyWhy 只看 uMotorTest）。
    if (g_teachSet.active) {
        if (r.source == "uteach" && r.action != "teachSet")
            return Refuse("motor.access " + r.action + "（uteach）: 手動教導中（" + g_teachSet.btn + "）—— golden 的 fTeachShow 是對話框，開著時其他按鈕按不到；先按確定或取消");
        //AI(W906-MERGE-56bbf785) 20260926: review finding (manual-teach guard bypass) -- a STOP-direction Motor Test request passes like
        //  STOP (above); otherwise every motor of the request (TeachAxisBusyWhy) and the page-wide actions (TeachPageWideBusyWhy) are checked.
        if (!mtStopDir) {
            const std::string busy = TeachAxisBusyWhy(r, be);
            if (!busy.empty()) return Refuse(busy);
            const std::string wide = TeachPageWideBusyWhy(r);
            if (!wide.empty()) return Refuse(wide);
        }
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
    if (r.action == "reloadMotorData")              return DoReloadMotorData(r, wireId, be);   // AI(W906-MT-E1) 20260925
    if (r.action == "setRangeAndInit" || r.action == "setRateAndInit") return DoRangeRate(r, wireId, be);   //AI(W906-MT-E3c): + wireId (InitMotor goes to the card)
    //AI(W906-MT-E3c) 20260925: Motor Power, FormShow / FormClose, Light Scale -- golden uMotorTest handlers only.
    if (r.action == "motorPowerToggle" || r.action == "formShow" || r.action == "formClose" ||
        r.action == "lightScale" || r.action == "lightScaleSave" || r.action == "lightScaleDataSave") {
        if (r.source != "uMotorTest")
            return Refuse("motor.access " + r.action + "（" + r.source + "）: 這是 golden uMotorTest 的動作（" + row->golden + "），教導頁沒有");
        if (r.action == "motorPowerToggle")   return DoMotorPower(r, wireId, be);
        if (r.action == "formShow")           return DoFormShow(r, wireId, be);
        if (r.action == "formClose")          return DoFormClose(r, be);
        if (r.action == "lightScale")         return DoLightScale(r, be);
        if (r.action == "lightScaleSave")     return DoLightScaleSave(r, be);
        return DoLightScaleDataSave(r, be);
    }
    //AI(W906-MT-E2) 20260925: the four Motor Test page actions -- golden uMotorTest handlers only; and BEFORE the generic
    //  set*Speed matcher below, which would otherwise take "setSpeed" for a parameter button.
    if (r.action == "selectMotor" || r.action == "setParamCell" || r.action == "copyFrom" || r.action == "setSpeed") {
        if (r.source != "uMotorTest")
            return Refuse("motor.access " + r.action + "（" + r.source + "）: 這是 golden uMotorTest 的畫面動作（" + row->golden + "），教導頁沒有");
        if (r.action == "selectMotor")  return DoSelectMotor(r, wireId, be);
        if (r.action == "setParamCell") return DoSetParamCell(r, be);
        if (r.action == "copyFrom")     return DoCopyFrom(r, be);
        return DoSetSpeed(r, wireId, be);
    }
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
//  AI(W906-MT-E2) 20260925 加的落差（每一條在程式旁邊都有說明）：
//   3. selectMotor：golden lM00Click 只把 btnHome 抬起、bSingleHome 不動（單軸歸零在背景繼續）；這裡跟其他 AllBtnUp 一樣取消 HOME 工作
//      （HomeFlag 留 0，要再歸零一次）—— 安全方向。
//   4. copyFrom 來源 <0／>24、setParamCell 列號不對：golden 靜默 return；這裡回 ok=false 說原因（一樣什麼都不做）。
//   5. Loop 計時：golden Timer1 每 5 ms 看一次到位；這裡在每次 1203 Poll 之後看（約 200 ms 一次），所以一段的時間最多多算一個輪詢週期。
//   6. 告警時選中軸 Alarm 燈的規則（UpdateMotorLed）：golden 在告警框顯示期間每 5 ms 看；這裡只在告警出現的那一刻看一次
//      （阻塞式告警期間 tick 執行緒停在 modal 等待裡，本來就看不到）。
//   7. LastHomePos：DS402 歸零由驅動器自己設原點，同一公式的值通常約 0，意思跟 golden 卡片式歸零不同；Direction=1 的軸不記。
//   8. Reload Motor Data：表值就地套回，但不重建物件、不 InitMotor（不開軸）、身分欄變了的列不套（要重開 wb_serve）。
//   9. setSpeed：golden LoopMove 進行中捲軸也設「所有 cbUsing 勾選的軸」；頁面不送 cbUsing（All 模式未接），只設這一軸。
//      [AI(W906-MT-E3c) 20260925：All 模式接上後，All 的來回會帶勾選的軸，捲軸照 golden 一起設；單軸來回仍是這個落差。]
//  AI(W906-MT-E3c) 20260925 加的落差：
//  10. Motor Power On：golden DoMotorPowerOn 忙等 1 秒（凍結 UI 執行緒）；這裡跨拍進行（輸出與時間同 golden，主執行緒不凍結），
//      那 1 秒內再按一次會被拒絕（golden 的點擊會排隊到 1 秒後才處理）。OutValue 先照卡片回讀同步（IO 頁寫不到 OutValue）。
//  11. Test Range／Rate：golden InitMotor 的 goto ResetMotorError 無上限；這裡最多 3 輪就停止並回報。WAR16121/16122/16123 的
//      golden 彈窗改成 ack 的逐步結果（本檔慣例：在 motor.access 處理中呼叫 ShowErrorMessage 會重入告警停機、可能卡住主執行緒）。
//  12. HT9050 R2：Mot_Table CardModel=PCI1203 的 Index 軸（M14）當一般 1203 軸 —— golden 對 Index 四軸空白的 SetSpeed 不套用
//      （否則它的速度沒有定義），也不走 Galil 的 "ST"。
//  13. All 模式的起始速度：勾選的 1203 軸以各自上一次 golden SetSpeed 的百分比重送一次（沒設過 = 1%）；golden 不重送（lead 預設）。
//  14. Light Scale：一步驟接一步驟連跑到需要等待為止（golden 10 ms 一步）；1203 讀數要等延遲之後的一次新輪詢；rgAxis 在掃描中途
//      改變，只有在頁面下一次送 BitBtn2／BitBtn3 時才傳到 C++（契約沒有 rgAxisClick 指令）；golden 畫面關閉時單軸歸零會停在
//      原地（MainProc 那一塊只在 fShow 時跑），這裡的 HOME 工作照常完成（完成才設 HomeFlag=1，安全方向）。
//  15. 存檔目錄建不起來時 golden 靜默 return 還顯示 "Save successfully!"；這裡照實拒絕。
//  16. FormClose：LoopMove 在關頁當下結束（golden 是凍結到下一次 FormShow 才抬起，結果相同：那一段已送出的移動照走完）。
// =============================================================================
