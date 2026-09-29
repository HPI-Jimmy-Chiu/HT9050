// ===========================================================================
//  tests/test_native_forms.cpp
//
//  AI(W906-NATIVE-PROTO) 20260928 [W906]: 原生表單原型（HW.IoSetView／Main.MotorView 唯讀＋拖曳保活）的無顯示 ctest ＋ 展示模式。
//  NOT in golden。
//
//    test_native_forms.exe --io          ctest NativeIoView_Headless：IO 視窗建起來「不顯示」→ 灌假資料 → 經 ListView 讀回 → 篩選 → 停用 → 關掉
//    test_native_forms.exe --motor       ctest NativeMotorView_Headless：同上，馬達表；另驗「沒有任何按鈕」與十顆燈的位元解碼
//    test_native_forms.exe --perf        ctest NativeGrid_Efficiency（20260929）：沒變就 0 格、一顆燈變只畫 2 格、看不到的列 0 格、
//                                        背景圖上的燈色就是資料的顏色；700 點／60 軸連續 1000 次更新的平均成本（Steven：「得快到20ms一次」）
//    test_native_forms.exe --keepalive   ctest NativeHost_Keepalive：拖曳保活（NativeHost.h）—— 用一個「在我們的泵分派的訊息裡
//                                        自己跑訊息迴圈」的輔助視窗模擬 Windows 的內部迴圈，驗保活有被叫、沒有多叫、不重入
//    test_native_forms.exe --modal-move  （不在 ctest）真的叫 Windows 的鍵盤移動迴圈（WM_SYSCOMMAND SC_MOVE），看保活有沒有被叫
//    test_native_forms.exe --snapshot <prefix> [IO_Table.csv|-] [Mot_Table.csv|-]
//                                        （不在 ctest）同展示，1.5 秒後用 PrintWindow 把兩個視窗存成 <prefix>_io.bmp／_motor.bmp 就結束
//    test_native_forms.exe --show [IO_Table.csv|-] [Mot_Table.csv|-]
//                                        展示：兩個看得到的視窗、假燈號／假位置持續變化；給了表就用真實點名與軸名（只讀）。
//                                        20260929：每 20 ms 更新一次（燈號照舊約 0.6～1.8 秒切一次，位置每 20 ms 都在變）。
//                                        按住標題列拖曳時燈號照樣在變 = 保活有效。兩個視窗都關掉就結束。
//
//  ⚠ 本測試只連 ui/native/NativeHost.cpp、NativeGrid.cpp、NativeIoView.cpp、NativeMotorView.cpp（ui/native/NativeForms.cmake）。
//    連得起來本身就證明：視窗模組沒有任何一條呼叫路徑通到 IO 寫入、motor.access、運動或 1203 指令。
// ===========================================================================
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#include "ui/native/NativeGrid.h"
#include "ui/native/NativeHost.h"
#include "ui/native/NativeIoView.h"
#include "ui/native/NativeMotorView.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <vector>

using namespace w906native;

