// ===========================================================================
//  tests/test_motor_points.cpp
//
//  AI(W906-MOTOR-POINTS) 20260924: JsonBridge/ChanMotorPoints.cpp 的測試
//  （GET /api/struct/motor/config 與 /api/struct/motor/runtime 的產生器）。
//
//    [1] 讀版控的 machines/HT9050/Mot_Table.csv（argv[1]），MotorConfigJson 的形狀：
//        軸數、motorId 唯一、index.byId 對得上、MInArmX 的 motorIndex／速度參數跟 CSV 同。
//    [2] MotorRuntimeJson：沒有任何驅動物件 -> 每一軸 quality=nosource、位置全 null（不是 0）、
//        runtime.connected=false 且附 why。
//    [3] 給 MOT[0] 掛一顆 TMySimMotor、塞快取欄位 -> 只有那一軸 good，
//        cmdPos／encPos／targetPos／homeDone／homeFlag 與 MOT[0] 的欄位逐一相同；其他軸仍 nosource。
//    [4] HomeFlag==2 -> homeDone=false 且 errText 說明歸零失敗。
//
//  用法：test_motor_points <Mot_Table.csv>
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"     // AI(W906-IOWEB-P5) 20260924: SOFT_SIMULTE decided before the #ifdef in [3]/[4]
#include "JsonBridge/ChanMotor.h"
#include "Public/cJSON.h"
#include "database.h"
#include "Motor/mymotor.h"
#include "Motor/mySimMotor.h"

#include <cmath>     // AI(W906-MT-E3c) 20260925: std::fabs
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

