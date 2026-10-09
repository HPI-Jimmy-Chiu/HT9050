//---------------------------------------------------------------------------
//  Automation/AtkAmrGlobals_St02.cpp -- golden 913 Automation/AGV.cpp:2132-2134 (= 912), the three ATK globals.
//  AI(W906-W195) 20261009 (St02-E).  Own TU in ht9045_globals: the SECSGEM SV / EC tables (ht9045_sm), forms/fLotInfo.cpp
//  (ht9045_forms) and Automation/AtkAmr_St02.cpp read them, so they live in the lowest archive every one of those links.
//---------------------------------------------------------------------------
#include "Automation/AtkAmr_St02.h"

AnsiString sTrackOutType_ATK;                                                   //AI(ht9045-atk-amr-flow) 20260820 (RogerYang) : ATK CEID8 SV38316 First/Final Track-Out
int        iATKPortTrayCount=0;                                                 //AI(ht9045-atk-amr-flow) 20260822 (RogerYang) : ATK CEID288 該軌排出總Tray數(挪用37007)
int        iATKPortUnitCount=0;                                                 //AI(ht9045-atk-amr-flow) 20260822 (RogerYang) : ATK CEID288 該軌排出總Unit數(挪用1102)