namespace {

int g_fail = 0;
int g_pass = 0;

void Check(bool ok, const char* what)
{
    if (ok) { ++g_pass; std::printf("  ok   %s\n", what); }
    else    { ++g_fail; std::printf("  FAIL %s\n", what); }
    std::fflush(stdout);
}

int Done()
{
    std::printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}

void Pump()
{
    for (int i = 0; i < 20; ++i) PumpThreadMessages(500, 0);
}

// 與 JsonBridge/ChanIoPoints.cpp IoDirectionOfType 同一張對照（假資料用；測試不連那支檔）。
int DirOf(const std::string& t)
{
    if (t == "Sensor" || t == "Cylinder_On" || t == "Cylinder_Off" || t == "Sucker") return kIoDirIn;
    if (t == "Switch" || t == "Cylinder" || t == "Sucker_On" || t == "Sucker_Off") return kIoDirOut;
    return kIoDirUnknown;
}

std::string CodeOf(int isa, int dir, int ip, int port, int bit)
{
    const char* d = dir == kIoDirIn ? "I" : dir == kIoDirOut ? "O" : "";
    if (!*d || ip < 0 || port < 0) return std::string();
    char b[40];
    if (isa == 3) std::snprintf(b, sizeof(b), "%s%d.%d", d, ip, port);
    else { if (bit < 0) return std::string(); std::snprintf(b, sizeof(b), "%s%02d%d%d", d, ip, port, bit); }
    return b;
}

IoRow MakeIo(int row, const char* alias, const char* type, int isa, int lane, int ip, int port, int bit,
             int inType, int enable, int raw, int isOn, const char* quality, const char* source)
{
    IoRow r;
    r.row = row; r.alias = alias; r.ioType = type; r.dir = DirOf(type);
    r.isaBase = isa; r.lane = lane; r.ip = ip; r.port = port; r.bit = bit; r.inType = inType; r.enable = enable;
    r.ioCode = CodeOf(isa, r.dir, ip, port, bit);
    r.raw = raw; r.isOn = isOn; r.quality = quality; r.source = source;
    return r;
}

std::vector<IoRow> FakeIoRows()
{
    std::vector<IoRow> v;
    v.push_back(MakeIo(2, "SnMotorPower",  "Sensor",   3, 1, 2, 30, 6, 1, 1, 1, 1, "good", "pci1203.di"));
    v.push_back(MakeIo(3, "SnEMG",         "Sensor",   3, 1, 2, 31, 7, 0, 1, 1, 0, "good", "pci1203.di"));
    v.push_back(MakeIo(4, "SwTowerRed",    "Switch",   3, 1, 16, 0, 0, 1, 1, 1, 1, "good", "pci1203.do"));
    v.push_back(MakeIo(5, "SwTowerGreen",  "Switch",   3, 1, 16, 1, 1, 1, 1, 0, 0, "good", "pci1203.do"));
    v.push_back(MakeIo(6, "CyInArmZ",      "Cylinder", 3, 1, 16, 2, 2, 1, 0, 0, -1, "disabled", "pci1203.do"));
    v.push_back(MakeIo(7, "SnInArmVac",    "Sucker",   3, 1, 2, 5, 5, 1, 1, -1, -1, "bad", "pci1203.di"));
    v.push_back(MakeIo(8, "SnOldMotionNet","Sensor",   0, 0, 1, 0, 3, 1, 1, -1, -1, "nosource", ""));
    return v;
}

MotorRow MakeMotor(int row, const char* alias, const char* no, int mi, bool vis, int order, const char* card)
{
    MotorRow m;
    m.row = row; m.alias = alias; m.no = no; m.motIndex = mi; m.mtVisible = vis; m.mtOrder = order; m.cardModel = card;
    m.enable = 1; m.boardId = row; m.port = 0;
    return m;
}

std::vector<MotorRow> FakeMotorRows()
{
    std::vector<MotorRow> v;
    MotorRow a = MakeMotor(0, "MInArmX", "M00", 0, true, 0, "PCI1203");
    a.hasCur = true; a.cur = 12345; a.hasSpeed = true; a.speed = 10000; a.can = 1; a.canL = 1; a.canM = 0; a.canR = 1;
    a.servoOn = 1; a.alarm = 0; a.inPos = 1; a.busy = 0; a.homeFlag = 1; a.ledKnown = true;
    a.motionIO = 0x00004000ul | 0x00000008ul;   // SVON + LMT-(CW)
    a.state = 1; a.quality = "good"; a.source = "pci1203-monitor";
    v.push_back(a);
    MotorRow b = MakeMotor(1, "MInArmY", "M01", 1, true, 1, "PCI1203");
    b.hasCur = true; b.cur = -50; b.servoOn = 1; b.alarm = 1; b.busy = 1; b.homeFlag = 0; b.ledKnown = true;
    b.motionIO = 0x00000002ul; b.state = 3; b.quality = "good"; b.source = "pci1203-monitor";
    b.errText = "Positive hardware limit has been exceeded";
    v.push_back(b);
    MotorRow c = MakeMotor(2, "MInArmZA", "M03", 3, true, 2, "PCI1203");
    c.hasCur = false; c.quality = "partial"; c.source = "pci1203-monitor"; c.homeFlag = 2; c.errText = "home failed (HomeFlag=2)";
    v.push_back(c);
    MotorRow d = MakeMotor(3, "MTestY1", "M14", 14, true, 3, "PCI1203");
    d.quality = "nosource"; d.source = "none"; d.errText = "1203 監看器沒有開任何軸";
    v.push_back(d);
    MotorRow e = MakeMotor(4, "MOutShuttle1", "M20", 20, false, 4, "PCI1203");   // golden 不列（MOutShuttle1/2 = false）
    e.quality = "nosource"; e.source = "none";
    v.push_back(e);
    MotorRow f = MakeMotor(5, "MOldSMC", "M40", 40, false, -1, "SMC");
    f.quality = "nosource"; f.source = "none";
    v.push_back(f);
    return v;
}

// ---- 讀表（展示用，只讀）----
int ToInt(const std::string& s, int dflt) { return s.empty() ? dflt : std::atoi(s.c_str()); }

std::vector<std::string> SplitCsv(const std::string& line)
{
    std::vector<std::string> out;
    std::string cur;
    for (std::size_t i = 0; i < line.size(); ++i) {
        if (line[i] == ',') { out.push_back(cur); cur.clear(); }
        else if (line[i] != '\r' && line[i] != '\n') cur += line[i];
    }
    out.push_back(cur);
    return out;
}

bool ReadCsv(const char* path, std::map<std::string, int>& col, std::vector<std::vector<std::string> >& data)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::string line;
    if (!std::getline(f, line)) return false;
    const std::vector<std::string> head = SplitCsv(line);
    for (std::size_t i = 0; i < head.size(); ++i) col[head[i]] = (int)i;
    while (std::getline(f, line)) {
        std::vector<std::string> c = SplitCsv(line);
        if (c.size() < head.size()) c.resize(head.size());
        data.push_back(c);
    }
    return true;
}

std::vector<IoRow> LoadIoCsv(const char* path, int* skipped)
{
    std::vector<IoRow> v;
    *skipped = 0;
    std::map<std::string, int> col;
    std::vector<std::vector<std::string> > data;
    if (!ReadCsv(path, col, data)) return v;
    const char* names[] = {"IOType", "Alias", "Lane", "IP", "Port", "Bit", "InType", "ISABase", "Enable"};
    for (int k = 0; k < 9; ++k) if (!col.count(names[k])) return v;
    for (std::size_t i = 0; i < data.size(); ++i) {
        const std::vector<std::string>& c = data[i];
        const std::string alias = c[(std::size_t)col["Alias"]];
        if (alias.empty()) { ++*skipped; continue; }
        v.push_back(MakeIo((int)i, alias.c_str(), c[(std::size_t)col["IOType"]].c_str(),
                           ToInt(c[(std::size_t)col["ISABase"]], 0), ToInt(c[(std::size_t)col["Lane"]], -1),
                           ToInt(c[(std::size_t)col["IP"]], -1), ToInt(c[(std::size_t)col["Port"]], -1),
                           ToInt(c[(std::size_t)col["Bit"]], -1), ToInt(c[(std::size_t)col["InType"]], 0),
                           ToInt(c[(std::size_t)col["Enable"]], 0), -1, -1, "nosource", ""));
    }
    return v;
}

std::vector<MotorRow> LoadMotorCsv(const char* path)
{
    std::vector<MotorRow> v;
    std::map<std::string, int> col;
    std::vector<std::vector<std::string> > data;
    if (!ReadCsv(path, col, data)) return v;
    const char* names[] = {"Motorname", "Alias", "BoardID", "Port", "Enable", "CardModel"};
    for (int k = 0; k < 6; ++k) if (!col.count(names[k])) return v;
    for (std::size_t i = 0; i < data.size(); ++i) {
        const std::vector<std::string>& c = data[i];
        const std::string alias = c[(std::size_t)col["Alias"]];
        if (alias.empty()) continue;
        const std::string no = c[(std::size_t)col["Motorname"]];
        MotorRow m = MakeMotor((int)i, alias.c_str(), no.c_str(), no.size() > 1 ? std::atoi(no.c_str() + 1) : -1,
                               true, (int)i, c[(std::size_t)col["CardModel"]].c_str());
        m.enable = ToInt(c[(std::size_t)col["Enable"]], 0);
        m.boardId = ToInt(c[(std::size_t)col["BoardID"]], -1);
        m.port = ToInt(c[(std::size_t)col["Port"]], -1);
        v.push_back(m);
    }
    return v;
}