using namespace ht9045::sjson;

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line) {
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_motor_points.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static std::string SOf(cJSON* o, const char* k) {
    cJSON* v = o ? cJSON_GetObjectItem(o, k) : 0;
    return (v && v->valuestring) ? std::string(v->valuestring) : std::string();
}
static cJSON* Get(cJSON* o, const char* k) { return o ? cJSON_GetObjectItem(o, k) : 0; }
static bool IsNull(cJSON* v) { return v && v->type == cJSON_NULL; }
static cJSON* FindMotor(cJSON* doc, const char* id) {
    cJSON* ms = Get(doc, "motors");
    for (int i = 0; ms && i < cJSON_GetArraySize(ms); ++i) {
        cJSON* m = cJSON_GetArrayItem(ms, i);
        if (SOf(m, "motorId") == id) return m;
    }
    return 0;
}
static void Check9050HomeLed();   // AI(W906-MP9050-PIN) 20261003 (NB2-1, nb2-assist R184): [6] at EOF -- the 9050GPIB HOME LED had no pin; run before both "checks passed" lines (both builds)
int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: test_motor_points <Mot_Table.csv>\n"); return 2; }
    const std::string env = std::string("W906_MOTTABLE_PATH=") + argv[1];
    HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));
    HSys.LoadMotData();
    std::printf("Mot_Table rows loaded: %d\n", (int)HSys.MotTable.size());
    CHECK(HSys.MotTable.size() == 48);                 // HT9050：48 列、每列都有 Alias（Python 另算）

    // 保證起點：沒有任何驅動物件
    for (int i = 0; i < MAX_TRAY_MOTOR; ++i) MOT[i].Motor = NULL;

    // ---------------------------------------------------------------- [1]
    std::printf("[1] MotorConfigJson\n");
    cJSON* cfg = cJSON_Parse(MotorConfigJson().c_str());
    CHECK(cfg != 0);
    int nCfg = 0;
    if (cfg) {
        cJSON* ms = Get(cfg, "motors");
        nCfg = ms ? cJSON_GetArraySize(ms) : 0;
        std::printf("config motors: %d\n", nCfg);
        CHECK(nCfg == 48);
        std::set<std::string> ids;
        for (int i = 0; i < nCfg; ++i) ids.insert(SOf(cJSON_GetArrayItem(ms, i), "motorId"));
        CHECK((int)ids.size() == nCfg);                // motorId 唯一
        cJSON* byId = Get(Get(cfg, "index"), "byId");
        CHECK(byId && cJSON_GetArraySize(byId) == nCfg);

        cJSON* x = FindMotor(cfg, "MInArmX");          // CSV：M00,MInArmX,-999999,999999,…,HomeHighSpeed 200000,HomeLowSpeed 50000（AI(W906-MACH1002) 20261002：機台 1002 19:29 正本；0928 版是 200／50）
        CHECK(x != 0);
        if (x) {
            CHECK(Get(x, "motorIndex") && Get(x, "motorIndex")->valueint == 0);
            CHECK(SOf(x, "no") == "M00");
            cJSON* p = Get(x, "params");
            CHECK(Get(p, "homeHighSpeed") && Get(p, "homeHighSpeed")->valueint == 200000);   //AI(W906-MACH1002) 20261002 (NB2): 200 -> 200000, machines/HT9050/Mot_Table.csv = the machine's table of 1002 19:29 (M00 HomeHighSpeed; a Motor Test write-back on 10/1 21:47 raised it 20000 -> 200000 -- the machine's Mot_Table.csv.bak_* files)
            CHECK(Get(p, "homeLowSpeed") && Get(p, "homeLowSpeed")->valueint == 50000);     //AI(W906-MACH1002) 20261002 (NB2): 50 -> 50000, same row
            CHECK(Get(Get(x, "limits"), "softN")->valueint == -999999);
            CHECK(Get(Get(x, "limits"), "softP")->valueint == 999999);
        }
        //AI(W906-MT-E1) 20260925: golden Motor Test bView (forms/fMotorTest.cpp W906_MotorTestVisibility) per row.
        int nBool = 0, nRule = 0;   //AI(W906-MT-ENABLE) 20260929: motorTestVisible = Mot_Table Enable=1 (EastSun ruling); golden bView moved to goldenMotorTestVisible
        for (int i = 0; i < nCfg; ++i) {
            cJSON* row = cJSON_GetArrayItem(ms, i);
            cJSON* v = Get(row, "motorTestVisible");
            cJSON* g = Get(row, "goldenMotorTestVisible");
            if (v && (v->type == cJSON_True || v->type == cJSON_False) && g && (g->type == cJSON_True || g->type == cJSON_False)) ++nBool;
            cJSON* en = Get(row, "enabledByDefault");
            if (v && en && (v->type == cJSON_True) == (en->type == cJSON_True)) ++nRule;
        }
        CHECK(nBool == nCfg);                          // 每一列都有兩個，而且是布林
        CHECK(nRule == nCfg);                          // 每一列：顯示 == Mot_Table Enable=1
        CHECK(x && Get(x, "goldenMotorTestVisible")->type == cJSON_True);  // MInArmX：golden 無條件 new
        cJSON* sh1 = FindMotor(cfg, "MOutShuttle1");   // golden :373 整行註解掉 → golden 不顯示
        if (sh1) CHECK(Get(sh1, "goldenMotorTestVisible")->type == cJSON_False);
        cJSON* l2y = FindMotor(cfg, "MLoad2Y");        // golden :560 以 bView=false 建立（RogerYang 20250828）
        if (l2y) CHECK(Get(l2y, "goldenMotorTestVisible")->type == cJSON_False);
        CHECK(SOf(Get(cfg, "source"), "motTable") == argv[1]);
        cJSON_Delete(cfg);
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] MotorRuntimeJson, no driver objects\n");
    cJSON* rt = cJSON_Parse(MotorRuntimeJson().c_str());
    CHECK(rt != 0);
    if (rt) {
        cJSON* ms = Get(rt, "motors");
        const int n = ms ? cJSON_GetArraySize(ms) : 0;
        CHECK(n == nCfg);
        int nNoSrc = 0, nNullPos = 0;
        for (int i = 0; i < n; ++i) {
            cJSON* m = cJSON_GetArrayItem(ms, i);
            if (SOf(Get(m, "state"), "quality") == "nosource") ++nNoSrc;
            if (IsNull(Get(Get(m, "position"), "cmdPos")) && IsNull(Get(Get(m, "state"), "homeDone"))) ++nNullPos;
        }
        CHECK(nNoSrc == n);
        CHECK(nNullPos == n);                          // null，不是 0 —— 0 是合法座標
        //AI(W906-MT-E2) 20260925: no golden object -> "cur" is null (not a block of zeros); no page-state hook -> the timing is null
        int nNullCur = 0, nNullTime = 0;
        for (int i = 0; i < n; ++i) {
            cJSON* m = cJSON_GetArrayItem(ms, i);
            if (IsNull(Get(m, "cur"))) ++nNullCur;
            cJSON* mo = Get(m, "motion");
            if (IsNull(Get(mo, "jogPTime")) && IsNull(Get(mo, "jogNTime")) && IsNull(Get(mo, "avgTime"))) ++nNullTime;
        }
        CHECK(nNullCur == n);
        CHECK(nNullTime == n);
        CHECK(IsNull(Get(Get(rt, "runtime"), "selectedMotor")));
        cJSON* r = Get(rt, "runtime");
        CHECK(Get(r, "connected") && Get(r, "connected")->type == cJSON_False);
        CHECK(!SOf(r, "why").empty());
        CHECK(Get(Get(r, "counts"), "nosource")->valueint == n);
        cJSON_Delete(rt);
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] MotorRuntimeJson, MOT[0] live\n");
    TMySimMotor* sim = new TMySimMotor();
    MOT[0].Motor = sim;
    MOT[0].Position = 12345;
    MOT[0].EncoderPosition = 12340;
    MOT[0].TargetPosition = 20000;
    MOT[0].HomeFlag = 1;
    //AI(W906-MT-E2) 20260925: [3a] "cur" = what golden Motor Test shows for the motor, read from MOT[0].Motor (golden
    //  UpdateMotorParameter uMotorTest.cpp:655-669 + ReadRate/GetSpeed/LastHomePos) -- in BOTH builds: it is the golden
    //  object's memory, not a card value, so the shipping build's P5 "no MOT[] positions" rule does not apply to it.
    //  And [3b] the page-state hook: runtime.selectedMotor + motion.jogPTime/jogNTime/avgTime on every row (one set of labels).
    //  (Done before the [3] body below: each MotorRuntimeJson() bumps seq, and [4] counts from [3]'s.)
    sim->InitSpeed = 500; sim->PJogHighSpeed = 10000; sim->PJogLowSpeed = 100; sim->PHomeHighSpeed = 200; sim->PHomeLowSpeed = 50;
    sim->PSoftLimitP = 900000; sim->PSoftLimitN = -5000; sim->SetAccDataBase(3000.5); sim->SetDecDataBase(3100);
    sim->SetRange(20); sim->SetRate(90); sim->LastHomePos = -123; sim->GearRatio = 2.5; sim->Enable = false;
    MOT[0].SetSpeed(37);                               // golden TMyMotor::SetSpeed with Enable=false: speed=p -> GetSpeed()=37
    {
        struct PageFake {
            static bool Fn(MotorTestPageState& o)
            {
                o.selectedMotor = "MInArmX"; o.hasJogPTime = true; o.jogPTime = 1234; o.hasJogNTime = false;
                o.hasAvgTime = true; o.avgTime = 800.5; return true;
            }
        };
        SetMotorTestPageState(&PageFake::Fn);
        cJSON* rc = cJSON_Parse(MotorRuntimeJson().c_str());
        SetMotorTestPageState(0);
        CHECK(rc != 0);
        cJSON* x = FindMotor(rc, "MInArmX");
        cJSON* cur = Get(x, "cur");
        CHECK(cur && cur->type == cJSON_Object);
        if (cur && cur->type == cJSON_Object) {
            CHECK(Get(cur, "enable")->type == cJSON_False);
            CHECK(Get(cur, "initSpeed")->valueint == 500 && Get(cur, "jogHigh")->valueint == 10000 && Get(cur, "jogLow")->valueint == 100);
            CHECK(Get(cur, "homeHigh")->valueint == 200 && Get(cur, "homeLow")->valueint == 50);
            CHECK(Get(cur, "softP")->valueint == 900000 && Get(cur, "softN")->valueint == -5000);
            CHECK(Get(cur, "acc")->valuedouble == 3000.5 && Get(cur, "dec")->valuedouble == 3100.0);   // ReadAcc/ReadDec = the database values
            CHECK(Get(cur, "range")->valueint == 20 && Get(cur, "rate")->valueint == 90);
            CHECK(Get(cur, "readSpeed")->valueint == 37);                                       // MOT.GetSpeed() (lblRealSpeed)
            CHECK(Get(cur, "lastHomePos")->valueint == -123 && Get(cur, "gearRatio")->valuedouble == 2.5);
            CHECK(Get(cur, "homeFlag")->valueint == 1);
        }
        cJSON* y = FindMotor(rc, "MInArmY");           // MOT[1].Motor is NULL
        CHECK(y && IsNull(Get(y, "cur")));
        CHECK(SOf(Get(rc, "runtime"), "selectedMotor") == "MInArmX");
        cJSON* xm = Get(x, "motion");
        cJSON* ym = Get(y, "motion");
        CHECK(Get(xm, "jogPTime") && Get(xm, "jogPTime")->valueint == 1234 && IsNull(Get(xm, "jogNTime")) &&
              Get(xm, "avgTime") && Get(xm, "avgTime")->valuedouble == 800.5);
        CHECK(Get(ym, "jogPTime") && Get(ym, "jogPTime")->valueint == 1234);   // golden: one set of labels -> every row the same
        //AI(W906-MT-E3c) 20260925: the page-state hook did not fill the three MT-E3c blocks -> null (not a block of defaults)
        CHECK(IsNull(Get(rc, "motorPower")) && IsNull(Get(rc, "lock")) && IsNull(Get(rc, "lightScale")));
        cJSON* tq = Get(xm, "torque");                                           // no overlay -> every torque value null, src "none"
        CHECK(tq && tq->type == cJSON_Object && IsNull(Get(tq, "pct")) && IsNull(Get(tq, "nm")) && IsNull(Get(tq, "raw")) &&
              SOf(tq, "src") == "none" && Get(tq, "unitVerified")->type == cJSON_False);
        cJSON_Delete(rc);
    }
    //AI(W906-MT-E3c) 20260925: [3c] the contract's additions -- per-motor motion.torque from the overlay (EastSun R6: actual
    //  torque next to Real Speed; null unless the monitor says the value is valid THIS poll) and the three top-level blocks
    //  motorPower / lock / lightScale from the page-state hook. Before [3]'s body so it runs in both builds.
    {
        struct OvFake {
            static bool Fn(const std::string& alias, MotorRuntimeOverlay& o)
            {
                if (alias == "MInArmX") { o.posKnown = true; o.cmdPos = 1; o.encPos = 1; o.state = 1;
                                          o.torqueRawValid = true; o.torqueRaw = -123; o.torqueSrc = 2; o.torquePctValid = true; o.torquePct = -12.3;
                                          o.torqueNmValid = true; o.torqueNm = -0.5; o.torqueUnitVerified = false; return true; }
                if (alias == "MInArmY") { o.posKnown = true; o.state = 1; o.torqueRawValid = true; o.torqueRaw = 7; o.torqueSrc = 1;
                                          o.torquePctValid = false; o.torqueNmValid = false; return true; }   // raw known, the ratio not
                return false;
            }
        };
        struct PgFake {
            static bool Fn(MotorTestPageState& o)
            {
                o.hasPower = true; o.relayOn = true; o.relayCardKnown = false; o.motorPowerState = true; o.powerPending = true;
                o.hasLock = true; o.locked = true; o.labLockVisible = true; o.lockText = "*Lock by [M00] MInArmX moveing"; o.unlockCount = 3;
                o.hasLightScale = true; o.lsActive = true; o.lsTask = 150; o.lsEditsEnabled = false; o.lsUseAxis = 0; o.lsAxisItem = 1; o.lsMoveType = 0;
                o.lsMemoCount = 1234; o.lsMemoTail.push_back("ArmPosition, 900, LightScalePos, 895 ,[ 5 ]");
                o.lsData[0].push_back("Old, 1, New, 2, [ -1 ]"); o.lsDataCounts[0] = 1; o.lsDataCounts[7] = 9;
                return true;
            }
        };
        SetMotorRuntimeOverlay(&OvFake::Fn);
        SetMotorTestPageState(&PgFake::Fn);
        cJSON* rc = cJSON_Parse(MotorRuntimeJson().c_str());
        SetMotorRuntimeOverlay(0);
        SetMotorTestPageState(0);
        CHECK(rc != 0);
        cJSON* tq = Get(Get(FindMotor(rc, "MInArmX"), "motion"), "torque");
        { char* s = tq ? cJSON_PrintUnformatted(tq) : 0; std::printf("MInArmX motion.torque = %s\n", s ? s : "(none)"); if (s) cJSON_free(s); }
        // (this cJSON's parse_number is n*10^scale, not correctly rounded: compare -12.3 with a tolerance)
        CHECK(tq && std::fabs(Get(tq, "pct")->valuedouble + 12.3) < 1e-9 && Get(tq, "nm")->valuedouble == -0.5 && Get(tq, "raw")->valueint == -123 &&
              SOf(tq, "src") == "sdo" && Get(tq, "unitVerified")->type == cJSON_False);
        cJSON* ty = Get(Get(FindMotor(rc, "MInArmY"), "motion"), "torque");
        CHECK(ty && IsNull(Get(ty, "pct")) && IsNull(Get(ty, "nm")) && Get(ty, "raw")->valueint == 7 && SOf(ty, "src") == "pdo");   // null never 0
        cJSON* mp = Get(rc, "motorPower");
        CHECK(mp && Get(mp, "relayOn")->type == cJSON_True && IsNull(Get(mp, "relayCard")) && Get(mp, "motorPowerState")->type == cJSON_True &&
              Get(mp, "pending")->type == cJSON_True);
        cJSON* lk = Get(rc, "lock");
        CHECK(lk && Get(lk, "locked")->type == cJSON_True && SOf(lk, "text") == "*Lock by [M00] MInArmX moveing" && SOf(lk, "pnlStop") == "#FFFF00");
        cJSON* lsj = Get(rc, "lightScale");
        CHECK(lsj && Get(lsj, "active")->type == cJSON_True && Get(lsj, "task")->valueint == 150 && Get(lsj, "memoCount")->valueint == 1234 &&
              cJSON_GetArraySize(Get(lsj, "memoTail")) == 1 && std::string(cJSON_GetArrayItem(Get(lsj, "memoTail"), 0)->valuestring) ==
              "ArmPosition, 900, LightScalePos, 895 ,[ 5 ]" && cJSON_GetArraySize(Get(lsj, "data")) == 8 &&
              cJSON_GetArraySize(cJSON_GetArrayItem(Get(lsj, "data"), 0)) == 1 && cJSON_GetArraySize(Get(lsj, "dataCounts")) == 8 &&
              cJSON_GetArrayItem(Get(lsj, "dataCounts"), 7)->valueint == 9 && Get(lsj, "editsEnabled")->type == cJSON_False);
        cJSON_Delete(rc);
    }
    unsigned long long seq1 = 0;
    rt = cJSON_Parse(MotorRuntimeJson().c_str());
    CHECK(rt != 0);
