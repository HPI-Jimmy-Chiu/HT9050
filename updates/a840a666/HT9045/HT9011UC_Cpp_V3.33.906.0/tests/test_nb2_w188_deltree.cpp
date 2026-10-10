// =============================================================================
//  AI(W906-W188) 20261010 (NB2-1): W-188 #11 -- Del_Tree = golden 913 (GitLab honprec/rd/rd5/ht9045_913 main e9908638)
//  csystem.cpp:24249-24305 (ht9045-v899 20260630): DeleteFile / RemoveDir are retried up to 3 times, 100 ms apart.
//  0618 tried each once: a file still held open (e.g. State Record's 7z) stayed, so its folder stayed too.
//  [1] a plain tree with a read-only file is removed.
//  [2] a file held open with no sharing, released ~120 ms later on another thread: the tree is still removed.
//  [3] source ratchets (argv[1] = source root).
//  Works only under %TEMP% (a fresh sub-folder per run).  Use: only through ctest (NB2_W188DelTree).
// =============================================================================
#include "csystem.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::vector<std::string> ReadLines(const std::string& path)
{
    std::vector<std::string> v;
    std::ifstream f(path.c_str(), std::ios::binary);
    std::string s;
    while (std::getline(f, s)) {
        if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
        v.push_back(s);
    }
    return v;
}
static int Count(const std::vector<std::string>& L, const std::string& n)
{
    int c = 0;
    for (size_t i = 0; i < L.size(); ++i) if (L[i].find(n) != std::string::npos) ++c;
    return c;
}
static bool DirExists(const std::string& p)
{
    DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}
static void WriteTxt(const std::string& p)
{
    std::ofstream f(p.c_str(), std::ios::binary);
    f << "nb2 w188 del_tree";
}
static std::string MakeTree(const std::string& root)
{
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA((root + "\\a").c_str(), 0);
    ::CreateDirectoryA((root + "\\a\\b").c_str(), 0);
    WriteTxt(root + "\\top.txt");
    WriteTxt(root + "\\a\\mid.txt");
    WriteTxt(root + "\\a\\b\\deep.txt");
    return root + "\\a\\b\\deep.txt";
}

static DWORD WINAPI ReleaseLater(LPVOID p)
{
    ::Sleep(120);
    ::CloseHandle((HANDLE)p);
    return 0;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("NB2_W188DelTree"))
        return 2;
    std::printf("==== W-188 #11 Del_Tree -> golden 913 ====\n");

    char tmp[MAX_PATH] = {0};
    ::GetTempPathA(MAX_PATH, tmp);
    char msg[300];
    const std::string base = std::string(tmp) + "nb2_w188_deltree_" + std::to_string((unsigned long)::GetCurrentProcessId());

    // ---------------------------------------------------------------- [1] plain tree + read-only file
    std::printf("-- [1] plain tree --\n");
    const std::string r1 = base + "_1";
    const std::string deep1 = MakeTree(r1);
    ::SetFileAttributesA(deep1.c_str(), FILE_ATTRIBUTE_READONLY);
    Del_Tree(AnsiString(r1.c_str()));
    std::snprintf(msg, sizeof msg, "tree with a read-only file removed (%s exists=%d)", r1.c_str(), (int)DirExists(r1));
    CHECK(!DirExists(r1), msg);

    // ---------------------------------------------------------------- [2] file held open, released ~120 ms later
    std::printf("-- [2] file in use, released during the retries --\n");
    const std::string r2 = base + "_2";
    const std::string deep2 = MakeTree(r2);
    HANDLE h = ::CreateFileA(deep2.c_str(), GENERIC_READ, 0 /*no sharing*/, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    CHECK(h != INVALID_HANDLE_VALUE, "deep.txt opened without sharing (simulated 7z / writer)");
    HANDLE th = ::CreateThread(0, 0, ReleaseLater, (LPVOID)h, 0, 0);
    const DWORD t0 = ::GetTickCount();
    Del_Tree(AnsiString(r2.c_str()));
    const DWORD dt = ::GetTickCount() - t0;
    ::WaitForSingleObject(th, 5000);
    ::CloseHandle(th);
    std::snprintf(msg, sizeof msg, "the in-use file was deleted on a retry and the whole tree is gone (exists=%d, %lu ms)", (int)DirExists(r2), (unsigned long)dt);
    CHECK(!DirExists(r2), msg);
    if (DirExists(r2)) Del_Tree(AnsiString(r2.c_str()));          // leave %TEMP% clean either way

    // ---------------------------------------------------------------- [3] source ratchets
    std::printf("-- [3] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> C = ReadLines(root + "/csystem.cpp");
    CHECK(Count(C, "for(int iSubRetry=0; iSubRetry<3; iSubRetry++) { if(RemoveDir(d+SearchRec.Name)) break; MySleep(100); }") == 1, "sub-folder RemoveDir retried");
    CHECK(Count(C, "AnsiString asDelFile=d+SearchRec.Name; for(int iDelRetry=0; iDelRetry<3; iDelRetry++) { FileSetAttr(asDelFile,faArchive); if(DeleteFile(asDelFile)) break; MySleep(100); }") == 1,
          "file DeleteFile retried (attribute reset each try)");
    CHECK(Count(C, "    for(int iRmRetry=0; iRmRetry<3; iRmRetry++) { if(RemoveDir(d)) break; MySleep(100); }") == 1, "final RemoveDir retried");

    std::printf("==== W-188 #11: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
