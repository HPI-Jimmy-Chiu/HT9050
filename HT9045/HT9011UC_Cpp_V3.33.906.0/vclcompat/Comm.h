// ===========================================================================
//  vclcompat/Comm.h
//  Portable shim for the SPComm VCL serial component (Spcomm::TComm).
//
//  Translator: AI(W?) 20260626
//
//  PROVENANCE (verified against golden ref):
//    * Component class: D:/HT9045/elec/Component/spcomm.hpp:154-277
//        class TComm : public Classes::TComponent  (the SPComm package, NOT a
//        custom rs232 class).  rs232.h:6 does #include "SPComm.hpp"; the data
//        module TCOM2 (rs232.h:11) hosts the IDE/.dfm-managed TComm instances
//        (Comm1-4, TempComm6, cmATC1-4, PadComm, cmVisionLight) plus a runtime
//        array TComm *CommSD[4] (rs232.h:119).
//    * 29 consumer .cpp files drive TComm via StartComm/StopComm/WriteCommData.
//
//  SCOPED API SURFACE (tallied via grep over all .cpp/.h; only what is used):
//      WriteCommData(char*, Word) -> bool   (116 calls; spcomm.hpp:242)
//      StopComm()                           ( 63 calls; spcomm.hpp:241)
//      StartComm()                          ( 37 calls; spcomm.hpp:240)
//      CommName   (AnsiString, 36 assigns;  spcomm.hpp:246, write=FCommName)
//      Parity     (TParity,    23 assigns;  spcomm.hpp:262, write=FParity)
//      ByteSize   (TByteSize,  20 assigns;  spcomm.hpp:261, write=SetByteSize)
//      StopBits   (TStopBits,  19 assigns;  spcomm.hpp:263, write=SetStopBits)
//      BaudRate   (unsigned,   18 assigns;  spcomm.hpp:247, write=SetBaudRate)
//      ParityCheck(bool,       12 assigns;  spcomm.hpp:248, write=SetParityCheck)
//      OnReceiveData (event,    3 runtime assigns; spcomm.hpp:272)
//      ReadIntervalTimeout / Outx_XonXoffFlow / Inx_XonXoffFlow (rare, plain)
//
//  Faithfully kept names so the 29 consumers translate UNCHANGED.  Since
//  vcl_compat.h neutralizes __property/__fastcall/__closure, the VCL property
//  syntax becomes plain public data members (CommName, BaudRate, ...) and the
//  setters' side effects (apply-to-open-port) are folded into StartComm/the
//  _ApplyCommState() helper -- consumers always set properties BEFORE
//  StartComm (verified: rs232.cpp/TCOM2::RS232Init, MyTempture.cpp, etc.), so
//  plain fields preserve behavior.
//
//  NOT implemented (do NOT add; never used on TComm in 906):
//      Handle, SendDataEmpty, GetModemState, OnReceiveError, OnRequestHangup,
//      OnModemStateChange, OnSendDataEmpty, Read/WriteTotalTimeout*,
//      Dtr/RtsControl, Xon/Xoff*, ReplacedChar, etc.  (Connected is NOT a TComm
//      member at all -- grep hits are TClientSocket->Socket->Connected.)
//
//  BACKING: Win32 \\.\COMx (CreateFileA/SetCommState/SetCommTimeouts/ReadFile/
//  WriteFile + a reader thread that posts received bytes to the OnReceiveData
//  closure).  SIM/offline mode (default when the port cannot be opened, or
//  forced via SetSimMode(true)) needs no hardware: WriteCommData buffers into
//  an internal tx capture, and a test hook SimInjectReceive(...) fires
//  OnReceiveData -- so the shim compiles/links/runs with zero hardware.
//
//  Lifecycle: 906 never does `new TComm` at runtime (grep clean); instances are
//  IDE/.dfm members constructed via the VCL component ctor TComm(TComponent*).
//  This shim therefore derives from vclcompat::TComponent and provides a
//  virtual ctor taking an owner so .dfm-equivalent member declarations compile.
//
//  Include via the vcl_compat.h umbrella (added there).
// ===========================================================================
#ifndef VCLCOMPAT_COMM_H
#define VCLCOMPAT_COMM_H

#include "vclcompat/AnsiString.h"
#include "vclcompat/TStringList.h"   // vclcompat::TObject
#include <string>
#include <vector>
#include <functional>

namespace vclcompat {

// BCB6 SysUtils Word (unsigned short).  Defined in TDateTime.h too; guard to
// avoid a duplicate typedef when both headers are visible in one TU.
#ifndef VCLCOMPAT_WORD_DEFINED
#define VCLCOMPAT_WORD_DEFINED
typedef unsigned short Word;
#endif

// ---------------------------------------------------------------------------
//  TComponent -- minimal VCL Classes::TComponent stand-in.
//
//  The real SPComm TComm derives from Classes::TComponent and is constructed by
//  the IDE with an owner.  We only need a base that (a) is virtual-dtor safe and
//  (b) accepts an owner pointer in its ctor, so .dfm-equivalent declarations and
//  the runtime `TComm *CommSD[4]` array translate without edits.  Derives from
//  the existing vclcompat::TObject (TStringList.h) for a common root.
// ---------------------------------------------------------------------------
class TComponent : public TObject {
public:
    explicit TComponent(TComponent* AOwner) : Owner_(AOwner) {}
    virtual ~TComponent() {}

