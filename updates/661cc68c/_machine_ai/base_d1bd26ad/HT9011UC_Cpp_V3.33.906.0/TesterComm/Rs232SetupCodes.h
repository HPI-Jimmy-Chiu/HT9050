// ===========================================================================
//  TesterComm/Rs232SetupCodes.h -- recipe RS232 radio index -> RS232Standard Setup.ini [COMPort] value.
//  AI(W906-GB-P6) 20260926.  暫照建議，待使用者確認 (user ruling 5B; decision #5 Q3 pending) -- header-only, no deps.
//
//  Recipe Tester.Data [RS-232C] stores the radio ItemIndex (TestIF_File.Rs232_Data, cprod.h).  Golden 912
//  cTesterIF.cpp:517-552 CheckRs232StandardIni maps it to the SPComm ordinals RS232Standard casts straight to its enums
//  (TesterComm/Rs232/Rs232Log.cpp LoadSetupData):
//      ByteSize  0:_5 1:_6 2:_7 3:_8       StopBits 0:_1 1:_1_5 2:_2       Parity 0:None 1:Odd 2:Even 3:Mark 4:Space
//  Golden recipe indices (kept):  Bit Length 0=7 1=8 | Stop Bit 0=1 1=2 | Parity 0=Even 1=Odd 2=None.
//  APPENDED by ruling 5B (old recipes keep their meaning): Bit Length 2=5 3=6 | Stop Bit 2=1.5 | Parity 3=Mark 4=Space.
//  Out-of-range values fall into golden's else branch (8 bits / 2 stop / None), exactly as golden.
// ===========================================================================
#ifndef HT9045_TESTERCOMM_RS232SETUPCODES_H
#define HT9045_TESTERCOMM_RS232SETUPCODES_H

inline int W906_Rs232ByteSizeCode(int iBitLength)
{
    switch (iBitLength)
    {
        case 0: return 2;   // 7 bits (golden)
        case 2: return 0;   // 5 bits (5B)
        case 3: return 1;   // 6 bits (5B)
        default: return 3;  // 1 = 8 bits; golden else
    }
}

inline int W906_Rs232StopBitsCode(int iStopBit)
{
    switch (iStopBit)
    {
        case 0: return 0;   // 1 bit (golden)
        case 2: return 1;   // 1.5 bits (5B)
        default: return 2;  // 1 = 2 bits; golden else
    }
}

inline int W906_Rs232ParityCode(int iParity)
{
    switch (iParity)
    {
        case 0: return 2;   // Even (golden)
        case 1: return 1;   // Odd (golden)
        case 3: return 3;   // Mark (5B)
        case 4: return 4;   // Space (5B)
        default: return 0;  // 2 = None; golden else
    }
}

// Windows' SetCommState rejects 5 data bits with 2 stop bits, and 6/7/8 data bits with 1.5 stop bits (DCB rules);
// SPComm and vclcompat ignore that failure, so the port would silently keep its old framing.  Used by the page's save
// pre-check (decision #5 Q3(1), 暫照建議 A: refuse).  Arguments are recipe indices.
inline bool W906_Rs232FramingValid(int iBitLength, int iStopBit)
{
    const int bs = W906_Rs232ByteSizeCode(iBitLength);   // 0:_5 1:_6 2:_7 3:_8
    const int sb = W906_Rs232StopBitsCode(iStopBit);     // 0:_1 1:_1_5 2:_2
    if (bs == 0 && sb == 2) return false;                // 5 bits + 2 stop
    if (bs != 0 && sb == 1) return false;                // 6/7/8 bits + 1.5 stop
    return true;
}

#endif
