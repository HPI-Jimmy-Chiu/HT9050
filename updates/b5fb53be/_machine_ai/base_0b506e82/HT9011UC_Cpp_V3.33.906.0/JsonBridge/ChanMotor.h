// ===========================================================================
//  JsonBridge/ChanMotor.h -- motor.axes 定序陣列（S9 的後半）。
//
//  AI(W906-SJSON-S9) 20260923.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/SKILL.md 六（通訊縮減）、八 S9
//
//  ---------------------------------------------------------------------------
//  形狀
//  ---------------------------------------------------------------------------
//      tag  motor.axes   一個字串，內容是 **JSON 文字**：
//                          [[actPos,cmdVel,state], null, [.,.,.], …]
//                        長度固定 = kPci1203TagAxes（32），順序由 /schema 給。
//      tag  motor.ver    只在內容真的變了才 +1（靜止時不動 ⇒ 0 patch）
//      tag  motor.count  監看器真的開了幾軸（不是 32）
//
//  為什麼是「一個字串裡放 JSON」而不是 32×3 個 tag：
//    * TagValue 只有 null/bool/int/double/string 五種，沒有陣列
//      （WebBridge/TagValue.h:67-71）。要嘛 96 個 tag，要嘛一個字串。
//    * 96 個 tag 的 JSON 有 ~2.4 KB 是 tag 名字；一個字串省掉全部。
//    * 順序不進線上，只在 /schema 送一次 —— 這是 §六 對 motor 那一列的原話
//      （「順序由 /schema 給一次」）。
//
//  ⚠ **不要**把它做成「只送有變動的那幾軸」。定長陣列的價值就是收端可以
//    用索引直接對位；長度會變的話瀏覽器每次都要重建綁定，而那正是
//    `WebBridgeTags.cpp:1601` 那段（tag 形狀固定在 kPci1203TagAxes）
//    已經付過學費的事。
//
//  ---------------------------------------------------------------------------
//  ⚠⚠ 資料源的三種「沒有值」，不可以壓成 0
//  ---------------------------------------------------------------------------
//      (a) 沒有監看器物件          -> motor.axes 整個 null
//      (b) 有監看器、這一格沒開軸  -> 該列 null（不是 [0,0,0]）
//      (c) 開了軸但這次取樣無效    -> 該列 null
//  0.0 是合法的座標、0 是合法的 STA_AX_* 狀態碼（STA_AX_DISABLE==0），
//  所以 [0,0,0] 會讀成「這根軸在原點、速度零、已停用」—— 三句都是捏造的。
//
//  ---------------------------------------------------------------------------
//  ⚠ 這不是 handler 自己的馬達層
//  ---------------------------------------------------------------------------
//  資料源是 PCIE-1203 的**唯讀監看**（`EtherCAT/Pci1203Monitor.h`），
//  不是 `MOT[]`／`TMyMotor`。兩者在真機上應該一致，但今天 wb_serve 離線時
//  `MOT[]` 沒有任何人灌值，而 1203 監看至少在有卡的機器上是真的。
//  schema 的 `from` 欄把這件事講明，免得有人拿這個數字去對 Mot_Table.csv
//  的軸編號 —— 那是另一套編號（見 Pci1203Monitor.h 的 station/alias 註解）。
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_CHANMOTOR_H
#define HT9045_JSONBRIDGE_CHANMOTOR_H

#include <cstddef>
#include <string>

#include "WebBridge/TagSnapshot.h"

namespace ht9045 {
namespace sjson {

std::size_t StageMotor(webbridge::TagSnapshot& snap);

// GET /api/struct/motor/schema -- 欄位順序、軸順序、每軸的位址身分。
std::string MotorSchemaJson();

}  // namespace sjson
}  // namespace ht9045

