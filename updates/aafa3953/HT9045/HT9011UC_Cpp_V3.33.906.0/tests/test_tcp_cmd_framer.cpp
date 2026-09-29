// =============================================================================
//  test_tcp_cmd_framer.cpp -- W10 R2: TesterComm/Tcp/TcpCmdFramer (where the 7016 bytes become commands).
//
//  AI(W906-W10) 20260927 (St02-E).  Suite name (add_test): TesterComm_TcpCmdFramer
//
//  Pure: no fMain, no socket, no file.  Covers (Steven 20260927 R2 + St02-M's list):
//    a. a command split across reads ("HTGR,1" + "01,"; "HTSET,322," + "5," -> ONE command, bin 5 not bin 0);
//    b. several commands in one read, in order; 12 in one read -> all of them, the pump takes 10 per pass;
//    c. noise before a head (#Discard# up to the head, not the whole buffer); CR / LF / space / NUL between commands;
//       a trailing "HT" kept for the next read;
//    d. a >2048-byte blob: a head waiting past kCap -> #Overflow#, no truncation, the next command still comes out;
//       4096 bytes of garbage -> #Discard#; a 2048-byte command is taken, a 2049-byte one is dropped whole;
//    e. a command cut short by a new head -> #Incomplete#, the new one is taken;
//    f. the per-id field count (2 / 3 / 4, HTSET,720's optional 4th) and two connections that do not mix.
// =============================================================================
#include "TesterComm/Tcp/TcpCmdFramer.h"

#include <cstdio>
#include <string>
#include <vector>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

using tcpcmd::Event;
using tcpcmd::Framer;

static const void* const A = reinterpret_cast<const void*>(0x1000);
static const void* const B = reinterpret_cast<const void*>(0x2000);

static void Push(Framer& f, const void* k, const std::string& s) { f.Append(k, s.data(), s.size()); }

// every complete command now available (no pass limit here -- that is the pump's), + the events
static std::vector<std::string> Drain(Framer& f, const void* k, std::vector<Event>* ev = 0)
{
    std::vector<std::string> out;
    std::vector<Event> local;
    std::string frame;
    while (f.Next(k, &frame, ev ? ev : &local))
        out.push_back(frame);
    return out;
}

static size_t Count(const std::vector<Event>& ev, tcpcmd::EventKind kind, size_t* bytes = 0)
{
    size_t n = 0, b = 0;
    for (size_t i = 0; i < ev.size(); ++i)
        if (ev[i].kind == kind) { ++n; b += ev[i].bytes; }
    if (bytes) *bytes = b;
    return n;
}

