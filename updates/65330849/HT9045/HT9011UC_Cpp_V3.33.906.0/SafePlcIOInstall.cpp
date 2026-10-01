//------------------------------------------------------------------------------
// AI(W906-H013) 20261001 (St02-E): golden 912 IsSafePLCIOInstall() (cmydef.h:5684-5689), ruling 11 = B
//   (docs/TESTERCOMM_PORT_LEDGER.md:194; todo H-013 item 1).  The ONE symbol of this file.
//   Caller: TfMain::MachineStatus (Command.cpp:15146, golden 912 Command.cpp:7407), which lives in ht9045_sm; this file
//   lives in ht9045_globals -- the archive that already defines Enable_PLCSafety_IO (cmydef.cpp:5792) and that every
//   machine archive links -- so any later caller links too (St02 rule: a new global symbol other libraries call gets a
//   file of its own).  Declaration: cmydef.h:5611 (golden 912 puts the function in cmydef.h as well).
//   golden 912 body:  if(IsSafePLCIOType_Schneider() || IsSafePLCIOType_ReeR()) return true;  return false;
//     = g_eSafePLCIOType (golden 912 cmydef.cpp:5627, read by database.cpp:1526 from Gerneral.ini [System] SafePlcIO)
//       is eSafePLCIOType_InstallSchneider (1) or eSafePLCIOType_InstallReeR (2).
//   The port is the 906 base: no ESafePLCIOType / g_eSafePLCIOType, only the 906 bool Enable_PLCSafety_IO, which
//   database.cpp:1651 reads from the SAME key ("System", "SafePlcIO").  For SafePlcIO 0 / 1 / 2 both answer the same;
//   only 3 or more, or a negative value, differ (912 false, the 906 bool true) -- the 906 setting offers only
//   "Un Install" / "Install" (golden 906 HandlerSys.dfm rgSafePlcIO).  The ReeR refactor (ESafePLCIOType,
//   GetPLCIOInfo, golden 912's other ~35 IsSafePLCIOInstall() call sites) is not in ruling 11.
//   Behaviour: unchanged (the 906 term answered Enable_PLCSafety_IO).  Test: tests/test_h013_terms.cpp (H013_Terms).
//------------------------------------------------------------------------------
#include "cmydef.h"

bool IsSafePLCIOInstall()
{
    return Enable_PLCSafety_IO;
}
