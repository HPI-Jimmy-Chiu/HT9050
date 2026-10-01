// ===========================================================================
//  EventLogAnalysis/ElaService.h -- the /api/ela handling behind wb_serve's W906_ElaHttp, and the wb_serve entry points
//  (ELA plan P3 / P4).  AI(W906-ELA-P3) 20260927 (St02).  Ledger docs/ELA_PORT_LEDGER.md; skill ht9045-eventlog-analyzer.
//
//  ela::ServeHttp takes the hub, so ctest ELA_Service runs it on a %TEMP% hub; W906_ElaHttp passes the one production
//  hub (ElaService.cpp).  wb_serve declares the four W906_Ela* functions locally (tools/wb_serve.cpp :439 / :4165 /
//  :5967) and does not include this header.
// ===========================================================================
#ifndef EVENTLOGANALYSIS_ELASERVICE_H
#define EVENTLOGANALYSIS_ELASERVICE_H

#include "EventLogAnalysis/ElaHub.h"

#include <string>

namespace ela {

// false = not an /api/ela path (the caller falls through).  Otherwise *status / *contentType / *body are set:
//   no hub                                                        503
//   GET|HEAD /api/ela[/][?since=<seq>]                            200 the snapshot, or {"seq","busy","unchanged":true}
//                                                                     when since == hub->Seq()
//   POST /api/ela/query?from=&to=&top=&filter=&area=&func=&row=   202 queued; 400 bad from / to; 403 when allowCmd is false
//                                                                     (never from wb_serve: its allowCmd is always true, ZEROARG)
//   GET /api/ela/schedule                                         200 {installed, build, ftp, o10, o10Source, schedule}
//                                                                     (schedule = ElaSchedule StatusJson or null; R6)
//   POST /api/ela/job?id=<job name>                               202 queued for the next tick; 400 bad id; 403 when allowCmd
//                                                                     is false; 503 no scheduler (R6 manual run)
//   POST /api/ela/summary?name=<file name part>                   202 queued (Save Summary on the worker; "" = today's
//                                                                     yyyy-mm-dd); 400 bad name; 403 when allowCmd is false
//   anything else under /api/ela                                  404
// base supplies what the page does not send: eventLogDir, prodLogDir, handlerId.
bool ServeHttp(Hub* hub, const QueryRequest& base, const std::string& method, const std::string& path,
               const std::string& query, bool allowCmd, int* status, std::string* contentType, std::string* body);

}  // namespace ela

void W906_ElaStart();
void W906_ElaStop();
void W906_ElaPost(int cmd);
bool W906_ElaHttp(const std::string& method, const std::string& path, const std::string& query, bool allowCmd,
                  int* status, std::string* contentType, std::string* body);

#endif  // EVENTLOGANALYSIS_ELASERVICE_H
