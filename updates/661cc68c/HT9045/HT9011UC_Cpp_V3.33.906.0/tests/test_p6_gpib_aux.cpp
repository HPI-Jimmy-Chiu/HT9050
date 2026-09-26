// =============================================================================
//  test_p6_gpib_aux.cpp -- tester-comm P6, ruling 2A + Q2 = (a): the GPIB program's extra RS232 port follows the
//                          recipe; each GPIB-mode recipe is seeded once from Setup.ini [COMPort].
//
//  AI(W906-GB-P6) 20260926.  Suite name (add_test): TesterComm_P6GpibAux
//
//  Covers TesterComm/Handler/HandlerGpibAux.cpp (file part) + TesterComm/Rs232SetupCodes.h + HandlerSettings.h:
//    1. recipe index <-> Setup.ini SPComm ordinal, both directions (golden + ruling 5B indices);
//    2. the packed framing value round-trips, invalid fields give -1;
//    3. W906_GpibAuxPortOn = golden OpenTesterComm (customer-code default, general.ini [OpenTesterComm] RS232);
//    4. W906_GpibAuxSeedRecipeFile: seeds once, writes only the four [RS-232C] keys + the marker, a second run does not
//       overwrite a value saved later, a missing Setup.ini seeds the engine defaults, a non-ordinal keeps the field,
//       no recipe file -> -1;
//    5. not enabled (every ctest) -> -1 published, no restart request, no file touched.
//  Every file is in %TEMP%\ht9045_p6_gpib_aux -- never D:\HT9045\IniData, D:\GPIB9045 or D:\RS232Standard.
// =============================================================================
#include "TesterComm/Handler/HandlerGpibAux.h"
#include "TesterComm/HandlerSettings.h"
#include "TesterComm/Rs232SetupCodes.h"
#include "common.h"
#include "cmydef.h"
#include <windows.h>
#include <cstdio>
#include <string>

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::string g_dir;

static AnsiString PathOf(const char* name) { return AnsiString((g_dir + "\\" + name).c_str()); }