void AnimateIo(std::vector<IoRow>& v, unsigned tick)
{
    for (std::size_t i = 0; i < v.size(); ++i) {
        IoRow& r = v[i];
        if (r.isaBase != 3 || r.dir == kIoDirUnknown) { r.raw = -1; r.isOn = -1; r.quality = "nosource"; r.source = ""; continue; }
        r.source = r.dir == kIoDirIn ? "pci1203.di" : "pci1203.do";
        const unsigned period = 3u + (unsigned)(i % 7u);
        r.raw = (int)(((tick + (unsigned)i) / period) & 1u);
        if (r.enable != 1) { r.isOn = -1; r.quality = "disabled"; continue; }
        r.isOn = r.inType ? r.raw : (r.raw ? 0 : 1);
        r.quality = "good";
    }
}

// tick＝慢拍（每 200 ms 加一：燈號、Can、到位的變化速度與 20260928 版相同）；fast＝快拍（每 20 ms 加一：位置每一拍都在變）。
void AnimateMotor(std::vector<MotorRow>& v, unsigned tick, unsigned fast)
{
    for (std::size_t i = 0; i < v.size(); ++i) {
        MotorRow& m = v[i];
        if (m.cardModel != "PCI1203" || m.enable != 1) { m.quality = "nosource"; m.source = "none"; m.errText = "DEMO：不是 1203 或 Enable=0"; continue; }
        m.hasCur = true;
        m.cur = (int)((fast * (7u + (unsigned)i * 3u)) % 200000u) - 100000;
        m.hasTarget = true; m.target = m.cur + 500;
        m.hasSpeed = true; m.speed = 1000 * (int)(1 + i % 5);
        m.can = 1; m.canL = 1; m.canM = (int)((tick / 10u + i) & 1u); m.canR = 1;
        m.servoOn = 1; m.inPos = (int)((tick / 4u + i) & 1u); m.busy = m.inPos ? 0 : 1;
        m.alarm = (i == 5) ? 1 : 0; m.homeFlag = (i == 7) ? 2 : 1;
        m.ledKnown = true;
        m.motionIO = 0x00004000ul | ((tick / 3u + i) % 5u == 0 ? 0x00000010ul : 0ul) | (m.alarm ? 0x00000002ul : 0ul);
        m.state = m.alarm ? 3u : (m.busy ? 5u : 1u);
        m.quality = "good"; m.source = "DEMO"; m.errText = m.alarm ? "DEMO：假警報" : "";
    }
}

// ---- 展示 ----
std::vector<IoRow>    g_demoIo;
std::vector<MotorRow> g_demoMot;
unsigned              g_demoTick = 0;
double                g_demoStart = 0, g_demoNext = 0;
const double          kDemoPeriodMs = 20.0;   // Steven 20260929：「得快到20ms一次」

// 毫秒時鐘用 QueryPerformanceCounter：GetTickCount 一格約 15.6 ms，排不出 20 ms。
double DemoNowMs()
{
    static LARGE_INTEGER f;
    static bool have = false;
    if (!have) { ::QueryPerformanceFrequency(&f); have = true; }
    LARGE_INTEGER c;
    ::QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000.0 / (double)f.QuadPart;
}

void DemoTick()
{
    const double now = DemoNowMs();
    if (now < g_demoNext) return;
    g_demoNext += kDemoPeriodMs;
    if (g_demoNext <= now) g_demoNext = now + kDemoPeriodMs;   // 落後一整拍以上就重設，不連發補拍
    ++g_demoTick;
    const unsigned ms = (unsigned)(now - g_demoStart);
    AnimateIo(g_demoIo, ms / 200u);
    IoSummary s;
    s.buildConfig = "DEMO 假資料";
    s.connected = true;
    s.pollCount = g_demoTick;
    s.keepaliveCalls = HostKeepaliveCalls();
    IoViewUpdate(g_demoIo, s);
    AnimateMotor(g_demoMot, ms / 200u, ms / 20u);
    MotorSummary m;
    m.buildConfig = "DEMO 假資料";
    m.monitorOpen = true;
    m.monitorAxes = (int)g_demoMot.size();
    m.pollCount = g_demoTick;
    m.keepaliveCalls = HostKeepaliveCalls();
    MotorViewUpdate(g_demoMot, m);
}

void ReopenIoDemo() { IoViewOpen(true); }
void ReopenMotorDemo() { MotorViewOpen(true); }

// 把一個視窗用 PrintWindow 畫進記憶體 DC，存成 24-bit BMP（--snapshot 用：螢幕鎖著也拍得到，給沒辦法看螢幕的人核對畫面）。
bool SaveWindowBmp(HWND h, const std::string& path)
{
    RECT rc;
    if (!h || !::GetWindowRect(h, &rc)) return false;
    const int w = rc.right - rc.left, hh = rc.bottom - rc.top;
    if (w <= 0 || hh <= 0) return false;
    HDC screen = ::GetDC(0);
    HDC mem = ::CreateCompatibleDC(screen);
    BITMAPINFO bi;
    std::memset(&bi, 0, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = hh;          // bottom-up，BMP 檔的原生方向
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = 0;
    HBITMAP bmp = ::CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, 0, 0);
    bool ok = false;
    if (bmp && bits) {
        HGDIOBJ old = ::SelectObject(mem, bmp);
        ::PrintWindow(h, mem, 0);
        ::GdiFlush();
        const int stride = ((w * 3 + 3) / 4) * 4;
        const DWORD imgSize = (DWORD)stride * (DWORD)hh;
        BITMAPFILEHEADER fh;
        std::memset(&fh, 0, sizeof(fh));
        fh.bfType = 0x4D42;
        fh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        fh.bfSize = fh.bfOffBits + imgSize;
        std::FILE* f = std::fopen(path.c_str(), "wb");
        if (f) {
            ok = std::fwrite(&fh, sizeof(fh), 1, f) == 1 &&
                 std::fwrite(&bi.bmiHeader, sizeof(BITMAPINFOHEADER), 1, f) == 1 &&
                 std::fwrite(bits, 1, imgSize, f) == imgSize;
            std::fclose(f);
        }
        ::SelectObject(mem, old);
    }
    if (bmp) ::DeleteObject(bmp);
    ::DeleteDC(mem);
    ::ReleaseDC(0, screen);
    return ok;
}

