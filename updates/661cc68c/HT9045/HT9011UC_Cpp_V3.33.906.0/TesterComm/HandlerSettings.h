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

// Ruling 2A + user ruling Q2 = (a) (20260926): the framing of the GPIB program's extra RS232 port (golden RS232.cpp
// TfRS232Main::CommTester) follows the recipe (TestIF_File.Rs232_Data) instead of D:\RS232Standard\System\Setup.ini
// [COMPort].  Published by the Handler before each bridge start (TesterComm/Handler/HandlerGpibAux.cpp), read by the
// engine once, in LoadSetupData (golden FormShow).  One atomic so the engine can never see half of an update:
//   bits 0-2 Parity (SPComm 0:None 1:Odd 2:Even 3:Mark 4:Space), bits 3-4 StopBits (0:_1 1:_1_5 2:_2),
//   bits 5-6 ByteSize (0:_5 1:_6 2:_7 3:_8), bits 8.. BaudRate.   -1 = not published -> golden Setup.ini values.
inline std::atomic<int>& HsGpibAuxFraming() { static std::atomic<int> v(-1); return v; }

inline int HsPackAuxFraming(int baud, int byteSize, int stopBits, int parity)
{
    if (baud <= 0 || baud > 0x7fffff || byteSize < 0 || byteSize > 3 || stopBits < 0 || stopBits > 2 ||
        parity < 0 || parity > 4)
        return -1;
    return (baud << 8) | (byteSize << 5) | (stopBits << 3) | parity;
}

inline bool HsUnpackAuxFraming(int v, int* baud, int* byteSize, int* stopBits, int* parity)
{
    if (v < 0)
        return false;
    *baud = v >> 8;
    *byteSize = (v >> 5) & 3;
    *stopBits = (v >> 3) & 3;
    *parity = v & 7;
    return true;
}

}  // namespace testercomm

#endif
