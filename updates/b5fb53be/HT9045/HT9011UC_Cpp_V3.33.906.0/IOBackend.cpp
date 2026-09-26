// =============================================================================
//  IOBackend.cpp  --  IO HAL concrete backend implementations
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/MyLaneIo.cpp
//    (vendor triads in IOBitOn/.cpp:143-158, IOBitOff/.cpp:210-224,
//     IOByteOut/.cpp:278-293, IOInputBit/.cpp:397-413, IOInputByte/.cpp:466-481)
//  Translation wave: W4-IO (IO HAL layer)
//  Translator: AI(W4-IO) 20260626
//
//  TSimIOBackend    -- always active (no guard).
//  TMN200Backend    -- body active only if HAVE_MN200 is defined; else {return 0;}.
//  TPci1203Backend  -- body active only if HAVE_PCI1203; else the installable route (IOBackend.h EOF), {return 0;} when none -- AI(W906-IOWEB-P17) 20260925.
//  TMnetLegacyBackend -- body active only if HAVE_MNET; else {return 0;}.
//  TRawPortBackend  -- body active only if HAVE_RAWPORT && x86; else {return 0;}.
//
//  VENDOR HEADER RULE: vendor headers are READ-ONLY and included ONLY inside
//  their respective #if HAVE_xxx guards.  Nothing outside those guards sees them.
// =============================================================================
#include "IOBackend.h"

// ---------------------------------------------------------------------------
//  TSimIOBackend
// ---------------------------------------------------------------------------
TSimIOBackend::TSimIOBackend()
{
    for(int r=0; r<IO_MAXRing; r++)
        for(int ip=0; ip<IO_MAXIP; ip++)
            for(int p=0; p<IO_MAXPort; p++)
                mBit[r][ip][p] = 0;
}

int TSimIOBackend::WriteBit(int Ring, int IP, int Port, int Bit, int Value)
{
    if(Ring<0||Ring>=IO_MAXRing||IP<0||IP>=IO_MAXIP||Port<0||Port>=IO_MAXPort)
        return 0;   // out-of-range silently ignored in sim
    if(Bit<0||Bit>7) return 0;
    if(Value)
        mBit[Ring][IP][Port] = static_cast<unsigned char>(mBit[Ring][IP][Port] | (1u << Bit));
    else
        mBit[Ring][IP][Port] = static_cast<unsigned char>(mBit[Ring][IP][Port] & ~(1u << Bit));
    return 0;
}

int TSimIOBackend::WriteByte(int Ring, int IP, int Port, unsigned int Byte)
{
    if(Ring<0||Ring>=IO_MAXRing||IP<0||IP>=IO_MAXIP||Port<0||Port>=IO_MAXPort)
        return 0;
    mBit[Ring][IP][Port] = static_cast<unsigned char>(Byte & 0xFFu);
    return 0;
}

int TSimIOBackend::ReadBit(int Ring, int IP, int Port, int Bit, unsigned char *Value)
{
    if(Value) *Value = 0;
    if(Ring<0||Ring>=IO_MAXRing||IP<0||IP>=IO_MAXIP||Port<0||Port>=IO_MAXPort)
        return 0;
    if(Bit<0||Bit>7) return 0;
    if(Value)
        *Value = static_cast<unsigned char>((mBit[Ring][IP][Port] >> Bit) & 1u);
    return 0;
}

int TSimIOBackend::ReadByte(int Ring, int IP, int Port, unsigned char *Value)
{
    if(Value) *Value = 0;
    if(Ring<0||Ring>=IO_MAXRing||IP<0||IP>=IO_MAXIP||Port<0||Port>=IO_MAXPort)
        return 0;
    if(Value)
        *Value = mBit[Ring][IP][Port];
    return 0;
}

// ---------------------------------------------------------------------------
//  TMN200Backend
//  Vendor header Motor/mn200.h included only inside the guard.
//  mn_set_port_bit / mn_set_port_byte / mn_get_port_bit / mn_get_di_byte
//  (MyLaneIo.cpp:151-158 / .cpp:218-224 / .cpp:287-289 / .cpp:405-408 / .cpp:474-476)
// ---------------------------------------------------------------------------
#if HAVE_MN200
// AI(W906-1203HAL-1) 20260820: MOTION_IO rename wrap -- this TU is the one
// place that legitimately includes BOTH vendor stacks (MN200 above,
// AdvMotApi below under HAVE_PCI1203), which is exactly where the global
// typedef collision fires. See MachineDefine.h's note; MN200 yields the name.
#define MOTION_IO  MN200_MOTION_IO
#define PMOTION_IO MN200_PMOTION_IO
#include "Motor/mn200.h"    // READ-ONLY vendor header
#undef MOTION_IO
#undef PMOTION_IO