int ShowDemo(const char* ioCsv, const char* motCsv, const char* snapshotPrefix)
{
    int skipped = 0;
    if (ioCsv && std::strcmp(ioCsv, "-") != 0) {
        g_demoIo = LoadIoCsv(ioCsv, &skipped);
        if (g_demoIo.empty()) { std::printf("cannot read IO table '%s'\n", ioCsv); return 2; }
    } else {
        for (int i = 0; i < 40; ++i) {
            char a[32];
            std::snprintf(a, sizeof(a), i % 2 ? "SwDemo%02d" : "SnDemo%02d", i);
            g_demoIo.push_back(MakeIo(i + 1, a, i % 2 ? "Switch" : "Sensor", 3, 1, i % 2 ? 16 : 2, i, i % 8, 1, i == 7 ? 0 : 1,
                                      -1, -1, "nosource", ""));
        }
    }
    if (motCsv && std::strcmp(motCsv, "-") != 0) {
        g_demoMot = LoadMotorCsv(motCsv);
        if (g_demoMot.empty()) { std::printf("cannot read motor table '%s'\n", motCsv); return 2; }
    } else {
        g_demoMot = FakeMotorRows();
    }
    if (!IoViewOpen(true) || !MotorViewOpen(true)) { std::printf("window create failed (%lu)\n", ::GetLastError()); return 1; }
    HostRegisterHotkey(0x4E49, MOD_CONTROL | MOD_ALT, 'I', &ReopenIoDemo);
    HostRegisterHotkey(0x4E4D, MOD_CONTROL | MOD_ALT, 'M', &ReopenMotorDemo);
    std::printf("DEMO: two read-only native windows. Drag a title bar: the lamps keep changing = the keepalive works.\n"
                "      Ctrl+Alt+I / Ctrl+Alt+M reopen a closed window. Close both windows to exit.\n");
    const DWORD started = ::GetTickCount();
    g_demoStart = g_demoNext = DemoNowMs();
    // 計時器解析度調到 1 ms（只在展示程式；wb_serve 沒有呼叫 timeBeginPeriod）：預設 15.6 ms 一格，等 20 ms 實際會等到 31 ms。
    ::timeBeginPeriod(1);
    while (IoViewIsOpen() || MotorViewIsOpen()) {
        PumpThreadMessages(200, &DemoTick);   // 拖曳時 DemoTick 由保活計時器叫到
        DemoTick();
        const double left = g_demoNext - DemoNowMs();
        const DWORD wait = left <= 0 ? 0u : left >= kDemoPeriodMs ? (DWORD)kDemoPeriodMs : (DWORD)left;
        ::MsgWaitForMultipleObjects(0, 0, FALSE, wait, QS_ALLINPUT);
        if (snapshotPrefix && ::GetTickCount() - started > 1500) {
            const std::string pre = snapshotPrefix;
            const bool a = SaveWindowBmp((HWND)IoViewHwnd(), pre + "_io.bmp");
            const bool b = SaveWindowBmp((HWND)MotorViewHwnd(), pre + "_motor.bmp");
            std::printf("snapshot: %s_io.bmp %s, %s_motor.bmp %s\n", snapshotPrefix, a ? "saved" : "FAILED", snapshotPrefix, b ? "saved" : "FAILED");
            IoViewClose();
            MotorViewClose();
            PumpThreadMessages(200, 0);
            break;
        }
    }
    ::timeEndPeriod(1);
    HostUnregisterHotkey(0x4E49);
    HostUnregisterHotkey(0x4E4D);
    std::printf("windows closed: %u updates, %lu keepalive ticks during drags; IO update last %.2f ms max %.2f ms, motor last %.2f ms max %.2f ms\n",
                g_demoTick, HostKeepaliveCalls(), IoViewLastUpdateMs(), IoViewMaxUpdateMs(), MotorViewLastUpdateMs(), MotorViewMaxUpdateMs());
    return 0;
}

// ---- ctest：IO ----
int TestIo()
{
    std::printf("NativeIoView headless test (window created hidden)\n");
    Check(!IoViewIsOpen(), "not open before IoViewOpen");
    Check(IoViewOpen(false), "IoViewOpen(false) creates the window");
    Check(IoViewIsOpen(), "IoViewIsOpen after create");
    HWND h = (HWND)IoViewHwnd();
    Check(h != 0 && !::IsWindowVisible(h), "window exists and is NOT visible (headless)");
    Pump();
    Check(IoViewButtonCount() >= 1, "there is at least one Button child (the output-operation button)");
    Check(IoViewEnabledButtonCount() == 0, "no Button child is enabled (outputs read-only by construction)");

    std::vector<IoRow> rows = FakeIoRows();
    IoSummary sum;
    sum.connected = true;
    sum.buildConfig = "TEST";
    sum.pollCount = 42;
    IoViewUpdate(rows, sum);
    Pump();
    Check(IoViewListCount() == (int)rows.size(), "list shows every row (7)");
    Check(IoViewCellText(0, kColAlias) == L"SnMotorPower", "row 0 Alias via LVN_GETDISPINFO");
    Check(IoViewCellText(0, kColCode) == L"I2.30", "row 0 ioCode I2.30");
    Check(IoViewCellText(2, kColCode) == L"O16.0", "row 2 ioCode O16.0 (output)");
    Check(IoViewCellText(0, kColDir) == L"輸入", "row 0 direction = 輸入");
    Check(IoViewCellText(2, kColDir) == L"輸出", "row 2 direction = 輸出");
    Check(IoViewLedState(0) == 1, "input SnMotorPower LED on");
    Check(IoViewLedState(1) == 0, "InType=0 input SnEMG: raw 1 -> LED off");
    Check(IoViewLedState(2) == 1, "output SwTowerRed LED on (shows current output state)");
    Check(IoViewLedState(4) == -1, "disabled row LED null, not off");
    Check(IoViewLedState(5) == -1, "bad row LED null, not off");
    Check(IoViewLedState(6) == -1, "nosource row LED null, not off");
    Check(IoViewCellText(2, kColOutBtn) == L"停用", "output row: output-button column says 停用");
    Check(IoViewCellText(0, kColOutBtn) == L"", "input row: no output button");
    Check(IoViewCellText(4, kColIp) == L"16", "station (IP) column");
    Check(IoViewCellText(6, kColRaw) == L"—", "null raw shows a dash");
    Check(IoViewSummaryText().find(L"共 7 點") != std::wstring::npos, "summary counts 7 points");
    Check(IoViewSummaryText().find(L"poll #42") != std::wstring::npos, "summary shows monitor poll count");

    rows[3].raw = 1; rows[3].isOn = 1;
    IoViewUpdate(rows, sum);
    Pump();
    Check(IoViewLedState(3) == 1, "SwTowerGreen turns on after update");
    Check(IoViewRedrawnItems() >= 1, "only-changed-row redraw path was taken");

    IoViewSetFilter(kFilterIn);
    Pump();
    Check(IoViewListCount() == 4, "filter 只看輸入 -> 4 rows");
    IoViewSetFilter(kFilterOut);
    Pump();
    Check(IoViewListCount() == 3, "filter 只看輸出 -> 3 rows");
    IoViewSetFilter(kFilterUnknown);
    Pump();
    Check(IoViewListCount() == 3, "filter 只看 null -> 3 rows");
    IoViewSetFilter(kFilterAll);
    IoViewSetSearch(L"tower");
    Pump();
    Check(IoViewListCount() == 2, "search 'tower' (case-insensitive) -> 2 rows");
    IoViewSetSearch(L"");
    Pump();
    Check(IoViewListCount() == 7, "search cleared -> 7 rows");

    rows.pop_back();
    IoViewUpdate(rows, sum);
    Pump();
    Check(IoViewListCount() == 6, "table shrank -> list shrank");

    ::SendMessageW(h, WM_CLOSE, 0, 0);
    Pump();
    Check(!IoViewIsOpen() && !::IsWindow(h), "WM_CLOSE destroys the window");
    MSG m;
    Check(!::PeekMessageW(&m, 0, WM_QUIT, WM_QUIT, PM_NOREMOVE), "closing does not post WM_QUIT");
    IoViewUpdate(rows, sum);
    Check(true, "IoViewUpdate on a closed view is a no-op");
    Check(IoViewOpen(false), "reopen after close");
    IoViewUpdate(rows, sum);
    Pump();
    Check(IoViewListCount() == 6, "reopened view shows the data again");
    IoViewClose();
    Pump();
    Check(!IoViewIsOpen(), "IoViewClose");
    return Done();
}

