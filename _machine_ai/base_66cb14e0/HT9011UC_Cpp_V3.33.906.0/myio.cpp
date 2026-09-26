// =============================================================================
//  myio.cpp  --  raw ISA/PCI digital-IO port read/write (outportb/inportb front
//                end): the flat 128-slot OutPortIndex/InPortIndex port-number
//                registry, bit set/clear/byte-out, and the output-echo backup/
//                restore pair.
//
//  Faithful translation of golden myio.cpp (318 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 (rootleaf group).
//
//  ROLE: SAFETY-CRITICAL raw digital-IO layer (this wave's own group brief
//  flags it explicitly).  See myio.h's own banner for how this API relates to
//  ht9045_io's TLaneIO -- they are two distinct golden call surfaces, NOT
//  merged here.
//
//  WAVE SCOPE -- ACTIVE vs SATISFIED-BY-SUBSTRATE:
//   ACTIVE (faithful, verbatim control flow -- all 12 function DEFINITIONS
//     golden myio.cpp actually contains; see the header-vs-cpp discrepancy
//     note below for the other 2 declared-only names):
//       InitialMyOutIOData   golden :47-54
//       IOSetOutport         golden :56-83
//       IOSetInport          golden :85-104
//       IOBitOn              golden :107-139
//       IOBitOff             golden :141-172
//       IOByteOut            golden :174-201
//       IOOutBitStatus       golden :203-233
//       IOOutByteStatus      golden :235-246
//       IOInputBit           golden :248-286
//       IOInputByte          golden :288-296
//       BackUpOutputData     golden :301-308
//       RestoreOutputData    golden :310-317
//     Plus all file-scope globals: MyLongMask[32], lOutPortData[128],
//     lInPortData[128], OutPortIndex[128], InPortIndex[128], OutPortData[128],
//     InPortData[128], OutPos, InPos, BackOutPortIndex[128], BackOutPortData
//     [128], BackOutLongData[128] (golden :12-26, :298-300).
//   HEADER-VS-CPP DISCREPANCY (golden's own, not introduced here): myio.h
//     declares IOInputLongByte(int) and InitialMyIOData() -- NEITHER has a
//     definition anywhere in golden myio.cpp (confirmed by reading the whole
//     318-line file: golden instead defines a DIFFERENTLY-NAMED
//     InitialMyOutIOData, and IOInputLongByte never appears as a definition at
//     all). Not this wave's gap to close -- myio.h's own banner records it,
//     and nothing in this port tree calls either symbol today (whole-tree
//     grep), so it is not a link failure yet either.
//   SATISFIED-BY-SUBSTRATE: TTL_CARD_TYPE + MyBitMask[8] (cmydef.h, real);
//     bPLCIO[2048][8] / bPLCInData[INPUT_MAX_Slave][INPUT_MAX_REGISTER][8] /
//     PLCIO_INPUT_START_PORT_NUMBER / INPUT_MAX_Slave / INPUT_MAX_REGISTER
//     (MyPLC/MyPLC_IO_Modbus.h, real -- defined MyLaneIo.cpp:101-102); byte
//     (windows.h, via <rpcndr.h>, same precedent as MyLaneIo.h:27).
//
//  GATE REGISTER -- 3 gates (was 4; GATE (4) opened 20260916, see :180)
//  (grep over every .cpp/.h outside build*/, confirmed with the CMakeLists.txt
//  W4-IO deferral note "myio.cpp raw-port free funcs (outportb/IOSetOutport/
//  TTL_CARD_TYPE + 2-arg IdleCheckSafeDoorByCylinder)" -- this wave is that
//  deferred work, and the grep below is independent confirmation, not a
//  re-statement of that comment):
//   (1) outportb(port, byte)         -- golden :137, :170, :199 (IOBitOn/
//       IOBitOff/IOByteOut). Grepped the whole tree: the only occurrences of
//       the bare identifier `outportb` are in COMMENTS (IOBackend.h/.cpp,
//       mycylin.cpp, myswitch.cpp, tests/test_sim_io.cpp) -- zero compiled
//       bodies. This is IOBackend.h's own documented TRawPortBackend gap
//       ("outportb/inportb are compiler builtins or from <conio.h> under
//       BCB6" -- IOBackend.cpp:224 -- and that whole backend is dead-on-64-bit
//       by that same file's own banner). FAITHFUL DEFAULT: a no-op (the write
//       to OutPortData[ret] two lines above every gated call site still
//       happens -- only the actual hardware line toggle is dropped), matching
//       this tree's tree-wide "Sim path (default): compiles+links with NO
//       vendor SDK" stance for the entire IO layer (CMakeLists.txt's own
//       ht9045_io banner: "real backends compile as stubs (each method
//       {return 0;})"). BEHAVIOUR DELTA: on a real ISA/PCI raw-port card,
//       golden would toggle the physical output bit each call; here the
//       software-side echo cache (OutPortData[]) still updates but no
//       hardware line moves. Retire together with IOBackend.h's own
//       TRawPortBackend real-mode wave (W4-IO-part2 / W6, its own words).
//   (2) inportb(port)                -- golden :280, :295 (IOInputBit/
//       IOInputByte). Same grep result as (1) -- comments only, zero bodies.
//       FAITHFUL DEFAULT: returns 0 (`(byte)0`) -- a raw digital input read
//       as no-bits-set, the same "absent hardware reads as its safe/idle
//       value" stance TIOBackend's own TSimIOBackend already uses tree-wide
//       for the newer facade. BEHAVIOUR DELTA: golden would report the real
//       input latch state; here every non-PLC raw input bit reads permanently
//       low (0) offline.
//   (3) EnableNTPort(port, port)     -- golden :79, :100 (IOSetOutport/
//       IOSetInport). Grepped the whole tree: ZERO occurrences anywhere,
//       not even in a comment -- this identifier does not exist in this port
//       at all. In real BCB6 this is the DriverLINX/WinIO-style "grant
//       user-mode raw I/O port access" call the NT-family kernel requires
//       before outportb/inportb can touch hardware at all; it has no
//       standalone meaning once (1)/(2) above are already no-ops. FAITHFUL
//       DEFAULT: a no-op -- there is nothing to "enable" access to when the
//       actual port read/write it would gate is itself gated to a no-op.
//       BEHAVIOUR DELTA: none beyond what (1)/(2) already state -- this call
//       has no observable effect of its own in golden either, beyond making
//       the (1)/(2) calls possible.
//   (4) IdleCheckSafeDoorByCylinder(int,int) [2-arg, default 2nd = -1]  --
//       golden :112, :146, :179 (IOBitOn/IOBitOff/IOByteOut).
//       ⚠⚠ AI(W906-BU-D7) 20260916: THIS ENTRY WAS RETIRED.  Everything the old
//       text asserted is now false and it is kept here only so the correction is
//       readable next to what it corrects:
//         - "grepped the whole tree ... NONE exists" -- the 2-arg body IS here,
//           csystem.cpp:23353; nm on ht9045_sm.dir/csystem.cpp.obj lists
//           `T IdleCheckSafeDoorByCylinder(int, int)`.
//         - "the only body ... is a file-local static stub at MyLaneIo.cpp:49-52"
//           -- false twice: the real 4-arg body is csystem.cpp:23395, and that
//           static stub was retired the same day (MyLaneIo.cpp:44-52).
//         - "no door-sensor model exists anywhere in this port tree yet" -- one
//           does: the Sen[iSafeDoor[i]].IsOff() scan in CheckSafeDoorIsClosed,
//           csystem.cpp:23144-23181.
//       The golden in-line caution is kept verbatim at the call sites: "注意!!
//       IdleCheckSafeDoorByCylinder為Handler安全門相關,修改時要小心!!"
//       ("CAUTION!! IdleCheckSafeDoorByCylinder is Handler-safe-door-related, be
//       careful when modifying!!").
//       ⚠ WHAT IS TRUE NOW: the macro calls the real body, but NOTHING IN ANY
//       BINARY CALLS myio's free functions -- nm --undefined-only over all 157
//       objects finds zero references to _Z7IOBitOnii and its siblings, so
//       myio.cpp.obj is never even extracted from libht9045_io.a.  The gate is
//       open and inert.  Before wiring myio, read the dormant re-entry hazard in
//       docs/BU_C_CAMPAIGN_PLAN.md §1.5: CheckSafeDoorIsClosed is not a pure
//       predicate -- it raises modals, calls StopAllMotor() and drives
//       Cylinder[iC_DoorLock[i]].On()/Off(), which re-enters this same IO layer.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped (golden :2, :10). The dead `//CheckWindowsIsXP` block (golden
//  :28-45) is kept as a comment, unchanged -- it was already dead in golden
//  (never called, guarded out with `//` on every line). No __fastcall /
//  __property / __published in this .cpp; golden has none either. `#include
//  "ModbusTCPClient.h"` (golden :8) is NOT re-included here: nothing in any
//  of the 12 translated bodies below reaches a ModbusTCPClient symbol
//  directly (only the PLC-side bPLCIO/bPLCInData globals inside
//  MyPLC_IO_Modbus.h are touched, and that header already #includes
//  ModbusTCPClient.h itself, golden myio.cpp :7 mirrored transitively).
//  `//#include "cmydef.h"` (golden :6) is golden's own dead duplicate-include
//  comment line -- cmydef.h IS included once (golden :5), just as here.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8, in
//  its original column. Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "myio.h"
//   #include "cmydef.h"
//   //#include "cmydef.h"                     -- golden's own dead duplicate
//   #include "MyPLC_IO_Modbus.h"              -- this tree's copy lives at MyPLC/MyPLC_IO_Modbus.h
//   #include "ModbusTCPClient.h"              -- NOT re-included here, see banner
//   ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)
#include "myio.h"                   // this unit's own contract (12 functions + globals defined below)
#include "cmydef.h"                 // TTL_CARD_TYPE (cmydef.cpp:3645), MyBitMask[8] (cmydef.cpp:119)
#include "MyPLC/MyPLC_IO_Modbus.h"  // bPLCIO[2048][8] / bPLCInData[][][8] / PLCIO_INPUT_START_PORT_NUMBER (real, defined MyLaneIo.cpp:101-102)

