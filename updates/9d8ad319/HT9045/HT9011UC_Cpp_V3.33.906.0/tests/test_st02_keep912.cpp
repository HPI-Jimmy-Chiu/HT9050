// =============================================================================
// test_st02_keep912.cpp -- ctest St02_Keep912.
// AI(W906-ST02-W71W73) 20261003 (St02-E): source pins for the three "Steven-approved V912 keeps" (RULINGS_20261003 #1 /
//   #23, the #20 exception = Steven's 1003 standing rule; Steven W8 / W72 / W73 = A, FROM_STEVEN §3 1003 14:39).
//   [1] W71 Qorvo Tester Pause buzzer: V912's delayed buzzer (RogerYang 20260626; 912 main.cpp:16371-16381 / :17023,
//       ckernel.cpp:744-748 / :2144-2148 / :2158-2162, uLotInfo.cpp:12548) instead of golden 0618's immediate one
//       (main.cpp:15806, ckernel.cpp:738 / :2113 / :2121).
//   [2] W72 AMD site map / ATC type: V912 main.cpp:18785 / :18834 / :19076 runtime `|| TestIF_File.i2DIDFormat==eAMD`
//       (Ifor 20260717) instead of golden 0618 main.cpp:18158-18168 / :18210-18220 / :18454-18464 (`#ifdef AMD_Version`,
//       not defined in the 906 build, `#else` Delta Castle only).
//   [3] W73 Murata 2DID-NG skip: V912 atester.cpp:1048 `AddTestResultRecord(iTestBinCount, "NonTestToRBin")` (the record
//       counts into the lot E1) after golden 0618 atester.cpp:1044-1047.
//  CODE ONLY: comments are stripped before counting, so a comment quoting the V912 line does not count.
//  argv[1] = the tree root (read only).  No machine state, no writes.
//  REVERSE CHECKS (expected; nothing is built or run on STEVEN-NB3 -- St01 runs it):
//    [1] drop `if(bPauseAlarmDelayActive && hPauseAlarmDelay.Off()) {...}` from ckernel.cpp:1638 -> CHECK [1]a red;
//        drop the re-arm from either Alarm Reset line (ckernel.cpp:3374 / :3382) -> [1]b red; drop the arm in
//        HandlerGpibMsg.cpp (:637) -> [1]c red; drop the clear in forms/fLotInfo.cpp:1964 or HandlerGpibMsg.cpp:1271 -> [1]d red.
//    [2] remove `|| TestIF_File.i2DIDFormat==eAMD` from any of HandlerBridgeCtl.cpp:487 / :543 / :797 -> [2]a red
//        (3 -> 2), and the Delta-Castle-only form is back -> [2]b red.
//    [3] comment out atester.cpp:1114 again -> [3] red.
// =============================================================================
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

static int g_total = 0, g_fail = 0;
#define CHECK(c) do { ++g_total; if (!(c)) { ++g_fail; std::printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

static std::string ReadSource(const std::string& root, const char* rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::ostringstream o;
    o << f.rdbuf();
    return o.str();
}

// Strip // and /* */ comments; keep string and char literals as they are.
static std::string CodeOnly(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ) {
        const char c = s[i];
        if (c == '/' && i + 1 < s.size() && s[i + 1] == '/') {
            while (i < s.size() && s[i] != '\n') ++i;
        } else if (c == '/' && i + 1 < s.size() && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/')) ++i;
            i += 2;
        } else if (c == '"' || c == '\'') {
            out += c;
            ++i;
            while (i < s.size() && s[i] != c && s[i] != '\n') {
                if (s[i] == '\\' && i + 1 < s.size()) { out += s[i]; ++i; }
                out += s[i];
                ++i;
            }
            if (i < s.size()) { out += s[i]; ++i; }
        } else {
            out += c;
            ++i;
        }
    }
    return out;
}

static int CountOf(const std::string& hay, const char* needle)
{
    int n = 0;
    const std::string nd(needle);
    for (size_t p = hay.find(nd); p != std::string::npos; p = hay.find(nd, p + nd.size())) ++n;
    return n;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::printf("usage: test_st02_keep912 <tree root>\n");
        return 2;
    }
    const std::string root = argv[1];
    const std::string ck = CodeOnly(ReadSource(root, "ckernel.cpp"));
    const std::string gm = CodeOnly(ReadSource(root, "TesterComm/Handler/HandlerGpibMsg.cpp"));
    const std::string li = CodeOnly(ReadSource(root, "forms/fLotInfo.cpp"));
    const std::string bc = CodeOnly(ReadSource(root, "TesterComm/Handler/HandlerBridgeCtl.cpp"));
    const std::string at = CodeOnly(ReadSource(root, "atester.cpp"));
    CHECK(!ck.empty() && !gm.empty() && !li.empty() && !bc.empty() && !at.empty());

    std::printf("[1] W71 Qorvo Tester Pause buzzer: V912's delayed buzzer\n");
    CHECK(CountOf(ck, "if(bPauseAlarmDelayActive && hPauseAlarmDelay.Off()) { bTesterPauseMusic=true; bPauseAlarmDelayActive=false; }") == 1);   // a
    CHECK(CountOf(ck, "if(bTesterSendPause && TestIF.iMaxTime>0) { hPauseAlarmDelay.SetSecAndOn(TestIF.iMaxTime); bPauseAlarmDelayActive=true; }") == 2);   // b
    CHECK(CountOf(gm, "hPauseAlarmDelay.SetSecAndOn(TestIF.iMaxTime);") == 1);                                                // c
    CHECK(CountOf(gm, "bPauseAlarmDelayActive=false;") >= 1 && CountOf(li, "bPauseAlarmDelayActive=false;") == 1);          // d

    std::printf("[2] W72 AMD: the runtime i2DIDFormat==eAMD term (V912)\n");
    CHECK(CountOf(bc, "if(TestIF.iGpibMode==InterfaceType_Delta_Castle || TestIF_File.i2DIDFormat==eAMD)") == 3);           // a
    CHECK(CountOf(bc, "if(TestIF.iGpibMode==InterfaceType_Delta_Castle)") == 0);                                              // b

    std::printf("[3] W73 Murata 2DID-NG skip writes the NonTestToRBin record (V912)\n");
    CHECK(CountOf(at, "TestSocket.PordRec[i][j].AddTestResultRecord(iTestBinCount, \"NonTestToRBin\");") == 1);

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
