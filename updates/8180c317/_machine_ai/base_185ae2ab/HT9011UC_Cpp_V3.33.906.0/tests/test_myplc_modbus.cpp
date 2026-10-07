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
//  [4] AI(W906-PLCMODEL) 20261006: SafePlcModel=1 (Reer MOSAIC M1S COM, vendor manual 8547780 Rev.4 p.28)
//      The reply is the 34 data bytes MEASURED on the machine 20261006 (FC4 0x400 x17, read-only): System status 0x01,
//      Input status bytes 0x55 0x15 0x10 0x05, Restart byte 0 = 0x04, OSSD 0.  Bytes on the wire = high byte first per register.
// ===========================================================================
extern int iSafePlcModel;   // cmydef.cpp
extern bool bOSSDData[INPUT_MAX_Slave][2][8];   // MyPLC_IO_Modbus.cpp (not in the header)
static void test_m1s_model()
{
    printf("\n[4] SafePlcModel=1 (Reer M1S COM): FC4 0x400 x17 -> golden's byte array -> IO_Table Port 0x300..0x303\n");

    iSafePlcModel = 1;
    PlcComm.SocketConnect(0, 0);
    bScanSlave[0] = true;
    iPLCIOTask = 1;
    iTransactionID = 0x0100;
    PLCIOTaskCycle();
    CHECK(iPLCIOTask == 100, "model 1: the request went out (task 1 -> 100)");

    const unsigned char wire[34] = {
        0x00, 0x01,  0x15, 0x55,  0x05, 0x10,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,
        0x00, 0x04,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00,  0x00, 0x00 };
    char frame[9 + 34];
    frame[0] = 0x01; frame[1] = 0x00;                  // transaction id 0x0100
    frame[2] = 0x00; frame[3] = 0x00;                  // protocol
    frame[4] = 0x00; frame[5] = (char)(3 + 34);        // length = unit + fc + byte count + data
    frame[6] = 0x00; frame[7] = 0x04;                  // unit 0, FC4
    frame[8] = (char)34;                               // byte count
    for (int i = 0; i < 34; ++i) frame[9 + i] = (char)wire[i];
    TCustomWinSocket rx;
    rx.SimPushReceive(frame, (int)sizeof(frame));
    PlcComm.SocketRead(0, &rx);
    PLCIOTaskCycle();
    CHECK(iPLCIOTask == 1 && bPLCIOEffect, "model 1: the reply was taken (task 100 -> 1, bPLCIOEffect)");

    CHECK(bPLCInData[0][0][0] == true, "System status 0x01 -> bPLCInData[0][0] bit 0");
    bool b0 = true;  for (int k = 0; k < 8; ++k) if (bPLCInData[0][1][k] != (((0x55 >> k) & 1) != 0)) b0 = false;
    bool b1 = true;  for (int k = 0; k < 8; ++k) if (bPLCInData[0][2][k] != (((0x15 >> k) & 1) != 0)) b1 = false;
    bool b2 = true;  for (int k = 0; k < 8; ++k) if (bPLCInData[0][3][k] != (((0x10 >> k) & 1) != 0)) b2 = false;
    bool b3 = true;  for (int k = 0; k < 8; ++k) if (bPLCInData[0][4][k] != (((0x05 >> k) & 1) != 0)) b3 = false;
    CHECK(b0 && b1 && b2 && b3, "Input status bytes 0..3 (0x55 0x15 0x10 0x05) land at bPLCInData[0][1..4] (manual low byte first)");
    bool ossd = true;  for (int k = 0; k < 8; ++k) if (bOSSDData[0][0][k] || bOSSDData[0][1][k]) ossd = false;
    CHECK(ossd, "OSSD status bytes 0..1 = 0 -> bOSSDData all false");

    TMyPLC_IO_Modbus p300b0, p300b1, p301b0, p302b4, p303b1;
    p300b0.SetPortInformation(0, 0x300, 0);  p300b1.SetPortInformation(0, 0x300, 1);
    p301b0.SetPortInformation(0, 0x301, 0);  p302b4.SetPortInformation(0, 0x302, 4);  p303b1.SetPortInformation(0, 0x303, 1);
    CHECK(p300b0.IsOn() && !p300b1.IsOn() && p301b0.IsOn() && p302b4.IsOn() && !p303b1.IsOn(),
          "IO_Table Port 0x300+n / Bit reads Input status byte n through golden's +1 index (iPLCSafetyVer 0)");

    // model 0 keeps golden's request: back to task 1, the next send uses golden's address/count (only the cursor is visible)
    iSafePlcModel = 0;
    iPLCIOTask = 1;
    PLCIOTaskCycle();
    CHECK(iPLCIOTask == 100, "model 0 (golden) still sends");
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
    test_m1s_model();   void test_m1s_alldoors(); test_m1s_alldoors();   void test_m1s_engine_path(); test_m1s_engine_path();   /*AI(W906-PLCDOOR-2) 20261006: [6], end of file*/   void test_plc_link_lost(); test_plc_link_lost();   /*AI(W906-PLCLOST) 20261007: [7], end of file*/   // AI(W906-PLCMODEL) 20261006   // AI(W906-PLCDOOR) 20261006: [5], end of file

    printf("\n-------------------------------------------------------------\n");
    printf(" RESULT: %d passed, %d failed\n", g_pass, g_fail);
    printf("-------------------------------------------------------------\n");
    return (g_fail == 0) ? 0 : 1;
}

