// =============================================================================
//  tests/test_c14_bindisp.cpp -- ST02-C14 Bin Display: the C++ bring-up (golden 906_0625_Steven).
//  AI(W906-ST02-C14) 20261002 (St02-E helper).  Suite: C14_BinDisp (both configs: SIM and -DW906_NO_SOFT_SIMULTE).
//
//  CONTAINMENT FIRST (st02_test_containment.h): exit 2 before any Handler code unless every log root and the Gerneral.ini
//  path are ctest's scratch.  The boot writes only the scratch Gerneral.ini (CheckAndReadIniDataGeneral) and, in SHIP, the
//  BinDisplayLog under <machine_log_scratch>\BinDisplayLog (slBinDispLog->Path, banner (6)).  NO COM PORT: both TComm are
//  in SIM mode (asserted), and the SHIP GetCOMPortStatus probe is answered from our own ports (real probes asserted 0).
//    0. the ctest guard sees the environment.
//    1. NUMBER_PANEL_TYPE 0 = what main does today: the boot creates nothing, the Timer1 slot never fires.
//    2. type 3 boot: TDataModule3 + TMyBinDispHT9046 from the real InstallColorBinDisplay, SIM mode, the log seam, P1 alias.
//    3. type 3 frames from a known tray state (DoShowBinDigital -> WriteTargetBin x36 -> ProcessStopStart(true)), the
//       panel bus answered by a fake "new module" responder: ReadVersion x12, (SHIP) WriteColor x12, WriteBin x12, then the
//       rotation of the one unit with two bins; every frame byte-exact with an independent Modbus-ASCII LRC.
//    4. ChangeBinDispStatus paints the same panels ShowBinSel writes (P1) from the acked state.
//    5. the BinSel FormShow pause: ProcessStopStart(false) when idle, not while running; DoShowBinDigital resumes.
//    6. type 4 (TFT): ReadVersion_TFT x12, DoOnce (8 setup frames per unit, the first byte-exact), DoCycle WriteBin_TFT x12
//       byte-exact ("Loader" / "Empty" / "Color" / "000" / "002" / "  E" / "---"), the count passes; then (ST02-S22, V912 D kept,
//       RULINGS_20261003 #1; 906 went idle here) light rounds keep cycling; E1 / E3 mirror the bin / colour shown.
//       6a. D: a new target set mid-round goes straight to DoOnce; the BinSel pause still stops the bus.
//       6b. A: a changed count jumps the queue (at most 8 in a row), an unchanged one is not re-sent, a count changed after a
//           round wakes DoCycle from case 1 (V912 behaviour), one still dirty at round end is not dropped (A10).
//       6c. E2: a fresh controller shows its initial bins once the bin font (DoOnce type 2) is acked.
//    7. the pump cadence on a fake clock: 50 ms steps -> one Timer1 call per 200 ms; 500 ms steps (the PumpTick beat) ->
//       one call per step; an Interval change restarts the deadline (VCL); FormClose copies InitialOK and stops the slot.
//    8. no COM port: both ports SIM, zero real GetCOMPortStatus probes.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "database.h"
#include "common.h"
#include "forms/fShowBinSelect.h"
#include "BinDisplay/MyBinDisp.h"
#include "BinDisplay/BinDispBringUp_St02.h"
#include "Public/MyStringList.h"     // slBinDispLog->Path
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace ht9045 {
void W906_St02TimersReset();
void W906_St02TimersCountsBD(unsigned long* bd);
void W906_St02BinDispSlotAt(unsigned long now);
}

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

// golden Graphics.hpp values (cShowBinSelect.cpp:117-134 defines them TU-locally; ColorMap :2263)
static const int kClRed = 0x000000FF, kClGreen = 0x00008000, kClGray = 0x00808080;

static unsigned long g_now = 1000;
static unsigned long FakeClock() { return g_now; }

static unsigned long BdCalls()
{
    unsigned long n = 0;
    ht9045::W906_St02TimersCountsBD(&n);
    return n;
}

// ---- an independent Modbus-ASCII frame builder (the panel protocol, MyBinDisp.cpp comment table :648-656) ----
static int Nib(char c) { return (c >= '0' && c <= '9') ? c - '0' : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : 0; }
static std::string Lrc12(const std::string& body12)   // two's complement of the sum of the 6 hex pairs
{
    unsigned sum = 0;
    for (int i = 0; i < 12; i += 2) sum += (unsigned)(Nib(body12[i]) * 16 + Nib(body12[i + 1]));
    const unsigned lrc = (0x100 - (sum & 0xFF)) & 0xFF;
    char b[3];
    std::snprintf(b, sizeof(b), "%02X", lrc);
    return b;
}
static std::string PanelAddr(int addr)   // golden: hex(addr+32) for addr<10, hex(addr+38) for addr>=10
{
    char b[3];
    std::snprintf(b, sizeof(b), "%02X", addr >= 10 ? addr + 38 : addr + 32);
    return b;
}
static std::string LegacyFrame(int addr, const char* mid8)   // ":" AA mid(8) LRC CR LF ; mid = "03008000" + "01" etc.
{
    const std::string body = PanelAddr(addr) + mid8;
    return ":" + body + Lrc12(body) + "\r\n";
}
static std::string ReadVersionFrame(int addr) { return LegacyFrame(addr, "0300800001"); }
static std::string WriteBinFrame(int addr, int cmd, int value)
{
    char m[16];
    std::snprintf(m, sizeof(m), "06008%d00%02d", cmd, value);
    return LegacyFrame(addr, m);
}
static std::string WriteColorFrame(int addr, int value)
{
    char m[16];
    std::snprintf(m, sizeof(m), "06008200%02d", value);
    return LegacyFrame(addr, m);
}

