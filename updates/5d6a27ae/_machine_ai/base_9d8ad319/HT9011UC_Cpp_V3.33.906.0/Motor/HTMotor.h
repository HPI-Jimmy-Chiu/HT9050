// =============================================================================
//  Motor/HTMotor.h  -- HAL motor port (abstract base)
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/Motor/HTMotor.h
//  Translation wave: W4 (HAL motor layer)
//  Translator: AI(W4) 20260626
//
//  Key changes vs. BCB6 original:
//    - Dropped `#include "halarm.h"` (dead include; halarm.h = VCL TComponent
//      class, not used by any HTMotor symbol -- verified: no HAlarm/SystemNG/
//      PopUpAlarm refs in HTMotor.h or HTMotor.cpp).
//    - `byte` supplied by windows.h (included by MachineDefine.h -> vclcompat).
//    - `BYTE` supplied by windows.h (SetGroup/LineNMove params).
//    - `PACKAGE` not present in this header (it was only in halarm.h).
//    - All virtual methods and their default bodies translated faithfully.
//    - Non-virtual concrete declarations translated faithfully (bodies in
//      HTMotor.cpp).
//    - MotorType extern consts declared here, defined in HTMotor.cpp.
// =============================================================================
#ifndef HTMotorH
#define HTMotorH

#include "vclcompat/vcl_compat.h"   // neutralize __fastcall/__published/etc.
#include <windows.h>                // BYTE, bool (via windef.h)

// ---------------------------------------------------------------------------
//  Sentinel value used by MotorMovePosition return codes
// ---------------------------------------------------------------------------
#define ALM_MOTOR_MOVE  55555

// ---------------------------------------------------------------------------
//  Function-pointer typedef for safe-door check callback
//  (2015.01.15, Joye: Safe door check)
// ---------------------------------------------------------------------------
typedef bool (*PF_CHECK)(void);

// ---------------------------------------------------------------------------
//  LED / status index enum (used by TMyMotor::Led[] and ScanMotorStatus)
// ---------------------------------------------------------------------------
enum {
    iCwLed          = 0,
    iHomeLed        = 1,
    iCcwLed         = 2,
    iEmgLed         = 3,
    iAlarmLed       = 4,
    iSoftcwLed      = 5,
    iSoftccwLed     = 6,
    iServoalarmLed  = 7,
    iInposLed       = 8,
    iServoOn        = 9
};

// ---------------------------------------------------------------------------
//  MotorType constants (defined in HTMotor.cpp)
// ---------------------------------------------------------------------------
extern const int Step_Motor;
extern const int Servo_Motor;
extern const int Rotate_Motor;
extern const int YASKAWA_Servo_Motor;
extern const int YASKAWA_Liner_Motor;
extern const int Step_Motor_Oriental;

// ---------------------------------------------------------------------------
//  HTMotor -- abstract hardware-abstraction base for all motor brands
//
//  Virtual methods:
//    - All have inline default bodies that return 0 / false / no-op.
//    - Overriding subclasses (brand drivers) provide real implementations.
//    - TMySimMotor (mySimMotor.h) overrides the offline-verifiable subset.
//
//  Non-virtual concrete members:
//    - Ctor sets default field values (see HTMotor.cpp).
//    - ReadSpeed/ReadRange/ReadRate/ReadInitSpeed/ReadAcc/ReadDec return
//      the corresponding protected members.
//    - ReadEncoderPos() = ReadEnCoderRealPos() * GearRatio (concrete).
//    - SetHomeobjectTask stores iHomeObjectTask.
//    - CheckIsSafeDoorOpen uses MotorIdleSafeDoorCheck callback or Enable.
// ---------------------------------------------------------------------------
class HTMotor
{
private:

protected:
    unsigned int    iSpeed;
    unsigned int    Rate;
    unsigned int    Range;
    int             iHomeObjectTask;
    double          dAcc;
    double          dDec;
    double          dAccDataBase;
    double          dDecDataBase;
    byte            bAxisID;            // Jimmychiu 20220926: ADD MC88X1

public:
    HTMotor();
    virtual ~HTMotor();

    // Public data members (set by cinitial / brand drivers)
    unsigned int    Address;
    unsigned int    iBoardID;
    unsigned int    iPortID;
    unsigned int    PHomeHighSpeed;
    unsigned int    PHomeLowSpeed;
    unsigned int    PJogHighSpeed;
    unsigned int    PJogLowSpeed;
    unsigned int    InitSpeed;
    bool            PServoAlarmOn;
    bool            Enable;
    bool            Direction;
    bool            HomeDirection;
    int             MotorType;          // Steven 20170505: MotorType bool -> int
    bool            bSensorType;
    bool            bLimitLogic;
    bool            bIn1Logic;
    double          GearRatio;
    int             PSoftLimitP;
    int             PSoftLimitN;
    int             LastHomePos;
    int             EncoderType;
    int             iHomePitch;

    // Non-virtual concrete accessors (defined in HTMotor.cpp)
    unsigned int    ReadSpeed();
    unsigned int    ReadInitSpeed();
    unsigned int    ReadRate();
    unsigned int    ReadRange();
    double          ReadAcc();
    double          ReadDec();
    int             ReadEncoderPos();       // = ReadEnCoderRealPos() * GearRatio
    void            SetHomeobjectTask(int Task);

    // Misc public data
    char            ErrorString[256];
    short           PulseEdgeCnt[8][2];

