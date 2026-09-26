// ===========================================================================
//  JsonBridge/ChanIo.h -- IO 位元打包通道（S9 的前半）。
//
//  AI(W906-SJSON-S9) 20260923.  NOT in golden.
//  規格：.claude/skills/ht9045-json-bridge/SKILL.md 六（通訊縮減）、八 S9
//
//  ---------------------------------------------------------------------------
//  它取代什麼，以及**它今天還沒有取代**
//  ---------------------------------------------------------------------------
//  現況（20260923 由原始碼實測）：`WebBridgeTags.cpp:1978`／`:2022` 兩個迴圈
//  每 tick stage
//      pci1203.di0 … pci1203.di319     320 個
//      pci1203.do0 … pci1203.do191     192 個
//                                      ---- 512 個「一個 byte 一個 tag」
//  另外每個 port 還有 ring／addr／flat／station／chan 等屬性 tag（DI 5×320、
//  DO 3×192），那些**不在**本檔的縮減範圍內 —— 它們是接線圖不是量測值，
//  一開機之後就不再變動，patch 一次也不會再送。
//
//  本檔把那 512 個**值**壓成 2 個 tag：
//      io.di        base64(320 bytes) = 428 字元   資料平面（每個 port 一個 byte）
//      io.di.valid  base64( 40 bytes) =  56 字元   在位平面（每個 port 一個 bit）
//      io.do        base64(192 bytes) = 256 字元
//      io.do.valid  base64( 24 bytes) =  32 字元
//
//  ⚠⚠ **SKILL.md §六 與派工單寫的「DI 320 bit＝40 bytes」是錯的算術**，
//    這裡刻意不照抄（20260923 實測）。1203 監看器的一個「port」是
//    **一個位元組**不是一個位元：`Pci1203DiSample::byteData` 宣告成
//    `unsigned char`（EtherCAT/Pci1203Monitor.h:1099，DO 側 :1136），而舊
//    的 `pci1203.di<i>` tag 送的就是那個 0..255 的值。
//    ⇒ 320 個 port ＝ 320 bytes ＝ 2,560 bit，base64 428 字元。
//    規格那個「40 bytes／56 字元」剛好等於**在位平面**的大小（320 個 port
//    一人一 bit），不是資料平面。兩個數字長得像，差 8 倍。
//    ⇒ 照規格開 40 bytes 的緩衝去裝 320 個 port 的值，會是一次
//      280 bytes 的溢位，而且前 40 個 port 看起來還是對的。
//
//    ⓘ 派工單引的 `Pci1203Monitor.h:383` 也不是常數的位置：
//      `kPci1203MaxDiPorts = 320` / `kPci1203MaxDoPorts = 192` 在 **:404-405**，
//      tag 層的 `kPci1203TagDiPorts` / `kPci1203TagDoPorts` 在 **:1637-1638**。
//      :383 是同一段註解的開頭。本檔用的是 tag 層那一對（理由見 .cpp）。
//
//  ⚠⚠ **但 512 個舊 tag 沒有被刪掉，本波次刻意不刪。**
//    `D:\HT9045\web\js\pci1203\view.js` 現在就在讀 `pci1203.di${i}`／
//    `pci1203.do${i}`（:3736／:3764／:3794／:3979 實測），刪掉等於當場
//    弄壞那一頁。刪除是**瀏覽器端改完之後**的一個獨立 commit，而且必須
//    跟 web 那邊同一個 commit 走 —— 這正是 S7 檔頭記下的「兩套命名並存」
//    危險的同一種：一邊改了另一邊沒改，畫面不會報錯，只會安靜地空掉。
//
//  ⇒ 所以本檔今天的貢獻是「2 個 tag 已經在線上、可驗、可量」，
//    **不是**「512 變成 2」。真正的減量數字在報告裡分兩行寫。
//
//  ---------------------------------------------------------------------------
//  為什麼是 base64 而不是十六進位字串或整數陣列
//  ---------------------------------------------------------------------------
//  量出來的（320 bytes 的 DI 影像，20260923）：
//      320 個個別 tag         ~ 5,800 bytes 的 JSON（tag 名就佔了大半；
//                             算法：`"pci1203.diNNN":NNN,` 平均 18 bytes × 320）
//      "0,12,255,…" 逗號串    ~ 1,300 bytes（值的位數會變，長度因此會飄）
//      base64                      428 bytes ＋ tag 名一次
//  ⇒ 減量約 13 倍，**不是**規格暗示的 100 倍 —— 那個數字來自上面那條錯算術。
//  base64 是三者裡唯一「長度固定、不因值而變」的：一個全 0 的影像和一個
//  全 1 的影像佔一樣多位元組，所以 patch 的大小不會隨機台忙碌程度漂移。
//  TagValue 沒有 bytes 型別，字串是唯一裝得下二進位又不會被 JSON 轉義
//  炸開的載體（0x22／0x5C 在 base64 裡不會出現）。
//
//  ---------------------------------------------------------------------------
//  null-vs-0：本檔最容易出錯的地方
//  ---------------------------------------------------------------------------
//  一個 DI byte 有三種狀態，而**打包會把後兩種壓成同一個 0**：
//      (a) 沒有這個 port（監看器只採到 21 個，窗口是 320）
//      (b) 有這個 port 但這次讀失敗（sample.valid == false）
//      (c) 讀到了，值就是 0x00
//  `WebBridgeTags.cpp` 的逐 port tag 靠 `stageInt(..., ok, ...)` 把 (a)(b)
//  送成 null、(c) 送成 0，那條規則是這棵樹的承重牆（WebBridgeTags.h 開頭）。
//
//  ⇒ 所以本檔**一定要有第二個平面**：`io.di.valid`／`io.do.valid`，同樣是
//    位元打包，1 = 這個 port 的那個 byte 是真的讀到的。瀏覽器必須先看
//    valid 再看 data；valid=0 的 byte 要畫成 "---" 不是 0x00。
//    只送 data 不送 valid 就是把三態壓成二態 —— 那正是這棵樹付過代價的
//    那一類缺陷（"dark lamp means either that coil is off or nobody looked"，
//    Pci1203Monitor.h:1133 的 DO 註解）。
//
//  ⇒ 而整個影像「完全沒有來源」時，四個 tag 一律 **null**，不是 320 個 0x00
//    的 base64。null = 沒看；全 0 的 base64 = 看了、都是 0。
//    ⚠ 「沒有來源」的判準是 **`diCount()>0 || doCount()>0`**，不是
//      `Pci1203Monitor() != 0`。實測（20260923，wb_serve --dry --port 8099）：
//      沒有 HAVE_PCI1203 的 binary 開機印「card NOT opened: not linked」，
//      但監看器物件照樣被建構出來、指標非 0、一個 port 都沒採到。
//      schema 另外送一個 `monitorPresent`，把「物件在不在」與「有沒有值」
//      分開講。詳見 ChanIo.cpp StageIo() 的更正註解。
//
//  ---------------------------------------------------------------------------
//  位元序（寫死在契約裡，不要改）
//  ---------------------------------------------------------------------------
//      bit n of the image  ->  byte n/8, bit n%8  (LSB first)
//      byte i of the image ==  pci1203.di<i>      (同一個 port 編號)
//  也就是 base64 解出來的第 i 個 byte，就是舊的 `pci1203.di${i}` 的值。
//  這條對應是**可驗的**，probe 會逐 byte 比對兩種表示法。
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_CHANIO_H
#define HT9045_JSONBRIDGE_CHANIO_H

