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
//       byte-exact ("Loader" / "Empty" / "Color" / "000" / "002" / "  E" / "---"), the count passes, then idle (906).
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

static bool Contains(const AnsiString& s, const char* sub) { return std::string(s.c_str()).find(sub) != std::string::npos; }

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
        const size_t idle = DataModule3->BinDisp->SimTxBuffer().size();
        for (int i = 0; i < 50; ++i) { g_now += 200; ht9045::W906_St02BinDispSlotAt(g_now); RespondTFT(); }
        CHECK(DataModule3->BinDisp->SimTxBuffer().size() == idle);     // 906: one TFT cycle per WriteTargetBin (912 delta D keeps cycling)
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

    CloseGeneralIniFile();
    std::printf("C14_BinDisp: %d / %d failed\n", g_fail, g_total);
    return g_fail ? 1 : 0;
}
