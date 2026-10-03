// ===========================================================================
//  Automation/HanaRmsPump_St02.cpp -- W906_HanaRmsPumpTick only.  AI(W906-ST02-C10) 20261002 (St02-E helper).
//  [W906] 1: golden's HANARMSClient was a ctNonBlocking VCL socket -- its connect / read / close came back as window
//  messages on the main thread.  Here the socket is POLLED (vclcompat/ClientSocket.cpp end of file, H-008 contract):
//  this Poll(), called on the Handler tick (TesterComm/Handler/TesterCommWiring.cpp W906_TesterCommTick and, for the
//  modal waits, W906_TesterCommPoll), finishes a pending connect, reads, notices the server's close, and fires the
//  golden HANARMSClient* events right here, on that thread.  Sim socket (the default, every ctest): Poll does nothing.
//  Called by another library (ht9045_testercomm_handler), so it has a file of its own (St02 1001 lesson).
// ===========================================================================
#include "MachineDefine.h"      // the tree's include hub
#include "Automation/HanaRms_St02.h"
#include "atester_shims.h"      // fAutomation

#include <cstddef>              // NULL

void W906_HanaRmsPumpTick()
{
    static bool s_bBusy = false;                                // a golden event that waits in a modal (none does today) must not re-enter Poll
    if(s_bBusy || fAutomation == NULL || fAutomation->HANARMSClient == NULL)
        return;                                                 // no socket yet: PrepareHANARMSConnect has not connected (non-HANA, A77 off, 127.0.0.1)
    struct Busy { Busy() { s_bBusy = true; } ~Busy() { s_bBusy = false; } } busy;
    fAutomation->HANARMSClient->Poll();
}
