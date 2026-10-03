// ===========================================================================
//  HeaterSimTick.cpp -- golden THeaterThread::HeaterThreadProcess for the SIM build (work card S-12).
//
//  AI(W906-S12) 20260929 (St02-E).  RULINGS_20260929 item 10 (Jimmy 16:1x: A).
//  Golden 906_0625_Steven uHeaterThread.cpp:
//    :56-66  HeaterThreadProcess: if(InitialOK) { CheckATC6System(); DoThermo(); HeaterDoorIsOpen(); CheckHeater();
//            DoHeaterOn(); }
//    :68-76  Execute: Synchronize(HeaterThreadProcess) every 20 ms, so on the main thread.  Created in TfMain::SetInitialData
//            (main.cpp:22559), started in FormShow (:10138).
//  The port never creates that thread (uHeaterThread.cpp:403-444), so fHeaterOK stayed false and the Hot-mode InArm case 50
//  (ainarm9045_2x2_4.cpp:1867-1872) went back to case 1.  This runs the same body once per wb_serve tick instead
//  (WebBridgeTags.cpp PumpTick, before MainProc -- the laptop's same-line claim S-12 3.1; the native keepalive calls
//  PumpTick too).  500 ms against golden's 20 ms makes no difference to the SIM branches below.  AI(W906-FASTCLK) 20261003: with MachineType.h W906_FASTCLK_HEATER (the default) the caller is the serve loop's 20 ms fast clock instead (FastClockJobs.cpp W906_FastClockHeaterBeat) and PumpTick skips it (RULINGS_20261002 #7); same line
//
//  SIM only.  Golden's own SIM branch of CheckHeater (:138-144; port uHeaterThread.cpp:505-511) ignores temperatures:
//  fHeaterOK / fHeaterStableOK / iHeaterWait = fMain->chkHeaterOk->Checked (checked by default, forms/fMain.cpp:63).
//    CheckATC6System   run  (golden order; its body is #if 0 today, csystem.cpp:19196)
//    DoThermo          NOT run: not needed for fHeaterOK in SIM, and no temperature controller is reachable (TC401 / KT4H /
//                      E5DC gated in bthermo.cpp / cpublic.cpp, DTK's g_pDTKComm null) -- it would only push UN150Read to
//                      999 after comm timeouts.  Its SHIP timing is left to Jimmy / Steven / EastSun (the ruling).
//    HeaterDoorIsOpen  run  (SIM branch: the four door flags false, csystem.cpp:14238)
//    CheckHeater       run  (the fix)
//    DoHeaterOn        run  (SIM: simulated outputs only -- the relay stays off, bHeatOverTenErrorOK is set only by the
//                      non-SIM CheckHeater; the heater fan on; one deduplicated HeaterLog line; clears fHeaterOK only
//                      when an EMG input reads pressed, as golden)
//  SHIP build: an empty function (DoThermo would really drive the controllers; not touched here).
//  An exception never escapes into PumpTick (MainProc must still run): counted, printed once.
// ===========================================================================
#include "MachineType.h"      // SOFT_SIMULTE -- before the #ifdef, or the body would compile empty in SIM too
#include "cmydef.h"           // InitialOK
#include "csystem.h"          // CheckATC6System / HeaterDoorIsOpen / DoHeaterOn
#include "uHeaterThread.h"    // CheckHeater

#include <cstdio>

namespace ht9045 {

static unsigned long g_heaterSimTickExceptions = 0;

unsigned long W906_HeaterSimTickExceptions() { return g_heaterSimTickExceptions; }

void W906_HeaterSimTick()
{
#ifdef SOFT_SIMULTE
    if(InitialOK)                                                               // golden uHeaterThread.cpp:58
    {
        try {
            ::CheckATC6System();                                                // :60
            // ::DoThermo();                                                    // :61 -- not in SIM (see the file head)
            ::HeaterDoorIsOpen();                                               // :62
            ::CheckHeater();                                                    // :63
            ::DoHeaterOn();                                                     // :64
        } catch (...) {
            if (g_heaterSimTickExceptions++ == 0)
                std::printf("[S-12] HeaterSimTick: exception in golden HeaterThreadProcess (counted; MainProc still runs)\n");
        }
    }
#endif
}

}  // namespace ht9045
