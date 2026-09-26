// ===========================================================================
//  vclcompat/Comm.cpp
//  Implementation of the Spcomm::TComm shim (see Comm.h).
//
//  Translator: AI(W?) 20260626
//
//  Backend:
//    * REAL mode  -- Win32 \\.\COMx via CreateFileA/SetCommState/
//      SetCommTimeouts/WriteFile, plus a background reader thread that blocks in
//      ReadFile and posts received bytes to OnReceiveData (mirrors SPComm's
//      TReadThread -> ReceiveData -> OnReceiveData path, spcomm.hpp:96-125,229).
//    * SIM mode   -- no hardware: StartComm succeeds, WriteCommData buffers into
//      an internal capture, SimInjectReceive() fires OnReceiveData.  SIM is the
//      automatic fallback when the port cannot be opened (so an offline build
//      runs), or can be forced via SetSimMode(true).
//
//  The reader thread uses the Win32 CreateThread API directly (MinGW.org g++
//  6.3 ships the Win32 thread model and does NOT provide a usable std::thread;
//  this also matches SPComm's own Win32 TReadThread).  The OS handle and thread
//  live in the pimpl so <windows.h> stays out of Comm.h.
// ===========================================================================
#include "vclcompat/Comm.h"

#include <cstring>

#if defined(_WIN32)
#  include <windows.h>
#endif

namespace Spcomm {

// ---------------------------------------------------------------------------
//  Pimpl: Win32 handle + reader-thread state + sim capture.
// ---------------------------------------------------------------------------
struct TComm::Impl {
    bool              bSim;        // sim mode active (forced or fallback)
    bool              bSimForced;  // SetSimMode(true) requested explicitly
    bool              bOpen;       // between StartComm and StopComm

#if defined(_WIN32)
    HANDLE            hFile;       // real serial handle (INVALID when not open)
    HANDLE            hReader;     // reader thread handle (NULL when none)
    volatile LONG     bStopReader; // signal the reader to exit (0/1)
    TComm*            self;        // back-pointer for the thread proc
#endif

    std::vector<char> simTx;       // sim-mode captured tx bytes

