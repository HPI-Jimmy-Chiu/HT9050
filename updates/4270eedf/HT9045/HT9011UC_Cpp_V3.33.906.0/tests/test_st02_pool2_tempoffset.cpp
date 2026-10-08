// =============================================================================
//  test_st02_pool2_tempoffset.cpp -- POOL-2 MainTempMode.cpp:246: TfMain::ChangeTempMode re-reads the offsets after LastSet, as
//  golden 913 main.cpp:22756 (0618 :21904) `fOffSet->ReadFile();  //jou 2010-01-15 offset需放在lastset讀取之後`.
//
//  AI(W906-POOL2-TEMPOFS) 20261008 (St02-E).  Suite name (add_test): St02_Pool2TempOffset.  argv[1] = port root (source pin [3]).
//  Harness as tests/test_st02_w140_command.cpp (SETTEMP through TfMain::WriteSetTempStatus -> ChangeTempMode).  Every path is a
//  %TEMP%\ht9045_p2to_<tick> sandbox: DataPath / LastDataPath (SetUp.inf = "P2TO") / asTeachPath / OffsetPath; the offset files are
//  written here, ReadFile only reads them (and MyForceDirectories the recipe folder, which exists).  Removed when green.
//    [1] ambient -> SETTEMP 125 (hot): the sentinels seeded in memory are replaced from the files -- InArmOffSet_File[0] "Hand X" from
//        Position Offset.Data (first half, cOffSet.cpp:385-394), Offset_File.iSHRightPod[0] ("Test Arm1" / "Shuttle Right") from
//        Position Offset Hot.Data (second half picks the hot file when LastSet.iTemperature==Tempture_Hot, cOffSet.cpp:621-622).
//    [2] SETTEMP 20 (ambient): iSHRightPod[0] comes back from Position Offset.Data.
//    [3] source: MainTempMode.cpp's `fOffSet->ReadFile();` is not inside an `#if 0`.
// =============================================================================
#include "forms/fMain.h"
#include "forms/fOffSet.h"
#include "MessageDef.h"
#include "LastSet.h"
#include "cprod.h"
#include "cmydef.h"
#include "common.h"
#include "Config.h"
#include "CosFunction.h"
#include "csystem.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

