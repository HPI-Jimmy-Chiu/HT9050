// =============================================================================
//  tests/test_st02_pad_interface.cpp -- AI(W906-ST02-P1) 20261005 (St02-E).  Suite: St02_PadInterface (both configs).
//
//  Card ST02-P1: the golden 0618 RS-232 operator pad (uPadInterface.cpp) in PadInterface_St02.h / .cpp, and the gates it
//  opens (mysensor.cpp / myswitch.cpp / cinitial.cpp / rs232.cpp / FastClockWbServe.cpp).  The port is vclcompat's simulated
//  TComm (W906_PadPortForTest(true): SimInjectReceive = bytes from the pad, SimTxBuffer = bytes to the pad).  No hardware, no
//  real file: the pad log root is moved under ctest's machine_log_scratch before anything runs (w906_ctest_guard.h).
//
//  SECTIONS (CHECK ids -> tests/REVERSE_P1 notes in the MR):
//    [1] ControlPanelMode 0 = today: boot open / boot port check / serve-loop job are no-ops; a pad-named Sen / SW reads the IO
//        path (Enable=false -> IsOn false, IsOff false) and SW On leaves the pad state alone
//    [2] mode 1, the table: IsPadKey / IsPadButton are exact (==); ProcessScanKey's golden Pos quirk (an Sw name reads false)
//    [3] the port opens (SHIP: W906_PadPortRS232Init = golden TrayStepMotor RS232Init; SIM: golden skips it, OpenCommPort);
//        first Main232 sends the boot scan "t051400000000\r" byte for byte
//    [4] front pad: "t050400000000" releases the safe lock, "t050400000040" -> Sen[SnFKStart].IsOn() true, release -> false
//    [5] PanelEnable: "t051400000004" -> rear pad active (Sen[SnRearPadActive], ScanPannelKey's bFrontPadActive false), the
//        rear lamp frame "t051490000004\r" then the third-pad copy "t052490000004\r"; rear START needs the enable bit held;
//        front START is ignored while the rear pad is active
//    [6] SafeLock: rear 0x4004 -> Sen[SnRKSafeLock] off, IsSafeLockCheck() true (SnRKCoverOpen wired), SW[SwRKSafeLock] lamp;
//        0x0004 -> released, IsSafeLockCheck() false
//    [7] SW gate: SW[SwFKStart].On() -> front lamp "t050490000040\r", Off -> "t050490000000\r"
//    [8] receive edges: a bad hex frame is logged "[Recv Error]" and the next frame still works; t07 / t08 are dropped
//    [9] version poll (~10 s, golden Main232 case 10): "t051120\r" goes out, a "20" reply sets bRequestVer; the port lost at
//        case 20 -> ResetComm (Disconnect / Connect, CommName "\\.\" + [TrayY] COM PORT)
//   [10] source pins (argv[1] = tree root, read only): the six sensor / switch gates are open, cinitial / rs232 / FastClock call
//        the pad entries
// =============================================================================
#include "PadInterface_St02.h"
#include "mysensor.h"
#include "myswitch.h"
#include "cmydef.h"
#include "common.h"
#include "cpublic.h"
#include "csystem.h"
#include "database.h"
#include "MachineType.h"
#include "FastClock.h"
#include "vclcompat/Comm.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

Spcomm::TComm* W906_PadPortForTest(bool forceSim);                              // PadInterface_St02.cpp test seams
unsigned long  W906_PadDroppedT07T08();
size_t         W906_PadRxQueuedBytesForTest();                                  // AI(W906-ST02-P1b) 20261006
void           W906_PadResetStateForTest();
void           W906_PadFastClockAdd(ht9045::fastclock::FastClock* clock, std::string& jobs);
int            ScanPannelKey();                                                 // ckernel.cpp

static int g_pass = 0, g_fail = 0;
#define CHECK(c) do { if (c) { ++g_pass; } else { ++g_fail; std::printf("  FAIL (line %d): %s\n", __LINE__, #c); } } while (0)

