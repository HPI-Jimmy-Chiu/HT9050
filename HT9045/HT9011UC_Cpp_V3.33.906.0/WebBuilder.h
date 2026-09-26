// =============================================================================
//  WebBuilder.h -- Data.Builder.html <-> golden V912 TfBuilder (cBuilder.cpp)
//  C++ entry. Steven 20260925 (Data.Builder). Body, act list, two-phase
//  confirmation and the port guards: see WebBuilder.cpp's header.
//
//  Integrator: declared in the GLOBAL namespace. Dispatch shape, same as
//  act.sortCT.clearCount in tools/wb_serve.cpp:
//      } else if (wc.cmd == "builder.op") {
//          extern std::string W906_BuilderOp(const std::string& payloadJson, bool* ok);
//          const std::string payload = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
//          bool bdOk = false; std::string bdRes;
//          try { bdRes = W906_BuilderOp(payload, &bdOk); }
//          catch (...) { bdOk = false; bdRes = "exception in golden TfBuilder"; }
//          server.CompleteCommand((unsigned long long)wc.id, bdOk, bdRes);
// =============================================================================
#ifndef HT9045_WEBBUILDER_H
#define HT9045_WEBBUILDER_H

#include <string>

// payloadJson = {"act":"open|state|change|create|delete|dir|drive|import|export|close", ...}.
// Returns a JSON object. *ok = carried out, or stopped at golden's OK/CANCEL question
// (needConfirm); false = refused by a guard / bad payload / exception. Takes the
// FormLock itself (re-entrant), so an outer lock does not deadlock.
std::string W906_BuilderOp(const std::string& payloadJson, bool* ok);

#endif // HT9045_WEBBUILDER_H