#ifndef SOFT_SIMULTE
    //AI(W906-IOWEB-P5) 20260924: SHIPPING BUILD -- M00 is a PCI1203 row, so its
    //  source is the 1203 monitor, and there is none in this test. The MOT[0]
    //  values planted above MUST NOT appear: that was exactly the defect (every
    //  row "good" with numbers no card wrote). Assert the refusal and its reason.
    if (rt) {
        cJSON* x = FindMotor(rt, "MInArmX");
        CHECK(x != 0);
        if (x) {
            CHECK(IsNull(Get(Get(x, "position"), "cmdPos")));
            CHECK(IsNull(Get(Get(x, "position"), "encPos")));
            CHECK(SOf(Get(x, "state"), "quality") == "nosource");
            CHECK(SOf(Get(x, "diag"), "source") == "none");
            CHECK(!SOf(Get(x, "diag"), "why").empty());
            CHECK(!SOf(Get(x, "diag"), "errText").empty());
        }
        cJSON* r = Get(rt, "runtime");
        CHECK(Get(r, "connected")->type == cJSON_False);
        CHECK(Get(Get(r, "counts"), "good")->valueint == 0);
        CHECK(Get(Get(r, "counts"), "card")->valueint == 0);
        CHECK(SOf(r, "provider") == "wb_serve:pci1203-monitor");
        CHECK(!SOf(r, "why").empty());
        cJSON_Delete(rt);
    }
    MOT[0].Motor = NULL;
    delete sim;
    Check9050HomeLed();  std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);   // AI(W906-MP9050-PIN) 20261003: [6] (shipping build ends here)
    return g_fail ? 1 : 0;
