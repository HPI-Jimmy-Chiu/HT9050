// test_webjsonscrubcache.cpp -- WebJsonScrubCache (the /JSON scrub route's file cache, tools/wb_serve.cpp W906_JsonScrubRoute).
// AI(W906-SCRUBCACHE) 20261001: NB2 problem A (docs/nb2_assist/notes_webref/P1_STOPLAT_first_load_stall.md on v906/nb2-assist).
// Real files in a %TEMP% folder of its own (removed at the end); no machine globals, no sockets.
// Pins: fill -> hit (the scrubber runs once; same body, count, ETag), HEAD on a hit copies no body, the key ignores case and
// "." segments, a new last-write time -> re-read, an unsettled stamp (written just now) -> no ETag and never cached, a small
// file -> ETag but not cached, a cached file that shrinks below the threshold -> dropped, a missing file -> false, and the
// prewarm fills only the big *.json / *.js directly in JSON\ and JSON\js\ (not JSON\runtime\, not other extensions).
#include "WebJsonScrubCache.h"

#include <windows.h>

#include <cctype>
#include <cstdio>
#include <string>

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("FAIL line %d: %s\n", line, what); }
}
#define CHECK(c) check((c), #c, __LINE__)

// Stand-in scrubber: deletes every "secret" and counts them. The cache must not care what the scrubber does.
static int g_scrubCalls = 0;
static std::string Scrub(const std::string& t, int* n)
{
    ++g_scrubCalls;
    std::string o = t;
    int k = 0;
    for (std::size_t p = o.find("secret"); p != std::string::npos; p = o.find("secret", p)) { o.erase(p, 6); ++k; }
    if (n) *n = k;
    return o;
}

static bool Put(const std::string& path, const std::string& s)
{
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    const bool ok = std::fwrite(s.data(), 1, s.size(), f) == s.size();
    std::fclose(f);
    return ok;
}

