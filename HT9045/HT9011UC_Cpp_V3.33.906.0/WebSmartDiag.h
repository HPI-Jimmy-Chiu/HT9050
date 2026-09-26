// =============================================================================
//  WebSmartDiag.h -- Data.SmartDiagnostic.html <-> golden V912 TfSmartDiagnostic
//  (SmartDiagnostic.cpp) C++ entry. Steven 20260925 (Data.SmartDiagnostic).
//  Body, act list and two-phase confirmation: see WebSmartDiag.cpp's header.
//
//  Integrator: declared in the GLOBAL namespace (a block-scope extern written inside
//  namespace ht9045::sjson would declare a different symbol). Dispatch shape, same as
//  act.sortCT.clearCount in tools/wb_serve.cpp:
//      } else if (wc.cmd == "smartdiag.op") {
//          extern std::string W906_SmartDiagOp(const std::string& payloadJson, bool* ok);
//          const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
//          bool sdOk = false; std::string sdRes;
//          try { sdRes = W906_SmartDiagOp(payload, &sdOk); }
//          catch (...) { sdOk = false; sdRes = "exception in golden TfSmartDiagnostic"; }
//          server.CompleteCommand((unsigned long long)wc.id, sdOk, sdRes);
// =============================================================================
#ifndef HT9045_WEBSMARTDIAG_H
#define HT9045_WEBSMARTDIAG_H

#include <string>

// payloadJson = {"act":"open|timer|create|reset|save|cell", ...} (see WebSmartDiag.cpp).
// Returns a JSON object. *ok = the request was carried out, or stopped at golden's
// confirmation question (needConfirm); false = refused by a guard / bad payload /
// exception (the JSON says which). Takes the FormLock itself (CRITICAL_SECTION,
// re-entrant on the same thread), so an outer lock does not deadlock.
std::string W906_SmartDiagOp(const std::string& payloadJson, bool* ok);

#endif // HT9045_WEBSMARTDIAG_H