#include <cstddef>
#include <string>

#include "WebBridge/TagSnapshot.h"

namespace ht9045 {
namespace sjson {

// -------------------------------------------------------------------------
//  stage io.di / io.di.valid / io.do / io.do.valid / io.ver。回傳 stage 筆數。
//
//  io.ver 是單調遞增的整數，**只有在任一影像（含 valid 平面）真的變動時**
//  才 +1。靜止時它不動 ⇒ 不進 patch ⇒ 靜止時這條通道的 patch 是 0 bytes。
//  這是本通道自己的「靜止 0」量測點，與 S8 的 prod.ver 同一個作法。
//
//  UI 執行緒限定（與 PublishHandlerTags 同一個 tick）。
// -------------------------------------------------------------------------
std::size_t StageIo(webbridge::TagSnapshot& snap);

// -------------------------------------------------------------------------
//  GET /api/struct/io/schema
//
//  告訴瀏覽器：影像有幾個 byte、位元序、valid 平面的意義、以及**目前有沒有
//  資料源**（anyLive）。沒有 schema 的話 base64 是不可解讀的 —— 這是把
//  值壓縮之後必須付的代價，所以它不是可選項。
// -------------------------------------------------------------------------
std::string IoSchemaJson();

// -------------------------------------------------------------------------
//  純函式，給測試用：把 bytes 編成 base64。
//  自己寫而不引 library：WebBridge/ 刻意零第三方依賴，這 20 行不值得破例。
// -------------------------------------------------------------------------
std::string Base64Encode(const unsigned char* data, std::size_t len);

}  // namespace sjson
}  // namespace ht9045

