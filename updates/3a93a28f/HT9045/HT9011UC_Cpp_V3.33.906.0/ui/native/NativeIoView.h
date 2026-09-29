// ===========================================================================
//  ui/native/NativeIoView.h
//
//  AI(W906-NATIVE-PROTO) 20260928 [W906]: HW.IoSetView 的原生 Win32 視窗 —— 唯讀原型（DEMO）。
//  NOT in golden（golden 是 BCB6 VCL 表單 iosetview.dfm／iosetview.cpp；這裡只做「1a 唯讀燈號」的
//  最小可看版本，版面不照 .dfm，改用一張表列出全部 IO 點）。
//
//  Steven 20260928：「IoSetView 先做只看燈號、不能按輸出的版本」「先表列全部的IO與相關的狀態」。
//  方案文件：D:\HT9045\.claude\skills\ht9045-cpp-generated-pages\references\native-forms-plan.md §7.2 P1-1a。
//
//  ⚠ 唯讀是「構造上」的：本模組（NativeIoView.cpp）只 include <windows.h>／STL 與 ui/native 的 NativeHost.h／NativeGrid.h，
//    不 include 任何機台標頭、不連任何機台函式庫。ctest test_native_forms 只連本檔＋NativeHost.cpp＋NativeGrid.cpp＋NativeMotorView.cpp 就連得起來 ——
//    這本身就證明本檔沒有一條呼叫路徑通到 IOBitOn／IOBitOff／motor.access／1203 指令。
//    資料由呼叫端（wb_serve 膠水 NativeFormsWbServe.cpp，或測試）用 IoViewUpdate() 灌進來。
//
//  ⚠ 執行緒：所有函式都必須在「同一條」執行緒呼叫 —— 建視窗的那條、也是 PumpThreadMessages() 的那條。
//    在 wb_serve 裡就是主迴圈執行緒（同 golden 單執行緒 UI；EtherCAT/Pci1203Control.h:800-803 的
//    1203 單執行緒規則）。本模組自己不開任何執行緒。
// ===========================================================================
#ifndef W906_UI_NATIVE_NATIVEIOVIEW_H
#define W906_UI_NATIVE_NATIVEIOVIEW_H

#include <string>
#include <vector>

namespace w906native {

enum IoDir { kIoDirUnknown = 0, kIoDirIn = 1, kIoDirOut = 2 };

// 一個 IO 點。欄位與 /api/struct/io/config＋/api/struct/io/runtime 同一套（JsonBridge/ChanIoPoints.cpp），
// 只是不經 JSON。字串一律 UTF-8。
struct IoRow {
    int         row;        // IO_Table.csv 的資料列序號（HSys.IOTable 的 index）
    std::string alias;
    std::string ioType;     // Sensor／Switch／Cylinder／Cylinder_On／Cylinder_Off／Sucker／Sucker_On／Sucker_Off
    int         dir;        // IoDir
    std::string ioCode;     // "I2.30"（1203 列）或 "I0101"（其他）；空＝沒有位址碼
    int         isaBase;    // 0 MotionNet／1 ISA／2 PCI1735U／3 PCI1203／4 PLC
    int         lane;       // -1＝空
    int         ip;         // 1203 列＝站號
    int         port;       // 1203 列＝站內通道
    int         bit;
    int         inType;
    int         enable;
    int         raw;        // -1＝沒有值；0／1＝卡片原始位元
    int         isOn;       // -1＝null（不是 off）；0／1＝邏輯狀態（依 InType 換算）
    std::string quality;    // "good" | "bad" | "nosource" | "disabled"
    std::string source;     // "pci1203.di" | "pci1203.do" | ""
};

// 視窗上方的摘要列。
struct IoSummary {
    bool          connected;       // 與 /api/struct/io/runtime 的 runtime.connected 同一個判準
    std::string   provider;        // 例 "wb_serve: HSys.IOTable + TPci1203Monitor"
    std::string   why;             // !connected 時的原因（UTF-8）
    std::string   tablePath;       // 載入的 IO 表路徑
    std::string   buildConfig;     // "SIM"／"SHIP"
    unsigned long pollCount;
    unsigned long pollErrors;
    int           skippedNoAlias;  // 沒有 Alias 的列數（網頁 BuildIoIds 也略過它們）
    unsigned long keepaliveCalls;  // 拖曳／改大小期間主迴圈被保活了幾次（NativeHost.h）
    IoSummary() : connected(false), pollCount(0), pollErrors(0), skippedNoAlias(0), keepaliveCalls(0) {}
};

// 篩選（下拉選單的順序）。
enum IoFilter { kFilterAll = 0, kFilterIn = 1, kFilterOut = 2, kFilterOn = 3, kFilterOff = 4, kFilterUnknown = 5 };

// 開視窗。show=false 給 ctest 用（建起來、不顯示）。已開著就回 true、不重建。
bool IoViewOpen(bool show);
// 關視窗（DestroyWindow）。沒開著是 no-op。
void IoViewClose();
bool IoViewIsOpen();
// 把最新一批資料灌進視窗：只比較「值變了」的列，只重畫跟畫面上不一樣的格子（NativeGrid.h）。視窗沒開著是 no-op。
// 20 ms 一次也可以（Steven 20260929）：點表沒變時不配置記憶體、不 Widen；成本在摘要第三行與 IoViewLastUpdateMs()。
void IoViewUpdate(const std::vector<IoRow>& rows, const IoSummary& sum);

// 訊息泵、拖曳保活、快速鍵在 ui/native/NativeHost.h（IO 與 MotorView 兩個視窗共用）。

// ---- 測試探針（ctest 用；正式功能不需要）----
void*        IoViewHwnd();
int          IoViewListCount();                       // 篩選後表格的列數
std::wstring IoViewCellText(int item, int column);    // 表格某格的文字
int          IoViewLedState(int item);                // 1 on／0 off／-1 null
int          IoViewEnabledButtonCount();              // 子視窗中「可按」的按鈕數（必須是 0）
int          IoViewButtonCount();                     // 子視窗中的按鈕總數
void         IoViewSetFilter(int filter);             // IoFilter
void         IoViewSetSearch(const std::wstring& text);
std::wstring IoViewSummaryText();
unsigned long IoViewUpdateCount();                    // IoViewUpdate 被呼叫幾次
unsigned long IoViewRedrawnItems();                   // 累計「值變了」而且看得到、被拿去比較的列數
void*         IoViewGridHwnd();                       // 表格控件（NativeGrid.h 的 GridGetStats 等）
double        IoViewLastUpdateMs();                   // 上一次 IoViewUpdate 花的時間（比較＋畫格子＋送上螢幕＋摘要）
double        IoViewMaxUpdateMs();

// 表格欄位（IoViewCellText 用）。
enum IoColumn {
    kColRow = 0, kColLed, kColDir, kColAlias, kColType, kColCode, kColLane, kColIp, kColPort, kColBit,
    kColInType, kColEnable, kColRaw, kColQuality, kColSource, kColOutBtn, kColCount
};

}  // namespace w906native

#endif  // W906_UI_NATIVE_NATIVEIOVIEW_H