// ---- the fake legacy bus: every unit is a "new module" (iVersion 2) that echoes as golden's checks expect ----------------
static size_t g_txOff = 0;
static std::vector<std::string> g_frames3;
static std::vector<std::string> g_mag3;   // golden 906 :1787-1797 Magazine-TFT restore bursts seen on the type-3 bus (never answered)
static bool HexAscii(char ch) { return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F'); }
static void Respond3()
{
    Spcomm::TComm* c = DataModule3->BinDisp;
    const std::vector<char>& tx = c->SimTxBuffer();
    if (g_txOff > tx.size()) g_txOff = 0;        // AI(W906-ST02-C14b) 20261003 (St02-E, gate b52a fix): a StartComm cleared the capture (ship case 1 re-opens the port)
    while (true)
    {
        // golden 906_0625 MyBinDisp.cpp:1787-1797: DoStartSetBin (iMagazineStatus==0, iSendCMD<=3, tMagTimer.Off()) writes
        // MagazineWriteBinFont_TFT(i, 6, false, true) for i=1..MAX_MGZ_TRAY -- 14 BINARY 20-byte TFT frames (byte 1 = layer
        // 0x01..0x0e) -- on the same CommBin even with NUMBER_PANEL_TYPE 3 and no magazine.  ":" + non-hex = such a frame:
        // keep it apart (a legacy panel does not answer it) so the legacy sequence below stays golden's.
        if (g_txOff + 1 < tx.size() && tx[g_txOff] == ':' && !HexAscii(tx[g_txOff + 1]))
        {
            if (g_txOff + 20 > tx.size()) return;
            g_mag3.push_back(std::string(&tx[g_txOff], 20));
            g_txOff += 20;
            continue;
        }
        size_t end = std::string::npos;
        for (size_t i = g_txOff; i + 1 < tx.size(); ++i)
            if (tx[i] == '\r' && tx[i + 1] == '\n') { end = i + 2; break; }
        if (end == std::string::npos) return;
        const std::string f(&tx[g_txOff], end - g_txOff);
        g_txOff = end;
        g_frames3.push_back(f);
        std::string reply;
        if (f.size() >= 15 && f.compare(3, 2, "03") == 0)
            reply = ":" + f.substr(1, 2) + "03020002" + "00\r\n";                       // DoStartGetStatus: ":AA03020002" = new module
        else if (f.size() >= 15 && f.compare(3, 2, "06") == 0)
            reply = ":" + f.substr(1, 2) + "0602" + "0" + f.substr(8, 1) + f.substr(11, 2) + "00\r\n";   // ":AA0602 0C VV"
        if (!reply.empty())
            c->SimInjectReceive(reply.data(), (Spcomm::Word)reply.size());
    }
}

// ---- the fake TFT bus ---------------------------------------------------------------------------------------------------
static std::vector<std::string> g_framesTFT;
static unsigned char Lrc16(const unsigned char* p)   // two's complement of bytes 1..16
{
    unsigned sum = 0;
    for (int i = 1; i <= 16; ++i) sum += p[i];
    return (unsigned char)((0x100 - (sum & 0xFF)) & 0xFF);
}
static void RespondTFT()
{
    Spcomm::TComm* c = DataModule3->BinDisp;
    const std::vector<char>& tx = c->SimTxBuffer();
    if (g_txOff > tx.size()) g_txOff = 0;        // AI(W906-ST02-C14b) 20261003 (St02-E, gate b52a fix): as Respond3
    while (g_txOff + 20 <= tx.size())
    {
        const unsigned char* f = (const unsigned char*)&tx[g_txOff];
        g_framesTFT.push_back(std::string((const char*)f, 20));
        g_txOff += 20;
        std::vector<unsigned char> r;
        if (f[4] == 0x08 && f[5] == 0x00)        // ReadVersion_TFT -> "3A<A>0D080030313030303030303030" then "32" = version 2
        {
            const unsigned char v[] = { 0x3A, f[1], 0x0D, 0x08, 0x00, 0x30, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x00, 0x0D, 0x0A };
            r.assign(v, v + sizeof(v));
        }
        else                                      // echo "3A <A> 0D" + function / item / data (the prefixes golden checks)
        {
            r.push_back(0x3A); r.push_back(f[1]); r.push_back(0x0D);
            for (int i = 4; i <= 16; ++i) r.push_back(f[i]);
            r.push_back(0x00); r.push_back(0x0D); r.push_back(0x0A);
        }
        c->SimInjectReceive(&r[0], (Spcomm::Word)r.size());
    }
}

static std::string TftTextFrame(int unitAddrByte, int item, const char* text)   // command_TFT_Input (MyBinDisp.cpp:798-848)
{
    unsigned char f[20];
    f[0] = 0x3A; f[1] = (unsigned char)unitAddrByte; f[2] = 0x00; f[3] = 0x0D; f[4] = 0x00; f[5] = 0x03; f[6] = 0x00; f[7] = (unsigned char)item;
    for (int i = 0; i < 9; ++i) f[8 + i] = 0x01;
    const size_t n = std::strlen(text);
    if (n == 0) f[8] = 0x00;
    for (size_t i = 0; i < n && i < 9; ++i) f[8 + i] = (unsigned char)text[i];
    f[17] = Lrc16(f); f[18] = 0x0D; f[19] = 0x0A;
    return std::string((const char*)f, 20);
}
// ---- ST02-S22 (V912 D / E / A), AI(W906-ST02-S22) 20261003 (St02-E): helpers for sections 6 / 6a / 6b / 6c ------------------
static bool TftIs(const std::string& f, int fn, int item, int addr = -1)   // 20-byte request: 3A addr 00 0D 00 fn 00 item data*9 LRC CR LF
{ return f.size()==20 && (unsigned char)f[4]==0x00 && (unsigned char)f[5]==fn && (unsigned char)f[6]==0x00 && (unsigned char)f[7]==item && (addr<0 || (unsigned char)f[1]==addr); }
static std::string TftText(const std::string& f)                          // data bytes 8..16 up to the 0x01 pad / 0x00
{ std::string s; for (int i=8;i<17;++i){ unsigned char c=(unsigned char)f[i]; if(c==0x00||c==0x01) break; s+=(char)c; } return s; }
static void TickTFT(int n) { for (int i=0;i<n;++i){ g_now+=200; ht9045::W906_St02BinDispSlotAt(g_now); RespondTFT(); } }   // one Timer1 call per step (Interval 200, as section 6)
static bool TickUntilNewFrame(int maxTicks, size_t& idx)                  // idx = index of the new frame; false = none came (never index past the end)
{ const size_t n=g_framesTFT.size(); for(int i=0;i<maxTicks && g_framesTFT.size()==n;++i) TickTFT(1); idx=n; return g_framesTFT.size()>n; }
static bool TickUntilSent(int maxTicks, int fn, int item, int addr)       // one call at a time until the frame just sent is (fn, item, addr)
{ for (int i=0;i<maxTicks;++i){ const size_t n=g_framesTFT.size(); TickTFT(1); if(g_framesTFT.size()>n && TftIs(g_framesTFT.back(),fn,item,addr)) return true; } return false; }
static std::vector<std::string> NextFrames(int count)                     // the next `count` frames, each within 10 calls (fewer = the bus went quiet)
{ std::vector<std::string> v; for (int k=0;k<count;++k){ size_t i=0; if(!TickUntilNewFrame(10,i)) break; v.push_back(g_framesTFT[i]); } return v; }
static bool PriorityRun8(const std::vector<std::string>& v, int base)    // v[0..7] = the count inserts of units Auto1..Fix5 (0x23..0x2A) in unit order
{
    if (v.size() < 8) return false;
    for (int k=0;k<8;++k)
    {
        char t[8]; std::snprintf(t, sizeof(t), "%d", base+eBinDispAuto1+k);
        if (!TftIs(v[k],3,4,0x23+k) || (k>0 && TftText(v[k])!=t)) return false;   // Auto1's text follows its rotating bin (0 / E = no text)
    }
    return true;
}

static void KnownTrayState()
{
    iTestBinCount = 3;
    for (int k = 0; k < ePosTrayCount; ++k) { iTo3PosUnload[k] = 0; iTo3Unload[k] = (k < e3TrayCount) ? k : 0; }
    iTo3PosUnload[10] = 1;                       // -> eBinDisp 3 = Auto1
    iTo3PosUnload[11] = 2;                       // -> eBinDisp 4 = Auto2
    Prod.iT6PosCate[0] = 10; Prod.iT6PosCate[1] = 10; Prod.iT6PosCate[2] = 11;   // bins 0, 1 -> Auto1; bin 2 -> Auto2
    Prod.iIfErrorT6 = 2;                         // iTo3Unload[2] = 2 -> eBinDisp 5 = Auto3 shows E (104)
    for (int j = 0; j < eTrayCount; ++j) { Prod.iIsFailT6[j] = 0; Prod.bLinkTo6Tray[j] = false; }
    Prod.iIsFailT6[0] = 1;                       // Auto1 red
    Prod.iIsFailT6[2] = 1;                       // Auto3 red; Auto2 green; the unused ones orange
    IniConfig.bAutoTrayLink = false;
    IniConfig.bSPILFunction = false;
    IniConfig.bP66AutoChangingFlashWarn = false;
    TestIF_File.bEnableQASampling = false;
    TestIF_File.iMagDisplayOrder = 0;
    CosFunction.bLoaderTrayToAuto1 = false;
    fShowBinSelect->bUpdateBinDigital = true;
    fShowBinSelect->DoShowBinDigital();          // types 3 / 4: WriteTargetBin x36 + ProcessStopStart(true) (GATE D7 / D8 lifted)
}

static void KnownTrayState9()                   // ST02-S22, section 6b only: Auto1-3 / Fix1-6 each get a bin, so 9 units have counts
{
    iTestBinCount = 9;
    for (int k = 0; k < ePosTrayCount; ++k) { iTo3PosUnload[k] = 0; iTo3Unload[k] = (k < e3TrayCount) ? k : 0; }
    for (int b = 0; b < 9; ++b) { iTo3PosUnload[10 + b] = b + 1; Prod.iT6PosCate[b] = 10 + b; }   // bin b -> Data b+1 -> eBinDisp b+3
    Prod.iIfErrorT6 = 0;                         // iTo3Unload[0] = 0 -> Auto1 also shows E (104): Auto1 = {0, E} keeps rotating
    for (int j = 0; j < eTrayCount; ++j) { Prod.iIsFailT6[j] = 0; Prod.bLinkTo6Tray[j] = false; }
    IniConfig.bAutoTrayLink = false;
    IniConfig.bSPILFunction = false;
    IniConfig.bP66AutoChangingFlashWarn = false;
    TestIF_File.bEnableQASampling = false;
    TestIF_File.iMagDisplayOrder = 0;
    CosFunction.bLoaderTrayToAuto1 = false;
    fShowBinSelect->bUpdateBinDigital = true;
    fShowBinSelect->DoShowBinDigital();          // WriteTargetBin x36 + ProcessStopStart(true)
}

static bool Contains(const AnsiString& s, const char* sub) { return std::string(s.c_str()).find(sub) != std::string::npos; }

// ---- ST02-C14b (NB2 R168), AI(W906-ST02-C14b) 20261003 (St02-E) -----------------------------------------------------------
extern bool (*W906_FormFShowHook)(const char* goldenForm);                     // csystem.cpp (csystem.h:421): the page table seam
static bool g_binSelOpen = false;
static bool FakeFShow(const char* goldenForm) { return std::strcmp(goldenForm, "fBinSel") == 0 && g_binSelOpen; }
static std::vector<std::string> ReadLines(const std::string& path)
{
    std::vector<std::string> out;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (f == NULL) return out;
    std::string cur;
    for (int c = std::fgetc(f); c != EOF; c = std::fgetc(f))
    {
        if (c == '\n') { out.push_back(cur); cur.clear(); }
        else if (c != '\r') cur += (char)c;
    }
    out.push_back(cur);
    std::fclose(f);
    return out;
}
static int FindLine(const std::vector<std::string>& L, const char* sub)        // the only line holding sub (-1: none or several)
{
    int at = -1;
    for (size_t i = 0; i < L.size(); ++i)
        if (L[i].find(sub) != std::string::npos) { if (at >= 0) return -1; at = (int)i; }
    return at;
}
static bool BeforeComment(const std::string& line, const char* sub)            // sub is code: before any // on that line
{
    const size_t a = line.find(sub), c = line.find("//");
    return a != std::string::npos && (c == std::string::npos || a < c);
}
// ---- W-191 MR-3, AI(W906-W191) 20261009 (St02-E): the TFT writers are protected virtuals of TMyBinDispHT9046 (BinDisplay/MyBinDisp.h:496-529);
//   a pointer to member named through a derived class is the standard way to call them on the controller the boot built.
struct W191TftAccess : TMyBinDispHT9046
{
    typedef void (TMyBinDispHT9046::*Fn2)(int, int);
    typedef void (TMyBinDispHT9046::*Fn3)(int, int, int);
    static Fn2 WriteBin()       { return &W191TftAccess::WriteBin_TFT; }
    static Fn2 WriteBinWord()   { return &W191TftAccess::WriteBinWord_TFT; }
    static Fn2 WriteEA()        { return &W191TftAccess::WriteEA_TFT; }
    static Fn3 WriteCount()     { return &W191TftAccess::WriteCount_TFT; }
    static Fn3 SetFontBin()     { return &W191TftAccess::SetFontBin_TFT; }
    static Fn3 SetFontBinWord() { return &W191TftAccess::SetFontBinWord_TFT; }
    static Fn3 SetFontEA()      { return &W191TftAccess::SetFontEA_TFT; }
    static Fn3 SetFontCount()   { return &W191TftAccess::SetFontCount_TFT; }
};
static size_t Tx() { return DataModule3->BinDisp->SimTxBuffer().size(); }
static void Ticks(int n) { for (int i = 0; i < n; ++i) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); Respond3(); } }

