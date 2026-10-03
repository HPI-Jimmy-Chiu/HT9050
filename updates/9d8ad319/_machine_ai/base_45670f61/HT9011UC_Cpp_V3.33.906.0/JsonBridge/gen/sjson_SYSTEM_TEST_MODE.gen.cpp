// ===========================================================================
//  sjson_SYSTEM_TEST_MODE.gen.cpp -- GENERATED, DO NOT EDIT BY HAND.
//
//  AI(W906-JSONBRIDGE-S2) 20260923.  NOT in golden.
//
//  產生器：tools/gen_sjson.py
//  來源　：cprod.h:2583-2589（結構 SYSTEM_TEST_MODE，5 個頂層成員）
//  規格　：.claude/skills/ht9045-json-bridge/SKILL.md 4.1
//
//  重跑：python tools\gen_sjson.py
//
//  ⚠ offset 與 elemSize 都是 offsetof/sizeof —— **編譯器**算的，不是產生器。
//    產生器只知道名字。
// ===========================================================================
#include "JsonBridge/FieldDesc.h"

#include "vclcompat/vcl_compat.h"
#include "cprod.h"

namespace ht9045 {
namespace sjson {

namespace {

const FieldDesc kFields_SYSTEM_TEST_MODE[] = {
    { "iTestConnection", offsetof(SYSTEM_TEST_MODE, iTestConnection), kFieldInt, sizeof(((SYSTEM_TEST_MODE*)0)->iTestConnection), (std::size_t)(0), (std::size_t)(0) },
    { "iTemperatureMode", offsetof(SYSTEM_TEST_MODE, iTemperatureMode), kFieldInt, sizeof(((SYSTEM_TEST_MODE*)0)->iTemperatureMode), (std::size_t)(0), (std::size_t)(0) },
    { "iDutOnOff", offsetof(SYSTEM_TEST_MODE, iDutOnOff), kFieldUnsupported, sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOff[0][0][0]), (std::size_t)(sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOff) / sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOff[0])), (std::size_t)(sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOff[0]) / sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOff[0][0])) },
    { "iRunMode", offsetof(SYSTEM_TEST_MODE, iRunMode), kFieldInt, sizeof(((SYSTEM_TEST_MODE*)0)->iRunMode), (std::size_t)(0), (std::size_t)(0) },
    { "iDutOnOffEE", offsetof(SYSTEM_TEST_MODE, iDutOnOffEE), kFieldUnsupported, sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOffEE[0][0][0]), (std::size_t)(sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOffEE) / sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOffEE[0])), (std::size_t)(sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOffEE[0]) / sizeof(((SYSTEM_TEST_MODE*)0)->iDutOnOffEE[0][0])) },
};

// AI(W906-JSONBRIDGE-S6): 欄位 -> ini 鍵。寫方向要用；讀方向用不到。
// multi=true 表示 golden 有不只一條路徑讀這個欄位，產生器不替你挑。
const IniKey kIniKeys_SYSTEM_TEST_MODE[] = {
    { "iTestConnection", "szDir", "TestMode", "Tester Connection", "ON_LINE", false },
    { "iTemperatureMode", "szDir", "TestMode", "Temperature Mode", "Tempture_Hot", false },
    { "iDutOnOff", "szDir", "DutOnOff", "<S2>", "LastSet.bUseTestSocket[1][0][j]", true },
    { "iRunMode", "szDir", "TestMode", "Running Mode", "REALLY", false },
    { "iDutOnOffEE", "szDir", "DutOnOffEE", "<S>", "LastSet.bUseTestSocketEE[1][0][j]", true },
};

}  // namespace

// ⚠ 一定要 extern。C++ 裡命名空間範圍的 `const` 物件預設是**內部連結**，
//   少了 extern 這個定義在別的 TU（Bindings.cpp）看不到，會是
//   「undefined reference to kType_...」。20260923 踩過一次。
extern const TypeDesc kType_SYSTEM_TEST_MODE;
extern const TypeDesc kType_SYSTEM_TEST_MODE = {
    "SYSTEM_TEST_MODE",
    kFields_SYSTEM_TEST_MODE,
    sizeof(kFields_SYSTEM_TEST_MODE) / sizeof(kFields_SYSTEM_TEST_MODE[0]),
    sizeof(SYSTEM_TEST_MODE),
    kIniKeys_SYSTEM_TEST_MODE,
    sizeof(kIniKeys_SYSTEM_TEST_MODE) / sizeof(kIniKeys_SYSTEM_TEST_MODE[0])
};

}  // namespace sjson
}  // namespace ht9045
