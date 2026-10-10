// =============================================================================
//  test_st02_g12_ftp_rcmd.cpp -- W-223 G12 MR-1: SECSGEM/uHGemHT9045.cpp GATE G12 (S2F41 RCMD DOWNLOAD_RECIPE_BY_FTP) opened per
//  golden 913 SECSGEM/uHGemHT9045.cpp:2172-2298 behind two empty hooks (W906_SecsFtpBusyHook / W906_SecsFtpDownloadHook, defined
//  on uHGemHT9045.cpp:4472).
//
//  AI(W906-W223) 20261010 (St02).  Suite name (add_test): St02_G12FtpRcmd.  argv[1] = port root (reads SECSGEM/uHGemHT9045.cpp for
//  the source pin, read only).  HT9045Gem::S2F42_Host_Command_Acknowledge is driven end to end by seeding the THGem codec's
//  SReceiveData (the token form of tests/test_st02_w213_hgem_s2f41.cpp); the HCACK is read back from the codec's outbound buffer.
//  The hooks are FAKES that only count calls and record the setup-file name: no fFTPClient, no FTP engine, no network, no file.
//    [1] hook null (what every program has today)        -> HCACK 1 (final else, as while gated), the fake is never called
//    [2] hook set, nothing in the machine, Setup_File "ABC" -> HCACK 0, one download call with "ABC", bSecsGemDownloadFTP true
//    [3] CP name not Setup_File                           -> HCACK 9 (golden 913 :2282), no call
//    [4] busy hook true (golden fFTPClient->bControlBySECSGEM) -> HCACK 7 (:2176), no call
//    [5] IC in the machine (MOT[MMTrayZ].fHasTray)         -> HCACK 2 (:2180), no call
//    [6] no parameter list after the RCMD                  -> HCACK 10 (:2294)
//    [7] the pair is not <L,2 ...>                         -> HCACK 8 (:2287)
//    [8] empty parameter list <L,0>                        -> HCACK 0 and SECS_GEM_PPSIGNALTOWER_CONTROL_flag cleared (GOLDEN BUG (G12a),
//                                                             :2242-2246 -- 913 only; the 0618 parse answered 8)
//    [9] two Setup_File pairs                              -> two calls A then B (913's loop, GOLDEN BUG (G12c); 0618 read one pair)
//   [10] source pin: the G12 gate line is `#if 1`, the arm needs the hook, no `fFTPClient->` left in the live arm
//  Reverse check: with the gate back to `#if 0`, [2]-[9] fail (every case answers HCACK 1); with the 0618 parse, [8] and [9] fail;
//  without `&& W906_SecsFtpDownloadHook!=0` in the condition, [1] calls a null pointer.
//  Containment first: every log / config root the Handler could write through (common.h) must not resolve under D:\HT9045*
//  (D:\HT9045, D:\HT9045_Log, ...) unless it is inside a build dir (\obj\v906\), and must be ctest's scratch
//  (st02_test_containment.h); otherwise the test returns 2 before any Handler code runs.
//  Never sends a mistyped parameter list: GetDataItemLenAndTypeAndDelete writes the incoming type into the global HType.LIST_TYPE
//  (GOLDEN BUG (G12e)), which would corrupt the later cases.
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"     // THGem, extern THGem *HGem
#include "SECSGEM/uHGemHT9045.h"        // HT9045Gem
#include "cmydef.h"                     // bSecsGemDownloadFTP (cmydef_rt.h), MMTrayZ (cmydef_io.h)
#include "Motor/mymotor.h"              // MOT[] (TTrayMotor::fHasTray -> HasICUnderMachine)
#include "common.h"                     // as9045LogPath / asSaveEventLogPath / asProductionLogPath / sProductionInfoFilePath / asGeneralPath
#include "st02_test_containment.h"      // W906TestInsideCtestRoots

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern bool (*W906_SecsFtpBusyHook)();                                   // SECSGEM/uHGemHT9045.cpp:4472
extern void (*W906_SecsFtpDownloadHook)(const AnsiString& sSetupName);  // same line
extern bool SECS_GEM_PPSIGNALTOWER_CONTROL_flag;                         // ckernel.cpp (extern'd in S2F42_Host_Command_Acknowledge)

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

// ---- containment -------------------------------------------------------------------------------------------------------
std::string Lower(const std::string& p)
{
    std::string s(p);
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/') s[i] = '\\';
    }
    return s;
}
// true = usable: not under D:\HT9045* (that includes D:\HT9045_Log), or inside a build dir (\obj\v906\).
bool PathOk(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.compare(0, 9, "d:\\ht9045") != 0) return true;
    return s.find("\\obj\\v906\\") != std::string::npos;
}
bool Contained()
{
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath, &asGeneralPath };
    const char* const names[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath", "asGeneralPath" };
    bool ok = true;
    for (int i = 0; i < 5; ++i) {
        const bool good = PathOk(std::string(roots[i]->c_str()));
        if (!good) {
            std::printf("  %s = %s   <-- under D:\\HT9045*, not a build dir\n", names[i], roots[i]->c_str());
            ok = false;
        }
    }
    if (!ok) {
        std::printf("  ABORT: a Handler root points at the machine -- nothing was called\n");
        return false;
    }
    return W906TestInsideCtestRoots("St02_G12FtpRcmd");
}

