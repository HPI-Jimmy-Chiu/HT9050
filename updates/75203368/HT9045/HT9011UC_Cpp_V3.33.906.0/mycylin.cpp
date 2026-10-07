// =============================================================================
//  mycylin.cpp  --  TMyCylinder per-cylinder controller (W6.0 substrate gap)
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/mycylin.cpp
//  Translation wave: W6.0 (substrate gap closed for the W6.1 Empty-tray canary)
//  Translator: AI(W6.0-SCAFFOLD) 20260626
//
//  Key changes vs. BCB6 original (faithful translation):
//    - `#pragma hdrstop` / `#pragma package(smart_init)` dropped (BCB-specific).
//    - IO routing mirrors myswitch.cpp / mysensor.cpp exactly:
//        eMotionNet || ePCI1203 -> MyLaneIO.IOBitOn/IOBitOff/IOOutBitStatus/
//                                  IOInputBit  (FULLY ACTIVE over the Sim HAL).
//        eISABase / ePCI1735U / ePLCbase raw-port free-funcs (myio.cpp) -> AI(W906-W3-AUDIT) 20260926 更正：W3-6 起改接 myio 的真函式（:36-39、:178、:246）；
//                                  myio 本身的 outportb／inportb 在 x64 上閘住（myio.cpp:162-176），所以那些舊式點讀回 0、寫入不做事。
//    - `extern HAlarm *Alarm; Alarm->Set/Clear` (alarm god-stack) -> local
//        SetAlarm/ClearAlarm sim stubs (no-op).  The On/Off/Push/Pop alarm   -> AI(W906-HALARM) 20260926 更正：已接上真的 HAlarm（halarm.h／HAlarm.cpp，:122-133），不再是空殼。
//        escalation only runs when SystemStart==true; SystemStart defaults
//        false in the sim build, so the stubs are not exercised by the canary.
//    - `fSmartDiagnostic->GetCyliderOnCount/OffCount` (SmartDiagnostic VCL form)
//        -> AI(W906-W3-AUDIT) 20260926 更正：W3-6b（db8cb324）已解開，改呼叫真的 fSmartDiagnostic（原寫 gated #if 0）。
//    - UpdateSimulateCompomentPosition / SetSimulateCompoment (VCL TControl /
//        TAnchorKind widget animation) -> no-op stubs (TODO(W7-UI)).
//    - All switch(Task) Push/Pop state-machine semantics preserved verbatim.
// =============================================================================
#include "mycylin.h"
#include "MyLaneIo.h"
#include "cmydef.h"     // TYPE_A/TYPE_B, eMotionNet/ePCI1203/eISABase/ePCI1735U/
                        // ePLCbase (via MachineType.h), SystemStart, bHandlerPause
#include "forms/fSmartDiagnostic.h"   // AI(W906-W3-6b) 20260925: fSmartDiagnostic (GetCyliderOn/OffCount) -- the four count calls below are live now

// ---------------------------------------------------------------------------
//  Stub: raw-port free-funcs (myio.cpp, W6) -- mirror myswitch.cpp's stubs.
//  BCB6: IOBitOn(Port,Bit)/IOBitOff(Port,Bit)/IOOutBitStatus(Port,Bit)/
//        IOInputBit(Port,Bit) (2-param raw outportb path).
// ---------------------------------------------------------------------------
extern void IOBitOn       (int port, int bit);  // AI(W906-W3-CYLIN) 20260924: 樁退役，接 myio.cpp 的真函式（同 myswitch.cpp:38；含 PLC 分支與兩參數 IdleCheckSafeDoorByCylinder 互鎖）。原始 port 在 x64 是 myio GATE (1)；HT9050 的點全是 1203，不走這條
extern void IOBitOff      (int port, int bit);  // AI(W906-W3-CYLIN) 20260924: 同上
extern bool IOOutBitStatus(int port, int bit);  // AI(W906-W3-CYLIN) 20260924: 同上（讀 myio 的輸出快取）
extern bool IOInputBit    (int port, int bit);  // AI(W906-W3-CYLIN) 20260924: 同上（同 mysensor.cpp:29）

class TMyCylinder Cylinder[MaxCylinderItem];

