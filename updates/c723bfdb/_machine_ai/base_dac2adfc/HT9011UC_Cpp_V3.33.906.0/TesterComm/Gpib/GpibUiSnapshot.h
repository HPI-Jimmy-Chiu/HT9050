// ===========================================================================
//  TesterComm/Gpib/GpibUiSnapshot.h -- what the web page (testercomm.html, GPIB tab) sees of the bridge's form and
//  what it may do to it.  AI(W906-GB-P7) 20260926.  Runs ONLY on the TesterComm thread (GpibEngine::RunOnce), so it
//  may read / write the golden widgets exactly like the golden bodies do.  See TesterComm/UiChannel.h.
// ===========================================================================
#ifndef TESTERCOMM_GPIB_GPIBUISNAPSHOT_H
#define TESTERCOMM_GPIB_GPIBUISNAPSHOT_H

#include <string>

namespace gpibbridge {

// JSON text of the golden Main.dfm screen (LEDs, status bar, labels, options, 32 site panels, log tails, notices).
// Empty string when no bridge form exists.
std::string BuildUiSnapshot(bool up, const char* driverName);

// One page command (see the table in GpibUiSnapshot.cpp).  Sets the widget the way a user would, then calls the
// golden OnClick / OnChange handler.  Returns false for an unknown or malformed command (ignored).
bool ApplyUiCommand(const std::string& command);

}  // namespace gpibbridge

#endif
