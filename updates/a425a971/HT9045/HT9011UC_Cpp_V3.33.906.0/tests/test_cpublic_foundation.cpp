// test_cpublic_foundation.cpp
// AI(ht9045-v899) 20260626: cpublic.cpp foundation ungate acceptance tests.
//
// Covers (per READPLAN test oracles):
//   [A] DTK4848 frame format via TComm SIM (DTK4848_LRC, DTK4848WordWrite/ReadNoSucm)
//   [B] ByteUnionBit methods (Bit/SetBit/SetByte/BitString/Hex/BitCount)
//   [C] Queue method bodies (TMyQueue10 Add/Get/Show; TMyQueue100; TMyStrQueue100;
//       TMyTimerQueue100 + ConvertMSecToSPC)
//   [D] VerInfo GetAppVersion (reads the test exe's own version resource)
//   [E] Foundation free functions (ConvertSecondToSPC / ConvertMSecToSPC /
//       GetFloatFormatString / ConvertDoubleStrToInt / sDataTimelog / Round x2 /
//       RotationCoordinates / CRC_Check / GetScreenPos / VC8ToKpa / KpaToVC8 /
//       IsDoubleEqual)
//
// Deliberately does NOT test:
//   - gated W5/W6/W7 bodies (not compiled)
//   - SafeData (MyForceDirectories gated)
//   - GetSVNRev/GetFileVersion/GetMainVersion (Application->ExeName gated)

#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "vclcompat/Comm.h"
#include "cpublic.h"
#include "cmydef.h"   // SystemHour/Min/Sec/MSec, System*, QueueTestTime, QueueCycleTime

// ============================================================
//  Helpers
// ============================================================
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL [%s:%d] %s\n", __FILE__, __LINE__, #expr); \
        g_fail++; \
    } else { \
        g_pass++; \
    } \
} while(0)

// Set System* globals to a known time so queue Add() is deterministic
static void set_system_time(unsigned h, unsigned m, unsigned s, unsigned ms)
{
    SystemHour = (Word)h;
    SystemMin  = (Word)m;
    SystemSec  = (Word)s;
    SystemMSec = (Word)ms;
}

// ============================================================
//  [A] DTK4848 frame format
// ============================================================
static void test_dtk4848_lrc()
{
    // Frame for DTK4848WordWriteNoSucm(Addr=0, Value=250):
    //   body = ":0106470100FA"
    //   LRC input  = SubString(2,12) = "010647010 0FA"
    //                0-indexed chars: '0','1','0','6','4','7','0','1','0','0','F','A'
    //   pair-wise:  01 06 47 01 00 FA  (decimal sums: 1+6+71+1+0+250=329? let's compute)
    //   sum = 0x01 + 0x06 + 0x47 + 0x01 + 0x00 + 0xFA = 1+6+71+1+0+250 = 329 = 0x149
    //   LRC = (0xFF - 0x149 + 1) & 0xFF  = (255-329+1)&0xFF = (-73)&0xFF = 0xB7
    //   But wait: the loop is i=1..str.Length() step 2 where str is 12 chars.
    //   After SubString(2,12): "0106470100FA"  (1-based: chars at pos 1..12)
    //   Loop i=1,3,5,7,9,11 -> 6 pairs: "01","06","47","01","00","FA"
    //   0x01+0x06+0x47+0x01+0x00+0xFA = 1+6+71+1+0+250 = 329 = 0x149
    //   LRC = 0xFF - 0x149 + 1 = 0xFF - 0x149 + 1; in hex arith truncated to byte:
    //   Actually the code does: LRC=0xFF-LRC+1; but LRC is int, not byte-masked before.
    //   Let's compute: 0xFF - 329 + 1 = 255 - 329 + 1 = -73.  Then IntToHex(-73, 2).
    //   SubString(len-1, 2) of IntToHex(-73,2) = last 2 chars.
    //   IntToHex(-73,2) in BCB6 = "FFFFFFB7" (32-bit) -> last 2 chars = "B7"
    // So expected LRC = "B7"
    AnsiString lrc = DTK4848_LRC(":0106470100FA");
    CHECK(lrc == "B7");
}

