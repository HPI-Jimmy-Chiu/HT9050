// =============================================================================
//  JamIniMerge.h -- S128 (Steven Q6 = B): merge the keys the background Jam export seeded into its snapshot back into
//  the real D:\HT9045\Error\English\JAM0000.dat on the main thread, and the one cross-thread lock for that file.
//
//  AI(W906-SEC-S128) 20260927 (St02, on St01's branch; RULINGS_20260926 S127／S128, St01 FROM_STEVEN §4 09:25／10:05).
//  Header-only on purpose: WebSecurityJam.cpp (wb_serve) and tests/test_security_jam_merge.cpp include it, so the
//  wb_serve source list in CMakeLists.txt does not change.
//
//  Why a merge at all: golden spbExportClick (V912 cSecurity.cpp:1587-1665) runs every getter's CheckAndReadIniData on the
//  real file, and a missing key is written with its default (common.cpp:432-491).  S54 moved the export to a background
//  thread that reads a %TEMP% snapshot, so those defaults landed only in the snapshot (S54-2).  Steven Q6 = B: the export
//  records every key it seeded (WebSecurityJam.cpp SnapRead), and the main thread writes the ones the real file still
//  lacks -- backup first, verify after, restore on failure.
//  Only MISSING keys are merged (the ruling's wording).  golden's other write in that path -- an existing key whose value
//  is "" gets the non-empty default (common.cpp:484-488) -- stays snapshot-only; that key is not missing.
//
//  Lock: a Win32 named mutex "Local\HT9045_JAM0000_dat".  The real file has two writers inside one wb_serve process:
//    * the main (tick) thread -- security.jam open／select／save／import (golden getters and SaveJamLevel through
//      common.cpp's INIFile) and the merge below;
//    * the Event Log Analyzer worker (EventLogAnalysis/ElaCore.cpp JamConfig::CheckAndReadBool／Int, #17 A), which opens
//      the same-named mutex on its own -- neither side links against the other.
//  Golden had the same two writers in two programs (HT9045.exe and EventlogAnalyzer.exe) with no lock.  A Win32 mutex is
//  recursive per thread; WAIT_ABANDONED counts as acquired (the owner died, the file is whatever it last wrote).
// =============================================================================
#pragma once

#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "vclcompat/IniFiles.h"

namespace jamini {

inline const char* MutexName() { return "Local\\HT9045_JAM0000_dat"; }

// Scoped hold of the JAM0000.dat lock.  held()==false after the timeout: the caller reports "busy" and retries.
class Lock {
public:
    explicit Lock(DWORD timeoutMs) : h_(::CreateMutexA(NULL, FALSE, MutexName())), held_(false)
    {
        if (h_)
        {
            const DWORD r = ::WaitForSingleObject(h_, timeoutMs);
            held_ = (r == WAIT_OBJECT_0 || r == WAIT_ABANDONED);
        }
    }
    ~Lock()
    {
        if (held_) ::ReleaseMutex(h_);
        if (h_) ::CloseHandle(h_);
    }
    bool held() const { return held_; }
private:
    HANDLE h_;
    bool held_;
    Lock(const Lock&);
    Lock& operator=(const Lock&);
};

// One key the export wrote into its snapshot because it was missing, as the exact text TIniFile wrote
// (WriteInteger -> decimal, WriteBool -> "0"／"1", WriteString -> the value), so WriteString replays it byte for byte.
struct Seed {
    std::string group, name, value;
};

struct MergeResult {
    bool createdFile;          // no real file before the merge (golden would have created it on the first seed)
    int written, skipped;      // skipped = the real file already had the key (added since the snapshot)
    std::string backup, error;
    MergeResult() : createdFile(false), written(0), skipped(0) {}
};

inline bool FileThere(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

inline bool ReadAll(const std::string& p, std::string* out)
{
    out->clear();
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return false;
    char buf[65536];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) out->append(buf, n);
    std::fclose(f);
    return true;
}

inline std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    size_t p = 0;
    while (p < text.size())
    {
        size_t e = text.find('\n', p);
        if (e == std::string::npos) e = text.size();
        std::string l = text.substr(p, e - p);
        if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1);
        v.push_back(l);
        p = e + 1;
    }
    return v;
}

