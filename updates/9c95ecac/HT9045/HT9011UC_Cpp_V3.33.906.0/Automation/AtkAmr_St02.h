//---------------------------------------------------------------------------
//  Automation/AtkAmr_St02.h -- the ATK (Amkor Korea) AMR flow pieces of golden 913 that V906 did not have (RogerYang
//  AI(ht9045-atk-amr-flow) 20260820-0825; same text in golden 912, i.e. 0618 -> 912).  AI(W906-W195) 20261009 (St02-E), laptop
//  card W-195 (3) "ATK" (MR-ATK-1 + 1b).  Globals: Automation/AtkAmrGlobals_St02.cpp (ht9045_globals); helpers:
//  Automation/AtkAmr_St02.cpp (ht9045_sm).  依 W-195 卡，S25 不適用（Steven 1009 13:2x）.
//  Reached only by ATK: fAGV->IsATK_AMR() = CUSTOMER_CODE==CC_AMKOR_Korea && USE_COVER_TRAYID==tCID_NFC && [A65] BundleIDList
//  (and SECS on).  HT9050 (957, USE_COVER_TRAYID=0) never reaches it.
//---------------------------------------------------------------------------
#ifndef AtkAmr_St02H
#define AtkAmr_St02H

#include "vclcompat/vcl_compat.h"   // AnsiString

// golden 913 Automation/AGV.h:247-249 (defined AGV.cpp:2132-2134 -> Automation/AtkAmrGlobals_St02.cpp)
extern AnsiString sTrackOutType_ATK;   //AI(ht9045-atk-amr-flow) 20260820 (RogerYang) : ATK CEID8 SV38316 First/Final Track-Out
extern int        iATKPortTrayCount;   //AI(ht9045-atk-amr-flow) 20260822 (RogerYang) : ATK CEID288 該軌排出總Tray數(挪用37007); 勿覆寫LastSet.iLoaderTotalTray
extern int        iATKPortUnitCount;   //AI(ht9045-atk-amr-flow) 20260822 (RogerYang) : ATK CEID288 該軌排出總Unit數(挪用1102); 勿覆寫RunInfo.iUnloadCount

// golden 913 main.cpp:15925-15953 (TfMain::SetLotState, iState==10, inside its IsATK_AMR / SECS / LotStart guard :15922-15923):
// CEID 288 once per Auto1..Auto3, then the restore.  Caller: forms/fMain_SetLotState.cpp.
void W906_AtkFinalLotEndCeid288();

// golden 913 csystem.cpp:8133-8135 (DoTrayFeed, a lot without sorting) and :19315-19317 (MainProc, after the sorting restore):
// the one CEID 8 of an ATK lot, marked "Final Track-Out" (SV 38316), and the flag that stops TfLotInfo::SetLotEnd from sending a
// second one (golden uLotInfo.cpp:2077).  Callers: csystem.cpp.
void W906_AtkFinalTrackOutCeid8();

// [W906] ctest seam: when non-null, W906_AtkFinalLotEndCeid288 calls it right before each CEID 288 with that lane's values
// (the sim EventReport keeps only the last CEID).  Never set in production.
extern void (*W906_AtkCeid288Hook)(int iPortNo, const char* sBundleID, const char* sBinCode, int iTrays, int iUnits);

#endif
