// ===========================================================================
//  TesterComm/Rs232/Rs232UiSnapshot.h -- the RS232 / TTL tab of testercomm.html: snapshot of the golden
//  RS232Standard form and the page commands.  AI(W906-GB-P7) 20260926.  TesterComm thread only (Rs232Engine::RunOnce).
// ===========================================================================
#ifndef TESTERCOMM_RS232_RS232UISNAPSHOT_H
#define TESTERCOMM_RS232_RS232UISNAPSHOT_H

#include <string>

namespace rs232std {

std::string BuildUiSnapshot(bool up);
bool ApplyUiCommand(const std::string& command);

}  // namespace rs232std

#endif
