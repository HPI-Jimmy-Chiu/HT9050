// =============================================================================
//  test_st02_w187_spoolsend.cpp -- W-187 (POOL-9 GOLDEN913-UPDATE) #2: THGem::DoSpoolSendLocalData sends a spooled frame only over a
//  live link, under pLockOnSocketRecvice (golden 913 SECSGEM/uHGemEquipment.cpp:4062-4083, RogerYang 20260715 ht9045-secs-sem).
//
//  AI(W906-W187) 20261009 (St02-E).  Suite name (add_test): St02_W187SpoolSend.  A real THGem in memory (its ctor opens no port --
//  SecsTagPublish.cpp:42-49); the vclcompat sockets are in sim mode, SendBuf captures into SimTxBuffer().  No file, no network.
//    [1] client role, link not active / not connected: nothing is sent (golden 0618 sent anyway).
//    [2] client role, Active = true (sim: connects): the frame goes out, RunLength bytes = 4-byte length + the message.
//    [3] server role with no connection (ActiveConnections 0): returns without touching Connections[0] (0618 indexed it).
//    [4] re-entry: the caller already holds pLockOnSocketRecvice on this thread -- the call does not block (re-entrant
//        CRITICAL_SECTION, vclcompat/SyncObjs.h) and still sends.
//    [5] the lock is free afterwards: another thread acquires and releases it within 2 s.
// =============================================================================
#include "SECSGEM/uHGemEquipment.h"
#include <windows.h>
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

static THGem* g_gem = 0;
static DWORD WINAPI LockProbe(LPVOID)
{
    g_gem->pLockOnSocketRecvice->Acquire();
    g_gem->pLockOnSocketRecvice->Release();
    return 0;
}

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W187SpoolSend -- THGem::DoSpoolSendLocalData (golden 913 uHGemEquipment.cpp:4048-4114)\n");
    THGem gem;
    g_gem = &gem;
    // an S1F1 header-only HSMS frame: 4-byte length (10), device id, S1 (W-bit), F1, ptype, stype, 4 system bytes
    unsigned char frame[14] = { 0, 0, 0, 10, 0x00, 0x00, 0x81, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07 };
    const size_t n = sizeof(frame);
    Check(gem.clientGem != 0 && gem.srvGem != 0 && gem.pLockOnSocketRecvice != 0, "setup: the ctor built clientGem, srvGem and the lock");

    // ---------------------------------------------------------------- [1]
    std::printf("[1] client role, no live link\n");
    gem.bUseClientSocket = true;
    gem.DoSpoolSendLocalData(frame);
    Check(gem.clientGem->Socket->SimTxBuffer().empty(), "[1] Active false: nothing sent (" + std::to_string(gem.clientGem->Socket->SimTxBuffer().size()) + " bytes)");

    // ---------------------------------------------------------------- [2]
    std::printf("[2] client role, connected\n");
    gem.clientGem->Active = true;
    const bool connected = gem.clientGem->Socket->Connected;
    const int rl = gem.DoSpoolSendLocalData(frame);
    const std::vector<char>& tx = gem.clientGem->Socket->SimTxBuffer();
    Check(connected && rl == (int)n && tx.size() == n && std::memcmp(tx.data(), frame, n) == 0,
          "[2] Active + Connected: the 14-byte frame is sent as is (RunLength " + std::to_string(rl) + ", sent " + std::to_string(tx.size()) + ")");
    gem.clientGem->Socket->Connected = false;
    const size_t before = tx.size();
    gem.DoSpoolSendLocalData(frame);
    Check(gem.clientGem->Socket->SimTxBuffer().size() == before, "[2] Active but the socket no longer Connected: nothing more sent");
    gem.clientGem->Socket->Connected = true;

    // ---------------------------------------------------------------- [3]
    std::printf("[3] server role, no connection\n");
    gem.bUseClientSocket = false;
    gem.bServoSocketConnect = true;
    gem.srvGem->Socket->ActiveConnections = 0;
    gem.srvGem->Socket->Connections.clear();
    gem.DoSpoolSendLocalData(frame);
    Check(true, "[3] ActiveConnections 0: returned without touching Connections[0]");

    // ---------------------------------------------------------------- [4]
    std::printf("[4] the caller already holds the lock (re-entry)\n");
    gem.bUseClientSocket = true;
    const size_t b4 = gem.clientGem->Socket->SimTxBuffer().size();
    gem.pLockOnSocketRecvice->Acquire();
    gem.DoSpoolSendLocalData(frame);
    gem.pLockOnSocketRecvice->Release();
    Check(gem.clientGem->Socket->SimTxBuffer().size() == b4 + n, "[4] no self-deadlock on the same thread, the frame is sent");

    // ---------------------------------------------------------------- [5]
    std::printf("[5] the lock is free afterwards\n");
    HANDLE th = ::CreateThread(0, 0, &LockProbe, 0, 0, 0);
    const DWORD w = th ? ::WaitForSingleObject(th, 2000) : WAIT_FAILED;
    if (th) ::CloseHandle(th);
    Check(w == WAIT_OBJECT_0, "[5] another thread acquires and releases pLockOnSocketRecvice within 2 s");

    gem.clientGem->Active = false;
    std::printf("St02_W187SpoolSend: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
