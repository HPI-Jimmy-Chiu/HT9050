// ===========================================================================
//  ui/native/NativeFormsWbServe.cpp
//
//  AI(W906-NATIVE-PROTO) 20260928 [W906]: wb_serve 與原生表單原型之間的膠水（只在 -DW906_NATIVE_FORMS=ON 時編進 wb_serve，
//  ui/native/NativeForms.cmake）。NOT in golden。
//
//  入口，由 tools/wb_serve.cpp 在主迴圈執行緒呼叫（同一行插入；OFF 時的空函式在 wb_serve.cpp 檔尾）：
//    W906_NativeFormsStart()     主迴圈開始前一次：開 HW.IoSetView 與 Main.MotorView 兩個原生視窗、登記重開快速鍵。
//    W906_NativeFormsPump(site)  site 0 = 主迴圈每一圈（≤50 ms 一圈）；site 1 = 三個阻塞等待框每一圈（W906_ModalWaitTick）。
//                                泵視窗訊息；IO 與馬達每 20 ms 灌一次最新狀態（20260929，Steven「得快到20ms一次」；
//                                視窗只重畫變了的格子，沒變就 0 格，ui/native/NativeGrid.h）。
//    W906_NativeFormsStop()      程式正常關閉時：關視窗、取消快速鍵。
//  拖曳／改大小保活（NativeHost.h）：site 0 的 keepalive＝wb_serve.cpp 檔尾的 W906_NativeKeepaliveMain（跑一拍主迴圈的
//  週期工作）＋本檔的畫面更新；site 1 的 keepalive＝W906_NativeKeepaliveModal（只跑 Index 煞車保護，同阻塞等待框每一圈）。
//
//  ⚠ IO 資料 = 網頁 HW.IoSetView 的同一份（GET /api/struct/io/runtime），只是不經 JSON：
//      * 點表：HSys.IOTable（database.h:357；/api/struct/io/config 同源，JsonBridge/ChanIoPoints.cpp:145-252）
//      * 值：TPci1203Monitor 的 DI byte 與 DO 回授 byte（EtherCAT/Pci1203Monitor.h:1107／:1148／:1543）——
//        取樣照抄 IoRuntimeJson（ChanIoPoints.cpp:359-406），每一點用同一支 ResolveIoPoint（:107-143）。
//  ⚠ 馬達資料 = 網頁 /api/struct/motor/runtime 的同一份（JsonBridge/ChanMotorPoints.cpp:212-420 MotorRuntimeJson），不經 JSON：
//      * 列：HSys.MotTable；MOT[i] 對 No=="M%02d"（MotorIndexOf，同檔 :52-60）
//      * 1203 軸的值：EastSun 的監看器覆蓋掛鉤（WebMotorAccessLive.cpp:1110 W906_MotorOverlay，主執行緒每拍抄一份、鎖內讀），
//        經 ChanMotorPoints.cpp:209 同一行加的 W906_NativeMotorOverlay() 取得**同一支**掛鉤
//      * MOT[] 的快取欄位：Position／TargetPosition／HomeFlag／fCanMove*／GetSpeed()（HTMotor::ReadSpeed 回快取 iSpeed，
//        Motor/HTMotor.cpp:60，不打卡）；SIM／SHIP 誰能當「值」的規則照 MotorRuntimeJson（P5：出貨組態不拿 MOT[] 位置當卡片值）
//      * golden 列哪幾軸：W906_MotorTestVisibility()（forms/fMotorTest.cpp:1041，golden uMotorTest.cpp:145-390 的 bView），
//        順序照它（golden UpdateMotorScreen 照 MotorTestClass 的順序，V912 main.cpp:8729-8738）
//  ⚠ 唯讀：本檔只呼叫純函式與讀取函式（監看器的 const 讀取、覆蓋掛鉤的鎖內讀、MOT[] 欄位）。
//    沒有 IOBitOn／IOBitOff／motor.access／任何運動或 1203 指令。
//  ⚠ 執行緒：全部在 wb_serve 主迴圈執行緒（建視窗、泵訊息、讀 HSys／MOT[]、讀監看器樣本 —— 監看器的 Poll 也在這條線）。
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600   // MinGW.org 6.3：見 NativeIoView.cpp 檔頭
#endif
#include "ui/native/NativeIoView.h"
#include "ui/native/NativeMotorView.h"
#include "ui/native/NativeMotorTest.h"   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.MotorTest v1 (display only)
#include "ui/native/NativeHome.h"        // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.home v1 (display only)
#include "ui/native/NativeTeach.h"       // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.teach v1 (a table, display only)
#include "ui/native/NativeHost.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "JsonBridge/ChanIo.h"            // ResolveIoPoint / IoDirectionOfType / IoCodeOf / IoByteSample
#include "JsonBridge/ChanMotor.h"         // MotorRuntimeOverlay / MotorRuntimeOverlayFn
#include "EtherCAT/Pci1203Monitor.h"      // TPci1203Monitor（唯讀樣本）
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"                  // SOFT_SIMULTE（先於下面的 #ifdef 決定）
#include "database.h"                     // HSys.IOTable / HSys.MotTable
#include "common.h"                       // IoTablePath / MotTablePath
#include "Motor/mymotor.h"                // MOT[MAX_TRAY_MOTOR]
#include "forms/fHome.h"                  // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): fHome->HomeClass / ListBox1 / fShow (HW.home)
#include "vclcompat/Controls.h"           // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): full TLabel / TEdit / TListBox (fHome.h forward-declares them)
#include "cmydef.h"                       // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): INDEX_MOTION_CARD, MTestY1 / Z1 / Z2 / Y2 (HW.teach)
#include "forms/fTeach.h"                 // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): fTeach->TechPara / TechTwoPara / ActiveMotorIndex (read only)

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif

std::vector<std::pair<int, bool> > W906_MotorTestVisibility();   // forms/fMotorTest.cpp:1041（global）
namespace ht9045 { namespace sjson { MotorRuntimeOverlayFn W906_NativeMotorOverlay(); } }   // JsonBridge/ChanMotorPoints.cpp:209（同一行加）
void W906_NativeKeepaliveMain();    // tools/wb_serve.cpp 檔尾（要用 wb_serve.cpp 檔內的 static：g_apiCacheDirty、WdMark、主迴圈的截止時間）
void W906_NativeKeepaliveModal();   // 同上
bool W906_NativeShuttleMoveOpen(bool show);   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1 (ui/native/NativeShuttleMoveGlue.cpp)
void W906_NativeShuttleMoveClose();
void W906_NativeShuttleMoveReopen();
void W906_NativeShuttleMoveRefresh(bool force);

