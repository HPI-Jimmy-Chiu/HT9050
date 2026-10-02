// ===========================================================================
//  JsonBridge/actions/LotInfoFtp.h -- WS act.lotInfoFtp.<op>: the dispatcher and its install seat (card LI-9 F1).
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).
//  ChanAction.cpp (HandleActionWithTag, St02 line) sends every "act.lotInfoFtp.*" here.  This file only forwards to a body
//  that wb_serve installs at boot (WebLotInfoFtpInstall_St02.cpp -> W906_St02_LotInfoFtpRegisterBody); with nothing
//  installed the answer is executed:false / guard "not-installed".  That keeps the programs that link ChanAction.cpp
//  without the dialog (test_sjson_chan) free of KYECFTP/FTPClientForm_St02.cpp and of everything it pulls in --
//  the same reason MainClarnData has an explicit install (forms/fMain.h, the install-seat note).
// ===========================================================================
#ifndef HT9045_JSONBRIDGE_ACTIONS_LOTINFOFTP_H
#define HT9045_JSONBRIDGE_ACTIONS_LOTINFOFTP_H

#include <string>

namespace ht9045 {
namespace sjson {

// op = the part after "act.lotInfoFtp."; payloadJson = the WS value (a JSON object as a string, may be empty).
typedef std::string (*LotInfoFtpBodyFn)(const std::string& op, const std::string& payloadJson);

void SetLotInfoFtpBody(LotInfoFtpBodyFn fn);   // NULL = uninstall
bool LotInfoFtpBodyInstalled();

// cmd = "act.lotInfoFtp.<op>".  Always a JSON object with "executed" (the wb_serve ack's ok is "executed":true).
std::string W906_LotInfoFtpAct(const std::string& cmd, const std::string& payloadJson);

}  // namespace sjson
}  // namespace ht9045

#endif // HT9045_JSONBRIDGE_ACTIONS_LOTINFOFTP_H
