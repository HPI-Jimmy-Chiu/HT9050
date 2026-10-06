// =============================================================================
//  tests/test_myplc_modbus.cpp -- W5 MyPLC Modbus TCP client pair acceptance test
//
//  AI(W5-MyPLC-Translate) 20260710: new file.
//
//  Proves the translated MyPLC Modbus stack LINKS + RUNS with NO hardware and
//  NO real network I/O (SIM-only, deterministic, same posture as
//  tests/test_keypro_tcomm.cpp):
//
//    [1] vclcompat/ClientSocket  (Scktcomp::TClientSocket/TCustomWinSocket)
//        Sim-mode connect/disconnect side effects, SendBuf tx capture,
//        SimPushReceive -> OnRead firing, ReceiveLength/ReceiveBuf pull.
//
//    [2] MyPLC::ModbusTCPClient  (MyPLC/ModbusTCPClient.{h,cpp})
//        SocketConnect/SocketDisConnect/SocketError/SocketRead driven
//        DIRECTLY (they are public methods, per golden's class layout) with
//        a synthetic TCustomWinSocket the test controls -- this reaches the
//        exact byte-level Modbus-TCP-ish frame PARSE logic in SocketRead
//        (length-guard, inner-size-guard, big-endian iID extraction, payload
//        slice) without needing access to ModbusTCPClient's PRIVATE
//        pClinetSocket (faithfully unchanged golden API -- no test hooks
//        were added to the translated production class).
//        Connect()/DisConnect()/IsConnected()/SendData() guard-clause
//        behavior verified via the SIM-default TClientSocket path.
//
//    [3] MyPLC_IO_Modbus  (MyPLC/MyPLC_IO_Modbus.{h,cpp})
//        InitPLCIO -> PLCIOTaskCycle -> PLCStatusCheck pumped over the global
//        PlcComm (SIM mode) without crash; TMyPLC_IO_Modbus::IsOn()/IsOff()
//        range-guard + iPLCSafetyVer branch, seeded via bPLCInData;
//        TPLCIOThread ctor/Resume()/PLCIOProcess() offline-inert acceptance
//        (Execute()'s do-while body is INTENTIONALLY never invoked here --
//        it is dead/inert code per the header note: no real thread spawns
//        it, and Terminated has no public setter, so calling it would spin
//        forever).
//
//  LIMITATION (documented, same posture as test_keypro_tcomm.cpp): no real
//  PLC / hardware / network is available in this environment, so the REAL
//  (WinSock2) connect path of vclcompat/ClientSocket is NOT exercised here
//  (implemented, syntax+link verified, but a live-network test would be
//  environment-dependent and is out of scope for an offline CI-safe suite).
// =============================================================================
#include "MyPLC/MyPLC_IO_Modbus.h"
#include "vclcompat/ClientSocket.h"

#include <cstdio>
#include <cstring>

// --------------------------------------------------------------------------
//  Tiny PASS / FAIL harness (same style as tests/test_keypro_tcomm.cpp)
// --------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                   \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

// ===========================================================================
//  [1] vclcompat/ClientSocket -- Sim mode round trip
// ===========================================================================
struct ConnCapture {
    int connectCount, disconnectCount, readCount;
    TObject* lastConnectSender;
    ConnCapture() : connectCount(0), disconnectCount(0), readCount(0), lastConnectSender(0) {}
};
static ConnCapture g_cc;

