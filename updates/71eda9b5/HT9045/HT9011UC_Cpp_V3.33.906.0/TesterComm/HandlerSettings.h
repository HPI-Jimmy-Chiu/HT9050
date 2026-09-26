// ===========================================================================
//  TesterComm/HandlerSettings.h -- Handler -> engine settings snapshot (tester-comm P6).
//  AI(W906-GB-P6) 20260926.  暫照建議，待使用者確認 (decision #5) where noted.
//
//  The golden bridges were separate programs that read some settings from their own ini files.  Rulings C-2 say a few
//  of them follow the HANDLER's value instead.  No MessageDef packet field carries them (the MV / VM layout is kept
//  identical to the BCB programs), so the Handler thread publishes them here and the TesterComm thread reads them.
//  One writer (Handler thread), one reader (TesterComm thread); plain atomics, -1 = "not published" (e.g. an engine
//  run without the Handler side, as in ctest) -> the engine keeps golden behaviour.
//  Function-local statics instead of C++17 inline variables: MinGW 6.3 has no inline variables.
// ===========================================================================
#ifndef HT9045_TESTERCOMM_HANDLERSETTINGS_H
#define HT9045_TESTERCOMM_HANDLERSETTINGS_H

#include <atomic>

namespace testercomm {

// Ruling 3A, decision #5 Q1 暫照建議 (a): the Auto Retest tester brand the Handler holds now (fSCKART->iTesterType,
// 0 = Flex, 1 = 93K).  The Handler keeps golden's own learning (LOTSTATUS -> 0, SRQMASK -> 1).
inline std::atomic<int>& HsArtTesterType() { static std::atomic<int> v(-1); return v; }

// Ruling 4A (decision #5 Q4 暫照建議 A): the Handler-ID reply format, IniConfig.iI25UseGPIBFormat (config.ini).
inline std::atomic<int>& HsUseGPIBFormat() { static std::atomic<int> v(-1); return v; }

}  // namespace testercomm

#endif