namespace {

// AI(W906-NATIVE-PROTO) 20260929 [W906]: 200／500 → 20 ms（Steven：「IO與馬達的顯示必須是很有效率的, 得快到20ms一次」）。
//   畫面這一段做得到：點表／馬達表沒變時，每一拍只重算「值」（沒有 AnsiString→std::string、沒有配置），
//   視窗只重畫跟畫面上不一樣的格子（NativeGrid.h）。
//   ⚠ 值本身多快變，不是這裡決定的（都在 tools/wb_serve.cpp 等處，本檔不改）：
//     * 1203 監看器 Poll 仍是每 200 ms（kIoTickMs，wb_serve.cpp:2940）⇒ IO 燈號的值 200 ms 才會變一次；
//     * 覆蓋掛鉤（1203 軸的位置／燈）的抄本仍是 500 ms 拍子（W906_MotorAccessTick → OverlaySnapshot）；
//     * 主迴圈一圈的等待上限 50 ms（wb_serve.cpp:4566），等的是命令佇列的事件、不是視窗訊息，而且沒有 timeBeginPeriod
//       ⇒ W906_NativeFormsPump 實際約每 50 ms 才輪到一次。要真的 20 ms，這三處要另外決定（摘要第三行顯示實際間隔）。
const DWORD kIoRefreshMs    = 20;
const DWORD kMotorRefreshMs = 20;
const int   kHotkeyIo = 0x4E49, kHotkeyMotor = 0x4E4D;
bool  g_started = false;
DWORD g_lastIo = 0, g_lastMotor = 0;
// AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.MotorTest, same 20 ms and a reopen hotkey (Ctrl+Alt+T)
const DWORD kMotorTestRefreshMs = 20;
const int   kHotkeyMotorTest = 0x4E54;
const int   kHotkeyShuttleMove = 0x4E53;   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1 (ui/native/NativeShuttleMoveGlue.cpp): Ctrl+Alt+S
DWORD g_lastMotorTest = 0;
// AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.home, same 20 ms and a reopen hotkey (Ctrl+Alt+H)
const DWORD kHomeRefreshMs = 20;
const int   kHotkeyHome = 0x4E48;
DWORD g_lastHome = 0;
// AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.teach, same 20 ms and a reopen hotkey (Ctrl+Alt+K)
const DWORD kTeachRefreshMs = 20;
const int   kHotkeyTeach = 0x4E4B;
DWORD g_lastTeach = 0;

std::string Str(const AnsiString& s) { return std::string(s.c_str()); }

const char* BuildConfig()
{
#ifdef SOFT_SIMULTE
    return "SIM 組態";
#else
    return "SHIP 組態";
#endif
}

// ------------------------------------------------------------------ IO ----
// AI(W906-NATIVE-PROTO) 20260929 [W906]: 點表的靜態欄位（Alias、型別、位址…）只在點表換了時才建一次；
// 每一拍只重算 raw／isOn／quality／source —— 20 ms 一拍也不做 AnsiString→std::string、不配置記憶體。
std::vector<const TIODATA*>                 g_ioSrc;   // g_ioRows[k] 來自哪一列
std::vector<ht9045::sjson::IoPointDir>      g_ioDir;
std::vector<w906native::IoRow>              g_ioRows;
std::size_t                                 g_ioTableSize = (std::size_t)-1;
int                                         g_ioSkipped = 0;
std::vector<ht9045::sjson::IoByteSample>    g_ioDi, g_ioDo;   // 留著容量，每一拍不重配

void AssignIfDiffers(std::string& dst, const char* src)
{
    if (!src) src = "";
    if (std::strcmp(dst.c_str(), src) != 0) dst = src;   // 一樣就不指定（不配置）
}

bool IoTableSame()
{
    if (HSys.IOTable.size() != g_ioTableSize) return false;
    for (std::size_t k = 0; k < g_ioSrc.size(); ++k) {
        const TIODATA* r = g_ioSrc[k];
        const w906native::IoRow& row = g_ioRows[k];
        if ((std::size_t)row.row >= HSys.IOTable.size() || HSys.IOTable[(std::size_t)row.row] != r) return false;
        if (r->iISABase != row.isaBase || r->iLane != row.lane || r->iIP != row.ip || r->iPort != row.port ||
            r->iBit != row.bit || r->iInType != row.inType || r->iEnable != row.enable ||
            std::strcmp(r->Alias.c_str(), row.alias.c_str()) != 0 || std::strcmp(r->Type.c_str(), row.ioType.c_str()) != 0)
            return false;
    }
    return true;
}

void IoBuildStatic()
{
    using namespace ht9045::sjson;
    g_ioSrc.clear();
    g_ioDir.clear();
    g_ioRows.clear();
    g_ioSkipped = 0;
    g_ioTableSize = HSys.IOTable.size();
    g_ioRows.reserve(HSys.IOTable.size());
    for (std::size_t i = 0; i < HSys.IOTable.size(); ++i) {
        const TIODATA* r = HSys.IOTable[i];
        if (!r) continue;
        const std::string alias = Str(r->Alias);
        if (alias.empty()) { ++g_ioSkipped; continue; }   // 網頁 BuildIoIds（ChanIoPoints.cpp:53-74）也略過
        const std::string type = Str(r->Type);
        const IoPointDir dir = IoDirectionOfType(type);
        w906native::IoRow row;
        row.row = (int)i;
        row.alias = alias;
        row.ioType = type;
        row.dir = dir == kIoDirIn ? w906native::kIoDirIn : dir == kIoDirOut ? w906native::kIoDirOut : w906native::kIoDirUnknown;
        row.ioCode = IoCodeOf(r->iISABase, dir, r->iIP, r->iPort, r->iBit);
        row.isaBase = r->iISABase;
        row.lane = r->iLane;
        row.ip = r->iIP;
        row.port = r->iPort;
        row.bit = r->iBit;
        row.inType = r->iInType;
        row.enable = r->iEnable;
        row.raw = -1;
        row.isOn = -1;
        g_ioRows.push_back(row);
        g_ioSrc.push_back(r);
        g_ioDir.push_back(dir);
    }
}

void IoSnapshot(w906native::IoSummary& sum)
{
    using namespace ht9045::sjson;
    // ---- 監看器樣本：照抄 IoRuntimeJson（ChanIoPoints.cpp:359-386）----
    std::vector<IoByteSample>& di = g_ioDi;
    std::vector<IoByteSample>& dout = g_ioDo;
    di.clear();
    dout.clear();
    ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor();
    const int nDi = mon ? mon->diCount() : 0;
    const int nDo = mon ? mon->doCount() : 0;
    for (int i = 0; i < nDi; ++i) {
        const ht9045::Pci1203DiSample& s = mon->di(i);
        IoByteSample b;
        b.valid = s.valid;
        b.ring = s.flat ? -1 : s.ring;
        b.station = s.station;
        b.stationChan = s.stationChan;
        b.byteData = s.byteData;
        di.push_back(b);
    }
    for (int i = 0; i < nDo; ++i) {
        const ht9045::Pci1203DoSample& s = mon->do_(i);
        IoByteSample b;
        b.valid = s.valid;
        b.ring = s.ring;
        b.station = s.station;
        b.stationChan = s.stationChan;
        b.byteData = s.byteData;
        dout.push_back(b);
    }
    bool open = false, disabled = false, linked = false;
    std::string disabledReason;
    if (mon) {
        const ht9045::Pci1203CardSample& c = mon->card();
        linked = c.linked;
        open = c.open;
        disabled = mon->Disabled();
        disabledReason = mon->disabledReason();
        sum.pollCount = (unsigned long)c.pollCount;
        sum.pollErrors = (unsigned long)c.pollErrors;
    }
    // 與 IoRuntimeJson 同一個判準（ChanIoPoints.cpp:405）。
    sum.connected = (mon != 0) && open && !disabled && (nDi > 0 || nDo > 0);
    if (!sum.connected) {
        if (mon && disabled) sum.why = "1203 監看器已自己停用（" + disabledReason + "）";
        else if (mon && linked && !open) sum.why = "1203 監看器沒有開到卡（見開機主控台的 card NOT opened）";
        else sum.why = "1203 監看器沒有採到任何 port（沒有卡，或這顆 binary 沒有 HAVE_PCI1203）";
    }
    sum.provider = "wb_serve: HSys.IOTable + TPci1203Monitor (same as /api/struct/io/runtime)";
    sum.tablePath = Str(IoTablePath);
    sum.buildConfig = BuildConfig();
    sum.keepaliveCalls = w906native::HostKeepaliveCalls();

    // ---- 點表沒變就沿用靜態欄位；每一點的值：同一支 ResolveIoPoint ----
    if (!IoTableSame()) IoBuildStatic();
    sum.skippedNoAlias = g_ioSkipped;
    for (std::size_t k = 0; k < g_ioRows.size(); ++k) {
        const TIODATA* r = g_ioSrc[k];
        w906native::IoRow& row = g_ioRows[k];
        const IoPointState s = ResolveIoPoint(r->iISABase, r->iLane, r->iIP, r->iPort, r->iInType, r->iEnable,
                                              g_ioDir[k], di, dout);
        row.raw = s.raw;
        row.isOn = s.isOn;
        AssignIfDiffers(row.quality, s.quality);
        AssignIfDiffers(row.source, s.source);
    }
}

// --------------------------------------------------------------- Motor ----
// "M07" -> 7；不是 M+數字 -> -1（照抄 ChanMotorPoints.cpp:52-60 MotorIndexOf）
int MotorIndexOf(const AnsiString& no)
{
    const std::string s = Str(no);
    if (s.size() < 2 || (s[0] != 'M' && s[0] != 'm')) return -1;
    for (std::size_t i = 1; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9') return -1;
    return std::atoi(s.c_str() + 1);
}

// 照抄 ChanMotorPoints.cpp:189-207 FindCardAxis（那支在匿名 namespace 裡，只用來寫「為什麼沒有值」）。
const ht9045::Pci1203AxisSample* FindCardAxis(ht9045::TPci1203Monitor* mon, int station, int sub, const char** why)
{
    *why = "";
    const int n = mon ? mon->axisCount() : 0;
    if (n <= 0) { *why = "1203 監看器沒有開任何軸（沒有卡、卡沒開，或這顆 binary 沒有 HAVE_PCI1203）"; return 0; }
    if (station < 0 || sub < 0) { *why = "馬達表這一列沒有 BoardID/Port"; return 0; }
    bool ambiguous = false, invalid = false;
    for (int k = 0; k < n; ++k) {
        const ht9045::Pci1203AxisSample& a = mon->axis(k);
        if (!a.opened || a.station != station || a.stationAxis != sub) continue;
        if (a.stationAmbiguous) { ambiguous = true; continue; }
        if (!a.valid) { invalid = true; continue; }
        return &a;
    }
    if (ambiguous) *why = "卡上這個站號不唯一（stationAmbiguous）：照站號對應的軸名不可信，拒絕對應";
    else if (invalid) *why = "卡上有這一軸，但這次取樣無效";
    else *why = "卡上沒有開到 (station=BoardID, stationAxis=Port) 這一軸 —— 檢查 Mot_Table 的 BoardID/Port";
    return 0;
}

bool GoldenOrderLess(const w906native::MotorRow& a, const w906native::MotorRow& b)
{
    // golden 會列的在前、照 MotorTestClass 順序；其餘照馬達表列序
    if (a.mtVisible != b.mtVisible) return a.mtVisible;
    if (a.mtVisible) return a.mtOrder < b.mtOrder;
    return a.row < b.row;
}

// AI(W906-NATIVE-PROTO) 20260929 [W906]: 馬達表的靜態欄位只在表或 golden 可見清單換了時才建一次（含排序）；
// 每一拍只重算值（20 ms 一拍：不做 AnsiString→std::string、不重新排序）。
std::vector<w906native::MotorRow>     g_motRows;   // 已照 golden 順序排好；m.row＝HSys.MotTable 的 index
std::vector<std::pair<int, bool> >    g_motVis;
std::size_t                           g_motTableSize = (std::size_t)-1;

bool MotorTableSame(const std::vector<std::pair<int, bool> >& mtVis)
{
    if (HSys.MotTable.size() != g_motTableSize || mtVis != g_motVis) return false;
    for (std::size_t k = 0; k < g_motRows.size(); ++k) {
        const w906native::MotorRow& m = g_motRows[k];
        if ((std::size_t)m.row >= HSys.MotTable.size()) return false;
        const TMOTDATA* r = HSys.MotTable[(std::size_t)m.row];
        if (!r || r->iEnable != m.enable || r->iBoardID != m.boardId || r->iPort != m.port ||
            std::strcmp(r->Alias.c_str(), m.alias.c_str()) != 0 || std::strcmp(r->No.c_str(), m.no.c_str()) != 0 ||
            std::strcmp(r->CardModel.c_str(), m.cardModel.c_str()) != 0)
            return false;
    }
    return true;
}

void MotorBuildStatic(const std::vector<std::pair<int, bool> >& mtVis)
{
    g_motRows.clear();
    g_motVis = mtVis;
    g_motTableSize = HSys.MotTable.size();
    for (std::size_t i = 0; i < HSys.MotTable.size(); ++i) {
        const TMOTDATA* r = HSys.MotTable[i];
        if (!r || r->Alias.Length() == 0) continue;   // 同 MotorRuntimeJson
        w906native::MotorRow m;
        m.row = (int)i;
        m.alias = Str(r->Alias);
        m.no = Str(r->No);
        const int mi = MotorIndexOf(r->No);
        m.motIndex = mi;
        m.cardModel = Str(r->CardModel);
        m.enable = r->iEnable;
        m.boardId = r->iBoardID;
        m.port = r->iPort;
        for (std::size_t k = 0; k < mtVis.size(); ++k)
            if (mi >= 0 && mtVis[k].first == mi) { m.mtOrder = (int)k; m.mtVisible = mtVis[k].second; break; }
        g_motRows.push_back(m);
    }
    std::stable_sort(g_motRows.begin(), g_motRows.end(), &GoldenOrderLess);
}

void MotorSnapshot(w906native::MotorSummary& sum)
{
    using ht9045::sjson::MotorRuntimeOverlay;
    const std::vector<std::pair<int, bool> > mtVis = W906_MotorTestVisibility();
    ht9045::sjson::MotorRuntimeOverlayFn ovFn = ht9045::sjson::W906_NativeMotorOverlay();
    ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor();
    sum.buildConfig = BuildConfig();
    sum.provider = "wb_serve: HSys.MotTable + MOT[] + EastSun overlay (same as /api/struct/motor/runtime)";
    sum.tablePath = Str(MotTablePath);
    sum.monitorOpen = mon && mon->card().open;
    sum.monitorAxes = mon ? mon->axisCount() : 0;
    sum.pollCount = mon ? (unsigned long)mon->card().pollCount : 0;
    sum.keepaliveCalls = w906native::HostKeepaliveCalls();

    if (!MotorTableSame(mtVis)) MotorBuildStatic(mtVis);
    int nLive = 0;
    const w906native::MotorRow blank;
    for (std::size_t k = 0; k < g_motRows.size(); ++k) {
        w906native::MotorRow& m = g_motRows[k];
        const TMOTDATA* r = HSys.MotTable[(std::size_t)m.row];
        const int mi = m.motIndex;
        // 值欄位先回到預設（同 20260928 版每一拍新建一筆 MotorRow）
        m.hasCur = blank.hasCur; m.cur = blank.cur; m.hasTarget = blank.hasTarget; m.target = blank.target;
        m.hasSpeed = blank.hasSpeed; m.speed = blank.speed;
        m.can = blank.can; m.canL = blank.canL; m.canM = blank.canM; m.canR = blank.canR;
        m.servoOn = blank.servoOn; m.alarm = blank.alarm; m.inPos = blank.inPos; m.busy = blank.busy;
        m.homeFlag = blank.homeFlag; m.ledKnown = blank.ledKnown; m.motionIO = blank.motionIO; m.state = blank.state;

        // ---- 以下逐條照 MotorRuntimeJson（ChanMotorPoints.cpp:226-345）----
        const bool is1203 = (m.cardModel == "PCI1203");
        const char* why = "";
        const ht9045::Pci1203AxisSample* ax = 0;
        if (is1203) {
            if (r->iEnable != 1) why = "馬達表 Enable=0";
            else ax = FindCardAxis(mon, r->iBoardID, r->iPort, &why);
        }
        const bool motObj = (mi >= 0 && mi < MAX_TRAY_MOTOR && MOT[mi].Motor != 0);
#ifdef SOFT_SIMULTE
        const bool motLive = motObj;
#else
        const bool motLive = false;   // P5：出貨組態不拿 MOT[] 位置當卡片值
        if (!ax && !is1203) why = "出貨組態：這一列不是 PCI1203，沒有接任何驅動後端（MOT[] 的值不是卡片來的）";
#endif
        (void)ax;
        MotorRuntimeOverlay ov;
        const bool hasOv = (ovFn != 0) && ovFn(m.alias, ov);
        const bool live = motLive || hasOv;
        if (live) ++nLive;

        if (hasOv) { m.hasCur = ov.posKnown; m.cur = ov.cmdPos; }
        else if (motLive) { m.hasCur = true; m.cur = MOT[mi].Position; }
        if (motLive) { m.hasTarget = true; m.target = MOT[mi].TargetPosition; }
        // golden 的「速度」欄是 MOT.GetSpeed()（main.cpp:8822-8830）：HTMotor::ReadSpeed 回快取 iSpeed，不打卡。
        if (motObj) { m.hasSpeed = true; m.speed = MOT[mi].GetSpeed(); }
        if (motObj) {
            m.can  = MOT[mi].fCanMove ? 1 : 0;
            m.canL = MOT[mi].fCanMoveL ? 1 : 0;
            m.canM = MOT[mi].fCanMoveM ? 1 : 0;
            m.canR = MOT[mi].fCanMoveR ? 1 : 0;
            m.homeFlag = MOT[mi].HomeFlag;
        }
        if (hasOv) {
            m.servoOn = ov.servoOn ? 1 : 0;
            m.alarm = ov.alarm ? 1 : 0;
            m.inPos = ov.inPos ? 1 : 0;
            m.busy = ov.busy ? 1 : 0;
            m.state = ov.state;
            m.ledKnown = ov.ioKnown;
            m.motionIO = ov.motionIO;
        }
        AssignIfDiffers(m.quality, hasOv ? (ov.posKnown ? "good" : "partial") : (live ? "good" : "nosource"));
        AssignIfDiffers(m.source, hasOv ? "pci1203-monitor" : (motLive ? "MOT[]" : "none"));
        std::string err;
        if (hasOv && !ov.posKnown) err = ov.why;
        if (hasOv && ov.alarm && !ov.driveErr.empty()) err += (err.empty() ? "" : "; ") + ov.driveErr;
        if (motObj && MOT[mi].HomeFlag == 2) err += (err.empty() ? "" : "; ") + std::string("home failed (HomeFlag=2)");
        if (!live && err.empty()) err = why;
        if (m.errText != err) m.errText.swap(err);
    }
    if (nLive == 0) {
#ifdef SOFT_SIMULTE
        sum.why = "沒有任何一軸建立了驅動物件（MOT[i].Motor 全為 NULL）：馬達表沒載入，或 InitialMotorParameter 還沒跑。";
#else
        sum.why = "沒有任何 PCI1203 列對到 1203 監看器開著的軸（見每一列說明）；出貨組態不拿 MOT[] 當來源。";
#endif
    }
}

// ------------------------------------------------------- HW.MotorTest ----
// AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): the golden-visible motors of TfMotorTest (MotorTestClass order), v1 display
//   only.  Per motor: MotorSnapshot's row (the same values as Main.MotorView and /api/struct/motor/runtime), plus from the
//   SAME overlay hook the encoder position and the loop / home jobs (MotorRuntimeOverlay), plus engine memory with no card
//   access: strngrdMotor's ten parameters (golden UpdateMotorParameter :655-669; HTMotor::Read* return cached fields,
//   Motor/HTMotor.cpp:60-99) and LastHomePos (edtHomeOffset).  Nothing here writes: no SetSpeed on select (golden
//   lM00Click :756), no FormShow power-on, no Timer1 loop / home (St02-E2 inventory §0.1).
//   The page-wide state (Jog / Avg times, Motor Power, Lock, ActiveIndex) is MotorTestPageState behind
//   ChanMotorPoints.cpp's g_pageState, which has no accessor yet (a same-line claim on :210, like :209's
//   W906_NativeMotorOverlay) -> page.known = false and the window shows those rows as —.
std::vector<w906native::MotorTestAxis> g_mtAxes;

void MotorTestSnapshot(w906native::MotorTestPage& page, w906native::MotorTestSummary& sum)
{
    w906native::MotorSummary ms;
    MotorSnapshot(ms);   // refreshes g_motRows (golden order, values)
    sum.buildConfig = ms.buildConfig;
    sum.provider = "wb_serve: MotorSnapshot + the EastSun overlay + HTMotor engine memory (no card access)";
    sum.why = ms.why;
    sum.pageWhy = "整頁狀態（Jog／Avg 時間、Motor Power、Lock、ActiveIndex）沒有來源：MotorTestPageState 的掛鉤還沒有取得函式（ChanMotorPoints.cpp:210，待認領）";
    sum.monitorOpen = ms.monitorOpen;
    sum.monitorAxes = ms.monitorAxes;
    sum.pollCount = ms.pollCount;
    sum.keepaliveCalls = ms.keepaliveCalls;
    page = w906native::MotorTestPage();   // known = false
    ht9045::sjson::MotorRuntimeOverlayFn ovFn = ht9045::sjson::W906_NativeMotorOverlay();
    std::size_t n = 0;
    for (std::size_t k = 0; k < g_motRows.size(); ++k)
        if (g_motRows[k].mtVisible) ++n;
    g_mtAxes.resize(n);   // kept between ticks: the strings below are assigned only when they differ
    std::size_t j = 0;
    for (std::size_t k = 0; k < g_motRows.size(); ++k) {
        const w906native::MotorRow& m = g_motRows[k];
        if (!m.mtVisible) continue;
        w906native::MotorTestAxis& a = g_mtAxes[j++];
        a.row = m.row;
        AssignIfDiffers(a.alias, m.alias.c_str());
        AssignIfDiffers(a.no, m.no.c_str());
        AssignIfDiffers(a.cardModel, m.cardModel.c_str());
        a.boardId = m.boardId;
        a.port = m.port;
        a.hasCmd = m.hasCur; a.cmd = m.cur;   // edtCommandPos = ReadPos (1203: monitor cmdPos; SIM: MOT[].Position)
        a.hasSpeed = m.hasSpeed; a.speed = m.speed;   // lblRealSpeed = GetSpeed() (cached iSpeed)
        a.servoOn = m.servoOn; a.alarm = m.alarm; a.busy = m.busy; a.inPos = m.inPos; a.homeFlag = m.homeFlag;
        a.ledKnown = m.ledKnown; a.motionIO = m.motionIO; a.state = m.state;
        AssignIfDiffers(a.quality, m.quality.c_str());
        AssignIfDiffers(a.source, m.source.c_str());
        AssignIfDiffers(a.errText, m.errText.c_str());
        a.hasEnc = false; a.enc = 0; a.loopJob = -1; a.homeJob = -1; a.hasLoopCount = false; a.loopCount = 0;
        ht9045::sjson::MotorRuntimeOverlay ov;
        if (ovFn != 0 && ovFn(m.alias, ov)) {   // pnlEncoderPos = ReadEncoderPos (1203: monitor encPos), lblLoopCount
            a.hasEnc = ov.posKnown; a.enc = ov.encPos;
            a.loopJob = ov.loopJob ? 1 : 0; a.homeJob = ov.homeJob ? 1 : 0;
            a.hasLoopCount = true; a.loopCount = ov.loopCount;
        }
        const int mi = m.motIndex;
        a.hasParams = false; a.hasHomeOffset = false;
        if (mi >= 0 && mi < MAX_TRAY_MOTOR && MOT[mi].Motor != 0) {   // engine memory only (Motor/HTMotor.h:102-128)
            HTMotor* h = MOT[mi].Motor;
            a.hasParams = true;
            a.initSpeed = h->ReadInitSpeed();
            a.jogHigh = h->PJogHighSpeed; a.jogLow = h->PJogLowSpeed;
            a.homeHigh = h->PHomeHighSpeed; a.homeLow = h->PHomeLowSpeed;
            a.softP = h->PSoftLimitP; a.softN = h->PSoftLimitN;
            a.acc = h->ReadAcc(); a.dec = h->ReadDec(); a.range = h->ReadRange();
            a.hasHomeOffset = true; a.homeOffset = h->LastHomePos;   // edtHomeOffset (golden :961, 1203: no GearRatio step)
        }
    }
}

// ---------------------------------------------------------------- HW.home ----
// AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): golden TfHome's display (V912 uhome.cpp), v1 display only.
//   Memory reads only, all on this thread (the home state machine, uhome.cpp ProcessMotorHome, runs on it too):
//     * rows   = fHome->HomeClass (forms/fHome.h:204), Visible == true only, vector order (golden layout loop :357-375);
//                name = labName->Caption (forms/fHome.cpp:161 = golden :82 MOT[MotNo].NumberAlias); position =
//                edPos->Text as the port keeps it: "0" from the ctor (forms/fHome.cpp:166 = golden :90), then what
//                ShowMotorHomePos writes when a motor finishes homing (forms/fHome.cpp:533 / :535 = golden :694 / :696)
//     * lamp   = no source yet: the port's ShowLed is a GATE no-op (forms/fHome.cpp:485-503) -> -1 ("—").  Once a
//                same-line claim on forms/fHome.cpp stores ShowLed's attr into W906_HomeLedState[], read it here.
//     * log    = fHome->ListBox1->Items (forms/fHome.h:228; uhome.cpp only ever Insert(0, ...) and Clear()) -- re-read
//                only when the count, the newest or the oldest line changed (else two AnsiString copies per tick)
//     * status = fShow / iHomeStep / fAbort (forms/fHome.h:140 / :165 / :180); Panel2 ("Reset OK") has no member in the
//                port (GATE W906-HOME-C2-PANEL2, uhome.cpp:639-645 / :3418-3424) -> -1 ("—")
//   NOT called (St02-E2 inventory §0.1): ShowMotorHomePos (reads the card; Z > 5000 -> StopAllMotor, SystemStart=false),
//   ShowLed / ResetAllMotorLed, Show() / Close() (fShow drives the home sequence), sbAbortHomeClick, InitialHomeClass.
std::vector<w906native::HomeRowData> g_homeRows;
std::vector<std::string>             g_homeLog;

void HomeSnapshot(w906native::HomePage& page, w906native::HomeSummary& sum)
{
    sum.buildConfig = BuildConfig();
    sum.provider = "wb_serve: fHome->HomeClass (labName / edPos) + fHome->ListBox1 + fShow / iHomeStep / fAbort (memory only)";
    sum.lampWhy = "移植樹的 ShowLed 是 GATE 空函式（forms/fHome.cpp:485-503），燈號狀態還沒有存起來（待認領）";
    sum.resetWhy = "移植樹沒有 Panel2（GATE W906-HOME-C2-PANEL2，uhome.cpp:639-645／:3418-3424）";
    sum.keepaliveCalls = w906native::HostKeepaliveCalls();
    page = w906native::HomePage();   // known = false, resetOk = -1
    if (fHome == 0) {
        sum.why = "fHome 是 NULL";
        g_homeRows.clear();
        g_homeLog.clear();
        return;
    }
    page.known = true;
    page.fShow = fHome->fShow;   //AI(W906-MERGE-NATIVE) 20260929: stays DIRECT on purpose -- the native HW.home window DISPLAYS the C++ member as it is (with fAbort / iHomeStep next to it); W906_FormShowing would OR in the web page's state and the cell would no longer show what C++ holds. FShow_Audit baseline raised in the same commit.
    page.fAbort = fHome->fAbort;
    page.homeStep = fHome->iHomeStep;

    const std::vector<THomeClass*>& hc = fHome->HomeClass;
    sum.classCount = (int)hc.size();
    std::size_t n = 0;
    for (std::size_t i = 0; i < hc.size(); ++i)
        if (hc[i] != 0 && hc[i]->Visible) ++n;
    sum.hiddenCount = (int)(hc.size() - n);
    if (hc.empty()) sum.why = "HomeClass 是空的：InitialHomeClass 還沒跑（cinitial.cpp:4636）";
    else if (n == 0) sum.why = "HomeClass 沒有 Visible 的列（這台的機型設定沒有要顯示的馬達）";
    g_homeRows.resize(n);   // kept between ticks: the strings below are assigned only when they differ
    std::size_t j = 0;
    for (std::size_t i = 0; i < hc.size(); ++i) {
        const THomeClass* c = hc[i];
        if (c == 0 || !c->Visible) continue;
        w906native::HomeRowData& r = g_homeRows[j++];
        r.index = c->index;
        AssignIfDiffers(r.name, c->labName ? c->labName->Caption.c_str() : "");
        r.hasPos = (c->edPos != 0);
        AssignIfDiffers(r.pos, c->edPos ? c->edPos->Text.c_str() : "");
        r.lamp = -1;   // no source yet (see above)
    }

    // ---- log: golden ListBox1, newest first ----
    const vclcompat::TStringList* items = fHome->ListBox1 ? fHome->ListBox1->Items : 0;
    const int cnt = items ? items->GetCount() : 0;
    sum.logLines = cnt;
    bool same = (cnt == (int)g_homeLog.size());
    if (same && cnt > 0)
        same = std::strcmp(items->GetString(0).c_str(), g_homeLog.front().c_str()) == 0 &&
               std::strcmp(items->GetString(cnt - 1).c_str(), g_homeLog.back().c_str()) == 0;
    if (!same) {
        g_homeLog.resize((std::size_t)cnt);
        for (int k = 0; k < cnt; ++k) AssignIfDiffers(g_homeLog[(std::size_t)k], items->GetString(k).c_str());
    }
}

// ------------------------------------------------------------ HW.teach ----
// AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): the teach registry of TfTeach as a table, v1 display only.
//   Rows: fTeach->TechPara / TechTwoPara (forms/fTeach.h:790-792; built by forms/fTeachRegistry.cpp = golden
//   uteach.cpp:342-995).  Live statements: 260 TECH_PARA (265 `TechPara.push_back` lines minus the five inside the
//   /* */ at forms/fTeachRegistry.cpp:352-358 = golden 906_0625_Steven uteach.cpp:619-625) and 52 TECH_TWOPARA; the
//   runtime count is smaller still (if / else config branches) and is printed at boot, tools/wb_serve.cpp:3916.
//   Per row: Key, MotorSelect, MOT[MotorSelect].Alias, the golden tab of the Key (w906native::TeachFindKey, generated
//   from the dfm2rc IR), and the value *Parameter.
//   Axes: one per DISTINCT motor the rows use -- the EastSun overlay (W906_NativeMotorOverlay, the same hook as
//   Main.MotorView / HW.MotorTest) is called ONCE per motor per tick, not once per row; MOT[i].Position (SIM only, the
//   P5 rule of MotorSnapshot), HomeFlag, NumberAlias (golden Panel2), Motor->MotorType / LastHomePos (engine memory).
//   Memory reads ONLY.  NOT done (St02-E2 inventory §0.1): no card read (golden ScanNowMotorStatus's ScanMotorStatus /
//   Gali_ScanMotStatus), no SetSpeed on axis select (V912 UpdateMotorTeachMonitor :1290), no Timer1 (DoZHome /
//   ProcessSingleMotorHome / DoPitch_Home), no ReadFile / teach.ini read (Tech is already in memory), no fTeach method
//   that changes state.  The static fields are built once and rebuilt only when the registry or an alias changes
//   (strcmp per tick, no AnsiString -> std::string per tick).
std::vector<w906native::TeachPoint>  g_tcPoints;
std::vector<w906native::TeachAxis>   g_tcAxes;
std::vector<const TECH_PARA*>        g_tcSrc1;
std::vector<const TECH_TWOPARA*>     g_tcSrc2;
const TfTeach*                       g_tcTeach = 0;
bool                                 g_tcBuilt = false;

bool TeachValidMot(int mi) { return mi >= 0 && mi < MAX_TRAY_MOTOR; }

int TeachMotOrNone(int mi) { return TeachValidMot(mi) ? mi : -1; }

const char* TeachAliasOf(int mi) { return TeachValidMot(mi) ? MOT[mi].Alias.c_str() : ""; }

bool TeachSlotSame(int motorSelect, const AnsiString& key, const w906native::TeachPoint& p, int k)
{
    return TeachMotOrNone(motorSelect) == p.motIndex[k] && std::strcmp(key.c_str(), p.key[k].c_str()) == 0 &&
           std::strcmp(TeachAliasOf(motorSelect), p.alias[k].c_str()) == 0;
}

bool TeachRegistrySame()
{
    if (!g_tcBuilt || fTeach != g_tcTeach) return false;
    if (!fTeach) return true;
    if (fTeach->TechPara.size() != g_tcSrc1.size() || fTeach->TechTwoPara.size() != g_tcSrc2.size()) return false;
    for (std::size_t i = 0; i < g_tcSrc1.size(); ++i) {
        const TECH_PARA* r = fTeach->TechPara[i];
        if (r != g_tcSrc1[i]) return false;
        if (r && !TeachSlotSame(r->MotorSelect, r->Key, g_tcPoints[i], 0)) return false;
    }
    for (std::size_t j = 0; j < g_tcSrc2.size(); ++j) {
        const TECH_TWOPARA* r = fTeach->TechTwoPara[j];
        if (r != g_tcSrc2[j]) return false;
        const w906native::TeachPoint& p = g_tcPoints[g_tcSrc1.size() + j];
        if (r && (!TeachSlotSame(r->MotorSelect[0], r->Key[0], p, 0) || !TeachSlotSame(r->MotorSelect[1], r->Key[1], p, 1)))
            return false;
    }
    for (std::size_t a = 0; a < g_tcAxes.size(); ++a) {
        const int mi = g_tcAxes[a].motIndex;
        if (std::strcmp(MOT[mi].Alias.c_str(), g_tcAxes[a].alias.c_str()) != 0 ||
            std::strcmp(MOT[mi].NumberAlias.c_str(), g_tcAxes[a].numberAlias.c_str()) != 0)
            return false;
    }
    return true;
}

void TeachFillSlot(w906native::TeachPoint& p, int k, int motorSelect, const AnsiString& key, std::vector<int>& mots)
{
    p.key[k] = Str(key);
    p.motIndex[k] = TeachMotOrNone(motorSelect);
    p.alias[k] = TeachAliasOf(motorSelect);
    if (p.motIndex[k] >= 0 && std::find(mots.begin(), mots.end(), p.motIndex[k]) == mots.end()) mots.push_back(p.motIndex[k]);
}

void TeachFillKeyInfo(w906native::TeachPoint& p)
{
    const w906native::TeachKeyInfo* ki = w906native::TeachFindKey(p.key[0]);
    if (ki) { p.tab = ki->tab; p.group = ki->group; p.label = ki->label; p.goldenHidden = ki->goldenHidden; }
    if (ki && ki->goldenHidden) p.label += "（golden 建構時隱藏：Visible=false）";   // AI(W906-NATIVE-ST02) 20260929 (St02-E, St02-E2 teach review note 2)
}

void TeachBuildStatic()
{
    g_tcBuilt = true;
    g_tcTeach = fTeach;
    g_tcPoints.clear();
    g_tcAxes.clear();
    g_tcSrc1.clear();
    g_tcSrc2.clear();
    if (!fTeach) return;
    std::vector<int> mots;
    g_tcPoints.reserve(fTeach->TechPara.size() + fTeach->TechTwoPara.size());
    for (std::size_t i = 0; i < fTeach->TechPara.size(); ++i) {
        const TECH_PARA* r = fTeach->TechPara[i];
        g_tcSrc1.push_back(r);
        w906native::TeachPoint p;
        p.index = (int)i;
        p.two = false;
        if (r) TeachFillSlot(p, 0, r->MotorSelect, r->Key, mots);
        TeachFillKeyInfo(p);
        g_tcPoints.push_back(p);
    }
    for (std::size_t j = 0; j < fTeach->TechTwoPara.size(); ++j) {
        const TECH_TWOPARA* r = fTeach->TechTwoPara[j];
        g_tcSrc2.push_back(r);
        w906native::TeachPoint p;
        p.index = (int)j;
        p.two = true;
        if (r) {
            TeachFillSlot(p, 0, r->MotorSelect[0], r->Key[0], mots);
            TeachFillSlot(p, 1, r->MotorSelect[1], r->Key[1], mots);
        }
        TeachFillKeyInfo(p);
        g_tcPoints.push_back(p);
    }
    std::sort(mots.begin(), mots.end());
    for (std::size_t k = 0; k < mots.size(); ++k) {
        w906native::TeachAxis a;
        a.motIndex = mots[k];
        a.alias = Str(MOT[mots[k]].Alias);
        a.numberAlias = Str(MOT[mots[k]].NumberAlias);
        g_tcAxes.push_back(a);
    }
}

void TeachSnapshot(w906native::TeachPage& page, w906native::TeachSummary& sum)
{
    using ht9045::sjson::MotorRuntimeOverlay;
    ht9045::sjson::MotorRuntimeOverlayFn ovFn = ht9045::sjson::W906_NativeMotorOverlay();
    ht9045::TPci1203Monitor* mon = ht9045::Pci1203Monitor();
    sum.buildConfig = BuildConfig();
    sum.provider = "wb_serve: fTeach->TechPara / TechTwoPara + *Parameter + MOT[] + the EastSun overlay (memory only)";
    sum.monitorOpen = mon && mon->card().open;
    sum.monitorAxes = mon ? mon->axisCount() : 0;
    sum.pollCount = mon ? (unsigned long)mon->card().pollCount : 0;
    sum.keepaliveCalls = w906native::HostKeepaliveCalls();

    if (!TeachRegistrySame()) TeachBuildStatic();
    sum.techParaCount = (int)g_tcSrc1.size();
    sum.twoParaCount = (int)g_tcSrc2.size();
    sum.distinctMotors = (int)g_tcAxes.size();
    if (!fTeach) sum.registryWhy = "fTeach 還沒建立（tools/wb_serve.cpp:3914，在 LoadMachineConfig 之後才建）：沒有教導點可列";
    page.known = fTeach != 0;
    page.setToOffset = (fTeach && fTeach->edtSetToOffset) ? fTeach->edtSetToOffset->Text.c_str() : "";   // AI(W906-NATIVE-ST02) 20260929 (St02-E, St02-E2 teach review note 3): golden's own edit
    page.activeMotor = fTeach ? fTeach->ActiveMotorIndex : -1;   // golden ActiveMotorIndex (the web page writes it, WebMotorAccessLive.cpp:539)

    // ---- values: *Parameter (memory; teach.ini is NOT read here -- ReadFile ran at boot) ----
    for (std::size_t i = 0; i < g_tcSrc1.size(); ++i) {
        const TECH_PARA* r = g_tcSrc1[i];
        w906native::TeachPoint& p = g_tcPoints[i];
        p.hasValue[0] = r && r->Parameter;
        p.value[0] = p.hasValue[0] ? *r->Parameter : 0;
    }
    for (std::size_t j = 0; j < g_tcSrc2.size(); ++j) {
        const TECH_TWOPARA* r = g_tcSrc2[j];
        w906native::TeachPoint& p = g_tcPoints[g_tcSrc1.size() + j];
        for (int k = 0; k < 2; ++k) {
            p.hasValue[k] = r && r->Parameter[k];
            p.value[k] = p.hasValue[k] ? *r->Parameter[k] : 0;
        }
    }

    // ---- axes: once per distinct motor; value rules as MotorSnapshot (ChanMotorPoints.cpp MotorRuntimeJson) ----
    unsigned long calls = 0;
    int nLive = 0;
    const w906native::TeachAxis blank;
    for (std::size_t k = 0; k < g_tcAxes.size(); ++k) {
        w906native::TeachAxis& a = g_tcAxes[k];
        const int mi = a.motIndex;
        a.motorType = blank.motorType; a.hasNow = blank.hasNow; a.now = blank.now; a.hasEnc = blank.hasEnc; a.enc = blank.enc;
        a.hasHomeOffset = blank.hasHomeOffset; a.homeOffset = blank.homeOffset;
        a.servoOn = blank.servoOn; a.alarm = blank.alarm; a.busy = blank.busy; a.inPos = blank.inPos; a.homeFlag = blank.homeFlag;
        a.ledKnown = blank.ledKnown; a.motionIO = blank.motionIO; a.state = blank.state;
        const bool motObj = MOT[mi].Motor != 0;
#ifdef SOFT_SIMULTE
        const bool motLive = motObj;
#else
        const bool motLive = false;   // P5: the ship build does not take MOT[] positions as card values
#endif
        // golden V912 ScanNowMotorStatus :1336-1342: the four Index axes are read through Galil when INDEX_MOTION_CARD==0
        const bool galil = INDEX_MOTION_CARD == 0 && (mi == MTestY1 || mi == MTestZ1 || mi == MTestZ2 || mi == MTestY2);
        MotorRuntimeOverlay ov;
        bool hasOv = false;
        if (ovFn != 0 && !a.alias.empty()) {
            ++calls;
            hasOv = ovFn(a.alias, ov);
        }
        const bool live = motLive || hasOv;
        if (live) ++nLive;
        if (motObj) a.motorType = MOT[mi].Motor->MotorType;
        if (hasOv) { a.hasNow = ov.posKnown; a.now = ov.cmdPos; }       // edtNowPosition = ReadPos (1203: monitor cmdPos)
        else if (motLive) { a.hasNow = true; a.now = MOT[mi].Position; }
        // pnlEncoderPos (golden V912 :1347-1350): ReadEncoderPos for MotorType 1 / 3, else ReadPos; Galil / no drive object:
        // the monitor's encoder when there is one (as HW.MotorTest)
        if (!galil && motObj && a.motorType != 1 && a.motorType != 3) { a.hasEnc = a.hasNow; a.enc = a.now; }
        else if (hasOv) { a.hasEnc = ov.posKnown; a.enc = ov.encPos; }
        if (motObj) {
            a.hasHomeOffset = true; a.homeOffset = MOT[mi].Motor->LastHomePos;   // edtSetToOffset (golden V912 Timer1 :1399 / :1406)
            a.homeFlag = MOT[mi].HomeFlag;
        }
        if (hasOv) {
            a.servoOn = ov.servoOn ? 1 : 0;
            a.alarm = ov.alarm ? 1 : 0;
            a.inPos = ov.inPos ? 1 : 0;
            a.busy = ov.busy ? 1 : 0;
            a.state = ov.state;
            a.ledKnown = ov.ioKnown;
            a.motionIO = ov.motionIO;
        }
        AssignIfDiffers(a.quality, hasOv ? (ov.posKnown ? "good" : "partial") : (live ? "good" : "nosource"));
        AssignIfDiffers(a.source, hasOv ? "pci1203-monitor" : (motLive ? "MOT[]" : "none"));
        std::string err;
        if (hasOv && !ov.posKnown) err = ov.why;
        if (hasOv && ov.alarm && !ov.driveErr.empty()) err += (err.empty() ? "" : "; ") + ov.driveErr;
        if (motObj && MOT[mi].HomeFlag == 2) err += (err.empty() ? "" : "; ") + std::string("home failed (HomeFlag=2)");
        if (galil) err += (err.empty() ? "" : "; ") + std::string("golden 這一軸走 Galil（INDEX_MOTION_CARD==0：Gali_ReadPos／Gali_ReadEncoderPos）；這裡用與 Motor Test 相同的來源");
        if (!live && !galil && err.empty()) {
            if (a.alias.empty()) err = "MOT[] 的 Alias 空：馬達表沒有這一軸";
#ifdef SOFT_SIMULTE
            else err = "SIM：MOT[i].Motor 是 NULL（沒有驅動物件）";
#else
            else err = "出貨組態：覆蓋掛鉤沒有這一軸（不是 PCI1203，或 1203 監看器沒有開到它）";
#endif
        }
        if (a.errText != err) a.errText.swap(err);
    }
    sum.overlayCalls = calls;
    if (nLive == 0 && !g_tcAxes.empty()) {
#ifdef SOFT_SIMULTE
        sum.why = "沒有任何一軸有值：MOT[i].Motor 全為 NULL，而且覆蓋掛鉤沒有任何一軸";
#else
        sum.why = "沒有任何一軸對到 1203 監看器開著的軸；出貨組態不拿 MOT[] 當來源";
#endif
    }
}

// ------------------------------------------------------------ 畫面更新 ----
void RefreshIfDue(bool force)
{
    const DWORD now = ::GetTickCount();
    if (w906native::IoViewIsOpen() && (force || now - g_lastIo >= kIoRefreshMs)) {
        g_lastIo = now;
        w906native::IoSummary sum;
        IoSnapshot(sum);
        w906native::IoViewUpdate(g_ioRows, sum);   // 視窗只比較值、只畫變了的格子
    }
    if (w906native::MotorViewIsOpen() && (force || now - g_lastMotor >= kMotorRefreshMs)) {
        g_lastMotor = now;
        w906native::MotorSummary sum;
        MotorSnapshot(sum);
        w906native::MotorViewUpdate(g_motRows, sum);
    }
    if (w906native::MotorTestIsOpen() && (force || now - g_lastMotorTest >= kMotorTestRefreshMs)) {   // AI(W906-NATIVE-ST02) 20260929
        g_lastMotorTest = now;
        w906native::MotorTestPage page;
        w906native::MotorTestSummary sum;
        MotorTestSnapshot(page, sum);
        w906native::MotorTestUpdate(g_mtAxes, page, sum);   // only changed rows are marked; nothing changed = 0 cells
    }
    W906_NativeShuttleMoveRefresh(force);   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1 (ui/native/NativeShuttleMoveGlue.cpp) -- its own 20 ms
    if (w906native::HomeIsOpen() && (force || now - g_lastHome >= kHomeRefreshMs)) {   // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016)
        g_lastHome = now;
        w906native::HomePage page;
        w906native::HomeSummary sum;
        HomeSnapshot(page, sum);
        w906native::HomeUpdate(g_homeRows, g_homeLog, page, sum);   // only changed rows are marked; nothing changed = 0 cells
    }
    if (w906native::TeachIsOpen() && (force || now - g_lastTeach >= kTeachRefreshMs)) {   // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016)
        g_lastTeach = now;
        w906native::TeachPage page;
        w906native::TeachSummary sum;
        TeachSnapshot(page, sum);
        w906native::TeachUpdate(g_tcPoints, g_tcAxes, page, sum);   // the rows of a moving motor only; nothing changed = 0 cells
    }
}

void KeepaliveMain()
{
    W906_NativeKeepaliveMain();   // 一拍主迴圈的週期工作（wb_serve.cpp 檔尾）
    RefreshIfDue(false);          // 拖曳時畫面也照常更新 —— 看得出主迴圈沒停
}

void KeepaliveModal()
{
    W906_NativeKeepaliveModal();
}

void ReopenIo()    { if (w906native::IoViewOpen(true)) RefreshIfDue(true); }
void ReopenMotor() { if (w906native::MotorViewOpen(true)) RefreshIfDue(true); }
void ReopenMotorTest() { if (w906native::MotorTestOpen(true)) RefreshIfDue(true); }   // AI(W906-NATIVE-ST02) 20260929
void ReopenShuttleMove() { W906_NativeShuttleMoveReopen(); RefreshIfDue(true); }   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1 (ui/native/NativeShuttleMoveGlue.cpp)
void ReopenHome() { if (w906native::HomeOpen(true)) RefreshIfDue(true); }   // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016)
void ReopenTeach() { if (w906native::TeachOpen(true)) RefreshIfDue(true); }   // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016)

void OnSizeMove(bool enter, unsigned long kaCalls)
{
    if (enter) std::printf("[native] window drag/resize started: the main loop is kept alive by the window timer (NativeHost.h)\n");
    else std::printf("[native] window drag/resize ended: %lu keepalive ticks ran during it\n", kaCalls);
    std::fflush(stdout);
}

}  // namespace

void W906_NativeFormsStart()
{
    if (g_started) return;
    g_started = true;
    w906native::HostSetSizeMoveNotify(&OnSizeMove);
    const bool okIo = w906native::IoViewOpen(true);
    const bool okMot = w906native::MotorViewOpen(true);
    const bool hkIo = w906native::HostRegisterHotkey(kHotkeyIo, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'I', &ReopenIo);
    const bool hkMot = w906native::HostRegisterHotkey(kHotkeyMotor, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'M', &ReopenMotor);
    // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.MotorTest v1 (display only), opened like the other two
    const bool okMt = w906native::MotorTestOpen(true);
    const bool hkMt = w906native::HostRegisterHotkey(kHotkeyMotorTest, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'T', &ReopenMotorTest);
    std::printf("[native] HW.MotorTest window (v1 display only) %s; reopen hotkey Ctrl+Alt+T %s\n", okMt ? "OPEN" : "FAILED",
                hkMt ? "registered" : "NOT registered (in use?)");
    const bool okSm = W906_NativeShuttleMoveOpen(true);   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1 (ui/native/NativeShuttleMoveGlue.cpp)
    const bool hkSm = w906native::HostRegisterHotkey(kHotkeyShuttleMove, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'S', &ReopenShuttleMove);
    std::printf("[native] HW.ShuttleMove window (v1 display only) %s; reopen hotkey Ctrl+Alt+S %s\n", okSm ? "OPEN" : "FAILED",
                hkSm ? "registered" : "NOT registered (in use?)");
    // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.home v1 (display only), opened like the others
    const bool okHm = w906native::HomeOpen(true);
    const bool hkHm = w906native::HostRegisterHotkey(kHotkeyHome, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'H', &ReopenHome);
    std::printf("[native] HW.home window (v1 display only) %s; reopen hotkey Ctrl+Alt+H %s\n", okHm ? "OPEN" : "FAILED",
                hkHm ? "registered" : "NOT registered (in use?)");
    // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016): HW.teach v1 (a table, display only), opened like the others; reopen Ctrl+Alt+K
    const bool okTc = w906native::TeachOpen(true);
    const bool hkTc = w906native::HostRegisterHotkey(kHotkeyTeach, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'K', &ReopenTeach);
    std::printf("[native] HW.teach window (v1 display only) %s; reopen hotkey Ctrl+Alt+K %s\n", okTc ? "OPEN" : "FAILED",
                hkTc ? "registered" : "NOT registered (in use?)");
    std::printf("[native] W906_NATIVE_FORMS prototype (read-only): HW.IoSetView window %s, Main.MotorView window %s; "
                "reopen hotkeys Ctrl+Alt+I %s, Ctrl+Alt+M %s\n",
                okIo ? "OPEN" : "FAILED", okMot ? "OPEN" : "FAILED",
                hkIo ? "registered" : "NOT registered (in use?)", hkMot ? "registered" : "NOT registered (in use?)");
    std::fflush(stdout);
    RefreshIfDue(true);
}

void W906_NativeFormsPump(int site)
{
    if (!g_started) return;
    w906native::PumpThreadMessages(200, site == 0 ? &KeepaliveMain : &KeepaliveModal);
    RefreshIfDue(false);
}

void W906_NativeFormsStop()
{
    if (!g_started) return;
    w906native::IoViewClose();
    w906native::MotorViewClose();
    w906native::MotorTestClose();   // AI(W906-NATIVE-ST02) 20260929
    W906_NativeShuttleMoveClose();   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1 (ui/native/NativeShuttleMoveGlue.cpp)
    w906native::HomeClose();   // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016)
    w906native::TeachClose();   // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016)
    w906native::PumpThreadMessages(200, 0);
    w906native::HostUnregisterHotkey(kHotkeyIo);
    w906native::HostUnregisterHotkey(kHotkeyMotor);
    w906native::HostUnregisterHotkey(kHotkeyMotorTest);   // AI(W906-NATIVE-ST02) 20260929
    w906native::HostUnregisterHotkey(kHotkeyShuttleMove);   // AI(W906-NATIVE-ST02) 20260929 (St02-E, E-016): HW.ShuttleMove v1 (ui/native/NativeShuttleMoveGlue.cpp)
    w906native::HostUnregisterHotkey(kHotkeyHome);   // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016)
    w906native::HostUnregisterHotkey(kHotkeyTeach);   // AI(W906-NATIVE-ST02) 20260929 (St02-E helper, E-016)
    w906native::HostSetSizeMoveNotify(0);
    g_started = false;
}