// ===========================================================================
//  [5] AI(W906-PLCDOOR) 20261006: model 1 makes golden's "all safety doors closed" input (IO_Table Port 310 / Bit 0) from the doors
//      the IO table puts on the PLC. Doors as mapped on the machine 20261006 (EastSun opening them one by one, software positions
//      SD1..SD7 of the HT9050 door list): 301/0, 302/6, 301/4, 302/0, 302/2, 301/2, 302/4 -> all closed = byte 1 0x15, byte 2 0x55.
// ===========================================================================
#include "cmydef.h"
#include "mysensor.h"
static void m1s_feed(unsigned char in1, unsigned char in2)
{
    PlcComm.SocketConnect(0, 0);
    bScanSlave[0] = true;
    iPLCIOTask = 1;
    static int s_id = 0x0200; iTransactionID = ++s_id;   // a fresh id per frame: the client keeps replies by id
    PLCIOTaskCycle();
    unsigned char wire[34] = {0};
    wire[1] = 0x01;                 // System status
    wire[2] = in1; wire[3] = 0x55;  // register 0x401: hi = Input byte 1, lo = Input byte 0
    wire[4] = 0x55; wire[5] = in2;  // register 0x402: hi = Input byte 3, lo = Input byte 2
    char frame[9 + 34];
    frame[0] = (char)((iTransactionID >> 8) & 0xFF); frame[1] = (char)(iTransactionID & 0xFF); frame[2] = 0x00; frame[3] = 0x00; frame[4] = 0x00; frame[5] = (char)(3 + 34);
    frame[6] = 0x00; frame[7] = 0x04; frame[8] = (char)34;
    for (int i = 0; i < 34; ++i) frame[9 + i] = (char)wire[i];
    TCustomWinSocket rx;
    rx.SimPushReceive(frame, (int)sizeof(frame));
    PlcComm.SocketRead(0, &rx);
    PLCIOTaskCycle();
}
void test_m1s_alldoors()
{
    printf("\n[5] SafePlcModel=1: all-doors-closed input (Port 310 / Bit 0) made from the PLC doors of the IO table\n");
    iSafePlcModel = 1;
    for (int i = 0; i < MAX_SAFE_DOOR_CNT; ++i) Sen[iSafeDoor[i]].ISABase = 0;

    m1s_feed(0x15, 0x55);
    CHECK(iPLCIOTask == 1 && bPLCInData[0][17][0] == false, "no door on the PLC in the IO table -> 0 (fail safe, never a silent 'closed')");

    const int sn[7]   = { SnSafeDoor1, SnSafeDoor2, SnSafeDoor3, SnSafeDoor4, SnSafeDoor5, SnSafeDoor6, SnSafeDoor7 };
    const int port[7] = { 0x301, 0x302, 0x301, 0x302, 0x302, 0x301, 0x302 };
    const int bit[7]  = { 0, 6, 4, 0, 2, 2, 4 };
    for (int i = 0; i < 7; ++i) { Sen[sn[i]].ISABase = ePLCbase; Sen[sn[i]].Port = port[i]; Sen[sn[i]].Bit = bit[i]; }

    m1s_feed(0x15, 0x55);
    TMyPLC_IO_Modbus all; all.SetPortInformation(0, 0x310, 0);
    CHECK(bPLCInData[0][17][0] == true && all.IsOn(), "every PLC door closed -> Port 310 / Bit 0 = 1 (read through golden's +1 index)");

    int bad = 0;
    for (int i = 0; i < 7; ++i)
    {
        unsigned char in1 = 0x15, in2 = 0x55;
        if (port[i] == 0x301) in1 = (unsigned char)(in1 & ~(1 << bit[i])); else in2 = (unsigned char)(in2 & ~(1 << bit[i]));
        m1s_feed(in1, in2);
        if (bPLCInData[0][17][0] != false) ++bad;
    }
    CHECK(bad == 0, "any ONE PLC door open (each of the 7 tried) -> Port 310 / Bit 0 = 0");

    m1s_feed(0x15, 0x45);           // Input byte 2 bit 4 open; bit 2/6 doors unchanged
    CHECK(bPLCInData[0][17][0] == false && bPLCInData[0][3][4] == false && bPLCInData[0][3][6] == true,
          "the per-door bits stay golden's (byte 2 bit 4 = 0, bit 6 = 1) next to the made bit");

    Sen[SnSafeDoor8].ISABase = ePLCbase; Sen[SnSafeDoor8].Port = 0x320; Sen[SnSafeDoor8].Bit = 0;
    m1s_feed(0x15, 0x55);
    CHECK(bPLCInData[0][17][0] == false, "a PLC door outside the 16 Input status bytes counts as open");

    for (int i = 0; i < MAX_SAFE_DOOR_CNT; ++i) Sen[iSafeDoor[i]].ISABase = 0;
    iSafePlcModel = 0;
}

