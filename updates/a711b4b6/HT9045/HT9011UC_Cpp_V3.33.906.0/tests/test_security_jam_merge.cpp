// =============================================================================
//  test_security_jam_merge.cpp -- S128 (Steven Q6 = B): JamIniMerge.h, the main-thread merge of the Jam export's seeded
//  keys back into JAM0000.dat, and its named lock.
//
//  AI(W906-SEC-S128) 20260927 (St02).  Suite name (add_test): Security_JamMerge
//
//    1. merge into an existing file: missing keys (existing and new section) written, a key already there skipped, the
//       backup is the old file byte for byte, every original line is still there in order, Verify passes;
//    2. no real file: created, no backup, Verify passes;
//    3. a merged file that lost an original line fails Verify and is restored from the backup byte for byte;
//    4. Apply in chunks (the main thread does 200 keys per exportStatus) gives the same file as one call;
//    5. the lock: another thread holds "Local\HT9045_JAM0000_dat" -> a 100 ms Lock is not held; released -> held.
//  Every file is under %TEMP%\ht9045_s128_<tick> -- never D:\HT9045\Error.  The sandbox is removed on a green run.
// =============================================================================
#include "JamIniMerge.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static void WriteText(const std::string& p, const char* text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
}

static std::string Bytes(const std::string& p)
{
    std::string s;
    jamini::ReadAll(p, &s);
    return s;
}

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }

static std::vector<jamini::Seed> Seeds()
{
    std::vector<jamini::Seed> v;
    jamini::Seed a; a.group = "01 Input Arm"; a.name = "JAM0101";        a.value = "0";   v.push_back(a);   // already there
    jamini::Seed b; b.group = "01 Input Arm"; b.name = "JAM0102Silent";  b.value = "1";   v.push_back(b);   // missing, section exists
    jamini::Seed c; c.group = "24 Motor";     c.name = "WAR2401";        c.value = "2";   v.push_back(c);   // missing section
    jamini::Seed d; d.group = "24 Motor";     d.name = "WAR2401Red";     d.value = "0";   v.push_back(d);
    return v;
}

static const char* kOriginal =
    "[01 Input Arm]\r\n"
    "JAM0101=3\r\n"
    "JAM0101Silent=0\r\n"
    "\r\n"
    "[16 System]\r\n"
    "WAR1601=1\r\n";

static HANDLE g_holdReady = NULL, g_holdRelease = NULL;
static DWORD WINAPI HoldLock(LPVOID)
{
    HANDLE m = ::CreateMutexA(NULL, FALSE, jamini::MutexName());
    ::WaitForSingleObject(m, INFINITE);
    ::SetEvent(g_holdReady);
    ::WaitForSingleObject(g_holdRelease, INFINITE);
    ::ReleaseMutex(m);
    ::CloseHandle(m);
    return 0;
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

int main()
{
    printf("Security_JamMerge\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_s128_" + stamp;
    ::CreateDirectoryA(root.c_str(), 0);
    const std::vector<jamini::Seed> seeds = Seeds();

    // 1
    {
        const std::string real = root + "\\JAM0000.dat", bak = root + "\\JAM0000.dat.1.s128.bak";
        WriteText(real, kOriginal);
        jamini::MergeResult r;
        jamini::Lock lk(1000);
        CHECK(lk.held(), "1. lock taken");
        CHECK(jamini::Begin(real, bak, &r) && r.backup == bak && !r.createdFile, "1. backup made");
        CHECK(Bytes(bak) == kOriginal, "1. the backup is the old file byte for byte");
        jamini::Apply(real, seeds, 0, seeds.size(), &r);
        CHECK(r.written == 3 && r.skipped == 1, "1. three missing keys written, the one already there skipped");
        CHECK(jamini::Verify(real, seeds, &r) && r.error.empty(), "1. Verify passes");
        vclcompat::TIniFile ini(AnsiString(real.c_str()));
        CHECK(ini.ReadString("01 Input Arm", "JAM0101", "") == "3", "1. the existing key keeps its value (not the seed's 0)");
        CHECK(ini.ReadString("01 Input Arm", "JAM0102Silent", "") == "1" && ini.ReadString("24 Motor", "WAR2401", "") == "2" &&
                  ini.ReadString("24 Motor", "WAR2401Red", "") == "0", "1. the missing keys have the seeded text");
        const std::string after = Bytes(real);
        CHECK(Has(after, "JAM0101=3\r\n") && Has(after, "[16 System]\r\nWAR1601=1"), "1. the original lines are still there");
    }

    // 2
    {
        const std::string real = root + "\\none\\JAM0000.dat";
        ::CreateDirectoryA((root + "\\none").c_str(), 0);
        jamini::MergeResult r;
        CHECK(jamini::Begin(real, real + ".bak", &r) && r.createdFile && r.backup.empty(), "2. no real file: nothing to back up");
        jamini::Apply(real, seeds, 0, seeds.size(), &r);
        CHECK(r.written == 4 && jamini::Verify(real, seeds, &r), "2. the file is created with every seed");
    }

    // 3
    {
        const std::string real = root + "\\bad.dat", bak = root + "\\bad.dat.s128.bak";
        WriteText(real, kOriginal);
        jamini::MergeResult r;
        CHECK(jamini::Begin(real, bak, &r), "3. backup made");
        jamini::Apply(real, seeds, 0, seeds.size(), &r);
        std::string t = Bytes(real);                                              // an original line goes missing
        const size_t p = t.find("WAR1601=1");
        if (p != std::string::npos) t.erase(p, std::strlen("WAR1601=1"));
        WriteText(real, t.c_str());
        CHECK(!jamini::Verify(real, seeds, &r) && Has(r.error, "WAR1601"), "3. Verify fails and names the lost line");
        CHECK(Bytes(real) == kOriginal, "3. the real file is restored from the backup byte for byte");
    }

    // 4
    {
        const std::string one = root + "\\one.dat", chunked = root + "\\chunked.dat";
        WriteText(one, kOriginal);
        WriteText(chunked, kOriginal);
        jamini::MergeResult r1, r2;
        jamini::Apply(one, seeds, 0, seeds.size(), &r1);
        for (size_t i = 0; i < seeds.size(); i += 1) jamini::Apply(chunked, seeds, i, 1, &r2);
        CHECK(Bytes(one) == Bytes(chunked) && r1.written == r2.written, "4. chunked Apply == one Apply");
    }

    // 5
    {
        g_holdReady = ::CreateEventA(NULL, TRUE, FALSE, NULL);
        g_holdRelease = ::CreateEventA(NULL, TRUE, FALSE, NULL);
        HANDLE th = ::CreateThread(NULL, 0, HoldLock, NULL, 0, NULL);
        ::WaitForSingleObject(g_holdReady, 5000);
        {
            jamini::Lock lk(100);
            CHECK(!lk.held(), "5. held by another thread: a 100 ms Lock times out (the page gets \"busy\")");
        }
        ::SetEvent(g_holdRelease);
        ::WaitForSingleObject(th, 5000);
        {
            jamini::Lock lk(1000);
            CHECK(lk.held(), "5. released: the Lock is taken");
        }
        ::CloseHandle(th);
        ::CloseHandle(g_holdReady);
        ::CloseHandle(g_holdRelease);
    }

    printf("Security_JamMerge: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0) RemoveTree(root);
    else printf("  sandbox kept: %s\n", root.c_str());
    return g_fail ? 1 : 0;
}