    TComponent* Owner() const { return Owner_; }

private:
    TComponent* Owner_;
};

} // namespace vclcompat

// ---------------------------------------------------------------------------
//  namespace Spcomm -- faithful to the original SPComm.hpp namespace so the
//  consumers' `using namespace Spcomm;` (and unqualified TComm/TParity/...) keep
//  working after translation.
// ---------------------------------------------------------------------------
namespace Spcomm {

using vclcompat::AnsiString;
using vclcompat::TObject;
using vclcompat::TComponent;
using vclcompat::Word;

// Enums byte-identical to spcomm.hpp:30-39 (same order => same integer values).
enum TParity   { None, Odd, Even, Mark, Space };   // spcomm.hpp:30
enum TStopBits { _1, _1_5, _2 };                   // spcomm.hpp:34
enum TByteSize { _5, _6, _7, _8 };                 // spcomm.hpp:38

// Receive-data event (spcomm.hpp:88).  In BCB6 this is a __closure method
// pointer; vcl_compat.h neutralizes __closure, so we model the closure with
// std::function<> (assignable from a bound member/handler in translated code).
// Consumer handler signature (rs232.h): void(TObject* Sender, void* Buffer,
// Word BufferLength)  -- consumers spell the 2nd/3rd params `Pointer Buffer,
// WORD BufferLength`, which are the same types.
typedef std::function<void(TObject* Sender, void* Buffer, Word BufferLength)>
        TReceiveDataEvent;

// ---------------------------------------------------------------------------
//  TComm
// ---------------------------------------------------------------------------
class TComm : public TComponent {
public:
    // VCL virtual component ctor (spcomm.hpp:238).  Owner may be 0.
    explicit TComm(TComponent* AOwner);
    virtual ~TComm();

    // ---- public methods (spcomm.hpp:240-242) ------------------------------
    void StartComm();                                  // open the port
    void StopComm();                                   // close the port
    // Faithful sig (spcomm.hpp:242): bool WriteCommData(char*, Word).
    bool WriteCommData(char* pDataToWrite, Word dwSizeofDataToWrite);

    // ---- properties surfaced as plain members (see header note) -----------
    // Set BEFORE StartComm by every consumer; StartComm applies them.
    AnsiString CommName;            // spcomm.hpp:246  ("\\\\.\\COMx")
    unsigned   BaudRate;            // spcomm.hpp:247
    bool       ParityCheck;         // spcomm.hpp:248
    bool       Outx_XonXoffFlow;    // spcomm.hpp:254
    bool       Inx_XonXoffFlow;     // spcomm.hpp:255
    TByteSize  ByteSize;            // spcomm.hpp:261
    TParity    Parity;              // spcomm.hpp:262
    TStopBits  StopBits;            // spcomm.hpp:263
    unsigned   ReadIntervalTimeout; // spcomm.hpp:267

    // ---- event ------------------------------------------------------------
    TReceiveDataEvent OnReceiveData;  // spcomm.hpp:272

    // -----------------------------------------------------------------------
    //  SIM / test surface (NOT part of the BCB6 API; additive).
    // -----------------------------------------------------------------------
    // Force simulation mode regardless of CommName.  When sim is active,
    // StartComm succeeds without touching hardware, WriteCommData buffers into
    // the tx capture, and SimInjectReceive() drives OnReceiveData.
    void SetSimMode(bool bSim);
    bool IsSimMode() const;

    // True between a successful StartComm and the next StopComm.
    bool IsOpen() const;

    // Inspect the bytes WriteCommData captured in sim mode.
    const std::vector<char>& SimTxBuffer() const;
    void                     SimClearTx();

    // Push RX bytes as if they arrived on the wire; fires OnReceiveData with
    // (this, copy-of-buffer, len) exactly like the real reader thread.  No-op
    // if OnReceiveData is unset.
    void SimInjectReceive(const void* pData, Word len);

private:
    // Non-copyable (owns an OS handle / reader thread when on real hardware).
    TComm(const TComm&);
    TComm& operator=(const TComm&);

    // Apply CommName/BaudRate/Parity/ByteSize/StopBits/timeouts to the open
    // handle (real mode only).  Mirrors SPComm's _SetCommState/_SetCommTimeout.
    void ApplyCommState_();

    struct Impl;   // hides the Win32 handle + reader-thread state from headers
    Impl* pImpl_;  // owned; RAII-released in the dtor / StopComm

    // Real-mode reader-thread entry (Win32 LPTHREAD_START_ROUTINE-compatible).
    // Declared here (not a free function) so it can touch the private Impl.
    // void* in the signature avoids leaking <windows.h> types into the header.
    static unsigned long __stdcall ReaderProc_(void* param);
    //AI(W906-TORQUE-COMM) 20260925: 寫入執行緒（SPComm 的 TWriteThread）；同上理由宣告成 private static
    static unsigned long __stdcall WriterProc_(void* param);
};

} // namespace Spcomm

// Bring the SPComm names into global scope, matching the original header's
// trailing `using namespace Spcomm;` (spcomm.hpp:295) so unqualified TComm /
// TParity / TByteSize / TStopBits / TReceiveDataEvent resolve in translated
// consumer code.  Suppressible via the same umbrella flag as the rest of the
// compat layer.
#ifndef VCLCOMPAT_NO_GLOBAL_USING
using namespace Spcomm;
#endif

#endif // VCLCOMPAT_COMM_H
