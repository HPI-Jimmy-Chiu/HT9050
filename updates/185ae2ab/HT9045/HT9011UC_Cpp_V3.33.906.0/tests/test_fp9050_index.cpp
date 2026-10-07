// =============================================================================
//  tests/test_fp9050_index.cpp -- ST01-C (W-77) slice 1 (SAFETY): the HT9050 Index FinePitch flow
//  (atester_FinePitch.cpp = Frank's V910 DoTestHeadMotorFP + MoveIndexZ, NO caller in the tree).
//  AI(W906-ST01C) 20261005 (St01): NEW FILE.  Plan: D:\AI_TempFile\st01e-c-fp-plan-20261005.md slice 1 tests.
//
//  One source, two ctests:
//    FP9050_Pure  (-DFP9050_PURE, no library linked): [P] the pure rules of IndexZFinePitchCore.h, and the source pins
//                 [F1] fidelity -- every 910 code line equals its port line after normalisation (FNV-1a table below,
//                      generated from the cp950 910 text by D:\AI_TempFile\st01e-c-fp-run\gen_c1.py), except the listed
//                      edit lines (count pinned), which keep every `case N:` / `Task=` / `return` token of the 910 line
//                      and carry `AI(W906-ST01C)`;
//                 [F2] census (comments, string / char literals, raw strings and #if 0 stripped): 0 live uses of the FP
//                      entry points / MoveIndexZ / the FP helpers outside atester_FinePitch.cpp/.h and tests; the live
//                      DoAllProcess Index slot (the line before `CSYS_TICK(CT_TESTHEAD);`) is still `DoTestHeadMotor();`;
//                      atester_FinePitch.cpp sits on the E-044 line of add_library(ht9045_sm) before its trailing `#`;
//                 [F11] the servo-off arms of DoTestZContactModeStart / End stay unreachable (no live Task=200/300/400 in
//                      their case-1 / case-100 arms; only 910 :296 inside case 300);
//                 [F13] no WriteIniData / CheckAndReadIniData* in the new files; abs( only at 910 :2443 (golden's no-op on
//                      the atoi of edTorue0, never on a raw 6077h).
//                 Census root: W906_ST01C_CENSUS_ROOT (red proofs point it at a scratch copy) else W906_SRC_ROOT.
//    FP9050_Index (god stack): runtime, the functions called DIRECTLY tick by tick.
//                 SIM: [F3] the index check 1 -> 11 -> 9 -> ... -> 12101 -> 12300 -> 121 -> 122100 -> 122110 -> 122 -> 130
//                      -> 15000 -> 1550 -> 1600 -> 600, then one IC cycle (pick from the In shuttle -> 130 -> 180 press ->
//                      210 (the test answers for the tester) -> 1500 -> 1600 -> place on the Out shuttle -> 5000), the
//                      Z1 target sequence and the shuttle handshake flags; [F12] no protection acts in SIM.
//                 SHIP (real TGaliRouteCore bound to a fake 1203, the tests/test_indexz_autoheight_1203.cpp fixture):
//                      [F4] MoveIndexZ reaches the 1203 through the route (and 910's MotorMove cannot: TMyGALILMotor
//                      without a card); [F5] the 910 CCD-Y interlock + Q4b's one message after 5 s; [F6] FP 12110 E-044;
//                      [F7] the 12000 / 12200 torque-limit writes reach the real iWriteAndCheckMotorTorque; [F8] the
//                      E-038 value path -> 12110 compare -> 12111 / 12112 WAR0321; [F9] W-44 entry + per tick, lifts never
//                      stopped; [F10] P7 on the 7 state machines, each fault kind mid-press -> ST in the same tick.
//  No machine file is read or written: Gerneral.ini is a %TEMP% copy (as test_indexz_autoheight_1203).
// =============================================================================
#include "IndexZFinePitchCore.h"
#include <windows.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#ifndef FP9050_PURE
#include "vclcompat/vcl_compat.h"
#include "atester_shims.h"                // COM2
#include "atester.h"                      // iTestHeadMotorTask, DoTestHeadMotorDelay
#include "aTester_Front.h"                // iTestYFrontTask, iFrontTestSuckICTask, iFrontTestDestroyICTask, fIndexNeedTest
#include "atester_FinePitch.h"
#include "cmydef.h"
#include "common.h"                       // INIFileGeneral
#include "Config.h"                       // IniConfig
#include "CosFunction.h"
#include "cprod.h"                        // Prod / Tech / TestIF / TestIF_File / DeviceForm
#include "FormsFacade.h"                  // fMain
#include "MachineType.h"
#include "canary_support.h"               // W906_ShowMyMessage_* / W906_ShowErrorMessage_*
#include "aHotPlateSubstrate.h"           // FTestSuck / FLCarryKit / FRCarryKit / InArmSuck
#include "acarry.h"                       // b1ShuttleMoveToLeft
#include "Motor/mymotor.h"
#include "Motor/HTMotor.h"
#include "Motor/mySimMotor.h"
#include "Motor/myGALILmotor.h"           // TMyGALILMotor
#include "Motor/GaliRoute.h"
#include "EtherCAT/Pci1203GaliRouteCore.h"
#include "IndexZTorqueCore.h"
#include "IndexZTorque1203.h"
#include "Ht9050TorqueWait.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"
extern bool IndexZCanMove[2];
void InitAllProcessTask();                                                  // csystem.cpp (HOME's cursor reset; no header)
extern int (*W906_Pci1203TorqueLimitHook)(int motIndex, int value01pct, AnsiString* why);   // rs232.cpp
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(c, msg) do { if (c) { g_pass++; std::printf("  PASS %s\n", msg); } \
                           else   { g_fail++; std::printf("  FAIL %s  [line %d]\n", msg, __LINE__); } } while (0)