// <real>.<yyyymmdd_hhnnss>.s128.bak next to the real file (not *.dat, so nothing that scans Error\English picks it up).
inline std::string BackupPathFor(const std::string& real)
{
    SYSTEMTIME t;
    ::GetLocalTime(&t);
    char s[48];
    std::snprintf(s, sizeof(s), ".%04d%02d%02d_%02d%02d%02d.s128.bak", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    return real + s;
}

// Step 1 (caller holds Lock): back up the real file byte for byte.  No real file -> nothing to back up, createdFile=true.
inline bool Begin(const std::string& real, const std::string& backup, MergeResult* r)
{
    *r = MergeResult();
    if (!FileThere(real)) { r->createdFile = true; return true; }
    if (!::CopyFileA(real.c_str(), backup.c_str(), FALSE))
    {
        r->error = "cannot back up " + real + " to " + backup;
        return false;
    }
    r->backup = backup;
    return true;
}

// Step 2 (caller holds Lock): write seeds [from, from+count) that the real file still lacks.
inline void Apply(const std::string& real, const std::vector<Seed>& seeds, size_t from, size_t count, MergeResult* r)
{
    vclcompat::TIniFile ini(AnsiString(real.c_str()));
    const size_t end = (from + count < seeds.size()) ? from + count : seeds.size();
    for (size_t i = from; i < end; ++i)
    {
        const AnsiString g(seeds[i].group.c_str()), n(seeds[i].name.c_str());
        if (ini.ValueExists(g, n)) { ++r->skipped; continue; }
        ini.WriteString(g, n, AnsiString(seeds[i].value.c_str()));
        ++r->written;
    }
}

// Step 3 (caller holds Lock): every seed is now present, and every non-blank line of the backup appears, in order, in the
// new file (the profile API only inserts lines, so the keys that were there keep their bytes; blank lines carry no data and
// are not compared, so where the API puts a separator line cannot fail a good merge).  On failure the backup is copied
// back over the real file (or the created file is removed) and r->error says why.
inline bool Verify(const std::string& real, const std::vector<Seed>& seeds, MergeResult* r)
{
    std::string why;
    {
        vclcompat::TIniFile ini(AnsiString(real.c_str()));
        for (size_t i = 0; i < seeds.size() && why.empty(); ++i)
            if (!ini.ValueExists(AnsiString(seeds[i].group.c_str()), AnsiString(seeds[i].name.c_str())))
                why = "[" + seeds[i].group + "] " + seeds[i].name + " is still missing after the merge";
    }
    if (why.empty() && !r->backup.empty())
    {
        std::string a, b;
        if (!ReadAll(r->backup, &a) || !ReadAll(real, &b))
            why = "cannot read the backup or the merged file back";
        else
        {
            const std::vector<std::string> la = Lines(a), lb = Lines(b);
            size_t j = 0;
            for (size_t i = 0; i < la.size() && why.empty(); ++i)
            {
                if (la[i].find_first_not_of(" \t") == std::string::npos) continue;
                while (j < lb.size() && lb[j] != la[i]) ++j;
                if (j == lb.size())
                {
                    char n[32];
                    std::snprintf(n, sizeof(n), "%lu", (unsigned long)(i + 1));   // no std::to_string on MinGW.org 6.3
                    why = std::string("original line ") + n + " changed or moved: " + la[i];
                }
                else ++j;
            }
        }
    }
    if (why.empty()) return true;
    r->error = why;
    if (!r->backup.empty()) ::CopyFileA(r->backup.c_str(), real.c_str(), FALSE);   // restore
    else if (r->createdFile) ::DeleteFileA(real.c_str());
    return false;
}

}  // namespace jamini
