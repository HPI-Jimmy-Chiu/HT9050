// =============================================================================
//  test_st02_w187_s9fx.cpp -- W-187 (POOL-9 GOLDEN913-UPDATE) #3: the S9F1/S9F3/S9F5/S9F7/S9F9 replies carry the 10-byte header
//  of the rejected message as <B[10]> (golden 913 SECSGEM/uHGemClass.cpp:2216-2235 SendS9FxEchoHeader, Ifor 20260625), not the
//  log text as <A> (golden 0618 :2222-2268).
//
//  AI(W906-W187) 20261009 (St02-E).  Suite name (add_test): St02_W187S9Fx.  No file of our own, no network (sim sockets).
//    [1] a standalone HTGem (ActiveWire = its own WireCodec), Remote = S6F11 with the W-bit, device 0x0102, system bytes
//        0x12345678: each of the five handlers -- Local = S9/Fn without the W-bit, the log line, the frame length 22, the body
//        0x21 0x0A + the 10 header bytes (S byte 0x86 = S6 | W-bit).
//    [2] Remote without the W-bit (S2F41, device 0): S byte 0x02.
//    [3] end to end: the HTGem dispatches on a real THGem's codec (as the HT9045Gem shim binds it); client role, sim socket
//        connected: the 26 bytes that go out (SendLocalDataHook -> THGem::SendLocalDataFrom -> SendBuf).
//    [4] ActiveWire NULL: golden 913's `if(p==NULL) return;` -- nothing composed, no crash.
//  Containment first (THGem's send logs through the machine log roots).
// =============================================================================
#include "SECSGEM/uHGemClass.h"
#include "SECSGEM/uHGemEquipment.h"
#include "st02_test_containment.h"
#include <cstdio>
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

static std::string Hex(const unsigned char* p, size_t n)
{
    std::string s;
    char b[4];
    for (size_t i = 0; i < n; ++i) { std::snprintf(b, sizeof(b), "%02X ", p[i]); s += b; }
    return s;
}

static void SeedRemote(SecsWireCodec& wc, unsigned short dev, unsigned char s, unsigned char f, unsigned w, unsigned int sb)
{
    wc.Remote.DeviceID = dev;
    wc.Remote.MessageID_S = s;
    wc.Remote.MessageID_F = f;
    wc.Remote.W_Bit = w;
    wc.Remote.PType = 0;
    wc.Remote.SType = 0;
    wc.Remote.SystemByte = sb;
}