int main()
{
    std::printf("TesterComm_TcpCmdFramer\n");

    // f. the field table (tools/tcp_cmd_fields_census.py keeps it equal to Command.cpp)
    bool opt = true;
    CHECK(tcpcmd::FieldsFor("HTGR", "101", &opt) == 2 && !opt, "f. HTGR,101 = 2 fields");
    CHECK(tcpcmd::FieldsFor("HTSET", "322", &opt) == 3 && !opt, "f. HTSET,322 = 3 fields");
    CHECK(tcpcmd::FieldsFor("HTSET", "350", &opt) == 4 && tcpcmd::FieldsFor("HTSET", "702", &opt) == 4, "f. HTSET,350 / 702 = 4 fields");
    CHECK(tcpcmd::FieldsFor("HTSET", "720", &opt) == 3 && opt, "f. HTSET,720 = 3 fields + an optional 4th");
    CHECK(tcpcmd::FieldsFor("HTSET", "999", &opt) == 2 && tcpcmd::FieldsFor("HTGR", "322", &opt) == 2, "f. unknown id / HTGR = 2");

    {   // a. split
        Framer f;
        Push(f, A, "HTGR,1");
        CHECK(Drain(f, A).empty() && f.Pending(A) == 6, "a. \"HTGR,1\" alone: nothing yet, kept");
        Push(f, A, "01,");
        std::vector<std::string> v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTGR,101,", "a. + \"01,\" -> one command HTGR,101,");
        Push(f, A, "HTSET,322,");
        CHECK(Drain(f, A).empty(), "a. \"HTSET,322,\" waits for its third field");
        Push(f, A, "5,");
        v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTSET,322,5,", "a. + \"5,\" -> HTSET,322,5, (golden would have parsed bin 0)");
        Push(f, A, "HTS");
        CHECK(Drain(f, A).empty() && f.Pending(A) == 3, "a. a head prefix \"HTS\" waits");
        Push(f, A, "ET,333,");
        v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTSET,333,", "a. + \"ET,333,\" -> HTSET,333,");
        CHECK(f.Connections() == 0, "a. an empty buffer is forgotten");
    }

    {   // b. several in one read
        Framer f;
        Push(f, A, "HTGR,101,HTGR,102,");
        std::vector<std::string> v = Drain(f, A);
        CHECK(v.size() == 2 && v[0] == "HTGR,101," && v[1] == "HTGR,102,", "b. two commands in one read, in order");
        std::string twelve;
        for (int i = 0; i < 12; ++i) twelve += "HTGR,109,";
        Push(f, A, twelve);
        std::string fr;
        std::vector<Event> ev;
        int n = 0;
        for (int i = 0; i < tcpcmd::kMaxFramesPerPass && f.Next(A, &fr, &ev); ++i) ++n;   // one pump pass
        CHECK(n == 10 && f.Pending(A) == 2 * 9, "b. 12 in one read: a pass takes 10, 2 wait (no new bytes needed)");
        CHECK(Drain(f, A).size() == 2, "b. the next pass takes the other 2");
        Push(f, A, "HTGR,101,\r\nHTGR,102,\r\n");
        v = Drain(f, A);
        CHECK(v.size() == 2 && v[1] == "HTGR,102,", "b. CRLF after each command is fine");
        CHECK(f.Connections() == 0, "b. CRLF consumed");
    }

    {   // c. noise
        Framer f;
        std::vector<Event> ev;
        Push(f, A, "xyz\r\nHTGR,101,");
        std::vector<std::string> v = Drain(f, A, &ev);
        size_t b = 0;
        CHECK(v.size() == 1 && v[0] == "HTGR,101," && Count(ev, tcpcmd::kDiscard, &b) == 1 && b == 5,
              "c. noise before a head: 5 bytes #Discard#, the command survives (RS232 cleared all)");
        ev.clear();
        Push(f, A, std::string(" \t\0\r\n", 5) + "HTGR,102,");
        v = Drain(f, A, &ev);
        CHECK(v.size() == 1 && v[0] == "HTGR,102," && ev.empty(), "c. separators (space TAB NUL CR LF) are skipped silently");
        ev.clear();
        Push(f, A, "abcHT");
        v = Drain(f, A, &ev);
        CHECK(v.empty() && Count(ev, tcpcmd::kDiscard, &b) == 1 && b == 3 && f.Pending(A) == 2, "c. \"abcHT\": 3 dropped, \"HT\" kept");
        Push(f, A, "GR,101,");
        v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTGR,101,", "c. + \"GR,101,\" -> HTGR,101,");
        ev.clear();
        Push(f, A, "htgr,101,");
        v = Drain(f, A, &ev);
        CHECK(v.empty() && Count(ev, tcpcmd::kDiscard) == 1 && f.Pending(A) == 0, "c. lower-case head = noise (golden answers nothing either)");
    }

    {   // d. the 2048-byte limit
        Framer f;
        std::vector<Event> ev;
        Push(f, A, "HTSET,403," + std::string(3000, 'x'));   // no third comma: waits past kCap
        std::vector<std::string> v = Drain(f, A, &ev);
        size_t b = 0;
        CHECK(v.empty() && Count(ev, tcpcmd::kOverflow, &b) == 1 && b == 3010 && f.Pending(A) == 0,
              "d. >2048 bytes waiting without a complete command -> #Overflow# 3010 bytes, nothing kept");
        Push(f, A, "HTGR,101,");
        v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTGR,101,", "d. the next command still comes out (the connection stays)");
        ev.clear();
        Push(f, A, std::string(4096, '#'));
        v = Drain(f, A, &ev);
        CHECK(v.empty() && Count(ev, tcpcmd::kDiscard, &b) == 1 && b == 4096 && f.Pending(A) == 0, "d. 4096 bytes of garbage -> #Discard# 4096");
        const std::string head = "HTSET,403,";
        const std::string ok = head + std::string(2048 - head.size() - 1, 'y') + ",";
        const std::string big = head + std::string(2049 - head.size() - 1, 'y') + ",";
        ev.clear();
        Push(f, A, ok);
        v = Drain(f, A, &ev);
        CHECK(v.size() == 1 && v[0].size() == 2048 && ev.empty(), "d. a 2048-byte command is taken whole");
        Push(f, A, big + "HTGR,102,");
        v = Drain(f, A, &ev);
        CHECK(v.size() == 1 && v[0] == "HTGR,102," && Count(ev, tcpcmd::kOverflow, &b) == 1 && b == 2049,
              "d. a 2049-byte command is dropped whole (#Overflow#, never truncated), the next one survives");
    }

    {   // e. cut short by a new head
        Framer f;
        std::vector<Event> ev;
        Push(f, A, "HTGR,101HTGR,102,");
        std::vector<std::string> v = Drain(f, A, &ev);
        size_t b = 0;
        CHECK(v.size() == 1 && v[0] == "HTGR,102," && Count(ev, tcpcmd::kIncomplete, &b) == 1 && b == 8,
              "e. \"HTGR,101\" + a new head -> #Incomplete# 8 bytes, HTGR,102, taken");
        ev.clear();
        Push(f, A, "HTSET,322,HTGR,109,");
        v = Drain(f, A, &ev);
        CHECK(v.size() == 1 && v[0] == "HTGR,109," && Count(ev, tcpcmd::kIncomplete) == 1, "e. HTSET,322 without its index is not run");
    }

    {   // f. field counts, 720, two connections
        Framer f;
        Push(f, A, "HTSET,350,-1.5,");
        CHECK(Drain(f, A).empty(), "f. HTSET,350 waits for its 4th field");
        Push(f, A, "-2.5,");
        std::vector<std::string> v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTSET,350,-1.5,-2.5,", "f. HTSET,350,-1.5,-2.5,");
        Push(f, A, "HTSET,720,LOT1,OP1,");
        v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTSET,720,LOT1,OP1,", "f. HTSET,720 with OPID: one command, 4 fields");
        Push(f, A, "HTSET,720,LOT2,");
        v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTSET,720,LOT2,", "f. HTSET,720 without OPID: 3 fields, not held back");
        Push(f, A, "HTSET,720,LOT3,\r\nHTGR,101,");
        v = Drain(f, A);
        CHECK(v.size() == 2 && v[0] == "HTSET,720,LOT3," && v[1] == "HTGR,101,", "f. HTSET,720 + CRLF + the next command");
        Push(f, A, "HTSET,322,");
        Push(f, B, "HTGR,101,");
        v = Drain(f, B);
        CHECK(v.size() == 1 && v[0] == "HTGR,101," && f.Pending(A) == 10, "f. connection B's command does not touch A's half");
        Push(f, A, "7,");
        v = Drain(f, A);
        CHECK(v.size() == 1 && v[0] == "HTSET,322,7,", "f. A's half joins A's next read");
        Push(f, A, "HTGR,1");
        f.Drop(A);
        CHECK(f.Pending(A) == 0 && f.Connections() == 0, "f. Drop (disconnect) forgets the half command");
    }

    {   // NextFrame on a plain string (the step the pump runs)
        std::string b = "HTGR,101,HTGR";
        std::string fr;
        CHECK(tcpcmd::NextFrame(b, &fr, 0) && fr == "HTGR,101," && b == "HTGR", "NextFrame leaves the rest in the buffer");
        CHECK(!tcpcmd::NextFrame(b, &fr, 0) && b == "HTGR", "NextFrame waits on \"HTGR\"");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
