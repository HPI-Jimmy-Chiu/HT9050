// WebStreamStats.h -- how much the web data stream costs, one [STREAM] line every 10 s.
//
// AI(W906-STREAM-S1) 20260930: RULINGS_20260930 #12 stage 1 = MEASUREMENT ONLY. No tag, frame, poll or behaviour changes.
//   User 20260930 20:4x: 「關於網頁更新過於頻繁問題，你能夠全部接手做嗎？後面通知st01按照我們做法」「有開的網頁才能更新資料」.
//   The design is St01's evaluation stream-by-open-page.md s4.0 (0bfcf9e0, reviewed by the laptop 0930 17:0x); the laptop
//   writes it (RULINGS #12 moved the whole job to the laptop). The later stages (skip families no open page reads, polls only
//   while their window is open, the hub forwarding only to open frames) are measured against these lines: one before, one after.
//
//   What one line holds (per 10 s window, the tick thread prints it; tools/wb_serve.cpp main loop, console + op log):
//     publish  : main-loop PublishHandlerTags() calls, tags staged (avg / max), time (avg / max ms)
//     tags     : the committed tag table by family -- pci1203.* / secs.sv.* / motionView.* / the rest; re-read only when the
//                staged total of the last main-loop publish changed or the last read is >= 60 s old, else the last counts
//                ("read" / "cached", and the ms of the last read; St01's family-read policy, WebStreamStats.cpp FamCache)
//     apiCache : /api/struct/{io,motor} cache rebuilds, time, size of the two runtime bodies the pages poll
//     ws       : live connections, snapshot / patch frames and bytes the server wrote, and its per-generation diff time
//     http     : io runtime / motor runtime / dialog mailbox / Production-update / other /JSON fetches, count and bytes
//   Threads: publish / apiCache / tick = the one tick thread; http = the socket thread (atomics); ws = WebBridgeStats (its lock).
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace webbridge { class TagSnapshot; struct WebBridgeStats; }

namespace ht9045 {

enum StreamHttpKind {
    kStreamHttpIoRuntime = 0,     // GET /api/struct/io/runtime     (HW.IoSetView, 200 ms)
    kStreamHttpMotorRuntime,      // GET /api/struct/motor/runtime  (Motor Test / Motor View / Motion View 500 ms, Teach 1 s)
    kStreamHttpMailbox,           // /JSON/*-dialog-request.json, Dialog-close-request.json (dialog-bridge.js, 100 ms)
    kStreamHttpProdUpdate,        // /JSON/Production-update.json (background.html, 250 ms)
    kStreamHttpJsonOther,         // any other /JSON/... the scrub route served
    kStreamHttpKinds
};

std::uint64_t StreamNowUs();                                          // steady clock
void StreamNotePublish(std::size_t staged, std::uint64_t us);         // tick thread
void StreamNoteApiCache(std::uint64_t us, std::size_t ioRuntimeBytes, std::size_t motorRuntimeBytes);   // tick thread
void StreamNoteHttp(int kind, std::size_t bytes);                     // any thread
int  StreamHttpKindOfJsonPath(const std::string& path);               // "/JSON/..." -> kStreamHttp*

struct StreamFamilies { std::size_t total, pci1203, secsSv, motionView, other; };
StreamFamilies StreamCountFamilies(const webbridge::TagSnapshot& snap);

// True once windowMs have passed since the current window opened (the first call opens the first window).
bool StreamDue(std::uint64_t nowMs, std::uint64_t windowMs = 10000);
// Closes the window: the finished "[STREAM] ..." line (no newline), then opens the next one. snap / ws may be null (the
// line then says "tags ?" / "ws ?"). ws is the server's cumulative counters; the line prints the change since the last call.
std::string StreamTick(std::uint64_t nowMs, const webbridge::TagSnapshot* snap, const webbridge::WebBridgeStats* ws);

// Print the line on the console (unless env W906_STREAM_STATS=0) and hand it to the op-log sink when one is installed
// (tools/wb_serve.cpp W906_OpLogInit installs its STREAM writer; 0 = console only).
void StreamSetSink(void (*sink)(const char* line));
void StreamEmit(const std::string& line);

void StreamResetForTest();

}  // namespace ht9045