// ---- fake hooks --------------------------------------------------------------------------------------------------------
int g_calls = 0;
std::vector<std::string> g_names;
bool g_busy = false;
bool FakeBusy() { return g_busy; }
void FakeDownload(const AnsiString& s) { ++g_calls; g_names.push_back(std::string(s.c_str())); }
void ResetFakes() { g_calls = 0; g_names.clear(); g_busy = false; bSecsGemDownloadFTP = false; }

// ---- S2F41 frames (token form: LIST <type, count>, ASCII <type, len, text>) ------------------------------------------------
void L(THGem& g, int n) { g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE)); g.WireCodec.SReceiveData->Add(AnsiString(n)); }
void A(THGem& g, const char* s)
{
    g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
    g.WireCodec.SReceiveData->Add(AnsiString((int)std::string(s).size()));
    g.WireCodec.SReceiveData->Add(AnsiString(s));
}
// Runs the handler on what is in SReceiveData and returns the HCACK of the S2F42 reply (uHGemHT9045.cpp final reply:
// <L,2 <B HCACK> <L,0>>): reset the encode cursor first, then the first BINARY item (0x21, length 1) holds it.
int Run(THGem& g, HT9045Gem& h)
{
    g.WireCodec.iReturnCode = 1;   // a fresh decode burst (golden resets it in ProcessReceiceData)
    g.WireCodec.ResetLocalBuffer();
    const int ret = h.S2F42_Host_Command_Acknowledge();
    const std::vector<unsigned char>& b = g.WireCodec.LocalBuffer;
    for (size_t i = 4; i + 2 < b.size() && i < 64; ++i)
        if (b[i] == 0x21 && b[i + 1] == 0x01) return b[i + 2];
    std::printf("    (no BINARY HCACK item in the reply; function returned %d)\n", ret);
    return -1;
}
// <L,2 <A DOWNLOAD_RECIPE_BY_FTP> <L,1 <L,2 <A name> <A value>>>>
int Download(THGem& g, HT9045Gem& h, const char* name, const char* value)
{
    g.WireCodec.SReceiveData->Clear();
    L(g, 2); A(g, "DOWNLOAD_RECIPE_BY_FTP"); L(g, 1); L(g, 2); A(g, name); A(g, value);
    return Run(g, h);
}
std::string N(int v) { return std::to_string(v); }
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_G12FtpRcmd -- uHGemHT9045.cpp GATE G12 DOWNLOAD_RECIPE_BY_FTP per golden 913 :2172-2298, hooks faked\n");
    if (!Contained())
        return 2;
    const std::string root = argc > 1 ? argv[1] : std::string();

    THGem gem;
    HGem = &gem;
    HT9045Gem h(AnsiString(""), &gem);
    h.ActiveWire = &gem.WireCodec;
    W906_SecsFtpBusyHook = 0;
    W906_SecsFtpDownloadHook = 0;

    // ---------------------------------------------------------------- [1]
    std::printf("[1] hook null -- the arm is off the ladder, final else\n");
    ResetFakes();
    int hc = Download(gem, h, "Setup_File", "ABC");
    Check(hc == 1 && g_calls == 0 && !bSecsGemDownloadFTP, "[1] HCACK 1 as while gated, no call (got HCACK " + N(hc) + ", calls " + N(g_calls) + ")");

    W906_SecsFtpDownloadHook = &FakeDownload;
    W906_SecsFtpBusyHook = &FakeBusy;

    // ---------------------------------------------------------------- [2]
    std::printf("[2] hook set, machine empty -- golden 913 :2263-2278\n");
    ResetFakes();
    hc = Download(gem, h, "Setup_File", "ABC");
    Check(hc == 0 && g_calls == 1 && g_names.size() == 1 && g_names[0] == "ABC" && bSecsGemDownloadFTP,
          "[2] HCACK 0, one download call with \"ABC\", bSecsGemDownloadFTP true (got HCACK " + N(hc) + ", calls " + N(g_calls) +
              (g_names.empty() ? std::string("") : ", name \"" + g_names[0] + "\"") + ")");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] CP name is not Setup_File -- :2280-2283\n");
    ResetFakes();
    hc = Download(gem, h, "Other", "ABC");
    Check(hc == 9 && g_calls == 0, "[3] HCACK 9, no call (got " + N(hc) + ", calls " + N(g_calls) + ")");

    // ---------------------------------------------------------------- [4]
    std::printf("[4] busy (golden fFTPClient->bControlBySECSGEM) -- :2174-2177\n");
    ResetFakes();
    g_busy = true;
    hc = Download(gem, h, "Setup_File", "ABC");
    Check(hc == 7 && g_calls == 0, "[4] HCACK 7, no call (got " + N(hc) + ", calls " + N(g_calls) + ")");

    // ---------------------------------------------------------------- [5]
    std::printf("[5] IC in the machine -- :2178-2181\n");
    ResetFakes();
    {
        const bool saved = MOT[MMTrayZ].fHasTray;
        MOT[MMTrayZ].fHasTray = true;
        hc = Download(gem, h, "Setup_File", "ABC");
        MOT[MMTrayZ].fHasTray = saved;
    }
    Check(hc == 2 && g_calls == 0, "[5] HCACK 2, no call (got " + N(hc) + ", calls " + N(g_calls) + ")");

    // ---------------------------------------------------------------- [6]
    std::printf("[6] no parameter list -- :2292-2295\n");
    ResetFakes();
    gem.WireCodec.SReceiveData->Clear();
    L(gem, 2); A(gem, "DOWNLOAD_RECIPE_BY_FTP");
    hc = Run(gem, h);
    Check(hc == 10 && g_calls == 0, "[6] HCACK 10, no call (got " + N(hc) + ", calls " + N(g_calls) + ")");

    // ---------------------------------------------------------------- [7]
    std::printf("[7] the pair is not <L,2 ...> -- :2285-2288\n");
    ResetFakes();
    gem.WireCodec.SReceiveData->Clear();
    L(gem, 2); A(gem, "DOWNLOAD_RECIPE_BY_FTP"); L(gem, 1); A(gem, "Setup_File");
    hc = Run(gem, h);
    Check(hc == 8 && g_calls == 0, "[7] HCACK 8, no call (got " + N(hc) + ", calls " + N(g_calls) + ")");

    // ---------------------------------------------------------------- [8]
    std::printf("[8] empty parameter list -- :2242-2246 (GOLDEN BUG (G12a), kept)\n");
    ResetFakes();
    SECS_GEM_PPSIGNALTOWER_CONTROL_flag = true;
    gem.WireCodec.SReceiveData->Clear();
    L(gem, 2); A(gem, "DOWNLOAD_RECIPE_BY_FTP"); L(gem, 0);
    hc = Run(gem, h);
    Check(hc == 0 && !SECS_GEM_PPSIGNALTOWER_CONTROL_flag && g_calls == 0,
          "[8] HCACK 0, PP signal-tower control flag cleared, no call (got " + N(hc) + ", flag " + N(SECS_GEM_PPSIGNALTOWER_CONTROL_flag) +
              ", calls " + N(g_calls) + ")");
    SECS_GEM_PPSIGNALTOWER_CONTROL_flag = false;

    // ---------------------------------------------------------------- [9]
    std::printf("[9] two Setup_File pairs -- :2249 loop (GOLDEN BUG (G12c))\n");
    ResetFakes();
    gem.WireCodec.SReceiveData->Clear();
    L(gem, 2); A(gem, "DOWNLOAD_RECIPE_BY_FTP"); L(gem, 2);
    L(gem, 2); A(gem, "Setup_File"); A(gem, "A");
    L(gem, 2); A(gem, "Setup_File"); A(gem, "B");
    hc = Run(gem, h);
    Check(hc == 0 && g_calls == 2 && g_names.size() == 2 && g_names[0] == "A" && g_names[1] == "B",
          "[9] HCACK 0, two calls A then B (got HCACK " + N(hc) + ", calls " + N(g_calls) + ")");

    // ---------------------------------------------------------------- [10]
    std::printf("[10] source pin -- SECSGEM/uHGemHT9045.cpp\n");
    {
        std::ifstream f((root + "/SECSGEM/uHGemHT9045.cpp").c_str(), std::ios::binary);
        std::ostringstream ss;
        ss << f.rdbuf();
        const std::string src = ss.str();
        const size_t g = src.find("\n#if 1   // ===== GATE G12 -- golden 913 SECSGEM/uHGemHT9045.cpp:2172-2298");
        const size_t els = g == std::string::npos ? g : src.find("\n#else", g);
        const std::string arm = (g == std::string::npos || els == std::string::npos) ? std::string() : src.substr(g, els - g);
        std::string code;   // the arm without its // comments (they quote golden's fFTPClient-> lines on purpose)
        {
            std::istringstream in(arm);
            std::string line;
            while (std::getline(in, line)) {
                const size_t c = line.find("//");
                code += (c == std::string::npos ? line : line.substr(0, c)) + "\n";
            }
        }
        Check(!code.empty() && code.find("else if(S.Pos(\"DOWNLOAD_RECIPE_BY_FTP\")!=0 && W906_SecsFtpDownloadHook!=0)") != std::string::npos &&
                  code.find("W906_SecsFtpDownloadHook(S3);") != std::string::npos && code.find("fFTPClient->") == std::string::npos,
              "[10] the G12 gate is `#if 1`, the arm needs the hook, calls it with S3, and has no fFTPClient-> left in code (read " +
                  N((int)src.size()) + " bytes)");
    }

    W906_SecsFtpBusyHook = 0;
    W906_SecsFtpDownloadHook = 0;
    HGem = NULL;
    std::printf("St02_G12FtpRcmd: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