// @@TABLE-BEGIN (generated by D:\AI_TempFile\st01e-c-fp-run\gen_c1_table.py from gen_c1.py's c1_table.json -- do not edit)
static const int kK = 128;      // port line = 910 atester_FinePitch.cpp line + kK
static const int kK2 = -3454;  // port line = 910 Motor/mymotor.cpp line + kK2 (MoveIndexZ :6626-6643)
static const int kNEditsFp = 35, kNEditsMi = 3;   // pinned: a new edit must be added here AND in gen_c1.py
static const int kEditsFp[] = {
    2, 13, 16, 17, 24, 25, 26, 33, 34, 35, 37, 38, 45, 47, 58, 62,
    86, 89, 245, 343, 981, 1013, 1200, 1422, 1439, 1473, 1563, 1676, 1851, 2015, 2137, 2225,
    2416, 2932, 2937,
};
static const int kEditsMi[] = {
    6632, 6633, 6639,
};
struct EditCode { int line; const char* code; };   // the normalised 910 code of each edit line
static const EditCode kEditCode[] = {
    {2, "#pragma hdrstop"},
    {13, "#include \"MyMotor.h\""},
    {16, "#include \"csetup.h\""},
    {17, "#include \"rs232.h\""},
    {24, "#include \"main.h\""},
    {25, "#include \"note.h\""},
    {26, "#include \"mymessbox.h\""},
    {33, "#include \"iosetview.h\""},
    {34, "#include \"automation.h\""},
    {35, "#include \"uShowMessage.h\""},
    {37, "#include \"CCDInterface.h\""},
    {38, "#include \"uLotInfo.h\""},
    {45, "#include \"InterfaceSYS.h\""},
    {47, "#include \"MyKitSuck.h\""},
    {58, "#include \"MyMotor.h\""},
    {62, "#pragma package(smart_init)"},
    {86, "switch(Task)"},
    {89, "if(TestIF_File.bEnableCalCCD)"},
    {245, "switch(Task)"},
    {343, "switch(Task)"},
    {981, "HTimer DoFrontTestDestroyICFPDelay;"},
    {1013, "switch(Task)"},
    {1200, "if(MOT[MTestZ1].ReadPos()<(Prod.TestZ1_Place+10))"},
    {1422, "HTimer DoTestYDelay;"},
    {1439, "switch(Task)"},
    {1473, "if(MOT[MTestZ1].ReadPos()!=Prod.TestZ1_Safe)"},
    {1563, "InitContactModeStart();"},
    {1676, "fMain->sbStateRecordClick(fMain->sbStateRecord);"},
    {1851, "switch(Task)"},
    {2015, "switch(Task)"},
    {2137, "Task=2;"},
    {2225, "fShowMessage->FormClick(fShowMessage);"},
    {2416, "fMain->lbArm0Torque->Caption=\"1:Reading\";"},
    {2932, "fContact->InitROILearningTask();"},
    {2937, "if(fContact->Do_ROILearning(true))"},
    {6632, "{"},
    {6633, "if(iPos==MOT[MTestZ1].ReadPos())"},
    {6639, "bResult=MOT[MTestZ1].MotorMove(iPos);"},
};
static const unsigned long long kFp910[3040] = {   // FNV-1a 64 of the normalised code per 910 line (0 = no code)
    0xfdbd1b9e55bff1b6ull, 0x69454192e089fed0ull, 0x0000000000000000ull, 0x77bc40bf8e19cba7ull, 0x0000000000000000ull, 0x48b8302bdba552bcull,
    0x0000000000000000ull, 0xad6d8a833788115full, 0x417b94d64ed771ccull, 0x0000000000000000ull, 0x0000000000000000ull, 0x5dc2f47a3ff9ef5full,
    0xd7caff855ed2c849ull, 0x7051ee32548f2774ull, 0x4642f010af4a18a6ull, 0x504696a353106fceull, 0xa32bf08b22701d4eull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xa568326b6b9607d5ull, 0x5238c99c6258b0a0ull, 0x9114a2b0852fc4efull,
    0x64cb4f425f5179a4ull, 0xd4d8ab0b2b091a69ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x5766baab30d074b7ull, 0x6a0c3f5a23a0015dull, 0x785a4050e3ae5da3ull, 0xc5a44de9f4024363ull,
    0xdec9c949e474df67ull, 0x73bab935bfe984deull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xc2c86d21ca1e1ba8ull, 0x0000000000000000ull, 0xefc18b9b83bd15e4ull, 0x70c08563589f54faull,
    0xcccd52af10f1dffcull, 0xe5f2868e40f95a3aull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xd7caff855ed2c849ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0xe197a700def42ef1ull, 0x0000000000000000ull, 0xe8400ce30bf78d1bull, 0x470b2c1576c90086ull, 0x0000000000000000ull,
    0x02fb41d7909fe301ull, 0xf29a7c4ed514bbb8ull, 0xaabd2dc581ece977ull, 0x412ab2f508db75d2ull, 0x9365df8586a0ed12ull, 0xc242a6b953dc22feull,
    0x6f966a66ee2c3e7aull, 0x0000000000000000ull, 0x20f838a5566f7bdeull, 0x83cf85b5a7afcab5ull, 0xaf63f64c860218baull, 0x878b41ee4e9cf219ull,
    0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x630912a4cd9d4427ull, 0xaf63f64c860218baull, 0x3af83bf8044f2756ull, 0x6a34d2ebab578114ull,
    0x0000000000000000ull, 0x0fd0b366ddc80db9ull, 0xaf63f64c860218baull, 0x80fdf78d8d829124ull, 0x201aae98d0aa5c38ull, 0xaf63f64c860218baull,
    0x0000000000000000ull, 0xbc8ba655ff4ac4e4ull, 0xfa69b29e801006e6ull, 0x9951ebb938d4f5dcull, 0x3129b5ae586b6417ull, 0x0000000000000000ull,
    0x3fc0db4687b464a2ull, 0xaf63f04c86020e88ull, 0xdab3e6c0e68a2dd6ull, 0xaf63f64c860218baull, 0x1605b1d648b14429ull, 0xbbf288165dedfdbcull,
    0xcbe8a8833e1453fdull, 0x128e3d09a5db0bdfull, 0xc46b4f1a2a853aacull, 0x3edd57c0325697a7ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0x3edd57c0325697a7ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xc85a70d7a4830f19ull, 0xd5f4678789802fe4ull, 0x0000000000000000ull, 0xaf63f64c860218baull,
    0x0ebd9b383ebb146dull, 0xd32271d551eab26eull, 0x0000000000000000ull, 0x1605b1d648b14429ull, 0xbbf288165dedfdbcull, 0xcbe8a8833e1453fdull,
    0x128e3d09a5db0bdfull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0x3edd57c0325697a7ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0xcd4b40878498bff5ull, 0x0000000000000000ull, 0xaf63f64c860218baull, 0x0ebd9b383ebb146dull, 0xd32271d551eab26eull, 0x32d3fdf5d8ba08b3ull,
    0x62ee9fe10678525dull, 0x1605b1d648b14429ull, 0xbbf288165dedfdbcull, 0xcbe8a8833e1453fdull, 0x128e3d09a5db0bdfull, 0xc46b4f1a2a853aacull,
    0x0000000000000000ull, 0x3edd57c0325697a7ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x4eee912b74e98a14ull, 0x77ba93545dc9f9c5ull,
    0xaf63f64c860218baull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f64c860218baull,
    0x5d0ab28f08658d50ull, 0xaf63f64c860218baull, 0x1605b1d648b14429ull, 0xc4a5f8500e409c8bull, 0xaf63f64c860218baull, 0x5f4ddd808bb805d4ull,
    0x2078726e97f0b5a5ull, 0xf4d69256873d1c26ull, 0xaf63f04c86020e88ull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xeeee9524f60f6133ull, 0x5d0ab28f08658d50ull, 0xaf63f64c860218baull,
    0x2dc65edd263e4ec7ull, 0x873cb397996ab9ebull, 0xd4f5e3ae4c47a951ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x39c49d1c02d9f13eull,
    0x686b22a8a69d2417ull, 0xaf63f64c860218baull, 0xc4c1e219959df297ull, 0x96c25453041bf6b5ull, 0x3c8ad9de8c456654ull, 0x5ec285e9e162f852ull,
    0x0000000000000000ull, 0x287e33ec012c1bf6ull, 0x7604c1fc0a712122ull, 0xc46b4f1a2a853aacull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x6ed3c15523f5ee25ull, 0xa6275dedc3745a81ull, 0xa119de2919e40183ull, 0xef52c660159e0a72ull, 0x6b2bb216769596edull, 0xa4f65e5116c7f5e8ull,
    0x019703f46d876c08ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0xf2755d5cb615b025ull, 0x3e0eecef74c5c43cull, 0xaf63f64c860218baull, 0x4274b2df1673038aull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x881345a22d46dcdcull, 0xaf63f64c860218baull, 0xf7ee2a5269763275ull, 0x0000000000000000ull, 0x0fd0b366ddc80db9ull, 0xaf63f64c860218baull,
    0x80fdf78d8d829124ull, 0x202078a52daeb85full, 0xaf63f64c860218baull, 0x3edd57c0325697a7ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0x3edd57c0325697a7ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xc85a70d7a4830f19ull, 0x4eee912b74e98a14ull, 0x77ba93545dc9f9c5ull, 0xaf63f64c860218baull,
    0x0a3788e6424feb26ull, 0xaf63f64c860218baull, 0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0xeeee9524f60f6133ull, 0x26633bc669a110f0ull, 0x8e3b76b03a754f56ull, 0x80a2ccac8584adf1ull, 0x835c7102ca1fa802ull, 0xd4f5e3ae4c47a951ull,
    0x0000000000000000ull, 0x287e33ec012c1bf6ull, 0xa99887e0ee2d7e2bull, 0xc46b4f1a2a853aacull, 0xc85a70d7a4830f19ull, 0x39c49d1c02d9f13eull,
    0xa6275dedc3745a81ull, 0xa119de2919e40183ull, 0xef52c660159e0a72ull, 0x26ead2c1161cdbb8ull, 0xf42a39943d614663ull, 0xb79fab40ee9b4c6dull,
    0xaf63f64c860218baull, 0x0bddd09d18a99cf3ull, 0x873cb397996ab9ebull, 0x5ec285e9e162f852ull, 0xaf63f04c86020e88ull, 0xdfeb0d930b410457ull,
    0xaf63f64c860218baull, 0x8a1fd9b73f7d15e0ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x6ed3c15523f5ee25ull, 0x686b22a8a69d2417ull,
    0xaf63f64c860218baull, 0xa99887e0ee2d7e2bull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x0e67414ea4bfc618ull, 0x0a3788e6424feb26ull,
    0xaf63f64c860218baull, 0x0000000000000000ull, 0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull,
    0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x84a963832f2fd9f6ull, 0xaf63f64c860218baull, 0x2df71cac3d18e605ull,
    0x5026107b3fcadeb2ull, 0x0a4152858424d90eull, 0x66c88a519a18f94full, 0x66c88a519a18f94full, 0xbea8fca8d7859d95ull, 0x3b6e32fa0e908e17ull,
    0x327e8eaf96932cc7ull, 0x66c88a519a18f94full, 0x66c88a519a18f94full, 0xbea8fca8d7859d95ull, 0x0000000000000000ull, 0x6db7883376a98940ull,
    0x66c88a519a18f94full, 0x66c88a519a18f94full, 0xbea8fca8d7859d95ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x3ff26621006d9215ull,
    0xda7b6928b297d994ull, 0x150d665ae093c693ull, 0xc910e46aa9c01b6full, 0x0000000000000000ull, 0x98ddd43682199a40ull, 0x0000000000000000ull,
    0x0fd0b366ddc80db9ull, 0xaf63f64c860218baull, 0x80fdf78d8d829124ull, 0xcfc10df61b8a8056ull, 0xaf63f64c860218baull, 0x36ad2878cb3b391dull,
    0xaf63f64c860218baull, 0xa07530bae0e0a339ull, 0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0xb899dcfa3aeecdc0ull, 0xaf63f64c860218baull, 0x514baf42a09b7a38ull, 0xaf63f64c860218baull, 0xa07530bae0e0a339ull, 0x7604c1fc0a712122ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x96621d0966c7e047ull, 0xaf63f64c860218baull, 0xbf003f4599a3cb72ull,
    0xaf63f64c860218baull, 0xa07530bae0e0a339ull, 0xfa69b29e801006e6ull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0xc32cc45c35912dd8ull, 0x0000000000000000ull, 0x62ee9fe10678525dull, 0xd4f5e3ae4c47a951ull, 0x0000000000000000ull,
    0xc85a70d7a4830f19ull, 0x2c52bc25190c0dfdull, 0xe2bf4970f0cf3fa4ull, 0xaf63f64c860218baull, 0xab3abc36c1348e5full, 0xc4a87c5288debf69ull,
    0xd4f963ae4c4ab8a8ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x39c49d1c02d9f13eull, 0xc32cc45c35912dd8ull, 0x62ee9fe10678525dull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x912b5a6ea9d0a85cull, 0xfa0978da149e2f67ull, 0xaf63f64c860218baull, 0x98b843cf2b1f29dcull,
    0x649b5db46bfa7b38ull, 0xf0aaf0ad8c8a5627ull, 0x7c172e255e1f002full, 0xc5d100b761082e2aull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x33edc24cdb48373full, 0x1702d5c8cc7decebull, 0x0000000000000000ull, 0x1605b1d648b14429ull, 0xbbf288165dedfdbcull,
    0xcbe8a8833e1453fdull, 0x4dcdae9c1884ace5ull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0xab3abc36c1348e5full, 0xc4a87c5288debf69ull,
    0xd4f963ae4c4ab8a8ull, 0xc85a70d7a4830f19ull, 0x39c81f1c02dd03fbull, 0x0000000000000000ull, 0x8c9150b8d20073c5ull, 0xaf63f64c860218baull,
    0x691767e911054a24ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xa119de2919e40183ull, 0x96621d0966c7e047ull,
    0xaf63f64c860218baull, 0xa81b9b233090a194ull, 0xaf63f64c860218baull, 0xa4722c19e077145eull, 0x63ee606ff81aa76aull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x544d8552da2abf6dull, 0xaf63f64c860218baull, 0xd46d3d804f954049ull, 0xaf63f64c860218baull,
    0xb70f8f6ee8fad001ull, 0x85b703ccaa230db9ull, 0x7f2b6c605332dd30ull, 0x30a56db92da4e1e1ull, 0x0000000000000000ull, 0x9b2f18fb33af6494ull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xbaf7c1881626ab53ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x33edc24cdb48373full, 0xaf63f64c860218baull, 0xf45445416b6e361cull, 0x6b4a7398916eedf2ull, 0xaf63f64c860218baull, 0x88b91f75a1578e4bull,
    0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x433a0608d18a4deeull, 0xaf63f04c86020e88ull, 0x0c9c230b44485c21ull,
    0xa4722c19e077145eull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x9e76e810a3e23cb7ull,
    0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x33738e0c7490ae55ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull,
    0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull, 0xaa8fe05a54f3edd4ull, 0xe3175f0baabc34a1ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x75493b8d2c241fd7ull, 0xaf63f64c860218baull, 0x1605b1d648b14429ull,
    0xc4a5f8500e409c8bull, 0xaf63f64c860218baull, 0x5f4ddd808bb805d4ull, 0x2078726e97f0b5a5ull, 0x5903ae85c20d0c04ull, 0xaf63f04c86020e88ull,
    0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull,
    0x5df0fab4f572904aull, 0xba1183ede66a8203ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x3b324207f0b126efull, 0x0000000000000000ull,
    0xdc590cae501a5a06ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x42dac81c081e39d9ull, 0xd8b0c6a0c38a0512ull, 0x287e33ec012c1bf6ull,
    0xcfce21c01bb431d1ull, 0xaf63f64c860218baull, 0xc7b5e854afb615f3ull, 0x67acb615ec0126ccull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xc46b4f1a2a853aacull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full,
    0xaf63f64c860218baull, 0xbfff63417d337bd2ull, 0xaf63f64c860218baull, 0xd27b0118c37fd8d7ull, 0xaf63f64c860218baull, 0x3e4f2f6357dd437eull,
    0xaf63f64c860218baull, 0x8cc6b005067c0657ull, 0xc75ec777d7229644ull, 0xaf63f04c86020e88ull, 0x4a7cad6d86b410b4ull, 0xaf63f64c860218baull,
    0xfb45d9c0bfb39bc4ull, 0xaf63f04c86020e88ull, 0x808959fdbf58167bull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0x9e6c55e464fc6ab1ull, 0xaf63f64c860218baull, 0xc75ec777d7229644ull, 0x33ce6168afe5843dull, 0x808959fdbf58167bull, 0x0000000000000000ull,
    0xaf63f04c86020e88ull, 0xb7bda924ed5e3b1full, 0xaf63f64c860218baull, 0x808959fdbf58167bull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0xf46590463b03b8d1ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0x3e4f2f6357dd437eull, 0x8cc6b005067c0657ull, 0x808959fdbf58167bull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xb352a172963343dfull, 0xaf63f64c860218baull, 0x089dfc420109f1dcull,
    0xaf63f64c860218baull, 0x44333464202ec732ull, 0xf46590463b03b8d1ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x6996c10e9c341e40ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full,
    0xaf63f64c860218baull, 0x6100c39e48e5b4a6ull, 0xaf63f64c860218baull, 0xd506ebae4c562a1cull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x43ba87e0a6884896ull, 0xaf63f64c860218baull, 0x10a566c9e9bae40bull,
    0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full,
    0xaf63f64c860218baull, 0xbfff63417d337bd2ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x4e3fa0b0750fd0b1ull, 0xfb45d9c0bfb39bc4ull,
    0x0000000000000000ull, 0x28fc8d926fd650a1ull, 0xaf63f64c860218baull, 0x8cc6b005067c0657ull, 0xc75ec777d7229644ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0x33ce6168afe5843dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x43ba87e0a6884896ull,
    0xaf63f64c860218baull, 0x96621d0966c7e047ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x53811cfd32fb218full,
    0xa99887e0ee2d7e2bull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x39d5a71c02e8756full, 0x84133e2f432fcb60ull, 0xcc6736d0ce454a99ull,
    0xaf63f64c860218baull, 0xe56f35ae555e9f3bull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x4bf16f1c0d635528ull, 0xc32cc45c35912dd8ull,
    0xe572bdae5561bc2aull, 0xc85a70d7a4830f19ull, 0x4bf4791c0d659bfdull, 0x0000000000000000ull, 0x4ef2e93eec883a6dull, 0x85c2101eaf626b7dull,
    0x95e6c551ea5b2e7eull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull, 0xead38cbb493a4312ull,
    0x9b881658afed2417ull, 0x0000000000000000ull, 0x6100c39e48e5b4a6ull, 0xaf63f64c860218baull, 0xcae0ede623a57106ull, 0x76e4ef22a48d2048ull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xf5c008d52db77843ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x563b78aa968260d2ull, 0xaf63f64c860218baull, 0x85c2101eaf626b7dull, 0x0000000000000000ull,
    0x0dacfec9dd4a2f61ull, 0xaf63f64c860218baull, 0x6f0a3af304b1d43bull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0x18f6a702d4e52bd4ull, 0x1edd03b63c590d52ull, 0x7f2b6c605332dd30ull, 0x38145d1d103593d6ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x09f45ab54199f491ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x8722aebff7308379ull, 0xaf63f64c860218baull, 0xf4ec3950faf85ae2ull,
    0x8cc6b005067c0657ull, 0xaf63f04c86020e88ull, 0xfb45d9c0bfb39bc4ull, 0x0000000000000000ull, 0x33ce6168afe5843dull, 0x0000000000000000ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0dacfec9dd4a2f61ull, 0xaf63f64c860218baull, 0x0000000000000000ull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x331a3e08b894d341ull, 0xaf63f64c860218baull,
    0x4e25fa4839de9f7aull, 0xc9dfaf2ad88a43d9ull, 0x6e864ad40a485019ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0x292c79f30e010f3full, 0x6100c39e48e5b4a6ull,
    0x6af613e5b5a7c5c6ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xe119e62523cc0bd5ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0dacfec9dd4a2f61ull, 0xaf63f64c860218baull, 0x53811cfd32fb218full, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull, 0xbfff63417d337bd2ull, 0xaf63f64c860218baull,
    0x0000000000000000ull, 0x4e3fa0b0750fd0b1ull, 0xfb45d9c0bfb39bc4ull, 0x0000000000000000ull, 0x28fc8d926fd650a1ull, 0xaf63f64c860218baull,
    0x8cc6b005067c0657ull, 0xc75ec777d7229644ull, 0x33ce6168afe5843dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xa99887e0ee2d7e2bull, 0xc85a70d7a4830f19ull, 0x0e67414ea4bfc618ull, 0x1605b1d648b14429ull, 0xbbf288165dedfdbcull,
    0xcbe8a8833e1453fdull, 0xb00e0a76301c69cdull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0xc32cc45c35912dd8ull, 0xa99c0fe0ee309b1aull,
    0xc85a70d7a4830f19ull, 0x0e6acb4ea4c2e66dull, 0xd46d3d804f954049ull, 0xaf63f64c860218baull, 0xb70f8f6ee8fad001ull, 0x85b703ccaa230db9ull,
    0x7f2b6c605332dd30ull, 0x30a56db92da4e1e1ull, 0x0000000000000000ull, 0x77e53ec1d1de20ccull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0xf1b6da356e39056bull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0ab2c2ec81fe1adeull, 0xab6e0bdbfd7e7602ull,
    0xaf63f64c860218baull, 0x1605b1d648b14429ull, 0xc4a5f8500e409c8bull, 0xaf63f64c860218baull, 0x5f4ddd808bb805d4ull, 0x2078726e97f0b5a5ull,
    0x383418ce2cbed67cull, 0xaf63f04c86020e88ull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0x9dd48b9888a02542ull, 0xaf63f64c860218baull,
    0x0000000000000000ull, 0x88cf66fd48968e5bull, 0xaf63f64c860218baull, 0x4ef2e93eec883a6dull, 0x85c2101eaf626b7dull, 0x95e6c551ea5b2e7eull,
    0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull, 0xead38cbb493a4312ull, 0x9b881658afed2417ull,
    0x0000000000000000ull, 0xd6e4a5077c4befafull, 0xaf63f64c860218baull, 0xcae0ede623a57106ull, 0x8cc6b005067c0657ull, 0x8c8dca7ced5d4c14ull,
    0x76e4ef22a48d2048ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xf5c008d52db77843ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x563b78aa968260d2ull, 0xaf63f64c860218baull, 0x61294e4f202fc282ull,
    0x5bdc8640ed705e02ull, 0x0000000000000000ull, 0x85c2101eaf626b7dull, 0xee15db4f90b19669ull, 0xaf63f64c860218baull, 0x68a5aeb369711308ull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xa71ce774cf7332ccull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x09f45ab54199f491ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull,
    0x6100c39e48e5b4a6ull, 0xaf63f64c860218baull, 0xf4ec3950faf85ae2ull, 0xf5c008d52db77843ull, 0xaf63f04c86020e88ull, 0x33ce6168afe5843dull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x331a3e08b894d341ull, 0xaf63f64c860218baull, 0x4e25fa4839de9f7aull,
    0xc9dfaf2ad88a43d9ull, 0x6e864ad40a485019ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull,
    0x6100c39e48e5b4a6ull, 0xaf63f64c860218baull, 0x6af613e5b5a7c5c6ull, 0x0e7db9aae2f274fcull, 0xf5c008d52db77843ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xd4f5e3ae4c47a951ull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0x0000000000000000ull,
    0xa07530bae0e0a339ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x3e7290b2acb7fe8dull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull,
    0x292c79f30e010f3full, 0xaf63f64c860218baull, 0x2a43873d1eb64ff6ull, 0x5a1afb57ab660c31ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f64c860218baull,
    0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x0e60c94ea4ba8e82ull, 0xe2bf4970f0cf3fa4ull,
    0xaf63f64c860218baull, 0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0xbb24ae92614bf1f0ull, 0x0000000000000000ull, 0x17f5f76f593cea90ull, 0xaf63f64c860218baull,
    0x8172fa4330bee8e5ull, 0xa184f659abf15f76ull, 0x5b493ebbf6c81e94ull, 0x5718da675f51ff70ull, 0x55fc4a06cd0a8584ull, 0xd5c954d25601c23full,
    0x2df71cac3d18e605ull, 0x0a4152858424d90eull, 0x66c88a519a18f94full, 0x66c88a519a18f94full, 0xbea8fca8d7859d95ull, 0x3b6e32fa0e908e17ull,
    0x0000000000000000ull, 0x327e8eaf96932cc7ull, 0x66c88a519a18f94full, 0x66c88a519a18f94full, 0xbea8fca8d7859d95ull, 0x3ff26621006d9215ull,
    0xda7b6928b297d994ull, 0x150d665ae093c693ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x403ae5813aa662b2ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xff00a07998f2062aull, 0x0fd0b366ddc80db9ull, 0xaf63f64c860218baull,
    0x80fdf78d8d829124ull, 0x721b115c75374c61ull, 0xaf63f64c860218baull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x3c003015760737efull, 0xaf63f64c860218baull, 0xfa69b29e801006e6ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x5bba07dc773337bcull, 0xaf63f64c860218baull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xc32cc45c35912dd8ull,
    0x62ee9fe10678525dull, 0xab3abc36c1348e5full, 0xc4a87c5288debf69ull, 0x0000000000000000ull, 0x8a1fd9b73f7d15e0ull, 0xc85a70d7a4830f19ull,
    0xeeee9524f60f6133ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x2bac2ad2d9b98e22ull, 0x7317bcae1d416c93ull, 0xa59f076f8b9cba93ull,
    0x0000000000000000ull, 0x77ba93545dc9f9c5ull, 0xaf63f64c860218baull, 0xd46d3d804f954049ull, 0xaf63f64c860218baull, 0xb70f8f6ee8fad001ull,
    0x85b703ccaa230db9ull, 0x7f2b6c605332dd30ull, 0x30a56db92da4e1e1ull, 0x0000000000000000ull, 0x77e53ec1d1de20ccull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0xab7183dbfd8177c1ull, 0xaf63f64c860218baull, 0x377343e32644684full, 0xdc590cae501a5a06ull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x544d8552da2abf6dull, 0xaf63f64c860218baull, 0x6b4ac11764036f7aull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x0c9c230b44485c21ull, 0xa4722c19e077145eull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x8ee4035a82765c2dull, 0xaf63f64c860218baull, 0x377343e32644684full, 0xdc590cae501a5a06ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x42dac81c081e39d9ull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full,
    0xaf63f64c860218baull, 0x22a100f9ef96a556ull, 0xaf63f64c860218baull, 0x8d9283c5fc84d8a3ull, 0xaf63f64c860218baull, 0xb33de406da8ff084ull,
    0xaf63f64c860218baull, 0x5773dcbbcf6373fdull, 0x2f481e5fc8fc3394ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xb33de406da8ff084ull,
    0xaf63f64c860218baull, 0x57e530cd0326b365ull, 0x96bece5036bd7d68ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x9ace28cfbfe42aceull, 0x33ce6168afe5843dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x55f62c12aac878daull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull,
    0x6e426465cd82221full, 0xaf63f64c860218baull, 0x4a08f9acf42f0cabull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xc29d06414464f7c0ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full,
    0xaf63f64c860218baull, 0x6100c39e48e5b4a6ull, 0xaf63f64c860218baull, 0xe56f35ae555e9f3bull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x9b6d80d80ebc4b3aull, 0xaf63f64c860218baull,
    0x417c079293d6cb80ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x417c079293d6cb80ull, 0xbf96e423adba8399ull,
    0x292c79f30e010f3full, 0x33ce6168afe5843dull, 0xa99887e0ee2d7e2bull, 0xc85a70d7a4830f19ull, 0x4bf16f1c0d635528ull, 0x0000000000000000ull,
    0x4ef2e93eec883a6dull, 0x85c2101eaf626b7dull, 0x95e6c551ea5b2e7eull, 0x0000000000000000ull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull,
    0x292c79f30e010f3full, 0xaf63f64c860218baull, 0xead38cbb493a4312ull, 0x9b881658afed2417ull, 0x0000000000000000ull, 0x6100c39e48e5b4a6ull,
    0xaf63f64c860218baull, 0xcae0ede623a57106ull, 0x76e4ef22a48d2048ull, 0xf5c008d52db77843ull, 0x6af613e5b5a7c5c6ull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x33ce6168afe5843dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x563b78aa968260d2ull, 0xaf9b35a699787d23ull, 0x0000000000000000ull, 0x85c2101eaf626b7dull, 0x0000000000000000ull,
    0xdc590cae501a5a06ull, 0xc85a70d7a4830f19ull, 0x0e67414ea4bfc618ull, 0x8d9283c5fc84d8a3ull, 0xaf63f64c860218baull, 0xf8692374ade0110bull,
    0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull, 0x5244f810221cbd56ull,
    0xaf63f64c860218baull, 0x5773dcbbcf6373fdull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull,
    0x5244f810221cbd56ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0xca891980b5e3a86dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x8dfa586af8473266ull, 0x0000000000000000ull, 0xd46d3d804f954049ull, 0xaf63f64c860218baull, 0xb70f8f6ee8fad001ull, 0x85b703ccaa230db9ull,
    0x7f2b6c605332dd30ull, 0x30a56db92da4e1e1ull, 0x0000000000000000ull, 0x77e53ec1d1de20ccull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0xf1b6da356e39056bull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0a3788e6424feb26ull, 0xaf63f64c860218baull,
    0x55f62c12aac878daull, 0x8d9283c5fc84d8a3ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full,
    0xaf63f64c860218baull, 0x5244f810221cbd56ull, 0xaf63f64c860218baull, 0x9ce8cf50d5d03d66ull, 0x4a08f9acf42f0cabull, 0x0000000000000000ull,
    0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull, 0x2d21d1660fee9b74ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0xe3175f0baabc34a1ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x9e7c9a1c2a5e05a7ull, 0xaf63f64c860218baull, 0xf279d4b517f84e25ull, 0x093139d8979b8f74ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xa07530bae0e0a339ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xbf96e423adba8399ull,
    0xaf63f64c860218baull, 0x292c79f30e010f3full, 0x33ce6168afe5843dull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x7604c1fc0a712122ull,
    0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x6fb2f5464f9f6f47ull, 0x0000000000000000ull, 0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull,
    0x71c5de23414911caull, 0x49a8477ac28e8336ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x91645746f9c7cefbull,
    0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xd7654af0b8a38348ull, 0xaf63f64c860218baull, 0x55f62c12aac878daull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x4ef2e93eec883a6dull, 0x85c2101eaf626b7dull, 0x0000000000000000ull, 0x0000000000000000ull, 0x95e6c551ea5b2e7eull,
    0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xead38cbb493a4312ull, 0x9b881658afed2417ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull,
    0x5244f810221cbd56ull, 0xaf63f64c860218baull, 0x79ecab7ad0aea85full, 0xaf63f64c860218baull, 0xcae0ede623a57106ull, 0x76e4ef22a48d2048ull,
    0x4a08f9acf42f0cabull, 0x6af613e5b5a7c5c6ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x174413505cbad6c3ull,
    0x8cc6b005067c0657ull, 0x33ce6168afe5843dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull, 0x8cc6b005067c0657ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x4122e2e8bc14edddull, 0x563b78aa968260d2ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0xaf9b35a699787d23ull, 0xaf63f04c86020e88ull,
    0x85c2101eaf626b7dull, 0x0000000000000000ull, 0x0000000000000000ull, 0xc29d06414464f7c0ull, 0xaf63f64c860218baull, 0xa99887e0ee2d7e2bull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x8d9283c5fc84d8a3ull, 0xaf63f64c860218baull,
    0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull, 0x5244f810221cbd56ull, 0xaf63f64c860218baull,
    0x9ce8cf50d5d03d66ull, 0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull, 0x2d21d1660fee9b74ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0xe3175f0baabc34a1ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xf279d4b517f84e25ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xe9fe201f6ff7da40ull,
    0xffc4dd6ef7663ccdull, 0xaf63f64c860218baull, 0x9bc3eb25eb9a179aull, 0x460c83664d38f324ull, 0x0000000000000000ull, 0xde9f9179adbc1909ull,
    0xd32278f37e4f12adull, 0x0000000000000000ull, 0x0000000000000000ull, 0x988a8a7a000f0cb6ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x73646adbf76c3190ull, 0x0fd0b366ddc80db9ull, 0xaf63f64c860218baull,
    0x80fdf78d8d829124ull, 0x10bdea570011ec4aull, 0x0000000000000000ull, 0xb13504ede14611d1ull, 0xaf63f64c860218baull, 0xd1163bd72757c214ull,
    0xaf63f64c860218baull, 0x0c5b360e02754a7bull, 0x3edd57c0325697a7ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x2bd6644a851fb707ull,
    0xaf63f64c860218baull, 0x0c5b360e02754a7bull, 0xc38fb6840d2440dcull, 0x2d12fdc02820ca00ull, 0xaf63f04c86020e88ull, 0x5b4506ccec49d690ull,
    0xaf63f64c860218baull, 0x10bdea570011ec4aull, 0x8109b0b73a38d0abull, 0xaf63f04c86020e88ull, 0xa14931192c05aee4ull, 0xaf63f64c860218baull,
    0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull, 0x61f6e8bcc4703f65ull, 0xaf63f64c860218baull, 0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull,
    0xc85a70d7a4830f19ull, 0x4eee912b74e98a14ull, 0x23460fb4127372daull, 0xaf63f64c860218baull, 0x3efbcfc03270611eull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x47f380c0379adcdcull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x4f0d0b2b750356f1ull,
    0xcc6736d0ce454a99ull, 0xaf63f64c860218baull, 0x47f380c0379adcdcull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x5804bc2b7a2dd2afull,
    0x8377ab9d02438ca7ull, 0xaf63f64c860218baull, 0xad5d686a56c36e13ull, 0x2d1d85c0282a059dull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x3ee1bf2b6c2e8c8aull, 0xf1d0f0c00e90db14ull, 0xaf63f64c860218baull, 0xa3fb4be01087b716ull, 0xaf63f64c860218baull, 0x3efbcfc03270611eull,
    0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xed993e49503803c1ull, 0xaf63f64c860218baull, 0x9e76e810a3e23cb7ull, 0xc85a70d7a4830f19ull,
    0xaf63f04c86020e88ull, 0xc4b4cc11c66e6365ull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0x292c79f30e010f3full,
    0xaf63f64c860218baull, 0x92648b1c3083d821ull, 0xaf63f64c860218baull, 0x814b2db1c03b9d38ull, 0xaf63f64c860218baull, 0x0e7db9aae2f274fcull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x5b957b34b7398a67ull,
    0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x3633aec02d6e4ad2ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x3ed7b92b6c262dd3ull, 0xcc6736d0ce454a99ull, 0xaf63f64c860218baull, 0x3633aec02d6e4ad2ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull,
    0xc85a70d7a4830f19ull, 0x4644ea2b700140a5ull, 0xa42c662c77b6d266ull, 0xaf63f64c860218baull, 0xc32cc45c35912dd8ull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xa07530bae0e0a339ull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x3c003015760737efull, 0xaf63f64c860218baull, 0xad630f5046c52cf0ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0x17508f6e67c4ce77ull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x3534ff8d8d599912ull, 0xaf63f64c860218baull,
    0x7a388588d35436d7ull, 0xaf63f04c86020e88ull, 0xa3d223bb89dca9d2ull, 0x83bc1fc059399fefull, 0xc85a70d7a4830f19ull, 0x9580d92b9d3f005cull,
    0x8f5f98ce9108feaaull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0xc6f0d23920c7e1e1ull, 0xe34484871f44c707ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x8d8f3c7e662df511ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f64c860218baull, 0x5981bf1191fc0081ull,
    0x4f9279f54ac9ecd3ull, 0x0000000000000000ull, 0x85328b6455c90a50ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x64633a7d0b1bf737ull,
    0x0000000000000000ull, 0xdbdbacea1770bfd7ull, 0xc38fb6840d2440dcull, 0x0000000000000000ull, 0x8a1fd9b73f7d15e0ull, 0xbb8734d47ea2034eull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xeeee9524f60f6133ull, 0x07fe93f42f995a55ull, 0xaf63f64c860218baull,
    0xd4f5e3ae4c47a951ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x8a04d9b73f664e28ull, 0xc85a70d7a4830f19ull, 0xeed39524f5f8997bull,
    0xeb1612de686b00fbull, 0xaf63f64c860218baull, 0xf3e5a22e2a7dfee9ull, 0xa62ab5ecfdb2ab70ull, 0xaf63f64c860218baull, 0x2d12fdc02820ca00ull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x8a0159b73f633ed1ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x8a0159b73f633ed1ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xeed01324f5f586beull,
    0x0000000000000000ull, 0xae2ff223442254a5ull, 0x85ab20cc00240621ull, 0x0000000000000000ull, 0x8109b0b73a38d0abull, 0xe5d86a24f0cb1898ull,
    0x0000000000000000ull, 0x27e8cec6106a5902ull, 0xaf63f64c860218baull, 0x8a0159b73f633ed1ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xeb572dab6ac7122bull, 0xaf63f64c860218baull, 0x4f85bcef9e35f700ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0xda2e72c4aaca4cb4ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x54ccb73a98191962ull, 0x6a8eefadcd4447fdull,
    0x0000000000000000ull, 0xe247db11a376c6d5ull, 0xaf63f64c860218baull, 0x289c2f95890fd81bull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f64c860218baull, 0x1e4d6d7b4c6efb0eull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x824e91c94244f0fcull,
    0xaf63f64c860218baull, 0xb8ac9b412b1880ddull, 0x0000000000000000ull, 0x98df0f612ce46a1dull, 0x76f5c7051ef4cc90ull, 0x8a0159b73f633ed1ull,
    0x879289e7e45e91c7ull, 0xb96d553a403f7457ull, 0xc5b2eccd9bbf078dull, 0xaf63f64c860218baull, 0x54ccb73a98191962ull, 0x76f5c7051ef4cc90ull,
    0x8a0159b73f633ed1ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x3dc392e582189467ull, 0x1bed7d6bbef37e09ull,
    0x9fd39fed7f06a199ull, 0xaf63f04c86020e88ull, 0xc46b4f1a2a853aacull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x39c49d1c02d9f13eull, 0x0000000000000000ull, 0x0000000000000000ull, 0xad2d8a503b0915d6ull, 0xaf63f64c860218baull, 0x120a8aa301482303ull,
    0xaf63f64c860218baull, 0x8a04d9b73f664e28ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0xc3949506cb906856ull, 0xaf63f64c860218baull, 0xc1b0cac6698ccd51ull,
    0xaf63f64c860218baull, 0x5773dcbbcf6373fdull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x85f35e54d4ece9c4ull,
    0xdc590cae501a5a06ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x42dac81c081e39d9ull, 0xcfc6cefede18e2acull, 0xaf63f64c860218baull,
    0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0xc3949506cb906856ull, 0xaf63f64c860218baull, 0xc1b0cac6698ccd51ull, 0x8cc6b005067c0657ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xfb251643fdaa0d0aull, 0x0000000000000000ull, 0x120a8aa301482303ull, 0xaf63f64c860218baull,
    0xee855eae5aa2e470ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xc85a70d7a4830f19ull,
    0x53549a1c11360943ull, 0x17f0c39deecfae2aull, 0xaf63f64c860218baull, 0xbf96e423adba8399ull, 0xaf63f64c860218baull, 0xc3949506cb906856ull,
    0xaf63f64c860218baull, 0xc1b0cac6698ccd51ull, 0xaf63f64c860218baull, 0xe3175f0baabc34a1ull, 0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull,
    0x7c13ff0ca82cc93dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x8a0159b73f633ed1ull, 0xc85a70d7a4830f19ull, 0x08d652d7a8c43c08ull, 0x0000000000000000ull, 0xbc6e98220df7183bull, 0xaf63f64c860218baull,
    0xd222b441b46f1c8cull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xa678311eb1ac7a19ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xeae94c8beb2254ddull, 0xaf63f64c860218baull, 0xddc62d9e7c455242ull, 0xa497ca77c4d9ccccull,
    0xaf63f04c86020e88ull, 0x1605b1d648b14429ull, 0xbbf288165dedfdbcull, 0xcbe8a8833e1453fdull, 0x770139ca6b86cbdbull, 0xc46b4f1a2a853aacull,
    0x0000000000000000ull, 0xc85a70d7a4830f19ull, 0xbe0052e09bf9b995ull, 0xcfc6cefede18e2acull, 0x1e4d6d7b4c6efb0eull, 0xc85a70d7a4830f19ull,
    0x08028ec8d2fc3642ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f64c860218baull, 0x0ebd9b383ebb146dull, 0xd32271d551eab26eull,
    0x1e4d6d7b4c6efb0eull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x534512bfe022b7e1ull, 0xfdabbeea4ed9492eull, 0xaf63f64c860218baull,
    0x1605b1d648b14429ull, 0xc4a5f8500e409c8bull, 0xaf63f64c860218baull, 0x5f4ddd808bb805d4ull, 0x2078726e97f0b5a5ull, 0x45dcde1b6e59aeb9ull,
    0xaf63f04c86020e88ull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0x52468b62e5b86ec1ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x88f12ea77addbcaeull, 0xfa69b29e801006e6ull, 0x476fad91530f63c7ull, 0x0000000000000000ull, 0xaf63f64c860218baull, 0x28fd8a83e6434277ull,
    0x0770816bd8eddb50ull, 0x62ee9fe10678525dull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xe95da6adfa13d723ull, 0xdd79c45e91c8d708ull,
    0xaf63f64c860218baull, 0xfa69b29e801006e6ull, 0x5b957b34b7398a67ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x3925a2a1f1cc36d4ull,
    0x7604c1fc0a712122ull, 0xaf63f04c86020e88ull, 0xc9d53c3afee65bc1ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x5d09f202dba5bac6ull,
    0xaf63f64c860218baull, 0x462992e3f2a68bb9ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x16c43cbd18729a24ull, 0xaf63f64c860218baull,
    0x4d2873fad8d94e0eull, 0x0000000000000000ull, 0x0fd0b366ddc80db9ull, 0xaf63f64c860218baull, 0x80fdf78d8d829124ull, 0x5680e042ecbb87e6ull,
    0x523115af05391a0bull, 0x7cf412da2155fb7aull, 0xcdce4cd8893194eeull, 0x8ed8469bde6ce987ull, 0xaf63f64c860218baull, 0x256f8f28ab14bba1ull,
    0x2056ee5ec68bad9full, 0xaf63f64c860218baull, 0x30b29dfc9e927344ull, 0x0000000000000000ull, 0x1d252f46741d2b3bull, 0xc85a70d7a4830f19ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x4baf4ac11039911bull, 0xaf63f64c860218baull, 0x486a84468c9cb177ull,
    0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xaaaf928771018328ull, 0xcc6736d0ce454a99ull, 0x5180ad4691e0f6acull, 0xc85a70d7a4830f19ull,
    0xdd5792878d52e3ffull, 0xa39710ec3ac99af1ull, 0x82346e1b67866d93ull, 0x9e76e810a3e23cb7ull, 0xc85a70d7a4830f19ull, 0xd5f4678789802fe4ull,
    0x3edd57c0325697a7ull, 0xc85a70d7a4830f19ull, 0xcd4b40878498bff5ull, 0xd74969603e0d4a82ull, 0xaf63f64c860218baull, 0x36ad2878cb3b391dull,
    0xaf63f64c860218baull, 0x9e76e810a3e23cb7ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x8fefc1f7058c6cf4ull,
    0xc85a70d7a4830f19ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xf8f643b93c92f417ull, 0xaf63f64c860218baull, 0x45e046fea68209a3ull,
    0xaf63f64c860218baull, 0xd68582a4c7e02b11ull, 0xaf63f64c860218baull, 0x06ba0871ddb02b6dull, 0xaf63f04c86020e88ull, 0xcba36c825881e6d9ull,
    0xbdf2e2690b03cf0full, 0xaf63f64c860218baull, 0x5e5d1e521c5dbd8bull, 0xaf63f64c860218baull, 0xec3e8a4592d1336eull, 0xae4d0982b3883384ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x08c287a53fbdd1fcull, 0xaf63f64c860218baull,
    0x731bb256cfe03fa4ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0xf80c2bae7b049973ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x3e48d8c9ed3bbc0cull, 0xaf63f64c860218baull, 0x75324c1b3d35e612ull, 0xaf63f64c860218baull, 0x980a4c83fce43cfbull,
    0xb4fea5875af6e6cfull, 0xaae9fc2ee10cc95bull, 0x97a68e96715d250dull, 0x3fafdb4687a5f16full, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0x36aab24682701f6dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0x36aab24682701f6dull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xcd3a3e87848a495cull, 0x23abf9706c44abe0ull, 0x36aab24682701f6dull,
    0xc85a70d7a4830f19ull, 0xc43515877f54775aull, 0x3edd57c0325697a7ull, 0xc85a70d7a4830f19ull, 0x4eee912b74e98a14ull, 0xa53a3b9814712b24ull,
    0x0000000000000000ull, 0xcd585de6ec27d6ebull, 0x0000000000000000ull, 0x47f380c0379adcdcull, 0x5804bc2b7a2dd2afull, 0x3aa66840c6898b6eull,
    0xaf63f64c860218baull, 0xb721411015386e82ull, 0x631fd6053bc9dd1cull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xf29efb771e3aeb07ull,
    0x3fc0db4687b464a2ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xf0ed668ab13d4706ull,
    0xf6752968e13ef574ull, 0xd333531a66916ec2ull, 0x40aa04bd3a6bc892ull, 0xaf63f64c860218baull, 0x9bc3eb25eb9a179aull, 0x497da8b4e7bdaa1full,
    0xdbad8faa6fedd55cull, 0xd6c6ea8d187ef876ull, 0xde9f9179adbc1909ull, 0x0000000000000000ull, 0xa458a9aa0d3c4abeull, 0x1270292885b6473cull,
    0x502a4f77ae0764a8ull, 0x2a0f4f53e9a60bb2ull, 0x08d7ae7ad3458cd5ull, 0x0000000000000000ull, 0xc8052c7aa915265dull, 0xaf63f64c860218baull,
    0xf9730cbb9a1e3f59ull, 0xaf63f64c860218baull, 0x8a1f108965d2101dull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xc09de2bdb83c1fb8ull, 0xaf63f64c860218baull,
    0x59025fc3f66d3fa5ull, 0x9e76e810a3e23cb7ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0fd0b366ddc80db9ull, 0xaf63f64c860218baull,
    0x80fdf78d8d829124ull, 0x0000000000000000ull, 0x4a5646275d351ab2ull, 0xd11e67aacb74cd0bull, 0x7f2b6c605332dd30ull, 0x59d9304a55f6018eull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x5aa5f9785704554full, 0xf030756fff53de49ull, 0xaf63f64c860218baull, 0xd68582a4c7e02b11ull,
    0xaf63f64c860218baull, 0xfa1a1c1a857b4186ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xcba36c825881e6d9ull, 0x0a15cf537fb32b2dull,
    0xaf63f04c86020e88ull, 0x0131836bc9be5f37ull, 0xaf63f64c860218baull, 0x196aecf5f3ad3de4ull, 0xaf63f04c86020e88ull, 0x0e791e7a6d55f821ull,
    0x0000000000000000ull, 0xc29d06414464f7c0ull, 0xaf63f64c860218baull, 0x2634e046795c38daull, 0xc8052c7aa915265dull, 0xaf63f64c860218baull,
    0xc15c8f816a4697bfull, 0xaf63f64c860218baull, 0x908cf920c74d68c3ull, 0x1bbd027bd870f309ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xb40f3aa0ad38917full, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x4392be712ed28249ull,
    0x44b9eafd5c8dda4aull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x8e0ccf7851a7e28dull,
    0xaf63f64c860218baull, 0xd68582a4c7e02b11ull, 0xaf63f64c860218baull, 0x4dd394e4b69be718ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0xcba36c825881e6d9ull, 0xbdf2e2690b03cf0full, 0xaf63f64c860218baull, 0x5e5d1e521c5dbd8bull, 0xaf63f64c860218baull, 0x6c7cfa81b82d05b8ull,
    0xda6e029636e42625ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x08c287a53fbdd1fcull,
    0xaf63f64c860218baull, 0x3f4d7ab88f564f93ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0x08af0fe42e11bd22ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x7c37f155a06bfcf7ull,
    0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xb20bc58774ce262dull, 0x0000000000000000ull, 0x0715804ce1f89269ull, 0xaf63f64c860218baull,
    0x051619258f003c19ull, 0xaf63f64c860218baull, 0xd805a1531e950835ull, 0xaf63f64c860218baull, 0x3339924dfb9742ccull, 0xaf63f64c860218baull,
    0xb5b2358b6ab57785ull, 0xdd4c0a9e65a51592ull, 0x9e76e810a3e23cb7ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xec355781170299a2ull, 0xaf63f64c860218baull,
    0x886bd89f77546878ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xb24249dc815adaebull, 0xe7b7f75099fff6c0ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xc8052c7aa915265dull, 0xaf63f64c860218baull,
    0x67d0b09351bbf5c8ull, 0x7a286db33bd24cb3ull, 0xaf63f64c860218baull, 0xa8220471943becdcull, 0x44b9eafd5c8dda4aull, 0x9a8f8e007efb05feull,
    0x9e736010a3df1fc8ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x9e91e810a3f9046full,
    0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x0000000000000000ull,
    0x0715804ce1f89269ull, 0xaf63f64c860218baull, 0xdf80882ad9fdd713ull, 0xdfb80e04076ced7cull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0x9e91e810a3f9046full, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x0000000000000000ull,
    0x80faf18d8d80511bull, 0xcc6736d0ce454a99ull, 0xaf63f64c860218baull, 0x9e6fe010a3dc1071ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x80f76f8d8d7d3e5eull, 0x9e87e810a3f0afeaull, 0xc85a70d7a4830f19ull, 0x810ef98d8d9107bdull, 0xdc8ba31789308b3eull, 0xaf63f64c860218baull,
    0x9e91e810a3f9046full, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x06a8eb659b777ed6ull, 0xe2f36b5194f2c74dull, 0xc85a70d7a4830f19ull,
    0x9e846010a3ed92fbull, 0x4c0b50d7eb16cb6dull, 0x59fe252595f0b449ull, 0xa372da5f4f961ccfull, 0xa8d32a8c6ff92d0full, 0x0711cf14fb956958ull,
    0xc85a70d7a4830f19ull, 0x810bef8d8d8ec0e8ull, 0x0000000000000000ull, 0x46b864df836ad966ull, 0xaf63f64c860218baull, 0x5d0dd28518e3c6deull,
    0xaf63f64c860218baull, 0x0711cf14fb956958ull, 0x4c0b50d7eb16cb6dull, 0xa8d32a8c6ff92d0full, 0xaf63f04c86020e88ull, 0x8f0d9df748daa0ecull,
    0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xd77e97d6bb1d54eaull,
    0xaf63f64c860218baull, 0x8d977c056d590d7dull, 0xaf63f64c860218baull, 0x06148b00a2177fcbull, 0x104b9b207b0bf63eull, 0xbe16ac64dbbeec84ull,
    0x9e91e810a3f9046full, 0xaf63f04c86020e88ull, 0xa5961522fe5ffe83ull, 0xaf63f64c860218baull, 0xc7f40af08d1a6e0aull, 0x9e80e810a3ea913cull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x9e87e810a3f0afeaull, 0xaf63f04c86020e88ull, 0x93129dcba497681cull, 0xab052c8b7e1994adull,
    0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x3339924dfb9742ccull, 0xaf63f64c860218baull, 0x440f74004e0542bbull,
    0x7f398a539919a696ull, 0xc139ef04c7cec5cbull, 0xaf63f64c860218baull, 0x908cf920c74d68c3ull, 0x1bbd027bd870f309ull, 0xe29a1c833c7db3d2ull,
    0x9a8f8e007efb05feull, 0xaf63f04c86020e88ull, 0x9e736010a3df1fc8ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x8108798d8d8bc28full, 0x9e7d6810a3e781e5ull, 0xc85a70d7a4830f19ull, 0x8104f78d8d88afd2ull, 0x62b0f46e7a36c30dull, 0x59fe252595f0b449ull,
    0x09c0e9b4c663bbd5ull, 0x9e76e810a3e23cb7ull, 0xc85a70d7a4830f19ull, 0x0000000000000000ull, 0x8119778d8d9a325cull, 0xcc6736d0ce454a99ull,
    0xaf63f64c860218baull, 0xbabd1fae6e8ff9a7ull, 0x9a8f8e007efb05feull, 0x2631584679591bebull, 0x0000000000000000ull, 0x6c8a0a97044ba7ebull,
    0xaf63f64c860218baull, 0xb11a7665763619a6ull, 0x158c151cdf188e31ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0x4948d4c4503f335bull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xb2083b8774cb05d8ull, 0x1d1b2f467414d6b6ull,
    0xc85a70d7a4830f19ull, 0xaaa5148770f85889ull, 0x1d17b7467411d4f7ull, 0xc85a70d7a4830f19ull, 0xaaa19a8770f55364ull, 0x2fef3c3ab4464d36ull,
    0x140506466ed09181ull, 0x3d7c3fae9897027bull, 0x3fc0db4687b464a2ull, 0x7f2b6c605332dd30ull, 0x3edd57c0325697a7ull, 0xc85a70d7a4830f19ull,
    0xa18ee9876bb40feeull, 0x5180ad4691e0f6acull, 0xc85a70d7a4830f19ull, 0xdd5792878d52e3ffull, 0x120a8aa301482303ull, 0xaf63f64c860218baull,
    0xdb3991de3853443cull, 0x486a84468c9cb177ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xd5f4678789802fe4ull, 0xbf96e423adba8399ull,
    0xaf63f64c860218baull, 0x292c79f30e010f3full, 0xaf63f64c860218baull, 0x4a23cfc250fafe84ull, 0xaf63f64c860218baull, 0xe3175f0baabc34a1ull,
    0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull, 0x2d21d1660fee9b74ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xf3586e8960308cc4ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x487b84468cab24aaull, 0xc85a70d7a4830f19ull,
    0xd6056987898ea67dull, 0x3339924dfb9742ccull, 0xaf63f64c860218baull, 0x8cd0940db883d3ddull, 0x9e76e810a3e23cb7ull, 0xaf63f04c86020e88ull,
    0xc85a70d7a4830f19ull, 0xcd4b40878498bff5ull, 0x6b102046a03c2433ull, 0xc85a70d7a4830f19ull, 0xf89a83879d207c20ull, 0x9e76e810a3e23cb7ull,
    0xc85a70d7a4830f19ull, 0x4eee912b74e98a14ull, 0x2d1d85c0282a059dull, 0xc85a70d7a4830f19ull, 0x3ee1bf2b6c2e8c8aull, 0x63fa9c17c72d08bfull,
    0x0c2ecc0048ba5d5full, 0xe2691fb786c9271dull, 0xc85a70d7a4830f19ull, 0x98a31f3e82a1448aull, 0x287e33ec012c1bf6ull, 0xac2484844b54dcf7ull,
    0x879289e7e45e91c7ull, 0xf81f8ceafb893d16ull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0x1a90dabb91c094adull, 0xaf63f64c860218baull,
    0xe09f352029098d6dull, 0x972615c079a1c1ccull, 0xaf63f04c86020e88ull, 0x7f12dc6e85e7e77bull, 0xaf63f64c860218baull, 0x2098cf36d84f413aull,
    0x9fd39fed7f06a199ull, 0x9e76e810a3e23cb7ull, 0xef204230cbcdac02ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x4d609747757abc1full,
    0x47c3c61072c11243ull, 0x4d5d15477577a962ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f64c860218baull, 0x4c21267b883e58a5ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xffe047973dbb2e98ull, 0xaf63f64c860218baull, 0xdb3991de3853443cull, 0x287e33ec012c1bf6ull,
    0xdbdbacea1770bfd7ull, 0x0000000000000000ull, 0xae0301cfed22e18aull, 0xc85a70d7a4830f19ull, 0x879289e7e45e91c7ull, 0x8fc2ecc075cf1117ull,
    0xc85a70d7a4830f19ull, 0xc46b4f1a2a853aacull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0x839e97caa333cd4aull, 0xaf63f64c860218baull, 0xdb3991de3853443cull, 0x287e33ec012c1bf6ull, 0xdbdbacea1770bfd7ull, 0x0000000000000000ull,
    0xae0301cfed22e18aull, 0x879289e7e45e91c7ull, 0x8fc2ecc075cf1117ull, 0xc46b4f1a2a853aacull, 0x328d90fd52054248ull, 0x2718d391356efbb8ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x444a6c4770367384ull, 0xb0d90f1380cabffbull, 0x0000000000000000ull, 0xe2f36b5194f2c74dull,
    0xaf63f64c860218baull, 0xa9794278238375a2ull, 0xaf63f64c860218baull, 0xab9407b22bd652ffull, 0xf99b45415abee63eull, 0x09f58a31af843b10ull,
    0x8716579baf3dc06bull, 0xaa75ccfe5d9887efull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xb8fbb22a6bc6e3e7ull,
    0xaf63f64c860218baull, 0x0000000000000000ull, 0x9c0b359d18d48ce7ull, 0xaf63f64c860218baull, 0xccf634fc572390a8ull, 0x9fd39fed7f06a199ull,
    0x9e76e810a3e23cb7ull, 0xaf63f04c86020e88ull, 0x8f0d9df748daa0ecull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xdd0ce97e3933aa3bull,
    0x4573863690dd8c18ull, 0x0000000000000000ull, 0xf1af29e2abdcad14ull, 0x9f398e19aac35248ull, 0x7f2b6c605332dd30ull, 0xbd83519b9e5269aaull,
    0x0000000000000000ull, 0xe7a675f47b58f20eull, 0xaf63f64c860218baull, 0xe022f2bb587af170ull, 0xaf63f64c860218baull, 0xe09f352029098d6dull,
    0x432f663faa390938ull, 0x0000000000000000ull, 0x8fc664c075d212d6ull, 0xef204230cbcdac02ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0x4502d31fd0fc3a44ull, 0x972615c079a1c1ccull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0xdbdbacea1770bfd7ull, 0x63fa9c17c72d08bfull, 0xf79303c6f8d8a4c3ull, 0xc85a70d7a4830f19ull, 0x444de647703978a9ull,
    0xcc6736d0ce454a99ull, 0x47c3c61072c11243ull, 0xc85a70d7a4830f19ull, 0x444364477030473eull, 0xe8fee0631a60674aull, 0xfec47a9639e9096eull,
    0x68a46281eaba8c39ull, 0x3d90eebaa3325897ull, 0x7f2b6c605332dd30ull, 0xcc8afe383029e436ull, 0x9e91e810a3f9046full, 0xc85a70d7a4830f19ull,
    0x028a835068b017b0ull, 0x934b45d5cea2c14cull, 0x1a90dabb91c094adull, 0xaf63f64c860218baull, 0x1c045bec5a3b5b95ull, 0x09f58a31af843b10ull,
    0xae0301cfed22e18aull, 0xaf63f04c86020e88ull, 0x7f12dc6e85e7e77bull, 0xaf63f64c860218baull, 0x81721ee60bed0356ull, 0x9fd39fed7f06a199ull,
    0x9e76e810a3e23cb7ull, 0xef204230cbcdac02ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xb74803595b899cddull, 0x0fe37867d9957f03ull,
    0xaf63f64c860218baull, 0x2d2105c0282d14f4ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x3633aec02d6e4ad2ull,
    0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x3ee5412b6c319f47ull, 0x0000000000000000ull, 0x8fe41b5eab90b6d5ull, 0x0000000000000000ull,
    0x306038eb3c889d6aull, 0xaf63f64c860218baull, 0x253c0b10bf42e3f5ull, 0xaf63f64c860218baull, 0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull,
    0x2d21d1660fee9b74ull, 0xf9c598b338f5b33bull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xe3175f0baabc34a1ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xe1cf1c89563263aaull,
    0x264739212d5a9046ull, 0xc85a70d7a4830f19ull, 0xa1fe03aa39e6e599ull, 0x0000000000000000ull, 0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull,
    0x664becae821fa65bull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x3339924dfb9742ccull, 0x1ee410212987df91ull, 0xc85a70d7a4830f19ull,
    0x98e7d8aa34a29cfeull, 0x0000000000000000ull, 0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull, 0x664becae821fa65bull, 0xaf63f04c86020e88ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xaf63f64c860218baull, 0xffe047973dbb2e98ull, 0xaf63f64c860218baull, 0xe1cf1c89563263aaull,
    0x2d23fdc0282f3d33ull, 0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xb9268a1dc33b0548ull,
    0x839e97caa333cd4aull, 0xaf63f64c860218baull, 0xe1cf1c89563263aaull, 0x2d23fdc0282f3d33ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x3ee8b72b6c349da0ull, 0x0000000000000000ull, 0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull, 0x71c5de23414911caull, 0x49a8477ac28e8336ull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x91645746f9c7cefbull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
    0xd549be94ff779560ull, 0xaf63f64c860218baull, 0xfb251643fdaa0d0aull, 0x55f62c12aac878daull, 0x0000000000000000ull, 0x8fe41b5eab90b6d5ull,
    0x0000000000000000ull, 0x4ef2e93eec883a6dull, 0xd0952155b87143afull, 0xaf63f64c860218baull, 0x1796d85667687668ull, 0xaf63f64c860218baull,
    0x2bac2ad2d9b98e22ull, 0xfaf2e46248de01ffull, 0xff36bdd5b3908ba3ull, 0xaf63f64c860218baull, 0x4a08f9acf42f0cabull, 0xcf1fb446ac12e246ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x5d11c1daee3e6ca9ull,
    0x2d2785c028325a22ull, 0x7f2b6c605332dd30ull, 0x3633aec02d6e4ad2ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x3eec412b6c37bdf5ull,
    0xcc6736d0ce454a99ull, 0x2d0ffdc0281e9429ull, 0xc85a70d7a4830f19ull, 0x3ed4372b6c231b16ull, 0x2d12fdc02820ca00ull, 0xc85a70d7a4830f19ull,
    0x3ed7b92b6c262dd3ull, 0x5bdc8640ed705e02ull, 0xfab27a59937675d1ull, 0xcb04ff651f30ca3bull, 0xaf63f64c860218baull, 0x4f8922c182dbbbeeull,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x58ebd819176616f0ull, 0xaf63f04c86020e88ull, 0x5d0587ca972e3fe9ull,
    0x9e76e810a3e23cb7ull, 0xc85a70d7a4830f19ull, 0x4644ea2b700140a5ull, 0xec6db582e07af668ull, 0xc85a70d7a4830f19ull, 0x6e1b13bfef939952ull,
    0x08c287a53fbdd1fcull, 0xaf63f64c860218baull, 0x4557430bcc896843ull, 0x52468b62e5b86ec1ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0x52468b62e5b86ec1ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x88f12ea77addbcaeull, 0x3534ff8d8d599912ull,
    0xaf63f64c860218baull, 0x7a388588d35436d7ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0xcc6736d0ce454a99ull, 0xaf63f64c860218baull,
    0x85328b6455c90a50ull, 0xdb3991de3853443cull, 0x144c2671a7f2683full, 0x7127b225fd442275ull, 0x82346e1b67866d93ull, 0x1c045bec5a3b5b95ull,
    0x09f58a31af843b10ull, 0x08c287a53fbdd1fcull, 0xaf63f64c860218baull, 0xf6f6af6bcfd60be6ull, 0x0000000000000000ull, 0xe1a1913109edf8a0ull,
    0x6c51dd25d28ea308ull, 0x1ce75cfc85a70af6ull, 0x4557430bcc896843ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0x0000000000000000ull, 0x093139d8979b8f74ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xe95da6adfa13d723ull,
    0x0000000000000000ull, 0x8fe41b5eab90b6d5ull, 0x0000000000000000ull, 0x306038eb3c889d6aull, 0xaf63f64c860218baull, 0x253c0b10bf42e3f5ull,
    0xaf63f64c860218baull, 0xe3175f0baabc34a1ull, 0xfb50d827ae3fe9b7ull, 0xaf63f64c860218baull, 0x2d21d1660fee9b74ull, 0xaf63f04c86020e88ull,
    0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0xc1e1ed8100aa80bdull, 0x000d586bd51b2a9bull, 0xc85a70d7a4830f19ull, 0xe0477badf4cf8e88ull, 0x3339924dfb9742ccull, 0xaf63f64c860218baull,
    0x0000000000000000ull, 0x8fe41b5eab90b6d5ull, 0x306038eb3c889d6aull, 0xaf63f64c860218baull, 0x253c0b10bf42e3f5ull, 0xaf63f64c860218baull,
    0x8e8b5b8799181959ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xf6f6af6bcfd60be6ull,
    0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xd73154adef8b4cb9ull, 0xe2bf4970f0cf3fa4ull, 0xaf63f64c860218baull, 0x0000000000000000ull,
    0x093139d8979b8f74ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x6fb2f5464f9f6f47ull, 0xe1224f695eb439c5ull, 0xc85a70d7a4830f19ull,
    0xa0f4b709dae182bbull, 0xcc6736d0ce454a99ull, 0x35fd3f79ec2fedb9ull, 0xc85a70d7a4830f19ull, 0xec373f00e8080b26ull, 0x376ff07b5a6d71e5ull,
    0xc85a70d7a4830f19ull, 0x970e6b39fe91ddcbull, 0xd3ae6296847af449ull, 0xc85a70d7a4830f19ull, 0x370ef3337fb89536ull, 0xcc6736d0ce454a99ull,
    0xddfd24b0c11bd156ull, 0xc85a70d7a4830f19ull, 0xeca0374b485a2029ull, 0x63f19f4609ff46b2ull, 0xaf63f64c860218baull, 0x3d95d6a86a89e29full,
    0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x764d00b7ac1944b3ull, 0xd4e6fbb0bbd78c21ull, 0x6d087965c99b91cfull,
    0xbfd0e1b14bd365eeull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xe38a0c4b4315d78eull, 0x80973e543a9e32c7ull, 0xec70fa95364e1133ull,
    0xaf63f64c860218baull, 0x6d087965c99b91cfull, 0xbfd0e1b14bd365eeull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull,
    0xbfd0e1b14bd365eeull, 0xddfb096e68f8cd60ull, 0xaf63f64c860218baull, 0x9e76e810a3e23cb7ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull,
    0x8c21ace3f7e4bf83ull, 0xaf63f64c860218baull, 0x9e76e810a3e23cb7ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x4c386742f1c7548cull,
    0xf32718c0332b6a2cull, 0x028e3103557bb715ull, 0xc85a70d7a4830f19ull, 0x56872b5d2e6834ffull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0xcc6736d0ce454a99ull,
    0xaf63f64c860218baull, 0x0000000000000000ull, 0x3e69a2b74051f5fdull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xa1ca33543b8f96eaull,
    0xdd79c45e91c8d708ull, 0xd3ae6296847af449ull, 0xc85a70d7a4830f19ull, 0x8b76e704bae77cc8ull, 0x028e3103557bb715ull, 0x1218a64977d24ad7ull,
    0xc85a70d7a4830f19ull, 0x20bbb6e3ff109644ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0x0000000000000000ull, 0x0000000000000000ull, 0xcc6736d0ce454a99ull, 0x7285944ff7092dceull, 0xc85a70d7a4830f19ull, 0xd5e5a6ecf245f8a1ull,
    0xdd79c45e91c8d708ull, 0xd3ae6296847af449ull, 0xc85a70d7a4830f19ull, 0x0000000000000000ull, 0x25e225b767c86e1eull, 0x9be10ce9a1fde855ull,
    0xaf63f64c860218baull, 0x2631584679591bebull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0xed5a0b625c66f4cfull, 0xcc6736d0ce454a99ull, 0xaf63f64c860218baull, 0x7fb897fd8893a84dull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull,
    0x36e99359681be8baull, 0x5bd6024fb6eda0e7ull, 0xaf63f64c860218baull, 0x88cec0fd8dd7ed82ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull,
    0xaf63f64c860218baull, 0xdf69adb6d7d0e3f3ull, 0x3e07f7e0a9d1709dull, 0x7f2b6c605332dd30ull, 0x98db12fd9692118cull, 0xaf63f04c86020e88ull,
    0xc85a70d7a4830f19ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x3fffbe596d603155ull, 0xcc3556fe90771e97ull, 0xaf63f64c860218baull,
    0x9e54e072332ff828ull, 0x9177e9fd92bf60d7ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0xdf69adb6d7d0e3f3ull,
    0x3e07f7e0a9d1709dull, 0x7f2b6c605332dd30ull, 0x98db12fd9692118cull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x48a9655972487ac4ull,
    0x5878883b482d5066ull, 0xaf63f64c860218baull, 0xdf69adb6d7d0e3f3ull, 0x3e07f7e0a9d1709dull, 0x7f2b6c605332dd30ull, 0x98db12fd9692118cull,
    0xaf63f04c86020e88ull, 0x12b62a2727c43af5ull, 0xaf63f64c860218baull, 0x1b0f22c82bc9d830ull, 0x16491fcba6b212eeull, 0xdf80882ad9fdd713ull,
    0x9e76e810a3e23cb7ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x51bf9059778cc35full, 0xea6e5ba972a574f6ull, 0x3dd0fcfaad034729ull,
    0x9e54e072332ff828ull, 0xca8e19f4955d54a6ull, 0xc85a70d7a4830f19ull, 0x81bf975074e671f9ull, 0xa384d876f56b6c20ull, 0xaf63f64c860218baull,
    0xdf80882ad9fdd713ull, 0x826fcf5be6e36a95ull, 0x5d73fdc105cdeaccull, 0x6deab8baeca6da06ull, 0x376ff07b5a6d71e5ull, 0x7f2b6c605332dd30ull,
    0x9e91e810a3f9046full, 0xaf63f04c86020e88ull, 0x2841e5c237690caeull, 0xaf63f64c860218baull, 0xdf80882ad9fdd713ull, 0x2a26cbec3ecb65efull,
    0xaf63f04c86020e88ull, 0x12b62a2727c43af5ull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x0000000000000000ull, 0xb5b2358b6ab57785ull,
    0xf77bf6e54fe20812ull, 0xdf80882ad9fdd713ull, 0x9e76e810a3e23cb7ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xe30b47481fc6145cull,
    0x6301c6a96af7982aull, 0xdf80882ad9fdd713ull, 0xce35dea630380ec1ull, 0x4cb7042be33d19b8ull, 0x7f2b6c605332dd30ull, 0x8a9c216c88b1c45dull,
    0x0390cd772a391b3eull, 0x0000000000000000ull, 0xa62ab5ecfdb2ab70ull, 0xaf63f64c860218baull, 0x9e76e810a3e23cb7ull, 0xaf63f04c86020e88ull,
    0x7f2b6c605332dd30ull, 0xaf63f64c860218baull, 0x333cf4ec440fab24ull, 0xdffc267051d9c8a1ull, 0x826fcf5be6e36a95ull, 0xaf63f04c86020e88ull,
    0xc85a70d7a4830f19ull, 0xea6e72482398c877ull, 0x94c636a6d6c20d5dull, 0xaf63f64c860218baull, 0x0000000000000000ull, 0x9e91e810a3f9046full,
    0x0000000000000000ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
    0xf441f767a5a98e0aull, 0x68b31001475bd7bbull, 0x23d8ebb57bb8d509ull, 0x9e54e072332ff828ull, 0x471e20e0af15b5d2ull, 0xc85a70d7a4830f19ull,
    0xfba52267a97c4225ull, 0x8cf0020acd2acefcull, 0xaf63f64c860218baull, 0x4fc7c9e0b3fe02a7ull, 0xaf63f04c86020e88ull, 0x12b62a2727c43af5ull,
    0xaf63f64c860218baull, 0xb5b2358b6ab57785ull, 0xc22b4db02fab45eeull, 0xdf80882ad9fdd713ull, 0x3e07f7e0a9d1709dull, 0xaf63f04c86020e88ull,
    0xc85a70d7a4830f19ull, 0x044ec967ae648b94ull, 0xbf407564121beef7ull, 0x5cba170f73b1c37dull, 0x9e54e072332ff828ull, 0x58ddf2e0b94247dcull,
    0xc85a70d7a4830f19ull, 0x0d64f467b3a8d42full, 0xb9fc8ceccceff450ull, 0xaf63f64c860218baull, 0x2477f4e09b754e66ull, 0xaf63f04c86020e88ull,
    0x12b62a2727c43af5ull, 0xaf63f64c860218baull, 0xb5b2358b6ab57785ull, 0xe3ab78680f2ee28aull, 0xdf80882ad9fdd713ull, 0x3e07f7e0a9d1709dull,
    0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0xd8ff766795dcb439ull, 0x2a6b488466cbdd74ull, 0x93708189d015ab13ull, 0x9e54e072332ff828ull,
    0xf2c4ede99caa0b4cull, 0xc85a70d7a4830f19ull, 0xa8ff6f709883059full, 0xd372f9b9feaa0218ull, 0xaf63f64c860218baull, 0x5331dbf01be0ee43ull,
    0xaf63f04c86020e88ull, 0xe07cd2ff1ec33310ull, 0xaf63f64c860218baull, 0x785dfc72486e189cull, 0xdf80882ad9fdd713ull, 0x3e07f7e0a9d1709dull,
    0xaf63f04c86020e88ull, 0x12b62a2727c43af5ull, 0xaf63f64c860218baull, 0xb5b2358b6ab57785ull, 0xa228489a68be19feull, 0xdf80882ad9fdd713ull,
    0x3e07f7e0a9d1709dull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x5e295b798bb86130ull, 0x2976e5c0a49c5b9bull, 0x8f4b22e3cadba32cull,
    0x9e54e072332ff828ull, 0x09a1d9f9102b2b0aull, 0xc85a70d7a4830f19ull, 0x12e6db827e91e65dull, 0x79a090c45a368d79ull, 0xaf63f64c860218baull,
    0x98db12fd9692118cull, 0xaf63f04c86020e88ull, 0xc28be4fc4683b477ull, 0xaf63f64c860218baull, 0xdf80882ad9fdd713ull, 0x2477f4e09b754e66ull,
    0xaf63f04c86020e88ull, 0x12b62a2727c43af5ull, 0xaf63f64c860218baull, 0xb5b2358b6ab57785ull, 0x7874efd2723a2f9cull, 0xdf80882ad9fdd713ull,
    0x5331dbf01be0ee43ull, 0xaf63f04c86020e88ull, 0xc85a70d7a4830f19ull, 0x8986b746e9d28c56ull, 0xf2d1fdce71a8c14eull, 0x5331dbf01be0ee43ull,
    0xc85a70d7a4830f19ull, 0xaf63f04c86020e88ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull,
};
static const unsigned long long kMi910[18] = {   // FNV-1a 64 of the normalised code per 910 line (0 = no code)
    0xd8f75ce115e51d17ull, 0xaf63f64c860218baull, 0x9247bb47a2e3bd6dull, 0x0000000000000000ull, 0x9dd48b9888a02542ull, 0x8968967325afa566ull,
    0xaf63f64c860218baull, 0x5e1e754d473d75d3ull, 0x99518be0aec2a3d4ull, 0xaf63f04c86020e88ull, 0x7f2b6c605332dd30ull, 0xc46b4f1a2a853aacull,
    0xaf63f64c860218baull, 0xde2faa262ce1fd70ull, 0xaf63f04c86020e88ull, 0x0000000000000000ull, 0x581bfd9e0bb66d55ull, 0xaf63f04c86020e88ull,
};
// @@TABLE-END

