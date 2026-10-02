// ===========================================================================
//  JsonBridge/actions/LotInfoFtp.cpp -- act.lotInfoFtp.<op> dispatcher (see LotInfoFtp.h).
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).  No machine globals, no KYECFTP: a function pointer and two refusals.
// ===========================================================================
#include "JsonBridge/actions/LotInfoFtp.h"

#include <cstdio>
#include <exception>
#include <string>

#include "WebBridge/JsonWriter.h"

namespace ht9045 {
namespace sjson {

namespace {

LotInfoFtpBodyFn g_body = 0;

const char kPrefix[] = "act.lotInfoFtp.";

std::string Refuse(const std::string& op, const char* guard, const std::string& detail)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("op").String(op);
    w.Key("guard").String(guard);
    w.Key("detail").String(detail);
    w.EndObject();
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}

}  // namespace

void SetLotInfoFtpBody(LotInfoFtpBodyFn fn) { g_body = fn; }
bool LotInfoFtpBodyInstalled() { return g_body != 0; }

std::string W906_LotInfoFtpAct(const std::string& cmd, const std::string& payloadJson)
{
    const std::size_t n = sizeof(kPrefix) - 1;
    if (cmd.compare(0, n, kPrefix) != 0 || cmd.size() == n)
        return Refuse(std::string(), "bad-op", "the command must be act.lotInfoFtp.<op> (open, state, filter, pick, loadHD, enterHD, "
                                               "testerType, testerId, inputMethod, testerName, saveTester, exit, memoClear, close)");
    const std::string op = cmd.substr(n);
    if (g_body == 0)
        return Refuse(op, "not-installed",
                      "W906_St02_InstallLotInfoFtp() was not called (wb_serve boot, claim LI-9 F1) -- the golden TfFTPClient is "
                      "not linked into this program");
    try {
        return g_body(op, payloadJson);
    } catch (const std::exception& e) {
        return Refuse(op, "exception", std::string("exception in golden TfFTPClient: ") + e.what());
    } catch (...) {
        return Refuse(op, "exception", "exception in golden TfFTPClient (non-std)");
    }
}

}  // namespace sjson
}  // namespace ht9045
