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
//
//  NOT COVERED：真實後端（WebMotorAccessLive.cpp：HSys.MotTable／MOT[]／監看器的對號）—— 那一半要
//  wb_serve 起來後用 WS 探針量；1203 那一段要有卡的機台。
// =============================================================================
#include "WebMotorAccess.h"
#include "Public/cJSON.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace ht9045;

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

    std::vector<Pci1203Cmd>  executed;
    std::vector<std::string> stopAll;         // GoldenStopAll 的 source
    std::vector<int>         stopMotor;
    std::vector<std::pair<int, bool> > servoSet;
    std::vector<std::string> refusals;

    FakeBackend() : ready(true), rejectNext(false), dry(false), doorClosed(true), canRun(true), moveRet(0),
                    zSafe(20), systemStart(false), resetCount(0),
                    polls(0), freezePolls(false), floodReady(true), readSpeedV(0), retNext(0), safeLock(false) {}
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
    Pci1203CmdResult Pci1203Execute(const Pci1203Cmd& c) override
    {
        executed.push_back(c);
        Pci1203CmdResult r;
        r.ret = 0;
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
    void GoldenStopMotor(int mi) override { stopMotor.push_back(mi); }
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
    bool GoldenSystemStart() override { return systemStart; }
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

int main(int argc, char** argv)
{
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
        }
        cJSON_Delete(cat);
        CHECK(n == 38, "38 catalog commands");
        CHECK(actions.size() == 28, "28 distinct actions");
        CHECK(unknown == 0, "every catalog action has a status");
        CHECK(live.size() == 19 && live.count("stop") && live.count("servoToggle") && live.count("jogP") &&
              live.count("moveRelative") && live.count("moveAbsolute") && live.count("moveSoftLimitN") &&
              live.count("setJogHighSpeed") && live.count("setSoftLimitP") && live.count("home") &&
              live.count("loopMove") && live.count("resetMNet") && live.count("setRangeAndInit"),
              "live = stop/servo + W4-b1 (12) + W4-b2 (5)");
        CHECK(std::string(MotorAccessActionStatus("motorPowerToggle")) == "blocked" &&
              std::string(MotorAccessActionStatus("reloadMotorData")) == "blocked", "blocked = motorPowerToggle, reloadMotorData");
        CHECK(ui.size() == 3 && ui.count("setPos1") && ui.count("setPos2") && ui.count("refreshParameter"),
              "ui = {setPos1, setPos2, refreshParameter}");
        CHECK(std::string(MotorAccessActionStatus("noSuchAction")) == "unknown", "unknown action -> unknown");
    }

    // ---------------------------------------------------------------- (3)
    printf("[3] honesty: queued / unknown never touch the backend\n");
    {
        FakeBackend be;
        be.table["MInArmX"] = Axis1203(0, 0, 0, 0);
        be.opened.assign(2, true);
        const char* queued[] = { "motorPowerToggle", "reloadMotorData" };
        bool allRefused = true, allSayW4b = true;
        for (unsigned i = 0; i < sizeof(queued) / sizeof(queued[0]); ++i) {
            const MotorAccessOutcome o = MotorAccessDispatch(Req("uMotorTest", queued[i], "MInArmX"), 1, be);
            if (o.ok) allRefused = false;
            if (!Has(o.ackJson, "不做")) allSayW4b = false;
        }
        CHECK(allRefused, "every blocked action -> ok=false");
        const MotorAccessOutcome tj = MotorAccessDispatch(Req("uteach", "jogP", "MInArmX"), 1, be);
        CHECK(!tj.ok && Has(tj.ackJson, "W5"), "uteach jogP (teach motion) -> refused, names W5");
        CHECK(allSayW4b, "refusal says why (missing dependency / EastSun ruling)");
        const MotorAccessOutcome t = MotorAccessDispatch(Req("uteach", "teachGo", "MInArmX"), 1, be);
        CHECK(!t.ok && Has(t.ackJson, "W5"), "teachGo -> refused, names W5");
        CHECK(be.Calls() == 0, "no backend call for any queued action (nothing moves)");

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
        CHECK(be.stopMotor.size() == 1 && be.stopMotor[0] == 2, "selected non-1203 motor -> MOT[2].PCIL132_StopMotor");

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

        MotorAccessReq r = Req("uMotorTest", "jogP", "MOutArmY"); r.button = "sbMotorTest_JogP"; r.num["speed"] = 50;
        MotorAccessOutcome o = MotorAccessDispatch(r, 11, be);
        bool seqOk = o.ok && be.executed.size() == 5 &&
                     be.executed[0].kind == kCmdAxSetSpeed && be.executed[0].speed == kSpeedInit && be.executed[0].value == 250.0 &&
                     be.executed[1].speed == kSpeedRun && be.executed[1].value == 5000.0 &&
                     be.executed[2].speed == kSpeedAcc && be.executed[3].speed == kSpeedDec &&
                     be.executed[4].kind == kCmdAxMoveVel && be.executed[4].dir == 1 && be.executed[4].axis == 3;
        CHECK(seqOk, "jogP 1203: 4 x setSpeed (golden SetSpeed) then moveVel +1 (EastSun, not Acm_AxJog)");
        r.action = "jogN"; r.button = "sbMotorTest_JogN";
        o = MotorAccessDispatch(r, 11, be);
        CHECK(o.ok && be.executed.size() == 10 && be.executed[9].dir == -1, "jogN -> moveVel -1");

        const int before = (int)be.executed.size();
        MotorAccessReq d1 = Req("uMotorTest", "jogP", "MInArmX"); d1.num["speed"] = 50;
        o = MotorAccessDispatch(d1, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "Direction=1") && (int)be.executed.size() == before, "Direction=1 1203 axis -> refused, nothing sent (§0 #5)");

        be.cmdPos[3] = 400000.0;                               // user 1,000,000
        o = MotorAccessDispatch(r, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "over limitation") && (int)be.executed.size() == before, "|NowPos|>999999 -> refused");
        be.cmdPos[3] = 400.0;
        be.doorClosed = false;
        o = MotorAccessDispatch(r, 1, be);
        CHECK(!o.ok && (int)be.executed.size() == before, "CheckSafeDoorIsClosed false -> refused");
        be.doorClosed = true;
        MotorAccessReq ns = r; ns.num.erase("speed");
        CHECK(!MotorAccessDispatch(ns, 1, be).ok, "jog without speed -> refused");
        be.doorOpen.insert(20);
        o = MotorAccessDispatch(r, 1, be);
        CHECK(!o.ok && (int)be.executed.size() == before, "TMyMotor::JogP safe-door check -> refused");
        be.doorOpen.clear();

        MotorAccessReq jm = Req("uMotorTest", "jogP", "MInArmPitch"); jm.num["speed"] = 30;
        o = MotorAccessDispatch(jm, 1, be);
        CHECK(o.ok && be.jogs.size() == 1 && be.jogs[0].mi == 2 && be.jogs[0].pos && be.jogs[0].pct == 30 &&
              (int)be.executed.size() == before, "non-1203 jog -> golden MOT[2].JogP(30), no 1203 command");

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
        CHECK(!o.ok && Has(o.ackJson, "Direction=1"), "setSoftLimitN on Direction=1 axis -> refused (§0 #5)");
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
        CHECK(!MotorAccessDispatch(ReqStart("uMotorTest", "home", "MInArmPitch", true), 1, be).ok, "non-1203 home -> refused (ProcessSingleMotorHome is a stub)");
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "MOutArmY", true), 27, be);
        be.state[3] = 4; MotorAccessTick(be, 500);
        MotorAccessDispatch(Req("uMotorTest", "stop", 0), 28, be);
        CHECK(MotorAccessJobs().homesActive == 0, "STOP cancels homing (golden AllBtnUp + bSingleHome=false)");
        CHECK(!MotorAccessDispatch(Req("uteach", "home", "MOutArmY"), 1, be).ok, "uteach home -> W5");

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
        CHECK(!MotorAccessDispatch(rr, 1, be).ok && be.rangeRates.empty(), "1203 range -> refused (InitMotor opens the axis the golden way)");
        rr.motors[0] = "MInArmPitch";
        o = MotorAccessDispatch(rr, 1, be);
        CHECK(o.ok && be.rangeRates.size() == 1 && be.rangeRates[0].mi == 2 && be.rangeRates[0].range && be.rangeRates[0].v == 100,
              "MN200 range -> golden SetRange + InitMotor + HomeFlag=0");
        o = MotorAccessDispatch(Req("uMotorTest", "motorPowerToggle", 0), 1, be);
        CHECK(!o.ok && Has(o.ackJson, "G9"), "motorPowerToggle -> blocked, names the missing brake-release dependency");
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
        int okN = 0, shapeN = 0;
        for (std::size_t i = 0; i < rs.size(); ++i) {
            const MotorAccessOutcome o = MotorAccessDispatch(rs[i], 1, be);
            if (!o.ok) { printf("    not ok: %s -> %s\n", rs[i].action.c_str(), o.ackJson.c_str()); continue; }
            ++okN;
            if (AckShapeOk(o.ackJson, 7)) ++shapeN;
            else printf("    bad shape: %s -> %s\n", rs[i].action.c_str(), o.ackJson.c_str());
        }
        CHECK(okN == (int)rs.size(), "all 13 success paths answered ok");
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
        CHECK(o.ok && be.executed.back().kind == kCmdAxMoveVel, "R22 sim build (Motor->Enable=false, Mot_Table Enable=1) -> jog allowed: the guard reads the table, not golden's simulation flag");
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
        CHECK(!MotorAccessDispatch(q, 1, be).ok && !MotorAccessJobs().loopActive, "loop mode All -> refused honestly (was: silently single-axis)");
        q.num.erase("mode");
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().loopActive && Has(o.ackJson, "\"loopActive\":true"), "loop start=true -> looping");
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(q, 1, be);
        CHECK(o.ok && MotorAccessJobs().loopActive && (int)be.executed.size() == n0, "loop start=true again -> no restart, nothing sent");

        // W4C-6: busy guard
        q = Req("uMotorTest", "jogP", "B"); q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "LoopMove 進行中") && (int)be.executed.size() == n0, "W4C-6 jog on another axis while looping -> refused (golden btnLoopMove->Down)");
        q = Req("uMotorTest", "moveRelative", "A"); q.button = "sbMotorTest_MoveP"; q.num["interval"] = 10;
        CHECK(!MotorAccessDispatch(q, 1, be).ok, "W4C-6 moveRelative while looping -> refused");
        q = Req("uMotorTest", "moveSoftLimitN", "B");
        CHECK(!MotorAccessDispatch(q, 1, be).ok, "W4C-6 moveSoftLimit while looping -> refused");
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
        n0 = (int)be.executed.size();
        MotorAccessOnAlarm(be, "JAM0101");
        CHECK(!MotorAccessJobs().loopActive && (int)be.executed.size() == n0 + 6 && be.executed.back().kind == kCmdAxStop,
              "W4C-4 alarm -> loop cancelled and stop sent to all 6 opened 1203 axes (golden StopAllMotor)");

        // servo cancels jobs (golden btnServoOffClick AllBtnUp)
        MotorAccessDispatch(q, 1, be);
        q = Req("uMotorTest", "servoToggle", "B"); q.flag["servoOn"] = true;
        MotorAccessDispatch(q, 1, be);
        CHECK(!MotorAccessJobs().loopActive, "servoToggle (uMotorTest) cancels the loop (golden AllBtnUp)");

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
        bool speedOk = (int)be.executed.size() == n0 + 5;
        for (int i = 0; speedOk && i < 4; ++i) speedOk = be.executed[n0 + i].kind == kCmdAxSetSpeed && be.executed[n0 + i].axis == 3;
        speedOk = speedOk && be.executed[n0].value == 500 && be.executed[n0 + 1].value == 2000 &&
                  be.executed[n0 + 2].value == 3000 && be.executed[n0 + 3].value == 3100 && be.executed[n0 + 4].kind == kCmdAxHome;
        CHECK(o.ok && speedOk && Has(o.ackJson, "\"homeActive\":true"),
              "W4C-3 home -> PTP speed = velLow PHomeLow 500, velHigh PHomeHigh 2000, acc/dec = database 3000/3100, then Acm_AxHome");
        n0 = (int)be.executed.size();
        o = MotorAccessDispatch(ReqStart("uMotorTest", "home", "A", true), 1, be);
        CHECK(o.ok && (int)be.executed.size() == n0 && MotorAccessJobs().homesActive == 1, "home start=true while homing -> no second command");
        q = Req("uMotorTest", "jogN", "B"); q.num["speed"] = 10;
        o = MotorAccessDispatch(q, 1, be);
        CHECK(!o.ok && Has(o.ackJson, "HOME 進行中"), "W4C-6 jog while another axis homes -> refused (golden btnHome->Down)");
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
        CHECK(!MotorAccessDispatch(rr, 1, be).ok && !MotorAccessJobs().loopActive, "setRate on 1203 refused, but the loop is cancelled first (golden :1366)");
        MotorAccessResetJobs();
    }

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