int main(int argc, char** argv)
{
    (void)argc; (void)argv;
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("C14_BinDisp\n");
    if (!W906TestInsideCtestRoots("C14_BinDisp"))
        return 2;
#ifdef SOFT_SIMULTE
    const bool kSim = true;
    std::printf(" config: SIM (SOFT_SIMULTE)\n");
#else
    const bool kSim = false;
    std::printf(" config: SHIP (W906_NO_SOFT_SIMULTE)\n");
#endif
    W906_BinDispSetRxClock_St02(&FakeClock);

    std::printf(" 0. the ctest guard\n");
    CHECK(W906_BinDispUnderCtest_St02());

    // common machine shape (sim_9378-like, 12 panels on one bus)
    AUTO_EMPTY_COLOR = 1; AUTO3_IS_MAGAZINE = 0; MAGAZINE_BIN_DISP_TYPE = 0;
    USE_Scanner_AOI_Inspection = 0; USE_OUT_SORT_ARM = 0;
    TrayForm.bEnableAMR = false;
    IniConfig.bG16BinDispNeedAlarm = false; IniConfig.bC14SaveBinDisplayLog = false;
    SystemStart = false; SoftStart = false; bSystemClose = false; InitialOK = false;
    HSys.sNumberPanelComPort = "COM14";
    HSys.sNumberPanelComPort2 = "COM4";

    std::printf(" 1. NUMBER_PANEL_TYPE 0 = today\n");
    {
        NUMBER_PANEL_TYPE = 0;
        W906_BinDispSystemModularBoot_St02();
        CHECK(DataModule3 == NULL);
        CHECK(HSys.BinDisCtrl == NULL);
        ht9045::W906_St02TimersReset();
        W906_BinDispTestReset_St02();
        InitialOK = true;
        for (int i = 0; i < 20; ++i) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); }
        CHECK(BdCalls() == 0);
        CHECK(W906_BinDispTimer1Interval_St02() == 0);
        CHECK(std::string(W906_BinDispBinSelFormShow_St02()).find("not type 3") != std::string::npos);
        InitialOK = false;
    }

    std::printf(" 2. type 3 boot\n");
    OpenGeneralIniFile();                       // the scratch Gerneral.ini (containment checked asGeneralPath)
    INIFileGeneral->WriteString("System", "AUTO_EMPTY_COLOR", "1");
    INIFileGeneral->WriteString("System", "SUPPORT_2_EMPTY_EMPTY", "1");   // Empty / Color stay installed
    NUMBER_PANEL_TYPE = 3;
    W906_BinDispSystemModularBoot_St02();
    CHECK(DataModule3 != NULL);
    CHECK(HSys.BinDisCtrl != NULL);
    if (DataModule3 == NULL || HSys.BinDisCtrl == NULL)
    {
        std::printf("C14_BinDisp: %d / %d failed (no instance, stopping)\n", g_fail + 1, g_total);
        return 1;
    }
    CHECK(dynamic_cast<TMyBinDispHT9046*>(HSys.BinDisCtrl) != NULL);
    CHECK(DataModule3->BinDisp->IsSimMode() && DataModule3->BinDisp2->IsSimMode());
    CHECK(DataModule3->BinDisp->BaudRate == 9600 && DataModule3->BinDisp->ReadIntervalTimeout == 100 && DataModule3->BinDisp->Inx_XonXoffFlow);
    {
        const AnsiString p = HSys.BinDisCtrl->slBinDispLog->Path;
        std::printf("  BinDisplayLog Path = %s\n", p.c_str());
        CHECK(Contains(W906TestSafeLower(p.c_str()).c_str(), "machine_log_scratch"));
    }
    CHECK(fShowBinSelect->UnLoadPanel[eAuto1] == fShowBinSelect->pnlAuto1);    // P1
    CHECK(fShowBinSelect->UnLoadPanel[eFix12] == fShowBinSelect->pnlFix12);
    CHECK(fShowBinSelect->UnLoadPanel[eMag14] == fShowBinSelect->pnlMag14);
    for (int i = 0; i <= eBinDispFix6; ++i)
        if (!HSys.BinDisCtrl->UnitHasInstall(i)) { std::printf("  unit %d not installed\n", i); CHECK(false); }
    CHECK(!HSys.BinDisCtrl->UnitHasInstall(eBinDispBulkBox) && !HSys.BinDisCtrl->UnitHasInstall(eBinDispMag1) && !HSys.BinDisCtrl->UnitHasInstall(eBinDispAuto4));
    HSys.BinDisCtrl->SetDelayTime(0.0);          // rotation without wall-clock waits
    if (kSim)
        DataModule3->BinDisp->StartComm();       // SIM: golden never opens the port (Timer1Timer :435); the test opens the SIM capture

    std::printf(" 3. type 3 frames from a known tray state\n");
    {
        KnownTrayState();
        CHECK(fShowBinSelect->bUpdateBinDigital == false);
        ht9045::W906_St02TimersReset();
        W906_BinDispTestReset_St02();
        InitialOK = true;
        g_txOff = 0; g_frames3.clear(); g_mag3.clear();
        const size_t want = 12 + (kSim ? 0 : 12) + 12 + 4;   // versions, (colors), first bin pass, 4 rotation frames (legacy frames only)
        for (int step = 0; step < 3000 && g_frames3.size() < want; ++step)
        {
            g_now += 200;
            ht9045::W906_St02BinDispSlotAt(g_now);
            Respond3();
        }
        std::printf("  %u frames after %lu Timer1 calls (+ %u Magazine-TFT restore frames, golden :1787-1797)\n", (unsigned)g_frames3.size(), BdCalls(), (unsigned)g_mag3.size());
        CHECK(!g_mag3.empty() && g_mag3.size() % MAX_MGZ_TRAY == 0);   // golden 906 :1792-1797: whole bursts of MAX_MGZ_TRAY frames, even on type 3
        CHECK(g_mag3.empty() || ((unsigned char)g_mag3[0][1] == 1 && g_mag3[0][18] == '\r' && g_mag3[0][19] == '\n'));   // layer 1 first; 20-byte frame ends CR LF
        CHECK(HSys.BinDisCtrl->InitialOK);                 // golden FormShow copy
        CHECK(g_frames3.size() >= want);
        std::vector<std::string> exp;
        for (int a = 0; a <= eBinDispFix6; ++a) exp.push_back(ReadVersionFrame(a));
        if (!kSim)
        {
            const int col[12] = { 3, 3, 3, 1, 2, 1, 3, 3, 3, 3, 3, 3 };   // L / E / C orange, Auto1 red, Auto2 green, Auto3 red, unused orange
            for (int a = 0; a <= eBinDispFix6; ++a) exp.push_back(WriteColorFrame(a, col[a]));
        }
        exp.push_back(WriteBinFrame(0, 1, 11));   // Loader 111 -> letter L
        exp.push_back(WriteBinFrame(1, 1, 4));    // Empty 104 -> E
        exp.push_back(WriteBinFrame(2, 1, 2));    // Color 102 -> C
        exp.push_back(WriteBinFrame(3, 0, 0));    // Auto1: bins 0, 1 -> first 0
        exp.push_back(WriteBinFrame(4, 0, 2));    // Auto2: bin 2
        exp.push_back(WriteBinFrame(5, 1, 4));    // Auto3: the error bin 104 -> E
        for (int a = eBinDispFix1; a <= eBinDispFix6; ++a) exp.push_back(WriteBinFrame(a, 1, 23));   // no bin -> X
        exp.push_back(WriteBinFrame(3, 0, 1));    // rotation: only Auto1 has two bins
        exp.push_back(WriteBinFrame(3, 0, 0));
        exp.push_back(WriteBinFrame(3, 0, 1));
        exp.push_back(WriteBinFrame(3, 0, 0));
        bool same = g_frames3.size() >= exp.size();
        for (size_t i = 0; same && i < exp.size(); ++i)
            if (g_frames3[i] != exp[i])
            {
                std::printf("  frame %u: got \"%s\" want \"%s\"\n", (unsigned)i, g_frames3[i].substr(0, 15).c_str(), exp[i].substr(0, 15).c_str());
                same = false;
            }
        CHECK(same);
        CHECK(HSys.BinDisCtrl->GetBinNow(0) == 111 && HSys.BinDisCtrl->GetBinNow(4) == 2 && HSys.BinDisCtrl->GetBinNow(5) == 104 && HSys.BinDisCtrl->GetBinNow(6) == -1);
        CHECK(HSys.BinDisCtrl->GetColorNow(4) == (kSim ? 1 : 2));   // SIM: golden skips the colour pass (Timer1Timer :436)
        CHECK(!HSys.BinDisCtrl->GerErrNow(3));
    }

    std::printf(" 4. ChangeBinDispStatus paints the panels ShowBinSel writes\n");
    {
        IniConfig.bG16BinDispNeedAlarm = true;   // golden paints with G16 even when tsUnloadMap is not shown
        fShowBinSelect->ChangeBinDispStatus();
        CHECK(std::string(fShowBinSelect->pnlLoader->Caption.c_str()) == "L");
        CHECK(std::string(fShowBinSelect->pnlEmpty->Caption.c_str()) == "E");
        CHECK(std::string(fShowBinSelect->pnlAuto2->Caption.c_str()) == "2");
        CHECK(std::string(fShowBinSelect->pnlAuto3->Caption.c_str()) == "E");
        CHECK(std::string(fShowBinSelect->pnlFix1->Caption.c_str()) == "X");
        CHECK(fShowBinSelect->pnlAuto2->Color == (kSim ? kClRed : kClGreen));
        CHECK(fShowBinSelect->pnlBinBox->Color == kClGray && std::string(fShowBinSelect->pnlBinBox->Caption.c_str()) == "X");
        CHECK(fShowBinSelect->UnLoadPanel[eAuto2]->Color == fShowBinSelect->pnlAuto2->Color);   // the same object (P1)
        CHECK(std::string(fShowBinSelect->sbRunStatus->Panels->Items[0]->Text.c_str()) == std::string(HSys.BinDisCtrl->GetRunStatus().c_str()));
        IniConfig.bG16BinDispNeedAlarm = false;
    }

    std::printf(" 5. the BinSel FormShow pause\n");
    {
        SystemStart = true;
        CHECK(std::string(W906_BinDispBinSelFormShow_St02()).find("running") != std::string::npos);
        SystemStart = false;
        CHECK(std::string(W906_BinDispBinSelFormShow_St02()).find("ProcessStopStart(false)") != std::string::npos);
        const size_t before = DataModule3->BinDisp->SimTxBuffer().size();
        for (int i = 0; i < 20; ++i) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); Respond3(); }
        CHECK(DataModule3->BinDisp->SimTxBuffer().size() == before);   // paused: bStopProcess false -> Timer1Timer returns
        fShowBinSelect->bUpdateBinDigital = true;
        fShowBinSelect->DoShowBinDigital();       // golden FormClose -> InitShowBinDigital -> the next DoShowBinDigital resumes
        for (int i = 0; i < 20; ++i) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); Respond3(); }
        CHECK(DataModule3->BinDisp->SimTxBuffer().size() > before);
    }

    std::printf(" 6. type 4 (TFT)\n");
    {
        CHECK(MAGAZINE_BIN_DISP_TYPE != eTFT);                                  // ST02-S22 premise: D (BinDispBringUp_St02.cpp Timer1Timer case 100) is for no Magazine TFT
        DataModule3->BinDisp->StopComm(); DataModule3->BinDisp2->StopComm();
        DataModule3->BinDisp->SimClearTx();
        delete HSys.BinDisCtrl;                   // test only: a fresh instance for the TFT machine
        HSys.BinDisCtrl = NULL;
        NUMBER_PANEL_TYPE = 4;
        W906_BinDispSystemModularBoot_St02();
        CHECK(HSys.BinDisCtrl != NULL);
        if (HSys.BinDisCtrl == NULL) { std::printf("C14_BinDisp: %d / %d failed\n", g_fail + 1, g_total); return 1; }
        HSys.BinDisCtrl->SetDelayTime(0.0);
        if (kSim)
            DataModule3->BinDisp->StartComm();
        KnownTrayState();
        // ST02-S22 E1 (V912 MyBinDisp.cpp:651-655): type 4 WriteTargetBin mirrors the target before any frame (new instance: 0 / 1, MyBinDisp.cpp:166-167)
        CHECK(HSys.BinDisCtrl->GetColorNow(4) == 2 && HSys.BinDisCtrl->GetBinNow(4) == 2 && HSys.BinDisCtrl->GetColorNow(3) == 1 && HSys.BinDisCtrl->GetBinNow(3) == 0);
        CHECK(HSys.BinDisCtrl->GetBinNow(5) == 104 && HSys.BinDisCtrl->GetBinNow(0) == 111 && HSys.BinDisCtrl->GetColorNow(0) == 3);
        ht9045::W906_St02TimersReset();
        W906_BinDispTestReset_St02();
        g_txOff = 0; g_framesTFT.clear();
        const size_t want = 12 + 8 * 12 + 12 + 12 + 3 + 3;   // versions, DoOnce, WriteBin_TFT, three count passes
        for (int step = 0; step < 6000 && g_framesTFT.size() < want; ++step)
        {
            g_now += 200;
            ht9045::W906_St02BinDispSlotAt(g_now);
            RespondTFT();
        }
        std::printf("  %u TFT frames\n", (unsigned)g_framesTFT.size());
        CHECK(g_framesTFT.size() >= want);
        CHECK(DataModule3->BinDisp->ReadIntervalTimeout == 50 && !DataModule3->BinDisp->Inx_XonXoffFlow);   // golden case 1 TFT
        if (g_framesTFT.size() >= want)
        {
            const unsigned char ver0[20] = { 0x3A, 0x20, 0x00, 0x0D, 0x08, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0, 0x0D, 0x0A };
            std::string v(ver0, ver0 + 20); v[17] = (char)Lrc16(ver0);
            CHECK(g_framesTFT[0] == v);                                       // ReadVersion_TFT(0)
            unsigned char nb[20] = { 0x3A, 0x20, 0x00, 0x0D, 0x00, 0x04, 0x00, 0x01, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x07, 0x00, 0, 0x0D, 0x0A };
            nb[17] = Lrc16(nb);
            CHECK(g_framesTFT[12] == std::string((const char*)nb, 20));     // DoOnce type 0: SetNoBackGround_TFT(0)
            unsigned char bg[20] = { 0x3A, 0x20, 0x00, 0x0D, 0x00, 0x01, 0x00, 0x00, 0x30, 0x30, 0x00, 0x59, 0xAA, 0x30, 0x69, 0x69, 0x69, 0, 0x0D, 0x0A };
            bg[17] = Lrc16(bg);                                              // index 0 < 3: data item 00 00 (golden :1105-1108)
            CHECK(g_framesTFT[24] == std::string((const char*)bg, 20));     // DoOnce type 1: SetBackGround_TFT(0), 20 bytes from a [21] buffer
            const size_t c0 = 12 + 8 * 12;
            const char* text[12] = { "Loader", "Empty", "Color", "000", "002", "  E", "---", "---", "---", "---", "---", "---" };
            bool same = true;
            for (int a = 0; a < 12; ++a)
                if (g_framesTFT[c0 + a] != TftTextFrame(0x20 + a, 0x01, text[a])) { std::printf("  WriteBin_TFT unit %d differs\n", a); same = false; }
            CHECK(same);
            CHECK(g_framesTFT[c0 + 12] == TftTextFrame(0x20, 0x04, ""));    // count pass 1, Loader: no count text
            CHECK(g_framesTFT[c0 + 15] == TftTextFrame(0x23, 0x04, "0"));   // Auto1 count 0
        }
        // ST02-S22 D (V912 MyBinDisp.cpp:509-512 kept, RULINGS_20261003 #1; was "906: one TFT cycle per WriteTargetBin, then idle"):
        // pure type 4 keeps cycling.  A light round is ~26 Timer1 calls: Auto1's WriteBin (its two bins alternate) + three count
        // passes over Auto1-3.  No WriteTargetBin / WriteTargetCount in this loop.
        const size_t n0 = g_framesTFT.size();
        bool e3 = false;
        for (int i = 0; i < 160; ++i)
        {
            const size_t before = g_framesTFT.size();
            TickTFT(1);
            if (!e3 && g_framesTFT.size() > before && g_framesTFT.back() == TftTextFrame(0x23, 0x01, "001"))
            {
                TickTFT(1);                                                     // the ack is taken by the next Timer1 call
                ++i;
                CHECK(HSys.BinDisCtrl->GetBinNow(3) == 1 && HSys.BinDisCtrl->GetColorNow(3) == 1);   // E3 (V912 :3113-3114): E1 alone leaves bin 0
                e3 = true;
            }
        }
        CHECK(e3);
        std::vector<std::string> w23;
        int w24 = 0, c24 = 0;
        for (size_t k = n0; k < g_framesTFT.size(); ++k)
        {
            if (TftIs(g_framesTFT[k], 3, 1, 0x23)) w23.push_back(TftText(g_framesTFT[k]));
            if (TftIs(g_framesTFT[k], 3, 1, 0x24)) ++w24;
            if (TftIs(g_framesTFT[k], 3, 4, 0x24)) ++c24;
        }
        bool alt = w23.size() >= 4;
        for (size_t k = 0; k < w23.size(); ++k)
            if ((w23[k] != "000" && w23[k] != "001") || (k > 0 && w23[k] == w23[k - 1])) alt = false;
        std::printf("  D: %u frames in 160 calls: Auto1 WriteBin x%u, Auto2 WriteBin x%d, Auto2 count x%d\n",
                    (unsigned)(g_framesTFT.size() - n0), (unsigned)w23.size(), w24, c24);
        CHECK(alt);                                                             // Auto1 rotates "000" / "001" (906: nothing after round 1)
        CHECK(w24 == 0);                                                        // Auto2 has one bin: its bSliding went false in round 1
        CHECK(c24 >= 12);                                                       // the counts are refreshed every round
    }

    std::printf(" 6a. ST02-S22 D: a new target set mid-round goes straight to DoOnce; the BinSel pause still stops the bus\n");
    {
        CHECK(TickUntilSent(60, 3, 1, 0x23));                                   // a light round's Auto1 WriteBin just went out (9 normal frames to come)
        fShowBinSelect->bUpdateBinDigital = true;
        fShowBinSelect->DoShowBinDigital();                                     // WriteTargetBin x36 + ProcessStopStart(true) -> iBinDispCtrlTask=50 (MyBinDisp.cpp:321-322)
        size_t idx = 0;
        const bool found = TickUntilNewFrame(4, idx);                           // case 50 (no frame), case 100 -> 400, DoOnce sends
        CHECK(found);
        CHECK(found && TftIs(g_framesTFT[idx], 4, 1, 0x20) && g_framesTFT.size() == idx + 1);   // DoOnce type 0 SetNoBackGround_TFT(0), no fn 3 frame first
        TickTFT(400);                                                           // DoOnce + a full round, then light rounds again
        SystemStart = false; SoftStart = false;
        CHECK(std::string(W906_BinDispBinSelFormShow_St02()).find("ProcessStopStart(false)") != std::string::npos);
        const size_t paused = Tx();
        TickTFT(20);
        CHECK(Tx() == paused);                                                  // D does not run past the pause (Timer1Timer bStopProcess guard)
        fShowBinSelect->bUpdateBinDigital = true;
        fShowBinSelect->DoShowBinDigital();                                     // resume (section 7's P66 check needs bStopProcess == true)
        TickTFT(400);
        CHECK(Tx() > paused);
    }

    std::printf(" 6b. ST02-S22 A: a changed count jumps the queue, an unchanged one is not re-sent, a round-end wake-up\n");
    {
        // A1: a count changes right after Auto2's count frame went out
        CHECK(TickUntilSent(60, 3, 4, 0x24));
        W906_BinDispWriteTargetCount_St02(eBinDispAuto2, 7);                    // what cSortCT.cpp's G2 calls (golden 0618 cSortCT.cpp:408)
        size_t i1 = 0, i2 = 0;
        const bool f1 = TickUntilNewFrame(10, i1);
        CHECK(f1 && g_framesTFT[i1] == TftTextFrame(0x24, 0x04, "7"));          // the insert, before Auto3's normal count (A4 + A8)
        const bool f2 = f1 && TickUntilNewFrame(10, i2);
        CHECK(f2 && TftIs(g_framesTFT[i2], 3, 4, 0x25));                        // then the rotation goes on at Auto3 (A9 clears the insert)
        // A2: the same value again is not sent again
        CHECK(TickUntilSent(60, 3, 4, 0x24) && TftText(g_framesTFT.back()) == "7");   // Auto2's next normal count shows the stored 7
        W906_BinDispWriteTargetCount_St02(eBinDispAuto2, 7);
        size_t i3 = 0;
        const bool f3 = TickUntilNewFrame(10, i3);
        CHECK(f3 && !TftIs(g_framesTFT[i3], 3, 4, 0x24));                       // no insert: Auto3's normal count comes next
        // A4b: a count changed after the round ended wakes DoCycle from case 1 (V912 behaviour)
        int c25 = -1;
        bool roundEnd = false;
        for (int i = 0; i < 120 && !roundEnd; ++i)
        {
            const size_t before = g_framesTFT.size();
            TickTFT(1);
            if (g_framesTFT.size() == before) continue;
            if (TftIs(g_framesTFT.back(), 3, 1, 0x23)) c25 = 0;
            else if (c25 >= 0 && TftIs(g_framesTFT.back(), 3, 4, 0x25) && ++c25 == 3) roundEnd = true;   // the round's last frame
        }
        CHECK(roundEnd);
        const size_t quiet = g_framesTFT.size();
        TickTFT(2);
        CHECK(g_framesTFT.size() == quiet);                                     // the ack, then the round ends (nothing dirty)
        W906_BinDispWriteTargetCount_St02(eBinDispAuto2, 8);
        std::vector<std::string> nx = NextFrames(7);
        CHECK(!nx.empty() && nx[0] == TftTextFrame(0x24, 0x04, "8"));           // the insert first
        bool loader = false;
        for (size_t k = 1; k < nx.size(); ++k)
            if (nx[k] == TftTextFrame(0x20, 0x01, "Loader")) loader = true;
        CHECK(loader);                                                          // DoCycle case 1: the whole round again, Loader included
        // A8: nine counts change at once -> 8 inserts in a row, one normal frame, then the 9th insert
        KnownTrayState9();
        TickTFT(400);
        CHECK(TickUntilSent(80, 3, 4, 0x23));                                   // Auto1's normal count just went out
        for (int u = eBinDispAuto1; u <= eBinDispFix6; ++u) W906_BinDispWriteTargetCount_St02(u, 200 + u);
        nx = NextFrames(10);
        CHECK(PriorityRun8(nx, 200));                                           // Auto1..Fix5 inserted, unit order
        CHECK(nx.size() == 10 && TftIs(nx[8], 3, 4, 0x24));                     // then the rotation's own next frame (Auto2): the cap of 8
        CHECK(nx.size() == 10 && TftIs(nx[9], 3, 4, 0x2B) && TftText(nx[9]) == "211");   // then Fix6's insert
        // A10: nine counts change after the round's last frame -> 8 inserts end the round, Fix6 is still dirty
        int c2b = -1;
        roundEnd = false;
        for (int i = 0; i < 200 && !roundEnd; ++i)
        {
            const size_t before = g_framesTFT.size();
            TickTFT(1);
            if (g_framesTFT.size() == before) continue;
            if (TftIs(g_framesTFT.back(), 3, 1, 0x23)) c2b = 0;
            else if (c2b >= 0 && TftIs(g_framesTFT.back(), 3, 4, 0x2B) && ++c2b == 3) roundEnd = true;
        }
        CHECK(roundEnd);
        for (int u = eBinDispAuto1; u <= eBinDispFix6; ++u) W906_BinDispWriteTargetCount_St02(u, 300 + u);
        nx = NextFrames(10);
        CHECK(PriorityRun8(nx, 300));
        CHECK(nx.size() == 10 && TftIs(nx[8], 3, 4, 0x2B) && TftText(nx[8]) == "311");   // the round ended, bStartCycle=IsAnyCountDirty() woke the next
        CHECK(nx.size() == 10 && nx[9] == TftTextFrame(0x20, 0x01, "Loader"));  // ... from DoCycle case 1 (Loader), not the case-50 resume
        KnownTrayState();                                                       // section 6's tray state back for the sections after (6c rebuilds the controller)
    }

    std::printf(" 6c. ST02-S22 E2: a fresh controller shows its initial bins once the bin font is acked (no WriteTargetBin)\n");
    {
        DataModule3->BinDisp->StopComm(); DataModule3->BinDisp2->StopComm();
        DataModule3->BinDisp->SimClearTx();
        delete HSys.BinDisCtrl;                   // test only: a fresh TFT controller, as section 6
        HSys.BinDisCtrl = NULL;
        W906_BinDispSystemModularBoot_St02();
        CHECK(HSys.BinDisCtrl != NULL);
        if (HSys.BinDisCtrl == NULL) { std::printf("C14_BinDisp: %d / %d failed\n", g_fail + 1, g_total); return 1; }
        HSys.BinDisCtrl->SetDelayTime(0.0);
        if (kSim)
            DataModule3->BinDisp->StartComm();
        ht9045::W906_St02TimersReset();
        W906_BinDispTestReset_St02();
        g_txOff = 0; g_framesTFT.clear();
        HSys.BinDisCtrl->ProcessStopStart(true);                                // bFirstInit -> InitialTask (MyBinDisp.cpp:314-318)
        CHECK(TickUntilSent(2000, 2, 2, -1));                                   // DoOnce type 3 began: every unit acked type 2 (SetFontBin_TFT)
        bool anyWriteBin = false;
        for (size_t k = 0; k < g_framesTFT.size(); ++k)
            if (TftIs(g_framesTFT[k], 3, 1)) anyWriteBin = true;
        CHECK(!anyWriteBin);
        CHECK(HSys.BinDisCtrl->GetBinNow(0) == 111 && HSys.BinDisCtrl->GetColorNow(0) == 3);   // the ctor's iSetBin / iSetColor (MyBinDisp.cpp:186-191), via E2
        TickTFT(400);
    }

    std::printf(" 7. the pump cadence\n");
    {
        ht9045::W906_St02TimersReset();
        const unsigned long ms = W906_BinDispTimer1Interval_St02();
        std::printf("  Timer1 Interval = %lu\n", ms);
        CHECK(ms == 200);
        unsigned long n0 = BdCalls();
        for (int i = 0; i < 40; ++i) { g_now += 50; ht9045::W906_St02BinDispSlotAt(g_now); }   // 2 s in 50 ms passes
        const unsigned long n50 = BdCalls() - n0;
        std::printf("  50 ms passes over 2 s -> %lu calls\n", n50);
        CHECK(n50 >= 9 && n50 <= 10);                                            // one per 200 ms (the first one Interval after the start)
        n0 = BdCalls();
        for (int i = 0; i < 10; ++i) { g_now += 500; ht9045::W906_St02BinDispSlotAt(g_now); }  // the ~500 ms PumpTick beat
        CHECK(BdCalls() - n0 == 10);
        // golden Timer1Timer :313-320: 50 ms while a P66 flash runs -- the new Interval restarts the deadline (VCL SetInterval)
        IniConfig.bP66AutoChangingFlashWarn = true;
        HSys.BinDisCtrl->StartFlash(eBinDispAuto1, 1, 4, 250);
        for (int i = 0; i < 3; ++i) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); }
        CHECK(W906_BinDispTimer1Interval_St02() == 50);
        n0 = BdCalls();
        for (int i = 0; i < 40; ++i) { g_now += 50; ht9045::W906_St02BinDispSlotAt(g_now); }
        std::printf("  flashing: 50 ms passes over 2 s -> %lu calls\n", BdCalls() - n0);
        CHECK(BdCalls() - n0 >= 38);
        IniConfig.bP66AutoChangingFlashWarn = false;                             // the next tick puts iOldTimerInterval back
        for (int i = 0; i < 3; ++i) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); }
        CHECK(W906_BinDispTimer1Interval_St02() == 200);
        bSystemClose = true;                                                     // the port's FormClose
        InitialOK = false;
        n0 = BdCalls();
        for (int i = 0; i < 10; ++i) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); }
        CHECK(BdCalls() == n0);
        CHECK(HSys.BinDisCtrl->InitialOK == false);                              // golden FormClose :11569 copy
        bSystemClose = false;
    }

    std::printf(" 8. no COM port was opened\n");
    CHECK(DataModule3->BinDisp->IsSimMode() && DataModule3->BinDisp2->IsSimMode());
    CHECK(W906_BinDispRealPortProbes_St02() == 0);

    // ---- ST02-C14b (NB2 R168), AI(W906-ST02-C14b) 20261003 (St02-E): golden 0618 ------------------------------------------
    std::printf(" 9. C14b H1: the boot chain's InitShowBinDigital starts the display (golden 0618 main.cpp:9577 -> RunStartMode :783)\n");
    {
        DataModule3->BinDisp->StopComm(); DataModule3->BinDisp2->StopComm();
        DataModule3->BinDisp->SimClearTx();
        delete HSys.BinDisCtrl;                   // test only: a fresh controller, as the boot makes it
        HSys.BinDisCtrl = NULL;
        NUMBER_PANEL_TYPE = 3;
        W906_BinDispSystemModularBoot_St02();
        CHECK(HSys.BinDisCtrl != NULL);
        if (HSys.BinDisCtrl == NULL) { std::printf("C14_BinDisp: %d / %d failed\n", g_fail + 1, g_total); return 1; }
        HSys.BinDisCtrl->SetDelayTime(0.0);
        if (kSim)
            DataModule3->BinDisp->StartComm();
        ht9045::W906_St02TimersReset();
        W906_BinDispTestReset_St02();
        InitialOK = true;
        g_txOff = 0; g_frames3.clear(); g_mag3.clear();
        CHECK(fShowBinSelect->bUpdateBinDigital == false);   // never set by hand here: the last DoShowBinDigital cleared it (= the facade ctor's boot value)
        // control = the port before C14b: golden Timer3's DoShowBinDigital alone -> the fresh controller stays idle, nothing is sent
        fShowBinSelect->DoShowBinDigital();
        Ticks(30);
        CHECK(Tx() == 0);
        fShowBinSelect->InitShowBinDigital();                                   // the boot chain's last call (RunStartMode.cpp SetRunStartMode; the chain is pinned in section 12)
        CHECK(fShowBinSelect->bUpdateBinDigital == true);
        fShowBinSelect->DoShowBinDigital();                                     // the next Timer3 second (MainTimer3.cpp)
        CHECK(fShowBinSelect->bUpdateBinDigital == false);
        for (int i = 0; i < 300 && g_frames3.size() < 12; ++i) Ticks(1);
        std::printf("  %u type-3 frames after the boot step\n", (unsigned)g_frames3.size());
        CHECK(g_frames3.size() >= 12);                                          // golden Timer1Timer case 1: the 12 read-version frames at least
    }

    std::printf("10. C14b M1: a BinSel pause whose FormClose was skipped mid-run is resumed\n");
    {
        bool (*keepHook)(const char*) = W906_FormFShowHook;
        W906_FormFShowHook = &FakeFShow;
        SystemStart = false; SoftStart = false;
        g_binSelOpen = true;                                                    // the page opens while idle
        CHECK(std::string(W906_BinDispBinSelFormShow_St02()).find("ProcessStopStart(false)") != std::string::npos);
        const size_t before = Tx();
        Ticks(20);
        CHECK(Tx() == before);                                                  // paused
        CHECK(std::string(W906_BinDispBinSelResumeTick_St02()).find("still open") != std::string::npos);
        SystemStart = true;                                                     // START with the page open
        g_binSelOpen = false;                                                   // closed mid-run: the close tail is skipped (kSkipRunning) -- not called here
        fShowBinSelect->DoShowBinDigital();                                     // control = before C14b: Timer3 alone does not resume
        Ticks(20);
        CHECK(Tx() == before);
        CHECK(std::string(W906_BinDispBinSelResumeTick_St02()).find("InitShowBinDigital") != std::string::npos);
        fShowBinSelect->DoShowBinDigital();                                     // the same Timer3 second (MainTimer3.cpp: the tick, then DoShowBinDigital)
        Ticks(20);
        CHECK(Tx() > before);                                                   // running again
        CHECK(std::string(W906_BinDispBinSelResumeTick_St02()).find("not paused") != std::string::npos);   // once
        // golden 0618 main.cpp:3842-3846 hides palSetup on SystemStart only: SoftStart alone does not block the pause (R168 low)
        SystemStart = false; SoftStart = true; g_binSelOpen = true;
        CHECK(std::string(W906_BinDispBinSelFormShow_St02()).find("ProcessStopStart(false)") != std::string::npos);
        g_binSelOpen = false;
        CHECK(std::string(W906_BinDispBinSelResumeTick_St02()).find("InitShowBinDigital") != std::string::npos);
        fShowBinSelect->DoShowBinDigital();
        SoftStart = false; SystemStart = true;
        CHECK(std::string(W906_BinDispBinSelFormShow_St02()).find("running") != std::string::npos);
        SystemStart = false;
        W906_FormFShowHook = keepHook;
    }

    std::printf("11. C14b M2: power-loss re-init (golden 0618 csystem.cpp:4354-4355)\n");
    {
        // AI(W906-ST02-C14b) 20261003 (St02-E, gate b52a fix): was `bFirstInit == true` + `Tx() > before` (red in gate b52a ship): golden :4355 ProcessStopStart(true)
        //   consumes bFirstInit at once (golden MyBinDisp.cpp:182-195: InitialTask, then bFirstInit=false), and ship case 1
        //   re-opens the port, which clears the SIM capture.  What InitialTask proves itself: the read-version frames again.
        HSys.BinDisCtrl->ProcessStopStart(false);                               // stopped
        Ticks(5);                                                               // answer what was in flight
        const size_t f0 = g_frames3.size();
        Ticks(10);
        CHECK(g_frames3.size() == f0);                                          // stopped: nothing new
        HSys.BinDisCtrl->bFirstInit = false;
        W906_BinDispPowerLossReinit_St02();
        CHECK(HSys.BinDisCtrl->bFirstInit == false);                            // :4354 set it, :4355 consumed it (golden :182-195)
        for (int i = 0; i < 300 && g_frames3.size() < f0 + 12; ++i) Ticks(1);
        bool versions = g_frames3.size() >= f0 + 12;
        for (int a = 0; versions && a <= eBinDispFix6; ++a) versions = (g_frames3[f0 + a] == ReadVersionFrame(a));
        if (!versions)
            std::printf("  [11] after the re-init: %u frames (had %u), Tx %u, g_txOff %u, Timer1 calls %lu, port open %d sim %d\n",
                        (unsigned)g_frames3.size(), (unsigned)f0, (unsigned)Tx(), (unsigned)g_txOff, BdCalls(),
                        (int)DataModule3->BinDisp->IsOpen(), (int)DataModule3->BinDisp->IsSimMode());
        CHECK(versions);                                                        // InitialTask ran: case 1 -> 50 reads every version again, as at boot
        TMyBinDispCtrl* const keep = HSys.BinDisCtrl;
        HSys.BinDisCtrl = NULL;                                                 // a machine without a type 3 / 4 display
        W906_BinDispPowerLossReinit_St02();                                     // [W906] (7): no crash, nothing to do
        HSys.BinDisCtrl = keep;
    }

    std::printf("12. C14b call sites (argv[1] = the source dir)\n");
    if (argc < 2)
    {
        std::printf("  no source dir given\n");
        CHECK(false);
    }
    else
    {
        const std::string root = argv[1];
        // H1 = the real boot chain (NB2-2 A5): wb_serve boot -> W906_DoReadLastData(true) -> if (bBoot) fMain->SetStartModeData()
        //   -> SetRunStartMode in every branch -> fShowBinSelect->InitShowBinDigital() (golden 0618 main.cpp:9577 / RunStartMode :783)
        const std::vector<std::string> wb = ReadLines(root + "/tools/wb_serve.cpp");
        const int dr = FindLine(wb, "W906_DoReadLastData(true, binSelLoaded, tempLoaded);");
        CHECK(dr >= 0 && BeforeComment(wb[dr], "W906_DoReadLastData(true, binSelLoaded, tempLoaded);"));   // the boot calls it
        const int ssmd = FindLine(wb, "fMain->SetStartModeData();");
        CHECK(ssmd >= 0 && BeforeComment(wb[ssmd], "fMain->SetStartModeData();"));
        bool inBoot = false;
        for (int k = ssmd; k >= 0 && k > ssmd - 8; --k)
            if (wb[k].find("if (bBoot) {") != std::string::npos) inBoot = true;
        CHECK(inBoot);                                                          // inside W906_DoReadLastData's boot-only block
        CHECK(FindLine(wb, "W906_BinDispBootInit_St02") < 0);                   // the dropped H1 call stays dropped
        const std::vector<std::string> rs = ReadLines(root + "/RunStartMode.cpp");
        int setRun = -1, ssd = -1, nInit = 0, calls = 0;
        for (size_t k = 0; k < rs.size(); ++k)
        {
            if (rs[k].find("void SetRunStartMode(eRunStartMode Mode, AnsiString ModeText)") == 0) setRun = (int)k;
            if (rs[k].find("void TfMain::SetStartModeData()") == 0) ssd = (int)k;
            if (setRun >= 0 && ssd < 0 && BeforeComment(rs[k], "fShowBinSelect->InitShowBinDigital();")) ++nInit;
            if (ssd >= 0 && BeforeComment(rs[k], "SetRunStartMode(eRunStartMode(")) ++calls;
        }
        CHECK(setRun >= 0 && ssd > setRun && nInit >= 1);                       // SetRunStartMode arms the display (golden :783 equivalent)
        CHECK(calls >= 5);                                                      // SetStartModeData's branches all call SetRunStartMode (:1390-1412)
        const std::vector<std::string> t3 = ReadLines(root + "/MainTimer3.cpp");
        const int r = FindLine(t3, "W906_BinDispBinSelResumeTick_St02();");
        CHECK(r > 0 && FindLine(t3, "fShowBinSelect->DoShowBinDigital();") == r);
        if (r > 0)
        {
            CHECK(BeforeComment(t3[r], "W906_BinDispBinSelResumeTick_St02();") && BeforeComment(t3[r], "fShowBinSelect->DoShowBinDigital();"));
            CHECK(t3[r].find("W906_BinDispBinSelResumeTick_St02();") < t3[r].find("fShowBinSelect->DoShowBinDigital();"));
            CHECK(t3[r - 1].find("if(NUMBER_PANEL_TYPE!=2)") != std::string::npos && t3[r].find_first_not_of(' ') == t3[r].find('{'));   // one braced statement under the if
        }
        const std::vector<std::string> cs = ReadLines(root + "/csystem.cpp");
        const int g = FindLine(cs, "W906_BinDispPowerLossReinit_St02();");
        bool gated = false;                                                     // AI(W906-ST02-C14b) 20261003 (St02-E, gate b52a fix): was FindLine(cs, "#if 0 // GATE G09") < 0 for the
        for (int k = g - 1; g >= 0 && k >= 0 && k >= g - 8; --k)               //   whole file (red: :19525 is DoHeaterOn's fiosetview fan gate,
            if (cs[k].find("#if 0") != std::string::npos && cs[k].find("GATE G09") != std::string::npos) gated = true;   //   golden 0618 :1246-1247, a different gate with the same label)
        CHECK(g >= 0 && !gated);                                                // this G09 (golden 0618 csystem.cpp:4354-4355) is lifted
        if (g >= 0)
        {
            CHECK(BeforeComment(cs[g], "W906_BinDispPowerLossReinit_St02();"));
            bool rec = false;
            for (int k = g + 1; k < g + 6 && k < (int)cs.size(); ++k)
                if (cs[k].find("RecordProcess(\"SnSystemPower Off\")") != std::string::npos) rec = true;
            CHECK(rec);                                                         // inside golden's SnSystemPower-off branch
        }
    }
    {
        const std::vector<std::string> sc = ReadLines(std::string(argc >= 2 ? argv[1] : ".") + "/cSortCT.cpp");
        const int g2 = FindLine(sc, "W906_BinDispWriteTargetCount_St02(iTo3Unload[i]+3, LastSet.BinCT[0][iTo3Unload[i]]);");
        CHECK(argc >= 2 && g2 > 0 && BeforeComment(sc[g2], "W906_BinDispWriteTargetCount_St02(iTo3Unload[i]+3"));   // G2 lifted (golden 0618 cSortCT.cpp:408)
        CHECK(g2 > 0 && BeforeComment(sc[g2 - 1], "if(NUMBER_PANEL_TYPE==4)"));   // under golden's :407 condition
    }

    std::printf("13. C14b type 4 (TFT): the SortCT count reaches the panel (golden 0618 cSortCT.cpp:407-408)\n");
    {
        DataModule3->BinDisp->StopComm(); DataModule3->BinDisp2->StopComm();
        DataModule3->BinDisp->SimClearTx();
        delete HSys.BinDisCtrl;                   // test only: a fresh TFT controller (HT9050 = type 4, RULINGS_20261003 #19)
        HSys.BinDisCtrl = NULL;
        W906_BinDispWriteTargetCount_St02(eBinDispAuto1, 5);                    // [W906] (7): no instance -> no crash, nothing written
        NUMBER_PANEL_TYPE = 4;
        W906_BinDispSystemModularBoot_St02();
        CHECK(HSys.BinDisCtrl != NULL);
        if (HSys.BinDisCtrl == NULL) { std::printf("C14_BinDisp: %d / %d failed\n", g_fail + 1, g_total); return 1; }
        HSys.BinDisCtrl->SetDelayTime(0.0);
        if (kSim)
            DataModule3->BinDisp->StartComm();
        W906_BinDispWriteTargetCount_St02(eBinDispAuto1, 123);                  // what cSortCT.cpp's lifted G2 does for the Auto1 tray
        KnownTrayState();                                                       // section 6's tray state -> the TFT rounds (ST02-S22 D: they keep cycling)
        ht9045::W906_St02TimersReset();
        W906_BinDispTestReset_St02();
        InitialOK = true;
        g_txOff = 0; g_framesTFT.clear();
        // AI(W906-ST02-C14b) 20261003 (St02-E, NB2-1 R195 fix): want ends at the FIRST count pass (was + 3 + 3, red at 132 / 138 in R195 both configs).  The
        //   two 3-frame passes after it are counted by DoCycle's function-static iCntCycle (golden 0618 MyBinDisp.cpp DoCycle;
        //   reset only in case 50 after a whole round, not in case 1), which section 6 leaves at 2 -- it stops before case 50 --
        //   so this fresh controller sends one count pass.  Golden quirk across controllers; the 123 is in the first pass.
        const size_t want = 12 + 8 * 12 + 1 + 12 + 12;                          // + 1 = ST02-S22 A: the count set before the boot goes out first (priority insert); then versions, DoOnce, WriteBin_TFT, the first count pass
        for (int step = 0; step < 6000 && g_framesTFT.size() < want; ++step) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); RespondTFT(); }
        if (g_framesTFT.size() < want)                                         // AI(W906-ST02-C14b) 20261003 (St02-E, gate b52a fix): state for the proxy run
            std::printf("  [13] %u TFT frames (want %u), Tx %u, g_txOff %u, Timer1 calls %lu, port open %d sim %d\n",
                        (unsigned)g_framesTFT.size(), (unsigned)want, (unsigned)Tx(), (unsigned)g_txOff, BdCalls(),
                        (int)DataModule3->BinDisp->IsOpen(), (int)DataModule3->BinDisp->IsSimMode());
        CHECK(g_framesTFT.size() >= want);
        const size_t c0 = 12 + 8 * 12;
        if (g_framesTFT.size() >= want)
        {
            CHECK(g_framesTFT[c0] == TftTextFrame(0x23, 0x04, "123"));         // ST02-S22 A (V912 MyBinDisp.cpp:3034-3064): the changed count jumps the queue, before the WriteBin pass
            CHECK(g_framesTFT[c0 + 1 + 15] == TftTextFrame(0x23, 0x04, "123"));    // Auto1's count pass shows G2's 123 (section 6, without it: "0")
            CHECK(g_framesTFT[c0 + 1 + 16] == TftTextFrame(0x24, 0x04, "0"));      // Auto2 untouched
        }
    }
    // ---- [14] W-191 MR-3, AI(W906-W191) 20261009 (St02-E): golden 913 MyBinDisp.cpp TFT bin code 117 ------------------------------------
    //   Bin code 117 occurs only at CC_MAXIM_THAILAND (Ifor 20260819); golden 913 has no customer condition.  WriteBin_TFT shows "  R"
    //   (:927-930); the seven other TFT writers treat it as empty like 999 / 104 (:949 / :967 / :985 / :1005 / :1051 / :1084 / :1116).
    std::printf("-- [14] TFT bin code 117 (golden 913) --\n");
    {
        TMyBinDispHT9046* t = dynamic_cast<TMyBinDispHT9046*>(HSys.BinDisCtrl);
        if (t == NULL)                                                          // independent of the sections above: boot a TFT controller if none is left
        {
            delete HSys.BinDisCtrl;
            HSys.BinDisCtrl = NULL;
            NUMBER_PANEL_TYPE = 4;
            W906_BinDispSystemModularBoot_St02();
            t = dynamic_cast<TMyBinDispHT9046*>(HSys.BinDisCtrl);
        }
        CHECK(t != NULL && DataModule3 != NULL);
        if (t != NULL && DataModule3 != NULL)
        {
            if (kSim)
                DataModule3->BinDisp->StartComm();
            const int ix = 4;                                                   // an Auto unit (index != 1, so the empty text is "  E")
            auto Sent2 = [&](W191TftAccess::Fn2 f, int v) { const size_t a = Tx(); (t->*f)(ix, v); const std::vector<char>& tx = DataModule3->BinDisp->SimTxBuffer(); return std::string(tx.begin() + a, tx.end()); };
            auto Sent3 = [&](W191TftAccess::Fn3 f, int v) { const size_t a = Tx(); (t->*f)(ix, 2, v); const std::vector<char>& tx = DataModule3->BinDisp->SimTxBuffer(); return std::string(tx.begin() + a, tx.end()); };
            const std::string r117 = Sent2(W191TftAccess::WriteBin(), 117), e104 = Sent2(W191TftAccess::WriteBin(), 104);
            CHECK(r117.size() == 20 && TftText(r117) == "  R" && TftText(e104) == "  E");
            std::printf("  WriteBin_TFT 117 -> '%s', 104 -> '%s'\n", TftText(r117).c_str(), TftText(e104).c_str());
            const struct { const char* name; W191TftAccess::Fn2 f; } F2[] = {
                { "WriteBinWord_TFT", W191TftAccess::WriteBinWord() }, { "WriteEA_TFT", W191TftAccess::WriteEA() } };
            for (size_t i = 0; i < sizeof(F2) / sizeof(F2[0]); ++i)
            {
                const std::string a117 = Sent2(F2[i].f, 117), a104 = Sent2(F2[i].f, 104), a5 = Sent2(F2[i].f, 5);
                const bool ok = !a117.empty() && a117 == a104 && a5 != a104;
                if (!ok) std::printf("  %s: 117 '%s' / 104 '%s' / 5 '%s'\n", F2[i].name, TftText(a117).c_str(), TftText(a104).c_str(), TftText(a5).c_str());
                CHECK(ok);
            }
            {
                auto SentC = [&](int v) { const size_t a = Tx(); (t->*W191TftAccess::WriteCount())(ix, v, 7); const std::vector<char>& tx = DataModule3->BinDisp->SimTxBuffer(); return std::string(tx.begin() + a, tx.end()); };
                const std::string a117 = SentC(117), a104 = SentC(104), a5 = SentC(5);   // WriteCount_TFT(index, ivalue = the bin, iCount)
                const bool ok = !a117.empty() && a117 == a104 && a5 != a104;
                if (!ok) std::printf("  WriteCount_TFT: 117 '%s' / 104 '%s' / 5 '%s'\n", TftText(a117).c_str(), TftText(a104).c_str(), TftText(a5).c_str());
                CHECK(ok);
            }
            const struct { const char* name; W191TftAccess::Fn3 f; } F3[] = {
                { "SetFontBin_TFT", W191TftAccess::SetFontBin() },
                { "SetFontBinWord_TFT", W191TftAccess::SetFontBinWord() }, { "SetFontEA_TFT", W191TftAccess::SetFontEA() },
                { "SetFontCount_TFT", W191TftAccess::SetFontCount() } };
            for (size_t i = 0; i < sizeof(F3) / sizeof(F3[0]); ++i)
            {
                const std::string a117 = Sent3(F3[i].f, 117), a104 = Sent3(F3[i].f, 104), a5 = Sent3(F3[i].f, 5);
                const bool ok = !a117.empty() && a117 == a104 && a5 != a104;
                if (!ok) std::printf("  %s: 117 %u bytes / 104 %u bytes / 5 %u bytes, 117==104 %d\n", F3[i].name, (unsigned)a117.size(), (unsigned)a104.size(), (unsigned)a5.size(), (int)(a117 == a104));
                CHECK(ok);
            }
        }
    }
    CHECK(W906_BinDispRealPortProbes_St02() == 0);

    CloseGeneralIniFile();
    std::printf("C14_BinDisp: %d / %d failed\n", g_fail, g_total);
    return g_fail ? 1 : 0;
}
