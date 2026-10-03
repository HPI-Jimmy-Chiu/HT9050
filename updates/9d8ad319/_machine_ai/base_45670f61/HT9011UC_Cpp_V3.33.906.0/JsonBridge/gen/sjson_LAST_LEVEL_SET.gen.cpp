// ===========================================================================
//  sjson_LAST_LEVEL_SET.gen.cpp -- GENERATED, DO NOT EDIT BY HAND.
//
//  AI(W906-JSONBRIDGE-S2) 20260923.  NOT in golden.
//
//  產生器：tools/gen_sjson.py
//  來源　：cprod.h:1149-1152（結構 LAST_LEVEL_SET，1 個頂層成員）
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

const FieldDesc kFields_LAST_LEVEL_SET[] = {
    { "AccessLevel", offsetof(LAST_LEVEL_SET, AccessLevel), kFieldInt, sizeof(((LAST_LEVEL_SET*)0)->AccessLevel[0]), (std::size_t)(sizeof(((LAST_LEVEL_SET*)0)->AccessLevel) / sizeof(((LAST_LEVEL_SET*)0)->AccessLevel[0])), (std::size_t)(0) },
};

// AI(W906-JSONBRIDGE-S6): 欄位 -> ini 鍵。寫方向要用；讀方向用不到。
// multi=true 表示 golden 有不只一條路徑讀這個欄位，產生器不替你挑。
const IniKey kIniKeys_LAST_LEVEL_SET[] = {
    { 0, 0, 0, 0, 0, false }   // 無 ini 對照（二進位 blob，或還沒掃到）
};

}  // namespace

// ⚠ 一定要 extern。C++ 裡命名空間範圍的 `const` 物件預設是**內部連結**，
//   少了 extern 這個定義在別的 TU（Bindings.cpp）看不到，會是
//   「undefined reference to kType_...」。20260923 踩過一次。
extern const TypeDesc kType_LAST_LEVEL_SET;
extern const TypeDesc kType_LAST_LEVEL_SET = {
    "LAST_LEVEL_SET",
    kFields_LAST_LEVEL_SET,
    sizeof(kFields_LAST_LEVEL_SET) / sizeof(kFields_LAST_LEVEL_SET[0]),
    sizeof(LAST_LEVEL_SET),
    kIniKeys_LAST_LEVEL_SET,
    sizeof(kIniKeys_LAST_LEVEL_SET) / sizeof(kIniKeys_LAST_LEVEL_SET[0])
};

}  // namespace sjson
}  // namespace ht9045