int TMN200Backend::WriteBit(int Ring, int IP, int Port, int Bit, int Value)
{
    return mn_set_port_bit(Ring, IP, Port, Bit, Value);
}

int TMN200Backend::WriteByte(int Ring, int IP, int Port, unsigned int Byte)
{
    return mn_set_port_byte(Ring, IP, Port, static_cast<unsigned char>(Byte & 0xFFu));
}

int TMN200Backend::ReadBit(int Ring, int IP, int Port, int Bit, unsigned char *Value)
{
    if(Value) *Value = 0;
    return mn_get_port_bit(Ring, IP, Port, Bit, Value);
}

int TMN200Backend::ReadByte(int Ring, int IP, int Port, unsigned char *Value)
{
    if(Value) *Value = 0;
    return mn_get_di_byte(Ring, IP, Port, Value);
}

#else   // HAVE_MN200 not defined -- stubs compile, never linked to vendor

int TMN200Backend::WriteBit (int /*Ring*/, int /*IP*/, int /*Port*/, int /*Bit*/,  int /*Value*/) { return 0; }
int TMN200Backend::WriteByte(int /*Ring*/, int /*IP*/, int /*Port*/, unsigned int /*Byte*/)        { return 0; }
int TMN200Backend::ReadBit  (int /*Ring*/, int /*IP*/, int /*Port*/, int /*Bit*/,  unsigned char *Value) { if(Value)*Value=0; return 0; }
int TMN200Backend::ReadByte (int /*Ring*/, int /*IP*/, int /*Port*/, unsigned char *Value)         { if(Value)*Value=0; return 0; }

#endif  // HAVE_MN200

// ---------------------------------------------------------------------------
//  TPci1203Backend
//  Vendor header EtherCAT/AdvMotApi.h included only inside the guard.
//  Returns RAW vendor ret (0==SUCCESS or error code).
//  TLaneIO maps: if(ret==0) ret=1; else ret=-1; BEFORE its ret<0 test.
//  (MyLaneIo.cpp:143-149 / .cpp:210-216 / .cpp:278-284 / .cpp:397-403 / .cpp:466-472)
// ---------------------------------------------------------------------------
#if HAVE_PCI1203
//AI(W906-PCI1203-IncludeFix) 20260818: was "EtherCAT/AdvMotApi.h" (does not
//  exist -- only EtherCAT/vendor/AdvMotApi.h does; EtherCAT/vendor/ is on the
//  include path, see CMakeLists.txt ht9045_motor/ht9045_io). Fails to compile
//  as soon as HAVE_PCI1203 is turned on -- caught via a HAVE_PCI1203=1 probe
//  build 20260818. Same fix applied to EtherCAT/MyNUEC1.cpp and MyLaneIo.cpp,
//  which had the identical typo; EtherCAT/MyEtherCAT.cpp:114 already used the
//  correct bare form.
#include "EtherCAT/AdvMotCompat.h"        // AI(W906-1203HAL-1) 20260820: ADVCMNAPI shim -> vendor AdvMotApi.h

int TPci1203Backend::WriteBit(int Ring, int IP, int Port, int Bit, int Value)
{
    // uiDevhand is a global from cmydef.h (extern unsigned int uiDevhand).
    // Bit param: Acm_DaqDoSetBitEx takes Ring/IP/Port/Value; Bit is embedded in Port.
    // Faithful translation of MyLaneIo.cpp:145.
    extern unsigned int uiDevhand;
    (void)Bit;  // NOTE: BCB6 passes Bit in Port for ePCI1203 (see IOBitOn .cpp:145)
    return (int)Acm_DaqDoSetBitEx(uiDevhand, Ring, IP, Port, (DWORD)Value);
}

int TPci1203Backend::WriteByte(int Ring, int IP, int Port, unsigned int Byte)
{
    extern unsigned int uiDevhand;
    return (int)Acm_DaqDoSetByteEx(uiDevhand, Ring, IP, Port, (BYTE)Byte);
}

