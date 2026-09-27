// =============================================================================
//  test_w1_art_brand.cpp -- Steven W1 = (b): the Auto Retest tester brand (0 = Flex, 1 = 93K) follows the recipe, and the
//                           Handler's own learning writes it back.
//
//  AI(W906-W1) 20260927 (St02).  Suite name (add_test): TesterComm_W1ArtBrand
//
//  Covers forms/fSCKART.cpp TfSCKART::AccessFile (golden 906_0625_Steven Automation/SCK_ART.cpp:190-209, the iTesterType key
//  only) and TesterComm/Handler/HandlerGpibMsg.cpp ProcessARTMessage LOTSTATUS / SRQMASK (golden 906_0625_Steven main.cpp
//  :15557-15599 / :15601-15612):
//    a. missing [AutoRetest] iTesterType -> 1 (Steven W1; golden 0), bAutoRetestGPIBmode on, and the read writes 1 back;
//    b. key present -> that value (0 and 1);
//    c. LOTSTATUS -> fSCKART 0, published 0 (HandlerSettings.h HsArtTesterType), written back 0;
//    d. SRQMASK   -> fSCKART 1, published 1, written back 1, bAutoRetestGPIBmode on;
//    e. CosFunction.bUseSCKART false -> no file access, value unchanged (golden :195).
//  Every file is under %TEMP%\ht9045_w1_<tick>: the test points DataPath / LastDataPath there itself and aborts before any
//  call if DataPath is still under D:\HT9045.  The sandbox is removed on a green run.
// =============================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "TesterComm/HandlerSettings.h"
#include "forms/fSCKART.h"
#include "forms/fMain.h"
#include "MessageDef.h"
#include "common.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "Config.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::string g_root, g_recipeFile;

static void WriteText(const std::string& p, const char* text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
}

static bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

// the key as text, "<none>" when it is not in the file
static std::string KeyText()
{
    char buf[64];
    ::GetPrivateProfileStringA("AutoRetest", "iTesterType", "<none>", buf, sizeof(buf), g_recipeFile.c_str());
    return buf;
}

static void SetKey(const char* v) { ::WritePrivateProfileStringA("AutoRetest", "iTesterType", v, g_recipeFile.c_str()); }

static bool Process(unsigned cmd)
{
    VM vm;
    std::memset(&vm, 0, sizeof(vm));
    vm.iCommand = cmd;
    HGpib2Handler = &vm;
    THandlerTesterSide side;
    const bool r = side.ProcessARTMessage();
    HGpib2Handler = 0;
    return r;
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

static bool UnderMachineTree(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return s.compare(0, 9, "d:\\ht9045") == 0;
}

int main()
{
    printf("TesterComm_W1ArtBrand\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    g_root = std::string(tmp) + "ht9045_w1_" + stamp;
    ::CreateDirectoryA(g_root.c_str(), 0);
    ::CreateDirectoryA((g_root + "\\Data").c_str(), 0);
    WriteText(g_root + "\\SetUp.inf", "W1R\r\n");
    DataPath     = AnsiString((g_root + "\\Data\\").c_str());
    LastDataPath = AnsiString((g_root + "\\SetUp.inf").c_str());
    g_recipeFile = std::string(GetRecipeFileName("Tester.Data").c_str());

    // containment first: nothing below may run against the machine's recipes
    if (UnderMachineTree(std::string(DataPath.c_str())) || UnderMachineTree(g_recipeFile) ||
        g_recipeFile.compare(0, g_root.size(), g_root) != 0)
    {
        printf("  ABORT: the recipe path is not in the sandbox (DataPath=%s, file=%s) -- nothing was called\n",
               DataPath.c_str(), g_recipeFile.c_str());
        return 2;
    }
    CHECK(g_recipeFile == g_root + "\\Data\\W1R\\Tester.Data", "0. the recipe file is <sandbox>\\Data\\W1R\\Tester.Data");

    CHECK(fSCKART != 0 && fMain != 0, "0. the fSCKART / fMain facades exist");
    CosFunction.bUseSCKART = true;
    IniConfig.bA10_AutoReTest = false;          // LOTSTATUS / QTY would forward to the bridge otherwise (golden, not under test)

    // a. missing key
    fSCKART->iTesterType = 0;
    CosFunction.bAutoRetestGPIBmode = false;
    fSCKART->AccessFile(true);
    CHECK(fSCKART->iTesterType == 1, "a. missing key -> 1 = 93K (Steven W1; golden 0)");
    CHECK(CosFunction.bAutoRetestGPIBmode, "a. iTesterType 1 -> bAutoRetestGPIBmode on (golden :205-206)");
    CHECK(KeyText() == "1", "a. the read wrote the default back (golden CheckAndReadIniData): iTesterType=1");

    // b. key present
    SetKey("0");
    fSCKART->iTesterType = 1;
    fSCKART->AccessFile(true);
    CHECK(fSCKART->iTesterType == 0 && !CosFunction.bAutoRetestGPIBmode, "b. key 0 -> 0 = Flex, bAutoRetestGPIBmode off");
    SetKey("1");
    fSCKART->AccessFile(true);
    CHECK(fSCKART->iTesterType == 1 && CosFunction.bAutoRetestGPIBmode, "b. key 1 -> 1 = 93K, bAutoRetestGPIBmode on");

    // c. LOTSTATUS? learns Flex
    testercomm::HsArtTesterType().store(-1);
    CHECK(Process(MSG_CMD_SCKART_LOTSTATUS), "c. LOTSTATUS handled");
    CHECK(fSCKART->iTesterType == 0, "c. LOTSTATUS -> fSCKART->iTesterType 0 (golden main.cpp:15560)");
    CHECK(testercomm::HsArtTesterType().load() == 0, "c. published to the engine before the reply returns (PublishSettings)");
    CHECK(KeyText() == "0", "c. written back: AccessFile(false, 0) (golden :15599) -> iTesterType=0");

    // d. SRQMASK learns 93K
    CosFunction.bAutoRetestGPIBmode = false;
    CHECK(Process(MSG_CMD_SCKART_SRQMASK), "d. SRQMASK handled");
    CHECK(fSCKART->iTesterType == 1 && CosFunction.bAutoRetestGPIBmode, "d. SRQMASK -> 1, bAutoRetestGPIBmode on (golden :15605-15610)");
    CHECK(testercomm::HsArtTesterType().load() == 1, "d. published 1");
    CHECK(KeyText() == "1", "d. written back: AccessFile(false, 10) (golden :15612) -> iTesterType=1");

    // e. no SCK ART: nothing is read or written
    ::DeleteFileA(g_recipeFile.c_str());
    CosFunction.bUseSCKART = false;
    fSCKART->iTesterType = 0;
    fSCKART->AccessFile(true);
    fSCKART->AccessFile(false, 0);
    CHECK(fSCKART->iTesterType == 0 && !Exists(g_recipeFile), "e. bUseSCKART false -> value unchanged, no file (golden :195)");

    printf("TesterComm_W1ArtBrand: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(g_root);
    else printf("  sandbox kept: %s\n", g_root.c_str());
    return g_fail ? 1 : 0;
}