#endif
    if (rt) {
        cJSON* x = FindMotor(rt, "MInArmX");
        CHECK(x != 0);
        if (x) {
            cJSON* pos = Get(x, "position");
            CHECK(Get(pos, "cmdPos")->valueint == 12345);
            CHECK(Get(pos, "encPos")->valueint == 12340);
            CHECK(Get(pos, "targetPos")->valueint == 20000);
            CHECK(Get(Get(x, "state"), "homeDone")->type == cJSON_True);
            CHECK(SOf(Get(x, "state"), "quality") == "good");
            CHECK(Get(Get(x, "diag"), "homeFlag")->valueint == 1);
            CHECK(IsNull(Get(Get(x, "motion"), "speed")));   // 沒快取的欄位：null，不打驅動
        }
        cJSON* r = Get(rt, "runtime");
        CHECK(Get(r, "connected")->type == cJSON_True);
        CHECK(Get(Get(r, "counts"), "good")->valueint == 1);
        CHECK(Get(Get(r, "counts"), "nosource")->valueint == nCfg - 1);
        seq1 = (unsigned long long)Get(r, "seq")->valuedouble;
        cJSON* y = FindMotor(rt, "MInArmY");                  // 別軸不受影響
        CHECK(y && SOf(Get(y, "state"), "quality") == "nosource");
        cJSON_Delete(rt);
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] HomeFlag==2\n");
    MOT[0].HomeFlag = 2;
    rt = cJSON_Parse(MotorRuntimeJson().c_str());
    CHECK(rt != 0);
    if (rt) {
        cJSON* x = FindMotor(rt, "MInArmX");
        CHECK(x && Get(Get(x, "state"), "homeDone")->type == cJSON_False);
        CHECK(x && !SOf(Get(x, "diag"), "errText").empty());
        CHECK((unsigned long long)Get(Get(rt, "runtime"), "seq")->valuedouble == seq1 + 1);   // 每次 +1
        cJSON_Delete(rt);
    }

    MOT[0].Motor = NULL;
    delete sim;
    // ---------------------------------------------------------------- [5]
    //  AI(W906-W4-MOTOR) 20260925: W4-c 覆蓋掛鉤 —— 1203 軸的值改由監看器供應（wb_serve 註冊；這裡用假的）。
    std::printf("[5] runtime overlay (1203 monitor samples)\n");
    struct Fake {
        static bool Fn(const std::string& alias, MotorRuntimeOverlay& o)
        {
            if (alias == "MInArmX") { o.posKnown = true; o.cmdPos = 1000; o.encPos = 998; o.servoOn = true;
                                      o.busy = false; o.alarm = false; o.inPos = true; o.state = 1;
                                      // AI(W906-MT-E1) 20260925: LMT- | ORG | SVON | SLMT_P | INP
                                      o.ioKnown = true; o.motionIO = 0x00000008ul | 0x00000010ul | 0x00004000ul | 0x00010000ul | 0x00002000ul;
                                      o.homeJob = true; return true; }
            if (alias == "MInArmY") { o.posKnown = false; o.why = "Direction=1 pending"; o.servoOn = false;
                                      o.busy = true; o.alarm = true; o.state = 3; o.driveErr = "Drive error"; return true; }   // ioKnown=false
            return false;
        }
    };
    SetMotorRuntimeOverlay(&Fake::Fn);
    rt = cJSON_Parse(MotorRuntimeJson().c_str());
    CHECK(rt != 0);
    if (rt) {
        cJSON* x = FindMotor(rt, "MInArmX");
        CHECK(x && Get(Get(x, "position"), "cmdPos")->valueint == 1000 && Get(Get(x, "position"), "encPos")->valueint == 998);
        CHECK(x && Get(Get(x, "state"), "servoOn")->type == cJSON_True && Get(Get(x, "motion"), "busy")->type == cJSON_False);
        CHECK(x && SOf(Get(x, "state"), "quality") == "good");
        CHECK(x && IsNull(Get(Get(x, "state"), "homeDone")));          // MOT[0].Motor 為 NULL ⇒ 歸零旗標沒有來源，仍是 null
        //AI(W906-MT-E1) 20260925: ALed1..10 decoded like golden TMyEtherCatMotor::ScanMotorStatus (myEthercatmotor.cpp:1166-1189)
        cJSON* led = x ? Get(Get(x, "state"), "led") : 0;
        CHECK(led && led->type == cJSON_Object);
        if (led && led->type == cJSON_Object) {
            CHECK(Get(led, "cw")->type == cJSON_True && Get(led, "home")->type == cJSON_True && Get(led, "ccw")->type == cJSON_False);
            CHECK(Get(led, "emg")->type == cJSON_False && Get(led, "alarm")->type == cJSON_False);
            CHECK(Get(led, "softCw")->type == cJSON_True && Get(led, "softCcw")->type == cJSON_False);
            CHECK(Get(led, "servoOn")->type == cJSON_True);
            CHECK(Get(led, "inPos")->type == cJSON_False && Get(led, "sAlarm")->type == cJSON_False);   // golden never lights them on a 1203 axis
        }
        CHECK(x && Get(Get(x, "motion"), "homeJob")->type == cJSON_True && Get(Get(x, "motion"), "loopJob")->type == cJSON_False &&
              IsNull(Get(Get(x, "motion"), "loopCount")));                // AI(W906-MT-E1): no loop on this motor -> count null
        cJSON* y = FindMotor(rt, "MInArmY");
        CHECK(y && IsNull(Get(Get(y, "state"), "led")));                  // no motionIO in the sample -> unknown, not "off"
        CHECK(y && IsNull(Get(Get(y, "position"), "cmdPos")) && SOf(Get(y, "state"), "quality") == "partial");
        CHECK(y && Get(Get(y, "state"), "alarm")->type == cJSON_True);
        CHECK(y && SOf(Get(y, "diag"), "errText").find("Direction=1") != std::string::npos &&
                   SOf(Get(y, "diag"), "errText").find("Drive error") != std::string::npos);
        CHECK(Get(Get(Get(rt, "runtime"), "counts"), "good")->valueint == 2);
        cJSON* z = FindMotor(rt, "MInArmPitch");                        // 掛鉤不認得 ⇒ 照舊
        CHECK(z && SOf(Get(z, "state"), "quality") == "nosource" && IsNull(Get(Get(z, "state"), "servoOn")));
        cJSON_Delete(rt);
    }
    SetMotorRuntimeOverlay(0);
    rt = cJSON_Parse(MotorRuntimeJson().c_str());
    if (rt) {
        CHECK(Get(Get(Get(rt, "runtime"), "counts"), "good")->valueint == 0);   // 取消掛鉤 ⇒ 回到原本行為
        cJSON_Delete(rt);
    }

    Check9050HomeLed();  std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);   // AI(W906-MP9050-PIN) 20261003: [6]
    return g_fail ? 1 : 0;
}