typedef void (HTGem::*S9Fn)(AnsiString);

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W187S9Fx -- HTGem S9F1/F3/F5/F7/F9 (golden 913 uHGemClass.cpp:2216-2271)\n");
    if (!W906TestInsideCtestRoots("St02_W187S9Fx"))
        return 2;

    const unsigned char echoW[10] = { 0x01, 0x02, 0x86, 0x0B, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78 };

    // ---------------------------------------------------------------- [1]
    std::printf("[1] standalone HTGem, Remote = S6F11 W, device 0x0102, system bytes 0x12345678\n");
    {
        HTGem g;
        SecsWireCodec& wc = g.WireCodec;
        Check(g.ActiveWire == &wc, "[1] setup: ActiveWire is the HTGem's own codec");
        const struct { int f; S9Fn fn; const char* name; } H[] = {
            { 1, &HTGem::S9F1_UnrecognizedDeviceID, "S9F1" },
            { 3, &HTGem::S9F3_Unrecognized_Stream_Function_Type, "S9F3" },
            { 5, &HTGem::S9F5_UnrecognizedFunctionType, "S9F5" },
            { 7, &HTGem::S9F7_IllegalData, "S9F7" },
            { 9, &HTGem::S9F9_TransactionTimerTimeout, "S9F9" },
        };
        for (size_t i = 0; i < sizeof(H) / sizeof(H[0]); ++i)
        {
            SeedRemote(wc, 0x0102, 6, 11, 1, 0x12345678u);
            const std::string msg = std::string(H[i].name) + " reject text";
            (g.*H[i].fn)(AnsiString(msg.c_str()));
            const std::string tag = std::string("[1] ") + H[i].name + ": ";
            Check(wc.Local.MessageID_S == 9 && wc.Local.MessageID_F == H[i].f && wc.Local.W_Bit == 0,
                  tag + "Local = S9/F" + std::to_string(H[i].f) + " without the W-bit (S " + std::to_string(wc.Local.MessageID_S) +
                  ", F " + std::to_string(wc.Local.MessageID_F) + ", W " + std::to_string(wc.Local.W_Bit) + ")");
            const int nlog = wc.LogDataString->Count;
            Check(nlog > 0 && std::string(wc.LogDataString->GetString(nlog - 1).c_str()) == msg, tag + "the log line is the message");
            const unsigned char* b = wc.LocalBuffer.data();
            Check(wc.LocalLength_4 == 26, tag + "frame = 4 + 10-byte header + 12-byte body (LocalLength_4 " + std::to_string(wc.LocalLength_4) + ")");
            Check(b[0] == 0 && b[1] == 0 && b[2] == 0 && b[3] == 22, tag + "length prefix 22 (" + Hex(b, 4) + ")");
            Check(b[14] == 0x21 && b[15] == 10, tag + "body item = binary, 1 length byte, length 10 (" + Hex(b + 14, 2) + ")");
            Check(std::memcmp(b + 16, echoW, 10) == 0, tag + "body = the received header 01 02 86 0B 00 00 12 34 56 78 (" + Hex(b + 16, 10) + ")");
        }
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] Remote without the W-bit (S2F41, device 0)\n");
    {
        HTGem g;
        SecsWireCodec& wc = g.WireCodec;
        SeedRemote(wc, 0, 2, 41, 0, 0x00000105u);
        g.S9F5_UnrecognizedFunctionType("no w-bit");
        const unsigned char want[10] = { 0x00, 0x00, 0x02, 0x29, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05 };
        Check(wc.LocalLength_4 == 26 && std::memcmp(wc.LocalBuffer.data() + 16, want, 10) == 0,
              "[2] S9F5 body = 00 00 02 29 00 00 00 00 01 05 (" + Hex(wc.LocalBuffer.data() + 16, 10) + ")");
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] end to end on a real THGem's codec, client role, sim socket\n");
    {
        THGem gem;
        HTGem h;
        h.ActiveWire = &gem.WireCodec;   // as the HT9045Gem shim / ProcessReceiceData bind it (uHGemHT9045.cpp:371, uHGemEquipment.cpp:5404)
        gem.bUseClientSocket = true;
        gem.clientGem->Active = true;
        const bool connected = gem.clientGem->Socket->Connected;
        const size_t before = gem.clientGem->Socket->SimTxBuffer().size();
        SeedRemote(gem.WireCodec, 0x0102, 6, 11, 1, 0x12345678u);
        h.S9F7_IllegalData("S6F11 data format error");
        const std::vector<char>& tx = gem.clientGem->Socket->SimTxBuffer();
        const size_t n = tx.size() - before;
        Check(connected && n == 26, "[3] connected; one 26-byte frame sent (" + std::to_string(n) + " bytes)");
        if (n == 26)
        {
            const unsigned char* t = reinterpret_cast<const unsigned char*>(tx.data()) + before;
            const SecsWireCodec& wc = gem.WireCodec;
            const unsigned int sb = wc.Local.SystemByte;
            const unsigned char head[14] = { 0, 0, 0, 22,
                (unsigned char)((wc.Local.DeviceID >> 8) & 0xff), (unsigned char)(wc.Local.DeviceID & 0xff), 0x09, 0x07, 0x00, 0x00,
                (unsigned char)((sb >> 24) & 0xff), (unsigned char)((sb >> 16) & 0xff), (unsigned char)((sb >> 8) & 0xff), (unsigned char)(sb & 0xff) };
            Check(std::memcmp(t, head, 14) == 0, "[3] length 22 + our header S9F7, no W-bit, our system bytes (" + Hex(t, 14) + ")");
            Check(t[14] == 0x21 && t[15] == 10 && std::memcmp(t + 16, echoW, 10) == 0,
                  "[3] body 21 0A + the S6F11 header received (" + Hex(t + 14, 12) + ")");
        }
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] ActiveWire NULL\n");
    {
        HTGem g;
        SeedRemote(g.WireCodec, 0x0102, 6, 11, 1, 0x12345678u);
        g.S9F1_UnrecognizedDeviceID("before");
        const unsigned len = g.WireCodec.LocalLength_4;
        const int nlog = g.WireCodec.LogDataString->Count;
        g.ActiveWire = nullptr;
        g.S9F7_IllegalData("not sent");
        Check(g.WireCodec.LocalLength_4 == len && g.WireCodec.LogDataString->Count == nlog && g.WireCodec.Local.MessageID_F == 1,
              "[4] golden 913 `if(p==NULL) return;`: nothing logged or composed");
        g.ActiveWire = &g.WireCodec;
    }

    std::printf("\nSt02_W187S9Fx: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
