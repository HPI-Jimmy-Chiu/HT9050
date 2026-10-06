#ifndef JSONBRIDGE_CARDTYPEDIAG_H
#define JSONBRIDGE_CARDTYPEDIAG_H
// ===========================================================================
//  JsonBridge/CardTypeDiag.h
//
//  AI(W906-CARDTYPE) 20261006 (Jerry, J-17 A)：「這個 IO_CARD_TYPE 綁得起來嗎」的單一出處。
//
//  起因（docs/handoff/FROM_JERRY.md 的 J-17）：
//    cinitial.cpp:3956 的 `HSys.LoadMotData()` 與 database.cpp:1231 的 `LoadIoData()`
//    都只在 `IO_CARD_TYPE` 是 2 / 3 / 4 時被呼叫。golden 在其他值會改走 BDE Paradox 表
//    （`DataModule1->MotorTable` / `SwitchTable` / `SensorTable` / `CylinderTable` / `SuckerTable`），
//    而那五段在移植樹是 `#if 0`（cinitial.cpp:656 / 1570 / 2808 / 4222 / 5151，
//    使用者 20260924 裁決，docs/WEEKEND_PLAN_20260925.md W3）。
//    ⇒ `IO_CARD_TYPE ∈ {0,1}` 時馬達／IO／氣缸／吸嘴全都沒有來源，**而且是靜默的**：
//      唯一的訊號是 BootSummary.cpp:51-54 的 `std::printf`，而 F5 的 cppdbg 不接
//      debuggee 的 stdout（tools/measure_tick.ps1:36-38）⇒ 沒有人看得到。1006 為此查了一個上午。
//
//  ⚠ 純診斷字串。本檔不改變任何繫結行為，也沒有任何呼叫端依它決定要不要做事。
//     支援的卡型 ⇒ `CardTypeUnsupportedWhy()` 回空字串 ⇒ 呼叫端的輸出與導入本檔前逐位元相同。
//
//  為什麼是 header-only inline 而不是一支 .cpp：
//    tests/CMakeLists.txt:420 的 `test_motor_points` 只編 `../JsonBridge/ChanMotorPoints.cpp`
//    （沒有 ChanIoPoints.cpp），:403 的 `test_io_points` 反之。跨 TU 的符號會打斷它們的連結，
//    而標頭不需要註冊進 CMake。
// ===========================================================================
#include <cstdio>
#include <string>

//AI(W906-CARDTYPE) 20261006: cmydef.h:3060-3062 的兩個全域。**GLOBAL SCOPE 是刻意的** ——
//  在 `namespace ht9045::sjson` 裡宣告會命名 `ht9045::sjson::IO_CARD_TYPE`，連結時才會炸
//  （同一個陷阱 ChanIoPoints.cpp:16 對函式記過）。
//  不 include cmydef.h：那支很大且有全域副作用，本目錄的慣例是就地宣告。
extern int IO_CARD_TYPE;
extern int MOTION_CARD_TYPE;

namespace ht9045 {
namespace sjson {

// 2 = NewIO_MN200（cmydef.h:100）／3 = PCI_P64C64（:105）／4 = PCI1203_IO（:5957）。
// 這三個值就是 cinitial.cpp:3956 與 database.cpp:1231 那兩個 `if` 的內容。
inline bool CardTypeBindsTables(int ioCardType)
{
    return ioCardType == 2 || ioCardType == 3 || ioCardType == 4;
}

// 支援 ⇒ 空字串。不支援 ⇒ 一句指名道姓的原因（值、少了哪一步、golden 走哪裡、去哪裡改）。
inline std::string CardTypeUnsupportedWhy()
{
    if (CardTypeBindsTables(IO_CARD_TYPE)) return std::string();

    char buf[768];
    std::snprintf(buf, sizeof buf,
        "IO_CARD_TYPE=%d 不在移植樹支援的 {2,3,4} 內（2=NewIO_MN200／3=PCI_P64C64／4=PCI1203_IO）"
        "⇒ cinitial.cpp:3956 的 LoadMotData() 與 database.cpp:1231 的 LoadIoData() 都沒有被呼叫，"
        "Mot_Table.csv／IO_Table.csv 一列都沒讀進來，馬達／IO／氣缸／吸嘴全都沒有來源。"
        "golden 在這個值會改走 BDE Paradox 表（MOTION_CARD_TYPE=%d ⇒ %s），"
        "那五段在移植樹是 #if 0（cinitial.cpp:656／1570／2808／4222／5151，使用者 20260924 裁決，"
        "docs/WEEKEND_PLAN_20260925.md W3）。"
        "⇒ 請改 system\\Gerneral.ini 的 [System] IO_CARD_TYPE，或見 docs/handoff/FROM_JERRY.md 的 J-17。",
        IO_CARD_TYPE, MOTION_CARD_TYPE,
        MOTION_CARD_TYPE == 0 ? "system\\motor.DB" : "system\\motor_SMC.DB");
    return std::string(buf);
}

}  // namespace sjson
}  // namespace ht9045

#endif  // JSONBRIDGE_CARDTYPEDIAG_H