// ---- ctest：MotorView ----
int TestMotor()
{
    std::printf("NativeMotorView headless test (window created hidden)\n");
    // 十顆燈的位元解碼（golden ScanMotorStatus；ChanMotorPoints.cpp "led" 同一套）
    MotorRow t;
    t.ledKnown = true;
    t.motionIO = 0x00000008ul; Check(MotorLedOn(t, kLedCw) && !MotorLedOn(t, kLedCcw), "LMT- (bit3) -> CW");
    t.motionIO = 0x00000004ul; Check(MotorLedOn(t, kLedCcw) && !MotorLedOn(t, kLedCw), "LMT+ (bit2) -> CCW");
    t.motionIO = 0x00000010ul; Check(MotorLedOn(t, kLedHome), "ORG (bit4) -> HOME");
    t.motionIO = 0x00000040ul; Check(MotorLedOn(t, kLedEmg), "EMG (bit6) -> EM Stop");
    t.motionIO = 0x00000002ul; Check(MotorLedOn(t, kLedAlarm), "ALM (bit1) -> Alarm");
    t.motionIO = 0; t.state = 3; Check(MotorLedOn(t, kLedAlarm), "ERROR_STOP state -> Alarm");
    t.state = 1;
    t.motionIO = 0x00010000ul; Check(MotorLedOn(t, kLedSoftCw), "SLMT_P (bit16) -> Soft CW");
    t.motionIO = 0x00020000ul; Check(MotorLedOn(t, kLedSoftCcw), "SLMT_N (bit17) -> Soft CCW");
    t.motionIO = 0x00004000ul; Check(MotorLedOn(t, kLedServo), "SVON (bit14) -> Servo On");
    t.motionIO = 0xFFFFFFFFul; Check(!MotorLedOn(t, kLedSAlarm) && !MotorLedOn(t, kLedInPos), "S Alarm / InPos never lit for 1203 (golden)");
    t.ledKnown = false; Check(!MotorLedOn(t, kLedServo), "no sample -> no lamp lit");

    Check(MotorViewOpen(false), "MotorViewOpen(false) creates the window");
    HWND h = (HWND)MotorViewHwnd();
    Check(h != 0 && !::IsWindowVisible(h), "window exists and is NOT visible (headless)");
    Pump();
    Check(MotorViewButtonCount() == 0, "the motor view has NO button at all (no motion command)");

    std::vector<MotorRow> rows = FakeMotorRows();
    MotorSummary sum;
    sum.buildConfig = "TEST";
    sum.monitorOpen = true;
    sum.monitorAxes = 4;
    sum.pollCount = 7;
    MotorViewUpdate(rows, sum);
    Pump();
    Check(MotorViewListCount() == 4, "default filter = golden MotorView rows (Motor Test visible) -> 4");
    Check(MotorViewCellText(0, kMColAlias) == L"MInArmX", "row 0 Alias");
    Check(MotorViewCellText(0, kMColCur) == L"12345", "row 0 目前位置");
    Check(MotorViewCellText(0, kMColTarget) == L"—", "row 0 目標位置 null -> dash (not 0)");
    Check(MotorViewCellText(0, kMColSpeed) == L"10000", "row 0 速度");
    Check(MotorViewCellText(0, kMColCan) == L"1" && MotorViewCellText(0, kMColM) == L"0", "Can / M columns");
    Check(MotorViewCellText(0, kMColLeds) == L"CW SVON", "LED text lists the lit lamps");
    Check(MotorViewCellText(0, kMColLamp0 + kLedCw) == L"1" && MotorViewCellText(0, kMColLamp0 + kLedServo) == L"1" &&
          MotorViewCellText(0, kMColLamp0 + kLedCcw) == L"0", "one column per lamp: CW / SVON lit, CCW dark");
    Check(MotorViewCellText(3, kMColLamp0 + kLedServo) == L"—", "no sample -> lamp column null (dash), not 0");
    Check(MotorViewCellText(0, kMColServo) == L"ON", "servo ON");
    Check(MotorViewCellText(1, kMColAlarm) == L"警報", "row 1 alarm");
    Check(MotorViewCellText(1, kMColLeds) == L"ALM", "row 1 ALM lamp (ALM bit / ERROR_STOP)");
    Check(MotorViewCellText(2, kMColCur) == L"—", "partial row: position not given -> dash");
    Check(MotorViewCellText(2, kMColHome) == L"2 失敗", "HomeFlag 2 -> 失敗");
    Check(MotorViewCellText(3, kMColLeds) == L"null", "no sample -> LED null, not dark");
    Check(MotorViewCellText(3, kMColServo) == L"—", "no sample -> servo dash");
    Check(MotorViewCellText(0, kMColStation) == L"0/0", "station/axis column");
    MotorViewSetFilter(kMotFilterAll);
    Pump();
    Check(MotorViewListCount() == 6, "filter 馬達表全部 -> 6");
    MotorViewSetFilter(kMotFilter1203);
    Pump();
    Check(MotorViewListCount() == 5, "filter 只看 PCI1203 -> 5");
    MotorViewSetFilter(kMotFilterAlarm);
    Pump();
    Check(MotorViewListCount() == 2, "filter 只看警報／歸零失敗 -> 2");
    MotorViewSetFilter(kMotFilterNull);
    Pump();
    Check(MotorViewListCount() == 3, "filter 只看沒有值的 -> 3");
    MotorViewSetFilter(kMotFilterAll);
    MotorViewSetSearch(L"inarm");
    Pump();
    Check(MotorViewListCount() == 3, "search 'inarm' -> 3");
    MotorViewSetSearch(L"M14");
    Pump();
    Check(MotorViewListCount() == 1, "search by No 'M14' -> 1");
    MotorViewSetSearch(L"");
    MotorViewSetFilter(kMotFilterGolden);
    Pump();

    rows[0].cur = 999; rows[0].motionIO = 0x00004000ul | 0x00000010ul;
    MotorViewUpdate(rows, sum);
    Pump();
    Check(MotorViewCellText(0, kMColCur) == L"999", "position follows the update");
    Check(MotorViewCellText(0, kMColLeds) == L"HOME SVON", "lamps follow the update");
    Check(MotorViewSummaryText().find(L"golden 會列 4") != std::wstring::npos, "summary counts golden rows");

    ::SendMessageW(h, WM_CLOSE, 0, 0);
    Pump();
    Check(!MotorViewIsOpen() && !::IsWindow(h), "WM_CLOSE destroys the window");
    MSG m;
    Check(!::PeekMessageW(&m, 0, WM_QUIT, WM_QUIT, PM_NOREMOVE), "closing does not post WM_QUIT");
    return Done();
}