    // -----------------------------------------------------------------------
    //  Virtual methods -- motion
    // -----------------------------------------------------------------------
    virtual void    Stop()                              {}
    virtual bool    JogP()                              { return false; }
    virtual bool    JogN()                              { return false; }
    virtual int     ReadPos()                           { return 0; }
    virtual bool    MoveToPos(int Tar)                  { return false; }
    virtual bool    MoveToPosShortDistance(int Tar)     { return MoveToPos(Tar); }
    virtual bool    MoveToPosShortDisSlowSP(int Tar)    { return MoveToPos(Tar); }
    virtual bool    HomeObject()                        { return false; }
    virtual bool    HomeFlag(void)                      { return false; }
    virtual bool    GetAlarm(void)                      { return false; }
    virtual bool    MotionDone()                        { return false; }
    virtual void    DecStop(void)                       {}
    virtual void    SetArmMaxSpeed()                    {}

    // -----------------------------------------------------------------------
    //  Virtual methods -- speed / accel
    // -----------------------------------------------------------------------
    virtual void    SetSpeed(unsigned int x, bool bSetJog = false) {}   // RogerYang 20250729: Add for 9046AU
    virtual void    SetSpeedShortDistance(unsigned int x)  { SetSpeed(x); }
    virtual void    SetSpeedShortDisSlowSP(unsigned int x) { SetSpeed(x); }
    virtual void    SetInitSpeed(unsigned int x)        {}
    virtual void    SetRange(unsigned int a)            {}
    virtual void    SetRate(unsigned int a)             {}
    virtual void    SetAcc(double a)                    { dAcc = a; }
    virtual void    SetDec(double a)                    { dDec = a; }
    virtual void    SetAccDataBase(double a)            { dAccDataBase = a; }
    virtual void    SetDecDataBase(double a)            { dDecDataBase = a; }
    virtual double  GetAccDataBase()                    { return dAccDataBase; }
    virtual double  GetDecDataBase()                    { return dDecDataBase; }

    // -----------------------------------------------------------------------
    //  Virtual methods -- init / servo
    // -----------------------------------------------------------------------
    virtual int     InitMotor(int IoAddress)            { return 0; }
    virtual void    SetServoAlarmOn(bool Value)         {}
    virtual void    ServerOnOff(bool bStatus)           {}
    virtual void    SetServoOn(bool IsOn)               {}
    virtual bool    ResetPos(int Pulse = 0)             { return false; }
    virtual void    SoftLimitEnable(bool bFlag)         {}
    virtual void    SetSoftLimit(int iPLimit, int iNLimit) {}

    // -----------------------------------------------------------------------
    //  Virtual methods -- position / encoder
    // -----------------------------------------------------------------------
    virtual int     ReadRealPos()                       { return 0; }
    virtual int     ReadEnCoderRealPos()                { return 0; }
    virtual int     SetCommand(int p)                   { return 0; }
    virtual int     SetPosition(int p)                  { return 0; }

    // -----------------------------------------------------------------------
    //  Virtual methods -- IO / status
    // -----------------------------------------------------------------------
    virtual void    MotOutputOn(int iOutPort)           {}
    virtual void    MotOutputOff(int iOutPort)          {}
    virtual void    MotInputStatus(bool *bInputPort)    {}
    virtual void    EnableTrigger(bool bFlag, int iMode, long lValue) {}
    virtual void    ScanMotorStatus(bool *Led)          {}
    virtual bool    LinearAxisMoveTo(int iPortID[8], long lPos[8], bool bFlag)
                                                        { return false; }

    // -----------------------------------------------------------------------
    //  Virtual methods -- latch
    // -----------------------------------------------------------------------
    virtual void    SetEnableLatch(bool a)              {}
    virtual void    ResetLatch()                        {}
    virtual int     GetLatchTotalLen()                  { return 0; }
    virtual int     GetLatchBuffer(int LatchInputNo,
                                   unsigned short *AxisCounterNo,
                                   long *LatchDataTable)        { return 0; }
    virtual int     GetLatchBuffer(int LatchInputNo,
                                   short *AxisCounterNo,
                                   long *LatchDataTable)        { return 0; }
    virtual bool    GetLatchIOStatus(unsigned short LatchInputNo) { return false; }
    virtual void    SetFIFOLatchSrc(int AxisCounterNo,
                                    int LatchInputNo,
                                    int bEnable)        {}  // ChungHung 20130625: Enable->bEnable

    // -----------------------------------------------------------------------
    //  Virtual methods -- MN200 group motion
    // -----------------------------------------------------------------------
    virtual int     SetGroup(BYTE bGrpNo, BYTE bNumDev, BYTE bDevNo[])
                                                        { return 0; }
    virtual int     LineNMove(BYTE bDevNo[], long DevPos[], BYTE bNumDev)
                                                        { return 0; }

    // -----------------------------------------------------------------------
    //  Virtual methods -- EtherCAT  (RogerYang 20250412: Add ECAT motor func.)
    // -----------------------------------------------------------------------
    virtual void    ResetState()                        {}
    virtual void    ResetAxisOpen()                     {}

    // -----------------------------------------------------------------------
    //  Safe-door check  (Jimmychiu 20221013)
    // -----------------------------------------------------------------------
    bool            CheckIsSafeDoorOpen();
    PF_CHECK        MotorIdleSafeDoorCheck; // 2015.01.15, Joye: Safe door check
};

#endif // HTMotorH
