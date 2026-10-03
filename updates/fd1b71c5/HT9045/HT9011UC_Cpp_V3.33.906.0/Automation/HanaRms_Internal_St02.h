// ===========================================================================
//  Automation/HanaRms_Internal_St02.h -- what HanaRms_St02.cpp shares with HanaRms_Prepare_St02.cpp.
//  AI(W906-ST02-C10) 20261002 (St02-E helper).  Not for other files (only these two and tests/test_c10_hana_rms.cpp):
//  include Automation/HanaRms_St02.h.
//
//  [W906] golden 912 automation.cpp keeps HANARMSAddLog file-static (:2523) because PrepareHANARMSConnect sits in the
//  same file; here PrepareHANARMSConnect has its own .cpp (the one member another library calls), so the helper is
//  shared -- in a NAMED namespace (global `T` in nm, cannot collide with or be shadowed by a global of the same name).
// ===========================================================================
#ifndef HanaRms_Internal_St02H
#define HanaRms_Internal_St02H

#include "vclcompat/vcl_compat.h"
#include "vclcompat/Controls.h"     // TMemo

namespace w906hanarms {

void       HANARMSAddLog(TMemo *mmo, AnsiString asTag, AnsiString asMsg);   // golden 912 automation.cpp:2522-2540 (HanaRms_St02.cpp)
AnsiString LogDir();      // [W906] 3: as9045LogPath + "\\HANARMS"  (golden literal "D:\\HT9045_Log\\HANARMS"; HanaRms_St02.cpp)
AnsiString IniPath();     // [W906] 3: AuthPath + "HANARMS.ini"     (golden literal "D:\\HT9045\\config\\HANARMS.ini"; HanaRms_St02.cpp)
bool       RealSocket();  // [W906] 1: the value W906_HanaRmsSetRealSocket stored (HanaRmsMembers_St02.cpp)

}  // namespace w906hanarms

#endif // HanaRms_Internal_St02H