// Last-write time = now - secondsAgo ("settled" when secondsAgo >= the settle time).
static bool Age(const std::string& path, long long secondsAgo)
{
    HANDLE h = ::CreateFileA(path.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    FILETIME now;
    ::GetSystemTimeAsFileTime(&now);
    ULARGE_INTEGER u;
    u.LowPart = now.dwLowDateTime;
    u.HighPart = now.dwHighDateTime;
    u.QuadPart -= (ULONGLONG)secondsAgo * 10000000ULL;
    FILETIME ft;
    ft.dwLowDateTime = u.LowPart;
    ft.dwHighDateTime = u.HighPart;
    const BOOL ok = ::SetFileTime(h, NULL, NULL, &ft);
    ::CloseHandle(h);
    return ok != 0;
}

int main()
{
    using namespace ht9045;
    char tmp[MAX_PATH];
    ::GetTempPathA(MAX_PATH, tmp);
    const std::string dir = std::string(tmp) + "w906_scrubcache_" + std::to_string((unsigned long)::GetCurrentProcessId());
    ::CreateDirectoryA(dir.c_str(), NULL);
    JsonScrubResult r;

    std::printf("[1] big + settled: fill, then hits\n");
    {
        JsonScrubCacheResetForTest(100, 2000);
        const std::string big = dir + "\\Big.json";
        const std::string content = std::string(300, 'x') + "secret" + std::string(50, 'y');
        CHECK(Put(big, content) && Age(big, 3600));
        g_scrubCalls = 0;
        CHECK(JsonScrubCached(big, &Scrub, false, &r));
        CHECK(!r.fromCache && r.body == std::string(300, 'x') + std::string(50, 'y') && r.scrubbed == 1 && r.length == 350);
        CHECK(r.etag.size() > 4 && r.etag.compare(0, 3, "W/\"") == 0 && r.etag[r.etag.size() - 1] == '"');
        const std::string tag1 = r.etag;
        const std::string body1 = r.body;
        CHECK(JsonScrubCached(big, &Scrub, false, &r));
        CHECK(r.fromCache && g_scrubCalls == 1 && r.body == body1 && r.scrubbed == 1 && r.length == 350 && r.etag == tag1);
        CHECK(JsonScrubCached(big, &Scrub, true, &r));                         // HEAD on a hit: no body copy
        CHECK(r.fromCache && r.body.empty() && r.length == 350 && g_scrubCalls == 1 && r.etag == tag1);
        std::string upper = big;
        for (std::size_t i = 0; i < upper.size(); ++i) upper[i] = (char)std::toupper((unsigned char)upper[i]);
        CHECK(JsonScrubCached(upper, &Scrub, false, &r) && r.fromCache && g_scrubCalls == 1);             // key ignores case
        CHECK(JsonScrubCached(dir + "\\.\\Big.json", &Scrub, false, &r) && r.fromCache && g_scrubCalls == 1);   // and "."

        std::printf("[2] same size, new bytes, new last-write time -> re-read\n");
        std::string c2 = content;
        c2[0] = 'z';
        CHECK(Put(big, c2) && Age(big, 1800));
        CHECK(JsonScrubCached(big, &Scrub, false, &r));
        CHECK(!r.fromCache && g_scrubCalls == 2 && r.body[0] == 'z' && r.length == 350 && !r.etag.empty() && r.etag != tag1);
        CHECK(JsonScrubCached(big, &Scrub, false, &r) && r.fromCache && g_scrubCalls == 2 && r.body[0] == 'z');

        std::printf("[3] written just now (not settled): no ETag, scrubbed on every request\n");
        CHECK(Put(big, content));
        CHECK(JsonScrubCached(big, &Scrub, false, &r) && !r.fromCache && r.etag.empty() && g_scrubCalls == 3 && r.body[0] == 'x');
        CHECK(JsonScrubCached(big, &Scrub, false, &r) && !r.fromCache && r.etag.empty() && g_scrubCalls == 4);
        CHECK(Age(big, 60));                                                    // settles -> cached again
        CHECK(JsonScrubCached(big, &Scrub, false, &r) && !r.fromCache && !r.etag.empty() && g_scrubCalls == 5);
        CHECK(JsonScrubCached(big, &Scrub, false, &r) && r.fromCache && g_scrubCalls == 5);

        std::printf("[4] a cached file shrinks below the threshold -> dropped, then never cached\n");
        CHECK(Put(big, "secret small") && Age(big, 3600));
        CHECK(JsonScrubCached(big, &Scrub, false, &r) && !r.fromCache && r.body == " small" && r.scrubbed == 1 && g_scrubCalls == 6);
        CHECK(!r.etag.empty());                                                 // settled => ETag even when not cached
        CHECK(JsonScrubCached(big, &Scrub, false, &r) && !r.fromCache && g_scrubCalls == 7);
        const JsonScrubCacheStats st = JsonScrubCacheStatsNow();
        CHECK(st.hits == 6 && st.fills == 3 && st.uncached == 4);
        ::DeleteFileA(big.c_str());
    }

    std::printf("[5] missing file -> false, nothing scrubbed\n");
    {
        g_scrubCalls = 0;
        CHECK(!JsonScrubCached(dir + "\\nope.json", &Scrub, false, &r) && g_scrubCalls == 0);
        CHECK(!JsonScrubCached(dir, &Scrub, false, &r) && g_scrubCalls == 0);   // a directory is not a file
    }

    std::printf("[6] prewarm: big *.json / *.js directly in JSON\\ and JSON\\js\\ only\n");
    {
        JsonScrubCacheResetForTest(100, 2000);
        const std::string root = dir + "\\web";
        const std::string J = root + "\\JSON", JS = J + "\\js", RT = J + "\\runtime";
        ::CreateDirectoryA(root.c_str(), NULL);
        ::CreateDirectoryA(J.c_str(), NULL);
        ::CreateDirectoryA(JS.c_str(), NULL);
        ::CreateDirectoryA(RT.c_str(), NULL);
        const std::string big(400, 'b');
        const char* files[] = { "\\P.json", "\\js\\P.js", "\\s.json", "\\big.txt", "\\runtime\\R.json" };
        for (int i = 0; i < 5; ++i) {
            const std::string f = J + files[i];
            CHECK(Put(f, i == 2 ? std::string("{}") : big) && Age(f, 3600));
        }
        g_scrubCalls = 0;
        JsonScrubPrewarm(root, &Scrub);
        JsonScrubCacheStats st = JsonScrubCacheStatsNow();
        for (int w = 0; w < 1000 && !st.prewarmDone; ++w) { ::Sleep(10); st = JsonScrubCacheStatsNow(); }
        CHECK(st.prewarmDone && st.prewarmFiles == 2 && st.fills == 2 && g_scrubCalls == 2);
        CHECK(JsonScrubCached(J + "\\P.json", &Scrub, false, &r) && r.fromCache && r.length == 400);
        CHECK(JsonScrubCached(J + "\\js\\P.js", &Scrub, false, &r) && r.fromCache && r.length == 400);
        CHECK(JsonScrubCached(J + "\\runtime\\R.json", &Scrub, false, &r) && !r.fromCache);   // fills now, not at boot
        CHECK(JsonScrubCached(J + "\\s.json", &Scrub, false, &r) && !r.fromCache && r.body == "{}");
        CHECK(g_scrubCalls == 4);
        for (int i = 0; i < 5; ++i) ::DeleteFileA((J + files[i]).c_str());
        ::RemoveDirectoryA(RT.c_str());
        ::RemoveDirectoryA(JS.c_str());
        ::RemoveDirectoryA(J.c_str());
        ::RemoveDirectoryA(root.c_str());
    }

    ::RemoveDirectoryA(dir.c_str());
    std::printf("test_webjsonscrubcache: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) { std::printf("FAILED: %d checks\n", g_fail); return 1; }
    std::printf("PASS\n");
    return 0;
}
