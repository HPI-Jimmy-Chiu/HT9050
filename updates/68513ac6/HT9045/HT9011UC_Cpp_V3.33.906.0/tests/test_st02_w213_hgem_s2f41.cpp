// =============================================================================
//  test_st02_w213_hgem_s2f41.cpp -- W-213 (3): SECSGEM/uHGemHT9045.cpp GATE G37 / G41 / G42 / G43 / G47 lifted (golden 913
//  SECSGEM/uHGemHT9045.cpp :3975-3989 / :4096 / :4105 / :4110 / :4390).
//
//  AI(W906-W213) 20261010 (St02-E).  Suite name (add_test): St02_W213HgemS2F41.  argv[1] = port root (reads SECSGEM/uHGemHT9045.cpp).
//  HT9045Gem::S2F42_Host_Command_Acknowledge is driven end to end by seeding the THGem codec's SReceiveData (the token form
//  tests/test_uHGemClass.cpp:513-525 uses for HTGem: LIST <type, count>, ASCII <type, len, text>).  In memory only: the START_LOT
//  lot-start click is still GATE G38, and nothing here writes a file.
//    [1] S2F41 START_LOT, CP TEST_TIMES "RP1" with RP0..RP2 in fLotInfo->cbTestTimes: HCACK 0, cbTestTimes->Text "RP1"
//        (gated it fell to the CP arm's final else: HCACK 1).
//    [2] TEST_TIMES "XX" (not in the list) -> HCACK 3, Text unchanged; an empty list (non-VTEST) -> HCACK 3.
//    [3] S2F41 START_AGV Loader / Empty / Color "Action": bLoaderSECSActionFlag[i] true and iLoaderTask[i] back to 1 (the test sets
//        it to 0 first); HCACK 0.  iLoaderTask has no reader in the port (golden TfLotInfo::LoaderAction is not ported).
//    [4] G47: the START arm's CheckActionFlag line is live (`#if 1` gate line, call inside it), and CheckActionFlag copies
//        bAMRReceiveStart to ledSTART (forms/fLotInfo.cpp:1858-1870).  Driven directly: the START arm's RecordProcess writes the
//        process log, so it is not run here.
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"     // THGem, extern THGem *HGem
#include "SECSGEM/uHGemHT9045.h"        // HT9045Gem
#include "forms/fLotInfo.h"             // fLotInfo (cbTestTimes, CheckActionFlag, ledSTART)
#include "cmydef.h"                     // bLoaderSECSActionFlag, bAMRReceiveStart (cmydef_rt.h)

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern int iLoaderTask[3];              // forms/fLotInfo.cpp:149 (golden uLotInfo.cpp file-scope global)

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
void L(THGem& g, int n) { g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE)); g.WireCodec.SReceiveData->Add(AnsiString(n)); }
void A(THGem& g, const char* s)
{
    g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
    g.WireCodec.SReceiveData->Add(AnsiString((int)std::string(s).size()));
    g.WireCodec.SReceiveData->Add(AnsiString(s));
}
// <L,2 <A rcmd> <L,1 <L,2 <A name> <A value>>>>.  On a parsed RCMD the function returns 1 and sends HCACK in the S2F42 reply
// (uHGemHT9045.cpp:8010-8016: <L,2 <B HCACK> <L,0>>), so the HCACK is read back from the codec's outbound buffer: reset the encode
// cursor first (LocalLength_4 = 4, SecsWireCodec.cpp:404-408), then the first BINARY item (0x21, length 1) holds it.
int Rcmd(THGem& g, HT9045Gem& h, const char* rcmd, const char* name, const char* value)
{
    g.WireCodec.SReceiveData->Clear();
    L(g, 2); A(g, rcmd); L(g, 1); L(g, 2); A(g, name); A(g, value);
    g.WireCodec.ResetLocalBuffer();
    const int ret = h.S2F42_Host_Command_Acknowledge();
    const std::vector<unsigned char>& b = g.WireCodec.LocalBuffer;
    for (size_t i = 4; i + 2 < b.size() && i < 64; ++i)
        if (b[i] == 0x21 && b[i + 1] == 0x01) return b[i + 2];
    std::printf("    (no BINARY HCACK item in the reply; function returned %d)\n", ret);
    return -1;
}
std::string Txt(const AnsiString& a) { return std::string(a.c_str()); }
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W213HgemS2F41 -- uHGemHT9045.cpp G37 / G41-43 / G47 lifted (golden 913)\n");
    const std::string root = argc > 1 ? argv[1] : std::string();

    THGem gem;
    HGem = &gem;
    HT9045Gem h(AnsiString(""), &gem);
    h.ActiveWire = &gem.WireCodec;
    Check(fLotInfo != 0 && fLotInfo->cbTestTimes != 0, "setup: fLotInfo and its cbTestTimes exist (static facade)");

    // ---------------------------------------------------------------- [1] / [2]
    std::printf("[1] START_LOT TEST_TIMES in the list -- golden 913 :3975-3980\n");
    fLotInfo->cbTestTimes->Items->Clear();
    fLotInfo->cbTestTimes->Items->Add("RP0");
    fLotInfo->cbTestTimes->Items->Add("RP1");
    fLotInfo->cbTestTimes->Items->Add("RP2");
    fLotInfo->cbTestTimes->Text = "RP0";
    int hc = Rcmd(gem, h, "START_LOT", "TEST_TIMES", "RP1");
    Check(hc == 0 && Txt(fLotInfo->cbTestTimes->Text) == "RP1",
          "[1] HCACK 0 and cbTestTimes->Text \"RP1\" (got HCACK " + std::to_string(hc) + ", Text \"" + Txt(fLotInfo->cbTestTimes->Text) + "\")");

    std::printf("[2] TEST_TIMES not in the list / empty list -- golden 913 :3981-3987\n");
    hc = Rcmd(gem, h, "START_LOT", "TEST_TIMES", "XX");
    Check(hc == 3 && Txt(fLotInfo->cbTestTimes->Text) == "RP1",
          "[2] \"XX\" -> HCACK 3, Text unchanged (got " + std::to_string(hc) + ", \"" + Txt(fLotInfo->cbTestTimes->Text) + "\")");
    fLotInfo->cbTestTimes->Items->Clear();
    hc = Rcmd(gem, h, "START_LOT", "TEST_TIMES", "RP1");
    Check(hc == 3, "[2] empty list (a non-VTEST machine) -> HCACK 3 (got " + std::to_string(hc) + ")");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] START_AGV Loader / Empty / Color Action -- golden 913 :4096 / :4105 / :4110\n");
    {
        const char* name[3] = {"Loader", "Empty", "Color"};
        for (int i = 0; i < 3; ++i) {
            iLoaderTask[i] = 0;
            bLoaderSECSActionFlag[i] = false;
            hc = Rcmd(gem, h, "START_AGV", name[i], "Action");
            Check(hc == 0 && bLoaderSECSActionFlag[i] && iLoaderTask[i] == 1,
                  std::string("[3] ") + name[i] + " Action: HCACK 0, action flag set, iLoaderTask[" + std::to_string(i) + "] = 1 (got " +
                      std::to_string(hc) + ", " + std::to_string(iLoaderTask[i]) + ")");
        }
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] G47 -- golden 913 :4390 fLotInfo->CheckActionFlag()\n");
    {
        std::ifstream f((root + "/SECSGEM/uHGemHT9045.cpp").c_str(), std::ios::binary);
        std::ostringstream ss;
        ss << f.rdbuf();
        const std::string src = ss.str();
        const size_t g = src.find("\n#if 1   // ===== GATE G47");
        const size_t call = g == std::string::npos ? g : src.find("fLotInfo->CheckActionFlag();", g);
        const size_t els = g == std::string::npos ? g : src.find("\n#else", g);
        Check(g != std::string::npos && call != std::string::npos && els != std::string::npos && call < els,
              "[4] uHGemHT9045.cpp: the G47 gate line is `#if 1` and the CheckActionFlag call sits in its live arm (read " +
                  std::to_string(src.size()) + " bytes)");
        const bool saved = bAMRReceiveStart;
        bAMRReceiveStart = true;
        fLotInfo->CheckActionFlag();
        const bool on = fLotInfo->ledSTART->Value;
        bAMRReceiveStart = false;
        fLotInfo->CheckActionFlag();
        const bool off = fLotInfo->ledSTART->Value;
        bAMRReceiveStart = saved;
        Check(on && !off, "[4] CheckActionFlag copies bAMRReceiveStart to ledSTART (true -> on, false -> off)");
    }

    HGem = NULL;
    std::printf("St02_W213HgemS2F41: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
