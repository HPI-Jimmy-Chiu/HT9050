// =============================================================================
//  IOBackend.h  --  IO HAL virtual base + concrete backend declarations
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/MyLaneIo.cpp
//    (vendor triads inside IOBitOn/IOBitOff/IOByteOut/IOInputBit/IOInputByte/
//     IOOutBitStatus)
//  Translation wave: W4-IO (IO HAL layer)
//  Translator: AI(W4-IO) 20260626
//
//  DESIGN (mirrors W4 motor HTMotor.h):
//    TIOBackend  -- abstract port.  int<0=fail contract: return >=0 means the
//                   hardware operation succeeded; return <0 means failure.
//                   Each method has a default no-op body so derived classes only
//                   override what they support.
//    TSimIOBackend -- offline in-memory backend (DEFAULT when no HAVE_xxx is
//                   defined).  Stores bits/bytes in a 3D array; writes succeed
//                   immediately; reads return the cached value.  Mirrors the
//                   BCB6 #ifdef SOFT_SIMULTE "return ok" path at runtime so the
//                   source compiles faithfully WITHOUT defining SOFT_SIMULTE.
//    TMN200Backend      -- mn_set/get_port_bit, mn_set_port_byte, mn_get_di_byte
//                          (Motor/mn200.h).  Active when HAVE_MN200 is defined.
//    TPci1203Backend    -- Acm_DaqDo/DiGet* (EtherCAT/AdvMotApi.h).
//                          Active when HAVE_PCI1203 is defined.
//    TMnetLegacyBackend -- _mnet_* functions (legacy PISO, dead path).
//                          Active when HAVE_MNET is defined.
//    TRawPortBackend    -- outportb/inportb (x86 raw, dead on 64-bit).
//                          Active when HAVE_RAWPORT && x86 arch.
//
//  NOTE ON VENDOR RETURN CODES:
//    Backends return the RAW vendor ret so that TLaneIO can apply per-method
//    interpretation (e.g. PCI1203 maps 0==SUCCESS -> +1, else -1 before passing
//    to the ret<0 guard; IO_CARD_TYPE==0 quirk Value=byte(ret) stays in TLaneIO
//    IOInputByte).  TSimIOBackend always returns 0 (>=0 => success).
//
//  DIMS (from Motor/myMN200motor.h:9-11):
//    MAXRing=4, MAXIP=64, MAXPort=4
//    Redeclared here locally (not via myMN200motor.h) to avoid pulling the
//    full MN200 motor chain into this pure IO header.
// =============================================================================
#ifndef IOBackendH
#define IOBackendH

#include "vclcompat/vcl_compat.h"   // neutralize __fastcall/__published/etc.
#include <windows.h>                // byte / BYTE / DWORD

// ---------------------------------------------------------------------------
//  IO array dimensions (same values as Motor/myMN200motor.h:9-11)
// ---------------------------------------------------------------------------
#ifndef IO_MAXRing
#define IO_MAXRing  4
#endif
#ifndef IO_MAXIP
#define IO_MAXIP   64
#endif
#ifndef IO_MAXPort
#define IO_MAXPort  4
#endif

// ---------------------------------------------------------------------------
//  TIOBackend -- abstract virtual base
//  All methods have default no-op bodies (return 0) so partial implementations
//  compile without warnings.  int<0=fail contract applies throughout.
// ---------------------------------------------------------------------------
class TIOBackend
{
public:
    virtual ~TIOBackend() {}

    // WriteBit: set a single output bit to Value (0 or 1).
    //   Sources: mn_set_port_bit / _mnet_bit_io_output / Acm_DaqDoSetBitEx
    virtual int WriteBit (int Ring, int IP, int Port, int Bit, int Value)
        { (void)Ring; (void)IP; (void)Port; (void)Bit; (void)Value; return 0; }

    // WriteByte: write an 8-bit byte to an output port.
    //   Sources: mn_set_port_byte / _mnet_io_output / Acm_DaqDoSetByteEx
    virtual int WriteByte(int Ring, int IP, int Port, unsigned int Byte)
        { (void)Ring; (void)IP; (void)Port; (void)Byte; return 0; }

    // ReadBit: read a single input bit into *Value (0 or 1).
    //   Sources: mn_get_port_bit / _mnet_bit_io_input / Acm_DaqDiGetBitEx
    //   *Value is zeroed on entry if non-null; backend fills it on success.
    virtual int ReadBit (int Ring, int IP, int Port, int Bit, unsigned char *Value)
        { (void)Ring; (void)IP; (void)Port; (void)Bit; if(Value) *Value=0; return 0; }

    // ReadByte: read an 8-bit input byte into *Value.
    //   Sources: mn_get_di_byte / _mnet_io_input / Acm_DaqDiGetByteEx
    //   Note: IO_CARD_TYPE==0 quirk (Value=byte(ret)) is handled in TLaneIO
    //   IOInputByte, NOT here.  Backends return RAW vendor ret.
    virtual int ReadByte(int Ring, int IP, int Port, unsigned char *Value)
        { (void)Ring; (void)IP; (void)Port; if(Value) *Value=0; return 0; }
};

