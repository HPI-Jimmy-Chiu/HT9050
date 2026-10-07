// =============================================================================
//  Motor/myMN200motor.h  -- TMyMN200Motor : PISO-MN200/MotionNet motor driver
//                           + MN200 ring bring-up / health-poll free functions
//
//  Faithful translation of golden Motor/myMN200motor.h (105 lines, BCB6,
//  Big5/cp950 -- verified: 72 non-ASCII bytes, all inside Chinese comments;
//  decoded via cp950, preserved as UTF-8).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 (motor group, motor_syntek_mn200).
//
//  ROLE: this header is TWO things golden bundled into one file (kept bundled
//  here too, mirroring golden exactly):
//    (1) TMyMN200Motor : HTMotor -- concrete driver wrapping the vendor
//        Motor/vendor/mn200.h "mn_*"/"mn200_*" PISO-MN200 MotionNet API.
//    (2) A handful of MODULE-LEVEL (non-member) declarations that golden's
//        myMN200motor.cpp itself defines and that OTHER translation units
//        reach as `extern`: the MyMNetLine baud-rate/type cache, ShowMNetTree
//        (tree-view diagnostic dump), OpenPCI132Card/ResetMNet (ring bring-up
//        and 24V-drop recovery), GetMN200_Error_Code/MN200_Line_status_OK
//        (vendor error-code -> message translation), CheckPCI_L112State/
//        CheckPCI_MN200State (periodic health poll, called from elsewhere in
//        the tree), MNetLog (diagnostic log sink), GetAD8Value_HotGenFlow
//        (analog-input helper unrelated to any one axis).
//
//  WAVE SCOPE -- this header carries NO function bodies (declarations only,
//  matching golden byte-for-byte structurally); all bodies live in
//  myMN200motor.cpp (see that file's own banner for the golden-line inventory
//  of the 2 ctor/dtor + 7 private/class-internal + 24 public virtual class
//  members, plus the 9 module-level free functions).
//
//  GATE REGISTER (header-level only; see myMN200motor.cpp banner for the
//  full register covering .cpp bodies) --
//    TTreeView (golden :ShowMNetTree parameter type, golden note.h/iosetview.h
//    god-header VCL type): this tree has NO TTreeView/TTreeNode port anywhere
//    (grepped the whole tree 20260807: zero hits outside tools/dfm2rc code-gen
//    JSON, which is not compiled C++).  Forward-declared here as an OPAQUE
//    incomplete type -- same idiom already established for TMyBinDispCtrl at
//    database.h:63 (`class TMyBinDispCtrl;  // UI wave -- opaque`) -- so the
//    `ShowMNetTree(TTreeView*)` PROTOTYPE can exist (a pointer to an
//    incomplete type is legal C++) even though the type is never completed in
//    this tree.  The function BODY is gated in the .cpp (see that file);
//    nothing here dereferences the incomplete type.
//
//  VCL/Borland conversions:
//    - `__fastcall` KEPT on the constructor declaration (golden :29), same
//      precedent as Motor/mySMCmotor.h / Motor/mySYNTEKmotor.h.
//    - `char cDeviceName[16]` / commented-out `//short Id ;` / `int
//      iSMCHomeObjectTask` / `int OldSpeed` / `SPEED_PAR MN200SpeedPar` /
//      `int LP, LN` kept EXACTLY as golden -- private data members (the
//      commented-out `Id` field, golden :17, is preserved as a dead comment,
//      matching golden -- it really is dead: no `.Id`/`->Id` reference
//      anywhere in golden myMN200motor.cpp).
//    - `GetMN200ErrorMessage(int nErrCode, AnsiString FUNC="")` (golden :24)
//      kept with its default argument exactly where golden put it (in-header,
//      same shape as this driver's own `ResetPos`/`ScanMotorStatus` etc, and
//      the same shape mySYNTEKmotor.h already uses for its own `ResetPos`).
//    - The commented-out golden method (golden :62,
//      `//        virtual void    SetEnableLatch(bool a);`) is preserved
//      verbatim as a dead comment -- NOT activated, matching golden exactly.
//    - The inline stub overload `GetLatchBuffer(int, unsigned short*, long*)
//      {return 0;}` (golden :67) is kept inline exactly as golden wrote it --
//      NOT split into a .cpp definition, since golden itself defines it
//      in-header (third sibling of this shape in the group, after
//      mySMCmotor.h's own unsigned-short stub and mySYNTEKmotor.h's own
//      short stub -- each driver stubs a DIFFERENT overload of the pair,
//      preserved per-driver, not normalized).
//    - `MyMNetLine` (golden :78-92): an anonymous-struct-typedef with an
//      in-body method sharing the typedef's OWN name (`void MyMNetLine()`).
//      This is legal standard C++ unmodified: because the struct body itself
//      has no elaborated tag (only the trailing `typedef ... MyMNetLine`
//      introduces that name, after the closing brace), the member function
//      `MyMNetLine()` is an ordinary named method, not a constructor -- the
//      name collision that would make it a constructor only exists for a
//      TAGGED class/struct.  `ZeroMemory` resolves via <windows.h>
//      (transitively through MachineDefine.h -> vclcompat), matching every
//      other ZeroMemory call already ported in this tree.
//    - `extern DWORD MN_200_ErrorTable[4];` / `extern bool bResetMNet;` /
//      the free-function externs (golden :99-108) kept verbatim; DWORD/WORD/
//      BYTE resolve via <windows.h> same as elsewhere in this tree.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
//---------------------------------------------------------------------------

#ifndef myMN200motorH
#define myMN200motorH