static void test_clientsocket_sim()
{
    printf("\n[1] vclcompat/ClientSocket: Sim mode connect/tx/rx round trip\n");

    TClientSocket sock(0);
    CHECK(sock.IsSimMode() == true, "TClientSocket defaults to Sim mode");
    CHECK(sock.IsActiveNow() == false, "not active before Active=true");

    sock.OnConnect = [](TObject* Sender, TCustomWinSocket*) {
        g_cc.connectCount++;
        g_cc.lastConnectSender = Sender;
    };
    sock.OnDisconnect = [](TObject*, TCustomWinSocket*) { g_cc.disconnectCount++; };
    sock.OnRead       = [](TObject*, TCustomWinSocket*) { g_cc.readCount++; };

    sock.Address = "127.0.0.1";
    sock.Port    = 502;

    // --- Active=true connects synchronously in Sim mode --------------------
    sock.Active = true;
    CHECK(sock.IsActiveNow() == true, "Active=true connects (Sim, synchronous)");
    CHECK(g_cc.connectCount == 1, "OnConnect fired exactly once");
    CHECK(g_cc.lastConnectSender == &sock, "OnConnect Sender == the TClientSocket instance");
    CHECK((bool)sock.Active == true, "Active reads back true (ActiveProxy::operator bool)");

    // Idempotent: a second Active=true while already open does not re-fire.
    sock.Active = true;
    CHECK(g_cc.connectCount == 1, "Active=true while already open is idempotent (no re-fire)");

    // --- SendBuf captures into the sim tx sink ------------------------------
    char tx[] = { 0x12, 0x34, 0x00, 0x00, 0x00, 0x03, 0x01 };
    int sent = sock.Socket->SendBuf(tx, (int)sizeof(tx));
    CHECK(sent == (int)sizeof(tx), "SendBuf returns the byte count sent (sim capture)");
    const std::vector<char>& txbuf = sock.Socket->SimTxBuffer();
    CHECK(txbuf.size() == sizeof(tx), "sim tx sink captured the exact byte count");
    CHECK(std::memcmp(&txbuf[0], tx, sizeof(tx)) == 0, "sim tx sink content matches exactly");
    sock.Socket->SimClearTx();
    CHECK(sock.Socket->SimTxBuffer().empty(), "SimClearTx() empties the tx sink");

    // --- SimPushReceive enqueues + fires OnRead -----------------------------
    CHECK(sock.Socket->ReceiveLength() == 0, "no bytes queued before SimPushReceive");
    const char rx[] = { (char)0xAA, (char)0xBB, (char)0xCC, (char)0xDD };
    sock.Socket->SimPushReceive(rx, (int)sizeof(rx));
    CHECK(g_cc.readCount == 1, "SimPushReceive fires OnRead once");
    CHECK(sock.Socket->ReceiveLength() == (int)sizeof(rx), "ReceiveLength() reports the queued byte count");

    char rxout[8] = {0};
    int got = sock.Socket->ReceiveBuf(rxout, (int)sizeof(rxout));
    CHECK(got == (int)sizeof(rx), "ReceiveBuf returns the exact queued byte count");
    CHECK(std::memcmp(rxout, rx, sizeof(rx)) == 0, "ReceiveBuf content matches exactly");
    CHECK(sock.Socket->ReceiveLength() == 0, "queue drained after ReceiveBuf");

    // --- Active=false disconnects -------------------------------------------
    sock.Active = false;
    CHECK(sock.IsActiveNow() == false, "Active=false disconnects");
    CHECK(g_cc.disconnectCount == 1, "OnDisconnect fired exactly once");

    // Idempotent close (golden also calls Close() explicitly right after
    // Active=false; matches ModbusTCPClient::DisConnect()).
    sock.Close();
    CHECK(g_cc.disconnectCount == 1, "Close() after already-closed does not re-fire OnDisconnect");

    // Re-open works after a close.
    sock.Active = true;
    CHECK(sock.IsActiveNow() == true, "re-Active=true reconnects after Close()");
    CHECK(g_cc.connectCount == 2, "OnConnect fires again on reconnect");
    sock.Active = false;
}

