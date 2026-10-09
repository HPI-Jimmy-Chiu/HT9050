//---------------------------------------------------------------------------
//  Interface/TesterTCP_N06_St02.h -- the N06 recipe sync (golden 913 Interface/TesterTCP.cpp:1057-1191) as V906 calls it.
//  AI(W906-W195) 20261009 (St02-E).  Bodies: Interface/TesterTCP_N06_St02.cpp.  TesterTCP_CopyRecipeToTester /
//  TesterTCP_CopyRecipeFromTester themselves stay declared in Interface/TesterTCP.h (bool since this card, golden 913 .h:259-260).
//---------------------------------------------------------------------------
#ifndef TesterTCP_N06_St02H
#define TesterTCP_N06_St02H

#include "vclcompat/vcl_compat.h"   // AnsiString

// Steven 1009 (RULINGS 1006 #21, beyond golden): the recipe-change callers (WebRecipeChange.cpp ChangeSetUpFile, the TCP
// tester sync in TesterComm/Handler/HandlerBridgeCtl.cpp) call this instead of TesterTCP_CopyRecipeToTester.
//   flag [N06] bN06_CopyTesterFile off -> true, nothing done (golden);  sync OK -> true;
//   sync failed -> false, a bilingual alarm with the reason, and START refused until a successful re-change.
bool W906_N06SyncRecipeOrBlock(AnsiString FileName);

// true while the last sync failed and the flag is still on (W906_N06CheckStartAllowed reads it; the ctest reads it too).
bool W906_N06RecipeSyncBlocked();

// the START gate itself: true = go on; false = blocked, a bilingual alarm with the reason was shown (WebStart.cpp).
bool W906_N06CheckStartAllowed();

// [W906] ctest seam: when non-null, the 7z run is this call instead of CreateProcess (never set in production).
extern bool (*W906_N06ExecHook)(const char* s7z, const char* sParam, unsigned long* pdwExit);

#endif