// ---- ctest：效率與「只畫變了的格子」（20260929）----
// Steven 20260929：「IO與馬達的顯示必須是很有效率的, 得快到20ms一次」「這樣的閃爍是不被允許的」。
// 視窗不顯示（無顯示 ctest），所以「送上螢幕」那一段（BitBlt）不在量到的時間裡；畫進背景圖、比較、摘要都在。
unsigned g_rand = 12345u;
unsigned NextRand() { g_rand = g_rand * 1103515245u + 12345u; return (g_rand >> 16) & 0x7FFFu; }

// LED 圓點中心附近的一點（NativeGrid.cpp DrawCell：kGridLedText 的點在 left+6，直徑 ≥ 6）
bool LedPixel(HWND grid, int row, int col, bool centered, COLORREF* c)
{
    RECT rc;
    if (!GridCellRect(grid, row, col, &rc)) return false;
    const int y = rc.top + (rc.bottom - 1 - rc.top) / 2;
    const int x = centered ? rc.left + (rc.right - 1 - rc.left) / 2 : rc.left + 6 + 3;
    *c = GridBackPixel(grid, x, y);
    return *c != CLR_INVALID;
}

int TestPerf()
{
    std::printf("NativeGrid efficiency test (hidden windows)\n");
    // ---- IO：700 點（HT9050 的 IO 表是這個量級）----
    Check(IoViewOpen(false), "IO window created (hidden)");
    std::vector<IoRow> rows;
    for (int i = 0; i < 700; ++i) {
        char a[32];
        std::snprintf(a, sizeof(a), i % 2 ? "SwPerf%03d" : "SnPerf%03d", i);
        rows.push_back(MakeIo(i, a, i % 2 ? "Switch" : "Sensor", 3, 1, i % 2 ? 16 : 2, i % 64, i % 8, 1, 1, 0, 0, "good",
                              i % 2 ? "pci1203.do" : "pci1203.di"));
    }
    IoSummary sum;
    sum.connected = true;
    sum.buildConfig = "TEST";
    IoViewUpdate(rows, sum);
    Pump();
    HWND grid = (HWND)IoViewGridHwnd();
    const int vis = GridVisibleRows(grid);
    std::printf("       IO grid: %d rows, %d visible\n", IoViewListCount(), vis);
    Check(grid != 0 && vis > 5 && vis < 700, "IO grid shows part of the 700 rows");

    GridStats s0 = GridGetStats(grid);
    IoViewUpdate(rows, sum);
    GridStats s1 = GridGetStats(grid);
    Check(s1.cellsPainted == s0.cellsPainted && s1.fullRenders == s0.fullRenders, "no value changed -> 0 cells repainted, no full repaint");

    COLORREF px = 0;
    Check(LedPixel(grid, 1, kColLed, false, &px) && px == RGB(60, 70, 60), "back buffer: row 1 LED is the OFF colour");
    rows[1].raw = 1; rows[1].isOn = 1;
    IoViewUpdate(rows, sum);
    GridStats s2 = GridGetStats(grid);
    std::printf("       one output toggled: %lu cells repainted\n", s2.cellsPainted - s1.cellsPainted);
    Check(s2.cellsPainted - s1.cellsPainted == 2, "one point toggles -> exactly 2 cells repainted (LED, Raw)");
    Check(s2.fullRenders == s1.fullRenders, "one point toggles -> no full repaint");
    Check(LedPixel(grid, 1, kColLed, false, &px) && px == RGB(255, 70, 40), "back buffer: row 1 (output) LED is now the output-ON colour");
    Check(IoViewLedState(1) == 1, "row 1 reads ON");

    rows[690].raw = 1; rows[690].isOn = 1;   // 看不到的列
    IoViewUpdate(rows, sum);
    GridStats s3 = GridGetStats(grid);
    Check(s3.cellsPainted == s2.cellsPainted && s3.fullRenders == s2.fullRenders, "an off-screen row changes -> 0 cells repainted");

    double total = 0, worst = 0;
    const int kRuns = 1000;
    unsigned long c0 = GridGetStats(grid).cellsPainted;
    for (int k = 0; k < kRuns; ++k) {
        for (int j = 0; j < 14; ++j) {   // 每一拍約 2% 的點變化
            IoRow& r = rows[NextRand() % rows.size()];
            r.raw = 1 - (r.raw < 0 ? 0 : r.raw);
            r.isOn = r.raw;
        }
        IoViewUpdate(rows, sum);
        total += IoViewLastUpdateMs();
        if (IoViewLastUpdateMs() > worst) worst = IoViewLastUpdateMs();
    }
    const double avgIo = total / kRuns;
    std::printf("       IO: %d updates of 700 points (~14 change each): avg %.3f ms, worst %.3f ms, %lu cells repainted in all\n",
                kRuns, avgIo, worst, GridGetStats(grid).cellsPainted - c0);
    Check(avgIo < 2.0, "IO: average update < 2 ms (budget 20 ms)");
    IoViewClose();
    Pump();

    // ---- MotorView：60 軸，每一拍每一軸的位置都變 ----
    Check(MotorViewOpen(false), "motor window created (hidden)");
    std::vector<MotorRow> mot;
    for (int i = 0; i < 60; ++i) {
        char a[32], no[8];
        std::snprintf(a, sizeof(a), "MPerf%02d", i);
        std::snprintf(no, sizeof(no), "M%02d", i);
        MotorRow m = MakeMotor(i, a, no, i, true, i, "PCI1203");
        m.hasCur = true; m.cur = i * 100; m.hasTarget = true; m.target = i * 100;
        m.servoOn = 1; m.alarm = 0; m.inPos = 1; m.busy = 0; m.homeFlag = 1; m.ledKnown = true; m.motionIO = 0x00004000ul;
        m.quality = "good"; m.source = "pci1203-monitor";
        mot.push_back(m);
    }
    MotorSummary msum;
    msum.buildConfig = "TEST";
    MotorViewUpdate(mot, msum);
    Pump();
    HWND mgrid = (HWND)MotorViewGridHwnd();
    const int mvis = GridVisibleRows(mgrid);
    const int shown = mvis < 60 ? mvis : 60;
    std::printf("       motor grid: %d rows, %d visible\n", MotorViewListCount(), mvis);
    Check(mgrid != 0 && mvis > 3, "motor grid has visible rows");
    GridStats m0 = GridGetStats(mgrid);
    for (std::size_t i = 0; i < mot.size(); ++i) mot[i].cur += 7;
    MotorViewUpdate(mot, msum);
    GridStats m1 = GridGetStats(mgrid);
    std::printf("       every axis moved: %lu cells repainted (visible rows %d)\n", m1.cellsPainted - m0.cellsPainted, shown);
    Check((int)(m1.cellsPainted - m0.cellsPainted) == shown, "every axis's position changes -> exactly one cell per visible row");
    Check(MotorViewCellText(0, kMColCur) == L"7", "row 0 position follows");
    Check(LedPixel(mgrid, 0, kMColLamp0 + kLedServo, true, &px) && px == RGB(0, 210, 60), "back buffer: SVON lamp is green");
    mot[0].motionIO = 0;
    MotorViewUpdate(mot, msum);
    Check(LedPixel(mgrid, 0, kMColLamp0 + kLedServo, true, &px) && px == RGB(60, 70, 60), "back buffer: SVON lamp goes dark after the update");

    total = 0; worst = 0;
    double flushTotal = 0;
    const unsigned long mc0 = GridGetStats(mgrid).cellsPainted;
    for (int k = 0; k < kRuns; ++k) {
        for (std::size_t i = 0; i < mot.size(); ++i) { mot[i].cur += 13; mot[i].target = mot[i].cur + 500; }
        MotorViewUpdate(mot, msum);
        total += MotorViewLastUpdateMs();
        flushTotal += GridGetStats(mgrid).lastFlushMs;
        if (MotorViewLastUpdateMs() > worst) worst = MotorViewLastUpdateMs();
    }
    const double avgMot = total / kRuns;
    std::printf("       motor: %d updates, all 60 axes move each time: avg %.3f ms (grid flush avg %.3f ms), worst %.3f ms, %lu cells repainted in all\n",
                kRuns, avgMot, flushTotal / kRuns, worst, GridGetStats(mgrid).cellsPainted - mc0);
    Check(avgMot < 3.0, "motor: average update < 3 ms (budget 20 ms)");
    MotorViewClose();
    Pump();

    // ---- 摘要標籤：字一樣就不重畫 ----
    WNDCLASSEXW wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &::DefWindowProcW;
    wc.hInstance = ::GetModuleHandleW(0);
    wc.lpszClassName = L"W906NativeTestLabelHost";
    ::RegisterClassExW(&wc);
    HWND host = ::CreateWindowExW(0, wc.lpszClassName, L"host", WS_OVERLAPPED, 0, 0, 300, 100, 0, 0, wc.hInstance, 0);
    HWND lbl = LabelCreate(host, 1, L"a");
    Check(lbl != 0, "label created");
    Check(!LabelSetText(lbl, L"a"), "label: same text -> nothing to do");
    Check(LabelSetText(lbl, L"b"), "label: new text -> redrawn");
    wchar_t b[8];
    ::GetWindowTextW(lbl, b, 8);
    Check(std::wstring(b) == L"b", "label: GetWindowText reads the new text");
    ::DestroyWindow(host);
    return Done();
}