    Impl()
        : bSim(false)
        , bSimForced(false)
        , bOpen(false)
#if defined(_WIN32)
        , hFile(INVALID_HANDLE_VALUE)
        , hReader(0)
        , bStopReader(0)
        , self(0)
#endif
    {}
};

// Reader thread proc (private static member, so it can touch the private
// Impl): blocks in ReadFile and posts received bytes to the OnReceiveData
// closure (mirrors SPComm's TReadThread -> ReceiveData path).  __stdcall +
// unsigned long return are LPTHREAD_START_ROUTINE-compatible.
unsigned long __stdcall TComm::ReaderProc_(void* param)
{
#if defined(_WIN32)
    TComm::Impl* im = static_cast<TComm::Impl*>(param);
    char buf[1024];
    for (;;) {
        if (::InterlockedCompareExchange(&im->bStopReader, 1, 1) != 0)
            break;
        DWORD nRead = 0;
        BOOL ok = ::ReadFile(im->hFile, buf, (DWORD)sizeof(buf), &nRead, 0);
        if (::InterlockedCompareExchange(&im->bStopReader, 1, 1) != 0)
            break;
        if (ok && nRead > 0) {
            if (im->self->OnReceiveData) {
                // Post bytes to the closure (Sender, Buffer, len).
                im->self->OnReceiveData(im->self,
                                        static_cast<void*>(buf),
                                        static_cast<Word>(nRead));
            }
        } else if (!ok) {
            break;  // read failed (port lost / handle closed): stop the loop
        }
        // ok && nRead==0 (interval timeout): loop and re-check the stop flag.
    }
#else
    (void)param;
#endif
    return 0;
}

// ---------------------------------------------------------------------------
//  ctor / dtor
// ---------------------------------------------------------------------------
TComm::TComm(TComponent* AOwner)
    : TComponent(AOwner)
    , CommName()
    , BaudRate(9600)
    , ParityCheck(false)
    , Outx_XonXoffFlow(false)
    , Inx_XonXoffFlow(false)
    , ByteSize(_8)
    , Parity(None)
    , StopBits(_1)
    , ReadIntervalTimeout(0)
    , OnReceiveData()
    , pImpl_(new Impl())
{
}

TComm::~TComm()
{
    StopComm();          // releases handle / joins reader if still open
    delete pImpl_;
    pImpl_ = 0;
}

// ---------------------------------------------------------------------------
//  SIM control / inspection
// ---------------------------------------------------------------------------
void TComm::SetSimMode(bool bSim)
{
    pImpl_->bSimForced = bSim;
    if (!pImpl_->bOpen)
        pImpl_->bSim = bSim;   // takes effect at next StartComm
}

bool TComm::IsSimMode() const { return pImpl_->bSim; }
bool TComm::IsOpen()   const { return pImpl_->bOpen; }

const std::vector<char>& TComm::SimTxBuffer() const { return pImpl_->simTx; }
void TComm::SimClearTx() { pImpl_->simTx.clear(); }

void TComm::SimInjectReceive(const void* pData, Word len)
{
    // Fire OnReceiveData exactly as the real reader thread would: a private
    // copy of the bytes + the length, with this component as Sender.
    if (!OnReceiveData)
        return;
    std::vector<char> buf;
    if (pData != 0 && len > 0)
        buf.assign(static_cast<const char*>(pData),
                   static_cast<const char*>(pData) + len);
    else
        buf.resize(0);
    void* p = buf.empty() ? static_cast<void*>(0) : static_cast<void*>(&buf[0]);
    OnReceiveData(this, p, len);
}

// ---------------------------------------------------------------------------
//  ApplyCommState_  (real mode only) -- map the plain-member settings onto a
//  DCB + COMMTIMEOUTS, mirroring SPComm's _SetCommState/_SetCommTimeout.
// ---------------------------------------------------------------------------
void TComm::ApplyCommState_()
{
#if defined(_WIN32)
    if (pImpl_->hFile == INVALID_HANDLE_VALUE)
        return;

    DCB dcb;
    ::memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (!::GetCommState(pImpl_->hFile, &dcb))
        return;

    dcb.BaudRate = BaudRate;

    // ByteSize enum (_5.._8) maps to 5..8 data bits.
    switch (ByteSize) {
        case _5: dcb.ByteSize = 5; break;
        case _6: dcb.ByteSize = 6; break;
        case _7: dcb.ByteSize = 7; break;
        case _8: default: dcb.ByteSize = 8; break;
    }

    // Parity enum -> Win32 *PARITY constants.
    switch (Parity) {
        case Odd:   dcb.Parity = ODDPARITY;   break;
        case Even:  dcb.Parity = EVENPARITY;  break;
        case Mark:  dcb.Parity = MARKPARITY;  break;
        case Space: dcb.Parity = SPACEPARITY; break;
        case None:  default: dcb.Parity = NOPARITY; break;
    }
    dcb.fParity = ParityCheck ? TRUE : FALSE;

    // StopBits enum (_1,_1_5,_2) -> Win32 stop-bit constants.
    switch (StopBits) {
        case _1_5: dcb.StopBits = ONE5STOPBITS; break;
        case _2:   dcb.StopBits = TWOSTOPBITS;  break;
        case _1:   default: dcb.StopBits = ONESTOPBIT; break;
    }

    // Software (XON/XOFF) flow control.
    dcb.fOutX = Outx_XonXoffFlow ? TRUE : FALSE;
    dcb.fInX  = Inx_XonXoffFlow  ? TRUE : FALSE;
    dcb.fBinary = TRUE;

    ::SetCommState(pImpl_->hFile, &dcb);

    COMMTIMEOUTS to;
    ::memset(&to, 0, sizeof(to));
    // ReadIntervalTimeout drives a per-char interval; the rest left at 0 so a
    // blocking ReadFile returns as soon as any byte (or the interval gap)
    // arrives -- the behavior the reader thread expects.
    to.ReadIntervalTimeout         = ReadIntervalTimeout ? ReadIntervalTimeout
                                                         : MAXDWORD;
    to.ReadTotalTimeoutMultiplier  = 0;
    to.ReadTotalTimeoutConstant    = 0;
    to.WriteTotalTimeoutMultiplier = 0;
    to.WriteTotalTimeoutConstant   = 0;
    ::SetCommTimeouts(pImpl_->hFile, &to);
#endif
}

// ---------------------------------------------------------------------------
//  StartComm -- open the port (real), else fall back to sim.
// ---------------------------------------------------------------------------
void TComm::StartComm()
{
    if (pImpl_->bOpen)
        return;  // idempotent: already running

    pImpl_->simTx.clear();

#if defined(_WIN32)
    if (!pImpl_->bSimForced) {
        // Try a real port.  CommName is set by consumers as "\\.\COMx".
        const char* name = CommName.c_str();
        HANDLE h = INVALID_HANDLE_VALUE;
        if (name != 0 && name[0] != '\0') {
            h = ::CreateFileA(name,
                              GENERIC_READ | GENERIC_WRITE,
                              0,           // no sharing (exclusive, as SPComm)
                              0,           // default security
                              OPEN_EXISTING,
                              0,           // synchronous I/O (blocking reader)
                              0);
        }
        if (h != INVALID_HANDLE_VALUE) {
            // Real port open.
            pImpl_->hFile = h;
            pImpl_->bSim  = false;
            pImpl_->bOpen = true;
            ApplyCommState_();

            // Launch the blocking reader thread (Win32 CreateThread).
            ::InterlockedExchange(&pImpl_->bStopReader, 0);
            pImpl_->self = this;
            DWORD tid = 0;
            pImpl_->hReader = ::CreateThread(
                0, 0,
                reinterpret_cast<LPTHREAD_START_ROUTINE>(&TComm::ReaderProc_),
                pImpl_, 0, &tid);
            return;
        }
    }
#endif

    // No hardware (or sim forced / non-Windows): succeed in sim mode.
    pImpl_->bSim  = true;
    pImpl_->bOpen = true;
}

// ---------------------------------------------------------------------------
//  StopComm -- close the port / stop the reader.  Idempotent.
// ---------------------------------------------------------------------------
void TComm::StopComm()
{
    if (!pImpl_->bOpen)
        return;

#if defined(_WIN32)
    if (!pImpl_->bSim) {
        // Signal the reader to exit.  The reader uses an interval-timeout
        // ReadFile (returns promptly with 0 bytes when idle), so it observes
        // the stop flag and returns; closing the handle also breaks any
        // in-flight ReadFile.  We avoid CancelIoEx (not available on the
        // MinGW.org default Win32 headers / pre-Vista target).
        ::InterlockedExchange(&pImpl_->bStopReader, 1);
        if (pImpl_->hReader != 0) {
            ::WaitForSingleObject(pImpl_->hReader, 2000);
            ::CloseHandle(pImpl_->hReader);
            pImpl_->hReader = 0;
        }
        if (pImpl_->hFile != INVALID_HANDLE_VALUE) {
            ::CloseHandle(pImpl_->hFile);
            pImpl_->hFile = INVALID_HANDLE_VALUE;
        }
        pImpl_->self = 0;
    }
#endif

    pImpl_->bOpen = false;
    // Restore sim flag to the explicit request for the next StartComm.
    pImpl_->bSim = pImpl_->bSimForced;
}

// ---------------------------------------------------------------------------
//  WriteCommData -- send bytes (real) / capture them (sim).
//  Faithful BCB6 sig: bool WriteCommData(char*, Word).  Returns true on
//  success (all bytes accepted), false otherwise.
// ---------------------------------------------------------------------------
bool TComm::WriteCommData(char* pDataToWrite, Word dwSizeofDataToWrite)
{
    if (!pImpl_->bOpen)
        return false;   // SPComm posts a write only when the port is started
    if (dwSizeofDataToWrite == 0)
        return true;    // nothing to send
    if (pDataToWrite == 0)
        return false;

    if (pImpl_->bSim) {
        // Capture into the tx sink so a test can inspect what was written.
        pImpl_->simTx.insert(pImpl_->simTx.end(),
                             pDataToWrite,
                             pDataToWrite + dwSizeofDataToWrite);
        return true;
    }

#if defined(_WIN32)
    DWORD nWritten = 0;
    BOOL ok = ::WriteFile(pImpl_->hFile, pDataToWrite,
                          (DWORD)dwSizeofDataToWrite, &nWritten, 0);
    return (ok && nWritten == (DWORD)dwSizeofDataToWrite);
#else
    return false;
#endif
}

} // namespace Spcomm
