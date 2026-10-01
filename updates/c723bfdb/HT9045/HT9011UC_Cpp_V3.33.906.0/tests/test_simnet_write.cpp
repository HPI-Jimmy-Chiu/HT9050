// =============================================================================
//  test_simnet_write.cpp -- W58: the SIM mask's config.ini write-back is atomic.
//
//  AI(W906-W58) 20261001 (St02-E).  Suite name (add_test): SimNet_Write.  Built and run in both configurations (SIM
//  `build`, SHIP `build_ship`); the expectations are the same in both.  (The W906_SIM_NET_KEYS offer was dropped:
//  Jimmy RULINGS_20261001 #3 keeps N07-1 / N07-2 out of the mask instead -- see test_simnet_mask.cpp sections 1 / 4.)
//    0. containment first: the sandbox is %TEMP%\ht9045_simnet_w_<tick>, refused (exit 2, nothing written) under
//       D:\HT9045* / D:\RMS / D:\MTBF_Summary / D:\EventlogAnalyzer / D:\HandlerSummary; the machine's config.ini is
//       stat'ed read-only;
//    1. WriteRawValue: the same bytes vclcompat's in-place writer (TIniFile::WriteString) produced -- a value replaced
//       in place up to '=', a missing key added after the section's last key, a missing section appended -- and no
//       "<file>.simnet.tmp" is left behind;
//    2. a stale "<file>.simnet.tmp" from a crash is simply overwritten and removed;
//    3. RemoveKey: that one line, atomically; a missing key writes nothing;
//    4. the end: the machine's config.ini unchanged, the sandbox removed if green.
//    5. AI(W906-W58) 20261001 (St02-E) (E2 delta m2): the move fails -- cfg held open with FILE_SHARE_READ only (readable, but
//       it cannot be replaced) -> RemoveKey false / WriteRawValue writes nothing, no temp file left, the bytes unchanged.
//       (Share mode 0 would make the read fail first and never reach the move.)
//  Safe to run by hand: it writes nothing outside its own %TEMP% sandbox and calls no Handler code.
// =============================================================================
#include "SimNet/SimNetMask.h"

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>


static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                                     \
    do {                                                                                     \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }                            \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; }      \
        std::fflush(stdout);                                                                 \
    } while (0)

namespace {

std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/') s[i] = '\\';
    }
    return s;
}

bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    const char* const bad[] = { "d:\\ht9045", "d:\\rms", "d:\\mtbf_summary", "d:\\eventloganalyzer", "d:\\handlersummary" };
    for (std::size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i)
        if (s.compare(0, std::strlen(bad[i]), bad[i]) == 0) return true;
    return false;
}

std::string Bytes(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    std::size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    return s;
}

void Put(const std::string& p, const std::string& data)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (!f) return;
    std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
}

bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

struct Stamp
{
    bool exists;
    unsigned long long size, write;
};

Stamp StampOf(const char* p)
{
    Stamp s;
    s.exists = false;
    s.size = s.write = 0;
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (::GetFileAttributesExA(p, GetFileExInfoStandard, &a))
    {
        s.exists = true;
        s.size = ((unsigned long long)a.nFileSizeHigh << 32) | a.nFileSizeLow;
        s.write = ((unsigned long long)a.ftLastWriteTime.dwHighDateTime << 32) | a.ftLastWriteTime.dwLowDateTime;
    }
    return s;
}

bool SameStamp(const Stamp& a, const Stamp& b) { return a.exists == b.exists && a.size == b.size && a.write == b.write; }

void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

}  // namespace

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef W906_NO_SOFT_SIMULTE
    const bool sim = false;
#else
    const bool sim = true;