// ===========================================================================
//  [2] MyPLC::ModbusTCPClient -- guard clauses + frame-parse oracle
// ===========================================================================
static void test_modbustcpclient()
{
    printf("\n[2] MyPLC::ModbusTCPClient: guard clauses + SocketRead frame-parse oracle\n");

    ModbusTCPClient client;
    client.SetTCPInfo("127.0.0.1", 502);

    CHECK(client.IsConnected() == false, "not connected before Connect()");

    // Connect() returns bConnected (see golden JerryYang-20250206 quirk note
    // in ModbusTCPClient.cpp): in Sim mode the SocketConnect handler fires
    // SYNCHRONOUSLY during Active=true, so bConnected is already true by the
    // time Connect() reads it back and returns.
    bool connectResult = client.Connect();
    CHECK(connectResult == true, "Connect() returns true (Sim synchronous OnConnect)");
    CHECK(client.IsConnected() == true, "IsConnected() true after Connect()");

    // --- SendData guard clauses (fully observable via return value) --------
    VTBYTEDATA empty;
    CHECK(client.SendData(1, 1, 4, 0, empty) == false, "SendData with empty payload returns false");

    VTBYTEDATA payload;
    payload.push_back(0x00);
    payload.push_back(0x0B);
    CHECK(client.SendData(1, 1, 4, 0, payload) == true, "SendData with a real payload returns true while connected");

    client.DisConnect();
    CHECK(client.IsConnected() == false, "DisConnect() clears IsConnected()");
    CHECK(client.SendData(1, 1, 4, 0, payload) == false, "SendData while disconnected returns false");

    // --- SocketRead frame-parse oracle --------------------------------------
    // SocketRead/SocketConnect/SocketDisConnect/SocketError are PUBLIC (per
    // golden's class layout), so they can be driven directly with a
    // test-owned TCustomWinSocket -- no access to the private pClinetSocket
    // is needed, and no test-only hooks were added to the translated class.
    //
    // Golden frame layout (ModbusTCPClient.cpp:SocketRead):
    //   [0..1] iID (big-endian)          [2..3] unused (protocol)
    //   [4..5] length = totalLen-6 (big-endian, GUARD #1)
    //   [6]    slave addr (unused)       [7] function code (unused)
    //   [8]    inner size = totalLen-9 (GUARD #2)
    //   [9..]  payload bytes
    ModbusTCPClient client2;
    TCustomWinSocket sock2;

    {
        // iID=0x1234, payload={0xAA,0xBB,0xCC} -> totalLen=12
        char frame[] = {
            0x12, 0x34,             // [0..1] iID = 0x1234
            0x00, 0x00,             // [2..3] unused
            0x00, 0x06,             // [4..5] length = 12-6 = 6
            0x01, 0x04,             // [6..7] slave / function (unused)
            0x03,                   // [8] inner size = 12-9 = 3
            (char)0xAA, (char)0xBB, (char)0xCC   // [9..11] payload
        };
        sock2.SimPushReceive(frame, (int)sizeof(frame));
        client2.SocketRead(0, &sock2);

        VTBYTEDATA out;
        bool got = client2.GetRecevie(0x1234, out, false);
        CHECK(got == true, "SocketRead parses a well-formed frame -> GetRecevie finds iID 0x1234");
        CHECK(out.size() == 3, "parsed payload has the expected 3-byte length");
        bool payloadMatch = out.size() == 3 && out[0]==0xAA && out[1]==0xBB && out[2]==0xCC;
        CHECK(payloadMatch, "parsed payload content == {0xAA,0xBB,0xCC} exactly");
    }

    {
        // GUARD #1 violated: length field says 99 (totalLen-6 would be 6).
        char badFrame[] = {
            0x22, 0x22, 0x00, 0x00,
            0x00, 99,               // wrong length field
            0x01, 0x04, 0x03, (char)0x11, (char)0x22, (char)0x33
        };
        sock2.SimPushReceive(badFrame, (int)sizeof(badFrame));
        client2.SocketRead(0, &sock2);

        VTBYTEDATA out;
        CHECK(client2.GetRecevie(0x2222, out, false) == false,
              "GUARD #1 (length mismatch) rejects the frame -- no reply recorded");
    }

    {
        // GUARD #2 violated: inner size says 99 (totalLen-9 would be 3).
        char badFrame2[] = {
            0x33, 0x33, 0x00, 0x00,
            0x00, 0x06,
            0x01, 0x04, 99,          // wrong inner size field
            (char)0x44, (char)0x55, (char)0x66
        };
        sock2.SimPushReceive(badFrame2, (int)sizeof(badFrame2));
        client2.SocketRead(0, &sock2);

        VTBYTEDATA out;
        CHECK(client2.GetRecevie(0x3333, out, false) == false,
              "GUARD #2 (inner-size mismatch) rejects the frame -- no reply recorded");
    }

    {
        // Length-insufficient early-return guard (<=6 bytes).
        char shortFrame[] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };  // 6 bytes exactly
        sock2.SimPushReceive(shortFrame, (int)sizeof(shortFrame));
        client2.SocketRead(0, &sock2);
        VTBYTEDATA out;
        CHECK(client2.GetRecevie(0x0001, out, false) == false,
              "6-byte-or-shorter frame hits the early-return guard -- no reply recorded");
    }

    // --- SocketConnect/SocketDisConnect/SocketError driven directly ---------
    ModbusTCPClient client3;
    CHECK(client3.IsConnected() == false, "client3 starts disconnected");
    client3.SocketConnect(0, 0);
    CHECK(client3.IsConnected() == true, "SocketConnect(...) sets bConnected=true");
    client3.SocketDisConnect(0, 0);
    CHECK(client3.IsConnected() == false, "SocketDisConnect(...) sets bConnected=false");

    client3.SocketConnect(0, 0);
    TCustomWinSocket errSock;
    int errCode = 12345;
    client3.SocketError(0, &errSock, eeConnect, errCode);
    CHECK(client3.IsConnected() == false, "SocketError(...) sets bConnected=false");
    CHECK(errCode == 0, "SocketError(...) clears ErrorCode (golden: ErrorCode=0;)");
}

