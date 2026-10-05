// =============================================================================
//  TesterComm/Handler/HandlerEscSuck.cpp -- AI(W906-ST02-ESC) 20261005 (St02-E): census 129 E-T1-007.  The four CheckDestoryFinish()
//  conjuncts of golden TfMain::ProcessForESC (906 0618 main.cpp:7622-7625) for THandlerTesterSide::ProcessForESC
//  (HandlerGpibMsg.cpp).  Its own TU: CheckDestoryFinish is a member of the FULL TMyKitSuck (mykitsuck.h:440,
//  mykitsuck.cpp:2885), while HandlerGpibMsg.cpp sees the minimal mirror (aHotPlateSubstrate.h:371) -- never both in one TU
//  (docs/KNOWLEDGE.md: the two-TMyKitSuck ODR trap).  Golden order, && short-circuit kept: CheckDestoryFinish clears an empty
//  site's iNozzleEvent (golden MyKitSuck.cpp:2870-2871), so a later suck is looked at only when golden would.
// =============================================================================
#include "mykitsuck.h"

bool W906_EscDestroysFinished()
{
    return InArmSuck.CheckDestoryFinish()==true &&                               // golden :7622
           OutArmSuck.CheckDestoryFinish()==true &&                              // golden :7623
           FTestSuck.CheckDestoryFinish()==true &&                               // golden :7624
           BTestSuck.CheckDestoryFinish()==true;                                 // golden :7625
}