// BCB6: _fastcall TMyCylinder::TMyCylinder()
TMyCylinder::TMyCylinder()
{
    Enable=false;
    Status=false;
    bCylinderOn=false;
    OnTask=1;
    OffTask=1;
    OnOff=2;
    AlarmEnable=false;
    OnAlarmCode=0;
    OffAlarmCode=0;
    OnDelayTime=0;
    OffDelayTime=0;

    OutRingUse="";
    OutRing=0;
    OutIP=0;
    OutPort=0;
    OutBit=0;
    OutType=TYPE_A;                                                             // A or B Type
    OutISABase=eMotionNet;

    OffSenRingUse="";
    OnSenRingUse="";
    OnSenRing=0;
    OnSenIP=0;
    OnSenPort=0;
    OnSenBit=0;
    OnSenType=TYPE_A;
    OnSenISABase=eMotionNet;

    OffSenRing=0;
    OffSenIP=0;
    OffSenPort=0;
    OffSenBit=0;
    OffSenType=TYPE_A;
    OffSenISABase=eMotionNet;

    NeedFinishOnFunction=false;
    NeedFinishOffFunction=false;
    OnTryTask=0;
    OffTryTask=0;
    Change=false;
    CylinderName="";
    OnSensorName="";
    OffSensorName="";
//    iX=0;
//    iY=0;
    PTempWinCtrl=NULL;
    ISABase=eMotionNet;                                                         //Nickliu 20230306 add IO Use ISABase
    bCheckSafeDoor=false;                                                       //Steven 20230703 : Add Cylinder action check SafeDoor

    // AI(W6.0) 20260626: members not explicitly init'd in BCB6 ctor but read by
    // the Pre-Alarm surface; initialise defensively for the sim build.
    OnSenEnable=false;
    OffSenEnable=false;
    OnAlarmTime=0;
    OffAlarmTime=0;
    iOnLeft=0; iOnTop=0; iOffLeft=0; iOffTop=0;
    bCylPreAlarmByPassT=false;
    iOnOffCountAlarm=0; iOnCountAlarm=0; iOffCountAlarm=0;
    dOnTimeAlarm=0.0; dOffTimeAlarm=0.0; iTimeOutCountAlarm=0; iResetCount=0;

    //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
    //==>
    bfirst=true;
    bOnOffState=false;
    iOnOffCount=0;
    iTimeOutCount=0;
    dOnTime=0.0;
    dOffTime=0.0;
    sResetTime="";
    sListOnTime=new TStringList();
    sListOffTime=new TStringList();
    //<==
    //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
}
//---------------------------------------------------------------------------
#include "halarm.h"   // AI(W906-HALARM) 20260926: golden mycylin.cpp:83 `extern HAlarm  *Alarm;`（宣告在 halarm.h）。原註 AI(W6.0) 20260626: `extern HAlarm *Alarm; Alarm->Set/Clear` (alarm god-stack)
//   -> local sim stubs（原註，20260926 起過期：現在呼叫真的 Alarm->Set/Clear）.  Only reachable via SystemStart==true escalation, which
//   is false in the sim build.
void SetAlarm(int AlarmCode)
{
    if(Alarm) Alarm->Set(AlarmCode);   // golden mycylin.cpp:86 `Alarm->Set(AlarmCode);`  //AI(W906-HALARM) 20260926: RULINGS_20260926 第 16 條接上 golden HAlarm（HAlarm.cpp）。if(Alarm)：golden 在 TfMain::SetInitialData（main.cpp:22472）就建好 Alarm；移植樹在 wb_serve 開機建（W906_BootCreateAlarm），ctest／其他工具不建 ⇒ 那時是 NULL（PT §8 NULL 全域慣例：呼叫點加判斷）。只在 SystemStart 時由 Push/Pop 逾時觸發
}
//---------------------------------------------------------------------------
void ClearAlarm(int AlarmCode)
{
    if(Alarm) Alarm->Clear(AlarmCode); // golden mycylin.cpp:91 `Alarm->Clear(AlarmCode);`  //AI(W906-HALARM) 20260926: 同 SetAlarm（if(Alarm) 的理由相同）
}
//---------------------------------------------------------------------------
bool TMyCylinder::Reset()
{
    OnTask=1;
    OffTask=1;
    OnTryTask=0;
    OffTryTask=0;
    Change=false;
    return true;
}
//---------------------------------------------------------------------------
bool TMyCylinder::GetOutBit()
{
    if(Enable)
    {
        if(OutISABase==eMotionNet ||
           OutISABase==ePCI1203)                                                //Sam 20230724 : add PCI1203 IO support
        {
            return MyLaneIO.IOOutBitStatus(OutRing, OutIP, OutPort, OutBit, OutISABase, CylinderName);
        }
        else if(OutISABase==eISABase ||                                         //Nickliu 20230309 add Cylinder ISABase Type
                OutISABase==ePCI1735U ||
                OutISABase==ePLCbase)
        {
            return IOOutBitStatus(OutPort, OutBit);                             // AI(W906-W3-CYLIN) 20260924: myio 的真函式（見 :36-39）
        }
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyCylinder::OnStatus()
{
    bool InRet=false;                                                           // AI(W6.0): init (BCB6 left uninit)
    if(OnSenEnable)
    {
        if(OnSenISABase==eMotionNet ||
           OnSenISABase==ePCI1203)                                              //Sam 20230724 : add PCI1203 IO support
        {
            InRet=MyLaneIO.IOInputBit(OnSenRing, OnSenIP, OnSenPort, OnSenBit, OnSenISABase, OnSensorName);
        }
        else if(OnSenISABase==eISABase ||                                       //Nickliu 20230309 add Cylinder ISABase Type
                OnSenISABase==ePCI1735U ||
                OnSenISABase==ePLCbase)
        {
            InRet=IOInputBit(OnSenPort, OnSenBit);                              // AI(W906-W3-CYLIN) 20260924: myio 的真函式（見 :36-39）
        }
    }

    if(OnSenEnable==false)
        return false;
    else if(OnSenType==TYPE_A && InRet==1)
        return true;
    else if(OnSenType==TYPE_B && InRet==0)
        return true;
    else
        return false;
}
//---------------------------------------------------------------------------
bool TMyCylinder::OnSensor()
{
    if(Enable==false)
    {
        return true;
    }

    if(OnSenEnable)
    {
        return OnStatus();
    }
    else if(OffSenEnable)
    {
        return !OffStatus();
    }
    return true;
}
//---------------------------------------------------------------------------
bool TMyCylinder::OffSensor()
{
    if(Enable==false)
    {
        return true;
    }

    if(OffSenEnable)
    {
        return OffStatus();
    }
    else if(OnSenEnable)
    {
        return !OnStatus();
    }
    else
    {
        return true;
    }
}
//---------------------------------------------------------------------------
bool TMyCylinder::OffStatus()
{
    bool InRet=false;                                                           // AI(W6.0): init (BCB6 left uninit)

    if(OffSenEnable)
    {
        if(OffSenISABase==eMotionNet ||
           OffSenISABase==ePCI1203)                                             //Sam 20230724 : add PCI1203 IO support
        {
            InRet=MyLaneIO.IOInputBit(OffSenRing, OffSenIP, OffSenPort, OffSenBit, OffSenISABase, OffSensorName);
        }
        else if(OffSenISABase==eISABase ||                                      //Nickliu 20230309 add Cylinder ISABase Type
                OffSenISABase==ePCI1735U ||
                OffSenISABase==ePLCbase)
        {
            InRet=IOInputBit(OffSenPort, OffSenBit);                            // AI(W906-W3-CYLIN) 20260924: myio 的真函式（見 :36-39）
        }
    }

    if(OffSenEnable==false)
        return false;
    else if(OffSenType==TYPE_A && InRet==1)
        return true;
    else if(OffSenType==TYPE_B && InRet==0)
        return true;
    else
        return false;
}
//---------------------------------------------------------------------------
void TMyCylinder::OnSwitch()                                                    //Steven 20230721 : unified On/Off switch control
{
    if(Enable)
    {   { extern void (*g_W906PairedCoilHook)(const AnsiString&, bool); if(g_W906PairedCoilHook) g_W906PairedCoilHook(CylinderName, false); }   /* AI(W906-DUALCOIL9050) 20261008: HT9050 five-port three-position valve -- Push / On first releases the "<name>Off" coil (EOF) */
        if(OutType==TYPE_A)
        {
            if(OutISABase==eMotionNet ||
               OutISABase==ePCI1203)                                            //Sam 20230724 : add PCI1203 IO support
            {
                MyLaneIO.IOBitOn(OutRing, OutIP, OutPort, OutBit, OutISABase, CylinderName);
            }
            else if(OutISABase==eISABase ||                                     //Nickliu 20230309 add Cylinder ISABase Type
                    OutISABase==ePCI1735U ||
                    OutISABase==ePLCbase)
            {
                IOBitOn(OutPort, OutBit);                                       // AI(W906-W3-CYLIN) 20260924: myio 的真函式（見 :36-39）
            }
//AI(W906-W3-6b) 20260925: gate lifted -- vclcompat TStringGrid now has BCB6 grids.pas semantics (read outside RowCount -> "", no throw), which was the only reason left (W3-6b)
            fSmartDiagnostic->GetCyliderOnCount(CylinderName);
//AI(W906-W3-6b) 20260925: (end of lifted gate)
        }
        else
        {
            if(OutISABase==eMotionNet ||
               OutISABase==ePCI1203)                                            //Sam 20230724 : add PCI1203 IO support
            {
                MyLaneIO.IOBitOff(OutRing, OutIP, OutPort, OutBit, OutISABase, CylinderName);
            }
            else if(OutISABase==eISABase ||                                     //Nickliu 20230309 add Cylinder ISABase Type
                    OutISABase==ePCI1735U ||
                    OutISABase==ePLCbase)
            {
                IOBitOff(OutPort, OutBit);                                      // AI(W906-W3-CYLIN) 20260924: myio 的真函式（見 :36-39）
            }
//AI(W906-W3-6b) 20260925: gate lifted -- vclcompat TStringGrid now has BCB6 grids.pas semantics (read outside RowCount -> "", no throw), which was the only reason left (W3-6b)
            fSmartDiagnostic->GetCyliderOffCount(CylinderName);
//AI(W906-W3-6b) 20260925: (end of lifted gate)
        }
        AddOnCount();                                                           //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
    }
}
//---------------------------------------------------------------------------
void TMyCylinder::OffSwitch()                                                   //Steven 20230721 : unified On/Off switch control
{
    if(Enable)
    {
        if(OutType==TYPE_B)
        {
            if(OutISABase==eMotionNet ||
               OutISABase==ePCI1203)                                            //Sam 20230724 : add PCI1203 IO support
            {
                MyLaneIO.IOBitOn(OutRing, OutIP, OutPort, OutBit, OutISABase, CylinderName);
            }
            else if(OutISABase==eISABase ||                                     //Nickliu 20230309 add Cylinder ISABase Type
                    OutISABase==ePCI1735U ||
                    OutISABase==ePLCbase)
            {
                IOBitOn(OutPort, OutBit);                                       // AI(W906-W3-CYLIN) 20260924: myio 的真函式（見 :36-39）
            }
//AI(W906-W3-6b) 20260925: gate lifted -- vclcompat TStringGrid now has BCB6 grids.pas semantics (read outside RowCount -> "", no throw), which was the only reason left (W3-6b)
            fSmartDiagnostic->GetCyliderOnCount(CylinderName);
//AI(W906-W3-6b) 20260925: (end of lifted gate)
        }
        else
        {
            if(OutISABase==eMotionNet ||
               OutISABase==ePCI1203)                                            //Sam 20230724 : add PCI1203 IO support
            {
                MyLaneIO.IOBitOff(OutRing, OutIP, OutPort, OutBit, OutISABase, CylinderName);
            }
            else if(OutISABase==eISABase ||                                     //Nickliu 20230309 add Cylinder ISABase Type
                    OutISABase==ePCI1735U ||
                    OutISABase==ePLCbase)
            {
                IOBitOff(OutPort, OutBit);                                      // AI(W906-W3-CYLIN) 20260924: myio 的真函式（見 :36-39）
            }
//AI(W906-W3-6b) 20260925: gate lifted -- vclcompat TStringGrid now has BCB6 grids.pas semantics (read outside RowCount -> "", no throw), which was the only reason left (W3-6b)
            fSmartDiagnostic->GetCyliderOffCount(CylinderName);
//AI(W906-W3-6b) 20260925: (end of lifted gate)
        }
        AddOffCount();  { extern void (*g_W906PairedCoilHook)(const AnsiString&, bool); if(g_W906PairedCoilHook) g_W906PairedCoilHook(CylinderName, true); }   /* AI(W906-DUALCOIL9050) 20261008: HT9050 -- Pop / Off energises the "<name>Off" coil after the On coil is released (EOF) */  //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
    }
}
//---------------------------------------------------------------------------
void TMyCylinder::On()                                                          // no delay,no alarm
{
    Status=true;
    Change=true;
    bCylinderOn=true;
    OnTask=1;
    OnSwitch();
    UpdateSimulateCompomentPosition();
}
//---------------------------------------------------------------------------
void TMyCylinder::Off()                                                         // no delay,no alarm
{
    Status=false;
    Change=false;
    bCylinderOn=false;
    OffTask=1;
    OffSwitch();
    UpdateSimulateCompomentPosition();
}
//---------------------------------------------------------------------------
bool TMyCylinder::Push()
{
    bool InRet;
    Change=true;
    int &Task=OnTask;
    OnSwitch();

    if(Task==1 || Task==2)
    {
        if(OnSenEnable)                                                         //  has install onsensor
        {
            if(OnAlarmTime==0)
            {
                InRet=OnStatus();
                if(InRet)                                                       // ok
                {
                    Task=100;                                                   // do on delay
                }
                else
                {
                    Status=false;
                    Task=1;
                    if(SystemStart)
                    {
                        OnTryTask++;
                        if(OnTryTask>=2)
                        {
                            iTimeOutCount++;                                    //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                            SetAlarm(OnAlarmCode);
                            OnTryTask=0;
                            Task=1;
                        }
                        else
                        {
                            Task=2;
                        }
                    }
                    return false;
                }
            }
            else
            {
                if(OnStatus())                                                  // already on
                {
                    Task=100;
                }
                else
                {
                    TOn.Set0_1SecAndOn(OnAlarmTime);
                    Task=50;
                }
            }
        }
        else
        {
            Task=100;                                                           //need delay
        }
    }

    if(Task==50)
    {
        InRet=OnStatus();
        if(InRet)
        {
            Task=100;                                                           // do on delay
        }
        else
        {
            if(bHandlerPause)                                                   //Steven 20190123 : record Handler paused, reset Timer
            {
                TOn.Set0_1SecAndOn(OnAlarmTime);
                tOnOff.LatchCycleTime(true);                                    //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                return false;
            }

            if(TOn.Off())
            {
                Status=false;
                Task=1;
                if(SystemStart)
                {
                    OnTryTask++;
                    if(OnTryTask>=2)
                    {
                        iTimeOutCount++;                                        //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                        SetAlarm(OnAlarmCode);
                        OnTryTask=0;
                    }
                    else
                    {
                        Task=2;
                    }
                }
                return false;
            }
            else
            {
                return false;
            }
        }
    }

    if(Task>=100)
    {
        switch(Task)
        {
            case 100:
                LatchOnffTime();                                                //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                if(OnDelayTime==0)
                {
                    Task=1;
                    Status=true;
                    ClearAlarm(OnAlarmCode);
                    OnTryTask=0;
                    bCylinderOn=true;
                    AddOnCount(true);                                           //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                    return true;
                }
                else
                {
                    TOnDelay.Set0_1SecAndOn(OnDelayTime);
                    Task=101;
                }
            case 101:
                if(TOnDelay.Off())
                {
                    Status=true;
                    ClearAlarm(OnAlarmCode);
                    Task=1;
                    OnTryTask=0;
                    bCylinderOn=true;
                    AddOnCount(true);                                           //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                    UpdateSimulateCompomentPosition();
                    return true;
                }
                else
                {
                    return false;
                }
        }
    }
    ClearAlarm(OnAlarmCode);
    Task=1;
    OnTryTask=0;
    bCylinderOn=true;
    UpdateSimulateCompomentPosition();
    return true;
}
//---------------------------------------------------------------------------
bool TMyCylinder::Pop()
{
    bool InRet;
    Change=false;
    int &Task=OffTask;
    OffSwitch();

    if(Task==1 || Task==2)
    {
        if(OffSenEnable)                                                        //  has install onsensor
        {
            if(OffAlarmTime==0)
            {
                InRet=OffStatus();
                if(InRet)
                {
                    Task=100;                                                   // do on delay
                }
                else
                {
                    Status=true;

                    if(SystemStart)
                    {
                        OffTryTask++;
                        if(OffTryTask>=2)
                        {
                           iTimeOutCount++;                                     //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                           SetAlarm(OffAlarmCode);
                           OffTryTask=0;
                           Task=1;
                        }
                        else
                        {
                           Task=2;
                        }
                    }
                    return false;
                }
            }
            else
            {
                if(OffStatus())                                                 //already off
                {
                    Task=100;
                }
                else
                {
                    TOff.Set0_1SecAndOn(OffAlarmTime);
                    Task=50;
                }
            }
        }
        else
        {
            Task=100;                                                           //need delay
        }
    }

    if(Task==50)
    {
        InRet=OffStatus();
        if(InRet)
        {
            Task=100;                                                           // do on delay
        }
        else
        {
            if(bHandlerPause)                                                   //Steven 20190123 : record Handler paused, reset Timer
            {
                TOff.Set0_1SecAndOn(OffAlarmTime);
                tOnOff.LatchCycleTime(true);                                    //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                return false;
            }

            if(TOff.Off())
            {
                Status=true;
                Task=1;
                if(SystemStart)
                {
                    OffTryTask++;
                    if(OffTryTask>=2)
                    {
                       iTimeOutCount++;                                         //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                       SetAlarm(OffAlarmCode);
                       OffTryTask=0;
                    }
                    else
                    {
                       Task=2;
                    }
                }
                return false;
            }
            else
            {
                return false;
            }
        }
    }

    if(Task>=100)
    {
        switch(Task)
        {
            case 100:
                LatchOnffTime();                                                //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
                if(OffDelayTime==0)
                {
                    Status=false;
                    ClearAlarm(OffAlarmCode);
                    Task=1;
                    OffTryTask=0;
                    bCylinderOn=false;
                    return true;
                }
                else
                {
                    TOffDelay.Set0_1SecAndOn(OffDelayTime);
                    Task=101;
                }
            case 101:
                if(TOffDelay.Off())
                {
                    Task=1;
                    Status=false;
                    ClearAlarm(OffAlarmCode);
                    bCylinderOn=false;
                    UpdateSimulateCompomentPosition();
                    OffTryTask=0;
                    return true;
                }
                else
                {
                    return false;
                }
        }
    }
    Status=false;
    ClearAlarm(OffAlarmCode);
    Task=1;
    OffTryTask=0;
    bCylinderOn=false;
    UpdateSimulateCompomentPosition();
    return true;
}
//---------------------------------------------------------------------------
bool TMyCylinder::FinshFullMotion()
{
    if(OnTask!=1 || OffTask!=1)
        return false;
    return true;
}
//---------------------------------------------------------------------------
bool TMyCylinder::FinshOnMotion()
{
    if(OnTask!=1 || OffTask!=1)
        return false;
    return true;
}
//---------------------------------------------------------------------------
bool TMyCylinder::FinshOffMotion()
{
    if(OnTask!=1 || OffTask!=1)
        return false;
    return true;
}
//---------------------------------------------------------------------------
void TMyCylinder::ResetOnAlarmTime()
{
    OnTask=2;
}
//---------------------------------------------------------------------------
void TMyCylinder::ResetOffAlarmTime()
{
    OffTask=2;
}
//---------------------------------------------------------------------------
void TMyCylinder::ResetOnDelayTime()
{
    OnTask=2;
}
//---------------------------------------------------------------------------
void TMyCylinder::ResetOffDelayTime()
{
    OffTask=2;
}
//---------------------------------------------------------------------------
// AI(W6.0) 20260626: UpdateSimulateCompomentPosition / SetSimulateCompoment were
//   VCL TControl widget animation (PTempWinCtrl->Left/Top assignments).  No VCL
//   TControl/TAnchorKind in vclcompat -> no-op stubs.  PTempWinCtrl stays NULL.
void TMyCylinder::UpdateSimulateCompomentPosition()                             // Simulate run //
{
#if 0   // AI(W906-W3-CYLIN) 20260924 維持閘住：模擬模式下移動畫面元件的純動畫（PTempWinCtrl->Left/Top），在 web 架構下歸瀏覽器（pt-wave-loop 陷阱 #6），C++ 不補假元件。原註：TODO(W7-UI): restore when VCL TControl is available
    if(PTempWinCtrl!=NULL)
    {
        if(bCylinderOn)
        {
            PTempWinCtrl->Left=iOnLeft;
            PTempWinCtrl->Top =iOnTop;
        }
        else
        {
            PTempWinCtrl->Left=iOffLeft;
            PTempWinCtrl->Top =iOffTop;
        }
    }
#endif
}
//---------------------------------------------------------------------------
void TMyCylinder::SetSimulateCompoment(void * /*PCtrl*/, int /*Alignment*/, int /*simuStart*/, int /*simuEnd*/)
{
#if 0   // AI(W906-W3-CYLIN) 20260924 維持閘住：模擬模式下移動畫面元件的純動畫（PTempWinCtrl->Left/Top），在 web 架構下歸瀏覽器（pt-wave-loop 陷阱 #6），C++ 不補假元件。原註：TODO(W7-UI): restore when VCL TControl / TAnchorKind is available
    PTempWinCtrl=dynamic_cast<TControl *> (PCtrl);
    if(PTempWinCtrl!=NULL)
    {
        iOnLeft =(Alignment==akLeft || Alignment==akRight )?PTempWinCtrl->Left:simuStart;
        iOffLeft=(Alignment==akLeft || Alignment==akRight )?PTempWinCtrl->Left:simuEnd;
        iOnTop  =(Alignment==akTop  || Alignment==akBottom)?PTempWinCtrl->Top:simuStart;
        iOffTop =(Alignment==akTop  || Alignment==akBottom)?PTempWinCtrl->Top:simuEnd;
        UpdateSimulateCompomentPosition();
    }
#endif
}
//---------------------------------------------------------------------------
//Sam 20230516 : Pre Alarm Cylinder
//==>
void TMyCylinder::AddOnCount(bool bResetTimer)
{
    if(bOnOffState!=true || bfirst)
    {
        bfirst=false;
        if(bResetTimer && OnSenEnable)
            tOnOff.LatchCycleTime(true);
        bOnOffState=true;
        iOnOffCount++;
    }
}
//---------------------------------------------------------------------------
void TMyCylinder::AddOffCount(bool bResetTimer)
{
    if(bOnOffState!=false || bfirst)
    {
        bfirst=false;
        if(bResetTimer && OffSenEnable)
            tOnOff.LatchCycleTime(true);
        bOnOffState=false;
        iOnOffCount++;
    }
}
//---------------------------------------------------------------------------
void TMyCylinder::LatchOnffTime()
{
    if(bOnOffState)
    {
        if(OnSenEnable)
        {
            dOnTime=(double)tOnOff.LatchCycleTime()/1000.0;
            AddOnTime(dOnTime);
        }
    }
    else
    {
        if(OffSenEnable)
        {
            dOffTime=(double)tOnOff.LatchCycleTime()/1000.0;
            AddOffTime(dOffTime);
        }
    }
}
//---------------------------------------------------------------------------
void TMyCylinder::AddOnTime(double dOnTimeArg)
{
    AnsiString sOnTime="";
    if(sListOnTime->Count>=5)
        sListOnTime->Delete(sListOnTime->Count-1);
    sOnTime.sprintf("%2.3f",dOnTimeArg);
    sListOnTime->Insert(0,sOnTime);
}
//---------------------------------------------------------------------------
void TMyCylinder::AddOffTime(double dOffTimeArg)
{
    AnsiString sOffTime="";
    if(sListOffTime->Count>=5)
        sListOffTime->Delete(sListOffTime->Count-1);
    sOffTime.sprintf("%2.3f",dOffTimeArg);
    sListOffTime->Insert(0,sOffTime);
}
//---------------------------------------------------------------------------
AnsiString TMyCylinder::GetOnTime()
{
    AnsiString sOnTime="NA";
    if(OnSenEnable && bCylPreAlarmByPassT==false)
        sOnTime.sprintf("%2.3f",dOnTime);
    return sOnTime;
}
//---------------------------------------------------------------------------
AnsiString TMyCylinder::GetOffTime()
{
    AnsiString sOffTime="NA";
    if(OffSenEnable && bCylPreAlarmByPassT==false)
        sOffTime.sprintf("%2.3f",dOffTime);
    return  sOffTime;
}
//---------------------------------------------------------------------------
AnsiString TMyCylinder::GetOnTimeAvg()
{
    int iCount=0;
    double dSum=0.0,dAvg=0.0;
    AnsiString sAvg="NA";
    for(int i=0; i<sListOnTime->Count; i++)
    {
        dSum+=atof(sListOnTime->GetString(i).c_str());                          // AI(W6.0): BCB6 Strings[i].c_str() -> GetString(i) (vclcompat proxy has no c_str)
        iCount++;
    }

    if(iCount>0)
        dAvg=dSum/(double)iCount;
    if(OnSenEnable && bCylPreAlarmByPassT==false)
        sAvg.sprintf("%2.3f",dAvg);
    return sAvg;
}
//---------------------------------------------------------------------------
AnsiString TMyCylinder::GetOffTimeAvg()
{
    int iCount=0;
    double dSum=0.0,dAvg=0.0;
    AnsiString sAvg="NA";
    for(int i=0; i<sListOffTime->Count; i++)
    {
        dSum+=atof(sListOffTime->GetString(i).c_str());                         // AI(W6.0): BCB6 Strings[i].c_str() -> GetString(i) (vclcompat proxy has no c_str)
        iCount++;
    }

    if(iCount>0)
        dAvg=dSum/(double)iCount;
    if(OffSenEnable && bCylPreAlarmByPassT==false)
        sAvg.sprintf("%2.3f",dAvg);
    return sAvg;
}
//---------------------------------------------------------------------------
AnsiString TMyCylinder::GetOnTimeAlarm()
{
    AnsiString sOnTimeAlarm="NA";
    if(OnSenEnable && bCylPreAlarmByPassT==false)
        sOnTimeAlarm.sprintf("%2.3f",dOnTimeAlarm);
    return sOnTimeAlarm;
}
//---------------------------------------------------------------------------
AnsiString TMyCylinder::GetOffTimeAlarm()
{
    AnsiString sOffTimeAlarm="NA";
    if(OffSenEnable && bCylPreAlarmByPassT==false)
        sOffTimeAlarm.sprintf("%2.3f",dOffTimeAlarm);
    return sOffTimeAlarm;
}
//<==
//Sam 20230516 : Pre Alarm Cylinder
//---------------------------------------------------------------------------
// AI(W6.0) 20260626: InitialCylinderName (cinitial god-stack) not needed by the
//   W6.1 canary; provide a no-op so the header decl links.  TODO(W6.x).
#if 0   // PT-W7a RETIRED (InitialCylinderName)
//AI(ht9045-v906) 20260810: PT-W7a -- RETIRED. cinitial.cpp now holds golden's real body (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0810_w7a.
void InitialCylinderName()
{
}
#endif
//---------------------------------------------------------------------------
//AI(W906-F9050-BD) 20261004: W906_CylinderRowsShown (declared in mycylin.h).  Not golden, not 910: 910 sizes the SmartDiagnostic
//  grid with MaxCylinderItem (321) for every model; this tree keeps the pre-9050 295 rows unless MachineTypeChoice==Type_HT9050,
//  so that grid and system/SmartDiagnosticRecord.txt do not change while 9050GPIB decodes as Type_HT9046_LS (database.cpp:517).
//  The type-switch batch decides whether to drop this helper (SmartDiagnostic.cpp then goes back to MaxCylinderItem, as 910).
int W906_CylinderRowsShown()
{
    return (MachineTypeChoice==Type_HT9050) ? MaxCylinderItem : 295;
}
//---------------------------------------------------------------------------
//AI(W906-DUALCOIL9050) 20261008: EastSun 1008 "ht9050 的汽缸都是用五口三位的 ... Cylinder[C_LoaderEdgePush].Pop() 會等於
//  Cylinder[C_LoaderEdgePush].Off() && Cylinder[C_LoaderEdgePushOff].On()，Push() 則與 Pop 相反". OnSwitch (On / Push) calls this
//  with false before it energises the On coil, OffSwitch (Off / Pop) with true after it releases the On coil. Set by
//  JsonBridge/IoBtnPanelClick.cpp (wb_serve; it owns the IO_Table lookup, which test targets that compile this file alone lack);
//  0 = nothing (ctests, other machines' builds without the bridge). The hook itself decides HT9050 and whether "<name>Off" exists.
void (*g_W906PairedCoilHook)(const AnsiString&, bool) = 0;
//---------------------------------------------------------------------------