// AI(W906-PT-W2) 20260807: iPLCSafetyVer is declared in cmydef.h (cmydef.cpp:5620),
// which is already #included above via the transitive chain cmydef.h itself pulls
// in nowhere for this symbol -- it needs its own line since MyPLC_IO_Modbus.h does
// not re-declare it. (cmydef.h line 5606's own extern is already in scope through
// the #include above; this comment exists only so a future reader does not go
// looking for iPLCSafetyVer's declaration inside MyPLC_IO_Modbus.h and fail to find it.)

//---------------------------------------------------------------------------
//  GATE REGISTER macro pairs (see banner for the full rationale of each).
//  Idiom matches aTester_Front.cpp's established macro-pair gate shape.
//---------------------------------------------------------------------------
#if 0 // AI(W906-W3-MYIO) 20260924 維持閘住（W3 20260924 重驗）：x64 上沒有這條硬體路徑；HT9050 的 IO 走 1203（MyLaneIo 的 pIO1203），不經原始 port；且 myio 的自由函式全樹 0 個被引用（nm，docs/W3_CALLER_CENSUS_20260924.md）。GATE (1) -- golden :137/:170/:199 outportb(port,byte), no compiled body anywhere in this tree (see banner)
#define MYIO_OUTPORTB(port, val)  outportb(port, val)
#else
#define MYIO_OUTPORTB(port, val)  ((void)0)
#endif

