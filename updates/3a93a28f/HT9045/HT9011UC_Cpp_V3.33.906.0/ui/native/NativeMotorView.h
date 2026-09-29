// ===========================================================================
//  ui/native/NativeMotorView.h
//
//  AI(W906-NATIVE-PROTO) 20260928 [W906]: Main.MotorView 的原生 Win32 視窗 —— 唯讀原型（DEMO）。NOT in golden。
//
//  golden：主畫面 MotionView 分頁的馬達表 StringGrid1，TfMain::UpdateMotorScreen（V912 main.cpp:8703-8858）：
//    列 = fMotorTest->MotorTestClass 裡 Visible 的馬達（:8729-8738，順序照 MotorTestClass）；
//    欄 = Alias｜目前位置（ReadPos，磁性尺用 ReadEncoderPos）｜目標位置（TargetPosition）｜速度（GetSpeed）｜
//         Can｜L｜M｜R（fCanMove／fCanMoveL／fCanMoveM／fCanMoveR，:8834-8856）。
//  這裡照這八欄，右邊再加「補充」欄：網頁 /api/struct/motor/runtime 同源的伺服／警報／到位／忙碌／HomeFlag／
//  Motor Test 的十顆燈（golden ALed1..10，uMotorTest.cpp:626-642）與品質／來源／原因。
//
//  ⚠ 唯讀是構造上的：本檔（NativeMotorView.cpp）只 include <windows.h>／STL 與 NativeHost.h／NativeGrid.h，
//    **視窗上沒有任何按鈕**（沒有 jog／move／home／servo／power），也不連任何機台碼（ctest 只連本檔＋NativeHost.cpp 就連得起來）。
//    資料由 wb_serve 膠水（NativeFormsWbServe.cpp）或測試用 MotorViewUpdate() 灌進來。
//  ⚠ 執行緒：同 NativeIoView.h —— 全部在建視窗、泵訊息的那一條（wb_serve 主迴圈）。
// ===========================================================================
#ifndef W906_UI_NATIVE_NATIVEMOTORVIEW_H
#define W906_UI_NATIVE_NATIVEMOTORVIEW_H

#include <string>
#include <vector>

namespace w906native {

struct MotorRow {
    int           row;          // HSys.MotTable 的 index
    std::string   alias;
    std::string   no;           // 馬達表 No（"M07" -> MOT[7]，cinitial.cpp 的對法）
    int           motIndex;     // -1＝No 不是 M+數字
    std::string   cardModel;
    int           enable;       // 馬達表 Enable
    int           boardId;      // 1203：站號（ESC 0x0010）
    int           port;         // 1203：站內第幾軸
    bool          mtVisible;    // golden MotorView 會列（Motor Test 可見，W906_MotorTestVisibility）
    int           mtOrder;      // 在 golden MotorTestClass 的順序；-1＝不在清單
    // ---- golden StringGrid1 八欄 ----
    bool          hasCur;    int cur;
    bool          hasTarget; int target;
    bool          hasSpeed;  int speed;
    int           can, canL, canM, canR;        // -1＝null
    // ---- 補充（網頁 Motor-runtime 同源）----
    int           servoOn, alarm, inPos, busy;  // -1＝null
    int           homeFlag;                     // 0 未歸零／1 完成／2 失敗；-1＝null
    bool          ledKnown;                     // motionIO 是這次監看器樣本讀到的
    unsigned long motionIO;                     // Acm_AxGetMotionIO 原值
    unsigned      state;                        // Acm_AxGetState 原值
    std::string   quality;                      // "good" | "partial" | "nosource"
    std::string   source;                       // "pci1203-monitor" | "MOT[]" | "none"
    std::string   errText;                      // 原因／驅動器錯誤字串（UTF-8）
    MotorRow()
        : row(-1), motIndex(-1), enable(0), boardId(-1), port(-1), mtVisible(false), mtOrder(-1),
          hasCur(false), cur(0), hasTarget(false), target(0), hasSpeed(false), speed(0),
          can(-1), canL(-1), canM(-1), canR(-1), servoOn(-1), alarm(-1), inPos(-1), busy(-1), homeFlag(-1),
          ledKnown(false), motionIO(0), state(0) {}
};

struct MotorSummary {
    std::string   buildConfig;     // "SIM 組態"／"SHIP 組態"
    std::string   provider;
    std::string   why;             // 一軸都沒有值時的原因
    std::string   tablePath;
    bool          monitorOpen;
    int           monitorAxes;     // 監看器開著的軸數
    unsigned long pollCount;
    unsigned long keepaliveCalls;
    MotorSummary() : monitorOpen(false), monitorAxes(0), pollCount(0), keepaliveCalls(0) {}
};

enum MotorFilter { kMotFilterGolden = 0, kMotFilterAll = 1, kMotFilter1203 = 2, kMotFilterAlarm = 3, kMotFilterNull = 4 };

// 欄位（MotorViewCellText 用）是表格（NativeGrid）的 column index；畫面上的左右順序另外排（十顆燈排在 R 後面，見 NativeMotorView.cpp）。
// kMColLeds＝「亮的燈」文字（例 "CW SVON"）；kMColLamp0 + MotorLed＝每顆燈一欄（表格畫圓點；文字 "1"／"0"／"—"）。
enum MotorColumn {
    kMColRow = 0, kMColAlias, kMColNo, kMColCur, kMColTarget, kMColSpeed, kMColCan, kMColL, kMColM, kMColR,
    kMColLeds, kMColServo, kMColAlarm, kMColInPos, kMColBusy, kMColHome, kMColCard, kMColStation,
    kMColQuality, kMColSource, kMColWhy, kMColLamp0, kMColCount = kMColLamp0 + 10
};

// Motor Test 十顆燈的解碼（golden TMyEtherCatMotor::ScanMotorStatus，Motor/myEthercatmotor.cpp:1166-1189；
// 與 JsonBridge/ChanMotorPoints.cpp 的 "led" 同一套位元）。S Alarm／InPos 兩顆 golden 對 1203 軸不設（恆 false）。
enum MotorLed { kLedCw = 0, kLedHome, kLedCcw, kLedEmg, kLedAlarm, kLedSoftCw, kLedSoftCcw, kLedSAlarm, kLedInPos, kLedServo, kLedCount };
bool MotorLedOn(const MotorRow& r, int led);

bool MotorViewOpen(bool show);
void MotorViewClose();
bool MotorViewIsOpen();
void MotorViewUpdate(const std::vector<MotorRow>& rows, const MotorSummary& sum);

// ---- 測試探針 ----
void*         MotorViewHwnd();
int           MotorViewListCount();
std::wstring  MotorViewCellText(int item, int column);
int           MotorViewButtonCount();          // 子視窗中的按鈕總數（必須是 0：沒有任何運動指令）
void          MotorViewSetFilter(int filter);  // MotorFilter
void          MotorViewSetSearch(const std::wstring& text);
std::wstring  MotorViewSummaryText();
void*         MotorViewGridHwnd();            // 表格控件（NativeGrid.h 的 GridGetStats 等）
double        MotorViewLastUpdateMs();        // 上一次 MotorViewUpdate 花的時間（比較＋畫格子＋送上螢幕＋摘要）
double        MotorViewMaxUpdateMs();

}  // namespace w906native

#endif  // W906_UI_NATIVE_NATIVEMOTORVIEW_H