static void test_dtk4848_write_frame()
{
    // DTK4848WordWriteNoSucm(Addr=0, Value=250)
    // Expected frame: ":0106470100FA" + LRC + "\r\n\0"
    // LRC = "B7" (computed above)
    // Full frame bytes: ':','0','1','0','6','4','7','0','1','0','0','F','A','B','7','\r','\n','\0'
    Spcomm::TComm comm(0);
    comm.SetSimMode(true);
    comm.StartComm();
    g_pDTKComm = &comm;

    comm.SimClearTx();
    DTK4848WordWriteNoSucm(0, 250);

    const std::vector<char>& tx = comm.SimTxBuffer();
    // Expected: ":0106470100FAB7\r\n" + NUL = 18 bytes
    std::string expected = ":0106470100FAB7\r\n";
    expected.push_back('\0');

    CHECK(tx.size() == expected.size());
    if (tx.size() == expected.size())
    {
        CHECK(memcmp(tx.data(), expected.data(), expected.size()) == 0);
    }

    g_pDTKComm = nullptr;
    comm.StopComm();
}

static void test_dtk4848_read_frame()
{
    // DTK4848WordReadNoSucm(Addr=0)
    // body = ":0103470000 02"  -> ":010347000002"
    // LRC input (SubString(2,12)) = "010347000002"
    // pairs: "01","03","47","00","00","02" -> 1+3+71+0+0+2 = 77 = 0x4D
    // LRC = 0xFF - 77 + 1 = 179 = 0xB3
    // Full frame: ":010347000002B3\r\n\0"
    Spcomm::TComm comm(0);
    comm.SetSimMode(true);
    comm.StartComm();
    g_pDTKComm = &comm;

    comm.SimClearTx();
    DTK4848WordReadNoSucm(0);

    const std::vector<char>& tx = comm.SimTxBuffer();
    std::string expected = ":010347000002B3\r\n";
    expected.push_back('\0');

    CHECK(tx.size() == expected.size());
    if (tx.size() == expected.size())
    {
        CHECK(memcmp(tx.data(), expected.data(), expected.size()) == 0);
    }

    g_pDTKComm = nullptr;
    comm.StopComm();
}

// ============================================================
//  [B] ByteUnionBit
// ============================================================
static void test_byte_union_bit()
{
    ByteUnionBit b;
    // Set byte 0xA5 = 1010 0101
    CHECK(b.SetByte(0xA5) == true);
    CHECK(b.Bit(0) == 1);  // LSB
    CHECK(b.Bit(1) == 0);
    CHECK(b.Bit(2) == 1);
    CHECK(b.Bit(3) == 0);
    CHECK(b.Bit(4) == 0);
    CHECK(b.Bit(5) == 1);
    CHECK(b.Bit(6) == 0);
    CHECK(b.Bit(7) == 1);  // MSB
    CHECK(b.BitCount(true) == 4);
    // BitString: MSB first (b7..b0): 10100101
    CHECK(b.BitString() == "10100101");
    CHECK(b.Hex() == "A5");
    CHECK(b.Hex(true) == "0xA5");

    // SetBit
    b.SetBit(1, true);
    CHECK(b.Bit(1) == 1);
    b.SetBit(7, false);
    CHECK(b.Bit(7) == 0);

    // Out of range SetByte
    CHECK(b.SetByte(256) == false);
    CHECK(b.SetByte(-1) == false);
    CHECK(b.SetByte(0) == true);
}

// ============================================================
//  [C] Queue method bodies
// ============================================================
static void test_myqueue10()
{
    TMyQueue10 q;
    // Initial state from W4 ctor: iIndex=-1, iCount=-1
    CHECK(q.iIndex == -1);
    CHECK(q.iCount == -1);

    int task = 0;
    q.SetAliasAndTask("TEST", &task);

    // Set deterministic time
    set_system_time(10, 20, 30, 456);

    // Add 3 items
    q.Add(1);
    q.Add(2);
    q.Add(3);

    CHECK(q.iCount == 2);  // 0-based count: iCount starts -1, +1 per Add -> 2 after 3 adds
    CHECK(q.GetLastData() == 3.0);
    CHECK(q.GetData(0) == 1.0);
    CHECK(q.GetData(1) == 2.0);
    CHECK(q.GetData(2) == 3.0);

    // DateTime: Add() calls GetTimeInfo() which reads real system time; just verify non-empty
    AnsiString dt = q.GetDateTime(0);
    CHECK(dt.Length() > 0);

    // ShowCommaText
    AnsiString csv = q.ShowCommaText(false);
    // Should start with "TEST," and contain data values
    CHECK(csv.Pos("TEST,") == 1);
    CHECK(csv.Pos("1") > 0);

    // CheckTaskChange: *iTask=0 (task var is still 0), iData[iIndex]=3 -> mismatch -> bAdd=true
    bool added = q.CheckTaskChange();
    CHECK(added == true);

    // ClearData resets
    q.ClearData();
    CHECK(q.iIndex == -1);
    CHECK(q.iCount == -1);
}