#endif
    std::printf("SimNet_Write (W58, %s build)\n", sim ? "SIM" : "SHIP");

    // ---- 0. containment first -----------------------------------------------------------------------------------------
    std::string root;
    {
        char tmp[MAX_PATH];
        const DWORD n = ::GetTempPathA(sizeof(tmp), tmp);
        char stamp[32];
        std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
        root = (n > 0 && n < sizeof(tmp)) ? std::string(tmp) + "ht9045_simnet_w_" + stamp : std::string();
        std::printf("  sandbox = %s\n", root.c_str());
        if (root.empty() || UnderMachineTree(root))
        {
            std::printf("  ABORT: the sandbox is not under %%TEMP%% outside the machine trees -- nothing was written\n");
            return 2;
        }
    }
    const char* const machine = "D:\\HT9045\\config\\config.ini";
    const Stamp before = StampOf(machine);
    ::CreateDirectoryA(root.c_str(), 0);
    const std::string cfg = root + "\\config.ini";
    const std::string tmpf = cfg + ".simnet.tmp";

    // ---- 1. WriteRawValue ---------------------------------------------------------------------------------------------
    std::printf("1. WriteRawValue (atomic, vclcompat's in-place rules)\n");
    {
        Put(cfg, "[FTP]\r\nEnable FTP = 1 \r\nHost=10.0.0.1\r\n\r\n[Event Log]\r\nbAlarmStatistAutoSaveNetDrive=0\r\n");
        simnet::WriteRawValue(cfg, "FTP", "Enable FTP", "0");
        CHECK(Bytes(cfg) == "[FTP]\r\nEnable FTP =0\r\nHost=10.0.0.1\r\n\r\n[Event Log]\r\nbAlarmStatistAutoSaveNetDrive=0\r\n",
              "1. a value replaced in place, the text up to '=' kept, every other byte unchanged");
        CHECK(!Exists(tmpf), "1. no <file>.simnet.tmp left behind");
        simnet::WriteRawValue(cfg, "ftp", "Second Enable FTP", "1");
        CHECK(Bytes(cfg) == "[FTP]\r\nEnable FTP =0\r\nHost=10.0.0.1\r\nSecond Enable FTP=1\r\n\r\n[Event Log]\r\nbAlarmStatistAutoSaveNetDrive=0\r\n",
              "1. a missing key: added after the section's last key (case-insensitive section)");
        simnet::WriteRawValue(cfg, "RMS", "RMS Enable", "1");
        CHECK(Bytes(cfg) == "[FTP]\r\nEnable FTP =0\r\nHost=10.0.0.1\r\nSecond Enable FTP=1\r\n\r\n[Event Log]\r\nbAlarmStatistAutoSaveNetDrive=0\r\n[RMS]\r\nRMS Enable=1\r\n",
              "1. a missing section: appended with its key");
        CHECK(!Exists(tmpf), "1. still no temp file");
        const std::string lf = root + "\\lf.ini";
        Put(lf, "[A]\nk1 = v one \n;k2=comment\nK2=two\n");
        simnet::WriteRawValue(lf, "A", "k1", "1");
        CHECK(Bytes(lf) == "[A]\nk1 =1\n;k2=comment\nK2=two\n", "1. LF lines untouched (the in-place writer's rule)");
        const std::string none = root + "\\none.ini";
        simnet::WriteRawValue(none, "N25", "bN25_3_EnableULJamLog", "0");
        CHECK(Bytes(none) == "[N25]\r\nbN25_3_EnableULJamLog=0\r\n", "1. a missing file is created with the section");
    }

    // ---- 2. a stale temp file -------------------------------------------------------------------------------------------
    std::printf("2. a stale temp file from a crash\n");
    {
        Put(tmpf, "garbage from an earlier crash");
        simnet::WriteRawValue(cfg, "FTP", "Enable FTP", "1");
        CHECK(Bytes(cfg).compare(0, 23, "[FTP]\r\nEnable FTP =1\r\nH") == 0 && !Exists(tmpf),
              "2. the stale temp file is overwritten, renamed over the file, gone afterwards");
    }

    // ---- 3. RemoveKey ---------------------------------------------------------------------------------------------------
    std::printf("3. RemoveKey (atomic)\n");
    {
        Put(cfg, "[FTP]\r\nEnable FTP=1\r\nHost=h\r\n[RMS]\r\nERMS Enable=0\r\n");
        CHECK(simnet::RemoveKey(cfg, "RMS", "ERMS Enable", true) && Bytes(cfg) == "[FTP]\r\nEnable FTP=1\r\nHost=h\r\n",
              "3. the key and its now-empty section removed");
        CHECK(!Exists(tmpf), "3. no temp file left");
        const Stamp s0 = StampOf(cfg.c_str());
        ::Sleep(20);
        CHECK(!simnet::RemoveKey(cfg, "FTP", "nope", false) && SameStamp(s0, StampOf(cfg.c_str())) && !Exists(tmpf),
              "3. a missing key: false, the file not written at all");
    }

    // ---- 5. the move fails (E2 delta m2) ---------------------------------------------------------------------------------
    std::printf("5. the move fails: the file stays as it was\n");
    {
        const std::string orig = "[FTP]\r\nEnable FTP=1\r\nHost=h\r\n[RMS]\r\nERMS Enable=0\r\n";
        Put(cfg, orig);
        HANDLE hold = ::CreateFileA(cfg.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        CHECK(hold != INVALID_HANDLE_VALUE, "5. cfg held open (read sharing only)");
        CHECK(!simnet::RemoveKey(cfg, "RMS", "ERMS Enable", true), "5. RemoveKey: false when the replace fails");
        CHECK(!Exists(tmpf), "5. RemoveKey: the temp file was removed");
        simnet::WriteRawValue(cfg, "FTP", "Host", "x");
        CHECK(!Exists(tmpf), "5. WriteRawValue: the temp file was removed");
        if (hold != INVALID_HANDLE_VALUE) ::CloseHandle(hold);
        CHECK(Bytes(cfg) == orig, "5. the bytes are unchanged");
    }

    // ---- 4. the end -------------------------------------------------------------------------------------------------------
    std::printf("4. the end\n");
    CHECK(SameStamp(before, StampOf(machine)), "4. the machine's D:\\HT9045\\config\\config.ini is unchanged (read-only stat)");
    std::printf("SimNet_Write: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(root);
    else std::printf("  sandbox kept: %s\n", root.c_str());
    return g_fail == 0 ? 0 : 1;
}
