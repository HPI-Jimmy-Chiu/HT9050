// test_d028_jamday_body.cpp -- AI(W906-D028) 20261001 St01: todo D-028 (census 129 (e) E-FT2-011).
//   golden RUN_INFO::AddAlarm (V912 cprod.cpp:1043-1047) saves yesterday's JamRate_Daily file on the first JAM of a new
//   day. The laptop's half (a4ccc040) made cprod.cpp:1070 RUN_INFO::SaveJamRateByDay call W906_SaveJamRateByDayBody
//   when one is installed (ctest JamDayHook pins that). St01's half installs FileRW/MainClose.cpp's golden-faithful
//   W906_RunInfo_SaveJamRateByDay as that body, at boot, from W906_FRW_InstallSaveRunMode (same bHandlerModel
//   precondition as SaveRunMode).
// FileRW/MainClose.cpp is compiled only into wb_serve (its dependencies are wb_serve-only), so this test is a source
//   ratchet over the real file (argv[1] = port root, read only). It writes nothing: no D:\HT9045_Log file is touched.
//   Line endings are normalised ('\r' dropped) so a CRLF checkout (the gate worktree, core.autocrlf=true) passes too.
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

static int g_total = 0, g_fail = 0;
static void Check(bool ok, const char* what)
{
    ++g_total;
    if (ok) std::printf("  PASS: %s\n", what);
    else { ++g_fail; std::printf("  FAIL: %s\n", what); }
}

static bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    const std::string raw = ss.str();
    out->clear();
    for (std::string::size_type i = 0; i < raw.size(); ++i) if (raw[i] != '\r') out->push_back(raw[i]);
    return true;
}

// the text of the function body that starts at the first occurrence of `head` (up to the matching closing brace)
static std::string FunctionBody(const std::string& src, const std::string& head)
{
    const std::string::size_type h = src.find(head);
    if (h == std::string::npos) return std::string();
    const std::string::size_type open = src.find('{', h);
    if (open == std::string::npos) return std::string();
    int depth = 0;
    for (std::string::size_type i = open; i < src.size(); ++i) {
        if (src[i] == '{') ++depth;
        else if (src[i] == '}' && --depth == 0) return src.substr(open, i - open + 1);
    }
    return std::string();
}

int main(int argc, char** argv)
{
    if (argc < 2) { std::printf("usage: test_d028_jamday_body <port root>\n"); return 2; }
    const std::string root = argv[1];
    std::string mc, cp;
    Check(ReadAll(root + "/FileRW/MainClose.cpp", &mc), "[0] read FileRW/MainClose.cpp");
    Check(ReadAll(root + "/cprod.cpp", &cp), "[0] read cprod.cpp");

    std::printf("[1] the laptop's half is present (cprod.cpp:1070 hook + call)\n");
    Check(cp.find("void (*W906_SaveJamRateByDayBody)(bool) = 0;") != std::string::npos, "[1] cprod.cpp defines W906_SaveJamRateByDayBody (default 0)");
    Check(cp.find("if(W906_SaveJamRateByDayBody!=0) W906_SaveJamRateByDayBody(bUpload);") != std::string::npos,
          "[1] RUN_INFO::SaveJamRateByDay calls the installed body");

    std::printf("[2] St01's install in W906_FRW_InstallSaveRunMode\n");
    const std::string inst = FunctionBody(mc, "void W906_FRW_InstallSaveRunMode()");
    Check(!inst.empty(), "[2] W906_FRW_InstallSaveRunMode found");
    const std::string::size_type guard = inst.find("if (bHandlerModel == false)");
    const std::string::size_type ret   = inst.find("return;", guard == std::string::npos ? 0 : guard);
    const std::string::size_type jam   = inst.find("W906_SaveJamRateByDayBody=[](bool bUpload){ (void)W906_RunInfo_SaveJamRateByDay(bUpload); };");
    Check(guard != std::string::npos && ret != std::string::npos, "[2] the bHandlerModel==false early return is still there");
    Check(jam != std::string::npos, "[2] installs W906_RunInfo_SaveJamRateByDay as W906_SaveJamRateByDayBody");
    Check(jam != std::string::npos && ret != std::string::npos && jam > ret,
          "[2] the install comes AFTER the bHandlerModel==false return (same precondition as SaveRunMode)");
    Check(inst.find("W906_SaveRunModeBody=&W906_TfMain_SaveRunMode;") != std::string::npos, "[2] SaveRunMode install unchanged");
    Check(mc.find("extern void (*W906_SaveJamRateByDayBody)(bool);") != std::string::npos, "[2] the hook is declared extern (cprod.cpp's definition)");

    std::printf("[3] the body itself: local file only, upload still gated\n");
    const std::string body = FunctionBody(mc, "AnsiString W906_RunInfo_SaveJamRateByDay(bool bUpload)");
    Check(!body.empty(), "[3] W906_RunInfo_SaveJamRateByDay found");
    Check(body.find("MyForceDirectories(sDailyJamPath);") != std::string::npos && body.find("slReport->SaveToFile(RunInfo.DailyJamFileName);") != std::string::npos,
          "[3] it writes <sDailyJamPath>\\..._DailyJamRate.txt (golden cprod.cpp:995-1110)");
    const std::string::size_type gate = body.find("#if 0 // GATE (S95-J1)");
    const std::string::size_type ftp  = body.find("FormHS->UpDataToServerByFTP(");
    Check(gate != std::string::npos && ftp != std::string::npos && ftp > gate, "[3] the FTP / N10 upload is still inside #if 0 GATE (S95-J1)");

    std::printf("D028_JamDayBody: %d checks, %d failed\n", g_total, g_fail);
    return g_fail == 0 ? 0 : 1;
}