int TPci1203Backend::ReadBit(int Ring, int IP, int Port, int Bit, unsigned char *Value)
{
    extern unsigned int uiDevhand;
    if(Value) *Value = 0;
    (void)Bit;  // NOTE: BCB6 passes Bit inside Port for ePCI1203 (see IOInputBit .cpp:399)
    return (int)Acm_DaqDiGetBitEx(uiDevhand, Ring, IP, Port, Value);
}

int TPci1203Backend::ReadByte(int Ring, int IP, int Port, unsigned char *Value)
{
    extern unsigned int uiDevhand;
    if(Value) *Value = 0;
    return (int)Acm_DaqDiGetByteEx(uiDevhand, Ring, IP, Port, Value);
}

#else   // HAVE_PCI1203 not defined

int TPci1203Backend::WriteBit (int Ring, int IP, int Port, int Bit,  int Value) { const TPci1203IoRoute *r = Pci1203IoRoute(); return (r && r->writeBit)  ? r->writeBit(Ring, IP, Port, Bit, Value) : 0; }  // AI(W906-IOWEB-P17) 20260925: A4-7 route (IOBackend.h EOF); no route = the stub as before
int TPci1203Backend::WriteByte(int Ring, int IP, int Port, unsigned int Byte)  { const TPci1203IoRoute *r = Pci1203IoRoute(); return (r && r->writeByte) ? r->writeByte(Ring, IP, Port, Byte) : 0; }
int TPci1203Backend::ReadBit  (int Ring, int IP, int Port, int Bit,  unsigned char *Value) { if(Value)*Value=0; const TPci1203IoRoute *r = Pci1203IoRoute(); return (r && r->readBit)  ? r->readBit(Ring, IP, Port, Bit, Value) : 0; }
int TPci1203Backend::ReadByte (int Ring, int IP, int Port, unsigned char *Value)         { if(Value)*Value=0; const TPci1203IoRoute *r = Pci1203IoRoute(); return (r && r->readByte) ? r->readByte(Ring, IP, Port, Value) : 0; }

#endif  // HAVE_PCI1203

// ---------------------------------------------------------------------------
//  TMnetLegacyBackend
//  _mnet_bit_io_output / _mnet_io_output / _mnet_bit_io_input / _mnet_io_input
//  (legacy PISO, dead path -- MyLaneIo.cpp:156-158 / .cpp:223-225 /
//   .cpp:289-293 / .cpp:409-412 / .cpp:479-481)
//  NOTE: _mnet_io_input returns the byte value directly as int (not via *Value
//  out-param) -- TLaneIO IOInputByte handles the IO_CARD_TYPE==0 quirk
//  (Value=byte(ret)).  Backend ReadByte sets *Value=0 and returns raw ret so
//  TLaneIO can apply the same quirk.
// ---------------------------------------------------------------------------
#if HAVE_MNET
// NOTE: _mnet_* functions have no single canonical header in this tree.
// Include whatever the project provides; bodies are #if-gated so this never
// links when the guard is off.
extern "C" {
    int _mnet_bit_io_output(int Ring, int IP, int Port, int Bit,  int Value);
    int _mnet_io_output    (int Ring, int IP, int Port, unsigned char Byte);
    int _mnet_bit_io_input (int Ring, int IP, int Port, int Bit,  unsigned char *Value);
    int _mnet_io_input     (int Ring, int IP, int Port);
}

int TMnetLegacyBackend::WriteBit(int Ring, int IP, int Port, int Bit, int Value)
{
    return _mnet_bit_io_output(Ring, IP, Port, Bit, Value);
}

int TMnetLegacyBackend::WriteByte(int Ring, int IP, int Port, unsigned int Byte)
{
    return _mnet_io_output(Ring, IP, Port, static_cast<unsigned char>(Byte & 0xFFu));
}

int TMnetLegacyBackend::ReadBit(int Ring, int IP, int Port, int Bit, unsigned char *Value)
{
    if(Value) *Value = 0;
    return _mnet_bit_io_input(Ring, IP, Port, Bit, Value);
}

int TMnetLegacyBackend::ReadByte(int Ring, int IP, int Port, unsigned char *Value)
{
    // _mnet_io_input returns byte value as int (IO_CARD_TYPE==0 quirk handled
    // in TLaneIO::IOInputByte).  Return raw ret; TLaneIO sets Value=byte(ret).
    if(Value) *Value = 0;
    int ret = _mnet_io_input(Ring, IP, Port);
    return ret;
}

