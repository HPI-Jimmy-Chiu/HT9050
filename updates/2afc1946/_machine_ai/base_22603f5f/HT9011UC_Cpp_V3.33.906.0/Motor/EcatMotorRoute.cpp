// =============================================================================
//  Motor/EcatMotorRoute.cpp -- the ENGINE side of a stop that bypassed the
//  engine motor route (Motor/EcatMotorRoute.h, W906_EcForeignStopResetFcmd).
//
//  AI(W906-ENG1203) 20260929: new file, ht9045_motor (CMakeLists.txt EOF,
//  target_sources). INBOX 112 review HIGH-1 (fidelity lens, 20260929):
//
//  WHY IT EXISTS
//      golden stops an engine axis in exactly one way: TMyMotor::PCIL132_StopMotor
//      (golden Motor/mymotor.cpp:1859-1874) does `fCMD=false;` and THEN
//      Motor->DecStop(). fCMD=false is what makes the engine's next
//      MotorMovePosition compare the position and re-issue the move instead of
//      taking the next READY as arrival (Motor/mymotor.cpp:5618-5745).
//      wb_serve also stops 1203 axes WITHOUT going through golden: WebMotorAccess's
//      Stop1203 on every opened axis (the alarm path W906_MotorAccessOnAlarm, which
//      also stops the ServoAlarmOn=0 axes golden's StopAllMotor deliberately leaves
//      running while SystemStart) and the pci1203 page's ax.stop. Those stops leave
//      fCMD=true, so a move stopped short would read as "arrived" after RETRY.
//      This function is the missing half: golden's bookkeeping, no card access.
//
//  THE RULE (golden PCIL132_StopMotor's own guard, nothing added):
//      MOT[i].Motor != NULL && MOT[i].Motor->Enable && Mot_Name not one of
//      MTestY1 / MTestZ1 / MTestZ2 / MTestY2   -> fCMD = false
//      ... for the rows whose motor IS the TMyEtherCatMotor at (board, port) --
//      the object's own iBoardID / iPortID, the identity the route was given at
//      Open_Axis (not HTMotor's same-name fields, which a TMyEtherCatMotor leaves
//      unset; tests/test_machine_motors.cpp:59-68).
//      The Index names keep golden's early return: on HT9050 M14 MTestZ1 is a 1203
//      axis but the engine moves it through Galil (Q4), never through this route.
//
//  No route installed (every ctest but the ones that install one, every SIM build,
//  every build without WB_ENGINE_MOTOR_1203) = returns 0 and touches nothing.
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL)
#include "Motor/mymotor.h"          // MOT[] / TMyMotor::fCMD / Mot_Name
#include "Motor/myEthercatmotor.h"  // TMyEtherCatMotor (its protected iBoardID / iPortID)
#include "Motor/EcatMotorRoute.h"   // EcatMotorRoute(), the declaration
#include "cmydef.h"                 // TOTAL_MOTOR, MTestY1 / MTestZ1 / MTestZ2 / MTestY2

namespace {
// The address a TMyEtherCatMotor was built with, read through a derived class's pointer-to-member (standard
// protected access; the same device as tests/test_machine_motors.cpp EcatPeek). Never instantiated.
struct EcatAddr : TMyEtherCatMotor {
    static short TMyEtherCatMotor::* Board() { return &EcatAddr::iBoardID; }
    static short TMyEtherCatMotor::* Port()  { return &EcatAddr::iPortID; }
};
}  // namespace

int W906_EcForeignStopResetFcmd(int board, int port)
{
    if (EcatMotorRoute() == 0) return 0;                                         // no route: nothing of the route's to correct
    int n = 0;
    for (int i = 0; i < TOTAL_MOTOR; ++i) {                                      // golden StopAllMotor's bound
        TMyMotor& M = MOT[i];
        if (M.Motor == NULL || M.Motor->Enable == false) continue;              // golden PCIL132_StopMotor :1861-1862
        if (M.Mot_Name == MTestY1 || M.Mot_Name == MTestZ1 ||
            M.Mot_Name == MTestZ2 || M.Mot_Name == MTestY2) continue;           //                         :1864-1867
        TMyEtherCatMotor* e = dynamic_cast<TMyEtherCatMotor*>(M.Motor);
        if (e == 0 || e->*EcatAddr::Board() != board || e->*EcatAddr::Port() != port) continue;
        M.fCMD = false;                                                          //                         :1868
        ++n;
    }
    return n;
}

//AI(W906-INDEXZ) 20260930: Motor/EcatMotorRoute.h -- the TMyEtherCatMotor rows at (board, port), same address read as above.
int W906_EcEngineMotorsAt(int board, int port)
{
    int n = 0;
    for (int i = 0; i < TOTAL_MOTOR; ++i) {
        TMyEtherCatMotor* e = dynamic_cast<TMyEtherCatMotor*>(MOT[i].Motor);
        if (e != 0 && e->*EcatAddr::Board() == board && e->*EcatAddr::Port() == port) ++n;
    }
    return n;
}
