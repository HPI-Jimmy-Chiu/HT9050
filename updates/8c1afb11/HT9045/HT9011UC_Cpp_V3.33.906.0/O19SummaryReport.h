// =============================================================================
//  O19SummaryReport.h -- AI(W906-O19-C4) 20261002 (St02-E helper): card ST02-C4 (todo D-004 (1)).  St02's file.
//
//  golden 906_0625_Steven TfObserver::DoProduction_Summary_Report (cObserver.cpp:4889-5058, declared cObserver.h:552;
//  V912 cObserver.cpp:5120 is identical) as a free function on `obs` (= golden `this`), plus the three helpers the O19
//  block of RecordTimeData needs (golden cMyDB.cpp:433-476, V906 cMyDB.cpp:576-634).  Only existing TfObserver members
//  are used (strngrdMDBQuery forms/fObserver.h:572, Chart2 :594): no facade field, no claim on forms/fObserver.* or
//  cObserver.cpp.  The member TfObserver::DoProduction_Summary_Report is still the stub (cObserver.cpp:2040).
// =============================================================================
#ifndef W906_O19SUMMARYREPORT_H
#define W906_O19SUMMARYREPORT_H

#include "vclcompat/vcl_compat.h"   // AnsiString, TDateTime
#include "forms/fObserver.h"        // TfObserver + extern fObserver (golden cObserver.h); cMyDB.cpp reads fObserver->strngrdMDBQuery (golden cMyDB.cpp:452)

// golden TfObserver::DoProduction_Summary_Report(asStartData, asStartTime, asEndData, asEndTime) on obs (cObserver.cpp:4889)
void       W906O19_DoProduction_Summary_Report(TfObserver *obs, AnsiString asStartData, AnsiString asStartTime,
                                               AnsiString asEndData, AnsiString asEndTime);
// golden fConfiguration->edtN04_Host->Text: the box is set once, to gethostname(), in the TfConfiguration ctor
// (cConfiguration.cpp:121-127) and nothing else writes it
AnsiString W906O19_HostName();
// golden TDateTime::DayOfWeek() (BCB SysUtils.DayOfWeek): 1 = Sunday .. 7 = Saturday
int        W906O19_DayOfWeek(const TDateTime &dt);
// golden IniConfig.asO19_SavePath; W906_O19_ROOT (non-empty) = the ctest seam (tests/CMakeLists.txt), unset = golden
AnsiString W906O19_SaveDir();

#endif // W906_O19SUMMARYREPORT_H