// ---------------------------------------------------------------- [6]
//  AI(W906-MP9050-PIN) 20261003 (NB2-1, nb2-assist R184; the main PC ACKed the claim in CHAT_JIMMY 1003 12:3x):
//  JsonBridge/ChanMotorPoints.cpp led.home on a 9050GPIB machine (:323-329) had no pin -- NB2-1's mutation_check.py
//  flipped a literal in it and MotorPoints_HT9050 stayed green, because nothing here ever set W906_GpibModel.
//  Rule pinned (HT9050-ORG-ST, MR !119 + the machine's W906_HT9050_ORG_INVERT, MachineType.h):
//    raw ORG = motionIO bit 0x10;  9050GPIB: home = (axis has a motor object && !bSensorType) ? raw : !raw;
//    then, when W906_HT9050_ORG_INVERT is defined, home = !home.   Any other model: home = raw (no SensorType, no invert).
//  A TMySimMotor on MOT[0] (MInArmX) gives the motor object; the runtime overlay hook supplies motionIO, as [5] does.
//  Everything changed here is restored before returning (W906_GpibModel, MOT[0].Motor, the overlay hook).
static unsigned long s_p6Io = 0;
static bool P6Overlay(const std::string& alias, MotorRuntimeOverlay& o)
{
    if (alias != "MInArmX") return false;
    o.posKnown = true; o.cmdPos = 0; o.encPos = 0; o.servoOn = true; o.state = 1;
    o.ioKnown = true; o.motionIO = s_p6Io;
    return true;
}
static int P6HomeLed()              // 1 / 0 = led.home true / false, -1 = no led object
{
    cJSON* rt = cJSON_Parse(MotorRuntimeJson().c_str());
    int r = -1;
    cJSON* led = Get(Get(FindMotor(rt, "MInArmX"), "state"), "led");
    cJSON* h = (led && led->type == cJSON_Object) ? Get(led, "home") : 0;
    if (h && h->type == cJSON_True) r = 1;
    else if (h && h->type == cJSON_False) r = 0;
    cJSON_Delete(rt);
    return r;
}
static void Check9050HomeLed()
{
#ifdef W906_HT9050_ORG_INVERT
    const bool inv = true;
#else
    const bool inv = false;
#endif
    std::printf("[6] 9050GPIB HOME LED: SensorType x ORG bit (W906_HT9050_ORG_INVERT %s)\n", inv ? "on" : "off");
    const AnsiString savedModel = W906_GpibModel;
    HTMotor* const savedMot = MOT[0].Motor;
    TMySimMotor* m = new TMySimMotor();
    MOT[0].Motor = m;
    SetMotorRuntimeOverlay(&P6Overlay);

    W906_GpibModel = "9050GPIB";
    for (int st = 0; st <= 1; ++st) {
        m->bSensorType = (st == 1);
        for (int org = 0; org <= 1; ++org) {
            s_p6Io = org ? 0x00000010ul : 0ul;
            const bool want = (st == 0 ? org == 1 : org == 0) != inv;
            const int got = P6HomeLed();
            if (got != (want ? 1 : 0))
                std::printf("  9050GPIB SensorType=%d ORG=%d: led.home %d, expected %d\n", st, org, got, want ? 1 : 0);
            CHECK(got == (want ? 1 : 0));
        }
    }
    MOT[0].Motor = NULL;                 // no motor object: the SensorType=1 rule (raw low = home), then the invert
    for (int org = 0; org <= 1; ++org) {
        s_p6Io = org ? 0x00000010ul : 0ul;
        CHECK(P6HomeLed() == (((org == 0) != inv) ? 1 : 0));
    }
    MOT[0].Motor = m;
    W906_GpibModel = "9045GPIB";         // control: not HT9050 -> the raw bit, SensorType and the invert ignored
    for (int st = 0; st <= 1; ++st) {
        m->bSensorType = (st == 1);
        for (int org = 0; org <= 1; ++org) {
            s_p6Io = org ? 0x00000010ul : 0ul;
            CHECK(P6HomeLed() == org);
        }
    }

    W906_GpibModel = savedModel;
    SetMotorRuntimeOverlay(0);
    MOT[0].Motor = savedMot;
    delete m;
}
