// =============================================================================
//  myswitch.cpp  --  TMySwitch per-point output switch controller
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/myswitch.cpp
//  Translation wave: W4-IO (IO HAL layer)
//  Translator: AI(W4-IO) 20260626
//
//  Key changes vs. BCB6 original:
//    - `#pragma hdrstop` / `#pragma package(smart_init)` dropped (BCB-specific).
//    - `#include "uPadInterface.h"` (Ken 20210702 AddPadInterface) is GOD-STACK
//      (VCL form / UI -- uPadInterface.h pulls TForm, VCL button arrays, etc.).
//      The iControlPanelMode==1 && fPadInterface->IsPadButton(Name) branch is
//      gated `#if 0` throughout (AI(W906-W3-SWITCH) 20260924: RS232 實體面板未翻，W3 子項 4b).  The hardware Enable/Type/ISABase
//      branch is translated and active now.
//    - eISABase/ePCI1735U/ePLCbase raw-port branch calls free-func IOBitOn(Port,Bit)
//      / IOBitOff(Port,Bit) / IOOutBitStatus(Port,Bit) from myio.cpp (x86, W6).
//      AI(W906-W3-SWITCH) 20260924: 已接真的（原本是回 void/false 的樁），見 :38-40.
//    - eMotionNet||ePCI1203 branch routes through MyLaneIO.IOBitOn/IOBitOff/
//      IOOutBitStatus -- FULLY ACTIVE (depends only on TLaneIO facade).
// =============================================================================
#include "MachineDefine.h"
#include "myswitch.h"
#include "MyLaneIo.h"
#include "cmydef.h"     // ISABase constants (eMotionNet / ePCI1203 / eISABase /
                        // ePCI1735U / ePLCbase) via MachineType.h

// ---------------------------------------------------------------------------
//  Stub: raw-port free-funcs (myio.cpp, W6)
//  BCB6: IOBitOn(Port,Bit) / IOBitOff(Port,Bit) / IOOutBitStatus(Port,Bit)
//  These are the 2-param free functions from myio.cpp (raw outportb path).
//  Stubs are no-ops so the eISABase/ePCI1735U/ePLCbase branch compiles.
// ---------------------------------------------------------------------------
#if 0   // AI(W906-W3-SWITCH) 20260924: 下面 #else 那三行已換成真的宣告（myio.cpp）；這一半只剩原樁的史料。原註：TODO(W6): replace stubs with real myio.cpp free-func declarations
static void IOBitOn       (int /*Port*/, int /*Bit*/) {}
static void IOBitOff      (int /*Port*/, int /*Bit*/) {}
static bool IOOutBitStatus(int /*Port*/, int /*Bit*/) { return false; }
#else
extern void IOBitOn       (int port, int bit);  // AI(W906-W3-SWITCH) 20260924: 樁退役，接 myio.cpp 的真函式（含 golden 的 PLC 分支與兩參數 IdleCheckSafeDoorByCylinder 互鎖：閒置時若該點是要檢查安全門的氣缸，門開著就擋下輸出）。原始 port 寫入在 x64 上是 myio GATE (1) no-op；HT9050 的點全是 1203，不走這條
extern void IOBitOff      (int port, int bit);  // AI(W906-W3-SWITCH) 20260924: 同上
extern bool IOOutBitStatus(int port, int bit);  // AI(W906-W3-SWITCH) 20260924: 同上（讀 myio 的 OutPortData 快取，不碰硬體）
#endif

// ---------------------------------------------------------------------------
//  Global arrays
// ---------------------------------------------------------------------------
class TMySwitch SW[MAX_SWITCH_ITEM];
class TMySwitch SWBackup[MAX_TTL_SWITCH];  // Alick 20161011 (Steven): TTL 8-site

// ---------------------------------------------------------------------------
//  Ctor
//  BCB6: _fastcall TMySwitch::TMySwitch()
// ---------------------------------------------------------------------------
TMySwitch::TMySwitch()
{
    Name     = "";
    Ring     = 0;
    IP       = 0;
    Port     = 0;
    Bit      = 0;
    Type     = 0;
    Enable   = false;
    OutValue = false;
    SetValue = false;
    Using    = "";
    ISABase  = 0;
}

