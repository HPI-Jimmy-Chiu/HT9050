// =============================================================================
//  mysensor.cpp  --  TMySensor per-point input sensor reader
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/mysensor.cpp
//  Translation wave: W4-IO (IO HAL layer)
//  Translator: AI(W4-IO) 20260626
//
//  Key changes vs. BCB6 original:
//    - `#pragma hdrstop` / `#pragma package(smart_init)` dropped.
//    - `#include "uPadInterface.h"` (Ken 20210702) is GOD-STACK (VCL form).
//      The iControlPanelMode==1 && fPadInterface->IsPadKey(Name) branch is
//      gated `#if 0` throughout -- AI(W906-W3-SENSOR) 20260924: 是 RS232 實體面板、未翻（W3 子項 4b），見 :72。
//    - eISABase/ePCI1735U raw IOInputBit(Port,Bit) free-func (myio.cpp) -- AI(W906-W3-SENSOR) 20260924: 已接真的，見 :29
//      （原本是回 false 的樁）。
//    - eMotionNet||ePCI1203||ePLCbase -> MyLaneIO.IOInputBit  FULLY ACTIVE.
//    - GetTrayBuildState / SetTrayBuildState / ClrTrayBuildState are NOT defined
//      here (they are defined in the main app cinitial/tray module, GOD-STACK).
//      Forward declarations are in mysensor.h.  They are not called in this file.
//    - Commented-out CheckSenPortRangeErr body preserved as comment (BCB6 .cpp:181).
// =============================================================================
#include "MachineDefine.h"
#include "mysensor.h"
#include "MyLaneIo.h"
#include "cmydef.h"     // eMotionNet / ePCI1203 / ePLCbase / eISABase / ePCI1735U

// ---------------------------------------------------------------------------
//  Stub: raw-port IOInputBit(Port,Bit) free-func (myio.cpp, W6)
// ---------------------------------------------------------------------------
extern bool IOInputBit(int port, int bit);  // AI(W906-W3-SENSOR) 20260924: 樁退役，改接真的（myio.cpp，golden myio.cpp 的舊式 ISA／PCI1735U 讀取；含 golden 的 PLC 分支與 TTL_CARD_TYPE 保護）。原始 port 讀取在 x64 上是 myio GATE (2)，讀到 0，與原樁的 false 結果相同；差別是 PLC 點（bPLCIO）現在照 golden 走 bPLCInData。nm 查過 myio.obj 的全域與其他 obj 無重複定義

// ---------------------------------------------------------------------------
//  Global arrays
// ---------------------------------------------------------------------------
class TMySensor Sen[MAX_SENSOR_ITEM];
class TMySensor SenBackUp[MAX_TTL_BIT];  // Alick 20161011 (Steven): TTL 8-site

// ---------------------------------------------------------------------------
//  Ctor
//  BCB6: _fastcall TMySensor::TMySensor()
// ---------------------------------------------------------------------------
TMySensor::TMySensor()
{
    Name   = "";
    Ring   = 0;
    IP     = 0;
    Port   = 0;
    Bit    = 0;
    Type   = 0;
    Enable = 0;
    Using  = "";
    ISABase = 0;
}

// ---------------------------------------------------------------------------
//  Dtor
//  BCB6: _fastcall TMySensor::~TMySensor()
// ---------------------------------------------------------------------------
TMySensor::~TMySensor()
{
    Enable = false;
}

// ---------------------------------------------------------------------------
//  Status()
//  BCB6: mysensor.cpp:37-77
//  Returns logical sensor state after Type inversion.
// ---------------------------------------------------------------------------
bool TMySensor::Status()
{
    bool ret = false;

#if 0   // AI(W906-W3-SENSOR) 20260924 維持閘住，理由更正：不是「VCL 表單」問題 —— fPadInterface 是 RS232 通訊式實體操作面板（golden uPadInterface.cpp 962 行，TPadRS232Thread；Start／Pause／Reset／Home／AlarmReset… 共 17 鍵），按鍵狀態來自硬體、歸 C++。相依不存在：uPadInterface 未翻 ⇒ W3 子項 4b。⚠ 筆電 Gerneral.ini ControlPanelMode=1（database.cpp:1624 讀入），且 cinitial.cpp:1678／:2897 會在此模式停用 IO 版面板鍵 ⇒ 此模式下實體面板鍵兩邊都讀不到。原註：TODO(W6): fPadInterface / iControlPanelMode (VCL GOD-STACK, Ken 20210702)
    if(iControlPanelMode==1 && fPadInterface->IsPadKey(Name))
    {
        ret = fPadInterface->ProcessScanKey(Name);
        State = 1;
        return ret;
    }
    else
#endif
    {
        if(Enable == false)
        {
            State = -1;
            return false;
        }

        if(ISABase == eMotionNet ||
           ISABase == ePCI1203   ||  // Sam 20230724: add PCI1203 IO support
           ISABase == ePLCbase)
        {
            ret = MyLaneIO.IOInputBit(Ring, IP, Port, Bit, ISABase, Name);
        }
        else if(ISABase == eISABase  ||  // Nickliu 20230309
                ISABase == ePCI1735U)    // Alick 20160809
        {
            ret = IOInputBit(Port, Bit);  // AI(W906-W3-SENSOR) 20260924: 接 myio 的真函式（見 :29）
        }

        if(Type)
        {
            State = 1;
            return ret;
        }
        else
        {
            State = 0;
            return !ret;
        }
    }
}