#if 0 // AI(W906-W3-MYIO) 20260924 維持閘住（W3 20260924 重驗）：x64 上沒有這條硬體路徑；HT9050 的 IO 走 1203（MyLaneIo 的 pIO1203），不經原始 port；且 myio 的自由函式全樹 0 個被引用（nm，docs/W3_CALLER_CENSUS_20260924.md）。GATE (2) -- golden :280/:295 inportb(port), no compiled body anywhere in this tree (see banner)
#define MYIO_INPORTB(port)        inportb(port)
#else
#define MYIO_INPORTB(port)        ((byte)0)
#endif

#if 0 // AI(W906-W3-MYIO) 20260924 維持閘住（W3 20260924 重驗）：x64 上沒有這條硬體路徑；HT9050 的 IO 走 1203（MyLaneIo 的 pIO1203），不經原始 port；且 myio 的自由函式全樹 0 個被引用（nm，docs/W3_CALLER_CENSUS_20260924.md）。GATE (3) -- golden :79/:100 EnableNTPort(port,port), symbol does not exist anywhere in this tree (see banner)
#define MYIO_ENABLENTPORT(a, b)   EnableNTPort(a, b)
#else
#define MYIO_ENABLENTPORT(a, b)   ((void)0)
#endif

// AI(W906-BU-D7) 20260916: GATE (4) OPENED by user ruling D-7.  Its premise, "no
//   compiled body for THIS arity", EXPIRED -- body is csystem.cpp:23353, nm lists
//   `T IdleCheckSafeDoorByCylinder(int, int)`.  Decl = the extern at :280.
//   ⚠ INERT TODAY: no object references myio's free functions, so this never runs.
#define MYIO_IDLECHECKSAFEDOOR(port, bit)  IdleCheckSafeDoorByCylinder(port, bit)