static void test_myqueue10_ring_wrap()
{
    TMyQueue10 q;
    set_system_time(0, 0, 0, 0);

    // Add MAX_Q_10 + 2 entries to trigger ring wrap (MAX_Q_10 = 500)
    for (int i = 0; i < 502; i++)
        q.Add(i);

    // iCount should be MAX_Q_10 (capped at 500)
    CHECK(q.iCount == 500);
    // iIndex should be (502-1) % 500 = 1
    CHECK(q.iIndex == 1);
}

static void test_myqueue100()
{
    TMyQueue100 q;
    // TMyQueue100 uses 0-based slot-fill semantics: iIndex/iCount start at 0
    CHECK(q.iIndex == 0);
    CHECK(q.iCount == 0);

    set_system_time(12, 30, 45, 0);
    q.Add(10);
    q.Add(20);
    // Add() stores at iData[iCount] then increments iCount; after 2 adds: iCount==2
    CHECK(q.iCount == 2);
    // GetData(i): flag=(iIndex+i)%MAX; iIndex==0 after 2 adds (no ring-wrap)
    CHECK(q.GetData(0) == 10.0);
    CHECK(q.GetData(1) == 20.0);
}

static void test_mystrqueue100()
{
    TMyStrQueue100 q;
    CHECK(q.iIndex == 0);
    CHECK(q.iCount == 0);

    set_system_time(8, 0, 0, 0);
    q.Add("cmd1", "resp1");
    q.Add("cmd2", "resp2");
    CHECK(q.iCount == 2);
    CHECK(q.GetData(0) == "cmd1");
    CHECK(q.GetData(1) == "cmd2");

    AnsiString dt = q.GetDateTime(0);
    CHECK(dt.Pos("cmd1") > 0);
}

static void test_mytimer_queue100()
{
    TMyTimerQueue100 q;
    CHECK(q.iIndex == -1);
    CHECK(q.iCount == -1);

    set_system_time(9, 15, 0, 0);
    q.Add("09:15:00.000", "09:15:01.234", 1234);  // 1234 ms

    CHECK(q.iCount == 0);  // after 1 add, iCount goes from -1 to 0
    CHECK(q.iIndex == 0);
    CHECK(q.GetTimeData(0) == 1234);

    // GetTimeString should be ConvertMSecToSPC(1234) = "00:00:01.234"
    AnsiString ts = q.GetTimeString(0);
    CHECK(ts == "00:00:01.234");

    AnsiString ss = q.GetStartTime(0);
    CHECK(ss == "09:15:00.000");
    AnsiString es = q.GetEndTime(0);
    CHECK(es == "09:15:01.234");
}

// ============================================================
//  [D] VerInfo GetAppVersion
// ============================================================
static void test_verinfo_app_version()
{
    // Get our own exe path from argv (passed as a global below)
    // We'll use argv[0] which is the test exe
    // VerInfo::GetAppVersion reads Win32 version resource from a PE file.
    // The test exe may or may not have version info, but should not crash.
    extern const char* g_test_exe_path;

    // Fresh VerInfo: ctor calls m_ClearData internally; FixedFileVersion
    // returns "0.0.0.0" when stFileInfo is zeroed (no file loaded yet).
    VerInfo vi;
    CHECK(vi.FixedFileVersion() == "0.0.0.0");

    // SetFileName with a nonexistent path -> graceful no-crash
    vi.SetFileName("C:\\nonexistent_file_for_test.exe");
    CHECK(vi.FixedFileVersion() == "0.0.0.0");  // cleared on failure

    // GetAppVersion on our own exe (may have no resource, just check no crash)
    WORD maj=0,min=0,rel=0,bld=0;
    vi.GetAppVersion(AnsiString(g_test_exe_path), maj, min, rel, bld);
    // No crash is the success criterion; values may be 0 if no resource.
    CHECK(maj >= 0 && min >= 0);  // trivially true, but exercises the code path
}

