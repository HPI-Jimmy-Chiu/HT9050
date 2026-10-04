// tests/w906_test_tmpname.h -- AI(W906-ST02-C17) 20261005 (St02-E): ST02-C17 / St01 todo E-039.  Tests that write scratch
//   files under %TEMP% with FIXED names collide when two ctest runs overlap (SIM + SHIP at once, or another session's gate):
//   one run overwrites or deletes the other's file and a test goes red at random (green when rerun alone).  The fix is
//   test-only: the scratch name gets a per-process tag ("_<pid>"; two processes alive at the same time never share a pid) and
//   the test removes what it made at the end.  Assertions do not change.
//   Header-only, C runtime only (no <windows.h>: some tests #undef DeleteFile for vclcompat and must not get the macro back).
//   tests/ is on the include path (like w906_ctest_guard.h), so tests/CMakeLists.txt is untouched.
#ifndef W906_TEST_TMPNAME_H
#define W906_TEST_TMPNAME_H

#include <cstdio>
#include <string>
#include <direct.h>    // _rmdir
#include <io.h>        // _findfirst / _findnext / _findclose, _chmod
#include <process.h>   // _getpid
#include <sys/stat.h>  // _S_IWRITE

// "_<pid>" for this process (the same value every call).
inline const std::string& W906_TestTmpTag()
{
    static const std::string tag = "_" + std::to_string(static_cast<long>(_getpid()));
    return tag;
}

// leaf + tag, the tag before the extension: "x.ini" -> "x_<pid>.ini", "dir" -> "dir_<pid>".
inline std::string W906_TestTmpName(const std::string& leaf)
{
    const std::string::size_type slash = leaf.find_last_of("\\/");
    const std::string::size_type dot = leaf.find_last_of('.');
    if (dot != std::string::npos && dot > 0 && (slash == std::string::npos || dot > slash + 1))
        return leaf.substr(0, dot) + W906_TestTmpTag() + leaf.substr(dot);
    return leaf + W906_TestTmpTag();
}

// Remove a scratch directory tree this test made: files first, then the directories.  Best effort, never throws.
inline void W906_TestTmpRemoveTree(std::string dir)
{
    while (!dir.empty() && (dir[dir.size() - 1] == '\\' || dir[dir.size() - 1] == '/'))
        dir.erase(dir.size() - 1);
    if (dir.size() < 4)                                    // never a drive root or an empty path
        return;
    _finddata_t fd;
    const auto h = _findfirst((dir + "\\*").c_str(), &fd);   // long on mingw.org, intptr_t on mingw-w64
    if (h != -1) {
        do {
            const std::string name = fd.name;
            if (name == "." || name == "..")
                continue;
            const std::string path = dir + "\\" + name;
            if (fd.attrib & _A_SUBDIR) {
                W906_TestTmpRemoveTree(path);
            } else {
                _chmod(path.c_str(), _S_IWRITE);
                std::remove(path.c_str());
            }
        } while (_findnext(h, &fd) == 0);
        _findclose(h);
    }
    _rmdir(dir.c_str());
}

#endif  // W906_TEST_TMPNAME_H
