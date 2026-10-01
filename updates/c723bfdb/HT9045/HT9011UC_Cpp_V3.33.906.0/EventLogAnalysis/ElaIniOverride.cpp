// ===========================================================================
//  EventLogAnalysis/ElaIniOverride.cpp -- the ELA's config.ini boolean override (ElaHub.h IniBoolOverride) and its
//  installer W906_ElaSetBoolOverride (ElaService.h).  St02's W58 code (AI(W906-SIM-W36-1) 20260928 St02-E helper,
//  W58 20260930), moved here unchanged from the end of ElaService.cpp by AI(W906-B20-ELAWININET) 20261001 (laptop, batch 20).
//
//  ⚠ Keep this file free of every other ELA symbol.  ElaHub.cpp (IniCheckAndReadBool) and ElaSchedule.cpp
//    (IniRead::Bool) call IniBoolOverride, so the object that defines it is linked into EVERY program that links
//    them.  In ElaService.cpp it dragged ElaService.o along, and ElaService.o names ela::NewWinInetFtp (W906_ElaStart)
//    -> ElaFtpWinInet.o -> WININET.DLL imported by every ELA test: ELA_Ftp's "wininet.dll is not loaded" checks
//    (tests/test_ela_ftp.cpp :211 / :913) failed in both configs (gate b20i 20261001 16:19; objdump -p and nm on
//    build_sim).  The rule is the one CMakeLists.txt states for ElaFtpWinInet.cpp: "linked only where NewWinInetFtp
//    is named".
// ===========================================================================
#include "EventLogAnalysis/ElaHub.h"       // ela::IniBoolOverrideFn / ela::IniBoolOverride
#include "EventLogAnalysis/ElaService.h"   // W906_ElaSetBoolOverride
#include <atomic>
#include <string>

// An atomic pointer: the SIM wb_serve installs it on the main thread at boot, the ELA worker reads it; nothing
//   installed = the file value.
namespace {
std::atomic<ela::IniBoolOverrideFn> g_iniBoolOverride(static_cast<ela::IniBoolOverrideFn>(0));
}  // namespace

bool ela::IniBoolOverride(const std::string& file, const std::string& group, const std::string& name, bool fileValue)
{
    const IniBoolOverrideFn fn = g_iniBoolOverride.load();
    return fn ? fn(file.c_str(), group.c_str(), name.c_str(), fileValue) : fileValue;
}

void W906_ElaSetBoolOverride(ela::IniBoolOverrideFn fn)
{
    g_iniBoolOverride.store(fn);
}
