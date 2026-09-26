// =============================================================================
//  Motor/myEthercatmotor.h  -- TMyEtherCatMotor : Advantech PCI1203 EtherCAT
//                              axis driver (HTMotor subclass)
//
//  Faithful translation of golden Motor/myEthercatmotor.h (138 lines, BCB6,
//  pure ASCII -- verified: 0 bytes > 0x7F in the golden header; the only
//  non-ASCII bytes in this translation unit live in the .cpp).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 (motor_ethercat group).
//
//  ROLE: concrete HTMotor subclass wrapping the vendor AdvMotApi.h "Acm_*" API
//  (Advantech PCI1203 EtherCAT motion master).  Ctor + dtor + 8 private
//  helpers + 34 public virtual overrides + 2 non-virtual public helpers
//  (ethercat_set_output_bit/_byte) -- see the .cpp banner for the full
//  golden-line inventory.
//
//  WAVE SCOPE -- this header carries NO function bodies (declarations only);
//  all bodies live in myEthercatmotor.cpp (see that file's own banner).
//
//  GATE REGISTER -- NONE at the header level (only forward declarations /
//  fallback typedefs here, no calls).  See myEthercatmotor.cpp for the
//  HAVE_PCI1203 vendor-SDK gate that DOES apply to (almost) every method body.
//
//  VCL/Borland conversions:
//    - `__fastcall` KEPT on the constructor declaration (golden :62), matching
//      this tree's own fastcall audit (vclcompat/vcl_compat.h, AI(W906-
//      FastcallFix) 20260720): __fastcall is a REAL i686 MinGW GCC calling-
//      convention attribute here, not a no-op, so every declaration/definition
//      pair must keep it in lockstep (myEthercatmotor.cpp's ctor definition
//      keeps it too).  Same precedent as Motor/mySMCmotor.h.
//    - `#include "HTimer.h"` (golden :7) -- HTimer is NOT a file in the
//      HT9011UC_Code_V3.33.906.0_20260618 golden tree at all; it lives in the
//      shared elec/Component library (out of the golden-scan boundary, same
//      class of exclusion the project has already hit once before -- see
//      MEMORY "golden scan boundary elec component").  This tree already has
//      an established substitute for exactly this situation: TQPF_Timer
//      (myTimer.h), which exposes the SAME Off()/SetSecAndOn(double) surface
//      HTimer does and is already used as HTimer's stand-in elsewhere in this
//      tree (aHotPlateSubstrate.h: "golden type is HTimer (cpublic.h fwd);
//      offline we use TQPF_Timer (same Off()/SetSecAndOn() surface)").
//      `HomeDelay` is declared as `TQPF_Timer` below, not `HTimer`.
//    - `#include "AdvMotDrv.h"` / `#include "AdvMotApi.h"` (golden :8-9) --
//      kept in golden's OWN bare form (no "EtherCAT/" prefix; EtherCAT/vendor/
//      is on the include path, per group guidance).  Both are wrapped in
//      `#if HAVE_PCI1203` (the flag this tree's W4-IO/W5/W906-PT-W2 waves
//      already established for this EXACT vendor header -- see
//      EtherCAT/MyEtherCAT.h/.cpp and EtherCAT/MyNUEC1.h/.cpp, which gate the
//      SAME two headers the SAME way).  Reusing the established flag, not
//      inventing a new one.  When HAVE_PCI1203 is undefined (the default --
//      no vendor SDK linked), two facts still need to be true for this HEADER
//      to compile standalone:
//        (a) `HAND m_Axishand[999]` (golden :37) needs the `HAND` type, which
//            is a vendor macro (`#define HAND UINT_PTR`, AdvMotDrv.h).
//        (b) `extern void EtherCatWriteAO(F32 Data);` (golden :14) needs the
//            `F32` type (`#define F32 float`, AdvMotDrv.h).
//      Both are supplied by a minimal `#else` fallback below (identical
//      values to the vendor macros) so the class layout and the extern
//      declaration are IDENTICAL whether or not the vendor SDK is staged --
//      only the .cpp's method BODIES differ behind the same flag.
// =============================================================================
#ifndef myEthercatmotorH
#define myEthercatmotorH
//---------------------------------------------------------------------------
//#include "MyMotor.h"
#include "HTMotor.h"
#include "myTimer.h"                // TQPF_Timer -- substitutes golden's HTimer (see banner above)

#if HAVE_PCI1203
// AI(W906-1203HAL-1) 20260820: single compat include -- AdvMotApi.h pulls
// AdvMotDrv.h in itself, and the shim supplies ADVCMNAPI for MinGW and
// #undef's the vendor's poisonous generic-name macro (`Direct`). Golden's
// two-line include pair (:8-9) is preserved semantically, not textually.
#include "EtherCAT/AdvMotCompat.h"
#else
#include <windows.h>                // UINT_PTR
#ifndef HAND
#define HAND UINT_PTR               // matches vendor AdvMotDrv.h: "#define HAND UINT_PTR"
#endif
#ifndef F32
#define F32 float                   // matches vendor AdvMotDrv.h: "#define F32 float"
#endif
#endif
//#include "General.h"

//---------------------------------------------------------------------------
extern unsigned int gDevhand;
extern void EtherCatWriteAO(F32 Data);
//---------------------------------------------------------------------------
class TMyEtherCatMotor:public HTMotor
{
    //friend class HTAdvantechGroup;

