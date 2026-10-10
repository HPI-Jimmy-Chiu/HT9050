// =============================================================================
//  test_st02_l13_hgem_s14f4.cpp -- L13: S14F3 (2DID bin code) while there is IC in the machine or the machine is running answers
//  HCACK 2 and really sends S14F4 (golden 913 SECSGEM/uHGemHT9045.cpp:6396-6407, RogerYang 0922, AI(ht9045-vtest-secs)).
//
//  AI(W906-L13) 20261010 (St02).  Suite name (add_test): St02_L13HgemS14F4.  No arguments; no file is opened.
//  golden 0618 set HCACK=4 and returned WITHOUT any S14F4 (the host waits for T3) and leaked the 10 MB CommandStr; golden 913 sets
//  HCACK=2 (SEMI E5 "cannot perform now"), frees the buffer and sends <L[2] <B HCACK> <L[0]>>.  The port line is
//  SECSGEM/uHGemHT9045.cpp:4015 (HGemPtr -> ActiveWire, as the same function's normal send path :4103 / :4118).
//  Frames are captured by replacing the codec's SendLocalDataHook (SecsWireCodec.h:424) with a recorder, so THGem::SendLocalDataFrom
//  (sockets) is not reached.
//    [1] SystemStart true, no IC: exactly one S14F4 frame (S14 F4, W bit 0), body 01 02 21 01 02 01 00 = <L[2] <B 2> <L[0]>>.
//    [2] SystemStart false, HasICUnderMachine() true (MOT[MMTrayZ].fHasTray, csystem.cpp:13468): the same single frame, HCACK 2.
//    [3] Normal path unchanged (SystemStart false, no IC): an empty body -> one frame, HCACK 1; a well-formed SUBSTRATETYPE body ->
//        one frame, HCACK 3 (GATE [D2] default, uHGemHT9045.cpp:4077) and the inbound tokens are consumed.
//  Reverse: put `HCACK=4;` back on :4015 -> [1] / [2] see no frame -> red.
//
//  Containment: every runtime log root the Handler could write through (common.h) is checked first; if one resolves under
//  D:\HT9045* (that includes D:\HT9045_Log) and is not inside a build dir (\obj\v906\), the test returns 2 before any Handler code.
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"     // THGem, extern THGem *HGem
#include "SECSGEM/uHGemHT9045.h"        // HT9045Gem
#include "SECSGEM/SecsWireCodec.h"      // SecsWireCodec::SendLocalDataHook, LocalBuffer, LocalLength_4
#include "csystem.h"                    // HasICUnderMachine
#include "cmydef.h"                     // SystemStart (cmydef_core.h:220)
#include "MachineType.h"
#include "common.h"                     // as9045LogPath, asSaveEventLogPath, asProductionLogPath, sProductionInfoFilePath, asGeneralPath
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"   // Motor/HTMotor.h virtual defaults (not this file's code; mymotor.h pulls it in)
#include "Motor/mymotor.h"              // MOT[], MMTrayZ
#pragma GCC diagnostic pop

#include <cstdio>
#include <string>
#include <vector>

#ifdef _WIN32
extern "C" char* _fullpath(char* absPath, const char* relPath, size_t maxLength);   // msvcrt; <stdlib.h> hides it under -std=c++17
#endif

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

// true = the path may be used: not under D:\HT9045* (D:\HT9045, D:\HT9045_Log, ...), or inside a build dir (\obj\v906\).
bool PathAllowed(const char* p)
{
    std::string s(p ? p : "");
#ifdef _WIN32
    char full[1024];
    if (!s.empty() && _fullpath(full, s.c_str(), sizeof full) != NULL)   // "resolves under": relative paths against the cwd
        s = full;
#endif
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/')
            s[i] = '\\';
    }
    if (s.compare(0, 9, "d:\\ht9045") != 0)
        return true;
    return s.find("\\obj\\v906\\") != std::string::npos;
}

bool Contained()
{
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath,
                                        &asGeneralPath };
    const char* const names[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath",
                                  "asGeneralPath" };
    bool okAll = true;
    for (int i = 0; i < 5; ++i)
    {
        const bool ok = PathAllowed(roots[i]->c_str());
        std::printf("  %s = %s%s\n", names[i], roots[i]->c_str(), ok ? "" : "   <-- under D:\\HT9045*, not a build dir");
        if (!ok)
            okAll = false;
    }
    if (!okAll)
        std::printf("  REFUSED (exit 2) -- nothing was called.  Run it through ctest: ctest --test-dir <build dir> -R \"^St02_L13HgemS14F4$\"\n");
    return okAll;
}

std::vector<std::vector<unsigned char> > g_frames;

void L(THGem& g, int n) { g.WireCodec.SReceiveData->Add(AnsiString((int)HType.LIST_TYPE)); g.WireCodec.SReceiveData->Add(AnsiString(n)); }
void A(THGem& g, const char* s)
{
    g.WireCodec.SReceiveData->Add(AnsiString((int)HType.ASCII_TYPE));
    g.WireCodec.SReceiveData->Add(AnsiString((int)std::string(s).size()));
    g.WireCodec.SReceiveData->Add(AnsiString(s));
}

