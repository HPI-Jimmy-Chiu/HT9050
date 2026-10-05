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
#include "Motor/EcatMotorRoute.h"   //AI(W906-HOME-DUALAXIS) 20261003: W906_EcSiblingHoming (StartHome1203); no includes of its own, inline only

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
//  28 個 action（motor-access.json 38 個命令去重）；AI(W906-MT-E2) 20260925：+4（43 個命令、32 個 action）；AI(W906-MT-E3c) 20260925：+5（48 個命令、37 個 action）；AI(W906-ARMCELL) 20261002：+ moveToTrayCell（58 個命令、42 個 action、live 37）；AI(W906-GEARRATIO) 20261002：+ gearCalMove／gearRatioPreview／gearRatioSave（61 個命令、45 個 action、live 40）。golden 欄 = 該按鈕事件在 golden 的位置，
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
    { "resetMNet",           "live",   "",     "uMotorTest.cpp:1718 btResetMNetClick" },  { "resetAlarm", "live", "", "（golden 沒有這顆鈕）AI(W906-MT-ALMRST) 20260929 EastSun：選取軸的 Alarm Reset = Acm_AxResetError（同 golden TMyEtherCatMotor::MotOutputOn 的 bAlarm 分支，myEthercatmotor.cpp:1794）" },
    //AI(W906-MT-E2) 20260925: the Motor Test functions that needed no ruling (contract with the web side: motor-access.json
    //  buttons labName / strngrdMotor / BitBtn1 / scrlbrMotorSpeed / edtSpeed, source uMotorTest, none of kind motion).
    { "selectMotor",         "live",   "",     "uMotorTest.cpp:734 lM00Click／:762 lpA00Click" },
    { "setParamCell",        "live",   "",     "uMotorTest.cpp:1176 strngrdMotorSelectCell" },
    { "copyFrom",            "live",   "",     "uMotorTest.cpp:1384 BitBtn1Click（來源 = cbbMotorName 的第 2、3 個字元）" },  { "saveMotTable", "live", "", "（golden 沒有這顆鈕；golden 唯一寫 Mot_Table 的是 Motor Database 分頁 sbUpdateClick uMotorTest.cpp:2240-2268，整檔重寫）AI(W906-MT-SAVEMOT) 20260930 EastSun：選取軸 Settings 表（MOT[mi]，UpdateMotorParameter :659-668）寫回 Mot_Table.csv 那一列，只改變了的格子（本檔 EOF）" },  { "gearCalMove", "live", "", "（golden 沒有這顆鈕）RULINGS_20261002 第 22 條 Motor Test Gear Ratio 分頁：一段量測移動（begin＝消背隙、量具零點），自己的閘門（EMG／ALM／伺服／HomeFlag／手臂 Z 在原點／golden 教導互鎖／佔位軟極限 50／100 mm）AI(W906-GEARRATIO) 20261002（本檔 EOF）" },  { "gearRatioPreview", "live", "", "（golden 沒有）Gear Ratio 預覽：C++ 重算擬合、新齒輪比與這一軸教導點／軟極限的「舊 → 新」，不寫檔、不改記憶體 AI(W906-GEARRATIO) 20261002" },  { "gearRatioSave", "live", "", "（golden 沒有）Gear Ratio 存檔：previewKey 相符才寫；備份 → Mot_Table 三格 → 記憶體 → teach.ini → 讀回比對（失敗全部還原）→ 這一軸 HomeFlag=0（不歸零任何軸）AI(W906-GEARRATIO) 20261002" },  { "gearHandBegin", "live", "", "（golden 沒有）Gear Ratio 手推量測開始：記下這一軸的編碼器 E0，不下任何移動命令（之後伺服 OFF → 手推到尺的記號 → 伺服 ON，C++ 自己記 E1；預覽／存檔帶 hand:true、rulerMm）AI(W906-GEARHAND) 20261003 RULINGS_20261003 第 8 條（本檔 EOF）" },
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
    { "teachSet",            "live",   "",     "uteach.cpp:3360 SetButton140Click／:3517 SetButton020Click／:3650 SetButton064Click（手動教導：關伺服→對話框→讀編碼器→開伺服）" },  { "moveToTrayCell", "live", "", "（golden 沒有這顆鈕；HT160 Teach→Advanced→Sort Arm 點位測試的對等物）RULINGS_20261002 第 18 條：Z→ZSafePos、golden IsCanQuickJogMove、X／Y、可選 Z 下（AI(W906-ARMCELL) 20261002，本檔 EOF）" },
    { "teachGo",             "live",   "",     "uteach.cpp:3401 GoButton140Click／:3559 GoButton020Click" },  { "teachZAllUp", "live", "", "uteach.cpp:4466 btnInZAllUpClick／:4480 btnOutZAllUpClick（那一臂每一個 Z：bIn/OutArmZHome=false＋InitProcessSingleMotorTask；Timer1Timer :1374 DoZHome :4505 推進）AI(W906-TEACH-ZALLUP) 20261001" },  { "teachIndexServo", "live", "", "uteach.cpp:4253 btnZ1ServoClick／:4270 btnZ2ServoClick／:2919 btnArm1YServoClick（btnArm2YServo 同一個處理函式）：Gali_Command(\"MO?\"／\"SH?\") 函式內 static 輪流、fAllMotorHome=false AI(W906-TEACH-ZALLUP) 20261001" },  { "teachSt02", "live", "", "uteach.cpp:4605 SpeedButtonInRotatePos90Click／:4657 SpeedButtonInRotateNeg90Click／:4721 btnSht1GoLatchClick／:4737 btnSht2GoLatchClick／:5948 btnSetAllInArmZ_MoveClick／:5973 btnSetAllOutArmZ_MoveClick（AI(W906-ST02-C9-G2) 20261002 St02：本檔 EOF TeachSt02Dispatch）" },  { "teachSetAllArmZ", "live", "", "uteach.cpp:4300 btnSetAllInArmZClick／:4367 btnSetAllOutArmZClick（各吸嘴 Z 讀 ReadPos 再減基準吸嘴；只填畫面欄位，不動、不寫檔）AI(W906-TEACH-ZDOWN) 20261003（本檔 EOF）" },  { "teachOutZAllDown", "live", "", "uteach.cpp:4542 btnOutZAllDownClick（每一支 Out Z：SetSpeed(10)，再 MotorMove(SetEditPickOutSht＋OutZEditPtr[i][j])）AI(W906-TEACH-ZDOWN) 20261003（本檔 EOF）" },
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
std::map<int, unsigned long> g_issuedPoll;  std::map<int, unsigned long> g_issuedSeq;   //AI(W906-GEARRATIO) 20261002: a per-axis count of this file's motion commands (bumped with g_issuedPoll, and by the Index Z1 route's MotorAccessNoteIssuedAt, EOF) -- the Gear Ratio session's 'nothing else moved this axis'. Same line, nothing moves
void NoteIssued(IMotorAccessBackend& be, int axis) { g_issuedPoll[axis] = be.Pci1203PollCount(); ++g_issuedSeq[axis]; }   //AI(W906-GEARRATIO) 20261002: + g_issuedSeq
bool SampleStale(IMotorAccessBackend& be, int axis)
{
    std::map<int, unsigned long>::const_iterator it = g_issuedPoll.find(axis);
    return it != g_issuedPoll.end() && be.Pci1203PollCount() <= it->second;
}
const char* kStaleWhy = "上一個命令之後監看器還沒更新（輪詢週期 200 ms），READY／位置還是命令前的樣本 —— 稍候再按";

//AI(W906-W5-b) 20260925: W5B-6 —— 手動教導（伺服關、手推）之後命令位置可能還停在推之前：golden TMyMotor::ServoOnOff(true) 在
//  PServoAlarmOn 的軸上 PCIL132_ResetPos（命令位置＝編碼器，mymotor.cpp:1936），EastSun 刻意不提供 Acm_AxSetCmdPosition
//  （Pci1203Control.h:222）⇒ 這些 1203 軸槽的「目前位置」（相對移動、jog 的軟體極限、setSoftLimit、畫面 cmdPos）改讀編碼器 actPos。
//  清單只增不減（這個行程結束才清）：之後的命令位置有沒有追上實際位置，要到機台量過才知道（docs/W5_PROGRESS.md §6（W5-b） 待決）。  [AI(W906-SVON-ENCSYNC) 20261004: 不再「只增不減」—— Servo ON 時照 golden 把命令位置寫成編碼器位置的軸會移出清單（ServoOnEncSync，檔尾）]
std::set<int> g_encBase;  bool ServoOnEncSync(IMotorAccessBackend& be, long long wireId, const MotorAccessAxis& a, std::string& note);   //AI(W906-SVON-ENCSYNC) 20261004: NB2-1 (n) -- a Servo ON that writes the encoder back to the command position (golden PCIL132_ResetPos) takes the axis OFF this list (def at EOF)

//AI(W906-W5-b) 20260925: 覆核 R-W5B-4 —— golden MOT[mi] 目前的 SetSpeed 百分比（只記 1203 軸；非 1203 軸 golden 的 MOT 物件自己記）。
//  golden 教導頁的 jog（TMyMotor::JogP(Speed) 不用 Speed，mymotor.cpp:1876-1888）與 MoveP／MoveN／MoveTo（uteach.cpp:2104-2131／
//  :2205-2232／:2391-2404）**都不設速度**，用的是這一軸上一次被設的速度；會設速度的是 ScrollBar1／edtSpeed 改變（:1312／:2442）、
//  選馬達（UpdateMotorTeachMonitor :1275 → 1%）、HOME 抬起（:2196 → 1%）、Go 鈕（GoButton140 → 1%、GoButton020 → 20%）、
//  以及 MotorTest 的每一次 SetSpeed（同一個 MOT 物件）。1203 軸在 EastSun 監看器開的 handle 上，卡上的 PTP 速度還會被 W4C-3 的
//  「歸零前把 PTP 速度設成歸零速度」蓋掉 ⇒ 教導頁每一次 jog／移動前，把這裡記的值重送一次（值不變，等於 golden 的「卡上是上次那個速度」）。
//AI(W906-MERGE-56bbf785) 20260926: 1203 軸的教導頁 jog 不再用這裡（改用 DoJog 的 g_jogSpd／JogPctOf，見 DoTeachJog）；這裡只剩移動（Move1203）用。
std::map<int, int> g_goldenPct;
int                g_teachEdt = -1;       // golden edtSpeed->Text 的鏡像（-1＝不知道；VCL Text 沒變不觸發 OnChange）
int                g_teachEdtMi = -1;     //AI(W906-TEACH-SPD-AXIS) 20261001: the motor g_teachEdt was last applied to (-1 = none)
std::string        g_teachHomeUnknown;    // 最近一次 MotorAccessTeachHomeLed 回 -1 的原因（W5B-R2）

// W4B-4：正在 jog 的軸（放開／STOP 清掉；操作員連線消失時由 MotorAccessTick 停下）
struct JogRec { bool is1203; int axis; int mi; std::string motorId; std::string source; };   //AI(W906-WSLINK-B) 20260929: + source (uMotorTest / uteach) -- MotorAccessPageClosed stops only the closed page's jogs
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
std::string JobsBusyWhy(const std::string& who, const MotorAccessReq& r, IMotorAccessBackend& be);   // 定義在 W4-b2 工作區塊（AI(W906-MT-AXISLOCK) 20260929: per axis）
void CancelJobsOnAxis(IMotorAccessBackend& be, const std::string& id, const MotorAccessAxis& a, const std::string& why);  void CancelJobsOnAxisKeepHand(IMotorAccessBackend& be, const std::string& id, const MotorAccessAxis& a, const std::string& why);  void GearHandServoSeen(IMotorAccessBackend& be, int axis, bool on);  void GearHandTick(IMotorAccessBackend& be);   //AI(W906-MT-AXISLOCK) 20260929: 同上  //AI(W906-GEARHAND) 20261003: + the Gear Ratio hand-push session's three hooks (bodies in the gear block, EOF)

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
        CancelJobsOnAxisKeepHand(be, r.motors[0], a, "servo toggled (golden btnServoOffClick: AllBtnUp)");   //AI(W906-MT-AXISLOCK) 20260929: only this axis' HOME / LoopMove (EastSun: axes independent)  //AI(W906-GEARHAND) 20261003: ...KeepHand -- the same, but a Gear Ratio HAND session on this axis is not ended (its servo OFF / ON IS this button)

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
        //AI(W906-BRAKE-AXIS) 20260929: SERVO OFF -> this axis' brake output to HOLD first, 100 ms for it to grip, then the servo off
        //  (a Z axis must not hang on nothing). SERVO ON -> nothing here: the brake is released 0.5 s after the monitor sees SVON
        //  (WebMotorAccessLive.cpp W906_BrakeAxisTick, EastSun 20260929).
        std::string brakeNote;
        if (!target && be.BrakeHoldBeforeServoOff(r.motors[0], brakeNote)) be.SleepMs(100);
        Pci1203Cmd c;
        c.kind = kCmdAxSvOn;
        c.wireId = wireId;
        c.axis = a.axis;
        c.value = target ? 1.0 : 0.0;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        if (CmdFailed(res))
            return Refuse("servoToggle " + r.motors[0] + ": 1203 失敗 —— " + CmdWhy(res) + (brakeNote.empty() ? std::string() : "（" + brakeNote + "）"));
        std::string syncNote; if (res.issued && target) ServoOnEncSync(be, wireId, a, syncNote);  if (res.issued) GearHandServoSeen(be, a.axis, target);  AckHead(w, r, target ? "servoOn" : "servoOff", "pci1203",   //AI(W906-GEARHAND) 20261003: a Gear Ratio hand session on this axis sees its servo OFF / ON (gear block, EOF)
                (res.issued ? std::string("sent: ") + res.wouldCall
                            : std::string("accepted, NOT issued (1203 control is in dry mode): ") + res.wouldCall) +
                (brakeNote.empty() ? std::string() : "; " + brakeNote) +
                (target ? "; brake: released 0.5 s after SVON (if this axis has a brake output)" : "") + (syncNote.empty() ? std::string() : "; " + syncNote));   //AI(W906-SVON-ENCSYNC) 20261004: golden ServoOnOff(true) -> PCIL132_ResetPos on a PServoAlarmOn axis (ServoOnEncSync, EOF)
        w.Key("axis").Number((wb_int64)a.axis);
        w.Key("issued").Bool(res.issued);
        w.Key("wouldCall").String(res.wouldCall);  w.Key("encSync").String(syncNote);
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
//   written (gap, noted in the ack). [AI(W906-JOG-VLTIME) 20261003: now written for STEPPER axes through kCmdAxSetInitCfg's U32
//   setter (0 = same bits), see the end of Send1203SpeedValues; servo axes still not, EastSun 1003.]
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
    //AI(W906-JOG-MAXVEL) 20261003: EastSun「M35~M40 … 我設定JOG 速度都沒有效過」. The card refuses / clamps a velocity above
    //  CFG_AxMaxVel (Pci1203Monitor.h:856-871; measured 13:15 on M35: re-apply "card returned 0x80000087"), and that ceiling was
    //  written ONLY by InitMotor (golden :363-382 CFG_AxMaxVel = PJogHighSpeed, MaxAcc = dAcc, MaxDec = dDec; InitMotor1203 below),
    //  which on this machine runs at boot for MLoaderZ only -- so a JogHigh edited later never got a ceiling to match. Before the jog
    //  family goes out, a ceiling the monitor KNOWS to be below what this jog needs is set again to golden InitMotor's own values
    //  (never below this velHigh / acc / dec); unknown or already high enough = nothing sent (as before).
    //  AI(W906-MT-ACCLIVE) 20261003: the PTP family too (Move + / Move - / Go run on it; CFG_AxMaxAcc from InitMotor was the old Acc).
    if (families & (kFamJog | kFamPtp)) {
        const struct { Pci1203SpeedParam which; double need; double v; } mx[3] = {
            { kSpeedMaxVel, sp.velHigh, (double)m.g.jogHigh > sp.velHigh ? (double)m.g.jogHigh : sp.velHigh },
            { kSpeedMaxAcc, sp.acc, m.g.acc > sp.acc ? m.g.acc : sp.acc }, { kSpeedMaxDec, sp.dec, m.g.dec > sp.dec ? m.g.dec : sp.dec } };
        for (int i = 0; i < 3; ++i) {
            double now = 0;
            if (!(mx[i].v > 0) || !be.Pci1203SpeedNow(m.a.axis, (int)mx[i].which, now) || now >= mx[i].need) continue;
            Pci1203Cmd c;
            c.kind = kCmdAxSetSpeed; c.wireId = wireId; c.axis = m.a.axis; c.speed = mx[i].which; c.value = mx[i].v;
            const Pci1203CmdResult res = be.Pci1203Execute(c);
            if (CmdFailed(res)) { why = "1203 設定速度上限失敗（CFG_AxMax*）—— " + CmdWhy(res); return false; }
        }
    }
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
    //AI(W906-JOG-VLTIME) 20261003: EastSun「M35 加減速已經設定成超大 一樣沒有動 反而是 我按了有一段時間 他會突然動很快」.
    //  Acm_AxJog runs at CFG_AxJogVelLow for CFG_AxJogVLTime, then jumps to CFG_AxJogVelHigh; on a stepper velLow = InitSpeed *
    //  percent (golden SetSpeed :915-921) is nearly standstill. golden writes CFG_AxJogVLTime = 0 with the jog family
    //  (myEthercatmotor.cpp:724) so the jog is at JogVelHigh at once. Stepper axes only (Mot_Table 1P2P=0, M35..M40):
    //  EastSun 1003「不要改到其他伺服馬達喔 因為其他是正常的」-- so the servo axes get exactly what they got before.
    //AI(W906-JOG-VLTIME-2) 20261005: EastSun「我現在所以軸 jog 按下後 超過10秒 會變得超級快」-> 「照原版：所有軸 VLTime=0」: the servo axes
    //  jogged at CFG_AxJogVelLow (1/10 of velHigh) for the card's default VLTime (~10 s) and then jumped to velHigh. Every axis now, as
    //  golden (myEthercatmotor.cpp:1006, no motor-type condition): the jog runs at the set JOG speed from the start.
    std::string vlTimeWarn;
    if (families & kFamJog) {
        Pci1203Cmd c;
        c.kind = kCmdAxSetInitCfg; c.wireId = wireId; c.axis = m.a.axis; c.initCfg = kInitCfgJogVLTime; c.value = 0.0;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        if (CmdFailed(res)) {
            if (m.g.stepMotor) { why = "1203 設定 CFG_AxJogVLTime=0 失敗 —— " + CmdWhy(res); return false; }   // the stepper rule of 10-03 unchanged
            vlTimeWarn = "; CFG_AxJogVLTime=0 失敗（照原版不擋 JOG，golden WAR16122）—— " + CmdWhy(res);           //AI(W906-JOG-VLTIME-2) 20261005: servo axes as golden
        }
    }
    //AI(W906-W5-b) 20260925: R-W5B-4（golden MOT.SetSpeed 之後 MOT 記住的速度）
    //AI(W906-MERGE-56bbf785) 20260926: laptop recorded every Send1203Speed (then PTP only); the machine split the families -- a
    //  golden SetSpeed(x[, true]) always writes the PTP family, a jog-family-only re-send is not a SetSpeed, so only kFamPtp records.
    if (families & kFamPtp) g_goldenPct[m.a.motIndex] = pct;
    char b[200];
    std::snprintf(b, sizeof(b), "speed %d%% -> s=%u velLow=%.1f velHigh=%.1f acc=%.1f dec=%.1f (%s)",
                  pct, sp.s, sp.velLow, sp.velHigh, sp.acc, sp.dec,
                  families == (kFamPtp | kFamJog) ? "PTP + jog" : (families == kFamJog ? "jog family only" : "PTP"));
    note = std::string(b) + vlTimeWarn;
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
    { const std::string busy = JobsBusyWhy(r.action, r, be); if (!busy.empty()) return Refuse(busy); }   // NB2 R23 W4C-6（:815／:865）
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
        { JogRec j = { false, -1, m.a.motIndex, r.motors[0], r.source }; ForgetJog(false, -1, m.a.motIndex); g_jogs.push_back(j); }
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
    { JogRec j = { true, m.a.axis, m.a.motIndex, r.motors[0], r.source }; ForgetJog(true, m.a.axis, m.a.motIndex); g_jogs.push_back(j); }
    return OkAck(r, "jogging", "pci1203",
                 std::string(res.issued ? "sent: " : "accepted, NOT issued (dry): ") + res.wouldCall + "; " + note,
                 m, 0, false, 0, false);
}

// ---- moveRelative：MovePClick :1252／MoveNClick :1224 ----
MotorAccessOutcome DoMoveRelative(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    { const std::string busy = JobsBusyWhy(r.action, r, be); if (!busy.empty()) return Refuse(busy); }   // NB2 R23 W4C-6（:1226／:1255）
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
    { const std::string busy = JobsBusyWhy(r.action, r, be); if (!busy.empty()) return Refuse(busy); }   // NB2 R23 W4C-6（:1099／:1107）
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
std::string ReapplySpeedAfterEdit(long long wireId, IMotorAccessBackend& be, const std::string& motorId);   //AI(W906-MT-SPDLIVE) 20261001: defined before DoSetParamCell
MotorAccessOutcome DoSetParam(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)   //AI(W906-MT-SPDLIVE) 20261001: + wireId
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
    //AI(W906-MT-SPDLIVE) 20261001: JogHigh / JogLow are what the speed is made of -> re-applied now (EastSun 1001)
    const std::string spd = (which == kParamJogHigh || which == kParamJogLow) ? ReapplySpeedAfterEdit(wireId, be, r.motors[0]) : std::string();
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "set", "MOT", std::string("MOT[") + std::to_string(m.a.motIndex) + "].Motor 參數已改（只在記憶體；golden 同，存檔走資料庫分頁）" + spd);
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
    bool        fromLS;  int kind;  bool zAllUp;   //AI(W906-MT-SMHOME) 20261001: 0 = 1203 (this file's home), 1 = golden ProcessSingleMotorHome (a non-1203 row; axis stays -1)  //AI(W906-TEACH-ZALLUP) 20261001: zAllUp = started by Teach's In/Out Z All Up (golden DoZHome: no SetSpeed when it ends)
    HomeJob() : active(false), mi(-1), axis(-1), phase(0), sawHoming(false), elapsedMs(0), readyMs(0), zTarget(0), zIssued(false),
                cardSide(false), zeroed(false), zeroWaitMs(-1), fromLS(false), kind(0), zAllUp(false) {}
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
std::vector<HomeJob> g_homes;  MotorAccessOutcome DoHomeGolden(const MotorAccessReq& r, IMotorAccessBackend& be, const MotionCtx& m, const std::string& who, bool wantStart);  void TickGoldenHome(IMotorAccessBackend& be, HomeJob& h);  bool LsStartGoldenHome(IMotorAccessBackend& be, int mi, const std::string& alias, const MotionCtx& m);  MotorAccessOutcome DoTeachZAllUp(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be);  MotorAccessOutcome DoTeachIndexServo(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be);  void TeachZAllUpResetState();   //AI(W906-MT-SMHOME) 20261001: bodies at the end of this file  //AI(W906-TEACH-ZALLUP) 20261001: + DoTeachZAllUp / DoTeachIndexServo (bodies at the end of this file too)
LoopJob              g_loop;  bool ArmCellActive();  void CancelArmCell(IMotorAccessBackend& be, const std::string& why);  bool ArmCellUsesAxis(int axis1203);  bool ArmCellOnAxis(const std::string& id, const MotorAccessAxis& b);  std::string ArmCellBusyWhy(const MotorAccessReq& r, IMotorAccessBackend& be);  void TickArmCell(IMotorAccessBackend& be);  void ArmCellPollTick(IMotorAccessBackend& be);  void ArmCellResetState();  std::string ArmCellStartBlockedWhy();  void ArmCellFillJobs(MotorAccessJobState& j);  MotorAccessOutcome DoArmCell(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be);  bool GearActive();  void CancelGear(IMotorAccessBackend& be, const std::string& why);  bool GearOnAxis(const std::string& id, const MotorAccessAxis& b);  void GearResetState();  void GearFillJobs(MotorAccessJobState& j);  MotorAccessOutcome DoGear(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be);   //AI(W906-ARMCELL) 20261002: the Teach Arm Cell job (RULINGS_20261002 #18); bodies at the end of this file  //AI(W906-GEARRATIO) 20261002: + the Motor Test Gear Ratio tab's (RULINGS_20261002 #22), bodies at the end of this file  //AI(W906-ARMCELL-ZORG) 20261003: + ArmCellPollTick (the Arm Cell Z origin watch right after each 1203 Poll; EOF)
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
    if (sendStop) { if (h.kind != 0) be.GoldenStopMotor(h.mi); else Stop1203(be, h.axis, -1); }                                    // AI(W906-MT-E1): StopDec + ExtDrive 0; //AI(W906-MT-SMHOME) 20261001: a golden (non-1203) home stops with golden PCIL132_StopMotor
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
    CancelLoop(be, false, why);  CancelArmCell(be, why);  CancelGear(be, why);   //AI(W906-ARMCELL) 20261002: + the Teach Arm Cell job (it also stops its own axes: D3, EOF)  //AI(W906-GEARRATIO) 20261002: + the Gear Ratio session  //AI(W906-GEARRATIO2) 20261003: NB2 R171 M1 -- it ends AND stops its own axis when its leg is that axis' latest command (CancelGear, EOF); unlike a HOME / Loop leg, which golden lets run on
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
    return g_loop.active || ArmCellActive() || GearActive();   //AI(W906-ARMCELL) 20261002  //AI(W906-GEARRATIO) 20261002: + the Gear Ratio session (so the beat's operator-gone / safe-lock gates end it, and Arm Cell refuses while it runs)
}

//AI(W906-MT-AXISLOCK) 20260929: EastSun「每個軸都是獨立可控的」-- golden's first line of every motion button
//  (`if(... btnHome->Down || btnLoopMove->Down) return;`) refuses while ANY HOME / LoopMove runs; here only a HOME / LoopMove
//  on the SAME axis refuses. Same axis = same Alias, same MOT index, or the same opened 1203 axis.
bool SameAxisAs(const std::string& idA, int miA, int axA, const std::string& idB, const MotorAccessAxis& b)
{
    return idA == idB || (miA >= 0 && miA == b.motIndex) || (axA >= 0 && b.Is1203() && axA == b.axis);
}
bool HomeOnAxis(const HomeJob& h, const std::string& id, const MotorAccessAxis& b) { return h.active && SameAxisAs(h.motorId, h.mi, h.axis, id, b); }
bool LoopOnAxis(const std::string& id, const MotorAccessAxis& b)                // the loop moves this axis (single: its motor; All: a checked one)
{
    if (!g_loop.active) return false;
    if (!g_loop.all) return SameAxisAs(g_loop.motorId, g_loop.a.motIndex, g_loop.a.Is1203() ? g_loop.a.axis : -1, id, b);
    for (std::size_t k = 0; k < g_loop.axes.size(); ++k) {
        const LoopAxis& x = g_loop.axes[k];
        if (SameAxisAs(x.motorId, x.a.motIndex, x.a.Is1203() ? x.a.axis : -1, id, b)) return true;
    }
    return false;
}
std::string JobsBusyWhy(const std::string& who, const MotorAccessReq& r, IMotorAccessBackend& be)
{
    MotorAccessAxis b;
    const std::string id = r.motors.empty() ? std::string() : r.motors[0];
    if (!id.empty() && !be.Resolve(id, b)) b = MotorAccessAxis();               // unknown Alias: compared by name only (the handler refuses it later)
    for (std::size_t i = 0; i < g_homes.size(); ++i)
        if (g_homes[i].active && !g_homes[i].fromLS && (id.empty() || HomeOnAxis(g_homes[i], id, b)))   //AI(W906-MT-E3c) 20260925: btnHome->Down only (not Light Scale's single home)
            return who + ": 這一軸 HOME 進行中（" + g_homes[i].motorId + "）—— 先按 HOME 停止或 STOP（EastSun 20260929：只擋同一軸，別軸照常可動）";
    if (g_loop.active && (id.empty() || LoopOnAxis(id, b)))
        return who + ": 這一軸 LoopMove 進行中（" + g_loop.motorId + "）—— 先按 LoopMove 停止或 STOP（EastSun 20260929：只擋同一軸，別軸照常可動）";
    if (ArmCellActive() && (id.empty() || ArmCellOnAxis(id, b))) return who + ": 這一軸是教導頁 Arm Cell 正在用的軸 —— 等它走完或按 STOP";  return std::string();   //AI(W906-ARMCELL) 20261002
}
// golden AllBtnUp of ONE axis' button (servo toggle): its HOME (not Light Scale's) and a loop that moves it.
void CancelJobsOnAxis(IMotorAccessBackend& be, const std::string& id, const MotorAccessAxis& a, const std::string& why)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (!g_homes[i].fromLS && HomeOnAxis(g_homes[i], id, a)) CancelHome(be, g_homes[i], false, why);
    if (LoopOnAxis(id, a)) CancelLoop(be, false, why);  if (ArmCellOnAxis(id, a)) CancelArmCell(be, why);  if (GearOnAxis(id, a)) CancelGear(be, why);   //AI(W906-ARMCELL) 20261002  //AI(W906-GEARRATIO) 20261002
}

//AI(W906-MT-ALMRST) 20260929: Alarm Reset of the SELECTED axis (EastSun 20260929「我需要有alarm reset的按鈕在介面上 並且要有實際功能」).
//  golden Motor Test has no such button. What it does is golden's own reset: TMyEtherCatMotor::MotOutputOn's bAlarm branch
//  (Motor/myEthercatmotor.cpp:1792-1799) = Acm_AxResetError on this axis -- it clears the card's ERROR_STOP and sends the
//  drive's fault reset (A.A12 after a power cycle etc.). It moves nothing and does not servo-ON (after a fault the servo is
//  OFF; press Servo On afterwards). One axis only (EastSun: axes independent). A non-1203 row is refused (no reset here).
MotorAccessOutcome DoResetAlarm(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (r.motors.empty()) return Refuse("resetAlarm: 沒有選馬達 —— 先在左邊點一軸");
    MotorAccessAxis a;
    if (!be.Resolve(r.motors[0], a)) return Refuse("resetAlarm: 馬達表上沒有 " + r.motors[0]);
    const std::string who = "resetAlarm " + r.motors[0];
    if (!a.Is1203()) return Refuse(who + "（" + a.cardModel + "）: 只有 PCI1203 軸有 Alarm Reset（Acm_AxResetError）");
    if (!a.tableEnable) return Refuse(who + ": " + kEnable0Why);
    std::string why;
    if (!be.Pci1203Ready(why)) return Refuse(who + "（PCI1203）: 1203 控制層不可用 —— " + why);
    if (a.axis < 0) {
        const std::string msg = who + "（PCI1203）: " + a.why;
        be.Pci1203NoteRefusal(wireId, "motor.access resetAlarm", msg);
        return Refuse(msg);
    }
    unsigned st = 0;
    unsigned long io = 0;
    const bool hasSt = be.Pci1203AxisState(a.axis, st), hasIo = be.Pci1203MotionIO(a.axis, io);
    Pci1203Cmd c;
    c.kind = kCmdAxResetError; c.wireId = wireId; c.axis = a.axis;
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) return Refuse(who + ": 1203 Acm_AxResetError 失敗 —— " + CmdWhy(res));
    NoteIssued(be, a.axis);                                                     // the next fresh sample shows the result
    char before[96];
    std::snprintf(before, sizeof(before), "before: state=%s ALM=%s", hasSt ? std::to_string(st & 0xFFu).c_str() : "?",
                  hasIo ? ((io & 0x00000002ul) ? "1" : "0") : "?");
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "alarmReset", "pci1203",
            std::string(res.issued ? "sent: " : "accepted, NOT issued (dry): ") + res.wouldCall + "; " + before +
            " -- 不會動軸、不會自己激磁（清完要再按 Servo On）；清不掉代表異常原因還在（看驅動器面板／1203 頁的 driveAlarm）");
    w.Key("axis").Number((wb_int64)a.axis);
    w.Key("issued").Bool(res.issued);
    w.Key("wouldCall").String(res.wouldCall);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
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
//AI(W906-HOME-MAXVEL-SINGLE) 20261003: EastSun 1003「查一下M30 為何歸原點失敗」-- Motor Test HOME on M30 MTrayX: "1203 設定卡片歸零速度
//  失敗 ... card returned 0x80000081" = InvalidAxParHomeVelLow (AdvMotErr.h:177): HomeLowSpeed 20000 / HomeHighSpeed 30000 above the card's
//  CFG_AxMaxVel = PJogHighSpeed 15000 that golden InitMotor writes (the full HOME's InitMotor had run). The engine home raises the
//  ceiling first (Pci1203MotorRoute.cpp HOME-MAXVEL); the single HOME did not. Same rule as the jog's JOG-MAXVEL above: a CFG_AxMax*
//  the monitor KNOWS to be below what this home needs is raised to it (never lowered; unknown = nothing sent).
bool RaiseCeilingForHome(IMotorAccessBackend& be, long long wireId, const MotionCtx& m, double vel, double acc, double dec, std::string& why)
{
    const struct { Pci1203SpeedParam which; double need; } mx[3] = { { kSpeedMaxVel, vel }, { kSpeedMaxAcc, acc }, { kSpeedMaxDec, dec } };
    for (int i = 0; i < 3; ++i) {
        double now = 0;
        if (!(mx[i].need > 0) || !be.Pci1203SpeedNow(m.a.axis, (int)mx[i].which, now) || now >= mx[i].need) continue;
        Pci1203Cmd c;
        c.kind = kCmdAxSetSpeed; c.wireId = wireId; c.axis = m.a.axis; c.speed = mx[i].which; c.value = mx[i].need;
        const Pci1203CmdResult res = be.Pci1203Execute(c);
        if (CmdFailed(res)) { why = "1203 回原點前拉高速度上限失敗（CFG_AxMax*）—— " + CmdWhy(res); return false; }
    }
    return true;
}
double HomeVelNeed(const MotorGolden& g) { return (double)(g.homeHigh > g.homeLow ? g.homeHigh : g.homeLow); }

std::string StartHomeCardSide(const std::string& motorId, long long wireId, IMotorAccessBackend& be,
                              const MotionCtx& m, const std::string& who, bool fromLS, HomeStartInfo& info)
{
    if (!be.Pci1203AxisReady(m.a.axis)) {                                       // golden case 1: !MotionDone() -> DecStop; break (wait)
        Stop1203(be, m.a.axis, wireId);
        return who + ": 軸還在動（golden EtherCatMotHome case 1：先 DecStop，停好後再按一次 HOME）";
    }
    Stop1203(be, m.a.axis, wireId);                                             // golden: DecStop() before SetHomeSpeed
    { std::string cw; if (!RaiseCeilingForHome(be, wireId, m, HomeVelNeed(m.g), m.g.accDb, m.g.decDb, cw)) return who + ": " + cw; }   //AI(W906-HOME-MAXVEL-SINGLE) 20261003
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
    //AI(W906-HOME-DUALAXIS) 20261003: EastSun 裁決「自動偵測雙軸驅動器 如果有一軸在回原點 另一軸 就不能先歸 要等雙軸的第一軸歸完後
    //  第二軸才能歸」—— 同一站號（雙軸驅動器）的另一軸正在回原點（1203 監看器取樣 HOMING，或引擎剛送出的回原點）就不送，回原因、等它回完再按。
    //  OFF: EastSun 1003「雙軸不同時歸原點 這也先取消 我先看看能不能雙軸同時」-- the refusal below is not applied (route function kept for later).
    //  if (W906_EcSiblingHoming(m.a.boardId, m.a.port)) return who + ": 同一台雙軸驅動器的另一軸正在回原點 —— 等它回完再按 HOME";
    {   //AI(W906-HOME-VENDOR) 20260929: EastSun「你看範例程式 怎寫就怎改」-- Advantech's own Home examples set the CARD's home family
        //  before the home call (Examples_EtherCAT/Windows/BCB/Home/Unit1.cpp:663-708 PAR_AxHomeVelLow/High/Acc/Dec, then :376
        //  Acm_AxHome(mode, dir); the WinForm one Form1.h:2371-2410 the same four, then :2164 with CiA402_MODE24/28 = 124/128 as here),
        //  and golden TMyEtherCatMotor::SetHomeSpeed (myEthercatmotor.cpp:2113-2140) writes exactly those four from PHomeLow/HighSpeed
        //  and dAcc/dDec. This DS402 path used to skip them (the card-side path StartHomeCardSide above always wrote them), relying only
        //  on the PTP seed below; now it writes them first, like the examples and golden. The PTP seed is KEPT: EastSun measured
        //  20260917 that Acm_AxHome copies PAR_AxVelHigh/VelLow/Acc into 6099h:1/:2 / 609Ah (Pci1203Gear.h:476-507), so without it the
        //  drive would home at whatever PTP speed the last jog left. The drive's 6099h/609Ah and gear are logged (op log MOT drvHome/gear).
        { std::string cw; if (!RaiseCeilingForHome(be, wireId, m, HomeVelNeed(m.g), m.g.accDb, m.g.decDb, cw)) return who + ": " + cw; }   //AI(W906-HOME-MAXVEL-SINGLE) 20261003: M30 0x80000081
        const struct { int which; double v; } hs[4] = {
            { kHomeVelLow, (double)m.g.homeLow }, { kHomeVelHigh, (double)m.g.homeHigh }, { kHomeAcc, m.g.accDb }, { kHomeDec, m.g.decDb } };
        for (int i = 0; i < 4; ++i) {
            Pci1203Cmd hc;
            hc.kind = kCmdAxSetHome; hc.wireId = wireId; hc.axis = m.a.axis; hc.home = hs[i].which; hc.value = hs[i].v;
            const Pci1203CmdResult hr = be.Pci1203Execute(hc);
            if (CmdFailed(hr)) return who + ": 1203 設定卡片歸零速度失敗（golden SetHomeSpeed／研華 Home 範例）—— " + CmdWhy(hr);
        }
    }
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

//AI(W906-HOME-STEPLEAVE) 20261003: StartHome1203 behind the shared MotorAccessStepperLeaveOrigin (EastSun ruling 1003, every home):
//  a SW3D stepper on its origin first gets a HomeJob in phase 10 (leaving the origin, TickHomes) and the home starts from there.
//  Returns "" when the home started or the leave job was pushed (`leaving` says which), else the refusal.
std::string StartHomeWithLeave(const std::string& motorId, long long wireId, IMotorAccessBackend& be, const MotionCtx& m,
                               const std::string& who, bool fromLS, HomeStartInfo& info, bool& leaving)
{
    leaving = false;
    //AI(W906-HOME-STEPLEAVE-3) 20261003 (review): the 1000-pulse move must pass the same gates StartHome1203 / StartHomeCardSide apply
    //  before any motion -- safe door (golden Home's CheckIsSafeDoorOpen), a fresh sample, the axis standing (golden case 1 DecStop first).
    if (be.GoldenSafeDoorOpen(m.a.motIndex))
        return who + ": 安全門開著（golden TMyMotor::Home 的 CheckIsSafeDoorOpen）—— 關門後再按 HOME";
    if (SampleStale(be, m.a.axis)) return who + ": " + kStaleWhy;
    if (!be.Pci1203AxisReady(m.a.axis)) { Stop1203(be, m.a.axis, wireId); return who + ": 軸還在動（golden EtherCatMotHome case 1：先 DecStop，停好後再按一次 HOME）"; }
    MotorAccessStepperLeaveReset(motorId);
    std::string why;
    const int lv = MotorAccessStepperLeaveOrigin(be, motorId, why);
    if (lv < 0) { be.GoldenSetHomeFlag(m.a.motIndex, 2); be.GoldenMotorHomeAlarm(m.a.motIndex, why); return who + ": " + why; }
    if (lv == 0) {
        be.GoldenSetHomeFlag(m.a.motIndex, 0);
        HomeJob h;
        h.active = true; h.motorId = motorId; h.mi = m.a.motIndex; h.axis = m.a.axis; h.g = m.g; h.fromLS = fromLS; h.phase = 10;
        h.zTarget = be.GoldenZSafePos();
        PushHomeJob(h);
        leaving = true;
        return std::string();
    }
    return StartHome1203(motorId, wireId, be, m, who, fromLS, info);
}

MotorAccessOutcome DoHome(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    { MotorAccessAxis hb; const std::string hid = r.motors.empty() ? std::string() : r.motors[0]; if (!hid.empty() && !be.Resolve(hid, hb)) hb = MotorAccessAxis();
      if (hid.empty() || LoopOnAxis(hid, hb)) CancelLoop(be, false, "home pressed (golden btnHomeClick :1115: btnLoopMove->Down=false)"); }   // golden 第一行，在任何檢查之前 —— AI(W906-MT-AXISLOCK) 20260929: only a loop on THIS axis (EastSun: axes independent)
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
        return DoHomeGolden(r, be, m, who, wantStart);   //AI(W906-MT-SMHOME) 20261001: RULINGS_20261001 #8「照golden接上」-- golden btnHomeClick's non-1203 branch (uMotorTest.cpp:1152-1158 / release :1168-1171), body at the end of this file. Was a refusal citing the return-true stub that AI(W906-SMHOME) 0927 retired (ProcessSingleMotorHome is translated at the end of uhome.cpp)
    HomeJob* running = FindHome(m.a.axis);
    if (!wantStart) { const bool wasLsHome = running && running->fromLS;   /*AI(W906-HOME-PERAXIS) 20261003*/   // golden else 支（:1166）：bSingleHome=false; PCIL132_StopMotor();（無條件停）
        if (running) CancelHome(be, *running, true, "home released (golden: bSingleHome=false; PCIL132_StopMotor)");
        else Stop1203(be, m.a.axis, wireId);                                   // AI(W906-MT-E1): golden PCIL132_StopMotor = DecStop
        //AI(W906-HOME-PERAXIS) 20261003: was EndSingleHome(...) -- golden's ONE bSingleHome flag, which here cancelled EVERY axis's home job
        //  (without stopping those axes: they kept moving untracked, HomeFlag never 1). Axes are independent (EastSun ruling 20260929 / 1003):
        //  only this axis's job ends above; Light Scale's single-home wait drops only when it was this axis's job.
        if (wasLsHome && g_ls.homePending) { g_ls.homePending = false; g_ls.lastNote = "bSingleHome=false (HOME released on the Light Scale axis) -> Light Scale case 1 goes on"; }   //AI(W906-MT-E3c) 20260925: the shared flag -- also ends Light Scale's single home
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
    bool leaving = false;
    const std::string refused = StartHomeWithLeave(r.motors[0], wireId, be, m, who, false, info, leaving);   //AI(W906-HOME-STEPLEAVE) 20261003: was StartHome1203 directly
    if (!refused.empty()) return Refuse(refused);
    webbridge::JsonWriter w;
    w.BeginObject();
    if (leaving) {                                                              //AI(W906-HOME-STEPLEAVE) 20261003
        AckHead(w, r, "homing", "pci1203", "步進軸在原點上：先往回原點的反方向移 1000（最多 3 次），離開原點後才開始回原點（EastSun 1003 規則）");
        w.Key("homeActive").Bool(true);
        w.Key("axis").Number((wb_int64)m.a.axis);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
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
        for (std::size_t k = 0; k < axes.size(); ++k)                           //AI(W906-MT-AXISLOCK) 20260929: only a HOME on an axis this loop moves (EastSun: axes independent)
            if (g_homes[i].active && !g_homes[i].fromLS && HomeOnAxis(g_homes[i], axes[k].motorId, axes[k].a)) CancelHome(be, g_homes[i], true, "loop pressed (golden btnHome->Down=false)");
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
        { MotorAccessAxis lb; const std::string lid = r.motors.empty() ? std::string() : r.motors[0]; if (!lid.empty() && !be.Resolve(lid, lb)) lb = MotorAccessAxis();
          if (!lid.empty() && lid != g_loop.motorId && !LoopOnAxis(lid, lb))   //AI(W906-MT-AXISLOCK) 20260929: say so instead of a "looping" ack for another axis
              return Refuse(r.action + " " + lid + ": 另一軸 " + g_loop.motorId + " 正在 LoopMove —— 一次只能跑一組來回，先停掉它（在那一軸按 LoopMove 或 STOP）"); }
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
        if (g_homes[i].active && !g_homes[i].fromLS && HomeOnAxis(g_homes[i], r.motors[0], m.a)) CancelHome(be, g_homes[i], true, "loop pressed (golden btnHome->Down=false)");   //AI(W906-MT-E3c) 20260925: not Light Scale's single home (bSingleHome, no button)  //AI(W906-MT-AXISLOCK) 20260929: only this axis' HOME
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

//AI(W906-MT-SPDLIVE) 20261001: EastSun 1001 (Motor Test, screenshot of Speed / Speed Adjust): 「當我百分比速度有變動時 請要
//  直接改速度 不然每次我案第二次 JOG 速度都不一樣」. oplog 21:10-21:12: a row-1 (InitialSpeed) edit to 5000 left the next
//  jogs at velLow=500 until the bar was wiggled (21:10:35 / 21:12:24) -- the jog re-sends g_jogSpd, the values of the LAST
//  scroll (MT-FIX1, = golden: a cell edit touches memory only). Changed from golden by that ruling: an edit that changes
//  what the speed is made of (InitialSpeed / JogHigh / JogLow / Acc / Dec, the JogHigh / JogLow buttons, Copy From)
//  re-applies the speed AT ONCE, at the motor's current percentages -- the jog family at the bar's (JogPctOf) and the PTP
//  family at the last SetSpeed's (PtpPctOf) -- so the card, g_jogSpd and the next jog all carry the new parameters.
//  MT-FIX1's concern (a silent re-derive at the press) still holds: the re-derive happens at the edit, and its ack says
//  the new values. Not while a HOME runs on the axis (golden scroll bar :796 `if(bSingleHome==true) return;`: the card's
//  PTP family is the home speed then). Returns the text for the ack ("" = nothing to say).
std::string ReapplySpeedAfterEdit(long long wireId, IMotorAccessBackend& be, const std::string& motorId)
{
    MotionCtx m;
    if (!be.Resolve(motorId, m.a) || m.a.motIndex < 0 || !be.GoldenMotor(m.a.motIndex, m.g)) return "";
    const int mi = m.a.motIndex;
    if (m.g.indexMotor) return "";                                              // golden TMyMotor::SetSpeed: empty for the Index four
    for (std::size_t i = 0; i < g_homes.size(); ++i)
        if (HomeOnAxis(g_homes[i], motorId, m.a)) return "; speed NOT re-applied: HOME running on this axis (golden :796 bSingleHome)";
    const int jogPct = JogPctOf(mi), ptpPct = PtpPctOf(mi);
    if (!m.a.Is1203()) {
        be.GoldenSetSpeed(mi, jogPct, true);                                    // golden SetSpeed(x, true) re-derives from the new parameters
        if (ptpPct != jogPct) be.GoldenSetSpeed(mi, ptpPct, false);
        return "; speed re-applied: MOT[" + std::to_string(mi) + "].SetSpeed(" + std::to_string(jogPct) + ", true)" +
               (ptpPct != jogPct ? " + SetSpeed(" + std::to_string(ptpPct) + ")" : std::string());
    }
    std::string w1;
    if (!m.a.tableEnable || !be.Pci1203Ready(w1) || m.a.axis < 0)
        return "; speed NOT re-applied (1203 axis not usable now" + (w1.empty() ? std::string() : ": " + w1) + ") -- the next scroll applies it";
    const Motor1203Speed spJog = MotorSpeedFromPct(jogPct, m.g), spPtp = MotorSpeedFromPct(ptpPct, m.g);
    std::string why, noteJ, noteP;
    if (!Send1203SpeedValues(be, wireId, m, jogPct, spJog, why, noteJ, kFamJog)) {
        //  same rule as DoSetSpeed's failure path (MT-FIX1 review low): keep the slower of the old and the new jog values
        std::map<int, Motor1203Speed>::const_iterator old = g_jogSpd.find(mi);
        if (old == g_jogSpd.end() || spJog.velHigh < old->second.velHigh) g_jogSpd[mi] = spJog;
        return "; speed re-apply FAILED (jog keeps the slower of old/new): " + why;
    }
    g_jogSpd[mi] = spJog;                                                       // what the next jog re-sends (DoJog)
    if (!Send1203SpeedValues(be, wireId, m, ptpPct, spPtp, why, noteP, kFamPtp))
        return "; jog speed re-applied (" + noteJ + "); PTP re-apply FAILED: " + why;
    be.GoldenSetSpeed(mi, ptpPct, false);                                       // the golden object's iSpeed, as DoSetSpeed
    return "; speed re-applied now -- jog " + noteJ + "; PTP " + noteP;
}

// ---- setParamCell：strngrdMotorSelectCell :1176-1222 ----
//   golden: `if(ACol!=1 || ARow==0) return; if(ActiveIndex==-1) return;` -> keypad (rows 8/9 N_DOUBLE + atof, the others
//   N_INTEGER + atoi, no range check) -> the row's setter (memory only, golden same: TMyEtherCatMotor::SetInitSpeed /
//   SetRange and HTMotor::Set*DataBase touch no card) -> btnLoopMove->Down=false -> HomeFlag=0 -> UpdateMotorParameter.
//   ⚠ golden writes the cell even when the keypad is cancelled (Edit2 still holds the old text) -- the page sends that too.
MotorAccessOutcome DoSetParamCell(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)   //AI(W906-MT-SPDLIVE) 20261001: + wireId
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
    //AI(W906-MT-SPDLIVE) 20261001: the rows MotorSpeedFromPct is made of -> the speed goes to the card now (EastSun 1001)
    const std::string spd = (row == 1 || row == 2 || row == 3 || row == 8 || row == 9) ? ReapplySpeedAfterEdit(wireId, be, r.motors[0]) : std::string();
    AckHead(w, r, "set", "MOT", std::string(b) + spd);
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
MotorAccessOutcome DoCopyFrom(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)   //AI(W906-MT-SPDLIVE) 20261001: + wireId
{
    MotionCtx m;
    std::string who, why;
    if (!PageMotor(r, be, m, who, why)) return Refuse(why);
    double sD = 0.0;
    if (!ParamNum(r, "source", sD)) return Refuse(who + ": 缺 source（golden cbbMotorName 第 2、3 個字元算出的編號）");
    char b[200];
    //AI(W906-MT-COPYALL) 20261003: EastSun「我複製M35的參數但是為啥沒作用」-- golden BitBtn1Click :1392 `Source>24 return` lets only
    //  M00..M24 be a source; HT9050 has M35..M42 (and M108), so the range is every MOT[] index (GoldenMotor below refuses a missing one).
    if (!(sD >= 0.0 && sD < 2560.0) || sD != (double)(int)sD) {                // range first: (int) of a huge double is undefined
        std::snprintf(b, sizeof(b), ": Source=%g —— 不是馬達編號（0~2559）", sD);
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
    AckHead(w, r, "copied", "MOT", std::string(b) + ReapplySpeedAfterEdit(wireId, be, r.motors[0]));   //AI(W906-MT-SPDLIVE) 20261001: JogHigh/JogLow changed
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
    bool homeHere = g_ls.homePending;                                           //AI(W906-MT-AXISLOCK) 20260929: golden bSingleHome was page-wide; now only a HOME on THIS axis keeps its speed (EastSun: axes independent)
    for (std::size_t i = 0; !homeHere && i < g_homes.size(); ++i) homeHere = HomeOnAxis(g_homes[i], r.motors.empty() ? std::string() : r.motors[0], m.a);
    if ((jog && homeHere) || m.g.indexMotor) {                                  // golden returns / does nothing
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
        be.GoldenSetSpeed(mi, pct, false);                                      // the golden object's iSpeed (ReadSpeed / lblRealSpeed); no card   //AI(W906-ENG1203) 20260929: "no card" holds only without the engine motor route (WB_ENGINE_MOTOR_1203 off); with it installed this also writes the PTP family through the route (same values; see WebMotorAccess.h GoldenSetSpeed)
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
        h.elapsedMs += ms;  if (h.kind != 0) { TickGoldenHome(be, h); continue; }   //AI(W906-MT-SMHOME) 20261001: a golden (non-1203) home = one golden MainProc pass per beat (csystem.cpp:16954-16960), none of the 1203 state below
        if (h.phase == 10) {                                                    //AI(W906-HOME-STEPLEAVE) 20261003: leaving the origin first (StartHomeWithLeave)
            std::string why;
            const int lv = MotorAccessStepperLeaveOrigin(be, h.motorId, why);
            if (lv == 0) continue;
            if (lv < 0) { be.GoldenSetHomeFlag(h.mi, 2); const int ami = h.mi; CancelHome(be, h, true, why + " -> HomeFlag=2"); be.GoldenMotorHomeAlarm(ami, why); continue; }   //AI(W906-HOME-STEPLEAVE) 20261003: + golden's motor jam note (EastSun「三次之後也沒叫異常」)
            MotionCtx mc;
            if (!be.Resolve(h.motorId, mc.a)) { be.GoldenSetHomeFlag(h.mi, 2); CancelHome(be, h, true, h.motorId + ": axis no longer resolves -> HomeFlag=2"); continue; }
            mc.g = h.g;
            const std::string mid = h.motorId; const bool ls = h.fromLS; const int mi = h.mi;
            h.active = false;                                                   // its slot takes the real home job (PushHomeJob reuses it: no reallocation)
            HomeStartInfo info;
            const std::string ref = StartHome1203(mid, -1, be, mc, "home " + mid + " (after leaving the origin)", ls, info);
            if (!ref.empty()) { be.GoldenSetHomeFlag(mi, 2); be.GoldenMotorHomeAlarm(mi, ref); }   //AI(W906-HOME-STEPLEAVE-3) 20261003 (review): the operator is told why the home did not start (was a console printf)
            continue;
        }
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
    CancelLoop(be, false, "FormClose (golden fShow=false: Timer1Timer no longer steps DoLoopMove)");  CancelGear(be, "Motor Test FormClose");   //AI(W906-GEARRATIO) 20261002: the page is gone -> the measurement session ends  //AI(W906-GEARRATIO2) 20261003: and its leg is stopped (NB2 R171 M1; the LoopMove above keeps golden's no-stop)
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
        return LsStartGoldenHome(be, mi, alias, m);   //AI(W906-MT-SMHOME) 20261001: golden LightScale case 0 (:1835-1840) = the same single home as btnHomeClick (fromLS); was a note citing the retired stub -- the next line is now unreachable
        return true;
    }
    if (FindHome(m.a.axis)) { g_ls.lastNote = "lightScale home " + alias + ": a single home already runs on this axis (it goes on)"; return true; }
    HomeStartInfo info;
    bool lsLeaving = false; const std::string refused = StartHomeWithLeave(alias, -1, be, m, "lightScale home " + alias, true, info, lsLeaving);   //AI(W906-HOME-STEPLEAVE) 20261003: every home (EastSun) -- was StartHome1203
    g_ls.lastNote = refused.empty() ? (lsLeaving ? "lightScale home " + alias + ": stepper on its origin -- moving off it first (1000 x up to 3), then the home"   //AI(W906-HOME-STEPLEAVE-3) 20261003 (review): info is empty while leaving
                                                 : "lightScale home " + alias + ": " + (info.issued ? "sent " : "accepted, NOT issued (dry) ") + info.wouldCall)
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
    if (g_teachEdt == v && g_teachEdtMi == ami) return;                          //AI(W906-TEACH-SPD-AXIS) 20261001: per axis (see TeachSpeedEvent)
    g_teachEdt = v;
    g_teachEdtMi = ami;
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
    { std::string bn; if (!on && be.BrakeHoldBeforeServoOff(be.AliasOfMotor(m.a.motIndex), bn)) be.SleepMs(100); }   //AI(W906-BRAKE-AXIS) 20260929: brake HOLD before the servo off (see DoServo)
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
//   不發明驅動呼叫，改成記下這一軸，之後相對移動／jog／目前位置顯示以編碼器位置為基準（CurrentUserPos）。回傳錯誤（空＝全部成功）。  [AI(W906-SVON-ENCSYNC) 20261004: 現在照 golden 在開伺服之後把命令位置寫成編碼器位置（ServoOnEncSync）；只有那一步沒送出時才記進 g_encBase]
std::string EndTeachJob(IMotorAccessBackend& be, long long wireId)
{
    std::string err;
    for (std::size_t i = 0; i < g_teachSet.m.size(); ++i) {
        const MotionCtx& m = g_teachSet.m[i];  bool synced = false;
        if (g_teachSet.servoOnAtEnd[i]) {
            std::string why;
            if (!TeachServo(be, wireId, m, true, why)) err += " " + why + ";"; else if (m.a.Is1203() && g_teachSet.servoOff[i]) { std::string sn; synced = ServoOnEncSync(be, wireId, m.a, sn); }   //AI(W906-SVON-ENCSYNC) 20261004: golden ServoOnOff(true) after ShowModal = PCIL132_ResetPos on a PServoAlarmOn axis
        }
        if (m.a.Is1203() && g_teachSet.servoOff[i] && m.g.servoAlarmOn && !synced) g_encBase.insert(m.a.axis);   //AI(W906-SVON-ENCSYNC) 20261004: only when the sync did not go out
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
        { JogRec j = { false, -1, m.a.motIndex, r.motors[0], r.source }; ForgetJog(false, -1, m.a.motIndex); g_jogs.push_back(j); }
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
    { JogRec j = { true, m.a.axis, m.a.motIndex, r.motors[0], r.source }; ForgetJog(true, m.a.axis, m.a.motIndex); g_jogs.push_back(j); }
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
    //AI(W906-HOME-PERAXIS) 20261003: EastSun「我按X回HOME 並且在回HOME過程中再去按Y X回HOME動作會停止 不是應該各歸各地嗎?」(ruling 20260929:
    //  axes independent). golden releases btnHome with StopAllMotor (:2194) -- one home at a time; a 1203 axis now stops ITS OWN home only
    //  (CancelJobsOnAxis + DecStop), every other axis keeps homing. STOP (btnStop) still stops everything. Galil / non-1203: golden as before.
    const bool perAxis = hasM && !g.galilIndex && a.Is1203();
    if (!perAxis) CancelAllJobs(be, "HOME released (golden btnHomeClick else: btnHome up, motors stopped)");
    if (perAxis) {
        CancelJobsOnAxis(be, r.motors[0], a, "HOME released on this axis (AI(W906-HOME-PERAXIS): the other axes' homes go on)");
        ready = be.Pci1203Ready(why1203);
        if (ready && a.axis >= 0) {
            const Pci1203CmdResult res = Stop1203(be, a.axis, wireId);
            sent = 1;
            if (CmdFailed(res)) { refused = 1; firstRefusal = CmdWhy(res); } else accepted = 1;
        }
        msg = ready ? std::string("HOME 抬起：只停 ") + r.motors[0] + "（各軸獨立，其他軸的回原點照常進行）" + (refused ? "；停止被拒：" + firstRefusal : std::string())
                    : std::string("HOME 抬起：1203 control not ready（") + why1203 + "）";
    } else if (hasM && g.galilIndex) {                                          // golden :2190-2192
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
                    return Refuse(r.action + "（uteach）" + r.motors[0] + ": HOME 抬起：停止已送（1203 軸：只停這一軸，AI(W906-HOME-PERAXIS)），但 golden MOT.SetSpeed(1) 失敗 —— " + why +
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
    //AI(W906-TEACH-SVON) 20261001: EastSun 1001 (screenshot of the hand-teach dialog): 「我這邊預設值是 激磁教導 如果要手動教導
    //  旁邊請新增勾選 是否手動教導」. Changed from golden (SetButton*Click ALWAYS does servo off -> fTeachShow -> GetTechPos -> servo
    //  on): the page sends params.hand (its 手動教導 checkbox). hand false / absent = 激磁教導: the checks above (motor, HomeFlag)
    //  as before, NO servo off, NO dialog -- GetTechPos at once (the same read as the dialog's OK, with the servo staying on)
    //  into EditPtr; golden GetTechPos :3476 (ActiveMotorIndex==-1 -> nothing written) kept. hand true = golden's hand teach below.
    {
        std::map<std::string, bool>::const_iterator hf = r.flag.find("hand");
        if (hf == r.flag.end() || !hf->second) {
            std::vector<int> pos;
            std::string err;
            const bool write = job.amiValid;
            if (write)
                for (std::size_t i = 0; i < job.m.size(); ++i) {
                    int p = 0;
                    if (!TeachTechPos(be, job.m[i], job.kind == kTkSet140, p)) err += " 第 " + std::to_string(i + 1) + " 軸位置讀不到;";
                    pos.push_back(p);
                }
            if (!err.empty()) return RefuseT(who + ": 激磁教導:" + err + "（位置沒有寫進欄位）", ec);
            return TeachOk(r, write ? "taught" : "taughtNothing",
                           write ? "激磁教導（伺服保持開）：讀編碼器位置寫進欄位 —— W906，EastSun 1001；勾「手動教導」才關伺服用手推"
                                 : "golden GetTechPos：ActiveMotorIndex==-1 → return（SetButton020／064 不設 ActiveMotorIndex，按的時候頁面沒有選馬達）—— 什麼都不寫",
                           write ? &pos : 0, true, false, &ec);
        }
    }
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
    //AI(W906-TEACH-SPD-AXIS) 20261001: a change OR another axis. The teach audit (oplog 10-01 20:21:36-47, 21:55:38): g_teachEdt was ONE
    //  value for the whole page, so after picking another axis a speed equal to the previous axis' was "no change" and that axis' jog
    //  family was never written -- MInArmX jogged at 1% with the bar at 100, MInArmY at Motor Test's 100% while Teach showed 1.
    //  golden's UpdateMotorTeachMonitor puts ScrollBar1 at 1 on every pick, so its next move IS a change; per axis here = the jog
    //  family always matches what the page shows for the selected axis (EastSun 1001 「每次我使用不是被鎖住 就是沒有功能」).
    const bool changed = (g_teachEdt != s || g_teachEdtMi != ami);              //AI(W906-MERGE-56bbf785) 20260926: golden OnChange fires only on a change
    g_teachEdt = s;
    g_teachEdtMi = ami;
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
bool (*g_W906OrgActiveLow)() = 0;   //AI(W906-HT9050-ORG) 20261001: 見 WebMotorAccess.h（HT9050＝1203 ORG low 才是在原點）
bool g_W906OrgInvert = false;       //AI(W906-ORG-INV) 20261002: 見 WebMotorAccess.h（MachineType.h W906_HT9050_ORG_INVERT）

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
    //AI(W906-HT9050-ORG) 20261001: HT9050＋1203 分支 —— EastSun「偵測全部都要改用1203回來的home訊號 不是用IO了，home 是 high 時 代表沒偵測到、
    //  是 low 時 代表有偵測到」。卡片 ORG 位元（motionIO bit 4）當成 /Home 原始準位；不看 kPci1203CardOrgLogic（不明＝不在原點那條）。
    //  沒樣本／命令後沒新樣本照上面兩行仍是 -1（fail-closed）。掛勾由 WebMotorAccessLive.cpp 依 W906_GpibModel=="9050GPIB" 裝上。
    //AI(W906-HT9050-ORG-ST) 20261002 (NB2, 使用者 1002 16:1x「照該軸 SensorType 決定」): 逐軸＝golden InitMotor :492 `uOrgLogic=bSensorType?0:1`——SensorType=1 ⇒ ORG 0（low）＝在原點，0 ⇒ ORG 1（high）＝在原點；前提與 Mot_Table 要一起改的事見 docs/RD5軟體_HT9050_1203原點極性與回HOME_20261002_163000.md
    MotorGolden g;  if (!be.GoldenMotor(mi, g)) { g_teachHomeUnknown = alias + "：沒有 golden 馬達物件（讀不到 SensorType）"; return -1; }
    if (g_W906OrgActiveLow && g_W906OrgActiveLow()) { const bool orgHigh = (io & 0x00000010ul) != 0; const bool home = g.sensorType ? !orgHigh : orgHigh; return (home != g_W906OrgInvert) ? 1 : 0; }   //AI(W906-HT9050-ORG-ST) 20261002; AI(W906-ORG-INV) 20261002: != g_W906OrgInvert
    // AX_MOTION_IO_ORG（golden myEthercatmotor.cpp:860 ScanMotorStatus `(Status>>4)&0x1`）；極性見 kPci1203CardOrgLogic（W5B-R2）
    const int led = MotorAccessOrgHomeLed((io & 0x00000010ul) != 0, g.sensorType, kPci1203CardOrgLogic);
    if (led < 0)
        g_teachHomeUnknown = alias + "：EastSun 監看器開軸沒有設 CFG_AxOrgLogic（golden InitMotor myEthercatmotor.cpp:252 依 SensorType 設），"
                                     "ORG 位元的極性還沒在機台上量（kPci1203CardOrgLogic=-1）";
    return led;
}

//AI(W906-HOME-STEPLEAVE) 20261003: see WebMotorAccess.h. The SW3D-680 stepper drive started ON its origin keeps searching forward
//  (M35 1003 20:34: 54 s, -9859 -> -60761, never ended) where the SGDXW servo backs off first (M3). Shared by DoHome / TickHomes (single
//  HOME) and the engine's TMyMotor::MotorHome (full HOME, through W906_PreHomeHook in WebMotorAccessLive.cpp).
namespace { struct StepLeave { bool active; bool moving; int tries; StepLeave() : active(false), moving(false), tries(0) {} };
            std::map<std::string, StepLeave> g_stepLeave; const int kStepLeavePulses = 1000; const int kStepLeaveTries = 3; }
void MotorAccessStepperLeaveReset(const std::string& motorId) { g_stepLeave.erase(motorId); }
int MotorAccessStepperLeaveOrigin(IMotorAccessBackend& be, const std::string& motorId, std::string& why)
{
    MotorAccessAxis a;
    if (!be.Resolve(motorId, a) || !a.Is1203() || a.axis < 0 || !a.tableEnable) { g_stepLeave.erase(motorId); return 1; }
    if (!be.Pci1203IsStepperDrive(a.axis)) { g_stepLeave.erase(motorId); return 1; }   // servo drives back off by themselves
    StepLeave& s = g_stepLeave[motorId];
    if (s.moving) {                                                             // wait for the relative move: a READY sample after it
        if (SampleStale(be, a.axis)) return 0;
        unsigned st = 0;
        if (!be.Pci1203AxisState(a.axis, st)) return 0;
        if (IsErrorState(st)) { g_stepLeave.erase(motorId); why = motorId + "：離開原點的移動中 ERROR_STOP"; return -1; }
        if (!IsReadyState(st)) return 0;
        s.moving = false;
    }
    const int org = MotorAccessTeachHomeLed(be, a.motIndex);                    // the same origin rule as Teach / the engine (HT9050 SensorType)
    if (org == -1) return 0;                                                    // no fresh sample: wait for one (review: returning 1 skipped the leave and brought back the endless forward search)
    if (org != 1) { g_stepLeave.erase(motorId); return 1; }                     // not (any more) on the origin: home now
    if (s.tries >= kStepLeaveTries) {
        g_stepLeave.erase(motorId);
        char b[200];
        std::snprintf(b, sizeof(b), "%s：步進軸在原點上，往回原點的反方向移 %d 共 %d 次仍沒有離開原點（EastSun 1003 規則）", motorId.c_str(), kStepLeavePulses, kStepLeaveTries);
        why = b;
        return -1;
    }
    MotorGolden g;
    if (!be.GoldenMotor(a.motIndex, g)) { g_stepLeave.erase(motorId); why = motorId + "：沒有 golden 馬達物件（讀不到回原點方向）"; return -1; }
    const unsigned run = (g.homeHigh && g.homeLow) ? (g.homeHigh < g.homeLow ? g.homeHigh : g.homeLow) : (g.homeHigh ? g.homeHigh : g.homeLow);
    if (run == 0) { g_stepLeave.erase(motorId); why = motorId + "：回原點速度是 0，不移動"; return -1; }
    const double init = (g.initSpeed && g.initSpeed < run) ? (double)g.initSpeed : (double)run;
    { MotionCtx mc; mc.a = a; mc.g = g; std::string cw;                       //AI(W906-HOME-MAXVEL-SINGLE) 20261003: the leave move runs at a home speed too (M30 0x80000081)
      if (!RaiseCeilingForHome(be, -1, mc, (double)run, g.accDb, g.decDb, cw)) { g_stepLeave.erase(motorId); why = motorId + "：" + cw; return -1; } }
    const struct { Pci1203SpeedParam which; double v; } sp[4] = { { kSpeedInit, init }, { kSpeedRun, (double)run }, { kSpeedAcc, g.accDb }, { kSpeedDec, g.decDb } };
    for (int i = 0; i < 4; ++i) {
        Pci1203Cmd c; c.kind = kCmdAxSetSpeed; c.axis = a.axis; c.speed = sp[i].which; c.value = sp[i].v;
        const Pci1203CmdResult r = be.Pci1203Execute(c);
        if (CmdFailed(r)) { g_stepLeave.erase(motorId); why = motorId + "：離開原點前設定速度失敗 —— " + CmdWhy(r); return -1; }
    }
    Pci1203Cmd c; c.kind = kCmdAxMoveRel; c.axis = a.axis;
    //AI(W906-HOME-STEPLEAVE) 20261003: EastSun「往回走1000 方向寫反了」「你的1000 也是要跟MOT_TABLE 歸原點方向跟著改變」-- the 1000 follows
    //  Mot_Table HomeDirection: HomeDirection 1 -> +1000, 0 -> -1000 (was the opposite)
    c.value = g.homeDirection ? (double)kStepLeavePulses : -(double)kStepLeavePulses;
    const Pci1203CmdResult r = be.Pci1203Execute(c);
    if (CmdFailed(r)) { g_stepLeave.erase(motorId); why = motorId + "：離開原點的移動失敗 —— " + CmdWhy(r); return -1; }
    NoteIssued(be, a.axis);
    s.active = true; s.moving = true; ++s.tries;
    return 0;
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
    if (ArmCellActive()) { why = ArmCellStartBlockedWhy(); return true; }  if (!g_teachSet.active) return false;   //AI(W906-ARMCELL) 20261002: START refused while the Teach Arm Cell job runs
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
    //   golden 不送停止（進行中的那一段走完），這裡同樣不送。  //AI(W906-GEARRATIO2) 20261003: 例外＝Gear Ratio 量測段（golden 沒有這個功能）：CancelAllJobs → CancelGear 會停它自己那一軸（NB2 R171 M1，檔尾）
    if (AnyJobActive()) {
        if (!operatorConnected)
            CancelAllJobs(be, "operator connection gone (golden Timer1Timer: fShow==false -> no further steps)");
        else if ((g_lastSafeLock = be.GoldenSafeLockActive()))                //AI(W906-MT-FIX1) 20260926: remembered for the post-Poll step
            CancelAllJobs(be, "safe lock active (golden Timer1Timer: IsSafeLockCheck() -> Close())");
    } else {
        g_lastSafeLock = false;
    }
    TickHomes(be, elapsedMs);  TickArmCell(be);   //AI(W906-ARMCELL) 20261002: the Teach Arm Cell job, after the homes (the 500 ms beat; EOF)
    TickLoop(be);                                                               //AI(W906-MT-E2) 20260925: times from LoopNow(), not elapsedMs
    TickPowerOn(be);  GearHandTick(be);                                         //AI(W906-MT-E3c) 20260925: Motor Power On's 1 s (cross-tick DoMotorPowerOn)  //AI(W906-GEARHAND) 20261003: + the Gear Ratio hand session (servo state, E1, its ends)
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
    ArmCellPollTick(be);  bool loop1203 = g_loop.active && g_loop.a.Is1203();   //AI(W906-ARMCELL-ZORG) 20261003: NB2 R174 M1 -- the Teach Arm Cell job's Z origin watch on the fresh sample too (S3 only; first dark sample stops X / Y, EOF ArmCellPollTick), after the gate above. Same line, nothing moves
    //AI(W906-MT-E3c) 20260925: an All-mode loop with any 1203 axis, Light Scale and the power-on job step here too (a 1203
    //  arrival is seen on the fresh sample; golden Timer1 is 5 ms, Timer2 10 ms).
    for (std::size_t k = 0; !loop1203 && g_loop.active && g_loop.all && k < g_loop.axes.size(); ++k) loop1203 = g_loop.axes[k].a.Is1203();
    if (loop1203) TickLoop(be);
    TickPowerOn(be);  GearHandTick(be);   //AI(W906-GEARHAND) 20261003: the Gear Ratio hand session on the fresh sample (E1 needs a sample after the servo on)
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
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (g_homes[i].active && !g_homes[i].fromLS && g_homes[i].axis == a.axis) return 1;  if (ArmCellUsesAxis(a.axis)) return 1;   /*AI(W906-ARMCELL) 20261002: the Teach Arm Cell job's axes, between its steps too*/   //AI(W906-MT-AXISLOCK) 20260929: this axis' HOME job (between its steps the axis may sit READY)
    if (SampleStale(be, a.axis)) return 1;                                      // a command went out, no fresh sample yet
    unsigned st = 0;
    if (!be.Pci1203AxisState(a.axis, st)) return 0;                             // no valid sample: unknown -> not locking on a monitor hiccup
    if (IsErrorState(st) || (st & 0xFFu) == 0u) return 0;                       //AI(W906-MT-AXISLOCK) 20260929: ERROR_STOP (alarm) / DISABLE are STOPPED -- EastSun 20260929: golden MotionDone (state==READY) made one alarmed axis lock every axis for ever
    return IsReadyState(st) ? 0 : 1;                                            // golden TMyEtherCatMotor::MotionDone: state==STA_AX_READY
}
int MotorAccessAxisLock(IMotorAccessBackend& be, int motIndex, std::string& why)
{
    why.clear();
    const std::string alias = be.AliasOfMotor(motIndex);
    if (alias.empty()) return -1;
    MotorAccessAxis a;
    if (!be.Resolve(alias, a) || !a.Is1203()) return -1;
    if (a.axis < 0) return 0;
    for (std::size_t i = 0; i < g_jogs.size(); ++i) if (g_jogs[i].is1203 && g_jogs[i].axis == a.axis) { why = "JOG"; return 1; }
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (g_homes[i].active && g_homes[i].axis == a.axis) { why = g_homes[i].fromLS ? "HOME (Light Scale)" : "HOME"; return 1; }  if (ArmCellUsesAxis(a.axis)) { why = "ARM CELL"; return 1; }   //AI(W906-ARMCELL) 20261002
    if (SampleStale(be, a.axis)) { why = "command sent"; return 1; }
    unsigned st = 0;
    if (!be.Pci1203AxisState(a.axis, st)) return 0;
    const unsigned s = st & 0xFFu;
    if (s == 0u || s == 1u || s == 3u) return 0;                                // DISABLE / READY / ERROR_STOP: not moving
    static const char* const kSt[16] = { "DISABLE", "READY", "STOPPING", "ERROR_STOP", "HOMING", "PTP_MOT", "CONTI_MOT", "SYNC_MOT",
                                         "EXT_JOG", "EXT_MPG", "PAUSE", "BUSY", "WAIT_DI", "WAIT_PTP", "WAIT_VEL", "EXT_JOG_READY" };   // Pci1203AxisStateText's spelling
    why = std::string("moving (") + (s < 16u ? kSt[s] : std::to_string(s).c_str()) + ")";
    return 1;
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
    if (be.Pci1203Ready(why)) {  MotorAccessAlarmSweep sweep;   //AI(W906-INDEXZ-1203) 20260930: review round 2 D2 -- the stops below are the alarm sweep's (MotorAccessInAlarmSweep, EOF): the Index Z1 Gali route keeps a halted Z1 job halted on them. Same line, no line moves
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
    j.encBaseAxes = (int)g_encBase.size();  ArmCellFillJobs(j);  GearFillJobs(j);   //AI(W906-ARMCELL) 20261002  //AI(W906-GEARRATIO) 20261002
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
    g_teachEdtMi = -1;                                                          //AI(W906-TEACH-SPD-AXIS) 20261001
    g_teachHomeUnknown.clear();  TeachZAllUpResetState();  ArmCellResetState();  GearResetState();   /*AI(W906-ARMCELL) 20261002*/ /*AI(W906-GEARRATIO) 20261002*/   //AI(W906-TEACH-ZALLUP) 20261001: the Index Servo buttons' golden statics
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
static MotorAccessOutcome DoSaveMotTable(const MotorAccessReq& r, IMotorAccessBackend& be);   //AI(W906-MT-SAVEMOT) 20260930: body at this file's EOF. Was a blank line: no line below moves
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
    { const std::string acBusy = ArmCellBusyWhy(r, be); if (!acBusy.empty()) return Refuse(acBusy); }  /*AI(W906-ARMCELL) 20261002: while the Teach Arm Cell job runs (EOF)*/  if (r.source == "uteach") TeachSpeedEvent(r, be);                           // AI(W906-W5-b) 20260925: R-W5B-4（頁面報的 golden 速度事件）
    if (r.action == "servoToggle") return DoServo(r, wireId, be);
    // W5-b：uteach 的運動鈕走 golden uteach 的互鎖（CheckCanMove／IsCanQuickJogMove／軟體極限吃目前位置…）
    if (r.source == "uteach") {
        if (r.action == "jogP" || r.action == "jogN") return DoTeachJog(r, wireId, be);
        if (r.action == "moveRelative")                 return DoTeachMoveRel(r, wireId, be);
        if (r.action == "moveAbsolute")                 return DoTeachMoveTo(r, wireId, be);
        if (r.action == "home")                         return DoTeachHome(r, wireId, be);
        if (r.action == "teachGo")                      return DoTeachButton(r, wireId, be);   // golden 按鈕的處理函式決定做什麼（W5B-2）
        if (r.action == "teachSet")                     return DoTeachSet(r, wireId, be);  if (r.action == "teachZAllUp") return DoTeachZAllUp(r, wireId, be);  if (r.action == "teachIndexServo") return DoTeachIndexServo(r, wireId, be);   if (r.action == "moveToTrayCell") return DoArmCell(r, wireId, be);  if (r.action == "teachSt02") { MotorAccessOutcome TeachSt02Dispatch(const MotorAccessReq&, long long, IMotorAccessBackend&); return TeachSt02Dispatch(r, wireId, be); }  /*AI(W906-ARMCELL) 20261002: the Teach Arm Cell tab (EOF)*/  //AI(W906-TEACH-ZALLUP) 20261001: golden uteach btnIn/OutZAllUp, btnZ1/Z2/Arm1Y/Arm2YServo (EOF) ‖ AI(W906-ST02-C9-G2) 20261002 (St02-E helper): teachSt02 -- body at this file's EOF (WebTeachSt02.h); after the run / manual-teach gates above. Same line, no line moves
    }  if (r.action == "teachZAllUp" || r.action == "teachIndexServo") return Refuse("motor.access " + r.action + "（" + r.source + "）: 這是 golden uteach 教導頁的按鈕（" + row->golden + "），MotorTest 沒有");  if (r.action == "moveToTrayCell") return Refuse("motor.access moveToTrayCell（" + r.source + "）: 這是教導頁 Arm Cell 分頁的動作（RULINGS_20261002 第 18 條），MotorTest 沒有");  if (r.action == "gearCalMove" || r.action == "gearRatioPreview" || r.action == "gearRatioSave") return DoGear(r, wireId, be);  if (r.action == "teachSetAllArmZ" || r.action == "teachOutZAllDown") { MotorAccessOutcome TeachZDownDispatch(const MotorAccessReq&, long long, IMotorAccessBackend&); return TeachZDownDispatch(r, wireId, be); }   //AI(W906-TEACH-ZALLUP) 20261001  //AI(W906-GEARRATIO) 20261002: the Motor Test Gear Ratio tab (EOF; DoGear refuses any source but uMotorTest)  //AI(W906-TEACH-ZDOWN) 20261003: INBOX 147 golden uteach btnSetAllIn/OutArmZ, btnOutZAllDown (EOF; after the dispatcher's run / hand-teach / Arm Cell gates; a Motor Test source is refused there)
    if (r.action == "gearHandBegin") return DoGear(r, wireId, be);  if (r.action == "jogP" || r.action == "jogN") return DoJog(r, wireId, be);   //AI(W906-GEARHAND) 20261003: the Gear Ratio hand-push session (RULINGS_20261003 #8; DoGear refuses any source but uMotorTest)
    if (r.action == "moveRelative")                 return DoMoveRelative(r, wireId, be);
    if (r.action == "moveAbsolute")                 return DoMoveAbsolute(r, wireId, be);
    if (r.action == "moveSoftLimitP" || r.action == "moveSoftLimitN") return DoMoveSoftLimit(r, wireId, be);
    if (r.action == "home")                         return DoHome(r, wireId, be);
    if (r.action == "loopMove")                     return DoLoopMove(r, wireId, be);
    if (r.action == "resetMNet")                    return DoResetMNet(r, be);
    if (r.action == "resetAlarm")                   return DoResetAlarm(r, wireId, be);   //AI(W906-MT-ALMRST) 20260929
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
    if (r.action == "selectMotor" || r.action == "setParamCell" || r.action == "copyFrom" || r.action == "setSpeed" || r.action == "saveMotTable") {   //AI(W906-MT-SAVEMOT) 20260930: + saveMotTable (Motor Test only)
        if (r.source != "uMotorTest")
            return Refuse("motor.access " + r.action + "（" + r.source + "）: 這是 golden uMotorTest 的畫面動作（" + row->golden + "），教導頁沒有");
        if (r.action == "selectMotor")  return DoSelectMotor(r, wireId, be);
        if (r.action == "setParamCell") return DoSetParamCell(r, wireId, be);   //AI(W906-MT-SPDLIVE) 20261001: + wireId
        if (r.action == "copyFrom")     return DoCopyFrom(r, wireId, be);  if (r.action == "saveMotTable") return DoSaveMotTable(r, be);   //AI(W906-MT-SAVEMOT) 20260930
        return DoSetSpeed(r, wireId, be);
    }
    if (r.action.compare(0, 3, "set") == 0 &&
        (r.action.find("Speed") != std::string::npos || r.action.compare(0, 12, "setSoftLimit") == 0))
        return DoSetParam(r, wireId, be);   //AI(W906-MT-SPDLIVE) 20261001: + wireId
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

//AI(W906-WSLINK-B) 20260929: St01 13:32 safety point (b) (TO_STEVEN §4 20260929 14:2x; St01 14:31 "laptop does (b)").  With one WebSocket
//  per browser (St01 ht9045_link.js) the page-table close edge of a window is the only server-side event that says "the
//  operator closed Motor Test / Teach" -- the frame hides the iframe (background.html closeWin: display:none), no connection
//  drops, and HW.teach.html sends nothing on Exit.  wb_serve's window-edge hook (W906_WindowEdgeRegister, WebTeachLeave.h;
//  about 0.5 s after the frame reports the close; registered in WebMotorAccessLive.cpp W906_MotorAccessPageEdges) calls
//  this, and it runs the C++ half of that window's golden FormClose:
//    fMotorTest -- golden TfMotorTest::FormClose (uMotorTest.cpp:1347-1362) = DoFormClose, the same thing the page's own
//      formClose already does (HW.MotorTest.html onHtWin -> formClosePage): fShow=false, LoopMove ends without a stop, HOME
//      and Light Scale keep going (DoFormClose's rules, unchanged).  Idempotent: the page's formClose may come first or
//      after.  Plus the web-only part: Motor Test's own jogs are stopped (the page can go away with a button held; golden
//      cannot close a form while its button is pressed).
//    fTeach -- golden TfTeach::FormClose (uteach.cpp:2076-2095): :2092 btnStop->Click() = btnStopClick :2948-2954
//      (StopAllMotor; Tech_Part=0; MOT[MTestY1].Gali_Command("ST"); AllBtnUp) = DoStop with source "uteach" (every
//      axis stopped, the button jobs cancelled, every jog released) -- plus Light Scale's single home is cancelled WITHOUT
//      releasing its case 1 (see the fTeach branch; re-review of a4581654 #1).  :2089-2090 bInArmXPitch_40mm / bOutArmXPitch_40mm = false are
//      done by the Live callback (globals).  Not done, missing dependencies (not a choice): Tech_Part=0 (the TfTeach facade
//      has no member, forms/fTeachPara.h:207; DoStop's GoldenStopAll says so too) and :2093 Set_Pitch_SetGroup (not
//      translated, forms/fTeach.h:398 [MOTION]).  While SystemStart: golden cannot get here (main.cpp:27827 sbTeachingClick
//      `if(SystemStart) return;` -- Teach cannot be open during a run); the web can, and a StopAllMotor from a closed page
//      would stop production with no alarm, so only Teach's own jogs are stopped and one line is printed (the S122 R82=A
//      rule for the same edge).
//  Known limits (the window registry's multi-connection rules, WebTeachLeave.h:150-152, review wf_f287d525-ed3 #3/#4): a
//  second HMI connection that still reports the same window open masks the close; a stale report next to a fresh one can
//  produce an extra close.  With the page table armed, F5 / closing the whole browser is ALSO a close edge of every open
//  window (WebTeachLeave.cpp WindowEdgeSample -> page table), so this runs too (Teach open -> the full golden STOP), besides
//  the connection dead-man when the socket drops -- the stopping side, no screen-present test (re-review #4).
namespace ht9045 {
static int StopJogsFrom(IMotorAccessBackend& be, const char* src, int& refused)
{
    //AI(W906-WSLINK-B) 20260929: re-review #2/#3 -- take the page's records out BEFORE any stop is sent: a Galil ST that fails
    //  raises ShowErrorMessage -> W906_MotorAccessOnAlarm -> g_jogs.clear() on this same thread, and an erase after that would
    //  walk an emptied vector.  A 1203 stop the card refuses keeps its record (the dead-man / a later STOP can retry it).
    std::vector<JogRec> mine, keep;
    for (std::size_t i = 0; i < g_jogs.size(); ++i) (g_jogs[i].source == src ? mine : keep).push_back(g_jogs[i]);
    g_jogs.swap(keep);
    refused = 0;
    for (std::size_t i = 0; i < mine.size(); ++i) {
        if (mine[i].is1203) {
            const Pci1203CmdResult res = Stop1203(be, mine[i].axis, -1);      // the dead-man's stop (MotorAccessTick)
            if (CmdFailed(res)) { ++refused; g_jogs.push_back(mine[i]); std::printf("[WSLINK-B] jog stop REFUSED on 1203 axis %d (%s): %s\n", mine[i].axis, mine[i].motorId.c_str(), CmdWhy(res).c_str()); }
        } else {
            be.GoldenStopMotor(mine[i].mi);
        }
    }
    return (int)mine.size() - refused;
}

void MotorAccessPageClosed(IMotorAccessBackend& be, const char* form)
{
    const std::string f = (form != 0) ? form : "";
    if (f == "fMotorTest") {
        int refused = 0;
        const int jogs = StopJogsFrom(be, "uMotorTest", refused);
        const bool loopWas = g_loop.active, shownWas = g_mtShown;
        if (shownWas || loopWas) {
            MotorAccessReq r = MotorAccessReq();
            r.source = "uMotorTest"; r.action = "formClose"; r.button = "PageTableCloseEdge"; r.kind = "control";
            DoFormClose(r, be);                                                 // golden TfMotorTest::FormClose, C++ half
        }
        if (jogs != 0) g_lastJobNote = "jog stopped: fMotorTest closed (page-table close edge, " + std::to_string(jogs) + " axis)";
        std::printf("[WSLINK-B] fMotorTest closed on the HMI: %d Motor Test jog(s) stopped%s%s\n", jogs, refused ? " (some REFUSED, kept)" : "",
                    (shownWas || loopWas) ? "; golden FormClose (fShow=false, LoopMove ended without a stop, HOME / Light Scale keep going)" : "");
    } else if (f == "fTeach") {
        if (be.GoldenSystemStart()) {
            int refused = 0;
            const int jogs = StopJogsFrom(be, "uteach", refused);
            std::printf("[WSLINK-B] fTeach closed while SystemStart: %d Teach jog(s) stopped; golden TfTeach::FormClose STOP not run "
                        "(golden cannot close Teach during a run, main.cpp:27827; S122 R82=A)%s\n", jogs, refused ? "; some jog stops REFUSED, kept" : "");
        } else {
            //AI(W906-WSLINK-B) 20260929: re-review #1 -- a Light Scale single home (fromLS) survives DoStop (CancelAllJobs leaves it;
            //  EndSingleHome runs for uMotorTest only) while Stop1203All stops its axis, and TickHomes would then read READY-after-HOMING
            //  as done: HomeFlag=1 (card homes also zero the position) on an axis stopped mid-home.  Golden: TfTeach::FormClose sets
            //  fShow=false (uteach.cpp:2078) before btnStop->Click() (:2092), and the single home is stepped only while Motor Test or
            //  Teach is shown (port csystem.cpp MainProc) -- so it never finishes, HomeFlag stays 0 and Light Scale waits at case 1
            //  (bSingleHome stays true).  Same here: cancel that job BEFORE the stop, keep g_ls.homePending.
            int lsHomes = 0;
            for (std::size_t i = 0; i < g_homes.size(); ++i)
                if (g_homes[i].active && g_homes[i].fromLS) { CancelHome(be, g_homes[i], false, "Teach closed (golden: fShow=false -> the single home is no longer stepped; Light Scale waits at case 1)"); ++lsHomes; }
            MotorAccessReq r = MotorAccessReq();
            r.source = "uteach"; r.action = "stop"; r.button = "btnStop"; r.kind = "control";
            const MotorAccessOutcome o = DoStop(r, -1, be);                     // golden uteach.cpp:2092 btnStop->Click()
            const bool partial = o.ackJson.find("\"partial\":true") != std::string::npos;
            std::printf("[WSLINK-B] fTeach closed on the HMI: golden TfTeach::FormClose -> btnStop->Click() (StopAllMotor, MTestY1 ST, AllBtnUp) -- %s%s%s\n",
                        o.ok ? "ok" : "NOT ok", partial ? ", PARTIAL (a 1203 stop was refused)" : "",
                        lsHomes ? "; Light Scale single home cancelled, HomeFlag left 0, Light Scale waits at case 1" : "");
            if (!o.ok || partial) std::printf("[WSLINK-B]   stop ack: %.300s\n", o.ackJson.c_str());
        }
    }
    std::fflush(stdout);
}
}  // namespace ht9045

//  AI(W906-INDEXZ-1203) 20260929: the per-axis ledger above (g_issuedPoll, W4B-5), exported for the Index Z1
//  Gali_* -> 1203 route (EtherCAT/Pci1203GaliRoute.cpp; declared in EtherCAT/Pci1203GaliRoute.h). One ledger for
//  both writers of the same 1203 axis: the route's "moving" answer waits for a sample newer than a Motor Test
//  command, and Motor Test's SampleStale waits for one newer than a route command.
//  AI(W906-INDEXZ) 20260930: re-applied for the INBOX 113 redo (unchanged; the route is OFF by default).
namespace ht9045 {
void MotorAccessNoteIssuedAt(int axis1203, unsigned long pollCount)
{
    g_issuedPoll[axis1203] = pollCount;  ++g_issuedSeq[axis1203];   //AI(W906-GEARRATIO) 20261002: the route's commands count too (the Gear Ratio session)
}
bool MotorAccessLastIssued(int axis1203, unsigned long& pollCount)
{
    std::map<int, unsigned long>::const_iterator it = g_issuedPoll.find(axis1203);
    if (it == g_issuedPoll.end()) return false;
    pollCount = it->second;
    return true;
}
}  // namespace ht9045

//  AI(W906-INDEXZ-1203) 20260930: review round 2 D7 -- g_issuedPoll (above) is tick-thread-only, like everything in this
//  file: Motor Test's own NoteIssued / SampleStale run in the WS dispatch and MotorAccessTick on the tick loop, and the
//  Index Z1 route calls the two exports above only on its owner thread (the same loop: TGaliRouteCore reads the ledger
//  only there, and EtherCAT/Pci1203GaliRoute.cpp's LiveIo refuses both on any other thread -- ctest GaliRouteCore part M,
//  GaliRouteLive). So the map needs no lock; a second thread must not be given one of these calls.
//  AI(W906-INDEXZ-1203) 20260930: review round 2 D2 -- the alarm-sweep scope (WebMotorAccess.h).
namespace ht9045 {
namespace { int g_alarmSweep = 0; }
bool MotorAccessInAlarmSweep() { return g_alarmSweep > 0; }
MotorAccessAlarmSweep::MotorAccessAlarmSweep()  { ++g_alarmSweep; }
MotorAccessAlarmSweep::~MotorAccessAlarmSweep() { --g_alarmSweep; }
}  // namespace ht9045

// =============================================================================
//  AI(W906-MT-SAVEMOT) 20260930: saveMotTable -- Motor Test「回寫 Mot_Table」(EastSun 20260930「圖片上也幫我新增回寫至 MOT_table的按鈕」,
//  the button next to the "<Alias> Settings" grid / Copy From). NOT golden (see WebMotorAccess.h). In order:
//   1. the motor = the request's motor with golden ActiveIndex rules (PageMotor: none selected / not in the table / no MOT object /
//      not selectable -> refused). uMotorTest only; refused while SystemStart like every Motor Test button (dispatcher).
//   2. the values = MOT[mi] as the grid shows them (UpdateMotorParameter :659-668): ReadInitSpeed, PJogHigh/Low, PHomeHigh/Low,
//      PSoftLimitP/N, ReadAcc/ReadDec (= dAccDataBase, the database value), ReadRange.
//   3. the file = the backend's MotTablePath (the boot's LoadMotData path: W906_MOTTABLE_PATH, else D:\HT9045\System\Mot_Table.csv),
//      split into lines and cells exactly the way the boot reads it (vclcompat TStringList SetText + CommaText parseDelimited);
//      columns by TMOTNO::SetMOTTableNo's rule (AnsiPos substring, the last match wins); the row = the ONE line whose Alias cell is
//      the motor's Alias (0 or 2+ -> refused), with >= 29 cells (TMOTDATA's valid path) and Motorname "M<n>" = its MOT index.
//   4. per column the old cell is read the boot's way (TMOTDATA atoi / atof, an empty cell = TMOTDATA's default, then
//      InitialMotorParameter's MN200 rule `if(dAcc>1) dAcc/=100` and SetRange's a>1000 -> 1000) and compared with the grid's value.
//      Equal -> the cell is NOT touched (its text stays, e.g. "70" on an MN200 row that the grid shows as 0.7). Different -> new text:
//        integers as "%d" (an unsigned member as its 32-bit int, so the boot's atoi -> unsigned gives it back);
//        Acc / Dec: the shortest "%.Nf" (N = 0..17) the boot reads back as exactly the grid's value -- 100000 stays "100000",
//        0.025 is "0.025"; an MN200 cell > 1 keeps the x100 style (the boot's /100 gives the value back).
//      A quoted cell stays quoted; nothing else in the line moves (other cells, blanks, separators, the line ending).
//   5. all-or-nothing refusals (nothing written): a column missing / two tokens on one header cell, a value not finite / not an
//      integer in range, an EMPTY cell whose value differs (TMOTDATA treats it as NULL data and sets Enable=0 -- filling it would
//      change more than the setting), a cell the boot does not read for this row (INDEX_MOTION_CARD==0 Index rows' Acc/Dec are
//      1.0; MC88X1's Acc/Dec come from Rate and its Range is 10), a CardModel that is not the one this run loaded.
//   6. nothing changed -> nothing written, no backup ("no change"). Otherwise: backup = path + ".bak_yyyymmdd_hhmmss" (the bytes
//      read), the new bytes to path + ".tmp_savemot" (read back), the file re-read (refused if it changed meanwhile), MoveFileEx
//      REPLACE_EXISTING|WRITE_THROUGH, read back. Same naming as wb_serve's system.file.put csv writer (tools/wb_serve.cpp CsvApplyEdits).
//   7. HSys.MotTable's row := the file (live backend MotTableRowSaved: _CommaText + the changed members).
//  ⚠ Range's clamp is the 1203 / MN200 / SMC / SYNTEK classes' SetRange; a Galil / sim object does not clamp -- on such a row a file
//    value > 1000 with the grid at exactly 1000 is kept as "no change" (no such row on this machine: PCI1203 / MN200 / SMC only).
#include "vclcompat/TStringList.h"   // AI(W906-COMMATEXT-FU) 20261003: NB2-1 -- MtSplitCells splits with vclcompat CommaTextCells (the boot's own BCB6 CommaText, INBOX 152); this line was the banner's closing rule, so nothing below moves
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>     //AI(W906-MT-SAVEMOT) 20260930: MoveFileExA / GetLastError only -- included HERE, after every other line of this file, so its macros reach nothing above
#include <algorithm>     //AI(W906-MT-SAVEMOT) 20260930: std::sort

namespace ht9045 {
namespace {

const int kMtRowCells = 29;        // TMOTNO::emotTotal (database.cpp TMOTNO::TMOTNO): TMOTDATA reads a row's cells only with >= 29 of them

struct MtLine { std::size_t b, e; };                                   // a line's text in the file, its terminator excluded
struct MtCell { std::size_t b, e; bool quoted; std::string v; };      // a cell's bytes [b,e) in its line (quotes included) + its value

// vclcompat TStringList::SetText (LoadFromFile): LF ends a line, so does CR LF (the CR is not part of the line) and a lone CR;
//   a last line without a terminator counts when it is not empty.
void MtSplitLines(const std::string& t, std::vector<MtLine>& out)
{
    out.clear();
    std::size_t b = 0;
    for (std::size_t i = 0; i < t.size(); ++i) {
        if (t[i] == '\n') {
            MtLine l; l.b = b; l.e = (i > b && t[i - 1] == '\r') ? i - 1 : i;
            out.push_back(l); b = i + 1;
        } else if (t[i] == '\r' && !(i + 1 < t.size() && t[i + 1] == '\n')) {
            MtLine l; l.b = b; l.e = i;
            out.push_back(l); b = i + 1;
        }
    }
    if (b < t.size()) { MtLine l; l.b = b; l.e = t.size(); out.push_back(l); }
}

// vclcompat TStringList CommaText (what SetMOTTableNo and TMOTDATA read with), plus each cell's byte span -- AI(W906-COMMATEXT-FU)
//   20261003: vclcompat::CommaTextCells, i.e. BCB6's SetDelimitedText since INBOX 152: an unquoted cell ends at ',' or at any blank
//   (a blank inside one makes two cells, as the boot reads it); "a,b," = 3 cells (the last ""); "" = no cell. Was a copy of the old rule.
void MtSplitCells(const std::string& s, std::vector<MtCell>& out)
{   { std::vector<vclcompat::CommaTextCell> cc; vclcompat::CommaTextCells(s, cc); out.clear(); for (std::size_t k = 0; k < cc.size(); ++k) { MtCell c; c.b = cc[k].b; c.e = cc[k].e; c.quoted = cc[k].quoted; c.v = cc[k].v; out.push_back(c); } return; }   //AI(W906-COMMATEXT-FU) 20261003: NB2-1 (laptop task (b)) -- the boot's split (vclcompat TStringList.cpp CtSetDelimited, one parser); the old copy below kept under #if 0 so no line moves
#if 0 // AI(W906-COMMATEXT-FU) 20261003: the pre-INBOX-152 rule; this line was `out.clear();`
    const std::size_t n = s.size();
    std::size_t i = 0;
    while (i <= n) {
        while (i < n && (unsigned char)s[i] <= ' ' && s[i] != ',') ++i;
        MtCell c; c.b = i; c.e = i; c.quoted = false;
        if (i < n && s[i] == '"') {
            c.quoted = true; ++i;
            while (i < n) {
                if (s[i] == '"') {
                    if (i + 1 < n && s[i + 1] == '"') { c.v += '"'; i += 2; }
                    else { ++i; break; }
                } else { c.v += s[i]; ++i; }
            }
            c.e = i;
        } else {
            while (i < n && s[i] != ',') { c.v += s[i]; ++i; }
            std::size_t e = i;
            while (!c.v.empty() && (unsigned char)c.v[c.v.size() - 1] <= ' ') { c.v.erase(c.v.size() - 1); --e; }
            c.e = e;
        }
        out.push_back(c);
        if (i < n && s[i] == ',') { ++i; continue; }
        break;
    }
#endif // AI(W906-COMMATEXT-FU) 20261003: this line was `if (s.empty()) out.clear();`
}

int MtColumn(const std::vector<MtCell>& hdr, const std::string& token)   // TMOTNO::SetMOTTableNo: AnsiPos substring, LAST match wins
{
    int k = -1;
    for (std::size_t i = 0; i < hdr.size(); ++i) if (hdr[i].v.find(token) != std::string::npos) k = (int)i;
    return k;
}

int MtMotorIndexOf(const std::string& s)                                // WebMotorAccessLive.cpp MotorIndexOf (cinitial.cpp's "M%02d")
{
    if (s.size() < 2 || (s[0] != 'M' && s[0] != 'm')) return -1;
    for (std::size_t i = 1; i < s.size(); ++i) if (s[i] < '0' || s[i] > '9') return -1;
    return std::atoi(s.c_str() + 1);
}

double MtDefault(const std::string& col)                                // TMOTDATA's value for an EMPTY cell (database.cpp)
{
    if (col == "SoftLimitN") return -999999.0;
    if (col == "SoftLimitP") return 999999.0;
    if (col == "Acc" || col == "Dec") return 1.0;
    if (col == "Range") return 1.0;  if (col == "GearRatio") return 1.0;   //AI(W906-GEARRATIO) 20261002: TMOTDATA's empty GearRatio = 1.0 (database.cpp ~:2768), not the 100 of the speeds below (NB2 spec §5.4 trap 2). Same line, nothing moves
    return 100.0;                                                       // InitSpeed / HomeHighSpeed / HomeLowSpeed / JogHighSpeed / JogLowSpeed
}

// what the boot puts into MOT from an Acc / Dec cell: TMOTDATA atof, then InitialMotorParameter's MN200 `if(dAcc>1) dAcc/100.0`
//   volatile: the quotient is rounded to a double (what the boot stores into dAccDataBase) whatever the x87 excess-precision flags
double MtBootReal(const std::string& text, bool mn200)
{
    volatile double a = std::atof(text.c_str());
    if (mn200 && a > 1) { volatile double d = a / 100.0; return d; }
    return a;
}

// the shortest cell text the boot reads back as exactly `want`; MN200: the x100 style first when the old cell used it
bool MtRealText(double want, bool mn200, bool scaledFirst, std::string& out)
{
    if (want == 0.0) { out = "0"; return true; }                        // never "-0"
    const int styles = mn200 ? 2 : 1;
    for (int st = 0; st < styles; ++st) {
        const bool scaled = mn200 && ((st == 0) == scaledFirst);
        volatile double t = scaled ? want * 100.0 : want;
        char b[400];
        for (int d = 0; d <= 17; ++d) {
            std::snprintf(b, sizeof(b), "%.*f", d, t);
            volatile double got = MtBootReal(b, mn200);
            if (got == want) { out = b; return true; }
        }
        std::snprintf(b, sizeof(b), "%.17g", t);
        volatile double got = MtBootReal(b, mn200);
        if (got == want) { out = b; return true; }
    }
    return false;
}

std::string MtNum(double v) { char b[64]; std::snprintf(b, sizeof(b), "%.15g", v); return b; }

bool MtReadFile(const std::string& p, std::string& out)
{
    out.clear();
    std::FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return false;
    char buf[65536];
    std::size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
    const bool bad = std::ferror(f) != 0;
    std::fclose(f);
    return !bad;
}
bool MtWriteFile(const std::string& p, const std::string& data)        // write, then read back what landed
{
    std::FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return false;
    const bool wr = std::fwrite(data.data(), 1, data.size(), f) == data.size();
    const bool fl = std::fflush(f) == 0;
    const bool cl = std::fclose(f) == 0;
    std::string back;
    return wr && fl && cl && MtReadFile(p, back) && back == data;
}
bool MtExists(const std::string& p) { struct _stat st; return _stat(p.c_str(), &st) == 0; }
std::string MtStamp()                                                   // local yyyymmdd_hhmmss (wb_serve CsvApplyEdits' format)
{
    const std::time_t now = std::time(0);
    const std::tm lt = *std::localtime(&now);
    char b[32];
    std::strftime(b, sizeof(b), "%Y%m%d_%H%M%S", &lt);
    return b;
}

struct MtRepl { std::size_t b, e; std::string s; };
bool MtReplAfter(const MtRepl& x, const MtRepl& y) { return x.b > y.b; }

}  // namespace

std::vector<MotTableSaveField> MotTableFieldsOf(const MotorGolden& g)
{
    std::vector<MotTableSaveField> v;
    const struct { const char* c; MotTableFieldKind k; double x; } t[10] = {
        { "InitSpeed",     kMtfUInt, (double)g.initSpeed },                 // :659 ReadInitSpeed()
        { "JogHighSpeed",  kMtfUInt, (double)g.jogHigh },                   // :660 PJogHighSpeed
        { "JogLowSpeed",   kMtfUInt, (double)g.jogLow },                    // :661 PJogLowSpeed
        { "HomeHighSpeed", kMtfUInt, (double)g.homeHigh },                  // :662 PHomeHighSpeed
        { "HomeLowSpeed",  kMtfUInt, (double)g.homeLow },                   // :663 PHomeLowSpeed
        { "SoftLimitP",    kMtfInt,  (double)g.softP },                     // :664 PSoftLimitP
        { "SoftLimitN",    kMtfInt,  (double)g.softN },                     // :665 PSoftLimitN
        { "Acc",           kMtfReal, g.accDb },                             // :666 ReadAcc() = dAccDataBase
        { "Dec",           kMtfReal, g.decDb },                             // :667 ReadDec() = dDecDataBase
        { "Range",         kMtfUInt, (double)g.range } };                   // :668 ReadRange()
    for (int i = 0; i < 10; ++i) { MotTableSaveField f; f.column = t[i].c; f.kind = t[i].k; f.value = t[i].x; v.push_back(f); }
    return v;
}

MotTableEditResult MotTableEditRow(const std::string& text, const std::string& alias, int motIndex,
                                   const std::vector<MotTableSaveField>& fields, bool indexMotionCard0,
                                   const std::string& expectCard, std::string& outText)
{
    MotTableEditResult R;
    outText = text;
    std::vector<MtLine> lines;
    MtSplitLines(text, lines);
    if (lines.size() < 2) { R.why = "檔案沒有資料列（不到 2 行；LoadMotData 的 `StrList->Count<=1`）"; return R; }
    std::vector<MtCell> hdr;
    MtSplitCells(text.substr(lines[0].b, lines[0].e - lines[0].b), hdr);
    const int cNo = MtColumn(hdr, "Motorname"), cAlias = MtColumn(hdr, "Alias"), cCard = MtColumn(hdr, "CardModel"), cRate = MtColumn(hdr, "Rate");
    if (cNo < 0 || cAlias < 0 || cCard < 0) {
        R.why = std::string("表頭找不到 ") + (cNo < 0 ? "Motorname" : cAlias < 0 ? "Alias" : "CardModel") + " 欄（TMOTNO::SetMOTTableNo 的欄名）—— 不寫";
        return R;
    }
    std::vector<int> col(fields.size(), -1);
    for (std::size_t i = 0; i < fields.size(); ++i) {
        col[i] = MtColumn(hdr, fields[i].column);
        if (col[i] < 0) { R.why = "表頭找不到 " + fields[i].column + " 欄（TMOTNO::SetMOTTableNo 的欄名）—— 不寫"; return R; }
        for (std::size_t j = 0; j < i; ++j)
            if (col[j] == col[i]) {
                R.why = "表頭第 " + std::to_string(col[i] + 1) + " 欄同時對到 " + fields[j].column + " 與 " + fields[i].column +
                        "（SetMOTTableNo 是子字串比對）—— 不知道哪一格是哪一個，不寫";
                return R;
            }
    }
    std::size_t li = 0;
    int hits = 0;
    std::vector<MtCell> cells;
    for (std::size_t k = 1; k < lines.size(); ++k) {
        std::vector<MtCell> c;
        MtSplitCells(text.substr(lines[k].b, lines[k].e - lines[k].b), c);
        if ((int)c.size() > cAlias && c[(std::size_t)cAlias].v == alias) { if (++hits == 1) { li = k; cells.swap(c); } }
    }
    if (hits == 0) { R.why = "Mot_Table 裡找不到 Alias=" + alias + " 的那一列 —— 不寫"; return R; }
    if (hits > 1)  { R.why = "Mot_Table 裡 Alias=" + alias + " 有 " + std::to_string(hits) + " 列 —— 不知道寫哪一列，不寫"; return R; }
    R.lineNo = (int)li + 1;
    const std::string where = "第 " + std::to_string(R.lineNo) + " 行（" + alias + "）";
    if ((int)cells.size() < kMtRowCells) {
        R.why = where + "只有 " + std::to_string(cells.size()) + " 格（TMOTDATA 要 >= 29 格才讀這一列的值，database.cpp）—— 不寫";
        return R;
    }
    for (std::size_t i = 0; i < col.size(); ++i)
        if (col[i] >= (int)cells.size()) { R.why = where + "沒有 " + fields[i].column + " 那一格 —— 不寫"; return R; }
    if (cNo >= (int)cells.size() || cCard >= (int)cells.size()) { R.why = where + "沒有 Motorname／CardModel 那一格 —— 不寫"; return R; }
    R.motorName = cells[(std::size_t)cNo].v;
    if (MtMotorIndexOf(R.motorName) != motIndex) {
        R.why = where + "的 Motorname=" + R.motorName + "，不是這一軸的 MOT[" + std::to_string(motIndex) + "] —— 不寫";
        return R;
    }
    const std::string fileCard = cells[(std::size_t)cCard].v;
    const bool indexRow = alias == "MTestY1" || alias == "MTestZ1" || alias == "MTestZ2" || alias == "MTestY2";
    const bool indexForced = indexMotionCard0 && indexRow && fileCard != "PCI1203";   // TMOTDATA's special case + EastSun R2 (database.cpp)
    R.cardModel = indexForced ? std::string("SMC") : fileCard;
    if (!expectCard.empty() && expectCard != R.cardModel) {
        R.why = where + "的 CardModel 開機讀起來是 " + R.cardModel + "，這次執行載入的是 " + expectCard +
                " —— 檔案在開機後被改過；先重開 wb_serve，不寫";
        return R;
    }
    const bool mn200 = R.cardModel == "MN200", mc88 = R.cardModel == "MC88X1";
    const double rateV = (cRate < 0 || cRate >= (int)cells.size() || cells[(std::size_t)cRate].v.empty())
                         ? 1.0 : (double)std::atoi(cells[(std::size_t)cRate].v.c_str());   // TMOTDATA iRate (empty -> 1)

    std::vector<MtRepl> reps;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        const MotTableSaveField& f = fields[i];
        const MtCell& c = cells[(std::size_t)col[i]];
        MotTableCellChange ch;
        ch.column = f.column; ch.oldText = c.v; ch.newText = c.v;
        const double v = f.value;
        if (!std::isfinite(v)) { R.why = f.column + " 的值不是有限的數字 —— 整列不寫"; return R; }
        const bool acc = (f.column == "Acc" || f.column == "Dec"), range = (f.column == "Range");  const bool mnCol = mn200 && acc;   //AI(W906-GEARRATIO) 20261002: NB2 spec §5.4 trap 1 -- the boot's MN200 /100 is InitialMotorParameter's `dAcc>1 -> /100` ONLY (cinitial.cpp:4049-4054); GearRatio is a plain atof on every card (cinitial.cpp:4043). Was `mn200` for every real column (only Acc / Dec were real then). Same line
        unsigned uv = 0;
        int iv = 0;
        if (f.kind != kMtfReal) {
            const bool okInt = v == std::floor(v) &&
                (f.kind == kMtfUInt ? (v >= 0.0 && v <= 4294967295.0) : (v >= -2147483648.0 && v <= 2147483647.0));
            if (!okInt) { R.why = f.column + " 的值 " + MtNum(v) + " 不是範圍內的整數 —— 整列不寫"; return R; }
            if (f.kind == kMtfUInt) { uv = (unsigned)(unsigned long long)v; iv = (int)uv; }
            else                    { iv = (int)v; uv = (unsigned)iv; }
        }
        bool fixed = false;
        double fixedV = 0.0;
        std::string fixedWhy;
        if (acc && indexForced)  { fixed = true; fixedV = 1.0;   fixedWhy = "INDEX_MOTION_CARD==0 的 Index 列（CardModel 不是 PCI1203）TMOTDATA 不讀這一格、一律 1.0（database.cpp）"; }
        else if (acc && mc88)    { fixed = true; fixedV = rateV; fixedWhy = "MC88X1 開機的 Acc/Dec 取 Rate 欄（cinitial.cpp InitialMotorParameter），不讀這一格"; }
        else if (range && mc88)  { fixed = true; fixedV = 10.0;  fixedWhy = "MC88X1 的 Range 在 TMOTDATA 一律 10，不讀這一格"; }
        bool same;
        if (fixed)                   same = (v == fixedV);
        else if (c.v.empty())        same = (f.kind == kMtfReal) ? (v == MtDefault(f.column))
                                          : (f.kind == kMtfUInt) ? (uv == (unsigned)(int)MtDefault(f.column)) : (iv == (int)MtDefault(f.column));
        else if (f.kind == kMtfReal) {
            volatile double got = MtBootReal(c.v, mnCol);   //AI(W906-GEARRATIO) 20261002: mnCol (was mn200)
            same = (got == v);
            if (same && mnCol && std::atof(c.v.c_str()) > 1) ch.note = "MN200：檔案 " + c.v + " 開機 /100 = " + MtNum(v) + "（畫面值），不動";   //AI(W906-GEARRATIO) 20261002: mnCol (was mn200)
        }
        else if (range) {
            const unsigned o = (unsigned)std::atoi(c.v.c_str());
            same = (o == uv) || (o > 1000u && uv == 1000u);
            if (same && o != uv) ch.note = "檔案 " + c.v + " 開機被 SetRange 夾成 1000（畫面值），不動";
        }
        else if (f.kind == kMtfUInt) same = ((unsigned)std::atoi(c.v.c_str()) == uv);
        else                         same = (std::atoi(c.v.c_str()) == iv);
        if (!same) {
            if (fixed) { R.why = f.column + "：畫面值 " + MtNum(v) + "，但 " + fixedWhy + " —— 存不進 Mot_Table，整列不寫"; return R; }
            if (c.v.empty()) {
                R.why = f.column + "：檔案裡這一格是空的（TMOTDATA 把空格當 NULL、該軸 Enable 變 0，database.cpp）—— 這顆鈕不填空格，整列不寫";
                return R;
            }
            std::string t;
            if (f.kind == kMtfReal) {
                if (!MtRealText(v, mnCol, mnCol && std::atof(c.v.c_str()) > 1, t)) {   //AI(W906-GEARRATIO) 20261002: mnCol (was mn200)
                    R.why = f.column + " 的值 " + MtNum(v) + " 找不到開機讀回來完全相同的寫法 —— 整列不寫";
                    return R;
                }
            } else {
                char b[32];
                std::snprintf(b, sizeof(b), "%d", iv);
                t = b;
            }
            ch.newText = t; ch.changed = true; ++R.changedCount;
            MtRepl rp; rp.b = c.b; rp.e = c.e; rp.s = c.quoted ? ("\"" + t + "\"") : t;
            reps.push_back(rp);
        }
        R.cells.push_back(ch);
    }

    std::string row = text.substr(lines[li].b, lines[li].e - lines[li].b);
    std::sort(reps.begin(), reps.end(), MtReplAfter);                  // right to left: the spans to the left stay valid
    for (std::size_t k = 0; k < reps.size(); ++k) row.replace(reps[k].b, reps[k].e - reps[k].b, reps[k].s);
    std::vector<MtCell> chk;                                            // read the new row back the boot's way
    MtSplitCells(row, chk);
    bool good = chk.size() == cells.size();
    for (std::size_t k = 0; good && k < chk.size(); ++k) {
        std::string want = cells[k].v;
        for (std::size_t i = 0; i < col.size(); ++i) if (col[i] == (int)k) want = R.cells[i].newText;
        if (chk[k].v != want) good = false;
    }
    if (!good) { R.why = "內部檢查失敗：改寫後的那一列重新讀回來不對 —— 不寫"; return R; }
    R.newRowText = row;
    outText = text.substr(0, lines[li].b) + row + text.substr(lines[li].e);
    R.ok = true;
    return R;
}

MotTableSaveResult MotTableSaveRow(const std::string& path, const std::string& stamp, const std::string& alias, int motIndex,
                                   const std::vector<MotTableSaveField>& fields, bool indexMotionCard0, const std::string& expectCard)
{
    MotTableSaveResult S;
    S.path = path;
    std::string text;
    if (path.empty() || !MtReadFile(path, text)) { S.why = "讀不到 " + path + " —— 不寫"; return S; }
    if (text.empty()) { S.why = path + " 是空的 —— 不寫"; return S; }
    std::string out;
    S.edit = MotTableEditRow(text, alias, motIndex, fields, indexMotionCard0, expectCard, out);
    if (!S.edit.ok) { S.why = S.edit.why; return S; }
    if (S.edit.changedCount == 0) { S.ok = true; return S; }             // no change: nothing written, no backup
    const std::string st = stamp.empty() ? MtStamp() : stamp;
    std::string bak = path + ".bak_" + st;
    for (int k = 2; MtExists(bak) && k < 100; ++k) bak = path + ".bak_" + st + "_" + std::to_string(k);
    if (MtExists(bak)) { S.why = "備份檔名都被占用了（" + bak + "）—— 不寫"; return S; }
    if (!MtWriteFile(bak, text)) { std::remove(bak.c_str()); S.why = "備份寫不出來（" + bak + "）—— 不寫"; return S; }
    const std::string tmp = path + ".tmp_savemot";
    if (!MtWriteFile(tmp, out)) {
        std::remove(tmp.c_str()); std::remove(bak.c_str());
        S.why = "暫存檔寫不出來（" + tmp + "）—— 原檔沒動"; return S;
    }
    std::string again;
    if (!MtReadFile(path, again) || again != text) {
        std::remove(tmp.c_str()); std::remove(bak.c_str());
        S.why = path + " 在讀取之後被別的程式改過 —— 原檔沒動，請再按一次"; return S;
    }
    if (!::MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        const unsigned long e = (unsigned long)::GetLastError();
        std::remove(tmp.c_str()); std::remove(bak.c_str());
        S.why = "取代 " + path + " 失敗（MoveFileEx GetLastError=" + std::to_string(e) + "；檔案被別的程式開著？）—— 原檔沒動";
        return S;
    }
    S.backup = bak;
    S.wrote = true;
    std::string back;
    if (!MtReadFile(path, back) || back != out) { S.why = "已取代 " + path + "，但讀回來的內容不對 —— 請用備份 " + bak + " 還原"; return S; }
    S.ok = true;
    return S;
}

// ---- saveMotTable (the dispatcher's forward declaration is above MotorAccessDispatch) ----
static MotorAccessOutcome DoSaveMotTable(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    MotionCtx m;
    std::string who, why;
    if (!PageMotor(r, be, m, who, why)) return Refuse(why);                     // golden ActiveIndex==-1 etc.
    const std::string path = be.MotTableFilePath();
    if (path.empty()) return Refuse(who + ": 沒有 Mot_Table 的路徑（MotTablePath 是空的）—— 不寫");
    const MotTableSaveResult s = MotTableSaveRow(path, std::string(), r.motors[0], m.a.motIndex, MotTableFieldsOf(m.g),
                                                 be.MotTableIndexForced(), m.a.cardModel);
    if (!s.ok) {
        std::printf("motor.access saveMotTable %s: NOT written -- %s\n", r.motors[0].c_str(), s.why.c_str());
        return Refuse(who + "（" + path + "）: " + s.why);
    }
    std::vector<std::pair<std::string, std::string> > changed;
    std::string list;
    for (std::size_t i = 0; i < s.edit.cells.size(); ++i) {
        const MotTableCellChange& c = s.edit.cells[i];
        if (!c.changed) continue;
        changed.push_back(std::make_pair(c.column, c.newText));
        list += (list.empty() ? "" : "、") + c.column + " " + c.oldText + " → " + c.newText;
    }
    if (s.wrote) be.MotTableRowSaved(r.motors[0], s.edit.newRowText, changed);
    const std::string at = s.edit.motorName + "，第 " + std::to_string(s.edit.lineNo) + " 行";
    const std::string msg = s.wrote
        ? "已寫回 " + path + "：" + r.motors[0] + "（" + at + "）" + list + "；備份 " + s.backup
        : r.motors[0] + "（" + at + "）的 10 個設定值與 " + path + " 相同 —— no change，沒有寫檔、沒有備份";
    std::printf("motor.access saveMotTable: %s\n", msg.c_str());
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, s.wrote ? "saved" : "unchanged", s.wrote ? "Mot_Table" : "none", msg);
    w.Key("file").String(path);
    w.Key("backup"); if (s.backup.empty()) w.Null(); else w.String(s.backup);
    w.Key("line").Number((wb_int64)s.edit.lineNo);
    w.Key("motorName").String(s.edit.motorName);
    w.Key("motIndex").Number((wb_int64)m.a.motIndex);
    w.Key("changed").Number((wb_int64)s.edit.changedCount);
    w.Key("changes").BeginArray();                                              // only the cells that were rewritten
    for (std::size_t i = 0; i < s.edit.cells.size(); ++i) {
        const MotTableCellChange& c = s.edit.cells[i];
        if (!c.changed) continue;
        w.BeginObject(); w.Key("column").String(c.column); w.Key("old").String(c.oldText); w.Key("new").String(c.newText); w.EndObject();
    }
    w.EndArray();
    w.Key("kept").BeginArray();                                                 // unchanged cells whose text is not the grid's number (told, not touched)
    for (std::size_t i = 0; i < s.edit.cells.size(); ++i) {
        const MotTableCellChange& c = s.edit.cells[i];
        if (c.changed || c.note.empty()) continue;
        w.BeginObject(); w.Key("column").String(c.column); w.Key("text").String(c.oldText); w.Key("note").String(c.note); w.EndObject();
    }
    w.EndArray();
    WriteCurOf(w, be, m.a.motIndex);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

}  // namespace ht9045

//AI(W906-MT-SMHOME) 20261001: RULINGS_20261001 #8 (Jimmy 1001 08:4x「照golden接上，現在所有功能都要接上，不然我在機台端無法測試」).
//  A non-1203 axis (MN200 / PCI-L132 / a non-1203 EtherCAT row ...) homes the golden way:
//    start    btnHomeClick's else branch (golden uMotorTest.cpp:1152-1158) `fHome->iHomeStep=1; iSingleHomeIndex=ActiveIndex;
//             InitProcessSingleMotorTask(iSingleHomeIndex); bSingleHome=true; MOT[ActiveIndex].HomeFlag=0;`
//    stepping golden MainProc while the page is shown (csystem.cpp:16954-16960)
//             `if(ProcessSingleMotorHome(i) && MOT[i].HomeFlag==1) bSingleHome=false;`
//    end      Timer1Timer (:962-966) sees bSingleHome==false: `btnHome->Down=false; edtHomeOffset->Text=MOT[ActiveIndex].Motor->
//             LastHomePos; MOT[ActiveIndex].SetSpeed(scrlbrMotorSpeed->Position);`
//    release  (:1168-1171) `bSingleHome=false; MOT[ActiveIndex].PCIL132_StopMotor();`
//  Light Scale's case 0 (:1835-1840) starts the same single home without the button (fromLS: no Timer1Timer SetSpeed).
//  Until today DoHome refused this and Light Scale only left a note, both citing the return-true stub that AI(W906-SMHOME) 0927
//  retired (ProcessSingleMotorHome is translated line for line at the end of uhome.cpp).
//  Here: a HomeJob of kind 1 (axis -1) in the same job list -- STOP / AllBtnUp / an alarm / the page gone / the safe lock already
//  cancel it like any HOME -- stepped once per 500 ms beat (golden: once per MainProc pass; same state machine, coarser).
//  ⚠ golden's failure path is kept: ProcessSingleMotorHome raises ShowMotorErrorMessage itself (in wb_serve a blocking alarm box
//  like every MainProc alarm; other web commands answer "modal-pending" meanwhile, so nothing re-enters the job list) and leaves
//  HomeFlag != 1 -- the job then stays, as golden's bSingleHome does, until HOME is released or STOP.
//  Not here: the Galil Index rows (INDEX_MOTION_CARD==0, golden :1133-1151 / DoGaliHome) -- MotionPreludeFor refuses every motion
//  of such a row before DoHome gets here (the Galil Motor Test family is its own back-fill).
namespace ht9045 {
namespace {

HomeJob* FindGoldenHome(int mi)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i)
        if (g_homes[i].active && g_homes[i].kind != 0 && g_homes[i].mi == mi) return &g_homes[i];
    return 0;
}

MotorAccessOutcome DoHomeGolden(const MotorAccessReq& r, IMotorAccessBackend& be, const MotionCtx& m, const std::string& who, bool wantStart)
{
    HomeJob* running = FindGoldenHome(m.a.motIndex);
    if (!wantStart) {                                                           // golden else branch (:1168-1171)
        if (running) CancelHome(be, *running, false, "home released (golden: bSingleHome=false)");
        be.GoldenStopMotor(m.a.motIndex);                                       // golden MOT[ActiveIndex].PCIL132_StopMotor() (unconditional)
        EndSingleHome(be, "HOME released (golden btnHomeClick :1170 bSingleHome=false)");
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "homeStopped", "MOT", running ? "HOME 抬起：停止歸零（golden else 支：bSingleHome=false; PCIL132_StopMotor）"
                                                    : "HOME 抬起：沒有進行中的歸零，照 golden 送停止");
        w.Key("homeActive").Bool(false);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    if (running) {                                                              // already homing: no second InitProcessSingleMotorTask
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "homing", "MOT", "已在歸零中（不重下命令）");
        w.Key("homeActive").Bool(true);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    if (!be.GoldenSingleHomeBegin(m.a.motIndex))
        return Refuse(who + "（" + m.a.cardModel + "）: 這個後端沒有 golden 單軸歸零（GoldenSingleHomeBegin）");
    HomeJob h;
    h.active = true; h.motorId = r.motors[0]; h.mi = m.a.motIndex; h.axis = -1; h.g = m.g; h.kind = 1; h.fromLS = false;
    PushHomeJob(h);
    char b[200];
    std::snprintf(b, sizeof(b), "golden InitProcessSingleMotorTask(%d) + MOT[%d].HomeFlag=0; ProcessSingleMotorHome(%d) every beat (golden MainProc) until HomeFlag==1",
                  m.a.motIndex, m.a.motIndex, m.a.motIndex);
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "homing", "MOT", b);
    w.Key("homeActive").Bool(true);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

void TickGoldenHome(IMotorAccessBackend& be, HomeJob& h)
{
    if (!be.GoldenSingleHomeStep(h.mi)) return;                                 // golden: bSingleHome stays true, the next MainProc steps again
    SingleHomeSucceeded();                                                      // golden bSingleHome=false (also releases Light Scale's case 1)
    if (!h.fromLS && !h.zAllUp) {                                               // golden Timer1Timer :962-966 (btnHome->Down only)  //AI(W906-TEACH-ZALLUP) 20261001: + !zAllUp (golden DoZHome sets no speed)
        const int pct = JogPctOf(h.mi);                                         // scrlbrMotorSpeed->Position (non-Galil rows: only the bar / the selection set it)
        be.GoldenSetSpeed(h.mi, pct, false);                                    // MOT[ActiveIndex].SetSpeed(Position) -- PTP family
        g_ptpPct[h.mi] = pct;
    }
    CancelHome(be, h, false, "golden ProcessSingleMotorHome done, HomeFlag=1 -> bSingleHome=false");
}

bool LsStartGoldenHome(IMotorAccessBackend& be, int mi, const std::string& alias, const MotionCtx& m)
{
    if (FindGoldenHome(mi)) { g_ls.lastNote = "lightScale home " + alias + ": a single home already runs on this axis (it goes on)"; return true; }
    if (!be.GoldenSingleHomeBegin(mi)) { g_ls.lastNote = "lightScale home " + alias + "（" + m.a.cardModel + "）: this backend has no golden single home -- the scan waits at case 1"; return true; }
    HomeJob h;
    h.active = true; h.motorId = alias; h.mi = mi; h.axis = -1; h.g = m.g; h.kind = 1; h.fromLS = true;
    PushHomeJob(h);
    g_ls.lastNote = "lightScale home " + alias + "（" + m.a.cardModel + "）: golden InitProcessSingleMotorTask + ProcessSingleMotorHome every beat";
    return true;
}

}  // namespace
}  // namespace ht9045

//AI(W906-TEACH-ZDISABLED) 20261001: EastSun 1001「如果 testz2 enable 是0 那就不要擋testz2」—— 合約在 WebMotorAccess.h（MotorAccessTeachInterlockHome）。
//  放在檔尾：本檔行號被別處引用（docs、WebCmdGuard.cpp），不推動它們。
namespace ht9045 {
int MotorAccessTeachInterlockHome(IMotorAccessBackend& be, int mi)
{
    std::string alias;
    MotorAccessAxis a;
    if (be.AliasOfMotIndex(mi, alias) && be.Resolve(alias, a) && !a.Is1203() && !a.tableEnable)
        return -3;                                                              // 這台沒裝這一軸（Mot_Table 的 Enable 欄，同 1203 路徑用的 tableEnable）⇒ Teach 互鎖不檢查它
    return MotorAccessTeachHomeLed(be, mi);                                     // 其餘一格不變：1203 軸（ORG 規則、Enable=0＝1）、Enable=1 的非 1203 軸（-2＝golden HOME 燈，false 照樣擋）、表上沒有（-2）
}
}  // namespace ht9045

// =============================================================================
//  AI(W906-TEACH-ZALLUP) 20261001: Teach buttons that were on the page with no handler (the 10-01 Teach audit): In / Out Z All Up
//  and the Index tab's four Servo buttons. Contract in WebMotorAccess.h EOF. At the end of the file: this file's line numbers
//  are cited elsewhere (docs, WebCmdGuard.cpp), nothing above moves.
//
//  teachZAllUp -- golden uteach.cpp (verbatim):
//      :4466 void __fastcall TfTeach::btnInZAllUpClick(TObject *Sender)
//            {   for(int i=0; i<InArmSuck.iMotRow; i++)
//                    for(int j=0; j<InArmSuck.iMotCol; j++)
//                    {   bInArmZHome[i][j]=false;
//                        InitProcessSingleMotorTask(InArmZIndex[i][j]);   }
//            //    bInArmAllUp=true;
//            }
//      :4480 btnOutZAllUpClick = the same with OutArmSuck / bOutArmZHome / OutArmZIndex.
//    Stepped by Timer1Timer :1374 `if(DoZHome()==false) return;` -- DoZHome :4505-4540 calls ProcessSingleMotorHome(index) for every
//    flag still false until it returns true (uhome.cpp:415: case 200 `MotorInitial(); if(Motor->Enable==false) { HomeFlag=1;
//    Position=0; return true; }`; case 300/400 MotorHome; an arm Z then case 500 `if(MOT[Index].MotorMove(ZSafePos)) HomeFlag=true`),
//    and shows btnInZAllUp / btnOutZAllUp->Caption="Homeing..." meanwhile.
//    golden has NO interlock here: no CheckCanMove / IsCanQuickJogMove / motor-power test / ActiveMotorIndex (Teach cannot be open
//    while SystemStart, main.cpp:27827). The checks that do apply are the ones inside the home itself (TMyMotor::Home's safe door,
//    case 500's MotorMove door / soft limit / lock) -- the same ones StartHome1203 / TickHomes already carry.
//  Port, per Z of that arm in golden order (backend GoldenTeachFixedMotors "inZ" / "outZ"):
//    * no Mot_Table row or no MOT object -> skipped, said in the ack (golden would step MOT[i] anyway);
//    * Enable=0 (a PCI1203 row: the Mot_Table column = tableEnable, as every 1203 path here; another row: Motor->Enable) ->
//      golden case 200: HomeFlag=1, nothing sent (MOT[].Position=0 is not reachable from here; the row is not installed);
//    * otherwise the very single home Teach's / Motor Test's HOME starts (DoHome): a 1203 row -> StartHome1203 (DS402 124/128 or
//      the card-side MODE12 by the drive; safe door, stale sample, home speeds) + TickHomes' case 500 (MotorMove(ZSafePos), then
//      HomeFlag=1); another row -> golden InitProcessSingleMotorTask + ProcessSingleMotorHome every beat (DoHomeGolden, no SetSpeed
//      at the end: zAllUp). A home already running on that axis keeps running (no second command; golden's InitProcessSingleMotorTask
//      would restart its state machine).
//    Result: something started / already running -> ok "homing" (partial when another Z was refused); nothing started and something
//    refused -> refused with every reason; nothing to home (every Z skipped or Enable=0) -> ok "zAllUpDone" (golden: the next DoZHome
//    pass finds every flag true at once).
//    Refused before anything (dispatcher): SystemStart, a hand teach open (golden fTeachShow is modal), a Motor Test source.
//  ⚠ Not reproduced: (1) golden DoZHome steps BOTH arms with InArmSuck's iMotRow/iMotCol (:4508-4510) -- an Out grid larger than
//    the In grid leaves golden on "Homeing..." for ever; here each arm's own grid (:4482) is armed and every armed job is stepped.
//    (2) while golden DoZHome runs, Timer1Timer returns before its btnHome / DoPitch_Home steps; here a HOME pressed meanwhile on
//    another axis starts at once (EastSun 20260929: axes independent). (3) STOP, closing Teach, the operator gone, the safe lock and
//    an alarm cancel these jobs like every HOME job of this file (HomeFlag stays 0); golden stops the motors and DoZHome keeps
//    stepping a stopped home. (4) fHome->iHomeStep=1 (inside InitProcessSingleMotorTask) only for the non-1203 rows, as Teach's HOME.
//
//  teachIndexServo -- golden uteach.cpp (verbatim):
//      :4253 void __fastcall TfTeach::btnZ1ServoClick(TObject *Sender)
//            {   static bool bflag=false;
//                if(bflag==false) {   MOT[MTestZ1].Gali_Command("MOY", __FUNC__);  btnZ1Servo->Caption="Server OFF";   }
//                else             {   MOT[MTestZ1].Gali_Command("SHY", __FUNC__);  btnZ1Servo->Caption="Server ON";    }
//                bflag=!bflag;
//                fAllMotorHome=false;   }
//      :4270 btnZ2ServoClick = MTestZ2 "MOZ" / "SHZ".  :2919 btnArm1YServoClick = MTestY1 "MOX" / "SHX", plus MTestY2 "MOW" / "SHW"
//            when USE_INDEX_ARM_AXES==IndexArm_4_Axis, and it sets the Caption of BOTH btnArm1YServo and btnArm2YServo (uteach.dfm
//            gives btnArm2YServo the same OnClick). Galil "MO" = motor off, "SH" = servo here (on).
//  Port: the motors come from golden (GoldenTeachFixedMotors "z1" / "z2" / "arm1y"), never from the page's selected motor.
//    target = the button's first motor's actual servo state inverted when it is known (1203 SVON bit / MOT Led[iServoOn]) -- the
//    rule of DoServo (落差清單 1) -- else golden's static (first press = OFF); one static per golden handler, it then follows the
//    target. A PCI1203 row -> DoServo's own path (Enable=0 refusal, brake HOLD before a servo off, kCmdAxSvOn; the Index Z1 Gali
//    route EtherCAT/Pci1203GaliRoute.cpp maps the same "MO" / "SH" onto that SvOn). Another row -> refused: golden's Gali_Command
//    talks to the Galil card and motor.access has no Galil path (an Enable=0 row gets the Enable=0 refusal first). Then
//    fAllMotorHome=false (golden; the dispatcher clears it on every Teach command while idle anyway).
//    Ack: servoOn, basis, caption ("Server OFF" / "Server ON" = golden's Caption after the press), captionButtons, per motor.
// =============================================================================
namespace ht9045 {
namespace {

bool g_idxServoStatic[3] = { false, false, false };   // golden `static bool bflag=false;` of btnZ1ServoClick / btnZ2ServoClick / btnArm1YServoClick

std::string AckField(const std::string& ack, const char* key)                  // a string member of a sub-ack (empty if none)
{
    std::string out;
    cJSON* j = cJSON_Parse(ack.c_str());
    const cJSON* v = j ? cJSON_GetObjectItemCaseSensitive(j, key) : 0;
    if (cJSON_IsString(v) && v->valuestring) out = v->valuestring;
    cJSON_Delete(j);
    return out;
}
bool HomeJobOnMotor(int mi)
{
    for (std::size_t i = 0; i < g_homes.size(); ++i) if (g_homes[i].active && g_homes[i].mi == mi) return true;
    return false;
}
std::string JoinList(const std::vector<std::string>& v)
{
    std::string s;
    for (std::size_t i = 0; i < v.size(); ++i) s += (i ? "; " : "") + v[i];
    return s;
}
void WriteList(webbridge::JsonWriter& w, const char* key, const std::vector<std::string>& v)
{
    w.Key(key).BeginArray();
    for (std::size_t i = 0; i < v.size(); ++i) w.String(v[i]);
    w.EndArray();
}
void TeachZAllUpResetState()                                                    // MotorAccessResetJobs (tests): golden's statics start false
{
    for (int i = 0; i < 3; ++i) g_idxServoStatic[i] = false;
}

MotorAccessOutcome DoTeachZAllUp(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    const bool in = (r.button == "btnInZAllUp");
    if (!in && r.button != "btnOutZAllUp")
        return Refuse("teachZAllUp: 按鈕 '" + r.button + "' 不是 golden 的 btnInZAllUp／btnOutZAllUp");
    const std::string who = std::string(in ? "In Z All Up（golden btnInZAllUpClick :4466）" : "Out Z All Up（golden btnOutZAllUpClick :4480）");
    std::vector<int> mis;
    std::string why;
    if (!be.GoldenTeachFixedMotors(in ? "inZ" : "outZ", mis, why)) return Refuse(who + ": " + why);
    std::vector<std::string> started, already, enable0, skipped, refused;
    for (std::size_t k = 0; k < mis.size(); ++k) {
        const int mi = mis[k];
        std::string alias;
        MotorAccessAxis a;
        MotorGolden g;
        if (!be.AliasOfMotIndex(mi, alias) || !be.Resolve(alias, a)) { skipped.push_back("MOT[" + std::to_string(mi) + "]: Mot_Table 沒有這一軸"); continue; }
        if (a.motIndex < 0 || !be.GoldenMotor(a.motIndex, g)) { skipped.push_back(alias + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL）"); continue; }
        if (a.Is1203() ? !a.tableEnable : !g.enable) {                          // golden ProcessSingleMotorHome case 200 (uhome.cpp:429-437)
            be.GoldenSetHomeFlag(a.motIndex, 1);
            enable0.push_back(alias);
            continue;
        }
        if (HomeJobOnMotor(a.motIndex)) { already.push_back(alias); continue; }
        MotorAccessReq h = r;                                                   // the single home of Teach's / Motor Test's HOME (DoHome)
        h.action = "home"; h.motors.assign(1, alias); h.flag["start"] = true;
        const MotorAccessOutcome o = DoHome(h, wireId, be);
        if (!o.ok) { refused.push_back(alias + ": " + o.ackJson); continue; }
        for (std::size_t i = 0; i < g_homes.size(); ++i)
            if (g_homes[i].active && g_homes[i].mi == a.motIndex) g_homes[i].zAllUp = true;
        started.push_back(alias + "（" + AckField(o.ackJson, "message") + "）");
    }
    std::string msg = who + ":";
    if (!started.empty()) msg += " 歸零 " + JoinList(started) + "；";
    if (!already.empty()) msg += " 已在歸零中（不重下命令）" + JoinList(already) + "；";
    if (!enable0.empty()) msg += " Enable=0 → HomeFlag=1（golden case 200）" + JoinList(enable0) + "；";
    if (!skipped.empty()) msg += " 略過 " + JoinList(skipped) + "；";
    if (!refused.empty()) msg += " 拒絕 " + JoinList(refused) + "；";
    const bool active = !started.empty() || !already.empty();
    if (!active && !refused.empty())
        return Refuse(who + ": 沒有一個 Z 開始歸零 —— " + JoinList(refused));
    if (mis.empty()) msg += " 這一臂沒有 Z 軸（golden iMotRow／iMotCol 是 0，迴圈不做）";
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, active ? "homing" : "zAllUpDone", "uteach", msg);
    w.Key("arm").String(in ? "in" : "out");
    w.Key("homeActive").Bool(active);
    w.Key("caption").String(active ? "Homeing..." : (in ? "In Z All Up" : "Out Z All Up"));   // golden DoZHome :4526-4534
    WriteList(w, "started", started);
    WriteList(w, "already", already);
    WriteList(w, "enable0", enable0);
    WriteList(w, "skipped", skipped);
    WriteList(w, "refused", refused);
    w.Key("partial").Bool(!refused.empty());
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

MotorAccessOutcome DoTeachIndexServo(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    int hd = -1;
    const char* which = 0;
    std::vector<std::string> capBtns;
    if (r.button == "btnZ1Servo")      { hd = 0; which = "z1"; capBtns.push_back("btnZ1Servo"); }
    else if (r.button == "btnZ2Servo") { hd = 1; which = "z2"; capBtns.push_back("btnZ2Servo"); }
    else if (r.button == "btnArm1YServo" || r.button == "btnArm2YServo") {      // the .dfm gives both the same OnClick
        hd = 2; which = "arm1y"; capBtns.push_back("btnArm1YServo"); capBtns.push_back("btnArm2YServo");
    } else {
        return Refuse("teachIndexServo: 按鈕 '" + r.button + "' 不是 golden 的 btnZ1Servo／btnZ2Servo／btnArm1YServo／btnArm2YServo");
    }
    static const char* const kH[3] = { "golden btnZ1ServoClick :4253", "golden btnZ2ServoClick :4270", "golden btnArm1YServoClick :2919" };
    const std::string who = r.button + "（" + kH[hd] + "）";
    std::vector<int> mis;
    std::string why;
    if (!be.GoldenTeachFixedMotors(which, mis, why)) return Refuse(who + ": " + why);
    if (mis.empty()) return Refuse(who + ": golden 沒有對應的馬達");
    struct One { int mi; std::string alias; MotorAccessAxis a; std::string bad; };
    std::vector<One> ms;
    for (std::size_t k = 0; k < mis.size(); ++k) {
        One x; x.mi = mis[k];
        if (!be.AliasOfMotIndex(x.mi, x.alias) || !be.Resolve(x.alias, x.a)) {
            x.bad = "MOT[" + std::to_string(x.mi) + "]: Mot_Table 沒有這一軸";
        } else if (!x.a.Is1203()) {
            MotorGolden g;
            const bool hasG = x.a.motIndex >= 0 && be.GoldenMotor(x.a.motIndex, g);
            if (!x.a.tableEnable || (hasG && !g.selectable))                     // the existing Enable=0 refusal (DoServo / lM00Click)
                x.bad = NotSelectable(x.alias + "（" + x.a.cardModel + "）");
            else
                x.bad = x.alias + "（" + x.a.cardModel + "）: golden Gali_Command(\"MO?\"／\"SH?\") 走 Galil 卡，motor.access 沒有 Galil 的路（只接 PCI1203 軸）";
        }
        ms.push_back(x);
    }
    // the target: the first motor's actual state inverted when known (DoServo's rule), else golden's static
    bool target = g_idxServoStatic[hd];                                        // golden: bflag==false -> "MO" (OFF)
    std::string basis = "golden-static";
    if (ms[0].bad.empty()) {
        bool known = false, cur = false;
        std::string w2;
        if (ms[0].a.Is1203()) { if (be.Pci1203Ready(w2) && ms[0].a.axis >= 0) cur = be.Pci1203ServoOn(ms[0].a.axis, known); }
        else cur = be.GoldenServoOn(ms[0].a.motIndex, known);
        if (known) { target = !cur; basis = "toggle-actual"; }
    }
    std::vector<std::string> done, bad;
    for (std::size_t k = 0; k < ms.size(); ++k) {
        if (!ms[k].bad.empty()) { bad.push_back(ms[k].bad); continue; }
        MotorAccessReq s = r;                                                   // DoServo's 1203 path with the target spelled out
        s.action = "servoToggle"; s.motors.assign(1, ms[k].alias); s.flag["servoOn"] = target;
        const MotorAccessOutcome o = DoServo(s, wireId, be);
        if (o.ok) done.push_back(ms[k].alias + "（" + AckField(o.ackJson, "message") + "）");
        else      bad.push_back(o.ackJson);
    }
    be.GoldenClearAllMotorHome();                                               // golden :4267 / :4284 / :2939 fAllMotorHome=false
    if (done.empty()) return Refuse(who + ": " + JoinList(bad));
    g_idxServoStatic[hd] = !target;                                             // golden bflag=!bflag (the next golden-static press is the other way)
    const std::string caption = target ? "Server ON" : "Server OFF";           // golden's Caption after the press
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, target ? "servoOn" : "servoOff", "uteach",
            who + ": " + (target ? "Servo ON（golden \"SH\"）" : "Servo OFF（golden \"MO\"）") + " " + JoinList(done) +
            (bad.empty() ? std::string() : "；拒絕 " + JoinList(bad)));
    w.Key("servoOn").Bool(target);
    w.Key("basis").String(basis);
    w.Key("caption").String(caption);
    WriteList(w, "captionButtons", capBtns);
    WriteList(w, "done", done);
    WriteList(w, "refused", bad);
    w.Key("partial").Bool(!bad.empty());
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

}  // namespace
}  // namespace ht9045

// =============================================================================
//  AI(W906-ARMCELL) 20261002 [W906]: the Teach page "Arm Cell" tab -- action moveToTrayCell (RULINGS_20261002 #18; NB2 spec
//  RD5軟體_NB2規格_Teach頁ArmCell分頁_20261002_101914.md §3.3, on origin/v906/nb2-assist). golden has no such button: the
//  HT160 equivalent is Teach -> Advanced -> Sort Arm "Pick / Place Test" (HT160S aSortArm.cpp MoveSuckerToCell :2819-2905:
//  Task 10 all Z to the safe height, 30 X + Y together, 40 the chosen Z down when ticked, 100 done, Z not lifted). The
//  targets come from the backend (GoldenArmCellPlan = ArmCellPlan.cpp, golden's production formulas copied into locals);
//  the job is this file's 1203 helpers, stepped by the 500 ms beat like a HOME job. At the end of the file: this file's
//  line numbers are cited elsewhere; every hook above is a same-line addition.
//
//  S0 accept (DoArmCell, synchronous -- refused = nothing moved): no other job (HOME / LoopMove / jog / Light Scale /
//     Motor Power On / hand teach), motor power on (golden uteach btnHomeClick :2137's test), the backend's plan (area /
//     nozzle / cell / D1 / the v1 refusals), every axis through MotionPreludeFor (Mot_Table Enable, 1203 control, the
//     monitor opened it) and a PCI1203 axis (v1), HomeFlag==1 on X, Y and every Enable Z of the arm, then EVERY target
//     against the soft limits (MoveBlock1203's >= / <=) before anything moves (TeachGo020 W5B-R8's rule); zDown asked on
//     an area that does not allow it -> refused with the area's reason.
//  S1 Z up      every Enable Z of the arm -> the runtime ZSafePos (GoldenZSafePos: Gerneral.ini [In Arm] ZSafePos, never a
//               literal) at 20 %; a Z already READY within the gap on a fresh sample is not commanded. MoveBlock1203 per Z.
//               Arrival = READY + a fresh sample + |cmdPos-target| <= gap (5 for an arm Z with 2 < GearRatio <= 5, else 2:
//               GoldenMove1203's USE_CompareCommandPos gap). ERROR_STOP -> failed; two fresh READY samples away from the
//               target -> cancelled ("沒到目標就停了"); 60 s -> timeout.
//  S2 verify    a poll newer than the arrival; MotorAccessTeachHomeLed == 1 for every Z; golden GoldenTeachCanMove(X) &&
//               GoldenTeachCanMove(Y) (CheckCanMove + IsCanQuickJogMove) ONCE per job. Fail -> failed (the reason carries
//               g_teachHomeUnknown). golden ActiveMotorIndex is then the last axis checked: echoed as job.activeMotor
//               (the page follows it like teachEchoFrom's active=).
//  S3 X / Y     every Z checked at ZSafePos again in the issuing beat (review m1), then both through MoveBlock1203, then both
//               commands in the same beat at 20 % (GoButton020Click's "Steven 20210723 Go按鈕速度提升到20%"); Y not issued -> X
//               stopped, failed. While they move: a Z off ZSafePos (not READY, out of its gap, or no valid / fresh sample --
//               fail closed, review m1) -> cancelled, all stopped. 120 s -> timeout.
//  S4 Z down    only with zDown: X and Y re-checked READY at the cell on a fresh sample (review m1); D2 for a shuttle area (the
//               shuttle axis READY on a fresh sample within 2 of its arm-side point, ArmCellPlan.cpp D2Of) -- from then on the
//               shuttle is one of the job's axes (locked, stopped with it, review m3); then the chosen Z only, at 1 %
//               (GoButton140Click's SetSpeed(1)), MoveBlock1203, the same arrival rule, 60 s.
//  S5 done      result "arrived" with cmdPos / actPos of X, Y and the chosen Z. Z is NOT lifted (HT160 does not either; In /
//               Out Z All Up lifts it).
//  Cancel -- every one also stops the job's own axes (D3 asks it for the operator gone / the token taken; the other
//  triggers stop them anyway, a second StopDec is harmless) and counts the stops like DoStop (a refused stop goes into why
//  and job.stop, review N5): STOP (DoStop -> CancelAllJobs), an alarm (MotorAccessOnAlarm), the safe lock and the operator
//  gone (MotorAccessTick / PollTick gates), VerifyMotorAction's AllBtnUp, the Teach close edge (DoStop), any other
//  CancelAllJobs (a Motor Test FormShow / selectMotor / Motor Power -- same as a HOME job), SystemStart turning true, a safe
//  door open on any of its axes. NOT a trigger (review N1): a servo toggle on one of its axes -- while the job runs the
//  busy gate refuses it before DoServo (uteach: every other action; Motor Test: the same axis); CancelJobsOnAxis stays
//  wired as a second lock only.
//  While it runs: every other uteach action is refused (a teachSet query passes), a Motor Test request naming one of its
//  axes or moving every motor (Reload Motor Data, a Light Scale start, an All-mode loop start) is refused, START is refused
//  (MotorAccessStartBlocked), its axes read "moving" for VerifyMotorAction (MotorAccessMovingOf / AxisLock). STOP is
//  dispatched before every gate.
//  [W906] each a ruling or the safe direction: the job itself (RULINGS_20261002 #18), HomeFlag at S0 and the speeds
//  (spec §4.4), the all-target soft-limit pre-check, D2, D3 + every cancel stops, the refusals while it runs (golden Teach
//  has no btnHome / Loop interlock), the 60 / 120 / 60 s timeouts, the job / catalog block of /api/struct/motor/runtime.
//
//  AI(W906-ARMCELL2) 20261002 [W906]: NB2 R165 (the user 1002 22:1x 「照建議」: what HT160 does and the spec missed) + NB2's
//  review list for 04:30:
//  (1) a box on screen stops it -- HT160 uteach IsFaultPopupShowing (fNote / MyMessageBox visible) -> StopAllAdvancedTests
//      (:1531, tmrUpdate :1078). The edge: wb_serve's five popup hosts call MotorAccessArmCellOnPopup the moment they show a
//      box (the blocking ones then hold the tick thread -- no beat runs until they are answered, so a per-beat look alone would
//      come too late); the state: g_W906ArmCellPopupUp (live: golden fNote->fShow / MyMessageBox->fShow, + a kcode==0 notice not yet acknowledged since INBOX 151) refuses S0 and
//      cancels on the beat. Both end "cancelled" (the page: 已中止) and stop the job's own axes. The box the job's own S2
//      golden check pops (CheckCanMove's message) is that check's -- not counted; the job ends "failed" on it right after.
//  (2) S0 reads the alarm lamps of every axis it will command (HT160 CheckSortArmTestReady :1351-1353 iAlarmLed /
//      iServoalarmLed) as golden TMyEtherCatMotor::ScanMotorStatus decodes them for this card (myEthercatmotor.cpp:1148-1211):
//      Led[iAlarmLed] = ALM (motionIO bit 1) or STA_AX_ERROR_STOP, Led[iEmgLed] = EMG (bit 6); golden never writes
//      Led[iServoalarmLed] for a 1203 axis, so the card's servo alarm IS that ALM bit. No sample -> refused (fail closed).
//      Also the four EMG sensors (golden InitMotor :180-181). While it runs, each beat: a box up, an EMG sensor off, an ALM or
//      EMG lamp on any of its axes -> cancelled + stopped (HT160 tmrUpdate: 警報窗、伺服警報或 EMG ⇒ 全停). ERROR_STOP while
//      it runs stays the step's own "failed" (unchanged).
//  (4) S3: every Z keeps the origin S2 saw (MotorAccessTeachHomeLed, the tree's ORG rule incl. W906_HT9050_ORG_INVERT) while
//      X / Y move -- HT160 MoveSortArmX :716 (Z home lost -> StopAllMotor + alarm, SUCK_HOME_LOST_MS 100, aSortArm.cpp:44).
//      Lost on two fresh monitor samples in a row (samples are >= 200 ms apart, so one is never enough and two always are --
//      the nearest this loop gets to 100 ms) -> cancelled, every axis of the job stopped.
//  Review "arrived = INP or the encoder, never the command position alone": golden itself compares the command position
//  (USE_CompareCommandPos, MachineType.h:43; mymotor.cpp:445-456) and this card's INP is not decoded (RogerYang 20250421,
//  myEthercatmotor.cpp:1180) -- so here a leg is at its target only when READY on a fresh sample, the command position is
//  within the gap AND the encoder (actPos) is within kArmCellEncTol. Command there but the encoder not within
//  kArmCellSettleMs -> failed, stopped. The same encoder rule for "Z already at ZSafePos", the S3 Z watch, the S4 X / Y
//  re-check and D2.
//  (3) the tab's "All Z Up (to Safe Z)" (HT160 uteach.dfm btnSaAllZUp) is the page's: it presses Teach's own In / Out Z All
//      Up (ht9045_teach_armcell.js) -- no second Z-up here.
//
//  AI(W906-ARMCELL-ZORG) 20261003 [W906]: NB2 R174 M1 (NIGHT_REPORT s0 #70, the laptop's promise 「第一次讀到就先停 X 不用問，筆電下一批
//  照 HT160 做」) -- (4) now has HT160's two stages (MoveSortArmX aSortArm.cpp:726-745 as NB2 read it; the HT160 source is not on
//  this box): the FIRST fresh sample with a Z's origin not lit stops the travel legs still under way (X / Y commanded and not yet
//  confirmed arrived; Stop1203, the job's own stop) and nothing else -- not the Zs, not the shuttle, no StopAllMotor. The next
//  fresh sample ends the job "cancelled" either way: still not lit = the two-sample rule above (every axis of the job stopped);
//  lit again = a one-sample flicker, also cancelled -- X / Y were stopped short and are NOT re-commanded (press GO again). The
//  watch also runs right after each 1203 Poll (MotorAccessPollTick, ~200 ms), not only on the 500 ms beat (ArmCellPollTick). The
//  escalation to StopAllMotor + an alarm box (HT160 :739-740) is s0 #70 item 2 -- ruled A (RULINGS_20261003 #22), done: WAR16442 (ArmCellOrgWatch).
//  ArmCellZStillSafe (the Z encoder at ZSafePos, checked first on every beat) is unchanged.
// =============================================================================
namespace ht9045 {
namespace {

struct ArmCellLeg {
    std::string     alias;
    MotorAccessAxis a;
    MotorGolden     g;
    int             target;
    int             gap;
    int             state;          // 0 = not commanded, 1 = commanded, 2 = arrived
    int             wrongReady;
    unsigned long   wrongPoll;
    double          encSince;       //AI(W906-ARMCELL2) 20261002: since when the command is there and the encoder is not (-1 = not waiting)
    int             encPos;         //   the last encoder read (user units) while waiting
    bool            encKnown;       //   ... readable at all
    int             orgLost;        //   (4) fresh samples in a row with this Z's origin not lit (S3)
    unsigned long   orgPoll;
    ArmCellLeg() : target(0), gap(2), state(0), wrongReady(0), wrongPoll(0), encSince(-1.0), encPos(0), encKnown(false), orgLost(0), orgPoll(0) {}
};
struct ArmCellJob {
    bool            active;
    unsigned long   jobId;          // this file's counter (the page matches the ack's cellSeq with it)
    long long       seq, wireId;
    std::string     reqId;
    int             step;           // 1..5 (where it is, or where it ended)
    std::string     result, why;    // "running" | "arrived" | "cancelled" | "failed" | "timeout"
    ArmCellPlan     plan;
    bool            zDown;
    ArmCellLeg      x, y, zSel;
    std::vector<ArmCellLeg> zs;     // S1: every Enable Z of the arm
    double          stepStartMs;
    unsigned long   verifyAfterPoll;
    bool            goldenChecked;
    std::string     activeMotor;    // golden ActiveMotorIndex after S2 (the last axis GoldenTeachCanMove was given)
    bool            hasFinal;
    int             finalCmd[3], finalAct[3];   // X, Y, the chosen Z (user units)
    bool            finalKnown[3];
    std::string     startedAt, endedAt;
    bool            d2Locked;       // review m3: the D2 shuttle is one of the job's axes from the moment D2 passed (S4) to the end
    bool            stopped;        // review N5: the end sent the job's stops -- counted like DoStop's partial report
    int             stopSent, stopAccepted, stopRefused;
    std::string     stopWhy;        // the axes whose stop was refused + the first reason
    ArmCellLeg      d2;
    bool            inGoldenCheck;  //AI(W906-ARMCELL2) 20261002: inside S2's GoldenTeachCanMove -- a box it pops is the check's (not a popup abort)
    bool            orgStop;        //AI(W906-ARMCELL-ZORG) 20261003: S3 -- the first fresh sample with a Z's origin not lit already stopped the travel legs
    std::string     orgStopNote;    //   ... which Z, which legs were stopped (and a refused stop); appended to the end's why
    ArmCellJob() : active(false), jobId(0), seq(-1), wireId(-1), step(0), zDown(false), stepStartMs(0.0), verifyAfterPoll(0),
                   goldenChecked(false), hasFinal(false), d2Locked(false), stopped(false), stopSent(0), stopAccepted(0), stopRefused(0),
                   inGoldenCheck(false), orgStop(false)
    { for (int i = 0; i < 3; ++i) { finalCmd[i] = finalAct[i] = 0; finalKnown[i] = false; } }
};
ArmCellJob    g_cell;
unsigned long g_cellSeq = 0;
std::string   g_cellCatalogLast;
unsigned long g_cellCatalogRev = 0;
const int     kArmCellXYTimeoutMs = 120000;   // S3 (TickLoop's leg timeout); S1 / S2 / S4 use kZSafeTimeoutMs (60 s)
//AI(W906-ARMCELL2) 20261002 [W906]: NB2 R165 review "arrived = the encoder" -- not golden (golden compares the command position).
//  kArmCellEncTol: |actPos - target| allowed, user units (1/100 mm; 10 = 0.1 mm, 5x the command gap -- a servo at rest after a
//  PTP move sits within a few counts; ES02 to confirm on the machine). kArmCellSettleMs: how long after the command position is
//  there the encoder may still be settling before the leg is called "did not follow".
const int     kArmCellEncTol = 10;
const double  kArmCellSettleMs = 3000.0;

const char* ArmCellStepName(int s)
{
    switch (s) {
    case 1: return "S1 Z 上升到 ZSafePos";
    case 2: return "S2 驗證 Z 在原點";
    case 3: return "S3 X／Y 走到格子";
    case 4: return "S4 Z 下降";
    case 5: return "S5 完成";
    }
    return "S0 受理";
}
std::string ArmCellNum(long long v) { return std::to_string(v); }

bool ArmCellActive() { return g_cell.active; }

std::vector<ArmCellLeg*> ArmCellLegs()                                         // X, Y, every Z (the chosen Z is one of them), the D2 shuttle once locked
{
    std::vector<ArmCellLeg*> v;
    v.push_back(&g_cell.x);
    v.push_back(&g_cell.y);
    for (std::size_t i = 0; i < g_cell.zs.size(); ++i) v.push_back(&g_cell.zs[i]);
    if (g_cell.d2Locked) v.push_back(&g_cell.d2);                              // review m3: locked (MovingOf / AxisLock / busy gate), door-watched, stopped with the job
    return v;
}
bool ArmCellUsesAxis(int axis1203)
{
    if (!g_cell.active || axis1203 < 0) return false;
    std::vector<ArmCellLeg*> legs = ArmCellLegs();
    for (std::size_t i = 0; i < legs.size(); ++i) if (legs[i]->a.Is1203() && legs[i]->a.axis == axis1203) return true;
    return false;
}
bool ArmCellOnAxis(const std::string& id, const MotorAccessAxis& b)
{
    if (!g_cell.active) return false;
    std::vector<ArmCellLeg*> legs = ArmCellLegs();
    for (std::size_t i = 0; i < legs.size(); ++i)
        if (SameAxisAs(legs[i]->alias, legs[i]->a.motIndex, legs[i]->a.Is1203() ? legs[i]->a.axis : -1, id, b)) return true;
    return false;
}
std::string ArmCellWhere()
{
    return "job " + ArmCellNum((long long)g_cell.jobId) + "，" + g_cell.plan.label + " (" + ArmCellNum(g_cell.plan.col + 1) + ", " +
           ArmCellNum(g_cell.plan.row + 1) + ")，" + ArmCellStepName(g_cell.step);
}

void ArmCellEnd(IMotorAccessBackend& be, const char* result, const std::string& why, bool stop)
{
    if (!g_cell.active) return;
    //AI(W906-ARMCELL) 20261002: review N5 -- the stop results are counted the way DoStop counts its partial stop (sent / accepted /
    //  refused + the first refusal); a refused stop is written into why (and the runtime job.stop) instead of being dropped.
    std::string refusedNames, firstRefusal;
    if (stop) {
        g_cell.stopped = true;
        std::vector<ArmCellLeg*> legs = ArmCellLegs();
        for (std::size_t i = 0; i < legs.size(); ++i) {
            if (!legs[i]->a.Is1203() || legs[i]->a.axis < 0) continue;
            const Pci1203CmdResult res = Stop1203(be, legs[i]->a.axis, -1);   // StopDec + ExtDrive 0 (golden DecStop)
            if (!CmdFailed(res)) { ++g_cell.stopAccepted; if (res.issued) ++g_cell.stopSent; continue; }
            ++g_cell.stopRefused;
            refusedNames += (refusedNames.empty() ? "" : "、") + legs[i]->alias;
            if (firstRefusal.empty()) firstRefusal = CmdWhy(res);
        }
    }
    if (g_cell.stopRefused) g_cell.stopWhy = refusedNames + "：" + firstRefusal;
    g_cell.active = false;
    g_cell.result = result;
    g_cell.why = why + (g_cell.stopRefused ? "；⚠ 停軸被拒 " + ArmCellNum(g_cell.stopRefused) + " 軸（" + g_cell.stopWhy + "）—— 請按 STOP 或急停確認" : std::string());
    g_cell.endedAt = NowIso();
    g_lastJobNote = std::string("arm cell ") + result + "（" + ArmCellStepName(g_cell.step) + "）: " + why;
}
void CancelArmCell(IMotorAccessBackend& be, const std::string& why)
{
    if (g_cell.active) ArmCellEnd(be, "cancelled", why, true);
}
void ArmCellResetState()                                                        // MotorAccessResetJobs (tests)
{
    g_cell = ArmCellJob();
    g_cellSeq = 0;
    g_cellCatalogLast.clear();
    g_cellCatalogRev = 0;
}
std::string ArmCellStartBlockedWhy()
{
    return MotorAccessArmCellBlockedWhy("START");                            // review m5: one text for START and main.home
}
void ArmCellFillJobs(MotorAccessJobState& j)
{
    j.cellActive = g_cell.active;
    j.cellMotors.clear();
    if (!g_cell.active) return;
    std::vector<ArmCellLeg*> legs = ArmCellLegs();
    for (std::size_t i = 0; i < legs.size(); ++i) j.cellMotors.push_back(legs[i]->alias);
}

// the dispatcher's gate while the job runs ("" = pass); STOP never reaches it
std::string ArmCellBusyWhy(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    if (!g_cell.active || r.action == "stop") return std::string();
    const std::string head = "motor.access " + r.action + "（" + r.source + "）: 教導頁 Arm Cell 進行中（" + ArmCellWhere() + "）—— ";
    if (r.source == "uteach") {
        std::map<std::string, bool>::const_iterator qf = r.flag.find("query");
        if (r.action == "teachSet" && qf != r.flag.end() && qf->second) return std::string();  if (r.action == "teachSt02") { std::map<std::string, std::string>::const_iterator sb = r.str.find("btn"); if (((sb != r.str.end() && !sb->second.empty()) ? sb->second : r.button) == "St02State") return std::string(); }   // read-only (the page's hand-teach query)  //AI(W906-ARMCELL3) 20261003: St02-M Q1 (CHAT_ST02 1003 02:50) -- St02's read-only state query (teachSt02 with btn St02State, St02BtnOf's rule: params.btn, else the button) passes too, so the Teach page's St02 panel stops retrying while a job runs; every other teachSt02 button stays refused
        if (r.action == "moveToTrayCell") return head + "已經有一個在走，先按 STOP 或等它走完";
        return head + "教導頁其他按鈕要等它走完或按 STOP（C++ 擋，不只畫面鎖）";
    }
    if (MotorTestStopDirection(r)) return std::string();
    for (std::size_t k = 0; k < r.motors.size(); ++k) {
        MotorAccessAxis b;
        if (!be.Resolve(r.motors[k], b)) continue;
        if (ArmCellOnAxis(r.motors[k], b)) return head + r.motors[k] + " 是它正在用的軸（MotorTest 只擋同一軸）";
    }
    const char* what = 0;
    if (r.action == "reloadMotorData") what = "Reload Motor Data 會把表值套回所有馬達";
    else if (r.action == "lightScale" && !g_ls.timer) what = "Light Scale 開始掃描會自己歸零、移動手臂";
    else if (r.action == "loopMove") {
        std::map<std::string, double>::const_iterator md = r.num.find("mode");
        std::map<std::string, bool>::const_iterator sf = r.flag.find("start");
        if (md != r.num.end() && md->second != 0.0 && !(sf != r.flag.end() && !sf->second)) what = "All 模式的來回會動所有勾選的軸";
    }
    return what ? head + what : std::string();
}

int ArmCellGap(const MotorGolden& g) { return (g.armZ && g.gearRatio > 2 && g.gearRatio <= 5) ? 5 : 2; }

//AI(W906-ARMCELL2) 20261002: the encoder of a 1203 axis in user units (the monitor's actPos); false = unreadable
bool ArmCellEncoder(IMotorAccessBackend& be, const MotorAccessAxis& a, const MotorGolden& g, int& e)
{
    double c = 0.0;
    if (!be.Pci1203ActPos(a.axis, c)) return false;
    e = MotorCardToUser(c, g.gearRatio);
    return true;
}
bool ArmCellEncNear(int e, int target) { return e - target <= kArmCellEncTol && target - e <= kArmCellEncTol; }

// READY + a fresh sample + within `gap` of `target` (pos = the sample's user position when one was read).
//AI(W906-ARMCELL) 20261002: review m7 -- the position is this file's CurrentUserPos (cmdPos, or actPos for a 1203 axis on the
//  encoder base after a hand teach, g_encBase W5B-6), like every other "where is it now" read here -- was cmdPos always.
//AI(W906-ARMCELL2) 20261002: NB2 R165 review -- AND the encoder within kArmCellEncTol (unreadable = not there, fail closed); pos
//  then reports the encoder when it is the one that is off.
bool ArmCellAt(IMotorAccessBackend& be, const MotorAccessAxis& a, const MotorGolden& g, int target, int gap, int* pos = 0, bool* stale = 0)
{
    unsigned st = 0;
    if (stale) *stale = SampleStale(be, a.axis);
    if (SampleStale(be, a.axis) || !be.Pci1203AxisState(a.axis, st) || !IsReadyState(st)) return false;
    MotionCtx m; m.a = a; m.g = g;
    int p = 0;
    if (!CurrentUserPos(be, m, p)) return false;
    if (pos) *pos = p;
    if (p - target > gap || target - p > gap) return false;
    int e = 0;
    if (!ArmCellEncoder(be, a, g, e)) return false;
    if (!ArmCellEncNear(e, target)) { if (pos) *pos = e; return false; }
    return true;
}

// a commanded leg: 1 arrived, 0 not yet, -1 ERROR_STOP, -2 stopped before the target (two fresh READY samples away; TickLoop's rule)
//AI(W906-ARMCELL2) 20261002: NB2 R165 review "arrived = INP or the encoder, never the command position alone" -- 1 needs the
//  encoder within kArmCellEncTol too; -3 = the command position is there but the encoder is not (or unreadable) kArmCellSettleMs
//  after it first was (the drive did not follow: servo off, a jam, a slipping coupling). now = LoopNow.
int ArmCellPoll(IMotorAccessBackend& be, ArmCellLeg& L, double now)
{
    unsigned st = 0;
    if (!be.Pci1203AxisState(L.a.axis, st)) return 0;
    if (IsErrorState(st)) return -1;
    if (SampleStale(be, L.a.axis) || !IsReadyState(st)) return 0;
    double card = 0.0;
    if (!be.Pci1203CmdPos(L.a.axis, card)) return 0;
    const int pos = MotorCardToUser(card, L.g.gearRatio);
    if (pos - L.target <= L.gap && L.target - pos <= L.gap) {
        int e = 0;
        L.encKnown = ArmCellEncoder(be, L.a, L.g, e);
        L.encPos = e;
        if (L.encKnown && ArmCellEncNear(e, L.target)) { L.encSince = -1.0; return 1; }
        if (L.encSince < 0.0) L.encSince = now;
        return (now - L.encSince > kArmCellSettleMs) ? -3 : 0;
    }
    L.encSince = -1.0;
    const unsigned long pc = be.Pci1203PollCount();
    if (pc != L.wrongPoll) { L.wrongPoll = pc; ++L.wrongReady; }
    return L.wrongReady >= 2 ? -2 : 0;
}
// the why of -3
std::string ArmCellEncWhy(const ArmCellLeg& L)
{
    const int d = L.encPos > L.target ? L.encPos - L.target : L.target - L.encPos;
    return L.alias + " 的命令位置到了（目標 " + ArmCellNum(L.target) + "），但" +
           (L.encKnown ? "編碼器在 " + ArmCellNum(L.encPos) + "（差 " + ArmCellNum(d) + "，容許 ±" + ArmCellNum(kArmCellEncTol) + "）"
                       : std::string("讀不到編碼器 actPos")) +
           "，" + ArmCellNum((long long)(kArmCellSettleMs / 1000.0)) + " 秒內沒跟上 —— 不算到位（到位看編碼器，不只看命令位置；NB2 R165）";
}

//AI(W906-ARMCELL2) 20261002: NB2 R165 (2) -- golden TMyEtherCatMotor::ScanMotorStatus's lamps of a 1203 axis, from the monitor
//  sample (myEthercatmotor.cpp:1179 ALM bit 1 and :1188 STA_AX_ERROR_STOP -> Led[iAlarmLed], :1183 EMG bit 6 -> Led[iEmgLed];
//  Led[iServoalarmLed] is never written for this card). "" = none lit. errorStop: count ERROR_STOP (S0) or not (the beat -- the
//  step reports an ERROR_STOP as "failed" itself). No sample = lit (fail closed).
std::string ArmCellLampWhy(IMotorAccessBackend& be, const ArmCellLeg& L, bool errorStop)
{
    unsigned long io = 0;
    unsigned st = 0;
    const bool hasIo = be.Pci1203MotionIO(L.a.axis, io), hasSt = be.Pci1203AxisState(L.a.axis, st);
    if (!hasIo || !hasSt) return L.alias + " 讀不到監看器的 motionIO／狀態（看不到 Alarm／EMG 燈，fail-closed）";
    std::string w;
    if (io & 0x00000002ul) w = "ALM 亮（驅動器警報；golden Led[iAlarmLed]，這張卡的伺服警報就是它）";
    if (errorStop && IsErrorState(st)) w += (w.empty() ? "" : "、") + std::string("ERROR_STOP（golden Led[iAlarmLed]）");
    if (io & 0x00000040ul) w += (w.empty() ? "" : "、") + std::string("EMG 亮（急停輸入；golden Led[iEmgLed]）");
    return w.empty() ? std::string() : L.alias + " " + w;
}

// command one leg (Move1203's tail): 1 = sent, 0 = the axis' last command has no fresh sample yet (wait), -1 = refused (why)
int ArmCellIssue(IMotorAccessBackend& be, ArmCellLeg& L, int pct, std::string& why)
{
    MotorGolden g;
    if (be.GoldenMotor(L.a.motIndex, g)) { L.g = g; L.gap = ArmCellGap(g); }   // the interlocks read the live soft limits
    if (SampleStale(be, L.a.axis)) return 0;
    const std::string who = "Arm Cell " + L.alias;
    why = MoveBlock1203(be, L.a, L.g, who, L.target);                           // door, lock, Enable, soft limits, READY, floodgate
    if (!why.empty()) return -1;
    MotionCtx m; m.a = L.a; m.g = L.g;
    std::string note;
    if (!Send1203Speed(be, g_cell.wireId, m, pct, why, note)) { why = who + ": " + why; return -1; }   // golden SetSpeed(pct)
    g_ptpPct[L.a.motIndex] = pct;
    NoteRotatorDir(be, L.a, L.g, L.target);                                     // golden MotorMovePosition :590-593
    Pci1203Cmd c;
    c.kind = kCmdAxMoveAbs; c.wireId = g_cell.wireId; c.axis = L.a.axis; c.value = (double)MotorUserToCard(L.target, L.g.gearRatio);
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) { why = who + ": 1203 移動失敗 —— " + CmdWhy(res); return -1; }
    NoteIssued(be, L.a.axis);
    L.state = 1; L.wrongReady = 0; L.wrongPoll = 0;  L.encSince = -1.0;   //AI(W906-ARMCELL2) 20261002: a new command, a new settle wait
    return 1;
}

bool ArmCellLegOf(const std::string& alias, long long wireId, IMotorAccessBackend& be, ArmCellLeg& L, std::string& why)
{
    MotionCtx m;
    if (!MotionPreludeFor("moveToTrayCell", alias, wireId, be, m, false)) { why = m.why; return false; }
    if (!m.a.Is1203()) {
        why = "moveToTrayCell " + alias + "（" + m.a.cardModel + "）: 第一版只接 PCI1203 軸（9050；RULINGS_20261002 第 18 條）";
        return false;
    }
    L.alias = alias; L.a = m.a; L.g = m.g; L.gap = ArmCellGap(m.g);
    return true;
}

void ArmCellArrive(IMotorAccessBackend& be)
{
    ArmCellLeg* L[3] = { &g_cell.x, &g_cell.y, &g_cell.zSel };
    for (int i = 0; i < 3; ++i) {
        double c = 0.0, a = 0.0;
        g_cell.finalKnown[i] = be.Pci1203CmdPos(L[i]->a.axis, c) && be.Pci1203ActPos(L[i]->a.axis, a);
        g_cell.finalCmd[i] = MotorCardToUser(c, L[i]->g.gearRatio);
        g_cell.finalAct[i] = MotorCardToUser(a, L[i]->g.gearRatio);
    }
    g_cell.hasFinal = true;
    g_cell.step = 5;
    ArmCellEnd(be, "arrived", g_cell.plan.label + " (" + ArmCellNum(g_cell.plan.col + 1) + ", " + ArmCellNum(g_cell.plan.row + 1) + ") 到位" +
                              (g_cell.zDown ? "，" + g_cell.zSel.alias + " 已降到 " + ArmCellNum(g_cell.zSel.target)
                                            : "，Z 停在 ZSafePos " + ArmCellNum(g_cell.plan.zSafe)) +
                              "（Z 不自動抬起；要抬請按 In／Out Z All Up）", false);
}

void TickArmCellZUp(IMotorAccessBackend& be, double now)
{
    ArmCellJob& J = g_cell;
    bool all = true;
    for (std::size_t i = 0; i < J.zs.size(); ++i) {
        ArmCellLeg& L = J.zs[i];
        if (L.state == 2) continue;
        if (L.state == 0) {
            if (ArmCellAt(be, L.a, L.g, L.target, L.gap)) { L.state = 2; continue; }   // already at ZSafePos: not commanded
            std::string why;
            if (ArmCellIssue(be, L, 20, why) < 0) { ArmCellEnd(be, "failed", "S1 Z 上升：" + why, true); return; }
            all = false;
            continue;
        }
        const int st = ArmCellPoll(be, L, now);
        if (st == -1) { ArmCellEnd(be, "failed", "S1 Z 上升：" + L.alias + " ERROR_STOP（驅動器／卡片報錯）", true); return; }
        if (st == -2) { ArmCellEnd(be, "cancelled", "S1 Z 上升：" + L.alias + " 沒到 ZSafePos 就停了（連續兩次 READY 卻停在別處）", true); return; }
        if (st == -3) { ArmCellEnd(be, "failed", "S1 Z 上升：" + ArmCellEncWhy(L), true); return; }   //AI(W906-ARMCELL2) 20261002
        if (st == 1) L.state = 2; else all = false;
    }
    if (!all) {
        if (now - J.stepStartMs > kZSafeTimeoutMs) ArmCellEnd(be, "timeout", "S1 Z 上升：60 秒還沒到 ZSafePos " + ArmCellNum(J.plan.zSafe), true);
        return;
    }
    J.step = 2; J.stepStartMs = now; J.verifyAfterPoll = be.Pci1203PollCount();
}

void TickArmCellVerify(IMotorAccessBackend& be, double now)
{
    ArmCellJob& J = g_cell;
    if (be.Pci1203PollCount() <= J.verifyAfterPoll) {                           // a poll newer than the arrival
        if (now - J.stepStartMs > kZSafeTimeoutMs) ArmCellEnd(be, "timeout", "S2 驗證：60 秒沒有新的監看器樣本", true);
        return;
    }
    for (std::size_t i = 0; i < J.zs.size(); ++i) {
        g_teachHomeUnknown.clear();
        const int led = MotorAccessTeachHomeLed(be, J.zs[i].a.motIndex);
        if (led == 1) continue;
        const std::string w = (led == 0) ? std::string("ORG 沒亮（不在原點）")
                            : (led == -1) ? "原點狀態不明（fail-closed）：" + g_teachHomeUnknown : std::string("不是 1203 軸，讀不到原點");
        ArmCellEnd(be, "failed", "S2 驗證：" + J.zs[i].alias + " 在 ZSafePos 但 " + w +
                                 " —— golden 教導頁的「Z 在原點」檢查過不了，X／Y 不動（現有的 Teach Go 也一樣，INBOX #24）", true);
        return;
    }
    if (!J.goldenChecked) {                                                     // once per job: a failure pops golden's message once
        J.goldenChecked = true;
        g_teachHomeUnknown.clear();
        J.inGoldenCheck = true;                                                 //AI(W906-ARMCELL2) 20261002: the box CheckCanMove pops is this check's own (not a popup abort)
        bool ok = be.GoldenTeachCanMove(J.x.a.motIndex);
        J.activeMotor = J.x.alias;
        if (ok) { ok = be.GoldenTeachCanMove(J.y.a.motIndex); J.activeMotor = J.y.alias; }
        J.inGoldenCheck = false;
        if (!ok) {
            ArmCellEnd(be, "failed", "S2 驗證：golden CheckCanMove()／IsCanQuickJogMove() 不允許（急停、臂的 Z 不在原點、閘門等；訊息框已顯示原因）" +
                                     (g_teachHomeUnknown.empty() ? std::string() : "；其中 1203 軸的原點狀態不明（當成不在原點，fail-closed）：" + g_teachHomeUnknown), true);
            return;
        }
    }
    J.step = 3; J.stepStartMs = now;
}

//AI(W906-ARMCELL) 20261002: review m1 -- fail closed: a Z with no valid sample, with a stale one (a command of this file and no
//  newer poll) or with no readable position cannot be shown at ZSafePos -> NOT safe (was: skipped, judged next time).
bool ArmCellZStillSafe(IMotorAccessBackend& be, std::string& why)
{
    for (std::size_t i = 0; i < g_cell.zs.size(); ++i) {
        const ArmCellLeg& L = g_cell.zs[i];
        unsigned st = 0;
        if (!be.Pci1203AxisState(L.a.axis, st)) { why = L.alias + " 沒有監看器樣本（無法確認在 ZSafePos，fail-closed）"; return false; }
        if (SampleStale(be, L.a.axis)) { why = L.alias + " 命令之後還沒有新的監看器樣本（無法確認在 ZSafePos，fail-closed）"; return false; }
        if (IsErrorState(st)) { why = L.alias + " ERROR_STOP"; return false; }
        int pos = 0;
        MotionCtx m; m.a = L.a; m.g = L.g;
        if (!CurrentUserPos(be, m, pos)) { why = L.alias + " 讀不到位置（無法確認在 ZSafePos，fail-closed）"; return false; }   // review m7: actPos on the encoder base
        if (!IsReadyState(st) || pos - L.target > L.gap || L.target - pos > L.gap) {
            why = L.alias + " 離開了 ZSafePos " + ArmCellNum(L.target) + "（現在 " + ArmCellNum(pos) + (IsReadyState(st) ? "" : "，不在 READY") + "）";
            return false;
        }
        int e = 0;                                                              //AI(W906-ARMCELL2) 20261002: NB2 R165 review -- the encoder too
        if (!ArmCellEncoder(be, L.a, L.g, e)) { why = L.alias + " 讀不到編碼器（無法確認在 ZSafePos，fail-closed）"; return false; }
        if (!ArmCellEncNear(e, L.target)) {
            why = L.alias + " 離開了 ZSafePos " + ArmCellNum(L.target) + "（編碼器 " + ArmCellNum(e) + "，命令位置 " + ArmCellNum(pos) + "）";
            return false;
        }
    }
    return true;
}

//AI(W906-ARMCELL-ZORG) 20261003: NB2 R174 M1 -- S3's Z origin watch (was inline in TickArmCellXY: AI(W906-ARMCELL2) 20261002,
//  NB2 R165 (4)). Every Z keeps the origin S2 saw (MotorAccessTeachHomeLed == 1, the tree's ORG rule incl. W906_HT9050_ORG_INVERT)
//  while X / Y move; not lit = 0 (not at home) or -1 (unknown: fail closed). HT160 MoveSortArmX (aSortArm.cpp:726-745 as NB2 read
//  it): the FIRST read of a Z off home stops X at once (MSortingArmX->Stop(), :733); the loss is then confirmed for
//  SUCK_HOME_LOST_MS 100 (:44) before StopAllMotor + ShowSystemError(K_RETRY). Here, per fresh monitor sample (one sample is
//  counted once, whoever looks at it -- the poll sequence, as before):
//    1st sample not lit -> Stop1203 (the job's own stop: StopDec + ExtDrive 0) on every travel leg still under way (state 1:
//                          X / Y commanded, arrival not yet confirmed; an arrived leg is at rest at its target and is left
//                          alone), once per job. Nothing else is stopped or sent, and no leg is judged on this sample.
//    2nd sample not lit -> cancelled, every axis of the job stopped (ArmCellEnd) -- the two-sample end as before; its why also
//                          names the first stop. Escalating to StopAllMotor + an alarm box (HT160 :739-740) is NIGHT_REPORT s0
//                          #70 item 2 -- ruled A (RULINGS_20261003 #22) and done: then be.GoldenArmCellZOrgAlarm (WAR16442, K_RETRY).
//    next sample lit    -> a one-sample flicker: cancelled as well, every axis of the job stopped. X / Y were stopped short of
//                          the cell and are NOT re-commanded (no automatic resume after a sensor glitch; press GO again). What
//                          HT160 does after its stop when the sensor comes back is not known here (its source is not on this box).
//  Callers: the 500 ms beat (TickArmCellXY, after ArmCellZStillSafe) and right after each 1203 Poll (ArmCellPollTick). The travel
//  stop is latched (orgStop) and ArmCellEnd ends a job once, so the two cannot act twice on one sample.
//  true = the watch holds the job (it ended, or the travel legs are stopped and the next sample decides): step nothing else.
bool ArmCellOrgWatch(IMotorAccessBackend& be)
{
    ArmCellJob& J = g_cell;
    bool darkNow = false;
    for (std::size_t i = 0; i < J.zs.size(); ++i) {
        ArmCellLeg& Z = J.zs[i];
        g_teachHomeUnknown.clear();
        const int led = MotorAccessTeachHomeLed(be, Z.a.motIndex);
        if (led == 1) { Z.orgLost = 0; continue; }
        const unsigned long pc = be.Pci1203PollCount();
        if (pc != Z.orgPoll) { Z.orgPoll = pc; ++Z.orgLost; }
        const std::string what = (led == 0) ? std::string("不在原點") : "原點狀態不明：" + g_teachHomeUnknown;
        if (Z.orgLost >= 2) {
            ArmCellEnd(be, "cancelled", "S3 X／Y 走的時候 " + Z.alias + " 的原點（ORG）不亮了（連續兩個新樣本；" + what +
                                        "）—— 照 HT160 MoveSortArmX：Z 離開原點就全停（NB2 R165）" + (J.orgStop ? "；" + J.orgStopNote : std::string()), true);
            be.GoldenArmCellZOrgAlarm(Z.alias, what);  return true;   //AI(W906-ARMCELL-ZALARM) 20261003: NB2-1, RULINGS_20261003 #22 = s0 #70 (2) -- HT160 aSortArm.cpp:739-740 StopAllMotor + ShowSystemError(K_RETRY) on the confirmed loss: the job ended above (its axes stopped, no job left for the alarm's own sweep), then the alarm box (live: WAR16442 K_RETRY, golden ShowErrorMessage stops all motors itself); a one-sample flicker below stays cancel-only. Same line
        }
        darkNow = true;
        if (J.orgStop) continue;
        J.orgStop = true;
        std::string names, refused;
        ArmCellLeg* T[2] = { &J.x, &J.y };
        for (int k = 0; k < 2; ++k) {
            if (T[k]->state != 1) continue;                                     // 2 = arrived: at rest at its target
            const Pci1203CmdResult res = Stop1203(be, T[k]->a.axis, -1);       // the job's own stop (ArmCellEnd's)
            names += (names.empty() ? "" : "、") + T[k]->alias;
            if (CmdFailed(res)) refused += (refused.empty() ? "" : "；") + T[k]->alias + " 停軸被拒：" + CmdWhy(res);
        }
        J.orgStopNote = "第一個樣本（" + Z.alias + " " + what + "）就先停了 " + (names.empty() ? std::string("（沒有還在走的 X／Y）") : names) +
                        "（HT160 MoveSortArmX :733：第一次讀到就先停 X；NB2 R174）" + (refused.empty() ? std::string() : "；⚠ " + refused);
        g_lastJobNote = "arm cell " + ArmCellWhere() + ": " + J.orgStopNote + " -- the next fresh sample decides (still dark: cancelled; lit again: cancelled, not resumed)";
    }
    if (!J.orgStop) return false;
    if (!darkNow)
        ArmCellEnd(be, "cancelled", "S3 X／Y 走的時候 Z 的原點（ORG）閃了一下：" + J.orgStopNote +
                                    "；下一個新樣本原點又亮了 —— X／Y 已停在半路，不自動續走（要重走請再按 GO）", true);
    return true;
}

//AI(W906-ARMCELL-ZORG) 20261003: NB2 R174 M1 (2) -- the same watch right after each 1203 Poll (MotorAccessPollTick: wb_serve calls
//  it on the tick thread right after Pci1203Monitor()->Poll(), kIoTickMs 200), so the first dark sample stops X / Y at the Poll
//  that read it, not at the next 500 ms beat (before: two dark samples, both judged on beats, ~0.5-1.0 s with X / Y moving).
//  Safe there: (a) the data is that Poll's -- MotorAccessTeachHomeLed reads Pci1203MotionIO, the monitor sample Poll() just
//  refreshed (the live motionIO and pollCount change only in Poll); (b) no double action with the beat -- one thread, one
//  sample counted once, the travel stop latched per job, ArmCellEnd once; (c) it can only stop or end the job, never command a
//  move; (d) only in S3 once X / Y were issued (S1 / S2 lift the Zs; in S4 the chosen Z goes down, its ORG dark by design).
//  The other gates, ArmCellZStillSafe, the legs and the timeouts stay on the beat.
void ArmCellPollTick(IMotorAccessBackend& be)
{
    if (!g_cell.active || g_cell.step != 3 || (g_cell.x.state == 0 && g_cell.y.state == 0)) return;
    ArmCellOrgWatch(be);
}

void TickArmCellXY(IMotorAccessBackend& be, double now)
{
    ArmCellJob& J = g_cell;
    if (J.x.state == 0 && J.y.state == 0) {
        MotorGolden g;
        if (be.GoldenMotor(J.x.a.motIndex, g)) J.x.g = g;
        if (be.GoldenMotor(J.y.a.motIndex, g)) J.y.g = g;
        if (SampleStale(be, J.x.a.axis) || SampleStale(be, J.y.a.axis)) {
            if (now - J.stepStartMs > kArmCellXYTimeoutMs) ArmCellEnd(be, "timeout", "S3 X／Y：120 秒沒有新的監看器樣本", true);
            return;
        }
        std::string zw0;                                                        // review m1: the Z again, in the same beat, right before X / Y go
        if (!ArmCellZStillSafe(be, zw0)) { ArmCellEnd(be, "cancelled", "S3 下 X／Y 之前：" + zw0 + "（Z 要停在 ZSafePos；X／Y 都還沒動）", true); return; }
        const std::string bx = MoveBlock1203(be, J.x.a, J.x.g, "Arm Cell " + J.x.alias, J.x.target);
        const std::string by = MoveBlock1203(be, J.y.a, J.y.g, "Arm Cell " + J.y.alias, J.y.target);
        if (!bx.empty() || !by.empty()) { ArmCellEnd(be, "failed", "S3 X／Y：" + (bx.empty() ? by : bx) + "（X／Y 都還沒動）", true); return; }
        std::string why;
        if (ArmCellIssue(be, J.x, 20, why) != 1) { ArmCellEnd(be, "failed", "S3 X：" + why + "（X／Y 都還沒動）", true); return; }
        if (ArmCellIssue(be, J.y, 20, why) != 1) {
            Stop1203(be, J.x.a.axis, -1);
            ArmCellEnd(be, "failed", "S3 Y 下不出去：" + why + "；X 已下命令，已停 X", true);
            return;
        }
        return;
    }
    std::string zw;
    if (!ArmCellZStillSafe(be, zw)) { ArmCellEnd(be, "cancelled", "S3 X／Y 走的時候 " + zw + "（Z 要停在 ZSafePos）", true); return; }
    //AI(W906-ARMCELL2) 20261002: NB2 R165 (4) -- every Z keeps the origin S2 saw while X / Y move (HT160 MoveSortArmX).
    //AI(W906-ARMCELL-ZORG) 20261003: NB2 R174 M1 -- the watch is ArmCellOrgWatch (above): the first dark sample stops X / Y at
    //  once, the next one ends the job; it also runs right after each 1203 Poll. While it holds the job no leg is judged here
    //  (never "arrived" after that stop); the S3 timeout still applies (a monitor that stops polling would otherwise hold it).
    if (ArmCellOrgWatch(be)) {
        if (J.active && now - J.stepStartMs > kArmCellXYTimeoutMs)
            ArmCellEnd(be, "timeout", "S3 X／Y：120 秒還沒到（Z 原點監看已先停 X／Y，等不到下一個新樣本）", true);
        return;
    }
    ArmCellLeg* L[2] = { &J.x, &J.y };
    for (int i = 0; i < 2; ++i) {
        if (L[i]->state == 2) continue;
        const int st = ArmCellPoll(be, *L[i], now);
        if (st == -1) { ArmCellEnd(be, "failed", "S3 X／Y：" + L[i]->alias + " ERROR_STOP（驅動器／卡片報錯）", true); return; }
        if (st == -2) { ArmCellEnd(be, "cancelled", "S3 X／Y：" + L[i]->alias + " 沒到目標就停了（連續兩次 READY 卻停在別處）", true); return; }
        if (st == -3) { ArmCellEnd(be, "failed", "S3 X／Y：" + ArmCellEncWhy(*L[i]), true); return; }   //AI(W906-ARMCELL2) 20261002
        if (st == 1) L[i]->state = 2;
    }
    if (J.x.state != 2 || J.y.state != 2) {
        if (now - J.stepStartMs > kArmCellXYTimeoutMs) ArmCellEnd(be, "timeout", "S3 X／Y：120 秒還沒到", true);
        return;
    }
    if (!J.zDown) { ArmCellArrive(be); return; }
    J.step = 4; J.stepStartMs = now;
}

void TickArmCellZDown(IMotorAccessBackend& be, double now)
{
    ArmCellJob& J = g_cell;
    ArmCellLeg& L = J.zSel;
    if (L.state == 0) {
        int px = 0, py = 0;                                                     // review m1: X and Y still READY at the cell (fresh sample) before the Z goes down
        const bool okx = ArmCellAt(be, J.x.a, J.x.g, J.x.target, J.x.gap, &px), oky = okx && ArmCellAt(be, J.y.a, J.y.g, J.y.target, J.y.gap, &py);
        if (!okx || !oky) {
            const ArmCellLeg& off = !okx ? J.x : J.y;
            ArmCellEnd(be, "cancelled", "S4 降 Z 之前：" + off.alias + " 不在格子上（目標 " + ArmCellNum(off.target) + "，現在 " + ArmCellNum(!okx ? px : py) +
                                        "，或不在 READY／沒有新樣本，fail-closed）—— 不降 Z", true);
            return;
        }
        if (J.plan.d2) {                                                        // RULINGS_20261002 #18 D2
            MotorAccessAxis d;
            MotorGolden dg;
            int pos = 0;
            bool stale = false;
            const bool resolved = be.Resolve(J.plan.d2Axis.alias, d) && d.Is1203() && d.axis >= 0 && d.motIndex >= 0 && be.GoldenMotor(d.motIndex, dg);
            const bool at = resolved && ArmCellAt(be, d, dg, J.plan.d2Target, 2, &pos, &stale);
            if (resolved && !at && stale) {
                if (now - J.stepStartMs > kZSafeTimeoutMs) ArmCellEnd(be, "timeout", "S4 D2：60 秒沒有 shuttle 的新樣本", true);
                return;
            }
            if (!at) {
                ArmCellEnd(be, "failed", "S4：X／Y 已到格子；D2 —— " + J.plan.d2What + "，但 " +
                                         (J.plan.d2Axis.alias.empty() ? std::string("shuttle 軸") : J.plan.d2Axis.alias) +
                                         (resolved ? " 不在那裡（目標 " + ArmCellNum(J.plan.d2Target) + "，現在 " + ArmCellNum(pos) + "，或不在 READY）"
                                                   : " 不是可判斷的 1203 軸") +
                                         "：不降 Z（RULINGS_20261002 第 18 條 D2）", false);
                return;
            }
            if (!J.d2Locked) {                                                  // review m3: D2 passed -> the shuttle is the job's until it ends (Motor Test can no longer move it)
                J.d2Locked = true; J.d2.alias = J.plan.d2Axis.alias; J.d2.a = d; J.d2.g = dg; J.d2.target = J.plan.d2Target; J.d2.state = 2;
            }
        }
        std::string why;
        const int rc = ArmCellIssue(be, L, 1, why);                             // golden GoButton140Click SetSpeed(1)
        if (rc < 0) { ArmCellEnd(be, "failed", "S4 Z 下降：" + why, true); return; }
        if (rc == 0 && now - J.stepStartMs > kZSafeTimeoutMs) ArmCellEnd(be, "timeout", "S4 Z 下降：60 秒沒有新的監看器樣本", true);
        return;
    }
    const int st = ArmCellPoll(be, L, now);
    if (st == -1) { ArmCellEnd(be, "failed", "S4 Z 下降：" + L.alias + " ERROR_STOP（驅動器／卡片報錯）", true); return; }
    if (st == -2) { ArmCellEnd(be, "cancelled", "S4 Z 下降：" + L.alias + " 沒到目標就停了（連續兩次 READY 卻停在別處）", true); return; }
    if (st == -3) { ArmCellEnd(be, "failed", "S4 Z 下降：" + ArmCellEncWhy(L), true); return; }   //AI(W906-ARMCELL2) 20261002
    if (st == 1) { L.state = 2; ArmCellArrive(be); return; }
    if (now - J.stepStartMs > kZSafeTimeoutMs) ArmCellEnd(be, "timeout", "S4 Z 下降：60 秒還沒到", true);
}

void TickArmCell(IMotorAccessBackend& be)
{
    if (!g_cell.active) return;
    if (be.GoldenSystemStart()) { ArmCellEnd(be, "cancelled", "SystemStart 變成 true（機台開始運轉）", true); return; }
    std::vector<ArmCellLeg*> legs = ArmCellLegs();
    for (std::size_t i = 0; i < legs.size(); ++i)
        if (be.GoldenSafeDoorOpen(legs[i]->a.motIndex)) {
            ArmCellEnd(be, "cancelled", "安全門開著（golden CheckIsSafeDoorOpen，" + legs[i]->alias + "）", true);
            return;
        }
    //AI(W906-ARMCELL2) 20261002: NB2 R165 (1) / (2) -- HT160 tmrUpdate on every beat: a box up (IsFaultPopupShowing), an EMG
    //  sensor off, an ALM / EMG lamp on any axis of the job -> cancelled, the job's axes stopped (ERROR_STOP: the step's "failed").
    std::string pw;
    if (g_W906ArmCellPopupUp && g_W906ArmCellPopupUp(pw)) {
        ArmCellEnd(be, "cancelled", pw + " —— 照 HT160 IsFaultPopupShowing 中止點位測試（NB2 R165）", true);
        return;
    }
    if (be.GoldenInitMotorEmgOff()) {
        ArmCellEnd(be, "cancelled", "急停 EMG 被按下（四顆 EMG 感測器之一 Off，golden InitMotor :180-181）—— 照 HT160 tmrUpdate：EMG ⇒ 全停", true);
        return;
    }
    for (std::size_t i = 0; i < legs.size(); ++i) {
        const std::string lw = ArmCellLampWhy(be, *legs[i], false);
        if (!lw.empty()) { ArmCellEnd(be, "cancelled", lw + " —— 照 HT160 tmrUpdate：伺服警報／EMG ⇒ 全停（NB2 R165）", true); return; }
    }
    const double now = LoopNow(be);
    switch (g_cell.step) {
    case 1: TickArmCellZUp(be, now);    break;
    case 2: TickArmCellVerify(be, now); break;
    case 3: TickArmCellXY(be, now);     break;
    case 4: TickArmCellZDown(be, now);  break;
    default: break;
    }
}

void ArmCellWritePlan(webbridge::JsonWriter& w, const ArmCellPlan& p, bool zDown)
{
    w.Key("plan").BeginObject();
    w.Key("arm").String(p.arm);
    w.Key("area").String(p.area);
    w.Key("label").String(p.label);
    w.Key("nozzle").String(p.nozzle);
    w.Key("col").Number((wb_int64)p.col);
    w.Key("row").Number((wb_int64)p.row);
    w.Key("zDown").Bool(zDown);
    w.Key("x").BeginObject(); w.Key("motor").String(p.x.alias); w.Key("target").Number((wb_int64)p.xTarget); w.EndObject();
    w.Key("y").BeginObject(); w.Key("motor").String(p.y.alias); w.Key("target").Number((wb_int64)p.yTarget); w.EndObject();
    w.Key("z").BeginObject();
    w.Key("motor").String(p.z.alias);
    w.Key("target").Number((wb_int64)p.zTarget);
    w.Key("kind").String(p.zKind);
    w.Key("allowed").Bool(p.zDownAllowed);
    w.Key("why").String(p.zDownWhy);
    w.EndObject();
    w.Key("zSafe").Number((wb_int64)p.zSafe);
    w.Key("zLift").BeginArray();
    for (std::size_t i = 0; i < p.zLift.size(); ++i) w.String(p.zLift[i].alias);
    w.EndArray();
    w.Key("teach").BeginObject(); w.Key("x").Number((wb_int64)p.teachX); w.Key("y").Number((wb_int64)p.teachY); w.Key("z").Number((wb_int64)p.teachZ); w.EndObject();
    w.Key("base").BeginObject(); w.Key("x").Number((wb_int64)p.baseX); w.Key("y").Number((wb_int64)p.baseY); w.EndObject();
    w.Key("prod");
    if (p.hasProd) { w.BeginObject(); w.Key("x").Number((wb_int64)p.prodBaseX); w.Key("y").Number((wb_int64)p.prodBaseY); w.EndObject(); }
    else w.Null();
    w.Key("cell").BeginObject(); w.Key("x").Number((wb_int64)p.cellX); w.Key("y").Number((wb_int64)p.cellY); w.EndObject();
    w.Key("d2");
    if (p.d2) {
        w.BeginObject();
        w.Key("motor").String(p.d2Axis.alias);
        w.Key("target").Number((wb_int64)p.d2Target);
        w.Key("what").String(p.d2What);
        w.EndObject();
    } else {
        w.Null();
    }
    w.Key("notes").BeginArray();
    for (std::size_t i = 0; i < p.notes.size(); ++i) w.String(p.notes[i]);
    w.EndArray();
    w.EndObject();
}

MotorAccessOutcome DoArmCell(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    const std::string who = "moveToTrayCell（Teach Arm Cell）";
    if (g_cell.active) return Refuse(who + ": 已經有一個 Arm Cell 在走（" + ArmCellWhere() + "）—— 先按 STOP 或等它走完");
    if (AnyJobActive() || !g_jogs.empty() || g_ls.timer || g_powerPending || g_teachSet.active)
        return Refuse(who + ": 還有別的工作在跑（HOME／LoopMove／JOG／Light Scale／Motor Power／手動教導／Motor Test Gear Ratio 量測）—— 等它結束或按 STOP");   //AI(W906-GEARRATIO) 20261002: + the Gear Ratio session (AnyJobActive)
    if (be.GoldenMotorPowerOff()) return Refuse(who + ": Motor power is OFF!! 電源被關閉!!（golden uteach btnHomeClick :2137 的同一道）");
    ArmCellRequest q;
    std::map<std::string, std::string>::const_iterator s;
    if ((s = r.str.find("arm")) != r.str.end()) q.arm = s->second;
    if ((s = r.str.find("nozzle")) != r.str.end()) q.nozzle = s->second;
    if ((s = r.str.find("area")) != r.str.end()) q.area = s->second;
    double col = 0.0, row = 0.0;
    if (!ParamNum(r, "col", col) || !ParamNum(r, "row", row) || !IsIntValue(col) || !IsIntValue(row))
        return Refuse(who + ": 缺 col／row（0 起算的整數）");
    q.col = (int)col; q.row = (int)row;
    std::map<std::string, bool>::const_iterator zf = r.flag.find("zDown");
    q.zDown = (zf != r.flag.end() && zf->second);
    ArmCellPlan p;
    std::string why;
    if (!be.GoldenArmCellPlan(q, p, why)) return Refuse(who + ": " + why);
    if (q.zDown && !p.zDownAllowed) return Refuse(who + ": " + p.label + " 不能降 Z —— " + p.zDownWhy);
    ArmCellJob J;
    J.plan = p; J.zDown = q.zDown; J.seq = r.seq; J.reqId = r.id; J.wireId = wireId;
    if (!ArmCellLegOf(p.x.alias, wireId, be, J.x, why) || !ArmCellLegOf(p.y.alias, wireId, be, J.y, why)) return Refuse(why);
    for (std::size_t i = 0; i < p.zLift.size(); ++i) {
        ArmCellLeg L;
        if (!ArmCellLegOf(p.zLift[i].alias, wireId, be, L, why)) return Refuse(why);
        L.target = p.zSafe;
        J.zs.push_back(L);
    }
    if (!ArmCellLegOf(p.z.alias, wireId, be, J.zSel, why)) return Refuse(why);
    J.x.target = p.xTarget; J.y.target = p.yTarget; J.zSel.target = p.zTarget;
    std::vector<const ArmCellLeg*> all;
    all.push_back(&J.x); all.push_back(&J.y);
    for (std::size_t i = 0; i < J.zs.size(); ++i) all.push_back(&J.zs[i]);
    for (std::size_t i = 0; i < all.size(); ++i)
        if (all[i]->g.homeFlag != 1)
            return Refuse(who + ": " + all[i]->alias + " 還沒回原點（HomeFlag=" + ArmCellNum(all[i]->g.homeFlag) + "）—— 請先按 HOME 或 In／Out Z All Up（NB2 規格 §3.3 S0）");
    //AI(W906-ARMCELL2) 20261002: NB2 R165 (1) / (2) -- not while a box is up (HT160 IsFaultPopupShowing), an EMG sensor is off, or
    //  an axis it will command shows its alarm / EMG lamp (HT160 CheckSortArmTestReady :1351-1353 iAlarmLed / iServoalarmLed, as golden
    //  decodes them for the 1203 -- ArmCellLampWhy) -- an explicit check, not only the 1203 READY state MoveBlock1203 reads later.
    {
        std::string pw;
        if (g_W906ArmCellPopupUp && g_W906ArmCellPopupUp(pw))
            return Refuse(who + ": " + pw + " —— 先處理、關掉它再按 GO（HT160 IsFaultPopupShowing：框開著不做點位測試；NB2 R165）");
        if (be.GoldenInitMotorEmgOff())
            return Refuse(who + ": 急停 EMG 被按下（四顆 EMG 感測器之一 Off，golden InitMotor :180-181）—— 先解除 EMG");
        std::vector<const ArmCellLeg*> lamps(all);
        lamps.push_back(&J.zSel);
        for (std::size_t i = 0; i < lamps.size(); ++i) {
            const std::string lw = ArmCellLampWhy(be, *lamps[i], true);
            if (!lw.empty()) return Refuse(who + ": " + lw + " —— 先排除再按 GO（HT160 CheckSortArmTestReady :1351-1353；NB2 R165），一軸都還沒動");
        }
    }
    // every target against the soft limits before anything moves (MoveBlock1203's >= / <=)
    if (q.zDown) all.push_back(&J.zSel);
    for (std::size_t i = 0; i < all.size(); ++i) {
        const ArmCellLeg& L = *all[i];
        char b[240];
        if (L.target >= L.g.softP) {
            std::snprintf(b, sizeof(b), " 的目標 %d 超過正向軟體極限 %d", L.target, L.g.softP);
            return Refuse(who + ": " + L.alias + b + "（所有目標先一起檢查，一軸都還沒動）");
        }
        if (L.target <= L.g.softN) {
            std::snprintf(b, sizeof(b), " 的目標 %d 低於負向軟體極限 %d", L.target, L.g.softN);
            return Refuse(who + ": " + L.alias + b + "（所有目標先一起檢查，一軸都還沒動）");
        }
    }
    J.active = true;
    J.jobId = ++g_cellSeq;
    J.step = 1;
    J.stepStartMs = LoopNow(be);
    J.result = "running";
    J.startedAt = NowIso();
    g_cell = J;
    g_lastJobNote = "arm cell " + ArmCellWhere() + " started";
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "cellMoving", "pci1203",
            "Arm Cell 受理：" + p.label + " (" + ArmCellNum(p.col + 1) + ", " + ArmCellNum(p.row + 1) + ")，" + p.nozzle +
            " → X " + ArmCellNum(p.xTarget) + "、Y " + ArmCellNum(p.yTarget) + (q.zDown ? "、Z " + ArmCellNum(p.zTarget) : std::string("（不降 Z）")) +
            "；先把 Z 抬到 ZSafePos " + ArmCellNum(p.zSafe) + "（受理不等於到位：到位看 runtime armCell.job.result）");
    w.Key("cellActive").Bool(true);
    w.Key("cellSeq").Number((wb_int64)g_cell.jobId);
    w.Key("step").Number((wb_int64)g_cell.step);
    w.Key("stepName").String(ArmCellStepName(g_cell.step));
    ArmCellWritePlan(w, p, q.zDown);
    w.Key("note").String(p.d1Note);
    w.Key("steps").BeginArray();
    for (int k = 1; k <= 5; ++k) if (k != 4 || q.zDown) w.String(ArmCellStepName(k));
    w.EndArray();
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

void ArmCellWriteAxis(webbridge::JsonWriter& w, const ArmCellAxis& a)
{
    w.BeginObject();
    w.Key("motor").String(a.alias);
    w.Key("mi").Number((wb_int64)a.mi);
    w.Key("present").Bool(a.present);
    w.Key("enable").Bool(a.enable);
    w.Key("pci1203").Bool(a.is1203);
    w.EndObject();
}

// Q3 (RULINGS_20261002 #18): zDown unchecked by default while an arm Z is a PCI1203 axis whose ORG polarity is unmeasured
// (kPci1203CardOrgLogic == -1) or whose soft limits are still the +-999999 placeholders -- decided from data, never from the
// machine-type name (9050GPIB decodes as Type_HT9046_LS, RULINGS_20260926 #25).
//AI(W906-ARMCELL) 20261002: review m4 -- it does NOT flip by itself on HT9050: kPci1203CardOrgLogic is a compile-time constant
//  (-1 today) that HT9050's ORG branch in MotorAccessTeachHomeLed never reads (EastSun's ORG-low rule; per-axis Mot_Table
//  SensorType once MR !119 is in), so the ORG half stays true there whatever ES02 measures. Only the soft-limit half follows
//  the data (real limits in Mot_Table). Turning the default on for HT9050 needs a decision on what Q3's ORG half reads.
bool ArmCellZDownDefault(const ArmCellCatalog& cat, IMotorAccessBackend& be, std::string& why)
{
    for (int arm = 0; arm < 2; ++arm)
        for (std::size_t i = 0; i < cat.nozzles[arm].size(); ++i) {
            const ArmCellAxis& z = cat.nozzles[arm][i].z;
            if (!z.is1203) continue;
            MotorGolden g;
            const bool hasG = z.mi >= 0 && be.GoldenMotor(z.mi, g);
            const bool placeholder = !hasG || (g.softP >= 999999 && g.softN <= -999999);
            if (kPci1203CardOrgLogic == -1 || placeholder) {
                why = z.alias + " 是 PCI1203 軸，" +
                      std::string(kPci1203CardOrgLogic == -1 ? "ORG 極性常數 kPci1203CardOrgLogic=-1（程式常數，不會自己變）" : "") +
                      std::string(kPci1203CardOrgLogic == -1 && placeholder ? "、" : "") +
                      std::string(placeholder ? "軟體極限還是 ±999999 佔位（Mot_Table 換成真值後這一條解除）" : "") +
                      " —— 降 Z 預設不勾（RULINGS_20261002 第 18 條 Q3）";   // review m4: no "turns on by itself" promise
                return false;
            }
        }
    why.clear();
    return true;
}

}  // namespace

// The armCell block of /api/struct/motor/runtime (NB2 spec §3.4) -- built on the tick thread (WebMotorAccessLive.cpp
// OverlaySnapshot) into the page-state copy the HTTP thread reads. catalog.ok=false (+why) when the backend has none: the page
// greys the whole pane, it never falls back to a static snapshot (W5B-7).
std::string MotorAccessArmCellJson(IMotorAccessBackend& be)
{
    webbridge::JsonWriter c;
    ArmCellCatalog cat;
    std::string why;
    const bool ok = be.GoldenArmCellCatalog(cat, why);
    c.BeginObject();
    c.Key("ok").Bool(ok);
    c.Key("why").String(ok ? std::string() : why);
    if (ok) {
        for (int arm = 0; arm < 2; ++arm) {
            c.Key(arm == 0 ? "in" : "out").BeginObject();
            c.Key("x"); ArmCellWriteAxis(c, cat.x[arm]);
            c.Key("y"); ArmCellWriteAxis(c, cat.y[arm]);
            c.Key("nozzles").BeginArray();
            for (std::size_t i = 0; i < cat.nozzles[arm].size(); ++i) {
                const ArmCellNozzleInfo& n = cat.nozzles[arm][i];
                c.BeginObject();
                c.Key("i").Number((wb_int64)n.i);
                c.Key("j").Number((wb_int64)n.j);
                c.Key("alias").String(n.z.alias);
                c.Key("pci1203").Bool(n.z.is1203);
                c.EndObject();
            }
            c.EndArray();
            c.Key("note").String(cat.note[arm]);
            c.Key("why").String(cat.armWhy[arm]);
            c.EndObject();
        }
        c.Key("areas").BeginArray();
        for (std::size_t i = 0; i < cat.areas.size(); ++i) {
            const ArmCellArea& a = cat.areas[i];
            c.BeginObject();
            c.Key("arm").String(a.arm);
            c.Key("id").String(a.id);
            c.Key("label").String(a.label);
            c.Key("v1").Bool(a.v1);
            c.Key("installed").Bool(a.installed);
            c.Key("usable").Bool(a.usable);
            c.Key("why").String(a.why);
            c.Key("cols").Number((wb_int64)a.cols);
            c.Key("rows").Number((wb_int64)a.rows);
            c.Key("zDownAllowed").Bool(a.zDownAllowed);
            c.Key("zDownWhy").String(a.zDownWhy);
            c.Key("zKind").String(a.zKind);
            c.EndObject();
        }
        c.EndArray();
        std::string dw;
        const bool dd = ArmCellZDownDefault(cat, be, dw);
        c.Key("zDownDefault").Bool(dd);
        c.Key("zDownWhy").String(dw);
        c.Key("zSafePos").Number((wb_int64)cat.zSafePos);
    }
    c.EndObject();
    const std::string catText = c.Ok() ? c.Str() : std::string("{\"ok\":false,\"why\":\"json writer misuse\"}");
    if (catText != g_cellCatalogLast) { g_cellCatalogLast = catText; ++g_cellCatalogRev; }

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("catalogRev").Number((wb_int64)g_cellCatalogRev);
    w.Key("catalog").RawValue(catText);
    w.Key("job").BeginObject();
    const ArmCellJob& J = g_cell;
    w.Key("active").Bool(J.active);
    w.Key("jobId").Number((wb_int64)J.jobId);
    w.Key("seq").Number((wb_int64)J.seq);
    w.Key("reqId").String(J.reqId);
    w.Key("step").Number((wb_int64)J.step);
    w.Key("stepName").String(J.jobId ? ArmCellStepName(J.step) : "");
    w.Key("result").String(J.result);
    w.Key("why").String(J.why);
    w.Key("arm").String(J.plan.arm);
    w.Key("area").String(J.plan.area);
    w.Key("label").String(J.plan.label);
    w.Key("nozzle").String(J.plan.nozzle);
    w.Key("col").Number((wb_int64)J.plan.col);
    w.Key("row").Number((wb_int64)J.plan.row);
    w.Key("zDown").Bool(J.zDown);
    w.Key("target").BeginObject();
    w.Key("x").Number((wb_int64)J.plan.xTarget);
    w.Key("y").Number((wb_int64)J.plan.yTarget);
    w.Key("zSafe").Number((wb_int64)J.plan.zSafe);
    w.Key("z"); if (J.zDown) w.Number((wb_int64)J.plan.zTarget); else w.Null();
    w.EndObject();
    w.Key("activeMotor"); if (J.activeMotor.empty()) w.Null(); else w.String(J.activeMotor);
    w.Key("motors").BeginArray();
    if (J.jobId) { w.String(J.x.alias); w.String(J.y.alias); for (std::size_t i = 0; i < J.zs.size(); ++i) w.String(J.zs[i].alias); if (J.d2Locked) w.String(J.d2.alias); }
    w.EndArray();
    w.Key("final");
    if (J.hasFinal) {
        static const char* const kN[3] = { "x", "y", "z" };
        w.BeginObject();
        for (int i = 0; i < 3; ++i) {
            w.Key(kN[i]);
            if (J.finalKnown[i]) {
                w.BeginObject();
                w.Key("cmdPos").Number((wb_int64)J.finalCmd[i]);
                w.Key("actPos").Number((wb_int64)J.finalAct[i]);
                w.EndObject();
            } else {
                w.Null();
            }
        }
        w.EndObject();
    } else {
        w.Null();
    }
    w.Key("stop");                                                              // review N5: the end's stops, DoStop-style (null = the end sent none)
    if (J.stopped) {
        w.BeginObject();
        w.Key("sent").Number((wb_int64)J.stopSent);
        w.Key("accepted").Number((wb_int64)J.stopAccepted);
        w.Key("refused").Number((wb_int64)J.stopRefused);
        w.Key("why").String(J.stopWhy);
        w.Key("partial").Bool(J.stopRefused > 0);
        w.EndObject();
    } else {
        w.Null();
    }
    w.Key("startedAt").String(J.startedAt);
    w.Key("endedAt").String(J.endedAt);
    w.EndObject();
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

//AI(W906-ARMCELL) 20261002: review m5 -- the refusal of an action that has to wait for a running Arm Cell job ("" = none runs).
//  START (MotorAccessStartBlocked) and wb_serve's main.home (W906_MotorAccessArmCellBlocked, WebMotorAccessLive.cpp EOF) use it.
std::string MotorAccessArmCellBlockedWhy(const std::string& what)
{
    if (!g_cell.active) return std::string();
    return what + " 拒絕：教導頁 Arm Cell 進行中（" + ArmCellWhere() + "）—— 等它走完或按 STOP（RULINGS_20261002 第 18 條）";
}

//AI(W906-ARMCELL2) 20261002: NB2 R165 (1) -- see WebMotorAccess.h. The edge half of HT160's IsFaultPopupShowing -> StopAllAdvancedTests;
//  the box's own golden stop (StopAllMotor, SystemStart=false ...) is the host's, unchanged -- this only ends the job and stops its axes.
bool (*g_W906ArmCellPopupUp)(std::string& what) = 0;
bool MotorAccessArmCellOnPopup(IMotorAccessBackend& be, const std::string& kind, const std::string& what)
{
    if (!g_cell.active || g_cell.inGoldenCheck) return false;
    ArmCellEnd(be, "cancelled", kind + "「" + what + "」出現 —— 照 HT160 IsFaultPopupShowing 中止點位測試（NB2 R165）", true);
    return true;
}

MotorAccessArmCellState MotorAccessArmCell()
{
    MotorAccessArmCellState s;
    const ArmCellJob& J = g_cell;
    s.active = J.active; s.jobId = J.jobId; s.seq = J.seq; s.step = J.step;
    s.stepName = J.jobId ? ArmCellStepName(J.step) : "";
    s.result = J.result; s.why = J.why; s.activeMotor = J.activeMotor;
    s.arm = J.plan.arm; s.area = J.plan.area; s.nozzle = J.plan.nozzle; s.col = J.plan.col; s.row = J.plan.row; s.zDown = J.zDown;
    s.xTarget = J.plan.xTarget; s.yTarget = J.plan.yTarget; s.zSafe = J.plan.zSafe; s.zTarget = J.plan.zTarget;
    if (J.jobId) { s.motors.push_back(J.x.alias); s.motors.push_back(J.y.alias); for (std::size_t i = 0; i < J.zs.size(); ++i) s.motors.push_back(J.zs[i].alias); if (J.d2Locked) s.motors.push_back(J.d2.alias); }
    s.hasFinal = J.hasFinal;
    s.stopped = J.stopped; s.stopSent = J.stopSent; s.stopAccepted = J.stopAccepted; s.stopRefused = J.stopRefused; s.stopWhy = J.stopWhy;   // review N5
    for (int i = 0; i < 3; ++i) { s.finalCmd[i] = J.finalCmd[i]; s.finalAct[i] = J.finalAct[i]; s.finalKnown[i] = J.finalKnown[i]; }
    return s;
}

}  // namespace ht9045

//AI(W906-BOOT-INITMOTOR) 20261002: EastSun 1002「一開始執行的時候開卡完就要進行馬達送電後就要開始進行InitMotor ... 請先以 M35 MLoaderZ」.
//  golden TMyEtherCatMotor::InitMotor (myEthercatmotor.cpp:172-394) on the monitor's handles, the same body Test Range / Test Rate
//  run (InitMotor1203 above, EastSun R4): ResetError until READY (bounded), the closed CFG table (CFG_AxOrgLogic from SensorType,
//  CFG_AxAlmLogic from In1Logic, ...), CFG_AxMaxVel/Acc/Dec, ResetError, SvOn 1, SetCommand(0)/SetPosition(0). Called once per
//  wb_serve run per axis by the live tick (WebMotorAccessLive.cpp EOF, W906_BootInitMotorTick) after the card is open and the
//  motor power is on. report = one line per step for the op log.
namespace ht9045 {
bool MotorAccessBootInitMotor(IMotorAccessBackend& be, const std::string& alias, std::string& report)
{
    MotionCtx m;
    if (!be.Resolve(alias, m.a)) { report = alias + ": not in Mot_Table"; return false; }
    if (!m.a.Is1203()) { report = alias + ": not a PCI1203 row (" + m.a.cardModel + ")"; return false; }
    if (m.a.motIndex < 0 || !be.GoldenMotor(m.a.motIndex, m.g)) { report = alias + ": no golden motor object (MOT[].Motor NULL)"; return false; }
    std::vector<InitStep> steps;
    std::string stopWhy;
    const bool ran = InitMotor1203(be, 0, m, steps, stopWhy);
    int fails = 0;
    report = alias + " InitMotor (boot): ";
    for (std::size_t i = 0; i < steps.size(); ++i) {
        if (!steps[i].ok) ++fails;
        report += (i ? " | " : "") + steps[i].name + (steps[i].ok ? " ok" : " FAIL") + (steps[i].note.empty() ? std::string() : " (" + steps[i].note + ")");
    }
    if (!ran) report += (steps.empty() ? "" : " | ") + std::string("STOPPED: ") + stopWhy;
    else report += " -- done, " + std::to_string(fails) + " step(s) failed";
    return ran && fails == 0;
}
}  // namespace ht9045

// =============================================================================
//AI(W906-ST02-C9-G2) 20261002 (St02-E helper): St02 card ST02-C9 group G2 (docs/handoff/TO_STEVEN.md 20261002 07:1x) --
//  golden uteach.cpp buttons the laptop has not wired, golden = HT9011UC_Code_V3.33.906.0_20260625_Steven:
//    SpeedButtonInRotatePos90Click :4605-4655 / SpeedButtonInRotateNeg90Click :4657-4707 -- the OnClick of 36 buttons
//      (uteach.dfm: the In / Out kit pairs in grpRotate_Kit, btnPos90 / btnNeg90 In / Out R A..H on tsRotate; St02RotGroups)
//    btnSht1GoLatchClick :4721-4735 / btnSht2GoLatchClick :4737-4751
//    btnSetAllInArmZ_MoveClick :5948-5971 / btnSetAllOutArmZ_MoveClick :5973-5996
//  AI(W906-ST02-C9) 20261002 (St02-E helper): St02's group G3 (BtnPanelLane1Click, btnArm1YServoClick, btnZ1 / Z2ServoClick,
//    spTTLResetClick / cbEnableTTLButtonUseClick) was dropped in the C9 rebase onto main 2dd90ef3: the machine wired them on main
//    first (IOWIDGET 60cc29f6, TEACH-ZALLUP 71132e21 -- teachIndexServo above -- and TEACH-FORMSHOW 00906385 greyed the TTL box).
//  ONE block, appended at the end after the machine's TEACH-ZALLUP block (St02's; the laptop's 20261002 07:42 placement rules). It drives the W5-b teach-move path
//  above (TeachPrelude / TeachCtxOfIndex / MotionPreludeFor / CurrentUserPos / TeachRotatorBacklash / Move1203 / MoveGolden /
//  TeachCurPct) and adds NOTHING to IMotorAccessBackend: what golden needs beyond it is ITeachSt02Ops (WebTeachSt02.h; live =
//  WebTeachSt02Live.cpp, wb_serve only). Hooks into this file (claims, same-line appends): the kActions row "teachSt02" (on
//  the teachGo row) and one dispatch line in MotorAccessDispatch's uteach block (on the teachSet line), so the dispatcher's
//  gates apply as to every other uteach motion: SystemStart refused, manual teach (fTeachShow) refused, STOP / the page
//  close edge = DoStop (every axis stopped). G2 has no cross-tick job: each press is one golden MotorMove per axis.
//  [W906] golden visibility: a button golden hides on this configuration (FormShow :1457-1458 / :1652 / :1668 / :1690-1703,
//  uteach.dfm:5850) is refused here -- the W5-b R-W5B-2 rule: golden cannot press it, so nothing happens -- and the page
//  hides it from the St02State answer. HT9050 (machines/HT9050/sim_9378/Gerneral.ini: USE_ROTATE_KIT=0, USE_PICKER_COUNT=4)
//  hides every G2 button (USE_ROTATE_KIT=0 hides grpRotate_Kit and the tsRotate tab both).
// =============================================================================
#include "WebTeachSt02.h"

namespace ht9045 {
namespace {

ITeachSt02Ops* g_st02Ops = 0;                    // WebTeachSt02Live.cpp installs it (wb_serve); 0 = every teachSt02 refused
bool g_st02SetAllZB[2] = { false, false };       // golden `static bool b=false;` btnSetAllInArmZ_MoveClick :5950 / ...OutArmZ_... :5975
int  g_st02ShuttleSelect = 0;                    // golden `static int iShuttleSelect=0;` (uteach.cpp:4709)

std::string St02BtnOf(const MotorAccessReq& r)   // params.btn (the golden button name), else the request's button
{
    std::map<std::string, std::string>::const_iterator b = r.str.find("btn");
    return (b != r.str.end() && !b->second.empty()) ? b->second : r.button;
}

// AckHead's keys in AckHead's order, with the state chosen: a press that refused some axes answers "error" (motor-access.js
//   finish() takes ack.state), so a partial result is not shown green (the NB2 R19 RW4-2 rule for STOP, here for a move).
void St02Head(webbridge::JsonWriter& w, const MotorAccessReq& r, const char* state, const std::string& result,
              const std::string& layer, const std::string& message)
{
    w.Key("seq").Number((wb_int64)r.seq);
    w.Key("reqId").String(r.id);                                                // never "id" (AckHead: WebBridgeServer::AckJson flattens the ack)
    w.Key("state").String(state);
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

// the keys of `extra` (a JsonWriter object) into the object `ack` -- AddEcho's way (no key of extra is in Move1203's ack)
std::string St02Merge(const std::string& ack, const std::string& extra)
{
    if (ack.size() < 2 || ack[ack.size() - 1] != '}' || extra.size() <= 2) return ack;
    std::string out = ack;
    out.insert(out.size() - 1, std::string(ack.size() > 2 ? "," : "") + extra.substr(1, extra.size() - 2));
    return out;
}

void St02AckResult(const std::string& ack, std::string& result, std::string& message)
{
    result.clear(); message.clear();
    cJSON* root = cJSON_Parse(ack.c_str());
    if (root == 0) return;
    const cJSON* jr = cJSON_GetObjectItemCaseSensitive(root, "result");
    const cJSON* jm = cJSON_GetObjectItemCaseSensitive(root, "message");
    if (cJSON_IsString(jr) && jr->valuestring) result = jr->valuestring;
    if (cJSON_IsString(jm) && jm->valuestring) message = jm->valuestring;
    cJSON_Delete(root);
}

MotorAccessOutcome St02Ok(const std::string& json) { MotorAccessOutcome o; o.ok = true; o.ackJson = json; return o; }

//AI(W906-ST02-C9-G2) 20261002 (St02-E helper): the 36 buttons of golden uteach.dfm whose OnClick is SpeedButtonInRotatePos90Click
//  / SpeedButtonInRotateNeg90Click, and the golden parent whose Visible decides each one:
//    TabSheet14 > grpRotate_Kit > grpInRotKit:  SpeedButtonInRotatePos90 :5575 / SpeedButtonInRotateNeg90 :5583
//    TabSheet14 > grpRotate_Kit > grpOutRotKit: SpeedButtonOutRotatepPos90 :5671 / SpeedButtonOutRotatepNeg90 :5679 (golden's "p")
//    tsRotate > grpInRotM8 > grbInRA..H:        btnPos90InRx / btnNeg90InRx   (:15310-15780)
//    tsRotate > grpOutRotM8 > grbOutRA..H:      btnPos90OutRx / btnNeg90OutRx (:15853-16323)
//  golden's handler never reads Sender: each of them moves ActiveMotorIndex (the page's selected motor) by +/-P2, and the
//  backlash depends on that motor (MInRotateKit / MOutRotateKit, :4639-4652), not on the button pressed.
//  Visible (FormShow): grpRotate_Kit :1652; tsRotate->TabVisible :1668; grbInRB .. grbOutRH :1690-1703. grbInRA / grbInRE have
//  no FormShow line (the dfm's Visible=True); TabSheet14 / grpInRotKit / grpOutRotKit / grpInRotM8 / grpOutRotM8 have none
//  either, and no dfm Visible (0625 grep of uteach.cpp / uteach.dfm; no other golden file writes them).
enum { kSt02GrbAlways = 0, kSt02GrbNot2Dut = 1, kSt02Grb8Only = 2, kSt02Grb8Or2Dut = 3 };
struct St02RotGroup { const char* group; int rule; int line; };
const St02RotGroup kSt02RotGroups[16] = {
    { "grbInRA",  kSt02GrbAlways,  0    }, { "grbInRB",  kSt02GrbNot2Dut, 1690 },
    { "grbInRC",  kSt02Grb8Only,   1691 }, { "grbInRD",  kSt02Grb8Only,   1692 },
    { "grbInRE",  kSt02GrbAlways,  0    }, { "grbInRF",  kSt02GrbNot2Dut, 1693 },
    { "grbInRG",  kSt02Grb8Only,   1694 }, { "grbInRH",  kSt02Grb8Only,   1695 },
    { "grbOutRA", kSt02GrbNot2Dut, 1696 }, { "grbOutRB", kSt02GrbNot2Dut, 1697 },
    { "grbOutRC", kSt02Grb8Or2Dut, 1698 }, { "grbOutRD", kSt02Grb8Only,   1699 },
    { "grbOutRE", kSt02GrbNot2Dut, 1700 }, { "grbOutRF", kSt02GrbNot2Dut, 1701 },
    { "grbOutRG", kSt02Grb8Or2Dut, 1702 }, { "grbOutRH", kSt02Grb8Only,   1703 },
};
const std::size_t kSt02RotGroupCount = sizeof(kSt02RotGroups) / sizeof(kSt02RotGroups[0]);
const St02RotGroup* St02RotGroupOf(const std::string& group)
{
    for (std::size_t i = 0; i < kSt02RotGroupCount; ++i) if (group == kSt02RotGroups[i].group) return &kSt02RotGroups[i];
    return 0;
}
std::string St02RotGroupRule(const St02RotGroup* g)                            // golden's right-hand side, for the refusal
{
    if (g == 0) return "?";
    switch (g->rule) {
    case kSt02GrbNot2Dut: return "((USE_ROTATE_KIT==1) && (iRotate_Type!=e2MotRotate2Dut))";
    case kSt02Grb8Only:   return "((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate))";
    case kSt02Grb8Or2Dut: return "((USE_ROTATE_KIT==1) && (iRotate_Type==e8MotRotate || iRotate_Type==e2MotRotate2Dut))";
    default:              return "True（沒有 FormShow 敘述，dfm 預設）";
    }
}

// [W906] a Mot_Table Enable=0 row of another card is not driven (a PCI1203 one is refused by MotionPreludeFor / Move1203):
//   golden TMyMotor::MotorMovePosition on Enable=false writes Position=Tar and returns 1 (golden Motor/mymotor.cpp:821-823,
//   real build) or simulates the walk (SOFT_SIMULTE :824-859) -- a success with no hardware behind it (honest rule, WebMotorAccess.h).
const char* const kSt02Enable0Why =
    "[W906] Enable=0（Mot_Table：這台沒裝這一軸）—— golden MotorMove 在 Enable=false 時只寫 MOT[i].Position=目標、回 1（mymotor.cpp:821-823，不碰硬體）；這裡不呼叫、什麼都不寫";

// ---- the page's state query: golden FormShow's visibility for these buttons + golden's form statics ----
MotorAccessOutcome St02StateAck(const MotorAccessReq& r, ITeachSt02Ops& ops)
{
    const TeachSt02Config c = ops.Config();
    webbridge::JsonWriter w;
    w.BeginObject();
    St02Head(w, r, "done", "st02State", "ui", "St02 ST02-C9: golden visibility (FormShow :1457-1458 / :1652 / :1668 / :1690-1703, uteach.dfm:5850) and form statics");
    w.Key("config").BeginObject();
    w.Key("useRotateKit").Number((wb_int64)c.useRotateKit);
    w.Key("rotateType").Number((wb_int64)c.rotateType);
    w.Key("usePickerCount").Number((wb_int64)c.usePickerCount);
    w.EndObject();
    w.Key("shown").BeginObject();
    w.Key("grpRotate_Kit").Bool(TeachSt02RotateKitShown(c));
    w.Key("pnlInArmZ").Bool(TeachSt02ArmZPanelShown(c));
    w.Key("pnlOutArmZ").Bool(TeachSt02ArmZPanelShown(c));
    w.Key("grpShtSensor").Bool(c.shtSensorGroupShown);
    w.Key("tsRotate").Bool(TeachSt02RotateTabShown(c));                        // the tab's TabVisible (:1668); the 16 groups' own Visible (:1690-1703)
    for (std::size_t i = 0; i < kSt02RotGroupCount; ++i) w.Key(kSt02RotGroups[i].group).Bool(TeachSt02RotateGroupShown(c, kSt02RotGroups[i].group));
    w.EndObject();
    w.Key("rotateP2").Number((wb_int64)TeachSt02RotateP2(c.rotateType));
    w.Key("setAllInArmZB").Bool(g_st02SetAllZB[0]);
    w.Key("setAllOutArmZB").Bool(g_st02SetAllZB[1]);
    w.Key("shuttleSelect").Number((wb_int64)g_st02ShuttleSelect);
    w.Key("groups").String("G2");                                              //AI(W906-ST02-C9) 20261002: G3 dropped (the machine's, on main)
    return St02Ok(Finish(w));
}

// ---- SpeedButtonInRotatePos90Click (uteach.cpp:4605-4655) / SpeedButtonInRotateNeg90Click (:4657-4707) ----
//   golden: Down=false; fTechAuto=false; `if(CheckCanMove()==false || ActiveMotorIndex==-1 || IsCanQuickJogMove()==false)
//   return;` P1=atoi(edtNowPosition->Text); P2 by iRotate_Type (800 / 1250 / 2000); abs(P1)>999999 -> ShowMyMessage, return;
//   only for MInRotateKit..H / MOutRotateKit..H: fCMD=false; iBacklash = MInRotateKit / MOutRotateKit ? GetRotatorBacklash(
//   P1±P2, bIsInRot, atoi(edtEditRotateIn/OutBacklash)) : 0; MotorMove(P1±P2+iBacklash). No speed is set (R-W5B-4).
//   [W906] Down=false / fTechAuto=false: the web button is not a toggle; fTechAuto is Auto Teach Z's flag and Auto Teach Z is
//     not wired on the web (HW.teach.html TEACH_UNWIRED btnAutoTeachZ) -- not done.
//   [W906] P1 = C++'s position of the axis (CurrentUserPos), the value golden's edtNowPosition shows (ScanNowMotorStatus :1342
//     MOT[ActiveMotorIndex].ReadPos()) -- the rule of the other teach moves (W5B-10, DoTeachMoveRel).
//   [W906] a motor outside golden's list: golden does nothing; here a refusal that says so (a golden `return` is answered).
//   The move = the W5-b teach move: a 1203 axis Move1203 re-sending this axis' current golden pct (TeachCurPct, as
//   DoTeachMoveRel), any other card the golden object's MotorMove (MoveGolden, no speed).
//   AI(W906-ST02-C9-G2) 20261002: btn = any of the 36 (TeachSt02RotateButton: positive = the Pos90 handler, group = the golden
//   parent); the visibility is that parent's, everything after it is golden's one handler body (Sender unused).
MotorAccessOutcome St02Rotate90(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, ITeachSt02Ops& ops,
                                const std::string& btn, bool positive, const std::string& group)
{
    const std::string handler = positive ? "SpeedButtonInRotatePos90Click" : "SpeedButtonInRotateNeg90Click";
    const std::string who = "teachSt02 " + btn + "（golden " + handler + "）";
    const TeachSt02Config cfg = ops.Config();
    const std::string here = "，這台 USE_ROTATE_KIT=" + std::to_string(cfg.useRotateKit) + "、RotateKit_Type=" + std::to_string(cfg.rotateType) +
                             "。golden 按不到＝什麼都不會發生，這裡同樣不做";
    if (group == "grpRotate_Kit") {
        if (!TeachSt02RotateKitShown(cfg))                                      // golden FormShow :1652
            return Refuse(who + ": golden 看不到這顆按鈕 —— grpRotate_Kit->Visible=(USE_ROTATE_KIT==1 && iRotate_Type 是 e1MotRotate／e1MotRotate1Dut／eInOutArm1Motor)"
                          "（uteach.cpp:1652）" + here);
    } else {
        if (!TeachSt02RotateTabShown(cfg))                                      // golden FormShow :1668
            return Refuse(who + ": golden 看不到這顆按鈕 —— 它在 tsRotate 頁，tsRotate->TabVisible=(USE_ROTATE_KIT==1 && iRotate_Type 是 e8MotRotate／e4MotRotate／"
                          "e2MotRotate2Dut)（uteach.cpp:1668）" + here);
        if (!TeachSt02RotateGroupShown(cfg, group)) {                           // golden FormShow :1690-1703
            const St02RotGroup* g = St02RotGroupOf(group);
            return Refuse(who + ": golden 看不到這顆按鈕 —— " + group + "->Visible=" + St02RotGroupRule(g) + "（uteach.cpp:" +
                          std::to_string(g ? g->line : 0) + "）" + here);
        }
    }
    MotionCtx m;
    if (!TeachPrelude(r, wireId, be, m, true)) return Refuse(m.why);           // :4610 / :4662 CheckCanMove / ActiveMotorIndex==-1 / IsCanQuickJogMove
    const int mi = m.a.motIndex;
    const std::string who2 = who + " " + r.motors[0] + "（" + m.a.cardModel + " MOT[" + std::to_string(mi) + "]）";
    if (!m.a.tableEnable) return Refuse(who2 + ": " + kSt02Enable0Why);       // [W906] (a 1203 row was refused by TeachPrelude already)
    int p1 = 0;
    if (!CurrentUserPos(be, m, p1)) return Refuse(who2 + ": 讀不到目前位置（golden P1=atoi(edtNowPosition->Text)）");   // :4612 / :4664
    const int p2 = TeachSt02RotateP2(cfg.rotateType);                          // :4613-4620 / :4665-4672
    if (p1 > 999999 || p1 < -999999) return Refuse(who2 + ": Position over limitation! 移動位置超過限制!!");   // :4622-4626 Steven 20100831
    const int kind = ops.RotateMotorKind(mi);                                   // :4628-4635 / :4680-4687
    if (kind == kSt02RotNone)
        return Refuse(who2 + ": 不是旋轉軸 —— golden 只對 MInRotateKit..H／MOutRotateKit..H 動作（:4628-4635），其他軸什麼都不做");
    ops.GoldenClearFCmd(mi);                                                    // :4637 / :4689 MOT[ActiveMotorIndex].fCMD=false
    const int goal = positive ? p1 + p2 : p1 - p2;
    int bl = 0;
    const bool kit = (kind == kSt02RotInKit || kind == kSt02RotOutKit);
    if (kit) {                                                                  // :4639-4652 RogerYang 20260113 : Rotator新增背隙補償
        const bool inRot = (kind == kSt02RotInKit);
        double set = 0.0;
        if (!ParamNum(r, inRot ? "backlashIn" : "backlashOut", set))
            return Refuse(who2 + ": 缺背隙設定（golden atoi(edtEditRotate" + std::string(inRot ? "In" : "Out") + "Backlash->Text)；頁面送 0＝空白）");
        if (!IsIntValue(set)) return Refuse(who2 + ": 背隙設定不是有限的整數");
        if (!TeachRotatorBacklash(be, m, goal, inRot, (int)set, bl))            // golden GetRotatorBacklash(P1±P2, bIsInRot, iBacklashSet)
            return Refuse(who2 + ": 讀不到旋轉站目前位置（背隙補償要用）");
    }
    const long long t = (long long)goal + (long long)bl;
    if (t > 2147483647LL || t < -2147483647LL - 1) return Refuse(who2 + ": 目標＋背隙超出 int 範圍");
    const int target = (int)t;
    MotorAccessOutcome o = m.a.Is1203() ? Move1203(r, wireId, be, m, target, true, TeachCurPct(mi))   // :4653 / :4705 MotorMove(P1±P2+iBacklash)
                                        : MoveGolden(r, be, m, target, false, 0);
    if (!o.ok) return o;
    webbridge::JsonWriter x;
    x.BeginObject();
    x.Key("st02Btn").String(btn);
    x.Key("st02Group").String(group);
    x.Key("p1").Number((wb_int64)p1);
    x.Key("p2").Number((wb_int64)p2);
    x.Key("rotateType").Number((wb_int64)cfg.rotateType);
    x.Key("rotateKind").Number((wb_int64)kind);
    x.Key("goal").Number((wb_int64)goal);
    if (kit) x.Key("backlash").Number((wb_int64)bl);
    x.EndObject();
    o.ackJson = St02Merge(o.ackJson, x.Ok() ? x.Str() : std::string());
    return o;
}

// ---- btnSht1GoLatchClick (uteach.cpp:4721-4735) / btnSht2GoLatchClick (:4737-4751) ----
//   golden: `if(MOT[MInShuttle1].HomeFlag==0){ ShowMyMessage("motor need home","馬達需要歸零","Teach"); return; }`
//   iShuttleSelect=1; MOT[MInShuttle1].InitMOTParameter(); SetSpeed(atoi(EditSh1Speed->Text)); MotorMove(Tech.iInShuttle1Right);
//   (Sht2: MInShuttle2, EditSh2Speed, Tech.iInShuttle2Right, iShuttleSelect=2.)
//   golden has NO CheckCanMove / IsCanQuickJogMove here; golden MotorMove's own interlocks are Move1203's (safe door, lock,
//   Mot_Table Enable, soft limits, READY, the shuttle floodgate mymotor.cpp:652-698) or the golden object's.
//   InitMOTParameter = fCMD / GaliSofDelayCount / bScanFlag / MovFlag = false (Motor/mymotor.cpp:374-380): memory only, on a
//   1203 axis too -- Move1203 drives the card and does not read fCMD. iShuttleSelect is kept for btnGetLatchClick (:4753-4777,
//   not wired: it reads fLtcSensor's latch tables) and reported in the ack.
//   golden never shows these two buttons (their group grpShtSensor is Visible=False in uteach.dfm:5850 and no golden line sets
//   it true) -> refused below on every machine; the body is here and tested so that un-hiding it is a one-line decision.
//   [W906] shtSpeed < 0 refused: golden's keypad for EditSh1Speed / EditSh2Speed is 1..max (EditSh1SpeedClick :4718), and
//     MotorSpeedFromPct's unsigned arithmetic would not give golden's double result for a negative value.
MotorAccessOutcome St02GoLatch(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, ITeachSt02Ops& ops, int which)
{
    const std::string btn = (which == 1) ? "btnSht1GoLatch" : "btnSht2GoLatch";
    const std::string n = std::to_string(which);
    const std::string who = "teachSt02 " + btn + "（golden " + btn + "Click）";
    const TeachSt02Config cfg = ops.Config();
    if (!cfg.shtSensorGroupShown)
        return Refuse(who + ": golden 看不到這顆按鈕 —— 它的群組 grpShtSensor 在 uteach.dfm:5850 是 Visible = False，golden 沒有任何一行把它設回 true"
                      "（任何機台都按不到）。golden 按不到＝什麼都不會發生，這裡同樣不做");
    const int mi = ops.GoldenMotorIndex(which == 1 ? kSt02MotInShuttle1 : kSt02MotInShuttle2);
    std::string alias;
    if (mi < 0 || !be.AliasOfMotIndex(mi, alias))
        return Refuse(who + ": MInShuttle" + n + "（MOT[" + std::to_string(mi) + "]）不在這台的 Mot_Table —— golden 給它一個 Enable=false 的替身物件（cinitial.cpp:4027-4034 TMySYNTEKMotor(-1)），SetSpeed 只寫記憶體、MotorMove 只寫 Position（不碰硬體）；這裡不呼叫");
    MotorGolden g;
    if (!be.GoldenMotor(mi, g)) return Refuse(who + " " + alias + ": 沒有 golden 馬達物件（MOT[" + std::to_string(mi) + "].Motor 為 NULL）");
    if (g.homeFlag == 0) return Refuse(who + " " + alias + ": motor need home 馬達需要歸零（golden :" + (which == 1 ? "4723-4727" : "4739-4743") + "）");
    double sp = 0.0;
    if (!ParamNum(r, "shtSpeed", sp)) return Refuse(who + ": 缺 shtSpeed（golden SetSpeed(atoi(EditSh" + n + "Speed->Text))；頁面送 atoi 的結果，空白＝0）");
    if (!IsIntValue(sp)) return Refuse(who + ": EditSh" + n + "Speed 不是有限的整數");
    if (sp < 0) return Refuse(who + ": EditSh" + n + "Speed=" + std::to_string((long long)sp) + " 是負數 —— golden 的小鍵盤是 1..上限（EditSh1SpeedClick :4718），這裡不送負的速度");
    g_st02ShuttleSelect = which;                                                // :4730 / :4746
    ops.GoldenInitMOTParameter(mi);                                             // :4731 / :4747
    MotionCtx m;
    if (!TeachCtxOfIndex(r, wireId, be, mi, true, false, m)) return Refuse(m.why);   // resolve, golden object, 1203 layer (no CheckCanMove: golden has none here)
    if (!m.a.tableEnable) return Refuse(who + " " + alias + ": " + kSt02Enable0Why);
    const int target = ops.TechInShuttleRight(which);                           // :4733 / :4749 Tech.iInShuttle1Right / 2Right
    MotorAccessReq r2 = r;
    r2.motors.assign(1, alias);
    MotorAccessOutcome o = m.a.Is1203() ? Move1203(r2, wireId, be, m, target, true, (int)sp)    // :4732 SetSpeed + :4733 MotorMove
                                        : MoveGolden(r2, be, m, target, true, (int)sp);
    if (!o.ok) return Refuse(o.ackJson + "（golden iShuttleSelect=" + n + " 與 InitMOTParameter 已經做了）");
    webbridge::JsonWriter x;
    x.BeginObject();
    x.Key("st02Btn").String(btn);
    x.Key("shuttleSelect").Number((wb_int64)g_st02ShuttleSelect);
    x.Key("shtSpeed").Number((wb_int64)(long long)sp);
    x.Key("techInShuttleRight").Number((wb_int64)target);
    x.EndObject();
    o.ackJson = St02Merge(o.ackJson, x.Ok() ? x.Str() : std::string());
    return o;
}

// ---- btnSetAllInArmZ_MoveClick (uteach.cpp:5948-5971) / btnSetAllOutArmZ_MoveClick (:5973-5996) ----
//   golden: static bool b=false; for(i<iMotRow) for(j<iMotCol) { iMot=Suck[i][j].iMotNo; if(i<iPickRow && j<iPickCol)
//   MOT[iMot].MotorMove(b==false ? Prod.ZInArm_Tray_Pick[i][j] (Out: Prod.ZOutArm_Auto_Place[0][i][j]) : 0); }  b=!b;
//   No CheckCanMove / IsCanQuickJogMove, no HomeFlag, no speed (golden has none); every axis is tried, the return is ignored,
//   and b flips after the loop whatever happened. Here every axis is reported (axes[]: moved / refused + why); any refusal
//   makes the ack an error (state "error", result "partial"; none moved = a refusal with every reason).
//   [W906] fail-closed (HUMAN_REVIEW):
//     * a Mot_Table Enable=0 axis is not called: golden MotorMove on Enable=false only writes MOT[i].Position=Tar and returns 1
//       (golden Motor/mymotor.cpp:821-823, no hardware) -- here "not on this machine", nothing written;
//     * a motor that is not an arm Z (golden ProcessSingleMotorHome's case-300 list, MotorGolden.armZ) is not moved -- golden
//       drives whatever Suck[i][j].iMotNo holds (cinitial.cpp:938-955 sets the arm Z's; an unset nozzle holds 0 = MInArmX);
//     * a nozzle outside Prod's [MAX_ARM_Row][MAX_ARM_Col] is not moved (golden would read past the array);
//     * a motor that is not in Mot_Table is not called: golden gives it an Enable=false TMySYNTEKMotor(-1) stand-in
//       (golden cinitial.cpp:4027-4034) whose MotorMove only writes Position -- same as Enable=0.
//   The move = the W5-b teach move per axis: 1203 Move1203 re-sending the axis' current golden pct (TeachCurPct), other
//   cards the golden object's MotorMove (MoveGolden, no speed).
MotorAccessOutcome St02SetAllZ(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, ITeachSt02Ops& ops, bool inArm)
{
    const std::string btn = inArm ? "btnSetAllInArmZ_Move" : "btnSetAllOutArmZ_Move";
    const std::string who = "teachSt02 " + btn + "（golden " + btn + "Click）";
    const TeachSt02Config cfg = ops.Config();
    if (!TeachSt02ArmZPanelShown(cfg))                                          // golden FormShow :1457 / :1458
        return Refuse(who + ": golden 看不到這顆按鈕 —— " + (inArm ? "pnlInArmZ" : "pnlOutArmZ") + "->Visible=(USE_PICKER_COUNT==ep16Picker)（uteach.cpp:" +
                      (inArm ? "1457" : "1458") + "），這台 USE_PICKER_COUNT=" + std::to_string(cfg.usePickerCount) + "。golden 按不到＝什麼都不會發生，這裡同樣不做");
    const TeachSt02ArmZGrid grid = ops.ArmZGrid(inArm);
    bool& gb = g_st02SetAllZB[inArm ? 0 : 1];
    const bool before = gb;                                                     // false -> the pick / place height, true -> 0
    struct AxisRes { int row, col, mi; std::string motor; int target; bool ok; std::string result, message, why; };
    std::vector<AxisRes> res;
    int moved = 0, refused = 0;
    for (std::size_t k = 0; k < grid.cells.size(); ++k) {
        const TeachSt02ArmZCell& c = grid.cells[k];
        if (!c.inPick) continue;                                                // :5957 / :5982
        AxisRes x;
        x.row = c.row; x.col = c.col; x.mi = c.motIndex; x.target = before ? 0 : c.target; x.ok = false;
        if (!before && !c.targetKnown) {
            x.why = "[W906] 吸嘴 [" + std::to_string(c.row) + "][" + std::to_string(c.col) + "] 在 Prod 的 Z 高度表外（" +
                    (inArm ? "ZInArm_Tray_Pick" : "ZOutArm_Auto_Place[0]") + " 只有 [MAX_ARM_Row][MAX_ARM_Col]），golden 會讀到陣列外 —— 不動";
        } else if (c.motIndex < 0 || !be.AliasOfMotIndex(c.motIndex, x.motor)) {
            x.why = "MOT[" + std::to_string(c.motIndex) + "] 不在這台的 Mot_Table —— golden 給它一個 Enable=false 的替身物件（cinitial.cpp:4027-4034），MotorMove 只寫 Position、回 1（不碰硬體）；這裡不呼叫";
        } else {
            MotorAccessAxis a;
            MotionCtx m;
            if (!be.Resolve(x.motor, a)) x.why = "馬達表上沒有 " + x.motor;
            else if (!a.tableEnable) x.why = kSt02Enable0Why;
            else if (!MotionPreludeFor(r.action, x.motor, wireId, be, m, true)) x.why = m.why;
            else if (!m.g.armZ)
                x.why = "[W906] MOT[" + std::to_string(c.motIndex) + "] 不是手臂 Z 軸（golden ProcessSingleMotorHome case 300 的清單）—— 吸嘴表的馬達號碼不對，不動（fail-closed）";
            else {
                MotorAccessReq r2 = r;
                r2.motors.assign(1, x.motor);
                const MotorAccessOutcome o = m.a.Is1203() ? Move1203(r2, wireId, be, m, x.target, true, TeachCurPct(m.a.motIndex))   // :5960 / :5962 MotorMove
                                                          : MoveGolden(r2, be, m, x.target, false, 0);
                if (o.ok) { x.ok = true; St02AckResult(o.ackJson, x.result, x.message); }
                else x.why = o.ackJson;
            }
        }
        if (x.ok) ++moved; else ++refused;
        res.push_back(x);
    }
    gb = !gb;                                                                   // :5967-5970 / :5992-5995 (after the loop, whatever happened)
    std::string sum;
    for (std::size_t k = 0; k < res.size(); ++k)
        sum += "; " + (res[k].motor.empty() ? "MOT[" + std::to_string(res[k].mi) + "]" : res[k].motor) + " -> " + std::to_string(res[k].target) +
               (res[k].ok ? ": " + res[k].result : " REFUSED: " + res[k].why);
    const std::string head = "golden " + btn + "Click: MotorMove(" + (before ? "0" : (inArm ? "Prod.ZInArm_Tray_Pick" : "Prod.ZOutArm_Auto_Place[0]")) +
                             ") -- moved " + std::to_string(moved) + ", refused " + std::to_string(refused) + "; golden b " +
                             (before ? "true" : "false") + " -> " + (gb ? "true" : "false");
    if (moved == 0) {
        const std::string none = res.empty() ? "; 沒有任何吸嘴在 iPickRow×iPickCol 裡（golden 這一下什麼都不動）" : sum;
        return Refuse(who + ": " + head + none + (grid.note.empty() ? "" : "; " + grid.note));
    }
    webbridge::JsonWriter w;
    w.BeginObject();
    St02Head(w, r, refused ? "error" : "done", refused ? "partial" : "moving", "MOT+pci1203", head + sum);
    w.Key("st02Btn").String(btn);
    w.Key("arm").String(inArm ? "in" : "out");
    w.Key("goldenB").Bool(before);
    w.Key("goldenBAfter").Bool(gb);
    w.Key("direction").String(before ? "zero" : "pick");
    w.Key("motRow").Number((wb_int64)grid.motRow);
    w.Key("motCol").Number((wb_int64)grid.motCol);
    w.Key("pickRow").Number((wb_int64)grid.pickRow);
    w.Key("pickCol").Number((wb_int64)grid.pickCol);
    if (!grid.note.empty()) w.Key("gridNote").String(grid.note);
    w.Key("moved").Number((wb_int64)moved);
    w.Key("refused").Number((wb_int64)refused);
    w.Key("partial").Bool(refused > 0);
    w.Key("axes").BeginArray();
    for (std::size_t k = 0; k < res.size(); ++k) {
        const AxisRes& x = res[k];
        w.BeginObject();
        w.Key("row").Number((wb_int64)x.row);
        w.Key("col").Number((wb_int64)x.col);
        w.Key("motIndex").Number((wb_int64)x.mi);
        w.Key("motor").String(x.motor);
        w.Key("target").Number((wb_int64)x.target);
        w.Key("ok").Bool(x.ok);
        if (x.ok) { w.Key("result").String(x.result); w.Key("message").String(x.message); }
        else w.Key("why").String(x.why);
        w.EndObject();
    }
    w.EndArray();
    return St02Ok(Finish(w));
}

}  // namespace

void           TeachSt02InstallOps(ITeachSt02Ops* ops) { g_st02Ops = ops; }
ITeachSt02Ops* TeachSt02InstalledOps() { return g_st02Ops; }

int TeachSt02RotateP2(int t)
{
    if (t == kSt02Rot4Mot || t == kSt02Rot8Mot || t == kSt02Rot2Mot2Dut) return 800;   // :4613-4616 Steven 20191112
    if (t == kSt02RotInOutArm1Motor) return 1250;                                         // :4617-4618 Ifor 20251204
    return 2000;                                                                          // :4619-4620 JerryYang 20160302
}
bool TeachSt02RotateKitShown(const TeachSt02Config& c)
{
    return c.useRotateKit == 1 &&
           (c.rotateType == kSt02Rot1Mot || c.rotateType == kSt02Rot1Mot1Dut || c.rotateType == kSt02RotInOutArm1Motor);
}
bool TeachSt02ArmZPanelShown(const TeachSt02Config& c) { return c.usePickerCount == kSt02Picker16; }
//AI(W906-ST02-C9-G2) 20261002: the 36 rotate buttons (St02RotGroups above)
bool TeachSt02RotateTabShown(const TeachSt02Config& c)
{
    return c.useRotateKit == 1 &&                                                        // :1668 Steven 20170329 (Wei)
           (c.rotateType == kSt02Rot8Mot || c.rotateType == kSt02Rot4Mot || c.rotateType == kSt02Rot2Mot2Dut);
}
bool TeachSt02RotateGroupShown(const TeachSt02Config& c, const std::string& group)
{
    if (group == "grpRotate_Kit") return TeachSt02RotateKitShown(c);
    const St02RotGroup* g = St02RotGroupOf(group);
    if (g == 0) return false;
    switch (g->rule) {
    case kSt02GrbNot2Dut: return c.useRotateKit == 1 && c.rotateType != kSt02Rot2Mot2Dut;
    case kSt02Grb8Only:   return c.useRotateKit == 1 && c.rotateType == kSt02Rot8Mot;
    case kSt02Grb8Or2Dut: return c.useRotateKit == 1 && (c.rotateType == kSt02Rot8Mot || c.rotateType == kSt02Rot2Mot2Dut);
    default:              return true;                                                   // grbInRA / grbInRE: the dfm's Visible=True
    }
}
bool TeachSt02RotateButton(const std::string& btn, bool& positive, std::string& group)
{
    positive = false;
    group.clear();
    if (btn == "SpeedButtonInRotatePos90" || btn == "SpeedButtonOutRotatepPos90") { positive = true; group = "grpRotate_Kit"; return true; }
    if (btn == "SpeedButtonInRotateNeg90" || btn == "SpeedButtonOutRotatepNeg90") { group = "grpRotate_Kit"; return true; }
    for (std::size_t i = 0; i < kSt02RotGroupCount; ++i) {
        const std::string sfx = std::string(kSt02RotGroups[i].group).substr(3);         // "InRA" .. "OutRH"
        if (btn == "btnPos90" + sfx) { positive = true; group = kSt02RotGroups[i].group; return true; }
        if (btn == "btnNeg90" + sfx) { group = kSt02RotGroups[i].group; return true; }
    }
    return false;
}
bool TeachSt02RotateButtonShown(const TeachSt02Config& c, const std::string& btn)
{
    bool positive = false;
    std::string group;
    if (!TeachSt02RotateButton(btn, positive, group)) return false;
    if (group == "grpRotate_Kit") return TeachSt02RotateKitShown(c);
    return TeachSt02RotateTabShown(c) && TeachSt02RotateGroupShown(c, group);
}
MotorAccessOutcome TeachSt02Dispatch(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    const std::string btn = St02BtnOf(r);
    if (r.source != "uteach")
        return Refuse("teachSt02 " + btn + "（" + r.source + "）: 這是 golden uteach 教導頁的按鈕，只收教導頁的請求");
    if (g_st02Ops == 0)
        return Refuse("teachSt02 " + btn + ": St02 的 golden 介面沒有安裝（WebTeachSt02Live.cpp 只連進 wb_serve）—— 什麼都不做");
    ITeachSt02Ops& ops = *g_st02Ops;
    if (btn == "St02State")                return St02StateAck(r, ops);
    bool rotPositive = false;                                                  //AI(W906-ST02-C9-G2) 20261002: the 36 rotate buttons
    std::string rotGroup;
    if (TeachSt02RotateButton(btn, rotPositive, rotGroup)) return St02Rotate90(r, wireId, be, ops, btn, rotPositive, rotGroup);
    if (btn == "btnSht1GoLatch")           return St02GoLatch(r, wireId, be, ops, 1);
    if (btn == "btnSht2GoLatch")           return St02GoLatch(r, wireId, be, ops, 2);
    if (btn == "btnSetAllInArmZ_Move")     return St02SetAllZ(r, wireId, be, ops, true);
    if (btn == "btnSetAllOutArmZ_Move")    return St02SetAllZ(r, wireId, be, ops, false);
    return Refuse("teachSt02: 不認得的按鈕 '" + btn + "'（G2＝旋轉 ±90 的 36 顆（uteach.dfm OnClick＝SpeedButtonInRotatePos90Click／Neg90Click）、"
                  "btnSht1／2GoLatch、btnSetAllIn／OutArmZ_Move。BtnPanelLane1～3／btnZ1、Z2、Arm1Y、Arm2YServo／TTL 不在這裡：機台在 main 上接好了"
                  "（io.btnPanelClick／teachIndexServo；TTL 勾選框照 FormShow 灰掉））");
}

TeachSt02State TeachSt02States()
{
    TeachSt02State s;
    s.setAllInArmZB = g_st02SetAllZB[0];
    s.setAllOutArmZB = g_st02SetAllZB[1];
    s.shuttleSelect = g_st02ShuttleSelect;
    return s;
}
void TeachSt02ResetForTests()
{
    g_st02SetAllZB[0] = g_st02SetAllZB[1] = false;
    g_st02ShuttleSelect = 0;
}

}  // namespace ht9045

// =============================================================================
//  AI(W906-GEARRATIO) 20261002 [W906]: the Motor Test "Gear Ratio" tab (RULINGS_20261002 #22; NB2 spec
//  RD5軟體_NB2規格_MotorTest頁GearRatio校正分頁_20261002_213826.md §5, on origin/v906/nb2-assist). golden has no such function.
//  Part 1 -- gearRatioSave's three Mot_Table cells (spec §5.4). The writer is MotTableEditRow / MotTableSaveRow above; the
//  only change there is that the boot's MN200 "/100" applies to Acc / Dec alone (it is InitialMotorParameter's `dAcc>1`,
//  cinitial.cpp:4049-4054; GearRatio is a plain atof, cinitial.cpp:4043) and an empty GearRatio cell reads 1.0 (MtDefault;
//  TMOTDATA database.cpp ~:2768). saveMotTable keeps writing its ten cells, never GearRatio.
// =============================================================================
namespace ht9045 {
std::vector<MotTableSaveField> MotTableGearFieldsOf(double gearRatio, int softP, int softN)
{
    std::vector<MotTableSaveField> v;
    MotTableSaveField g; g.column = "GearRatio";  g.kind = kMtfReal; g.value = gearRatio;       v.push_back(g);
    MotTableSaveField p; p.column = "SoftLimitP"; p.kind = kMtfInt;  p.value = (double)softP;   v.push_back(p);
    MotTableSaveField n; n.column = "SoftLimitN"; n.kind = kMtfInt;  n.value = (double)softN;   v.push_back(n);
    return v;
}
}  // namespace ht9045

// =============================================================================
//  AI(W906-GEARRATIO) 20261002 [W906]: Part 2 -- the three actions (NB2 spec §5.1-§5.3) and the runtime "gearCal" block.
//  The interface and the parameters: GearRatioBackend.h. Every hook above this block is a same-line addition (no line moves).
//
//  The measurement session (one at a time, Motor Test only). gearCalMove with begin=true opens it: that move is the backlash
//  take-up (|d| <= 5 mm, in the measurement direction) and its target is the gauge zero (c = 0); every later gearCalMove of the
//  same motor goes from the session's last target by distanceMm. A point = the cumulative commanded distance from the zero:
//  cNom = (target - zero) / 100 mm as the page commanded it, cTrue = (card target - card zero) x GearRatio / 100 mm = the
//  distance the pulses really commanded (equal for GearRatio 1; the fit uses cTrue, the page's c only finds the point).
//  "Intact" = the same motor and GearRatio, no other command of this tree on the axis since the session's last move (a
//  per-axis counter bumped by every NoteIssued and the Index Z1 route's MotorAccessNoteIssuedAt), and the axis READY on a
//  fresh sample at exactly that last card target. The session ends on STOP, an alarm, the operator gone / the token taken,
//  the safe lock, a motor selection, Motor Test FormShow / FormClose, a servo toggle on its axis, a save.   AI(W906-GEARRATIO2) 20261003: every end but a save stops the session's own axis (NB2 R171 M1, CancelGear); a new begin no longer replaces it -- refused while it is active (R171 L1 (b))
//  gearCalMove's gates, in order (refused = nothing moved; the ones golden Motor Test does not have are this feature's):
//    the dispatcher's (SystemStart, hand teach, Arm Cell's axes), JobsBusyWhy (HOME / LoopMove / Arm Cell on the axis),
//    MotionPrelude (Mot_Table, golden object, selectable, 1203 layer, the monitor opened the axis), PCI1203 only (v1), not a
//    rotary / scale axis, a Z axis only with REAL soft limits (+-999999 = placeholders, W-13), EMG (golden IsMotorCanRun's
//    sensors, no box), the axis' ALM / ERROR_STOP, servo ON, HomeFlag == 1, the parameters (distanceMm a multiple of
//    0.01 mm, speedPct 1..20), the session (begin: no session active (R171 L1 b), take-up <= 5 mm; else intact), the caps while the soft limits are
//    placeholders (<= 50 mm per move, <= 100 mm from the session's START point, R171 L1 a -- with real limits the soft limits bound it), an arm X / Y:
//    every Z of that arm at home (MotorAccessTeachInterlockHome = the tree's per-axis SensorType ORG rule; unknown =
//    refused), then golden's teach interlock GoldenTeachCanMove (CheckCanMove + IsCanQuickJogMove) for EVERY axis (an arm
//    X / Y: its Zs again the golden way; a shuttle: the Index Zs, the arm Zs and the floodgate; Tray X: its own), then
//    MoveBlock1203 (door, lock, Enable, the soft limits with golden's >= / <=, READY, floodgate) and the fresh-sample rule.
//    ⚠ The +-999999 placeholders limit NOTHING (MoveBlock1203's test never fires) -- the 50 / 100 mm caps are the only bound
//    then. The operator token is the transport's (WebBridgeServer: motor.access is not exempt), like every Motor Test move.
// =============================================================================
namespace ht9045 {
namespace {

struct GearCalPoint { double cNom = 0.0, cTrue = 0.0; int dir = 0; int targetUser = 0; double targetCard = 0.0; };
struct GearCalSession {
    bool          active = false;
    unsigned long id = 0;
    std::string   motor;
    int           mi = -1, axis = -1;
    double        ratio = 0.0;
    int           dirSign = 0;
    int           startUser = 0, zeroUser = 0, lastTargetUser = 0, arriveCmdPos = 0;
    double        zeroCard = 0.0, lastTargetCard = 0.0;
    unsigned long issuedSeq = 0;
    bool          placeholder = true;
    std::vector<GearCalPoint> points;
    std::string   startedAt, why;
    //AI(W906-GEARHAND) 20261003: the hand-push session (RULINGS_20261003 #8, INBOX 154; block "HAND-PUSH" below) -- no move ever.
    bool          hand = false;
    int           handPhase = 0;              // 0 = E0 taken (servo on), 1 = servo OFF (being pushed), 2 = servo ON again (waiting for a settled sample), 3 = E1 taken
    double        e0Card = 0.0, e1Card = 0.0;  // the encoder (monitor actPos, card pulses) at the start / after the push
    unsigned long onPoll = 0;                  // the monitor's poll count when the servo came back ON (E1 needs a later sample)
    bool          candKnown = false;           // phase 2: the first READY sample after the servo on, waiting for a second equal one
    double        cand = 0.0;
    unsigned long candPoll = 0;
    int           pushes = 0;                  // servo OFF seen (each one voids E1)
    std::string   e1At;
};
GearCalSession g_gear;
unsigned long  g_gearIds = 0;
const double   kGearTakeupMaxMm = 5.0, kGearStepMaxMm = 50.0, kGearSpanMaxMm = 100.0;
const int      kGearSpeedMaxPct = 20;
const double   kGearMatchMm = 0.005;               // a page point finds its session point within half a hundredth
const double   kGearHandRulerMaxMm = 2000.0;       //AI(W906-GEARHAND) 20261003: a ruler distance beyond this is a typing error (the page's READ_MAX_MM)
const double   kGearHandSettleUser = 2.0;          //AI(W906-GEARHAND) 20261003: E1 = two fresh samples within 0.02 mm (2 user units) of each other
int            g_gearKeepHand = 0;                 //AI(W906-GEARHAND) 20261003: > 0 while a Motor Test servo toggle runs CancelJobsOnAxis (CancelJobsOnAxisKeepHand)

bool GearActive() { return g_gear.active; }
void GearEnd(const std::string& why)
{
    if (!g_gear.active) return;
    g_gear.active = false;
    g_gear.why = why;
    g_lastJobNote = "gear ratio " + g_gear.motor + ": " + why;
}
//AI(W906-GEARRATIO2) 20261003: NB2 R171 M1 -- every end of a session but a save (STOP, an alarm, the operator gone / the token
//  taken, the safe lock, Motor Test FormShow / FormClose, a motor selection, Motor Power, a servo toggle on its axis, the engine's
//  AllBtnUp) now stops the session's OWN axis with the stop the Teach Arm Cell job gives its legs (ArmCellEnd(..., stop=true):
//  Stop1203 = golden DecStop, StopDec + ExtDrive 0). It was `(void)be;`: the leg under way ran on (MInArmX at 20 %, a 50 mm leg
//  cut by the safe lock at 5 mm, went the other 45 mm; with real soft limits a leg has no length cap). Only that one axis, and
//  only while the session's last move is still the axis' latest command of this tree (g_issuedSeq == the session's): after a
//  JOG / move / HOME on it the motion is that command's, which keeps its own rule (golden: a HOME / Loop leg is not stopped,
//  MotorAccessTick). Never while SystemStart: the machine (MainProc) owns the axes then. A refused stop is written into why.
//AI(W906-GEARHAND) 20261003: the end of a HAND session (spec (10)): it never commanded anything, so nothing is stopped and the servo
//  is left as the operator set it -- but an end with the servo OFF is said (the page shows why), so nobody walks away from a free axis
std::string GearHandServoNote(IMotorAccessBackend& be)
{
    if (g_gear.axis < 0) return std::string();
    bool known = false;
    const bool on = be.Pci1203ServoOn(g_gear.axis, known);
    if (!known) return "；這一軸的伺服狀態讀不到 —— 請到機台旁確認";
    return on ? std::string() : "；⚠ 這一軸伺服還是 OFF（照操作員的設定，沒有動它）—— 要用這一軸先按 Servo On";
}
void CancelGear(IMotorAccessBackend& be, const std::string& why)
{
    if (!g_gear.active) return;
    if (g_gear.hand) {                                                          //AI(W906-GEARHAND) 20261003: a hand session has no leg to stop (spec (9)/(10))
        if (g_gearKeepHand > 0) return;                                         //   the Motor Test servo toggle on its axis IS the procedure (CancelJobsOnAxisKeepHand)
        { const bool pushed = g_gear.pushes > 0 && g_encBase.count(g_gear.axis) != 0; if (pushed && g_gear.mi >= 0) be.GoldenSetHomeFlag(g_gear.mi, 0); GearEnd("量測中止：" + why + GearHandServoNote(be) + (pushed ? std::string("；這一軸手推過、沒有存：HomeFlag 已設成 0（W5B-6：命令位置可能還停在推之前）—— 移動這一軸之前，先重開 wb_serve，再回原點（HOME）") : (g_gear.pushes > 0 ? std::string("；這一軸手推過、沒有存；Servo ON 時命令位置已照 golden 寫成編碼器位置，HomeFlag 不動") : std::string()))); }   //AI(W906-SVON-ENCSYNC) 20261004: pushed = pushed AND not synced since (a Servo ON sync takes the axis off g_encBase; Steven 1003 22:5x: a homed axis need not re-home, the encoder goes to the command position)  //AI(W906-GEARHAND) 20261003: NB2-1 (g) M1 (St01 ST01-E2 review, FROM_STEVEN s3 18:30) -- an end WITHOUT a save after a push (STOP, motor select, ALM, FormClose, SystemStart, another command, the operator gone ...) left HomeFlag 1 while the card's command position may still be where the push began (g_encBase re-bases only CurrentUserPos): moveAbsolute / teachGo then started from it and ended off by the push. Now HomeFlag 0 as the save does (:7950), and the end says so; no push -> HomeFlag untouched. Same line
        return;
    }
    std::string stop;
    std::map<int, unsigned long>::const_iterator it = g_issuedSeq.find(g_gear.axis);
    const unsigned long seq = (it == g_issuedSeq.end()) ? 0ul : it->second;
    if (g_gear.axis >= 0 && seq == g_gear.issuedSeq && !be.GoldenSystemStart()) {
        const Pci1203CmdResult res = Stop1203(be, g_gear.axis, -1);            // StopDec + ExtDrive 0 (golden DecStop), as ArmCellEnd
        stop = CmdFailed(res) ? "；⚠ 停軸被拒（" + CmdWhy(res) + "）—— 請按 STOP 或急停確認"
                              : std::string(res.issued ? "；已停這一軸" : "；停軸已受理但沒有送到卡（控制層是 dry）");
    }
    GearEnd("量測中止：" + why + stop);
}
bool GearOnAxis(const std::string& id, const MotorAccessAxis& b) { return g_gear.active && SameAxisAs(g_gear.motor, g_gear.mi, g_gear.axis, id, b); }
void GearResetState() { g_gear = GearCalSession(); g_gearIds = 0; g_issuedSeq.clear(); g_gearKeepHand = 0; }   //AI(W906-GEARHAND) 20261003: + g_gearKeepHand
void GearFillJobs(MotorAccessJobState& j) { j.gearActive = g_gear.active; j.gearMotor = g_gear.active ? g_gear.motor : std::string(); }

std::string GearKindOf(const std::string& alias, const MotorGolden& g)
{
    if (g.scaleMotor) return "scale";
    if (g.rotateKit != 0 || g.motorClass == kInitCfgMotorRotate || alias.find("Rotate") != std::string::npos) return "rotate";
    if (g.armZ || g.zStack || g.indexZ || alias.find('Z') != std::string::npos) return "z";
    return "linear";
}
bool GearPlaceholderLimits(const MotorGolden& g) { return GearIsPlaceholder(g.softP) || GearIsPlaceholder(g.softN); }
// the static half (no box, no side effect): "" = this axis can be calibrated by the first version
std::string GearStaticWhy(const std::string& alias, const MotorAccessAxis& a, const MotorGolden& g)
{
    if (!a.Is1203()) return alias + "（" + a.cardModel + "）：第一版只接 PCI1203 軸（9050）";
    if (!a.tableEnable) return alias + "：Mot_Table Enable=0（這台沒裝這一軸）";
    const std::string k = GearKindOf(alias, g);
    if (k == "rotate") return alias + "：旋轉軸（單位是角度不是 mm）—— Gear Ratio 分頁只校直線軸";
    if (k == "scale")  return alias + "：光學尺（只讀編碼器的軸，不下移動命令）";
    if (k == "z" && GearPlaceholderLimits(g))
        return alias + "：Z 軸的軟體極限還是 ±999999 佔位值 —— 先在 Mot_Table 填真實軟體極限（W-13），Z 軸才開放量測";
    return std::string();
}
// A hand-taught 1203 axis (servo off + pushed, g_encBase W5B-6) reads its "where is it now" from the encoder while its command
//   position may still be where it was before the push (EastSun gives no Acm_AxSetCmdPosition): the first MoveAbs would travel
//   (target - command position), not the take-up the page asked for, and nobody has measured on the machine whether the command
//   position catches up. A calibration on top of an unknown offset would be a guess -> refused (fail-closed) until wb_serve restarts.
std::string GearEncBaseWhy(const std::string& alias, const MotorAccessAxis& a)
{
    if (a.Is1203() && g_encBase.count(a.axis))
        return alias + "：這一軸手動教導過（伺服關、手推；W5B-6 編碼器基準）：命令位置跟編碼器的關係還沒在機台量過，"
                       "量出來的距離不可信 —— 不移動（重開 wb_serve、回原點之後再量）";
    return std::string();
}
std::string GearMmText(double v)
{
    char b[48];
    std::snprintf(b, sizeof(b), "%.2f", v);
    return b;
}
//AI(W906-GEARRATIO2) 20261003: NB2 R171 L5 -- the check after a save, worded so it can be done: with +-999999 placeholder soft
//  limits this tab goes at most 100 mm from the start (gearCalMove's cap), so "walk 200 mm again" was impossible there
std::string GearVerifyHint(const MotorGolden& g)
{
    if (GearPlaceholderLimits(g))
        return "再用這個分頁量一次驗證（軟體極限還是 ±999999 佔位值：離開起點最多 " + GearNum(kGearSpanMaxMm) + " mm，例如消背隙 2 mm、往前 50／95 mm；誤差應 ≤ 0.05 mm）";
    return "再走一次 200 mm 驗證（行程不到 200 mm 就走最長的一段；誤差應 ≤ 0.05 mm）";
}
// "" = intact (see the block head); `wait` = the reason is "not there yet" (moving / no fresh sample), not an interruption
std::string GearIntactWhy(IMotorAccessBackend& be, const std::string& alias, const MotorGolden& g, bool* wait = 0)
{
    if (wait) *wait = false;
    if (!g_gear.active) return "沒有進行中的量測" + (g_gear.why.empty() ? std::string() : "（上一次：" + g_gear.why + "）") + " —— 先按「開始（消背隙）」";
    if (g_gear.hand) return "進行中的是手推量測（" + g_gear.motor + "，不下移動命令）—— 先按 STOP 結束，才能用移動量測";   //AI(W906-GEARHAND) 20261003
    if (g_gear.motor != alias) return "進行中的量測是 " + g_gear.motor + " 的，不是 " + alias;
    if (g.gearRatio != g_gear.ratio) return "量測開始之後齒輪比變了（" + GearNum(g_gear.ratio) + " → " + GearNum(g.gearRatio) + "）";
    std::map<int, unsigned long>::const_iterator it = g_issuedSeq.find(g_gear.axis);
    const unsigned long seq = (it == g_issuedSeq.end()) ? 0ul : it->second;
    if (seq != g_gear.issuedSeq) return "量測期間這一軸有別的命令（JOG／移動／HOME／Reload Motor Data／教導頁……）—— 量測被打斷，請按 STOP 結束這次量測後重新開始";   //AI(W906-GEARRATIO2) 20261003: + "按 STOP 結束" (R171 L1 (b): Start is refused while the session is active)
    if (SampleStale(be, g_gear.axis)) { if (wait) *wait = true; return "上一段命令之後監看器還沒有新樣本（稍候再按）"; }
    unsigned st = 0;
    if (!be.Pci1203AxisState(g_gear.axis, st)) return "讀不到這一軸的監看器樣本";
    if (IsErrorState(st)) return "這一軸 ERROR_STOP（驅動器／卡片報錯）—— 量測中止";
    if (!IsReadyState(st)) { if (wait) *wait = true; return "上一段還在走（不在 READY）—— 等到位再按"; }
    double card = 0.0;
    if (!be.Pci1203CmdPos(g_gear.axis, card)) return "讀不到這一軸的命令位置";
    if (card - g_gear.lastTargetCard > 0.5 || g_gear.lastTargetCard - card > 0.5) {
        char b[320];   //AI(W906-GEARRATIO2) 20261003: was 160 -- the longer text below (UTF-8, 3 bytes a character) must not be cut
        std::snprintf(b, sizeof(b), "軸不在這次量測最後送的位置（命令位置 %.0f，應該是 %.0f 脈波）—— 被別的動作打斷，請按 STOP 結束這次量測後重新開始", card, g_gear.lastTargetCard);   //AI(W906-GEARRATIO2) 20261003: R171 L1 (b) -- "按 STOP 結束" (a STOP itself ends the session now, so it is not what stopped it short)
        return b;
    }
    return std::string();
}

// =============================================================================
//  AI(W906-GEARHAND) 20261003 [W906]: HAND-PUSH measurement -- RULINGS_20261003 #8 (NIGHT_REPORT s0 #81 = A, INBOX 154). Jimmy:
//  "we put a ruler under the axis, servo off and push it by hand to a mark, servo on and read the encoder". golden has no such
//  function. The whole mode NEVER commands a move: it reads the encoder; the servo goes through the existing servo button.
//    gearHandBegin (action, Motor Test only): gearCalMove's gates minus its move gates (HomeFlag, the arm's Zs at home, golden
//      CheckCanMove / IsCanQuickJogMove, MoveBlock1203's door / soft limits / floodgate, the 50 / 100 mm caps, the W5B-6
//      encoder-base refusal) -- i.e. the dispatcher's (SystemStart, hand teach, Arm Cell), JobsBusyWhy, MotionPrelude, one session
//      at a time, a PCI1203 LINEAR axis (GearStaticWhy + no Z at all: a Z drops when its servo goes off), EMG, ALM / ERROR_STOP,
//      servo ON, a fresh sample, READY. E0 = the monitor's actPos (card pulses) then.
//    servo OFF / ON = the existing servoToggle (DoServo: brake HOLD + 100 ms before a servo off, kCmdAxSvOn). A Motor Test servo
//      toggle on the session's axis does NOT end a HAND session (CancelJobsOnAxisKeepHand; it still ends a move session).
//      Servo OFF seen (DoServo's issued command, or the monitor's SVON bit in GearHandTick) -> phase 1, E1 void, and the axis goes
//      into g_encBase (W5B-6: its command position may stay where it was before the push -- every axis this mode pushes, not only
//      the PServoAlarmOn ones EndTeachJob marks: the push is certain here, so fail-closed; gearCalMove refuses it until restart).
//    E1 = actPos on a sample taken AFTER the servo came back on (poll count > the one at the servo on), SVON on, READY, no ALM, and
//      a second fresh sample within 0.02 mm of it (a drive pulling the axis back toward its old command position would show it).
//    ratio: the move mode's fit with ONE point (GearFitLimits.minForward = 1; every other rule as it is): c = (E1 - E0) x
//      GearRatio / 100 mm = exactly the move mode's cTrue (the distance the pulses stand for), a = the ruler (mm, signed).
//    preview / save: gearRatioPreview / gearRatioSave with {hand:true, rulerMm} -> GearHandMeasure, then the same GearComputeTail /
//      GearSaveTxn (backups, Mot_Table, memory, teach.ini, verify, restore). HomeFlag is not asked (a move gate); the save still
//      sets it 0, and its message asks for a wb_serve restart before this axis moves (g_encBase lasts until then).
//    ends: what ends a move session (STOP, a motor selection, the operator gone, the safe lock, an alarm, FormShow / FormClose,
//      Motor Power, AllBtnUp, a message box) + SystemStart / another command of this tree on the axis / ALM / ERROR_STOP
//      (GearHandTick). The session stops nothing (it moved nothing); an end with the servo OFF says so (GearHandServoNote).
// =============================================================================
unsigned long GearIssuedSeqOf(int axis)
{
    std::map<int, unsigned long>::const_iterator it = g_issuedSeq.find(axis);
    return it == g_issuedSeq.end() ? 0ul : it->second;
}
std::string GearHandStaticWhy(const std::string& alias, const MotorAccessAxis& a, const MotorGolden& g)
{
    const std::string sw = GearStaticWhy(alias, a, g);
    if (!sw.empty()) return sw;
    if (GearKindOf(alias, g) == "z") return alias + "：Z 軸 —— 手推模式只量直線軸（伺服一關，Z 軸會往下掉）";
    return std::string();
}
double GearHandSettleCard(double ratio) { return (std::isfinite(ratio) && ratio > 0.0) ? kGearHandSettleUser / ratio : kGearHandSettleUser; }
// spec (9): what a hand-session save asks for (the preview's note, the save's message and next)
std::string GearHandRestartText(int axis = -1)   //AI(W906-SVON-ENCSYNC) 20261004: + axis -- synced at Servo ON (not on g_encBase) -> no wb_serve restart
{   if (axis >= 0 && g_encBase.count(axis) == 0) return "存檔之後這一軸 HomeFlag=0。這次是手推量測，Servo ON 時命令位置已照 golden 寫成編碼器位置（不用重開 wb_serve）：""移動這一軸之前，先回原點（HOME）；驗證：回原點之後再用手推模式量一次（誤差應 ≤ 0.05 mm）";
    return "存檔之後這一軸 HomeFlag=0。這次是手推量測（伺服關過；W5B-6：這一軸的命令位置可能還停在推之前）：移動這一軸之前，先重開 wb_serve，"
           "再回原點（HOME）；驗證：重開之後再用手推模式量一次（誤差應 ≤ 0.05 mm）";
}
void GearHandMarkOff()
{
    g_gear.handPhase = 1; g_gear.candKnown = false; g_gear.e1Card = 0.0; g_gear.e1At.clear();
    ++g_gear.pushes;
    g_encBase.insert(g_gear.axis);                                              // W5B-6 (see the block head)
}
// DoServo's hook: a servo command on the session's axis went out
void GearHandServoSeen(IMotorAccessBackend& be, int axis, bool on)
{
    if (!g_gear.active || !g_gear.hand || axis < 0 || axis != g_gear.axis) return;
    if (!on) { if (g_gear.handPhase != 1) GearHandMarkOff(); return; }
    if (g_gear.handPhase == 1) { g_gear.handPhase = 2; g_gear.onPoll = be.Pci1203PollCount(); g_gear.candKnown = false; }
}
// a Motor Test servo toggle = golden btnServoOffClick's AllBtnUp of that axis, minus the end of a hand session it is part of
void CancelJobsOnAxisKeepHand(IMotorAccessBackend& be, const std::string& id, const MotorAccessAxis& a, const std::string& why)
{
    struct Keep { Keep() { ++g_gearKeepHand; } ~Keep() { --g_gearKeepHand; } } keep;
    CancelJobsOnAxis(be, id, a, why);
}
// every beat and every fresh sample (MotorAccessTick / MotorAccessPollTick): the servo state, E1, and the ends of a hand session
void GearHandTick(IMotorAccessBackend& be)
{
    if (!g_gear.active || !g_gear.hand) return;
    if (be.GoldenSystemStart()) { CancelGear(be, "機台運轉中（SystemStart：MainProc 接手所有軸）"); return; }
    if (GearIssuedSeqOf(g_gear.axis) != g_gear.issuedSeq) { CancelGear(be, "這一軸有別的命令（JOG／移動／HOME／教導頁……）—— 手推量測被打斷"); return; }
    const int ax = g_gear.axis;
    unsigned long io = 0;
    unsigned st = 0;
    const bool hasIo = be.Pci1203MotionIO(ax, io), hasSt = be.Pci1203AxisState(ax, st);
    if ((hasIo && (io & 0x00000002ul) != 0) || (hasSt && IsErrorState(st))) { CancelGear(be, "這一軸驅動器 ALM 或 ERROR_STOP"); return; }
    bool known = false;
    const bool on = be.Pci1203ServoOn(ax, known);
    if (!known) return;                                                         // no sample: wait (the session block says what it waits for)
    if (!on) { if (g_gear.handPhase == 2) { g_gear.candKnown = false; return; } if (g_gear.handPhase != 1) GearHandMarkOff(); return; }   // NB2-1: phase 2 = a servo ON was sent; an OFF sample then is the drive not enabled yet (its SVON comes a few polls later) -- wait, do not count a new push (was: back to phase 1, pushes + 1, the page told the operator to push again); an explicit servo OFF still voids E1 (GearHandServoSeen)
    if (g_gear.handPhase == 1) { g_gear.handPhase = 2; g_gear.onPoll = be.Pci1203PollCount(); g_gear.candKnown = false; return; }
    if (g_gear.handPhase != 2) return;
    const unsigned long pc = be.Pci1203PollCount();
    if (pc <= g_gear.onPoll) return;                                            // no sample taken after the servo on yet
    double act = 0.0;
    if (!hasSt || !IsReadyState(st) || !be.Pci1203ActPos(ax, act) || !std::isfinite(act)) { g_gear.candKnown = false; return; }
    if (!g_gear.candKnown) { g_gear.cand = act; g_gear.candPoll = pc; g_gear.candKnown = true; return; }
    if (pc <= g_gear.candPoll) return;                                          // the same sample again
    if (std::fabs(act - g_gear.cand) > GearHandSettleCard(g_gear.ratio)) { g_gear.cand = act; g_gear.candPoll = pc; return; }   // still moving: start over
    g_gear.e1Card = act; g_gear.handPhase = 3; g_gear.candKnown = false; g_gear.e1At = NowIso();
    g_lastJobNote = "gear ratio " + g_gear.motor + ": hand session " + std::to_string(g_gear.id) + " E1 taken";
}
// "" = the hand session can be previewed / saved now; `wait` = not there yet (a step to do), not an interruption
std::string GearHandWhy(IMotorAccessBackend& be, const std::string& alias, const MotorGolden& g, bool* wait = 0)
{
    if (wait) *wait = false;
    if (!g_gear.active) return "沒有進行中的手推量測" + (g_gear.why.empty() ? std::string() : "（上一次：" + g_gear.why + "）") + " —— 先按「開始（記錄 E0）」";
    if (!g_gear.hand) return "進行中的是移動量測（" + g_gear.motor + "）—— 先按 STOP 結束，才能用手推量測";
    if (g_gear.motor != alias) return "進行中的手推量測是 " + g_gear.motor + " 的，不是 " + alias;
    if (g.gearRatio != g_gear.ratio) return "量測開始之後齒輪比變了（" + GearNum(g_gear.ratio) + " → " + GearNum(g.gearRatio) + "）";
    if (GearIssuedSeqOf(g_gear.axis) != g_gear.issuedSeq) return "量測期間這一軸有別的命令（JOG／移動／HOME／教導頁……）—— 手推量測被打斷，請按 STOP 結束後重新開始";
    if (g_gear.handPhase != 3) {
        if (wait) *wait = true;
        if (g_gear.handPhase == 0) return "還沒推：按「伺服 OFF」、用手把軸推到尺上的終點記號、再按「伺服 ON」";
        if (g_gear.handPhase == 1) return "伺服 OFF：推到尺上的終點記號，再按「伺服 ON」";
        return "伺服 ON 了：等監看器讀到停穩的新樣本（READY、連續兩次相同）才記 E1";
    }
    bool known = false;
    const bool on = be.Pci1203ServoOn(g_gear.axis, known);
    if (!known || !on) { if (wait) *wait = true; return "伺服讀不到或是 OFF：記下 E1 之後要保持伺服 ON"; }
    if (SampleStale(be, g_gear.axis)) { if (wait) *wait = true; return "監看器還沒有新樣本（稍候再按）"; }
    unsigned st = 0;
    if (!be.Pci1203AxisState(g_gear.axis, st)) return "讀不到這一軸的監看器樣本";
    if (IsErrorState(st)) return "這一軸 ERROR_STOP（驅動器／卡片報錯）—— 量測中止";
    if (!IsReadyState(st)) { if (wait) *wait = true; return "軸不在 READY（還在動？）—— 等它停好"; }
    double act = 0.0;
    if (!be.Pci1203ActPos(g_gear.axis, act)) return "讀不到這一軸的編碼器位置（監看器 actPos）";
    if (std::fabs(act - g_gear.e1Card) > GearHandSettleCard(g_gear.ratio)) {
        char b[320];
        std::snprintf(b, sizeof(b), "記下 E1 之後軸動了（編碼器現在 %.0f，E1 是 %.0f 脈波）—— 再推一次（伺服 OFF → 推 → 伺服 ON）", act, g_gear.e1Card);
        return b;
    }
    if (std::fabs(g_gear.e1Card - g_gear.e0Card) <= GearHandSettleCard(g_gear.ratio))
        return "E1 跟 E0 幾乎一樣：軸沒有被推動（或伺服 ON 時被拉回原位）—— 再推一次";
    return std::string();
}

void GearWriteSession(webbridge::JsonWriter& w, IMotorAccessBackend& be)
{
    if (g_gear.id == 0) { w.Null(); return; }
    w.BeginObject();
    w.Key("active").Bool(g_gear.active);
    w.Key("id").Number((wb_int64)g_gear.id);
    w.Key("motor").String(g_gear.motor);
    w.Key("ratio").Number(g_gear.ratio);
    w.Key("dirSign").Number((wb_int64)g_gear.dirSign);
    w.Key("startUser").Number((wb_int64)g_gear.startUser);
    w.Key("zeroUser").Number((wb_int64)g_gear.zeroUser);
    w.Key("zeroCard").Number(g_gear.zeroCard);
    w.Key("lastTargetUser").Number((wb_int64)g_gear.lastTargetUser);
    w.Key("lastTargetCard").Number(g_gear.lastTargetCard);
    w.Key("arriveCmdPos").Number((wb_int64)g_gear.arriveCmdPos);
    w.Key("placeholder").Bool(g_gear.placeholder);
    w.Key("points").BeginArray();
    for (std::size_t i = 0; i < g_gear.points.size(); ++i) {
        const GearCalPoint& p = g_gear.points[i];
        w.BeginObject(); w.Key("c").Number(p.cNom); w.Key("cTrue").Number(p.cTrue); w.Key("dir").Number((wb_int64)p.dir);
        w.Key("target").Number((wb_int64)p.targetUser); w.Key("cardTarget").Number(p.targetCard); w.EndObject();
    }
    w.EndArray();
    w.Key("mode").String(g_gear.hand ? "hand" : "move");                         //AI(W906-GEARHAND) 20261003: + the hand-push session's own keys
    if (g_gear.hand) {
        const bool e1 = g_gear.handPhase == 3;
        w.Key("phase").Number((wb_int64)g_gear.handPhase);
        w.Key("e0Card").Number(g_gear.e0Card);
        w.Key("e1Card"); if (e1) w.Number(g_gear.e1Card); else w.Null();
        w.Key("deltaCard"); if (e1) w.Number(g_gear.e1Card - g_gear.e0Card); else w.Null();
        w.Key("deltaMm"); if (e1) w.Number((g_gear.e1Card - g_gear.e0Card) * g_gear.ratio / 100.0); else w.Null();   // what the encoder says at this GearRatio (the move mode's cTrue)
        w.Key("e1At").String(g_gear.e1At);
        w.Key("pushes").Number((wb_int64)g_gear.pushes);
        bool known = false;
        const bool on = g_gear.axis >= 0 && be.Pci1203ServoOn(g_gear.axis, known);
        w.Key("servoOn"); if (known) w.Bool(on); else w.Null();
        double act = 0.0;
        w.Key("actCard"); if (g_gear.axis >= 0 && be.Pci1203ActPos(g_gear.axis, act)) w.Number(act); else w.Null();
        w.Key("encBase").Bool(g_gear.axis >= 0 && g_encBase.count(g_gear.axis) != 0);
    }
    if (g_gear.active) {
        MotorAccessAxis a;
        MotorGolden g;
        bool wait = false;
        std::string iw = "馬達表上沒有這一軸";
        if (be.Resolve(g_gear.motor, a) && a.motIndex >= 0 && be.GoldenMotor(a.motIndex, g)) iw = g_gear.hand ? GearHandWhy(be, g_gear.motor, g, &wait) : GearIntactWhy(be, g_gear.motor, g, &wait);   //AI(W906-GEARHAND) 20261003: a hand session's "intact" = GearHandWhy
        w.Key("fresh").Bool(!SampleStale(be, g_gear.axis));
        w.Key("arrived").Bool(iw.empty());
        w.Key("waiting").Bool(wait);
        w.Key("intact").Bool(iw.empty() || wait);
        w.Key("intactWhy").String(iw);
    }
    w.Key("why").String(g_gear.why);
    w.Key("startedAt").String(g_gear.startedAt);
    w.EndObject();
}

MotorAccessOutcome DoGearCalMove(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    { const std::string busy = JobsBusyWhy(r.action, r, be); if (!busy.empty()) return Refuse(busy); }
    MotionCtx m;
    if (!MotionPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string alias = r.motors[0], who = "gearCalMove " + alias;
    { const std::string sw = GearStaticWhy(alias, m.a, m.g); if (!sw.empty()) return Refuse(who + ": " + sw); }
    { const std::string bw = GearEncBaseWhy(alias, m.a); if (!bw.empty()) return Refuse(who + ": " + bw); }
    { std::string ew; if (be.GearEmgStop(ew)) return Refuse(who + ": " + ew); }
    {
        unsigned long io = 0;
        unsigned st = 0;
        if (!be.Pci1203MotionIO(m.a.axis, io) || !be.Pci1203AxisState(m.a.axis, st))
            return Refuse(who + ": 讀不到這一軸的監看器樣本（ALM／狀態不明，fail-closed）");
        if ((io & 0x00000002ul) != 0 || IsErrorState(st))
            return Refuse(who + ": 驅動器 ALM 或 ERROR_STOP（先排除異常、按 Alarm Reset、再開伺服）");
    }
    { bool known = false; const bool on = be.Pci1203ServoOn(m.a.axis, known); if (!known || !on) return Refuse(who + ": 伺服沒開（SVON 讀不到或是 OFF）—— 先按 Servo On"); }
    if (m.g.homeFlag != 1) return Refuse(who + ": Motor not home 馬達尚未歸零（HomeFlag=" + std::to_string(m.g.homeFlag) + "）—— 先按 HOME");
    double d = 0.0, pctD = 0.0;
    if (!ParamNum(r, "distanceMm", d) || !std::isfinite(d)) return Refuse(who + ": 缺 distanceMm（帶正負號的 mm）");
    if (!ParamNum(r, "speedPct", pctD) || !std::isfinite(pctD)) return Refuse(who + ": 缺 speedPct（1..20）");
    const double hund = d * 100.0;
    const long long du = std::llround(hund);
    if (du == 0 || std::fabs(hund - (double)du) > 1e-6) return Refuse(who + ": 距離 " + GearNum(d) + " mm 不是 0.01 mm 的整數倍（或是 0）");
    if (pctD != std::floor(pctD) || pctD < 1.0 || pctD > (double)kGearSpeedMaxPct)
        return Refuse(who + ": 速度 " + GearNum(pctD) + " % 不在 1..20 %（量測用低速，NB2 規格 §5.1）");
    const int pct = (int)pctD;
    std::map<std::string, bool>::const_iterator bf = r.flag.find("begin");
    const bool begin = (bf != r.flag.end() && bf->second);
    const bool placeholder = GearPlaceholderLimits(m.g);
    int base = 0;
    if (begin) {
        if (g_gear.active) return Refuse(who + ": 量測進行中（" + g_gear.motor + "，第 " + std::to_string(g_gear.id) + " 次量測）—— 先按 STOP 結束這次量測，才能重新按 Start");   //AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (b) -- a begin used to REPLACE the active session (a fresh zero where the last leg stopped: Start after a +100 mm leg gave another 100 mm, nothing else bounding a +-999999 axis); now refused until the session ends (STOP, a save, FormClose ... -- CancelGear)
        if (std::fabs(d) > kGearTakeupMaxMm) return Refuse(who + ": 消背隙那一段要 ≤ " + GearMmText(kGearTakeupMaxMm) + " mm（現在 " + GearNum(d) + " mm）");
        if (!CurrentUserPos(be, m, base)) return Refuse(who + ": 讀不到目前位置");
    } else {
        bool wait = false;
        const std::string iw = GearIntactWhy(be, alias, m.g, &wait);
        if (!iw.empty()) return Refuse(who + ": " + iw);
        base = g_gear.lastTargetUser;
        if (placeholder && std::fabs(d) > kGearStepMaxMm)
            return Refuse(who + ": 軟體極限還是 ±999999 佔位值（不限制任何東西）：每段要 ≤ " + GearMmText(kGearStepMaxMm) + " mm（現在 " + GearNum(d) + " mm）");
    }
    if (base > 99999999 || base < -99999999) return Refuse(who + ": 目前位置超出範圍");
    const long long t64 = (long long)base + du;
    if (t64 > 2000000000LL || t64 < -2000000000LL) return Refuse(who + ": 目標超出範圍");
    const int target = (int)t64;
    if (!begin && placeholder) {   //AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (a) -- counted from the session's START point (spec §5.1 「離開起點的總距離 ≤ 100 mm」), not from the gauge zero, which sits the take-up (<= 5 mm) past it: the old rule let a leg end 105 mm from where Start was pressed
        const long long fromStart = t64 - (long long)g_gear.startUser;
        if (fromStart > (long long)(kGearSpanMaxMm * 100.0) || fromStart < -(long long)(kGearSpanMaxMm * 100.0))
            return Refuse(who + ": 軟體極限還是 ±999999 佔位值：離開起點（按 Start 時的位置）最多 " + GearMmText(kGearSpanMaxMm) + " mm（這一段會到離起點 " + GearMmText((double)fromStart / 100.0) + " mm；量具零點在起點的 " + GearMmText((double)(g_gear.zeroUser - g_gear.startUser) / 100.0) + " mm 處）");
    }
    const int arm = be.GearArmOf(m.a.motIndex);
    if (arm < 0) return Refuse(who + ": 這個後端分不出這一軸是不是手臂的 X／Y（GearArmOf）—— 不移動");
    if (arm == 1 || arm == 2) {
        std::vector<int> zs;
        std::string zw;
        if (!be.GoldenTeachFixedMotors(arm == 1 ? "inZ" : "outZ", zs, zw)) return Refuse(who + ": 找不到這支手臂的 Z（" + zw + "）—— 不移動");
        for (std::size_t i = 0; i < zs.size(); ++i) {
            g_teachHomeUnknown.clear();
            const int led = MotorAccessTeachInterlockHome(be, zs[i]);
            if (led == 1 || led == -3 || led == -2) continue;                  // at home / Enable=0 non-1203 (not checked) / non-1203: golden's own check below
            std::string za;
            if (!be.AliasOfMotIndex(zs[i], za)) za = "MOT[" + std::to_string(zs[i]) + "]";
            return Refuse(who + ": " + za + (led == 0 ? " 不在原點（ORG 沒亮）" : " 的原點狀態不明（fail-closed）：" + g_teachHomeUnknown) +
                          " —— 手臂 X／Y 量測前，這支手臂所有的 Z 都要在原點（先按 HOME 或教導頁 In／Out Z All Up）");
        }
    }
    g_teachHomeUnknown.clear();
    if (!be.GoldenTeachCanMove(m.a.motIndex)) {
        std::string w = who + ": golden 教導頁的移動互鎖 CheckCanMove()／IsCanQuickJogMove() 不允許（急停、Z 不在原點、閘門……；訊息框已顯示原因）";
        if (!g_teachHomeUnknown.empty()) w += "；其中 1203 軸的原點狀態不明（當成不在原點，fail-closed）：" + g_teachHomeUnknown;
        return Refuse(w);
    }
    if (SampleStale(be, m.a.axis)) return Refuse(who + ": " + kStaleWhy);
    {
        const std::string mb = MoveBlock1203(be, m.a, m.g, who, target);
        if (!mb.empty()) return Refuse(mb);
    }
    std::string why, note;
    if (!Send1203Speed(be, wireId, m, pct, why, note)) return Refuse(who + ": " + why);
    g_ptpPct[m.a.motIndex] = pct;
    NoteRotatorDir(be, m.a, m.g, target);
    const int card = MotorUserToCard(target, m.g.gearRatio);
    Pci1203Cmd c;
    c.kind = kCmdAxMoveAbs; c.wireId = wireId; c.axis = m.a.axis; c.value = (double)card;
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res)) return Refuse(who + ": 1203 移動失敗 —— " + CmdWhy(res));
    NoteIssued(be, m.a.axis);
    if (begin) {
        g_gear = GearCalSession();
        g_gear.active = true;
        g_gear.id = ++g_gearIds;
        g_gear.motor = alias; g_gear.mi = m.a.motIndex; g_gear.axis = m.a.axis; g_gear.ratio = m.g.gearRatio;
        g_gear.dirSign = d > 0 ? 1 : -1;
        g_gear.startUser = base;
        g_gear.zeroUser = target; g_gear.zeroCard = (double)card;
        g_gear.placeholder = placeholder;
        g_gear.startedAt = NowIso();
        g_lastJobNote = "gear ratio " + alias + ": session " + std::to_string(g_gear.id) + " started (take-up " + GearNum(d) + " mm)";
    } else {
        GearCalPoint p;
        p.cNom = (double)(target - g_gear.zeroUser) / 100.0;
        volatile double ct = ((double)card - g_gear.zeroCard) * g_gear.ratio / 100.0;
        p.cTrue = ct;
        p.dir = ((d > 0 ? 1 : -1) == g_gear.dirSign) ? 1 : -1;
        p.targetUser = target; p.targetCard = (double)card;
        g_gear.points.push_back(p);
    }
    g_gear.lastTargetUser = target;
    g_gear.lastTargetCard = (double)card;
    g_gear.arriveCmdPos = MotorCardToUser((double)card, m.g.gearRatio);
    g_gear.issuedSeq = g_issuedSeq[m.a.axis];
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "moving", "pci1203",
            std::string(res.issued ? "sent: " : "accepted, NOT issued (dry): ") + res.wouldCall + "; " + note +
            (begin ? "；量測開始：這一段是消背隙，到位後把量具歸零" : "") + "（受理不等於到位：到位看 runtime）");
    w.Key("axis").Number((wb_int64)m.a.axis);
    w.Key("motIndex").Number((wb_int64)m.a.motIndex);
    w.Key("target").Number((wb_int64)target);
    w.Key("cardTarget").Number((wb_int64)card);
    w.Key("arriveCmdPos").Number((wb_int64)g_gear.arriveCmdPos);
    w.Key("begin").Bool(begin);
    w.Key("c").Number(begin ? 0.0 : g_gear.points.back().cNom);
    w.Key("dir").Number((wb_int64)(begin ? 0 : g_gear.points.back().dir));
    w.Key("speedPct").Number((wb_int64)pct);
    w.Key("placeholder").Bool(placeholder);
    w.Key("session"); GearWriteSession(w, be);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

//AI(W906-GEARHAND) 20261003: gearHandBegin -- open a hand-push session and take E0 (see the HAND-PUSH block head). Sends nothing.
MotorAccessOutcome DoGearHandBegin(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    { const std::string busy = JobsBusyWhy(r.action, r, be); if (!busy.empty()) return Refuse(busy); }
    MotionCtx m;
    if (!MotionPrelude(r, wireId, be, m, true)) return Refuse(m.why);
    const std::string alias = r.motors[0], who = "gearHandBegin " + alias;
    if (g_gear.active)
        return Refuse(who + ": 量測進行中（" + g_gear.motor + "，第 " + std::to_string(g_gear.id) + " 次量測，" + (g_gear.hand ? "手推" : "移動") + "）—— 先按 STOP 結束這次量測，才能重新開始");
    { const std::string sw = GearHandStaticWhy(alias, m.a, m.g); if (!sw.empty()) return Refuse(who + ": " + sw); }
    { std::string ew; if (be.GearEmgStop(ew)) return Refuse(who + ": " + ew); }
    {
        unsigned long io = 0;
        unsigned st = 0;
        if (!be.Pci1203MotionIO(m.a.axis, io) || !be.Pci1203AxisState(m.a.axis, st))
            return Refuse(who + ": 讀不到這一軸的監看器樣本（ALM／狀態不明，fail-closed）");
        if ((io & 0x00000002ul) != 0 || IsErrorState(st))
            return Refuse(who + ": 驅動器 ALM 或 ERROR_STOP（先排除異常、按 Alarm Reset、再開伺服）");
        bool known = false;
        const bool on = be.Pci1203ServoOn(m.a.axis, known);
        if (!known || !on) return Refuse(who + ": 伺服沒開（SVON 讀不到或是 OFF）—— 先按 Servo On，讓軸停在尺的起點記號，再按開始");
        if (SampleStale(be, m.a.axis)) return Refuse(who + ": " + kStaleWhy);
        if (!IsReadyState(st)) return Refuse(who + ": 這一軸不在 READY（還在動？）—— 等它停好再按開始");
    }
    double e0 = 0.0;
    if (!be.Pci1203ActPos(m.a.axis, e0) || !std::isfinite(e0)) return Refuse(who + ": 讀不到這一軸的編碼器位置（監看器 actPos）—— 不開始");
    g_gear = GearCalSession();
    g_gear.active = true;
    g_gear.hand = true;
    g_gear.id = ++g_gearIds;
    g_gear.motor = alias; g_gear.mi = m.a.motIndex; g_gear.axis = m.a.axis; g_gear.ratio = m.g.gearRatio;
    g_gear.e0Card = e0;
    g_gear.issuedSeq = GearIssuedSeqOf(m.a.axis);
    g_gear.placeholder = GearPlaceholderLimits(m.g);
    g_gear.startedAt = NowIso();
    char b[64];
    std::snprintf(b, sizeof(b), "%.0f", e0);
    g_lastJobNote = "gear ratio " + alias + ": hand session " + std::to_string(g_gear.id) + " started (E0 " + b + ")";
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "handArmed", "pci1203", std::string("手推量測開始：E0 = ") + b + " 脈波（監看器的編碼器；沒有下任何移動命令）。"
            "下一步：按「伺服 OFF」、用手把軸推到尺上的終點記號、再按「伺服 ON」");
    w.Key("axis").Number((wb_int64)m.a.axis);
    w.Key("motIndex").Number((wb_int64)m.a.motIndex);
    w.Key("e0Card").Number(e0);
    w.Key("session"); GearWriteSession(w, be);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- gearRatioPreview / gearRatioSave: the common computation ----
struct GearCalc1 {
    MotionCtx   m;
    std::string alias, who;
    double      oldRatio = 0.0;
    std::vector<GearMeas> meas;       // what the fit used (cTrue where the session had the point)
    std::vector<double> pageC;
    bool        sessionMatched = false;
    std::string sessionWhy;           // "" = every page point is a point of the intact session
    GearFit     fit;
    bool        hasPlan = false;
    GearPlan    plan;
    std::string planWhy;
    std::vector<GearTeachRef> refs;
    std::vector<GearManualItem> extras;   // not in the key: the per-axis reminders of spec §6
    std::vector<std::string> notes;
    bool        teachOpen = true;
    std::string teachOpenWhy;
    bool        hand = false;                 //AI(W906-GEARHAND) 20261003: {hand:true, rulerMm}: the hand-push session's E0 / E1 + the ruler
    double      rulerMm = 0.0, handE0 = 0.0, handE1 = 0.0;
};
void GearComputeTail(IMotorAccessBackend& be, GearCalc1& C);

//AI(W906-GEARHAND) 20261003: the measurement of a hand session -- ONE forward point, c = (E1 - E0) x GearRatio / 100 mm (the move
//  mode's cTrue convention: what the pulses stand for), a = the ruler; the fit with minForward = 1, every other rule unchanged.
//  false + why = refused (no ruler / a bad one / no E1 yet); E1 taken but not intact (moved since, servo off ...) = computed, not saveable.
bool GearHandMeasure(const MotorAccessReq& r, IMotorAccessBackend& be, GearCalc1& C, std::string& why)
{
    C.hand = true;
    double ruler = 0.0;
    if (!ParamNum(r, "rulerMm", ruler) || !std::isfinite(ruler)) { why = C.who + ": 缺 rulerMm（尺上量到的距離 mm，帶正負號：往負方向推就是負數）"; return false; }
    const double h = ruler * 100.0;
    const long long ru = std::llround(h);
    if (ru == 0 || std::fabs(h - (double)ru) > 1e-6) { why = C.who + ": 尺的距離 " + GearNum(ruler) + " mm 不是 0.01 mm 的整數倍（或是 0）"; return false; }
    if (std::fabs(ruler) > kGearHandRulerMaxMm) { why = C.who + ": 尺的距離 " + GearNum(ruler) + " mm 不合理（> " + GearMmText(kGearHandRulerMaxMm) + " mm）"; return false; }
    C.rulerMm = ruler;
    bool wait = false;
    const std::string hw = GearHandWhy(be, C.alias, C.m.g, &wait);
    if (!(g_gear.active && g_gear.hand && g_gear.motor == C.alias && g_gear.handPhase == 3)) { why = C.who + ": " + hw; return false; }
    C.sessionMatched = hw.empty();
    C.sessionWhy = hw;
    C.handE0 = g_gear.e0Card; C.handE1 = g_gear.e1Card;
    GearMeas x;
    volatile double ct = (g_gear.e1Card - g_gear.e0Card) * g_gear.ratio / 100.0;
    x.c = ct; x.a = ruler; x.dir = 1;
    C.meas.push_back(x);
    C.pageC.push_back(x.c);
    GearFitLimits L;
    L.minForward = 1;
    C.fit = GearFitCompute(C.oldRatio, C.meas, L);
    if (x.c * x.a < 0.0)
        C.fit.why = "尺的距離正負號跟編碼器走的方向相反（編碼器 " + GearMmText(x.c) + " mm、尺 " + GearMmText(x.a) + " mm）：往負方向推，尺的距離就輸入負數";
    return true;
}

// false + why = refused before any computation (the request itself is wrong)
bool GearCompute(const MotorAccessReq& r, IMotorAccessBackend& be, GearCalc1& C, std::string& why)
{
    std::string who;
    if (!PageMotor(r, be, C.m, who, why)) return false;
    C.alias = r.motors[0];
    C.who = r.action + " " + C.alias;
    { const std::string sw = GearStaticWhy(C.alias, C.m.a, C.m.g); if (!sw.empty()) { why = C.who + ": " + sw; return false; } }
    if (!ParamNum(r, "oldRatio", C.oldRatio)) { why = C.who + ": 缺 oldRatio（頁面看到的目前齒輪比）"; return false; }
    if (C.oldRatio != C.m.g.gearRatio) {
        why = C.who + ": 頁面的目前齒輪比 " + GearNum(C.oldRatio, 17) + " 不是 C++ 的 MOT 現值 " + GearNum(C.m.g.gearRatio, 17) + " —— 重新整理後再算";
        return false;
    }
    {   //AI(W906-GEARHAND) 20261003: {hand:true} = the hand-push session (GearHandMeasure), then the same plan / notes as a move session
        std::map<std::string, bool>::const_iterator hf = r.flag.find("hand");
        if (hf != r.flag.end() && hf->second) {
            if (!GearHandMeasure(r, be, C, why)) return false;
            GearComputeTail(be, C);
            return true;
        }
    }
    std::map<std::string, std::vector<double> >::const_iterator ic = r.arr.find("measC"), ia = r.arr.find("measA"), id = r.arr.find("measDir");
    if (ic == r.arr.end() || ia == r.arr.end() || id == r.arr.end() || ic->second.size() != ia->second.size() || ic->second.size() != id->second.size() || ic->second.empty()) {
        why = C.who + ": 量測值要三個一樣長的數字陣列 measC／measA／measDir（命令距離 mm、量具讀數 mm、+1 往前／-1 往回）";
        return false;
    }
    if (ic->second.size() > 64) { why = C.who + ": 量測點太多（最多 64 點）"; return false; }
    bool intact = false;
    std::string iw;
    {
        bool wait = false;
        iw = GearIntactWhy(be, C.alias, C.m.g, &wait);
        intact = iw.empty();
    }
    int matched = 0;
    for (std::size_t i = 0; i < ic->second.size(); ++i) {
        GearMeas x;
        x.c = ic->second[i]; x.a = ia->second[i];
        const double dv = id->second[i];
        x.dir = (dv == 1.0) ? 1 : (dv == -1.0) ? -1 : 0;
        C.pageC.push_back(x.c);
        if (g_gear.motor == C.alias && g_gear.id != 0 && x.dir != 0) {
            int hit = -1;
            for (std::size_t k = 0; k < g_gear.points.size(); ++k)
                if (g_gear.points[k].dir == x.dir && std::fabs(g_gear.points[k].cNom - x.c) <= kGearMatchMm) hit = (int)k;   // the last one wins
            if (hit >= 0) { x.c = g_gear.points[(std::size_t)hit].cTrue; ++matched; }
        }
        C.meas.push_back(x);
    }
    C.sessionMatched = intact && matched == (int)C.meas.size();
    if (!intact) C.sessionWhy = iw;
    else if (matched != (int)C.meas.size())
        C.sessionWhy = std::to_string(C.meas.size() - (std::size_t)matched) + " 個量測點不是這次量測走過的位置（命令距離或方向對不上）";
    C.fit = GearFitCompute(C.oldRatio, C.meas);
    GearComputeTail(be, C);   //AI(W906-GEARHAND) 20261003: the plan / extras / notes below moved into GearComputeTail (shared with the hand mode), unchanged
    return true;
}
void GearComputeTail(IMotorAccessBackend& be, GearCalc1& C)
{
    C.teachOpen = be.GearTeachWindowOpen(C.teachOpenWhy);
    if (C.fit.ok()) {
        std::string rw;
        if (!be.GearTeachRefs(C.refs, rw)) C.planWhy = "讀不到教導頁的登錄表：" + rw;
        else {
            GearPlanInput in;
            in.alias = C.alias; in.mi = C.m.a.motIndex; in.oldRatio = C.oldRatio; in.newRatio = C.fit.newRatio;
            in.softP = C.m.g.softP; in.softN = C.m.g.softN;
            IMotorAccessBackend* pb = &be;
            C.plan = GearPlanBuild(in, C.refs, [pb](int mot) { return pb->GoldenTeachRemap(mot); });
            C.hasPlan = C.plan.ok;
            if (!C.plan.ok) C.planWhy = C.plan.why;
            else if (C.fit.newRatio == C.oldRatio) C.planWhy = "新齒輪比跟目前的一樣（" + GearNum(C.oldRatio) + "）—— 不需要存";
        }
    } else {
        C.planWhy = "量測沒有通過：" + C.fit.why;
    }
    // spec §6: not rescaled, listed for a manual check (per axis kind; not part of the preview key)
    GearManualItem mi;
    if (C.m.g.armZ) { mi = GearManualItem(); mi.where = "Gerneral.ini [In Arm] ZSafePos"; mi.hasValue = true; mi.value = be.GoldenZSafePos(); mi.why = "所有手臂 Z 共用的安全高度：不自動換算，請人工確認"; C.extras.push_back(mi); }
    if (C.m.g.indexZ) { mi = GearManualItem(); mi.where = "Mot_Table PickLimit（IndexPickLimit）"; mi.why = "Index Z 的取放極限：不自動換算，請人工確認"; C.extras.push_back(mi); }
    if (C.m.g.inShuttle) { mi = GearManualItem(); mi.where = "Gerneral.ini [Shuttle] CHECK_RANGE"; mi.why = "飛梭位置檢查範圍：不自動換算，請人工確認"; C.extras.push_back(mi); }
    { mi = GearManualItem(); char b[48]; std::snprintf(b, sizeof(b), "MotorTest.ini [M%02d] Position 1 / Position 2", C.m.a.motIndex); mi.where = b; mi.why = "Loop Move 的兩個位置：不自動換算，請人工確認"; C.extras.push_back(mi); }
    C.notes.push_back("工單裡的 offset 表（InArmOffSet 等）與料盤尺寸是實體 mm：不換算 —— 它們正是這次校正要對準的基準");
    C.notes.push_back("速度（脈波單位）不換算：物理速度只差 k 倍");
    C.notes.push_back("教導點換算後取整數：最多差 1 個單位（0.01 mm）");
    if (!C.hand) C.notes.push_back("存檔之後這一軸 HomeFlag=0：要重新回原點，" + GearVerifyHint(C.m.g));   //AI(W906-GEARRATIO2) 20261003: R171 L5 (was always "再走一次 200 mm")
    else {                                                                      //AI(W906-GEARHAND) 20261003: spec (9) -- the restart before this axis moves
        char b[200];
        std::snprintf(b, sizeof(b), "手推量測：E0 %.0f、E1 %.0f 脈波（差 %.0f 脈波，照目前齒輪比是 %.3f mm）；尺 %.2f mm",
                      C.handE0, C.handE1, C.handE1 - C.handE0, C.meas.empty() ? 0.0 : C.meas[0].c, C.rulerMm);
        C.notes.push_back(b);
        C.notes.push_back(GearHandRestartText(C.m.a.axis));
    }
}

void GearWriteFit(webbridge::JsonWriter& w, const GearFit& f)
{
    w.Key("fit").BeginObject();
    w.Key("level").String(f.level);
    w.Key("why").String(f.why);
    w.Key("computed").Bool(f.computed);
    w.Key("oldRatio").Number(f.oldRatio);
    w.Key("k"); if (f.computed) w.Number(f.k); else w.Null();
    w.Key("newRatio"); if (f.computed && f.newRatio > 0.0) w.Number(f.newRatio); else w.Null();
    w.Key("newRatioText").String(f.computed && f.newRatio > 0.0 ? GearNum(f.newRatio, 7) : std::string());
    w.Key("deviationPct"); if (f.computed) w.Number((f.k - 1.0) * 100.0); else w.Null();
    w.Key("consistencyPct"); if (f.computed) w.Number(f.consistency * 100.0); else w.Null();
    w.Key("ki").BeginArray(); for (std::size_t i = 0; i < f.ki.size(); ++i) w.Number(f.ki[i]); w.EndArray();
    w.Key("nForward").Number((wb_int64)f.nForward);
    w.Key("nBack").Number((wb_int64)f.nBack);
    w.Key("maxSpanMm").Number(f.maxSpan);
    w.Key("backlashMm"); if (f.hasBacklash) w.Number(f.backlash); else w.Null();
    w.Key("backlashPairs").Number((wb_int64)f.backlashPairs);
    w.EndObject();
}
void GearWriteManual(webbridge::JsonWriter& w, const GearManualItem& m)
{
    w.BeginObject(); w.Key("where").String(m.where); w.Key("value"); if (m.hasValue) w.Number((wb_int64)m.value); else w.Null(); w.Key("why").String(m.why); w.EndObject();
}
void GearWritePlan(webbridge::JsonWriter& w, const GearCalc1& C)
{
    w.Key("plan").BeginObject();
    w.Key("ok").Bool(C.hasPlan && C.planWhy.empty());
    w.Key("why").String(C.planWhy);
    w.Key("previewKey").String(C.hasPlan ? C.plan.key : std::string());
    if (C.hasPlan) {
        const GearPlan& p = C.plan;
        w.Key("softP").BeginObject(); w.Key("old").Number((wb_int64)p.in.softP); w.Key("new").Number((wb_int64)p.softPNew); w.Key("placeholder").Bool(p.softPPlaceholder); w.EndObject();
        w.Key("softN").BeginObject(); w.Key("old").Number((wb_int64)p.in.softN); w.Key("new").Number((wb_int64)p.softNNew); w.Key("placeholder").Bool(p.softNPlaceholder); w.EndObject();
        w.Key("teach").BeginArray();
        for (std::size_t i = 0; i < p.teach.size(); ++i) {
            const GearTeachChange& c = p.teach[i];
            w.BeginObject(); w.Key("where").String(c.where); w.Key("old").Number((wb_int64)c.oldV); w.Key("new").Number((wb_int64)c.newV);
            w.Key("slots").BeginArray(); for (std::size_t s = 0; s < c.slots.size(); ++s) w.String(c.slots[s]); w.EndArray(); w.EndObject();
        }
        w.EndArray();
        w.Key("changedCount").Number((wb_int64)p.changedCount);
        w.Key("zeroSkipped").Number((wb_int64)p.zeroSkipped);
    }
    w.Key("manual").BeginArray();
    if (C.hasPlan) for (std::size_t i = 0; i < C.plan.manual.size(); ++i) GearWriteManual(w, C.plan.manual[i]);
    for (std::size_t i = 0; i < C.extras.size(); ++i) GearWriteManual(w, C.extras[i]);
    w.EndArray();
    w.EndObject();
    w.Key("notes").BeginArray(); for (std::size_t i = 0; i < C.notes.size(); ++i) w.String(C.notes[i]); w.EndArray();
}
//AI(W906-GEARHAND) 20261003: the preview's mode, and a hand session's readings (what the fit's single point is made of)
void GearWriteHand(webbridge::JsonWriter& w, const GearCalc1& C)
{
    w.Key("mode").String(C.hand ? "hand" : "move");
    if (!C.hand) return;
    w.Key("hand").BeginObject();
    w.Key("e0Card").Number(C.handE0); w.Key("e1Card").Number(C.handE1); w.Key("deltaCard").Number(C.handE1 - C.handE0);
    w.Key("encMm").Number(C.meas.empty() ? 0.0 : C.meas[0].c); w.Key("rulerMm").Number(C.rulerMm);
    w.EndObject();
}

// "" = the save may run (preview and save use the same rule; save adds the key and the confirmation)
std::string GearSaveBlockedWhy(const GearCalc1& C)
{
    if (!C.fit.ok()) return "量測沒有通過：" + C.fit.why;
    if (!C.planWhy.empty()) return C.planWhy;
    if (!C.sessionMatched) return C.hand ? "手推量測還不能存：" + C.sessionWhy : "量測值要是這次量測（gearCalMove）走過的點，而且量測沒有被打斷：" + C.sessionWhy;   //AI(W906-GEARHAND) 20261003: + the hand session's own reason
    if (C.teachOpen) return "教導頁開著（" + C.teachOpenWhy + "）—— 先關掉教導頁：它開著的話，下一次存檔會把換算前的舊值蓋回去";
    return std::string();
}

// ---- the save transaction (spec §5.3 steps 1-5) ----
bool GearReadText(const std::string& p, std::string& out) { return MtReadFile(p, out); }
std::string GearUniqueBak(const std::string& path, const std::string& st)
{
    std::string bak = path + ".bak_" + st;
    for (int k = 2; MtExists(bak) && k < 100; ++k) bak = path + ".bak_" + st + "_" + std::to_string(k);
    return MtExists(bak) ? std::string() : bak;
}
std::vector<std::pair<const int*, int> > GearTeachValues(const GearPlan& p, bool newValues)
{
    std::vector<std::pair<const int*, int> > v;
    for (std::size_t i = 0; i < p.teach.size(); ++i)
        if (p.teach[i].newV != p.teach[i].oldV) v.push_back(std::make_pair(p.teach[i].ptr, newValues ? p.teach[i].newV : p.teach[i].oldV));
    return v;
}
//AI(W906-GEARRATIO2) 20261003: NB2 R171 M2 follow-up (coordinator, the user's default = option A) -- what the operator does when the
//  verify below finds keys OUTSIDE the plan: on a teach.ini that this version's Teach page has never saved, the Teach writer
//  itself adds the elTeach keys the file lacks and evens out the format (ctest GearTeachSave: 49 keys on the HT9050 snapshot),
//  so the first Gear Ratio save there is refused and restored. Said first (the page's one-line status shows the start), the
//  detail list after it, unchanged. Only for keys OUTSIDE the plan (GearTeachDiffWhy's `outside`): the live writer writes every
//  key from memory, so an outside key that moved was stale in the file and a Teach-page save rewrites it the same way; a PLAN key
//  with a value the plan did not ask for is a writer fault that a Teach-page save does not cure -> the plain refusal.
const char* const kGearTeachNotNormalized =
    "teach.ini 還沒有用目前版本的教導頁存過檔，所以這次沒有存：Mot_Table.csv、teach.ini 與記憶體都已用備份還原，什麼都沒改 —— "
    "請先在教導頁按一次「存檔」、關掉教導頁，再回來重新 Preview、Save（量測若已結束就重新量）。"
    "教導頁的寫檔器存檔時會補上新版的鍵、統一格式，所以這次多改了不在換算計畫裡的鍵。明細：";
// the teach.ini diff may only touch the keys of the changed variables, each with its new value
//AI(W906-GEARRATIO2) 20261003: + `outside` (optional) = how many of the bad keys are not plan keys at all (kGearTeachNotNormalized)
std::string GearTeachDiffWhy(const GearPlan& p, const std::string& oldText, const std::string& newText, int* outside = 0)
{
    std::map<std::string, std::set<int> > want;
    for (std::size_t i = 0; i < p.teach.size(); ++i) {
        const GearTeachChange& c = p.teach[i];
        if (c.newV == c.oldV) continue;
        for (std::size_t k = 0; k < c.keys.size(); ++k) {
            std::string kl = c.keys[k];
            for (std::size_t q = 0; q < kl.size(); ++q) if (kl[q] >= 'A' && kl[q] <= 'Z') kl[q] = (char)(kl[q] - 'A' + 'a');
            want[kl].insert(c.newV);
        }
    }
    const std::vector<GearIniDelta> d = GearIniDiff(oldText, newText);
    std::string bad;
    int nBad = 0;
    for (std::size_t i = 0; i < d.size(); ++i) {
        std::string kl = d[i].key;
        for (std::size_t q = 0; q < kl.size(); ++q) if (kl[q] >= 'A' && kl[q] <= 'Z') kl[q] = (char)(kl[q] - 'A' + 'a');
        std::map<std::string, std::set<int> >::const_iterator it = want.find(kl);
        char* e = 0;
        const long v = d[i].hasNew ? std::strtol(d[i].newV.c_str(), &e, 10) : 0;
        const bool okNum = d[i].hasNew && e && *e == '\0' && !d[i].newV.empty();
        if (it != want.end() && okNum && it->second.count((int)v)) continue;
        if (outside && it == want.end()) ++*outside;                            //AI(W906-GEARRATIO2) 20261003: not a plan key
        if (++nBad <= 5) bad += (bad.empty() ? "" : "；") + std::string("[") + d[i].section + "] " + d[i].key + " " +
                                (d[i].hadOld ? d[i].oldV : std::string("(沒有)")) + " → " + (d[i].hasNew ? d[i].newV : std::string("(刪掉)"));
    }
    if (nBad == 0) return std::string();
    return "teach.ini 有 " + std::to_string(nBad) + " 個不該變的鍵變了：" + bad;
}
struct GearTxn {
    std::string teachPath, motPath, teachBak, motBak, stamp, teachOld, motOld, teachNote;
    bool        teachBakMade = false, motWritten = false, memoryTouched = false, teachWritten = false;
    bool        restored = false;
    std::string restoreWhy;
    MotTableSaveResult mot;
};
std::string GearRestore(IMotorAccessBackend& be, const GearCalc1& C, GearTxn& T)
{
    std::string bad;
    if (T.teachWritten && !MtWriteFile(T.teachPath, T.teachOld)) bad += "teach.ini 寫不回去；";
    if (T.motWritten && !MtWriteFile(T.motPath, T.motOld)) bad += "Mot_Table.csv 寫不回去；";
    if (T.memoryTouched) {
        std::string w;
        if (!be.GearSetMotor(C.m.a.motIndex, C.oldRatio, C.m.g.softP, C.m.g.softN, w)) bad += "MOT 參數改不回去（" + w + "）；";
        std::string out;
        const MotTableEditResult er = MotTableEditRow(T.motOld, C.alias, C.m.a.motIndex, MotTableGearFieldsOf(C.oldRatio, C.m.g.softP, C.m.g.softN),
                                                      be.MotTableIndexForced(), C.m.a.cardModel, out);
        if (er.ok) {
            std::vector<std::pair<std::string, std::string> > cells;
            for (std::size_t i = 0; i < er.cells.size(); ++i) cells.push_back(std::make_pair(er.cells[i].column, er.cells[i].oldText));
            be.MotTableRowSaved(C.alias, er.newRowText, cells);
        } else bad += "HSys.MotTable 那一列還原不了（" + er.why + "）；";
        if (!be.GearSetTeach(GearTeachValues(C.plan, false), w)) bad += "教導點改不回去（" + w + "）；";
    }
    if (T.teachWritten || T.memoryTouched) {
        std::string w, note;
        if (!be.GearTeachSaveReload(false, w, note)) bad += "teach.ini 重讀失敗（" + w + "）；";
    }
    std::string now;
    if (T.teachWritten && (!MtReadFile(T.teachPath, now) || now != T.teachOld)) bad += "teach.ini 還原後內容不對；";
    if (T.motWritten && (!MtReadFile(T.motPath, now) || now != T.motOld)) bad += "Mot_Table.csv 還原後內容不對；";
    T.restored = bad.empty();
    T.restoreWhy = bad;
    return bad;
}
// "" = saved and verified; else why (the files and the memory were restored when anything had been written -- T says)
std::string GearSaveTxn(IMotorAccessBackend& be, const GearCalc1& C, GearTxn& T)
{
    const GearPlan& P = C.plan;
    T.teachPath = be.GearTeachIniPath();
    T.motPath = be.MotTableFilePath();
    if (T.teachPath.empty()) return "沒有 teach.ini 的路徑（GearTeachIniPath）—— 不寫";
    if (T.motPath.empty()) return "沒有 Mot_Table 的路徑（MotTablePath）—— 不寫";
    if (!GearReadText(T.teachPath, T.teachOld) || T.teachOld.empty()) return "讀不到 " + T.teachPath + " —— 不寫";
    if (!GearReadText(T.motPath, T.motOld) || T.motOld.empty()) return "讀不到 " + T.motPath + " —— 不寫";
    const std::vector<MotTableSaveField> fields = MotTableGearFieldsOf(C.fit.newRatio, P.softPNew, P.softNNew);
    {
        std::string out;
        const MotTableEditResult pre = MotTableEditRow(T.motOld, C.alias, C.m.a.motIndex, fields, be.MotTableIndexForced(), C.m.a.cardModel, out);
        if (!pre.ok) return "Mot_Table 這一列不能寫：" + pre.why;
        if (pre.changedCount == 0) return "Mot_Table 這一列已經是新的值 —— 沒有東西要寫";
    }
    T.stamp = be.GearStamp();
    if (T.stamp.empty()) T.stamp = MtStamp();
    // 1. backups (teach.ini here; Mot_Table.csv inside MotTableSaveRow, the same name rule: path.bak_<stamp>)
    T.teachBak = GearUniqueBak(T.teachPath, T.stamp);
    if (T.teachBak.empty() || !MtWriteFile(T.teachBak, T.teachOld)) { if (!T.teachBak.empty()) std::remove(T.teachBak.c_str()); return "teach.ini 的備份寫不出來 —— 不寫"; }
    T.teachBakMade = true;
    // 3. Mot_Table: GearRatio, SoftLimitP, SoftLimitN of this row only (backup + temp + MoveFileEx + read back)
    T.mot = MotTableSaveRow(T.motPath, T.stamp, C.alias, C.m.a.motIndex, fields, be.MotTableIndexForced(), C.m.a.cardModel);
    if (!T.mot.ok) {
        if (T.mot.wrote) { T.motWritten = true; const std::string rw = GearRestore(be, C, T); return "Mot_Table 寫入後讀回不對：" + T.mot.why + (rw.empty() ? "；已用備份還原" : "；⚠ 還原也失敗：" + rw); }
        std::remove(T.teachBak.c_str()); T.teachBakMade = false;
        return "Mot_Table 沒寫：" + T.mot.why;
    }
    T.motWritten = T.mot.wrote;
    T.motBak = T.mot.backup;
    // 2. memory (the teach.ini writer saves the variables, so this comes first)
    T.memoryTouched = true;
    std::string w;
    if (!be.GearSetMotor(C.m.a.motIndex, C.fit.newRatio, P.softPNew, P.softNNew, w)) { const std::string rw = GearRestore(be, C, T); return "MOT 參數改不了：" + w + (rw.empty() ? "；已還原" : "；⚠ 還原失敗：" + rw); }
    {
        std::vector<std::pair<std::string, std::string> > cells;
        for (std::size_t i = 0; i < T.mot.edit.cells.size(); ++i) cells.push_back(std::make_pair(T.mot.edit.cells[i].column, T.mot.edit.cells[i].newText));
        be.MotTableRowSaved(C.alias, T.mot.edit.newRowText, cells);
    }
    if (!be.GearSetTeach(GearTeachValues(P, true), w)) { const std::string rw = GearRestore(be, C, T); return "教導點改不了：" + w + (rw.empty() ? "；已還原" : "；⚠ 還原失敗：" + rw); }
    // 4. teach.ini: the Teach page's own writer + golden btnSaveClick's tail (ReadFile, InitShuttleThreadParameter, fAllMotorHome=false)
    T.teachWritten = true;
    if (!be.GearTeachSaveReload(true, w, T.teachNote)) { const std::string rw = GearRestore(be, C, T); return "teach.ini 寫不了：" + w + (rw.empty() ? "；已還原" : "；⚠ 還原失敗：" + rw); }
    // 5. read both files back and compare
    std::string why, teachNew, motNew;
    int outside = 0;                                                            //AI(W906-GEARRATIO2) 20261003: keys outside the plan that the teach.ini diff touched (kGearTeachNotNormalized)
    if (!MtReadFile(T.teachPath, teachNew)) why = "teach.ini 讀不回來";
    else why = GearTeachDiffWhy(P, T.teachOld, teachNew, &outside);
    const bool outOfPlan = !why.empty() && outside > 0;
    if (why.empty()) {
        std::vector<GearTeachRef> now;
        std::string rw;
        if (!be.GearTeachRefs(now, rw)) why = "存檔後讀不到教導頁的登錄表：" + rw;
        else {
            std::map<const int*, int> want;
            for (std::size_t i = 0; i < C.refs.size(); ++i) want[C.refs[i].ptr] = C.refs[i].value;
            for (std::size_t i = 0; i < P.teach.size(); ++i) want[P.teach[i].ptr] = P.teach[i].newV;
            int bad = 0;
            std::string first;
            for (std::size_t i = 0; i < now.size(); ++i) {
                std::map<const int*, int>::const_iterator it = want.find(now[i].ptr);
                if (it == want.end() || it->second == now[i].value) continue;
                if (++bad == 1) first = now[i].list + "[" + std::to_string(now[i].index) + "] " + now[i].key + "=" + std::to_string(now[i].value) + "（應該是 " + std::to_string(it->second) + "）";
            }
            if (bad) why = "存檔並重讀之後有 " + std::to_string(bad) + " 個教導值不對：" + first;
        }
    }
    if (why.empty()) {
        std::string out;
        if (!MtReadFile(T.motPath, motNew)) why = "Mot_Table.csv 讀不回來";
        else {
            const MotTableEditResult chk = MotTableEditRow(motNew, C.alias, C.m.a.motIndex, fields, be.MotTableIndexForced(), C.m.a.cardModel, out);
            if (!chk.ok || chk.changedCount != 0) why = "Mot_Table.csv 讀回來不是新的值" + (chk.ok ? std::string() : "：" + chk.why);
        }
    }
    if (why.empty()) {
        MotorGolden g;
        if (!be.GoldenMotor(C.m.a.motIndex, g) || g.gearRatio != C.fit.newRatio || g.softP != P.softPNew || g.softN != P.softNNew)
            why = "MOT 的齒輪比／軟體極限讀回來不是新的值";
    }
    if (!why.empty()) {
        const std::string rw = GearRestore(be, C, T);
        if (outOfPlan && rw.empty()) return std::string(kGearTeachNotNormalized) + why;   //AI(W906-GEARRATIO2) 20261003: the actionable text first, the detail list after it (only when the restore really worked)
        return why + (rw.empty() ? "；兩個檔與記憶體都已用備份還原" : "；⚠ 還原也失敗：" + rw + "（備份 " + T.teachBak + "、" + T.motBak + "）");
    }
    return std::string();
}

std::string GearCsvField(const std::string& s)
{
    if (s.find_first_of(",\"\r\n") == std::string::npos) return s;
    std::string q = "\"";
    for (std::size_t i = 0; i < s.size(); ++i) { if (s[i] == '"') q += '"'; q += s[i]; }
    return q + "\"";
}
// spec §5.3 step 7: <W906_OPLOG_DIR>\GearRatioCal.csv, one line per save (header on a new file). "" + where = written.  AI(W906-GEARLOGDIR) 20261003: W906_OPLOG_DIR not set -> <as9045LogPath>\GearRatioCal.csv (s0 #71 (4) A).
std::string GearLog(IMotorAccessBackend& be, const GearCalc1& C, const GearTxn& T, std::string& where)
{
    where.clear();
    const std::string dir = be.GearCalLogDir();
    if (dir.empty()) return "沒有設 W906_OPLOG_DIR，機台紀錄根目錄也是空的：校正紀錄沒寫檔（內容在這個 ack 與主控台）";   //AI(W906-GEARLOGDIR) 20261003: the live dir falls back to the machine log root (GearRatioBackend.h EOF GearCalLogDirOf), so "" now means both are empty
    LsForceDir(dir);
    const std::string path = dir + (dir[dir.size() - 1] == '\\' || dir[dir.size() - 1] == '/' ? "" : "\\") + "GearRatioCal.csv";
    std::string meas;
    if (C.hand) { char hb[96]; std::snprintf(hb, sizeof(hb), "hand E0=%.0f E1=%.0f;", C.handE0, C.handE1); meas = hb; }   //AI(W906-GEARHAND) 20261003: a hand session's line says so, with its encoder readings
    for (std::size_t i = 0; i < C.meas.size(); ++i) {
        char b[96];
        std::snprintf(b, sizeof(b), "%s%.4f:%.3f:%+d", i ? ";" : "", C.meas[i].c, C.meas[i].a, C.meas[i].dir);
        meas += b;
    }
    const std::vector<std::string> cols = {
        NowIso(), be.GearOperator(), C.alias, std::to_string(C.m.a.motIndex), GearNum(C.oldRatio, 17), GearNum(C.fit.newRatio, 17),
        GearNum(C.fit.k, 10), GearNum(C.fit.consistency * 100.0, 6), C.fit.hasBacklash ? GearNum(C.fit.backlash, 6) : std::string(),
        meas, std::to_string(C.plan.changedCount),
        std::to_string(C.plan.in.softP) + ">" + std::to_string(C.plan.softPNew), std::to_string(C.plan.in.softN) + ">" + std::to_string(C.plan.softNNew),
        T.motBak, T.teachBak, std::to_string(g_gear.id) };
    std::string line;
    for (std::size_t i = 0; i < cols.size(); ++i) line += (i ? "," : "") + GearCsvField(cols[i]);
    const bool fresh = !MtExists(path);
    std::FILE* f = std::fopen(path.c_str(), "ab");
    if (!f) return "校正紀錄 " + path + " 打不開 —— 沒寫";
    if (fresh) std::fputs("time,operator,motor,mi,oldRatio,newRatio,k,consistencyPct,backlashMm,measurements(c:a:dir),teachChanged,softP,softN,motTableBackup,teachBackup,session\r\n", f);
    std::fputs((line + "\r\n").c_str(), f);
    const bool ok = std::fclose(f) == 0;
    if (!ok) return "校正紀錄 " + path + " 寫入失敗";
    where = path;
    return std::string();
}

MotorAccessOutcome DoGearPreviewSave(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be, bool save)
{
    (void)wireId;
    GearCalc1 C;
    std::string why;
    if (!GearCompute(r, be, C, why)) return Refuse(why);
    const std::string blocked = GearSaveBlockedWhy(C);
    if (!save) {
        webbridge::JsonWriter w;
        w.BeginObject();
        AckHead(w, r, "preview", "none", blocked.empty() ? "預覽（沒有寫任何檔、沒有改記憶體）：可以存檔" : "預覽（沒有寫任何檔）：不能存檔 —— " + blocked);
        w.Key("motIndex").Number((wb_int64)C.m.a.motIndex);
        GearWriteFit(w, C.fit);
        GearWritePlan(w, C);
        w.Key("sessionMatched").Bool(C.sessionMatched);
        w.Key("sessionWhy").String(C.sessionWhy);
        w.Key("teachWindowOpen").Bool(C.teachOpen);
        w.Key("canSave").Bool(blocked.empty());
        w.Key("saveWhy").String(blocked);
        w.Key("needConfirm").Bool(C.fit.level == "confirm");
        GearWriteHand(w, C);                                                    //AI(W906-GEARHAND) 20261003
        w.Key("session"); GearWriteSession(w, be);
        MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
    }
    const std::string who = C.who;
    if (!blocked.empty()) return Refuse(who + ": " + blocked);
    { const std::string busy = JobsBusyWhy(r.action, r, be); if (!busy.empty()) return Refuse(busy); }
    for (std::size_t i = 0; i < g_jogs.size(); ++i)
        if (g_jogs[i].is1203 && g_jogs[i].axis == C.m.a.axis) return Refuse(who + ": 這一軸正在 JOG —— 先放開");
    if (!C.hand && C.m.g.homeFlag != 1) return Refuse(who + ": 這一軸 HomeFlag=" + std::to_string(C.m.g.homeFlag) + "（量測之後回原點狀態掉了）—— 重新量");   //AI(W906-GEARHAND) 20261003: a move session's rule (its points need the homed axis); a hand session reads the encoder, HomeFlag is a move gate there
    std::map<std::string, std::string>::const_iterator pk = r.str.find("previewKey");
    if (pk == r.str.end() || pk->second != C.plan.key)
        return Refuse(who + ": 預覽之後資料變了（或沒有先預覽）：previewKey " + (pk == r.str.end() ? std::string("（沒給）") : pk->second) + " ≠ " + C.plan.key + " —— 請重新預覽，看過再存");
    std::map<std::string, bool>::const_iterator cf = r.flag.find("confirmLarge");
    if (C.fit.level == "confirm" && !(cf != r.flag.end() && cf->second))
        return Refuse(who + ": " + C.fit.why + "（要再確認一次：confirmLarge=true）");
    GearTxn T;
    const std::string tw = GearSaveTxn(be, C, T);
    if (!tw.empty()) {
        std::printf("motor.access gearRatioSave %s: NOT saved -- %s\n", C.alias.c_str(), tw.c_str());
        return Refuse(who + ": " + tw);
    }
    // 6. the session is used up; this axis must home again (HomeFlag=0 on THIS axis only -- no axis is zeroed, no other flag moves)
    GearEnd("已存檔（齒輪比 " + GearNum(C.oldRatio) + " → " + GearNum(C.fit.newRatio, 7) + "）");
    CancelJobsOnAxis(be, C.alias, C.m.a, "gear ratio saved");
    be.GoldenSetHomeFlag(C.m.a.motIndex, 0);
    // 7. the calibration log
    std::string logAt;
    const std::string logWhy = GearLog(be, C, T, logAt);
    const std::string msg = "已存檔：" + C.alias + " 齒輪比 " + GearNum(C.oldRatio) + " → " + GearNum(C.fit.newRatio, 7) + "（k=" + GearNum(C.fit.k, 7) +
                            "）；教導點 " + std::to_string(C.plan.changedCount) + " 個換算；Mot_Table 備份 " + T.motBak + "、teach.ini 備份 " + T.teachBak +
                            (C.hand ? "；" + GearHandRestartText(C.m.a.axis) :   //AI(W906-GEARHAND) 20261003: spec (9) -- restart wb_serve before this axis moves
                            "；這一軸 HomeFlag=0 —— 請重新回原點（HOME），" + GearVerifyHint(C.m.g));   //AI(W906-GEARRATIO2) 20261003: R171 L5
    std::printf("motor.access gearRatioSave: %s\n", msg.c_str());
    webbridge::JsonWriter w;
    w.BeginObject();
    AckHead(w, r, "saved", "Mot_Table+teach.ini", msg);
    w.Key("motIndex").Number((wb_int64)C.m.a.motIndex);
    w.Key("gearRatio").BeginObject(); w.Key("old").Number(C.oldRatio); w.Key("new").Number(C.fit.newRatio); w.Key("newText").String(GearNum(C.fit.newRatio, 7)); w.EndObject();
    GearWriteFit(w, C.fit);
    GearWritePlan(w, C);
    w.Key("files").BeginObject();
    w.Key("motTable").String(T.motPath); w.Key("motTableBackup").String(T.motBak);
    w.Key("teach").String(T.teachPath); w.Key("teachBackup").String(T.teachBak);
    w.Key("changes").BeginArray();
    for (std::size_t i = 0; i < T.mot.edit.cells.size(); ++i) {
        const MotTableCellChange& c = T.mot.edit.cells[i];
        if (!c.changed) continue;
        w.BeginObject(); w.Key("column").String(c.column); w.Key("old").String(c.oldText); w.Key("new").String(c.newText); w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    w.Key("teachWriterNote").String(T.teachNote);
    w.Key("homeFlag").Number((wb_int64)0);
    w.Key("log"); if (logAt.empty()) w.Null(); else w.String(logAt);
    w.Key("logWhy").String(logWhy);
    w.Key("next").String(C.hand && g_encBase.count(C.m.a.axis) == 0 ? std::string("回原點（HOME）；回原點之後再用手推模式量一次驗證；抽查一個教導點，位置應該跟換算前一樣") : C.hand ? std::string("先重開 wb_serve（這一軸手推過，W5B-6），再回原點（HOME）；重開之後再用手推模式量一次驗證；抽查一個教導點，位置應該跟換算前一樣")   //AI(W906-GEARHAND) 20261003  AI(W906-SVON-ENCSYNC) 20261004: synced at Servo ON (off g_encBase) -> HOME, no restart
                                : "請重新回原點（HOME），" + GearVerifyHint(C.m.g) + "；抽查一個教導點，位置應該跟換算前一樣");   //AI(W906-GEARRATIO2) 20261003: R171 L5
    w.Key("mode").String(C.hand ? "hand" : "move");                              //AI(W906-GEARHAND) 20261003
    w.Key("restartWbServe").Bool(C.hand);
    WriteCurOf(w, be, C.m.a.motIndex);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

MotorAccessOutcome DoGear(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (r.source != "uMotorTest")
        return Refuse("motor.access " + r.action + "（" + r.source + "）: 這是 Motor Test 頁 Gear Ratio 分頁的動作（RULINGS_20261002 第 22 條），教導頁沒有");
    if (r.action == "gearCalMove") return DoGearCalMove(r, wireId, be);
    if (r.action == "gearHandBegin") return DoGearHandBegin(r, wireId, be);   //AI(W906-GEARHAND) 20261003
    return DoGearPreviewSave(r, wireId, be, r.action == "gearRatioSave");
}

}  // namespace

MotorAccessGearCalState MotorAccessGearCal()
{
    MotorAccessGearCalState s;
    s.active = g_gear.active; s.id = g_gear.id; s.motor = g_gear.motor; s.mi = g_gear.mi; s.axis = g_gear.axis; s.dirSign = g_gear.dirSign;
    s.ratio = g_gear.ratio; s.zeroUser = g_gear.zeroUser; s.lastTargetUser = g_gear.lastTargetUser; s.arriveCmdPos = g_gear.arriveCmdPos;
    s.zeroCard = g_gear.zeroCard; s.lastTargetCard = g_gear.lastTargetCard; s.placeholder = g_gear.placeholder; s.why = g_gear.why;
    for (std::size_t i = 0; i < g_gear.points.size(); ++i) { s.cNom.push_back(g_gear.points[i].cNom); s.cTrue.push_back(g_gear.points[i].cTrue); s.dir.push_back(g_gear.points[i].dir); }
    s.hand = g_gear.hand; s.handPhase = g_gear.handPhase; s.e0Card = g_gear.e0Card; s.e1Card = g_gear.e1Card; s.pushes = g_gear.pushes;   //AI(W906-GEARHAND) 20261003
    return s;
}

std::string MotorAccessGearCalJson(IMotorAccessBackend& be, const std::vector<std::string>& aliases)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("session"); GearWriteSession(w, be);
    w.Key("axes").BeginArray();
    for (std::size_t i = 0; i < aliases.size(); ++i) {
        MotorAccessAxis a;
        MotorGolden g;
        if (!be.Resolve(aliases[i], a)) continue;
        const bool hasG = a.motIndex >= 0 && be.GoldenMotor(a.motIndex, g);
        std::string why = hasG ? GearStaticWhy(aliases[i], a, g) : aliases[i] + "：沒有 golden 馬達物件";
        if (why.empty()) why = GearEncBaseWhy(aliases[i], a);
        if (why.empty() && a.axis < 0) why = aliases[i] + "：" + a.why;
        w.BeginObject();
        w.Key("motor").String(aliases[i]);
        w.Key("mi").Number((wb_int64)a.motIndex);
        w.Key("eligible").Bool(why.empty());
        w.Key("kind").String(hasG ? GearKindOf(aliases[i], g) : std::string());
        w.Key("why").String(why);
        w.Key("gearRatio"); if (hasG) w.Number(g.gearRatio); else w.Null();
        w.Key("softP"); if (hasG) w.Number((wb_int64)g.softP); else w.Null();
        w.Key("softN"); if (hasG) w.Number((wb_int64)g.softN); else w.Null();
        w.Key("placeholder").Bool(hasG && GearPlaceholderLimits(g));
        w.Key("homeFlag"); if (hasG) w.Number((wb_int64)g.homeFlag); else w.Null();
        {   //AI(W906-GEARHAND) 20261003: the hand mode's own eligibility (no Z; the encoder base and the other move gates do not apply)
            std::string hw = hasG ? GearHandStaticWhy(aliases[i], a, g) : aliases[i] + "：沒有 golden 馬達物件";
            if (hw.empty() && a.axis < 0) hw = aliases[i] + "：" + a.why;
            w.Key("handEligible").Bool(hw.empty());
            w.Key("handWhy").String(hw);
        }
        w.EndObject();
    }
    w.EndArray();
    w.Key("limits").BeginObject();
    w.Key("takeupMaxMm").Number(kGearTakeupMaxMm); w.Key("stepMaxMm").Number(kGearStepMaxMm); w.Key("spanMaxMm").Number(kGearSpanMaxMm);
    w.Key("speedMaxPct").Number((wb_int64)kGearSpeedMaxPct);
    w.Key("handRulerMaxMm").Number(kGearHandRulerMaxMm);                         //AI(W906-GEARHAND) 20261003
    w.EndObject();
    const GearFitLimits L;
    w.Key("fitLimits").BeginObject();
    w.Key("minForward").Number((wb_int64)L.minForward); w.Key("minSpanMm").Number(L.minSpanMm); w.Key("maxSpreadPct").Number(L.maxSpread * 100.0);
    w.Key("okBandPct").Number(L.okBand * 100.0); w.Key("confirmBandPct").Number(L.confirmBand * 100.0);
    w.EndObject();
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"error\":\"json writer misuse\"}");
}

}  // namespace ht9045

// =============================================================================
//  AI(W906-TEACH-ZDOWN) 20261003: INBOX 147 -- three golden uteach.cpp Teach buttons the page still lacked (contract and the
//  live ops: WebTeachZDown.h).  golden = 906_20260618 (RULINGS_20261001 #0).  One block at the end of the file (this file's
//  line numbers are cited elsewhere); the hooks above are same-line additions: the two kActions rows (on the teachGo row) and
//  one dispatch line (after the uteach block, on the Arm Cell / Gear Ratio line -- every source reaches it; Motor Test is
//  refused in TeachZDownDispatch).  A withdrawn earlier attempt (laptop batch 39, f41916a2 / d2ef0dab, reverted by f511186c)
//  was reviewed by NB2 R144 / R151; what that review found is answered below (R144 #2, #3, #4, #7, #8; R169 M2).
//
//  teachSetAllArmZ  golden btnSetAllInArmZClick :4300-4365 / btnSetAllOutArmZClick :4367-4435
//    (X = InArmSuck / OutArmSuck, Z = InArmZIndex / OutArmZIndex, E = InZEditPtr / OutZEditPtr = teInArm / teOutArm :286-290,
//     base = [iIn/OutArmYBase][iIn/OutArmXBase]):
//      USE_PICKER_COUNT==0 (ep4Picker, :4312 / :4381):
//          for(i<X.iMaxRow) for(j<X.iMaxCol)  E[i][j] = (i==0 || i==2) ? MOT[Z[i][i]].ReadPos() : 0;      <- [i][i] sic (:4319 / :4388)
//          stand = atoi(E[base]);
//          for(i<X.iMaxRow) for(j<X.iMaxCol)  if((i,j)!=base) E[i][j] = (i==0 || i==2) ? atoi(E[i][j])-stand : 0;   (the base keeps its ReadPos)
//      otherwise:
//          for(...)  E[i][j] = MOT[Z[i][j]].ReadPos();   stand = atoi(E[base]);   for(...) E[i][j] = atoi(E[i][j])-stand;   E[base] = 0;
//    The base can lie OUTSIDE the grid (ep1Picker: base [0][2], grid 1x1 -- golden database.cpp:801-806, cinitial.cpp:5561-5570):
//    golden then takes stand from that edit's SCREEN text (setEditZ1E / setEditZ2E) and the non-ep4 branch writes 0 into it,
//    so the page sends that field (params.fields, TeachFieldValue) and the ack writes it (NB2 R144 #2: batch 39 refused instead).
//    MOVES NOTHING, WRITES NO FILE: the ack carries every edit golden writes ("edits", golden order); the page fills them
//    (data-src cpp-pos) and the operator saves with the page's Save, as golden's TEdits are saved.
//    ReadPos per nozzle: a PCI1203 row with Mot_Table Enable=1 -> the 1203 monitor (CurrentUserPos: cmdPos, the encoder after a
//    hand teach, W5B-6) on a FRESH sample; every other motor -- a PCI1203 row with Enable=0 (NB2 R144 #7), a motor this Mot_Table
//    does not list or that has no object (R144 #8: batch 39 refused), another card -- golden's own MOT[i].ReadPos()
//    (TMyMotor::ReadPos returns the cached Position when Motor==NULL or !Enable, golden Motor/mymotor.cpp:417-432).
//    One unreadable 1203 nozzle -> refused, no edit written.
//
//  teachOutZAllDown  golden btnOutZAllDownClick :4542-4562:
//      for(i<OutArmSuck.iMotRow) for(j<iMotCol) { MOT[OutArmZIndex[i][j]].SetSpeed(10); Pos[i][j]=atoi(SetEditPickOutSht)+atoi(OutZEditPtr[i][j]); }
//      for(i<OutArmSuck.iMotRow) for(j<iMotCol)   MOT[OutArmZIndex[i][j]].MotorMove(Pos[i][j]);
//    golden has no CheckCanMove / IsCanQuickJogMove / HomeFlag / motor-power test here -- none is added.  The SetSpeed(10) loop
//    runs for every Z before any move, whatever the moves then do (NB2 R144 #3).  SetSpeed(10): a PCI1203 row with Enable=1 ->
//    the PTP family on the card (Send1203Speed, as Move1203's); an Enable=0 1203 row -> the recorded percentage only (golden
//    TMyMotor::SetSpeed `speed=p`); another card -> the golden object's SetSpeed(10).  MotorMove: a PCI1203 row -> GoldenMove1203
//    (this file: door -1, lock, Enable=0, soft limits -2 / -3, MotionDone, CompareCommandPos, MoveToPos, and golden's fCMD /
//    iOldPos -- a second press with the same target is golden's "already arrived", nothing sent: NB2 R144 #4); another card ->
//    the golden object's own MotorMove (no InitMOTParameter: golden's button has none, unlike the Teach Go path).  The ack lists
//    every Z (target, speed, golden's return, what happened).  There is no job: golden calls MotorMove once per axis and the card
//    finishes the move.  STOP and closing Teach stop the Zs (DoStop / the Teach close edge = golden btnStop->Click():
//    StopAllMotor); the safe lock, an alarm, a message box and the operator gone do what they do to any other one-shot Teach
//    move (RULINGS_20261002 #1 / s0 #46 = A: those end Teach multi-axis JOBS; this press leaves none).
//
//  Gates -- each refuses before anything is touched (no speed, no move, no edit):
//    dispatcher (as every uteach motion): SystemStart, a hand teach open (golden fTeachShow is ShowModal), an Arm Cell job.
//    TeachZDownDispatch: a Motor Test source (golden uteach buttons); the ops not installed (WebTeachZDownLive.cpp, wb_serve only);
//      a grid / base outside golden's [2][8] tables (golden would index past InZEditPtr / InArmZIndex / Pos [2][8]).
//    [W906] NB2 R169 M2 (the per-axis-lock gap R169 found in St02's Set All Z Move).  golden: these buttons sit under PageControl2,
//      which VerifyMotorAction (uteach.cpp:5346-5405 -> LockAllButton :5321-5329) disables while ANY motor moves; the port's Teach
//      lock is per axis (AI(W906-MT-AXISLOCK) / TEACH-LOCK: PageControl2 is never disabled), so each press checks the axes itself,
//      one test per axis (ZdAxisBusy): a jog / HOME / LoopMove / Arm Cell / Gear Ratio on it, a 1203 axis not READY or commanded
//      without a fresh sample, a golden-object motor still moving (golden VerifyMotorAction's per-motor test, ITeachZDownOps).
//      Set InArm Z / Set OutArm Z (they only read positions): THAT arm -- X, Y, the pitch axes and every Z the press reads.
//      Out Z All Down (it drives the Out Zs down): EVERY motor of the machine -- golden VerifyMotorAction's for(i<TOTAL_MOTOR),
//        ITeachZDownOps::MotorCount (unknown -> refused).  AI(W906-TEACH-ZDOWN) 20261003: decision 1 = B, the coordinator's safe
//        default while the user is away (was THAT arm, as Set All): a shuttle moving under the Out arm is golden's collision case.
//        Not golden's 2 s lock after the last motion (MoveDelay): a press right after a motion ended goes.
//    [W906] W5B-4: the screen values golden reads with atoi (SetEditPickOutSht, OutZEditPtr[i][j]; the base edit of a Set All whose
//      base is off the grid) come in params.fields, loaded from C++ or typed (TeachFieldValue) -- golden's atoi takes a blank as 0.
//    teachOutZAllDown: a target outside int (golden's int addition would overflow).
//  Not reproduced: (1) the port's other Teach moves (Move1203: Go / MoveTo / MoveP / MoveN, St02's Set All Z Move, Arm Cell) do
//    not keep GoldenMove1203's fCMD / iOldPos the way golden's MotorMove / InitMOTParameter keep MOT[i]'s -- after such a move on
//    an Out Z, the next All Down to the old target can be golden's "already arrived" once (nothing sent, said in the ack; the
//    press after it moves).  (2) an Enable=0 PCI1203 Z: golden MotorMovePosition writes Position=Tar (mymotor.cpp:821-823);
//    GoldenMove1203 returns 1 without it.  (3) a golden-object motor's motion is seen only through ITeachZDownOps::GoldenMotionDone
//    (the HT9050 arms are PCI1203; their MN200 Zs are Enable=0).
// =============================================================================
#include "WebTeachZDown.h"

namespace ht9045 {
namespace {

ITeachZDownOps* g_zdOps = 0;                       // WebTeachZDownLive.cpp installs it (wb_serve); 0 = both actions refused

// golden FormCreate teInArm / teOutArm (uteach.cpp:286-290) = InZEditPtr / OutZEditPtr; [0 In / 1 Out][i][j]
const char* const kZdEdit[2][2][8] = {
    { { "setEditZ1A", "setEditZ1C", "setEditZ1E", "setEditZ1G", "setEditZ1I", "setEditZ1K", "setEditZ1M", "setEditZ1O" },
      { "setEditZ1B", "setEditZ1D", "setEditZ1F", "setEditZ1H", "setEditZ1J", "setEditZ1L", "setEditZ1N", "setEditZ1P" } },
    { { "setEditZ2A", "setEditZ2C", "setEditZ2E", "setEditZ2G", "setEditZ2I", "setEditZ2K", "setEditZ2M", "setEditZ2O" },
      { "setEditZ2B", "setEditZ2D", "setEditZ2F", "setEditZ2H", "setEditZ2J", "setEditZ2L", "setEditZ2N", "setEditZ2P" } } };

// AckHead's keys in AckHead's order with the state chosen: a press that refused some axes answers "error" (motor-access.js
//   finish() takes ack.state), so a partial result is not shown green (the NB2 R19 RW4-2 rule, as St02's).
void ZdHead(webbridge::JsonWriter& w, const MotorAccessReq& r, const char* state, const std::string& result, const std::string& message)
{
    w.Key("seq").Number((wb_int64)r.seq);
    w.Key("reqId").String(r.id);                                                // never "id" (AckHead: WebBridgeServer::AckJson flattens the ack)
    w.Key("state").String(state);
    w.Key("result").String(result);
    w.Key("message").String(message);
    w.Key("source").String(r.source);
    w.Key("button").String(r.button);
    w.Key("action").String(r.action);
    w.Key("motorId");
    if (r.motors.empty()) w.Null(); else w.String(r.motors[0]);
    w.Key("position").Null();
    w.Key("layer").String("uteach");
    w.Key("completedAt").String(NowIso());
}

// golden MOT[mi] -> its Mot_Table row (false = this Mot_Table does not list it)
bool ZdRow(IMotorAccessBackend& be, int mi, std::string& alias, MotorAccessAxis& a)
{
    alias.clear();
    a = MotorAccessAxis();
    return mi >= 0 && be.AliasOfMotIndex(mi, alias) && be.Resolve(alias, a) && a.motIndex >= 0;
}
std::string ZdName(int mi, const std::string& alias) { return alias.empty() ? "MOT[" + std::to_string(mi) + "]" : alias; }

// [W906] NB2 R169 M2: is this motor busy (an arm's motor for Set All; every motor of the machine for All Down, decision 1 = B)?  "" = idle.  Only what this tree can see moving it.
std::string ZdAxisBusy(IMotorAccessBackend& be, int mi)
{
    std::string alias;
    MotorAccessAxis a;
    if (!ZdRow(be, mi, alias, a)) return std::string();                         // not in this Mot_Table: nothing of this tree drives it
    for (std::size_t i = 0; i < g_jogs.size(); ++i)
        if (g_jogs[i].is1203 ? (a.Is1203() && a.axis >= 0 && g_jogs[i].axis == a.axis) : (g_jogs[i].mi == a.motIndex))
            return alias + " JOG 中";
    for (std::size_t i = 0; i < g_homes.size(); ++i)
        if (HomeOnAxis(g_homes[i], alias, a)) return alias + " HOME 進行中（" + g_homes[i].motorId + "）";
    if (LoopOnAxis(alias, a)) return alias + " LoopMove 進行中";
    if (ArmCellActive() && ArmCellOnAxis(alias, a)) return alias + " 是 Arm Cell 正在用的軸";
    if (GearActive() && GearOnAxis(alias, a)) return alias + " Gear Ratio 量測中";
    if (!a.Is1203()) {
        const int d = g_zdOps ? g_zdOps->GoldenMotionDone(a.motIndex) : -1;     // golden VerifyMotorAction :5382-5384
        return d == 0 ? alias + " 還在動（golden MotionDone()==false）" : std::string();
    }
    std::string w;
    if (!a.tableEnable || a.axis < 0 || !be.Pci1203Ready(w)) return std::string();   // nothing of this tree can command it
    if (SampleStale(be, a.axis)) return alias + " 剛下過命令、監看器還沒更新（輪詢 200 ms）";
    unsigned st = 0;
    if (be.Pci1203AxisState(a.axis, st) && (st & 0xFFu) != 0u && !IsReadyState(st) && !IsErrorState(st))
        return alias + " 還在動（1203 狀態 " + std::to_string(st & 0xFFu) + "）";
    return std::string();
}
// the busy ones of these motors (each asked once, in this order); at most six named, then how many in all
std::string ZdBusyOf(IMotorAccessBackend& be, const std::vector<int>& mis)
{
    std::set<int> seen;
    std::vector<std::string> busy;
    std::size_t n = 0;
    for (std::size_t k = 0; k < mis.size(); ++k) {
        if (!seen.insert(mis[k]).second) continue;
        const std::string s = ZdAxisBusy(be, mis[k]);
        if (s.empty()) continue;
        if (++n <= 6) busy.push_back(s);
    }
    if (n > 6) busy.push_back("… 共 " + std::to_string(n) + " 軸");
    return JoinList(busy);
}
std::string ZdArmBusy(IMotorAccessBackend& be, const TeachZArmGrid& g, const std::vector<int>& zs)
{
    std::vector<int> all = g.armAxes;
    all.insert(all.end(), zs.begin(), zs.end());
    return ZdBusyOf(be, all);
}
//AI(W906-TEACH-ZDOWN) 20261003: decision 1 = B (the banner) -- Out Z All Down: the arm and its Zs first (so they are named
//  first), then every motor golden VerifyMotorAction walks (MOT[0 .. TOTAL_MOTOR-1]).  The same per-axis test as the arm gate.
std::string ZdMachineBusy(IMotorAccessBackend& be, const TeachZArmGrid& g, const std::vector<int>& zs, int motorCount)
{
    std::vector<int> all = g.armAxes;
    all.insert(all.end(), zs.begin(), zs.end());
    for (int mi = 0; mi < motorCount; ++mi) all.push_back(mi);
    return ZdBusyOf(be, all);
}
std::string ZdBusyWhy(const std::string& busy)
{
    return "[W906] 這支手臂有軸在動或有工作在跑 —— " + busy + "（golden 這顆鈕在 PageControl2 底下，VerifyMotorAction（uteach.cpp:5346-5405）"
           "在任何馬達動的時候整頁鎖住；移植樹的教導頁鎖是每軸各自鎖，所以按下時檢查這支手臂，NB2 R169 M2）";
}
std::string ZdMachineBusyWhy(const std::string& busy)
{
    return "[W906] 機台上有軸在動或有工作在跑 —— " + busy + "（golden 這顆鈕在 PageControl2 底下，VerifyMotorAction（uteach.cpp:5346-5405）"
           "在任何馬達動的時候整頁鎖住；All Down 會把 Out Z 往下壓，所以檢查整台機台的每一軸，例如出料飛梭在手臂底下移動時不放行"
           "（裁決 1＝B，20261003；Set InArm Z／Set OutArm Z 只讀位置，仍只看那支手臂，NB2 R169 M2））";
}

// golden MOT[mi].ReadPos() for one nozzle (see the banner).  false + why = cannot read (the press writes nothing).
bool ZdReadPos(IMotorAccessBackend& be, int mi, int& pos, std::string& src, std::string& why)
{
    std::string alias;
    MotorAccessAxis a;
    const bool row = ZdRow(be, mi, alias, a);
    if (row && a.Is1203() && a.tableEnable) {
        std::string w;
        if (!be.Pci1203Ready(w)) { why = alias + "（PCI1203）: 1203 控制層不可用 —— " + w; return false; }
        if (a.axis < 0) { why = alias + "（PCI1203）: " + a.why; return false; }
        MotionCtx m;
        m.a = a;
        if (!be.GoldenMotor(a.motIndex, m.g)) { why = alias + ": 沒有 golden 馬達物件（MOT[].Motor 為 NULL）"; return false; }
        if (SampleStale(be, a.axis)) { why = alias + ": " + kStaleWhy; return false; }
        if (!CurrentUserPos(be, m, pos)) { why = "讀不到 " + alias + " 的目前位置（監看器沒有樣本）"; return false; }
        src = MotorAccessEncoderBase(a.axis) ? "pci1203 actPos" : "pci1203 cmdPos";
        return true;
    }
    pos = be.GoldenReadPos(mi);                                                 // golden TMyMotor::ReadPos (cached Position when absent / disabled)
    src = !row ? "MOT（Mot_Table 沒有這一列）" : (a.Is1203() ? "MOT（PCI1203 Enable=0）" : "MOT（" + a.cardModel + "）");
    return true;
}

// ---- Set InArm Z / Set OutArm Z: golden btnSetAllInArmZClick :4300-4365 / btnSetAllOutArmZClick :4367-4435 ----
MotorAccessOutcome ZdSetAll(const MotorAccessReq& r, IMotorAccessBackend& be)
{
    const bool in = (r.button == "btnSetAllInArmZ");
    if (!in && r.button != "btnSetAllOutArmZ")
        return Refuse("teachSetAllArmZ: 按鈕 '" + r.button + "' 不是 golden 的 btnSetAllInArmZ／btnSetAllOutArmZ");
    const std::string who = in ? "Set InArm Z（golden btnSetAllInArmZClick :4300）" : "Set OutArm Z（golden btnSetAllOutArmZClick :4367）";
    if (g_zdOps == 0) return Refuse(who + ": 教導頁 Z 鈕的 golden 介面沒有安裝（WebTeachZDownLive.cpp 只連進 wb_serve）—— 一格都沒寫");
    TeachZArmGrid g;
    std::string why;
    if (!g_zdOps->ArmGrid(in, g, why)) return Refuse(who + ": " + why + "（一格都沒寫）");
    if (g.maxRow < 0 || g.maxRow > 2 || g.maxCol < 0 || g.maxCol > 8 || g.yBase < 0 || g.yBase > 1 || g.xBase < 0 || g.xBase > 7) {
        char b[320];
        std::snprintf(b, sizeof(b), "%s.iMaxRow=%d／iMaxCol=%d、基準 [%d][%d] 超出 golden 的 [2][8]（%s／%s）—— golden 會讀寫表外，一格都沒寫",
                      in ? "InArmSuck" : "OutArmSuck", g.maxRow, g.maxCol, g.yBase, g.xBase,
                      in ? "InZEditPtr" : "OutZEditPtr", in ? "InArmZIndex" : "OutArmZIndex");
        return Refuse(who + ": " + b);
    }
    const int arm = in ? 0 : 1;
    const bool ep4 = (g.pickerCount == kZdPicker4);                             // golden :4312 / :4381 USE_PICKER_COUNT==0
    std::vector<int> zs;                                                        // the motors golden reads (for the busy gate)
    for (int i = 0; i < g.maxRow; ++i)
        for (int j = 0; j < g.maxCol; ++j)
            if (!ep4 || i == 0 || i == 2) zs.push_back(ep4 ? g.zIndex[i][i] : g.zIndex[i][j]);
    { const std::string busy = ZdArmBusy(be, g, zs); if (!busy.empty()) return Refuse(who + ": " + ZdBusyWhy(busy) + "。一格都沒寫"); }
    long long E[2][8] = { { 0 } };
    std::vector<std::string> src;
    for (int i = 0; i < g.maxRow; ++i)
        for (int j = 0; j < g.maxCol; ++j) {
            if (ep4 && !(i == 0 || i == 2)) { E[i][j] = 0; continue; }          // :4320-4321 / :4389-4390
            const int mi = ep4 ? g.zIndex[i][i] : g.zIndex[i][j];             // :4319 / :4388 [i][i] sic; :4331 / :4400 [i][j]
            int pos = 0;
            std::string s;
            if (!ZdReadPos(be, mi, pos, s, why)) return Refuse(who + ": " + why + "（一格都沒寫）");
            E[i][j] = pos;
            src.push_back(std::string(kZdEdit[arm][i][j]) + "=" + std::to_string(pos) + " " + s);
        }
    const bool baseIn = g.yBase < g.maxRow && g.xBase < g.maxCol;
    const std::string baseEdit = kZdEdit[arm][g.yBase][g.xBase];
    long long stand = 0;
    if (baseIn) {
        stand = E[g.yBase][g.xBase];                                            // :4335 / :4405 atoi(the text the first loop wrote)
    } else {
        int v = 0;
        if (!TeachFieldValue(r, baseEdit, v, why))
            return Refuse(who + ": 基準吸嘴 [" + std::to_string(g.yBase) + "][" + std::to_string(g.xBase) + "] 在 iMaxRow×iMaxCol（" +
                          std::to_string(g.maxRow) + "×" + std::to_string(g.maxCol) + "）之外，golden 拿畫面上 " + baseEdit +
                          " 的字當基準（NB2 R144 #2）—— " + why + "（一格都沒寫）");
        stand = v;
    }
    for (int i = 0; i < g.maxRow; ++i)
        for (int j = 0; j < g.maxCol; ++j) {
            if (ep4) {
                if (i == g.yBase && j == g.xBase) continue;                    // :4343-4344 / :4413-4414 (the base keeps its ReadPos)
                E[i][j] = (i == 0 || i == 2) ? E[i][j] - stand : 0;           // :4345-4349 / :4415-4419
            } else {
                E[i][j] -= stand;                                               // :4359-4360 / :4429-4430
            }
            if (E[i][j] > 2147483647LL || E[i][j] < -2147483647LL - 1)
                return Refuse(who + ": " + kZdEdit[arm][i][j] + " 減基準後超出 int 範圍（一格都沒寫）");
        }
    webbridge::JsonWriter w;
    w.BeginObject();
    ZdHead(w, r, "done", "edits",
           std::string("golden ") + (in ? "btnSetAllInArmZClick" : "btnSetAllOutArmZClick") + "：各吸嘴 Z 讀目前位置（ReadPos）再減基準 " + baseEdit +
           "（" + std::to_string(stand) + (baseIn ? "" : "，畫面上的字") + "）—— 只填畫面欄位，沒有移動；按存檔才寫進教導檔");
    w.Key("arm").String(in ? "in" : "out");
    w.Key("edits").BeginObject();
    for (int i = 0; i < g.maxRow; ++i)
        for (int j = 0; j < g.maxCol; ++j) w.Key(kZdEdit[arm][i][j]).Number((wb_int64)E[i][j]);
    if (!ep4 && !baseIn) w.Key(baseEdit.c_str()).Number((wb_int64)0);         // :4363 / :4433 -- golden writes 0 into it even off the grid
    w.EndObject();
    w.Key("standZ").Number((wb_int64)stand);
    w.Key("base").String(baseEdit);
    w.Key("baseInGrid").Bool(baseIn);
    w.Key("branch").String(ep4 ? "USE_PICKER_COUNT==0" : "other");
    w.Key("maxRow").Number((wb_int64)g.maxRow);
    w.Key("maxCol").Number((wb_int64)g.maxCol);
    WriteList(w, "read", src);
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

// ---- Out Z All Down: golden btnOutZAllDownClick :4542-4562 ----
MotorAccessOutcome ZdOutAllDown(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (r.button != "btnOutZAllDown") return Refuse("teachOutZAllDown: 按鈕 '" + r.button + "' 不是 golden 的 btnOutZAllDown");
    const std::string who = "Out Z All Down（golden btnOutZAllDownClick :4542）";
    if (g_zdOps == 0) return Refuse(who + ": 教導頁 Z 鈕的 golden 介面沒有安裝（WebTeachZDownLive.cpp 只連進 wb_serve）—— 一軸都沒動");
    TeachZArmGrid g;
    std::string why;
    if (!g_zdOps->ArmGrid(false, g, why)) return Refuse(who + ": " + why + "（一軸都沒動）");
    if (g.motRow < 0 || g.motRow > 2 || g.motCol < 0 || g.motCol > 8) {
        char b[256];
        std::snprintf(b, sizeof(b), "OutArmSuck.iMotRow=%d／iMotCol=%d 超出 golden 的 Pos[2][8]／OutArmZIndex[2][8] —— golden 會讀寫表外，一軸都沒動",
                      g.motRow, g.motCol);
        return Refuse(who + ": " + b);
    }
    std::vector<int> zs;
    for (int i = 0; i < g.motRow; ++i)
        for (int j = 0; j < g.motCol; ++j) zs.push_back(g.zIndex[i][j]);
    {   //AI(W906-TEACH-ZDOWN) 20261003: decision 1 = B -- every motor of the machine (was ZdArmBusy: that arm only; the banner)
        const int n = g_zdOps->MotorCount();
        if (n <= 0) return Refuse(who + ": 讀不到機台的馬達數（golden TOTAL_MOTOR）—— 整台機台的忙碌檢查做不了，一軸都沒動");
        const std::string busy = ZdMachineBusy(be, g, zs, n);
        if (!busy.empty()) return Refuse(who + ": " + ZdMachineBusyWhy(busy) + "。一軸都沒動");
    }
    int pick = 0;
    if (!TeachFieldValue(r, "SetEditPickOutSht", pick, why)) return Refuse(who + ": " + why + "（一軸都沒動）");   // :4551 atoi(SetEditPickOutSht->Text)
    int pos[2][8] = { { 0 } };
    for (int i = 0; i < g.motRow; ++i)
        for (int j = 0; j < g.motCol; ++j) {
            int z = 0;
            if (!TeachFieldValue(r, kZdEdit[1][i][j], z, why)) return Refuse(who + ": " + why + "（一軸都沒動）");   // :4551 atoi(OutZEditPtr[i][j]->Text)
            const long long t = (long long)pick + (long long)z;
            if (t > 2147483647LL || t < -2147483647LL - 1)
                return Refuse(who + ": SetEditPickOutSht＋" + kZdEdit[1][i][j] + " 超出 int 範圍（一軸都沒動）");
            pos[i][j] = (int)t;
        }
    struct Ax { int i, j, mi; std::string alias; MotorAccessAxis a; MotorGolden gd; bool row, obj, sent; int ret; std::string speed, outcome, why; };
    std::vector<Ax> ax;
    // golden :4546-4553 -- SetSpeed(10) on every Out Z first, whatever the moves do afterwards (NB2 R144 #3)
    for (int i = 0; i < g.motRow; ++i)
        for (int j = 0; j < g.motCol; ++j) {
            Ax x;
            x.i = i; x.j = j; x.mi = g.zIndex[i][j]; x.sent = false; x.ret = 0;
            x.row = ZdRow(be, x.mi, x.alias, x.a);
            x.obj = x.row && be.GoldenMotor(x.a.motIndex, x.gd);
            if (!x.row) {
                x.speed = "沒設（這台的 Mot_Table 沒有這一軸）";
            } else if (!x.obj) {
                x.speed = "沒設（沒有 golden 馬達物件）";
            } else if (x.a.Is1203() && !x.a.tableEnable) {
                g_goldenPct[x.a.motIndex] = 10; g_ptpPct[x.a.motIndex] = 10;
                x.speed = "Enable=0：golden SetSpeed 只記 speed=10（不碰卡）";
            } else if (x.a.Is1203()) {
                std::string w2, note;
                if (!be.Pci1203Ready(w2)) x.speed = "沒設（1203 控制層不可用 —— " + w2 + "）";
                else if (x.a.axis < 0) x.speed = "沒設（" + x.a.why + "）";
                else {
                    MotionCtx m;
                    m.a = x.a; m.g = x.gd;
                    if (Send1203Speed(be, wireId, m, 10, w2, note)) { g_ptpPct[x.a.motIndex] = 10; x.speed = note; }   // golden TMyEtherCatMotor::SetSpeed (PTP family)
                    else x.speed = "沒設（" + w2 + "）";
                }
            } else {
                be.GoldenSetSpeed(x.a.motIndex, 10, false);                     // golden MOT[i].SetSpeed(10)
                g_ptpPct[x.a.motIndex] = 10;
                x.speed = "MOT[" + std::to_string(x.a.motIndex) + "].SetSpeed(10)";
            }
            ax.push_back(x);
        }
    // golden :4555-4561 -- MotorMove(Pos[i][j]) on every Out Z, its return ignored
    int moving = 0, arrived = 0, noHw = 0, refused = 0;
    for (std::size_t k = 0; k < ax.size(); ++k) {
        Ax& x = ax[k];
        const int target = pos[x.i][x.j];
        if (!x.row || !x.obj) {
            x.outcome = "noHardware";
            x.why = "這台的 Mot_Table 沒有這一軸（或沒有 golden 物件）—— golden 的替身 MotorMove 只寫 Position；這裡不呼叫";
            ++noHw;
            continue;
        }
        if (x.a.Is1203()) {
            MotorGolden fresh;
            if (be.GoldenMotor(x.a.motIndex, fresh)) x.gd = fresh;
            const unsigned long s0 = x.a.axis >= 0 ? g_issuedSeq[x.a.axis] : 0ul;
            std::string note;
            x.ret = GoldenMove1203(be, x.a, x.gd, target, wireId, note);
            x.sent = x.a.axis >= 0 && g_issuedSeq[x.a.axis] != s0;
            const bool failed = note.find("MoveToPos failed") != std::string::npos;   // GoldenMove1203's own text when the card refused
            if (!x.a.tableEnable) { x.outcome = "noHardware"; x.why = "Enable=0（Mot_Table）：golden MotorMove 回 1、不下硬體命令（mymotor.cpp:821-823）"; ++noHw; }
            else if (x.sent && !failed) { x.outcome = "moving"; ++moving; }
            else if (x.ret == 1) {
                x.outcome = "arrived";
                x.why = "golden MotorMove=1，沒有送命令（已在目標的 CompareCommandPos 間隙內，或上一次對同一目標的 MotorMove 還算數：fCMD／iOldPos，mymotor.cpp:883-889、:718-815；再按一次會動）";
                ++arrived;
            } else {
                x.outcome = "refused";
                x.why = x.ret == -1 ? std::string("安全門開著（golden MotorMove -1）") :
                        !note.empty() ? note : std::string("軸不在 READY 或監看器還沒更新：golden MotionDone()==false → MotorMove 回 0，沒有送命令");
                ++refused;
            }
        } else {
            x.ret = be.GoldenMotorMove(x.a.motIndex, target, 0, false);          // golden MOT[i].MotorMove(Pos) itself (no SetSpeed, no InitMOTParameter)
            if (!x.a.tableEnable) { x.outcome = "noHardware"; x.why = "Enable=0（Mot_Table）：golden MotorMove 只記 Position，不碰硬體"; ++noHw; }
            else if (x.ret == 0) { x.outcome = "moving"; ++moving; }
            else if (x.ret == 1) { x.outcome = "arrived"; x.why = "golden MotorMove=1（已在目標，或上一次同目標的 fCMD 還算數）"; ++arrived; }
            else {
                x.outcome = "refused";
                x.why = x.ret == -1 ? "安全門開著（golden MotorMove -1）" : x.ret == -2 ? "目標超過正向軟體極限（golden MotorMove -2）" :
                        x.ret == -3 ? "目標低於負向軟體極限（golden MotorMove -3）" : "golden MotorMove 回 " + std::to_string(x.ret);
                ++refused;
            }
        }
    }
    char hb[200];
    std::snprintf(hb, sizeof(hb), "golden btnOutZAllDownClick：每一支 Out Z SetSpeed(10)，再 MotorMove(SetEditPickOutSht %d＋各吸嘴 Z) —— 移動 %d、已在目標 %d、沒有硬體 %d、沒動 %d",
                  pick, moving, arrived, noHw, refused);
    std::string sum;
    for (std::size_t k = 0; k < ax.size(); ++k)
        sum += "; " + ZdName(ax[k].mi, ax[k].alias) + "→" + std::to_string(pos[ax[k].i][ax[k].j]) + " " + ax[k].outcome +
               (ax[k].why.empty() ? std::string() : "（" + ax[k].why + "）") + " [速度 " + ax[k].speed + "]";
    if (moving + arrived == 0)
        return Refuse(who + ": " + hb + sum + (ax.empty() ? "；這台 OutArmSuck 的 iMotRow×iMotCol 是空的（golden 迴圈不做）" : "") +
                      "（golden 照樣先把每一支設成 10%：NB2 R144 #3）");
    webbridge::JsonWriter w;
    w.BeginObject();
    ZdHead(w, r, refused ? "error" : "done", moving ? "moving" : "inPosition", std::string(hb) + sum);
    w.Key("arm").String("out");
    w.Key("pickOutSht").Number((wb_int64)pick);
    w.Key("motRow").Number((wb_int64)g.motRow);
    w.Key("motCol").Number((wb_int64)g.motCol);
    w.Key("moving").Number((wb_int64)moving);
    w.Key("arrived").Number((wb_int64)arrived);
    w.Key("noHardware").Number((wb_int64)noHw);
    w.Key("refused").Number((wb_int64)refused);
    w.Key("partial").Bool(refused > 0);
    w.Key("axes").BeginArray();
    for (std::size_t k = 0; k < ax.size(); ++k) {
        const Ax& x = ax[k];
        w.BeginObject();
        w.Key("row").Number((wb_int64)x.i);
        w.Key("col").Number((wb_int64)x.j);
        w.Key("edit").String(kZdEdit[1][x.i][x.j]);
        w.Key("motIndex").Number((wb_int64)x.mi);
        w.Key("motor").String(x.alias);
        w.Key("target").Number((wb_int64)pos[x.i][x.j]);
        w.Key("speed").String(x.speed);
        w.Key("goldenRet").Number((wb_int64)x.ret);
        w.Key("sent").Bool(x.sent);
        w.Key("outcome").String(x.outcome);
        w.Key("why").String(x.why);
        w.EndObject();
    }
    w.EndArray();
    MotorAccessOutcome o; o.ok = true; o.ackJson = Finish(w); return o;
}

}  // namespace

void            TeachZDownInstallOps(ITeachZDownOps* ops) { g_zdOps = ops; }
ITeachZDownOps* TeachZDownInstalledOps() { return g_zdOps; }

MotorAccessOutcome TeachZDownDispatch(const MotorAccessReq& r, long long wireId, IMotorAccessBackend& be)
{
    if (r.source != "uteach")
        return Refuse("motor.access " + r.action + "（" + r.source + "）: 這是 golden uteach 教導頁的按鈕（btnSetAllInArmZ :4300／btnSetAllOutArmZ :4367／"
                      "btnOutZAllDown :4542），MotorTest 沒有");
    if (r.action == "teachSetAllArmZ") return ZdSetAll(r, be);
    return ZdOutAllDown(r, wireId, be);
}

}  // namespace ht9045

// =============================================================================
//  AI(W906-MSGBOX-ABANDON) 20261003: INBOX 150 -- golden abandons Motor Test HOME / LoopMove (and Teach's btnHome) when a MESSAGE
//  box shows, not only for the alarm note.  golden 906_20260618 (RULINGS_20261001 #0):
//    uMotorTest.cpp:912-926 Timer1Timer -- every tick while Motor Test is shown (`if(fShow==false) return;` :914):
//        if(fNote->fShow || MyMessageBox->fShow) { btnLoopMove->Down=false; btnHome->Down=false; bSingleHome=false; }   (:921-926)
//      and :977-980 `if(btnHome->Down && (MyMessageBox->fShow || fNote->fShow)) btnHome->Down=false;`
//    uteach.cpp:1359-1368 Timer1Timer -- `if(ActiveMotorIndex==-1 || (btnHome->Down && (MyMessageBox->fShow || fNote->fShow)))
//        btnHome->Down=false;`
//    MyMessageBox->fShow is set by TMyMessageBox::FormShow (mymessbox.cpp:344), i.e. by every MyMessageBox: ShowMyMessage (:761-863,
//    ShowModal :860 -- VCL timers keep firing under another form's ShowModal, so Motor Test's Timer1 sees it), ShowUnloaderTrayMessage
//    (:911-936, Show :935), ShowMyMessageBox_YES_NO (:1009-1054).  The stop: ShowMyMessage :830-831 `if(SystemInitialOK==true)
//    StopAllMotor();` and FormShow :302-310 `if(!iUnLoaderCount) { SystemStart=false; SoftStart=false; if(SystemInitialOK==true)
//    StopAllMotor(); ... }` -- a NonStop box (iUnLoaderCount!=0, ShowUnloaderTrayMessage) stops nothing, yet Timer1 still raises the
//    buttons.
//  The port did this for the alarm note (ShowErrorMessage / ShowMotorErrorMessage -> MotorAccessOnAlarm) and the YES / NO box
//  (W906_YesNoShowLikeGolden -> MotorAccessOnAlarm("YESNO")); ShowMyMessage and ShowUnloaderTrayMessage only stopped golden's own
//  motor objects (StopAllMotor(true): the 1203 monitor's axes are not reached) and the Teach Arm Cell job.  wb_serve's two hosts now
//  call MotorAccessOnMessageBox at that same edge (W906_MotorAccessOnMessageBox, WebMotorAccessLive.cpp EOF; same-line inserts in
//  tools/wb_serve.cpp W906MbShowMyMessage / W906MbShowUnloaderTray, right after the Arm Cell edge).  A blocking box then holds the
//  tick thread until it is answered, so the moment it shows is the only moment the port can act -- as for the alarm.
//  stopsMotors (the host passes golden's condition of that box):
//    true  -> MotorAccessOnAlarm's sweep without its alarm-lamp rule (golden UpdateMotorLed :644-652 is fNote-only): every HOME job
//             (not Light Scale's) / the LoopMove / a Gear Ratio session ends, bSingleHome=false while Motor Test is shown
//             (EndSingleHome, golden :925), the jogs are forgotten and every opened 1203 axis gets the stop (the 1203 half of golden
//             StopAllMotor, inside the same MotorAccessAlarmSweep scope).
//    false -> the NonStop box: nothing is stopped and the jogs stay (golden stops nothing); the btnHome HOME jobs and the loop end
//             (Timer1 :922-923, uteach :1364-1368), Light Scale's single home only while Motor Test is shown (:925), a Gear Ratio
//             session ends; a Teach Z All Up home goes ON -- golden DoZHome (uteach.cpp:4505-4540) does not look at MyMessageBox and
//             nothing stops its motor.
//  The Teach Arm Cell job is NOT touched here: its own edge (MotorAccessArmCellOnPopup, called by the same hosts just before) applies
//  R165's rules, including "a box popped by the job's own S2 check is that check's".
//  Same deviation as the alarm edge (the machine's TEACH-ZALLUP banner, Not reproduced (3)): when the box STOPS the motors, a Teach Z
//  All Up home is cancelled too (HomeFlag stays 0); golden stops the motor and DoZHome keeps stepping a stopped home.  Likewise a Light
//  Scale single home whose axis this stop reaches while Motor Test is NOT shown ends here (Light Scale keeps waiting at case 1, golden's
//  bSingleHome stays true) -- so TickHomes never reads that stopped DS402 home as done (the WSLINK-B rule of the Teach close edge;
//  the alarm edge does not do this one).
//  Not done: the "state" half -- golden's Timer1 also raises a HOME / LoopMove started WHILE a box is up (within 5 ms); here a blocking
//  box holds this tick thread, and the NonStop box comes from production steps that do not run while Motor Test / Teach are open
//  (VerifyMotorAction pauses MainProc, EastSun R8).
// =============================================================================
namespace ht9045 {
void MotorAccessOnMessageBox(IMotorAccessBackend& be, const std::string& kind, const std::string& what, bool stopsMotors)
{
    const std::string why = "message box " + kind + (what.empty() ? std::string() : " \"" + what + "\"") +
                            " (golden uMotorTest Timer1Timer :921-926 / uteach Timer1Timer :1364-1368: MyMessageBox->fShow -> btnLoopMove / btnHome up)";
    const bool had = AnyJobActive() || !g_jogs.empty();
    int n = 0;
    for (std::size_t i = 0; i < g_homes.size(); ++i) {
        HomeJob& h = g_homes[i];
        if (!h.active) continue;
        if (h.fromLS) {                                                         // Light Scale's single home = golden bSingleHome
            if (g_mtShown) CancelHome(be, h, false, why + " + bSingleHome=false (:925)");
            else if (stopsMotors) CancelHome(be, h, false, why + " -- its axis is stopped below (golden: a stopped single home is not stepped while no page is shown); Light Scale keeps waiting at case 1");
            continue;                                                           //   (the second: never read a stopped DS402 home as done -- the WSLINK-B rule of the Teach close edge)
        }
        if (h.zAllUp && !stopsMotors) continue;                                 // NonStop: golden DoZHome goes on (nothing stops its motor)
        CancelHome(be, h, false, why);
    }
    CancelLoop(be, false, why);
    CancelGear(be, why);
    if (g_mtShown && g_ls.homePending) {                                        // golden :925 bSingleHome=false (Timer1 runs while Motor Test is shown)
        g_ls.homePending = false; g_ls.lastNote = "bSingleHome=false (" + why + ") -> Light Scale case 1 goes on";
    }
    if (stopsMotors) {
        g_jogs.clear();
        std::string w;
        if (be.Pci1203Ready(w)) {
            MotorAccessAlarmSweep sweep;                                        // the 1203 half of golden StopAllMotor (MotorAccessInAlarmSweep)
            for (int ax = 0; ax < be.Pci1203AxisCount(); ++ax) {
                if (!be.Pci1203AxisOpened(ax)) continue;
                Stop1203(be, ax, -1);                                           // golden DecStop (StopDec + ExtDrive 0)
                ++n;
            }
        }
    }
    if (had || n)
        g_lastJobNote = why + ": jobs ended" + (stopsMotors ? ", 1203 stop sent to " + std::to_string(n) + " axis(es)" : std::string(", nothing stopped (NonStop box)"));
}
}  // namespace ht9045

namespace ht9045 {
//AI(W906-HOMEBLOCK) 20261003: NB2-1 (URGENT U14 item 3, README R175 C1 / R188 H1; the laptop's CHAT_JIMMY 1003 14:0x) -- see WebMotorAccess.h.
//  The same question START (start.run, W906_RemoteRun.Start) and wb_serve's main.home ask (MotorAccessStartBlocked), asked by
//  TfMain::Home itself, so every HOME is covered: the panel HOME key, SECS RCMD HOME (S2F42), OLP DoHomeAndStart / ProcessBuffer,
//  ESD DoAutoDecayCheck, Home by Start. Golden has no Arm Cell; the hand-teach refusal is the port's W5B-R5 rule that START and
//  main.home already apply (in the port the teach page can close while the hand teach stays on -- golden's fTeachShow is modal).
//  Only the refusal text says HOME; the decision is MotorAccessStartBlocked's, unchanged.
bool MotorAccessHomeBlocked(const std::string& func, std::string& why, bool& logOnce)
{
    static std::string s_last;                                                  // the (func, why) last logged; cleared by the first pass
    std::string sw;
    if (!MotorAccessStartBlocked(sw)) { s_last.clear(); logOnce = false; return false; }
    why = MotorAccessArmCellBlockedWhy("HOME");
    if (why.empty())
        why = "HOME 拒絕：教導頁手動教導中（" + g_teachSet.btn + "，伺服是關的）—— 歸零會讓馬達上電、接手操作員正在手推的軸；"
              "先在教導頁按確定或取消（同 START 的 W5B-R5，" + std::string(kW5bDoc) + " D9）";
    const std::string key = func + "\n" + why;
    logOnce = key != s_last;
    s_last = key;
    return true;
}

//AI(W906-BRAKE-SERVOFIRST) 20261004: NB2-1 task (m), RULINGS_20261003 #24 -- see WebMotorAccess.h.
bool BrakeServoOnLedger::Step(unsigned seq, bool powerOn)
{
    if (!init) { init = true; base = seen = seq; }       // what was issued before the first look is not "this power-on"
    bool fresh = false;
    if (seq != seen) { seen = seq; fresh = powerOn; }
    if (!powerOn) base = seq;                             // power not on: every Servo ON so far is history
    return fresh;
}
bool BrakeServoOnLedger::Issued(unsigned seq) const { return init && seq != base; }
bool BrakePowerOn(bool relayEnabled, bool relayOut, bool emg, bool snPowerOff)
{
    return (!relayEnabled || relayOut) && !emg && !snPowerOff;
}
bool BrakePowerReady(bool powerOn, bool motorPowerState, int powerOnDelay, std::string* why)
{
    const char* w = !powerOn ? "馬達電源沒開（繼電器 OFF／EMG／SnMotorPower OFF）"
                  : !motorPowerState ? "bMotorPowerState=false（golden 的上電還沒完成）"
                  : powerOnDelay > 0 ? "上電延遲還沒數完（MotorPowerOnDelay>0）" : 0;
    if (w && why) *why = w;
    return w == 0;
}
bool BrakeMotionKind(int kind)
{
    return kind == kCmdAxJogStart || kind == kCmdAxMoveRel || kind == kCmdAxMoveAbs || kind == kCmdAxHome ||
           kind == kCmdAxMoveVel || kind == kCmdAxMoveImpose || kind == kCmdAxMoveHome;
}
}  // namespace ht9045

namespace ht9045 {
namespace {
//AI(W906-SVON-ENCSYNC) 20261004: NB2-1 (n) (laptop CHAT_JIMMY 1003 23:1x; RULINGS_20261003 #24; Steven 1003 22:5x 「一直沒斷電：已經歸零過的軸不用再歸零。
//  但是要把encoder pulse回寫給command pulse」「比照golden的 servo on功能」).
//  golden TMyMotor::ServoOnOff(true) (Motor/mymotor.cpp:1373-1398): SetServoOn, then on a PServoAlarmOn axis PCIL132_ResetPos
//  (:1320-1335): p = ReadEnCoderRealPos() -- the drive's actual position as an int (the Direction flip there and at :1331 cancel) --
//  then ResetPos(p) = SetCommand(p) + SetPosition(p): the command position becomes the encoder's; the actual position is written
//  with the value it already has. A web Servo ON / the end of a hand teach on a 1203 axis does the same through the card: after the
//  issued SvOn, kCmdAxSetCmdPos value = (int) the monitor's actPos. SetPosition(p) of what the card already reports is not sent: it
//  changes nothing, and on a DS402 drive the drive owns that number (EastSun Q1). Synced -> the axis leaves g_encBase (W5B-6): its
//  command position IS the encoder's now. The next command waits for a fresh sample (g_issuedPoll, SampleStale) -- not g_issuedSeq,
//  which would end a Gear Ratio hand session on this axis (GearHandTick: "another command") at the Servo ON that is part of it.
//  Not a PServoAlarmOn axis -> nothing, as golden. A refused / failed / dry / unreadable sync -> note says so, the axis stays on
//  g_encBase (fail-closed, as before). Golden reads the card right after SetServoOn (its MySleep(200) is gated here too); the
//  monitor's actPos is the last poll's (<= 200 ms old) -- the same race golden has with an operator still pushing the axis.
bool ServoOnEncSync(IMotorAccessBackend& be, long long wireId, const MotorAccessAxis& a, std::string& note)
{
    MotorGolden g;
    if (!a.Is1203() || a.axis < 0 || a.motIndex < 0 || !be.GoldenMotor(a.motIndex, g) || !g.servoAlarmOn) return false;
    double act = 0.0;
    if (!be.Pci1203ActPos(a.axis, act) || !std::isfinite(act) || act > 2147483647.0 || act < -2147483648.0) {
        note = "命令位置沒有寫成編碼器位置：讀不到這一軸的實際位置（golden PCIL132_ResetPos）—— 這一軸照舊以編碼器為基準（W5B-6）";
        return false;
    }
    Pci1203Cmd c;
    c.kind = kCmdAxSetCmdPos; c.wireId = wireId; c.axis = a.axis; c.value = (double)(int)act;   // golden: int p = ReadEnCoderRealPos()
    const Pci1203CmdResult res = be.Pci1203Execute(c);
    if (CmdFailed(res) || !res.issued) {
        note = "命令位置沒有寫成編碼器位置：" + (CmdFailed(res) ? CmdWhy(res) : std::string("1203 控制層是 dry，沒有送出")) + " —— 這一軸照舊以編碼器為基準（W5B-6）";
        return false;
    }
    g_issuedPoll[a.axis] = be.Pci1203PollCount();
    g_encBase.erase(a.axis);
    note = "命令位置＝編碼器 " + std::to_string((int)act) + "（golden ServoOnOff → PCIL132_ResetPos）";
    return true;
}
}  // namespace
}  // namespace ht9045
