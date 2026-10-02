// =============================================================================
//  JsonBridge/IoFormShow.cpp -- AI(W906-IO-FORMSHOW-OUT) 20261002: the IO window's golden open steps that touch the machine.
//  (Own file so tests/test_io_formshow.cpp can link it without the IO click layer.)
// =============================================================================
#include "MachineType.h"          // first (tools/macro_order_gate.ps1)
#include "vclcompat/vcl_compat.h"
#include <cstdio>
#include <string>
#include "cmydef.h"              // SystemStart, SwFMotorBreaker / SwBMotorBreaker, MTestZ1/Z2/Y1/Y2
#include "myswitch.h"            // SW[]
#include "Motor/mymotor.h"       // MOT[]

// =============================================================================
//  AI(W906-IO-FORMSHOW-OUT) 20261002: the IO window's golden open steps that touch the machine, run by C++ when the window opens.
//
//  EastSun 20261002 (asked about golden iosetview.cpp:306-307): 「這應該是搬到c++軟體開啟 io off」, and「剩下要決定的 都照bcb怎做你就怎做」.
//  FileRW/IoSetViewFormShow_File.cpp (the GET /api/form bridge) leaves these out on purpose -- a GET also runs when the IO iframe
//  loads hidden at boot, and the web IO window can open while the machine runs, which golden's cannot. So C++ watches the window
//  registry instead (ui.windows.put, WebWindowRegistry.cpp) and runs them on the window's closed -> open edge:
//    golden TfMain::sbIOClick main.cpp:27943-27957 (the IO button):
//      :27946-27947 if(SystemStart) return;   -> the IO form never opens while running: nothing is done (logged)
//      :27954 NewRecordProcess("MES2190", "Enter IO");
//    golden Tfiosetview::FormShow iosetview.cpp:236 ->
//      :306-307 SW[SwFMotorBreaker].Off(); SW[SwBMotorBreaker].Off();   //Kenhsieh 20210923: 防止Index下墜並記錄 (Off = the brake holds)
//      :309-329 the four Index axes' command / encoder positions, NewRecordProcess("", strPos, "") twice
//  Only a FRESH report counts (a stale / reconnecting report keeps the last state), so a WS reconnect with the window open is not a
//  second open (it would re-engage brakes the operator released from the IO page).  minimized -> open is not an open (golden: modal).
//  Not done here (listed, not silently dropped): :27952 TTLLog, :27953 ProceeToolBar, :27955 SetWorkParameter,
//  :27956 MyLaneIO.BackUpOutputData + the close-time restore question (review RG-4 above: the restore cannot reach a 1203 point),
//  :27960-27980 the C_TurnTrayArm lock sequence (only when that cylinder is enabled).
// =============================================================================
#include "WebWindowRegistry.h"
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);   // cMyDB.h:129 (body cMyDB.cpp, golden :1545-1562)

bool W906_IoFormShowOutputs(std::string* log)
{
    if (SystemStart) {                                                           // golden main.cpp:27946-27947
        if (log) *log = "IO window opened while SystemStart: golden sbIOClick returns (main.cpp:27946) -- brakes not touched";
        return false;
    }
    NewRecordProcess("MES2190", "Enter IO", " ");                                // golden main.cpp:27954 (Debug default " ")
    SW[SwFMotorBreaker].Off();                                                   // golden iosetview.cpp:306
    SW[SwBMotorBreaker].Off();                                                   // golden iosetview.cpp:307
    const long z1 = MOT[MTestZ1].Gali_ReadPos(), z2 = MOT[MTestZ2].Gali_ReadPos();                     // :313-314
    const long y1 = MOT[MTestY1].Gali_ReadPos(), y2 = MOT[MTestY2].Gali_ReadPos();                     // :315-316
    const long ez1 = MOT[MTestZ1].Gali_ReadEncoderPos(), ez2 = MOT[MTestZ2].Gali_ReadEncoderPos();     // :317-318
    const long ey1 = MOT[MTestY1].Gali_ReadEncoderPos(), ey2 = MOT[MTestY2].Gali_ReadEncoderPos();     // :319-320
    char strPos[160], strEncoderPos[200];
    std::snprintf(strPos, sizeof strPos, "TestZ1: %d, iTestZ2: %d, iTestY1: %d, iTestY2: %d", (int)z1, (int)z2, (int)y1, (int)y2);   // :325
    std::snprintf(strEncoderPos, sizeof strEncoderPos, "TestZ1(Encoder): %d, iTestZ2(Encoder): %d, iTestY1(Encoder): %d, iTestY2(Encoder): %d",
                  (int)ez1, (int)ez2, (int)ey1, (int)ey2);                       // :326
    NewRecordProcess("", strPos, "");                                            // :328
    NewRecordProcess("", strEncoderPos, "");                                     // :329
    if (log) *log = std::string("Enter IO; SwFMotorBreaker / SwBMotorBreaker Off; ") + strPos + "; " + strEncoderPos;
    return true;
}

void W906_IoFormShowTick()
{
    static bool s_wasOpen = false;
    const ht9045::WinQuery q = ht9045::WebWindowRegistryQuery("fiosetview");
    if (!q.fromAnyConn || q.stale) return;                                       // no fresh report: keep the last state
    const bool openNow = (q.state == ht9045::kWinOpen || q.state == ht9045::kWinMinimized);
    if (openNow && !s_wasOpen) {
        std::string lg;
        const bool done = W906_IoFormShowOutputs(&lg);
        std::printf("io.formShow: IO window opened -> %s%s\n", done ? "" : "(skipped) ", lg.c_str());
    }
    s_wasOpen = openNow;
}
