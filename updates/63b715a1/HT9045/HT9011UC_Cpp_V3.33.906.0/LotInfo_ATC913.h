// =============================================================================
//  LotInfo_ATC913.h -- W-224 POOL-13 MR-A (St02-E): the pieces of LotInfo_ATC913.cpp that tests (and later MR-C / MR-B) need.
//
//  AI(W906-POOL13) 20261010 [W906] (St02-E).  TfLotInfo::SetATCOffset itself is declared in forms/fLotInfo.h:1690.
//  Kept free of forms/fATCHandlerSide.h on purpose: that header #defines ATC_MAX_SITE 40, which clashes with
//  TesterComm/Gpib/GpibBridge.h:64 `const int ATC_MAX_SITE = 32` in any TU that sees both.  LotInfo_ATC913.cpp static_asserts
//  W906_ATC913_REC_MAX == ATC_MAX_SITE.
// =============================================================================
#ifndef LOTINFO_ATC913_H
#define LOTINFO_ATC913_H

#include "vclcompat/HTimer.h"   // vclcompat::HTimer (never the bare name: atester_shims.h:467 has a fake global `struct HTimer`)

// golden 913 uLotInfo.cpp:5586-5587 -- file-scope globals in golden (same names, external linkage; 0 other definitions in the port).
extern vclcompat::HTimer ATCOFSDelay;            // golden 913 :5586 (read by ShowNewATCThermo, MR-B)
extern vclcompat::HTimer ATCPreOFSDelay[32];     // golden 913 :5587 //JerryYang 20260917 : 測試中變溫預先補償持續時間(依ATC硬體位置)

// golden 913 uLotInfo.cpp:9643-9694 -- file-scope function in golden (three-point package offset).
double ConvertPackageOffset();

// ---- W-224 Q3 recorder: what golden would have sent to the ATC (the send itself stays GATE(W906-POOL13-ATCSEND)) ----
const int W906_ATC913_REC_MAX = 40;              // = golden ATC_MAX_SITE (ATC_Handler_Side.h:17 / forms/fATCHandlerSide.h:684)
enum
{
    W906_ATC913_SETOFFSET            = 0,        // ATC_InterfaceForm->SetOffset(iChCount, dOffset)
    W906_ATC913_SETTC2OFFSET         = 1,        // ATC_InterfaceForm->SetTC2Offset(iChCount, dOffset)
    W906_ATC913_SETMULTISENSOROFFSET = 2,        // ATC_InterfaceForm->SetMultiSensorOffset(iChCount, dOffset)
    W906_ATC913_SET2NDFUNCTION       = 3,        // ATC_InterfaceForm->Set2ndFunction(bEnabled)
    W906_ATC913_KINDS                = 4
};
struct W906_ATC913_Recorder
{
    int    iCount[W906_ATC913_KINDS];                        // sends golden would have made, per command
    int    iChCount[W906_ATC913_KINDS];                      // iChCount of the last one
    double dLast[W906_ATC913_KINDS][W906_ATC913_REC_MAX];    // the last array (first iDataLen entries, rest 0)
    bool   bLast2nd;                                         // the last Set2ndFunction argument
    int    iGuardSite;                                       // GOLDEN BUG guard 1 hits: site skipped, j1 / j2 outside 0..31
    int    iGuardTc2;                                        // GOLDEN BUG guard 2 hits: TC2 index j3+8 / j4+8 >= 32 not read
};
extern W906_ATC913_Recorder W906_ATC913_Rec;
void W906_ATC913_ResetRecorder();
void W906_ATC913_Record(int iKind, int iChCount, const double *pData, int iDataLen);
void W906_ATC913_RecordBool(bool bEnabled);
void W906_ATC913_NoteGuard(int iGuard, int iRow, int iCol, int iIndex);

#endif // LOTINFO_ATC913_H