void EnsureArmOffsetObjects();   // cOffSet.cpp:166 (forms/fOffSet.h:450), golden main.cpp:2123-2139

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
static bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0;
}
static std::string g_reply;
static void CaptureReply(int, AnsiString msg) { g_reply = msg.c_str(); }
static void Gpib(const char* text)
{
    static VM vm;
    std::memset(&vm, 0, sizeof(vm));
    std::strncpy(vm.cReturn, text, sizeof(vm.cReturn) - 1);
    HGpib2Handler = &vm;
    g_reply.clear();
}
static void Put(const std::string& path, const std::string& body)
{
    FILE* f = std::fopen(path.c_str(), "wb");
    if (f) { std::fputs(body.c_str(), f); std::fclose(f); }
}
static std::string Num(double v) { char b[32]; std::snprintf(b, sizeof(b), "%g", v); return b; }

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_Pool2TempOffset -- MainTempMode.cpp:246, ChangeTempMode re-reads the offsets (golden 913 main.cpp:22756)\n");
    const std::string src = argc > 1 ? argv[1] : std::string();
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    const std::string root = std::string(tmp) + "ht9045_p2to_" + std::to_string((unsigned long)::GetTickCount());
    const std::string ofs = root + "\\Offset\\P2TO";
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA((root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((root + "\\Data\\P2TO").c_str(), 0);
    ::CreateDirectoryA((root + "\\Offset").c_str(), 0);
    ::CreateDirectoryA(ofs.c_str(), 0);
    Put(root + "\\SetUp.inf", "P2TO\r\n");
    DataPath     = AnsiString((root + "\\Data\\").c_str());
    LastDataPath = AnsiString((root + "\\SetUp.inf").c_str());
    asTeachPath  = AnsiString((root + "\\teach.ini").c_str());
    OffsetPath   = AnsiString((root + "\\Offset\\").c_str());
    if (UnderMachineTree(DataPath.c_str()) || UnderMachineTree(asTeachPath.c_str()) || UnderMachineTree(OffsetPath.c_str()) ||
        UnderMachineTree(asGeneralPath.c_str()))
    {
        std::printf("  ABORT: a path is not in a sandbox (OffsetPath=%s general=%s) -- nothing was called\n", OffsetPath.c_str(), asGeneralPath.c_str());
        return 2;
    }
    const std::string sec = CapStrInput[0].c_str();
    Put(ofs + "\\Position Offset.Data", "[" + sec + "]\r\nHand X=1.25\r\n[Test Arm1]\r\nShuttle Right=11\r\n");
    Put(ofs + "\\Position Offset Hot.Data", "[Test Arm1]\r\nShuttle Right=22\r\n");

    OpenGeneralIniFile();
    EnsureArmOffsetObjects();
    W906_TesterForward.SendMSG_CMD_Msg = &CaptureReply;
    CosFunction.bLastSetInSetUpFile = false;
    CosFunction.bFTPDownLoadTempModeBySetupFile = false;
    CosFunction.bDLLCommands = false;
    CosFunction.bSaveOffsetByMachine = false;
    CosFunction.bUseInvisibleOffset = false;
    IniConfig.bE45_AllSetupFileUseOneFile = false;
    IniConfig.bE59GroupOffsetFile = false;
    IniConfig.iI38SETTEMPRespondSetTemp = 0;
    Temperature.bATC70Active = false;
    Tri_Temp_Machine = 0;
    SystemStart = false;
    Check(fOffSet != 0 && InArmOffSet_File[0] != 0 && !HasICUnderMachine() && !sec.empty(),
          "setup: fOffSet, InArmOffSet_File[0] allocated, no IC under the machine, section \"" + sec + "\"");

    // ---------------------------------------------------------------- [1]
    std::printf("[1] ambient -> SETTEMP 125\n");
    LastSet.iTemperature = Tempture_Ambient;
    InArmOffSet_File[0]->SetX(99.5);
    Offset_File.iSHRightPod[0] = 99;
    Gpib("125");
    fMain->WriteSetTempStatus();
    Check(LastSet.iTemperature == Tempture_Hot && g_reply == "SETTINGOK", "[1] fixture: ChangeTempMode switched to hot (reply " + g_reply + ")");
    Check(InArmOffSet_File[0]->GetX() == 1.25, "[1] InArmOffSet_File[0] Hand X re-read from Position Offset.Data (" + Num(InArmOffSet_File[0]->GetX()) + ", sentinel 99.5)");
    Check(Offset_File.iSHRightPod[0] == 22, "[1] Test Arm1 Shuttle Right from Position Offset Hot.Data (" + Num(Offset_File.iSHRightPod[0]) + ", sentinel 99)");

    // ---------------------------------------------------------------- [2]
    std::printf("[2] SETTEMP 20 -> ambient\n");
    Offset_File.iSHRightPod[0] = 99;
    Gpib("20");
    fMain->WriteSetTempStatus();
    Check(LastSet.iTemperature == Tempture_Ambient && g_reply == "SETTINGOK", "[2] fixture: back to ambient (reply " + g_reply + ")");
    Check(Offset_File.iSHRightPod[0] == 11, "[2] Test Arm1 Shuttle Right from Position Offset.Data again (" + Num(Offset_File.iSHRightPod[0]) + ")");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] source pin\n");
    {
        std::ifstream f((src + "/MainTempMode.cpp").c_str(), std::ios::binary);
        std::ostringstream ss;
        ss << f.rdbuf();
        const std::string s = ss.str();
        const size_t call = s.find("        fOffSet->ReadFile();");
        const size_t prevNl = call == std::string::npos ? std::string::npos : s.rfind('\n', call - 1);
        const size_t prevStart = prevNl == std::string::npos ? std::string::npos : s.rfind('\n', prevNl - 1);
        const std::string prev = (prevStart == std::string::npos) ? std::string() : s.substr(prevStart + 1, prevNl - prevStart - 1);
        Check(call != std::string::npos && prev.compare(0, 2, "//") == 0 && prev.find("GATE(dep-fOffSet-ReadFile)") != std::string::npos,
              "[3] MainTempMode.cpp: the line before fOffSet->ReadFile() is the retired gate comment, not a live #if 0");
    }

    W906_TesterForward.SendMSG_CMD_Msg = 0;
    HGpib2Handler = 0;
    LastSet.iTemperature = Tempture_Ambient;
    if (g_fail == 0) {
        ::DeleteFileA((ofs + "\\Position Offset.Data").c_str());
        ::DeleteFileA((ofs + "\\Position Offset Hot.Data").c_str());
        ::DeleteFileA((root + "\\SetUp.inf").c_str());
        ::RemoveDirectoryA(ofs.c_str());
        ::RemoveDirectoryA((root + "\\Offset").c_str());
        ::RemoveDirectoryA((root + "\\Data\\P2TO").c_str());
        ::RemoveDirectoryA((root + "\\Data").c_str());
        ::RemoveDirectoryA(root.c_str());
    }
    std::printf("St02_Pool2TempOffset: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