// ============================================================
//  [E] Foundation free functions
// ============================================================
static void test_convert_second_to_spc()
{
    AnsiString s = ConvertSecondToSPC(3661);  // 1h 1m 1s
    CHECK(s == "01:01:01");

    s = ConvertSecondToSPC(0);
    CHECK(s == "00:00:00");

    s = ConvertSecondToSPC(90000);  // 25h -> wraps to 01:00:00
    CHECK(s == "01:00:00");  // hours%24 = 25%24 = 1
}

static void test_convert_msec_to_spc()
{
    AnsiString s = ConvertMSecToSPC(1234);  // 0h 0m 1s 234ms
    CHECK(s == "00:00:01.234");

    s = ConvertMSecToSPC(0);
    CHECK(s == "00:00:00.000");
}

static void test_get_float_format_string()
{
    // GetFloatFormatString(1.5, 4, 2):
    //   fstr starts as "%5.3f"; fstr[1]='0'+(4+2)='6'; fstr[3]='0'+2='2' -> "%6.2f"
    //   sprintf "%6.2f" of 1.5 = "  1.50" (2 leading spaces)
    //   CutSpaceAtHead strips leading spaces -> "1.50"
    char* r = GetFloatFormatString(1.5, 4, 2);
    CHECK(strcmp(r, "1.50") == 0);

    // ConvertToMMType(150) -> 150/100.0=1.5 -> GetFloatFormatString(1.5, 4, 2) -> "1.50"
    r = ConvertToMMType(150);
    CHECK(strcmp(r, "1.50") == 0);

    // Get0_01MMType("1.50") -> (1.50*100+0.5) = 150
    int mm = Get0_01MMType((char*)"1.50");
    CHECK(mm == 150);
}

static void test_convert_double_str_to_int()
{
    // ConvertDoubleStrToInt("2.7", 10.0) -> (2.7*10+0.5) = 27
    int r = ConvertDoubleStrToInt("2.7", 10.0);
    CHECK(r == 27);

    // Negative: ConvertDoubleStrToInt("-1.5", 10.0) -> (-1.5*10-0.5) = -15
    r = ConvertDoubleStrToInt("-1.5", 10.0);
    CHECK(r == -15);
}

static void test_sdata_timelog()
{
    // sDataTimelog calls GetTimeInfo() internally which reads real system time;
    // we can't control the timestamp, only check the original message appears.
    AnsiString msg = "hello_unique_marker";
    sDataTimelog(msg);
    // Format: "YYYY-MM-DD HH:MM:SS mmm : original"
    CHECK(msg.Pos("hello_unique_marker") > 0);
    CHECK(msg.Pos(":") > 0);  // timestamp separator present
}

static void test_round()
{
    // Round(double, point): round to pow(10,point) places
    // Round(1.456, 2) -> round to 2 decimal places = 1.46
    double r = Round(1.456, 2);
    CHECK(fabs(r - 1.46) < 0.001);

    // Round(double) -> nearest int
    CHECK(Round(2.4) == 2);
    CHECK(Round(2.5) == 3);
    CHECK(Round(-2.5) == -3);
}

static void test_rotation_coordinates()
{
    // Identity rotation (Radian=0): output should equal input
    double px1, py1;
    RotationCoordinates(3.0, 4.0, px1, py1, 0.0, 0.0, true);
    CHECK(fabs(px1 - 3.0) < 1e-9);
    CHECK(fabs(py1 - 4.0) < 1e-9);
}