// One S14F3 call; returns the number of frames sent.  Frame layout (SecsWireCodec::CreateLocalHead): [0..3] length, [4..5] device,
// [6] S | W bit, [7] F, [8] PType, [9] SType, [10..13] system bytes, [14..] body.
size_t Call(HT9045Gem& h)
{
    g_frames.clear();
    h.S14F4_Get2DID_BinCode();
    return g_frames.size();
}

std::string Hex(const std::vector<unsigned char>& b, size_t from)
{
    std::string s;
    char t[4];
    for (size_t i = from; i < b.size(); ++i) { std::snprintf(t, sizeof t, "%02X ", b[i]); s += t; }
    return s;
}

// true = the one captured frame is S14F4 (W bit 0) with body <L[2] <B hcack> <L[0]>>.
bool IsS14F4(int hcack, std::string& got)
{
    if (g_frames.size() != 1) { got = std::to_string(g_frames.size()) + " frames"; return false; }
    const std::vector<unsigned char>& f = g_frames[0];
    if (f.size() < 14) { got = "short frame"; return false; }
    got = "S" + std::to_string(f[6] & 0x7f) + " F" + std::to_string(f[7]) + " W" + std::to_string((f[6] & 0x80) ? 1 : 0) +
          " body " + Hex(f, 14);
    const unsigned char body[] = { 0x01, 0x02, 0x21, 0x01, (unsigned char)hcack, 0x01, 0x00 };
    if ((f[6] & 0x7f) != 14 || f[7] != 4 || (f[6] & 0x80) != 0 || f.size() != 14 + sizeof body)
        return false;
    for (size_t i = 0; i < sizeof body; ++i)
        if (f[14 + i] != body[i])
            return false;
    return true;
}

void CheckFrame(int hcack, const std::string& what)
{
    std::string got;
    const bool ok = IsS14F4(hcack, got);   // before the message is built (argument order is unspecified)
    Check(ok, what + " (got " + got + ")");
}
}  // namespace

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_L13HgemS14F4 -- S14F3 with IC in machine / running -> HCACK 2 + S14F4 (golden 913 uHGemHT9045.cpp:6396-6407)\n");
    if (!Contained())
        return 2;

    THGem gem;
    HGem = &gem;
    HT9045Gem h(AnsiString(""), &gem);
    h.ActiveWire = &gem.WireCodec;
    gem.WireCodec.SendLocalDataHook = [](SecsWireCodec& wc) {
        g_frames.push_back(std::vector<unsigned char>(wc.LocalBuffer.begin(), wc.LocalBuffer.begin() + wc.LocalLength_4));
    };

    const bool savedStart = SystemStart;
    const bool savedTrayZ = MOT[MMTrayZ].fHasTray;
    SystemStart = false;
    MOT[MMTrayZ].fHasTray = false;
    Check(HasICUnderMachine() == false, "setup: HasICUnderMachine() false on the empty offline grid");

    // ---------------------------------------------------------------- [1]
    std::printf("[1] SystemStart true -- golden 913 :6396-6406\n");
    SystemStart = true;
    gem.WireCodec.SReceiveData->Clear();
    Call(h);
    CheckFrame(2, "[1] exactly one S14F4 <L[2] <B 2> <L[0]>>");
    SystemStart = false;

    // ---------------------------------------------------------------- [2]
    std::printf("[2] HasICUnderMachine() true (Tray Z has a tray) -- golden 913 :6396-6406\n");
    MOT[MMTrayZ].fHasTray = true;
    Check(HasICUnderMachine() == true, "[2] setup: MOT[MMTrayZ].fHasTray -> HasICUnderMachine() true");
    gem.WireCodec.SReceiveData->Clear();
    L(gem, 1); L(gem, 2); A(gem, "PPID1"); L(gem, 1); L(gem, 2); A(gem, "SUBSTRATETYPE"); A(gem, "<xml/>");
    Call(h);
    CheckFrame(2, "[2] exactly one S14F4 <L[2] <B 2> <L[0]>>");
    MOT[MMTrayZ].fHasTray = false;

    // ---------------------------------------------------------------- [3]
    std::printf("[3] normal path unchanged (SystemStart false, no IC) -- golden 913 :6408-6501\n");
    Check(HasICUnderMachine() == false && SystemStart == false, "[3] setup: no IC, not running");
    gem.WireCodec.SReceiveData->Clear();
    Call(h);
    CheckFrame(1, "[3] empty body -> one S14F4, HCACK 1");
    gem.WireCodec.SReceiveData->Clear();
    L(gem, 1); L(gem, 2); A(gem, "PPID1"); L(gem, 1); L(gem, 2); A(gem, "SUBSTRATETYPE"); A(gem, "<xml/>");
    Call(h);
    CheckFrame(3, "[3] SUBSTRATETYPE body -> one S14F4, HCACK 3 (GATE [D2] default)");
    Check(gem.WireCodec.SReceiveData->Count == 0,
          "[3] the inbound tokens are consumed (left " + std::to_string(gem.WireCodec.SReceiveData->Count) + ")");

    SystemStart = savedStart;
    MOT[MMTrayZ].fHasTray = savedTrayZ;
    gem.WireCodec.SendLocalDataHook = nullptr;
    HGem = NULL;
    std::printf("St02_L13HgemS14F4: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
