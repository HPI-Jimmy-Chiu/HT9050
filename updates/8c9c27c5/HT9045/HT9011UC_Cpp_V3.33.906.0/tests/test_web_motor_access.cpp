// =============================================================================
//  tests/test_web_motor_access.cpp
//
//  AI(W906-W4-MOTOR) 20260925: WebMotorAccess.cpp（`motor.access` 的解析與分派）配假後端。
//
//  鎖住的東西：
//   (1) 解析：motor-access.js 組出來的 request 原樣吃得進來；null 參數「沒給」≠ 0；壞輸入拒絕。
//   (2) catalog 覆蓋：web/JSON/motor-access.json 的 38 個命令（28 個 action）每一個都有歸屬；
//       live 19 個（stop／servoToggle＋W4-b1 的 12 個＋W4-b2 的 5 個），blocked 2 個，ui 只有 setPos1／setPos2／refreshParameter。
//   (3) 誠實：還沒接上的 action 回 ok=false 並說哪一波，而且**後端一次都沒被呼叫**（不會偷偷動）。
//   (4) stop = golden StopAllMotor（帶 source）＋ 對每一個開成功的 1203 軸送 kCmdAxStop（跳過沒開的槽）；
//       控制層不存在時照樣做 golden 那半並照實回報。
//   (5) servoToggle 的目標狀態：頁面明講 > 實際狀態取反 > uMotorTest 的 golden static；uteach 讀不到就拒絕。
//   (6) W4-b golden 換算的手算值：GetRealPos 的逼近迴圈、ReadRealPos 回 int、SetSpeed 的 % → s → velLow/velHigh（含夾限、Z 內插、index 不設）。
//   (7) W4-b1 運動（uMotorTest）：jog = 4 個 setSpeed＋moveVel（EastSun）；Direction=1 的 1203 軸拒絕（§0 第 5 件）；
//       999999／安全門／IsMotorCanRun／HomeFlag／鎖（先停）／軟體極限 >= <=／軸忙／Enable=0 的拒絕順序；非 1203 軸走 golden MOT；
//       參數鈕寫 golden 記憶體；NB2 R19 的 dry 計數與 partial 旗標。
//   (8) W4-b2：HOME（124／128 依 HomeDirection、HOMING→READY 才設 1、ERROR 設 2、沒進 HOMING 5 秒設 2、toggle、arm Z 去 ZSafePos、
//       STOP 取消）、LoopMove（未歸零要確認、兩段＋等待＋計數、toggle、半途被軟體極限擋就停）、ResetMNet、Range／Rate、blocked 兩顆。
//   (9) 每一種成功 ack 都不帶傳輸層保留字 type／id／ok／error（WebBridgeServer::AckJson 會把它攤平併進 WS ack），請求 id 叫 reqId。
//  (10) NB2 R21：Enable=0 真機不可選、InShuttle 閘門互鎖、卡片回傳碼、jog 死人開關、命令後的輪詢新鮮度、Index 軸速度鈕寫 ReadSpeed。
//  (11) NB2 R22／R23：1203 軸 Enable=0 任何組態都拒絕；HOME 門檢查＋歸零速度＋一按就清 HomeFlag；arm Z 的 case 500 照 golden 回傳值；
//       Timer1Timer 三道閘（連線、安全鎖、告警）；home／loop 改成明講狀態；HOME／Loop 中其他運動鈕被擋；伺服鈕取消工作；All 模式拒絕。
//  (12) AI(W906-MT-E2) 20260925：選馬達（Enable=0 先擋、SetSpeed(1)）、速度捲軸／edtSpeed 與 jog 速度來源（Jog = golden）、參數表格子、
//       Copy From、告警時選中軸 Alarm 燈的規則、LastHomePos、Loop 計時（假時鐘；只平均第二段、只有已歸零的開始才歸零、golden static flag）。
//       Reload Motor Data 的表值重套在 (9) 的最後。catalog 覆蓋 (2) 另驗與網頁那邊約好的 5 個命令。
//  (13) AI(W906-MT-E3c) 20260925：Motor Power（needConfirm、跨拍 1 秒、關電先停 1203 再 GaliMotorServoOff、卡片回讀同步）、
//       FormShow／FormClose、Test Range／Rate 的完整 InitMotor（順序、ResetError 有界重試、EMG、bAlarm、SetRate 手算值）、
//       R2（M14 走 1203）、All 模式來回、VerifyMotorAction 的 hook、Light Scale（目標序列、Memo1 逐字、存檔內容、golden 停不下來的行為）。
//       catalog (2) 另驗 MT-E3c 的 6 個命令（48 個命令、37 個 action）。
//  (14) AI(W906-MT-FIX1) 20260926：Motor Power 方向防呆、Poll 之後的閘、jog 重送捲軸寫的值、捲軸寫卡失敗（機台）。
//  (15) W5-b 教導頁運動鈕（AI(W906-MERGE-56bbf785) 20260926：筆電的 (12)，合進機台後改編 (15)；另加合併後的交互：教導頁 jog 走機台的
//       golden jog、運轉中不清 fAllMotorHome、Motor Power／FormClose 不被運轉閘擋、Direction=1 照 6B）：golden 按鈕表（處理函式＋所屬清單＋馬達）、GoButton140（1% 速度、HomeFlag、嚴格軟體極限、背隙）、
//       GoButton020（兩軸 20%）、SetButton140／020／064 兩段式手動教導（關伺服→讀編碼器→開伺服）、怪按鈕拒絕、教導頁 jog／移動／歸零。
//       AI(W906-W5-b) 20260925: W5-b 覆核 W5B-11 —— 改用**真的** WebTeachButtons.gen.inc（依組態條件建登錄表）：建構子 OnClick 覆寫、
//       死登錄、Tag 語意（068／069／064／065）、怪按鈕判定與選項 C、目標值驗證、EditPtr、IndexArm_3_Axis、ep1Picker remap、非 1203 軸、
//       開／關伺服失敗與回滾、GoButton020 第 2 軸超極限、HOME 抬起 SetSpeed(1)、死人開關、編碼器基準、背隙方向、Contec 分支、
//       IsCanQuickJogMove 本體的 1203 狀態來源（MotorAccessTeachHomeLed／Live1203）。
//       AI(W906-MERGE-56bbf785) 20260926: 合併後覆核 —— 手動教導中 MotorTest 的整頁動作（Reload／Light Scale 開始／Motor Power／FormShow／
//       All 模式來回）與請求裡每一軸都擋、放開類（HOME／Loop 抬起、關掉進行中的 Light Scale）在運轉閘與手動教導都放行、教導頁 jog 的速度＝DoJog 的來源。
//  (ZALLUP) AI(W906-TEACH-ZALLUP) 20261001: Teach In/Out Z All Up (golden btnInZAllUpClick / btnOutZAllUpClick: every Z of the arm homes,
//       Enable=0 -> HomeFlag=1, case 500 ZSafePos, re-home when already homed, no second command while homing, the door interlock inside
//       the home, partial, STOP) and the Index tab Servo buttons (btnZ1/Z2/Arm1Y/Arm2YServo: actual state / golden static, brake hold,
//       Enable=0 refusal, 4-axis Y pair); SystemStart / hand teach / Motor Test source refused. Catalog (2): 58 commands, 42 actions, live 37 (AI(W906-ST02-C9-G2) 20261002: + St02Teach* / teachSt02).
//  (ARMCELL) AI(W906-ARMCELL) 20261002: the Teach page "Arm Cell" tab, moveToTrayCell (RULINGS_20261002 #18): S0 refusals, S1 Z to ZSafePos,
//       S2 Z-at-home + golden CheckCanMove once, S3 X / Y in one beat, S4 only the chosen Z + D2, "arrived" last, every cancel trigger
//       stopping its own axes (D3), the gates while it runs, the runtime armCell block (Q3). Catalog (2): 58 commands, 42 actions, live 37; with St02's teachSt02 (laptop batch 49 merge) 59 / 43 / 38 (62 / 46 / 41 with GEARRATIO below).
//  (ARMCELL2) AI(W906-ARMCELL2) 20261002: NB2 R165 -- a box on screen stops the job (edge + state; the S2 check's own box excluded),
//       S0 / the beat read the alarm / EMG lamps and the EMG sensors, the S3 Z origin watch, arrival by the encoder, X refused -> no Y,
//       every timeout stops the axes. Catalog unchanged (the tab's All Z Up presses Teach's own btnIn/OutZAllUp rows).  (ARMCELL-ZORG) AI(W906-ARMCELL-ZORG) 20261003: NB2 R174 M1 -- the first dark Z origin sample stops X / Y at once (HT160 MoveSortArmX :733), the next fresh sample ends the job (still dark / a flicker), the post-Poll watch; at the end of ARMCELL2. Same line, nothing moves
//  (GEAR) AI(W906-GEARRATIO) 20261002: the Motor Test "Gear Ratio" tab (RULINGS_20261002 #22): gearCalMove's gates (each refuses, nothing
//       moved), the session (take-up = gauge zero, cTrue from the pulses, interrupted / stopped short / not READY / stale), what ends it,
//       gearRatioPreview (the spec's 50 / 100 / 200 example, nothing written), gearRatioSave (key, Teach window, HomeFlag, confirmation band;
//       three Mot_Table cells, this axis' teach keys, backups, memory, ONLY this axis HomeFlag=0, no zeroing, the log), three writer faults
//       restored, the runtime gearCal block. argv[2] = its scratch directory. Catalog (2): 61 commands, 45 actions, live 40 on its own; with St02's teachSt02 (laptop batch 49 merge) 62 / 46 / 41.  (ZDOWN) AI(W906-TEACH-ZDOWN) 20261003: Teach Set All In / Out Arm Z + Out Z All Down (INBOX 147, at the end of main) -- 65 / 48 / 43.
//
//  NOT COVERED：真實後端（WebMotorAccessLive.cpp：HSys.MotTable／MOT[]／監看器的對號）—— 那一半要
//  wb_serve 起來後用 WS 探針量；1203 那一段要有卡的機台。
// =============================================================================
#include "WebMotorAccess.h"
#include "Public/cJSON.h"
#include "WebTeachZDown.h"     // AI(W906-TEACH-ZDOWN) 20261003: ITeachZDownOps / TeachZDownInstallOps (section ZDOWN). Was a blank line: no line below moves
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>       // AI(W906-MT-E3c) 20260925: std::fabs (SetRate)
#include <cstdlib>     //   std::getenv (the Light Scale temp root)
#include <direct.h>    //   _rmdir
#include <process.h>   //   _getpid
#include <cstring>     //   std::strlen (AI(W906-GEARRATIO) 20261002: the scratch-directory guard)
#include <functional>  //   std::function (AI(W906-GEARRATIO) 20261002: the gate / end / fault tables)

using namespace ht9045;

// AI(W906-MT-E3c) 20260925: the Light Scale save files, read back byte for byte.
static std::string ReadAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static bool FileExists(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); return f.good(); }
//AI(W906-GEARRATIO) 20261002: the Gear Ratio section's helpers -- the lines of a text (no terminators), and the first number after "key":
static std::vector<std::string> Lines2(const std::string& t)
{
    std::vector<std::string> v;
    std::string cur;
    for (std::size_t i = 0; i < t.size(); ++i) {
        if (t[i] == '\n') { v.push_back(cur); cur.clear(); }
        else if (t[i] != '\r') cur += t[i];
    }
    if (!cur.empty()) v.push_back(cur);
    return v;
}
static double NumIn(const std::string& json, const char* key)
{
    const std::string k = std::string("\"") + key + "\":";
    const std::size_t i = json.find(k);
    if (i == std::string::npos) return -1e300;
    return std::strtod(json.c_str() + i + k.size(), 0);
}

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }

// ---------------------------------------------------------------------------
class FakeBackend : public IMotorAccessBackend {
public:
    std::map<std::string, MotorAccessAxis> table;
    bool        ready;
    std::string notReadyWhy;
    std::vector<bool> opened;                 // 監看器軸槽
    std::map<int, bool> svon;                 // 1203 軸的 SVON（沒有 = 讀不到）
    std::map<int, bool> goldenSvon;           // MOT[i].Led[iServoOn]（沒有 = 讀不到）
    bool        rejectNext;                   // 下一次 Execute 回 accepted=false
    bool        dry;                          // Execute accepted 但 issued=false

    std::vector<Pci1203Cmd>  executed;  std::vector<bool> stopSweep;   //AI(W906-INDEXZ-1203) 20260930: MotorAccessInAlarmSweep() at every stop Execute (same line)
    std::set<int>            rejectKinds;     // AI(W906-MT-E3c) 20260925: Execute refuses every command of these kinds
    std::vector<std::string> stopAll;         // GoldenStopAll 的 source
    std::vector<int>         stopMotor;
    std::vector<std::pair<int, bool> > servoSet;
    std::vector<std::string> refusals;

    FakeBackend() : ready(true), rejectNext(false), dry(false), doorClosed(true), canRun(true), moveRet(0),
                    zSafe(20), systemStart(false), resetCount(0),
                    polls(0), freezePolls(false), floodReady(true), readSpeedV(0), retNext(0), safeLock(false),
                    regOk(true), canTeach(true), powerOff(false), indexArm3(false), ep1(false),
                    prodBacklashIn(0), prodBacklashOut(0), failSetSpeed(0), clearAllHome(0) {}
    int Calls() const
    {
        return (int)(executed.size() + stopAll.size() + stopMotor.size() + servoSet.size());
    }

    bool Resolve(const std::string& id, MotorAccessAxis& out) override
    {
        std::map<std::string, MotorAccessAxis>::const_iterator it = table.find(id);
        if (it == table.end()) return false;
        out = it->second;
        return true;
    }
    bool Pci1203Ready(std::string& why) override { if (!ready) why = notReadyWhy; return ready; }
    int  Pci1203AxisCount() override { return (int)opened.size(); }
    bool Pci1203AxisOpened(int ax) override { return ax >= 0 && ax < (int)opened.size() && opened[ax]; }
    bool Pci1203ServoOn(int ax, bool& known) override
    {
        std::map<int, bool>::const_iterator it = svon.find(ax);
        known = (it != svon.end());
        return known ? it->second : false;
    }
    std::vector<Pci1203Cmd>  extDrive;           // AI(W906-MT-E1) 20260925: Acm_AxSetExtDrive (jog mode 1 / 0) kept apart, so the stop/move sequences below stay readable
    Pci1203CmdResult Pci1203Execute(const Pci1203Cmd& c) override
    {
        if (c.kind == kCmdAxSetExtDrive) { extDrive.push_back(c); Pci1203CmdResult x; x.ret = 0; x.accepted = true; x.issued = !dry; x.wouldCall = "Acm_fake(ExtDrive)"; return x; }
        executed.push_back(c);  if (c.kind == kCmdAxStop || c.kind == kCmdAxEmgStop) stopSweep.push_back(MotorAccessInAlarmSweep());   //AI(W906-INDEXZ-1203) 20260930: review round 2 D2 -- was this stop the alarm sweep's? (same line)
        Pci1203CmdResult r;
        r.ret = 0;
        if (rejectKinds.count((int)c.kind)) { r.accepted = false; r.issued = false; r.why = "fake: this kind refused"; return r; }   // AI(W906-MT-E3c)
        if (c.kind == kCmdAxMoveAbs && rejectMoveAxes.count(c.axis)) { r.accepted = false; r.issued = false; r.why = "fake: move refused on this axis"; return r; }   //AI(W906-ARMCELL) 20261002
        if (c.kind == kCmdAxSvOn) {                                             // W5-b：指定哪一軸的開／關伺服失敗
            std::map<std::pair<int, int>, int>::iterator f = svFail.find(std::make_pair(c.axis, (int)c.value));
            if (f != svFail.end() && f->second > 0) { --f->second; r.accepted = false; r.issued = false; r.why = "fake SvOn refused"; return r; }
        }
        if (c.kind == kCmdAxSetSpeed && failSetSpeed > 0) { --failSetSpeed; r.accepted = false; r.issued = false; r.why = "fake SetSpeed refused"; return r; }
        if (rejectNext) { rejectNext = false; r.accepted = false; r.issued = false; r.why = "axis in ERROR_STOP"; return r; }
        if (retNext) { r.accepted = true; r.issued = true; r.ret = retNext; retNext = 0; r.why = "Positive hardware limit has been exceeded"; return r; }
        r.accepted = true;
        r.issued = !dry;
        char b[64];
        std::snprintf(b, sizeof(b), "Acm_fake(kind=%d, ax=%d, v=%g)", (int)c.kind, c.axis, c.value);
        r.wouldCall = b;
        return r;
    }
    void Pci1203NoteRefusal(long long, const std::string&, const std::string& why) override { refusals.push_back(why); }
    void GoldenStopAll(const std::string& source) override { stopAll.push_back(source); }
    void GoldenStopMotor(int mi) override { stopMotor.push_back(mi); }  std::vector<int> smBegin, smStep; std::map<int, int> smStepsLeft;  bool GoldenSingleHomeBegin(int mi) override { smBegin.push_back(mi); return true; }  bool GoldenSingleHomeStep(int mi) override { smStep.push_back(mi); std::map<int, int>::iterator it = smStepsLeft.find(mi); if (it == smStepsLeft.end() || it->second < 0) return false; if (it->second > 0) { --it->second; return false; } homeFlags[mi] = 1; return true; }   //AI(W906-MT-SMHOME) 20261001: golden InitProcessSingleMotorTask / one MainProc pass of ProcessSingleMotorHome && HomeFlag==1 (smStepsLeft[mi] = passes still homing; -1 = never)
    bool GoldenServoOn(int mi, bool& known) override
    {
        std::map<int, bool>::const_iterator it = goldenSvon.find(mi);
        known = (it != goldenSvon.end());
        return known ? it->second : false;
    }
    void GoldenServoOnOff(int mi, bool on) override { servoSet.push_back(std::make_pair(mi, on)); }

    // ---- W4-b ----
    std::map<int, double>      cmdPos;       // 1203 軸槽 → 卡片 cmdPos（沒有 = 讀不到）
    std::set<int>              notReady;     // 不在 READY 的軸槽
    std::map<int, MotorGolden> golden;       // MOT 下標 → golden 參數（沒有 = Motor 為 NULL）
    std::set<int>              doorOpen;     // CheckIsSafeDoorOpen() 為 true 的 MOT 下標
    bool                       doorClosed;   // CheckSafeDoorIsClosed()
    bool                       canRun;       // IsMotorCanRun
    std::set<int>              locked;
    std::map<int, int>         readPos;      // 非 1203：MOT.ReadPos()
    int                        moveRet;      // 非 1203：GoldenMotorMove 的回傳
    struct Jog { int mi; bool pos; int pct; };
    struct Move { int mi; int target; int pct; bool setSpeed; };
    struct Param { int mi; MotorParamWhich which; int value; };
    std::vector<Jog>   jogs;
    std::vector<Move>  moves;
    std::vector<Param> params;

    bool Pci1203CmdPos(int ax, double& card) override
    {
        std::map<int, double>::const_iterator it = cmdPos.find(ax);
        if (it == cmdPos.end()) return false;
        card = it->second;
        return true;
    }
    bool Pci1203AxisReady(int ax) override { return notReady.count(ax) == 0; }
    bool GoldenMotor(int mi, MotorGolden& g) override
    {
        std::map<int, MotorGolden>::const_iterator it = golden.find(mi);
        if (it == golden.end()) return false;
        g = it->second;
        return true;
    }
    bool GoldenSafeDoorOpen(int mi) override { return doorOpen.count(mi) != 0; }
    bool GoldenSafeDoorClosed() override { return doorClosed; }
    bool GoldenMotorCanRun() override { return canRun; }
    bool GoldenMoveLocked(int mi) override { return locked.count(mi) != 0; }
    int  GoldenReadPos(int mi) override { return readPos.count(mi) ? readPos[mi] : 0; }
    void GoldenJog(int mi, bool pos, int pct) override { Jog j = { mi, pos, pct }; jogs.push_back(j); }
    int  GoldenMotorMove(int mi, int target, int pct, bool setSpeed) override
    {
        Move m = { mi, target, pct, setSpeed }; moves.push_back(m); return moveRet;
    }
    void GoldenSetParam(int mi, MotorParamWhich which, int value) override
    {
        Param p = { mi, which, value }; params.push_back(p);
        std::map<int, MotorGolden>::iterator it = golden.find(mi);            // AI(W906-MT-E2) 20260925: opt-in (older sections keep golden[] as set)
        if (!paramsUpdateGolden || it == golden.end()) return;
        switch (which) {
            case kParamJogHigh:  it->second.jogHigh  = (unsigned)value; break;
            case kParamJogLow:   it->second.jogLow   = (unsigned)value; break;
            case kParamHomeHigh: it->second.homeHigh = (unsigned)value; break;
            case kParamHomeLow:  it->second.homeLow  = (unsigned)value; break;
            case kParamSoftP:    it->second.softP    = value; break;
            case kParamSoftN:    it->second.softN    = value; break;
        }
    }
    int AllCalls() const { return Calls() + (int)(jogs.size() + moves.size() + params.size()); }

    // ---- W4-b2 ----
    std::map<int, unsigned> state;           // 1203 軸槽 → STA_AX_*（沒有 = 讀不到）
    std::map<int, int>      homeFlags;       // GoldenSetHomeFlag 寫到哪
    int                     zSafe;
    bool                    systemStart;
    int                     resetCount;
    struct RR { int mi; bool range; int v; };
    std::vector<RR>         rangeRates;
    bool Pci1203AxisState(int ax, unsigned& st) override
    {
        std::map<int, unsigned>::const_iterator it = state.find(ax);
        if (it == state.end()) return false;
        st = it->second;
        return true;
    }
    void GoldenSetHomeFlag(int mi, int v) override
    {
        homeFlags[mi] = v;
        std::map<int, MotorGolden>::iterator it = golden.find(mi);
        if (it != golden.end()) it->second.homeFlag = v;
    }
    int  GoldenZSafePos() override { return zSafe; }
    std::map<int, double> speedNow;   //AI(W906-JOG-MAXVEL) 20261003: the card's current speed params the monitor "read" (empty = unknown)
    bool Pci1203SpeedNow(int axis, int which, double& v) override { (void)axis; std::map<int, double>::iterator it = speedNow.find(which); if (it == speedNow.end()) return false; v = it->second; return true; }
    bool GoldenSystemStart() override { return systemStart; }  std::vector<std::string> zOrgAlarms; std::vector<bool> zOrgAlarmJobActive;  void GoldenArmCellZOrgAlarm(const std::string& z, const std::string& what) override { zOrgAlarms.push_back(z + "|" + what); zOrgAlarmJobActive.push_back(MotorAccessJobs().cellActive); }   //AI(W906-ARMCELL-ZALARM) 20261003: NB2-1 -- the WAR16442 box, recorded with whether the job was still running then
    void GoldenResetMNet() override { ++resetCount; }
    void GoldenSetRangeRate(int mi, bool range, int v) override { RR x = { mi, range, v }; rangeRates.push_back(x); }

    // ---- NB2 R21 ----
    unsigned long polls;          // 監看器輪詢序號：預設每次被問就前進（＝兩個命令之間有輪詢）；freezePolls 時不動
    bool          freezePolls;
    bool          floodReady;
    std::vector<int> floodCalls;
    unsigned      readSpeedV;
    unsigned long retNext;        // 非 0：下一次 Execute accepted＋issued 但卡片回這個錯誤碼
    unsigned long Pci1203PollCount() override { return freezePolls ? polls : ++polls; }
    bool GoldenShuttleFloodgateReady(int which) override { floodCalls.push_back(which); return floodReady; }
    unsigned GoldenReadSpeed(int) override { return readSpeedV; }

    // ---- NB2 R23 ----
    bool safeLock;
    bool GoldenSafeLockActive() override { return safeLock; }
    int  driveKind = 1;                                                         // AI(W906-MT-E1) 20260925: 1 DS402, 0 other, -1 unknown
    int  Pci1203DriveKind(int) override { return driveKind; }
    int  clearAllHomeFlagsCalls = 0;                                            // AI(W906-MT-E1) 20260925
    std::size_t clearAllAtExecuted = 0;                                         // executed.size() when it was called
    void GoldenClearAllHomeFlags() override { ++clearAllHomeFlagsCalls; clearAllAtExecuted = executed.size(); }

    // ---- AI(W906-MT-E2) 20260925 ----
    bool paramsUpdateGolden = false;          // GoldenSetParam also writes golden[] (the MT-E2 acks read it back as "cur")
    struct SpeedCall { int mi; int pct; bool jog; };
    std::vector<SpeedCall> speedCalls;
    void GoldenSetSpeed(int mi, int pct, bool jog) override
    {
        SpeedCall s = { mi, pct, jog }; speedCalls.push_back(s);
        setSpeeds.push_back(std::make_pair(mi, pct));                          // AI(W906-MERGE-56bbf785): the laptop's W5-b log of the same call (its tests read it)
        std::map<int, MotorGolden>::iterator it = golden.find(mi);            // TMyMotor::SetSpeed -> Motor->SetSpeed(s): iSpeed = s
        if (it != golden.end() && !it->second.indexMotor) it->second.readSpeed = (int)MotorSpeedFromPct(pct, it->second).s;
    }
    struct CellCall { int mi; int row; double v; };
    std::vector<CellCall> cells;
    void GoldenSetCell(int mi, int row, double v) override
    {
        CellCall c = { mi, row, v }; cells.push_back(c);
        std::map<int, MotorGolden>::iterator it = golden.find(mi);
        if (it == golden.end()) return;
        MotorGolden& g = it->second;
        const int iv = (row == 8 || row == 9) ? 0 : (int)v;
        switch (row) {
            case 1: g.initSpeed = (unsigned)iv; break;  case 2: g.jogHigh = (unsigned)iv; break;
            case 3: g.jogLow = (unsigned)iv; break;     case 4: g.homeHigh = (unsigned)iv; break;
            case 5: g.homeLow = (unsigned)iv; break;    case 6: g.softP = iv; break;
            case 7: g.softN = iv; break;                case 8: g.accDb = v; break;
            case 9: g.decDb = v; break;                 case 10: g.range = ((unsigned)iv > 1000u) ? 1000u : (unsigned)iv; break;
        }
    }
    std::map<int, int> lastHome;                                              // GoldenSetLastHomePos
    std::size_t lastHomeAtExecuted = 0;
    void GoldenSetLastHomePos(int mi, int v) override
    {
        lastHome[mi] = v; lastHomeAtExecuted = executed.size();
        std::map<int, MotorGolden>::iterator it = golden.find(mi);
        if (it != golden.end()) it->second.lastHomePos = v;
    }
    std::map<int, unsigned long> motionIO;                                    // 1203 軸槽 → motionIO（沒有 = 讀不到）（W5-b 也用：ORG bit 4）
    bool Pci1203MotionIO(int ax, unsigned long& io) override
    {
        std::map<int, unsigned long>::const_iterator it = motionIO.find(ax);
        if (it == motionIO.end()) return false;
        io = it->second;
        return true;
    }
    std::map<int, bool> alarmLed;                                             // 非 1203：MOT.Led[iAlarmLed]（沒有 = 讀不到）
    bool GoldenAlarmLed(int mi, bool& known) override
    {
        std::map<int, bool>::const_iterator it = alarmLed.find(mi);
        known = (it != alarmLed.end());
        return known ? it->second : false;
    }
    MotorReloadResult reloadResult;
    int               reloadCalls = 0, reloadAtClearCalls = -1;
    std::size_t       reloadAtExecuted = 0;
    MotorReloadResult GoldenReloadMotorParams() override
    {
        ++reloadCalls; reloadAtClearCalls = clearAllHomeFlagsCalls; reloadAtExecuted = executed.size();
        return reloadResult;
    }
    double clockMs = -1.0;                                                    // < 0: no clock (the loop counts the beats)
    double MonotonicMs() override { return clockMs; }

    // ---- AI(W906-MT-E3c) 20260925 ----
    bool relayOut = false;                   // SW[SwMotorRelay].OutValue
    int  relayCard = -1;                     // the monitor's DO read-back of that bit: -1 none, 0/1
    bool powerState = false;                 // bMotorPowerState
    int  powerOnBegins = 0, powerOnSteps = 0, powerOnStepsToDone = 2, serverOns = 0;
    std::vector<std::string> servoOffs;      // GaliMotorServoOff(sFunc)
    std::vector<std::string> order;          // the power calls in order
    MotorPowerState GoldenMotorPower(bool sync) override
    {
        MotorPowerState s;
        s.cardKnown = relayCard >= 0; s.card = relayCard == 1;
        if (sync && s.cardKnown && relayOut != s.card) { relayOut = s.card; s.synced = true; }
        s.relayOn = relayOut; s.motorPowerState = powerState;
        return s;
    }
    void GoldenMotorPowerOnBegin() override { ++powerOnBegins; order.push_back("begin"); relayOut = true; if (relayCard >= 0) relayCard = 1; }
    bool GoldenMotorPowerOnStep() override { ++powerOnSteps; order.push_back("step"); return powerOnSteps >= powerOnStepsToDone; }
    void GoldenServerOn() override { ++serverOns; order.push_back("serverOn"); powerState = true; }
    void GoldenMotorServoOff(const std::string& f) override
    {
        servoOffs.push_back(f); order.push_back("servoOff:" + std::to_string(executed.size()));
        relayOut = false; if (relayCard >= 0) relayCard = 0; powerState = false;
    }
    std::string GoldenRouteLastWrite() override { return "fake route"; }
    std::map<int, unsigned> rangeMem;  std::map<int, double> accMem;
    void GoldenSetRangeMemory(int mi, unsigned v) override
    {
        rangeMem[mi] = v;
        std::map<int, MotorGolden>::iterator it = golden.find(mi);
        if (it != golden.end()) it->second.range = v > 1000u ? 1000u : v;    // TMyEtherCatMotor::SetRange
    }
    void GoldenSetAccMemory(int mi, double a) override
    {
        accMem[mi] = a;
        std::map<int, MotorGolden>::iterator it = golden.find(mi);
        if (it != golden.end()) it->second.acc = a;                          // the runtime dAcc (CFG_AxMaxAcc reads it)
    }
    bool emgOff = false;
    bool GoldenInitMotorEmgOff() override { return emgOff; }
    int planClass = -1; bool planSensor = false, planIn1 = false;              // what the dispatcher asked the plan for
    std::vector<InitCfg> GoldenInitCfgPlan(const MotorGolden& g) override      // a 3-entry stand-in (the real table: test_pci1203_pure)
    {
        planClass = g.motorClass; planSensor = g.sensorType; planIn1 = g.in1Logic;
        std::vector<InitCfg> v;
        InitCfg a = { kInitCfgPPU, 1.0, "CFG_AxPPU" }, b = { kInitCfgOrgLogic, g.sensorType ? 0.0 : 1.0, "CFG_AxOrgLogic" },
                c = { kInitCfgJerk, 0.0, "PAR_AxJerk" };
        v.push_back(a); v.push_back(b); v.push_back(c);
        return v;
    }
    std::vector<int> sleeps;
    void SleepMs(int ms) override { sleeps.push_back(ms); }
    //AI(W906-BRAKE-AXIS) 20260929: the axis whose brake output is released ("" = no brake anywhere, the default)
    std::string brakeAlias; bool brakeReleased = false; int brakeHoldAt = -1;
    bool BrakeHoldBeforeServoOff(const std::string& alias, std::string& note) override
    {
        note.clear();
        if (alias.empty() || alias != brakeAlias) return false;
        const bool was = brakeReleased;
        brakeReleased = false; brakeHoldAt = (int)executed.size(); note = "brake held before servo off";
        return was;
    }
    int initMotParamCalls = 0;
    void GoldenInitMOTParameterAll() override { ++initMotParamCalls; }
    int formCloses = 0;
    void GoldenFormClose() override { ++formCloses; }
    std::map<int, std::string> aliasOf;      // MOT index -> Alias
    std::string AliasOfMotor(int mi) override { return aliasOf.count(mi) ? aliasOf[mi] : std::string(); }
    int  lsEncoder = 0; bool lsEncFromMonitor = false;
    int GoldenLightScaleEncoder(std::string& src, bool& fromMonitor) override
    {
        fromMonitor = lsEncFromMonitor; src = lsEncFromMonitor ? "pci1203" : "MOT"; return lsEncoder;
    }
    std::vector<std::string> lsData[9]; int lsCount[9] = { 0 };
    int lsCountsResets = 0;
    int GoldenLightScaleDataCount(int k) override { return (k >= 1 && k <= 8) ? lsCount[k] : 0; }
    std::vector<std::string> GoldenLightScaleData(int k) override { return (k >= 1 && k <= 8) ? lsData[k] : std::vector<std::string>(); }
    void GoldenLightScaleDataClear(int k) override { if (k >= 1 && k <= 8) lsData[k].clear(); }
    void GoldenLightScaleCountsReset() override { ++lsCountsResets; for (int k = 1; k <= 8; ++k) lsCount[k] = 0; }
    std::string lsRoot;
    std::string LightScaleRoot() override { return lsRoot; }
    std::string LocalStampYmdhm() override { return "202609251234"; }

    // ---- W5-b ----
    //AI(W906-W5-b) 20260925: W5B-11 —— 登錄表不再手寫：reg 由 BuildTeachRegistry() 從真的 WebTeachButtons.gen.inc 依組態條件建
    //AI(W906-MERGE-56bbf785) 20260926: machine MT-E1..E3c members above + the laptop's W5-b members here. The two maps both sides
    //  declared -- aliasOf (MOT index -> Alias: machine AliasOfMotor, laptop AliasOfMotIndex) and motionIO -- are declared once above;
    //  the laptop's two-argument GoldenSetSpeed is the machine's (mi, pct, jog) (it also fills setSpeeds).
    TeachRegistry    reg;
    bool             regOk;
    bool             canTeach, powerOff, indexArm3, ep1;
    std::vector<int> teachCanMoveCalls;
    std::map<int, int>    encPos;           // 非 1203 ReadEncoderPos
    std::map<int, double> actPos;           // 1203 監看器 actPos
    std::set<int>    usesReadPos;           // golden GetTechPos 的 Contec＋MotorType==0
    std::map<int, bool> lastDirP;           // MOT[i].iLastRotatorDirP（沒有 = golden 初值 true，mymotor.cpp:189）
    int              prodBacklashIn, prodBacklashOut;
    std::vector<std::pair<int, int> > setSpeeds;   // 非 1203 GoldenSetSpeed
    std::map<std::pair<int, int>, int> svFail;     // (軸槽, 0／1) → kCmdAxSvOn 還要被拒幾次
    int              failSetSpeed;          // kCmdAxSetSpeed 還要被拒幾次
    bool GoldenTeachRegistry(TeachRegistry& out, std::string& why) override
    {
        if (!regOk) { why = "fake: registry mismatch"; return false; }
        out = reg; return true;
    }
    bool GoldenTeachCanMove(int mi) override { teachCanMoveCalls.push_back(mi); if (popupInTeachCanMove) popupDuringCheck = MotorAccessArmCellOnPopup(*this, "訊息框", "Z axis not at home");  return canTeach; }   //AI(W906-ARMCELL2) 20261002: golden CheckCanMove pops its own message (ShowMyMessage) -- the host's edge call, as wb_serve makes it
    bool GoldenMotorPowerOff() override { return powerOff; }
    int  GoldenReadEncoderPos(int mi) override { return encPos.count(mi) ? encPos[mi] : 0; }
    bool GoldenTechPosUsesReadPos(int mi) override { return usesReadPos.count(mi) != 0; }
    bool GoldenRotatorLastDirP(int mi) override { return lastDirP.count(mi) ? lastDirP[mi] : true; }
    void GoldenSetRotatorLastDirP(int mi, bool p) override { lastDirP[mi] = p; }
    int  GoldenProdRotatorBacklash(bool in) override { return in ? prodBacklashIn : prodBacklashOut; }
    bool GoldenIndexArm3Axis() override { return indexArm3; }
    int  GoldenTeachRemap(int mi) override { return (ep1 && (mi == 7 || mi == 26)) ? mi - 4 : mi; }   // 同 Live（golden uteach.cpp:3372／:3409）
    bool AliasOfMotIndex(int mi, std::string& a) override
    {
        std::map<int, std::string>::const_iterator it = aliasOf.find(mi);
        if (it == aliasOf.end()) return false;
        a = it->second; return true;
    }
    bool Pci1203ActPos(int ax, double& card) override
    {
        std::map<int, double>::const_iterator it = actPos.find(ax);
        if (it == actPos.end()) return false;
        card = it->second; return true;
    }
    int              clearAllHome;          // AI(W906-W5-b) 20260925: W5B-R3 GoldenClearAllMotorHome 被叫幾次
    void GoldenClearAllMotorHome() override { ++clearAllHome; }
    //AI(W906-MT-SAVEMOT) 20260930: saveMotTable -- the file is a temp copy the test writes; MotTableRowSaved is recorded.
    std::string motTablePath;
    bool        motIndexForced = false;
    std::vector<std::string> motSaved;      // "alias|rowText|col=new;col=new;"
    std::string MotTableFilePath() override { return motTablePath; }
    bool MotTableIndexForced() override { return motIndexForced; }
    void MotTableRowSaved(const std::string& alias, const std::string& rowText,
                          const std::vector<std::pair<std::string, std::string> >& cells) override
    {
        std::string s = alias + "|" + rowText + "|";
        for (std::size_t i = 0; i < cells.size(); ++i) s += cells[i].first + "=" + cells[i].second + ";";
        motSaved.push_back(s);
    }
    //AI(W906-TEACH-ZALLUP) 20261001: golden uteach's fixed motors ("inZ" / "outZ" / "z1" / "z2" / "arm1y" -> MOT indexes); a missing key
    //  = the interface default (false: "this backend has none"), as the older fakes see it.
    std::map<std::string, std::vector<int> > fixedMots;
    bool GoldenTeachFixedMotors(const std::string& which, std::vector<int>& mis, std::string& why) override
    {
        std::map<std::string, std::vector<int> >::const_iterator it = fixedMots.find(which);
        if (it == fixedMots.end()) return IMotorAccessBackend::GoldenTeachFixedMotors(which, mis, why);
        mis = it->second;
        return true;
    }
    //AI(W906-ARMCELL) 20261002: the Teach Arm Cell tab (RULINGS_20261002 #18) -- a canned catalog / plan (the real ones: ArmCellPlan.cpp,
    //  ctest ArmCellPlan); not set = the interface default (false: "this backend has no Arm Cell"), as the older fakes see it.
    //  rejectMoveAxes: Execute refuses a kCmdAxMoveAbs on these 1203 axes (S3 "Y cannot be issued after X was").
    bool cellHasCatalog = false;  ArmCellCatalog cellCatalog;
    bool cellHasPlan = false;     ArmCellPlan cellPlan;  std::string cellPlanWhy;  std::vector<ArmCellRequest> cellReqs;
    std::set<int> rejectMoveAxes;
    bool popupInTeachCanMove = false, popupDuringCheck = false;   //AI(W906-ARMCELL2) 20261002: GoldenTeachCanMove raises a box (golden CheckCanMove's ShowMyMessage)
    bool GoldenArmCellCatalog(ArmCellCatalog& out, std::string& why) override
    {
        if (!cellHasCatalog) return IMotorAccessBackend::GoldenArmCellCatalog(out, why);
        out = cellCatalog;
        return true;
    }
    bool GoldenArmCellPlan(const ArmCellRequest& q, ArmCellPlan& out, std::string& why) override
    {
        cellReqs.push_back(q);
        if (!cellHasPlan) return IMotorAccessBackend::GoldenArmCellPlan(q, out, why);
        if (!cellPlanWhy.empty()) { why = cellPlanWhy; return false; }
        out = cellPlan;
        return true;
    }
    //AI(W906-GEARRATIO) 20261002: the Motor Test Gear Ratio tab (GearRatioBackend.h). gearOn=false = the interface defaults (they refuse),
    //  as every older fake sees them. The teach registry is gearRefs (its pointers point at the test's ints); the "teach.ini" is a real
    //  scratch file at gearTeachPath that GearTeachSaveReload writes the way the live writer does (every slot from its variable) and
    //  reads back the way fTeach->ReadFile does (TECH_* first, elTeach last) -- with three injectable faults.
    bool gearOn = false;
    std::map<int, int> gearArm;                   // MOT index -> 1 (In arm X / Y), 2 (Out); missing = 0
    bool gearEmg = false, gearTeachOpen = false, gearWriterFail = false;
    std::string gearWriterSkipKey;                // the writer writes this key with its OLD value (ReadFile then reads the old one back)
    std::string gearWriterExtraKey;               // the writer also changes this key (to 4242)
    std::string gearWriterWrongKey;               // the writer writes this key with its value + 1 (a plan key: a value the plan did not ask for)   //AI(W906-GEARRATIO2) 20261003
    std::vector<GearTeachRef> gearRefs;
    std::string gearTeachPath, gearLogDir;
    int gearSetMotorCalls = 0, gearSetTeachCalls = 0, gearSaveCalls = 0, gearReloadCalls = 0;
    std::map<const int*, int> gearOldAtSave;      // the value of each registry variable when the writer last saved (skip-key fault)
    int GearArmOf(int mi) override
    {
        if (!gearOn) return IMotorAccessBackend::GearArmOf(mi);
        std::map<int, int>::const_iterator it = gearArm.find(mi);
        return it == gearArm.end() ? 0 : it->second;
    }
    bool GearEmgStop(std::string& w) override
    {
        if (!gearOn) return IMotorAccessBackend::GearEmgStop(w);
        if (gearEmg) { w = "fake EMG pressed"; return true; }
        w.clear(); return false;
    }
    bool GearTeachWindowOpen(std::string& w) override
    {
        if (!gearOn) return IMotorAccessBackend::GearTeachWindowOpen(w);
        w = gearTeachOpen ? "fake: the Teach window is open" : ""; return gearTeachOpen;
    }
    bool GearTeachRefs(std::vector<GearTeachRef>& o, std::string& w) override
    {
        if (!gearOn) return IMotorAccessBackend::GearTeachRefs(o, w);
        o = gearRefs;
        for (std::size_t i = 0; i < o.size(); ++i) o[i].value = *o[i].ptr;
        return true;
    }
    std::string GearTeachIniPath() override { return gearOn ? gearTeachPath : std::string(); }
    bool GearSetMotor(int mi, double r, int p, int n, std::string& w) override
    {
        if (!gearOn) return IMotorAccessBackend::GearSetMotor(mi, r, p, n, w);
        ++gearSetMotorCalls;
        std::map<int, MotorGolden>::iterator it = golden.find(mi);
        if (it == golden.end()) { w = "fake: no MOT object"; return false; }
        it->second.gearRatio = r; it->second.softP = p; it->second.softN = n;
        return true;
    }
    bool GearSetTeach(const std::vector<std::pair<const int*, int> >& v, std::string& w) override
    {
        if (!gearOn) return IMotorAccessBackend::GearSetTeach(v, w);
        ++gearSetTeachCalls;
        for (std::size_t i = 0; i < v.size(); ++i) {
            if (!gearOldAtSave.count(v[i].first)) gearOldAtSave[v[i].first] = *v[i].first;   // the value before the first write
            *const_cast<int*>(v[i].first) = v[i].second;
        }
        return true;
    }
    std::string GearIniText(bool applyFaults)                 // every slot from its variable, sections in first-seen order
    {
        std::vector<std::string> secs;
        std::map<std::string, std::vector<std::pair<std::string, int> > > keys;
        for (std::size_t i = 0; i < gearRefs.size(); ++i) {
            const GearTeachRef& r = gearRefs[i];
            if (!keys.count(r.section)) secs.push_back(r.section);
            int v = *r.ptr;
            if (applyFaults && r.key == gearWriterSkipKey && gearOldAtSave.count(r.ptr)) v = gearOldAtSave[r.ptr];
            if (applyFaults && r.key == gearWriterExtraKey) v = 4242;
            if (applyFaults && r.key == gearWriterWrongKey) v = v + 1;
            keys[r.section].push_back(std::make_pair(r.key, v));
        }
        std::string t;
        for (std::size_t s = 0; s < secs.size(); ++s) {
            t += "[" + secs[s] + "]\r\n";
            const std::vector<std::pair<std::string, int> >& k = keys[secs[s]];
            for (std::size_t i = 0; i < k.size(); ++i) t += k[i].first + "=" + std::to_string(k[i].second) + "\r\n";
        }
        return t;
    }
    bool GearTeachSaveReload(bool save, std::string& w, std::string& note) override
    {
        if (!gearOn) return IMotorAccessBackend::GearTeachSaveReload(save, w, note);
        if (save) {
            ++gearSaveCalls;
            if (gearWriterFail) { w = "fake: the teach writer failed"; return false; }
            std::ofstream f(gearTeachPath.c_str(), std::ios::binary | std::ios::trunc);
            const std::string t = GearIniText(true);
            f.write(t.data(), (std::streamsize)t.size());
            if (!f.good()) { w = "fake: cannot write teach.ini"; return false; }
        } else {
            ++gearReloadCalls;
        }
        // fTeach->ReadFile: TECH_* slots first, elTeach last (elTeach wins on a shared variable)
        std::ifstream g(gearTeachPath.c_str(), std::ios::binary);
        std::stringstream ss; ss << g.rdbuf();
        const GearIni ini = GearIniParse(ss.str());
        for (int pass = 0; pass < 2; ++pass)
            for (std::size_t i = 0; i < gearRefs.size(); ++i) {
                const GearTeachRef& r = gearRefs[i];
                if ((r.list == "elTeach") != (pass == 1)) continue;
                std::string sl = r.section, kl = r.key;
                for (std::size_t q = 0; q < sl.size(); ++q) if (sl[q] >= 'A' && sl[q] <= 'Z') sl[q] = (char)(sl[q] - 'A' + 'a');
                for (std::size_t q = 0; q < kl.size(); ++q) if (kl[q] >= 'A' && kl[q] <= 'Z') kl[q] = (char)(kl[q] - 'A' + 'a');
                GearIni::const_iterator s = ini.find(sl);
                if (s == ini.end()) continue;
                std::map<std::string, GearIniEntry>::const_iterator k = s->second.find(kl);
                if (k != s->second.end()) *const_cast<int*>(r.ptr) = std::atoi(k->second.value.c_str());
            }
        note = "{\"todo\":[\"fake\"]}";
        return true;
    }
    std::string GearCalLogDir() override { return gearOn ? gearLogDir : std::string(); }
    std::string GearOperator() override { return "fake operator"; }
    std::string GearStamp() override { return gearOn ? std::string("20261002_120000") : std::string(); }
};

static MotorAccessAxis Axis1203(int mi, int board, int port, int axisSlot, const char* why = "")
{
    MotorAccessAxis a;
    a.motIndex = mi; a.cardModel = "PCI1203"; a.boardId = board; a.port = port;
    a.axis = axisSlot; a.why = why; a.motorLive = true;
    return a;
}
static MotorAccessAxis AxisOther(int mi, const char* card, bool live)
{
    MotorAccessAxis a;
    a.motIndex = mi; a.cardModel = card; a.motorLive = live;
    return a;
}

static MotorAccessReq Req(const char* source, const char* action, const char* motor)
{
    MotorAccessReq r;
    r.seq = 7; r.id = "cmd-7"; r.source = source; r.action = action; r.button = "btn";
    r.kind = "control";
    if (motor) r.motors.push_back(motor);
    return r;
}
// NB2 R23 W4C-5：home／loopMove 帶「按下後 Down 的值」
static MotorAccessReq ReqStart(const char* source, const char* action, const char* motor, bool start)
{
    MotorAccessReq r = Req(source, action, motor);
    r.flag["start"] = start;
    return r;
}

// ack 必須是 JSON 物件，而且 state/seq 照 Steven 的 ack 形狀；不可帶傳輸層保留字（type/id/ok/error），請求 id 放 reqId
static bool AckShapeOk(const std::string& ack, long long seq)
{
    cJSON* j = cJSON_Parse(ack.c_str());
    bool ok = j && cJSON_IsObject(j);
    if (ok) {
        const cJSON* st = cJSON_GetObjectItemCaseSensitive(j, "state");
        const cJSON* sq = cJSON_GetObjectItemCaseSensitive(j, "seq");
        ok = cJSON_IsString(st) && std::string(st->valuestring) == "done" &&
             cJSON_IsNumber(sq) && (long long)sq->valuedouble == seq;
        // 傳輸層保留字：WebBridgeServer::AckJson 把這個物件攤平併進 WS ack，撞名會蓋掉 WS 的 id（20260925 探針量到）
        const char* reserved[] = { "type", "id", "ok", "error" };
        for (unsigned k = 0; ok && k < 4; ++k)
            if (cJSON_GetObjectItemCaseSensitive(j, reserved[k]) != 0) ok = false;
        const cJSON* rid = cJSON_GetObjectItemCaseSensitive(j, "reqId");
        if (ok && !cJSON_IsString(rid)) ok = false;
    }
    cJSON_Delete(j);
    return ok;
}

// ---------------------------------------------------------------------------
//  AI(W906-W5-b) 20260925: W5B-11 —— 真的 WebTeachButtons.gen.inc（golden 建構子登錄表的 312 列＋條件）。
//  馬達：golden 列舉名 → 假的 MOT 下標（TestMot；可先種值）；條件：atoms 字串依測試給的組態求值（Live 是 C++ 式在機台上求值）。
// ---------------------------------------------------------------------------
static std::map<std::string, int>& TestMotMap() { static std::map<std::string, int> m; return m; }
static int TestMot(const char* name)
{
    std::map<std::string, int>& m = TestMotMap();
    std::map<std::string, int>::const_iterator it = m.find(name);
    if (it != m.end()) return it->second;
    const int v = 100 + (int)m.size();
    m[name] = v;
    return v;
}
struct GenRow { char owner; const char* atoms; int m0; const char* n0; int m1; const char* n1;
                const char* k0; const char* k1; const char* e0; const char* e1; const char* sb; const char* sh; const char* gb; const char* gh; int vis; };
struct GenOvr { const char* btn; const char* axle; int m; const char* n; };
struct GenBtn { const char* btn; const char* handler; int tag; int dfmVis; const char* selfOther; const char* hiddenWhy; int lateReg; };   // AI(W906-W5-b) 20260925: R-W5B-2／3
static const std::vector<GenRow>& GenRows()
{
    static std::vector<GenRow> v;
    if (v.empty()) {
#define W5B_COND(e)  0
#define W5B_PTR(x)   0
#define W5B_NOPTR    0
#define W5B_MOT(x)   TestMot(#x), #x
#define W5B_NOMOT    -1, ""
#define W5B_ROW(o, live, atoms, p0, p1, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis) { GenRow g = { o, atoms, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis }; v.push_back(g); }
#define W5B_TAGOVR(b, a, m)
#define W5B_BTN(b, h, tag, dv, so, hw, lr)
#include "WebTeachButtons.gen.inc"
    }
    return v;
}
static const std::vector<GenOvr>& GenOvrs()
{
    static std::vector<GenOvr> v;
    if (v.empty()) {
#define W5B_COND(e)  0
#define W5B_PTR(x)   0
#define W5B_NOPTR    0
#define W5B_MOT(x)   TestMot(#x), #x
#define W5B_NOMOT    -1, ""
#define W5B_ROW(o, live, atoms, p0, p1, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis)
#define W5B_TAGOVR(b, a, m) { GenOvr g = { b, a, m }; v.push_back(g); }
#define W5B_BTN(b, h, tag, dv, so, hw, lr)
#include "WebTeachButtons.gen.inc"
    }
    return v;
}
static const std::vector<GenBtn>& GenBtns()
{
    static std::vector<GenBtn> v;
    if (v.empty()) {
#define W5B_COND(e)  0
#define W5B_PTR(x)   0
#define W5B_NOPTR    0
#define W5B_MOT(x)   TestMot(#x), #x
#define W5B_NOMOT    -1, ""
#define W5B_ROW(o, live, atoms, p0, p1, m0, m1, k0, k1, e0, e1, sb, sh, gb, gh, vis)
#define W5B_TAGOVR(b, a, m)
#define W5B_BTN(b, h, tag, dv, so, hw, lr) { GenBtn g = { b, h, tag, dv, so, hw, lr }; v.push_back(g); }
#include "WebTeachButtons.gen.inc"
    }
    return v;
}
static bool RowLive(const GenRow& g, const std::set<std::string>& on)
{
    const std::string s = g.atoms;
    std::size_t p = 0;
    while (!s.empty()) {
        const std::size_t q = s.find(';', p);
        std::string a = s.substr(p, q == std::string::npos ? std::string::npos : q - p);
        const bool neg = !a.empty() && a[0] == '!';
        if (neg) a = a.substr(1);
        if ((on.count(a) != 0) == neg) return false;
        if (q == std::string::npos) break;
        p = q + 1;
    }
    return true;
}
static TeachRegistry BuildTeachRegistry(const std::set<std::string>& on)
{
    TeachRegistry r;
    const std::vector<GenRow>& rows = GenRows();
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const GenRow& g = rows[i];
        if (!RowLive(g, on)) continue;
        TeachRegRow t;
        t.owner = g.owner; t.mot0 = g.m0; t.mot1 = g.m1; t.name0 = g.n0; t.name1 = g.n1; t.key0 = g.k0; t.key1 = g.k1;
        t.edit0 = g.e0; t.edit1 = g.e1; t.setBtn = g.sb; t.setHandler = g.sh; t.goBtn = g.gb; t.goHandler = g.gh;
        t.seq = (int)i; t.vis = g.vis != 0;                                      // AI(W906-W5-b) 20260925: R-W5B-2
        (g.owner == 'P' ? r.P : r.T).push_back(t);
    }
    const std::vector<GenOvr>& ov = GenOvrs();
    for (std::size_t i = 0; i < ov.size(); ++i) r.selectOnly[ov[i].btn] = std::make_pair(ov[i].m, std::string(ov[i].n));
    const std::vector<GenBtn>& bt = GenBtns();
    for (std::size_t i = 0; i < bt.size(); ++i) {
        TeachBtnInfo b;
        b.handler = bt[i].handler; b.dfmTag = bt[i].tag; b.dfmVisible = bt[i].dfmVis != 0;
        b.selfOther = bt[i].selfOther; b.hiddenWhy = bt[i].hiddenWhy; b.lateReg = bt[i].lateReg != 0;
        r.btns[bt[i].btn] = b;
    }
    return r;
}
// 一個 1203 軸：golden 列舉名 → MOT 下標（TestMot）、Mot_Table 別名同名、監看器軸槽 slot
static int Add1203(FakeBackend& be, const char* name, int slot, const MotorGolden& g)
{
    const int mi = TestMot(name);
    be.table[name] = Axis1203(mi, 10 + slot, 0, slot);
    be.golden[mi] = g; be.aliasOf[mi] = name;
    if ((int)be.opened.size() <= slot) be.opened.resize(slot + 1, true);
    be.cmdPos[slot] = 0.0; be.actPos[slot] = 0.0; be.state[slot] = 1;
    return mi;
}
static MotorAccessReq TeachReq(const char* action, const char* btn, bool hasStart, bool start)
{
    MotorAccessReq r = Req("uteach", action, 0);
    r.button = std::string(action) == "teachSet" ? "SetButton*" : "GoButton*";
    r.str["btn"] = btn;
    if (hasStart) r.flag["start"] = start;
    //AI(W906-TEACH-SVON) 20261001: a teachSet start without params.hand is now the servo-on teach (EastSun 1001); the hand-teach
    //  cases below are the page's 手動教導 checkbox ticked -> hand=true. The servo-on case clears it (W906-TEACH-SVON block).
    if (hasStart && start && std::string(action) == "teachSet") r.flag["hand"] = true;
    return r;
}
// AI(W906-W5-b) 20260925: 最近一次送到卡上的「運轉速度」（kCmdAxSetSpeed＋kSpeedRun 的值；-1＝沒有）
static double LastRunSpeed(const FakeBackend& be)
{
    for (std::size_t i = be.executed.size(); i-- > 0; )
        if (be.executed[i].kind == kCmdAxSetSpeed && be.executed[i].speed == kSpeedRun) return be.executed[i].value;
    return -1.0;
}
// AI(W906-MERGE-56bbf785) 20260926: the same for the jog family (kSpeedJogRun = CFG_AxJogVelHigh, what Acm_AxJog runs on)
static double LastJogRunSpeed(const FakeBackend& be)
{
    for (std::size_t i = be.executed.size(); i-- > 0; )
        if (be.executed[i].kind == kCmdAxSetSpeed && be.executed[i].speed == kSpeedJogRun) return be.executed[i].value;
    return -1.0;
}
static int CountKind(const FakeBackend& be, std::size_t from, Pci1203CmdKind k)
{
    int n = 0;
    for (std::size_t i = from; i < be.executed.size(); ++i) if (be.executed[i].kind == k) ++n;
    return n;
}

int main(int argc, char** argv)
{
    //AI(W906-MEMO) 20260927: unbuffered stdout -- the sim gate timed this test out twice under `ctest -j 8` (159 s at 187 tests,
    //  126.94 s at 210) while it passes alone (0.82 s) and 64/64 in an 8-way parallel rerun; with a piped stdout fully buffered,
    //  the killed process lost its last ~6.6 KB, so the log could not show where it stopped. Now the last line IS the stall point.
    setvbuf(stdout, NULL, _IONBF, 0);
    // ---------------------------------------------------------------- (1)
    printf("[1] parse\n");
    {
        const std::string js =
            "{\"seq\":12,\"id\":\"cmd-12\",\"source\":\"uMotorTest\",\"button\":\"btnServoOff\","
            "\"action\":\"servoToggle\",\"kind\":\"control\",\"motors\":[\"MInArmX\"],"
            "\"params\":{\"speed\":5,\"currentPos\":-120,\"softLimitP\":null,\"servoOn\":true},"
            "\"issuedAt\":\"2026-09-25T00:00:00Z\",\"state\":\"requested\"}";
        MotorAccessReq r; std::string why;
        CHECK(MotorAccessParse(js, r, why), "motor-access.js request parses");
        CHECK(r.seq == 12 && r.id == "cmd-12" && r.source == "uMotorTest" && r.button == "btnServoOff" &&
              r.action == "servoToggle" && r.kind == "control", "scalar fields");
        CHECK(r.motors.size() == 1 && r.motors[0] == "MInArmX", "motors[]");
        CHECK(r.num.count("speed") && r.num["speed"] == 5 && r.num["currentPos"] == -120, "numeric params");
        CHECK(r.num.count("softLimitP") == 0, "null param is NOT stored as 0 (absent != zero)");
        CHECK(r.flag.count("servoOn") && r.flag["servoOn"] == true, "boolean params");

        MotorAccessReq bad;
        CHECK(!MotorAccessParse("not json", bad, why), "non-JSON refused");
        CHECK(!MotorAccessParse("[1,2]", bad, why), "array refused");
        CHECK(!MotorAccessParse("{\"source\":\"uMotorTest\"}", bad, why) && Has(why, "action"), "missing action refused");
        CHECK(!MotorAccessParse("{\"action\":\"stop\"}", bad, why) && Has(why, "source"), "missing source refused");
        CHECK(!MotorAccessParse("{\"action\":\"stop\",\"source\":\"uMotorTest\",\"motors\":\"MInArmX\"}", bad, why),
              "motors as string refused");
        CHECK(!MotorAccessParse("{\"action\":\"stop\",\"source\":\"uMotorTest\",\"motors\":[3]}", bad, why),
              "non-string motor refused");
    }

    // ---------------------------------------------------------------- (2)
    printf("[2] catalog coverage (web/JSON/motor-access.json)\n");
    if (argc < 2) {
        CHECK(false, "argv[1] = path to web/JSON/motor-access.json");
    } else {
        std::ifstream f(argv[1], std::ios::binary);
        std::stringstream ss; ss << f.rdbuf();
        cJSON* cat = cJSON_Parse(ss.str().c_str());
        const cJSON* cmds = cat ? cJSON_GetObjectItemCaseSensitive(cat, "commands") : 0;
        CHECK(cJSON_IsArray(cmds), "catalog has commands[]");
        int n = 0, unknown = 0;
        std::set<std::string> actions, live, ui;
        // AI(W906-MT-E2) 20260925: the contract with the web side -- five new commands, source uMotorTest, none of kind motion.
        struct Contract { const char* button; const char* action; const char* params; int seen; };
        Contract contract[5] = {
            { "labName",          "selectMotor",  "motorId",             0 },
            { "strngrdMotor",     "setParamCell", "motorId,row,value",   0 },
            { "BitBtn1",          "copyFrom",     "motorId,source",      0 },
            { "scrlbrMotorSpeed", "setSpeed",     "motorId,pct,jog",     0 },
            { "edtSpeed",         "setSpeed",     "motorId,pct,jog",     0 } };
        int contractBad = 0;
        // AI(W906-MT-E3c) 20260925: the MT-E3c contract rows (source uMotorTest; only BitBtn2 = lightScale is kind motion)
        struct Contract3 { const char* button; const char* action; const char* params; bool motion; int seen; };
        Contract3 contract3[6] = {
            { "btnMotorPower",            "motorPowerToggle",   "confirmOff",                        false, 0 },
            { "FormShow",                 "formShow",           "",                                  false, 0 },
            { "FormClose",                "formClose",          "",                                  false, 0 },
            { "BitBtn2",                  "lightScale",         "axisItem,moveType,pitch,delayMs",   true,  0 },
            { "BitBtn3",                  "lightScaleSave",     "axisItem,moveType",                 false, 0 },
            { "btnSaveLogLightScaleData", "lightScaleDataSave", "",                                  false, 0 } };
        const cJSON* c = 0;
        cJSON_ArrayForEach(c, cmds) {
            const cJSON* a = cJSON_GetObjectItemCaseSensitive(c, "action");
            if (!cJSON_IsString(a)) continue;
            ++n;
            const std::string act = a->valuestring;
            actions.insert(act);
            const std::string st = MotorAccessActionStatus(act);
            if (st == "unknown") { ++unknown; printf("    unknown action: %s\n", act.c_str()); }
            if (st == "live") live.insert(act);
            if (st == "ui") ui.insert(act);
            const cJSON* bt = cJSON_GetObjectItemCaseSensitive(c, "button");
            for (int k = 0; k < 5; ++k) {
                if (!cJSON_IsString(bt) || std::string(bt->valuestring) != contract[k].button) continue;
                ++contract[k].seen;
                const cJSON* src = cJSON_GetObjectItemCaseSensitive(c, "source");
                const cJSON* kd  = cJSON_GetObjectItemCaseSensitive(c, "kind");
                const cJSON* ps  = cJSON_GetObjectItemCaseSensitive(c, "params");
                std::string plist;
                const cJSON* p = 0;
                if (cJSON_IsArray(ps)) cJSON_ArrayForEach(p, ps) if (cJSON_IsString(p)) plist += (plist.empty() ? "" : ",") + std::string(p->valuestring);
                const bool okRow = act == contract[k].action && cJSON_IsString(src) && std::string(src->valuestring) == "uMotorTest" &&
                                   !(cJSON_IsString(kd) && std::string(kd->valuestring) == "motion") && plist == contract[k].params;
                if (!okRow) { ++contractBad; printf("    contract row %s: action=%s params=%s -- expected %s [%s], source uMotorTest, kind not motion\n",
                                                    contract[k].button, act.c_str(), plist.c_str(), contract[k].action, contract[k].params); }
            }
            for (int k = 0; k < 6; ++k) {                                       // AI(W906-MT-E3c) 20260925
                if (!cJSON_IsString(bt) || std::string(bt->valuestring) != contract3[k].button) continue;
                ++contract3[k].seen;
                const cJSON* src = cJSON_GetObjectItemCaseSensitive(c, "source");
                const cJSON* kd  = cJSON_GetObjectItemCaseSensitive(c, "kind");
                const cJSON* ps  = cJSON_GetObjectItemCaseSensitive(c, "params");
                std::string plist;
                const cJSON* p = 0;
                if (cJSON_IsArray(ps)) cJSON_ArrayForEach(p, ps) if (cJSON_IsString(p)) plist += (plist.empty() ? "" : ",") + std::string(p->valuestring);
                const bool isMotion = cJSON_IsString(kd) && std::string(kd->valuestring) == "motion";
                const bool okRow = act == contract3[k].action && cJSON_IsString(src) && std::string(src->valuestring) == "uMotorTest" &&
                                   isMotion == contract3[k].motion && plist == contract3[k].params;
                if (!okRow) { ++contractBad; printf("    contract row %s: action=%s params=%s kind motion=%d -- expected %s [%s] motion=%d\n",
                                                    contract3[k].button, act.c_str(), plist.c_str(), (int)isMotion, contract3[k].action,
                                                    contract3[k].params, (int)contract3[k].motion); }
            }
        }
        cJSON_Delete(cat);
        int contractSeen = 0;
        for (int k = 0; k < 5; ++k) { if (contract[k].seen == 1) ++contractSeen; else printf("    contract button %s seen %d time(s)\n", contract[k].button, contract[k].seen); }
        for (int k = 0; k < 6; ++k) { if (contract3[k].seen == 1) ++contractSeen; else printf("    contract button %s seen %d time(s)\n", contract3[k].button, contract3[k].seen); }
        CHECK(contractSeen == 11 && contractBad == 0,
              "AI(W906-MT-E2/E3c): the 5 + 6 contract commands are in the catalog exactly once, with the agreed action/params/kind, source uMotorTest");
        CHECK(n == 66, "66 catalog commands (AI(W906-GEARHAND) 20261003: + uMotorTest mtGearHandStart; AI(W906-TEACH-ZDOWN) 20261003: + uteach btnSetAllInArmZ / btnSetAllOutArmZ / btnOutZAllDown; AI(W906-GEARRATIO) 20261002: + uMotorTest mtGearMove / mtGearPreview / mtGearSave; AI(W906-ST02-C9-G2) 20261002: + uteach St02Teach*; 38 + 5 MT-E2 + 5 MT-E3c: FormShow/FormClose/BitBtn2/BitBtn3/btnSaveLogLightScaleData + AI(W906-MT-ALMRST) 20260929 btnAlarmReset + AI(W906-TEACH-ALMRST) 20260930 uteach btnAlarmReset + AI(W906-MT-SAVEMOT) 20260930 btnSaveMotTable + AI(W906-TEACH-ZALLUP) 20261001 uteach btnInZAllUp/btnOutZAllUp/btnZ1Servo/btnZ2Servo/btnArm1YServo/btnArm2YServo + AI(W906-ARMCELL) 20261002 uteach armCellGo)");
        CHECK(actions.size() == 49, "49 distinct actions (AI(W906-GEARHAND) 20261003: + gearHandBegin; AI(W906-TEACH-ZDOWN) 20261003: + teachSetAllArmZ / teachOutZAllDown; AI(W906-GEARRATIO) 20261002: + gearCalMove / gearRatioPreview / gearRatioSave; AI(W906-ST02-C9-G2) 20261002: + teachSt02; 28 + 4 MT-E2 + formShow/formClose/lightScale/lightScaleSave/lightScaleDataSave + resetAlarm + AI(W906-MT-SAVEMOT) saveMotTable + AI(W906-TEACH-ZALLUP) teachZAllUp/teachIndexServo + AI(W906-ARMCELL) moveToTrayCell)");
        {   //AI(W906-TEACH-ZALLUP) 20261001: the six Teach rows -- source uteach; the two Z All Up are kind motion (the page locks while the C++ answer is pending), the four Servo control
            const char* zb[6] = { "btnInZAllUp", "btnOutZAllUp", "btnZ1Servo", "btnZ2Servo", "btnArm1YServo", "btnArm2YServo" };
            int zseen = 0;
            cJSON* cat2 = cJSON_Parse(ss.str().c_str());
            const cJSON* cm2 = cat2 ? cJSON_GetObjectItemCaseSensitive(cat2, "commands") : 0;
            const cJSON* e = 0;
            if (cJSON_IsArray(cm2)) cJSON_ArrayForEach(e, cm2) {
                const cJSON* bt = cJSON_GetObjectItemCaseSensitive(e, "button");
                const cJSON* src = cJSON_GetObjectItemCaseSensitive(e, "source");
                const cJSON* ac = cJSON_GetObjectItemCaseSensitive(e, "action");
                const cJSON* kd = cJSON_GetObjectItemCaseSensitive(e, "kind");
                if (!cJSON_IsString(bt) || !cJSON_IsString(src) || !cJSON_IsString(ac) || !cJSON_IsString(kd)) continue;
                for (int k = 0; k < 6; ++k)
                    if (std::string(bt->valuestring) == zb[k] && std::string(src->valuestring) == "uteach" &&
                        std::string(ac->valuestring) == (k < 2 ? "teachZAllUp" : "teachIndexServo") &&
                        std::string(kd->valuestring) == (k < 2 ? "motion" : "control")) ++zseen;
            }
            cJSON_Delete(cat2);
            CHECK(zseen == 6, "AI(W906-TEACH-ZALLUP): the six Teach rows (uteach btnIn/OutZAllUp -> teachZAllUp motion; btnZ1/Z2/Arm1Y/Arm2YServo -> teachIndexServo control)");
        }
        {   //AI(W906-ARMCELL) 20261002: the Teach Arm Cell row -- source uteach, kind motion (the page locks while the C++ answer is pending),
            //  params arm, nozzle, area, col, row, zDown (RULINGS_20261002 #18; the page: web/page/ht9045_teach_armcell.js)
            int aseen = 0;
            cJSON* cat3 = cJSON_Parse(ss.str().c_str());
            const cJSON* cm3 = cat3 ? cJSON_GetObjectItemCaseSensitive(cat3, "commands") : 0;
            const cJSON* e = 0;
            if (cJSON_IsArray(cm3)) cJSON_ArrayForEach(e, cm3) {
                const cJSON* bt = cJSON_GetObjectItemCaseSensitive(e, "button");
                const cJSON* ac = cJSON_GetObjectItemCaseSensitive(e, "action");
                if (!cJSON_IsString(bt) || !cJSON_IsString(ac) || std::string(ac->valuestring) != "moveToTrayCell") continue;
                const cJSON* src = cJSON_GetObjectItemCaseSensitive(e, "source");
                const cJSON* kd = cJSON_GetObjectItemCaseSensitive(e, "kind");
                const cJSON* ps = cJSON_GetObjectItemCaseSensitive(e, "params");
                std::string plist;
                const cJSON* p = 0;
                if (cJSON_IsArray(ps)) cJSON_ArrayForEach(p, ps) if (cJSON_IsString(p)) plist += (plist.empty() ? "" : ",") + std::string(p->valuestring);
                if (std::string(bt->valuestring) == "armCellGo" && cJSON_IsString(src) && std::string(src->valuestring) == "uteach" &&
                    cJSON_IsString(kd) && std::string(kd->valuestring) == "motion" && plist == "arm,nozzle,area,col,row,zDown") ++aseen;
                else printf("    moveToTrayCell row: button=%s params=%s -- expected armCellGo, uteach, motion, arm,nozzle,area,col,row,zDown\n", bt->valuestring, plist.c_str());
            }
            cJSON_Delete(cat3);
            CHECK(aseen == 1, "AI(W906-ARMCELL): one moveToTrayCell row (uteach armCellGo, kind motion, params arm,nozzle,area,col,row,zDown)");
        }
        {   //AI(W906-GEARRATIO) 20261002: the Motor Test Gear Ratio rows (RULINGS_20261002 #22; the page: web/page/ht9045_mt_gearratio.js) --
            //  source uMotorTest; gearCalMove kind motion (the page locks while the answer is pending), the other two control
            struct GR { const char* button; const char* action; const char* kind; const char* params; int seen; };
            GR gr[4] = { { "mtGearMove", "gearCalMove", "motion", "motorId,distanceMm,speedPct,begin", 0 },
                         { "mtGearPreview", "gearRatioPreview", "control", "motorId,oldRatio,measC,measA,measDir,hand,rulerMm", 0 },   //AI(W906-GEARHAND) 20261003: + hand, rulerMm (the hand-push session)
                         { "mtGearSave", "gearRatioSave", "control", "motorId,oldRatio,measC,measA,measDir,previewKey,confirmLarge,hand,rulerMm", 0 } };  gr[3] = { "mtGearHandStart", "gearHandBegin", "control", "motorId", 0 };   //AI(W906-GEARHAND) 20261003: + the hand session's start (control: it sends nothing); same line
            cJSON* cat4 = cJSON_Parse(ss.str().c_str());
            const cJSON* cm4 = cat4 ? cJSON_GetObjectItemCaseSensitive(cat4, "commands") : 0;
            const cJSON* e = 0;
            int bad4 = 0;
            if (cJSON_IsArray(cm4)) cJSON_ArrayForEach(e, cm4) {
                const cJSON* ac = cJSON_GetObjectItemCaseSensitive(e, "action");
                if (!cJSON_IsString(ac)) continue;
                for (int k = 0; k < 4; ++k) {   //AI(W906-GEARHAND) 20261003: 3 -> 4 (gearHandBegin)
                    if (std::string(ac->valuestring) != gr[k].action) continue;
                    ++gr[k].seen;
                    const cJSON* bt = cJSON_GetObjectItemCaseSensitive(e, "button");
                    const cJSON* src = cJSON_GetObjectItemCaseSensitive(e, "source");
                    const cJSON* kd = cJSON_GetObjectItemCaseSensitive(e, "kind");
                    const cJSON* ps = cJSON_GetObjectItemCaseSensitive(e, "params");
                    std::string plist;
                    const cJSON* p = 0;
                    if (cJSON_IsArray(ps)) cJSON_ArrayForEach(p, ps) if (cJSON_IsString(p)) plist += (plist.empty() ? "" : ",") + std::string(p->valuestring);
                    if (!(cJSON_IsString(bt) && std::string(bt->valuestring) == gr[k].button && cJSON_IsString(src) && std::string(src->valuestring) == "uMotorTest" &&
                          cJSON_IsString(kd) && std::string(kd->valuestring) == gr[k].kind && plist == gr[k].params)) {
                        ++bad4; printf("    %s row: button=%s params=%s\n", gr[k].action, cJSON_IsString(bt) ? bt->valuestring : "?", plist.c_str());
                    }
                }
            }
            cJSON_Delete(cat4);
            CHECK(bad4 == 0 && gr[0].seen == 1 && gr[1].seen == 1 && gr[2].seen == 1 && gr[3].seen == 1,   //AI(W906-GEARHAND) 20261003: + gearHandBegin (mtGearHandStart, control, motorId)
                  "AI(W906-GEARRATIO): one row each -- uMotorTest mtGearMove gearCalMove (motion), mtGearPreview gearRatioPreview, mtGearSave gearRatioSave (control), with their params");
        }
        CHECK(unknown == 0, "every catalog action has a status");
        //AI(W906-MERGE-56bbf785) 20260926: machine live 30 (MT-E1..E3c) + laptop W5-b teachSet/teachGo = 32; ui = machine 3 + the laptop's
        //  setTeachFromCurrent/Offset = 5; 32 + 5 = the 37 actions -- nothing queued or blocked any more.
        CHECK(live.size() == 44 && live.count("gearHandBegin") && live.count("teachSetAllArmZ") && live.count("teachOutZAllDown") && live.count("gearCalMove") && live.count("gearRatioPreview") && live.count("gearRatioSave") && live.count("moveToTrayCell") && live.count("teachSt02") && live.count("teachZAllUp") && live.count("teachIndexServo") && live.count("saveMotTable") && live.count("resetAlarm") && live.count("reloadMotorData") && live.count("stop") && live.count("servoToggle") && live.count("jogP") &&   //AI(W906-MT-ALMRST) 20260929: + resetAlarm  //AI(W906-MT-SAVEMOT) 20260930: + saveMotTable (live 34)  //AI(W906-TEACH-ZALLUP) 20261001: + teachZAllUp / teachIndexServo (live 36)  //AI(W906-ARMCELL) 20261002: + moveToTrayCell (live 37)  //AI(W906-ST02-C9-G2) 20261002: + teachSt02 (live 41 with ARMCELL + GEARRATIO, laptop batch 49 merge)  //AI(W906-TEACH-ZDOWN) 20261003: + teachSetAllArmZ / teachOutZAllDown (live 43)
              live.count("moveRelative") && live.count("moveAbsolute") && live.count("moveSoftLimitN") &&
              live.count("setJogHighSpeed") && live.count("setSoftLimitP") && live.count("home") &&
              live.count("loopMove") && live.count("resetMNet") && live.count("setRangeAndInit") &&
              live.count("selectMotor") && live.count("setParamCell") && live.count("copyFrom") && live.count("setSpeed") &&
              live.count("motorPowerToggle") && live.count("formShow") && live.count("formClose") && live.count("lightScale") &&
              live.count("lightScaleSave") && live.count("lightScaleDataSave") && live.count("teachSet") && live.count("teachGo"),
              "live = stop/servo + W4-b1 (12) + W4-b2 (5) + reloadMotorData + 4 MT-E2 + AI(W906-MT-E3c): motorPowerToggle + the 5 MT-E3c actions + W5-b teachSet/teachGo");
        CHECK(std::string(MotorAccessActionStatus("motorPowerToggle")) == "live" &&
              std::string(MotorAccessActionStatus("reloadMotorData")) == "live", "AI(W906-MT-E3c): nothing is blocked any more (motorPowerToggle live)");
        CHECK(ui.size() == 5 && ui.count("setPos1") && ui.count("setPos2") && ui.count("refreshParameter") &&
              ui.count("setTeachFromCurrent") && ui.count("setTeachFromOffset"),
              "ui = {setPos1, setPos2, refreshParameter, setTeachFromCurrent, setTeachFromOffset}");
        CHECK(live.size() + ui.size() == actions.size(), "AI(W906-MERGE-56bbf785): every catalog action is live or ui (no queued / blocked left)");
        CHECK(std::string(MotorAccessActionStatus("noSuchAction")) == "unknown", "unknown action -> unknown");
    }

    // ---------------------------------------------------------------- (3)
    printf("[3] honesty: queued / unknown never touch the backend\n");
    {
        FakeBackend be;
        be.table["MInArmX"] = Axis1203(0, 0, 0, 0);
        be.opened.assign(2, true);
        //AI(W906-MERGE-56bbf785) 20260926: the machine's last queued actions were the four W5 teach ones (AI(W906-MT-E3c): motorPowerToggle
        //  is live, nothing blocked); the laptop's W5-b wires them (teachSet/teachGo live, setTeachFrom* ui), so nothing is queued any more.
        //  What stays to lock here: the W5-b live paths refuse -- and send nothing -- when their preconditions are missing.
        CHECK(std::string(MotorAccessActionStatus("teachSet")) == "live" && std::string(MotorAccessActionStatus("teachGo")) == "live" &&
              std::string(MotorAccessActionStatus("setTeachFromCurrent")) == "ui" && std::string(MotorAccessActionStatus("setTeachFromOffset")) == "ui",
              "no queued action left (W5-b: teachSet/teachGo live, setTeachFromCurrent/Offset ui)");
        const MotorAccessOutcome ts = MotorAccessDispatch(Req("uteach", "teachSet", "MInArmX"), 1, be);
        CHECK(!ts.ok && Has(ts.ackJson, "start"), "teachSet without start -> refused (says what is missing)");
        const MotorAccessOutcome tj = MotorAccessDispatch(Req("uteach", "jogP", "MInArmX"), 1, be);
        CHECK(!tj.ok && Has(tj.ackJson, "MOT[].Motor"), "uteach jogP without a golden motor object -> refused (W5-b live path), nothing sent");
        const MotorAccessOutcome t = MotorAccessDispatch(Req("uteach", "teachGo", "MInArmX"), 1, be);
        CHECK(!t.ok && Has(t.ackJson, "btn"), "teachGo without btn -> refused (says which golden button is needed)");
        CHECK(be.Calls() == 0, "no backend call for any refused teach action (nothing moves)");

        CHECK(!MotorAccessDispatch(Req("uFoo", "stop", "MInArmX"), 1, be).ok, "unknown source refused");
        CHECK(!MotorAccessDispatch(Req("uMotorTest", "fly", "MInArmX"), 1, be).ok, "unknown action refused");
        CHECK(be.Calls() == 0, "still no backend call");

        const MotorAccessOutcome u = MotorAccessDispatch(Req("uMotorTest", "setPos1", "MInArmX"), 1, be);
        CHECK(u.ok && AckShapeOk(u.ackJson, 7) && Has(u.ackJson, "\"layer\":\"ui\""), "setPos1 -> done, layer ui");
        CHECK(be.Calls() == 0, "ui-only action touches nothing");
    }

    // ---------------------------------------------------------------- (4)
    printf("[4] stop\n");
    {
        FakeBackend be;
        be.table["MInArmX"] = Axis1203(0, 0, 0, 0);
        be.table["MInArmPitch"] = AxisOther(2, "MN200", true);
        be.opened.push_back(true); be.opened.push_back(true);
        be.opened.push_back(false); be.opened.push_back(true);     // slot 2 never opened
        const MotorAccessOutcome o = MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmPitch"), 4242, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7), "stop -> ok, ack shape");
        CHECK(be.stopAll.size() == 1 && be.stopAll[0] == "uMotorTest", "golden StopAllMotor called once with source");
        bool axesOk = be.executed.size() == 3;
        const int want[3] = { 0, 1, 3 };
        for (unsigned i = 0; axesOk && i < 3; ++i)
            axesOk = be.executed[i].kind == kCmdAxStop && be.executed[i].axis == want[i] && be.executed[i].wireId == 4242;
        CHECK(axesOk, "kCmdAxStop to opened axes 0,1,3 only, carrying the wire id");
        CHECK(Has(o.ackJson, "\"stopSent\":3"), "ack reports 3 stops sent");
        CHECK(be.stopMotor.size() == 1 && be.stopMotor[0] == 2, "selected non-1203 motor -> MOT[2].PCIL132_StopMotor");  CHECK(be.stopSweep.size() == 3 && !be.stopSweep[0] && !be.stopSweep[1] && !be.stopSweep[2] && !MotorAccessInAlarmSweep(), "AI(W906-INDEXZ-1203) D2: Motor Test STOP's 1203 stops are NOT the alarm sweep (an operator stop = golden ST for the Index Z1 route)");

        FakeBackend nb;
        nb.ready = false; nb.notReadyWhy = "no SDK";
        nb.opened.assign(3, true);
        nb.table["MInArmX"] = Axis1203(0, 0, 0, 0);
        const MotorAccessOutcome n = MotorAccessDispatch(Req("uteach", "stop", "MInArmX"), 1, nb);
        CHECK(n.ok && Has(n.ackJson, "\"ready\":false") && Has(n.ackJson, "no SDK"), "1203 not ready -> ok, says so");
        CHECK(nb.stopAll.size() == 1 && nb.stopAll[0] == "uteach", "golden half still runs (uteach)");
        CHECK(nb.executed.empty() && nb.stopMotor.empty(), "no 1203 command without control; 1203 motor not sent to MOT");

        FakeBackend eb;
        eb.opened.assign(1, true);
        const MotorAccessOutcome e = MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, eb);
        CHECK(e.ok && eb.stopAll.size() == 1 && eb.executed.size() == 1, "stop with no motor selected still stops all");
    }

    // ---------------------------------------------------------------- (4b)
    printf("[4b] jog release = golden MouseUp (that one axis only)\n");
    {
        FakeBackend be;
        be.table["MInArmX"] = Axis1203(0, 0, 0, 5);
        be.table["MInArmPitch"] = AxisOther(2, "MN200", true);
        be.table["MInArmY"] = Axis1203(1, 1, 0, -1, "no such station");
        be.opened.assign(8, true);
        MotorAccessReq r = Req("uMotorTest", "stop", "MInArmX");
        r.button = "sbMotorTest_JogP";
        MotorAccessOutcome o = MotorAccessDispatch(r, 77, be);
        CHECK(o.ok && be.executed.size() == 1 && be.executed[0].kind == kCmdAxStop && be.executed[0].axis == 5 &&
              be.executed[0].wireId == 77, "1203 jog release -> kCmdAxStop on that axis only");
        CHECK(be.stopAll.empty(), "jog release does NOT StopAllMotor");

        r = Req("uteach", "stop", "MInArmPitch"); r.button = "btnJogN";
        o = MotorAccessDispatch(r, 1, be);
        CHECK(o.ok && be.stopMotor.size() == 1 && be.stopMotor[0] == 2 && be.executed.size() == 1,
              "non-1203 jog release -> golden MOT[2] stop, no 1203 command");

        r = Req("uMotorTest", "stop", "MInArmY"); r.button = "sbMotorTest_JogN";
        o = MotorAccessDispatch(r, 1, be);
        CHECK(!o.ok && be.refusals.size() == 1, "unresolved 1203 axis on release -> ok=false (page shows it) + NoteRefusal");

        be.ready = false; be.notReadyWhy = "no SDK";
        r = Req("uMotorTest", "stop", "MInArmX"); r.button = "sbMotorTest_JogP";
        o = MotorAccessDispatch(r, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "no SDK"), "control missing on release -> ok=false, says it could not stop");

        r = Req("uMotorTest", "stop", 0); r.button = "sbMotorTest_JogP";
        o = MotorAccessDispatch(r, 1, be);
        CHECK(o.ok && Has(o.ackJson, "no-motor") && be.stopAll.empty(), "no motor selected -> golden early return");

        r = Req("uMotorTest", "stop", "MInArmX"); r.button = "btnStop";
        be.ready = true;
        o = MotorAccessDispatch(r, 1, be);
        CHECK(o.ok && be.stopAll.size() == 1, "btnStop (not a jog key) -> full StopAllMotor path");
    }

    // ---------------------------------------------------------------- (5)
    printf("[5] servoToggle\n");
    {
        FakeBackend be;
        be.table["MInArmX"] = Axis1203(0, 0, 0, 5);
        be.table["MInArmY"] = Axis1203(1, 1, 0, -1, "1203 監看器找不到 站 1 軸 0");
        be.table["MInArmPitch"] = AxisOther(2, "MN200", true);
        be.table["MDead"] = AxisOther(9, "MN200", false);
        be.opened.assign(8, true);

        MotorAccessReq r = Req("uMotorTest", "servoToggle", "MInArmX");
        r.flag["servoOn"] = false;
        MotorAccessOutcome o = MotorAccessDispatch(r, 9, be);
        CHECK(o.ok && be.executed.size() == 1 && be.executed[0].kind == kCmdAxSvOn && be.executed[0].axis == 5 &&
              be.executed[0].value == 0.0 && be.executed[0].wireId == 9, "explicit servoOn=false -> SvOn(ax5, 0)");
        CHECK(Has(o.ackJson, "\"basis\":\"page\"") && Has(o.ackJson, "\"servoOn\":false"), "ack basis=page");
        {   //AI(W906-BRAKE-AXIS) 20260929: servo OFF of an axis with a released brake -> hold it FIRST, 100 ms, then SvOn 0
            be.brakeAlias = "MInArmX"; be.brakeReleased = false; be.brakeHoldAt = -1;
            const std::size_t e0 = be.executed.size(), s1 = be.sleeps.size();
            MotorAccessReq rb = Req("uMotorTest", "servoToggle", "MInArmX"); rb.flag["servoOn"] = true;
            o = MotorAccessDispatch(rb, 9, be);
            CHECK(o.ok && be.sleeps.size() == s1 && be.brakeHoldAt == -1 && be.executed.back().value == 1.0 && Has(o.ackJson, "0.5 s after SVON"),
                  "BRAKE-AXIS: servo on -> no brake here (released 0.5 s after SVON by the live tick), no wait");
            be.brakeReleased = true;                                            // the live tick released it
            const std::size_t s0 = be.sleeps.size(), e1 = be.executed.size();
            rb.flag["servoOn"] = false;                                         // (last: leaves golden's static toggle at false, as before)
            o = MotorAccessDispatch(rb, 9, be);
            CHECK(o.ok && be.brakeHoldAt == (int)e1 && be.executed.size() == e1 + 1 && be.executed.back().kind == kCmdAxSvOn &&
                  be.executed.back().value == 0.0 && be.sleeps.size() == s0 + 1 && be.sleeps.back() == 100 && Has(o.ackJson, "brake held"),
                  "BRAKE-AXIS: servo off -> brake held BEFORE SvOn 0, 100 ms between, the ack says so");
            be.brakeAlias.clear();
            be.executed.erase(be.executed.begin() + (std::ptrdiff_t)e0, be.executed.end());   // the checks below count from here
        }

        be.svon[5] = true;
        o = MotorAccessDispatch(Req("uteach", "servoToggle", "MInArmX"), 9, be);
        CHECK(o.ok && be.executed.size() == 2 && be.executed[1].value == 0.0 && Has(o.ackJson, "toggle-actual"),
              "no param, SVON known on -> off (golden uteach !Led[iServoOn])");

        be.ready = false; be.notReadyWhy = "no SDK";
        o = MotorAccessDispatch(Req("uMotorTest", "servoToggle", "MInArmX"), 9, be);
        CHECK(!o.ok && Has(o.ackJson, "no SDK") && be.executed.size() == 2, "1203 not ready -> refused, nothing sent");
        be.ready = true;

        o = MotorAccessDispatch(Req("uMotorTest", "servoToggle", "MInArmY"), 9, be);
        CHECK(!o.ok && be.executed.size() == 2 && be.refusals.size() == 1, "unresolved 1203 axis -> refused + NoteRefusal");

        be.rejectNext = true;
        o = MotorAccessDispatch(Req("uMotorTest", "servoToggle", "MInArmX"), 9, be);
        CHECK(!o.ok && Has(o.ackJson, "ERROR_STOP"), "1203 Execute refusal propagates as ok=false");

        be.dry = true;
        r = Req("uMotorTest", "servoToggle", "MInArmX"); r.flag["servoOn"] = true;
        o = MotorAccessDispatch(r, 9, be);
        CHECK(o.ok && Has(o.ackJson, "\"issued\":false") && Has(o.ackJson, "NOT issued"), "dry control -> says NOT issued");
        be.dry = false;

        o = MotorAccessDispatch(Req("uteach", "servoToggle", "MInArmPitch"), 9, be);
        CHECK(!o.ok && be.servoSet.empty(), "uteach, MN200 state unknown, no param -> refused (no guessing)");
        be.goldenSvon[2] = false;
        o = MotorAccessDispatch(Req("uteach", "servoToggle", "MInArmPitch"), 9, be);
        CHECK(o.ok && be.servoSet.size() == 1 && be.servoSet[0].first == 2 && be.servoSet[0].second == true,
              "MN200 axis -> golden MOT[2].ServoOnOff(true)");

        o = MotorAccessDispatch(Req("uMotorTest", "servoToggle", "MDead"), 9, be);
        CHECK(!o.ok && be.servoSet.size() == 1, "no golden motor object -> refused");
        o = MotorAccessDispatch(Req("uMotorTest", "servoToggle", 0), 9, be);
        CHECK(!o.ok, "no motor selected -> refused (golden ActiveIndex==-1)");
        o = MotorAccessDispatch(Req("uMotorTest", "servoToggle", "MNope"), 9, be);
        CHECK(!o.ok, "alias not in table -> refused");

        // golden uMotorTest static toggle: anchor it with an explicit command, then two unknown-state clicks alternate.
        FakeBackend sb;
        sb.table["MInArmPitch"] = AxisOther(2, "MN200", true);    // goldenSvon empty -> state unknown
        r = Req("uMotorTest", "servoToggle", "MInArmPitch"); r.flag["servoOn"] = true;
        MotorAccessDispatch(r, 1, sb);
        MotorAccessDispatch(Req("uMotorTest", "servoToggle", "MInArmPitch"), 1, sb);
        o = MotorAccessDispatch(Req("uMotorTest", "servoToggle", "MInArmPitch"), 1, sb);
        CHECK(sb.servoSet.size() == 3 && sb.servoSet[0].second == true && sb.servoSet[1].second == false &&
              sb.servoSet[2].second == true && Has(o.ackJson, "golden-static"),
              "uMotorTest, state unknown -> golden static alternates relative to the last command");
    }

    // ---------------------------------------------------------------- (6)
    printf("[6] golden unit / speed conversions (hand values)\n");
    {
        CHECK(MotorUserToCard(1000, 2.5) == 400, "user 1000 / gear 2.5 -> card 400 (exact)");
        CHECK(MotorUserToCard(1001, 2.5) == 401, "user 1001 -> 401 (golden walks up until p1*r >= p)");
        CHECK(MotorUserToCard(-1001, 2.5) == -401, "user -1001 -> -401 (walks down until p1*r <= p)");
        CHECK(MotorUserToCard(1234, 0.0) == 1234, "gear 0 treated as 1.0 (golden GetRealPos)");
        CHECK(MotorCardToUser(400.9, 2.5) == 1000, "card 400.9 -> (int)400 * 2.5 = 1000 (ReadRealPos returns int)");
        CHECK(MotorCardToUser(-17647.6, 1.0) == -17647, "card -17647.6 -> -17647");

        MotorGolden g;
        g.jogHigh = 10000; g.jogLow = 100; g.initSpeed = 500; g.acc = 100000; g.dec = 200000;
        Motor1203Speed sp = MotorSpeedFromPct(50, g);
        CHECK(!sp.skip && sp.s == 5000 && sp.velLow == 250.0 && sp.velHigh == 5000.0 && sp.acc == 100000.0 && sp.dec == 200000.0,
              "50% -> s 5000, velLow 500*0.5, velHigh 10000*0.5, acc/dec from golden");
        sp = MotorSpeedFromPct(150, g);
        CHECK(sp.s == 10000 && sp.velHigh == 10000.0, "150% clamped to 100 (TMyMotor::SetSpeed)");
        sp = MotorSpeedFromPct(0, g);
        CHECK(sp.s == 0 && sp.velLow == 5.0 && sp.velHigh == 100.0, "0% -> persent clamped to 0.01 (TMyEtherCatMotor::SetSpeed)");
        g.initSpeed = 8000;
        sp = MotorSpeedFromPct(50, g);
        CHECK(sp.velLow == 4000.0 && sp.velHigh == 5000.0, "velLow below velHigh stays");
        g.initSpeed = 20000;
        sp = MotorSpeedFromPct(50, g);
        CHECK(sp.velLow == 10000.0 && sp.velHigh == 10000.0, "velHigh < velLow -> raised to velLow");
        MotorGolden z = g; z.initSpeed = 500; z.zStack = true; z.jogHigh = 20000; z.jogLow = 200;
        sp = MotorSpeedFromPct(50, z);
        CHECK(sp.s == 10100, "Z stack: (high-low)*p/100 + low");
        MotorGolden ix = g; ix.indexMotor = true;
        CHECK(MotorSpeedFromPct(50, ix).skip, "index motor: golden SetSpeed does nothing");
        MotorGolden h0 = g; h0.jogHigh = 0;
        CHECK(MotorSpeedFromPct(50, h0).skip, "PJogHighSpeed==0: golden returns before setting speed");
    }

    // ---------------------------------------------------------------- (7)
    printf("[7] W4-b1 motion (uMotorTest)\n");
    {
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 2.5; g.softP = 999999; g.softN = -999999;
        g.jogHigh = 10000; g.jogLow = 100; g.initSpeed = 500; g.acc = 100000; g.dec = 200000; g.homeFlag = 1;
        MotorGolden gDir1 = g; gDir1.direction = true;

        FakeBackend be;
        be.table["MOutArmY"] = Axis1203(20, 20, 0, 3);
        be.table["MInArmX"]  = Axis1203(0, 0, 0, 1);
        be.table["MInArmPitch"] = AxisOther(2, "MN200", true);
        be.golden[20] = g; be.golden[0] = gDir1; be.golden[2] = g;
        be.cmdPos[3] = 400.0;                                  // user 1000
        be.cmdPos[1] = 400.0;
        be.readPos[2] = 5000;
        be.opened.assign(8, true);

        MotorAccessResetJobs();                                                  // AI(W906-MT-E2): no scroll-bar position remembered yet
        MotorAccessReq r = Req("uMotorTest", "jogP", "MOutArmY"); r.button = "sbMotorTest_JogP"; r.num["speed"] = 50;
        MotorAccessOutcome o = MotorAccessDispatch(r, 11, be);
        // AI(W906-MT-E2) 20260925: JOG = GOLDEN (user ruling). The page's speed (edtSpeed 50) is dropped like golden JogP does;
        //   the jog family goes out at the last scroll position -- none yet => 1% (s=100, persent 0.01: velLow 500*0.01=5,
        //   velHigh 10000*0.01=100) -- and the PTP family is NOT touched.
        bool seqOk = o.ok && be.executed.size() == 5 &&
                     be.executed[0].kind == kCmdAxSetSpeed && be.executed[0].speed == kSpeedJogInit && be.executed[0].value == 5.0 &&
                     be.executed[1].speed == kSpeedJogRun && be.executed[1].value == 100.0 &&
                     be.executed[2].speed == kSpeedJogAcc && be.executed[2].value == 100000.0 &&
                     be.executed[3].speed == kSpeedJogDec && be.executed[3].value == 200000.0 &&
                     be.executed[4].kind == kCmdAxJogStart && be.executed[4].dir == 1 && be.executed[4].axis == 3;
        CHECK(seqOk, "jogP 1203 with no scroll position yet: 4 CFG_AxJog* speeds at 1% (page speed 50 ignored, PTP untouched), then Acm_AxJog +1");
        CHECK(be.extDrive.size() == 1 && be.extDrive[0].value == 1.0 && be.extDrive[0].axis == 3, "jogP 1203: Acm_AxSetExtDrive(ax, 1) before the jog (golden bFirstClickJog)");
        r.action = "jogN"; r.button = "sbMotorTest_JogN";
        o = MotorAccessDispatch(r, 11, be);
        CHECK(o.ok && be.executed.size() == 10 && be.executed[9].kind == kCmdAxJogStart && be.executed[9].dir == -1 && be.extDrive.size() == 2, "jogN -> ExtDrive(1) + Acm_AxJog -1");
        {   //AI(W906-JOG-MAXVEL) 20261003: EastSun「M35~M40 … 我設定JOG 速度都沒有效過」-- a CFG_AxMaxVel the monitor knows to be below the
            //  jog's velHigh (100 here) is raised to golden InitMotor's PJogHighSpeed (10000) before the jog family; MaxAcc / MaxDec
            //  already high enough are left alone; unknown (the cases above) sends nothing extra.
            be.speedNow[kSpeedMaxVel] = 50.0; be.speedNow[kSpeedMaxAcc] = 1.0e9; be.speedNow[kSpeedMaxDec] = 1.0e9;
            const std::size_t n0 = be.executed.size();
            r.action = "jogP"; r.button = "sbMotorTest_JogP";
            o = MotorAccessDispatch(r, 11, be);
            CHECK(o.ok && be.executed.size() == n0 + 6 &&
                  be.executed[n0].kind == kCmdAxSetSpeed && be.executed[n0].speed == kSpeedMaxVel && be.executed[n0].value == 10000.0 &&
                  be.executed[n0 + 1].speed == kSpeedJogInit && be.executed[n0 + 5].kind == kCmdAxJogStart,
                  "JOG-MAXVEL: ceiling 50 < velHigh 100 -> CFG_AxMaxVel = PJogHighSpeed 10000 first, then the 4 jog values and Acm_AxJog");
            be.speedNow.clear();
            MotorAccessReq rel2 = Req("uMotorTest", "stop", "MOutArmY"); rel2.button = "sbMotorTest_JogP";
            (void)MotorAccessDispatch(rel2, 11, be);
        }
        {   //AI(W906-JOG-VLTIME) 20261003: EastSun「按了有一段時間 他會突然動很快」-- a STEPPER axis gets golden's CFG_AxJogVLTime = 0
            //  (myEthercatmotor.cpp:724) after the 4 jog values; the servo jogs above got nothing extra (「不要改到其他伺服馬達」).
            be.golden[20].stepMotor = true;
            const std::size_t n0 = be.executed.size();
            r.action = "jogP"; r.button = "sbMotorTest_JogP";
            o = MotorAccessDispatch(r, 11, be);
            CHECK(o.ok && be.executed.size() == n0 + 6 &&
                  be.executed[n0].speed == kSpeedJogInit && be.executed[n0 + 3].speed == kSpeedJogDec &&
                  be.executed[n0 + 4].kind == kCmdAxSetInitCfg && be.executed[n0 + 4].initCfg == kInitCfgJogVLTime &&
                  be.executed[n0 + 4].value == 0.0 && be.executed[n0 + 4].axis == 3 &&
                  be.executed[n0 + 5].kind == kCmdAxJogStart,
                  "JOG-VLTIME: stepper jog -> 4 jog values, CFG_AxJogVLTime = 0, then Acm_AxJog");
            be.golden[20].stepMotor = false;
            MotorAccessReq rel3 = Req("uMotorTest", "stop", "MOutArmY"); rel3.button = "sbMotorTest_JogP";
            (void)MotorAccessDispatch(rel3, 11, be);
        }
        {
            MotorAccessReq rel = Req("uMotorTest", "stop", "MOutArmY"); rel.button = "sbMotorTest_JogN";
            const std::size_t e0 = be.extDrive.size(), x0 = be.executed.size();
            CHECK(MotorAccessDispatch(rel, 12, be).ok && be.executed.size() == x0 + 1 && be.executed.back().kind == kCmdAxStop &&
                  be.extDrive.size() == e0 + 1 && be.extDrive.back().value == 0.0 && be.extDrive.back().axis == 3,
                  "jog release = golden DecStop: Acm_AxStopDec then Acm_AxSetExtDrive(ax, 0) (AI(W906-MT-E1))");
        }
        {   // AI(W906-MT-E2): no speed at all is fine now (golden SelMotSpeed goes nowhere)
            MotorAccessReq ns = r; ns.num.erase("speed");
            const std::size_t x0 = be.executed.size();
            CHECK(MotorAccessDispatch(ns, 1, be).ok && be.executed.size() == x0 + 5 && be.executed[x0 + 1].value == 100.0,
                  "jog without speed -> allowed, jog family at the remembered 1% (was: refused for a missing edtSpeed)");
            MotorAccessReq rel = Req("uMotorTest", "stop", "MOutArmY"); rel.button = "sbMotorTest_JogN";
            MotorAccessDispatch(rel, 12, be);
        }

        int before = (int)be.executed.size();
        MotorAccessReq d1 = Req("uMotorTest", "jogP", "MInArmX"); d1.num["speed"] = 50;
        o = MotorAccessDispatch(d1, 1, be);
        CHECK(o.ok && (int)be.executed.size() > before && be.executed.back().dir == 1,   // AI(W906-DIR6B) 20260925: 第 13 條 6B
              "ruling 6B: Direction=1 on a 1203 axis is ignored (direction belongs to the drive, Pn000) -> jogP accepted, card +1");
        before = (int)be.executed.size();   // AI(W906-DIR6B) 20260925: 6B 之後這筆寸動會送出命令；後面的「nothing sent」段以此為基準

        be.cmdPos[3] = 400000.0;                               // user 1,000,000
        o = MotorAccessDispatch(r, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "over limitation") && (int)be.executed.size() == before, "|NowPos|>999999 -> refused");
        be.cmdPos[3] = 400.0;
        be.doorClosed = false;
        o = MotorAccessDispatch(r, 1, be);
        CHECK(!o.ok && (int)be.executed.size() == before, "CheckSafeDoorIsClosed false -> refused");
        be.doorClosed = true;
        be.doorOpen.insert(20);
        o = MotorAccessDispatch(r, 1, be);
        CHECK(!o.ok && (int)be.executed.size() == before, "TMyMotor::JogP safe-door check -> refused");
        be.doorOpen.clear();

        MotorAccessReq jm = Req("uMotorTest", "jogP", "MInArmPitch"); jm.num["speed"] = 30;
        o = MotorAccessDispatch(jm, 1, be);
        CHECK(o.ok && be.jogs.size() == 1 && be.jogs[0].mi == 2 && be.jogs[0].pos && be.jogs[0].pct == 1 &&
              (int)be.executed.size() == before, "non-1203 jog -> golden MOT[2].JogP at the remembered scroll position 1% (page speed 30 ignored, AI(W906-MT-E2)), no 1203 command");

        // moveRelative
        MotorAccessReq mp = Req("uMotorTest", "moveRelative", "MOutArmY");
        mp.button = "sbMotorTest_MoveP"; mp.num["interval"] = 100; mp.num["speed"] = 50;
        o = MotorAccessDispatch(mp, 12, be);
        CHECK(o.ok && be.executed.size() == (unsigned)before + 5 && be.executed.back().kind == kCmdAxMoveAbs &&
              be.executed.back().value == 440.0 && Has(o.ackJson, "\"target\":1100"),
              "MoveP: ReadPos 1000 + 100 = 1100 -> card 440 via moveAbs (golden MoveToPos)");
        MotorAccessReq mn = mp; mn.button = "sbMotorTest_MoveN"; mn.num["interval"] = -100;
        o = MotorAccessDispatch(mn, 12, be);
        CHECK(o.ok && be.executed.back().value == 360.0 && Has(o.ackJson, "\"target\":900"), "MoveN (page sends -interval): 1000 - 100 -> card 360");

        int n0 = (int)be.executed.size();
        be.golden[20].softP = 1100;
        o = MotorAccessDispatch(mp, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "positive soft limit") && (int)be.executed.size() == n0, "target == softP -> refused (golden >=), no speed/move sent");
        be.golden[20].softP = 999999;
        be.golden[20].softN = 900;
        o = MotorAccessDispatch(mn, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "negative soft limit"), "target == softN -> refused (golden <=)");
        be.golden[20].softN = -999999;
        be.locked.insert(20);
        o = MotorAccessDispatch(mp, 1, be);
        CHECK(!o.ok && (int)be.executed.size() == n0 + 1 && be.executed.back().kind == kCmdAxStop, "locked -> golden stops the motor, no move");
        be.locked.clear();
        n0 = (int)be.executed.size();
        be.notReady.insert(3);
        o = MotorAccessDispatch(mp, 1, be);
        CHECK(!o.ok && (int)be.executed.size() == n0, "axis not READY -> refused (golden MotionDone)");
        be.notReady.clear();
        be.table["MOutArmY"].tableEnable = false;                                  // NB2 R22：1203 看 Mot_Table 的 Enable 欄
        o = MotorAccessDispatch(mp, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Enable=0"), "Enable=0 (Mot_Table) -> refused (golden: not installed)");
        be.table["MOutArmY"].tableEnable = true;
        be.golden[20].enable = false;                                              // SOFT_SIMULTE 建置：golden 把 Motor->Enable 一律設 false
        o = MotorAccessDispatch(mp, 1, be);
        CHECK(!Has(o.ackJson, "Enable=0"), "sim build (Motor->Enable forced false, table Enable=1) -> NOT refused for Enable (was: every 1203 move refused)");
        be.golden[20].enable = true;
        MotorAccessReq mg = Req("uMotorTest", "moveRelative", "MInArmPitch"); mg.button = "sbMotorTest_MoveN";
        mg.num["interval"] = 250; mg.num["speed"] = 40;
        be.moveRet = 0;
        o = MotorAccessDispatch(mg, 1, be);
        CHECK(o.ok && be.moves.size() == 1 && be.moves[0].target == 4750 && be.moves[0].setSpeed && be.moves[0].pct == 40,
              "non-1203 MoveN -> golden MotorMove(ReadPos 5000 - 250) with SetSpeed");

        // moveAbsolute
        MotorAccessReq ga = Req("uMotorTest", "moveAbsolute", "MOutArmY"); ga.num["targetPos"] = 2000; ga.num["speed"] = 50;
        be.canRun = false;
        o = MotorAccessDispatch(ga, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "EMG"), "btnGo: IsMotorCanRun false -> EMG Stop");
        be.canRun = true;
        be.golden[20].homeFlag = 0;
        o = MotorAccessDispatch(ga, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "not home"), "btnGo: HomeFlag 0 -> Motor not home");
        be.golden[20].homeFlag = 1;
        o = MotorAccessDispatch(ga, 13, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 800.0, "btnGo 2000 -> card 800");

        // soft-limit moves (targets from C++, golden refuses the positive one)
        MotorAccessReq sp = Req("uMotorTest", "moveSoftLimitP", "MOutArmY");
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(sp, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "positive soft limit") && (int)be.executed.size() == n0,
              "moveSoftLimitP -> Tar==PSoftLimitP -> golden refuses (faithful)");
        MotorAccessReq sn = Req("uMotorTest", "moveSoftLimitN", "MInArmPitch");
        be.moveRet = -3;
        o = MotorAccessDispatch(sn, 1, be);
        CHECK(!o.ok && be.moves.back().target == -999999, "non-1203 moveSoftLimitN -> golden MotorMove(PSoftLimitN), -3 propagated");

        // parameter buttons
        MotorAccessReq ph = Req("uMotorTest", "setJogHighSpeed", "MOutArmY"); ph.num["speed"] = 50;
        o = MotorAccessDispatch(ph, 1, be);
        CHECK(o.ok && be.params.size() == 1 && be.params[0].which == kParamJogHigh && be.params[0].value == 5000,
              "setJogHighSpeed: PJogHighSpeed = ReadSpeed() = s of SetSpeed(50%)");
        MotorAccessReq ps = Req("uMotorTest", "setSoftLimitP", "MOutArmY");
        o = MotorAccessDispatch(ps, 1, be);
        CHECK(o.ok && be.params.back().which == kParamSoftP && be.params.back().value == 1000, "setSoftLimitP = current user pos 1000");
        MotorAccessReq pd = Req("uMotorTest", "setSoftLimitN", "MInArmX");
        o = MotorAccessDispatch(pd, 1, be);
        CHECK(o.ok && be.params.back().which == kParamSoftN,   // AI(W906-DIR6B) 20260925: 第 13 條 6B
              "ruling 6B: setSoftLimitN on a Direction=1 1203 axis is accepted (Direction ignored)");
        MotorAccessReq pl = Req("uMotorTest", "setJogLowSpeed", "MInArmX"); pl.num["speed"] = 10;
        o = MotorAccessDispatch(pl, 1, be);
        CHECK(o.ok && be.params.back().which == kParamJogLow && be.params.back().value == 1000,
              "speed params do not depend on Direction (10% of 10000)");

        // NB2 R19 RW4-1 / RW4-2: dry control and partial stop
        FakeBackend db;
        db.opened.assign(3, true); db.dry = true;
        o = MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, db);
        CHECK(o.ok && Has(o.ackJson, "\"stopSent\":0") && Has(o.ackJson, "\"stopAccepted\":3") && Has(o.ackJson, "NOT issued"),
              "dry control: 3 accepted, 0 sent, says NOT issued");
        FakeBackend pb;
        pb.opened.assign(2, true); pb.rejectNext = true;
        o = MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, pb);
        CHECK(o.ok && Has(o.ackJson, "\"partial\":true") && Has(o.ackJson, "REFUSED"), "one axis refused stop -> partial:true");
    }

    // ---------------------------------------------------------------- (8)
    printf("[8] W4-b2 home / loop / resetMNet / range-rate\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 2.5; g.softP = 999999; g.softN = -999999;
        g.jogHigh = 10000; g.initSpeed = 500; g.acc = 1000; g.dec = 1000; g.homeFlag = 1;
        g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100;          // NB2 R23 W4C-3
        MotorGolden gh1 = g; gh1.homeDirection = true;
        MotorGolden gz = g;  gz.armZ = true;
        MotorGolden gs = g;  gs.scaleMotor = true;
        FakeBackend be;
        be.table["MOutArmY"]     = Axis1203(20, 20, 0, 3);
        be.table["MTrayX"]       = Axis1203(30, 30, 0, 4);
        be.table["MOutArmZA"]    = Axis1203(22, 22, 0, 5);
        be.table["MInArmXScale"] = Axis1203(50, 50, 0, 6);
        be.table["MInArmPitch"]  = AxisOther(2, "MN200", true);
        be.golden[20] = g; be.golden[30] = gh1; be.golden[22] = gz; be.golden[50] = gs; be.golden[2] = g;
        be.opened.assign(8, true);

        MotorAccessOutcome o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MOutArmY", true), 21, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxHome && be.executed.back().homeMode == 128 && be.executed.back().dir == -1,
              "HomeDirection=0 -> Acm_AxHome(128, -1) (golden: MoveHome dir 1 = negative)");
        CHECK(be.homeFlags[20] == 0 && MotorAccessJobs().homesActive == 1, "HomeFlag=0 at start (golden :1150), job active");
        be.state[3] = 4; MotorAccessTick(be, 500);
        CHECK(be.homeFlags[20] == 0 && MotorAccessJobs().homesActive == 1, "HOMING -> still waiting");
        be.state[3] = 1; MotorAccessTick(be, 500);
        CHECK(be.homeFlags[20] == 1 && MotorAccessJobs().homesActive == 0, "READY after HOMING -> HomeFlag=1");

        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MTrayX", true), 22, be);
        CHECK(o.ok && be.executed.back().homeMode == 124 && be.executed.back().dir == 1, "HomeDirection=1 -> Acm_AxHome(124, +1)");
        be.state[4] = 3; MotorAccessTick(be, 500);
        CHECK(be.homeFlags[30] == 2 && MotorAccessJobs().homesActive == 0, "ERROR_STOP -> HomeFlag=2");

        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MTrayX", true), 23, be);
        be.state[4] = 1;
        for (int i = 0; i < 9; ++i) MotorAccessTick(be, 500);
        CHECK(MotorAccessJobs().homesActive == 1 && be.homeFlags[30] == 0, "READY but never HOMING: still waiting at 4.5 s");
        MotorAccessTick(be, 500);
        CHECK(MotorAccessJobs().homesActive == 0 && be.homeFlags[30] == 2, "never entered HOMING within 5 s -> HomeFlag=2 (never a false 1)");

        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MOutArmY", true), 24, be);
        be.state[3] = 4; MotorAccessTick(be, 500);
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MOutArmY", false), 25, be);
        CHECK(o.ok && Has(o.ackJson, "homeStopped") && be.executed.back().kind == kCmdAxStop && MotorAccessJobs().homesActive == 0 &&
              Has(o.ackJson, "\"homeActive\":false"), "HOME released (start=false) while homing -> stop, homeActive=false");

        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MOutArmZA", true), 26, be);
        be.state[5] = 4; MotorAccessTick(be, 500);
        be.state[5] = 1; be.cmdPos[5] = 0.0; MotorAccessTick(be, 500);
        MotorAccessTick(be, 500);                                                   // phase 1（golden case 500）在下一拍下命令
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 8.0 && be.homeFlags[22] == 0,
              "arm Z: after homing -> moveAbs to ZSafePos 20 (card 8), HomeFlag still 0 (golden case 500)");
        be.cmdPos[5] = 8.0; MotorAccessTick(be, 500);
        CHECK(be.homeFlags[22] == 1 && MotorAccessJobs().homesActive == 0, "arm Z at ZSafePos -> HomeFlag=1");

        const int n0 = (int)be.executed.size();
        CHECK(!MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmXScale", true), 1, be).ok && (int)be.executed.size() == n0,
              "scale motor: golden ResetPos(0) -> EastSun forbids -> refused");
        CHECK(MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmPitch", true), 1, be).ok && be.smBegin.size() == 1 && be.smBegin[0] == 2, "non-1203 home -> golden InitProcessSingleMotorTask (RULINGS_20261001 #8; was refused while ProcessSingleMotorHome was a stub)");
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MOutArmY", true), 27, be);
        be.state[3] = 4; MotorAccessTick(be, 500);
        MotorAccessDispatch(Req("uMotorTest", "stop", 0), 28, be);
        CHECK(MotorAccessJobs().homesActive == 0, "STOP cancels homing (golden AllBtnUp + bSingleHome=false)");
        CHECK(!MotorAccessDispatch(Req("uteach", "home", "MOutArmY"), 1, be).ok, "uteach home without start -> refused");

        // LoopMove
        MotorAccessResetJobs();
        be.state[3] = 1; be.cmdPos[3] = 400.0;
        MotorAccessReq lr = ReqStart("uMotorTest", "loopMove", "MOutArmY", true);
        lr.num["pos1"] = 2000; lr.num["pos2"] = 500; lr.num["waitTime"] = 5; lr.num["speed"] = 50;
        be.golden[20].homeFlag = 0;
        o = MotorAccessDispatch(lr, 30, be);
        CHECK(!o.ok && Has(o.ackJson, "not home yet") && !MotorAccessJobs().loopActive, "not homed, no confirm -> golden question, no loop");
        lr.flag["confirmNotHomed"] = true;
        int e0 = (int)be.executed.size();
        o = MotorAccessDispatch(lr, 31, be);
        CHECK(o.ok && MotorAccessJobs().loopActive && (int)be.executed.size() == e0 + 4, "confirmed -> loop starts, speed set (4 cmds)");
        be.golden[20].homeFlag = 1;
        MotorAccessTick(be, 500);
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 800.0, "tick -> moveAbs pos1 2000 (card 800)");
        be.cmdPos[3] = 800.0; MotorAccessTick(be, 500);
        CHECK(MotorAccessJobs().loopTask == 10 && MotorAccessJobs().loopCount == 0, "arrived at pos1 -> wait (waitTime 5 = 500 ms), count still 0 (golden counts on the pos2 leg)");
        MotorAccessTick(be, 500);
        CHECK(MotorAccessJobs().loopTask == 50, "wait done -> leg 2");
        MotorAccessTick(be, 500);
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 200.0, "moveAbs pos2 500 (card 200)");
        be.cmdPos[3] = 200.0; MotorAccessTick(be, 500);
        CHECK(MotorAccessJobs().loopCount == 1 && MotorAccessJobs().loopTask == 60, "arrived at pos2 -> count 1 (golden dwLoopCount++), wait");
        MotorAccessReq lrOff = lr; lrOff.flag["start"] = false;
        o = MotorAccessDispatch(lrOff, 32, be);
        CHECK(o.ok && Has(o.ackJson, "loopStopped") && be.executed.back().kind == kCmdAxStop && !MotorAccessJobs().loopActive &&
              Has(o.ackJson, "\"loopActive\":false"), "LoopMove released (start=false) -> stop, loopActive=false");

        MotorAccessReq lr2 = lr; lr2.num["waitTime"] = 0;
        o = MotorAccessDispatch(lr2, 33, be);
        be.golden[20].softP = 1500;
        MotorAccessTick(be, 500);
        CHECK(!MotorAccessJobs().loopActive && Has(MotorAccessJobs().lastNote, "soft limit"),
              "soft limit hit mid-loop -> loop stops and says why (golden would spin on the message)");
        be.golden[20].softP = 999999;

        MotorAccessReq lm = ReqStart("uMotorTest", "loopMove", "MInArmPitch", true);
        lm.num["pos1"] = 100; lm.num["pos2"] = 200; lm.num["waitTime"] = 0;
        be.moveRet = 1;
        o = MotorAccessDispatch(lm, 34, be);
        MotorAccessTick(be, 500);
        MotorAccessTick(be, 500);
        CHECK(o.ok && MotorAccessJobs().loopCount == 1 && be.moves.back().target == 200,
              "non-1203 loop: golden MotorMove polled until 1, pos1 -> pos2");
        MotorAccessDispatch(Req("uMotorTest", "stop", 0), 35, be);
        CHECK(!MotorAccessJobs().loopActive, "STOP cancels the loop");

        // ResetMNet / Range-Rate / blocked
        CHECK(!MotorAccessDispatch(Req("uMotorTest", "resetMNet", 0), 1, be).ok && be.resetCount == 0, "resetMNet without confirm -> refused");
        MotorAccessReq rm = Req("uMotorTest", "resetMNet", 0); rm.flag["confirm"] = true;
        be.systemStart = true;
        CHECK(!MotorAccessDispatch(rm, 1, be).ok && be.resetCount == 0, "resetMNet while SystemStart -> refused (golden)");
        be.systemStart = false;
        CHECK(MotorAccessDispatch(rm, 1, be).ok && be.resetCount == 1, "resetMNet confirmed, idle -> ResetMNet(0)");
        MotorAccessReq rr = Req("uMotorTest", "setRangeAndInit", "MOutArmY"); rr.num["value"] = 100;
        o = MotorAccessDispatch(rr, 1, be);
        CHECK(o.ok && be.rangeRates.empty() && be.rangeMem.count(20) && be.rangeMem[20] == 100 && Has(o.ackJson, "\"initMotor\""),
              "AI(W906-MT-E3c) R4: 1203 range -> SetRange in memory + the full InitMotor through Pci1203Execute, never the golden object's InitMotor (was: refused)");
        rr.motors[0] = "MInArmPitch";
        o = MotorAccessDispatch(rr, 1, be);
        CHECK(o.ok && be.rangeRates.size() == 1 && be.rangeRates[0].mi == 2 && be.rangeRates[0].range && be.rangeRates[0].v == 100,
              "MN200 range -> golden SetRange + InitMotor + HomeFlag=0");
        o = MotorAccessDispatch(Req("uMotorTest", "motorPowerToggle", 0), 1, be);
        CHECK(o.ok && be.powerOnBegins == 1 && Has(o.ackJson, "\"caption\":\"Motor Power On\""),
              "AI(W906-MT-E3c): motorPowerToggle is live (relay off -> DoMotorPowerOn; was blocked on G9)");
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (9)
    //  AI(W906-W4-MOTOR) 20260925: 每一種成功 ack 都不可帶傳輸層保留字。端對端探針量到 ack 的 "id":"cmd-N" 蓋掉 WS 的 id，
    //  頁面收不到任何成功回應 —— 單元測試原本只看 state／seq，看不出來。
    printf("[9] every ok ack keeps type/id/ok/error free (WS ack splice)\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 999999; g.softN = -999999;
        g.jogHigh = 10000; g.initSpeed = 500; g.homeFlag = 1; g.homeHigh = 1000;
        FakeBackend be;
        be.table["A"] = Axis1203(20, 20, 0, 3);
        be.table["M"] = AxisOther(2, "MN200", true);
        be.golden[20] = g; be.golden[2] = g;
        be.opened.assign(4, true); be.cmdPos[3] = 0.0; be.state[3] = 1; be.readPos[2] = 0; be.moveRet = 0;
        be.goldenSvon[2] = false;
        std::vector<MotorAccessReq> rs;
        MotorAccessReq q;
        q = Req("uMotorTest", "setPos1", "A"); rs.push_back(q);
        q = Req("uMotorTest", "stop", 0); q.button = "btnStop"; rs.push_back(q);
        q = Req("uMotorTest", "stop", "A"); q.button = "sbMotorTest_JogP"; rs.push_back(q);
        q = Req("uMotorTest", "servoToggle", "A"); q.flag["servoOn"] = true; rs.push_back(q);
        q = Req("uMotorTest", "servoToggle", "M"); rs.push_back(q);
        q = Req("uMotorTest", "jogP", "A"); q.num["speed"] = 10; rs.push_back(q);
        q = Req("uMotorTest", "moveRelative", "A"); q.button = "sbMotorTest_MoveP"; q.num["interval"] = 10; rs.push_back(q);
        q = Req("uMotorTest", "moveAbsolute", "M"); q.num["targetPos"] = 5; rs.push_back(q);
        q = Req("uMotorTest", "setJogHighSpeed", "A"); q.num["speed"] = 10; rs.push_back(q);
        q = ReqStart("uMotorTest", "home", "A", true); rs.push_back(q);
        q = ReqStart("uMotorTest", "loopMove", "A", true); q.num["pos1"] = 1; q.num["pos2"] = 2; q.flag["confirmNotHomed"] = true; rs.push_back(q);   // 上一個 home 把 HomeFlag 設回 0
        q = Req("uMotorTest", "resetMNet", 0); q.flag["confirm"] = true; rs.push_back(q);
        q = Req("uMotorTest", "setRateAndInit", "M"); q.num["value"] = 5; rs.push_back(q);
        // AI(W906-MT-E2) 20260925: the four page actions
        q = Req("uMotorTest", "selectMotor", "A"); rs.push_back(q);
        q = Req("uMotorTest", "setParamCell", "A"); q.num["row"] = 2; q.num["value"] = 5000; rs.push_back(q);
        q = Req("uMotorTest", "copyFrom", "A"); q.num["source"] = 20; rs.push_back(q);
        q = Req("uMotorTest", "setSpeed", "A"); q.num["pct"] = 10; q.flag["jog"] = true; rs.push_back(q);
        q = Req("uMotorTest", "setSpeed", "M"); q.num["pct"] = 10; q.flag["jog"] = false; rs.push_back(q);
        int okN = 0, shapeN = 0;
        for (std::size_t i = 0; i < rs.size(); ++i) {
            const MotorAccessOutcome o = MotorAccessDispatch(rs[i], 1, be);
            if (!o.ok) { printf("    not ok: %s -> %s\n", rs[i].action.c_str(), o.ackJson.c_str()); continue; }
            ++okN;
            if (AckShapeOk(o.ackJson, 7)) ++shapeN;
            else printf("    bad shape: %s -> %s\n", rs[i].action.c_str(), o.ackJson.c_str());
        }
        CHECK(okN == (int)rs.size(), "all 18 success paths answered ok (13 + the 5 MT-E2 page commands)");
        CHECK(shapeN == okN, "none of them carries type/id/ok/error; request id travels as reqId");
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (10)
    printf("[10] NB2 R21: Enable selectability, floodgate, card ret, jog deadman, stale sample, index speed\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 999999; g.softN = -999999;
        g.jogHigh = 10000; g.initSpeed = 500; g.homeFlag = 1;
        MotorGolden gOff = g; gOff.enable = false; gOff.selectable = false;
        MotorGolden gSh = g; gSh.inShuttle = 1;
        MotorGolden gIx = g; gIx.indexMotor = true;
        FakeBackend be;
        be.table["A"]   = Axis1203(20, 20, 0, 3);
        be.table["OFF"] = Axis1203(21, 21, 0, 1);
        be.table["SH"]  = Axis1203(11, 11, 0, 2);
        be.table["IX"]  = Axis1203(14, 14, 0, 0);
        be.golden[20] = g; be.golden[21] = gOff; be.golden[11] = gSh; be.golden[14] = gIx;
        be.opened.assign(4, true);
        for (int ax = 0; ax < 4; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }

        // W4B-1
        MotorAccessReq q = Req("uMotorTest", "jogP", "OFF"); q.num["speed"] = 10;
        int n0 = (int)be.executed.size();
        CHECK(!MotorAccessDispatch(q, 1, be).ok && (int)be.executed.size() == n0, "W4B-1 jog on unselectable (Enable=0) axis -> refused, nothing sent");
        q = Req("uMotorTest", "home", "OFF");
        CHECK(!MotorAccessDispatch(q, 1, be).ok, "W4B-1 home on unselectable axis -> refused");
        q = Req("uMotorTest", "servoToggle", "OFF"); q.flag["servoOn"] = true;
        CHECK(!MotorAccessDispatch(q, 1, be).ok && Has(MotorAccessDispatch(q, 1, be).ackJson, "Enable=0"), "W4B-1 servo on unselectable axis -> refused");
        q = Req("uMotorTest", "setJogHighSpeed", "OFF"); q.num["speed"] = 10;
        CHECK(!MotorAccessDispatch(q, 1, be).ok, "W4B-1 parameter button on unselectable axis -> refused");
        CHECK(MotorAccessDispatch(Req("uMotorTest", "stop", "OFF"), 1, be).ok, "stop still works with an unselectable axis selected");

        // W4B-2
        be.floodReady = false;
        q = Req("uMotorTest", "moveRelative", "SH"); q.button = "sbMotorTest_MoveP"; q.num["interval"] = 100;
        n0 = (int)be.executed.size();
        MotorAccessOutcome o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "閘門") && be.floodCalls.size() == 1 && be.floodCalls[0] == 1 && (int)be.executed.size() == n0,
              "W4B-2 InShuttle1 move with floodgate not open -> gates commanded, move refused this time (golden)");
        be.floodReady = true;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveAbs, "W4B-2 floodgate open -> move goes");
        q = Req("uMotorTest", "moveRelative", "A"); q.button = "sbMotorTest_MoveP"; q.num["interval"] = 100;
        const std::size_t fc = be.floodCalls.size();
        MotorAccessDispatch(q, 1, be);
        CHECK(be.floodCalls.size() == fc, "W4B-2 non-shuttle axis never touches the floodgate");

        // W4B-3
        be.retNext = 0x80005111ul;
        q = Req("uMotorTest", "moveAbsolute", "A"); q.num["targetPos"] = 50;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "0x80005111"), "W4B-3 card error code on moveAbs -> ok=false with the vendor code (was success)");
        FakeBackend sb; sb.opened.assign(2, true); sb.retNext = 0x80005111ul;
        o = MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, sb);
        CHECK(o.ok && Has(o.ackJson, "\"partial\":true") && Has(o.ackJson, "\"stopRefused\":1"), "W4B-3 card error on one stop -> counted as refused, partial");

        // W4B-4
        q = Req("uMotorTest", "jogP", "A"); q.button = "sbMotorTest_JogP"; q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().jogsActive == 1, "W4B-4 jog registers");
        MotorAccessTick(be, 500, true);
        CHECK(MotorAccessJobs().jogsActive == 1, "W4B-4 operator still connected -> jog keeps going");
        n0 = (int)be.executed.size();
        MotorAccessTick(be, 500, false);
        CHECK(MotorAccessJobs().jogsActive == 0 && (int)be.executed.size() == n0 + 1 && be.executed.back().kind == kCmdAxStop &&
              be.executed.back().axis == 3 && Has(MotorAccessJobs().lastNote, "deadman"),
              "W4B-4 operator connection gone -> jogging axis stopped (deadman)");
        o = MotorAccessDispatch(q, 1, be);
        MotorAccessReq rel = Req("uMotorTest", "stop", "A"); rel.button = "sbMotorTest_JogP";
        MotorAccessDispatch(rel, 1, be);
        CHECK(MotorAccessJobs().jogsActive == 0, "W4B-4 release forgets the jog");

        // W4B-5
        be.freezePolls = true;
        q = Req("uMotorTest", "moveRelative", "A"); q.button = "sbMotorTest_MoveP"; q.num["interval"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "監看器還沒更新"), "W4B-5 no poll since the last command on this axis -> refused (stale READY/position)");
        be.freezePolls = false;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok, "W4B-5 after a new poll -> allowed");

        // W4B-6
        be.readSpeedV = 1234;
        q = Req("uMotorTest", "setJogHighSpeed", "IX"); q.num["speed"] = 50;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && be.params.back().value == 1234, "W4B-6 index axis: PJogHighSpeed = ReadSpeed() (golden SetSpeed skips index), not 0");
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (11)
    printf("[11] NB2 R22/R23: Enable=0 on 1203, HOME door/speed, case 500, Timer1Timer gates, explicit intent, busy guard\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 999999; g.softN = -999999;
        g.jogHigh = 10000; g.initSpeed = 500; g.homeFlag = 1; g.acc = 1000; g.dec = 1000;
        g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100;
        MotorGolden gOffSim = g; gOffSim.enable = false; gOffSim.selectable = true;   // 模擬組態：畫面選得到，但 golden InitMotor 不開這一軸
        MotorGolden gz = g; gz.armZ = true;
        FakeBackend be;
        be.table["A"]  = Axis1203(20, 20, 0, 3);
        be.table["B"]  = Axis1203(21, 21, 0, 4);
        be.table["E0"] = Axis1203(23, 23, 0, 1);
        be.table["Z"]  = Axis1203(22, 22, 0, 5);
        be.golden[20] = g; be.golden[21] = g; be.golden[23] = gOffSim; be.golden[22] = gz;
        be.table["E0"].tableEnable = false;                                        // Mot_Table Enable=0
        MotorGolden gSim = g; gSim.enable = false;                                 // SOFT_SIMULTE：golden Motor->Enable 一律 false，但 Mot_Table 是 1
        be.table["S"] = Axis1203(24, 24, 0, 2); be.golden[24] = gSim;
        be.opened.assign(6, true);
        for (int ax = 0; ax < 6; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }

        // R22
        int n0 = (int)be.executed.size();
        MotorAccessReq q = Req("uMotorTest", "jogP", "E0"); q.num["speed"] = 10;
        MotorAccessOutcome o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "InitMotor") && (int)be.executed.size() == n0, "R22 jog on 1203 Enable=0 (sim-selectable) -> refused, nothing sent");
        CHECK(!MotorAccessDispatch(ReqStart("uMotorTest", "home", "E0", true), 1, be).ok && (int)be.executed.size() == n0, "R22 home on Enable=0 -> refused");
        q = ReqStart("uMotorTest", "loopMove", "E0", true); q.num["pos1"] = 1; q.num["pos2"] = 2;
        CHECK(!MotorAccessDispatch(q, 1, be).ok && !MotorAccessJobs().loopActive, "R22 loop on Enable=0 -> refused");
        q = Req("uMotorTest", "servoToggle", "E0"); q.flag["servoOn"] = true;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "InitMotor") && (int)be.executed.size() == n0, "R22 servo on Enable=0 -> refused");
        CHECK(MotorAccessDispatch(Req("uMotorTest", "stop", "E0"), 1, be).ok, "R22 stop still works");
        q = Req("uMotorTest", "jogP", "S"); q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxJogStart, "R22 sim build (Motor->Enable=false, Mot_Table Enable=1) -> jog allowed: the guard reads the table, not golden's simulation flag");
        MotorAccessReq qr = Req("uMotorTest", "stop", "S"); qr.button = "sbMotorTest_JogP";
        MotorAccessDispatch(qr, 1, be);

        // W4C-5: explicit intent
        CHECK(!MotorAccessDispatch(Req("uMotorTest", "home", "A"), 1, be).ok, "W4C-5 home without start -> refused (old page)");
        q = Req("uMotorTest", "loopMove", "A"); q.num["pos1"] = 1; q.num["pos2"] = 2;
        CHECK(!MotorAccessDispatch(q, 1, be).ok && !MotorAccessJobs().loopActive, "W4C-5 loopMove without start -> refused (old page)");
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", false), 1, be);
        CHECK(o.ok && MotorAccessJobs().homesActive == 0 && be.executed.back().kind == kCmdAxStop && (int)be.executed.size() == n0 + 1 &&
              Has(o.ackJson, "\"homeActive\":false"), "W4C-5 home start=false with nothing running -> golden else branch: stop only, never starts");
        q = ReqStart("uMotorTest", "loopMove", "A", false); q.num["pos1"] = 1; q.num["pos2"] = 2;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && !MotorAccessJobs().loopActive && Has(o.ackJson, "\"loopActive\":false"), "W4C-5 loop start=false with nothing running -> stays stopped (was: toggle started a loop)");
        q.flag["start"] = true; q.num["mode"] = 1;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "ActiveIndex==-1") && !MotorAccessJobs().loopActive,
              "loop mode All with no motor selected -> refused like golden :1303-1307 (AI(W906-MT-E3c): All mode is wired; was: refused as unwired)");
        q.num.erase("mode");
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().loopActive && Has(o.ackJson, "\"loopActive\":true"), "loop start=true -> looping");
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().loopActive && (int)be.executed.size() == n0, "loop start=true again -> no restart, nothing sent");

        // W4C-6: busy guard
        q = Req("uMotorTest", "jogP", "B"); q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!Has(o.ackJson, "LoopMove 進行中"), "AI(W906-MT-AXISLOCK) 20260929: jog on ANOTHER axis while A loops -> not refused by the loop (EastSun: axes independent)");
        { MotorAccessReq rel = Req("uMotorTest", "stop", "B"); rel.button = "sbMotorTest_JogP"; MotorAccessDispatch(rel, 1, be); }
        CHECK(MotorAccessJobs().loopActive, "releasing B's jog leaves A's loop running");
        n0 = (int)be.executed.size();
        q = Req("uMotorTest", "jogP", "A"); q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "LoopMove 進行中") && (int)be.executed.size() == n0, "W4C-6 jog on the LOOPING axis -> refused (golden btnLoopMove->Down)");
        q = Req("uMotorTest", "moveRelative", "A"); q.button = "sbMotorTest_MoveP"; q.num["interval"] = 10;
        CHECK(!MotorAccessDispatch(q, 1, be).ok, "W4C-6 moveRelative while looping -> refused");
        q = Req("uMotorTest", "moveSoftLimitN", "A");
        CHECK(!MotorAccessDispatch(q, 1, be).ok, "W4C-6 moveSoftLimit on the looping axis -> refused");
        q = ReqStart("uMotorTest", "loopMove", "B", true); q.num["pos1"] = 1; q.num["pos2"] = 2;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "一次只能跑一組"), "AI(W906-MT-AXISLOCK) 20260929: LoopMove on another axis while A loops -> refused and says so (was a 'looping' ack for A)");
        q = Req("uMotorTest", "moveAbsolute", "B"); q.num["targetPos"] = 5;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!Has(o.ackJson, "進行中"), "W4C-6 moveAbsolute is NOT guarded (golden btnGoClick has no Down check)");

        // W4C-4: Timer1Timer gates
        MotorAccessTick(be, 500, true);
        CHECK(MotorAccessJobs().loopActive, "connected, no safe lock -> loop keeps going");
        n0 = (int)be.executed.size();
        MotorAccessTick(be, 500, false);
        CHECK(!MotorAccessJobs().loopActive && (int)be.executed.size() == n0 && Has(MotorAccessJobs().lastNote, "fShow"),
              "W4C-4 operator gone -> loop cancelled (golden fShow==false: no further legs; no stop, the running leg ends)");
        q = ReqStart("uMotorTest", "loopMove", "A", true); q.num["pos1"] = 1; q.num["pos2"] = 2;
        MotorAccessDispatch(q, 1, be);
        be.safeLock = true;
        MotorAccessTick(be, 500, true);
        CHECK(!MotorAccessJobs().loopActive && Has(MotorAccessJobs().lastNote, "IsSafeLockCheck"), "W4C-4 safe lock -> loop cancelled (golden Close())");
        be.safeLock = false;
        MotorAccessDispatch(q, 1, be);
        CHECK(MotorAccessJobs().loopActive, "loop again");
        n0 = (int)be.executed.size();  const std::size_t sw0 = be.stopSweep.size();   //AI(W906-INDEXZ-1203) 20260930 (same line)
        MotorAccessOnAlarm(be, "JAM0101");
        CHECK(!MotorAccessJobs().loopActive && (int)be.executed.size() == n0 + 6 && be.executed.back().kind == kCmdAxStop,
              "W4C-4 alarm -> loop cancelled and stop sent to all 6 opened 1203 axes (golden StopAllMotor)");  { bool allSweep = be.stopSweep.size() == sw0 + 6; for (std::size_t k = sw0; allSweep && k < be.stopSweep.size(); ++k) allSweep = be.stopSweep[k]; CHECK(allSweep && !MotorAccessInAlarmSweep(), "AI(W906-INDEXZ-1203) D2: the alarm's 6 stops ran inside MotorAccessAlarmSweep (the Index Z1 route keeps a halted Z1 job halted on them), the scope closed afterwards"); }

        // servo cancels jobs (golden btnServoOffClick AllBtnUp)
        MotorAccessDispatch(q, 1, be);
        q = Req("uMotorTest", "servoToggle", "B"); q.flag["servoOn"] = true;
        MotorAccessDispatch(q, 1, be);
        CHECK(MotorAccessJobs().loopActive, "AI(W906-MT-AXISLOCK) 20260929: servoToggle on ANOTHER axis leaves A's loop running (EastSun: axes independent)");
        q = Req("uMotorTest", "servoToggle", "A"); q.flag["servoOn"] = true;
        MotorAccessDispatch(q, 1, be);
        CHECK(!MotorAccessJobs().loopActive, "servoToggle (uMotorTest) on the looping axis cancels the loop (golden AllBtnUp)");

        // W4C-1 / W4C-3 / HomeFlag at press
        be.golden[20].homeFlag = 1;
        be.doorOpen.insert(20);
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", true), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "安全門") && (int)be.executed.size() == n0 && be.homeFlags[20] == 0,
              "W4C-1 home with door open -> refused, nothing sent, HomeFlag cleared at press (golden :1158)");
        be.doorOpen.clear();
        be.golden[20].homeHigh = 0;
        CHECK(!MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", true), 1, be).ok && (int)be.executed.size() == n0,
              "W4C-3 PHomeHighSpeed=0 -> refused (never home at whatever speed is left on the card)");
        be.golden[20].homeHigh = 2000;
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", true), 1, be);
        //AI(W906-HOME-VENDOR) 20260929: first the card's home family (Advantech Home examples / golden SetHomeSpeed), then the PTP seed
        bool speedOk = (int)be.executed.size() == n0 + 9;
        for (int i = 0; speedOk && i < 4; ++i) speedOk = be.executed[n0 + i].kind == kCmdAxSetHome && be.executed[n0 + i].axis == 3;
        speedOk = speedOk && be.executed[n0].home == kHomeVelLow && be.executed[n0].value == 500 && be.executed[n0 + 1].home == kHomeVelHigh &&
                  be.executed[n0 + 1].value == 2000 && be.executed[n0 + 2].home == kHomeAcc && be.executed[n0 + 2].value == 3000 &&
                  be.executed[n0 + 3].home == kHomeDec && be.executed[n0 + 3].value == 3100;
        for (int i = 4; speedOk && i < 8; ++i) speedOk = be.executed[n0 + i].kind == kCmdAxSetSpeed && be.executed[n0 + i].axis == 3;
        speedOk = speedOk && be.executed[n0 + 4].value == 500 && be.executed[n0 + 5].value == 2000 &&
                  be.executed[n0 + 6].value == 3000 && be.executed[n0 + 7].value == 3100 && be.executed[n0 + 8].kind == kCmdAxHome;
        CHECK(o.ok && speedOk && Has(o.ackJson, "\"homeActive\":true"),
              "W4C-3 home -> PAR_AxHomeVelLow/High/Acc/Dec 500/2000/3000/3100 (vendor example, golden SetHomeSpeed), then PTP 500/2000/3000/3100, then Acm_AxHome");
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", true), 1, be);
        CHECK(o.ok && (int)be.executed.size() == n0 && MotorAccessJobs().homesActive == 1, "home start=true while homing -> no second command");
        q = Req("uMotorTest", "jogN", "B"); q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!Has(o.ackJson, "HOME 進行中"), "AI(W906-MT-AXISLOCK) 20260929: jog on ANOTHER axis while A homes -> not refused by the home (EastSun: axes independent)");
        { MotorAccessReq rel = Req("uMotorTest", "stop", "B"); rel.button = "sbMotorTest_JogN"; MotorAccessDispatch(rel, 1, be); }
        CHECK(MotorAccessJobs().homesActive == 1, "releasing B's jog leaves A's HOME running");
        q = Req("uMotorTest", "jogN", "A"); q.num["speed"] = 10;
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "HOME 進行中") && (int)be.executed.size() == n0, "W4C-6 jog on the HOMING axis itself -> refused (golden btnHome->Down)");
        MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, be);

        // W4C-2: arm Z case 500, golden return values
        be.state[5] = 1;
        MotorAccessDispatch(ReqStart("uMotorTest", "home", "Z", true), 1, be);
        be.state[5] = 4; MotorAccessTick(be, 500);
        be.state[5] = 1; MotorAccessTick(be, 500);                                 // -> phase 1
        be.doorOpen.insert(22);
        n0 = (int)be.executed.size();
        MotorAccessTick(be, 500);
        CHECK(MotorAccessJobs().homesActive == 0 && be.homeFlags[22] == 1 && (int)be.executed.size() == n0,
              "W4C-2 door open at case 500 -> golden MotorMove -1 is truthy: HomeFlag=1, NO move");
        be.doorOpen.clear();
        MotorAccessDispatch(ReqStart("uMotorTest", "home", "Z", true), 1, be);
        be.state[5] = 4; MotorAccessTick(be, 500);
        be.state[5] = 1; MotorAccessTick(be, 500);
        be.locked.insert(22);
        n0 = (int)be.executed.size();
        MotorAccessTick(be, 500);
        CHECK(MotorAccessJobs().homesActive == 1 && (int)be.executed.size() == n0 + 1 && be.executed.back().kind == kCmdAxStop &&
              be.homeFlags[22] == 0, "W4C-2 locked at case 500 -> golden stops the axis and waits (no move, HomeFlag still 0)");
        be.locked.clear();
        MotorAccessTick(be, 500);
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 20.0, "unlocked -> moveAbs ZSafePos 20");
        be.cmdPos[5] = 20.0; MotorAccessTick(be, 500);
        CHECK(be.homeFlags[22] == 1 && MotorAccessJobs().homesActive == 0, "arrived -> HomeFlag=1");

        // range/rate cancels the loop before the 1203 refusal (golden first line)
        q = ReqStart("uMotorTest", "loopMove", "A", true); q.num["pos1"] = 1; q.num["pos2"] = 2;
        MotorAccessDispatch(q, 1, be);
        MotorAccessReq rr = Req("uMotorTest", "setRateAndInit", "A"); rr.num["value"] = 5;
        CHECK(MotorAccessDispatch(rr, 1, be).ok && !MotorAccessJobs().loopActive, "setRate on 1203 runs (R4), and the loop is cancelled first (golden :1366)");
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (9)
    //  AI(W906-MT-E1) 20260925: user EastSun's rulings -- the home branches on the DRIVE, and Reload Motor Data
    //  zeroes positions (SetCmd/ActualPosition allowed).
    printf("[9] AI(W906-MT-E1): home branch by drive type, reloadMotorData\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 2.5; g.softP = 999999; g.softN = -999999;
        g.jogHigh = 10000; g.initSpeed = 500; g.acc = 1000; g.dec = 1000; g.homeFlag = 1;
        g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100;
        MotorGolden gh1 = g; gh1.homeDirection = true;
        FakeBackend be;
        be.table["A"] = Axis1203(20, 20, 0, 3);
        be.table["B"] = Axis1203(30, 30, 0, 4);
        be.golden[20] = g; be.golden[30] = gh1;
        be.opened.assign(6, true); be.opened[2] = false;

        // unknown drive -> refused, nothing sent, HomeFlag cleared like golden's first line
        be.driveKind = -1;
        const std::size_t x0 = be.executed.size();
        MotorAccessOutcome o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", true), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "判斷不出") && be.executed.size() == x0 && MotorAccessJobs().homesActive == 0,
              "home on an axis whose drive type is unknown -> refused, no command (never guess MODE12 on a DS402 drive)");

        // DS402 -> Acm_AxHome 124/128, no zeroing afterwards
        be.driveKind = 1;
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "B", true), 2, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxHome && be.executed.back().homeMode == 124 && be.executed.back().dir == 1,
              "DS402 drive -> Acm_AxHome(124, +1) (the way every home on this machine succeeded)");
        be.state[4] = 4; MotorAccessTick(be, 500);
        be.state[4] = 1; MotorAccessTick(be, 500);
        bool zeroedDs402 = false;
        for (std::size_t i = 0; i < be.executed.size(); ++i)
            if (be.executed[i].kind == kCmdAxSetCmdPos || be.executed[i].kind == kCmdAxSetActPos) zeroedDs402 = true;
        CHECK(be.homeFlags[30] == 1 && !zeroedDs402, "DS402 home done -> HomeFlag=1 and NO SetCmd/ActualPosition (the drive sets its origin)");

        // another drive -> golden EtherCatMotHome: card home speeds, Acm_AxMoveHome(MODE12=11, dir), READY + 0.3 s -> zero
        be.driveKind = 0;
        const std::size_t y0 = be.executed.size();
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", true), 3, be);
        bool cardSeq = o.ok && be.executed.size() == y0 + 6 && be.executed[y0].kind == kCmdAxStop;   // golden case 1: DecStop first
        for (int i = 1; cardSeq && i < 5; ++i) cardSeq = be.executed[y0 + i].kind == kCmdAxSetHome;
        cardSeq = cardSeq && be.executed[y0 + 1].home == kHomeVelLow && be.executed[y0 + 1].value == 500.0 &&
                  be.executed[y0 + 2].home == kHomeVelHigh && be.executed[y0 + 2].value == 2000.0 &&
                  be.executed[y0 + 5].kind == kCmdAxMoveHome && be.executed[y0 + 5].homeMode == 11 && be.executed[y0 + 5].dir == -1;
        CHECK(cardSeq, "non-DS402 drive -> golden DecStop, SetHomeSpeed (PAR_AxHomeVel*/Acc/Dec), Acm_AxMoveHome(MODE12=11, HomeDirection 0 -> -1)");
        CHECK(!be.extDrive.empty() && be.extDrive.back().value == 0.0, "card-side home starts with golden DecStop (StopDec + ExtDrive 0)");
        be.state[3] = 4; MotorAccessTick(be, 500);
        be.state[3] = 1; MotorAccessTick(be, 500);                                  // first READY tick: golden HomeDelay starts
        const std::size_t z0 = be.executed.size();
        CHECK(be.homeFlags[20] == 0 && MotorAccessJobs().homesActive == 1, "card-side home: READY seen, still waiting the 0.3 s (golden case 20)");
        MotorAccessTick(be, 500);
        CHECK(be.executed.size() == z0 + 2 && be.executed[z0].kind == kCmdAxSetCmdPos && be.executed[z0].value == 0.0 &&
              be.executed[z0 + 1].kind == kCmdAxSetActPos && be.executed[z0 + 1].value == 0.0 &&
              be.homeFlags[20] == 1 && MotorAccessJobs().homesActive == 0,
              "card-side home done -> SetCommand(0), SetPosition(0), HomeFlag=1 (golden EtherCatMotHome case 20)");

        // reloadMotorData -> SetCmdPos(0) + SetActPos(0) on every opened axis (golden PCIL132_SetPos(0) per motor)
        const std::size_t r0 = be.executed.size();
        o = MotorAccessDispatch(Req("uMotorTest", "reloadMotorData", 0), 4, be);
        bool relOk = o.ok && be.executed.size() == r0 + 10 && Has(o.ackJson, "\"axesZeroed\":5");
        for (std::size_t i = r0; relOk && i < be.executed.size(); i += 2)
            relOk = be.executed[i].kind == kCmdAxSetCmdPos && be.executed[i + 1].kind == kCmdAxSetActPos &&
                    be.executed[i].axis == be.executed[i + 1].axis && be.executed[i].axis != 2;
        CHECK(relOk, "reloadMotorData -> SetCommand(0) then SetPosition(0) on each of the 5 opened axes (slot 2 not opened)");
        CHECK(be.clearAllHomeFlagsCalls == 1 && be.clearAllAtExecuted == r0,
              "reloadMotorData clears every HomeFlag BEFORE zeroing (golden InitialMotorParameter, cinitial.cpp:3615) -- coordinates reset => not homed");
        // AI(W906-MT-E2) 20260925: the table values come back too -- after the HomeFlag clear, before the zeroing (golden order)
        CHECK(be.reloadCalls == 1 && be.reloadAtClearCalls == 1 && be.reloadAtExecuted == r0,
              "reloadMotorData re-applies Mot_Table (GoldenReloadMotorParams) after the HomeFlag clear and before SetPos(0)");
        CHECK(Has(o.ackJson, "\"reload\":{\"read\":false") && Has(o.ackJson, "\"result\":\"partial\"") && Has(o.ackJson, "NOT re-read"),
              "Mot_Table could not be read -> the zeroing still runs (golden button), the ack says partial / NOT re-read");
        be.reloadResult.read = true; be.reloadResult.applied = 44; be.reloadResult.identityChanged = 1; be.reloadResult.firstChanged = "M03 (row added)";
        o = MotorAccessDispatch(Req("uMotorTest", "reloadMotorData", 0), 5, be);
        CHECK(o.ok && Has(o.ackJson, "\"applied\":44") && Has(o.ackJson, "\"restartNeeded\":true") && Has(o.ackJson, "M03 (row added)") &&
              Has(o.ackJson, "\"result\":\"partial\"") && Has(o.ackJson, "restart wb_serve"),
              "a row whose identity changed -> nothing applied to it, the ack says restart wb_serve");
        be.reloadResult.identityChanged = 0; be.reloadResult.firstChanged.clear();
        o = MotorAccessDispatch(Req("uMotorTest", "reloadMotorData", 0), 6, be);
        CHECK(o.ok && Has(o.ackJson, "\"result\":\"reloaded\"") && Has(o.ackJson, "\"restartNeeded\":false"), "everything re-applied -> reloaded");
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (12)
    //  AI(W906-MT-E2) 20260925: the Motor Test functions that needed no ruling. Every expected value is worked out by hand
    //  from the golden handler named in the message (golden uMotorTest.cpp line numbers).
    printf("[12] AI(W906-MT-E2): selectMotor / setSpeed + jog speed / setParamCell / copyFrom / alarm rule / LastHomePos / loop timing\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 2.5; g.softP = 999999; g.softN = -999999;
        g.jogHigh = 10000; g.jogLow = 100; g.initSpeed = 500; g.acc = 1000; g.dec = 2000; g.homeFlag = 1;
        g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100; g.range = 100; g.rate = 100;
        MotorGolden gOff = g; gOff.enable = false; gOff.selectable = false;
        MotorGolden gIx  = g; gIx.indexMotor = true;
        MotorGolden gSrc = g; gSrc.jogHigh = 12000; gSrc.jogLow = 120; gSrc.homeHigh = 3000; gSrc.homeLow = 300; gSrc.softP = 500000; gSrc.softN = -1000;
        FakeBackend be;
        be.paramsUpdateGolden = true;
        be.table["A"]   = Axis1203(20, 20, 0, 3);
        be.table["B"]   = Axis1203(21, 21, 0, 4);
        be.table["OFF"] = Axis1203(22, 22, 0, 5);
        be.table["IX"]  = Axis1203(14, 14, 0, 1);
        be.table["M"]   = AxisOther(2, "MN200", true);
        be.table["NUL"] = AxisOther(9, "MN200", false);
        be.golden[20] = g; be.golden[21] = g; be.golden[22] = gOff; be.golden[14] = gIx; be.golden[2] = g; be.golden[7] = gSrc;
        be.opened.assign(6, true);
        for (int ax = 0; ax < 6; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }

        // ---- selectMotor (golden lM00Click :734-760) ----
        MotorAccessReq lq = ReqStart("uMotorTest", "loopMove", "B", true); lq.num["pos1"] = 100; lq.num["pos2"] = 200;
        lq.flag["confirmNotHomed"] = true;                                      // B's HomeFlag drops to 0 once it is homed below
        CHECK(MotorAccessDispatch(lq, 1, be).ok && MotorAccessJobs().loopActive, "setup: a loop on B");
        std::size_t x0 = be.executed.size();
        MotorAccessOutcome o = MotorAccessDispatch(Req("uMotorTest", "selectMotor", "OFF"), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Enable=0") && Has(o.ackJson, "740-743") && MotorAccessJobs().loopActive &&
              be.executed.size() == x0 && be.speedCalls.empty() && MotorAccessJobs().selectedMotor.empty(),
              "select an Enable=0 motor -> refused with golden's reason BEFORE anything: the loop runs on, nothing sent, nothing selected");
        o = MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 31, be);
        bool selOk = o.ok && !MotorAccessJobs().loopActive && MotorAccessJobs().selectedMotor == "A" && be.executed.size() == x0 + 4;
        for (int i = 0; selOk && i < 4; ++i)
            selOk = be.executed[x0 + i].kind == kCmdAxSetSpeed && be.executed[x0 + i].axis == 3 && be.executed[x0 + i].wireId == 31;
        selOk = selOk && be.executed[x0].speed == kSpeedInit && be.executed[x0].value == 5.0 &&
                be.executed[x0 + 1].speed == kSpeedRun && be.executed[x0 + 1].value == 100.0 &&
                be.executed[x0 + 2].speed == kSpeedAcc && be.executed[x0 + 2].value == 1000.0 && be.executed[x0 + 3].speed == kSpeedDec;
        CHECK(selOk, "select A -> the loop goes up WITHOUT a stop (golden :744), golden SetSpeed(1) = the 4 PTP values at 1% (velLow 5, velHigh 100)");
        CHECK(be.speedCalls.size() == 1 && be.speedCalls[0].mi == 20 && be.speedCalls[0].pct == 1 && !be.speedCalls[0].jog,
              "select A -> MOT[20].SetSpeed(1) on the golden object too (its iSpeed = ReadSpeed / lblRealSpeed)");
        CHECK(Has(o.ackJson, "\"selectedMotor\":\"A\"") && Has(o.ackJson, "\"previousMotor\":null") && Has(o.ackJson, "\"cardSpeedSent\":true") &&
              Has(o.ackJson, "\"rate\":100") && Has(o.ackJson, "\"range\":100") && Has(o.ackJson, "\"readSpeed\":100") && AckShapeOk(o.ackJson, 7),
              "select ack: A selected, card speed sent, cur = rate/range (golden edtMotorRate/edtMotorRange :757-758), readSpeed = s of SetSpeed(1) = 100");
        be.ready = false; be.notReadyWhy = "no SDK";
        x0 = be.executed.size();
        o = MotorAccessDispatch(Req("uMotorTest", "selectMotor", "B"), 1, be);
        CHECK(o.ok && be.executed.size() == x0 && Has(o.ackJson, "\"cardSpeedSent\":false") && Has(o.ackJson, "no SDK") &&
              Has(o.ackJson, "\"previousMotor\":\"A\"") && MotorAccessJobs().selectedMotor == "B",
              "1203 control missing -> still selected (a screen action), the card speed is not sent and the ack says why");
        be.ready = true;
        CHECK(!MotorAccessDispatch(Req("uMotorTest", "selectMotor", "NUL"), 1, be).ok, "no golden object -> refused (golden would dereference NULL)");
        CHECK(!MotorAccessDispatch(Req("uteach", "selectMotor", "A"), 1, be).ok, "selectMotor from uteach -> refused (a uMotorTest screen action)");
        CHECK(!MotorAccessDispatch(Req("uMotorTest", "selectMotor", 0), 1, be).ok, "selectMotor without a motor -> refused");
        x0 = be.executed.size();
        o = MotorAccessDispatch(Req("uMotorTest", "selectMotor", "IX"), 1, be);
        CHECK(o.ok && be.executed.size() == x0 && Has(o.ackJson, "\"cardSpeedSent\":false"), "Index axis -> golden SetSpeed sets nothing: selected, nothing sent");
        o = MotorAccessDispatch(Req("uMotorTest", "selectMotor", "M"), 1, be);
        CHECK(o.ok && be.speedCalls.back().mi == 2 && be.speedCalls.back().pct == 1 && Has(o.ackJson, "\"layer\":\"MOT\""),
              "non-1203 -> golden MOT[2].SetSpeed(1) (its only path)");

        // ---- setSpeed (scroll bar :791-810 / edtSpeed :1587-1603) and the jog speed source (user ruling: Jog = golden) ----
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be);
        x0 = be.executed.size();
        MotorAccessReq sq = Req("uMotorTest", "setSpeed", "A"); sq.num["pct"] = 50; sq.flag["jog"] = true;
        o = MotorAccessDispatch(sq, 41, be);
        bool ssOk = o.ok && be.executed.size() == x0 + 8;
        const Pci1203SpeedParam order8[8] = { kSpeedInit, kSpeedRun, kSpeedAcc, kSpeedDec, kSpeedJogInit, kSpeedJogRun, kSpeedJogAcc, kSpeedJogDec };
        for (int i = 0; ssOk && i < 8; ++i)
            ssOk = be.executed[x0 + i].kind == kCmdAxSetSpeed && be.executed[x0 + i].speed == order8[i] && be.executed[x0 + i].wireId == 41;
        ssOk = ssOk && be.executed[x0].value == 250.0 && be.executed[x0 + 1].value == 5000.0 &&
               be.executed[x0 + 4].value == 250.0 && be.executed[x0 + 5].value == 5000.0;
        CHECK(ssOk, "scroll bar 50 (jog=true) -> golden SetSpeed(50, true): 4 PTP + 4 CFG_AxJog* at 50% (velLow 500*0.5=250, velHigh 5000)");
        CHECK(be.speedCalls.back().mi == 20 && be.speedCalls.back().pct == 50 && !be.speedCalls.back().jog &&
              Has(o.ackJson, "\"jogPct\":50") && Has(o.ackJson, "\"readSpeed\":5000"),
              "the golden object mirrored with SetSpeed(50) (an unopened 1203 object: memory only); ack readSpeed 5000");
        MotorAccessReq jq = Req("uMotorTest", "jogP", "A"); jq.button = "sbMotorTest_JogP"; jq.num["speed"] = 5;
        MotorAccessReq jr = Req("uMotorTest", "stop", "A"); jr.button = "sbMotorTest_JogP";
        x0 = be.executed.size();
        o = MotorAccessDispatch(jq, 1, be);
        CHECK(o.ok && be.executed.size() == x0 + 5 && be.executed[x0].speed == kSpeedJogInit && be.executed[x0].value == 250.0 &&
              be.executed[x0 + 1].value == 5000.0 && be.executed[x0 + 4].kind == kCmdAxJogStart,
              "jog after the scroll bar went to 50 -> the jog family at 50% (edtSpeed 5 ignored -- golden JogP drops it)");
        MotorAccessDispatch(jr, 1, be);
        MotorAccessReq eq = Req("uMotorTest", "setSpeed", "A"); eq.num["pct"] = 30; eq.flag["jog"] = false;
        x0 = be.executed.size();
        o = MotorAccessDispatch(eq, 1, be);
        CHECK(o.ok && be.executed.size() == x0 + 4 && be.executed[x0].speed == kSpeedInit && be.executed[x0].value == 150.0 &&
              be.executed[x0 + 1].value == 3000.0 && be.executed[x0 + 3].speed == kSpeedDec && Has(o.ackJson, "\"jogPct\":50"),
              "edtSpeed 30 (jog=false) -> golden SetSpeed(30): the 4 PTP values only (velLow 150, velHigh 3000); the jog stays at 50");
        x0 = be.executed.size();
        MotorAccessDispatch(jq, 1, be);
        CHECK(be.executed.size() == x0 + 5 && be.executed[x0 + 1].value == 5000.0, "jog after edtSpeed 30 -> still the scroll bar's 50%");
        MotorAccessDispatch(jr, 1, be);
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be);      // golden :755 scrlbrMotorSpeed->Position=1
        x0 = be.executed.size();
        MotorAccessDispatch(jq, 1, be);
        CHECK(be.executed.size() == x0 + 5 && be.executed[x0 + 1].value == 100.0, "selected again -> the jog is back at 1% (golden: the bar goes to 1)");
        MotorAccessDispatch(jr, 1, be);
        MotorAccessReq bad = sq; bad.num["pct"] = 0;
        CHECK(!MotorAccessDispatch(bad, 1, be).ok, "pct 0 -> refused (scroll bar Min=1)");
        bad.num["pct"] = 101;  CHECK(!MotorAccessDispatch(bad, 1, be).ok, "pct 101 -> refused (Max=100)");
        bad.num["pct"] = 50.5; CHECK(!MotorAccessDispatch(bad, 1, be).ok, "pct 50.5 -> refused (integer positions)");
        bad = sq; bad.flag.erase("jog"); CHECK(!MotorAccessDispatch(bad, 1, be).ok, "setSpeed without jog -> refused");
        MotorAccessReq ixq = Req("uMotorTest", "setSpeed", "IX"); ixq.num["pct"] = 40; ixq.flag["jog"] = true;
        x0 = be.executed.size();
        const std::size_t sc0 = be.speedCalls.size();
        o = MotorAccessDispatch(ixq, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"result\":\"ignored\"") && Has(o.ackJson, "Index") && be.executed.size() == x0 && be.speedCalls.size() == sc0,
              "Index axis -> golden TMyMotor::SetSpeed is empty for it: ignored, nothing sent, the ack says so");
        MotorAccessReq mq = Req("uMotorTest", "setSpeed", "M"); mq.num["pct"] = 60; mq.flag["jog"] = true;
        o = MotorAccessDispatch(mq, 1, be);
        CHECK(o.ok && be.speedCalls.size() == sc0 + 2 && be.speedCalls[sc0].mi == 2 && be.speedCalls[sc0].pct == 60 && be.speedCalls[sc0].jog &&
              !be.speedCalls[sc0 + 1].jog && be.executed.size() == x0,
              "non-1203 scroll bar -> golden MOT.SetSpeed(60, true), then edtSpeedChange's SetSpeed(60); no 1203 command");
        MotorAccessReq mj = Req("uMotorTest", "jogN", "M"); mj.button = "sbMotorTest_JogN"; mj.num["speed"] = 5;
        be.readPos[2] = 0;
        o = MotorAccessDispatch(mj, 1, be);
        CHECK(o.ok && be.jogs.back().mi == 2 && be.jogs.back().pct == 60, "non-1203 jog -> at the scroll bar's 60 (golden)");
        { MotorAccessReq rel = Req("uMotorTest", "stop", "M"); rel.button = "sbMotorTest_JogN"; MotorAccessDispatch(rel, 1, be); }
        MotorAccessDispatch(ReqStart("uMotorTest", "home", "B", true), 1, be);
        CHECK(MotorAccessJobs().homesActive == 1, "setup: HOME running on B");
        x0 = be.executed.size();
        { MotorAccessReq sqB = sq; sqB.motors[0] = "B"; o = MotorAccessDispatch(sqB, 1, be); }
        CHECK(o.ok && Has(o.ackJson, "\"result\":\"ignored\"") && Has(o.ackJson, "bSingleHome") && be.executed.size() == x0,
              "scroll bar of the HOMING axis -> golden :796 `if(bSingleHome==true) return;`: nothing sent");
        o = MotorAccessDispatch(sq, 1, be);
        CHECK(o.ok && !Has(o.ackJson, "\"result\":\"ignored\"") && be.executed.size() > x0,
              "AI(W906-MT-AXISLOCK) 20260929: scroll bar of ANOTHER axis while B homes -> set (EastSun: axes independent; golden bSingleHome was page-wide)");
        x0 = be.executed.size();
        o = MotorAccessDispatch(eq, 1, be);
        CHECK(o.ok && be.executed.size() == x0 + 4, "edtSpeed while HOME runs -> set (golden edtSpeedChange has no bSingleHome check)");
        MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, be);

        // ---- setParamCell (golden strngrdMotorSelectCell :1176-1222) ----
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be);
        be.golden[20].homeFlag = 1; be.homeFlags.erase(20);
        CHECK(MotorAccessDispatch(lq, 1, be).ok && MotorAccessJobs().loopActive, "setup: a loop on B again");
        x0 = be.executed.size();
        MotorAccessReq pc = Req("uMotorTest", "setParamCell", "A"); pc.num["row"] = 2; pc.num["value"] = 7000;
        o = MotorAccessDispatch(pc, 1, be);
        //AI(W906-MT-SPDLIVE) 20261001: EastSun 1001 「當我百分比速度有變動時 請要直接改速度」 -- changed from golden (memory only):
        //  a row that the speed is made of re-applies it at once: jog family (4) at the bar's pct + PTP family (4) at the last SetSpeed's.
        CHECK(o.ok && be.cells.size() == 1 && be.cells[0].mi == 20 && be.cells[0].row == 2 && be.cells[0].v == 7000.0 &&
              Has(o.ackJson, "\"jogHigh\":7000") && be.executed.size() == x0 + 8 && Has(o.ackJson, "speed re-applied now") &&
              AckShapeOk(o.ackJson, 7),
              "row 2 = 7000 -> PJogHighSpeed=ret AND the speed re-applied to the card now (MT-SPDLIVE), the ack's cur shows what C++ now holds");
        {
            //  the next jog re-sends exactly the re-applied jog values (no bar wiggle needed): its jog-family run speed = 7000 x 1% = 70
            const std::size_t j0 = be.executed.size();
            MotorAccessOutcome oj = MotorAccessDispatch(Req("uMotorTest", "jogP", "A"), 1, be);
            bool saw70 = false;
            for (std::size_t k = j0; k < be.executed.size(); ++k)
                if (be.executed[k].kind == kCmdAxSetSpeed && be.executed[k].speed == kSpeedJogRun && be.executed[k].value == 70.0) saw70 = true;
            CHECK(oj.ok && saw70, "MT-SPDLIVE: the jog after the row-2 edit runs at the NEW JogHigh (7000 x 1% = 70), no scroll needed");
            MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, be);
        }
        CHECK(!MotorAccessJobs().loopActive && be.homeFlags.count(20) && be.homeFlags[20] == 0,
              "golden SelectCell :1219-1220 -> LoopMove up (no stop) and HomeFlag=0");
        pc.num["row"] = 3; pc.num["value"] = 12.7;
        MotorAccessDispatch(pc, 1, be);
        CHECK(be.cells.back().row == 3 && be.cells.back().v == 12.0, "an integer row takes golden atoi: 12.7 -> 12");
        pc.num["value"] = -12.7;
        MotorAccessDispatch(pc, 1, be);
        CHECK(be.cells.back().v == -12.0, "atoi truncates toward zero: -12.7 -> -12");
        pc.num["row"] = 8; pc.num["value"] = 1234.5;
        o = MotorAccessDispatch(pc, 1, be);
        CHECK(be.cells.back().row == 8 && be.cells.back().v == 1234.5 && Has(o.ackJson, "\"acc\":1234.5"),
              "row 8 (Acc) takes golden atof: SetAccDataBase(1234.5), the ack's cur acc = ReadAcc() = 1234.5");
        pc.num["row"] = 10; pc.num["value"] = 5000;
        o = MotorAccessDispatch(pc, 1, be);
        CHECK(be.cells.back().row == 10 && Has(o.ackJson, "\"range\":1000"),
              "row 10 (Range) -> SetRange; the ack reads back what the object kept (the fake caps at 1000 like TMyEtherCatMotor::SetRange)");
        const std::size_t nc = be.cells.size();
        pc.num["row"] = 0;   CHECK(!MotorAccessDispatch(pc, 1, be).ok, "row 0 (the header) -> refused (golden `ARow==0 -> return`)");
        pc.num["row"] = 11;  CHECK(!MotorAccessDispatch(pc, 1, be).ok, "row 11 -> refused (the grid has rows 1..10)");
        pc.num["row"] = 2.5; CHECK(!MotorAccessDispatch(pc, 1, be).ok, "row 2.5 -> refused");
        pc.num["row"] = 2; pc.num.erase("value"); CHECK(!MotorAccessDispatch(pc, 1, be).ok, "no value -> refused");
        pc.num["value"] = 3e10; CHECK(!MotorAccessDispatch(pc, 1, be).ok, "a value outside int for an integer row -> refused");
        CHECK(be.cells.size() == nc, "the refusals wrote nothing");
        MotorAccessReq po = Req("uMotorTest", "setParamCell", "OFF"); po.num["row"] = 2; po.num["value"] = 1;
        CHECK(!MotorAccessDispatch(po, 1, be).ok && be.cells.size() == nc, "an unselectable motor -> refused");

        // ---- copyFrom (golden BitBtn1Click :1384-1401) ----
        CHECK(MotorAccessDispatch(lq, 1, be).ok && MotorAccessJobs().loopActive, "setup: a loop on B again");
        be.homeFlags.erase(20);
        MotorAccessReq cq = Req("uMotorTest", "copyFrom", "A"); cq.num["source"] = 7;
        const std::size_t np = be.params.size();
        o = MotorAccessDispatch(cq, 1, be);
        bool cpOk = o.ok && be.params.size() == np + 6;
        const MotorParamWhich wh[6] = { kParamJogHigh, kParamJogLow, kParamHomeHigh, kParamHomeLow, kParamSoftP, kParamSoftN };
        const int want[6] = { 12000, 120, 3000, 300, 500000, -1000 };
        for (int i = 0; cpOk && i < 6; ++i) cpOk = be.params[np + i].mi == 20 && be.params[np + i].which == wh[i] && be.params[np + i].value == want[i];
        CHECK(cpOk, "copyFrom M07 -> A: PJogHigh/PJogLow/PHomeHigh/PHomeLow/PSoftLimitP/PSoftLimitN = MOT[7]'s, in golden's order");
        CHECK(MotorAccessJobs().loopActive && be.homeFlags.count(20) == 0 && Has(o.ackJson, "\"softP\":500000") && Has(o.ackJson, "\"source\":7"),
              "copyFrom leaves HomeFlag and LoopMove alone (golden), the ack's cur shows the copied values");
        cq.num["source"] = 25;
        o = MotorAccessDispatch(cq, 1, be);
        //AI(W906-MT-COPYALL) 20261003: golden's `Source>24 return` dropped (HT9050 has M35..M42): 25 is refused only because MOT[25] has no
        //  motor in this fake; a number beyond the MOT[] range is refused by the range check.
        CHECK(!o.ok && Has(o.ackJson, "NULL") && be.params.size() == np + 6, "source 25 (no MOT[25] motor here) -> refused, nothing copied (golden's Source>24 limit is gone: M35 copies on HT9050)");
        cq.num["source"] = 3000;  o = MotorAccessDispatch(cq, 1, be);
        CHECK(!o.ok && be.params.size() == np + 6, "source 3000 -> refused (not a MOT[] index)");
        cq.num["source"] = -1;  CHECK(!MotorAccessDispatch(cq, 1, be).ok, "source -1 -> refused (golden Source<0)");
        cq.num["source"] = 10;
        o = MotorAccessDispatch(cq, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "NULL") && be.params.size() == np + 6, "a source whose MOT[].Motor is NULL -> refused, nothing copied");
        MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, be);

        // ---- the alarm rule (golden UpdateMotorLed :644-652) ----
        MotorAccessResetJobs();
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be);
        be.golden[20].homeFlag = 1; be.homeFlags.erase(20);
        be.motionIO[3] = 0x00004000ul;                                          // SVON only
        MotorAccessOnAlarm(be, "JAM0101");
        CHECK(be.homeFlags.count(20) == 0, "alarm while the selected motor's alarm lamp is off -> its HomeFlag is left alone");
        be.motionIO[3] = 0x00004002ul;                                          // + ALM
        std::size_t s0 = be.stopMotor.size();
        MotorAccessOnAlarm(be, "JAM0102");
        CHECK(be.homeFlags.count(20) && be.homeFlags[20] == 0 && Has(MotorAccessJobs().lastNote, "UpdateMotorLed") && be.stopMotor.size() == s0,
              "alarm + the selected 1203 motor's ALM lamp -> HomeFlag=0 (golden UpdateMotorLed); its stop is the 1203 stop-all (DecStop)");
        be.motionIO.erase(3); be.state[3] = 3; be.homeFlags.erase(20);
        MotorAccessOnAlarm(be, "JAM0103");
        CHECK(be.homeFlags.count(20) && be.homeFlags[20] == 0, "ERROR_STOP lights the alarm lamp too (golden ScanMotorStatus)");
        be.state[3] = 1;
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "M"), 1, be);
        be.alarmLed[2] = true; be.homeFlags.erase(2); s0 = be.stopMotor.size();
        be.ready = false;
        MotorAccessOnAlarm(be, "JAM0104");
        CHECK(be.homeFlags.count(2) && be.homeFlags[2] == 0 && be.stopMotor.size() == s0 + 1 && be.stopMotor.back() == 2,
              "non-1203 selected motor with its alarm lamp on -> golden PCIL132_StopMotor + HomeFlag=0, even with no 1203 control");
        be.ready = true;
        MotorAccessResetJobs();
        be.homeFlags.clear();
        MotorAccessOnAlarm(be, "JAM0105");
        CHECK(be.homeFlags.empty(), "nothing selected -> the rule does nothing");

        // ---- LastHomePos (golden EtherCatMotHome case 20 `LastHomePos=-ReadPos();`) ----
        be.driveKind = 1;
        be.state[4] = 1; be.cmdPos[4] = 4.0;
        MotorAccessDispatch(ReqStart("uMotorTest", "home", "B", true), 1, be);
        be.state[4] = 4; MotorAccessTick(be, 500);
        be.state[4] = 1; MotorAccessTick(be, 500);
        CHECK(be.homeFlags[21] == 1 && be.lastHome.count(21) && be.lastHome[21] == -10,
              "DS402 home done -> LastHomePos = -ReadPos() = -(int)(4 x 2.5) = -10 (golden formula; the drive defined the origin)");
        be.driveKind = 0;
        be.cmdPos[3] = 400.9;
        MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", true), 1, be);
        be.state[3] = 4; MotorAccessTick(be, 500);
        be.state[3] = 1; MotorAccessTick(be, 500);
        const std::size_t z0 = be.executed.size();
        MotorAccessTick(be, 500);
        CHECK(be.lastHome.count(20) && be.lastHome[20] == -1000 && be.lastHomeAtExecuted == z0 &&
              be.executed.size() == z0 + 2 && be.executed[z0].kind == kCmdAxSetCmdPos,
              "card-side home -> LastHomePos = -(int)((int)400.9 x 2.5) = -1000, taken BEFORE SetCommand(0)/SetPosition(0)");
        o = MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be);
        CHECK(Has(o.ackJson, "\"lastHomePos\":-1000"), "the ack's cur carries LastHomePos (golden edtHomeOffset :965)");
        be.driveKind = 1;
        be.golden[21].direction = true; be.lastHome.erase(21);
        MotorAccessDispatch(ReqStart("uMotorTest", "home", "B", true), 1, be);
        be.state[4] = 4; MotorAccessTick(be, 500);
        be.state[4] = 1; MotorAccessTick(be, 500);
        //AI(W906-MERGE-56bbf785) 20260926: was "Direction=1 axis -> no LastHomePos (withheld, §0 #5)" -- ruled since: 6B (RULINGS_20260925
        //  #13, a 1203 axis ignores Direction), which the laptop applied; the machine's copy of the hold is removed with it.
        CHECK(be.homeFlags[21] == 1 && be.lastHome.count(21) && be.lastHome[21] == -10,
              "ruling 6B: Direction=1 1203 axis -> LastHomePos recorded like any axis (-(int)(4 x 2.5) = -10)");
        be.golden[21].direction = false;

        // ---- loop timing (golden DoLoopMove :393-610 with TQPF_Timer), a fake QPC clock ----
        MotorAccessResetJobs();
        be.clockMs = 0.0;
        be.golden[20].homeFlag = 1;
        be.state[3] = 1; be.cmdPos[3] = 0.0;
        MotorAccessReq tl = ReqStart("uMotorTest", "loopMove", "A", true);
        tl.num["pos1"] = 1000; tl.num["pos2"] = 500; tl.num["waitTime"] = 0;
        MotorAccessReq off = tl; off.flag["start"] = false;
        CHECK(MotorAccessDispatch(tl, 1, be).ok, "setup: a timed loop on A (homed)");
        MotorAccessJobState js = MotorAccessJobs();
        CHECK(js.hasAvgTime && js.avgTime == 0.0 && !js.hasJogPTime && !js.hasJogNTime,
              "homed start -> lblAvgTime=0 (golden :1334); lblJogPTime / lblJogNTime never measured yet");
        MotorAccessPollTick(be);
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 400.0, "t=0 post-Poll tick -> leg 1 issued (pos1 1000 -> card 400), latched");
        be.clockMs = 200.0; be.state[3] = 5; MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().loopTask == 1 && !MotorAccessJobs().hasJogPTime, "t=200 still moving");
        be.clockMs = 1234.6; be.state[3] = 1; be.cmdPos[3] = 400.0; MotorAccessPollTick(be);
        js = MotorAccessJobs();
        CHECK(js.hasJogPTime && js.jogPTime == 1234 && js.loopTask == 50 && js.loopCount == 0,
              "t=1234.6 arrival at pos1 on the fresh sample -> lblJogPTime = (int)(float)1234.6 = 1234 ms; count still 0");
        be.clockMs = 1300.0; MotorAccessPollTick(be);
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 200.0, "t=1300 leg 2 issued (pos2 500 -> card 200)");
        be.clockMs = 2100.9; be.cmdPos[3] = 200.0; MotorAccessPollTick(be);
        js = MotorAccessJobs();
        CHECK(js.hasJogNTime && js.jogNTime == 800 && js.loopCount == 1 && js.hasAvgTime && js.avgTime == 800.0 && js.jogPTime == 1234,
              "t=2100.9 pos2 -> lblJogNTime 800, dwLoopCount 1, Average 800 -> lblAvgTime 800");
        be.clockMs = 2200.0; MotorAccessPollTick(be);
        be.clockMs = 3200.0; be.cmdPos[3] = 400.0; MotorAccessPollTick(be);
        be.clockMs = 3300.0; MotorAccessPollTick(be);
        be.clockMs = 4500.0; be.cmdPos[3] = 200.0; MotorAccessPollTick(be);
        js = MotorAccessJobs();
        CHECK(js.jogPTime == 1000 && js.jogNTime == 1200 && js.loopCount == 2 && js.avgTime == 1000.0,
              "2nd cycle: lblJogPTime 1000, lblJogNTime 1200, lblAvgTime (800+1200)/2 = 1000 -- the pos1 legs are NOT averaged (golden :560)");
        be.clockMs = 4600.0; MotorAccessTick(be, 500);
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 400.0, "the 500 ms beat steps the same job on the same clock (leg 1, latched at 4600)");
        be.clockMs = 5000.0;
        MotorAccessDispatch(off, 1, be);
        CHECK(!MotorAccessJobs().loopActive && MotorAccessJobs().jogPTime == 1000 && MotorAccessJobs().avgTime == 1000.0,
              "stopped mid-leg: the labels keep their values (golden captions)");
        be.clockMs = 6000.0;
        MotorAccessDispatch(tl, 1, be);
        js = MotorAccessJobs();
        CHECK(js.loopActive && js.loopCount == 0 && js.avgTime == 0.0 && js.jogNTime == 1200,
              "homed restart -> dwLoopCount 0, Average / lblAvgTime 0; lblJogNTime keeps 1200 (golden never clears it)");
        MotorAccessPollTick(be);
        be.clockMs = 6500.0; be.cmdPos[3] = 400.0; MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().jogPTime == 1900,
              "golden quirk kept: after a mid-leg stop the next first leg is timed from the OLD latch (`static bool flag`): 6500-4600 = 1900, not 500");
        be.clockMs = 7000.0; MotorAccessPollTick(be);
        be.clockMs = 7400.0; be.cmdPos[3] = 200.0; MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().avgTime == 400.0 && MotorAccessJobs().loopCount == 1, "after the restart: Average 400 -> lblAvgTime 400");
        MotorAccessDispatch(off, 1, be);
        be.golden[20].homeFlag = 0;
        MotorAccessReq nh = tl; nh.flag["confirmNotHomed"] = true;
        MotorAccessDispatch(nh, 1, be);
        js = MotorAccessJobs();
        CHECK(js.loopActive && js.loopCount == 0 && js.avgTime == 400.0,
              "not-homed start (confirmed) -> dwLoopCount 0, but Average / lblAvgTime kept (golden resets them on the homed branch only)");
        be.clockMs = 8000.0; MotorAccessPollTick(be);
        be.clockMs = 8100.0; be.cmdPos[3] = 400.0; MotorAccessPollTick(be);
        be.clockMs = 8200.0; MotorAccessPollTick(be);
        be.clockMs = 8800.0; be.cmdPos[3] = 200.0; MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().loopCount == 1 && MotorAccessJobs().jogNTime == 600 && MotorAccessJobs().avgTime == 1000.0,
              "golden quirk kept: the kept Average 400 + this leg 600 over the new count 1 = 1000");
        MotorAccessDispatch(off, 1, be);
        be.golden[20].homeFlag = 1;
        MotorAccessReq wl = tl; wl.num["waitTime"] = 5;                          // golden Set0_1SecAndOn(5) = 0.5 s
        be.clockMs = 10000.0; be.cmdPos[3] = 0.0;
        MotorAccessDispatch(wl, 1, be);
        MotorAccessPollTick(be);
        be.clockMs = 10300.0; be.cmdPos[3] = 400.0; MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().loopTask == 10, "arrival with waitTime 5 -> waiting (golden LoopWait)");
        be.clockMs = 10700.0; MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().loopTask == 10, "t+400 ms: still waiting");
        be.clockMs = 10800.0; MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().loopTask == 50, "t+500 ms: the wait is over on the clock (not rounded to 500 ms beats)");
        MotorAccessDispatch(off, 1, be);
        be.moveRet = 0;
        be.golden[2].homeFlag = 1;                                              // the alarm rule above cleared it
        MotorAccessReq ml = ReqStart("uMotorTest", "loopMove", "M", true); ml.num["pos1"] = 10; ml.num["pos2"] = 20; ml.num["waitTime"] = 0;
        CHECK(MotorAccessDispatch(ml, 1, be).ok, "setup: a golden-object loop on M");
        const std::size_t nm = be.moves.size();
        MotorAccessPollTick(be);
        CHECK(be.moves.size() == nm, "a non-1203 (golden object) loop is not stepped by the post-Poll tick");
        MotorAccessTick(be, 500);
        CHECK(be.moves.size() == nm + 1, "... the beat steps it (golden MotorMove polled), as before");
        be.clockMs = -1.0;
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (13)
    //  AI(W906-MT-E3c) 20260925: Motor Power, FormShow / FormClose, Test Range / Rate (full InitMotor), R2 (M14), All-mode
    //  loop, VerifyMotorAction hooks, Light Scale. Every expected value is worked out by hand from the golden lines named.
    printf("[13] AI(W906-MT-E3c): motor power / form show-close / InitMotor / R2 / All loop / engine hooks / Light Scale\n");
    {
        // ---- pure golden arithmetic and text ----
        MotorGoldenRate gr = MotorRateFromGolden(90, 10000, 500, 100, 100);
        CHECK(!gr.skip && std::fabs(gr.dAcc - 0.106875) < 1e-15 && gr.accPersent == 1 && gr.accMin == 2001 && gr.accMax == 8192000 && gr.rate == 8192000.0,
              "SetRate(90), JogHigh 10000, Init 500, iSpeed 100, Range 100: dAcc = 9500*90/8e6 = 0.106875; (100*100-500*100) wraps to 4294927296 -> /dAcc > max -> 8192000 (golden :583-601)");
        gr = MotorRateFromGolden(90, 10000, 500, 10000, 100);
        CHECK(gr.accPersent == 15 && gr.accMin == 30015 && gr.accMax == 122880000 && std::fabs(gr.rate - 950000.0 / 0.106875) < 1e-6,
              "iSpeed 10000: (1000000-50000)/0.106875 = 8888888.9 (inside 15*2001 .. 15*8192000; iAccPersent = 1000000/65535 = 15)");
        gr = MotorRateFromGolden(0, 10000, 500, 100, 100);
        CHECK(gr.skip && gr.dAcc == 0.0, "SetRate(0): dAcc == 0 -> golden returns before PAR_AxAcc/PAR_AxDec (:584-585)");
        gr = MotorRateFromGolden(1, 100, 500, 0, 0);
        CHECK(std::fabs(gr.dAcc - 4294966896.0 / 8000000.0) < 1e-9 && gr.rate == 2001.0,
              "JogHigh 100 < Init 500: (100-500) wraps to 4294966896 (unsigned int, golden) -> dAcc 536.870862; Rate 0 -> clamped up to 2001");
        CHECK(MotorLightScaleLine(900, 895) == "ArmPosition, 900, LightScalePos, 895 ,[ 5 ]" &&
              MotorLightScaleLine(-3, -10) == "ArmPosition, -3, LightScalePos, -10 ,[ 7 ]",
              "Memo1 line = golden \"ArmPosition, %d, LightScalePos, %d ,[ %d ]\" byte for byte (:1892)");
        std::vector<std::string> two; two.push_back("a"); two.push_back("b,c");
        CHECK(MotorStringsFileText(two) == "a\r\nb,c\r\n" && MotorStringsFileText(std::vector<std::string>()).empty(),
              "SaveToFile text = every line + CR LF, the last too; an empty memo = an empty file (TStrings::GetTextStr)");

        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 1000; g.softN = 0;
        g.jogHigh = 10000; g.jogLow = 100; g.initSpeed = 500; g.acc = 1000; g.dec = 2000; g.homeFlag = 1;
        g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100; g.range = 100; g.rate = 100;
        g.motorClass = 0; g.sensorType = true; g.in1Logic = false;
        FakeBackend be;
        be.table["MInArmX"] = Axis1203(0, 0, 0, 2);
        be.table["A"] = Axis1203(20, 20, 0, 3);
        be.table["B"] = Axis1203(21, 21, 0, 4);
        be.table["M"] = AxisOther(2, "MN200", true);
        be.aliasOf[0] = "MInArmX"; be.aliasOf[20] = "A"; be.aliasOf[21] = "B"; be.aliasOf[2] = "M";
        be.golden[0] = g; be.golden[20] = g; be.golden[21] = g; be.golden[2] = g;
        be.opened.assign(5, true);
        for (int ax = 0; ax < 5; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }

        // ---- Motor Power (golden btnMotorPowerClick :1621-1645) ----
        MotorAccessReq mp = Req("uMotorTest", "motorPowerToggle", 0); mp.button = "btnMotorPower";
        MotorAccessOutcome o = MotorAccessDispatch(mp, 51, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && be.powerOnBegins == 1 && be.powerOnSteps == 1 && be.serverOns == 0 &&
              Has(o.ackJson, "\"motorPowerOn\":true") && Has(o.ackJson, "\"caption\":\"Motor Power On\"") && Has(o.ackJson, "\"pending\":true") &&
              MotorAccessJobs().powerPending,
              "relay off -> DoMotorPowerOn begins (relay On + the three brake holds); golden's 1 s runs across ticks (pending); caption On");
        MotorAccessTick(be, 500);
        CHECK(be.powerOnSteps == 2 && be.serverOns == 1 && !MotorAccessJobs().powerPending && be.order.size() == 4 &&
              be.order[0] == "begin" && be.order[1] == "step" && be.order[2] == "step" && be.order[3] == "serverOn",
              "the 1 s over (Step true) -> only THEN SW[SwServerON].On() (golden :1635 follows DoMotorPowerOn)");
        MotorAccessReq lq = ReqStart("uMotorTest", "loopMove", "B", true); lq.num["pos1"] = 100; lq.num["pos2"] = 200;
        CHECK(MotorAccessDispatch(lq, 1, be).ok && MotorAccessJobs().loopActive, "setup: a loop on B");
        std::size_t x0 = be.executed.size();
        o = MotorAccessDispatch(mp, 52, be);
        CHECK(!o.ok && Has(o.ackJson, "needConfirm:true") && Has(o.ackJson, "Sure to turn off motor power? (確定要關掉馬達電源？)") &&
              be.servoOffs.empty() && be.executed.size() == x0,
              "relay ON and no confirmOff -> refused with golden's question and needConfirm:true; nothing switched");
        CHECK(!MotorAccessJobs().loopActive, "... but golden's AllBtnUp (:1624) already ran before the dialog: the loop is up");
        be.homeFlags.clear();
        mp.flag["confirmOff"] = true;
        x0 = be.executed.size();
        o = MotorAccessDispatch(mp, 53, be);
        bool offOk = o.ok && be.servoOffs.size() == 1 && be.servoOffs[0] == "TfMotorTest::btnMotorPowerClick" && be.executed.size() == x0 + 5;
        for (std::size_t i = x0; offOk && i < be.executed.size(); ++i) offOk = be.executed[i].kind == kCmdAxStop;
        CHECK(offOk && be.order.back() == "servoOff:" + std::to_string(x0 + 5) && Has(o.ackJson, "\"caption\":\"Motor Power Off\"") &&
              Has(o.ackJson, "\"motorPowerOn\":false") && AckShapeOk(o.ackJson, 7),
              "confirmOff -> Stop1203 on all 5 opened axes, THEN fHome->GaliMotorServoOff(\"TfMotorTest::btnMotorPowerClick\"); caption Off");
        CHECK(be.homeFlags.empty(), "power off keeps HomeFlag (golden GaliMotorServoOff does not clear it)");
        be.relayOut = false; be.relayCard = 1;                                  // the IO page switched the relay on: OutValue never heard of it
        mp.flag.erase("confirmOff");
        o = MotorAccessDispatch(mp, 54, be);
        CHECK(!o.ok && Has(o.ackJson, "needConfirm:true") && be.relayOut, "lead default: OutValue synced from the card read-back first -> the relay is ON -> golden asks");
        be.relayOut = false; be.relayCard = -1; be.powerOnStepsToDone = 100;
        CHECK(MotorAccessDispatch(mp, 55, be).ok && MotorAccessJobs().powerPending, "setup: a power-on job pending");
        o = MotorAccessDispatch(mp, 56, be);
        CHECK(!o.ok && Has(o.ackJson, "還在進行") && be.powerOnBegins == 2, "a second press inside golden's second -> refused (golden's UI is frozen then)");
        MotorAccessResetJobs(); be.powerOnStepsToDone = 2; be.powerOnSteps = 0;

        // ---- FormShow / FormClose (golden :985-1077 / :1347-1362) ----
        be.relayOut = false; be.relayCard = -1; be.servoOffs.clear();
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be);
        MotorAccessReq fs = Req("uMotorTest", "formShow", 0); fs.button = "FormShow";
        o = MotorAccessDispatch(fs, 61, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && be.servoOffs.size() == 1 && be.servoOffs[0] == "TfMotorTest::FormShow" &&
              MotorAccessJobs().selectedMotor.empty() && MotorAccessJobs().pageShown && Has(o.ackJson, "\"selectedMotor\":null") &&
              Has(o.ackJson, "\"caption\":\"Motor Power Off\""),
              "FormShow, relay off -> golden GaliMotorServoOff(\"TfMotorTest::FormShow\"), ActiveIndex=-1 (selectedMotor null), caption Off");
        be.relayOut = true;
        const int b0 = be.powerOnBegins;
        o = MotorAccessDispatch(fs, 62, be);
        CHECK(o.ok && be.powerOnBegins == b0 + 1 && Has(o.ackJson, "\"caption\":\"Motor Power On\""), "FormShow, relay on -> DoMotorPowerOn + (after its 1 s) SW[SwServerON].On(); caption On");
        MotorAccessTick(be, 500);
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be);
        CHECK(MotorAccessDispatch(lq, 1, be).ok && MotorAccessJobs().loopActive, "setup: a loop on B again");
        x0 = be.executed.size();
        MotorAccessReq fc = Req("uMotorTest", "formClose", 0); fc.button = "FormClose";
        o = MotorAccessDispatch(fc, 63, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && !MotorAccessJobs().loopActive && be.executed.size() == x0 && be.formCloses == 1 && !MotorAccessJobs().pageShown,
              "FormClose -> fShow=false, PauseUT150Polling=false, the loop ends WITHOUT a stop (golden Timer1 just stops stepping it)");
        CHECK(!MotorAccessDispatch(Req("uteach", "formShow", 0), 1, be).ok, "formShow from uteach -> refused (a uMotorTest action)");

        // ---- Test Range / Test Rate on a 1203 axis: the FULL golden InitMotor (R4, myEthercatmotor.cpp:166-396) ----
        MotorAccessResetJobs();
        be.executed.clear(); be.homeFlags.clear();
        MotorAccessReq tr = Req("uMotorTest", "setRangeAndInit", "A"); tr.num["value"] = 5000;
        o = MotorAccessDispatch(tr, 71, be);
        const Pci1203CmdKind want[11] = { kCmdAxResetError, kCmdAxSetInitCfg, kCmdAxSetInitCfg, kCmdAxSetInitCfg, kCmdAxSetSpeed, kCmdAxSetSpeed,
                                          kCmdAxSetSpeed, kCmdAxResetError, kCmdAxSvOn, kCmdAxSetCmdPos, kCmdAxSetActPos };
        bool seqOk = o.ok && be.executed.size() == 11;
        for (int i = 0; seqOk && i < 11; ++i) seqOk = be.executed[i].kind == want[i] && be.executed[i].axis == 3 && be.executed[i].wireId == 71;
        CHECK(seqOk, "Test Range: ResetError, the config plan (kCmdAxSetInitCfg x3 here), MaxVel/MaxAcc/MaxDec, ResetError, SvOn, SetCmdPos, SetActPos -- golden order");
        CHECK(seqOk && be.executed[1].initCfg == kInitCfgPPU && be.executed[1].value == 1.0 && be.executed[2].initCfg == kInitCfgOrgLogic &&
              be.executed[2].value == 0.0 && be.executed[3].initCfg == kInitCfgJerk && be.planClass == 0 && be.planSensor,
              "the plan comes from EastSun's Pci1203GoldenInitCfgPlan for this motor's MotorType / bSensorType / bIn1Logic (OrgLogic = bSensorType ? 0 : 1)");
        CHECK(seqOk && be.executed[4].speed == kSpeedMaxVel && be.executed[4].value == 10000.0 && be.executed[5].speed == kSpeedMaxAcc &&
              be.executed[5].value == 1000.0 && be.executed[6].speed == kSpeedMaxDec && be.executed[6].value == 2000.0 &&
              be.executed[8].value == 1.0 && be.executed[9].value == 0.0 && be.executed[10].value == 0.0,
              "CFG_AxMaxVel = PJogHighSpeed 10000, MaxAcc = dAcc 1000, MaxDec = dDec 2000 (:363-382); SvOn 1; SetCommand(0), SetPosition(0)");
        CHECK(be.rangeMem.count(20) && be.rangeMem[20] == 5000 && be.homeFlags.count(20) && be.homeFlags[20] == 0 &&
              Has(o.ackJson, "\"result\":\"initialized\"") && Has(o.ackJson, "\"ran\":true") && Has(o.ackJson, "\"range\":1000") && AckShapeOk(o.ackJson, 7),
              "SetRange(5000) in memory (the object keeps min(a,1000)), then HomeFlag=0 (:1381); the ack lists every step");
        be.executed.clear(); be.state[3] = 3; be.motionIO[3] = 0x00000002ul;   // ERROR_STOP + ALM
        o = MotorAccessDispatch(tr, 72, be);
        CHECK(o.ok && be.executed.size() == 13 && be.executed[0].kind == kCmdAxResetError && be.executed[1].kind == kCmdAxResetError &&
              be.executed[9].kind == kCmdAxResetError && be.executed[10].kind == kCmdAxSvOn && be.sleeps.size() == 1 && be.sleeps[0] == 100,
              "not READY -> golden's second ResetError (:203); alarm lamp (bAlarm) -> ResetError + MySleep(100) before SvOn (MotOutputOn :1398-1406)");
        be.executed.clear(); be.rejectKinds.insert((int)kCmdAxResetError); be.homeFlags.clear();
        o = MotorAccessDispatch(tr, 73, be);
        bool onlyResets = be.executed.size() == 6;
        for (std::size_t i = 0; onlyResets && i < be.executed.size(); ++i) onlyResets = be.executed[i].kind == kCmdAxResetError;
        CHECK(o.ok && onlyResets && Has(o.ackJson, "\"ran\":false") && Has(o.ackJson, "goto ResetMotorError") && be.homeFlags[20] == 0 &&
              Has(o.ackJson, "\"result\":\"initMotorNotRun\""),
              "ResetError keeps failing while not READY -> golden's endless goto bounded to 3 rounds (6 ResetErrors), then InitMotor stops; HomeFlag=0 still");
        be.rejectKinds.clear(); be.state[3] = 1; be.motionIO.erase(3);
        be.executed.clear(); be.emgOff = true;
        o = MotorAccessDispatch(tr, 74, be);
        CHECK(o.ok && be.executed.empty() && Has(o.ackJson, "Please Unlock EMG And Restart The Software!!") && be.homeFlags[20] == 0,
              "an EMG sensor IsOff -> golden ShowMyMessage + return false (:180-185): nothing written, HomeFlag=0 like golden's button");
        be.emgOff = false;
        be.table["A"].tableEnable = false;
        o = MotorAccessDispatch(tr, 75, be);
        CHECK(o.ok && be.executed.empty() && Has(o.ackJson, "if(!Enable) return true;"), "Mot_Table Enable=0 -> golden InitMotor returns at once (:172), nothing written");
        be.table["A"].tableEnable = true;
        // Test Rate: SetRate(90) with iSpeed 100 (GoldenReadSpeed), JogHigh 10000, Init 500, Range 100 (hand values above)
        be.executed.clear(); be.readSpeedV = 100; be.golden[20].range = 100;
        MotorAccessReq ta = Req("uMotorTest", "setRateAndInit", "A"); ta.num["value"] = 90;
        o = MotorAccessDispatch(ta, 76, be);
        CHECK(o.ok && be.accMem.count(20) && std::fabs(be.accMem[20] - 0.106875) < 1e-15 && be.executed.size() == 13 &&
              be.executed[0].kind == kCmdAxSetSpeed && be.executed[0].speed == kSpeedAcc && be.executed[0].value == 8192000.0 &&
              be.executed[1].speed == kSpeedDec && be.executed[1].value == 8192000.0,
              "Test Rate: dAcc = 0.106875 in memory, PAR_AxAcc = PAR_AxDec = the clamped Rate 8192000 (golden :604-615), then InitMotor");
        CHECK(be.executed[7].speed == kSpeedMaxAcc && be.executed[7].value == be.accMem[20],
              "... and InitMotor's CFG_AxMaxAcc = the NEW dAcc 0.106875 (golden: that is what slows the axis -- kept, Test Rate stays hidden like golden)");
        be.executed.clear(); ta.num["value"] = 0;
        o = MotorAccessDispatch(ta, 77, be);
        CHECK(o.ok && be.executed.size() == 11 && be.executed[0].kind == kCmdAxResetError && be.accMem[20] == 0.0,
              "SetRate(0): dAcc = 0, golden returns before PAR_AxAcc/Dec -- InitMotor still runs (MaxAcc 0)");

        // ---- R2: a PCI1203 row is a 1203 axis whatever INDEX_MOTION_CARD says (M14 MTestZ1) ----
        MotorGolden gIx = g; gIx.indexMotor = false; gIx.galilIndex = true;    // a stale galilIndex on a 1203 row
        be.table["MTestZ1"] = Axis1203(14, 14, 0, 1); be.golden[14] = gIx; be.aliasOf[14] = "MTestZ1";
        MotorAccessReq jz = Req("uMotorTest", "jogP", "MTestZ1"); jz.button = "sbMotorTest_JogP";
        o = MotorAccessDispatch(jz, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxJogStart && be.executed.back().axis == 1,
              "R2: M14 on a PCI1203 row jogs through the 1203 (Is1203 is tested before the Galil refusal; was refused)");
        { MotorAccessReq rel = Req("uMotorTest", "stop", "MTestZ1"); rel.button = "sbMotorTest_JogP"; MotorAccessDispatch(rel, 1, be); }
        MotorGolden gGal = g; gGal.indexMotor = true; gGal.galilIndex = true;
        be.table["MTestZ2"] = AxisOther(15, "SMC", true); be.golden[15] = gGal;
        MotorAccessReq jg = Req("uMotorTest", "jogP", "MTestZ2"); jg.button = "sbMotorTest_JogP";
        CHECK(!MotorAccessDispatch(jg, 1, be).ok, "a NON-1203 Index row with INDEX_MOTION_CARD==0 is still refused (golden Galil branch, not wired)");

        // ---- engine hooks (VerifyMotorAction, R8) ----
        MotorAccessResetJobs();
        CHECK(MotorAccessMovingOf(be, 20) == 0, "hook: a 1203 row in READY -> 0 (not moving)");
        be.state[3] = 5;
        CHECK(MotorAccessMovingOf(be, 20) == 1, "hook: not READY (PTP) -> 1 (golden MotionDone false)");
        be.state[3] = 3;
        CHECK(MotorAccessMovingOf(be, 20) == 0, "AI(W906-MT-AXISLOCK) 20260929: ERROR_STOP is STOPPED -> 0 (EastSun: one alarmed axis locked every axis; golden MotionDone is state==READY)");
        be.state[3] = 0;
        CHECK(MotorAccessMovingOf(be, 20) == 0, "AXISLOCK: DISABLE -> 0 (not moving)");
        {   // the per-axis lock the Motor Test page uses
            std::string why;
            CHECK(MotorAccessAxisLock(be, 20, why) == 0 && why.empty(), "AXISLOCK: DISABLE -> free");
            be.state[3] = 3;
            CHECK(MotorAccessAxisLock(be, 20, why) == 0, "AXISLOCK: ERROR_STOP -> free (the alarmed axis itself can be reset / jogged away)");
            be.state[3] = 5;
            CHECK(MotorAccessAxisLock(be, 20, why) == 1 && why == "moving (PTP_MOT)", "AXISLOCK: PTP -> locked, why names the state");
            CHECK(be.state[4] == 1 && MotorAccessAxisLock(be, 21, why) != -1 && why.find("PTP_MOT") == std::string::npos,
                  "AXISLOCK: another axis (B, READY) is not locked by axis 3's motion");
            CHECK(MotorAccessAxisLock(be, 2, why) == -1, "AXISLOCK: a non-1203 row -> -1 (the page keeps golden's page lock for it)");
            be.state[3] = 1;
            CHECK(MotorAccessAxisLock(be, 20, why) == 0 && why.empty(), "AXISLOCK: READY -> free");
        }
        {   //AI(W906-MT-ALMRST) 20260929: Alarm Reset of the selected axis (EastSun)
            const std::size_t xr = be.executed.size();
            be.state[3] = 3;                                                    // A in ERROR_STOP
            MotorAccessOutcome ro = MotorAccessDispatch(Req("uMotorTest", "resetAlarm", "A"), 1, be);
            CHECK(ro.ok && be.executed.size() == xr + 1 && be.executed.back().kind == kCmdAxResetError && be.executed.back().axis == 3 &&
                  Has(ro.ackJson, "alarmReset") && Has(ro.ackJson, "state=3"),
                  "ALMRST: resetAlarm A -> exactly one Acm_AxResetError on A's axis (3), nothing else; the ack shows the state before");
            be.state[3] = 1;
            CHECK(!MotorAccessDispatch(Req("uMotorTest", "resetAlarm", "M"), 1, be).ok && be.executed.size() == xr + 1, "ALMRST: a non-1203 row -> refused, nothing sent");
            CHECK(!MotorAccessDispatch(Req("uMotorTest", "resetAlarm", 0), 1, be).ok && be.executed.size() == xr + 1, "ALMRST: no motor -> refused, nothing sent");
        }
        be.state[3] = 1;
        CHECK(MotorAccessMovingOf(be, 2) == -1 && MotorAccessMovingOf(be, 99) == -1, "hook: a non-1203 row / no row -> -1 (golden MotionDone decides)");
        CHECK(MotorAccessMovingOf(be, 14) == 0, "hook: M14 (R2) is claimed like every PCI1203 row");
        MotorAccessDispatch(ReqStart("uMotorTest", "home", "B", true), 1, be);
        CHECK(MotorAccessHomingActive(), "hook: a btnHome HOME job -> homing (golden btnHome->Down)");
        lq.flag["confirmNotHomed"] = true;                                      // B's HomeFlag is 0 now (the home just cleared it)
        MotorAccessDispatch(lq, 1, be);   // cancels the home (golden btnHome->Down=false) and starts a loop
        CHECK(!MotorAccessHomingActive() && MotorAccessJobs().loopActive, "setup: the loop");
        MotorAccessAllBtnUp(be, "VerifyMotorAction: 2 s without motion");
        CHECK(!MotorAccessJobs().loopActive, "hook AllBtnUp -> the loop ends (golden AllBtnUp)");
        x0 = be.executed.size();
        MotorAccessStop1203All(be, "GaliMotorServoOff - x");
        CHECK(be.executed.size() == x0 + 5 && be.executed.back().kind == kCmdAxStop, "hook Stop1203All -> Stop1203 on every opened axis");

        // ---- All-mode loop (golden DoLoopMove :468-501 / :577-617, ck00Click) ----
        MotorAccessResetJobs();
        be.clockMs = 0.0; be.moveRet = 0;
        for (int ax = 0; ax < 5; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }
        be.golden[20].homeFlag = 1; be.golden[21].homeFlag = 0;                 // B is NOT homed: golden asks only the selected (A)
        MotorAccessReq la = ReqStart("uMotorTest", "loopMove", 0, true);
        la.num["mode"] = 1; la.num["waitTime"] = 0;
        la.motors.push_back("A"); la.motors.push_back("B"); la.motors.push_back("M");
        la.num["pos1.A"] = 100; la.num["pos2.A"] = 200; la.num["pos1.B"] = 300; la.num["pos2.B"] = 400; la.num["pos1.M"] = 10; la.num["pos2.M"] = 20;
        CHECK(!MotorAccessDispatch(la, 1, be).ok, "All mode, nothing selected -> refused (golden ActiveIndex==-1)");
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be);
        MotorAccessReq s40 = Req("uMotorTest", "setSpeed", "A"); s40.num["pct"] = 40; s40.flag["jog"] = false;
        MotorAccessDispatch(s40, 1, be);
        x0 = be.executed.size();
        const int ip0 = be.initMotParamCalls;
        o = MotorAccessDispatch(la, 81, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && MotorAccessJobs().loopActive && MotorAccessJobs().loopAll && be.initMotParamCalls == ip0 + 1 &&
              MotorAccessJobs().loopMotors.size() == 3 && Has(o.ackJson, "\"all\":true"),
              "All mode starts (only the selected A was asked about HomeFlag; B unhomed is fine, golden :1316); InitMOTParameter for all (:1314)");
        CHECK(be.executed.size() == x0 + 8 && be.executed[x0].axis == 3 && be.executed[x0 + 1].value == 4000.0 &&
              be.executed[x0 + 4].axis == 4 && be.executed[x0 + 5].value == 100.0,
              "lead default: each 1203 motor starts at its last golden SetSpeed pct -- A 40% (velHigh 4000), B never set -> 1% (100)");
        x0 = be.executed.size();
        MotorAccessPollTick(be);
        CHECK(be.executed.size() == x0 + 2 && be.executed[x0].kind == kCmdAxMoveAbs && be.executed[x0].value == 100.0 &&
              be.executed[x0 + 1].value == 300.0 && be.moves.back().mi == 2 && be.moves.back().target == 10,
              "tick: every checked motor gets MotorMove(its edPos1): A 100, B 300 (1203 MoveAbs), M 10 (golden object)");
        be.cmdPos[3] = 100.0; be.notReady.insert(4);
        MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().loopTask == 1, "A arrived, B still moving -> the leg waits for ALL of them");
        be.notReady.erase(4); be.cmdPos[4] = 300.0; be.moveRet = 1;
        MotorAccessPollTick(be);
        CHECK(MotorAccessJobs().loopTask == 50 && MotorAccessJobs().loopCount == 0, "all arrived -> leg 2 (count still 0: golden counts on pos2)");
        be.doorOpen.insert(21);
        x0 = be.executed.size();
        MotorAccessPollTick(be);
        bool b400 = false;
        for (std::size_t i = x0; i < be.executed.size(); ++i) if (be.executed[i].kind == kCmdAxMoveAbs && be.executed[i].value == 400.0) b400 = true;
        CHECK(!b400 && be.executed.size() == x0 + 1 && be.executed[x0].value == 200.0, "B's door open -> MotorMove -1: B does not move; A goes to 200");
        be.cmdPos[3] = 200.0; be.clockMs = 5000.0;
        MotorAccessPollTick(be);
        MotorAccessJobState js = MotorAccessJobs();
        CHECK(js.loopCount == 1 && js.loopTask == 1 && js.hasAvgTime && js.avgTime == 5000.0 && !js.hasJogNTime,
              "A arrived, B's -1 counts as arrived (golden int->bool) -> dwLoopCount 1; Average += ms since the last single-axis latch (none: 0) = 5000; lblJogNTime untouched");
        be.doorOpen.clear();
        x0 = be.executed.size();
        const std::size_t sp0 = be.speedCalls.size();
        MotorAccessReq s70 = Req("uMotorTest", "setSpeed", "A"); s70.num["pct"] = 70; s70.flag["jog"] = true;
        o = MotorAccessDispatch(s70, 1, be);
        CHECK(o.ok && be.executed.size() == x0 + 16 && be.executed[x0 + 8].axis == 4 && Has(o.ackJson, "LoopMove(All)") &&
              be.speedCalls.back().mi == 2 && be.speedCalls.back().pct == 70 && be.speedCalls.back().jog && be.speedCalls.size() > sp0,
              "the scroll bar during the All loop -> SetSpeed(70, true) on every checked motor too (golden :803-809): B 8 values, M the golden object");
        x0 = be.executed.size();
        MotorAccessReq laOff = la; laOff.flag["start"] = false;
        o = MotorAccessDispatch(laOff, 1, be);
        CHECK(o.ok && !MotorAccessJobs().loopActive && be.executed.size() == x0 + 1 && be.executed[x0].kind == kCmdAxStop && be.executed[x0].axis == 3,
              "LoopMove up -> golden PCIL132_StopMotor on the SELECTED motor only (:1343)");
        be.golden[20].homeFlag = 0;
        o = MotorAccessDispatch(la, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "not home yet"), "the selected motor not homed, no confirmation -> golden's question");
        MotorAccessReq lb = la; lb.num.erase("pos2.B"); lb.flag["confirmNotHomed"] = true;
        CHECK(!MotorAccessDispatch(lb, 1, be).ok, "a checked motor without its pos2.<Alias> -> refused");
        be.golden[20].homeFlag = 1;
        be.clockMs = -1.0;

        // ---- Light Scale (golden uMotorTest.cpp:1749-2133, R7 completely golden) ----
        MotorAccessResetJobs();
        be.executed.clear(); be.moveRet = 0; be.driveKind = 1; be.homeFlags.clear();
        for (int ax = 0; ax < 5; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }
        be.golden[0].homeFlag = 0;
        be.lsEncoder = -895; be.lsEncFromMonitor = true;
        MotorAccessReq ls = Req("uMotorTest", "lightScale", 0); ls.button = "BitBtn2"; ls.kind = "motion";
        ls.num["axisItem"] = 1; ls.num["moveType"] = 0; ls.num["pitch"] = 300; ls.num["delayMs"] = 0;
        o = MotorAccessDispatch(ls, 91, be);
        MotorLightScaleState st = MotorAccessLightScale(500);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && st.active && st.task == 0 && !st.editsEnabled && st.useAxis == 0 && Has(o.ackJson, "\"useMotor\":\"MInArmX\""),
              "BitBtn2: rgAxis 1 -> iUseAxis 0 (MInArmX); LightScale(true): Task 0, edits disabled; Timer2 ON");
        MotorAccessPollTick(be);
        st = MotorAccessLightScale(500);
        CHECK(st.task == 1 && st.homePending && be.executed.back().kind == kCmdAxHome && be.executed.back().axis == 2 &&
              be.homeFlags.count(0) && be.homeFlags[0] == 0 && !MotorAccessHomingActive() && MotorAccessJobs().homesActive == 1,
              "case 0: the single home of the arm (bSingleHome=true, HomeFlag=0) -- not btnHome (the Homeing lock does not see it)");
        be.state[2] = 4; MotorAccessTick(be, 500);
        CHECK(MotorAccessLightScale(500).task == 1, "case 1 waits while the arm homes");
        be.state[2] = 1; MotorAccessTick(be, 500);
        st = MotorAccessLightScale(500);
        CHECK(!st.homePending && st.task == 150 && st.needMovePos == 900 && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 900.0,
              "home done (HomeFlag=1) -> bSingleHome drops -> case 10: 去(單趟) starts at PSoftLimitP-100 = 900 (:1853) -> MotorMove(900)");
        be.cmdPos[2] = 900.0; MotorAccessPollTick(be);
        st = MotorAccessLightScale(500);
        CHECK(st.memoCount == 1 && st.memoTail.size() == 1 && st.memoTail[0] == "ArmPosition, 900, LightScalePos, 895 ,[ 5 ]" &&
              be.executed.back().value == 600.0 && st.task == 150,
              "arrived -> Delay 0 -> scale x(-1) = 895 -> Memo1 'ArmPosition, 900, LightScalePos, 895 ,[ 5 ]' -> next target 900-300 = 600");
        be.cmdPos[2] = 600.0; MotorAccessPollTick(be);
        be.cmdPos[2] = 300.0; MotorAccessPollTick(be);
        st = MotorAccessLightScale(500);
        CHECK(st.memoCount == 3 && st.memoTail[1] == "ArmPosition, 600, LightScalePos, 895 ,[ -295 ]" &&
              st.memoTail[2] == "ArmPosition, 300, LightScalePos, 895 ,[ -595 ]" && !st.active && st.editsEnabled && st.task == 180,
              "900, 600, 300; next would be 0 <= PSoftLimitN+100 = 100 -> done: edits enabled, Timer2 off (:1906-1912)");
        CHECK(MotorAccessLightScale(2).memoTail.size() == 2 && MotorAccessLightScale(2).memoTail[0] == st.memoTail[1], "runtime tail = the last N lines");
        // save (BitBtn3 :2102-2119, SaveAsCSV :1749-1787) into a temp root
        const char* tmpEnv = std::getenv("TEMP");
        const std::string root = std::string(tmpEnv ? tmpEnv : ".") + "\\ht9045_mte3c_ls_" + std::to_string((long long)_getpid());
        be.lsRoot = root;
        MotorAccessReq sv = Req("uMotorTest", "lightScaleSave", 0); sv.button = "BitBtn3";
        sv.num["axisItem"] = 1; sv.num["moveType"] = -1;
        o = MotorAccessDispatch(sv, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "往返動作選擇異常!"), "BitBtn3 with rgMoveType -1 -> golden '往返動作選擇異常!'");
        sv.num["moveType"] = 0; sv.num["axisItem"] = 0;
        o = MotorAccessDispatch(sv, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "量測軸選擇異常!"), "BitBtn3 with rgAxis No Use -> golden '量測軸選擇異常!'");
        sv.num["axisItem"] = 1;
        o = MotorAccessDispatch(sv, 1, be);
        const std::string f1 = root + "\\Motor1Positive.csv";
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "Save successfully! Path:") && ReadAll(f1) ==
              "ArmPosition, 900, LightScalePos, 895 ,[ 5 ]\r\nArmPosition, 600, LightScalePos, 895 ,[ -295 ]\r\nArmPosition, 300, LightScalePos, 895 ,[ -595 ]\r\n" &&
              MotorAccessLightScale(500).memoCount == 0,
              "去 + InArm X -> <root>\\Motor1Positive.csv = Memo1's lines, each + CR LF (golden SaveToFile); Memo1 cleared (:1786)");
        sv.num["axisItem"] = 3; sv.num["moveType"] = 1;
        o = MotorAccessDispatch(sv, 1, be);
        const std::string f6 = root + "\\Motor6Positive.csv";
        CHECK(o.ok && ReadAll(f6).empty() && Has(o.ackJson, "Motor6Positive.csv"), "返 + OutArm X -> Motor6Positive.csv (an empty Memo1 -> an empty file)");
        // Light Scale Data (btnSaveLogLightScaleData :2051-2094)
        be.lsData[1].push_back("Old, 1, New, 2, [ -1 ]"); be.lsCount[1] = 1; be.lsCount[8] = 7;
        MotorAccessReq ds = Req("uMotorTest", "lightScaleDataSave", 0); ds.button = "btnSaveLogLightScaleData";
        o = MotorAccessDispatch(ds, 1, be);
        const std::string dd = root + "\\LightScaleData_202609251234";
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && ReadAll(dd + "\\InArmX_Motor1Positive.csv") == "Old, 1, New, 2, [ -1 ]\r\n" &&
              ReadAll(dd + "\\OutArmY_Motor8Positive.csv").empty() && be.lsData[1].empty() && be.lsCountsResets == 1 && be.lsCount[8] == 0 &&
              Has(o.ackJson, "Save successfully!\\nPath:"),
              "Save -> <root>\\LightScaleData_yyyymmddhhmm\\ InArmX_Motor1Positive.csv .. OutArmY_Motor8Positive.csv, mmo cleared, the 8 counters reset");
        static const char* const kLsNames[8] = { "InArmX_Motor1Positive", "InArmX_Motor2Positive", "InArmY_Motor3Positive", "InArmY_Motor4Positive",
                                                 "OutArmX_Motor5Positive", "OutArmX_Motor6Positive", "OutArmY_Motor7Positive", "OutArmY_Motor8Positive" };
        int nFiles = 0;
        for (int k = 0; k < 8; ++k) { const std::string p = dd + "\\" + kLsNames[k] + ".csv"; if (FileExists(p)) ++nFiles; std::remove(p.c_str()); }
        CHECK(nFiles == 8, "all eight files written");
        std::remove(f1.c_str()); std::remove(f6.c_str()); _rmdir(dd.c_str()); _rmdir(root.c_str());

        // golden's un-stoppable behaviours, one by one
        MotorAccessResetJobs();
        be.executed.clear();
        for (int ax = 0; ax < 5; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }
        be.golden[0].homeFlag = 1;
        ls.num["moveType"] = 1;                                                 // 返(單趟): from PSoftLimitN+100 upwards
        MotorAccessDispatch(ls, 1, be);
        MotorAccessPollTick(be);                                                // case 0 -> the home
        be.state[2] = 4; MotorAccessTick(be, 500);
        be.state[2] = 1; MotorAccessTick(be, 500);
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 100.0, "返: the first target is PSoftLimitN+100 = 100 (:1860)");
        be.cmdPos[2] = 100.0; MotorAccessPollTick(be);
        CHECK(be.executed.back().value == 400.0, "... then +Pitch: 400");
        x0 = be.executed.size();
        MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, be);
        CHECK(MotorAccessLightScale(500).active && be.executed.size() > x0 && be.executed.back().kind == kCmdAxStop,
              "STOP stops the axes but NOT the scan (golden btnStopClick never touches Timer2)");
        MotorAccessPollTick(be);                                                // the arm stopped at 100: READY after a command
        st = MotorAccessLightScale(500);
        CHECK(st.memoCount == 2 && st.memoTail[1] == "ArmPosition, 400, LightScalePos, 895 ,[ -495 ]" && be.executed.back().value == 700.0,
              "golden MotorMove after a stop: fCMD + MotionDone = 'arrived' -> the point 400 is recorded where the arm never went, and the scan goes on to 700");
        x0 = be.executed.size();
        o = MotorAccessDispatch(ls, 1, be);
        st = MotorAccessLightScale(500);
        CHECK(o.ok && !st.active && st.task == 0 && be.executed.size() == x0, "BitBtn2 again: Timer2 OFF and Task reset to 0 -- the axis in flight is NOT stopped (:2096-2100)");
        MotorAccessPollTick(be);
        CHECK(MotorAccessLightScale(500).task == 0, "timer off -> nothing steps");
        MotorAccessDispatch(ls, 1, be);
        MotorAccessPollTick(be);
        CHECK(MotorAccessLightScale(500).task == 1 && MotorAccessLightScale(500).homePending, "BitBtn2 a third time: Timer2 ON from Task 0 -> it homes again first");
        MotorAccessDispatch(Req("uMotorTest", "stop", 0), 1, be);
        CHECK(!MotorAccessLightScale(500).homePending, "STOP -> bSingleHome=false (golden :1651) -> case 1 is released (the scan then starts un-homed: golden)");
        MotorAccessDispatch(fc, 1, be);
        CHECK(MotorAccessLightScale(500).active, "FormClose does not stop it (golden Timer2Timer has no fShow test)");
        be.doorOpen.insert(0);
        const std::size_t m0 = MotorAccessLightScale(500).memoCount;
        x0 = be.executed.size();
        MotorAccessPollTick(be);
        bool anyMove = false;
        for (std::size_t i = x0; i < be.executed.size(); ++i) if (be.executed[i].kind == kCmdAxMoveAbs) anyMove = true;
        CHECK(!anyMove && MotorAccessLightScale(500).memoCount > m0, "door open -> MotorMove -1 is 'arrived': points are recorded, nothing moves (golden)");
        be.doorOpen.clear();
        MotorAccessReq lsNo = ls; lsNo.num["axisItem"] = 0;
        MotorAccessDispatch(lsNo, 1, be);                                       // BitBtn2 with rgAxis No Use: the guard returns, the timer toggles
        st = MotorAccessLightScale(500);
        CHECK(st.axisItem == 0 && st.useAxis == -1, "rgAxis No Use -> iUseAxis -1 (rgAxisClick)");
        MotorAccessResetJobs();
        MotorAccessReq lsNever = ls; lsNever.num["axisItem"] = -1;
        o = MotorAccessDispatch(lsNever, 1, be);
        CHECK(o.ok && MotorAccessLightScale(500).useAxis == 0 && MotorAccessLightScale(500).active && Has(o.ackJson, "MInArmX"),
              "rgAxis never clicked (-1) -> golden iUseAxis is still 0 -> the scan runs on MOT[0] (golden)");
        MotorAccessReq lsBad = ls; lsBad.num["axisItem"] = 5;
        CHECK(!MotorAccessDispatch(lsBad, 1, be).ok, "axisItem outside -1..4 -> refused (contract)");
        MotorAccessResetJobs();
    }

    //AI(W906-MT-FIX1) 20260926: fixes from the adversarial review of 2c1afab / 5a0f02d.
    printf("\n[14] AI(W906-MT-FIX1): Motor Power direction, post-Poll gate, jog re-sends the bar's values, failed bar write\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 1000; g.softN = 0;
        g.jogHigh = 10000; g.jogLow = 100; g.initSpeed = 500; g.acc = 1000; g.dec = 2000; g.homeFlag = 1;
        g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100; g.range = 100; g.rate = 100;
        g.motorClass = 0; g.sensorType = true; g.in1Logic = false;
        FakeBackend be;
        be.table["A"] = Axis1203(20, 20, 0, 3);
        be.table["B"] = Axis1203(21, 21, 0, 4);
        be.aliasOf[20] = "A"; be.aliasOf[21] = "B";
        be.golden[20] = g; be.golden[21] = g;
        be.opened.assign(5, true);
        for (int ax = 0; ax < 5; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }
        be.powerOnStepsToDone = 1;

        // (a) Motor Power carries the operator's direction
        be.relayOut = false; be.relayCard = -1;
        MotorAccessReq mp = Req("uMotorTest", "motorPowerToggle", 0); mp.button = "btnMotorPower";
        mp.flag["confirmOff"] = true;                                           // the operator answered "Sure to turn off motor power?"
        const int pb = be.powerOnBegins;
        MotorAccessOutcome o = MotorAccessDispatch(mp, 71, be);
        CHECK(!o.ok && Has(o.ackJson, "relay-changed") && be.powerOnBegins == pb && be.servoOffs.empty(),
              "confirmOff:true but the relay is already off -> refused relay-changed; a confirmed power-OFF never powers ON");
        mp.flag.erase("confirmOff"); mp.flag["expectRelayOn"] = true;
        o = MotorAccessDispatch(mp, 72, be);
        CHECK(!o.ok && Has(o.ackJson, "relay-changed") && be.powerOnBegins == pb, "expectRelayOn=true, relay off -> refused, nothing switched");
        mp.flag["expectRelayOn"] = false;
        o = MotorAccessDispatch(mp, 73, be);
        CHECK(o.ok && be.powerOnBegins == pb + 1, "expectRelayOn=false matches -> golden DoMotorPowerOn");
        MotorAccessTick(be, 500);
        o = MotorAccessDispatch(mp, 74, be);                                    // relay is ON now, page still thinks off
        CHECK(!o.ok && Has(o.ackJson, "relay-changed") && be.servoOffs.empty() && be.relayOut, "expectRelayOn=false, relay on -> refused, power stays on");
        MotorAccessResetJobs();

        // (b) the post-Poll step is gated like the beat
        MotorAccessReq lq = ReqStart("uMotorTest", "loopMove", "B", true); lq.num["pos1"] = 100; lq.num["pos2"] = 200;
        CHECK(MotorAccessDispatch(lq, 1, be).ok && MotorAccessJobs().loopActive, "setup: a loop on B");
        std::size_t x0 = be.executed.size();
        MotorAccessPollTick(be, false);
        bool mv = false;
        for (std::size_t i = x0; i < be.executed.size(); ++i) if (be.executed[i].kind == kCmdAxMoveAbs) mv = true;
        CHECK(!MotorAccessJobs().loopActive && !mv, "operator gone -> the post-Poll step cancels the loop BEFORE issuing a leg");
        CHECK(MotorAccessDispatch(lq, 1, be).ok && MotorAccessJobs().loopActive, "setup: a loop again");
        be.safeLock = true;
        MotorAccessTick(be, 500);
        CHECK(!MotorAccessJobs().loopActive, "beat: safe lock -> cancelled (and remembered)");
        CHECK(MotorAccessDispatch(lq, 1, be).ok && MotorAccessJobs().loopActive, "a new loop started right after that beat");
        x0 = be.executed.size();
        MotorAccessPollTick(be, true);
        mv = false;
        for (std::size_t i = x0; i < be.executed.size(); ++i) if (be.executed[i].kind == kCmdAxMoveAbs) mv = true;
        CHECK(!MotorAccessJobs().loopActive && !mv, "post-Poll step: the last beat's safe lock -> cancelled before a leg");
        be.safeLock = false;
        MotorAccessTick(be, 500);                                               // no job -> the remembered lock is cleared
        CHECK(MotorAccessDispatch(lq, 1, be).ok, "setup: loop after the lock is gone");
        MotorAccessPollTick(be, true);
        CHECK(MotorAccessJobs().loopActive, "lock gone -> the loop keeps running after a Poll");
        MotorAccessResetJobs();

        // (c) a jog re-sends the values the scroll bar wrote, not today's parameters
        CHECK(MotorAccessDispatch(Req("uMotorTest", "selectMotor", "A"), 1, be).ok, "select A");
        MotorAccessReq sq = Req("uMotorTest", "setSpeed", "A"); sq.num["pct"] = 50; sq.flag["jog"] = true;
        CHECK(MotorAccessDispatch(sq, 1, be).ok, "scroll bar 50");
        be.golden[20].jogHigh = 100000;                                         // a later cell edit / Copy From raised JogHighSpeed (memory only)
        MotorAccessReq jq = Req("uMotorTest", "jogP", "A"); jq.button = "sbMotorTest_JogP";
        MotorAccessReq jr = Req("uMotorTest", "stop", "A"); jr.button = "sbMotorTest_JogP";
        x0 = be.executed.size();
        o = MotorAccessDispatch(jq, 1, be);
        CHECK(o.ok && be.executed.size() >= x0 + 2 && be.executed[x0 + 1].speed == kSpeedJogRun && be.executed[x0 + 1].value == 5000.0,
              "JogHighSpeed changed 10000 -> 100000 after the scroll -> the jog still runs at the 5000 the bar wrote (golden), not 50000");
        MotorAccessDispatch(jr, 1, be);

        // (d) a failed scroll-bar write keeps the slower jog values
        be.golden[20].jogHigh = 10000;
        be.rejectNext = true;
        MotorAccessReq s10 = sq; s10.num["pct"] = 10;
        o = MotorAccessDispatch(s10, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "10%"), "bar to 10 fails on the card -> refused; the jog now uses the slower 10%");
        x0 = be.executed.size();
        MotorAccessDispatch(jq, 1, be);
        CHECK(be.executed.size() >= x0 + 2 && be.executed[x0 + 1].value == 1000.0, "... the next jog runs at the 10% values (1000), not the old 50% (5000)");
        MotorAccessDispatch(jr, 1, be);
        be.rejectNext = true;
        MotorAccessReq s90 = sq; s90.num["pct"] = 90;
        o = MotorAccessDispatch(s90, 1, be);
        x0 = be.executed.size();
        MotorAccessDispatch(jq, 1, be);
        CHECK(!o.ok && be.executed.size() >= x0 + 2 && be.executed[x0 + 1].value == 1000.0, "bar to 90 fails -> the jog stays at the slower 10% (never faster than shown)");
        MotorAccessDispatch(jr, 1, be);
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (15)
    //AI(W906-W5-b) 20260925: W5-b 覆核 W5B-11 —— 整段改用真的產生表（WebTeachButtons.gen.inc）；假後端只模擬機台狀態。
    //AI(W906-MERGE-56bbf785) 20260926: the laptop's section (12), renumbered after the machine's (12)..(14); its checks unchanged except
    //  where the merged code follows the machine (teach jog = golden ExtDrive+Acm_AxJog; jog family instead of PTP), plus the merge checks at the end.
    printf("[15] W5-b teach page motion (uteach): real golden registry table, Tag semantics, Go 140/020, hand-teach, quirks, dead-man\n");
    {
        MotorAccessResetJobs();
        TestMotMap()["MInArmZA"] = 7;                                           // ep1Picker remap：golden :3409 的 7 → 3（在第一次建表之前種）
        const std::set<std::string> none;
        std::set<std::string> ax3;  ax3.insert("USE_INDEX_ARM_AXES==IndexArm_3_Axis");
        std::set<std::string> ep16; ep16.insert("USE_PICKER_COUNT==ep16Picker"); ep16.insert("USE_PICKER_COUNT==ep16Picker||SubMachineType==Type_HT9046AU");
        const TeachRegistry R0 = BuildTeachRegistry(none), R3 = BuildTeachRegistry(ax3), R16 = BuildTeachRegistry(ep16);
        CHECK(GenRows().size() == 312 + 6 && R0.P.size() == 212 && R0.T.size() == 45 && R3.P.size() == 212 && R3.T.size() == 45 &&
              R16.P.size() == 226 && R16.T.size() == 45 && R0.selectOnly.size() == 4,
              "real table: 312 golden registrations + 6 W906 extension rows (AI(W906-TEACH-3AXES), IO_CARD_TYPE==PCI1203_IO only); default config TechPara 212 / TechTwoPara 45, ep16Picker 226 / 45; 4 FormShow Tag overrides");
        //AI(W906-TEACH-3AXES) 20261001: EastSun 1001「三軸都幫我加 在合適的地方」—— the six W906 extension rows (tools/gen_teach_registry.py EXT_ROWS)
        //  are the LAST rows of the table (golden's TechPara indices = button Tags unchanged) and live only on a PCI1203_IO machine.
        {
            std::set<std::string> io1203; io1203.insert("IO_CARD_TYPE==PCI1203_IO");
            const TeachRegistry RX = BuildTeachRegistry(io1203);
            bool prefix = RX.P.size() == R0.P.size() + 6 && RX.T.size() == R0.T.size();
            for (std::size_t i = 0; prefix && i < R0.P.size(); ++i) prefix = RX.P[i].key0 == R0.P[i].key0 && RX.P[i].setBtn == R0.P[i].setBtn;
            CHECK(prefix && RX.P[212].name0 == "MOutShuttle1" && RX.P[212].key0 == "setEditOutSht1Left" && RX.P[217].name0 == "MCCDY" &&
                  RX.P[217].key0 == "setEditCCDYCal1x1",
                  "W906 extension: PCI1203_IO config TechPara 212 + 6 (golden rows first, unchanged; MOutShuttle1 left .. MCCDY calibration last)");
            const char* xs[6] = { "btnSetOutSht1Left", "btnGoOutSht1Right", "btnSetOutSht2Left", "btnGoOutSht2Right", "btnSetCCDYSite1x1", "btnGoCCDYCal1x1" };
            const char* xm[6] = { "MOutShuttle1", "MOutShuttle1", "MOutShuttle2", "MOutShuttle2", "MCCDY", "MCCDY" };
            const char* xe[6] = { "setEditOutSht1Left", "setEditOutSht1Right", "setEditOutSht2Left", "setEditOutSht2Right", "setEditCCDYSite1x1", "setEditCCDYCal1x1" };
            bool okX = true;
            for (int i = 0; i < 6; ++i) {
                TeachTarget x;
                const bool isSet = std::string(xs[i]).compare(0, 6, "btnSet") == 0;
                okX = okX && MotorAccessResolveTeachButton(RX, xs[i], kTeachQuirkPolicy, x).empty() && !x.quirk && !x.unregistered &&
                      x.kind == (isSet ? kTkSet140 : kTkGo140) && x.handler == (isSet ? "SetButton140Click" : "GoButton140Click") &&
                      x.ownList == 'P' && x.ownIndex == 212 + i && x.row.name0 == xm[i] && x.row.edit0 == xe[i];
            }
            CHECK(okX, "W906 extension buttons resolve (PCI1203_IO): Set -> SetButton140Click / Go -> GoButton140Click on their own TECH_PARA row (Tag 212..217), not quirks");
            TeachTarget x0;
            CHECK(Has(MotorAccessResolveTeachButton(R0, "btnGoCCDYSite1x1", kTeachQuirkPolicy, x0), "沒有這顆教導按鈕"),
                  "W906 extension buttons on a non-PCI1203_IO machine: not registered, not in golden's dfm -> refused (unknown button)");
        }

        // ---- W5B-1：建構子 OnClick 覆寫、W5B-8：死登錄
        TeachTarget t;
        const char* s06[4] = { "SetButton060", "SetButton061", "SetButton062", "SetButton063" };
        const char* s06e[4] = { "setEditInSht1Left", "setEditInSht1Right", "setEditInSht2Left", "setEditInSht2Right" };
        bool ok06 = true;
        for (int i = 0; i < 4; ++i)
            ok06 = ok06 && MotorAccessResolveTeachButton(R0, s06[i], kTeachQuirkPolicy, t).empty() && t.kind == kTkSet140 && !t.quirk &&
                   t.handler == "SetButton140Click" && t.row.edit0 == s06e[i] && t.row.name0 == (i < 2 ? "MInShuttle1" : "MInShuttle2");
        CHECK(ok06, "W5B-1: SetButton060-063 = SetButton140Click (golden ctor uteach.cpp:271-274 overrides the .dfm SetButton064Click) on their own In-Shuttle rows -- not quirks");
        const char* dead[4] = { "SetButton202", "GoButton202", "SetButton203", "GoButton203" };
        bool okDead = true;
        for (int i = 0; i < 4; ++i) okDead = okDead && Has(MotorAccessResolveTeachButton(R0, dead[i], kTeachQuirkPolicy, t), "按不到") && !t.hiddenWhy.empty();
        CHECK(okDead, "W5B-8 + R-W5B-2: Set/GoButton202/203 (golden :474-475 commented out, .dfm Visible=False) -> golden cannot press them -> refused");

        // ---- W5B-2：golden Tag 語意（最後登錄的列勝出；TechPara 迴圈先、TechTwoPara 後）
        CHECK(MotorAccessResolveTeachButton(R0, "GoButton068", kTeachQuirkPolicy, t).empty() && t.kind == kTkGo140 && t.row.name0 == "MTestZ2" &&
              t.row.edit0 == "setEdLoadCellZ2" && t.ownIndex == 9,
              "W5B-2: GoButton068 (5 rows) -> the LAST one: MTestZ2 / setEdLoadCellZ2 (golden :399), Tag 9");
        CHECK(MotorAccessResolveTeachButton(R0, "GoButton069", kTeachQuirkPolicy, t).empty() && t.row.name0 == "MTestZ2" && t.row.key0 == "setEditTestZSafePos" &&
              t.ownIndex == 5, "W5B-2: GoButton069 (2 rows) -> MTestZ2 (golden :395), Tag 5");
        CHECK(MotorAccessResolveTeachButton(R0, "SetButton064", kTeachQuirkPolicy, t).empty() && t.kind == kTkSet064 && t.ownList == 'T' &&
              t.row.name0 == "MTestY1" && t.row.name1 == "MTestY2", "W5B-2: 4-axis machine: SetButton064 -> TECH_TWOPARA (MTestY1, MTestY2) (golden :916)");
        CHECK(MotorAccessResolveTeachButton(R0, "SetButton065", kTeachQuirkPolicy, t).empty() && t.row.name0 == "MTestY2" && t.row.name1 == "MTestY1",
              "W5B-2: 4-axis machine: SetButton065 -> (MTestY2, MTestY1) (golden :917)");
        CHECK(MotorAccessResolveTeachButton(R3, "GoButton064", kTeachQuirkPolicy, t).empty() && t.kind == kTkGo020 && t.row.name0 == "MTestY1" && t.row.name1 == "MTestY1",
              "W5B-2: IndexArm_3_Axis machine: GoButton064 -> (MTestY1, MTestY1) (golden :911)");

        // ---- 裁決 2：怪按鈕（處理函式讀的清單 != 按鈕 Tag 所屬的清單）
        std::set<std::string> quirks;
        bool allOut = true;
        for (int pass = 0; pass < 2; ++pass) {
            const std::vector<TeachRegRow>& L = pass ? R0.T : R0.P;
            for (std::size_t i = 0; i < L.size(); ++i) {
                const std::string bs[2] = { L[i].setBtn, L[i].goBtn };
                for (int k = 0; k < 2; ++k) {
                    if (bs[k].empty()) continue;
                    TeachTarget q;
                    MotorAccessResolveTeachButton(R0, bs[k], kTeachQuirkRefuse, q);
                    if (q.quirk) { quirks.insert(bs[k]); allOut = allOut && q.outOfRange; }
                }
            }
        }
        CHECK(quirks.size() == 6 && quirks.count("SetBtnPADView_Z") && quirks.count("GoBtnPADView_Z") && quirks.count("SetBtnBGAView_Z") &&
              quirks.count("GoBtnBGAView_Z") && quirks.count("SetBtnScannerAOI_Z") && quirks.count("GoBtnScannerAOI_Z") && allOut,
              "quirk buttons (default config, registered): the 6 reachable TECH_PARA buttons bound to Set/GoButton020Click; their Tag is past TechTwoPara's 45 rows "
              "(SetBtnTopViewKit_Zup/_Z are registered Visible=false -> golden cannot press them, R-W5B-2)");
        // ---- R-W5B-2：golden 按不到的已登錄按鈕（建構子 Visible=false，或父物件 .dfm Visible=False 且沒有任何地方設回 true）
        std::set<std::string> hid;
        for (int pass = 0; pass < 2; ++pass) {
            const std::vector<TeachRegRow>& L = pass ? R0.T : R0.P;
            for (std::size_t i = 0; i < L.size(); ++i) {
                const std::string bs[2] = { L[i].setBtn, L[i].goBtn };
                for (int k = 0; k < 2; ++k) {
                    if (bs[k].empty()) continue;
                    TeachTarget q;
                    if (Has(MotorAccessResolveTeachButton(R0, bs[k], kTeachQuirkRefuse, q), "按不到")) hid.insert(bs[k]);
                }
            }
        }
        const char* hidWant[] = { "SetBtnTopView_Pick", "GoBtnTopView_Pick", "SetBtnTopView_Place", "GoBtnTopView_Place", "SetBtnTopViewKit_Zup",
                                  "GoBtnTopViewKit_Zup", "SetBtnTopViewKit_Z", "GoBtnTopViewKit_Z", "SetBtnInPick", "GoBtnInPick", "SetBtnOutPick", "GoBtnOutPick",
                                  "SetBtnTopView", "GoBtnTopView" };
        bool hidOk = hid.size() == 14;
        for (unsigned i = 0; i < 14; ++i) hidOk = hidOk && hid.count(hidWant[i]);
        CHECK(hidOk, "R-W5B-2: registered buttons golden cannot press = the 14 registered with Visible=false (golden :558-563, :920-921, :927)");
        CHECK(Has(MotorAccessResolveTeachButton(R0, "SetBtnTopView_Place", kTeachQuirkPolicy, t), "Visible=false") &&
              Has(MotorAccessResolveTeachButton(R0, "GoBtnInPick", kTeachQuirkPolicy, t), "pnlInPicker_Go"),
              "R-W5B-2: hidden refusal says why (ctor Visible=false / a hidden .dfm parent)");
        // ---- R-W5B-3：沒登錄、但 .dfm 處理函式是教導處理函式的按鈕 ⇒ golden 用 .dfm Tag 讀清單
        std::string um = MotorAccessResolveTeachButton(R0, "SetInSmartSetupButton", kTeachQuirkPolicy, t);
        CHECK(Has(um, "未登錄") && Has(um, "TechTwoPara[0] = ") && Has(um, "setEditPreciserX") && t.unregistered && t.quirk && t.goldenRowKnown &&
              t.goldenRow.edit0 == "setEditPreciserX" && t.kind == kTkSet020,
              "R-W5B-3: SetInSmartSetupButton (never registered, .dfm Tag 0, SetButton020Click) -> golden reads TechTwoPara[0] = Preciser X/Y -> refused");
        CHECK(Has(MotorAccessResolveTeachButton(R0, "GoInSmartSetup_ZButton", kTeachQuirkOwnRow, t), "TechPara[0] = ") && t.goldenRow.name0 == "MInArmZA",
              "R-W5B-3: GoInSmartSetup_ZButton -> golden GoButton140 on TechPara[0] (MInArmZA); option C does not apply (refused under both policies)");
        um = MotorAccessResolveTeachButton(R0, "SetBtnLoadPort1Z", kTeachQuirkPolicy, t);
        CHECK(Has(um, "TechPara[1004]") && Has(um, "超出") && t.outOfRange, "R-W5B-3: SetBtnLoadPort1Z (.dfm Tag 1004) -> TechPara[1004] out of range");
        CHECK(Has(MotorAccessResolveTeachButton(R0, "sbLeftTopRecs65", kTeachQuirkPolicy, t), "InitialFormOncetime"),
              "R-W5B-3: AOI top/bottom buttons registered only by golden InitialFormOncetime (no FrmAOI in the port) -> refused, says so");
        CHECK(Has(MotorAccessResolveTeachButton(R0, "SetButton070", kTeachQuirkPolicy, t), "按不到") &&
              Has(MotorAccessResolveTeachButton(R0, "GoButtonX", kTeachQuirkPolicy, t), "pnlOutWaitRobot"),
              "R-W5B-2/3: SetButton070 (FormShow :1427 Visible=false) and GoButtonX (hidden panel) -> golden cannot press them");
        CHECK(Has(MotorAccessResolveTeachButton(R0, "btnNoSuchButton", kTeachQuirkPolicy, t), "沒有這顆教導按鈕"), "unknown button -> refused");
        std::string qm = MotorAccessResolveTeachButton(R0, "SetBtnPADView_Z", kTeachQuirkRefuse, t);   // AI(W906-W5-b) 20260925: 拒絕路徑明確帶 kTeachQuirkRefuse（預設已照裁決 2C 改成 OwnRow）
        CHECK(Has(qm, "怪按鈕") && Has(qm, "TechTwoPara[110]") && Has(qm, "未定義行為") && Has(qm, "docs/W5_PROGRESS.md") && Has(qm, "setEditPADViewZ"),
              "quirk refusal says what golden would read (TechTwoPara[110], out of range -> undefined behaviour), its own row, and points at docs/W5_PROGRESS.md");
        CHECK(MotorAccessResolveTeachButton(R0, "SetBtnPADView_Z", kTeachQuirkOwnRow, t).empty() && t.kind == kTkSet140 && t.row.name0 == "MOutArmZE" &&
              MotorAccessResolveTeachButton(R0, "GoBtnPADView_Z", kTeachQuirkOwnRow, t).empty() && t.kind == kTkGo140,
              "option C (kTeachQuirkOwnRow): quirk button uses its own TECH_PARA row with the 140 handlers");
        CHECK(kTeachQuirkPolicy == kTeachQuirkOwnRow, "ruling 2C (RULINGS_20260925 #9): quirk policy constant is OWN ROW (option C)");
        CHECK(MotorAccessResolveTeachButton(R0, "SetBtnPreciserOpen", kTeachQuirkPolicy, t).empty() && t.kind == kTkGo140,
              "SetBtnPreciserOpen: golden .dfm OnClick = GoButton140Click -> it MOVES (handler decides, not the Set/Go column)");
        CHECK(MotorAccessResolveTeachButton(R0, "SetButtonInSh1LtcSenZ1", kTeachQuirkPolicy, t).empty() && t.kind == kTkSelectOnly && t.selectName == "MInSh1LtcSenZ1",
              "SetButtonInSh1LtcSenZ1: FormShow :1518 Tag override -> MotorTrayXClick selects MInSh1LtcSenZ1 only");
        CHECK(Has(MotorAccessResolveTeachButton(R0, "btnStop", kTeachQuirkPolicy, t), "btnStopClick"), "btnStop (registered in a TECH_TWOPARA row) is not a teach handler -> refused");

        // ---- 假機台：1203 軸
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 10000; g.softN = -10000;
        g.jogHigh = 10000; g.initSpeed = 500; g.homeFlag = 1; g.homeHigh = 1000; g.homeLow = 100; g.accDb = 2000; g.decDb = 2000;
        MotorGolden gz = g; gz.indexZ = true; gz.indexMotor = true;
        MotorGolden gy = g; gy.indexY = true; gy.indexMotor = true;
        MotorGolden gr = g; gr.rotateKit = 1;
        MotorGolden gb = g; gb.teachNoServoOff = true;
        MotorGolden ga = g; ga.servoAlarmOn = true;
        FakeBackend be;
        be.reg = R0;
        const int mZA  = Add1203(be, "MInArmZA", 1, ga);
        const int mZ1  = Add1203(be, "MTestZ1", 2, gz);
        const int mZ2  = Add1203(be, "MTestZ2", 3, gz);
        const int mY1  = Add1203(be, "MTestY1", 4, gy);
        const int mY2  = Add1203(be, "MTestY2", 5, gy);
        be.table["MREMAP3"] = Axis1203(3, 16, 0, 6); be.golden[3] = g; be.aliasOf[3] = "MREMAP3"; be.opened.resize(7, true); be.cmdPos[6] = 0; be.state[6] = 1;
        const int mSh1 = Add1203(be, "MInShuttle1", 7, g);
        const int mX   = Add1203(be, "MInArmX", 8, g);
        const int mY   = Add1203(be, "MInArmY", 9, g);
        const int mRot = Add1203(be, "MInRotateKit", 10, gr);
        const int mBrk = Add1203(be, "MTrayBracketZ", 11, gb);
        Add1203(be, "MPreciser", 12, g);
        const int mSh2 = TestMot("MInShuttle2");                               // 非 1203（MN200）
        be.table["MInShuttle2"] = AxisOther(mSh2, "MN200", true); be.golden[mSh2] = g; be.aliasOf[mSh2] = "MInShuttle2";
        CHECK(mZA == 7, "seeded: MInArmZA is MOT[7] in this fake machine");

        // parse: fields object（數字收、null／字串不收）
        MotorAccessReq pr;
        std::string why;
        CHECK(MotorAccessParse("{\"source\":\"uteach\",\"action\":\"teachGo\",\"params\":{\"btn\":\"GoButton020\",\"fields\":{\"a\":1,\"b\":null,\"c\":\"7\",\"d\":-3}}}", pr, why) &&
              pr.str["btn"] == "GoButton020" && pr.obj["fields"].size() == 2 && pr.obj["fields"]["a"] == 1 && pr.obj["fields"]["d"] == -3,
              "parse: fields object keeps numbers only (null / string = not given)");

        // ---- GoButton140（GoButton030 → MInArmZA 軸槽 1）＋ W5B-4 目標值驗證 ＋ W5B-13 EditPtr
        MotorAccessReq q = TeachReq("teachGo", "GoButton030", false, false);
        int n0 = (int)be.executed.size();
        MotorAccessOutcome o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "缺 fields") && (int)be.executed.size() == n0, "W5B-4: no fields -> refused, nothing sent");
        q.obj["fields"]["somethingElse"] = 5;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "頁面沒送欄位 setEditInZSafeHeight") && Has(o.ackJson, "[editPtr=setEditInZSafeHeight,active=MInArmZA]") && (int)be.executed.size() == n0,
              "W5B-4: the row's field missing (page did not load it from C++) -> refused; golden already set EditPtr (W5B-13) and ActiveMotorIndex (R-W5B-1)");
        q.obj["fields"]["setEditInZSafeHeight"] = 12.5;
        CHECK(Has(MotorAccessDispatch(q, 1, be).ackJson, "不是有限的整數"), "W5B-4: non-integer -> refused");
        q.obj["fields"]["setEditInZSafeHeight"] = std::nan("");
        CHECK(Has(MotorAccessDispatch(q, 1, be).ackJson, "不是有限的整數") && (int)be.executed.size() == n0, "W5B-4: NaN -> refused, nothing sent");
        q.obj["fields"]["setEditInZSafeHeight"] = 500;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 500 && be.executed.back().axis == 1 &&
              (int)be.executed.size() == n0 + 5 && be.executed[n0 + 1].value == 100.0 && !be.teachCanMoveCalls.empty() && be.teachCanMoveCalls.back() == 7 &&
              Has(o.ackJson, "\"editPtr\":\"setEditInZSafeHeight\"") && Has(o.ackJson, "\"activeMotor\":\"MInArmZA\"") && AckShapeOk(o.ackJson, 7),
              "GoButton140: CheckCanMove/IsCanQuickJogMove on MOT[7], speed 1% (velHigh 100), moveAbs 500 = the field value; ack carries editPtr");
        be.golden[mZA].homeFlag = 0;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "need home") && !Has(o.ackJson, "editPtr") && Has(o.ackJson, "[active=MInArmZA]"),
              "GoButton140: HomeFlag==0 -> need home, EditPtr NOT changed (golden sets it after :3421); ActiveMotorIndex already changed (:3408, R-W5B-1)");
        be.golden[mZA].homeFlag = 1;
        q.obj["fields"]["setEditInZSafeHeight"] = 10001;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Over max soft limit") && Has(o.ackJson, "[editPtr=setEditInZSafeHeight,active=MInArmZA]"), "GoButton140: > softP -> golden message, EditPtr already set");
        be.canTeach = false; n0 = (int)be.executed.size();
        q.obj["fields"]["setEditInZSafeHeight"] = 500;
        CHECK(!MotorAccessDispatch(q, 1, be).ok && (int)be.executed.size() == n0, "CheckCanMove/IsCanQuickJogMove false -> refused, nothing sent");
        be.canTeach = true;
        be.ep1 = true;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && be.executed.back().axis == 6 && !be.teachCanMoveCalls.empty() && be.teachCanMoveCalls.back() == 3, "W5B-11: ep1Picker: GoButton140 remaps MOT[7] -> MOT[3] (golden :3409) and moves that axis");
        be.ep1 = false;
        be.regOk = false;
        CHECK(Has(MotorAccessDispatch(q, 1, be).ackJson, "對不上"), "registry mismatch (generated table stale) -> every teach button refused (fail-closed)");
        be.regOk = true;

        // ---- W5B-2 端對端：GoButton068 動 golden 的最後一列（MTestZ2），不是頁面第一個點的 MTestZ1
        MotorAccessReq q68 = TeachReq("teachGo", "GoButton068", false, false);
        q68.obj["fields"]["setEditWaitTestZDown"] = 111; q68.obj["fields"]["setEdLoadCellY1"] = 222; q68.obj["fields"]["setEdLoadCellY2"] = 333;
        q68.obj["fields"]["setEdLoadCellZ1"] = 444; q68.obj["fields"]["setEdLoadCellZ2"] = 555;
        q68.motors.push_back("MTestZ1");                                        // 頁面送的馬達不再決定列
        o = MotorAccessDispatch(q68, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().axis == 3 && be.executed.back().value == 555,
              "W5B-2: GoButton068 -> MTestZ2 (slot 3) to setEdLoadCellZ2 = 555 (golden TechPara[Tag], last registration), page motors ignored");
        MotorAccessReq q69 = TeachReq("teachGo", "GoButton069", false, false); q69.obj["fields"]["setEditTestZSafePos"] = 77;
        o = MotorAccessDispatch(q69, 1, be);
        CHECK(o.ok && be.executed.back().axis == 3 && be.executed.back().value == 77, "W5B-2: GoButton069 -> MTestZ2");

        // ---- GoButton020（兩軸 20%）＋ W5B-11 IndexArm_3_Axis ＋ 第 2 軸超軟體極限
        MotorAccessReq q64 = TeachReq("teachGo", "GoButton064", false, false);
        q64.obj["fields"]["setEditIndex1ToSocketY"] = 100; q64.obj["fields"]["setEditIndex2ToSht2Y"] = 200;
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(q64, 1, be);
        CHECK(o.ok && CountKind(be, n0, kCmdAxMoveAbs) == 2 && Has(o.ackJson, "\"editPtr\":\"setEditIndex2ToSht2Y\""),
              "GoButton064 (4-axis): MTestY1 -> 100 and MTestY2 -> 200; EditPtr ends on SetEdit[1] (golden :3593)");
        be.reg = R3; be.indexArm3 = true;
        q64.obj["fields"].clear(); q64.obj["fields"]["setEditIndex1ToSocketY"] = 150;
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(q64, 1, be);
        CHECK(o.ok && CountKind(be, n0, kCmdAxMoveAbs) == 1 && be.executed.back().axis == 4 && Has(o.ackJson, "axis 2 skipped") &&
              Has(o.ackJson, "\"editPtr\":\"setEditIndex1ToSocketY\""),
              "W5B-11: IndexArm_3_Axis: GoButton064 = (MTestY1, MTestY1), axis 2 skipped (golden :3578-3584), EditPtr stays SetEdit[0]");
        be.reg = R0; be.indexArm3 = false;
        MotorAccessReq q20 = TeachReq("teachGo", "GoButton020", false, false);
        q20.obj["fields"]["setEditLoaderX"] = 100; q20.obj["fields"]["setEditLoaderY"] = 200;
        be.golden[mX].homeFlag = 0;                                             // golden GoButton020 不看 HomeFlag
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(q20, 1, be);
        CHECK(o.ok && CountKind(be, n0, kCmdAxMoveAbs) == 2 && be.executed[n0 + 1].value == 2000.0, "GoButton020: both axes at 20% (velHigh 2000), no HomeFlag check");
        be.golden[mX].homeFlag = 1;
        q20.obj["fields"]["setEditLoaderY"] = 20000;
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(q20, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Over max soft limit") && Has(o.ackJson, "第 1 軸已經在走") && Has(o.ackJson, "[editPtr=setEditLoaderY,active=MInArmY]") &&
              CountKind(be, n0, kCmdAxMoveAbs) == 1 && be.executed.back().axis == 8,
              "W5B-11: GoButton020 axis 2 over soft limit: axis 1 already moving (golden same), refusal says so; EditPtr = SetEdit[1]");

        // ---- 旋轉站背隙（W5B-9）
        MotorAccessReq qr = TeachReq("teachGo", "GoButtonRotateA", false, false); qr.obj["fields"]["setEditRotateA"] = 1000;
        CHECK(!MotorAccessDispatch(qr, 1, be).ok, "rotate kit without backlash setting -> refused (golden reads edtEditRotateInBacklash)");
        qr.num["backlashIn"] = 5;
        be.cmdPos[10] = 0.0;                                                    // lastDirP 初值 true、目標 1000 > 0 ⇒ 不補
        o = MotorAccessDispatch(qr, 1, be);
        CHECK(o.ok && be.executed.back().value == 1000 && Has(o.ackJson, "\"backlash\":0") && be.lastDirP[mRot] == true,
              "W5B-9: last dir + (golden init true), goal > pos -> no backlash; Move1203 records iLastRotatorDirP=true (golden MotorMovePosition :590)");
        be.lastDirP[mRot] = false;
        o = MotorAccessDispatch(qr, 1, be);
        CHECK(o.ok && be.executed.back().value == 1005, "W5B-9: last dir -, goal > pos -> +5 (golden GetRotatorBacklash)");
        be.cmdPos[10] = 2000.0; be.lastDirP[mRot] = true; qr.num["backlashIn"] = 0; be.prodBacklashIn = 3;
        o = MotorAccessDispatch(qr, 1, be);
        CHECK(o.ok && be.executed.back().value == 997 && be.lastDirP[mRot] == false,
              "W5B-9: position from the 1203 monitor (2000), last dir +, goal < pos -> -Prod.iIn_iRotateA_Backlash (setting 0), then dir recorded as -");
        MotorAccessReq ma = Req("uMotorTest", "moveAbsolute", "MInRotateKit"); ma.num["targetPos"] = 5000;
        MotorAccessDispatch(ma, 1, be);
        CHECK(be.lastDirP[mRot] == true, "W5B-9: uMotorTest moveAbsolute (Move1203) records iLastRotatorDirP too");

        // ---- AI(W906-TEACH-SVON) 20261001: 激磁教導（the default, no 手動教導 tick): no servo off, no dialog, the position at once
        {
            MotorAccessReq sv = TeachReq("teachSet", "SetButton030", true, true); sv.flag.erase("hand");
            be.actPos[1] = 222.0;
            const std::size_t x0s = be.executed.size();
            o = MotorAccessDispatch(sv, 1, be);
            bool svCmd = false;
            for (std::size_t k = x0s; k < be.executed.size(); ++k) if (be.executed[k].kind == kCmdAxSvOn) svCmd = true;
            CHECK(o.ok && Has(o.ackJson, "\"result\":\"taught\"") && Has(o.ackJson, "\"positions\":[222]") && !svCmd &&
                  !Has(o.ackJson, "\"teachActive\":true") && !MotorAccessJobs().teachActive && Has(o.ackJson, "\"editPtr\":\"setEditInZSafeHeight\""),
                  "TEACH-SVON: Set without the hand-teach tick -> the encoder 222 written to EditPtr at once, no servo command, no hand-teach job");
            sv.flag["hand"] = false;
            o = MotorAccessDispatch(sv, 1, be);
            CHECK(o.ok && Has(o.ackJson, "\"positions\":[222]") && !MotorAccessJobs().teachActive, "TEACH-SVON: hand=false is the same servo-on teach");
            be.actPos[1] = 0.0;
        }
        // ---- 手動教導 SetButton140（SetButton030 → MInArmZA 軸槽 1，ServoAlarmOn）
        MotorAccessReq qs = TeachReq("teachSet", "SetButton030", true, true);
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(qs, 1, be);
        CHECK(o.ok && (int)be.executed.size() == n0 + 1 && be.executed.back().kind == kCmdAxSvOn && be.executed.back().value == 0.0 && be.executed.back().axis == 1 &&
              Has(o.ackJson, "\"teachActive\":true") && Has(o.ackJson, "\"editPtr\":\"setEditInZSafeHeight\""), "SetButton140 begin: servo off, teachActive, EditPtr");
        MotorAccessReq jq = Req("uteach", "jogP", "MInArmX"); jq.num["speed"] = 10;
        CHECK(Has(MotorAccessDispatch(jq, 1, be).ackJson, "手動教導中"), "while hand-teaching, other teach-page motion is refused (golden modal)");
        CHECK(Has(MotorAccessDispatch(Req("uteach", "servoToggle", "MInArmZA"), 1, be).ackJson, "手動教導中"), "W5B-5: teach-page servo button refused too");
        MotorAccessReq mj = Req("uMotorTest", "jogP", "MInArmZA"); mj.num["speed"] = 10;
        CHECK(Has(MotorAccessDispatch(mj, 1, be).ackJson, "教導頁手動教導中"), "W5B-5: uMotorTest on the SAME axis refused");
        MotorAccessReq mj2 = Req("uMotorTest", "jogP", "MInArmX"); mj2.num["speed"] = 10;
        CHECK(MotorAccessDispatch(mj2, 1, be).ok, "W5B-5: uMotorTest on another axis still allowed");
        { MotorAccessReq rs = Req("uMotorTest", "stop", "MInArmX"); rs.button = "sbMotorTest_JogP"; MotorAccessDispatch(rs, 1, be); }
        CHECK(MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmZA"), 1, be).ok, "STOP is never blocked");
        //AI(W906-MERGE-56bbf785) 20260926: review finding (manual-teach guard bypass) -- every motor of a request is checked, and the Motor
        //  Test page-wide actions are refused while hand-teaching; nothing reaches the card or the golden objects. Releases pass (a stop).
        {
            const int cT = be.Calls(), rlT = be.reloadCalls, pwT = be.powerOnBegins, fcT = be.formCloses;
            const std::size_t soT = be.servoOffs.size();
            MotorAccessReq laT = ReqStart("uMotorTest", "loopMove", 0, true); laT.num["mode"] = 1; laT.num["waitTime"] = 0;
            laT.motors.push_back("MInArmX"); laT.motors.push_back("MInArmZA");                 // the taught axis SECOND
            laT.num["pos1.MInArmX"] = 100; laT.num["pos2.MInArmX"] = 200; laT.num["pos1.MInArmZA"] = 100; laT.num["pos2.MInArmZA"] = 200;
            o = MotorAccessDispatch(laT, 1, be);
            CHECK(!o.ok && Has(o.ackJson, "MInArmZA: 教導頁手動教導中") && !MotorAccessJobs().loopActive && be.Calls() == cT,
                  "review: All-mode loop with the taught axis SECOND -> refused (TeachAxisBusyWhy checks every motor, not only motors[0]), nothing sent");
            MotorAccessReq laO = laT; laO.motors.pop_back();                                  // the taught axis not checked at all
            o = MotorAccessDispatch(laO, 1, be);
            CHECK(!o.ok && Has(o.ackJson, "教導頁手動教導中") && Has(o.ackJson, "All 模式") && !MotorAccessJobs().loopActive && be.Calls() == cT,
                  "review: All-mode loop without the taught axis -> refused too (page-wide: golden fTeachShow is ShowModal)");
            MotorAccessReq rlq = Req("uMotorTest", "reloadMotorData", 0); rlq.flag["confirm"] = true;
            o = MotorAccessDispatch(rlq, 1, be);
            CHECK(!o.ok && Has(o.ackJson, "教導頁手動教導中") && be.reloadCalls == rlT && be.Calls() == cT,
                  "review: reloadMotorData while hand-teaching -> refused, the table is not re-applied (be.Calls() unchanged)");
            MotorAccessReq lsT = Req("uMotorTest", "lightScale", 0); lsT.button = "BitBtn2";
            lsT.num["axisItem"] = 0; lsT.num["moveType"] = 0; lsT.num["pitch"] = 300; lsT.num["delayMs"] = 0;
            const MotorAccessOutcome lsO = MotorAccessDispatch(lsT, 1, be);
            const MotorAccessOutcome pwO = MotorAccessDispatch(Req("uMotorTest", "motorPowerToggle", 0), 1, be);
            const MotorAccessOutcome fsO = MotorAccessDispatch(Req("uMotorTest", "formShow", 0), 1, be);
            CHECK(!lsO.ok && Has(lsO.ackJson, "教導頁手動教導中") && !MotorAccessLightScale(1).active &&
                  !pwO.ok && Has(pwO.ackJson, "教導頁手動教導中") && be.powerOnBegins == pwT &&
                  !fsO.ok && Has(fsO.ackJson, "教導頁手動教導中") && be.servoOffs.size() == soT && be.formCloses == fcT && be.Calls() == cT,
                  "review: Light Scale start / Motor Power / FormShow while hand-teaching -> refused, nothing switched or sent");
            //AI(W906-MERGE-56bbf785) 20260926: second review -- a release passes only when Motor Test has something to stop. Here no
            //  loop and no home job exist, so both releases name the taught axis for nothing: refused, and NO Stop1203 reaches the
            //  axis the operator is pushing by hand (DoHome's no-job release would send golden's unconditional DecStop).
            const std::size_t xh = be.executed.size();
            o = MotorAccessDispatch(ReqStart("uMotorTest", "loopMove", "MInArmZA", false), 1, be);
            CHECK(!o.ok && Has(o.ackJson, "教導頁手動教導中") && be.executed.size() == xh,
                  "review 2: a LoopMove release with no loop running, naming the taught axis -> refused, nothing sent");
            o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmZA", false), 1, be);
            CHECK(!o.ok && Has(o.ackJson, "教導頁手動教導中") && be.executed.size() == xh,
                  "review 2: a HOME release with no home job, naming the taught axis -> refused, no Stop1203 on the hand-pushed axis");
            CHECK(MotorAccessJobs().teachActive, "... and the hand-teach is still active");
        }
        MotorAccessReq qq = Req("uteach", "teachSet", "MInArmZA"); qq.flag["query"] = true;   // W5B-R1：頁面送的 motors＝教導軸本身
        o = MotorAccessDispatch(qq, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"teachActive\":true") && Has(o.ackJson, "\"teachBtn\":\"SetButton030\"") && Has(o.ackJson, "\"editPtr\":\"setEditInZSafeHeight\""),
              "W5B-5: query after a page reload sees the orphan hand-teach (button, EditPtr)");
        CHECK(!MotorAccessDispatch(qs, 1, be).ok, "second begin refused");
        be.actPos[1] = 333.0;
        MotorAccessReq qe = TeachReq("teachSet", "SetButton030", true, false); qe.flag["accept"] = true;
        qe.motors.push_back("MInArmZA");                                        // W5B-R1：頁面的確定鈕帶著目前選的馬達（＝教導軸）
        o = MotorAccessDispatch(qe, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxSvOn && be.executed.back().value == 1.0 && Has(o.ackJson, "\"positions\":[333]") &&
              MotorAccessJobs().encBaseAxes == 1 && !MotorAccessJobs().teachActive,
              "W5B-R1: accept with motors=[the taught axis] (the page's real request) is NOT refused as 'same axis'; encoder 333 (golden GetTechPos), servo back on; ServoAlarmOn axis -> encoder base (W5B-6)");
        be.cmdPos[1] = 0.0; be.actPos[1] = 4000.0;
        MotorAccessReq mv = Req("uteach", "moveRelative", "MInArmZA"); mv.button = "btnMoveP"; mv.num["interval"] = 50;
        o = MotorAccessDispatch(mv, 1, be);
        CHECK(o.ok && be.executed.back().value == 4050, "W5B-6: after hand-teach the relative move is based on the encoder (4000+50), not the stale cmdPos (0)");
        o = MotorAccessDispatch(qe, 1, be);
        CHECK(o.ok && Has(o.ackJson, "teachIdle"), "end with nothing active -> idle, harmless");

        // W5B-14：Contec＋MotorType==0 單軸讀 ReadPos；SetButton140 對不關伺服的軸結束時照樣 ServoOnOff(true)
        be.usesReadPos.insert(mSh1); be.cmdPos[7] = 1234.0; be.actPos[7] = 999.0;
        MotorAccessDispatch(TeachReq("teachSet", "SetButton060", true, true), 1, be);
        MotorAccessReq qe60 = TeachReq("teachSet", "SetButton060", true, false); qe60.flag["accept"] = true;
        o = MotorAccessDispatch(qe60, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"positions\":[1234]"), "W5B-14: Contec && MotorType==0 -> GetTechPos reads ReadPos (1234), not the encoder (999)");
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(TeachReq("teachSet", "SetBtnTrayBracketSaftZ", true, true), 1, be);
        CHECK(o.ok && (int)be.executed.size() == n0, "MTrayBracketZ: golden does not servo off (:3392)");
        MotorAccessReq qeb = TeachReq("teachSet", "SetBtnTrayBracketSaftZ", true, false);
        o = MotorAccessDispatch(qeb, 1, be);
        CHECK(o.ok && (int)be.executed.size() == n0 + 1 && be.executed.back().kind == kCmdAxSvOn && be.executed.back().value == 1.0 && be.executed.back().axis == 11,
              "W5B-14: ...but golden's unconditional ServoOnOff(true) after the dialog (:3396) is sent");
        (void)mBrk;

        // 兩軸手動教導 SetButton020（loader X/Y）＋ W5B-12 回滾
        MotorAccessReq qab = TeachReq("teachSet", "SetButton020", true, true);
        be.golden[mY].homeFlag = 0;
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(qab, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "need home") && (int)be.executed.size() == n0 && Has(o.ackJson, "[editPtr=setEditLoaderX,editPtr1=setEditLoaderY]"),
              "SetButton020: 2nd axis not homed -> refused BEFORE any servo off (golden would leave axis 1 free); EditPtr/EditPtr1 already set");
        be.golden[mY].homeFlag = 1;
        be.svFail[std::make_pair(9, 0)] = 1;
        o = MotorAccessDispatch(qab, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "已把前面關掉的伺服開回") && be.executed.back().kind == kCmdAxSvOn && be.executed.back().value == 1.0 && be.executed.back().axis == 8 &&
              !MotorAccessJobs().teachActive, "W5B-12: 2nd servo-off fails -> 1st turned back on (checked), honest message");
        be.svFail[std::make_pair(9, 0)] = 1; be.svFail[std::make_pair(8, 1)] = 1;
        o = MotorAccessDispatch(qab, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "開回失敗") && !Has(o.ackJson, "已把前面關掉的伺服開回"), "W5B-12: rollback servo-on also fails -> says so (no false 'turned back on')");
        o = MotorAccessDispatch(qab, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"editPtr1\":\"setEditLoaderY\""), "SetButton020 begin ok (both axes servo off)");
        be.svFail[std::make_pair(8, 1)] = 1;
        MotorAccessReq qabe = TeachReq("teachSet", "SetButton020", true, false);
        o = MotorAccessDispatch(qabe, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "伺服已嘗試開回") && !MotorAccessJobs().teachActive, "W5B-11: servo-on failure at the end -> refused, honest, job ended");

        // SetButton064（Index Y 不關伺服）、SetButton066（Index Z 不做手動教導）
        n0 = (int)be.executed.size();
        MotorAccessReq qb64 = TeachReq("teachSet", "SetButton064", true, true); qb64.motors.push_back("MInArmX");   // 頁面當時選的馬達（golden ActiveMotorIndex）
        o = MotorAccessDispatch(qb64, 1, be);
        CHECK(o.ok && (int)be.executed.size() == n0 && Has(o.ackJson, "\"teachActive\":true") && !Has(o.ackJson, "activeMotor"),
              "SetButton064: MTestY1/Y2 keep servo on (:3683), dialog still shown; golden does not change ActiveMotorIndex (no activeMotor, R-W5B-1)");
        MotorAccessReq qe64 = TeachReq("teachSet", "SetButton064", true, false); qe64.flag["accept"] = true;
        be.actPos[4] = 41.0; be.actPos[5] = 52.0;
        o = MotorAccessDispatch(qe64, 1, be);
        CHECK(o.ok && (int)be.executed.size() == n0 && Has(o.ackJson, "\"positions\":[41,52]"), "SetButton064 accept: both encoders, no servo command at the end either");
        // R-W5B-8：頁面沒有選馬達（golden ActiveMotorIndex==-1）時，SetButton064／020 的確定什麼都不寫（golden GetTechPos :3476）
        o = MotorAccessDispatch(TeachReq("teachSet", "SetButton064", true, true), 1, be);
        o = MotorAccessDispatch(qe64, 1, be);
        CHECK(o.ok && Has(o.ackJson, "taughtNothing") && !Has(o.ackJson, "positions") && !MotorAccessJobs().teachActive,
              "R-W5B-8: two-axis hand-teach with ActiveMotorIndex==-1 -> golden GetTechPos returns first: nothing written (job still ends)");
        o = MotorAccessDispatch(TeachReq("teachSet", "SetButton066", true, true), 1, be);
        CHECK(o.ok && Has(o.ackJson, "noHandTeach"), "SetButton140 on Index Z -> golden does no hand-teach");
        (void)mZ1; (void)mZ2; (void)mY1; (void)mY2;

        // 非 1203 軸（MN200）：golden MOT 物件
        const std::size_t sv0 = be.servoSet.size();
        o = MotorAccessDispatch(TeachReq("teachSet", "SetButton062", true, true), 1, be);
        CHECK(o.ok && be.servoSet.size() == sv0 + 1 && be.servoSet.back().first == mSh2 && !be.servoSet.back().second, "W5B-11: non-1203 hand-teach -> golden MOT.ServoOnOff(false)");
        be.encPos[mSh2] = 4321;
        MotorAccessReq qe62 = TeachReq("teachSet", "SetButton062", true, false); qe62.flag["accept"] = true;
        o = MotorAccessDispatch(qe62, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"positions\":[4321]") && be.servoSet.back().second, "W5B-11: non-1203 accept -> MOT.ReadEncoderPos, ServoOnOff(true)");
        MotorAccessReq qg62 = TeachReq("teachGo", "GoButton062", false, false); qg62.obj["fields"]["setEditInSht2Left"] = 600;
        const std::size_t mv0 = be.moves.size();
        o = MotorAccessDispatch(qg62, 1, be);
        CHECK(o.ok && be.moves.size() == mv0 + 1 && be.moves.back().mi == mSh2 && be.moves.back().target == 600 && be.moves.back().pct == 1,
              "W5B-11: non-1203 GoButton140 -> golden MOT.MotorMove at 1%");

        // SetBtnPreciserOpen 是移動、LtcSen 只選馬達、怪按鈕不動
        MotorAccessReq qp = TeachReq("teachSet", "SetBtnPreciserOpen", true, true); qp.obj["fields"]["setEditPreciserPitchOpen"] = 300;
        o = MotorAccessDispatch(qp, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 300 && !Has(o.ackJson, "teachActive\":true"),
              "SetBtnPreciserOpen (golden OnClick GoButton140Click): pressing it MOVES, no hand-teach");
        const int c0 = be.AllCalls();
        o = MotorAccessDispatch(TeachReq("teachSet", "SetButtonInSh1LtcSenZ1", true, true), 1, be);
        CHECK(o.ok && Has(o.ackJson, "selectOnly") && Has(o.ackJson, "\"selectMotor\":\"MInSh1LtcSenZ1\"") && be.AllCalls() == c0, "LtcSen Set button: select motor only, nothing sent");
        // AI(W906-W5-b) 20260925: 裁決 2C 之後怪按鈕不再被拒；這裡只驗預設政策下解析到自己那一列（不送分派：會開始手動教導、影響後面的段落）
        CHECK(MotorAccessResolveTeachButton(R0, "SetBtnPADView_Z", kTeachQuirkPolicy, t).empty() && t.kind == kTkSet140 && t.row.name0 == "MOutArmZE" && be.AllCalls() == c0,
              "ruling 2C: quirk button resolves to its own TECH_PARA row under the default policy (SetButton140 on MOutArmZE), nothing sent");

        // ---- W5B-R5／R6：手動教導沒有死人開關（golden 的 fTeachShow 不會自己關，伺服只在確定／取消後開）；START 在手動教導中拒絕
        MotorAccessDispatch(qs, 1, be);
        CHECK(MotorAccessJobs().teachActive, "hand-teach active again");
        std::size_t e0 = be.executed.size();
        MotorAccessTick(be, 500, false);
        bool noSvOn = true;
        for (std::size_t i = e0; i < be.executed.size(); ++i) if (be.executed[i].kind == kCmdAxSvOn) noSvOn = false;
        CHECK(MotorAccessJobs().teachActive && noSvOn, "W5B-R5: operator connection gone -> hand-teach KEPT, no automatic servo-on (the operator's hand may be on the axis)");
        o = MotorAccessDispatch(qq, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"teachActive\":true") && Has(o.ackJson, "\"teachBtn\":\"SetButton030\""),
              "W5B-R6: after a disconnect / reload the page's query finds the hand-teach and can bring the dialog back");
        e0 = be.executed.size();
        MotorAccessOnAlarm(be, "WAR16435");
        noSvOn = true;
        int stops = 0;
        for (std::size_t i = e0; i < be.executed.size(); ++i) { if (be.executed[i].kind == kCmdAxSvOn) noSvOn = false; if (be.executed[i].kind == kCmdAxStop) ++stops; }
        CHECK(MotorAccessJobs().teachActive && noSvOn && stops > 0, "W5B-R5: alarm -> stop all (golden note.cpp:795), hand-teach and servo-off stay (golden dialog stays open)");
        std::string sw;
        CHECK(MotorAccessStartBlocked(sw) && Has(sw, "手動教導中") && Has(sw, "SetButton030"), "W5B-R5: START is refused while hand-teaching (golden: START unreachable while fTeachShow is modal)");  { std::string hw, hw2; bool lg = false, lg2 = true; CHECK(MotorAccessHomeBlocked("ScanKey", hw, lg) && lg && hw.compare(0, std::string("HOME 拒絕：教導頁手動教導中").size(), "HOME 拒絕：教導頁手動教導中") == 0 && Has(hw, "SetButton030") && !Has(hw, "Arm Cell"), "HOMEBLOCK (U14 item 3): TfMain::Home is refused too while hand-teaching, with the HOME text, logged the first time"); CHECK(MotorAccessHomeBlocked("ScanKey", hw2, lg2) && !lg2 && hw2 == hw, "HOMEBLOCK: the same (func, why) again (ESD decay re-asks every other tick) -> refused, not logged again"); CHECK(MotorAccessHomeBlocked("S2F42", hw2, lg2) && lg2, "HOMEBLOCK: another source (SECS RCMD HOME) -> logged once for it"); }   //AI(W906-HOMEBLOCK) 20261003: NB2-1, same line
        o = MotorAccessDispatch(qe, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxSvOn && be.executed.back().value == 1.0 && !MotorAccessStartBlocked(sw),
              "the operator's OK ends it (servo on) -> START allowed again");  { std::string hw; bool lg = true; CHECK(!MotorAccessHomeBlocked("ScanKey", hw, lg) && !lg, "HOMEBLOCK: hand teach over -> TfMain::Home allowed again"); CHECK(MotorAccessHomeBlocked("ScanKey", hw, lg) == false, "HOMEBLOCK: still allowed (the pass cleared the log latch)"); }   //AI(W906-HOMEBLOCK) 20261003: NB2-1, same line

        // ---- W5B-R2：1203 軸的「在原點」—— EastSun 監看器沒設 CFG_AxOrgLogic，極性未量 ⇒ 不明（fail-closed）
        CHECK(kPci1203CardOrgLogic == -1, "W5B-R2: card ORG logic not measured yet (kPci1203CardOrgLogic = -1)");
        be.motionIO.erase(1);
        CHECK(MotorAccessTeachHomeLed(be, mZA) == -1 && Has(MotorAccessTeachHomeUnknownWhy(), "樣本"), "W5B-3: 1203 axis without a monitor sample -> -1 (unknown -> treated as NOT home)");
        be.motionIO[1] = 0x10;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == -1 && Has(MotorAccessTeachHomeUnknownWhy(), "CFG_AxOrgLogic"),
              "W5B-R2: ORG bit set but the card's ORG logic is unknown -> -1 (NOT 'at home'), reason names CFG_AxOrgLogic");
        CHECK(MotorAccessOrgHomeLed(true, false, 1) == 1 && MotorAccessOrgHomeLed(false, false, 1) == 0 &&
              MotorAccessOrgHomeLed(true, false, 0) == 0 && MotorAccessOrgHomeLed(false, false, 0) == 1 &&
              MotorAccessOrgHomeLed(true, true, 0) == 1 && MotorAccessOrgHomeLed(false, true, 1) == 1 && MotorAccessOrgHomeLed(true, false, -1) == -1,
              "W5B-R2: card logic == golden (SensorType ? 0 : 1) -> bit as-is; different -> inverted; unknown -> -1");
        //AI(W906-HT9050-ORG) 20261001: HT9050＋1203 分支 —— EastSun「home 是 high 時 代表沒偵測到 是 low 時 代表有偵測到」
        g_W906OrgActiveLow = []() { return true; };
        //AI(W906-HT9050-ORG-ST) 20261002 (NB2): per axis by Mot_Table SensorType (golden InitMotor :492 uOrgLogic = bSensorType ? ORG_ACT_LOW : ORG_ACT_HIGH)
        const bool zaSensorSaved = be.golden[mZA].sensorType;
        be.golden[mZA].sensorType = true;                                       // SensorType=1: ORG low = at home (EastSun 1001, the HT9050 table after this change)
        be.motionIO[1] = 0x10;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 0, "HT9050-ORG-ST: SensorType=1, ORG bit high -> NOT at home");
        be.motionIO[1] = 0x00;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 1, "HT9050-ORG-ST: SensorType=1, ORG bit low -> at home");
        be.golden[mZA].sensorType = false;                                      // SensorType=0: ORG high = at home (as SMC's ORG_Positive)
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 0, "HT9050-ORG-ST: SensorType=0, ORG bit low -> NOT at home");
        be.motionIO[1] = 0x10;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 1, "HT9050-ORG-ST: SensorType=0, ORG bit high -> at home");
        //AI(W906-ORG-INV) 20261002: EastSun「分支1203時 teach 和 mottest 頁面home燈號都反向」-- g_W906OrgInvert flips the HT9050 answer
        g_W906OrgInvert = true;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 0, "ORG-INV: SensorType=0, ORG bit high -> NOT at home when inverted");
        be.motionIO[1] = 0x00;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 1, "ORG-INV: SensorType=0, ORG bit low -> at home when inverted");
        be.golden[mZA].sensorType = true;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 0, "ORG-INV: SensorType=1, ORG bit low -> NOT at home when inverted");
        be.golden[mZA].sensorType = false;
        be.motionIO[1] = 0x10;
        g_W906OrgInvert = false;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 1, "ORG-INV: back to not inverted -> SensorType=0, ORG bit high -> at home");
        {   // no golden motor object -> SensorType unknown -> -1 (fail-closed), reason names SensorType
            const MotorGolden gSaved = be.golden[mZA];
            be.golden.erase(mZA);
            CHECK(MotorAccessTeachHomeLed(be, mZA) == -1 && Has(MotorAccessTeachHomeUnknownWhy(), "SensorType"),
                  "HT9050-ORG-ST: no golden motor object -> -1 (SensorType unknown, NOT 'at home')");
            be.golden[mZA] = gSaved;
        }
        be.golden[mZA].sensorType = true;
        be.motionIO.erase(1);
        CHECK(MotorAccessTeachHomeLed(be, mZA) == -1, "HT9050-ORG: no monitor sample -> still -1 (fail-closed)");
        be.golden[mZA].sensorType = zaSensorSaved;
        g_W906OrgActiveLow = 0;
        be.motionIO[1] = 0x10;                                                  // 回到上面 W5B-R2 那一段的狀態
        be.freezePolls = true;
        MotorAccessDispatch(mv, 1, be);                                         // 下一個命令，輪詢不前進
        CHECK(MotorAccessTeachHomeLed(be, mZA) == -1 && Has(MotorAccessTeachHomeUnknownWhy(), "新樣本"), "W5B-3: sample older than the last command -> unknown (fail-closed)");
        be.freezePolls = false;
        CHECK(MotorAccessTeachHomeLed(be, mSh2) == -2 && MotorAccessTeachHomeLed(be, 999) == -2, "W5B-3: non-1203 / unknown index -> -2 (golden Led path)");
        be.table["MInArmZA"].tableEnable = false;
        CHECK(MotorAccessTeachHomeLed(be, mZA) == 1, "W5B-3: Enable=0 -> at home (golden ScanMotorStatus else-branch)");
        be.table["MInArmZA"].tableEnable = true;
        CHECK(MotorAccessTeachLive1203(be, mZA) && !MotorAccessTeachLive1203(be, mSh2), "W5B-3: live 1203 -> body runs in SOFT_SIMULTE; MN200 -> golden return true");
        be.ready = false;
        CHECK(!MotorAccessTeachLive1203(be, mZA), "W5B-3: 1203 control not armed -> not live");
        be.ready = true;
        //AI(W906-TEACH-ZDISABLED) 20261001: EastSun 1001「如果 testz2 enable 是0 那就不要擋testz2」—— Teach 互鎖的掛勾（MotorAccessTeachInterlockHome）。
        //  -3 進 forms/fTeach.cpp W906_TeachHomeLed＝不檢查；-2＝golden Led[iHomeLed]（false 照樣擋，訊息照 golden）。本體那一半在 tests/test_gali_route_engine.cpp part 15。
        {
            const MotorAccessAxis z2Saved = be.table["MTestZ2"];
            be.table["MTestZ2"] = AxisOther(mZ2, "SMC", true);                  // HT9050 Mot_Table M15：CardModel SMC、Enable 0（golden Galil 分支照樣 Enable=true）
            be.table["MTestZ2"].tableEnable = false;
            CHECK(MotorAccessTeachInterlockHome(be, mZ2) == -3, "TEACH-ZDISABLED (a): MTestZ2 non-1203 with Mot_Table Enable=0 -> -3 (the teach interlock does not check it)");
            CHECK(MotorAccessTeachHomeLed(be, mZ2) == -2, "TEACH-ZDISABLED: the engine's MotorAccessTeachHomeLed is unchanged for that row (-2 = golden Led)");
            be.table["MTestZ2"].tableEnable = true;
            CHECK(MotorAccessTeachInterlockHome(be, mZ2) == -2, "TEACH-ZDISABLED (b): MTestZ2 non-1203 with Enable=1 -> -2 (golden Led[iHomeLed]; LED false still refuses)");
            CHECK(MotorAccessTeachInterlockHome(be, 999) == -2, "TEACH-ZDISABLED: no Mot_Table row -> -2 (golden, as before)");
            be.table["MTestZ2"] = z2Saved;
            CHECK(MotorAccessTeachInterlockHome(be, mSh2) == -2, "TEACH-ZDISABLED: an enabled MN200 row -> -2 as before");
            // (c) 1203 軸：跟 MotorAccessTeachHomeLed 一模一樣（HT9050 ORG 規則、Enable=0＝1 不是 -3、沒樣本＝-1 fail-closed）
            g_W906OrgActiveLow = []() { return true; };
            const bool zaSensorSaved2 = be.golden[mZA].sensorType;
            be.golden[mZA].sensorType = true;                                   //AI(W906-HT9050-ORG-ST) 20261002: SensorType=1 = low at home (the HT9050 table)
            be.motionIO[1] = 0x10;
            CHECK(MotorAccessTeachInterlockHome(be, mZA) == 0 && MotorAccessTeachHomeLed(be, mZA) == 0, "TEACH-ZDISABLED (c): 1203 ORG bit high -> 0 (unchanged)");
            be.motionIO[1] = 0x00;
            CHECK(MotorAccessTeachInterlockHome(be, mZA) == 1 && MotorAccessTeachHomeLed(be, mZA) == 1, "TEACH-ZDISABLED (c): 1203 ORG bit low -> 1 (unchanged)");
            be.motionIO.erase(1);
            CHECK(MotorAccessTeachInterlockHome(be, mZA) == -1, "TEACH-ZDISABLED (c): enabled 1203 axis without a sample -> -1 (still fail-closed)");
            be.table["MInArmZA"].tableEnable = false;
            CHECK(MotorAccessTeachInterlockHome(be, mZA) == 1, "TEACH-ZDISABLED (c): 1203 Enable=0 -> 1 as before (not -3)");
            be.table["MInArmZA"].tableEnable = true;
            be.golden[mZA].sensorType = zaSensorSaved2;
            g_W906OrgActiveLow = 0;
            be.motionIO[1] = 0x10;                                              // 回到上面那一段的狀態
        }

        // ---- 教導頁 jog／移動／歸零
        be.cmdPos[8] = 0.0;
        jq.motors[0] = "MInArmX";
        const std::size_t xd0 = be.extDrive.size();
        o = MotorAccessDispatch(jq, 1, be);
        //AI(W906-MERGE-56bbf785) 20260926: was `kind == kCmdAxMoveVel` ("teach jogP -> moveVel (EastSun)") -- the merged teach jog is the
        //  machine's golden jog (MT-E1): jog family at the axis' current pct, Acm_AxSetExtDrive(ax, 1), Acm_AxJog.
        //AI(W906-MERGE-56bbf785) 20260926: review finding -- + the jog-family value: DoJog's source (no Motor Test bar yet -> 1% = 100),
        //  no longer the teach page's PTP pct (TeachCurPct was 20% here after GoButton020 -> it used to send 2000).
        CHECK(o.ok && be.executed.back().kind == kCmdAxJogStart && be.executed.back().dir == 1 && be.executed.back().axis == 8 &&
              be.executed.size() >= 5 && be.executed[be.executed.size() - 4].speed == kSpeedJogRun && be.executed[be.executed.size() - 4].value == 100.0 &&
              be.extDrive.size() == xd0 + 1 && be.extDrive.back().value == 1.0 && be.extDrive.back().axis == 8,
              "teach jogP -> golden jog: CFG_AxJog* speeds at DoJog's source (1%, velHigh 100), Acm_AxSetExtDrive(ax, 1), Acm_AxJog +1 (machine MT-E1; was moveVel)");
        { MotorAccessReq rs = Req("uteach", "stop", "MInArmX"); rs.button = "btnJogP"; MotorAccessDispatch(rs, 1, be); }
        CHECK(be.executed.back().kind == kCmdAxStop && be.extDrive.back().value == 0.0 && be.extDrive.back().axis == 8,
              "teach jog release -> Stop1203 (Acm_AxStopDec + Acm_AxSetExtDrive(ax, 0)): the axis leaves jog mode");
        be.cmdPos[8] = 10000.0;
        o = MotorAccessDispatch(jq, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "over positive soft limit"), "teach jogP at >= softP -> golden Ztex soft-limit refusal");
        be.cmdPos[8] = 1000.0;
        MotorAccessReq mn = Req("uteach", "moveRelative", "MInArmX"); mn.button = "btnMoveN"; mn.num["interval"] = -50;
        o = MotorAccessDispatch(mn, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 950, "teach MoveN: now 1000 - 50 -> 950");
        be.golden[mX].homeFlag = 0;
        MotorAccessReq mt = Req("uteach", "moveAbsolute", "MInArmX"); mt.num["targetPos"] = 300;
        o = MotorAccessDispatch(mt, 1, be);
        CHECK(o.ok && be.executed.back().value == 300, "teach MoveTo: no HomeFlag check (golden btnMoveToClick)");
        be.golden[mX].homeFlag = 1;
        be.powerOff = true;
        MotorAccessReq hq = Req("uteach", "home", "MInArmX"); hq.flag["start"] = true;
        CHECK(Has(MotorAccessDispatch(hq, 1, be).ackJson, "Motor power is OFF"), "teach home with motor power off -> refused");
        hq.flag["start"] = false;
        std::size_t sa = be.stopAll.size();
        CHECK(Has(MotorAccessDispatch(hq, 1, be).ackJson, "Motor power is OFF") && be.stopAll.size() == sa,
              "W5-b extra: golden checks motor power before `if(btnHome->Down)` -- release refused too (power is off, nothing can move)");
        be.powerOff = false;
        hq.flag["start"] = true;
        o = MotorAccessDispatch(hq, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxHome && MotorAccessJobs().homesActive == 1, "teach home start -> DS402 home job (W4-b2 path)");
        hq.flag["start"] = false;
        sa = be.stopAll.size();
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(hq, 1, be);
        CHECK(o.ok && be.stopAll.size() == sa + 1 && MotorAccessJobs().homesActive == 0 && CountKind(be, n0, kCmdAxSetSpeed) == 4 &&
              be.executed.back().kind == kCmdAxSetSpeed && be.executed[be.executed.size() - 3].value == 100.0,
              "W5B-11: teach home release -> StopAllMotor + home cancelled + golden MOT.SetSpeed(1) on the 1203 axis (velHigh 100)");
        be.failSetSpeed = 1;
        sa = be.stopAll.size();
        o = MotorAccessDispatch(hq, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "SetSpeed(1) 失敗") && Has(o.ackJson, "停止已送") && be.stopAll.size() == sa + 1,
              "W5B-12: SetSpeed(1) refused by the card -> honest refusal (the stop was sent)");
        MotorAccessReq hn = Req("uteach", "home", "MInShuttle2"); hn.flag["start"] = false;
        const std::size_t ss0 = be.setSpeeds.size();
        o = MotorAccessDispatch(hn, 1, be);
        CHECK(o.ok && be.setSpeeds.size() == ss0 + 1 && be.setSpeeds.back().first == mSh2 && be.setSpeeds.back().second == 1,
              "W5B-11: teach home release on a non-1203 axis -> golden MOT.SetSpeed(1)");

        // ---- 覆核第二輪（AI(W906-W5-b) 20260925）----
        // R-W5B-1：golden ActiveMotorIndex 跟著處理函式走，ack 帶 activeMotor 讓頁面改選
        o = MotorAccessDispatch(q68, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"activeMotor\":\"MTestZ2\""), "R-W5B-1: GoButton068 ack -> activeMotor MTestZ2 (golden TechPara[Tag]->MotorSelect, :3408), not the page's first point MTestZ1");
        MotorAccessReq q64b = TeachReq("teachGo", "GoButton064", false, false);
        q64b.obj["fields"]["setEditIndex1ToSocketY"] = 100; q64b.obj["fields"]["setEditIndex2ToSht2Y"] = 200;
        o = MotorAccessDispatch(q64b, 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"activeMotor\":\"MTestY2\""), "R-W5B-1: GoButton064 (GoButton020Click) -> ActiveMotorIndex ends on MotorSelect[1] = MTestY2 (first loop :3569-3574)");
        be.aliasOf[TestMot("MInSh1LtcSenZ1")] = "MInSh1LtcSenZ1";              // Mot_Table 有這一軸的別名
        o = MotorAccessDispatch(TeachReq("teachSet", "SetButtonInSh1LtcSenZ1", true, true), 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"activeMotor\":\"MInSh1LtcSenZ1\""), "R-W5B-1: MotorTrayXClick -> activeMotor");

        // R-W5B-4：教導頁的 jog／MoveP／MoveN／MoveTo 不設速度，用 golden 這一軸目前的速度
        MotorAccessReq hx = Req("uteach", "home", "MInArmX"); hx.flag["start"] = false;
        o = MotorAccessDispatch(hx, 1, be);
        CHECK(o.ok && MotorAccessGoldenSpeedPct(mX) == 1 && be.stopAll.back() == "uteach.home",
              "R-W5B-6: teach HOME release -> golden StopAllMotor only (source uteach.home: no MTestY1 Galil ST) + SetSpeed(1) (R-W5B-4 records 1%)");
        be.cmdPos[8] = 1000.0;
        MotorAccessReq mp = Req("uteach", "moveRelative", "MInArmX"); mp.button = "btnMoveP"; mp.num["interval"] = 50; mp.num["speed"] = 50;
        o = MotorAccessDispatch(mp, 1, be);
        CHECK(o.ok && be.executed.back().value == 1050 && LastRunSpeed(be) == 100.0,
              "R-W5B-4: MoveP after HOME release runs at 1% (velHigh 100) even though the page shows edtSpeed 50 (golden btnMovePClick does not SetSpeed)");
        MotorAccessReq mpe = mp; mpe.flag["speedEvent"] = true;
        o = MotorAccessDispatch(mpe, 1, be);
        CHECK(o.ok && LastRunSpeed(be) == 5000.0 && MotorAccessGoldenSpeedPct(mX) == 50,
              "R-W5B-4: the page reports the operator's edtSpeed change (speedEvent) -> golden edtSpeedChange SetSpeed(50) -> 50%");
        o = MotorAccessDispatch(mp, 1, be);
        CHECK(o.ok && LastRunSpeed(be) == 5000.0, "R-W5B-4: next MoveP without an event keeps golden's 50%");
        MotorAccessReq jx = Req("uteach", "jogP", "MInArmX"); jx.num["speed"] = 80;
        o = MotorAccessDispatch(jx, 1, be);
        //AI(W906-MERGE-56bbf785) 20260926: was `kCmdAxMoveVel && LastRunSpeed == 5000` -- same speed, now on the jog family (golden jog)
        //AI(W906-MERGE-56bbf785) 20260926: review finding -- the jog family is DoJog's source now: the Motor Test bar never moved -> 1% (100),
        //  not the teach page's 50% (was `LastJogRunSpeed == 5000`); the PTP family keeps the 50% of the MoveP above (5000).
        //AI(W906-MERGE-56bbf785) 20260926: second review -- golden ScrollBar1Change (uteach.cpp:1318) is SetSpeed(Speed/3, true): the teach
        //  page's edtSpeed change to 50 above also wrote the jog family at 50/3 = 16% (1600 at jogHigh 10000). The jog's own speed 80 is
        //  still ignored (golden JogP drops it) and the PTP family keeps 50% (5000).
        CHECK(o.ok && be.executed.back().kind == kCmdAxJogStart && LastJogRunSpeed(be) == 1600.0 && LastRunSpeed(be) == 5000.0,
              "R-W5B-4 + review 2: teach jog ignores its speed 80; the jog family is the teach edtSpeed 50 / 3 = 16% (golden ScrollBar1Change), PTP untouched");
        { MotorAccessReq rs = Req("uteach", "stop", "MInArmX"); rs.button = "btnJogP"; MotorAccessDispatch(rs, 1, be); }
        //AI(W906-TEACH-SPD-AXIS) 20261001: the SAME speed event value on ANOTHER axis is a change for that axis: MInArmX's 50 above, then
        //  the operator picks MInArmY and its speed is 50 too -> MInArmY's jog family must be written (50/3 = 16%), not skipped as "no change".
        {
            MotorAccessReq ye = Req("uteach", "moveRelative", "MInArmY"); ye.button = "btnMoveP"; ye.num["interval"] = 10;
            ye.num["speed"] = 50; ye.flag["speedEvent"] = true;
            CHECK(MotorAccessDispatch(ye, 1, be).ok, "TEACH-SPD-AXIS setup: MInArmY MoveP carrying the speed event 50 (same value as MInArmX's)");
            MotorAccessReq jy = Req("uteach", "jogP", "MInArmY"); jy.num["speed"] = 80;
            o = MotorAccessDispatch(jy, 1, be);
            const double expY = MotorSpeedFromPct(16, be.golden[mY]).velHigh;
            CHECK(o.ok && be.executed.back().kind == kCmdAxJogStart && LastJogRunSpeed(be) == expY,
                  "TEACH-SPD-AXIS: MInArmY's jog runs at ITS 50/3 = 16% (the per-axis change test), not whatever its jog family held");
            { MotorAccessReq rs = Req("uteach", "stop", "MInArmY"); rs.button = "btnJogP"; MotorAccessDispatch(rs, 1, be); }
        }
        //AI(W906-MERGE-56bbf785) 20260926: review finding -- after a Motor Test speed bar (golden SetSpeed(30, true)) the teach jog runs at the
        //  values that bar WROTE (g_jogSpd), even when JogHighSpeed changed in memory since (golden: the card keeps CFG_AxJog*).
        CHECK(MotorAccessDispatch(Req("uMotorTest", "selectMotor", "MInArmX"), 1, be).ok, "setup: Motor Test selects MInArmX (golden lM00Click: bar at 1)");
        MotorAccessReq sb30 = Req("uMotorTest", "setSpeed", "MInArmX"); sb30.num["pct"] = 30; sb30.flag["jog"] = true;
        CHECK(MotorAccessDispatch(sb30, 1, be).ok && LastJogRunSpeed(be) == 3000.0, "setup: Motor Test speed bar 30 -> SetSpeed(30, true): jog family 3000");
        be.golden[mX].jogHigh = 100000;                                         // a later cell edit / Copy From raised JogHighSpeed (memory only)
        o = MotorAccessDispatch(jx, 1, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxJogStart && be.executed.size() >= 5 &&
              be.executed[be.executed.size() - 4].speed == kSpeedJogRun && be.executed[be.executed.size() - 4].value == 3000.0,
              "review: teach jog = DoJog's source -- the 3000 the Motor Test bar wrote, not the teach pct and not 30% of today's JogHighSpeed (30000)");
        { MotorAccessReq rs = Req("uteach", "stop", "MInArmX"); rs.button = "btnJogP"; MotorAccessDispatch(rs, 1, be); }
        be.golden[mX].jogHigh = 10000;
        be.cmdPos[9] = 0.0;
        MotorAccessReq q20c = TeachReq("teachGo", "GoButton020", false, false);
        q20c.obj["fields"]["setEditLoaderX"] = 100; q20c.obj["fields"]["setEditLoaderY"] = 200;
        o = MotorAccessDispatch(q20c, 1, be);
        CHECK(o.ok && MotorAccessGoldenSpeedPct(mX) == 20 && MotorAccessGoldenSpeedPct(mY) == 20, "R-W5B-4: GoButton020 leaves both axes at golden's 20%");
        MotorAccessReq my = Req("uteach", "moveRelative", "MInArmY"); my.button = "btnMoveP"; my.num["interval"] = 10;
        o = MotorAccessDispatch(my, 1, be);
        CHECK(o.ok && LastRunSpeed(be) == 2000.0, "R-W5B-4: MoveP right after GoButton020 on MInArmY runs at 20% (velHigh 2000), as golden");
        const std::size_t mvn = be.moves.size(), jgn = be.jogs.size();
        MotorAccessReq ms = Req("uteach", "moveRelative", "MInShuttle2"); ms.button = "btnMoveP"; ms.num["interval"] = 10; ms.num["speed"] = 70;
        be.readPos[mSh2] = 100;
        o = MotorAccessDispatch(ms, 1, be);
        MotorAccessReq js = Req("uteach", "jogN", "MInShuttle2"); js.num["speed"] = 70;
        MotorAccessDispatch(js, 1, be);
        { MotorAccessReq rs = Req("uteach", "stop", "MInShuttle2"); rs.button = "btnJogN"; MotorAccessDispatch(rs, 1, be); }
        CHECK(o.ok && be.moves.size() == mvn + 1 && !be.moves.back().setSpeed && be.jogs.size() == jgn + 1 && be.jogs.back().pct == -1,
              "R-W5B-4: non-1203 teach MoveP / jog -> golden MOT.MotorMove / JogP without SetSpeed (the MOT object keeps golden's speed)");
        const std::size_t ssn = be.setSpeeds.size();
        MotorAccessReq mse = ms; mse.flag["speedEvent"] = true; mse.num["speed"] = 30;
        MotorAccessDispatch(mse, 1, be);
        CHECK(be.setSpeeds.size() >= ssn + 1 && be.setSpeeds[ssn].first == mSh2 && be.setSpeeds[ssn].second == 30,
              "R-W5B-4: speed event on a non-1203 axis -> golden MOT.SetSpeed(30) right away (edtSpeedChange)");
        //AI(W906-MERGE-56bbf785) 20260926: review finding (manual-teach guard) -- a Light Scale scan running when the hand-teach begins can
        //  still be turned OFF from Motor Test (a stop); only a press that would START a scan is refused (checked in the W5B-5 block above).
        MotorAccessReq lsK = Req("uMotorTest", "lightScale", 0); lsK.button = "BitBtn2";
        lsK.num["axisItem"] = 0; lsK.num["moveType"] = 0; lsK.num["pitch"] = 300; lsK.num["delayMs"] = 0;
        CHECK(MotorAccessDispatch(lsK, 1, be).ok && MotorAccessLightScale(1).active, "setup: Light Scale on before the hand-teach");
        MotorAccessDispatch(qs, 1, be);
        CHECK(MotorAccessGoldenSpeedPct(mZA) == 1, "R-W5B-4: SetButton140 -> golden UpdateMotorTeachMonitor SetSpeed(1) on the taught axis");
        o = MotorAccessDispatch(lsK, 1, be);
        CHECK(MotorAccessJobs().teachActive && o.ok && Has(o.ackJson, "lightScaleOff") && !MotorAccessLightScale(1).active,
              "review: while hand-teaching, turning a RUNNING Light Scale off passes (stop direction); the hand-teach stays");
        MotorAccessReq qc = TeachReq("teachSet", "SetButton030", true, false); qc.motors.push_back("MInArmZA");
        CHECK(MotorAccessDispatch(qc, 1, be).ok && !MotorAccessJobs().teachActive, "cancel ends the hand-teach (servo back on)");

        // W5B-R3：教導頁／MotorTest 的每一個命令都清 fAllMotorHome（golden 進教導頁 FormShow :1606）
        const int ca = be.clearAllHome;
        MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmX"), 1, be);
        MotorAccessDispatch(Req("uteach", "setTeachFromCurrent", "MInArmX"), 1, be);
        CHECK(ca > 0 && be.clearAllHome == ca + 2, "W5B-R3: every uteach / uMotorTest command clears fAllMotorHome (next START homes everything first)");

        // W5B-R4：運轉中（SystemStart）教導頁／MotorTest 不動，只有停止放行
        be.systemStart = true;
        const int cs0 = be.AllCalls();
        CHECK(Has(MotorAccessDispatch(jx, 1, be).ackJson, "SystemStart") && Has(MotorAccessDispatch(mj2, 1, be).ackJson, "SystemStart") &&
              Has(MotorAccessDispatch(q20c, 1, be).ackJson, "SystemStart") && be.AllCalls() == cs0,
              "W5B-R4: SystemStart -> teach jog / MotorTest jog / teach Go refused, nothing sent (golden main.cpp:27827 sbTeachingClick)");
        CHECK(MotorAccessDispatch(Req("uteach", "stop", "MInArmX"), 1, be).ok && MotorAccessDispatch(qq, 1, be).ok && MotorAccessDispatch(qc, 1, be).ok,
              "W5B-R4: STOP, teachSet query and teachSet end still allowed while running");
        //AI(W906-MERGE-56bbf785) 20260926: the laptop's run gate meets the machine's actions --
        //  (a) nothing clears fAllMotorHome while SystemStart (golden sbTeachingClick returns before the teach FormShow can clear it;
        //      the machine's Motor Test page re-sends a refused formShow every 1.5-10 s);
        //  (b) Motor Power passes the gate (EastSun R9 "must not be blocked"), so does formClose (golden FormClose moves nothing);
        //  (c) formShow is still refused while running (golden cannot open Motor Test during a run).
        {
            const int caRun = be.clearAllHome;
            be.relayOut = true;                                                 // relay on: golden Motor Power asks first (:1628) -- no side effect
            const MotorAccessOutcome mpw = MotorAccessDispatch(Req("uMotorTest", "motorPowerToggle", 0), 1, be);
            const int fc0 = be.formCloses;
            const MotorAccessOutcome fcl = MotorAccessDispatch(Req("uMotorTest", "formClose", 0), 1, be);
            const MotorAccessOutcome fsh = MotorAccessDispatch(Req("uMotorTest", "formShow", 0), 1, be);
            be.relayOut = false;
            CHECK(!mpw.ok && Has(mpw.ackJson, "needConfirm") && !Has(mpw.ackJson, "SystemStart"),
                  "merge: Motor Power is not blocked while running (EastSun R9) -- it reaches golden's own YES/NO");
            CHECK(fcl.ok && be.formCloses == fc0 + 1, "merge: formClose passes while running (golden FormClose moves nothing)");
            CHECK(!fsh.ok && Has(fsh.ackJson, "SystemStart"), "merge: formShow is still refused while running");
            CHECK(be.clearAllHome == caRun, "merge: no command clears fAllMotorHome while SystemStart (golden: the teach page cannot open during a run)");
        }
        //AI(W906-MERGE-56bbf785) 20260926: review finding (run gate blocked STOP-direction requests) -- while SystemStart, Motor Test's
        //  LoopMove / HOME release and turning a RUNNING Light Scale off pass and really stop; a Light Scale that would start stays refused.
        {
            const int hfX = be.golden[mX].homeFlag;
            be.systemStart = false;
            MotorAccessReq lpS = ReqStart("uMotorTest", "loopMove", "MInArmX", true);
            lpS.num["pos1"] = 100; lpS.num["pos2"] = 200; lpS.flag["confirmNotHomed"] = true;
            CHECK(MotorAccessDispatch(lpS, 1, be).ok && MotorAccessJobs().loopActive, "setup: a Motor Test loop on MInArmX (machine idle)");
            be.systemStart = true;
            std::size_t xs = be.executed.size();
            MotorAccessReq lpE = lpS; lpE.flag["start"] = false;
            o = MotorAccessDispatch(lpE, 1, be);
            CHECK(o.ok && Has(o.ackJson, "loopStopped") && !MotorAccessJobs().loopActive && CountKind(be, xs, kCmdAxStop) == 1 && be.executed.back().axis == 8,
                  "review: LoopMove release while SystemStart -> passes the run gate and stops the loop (golden PCIL132_StopMotor on MInArmX)");
            be.systemStart = false;
            MotorAccessReq hmS = ReqStart("uMotorTest", "home", "MInArmX", true);
            CHECK(MotorAccessDispatch(hmS, 1, be).ok && MotorAccessJobs().homesActive == 1, "setup: a Motor Test HOME on MInArmX (machine idle)");
            be.systemStart = true;
            xs = be.executed.size();
            MotorAccessReq hmE = hmS; hmE.flag["start"] = false;
            o = MotorAccessDispatch(hmE, 1, be);
            CHECK(o.ok && Has(o.ackJson, "homeStopped") && MotorAccessJobs().homesActive == 0 && CountKind(be, xs, kCmdAxStop) == 1 && be.executed.back().axis == 8,
                  "review: HOME release while SystemStart -> passes the run gate, the home job is cancelled and the axis stopped");
            //AI(W906-MERGE-56bbf785) 20260926: second review -- with NO Motor Test job, a HOME / LoopMove release while running is not a
            //  stop of anything of Motor Test's: refused, and no Stop1203 reaches an axis the engine may be driving.
            xs = be.executed.size();
            CHECK(Has(MotorAccessDispatch(hmE, 1, be).ackJson, "SystemStart") && Has(MotorAccessDispatch(lpE, 1, be).ackJson, "SystemStart") &&
                  be.executed.size() == xs,
                  "review 2: HOME / LoopMove release with no job while SystemStart -> refused, nothing sent (no Stop1203 on an engine axis)");
            xs = be.executed.size();
            CHECK(Has(MotorAccessDispatch(hmS, 1, be).ackJson, "SystemStart") && Has(MotorAccessDispatch(lpS, 1, be).ackJson, "SystemStart") &&
                  be.executed.size() == xs && MotorAccessJobs().homesActive == 0 && !MotorAccessJobs().loopActive,
                  "review: a HOME / LoopMove START is still refused while running, nothing sent");
            MotorAccessReq lsR = Req("uMotorTest", "lightScale", 0); lsR.button = "BitBtn2";
            lsR.num["axisItem"] = 0; lsR.num["moveType"] = 0; lsR.num["pitch"] = 300; lsR.num["delayMs"] = 0;
            o = MotorAccessDispatch(lsR, 1, be);
            CHECK(!o.ok && Has(o.ackJson, "SystemStart") && !MotorAccessLightScale(1).active,
                  "review: Light Scale with Timer2 OFF (the press would START a scan) is still refused while running");
            be.systemStart = false;
            CHECK(MotorAccessDispatch(lsR, 1, be).ok && MotorAccessLightScale(1).active, "setup: Light Scale on (machine idle)");
            be.systemStart = true;
            o = MotorAccessDispatch(lsR, 1, be);
            CHECK(o.ok && Has(o.ackJson, "lightScaleOff") && !MotorAccessLightScale(1).active,
                  "review: Light Scale with Timer2 ON while SystemStart -> passes the run gate and turns the scan off (golden BitBtn2Click)");
            be.golden[mX].homeFlag = hfX;                                       // the HOME start above cleared it (golden); put back what the later checks had
        }
        be.systemStart = false;

        // R-W5B-5：歸零完成照 golden MotorHome case 20 設 iLastRotatorDirP=true
        be.lastDirP[mRot] = false; be.state[10] = 1;
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInRotateKit", true), 1, be);
        be.state[10] = 4; MotorAccessTick(be, 500, true);
        be.state[10] = 1; MotorAccessTick(be, 500, true);
        CHECK(o.ok && be.homeFlags[mRot] == 1 && be.lastDirP[mRot] == true, "R-W5B-5: home done -> HomeFlag=1 and iLastRotatorDirP=true (golden mymotor.cpp:1721)");

        // W5B-R8：GoButton020 的欄位檢查在第 1 軸動之前兩軸一起做
        MotorAccessReq q20d = TeachReq("teachGo", "GoButton020", false, false); q20d.obj["fields"]["setEditLoaderX"] = 100;
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(q20d, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "兩軸都還沒動") && Has(o.ackJson, "setEditLoaderY") && CountKind(be, n0, kCmdAxMoveAbs) == 0,
              "W5B-R8: axis 2's field missing -> refused before axis 1 moves (no 'only one axis moved')");

        // W5B-R9：背隙畫面值要是有限整數；目標＋背隙不可溢位
        be.cmdPos[10] = 0.0; be.lastDirP[mRot] = false;
        n0 = (int)be.executed.size();
        qr.num["backlashIn"] = std::nan("");
        CHECK(Has(MotorAccessDispatch(qr, 1, be).ackJson, "背隙設定不是有限的整數"), "W5B-R9: backlash NaN -> refused");
        qr.num["backlashIn"] = 1e12;
        CHECK(Has(MotorAccessDispatch(qr, 1, be).ackJson, "背隙設定不是有限的整數"), "W5B-R9: backlash 1e12 -> refused");
        qr.num["backlashIn"] = 2147483647.0;
        CHECK(Has(MotorAccessDispatch(qr, 1, be).ackJson, "超出 int 範圍") && CountKind(be, n0, kCmdAxMoveAbs) == 0, "W5B-R9: target + backlash overflow -> refused, nothing moved");
        qr.num["backlashIn"] = 5;

        // R-W5B-2／3 走分派的整條路：按不到／未登錄的按鈕什麼都不送
        const int cu0 = be.AllCalls();
        CHECK(Has(MotorAccessDispatch(TeachReq("teachSet", "SetInSmartSetupButton", true, true), 1, be).ackJson, "未登錄") &&
              Has(MotorAccessDispatch(TeachReq("teachGo", "GoBtnTopView", false, false), 1, be).ackJson, "按不到") && be.AllCalls() == cu0,
              "R-W5B-2/3: unregistered / hidden buttons refused end-to-end, nothing sent");

        // ui
        const int u0 = be.Calls();
        o = MotorAccessDispatch(Req("uteach", "setTeachFromCurrent", "MInArmX"), 1, be);
        CHECK(o.ok && Has(o.ackJson, "\"layer\":\"ui\"") && be.Calls() == u0, "setTeachFromCurrent -> ui only (page writes EditPtr)");
        (void)mSh1;
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (WSLINK-B)
    //AI(W906-WSLINK-B) 20260929: St01 13:32 (b) -- the page-table close edge (MotorAccessPageClosed) runs the C++ half of the
    //  closed window's golden FormClose while the connection is still up (one WebSocket per browser).
    printf("[WSLINK-B] page closed with the connection up: Motor Test = DoFormClose + its jogs; Teach = btnStop->Click()\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 999999; g.softN = -999999;
        g.jogHigh = 10000; g.initSpeed = 500; g.homeFlag = 1; g.acc = 1000; g.dec = 1000;
        g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100;
        FakeBackend be;
        be.table["A"] = Axis1203(20, 20, 0, 3);
        be.table["B"] = Axis1203(21, 21, 0, 4);
        be.golden[20] = g; be.golden[21] = g;
        be.opened.assign(6, true);
        for (int ax = 0; ax < 6; ++ax) { be.cmdPos[ax] = 0.0; be.state[ax] = 1; }

        // [1] a Motor Test jog: the connection dead-man keeps it (operator connected), the fMotorTest close edge stops it
        MotorAccessReq q = Req("uMotorTest", "jogP", "A"); q.button = "sbMotorTest_JogP"; q.num["speed"] = 10;
        MotorAccessOutcome o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().jogsActive == 1, "WSLINK-B Motor Test jog registers");
        MotorAccessTick(be, 500, true);
        CHECK(MotorAccessJobs().jogsActive == 1, "WSLINK-B operator still connected (same browser) -> the connection dead-man keeps the jog");
        int n0 = (int)be.executed.size();
        std::size_t sa0 = be.stopAll.size();
        MotorAccessPageClosed(be, "fMotorTest");
        CHECK(MotorAccessJobs().jogsActive == 0 && (int)be.executed.size() == n0 + 1 && be.executed.back().kind == kCmdAxStop &&
              be.executed.back().axis == 3 && be.stopAll.size() == sa0 && Has(MotorAccessJobs().lastNote, "fMotorTest closed"),
              "WSLINK-B fMotorTest closed -> its jogging axis gets the dead-man stop (kCmdAxStop), no StopAllMotor");

        // [2] a Teach jog survives a Motor Test close (only the closed page's jogs stop)
        q = Req("uteach", "jogP", "B"); q.button = "btnJogP"; q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().jogsActive == 1, "WSLINK-B Teach jog registers");
        n0 = (int)be.executed.size();
        MotorAccessPageClosed(be, "fMotorTest");
        CHECK(MotorAccessJobs().jogsActive == 1 && (int)be.executed.size() == n0, "WSLINK-B fMotorTest closed -> the Teach jog keeps going, nothing sent");
        MotorAccessReq rel = Req("uteach", "stop", "B"); rel.button = "btnJogP";
        MotorAccessDispatch(rel, 1, be);
        CHECK(MotorAccessJobs().jogsActive == 0, "WSLINK-B Teach jog released by its page");

        // [3] LoopMove + formShow, then fMotorTest closes: golden FormClose -- loop ended without a stop, fShow=false, GoldenFormClose once
        MotorAccessReq fs = Req("uMotorTest", "formShow", 0); fs.button = "FormShow";
        MotorAccessDispatch(fs, 1, be);
        q = ReqStart("uMotorTest", "loopMove", "A", true); q.num["pos1"] = 1; q.num["pos2"] = 2;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().loopActive && MotorAccessJobs().pageShown, "WSLINK-B loop runs, page shown");
        n0 = (int)be.executed.size();
        int fc0 = be.formCloses;
        MotorAccessPageClosed(be, "fMotorTest");
        CHECK(!MotorAccessJobs().loopActive && !MotorAccessJobs().pageShown && (int)be.executed.size() == n0 && be.formCloses == fc0 + 1,
              "WSLINK-B fMotorTest closed -> DoFormClose: LoopMove ended, no stop sent, fShow=false, GoldenFormClose once");
        MotorAccessPageClosed(be, "fMotorTest");
        CHECK(be.formCloses == fc0 + 1, "WSLINK-B closed again with nothing shown / looping -> FormClose half not repeated");

        // [4] a Motor Test HOME keeps going over a Motor Test close (DoFormClose's rule) and still completes
        MotorAccessDispatch(fs, 1, be);
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "B", true), 1, be);
        CHECK(o.ok && MotorAccessJobs().homesActive == 1, "WSLINK-B home job runs");
        n0 = (int)be.executed.size();
        MotorAccessPageClosed(be, "fMotorTest");
        CHECK(MotorAccessJobs().homesActive == 1 && (int)be.executed.size() == n0,
              "WSLINK-B fMotorTest closed -> the HOME job keeps going, nothing sent (DoFormClose: golden-safe side, HomeFlag only on a real finish)");

        // [5] fTeach closes while idle: golden btnStop->Click() -- StopAllMotor("uteach"), the 1203 axes stopped, every job cancelled
        //     (the HOME from [4] is still running -- golden cannot jog then: every motion button returns on btnHome->Down)
        sa0 = be.stopAll.size();
        n0 = (int)be.executed.size();
        MotorAccessPageClosed(be, "fTeach");
        CHECK(be.stopAll.size() == sa0 + 1 && be.stopAll.back() == "uteach" && MotorAccessJobs().homesActive == 0,
              "WSLINK-B fTeach closed (idle) -> GoldenStopAll(uteach) once, HOME cancelled (golden uteach.cpp:2092 btnStop->Click())");
        CHECK(CountKind(be, (std::size_t)n0, kCmdAxStop) >= 1, "WSLINK-B fTeach closed -> the 1203 axes get a stop (Stop1203All)");
        q = Req("uMotorTest", "jogP", "A"); q.button = "sbMotorTest_JogP"; q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().jogsActive == 1, "WSLINK-B a Motor Test jog after the HOME is gone");
        MotorAccessPageClosed(be, "fTeach");
        CHECK(MotorAccessJobs().jogsActive == 0 && be.stopAll.size() == sa0 + 2, "WSLINK-B fTeach closed -> every jog released too (DoStop: g_jogs.clear, AllBtnUp)");

        // [6] fTeach closes while SystemStart: no StopAllMotor, only Teach's own jogs stop
        q = Req("uteach", "jogP", "B"); q.button = "btnJogP"; q.num["speed"] = 10;
        MotorAccessDispatch(q, 1, be);
        CHECK(MotorAccessJobs().jogsActive == 1, "WSLINK-B Teach jog before START");
        be.systemStart = true;
        sa0 = be.stopAll.size();
        n0 = (int)be.executed.size();
        MotorAccessPageClosed(be, "fTeach");
        CHECK(be.stopAll.size() == sa0 && MotorAccessJobs().jogsActive == 0 && (int)be.executed.size() == n0 + 1 && be.executed.back().kind == kCmdAxStop,
              "WSLINK-B fTeach closed while SystemStart -> no StopAllMotor (golden cannot close Teach in a run; S122 R82=A), Teach jog stopped");
        be.systemStart = false;

        // [6b] re-review #1: a Light Scale single home + Teach close -> the home is cancelled (no HomeFlag=1 from the stopped axis),
        //      Light Scale keeps waiting at case 1 (golden bSingleHome stays true). Setup as block (13) Light Scale.
        {
            be.table["MInArmX"] = Axis1203(0, 0, 0, 2); be.aliasOf[0] = "MInArmX"; be.golden[0] = g; be.golden[0].homeFlag = 0;
            be.driveKind = 1; be.homeFlags.clear(); be.lsEncoder = -895; be.lsEncFromMonitor = true;
            MotorAccessReq ls = Req("uMotorTest", "lightScale", 0); ls.button = "BitBtn2"; ls.kind = "motion";
            ls.num["axisItem"] = 1; ls.num["moveType"] = 0; ls.num["pitch"] = 300; ls.num["delayMs"] = 0;
            MotorAccessOutcome lo = MotorAccessDispatch(ls, 1, be);
            MotorAccessPollTick(be, true);                                      // Light Scale case 0 -> its single home
            CHECK(lo.ok && MotorAccessLightScale(500).homePending && MotorAccessJobs().homesActive == 1 && be.homeFlags.count(0) && be.homeFlags[0] == 0,
                  "WSLINK-B [6b] Light Scale started its single home (homePending, HomeFlag=0)");
            be.state[2] = 4; MotorAccessTick(be, 500, true);                   // HOMING
            MotorAccessPageClosed(be, "fTeach");
            CHECK(MotorAccessJobs().homesActive == 0 && MotorAccessLightScale(500).homePending,
                  "WSLINK-B [6b] Teach closed -> the Light Scale home job is cancelled, homePending kept (case 1 waits)");
            be.state[2] = 1;                                                    // the axis the stop halted now reads READY
            MotorAccessTick(be, 500, true); MotorAccessPollTick(be, true);
            const MotorLightScaleState st6 = MotorAccessLightScale(500);
            CHECK(be.homeFlags[0] == 0 && st6.homePending && st6.task == 1,
                  "WSLINK-B [6b] next beat: READY after HOMING is NOT read as a home (HomeFlag stays 0), Light Scale still waits at case 1");
            MotorAccessDispatch(ls, 1, be);                                     // BitBtn2 again toggles the timer off (golden)
        }

        // [7] other forms: nothing
        sa0 = be.stopAll.size(); n0 = (int)be.executed.size(); fc0 = be.formCloses;
        MotorAccessPageClosed(be, "fHome"); MotorAccessPageClosed(be, 0);
        CHECK(be.stopAll.size() == sa0 && (int)be.executed.size() == n0 && be.formCloses == fc0, "WSLINK-B fHome / null -> nothing (fHome: WebPageTable homeClose)");
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (16)
    //AI(W906-MT-SAVEMOT) 20260930: saveMotTable (Motor Test「回寫 Mot_Table」) through the dispatcher -- the glue: who may, where the
    //  file is, the ack, the in-memory callback. The writer's own rules and byte identity: tests/test_mt_savemot.cpp.
    printf("[16] AI(W906-MT-SAVEMOT): saveMotTable through the dispatcher (a temp copy, never a machine file)\n");
    {
        MotorAccessResetJobs();
        FakeBackend be;
        be.table["MInArmZA"] = Axis1203(3, 3, 0, 0);
        be.opened.assign(1, true);
        MotorGolden g;
        g.valid = true; g.enable = true; g.initSpeed = 100; g.jogHigh = 800; g.jogLow = 100; g.homeHigh = 10000; g.homeLow = 1000;
        g.softP = 999999; g.softN = -999999; g.accDb = 8000.0; g.decDb = 8000.0; g.range = 75;
        be.golden[3] = g; be.aliasOf[3] = "MInArmZA";
        const char* tmpEnv = std::getenv("TEMP");
        const std::string dir = std::string(tmpEnv ? tmpEnv : ".") + "\\ht9045_savemot_disp_" + std::to_string((long long)_getpid());
        _mkdir(dir.c_str());
        const std::string path = dir + "\\Mot_Table.csv";
        const std::string H = "Motorname,Alias,SoftLimitN,SoftLimitP,BoardID,Port,IP,Direction,GearRatio,HomeDirectior,HomeHighSpeed,HomeLowSpeed,"
                              "InitSpeed,JogHighSpeed,JogLowSpeed,Rate,Enable,ServoAlarmOn,Range,1P2P,SensorType,SimulateSpeed,CardModel,Acc,Dec,"
                              "EncodeType,PickLimit,LimitLogic,In1Logic";
        const std::string R0 = "M00,MInArmX,-999999,999999,0,0,0,0,1,0,20000,5000,500,10000,100,90,1,1,100,1,1,10000,PCI1203,100000,100000,2,,0,1";
        const std::string R3 = "M03,MInArmZA,-999999,999999,3,0,3,0,1,0,10000,1000,100,800,100,60,1,0,75,1,0,10000,PCI1203,8000,8000,0,,0,1";
        const std::string T = H + "\r\n" + R0 + "\r\n" + R3 + "\r\n";
        { std::ofstream f(path.c_str(), std::ios::binary | std::ios::trunc); f << T; }
        MotorAccessReq rq = Req("uMotorTest", "saveMotTable", 0);
        rq.button = "btnSaveMotTable";
        MotorAccessOutcome o = MotorAccessDispatch(rq, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "沒有選馬達"), "saveMotTable with no motor -> refused (golden ActiveIndex==-1)");
        rq.motors.push_back("MInArmZA");
        o = MotorAccessDispatch(rq, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "MotTablePath"), "no Mot_Table path from the backend -> refused");
        be.motTablePath = path;
        MotorAccessReq rt = rq; rt.source = "uteach";
        o = MotorAccessDispatch(rt, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "uMotorTest"), "saveMotTable from the teach page -> refused (a Motor Test action)");
        be.systemStart = true;
        o = MotorAccessDispatch(rq, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "SystemStart"), "while SystemStart -> refused (like every Motor Test button)");
        be.systemStart = false;
        o = MotorAccessDispatch(rq, 1, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"unchanged\"") && Has(o.ackJson, "\"backup\":null") &&
              Has(o.ackJson, "no change") && be.motSaved.empty() && ReadAll(path) == T,
              "values equal to the file -> ok, result unchanged, no backup, no memory update, file untouched");
        be.golden[3].jogHigh = 900; be.golden[3].accDb = 9000.0;
        o = MotorAccessDispatch(rq, 1, be);
        std::string bak, c0, o0, n0, c1, o1, n1, file;
        int nch = -1;
        {
            cJSON* j = cJSON_Parse(o.ackJson.c_str());
            const cJSON* b = j ? cJSON_GetObjectItemCaseSensitive(j, "backup") : 0;
            const cJSON* fl = j ? cJSON_GetObjectItemCaseSensitive(j, "file") : 0;
            const cJSON* ch = j ? cJSON_GetObjectItemCaseSensitive(j, "changes") : 0;
            if (cJSON_IsString(b)) bak = b->valuestring;
            if (cJSON_IsString(fl)) file = fl->valuestring;
            if (cJSON_IsArray(ch)) {
                nch = cJSON_GetArraySize(ch);
                for (int k = 0; k < nch && k < 2; ++k) {
                    const cJSON* e = cJSON_GetArrayItem(ch, k);
                    const cJSON* ec = cJSON_GetObjectItemCaseSensitive(e, "column");
                    const cJSON* eo = cJSON_GetObjectItemCaseSensitive(e, "old");
                    const cJSON* en = cJSON_GetObjectItemCaseSensitive(e, "new");
                    std::string& C = k ? c1 : c0; std::string& O = k ? o1 : o0; std::string& N = k ? n1 : n0;
                    if (cJSON_IsString(ec)) C = ec->valuestring;
                    if (cJSON_IsString(eo)) O = eo->valuestring;
                    if (cJSON_IsString(en)) N = en->valuestring;
                }
            }
            cJSON_Delete(j);
        }
        std::string R3n = R3;
        R3n.replace(R3n.find(",800,"), 5, ",900,");
        R3n.replace(R3n.find(",8000,8000,"), 11, ",9000,8000,");
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"saved\"") && file == path && nch == 2 &&
              c0 == "JogHighSpeed" && o0 == "800" && n0 == "900" && c1 == "Acc" && o1 == "8000" && n1 == "9000",
              "JogHighSpeed 800 -> 900 and Acc 8000 -> 9000: result saved, the ack names the file and each column old -> new");
        CHECK(!bak.empty() && bak.compare(0, path.size() + 5, path + ".bak_") == 0 && ReadAll(bak) == T, "the ack's backup = path.bak_<stamp>, byte-identical to the old file");
        CHECK(ReadAll(path) == H + "\r\n" + R0 + "\r\n" + R3n + "\r\n", "the file = the old bytes with those two cells replaced");
        CHECK(be.motSaved.size() == 1 && be.motSaved[0] == "MInArmZA|" + R3n + "|JogHighSpeed=900;Acc=9000;",
              "MotTableRowSaved: the Alias, the new row text (_CommaText) and only the changed cells");
        std::remove(bak.c_str()); std::remove(path.c_str()); _rmdir(dir.c_str());
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (SMHOME)
    //AI(W906-MT-SMHOME) 20261001: RULINGS_20261001 #8 -- a non-1203 axis homes the golden way: btnHomeClick :1152-1158 ->
    //  golden MainProc `ProcessSingleMotorHome(i) && MOT[i].HomeFlag==1` once per beat -> Timer1Timer :962-966
    //  SetSpeed(scrlbrMotorSpeed->Position); release :1168-1171 PCIL132_StopMotor. NOT COVERED: Light Scale's case 0 on a
    //  non-1203 row (LsStartGoldenHome) and the live backend's bodies (they call the golden functions).
    printf("[SMHOME] non-1203 single home (golden ProcessSingleMotorHome)\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 999999; g.softN = -999999; g.homeFlag = 1;
        FakeBackend be;
        be.table["MInArmPitch"] = AxisOther(2, "MN200", true);
        be.table["MInArmY"]     = AxisOther(3, "MN200", true);
        be.golden[2] = g; be.golden[3] = g;
        MotorAccessDispatch(Req("uMotorTest", "selectMotor", "MInArmPitch"), 1, be);
        MotorAccessReq sq = Req("uMotorTest", "setSpeed", "MInArmPitch"); sq.num["pct"] = 37; sq.flag["jog"] = true;
        CHECK(MotorAccessDispatch(sq, 2, be).ok, "speed bar 37 on the non-1203 axis (golden scrlbrMotorSpeed->Position)");
        be.smStepsLeft[2] = 2;                                                  // two passes still homing, then HomeFlag==1
        const std::size_t nStop0 = be.stopMotor.size(), nSpeed0 = be.speedCalls.size();
        MotorAccessOutcome o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmPitch", true), 3, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"homing\"") && Has(o.ackJson, "\"layer\":\"MOT\"") &&
              Has(o.ackJson, "\"homeActive\":true"), "HOME down on MN200 -> accepted on the golden path (layer MOT), homeActive");
        CHECK(be.smBegin.size() == 1 && be.smBegin[0] == 2 && MotorAccessJobs().homesActive == 1,
              "golden InitProcessSingleMotorTask(2) + HomeFlag=0 once, one HOME job");
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmPitch", true), 4, be);
        CHECK(o.ok && be.smBegin.size() == 1 && Has(o.ackJson, "\"homeActive\":true"), "HOME down again while homing -> not re-initialised");
        MotorAccessTick(be, 500); MotorAccessTick(be, 500);
        CHECK(be.smStep.size() == 2 && MotorAccessJobs().homesActive == 1 && be.speedCalls.size() == nSpeed0,
              "two beats = two ProcessSingleMotorHome passes, still homing, no SetSpeed yet");
        MotorAccessTick(be, 500);
        CHECK(be.smStep.size() == 3 && MotorAccessJobs().homesActive == 0,
              "third pass: ProcessSingleMotorHome && HomeFlag==1 -> golden bSingleHome=false, the job ends");
        CHECK(be.speedCalls.size() == nSpeed0 + 1 && be.speedCalls.back().mi == 2 && be.speedCalls.back().pct == 37 && !be.speedCalls.back().jog,
              "golden Timer1Timer: MOT[2].SetSpeed(scrlbrMotorSpeed->Position = 37), PTP family (jog=false)");
        CHECK(be.stopMotor.size() == nStop0, "a home that ends by itself sends no stop");

        be.smStepsLeft[3] = -1;                                                 // never HomeFlag==1 (golden: its own alarm already shown)
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmY", true), 5, be);
        MotorAccessTick(be, 500); MotorAccessTick(be, 500);
        CHECK(o.ok && MotorAccessJobs().homesActive == 1, "no HomeFlag==1 -> golden bSingleHome stays true, the job stays");
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmY", false), 6, be);
        CHECK(o.ok && Has(o.ackJson, "\"result\":\"homeStopped\"") && MotorAccessJobs().homesActive == 0 &&
              be.stopMotor.size() == nStop0 + 1 && be.stopMotor.back() == 3,
              "HOME up -> golden bSingleHome=false + MOT[3].PCIL132_StopMotor, exactly once");
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmY", true), 7, be);
        CHECK(o.ok && MotorAccessJobs().homesActive == 1, "HOME down again -> a new golden single home");
        MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmY"), 8, be);
        CHECK(MotorAccessJobs().homesActive == 0, "STOP (golden AllBtnUp + bSingleHome=false) cancels the golden home too");
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (ZALLUP)
    //AI(W906-TEACH-ZALLUP) 20261001: Teach buttons that had no handler -- golden uteach btnInZAllUpClick :4466 / btnOutZAllUpClick :4480
    //  (every Z of the arm: InitProcessSingleMotorTask, stepped by Timer1Timer's DoZHome; no interlock in golden) and btnZ1ServoClick
    //  :4253 / btnZ2ServoClick :4270 / btnArm1YServoClick :2919 (Gali "MO"/"SH" toggled by a static, fAllMotorHome=false).
    printf("[ZALLUP] Teach In/Out Z All Up + the Index tab Servo buttons (golden uteach)\n");
    {
        MotorAccessResetJobs();
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 999999; g.softN = -999999; g.homeFlag = 1;
        g.jogHigh = 10000; g.initSpeed = 500; g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100;
        MotorGolden gz = g;  gz.armZ = true;
        MotorGolden g0 = g;  g0.enable = false; g0.selectable = false;           // Enable=0, real-machine build (lM00Click :740-743)
        FakeBackend be;
        be.reg = BuildTeachRegistry(std::set<std::string>());
        const int zA = Add1203(be, "MInArmZA", 1, gz);                          // the HT9050 picture: In Arm ZA is the one live 1203 Z
        const int zC = Add1203(be, "MInArmZC", 2, gz);  be.table["MInArmZC"].tableEnable = false;   // a PCI1203 row with Enable=0
        const int zB = TestMot("MInArmZB");                                     // MN200, Enable=0 (Mot_Table on this machine)
        be.table["MInArmZB"] = AxisOther(zB, "MN200", true); be.table["MInArmZB"].tableEnable = false; be.golden[zB] = g0; be.aliasOf[zB] = "MInArmZB";
        const int zG = TestMot("MInArmZG");                                     // golden InArmZIndex has it, this Mot_Table does not
        const int oA = Add1203(be, "MOutArmZA", 3, gz);
        std::vector<int> inZ;  inZ.push_back(zA); inZ.push_back(zC); inZ.push_back(zB); inZ.push_back(zG);
        be.fixedMots["inZ"] = inZ;
        be.fixedMots["outZ"] = std::vector<int>(1, oA);
        be.cmdPos[1] = 500.0;
        MotorAccessReq zin = Req("uteach", "teachZAllUp", "MInArmX"); zin.button = "btnInZAllUp"; zin.kind = "motion";   // motors = the page's selection (ignored, golden)
        MotorAccessReq zout = zin; zout.button = "btnOutZAllUp";

        // ---- happy path: only the live 1203 Z homes; Enable=0 rows -> golden case 200 HomeFlag=1; no row -> skipped
        std::size_t x0 = be.executed.size();
        MotorAccessOutcome o = MotorAccessDispatch(zin, 41, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"homing\"") && Has(o.ackJson, "\"homeActive\":true") &&
              Has(o.ackJson, "\"caption\":\"Homeing...\"") && Has(o.ackJson, "\"arm\":\"in\"") && Has(o.ackJson, "\"partial\":false"),
              "In Z All Up -> ok homing, caption Homeing... (golden DoZHome :4527)");
        CHECK(CountKind(be, x0, kCmdAxHome) == 1 && be.executed.back().kind == kCmdAxHome && be.executed.back().axis == 1,
              "exactly one DS402 home, on MInArmZA's 1203 axis (golden InitProcessSingleMotorTask -> ProcessSingleMotorHome)");
        CHECK(be.homeFlags.count(zB) && be.homeFlags[zB] == 1 && be.homeFlags.count(zC) && be.homeFlags[zC] == 1 && be.homeFlags[zA] == 0,
              "Enable=0 (MN200 ZB, PCI1203 ZC) -> golden ProcessSingleMotorHome case 200: HomeFlag=1, nothing sent; ZA HomeFlag=0 at the start");
        CHECK(Has(o.ackJson, "MInArmZB") && Has(o.ackJson, "MInArmZC") && Has(o.ackJson, "MOT[" + std::to_string(zG) + "]") && be.smBegin.empty(),
              "the ack names the Enable=0 rows and the Z this Mot_Table does not have; no golden single home for a 1203 row");
        CHECK(be.teachCanMoveCalls.empty() && be.Calls() == (int)be.executed.size(),
              "golden btnInZAllUpClick has no CheckCanMove / IsCanQuickJogMove -> not asked; no golden stop / servo call");
        CHECK(MotorAccessJobs().homesActive == 1 && MotorAccessJobs().homeMotors.size() == 1 && MotorAccessJobs().homeMotors[0] == "MInArmZA",
              "one HOME job (MInArmZA) -- what the runtime's motion.homeJob shows the page");
        be.state[1] = 4; MotorAccessTick(be, 500);
        be.state[1] = 1; be.cmdPos[1] = 0.0; MotorAccessTick(be, 500);
        MotorAccessTick(be, 500);
        CHECK(be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().axis == 1 && be.executed.back().value == 20.0 && be.homeFlags[zA] == 0,
              "after the home: golden case 500 MotorMove(ZSafePos 20), HomeFlag still 0");
        const std::size_t sp0 = be.speedCalls.size();
        be.cmdPos[1] = 20.0; MotorAccessTick(be, 500);
        CHECK(be.homeFlags[zA] == 1 && MotorAccessJobs().homesActive == 0 && be.speedCalls.size() == sp0,
              "Z at ZSafePos -> HomeFlag=1, the job ends (golden DoZHome -> caption back to In Z All Up), no SetSpeed");

        // ---- the Z is at home already: golden has no shortcut, it homes again; while it homes, no second command
        x0 = be.executed.size();
        o = MotorAccessDispatch(zin, 42, be);
        CHECK(o.ok && CountKind(be, x0, kCmdAxHome) == 1 && be.homeFlags[zA] == 0,
              "pressed with ZA already homed (HomeFlag=1) -> homed again (golden InitProcessSingleMotorTask, no 'already at home' test), HomeFlag=0");
        x0 = be.executed.size();
        o = MotorAccessDispatch(zin, 43, be);
        CHECK(o.ok && Has(o.ackJson, "\"already\":[\"MInArmZA\"]") && be.executed.size() == x0 && MotorAccessJobs().homesActive == 1,
              "pressed again while ZA homes -> ok, 'already', no second command");
        MotorAccessDispatch(Req("uteach", "stop", 0), 44, be);
        CHECK(MotorAccessJobs().homesActive == 0 && be.homeFlags[zA] == 0, "STOP (golden btnStopClick: StopAllMotor + AllBtnUp) cancels it, HomeFlag stays 0");

        // ---- Out Z All Up: OutArmSuck's Z (golden :4480-4489)
        x0 = be.executed.size();
        o = MotorAccessDispatch(zout, 45, be);
        CHECK(o.ok && Has(o.ackJson, "\"arm\":\"out\"") && CountKind(be, x0, kCmdAxHome) == 1 && be.executed.back().axis == 3 && be.homeFlags[oA] == 0,
              "Out Z All Up -> MOutArmZA's 1203 axis homes (golden btnOutZAllUpClick)");
        MotorAccessDispatch(Req("uteach", "stop", 0), 46, be);

        // ---- the interlocks inside the home (golden TMyMotor::Home's safe door): nothing started -> refused with the reason
        be.doorOpen.insert(zA);
        x0 = be.executed.size();
        o = MotorAccessDispatch(zin, 47, be);
        CHECK(!o.ok && Has(o.ackJson, "沒有一個 Z 開始歸零") && Has(o.ackJson, "安全門") && CountKind(be, x0, kCmdAxHome) == 0 && be.homeFlags[zA] == 0 &&
              MotorAccessJobs().homesActive == 0,
              "safe door open on ZA (golden TMyMotor::Home CheckIsSafeDoorOpen) -> nothing homes -> refused, says why; HomeFlag=0 (golden :1158)");
        const int zE = Add1203(be, "MInArmZE", 4, gz);                          // a second live In Z: the other one still homes
        inZ.push_back(zE); be.fixedMots["inZ"] = inZ;
        x0 = be.executed.size();
        o = MotorAccessDispatch(zin, 48, be);
        CHECK(o.ok && Has(o.ackJson, "\"partial\":true") && Has(o.ackJson, "安全門") && CountKind(be, x0, kCmdAxHome) == 1 && be.executed.back().axis == 4,
              "two live Z, one behind the open door -> the other homes, ok partial, the refusal is in the ack");
        MotorAccessDispatch(Req("uteach", "stop", 0), 49, be);
        be.doorOpen.clear();
        be.ready = false; be.notReadyWhy = "fake: no 1203";
        o = MotorAccessDispatch(zin, 50, be);
        CHECK(!o.ok && Has(o.ackJson, "1203 控制層不可用") && MotorAccessJobs().homesActive == 0, "1203 control not ready -> refused, nothing started");
        be.ready = true;

        // ---- a non-1203 Z with Enable=1: golden InitProcessSingleMotorTask + ProcessSingleMotorHome, no SetSpeed at the end (DoZHome)
        {
            FakeBackend gb;
            const int zN = TestMot("MInArmZH");
            gb.table["MInArmZH"] = AxisOther(zN, "MN200", true); gb.golden[zN] = gz; gb.aliasOf[zN] = "MInArmZH";
            gb.fixedMots["inZ"] = std::vector<int>(1, zN);
            gb.smStepsLeft[zN] = 1;
            o = MotorAccessDispatch(zin, 51, gb);
            CHECK(o.ok && gb.smBegin.size() == 1 && gb.smBegin[0] == zN && MotorAccessJobs().homesActive == 1,
                  "non-1203 Z (Enable=1) -> golden InitProcessSingleMotorTask (DoHomeGolden), one HOME job");
            MotorAccessTick(gb, 500); MotorAccessTick(gb, 500);
            CHECK(MotorAccessJobs().homesActive == 0 && gb.homeFlags[zN] == 1 && gb.speedCalls.empty(),
                  "ProcessSingleMotorHome && HomeFlag==1 -> the job ends; no SetSpeed (golden DoZHome sets none; Motor Test's btnHome would)");
            MotorAccessResetJobs();
        }

        // ---- refused before anything: SystemStart, hand teach, Motor Test source, unknown button, a backend without the table
        int c0 = be.Calls();
        be.systemStart = true;
        o = MotorAccessDispatch(zin, 52, be);
        CHECK(!o.ok && Has(o.ackJson, "SystemStart") && be.Calls() == c0 && MotorAccessJobs().homesActive == 0,
              "SystemStart -> refused (golden: Teach cannot be open during a run, main.cpp:27827), nothing sent");
        be.systemStart = false;
        be.golden[zA].homeFlag = 1;                                             // SetButton140 wants a homed axis (golden :3385)
        o = MotorAccessDispatch(TeachReq("teachSet", "SetButton030", true, true), 53, be);   // hand teach on MInArmZA (golden fTeachShow modal)
        CHECK(o.ok && MotorAccessJobs().teachActive, "setup: hand teach open on MInArmZA");
        c0 = be.Calls();
        o = MotorAccessDispatch(zin, 54, be);
        CHECK(!o.ok && Has(o.ackJson, "手動教導中") && be.Calls() == c0 && MotorAccessJobs().homesActive == 0,
              "hand teach open -> In Z All Up refused (golden fTeachShow is ShowModal), nothing sent");
        MotorAccessReq z1h = Req("uteach", "teachIndexServo", 0); z1h.button = "btnZ1Servo";
        o = MotorAccessDispatch(z1h, 55, be);
        CHECK(!o.ok && Has(o.ackJson, "手動教導中") && be.Calls() == c0, "hand teach open -> Index servo refused too");
        MotorAccessReq hend = TeachReq("teachSet", "SetButton030", true, false); hend.flag["accept"] = false;
        CHECK(MotorAccessDispatch(hend, 56, be).ok && !MotorAccessJobs().teachActive, "setup: hand teach cancelled");
        MotorAccessReq zmt = zin; zmt.source = "uMotorTest";
        CHECK(!MotorAccessDispatch(zmt, 57, be).ok && Has(MotorAccessDispatch(zmt, 57, be).ackJson, "MotorTest 沒有"), "Motor Test source -> refused (a Teach button)");
        MotorAccessReq zbad = zin; zbad.button = "btnSortZAllUp";
        CHECK(!MotorAccessDispatch(zbad, 58, be).ok, "another button name -> refused (btnSortZAllUp is not wired)");
        FakeBackend nb;
        nb.table["MInArmZA"] = Axis1203(1, 3, 0, 1);
        o = MotorAccessDispatch(zin, 59, nb);
        CHECK(!o.ok && Has(o.ackJson, "GoldenTeachFixedMotors") && nb.Calls() == 0, "a backend without golden's fixed-motor table -> refused honestly, nothing sent");
        MotorAccessResetJobs();

        // ---- Index Servo: btnZ1Servo on the live 1203 MTestZ1
        FakeBackend sb;
        MotorGolden gi = g;  gi.indexZ = true; gi.indexMotor = true;
        const int z1 = Add1203(sb, "MTestZ1", 2, gi);
        const int z2 = TestMot("MTestZ2");
        sb.table["MTestZ2"] = AxisOther(z2, "SMC", true); sb.table["MTestZ2"].tableEnable = false; sb.golden[z2] = g0; sb.aliasOf[z2] = "MTestZ2";
        const int y1 = TestMot("MTestY1");
        sb.table["MTestY1"] = AxisOther(y1, "SMC", true); sb.table["MTestY1"].tableEnable = false; sb.golden[y1] = g0; sb.aliasOf[y1] = "MTestY1";
        sb.fixedMots["z1"] = std::vector<int>(1, z1);
        sb.fixedMots["z2"] = std::vector<int>(1, z2);
        sb.fixedMots["arm1y"] = std::vector<int>(1, y1);                         // a 3-axis Index (no MTestY2)
        sb.brakeAlias = "MTestZ1"; sb.brakeReleased = true;
        sb.svon[2] = true;
        MotorAccessReq z1s = Req("uteach", "teachIndexServo", "MInArmX"); z1s.button = "btnZ1Servo";   // motors = the page's selection (ignored)
        const int ca0 = sb.clearAllHome;
        o = MotorAccessDispatch(z1s, 61, sb);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"servoOff\"") && Has(o.ackJson, "\"caption\":\"Server OFF\"") &&
              Has(o.ackJson, "\"basis\":\"toggle-actual\"") && Has(o.ackJson, "\"captionButtons\":[\"btnZ1Servo\"]"),
              "btnZ1Servo with MTestZ1's servo ON -> Servo OFF (golden \"MOY\"), caption Server OFF");
        CHECK(sb.executed.back().kind == kCmdAxSvOn && sb.executed.back().axis == 2 && sb.executed.back().value == 0.0 && sb.brakeHoldAt >= 0 &&
              !sb.sleeps.empty() && sb.sleeps.back() == 100,
              "the SvOn(0) goes to MTestZ1's 1203 axis, after its brake is held (BRAKE-AXIS, as DoServo)");
        CHECK(sb.clearAllHome > ca0, "golden fAllMotorHome=false (:4267)");
        sb.svon[2] = false;
        o = MotorAccessDispatch(z1s, 62, sb);
        CHECK(o.ok && Has(o.ackJson, "\"caption\":\"Server ON\"") && sb.executed.back().kind == kCmdAxSvOn && sb.executed.back().value == 1.0,
              "servo OFF -> the next press turns it ON (golden \"SHY\"), caption Server ON");
        sb.svon.erase(2);                                                       // state unreadable: golden's own static decides
        o = MotorAccessDispatch(z1s, 63, sb);
        CHECK(o.ok && Has(o.ackJson, "\"basis\":\"golden-static\"") && sb.executed.back().value == 0.0,
              "state unknown -> golden static: after an ON the next is OFF (bflag=!bflag)");
        o = MotorAccessDispatch(z1s, 64, sb);
        CHECK(o.ok && Has(o.ackJson, "\"basis\":\"golden-static\"") && sb.executed.back().value == 1.0, "... and then ON again");
        int s0 = sb.Calls();
        MotorAccessReq z2s = z1s; z2s.button = "btnZ2Servo";
        o = MotorAccessDispatch(z2s, 65, sb);
        CHECK(!o.ok && Has(o.ackJson, "Enable=0") && sb.Calls() == s0, "btnZ2Servo: MTestZ2 is SMC Enable=0 here -> the existing Enable=0 refusal, nothing sent");
        MotorAccessReq y1s = z1s; y1s.button = "btnArm1YServo";
        o = MotorAccessDispatch(y1s, 66, sb);
        MotorAccessReq y2s = z1s; y2s.button = "btnArm2YServo";
        const MotorAccessOutcome o2 = MotorAccessDispatch(y2s, 67, sb);
        CHECK(!o.ok && Has(o.ackJson, "Enable=0") && !o2.ok && Has(o2.ackJson, "Enable=0") && Has(o2.ackJson, "btnArm1YServoClick") && sb.Calls() == s0,
              "btnArm1YServo / btnArm2YServo (one golden handler): MTestY1 Enable=0 -> refused, nothing sent");
        // a 4-axis Index with both Y on 1203: one target for both, both captions
        const int y1b = Add1203(sb, "MTestY1", 5, gi);
        const int y2b = Add1203(sb, "MTestY2", 6, gi);
        std::vector<int> a4; a4.push_back(y1b); a4.push_back(y2b);
        sb.fixedMots["arm1y"] = a4;
        sb.svon[5] = true; sb.svon[6] = true;
        x0 = sb.executed.size();
        o = MotorAccessDispatch(y2s, 68, sb);
        CHECK(o.ok && CountKind(sb, x0, kCmdAxSvOn) == 2 && sb.executed[sb.executed.size() - 2].axis == 5 && sb.executed.back().axis == 6 &&
              sb.executed.back().value == 0.0 && Has(o.ackJson, "\"captionButtons\":[\"btnArm1YServo\",\"btnArm2YServo\"]") && Has(o.ackJson, "\"partial\":false"),
              "IndexArm_4_Axis: MTestY1 then MTestY2 (golden :2924-2926), the same target, both buttons' captions");
        // refused before anything
        s0 = sb.Calls();
        sb.systemStart = true;
        o = MotorAccessDispatch(z1s, 69, sb);
        CHECK(!o.ok && Has(o.ackJson, "SystemStart") && sb.Calls() == s0, "SystemStart -> Index servo refused, nothing sent");
        sb.systemStart = false;
        MotorAccessReq zms = z1s; zms.source = "uMotorTest";
        CHECK(!MotorAccessDispatch(zms, 70, sb).ok && sb.Calls() == s0, "Motor Test source -> refused");
        MotorAccessReq zub = z1s; zub.button = "btnServo";
        CHECK(!MotorAccessDispatch(zub, 71, sb).ok && sb.Calls() == s0, "a button that is not one of the four -> refused");
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (ARMCELL)
    //AI(W906-ARMCELL) 20261002: the Teach page "Arm Cell" tab -- motor.access moveToTrayCell (RULINGS_20261002 #18; the job rules at
    //  the EOF of WebMotorAccess.cpp). golden has no such button (HT160 Sort Arm "Pick / Place Test" is the model). The plan is canned
    //  here (its formulas: ctest ArmCellPlan); this section locks the job: S0 refuses before anything moves, S1 every Enable Z to the
    //  runtime ZSafePos, S2 the Z-at-home check + golden CheckCanMove / IsCanQuickJogMove once, S3 X and Y in the same beat, S4 only
    //  the chosen Z (D2 for a shuttle), "arrived" only at the very end, every cancel trigger stops the job's own axes (D3), the
    //  gates while it runs, and the runtime block (catalog + job, Q3 zDownDefault).
    printf("[ARMCELL] Teach Arm Cell: moveToTrayCell (RULINGS_20261002 #18)\n");
    {
        g_W906OrgActiveLow = []() { return true; };                             // HT9050: the 1203 ORG bit low = at home (EastSun 20261001)
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 999999; g.softN = -999999; g.homeFlag = 1;
        g.jogHigh = 10000; g.initSpeed = 500; g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100;
        MotorGolden gz = g;  gz.armZ = true;
        gz.sensorType = true;                                                   // Mot_Table SensorType=1 (machines/HT9050 with MR !119): ORG low = at home under the
                                                                                //   1001 HT9050 rule AND !119's per-axis SensorType rule -- S2 asks MotorAccessTeachHomeLed
        auto Ax = [](const char* alias) { ArmCellAxis a; a.alias = alias; a.present = true; a.is1203 = true; a.enable = true; return a; };
        // ctest ArmCellPlan [3]'s FT005054_9050 Loader (3,8): X 20075, Y -78190, pick Z -2015; ZSafePos 50; In Arm ZA only
        auto Plan = [&Ax](const char* area, bool zAllowed) {
            ArmCellPlan p;
            p.arm = "in"; p.area = area; p.label = area; p.zKind = "pick"; p.nozzle = "MInArmZA";
            p.armIndex = 0; p.col = 2; p.row = 7; p.nr = 0; p.nc = 0;
            p.x = Ax("MInArmX"); p.y = Ax("MInArmY"); p.z = Ax("MInArmZA"); p.zLift.push_back(p.z);
            p.xTarget = 20075; p.yTarget = -78190; p.zTarget = -2015; p.zSafe = 50;
            p.zDownAllowed = zAllowed;
            p.zDownWhy = zAllowed ? "" : "Bin Box：golden Prod.ZPlace[eBulkBox] 從來沒被寫過";
            p.d1Note = "USE_PICKER_COUNT=1 D1 note"; p.notes.push_back(p.d1Note);
            return p;
        };
        // a fresh backend: In Arm X / Y / ZA and MInShutte1 on the 1203 (the HT9050 picture), ZA's ORG lit, the job state reset
        auto Fresh = [&](FakeBackend& b) {
            MotorAccessResetJobs();
            Add1203(b, "MInArmX", 1, g); Add1203(b, "MInArmY", 2, g); Add1203(b, "MInArmZA", 3, gz); Add1203(b, "MInShutte1", 4, g);
            b.motionIO[3] = 0;
            b.motionIO[1] = 0; b.motionIO[2] = 0; b.motionIO[4] = 0;              //AI(W906-ARMCELL2) 20261002: the lamps S0 / the beat read (no sample = fail closed)
            b.cellHasPlan = true; b.cellPlan = Plan("Loader", true);
        };
        auto CellReq = [](bool zDown) {
            MotorAccessReq r = Req("uteach", "moveToTrayCell", 0);
            r.button = "armCellGo"; r.kind = "motion";
            r.str["arm"] = "in"; r.str["nozzle"] = "MInArmZA"; r.str["area"] = "Loader";
            r.num["col"] = 2; r.num["row"] = 7; r.flag["zDown"] = zDown;
            return r;
        };
        auto Cell = []() { return MotorAccessArmCell(); };
        auto Arrive = [](FakeBackend& b, int slot, int pos) { b.cmdPos[slot] = pos; b.actPos[slot] = pos; b.state[slot] = 1; };
        auto Moving = [](FakeBackend& b, int slot) { b.state[slot] = 5; };      // STA_AX_PTP_MOT
        auto MovesTo = [](const FakeBackend& b, std::size_t from, int slot, double v) {
            int n = 0;
            for (std::size_t i = from; i < b.executed.size(); ++i)
                if (b.executed[i].kind == kCmdAxMoveAbs && b.executed[i].axis == slot && b.executed[i].value == v) ++n;
            return n;
        };
        auto StopsOn = [](const FakeBackend& b, std::size_t from, int slot) {
            int n = 0;
            for (std::size_t i = from; i < b.executed.size(); ++i) if (b.executed[i].kind == kCmdAxStop && b.executed[i].axis == slot) ++n;
            return n;
        };
        // dispatch + drive to S3 with X and Y commanded and moving (true = got there)
        auto ToS3 = [&](FakeBackend& b, const MotorAccessReq& r) {
            if (!MotorAccessDispatch(r, 900, b).ok) return false;
            MotorAccessTick(b, 500);                                            // S1: ZA commanded to ZSafePos
            Arrive(b, 3, 50); MotorAccessTick(b, 500);                          // S1 done -> S2
            MotorAccessTick(b, 500);                                            // S2 -> S3
            MotorAccessTick(b, 500);                                            // S3: X and Y commanded
            Moving(b, 1); Moving(b, 2);
            return Cell().active && Cell().step == 3;
        };
        const int ix = TestMot("MInArmX"), iy = TestMot("MInArmY"), iz = TestMot("MInArmZA"), ish = TestMot("MInShutte1");

        // ---- the whole job, zDown ticked
        FakeBackend cb;
        Fresh(cb);
        CHECK(std::string(MotorAccessActionStatus("moveToTrayCell")) == "live", "moveToTrayCell is a live kActions row");
        const int c0 = cb.Calls();
        MotorAccessOutcome o = MotorAccessDispatch(CellReq(true), 81, cb);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"cellMoving\"") && Has(o.ackJson, "\"cellActive\":true") &&
              Has(o.ackJson, "\"cellSeq\":1") && Has(o.ackJson, "\"step\":1") && Has(o.ackJson, "\"target\":20075") &&
              Has(o.ackJson, "\"target\":-78190") && Has(o.ackJson, "\"target\":-2015") && Has(o.ackJson, "\"zSafe\":50") &&
              Has(o.ackJson, "\"note\":\"USE_PICKER_COUNT=1 D1 note\"") && Has(o.ackJson, "S4 Z"),
              "accepted: ok cellMoving (NOT arrived), cellSeq 1, the plan's X / Y / Z targets, ZSafePos, the D1 note, the steps S1..S5");
        CHECK(cb.cellReqs.size() == 1 && cb.cellReqs[0].arm == "in" && cb.cellReqs[0].area == "Loader" && cb.cellReqs[0].nozzle == "MInArmZA" &&
              cb.cellReqs[0].col == 2 && cb.cellReqs[0].row == 7 && cb.cellReqs[0].zDown,
              "the backend's plan gets the request as sent (arm, area, nozzle, 0-based col / row, zDown)");
        CHECK(cb.Calls() == c0 && Cell().active && Cell().step == 1 && Cell().result == "running" && Cell().jobId == 1,
              "S0 sends nothing; the job waits for the beat at S1");
        CHECK(MotorAccessJobs().cellActive && MotorAccessJobs().cellMotors.size() == 3 && MotorAccessJobs().cellMotors[0] == "MInArmX" &&
              MotorAccessJobs().cellMotors[1] == "MInArmY" && MotorAccessJobs().cellMotors[2] == "MInArmZA",
              "MotorAccessJobs: cellActive, its axes X, Y, ZA");
        std::string lw, sw;
        CHECK(MotorAccessMovingOf(cb, ix) == 1 && MotorAccessMovingOf(cb, iy) == 1 && MotorAccessMovingOf(cb, iz) == 1 &&
              MotorAccessMovingOf(cb, ish) == 0 && MotorAccessAxisLock(cb, iy, lw) == 1 && lw == "ARM CELL",
              "VerifyMotorAction: its axes read moving from S0 on (between steps too), the shuttle does not; AxisLock says ARM CELL");
        CHECK(MotorAccessStartBlocked(sw) && Has(sw, "Arm Cell"), "START is refused while it runs (MotorAccessStartBlocked)");  { std::string hw; bool lg = false; CHECK(MotorAccessHomeBlocked("S2F42", hw, lg) && lg && hw.compare(0, std::string("HOME 拒絕：教導頁 Arm Cell 進行中").size(), "HOME 拒絕：教導頁 Arm Cell 進行中") == 0 && !Has(hw, "手動教導"), "HOMEBLOCK (U14 item 3 / R175 C1): TfMain::Home (SECS RCMD HOME, panel HOME, OLP, ESD decay) is refused while the Arm Cell job runs, with the Arm Cell HOME text"); }   //AI(W906-HOMEBLOCK) 20261003: NB2-1, same line
        std::size_t x0 = cb.executed.size();
        MotorAccessTick(cb, 500);
        CHECK(CountKind(cb, x0, kCmdAxMoveAbs) == 1 && MovesTo(cb, x0, 3, 50.0) == 1 && LastRunSpeed(cb) == MotorSpeedFromPct(20, gz).velHigh,
              "S1: only ZA is commanded -- to the runtime ZSafePos 50, at 20 %");
        Moving(cb, 3); MotorAccessTick(cb, 500);
        CHECK(Cell().step == 1, "S1: ZA moving -> still S1");
        Arrive(cb, 3, 50); MotorAccessTick(cb, 500);
        CHECK(Cell().step == 2 && cb.teachCanMoveCalls.empty(), "S1: ZA READY at ZSafePos -> S2 (verified on the next beat, a newer sample)");
        x0 = cb.executed.size();
        MotorAccessTick(cb, 500);
        CHECK(Cell().step == 3 && cb.teachCanMoveCalls.size() == 2 && cb.teachCanMoveCalls[0] == ix && cb.teachCanMoveCalls[1] == iy &&
              Cell().activeMotor == "MInArmY" && cb.executed.size() == x0,
              "S2: ZA's ORG lit; golden CheckCanMove / IsCanQuickJogMove on X then Y (ActiveMotorIndex ends on Y); nothing sent");
        x0 = cb.executed.size();
        MotorAccessTick(cb, 500);
        CHECK(CountKind(cb, x0, kCmdAxMoveAbs) == 2 && MovesTo(cb, x0, 1, 20075.0) == 1 && MovesTo(cb, x0, 2, -78190.0) == 1 &&
              LastRunSpeed(cb) == MotorSpeedFromPct(20, g).velHigh,
              "S3: X and Y commanded in the same beat, 20 % (GoButton020Click)");
        Moving(cb, 1); Moving(cb, 2); MotorAccessTick(cb, 500);
        CHECK(Cell().step == 3, "S3: both moving -> still S3");
        Arrive(cb, 1, 20075); MotorAccessTick(cb, 500);
        CHECK(Cell().step == 3, "S3: X there, Y not -> still S3");
        Arrive(cb, 2, -78190); x0 = cb.executed.size(); MotorAccessTick(cb, 500);
        CHECK(Cell().step == 4 && cb.executed.size() == x0, "S3: both READY at their targets -> S4 (the Z goes down on the next beat)");
        x0 = cb.executed.size();
        MotorAccessTick(cb, 500);
        CHECK(CountKind(cb, x0, kCmdAxMoveAbs) == 1 && MovesTo(cb, x0, 3, -2015.0) == 1 && LastRunSpeed(cb) == MotorSpeedFromPct(1, gz).velHigh,
              "S4: only the chosen Z (ZA) to its pick Z -2015, at 1 % (GoButton140Click SetSpeed(1))");
        Moving(cb, 3); MotorAccessTick(cb, 500);
        CHECK(Cell().active && Cell().result == "running", "S4: Z moving -> still running (no early 'arrived')");
        Arrive(cb, 3, -2015); cb.actPos[1] = 20074; x0 = cb.executed.size();
        MotorAccessTick(cb, 500);
        {
            const MotorAccessArmCellState st = Cell();
            CHECK(!st.active && st.result == "arrived" && st.step == 5 && st.hasFinal && st.finalKnown[0] && st.finalCmd[0] == 20075 &&
                  st.finalAct[0] == 20074 && st.finalCmd[1] == -78190 && st.finalKnown[2] && st.finalCmd[2] == -2015 && Has(st.why, "到位"),
                  "S5: arrived, with cmdPos / actPos of X, Y and the chosen Z");
        }
        CHECK(cb.executed.size() == x0 && !MotorAccessJobs().cellActive && MotorAccessMovingOf(cb, ix) == 0 && !MotorAccessStartBlocked(sw),
              "arrival sends nothing (no stop, the Z is not lifted); the axes and START are free again");  { std::string hw; bool lg = true; CHECK(!MotorAccessHomeBlocked("S2F42", hw, lg) && !lg, "HOMEBLOCK: the job is over -> TfMain::Home allowed again"); }   //AI(W906-HOMEBLOCK) 20261003: NB2-1, same line
        CHECK(cb.teachCanMoveCalls.size() == 2, "golden CheckCanMove asked once per job (S3 / S4 do not ask again)");
        {
            const std::string js = MotorAccessArmCellJson(cb);
            cJSON* jp = cJSON_Parse(js.c_str());
            CHECK(jp && cJSON_IsObject(jp) && Has(js, "\"catalog\":{\"ok\":false") && Has(js, "GoldenArmCellCatalog") &&
                  Has(js, "\"result\":\"arrived\"") && Has(js, "\"jobId\":1") && Has(js, "\"final\":{\"x\":{\"cmdPos\":20075,\"actPos\":20074}") &&
                  Has(js, "\"activeMotor\":\"MInArmY\"") && Has(js, "\"motors\":[\"MInArmX\",\"MInArmY\",\"MInArmZA\"]") && Has(js, "\"z\":-2015"),
                  "runtime armCell block: JSON; no catalog on this backend (ok false + why); the job: arrived, jobId, final, activeMotor, motors");
            cJSON_Delete(jp);
        }

        // ---- zDown not ticked: arrived after S3, the Z stays at ZSafePos
        {
            FakeBackend b;
            Fresh(b);
            const MotorAccessOutcome ob = MotorAccessDispatch(CellReq(false), 82, b);
            CHECK(ob.ok && Has(ob.ackJson, "\"zDown\":false") && !Has(ob.ackJson, "S4") && Has(ob.ackJson, "S5"),
                  "zDown off: accepted, the ack's steps skip S4");
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            const std::size_t y0 = b.executed.size();
            MotorAccessTick(b, 500);
            Arrive(b, 1, 20075); Arrive(b, 2, -78190);
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "arrived" && Cell().step == 5 && MovesTo(b, y0, 3, -2015.0) == 0 && Has(Cell().why, "ZSafePos 50"),
                  "zDown off: arrived after S3; the Z is never sent down");
        }

        // ---- S0: refused before anything moves (no job, nothing sent)
        {
            FakeBackend b;
            Fresh(b);
            int k0 = b.Calls();
            b.golden[iz].homeFlag = 0;
            MotorAccessOutcome r = MotorAccessDispatch(CellReq(false), 83, b);
            CHECK(!r.ok && Has(r.ackJson, "MInArmZA") && Has(r.ackJson, "HomeFlag=0") && b.Calls() == k0 && !Cell().active && Cell().jobId == 0,
                  "S0: a Z not homed (HomeFlag 0) -> refused, names it, nothing sent, no job");
            b.golden[iz].homeFlag = 1; b.golden[ix].homeFlag = 2;
            r = MotorAccessDispatch(CellReq(false), 84, b);
            CHECK(!r.ok && Has(r.ackJson, "MInArmX") && Has(r.ackJson, "HomeFlag=2") && b.Calls() == k0, "S0: X's home failed (HomeFlag 2) -> refused");
            b.golden[ix].homeFlag = 1;
            b.golden[iy].softN = -50000;                                        // Y's target -78190 is below it
            r = MotorAccessDispatch(CellReq(false), 85, b);
            CHECK(!r.ok && Has(r.ackJson, "MInArmY") && Has(r.ackJson, "-78190") && Has(r.ackJson, "負向軟體極限") && b.Calls() == k0,
                  "S0: Y's target past its soft limit -> refused before anything moves (every target is checked first)");
            b.golden[iy].softN = -999999;
            b.golden[iz].softN = -1000;                                         // ZA's pick Z -2015 is below it; ZSafePos 50 is not
            r = MotorAccessDispatch(CellReq(true), 86, b);
            CHECK(!r.ok && Has(r.ackJson, "-2015") && b.Calls() == k0, "S0: zDown asked and the Z-down target is past the soft limit -> refused");
            r = MotorAccessDispatch(CellReq(false), 87, b);
            CHECK(r.ok && Cell().active, "... the same cell without zDown is accepted (the Z-down target is checked only when it will be used)");
            MotorAccessDispatch(Req("uteach", "stop", 0), 88, b);
            b.golden[iz].softN = -999999; b.golden[iz].softP = 40;              // ZSafePos 50 is past it
            k0 = b.Calls();
            r = MotorAccessDispatch(CellReq(false), 89, b);
            CHECK(!r.ok && Has(r.ackJson, "正向軟體極限") && b.Calls() == k0, "S0: ZSafePos itself past the Z's soft limit -> refused");
            b.golden[iz].softP = 999999;
            b.cellPlan = Plan("BinBox", false);
            MotorAccessReq rb = CellReq(true); rb.str["area"] = "BinBox";
            r = MotorAccessDispatch(rb, 90, b);
            CHECK(!r.ok && Has(r.ackJson, "不能降 Z") && Has(r.ackJson, "ZPlace[eBulkBox]") && b.Calls() == k0,
                  "S0: zDown on an area that does not allow it (q6 BinBox) -> refused with the area's reason");
            b.cellPlan = Plan("Loader", true);
            b.cellPlanWhy = "HotPlate 1：第一版未做";
            r = MotorAccessDispatch(CellReq(false), 91, b);
            CHECK(!r.ok && Has(r.ackJson, "第一版未做") && b.Calls() == k0, "S0: the backend's plan refuses (a v1 area) -> refused with its reason");
            b.cellPlanWhy.clear();
            MotorAccessReq rc = CellReq(false); rc.num.erase("row");
            CHECK(!MotorAccessDispatch(rc, 92, b).ok && b.Calls() == k0, "S0: no row -> refused");
            rc = CellReq(false); rc.num["col"] = 1.5;
            r = MotorAccessDispatch(rc, 93, b);
            CHECK(!r.ok && Has(r.ackJson, "col") && b.Calls() == k0, "S0: a non-integer col -> refused");
            b.powerOff = true;
            r = MotorAccessDispatch(CellReq(false), 94, b);
            CHECK(!r.ok && Has(r.ackJson, "Motor power is OFF") && b.Calls() == k0, "S0: motor power off -> refused (golden btnHomeClick :2137's test)");
            b.powerOff = false;
            b.table["MInArmY"].tableEnable = false;
            r = MotorAccessDispatch(CellReq(false), 95, b);
            CHECK(!r.ok && Has(r.ackJson, "MInArmY") && b.Calls() == k0, "S0: an Enable=0 axis in the plan -> refused (MotionPreludeFor)");
            b.table["MInArmY"].tableEnable = true;
            const int mn = TestMot("MInArmXmn");
            b.table["MInArmXmn"] = AxisOther(mn, "MN200", true); b.golden[mn] = g; b.aliasOf[mn] = "MInArmXmn";
            b.cellPlan.x = Ax("MInArmXmn");
            r = MotorAccessDispatch(CellReq(false), 96, b);
            CHECK(!r.ok && Has(r.ackJson, "PCI1203") && b.Calls() == k0, "S0: a non-1203 arm axis -> refused (v1: the 9050 arms are PCI1203)");
            b.cellPlan = Plan("Loader", true);
            MotorAccessReq rm = CellReq(false); rm.source = "uMotorTest";
            r = MotorAccessDispatch(rm, 97, b);
            CHECK(!r.ok && Has(r.ackJson, "MotorTest 沒有") && b.Calls() == k0, "Motor Test source -> refused (a Teach page action)");
            b.systemStart = true;
            r = MotorAccessDispatch(CellReq(false), 98, b);
            CHECK(!r.ok && Has(r.ackJson, "SystemStart") && b.Calls() == k0, "SystemStart -> refused (the dispatcher's run gate)");
            b.systemStart = false;
            FakeBackend nb;
            Add1203(nb, "MInArmX", 1, g);
            r = MotorAccessDispatch(CellReq(false), 99, nb);
            CHECK(!r.ok && Has(r.ackJson, "GoldenArmCellPlan") && nb.Calls() == 0, "a backend without the Arm Cell plan -> refused honestly, nothing sent");
            b.fixedMots["inZ"] = std::vector<int>(1, iz);
            MotorAccessReq zin = Req("uteach", "teachZAllUp", 0); zin.button = "btnInZAllUp"; zin.kind = "motion";
            CHECK(MotorAccessDispatch(zin, 100, b).ok && MotorAccessJobs().homesActive == 1, "setup: In Z All Up homes ZA (a HOME job)");
            k0 = b.Calls();
            r = MotorAccessDispatch(CellReq(false), 101, b);
            CHECK(!r.ok && Has(r.ackJson, "別的工作") && b.Calls() == k0 && !Cell().active, "S0: another job running (a HOME) -> refused");
            MotorAccessDispatch(Req("uteach", "stop", 0), 102, b);
        }

        // ---- S1: ERROR_STOP / stopped short / timeout / already there
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(true), 110, b);
            MotorAccessTick(b, 500);
            const std::size_t s0 = b.executed.size();
            b.state[3] = 3;                                                     // STA_AX_ERROR_STOP
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "ERROR_STOP") && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 &&
                  StopsOn(b, s0, 3) == 1 && StopsOn(b, s0, 4) == 0,
                  "S1: ZA in ERROR_STOP -> failed; the job's own axes stopped (X, Y, ZA -- not the shuttle)");
        }
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(true), 111, b);
            MotorAccessTick(b, 500);                                            // ZA commanded; the fake leaves it READY at 0
            MotorAccessTick(b, 500);
            CHECK(Cell().active, "S1: one fresh READY sample away from ZSafePos -> still waiting (TickLoop's rule needs two)");
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "沒到 ZSafePos"),
                  "S1: two fresh READY samples away from ZSafePos -> cancelled (it stopped short)");
        }
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(true), 112, b);
            MotorAccessTick(b, 500); Moving(b, 3);
            MotorAccessTick(b, 30000);
            CHECK(Cell().active, "S1: 30 s moving -> still running");
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 31000);
            CHECK(!Cell().active && Cell().result == "timeout" && Has(Cell().why, "60") && StopsOn(b, s0, 3) == 1, "S1: past 60 s -> timeout, stopped");
        }
        {
            FakeBackend b;
            Fresh(b);
            Arrive(b, 3, 49);                                                   // within the gap (2) of ZSafePos 50
            MotorAccessDispatch(CellReq(false), 113, b);
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(Cell().step == 2 && b.executed.size() == s0, "S1: ZA already READY within the gap of ZSafePos -> not commanded, straight to S2");
            MotorAccessDispatch(Req("uteach", "stop", 0), 114, b);
        }

        // ---- S2: the Z-at-home check and golden CheckCanMove (fail -> failed, X / Y never commanded)
        {
            FakeBackend b;
            Fresh(b);
            g_W906OrgActiveLow = 0;                                             // not the HT9050 hook: the card's ORG polarity is unmeasured here
            MotorAccessDispatch(CellReq(false), 120, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500);
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "fail-closed") && Has(Cell().why, "kPci1203CardOrgLogic") &&
                  b.teachCanMoveCalls.empty() && CountKind(b, s0, kCmdAxMoveAbs) == 0 && StopsOn(b, s0, 3) == 1,
                  "S2: the ORG state unknown (polarity unmeasured) -> failed fail-closed with the reason; X / Y never commanded");
            g_W906OrgActiveLow = []() { return true; };
        }
        {
            FakeBackend b;
            Fresh(b);
            b.motionIO[3] = 0x10;                                               // HT9050: ORG high = not at home
            MotorAccessDispatch(CellReq(false), 121, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500);
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "ORG 沒亮") && CountKind(b, s0, kCmdAxMoveAbs) == 0,
                  "S2: ZA at ZSafePos but its ORG not lit -> failed (golden Teach's Z-at-home check), X / Y never commanded");
        }
        {
            FakeBackend b;
            Fresh(b);
            b.canTeach = false;
            MotorAccessDispatch(CellReq(false), 122, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500);
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "CheckCanMove") && b.teachCanMoveCalls.size() == 1 &&
                  b.teachCanMoveCalls[0] == ix && CountKind(b, s0, kCmdAxMoveAbs) == 0,
                  "S2: golden CheckCanMove / IsCanQuickJogMove refuses X -> failed (asked once, Y not asked), X / Y never commanded");
        }

        // ---- S3: a refusal before / between the two commands, a Z leaving ZSafePos, the timeout
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(false), 130, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            b.notReady.insert(2);                                               // Y not READY (golden MotionDone()==false)
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "MInArmY") && CountKind(b, s0, kCmdAxMoveAbs) == 0,
                  "S3: Y blocked by MoveBlock1203 -> failed before either axis is commanded");
        }
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(false), 131, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            b.rejectMoveAxes.insert(2);                                         // the card refuses Y's move (after X's went out)
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            std::size_t mx = b.executed.size(), firstStopX = b.executed.size();
            for (std::size_t i = s0; i < b.executed.size(); ++i) if (b.executed[i].kind == kCmdAxMoveAbs && b.executed[i].axis == 1) { mx = i; break; }
            for (std::size_t i = mx; i < b.executed.size(); ++i) if (b.executed[i].kind == kCmdAxStop && b.executed[i].axis == 1) { firstStopX = i; break; }
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "已停 X") && mx < b.executed.size() && firstStopX < b.executed.size(),
                  "S3: Y cannot be issued after X was -> X is stopped, failed");
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            b.state[3] = 5;                                                     // ZA moving while X / Y move
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "離開了 ZSafePos") && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1,
                  "S3: a Z leaves ZSafePos while X / Y move -> cancelled, all stopped");
        }
        {
            FakeBackend b;
            Fresh(b);
            ToS3(b, CellReq(false));
            Arrive(b, 3, 30);                                                   // READY, but 20 below ZSafePos
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "現在 30"), "S3: a Z READY outside its gap of ZSafePos -> cancelled");
        }
        {
            FakeBackend b;
            Fresh(b);
            ToS3(b, CellReq(false));
            MotorAccessTick(b, 60000);
            CHECK(Cell().active, "S3: 60 s moving -> still running (S3 allows 120 s)");
            MotorAccessTick(b, 61000);
            CHECK(!Cell().active && Cell().result == "timeout" && Has(Cell().why, "120"), "S3: past 120 s -> timeout");
        }

        // ---- S4: D2 (a shuttle area) and only the chosen Z of several
        {
            FakeBackend b;
            Fresh(b);
            ArmCellPlan p = Plan("InShuttle1", true);
            p.zKind = "place"; p.d2 = true; p.d2Axis = Ax("MInShutte1"); p.d2Target = 1000; p.d2What = "In Arm 放料時 In Shuttle 1 停在左邊";
            b.cellPlan = p;
            Arrive(b, 4, 3000);                                                 // the shuttle is NOT at the arm side
            MotorAccessReq rs = CellReq(true); rs.str["area"] = "InShuttle1";
            CHECK(ToS3(b, rs), "setup: In Shuttle 1 cell, at S3");
            Arrive(b, 1, 20075); Arrive(b, 2, -78190); MotorAccessTick(b, 500);
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "D2") && Has(Cell().why, "MInShutte1") &&
                  CountKind(b, s0, kCmdAxMoveAbs) == 0 && CountKind(b, s0, kCmdAxStop) == 0,
                  "S4 D2: the shuttle away from its arm-side point -> failed, the Z does not go down (X / Y stay at the cell, nothing to stop)");
        }
        {
            FakeBackend b;
            Fresh(b);
            ArmCellPlan p = Plan("InShuttle1", true);
            p.d2 = true; p.d2Axis = Ax("MInShutte1"); p.d2Target = 1000;
            b.cellPlan = p;
            Arrive(b, 4, 1001);                                                 // within 2 of its arm-side point, READY
            ToS3(b, CellReq(true));
            Arrive(b, 1, 20075); Arrive(b, 2, -78190); MotorAccessTick(b, 500);
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(Cell().active && Cell().step == 4 && MovesTo(b, s0, 3, -2015.0) == 1, "S4 D2: the shuttle at the arm side -> the Z goes down");
            std::string lk;
            const MotorAccessOutcome mj = MotorAccessDispatch(Req("uMotorTest", "jogP", "MInShutte1"), 192, b);
            CHECK(MotorAccessMovingOf(b, ish) == 1 && MotorAccessAxisLock(b, ish, lk) == 1 && lk == "ARM CELL" && !mj.ok && Has(mj.ackJson, "正在用的軸") &&
                  MotorAccessJobs().cellMotors.size() == 4 && MotorAccessJobs().cellMotors[3] == "MInShutte1" && Cell().motors.size() == 4,
                  "review m3: D2 passed -> the shuttle is the job's while the Z goes down (moving / ARM CELL lock, a Motor Test jog on it refused)");
            Arrive(b, 3, -2015); MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "arrived" && MotorAccessMovingOf(b, ish) == 0, "... and arrives; the shuttle is free again");
        }
        {
            FakeBackend b;
            Fresh(b);
            Add1203(b, "MInArmZC", 5, gz); b.motionIO[5] = 0;
            ArmCellPlan p = Plan("Loader", true);
            p.zLift.push_back(Ax("MInArmZC"));                                  // a job-level check: v1 plans never carry two (ctest ArmCellPlan [7])
            b.cellPlan = p;
            MotorAccessDispatch(CellReq(true), 140, b);
            std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(MovesTo(b, s0, 3, 50.0) == 1 && MovesTo(b, s0, 5, 50.0) == 1, "S1: every Z of the plan's zLift goes to ZSafePos (ZA and ZC)");
            Arrive(b, 3, 50); MotorAccessTick(b, 500);
            CHECK(Cell().step == 1, "S1: ZC not there yet -> still S1");
            Arrive(b, 5, 50); MotorAccessTick(b, 500); MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            Arrive(b, 1, 20075); Arrive(b, 2, -78190); MotorAccessTick(b, 500);
            s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(Cell().step == 4 && CountKind(b, s0, kCmdAxMoveAbs) == 1 && MovesTo(b, s0, 3, -2015.0) == 1,
                  "S4: only the chosen Z (ZA) goes down; ZC stays at ZSafePos");
            MotorAccessDispatch(Req("uteach", "stop", 0), 141, b);
        }

        // ---- review m5: the refusal text other actions use while the job runs (START here; wb_serve's main.home through the Live thunk)
        {
            FakeBackend b;
            Fresh(b);
            CHECK(MotorAccessArmCellBlockedWhy("HOME").empty(), "review m5: no job -> no Arm Cell refusal text");
            CHECK(ToS3(b, CellReq(false)), "setup: at S3");
            std::string sw2;
            const std::string hw = MotorAccessArmCellBlockedWhy("HOME");
            CHECK(hw.compare(0, std::string("HOME 拒絕：教導頁 Arm Cell 進行中").size(), "HOME 拒絕：教導頁 Arm Cell 進行中") == 0 && Has(hw, "Loader (3, 8)") &&
                  !Has(hw, "手動教導") && MotorAccessStartBlocked(sw2) && sw2.compare(0, std::string("START 拒絕：教導頁 Arm Cell").size(), "START 拒絕：教導頁 Arm Cell") == 0,
                  "review m5: while it runs -> 'HOME 拒絕：教導頁 Arm Cell 進行中（Loader (3, 8)…）' (main.home) / the START one -- not the hand-teach text");
            MotorAccessDispatch(Req("uteach", "stop", 0), 193, b);
        }

        // ---- review N5: a refused stop at the job's end is reported (DoStop's partial), not dropped
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            b.rejectKinds.insert((int)kCmdAxStop);                              // the card refuses every stop
            MotorAccessTick(b, 500, false);                                     // the operator gone -> cancel + the job's stops (D3)
            const std::string js = MotorAccessArmCellJson(b);
            CHECK(!Cell().active && Cell().result == "cancelled" && Cell().stopped && Cell().stopRefused == 3 && Cell().stopAccepted == 0 &&
                  Has(Cell().why, "停軸被拒 3 軸") && Has(Cell().stopWhy, "MInArmX") && Has(Cell().stopWhy, "fake: this kind refused") &&
                  Has(js, "\"refused\":3") && Has(js, "\"partial\":true"),
                  "review N5: every stop of the job's axes refused -> why says so, runtime job.stop {refused 3, partial true}");
            b.rejectKinds.clear();
            Fresh(b);
            ToS3(b, CellReq(false));
            MotorAccessTick(b, 500, false);
            CHECK(Cell().stopped && Cell().stopRefused == 0 && Cell().stopAccepted == 3 && !Has(Cell().why, "停軸被拒") &&
                  Has(MotorAccessArmCellJson(b), "\"partial\":false"), "review N5: stops accepted -> counted, no warning");
        }

        // ---- review m1: the Z re-checked in the same beat right before X / Y go; the Z watch fails closed; X / Y re-checked before Z down
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(false), 190, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            CHECK(Cell().step == 3, "setup: at S3, X / Y not commanded yet");
            Arrive(b, 3, 30);                                                   // the Z sank between S2 and this beat
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "S3 下 X／Y 之前") && Has(Cell().why, "現在 30") && CountKind(b, s0, kCmdAxMoveAbs) == 0,
                  "review m1: the Z re-checked in the beat X / Y would go -> off ZSafePos -> cancelled, X / Y never commanded");
        }
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(false), 191, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            b.state.erase(3);                                                   // no valid monitor sample of the Z
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "fail-closed") && CountKind(b, s0, kCmdAxMoveAbs) == 0,
                  "review m1: no Z sample when X / Y would go -> fail closed, X / Y never commanded");
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            b.state.erase(3);
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "fail-closed") && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1,
                  "review m1: the Z watch while X / Y move fails closed on a missing sample (was: skipped) -> cancelled, stopped");
        }
        {
            FakeBackend b;
            Fresh(b);
            ToS3(b, CellReq(true));
            Arrive(b, 1, 20075); Arrive(b, 2, -78190); MotorAccessTick(b, 500);
            CHECK(Cell().step == 4, "setup: X / Y at the cell -> S4");
            Arrive(b, 1, 20000);                                                // X pushed off the cell before the Z goes down
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "S4 降 Z 之前") && Has(Cell().why, "MInArmX") && MovesTo(b, s0, 3, -2015.0) == 0,
                  "review m1: X / Y re-checked before the Z goes down -> X off the cell -> cancelled, the Z never sent down");
        }

        // ---- review m7: a hand-taught 1203 Z sits on the encoder base (g_encBase, W5B-6): S1's "already at ZSafePos" and the S3 Z
        //   watch read it the way CurrentUserPos does (actPos), not the stale command position
        {
            FakeBackend b;
            Fresh(b);
            b.reg = BuildTeachRegistry(std::set<std::string>());
            b.golden[iz].servoAlarmOn = true;                                   // golden PServoAlarmOn -> encoder base after the hand teach
            const MotorAccessOutcome hs = MotorAccessDispatch(TeachReq("teachSet", "SetButton030", true, true), 185, b);   // hand teach on MInArmZA
            MotorAccessReq he = TeachReq("teachSet", "SetButton030", true, false); he.flag["accept"] = true;
            const MotorAccessOutcome hd = MotorAccessDispatch(he, 186, b);
            CHECK(hs.ok && hd.ok && MotorAccessEncoderBase(3) && !MotorAccessJobs().teachActive, "setup: a hand teach on MInArmZA leaves its 1203 axis on the encoder base");
            b.cmdPos[3] = 50.0; b.actPos[3] = 0.0;                              // the command position says ZSafePos, the encoder says 0
            CHECK(MotorAccessDispatch(CellReq(false), 187, b).ok, "setup: accepted");
            std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(MovesTo(b, s0, 3, 50.0) == 1, "review m7: S1 reads a hand-taught Z's encoder (0) -> not at ZSafePos -> commanded");
            Arrive(b, 3, 50); MotorAccessTick(b, 500); MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            Moving(b, 1); Moving(b, 2);
            b.actPos[3] = 30.0;                                                 // the encoder says the Z sank; the command position still 50
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "現在 30"), "review m7: the S3 Z watch reads the encoder of a hand-taught Z too");
        }

        // ---- every cancel trigger: cancelled, and the job's own axes stopped (D3; HOME / Loop leave theirs running, golden)
        for (int t = 0; t < 10; ++t) {
            FakeBackend b;
            Fresh(b);
            const bool up = ToS3(b, CellReq(true));
            const std::size_t s0 = b.executed.size();
            std::string label;
            switch (t) {
            case 0: MotorAccessDispatch(Req("uteach", "stop", 0), 150, b); label = "STOP (golden btnStopClick: StopAllMotor + AllBtnUp)"; break;
            case 1: MotorAccessOnAlarm(b, "ALM9999"); label = "an alarm (MotorAccessOnAlarm)"; break;
            case 2: MotorAccessTick(b, 500, false); label = "the operator gone / the token taken (beat)"; break;
            case 3: MotorAccessPollTick(b, false); label = "the operator gone, seen right after a Poll"; break;
            case 4: b.safeLock = true; MotorAccessTick(b, 500); label = "the safe lock"; break;
            case 5: b.systemStart = true; MotorAccessTick(b, 500); label = "SystemStart turning true"; break;
            case 6: MotorAccessAllBtnUp(b, "VerifyMotorAction"); label = "VerifyMotorAction's AllBtnUp"; break;
            case 7: b.doorOpen.insert(iy); MotorAccessTick(b, 500); label = "a safe door open on one of its axes"; break;
            case 8: MotorAccessDispatch(Req("uMotorTest", "formShow", 0), 151, b); label = "Motor Test FormShow (golden :1003 AllBtnUp)"; break;
            default: MotorAccessDispatch(Req("uMotorTest", "selectMotor", "MInShutte1"), 152, b); label = "Motor Test selects another motor (golden lM00Click)"; break;
            }
            const MotorAccessArmCellState s = Cell();
            const bool own = StopsOn(b, s0, 1) >= 1 && StopsOn(b, s0, 2) >= 1 && StopsOn(b, s0, 3) >= 1;
            const bool onlyOwn = (t <= 1 || t >= 8) || StopsOn(b, s0, 4) == 0;  // STOP / an alarm also stop every opened axis (golden StopAllMotor)
            const std::string msg = "cancel: " + label + " -> cancelled, the job's own axes stopped (D3)";
            CHECK(up && !s.active && s.result == "cancelled" && own && onlyOwn && !MotorAccessJobs().cellActive, msg.c_str());
        }

        // ---- AI(W906-ARMCELL2) 20261002: NB2 R165 (the user 1002 22:1x 「照建議」) + NB2's 04:30 review list
        //   (1) a box on screen stops it (HT160 IsFaultPopupShowing -> StopAllAdvancedTests): the hosts' edge, the state half, and
        //       the box the job's own S2 golden check pops; (2) S0 reads every commanded axis' alarm / EMG lamp + the EMG sensors, the
        //       beat cancels on them (HT160 CheckSortArmTestReady / tmrUpdate); (4) S3 watches every Z's origin, two fresh samples;
        //   review: arrived = the encoder (never the command position alone), X refused -> no Y, every timeout stops the axes.
        printf("[ARMCELL2] NB2 R165: popups, lamps / EMG, the Z origin watch, the encoder arrival\n");
        static std::string s_cellPopup;                                         // what the state half reports ("" = no box up)
        g_W906ArmCellPopupUp = [](std::string& w) { if (s_cellPopup.empty()) return false; w = s_cellPopup; return true; };
        {
            FakeBackend b;
            Fresh(b);
            CHECK(!MotorAccessArmCellOnPopup(b, "訊息框", "Tray full") && b.Calls() == 0, "R165 (1): a box with no job running -> nothing (false, nothing sent)");
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            const std::size_t s0 = b.executed.size();
            const bool hit = MotorAccessArmCellOnPopup(b, "訊息框", "Tray full");
            const MotorAccessArmCellState s = Cell();
            CHECK(hit && !s.active && s.result == "cancelled" && Has(s.why, "訊息框「Tray full」") && Has(s.why, "IsFaultPopupShowing") &&
                  StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 1 && StopsOn(b, s0, 4) == 0 && CountKind(b, s0, kCmdAxMoveAbs) == 0,
                  "R165 (1): a box shown while X / Y move (the hosts' edge) -> cancelled (the page: 已中止) naming the box, the job's own axes stopped, not the shuttle");
            Arrive(b, 1, 20075); Arrive(b, 2, -78190);
            const std::size_t s1 = b.executed.size();
            MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            CHECK(Cell().result == "cancelled" && b.executed.size() == s1,
                  "R165 (1): after the box nothing moves again (no 'one more Y step', HT160's abort flag) and it never turns 'arrived'");
        }
        {
            FakeBackend b;
            Fresh(b);
            b.popupInTeachCanMove = true; b.canTeach = false;                   // golden CheckCanMove refuses AND pops its message
            MotorAccessDispatch(CellReq(false), 300, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500);
            MotorAccessTick(b, 500);
            CHECK(!b.popupDuringCheck && !Cell().active && Cell().result == "failed" && Has(Cell().why, "CheckCanMove"),
                  "R165 (1): the box golden CheckCanMove pops inside S2 is that check's -- not a popup abort; the job ends 'failed' with the check's reason");
        }
        {
            FakeBackend b;
            Fresh(b);
            s_cellPopup = "訊息框（golden MyMessageBox）還開著";
            const int k0 = b.Calls();
            const MotorAccessOutcome r = MotorAccessDispatch(CellReq(false), 301, b);
            CHECK(!r.ok && Has(r.ackJson, "MyMessageBox") && Has(r.ackJson, "IsFaultPopupShowing") && b.Calls() == k0 && !Cell().active,
                  "R165 (1): a box already up -> S0 refused naming it, nothing sent, no job");
            s_cellPopup.clear();
            CHECK(ToS3(b, CellReq(false)), "setup: no box -> accepted, at S3");
            s_cellPopup = "告警框（golden fNote）還開著";
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            s_cellPopup.clear();
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "fNote") && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 1,
                  "R165 (1): a box up on a beat (a modeless one: the tick still runs) -> cancelled, the job's axes stopped");
        }
        {
            FakeBackend b;
            Fresh(b);
            const int k0 = b.Calls();
            struct LampCase { int slot; unsigned long io; unsigned st; bool dropIo; const char* has; const char* who; };
            const LampCase cs[] = { { 1, 0x2ul, 1, false, "ALM", "MInArmX" }, { 2, 0x40ul, 1, false, "EMG 亮", "MInArmY" },
                                    { 3, 0ul, 3, false, "ERROR_STOP", "MInArmZA" }, { 2, 0ul, 1, true, "fail-closed", "MInArmY" } };
            int refused = 0;
            for (std::size_t k = 0; k < sizeof(cs) / sizeof(cs[0]); ++k) {
                const LampCase& c = cs[k];
                b.motionIO[c.slot] = c.io; b.state[c.slot] = c.st; if (c.dropIo) b.motionIO.erase(c.slot);
                const MotorAccessOutcome r = MotorAccessDispatch(CellReq(false), 310 + (long long)k, b);
                if (!r.ok && Has(r.ackJson, c.has) && Has(r.ackJson, c.who) && Has(r.ackJson, "CheckSortArmTestReady") && !Cell().active && b.Calls() == k0) ++refused;
                else printf("    S0 lamp case %s / %s: ok=%d ack=%s\n", c.who, c.has, (int)r.ok, r.ackJson.c_str());
                b.motionIO[c.slot] = 0; b.state[c.slot] = 1;
            }
            CHECK(refused == 4, "R165 (2): S0 refuses on a commanded axis' lamp as golden decodes it for the 1203 -- X ALM (bit 1), Y EMG (bit 6), ZA ERROR_STOP, Y with no sample (fail closed); nothing sent, no job");
            b.emgOff = true;
            MotorAccessOutcome r = MotorAccessDispatch(CellReq(false), 315, b);
            CHECK(!r.ok && Has(r.ackJson, "急停 EMG") && b.Calls() == k0 && !Cell().active, "R165 (2): an EMG sensor off (golden InitMotor :180-181) -> S0 refused");
            b.emgOff = false;
            r = MotorAccessDispatch(CellReq(false), 316, b);
            CHECK(r.ok && Cell().active, "... every lamp dark, no EMG -> accepted");
            MotorAccessDispatch(Req("uteach", "stop", 0), 317, b);
        }
        for (int t = 0; t < 3; ++t) {
            FakeBackend b;
            Fresh(b);
            const bool up = ToS3(b, CellReq(true));
            const std::size_t s0 = b.executed.size();
            const char* label = (t == 0) ? "the EMG lamp (bit 6) on X" : (t == 1) ? "ALM on Y" : "an EMG sensor off (EMG pressed)";
            if (t == 0) b.motionIO[1] = 0x40ul; else if (t == 1) b.motionIO[2] = 0x2ul; else b.emgOff = true;
            MotorAccessTick(b, 500);
            const MotorAccessArmCellState s = Cell();
            const bool ok1 = up && !s.active && s.result == "cancelled" && Has(s.why, t == 1 ? "ALM" : "EMG") &&
                             StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 1;
            Arrive(b, 1, 20075); Arrive(b, 2, -78190); b.emgOff = false; b.motionIO[1] = 0; b.motionIO[2] = 0;
            const std::size_t s1 = b.executed.size();
            MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            const bool ok2 = Cell().result == "cancelled" && b.executed.size() == s1 && !Has(MotorAccessArmCellJson(b), "\"result\":\"arrived\"");
            const std::string msg = std::string("R165 (2) / review: ") + label + " while X / Y move -> cancelled (the page: 已中止), the job's axes stopped; never 'arrived' afterwards";
            CHECK(ok1 && ok2, msg.c_str());
        }
        {   //AI(W906-ARMCELL-ZORG) 20261003: R165 (4) as NB2 R174 M1 changed it -- one dark sample no longer means "nothing" (it stops
            //  X / Y, see [ARMCELL-ZORG] below), so "lost, back, lost -> still running" is gone: a flicker now ends the job (cancelled).
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            b.motionIO[3] = 0x10ul;                                             // HT9050 + SensorType 1: ORG high = ZA not at home
            MotorAccessTick(b, 500);
            CHECK(Cell().active, "R165 (4): ZA's origin lost on one fresh sample -> the job is not ended on it (the next sample decides)");
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "ORG") && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 1,
                  "R165 (4): ZA's origin lost on two fresh samples in a row while X / Y move -> cancelled, the job's axes stopped (HT160 MoveSortArmX)");
        }

        // ---- AI(W906-ARMCELL-ZORG) 20261003: NB2 R174 M1 (NIGHT_REPORT s0 #70: 「第一次讀到就先停 X 不用問，筆電下一批照 HT160 做」) --
        //   HT160 MoveSortArmX (aSortArm.cpp:726-745 as NB2 read it): the FIRST read of a Z off home stops X at once (:733), the loss is
        //   confirmed (SUCK_HOME_LOST_MS 100) before StopAllMotor + an alarm. Here: the first dark sample stops the travel legs still
        //   under way (X / Y; Stop1203 = StopDec + ExtDrive 0) and nothing else; the next fresh sample ends the job "cancelled" (still
        //   dark = the two-sample rule; lit again = a flicker, X / Y not re-commanded). The watch also runs right after each 1203 Poll.
        //   ZA's encoder stays at ZSafePos in every case (the ORG goes dark while the motor encoder does not move: belt / coupling
        //   slip, a failing sensor) -- so ArmCellZStillSafe (the encoder check, unchanged) does not end the job first.
        printf("[ARMCELL-ZORG] NB2 R174 M1: the first dark Z origin sample stops X / Y at once (HT160 MoveSortArmX :733); the post-Poll watch\n");
        auto ExtDriveOff = [](const FakeBackend& b, std::size_t from, int slot) {   // Stop1203's second half (Acm_AxSetExtDrive(ax, 0))
            int n = 0;
            for (std::size_t i = from; i < b.extDrive.size(); ++i) if (b.extDrive[i].axis == slot && b.extDrive[i].value == 0.0) ++n;
            return n;
        };
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving, ZA at ZSafePos (command position and encoder 50)");
            const std::size_t s0 = b.executed.size(), e0 = b.extDrive.size();
            b.motionIO[3] = 0x10ul;                                             // ZA's ORG dark; its encoder stays at 50
            MotorAccessTick(b, 500);
            CHECK(Cell().active && Cell().result == "running" && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && ExtDriveOff(b, e0, 1) == 1 &&
                  ExtDriveOff(b, e0, 2) == 1 && StopsOn(b, s0, 3) == 0 && StopsOn(b, s0, 4) == 0 && CountKind(b, s0, kCmdAxMoveAbs) == 0 &&
                  b.executed.size() == s0 + 2 && b.stopAll.empty() && b.stopMotor.empty() && Has(MotorAccessJobs().lastNote, "就先停了 MInArmX、MInArmY"),
                  "ZORG: ZA's origin dark on the FIRST fresh sample while X / Y move, its encoder unchanged (belt / coupling slip) -> X and Y stopped at once with the job's own stop (StopDec + ExtDrive 0); ZA and the shuttle not, nothing moved, no stop-all; the job still running (the next sample decides)");
            Arrive(b, 1, 20075); Arrive(b, 2, -78190);                          // X / Y even read at the cell now
            const std::size_t s1 = b.executed.size();
            MotorAccessTick(b, 500);                                            // ZA still dark: the confirming sample
            const MotorAccessArmCellState s = Cell();
            CHECK(!s.active && s.result == "cancelled" && !s.hasFinal && Has(s.why, "連續兩個新樣本") && Has(s.why, "就先停了 MInArmX、MInArmY") &&
                  StopsOn(b, s1, 1) == 1 && StopsOn(b, s1, 2) == 1 && StopsOn(b, s1, 3) == 1 && StopsOn(b, s1, 4) == 0 && CountKind(b, s1, kCmdAxMoveAbs) == 0,
                  "ZORG: still dark on the next fresh sample -> cancelled, never 'arrived' (no leg is judged after the first stop, though X / Y read at the cell); the end stops the job's axes (X, Y again, ZA), not the shuttle; the why names the first stop");  CHECK(b.zOrgAlarms.size() == 1 && b.zOrgAlarms[0].compare(0, 9, "MInArmZA|") == 0 && Has(b.zOrgAlarms[0], "不在原點") && !b.zOrgAlarmJobActive[0], "ZALARM (RULINGS_20261003 #22 = s0 #70 (2), HT160 aSortArm.cpp:739-740): the confirmed loss raises the alarm box once, for MInArmZA, after the job ended");   //AI(W906-ARMCELL-ZALARM) 20261003: NB2-1, same line
        }
        for (int t = 0; t < 2; ++t) {
            FakeBackend b;
            Fresh(b);
            const bool up = ToS3(b, CellReq(false));
            if (t == 0) Arrive(b, 2, -78190); else Arrive(b, 1, 20075);         // one leg already at the cell (at rest), the other moving
            MotorAccessTick(b, 500);
            const bool atS3 = up && Cell().active && Cell().step == 3;
            const std::size_t s0 = b.executed.size();
            b.motionIO[3] = 0x10ul;
            MotorAccessTick(b, 500);
            const int moving = (t == 0) ? 1 : 2, rested = (t == 0) ? 2 : 1;
            const std::string msg = std::string("ZORG: ") + (t == 0 ? "Y" : "X") + " already arrived (at rest at the cell) -> only " + (t == 0 ? "X" : "Y") +
                                    ", the travel leg still under way, is stopped on the first dark sample (nothing else)";
            CHECK(atS3 && Cell().active && StopsOn(b, s0, moving) == 1 && StopsOn(b, s0, rested) == 0 && StopsOn(b, s0, 3) == 0 && StopsOn(b, s0, 4) == 0 &&
                  b.executed.size() == s0 + 1, msg.c_str());
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            const std::size_t s0 = b.executed.size();
            b.motionIO[3] = 0x10ul; MotorAccessTick(b, 500);                    // one dark sample ...
            const bool firstStop = Cell().active && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 0;
            const std::size_t s1 = b.executed.size();
            b.motionIO[3] = 0; MotorAccessTick(b, 500);                         // ... lit again on the next fresh sample
            const MotorAccessArmCellState s = Cell();
            CHECK(firstStop && !s.active && s.result == "cancelled" && Has(s.why, "閃了一下") && Has(s.why, "不自動續走") && Has(s.why, "就先停了 MInArmX、MInArmY") &&
                  StopsOn(b, s1, 1) == 1 && StopsOn(b, s1, 2) == 1 && StopsOn(b, s1, 3) == 1 && StopsOn(b, s1, 4) == 0 && CountKind(b, s1, kCmdAxMoveAbs) == 0,
                  "ZORG: the ORG flickers once (dark, lit on the next fresh sample) -> the X / Y stop already went out on the first sample (HT160's behaviour); the next sample ends the job cancelled at once -- X / Y are NOT re-commanded (no resume after a sensor glitch; press GO again); the end stops the job's axes, not the shuttle");
            Arrive(b, 1, 20075); Arrive(b, 2, -78190);
            const std::size_t s2 = b.executed.size();
            MotorAccessTick(b, 500); MotorAccessPollTick(b); MotorAccessTick(b, 500);
            CHECK(Cell().result == "cancelled" && b.executed.size() == s2 && !Has(MotorAccessArmCellJson(b), "\"result\":\"arrived\""),
                  "ZORG: ... after the flicker's end nothing is sent again and it never turns 'arrived'");  CHECK(b.zOrgAlarms.empty(), "ZALARM: a one-sample flicker raises no alarm box (cancel only; HT160 :744-745 resumes when the sensor comes back)");   //AI(W906-ARMCELL-ZALARM) 20261003: NB2-1, same line
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            const std::size_t s0 = b.executed.size();
            for (int k = 0; k < 3; ++k) { MotorAccessPollTick(b); MotorAccessPollTick(b); MotorAccessTick(b, 500); }
            CHECK(Cell().active && Cell().step == 3 && b.executed.size() == s0, "ZORG: ZA's origin stays lit while X / Y move (polls and beats) -> nothing is stopped or sent");
            Arrive(b, 1, 20075); Arrive(b, 2, -78190);
            MotorAccessPollTick(b);
            const bool notOnPoll = Cell().active && Cell().step == 3;
            MotorAccessTick(b, 500);
            CHECK(notOnPoll && !Cell().active && Cell().result == "arrived" && b.executed.size() == s0,
                  "ZORG: ... the post-Poll watch never judges the legs (arrival stays the beat's); the job arrives as before, no stop sent at any point");
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            const std::size_t s0 = b.executed.size();
            b.motionIO[3] = 0x10ul;
            MotorAccessPollTick(b);                                             // the Poll that read the dark sample -- no beat yet
            CHECK(Cell().active && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 0 && StopsOn(b, s0, 4) == 0 && b.executed.size() == s0 + 2,
                  "ZORG (2): the post-Poll tick (~200 ms) reads the first dark sample -> X / Y stopped right there, without waiting for the 500 ms beat; ZA and the shuttle not");
            b.freezePolls = true;                                               // the beat (and a second look) read the SAME sample
            const std::size_t s1 = b.executed.size();
            MotorAccessTick(b, 500); MotorAccessPollTick(b);
            CHECK(Cell().active && b.executed.size() == s1,
                  "ZORG (2): the beat on the same sample (one thread; the sample counted once, the stop latched) -> no second stop, not counted again: still waiting for the next sample");
            b.freezePolls = false;
            MotorAccessPollTick(b);                                             // the next Poll's sample, still dark
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "連續兩個新樣本") && StopsOn(b, s1, 1) == 1 && StopsOn(b, s1, 2) == 1 &&
                  StopsOn(b, s1, 3) == 1 && StopsOn(b, s1, 4) == 0,
                  "ZORG (2): the next Poll's sample still dark -> cancelled right after that Poll (one poll period after the first; was ~0.5-1 s of beats), the job's axes stopped");
            const std::size_t s2 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(b.executed.size() == s2, "ZORG (2): the beat after it finds the job over -> nothing sent twice");  CHECK(b.zOrgAlarms.size() == 1 && !b.zOrgAlarmJobActive[0], "ZALARM (2): the post-Poll confirmation raises the alarm once (after the job ended); the beat after it does not raise it again");   //AI(W906-ARMCELL-ZALARM) 20261003: NB2-1, same line
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            const std::size_t s0 = b.executed.size();
            b.motionIO.erase(3);                                                // no motionIO sample for ZA at all
            MotorAccessPollTick(b);
            const bool first = Cell().active && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 0;
            MotorAccessTick(b, 500);
            CHECK(first && !Cell().active && Cell().result == "cancelled" && StopsOn(b, s0, 3) == 1,
                  "ZORG (2): ZA's origin unknown (no motionIO sample: fail closed) -> the post-Poll watch stops X / Y on it; the beat then ends the job (its own fail-closed lamp read), every axis stopped");
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(MotorAccessDispatch(CellReq(true), 350, b).ok, "setup: accepted (zDown)");
            b.motionIO[3] = 0x10ul;                                             // ZA below its origin before S1 lifts it
            const std::size_t s0 = b.executed.size();
            MotorAccessPollTick(b);
            const bool quietS1 = Cell().active && Cell().step == 1 && b.executed.size() == s0;
            MotorAccessTick(b, 500);                                            // S1: ZA commanded up
            Arrive(b, 3, 50); b.motionIO[3] = 0; MotorAccessTick(b, 500);      // at ZSafePos, ORG lit -> S2
            MotorAccessTick(b, 500);                                            // S2 -> S3
            const std::size_t s1 = b.executed.size();
            b.motionIO[3] = 0x10ul; MotorAccessPollTick(b); b.motionIO[3] = 0;
            const bool quietS3 = Cell().active && Cell().step == 3 && b.executed.size() == s1;
            CHECK(quietS1 && quietS3, "ZORG (2): the post-Poll watch does nothing in S1 (the Zs not up yet) or in S3 before X / Y are issued (S2 checked the origin on a newer sample; the issuing beat re-checks the Z position, unchanged)");
            MotorAccessTick(b, 500);                                            // S3: X / Y issued
            Moving(b, 1); Moving(b, 2);
            Arrive(b, 1, 20075); Arrive(b, 2, -78190); MotorAccessTick(b, 500);
            MotorAccessTick(b, 500);                                            // S4: ZA commanded down
            Moving(b, 3); b.motionIO[3] = 0x10ul;                               // ZA leaves its origin -- by design
            const std::size_t s2 = b.executed.size();
            MotorAccessPollTick(b); MotorAccessPollTick(b); MotorAccessTick(b, 500);
            const bool quietS4 = Cell().active && Cell().step == 4 && b.executed.size() == s2;
            Arrive(b, 3, -2015); MotorAccessTick(b, 500);
            CHECK(quietS4 && !Cell().active && Cell().result == "arrived",
                  "ZORG (2): S4 (the chosen Z going down, its ORG dark by design) -> neither the post-Poll watch nor the beat stops anything; S4 arrives as before");
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3");
            b.cmdPos[1] = 20075; b.actPos[1] = 20075 - 50; b.state[1] = 1;      // X: the command position at the cell, the encoder 0.5 mm short
            Arrive(b, 2, -78190);
            MotorAccessTick(b, 500);
            CHECK(Cell().active && Cell().step == 3 && Cell().result == "running",
                  "review: X's command position at the cell but its encoder 50 off -> NOT arrived (the command position alone never counts)");
            b.actPos[1] = 20075 - 3;                                            // settled within kArmCellEncTol
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "arrived" && Cell().finalAct[0] == 20072, "review: ... the encoder within 0.1 mm -> arrived (final actPos 20072)");
        }
        {
            FakeBackend b;
            Fresh(b);
            ToS3(b, CellReq(false));
            b.cmdPos[1] = 20075; b.actPos[1] = 19000; b.state[1] = 1; Arrive(b, 2, -78190);
            MotorAccessTick(b, 500);
            MotorAccessTick(b, 2000);
            CHECK(Cell().active, "review: the encoder still off 2 s after the command got there -> still waiting (settling)");
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 1500);
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "編碼器在 19000") && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 1,
                  "review: the encoder not following 3 s after the command got there -> failed with both numbers, the job's axes stopped");
        }
        {
            FakeBackend b;
            Fresh(b);
            b.cmdPos[3] = 50.0; b.actPos[3] = 0.0;                              // ZA: the command position says ZSafePos, the encoder says 0 (no hand teach)
            CHECK(MotorAccessDispatch(CellReq(false), 320, b).ok, "setup: accepted");
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(MovesTo(b, s0, 3, 50.0) == 1, "review: S1 'already at ZSafePos' needs the encoder too -> a Z whose encoder is elsewhere is commanded up");
            MotorAccessDispatch(Req("uteach", "stop", 0), 321, b);
        }
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(false)), "setup: at S3, X / Y moving");
            b.actPos[3] = 30.0;                                                 // ZA's encoder sank 0.2 mm, its command position still ZSafePos (no hand teach)
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "cancelled" && Has(Cell().why, "編碼器 30") && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1,
                  "review: the S3 Z watch reads the encoder too -> ZA's encoder off ZSafePos (its command position still there) -> cancelled, stopped");
        }
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(false), 330, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500); MotorAccessTick(b, 500);
            b.rejectMoveAxes.insert(1);                                         // the card refuses X's move
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 500);
            CHECK(!Cell().active && Cell().result == "failed" && Has(Cell().why, "S3 X") && MovesTo(b, s0, 2, -78190.0) == 0 && CountKind(b, s0, kCmdAxMoveAbs) == 1,
                  "review: X's command refused by the card -> failed, Y never commanded");
        }
        {
            FakeBackend b;
            Fresh(b);
            MotorAccessDispatch(CellReq(false), 340, b);
            MotorAccessTick(b, 500); Arrive(b, 3, 50); MotorAccessTick(b, 500);
            const bool atS2 = Cell().step == 2;
            b.freezePolls = true;                                               // no monitor sample newer than the arrival
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 61000);
            b.freezePolls = false;
            CHECK(atS2 && !Cell().active && Cell().result == "timeout" && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 1,
                  "review: the S2 timeout stops the job's axes");
        }
        {
            FakeBackend b;
            Fresh(b);
            ToS3(b, CellReq(false));
            MotorAccessTick(b, 60000);
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 61000);
            CHECK(!Cell().active && Cell().result == "timeout" && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1 && StopsOn(b, s0, 3) == 1,
                  "review: the S3 timeout stops X, Y and the Z");
        }
        {
            FakeBackend b;
            Fresh(b);
            ToS3(b, CellReq(true));
            Arrive(b, 1, 20075); Arrive(b, 2, -78190); MotorAccessTick(b, 500); // -> S4
            MotorAccessTick(b, 500); Moving(b, 3);                              // the Z commanded down, moving
            const std::size_t s0 = b.executed.size();
            MotorAccessTick(b, 61000);
            CHECK(!Cell().active && Cell().result == "timeout" && Has(Cell().why, "S4") && StopsOn(b, s0, 3) == 1 && StopsOn(b, s0, 1) == 1 && StopsOn(b, s0, 2) == 1,
                  "review: the S4 timeout stops the Z (and X / Y)");
        }
        // ---- R165 (1): the hosts' five edge calls, pinned in the source -- tools/wb_serve.cpp cannot be linked into a ctest (the
        //   practice of test_note_motorerror [5] / test_notice_ack [M]). The tree root comes from argv[1] (<tree>/../web/JSON/motor-access.json).
        {
            std::string a1 = (argc > 1) ? std::string(argv[1]) : std::string();
            for (std::size_t k = 0; k < a1.size(); ++k) if (a1[k] == '\\') a1[k] = '/';
            const std::string tail = "/../web/JSON/motor-access.json";
            const bool shaped = a1.size() > tail.size() && a1.compare(a1.size() - tail.size(), tail.size(), tail) == 0;
            const std::string root = shaped ? a1.substr(0, a1.size() - tail.size()) : std::string();
            const std::string ws = shaped ? ReadAll(root + "/tools/wb_serve.cpp") : std::string();
            const std::string lv = shaped ? ReadAll(root + "/ArmCellLive.cpp") : std::string();
            auto Pin = [](const std::string& s, const char* fn, const char* call, const char* before) {   // call inside fn, ahead of `before`
                const std::size_t f = s.find(fn);
                if (f == std::string::npos) return 0;
                const std::size_t c = s.find(call, f), b = s.find(before, f);
                return (c != std::string::npos && b != std::string::npos && c < b) ? 1 : 0;
            };
            CHECK(shaped && !ws.empty() && !lv.empty(), "R165 (1) source pins: argv[1] gives the tree; tools/wb_serve.cpp and ArmCellLive.cpp read");
            const int pins =
                Pin(ws, "void W906_AlarmStopLikeGolden(const char* code)",
                    "W906_MotorAccessArmCellOnPopup(W906_ShowErrorMessage_LastKCode == 0 ? \"通知框\" : \"告警框\", code);", "W906_MotorAccessOnAlarm(code);") +
                Pin(ws, "void ForwardShowMotorErrorMessage(const char* code, int keyCode, int pos, const char* unitName, const char* message)",
                    "W906_MotorAccessArmCellOnPopup(\"馬達告警框\", code);", "ForwardShowErrorMessage(code, 0, pos);") +
                Pin(ws, "static int ForwardShowMyMessageBoxYesNo(", "W906_MotorAccessArmCellOnPopup(\"是／否框\", s1);", "PostQueryOptions(qid, \"yes-no\"") +
                Pin(ws, "void W906MbShowMyMessage(const char* s1, const char* s2, const char* s3, bool Ok, bool bServoOff)",
                    "W906_MotorAccessArmCellOnPopup(\"訊息框\", s1);", "MbWait(qid, kOpts, 2);") +
                Pin(ws, "void W906MbShowUnloaderTray(const char* s1, const char* s2)",
                    "W906_MotorAccessArmCellOnPopup(v.nonStop ? \"訊息框（不停機）\" : \"訊息框\", s1);", "MbFormShow();");
            CHECK(pins == 5, "R165 (1): every box wb_serve shows calls the Arm Cell edge first -- ShowErrorMessage (alarm + notice), the motor note, YES / NO, ShowMyMessage, ShowUnloaderTrayMessage");
            std::size_t installs = 0;
            for (std::size_t p = lv.find("g_W906ArmCellPopupUp = &ArmCellPopupUpLive;"); p != std::string::npos; p = lv.find("g_W906ArmCellPopupUp = &ArmCellPopupUpLive;", p + 1)) ++installs;
            CHECK(installs == 2 && Pin(lv, "bool W906_MotorAccessArmCellOnPopup(const char* kind, const char* what)", "ht9045::MotorAccessArmCellOnPopup(", "}") == 1 &&
                  Pin(lv, "bool ArmCellPopupUpLive(std::string& what)", "W906_FormShowing(\"fNote\", fNote->fShow)", "W906_FormShowing(\"MyMessageBox\", MyMessageBox->fShow)") == 1,
                  "R165 (1): ArmCellLive.cpp -- the live state half (golden fNote / MyMessageBox fShow) installed by the catalog and the plan, the thunk wb_serve calls");
        }
        g_W906ArmCellPopupUp = 0;

        // ---- the gates while it runs
        {
            FakeBackend b;
            Fresh(b);
            CHECK(ToS3(b, CellReq(true)), "setup: at S3");
            const int k1 = b.Calls();
            const char* teachActs[] = { "teachGo", "jogP", "moveRelative", "moveAbsolute", "home", "teachZAllUp", "teachIndexServo", "servoToggle" };
            int refused = 0;
            for (int k = 0; k < 8; ++k) {
                MotorAccessReq q = Req("uteach", teachActs[k], "MInShutte1");
                const MotorAccessOutcome r = MotorAccessDispatch(q, 160 + k, b);
                if (!r.ok && Has(r.ackJson, "Arm Cell 進行中")) ++refused; else printf("    passed the gate: uteach %s\n", teachActs[k]);
            }
            const MotorAccessOutcome r2 = MotorAccessDispatch(CellReq(true), 170, b);
            CHECK(refused == 8 && !r2.ok && Has(r2.ackJson, "已經有一個在走") && b.Calls() == k1 && Cell().active,
                  "every other Teach action (and a second Arm Cell) is refused while it runs -- in C++, not only the page lock");
            MotorAccessReq tq = TeachReq("teachSet", "SetButton030", false, false); tq.flag["query"] = true;
            const MotorAccessOutcome r3 = MotorAccessDispatch(tq, 171, b);
            CHECK(!Has(r3.ackJson, "Arm Cell 進行中") && Cell().active, "a teachSet query (read-only) passes");
            MotorAccessOutcome r = MotorAccessDispatch(Req("uMotorTest", "jogP", "MInArmY"), 172, b);
            CHECK(!r.ok && Has(r.ackJson, "MInArmY") && Has(r.ackJson, "正在用的軸"), "Motor Test jog on one of its axes -> refused");
            MotorAccessReq sv = Req("uMotorTest", "servoToggle", "MInArmX"); sv.flag["servoOn"] = false;
            r = MotorAccessDispatch(sv, 173, b);
            CHECK(!r.ok && Has(r.ackJson, "正在用的軸") && Cell().active,
                  "Motor Test servo toggle on one of its axes -> refused (STOP first); CancelJobsOnAxis stays a second lock");
            r = MotorAccessDispatch(Req("uMotorTest", "reloadMotorData", 0), 174, b);
            CHECK(!r.ok && Has(r.ackJson, "Reload Motor Data"), "Motor Test Reload Motor Data -> refused (it re-applies every motor)");
            r = MotorAccessDispatch(Req("uMotorTest", "lightScale", 0), 175, b);
            CHECK(!r.ok && Has(r.ackJson, "Light Scale"), "Motor Test Light Scale start -> refused (it homes and moves the arm)");
            MotorAccessReq lp = ReqStart("uMotorTest", "loopMove", "MInShutte1", true); lp.num["mode"] = 1;
            r = MotorAccessDispatch(lp, 176, b);
            CHECK(!r.ok && Has(r.ackJson, "All 模式"), "Motor Test All-mode loop start -> refused");
            CHECK(Cell().active && b.Calls() == k1, "... none of them sent anything or ended the job");  { MotorAccessReq sq = Req("uteach", "teachSt02", 0); sq.button = "St02Query"; sq.str["btn"] = "St02State"; sq.flag["query"] = true; const MotorAccessOutcome q1 = MotorAccessDispatch(sq, 178, b); MotorAccessReq rq = Req("uteach", "teachSt02", 0); rq.button = "St02Teach*"; rq.str["btn"] = "btnPos90InRA"; const MotorAccessOutcome q2 = MotorAccessDispatch(rq, 179, b); MotorAccessReq bq = Req("uteach", "teachSt02", 0); bq.button = "St02State"; const MotorAccessOutcome q3 = MotorAccessDispatch(bq, 180, b); CHECK(!q1.ok && !Has(q1.ackJson, "Arm Cell 進行中") && Has(q1.ackJson, "沒有安裝") && !q3.ok && !Has(q3.ackJson, "Arm Cell 進行中") && !q2.ok && Has(q2.ackJson, "Arm Cell 進行中") && b.Calls() == k1 && Cell().active, "AI(W906-ARMCELL3) St02-M Q1: while the job runs St02's state query (teachSt02 btn St02State, by params.btn or by the button) passes the busy gate -- here it reaches the St02 dispatcher (not installed in this harness) -- and a rotate button (btnPos90InRA) is still refused (Arm Cell 進行中); nothing sent, the job goes on"); }   //AI(W906-ARMCELL3) 20261003: St02-M Q1, same line
            MotorAccessDispatch(Req("uteach", "stop", 0), 177, b);
            CHECK(!Cell().active && Cell().result == "cancelled", "STOP always passes and ends it");
        }

        // ---- the runtime block: catalog + catalogRev + Q3 zDownDefault (data, never the machine-type name)
        {
            FakeBackend b;
            Fresh(b);
            ArmCellCatalog cat;
            cat.zSafePos = 50;
            ArmCellNozzleInfo nz; nz.i = 0; nz.j = 0; nz.z = Ax("MInArmZA"); nz.z.mi = iz;
            cat.nozzles[0].push_back(nz);
            cat.x[0] = Ax("MInArmX"); cat.y[0] = Ax("MInArmY"); cat.note[0] = "D1 note";
            ArmCellArea a;
            a.arm = "in"; a.id = "Loader"; a.label = "Loader"; a.v1 = true; a.installed = true; a.usable = true; a.cols = 3; a.rows = 8;
            a.zDownAllowed = true; a.zKind = "pick";
            cat.areas.push_back(a);
            ArmCellArea h = a;
            h.id = "HotPlate1"; h.label = "HotPlate 1"; h.v1 = false; h.installed = false; h.usable = false; h.why = "HotPlate 1：Using Flag 不含這一盤";
            cat.areas.push_back(h);
            b.cellHasCatalog = true; b.cellCatalog = cat;
            const std::string j1 = MotorAccessArmCellJson(b);
            cJSON* jp = cJSON_Parse(j1.c_str());
            CHECK(jp && Has(j1, "\"catalogRev\":1") && Has(j1, "\"catalog\":{\"ok\":true") && Has(j1, "\"alias\":\"MInArmZA\"") &&
                  Has(j1, "\"id\":\"HotPlate1\"") && Has(j1, "Using Flag 不含這一盤") && Has(j1, "\"zSafePos\":50") && Has(j1, "\"result\":\"\"") &&
                  Has(j1, "\"final\":null"),
                  "runtime armCell: the catalog (nozzles, areas with their reasons, ZSafePos); no job yet (result \"\", final null)");
            cJSON_Delete(jp);
            CHECK(Has(j1, "\"zDownDefault\":false") && Has(j1, "kPci1203CardOrgLogic=-1") && Has(j1, "±999999"),
                  "Q3: a PCI1203 arm Z, ORG polarity unmeasured and ±999999 soft limits -> zDownDefault false, both reasons");
            CHECK(!Has(j1, "自己變成勾") && Has(j1, "程式常數"),
                  "review m4: the Q3 reason makes no 'turns on by itself' promise (kPci1203CardOrgLogic is a constant HT9050's ORG branch never reads)");
            CHECK(Has(MotorAccessArmCellJson(b), "\"catalogRev\":1"), "the same catalog -> catalogRev unchanged (the page rebuilds only on a change)");
            b.cellCatalog.areas[1].why = "changed";
            CHECK(Has(MotorAccessArmCellJson(b), "\"catalogRev\":2"), "a changed catalog -> catalogRev + 1");
            b.golden[iz].softP = 9000; b.golden[iz].softN = -9000;
            const std::string j2 = MotorAccessArmCellJson(b);
            CHECK(Has(j2, "\"zDownDefault\":false") && Has(j2, "kPci1203CardOrgLogic=-1") && !Has(j2, "±999999"),
                  "Q3: soft limits set, the ORG polarity still unmeasured -> zDownDefault stays false (one reason left)");
            b.cellCatalog.nozzles[0][0].z.is1203 = false;
            CHECK(Has(MotorAccessArmCellJson(b), "\"zDownDefault\":true"), "Q3: no PCI1203 arm Z -> zDownDefault true");
            FakeBackend nb;
            CHECK(Has(MotorAccessArmCellJson(nb), "\"catalog\":{\"ok\":false"), "a backend without the catalog -> catalog ok false (the page greys the pane)");
        }
        g_W906OrgActiveLow = 0;
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (GEAR)
    //AI(W906-GEARRATIO) 20261002: the Motor Test "Gear Ratio" tab (RULINGS_20261002 #22; NB2 spec §5 / §8; WebMotorAccess.cpp EOF):
    //  gearCalMove's gates (every one refuses with nothing sent), the session (take-up = gauge zero, cTrue from the pulses, intact or
    //  interrupted), gearRatioPreview (the spec's 50 / 100 / 200 example, nothing written), gearRatioSave (the preview key, the Teach
    //  window, the confirmation band; Mot_Table only three cells, teach.ini only this axis' keys, backups = the old bytes, memory,
    //  ONLY this axis HomeFlag=0, no axis zeroed, the calibration log) and its three injected writer faults (all restored), the runtime
    //  block. Files: argv[2] (CMake: <build>/tests/gear_scratch), refused under the machine's folders.
    printf("[GEAR] Motor Test Gear Ratio: gearCalMove / gearRatioPreview / gearRatioSave (RULINGS_20261002 #22)\n");
    {
        std::string dir = (argc >= 3) ? std::string(argv[2]) : std::string(std::getenv("TEMP") ? std::getenv("TEMP") : ".") + "\\ht9045_gear_" + std::to_string((long long)_getpid());
        for (std::size_t i = 0; i < dir.size(); ++i) if (dir[i] == '/') dir[i] = '\\';
        std::string low = dir;
        for (std::size_t i = 0; i < low.size(); ++i) if (low[i] >= 'A' && low[i] <= 'Z') low[i] = (char)(low[i] - 'A' + 'a');
        const char* prod[] = { "d:\\ht9045\\system", "d:\\ht9045\\config", "d:\\ht9045\\inidata", "d:\\ht9045\\cfg", "d:\\ht9045\\error", "d:\\ht9045\\exe" };
        bool refused = false;
        for (int k = 0; k < 6; ++k) if (low.compare(0, std::strlen(prod[k]), prod[k]) == 0) refused = true;
        CHECK(!refused, "the Gear Ratio scratch directory is not under the machine's folders (system / config / IniData / CFG / Error / EXE)");
        if (!refused) {
            for (std::size_t i = 3; i <= dir.size(); ++i) if (i == dir.size() || dir[i] == '\\') _mkdir(dir.substr(0, i).c_str());
            const std::string teachPath = dir + "\\teach.ini", motPath = dir + "\\Mot_Table.csv", logDir = dir + "\\oplog", logFile = logDir + "\\GearRatioCal.csv";
            const std::string bakT = teachPath + ".bak_20261002_120000", bakM = motPath + ".bak_20261002_120000";
            auto Clean = [&]() {
                std::remove(teachPath.c_str()); std::remove(motPath.c_str()); std::remove(logFile.c_str());
                std::remove(bakT.c_str()); std::remove(bakM.c_str());
                for (int k = 2; k < 8; ++k) { std::remove((bakT + "_" + std::to_string(k)).c_str()); std::remove((bakM + "_" + std::to_string(k)).c_str()); }
                std::remove((motPath + ".tmp_savemot").c_str());
            };
            g_W906OrgActiveLow = []() { return true; };                         // HT9050: SensorType 1 + ORG bit low = at home
            MotorGolden g;
            g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 30000; g.softN = -30000; g.homeFlag = 1;
            g.jogHigh = 10000; g.initSpeed = 500; g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100; g.motorClass = kInitCfgMotorServo;
            MotorGolden gph = g;  gph.softP = 999999; gph.softN = -999999;      // the HT9050 placeholders
            MotorGolden gz = gph; gz.armZ = true; gz.sensorType = true;
            MotorGolden gzr = g;  gzr.indexZ = true;                            // MTestZ1 with real soft limits
            MotorGolden gsh = gph; gsh.inShuttle = 1;
            int xLoad = 0, yLoad = 0, xPlate = 0, yPlate = 0, ccd = 0, zSub = 0;
            const int ix = TestMot("MInArmX"), iy = TestMot("MInArmY"), iz = TestMot("MInArmZA"), io = TestMot("MOutArmX"), ish = TestMot("MInShuttle1"),
                      irot = TestMot("MInRotate"), iz1 = TestMot("MTestZ1");
            std::string teach0, mot0;
            auto Fresh = [&](FakeBackend& b) {
                MotorAccessResetJobs();
                Clean();
                Add1203(b, "MInArmX", 1, g); Add1203(b, "MInArmY", 2, gph); Add1203(b, "MInArmZA", 3, gz); Add1203(b, "MOutArmX", 4, gph);
                Add1203(b, "MInShuttle1", 5, gsh); Add1203(b, "MInRotate", 6, g); Add1203(b, "MTestZ1", 7, gzr);
                for (int s = 1; s <= 7; ++s) { b.svon[s] = true; b.motionIO[s] = 0x00004000ul; }
                b.gearOn = true;
                b.gearArm[ix] = 1; b.gearArm[iy] = 1; b.gearArm[io] = 2;
                b.fixedMots["inZ"] = std::vector<int>(1, iz); b.fixedMots["outZ"] = std::vector<int>();
                b.gearTeachPath = teachPath; b.motTablePath = motPath; b.gearLogDir = logDir;
                xLoad = 10000; yLoad = 5000; xPlate = 20000; yPlate = 7000; ccd = 1234; zSub = 50;
                b.gearRefs.clear();
                GearTeachRef r;
                r = GearTeachRef(); r.ptr = &xLoad;  r.list = "TechPara";     r.index = 0; r.section = "MInArmX";  r.key = "setEditInArmLoadX"; r.rawMotor = ix; b.gearRefs.push_back(r);
                r = GearTeachRef(); r.ptr = &yLoad;  r.list = "TechPara";     r.index = 1; r.section = "MInArmY";  r.key = "setEditInArmLoadY"; r.rawMotor = iy; b.gearRefs.push_back(r);
                r = GearTeachRef(); r.ptr = &xPlate; r.list = "TechTwoPara";  r.index = 0; r.side = 0; r.section = "MInArmX"; r.key = "setEditPlateX"; r.rawMotor = ix; b.gearRefs.push_back(r);
                r = GearTeachRef(); r.ptr = &yPlate; r.list = "TechTwoPara";  r.index = 0; r.side = 1; r.section = "MInArmY"; r.key = "setEditPlateY"; r.rawMotor = iy; b.gearRefs.push_back(r);
                r = GearTeachRef(); r.ptr = &zSub;   r.list = "TechSuckPara"; r.index = 0; r.side = 1; r.section = "InArmZSub"; r.key = "ZSub01"; r.rawMotor = iz; b.gearRefs.push_back(r);
                r = GearTeachRef(); r.ptr = &xLoad;  r.list = "elTeach";      r.index = 0; r.section = "InArm";    r.key = "iLoadX"; b.gearRefs.push_back(r);
                r = GearTeachRef(); r.ptr = &ccd;    r.list = "elTeach";      r.index = 1; r.section = "ArmAlignment"; r.key = "InArmAlignmentPick1_X"; b.gearRefs.push_back(r);
                teach0 = b.GearIniText(false);
                { std::ofstream f(teachPath.c_str(), std::ios::binary); f.write(teach0.data(), (std::streamsize)teach0.size()); }
                mot0 = "Motorname,Alias,SoftLimitN,SoftLimitP,BoardID,Port,IP,Direction,GearRatio,HomeDirectior,HomeHighSpeed,HomeLowSpeed,"
                       "InitSpeed,JogHighSpeed,JogLowSpeed,Rate,Enable,ServoAlarmOn,Range,1P2P,SensorType,SimulateSpeed,CardModel,Acc,Dec,"
                       "EncodeType,PickLimit,LimitLogic,In1Logic\r\n";
                const char* names[7] = { "MInArmX", "MInArmY", "MInArmZA", "MOutArmX", "MInShuttle1", "MInRotate", "MTestZ1" };
                for (int k = 0; k < 7; ++k) {
                    const MotorGolden& gg = b.golden[TestMot(names[k])];
                    mot0 += "M" + std::to_string(TestMot(names[k])) + "," + names[k] + "," + std::to_string(gg.softN) + "," + std::to_string(gg.softP) +
                            ",3," + std::to_string(k) + ",3,0,1,0,10000,1000,100,800,100,60,1,0,75,1,0,10000,PCI1203,8000,8000,0,,0,1\r\n";
                }
                { std::ofstream f(motPath.c_str(), std::ios::binary); f.write(mot0.data(), (std::streamsize)mot0.size()); }
            };
            auto GReq = [](const char* action, const char* motor) {
                MotorAccessReq r = Req("uMotorTest", action, motor);
                r.button = std::string(action) == "gearCalMove" ? "mtGearMove" : std::string(action) == "gearRatioSave" ? "mtGearSave" : "mtGearPreview";
                r.kind = std::string(action) == "gearCalMove" ? "motion" : "control";
                return r;
            };
            auto MoveReq = [&](const char* motor, double d, bool begin, double pct) {
                MotorAccessReq r = GReq("gearCalMove", motor);
                r.num["distanceMm"] = d; r.num["speedPct"] = pct; r.flag["begin"] = begin;
                return r;
            };
            auto Mv = [&](FakeBackend& b, const char* motor, double d, bool begin) { return MotorAccessDispatch(MoveReq(motor, d, begin, 10), 700, b); };
            auto Arrive = [](FakeBackend& b, int slot) {
                const MotorAccessGearCalState s = MotorAccessGearCal();
                b.cmdPos[slot] = s.lastTargetCard; b.actPos[slot] = s.lastTargetCard; b.state[slot] = 1;
            };
            auto MvA = [&](FakeBackend& b, const char* motor, int slot, double d, bool begin) {
                const MotorAccessOutcome o = Mv(b, motor, d, begin);
                if (o.ok) Arrive(b, slot);
                return o.ok;
            };
            auto Moves = [](const FakeBackend& b) { int n = 0; for (std::size_t i = 0; i < b.executed.size(); ++i) if (b.executed[i].kind == kCmdAxMoveAbs) ++n; return n; };
            auto LastMoveValue = [](const FakeBackend& b) { for (std::size_t i = b.executed.size(); i-- > 0; ) if (b.executed[i].kind == kCmdAxMoveAbs) return b.executed[i].value; return -1.0; };
            // the spec §3.2 measurement on MInArmX (real limits +-30000 = 300 mm): take-up 2 mm, 50 / 100 / 200 forward, 100 / 50 / 0 back
            auto Measure = [&](FakeBackend& b) {
                return MvA(b, "MInArmX", 1, 2, true) && MvA(b, "MInArmX", 1, 50, false) && MvA(b, "MInArmX", 1, 50, false) && MvA(b, "MInArmX", 1, 100, false) &&
                       MvA(b, "MInArmX", 1, -100, false) && MvA(b, "MInArmX", 1, -50, false) && MvA(b, "MInArmX", 1, -50, false);
            };
            auto PvReq = [&](const char* action, double k) {
                MotorAccessReq r = GReq(action, "MInArmX");
                r.num["oldRatio"] = 1.0;
                const double c[6] = { 50, 100, 200, 100, 50, 0 };
                const double spec[6] = { 49.62, 99.21, 198.43, 99.25, 49.66, 0.04 };
                std::vector<double> vc, va, vd;
                for (int i = 0; i < 6; ++i) { vc.push_back(c[i]); va.push_back(k > 0 ? c[i] * k : spec[i]); vd.push_back(i < 3 ? 1 : -1); }
                r.arr["measC"] = vc; r.arr["measA"] = va; r.arr["measDir"] = vd;
                return r;
            };
            auto KeyOf = [](const std::string& ack) {
                std::string key;
                cJSON* j = cJSON_Parse(ack.c_str());
                const cJSON* p = j ? cJSON_GetObjectItemCaseSensitive(j, "plan") : 0;
                const cJSON* k = p ? cJSON_GetObjectItemCaseSensitive(p, "previewKey") : 0;
                if (cJSON_IsString(k)) key = k->valuestring;
                cJSON_Delete(j);
                return key;
            };
            const double nr = std::strtod("0.9921524", 0);

            // ---- gearCalMove: the happy path and the session
            FakeBackend b;
            Fresh(b);
            CHECK(std::string(MotorAccessActionStatus("gearCalMove")) == "live" && std::string(MotorAccessActionStatus("gearRatioPreview")) == "live" &&
                  std::string(MotorAccessActionStatus("gearRatioSave")) == "live", "the three actions are live kActions rows");
            MotorAccessOutcome o = Mv(b, "MInArmX", 2, true);
            CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"moving\"") && Has(o.ackJson, "\"target\":200") && Has(o.ackJson, "\"cardTarget\":200") &&
                  Has(o.ackJson, "\"arriveCmdPos\":200") && Has(o.ackJson, "\"begin\":true") && Moves(b) == 1 && LastMoveValue(b) == 200.0 &&
                  LastRunSpeed(b) == MotorSpeedFromPct(10, g).velHigh,
                  "begin: the take-up 2 mm = MoveAbs 200 at 10 %, ack moving (not arrived) + target / cardTarget / arriveCmdPos");
            MotorAccessGearCalState s = MotorAccessGearCal();
            CHECK(s.active && s.motor == "MInArmX" && s.zeroUser == 200 && s.zeroCard == 200.0 && s.dirSign == 1 && s.cNom.empty() && MotorAccessJobs().gearActive,
                  "the session: zero = the take-up's target, measurement direction +, no point yet; MotorAccessJobs().gearActive");
            o = Mv(b, "MInArmX", 50, false);
            CHECK(!o.ok && Has(o.ackJson, "在走") == false && Moves(b) == 1, "the next move before the take-up arrived (cmdPos still 0) -> refused, nothing sent");
            Arrive(b, 1);
            CHECK(MvA(b, "MInArmX", 1, 50, false) && LastMoveValue(b) == 5200.0, "+50 mm -> MoveAbs 5200 (from the session's last target, not the page's number)");
            CHECK(MvA(b, "MInArmX", 1, 50, false) && MvA(b, "MInArmX", 1, 100, false) && MvA(b, "MInArmX", 1, -100, false) && MvA(b, "MInArmX", 1, -50, false) &&
                  MvA(b, "MInArmX", 1, -50, false) && LastMoveValue(b) == 200.0, "the rest of the spec's measurement: 100 / 200 forward, 100 / 50 / 0 back");
            s = MotorAccessGearCal();
            CHECK(s.cNom.size() == 6 && s.cNom[0] == 50 && s.cNom[1] == 100 && s.cNom[2] == 200 && s.cNom[3] == 100 && s.cNom[4] == 50 && s.cNom[5] == 0 &&
                  s.dir[0] == 1 && s.dir[2] == 1 && s.dir[3] == -1 && s.dir[5] == -1 && s.cTrue[2] == 200.0,
                  "six points: c 50 / 100 / 200 (+) and 100 / 50 / 0 (-); cTrue = the pulses x GearRatio (200 mm with GearRatio 1)");
            {   // GearRatio 0.071425: cTrue comes from the pulses, cNom from the page's numbers
                FakeBackend e;
                Fresh(e);
                e.golden[ix].gearRatio = 0.071425;
                CHECK(MvA(e, "MInArmX", 1, 2, true) && MvA(e, "MInArmX", 1, 50, false), "GearRatio 0.071425: take-up + 50 mm");
                const MotorAccessGearCalState t = MotorAccessGearCal();
                const double want = ((double)MotorUserToCard(5200, 0.071425) - (double)MotorUserToCard(200, 0.071425)) * 0.071425 / 100.0;
                CHECK(t.cNom.size() == 1 && t.cNom[0] == 50.0 && std::fabs(t.cTrue[0] - want) < 1e-12 && t.cTrue[0] != 50.0,
                      "GearRatio 0.071425: cNom 50 mm, cTrue = (card 5200 - card 200) x 0.071425 / 100 (the distance the pulses commanded)");
            }

            // ---- gearCalMove: every gate refuses with nothing sent
            struct Gate { const char* what; std::function<void(FakeBackend&)> arm; MotorAccessReq req; const char* expect; };
            std::vector<Gate> gates;
            gates.push_back({ "teach page source", [](FakeBackend&) {}, ([&]() { MotorAccessReq r = MoveReq("MInArmX", 2, true, 10); r.source = "uteach"; return r; })(), "教導頁沒有" });
            gates.push_back({ "a rotary axis", [](FakeBackend&) {}, MoveReq("MInRotate", 2, true, 10), "旋轉軸" });
            gates.push_back({ "a Z with +-999999 soft limits", [](FakeBackend&) {}, MoveReq("MInArmZA", 2, true, 10), "佔位" });
            gates.push_back({ "EMG", [](FakeBackend& x) { x.gearEmg = true; }, MoveReq("MInArmX", 2, true, 10), "EMG" });
            gates.push_back({ "ALM bit", [](FakeBackend& x) { x.motionIO[1] |= 0x00000002ul; }, MoveReq("MInArmX", 2, true, 10), "ALM" });
            gates.push_back({ "ERROR_STOP", [](FakeBackend& x) { x.state[1] = 3; }, MoveReq("MInArmX", 2, true, 10), "ERROR_STOP" });
            gates.push_back({ "no monitor sample (ALM unknown)", [](FakeBackend& x) { x.motionIO.erase(1); }, MoveReq("MInArmX", 2, true, 10), "fail-closed" });
            gates.push_back({ "servo OFF", [](FakeBackend& x) { x.svon[1] = false; }, MoveReq("MInArmX", 2, true, 10), "伺服沒開" });
            gates.push_back({ "servo unknown", [](FakeBackend& x) { x.svon.erase(1); }, MoveReq("MInArmX", 2, true, 10), "伺服沒開" });
            gates.push_back({ "HomeFlag 0", [&](FakeBackend& x) { x.golden[ix].homeFlag = 0; }, MoveReq("MInArmX", 2, true, 10), "尚未歸零" });
            gates.push_back({ "HomeFlag 2", [&](FakeBackend& x) { x.golden[ix].homeFlag = 2; }, MoveReq("MInArmX", 2, true, 10), "尚未歸零" });
            gates.push_back({ "speed 21 %", [](FakeBackend&) {}, MoveReq("MInArmX", 2, true, 21), "1..20" });
            gates.push_back({ "speed 0 %", [](FakeBackend&) {}, MoveReq("MInArmX", 2, true, 0), "1..20" });
            gates.push_back({ "speed 10.5 %", [](FakeBackend&) {}, MoveReq("MInArmX", 2, true, 10.5), "1..20" });
            gates.push_back({ "distance 0.005 mm", [](FakeBackend&) {}, MoveReq("MInArmX", 0.005, true, 10), "0.01 mm" });
            gates.push_back({ "distance 0", [](FakeBackend&) {}, MoveReq("MInArmX", 0, true, 10), "0.01 mm" });
            gates.push_back({ "take-up 6 mm", [](FakeBackend&) {}, MoveReq("MInArmX", 6, true, 10), "消背隙" });
            gates.push_back({ "no session", [](FakeBackend&) {}, MoveReq("MInArmX", 50, false, 10), "沒有進行中的量測" });
            gates.push_back({ "the arm's Z off its home (ORG high)", [](FakeBackend& x) { x.motionIO[3] |= 0x00000010ul; }, MoveReq("MInArmX", 2, true, 10), "不在原點" });
            gates.push_back({ "the arm's Z without a sample", [](FakeBackend& x) { x.motionIO.erase(3); }, MoveReq("MInArmX", 2, true, 10), "原點狀態不明" });
            gates.push_back({ "golden IsCanQuickJogMove says no", [](FakeBackend& x) { x.canTeach = false; }, MoveReq("MInArmX", 2, true, 10), "IsCanQuickJogMove" });
            gates.push_back({ "no arm answer (GearArmOf -1)", [](FakeBackend& x) { x.gearArm[TestMot("MInArmX")] = -1; }, MoveReq("MInArmX", 2, true, 10), "GearArmOf" });
            gates.push_back({ "the door", [&](FakeBackend& x) { x.doorOpen.insert(ix); }, MoveReq("MInArmX", 2, true, 10), "安全門" });
            gates.push_back({ "not READY", [](FakeBackend& x) { x.notReady.insert(1); }, MoveReq("MInArmX", 2, true, 10), "READY" });
            gates.push_back({ "SystemStart", [](FakeBackend& x) { x.systemStart = true; }, MoveReq("MInArmX", 2, true, 10), "SystemStart" });
            gates.push_back({ "a backend without the gear calls", [](FakeBackend& x) { x.gearOn = false; }, MoveReq("MInArmX", 2, true, 10), "急停" });
            int gateBad = 0;
            for (std::size_t k = 0; k < gates.size(); ++k) {
                FakeBackend x;
                Fresh(x);
                gates[k].arm(x);
                const int m0 = Moves(x), c0 = (int)x.executed.size();
                const MotorAccessOutcome r = MotorAccessDispatch(gates[k].req, 701, x);
                const bool okGate = !r.ok && Has(r.ackJson, gates[k].expect) && Moves(x) == m0 && !MotorAccessGearCal().active;
                if (!okGate) { ++gateBad; printf("    GATE %s: ok=%d sent=%d why=%s\n", gates[k].what, (int)r.ok, (int)x.executed.size() - c0, r.ackJson.substr(0, 240).c_str()); }
                else printf("    gate %s -> refused (%s)\n", gates[k].what, gates[k].expect);
            }
            CHECK(gateBad == 0, "gearCalMove: every gate refuses with nothing moved (rotary, Z placeholder, EMG, ALM, ERROR_STOP, no sample, servo, HomeFlag, speed, distance, take-up, no session, arm Z off home / unknown, golden teach interlock, GearArmOf, door, READY, SystemStart, no backend)");
            {   // a hand-taught axis (servo off + pushed -> g_encBase, W5B-6): its command position may not be where the encoder is -> refused,
                //   and the runtime axis list greys it with the reason (control: the same axis is eligible before the hand teach)
                FakeBackend x;
                Fresh(x);
                x.golden[iz].softP = 30000; x.golden[iz].softN = -30000;          // MInArmZA with real limits: past the Z placeholder rule
                const std::vector<std::string> al(1, "MInArmZA");
                CHECK(Has(MotorAccessGearCalJson(x, al), "\"eligible\":true"), "control: MInArmZA with real soft limits is eligible before the hand teach");
                x.reg = BuildTeachRegistry(std::set<std::string>());
                x.golden[iz].servoAlarmOn = true;                                 // golden PServoAlarmOn -> encoder base after the hand teach
                const MotorAccessOutcome hs = MotorAccessDispatch(TeachReq("teachSet", "SetButton030", true, true), 702, x);
                MotorAccessReq he = TeachReq("teachSet", "SetButton030", true, false); he.flag["accept"] = true;
                const MotorAccessOutcome hd = MotorAccessDispatch(he, 703, x);
                CHECK(hs.ok && hd.ok && MotorAccessEncoderBase(3), "setup: a hand teach on MInArmZA leaves its 1203 axis on the encoder base");
                x.cmdPos[3] = 0.0; x.actPos[3] = 3000.0;                         // pushed 30 mm with the servo off; the command position stayed
                const int m0 = Moves(x);
                const MotorAccessOutcome r = Mv(x, "MInArmZA", 2, true);
                CHECK(!r.ok && Has(r.ackJson, "編碼器基準") && Moves(x) == m0 && !MotorAccessGearCal().active,
                      "a hand-taught axis (encoder base) -> gearCalMove refused (the take-up would travel 32 mm, not 2), nothing moved, no session");
                const std::string j1 = MotorAccessGearCalJson(x, al);
                CHECK(Has(j1, "\"eligible\":false") && Has(j1, "編碼器基準"), "the runtime axis list greys the hand-taught axis with the reason");
            }
            {   // the placeholder caps (MInArmY: +-999999) and the real limits (MInArmX: +-30000)
                FakeBackend x;
                Fresh(x);
                CHECK(MvA(x, "MInArmY", 2, 2, true), "placeholder axis: the take-up is allowed");
                const int m0 = Moves(x);
                o = Mv(x, "MInArmY", 60, false);
                CHECK(!o.ok && Has(o.ackJson, "50") && Has(o.ackJson, "±999999") && Moves(x) == m0, "placeholder soft limits: a 60 mm step -> refused (<= 50 mm)");
                //AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (a) -- the 100 mm cap counts from the session's START point (spec §5.1), the take-up
                //  included: was "50 + 50 = 100 mm from the zero is allowed" / "101 mm from the zero -> refused" (= up to 105 mm from the start)
                CHECK(MvA(x, "MInArmY", 2, 50, false) && MvA(x, "MInArmY", 2, 48, false), "placeholder: take-up 2 + 50 + 48 = 100 mm from the start is allowed");
                o = Mv(x, "MInArmY", 0.01, false);
                CHECK(!o.ok && Has(o.ackJson, "100.00") && Has(o.ackJson, "起點") && Has(o.ackJson, "100.01") && Moves(x) == m0 + 2,
                      "R171 L1 (a): 100.01 mm from the start (98.01 from the zero) -> refused, the text names the start");
                FakeBackend x2;
                Fresh(x2);
                CHECK(MvA(x2, "MInArmY", 2, 2, true) && MvA(x2, "MInArmY", 2, 50, false), "placeholder: take-up 2 + 50");
                const int m2 = Moves(x2);
                o = Mv(x2, "MInArmY", 50, false);
                CHECK(!o.ok && Has(o.ackJson, "起點") && Has(o.ackJson, "102.00") && Moves(x2) == m2, "R171 L1 (a): 50 + 50 from the zero = 102 mm from the start -> refused (the old rule let it go)");
                FakeBackend x3;
                Fresh(x3);
                CHECK(MvA(x3, "MInArmY", 2, -2, true) && MvA(x3, "MInArmY", 2, -50, false) && MvA(x3, "MInArmY", 2, -48, false), "placeholder, the minus direction: take-up -2, -50, -48 = 100 mm from the start");
                const int m3 = Moves(x3);
                o = Mv(x3, "MInArmY", -0.01, false);
                CHECK(!o.ok && Has(o.ackJson, "起點") && Moves(x3) == m3, "R171 L1 (a): the minus direction, 100.01 mm from the start -> refused");
                FakeBackend y;
                Fresh(y);
                CHECK(MvA(y, "MInArmX", 1, 2, true) && MvA(y, "MInArmX", 1, 60, false), "real soft limits: a 60 mm step is allowed (the soft limits bound it)");
                o = Mv(y, "MInArmX", 240, false);
                CHECK(!o.ok && Has(o.ackJson, "正向軟體極限"), "real soft limits: a target past SoftLimitP (golden's >=) -> refused by MoveBlock1203");
                FakeBackend z;
                Fresh(z);
                CHECK(MvA(z, "MTestZ1", 7, 2, true), "a Z with REAL soft limits (MTestZ1 +-30000) is open");
            }
            {   //AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (b) -- Start (begin) while a session is active is refused (it used to replace the
                //  session: a new zero where the last leg stopped, so Start after a +100 mm leg gave another 100 mm); STOP ends it, then Start works
                FakeBackend x;
                Fresh(x);
                CHECK(MvA(x, "MInArmX", 1, 2, true) && MvA(x, "MInArmX", 1, 50, false), "a session with one point");
                const MotorAccessGearCalState s1 = MotorAccessGearCal();
                const int m0 = Moves(x);
                o = Mv(x, "MInArmX", 2, true);
                const MotorAccessGearCalState s2 = MotorAccessGearCal();
                CHECK(!o.ok && Has(o.ackJson, "量測進行中") && Has(o.ackJson, "STOP") && Moves(x) == m0 && s2.active && s2.id == s1.id && s2.zeroUser == s1.zeroUser && s2.cNom.size() == 1,
                      "R171 L1 (b): Start (begin) while the session is active -> refused, nothing moved, the session kept (same id, zero and point)");
                o = Mv(x, "MOutArmX", 2, true);
                CHECK(!o.ok && Has(o.ackJson, "量測進行中") && Has(o.ackJson, "MInArmX") && Moves(x) == m0, "R171 L1 (b): Start on another axis while MInArmX's session is active -> refused too (one session at a time)");
                MotorAccessReq st = Req("uMotorTest", "stop", "MInArmX");
                st.button = "btnStop";
                CHECK(MotorAccessDispatch(st, 705, x).ok && !MotorAccessGearCal().active, "STOP ends the session");
                CHECK(MvA(x, "MInArmX", 1, 2, true) && MotorAccessGearCal().active && MotorAccessGearCal().id == s1.id + 1 && Moves(x) == m0 + 1,
                      "R171 L1 (b): after STOP, Start opens a new session (the next id)");
            }
            {   // interrupted: another command on the axis, stopped short, not READY, a stale sample
                FakeBackend x;
                Fresh(x);
                CHECK(MvA(x, "MInArmX", 1, 2, true) && MvA(x, "MInArmX", 1, 50, false), "a session with one point");
                MotorAccessReq mr = Req("uMotorTest", "moveRelative", "MInArmX");
                mr.button = "sbMotorTest_MoveP"; mr.kind = "motion"; mr.num["interval"] = 100;
                CHECK(MotorAccessDispatch(mr, 702, x).ok, "(a Motor Test MoveP on the same axis goes out)");
                x.cmdPos[1] = 5200;                                             // even back at the session's place
                const int m0 = Moves(x);
                o = Mv(x, "MInArmX", 50, false);
                CHECK(!o.ok && Has(o.ackJson, "打斷") && Moves(x) == m0, "another command on the axis since the last gear move -> refused (interrupted), even at the right place");
                FakeBackend y;
                Fresh(y);
                CHECK(MvA(y, "MInArmX", 1, 2, true) && MvA(y, "MInArmX", 1, 50, false), "a session with one point");
                y.cmdPos[1] = 4000;                                             // stopped short (STOP after the ack, say)
                o = Mv(y, "MInArmX", 50, false);
                CHECK(!o.ok && Has(o.ackJson, "不在這次量測最後送的位置"), "the axis not at the last target -> refused");
                y.cmdPos[1] = 5200; y.state[1] = 5;
                o = Mv(y, "MInArmX", 50, false);
                CHECK(!o.ok && Has(o.ackJson, "還在走"), "the last move not finished (not READY) -> refused (wait)");
                y.state[1] = 1;
                y.freezePolls = true;                                           // the monitor stops polling: the next command's sample never comes
                CHECK(Mv(y, "MInArmX", 50, false).ok, "(a move while the monitor is frozen)");
                Arrive(y, 1);
                o = Mv(y, "MInArmX", 50, false);
                CHECK(!o.ok && Has(o.ackJson, "監看器"), "no fresh sample after the last command -> refused (wait)");
                y.freezePolls = false;
            }
            {   // what ends a session
                //AI(W906-GEARRATIO2) 20261003: NB2 R171 M1 -- every end but a save also stops the session's OWN axis (slot 1, MInArmX)
                //  once, with Stop1203 (kCmdAxStop + ExtDrive 0) as the Arm Cell job does. `sweep` = the trigger stops every opened axis
                //  anyway (STOP / an alarm: golden StopAllMotor; FormShow with the relay off: GaliMotorServoOff) -> the session's axis
                //  gets exactly one stop more than slot 2 (MInArmY, not in the session); every other trigger stops slot 1 only.
                struct End { const char* what; std::function<void(FakeBackend&)> act; bool sweep; };
                std::vector<End> ends;
                ends.push_back({ "STOP", [](FakeBackend& x) { MotorAccessReq r = Req("uMotorTest", "stop", "MInArmX"); r.button = "btnStop"; MotorAccessDispatch(r, 703, x); }, true });
                ends.push_back({ "selectMotor", [](FakeBackend& x) { MotorAccessReq r = Req("uMotorTest", "selectMotor", "MInArmY"); r.button = "labName"; r.kind = "edit"; MotorAccessDispatch(r, 703, x); }, false });
                ends.push_back({ "the operator gone", [](FakeBackend& x) { MotorAccessTick(x, 500, false); }, false });
                ends.push_back({ "the operator gone, seen after a Poll", [](FakeBackend& x) { MotorAccessPollTick(x, false); }, false });
                ends.push_back({ "the safe lock", [](FakeBackend& x) { x.safeLock = true; MotorAccessTick(x, 500, true); }, false });
                ends.push_back({ "an alarm", [](FakeBackend& x) { MotorAccessOnAlarm(x, "JAM9999"); }, true });
                ends.push_back({ "Motor Test FormClose", [](FakeBackend& x) { MotorAccessReq r = Req("uMotorTest", "formClose", 0); r.button = "FormClose"; MotorAccessDispatch(r, 703, x); }, false });
                ends.push_back({ "Motor Test FormShow (relay off)", [](FakeBackend& x) { MotorAccessReq r = Req("uMotorTest", "formShow", 0); r.button = "FormShow"; MotorAccessDispatch(r, 703, x); }, true });
                ends.push_back({ "a servo toggle on its axis", [](FakeBackend& x) { MotorAccessReq r = Req("uMotorTest", "servoToggle", "MInArmX"); r.button = "btnServoOff"; r.flag["servoOn"] = true; MotorAccessDispatch(r, 703, x); }, false });
                ends.push_back({ "Motor Power", [](FakeBackend& x) { MotorAccessReq r = Req("uMotorTest", "motorPowerToggle", 0); r.button = "btnMotorPower"; MotorAccessDispatch(r, 703, x); }, false });
                ends.push_back({ "the engine's AllBtnUp (VerifyMotorAction)", [](FakeBackend& x) { MotorAccessAllBtnUp(x, "VerifyMotorAction"); }, false });
                auto StopsOn = [](const FakeBackend& b, std::size_t from, int slot) {
                    int n = 0;
                    for (std::size_t i = from; i < b.executed.size(); ++i) if (b.executed[i].kind == kCmdAxStop && b.executed[i].axis == slot) ++n;
                    return n;
                };
                int endBad = 0, stopBad = 0;
                std::map<std::string, int> ownStops;
                for (std::size_t k = 0; k < ends.size(); ++k) {
                    FakeBackend x;
                    Fresh(x);
                    MvA(x, "MInArmX", 1, 2, true);
                    const std::size_t s0 = x.executed.size();
                    ends[k].act(x);
                    const MotorAccessGearCalState es = MotorAccessGearCal();
                    const int own = StopsOn(x, s0, 1), other = StopsOn(x, s0, 2);
                    ownStops[ends[k].what] = own;
                    if (es.active) { ++endBad; printf("    END %s: the session is still active\n", ends[k].what); }
                    else printf("    %s ends it: %s (stops: its axis %d, MInArmY %d)\n", ends[k].what, es.why.c_str(), own, other);
                    if (own != other + 1 || (!ends[k].sweep && other != 0) || !Has(es.why, "已停這一軸")) {
                        ++stopBad;
                        printf("    STOP %s: its axis %d stop(s), MInArmY %d, why \"%s\"\n", ends[k].what, own, other, es.why.c_str());
                    }
                }
                CHECK(endBad == 0, "the session ends on STOP, a motor selection, the operator gone (beat / after a Poll), the safe lock, an alarm, FormClose, FormShow, a servo toggle on its axis, Motor Power, the engine's AllBtnUp");
                CHECK(stopBad == 0, "R171 M1: each of those ends stops the session's own axis once (Stop1203) and no other axis -- STOP / an alarm / FormShow, which stop every opened axis anyway, give it exactly one stop more; the why says so");
                CHECK(ownStops["the safe lock"] == 1 && ownStops["the operator gone"] == 1 && ownStops["the operator gone, seen after a Poll"] == 1,
                      "R171 M1: x.safeLock=true; MotorAccessTick -> 1 stop on the gear axis (was 0); the link lost (beat / after a Poll) -> 1");
                FakeBackend x;
                Fresh(x);
                MvA(x, "MInArmX", 1, 2, true);
                const std::size_t sk = x.executed.size();
                MotorAccessTick(x, 500, true);
                CHECK(MotorAccessGearCal().active && StopsOn(x, sk, 1) == 0, "an ordinary beat (operator there, no safe lock) keeps it, nothing stopped");
                {   // R171 M1: when the stop is NOT sent
                    FakeBackend y;
                    Fresh(y);
                    MvA(y, "MInArmX", 1, 2, true);
                    MotorAccessReq mr = Req("uMotorTest", "moveRelative", "MInArmX");
                    mr.button = "sbMotorTest_MoveP"; mr.kind = "motion"; mr.num["interval"] = 100;
                    const bool moved = MotorAccessDispatch(mr, 704, y).ok;
                    const std::size_t s1 = y.executed.size();
                    y.safeLock = true;
                    MotorAccessTick(y, 500, true);
                    CHECK(moved && !MotorAccessGearCal().active && StopsOn(y, s1, 1) == 0 && !Has(MotorAccessGearCal().why, "已停"),
                          "R171 M1: a Motor Test move on the axis after the last gear move -> the safe lock ends the session, no stop (that move is not the session's leg; golden lets it run)");
                    FakeBackend z;
                    Fresh(z);
                    MvA(z, "MInArmX", 1, 2, true);
                    z.systemStart = true;
                    const std::size_t s2 = z.executed.size();
                    MotorAccessTick(z, 500, false);
                    CHECK(!MotorAccessGearCal().active && StopsOn(z, s2, 1) == 0, "R171 M1: SystemStart -> the session ends without a stop from it (MainProc owns the axes)");
                    FakeBackend w;
                    Fresh(w);
                    MvA(w, "MInArmX", 1, 2, true);
                    w.rejectKinds.insert((int)kCmdAxStop);
                    w.safeLock = true;
                    MotorAccessTick(w, 500, true);
                    CHECK(!MotorAccessGearCal().active && Has(MotorAccessGearCal().why, "停軸被拒"), "R171 M1: the card refuses the stop -> the session's why says so (the page shows it)");
                }
            }

            // ---- gearRatioPreview: the spec example, nothing written
            FakeBackend p;
            Fresh(p);
            CHECK(Measure(p), "the spec's measurement on MInArmX (session)");
            const int pm0 = Moves(p);
            o = MotorAccessDispatch(PvReq("gearRatioPreview", 0), 704, p);
            printf("    preview: %.900s\n", o.ackJson.c_str());
            CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"preview\"") && Has(o.ackJson, "\"level\":\"ok\"") &&
                  Has(o.ackJson, "\"newRatioText\":\"0.9921524\"") && Has(o.ackJson, "\"canSave\":true") && Has(o.ackJson, "\"sessionMatched\":true"),
                  "preview: k = 0.9921524 (7 digits), level ok, the session matched, canSave");
            CHECK(Has(o.ackJson, "\"where\":\"TechPara[0] [MInArmX] setEditInArmLoadX\",\"old\":10000,\"new\":9922") &&
                  Has(o.ackJson, "\"where\":\"TechTwoPara[0][0] [MInArmX] setEditPlateX\",\"old\":20000,\"new\":19843") &&
                  !Has(o.ackJson, "setEditInArmLoadY\",\"old\"") && !Has(o.ackJson, "setEditPlateY\",\"old\"") && Has(o.ackJson, "InArmAlignmentPick1_X") &&
                  Has(o.ackJson, "\"softP\":{\"old\":30000,\"new\":29765,\"placeholder\":false}") && Has(o.ackJson, "\"softN\":{\"old\":-30000,\"new\":-29765"),
                  "preview: old -> new for MInArmX's teach points (TechPara + its elTeach twin once; the Plate X side only), soft limits 30000 -> 29765, the elTeach-only value listed");
            CHECK(Has(o.ackJson, "MotorTest.ini [M") && Has(o.ackJson, "offset 表") && std::fabs(NumIn(o.ackJson, "backlashMm") - 0.04) < 1e-9,
                  "preview: the manual-check list and the notes; the lost motion 0.04 mm");
            CHECK(ReadAll(teachPath) == teach0 && ReadAll(motPath) == mot0 && !FileExists(bakT) && !FileExists(bakM) && p.gearSetMotorCalls == 0 &&
                  p.gearSetTeachCalls == 0 && p.gearSaveCalls == 0 && xLoad == 10000 && Moves(p) == pm0 && MotorAccessGearCal().active,
                  "preview writes nothing: files, backups, memory, no move, the session stays");
            const std::string key = KeyOf(o.ackJson);
            CHECK(key.size() == 16, "the preview key (16 hex digits)");
            {
                MotorAccessReq bad = PvReq("gearRatioPreview", 0); bad.num["oldRatio"] = 1.000001;
                CHECK(!MotorAccessDispatch(bad, 704, p).ok, "preview with an oldRatio that is not MOT's -> refused");
                MotorAccessReq nm = PvReq("gearRatioPreview", 0); nm.arr["measC"][1] = 101;   // a point the session never reached
                const MotorAccessOutcome q = MotorAccessDispatch(nm, 704, p);
                CHECK(q.ok && Has(q.ackJson, "\"sessionMatched\":false") && Has(q.ackJson, "\"canSave\":false"), "preview with a point the session never reached -> still computed, canSave false");
                MotorAccessReq ar = PvReq("gearRatioPreview", 0); ar.arr["measA"].pop_back();
                CHECK(!MotorAccessDispatch(ar, 704, p).ok, "measC / measA / measDir of different lengths -> refused");
                p.gearTeachOpen = true;
                const MotorAccessOutcome t = MotorAccessDispatch(PvReq("gearRatioPreview", 0), 704, p);
                CHECK(t.ok && Has(t.ackJson, "\"teachWindowOpen\":true") && Has(t.ackJson, "\"canSave\":false"), "the Teach window open -> the preview says canSave false");
                p.gearTeachOpen = false;
            }

            // ---- gearRatioSave: refusals first (nothing written)
            auto SvReq = [&](const std::string& k, double kk) { MotorAccessReq r = PvReq("gearRatioSave", kk); if (!k.empty()) r.str["previewKey"] = k; return r; };
            auto Untouched = [&](const FakeBackend& x) {
                return ReadAll(teachPath) == teach0 && ReadAll(motPath) == mot0 && !FileExists(bakT) && !FileExists(bakM) && x.gearSetMotorCalls == 0 &&
                       x.gearSaveCalls == 0 && xLoad == 10000 && x.homeFlags.empty();
            };
            o = MotorAccessDispatch(SvReq("", 0), 705, p);
            CHECK(!o.ok && Has(o.ackJson, "previewKey") && Untouched(p), "save without the preview key -> refused, nothing written");
            o = MotorAccessDispatch(SvReq("0123456789abcdef", 0), 705, p);
            CHECK(!o.ok && Has(o.ackJson, "預覽之後資料變了") && Untouched(p), "save with another key -> refused");
            p.gearTeachOpen = true;
            o = MotorAccessDispatch(SvReq(key, 0), 705, p);
            CHECK(!o.ok && Has(o.ackJson, "教導頁開著") && Untouched(p), "save while the Teach window is open -> refused (its next save would put the old values back)");
            p.gearTeachOpen = false;
            xLoad = 10001;
            o = MotorAccessDispatch(SvReq(key, 0), 705, p);
            CHECK(!o.ok && Has(o.ackJson, "預覽之後資料變了"), "one of THIS axis' teach values changed after the preview -> the key differs -> refused");
            xLoad = 10000;
            p.golden[ix].homeFlag = 0;
            o = MotorAccessDispatch(SvReq(key, 0), 705, p);
            CHECK(!o.ok && Has(o.ackJson, "HomeFlag") && p.gearSaveCalls == 0, "HomeFlag lost since the measurement -> refused");
            p.golden[ix].homeFlag = 1;

            // ---- gearRatioSave: the real thing
            const std::size_t svStop0 = p.executed.size();   //AI(W906-GEARRATIO2) 20261003: R171 M1 -- a save is the normal end: it stops nothing
            o = MotorAccessDispatch(SvReq(key, 0), 707, p);
            printf("    save: %.700s\n", o.ackJson.c_str());
            CHECK(o.ok && CountKind(p, svStop0, kCmdAxStop) == 0, "R171 M1: the save (the normal end of a session) sends no stop");
            CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"saved\"") && Has(o.ackJson, "\"newText\":\"0.9921524\"") &&
                  Has(o.ackJson, "\"homeFlag\":0") && Has(o.ackJson, "200 mm"), "save: ok, the new ratio, HomeFlag 0, the re-home + 200 mm check line");
            {
                std::vector<std::string> ol = Lines2(mot0), nl = Lines2(ReadAll(motPath));
                int diff = 0; std::string row;
                for (std::size_t i = 0; i < ol.size() && i < nl.size(); ++i) if (ol[i] != nl[i]) { ++diff; row = nl[i]; }
                CHECK(ol.size() == nl.size() && diff == 1 && Has(row, "MInArmX,-29765,29765,") && Has(row, ",0,0.9921524,0,") && ReadAll(bakM) == mot0,
                      "Mot_Table.csv: one line changed (MInArmX: GearRatio 0.9921524, SoftLimitN -29765, SoftLimitP 29765), the backup = the old bytes");
            }
            {
                const std::vector<GearIniDelta> d = GearIniDiff(teach0, ReadAll(teachPath));
                int ok3 = 0, other = 0;
                for (std::size_t i = 0; i < d.size(); ++i) {
                    if ((d[i].key == "setEditInArmLoadX" && d[i].newV == "9922") || (d[i].key == "iLoadX" && d[i].newV == "9922") ||
                        (d[i].key == "setEditPlateX" && d[i].newV == "19843")) ++ok3;
                    else ++other;
                }
                CHECK(ok3 == 3 && other == 0 && ReadAll(bakT) == teach0, "teach.ini: exactly MInArmX's three keys changed (the elTeach twin too), the backup = the old bytes");
            }
            CHECK(p.golden[ix].gearRatio == nr && p.golden[ix].softP == 29765 && p.golden[ix].softN == -29765 && xLoad == 9922 && xPlate == 19843 &&
                  yLoad == 5000 && yPlate == 7000 && zSub == 50 && ccd == 1234, "memory: MOT GearRatio / soft limits and MInArmX's variables new; every other variable as it was");
            CHECK(p.homeFlags.size() == 1 && p.homeFlags[ix] == 0 && p.golden[iy].homeFlag == 1 && p.golden[iz].homeFlag == 1 && p.clearAllHomeFlagsCalls == 0,
                  "ONLY this axis HomeFlag=0 (GoldenSetHomeFlag once, for MInArmX); no other flag, no ClearAllHomeFlags (NB2 spec: never reloadMotorData)");
            CHECK(CountKind(p, 0, kCmdAxSetCmdPos) == 0 && CountKind(p, 0, kCmdAxSetActPos) == 0 && p.reloadCalls == 0, "no axis zeroed (no SetCmdPos / SetActPos), no Reload Motor Data");
            CHECK(!p.motSaved.empty() && Has(p.motSaved.back(), "MInArmX|") && Has(p.motSaved.back(), "GearRatio=0.9921524;") && Has(p.motSaved.back(), "SoftLimitP=29765;"),
                  "HSys.MotTable's row := the file (MotTableRowSaved with the new cells)");
            CHECK(!MotorAccessGearCal().active && Has(MotorAccessGearCal().why, "已存檔"), "the session is used up");
            {
                const std::string lg = ReadAll(logFile);
                CHECK(Has(lg, "time,operator,motor") && Has(lg, ",fake operator,MInArmX,") && Has(lg, "0.9921524") && Has(lg, bakM) && Lines2(lg).size() == 2,
                      "GearRatioCal.csv: header + one line (operator, motor, ratios, the backups)");
            }
            o = MotorAccessDispatch(SvReq(key, 0), 708, p);
            CHECK(!o.ok, "the same save again -> refused (no session, the ratio changed)");
            {   // (the session is the dispatcher's, one at a time: a fresh backend here starts a new one)
                FakeBackend q;
                Fresh(q);
                CHECK(Measure(q), "a second session");
                const std::string k2 = KeyOf(MotorAccessDispatch(PvReq("gearRatioPreview", 0), 706, q).ackJson);
                MotorAccessReq mr = Req("uMotorTest", "moveRelative", "MInArmX");
                mr.button = "sbMotorTest_MoveP"; mr.kind = "motion"; mr.num["interval"] = 10;
                MotorAccessDispatch(mr, 706, q);
                q.cmdPos[1] = 200;
                o = MotorAccessDispatch(SvReq(k2, 0), 706, q);
                CHECK(!o.ok && Has(o.ackJson, "打斷") && Untouched(q), "a command on the axis after the measurement -> save refused (interrupted), nothing written");
            }

            // ---- the confirmation band (2 % < |k-1| <= 10 %)
            {
                FakeBackend q;
                Fresh(q);
                CHECK(Measure(q), "a session for the 3 % case");
                const MotorAccessOutcome pv = MotorAccessDispatch(PvReq("gearRatioPreview", 0.97), 709, q);
                CHECK(pv.ok && Has(pv.ackJson, "\"level\":\"confirm\"") && Has(pv.ackJson, "\"needConfirm\":true") && Has(pv.ackJson, "EastSun"),
                      "k = 0.97: preview level confirm + EastSun's warning");
                const std::string k3 = KeyOf(pv.ackJson);
                o = MotorAccessDispatch(SvReq(k3, 0.97), 709, q);
                CHECK(!o.ok && Has(o.ackJson, "confirmLarge") && Untouched(q), "k = 0.97 without confirmLarge -> refused, nothing written");
                MotorAccessReq ok2 = SvReq(k3, 0.97); ok2.flag["confirmLarge"] = true;
                o = MotorAccessDispatch(ok2, 709, q);
                CHECK(o.ok && Has(o.ackJson, "\"newText\":\"0.97\""), "k = 0.97 with confirmLarge -> saved, ratio 0.97");
            }

            {   //AI(W906-GEARRATIO2) 20261003: NB2 R171 L5 -- a +-999999 axis (MInArmY): the check asked for after the save is one this tab
                //  can do (it goes at most 100 mm from the start there), not "walk 200 mm again"
                FakeBackend q;
                Fresh(q);
                CHECK(MvA(q, "MInArmY", 2, 2, true) && MvA(q, "MInArmY", 2, 50, false) && MvA(q, "MInArmY", 2, 45, false) &&
                      MvA(q, "MInArmY", 2, -45, false) && MvA(q, "MInArmY", 2, -50, false), "MInArmY (placeholder limits): take-up 2, 50 / 95 forward, 50 / 0 back");
                auto YReq = [](const char* action) {
                    MotorAccessReq r = Req("uMotorTest", action, "MInArmY");
                    r.button = std::string(action) == "gearRatioSave" ? "mtGearSave" : "mtGearPreview";
                    r.kind = "control";
                    r.num["oldRatio"] = 1.0;
                    const double c[4] = { 50, 95, 50, 0 }, a[4] = { 49.75, 94.525, 49.75, 0.0 }, d[4] = { 1, 1, -1, -1 };
                    for (int i = 0; i < 4; ++i) { r.arr["measC"].push_back(c[i]); r.arr["measA"].push_back(a[i]); r.arr["measDir"].push_back(d[i]); }
                    return r;
                };
                const MotorAccessOutcome pv = MotorAccessDispatch(YReq("gearRatioPreview"), 711, q);
                CHECK(pv.ok && Has(pv.ackJson, "\"canSave\":true") && Has(pv.ackJson, "\"placeholder\":true") && Has(pv.ackJson, "最多 100 mm") && !Has(pv.ackJson, "200 mm"),
                      "R171 L5: preview on a placeholder axis -- the soft limits say placeholder, the note asks for a check the tab can do (<= 100 mm from the start), no 200 mm");
                MotorAccessReq sv = YReq("gearRatioSave");
                sv.str["previewKey"] = KeyOf(pv.ackJson);
                const MotorAccessOutcome so = MotorAccessDispatch(sv, 712, q);
                CHECK(so.ok && Has(so.ackJson, "\"result\":\"saved\"") && Has(so.ackJson, "最多 100 mm") && !Has(so.ackJson, "200 mm"),
                      "R171 L5: save on a placeholder axis -- its message and next ask for the possible check, no 200 mm");
            }

            // ---- three writer faults: every one restored (both files byte-identical, memory back, no HomeFlag touched)
            //AI(W906-GEARRATIO2) 20261003: + teachHint -- keys outside the plan (what the real writer does to a teach.ini this Teach page
            //  never saved: ctest GearTeachSave) get the actionable refusal (kGearTeachNotNormalized) before the detail list; the others not,
            //  incl. a 4th fault: a PLAN key written with a value the plan did not ask for (a Teach-page save would not cure that)
            struct Fault { const char* what; std::function<void(FakeBackend&)> arm; const char* expect; bool teachHint; };
            std::vector<Fault> faults;
            faults.push_back({ "the teach writer fails", [](FakeBackend& x) { x.gearWriterFail = true; }, "teach.ini 寫不了", false });
            faults.push_back({ "the writer misses a changed key (ReadFile reads the old value)", [](FakeBackend& x) { x.gearWriterSkipKey = "setEditPlateX"; }, "教導值不對", false });
            faults.push_back({ "the writer changes a key it must not", [](FakeBackend& x) { x.gearWriterExtraKey = "setEditInArmLoadY"; }, "不該變的鍵", true });
            faults.push_back({ "the writer writes a plan key with a value the plan did not ask for", [](FakeBackend& x) { x.gearWriterWrongKey = "setEditPlateX"; }, "不該變的鍵", false });
            int faultBad = 0, hintBad = 0;
            std::string outOfPlanAck;
            for (std::size_t k = 0; k < faults.size(); ++k) {
                FakeBackend x;
                Fresh(x);
                Measure(x);
                const std::string kk = KeyOf(MotorAccessDispatch(PvReq("gearRatioPreview", 0), 710, x).ackJson);
                faults[k].arm(x);
                const MotorAccessOutcome r = MotorAccessDispatch(SvReq(kk, 0), 710, x);
                const bool restored = !r.ok && Has(r.ackJson, faults[k].expect) && Has(r.ackJson, "還原") && ReadAll(teachPath) == teach0 && ReadAll(motPath) == mot0 &&
                                      x.golden[ix].gearRatio == 1.0 && x.golden[ix].softP == 30000 && x.golden[ix].softN == -30000 && xLoad == 10000 && xPlate == 20000 &&
                                      yLoad == 5000 && x.homeFlags.empty() && !x.motSaved.empty() && Has(x.motSaved.back(), "MInArmX,-30000,30000,") && MotorAccessGearCal().active;
                if (!restored) { ++faultBad; printf("    FAULT %s: ok=%d why=%s xLoad=%d xPlate=%d yLoad=%d ratio=%g\n", faults[k].what, (int)r.ok, r.ackJson.substr(0, 300).c_str(), xLoad, xPlate, yLoad, x.golden[ix].gearRatio); }
                else printf("    fault %s -> refused + restored\n", faults[k].what);
                const bool hint = Has(r.ackJson, "還沒有用目前版本的教導頁存過檔") && Has(r.ackJson, "請先在教導頁按一次「存檔」") && Has(r.ackJson, "什麼都沒改");
                if (hint != faults[k].teachHint) { ++hintBad; printf("    HINT %s: the Teach-page-save hint %s\n", faults[k].what, hint ? "given (should not be)" : "missing"); }
                if (faults[k].teachHint) outOfPlanAck = r.ackJson;
            }
            CHECK(faultBad == 0, "save faults (writer fails / misses a key / changes another key / writes a plan key wrong): refused, both files byte-identical to the backups, memory and HSys row back, no HomeFlag touched, the session kept");
            {
                const std::size_t hintAt = outOfPlanAck.find("請先在教導頁按一次「存檔」"), listAt = outOfPlanAck.find("明細：teach.ini 有 1 個不該變的鍵變了");
                CHECK(hintBad == 0 && hintAt != std::string::npos && listAt != std::string::npos && hintAt < listAt && Has(outOfPlanAck, "[MInArmY] setEditInArmLoadY 5000 → 4242"),
                      "R171 M2 follow-up: keys outside the plan -> the refusal says FIRST that teach.ini was not saved by this Teach page yet (press its Save once, retry; nothing changed, all restored), then the unchanged detail list; the other three faults (incl. a plan key written with a wrong value) do not say it");
            }

            // ---- the runtime block
            {
                FakeBackend x;
                Fresh(x);
                MvA(x, "MInArmX", 1, 2, true);
                std::vector<std::string> al;
                const char* names[8] = { "MInArmX", "MInArmY", "MInArmZA", "MOutArmX", "MInShuttle1", "MInRotate", "MTestZ1", "MNoSuch" };
                for (int k = 0; k < 8; ++k) al.push_back(names[k]);
                const std::string j = MotorAccessGearCalJson(x, al);
                cJSON* jp = cJSON_Parse(j.c_str());
                CHECK(jp != 0, "runtime gearCal: JSON");
                std::map<std::string, bool> el;
                const cJSON* axes = jp ? cJSON_GetObjectItemCaseSensitive(jp, "axes") : 0;
                const cJSON* e = 0;
                if (cJSON_IsArray(axes)) cJSON_ArrayForEach(e, axes) {
                    const cJSON* m = cJSON_GetObjectItemCaseSensitive(e, "motor");
                    const cJSON* ok = cJSON_GetObjectItemCaseSensitive(e, "eligible");
                    if (cJSON_IsString(m)) el[m->valuestring] = cJSON_IsTrue(ok) != 0;
                }
                cJSON_Delete(jp);
                CHECK(el.size() == 7 && el["MInArmX"] && el["MInArmY"] && !el["MInArmZA"] && el["MOutArmX"] && el["MInShuttle1"] && !el["MInRotate"] && el["MTestZ1"],
                      "runtime axes: linear axes eligible; the placeholder Z and the rotary axis not; a Z with real limits eligible; an unknown alias not listed");
                CHECK(Has(j, "\"session\":{\"active\":true") && Has(j, "\"motor\":\"MInArmX\"") && Has(j, "\"zeroUser\":200") && Has(j, "\"spanMaxMm\":100") &&
                      Has(j, "\"maxSpreadPct\":0.1"), "runtime session + the limits the page shows");
            }
            {   //AI(W906-GEARHAND) 20261003: the hand-push session (RULINGS_20261003 #8) -- the first smoke cases; the rest of the case list
                //  (every gate, every end, the E1 rules, the save refusals, mutation controls) is open work (see the commit message)
                auto HReq = [](const char* action) {
                    MotorAccessReq r = Req("uMotorTest", action, "MInArmX");
                    r.button = std::string(action) == "gearHandBegin" ? "mtGearHandStart" : std::string(action) == "gearRatioSave" ? "mtGearSave" : "mtGearPreview";
                    r.kind = "control";
                    return r;
                };
                auto MoveKinds = [](const FakeBackend& b) {
                    int n = 0;
                    for (std::size_t i = 0; i < b.executed.size(); ++i) {
                        const int k = (int)b.executed[i].kind;
                        if (k == kCmdAxJogStart || k == kCmdAxMoveRel || k == kCmdAxMoveAbs || k == kCmdAxHome || k == kCmdAxMoveVel || k == kCmdAxMoveImpose) ++n;
                    }
                    return n + (int)b.moves.size() + (int)b.jogs.size();
                };
                auto Servo = [](FakeBackend& b, bool on) {
                    MotorAccessReq r = Req("uMotorTest", "servoToggle", "MInArmX");
                    r.button = "btnServoOff"; r.flag["servoOn"] = on;
                    return MotorAccessDispatch(r, 731, b);
                };
                FakeBackend h;
                Fresh(h);
                h.actPos[1] = 1000.0; h.cmdPos[1] = 1000.0;
                h.brakeAlias = "MInArmX"; h.brakeReleased = true;
                const MotorAccessOutcome hb = MotorAccessDispatch(HReq("gearHandBegin"), 730, h);
                MotorAccessGearCalState hs = MotorAccessGearCal();
                CHECK(hb.ok && Has(hb.ackJson, "\"result\":\"handArmed\"") && hs.active && hs.hand && hs.handPhase == 0 && hs.e0Card == 1000.0,
                      "GEARHAND: gearHandBegin -> a hand session, E0 = the encoder (1000 pulses)");
                const std::size_t sv0 = h.executed.size();
                const MotorAccessOutcome so = Servo(h, false);
                hs = MotorAccessGearCal();
                CHECK(so.ok && hs.active && hs.handPhase == 1 && MotorAccessEncoderBase(1) && h.brakeHoldAt == (int)sv0 && !h.sleeps.empty() && h.sleeps.back() == 100,
                      "GEARHAND: servo OFF through the existing servoToggle (brake HOLD + 100 ms first) keeps the hand session (phase 1), the axis goes on the encoder base (W5B-6)");
                h.svon[1] = false; h.actPos[1] = 21000.0;                       // pushed by hand: 20000 pulses = 200 mm at GearRatio 1
                MotorAccessPollTick(h, true);
                const MotorAccessOutcome sn = Servo(h, true);
                h.svon[1] = true;
                for (int t = 0; t < 4; ++t) MotorAccessPollTick(h, true);
                hs = MotorAccessGearCal();
                CHECK(sn.ok && hs.active && hs.handPhase == 3 && hs.e1Card == 21000.0, "GEARHAND: servo ON -> E1 = 21000 on two equal samples after the servo on");
                MotorAccessReq pv = HReq("gearRatioPreview");
                pv.num["oldRatio"] = 1.0; pv.flag["hand"] = true; pv.num["rulerMm"] = 198.43;
                const MotorAccessOutcome hp = MotorAccessDispatch(pv, 732, h);
                CHECK(hp.ok && Has(hp.ackJson, "\"canSave\":true") && Has(hp.ackJson, "\"newRatioText\":\"0.99215\"") && Has(hp.ackJson, "\"mode\":\"hand\""),
                      "GEARHAND: preview {hand:true, rulerMm 198.43} -> k = 198.43 / ((21000 - 1000) x 1 / 100) = 0.99215, canSave");
                const std::string hk = KeyOf(hp.ackJson);
                MotorAccessReq sv = HReq("gearRatioSave");
                sv.num["oldRatio"] = 1.0; sv.flag["hand"] = true; sv.num["rulerMm"] = 198.43; sv.str["previewKey"] = hk;
                const MotorAccessOutcome hsv = MotorAccessDispatch(sv, 733, h);
                CHECK(hsv.ok && Has(hsv.ackJson, "\"result\":\"saved\"") && Has(hsv.ackJson, "重開 wb_serve") && h.golden[ix].gearRatio == std::strtod("0.99215", 0) && !MotorAccessGearCal().active,
                      "GEARHAND: save -> GearRatio 0.99215 through the same save path; the message asks for a wb_serve restart; the session is used up");
                CHECK(MoveKinds(h) == 0, "GEARHAND: begin, servo OFF / ON, the ticks, preview and save issued no move command");
                CHECK(ReadAll(logFile).find("hand E0=1000 E1=21000;") != std::string::npos, "GEARHAND (NB2-1): the GearRatioCal.csv line says hand, with E0 / E1");
                FakeBackend q;                                                  // spec (7): the same physical truth in the move mode
                Fresh(q);
                CHECK(Measure(q), "GEARHAND: a move session on the same axis, for the comparison");
                const MotorAccessOutcome qp = MotorAccessDispatch(PvReq("gearRatioPreview", 0.99215), 734, q);
                CHECK(qp.ok && Has(qp.ackJson, "\"newRatioText\":\"0.99215\"") && !hk.empty() && KeyOf(qp.ackJson) == hk,
                      "GEARHAND: the same truth (0.99215 mm a commanded mm) measured by the move mode -> the same GearRatio and the same preview key");
                FakeBackend z;
                Fresh(z);
                MotorAccessReq zr = HReq("gearHandBegin"); zr.motors[0] = "MTestZ1";
                const MotorAccessOutcome zo = MotorAccessDispatch(zr, 735, z);
                CHECK(!zo.ok && Has(zo.ackJson, "Z 軸") && !MotorAccessGearCal().active && z.executed.empty(), "GEARHAND: a Z axis (even with real soft limits) -> refused, nothing sent");
                FakeBackend n;
                Fresh(n);
                n.golden[ix].homeFlag = 0;
                CHECK(MotorAccessDispatch(HReq("gearHandBegin"), 736, n).ok && MotorAccessGearCal().hand, "GEARHAND: HomeFlag 0 does not refuse the hand start (a move gate)");
                //AI(W906-GEARHAND) 20261003: NB2-1 -- the rest of the case list (the WIP's "open work"): the start's gates, one session at a
                //  time / no move mode under it, the E1 rules, the ruler and save refusals, the ends. Every case: nothing moves.
                auto HPv = [&](double ruler) { MotorAccessReq p = HReq("gearRatioPreview"); p.num["oldRatio"] = 1.0; p.flag["hand"] = true; p.num["rulerMm"] = ruler; return p; };
                auto Begin = [&](FakeBackend& b, double e0, long long id) { b.actPos[1] = e0; b.cmdPos[1] = e0; return MotorAccessDispatch(HReq("gearHandBegin"), id, b); };
                auto Push = [&](FakeBackend& b, double e1) {                  // servo OFF -> pushed to e1 -> servo ON -> four fresh samples
                    Servo(b, false); b.svon[1] = false; b.actPos[1] = e1; MotorAccessPollTick(b, true);
                    Servo(b, true); b.svon[1] = true;
                    for (int t = 0; t < 4; ++t) MotorAccessPollTick(b, true);
                };
                {
                    FakeBackend b; Fresh(b); b.svon[1] = false;
                    const MotorAccessOutcome o = Begin(b, 1000.0, 740);
                    CHECK(!o.ok && Has(o.ackJson, "伺服沒開") && !MotorAccessGearCal().active && b.executed.empty(), "GEARHAND: begin refused with the servo OFF (E0 is taken with the axis held), nothing sent");
                }
                {
                    FakeBackend b; Fresh(b); b.motionIO[1] = 0x00004002ul;
                    const MotorAccessOutcome o = Begin(b, 1000.0, 741);
                    CHECK(!o.ok && Has(o.ackJson, "ALM") && !MotorAccessGearCal().active && b.executed.empty(), "GEARHAND: begin refused on a drive alarm, nothing sent");
                }
                {
                    FakeBackend b; Fresh(b); b.gearEmg = true;
                    const MotorAccessOutcome o = Begin(b, 1000.0, 742);
                    CHECK(!o.ok && Has(o.ackJson, "EMG") && !MotorAccessGearCal().active && b.executed.empty(), "GEARHAND: begin refused with EMG pressed, nothing sent");
                }
                {
                    FakeBackend b; Fresh(b); b.state[1] = 2;
                    const MotorAccessOutcome o = Begin(b, 1000.0, 743);
                    CHECK(!o.ok && !MotorAccessGearCal().active && b.executed.empty(), "GEARHAND: begin refused when the axis is not READY, nothing sent");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 744).ok, "setup: a hand session (E0 1000)");
                    const MotorAccessOutcome o2 = Begin(b, 5000.0, 745);
                    CHECK(!o2.ok && Has(o2.ackJson, "量測進行中") && MotorAccessGearCal().hand && MotorAccessGearCal().e0Card == 1000.0, "GEARHAND: a second begin is refused, the first session and its E0 stay");
                    const std::size_t x0 = b.executed.size();
                    const MotorAccessOutcome mv = Mv(b, "MInArmX", 2, true);
                    CHECK(!mv.ok && MotorAccessGearCal().hand && b.executed.size() == x0, "GEARHAND: a move measurement's start is refused during a hand session, nothing sent");
                    const MotorAccessOutcome pm = MotorAccessDispatch(PvReq("gearRatioPreview", 0.99215), 746, b);
                    CHECK(!Has(pm.ackJson, "\"canSave\":true"), "GEARHAND: move-mode readings cannot be saved against a hand session");
                    const MotorAccessOutcome p0 = MotorAccessDispatch(HPv(198.43), 747, b);
                    CHECK(!p0.ok && Has(p0.ackJson, "還沒推"), "GEARHAND: a hand preview before the push (phase 0) is refused with the next step");
                    b.state[1] = 1;
                    Servo(b, false); b.svon[1] = false; b.actPos[1] = 21000.0; MotorAccessPollTick(b, true);
                    Servo(b, true); b.svon[1] = true;
                    MotorAccessPollTick(b, true);                                // the first sample after the servo on: a candidate
                    b.actPos[1] = 21100.0; MotorAccessPollTick(b, true);         // 1 mm off it: still settling, the candidate restarts
                    CHECK(MotorAccessGearCal().handPhase == 2, "GEARHAND: E1 waits while the encoder still changes (21000 -> 21100 pulses > 0.02 mm)");
                    MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().handPhase == 3 && MotorAccessGearCal().e1Card == 21100.0, "GEARHAND: E1 = two equal fresh samples after the servo on (21100)");
                    b.actPos[1] = 21500.0;
                    const MotorAccessOutcome mvd = MotorAccessDispatch(HPv(198.43), 748, b);
                    CHECK(mvd.ok && Has(mvd.ackJson, "\"canSave\":false") && Has(mvd.ackJson, "軸動了"), "GEARHAND: the axis moved after E1 -> computed, not saveable, says so");
                    b.actPos[1] = 21100.0;
                    CHECK(!MotorAccessDispatch(HPv(0.0), 749, b).ok, "GEARHAND: ruler 0 mm refused");
                    const MotorAccessOutcome r3 = MotorAccessDispatch(HPv(198.435), 750, b);
                    CHECK(!r3.ok && Has(r3.ackJson, "0.01 mm"), "GEARHAND: a ruler distance that is not a multiple of 0.01 mm is refused");
                    const MotorAccessOutcome r4 = MotorAccessDispatch(HPv(2000.01), 751, b);
                    CHECK(!r4.ok && Has(r4.ackJson, "不合理"), "GEARHAND: a ruler distance beyond 2000 mm is refused (a typing error)");
                    MotorAccessReq nr = HPv(198.43); nr.num.erase("rulerMm");
                    CHECK(!MotorAccessDispatch(nr, 752, b).ok, "GEARHAND: no rulerMm refused");
                    const MotorAccessOutcome r5 = MotorAccessDispatch(HPv(-198.43), 753, b);
                    CHECK(r5.ok && Has(r5.ackJson, "\"canSave\":false") && Has(r5.ackJson, "正負號"), "GEARHAND: a ruler sign opposite to the encoder's -> computed, not saveable, says why");
                    b.gearTeachOpen = true;
                    const MotorAccessOutcome r6 = MotorAccessDispatch(HPv(198.43), 754, b);
                    CHECK(r6.ok && Has(r6.ackJson, "\"canSave\":false") && Has(r6.ackJson, "教導頁開著"), "GEARHAND: the Teach page open -> not saveable (the move mode's rule)");
                    b.gearTeachOpen = false;
                    const MotorAccessOutcome r7 = MotorAccessDispatch(HPv(198.43), 755, b);
                    CHECK(r7.ok && Has(r7.ackJson, "\"canSave\":true"), "control: the same session and ruler, the page closed -> saveable");
                    Servo(b, false); b.svon[1] = false; MotorAccessPollTick(b, true);
                    const MotorAccessGearCalState s2 = MotorAccessGearCal();
                    CHECK(s2.active && s2.handPhase == 1 && s2.pushes == 2 && s2.e1Card == 0.0, "GEARHAND: a second servo OFF voids E1 (phase 1, pushes 2)");
                    CHECK(MoveKinds(b) == 0, "GEARHAND: no move command in any of the above");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 764).ok, "setup: a hand session");
                    Servo(b, false); b.svon[1] = false; b.actPos[1] = 21000.0; MotorAccessPollTick(b, true);
                    Servo(b, true);                                              // sent; the drive's SVON shows a few polls later
                    MotorAccessPollTick(b, true); MotorAccessPollTick(b, true);
                    const MotorAccessGearCalState w = MotorAccessGearCal();
                    CHECK(w.active && w.handPhase == 2 && w.pushes == 1, "GEARHAND: OFF samples right after the servo ON was sent = the drive not enabled yet -> still phase 2, not a second push");
                    b.svon[1] = true;
                    for (int t = 0; t < 4; ++t) MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().handPhase == 3 && MotorAccessGearCal().e1Card == 21000.0 && MotorAccessGearCal().pushes == 1, "GEARHAND: SVON on -> E1 21000, one push");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 756).ok, "setup: a hand session");
                    Push(b, 1000.0);                                             // servo OFF / ON without moving it
                    const MotorAccessOutcome o = MotorAccessDispatch(HPv(198.43), 757, b);
                    CHECK(MotorAccessGearCal().handPhase == 3 && o.ok && Has(o.ackJson, "\"canSave\":false") && Has(o.ackJson, "沒有被推動"), "GEARHAND: E1 = E0 (not pushed, or pulled back at the servo on) -> not saveable, says so");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 758).ok, "setup: a hand session");
                    const std::size_t x0 = b.executed.size();
                    b.systemStart = true; MotorAccessPollTick(b, true); b.systemStart = false;
                    const MotorAccessGearCalState s = MotorAccessGearCal();
                    CHECK(!s.active && Has(s.why, "SystemStart") && b.executed.size() == x0, "GEARHAND: SystemStart ends the hand session; nothing is stopped (it moved nothing)");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 759).ok, "setup: a hand session");
                    b.motionIO[1] = 0x00004002ul; MotorAccessPollTick(b, true);
                    CHECK(!MotorAccessGearCal().active && Has(MotorAccessGearCal().why, "ALM"), "GEARHAND: a drive alarm during the session ends it");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 760).ok, "setup: a hand session");
                    MotorAccessReq j = Req("uMotorTest", "jogP", "MInArmX"); j.button = "sbMotorTest_JogP"; j.num["speed"] = 50;
                    const MotorAccessOutcome jo = MotorAccessDispatch(j, 761, b);
                    MotorAccessPollTick(b, true);
                    CHECK(jo.ok && !MotorAccessGearCal().active && Has(MotorAccessGearCal().why, "量測中止"),"GEARHAND: a JOG on the axis (another command of this tree) ends the hand session on the next sample");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 762).ok, "setup: a hand session");
                    Servo(b, false); b.svon[1] = false; MotorAccessPollTick(b, true);
                    const MotorAccessOutcome so = MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmX"), 763, b);
                    CHECK(so.ok && !MotorAccessGearCal().active && Has(MotorAccessGearCal().why, "伺服還是 OFF"), "GEARHAND: STOP ends a hand session; with the servo still OFF the end says so (nobody walks away from a free axis)");
                }
                //AI(W906-GEARHAND) 20261003: NB2-1 -- pins for the mutants the first red check left alive (README R191)
                auto TopNum = [](const std::string& ack, const char* key, double& v) {
                    cJSON* j = cJSON_Parse(ack.c_str());
                    const cJSON* x = j ? cJSON_GetObjectItemCaseSensitive(j, key) : 0;
                    const bool ok = cJSON_IsNumber(x);
                    if (ok) v = x->valuedouble;
                    cJSON_Delete(j);
                    return ok;
                };
                {
                    FakeBackend b; Fresh(b);
                    MotorAccessReq j = Req("uMotorTest", "jogP", "MInArmX"); j.button = "sbMotorTest_JogP"; j.num["speed"] = 50;
                    CHECK(MotorAccessDispatch(j, 770, b).ok && MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmX"), 771, b).ok, "setup: a JOG and its stop on MInArmX (its issued sequence is not 0)");
                    b.freezePolls = true;
                    const MotorAccessOutcome st = Begin(b, 1000.0, 772);
                    CHECK(!st.ok && Has(st.ackJson, "監看器還沒更新") && !MotorAccessGearCal().active, "GEARHAND: begin refused on a stale sample (the stop's poll has not passed)");
                    b.freezePolls = false;
                    const MotorAccessOutcome hb = Begin(b, 1000.0, 773);
                    double e0 = -1.0, axv = -1.0;
                    CHECK(hb.ok && Has(hb.ackJson, "E0 = 1000") && TopNum(hb.ackJson, "e0Card", e0) && e0 == 1000.0 && TopNum(hb.ackJson, "axis", axv) && axv == 1.0 &&
                          !Has(hb.ackJson, "\"startedAt\":\"\"") && Has(hb.ackJson, "\"waiting\":true"),
                          "GEARHAND: the begin ack -- the E0 text, e0Card 1000 and axis 1 at the top, the session's startedAt, waiting (phase 0)");
                    double miv = -1.0;
                    CHECK(TopNum(hb.ackJson, "motIndex", miv) && miv == (double)ix, "GEARHAND: the begin ack carries the golden MOT index");
                    CHECK(Has(hb.ackJson, "\"placeholder\":false"), "GEARHAND: a hand session on a real-limit axis (MInArmX +-30000) is not placeholder (the session default is true)");
                    MotorAccessPollTick(b, true); MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().active, "GEARHAND: the session took the axis' issued sequence at begin -- the earlier JOG / stop do not end it");
                    MotorAccessReq so = Req("uMotorTest", "servoToggle", "MInArmY"); so.button = "btnServoOff"; so.flag["servoOn"] = false;
                    MotorAccessDispatch(so, 774, b);
                    CHECK(MotorAccessGearCal().active && MotorAccessGearCal().handPhase == 0 && MotorAccessGearCal().pushes == 0 && !MotorAccessEncoderBase(2),
                          "GEARHAND: a servo OFF on another axis (MInArmY) is not a push of this session");
                    MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmX"), 775, b);
                    Servo(b, false);
                    CHECK(!MotorAccessGearCal().active && MotorAccessGearCal().pushes == 0 && !MotorAccessEncoderBase(1), "GEARHAND: after STOP a servo OFF on the axis is not a push of the ended session");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 780).ok, "setup: a hand session");
                    Servo(b, false); b.svon[1] = false; MotorAccessPollTick(b, true);
                    const MotorAccessOutcome p1 = MotorAccessDispatch(HPv(198.43), 781, b);
                    CHECK(!p1.ok && Has(p1.ackJson, "推到尺上的終點記號"), "GEARHAND: a preview in phase 1 (servo OFF, pushing) is refused with that step");
                    b.actPos[1] = 21000.0;
                    Servo(b, true); b.svon[1] = true;
                    for (int t = 0; t < 4; ++t) MotorAccessPollTick(b, true);
                    MotorAccessReq py = HPv(198.43); py.motors[0] = "MInArmY";
                    const MotorAccessOutcome wy = MotorAccessDispatch(py, 782, b);
                    CHECK(!wy.ok && Has(wy.ackJson, "不是 MInArmY"), "GEARHAND: a hand preview for another axis is refused, naming the session's axis");
                    const MotorAccessOutcome ok0 = MotorAccessDispatch(HPv(198.43), 783, b);
                    CHECK(ok0.ok && Has(ok0.ackJson, "\"canSave\":true") && Has(ok0.ackJson, "\"waiting\":false"), "control: phase 3, intact -> saveable, not waiting");
                    b.svon[1] = false;                                           // the monitor shows the servo OFF, no tick yet
                    const MotorAccessOutcome sv = MotorAccessDispatch(HPv(198.43), 784, b);
                    CHECK(!Has(sv.ackJson, "\"canSave\":true") && Has(sv.ackJson, "保持伺服 ON") && Has(sv.ackJson, "\"waiting\":true"), "GEARHAND: E1 taken but the servo now reads OFF -> not saveable (keep it ON), waiting");
                    b.svon[1] = true; b.state[1] = 3;
                    const MotorAccessOutcome es = MotorAccessDispatch(HPv(198.43), 785, b);
                    CHECK(!Has(es.ackJson, "\"canSave\":true") && Has(es.ackJson, "ERROR_STOP"), "GEARHAND: E1 taken, the axis now ERROR_STOP -> not saveable, says so");
                    b.state[1] = 1;
                    b.golden[ix].gearRatio = 1.1;
                    MotorAccessReq pr = HPv(198.43); pr.num["oldRatio"] = 1.1;
                    const MotorAccessOutcome rc = MotorAccessDispatch(pr, 786, b);
                    CHECK(!Has(rc.ackJson, "\"canSave\":true") && Has(rc.ackJson, "齒輪比變了"), "GEARHAND: GearRatio changed since the begin -> not saveable, says so");
                    b.golden[ix].gearRatio = 1.0;
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 790).ok, "setup: a hand session");
                    Push(b, 21000.0);
                    CHECK(MotorAccessGearCal().handPhase == 3, "setup: E1");
                    MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmX"), 791, b);
                    const MotorAccessOutcome again = Begin(b, 21000.0, 792);
                    const MotorAccessGearCalState s = MotorAccessGearCal();
                    CHECK(again.ok && s.handPhase == 0 && s.pushes == 0 && s.e1Card == 0.0 && s.e0Card == 21000.0, "GEARHAND: a new begin starts clean (phase 0, no pushes, no E1 left from the last session)");
                    FakeBackend m; Fresh(m);
                    CHECK(Mv(m, "MInArmX", 2, true).ok && MotorAccessGearCal().active && !MotorAccessGearCal().hand, "setup: a MOVE session on MInArmX");
                    const MotorAccessOutcome hm = MotorAccessDispatch(HPv(198.43), 793, m);
                    CHECK(!hm.ok && Has(hm.ackJson, "進行中的是移動量測"), "GEARHAND: a hand preview during a move session is refused, says which session runs");
                }
                {
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 795).ok, "setup: a hand session");
                    b.svon.erase(1); MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().handPhase == 0 && MotorAccessGearCal().pushes == 0, "GEARHAND: no SVON sample -> wait (not a push)");
                    b.svon[1] = true;
                    Servo(b, false); b.svon[1] = false; b.actPos[1] = 0.0; MotorAccessPollTick(b, true);   // pushed back to 0
                    b.svon[1] = true; MotorAccessPollTick(b, true);              // the drive on, not by a command of this tree
                    CHECK(MotorAccessGearCal().handPhase == 2, "GEARHAND: SVON seen on a sample in phase 1 (not a servo command of this tree) -> phase 2, E1 not yet");
                    b.state[1] = 2; for (int t = 0; t < 4; ++t) MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().handPhase == 2, "GEARHAND: not READY -> no E1");
                    b.state[1] = 1; MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().handPhase == 2, "GEARHAND: one READY sample (at 0) -> a candidate only, E1 needs two");
                    MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().handPhase == 3 && MotorAccessGearCal().e1Card == 0.0, "GEARHAND: the second equal sample -> E1 = 0");
                }
                {   // second red-check round (README R191)
                    FakeBackend b; Fresh(b);
                    const MotorAccessOutcome nos = MotorAccessDispatch(HPv(198.43), 800, b);
                    CHECK(!nos.ok && Has(nos.ackJson, "沒有進行中的手推量測"), "GEARHAND: a hand preview with no session is refused, says so");
                    CHECK(Begin(b, 1000.0, 801).ok, "setup: a hand session");
                    Servo(b, false); b.svon[1] = false; b.actPos[1] = 21000.0; MotorAccessPollTick(b, true);
                    Servo(b, true); b.svon[1] = true;
                    MotorAccessPollTick(b, true);                                // a candidate (21000)
                    b.svon[1] = false; MotorAccessPollTick(b, true);             // the drive drops for a sample
                    b.svon[1] = true; MotorAccessPollTick(b, true);              // back on, the same position
                    CHECK(MotorAccessGearCal().handPhase == 2, "GEARHAND: an OFF sample between two ON samples drops the candidate -- E1 needs two equal samples after it");
                    b.actPos.erase(1); for (int t = 0; t < 4; ++t) MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().handPhase == 2, "GEARHAND: no encoder sample -> no E1 (never 0 by default)");
                    b.actPos[1] = 21000.0; MotorAccessPollTick(b, true); MotorAccessPollTick(b, true);
                    CHECK(MotorAccessGearCal().handPhase == 3 && MotorAccessGearCal().e1Card == 21000.0, "setup: E1 21000");
                    const MotorAccessOutcome ra = MotorAccessDispatch(Req("uMotorTest", "resetAlarm", "MInArmX"), 802, b);   // another command on the axis, no tick after it
                    const MotorAccessOutcome pa = MotorAccessDispatch(HPv(198.43), 803, b);
                    CHECK(ra.ok && !Has(pa.ackJson, "\"canSave\":true"), "GEARHAND: another command on the axis after E1 (Alarm Reset), before the next sample -> not saveable");
                    FakeBackend y; Fresh(y);
                    y.actPos[2] = 500.0; y.cmdPos[2] = 500.0;
                    MotorAccessReq yb = HReq("gearHandBegin"); yb.motors[0] = "MInArmY";
                    const MotorAccessOutcome yo = MotorAccessDispatch(yb, 804, y);
                    CHECK(yo.ok && Has(yo.ackJson, "\"placeholder\":true"), "GEARHAND: a hand session on a +-999999 axis says placeholder (the plan's soft-limit rule)");
                }
                {   //AI(W906-GEARHAND) 20261003: NB2-1 (g) M1 (St01's review of 476467ab) -- an end WITHOUT a save after a push: HomeFlag 0 (the
                    //  save's rule), the end says restart + HOME, moveAbsolute on the axis refused (it would start from the stale command position)
                    auto MoveAbs = [&](FakeBackend& x, long long id) { MotorAccessReq ga = Req("uMotorTest", "moveAbsolute", "MInArmX"); ga.num["targetPos"] = 2000; ga.num["speed"] = 50; return MotorAccessDispatch(ga, id, x); };
                    FakeBackend b; Fresh(b);
                    CHECK(Begin(b, 1000.0, 810).ok, "setup: a hand session");
                    Push(b, 21000.0);
                    MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmX"), 811, b);
                    const MotorAccessGearCalState s = MotorAccessGearCal();
                    const MotorAccessOutcome mv = MoveAbs(b, 812);
                    CHECK(!s.active && b.golden[ix].homeFlag == 0 && Has(s.why, "HomeFlag 已設成 0") && Has(s.why, "重開 wb_serve"),
                          "GEARHAND (g) M1: pushed, then STOP without a save -> HomeFlag 0, and the end says restart wb_serve + HOME");
                    CHECK(!mv.ok && Has(mv.ackJson, "Motor not home"), "GEARHAND (g) M1: ... moveAbsolute on the pushed axis is refused (Motor not home)");
                    FakeBackend a; Fresh(a);
                    CHECK(Begin(a, 1000.0, 813).ok, "setup: a hand session");
                    Push(a, 21000.0);
                    a.motionIO[1] = 0x00004002ul; MotorAccessPollTick(a, true);   // a drive alarm ends it
                    CHECK(!MotorAccessGearCal().active && a.golden[ix].homeFlag == 0 && Has(MotorAccessGearCal().why, "HomeFlag 已設成 0"),
                          "GEARHAND (g) M1: pushed, then ended by a drive alarm -> HomeFlag 0 as well");
                    FakeBackend n; Fresh(n);
                    CHECK(Begin(n, 1000.0, 814).ok, "setup: a hand session");
                    MotorAccessDispatch(Req("uMotorTest", "stop", "MInArmX"), 815, n);
                    const MotorAccessOutcome nm = MoveAbs(n, 816);
                    CHECK(!MotorAccessGearCal().active && n.golden[ix].homeFlag == 1 && !Has(MotorAccessGearCal().why, "HomeFlag") && !Has(nm.ackJson, "Motor not home"),
                          "GEARHAND (g) M1: no push, STOP -> HomeFlag untouched (1), moveAbsolute not refused for homing");
                }
                {   //AI(W906-BRAKE-SERVOFIRST) 20261004: NB2-1 task (m), RULINGS_20261003 #24 (Jimmy A + Steven 22:57 「應該要先servo on後才能放煞車」) --
                    //  the laptop's pins (1)-(4) on the two pieces BrakeAxisTick / the group hook / the G05 per-axis release use
                    //  (WebMotorAccessLive.cpp is not linked here: its wiring is read + built, said so in the MR).
                    using ht9045::BrakeServoOnLedger; using ht9045::BrakePowerOn; using ht9045::BrakePowerReady;
                    std::string w;
                    BrakeServoOnLedger L; unsigned seq = 7;                         // the issue counter is wherever it is at the first look
                    bool pOn = BrakePowerOn(true, false, false, false);             // boot: relay output 0
                    CHECK(!pOn && !L.Step(seq, pOn) && !L.Issued(seq) && !BrakePowerReady(pOn, false, 10, &w) && Has(w, "馬達電源沒開"),
                          "BRAKE-SERVOFIRST (1) boot: a leftover SVON, relay 0, bMotorPowerState false -> no release");
                    pOn = BrakePowerOn(true, true, false, false);                   // G31a DoMotorPowerOn: relay ON, the delay counting
                    CHECK(!L.Step(seq, pOn) && !L.Issued(seq) && !BrakePowerReady(pOn, true, 4, &w) && Has(w, "上電延遲"),
                          "BRAKE-SERVOFIRST (1) relay on, power-on delay still counting -> no release");
                    CHECK(BrakePowerReady(pOn, true, 0, &w) && !L.Step(seq, pOn) && !L.Issued(seq),
                          "BRAKE-SERVOFIRST (1) power ready but no Servo ON from this run (the port sends none at boot) -> held");
                    ++seq;                                                          // a Servo ON from this run (HOME InitMotor, Motor Test, M35 boot)
                    CHECK(L.Step(seq, pOn) && L.Issued(seq), "BRAKE-SERVOFIRST (2) a Servo ON after the delay -> fresh (0.5 s, then released)");
                    CHECK(!L.Step(seq, pOn) && L.Issued(seq), "BRAKE-SERVOFIRST (2) ... once: the next tick is not fresh again");
                    ++seq;
                    CHECK(L.Step(seq, pOn) && L.Issued(seq), "BRAKE-SERVOFIRST (3) running, power long on: a Motor Test servo off/on -> fresh again (EastSun 0929 per axis)");
                    pOn = BrakePowerOn(true, false, false, false);                  // HOME case 2: relay OFF 3 s
                    CHECK(!L.Step(seq, pOn) && !L.Issued(seq), "BRAKE-SERVOFIRST (4) HOME case 2 relay off: every earlier Servo ON forgotten");
                    pOn = BrakePowerOn(true, true, false, false);                   // case 3 DoMotorPowerOn; bMotorPowerState stays true (forms/fMain.cpp:981)
                    CHECK(!L.Step(seq, pOn) && !L.Issued(seq) && BrakePowerReady(pOn, true, 0, &w),
                          "BRAKE-SERVOFIRST (4) case 3 relay back, power ready: case 20's group release is held (golden releases here, before servo on -- ruling #24)");
                    ++seq;
                    CHECK(L.Step(seq, pOn) && L.Issued(seq), "BRAKE-SERVOFIRST (4) case 300 InitMotor Servo ON -> fresh, released 0.5 s later");
                    pOn = BrakePowerOn(true, false, false, false); ++seq;            // a count moving while the power is not on is history too
                    CHECK(!L.Step(seq, pOn) && !L.Issued(seq), "BRAKE-SERVOFIRST a Servo ON while the motor power is off does not count");
                    CHECK(!BrakePowerOn(true, true, true, false) && !BrakePowerOn(true, true, false, true) && BrakePowerOn(false, false, false, false),
                          "BRAKE-SERVOFIRST power on = relay (or no relay output) && no EMG && SnMotorPower not off");
                    CHECK(!BrakePowerReady(true, false, 0, &w) && Has(w, "bMotorPowerState"), "BRAKE-SERVOFIRST bMotorPowerState false -> not ready, says so");
                    //  laptop 1004 01:1x: a held brake blocks MOTION only -- every kind that moves an axis, and never a stop / servo / reset
                    using ht9045::BrakeMotionKind;
                    CHECK(BrakeMotionKind(ht9045::kCmdAxJogStart) && BrakeMotionKind(ht9045::kCmdAxMoveRel) && BrakeMotionKind(ht9045::kCmdAxMoveAbs) &&
                          BrakeMotionKind(ht9045::kCmdAxHome) && BrakeMotionKind(ht9045::kCmdAxMoveVel) && BrakeMotionKind(ht9045::kCmdAxMoveImpose) &&
                          BrakeMotionKind(ht9045::kCmdAxMoveHome),
                          "BRAKE-SERVOFIRST motion kinds (jog / rel / abs / home / vel / impose / move-home) are held back while the brake is held");
                    CHECK(!BrakeMotionKind(ht9045::kCmdAxStop) && !BrakeMotionKind(ht9045::kCmdAxEmgStop) && !BrakeMotionKind(ht9045::kCmdAxSvOn) &&
                          !BrakeMotionKind(ht9045::kCmdAxResetError) && !BrakeMotionKind(ht9045::kCmdAxSetSpeed) && !BrakeMotionKind(ht9045::kCmdAxSetCmdPos),
                          "BRAKE-SERVOFIRST ... and a stop / emergency stop / Servo ON / reset / parameter write is never blocked");
                }
                CHECK(GearCalLogDirOf("D:\\oplog", "D:\\HT9045_Log") == "D:\\oplog" &&GearCalLogDirOf(0, "D:\\HT9045_Log") == "D:\\HT9045_Log" &&
                      GearCalLogDirOf("", "D:\\HT9045_Log") == "D:\\HT9045_Log" && GearCalLogDirOf(0, "").empty(),
                      "GEARLOGDIR (s0 #71 (4) A): W906_OPLOG_DIR set -> there; not set (or empty) -> the machine log root; both empty -> no file");
            }
            Clean();
        }
        g_W906OrgActiveLow = 0;
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (ZDOWN)
    //AI(W906-TEACH-ZDOWN) 20261003: INBOX 147 -- golden uteach btnSetAllInArmZClick :4300-4365 / btnSetAllOutArmZClick :4367-4435
    //  (teachSetAllArmZ: edits only) and btnOutZAllDownClick :4542-4562 (teachOutZAllDown: SetSpeed(10), then MotorMove on every
    //  Out Z).  Locks: every gate refuses and touches nothing; targets / edits equal golden's arithmetic; golden's two loops (every
    //  speed before any move, NB2 R144 #3); golden's fCMD on a second press (R144 #4); a base off the grid (R144 #2, ep1Picker);
    //  Enable=0 / not-in-Mot_Table nozzles read golden's cached Position (R144 #7 / #8); the [i][i] quirk (ep4Picker); STOP and
    //  the Teach close edge stop the Zs; the busy gates: Set All = that arm (NB2 R169 M2), All Down = every motor of the machine
    //  (decision 1 = B, 20261003: an axis of no arm busy refuses it, Set All still goes; an unknown motor count refuses it).
    //  Catalog: 65 / 48 / 43.
    printf("[ZDOWN] Teach Set All In / Out Arm Z + Out Z All Down (golden uteach, INBOX 147)\n");
    {
        MotorAccessResetJobs();
        struct ZdOps : public ITeachZDownOps {
            TeachZArmGrid in, out;
            bool ok;
            std::string why;
            std::map<int, int> done;                                            // golden MotionDone of a golden-object motor (missing = -1)
            ZdOps() : ok(true), aliases(0), forcedCount(-1) {}
            bool ArmGrid(bool inArm, TeachZArmGrid& g, std::string& w) override { if (!ok) { w = why; return false; } g = inArm ? in : out; return true; }
            int GoldenMotionDone(int mi) override { std::map<int, int>::const_iterator it = done.find(mi); return it == done.end() ? -1 : it->second; }
            //AI(W906-TEACH-ZDOWN) 20261003: decision 1 = B -- MotorCount = the fake's highest MOT index + 1 (TestMot hands out 100..);
            //  forcedCount >= 0 overrides it (the fail-closed case)
            const std::map<int, std::string>* aliases;
            int forcedCount;
            int MotorCount() override { return forcedCount >= 0 ? forcedCount : (aliases && !aliases->empty() ? aliases->rbegin()->first + 1 : 0); }
        };
        struct ZdFake : public FakeBackend {                                   // golden's two loops, in the order the calls happen
            std::vector<std::string> log;                                      // "S<mi>" golden SetSpeed, "M<mi>" golden MotorMove, "C<kind>:<axis>" a card command
            void GoldenSetSpeed(int mi, int pct, bool jog) override { log.push_back("S" + std::to_string(mi)); FakeBackend::GoldenSetSpeed(mi, pct, jog); }
            int GoldenMotorMove(int mi, int t, int pct, bool ss) override { log.push_back("M" + std::to_string(mi)); return FakeBackend::GoldenMotorMove(mi, t, pct, ss); }
            Pci1203CmdResult Pci1203Execute(const Pci1203Cmd& c) override { log.push_back("C" + std::to_string((int)c.kind) + ":" + std::to_string(c.axis)); return FakeBackend::Pci1203Execute(c); }
        };
        MotorGolden gz;
        gz.valid = true; gz.enable = true; gz.gearRatio = 1.0; gz.softP = 999999; gz.softN = -999999; gz.homeFlag = 1;
        gz.jogHigh = 10000; gz.initSpeed = 500; gz.homeHigh = 2000; gz.homeLow = 500; gz.accDb = 3000; gz.decDb = 3100; gz.acc = 3000; gz.dec = 3100;
        gz.armZ = true;
        MotorGolden gx = gz;  gx.armZ = false;
        MotorGolden g0 = gz;  g0.enable = false; g0.selectable = false;          // MN200 Enable=0 (real-machine build)
        ZdFake be;
        be.reg = BuildTeachRegistry(std::set<std::string>());
        // the HT9050 picture: X / Y / ZA on PCI1203, ZB..ZH MN200 Enable=0 (machines/HT9050/Mot_Table.csv), USE_PICKER_COUNT=1 (ep8, 2x4)
        const int oX = Add1203(be, "MOutArmX", 1, gx), oY = Add1203(be, "MOutArmY", 2, gx), oZA = Add1203(be, "MOutArmZA", 3, gz);
        const char* oNames[8] = { "MOutArmZA", "MOutArmZC", "MOutArmZE", "MOutArmZG", "MOutArmZB", "MOutArmZD", "MOutArmZF", "MOutArmZH" };   // [0][0..3], [1][0..3]
        int oZ[8];
        oZ[0] = oZA;
        for (int k = 1; k < 8; ++k) {
            oZ[k] = TestMot(oNames[k]);
            be.table[oNames[k]] = AxisOther(oZ[k], "MN200", true); be.table[oNames[k]].tableEnable = false; be.golden[oZ[k]] = g0; be.aliasOf[oZ[k]] = oNames[k];
        }
        const int oP = TestMot("MOutArmPitch");
        be.table["MOutArmPitch"] = AxisOther(oP, "MN200", true); be.table["MOutArmPitch"].tableEnable = false; be.golden[oP] = g0; be.aliasOf[oP] = "MOutArmPitch";
        ZdOps ops;
        ops.out.motRow = 2; ops.out.motCol = 4; ops.out.maxRow = 2; ops.out.maxCol = 4; ops.out.yBase = 0; ops.out.xBase = 2; ops.out.pickerCount = 1;
        for (int k = 0; k < 8; ++k) ops.out.zIndex[k / 4][k % 4] = oZ[k];
        ops.out.armAxes.push_back(oX); ops.out.armAxes.push_back(oY); ops.out.armAxes.push_back(oP);
        const int iX = Add1203(be, "MInArmX", 6, gx), iY = Add1203(be, "MInArmY", 7, gx), iZA = Add1203(be, "MInArmZA", 4, gz);
        const int iZC = Add1203(be, "MInArmZC", 5, gz);  be.table["MInArmZC"].tableEnable = false;   // a PCI1203 row with Enable=0 (R144 #7)
        const char* iNames[8] = { "MInArmZA", "MInArmZC", "MInArmZE", "MInArmZG", "MInArmZB", "MInArmZD", "MInArmZF", "MInArmZH" };
        int iZ[8];
        iZ[0] = iZA; iZ[1] = iZC;
        for (int k = 2; k < 8; ++k) {
            iZ[k] = TestMot(iNames[k]);
            if (k == 3) continue;                                               // MInArmZG: golden InArmZIndex has it, this Mot_Table does not (R144 #8)
            be.table[iNames[k]] = AxisOther(iZ[k], "MN200", true); be.table[iNames[k]].tableEnable = false; be.golden[iZ[k]] = g0; be.aliasOf[iZ[k]] = iNames[k];
        }
        ops.in = ops.out;
        for (int k = 0; k < 8; ++k) ops.in.zIndex[k / 4][k % 4] = iZ[k];
        ops.in.armAxes.clear(); ops.in.armAxes.push_back(iX); ops.in.armAxes.push_back(iY);
        TeachZDownInstallOps(&ops);
        ops.aliases = &be.aliasOf;                                              // decision 1 = B: All Down asks MOT[0 .. MotorCount()-1]

        auto Fld = [](MotorAccessReq& q, const char* k, double v) { q.obj["fields"][k] = v; };
        auto EditOf = [](const std::string& ack, const char* key, long long& v) -> bool {
            cJSON* j = cJSON_Parse(ack.c_str());
            const cJSON* e = j ? cJSON_GetObjectItemCaseSensitive(j, "edits") : 0;
            const cJSON* x = e ? cJSON_GetObjectItemCaseSensitive(e, key) : 0;
            const bool ok = cJSON_IsNumber(x);
            if (ok) v = (long long)x->valuedouble;
            cJSON_Delete(j);
            return ok;
        };
        auto EditCount = [](const std::string& ack) -> int {
            cJSON* j = cJSON_Parse(ack.c_str());
            const cJSON* e = j ? cJSON_GetObjectItemCaseSensitive(j, "edits") : 0;
            int n = 0;
            const cJSON* x = 0;
            if (cJSON_IsObject(e)) cJSON_ArrayForEach(x, e) ++n;
            cJSON_Delete(j);
            return n;
        };
        auto Untouched = [](ZdFake& b, std::size_t log0, std::size_t ex0, std::size_t mv0, std::size_t sp0) {
            return b.log.size() == log0 && b.executed.size() == ex0 && b.moves.size() == mv0 && b.speedCalls.size() == sp0;
        };
        MotorAccessReq dn = Req("uteach", "teachOutZAllDown", "MInArmX");       // motors = the page's selection (golden ignores it)
        dn.button = "btnOutZAllDown"; dn.kind = "motion";
        const char* oEdit[8] = { "setEditZ2A", "setEditZ2C", "setEditZ2E", "setEditZ2G", "setEditZ2B", "setEditZ2D", "setEditZ2F", "setEditZ2H" };
        Fld(dn, "SetEditPickOutSht", 5000);
        for (int k = 0; k < 8; ++k) Fld(dn, oEdit[k], 10 * (k + 1));            // ZA 10, ZC 20, ZE 30, ZG 40, ZB 50, ZD 60, ZF 70, ZH 80
        const int kSet = (int)kCmdAxSetSpeed, kMov = (int)kCmdAxMoveAbs;
        const std::string cSet = "C" + std::to_string(kSet) + ":3", cMov = "C" + std::to_string(kMov) + ":3";

        // ---- Out Z All Down, the happy path: golden's two loops and its arithmetic
        std::size_t l0 = be.log.size(), x0 = be.executed.size();
        MotorAccessOutcome o = MotorAccessDispatch(dn, 41, be);
        CHECK(o.ok && AckShapeOk(o.ackJson, 7) && Has(o.ackJson, "\"result\":\"moving\"") && Has(o.ackJson, "\"moving\":1") && Has(o.ackJson, "\"noHardware\":7") &&
              Has(o.ackJson, "\"refused\":0") && Has(o.ackJson, "\"partial\":false"),
              "Out Z All Down -> ok moving: ZA (PCI1203) moves, ZB..ZH (MN200 Enable=0) are golden bookkeeping only");
        CHECK(CountKind(be, x0, kCmdAxMoveAbs) == 1 && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().axis == 3 && be.executed.back().value == 5010.0,
              "ZA's 1203 axis: MoveAbs to SetEditPickOutSht 5000 + setEditZ2A 10 = 5010 (golden :4551 / :4559)");
        {
            bool tg = be.moves.size() >= 7;
            for (int k = 1; k < 8 && tg; ++k) {
                const FakeBackend::Move& m = be.moves[be.moves.size() - 7 + (k - 1)];
                tg = m.mi == oZ[k] && m.target == 5000 + 10 * (k + 1) && !m.setSpeed;
            }
            CHECK(tg, "ZC, ZE, ZG, ZB, ZD, ZF, ZH (golden order i=0 then i=1): golden MOT.MotorMove(5000 + their field), no SetSpeed / InitMOTParameter with it");
        }
        {
            std::size_t lastSpeed = 0, firstMove = be.log.size();
            int speeds = 0;
            for (std::size_t i = l0; i < be.log.size(); ++i) {
                const std::string& e = be.log[i];
                if (e == cSet || e[0] == 'S') { lastSpeed = i; ++speeds; }
                if ((e == cMov || e[0] == 'M') && firstMove == be.log.size()) firstMove = i;
            }
            CHECK(speeds == 4 + 7 && lastSpeed < firstMove && firstMove < be.log.size(),
                  "golden's two loops: every Out Z's SetSpeed(10) (4 PTP writes on ZA's card axis + 7 golden objects) before the first MotorMove (NB2 R144 #3)");
        }
        CHECK(LastRunSpeed(be) == MotorSpeedFromPct(10, gz).velHigh && be.speedCalls.size() == 7 && be.speedCalls.back().pct == 10 && !be.speedCalls.back().jog,
              "SetSpeed(10): ZA's PTP run speed = golden 10% of PJogHighSpeed; the MN200 rows get golden SetSpeed(10) (not the jog family)");
        CHECK(be.teachCanMoveCalls.empty(), "golden btnOutZAllDownClick has no CheckCanMove / IsCanQuickJogMove -> not asked");

        // ---- NB2 R144 #4: the second press with the same target is golden's "already arrived" (fCMD), the third moves again
        be.cmdPos[3] = 0.0;                                                     // the operator lifted ZA by hand / jog after the first press
        x0 = be.executed.size();
        o = MotorAccessDispatch(dn, 42, be);
        CHECK(o.ok && CountKind(be, x0, kCmdAxMoveAbs) == 0 && CountKind(be, x0, kCmdAxSetSpeed) == 4 && Has(o.ackJson, "\"result\":\"inPosition\"") &&
              Has(o.ackJson, "\"arrived\":1") && Has(o.ackJson, "fCMD"),
              "second press, same target: golden MotorMove keeps fCMD (iOldPos unchanged) -> MotionDone -> 1, nothing sent; the speed is set again (R144 #4)");
        x0 = be.executed.size();
        o = MotorAccessDispatch(dn, 43, be);
        CHECK(o.ok && CountKind(be, x0, kCmdAxMoveAbs) == 1 && be.executed.back().value == 5010.0 && Has(o.ackJson, "\"result\":\"moving\""),
              "third press: fCMD was cleared by the second -> ZA goes down to 5010 again");
        be.cmdPos[3] = 5010.0;
        o = MotorAccessDispatch(dn, 44, be);                                    // fCMD -> arrived (and cleared)
        x0 = be.executed.size();
        o = MotorAccessDispatch(dn, 45, be);
        CHECK(o.ok && CountKind(be, x0, kCmdAxMoveAbs) == 0 && Has(o.ackJson, "\"arrived\":1"),
              "ZA already at 5010 with fCMD clear: golden CompareCommandPos -> 1, nothing sent");

        // ---- STOP and the Teach close edge stop the Zs (golden btnStopClick / FormClose -> btnStop->Click(): StopAllMotor)
        be.cmdPos[3] = 0.0;
        o = MotorAccessDispatch(dn, 46, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().axis == 3, "setup: ZA commanded down");
        be.state[3] = 5;                                                        // PTP_MOT
        x0 = be.executed.size();
        const std::size_t sa0 = be.stopAll.size();
        MotorAccessDispatch(Req("uteach", "stop", 0), 47, be);
        bool stopped3 = false;
        for (std::size_t i = x0; i < be.executed.size(); ++i) if (be.executed[i].kind == kCmdAxStop && be.executed[i].axis == 3) stopped3 = true;
        CHECK(stopped3 && be.stopAll.size() == sa0 + 1 && be.stopAll.back() == "uteach", "STOP -> golden StopAllMotor + a 1203 stop on ZA's axis (the move ends)");
        x0 = be.executed.size();
        MotorAccessPageClosed(be, "fTeach");
        stopped3 = false;
        for (std::size_t i = x0; i < be.executed.size(); ++i) if (be.executed[i].kind == kCmdAxStop && be.executed[i].axis == 3) stopped3 = true;
        CHECK(stopped3, "closing Teach (the page close edge = golden FormClose -> btnStop->Click()) stops ZA too");
        be.state[3] = 1;

        // ---- NB2 R144 #3: a refused move still had its speed set first (golden sets 10% unconditionally)
        be.doorOpen.insert(oZA);
        be.cmdPos[3] = 0.0;
        l0 = be.log.size(); x0 = be.executed.size();
        std::size_t sp0 = be.speedCalls.size();
        o = MotorAccessDispatch(dn, 48, be);
        CHECK(!o.ok && Has(o.ackJson, "安全門") && CountKind(be, x0, kCmdAxMoveAbs) == 0 && CountKind(be, x0, kCmdAxSetSpeed) == 4 && be.speedCalls.size() == sp0 + 7 &&
              Has(o.ackJson, "R144 #3"),
              "door open on ZA (golden MotorMove -1): nothing moves, refused with the reason -- but every Out Z got SetSpeed(10) first, as golden");
        be.doorOpen.clear();

        // ---- [W906] NB2 R169 M2: any axis of the Out arm busy -> refused before anything (no speed, no move)
        int busyBad = 0;
        for (int c = 0; c < 5; ++c) {
            if (c == 0) be.state[2] = 5;                                        // Out Y still moving (PTP_MOT)
            if (c == 1) be.state[3] = 4;                                        // ZA homing state (HOMING)
            if (c == 2) ops.done[oP] = 0;                                       // the Out pitch (a golden-object motor) moving: MotionDone()==false
            if (c == 3) { be.state[3] = 1; be.freezePolls = true; be.cmdPos[3] = 0.0; MotorAccessReq d2 = dn; Fld(d2, "setEditZ2A", 11);
                          o = MotorAccessDispatch(d2, 49, be);                  // a new target (fCMD reset) -> a MoveAbs on ZA, no fresh sample since
                          if (!(o.ok && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().value == 5011.0)) printf("    setup c3: %s\n", o.ackJson.substr(0, 200).c_str()); }
            if (c == 4) { be.freezePolls = false; MotorAccessReq zu = Req("uteach", "teachZAllUp", "MInArmX"); zu.button = "btnOutZAllUp"; be.fixedMots["outZ"] = std::vector<int>(1, oZA);
                          o = MotorAccessDispatch(zu, 50, be);                  // a HOME job (Z All Up) on ZA
                          if (!(o.ok && MotorAccessJobs().homesActive == 1)) printf("    setup c4: %s\n", o.ackJson.substr(0, 200).c_str()); }
            l0 = be.log.size(); x0 = be.executed.size(); std::size_t m0 = be.moves.size(); sp0 = be.speedCalls.size();
            o = MotorAccessDispatch(dn, 51 + c, be);
            const char* expect[5] = { "MOutArmY", "MOutArmZA", "MOutArmPitch", "MOutArmZA", "HOME" };
            if (!(!o.ok && Has(o.ackJson, "R169 M2") && Has(o.ackJson, expect[c]) && Untouched(be, l0, x0, m0, sp0))) {
                ++busyBad; printf("    busy case %d: ok=%d %s\n", c, (int)o.ok, o.ackJson.substr(0, 240).c_str());
            }
            if (c == 0) be.state[2] = 1;
            if (c == 1) be.state[3] = 1;
            if (c == 2) ops.done.erase(oP);
            if (c == 4) MotorAccessDispatch(Req("uteach", "stop", 0), 56, be);
        }
        CHECK(busyBad == 0, "R169 M2: Out Y moving / ZA homing / the Out pitch moving (golden MotionDone) / a command without a fresh sample / a HOME job on ZA -> each refused, nothing sent");
        be.state[7] = 5;                                                        // the IN arm's Y moving
        be.cmdPos[3] = 0.0;
        l0 = be.log.size(); x0 = be.executed.size(); sp0 = be.speedCalls.size();
        std::size_t mv0 = be.moves.size();
        o = MotorAccessDispatch(dn, 57, be);
        CHECK(!o.ok && Has(o.ackJson, "MInArmY") && Has(o.ackJson, "機台上有軸在動") && Untouched(be, l0, x0, mv0, sp0),
              "decision 1 = B: the In arm's Y moving refuses the Out arm's All Down too (every motor of the machine, golden's page lock), nothing sent");
        be.state[7] = 1;

        // ---- [W906] decision 1 = B (20261003): All Down asks every motor of the machine; the two Set All buttons still only their arm
        {
            Add1203(be, "MOutShuttle1", 8, gx);                                // axes of no arm, idle (READY)
            Add1203(be, "MTrayX", 9, gx);
            auto Mark = [&]() { l0 = be.log.size(); x0 = be.executed.size(); mv0 = be.moves.size(); sp0 = be.speedCalls.size(); };
            int bb = 0;
            be.state[8] = 5;                                                    // the Out shuttle moving under the Out arm (PTP_MOT)
            Mark();
            o = MotorAccessDispatch(dn, 101, be);
            if (!(!o.ok && Has(o.ackJson, "MOutShuttle1") && Has(o.ackJson, "機台上有軸在動") && Untouched(be, l0, x0, mv0, sp0))) {
                ++bb; printf("    B shuttle: %s\n", o.ackJson.substr(0, 240).c_str());
            }
            MotorAccessReq so = Req("uteach", "teachSetAllArmZ", "MInArmX");
            so.button = "btnSetAllOutArmZ"; so.kind = "edit";
            Mark();
            o = MotorAccessDispatch(so, 102, be);                               // Set OutArm Z while the shuttle still moves
            const bool setAllGoes = o.ok && EditCount(o.ackJson) == 8 && Untouched(be, l0, x0, mv0, sp0);
            if (!setAllGoes) printf("    B Set All: %s\n", o.ackJson.substr(0, 240).c_str());
            be.state[8] = 1;
            MotorAccessReq jg = Req("uMotorTest", "jogP", "MTrayX"); jg.button = "sbMotorTest_JogP"; jg.num["speed"] = 50;
            const bool jogging = MotorAccessDispatch(jg, 103, be).ok && MotorAccessJobs().jogsActive == 1;   // TrayArm X jogs (Motor Test)
            Mark();
            o = MotorAccessDispatch(dn, 104, be);
            if (!(jogging && !o.ok && Has(o.ackJson, "MTrayX JOG") && Untouched(be, l0, x0, mv0, sp0))) {
                ++bb; printf("    B TrayArm X: jog=%d %s\n", (int)jogging, o.ackJson.substr(0, 240).c_str());
            }
            MotorAccessReq rl = Req("uMotorTest", "stop", "MTrayX"); rl.button = "sbMotorTest_JogP";
            MotorAccessDispatch(rl, 105, be);                                   // the jog released
            CHECK(bb == 0, "decision 1 = B: the Out shuttle moving / TrayArm X jogging (axes of no arm) -> All Down refused, that axis named, nothing sent");
            CHECK(setAllGoes, "decision 1 = B: Set OutArm Z while the Out shuttle moves -> still goes (Set All reads that arm only, the per-arm gate)");
            MotorAccessReq d4 = dn;
            Fld(d4, "setEditZ2A", 13);
            o = MotorAccessDispatch(d4, 106, be);
            CHECK(o.ok && Has(o.ackJson, "\"moving\":1") && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().axis == 3 && be.executed.back().value == 5013.0,
                  "decision 1 = B: nothing busy on the machine any more -> All Down goes (ZA to 5000 + 13)");
            ops.forcedCount = 0;
            Mark();
            o = MotorAccessDispatch(dn, 107, be);
            CHECK(!o.ok && Has(o.ackJson, "TOTAL_MOTOR") && Untouched(be, l0, x0, mv0, sp0), "decision 1 = B: the machine's motor count unknown -> All Down refused (fail closed), nothing sent");
            ops.forcedCount = -1;
            std::string lzd;                                                    // the Live ops (wb_serve only, cannot be linked here): source pins
            if (argc > 1) {
                std::string a1 = argv[1];
                for (std::size_t k = 0; k < a1.size(); ++k) if (a1[k] == '\\') a1[k] = '/';
                const std::string tail = "/../web/JSON/motor-access.json";
                if (a1.size() > tail.size() && a1.compare(a1.size() - tail.size(), tail.size(), tail) == 0)
                    lzd = ReadAll(a1.substr(0, a1.size() - tail.size()) + "/WebTeachZDownLive.cpp");
            }
            CHECK(Has(lzd, "int MotorCount() override { return TOTAL_MOTOR < MAX_TRAY_MOTOR ? TOTAL_MOTOR : MAX_TRAY_MOTOR; }") &&
                  Has(lzd, "MOT[mi].ScanMotorStatus();") && Has(lzd, "return MOT[mi].Led[iInposLed] ? 0 : 1;"),
                  "decision 1 = B, the Live ops: MotorCount = golden TOTAL_MOTOR (VerifyMotorAction's bound); the motion test keeps golden's Galil Index branch");
        }

        // ---- [W906] W5B-4: the screen values come from the page; missing / not an integer / overflow -> refused, nothing touched
        {
            int fb = 0;
            for (int c = 0; c < 4; ++c) {
                MotorAccessReq q = dn;
                if (c == 0) q.obj["fields"].erase("SetEditPickOutSht");
                if (c == 1) q.obj["fields"].erase("setEditZ2H");
                if (c == 2) q.obj["fields"]["setEditZ2E"] = 12.5;
                if (c == 3) { q.obj["fields"]["SetEditPickOutSht"] = 2147483000.0; q.obj["fields"]["setEditZ2A"] = 1000.0; }
                l0 = be.log.size(); x0 = be.executed.size(); std::size_t m0 = be.moves.size(); sp0 = be.speedCalls.size();
                o = MotorAccessDispatch(q, 60 + c, be);
                if (!(!o.ok && Untouched(be, l0, x0, m0, sp0))) { ++fb; printf("    fields case %d: ok=%d %s\n", c, (int)o.ok, o.ackJson.substr(0, 200).c_str()); }
            }
            CHECK(fb == 0, "SetEditPickOutSht missing / setEditZ2H missing / a non-integer field / pick+Z past int -> refused, no speed, no move");
        }

        // ---- refused before anything: SystemStart, a hand teach open, Motor Test, no ops, a grid past [2][8], another button
        {
            int rb = 0;
            l0 = be.log.size(); x0 = be.executed.size(); std::size_t m0 = be.moves.size(); sp0 = be.speedCalls.size();
            be.systemStart = true;
            o = MotorAccessDispatch(dn, 70, be);
            if (!(!o.ok && Has(o.ackJson, "SystemStart"))) { ++rb; printf("    SystemStart: %s\n", o.ackJson.substr(0, 160).c_str()); }
            be.systemStart = false;
            MotorAccessReq mt = dn; mt.source = "uMotorTest";
            o = MotorAccessDispatch(mt, 71, be);
            if (!(!o.ok && Has(o.ackJson, "MotorTest 沒有"))) { ++rb; printf("    Motor Test: %s\n", o.ackJson.substr(0, 160).c_str()); }
            TeachZDownInstallOps(0);
            o = MotorAccessDispatch(dn, 72, be);
            if (!(!o.ok && Has(o.ackJson, "沒有安裝"))) { ++rb; printf("    no ops: %s\n", o.ackJson.substr(0, 160).c_str()); }
            TeachZDownInstallOps(&ops);
            ops.out.motRow = 3;
            o = MotorAccessDispatch(dn, 73, be);
            if (!(!o.ok && Has(o.ackJson, "[2][8]"))) { ++rb; printf("    grid: %s\n", o.ackJson.substr(0, 160).c_str()); }
            ops.out.motRow = 2;
            ops.ok = false; ops.why = "fake: no grid";
            o = MotorAccessDispatch(dn, 74, be);
            if (!(!o.ok && Has(o.ackJson, "fake: no grid"))) { ++rb; printf("    grid unreadable: %s\n", o.ackJson.substr(0, 160).c_str()); }
            ops.ok = true;
            MotorAccessReq bb = dn; bb.button = "btnInZAllDown";
            o = MotorAccessDispatch(bb, 75, be);
            if (o.ok) { ++rb; printf("    unknown button passed\n"); }
            CHECK(rb == 0 && Untouched(be, l0, x0, m0, sp0),
                  "SystemStart / Motor Test source / ops not installed / a grid past golden's [2][8] / no grid / another button -> refused, nothing sent");
            be.golden[iZA].homeFlag = 1;
            o = MotorAccessDispatch(TeachReq("teachSet", "SetButton030", true, true), 76, be);   // hand teach on MInArmZA (golden fTeachShow modal)
            CHECK(o.ok && MotorAccessJobs().teachActive, "setup: hand teach open on MInArmZA");
            l0 = be.log.size(); x0 = be.executed.size(); m0 = be.moves.size(); sp0 = be.speedCalls.size();
            o = MotorAccessDispatch(dn, 77, be);
            CHECK(!o.ok && Has(o.ackJson, "手動教導中") && Untouched(be, l0, x0, m0, sp0), "hand teach open -> All Down refused (golden fTeachShow is ShowModal)");
            MotorAccessReq hend = TeachReq("teachSet", "SetButton030", true, false); hend.flag["accept"] = false;
            CHECK(MotorAccessDispatch(hend, 78, be).ok && !MotorAccessJobs().teachActive, "setup: hand teach cancelled");
        }

        // ---- Set OutArm Z (ep8, base [0][2] = ZE in the grid): every edit = ReadPos - ReadPos(ZE), base 0; nothing moves
        be.cmdPos[3] = 3000.0;                                                  // ZA (PCI1203 Enable=1) -> the monitor
        const int oRead[8] = { 3000, 100, 250, 300, 10, 20, 30, 40 };           // ZA ZC ZE ZG / ZB ZD ZF ZH
        for (int k = 1; k < 8; ++k) be.readPos[oZ[k]] = oRead[k];              // MN200 Enable=0 -> golden ReadPos (cached Position)
        MotorAccessReq sa = Req("uteach", "teachSetAllArmZ", "MInArmX");
        sa.button = "btnSetAllOutArmZ"; sa.kind = "edit";
        l0 = be.log.size(); x0 = be.executed.size(); std::size_t m0 = be.moves.size(); sp0 = be.speedCalls.size();
        o = MotorAccessDispatch(sa, 80, be);
        {
            int bad = 0;
            for (int k = 0; k < 8; ++k) { long long v = 0; if (!EditOf(o.ackJson, oEdit[k], v) || v != oRead[k] - 250) { ++bad; printf("    %s = %lld (want %d)\n", oEdit[k], v, oRead[k] - 250); } }
            CHECK(o.ok && AckShapeOk(o.ackJson, 7) && bad == 0 && EditCount(o.ackJson) == 8 && Has(o.ackJson, "\"base\":\"setEditZ2E\"") && Has(o.ackJson, "\"standZ\":250"),
                  "Set OutArm Z: each Out Z edit = its ReadPos - ReadPos(ZE) (golden :4400 / :4405 / :4429-4433); the base setEditZ2E = 0");
        }
        CHECK(Untouched(be, l0, x0, m0, sp0) && be.teachCanMoveCalls.empty(), "Set All moves nothing, sets no speed, asks no CheckCanMove (edits only, golden)");

        // ---- Set InArm Z: a PCI1203 Enable=0 Z and a Z this Mot_Table lacks read golden's cached Position (NB2 R144 #7 / #8)
        be.cmdPos[4] = 1000.0;                                                  // In ZA
        be.cmdPos[5] = 99999.0; be.readPos[iZC] = 120;                          // In ZC: PCI1203 Enable=0 -> NOT the card, golden's cached Position
        const int iRead[8] = { 1000, 120, 250, 330, 10, 20, 30, 40 };
        for (int k = 2; k < 8; ++k) be.readPos[iZ[k]] = iRead[k];              // incl. MInArmZG (no Mot_Table row)
        const char* iEdit[8] = { "setEditZ1A", "setEditZ1C", "setEditZ1E", "setEditZ1G", "setEditZ1B", "setEditZ1D", "setEditZ1F", "setEditZ1H" };
        MotorAccessReq si = sa; si.button = "btnSetAllInArmZ";
        o = MotorAccessDispatch(si, 81, be);
        {
            int bad = 0;
            for (int k = 0; k < 8; ++k) { long long v = 0; if (!EditOf(o.ackJson, iEdit[k], v) || v != iRead[k] - 250) { ++bad; printf("    %s = %lld (want %d)\n", iEdit[k], v, iRead[k] - 250); } }
            CHECK(o.ok && bad == 0 && Has(o.ackJson, "PCI1203 Enable=0") && Has(o.ackJson, "Mot_Table 沒有這一列"),
                  "Set InArm Z: In ZC (PCI1203, Enable=0) reads golden's cached Position 120, not the card's 99999 (R144 #7); MInArmZG (not in Mot_Table) reads golden's MOT ReadPos 330 (R144 #8)");
        }

        // ---- NB2 R144 #2: ep1Picker -- grid 1x1, base [0][2] off the grid: golden takes setEditZ1E's screen text, then writes 0 into it
        ops.in.motRow = ops.in.motCol = ops.in.maxRow = ops.in.maxCol = 1; ops.in.pickerCount = 4;
        o = MotorAccessDispatch(si, 82, be);
        CHECK(!o.ok && Has(o.ackJson, "setEditZ1E") && Has(o.ackJson, "R144 #2"), "ep1Picker, the page did not send setEditZ1E -> refused (golden would atoi a blank as 0), no edit");
        MotorAccessReq s1 = si; Fld(s1, "setEditZ1E", 7);
        o = MotorAccessDispatch(s1, 83, be);
        {
            long long a = 0, e = 1;
            CHECK(o.ok && EditOf(o.ackJson, "setEditZ1A", a) && a == 1000 - 7 && EditOf(o.ackJson, "setEditZ1E", e) && e == 0 && EditCount(o.ackJson) == 2 &&
                  Has(o.ackJson, "\"baseInGrid\":false"),
                  "ep1Picker: setEditZ1A = ReadPos(ZA) 1000 - the screen's setEditZ1E 7 = 993, and setEditZ1E written 0 (golden :4335 / :4359-4363)");
        }

        // ---- ep4Picker (USE_PICKER_COUNT==0): row 0 reads InArmZIndex[i][i] (= ZA for every column), row 1 is 0, the base keeps its ReadPos
        ops.in.motRow = 2; ops.in.motCol = 4; ops.in.maxRow = 2; ops.in.maxCol = 4; ops.in.pickerCount = 0;
        be.table["MInArmZC"].tableEnable = true; be.cmdPos.erase(5);            // In ZC readable only if golden read it -- ep4 never does ([i][i])
        o = MotorAccessDispatch(si, 84, be);
        {
            const long long want[8] = { 0, 0, 1000, 0, 0, 0, 0, 0 };            // ZA ZC ZE(base) ZG / row 1
            int bad = 0;
            for (int k = 0; k < 8; ++k) { long long v = -1; if (!EditOf(o.ackJson, iEdit[k], v) || v != want[k]) { ++bad; printf("    ep4 %s = %lld (want %lld)\n", iEdit[k], v, want[k]); } }
            CHECK(o.ok && bad == 0 && Has(o.ackJson, "USE_PICKER_COUNT==0"),
                  "ep4Picker, base [0][2]: row 0 = ReadPos(InArmZIndex[0][0]) minus it = 0, the base setEditZ1E keeps 1000, row 1 = 0 (golden :4318-4321 / :4343-4349, [i][i] sic)");
        }
        ops.in.yBase = 1; ops.in.xBase = 1;
        o = MotorAccessDispatch(si, 85, be);
        {
            const long long want[8] = { 1000, 1000, 1000, 1000, 0, 0, 0, 0 };
            int bad = 0;
            for (int k = 0; k < 8; ++k) { long long v = -1; if (!EditOf(o.ackJson, iEdit[k], v) || v != want[k]) { ++bad; printf("    ep4/[1][1] %s = %lld (want %lld)\n", iEdit[k], v, want[k]); } }
            CHECK(o.ok && bad == 0, "ep4Picker, base [1][1]: stand = row 1's 0 -> row 0 keeps ReadPos(ZA) 1000, row 1 = 0");
        }
        ops.in.yBase = 0; ops.in.xBase = 2; ops.in.pickerCount = 1;
        o = MotorAccessDispatch(si, 86, be);
        CHECK(!o.ok && Has(o.ackJson, "MInArmZC") && Has(o.ackJson, "一格都沒寫"), "control: ep8 reads In ZC ([i][j]) -> its missing sample refuses the press, no edit");
        be.cmdPos[5] = 120.0;
        be.freezePolls = true;
        MotorAccessReq jz = Req("uMotorTest", "moveAbsolute", "MInArmZC");      // a command on In ZC through the Motor Test Go, no sample after it
        jz.num["targetPos"] = 50;
        o = MotorAccessDispatch(jz, 88, be);
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveAbs && be.executed.back().axis == 5, "setup: In ZC commanded (Motor Test Go)");
        o = MotorAccessDispatch(si, 89, be);
        CHECK(!o.ok && Has(o.ackJson, "MInArmZC") && (Has(o.ackJson, "R169 M2") || Has(o.ackJson, "監看器")),
              "a 1203 nozzle commanded without a fresh sample since -> refused, no edit (not a pre-command value)");
        be.freezePolls = false;
        be.state[6] = 5;                                                        // In X moving
        o = MotorAccessDispatch(si, 90, be);
        CHECK(!o.ok && Has(o.ackJson, "MInArmX") && Has(o.ackJson, "R169 M2"), "Set InArm Z while In X moves -> refused (R169 M2: golden's page lock)");
        be.state[6] = 1;
        MotorAccessReq smt = si; smt.source = "uMotorTest";
        CHECK(!MotorAccessDispatch(smt, 91, be).ok, "Set All from Motor Test -> refused");
        MotorAccessReq sbb = si; sbb.button = "btnSetAllSortArmZ";
        CHECK(!MotorAccessDispatch(sbb, 92, be).ok, "btnSetAllSortArmZ is not wired here -> refused");

        // ---- the catalog rows (web/JSON/motor-access.json, argv[1]): source uteach; All Down kind motion, the two Set All kind edit
        if (argc > 1) {
            std::ifstream f(argv[1], std::ios::binary);
            std::stringstream ss; ss << f.rdbuf();
            cJSON* cat = cJSON_Parse(ss.str().c_str());
            const cJSON* cm = cat ? cJSON_GetObjectItemCaseSensitive(cat, "commands") : 0;
            const char* rb[3] = { "btnSetAllInArmZ", "btnSetAllOutArmZ", "btnOutZAllDown" };
            int seen[3] = { 0, 0, 0 }, bad = 0;
            const cJSON* e = 0;
            if (cJSON_IsArray(cm)) cJSON_ArrayForEach(e, cm) {
                const cJSON* bt = cJSON_GetObjectItemCaseSensitive(e, "button");
                if (!cJSON_IsString(bt)) continue;
                for (int k = 0; k < 3; ++k) {
                    if (std::string(bt->valuestring) != rb[k]) continue;
                    ++seen[k];
                    const cJSON* src = cJSON_GetObjectItemCaseSensitive(e, "source");
                    const cJSON* ac = cJSON_GetObjectItemCaseSensitive(e, "action");
                    const cJSON* kd = cJSON_GetObjectItemCaseSensitive(e, "kind");
                    const cJSON* ps = cJSON_GetObjectItemCaseSensitive(e, "params");
                    const bool okRow = cJSON_IsString(src) && std::string(src->valuestring) == "uteach" && cJSON_IsString(ac) &&
                                       std::string(ac->valuestring) == (k < 2 ? "teachSetAllArmZ" : "teachOutZAllDown") && cJSON_IsString(kd) &&
                                       std::string(kd->valuestring) == (k < 2 ? "edit" : "motion") && cJSON_IsArray(ps) && cJSON_GetArraySize(ps) == 1 &&
                                       cJSON_IsString(cJSON_GetArrayItem(ps, 0)) && std::string(cJSON_GetArrayItem(ps, 0)->valuestring) == "fields";
                    if (!okRow) { ++bad; printf("    catalog row %s is not {uteach, %s, %s, [fields]}\n", rb[k], k < 2 ? "teachSetAllArmZ" : "teachOutZAllDown", k < 2 ? "edit" : "motion"); }
                }
            }
            cJSON_Delete(cat);
            CHECK(bad == 0 && seen[0] == 1 && seen[1] == 1 && seen[2] == 1 &&
                  std::string(MotorAccessActionStatus("teachSetAllArmZ")) == "live" && std::string(MotorAccessActionStatus("teachOutZAllDown")) == "live",
                  "catalog: uteach btnSetAllIn/OutArmZ -> teachSetAllArmZ (edit), btnOutZAllDown -> teachOutZAllDown (motion), params [fields]; both live kActions rows");
        }
        TeachZDownInstallOps(0);
        MotorAccessResetJobs();
    }

    // ---------------------------------------------------------------- (MSGBOX)
    //AI(W906-MSGBOX-ABANDON) 20261003: INBOX 150 -- golden uMotorTest Timer1Timer :921-926 / uteach Timer1Timer :1364-1368: a MyMessageBox on
    //  screen raises btnLoopMove / btnHome (and bSingleHome=false), not only the alarm note.  MotorAccessOnMessageBox (WebMotorAccess.cpp
    //  EOF), called by wb_serve's ShowMyMessage / ShowUnloaderTrayMessage hosts: a box that stops the motors (golden StopAllMotor) ends the
    //  HOME / LoopMove jobs, forgets the jogs and stops every opened 1203 axis, WITHOUT the alarm-lamp rule (golden UpdateMotorLed :644-652
    //  is fNote-only); a NonStop box (FormShow :302-310 stops nothing) ends the Motor Test HOME and the loop but stops nothing, keeps the
    //  jog and leaves a Teach Z All Up home running (golden DoZHome).  Source pins: the two hosts call it right after the Arm Cell edge.
    printf("[MSGBOX] INBOX 150: a message box abandons Motor Test HOME / LoopMove (golden Timer1Timer :921-926)\n");
    {
        MotorGolden g;
        g.valid = true; g.enable = true; g.gearRatio = 1.0; g.softP = 999999; g.softN = -999999; g.homeFlag = 1;
        g.jogHigh = 10000; g.initSpeed = 500; g.homeHigh = 2000; g.homeLow = 500; g.accDb = 3000; g.decDb = 3100; g.acc = 3000; g.dec = 3100;
        MotorGolden gz = g;  gz.armZ = true;
        // a Motor Test HOME (MOutArmX), a LoopMove (MOutArmY, the selected motor), a jog (MTrayX), a Teach Z All Up home (MInArmZA)
        auto Jobs = [&](FakeBackend& b, int seq0) -> bool {
            MotorAccessResetJobs();
            Add1203(b, "MOutArmX", 1, g); Add1203(b, "MOutArmY", 2, g); Add1203(b, "MTrayX", 3, g); Add1203(b, "MInArmZA", 4, gz);
            b.fixedMots["inZ"] = std::vector<int>(1, TestMot("MInArmZA"));
            bool ok = MotorAccessDispatch(Req("uMotorTest", "selectMotor", "MOutArmY"), seq0, b).ok;          // golden lM00Click: Motor Test shown
            ok = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MOutArmX", true), seq0 + 1, b).ok && ok;
            MotorAccessReq lr = ReqStart("uMotorTest", "loopMove", "MOutArmY", true);
            lr.num["pos1"] = 2000; lr.num["pos2"] = 500; lr.num["waitTime"] = 5; lr.num["speed"] = 50;
            ok = MotorAccessDispatch(lr, seq0 + 2, b).ok && ok;
            ok = MotorAccessDispatch(Req("uMotorTest", "jogP", "MTrayX"), seq0 + 3, b).ok && ok;
            MotorAccessReq zu = Req("uteach", "teachZAllUp", "MInArmX"); zu.button = "btnInZAllUp";
            ok = MotorAccessDispatch(zu, seq0 + 4, b).ok && ok;
            const MotorAccessJobState j = MotorAccessJobs();
            return ok && j.homesActive == 2 && j.loopActive && j.jogsActive == 1 && j.pageShown;
        };
        auto Stops = [](const FakeBackend& b, std::size_t from, std::set<int>& axes) {
            int n = 0;
            for (std::size_t i = from; i < b.executed.size(); ++i) if (b.executed[i].kind == kCmdAxStop) { ++n; axes.insert(b.executed[i].axis); }
            return n;
        };
        {   // ShowMyMessage (SystemInitialOK): golden StopAllMotor :830-831 + FormShow, then Timer1 raises the buttons
            FakeBackend b;
            CHECK(Jobs(b, 300), "setup: Motor Test shown; a HOME, a LoopMove, a jog and a Teach Z All Up home running");
            b.motionIO[2] = 2;                                                  // the selected MOutArmY's ALM lamp is on
            const int miY = TestMot("MOutArmY");
            const std::size_t hf = b.homeFlags.count(miY), x0 = b.executed.size();
            MotorAccessOnMessageBox(b, "訊息框", "test message", true);
            const MotorAccessJobState j = MotorAccessJobs();
            std::set<int> axes;
            const int n = Stops(b, x0, axes);
            int opened = 0;
            for (std::size_t k = 0; k < b.opened.size(); ++k) if (b.opened[k]) ++opened;
            CHECK(j.homesActive == 0 && !j.loopActive && j.jogsActive == 0,
                  "a box that stops the motors: the Motor Test HOME, the Teach Z All Up home (as the alarm edge), the LoopMove and the jog are gone");
            CHECK(n == opened && (int)axes.size() == opened && opened >= 5,
                  "every opened 1203 axis got the stop -- the 1203 half of golden StopAllMotor, which golden's own objects cannot reach here");
            CHECK(b.homeFlags.count(miY) == hf, "no alarm-lamp rule: the selected motor's HomeFlag is untouched (golden UpdateMotorLed :644-652 needs fNote->fShow)");
            CHECK(Has(j.lastNote, "message box 訊息框") && Has(j.lastNote, ":921-926"), "the job note names the box and golden's lines");
        }
        {   // control: the alarm edge on the same picture does apply the alarm-lamp rule
            FakeBackend b;
            Jobs(b, 310);
            b.motionIO[2] = 2;
            const int miY = TestMot("MOutArmY");
            MotorAccessOnAlarm(b, "WAR0001");
            CHECK(b.homeFlags.count(miY) == 1 && b.homeFlags[miY] == 0, "control: MotorAccessOnAlarm (fNote) sets the selected motor's HomeFlag 0 on its ALM lamp");
        }
        {   // ShowUnloaderTrayMessage with iUnLoaderCount != 0: FormShow stops nothing; Timer1 still raises the buttons
            FakeBackend b;
            CHECK(Jobs(b, 320), "setup: the same four jobs");
            const std::size_t x0 = b.executed.size();
            MotorAccessOnMessageBox(b, "訊息框（不停機）", "Auto tray full", false);
            const MotorAccessJobState j = MotorAccessJobs();
            CHECK(j.homesActive == 1 && j.homeMotors.size() == 1 && j.homeMotors[0] == "MInArmZA" && !j.loopActive,
                  "a NonStop box: the Motor Test HOME and the LoopMove end; the Teach Z All Up home goes on (golden DoZHome does not look at MyMessageBox)");
            CHECK(j.jogsActive == 1 && CountKind(b, x0, kCmdAxStop) == 0, "nothing is stopped and the jog stays (golden stops nothing for a NonStop box)");
        }
        MotorAccessResetJobs();
        {   // the hosts (tools/wb_serve.cpp cannot be linked into a ctest -- the ARMCELL2 / test_note_motorerror practice): source pins
            std::string a1 = (argc > 1) ? std::string(argv[1]) : std::string();
            for (std::size_t k = 0; k < a1.size(); ++k) if (a1[k] == '\\') a1[k] = '/';
            const std::string tail = "/../web/JSON/motor-access.json";
            const bool shaped = a1.size() > tail.size() && a1.compare(a1.size() - tail.size(), tail.size(), tail) == 0;
            const std::string root = shaped ? a1.substr(0, a1.size() - tail.size()) : std::string();
            const std::string ws = shaped ? ReadAll(root + "/tools/wb_serve.cpp") : std::string();
            const std::string lv = shaped ? ReadAll(root + "/WebMotorAccessLive.cpp") : std::string();
            auto Pin = [](const std::string& s, const char* fn, const char* first, const char* call, const char* before) {   // in fn: first < call < before; call is code
                const std::size_t f = s.find(fn);
                if (f == std::string::npos) return 0;
                const std::size_t a = s.find(first, f), c = s.find(call, f), b = s.find(before, f);
                if (a == std::string::npos || c == std::string::npos || b == std::string::npos || !(a < c && c < b)) return 0;
                const std::size_t ls = s.rfind('\n', c);
                const std::string head = s.substr(ls == std::string::npos ? 0 : ls + 1, c - (ls == std::string::npos ? 0 : ls + 1));
                return head.find("//") == std::string::npos ? 1 : 0;
            };
            CHECK(shaped && !ws.empty() && !lv.empty(), "source pins: argv[1] gives the tree; tools/wb_serve.cpp and WebMotorAccessLive.cpp read");
            const int pins =
                Pin(ws, "void W906MbShowMyMessage(const char* s1, const char* s2, const char* s3, bool Ok, bool bServoOff)",
                    "W906_MotorAccessArmCellOnPopup(\"訊息框\", s1);", "W906_MotorAccessOnMessageBox(\"訊息框\", s1, SystemInitialOK == true);", "MbWait(qid, kOpts, 2);") +
                Pin(ws, "void W906MbShowUnloaderTray(const char* s1, const char* s2)",
                    "W906_MotorAccessArmCellOnPopup(v.nonStop ? \"訊息框（不停機）\" : \"訊息框\", s1);",
                    "W906_MotorAccessOnMessageBox(v.nonStop ? \"訊息框（不停機）\" : \"訊息框\", s1, !v.nonStop && SystemInitialOK == true);", "MbFormShow();") +
                Pin(lv, "void W906_MotorAccessOnMessageBox(const char* kind, const char* what, bool stopsMotors)", "{",
                    "ht9045::MotorAccessOnMessageBox(ht9045::MotorAccessLiveBackend(), kind ? kind : \"\", what ? what : \"\", stopsMotors);", "}");
            CHECK(pins == 3, "ShowMyMessage (stops when SystemInitialOK, golden :830-831) and ShowUnloaderTrayMessage (stops only when not NonStop, golden FormShow "
                             ":302-310) call the message-box edge as code, after the Arm Cell edge and before the wait / FormShow; the Live thunk forwards it");
        }
    }

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