// ---- ctest：拖曳保活 ----
HWND  g_helper = 0;
int   g_ka1 = 0, g_ka2 = 0;
bool  g_nestOnce = false;
bool  g_ka1Reentered = false;
int   g_depthSeen = 0;

// 輔助視窗：收到 WM_APP 就「自己跑一個訊息迴圈 wParam 毫秒」，把所有訊息（含原生視窗的 WM_TIMER）分派出去——
// 形狀同 DefWindowProc 的拖曳／改大小迴圈。
LRESULT CALLBACK HelperProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_APP) {
        const DWORD t0 = ::GetTickCount();
        while (::GetTickCount() - t0 < (DWORD)wp) {
            MSG m;
            while (::PeekMessageW(&m, 0, 0, 0, PM_REMOVE)) { ::TranslateMessage(&m); ::DispatchMessageW(&m); }
            ::Sleep(2);
        }
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wp, lp);
}

void Ka2() { ++g_ka2; }

void Ka1()
{
    ++g_ka1;
    if (g_depthSeen < HostPumpDepth()) g_depthSeen = HostPumpDepth();
    if (g_nestOnce) {
        // 模擬「保活跑 PumpTick → 跑到阻塞等待框 → 那裡再泵一次 → 又被拖曳」：內層有自己的 keepalive，外層不可被重入。
        g_nestOnce = false;
        const int before = g_ka1;
        ::PostMessageW(g_helper, WM_APP, 300, 0);
        PumpThreadMessages(20, &Ka2);
        g_ka1Reentered = (g_ka1 != before);
    }
}