// ---------------------------------------------------------------------------
//  On()
//  BCB6: myswitch.cpp:28-72
//  Gate: iControlPanelMode==1 && fPadInterface->IsPadButton(Name) -> #if 0
//  Active: Enable/Type/ISABase hardware routing -> MyLaneIO
// ---------------------------------------------------------------------------
void TMySwitch::On()
{
    OutValue = true;

#if 0   // AI(W906-W3-SWITCH) 20260924 維持閘住，理由更正：fPadInterface 是 RS232 通訊式實體操作面板（golden uPadInterface.cpp，SendSwitchStatus 點面板燈），相依不存在 ⇒ W3 子項 4b（見 mysensor.cpp:72）。原註：TODO(W6): fPadInterface / iControlPanelMode (VCL GOD-STACK, Ken 20210702)
    if(iControlPanelMode==1 && fPadInterface->IsPadButton(Name))
    {
        fPadInterface->SendSwitchStatus(Name, true);
    }
    else
#endif
    {
        if(Enable == false)
            return;

        if(Type)
        {
            if(ISABase == eMotionNet ||
               ISABase == ePCI1203)             // Sam 20230724: add PCI1203 IO support
            {
                MyLaneIO.IOBitOn(Ring, IP, Port, Bit, ISABase, Name);
            }
            else if(ISABase == eISABase  ||     // Nickliu 20230309: add Cylinder ISABase Type
                    ISABase == ePCI1735U ||     // Alick 20160809: PCI-1735U
                    ISABase == ePLCbase)
            {
                IOBitOn(Port, Bit);             // AI(W906-W3-SWITCH) 20260924: myio 的真函式（見 :38）
            }
        }
        else
        {
            if(ISABase == eMotionNet ||
               ISABase == ePCI1203)             // Sam 20230724
            {
                MyLaneIO.IOBitOff(Ring, IP, Port, Bit, ISABase, Name);
            }
            else if(ISABase == eISABase  ||
                    ISABase == ePCI1735U ||
                    ISABase == ePLCbase)
            {
                IOBitOff(Port, Bit);            // AI(W906-W3-SWITCH) 20260924: myio 的真函式（見 :38）
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  Off()
//  BCB6: myswitch.cpp:74-116
// ---------------------------------------------------------------------------
void TMySwitch::Off()
{
    OutValue = false;

#if 0   // AI(W906-W3-SWITCH) 20260924 維持閘住：同 :78（uPadInterface 未翻，W3 子項 4b）。原註：TODO(W6): fPadInterface / iControlPanelMode
    if(iControlPanelMode==1 && fPadInterface->IsPadButton(Name))
    {
        fPadInterface->SendSwitchStatus(Name, false);
    }
    else
#endif
    {
        if(Enable == false)
            return;

        if(Type)
        {
            if(ISABase == eMotionNet ||
               ISABase == ePCI1203)             // Sam 20230724
            {
                MyLaneIO.IOBitOff(Ring, IP, Port, Bit, ISABase, Name);
            }
            else if(ISABase == eISABase  ||
                    ISABase == ePCI1735U ||
                    ISABase == ePLCbase)
            {
                IOBitOff(Port, Bit);            // AI(W906-W3-SWITCH) 20260924: myio 的真函式（見 :38）
            }
        }
        else
        {
            if(ISABase == eMotionNet ||
               ISABase == ePCI1203)             // Sam 20230724
            {
                MyLaneIO.IOBitOn(Ring, IP, Port, Bit, ISABase, Name);
            }
            else if(ISABase == eISABase  ||
                    ISABase == ePCI1735U ||
                    ISABase == ePLCbase)
            {
                IOBitOn(Port, Bit);             // AI(W906-W3-SWITCH) 20260924: myio 的真函式（見 :38）
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  Status()
//  BCB6: myswitch.cpp:118-147
//  Returns OutPortData cache (IOOutBitStatus -- no hardware read).
//  Type inversion applied: if(Type) return OutValue; else return !OutValue.
// ---------------------------------------------------------------------------
bool TMySwitch::Status()
{
#if 0   // AI(W906-W3-SWITCH) 20260924 維持閘住：同 :78（uPadInterface 未翻，W3 子項 4b）。原註：TODO(W6): fPadInterface / iControlPanelMode
    if(iControlPanelMode==1 && fPadInterface->IsPadButton(Name))
    {
        OutValue = fPadInterface->ProcessScanKey(Name);
        return OutValue;   // Steven 20211229: add return value
    }
    else
#endif
    {
        if(Enable == false)
            return false;

        if(ISABase == eMotionNet ||
           ISABase == ePCI1203)             // Sam 20230724
        {
            OutValue = MyLaneIO.IOOutBitStatus(Ring, IP, Port, Bit, ISABase, Name);
        }
        else if(ISABase == eISABase  ||     // Nickliu 20230309
                ISABase == ePCI1735U ||     // Alick 20160809
                ISABase == ePLCbase)
        {
            OutValue = IOOutBitStatus(Port, Bit);  // AI(W906-W3-SWITCH) 20260924: myio 的真函式（見 :40）
        }

        if(Type)
            return OutValue;
        else
            return !OutValue;
    }
}

// ---------------------------------------------------------------------------
//  OnOff()
//  BCB6: myswitch.cpp:149-155
// ---------------------------------------------------------------------------
void TMySwitch::OnOff(bool bOn)
{
    if(bOn)
        On();
    else
        Off();
}

// ---------------------------------------------------------------------------
//  CopySwitch()
//  BCB6: myswitch.cpp:157-176
//  Pure field copy -- no IO operations.
// ---------------------------------------------------------------------------
void CopySwitch(TMySwitch *Source, TMySwitch *Target)
{
    if(Source->ISABase == eISABase  ||
       Source->ISABase == ePCI1735U ||  // Alick 20160809
       Source->ISABase == ePLCbase)
    {
        Target->Port    = Source->Port;
        Target->Bit     = Source->Bit;
        Target->Type    = Source->Type;
        Target->ISABase = Source->ISABase;
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
