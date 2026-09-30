// WebJsonScrubCache.h -- the /JSON scrub route's file cache (tools/wb_serve.cpp W906_JsonScrubRoute).
//
// AI(W906-SCRUBCACHE) 20261001: NB2 S9-S13 (v906/nb2-assist 3fb68638, docs/nb2_assist/notes_webref/
//   P1_STOPLAT_first_load_stall.md, problem A). web/background.html polls JSON/Production-update.json every 250 ms; the file
//   is 8.6 MB and nothing in this tree writes it (it is the HTML project's deployed snapshot, unchanged since the 0922
//   deployment on the laptop). Every poll made the SOCKET thread read the whole file and run W906_ScrubCredentials over it:
//   115-132 ms per poll, ~40 % of that thread, and the idle stop-command p95 was 155-285 ms (27 ms with that poll blocked) --
//   HTTP routes and WebSocket commands share the one select() thread (WebBridge/WebBridgeServer.cpp ThreadMain).
//
//   The route now asks this cache. A file whose size, last-write time and creation time are the same as at the last read
//   is not read or scrubbed again; the response bytes are exactly what the route sent before (same file, same scrubber).
//   * Cached only when the file is >= 256 KB (the small mailbox files stay uncached: cheap, and C++ rewrites them quickly)
//     and its last-write time was >= 2 s old when it was read ("settled"). Two rewrites inside one clock tick with the same
//     size would share a stamp; a settled stamp cannot be reused by a later write, which gets a newer time.
//   * The ETag (W/"size-write-create", hex) is given only for a settled stamp, for the same reason; web/page/settings.js
//     refreshProduction sends a HEAD first and skips the 8.6 MB GET + JSON.parse while the ETag is the one it last parsed.
//   * JsonScrubPrewarm (boot, before server.Start) reads + scrubs the big files under <root>\JSON and <root>\JSON\js on a
//     background thread, so the first page load does not pay for them on the socket thread (NB2 problem B -- the 4-8 s
//     first-load stall -- has no confirmed cause yet; this only moves the first big read off the socket thread).
//   Threads: the socket thread (route) and the prewarm thread. One lock around the map; file I/O and scrubbing are outside it.
//   Not here: which pages poll, and how often (web/background.html, web/page/settings.js).
#pragma once

#include <string>

namespace ht9045 {

typedef std::string (*JsonScrubFn)(const std::string& text, int* scrubbed);

struct JsonScrubResult {
    std::string body;        // the scrubbed text; left empty when headOnly
    long long   length;      // byte length of the scrubbed text (the HEAD Content-Length)
    int         scrubbed;    // values the scrubber cleared (X-W906-Scrubbed)
    std::string etag;        // W/"..." for a settled stamp, else empty
    bool        fromCache;
};

// false = the file cannot be read (the route then falls back to the static handler, exactly as when ReadWholeFile failed).
bool JsonScrubCached(const std::string& path, JsonScrubFn scrub, bool headOnly, JsonScrubResult* out);

// Boot: read + scrub every *.json / *.js >= 256 KB directly in <webRoot>\JSON and <webRoot>\JSON\js on a background thread.
void JsonScrubPrewarm(const std::string& webRoot, JsonScrubFn scrub);

struct JsonScrubCacheStats { long hits, fills, uncached, prewarmFiles; bool prewarmDone; };
JsonScrubCacheStats JsonScrubCacheStatsNow();

// tests only
void JsonScrubCacheResetForTest(long long minBytes, long long settleMs);   // empties the map; defaults are 262144 / 2000

}  // namespace ht9045
