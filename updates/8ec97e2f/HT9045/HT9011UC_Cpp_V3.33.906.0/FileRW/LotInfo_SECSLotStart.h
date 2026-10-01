// =============================================================================
//  FileRW/LotInfo_SECSLotStart.h -- golden TfLotInfo::sbSECSLotStartClick as ONE callable (todo E-020 LI-1).
//
//  //AI(W906-E020-LI1) 20261002 [W906] (St01): new file.  Golden V912
//    D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:7501-8583 (cp950).  Body, gates and the
//    wb_serve one-line call: FileRW/LotInfo_SECSLotStart.cpp.
//
//  Standard types only, so a caller can declare it on one line instead of including this header.
// =============================================================================
#ifndef FILERW_LOTINFO_SECSLOTSTART_H
#define FILERW_LOTINFO_SECSLOTSTART_H

#include <string>
#include <vector>

// Runs golden TfLotInfo::sbSECSLotStartClick on the global fLotInfo (the caller sets edtSysLotID / edtSysOperatorID /
// cbRunMode first, as the operator would have typed them).
//   true  = golden ran to its end: SetLotStart(__FUNC__) was called.  Golden's own SetLotStart may still leave
//           RunInfo.bLotStart false (an empty Lot ID after the special-character blanking), so a caller that needs
//           "is a lot started" reads RunInfo.bLotStart, not this value.
//   false = golden returned early.  *whyNot = golden's message text ("S1 | S2" for the two-text boxes), a description
//           when golden returns without a message, or "GATE (W906-E020-LI1-n): ..." when a missing port piece stops it.
//   *skipped (optional) = one line per gated golden ACTION that golden would have run on this call (no refusal).
// Modal boxes (ShowMyMessage / ShowErrorMessage) are golden's: under wb_serve they wait for the browser's answer.
bool W906_LotInfo_SECSLotStart(std::string* whyNot, std::vector<std::string>* skipped = 0);

#endif // FILERW_LOTINFO_SECSLOTSTART_H