// ===========================================================================
//  AI(W906-IO-POINTS) 20260924: 每一個 IO 點（Alias）的即時狀態 —— HW.IoSetView 用。
//  實作在 JsonBridge/ChanIoPoints.cpp。
//
//  使用者 20260924 定調：網頁的 IO-config.json／IO-runtime.json 結構是契約，
//  **來源換成 C++ 當場讀真實機台**（原本那兩份是 09-02 的過渡快照：643 點全 unknown，
//  而且是 HT9045 的表，isaBase 列舉還把 3／4 對調了）。
//
//      GET /api/struct/io/config   = C++ 實際載入的 IO 表（HSys.IOTable），IO-config.json 同一個形狀
//      GET /api/struct/io/runtime  = 每一點的 on/off，IO-runtime.json 同一個形狀
//
//  ⚠ 值從哪來（不是 MyLaneIO）：1203 監看器已經在讀的 DI byte 與 DO 回授 byte
//    （上面 StageIo 用的同一批 TPci1203Monitor 樣本）。理由：
//      * MyLaneIO 的後端到今天還是 TSimIOBackend（SetBackend 在測試以外 0 個呼叫者），
//        從 Sen[]／SW[] 讀只會讀到模擬值；
//      * golden 的 TMySwitch::Status() 回的是「命令快取」OutPortData 不是硬體，
//        而且 1203 且 Ring==0 時恆回 false（MyLaneIo.cpp IOOutBitStatus）；
//      * 監看器只讀不寫（它的 allowlist 沒有任何 Set）。這一條不會讓任何線圈通電。
//
//  ⚠ 位址對應（1203 列，ISABase==3）—— 由 golden 與實機量測兩邊交叉確認：
//      golden MyLaneIo.cpp:399  Acm_DaqDiGetBitEx(dev, Ring=Lane, SlaveIP=IP, DiChannel=Port, &v)
//      ⇒ IO 表的 Port 是**站內的通道號**，Bit 欄 golden 根本不用（HT9050 表啟用的 236 列都滿足 Port%8==Bit）。
//      監看器 Pci1203Monitor.cpp（W906-1203RING-1，20260922 實機）用
//      Acm_DaqDiGetByteEx(dev, ring, station, stationChan) 讀 byte，station 是站號旋鈕（十進位），
//      與 IO 表 IP 欄的值（1、2、16、17、18、32、80…179 = 0x01、0x02、0x10…0xB3）一致。
//      ⇒ (Lane, IP, Port) 對到 ring==Lane（flat 讀法時不比）、station==IP、stationChan==Port/8 的那個 byte，
//        取第 Port%8 個位元。
//
//  ⚠ 邏輯狀態依 InType 換算，與 golden 相同：TMySensor::GetStatus（mysensor.cpp）與
//    TMySwitch::Status（myswitch.cpp）都是 `if(Type) return v; else return !v;`。
//    raw 欄另外給卡片上的原始位元，現場對線用。
//
//  ⚠ null 不是 0：沒有資料來源（沒有卡、不是 1203 列、對不到站號、方向不明）一律 isOn=null、
//    quality="nosource"；卡片讀失敗 quality="bad"；表上 Enable=0 的列 quality="disabled"
//    （golden 綁定時把它關掉，InitialSwitch／InitialSensor）。
// ===========================================================================
#include <vector>