#include "HTMotor.h"
// AI(W906-1203HAL-1) 20260820: MOTION_IO rename wrap -- see MachineDefine.h's
// note (the MN200 side yields the global typedef name to Advantech per the
// 20260819 1203-first ruling). Tree code spells it MN200_MOTION_IO.
#define MOTION_IO  MN200_MOTION_IO
#define PMOTION_IO MN200_PMOTION_IO
#include "MN200.h"
#undef MOTION_IO
#undef PMOTION_IO
#include "database.h"
const int MAXRing=4;      //Steven 20140828 : 2 --> 4 for XY-Pitch
const int MAXIP=64;
const int MAXPort=4;

// AI(W906-PT-W3) 20260807: opaque forward declaration -- see GATE REGISTER
// above.  No TTreeView/TTreeNode port exists anywhere in this tree; a pointer
// to this incomplete type is enough to keep ShowMNetTree's PROTOTYPE
// byte-faithful to golden.  The .cpp gates the one call site that would need
// a complete type.
class TTreeView;

//---------------------------------------------------------------------------
class TMyMN200Motor:public HTMotor
{
    private:
        char  cDeviceName[16];
//        short Id ;
        int   iSMCHomeObjectTask;
        int   OldSpeed;
        bool  Open_MN200Card();
        void  MN200SoftLimitEnable(bool bFlag);
        bool  MN200MotHome();
        void  SetEncodeMultiple(int iMultiple);
        void  SetServoAlarm();
        void  SetMN200InType();
        void  GetMN200ErrorMessage(int nErrCode, AnsiString FUNC="");           //Steven 20220520 : 針對MN200增加錯誤訊息的紀錄
        SPEED_PAR MN200SpeedPar;
        int LP, LN;
    protected:
    public:
        __fastcall TMyMN200Motor(int Addr);
        ~TMyMN200Motor();
        virtual int     InitMotor(int IoAddress);
        virtual void    SetSpeed(unsigned int x, bool bSetJog=false);           //RogerYang 20250729 Add for 9046AU
        virtual void    SetInitSpeed(unsigned int x);
        virtual void    SetServoAlarmOn(bool Value);
        virtual void    SetAcc(double a);
        virtual void    SetDec(double a);
        virtual int     ReadPos();
        virtual void    ScanMotorStatus(bool *Led);
        virtual bool    MoveTo(int Tar);
        virtual void    Stop();
        virtual void    DecStop();
        virtual bool    JogP();
        virtual bool    JogN();
        virtual bool    HomeObject();
        virtual void    SetRange(unsigned int a);
        virtual bool    GetAlarm(void);
        virtual bool    HomeFlag(void);

        virtual bool    LinearAxisMoveTo(int iPortID[8],long lPos[8],bool bFlag);
        virtual void    EnableTrigger(bool bFlag,int iMode,long lValue);
        virtual bool    ResetPos(int Pulse);
        virtual bool    MotionDone();
        virtual int     ReadRealPos();
        virtual int     ReadEnCoderRealPos();
        virtual int     SetCommand(int p);
        virtual int     SetPosition(int p);
        virtual void    SetServoOn(bool IsOn);
        virtual void    SetSoftLimit(int iPLimit, int iNLimit);

        //-----------------------------
//        virtual void    SetEnableLatch(bool a);
        virtual bool    MoveToPos(int Tar);
        virtual void    ResetLatch();
        virtual int     GetLatchTotalLen();
        virtual int     GetLatchBuffer(int LatchInputNo, short *AxisCounterNo, long *LatchDataTable);
        virtual int     GetLatchBuffer(int LatchInputNo, unsigned short *AxisCounterNo, long *LatchDataTable) {return 0;};
        virtual bool    GetLatchIOStatus(unsigned short LatchInputNo);
        virtual void    SetFIFOLatchSrc(int AxisCounterNo, int LatchInputNo, int bEnable); //ChungHung 20130625 add 跟原本Motor Enable衝突   Enable-->bEnable

        virtual int  SetGroup(BYTE bGrpNo, BYTE bNumDev, BYTE bDevNo[]);
        virtual int  LineNMove(BYTE bDevNo[], long DevPos[], BYTE bNumDev);
};

//------------
//Isaac 20181212 (Steven) : Baud Rate防呆功能
//=>start
typedef struct
{
    public:
    int RaudRate[MAXIP];
    int Type[MAXIP];
    void MyMNetLine()
    {
        ZeroMemory(RaudRate, sizeof(RaudRate));
        ZeroMemory(Type, sizeof(Type));
    };
}MyMNetLine;
extern MyMNetLine myLine[MAXRing];
void ShowMNetTree(TTreeView *TView);
//<==end
//Isaac 20181212 (Steven) : Baud Rate防呆功能
//PISO-MN200====================================================================
extern DWORD MN_200_ErrorTable[4];                                              //jou 2016-04-29 修正記憶體破壞 MN_200_ErrorTable[2] -> MN_200_ErrorTable[4]
extern void OpenPCI132Card(bool bfirst);
extern void ResetMNet(int iRingNo, AnsiString EngMessage, AnsiString ChtMessage, bool bShowMess);
extern bool GetMN200_Error_Code(int iRing, int iCode, AnsiString *EngStr, AnsiString *ChStr, int iIP=-1);
extern bool MN200_Line_status_OK(int iRing, WORD pD, AnsiString *EngStr, AnsiString *ChStr);
extern bool bResetMNet;                                                         //jou 2012-01-10 避免24V還沒開啟就在重置先達模組
extern bool MNetLog(AnsiString Message);                                        //Steven 20110406
extern int  CheckPCI_L112State();
extern int  CheckPCI_MN200State();
extern int GetAD8Value_HotGenFlow(unsigned int iLineNo, unsigned int iDevNo, unsigned int iChannelNo,  AnsiString asMode);  //KaiChen 20190729 ：Hot Gun Flow
#endif