//---------------------------------------------------------------------------
unsigned int MyLongMask[32]=
    {0x00000001,0x00000002,0x00000004,0x00000008,0x00000010,0x00000020,0x00000040,0x00000080,
     0x00000100,0x00000200,0x00000400,0x00000800,0x00001000,0x00002000,0x00004000,0x00008000,
     0x00010000,0x00020000,0x00040000,0x00080000,0x00100000,0x00200000,0x00400000,0x00800000,
     0x01000000,0x02000000,0x04000000,0x08000000,0x10000000,0x20000000,0x40000000,0x80000000};

unsigned int lOutPortData[128];
unsigned int lInPortData[128];

int  OutPortIndex[128];
int  InPortIndex[128];
byte OutPortData[128];
byte InPortData[128];
int  OutPos=0;
int  InPos=0;
//---------------------------------------------------------------------------
//void CheckWindowsIsXP()
//{
//    AnsiString Str;
//    OSVERSIONINFO osvi;
//    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
//    osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
//    GetVersionEx(&osvi);
//    int a=osvi.dwMajorVersion;                                                  //主版本
//    int b=osvi.dwMinorVersion;                                                  //次版本
//    bIsWinXP=(a==5 && b==1);
//    if(a==4 && b==0)  Str.sprintf("Windows NT Ver:%d.%d\n", a, b);
//    if(a==4 && b==10) Str.sprintf("Windows 98 Ver:%d.%d\n", a, b);
//    if(a==4 && b==90) Str.sprintf("Windows ME Ver:%d.%d\n", a, b);
//    if(a==5 && b==0)  Str.sprintf("Windows 2000 Ver:%d.%d\n", a, b);
//    if(a==5 && b==1)  Str.sprintf("Windows XP Ver:%d.%d\n", a, b);
//    if(a==6 && b==0)  Str.sprintf("Windows Vista Ver:%d.%d\n", a, b);
//    if(a==6 && b==1)  Str.sprintf("Windows 7 Ver:%d.%d\n", a, b);
//}
//---------------------------------------------------------------------------
void InitialMyOutIOData()
{
    for(int i=0; i<128; i++)
    {
        OutPortIndex[i]=0;
        OutPortData[i]=0;
    }
}
//---------------------------------------------------------------------------
int IOSetOutport(int port)
{
    static bool flag=true;

    if(flag)
    {
        InitialMyOutIOData();
        flag=false;
    }

    if(TTL_CARD_TYPE>0)                                                         //Steven 20200901 : Win10 & Win7也可以用TTL
        return 999;

    if(port==0)
        return 999;

    for(int i=0; i<OutPos; i++)
        if(OutPortIndex[i]==port)
            return i;

    if(OutPos>=100)
        return 999;                                                             // 不能發生 !!!

    MYIO_ENABLENTPORT(port, port);
    OutPortIndex[OutPos]=port;
    OutPos++;
    return (OutPos-1);
};
//---------------------------------------------------------------------------
bool IOSetInport(int port)
{
    if(TTL_CARD_TYPE>0)                                                         //Steven 20200901 : Win10 & Win7也可以用TTL
        return 999;

    if(port==0)
        return false;

    for(int i=0; i<InPos; i++)
        if(InPortIndex[i]==port)
            return true;

    if(InPos>=100)
        return false;                                                           // 不能發生 !!!

    MYIO_ENABLENTPORT(port, port);
    InPortIndex[InPos]=port;
    InPos++;
    return true;
};
extern bool IdleCheckSafeDoorByCylinder(int iPort, int iBit);   // csystem.h:240's 2-arg overload, default arg dropped for this local decl (golden re-declares it here too). AI(W906-BU-D7) 20260916: it used to say "resolved via GATE (4) macro below, not this decl" -- that is now BACKWARDS. Since the gate opened, THIS decl is the only thing that makes :287/:321/:354 compile; csystem.h is not in this TU.
//---------------------------------------------------------------------------
void IOBitOn(int port, int bit)
{
    //******************************************************************************
    //  注意!! IdleCheckSafeDoorByCylinder為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(MYIO_IDLECHECKSAFEDOOR(port, bit)==true)                            // 2015.01.15 , Joye , Safe door check
        return;

    int ret=-1;
    AnsiString S="";
    if(TTL_CARD_TYPE>0)                                                         //Steven 20200901 : Win10 & Win7也可以用TTL
        return;

    if(port<0x210)
    {
        if((bit>=32 || bit<0 || port==0)==true)
            return;
        ret=IOSetOutport(port);
        if(ret==999 || ret<0 || ret>127)
            return;
        lOutPortData[ret]|=MyLongMask[bit];
    }
    else
    {
        if((bit>=8 || bit<0 || port==0)==true)
            return;
        ret=IOSetOutport(port);
        if(ret==999 || ret<0 || ret>127)
            return;
        OutPortData[ret]|=MyBitMask[bit];
        MYIO_OUTPORTB(port,OutPortData[ret]);
    }
}
//---------------------------------------------------------------------------
void IOBitOff(int port, int bit)
{
    //******************************************************************************
    //  注意!! IdleCheckSafeDoorByCylinder為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(MYIO_IDLECHECKSAFEDOOR(port, bit)==true)                            // 2015.01.15 , Joye , Safe door check
        return;

    int ret=-1;
    if(TTL_CARD_TYPE>0)                                                         //Steven 20200901 : Win10 & Win7也可以用TTL
        return;

    if(port<0x210)
    {
        if((bit>=32 || bit<0 || port==0)==true)
            return;
        ret=IOSetOutport(port);
        if(ret==999 || ret<0 || ret>127)
            return;
        lOutPortData[ret]&=~MyLongMask[bit];
    }
    else
    {
        if((bit>=8 || bit<0 || port==0)==true)
            return;
        ret=IOSetOutport(port);
        if(ret==999 || ret<0 || ret>127)
            return;
        OutPortData[ret]&=~MyBitMask[bit];
        MYIO_OUTPORTB(port,OutPortData[ret]);
    }
}
//---------------------------------------------------------------------------
void IOByteOut(int port, unsigned int Byte)
{
    //******************************************************************************
    //  注意!! IdleCheckSafeDoorByCylinder為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(MYIO_IDLECHECKSAFEDOOR(port, -1)==true)                             //Steven 20230703 : Add for MotionNet IO check SafeDoor
        return;

    int ret=-1;
    if(TTL_CARD_TYPE>0)
        return;

    if(port<0x210)
    {
        ret=IOSetOutport(port);
        if(ret==999 || ret<0 || ret>127)
            return;
        lOutPortData[ret]=Byte;
    }
    else
    {
        ret=IOSetOutport(port);
        if(ret==999 || ret<0 || ret>127)
            return;
        OutPortData[ret]=Byte;
        MYIO_OUTPORTB(port,OutPortData[ret]);
    }
}
//---------------------------------------------------------------------------
bool IOOutBitStatus(int port,int bit)
{
    int ret=-1;

    if(TTL_CARD_TYPE>0)
        return false;

    if(port<0x210)
    {
        ret=IOSetOutport(port);
        if(ret==999 || ret<0 || ret>127)
            return false;
        if(lOutPortData[ret] & MyLongMask[bit])
            return true;
        else
            return false;
    }
    else
    {
        int ret;
        if(bit>=8 || bit<0)
            return false;
        ret=IOSetOutport(port);
        if(ret==999 || ret<0 || ret>127)
            return false;
        if(OutPortData[ret] & MyBitMask[bit])
            return true;
        else
            return false;
    }
}
//---------------------------------------------------------------------------
byte IOOutByteStatus(int port)
{
    int ret=-1;

    if(TTL_CARD_TYPE>0)
        return false;

    ret=IOSetOutport(port);
    if(ret==999 || ret<0 || ret>127)
        return false;
    return OutPortData[ret];
}
//---------------------------------------------------------------------------
bool IOInputBit(int port, int bit)
{
    if(TTL_CARD_TYPE>0)
        return false;

    if(port>=0 &&                                                               //Steven 20240103 : 加入保護
       bit>=0  &&
       bPLCIO[port][bit])
    {                                                                           //Jason 20230619 增加安全PLC部分
        if(port>=PLCIO_INPUT_START_PORT_NUMBER)                                 //Austin 20190218.01 增加PLC IO for CE
        {
            if(iPLCSafetyVer==202204)
            {
                return bPLCInData[0][port-PLCIO_INPUT_START_PORT_NUMBER+2][bit];                                        //joe 20220406.01 debug safe plc
            }
            else
            {
                return bPLCInData[0][port-PLCIO_INPUT_START_PORT_NUMBER+1][bit];                                        //第1個Byte為系統狀態 預留
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(bit>=8 || bit<0)                                                     //Steven 20160520 : TTL IO加上保護
            return false;

        if(IOSetInport(port)==false) return false;
        byte b;
        b=MYIO_INPORTB(port);
        if(b&MyBitMask[bit])
            return true;
        else
            return false;
    }
}
//---------------------------------------------------------------------------
byte IOInputByte(int port)
{
    if(TTL_CARD_TYPE>0)                                                         //Steven 20200901 : Win10 & Win7也可以用TTL
        return false;

    if(IOSetInport(port)==false)
        return false;
    return MYIO_INPORTB(port);
}
//---------------------------------------------------------------------------
int  BackOutPortIndex[128];
byte BackOutPortData[128];
unsigned int BackOutLongData[128];
void BackUpOutputData()
{
    for(int i=0; i<128; i++)
    {
        BackOutLongData[i]=lOutPortData[i];
        BackOutPortData[i]=OutPortData[i];
    }
}
//---------------------------------------------------------------------------
void RestoreOutputData()
{
    for(int i=0; i<OutPos; i++)
    {
        OutPortData[i]=BackOutPortData[i];
        IOByteOut(OutPortIndex[i], OutPortData[i]);
    }
}
//---------------------------------------------------------------------------