// ===========================================================================
//  [6] AI(W906-PLCDOOR-2) 20261006: the ENGINE's path, not TMyPLC_IO_Modbus::IsOn. IO audit 20261006: Sen[] reads PLC rows through
//      MyLaneIO.IOInputBit (MyLaneIo.cpp), whose golden 0x300..0x303 cap answered `true` for SnAllSafeDoor 0x310 -> the made bit was
//      never seen; and InitialSafeDoor's temporary MARK switched the PLC doors off -> IsOff() always false. [5] passed through IsOn.
// ===========================================================================
#include "MyLaneIo.h"
extern bool Enable_PLCSafety_IO;
void W906_SafeDoorPlcEnable();
void test_m1s_engine_path()
{
    printf("\n[6] SafePlcModel=1 through the engine's path: MyLaneIO.IOInputBit for Port 0x30F / 0x310, and the PLC doors re-enabled\n");
    bPLCIO[0x310][0] = true;  bPLCIO[0x30F][7] = true;
    iSafePlcModel = 1;
    bPLCInData[0][17][0] = false;  bPLCInData[0][16][7] = true;
    const bool off310 = MyLaneIO.IOInputBit(0, 0, 0x310, 0, ePLCbase, "SnAllSafeDoor");
    const bool on30F  = MyLaneIO.IOInputBit(0, 0, 0x30F, 7, ePLCbase, "last input bit");
    bPLCInData[0][17][0] = true;
    const bool on310  = MyLaneIO.IOInputBit(0, 0, 0x310, 0, ePLCbase, "SnAllSafeDoor");
    CHECK(off310 == false && on310 == true && on30F == true,
          "model 1: Port 0x310 bit 0 reads the made all-doors bit (0 -> open, 1 -> closed); Port 0x30F reads Input byte 15");
    iSafePlcModel = 0;
    bPLCInData[0][17][0] = false;
    CHECK(MyLaneIO.IOInputBit(0, 0, 0x310, 0, ePLCbase, "SnAllSafeDoor") == true,
          "model 0 (golden): Port 0x310 is outside golden's 0x300..0x303 and answers true, unchanged");
    bPLCIO[0x310][0] = false;  bPLCIO[0x30F][7] = false;

    const bool savedPlc = Enable_PLCSafety_IO;
    Sen[SnSafeDoor1].ISABase = ePLCbase; Sen[SnSafeDoor1].Enable = false; Sen[SnSafeDoor1].Type = 0;
    Sen[SnSafeDoor9].ISABase = 0;        Sen[SnSafeDoor9].Enable = false;
    Enable_PLCSafety_IO = true; iSafePlcModel = 0;
    W906_SafeDoorPlcEnable();
    CHECK(Sen[SnSafeDoor1].Enable == false, "model 0: the temporary door MARK is left alone");
    iSafePlcModel = 1;
    W906_SafeDoorPlcEnable();
    CHECK(Sen[SnSafeDoor1].Enable == true && Sen[SnSafeDoor1].Type == 1 && Sen[SnSafeDoor9].Enable == false,
          "SafePlcIO=1 + model 1: a PLC door (ISABase 4) is enabled again (Type 1); a door not on the PLC stays off");
    Enable_PLCSafety_IO = false; Sen[SnSafeDoor1].Enable = false;
    W906_SafeDoorPlcEnable();
    CHECK(Sen[SnSafeDoor1].Enable == false, "SafePlcIO=0: nothing re-enabled");
    Sen[SnSafeDoor1].ISABase = 0;
    Enable_PLCSafety_IO = savedPlc; iSafePlcModel = 0;
}
// ===========================================================================
//  [7] AI(W906-PLCLOST-2) 20261007: EastSun「Plc斷線額外寫異常」-- W906_SafePlcLinkLost (MyPLC_IO_Modbus.cpp EOF) drives WAR16156 in
//      csystem.cpp DoSystem: lost = SafePlcIO=1 and (socket not connected, or 2 requests in a row unanswered). Driven through the REAL
//      PLCIOTaskCycle (the frame mark on the reply line, the miss on the 1 s timeout) -- the first version called the mark directly
//      and stayed green while the production call was swallowed by a `//` comment (review 20261007).
// ===========================================================================
#include <windows.h>   //AI(W906-REVIEW1-ORACLE) 20261007 laptop: was <thread> -- the oracle MinGW.org 6.3 (win32 thread model) has no std::this_thread (cStateRecord.cpp:1462, tools/wb_serve.cpp:227); ::Sleep below
#include <chrono>
bool W906_SafePlcLinkLost();
void test_plc_link_lost()
{
    printf("\n[7] safety PLC link lost (WAR16156) through PLCIOTaskCycle: SafePlcIO off / socket down / 2 timeouts / a reply\n");
    const bool savedPlc = Enable_PLCSafety_IO;
    Enable_PLCSafety_IO = false;
    CHECK(W906_SafePlcLinkLost() == false, "SafePlcIO=0: never lost (golden machines untouched)");
    Enable_PLCSafety_IO = true;
    iSafePlcModel = 1;
    PlcComm.SocketDisConnect(0, 0);
    CHECK(W906_SafePlcLinkLost() == true, "SafePlcIO=1, socket not connected: lost");
    m1s_feed(0x15, 0x55);                                       // connect + one real request/reply through PLCIOTaskCycle
    CHECK(iPLCIOTask == 1 && W906_SafePlcLinkLost() == false, "a reply taken by PLCIOTaskCycle: not lost (the production frame mark ran)");
    int timeouts = 0;
    for (int k = 0; k < 2; ++k) {                               // two requests, no reply, each timed out by hCheckTime (1 s)
        iPLCIOTask = 1;  PLCIOTaskCycle();                      // send
        ::Sleep(1100);   //AI(W906-REVIEW1-ORACLE) 20261007 laptop: was std::this_thread::sleep_for(std::chrono::milliseconds(1100)) -- not in the oracle MinGW.org 6.3
        PLCIOTaskCycle();                                       // case 100: no reply, timer off -> miss
        if (iPLCIOTask == 1) ++timeouts;
        if (k == 0) CHECK(W906_SafePlcLinkLost() == false, "one unanswered request (e.g. after a tick stall): not lost yet");
    }
    CHECK(timeouts == 2 && W906_SafePlcLinkLost() == true, "two requests in a row unanswered (socket still up, cable pulled): lost");
    m1s_feed(0x15, 0x55);
    CHECK(W906_SafePlcLinkLost() == false, "the next reply: recovered");
    iSafePlcModel = 0;
    Enable_PLCSafety_IO = savedPlc;
}