int TestKeepalive()
{
    std::printf("NativeHost keepalive test (emulated Windows modal loop)\n");
    WNDCLASSEXW wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &HelperProc;
    wc.hInstance = ::GetModuleHandleW(0);
    wc.lpszClassName = L"W906NativeTestHelper";
    ::RegisterClassExW(&wc);
    g_helper = ::CreateWindowExW(0, wc.lpszClassName, L"helper", WS_OVERLAPPED, 0, 0, 10, 10, 0, 0, wc.hInstance, 0);
    Check(g_helper != 0, "helper window created");
    Check(IoViewOpen(false) && MotorViewOpen(false), "two native windows open (two keepalive timers)");

    // (1) 平常泵：保活計時器被我們自己的泵丟掉，keepalive 一次都不叫
    {
        const DWORD t0 = ::GetTickCount();
        while (::GetTickCount() - t0 < 300) { PumpThreadMessages(100, &Ka1); ::Sleep(5); }
    }
    Check(g_ka1 == 0, "normal pumping for 300 ms: keepalive NOT called (main loop is running)");
    Check(HostTimerDropped() > 0, "the keepalive timer was seen and dropped by our own pump");

    // (2) 我們的泵分派的訊息裡面跑起「內部迴圈」400 ms：keepalive 要被叫到（≈ 每 50 ms 一次）
    g_ka1 = 0;
    ::PostMessageW(g_helper, WM_APP, 400, 0);
    PumpThreadMessages(10, &Ka1);
    std::printf("       keepalive calls during a 400 ms nested loop: %d\n", g_ka1);
    Check(g_ka1 >= 4, "nested modal loop inside our dispatch: keepalive called (>= 4 in 400 ms)");
    Check(g_ka1 <= 12, "two windows' timers do not double the rate (<= 12 in 400 ms, min gap 40 ms)");
    Check(g_depthSeen == 1, "keepalive runs at pump depth 1");

    // (3) 不在我們的泵裡的內部迴圈：不插手
    g_ka1 = 0;
    ::SendMessageW(g_helper, WM_APP, 300, 0);
    Check(g_ka1 == 0, "a modal loop NOT inside our pump: keepalive not called");

    // (4) 巢狀：外層 keepalive 裡再泵、又遇到內部迴圈 —— 內層 keepalive 被叫，外層不重入
    g_ka1 = 0; g_ka2 = 0; g_nestOnce = true; g_ka1Reentered = false;
    ::PostMessageW(g_helper, WM_APP, 400, 0);
    PumpThreadMessages(10, &Ka1);
    std::printf("       nested: outer %d, inner %d\n", g_ka1, g_ka2);
    Check(g_ka2 >= 3, "inner pump's keepalive runs inside the outer keepalive");
    Check(!g_ka1Reentered, "outer keepalive is NOT re-entered while it runs");
    Check(HostPumpDepth() == 0, "pump depth back to 0");

    // (5) 關掉視窗後計時器也停
    IoViewClose();
    MotorViewClose();
    PumpThreadMessages(100, 0);
    g_ka1 = 0;
    ::PostMessageW(g_helper, WM_APP, 200, 0);
    PumpThreadMessages(10, &Ka1);
    Check(g_ka1 == 0, "no native window -> no keepalive timer -> no keepalive");
    ::DestroyWindow(g_helper);
    return Done();
}

// ---- 手動：真的 Windows 鍵盤移動迴圈 ----
int g_realKa = 0;
void RealKa()
{
    ++g_realKa;
    if (g_realKa == 5) {
        // 結束移動迴圈：送 Esc（移動迴圈自己在讀鍵盤訊息）
        HWND h = (HWND)IoViewHwnd();
        ::PostMessageW(h, WM_KEYDOWN, VK_ESCAPE, 0);
        ::PostMessageW(h, WM_KEYUP, VK_ESCAPE, 0);
    }
    if (g_realKa > 200) ::ReleaseCapture();   // 保險
}

int TestModalMove()
{
    std::printf("real Windows move loop (WM_SYSCOMMAND SC_MOVE): the window is shown briefly\n");
    Check(IoViewOpen(true), "IO window shown");
    Pump();
    HWND h = (HWND)IoViewHwnd();
    ::PostMessageW(h, WM_SYSCOMMAND, SC_MOVE, 0);   // 由我們的泵分派 → DefWindowProc 進入鍵盤移動迴圈
    const DWORD t0 = ::GetTickCount();
    while (::GetTickCount() - t0 < 3000 && g_realKa < 5) PumpThreadMessages(50, &RealKa);
    std::printf("       keepalive calls inside the real move loop: %d (entered %lu)\n", g_realKa, HostSizeMoveEntered());
    Check(HostSizeMoveEntered() >= 1, "WM_ENTERSIZEMOVE seen (Windows' own move loop ran)");
    Check(g_realKa >= 5, "keepalive ran inside Windows' own move loop");
    IoViewClose();
    Pump();
    return Done();
}

}  // namespace

int main(int argc, char** argv)
{
    const char* mode = argc >= 2 ? argv[1] : "--io";
    if (std::strcmp(mode, "--show") == 0) return ShowDemo(argc >= 3 ? argv[2] : 0, argc >= 4 ? argv[3] : 0, 0);
    if (std::strcmp(mode, "--snapshot") == 0 && argc >= 3)   // --snapshot <out prefix> [IO_Table.csv|-] [Mot_Table.csv|-]
        return ShowDemo(argc >= 4 ? argv[3] : 0, argc >= 5 ? argv[4] : 0, argv[2]);
    if (std::strcmp(mode, "--io") == 0) return TestIo();
    if (std::strcmp(mode, "--motor") == 0) return TestMotor();
    if (std::strcmp(mode, "--perf") == 0) return TestPerf();
    if (std::strcmp(mode, "--keepalive") == 0) return TestKeepalive();
    if (std::strcmp(mode, "--modal-move") == 0) return TestModalMove();
    std::printf("usage: test_native_forms --io | --motor | --perf | --keepalive | --modal-move | --show [IO_Table.csv|-] [Mot_Table.csv|-]\n"
                "       test_native_forms --snapshot <out prefix> [IO_Table.csv|-] [Mot_Table.csv|-]   (PrintWindow -> <prefix>_io.bmp / _motor.bmp)\n");
    return 2;
}