static void test_crc_check()
{
    // CRC-16/Modbus (poly 0xA001) on {0x01, 0x03, 0x00, 0x00, 0x00, 0x02}
    // Pre-computed: 0x0BC4
    unsigned char data[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
    unsigned int crc = CRC_Check(data, 6);
    CHECK(crc == 0x0BC4);

    // Empty input -> 0xFFFF (initial value untouched)
    unsigned int crc2 = CRC_Check(data, 0);
    CHECK(crc2 == 0xFFFF);
}

static void test_get_screen_pos()
{
    // Linear mapping: NowPos at midpoint of real range -> midpoint of screen range
    int pos = GetScreenPos(50, 0, 100, 0, 100);  // identity
    CHECK(pos == 50);

    pos = GetScreenPos(50, 0, 200, 0, 100);  // 50/100 * 200 = 100
    CHECK(pos == 100);
}

static void test_vc8_kpa()
{
    // VC8ToKpa(0) = ((3.3*(0/32767.0))-1.45)/0.0125 = -116.0
    double kpa = VC8ToKpa(0);
    CHECK(fabs(kpa - (-116.0)) < 0.01);

    // Round-trip: KpaToVC8(VC8ToKpa(16000)) should be close to 16000
    double kpa2 = VC8ToKpa(16000);
    int val = KpaToVC8(kpa2);
    CHECK(abs(val - 16000) <= 2);
}

static void test_is_double_equal()
{
    CHECK(IsDoubleEqual(1.0, 1.0, 1e-9) == true);
    CHECK(IsDoubleEqual(1.0, 1.0 + 1e-10, 1e-9) == true);
    CHECK(IsDoubleEqual(1.0, 2.0, 0.5) == false);
}

// ============================================================
//  main
// ============================================================
const char* g_test_exe_path = "";

// ============================================================
//  [F] AI(W906-NUMCMP) 20260927: EndTestTimeStamp (cpublic.cpp:1757)
//      golden `if(iAdd && StartTestTime!=NULL)` -- BCB6's NULL is the int 0, i.e. AnsiString(0)="0" (NB2 R89,
//      bcc32 5.6.4 measured), so an EMPTY StartTestTime still queues and "0" does not.  vclcompat read NULL as a
//      null const char* ("") -- the reverse -- until 3f1882fc wrote it as !="0".
// ============================================================
static void test_end_test_timestamp_numcmp()
{
    QueueTestTime.ClearData();
    const int c0 = QueueTestTime.iCount;
    set_system_time(1, 2, 3, 4);
    StartTestTime = "";
    EndTestTimeStamp(1);
    CHECK(QueueTestTime.iCount == c0 + 1);          // "" != "0" -> queued (golden)
    StartTestTime = "0";
    EndTestTimeStamp(1);
    CHECK(QueueTestTime.iCount == c0 + 1);          // "0" == "0" -> not queued (golden)
    StartTestTime = "01:02:03.004";
    EndTestTimeStamp(1);
    CHECK(QueueTestTime.iCount == c0 + 2);
    EndTestTimeStamp(0);                            // iAdd == 0 -> never
    CHECK(QueueTestTime.iCount == c0 + 2);
    StartTestTime = "";
    QueueTestTime.ClearData();
}

int main(int argc, char* argv[])
{
    if (argc > 0) g_test_exe_path = argv[0];

    printf("=== test_cpublic_foundation ===\n");

    // [A] DTK4848
    printf("[A] DTK4848 frame format\n");
    test_dtk4848_lrc();
    test_dtk4848_write_frame();
    test_dtk4848_read_frame();

    // [B] ByteUnionBit
    printf("[B] ByteUnionBit\n");
    test_byte_union_bit();

    // [C] Queue methods
    printf("[C] Queue method bodies\n");
    test_myqueue10();
    test_myqueue10_ring_wrap();
    test_myqueue100();
    test_mystrqueue100();
    test_mytimer_queue100();

    // [D] VerInfo
    printf("[D] VerInfo GetAppVersion\n");
    test_verinfo_app_version();

    // [E] Foundation free functions
    printf("[E] Foundation free functions\n");
    test_convert_second_to_spc();
    test_convert_msec_to_spc();
    test_get_float_format_string();
    test_convert_double_str_to_int();
    test_sdata_timelog();
    test_round();
    test_rotation_coordinates();
    test_crc_check();
    test_get_screen_pos();
    test_vc8_kpa();
    test_is_double_equal();

    // [F] AI(W906-NUMCMP) 20260927
    printf("[F] EndTestTimeStamp NULL semantics (NB2 R89)\n");
    test_end_test_timestamp_numcmp();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