// ---------------------------------------------------------------------------
//  IsOn()
//  BCB6: mysensor.cpp:79-117
// ---------------------------------------------------------------------------
bool TMySensor::IsOn()
{
#if 0   // AI(W906-W3-SENSOR) 20260924 維持閘住：同 :72（uPadInterface 未翻，W3 子項 4b）。原註：TODO(W6): fPadInterface / iControlPanelMode
    if(iControlPanelMode==1 && fPadInterface->IsPadKey(Name))
    {
        return fPadInterface->ProcessScanKey(Name);
    }
    else
#endif
    {
        if(Enable == false)
        {
            State = -1;
            return false;
        }

        bool ret = false;
        if(ISABase == eMotionNet ||
           ISABase == ePCI1203   ||  // Sam 20230724
           ISABase == ePLCbase)
        {
            ret = MyLaneIO.IOInputBit(Ring, IP, Port, Bit, ISABase, Name);
        }
        else if(ISABase == eISABase  ||  // Nickliu 20230309
                ISABase == ePCI1735U)    // Alick 20160809
        {
            ret = IOInputBit(Port, Bit);  // AI(W906-W3-SENSOR) 20260924: 接 myio 的真函式（見 :29）
        }

        if(Type)
        {
            State = 1;
            return ret;
        }
        else
        {
            State = 0;
            return !ret;
        }
    }
}

// ---------------------------------------------------------------------------
//  IsOff()
//  BCB6: mysensor.cpp:119-159
// ---------------------------------------------------------------------------
bool TMySensor::IsOff()
{
    bool ret = false;

#if 0   // AI(W906-W3-SENSOR) 20260924 維持閘住：同 :72（uPadInterface 未翻，W3 子項 4b）。原註：TODO(W6): fPadInterface / iControlPanelMode
    if(iControlPanelMode==1 && fPadInterface->IsPadKey(Name))
    {
        ret = fPadInterface->ProcessScanKey(Name);
        State = 0;
        return !ret;
    }
    else
#endif
    {
        if(Enable == false)
        {
            State = -1;
            return false;
        }

        if(ISABase == eMotionNet ||
           ISABase == ePCI1203   ||  // Sam 20230724
           ISABase == ePLCbase)
        {
            ret = MyLaneIO.IOInputBit(Ring, IP, Port, Bit, ISABase, Name);
        }
        else if(ISABase == eISABase  ||  // Nickliu 20230309
                ISABase == ePCI1735U)    // Alick 20160809
        {
            ret = IOInputBit(Port, Bit);  // AI(W906-W3-SENSOR) 20260924: 接 myio 的真函式（見 :29）
        }

        if(Type)
        {
            State = 0;
            return !ret;
        }
        else
        {
            State = 1;
            return ret;
        }
    }
}

// ---------------------------------------------------------------------------
//  CopySensor()
//  BCB6: mysensor.cpp:161-178
//  Pure field copy -- no IO operations.
// ---------------------------------------------------------------------------
void CopySensor(TMySensor *Source, TMySensor *Target)
{
    if(Source->ISABase == eISABase  ||   // Nickliu 20230309
       Source->ISABase == ePCI1735U)     // Alick 20160809
    {
        Target->Port    = Source->Port;
        Target->Bit     = Source->Bit;
        Target->Type    = Source->Type;
    }
    else
    {
        Target->Ring    = Source->Ring;
        Target->IP      = Source->IP;
        Target->Port    = Source->Port;
        Target->Bit     = Source->Bit;
        Target->Type    = Source->Type;
    }
}

