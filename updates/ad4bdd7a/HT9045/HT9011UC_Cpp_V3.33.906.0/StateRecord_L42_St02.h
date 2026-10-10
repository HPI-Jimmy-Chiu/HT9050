// =============================================================================
//  StateRecord_L42_St02.h -- card L42 (ST02_V912_VS_V913 s3 L42): the golden 913 State Record diagnostics that
//  cStateRecord.cpp calls on single lines.  Bodies: StateRecord_L42_St02.cpp (St02's own TU, ht9045_sm).
//
//  AI(W906-L42) 20261010 (St02).  Included by cStateRecord.cpp :101 (a former blank line) and by the ctest
//  St02_L42StateRecord913.
// =============================================================================
#ifndef STATERECORD_L42_ST02_H
#define STATERECORD_L42_ST02_H

#include "vclcompat/vcl_compat.h"   // AnsiString, TStringList

void W906_L42_SaveMachineMaterial(AnsiString NewPath);   // golden 913 main.cpp:27706-27727 -- NewPath\MachineMaterial.txt
void W906_L42_TaskListPattern31(TStringList *sList);        // golden 913 main.cpp:6835-6863 -- SaveTaskList, Pattern #31 rows
void W906_L42_TaskListInShuttleSocket(TStringList *sList);  // golden 913 main.cpp:7079-7116 -- SaveTaskList, FLCarryKit / BLCarryKit / TestSocket
void W906_L42_TaskListATKFixFull(TStringList *sList);       // golden 913 main.cpp:7140-7166 -- SaveTaskList, ATK rows
void W906_L42_DecisionASM(TStringList *sList);              // golden 913 main.cpp:7310-7427 -- SaveDecisionVariables, section 5b ASM
void W906_L42_DecisionHangTime(TStringList *sList);         // golden 913 main.cpp:7474-7484 -- SaveDecisionVariables, hang-up watchdog rows

#endif // STATERECORD_L42_ST02_H