namespace ht9045 {
namespace sjson {

// 一個 byte 的樣本（監看器的 DI 或 DO 回授；測試用假樣本也走這個形狀）。
struct IoByteSample {
    bool          valid;
    int           ring;          // -1 = flat 讀法，不比對 ring
    int           station;       // 站號旋鈕（十進位）；-1 = 對應表沒有這一格
    int           stationChan;   // 站內第幾個 byte；-1 = 不明
    unsigned char byteData;
};

enum IoPointDir { kIoDirUnknown = 0, kIoDirIn = 1, kIoDirOut = 2 };

// IOType -> 方向。與 09-02 產生 IO-config.json 的規則相同，並經 HT9050 表驗證：
// 啟用的每一個站號不是全部輸入就是全部輸出（DI／DO 模組分開）。
//   in : Sensor, Cylinder_On, Cylinder_Off, Sucker      （到位／真空感測）
//   out: Switch, Cylinder, Sucker_On, Sucker_Off        （閥／開關）
IoPointDir IoDirectionOfType(const std::string& ioType);

struct IoPointState {
    int         raw;        // -1 = 沒有值；0／1 = 卡片上的原始位元
    int         isOn;       // -1 = null；0／1 = 邏輯狀態（依 InType 換算）
    const char* quality;    // "good" | "bad" | "nosource" | "disabled"
    const char* source;     // "pci1203.di" | "pci1203.do" | 0
};

IoPointState ResolveIoPoint(int isaBase, int lane, int ip, int port, int inType, int enable,
                            IoPointDir dir,
                            const std::vector<IoByteSample>& di,
                            const std::vector<IoByteSample>& dout);

// 畫面上 title 顯示的位址碼。1203 列："I2.30" = 輸入、站 2、通道 30；
// 其他（MotionNet）沿用舊產生器的 "I" + IP(2 位) + Port + Bit（例 I0101）。
std::string IoCodeOf(int isaBase, IoPointDir dir, int ip, int port, int bit);

std::string IoConfigJson();                       // GET /api/struct/io/config
std::string IoRuntimeJson();                      // GET /api/struct/io/runtime
// 可測版本：樣本與時間由呼叫端給。
std::string IoRuntimeJsonFrom(const std::vector<IoByteSample>& di,
                              const std::vector<IoByteSample>& dout,
                              bool connected, const std::string& nowIso);

// AI(W906-IOOBS) 20260925: 監看器本身的狀態，讓頁面分得出「卡沒開／監看器停用／輪詢停了」。
//   原本 connected 只看「有沒有 port」，監看器連續失敗自己停用、或 tick 卡在 modal 時，
//   runtime 照樣回 connected＝true、lastPollAt＝現在 ⇒ 值凍結但畫面看不出來（EastSun 20260925 機台端調查）。
struct IoMonitorStatus {
    bool          present;        // 有監看器物件
    bool          linked;         // 這顆 binary 有 HAVE_PCI1203
    bool          open;           // 監看器手上有 device handle
    bool          disabled;       // 監看器自己停用了（連續讀取失敗等）
    std::string   disabledReason;
    unsigned long pollCount;      // 單調遞增；兩次讀到同一個數字 = 這段時間沒有輪詢
    unsigned long pollErrors;
    unsigned long pollMs;         // 最後一次 Poll() 花的時間
};
// 帶監看器狀態的版本：st 為 0 時與上面完全相同。
std::string IoRuntimeJsonFrom(const std::vector<IoByteSample>& di,
                              const std::vector<IoByteSample>& dout,
                              bool connected, const std::string& nowIso,
                              const IoMonitorStatus* st);

}  // namespace sjson
}  // namespace ht9045

#endif  // HT9045_JSONBRIDGE_CHANIO_H
