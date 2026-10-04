// =============================================================================
//  mycylin.h  --  TMyCylinder per-cylinder 2-switch / 2-sensor controller
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/mycylin.h
//  Translation wave: W6.0 (substrate gap closed for the W6.1 Empty-tray canary)
//  Translator: AI(W6.0-SCAFFOLD) 20260626
//
//  RATIONALE
//  ---------
//  The IO HAL note in CMakeLists.txt (W4-IO) explicitly DEFERRED TMyCylinder to
//  "W4-IO-part2 / W6" because Push/Pop/On/Off pull the Alarm / SystemStart /
//  bHandlerPause / TControl god-stack.  W6.1's Empty-tray canary (asendic_Empty)
//  is the first consumer of Cylinder[], so this wave closes that gap.
//
//  WHAT IS TRANSLATED AND ACTIVE (W6.0):
//    - All data members + the public surface the canary uses:
//      On / Off / Push / Pop / OnStatus / OffStatus / Enable (7 surfaces),
//      plus Reset / OnSensor / OffSensor / GetOutBit / Finsh*Motion.
//    - Hardware routing goes through the Sim IO HAL exactly like myswitch /
//      mysensor (eMotionNet||ePCI1203 -> MyLaneIO; eISABase/ePCI1735U/ePLCbase
//      raw-port -> gated stub).
//    - The Pre-Alarm time-tracking surface (AddOnTime / GetOnTimeAvg / ...) is
//      kept verbatim -- it only needs TStringList (present in vclcompat).
//
//  GATED (#if 0 // TODO(W7-UI)):
//    - PTempWinCtrl (TControl*) + SetSimulateCompoment(TObject*,TAnchorKind,...)
//      + UpdateSimulateCompomentPosition: VCL TControl/TAnchorKind not in
//      vclcompat.  The On/Off/Push/Pop bodies call a no-op stub in their place.
//
//  Key BCB6 changes:
//    - `_fastcall TMyCylinder::TMyCylinder()` -> unqualified `TMyCylinder()`.
//    - AnsiString / TStringList from vclcompat (via myTimer.h umbrella).
//    - MaxCylinderItem preserved (295).
// =============================================================================
//---------------------------------------------------------------------------
#ifndef mycylinH
#define mycylinH
//---------------------------------------------------------------------------

#include "vclcompat/vcl_compat.h"   // AnsiString, TStringList, __fastcall no-op
#include "myTimer.h"                // TQPF_Timer
#define MaxCylinderItem 295

class TMyCylinder{
    private:
        int OnTask;
        int OffTask;
        int OnOff;
        int OnTryTask;
        int OffTryTask;
        int iOnLeft;
        int iOnTop;
        int iOffLeft;
        int iOffTop;
        void OnSwitch();                                                        //Steven 20230721 : unified On/Off switch control
        void OffSwitch();                                                       //Steven 20230721 : unified On/Off switch control

    protected:
        // AI(W6.0) 20260626: was `TControl *PTempWinCtrl;` (VCL widget) -- gated.
        void *PTempWinCtrl;     // NULL in sim build; SetSimulateCompoment is a no-op
    public:
        TMyCylinder();          // BCB6: _fastcall TMyCylinder::TMyCylinder()
        AnsiString CylinderName;
        AnsiString OnSensorName;
        AnsiString OffSensorName;
        TQPF_Timer  TOn;
        TQPF_Timer  TOff;
        TQPF_Timer  TOnDelay;
        TQPF_Timer  TOffDelay;
        bool    NeedFinishOnFunction;
        bool    NeedFinishOffFunction;
        bool    Enable;
        bool    Status;
        bool    Change;
        bool    AlarmEnable;
        int     OnAlarmCode;
        int     OffAlarmCode;
        int     OnAlarmTime;
        int     OffAlarmTime;

        int     OnDelayTime;
        int     OffDelayTime;

        AnsiString OutRingUse;
        int     OutRing;
        int     OutIP;
        int     OutPort;
        int     OutBit;
        int     OutType;                                                        // A or B Type
        int     OutISABase;

        AnsiString OnSenRingUse;
        int     OnSenRing;
        int     OnSenIP;
        int     OnSenPort;
        int     OnSenBit;
        int     OnSenType;
        int     OnSenISABase;
        bool    OnSenEnable;

        AnsiString OffSenRingUse;
        int     OffSenRing;
        int     OffSenIP;
        int     OffSenPort;
        int     OffSenBit;
        int     OffSenType;
        int     OffSenISABase;
        bool    OffSenEnable;

        bool    Push();
        bool    Pop();
        void    On();
        void    Off();

        bool    bCylinderOn;                                                    // ben edit //
        bool    Reset();
        bool    OnStatus();
        bool    OffStatus();
        bool    OnSensor();
        bool    OffSensor();
        bool    GetOutBit();

        bool    FinshFullMotion();
        bool    FinshOnMotion();
        // AI(W6.0) 20260626: was SetSimulateCompoment(TObject*,TAnchorKind,...)
        //   + UpdateSimulateCompomentPosition() (VCL TControl/TAnchorKind).
        //   Kept as no-op stubs so the On/Off/Push/Pop bodies compile unchanged.
        void    SetSimulateCompoment(void *PCtrl, int Alignment, int simuStart, int simuEnd);
        void    UpdateSimulateCompomentPosition();
        bool FinshOffMotion();
        void ResetOnAlarmTime();
        void ResetOffAlarmTime();
        void ResetOnDelayTime();
        void ResetOffDelayTime();
//        int  iX;
//        int  iY;
        int ISABase;                                                            //Nickliu 20230309 add Cylinder ISABase Type
        bool bCheckSafeDoor;                                                    //Steven 20230703 : Add Cylinder action check SafeDoor
        //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
        //==>
        TQPF_Timer tOnOff;
        bool bfirst;
        void AddOnCount(bool bResetTimer=false);
        void AddOffCount(bool bResetTimer=false);
        void LatchOnffTime();
        bool bOnOffState;
        int iOnOffCount;        // accumulated On + Off count
        double dOnTime;         // current On Time (needs OnSenEnable)
        double dOffTime;        // current Off Time (needs OffSenEnable)
        int iTimeOutCount;      // accumulated Push/Pop time-out count
        bool bCylPreAlarmByPassT;

        TStringList *sListOnTime;
        TStringList *sListOffTime;
        void AddOnTime(double dOnTime);
        void AddOffTime(double dOffTime);
        AnsiString GetOnTime();
        AnsiString GetOffTime();
        AnsiString GetOnTimeAvg();
        AnsiString GetOffTimeAvg();
        AnsiString GetOnTimeAlarm();
        AnsiString GetOffTimeAlarm();

        int iOnOffCountAlarm;
        int iOnCountAlarm;
        int iOffCountAlarm;
        double dOnTimeAlarm;
        double dOffTimeAlarm;
        int iTimeOutCountAlarm;
        int iResetCount;
        AnsiString sResetTime;
        //<==
        //Eastsun 20260521 integrate //Sam 20230516 : Pre Alarm Cylinder
};

extern class TMyCylinder Cylinder[MaxCylinderItem];
void InitialCylinderName();
#endif