#ifdef FP9050_PURE   // the source helpers are used by the PURE ctest only
// ---- source helpers (the E-042 [B9] census: tests/test_indexz_autoheight_1203.cpp) ------------------------------------
static bool ReadLines(const std::string& path, std::vector<std::string>& out)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    std::string l;
    while (std::getline(f, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); out.push_back(l); }
    return true;
}
//  Code of one line: same-line /* */ and the trailing // removed (string literals KEPT -- same as gen_c1.py's code_of).
static std::string CodeOf(const std::string& l)
{
    std::string s = l;
    for (;;) {
        const std::size_t a = s.find("/*");
        if (a == std::string::npos) break;
        const std::size_t b = s.find("*/", a + 2);
        s.erase(a, b == std::string::npos ? std::string::npos : b + 2 - a);
    }
    const std::size_t c = s.find("//");
    return c == std::string::npos ? s : s.substr(0, c);
}
static std::string Norm(const std::string& l)
{
    const std::string c = CodeOf(l);
    std::string o;
    bool sp = false;
    for (std::size_t i = 0; i < c.size(); ++i) {
        const unsigned char ch = (unsigned char)c[i];
        const bool w = ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\f';
        if (w) { sp = !o.empty(); continue; }
        if (sp) { o += ' '; sp = false; }
        o += c[i];
    }
    return o;
}
static unsigned long long Fnv(const std::string& s)
{
    unsigned long long h = 0xcbf29ce484222325ull;
    for (std::size_t i = 0; i < s.size(); ++i) { h ^= (unsigned char)s[i]; h *= 0x100000001b3ull; }
    return h;
}
static bool IdentChar(char c) { return std::isalnum((unsigned char)c) != 0 || c == '_'; }
//  Live code per line for the CENSUS (as E-042 [B9]): comments, literal CONTENTS, raw strings and `#if 0` blocks blanked.
static std::vector<std::string> LiveCode(const std::vector<std::string>& v)
{
    std::vector<std::string> out(v.size());
    std::vector<int> st;
    enum { kCode, kBlock, kRaw } mode = kCode;
    std::string rawEnd;
    for (std::size_t i = 0; i < v.size(); ++i) {
        const std::string& l = v[i];
        const bool startsInCode = (mode == kCode);
        std::string code;
        std::size_t k = 0;
        while (k < l.size()) {
            if (mode == kBlock) {
                const std::size_t e = l.find("*/", k);
                if (e == std::string::npos) { k = l.size(); break; }
                k = e + 2; mode = kCode; code += ' '; continue;
            }
            if (mode == kRaw) {
                const std::size_t e = l.find(rawEnd, k);
                if (e == std::string::npos) { k = l.size(); break; }
                k = e + rawEnd.size(); mode = kCode; code += '"'; continue;
            }
            const char c = l[k];
            const char n = (k + 1 < l.size()) ? l[k + 1] : '\0';
            if (c == '/' && n == '/') break;
            if (c == '/' && n == '*') { mode = kBlock; k += 2; continue; }
            if (c == 'R' && n == '"') {
                std::size_t s = k;
                while (s > 0 && IdentChar(l[s - 1])) --s;
                const std::string pre = l.substr(s, k - s);
                const std::size_t p = l.find('(', k + 2);
                if ((pre.empty() || pre == "u8" || pre == "u" || pre == "U" || pre == "L") && p != std::string::npos) {
                    rawEnd = ")" + l.substr(k + 2, p - (k + 2)) + "\"";
                    code += "R\""; mode = kRaw; k = p + 1; continue;
                }
            }
            if (c == '\'' && k > 0 && IdentChar(l[k - 1])) {
                std::size_t s = k;
                while (s > 0 && IdentChar(l[s - 1])) --s;
                if (std::isdigit((unsigned char)l[s])) { ++k; continue; }
            }
            if (c == '"' || c == '\'') {
                code += c; ++k;
                while (k < l.size() && l[k] != c) k += (l[k] == '\\') ? 2 : 1;
                if (k < l.size()) { code += c; ++k; }
                continue;
            }
            code += c; ++k;
        }
        bool dead = false; for (std::size_t q = 0; q < st.size(); ++q) if (st[q]) dead = true;
        std::string t = code; t.erase(0, t.find_first_not_of(" \t"));
        if (startsInCode && !t.empty() && t[0] == '#') {
            std::string d = t.substr(1); d.erase(0, d.find_first_not_of(" \t"));
            while (!d.empty() && (d[d.size() - 1] == ' ' || d[d.size() - 1] == '\t')) d.erase(d.size() - 1);
            if (d.compare(0, 2, "if") == 0) st.push_back(d.compare(0, 4, "if 0") == 0 && (d.size() == 4 || d[4] == ' ' || d[4] == '\t') ? 1 : 0);
            else if (d.compare(0, 4, "else") == 0 || d.compare(0, 4, "elif") == 0) { if (!st.empty() && st.back() == 1) st.back() = 0; }
            else if (d.compare(0, 5, "endif") == 0) { if (!st.empty()) st.pop_back(); }
            if (!dead) out[i] = code;                         // a live #include / #define line stays visible to the census
            continue;
        }
        if (!dead) out[i] = code;
    }
    return out;
}
static void Walk(const std::string& dir, std::vector<std::string>& files)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string n = fd.cFileName;
        if (n == "." || n == "..") continue;
        const std::string p = dir + "\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (n == "tests" || n == "docs" || n == "build" || n.compare(0, 6, "build_") == 0 || n == "third_party" || n == ".git") continue;
            Walk(p, files);
        } else {
            const std::size_t dot = n.rfind('.');
            const std::string ext = dot == std::string::npos ? "" : n.substr(dot);
            if (ext == ".cpp" || ext == ".h" || ext == ".inc") files.push_back(p);
        }
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}
static bool HasWord(const std::string& code, const std::string& w, bool call)
{
    std::size_t p = 0;
    while ((p = code.find(w, p)) != std::string::npos) {
        const bool lb = p == 0 || !IdentChar(code[p - 1]);
        const std::size_t e = p + w.size();
        const bool rb = e >= code.size() || !IdentChar(code[e]);
        if (lb && rb) {
            if (!call) return true;
            std::size_t q = e;
            while (q < code.size() && (code[q] == ' ' || code[q] == '\t')) ++q;
            if (q < code.size() && code[q] == '(') return true;
        }
        p = e;
    }
    return false;
}
static std::string BaseName(const std::string& p)
{
    const std::size_t s = p.find_last_of("\\/");
    std::string b = s == std::string::npos ? p : p.substr(s + 1);
    for (std::size_t i = 0; i < b.size(); ++i) b[i] = (char)std::tolower((unsigned char)b[i]);
    return b;
}
static bool Contains(const std::string& s, const char* sub) { return s.find(sub) != std::string::npos; }
static bool Contains(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }
static std::string SrcRoot()
{
    const char* c = std::getenv("W906_ST01C_CENSUS_ROOT");
    if (c && *c) return c;
#ifdef W906_SRC_ROOT
    return W906_SRC_ROOT;
#else
    return ".";
#endif
}
//  The case / Task= / return tokens of a code line (for the edit lines of [F1]).
static std::vector<std::string> FlowTokens(const std::string& code)
{
    std::vector<std::string> t;
    for (std::size_t i = 0; i < code.size(); ++i) {
        if (code.compare(i, 5, "case ") == 0 && (i == 0 || !IdentChar(code[i - 1]))) {
            std::size_t e = code.find(':', i);
            if (e != std::string::npos) t.push_back(code.substr(i, e + 1 - i));
        }
        if (code.compare(i, 5, "Task=") == 0 && (i == 0 || !IdentChar(code[i - 1]))) {
            std::size_t e = i + 5;
            while (e < code.size() && (std::isdigit((unsigned char)code[e]) || code[e] == '+' )) ++e;
            t.push_back(code.substr(i, e - i));
        }
        if (code.compare(i, 6, "return") == 0 && (i == 0 || !IdentChar(code[i - 1])) && (i + 6 >= code.size() || !IdentChar(code[i + 6])))
            t.push_back("return");
    }
    return t;
}