// ---------------------------------------------------------------------------
//  CheckSenPortRangeErr -- commented out in BCB6 .cpp:181-232
//  Not translated (body was already disabled upstream).
// ---------------------------------------------------------------------------
// =============================================================================
//  WAVE SCOPE EXTENSION -- PT-Wk (APPEND-ONLY; the head banner above is left
//  byte-for-byte untouched, per this wave's append-only rule)
//  Translator: AI(PT-Wk-mysensor) 20260811
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/mysensor.cpp (cp950)
//
//  ROLE
//  ----
//  This unit is the per-point digital-INPUT sensor reader (TMySensor), pumped
//  by every state machine that asks "is this sensor on?".  This extension adds
//  no runnable code: the one golden function assigned to it does not exist as
//  code in golden.
//
//  WAVE SCOPE
//  ----------
//    * TMySensor::CheckSenPortRangeErr(int Type)   golden mysensor.cpp:181
//      GATED -- and gated IN GOLDEN, not by this port.  Golden lines 180-231 are
//      one `/* ... */` block comment; :181 is the first line INSIDE it.  The
//      matching declaration in golden mysensor.h:32 is likewise commented out
//      (`//    bool CheckSenPortRangeErr(int Type);`).  There is therefore no
//      compiled body and no declaration ANYWHERE in golden -- the symbol does
//      not exist in the shipped BCB6 binary.
//
//  GATE REGISTER
//  -------------
//  G-PTk3 -- TMySensor::CheckSenPortRangeErr.
//    (a) GOLDEN LINE: mysensor.cpp:180 opens `/*`, :181 is the signature,
//        :231 closes `}      */`.  Declaration site: golden mysensor.h:32,
//        also inside `//`.
//    (b) WHY THE OFFLINE DEFAULT IS FAITHFUL: the "offline default" here is
//        "the function does not exist", which is EXACTLY golden's own state.
//        Emitting an active body would not be a translation, it would be new
//        behaviour that golden never had: golden's disabled body ends in
//        `ShowMyMessage(sAlm); return true;`, i.e. a MODAL operator dialog plus
//        a hard "port range error" verdict on every sensor whose IP falls
//        outside the machine-type window.  Turning that on would (1) invent an
//        interlock golden does not enforce, and (2) in this port ShowMyMessage
//        is the canary_support stand-in -- and per the campaign's own history a
//        modal dialog inside a batch run is what hangs ctest with an empty log.
//        Neither risk is worth taking for a body upstream deliberately switched
//        off.  So it is carried forward as a COMMENT, verbatim, byte for byte,
//        including its Big5 Chinese annotations (transcoded cp950 -> UTF-8).
//    (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS: it does not.  The real machine runs
//        the same disabled code.  No caller exists in either tree (verified:
//        zero references to CheckSenPortRangeErr in the golden tree outside
//        these two commented lines, and zero in the port tree outside this
//        file's own notes).
//    (d) IF A LATER WAVE WANTS IT LIVE it needs DI_IP_Max_9045 / DI_IP_Min_9045 /
//        DI_IP_Max_9046 / DI_IP_Min_9046 / DI_IP_Max_9046LS / DI_IP_Min_9046LS /
//        DI_IP_Max_9045_12Site / DI_IP_Min_9045_12Site, MachineTypeChoice,
//        USE_46_SENSOR_DB / USE_46_SUCKER_DB and ShowMyMessage -- and, first, a
//        decision from the machine owners, because switching it on is a
//        BEHAVIOUR CHANGE to golden, not a port fix.
//
//  Golden mysensor.cpp:180-231, reproduced VERBATIM below (it is already a
//  comment in golden; nothing is nested, no `/*` or `*/` occurs inside it).
// =============================================================================
/*
bool TMySensor::CheckSenPortRangeErr(int Type)
{
    int iMax =0 ,iMin =0;
    AnsiString sType[]={"Status","IsOn","IsOff"};
    AnsiString sAlm;
    sAlm.sprintf("Sensor %s -- %s Ring %d|IP %d |Port %d, Fail", Name, sType[Type], Ring, IP, Port);

    switch(MachineTypeChoice)
    {
        case Type_HT9045:
            if(USE_46_SENSOR_DB || USE_46_SUCKER_DB)                            //Steven 20131008 : HT9045使用46配電
            {
                iMax=DI_IP_Max_9046;
                iMin=DI_IP_Min_9046;
            }
            else
            {
                iMax=DI_IP_Max_9045;
                iMin=DI_IP_Min_9045;
            }
            break;
        case Type_HT9046:
            iMax = DI_IP_Max_9046;
            iMin = DI_IP_Min_9046;
            break;
        case  Type_HT9046_LS:
            iMax = DI_IP_Max_9046LS;
            iMin = DI_IP_Min_9046LS;
            break;
        case Type_HT9045_12Site:
            if(USE_46_SENSOR_DB || USE_46_SUCKER_DB)                            //Steven 20131008 : HT9045使用46配電
            {
                iMax=DO_IP_Max_9046;
                iMin=DO_IP_Min_9046;
            }
            else
            {
                iMax = DI_IP_Max_9045_12Site;
                iMin = DI_IP_Min_9045_12Site;
            }
            break;
    }

    if(Ring!=0 || IP<iMin || IP>iMax || Port<0 || Port>3 || Bit<0 || Bit>7)
    {
        ShowMyMessage(sAlm);
        return true;
    }

    return false;
}      */
// ---------------------------------------------------------------------------
//  end golden mysensor.cpp:180-231 verbatim block
// ---------------------------------------------------------------------------