// ===========================================================================
//  [3] MyPLC_IO_Modbus -- InitPLCIO/PLCIOTaskCycle/PLCStatusCheck + IsOn/IsOff
// ===========================================================================
static void test_myplc_io_modbus()
{
    printf("\n[3] MyPLC_IO_Modbus: InitPLCIO/PLCIOTaskCycle/PLCStatusCheck + IsOn/IsOff\n");

    // Global PlcComm starts disconnected until InitPLCIO/Connect is driven.
    InitPLCIO("127.0.0.1", 502);
    CHECK(PlcComm.IsConnected() == true, "InitPLCIO -> PlcComm connects (Sim mode)");
    CHECK(MyPLCIOThread != 0, "InitPLCIO lazily creates MyPLCIOThread exactly once");
    TPLCIOThread* firstThread = MyPLCIOThread;
    InitPLCIO("127.0.0.1", 502);
    CHECK(MyPLCIOThread == firstThread, "a second InitPLCIO() does not recreate MyPLCIOThread");

    // Pump the task cycle a few times; iPLCIOTask must stay within {1,100}.
    bool cursorSane = true;
    for (int i = 0; i < 8; ++i)
    {
        PLCIOTaskCycle();
        if (iPLCIOTask != 1 && iPLCIOTask != 100)
            cursorSane = false;
    }
    CHECK(cursorSane, "PLCIOTaskCycle() cursor stays within the documented {1,100} set");

    // PLCStatusCheck / bPLCStatusCheck / TPLCIOThread::PLCIOProcess must not
    // crash when driven repeatedly (mirrors the golden Synchronize(PLCIOProcess)
    // tick -- see header note: this is exercised directly, NOT via Execute(),
    // which is offline-inert and never invoked here).
    for (int i = 0; i < 4; ++i)
    {
        PLCStatusCheck();
        bPLCStatusCheck();
        MyPLCIOThread->PLCIOProcess();
    }
    CHECK(true, "PLCStatusCheck/bPLCStatusCheck/PLCIOProcess pump without crash");

    MyPLCIOThread->Resume();   // offline no-op (see header note); must not crash
    CHECK(true, "TPLCIOThread::Resume() is a safe offline no-op");

    // --- TMyPLC_IO_Modbus::IsOn()/IsOff() range guard + iPLCSafetyVer branch
    TMyPLC_IO_Modbus port;
    port.SetPortInformation(0, PLCIO_INPUT_START_PORT_NUMBER, 3);
    CHECK(port.GetSlave() == 0 && port.GetPort() == PLCIO_INPUT_START_PORT_NUMBER && port.GetBit() == 3,
          "SetPortInformation stores Card/Port/Bit (GetSlave/GetPort/GetBit)");

    // Out-of-range guard: negative slave -> IsOn() false, IsOff() true.
    TMyPLC_IO_Modbus outOfRange;
    outOfRange.SetPortInformation(-1, PLCIO_INPUT_START_PORT_NUMBER, 0);
    CHECK(outOfRange.IsOn() == false, "IsOn() false for an out-of-range slave (guard clause)");
    CHECK(outOfRange.IsOff() == true, "IsOff() == !IsOn()");

    // In-range: seed bPLCInData directly and read back through IsOn() with
    // the default (non-202204) iPLCSafetyVer branch: index = Port-START+1.
    bPLCInData[0][1][3] = true;   // (iPort-PLCIO_INPUT_START_PORT_NUMBER+1)==1, iBit==3
    CHECK(port.IsOn() == true, "IsOn() reads bPLCInData at the default (+1) index when seeded true");
    bPLCInData[0][1][3] = false;
    CHECK(port.IsOn() == false, "IsOn() reflects bPLCInData==false");

    // PLC_IO_Members: every constructed TMyPLC_IO_Modbus registers itself
    // (this function alone constructs 2: port, outOfRange).
    CHECK(TMyPLC_IO_Modbus::PLC_IO_Members.size() >= 2,
          "TMyPLC_IO_Modbus ctor self-registers into the static PLC_IO_Members list");
}

// ===========================================================================
int main()
{
    printf("=============================================================\n");
    printf(" test_myplc_modbus : ClientSocket shim + ModbusTCPClient + MyPLC_IO_Modbus\n");
    printf("=============================================================\n");

    test_clientsocket_sim();
    test_modbustcpclient();
    test_myplc_io_modbus();

    printf("\n-------------------------------------------------------------\n");
    printf(" RESULT: %d passed, %d failed\n", g_pass, g_fail);
    printf("-------------------------------------------------------------\n");
    return (g_fail == 0) ? 0 : 1;
}