#endif  // FP9050_PURE (source helpers)

// =========================================================================================================================
#ifndef FP9050_PURE
//  ---- the runtime fixture ----------------------------------------------------------------------------------------------
static bool DoorClosed() { return false; }
//  A sim motor whose in-position LED the test sets (golden reads Led[iInposLed]==true as "still moving"), as
//  tests/test_flow9050_shuttle.cpp's TFlowSimMotor.
class TFpSimMotor : public TMySimMotor
{
public:
    bool bBusy;
    TFpSimMotor() : bBusy(false) {}
    virtual void ScanMotorStatus(bool* Led) { TMySimMotor::ScanMotorStatus(Led); if (Led != NULL) Led[iInposLed] = bBusy; }
};
//  The Index Z1 TMyGALILMotor, counting how often golden's no-card MoveToPos is reached ([F4]).
static int g_moveToPos = 0;
class TCountingGalil : public TMyGALILMotor
{
public:
    explicit TCountingGalil(int port) : TMyGALILMotor(port) {}
    virtual bool MoveToPos(int Tar) { ++g_moveToPos; return TMyGALILMotor::MoveToPos(Tar); }
};
static void SetMotPos(int m, int p)
{
    MOT[m].Position = p;
    if (MOT[m].Motor != NULL) MOT[m].Motor->SetPosition(p);
    MOT[m].fCMD = false;
}
//  Index Z1 in FLOW coordinates through its class API (W906_FP9050Z1Pos: Gali_ReadPos for the Galil-API Index axes, whose
//  no-card / disabled branch stores Position = -flow -- golden's Galil sign convention, myGALILmotor.cpp Gali_MotMove / Gali_ReadPos).
static void SetZ1(int flow) { MOT[MTestZ1].Position = (INDEX_MOTION_CARD == 0) ? -flow : flow; if (MOT[MTestZ1].Motor) MOT[MTestZ1].Motor->SetPosition(flow); MOT[MTestZ1].fCMD = false; MOT[MTestZ1].MovFlag = false; }
static int  Z1() { return W906_FP9050Z1Pos(); }
static void FreeMot(int m) { MOT[m].fCanMove = true; MOT[m].fCanMoveR = true; MOT[m].fCanMoveM = true; MOT[m].fCanMoveL = true; }