    private:
        //char  cDeviceName[24];
        AnsiString  cDeviceName;
        short   MotorID ;
        int   iSMCHomeObjectTask;
        int   OldSpeed;
        static unsigned int gDevhand;
        bool   DoHome(unsigned int mode, unsigned int dir);
        void   Open_Card();
        void   Close_Card();
        void   Open_Axis();
        TQPF_Timer HomeDelay;
        bool EtherCatMotHome();
        int iMotorCount;
        void  SetEtherCatInType();
        void SetHomeSpeed();
        bool bAxisOpen;                                                         //RogerYang 20250402 9046AU
        HAND m_Axishand[2560];                                                  //RogerYang 20250402 9046AU  //AI(W906-M108-AXIS) 20260925: 999 -> 2560。MotorID = iBoardID*10+iPortID（myEthercatmotor.cpp:291），HT9050 的 M108 MCCDY BoardID=108 ⇒ 1080 越界（golden 同樣是 [999]）；RULINGS_20260925 第 28 條使用者「依據建議」＝開大，偏離 golden。2560 涵蓋 BoardID 0～255
    protected:
        short  iBoardID;
        short  iPortID;
        ULONG MotorCount;
        int  bAxisID;
        double dAcc;

        unsigned int axisHand;

        bool GetHomeIO(void);
        bool RealG00(int p);
        //int ReadRealPos(void);

        bool Error(void);
        bool Busy(void);

        bool PosDirectHome();
        bool NegDirectHome();
        bool PosDirectHomeOneTouch();
        bool NegDirectHomeOneTouch();
//        bool AddAxis(int iAxis);
//        bool HomeType90();

    public:
        __fastcall TMyEtherCatMotor(int addr);
        ~TMyEtherCatMotor();

        virtual int     InitMotor(int IoAddress);

        virtual bool    G00(int iPos);           //相對Move to

        virtual void    SetRate(unsigned int a); //加減速功能
        virtual void    SetSpeed(unsigned int x, bool bSetJog=false);
        virtual void    SetInitSpeed(unsigned int x);
        virtual void    SetServoAlarmOn(bool Value);
        virtual void    SetAcc(double a);
        virtual void    SetDec(double a);

        virtual int     ReadPos();
        virtual void    ScanMotorStatus(bool *Led);
        virtual bool    MoveTo(int Tar);
        virtual void    Stop();
        virtual void    DecStop();   double W906_RuntimeAcc() const { return dAcc; }  double W906_RuntimeDec() const { return dDec; }   // AI(W906-W4-MOTOR) 20260925: 唯讀存取子（port-only）—— SetSpeed 寫進 PAR_AxAcc／PAR_AxDec 的是本類別遮蔽的 dAcc 與 HTMotor::dDec（SetADCRate 依配方百分比改過），HTMotor::ReadAcc/ReadDec 回的是資料庫值；motor.access 要送跟 golden 同一組值
        virtual bool    JogP();
        virtual bool    JogN();
        virtual bool    HomeObject();

        virtual void    SetRange(unsigned int a);
        virtual bool    GetAlarm(void);
        virtual bool    HomeFlag(void);

        virtual void    MotOutputOn(int iOutPort);
        virtual void    MotOutputOff(int iOutPort);
        virtual void    MotInputStatus(bool *bInputPort);//not sure
        virtual bool    LinearAxisMoveTo(int iPortID[8],long lPos[8],bool bFlag);   //not sure
        virtual void    EnableTrigger(bool bFlag,int iMode,long lValue);            //not sure

        virtual bool    AddAxis(int axis);  //mark //Jeffrey 20161005 Add 4 AdvantechMotor
        virtual bool    AddPath(int axis);  //mark //Jeffrey 20161005 Add 4 AdvantechMotor
        virtual bool    RunPath(void);      //mark //Jeffrey 20161005 Add 4 AdvantechMotor

        //virtual void    SetPos(int p);//跟原SetPosition

        virtual bool    ResetPos(int Pulse);
        virtual bool    MotionDone();
        virtual int     ReadRealPos();
        virtual int     ReadEnCoderRealPos();
        virtual int     SetCommand(int p);
        virtual int     SetPosition(int p);
        virtual void    SetServoOn(bool IsOn);
        virtual void    SetSoftLimit(int iPLimit,int iNLimit);

        //-----------------------------
//        virtual void    SetEnableLatch(bool a);
        virtual bool    MoveToPos(int Tar);
        virtual void    ResetLatch();
        virtual int     GetLatchTotalLen();
        virtual int     GetLatchBuffer(int LatchInputNo, short *AxisCounterNo, long *LatchDataTable);
        virtual int     GetLatchBuffer(int LatchInputNo, unsigned short *AxisCounterNo, long *LatchDataTable) {return 0;};
        virtual bool    GetLatchIOStatus(unsigned short LatchInputNo);
        virtual void    SetFIFOLatchSrc(int AxisCounterNo, int LatchInputNo, int bEnable); //ChungHung 20130625 add 跟原本Motor Enable衝突   Enable-->bEnable

        virtual void    ResetState();
        virtual void    ResetAxisOpen();
        //
        int iDelayCount;
        int iWaitCount;
        int iStepRange;

        ULONG AxisCount;
        DWORD m_dwDevNum;
        //

        // 2018.12.13 Advantech Added Ether DO
        //int ethercat_set_output_bit(int Ring, int IP, int Port, int Bit, int Data);
        int ethercat_set_output_bit(int Ring, int IP, int Port, int Bit, int Data);
        int ethercat_set_output_byte(int Ring, int IP, int Port, int Byte);
        bool IOBitOff(int port, int bit);
};
//---------------------------------------------------------------------------
#endif