// ---- the port -------------------------------------------------------------------------------------------------------
static Spcomm::TComm* Port()       { return W906_PadPortForTest(true); }
static std::string Tx()            { const std::vector<char>& v = Port()->SimTxBuffer(); return std::string(v.begin(), v.end()); }
static void ClearTx()              { Port()->SimClearTx(); }
static void Rx(const char* s)      { Port()->SimInjectReceive(s, (Spcomm::Word)std::strlen(s)); }
static void Ticks(int n, DWORD ms) { for (int i = 0; i < n; ++i) { W906_PadThreadTick(); if (ms) ::Sleep(ms); } }
// ticks until the TX buffer holds `want` (a lamp frame waits for tSendDataDelay's 50 ms; the version poll for 10 s)
static bool TxUntil(const std::string& want, DWORD capMs)
{
    const DWORD t0 = ::GetTickCount();
    while ((DWORD)(::GetTickCount() - t0) < capMs) {
        if (Tx().find(want) != std::string::npos) return true;
        W906_PadThreadTick();
        ::Sleep(2);
    }
    const bool ok = Tx().find(want) != std::string::npos;
    if (!ok) std::printf("    (TX has no \"%s\"; it has: \"%s\")\n", want.c_str(), Tx().c_str());
    return ok;
}
// a frame from the pad, then enough ticks to drain it, process it and let the lamp sender run
static void Frame(const char* s) { Rx(s); Ticks(6, 2); }
static bool Memo(const char* s)
{
    for (size_t i = 0; i < fPadInterface->MemoLines.size(); ++i)
        if (std::string(fPadInterface->MemoLines[i].c_str()).find(s) != std::string::npos) return true;
    return false;
}

// ---- the 30 pad sensors / 31 pad lamps by name (golden uPadInterface.cpp:211-241; there is no SnFrontPadActive point) ----
struct NamedIdx { int idx; const char* name; };
static void NameAll()
{
    const NamedIdx sn[] = {
        {SnFKPowerOff,"SnFKPowerOff"},{SnFKPowerOn,"SnFKPowerOn"},{SnFKReset,"SnFKReset"},{SnFKPause,"SnFKPause"},
        {SnFKHome,"SnFKHome"},{SnFKStart,"SnFKStart"},{SnFKOneCycle,"SnFKOneCycle"},{SnFKRetry,"SnFKRetry"},{SnFKSkip,"SnFKSkip"},
        {SnFKCleanOut,"SnFKCleanOut"},{SnFKTrayFeed,"SnFKTrayFeed"},{SnFKTrayEnd,"SnFKTrayEnd"},{SnFKAlarmReset,"SnFKAlarmReset"},
        {SnRKPowerOff,"SnRKPowerOff"},{SnRKPowerOn,"SnRKPowerOn"},{SnRKReset,"SnRKReset"},{SnRKPause,"SnRKPause"},
        {SnRKHome,"SnRKHome"},{SnRKStart,"SnRKStart"},{SnRKOneCycle,"SnRKOneCycle"},{SnRKRetry,"SnRKRetry"},{SnRKSkip,"SnRKSkip"},
        {SnRKCleanOut,"SnRKCleanOut"},{SnRKTrayFeed,"SnRKTrayFeed"},{SnRKTrayEnd,"SnRKTrayEnd"},{SnRKAlarmReset,"SnRKAlarmReset"},
        {SnRKSafeLock,"SnRKSafeLock"},{SnRKManualStep,"SnRKManualStep"},{SnRKManualTStart,"SnRKManualTStart"},
        {SnRearPadActive,"SnRearPadActive"},{-1,0}};
    const NamedIdx sw[] = {
        {SwFKPowerOff,"SwFKPowerOff"},{SwFKPowerOn,"SwFKPowerOn"},{SwFrontActiveLed,"SwFrontActiveLed"},{SwFKReset,"SwFKReset"},
        {SwFKPause,"SwFKPause"},{SwFKHome,"SwFKHome"},{SwFKStart,"SwFKStart"},{SwFKOneCycle,"SwFKOneCycle"},{SwFKRetry,"SwFKRetry"},
        {SwFKSkip,"SwFKSkip"},{SwFKCleanOut,"SwFKCleanOut"},{SwFKTrayFeed,"SwFKTrayFeed"},{SwFKTrayEnd,"SwFKTrayEnd"},
        {SwFKAlarmReset,"SwFKAlarmReset"},{SwRKPowerOff,"SwRKPowerOff"},{SwRKPowerOn,"SwRKPowerOn"},{SwRKReset,"SwRKReset"},
        {SwRKPause,"SwRKPause"},{SwRKHome,"SwRKHome"},{SwRKStart,"SwRKStart"},{SwRKOneCycle,"SwRKOneCycle"},{SwRKRetry,"SwRKRetry"},
        {SwRKSkip,"SwRKSkip"},{SwRKCleanOut,"SwRKCleanOut"},{SwRKTrayFeed,"SwRKTrayFeed"},{SwRKTrayEnd,"SwRKTrayEnd"},
        {SwRKAlarmReset,"SwRKAlarmReset"},{SwRKSafeLock,"SwRKSafeLock"},{SwRKManualStep,"SwRKManualStep"},
        {SwRKManualTStart,"SwRKManualTStart"},{SwRearActiveLed,"SwRearActiveLed"},{-1,0}};
    for (int i = 0; sn[i].name; ++i) { Sen[sn[i].idx].Name = sn[i].name; Sen[sn[i].idx].Enable = false; }   // golden cinitial :2897-2934 disables them at mode 1
    for (int i = 0; sw[i].name; ++i) { SW[sw[i].idx].Name = sw[i].name; SW[sw[i].idx].Enable = false; }
}