//  test geometry (counts; Z down = more negative, as the port's Index Z)
static const int Z_SAFE = -1000, Z_TEST = -12000, Z_PICK = -4000, Z_PLACE = -4500, Z_CCDSAFE = -3000, SOCK_OFS = 500;
static const int IN_L = 10000, IN_R = 60000, OUT_L = 12000, OUT_R = 62000;

//  W-44 seam (E-042's): the In / Out shuttle 1 positions the decision reads; default = home (InSHT[0].iLeft / OutSHT[0].iRight).
//  W-44 seam (Steven 1005 23:1x: shuttle X outside the index safe zone): stands in for Frank's guard (FR-NB2 (2), not in the tree).
static bool g_clear = true;
static int  g_clearCalls = 0;
static bool FakeClear(AnsiString* why) { ++g_clearCalls; if (!g_clear && why) *why = "W-44 test: Out shuttle X inside the index safe zone"; return g_clear; }
static unsigned long g_now = 1000000ul;
static unsigned long FakeNow() { return g_now; }
static unsigned long g_passNo = 100ul;
static unsigned long FakePass() { return g_passNo; }

#ifndef SOFT_SIMULTE
using namespace ht9045;   // IGaliRouteIo / TGaliRouteCore / Pci1203Cmd (EtherCAT/Pci1203GaliRouteCore.h)
//  ---- the fake 1203 behind the REAL TGaliRouteCore (tests/test_indexz_autoheight_1203.cpp) ----
static long          g_target   = 0;
static long          g_surface  = -12000;   // a move below it stalls there (the socket); 6077h reads pressing below it
static unsigned      g_state    = 1;
static unsigned long g_bits     = 0x00004000ul;   // SVON
static long          g_rawPress = -150;     // 15.0 % while pressing
static long          g_rawAir   = 80;
static bool          g_torqueOk = true;
static bool          g_healthMonitor = true, g_sampleValid = true, g_routeFault = false;
static int           g_stops = 0;
static std::vector<long> g_moves;
static std::vector<int>  g_limits;
struct FakeIo : IGaliRouteIo {
    unsigned long   poll;
    GaliRouteSample s;
    unsigned long   now;
    std::map<int, unsigned long> ledger;
    FakeIo() : poll(10), now(1000) { s.state = 1; s.motionIO = 0x00004000ul; }
    bool CardOpen() override { return true; }
    unsigned long PollCount() override { return poll; }
    int Slot(std::string&) override { return 7; }
    bool Sample(int, GaliRouteSample& o) override { o = s; return true; }
    int DriveKind(int) override { return 1; }
    Pci1203CmdResult Exec(const Pci1203Cmd& c) override
    {
        if (c.kind == kCmdAxMoveAbs) { g_target = (long)std::lround(c.value); g_moves.push_back(g_target); }
        if (c.kind == kCmdAxStop || c.kind == kCmdAxEmgStop) ++g_stops;
        Pci1203CmdResult r;
        r.accepted = true; r.issued = true; r.ret = 0;
        return r;
    }
    void Caps(GaliRouteCaps& c) override
    {
        HTMotor* M = MOT[MTestZ1].Motor;
        c.jogHigh = M->PJogHighSpeed; c.initSpeed = M->InitSpeed; c.accDb = M->GetAccDataBase(); c.decDb = M->GetDecDataBase();
    }
    unsigned long NowMs() override { return now; }
    void NoteIssued(int sl, unsigned long p) override { ledger[sl] = p; }
    bool LastIssued(int sl, unsigned long& p) override
    {
        std::map<int, unsigned long>::const_iterator it = ledger.find(sl);
        if (it == ledger.end()) return false;
        p = it->second;
        return true;
    }
    bool OnOwnerThread() override { return true; }
    void SleepMs(int) override {}
    int OrgHome(int, unsigned long) override { return -2; }
    void Log(const std::string&) override {}
    void LogForce(const std::string& l) override { std::printf("    [route!] %s\n", l.c_str()); }
};
static FakeIo         g_io;
static TGaliRouteCore g_core;
static bool T_Command(int, const char* d, long* r) { return g_core.Command(d, r); }
static int  T_HomeStart(int, bool dir, unsigned hi, unsigned lo, double a, double d) { return g_core.HomeStart(dir, hi, lo, a, d); }
static int  T_HomePoll(int) { return g_core.HomePoll(); }
static TGaliRoute g_route = { -1, T_Command, T_HomeStart, T_HomePoll };
static void Fresh()
{
    ++g_io.poll;
    g_io.now += 100;
    g_io.s.state    = g_state;
    g_io.s.motionIO = g_bits;
    g_io.s.cmdPos   = (double)g_target;
    g_io.s.actPos   = (double)(g_target < g_surface ? g_surface : g_target);
}
static unsigned long Poll() { return g_io.poll; }
static void FakeRead(int, ht9045::idxz::Sample* out)
{
    ht9045::idxz::Sample s;
    s.haveMonitor = true; s.poll = Poll(); s.torqueValid = g_torqueOk; s.src = 2;
    s.raw = (g_target <= g_surface) ? g_rawPress : g_rawAir;
    s.pctValid = true; s.num = 1; s.den = 10;
    s.servoOn = (g_bits & 0x4000ul) != 0; s.alarm = (g_bits & 0x2ul) != 0; s.focusOnAxis = g_torqueOk; s.slot = 7;
    if (!g_torqueOk) s.why = "test: focus on another axis";
    if (out) *out = s;
}
static void FakeHealth(int, ht9045::idxz::Health* out)
{
    ht9045::idxz::Health h;
    h.haveMonitor = g_healthMonitor; h.poll = Poll(); h.sampleValid = g_sampleValid;
    h.alarm = (g_bits & 0x2ul) != 0; h.errorStop = (g_state & 0xFFu) == 3u; h.servoOn = (g_bits & 0x4000ul) != 0;
    h.routeFault = g_routeFault; h.routeWhy = g_routeFault ? "test: move refused (latched)" : "";
    if (!g_healthMonitor) h.why = "test: monitor gone";
    if (out) *out = h;
}
static int FakeLimit(int, int v, AnsiString*) { g_limits.push_back(v); return 1; }
static int Moves() { return (int)g_moves.size(); }
static bool HasMove(long t) { for (std::size_t i = 0; i < g_moves.size(); ++i) if (g_moves[i] == t) return true; return false; }
#else
static void Fresh() {}
#endif

//  Gerneral.ini in %TEMP% only (E-038's CONFIRMED switch is read from INIFileGeneral, read only)
static std::string g_dir, g_ini;
static TIniFile* g_oldIni = 0;
static TIniFile* g_tmpIni = 0;
static void UseIni(const char* extra1)
{
    std::string s = "[Version]\r\nModel=9050GPIB\r\n[IndexDriver]\r\nINDEX_DRIVER_TYPE=0\r\n";
    if (extra1) s += std::string(extra1) + "\r\n";
    s += "[System]\r\nX=1\r\n";
    std::ofstream f(g_ini.c_str(), std::ios::binary | std::ios::trunc);
    f.write(s.data(), (std::streamsize)s.size());
    f.close();
    INIFileGeneral = g_oldIni;
    delete g_tmpIni;
    g_tmpIni = new TIniFile(AnsiString(g_ini.c_str()));
    INIFileGeneral = g_tmpIni;
    W906_IndexZTorqueFlagsReset();
}
static bool Has(const AnsiString& s, const char* sub) { return std::string(s.c_str()).find(sub) != std::string::npos; }

//  one MainProc-shaped tick of the FP Index machine: (the monitor polls) -> COM2->ReadTorque (csystem.cpp MainProc) -> FP
static void Tick(bool readTorque = false, unsigned long dtMs = 500)
{
    g_now += dtMs;
    ++g_passNo;
    Fresh();
    if (readTorque) COM2->ReadTorque();
    DoTestHeadMotorFP();
}
static void ResetFlow()
{
    W906_FP9050Reset();
    W906_IndexZHealthReset();
    W906_IndexZTorqueBridgeReset();
    W906_Ht9050TorqueWaitReset();
    W906_ShowMyMessage_Reset();
    W906_ShowErrorMessage_Reset();
    fMain->chkReadTorque1->Checked = false; fMain->chkReadTorque2->Checked = false;
    fMain->edTorue0->Text = "";
    fAllMotorHome = true; SystemStart = true; SoftStop = false; iHome = 0;
    iTestHeadMotorTask = 1; iTestYFinePitchTask = 1; iTestYFrontTask = 1;
    iFrontTestSuckICTask = 1; iFrontTestDestroyICTask = 1; iInitContactModeStartTask = 1; iInitContactModeEndTask = 1;
    for (int i = 0; i < MAX_Index_Row; ++i)
        for (int j = 0; j < 8; ++j) { FTestSuck.SetItemData(i, j, NULL_IC); FLCarryKit.SetItemData(i, j, NULL_IC); FRCarryKit.SetItemData(i, j, NULL_IC); }
#ifndef SOFT_SIMULTE
    g_state = 1; g_bits = 0x00004000ul; g_torqueOk = true; g_healthMonitor = true; g_sampleValid = true; g_routeFault = false;
    g_rawPress = -150;
    g_moves.clear(); g_limits.clear(); g_stops = 0;
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].bScanFlag = false; MOT[MTestZ1].GaliSofDelayCount = 0; MOT[MTestZ1].iCheckStatusCT = 0;
    W906_Pci1203IndexZHealthHook = &FakeHealth;
#endif
    g_clear = true; g_clearCalls = 0; W906_Ht9050ShuttlesClearOfIndexHook = &FakeClear;
}
#endif  // !FP9050_PURE