static void WriteText(const char* name, const char* text)
{
    FILE* f = std::fopen((g_dir + "\\" + name).c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
}

static void Remove(const char* name) { ::DeleteFileA((g_dir + "\\" + name).c_str()); }

int main()
{
    printf("TesterComm_P6GpibAux\n");
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    g_dir = std::string(tmp) + "ht9045_p6_gpib_aux";
    ::CreateDirectoryA(g_dir.c_str(), 0);

    // 1. code mappings: every recipe index survives index -> ordinal -> index
    bool ok = true;
    for (int i = 0; i <= 3; ++i) ok = ok && W906_Rs232BitLengthFromCode(W906_Rs232ByteSizeCode(i)) == i;
    CHECK(ok, "1. Bit Length 0..3 (7/8/5/6 bits) round-trips");
    ok = true;
    for (int i = 0; i <= 2; ++i) ok = ok && W906_Rs232StopBitFromCode(W906_Rs232StopBitsCode(i)) == i;
    CHECK(ok, "1. Stop Bit 0..2 (1/2/1.5) round-trips");
    ok = true;
    for (int i = 0; i <= 4; ++i) ok = ok && W906_Rs232ParityFromCode(W906_Rs232ParityCode(i)) == i;
    CHECK(ok, "1. Parity 0..4 (Even/Odd/None/Mark/Space) round-trips");
    CHECK(W906_Rs232BitLengthFromCode(7) == -1 && W906_Rs232StopBitFromCode(3) == -1 && W906_Rs232ParityFromCode(5) == -1,
          "1. a value that is not an SPComm ordinal -> -1");
    CHECK(W906_Rs232BitLengthFromCode(2) == 0 && W906_Rs232StopBitFromCode(0) == 0 && W906_Rs232ParityFromCode(2) == 0,
          "1. Setup.ini 7 / 1 / Even -> recipe 0 / 0 / 0 (golden)");

    // 2. packed value
    {
        int baud = 0, bs = 0, sb = 0, pa = 0;
        const int v = testercomm::HsPackAuxFraming(115200, 3, 2, 4);
        CHECK(v >= 0 && testercomm::HsUnpackAuxFraming(v, &baud, &bs, &sb, &pa) && baud == 115200 && bs == 3 && sb == 2 &&
                  pa == 4, "2. pack / unpack 115200 8 2 Space");
        CHECK(testercomm::HsPackAuxFraming(0, 2, 0, 2) == -1 && testercomm::HsPackAuxFraming(9600, 4, 0, 2) == -1 &&
                  testercomm::HsPackAuxFraming(9600, 2, 3, 2) == -1 && testercomm::HsPackAuxFraming(9600, 2, 0, 5) == -1,
              "2. baud 0 or an out-of-range field -> -1");
        CHECK(!testercomm::HsUnpackAuxFraming(-1, &baud, &bs, &sb, &pa), "2. -1 unpacks to 'not published'");
        RS232_DATA rd = { 19200, 1, 1, 2 };   // 8 bits, 2 stop, None
        CHECK(W906_GpibAuxPackRecipe(rd) == testercomm::HsPackAuxFraming(19200, 3, 2, 0), "2. recipe -> packed ordinals");
    }

    // 3. port on (golden OpenTesterComm)
    Remove("general.ini");
    CHECK(W906_GpibAuxPortOn(PathOf("general.ini"), CC_HONPREC_QC), "3. no general.ini, customer 0 -> on");
    CHECK(W906_GpibAuxPortOn(PathOf("general.ini"), CC_MAXIM), "3. no general.ini, customer 985 -> on");
    CHECK(!W906_GpibAuxPortOn(PathOf("general.ini"), 910), "3. no general.ini, customer 910 -> off");
    WriteText("general.ini", "[OpenTesterComm]\r\nRS232=0\r\n");
    CHECK(!W906_GpibAuxPortOn(PathOf("general.ini"), CC_HONPREC_QC), "3. RS232=0 turns customer 0 off");
    WriteText("general.ini", "[OpenTesterComm]\r\nRS232=1\r\n");
    CHECK(W906_GpibAuxPortOn(PathOf("general.ini"), 910), "3. RS232=1 turns customer 910 on");

    // 4. seeding
    {
        RS232_DATA rd = { 0, 0, 0, 0 };
        AnsiString detail;
        Remove("Tester.Data");
        CHECK(W906_GpibAuxSeedRecipeFile(PathOf("Tester.Data"), PathOf("Setup.ini"), &rd, &detail) == -1,
              "4. no recipe file -> -1");

        WriteText("Tester.Data", "[Mode]\r\nTester Type=1\r\n[RS-232C]\r\nType=0\r\nBin Count=32\r\nBaudRate=0\r\n"
                                 "Bit Length=0\r\nStop Bit=0\r\nParity=0\r\n");
        WriteText("Setup.ini", "[COMPort]\r\nCommName=COM5\r\nBaudRate=19200\r\nByteSize=3\r\nStopBits=2\r\nParity=0\r\n");
        const AnsiString td = PathOf("Tester.Data");
        const int r = W906_GpibAuxSeedRecipeFile(td, PathOf("Setup.ini"), &rd, &detail);
        CHECK(r == 1, "4. first run seeds (returns 1)");
        CHECK(ReadIniData(td, "RS-232C", "BaudRate", -1) == 19200 && ReadIniData(td, "RS-232C", "Bit Length", -1) == 1 &&
                  ReadIniData(td, "RS-232C", "Stop Bit", -1) == 1 && ReadIniData(td, "RS-232C", "Parity", -1) == 2,
              "4. file now 19200 / 8 bits / 2 stop / None (Setup.ini 19200 / _8 / _2 / None)");
        CHECK(ReadIniData(td, "RS-232C", W906_GPIBAUX_MARKER, 0) == 1, "4. marker written");
        CHECK(ReadIniData(td, "Mode", "Tester Type", -1) == 1 && ReadIniData(td, "RS-232C", "Bin Count", -1) == 32 &&
                  ReadIniData(td, "RS-232C", "Type", -1) == 0, "4. other keys untouched");
        CHECK(rd.Baud_Rate == 19200 && rd.Bit_Length == 1 && rd.Stop_Bit == 1 && rd.Parity == 2,
              "4. in-memory copy updated to the written values");
        CHECK(detail.Pos("BaudRate 0->19200") > 0, "4. detail says old -> new");

        WriteIniData(td, "RS-232C", "BaudRate", 4800);   // the operator saves another value later
        rd.Baud_Rate = 4800;
        CHECK(W906_GpibAuxSeedRecipeFile(td, PathOf("Setup.ini"), &rd, &detail) == 0, "4. second run: marker -> 0");
        CHECK(ReadIniData(td, "RS-232C", "BaudRate", -1) == 4800 && rd.Baud_Rate == 4800,
              "4. second run does not overwrite the value saved later");

        // missing Setup.ini -> the engine's defaults (9600 / _7 / _1 / Even = recipe 0 / 0 / 0)
        WriteText("Tester.Data", "[RS-232C]\r\nBaudRate=19200\r\nBit Length=1\r\nStop Bit=1\r\nParity=1\r\n");
        Remove("Setup.ini");
        RS232_DATA rd2 = { 19200, 1, 1, 1 };
        CHECK(W906_GpibAuxSeedRecipeFile(td, PathOf("Setup.ini"), &rd2, &detail) == 1 && rd2.Baud_Rate == 9600 &&
                  rd2.Bit_Length == 0 && rd2.Stop_Bit == 0 && rd2.Parity == 0,
              "4. missing Setup.ini seeds the engine defaults 9600 / 7 / 1 / Even");

        // a ByteSize that is not an SPComm ordinal keeps the recipe's Bit Length
        WriteText("Tester.Data", "[RS-232C]\r\nBaudRate=9600\r\nBit Length=1\r\nStop Bit=0\r\nParity=0\r\n");
        WriteText("Setup.ini", "[COMPort]\r\nBaudRate=38400\r\nByteSize=7\r\nStopBits=0\r\nParity=2\r\n");
        RS232_DATA rd3 = { 9600, 1, 0, 0 };
        CHECK(W906_GpibAuxSeedRecipeFile(td, PathOf("Setup.ini"), &rd3, &detail) == 1 && rd3.Baud_Rate == 38400 &&
                  rd3.Bit_Length == 1 && ReadIniData(td, "RS-232C", "Bit Length", -1) == 1,
              "4. ByteSize=7 (not an ordinal) keeps Bit Length; the other fields are seeded");
    }

    // 5. not enabled (no wb_serve composition root): inert
    {
        W906_GpibAuxEnable(false);
        testercomm::HsGpibAuxFraming().store(12345);
        InitialOK = true;
        TestIF.iTestType = GPIB_MODE;
        W906_GpibAuxBeforeBridgeStart(GPIB_MODE);
        CHECK(testercomm::HsGpibAuxFraming().load() == -1, "5. not enabled -> -1 published (golden Setup.ini)");
        CHECK(!W906_GpibAuxNeedsRestart(true), "5. not enabled -> no restart request");
    }

    printf("TesterComm_P6GpibAux: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
