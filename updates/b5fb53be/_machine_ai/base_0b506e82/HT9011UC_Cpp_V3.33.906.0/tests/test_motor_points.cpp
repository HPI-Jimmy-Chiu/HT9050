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
#include "JsonBridge/ChanMotor.h"
#include "Public/cJSON.h"
#include "database.h"
#include "Motor/mymotor.h"
#include "Motor/mySimMotor.h"

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

        cJSON* x = FindMotor(cfg, "MInArmX");          // CSV：M00,MInArmX,-999999,999999,…,HomeHighSpeed 200,HomeLowSpeed 50
        CHECK(x != 0);
        if (x) {
            CHECK(Get(x, "motorIndex") && Get(x, "motorIndex")->valueint == 0);
            CHECK(SOf(x, "no") == "M00");
            cJSON* p = Get(x, "params");
            CHECK(Get(p, "homeHighSpeed") && Get(p, "homeHighSpeed")->valueint == 200);
            CHECK(Get(p, "homeLowSpeed") && Get(p, "homeLowSpeed")->valueint == 50);
            CHECK(Get(Get(x, "limits"), "softN")->valueint == -999999);
            CHECK(Get(Get(x, "limits"), "softP")->valueint == 999999);
        }
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
    unsigned long long seq1 = 0;
    rt = cJSON_Parse(MotorRuntimeJson().c_str());
    CHECK(rt != 0);
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
                                      o.busy = false; o.alarm = false; o.inPos = true; o.state = 1; return true; }
            if (alias == "MInArmY") { o.posKnown = false; o.why = "Direction=1 pending"; o.servoOn = false;
                                      o.busy = true; o.alarm = true; o.state = 3; o.driveErr = "Drive error"; return true; }
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
        cJSON* y = FindMotor(rt, "MInArmY");
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

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
