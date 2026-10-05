// =============================================================================
//  tests/test_st02_process_for_esc_nozzle.cpp -- AI(W906-ST02-ESC) 20261005 (St02-E): the FULL-TMyKitSuck half of St02_ProcessForESC.
//  TMySucker::iNozzleEvent (what CheckDestoryFinish reads) is only in mykitsuck.h, and the main test TU sees the minimal mirror
//  (aHotPlateSubstrate.h, like HandlerGpibMsg.cpp) -- never both in one TU (docs/KNOWLEDGE.md two-TMyKitSuck), the same split as
//  TesterComm/Handler/HandlerEscSuck.cpp.  Site [0][0] of InArm / OutArm / FTest / BTest only.
// =============================================================================
#include "mykitsuck.h"

static TMyKitSuck* EscSuck(int which)
{
    TMyKitSuck* const k[4] = { &InArmSuck, &OutArmSuck, &FTestSuck, &BTestSuck };
    return (which >= 0 && which < 4) ? k[which] : 0;
}

// Site [0][0] of suck `which` (0 InArm, 1 OutArm, 2 FTest, 3 BTest): Item and the nozzle event (2 = Destroy).
void EscTest_Nozzle(int which, int item, int ev)
{
    TMyKitSuck* s = EscSuck(which);
    if (!s) return;
    if (s->iMaxRow < 1) s->iMaxRow = 1;
    if (s->iMaxCol < 1) s->iMaxCol = 1;
    s->Item[0][0] = item;
    s->Suck[0][0].iNozzleEvent = ev;
}

int EscTest_NozzleEvent(int which)
{
    TMyKitSuck* s = EscSuck(which);
    return s ? s->Suck[0][0].iNozzleEvent : -1;
}

void EscTest_NozzleClear()
{
    for (int k = 0; k < 4; ++k)
        EscSuck(k)->Suck[0][0].iNozzleEvent = 0;
}
