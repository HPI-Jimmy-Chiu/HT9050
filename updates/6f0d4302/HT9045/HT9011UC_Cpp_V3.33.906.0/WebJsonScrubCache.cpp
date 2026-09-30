// WebJsonScrubCache.cpp -- see WebJsonScrubCache.h.
//
// AI(W906-SCRUBCACHE) 20261001: pure Win32 + WebBridge/Sync.h (this toolchain has no working std::mutex / std::thread,
//   see that header). No machine headers; -Wall -Wextra via tests/ test_webjsonscrubcache.
#include "WebJsonScrubCache.h"

#include "WebBridge/Sync.h"

#include <cctype>
#include <cstdio>
#include <map>

namespace ht9045 {
namespace {

struct Stamp {
    unsigned long long size, write, create;
    bool operator==(const Stamp& o) const { return size == o.size && write == o.write && create == o.create; }
};

struct Entry {
    Stamp       st;
    std::string body;
    int         scrubbed;
};

// Function-local statics: first touched by JsonScrubPrewarm on the boot thread, before server.Start().
webbridge::WbMutex& Mu() { static webbridge::WbMutex m; return m; }
std::map<std::string, Entry>& Map() { static std::map<std::string, Entry> m; return m; }

// All under Mu().
JsonScrubCacheStats g_stats = { 0, 0, 0, 0, false };
long long g_minBytes = 262144;
long long g_settleMs = 2000;

unsigned long long U64(DWORD hi, DWORD lo) { return ((unsigned long long)hi << 32) | lo; }

bool StampOf(const std::string& path, Stamp* s)
{
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (!::GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &a)) return false;
    if (a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return false;
    s->size   = U64(a.nFileSizeHigh, a.nFileSizeLow);
    s->write  = U64(a.ftLastWriteTime.dwHighDateTime, a.ftLastWriteTime.dwLowDateTime);
    s->create = U64(a.ftCreationTime.dwHighDateTime, a.ftCreationTime.dwLowDateTime);
    return true;
}

// Last write at least settleMs before now (FILETIME ticks are 100 ns). A time in the future is not settled.
bool Settled(const Stamp& s, long long settleMs)
{
    FILETIME f;
    ::GetSystemTimeAsFileTime(&f);
    const unsigned long long now = U64(f.dwHighDateTime, f.dwLowDateTime);
    return now >= s.write && now - s.write >= (unsigned long long)settleMs * 10000ULL;
}

void Hex(std::string* o, unsigned long long v)   // no %llx: msvcrt dialect (same reason as WebStreamStats.cpp)
{
    char b[17];
    int i = 16;
    b[16] = 0;
    do { b[--i] = "0123456789abcdef"[v & 15]; v >>= 4; } while (v && i > 0);
    o->append(b + i);
}

std::string EtagOf(const Stamp& s)
{
    std::string e = "W/\"";
    Hex(&e, s.size);
    e += '-';
    Hex(&e, s.write);
    e += '-';
    Hex(&e, s.create);
    e += '"';
    return e;
}

// The same read as tools/wb_serve.cpp ReadWholeFile (fopen "rb", 8 KB freads).
bool ReadAll(const std::string& path, std::string* out)
{
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    char buf[8192];
    std::size_t got;
    out->clear();
    while ((got = std::fread(buf, 1, sizeof(buf), f)) > 0) out->append(buf, got);
    std::fclose(f);
    return true;
}

// Absolute, long-name, lower-case: the route (8.3 names already expanded, URL casing) and the prewarm (directory listing
// casing) must land on the same key for the same file.
std::string KeyOf(const std::string& path)
{
    std::string k = path;
    char full[4 * MAX_PATH];
    const DWORD m = ::GetFullPathNameA(path.c_str(), sizeof full, full, NULL);
    if (m && m < sizeof full) k.assign(full, m);
    char lp[4 * MAX_PATH];
    const DWORD n = ::GetLongPathNameA(k.c_str(), lp, sizeof lp);
    if (n && n < sizeof lp) k.assign(lp, n);
    for (std::size_t i = 0; i < k.size(); ++i) k[i] = (char)std::tolower((unsigned char)k[i]);
    return k;
}

bool EndsNoCase(const std::string& s, const char* tail)
{
    const std::size_t n = std::char_traits<char>::length(tail);
    if (s.size() < n) return false;
    for (std::size_t i = 0; i < n; ++i)
        if (std::tolower((unsigned char)s[s.size() - n + i]) != std::tolower((unsigned char)tail[i])) return false;
    return true;
}

struct PrewarmArg {
    std::string root;
    JsonScrubFn scrub;
};

void PrewarmMain(void* raw)
{
    PrewarmArg* a = static_cast<PrewarmArg*>(raw);
    const DWORD t0 = ::GetTickCount();
    long files = 0;
    unsigned long long bytes = 0;
    long long minBytes;
    { webbridge::WbGuard g(Mu()); minBytes = g_minBytes; }
    static const char* const kDirs[] = { "\\JSON\\", "\\JSON\\js\\" };
    for (int d = 0; d < 2; ++d) {
        const std::string dir = a->root + kDirs[d];
        WIN32_FIND_DATAA fd;
        HANDLE h = ::FindFirstFileA((dir + "*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) continue;
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            const unsigned long long size = U64(fd.nFileSizeHigh, fd.nFileSizeLow);
            const std::string name(fd.cFileName);
            if ((long long)size < minBytes || !(EndsNoCase(name, ".json") || EndsNoCase(name, ".js"))) continue;
            JsonScrubResult r;
            if (JsonScrubCached(dir + name, a->scrub, true, &r)) { ++files; bytes += (unsigned long long)r.length; }
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    {
        webbridge::WbGuard g(Mu());
        g_stats.prewarmFiles = files;
        g_stats.prewarmDone = true;
    }
    std::printf("[SCRUBCACHE] prewarm %s: %ld file(s) >= %ld bytes, %.1f MB scrubbed, %lu ms (off the socket thread)\n",
                a->root.c_str(), files, (long)minBytes, bytes / 1048576.0, (unsigned long)(::GetTickCount() - t0));
    std::fflush(stdout);
    delete a;
}

}  // namespace

bool JsonScrubCached(const std::string& path, JsonScrubFn scrub, bool headOnly, JsonScrubResult* out)
{
    out->body.clear();
    out->length = 0;
    out->scrubbed = 0;
    out->etag.clear();
    out->fromCache = false;

    Stamp a;
    const bool haveA = StampOf(path, &a);
    const std::string key = KeyOf(path);
    if (haveA) {
        webbridge::WbGuard g(Mu());
        std::map<std::string, Entry>::const_iterator it = Map().find(key);
        if (it != Map().end() && it->second.st == a) {
            if (!headOnly) out->body = it->second.body;
            out->length = (long long)it->second.body.size();
            out->scrubbed = it->second.scrubbed;
            out->etag = EtagOf(a);                   // it was settled when it was filled, and the stamp has not moved
            out->fromCache = true;
            ++g_stats.hits;
            return true;
        }
    }

    std::string text;
    if (!ReadAll(path, &text)) return false;
    int n = 0;
    std::string body = scrub(text, &n);
    long long minBytes, settleMs;
    { webbridge::WbGuard g(Mu()); minBytes = g_minBytes; settleMs = g_settleMs; }
    Stamp b;
    // The stamp must not have moved while the file was read, must match the bytes read, and must be settled.
    const bool stable = haveA && StampOf(path, &b) && b == a && a.size == (unsigned long long)text.size()
                        && Settled(a, settleMs);
    out->length = (long long)body.size();
    out->scrubbed = n;
    if (stable) out->etag = EtagOf(a);
    {
        webbridge::WbGuard g(Mu());
        if (stable && (long long)a.size >= minBytes) {
            Entry& e = Map()[key];
            e.st = a;
            e.body = body;
            e.scrubbed = n;
            ++g_stats.fills;
        } else {
            Map().erase(key);
            ++g_stats.uncached;
        }
    }
    if (!headOnly) out->body.swap(body);
    return true;
}

void JsonScrubPrewarm(const std::string& webRoot, JsonScrubFn scrub)
{
    { webbridge::WbGuard g(Mu()); (void)Map(); }     // construct both statics on this thread, before the socket thread runs
    PrewarmArg* a = new PrewarmArg;
    a->root = webRoot;
    a->scrub = scrub;
    webbridge::WbThread t;                          // detaches when it goes out of scope (WebBridge/Sync.h)
    if (!t.start(&PrewarmMain, a)) delete a;
}

JsonScrubCacheStats JsonScrubCacheStatsNow()
{
    webbridge::WbGuard g(Mu());
    return g_stats;
}

void JsonScrubCacheResetForTest(long long minBytes, long long settleMs)
{
    webbridge::WbGuard g(Mu());
    Map().clear();
    g_minBytes = minBytes;
    g_settleMs = settleMs;
    JsonScrubCacheStats z = { 0, 0, 0, 0, false };
    g_stats = z;
}

}  // namespace ht9045