#else   // HAVE_MNET not defined

int TMnetLegacyBackend::WriteBit (int /*Ring*/, int /*IP*/, int /*Port*/, int /*Bit*/,  int /*Value*/) { return 0; }
int TMnetLegacyBackend::WriteByte(int /*Ring*/, int /*IP*/, int /*Port*/, unsigned int /*Byte*/)        { return 0; }
int TMnetLegacyBackend::ReadBit  (int /*Ring*/, int /*IP*/, int /*Port*/, int /*Bit*/,  unsigned char *Value) { if(Value)*Value=0; return 0; }
int TMnetLegacyBackend::ReadByte (int /*Ring*/, int /*IP*/, int /*Port*/, unsigned char *Value)         { if(Value)*Value=0; return 0; }

#endif  // HAVE_MNET

// ---------------------------------------------------------------------------
//  TRawPortBackend -- raw x86 outportb/inportb (dead on 64-bit / non-x86)
//  NOT used by TLaneIO itself; provided for completeness (the raw-port path
//  belongs to myio.cpp free funcs which the object layer calls directly -- W6).
// ---------------------------------------------------------------------------
#if defined(HAVE_RAWPORT) && (defined(_M_IX86) || defined(__i386__))
// outportb / inportb are compiler builtins or from <conio.h> under BCB6.
// Under MinGW x86 they are compiler intrinsics.
#include <conio.h>

int TRawPortBackend::WriteBit(int /*Ring*/, int /*IP*/, int Port, int Bit, int Value)
{
    // Raw path: read-modify-write a single bit at I/O port Port.
    // NOTE: this is the myio.cpp path for ISABase/PCI1735U, not the TLaneIO path.
    unsigned char cur = static_cast<unsigned char>(_inp(Port));
    if(Value)
        cur = static_cast<unsigned char>(cur | (1u << Bit));
    else
        cur = static_cast<unsigned char>(cur & ~(1u << Bit));
    _outp(Port, cur);
    return 0;
}

int TRawPortBackend::WriteByte(int /*Ring*/, int /*IP*/, int Port, unsigned int Byte)
{
    _outp(Port, static_cast<int>(Byte & 0xFFu));
    return 0;
}

int TRawPortBackend::ReadBit(int /*Ring*/, int /*IP*/, int Port, int Bit, unsigned char *Value)
{
    unsigned char cur = static_cast<unsigned char>(_inp(Port));
    if(Value) *Value = static_cast<unsigned char>((cur >> Bit) & 1u);
    return 0;
}

int TRawPortBackend::ReadByte(int /*Ring*/, int /*IP*/, int Port, unsigned char *Value)
{
    unsigned char cur = static_cast<unsigned char>(_inp(Port));
    if(Value) *Value = cur;
    return 0;
}

#else   // not x86 or HAVE_RAWPORT not defined

int TRawPortBackend::WriteBit (int /*Ring*/, int /*IP*/, int /*Port*/, int /*Bit*/,  int /*Value*/) { return 0; }
int TRawPortBackend::WriteByte(int /*Ring*/, int /*IP*/, int /*Port*/, unsigned int /*Byte*/)        { return 0; }
int TRawPortBackend::ReadBit  (int /*Ring*/, int /*IP*/, int /*Port*/, int /*Bit*/,  unsigned char *Value) { if(Value)*Value=0; return 0; }
int TRawPortBackend::ReadByte (int /*Ring*/, int /*IP*/, int /*Port*/, unsigned char *Value)         { if(Value)*Value=0; return 0; }

#endif  // HAVE_RAWPORT && x86

// ---------------------------------------------------------------------------
//  AI(W906-IOWEB-P17) 20260925: A4-7 -- the TPci1203Backend route (IOBackend.h
//  EOF). One pointer, set once by the process that owns the card and read on the
//  same tick thread that runs the engine, so no lock: the vendor API is called
//  from exactly one thread everywhere in this tree (Pci1203Control.h:802-803).
// ---------------------------------------------------------------------------
static const TPci1203IoRoute *s_pci1203IoRoute = 0;

void SetPci1203IoRoute(const TPci1203IoRoute *route) { s_pci1203IoRoute = route; }
const TPci1203IoRoute *Pci1203IoRoute()              { return s_pci1203IoRoute; }