int main()
{
#ifdef FP9050_PURE
    std::printf("[fp9050_index] PURE part (no library) -- %s configuration\n",
#else
    if (!W906TestRequireCtestRedirects("FP9050_Index")) return 2;
    std::printf("[fp9050_index] RUNTIME part (god stack) -- %s configuration\n",
#endif
#if defined(FP9050_PURE) && !defined(W906_NO_SOFT_SIMULTE)
        "SIM (SOFT_SIMULTE)"
#elif defined(FP9050_PURE)
        "SHIP (no SOFT_SIMULTE)"
#elif defined(SOFT_SIMULTE)
        "SIM (SOFT_SIMULTE)"
#else
        "SHIP (no SOFT_SIMULTE)"
#endif
    );
#ifdef FP9050_PURE
    using namespace ht9045::fp9050;
    // ================= [P] the pure rules =================
    std::printf("-- [P] IndexZFinePitchCore.h\n");
    {
        const int hmPress[] = { 40, 50, 55, 12100, 12101, 12110, 12200, 12300, 121, 122100, 122110, 122, 130 };
        const int hmFree[]  = { 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 20, 21, 30, 60, 90, 100, 120, 12000, 12111, 12112, 123, 124, 125,
                                1550, 1600, 1700, 1710, 1720, 600, 15000, 15100, 20000, 20100, 20200, 20210, 20300, 20400, 20500,
                                21000, 21400, 21500, 30000, 40200, 40300, 40500, 40510, 41000, 41400 };
        const int yfPress[] = { 180, 200, 208, 209, 210, 300, 310, 330 };
        const int yfFree[]  = { 1, 100, 109, 110, 120, 125, 130, 1000, 1100, 1200, 1500, 1600, 1700, 5000 };
        bool ok = true;
        for (std::size_t i = 0; i < sizeof(hmPress) / sizeof(hmPress[0]); ++i) if (!W44PressTaskFP(kSmHeadMotor, hmPress[i])) ok = false;
        for (std::size_t i = 0; i < sizeof(hmFree) / sizeof(hmFree[0]); ++i)   if (W44PressTaskFP(kSmHeadMotor, hmFree[i])) ok = false;
        CHECK(ok, "[P] W-44 DoTestHeadMotorFP set: 40 / 50 / 55 / 12100-12300 / 121 / 122100 / 122110 / 122 / 130 checked; every other 910 case (lifts, safe height, CCD, RTC, place-first) not");
        ok = true;
        for (std::size_t i = 0; i < sizeof(yfPress) / sizeof(yfPress[0]); ++i) if (!W44PressTaskFP(kSmTestYFront, yfPress[i])) ok = false;
        for (std::size_t i = 0; i < sizeof(yfFree) / sizeof(yfFree[0]); ++i)   if (W44PressTaskFP(kSmTestYFront, yfFree[i])) ok = false;
        CHECK(ok, "[P] W-44 DoTestYFrontFP set: 180 / 200 / 208 / 209 / 210 / 300 / 310 / 330 checked; 1000 / 1100 / 1500 (lifts), pick 110 / 120, place 1600 / 1700, 130 (entry) not");
        CHECK(!W44PressTaskFP(2, 180) && !W44PressTaskFP(-1, 40), "[P] an unknown state machine id is never checked (no accidental stop)");
        CcdYHold h;
        bool f1 = false, f2 = false, f3 = false;
        unsigned long t = 1000;
        for (int k = 0; k <= 10; ++k) { const bool r = CcdYHoldStep(h, t + 500ul * (unsigned long)k); if (k < 10 && r) f1 = true; if (k == 10) f2 = r; }
        for (int k = 11; k <= 30; ++k) if (CcdYHoldStep(h, t + 500ul * (unsigned long)k)) f3 = true;
        CHECK(!f1 && f2 && !f3, "[P] Q4b: refusals every 500 ms -> exactly one message at 5.0 s, none before (4.5 s), none after");
        CcdYHoldReset(h);
        bool g1 = false;
        for (int k = 0; k < 9; ++k) if (CcdYHoldStep(h, 50000ul + 500ul * (unsigned long)k)) g1 = true;     // 4.0 s
        const bool afterGap = CcdYHoldStep(h, 50000ul + 4000ul + 2500ul);                                    // a 2.5 s gap: new window
        bool g2 = false;
        for (int k = 1; k < 10; ++k) if (CcdYHoldStep(h, 56500ul + 500ul * (unsigned long)k)) g2 = true;    // 4.5 s into the new one
        const bool at5 = CcdYHoldStep(h, 56500ul + 5000ul);
        CHECK(!g1 && !afterGap && !g2 && at5, "[P] Q4b: a gap > 2 s between refusals starts a new 5 s window (no message carried over)");
        CcdYHold w; w.active = true; w.startMs = 0xFFFFF000ul; w.lastMs = 0xFFFFFE00ul;
        CHECK(!CcdYHoldStep(w, 0xFFFFFF00ul) && !CcdYHoldStep(w, 0x00000380ul) && CcdYHoldStep(w, 0x00000390ul),   // 0xFFFFF000 + 5000 = 0x388 after the wrap
              "[P] Q4b: wrap-safe on the 32-bit tick (0xFFFFF000 + 5 s fires after the wrap, not before)");
    }

    // ================= source pins =================
    const std::string root = SrcRoot();
    std::printf("-- source pins, root %s\n", root.c_str());
    std::vector<std::string> fpLines;
    const bool haveFp = ReadLines(root + "\\atester_FinePitch.cpp", fpLines);
    CHECK(haveFp && !fpLines.empty(), "[F1] atester_FinePitch.cpp readable");
    // ---- [F1] fidelity ----
    {
        int bad = 0, missingTag = 0, codeLines = 0, reverted = 0;
        std::string firstBad, revWhere;
        const int nfp = (int)(sizeof(kFp910) / sizeof(kFp910[0]));
        CHECK(nfp == 3040, "[F1] the table covers 910 atester_FinePitch.cpp:1-3040");
        auto isEdit = [](const int* v, int n, int x) { for (int i = 0; i < n; ++i) if (v[i] == x) return true; return false; };
        const int nEf = (int)(sizeof(kEditsFp) / sizeof(kEditsFp[0])), nEm = (int)(sizeof(kEditsMi) / sizeof(kEditsMi[0]));
        for (int n = 1; n <= nfp; ++n) {
            const int p = n + kK;                              // 1-based port line
            if (p - 1 >= (int)fpLines.size()) { ++bad; if (firstBad.empty()) firstBad = "910 :" + std::to_string(n) + " beyond EOF"; continue; }
            const std::string& pl = fpLines[p - 1];
            if (isEdit(kEditsFp, nEf, n)) {
                if (!Contains(pl, "AI(W906-ST01C)")) ++missingTag;
                const std::string ce = Norm(pl);
                if ((ce.empty() ? 0ull : Fnv(ce)) == kFp910[n - 1]) { ++reverted; revWhere += " :" + std::to_string(n); }   // the edit is gone
                continue;
            }
            const std::string c = Norm(pl);
            const unsigned long long h = c.empty() ? 0ull : Fnv(c);
            if (kFp910[n - 1]) ++codeLines;
            if (h != kFp910[n - 1]) { ++bad; if (firstBad.empty()) firstBad = "910 :" + std::to_string(n) + " / port :" + std::to_string(p) + " `" + pl.substr(0, 80) + "`"; }
        }
        for (int n = 6626; n <= 6643; ++n) {
            const int p = n + kK2;
            if (p - 1 >= (int)fpLines.size() || p < 1) { ++bad; continue; }
            const std::string& pl = fpLines[p - 1];
            if (isEdit(kEditsMi, nEm, n)) {
                if (!Contains(pl, "AI(W906-ST01C)")) ++missingTag;
                const std::string ce = Norm(pl);
                if ((ce.empty() ? 0ull : Fnv(ce)) == kMi910[n - 6626]) { ++reverted; revWhere += " mymotor:" + std::to_string(n); }
                continue;
            }
            const std::string c = Norm(pl);
            const unsigned long long h = c.empty() ? 0ull : Fnv(c);
            if (h != kMi910[n - 6626]) { ++bad; if (firstBad.empty()) firstBad = "mymotor :" + std::to_string(n) + " / port :" + std::to_string(p); }
        }
        char m[400];
        std::snprintf(m, sizeof(m), "[F1] every 910 line not in the edit list equals its port line after normalisation (%d code lines of :1-3040 + MoveIndexZ :6626-6643) %s", codeLines, firstBad.c_str());
        CHECK(bad == 0, m);
        std::snprintf(m, sizeof(m), "[F1] the edit list is pinned: %d lines of atester_FinePitch.cpp + %d of mymotor.cpp (MoveIndexZ)", nEf, nEm);
        CHECK(nEf == kNEditsFp && nEm == kNEditsMi, m);
        CHECK(missingTag == 0, "[F1] every edit line carries AI(W906-ST01C) (the 910 text / reason on the same line)");
        std::snprintf(m, sizeof(m), "[F1] no edit line has silently gone back to 910's code (each listed edit still changes its line)%s", revWhere.c_str());
        CHECK(reverted == 0, m);
        bool tokensKept = true;
        std::string lost;
        for (std::size_t e = 0; e < sizeof(kEditCode) / sizeof(kEditCode[0]); ++e) {
            const int n = kEditCode[e].line;
            const int p = (n >= 6626) ? n + kK2 : n + kK;
            if (p < 1 || p - 1 >= (int)fpLines.size()) { tokensKept = false; continue; }
            const std::string pc = CodeOf(fpLines[p - 1]);
            const std::vector<std::string> tk = FlowTokens(kEditCode[e].code);
            for (std::size_t q = 0; q < tk.size(); ++q) {
                if (n == 2137 && tk[q] == "Task=2") continue;   // the documented leaf stop (CCD branch entry); Task stays 11
                if (pc.find(tk[q]) == std::string::npos) { tokensKept = false; lost += " :" + std::to_string(n) + " " + tk[q]; }
            }
        }
        std::snprintf(m, sizeof(m), "[F1] every edit line keeps the 910 line's case / Task= / return tokens (only :2137 Task=2 -> leaf stop)%s", lost.c_str());
        CHECK(tokensKept, m);
        const std::string mk1 = "port line = 910 line + (" + std::to_string(kK) + ")";
        const std::string mk2 = "port line = 910 line + (" + std::to_string(kK2) + ")";
        bool m1 = false, m2 = false;
        if (kK >= 1 && kK - 1 < (int)fpLines.size()) m1 = Contains(fpLines[kK - 1], mk1.c_str()) && Contains(fpLines[kK - 1], "910 atester_FinePitch.cpp:1-3040");
        if (6626 + kK2 - 2 >= 0 && 6626 + kK2 - 2 < (int)fpLines.size()) m2 = Contains(fpLines[6626 + kK2 - 2], mk2.c_str());
        CHECK(m1 && m2, "[F1] the two span markers sit on the line before each span and state the offsets the table uses");
        int heads = 0; for (int k = 0; k < kK && k < (int)fpLines.size(); ++k) { if (Contains(fpLines[k], "port line = 910 line + (" + std::to_string(kK) + ")") || Contains(fpLines[k], "port line = 910 line + (" + std::to_string(kK2) + ")")) ++heads; }
        CHECK(heads >= 2, "[F1] the file head states both offsets");
    }
    // ---- [F11] unreachable servo-off arms ----
    {
        std::vector<std::string> lv = LiveCode(fpLines);
        //  DoTestZContactModeStart = 910 :81-233, case 1 = :88-117, case 100 = :149-183;  End = :241-314, case 1 :247-261, case 100 :262-270
        int liveSet = 0;
        std::string where;
        const int ranges[4][2] = { {88, 117}, {149, 183}, {247, 261}, {262, 270} };
        for (int r = 0; r < 4; ++r)
            for (int n = ranges[r][0]; n <= ranges[r][1]; ++n) {
                const std::string& c = lv[n + kK - 1];
                if (c.find("Task=200") != std::string::npos || c.find("Task=300") != std::string::npos || c.find("Task=400") != std::string::npos) { ++liveSet; where += " :" + std::to_string(n); }
            }
        char m[300];
        std::snprintf(m, sizeof(m), "[F11] no live Task=200 / 300 / 400 in the case-1 / case-100 arms of DoTestZContactModeStart / End (910 commented the ServoOff entries :112-116 / :256-260)%s", where.c_str());
        CHECK(liveSet == 0, m);
        int t200 = 0;
        for (int n = 81; n <= 314; ++n) if (lv[n + kK - 1].find("Task=200") != std::string::npos) { if (n != 296) ++t200; }
        CHECK(t200 == 0 && lv[296 + kK - 1].find("Task=200") != std::string::npos, "[F11] the only live Task=200 in the contact-mode machines is 910 :296 (inside case 300, itself unreachable)");
        CHECK(Contains(fpLines[115 + kK - 1], "//") && Contains(fpLines[115 + kK - 1], "Task=200;") && Contains(fpLines[259 + kK - 1], "//") && Contains(fpLines[259 + kK - 1], "Task=200;"),
              "[F11] 910 :115 / :259 (the ServoOff entries) are still commented out (910 原文已註解)");
    }
    // ---- [F13] no ini write-back, no abs() on a raw 6077h ----
    {
        std::vector<std::string> core, hdr;
        ReadLines(root + "\\IndexZFinePitchCore.h", core);
        ReadLines(root + "\\atester_FinePitch.h", hdr);
        int bad = 0, absN = 0, absAt = 0;
        std::vector<std::string>* files[3] = { &fpLines, &core, &hdr };
        for (int f = 0; f < 3; ++f) {
            std::vector<std::string> lv = LiveCode(*files[f]);
            for (std::size_t i = 0; i < lv.size(); ++i) {
                if (HasWord(lv[i], "WriteIniData", false) || lv[i].find("CheckAndReadIniData") != std::string::npos) ++bad;
                if (HasWord(lv[i], "abs", true)) { ++absN; if (f == 0) absAt = (int)i + 1; }
            }
        }
        CHECK(bad == 0, "[F13] no WriteIniData / CheckAndReadIniData* in atester_FinePitch.cpp / .h / IndexZFinePitchCore.h (Gerneral.ini is never written)");
        CHECK(absN == 1 && absAt == 2443 + kK, "[F13] abs( appears once: 910 :2443 `TorqueData=abs(TorqueData);` (golden's no-op on atoi(edTorue0), never a raw 6077h)");
    }
    // ---- [F2] census ----
    {
        std::vector<std::string> files;
        Walk(root, files);
        const char* names[] = { "DoTestHeadMotorFP", "DoTestYFinePitch", "InitTestYFPTask", "DoTestYFrontFP", "DoFrontTestSuckICFP",
                                "DoFrontTestDestroyICFP", "InitContactModeStart", "DoTestZContactModeStart", "InitContactModeEnd",
                                "DoTestZContactModeEnd", "MoveIndexZ", "iTestYFinePitchTask", "W906_FP9050DriveFaultStop",
                                "W906_FP9050ShuttlesClearRefused", "W906_FP9050ShuttleClearStop", "W906_FP9050ShuttlesClearOfIndex", "W906_FP9050Z1Pos",
                                "W906_FP9050CcdYHeld", "W906_FP9050CcdYHeldReset" };
        const int nn = (int)(sizeof(names) / sizeof(names[0]));
        int uses = 0, includers = 0, scanned = 0;
        std::string where;
        int slotOk = 0, ticks = 0;
        std::string slotPrev;
        for (std::size_t f = 0; f < files.size(); ++f) {
            const std::string b = BaseName(files[f]);
            std::vector<std::string> v;
            if (!ReadLines(files[f], v)) continue;
            ++scanned;
            const std::vector<std::string> lv = LiveCode(v);
            const bool own = (b == "atester_finepitch.cpp" || b == "atester_finepitch.h");
            for (std::size_t i = 0; i < lv.size(); ++i) {
                const std::string& c = lv[i];
                if (c.empty()) continue;
                if (!own) {
                    for (int k = 0; k < nn; ++k)
                        if (HasWord(c, names[k], false) && !(b == "cstaterecord.cpp" && k == 11) && !(b == "csystem.cpp" && ((k == 0 && (Norm(c) == "extern void DoTestHeadMotorFP();" || Norm(c) == "DoTestHeadMotorFP();")) || (k == 2 && (Norm(c) == "extern void InitTestYFPTask();" || Norm(c) == "if(iTestHeadMotorTask==1) InitTestYFPTask();"))))) { ++uses; if (where.size() < 300) where += " " + b + ":" + std::to_string(i + 1) + " " + names[k]; }
                    if (c.find("#") != std::string::npos && c.find("include") != std::string::npos) {
                        std::string low = v[i]; for (std::size_t q = 0; q < low.size(); ++q) low[q] = (char)std::tolower((unsigned char)low[q]);
                        if (low.find("atester_finepitch.h") != std::string::npos) { ++includers; where += " include:" + b; }
                    }
                }
                if (b == "csystem.cpp" && Norm(c) == "CSYS_TICK(CT_TESTHEAD);") {
                    ++ticks;
                    for (std::size_t q = i; q-- > 0;) { const std::string pn = Norm(lv[q]); if (!pn.empty()) { slotPrev = pn; break; } }
                    if (slotPrev == "DoTestHeadMotor();") ++slotOk;
                }
            }
        }
        char m[600];
        std::snprintf(m, sizeof(m), "[F2] census over %d files: 0 live uses of the FP entry points / MoveIndexZ / the FP helpers outside atester_FinePitch.cpp/.h (and tests)%s", scanned, where.c_str());
        CHECK(scanned > 500 && uses == 0, m);
        CHECK(includers == 0, "[F2] no source outside tests includes atester_FinePitch.h");
        std::snprintf(m, sizeof(m), "[F2] the live DoAllProcess Index slot (the line before csystem.cpp `CSYS_TICK(CT_TESTHEAD);`) is still golden `DoTestHeadMotor();` (slice 2 flips this pin) -- prev `%s`", slotPrev.c_str());
        CHECK(ticks == 1 && slotOk == 1, m);
        std::vector<std::string> dispatch;
        CHECK(ReadLines(root + "\\csystem.cpp", dispatch), "[SIM9050] dispatch source readable");
        std::string dc;
        for (std::size_t i=0; i<dispatch.size(); ++i) { const std::string line=Norm(dispatch[i]); for (std::size_t j=0; j<line.size(); ++j) if (line[j]!=' ') dc+=line[j]; }
        CHECK(dc.find("#ifdefSOFT_SIMULTEif(MachineTypeChoice==Type_HT9050){externvoidDoTestHeadMotorFP();externvoidInitTestYFPTask();if(iTestHeadMotorTask==1)InitTestYFPTask();DoTestHeadMotorFP();}else#endifDoTestHeadMotor();CSYS_TICK(CT_TESTHEAD);") != std::string::npos,
              "[SIM9050] only HT9050 SIM uses FP; SHIP and other types retain generic Index");

    }
    // ---- [F14] mixed FP / generic flow: the MECHANISM, read from the source (Jimmy 10/05 19:5x via ST01-E) ----
    //  Q2 = B (RULINGS_20261005 #14): on Type_HT9050 MainProc's [I01] sites and CheckNozzleEventFinish keep calling the
    //  GENERIC DoTestHeadMotor (910). The two families share iTestHeadMotorTask / iTestYFrontTask / the suck / destroy
    //  cursors; HOME (InitAllProcessTask) resets only the head task and the suck cursor; FP 1600 resets the GENERIC
    //  InitTestYTask and nothing calls InitTestYFPTask. These pins document 910 as translated -- they are NOT a fix.
    {
        //  every `case N:` label of a top-level function (LiveCode; body found by brace matching from its signature line)
        auto labels = [](const std::vector<std::string>& v, const std::string& sig, int& at) {
            std::vector<int> out;
            const std::vector<std::string> lv = LiveCode(v);
            at = -1;
            for (std::size_t i = 0; i < lv.size(); ++i) if (Norm(lv[i]).compare(0, sig.size(), sig) == 0) { at = (int)i + 1; break; }
            if (at < 0) return out;
            int depth = 0; bool open = false;
            for (std::size_t i = (std::size_t)at - 1; i < lv.size(); ++i) {
                const std::string& c = lv[i];
                for (std::size_t k = 0; k < c.size(); ++k) { if (c[k] == '{') { ++depth; open = true; } else if (c[k] == '}') --depth; }
                std::size_t p = 0;
                while ((p = c.find("case ", p)) != std::string::npos) {
                    if (p == 0 || !IdentChar(c[p - 1])) { const int n = std::atoi(c.c_str() + p + 5); if (n > 0 || c.compare(p + 5, 2, "0:") == 0) out.push_back(n); }
                    p += 5;
                }
                if (open && depth == 0) break;
            }
            return out;
        };
        auto has = [](const std::vector<int>& v, int x) { for (std::size_t i = 0; i < v.size(); ++i) if (v[i] == x) return true; return false; };
        std::vector<std::string> fr, cs;
        ReadLines(root + "\\aTester_Front.cpp", fr);
        ReadLines(root + "\\csystem.cpp", cs);
        int a1 = 0, a2 = 0;
        const std::vector<int> fpY = labels(fpLines, "bool DoTestYFrontFP()", a1);
        const std::vector<int> gY  = labels(fr, "bool DoTestYFront()", a2);
        std::string gOnly; int nGOnly = 0;
        for (std::size_t i = 0; i < gY.size(); ++i) if (!has(fpY, gY[i])) { ++nGOnly; if (nGOnly <= 12) gOnly += " " + std::to_string(gY[i]); }
        char m[500];
        std::snprintf(m, sizeof(m), "[F14] DoTestYFrontFP has %d case labels, generic DoTestYFront %d; %d generic labels have NO case in DoTestYFrontFP (e.g.%s) -- an iTestYFrontTask left there by the generic machine is a task FP never leaves",
                      (int)fpY.size(), (int)gY.size(), nGOnly, gOnly.c_str());
        CHECK(a1 > 0 && a2 > 0 && fpY.size() == 22 && nGOnly > 40, m);
        //  HOME = InitAllProcessTask (csystem.cpp): what it resets, and what it does not
        int ia = 0;
        const std::vector<std::string> lcs = LiveCode(cs);
        std::string body;
        for (std::size_t i = 0; i < lcs.size(); ++i) if (Norm(lcs[i]) == "void InitAllProcessTask()") { ia = (int)i + 1; break; }
        if (ia > 0) { int depth = 0; bool open = false; for (std::size_t i = (std::size_t)ia - 1; i < lcs.size(); ++i) { body += lcs[i] + "\n"; for (std::size_t k = 0; k < lcs[i].size(); ++k) { if (lcs[i][k] == '{') { ++depth; open = true; } else if (lcs[i][k] == '}') --depth; } if (open && depth == 0) break; } }
        const bool resetsHead = HasWord(body, "InitialTestHeadMotorTask", true) && HasWord(body, "InitFrontTestSuckICTask", true);
        const bool resetsFpSub = HasWord(body, "InitTestYTask", true) || HasWord(body, "InitTestYFrontTask", true) || HasWord(body, "InitTestYFPTask", true) ||
                                 HasWord(body, "iTestYFrontTask", false) || HasWord(body, "iTestYFinePitchTask", false) || HasWord(body, "iTestYTask", false);
        CHECK(ia > 0 && resetsHead && !resetsFpSub, "[F14] HOME (InitAllProcessTask) resets iTestHeadMotorTask and the suck cursor, but NOT iTestYTask / iTestYFrontTask / iTestYFinePitchTask (golden + 910)");
        CHECK(Contains(fpLines[2656 + kK - 1], "InitTestYTask();") && Norm(fpLines[2656 + kK - 1]) == "InitTestYTask();",
              "[F14] FP 1600 (910 :2656) resets the GENERIC InitTestYTask(), not InitTestYFPTask (Q1 default keep 910; InitTestYFPTask has no caller, [F2])");
        int gen = 0;
        for (std::size_t i = 0; i < lcs.size(); ++i) if (Norm(lcs[i]) == "DoTestHeadMotor();") ++gen;
        CHECK(gen == 4, "[F14] csystem.cpp keeps 4 live calls of the GENERIC DoTestHeadMotor() (DoAllProcess slot, CheckNozzleEventFinish, the two MainProc [I01] sites) -- Q2 = B, RULINGS_20261005 #14");
    }
    // ---- CMake: the archive row ----
    {
        std::vector<std::string> cm;
        ReadLines(root + "\\CMakeLists.txt", cm);
        int hits = 0, ok = 0;
        bool inSm = false;
        for (std::size_t i = 0; i < cm.size(); ++i) {
            const std::string& l = cm[i];
            if (l.find("add_library(ht9045_sm") != std::string::npos) inSm = true;
            else if (inSm && l.find("add_library(") != std::string::npos) inSm = false;
            const std::size_t h = l.find('#');
            const std::size_t a = l.find("atester_FinePitch.cpp");
            if (a != std::string::npos) { ++hits; if (inSm && (h == std::string::npos || a < h) && l.find("Ht9050TorqueWait.cpp") < a) ++ok; }
        }
        CHECK(hits >= 1 && ok == 1, "[F2] CMakeLists.txt: atester_FinePitch.cpp is on the E-044 line of add_library(ht9045_sm), before its trailing # (an insert after the # is dead text)");
    }
#else   // ===================================================== runtime (god stack) =========================================
    // ---------------- shared setup (golden boot invariants, HT9050) ----------------
    MachineTypeChoice = Type_HT9050;
    INDEX_MOTION_CARD = 0;
    USE_INDEX_ARM_AXES = IndexArm_4_Axis;
    IniConfig.GaliPosRange = 50;
    CosFunction.bIndexProtect = false;
    IniConfig.bEnableCCDUSETCPIP = false;
    REAL_TIME_CCD = 0;
    TTL_CARD_TYPE = 0;
    INDEX_SUCKER_TYPE = 1;                                                    // HT9050
    NEW_MAX_Index_Col = 8;                                                    // HT9050 decode (database.cpp)
    LastSet.iRealDummy = DUMMY;
    LastSet.iTemperature = Tempture_Ambient;
    LastSet.bEnableFinishTestUpWait = false;
    LastSet.bCheckIndexICDestroy = false;
    IniConfig.bD41CheckbySetup = false;
    IniConfig.bD54SlowDown = false;
    IniConfig.bControlTorque = false;
    IniConfig.iD41SocketInitialICCheckPosition = 1; IniConfig.bTestIcCheckInContact = false; IniConfig.fIndexCheckOffset = SOCK_OFS / 100.0;
    CosFunction.bIndexZDownToAboveSocket = false;
    TestIF_File.iTestMode = SingleSite;
    DeviceForm.ContactMode = DirectContactMode;
    iCleanOut = 0; iOneCycle = 0; bCanNotDisableOneCycle = false; bPlaceToShuttleFirst = false;
    Prod.TestZ1_Safe = Z_SAFE; Prod.TestZ1_Test = Z_TEST; Prod.TestZ1_Drop_Offset = 0; Prod.TestZ1_Pick = Z_PICK; Prod.TestZ1_Place = Z_PLACE;
    Prod.All_TestZ_Test_Safe = Z_CCDSAFE; Prod.iMaxPreasure = 15; Prod.iHangupMaxTime = 600;
    Prod.InSHT[0].iLeft = IN_L; Prod.InSHT[0].iRight = IN_R; Prod.OutSHT[0].iLeft = OUT_L; Prod.OutSHT[0].iRight = OUT_R;
    W906_TestEnsureSimMotors();
    TFpSimMotor* inSht = new TFpSimMotor(); TFpSimMotor* outSht = new TFpSimMotor(); TFpSimMotor* ccdY = new TFpSimMotor();
    MOT[MInShuttle1].Motor = inSht; MOT[MOutShuttle1].Motor = outSht; MOT[MCCDY].Motor = ccdY;
    const int ax[3] = { MInShuttle1, MOutShuttle1, MCCDY };
    const char* axNames[3] = { "MInShuttle1", "MOutShuttle1", "MCCDY" };
    for (int k = 0; k < 3; ++k) { MOT[ax[k]].SetAlias(ax[k], axNames[k]); MOT[ax[k]].Motor->Enable = true; MOT[ax[k]].Motor->MotorIdleSafeDoorCheck = &DoorClosed; FreeMot(ax[k]); }
    SetMotPos(MCCDY, 0); MOT[MCCDY].Led[iHomeLed] = true;                     // 910's CCD-Y interlock satisfied
    const char* idxNames[4] = { "MTestY1", "MTestZ1", "MTestZ2", "MTestY2" };
    for (int k = 0; k < 4; ++k) {
        const int i = MTestY1 + k;
        MOT[i].Motor = (i == MTestZ1) ? new TCountingGalil(k) : new TMyGALILMotor(k);
        MOT[i].SetAlias(i, idxNames[k]);                                   // Mot_Name (cinitial.cpp InitialMotorParameter); the Gali_* layer keys on it
        MOT[i].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
        MOT[i].Motor->PServoAlarmOn = true;
        MOT[i].Motor->GearRatio = 1.0;
        MOT[i].Motor->Enable = false;
        for (int l = 0; l < 10; ++l) MOT[i].Led[l] = false;
        FreeMot(i);
    }
    HTMotor* Z = MOT[MTestZ1].Motor;
    Z->PJogHighSpeed = 900000; Z->InitSpeed = 100; Z->SetAccDataBase(9000000.0); Z->SetDecDataBase(9000000.0);   // machines/HT9050/Mot_Table.csv
    Z->Direction = false;
    Z->PSoftLimitP = 999999; Z->PSoftLimitN = -999999;                   // Mot_Table soft limits (0 / 0 would make golden MotorMovePosition return -3 = 'true')
    MOT[MTestZ1].GailSpeed = 90000;
    MOT[MTestZ1].speed = 1000000;                                          // golden's no-card Gali_MotMove steps by TMyMotor::speed: one step = arrival
    LastSet.SoftSpeed[MTestZ1] = 100000; ArmSpeed[1].iBodySP = 100;        // FP's SetMotorScaleSpeed(MTestZ1, ...) rewrites speed = SoftSpeed*scale/100 on a disabled
                                                                           // (SIM) axis (TMyMotor::SetSpeed); 0 from the zeroed LastSet = golden's sim never steps
    IndexZCanMove[0] = IndexZCanMove[1] = true;
    W906_Ht9050ShuttlesClearOfIndexHook = &FakeClear;
    W906_IndexZNowMsHook = &FakeNow;
    W906_Ht9050TorqueWaitClock = &FakeNow;
    W906_Ht9050TorqueWaitPass = &FakePass;
    {
        char b[MAX_PATH + 1] = { 0 };
        ::GetTempPathA(MAX_PATH, b);
        g_dir = std::string(b) + "w906_st01c_" + std::to_string((unsigned long)::GetCurrentProcessId());
        ::CreateDirectoryA(g_dir.c_str(), 0);
        g_ini = g_dir + "\\Gerneral.ini";
    }
    g_oldIni = INIFileGeneral;
    UseIni("HT9050_INDEXZ_TORQUE_CONFIRMED=1");
    COM2->Comm1->SetSimMode(true);
    COM2->Comm1->StartComm();

    // ================= [F15] the CCD calibration arm is refused on HT9050 (Steven 1005 23:1x), both configurations =================
    //  「9050也有ccd,但是不是用來做校正的。後續再改ccd這一段」: the web can set TestIF_File.bEnableCalCCD (JSON struct table); 910's arm
    //  (DoTestZContactModeStart case 1 -> 60: contraposition flag, Out shuttle fCanMoveM, CCD speeds) must not run un-designed.
    std::printf("-- [F15] bEnableCalCCD arm refused on HT9050 (minimal safe refusal)\n");
    {
        ResetFlow();
        TestIF_File.bEnableCalCCD = true;
        bContraPoisitionFlag = false;
        MOT[MOutShuttle1].fCanMoveM = false;
        iInitContactModeStartTask = 1;
        const int z0 = Z1();
        const bool r = DoTestZContactModeStart(DirectContactMode);
        CHECK(!r && !SystemStart && iInitContactModeStartTask == 1 && W906_FP9050LeafCount == 1 && !bContraPoisitionFlag && !MOT[MOutShuttle1].fCanMoveM &&
              Z1() == z0 && W906_ShowMyMessage_Count == 1 && Has(W906_ShowMyMessage_LastS1, "CCD calibration arm"),
              "[F15] HT9050 + bEnableCalCCD: refused at case 1 (message, SystemStart off, task stays 1), the contraposition arm does not run, nothing moves");
        MachineTypeChoice = Type_HT9045;                                      // any other machine: 910's arm, untouched
        ResetFlow();
        TestIF_File.bEnableCalCCD = true;
        iInitContactModeStartTask = 1;
        DoTestZContactModeStart(DirectContactMode);
        CHECK(iInitContactModeStartTask == 60 && bContraPoisitionFlag && MOT[MOutShuttle1].fCanMoveM && SystemStart && W906_FP9050LeafCount == 0,
              "[F15] another machine type: 910's CalCCD arm runs as written (Task 60, contraposition flag, Out shuttle fCanMoveM)");
        MachineTypeChoice = Type_HT9050;
        TestIF_File.bEnableCalCCD = false; bContraPoisitionFlag = false; MOT[MOutShuttle1].fCanMoveM = true;
        ResetFlow();
        iInitContactModeStartTask = 1;
        DoTestZContactModeStart(DirectContactMode);
        CHECK(iInitContactModeStartTask == 100 && SystemStart && W906_FP9050LeafCount == 0, "[F15] HT9050 without bEnableCalCCD: 910's case 1 -> 100 (the production press), no refusal");
        iInitContactModeStartTask = 1;
    }

#ifdef SOFT_SIMULTE
    // ================= [F12] SIM: no protection acts =================
    std::printf("-- [F12] SIM: every ST01-C protection is inert (golden / 910 SOFT_SIMULTE)\n");
    {
        MOT[MTestZ1].CardType = "PCI1203";
        W906_Pci1203IndexZHealthHook = 0;                                   // would be a P7 fault on SHIP
        g_clear = false; W906_Ht9050ShuttlesClearOfIndexHook = 0;           // would be a W-44 stop on SHIP (no guard = fail-safe)
        W906_ShowMyMessage_Reset();
        CHECK(!W906_IndexZLive1203() && !W906_FP9050DriveFaultStop("t") && !W906_FP9050ShuttleClearStop(ht9045::fp9050::kSmHeadMotor, 12101, "t") &&
              W906_FP9050ShuttlesClearOfIndex(0) && !W906_FP9050ShuttlesClearRefused("t") && !W906_GaliRouteOwns(MTestZ1) && W906_ShowMyMessage_Count == 0,
              "[F12] SIM: P7 / W-44 entry / W-44 per tick inert (no guard needed), no route");
        SetMotPos(MCCDY, 5); MOT[MCCDY].Led[iHomeLed] = false;
        SetZ1(Z_SAFE);
        bool r = false;
        for (int k = 0; k < 5 && !r; ++k) r = MoveIndexZ(Z_TEST);
        CHECK(r && Z1() == Z_TEST, "[F12] SIM: MoveIndexZ below All_TestZ_Test_Safe with CCD Y away = 910's #ifndef SOFT_SIMULTE bypass; the Galil-API Index axis arrives through golden's no-card Gali_MotMove simulation");
        SetMotPos(MCCDY, 0); MOT[MCCDY].Led[iHomeLed] = true;
        MOT[MTestZ1].CardType = "";
        g_clear = true; W906_Ht9050ShuttlesClearOfIndexHook = &FakeClear;
    }
    // State Record 20261006 19:38:59: enabled table axis, disabled SIM driver,
    // Z1 at 0 with a positive safe target 200. Exercise the real shuttle guard.
    {
        ResetFlow();
        INDEX_MOTION_CARD=1;
        Prod.TestZ1_Safe=200;
        MOT[MTestZ1].Motor->Enable=false;
        SetZ1(0);
        iTestHeadMotorTask=1600;
        AutoSHT1Task=1;
        MOT[MInShuttle1].fCanMove=MOT[MInShuttle1].fCanMoveR=true;
        MOT[MInShuttle1].fCanMoveM=MOT[MInShuttle1].fCanMoveL=true;
        bContraPoisitionFlag=false;
        extern bool SystemNG; SystemNG=false;
        Do_Auto_InSH();
        CHECK(AutoSHT1Task==1, "[SIM9050] shuttle must remain blocked while Z1 is 0, safe is 200");
        for(int n=0; n<100 && iTestHeadMotorTask!=600; ++n) DoTestHeadMotorFP();
        CHECK(iTestHeadMotorTask==600 && MOT[MTestZ1].ReadPos()==200,
              "[SIM9050] FP moves disabled SIM driver Z1 to positive safe height before production");
        Do_Auto_InSH();
        CHECK(AutoSHT1Task==10, "[SIM9050] actual In Shuttle gate advances only after FP reaches safe height");
        INDEX_MOTION_CARD=0;
        Prod.TestZ1_Safe=Z_SAFE;
    }
    // ================= [F3] SIM walk =================
    std::printf("-- [F3] SIM walk: index check, then one IC pick / press / test / lift / place cycle\n");
    {
        ResetFlow();
        SetZ1(0);
        SetMotPos(MInShuttle1, IN_R); SetMotPos(MOutShuttle1, OUT_R);          // the In shuttle waits under the index with nothing on it yet
        b1ShuttleMoveToLeft = false;
        std::vector<int> zs;                                                   // Z1 positions, consecutive duplicates collapsed
        std::vector<int> hm;                                                   // DoTestHeadMotorFP tasks visited, collapsed
        auto rec = [&]() {
            const int z = Z1();
            if (zs.empty() || zs.back() != z) zs.push_back(z);
            if (hm.empty() || hm.back() != iTestHeadMotorTask) hm.push_back(iTestHeadMotorTask);
        };
        rec();
        int ticks = 0;
        while (iTestHeadMotorTask != 600 && ticks < 400) { Tick(); ++ticks; rec(); if (iTestHeadMotorTask == 122100 || iTestHeadMotorTask == 122 || iTestHeadMotorTask == 1720) ::Sleep(60); }
        const int expHm[] = { 1, 11, 9, 10, 20, 21, 100, 120, 12000, 12100, 12300, 121, 122100, 122110, 122, 130, 15000, 15100, 1550, 1600, 600 };
        bool hmOk = hm.size() == sizeof(expHm) / sizeof(expHm[0]);
        for (std::size_t i = 0; hmOk && i < hm.size(); ++i) if (hm[i] != expHm[i]) hmOk = false;
        std::string seen; for (std::size_t i = 0; i < hm.size(); ++i) seen += " " + std::to_string(hm[i]);
        char m[600];
        std::snprintf(m, sizeof(m), "[F3] index check task path 1 -> 11 -> 9 -> 10 -> 20 -> 21 -> 100 -> 120 -> 12000 (SIM ret=1) -> 12100/12101 -> 12300 (12110 unreachable in SIM) -> 121 -> 122100 -> 122110 -> 122 -> 130 -> 15000 -> 15100 -> 1550 -> 1600 -> 600 (seen:%s)", seen.c_str());
        CHECK(hmOk, m);
        const int expZ1[] = { 0, Z_SAFE, Z_TEST, Z_TEST + SOCK_OFS, Z_SAFE };
        bool zOk = zs.size() == 5;
        for (std::size_t i = 0; zOk && i < zs.size(); ++i) if (zs[i] != expZ1[i]) zOk = false;
        std::string zseen; for (std::size_t i = 0; i < zs.size(); ++i) zseen += " " + std::to_string(zs[i]);
        std::snprintf(m, sizeof(m), "[F3] index check Z1 targets: Safe, Test-Drop (empty socket), Test-Drop+GetSocketCheckPos, Safe (seen:%s)", zseen.c_str());
        CHECK(zOk, m);
        CHECK(fAllMotorHome && SystemStart && W906_ShowErrorMessage_Count == 0 && W906_FP9050LeafCount == 0, "[F3] no alarm, no leaf stop, no port exit during the index check");

        //  one IC: In Arm placed it on the In shuttle; the In shuttle is under the index (InSHT1InRT), the Out shuttle is clear
        FLCarryKit.SetItemData(0, 0, HAS_IC);
        zs.clear(); zs.push_back(Z1());
        std::vector<int> yf;
        bool sawInMoveM = false, sawOutM1600 = false, inLeftDone = false, outLeftDone = false, testerAnswered = false;
        bool fCanMoveR130 = false;
        MOT[MInShuttle1].fCanMoveR = false;                                    // 910 sets it at 130 (Out shuttle clear)
        ticks = 0;
        while (ticks < 600) {
            Tick(); ++ticks;
            const int z = Z1(); if (zs.back() != z) zs.push_back(z);
            if (yf.empty() || yf.back() != iTestYFrontTask) yf.push_back(iTestYFrontTask);
            //  the shuttle machines' half of the handshake (Do_Auto_InSH / Do_Auto_OutSH, played by the test):
            if (iTestYFrontTask == 130 && !inLeftDone && MOT[MInShuttle1].fCanMoveM) {      // FP released the In shuttle -> it goes left (clear)
                sawInMoveM = true; SetMotPos(MInShuttle1, IN_L); b1ShuttleMoveToLeft = true; inLeftDone = true;
            }
            if (iTestYFrontTask == 180 && MOT[MInShuttle1].fCanMoveR) fCanMoveR130 = true;
            if ((iTestYFrontTask == 208 || iTestYFrontTask == 209) && !testerAnswered) {        // the tester answers: test done, bin PASS
                FTestSuck.SetItemData(0, 0, TEST_PASS); fIndexNeedTest = false; testerAnswered = true;
            }
            if (iTestYFrontTask == 1600 && !outLeftDone && MOT[MOutShuttle1].fCanMoveM) {   // FP asks for the Out shuttle under the index
                sawOutM1600 = true; SetMotPos(MOutShuttle1, OUT_L); outLeftDone = true;
            }
            if (iTestYFrontTask == 5000 || (outLeftDone && iTestYFrontTask == 1 && iTestYFinePitchTask != 110)) break;
            if (iFrontTestSuckICTask == 310 || iFrontTestDestroyICTask == 310 || iTestYFrontTask == 1100) ::Sleep(20);
        }
        std::string yseen; for (std::size_t i = 0; i < yf.size(); ++i) yseen += " " + std::to_string(yf[i]);
        const int expYf[] = { 1, 100, 110, 120, 130, 180, 200, 208, 209, 1500, 1600, 1700, 5000 };   // 209 falls through into 210 in the same tick
        bool yOk = true; std::size_t q = 0;
        for (std::size_t i = 0; i < yf.size() && q < sizeof(expYf) / sizeof(expYf[0]); ++i) if (yf[i] == expYf[q]) ++q;
        yOk = (q == sizeof(expYf) / sizeof(expYf[0]));
        std::snprintf(m, sizeof(m), "[F3] IC cycle DoTestYFrontFP path 1 -> 100 -> 110 -> 120 (pick) -> 130 -> 180 (press) -> 200 -> 208 -> 209/210 (test, one tick) -> 1500 -> 1600 -> 1700 (place) -> 5000 (seen:%s)", yseen.c_str());
        CHECK(yOk, m);
        const int expZ2[] = { Z_SAFE, Z_PICK, Z_SAFE, Z_TEST, Z_SAFE, Z_PLACE, Z_SAFE };
        bool z2Ok = zs.size() == sizeof(expZ2) / sizeof(expZ2[0]);
        for (std::size_t i = 0; z2Ok && i < zs.size(); ++i) if (zs[i] != expZ2[i]) z2Ok = false;
        zseen.clear(); for (std::size_t i = 0; i < zs.size(); ++i) zseen += " " + std::to_string(zs[i]);
        std::snprintf(m, sizeof(m), "[F3] IC cycle Z1 targets: Safe, Pick (In shuttle), Safe, Test (socket), Safe, Place (Out shuttle), Safe (seen:%s)", zseen.c_str());
        CHECK(z2Ok, m);
        CHECK(sawInMoveM && fCanMoveR130 && sawOutM1600, "[F3] shuttle handshake: In shuttle released (fCanMoveM) after the pick, fCanMoveR at 130 with the Out shuttle clear, Out shuttle requested (fCanMoveM) at 1600");
        CHECK(FRCarryKit.Item[0][0] == TEST_PASS && FTestSuck.NoIC() && FLCarryKit.NoIC(), "[F3] the tested IC ended on the Out shuttle (FRCarryKit), Index and In shuttle empty");
        CHECK(fAllMotorHome && SystemStart && W906_ShowErrorMessage_Count == 0 && W906_FP9050LeafCount == 0, "[F3] no alarm, no leaf stop, no port exit during the cycle");
    }
    // ================= [F14] mixed FP / generic flow, LIVE (Jimmy 10/05 19:5x via ST01-E; Q2 = B) =================
    //  Documents what 910 does as translated -- NOT a fix (the code keeps 910). Sequence:
    //    (1) FP in production presses an IC on the socket and is about to test it (DoTestYFrontFP 209/210, bFinshTest false);
    //    (2) an alarm takes fAllMotorHome away; START with [I01] TesterFinishThenHome + tester ON_LINE: MainProc
    //        (csystem.cpp, golden :17597) calls the GENERIC DoTestHeadMotor() every pass while bFinshTest==false;
    //    (3) HOME (InitAllProcessTask) and production START again: FP resumes.
    std::printf("-- [F14] mixed FP / generic flow (shared task cursors)\n");
    {
        ResetFlow();
        LastSet.bUseTestSocket[0][0][0] = true;                                // site 1 in use (HT9050 single site)
        SetZ1(Z_SAFE);
        SetMotPos(MInShuttle1, IN_R); SetMotPos(MOutShuttle1, OUT_R); b1ShuttleMoveToLeft = false;
        MOT[MInShuttle1].fCanMoveR = false;
        iTestHeadMotorTask = 600; iTestYFinePitchTask = 1; InitTestYTask();   // where FP 1600 leaves them (it calls the generic InitTestYTask)
        FLCarryKit.SetItemData(0, 0, HAS_IC);
        bool inLeft = false;
        int ticks = 0;
        while (iTestYFrontTask != 209 && ticks < 400) {
            Tick(); ++ticks;
            if (iTestYFrontTask == 130 && !inLeft && MOT[MInShuttle1].fCanMoveM) { SetMotPos(MInShuttle1, IN_L); b1ShuttleMoveToLeft = true; inLeft = true; }
            if (iFrontTestSuckICTask == 310) ::Sleep(20);
        }
        bFinshTest = false;                                                    // DoFTestSuckTestIC sets it at the start of a test (aTester_Front.cpp)
        char m[600];
        std::snprintf(m, sizeof(m), "[F14] (1) FP mid-test: head 600 / FinePitch %d / TestYFront %d / generic TestY %d, IC untested on the index, Z1 pressed at %d",
                      iTestYFinePitchTask, iTestYFrontTask, iTestYTask, Z1());
        CHECK(iTestHeadMotorTask == 600 && iTestYFinePitchTask == 110 && iTestYFrontTask == 209 && iTestYTask == 1 &&
              FTestSuck.Item[0][0] == HAS_IC && Z1() == Z_TEST && fIndexNeedTest, m);
        //  (2) [I01]: the generic machine runs on FP's cursors
        fAllMotorHome = false; iHome = 1;
        IniConfig.bI01TesterFinishThenHome = true; LastSet.iTester = ON_LINE; bWaitTesterFinish = true;
        std::vector<int> gTy, gYf, gYr;
        auto push = [](std::vector<int>& v, int x) { if (v.empty() || v.back() != x) v.push_back(x); };
        push(gTy, iTestYTask); push(gYf, iTestYFrontTask); push(gYr, iTestYRearTask);
        const int z0 = Z1();
        bool testedByGeneric = false, finished = false;
        for (int k = 0; k < 300 && !finished; ++k) {
            g_now += 500; ++g_passNo;
            DoTestHeadMotor();
            push(gTy, iTestYTask); push(gYf, iTestYFrontTask); push(gYr, iTestYRearTask);
            if (FTestSuck.Item[0][0] >= TEST_PASS) testedByGeneric = true;
            if (bFinshTest) finished = true;
            ::Sleep(2);
        }
        std::string s1, s2, s3;
        for (std::size_t i = 0; i < gTy.size() && i < 20; ++i) s1 += " " + std::to_string(gTy[i]);
        for (std::size_t i = 0; i < gYf.size() && i < 20; ++i) s2 += " " + std::to_string(gYf[i]);
        for (std::size_t i = 0; i < gYr.size() && i < 20; ++i) s3 += " " + std::to_string(gYr[i]);
        std::printf("    [F14] (2) generic DoTestHeadMotor x300: iTestYTask%s | iTestYFrontTask%s | iTestYRearTask%s | head %d | Z1 %d -> %d | bFinshTest %d | FTestSuck[0][0] %d\n",
                    s1.c_str(), s2.c_str(), s3.c_str(), iTestHeadMotorTask, z0, Z1(), (int)bFinshTest, FTestSuck.Item[0][0]);
        const int afterGenYf = iTestYFrontTask, afterGenTy = iTestYTask;
        std::snprintf(m, sizeof(m), "[F14] (2) REPRODUCES (pinned 910 behaviour): 300 [I01] passes of the generic machine never finish FP's test (bFinshTest stays false, the IC stays untested) -> MainProc keeps 'Wait Tester' and never homes (iTestYTask%s)", s1.c_str());
        CHECK(!finished && !testedByGeneric, m);
        //  (3) HOME anyway (operator clears it / power cycle): InitAllProcessTask, Z1 safe; production START -> FP resumes
        InitAllProcessTask();
        SetZ1(Z_SAFE);
        fAllMotorHome = true; iHome = 0; bWaitTesterFinish = false; bFinshTest = true;
        std::snprintf(m, sizeof(m), "[F14] (3) after HOME the FP sub-cursors are NOT reset: FinePitch %d, TestYFront %d (left by the generic run), generic TestY %d; head %d",
                      iTestYFinePitchTask, iTestYFrontTask, iTestYTask, iTestHeadMotorTask);
        CHECK(iTestHeadMotorTask == 1 && iTestYFinePitchTask == 110 && iTestYFrontTask == afterGenYf && iTestYTask == afterGenTy, m);
        std::vector<int> fHm, fYf;
        push(fHm, iTestHeadMotorTask); push(fYf, iTestYFrontTask);
        for (int k = 0; k < 400; ++k) {
            Tick();
            push(fHm, iTestHeadMotorTask); push(fYf, iTestYFrontTask);
            if (iTestHeadMotorTask == 122100 || iTestHeadMotorTask == 122) ::Sleep(60);
        }
        std::string s4, s5;
        for (std::size_t i = 0; i < fHm.size() && i < 30; ++i) s4 += " " + std::to_string(fHm[i]);
        for (std::size_t i = 0; i < fYf.size() && i < 30; ++i) s5 += " " + std::to_string(fYf[i]);
        std::printf("    [F14] (3) FP resumed x400: head%s | TestYFront%s | FinePitch %d | FTestSuck[0][0] %d | Z1 %d\n",
                    s4.c_str(), s5.c_str(), iTestYFinePitchTask, FTestSuck.Item[0][0], Z1());
        std::snprintf(m, sizeof(m), "[F14] (3) REPRODUCES (pinned 910 behaviour): FP resumed at the stale DoTestYFrontFP task (%d) -- it skipped 130 (shuttle gate) and 180 (the press) and waits in 210 for the tester on an IC that is NOT in the socket (Z1 at Safe %d, FTestSuck[0][0] %d untested); 400 passes, the cycle never ends (FinePitch %d)",
                      iTestYFrontTask, Z1(), FTestSuck.Item[0][0], iTestYFinePitchTask);
        CHECK(iTestHeadMotorTask == 600 && iTestYFinePitchTask == 110 && iTestYFrontTask == 210 && fIndexNeedTest &&
              Z1() == Z_SAFE && FTestSuck.Item[0][0] == HAS_IC, m);
        W906_ShowMyMessage_Reset();
        LastSet.bUseTestSocket[0][0][0] = false;
        IniConfig.bI01TesterFinishThenHome = false; LastSet.iTester = 0;
    }
    //  [F14] the direct form: a generic-only task number in iTestYFrontTask (e.g. what the generic front machine leaves)
    {
        ResetFlow();
        SetZ1(Z_SAFE);
        iTestHeadMotorTask = 600; iTestYFinePitchTask = 110; iTestYFrontTask = 55;   // 55: a generic DoTestYFront case, none in DoTestYFrontFP
        FLCarryKit.SetItemData(0, 0, HAS_IC);
        for (int k = 0; k < 200; ++k) Tick();
        CHECK(iTestYFrontTask == 55 && iTestYFinePitchTask == 110 && iTestHeadMotorTask == 600 && FLCarryKit.Item[0][0] == HAS_IC &&
              W906_ShowMyMessage_Count == 0 && W906_ShowErrorMessage_Count == 0,
              "[F14] REPRODUCES (pinned 910 behaviour): iTestYFrontTask on a generic-only number (55) -> DoTestYFrontFP matches no case: 200 passes, nothing moves, no message, no alarm -- the Index is stuck silently");
    }
#else   // SHIP
    g_route.owner = MTestZ1;
    g_core.Bind(&g_io);
    MOT[MTestZ1].Motor->Enable = true;
    MOT[MTestZ1].CardType = "PCI1203";
    INDEX_DRIVER_TYPE = Panasonic_DRIVER;
    iPanasonicDriverType = Panasonic_DRIVER_A5;                                // as test_indexz_autoheight_1203
    TorqueUseHPComCard = false;
    W906_Pci1203TorqueReadHook   = &FakeRead;
    W906_Pci1203IndexZHealthHook = &FakeHealth;
    W906_Pci1203TorqueLimitHook  = &FakeLimit;
    W906_SetGaliRoute(&g_route);
    W906_GaliRouteDisableAbsentIndexAxes(false, false, false);

    // ================= [F4] MoveIndexZ reaches the 1203 through the route =================
    std::printf("-- [F4] MoveIndexZ: routed Gali_MotMove, never golden's no-card MoveToPos\n");
    {
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        g_moveToPos = 0;
        bool arrived = false; int n = 0;
        while (!arrived && n < 50) { Fresh(); arrived = MoveIndexZ(-2000); ++n; }
        CHECK(arrived && HasMove(-2000) && g_moveToPos == 0, "[F4] MoveIndexZ(-2000): the 1203 got the move (card target -2000), TMyGALILMotor::MoveToPos never reached, arrived");
        CHECK(W906_FP9050Z1Pos() == -2000, "[F4] W906_FP9050Z1Pos() reads the routed position (Gali_ReadPos)");
        //  F1 in class terms: 910's own call, MOT[MTestZ1].MotorMove, on this TMyGALILMotor (route installed, no DMC card)
        g_moves.clear(); g_moveToPos = 0;
        bool arr2 = false;
        for (int k = 0; k < 20 && !arr2; ++k) { Fresh(); arr2 = (MOT[MTestZ1].MotorMove(-2500) == 1); }
        //  AI(W906-ST01C) 20261005: measured (SHIP, 21:15): golden's no-card MotorMovePosition reaches MoveToPos (false), then
        //  on the next call reports the move as DONE (arrived=1, Position stays what TMyGALILMotor::ReadPos says: 0) --
        //  worse than the plan's "false for ever": 910's MoveIndexZ would let the flow go on as if Z1 had moved.
        char m4[300];
        std::snprintf(m4, sizeof(m4), "[F4] 910's verbatim MotorMove on the Galil-class Index axis: TMyGALILMotor::MoveToPos is reached (needs a DMC card), NOTHING reaches the 1203 (MotorMove reported %s) -- plan F1",
                      arr2 ? "a FAKE arrival" : "no arrival");
        CHECK(g_moves.empty() && g_moveToPos > 0, m4);
        MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].fCMD = false;
        //  the class selector: Index axes on another HTMotor class (INDEX_MOTION_CARD != 0, e.g. EtherCAT / MotionNet) -> 910's MotorMove
        HTMotor* galil = MOT[MTestZ1].Motor;
        TFpSimMotor* other = new TFpSimMotor();
        other->Enable = true; other->MotorIdleSafeDoorCheck = &DoorClosed; other->PSoftLimitP = 999999; other->PSoftLimitN = -999999;
        MOT[MTestZ1].Motor = other;
        INDEX_MOTION_CARD = 1;
        SetZ1(Z_SAFE); g_moves.clear();
        bool arr3 = false;
        for (int k = 0; k < 20 && !arr3; ++k) arr3 = MoveIndexZ(-2600);
        CHECK(arr3 && Z1() == -2600 && g_moves.empty(), "[F4] INDEX_MOTION_CARD != 0 (the Index axis is another HTMotor class): MoveIndexZ = 910's MotorMove, the axis's own MoveToPos moves it, the Galil route is not used");
        INDEX_MOTION_CARD = 0;
        MOT[MTestZ1].Motor = galil;
        delete other;
        g_target = Z_SAFE; Fresh();
        MOT[MTestZ1].MovFlag = false; MOT[MTestZ1].fCMD = false;
    }
    // ================= [F5] 910's CCD-Y interlock + Q4b =================
    std::printf("-- [F5] CCD-Y interlock (910 MoveIndexZ) and Q4b's message\n");
    {
        ResetFlow();
        for (int v = 0; v < 2; ++v) {
            g_target = Z_SAFE; Fresh(); g_moves.clear();
            if (v == 0) { SetMotPos(MCCDY, 5); MOT[MCCDY].Led[iHomeLed] = true; } else { SetMotPos(MCCDY, 0); MOT[MCCDY].Led[iHomeLed] = false; }
            bool any = false;
            for (int k = 0; k < 5; ++k) { Fresh(); if (MoveIndexZ(-5000)) any = true; }
            CHECK(!any && g_moves.empty(), v == 0 ? "[F5] M108 at 5 (ORG lit): a descent below All_TestZ_Test_Safe is refused, nothing commanded" : "[F5] M108 at 0, ORG off: refused, nothing commanded");
        }
        g_target = -5000; Fresh(); g_moves.clear();
        bool there = false; for (int k = 0; k < 3; ++k) { Fresh(); there = MoveIndexZ(-5000); }
        CHECK(there && g_moves.empty(), "[F5] refused but Z1 already at the target: true (910), nothing commanded");
        g_target = Z_SAFE; Fresh();
        bool above = false; for (int k = 0; k < 40 && !above; ++k) { Fresh(); above = MoveIndexZ(-2500); }
        CHECK(above && HasMove(-2500), "[F5] a target at / above All_TestZ_Test_Safe is never refused");
        SetMotPos(MCCDY, 0); MOT[MCCDY].Led[iHomeLed] = true; g_moves.clear();
        bool down = false; for (int k = 0; k < 40 && !down; ++k) { Fresh(); down = MoveIndexZ(-5000); }
        CHECK(down && HasMove(-5000), "[F5] M108 at 0 with ORG lit: the descent is commanded");
        //  Q4b
        W906_FP9050Reset(); W906_ShowMyMessage_Reset();
        g_target = Z_SAFE; Fresh(); g_moves.clear();
        SetMotPos(MCCDY, 5);
        int msgAt = -1;
        for (int k = 0; k <= 30; ++k) { g_now += 500; Fresh(); MoveIndexZ(-5000); if (msgAt < 0 && W906_ShowMyMessage_Count > 0) msgAt = k; }
        CHECK(msgAt == 10 && W906_ShowMyMessage_Count == 1 && Has(W906_ShowMyMessage_LastS1, "CCD Y (M108)") && g_moves.empty(),
              "[F5] Q4b: one message after 5 s of refusals (not at 4.5 s), naming M108; 15 s of refusals = one message; nothing commanded");
        SetMotPos(MCCDY, 0);
        bool ok2 = false; for (int k = 0; k < 40 && !ok2; ++k) { g_now += 500; Fresh(); ok2 = MoveIndexZ(-5000); }
        SetMotPos(MCCDY, 5); g_target = Z_SAFE; Fresh();
        const int c0 = W906_ShowMyMessage_Count;
        int again = -1;
        for (int k = 0; k <= 12; ++k) { g_now += 500; Fresh(); MoveIndexZ(-6000); if (again < 0 && W906_ShowMyMessage_Count > c0) again = k; }
        CHECK(ok2 && again == 10, "[F5] Q4b: an accepted descent ends the window; a new refusal episode gets its own message after 5 s");
        SetMotPos(MCCDY, 0); MOT[MCCDY].Led[iHomeLed] = true;
    }
    // ================= [F7] torque-limit writes reach the real iWriteAndCheckMotorTorque =================
    std::printf("-- [F7] 12000 / 12200 torque-limit writes\n");
    {
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        iTestHeadMotorTask = 12000;
        Tick();
        CHECK(!g_limits.empty() && g_limits.back() == 150 && iTestHeadMotorTask == 12100, "[F7] 12000: Prod.iMaxPreasure 15 % -> the 1203 limit hook got 150 (0.1 %), Task 12100");
        iTestHeadMotorTask = 12200;
        fMain->chkReadTorque1->Checked = true;
        Tick();
        CHECK(g_limits.back() == 3000 && iTestHeadMotorTask == 12300 && !fMain->chkReadTorque1->Checked, "[F7] 12200: limit back to 300 % (3000), reader flags cleared, Task 12300");
        W906_Pci1203TorqueLimitHook = 0;
        iTestHeadMotorTask = 12000;
        W906_ShowMyMessage_Reset();
        Tick();
        CHECK(iTestHeadMotorTask == 1 && !fAllMotorHome && Has(W906_ShowMyMessage_LastS1, "Motor torque set error"), "[F7] no 1203 limit hook: 12000 gets 910's ret==2 exit (message, fAllMotorHome=false, Task 1) -- never a fake OK");
        W906_Pci1203TorqueLimitHook = &FakeLimit;
    }
    // ================= [F6] FP 12110 E-044 =================
    std::printf("-- [F6] FP 12110: E-044 bounded torque wait\n");
    {
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        g_torqueOk = false;                                                   // no 6077h value ever
        iTestHeadMotorTask = 12101;
        int n = 0;
        while (iTestHeadMotorTask != 12110 && n < 60) { Tick(true); ++n; }
        CHECK(iTestHeadMotorTask == 12110 && HasMove(Z_TEST), "[F6] 12101 pressed the empty socket (Test-Drop) and reached 12110");
        const unsigned long t0 = g_now;
        int ticks = 0;
        while (fAllMotorHome && ticks < 40) { Tick(true); ++ticks; }
        const unsigned long waited = g_now - t0;
        CHECK(!fAllMotorHome && iTestHeadMotorTask == 1 && W906_ShowErrorMessage_LastCode == AnsiString("WAR0361") &&
              !fMain->chkReadTorque1->Checked && !fMain->chkReadTorque2->Checked && waited >= 5000ul && waited <= 6000ul,
              "[F6] no value: WAR0361 after 5 s (not before), reader disarmed, 910's exit fAllMotorHome=false / Task 1");
        //  a value in time: compared, no alarm
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        g_torqueOk = true; g_rawPress = -50;                                  // 5 % < kg 15
        iTestHeadMotorTask = 12101;
        n = 0;
        while (iTestHeadMotorTask != 12200 && iTestHeadMotorTask != 12300 && n < 80 && fAllMotorHome) { Tick(true); ++n; }
        CHECK((iTestHeadMotorTask == 12200 || iTestHeadMotorTask == 12300) && fAllMotorHome && W906_ShowErrorMessage_Count == 0,
              "[F6] a value within 5 s (5.00 < 15): 910 compares it and goes on to 12200, no alarm");
        //  F6 / Q7: 12101 with an IC on the index -> 12110 without arming the reader: still bounded by E-044
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        g_torqueOk = false;
        FTestSuck.SetItemData(0, 0, HAS_IC);
        iTestHeadMotorTask = 12101;
        Tick(true);
        CHECK(iTestHeadMotorTask == 12110 && !HasMove(Z_TEST), "[F6] 12101 with an IC on the index: 910 jumps to 12110 without pressing");
        ticks = 0;
        while (fAllMotorHome && ticks < 40) { Tick(true); ++ticks; }
        CHECK(!fAllMotorHome && W906_ShowErrorMessage_LastCode == AnsiString("WAR0361"), "[F6] ... and E-044 still ends the wait (WAR0361) -- Q7 keeps 910's unarmed jump");
        FTestSuck.SetItemData(0, 0, NULL_IC);
        //  a non-1203 row: 910 waits for ever
        ResetFlow();
        W906_SetGaliRoute(0);
        MOT[MTestZ1].CardType = "";
        MOT[MTestZ1].Motor->Enable = false;
        SetZ1(Z_SAFE);
        iTestHeadMotorTask = 12101;
        for (int k = 0; k < 130; ++k) Tick(false);
        CHECK(iTestHeadMotorTask == 12110 && fAllMotorHome && W906_ShowErrorMessage_Count == 0, "[F6] CardType != PCI1203: 65 s at 12110, no E-044 alarm, no exit (910)");
        MOT[MTestZ1].Motor->Enable = true;
        MOT[MTestZ1].CardType = "PCI1203";
        W906_SetGaliRoute(&g_route);
    }
    // ================= [F8] the E-038 value path -> 12110 compare -> WAR0321 =================
    std::printf("-- [F8] value path: 6077h -> edTorue0 -> FP 12110; over the limit 11 times -> 12111 -> 12112 WAR0321\n");
    {
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        g_torqueOk = true; g_rawPress = -150;                                 // 15.00 >= kg 15
        iTestHeadMotorTask = 12101;
        int n = 0;
        while (iTestHeadMotorTask != 9 && n < 600 && fAllMotorHome) { Tick(true); ++n; }
        CHECK(iTestHeadMotorTask == 9 && W906_ShowErrorMessage_LastCode == AnsiString("WAR0321") && Has(W906_ShowMyMessage_LastS1, "contact force over error") &&
              g_moves.back() == Z_SAFE && W906_ShowErrorMessage_Count == 1,
              "[F8] the bridge value reached FP's edTorue0 every press; > 10 over-limit reads -> 12111 (Z1 to Safe) -> 12112 WAR0321 -> Task 9; no E-044 alarm");
    }
    // ================= [F9] W-44 =================
    std::printf("-- [F9] W-44: entry beside 910's case-130 gate + per tick on the press / hold tasks\n");
    {
        const int hm[] = { 40, 50, 55, 12100, 12101, 12110, 12200, 12300, 121, 122100, 122110, 122, 130 };
        int okN = 0;
        for (std::size_t i = 0; i < sizeof(hm) / sizeof(hm[0]); ++i) {
            ResetFlow();
            g_target = Z_TEST; Fresh();
            g_clear = false;                                                  // Frank's guard: a shuttle inside the safe zone
            iTestHeadMotorTask = hm[i];
            const int mv = Moves();
            Tick();
            if (g_stops == 1 && !fAllMotorHome && iTestHeadMotorTask == 1 && Moves() == mv && W906_ShowMyMessage_Count == 1 && Has(W906_ShowMyMessage_LastS1, "W-44")) ++okN;
            else std::printf("    task %d: stops %d home %d task %d msgs %d\n", hm[i], g_stops, (int)fAllMotorHome, iTestHeadMotorTask, W906_ShowMyMessage_Count);
        }
        CHECK(okN == (int)(sizeof(hm) / sizeof(hm[0])), "[F9] DoTestHeadMotorFP: every press / hold task with the In shuttle away -> ST Z1 + one message + exit (Task 1), no move");
        const int yfp[] = { 180, 200, 208, 209, 210, 300, 310, 330 };
        okN = 0;
        for (std::size_t i = 0; i < sizeof(yfp) / sizeof(yfp[0]); ++i) {
            ResetFlow();
            g_target = Z_TEST; Fresh();
            g_clear = false;                                                  // Frank's guard: a shuttle inside the safe zone
            iTestYFrontTask = yfp[i];
            const int mv = Moves();
            const bool r = DoTestYFrontFP();
            if (!r && g_stops == 1 && !fAllMotorHome && iTestYFrontTask == yfp[i] && Moves() == mv && W906_ShowMyMessage_Count == 1) ++okN;
            else std::printf("    yf task %d: stops %d home %d msgs %d\n", yfp[i], g_stops, (int)fAllMotorHome, W906_ShowMyMessage_Count);
        }
        CHECK(okN == (int)(sizeof(yfp) / sizeof(yfp[0])), "[F9] DoTestYFrontFP: every press / hold task with the Out shuttle away -> ST Z1 + one message + exit, no move");
        //  lifts are never stopped
        ResetFlow();
        g_target = Z_TEST; Fresh(); SetZ1(Z_TEST);
        g_clear = false;
        iTestHeadMotorTask = 15000;
        bool lifted = false;
        for (int k = 0; k < 40 && !lifted; ++k) { Tick(); lifted = (iTestHeadMotorTask == 15100); }
        CHECK(lifted && HasMove(Z_SAFE) && fAllMotorHome && g_stops == 0, "[F9] DoTestHeadMotorFP 15000 (lift to Safe) with the In shuttle away: not stopped, Z1 lifted");
        ResetFlow();
        g_target = Z_TEST; Fresh(); SetZ1(Z_TEST);
        g_clear = false;
        iTestYFrontTask = 1500; iInitContactModeEndTask = 1;
        bool up = false;
        for (int k = 0; k < 40 && !up; ++k) { g_now += 500; Fresh(); DoTestYFrontFP(); up = (iTestYFrontTask == 1600); }
        CHECK(up && HasMove(Z_SAFE) && fAllMotorHome && g_stops == 0, "[F9] DoTestYFrontFP 1500 (lift after the test) with the Out shuttle away: not stopped, Z1 lifted");
        //  the entry beside 910's case-130 gate
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        SetMotPos(MInShuttle1, IN_L); SetMotPos(MOutShuttle1, OUT_R); b1ShuttleMoveToLeft = true;   // 910's own gate passes
        g_clear = false;                                                      // ... but Frank's guard says a shuttle is inside the zone
        iTestYFrontTask = 130;
        const bool r130 = DoTestYFrontFP();
        CHECK(!r130 && !SystemStart && iTestYFrontTask == 130 && g_stops == 0 && Moves() == 0 && W906_ShowMyMessage_Count == 1 && Has(W906_ShowMyMessage_LastS1, "refused"),
              "[F9] entry at 130: refused (SystemStart off, task stays 130), nothing commanded -- not even a stop");
        ResetFlow();
        g_target = Z_SAFE; Fresh();
        b1ShuttleMoveToLeft = true;
        iTestYFrontTask = 130; iInitContactModeStartTask = 77;
        DoTestYFrontFP();
        CHECK(iTestYFrontTask == 180 && iInitContactModeStartTask == 1 && SystemStart && W906_ShowMyMessage_Count == 0, "[F9] entry at 130 with both shuttles home: 910 runs (InitContactModeStart, Task 180)");
        //  W-44 stop mid-press: the shuttle leaves home while Z1 is held at the socket
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        g_torqueOk = false;
        iTestHeadMotorTask = 12101;
        int n = 0;
        while (iTestHeadMotorTask != 12110 && n < 60) { Tick(true); ++n; }
        g_clear = false;
        const int mv = Moves();
        Tick(true);
        CHECK(iTestHeadMotorTask == 1 && !fAllMotorHome && g_stops == 1 && Moves() == mv && W906_ShowErrorMessage_Count == 0,
              "[F9] Out shuttle leaves home during the 12110 hold: ST Z1 in that tick, exit, no E-044 alarm on top");
        b1ShuttleMoveToLeft = false;
    }
    // ================= [F16] W-44 hook missing = fail-safe (Frank's guard not in this build) =================
    std::printf("-- [F16] W-44: no safe-zone guard installed -> not clear (refuse / stop)\n");
    {
        ResetFlow();
        W906_Ht9050ShuttlesClearOfIndexHook = 0;
        AnsiString why;
        CHECK(!W906_FP9050ShuttlesClearOfIndex(&why) && Has(why, "no shuttle safe-zone guard"), "[F16] W906_FP9050ShuttlesClearOfIndex with no hook: NOT clear, the reason names the missing guard");
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        iTestHeadMotorTask = 12101;
        Tick();
        CHECK(iTestHeadMotorTask == 1 && !fAllMotorHome && g_stops == 1 && Moves() == 0 && W906_ShowMyMessage_Count == 1,
              "[F16] DoTestHeadMotorFP 12101 (index-check press) with no guard: ST + exit before the descent is commanded");
        ResetFlow();
        W906_Ht9050ShuttlesClearOfIndexHook = 0;
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        SetMotPos(MInShuttle1, IN_L); SetMotPos(MOutShuttle1, OUT_R); b1ShuttleMoveToLeft = true;   // 910's own case-130 gate passes
        iTestYFrontTask = 130;
        const bool r = DoTestYFrontFP();
        CHECK(!r && !SystemStart && iTestYFrontTask == 130 && g_stops == 0 && Moves() == 0 && W906_ShowMyMessage_Count == 1,
              "[F16] DoTestYFrontFP 130 with no guard: the production press is refused, nothing commanded");
        b1ShuttleMoveToLeft = false;
        W906_Ht9050ShuttlesClearOfIndexHook = &FakeClear;
    }
    // ================= [F10] P7 =================
    std::printf("-- [F10] P7: drive faults -> ST in the same tick, no later move\n");
    {
        struct V { const char* name; int kind; };
        const V vs[] = { {"drive ALARM", 0}, {"ERROR_STOP", 1}, {"servo OFF", 2}, {"sample invalid (3 polls)", 3},
                         {"monitor frozen (2 s)", 4}, {"no health hook", 5}, {"route-latched failure", 6} };
        for (std::size_t vi = 0; vi < sizeof(vs) / sizeof(vs[0]); ++vi) {
            ResetFlow();
            g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
            g_torqueOk = false;
            iTestHeadMotorTask = 12101;
            Tick();                                                           // the press is issued (MovFlag), not yet arrived
            const int mv = Moves(), s0 = g_stops;
            switch (vs[vi].kind) {
                case 0: g_bits |= 0x2ul; break;
                case 1: g_state = 3; break;
                case 2: g_bits &= ~0x4000ul; break;
                case 3: g_sampleValid = false; break;
                case 4: break;
                case 5: W906_Pci1203IndexZHealthHook = 0; break;
                case 6: g_routeFault = true; break;
            }
            int k = 0;
            const int maxN = (vs[vi].kind == 3) ? 3 : (vs[vi].kind == 4 ? 40 : 1);
            while (fAllMotorHome && k < maxN) {
                g_now += (vs[vi].kind == 4) ? 100 : 500; ++g_passNo;
                if (vs[vi].kind != 4) Fresh();
                DoTestHeadMotorFP(); ++k;
            }
            char m[220];
            std::snprintf(m, sizeof(m), "[F10] %s mid-press (12101): ST + message once + exit (fAllMotorHome=false, Task 1) within %d tick(s)", vs[vi].name, maxN);
            CHECK(!fAllMotorHome && iTestHeadMotorTask == 1 && g_stops > s0 && W906_ShowMyMessage_Count == 1 && Has(W906_ShowMyMessage_LastS1, "drive error"), m);
            const int mvAt = Moves();
            for (int j = 0; j < 20; ++j) { g_now += 500; if (vs[vi].kind != 4) Fresh(); iTestHeadMotorTask = 12101; DoTestHeadMotorFP(); }
            std::snprintf(m, sizeof(m), "[F10] %s: no move while the fault stays (Steven 1004 07:5x), %d before", vs[vi].name, mv);
            CHECK(Moves() == mvAt, m);
            W906_Pci1203IndexZHealthHook = &FakeHealth;
        }
        //  P7 sits on all 7 switch lines
        struct S { const char* name; int which; };
        const S sm[] = { {"DoTestZContactModeStart", 0}, {"DoTestZContactModeEnd", 1}, {"DoFrontTestSuckICFP", 2}, {"DoFrontTestDestroyICFP", 3},
                         {"DoTestYFrontFP", 4}, {"DoTestYFinePitch", 5}, {"DoTestHeadMotorFP", 6} };
        int okN = 0;
        for (std::size_t i = 0; i < sizeof(sm) / sizeof(sm[0]); ++i) {
            ResetFlow();
            g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
            g_bits |= 0x2ul;
            const int mv = Moves();
            bool r = true;
            switch (sm[i].which) {
                case 0: iInitContactModeStartTask = 100; r = DoTestZContactModeStart(DirectContactMode); break;
                case 1: iInitContactModeEndTask = 100; r = DoTestZContactModeEnd(DirectContactMode, Z_SAFE); break;
                case 2: iFrontTestSuckICTask = 301; FLCarryKit.SetItemData(0, 0, HAS_IC); r = DoFrontTestSuckICFP(); break;
                case 3: iFrontTestDestroyICTask = 500; r = DoFrontTestDestroyICFP(); break;
                case 4: iTestYFrontTask = 109; r = DoTestYFrontFP(); break;
                case 5: iTestYFinePitchTask = 25; DoTestYFinePitch(); r = false; break;
                case 6: iTestHeadMotorTask = 9; DoTestHeadMotorFP(); r = false; break;
            }
            if (!r && !fAllMotorHome && g_stops == 1 && Moves() == mv) ++okN;
            else std::printf("    %s: r %d home %d stops %d moves %d/%d\n", sm[i].name, (int)r, (int)fAllMotorHome, g_stops, Moves(), mv);
        }
        CHECK(okN == 7, "[F10] P7 on the switch line of all 7 state machines: ALM -> ST + exit before their case body moves Z1");
        //  Steven 1005 23:2x (Q-R5): the protections are not keyed on the card -- a Galil-class Index Z1 that is NOT a 1203 row
        //  (no route; the motor object's own lamps) gets the same P7 (class level) and W-44; E-042's 1203 part stays inert there.
        ResetFlow();
        W906_SetGaliRoute(0);
        MOT[MTestZ1].CardType = "";
        for (int l = 0; l < 10; ++l) MOT[MTestZ1].Led[l] = false;
        W906_ShowMyMessage_Reset();
        AnsiString w1;
        const bool e042Inert = !W906_IndexZDriveFault(&w1);
        const bool healthy = !W906_FP9050DriveFaultStop("t");
        MOT[MTestZ1].Led[iAlarmLed] = true;                                   // the motor object's alarm lamp (golden ScanIndexMotorCanMove's signal)
        const bool trips = W906_FP9050DriveFaultStop("t");
        CHECK(e042Inert && healthy && trips && W906_ShowMyMessage_Count == 1 && Has(W906_ShowMyMessage_LastS1, "alarm"),
              "[F10] non-1203 Galil-class row: E-042's 1203 health is inert, the CLASS-level P7 still trips on the motor object's alarm lamp (message once)");
        MOT[MTestZ1].Motor->PServoAlarmOn = false;
        W906_FP9050Reset();
        CHECK(!W906_FP9050DriveFaultStop("t"), "[F10] golden's rule: an alarm lamp with Mot_Table ServoAlarmOn = 0 is not a fault (ScanIndexMotorCanMove)");
        MOT[MTestZ1].Motor->PServoAlarmOn = true;
        MOT[MTestZ1].Led[iAlarmLed] = false;
        g_clear = false;
        CHECK(W906_FP9050ShuttleClearStop(ht9045::fp9050::kSmHeadMotor, 12101, "t"), "[F10] non-1203 row: W-44 per tick applies too (Frank's guard says not clear -> stop)");
        MOT[MTestZ1].CardType = "PCI1203";
        W906_SetGaliRoute(&g_route);
    }
    // ================= [F14] (SHIP, live 1203 Z1) the direct form of the stuck cursor =================
    std::printf("-- [F14] SHIP: iTestYFrontTask on a generic-only number -> FP never leaves it (P7 / W-44 do not act either)\n");
    {
        ResetFlow();
        g_target = Z_SAFE; Fresh(); SetZ1(Z_SAFE);
        iTestHeadMotorTask = 600; iTestYFinePitchTask = 110; iTestYFrontTask = 55;
        FLCarryKit.SetItemData(0, 0, HAS_IC);
        for (int k = 0; k < 200; ++k) Tick();
        CHECK(iTestYFrontTask == 55 && iTestHeadMotorTask == 600 && Moves() == 0 && g_stops == 0 && W906_ShowMyMessage_Count == 0 && W906_ShowErrorMessage_Count == 0 && fAllMotorHome,
              "[F14] REPRODUCES on SHIP too (pinned 910 behaviour): 200 passes at a generic-only DoTestYFront task, no move, no stop, no message -- silent stuck");
        FLCarryKit.SetItemData(0, 0, NULL_IC);
    }
    W906_SetGaliRoute(0);
#endif  // SHIP
    INIFileGeneral = g_oldIni;
    delete g_tmpIni;
    ::DeleteFileA(g_ini.c_str());
    ::RemoveDirectoryA(g_dir.c_str());
#endif  // runtime
    std::printf("[fp9050_index] %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