static std::string ReadSource(const std::string& root, const char* rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static std::vector<std::string> Lines(const std::string& s)
{
    std::vector<std::string> v;
    std::string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\n') { v.push_back(cur); cur.clear(); }
        else if (s[i] != '\r') cur += s[i];
    }
    v.push_back(cur);
    return v;
}
// lines containing `code` whose previous line is not a preprocessor `#if`, i.e. live (not under a `#if 0` opened right above)
static int LiveLines(const std::string& src, const char* code)
{
    const std::vector<std::string> v = Lines(src);
    int n = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i].find(code) == std::string::npos) continue;
        const std::string prev = (i > 0) ? v[i - 1] : std::string();
        const size_t p = prev.find_first_not_of(" \t");
        const bool gatedAbove = (p != std::string::npos && prev.compare(p, 3, "#if") == 0);
        const size_t q = v[i].find_first_not_of(" \t");
        const bool commented = (q != std::string::npos && v[i].compare(q, 2, "//") == 0);
        if (!gatedAbove && !commented) ++n;
    }
    return n;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef SOFT_SIMULTE
    std::printf("St02_PadInterface (SIM)\n");
#else
    std::printf("St02_PadInterface (SHIP)\n");
#endif
    asPadCommLogPath = as9045LogPath + "\\PadCommLog";                         // the pad log root under ctest's scratch (golden D:\HT9045_Log\PadCommLog)
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asPadCommLogPath", asPadCommLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("St02_PadInterface", rt))
        return 2;
    GetTimeInfo();
    NameAll();
    HSys.TrayStepMotor_ComPort = "COM18";                                       // golden database.cpp:522 default
    InitialOK = true;  SystemInitialOK = true;                              // Main232 / ScanPannelKey run only after boot

    // [1] ControlPanelMode 0 (every machine today) = today's behaviour
    std::printf("[1] ControlPanelMode 0: nothing opens, no job, the gates read the IO path\n");
    {
        iControlPanelMode = 0;
        LoaderUnload_StepMotor = 1;                                             // golden would open the shared port for the step motor: not ported (header (6))
        W906_PadPortRS232Init("COM18");
        CHECK(W906_PadPortForTest(false)->IsOpen() == false);                   // P1-1a
        bool flag[11];
        for (int i = 0; i < 11; ++i) flag[i] = true;
        W906_RS232InitPadCheck(flag);
        CHECK(flag[9] == true);                                                 // P1-1b: no port probe
        ht9045::fastclock::FastClock clk;
        std::string jobs = "x";
        W906_PadFastClockAdd(&clk, jobs);
        CHECK(clk.Size() == 0 && jobs == "x");                                  // P1-1c: no pad job, the [FASTCLK] line unchanged
        CHECK(Sen[SnFKStart].IsOn() == false && Sen[SnFKStart].IsOff() == false);   // P1-1d: IO path, Enable=false
        SW[SwFKStart].On();
        CHECK(fPadInterface->bPadStatus[6] == false);                           // P1-1e: the pad lamp state is not touched
        SW[SwFKStart].Off();
        LoaderUnload_StepMotor = 0;
    }

    // [2] mode 1: the table
    std::printf("[2] the table: exact names, golden's Pos quirk\n");
    {
        iControlPanelMode = 1;
        CHECK(fPadInterface->CheckPadItem == 31);
        CHECK(fPadInterface->IsPadKey("SnFKStart") && !fPadInterface->IsPadKey("SnFKStar") && !fPadInterface->IsPadKey("SwFKStart"));   // P1-2a
        CHECK(fPadInterface->IsPadButton("SwRKSafeLock") && !fPadInterface->IsPadButton("SnRKSafeLock"));                             // P1-2b
        CHECK(fPadInterface->ProcessScanKey("SwFKStart") == false);             // P1-2c: TMySwitch::Status reads false (golden quirk)
        CHECK(fPadInterface->PadItem[6].iData == 0x000040 && fPadInterface->PadItem[27].iData == 0x004000 &&
              fPadInterface->PadItem[19].mlEvent->Tag == 1 && fPadInterface->PadItem[6].mlEvent->Tag == 0);                         // P1-2d
        CHECK(Sen[SnFKStart].IsOn() == false && Sen[SnFKStart].IsOff() == true);   // P1-2e: mode 1 reads the pad (nothing yet)
    }

    // [3] the port opens; the boot scan
    std::printf("[3] the port opens; the first Main232 sends the boot scan\n");
    {
        Port();
        W906_PadResetStateForTest();
#ifdef SOFT_SIMULTE
        W906_PadPortRS232Init("COM18");                                         // golden RS232Init is #ifndef SOFT_SIMULTE: nothing
        CHECK(fPadInterface->bRs232Ok == false);
        CHECK(fPadInterface->OpenCommPort() == true && fPadInterface->bRs232Ok == true);   // P1-3a
#else
        W906_PadPortRS232Init("COM18");
        CHECK(fPadInterface->bRs232Ok == true && Port()->CommName == AnsiString("\\\\.\\COM18") && Port()->BaudRate == 115200 &&
              Port()->ReadIntervalTimeout == 1);                                // P1-3a: golden TrayStepMotor.cpp:69-77
#endif
        ClearTx();
        Ticks(1, 0);
        CHECK(Tx() == std::string("t051400000000\r"));                          // P1-3b: golden uPadInterface.cpp:887 + SendCommand's CR
        CHECK(Memo("[Send]") && Memo("t051400000000"));                         // P1-3c: RecordCommunication
    }

    // [4] front pad keys
    std::printf("[4] front pad: release frame, START pressed, START released\n");
    {
        Frame("t050400000000\r");
        CHECK(Sen[SnRKSafeLock].IsOn() == true);                                // P1-4a: a frame without the SafeLock bit = released (golden :653-658)
        Frame("t050400000040\r");
        CHECK(Sen[SnFKStart].IsOn() == true && Sen[SnFKStart].IsOff() == false);   // P1-4b
        CHECK(Memo("[Recv]") && Memo("t050400000040"));                         // P1-4c
        Frame("t050400000000\r");
        CHECK(Sen[SnFKStart].IsOn() == false);                                  // P1-4d
    }

    // [5] PanelEnable: the rear pad
    std::printf("[5] PanelEnable: rear pad active, its lamp + the third-pad copy, rear START, front ignored\n");
    {
        ClearTx();
        Frame("t051400000004\r");
        CHECK(Sen[SnRearPadActive].IsOn() == true && fPadInterface->bPadStatus[30] == true);   // P1-5a: golden :555-559
        ScanPannelKey();
        CHECK(bFrontPadActive == false);                                        // P1-5b: ckernel ScanPannelKey (golden :1927)
        CHECK(TxUntil("t051490000004\r", 2000));                                // P1-5c: golden ProcessSendDataNew, rear lamp
        CHECK(TxUntil("t052490000004\r", 2000));                                // P1-5d: the third-pad copy (golden :833-837 / :763-769)
        Frame("t051400000044\r");
        CHECK(Sen[SnRKStart].IsOn() == true && Sen[SnRearPadActive].IsOn() == true);   // P1-5e
        Frame("t051400000004\r");
        CHECK(Sen[SnRKStart].IsOn() == false);
        Frame("t050400000040\r");
        CHECK(Sen[SnFKStart].IsOn() == false);                                  // P1-5f: front keys off while the rear pad is active (golden :603-623)
        Frame("t051400000000\r");
        CHECK(Sen[SnRearPadActive].IsOn() == false);                            // P1-5g: rear iKey 0 -> inactive (golden :560-564)
        ScanPannelKey();
        CHECK(bFrontPadActive == true);                                         // P1-5h: IsOff's pad arm (with P1-5b both polarities)
        ScanPannelKey();                                                        // ScanPannelKey's release sweep
    }

    // [6] SafeLock
    std::printf("[6] SafeLock: locked -> IsSafeLockCheck true + lamp, released -> false\n");
    {
        Sen[SnRKCoverOpen].Enable = true;  Sen[SnRKCoverOpen].Type = TYPE_B;  Sen[SnRKCoverOpen].ISABase = eISABase;   // wired, cover closed (reads ON offline, test_sysinit_boot.cpp seam)
        Frame("t051400000004\r");
        ClearTx();                                                              // before the frame: its lamp may go out inside Frame()
        Frame("t051400004004\r");
        CHECK(Sen[SnRKSafeLock].IsOff() == true && fPadInterface->bPadStatus[27] == true);   // P1-6a: golden :636-640
        CHECK(IsSafeLockCheck() == true);                                       // P1-6b: csystem.cpp IsSafeLockCheck, the pad arm
        CHECK(TxUntil("t051490004004\r", 2000));                                // P1-6c: rear lamp with SafeLock + enable
        Frame("t051400000004\r");
        CHECK(Sen[SnRKSafeLock].IsOn() == true && fPadInterface->bPadStatus[27] == false);  // P1-6d
        CHECK(IsSafeLockCheck() == false);                                      // P1-6e
        Sen[SnRKCoverOpen].Enable = false;
        Frame("t051400000000\r");
    }

    // [7] the switch gate drives the lamps
    std::printf("[7] SW[SwFKStart] On / Off -> the front lamp frames\n");
    {
        ClearTx();
        SW[SwFKStart].On();
        CHECK(fPadInterface->bPadStatus[6] == true);                            // P1-7a: myswitch gate -> SendSwitchStatus(name,true)
        CHECK(TxUntil("t050490000040\r", 2000));                                // P1-7b
        ClearTx();
        SW[SwFKStart].Off();
        CHECK(TxUntil("t050490000000\r", 2000));                                // P1-7c
        CHECK(SW[SwFKStart].Status() == false);                                 // P1-7d: golden quirk (Sw name, ProcessScanKey)
    }

    // [8] receive edges
    // AI(W906-ST02-P1b) 20261006: one receive call = one golden chunk, no buffer across chunks (header (3); Ifor01's !221 review)
    std::printf("[8] a bad frame is logged and skipped; t07 / t08 are dropped; one receive call = one golden chunk\n");
    {
        Frame("t050400ZZZZZZ\r");
        CHECK(Memo("[Recv Error]"));                                            // P1-8a: header (5)
        Frame("t050400000040\r");
        CHECK(Sen[SnFKStart].IsOn() == true);                                   // P1-8b: the next frame still works
        Frame("t050400000000\r");
        const unsigned long d0 = W906_PadDroppedT07T08();
        Frame("t070400000000\rt080400000000\r");
        CHECK(W906_PadDroppedT07T08() == d0 + 1);                               // P1-8c: header (1)+(3): one chunk = one golden call, unit = its first 3 chars (TrayStepMotor.cpp:410)
        Frame("t070400000000\r");
        Frame("t080400000000\r");
        CHECK(W906_PadDroppedT07T08() == d0 + 3);                               // P1-8c: two chunks = two calls
        Rx("t050400000040");                                                    // a chunk without CR: Main232 parses it once (Length 13 < 14 -> nothing) and drops it
        Ticks(3, 1);
        Frame("\r");                                                            // so this CR completes nothing (the old cross-chunk buffer did)
        CHECK(Sen[SnFKStart].IsOn() == false);                                  // P1-8d: no buffer across chunks
        Frame("t050400000040\rt050400000000");                                  // a CR-less tail after the last CR
        CHECK(Sen[SnFKStart].IsOn() == true);
        Frame("\r");
        CHECK(Sen[SnFKStart].IsOn() == true);                                   // P1-8e: the tail was dropped unparsed (golden do...while uPadInterface.cpp:716-745)
        Frame("t050400000000\r");
        CHECK(Sen[SnFKStart].IsOn() == false);
        const std::string noise(4000, 'x');
        for (int i = 0; i < 16; ++i) Rx(noise.c_str());                         // 64,000 bytes without CR (wrong port / noise)
        Ticks(3, 1);
        CHECK(W906_PadRxQueuedBytesForTest() == 0);                             // P1-8f: nothing is kept between ticks (the old buffer had no bound)
        Frame("t050400000040\r");
        CHECK(Sen[SnFKStart].IsOn() == true);                                   // P1-8g: the next frame works right after the noise
        Frame("t050400000000\r");
    }

    // [9] the version poll and ResetComm (golden Main232 :899-937)
    std::printf("[9] the version poll (~10 s), its reply, and ResetComm when the port is lost\n");
    {
        ClearTx();
        CHECK(TxUntil("t051120\r", 13000));                                     // P1-9a: golden :396-415
        Frame("t051120\r");
        CHECK(fPadInterface->bRequestVer == true);                              // P1-9b: golden :729-732
        ClearTx();
        CHECK(TxUntil("t051120\r", 13000));                                     // the next cycle
        fPadInterface->MemoLines.clear();
        fPadInterface->bRs232Ok = false;                                        // the port is lost while case 20 waits
        Ticks(3, 2);
        CHECK(Memo("[Disconnect]") && Memo("[Connect]") && fPadInterface->bRs232Ok == true);   // P1-9c: golden :920-925 / :378-383
        CHECK(Port()->CommName == AnsiString("\\\\.\\COM18"));                  // P1-9d
    }

    // [10] source pins
    std::printf("[10] source pins: the gates are open, the entries are called\n");
    if (argc > 1) {
        const std::string root = argv[1];
        const std::string ms = ReadSource(root, "mysensor.cpp"), mw = ReadSource(root, "myswitch.cpp");
        CHECK(LiveLines(ms, "if(iControlPanelMode==1 && fPadInterface->IsPadKey(Name))") == 3);      // P1-10a
        CHECK(LiveLines(mw, "if(iControlPanelMode==1 && fPadInterface->IsPadButton(Name))") == 3);   // P1-10b
        CHECK(LiveLines(ReadSource(root, "cinitial.cpp"), "W906_PadPortRS232Init(HSys.TrayStepMotor_ComPort)") == 1);   // P1-10c
        CHECK(LiveLines(ReadSource(root, "rs232.cpp"), "W906_RS232InitPadCheck(flag)") == 1);       // P1-10d
        CHECK(LiveLines(ReadSource(root, "FastClockWbServe.cpp"), "::W906_PadFastClockAdd(g_clock, jobs)") == 1);   // P1-10e
    } else
        std::printf("  (skipped: no tree root given)\n");

    iControlPanelMode = 0;  InitialOK = false;  SystemInitialOK = false;
    std::printf("St02_PadInterface: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