// ===========================================================================
//  AI(W906-MOTOR-POINTS) 20260924: 每一軸（馬達表 Alias）的設定與即時狀態 —— HW.MotorTest／HW.teach 用。
//  實作在 JsonBridge/ChanMotorPoints.cpp。
//
//  週末任務 W1 普查（docs/CHANNEL_CENSUS_20260924.md）量到：`Motor-runtime.json` 馬達測試頁、教導頁在讀，
//  C++ 沒寫 ⇒ 兩頁顯示的是 09-02 的過渡快照（86 軸、全部 unknown、provider "TBD"）。比照 IO（ChanIoPoints）：
//  **JSON 結構是契約，來源換成 C++**（使用者 20260924 定調）。
//
//      GET /api/struct/motor/config   = C++ 實際載入的馬達表（HSys.MotTable），Motor-config.json 同形狀
//      GET /api/struct/motor/runtime  = 每一軸的位置／狀態，Motor-runtime.json 同形狀
//
//  ⚠ 值從哪來：**handler 自己的馬達物件 `MOT[]`**（golden fMotorTest 顯示的就是它）——
//    `MOT[i]` 對 `Motorname == "M%02d"(i)` 那一列（cinitial.cpp InitialMotorParameter 的對法）。
//    上面 motor.axes 是 1203 監看器的 32 格、另一套編號；兩者在真機上應一致，不混用。
//  ⚠ **只讀快取欄位，不呼叫驅動**：`Position`／`EncoderPosition`／`TargetPosition`／`HomeFlag`。
//    `GetMotorAlarm()`／`GetSpeed()` 可能直接打驅動卡，網頁每秒輪詢 48 軸不能讓 HTTP 回應去打卡
//    （單執行緒 tick）。沒有快取的欄位一律 null，不是 0／false。
//  ⚠ `MOT[i].Motor==NULL`（這一軸沒有建立驅動物件：表上沒有、`CardModel` 不認得、Enable=0…）⇒ quality="nosource"。
//  `HomeFlag`：0 = 未歸零、1 = 歸零完成、2 = 歸零失敗（Motor/mymotor.cpp:1073-1202）。
// ===========================================================================
namespace ht9045 {
namespace sjson {

std::string MotorConfigJson();                    // GET /api/struct/motor/config
std::string MotorRuntimeJson();                   // GET /api/struct/motor/runtime

}  // namespace sjson
}  // namespace ht9045

// ===========================================================================
//  AI(W906-W4-MOTOR) 20260925: W4-c —— 1203 軸的即時值改讀 EastSun 的監看器樣本（覆蓋掛鉤）。
//  為什麼：HT9050 的 1203 軸是 EastSun 的監看器開的，golden 馬達物件（TMyEtherCatMotor）的快取欄位
//  （MOT[i].Position／EncoderPosition）在機台上不會被更新 ⇒ 頁面看不到軸實際跑到哪。
//  掛鉤由 wb_serve（WebMotorAccessLive.cpp）註冊；沒註冊（ctest 等）時行為與之前逐欄相同。
//  單位：使用者單位（卡片脈波 × GearRatio，golden TMyEtherCatMotor::ReadPos）；Direction=1 的軸位置不給（§0 第 5 件）。
// ===========================================================================
namespace ht9045 {
namespace sjson {

struct MotorRuntimeOverlay {
    bool        posKnown;         // false = 位置不給（why 說原因）
    int         cmdPos, encPos;   // 使用者單位
    bool        servoOn, alarm, busy, inPos;
    unsigned    state;            // Acm_AxGetState 原值（STA_AX_*）
    std::string why;
    std::string driveErr;         // 監看器的 driveErrText（廠商字串，不在本樹改寫）
    MotorRuntimeOverlay() : posKnown(false), cmdPos(0), encPos(0), servoOn(false), alarm(false),
                            busy(false), inPos(false), state(0) {}
};
typedef bool (*MotorRuntimeOverlayFn)(const std::string& alias, MotorRuntimeOverlay& out);
void SetMotorRuntimeOverlay(MotorRuntimeOverlayFn fn);   // 0 = 取消

}  // namespace sjson
}  // namespace ht9045

#endif  // HT9045_JSONBRIDGE_CHANMOTOR_H