// ---------------------------------------------------------------------------
//  TSimIOBackend -- offline in-memory backend (DEFAULT, no vendor SDK)
//  Provides a 3D bit-map mBit[IO_MAXRing][IO_MAXIP][IO_MAXPort] of bytes.
//  WriteBit stores the bit; ReadBit returns it.
//  WriteByte stores the byte; ReadByte returns it.
//  All operations succeed (return 0) -- mirrors BCB6 SOFT_SIMULTE "ok" path.
// ---------------------------------------------------------------------------
class TSimIOBackend : public TIOBackend
{
private:
    unsigned char mBit[IO_MAXRing][IO_MAXIP][IO_MAXPort];  // per-port byte cache

public:
    TSimIOBackend();

    virtual int WriteBit (int Ring, int IP, int Port, int Bit, int Value) override;
    virtual int WriteByte(int Ring, int IP, int Port, unsigned int Byte)  override;
    virtual int ReadBit  (int Ring, int IP, int Port, int Bit, unsigned char *Value) override;
    virtual int ReadByte (int Ring, int IP, int Port, unsigned char *Value) override;
};

// ---------------------------------------------------------------------------
//  TMN200Backend -- MN200 motion-net IO (ACTIVE real backend on this machine)
//  Vendor calls: mn_set_port_bit / mn_set_port_byte / mn_get_port_bit /
//                mn_get_di_byte   (Motor/mn200.h)
//  Guarded by #if HAVE_MN200; #else each method {return 0;} (compiles, never
//  linked to vendor).  Default OFF.
// ---------------------------------------------------------------------------
class TMN200Backend : public TIOBackend
{
public:
    virtual int WriteBit (int Ring, int IP, int Port, int Bit, int Value) override;
    virtual int WriteByte(int Ring, int IP, int Port, unsigned int Byte)  override;
    virtual int ReadBit  (int Ring, int IP, int Port, int Bit, unsigned char *Value) override;
    virtual int ReadByte (int Ring, int IP, int Port, unsigned char *Value) override;
};

// ---------------------------------------------------------------------------
//  TPci1203Backend -- Advantech PCI-1203 / EtherCAT DAQ IO
//  Vendor calls: Acm_DaqDoSetBitEx / Acm_DaqDoSetByteEx / Acm_DaqDiGetBitEx /
//                Acm_DaqDiGetByteEx   (EtherCAT/AdvMotApi.h)
//  Returns RAW vendor ret (TLaneIO maps 0==SUCCESS -> +1, else -1).
//  Guarded by #if HAVE_PCI1203; #else {return 0;}.  Default OFF.
// ---------------------------------------------------------------------------
class TPci1203Backend : public TIOBackend
{
public:
    virtual int WriteBit (int Ring, int IP, int Port, int Bit, int Value) override;
    virtual int WriteByte(int Ring, int IP, int Port, unsigned int Byte)  override;
    virtual int ReadBit  (int Ring, int IP, int Port, int Bit, unsigned char *Value) override;
    virtual int ReadByte (int Ring, int IP, int Port, unsigned char *Value) override;
};

// ---------------------------------------------------------------------------
//  TMnetLegacyBackend -- legacy PISO _mnet_* functions (dead path)
//  Vendor calls: _mnet_bit_io_output / _mnet_io_output / _mnet_bit_io_input /
//                _mnet_io_input
//  Guarded by #if HAVE_MNET; #else {return 0;}.  Default OFF.
// ---------------------------------------------------------------------------
class TMnetLegacyBackend : public TIOBackend
{
public:
    virtual int WriteBit (int Ring, int IP, int Port, int Bit, int Value) override;
    virtual int WriteByte(int Ring, int IP, int Port, unsigned int Byte)  override;
    virtual int ReadBit  (int Ring, int IP, int Port, int Bit, unsigned char *Value) override;
    virtual int ReadByte (int Ring, int IP, int Port, unsigned char *Value) override;
};

// ---------------------------------------------------------------------------
//  TRawPortBackend -- raw outportb/inportb (x86 only, dead on 64-bit)
//  Guarded by #if defined(HAVE_RAWPORT) && (defined(_M_IX86)||defined(__i386__)).
//  This is the myio.cpp raw path; TLaneIO itself never calls raw outportb --
//  the object layer routes raw-port to myio free funcs (W6).  Provided here
//  for completeness; NOT needed for the W4-IO facade batch.
// ---------------------------------------------------------------------------
class TRawPortBackend : public TIOBackend
{
public:
    virtual int WriteBit (int Ring, int IP, int Port, int Bit, int Value) override;
    virtual int WriteByte(int Ring, int IP, int Port, unsigned int Byte)  override;
    virtual int ReadBit  (int Ring, int IP, int Port, int Bit, unsigned char *Value) override;
    virtual int ReadByte (int Ring, int IP, int Port, unsigned char *Value) override;
};

#endif // IOBackendH
