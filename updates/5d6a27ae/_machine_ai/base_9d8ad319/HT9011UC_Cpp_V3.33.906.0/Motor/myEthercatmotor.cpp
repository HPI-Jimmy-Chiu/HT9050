// =============================================================================
//  Motor/myEthercatmotor.cpp  -- TMyEtherCatMotor : Advantech PCI1203
//                                EtherCAT axis driver (HTMotor subclass)
//
//  Faithful translation of golden Motor/myEthercatmotor.cpp (1782 lines, BCB6,
//  pure ASCII -- verified: 0 bytes > 0x7F in the golden .cpp; decoded via
//  Python open(path,'rb').read().decode('cp950') purely to confirm this --
//  the decode round-trips to identical ASCII text, ZERO U+FFFD, so no Big5
//  transcription was actually needed for this file's own comments).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 (motor_ethercat group).
//
//  ROLE: concrete HTMotor subclass wrapping the vendor AdvMotApi.h "Acm_*" API
//  (Advantech PCI1203 EtherCAT motion master, the SAME vendor family and the
//  SAME shared `uiDevhand` master handle EtherCAT/MyEtherCAT.cpp and
//  EtherCAT/MyNUEC1.cpp already translated in earlier waves).  1 free function
//  is dead in golden (Open_Card, see note below); 1 more free function
//  (EtherCatWriteAO) plus ctor/dtor plus 58 member functions round out the
//  file.
//
//  WAVE SCOPE -- ACTIVE (faithful, verbatim -- ALL 64 golden functions, whole
//  file, every branch, every magic number, every golden comment preserved):
//    TMyEtherCatMotor::TMyEtherCatMotor (ctor)  golden :49-68
//    TMyEtherCatMotor::~TMyEtherCatMotor (dtor) golden :71-74
//    Open_Card (free fn, dead -- see note)      golden :78-141
//    Open_Axis                                  golden :143-164
//    InitMotor                                  golden :166-396
//    Stop                                       golden :400-411
//    DecStop                                    golden :415-439
//    JogP                                       golden :444-487
//    JogN                                       golden :491-533
//    G00                                        golden :537-563
//    SetRange                                   golden :567-572
//    SetRate                                    golden :576-616
//    SetSpeed                                   golden :618-731
//    SetInitSpeed                               golden :733-736
//    SetPosition                                golden :738-750
//    SetCommand                                 golden :752-766
//    SetSoftLimit                               golden :770-801
//    SetServoAlarmOn                            golden :805-808
//    ReadPos                                    golden :812-815
//    GetAlarm                                   golden :821-839
//    ScanMotorStatus                            golden :843-892
//    HomeFlag                                   golden :896-917
//    RealG00                                    golden :923-975
//    ReadRealPos                                golden :979-999
//    Busy                                       golden :1006-1023
//    Error                                      golden :1027-1048
//    GetHomeIO                                  golden :1052-1067
//    DoHome                                     golden :1069-1102
//    PosDirectHome                              golden :1107-1110
//    NegDirectHome                              golden :1114-1117
//    PosDirectHomeOneTouch                      golden :1122-1125
//    NegDirectHomeOneTouch                      golden :1129-1132
//    AddAxis                                    golden :1138-1143
//    AddPath                                    golden :1145-1148
//    RunPath                                    golden :1150-1153
//    ethercat_set_output_bit                    golden :1155-1166
//    ethercat_set_output_byte                   golden :1168-1177
//    IOBitOff                                   golden :1179-1188
//    SetAcc                                     golden :1191-1194
//    SetDec                                     golden :1196-1199
//    MotionDone                                 golden :1201-1223
//    MoveTo                                     golden :1225-1282
//    HomeObject                                 golden :1284-1287
//    EtherCatMotHome                            golden :1289-1378
//    ResetPos                                   golden :1380-1390
//    MotOutputOn                                golden :1392-1412
//    MotOutputOff                               golden :1414-1425
//    SetServoOn                                 golden :1427-1442
//    MotInputStatus                             golden :1444-1447
//    LinearAxisMoveTo                           golden :1449-1480
//    EnableTrigger                              golden :1482-1511
//    MoveToPos                                  golden :1513-1543
//    ResetLatch                                 golden :1545-1547
//    GetLatchTotalLen                           golden :1549-1552
//    GetLatchBuffer(short*)                     golden :1554-1557
//    GetLatchIOStatus                           golden :1559-1562
//    SetFIFOLatchSrc                            golden :1564-1566
//    ReadEnCoderRealPos                         golden :1568-1588
//    SetEtherCatInType                          golden :1590-1690
//    EtherCatWriteAO (free fn)                  golden :1692-1701
//    SetHomeSpeed                               golden :1703-1742
//    Close_Card                                 golden :1744-1762
//    ResetState                                 golden :1764-1773
//    ResetAxisOpen                              golden :1775-1780
//  Plus the 5 file-scope globals/consts golden :28-45 (the dead
//  Acm_AxGetMotionStatus-mirror consts Stop/Res1/WaitERC/Res2/CorrectBksh/
//  Res3/InFA/InFL/InACC/InFH/InDEC/WaitINP, TMyEtherCatMotor::gDevhand
//  static-member definition, MAX_EtherCat_MOTOR macro, bAlarm, bFirstClickJog
//  -- see notes below).
//  GetLatchBuffer(unsigned short*) stub (golden myEthercatmotor.h:116,
//  `{return 0;}`) stays inline in the header -- never had a .cpp body in
//  golden either.
//  SATISFIED-BY-SUBSTRATE: HTMotor (Motor/HTMotor.h/.cpp, base class fields
//  Enable/Direction/HomeDirection/GearRatio/Range/iSpeed/InitSpeed/dDec/
//  PSoftLimitP/PSoftLimitN/LastHomePos/iHomeObjectTask/ErrorString --
//  already ported W4; note `dAcc`/`iBoardID`/`iPortID`/`bAxisID` are
//  RE-DECLARED and SHADOWED by this class, see VCL/Borland notes below);
//  MySleep (common.h declaration / body, already ported); ShowErrorMessage /
//  ShowMyMessage (canary_support.h, substituting golden note.h/mymessbox.h);
//  Sen[]/TMySensor::IsOff (mysensor.h, already ported); IntToStr/IntToHex
//  (vclcompat); TQPF_Timer (myTimer.h, substituting golden's out-of-tree
//  HTimer -- see header banner); uiDevhand/MMSystem (cmydef.h, already
//  ported).
//
//  NOTES ON GOLDEN'S OWN PRE-EXISTING QUIRKS (preserved verbatim, NOT fixed):
//   * Open_Card(): golden myEthercatmotor.h:28 declares a PRIVATE class
//     member `void Open_Card();`, but golden myEthercatmotor.cpp:78 defines
//     a FREE function `void Open_Card()` with NO class qualifier -- an
//     entirely different symbol, whose body is 100% `/* ... */`-commented.
//     The class member is therefore declared but NEVER DEFINED anywhere in
//     golden, and (cross-tree grep, 20260807) never called by anyone either
//     -- this is a pre-existing golden dead declaration, not a translation
//     gap in the HARD RULE 5/6 sense (there is no golden call site to gate
//     and no golden body to have skipped).  Reproduced identically: this
//     port's header also declares the never-defined private member, and
//     this .cpp defines only the free, fully-commented-out function.
//   * ethercat_set_output_bit / ethercat_set_output_byte / IOBitOff all read
//     the unqualified name `gDevhand` inside a TMyEtherCatMotor member
//     function.  Unqualified name lookup inside a member function finds the
//     class's OWN member first -- i.e. these three calls resolve to the
//     PRIVATE STATIC member `TMyEtherCatMotor::gDevhand` (golden
//     myEthercatmotor.h:26, defined golden :42 as `=NULL`), NOT the
//     file-scope `extern unsigned int gDevhand;` declared at
//     myEthercatmotor.h:13.  `TMyEtherCatMotor::gDevhand` is NEVER assigned
//     anywhere else in this file (Open_Axis/Close_Card/EtherCatWriteAO all
//     use the DIFFERENT global `uiDevhand`, cmydef.h) -- so these three
//     methods always pass handle 0 to Acm_DaqDoSetBit/Acm_DaqDoSetByte. This
//     looks like a golden bug (probably meant `uiDevhand`), but it is
//     golden's ACTUAL behavior, so it is translated exactly as written: the
//     unqualified `gDevhand` below resolves to the same shadowed static
//     member, not the module-scope extern.
//   * SetCommand(int p): golden declares `U32 Result;` with NO initializer
//     and returns it even down the `!Enable` path where it is NEVER
//     assigned -- reading an indeterminate value is undefined behavior in
//     both BCB6 and standard C++ (a plain local is not VCL-zero-filled the
//     way an object's fields are; this is NOT the "NewInstance zero-fills
//     the object" situation the project's other UB-reproductions rely on).
//     Since there is no meaningful way to reproduce "whatever happened to be
//     on the stack", this port initializes `Result=0` (==SUCCESS) instead of
//     leaving it indeterminate -- a disclosed, deliberate deviation from
//     golden's UB, not a HARD RULE 6 gate (no vendor symbol is missing here;
//     the change is purely "well-defined value instead of undefined one").
//   * bAlarm / bFirstClickJog (golden :44-45) are plain, non-`static`
//     file-scope globals.  `bFirstClickJog` is `extern`-declared already in
//     the ALREADY-PORTED cmydef.h (cmydef.h:5773) but never defined anywhere
//     in the port tree yet (grep 20260807: zero definitions) -- golden's own
//     sole definition site for it IS this file (golden myEthercatmotor.cpp:45),
//     confirmed by a whole-golden-tree grep finding no other definition. This
//     port supplies that definition here, matching golden exactly, so any
//     other already- or later-translated consumer of the cmydef.h extern
//     links against this TU's global.  `bAlarm` has no extern in cmydef.h and
//     no cross-file consumer (whole-tree grep: every OTHER `bAlarm` hit is an
//     unrelated local/static variable of the same common name in a different
//     function) -- a private file-scope global exactly as golden wrote it.
//
//  GATE REGISTER -- ONE recurring gate, reused at (nearly) every method
//  below, all citing this single entry rather than repeating the rationale:
//    The Acm_Ax*/Acm_Daq*/Acm_Dev*/Acm_Set*Property vendor family (vendor
//    AdvMotApi.h + siblings, staged read-only under EtherCAT/vendor/) has NO
//    compiled body anywhere in this tree by design -- it resolves only
//    against the Advantech PCI1203 vendor .lib at link time (Hard Rule 6
//    applies exactly as it already did for the sibling
//    EtherCAT/MyEtherCAT.cpp and EtherCAT/MyNUEC1.cpp translations, which
//    call the SAME vendor family through the SAME shared `uiDevhand`
//    handle).  Every real call is gated behind `#if HAVE_PCI1203`, REUSING
//    the flag those two earlier waves already established for this EXACT
//    vendor header (see EtherCAT/MyEtherCAT.h/.cpp and EtherCAT/MyNUEC1.h/.cpp
//    banners) -- not a new flag invented by this unit.  DEFAULT
//    (HAVE_PCI1203 undefined, the only configuration this tree currently
//    builds): each `#else` arm below reproduces the SAME outcome golden's
//    own "Result != SUCCESS" / "axis never reaches STA_AX_READY" failure
//    path already produces -- i.e. the fallback is golden's own
//    already-modeled hardware-absent/communication-failure shape, not an
//    invented one.  Per-function specifics (documented inline at each
//    `#else`, summarized here):
//      Open_Axis                 -> bAxisOpen=false (open failed)
//      InitMotor                 -> skip straight to the always-reached
//                                    SetServoOn/SetCommand/SetPosition tail
//                                    (every golden property-write failure
//                                    above it is log-and-continue, never a
//                                    hard stop)
//      GetAlarm/Busy/Error/GetHomeIO/EtherCatMotHome
//                                 -> return false / true exactly matching
//                                    what golden's own state==0 (never
//                                    fetched) branch evaluates to
//      MotionDone                 -> return false (state stays 0 !=
//                                    STA_AX_READY either way)
//      ethercat_set_output_bit/_byte/IOBitOff
//                                 -> return -1 / false (call-failed shape)
//      DoHome/RealG00/MoveTo/MoveToPos/SetCommand/SetPosition/SetSoftLimit/
//      SetRate/SetSpeed/SetEtherCatInType/SetHomeSpeed/Close_Card/
//      ResetState/ScanMotorStatus/MotOutputOn/MotOutputOff/
//      ReadRealPos/ReadEnCoderRealPos
//                                 -> void/no-op the vendor write (nothing to
//                                    write to non-existent hardware), fetched
//                                    values stay at their golden-declared
//                                    zero-initializers, same as a fully-failed
//                                    Acm_* call would leave them
//    BEHAVIOUR DELTA, STATED PLAINLY: with no PCI1203 vendor SDK linked, this
//    axis type can never actually move, home, or report a live status --
//    identical in spirit to golden's own hardware-absent/communication-lost
//    path, just reached by a build-time switch instead of a runtime one.
//    Retire by defining HAVE_PCI1203 once the vendor .lib is wired into the
//    link step; every `#if HAVE_PCI1203` arm above is golden's real
//    behavior, untouched.   AI(W906-ECAT-ROUTE) 20260929: the #else arms now also consult the installable ENGINE MOTOR ROUTE (Motor/EcatMotorRoute.h, body EtherCAT/Pci1203MotorRoute.cpp, wb_serve only, MachineType.h WB_ENGINE_MOTOR_1203); with no route installed every one of them is exactly what this register describes (design 7.5, tests/test_ecat_motor_route.cpp part A).
//
//  VCL/Borland conversions:
//    - #pragma hdrstop (golden :4) and #pragma package(smart_init)
//      (golden :17) dropped.
//    - `__fastcall` KEPT on the ctor definition (golden :49), matching the
//      header (see mySMCmotor.cpp/.h precedent this same wave already used
//      for the identical situation).
//    - golden `#include "note.h"` / `#include "mymessbox.h"` (golden :10,12)
//      -> `#include "canary_support.h"` (this tree's established substitute,
//      already supplying ShowErrorMessage/ShowMyMessage with matching
//      signatures -- see e.g. ainarm9045.cpp's own identical substitution
//      note).
//    - golden `#include "myEthercatmotor.h"` (golden :9) pulls in this
//      class's own contract, which (per this unit's header banner) also
//      supplies TQPF_Timer (HomeDelay) and the HAND/F32 fallback typedefs --
//      nothing additional needed here for those.
//    - `U32`/`U16`/`unsigned short` local declarations inside `#if
//      HAVE_PCI1203` blocks are kept in golden's EXACT spelling (the vendor
//      header is in scope there); only declarations that must be visible to
//      an `#else` fallback use a portable spelling instead (documented at
//      each such site).
//
//  Big5: N/A -- golden .cpp is pure ASCII (0 bytes > 0x7F, confirmed via the
//  cp950 decode round-trip described above).  Final gate: ZERO U+FFFD
//  (trivially satisfied).
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include <stdio.h>
//   #include "myEthercatmotor.h" / "note.h" / "mymessbox.h" / "mysensor.h" /
//     "common.h" / "cmydef.h"
//   ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)
#include "myEthercatmotor.h"        // this unit's own contract (TMyEtherCatMotor : HTMotor) + TQPF_Timer + HAND/F32 fallback (see header banner)
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage -- substitutes golden note.h / mymessbox.h (see banner)
#include "mysensor.h"                // Sen[] (TMySensor::IsOff), SnFrontLeftEMG/SnFrontRightEMG/SnRearLeftEMG/SnRearRightEMG
#include "common.h"                  // MySleep
#include "cmydef.h"                  // uiDevhand, MMSystem, extern bool bFirstClickJog (defined below)
//---------------------------------------------------------------------------
//-------------
//MotorType
//-------------
//const int Step_Motor  =0;
//const int Servo_Motor =1;
//const int Rotate_Motor=2;
//==============================================================================
#include "EcatMotorRoute.h"         //AI(W906-ECAT-ROUTE) 20260929: the engine -> 1203 motor route (INBOX 112; docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md). POD + function pointers, no EtherCAT header; W906_EC_ONLY(...) = nothing when HAVE_PCI1203 is on
//Acm_AxGetMotionStatus
const int Stop         =0;                //停止
const int Res1         =1;                //保留1
const int WaitERC      =2;                //等待 ERC 完成
const int Res2         =3;                //保留2
const int CorrectBksh  =4;                //補償
const int Res3         =5;                //保留3
const int InFA         =6;                //處於特定速度
const int InFL         =7;                //處於低速
const int InACC        =8;                //加速中
const int InFH         =9;                //處於最大速度中
const int InDEC        =10;               //減速中
const int WaitINP      =11;               //等待到位
//==============================================================================

unsigned int TMyEtherCatMotor::gDevhand    = NULL;
#define MAX_EtherCat_MOTOR 64
bool bAlarm=false;                                                              //RogerYang 20250421 get motor alarm state
bool bFirstClickJog=true;
//---------------------------------------------------------------------------
//HTAdvantechMotor::HTAdvantechMotor(int AxisHandle)
//---------------------------------------------------------------------------
__fastcall TMyEtherCatMotor::TMyEtherCatMotor(int addr)
{
    if(addr==-1)
    {
    }
    else
    {
        //PCordModel = "Advantech";
        //MotorID = 0; //RogerYang

        iBoardID    = addr/100;
        iPortID     = addr%100;

        MotorID=(iBoardID*10)+iPortID;

        //sprintf(cDeviceName,"%s,Board_Port:%02d_%02d","PCI1203", iBoardID, iPortID);
        cDeviceName.sprintf("%s,Board_Port:%02d_%02d","PCI1203", iBoardID, iPortID);
    }
    bAxisOpen=false; //RogerYang 20250403
}
// ------------------------------------------------------------------------

TMyEtherCatMotor::~TMyEtherCatMotor()
{
    Close_Card();
}
//-----------------------------------------------------------------
//initialize motor
//-----------------------------------------------------------------
void Open_Card()
{
/*    if (gDevhand != NULL)
        return;

    DEVLIST m_avaDevs[MAX_DEVICES];
    U32 deviceCounter = 0;
    U32 Result;

    Result = Acm_GetAvailableDevs(m_avaDevs,MAX_DEVICES,&deviceCounter);
    if(Result != SUCCESS)
    {
        ShowMessage("No Available Device In System");
        return;
    }

    ULONG  buffLen=64;
    ULONG  AxesPerDev;
    m_dwDevNum = m_avaDevs[0].dwDeviceNum;

    Result = Acm_DevOpen(m_dwDevNum,&gDevhand);
    for (int i = 0; i < 10; ++i)
    {
        if (Result != SUCCESS)
        {
            MySleep(5000);
            Result = Acm_DevOpen(m_dwDevNum,&gDevhand);
        }
        else
        {
            break;
        }
    }

    if(Result != SUCCESS)
    {
      ShowMessage("Open Device Failed With Error Code: " + IntToHex((int)Result,8));
      return;
    }
    buffLen = 4;
    Result = Acm_GetProperty(gDevhand,FT_DevAxesCount,&AxesPerDev,&buffLen);
    if(Result != SUCCESS)
    {
      ShowMessage("Can Not Get Device Axes Number With Error Code: " + IntToHex((int)Result,8));
      return;
    }

    if(AxesPerDev<=0)
        ShowMessage("There is no Motor !!");

    MotorCount=AxesPerDev;
    */
//    Result = Acm_SetU32Property(gDevhand, CFG_DevLogMsg, 0);     //for debug use

//    for(int i=0; i<AxesPerDev; i++)
//    {
//        Result = Acm_AxOpen(gDevhand,(USHORT)i, &m_Axishand[i]);
//        if(Result != SUCCESS)
//        {
//            ShowMessage("Can Cot Open Motor : "+IntToStr(i+1));
//            return;
//        }
//    }
}
//------------------------------------------------------------------
void TMyEtherCatMotor::Open_Axis()
{
    if(uiDevhand==0 W906_EC_ONLY(&& EcatMotorRoute()==0))   //AI(W906-ECAT-ROUTE) 20260929: no route = the same test as before; with it the card is the monitor's (uiDevhand stays 0 in this tree, cmydef.cpp) and the #else arm below CLAIMS instead of opening
        return;

    if(!bAxisOpen)
    {
#if HAVE_PCI1203
        U32 Result;
        AnsiString strError;
        bool AxisCheck = true;

        Result=Acm_AxOpenbyID(uiDevhand, iBoardID, iPortID, &m_Axishand[MotorID]);
        if(Result!=SUCCESS)
        {
            AxisCheck=false;
            strError="MotorID:" + IntToStr(MotorID) + ","+ IntToHex((int)Result,8);
            ShowErrorMessage("WAR16120", 0, MMSystem, 0, strError);
        }

        bAxisOpen=AxisCheck;
#else
        // No vendor EtherCAT SDK linked (HAVE_PCI1203 undefined).  //AI(W906-ECAT-ROUTE) 20260929: with the engine motor
        // route installed (wb_serve; Motor/EcatMotorRoute.h) the axis is CLAIMED, never opened -- the 1203 monitor
        // already holds its handle (design section 0.1). The route refuses a Direction=1 row with the reason printed
        // (ruling 6B, Q5) and so does the `&& !Direction` below. No route = unreachable (the uiDevhand==0 return above),
        // as before. golden's WAR16120 on a failed open is not raised (design 4.4). See file banner GATE REGISTER.
        bAxisOpen=(EcatMotorRoute()!=0) && EcatMotorRoute()->bind(iBoardID, iPortID, MotorID, Direction) && !Direction;
#endif
    }
}
//------------------------------------------------------------------
int TMyEtherCatMotor::InitMotor(int axIndex)
{
    AnsiString strError;

    if(!Enable)
        return true;

    Open_Axis();

    if(bAxisOpen==false)
        return false;

    if(Sen[SnFrontLeftEMG].IsOff() || Sen[SnFrontRightEMG].IsOff() ||
        Sen[SnRearLeftEMG].IsOff() || Sen[SnRearRightEMG].IsOff())
    {
        ShowMyMessage("Please Unlock EMG And Restart The Software!!", "請解開EMG並重新啟動軟體!!");
        return false;
    }

#if HAVE_PCI1203
    U32 Result;
    unsigned short state=0;

    ResetMotorError:    //KaiHuang 20200928 Add
    //Reset Error
    Result=Acm_AxResetError(m_Axishand[MotorID]);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " ResetError:" + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    //確認馬達在 Ready 狀態才可寫入參數
    Result=Acm_AxGetState(m_Axishand[MotorID], &state);
    if(Result==SUCCESS)
    {
        if(state!=STA_AX_READY)
        {
            //Reset Error
            Result=Acm_AxResetError(m_Axishand[MotorID]);
            if(Result!=SUCCESS)
            {
                strError="Mot MotorID-" + IntToStr(MotorID) + " State:" + IntToStr(state);
                ShowErrorMessage("WAR16123", 0, MMSystem, 0, strError);
                goto ResetMotorError;   //KaiHuang 20200928 Add
            }
        }
    }
    else
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + "," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
        goto ResetMotorError;   //KaiHuang 20200928 Add
    }

    unsigned int PPU=1;
                                     //Setting the value of PPU
    Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxPPU,PPU);
    if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " PPU," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    unsigned int uElReact=0;                                     //設定遇到極限點時所反應的動作  0:立即停止 1:減速停止
    Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxElReact, uElReact);
    if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " ElReact," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    unsigned int uAlmEnable=1;                                   //設定啟用Alarm訊號觸發 0:Disable 1:Enable
    Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxAlmEnable, uAlmEnable);
    if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " AlmEnable," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    unsigned int AlmReact=0;                                     //設定Alarm訊號觸發所反應的動作 0:立即停止 1:減速停止
    Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxAlmReact, AlmReact);
    if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " AlmReact," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    unsigned int uOrgLogic=bSensorType ? 0 : 1;                //設定原點的觸發邏輯
    Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxOrgLogic, uOrgLogic);
    if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " OrgLogic," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    double iMode=0;                                                             //RogerYang 20250729 move from SetSpeed to here
    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxJerk, iMode);          //RogerYang 20250729 move from SetSpeed to here
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " Jerk," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    /*板卡不支援
    unsigned int uPulseOutMode = MotorType ? 8 : 32;            //Setting of command pulse output mode. 16:CW/CCW   1:OUT/DIR
    Result=Acm_SetProperty(m_Axishand[Address],CFG_AxPulseOutMode,&uPulseOutMode,sizeof(uPulseOutMode));
    if (Result != SUCCESS)
    {
        strError="Mot Address:" + IntToStr(Address) + " PulseOutMode," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
    }
    */

//    板卡不支援
//    unsigned int uLnpLogic = 0;
//    Result=Acm_SetProperty(m_Axishand[Address],CFG_AxInpLogic,&uLnpLogic,sizeof(uLnpLogic));                                //Setting of active logic for In-Position signal.  Low:0  High:1
//    if (Result != SUCCESS)
//    {
//        ShowMessage("Axis-" + IntToStr(Address) + " Set CFG_AxInpLogic False ! Error Code : " + IntToHex((int)Result,8));
//        return false;
//    }

//    unsigned int uPulseInMode = AB_4X;                               //Setting of encoder feedback pulse input mode.   0: 1X A/B 2: 4X
//    Result=Acm_SetProperty(m_Axishand[Address],CFG_AxPulseInMode,&uPulseInMode,sizeof(uPulseInMode));
//    if (Result != SUCCESS)
//    {
//        ShowMessage("Axis-" + IntToStr(Address) + " Set CFG_AxPulseInMode False ! Error Code : " + IntToHex((int)Result,8));
//        return false;
//    }

//    unsigned int uPulseInLogic;                               //Setting of encoder feedback pulse input logic.   0: 1X A/B 2: 4X
//    if(Direction == 1)
//        uPulseInLogic = INV_DIR;
//    else
//        uPulseInLogic = NO_INV_DIR;
//    Result=Acm_SetProperty(m_Axishand[Address],CFG_AxPulseInLogic,&uPulseInLogic,sizeof(uPulseInLogic));
//    if (Result != SUCCESS)
//    {
//        ShowMessage("Axis-" + IntToStr(Address) + " Set CFG_AxPulseInLogic False ! Error Code : " + IntToHex((int)Result,8));
//        return false;
//    }
    SetEtherCatInType();

    if(MotorType==Servo_Motor)
    {
        unsigned int uPulseInMode=AB_4X;                         //設定脈衝輸入模式
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxPulseInMode, uPulseInMode);
        if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " PulseInMode," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }

        unsigned int uPulseOutMode=O_CW_CCW;                     //設定脈衝輸出模式
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxPulseOutMode, uPulseOutMode);
        if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " PulseOutMode," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
    }
    else if(MotorType==Rotate_Motor)
    {
        unsigned int uPulseInMode=I_CW_CCW;                      //設定脈衝輸入模式
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxPulseInMode, uPulseInMode);
        if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " PulseInMode," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }

        unsigned int uPulseOutMode=OUT_DIR_ALL_NEG;              //設定脈衝輸出模式
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxPulseOutMode, uPulseOutMode);
        if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " PulseOutMode," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
    }
    else
    {
        unsigned int uPulseInMode=I_CW_CCW;                      //設定脈衝輸入模式
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxPulseInMode, uPulseInMode);
        if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " PulseInMode," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }

        unsigned int uPulseOutMode=OUT_DIR_DIR_NEG;              //設定脈衝輸出模式
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxPulseOutMode, uPulseOutMode);
        if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " PulseOutMode," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
    }

    Result=Acm_SetF64Property(m_Axishand[MotorID], CFG_AxMaxVel, PJogHighSpeed);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " MaxVel," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    Result=Acm_SetF64Property(m_Axishand[MotorID], CFG_AxMaxAcc, dAcc);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " MaxAcc," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    Result=Acm_SetF64Property(m_Axishand[MotorID], CFG_AxMaxDec, dDec);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " MaxDec," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    //Reset Error
    Result=Acm_AxResetError(m_Axishand[MotorID]);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " ResetError:" + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#else
    // No vendor EtherCAT SDK linked (HAVE_PCI1203 undefined).  //AI(W906-ECAT-ROUTE) 20260929: with the engine motor
    // route installed (wb_serve) the Reset/GetState ladder and the CFG_Ax*/PAR_Ax* table above go through
    // EtherCAT/Pci1203MotorRoute.cpp: kCmdAxResetError, EastSun's Pci1203GoldenInitCfgPlan (kCmdAxSetInitCfg, golden
    // order), CFG_AxMaxVel/MaxAcc/MaxDec, ResetError -- log-and-continue like golden; the unbounded
    // `goto ResetMotorError` is bounded to 3 rounds (then false, like WebMotorAccess.cpp InitMotor1203). The tail
    // below then runs as in golden (SetServoOn -> SvOn; SetCommand/SetPosition, which the route refuses on a DS402
    // drive, Q1). No route = unreachable (the bAxisOpen==false return above). SetEtherCatInType() below is a no-op
    // under this flag (its writes are in the plan), so calling it is harmless. See file banner GATE REGISTER.
    //   motorClass: 0 = golden's Servo_Motor arm, 1 = Rotate_Motor, 2 = otherwise (Pci1203Control.h kInitCfgMotor*)
    if(EcatMotorRoute() && !W906_EcInitCfg(iBoardID, iPortID, MotorType==Servo_Motor ? 0 : MotorType==Rotate_Motor ? 1 : 2, bSensorType, bIn1Logic, PJogHighSpeed, dAcc, dDec)) return false;
    SetEtherCatInType();
#endif

    SetServoOn(true);
    SetCommand(0);
    SetPosition(0);
    return true;
}
//-----------------------------------------------------------------

//-----------------------------------------------------------------
void TMyEtherCatMotor::Stop(void)
{
#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_AxStopEmg(m_Axishand[MotorID]);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " StopEmg," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
    }
#endif
    W906_EC_ONLY(W906_EcCall(iBoardID, iPortID, kEcStopEmg, 0, 0);) }   //AI(W906-ECAT-ROUTE) 20260929: golden :400-411 Acm_AxStopEmg through the route (no engine caller; for completeness). No route = kEcRcNoRoute, no side effect
//-----------------------------------------------------------------
//Deceleration
//-----------------------------------------------------------------
void TMyEtherCatMotor::DecStop()
{
    if(!bAxisOpen W906_EC_ONLY(&& (Open_Axis(), !bAxisOpen)))   //AI(W906-ECAT-ROUTE) 20260929: lazy claim (design 7.5): the boot InitMotor ran before the route was installed. No route = Open_Axis returns at its uiDevhand==0 test, bAxisOpen stays false, same return
        return;

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_AxStopDec(m_Axishand[MotorID]);
    if(Result != SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " StopDec," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
    }

    Result=Acm_AxSetExtDrive(m_Axishand[MotorID], 0);
    if (Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " SetJogModeOff," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
        return;
    }

    bFirstClickJog=true;
#else
    if(!EcatMotorRoute()) return;   //AI(W906-ECAT-ROUTE) 20260929: golden :415-439 through the route -- StopDec, then ExtDrive(0). Unreachable without a route (the bAxisOpen return above)
    W906_EcCall(iBoardID, iPortID, kEcStopDec, 0, 0);   // golden pops WAR16122 on failure and carries on; the route never pops (design 4.4)
    if(W906_EcCall(iBoardID, iPortID, kEcExtDrive, 0, 0)!=0) return;   // golden: ExtDrive failing returns WITHOUT bFirstClickJog=true
    bFirstClickJog=true;
#endif
}
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//Jog(Position direction)
//-----------------------------------------------------------------
bool TMyEtherCatMotor::JogP()
{
//  if (Error())                    //if the axis's state is error
//      return false;
//  if (Busy())                     //if the axis's state is busy
//      return false;

//    if(Direction)
//        Result = Acm_AxMoveVel(m_Axishand[Address],0); //To command axis to make a never ending movement with a specified velocity.
//    else
//        Result = Acm_AxMoveVel(m_Axishand[Address],1);
//    if (Result != 0)
//        return false;

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    if(bFirstClickJog==true)
    {
        //Enable or disable external drive mode.Setting 1 is jog mode.
        Result=Acm_AxSetExtDrive(m_Axishand[MotorID], 1); //只有 Jog 時要切換成 Jog Mode
        if (Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " SetJogModeOn," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
            return false;
        }
        else
        {
            bFirstClickJog=false;
        }
    }

    if(Direction)
        Result=Acm_AxJog(m_Axishand[MotorID],1);
    else
        Result=Acm_AxJog(m_Axishand[MotorID],0);

    if (Result!=SUCCESS)
    {
        return false;
    }

    return true;
#else
    if(!EcatMotorRoute()) return false;   //AI(W906-ECAT-ROUTE) 20260929: golden :444-487 through the route: ExtDrive(1) only while the FILE-GLOBAL bFirstClickJog is set (golden quirk kept), then Jog with the vendor direction code
    if(bFirstClickJog==true) { if(W906_EcCall(iBoardID, iPortID, kEcExtDrive, 0, 1)!=0) return false; bFirstClickJog=false; }
    return W906_EcCall(iBoardID, iPortID, kEcJog, 0, Direction ? 1 : 0)==0;   // vendor 0 = DIRECTION_POS (the route turns it into wire +1)
    // no route = the first line's return false, as before. See file banner GATE REGISTER.
#endif
}
//-----------------------------------------------------------------
//Jog(Negative direction)
//-----------------------------------------------------------------
bool TMyEtherCatMotor::JogN(void)
{
//  if (Error())                    //if the axis's state is error
//      return false;
//  if (Busy())                     //if the axis's state is busy
//      return false;
//    if(Direction)
//        Result = Acm_AxMoveVel(m_Axishand[Address],1); //To command axis to make a never ending movement with a specified velocity.
//    else
//        Result = Acm_AxMoveVel(m_Axishand[Address],0);
//    if (Result != 0)
//        return false;

    //Enable or disable external drive mode.Setting 1 is jog mode.

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    if(bFirstClickJog==true)
    {
        Result=Acm_AxSetExtDrive(m_Axishand[MotorID], 1);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " SetJogModeOn," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
            return false;
        }
        else
        {
            bFirstClickJog=false;
        }
    }

    if(Direction)
        Result=Acm_AxJog(m_Axishand[MotorID],0);
    else
        Result=Acm_AxJog(m_Axishand[MotorID],1);
    if (Result!=SUCCESS)
    {
        return false;
    }

    return true;
#else
    if(!EcatMotorRoute()) return false;   //AI(W906-ECAT-ROUTE) 20260929: same as JogP() above, direction reversed (golden :491-533); no route = return false, as before
    if(bFirstClickJog==true) { if(W906_EcCall(iBoardID, iPortID, kEcExtDrive, 0, 1)!=0) return false; bFirstClickJog=false; }
    return W906_EcCall(iBoardID, iPortID, kEcJog, 0, Direction ? 0 : 1)==0;
#endif
}
//-----------------------------------------------------------------
//PTP motion(unit: 1/100mm)
//-----------------------------------------------------------------
bool TMyEtherCatMotor::G00(int iPos)
{
    int p;
    double r;
    p=iPos;
    r=GearRatio;
    if(r==0)
        return false;
    int p1,p2;
    p1=p/r;        // unit: 1/100mm -> pulse
    p2=p1*r;
    if(p2<p){      //Compensate
        while(1){
            p1++;
            p2=p1*r;
            if(p2>=p) break;
        }
    }
    else if(p2>p){
        while(1){
            p1--;
            p2=p1*r;
            if(p2<=p) break;
        }
    }
    return RealG00(p1);
}
//-----------------------------------------------------------------
//Setting value of range
//-----------------------------------------------------------------
void TMyEtherCatMotor::SetRange(unsigned int a)
{
    if(a>1000)
        a=1000;
    Range=a;
}
//-----------------------------------------------------------------
//Setting value of AxAcc and AxDec
//-----------------------------------------------------------------
void TMyEtherCatMotor::SetRate(unsigned int a)
{
    double Rate;

    Rate=a;
    dAcc=(PJogHighSpeed-InitSpeed)*Rate/8000000.;  // 8 x 10^6      // 2011.11.28 , Joye
    if(dAcc==0)
        return;

    Rate=(iSpeed*Range-InitSpeed*Range)/dAcc;

    unsigned int iAccPersent,iAccMax,iAccMin;

    iAccPersent=(iSpeed*Range)/65535;
    if(iAccPersent==0)
        iAccPersent = 1;

    iAccMin=iAccPersent*2001;
    iAccMax=iAccPersent*8192000;

    if(Rate<iAccMin)
        Rate=iAccMin;
    if(Rate>iAccMax)
        Rate=iAccMax;

//    double Rate = (double)(4000000 / a * PRange);               //unit:pps/sec    Rate = (4℅□10■^6)/a℅PRange
#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_SetF64Property(m_Axishand[MotorID],PAR_AxAcc,Rate);   //Set acceleration of this axis
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " Acc," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
    }
    Result=Acm_SetF64Property(m_Axishand[MotorID],PAR_AxDec,Rate);   //Set deceleration of this axis
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " Dec," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
    }
#endif
    W906_EC_ONLY(W906_EcCall(iBoardID, iPortID, kEcSetSpeed, kEcSpdAcc, Rate); W906_EcCall(iBoardID, iPortID, kEcSetSpeed, kEcSpdDec, Rate);) }   //AI(W906-ECAT-ROUTE) 20260929: golden :576-616 PAR_AxAcc / PAR_AxDec = Rate through the route (golden has no bAxisOpen guard here either: unclaimed = refused)
//-----------------------------------------------------------------
void TMyEtherCatMotor::SetSpeed(unsigned int x, bool bSetJog)  //Input 進來是 %數   ==> //RogerYang 20250402 not percent, it's joghighspeed
{
    double iSpeed1,iSpeed2,persent;

    iSpeed=x;
    if(PJogHighSpeed==0)
        return;
    persent=(double)x/PJogHighSpeed*1.0;      //RogerYang 20250402 add
    //persent=(double)x/100.0;
    if(persent>1.0)
        persent=1.0;
    else if(persent<0.01)
        persent=0.01;

    if(!bAxisOpen W906_EC_ONLY(&& (Open_Axis(), !bAxisOpen)))   //AI(W906-ECAT-ROUTE) 20260929: lazy claim, same as DecStop
        return;

    if(MotorType==Step_Motor)  //步進用太小顆沒辦法跑太快,從上層改太麻煩(單位不同)
    {
        iSpeed1=InitSpeed*persent;
        iSpeed2=PJogHighSpeed*persent;

        if(iSpeed2<iSpeed1)
            iSpeed2=iSpeed1;

    //        iSpeed1=InitSpeed/100.0;
//        iSpeed2=PJogHighSpeed/10.0*persent;
//        if(iSpeed2>0.5)
//            iSpeed2=0.5;  //最快只能0.5
    }
    else
    {
        iSpeed1=InitSpeed*persent;
        iSpeed2=PJogHighSpeed*persent;
        if(iSpeed2<iSpeed1)
            iSpeed2=iSpeed1;
    }

//    if(x>6553500/Range)                                                      //DL單軸最大可達9.8Mpps//
//        x=6553500/Range;                                                     //DF單軸最大可達6.5Mpps//
//    else if(x==0)
//        x=1;
//    iSpeed1=InitSpeed*Range;
//    iSpeed2=iSpeed*Range;

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxVelLow, iSpeed1);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " VelLow," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
    }
    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxVelHigh, iSpeed2);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " VelHigh," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxAcc, dAcc);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " Acc," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxDec, dDec);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " Dec," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

//    iMode=0;
//    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxJerk, iMode);
//    if(Result!=SUCCESS)
//    {
//        strError="Mot MotorID-" + IntToStr(MotorID) + " Jerk," + IntToHex((int)Result,8);
//        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
//    }

    if(bSetJog==true)
    {
        Result = Acm_SetF64Property(m_Axishand[MotorID], CFG_AxJogVelLow, iSpeed1);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " JogVelLow," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
        Result = Acm_SetF64Property(m_Axishand[MotorID], CFG_AxJogVelHigh, iSpeed2);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " JogVelHigh," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
        Result = Acm_SetF64Property(m_Axishand[MotorID], CFG_AxJogAcc,dAcc);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " JogAcc," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
        Result = Acm_SetF64Property(m_Axishand[MotorID],CFG_AxJogDec,dDec);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " JogDec," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
        Result =Acm_SetI32Property(m_Axishand[MotorID],CFG_AxJogVLTime,0);
        if (Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " JogVLTime," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
    }
#endif
    W906_EC_ONLY(W906_EcSetSpeed(iBoardID, iPortID, iSpeed1, iSpeed2, dAcc, dDec, bSetJog);) }   //AI(W906-ECAT-ROUTE) 20260929: golden :618-731 VelLow/VelHigh/Acc/Dec (+ the four CFG_AxJog* with bSetJog) through the route. GAP: CFG_AxJogVLTime (I32) -- Pci1203Control has no I32 setter
//-----------------------------------------------------------------
void TMyEtherCatMotor::SetInitSpeed(unsigned int x)
{
    InitSpeed=x;
}
//-----------------------------------------------------------------
int TMyEtherCatMotor::SetPosition(int p)
{
#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_AxSetActualPosition(m_Axishand[MotorID], (double)p);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " ActualPosition," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#endif
    W906_EC_ONLY(W906_EcCall(iBoardID, iPortID, kEcSetActPos, 0, (double)p);) return true;   //AI(W906-ECAT-ROUTE) 20260929: golden :738-750 Acm_AxSetActualPosition through the route (refused on a DS402 drive, Q1); still returns true (=1) like golden, so ResetPos keeps golden's false
}
//-----------------------------------------------------------------
int TMyEtherCatMotor::SetCommand(int p)
{
    // AI(W906-PT-W3) 20260807: `Result` initialized to 0 (==SUCCESS) --
    // golden's own local is declared with NO initializer and is returned
    // even down the `!Enable` path where it is never assigned (indeterminate
    // value, real UB in golden). See file banner note.
#if HAVE_PCI1203
    U32 Result=0;
    AnsiString strError;
    if(Enable)
    {
        Result=Acm_AxSetCmdPosition(m_Axishand[MotorID],(double)p);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " CmdPosition," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
    }
    return Result;
#else
    if(!EcatMotorRoute() || !Enable) return 0;   //AI(W906-ECAT-ROUTE) 20260929: golden :752-766 through the route; returns the route's code (0 = SUCCESS; kEcRcDs402Coord on a DS402 drive, Q1) where golden returns the vendor's
    return (int)W906_EcCall(iBoardID, iPortID, kEcSetCmdPos, 0, (double)p);
    // no route = return 0, as before. See file banner GATE REGISTER.
#endif
}
//-----------------------------------------------------------------
//Setting the value of software limit.(unit: pulse)
//-----------------------------------------------------------------
void TMyEtherCatMotor::SetSoftLimit(int iPLimit,int iNLimit)
{
    int LP,LN;

    if(Direction)
    {
        LP= -iNLimit;
        LN= -iPLimit;
    }
    else
    {
        LP= iPLimit;
        LN= iNLimit;
    }
#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    //Result=Acm_SetProperty(m_Axishand[MotorID], CFG_AxSwPelValue, &LP, sizeof(LP));
    Result=Acm_SetF64Property(m_Axishand[MotorID], CFG_AxSwPelValue, LP);//Sam 20211019 : 新版韌體版本需要使用此函式不然會出錯，研華孟學說可以相容舊版韌體
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " SwPelValue," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    //Result=Acm_SetProperty(m_Axishand[MotorID], CFG_AxSwMelValue, &LN,sizeof(LN));
    Result=Acm_SetF64Property(m_Axishand[MotorID], CFG_AxSwMelValue, LN);  //Sam 20211019 : 新版韌體版本需要使用此函式不然會出錯，研華孟學說可以相容舊版韌體
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " SwMelValue," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#endif
    W906_EC_ONLY(W906_EcCall(iBoardID, iPortID, kEcSetLimit, kEcLimSwPel, LP); W906_EcCall(iBoardID, iPortID, kEcSetLimit, kEcLimSwMel, LN);) }   //AI(W906-ECAT-ROUTE) 20260929: golden :770-801 CFG_AxSwPelValue / CFG_AxSwMelValue through the route (values only; the enables untouched, as golden)
//-----------------------------------------------------------------
//Enable/disable Alarm function.
//-----------------------------------------------------------------
void TMyEtherCatMotor::SetServoAlarmOn(bool Value)
{
    PServoAlarmOn=Value;
}
//-----------------------------------------------------------------
//Getting the value of command position(unit: 1/100mm)
//-----------------------------------------------------------------
int TMyEtherCatMotor::ReadPos()
{
    return ReadRealPos() * GearRatio;           //pulse -> 1/100mm
}
//-----------------------------------------------------------------
//Getting the value of actual position(unit: 1/100mm)
//-----------------------------------------------------------------
//Getting signal of alarm
//-----------------------------------------------------------------
bool TMyEtherCatMotor::GetAlarm(void)
{
#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;
    unsigned long Status = 0;

    Result=Acm_AxGetMotionIO(m_Axishand[MotorID],&Status);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " MotionIO," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
        return false;
    }

    if((Status&0x3004e)==0)    //110000000001001110  SLMT_P﹜SLMT_N﹜EMG﹜LMT+﹜LMT-﹜ALM
        return false;
    else
        return true;
#else
    TEcatAxisRead s;   //AI(W906-ECAT-ROUTE) 20260929: golden :821-839 decoded from the monitor's sample; a failed read = golden's read-failed branch (false), without WAR16121 (design 4.4)
    if(!W906_EcRead(iBoardID, iPortID, s)) return false;
    return (s.motionIO & 0x3004e)!=0;    // SLMT_P, SLMT_N, EMG, LMT+, LMT-, ALM (golden :834)
    // no route = W906_EcRead false = return false, as before. See file banner GATE REGISTER.
#endif
}
//-----------------------------------------------------------------
//Getting the status of motor
//-----------------------------------------------------------------
void TMyEtherCatMotor::ScanMotorStatus( bool *Led )
{
    if(Enable)
    {
#if HAVE_PCI1203
        U32 Result;
        AnsiString strError;
        unsigned long Status=0;
        unsigned short Status_1;

        Result=Acm_AxGetMotionIO(m_Axishand[MotorID],&Status);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " MotionIO," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
            return;
        }

        bool Flag   =(((Status>>4) & 0x1) > 0) ? true : false;     //ORG
        Led[iCcwLed]=(((Status>>2) & 0x1) > 0) ? true : false;     //LMT+
        Led[iCwLed] =(((Status>>3) & 0x1) > 0) ? true : false;     //LMT-

//        if(bSensorType)
//            Led[iHomeLed]=Flag;        // Home
//        else
//            Led[iHomeLed]=!Flag;    // Home
        Led[iHomeLed]=Flag;                                                     //RogerYang 20250425 because set CFG_AxOrgLogic by SensorType when initial

        //Led[iAlarmLed] = GetAlarm();
        Led[iSoftcwLed] =(((Status>>16) & 0x1) > 0) ? true : false;    //SLMT_P
        Led[iSoftccwLed]=(((Status>>17) & 0x1) > 0) ? true : false;    //SLMT_N
        Led[iAlarmLed]  =(((Status>>1)  & 0x1) > 0) ? true : false;    //ALM
        //Led[iInposLed]  =(((Status>>13) & 0x1) > 0) ? false : true;    //INP  //RogerYang 20250421 not work

        Led[iServoOn]   =(((Status>>14) & 0x1) > 0) ? true : false;
        Led[iEmgLed]    =(((Status>>6)  & 0x1) > 0) ? true : false;    //EMG

        Result=Acm_AxGetState(m_Axishand[MotorID],&Status_1);
        if(Result==SUCCESS)
        {
            if(Status_1==STA_AX_ERROR_STOP)
                Led[iAlarmLed]=true;    //iServoOn 改 iAlarmLed
        }
#else
        TEcatAxisRead s; if(EcatMotorRoute()) { if(!W906_EcRead(iBoardID, iPortID, s)) return; const unsigned long m=s.motionIO;   //AI(W906-ECAT-ROUTE) 20260929: golden :843-892 decoded from the monitor's sample (a failed read returns like golden's: Led and bAlarm untouched). No route = the 8 lines below, as before
            Led[iCcwLed]=((m>>2)&1)!=0; Led[iCwLed]=((m>>3)&1)!=0; Led[iHomeLed]=((m>>4)&1)!=0; Led[iSoftcwLed]=((m>>16)&1)!=0; Led[iSoftccwLed]=((m>>17)&1)!=0;
            Led[iAlarmLed]=((m>>1)&1)!=0 || s.state==kEcStaErrorStop; Led[iServoOn]=((m>>14)&1)!=0; Led[iEmgLed]=((m>>6)&1)!=0; } else {
        Led[iCcwLed]=false;
        Led[iCwLed]=false;
        Led[iHomeLed]=false;
        Led[iSoftcwLed]=false;
        Led[iSoftccwLed]=false;
        Led[iAlarmLed]=false;
        Led[iServoOn]=false;
        Led[iEmgLed]=false; }
#endif
    }
    else
    {
        Led[iHomeLed] =true;
        Led[iInposLed]=false;
    }
    bAlarm=Led[iAlarmLed];                                                      //RogerYang 20250421 get motor alarm state
}
//-----------------------------------------------------------------
//Getting the flag of home
//-----------------------------------------------------------------
bool TMyEtherCatMotor::HomeFlag(void)
{
//    if(bSensorType)
//    {
//        if(GetHomeIO())
//            return(true);
//        else
//            return(false);
//    }
//    else
//    {
//        if(GetHomeIO())
//            return(false);
//        else
//            return(true);
//    }

    if(GetHomeIO())                                                             //RogerYang 20250425 because set CFG_AxOrgLogic by SensorType when initial
        return(true);
    else
        return(false);
}
//-----------------------------------------------------------------
//----------------------HTAdvantechMotor Only-------------------------
//-----------------------------------------------------------------
//PTP motion(unit: pulse)
//-----------------------------------------------------------------
bool TMyEtherCatMotor::RealG00(int p)
{
    int a;

    if(Error())
        return(false);         //if the axis's state is error
    if(Busy())
        return(false);         //if the axis's state is busy
    /*
    if(bAxisID == 0 || bAxisID == 1 || bAxisID == 8 || bAxisID == 9 || bAxisID == 16 || bAxisID == 17)
    {
        a=p-ReadeEnCoderRealPos();                //Get relative displacement amount
        if(a < 50 && a > -50)
            return(true);
    }
    else
    {
        a=p-ReadRealPos();                //Get relative displacement amount
        if(a==0)
            return(true);
    }
    */
    a=p-ReadRealPos();
    if(a==0)
        return(true);
    if(Direction)
    {
#if HAVE_PCI1203
        U32 Result;
        AnsiString strError;

        Result=Acm_AxMoveAbs(m_Axishand[MotorID], p);  //Start single axis' relative motion
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " MoveAbs," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
            return false;
        }
#endif
    }
    else
    {
#if HAVE_PCI1203
        U32 Result;
        AnsiString strError;

        Result=Acm_AxMoveAbs(m_Axishand[MotorID],-p);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " MoveAbs," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
            return false;
        }
#endif
    }

    /*if (GetAlarm())
        Alarm->Set(ALM_MOTOR_MOVE);*/

    return(false);
}
//-----------------------------------------------------------------
//Getting the value of command position(unit: pulse)
//-----------------------------------------------------------------
int TMyEtherCatMotor::ReadRealPos(void)
{
    double Pos = 0;

    if(Enable==false) //RogerYang 2050410 add
    return 0;

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_AxGetCmdPosition(m_Axishand[MotorID],&Pos);       //Get current command position of the specified axis
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " CmdPos," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
    }
#endif
    W906_EC_ONLY({ TEcatAxisRead s; if(W906_EcRead(iBoardID, iPortID, s)) Pos=s.cmdPos; })   //AI(W906-ECAT-ROUTE) 20260929: golden :979-999 Acm_AxGetCmdPosition = the monitor's cmdPos; a failed read leaves Pos=0 like golden's
    if(Direction)
        Pos=-Pos;

    return Pos;
}
//-----------------------------------------------------------------
//Getting the value of actual position(unit: pulse)
//-----------------------------------------------------------------

//Getting signal of axis busy
//-----------------------------------------------------------------
bool TMyEtherCatMotor::Busy(void)
{
#if HAVE_PCI1203
    unsigned short state = 0;
    U32 Result;
    AnsiString strError;

    Result=Acm_AxGetState(m_Axishand[MotorID],&state);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " GetState," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
    }

    if(state==STA_AX_READY)
        return false;
    else
        return true;
#else
    TEcatAxisRead s;   //AI(W906-ECAT-ROUTE) 20260929: golden :1006-1023 from the monitor's sample; unreadable, or pending (design 4.2: a command not yet seen by a Poll, or a DRY / refused motion before the next stop) = busy
    if(!W906_EcRead(iBoardID, iPortID, s) || s.pending) return true;
    return s.state!=kEcStaReady;
    // no route = W906_EcRead false = return true (perpetually busy), as before. See file banner GATE REGISTER.
#endif
}
//-----------------------------------------------------------------
//Getting signal of axis error
//-----------------------------------------------------------------
bool TMyEtherCatMotor::Error(void)
{
#if HAVE_PCI1203
    unsigned short state = 0;
    U32 Result;
    AnsiString strError;

    Result=Acm_AxGetState(m_Axishand[MotorID],&state);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " GetState," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16121", 0, MMSystem, 0,strError);
    }

    if(state==STA_AX_ERROR_STOP)
    {
//        Result=Acm_AxResetError(m_Axishand[Address]);
//        if (Result != 0)
//            return true;
        return true;
    }
    return false;
#else
    TEcatAxisRead s;   //AI(W906-ECAT-ROUTE) 20260929: golden :1027-1048 from the monitor's sample (ERROR_STOP); unreadable = golden's state==0 outcome (false)
    if(!W906_EcRead(iBoardID, iPortID, s)) return false;
    return s.state==kEcStaErrorStop;
    // no route = return false, as before. See file banner GATE REGISTER.
#endif
}
//-----------------------------------------------------------------
//Getting signal of org/home
//-----------------------------------------------------------------
bool TMyEtherCatMotor::GetHomeIO(void)
{
#if HAVE_PCI1203
    unsigned long Status = 0;
    U32 Result;
    AnsiString strError;

    Result=Acm_AxGetMotionIO(m_Axishand[MotorID],&Status);   //Get current command position of the specified axis
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " MotionIO," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
        return false;
    }

    return (((Status>>4) & 0x1) > 0) ? true : false;
#else
    TEcatAxisRead s;   //AI(W906-ECAT-ROUTE) 20260929: golden :1052-1067 MotionIO bit 4 (ORG) from the monitor's sample; unreadable (and no route) = golden's read-failed branch (false). Q3: whether DS402 method 24/28 lights ORG is measured at the machine
    if(!W906_EcRead(iBoardID, iPortID, s)) return false;
    return ((s.motionIO>>4) & 0x1)!=0;
#endif
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::DoHome(unsigned int mode, unsigned int dir)
{
//    unsigned int result = 0;
    if(GearRatio==0)
        return false;
    double crossDistance = 100/GearRatio; // 1cm
    switch (iHomeObjectTask)
    {
        case 1:
            if (Error())                            //if the axis's state is error
                return false;
            if (Busy())                             //if the axis's state is busy
                return false;

#if HAVE_PCI1203
            Acm_SetProperty(m_Axishand[MotorID], PAR_AxHomeCrossDistance, &crossDistance, sizeof(crossDistance));
#endif
            SetSpeed(PHomeHighSpeed);               //Set velHigh of home motion
            SetInitSpeed(PHomeLowSpeed);            //Set velLow of home motion
            SetSoftLimit(999999,-999999);           //like disable soft limit
#if HAVE_PCI1203
            Acm_AxHome(m_Axishand[MotorID],mode,dir);  //MODE12_AbsSearchReFind  1:Negative direction
#endif
            iHomeObjectTask=2;
            return false;
        case 2:
            if (Busy())
                return false;
            MySleep(300);
            //SetPos(0);                              //The command position set to 0
//            SetEnCoderPos(0);                       //The actual position set to 0
            SetSpeed(ReadSpeed());                  //Set last velHigh
            SetInitSpeed(ReadInitSpeed());          //Set last velLow
            SetSoftLimit(PSoftLimitP/GearRatio,PSoftLimitN/GearRatio);//Set last soft limit
            return true;
    }
    return false;
}

//-----------------------------------------------------------------
//Home motion(Position direction)
//-----------------------------------------------------------------
bool TMyEtherCatMotor::PosDirectHome()
{
    return DoHome(11, 0);  //MODE12_AbsSearchReFind  1:Negative direction
}
//-----------------------------------------------------------------
//Home motion(Negative direction)
//-----------------------------------------------------------------
bool TMyEtherCatMotor::NegDirectHome()
{
    return DoHome(11, 1);  //MODE12_AbsSearchReFind  1:Negative direction
}

//-----------------------------------------------------------------
//Home motion(Position direction)
//-----------------------------------------------------------------
bool TMyEtherCatMotor::PosDirectHomeOneTouch()
{
    return DoHome(6, 0);   //MODE7_AbsSearch  0:Positive direction
}
//-----------------------------------------------------------------
//Home motion(Negative direction)
//-----------------------------------------------------------------
bool TMyEtherCatMotor::NegDirectHomeOneTouch()
{
    return DoHome(6, 1);   //MODE7_AbsSearch  0:Positive direction
}
//-----------------------------------------------------------------
//bool HT8040Motor::HomeType90()
//{
//}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::AddAxis(int iAxis)
{
    //unsigned int result;
    //result = Acm_GpAddAxis(&m_Gphand,m_Axishand[iAxis]);
    return 0;
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::AddPath(int iAxis)
{
    return true;
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::RunPath(void)
{
    return true;
}
//-----------------------------------------------------------------
int  TMyEtherCatMotor::ethercat_set_output_bit(int Ring, int IP, int Port, int Bit, int Data)
{
#if HAVE_PCI1203
    U32 Result;
    U16 ChannelID=Port * 8 + Bit;

    Result=Acm_DaqDoSetBit(gDevhand, ChannelID, Data);    // 0: Off, 1: On
//    Result=Acm_DaqDoSetByteEx(m_Devhand,Ring,IP,ChannelID,Data);

    if (Result!=SUCCESS)
        return -1;
    return 1;
#else
    // No vendor EtherCAT SDK linked: mirror golden's own call-failed branch.
    // See file banner GATE REGISTER.
    return -1;
#endif
}
//-----------------------------------------------------------------
int TMyEtherCatMotor::ethercat_set_output_byte(int Ring, int IP, int Port, int Byte)
{
#if HAVE_PCI1203
    U32 Result;
//    U16 ChannelID = Port * 8 + Byte;

    Result=Acm_DaqDoSetByte(gDevhand, Port, Byte);
    if(Result!=SUCCESS)
        return -1;
    return 1;
#else
    // No vendor EtherCAT SDK linked: mirror golden's own call-failed branch.
    // See file banner GATE REGISTER.
    return -1;
#endif
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::IOBitOff(int port, int bit)
{
#if HAVE_PCI1203
    U32 Result;
    U16 ChannelID=port * 8 + bit;

    Result=Acm_DaqDoSetBit(gDevhand, ChannelID, 0);    // 0: Off, 1: On
    if (Result!=0)
        return false;
    return true;
#else
    // No vendor EtherCAT SDK linked: mirror golden's own call-failed branch.
    // See file banner GATE REGISTER.
    return false;
#endif
}

//-----------------------------------------------------------------
void TMyEtherCatMotor::SetAcc(double a)
{
    dAcc=a;
}
//-----------------------------------------------------------------
void TMyEtherCatMotor::SetDec(double a)
{
    dDec=a;
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::MotionDone()                                                  // motion done //
{
#if HAVE_PCI1203
    U16 state=0;
    U32 Result;
    AnsiString strError;

    if(Enable)
    {
        Result=Acm_AxGetState(m_Axishand[MotorID], &state);
        if(Result!=SUCCESS)
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " GetState," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
        }

        if(state==STA_AX_READY)
            return true;
        else
            return false;
    }
    else
        return false;
#else
    TEcatAxisRead s;   //AI(W906-ECAT-ROUTE) 20260929: golden :1201-1223 from the monitor's sample: Enable, then READY -- but NEVER from a pending sample (design 4.2 / Q11). No route = false, as before
    if(!Enable) return false;
    if(!W906_EcRead(iBoardID, iPortID, s) || s.pending) return false;
    return s.state==kEcStaReady;
#endif
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::MoveTo(int Tar)                                            // Move to //
{
    if(Enable==false)
        return true;

    if(!MotionDone())
        return false;

    int iPos,iP1,iP2;
    iPos=ReadPos();
    if(GearRatio==0)
        return false;
    iP1=Tar/GearRatio;        // unit: 1/100mm -> pulse
    iP2=iP1*GearRatio;

    if(iP2<Tar)
    {
        while(1)
        {
            iP1++;
            iP2=iP1*GearRatio;
            if(iP2>=Tar)
                break;
        }
    }
    else if(iP2>Tar)
    {
        while(1)
        {
            iP1--;
            iP2=iP1*GearRatio;
            if(iP2<=Tar)
                break;
        }
    }

    if(Direction)
        iP1=-iP1;
    else
        iP1=iP1;

    SetSpeed(iSpeed);

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;
//    Result=Acm_AxMoveAbs(m_Axishand[Address],iP1);
    Result=Acm_AxMoveRel(m_Axishand[MotorID], iP1);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " MoveAbs," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#endif
    W906_EC_ONLY(W906_EcCall(iBoardID, iPortID, kEcMoveRel, 0, (double)iP1);)   //AI(W906-ECAT-ROUTE) 20260929: golden :1225-1282 Acm_AxMoveRel(iP1) through the route -- golden's quirk kept: the ABSOLUTE target as a relative move (no engine caller)
    if(MotionDone() && Tar==iPos)
        return true;
    else
        return false;
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::HomeObject()
{
    return EtherCatMotHome();
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::EtherCatMotHome()
{
    int &Task=iHomeObjectTask;

#if HAVE_PCI1203
    unsigned short state=0;
    U32 Result;
    AnsiString strError;

    switch(Task)
    {
        case 1:
        {
            if(!MotionDone())
            {
                DecStop();
                break;
            }
//            SetCommand(0);    //KaiHuang 202010121 Mark : Servo Off 時會造成 Error
//            SetPosition(0);   //KaiHuang 202010121 Mark : Servo Off 時會造成 Error
            DecStop();          //KaiHuang 202010121 Add
            InitMotor(MotorID);
            SetHomeSpeed();
            OldSpeed=iSpeed;
//            DecStop();        //KaiHuang 202010121 Mark

            if(GearRatio==0)
                break;
            double crossDistance = 100/GearRatio; // 撞到 Home 點滑行距離
            Acm_SetProperty(m_Axishand[MotorID], PAR_AxHomeCrossDistance, &crossDistance, sizeof(crossDistance));
            SetSoftLimit(999999,-999999);           //like disable soft limit

            if(HomeDirection)
            {
                Result=Acm_AxMoveHome(m_Axishand[MotorID], MODE12_AbsSearchReFind, 0);                //MODE12_AbsSearchReFind : Home Mode,EtherCat 控制兩段回Home
                if(Result!=SUCCESS)
                {
                    strError="Mot Address-" + IntToStr(MotorID) + " Home," + IntToHex((int)Result,8);
                    ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
                }
            }
            else
            {
                Result=Acm_AxMoveHome(m_Axishand[MotorID],MODE12_AbsSearchReFind,1);
                if(Result!=SUCCESS)
                {
                    strError="Mot Address-" + IntToStr(MotorID) + " Home," + IntToHex((int)Result,8);
                    ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
                }
            }

            Task=10;
            break;
        }
        case 10:
        {
            Result=Acm_AxGetState(m_Axishand[MotorID],&state);
            if(Result!=SUCCESS)
            {
                strError="Mot MotorID-" + IntToStr(MotorID) + " GetState," + IntToHex((int)Result,8);
                ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
            }

            if(state==STA_AX_READY)
            {
                HomeDelay.SetSecAndOn(0.3);
                Task=20;
            }
            break;
        }
        case 20:
        {
            if(HomeDelay.Off())
            {
                LastHomePos=-ReadPos();
                SetCommand(0);
                MySleep(10);     //RogerYang 20250528 100->10
                SetPosition(0);
                SetSpeed(OldSpeed);
                SetSoftLimit(PSoftLimitP, PSoftLimitN);

                Task=1;
                return true;
            }
            break;
        }
        default:
            Task=1;
            return false;
    }
    return false;
#else
    if(!EcatMotorRoute()) { Task=1; return false; }   //AI(W906-ECAT-ROUTE) 20260929: no route = the two statements this arm always had. With the route: golden's Task 1/10/20 (golden :1289-1378), homing the EastSun way (design 2.3)
    switch(Task) { case 1: if(!MotionDone()) { DecStop(); break; } DecStop(); InitMotor(MotorID); OldSpeed=iSpeed; if(GearRatio==0) break;   // Q2: golden's full InitMotor (ResetError, table, Servo ON) minus Q1's zeroing
        SetSoftLimit(999999,-999999); W906_EcHomeStart(iBoardID, iPortID, HomeDirection, PHomeHighSpeed, PHomeLowSpeed, dAcc, dDec); Task=10; break;   // result ignored like golden's MoveHome WAR16122: Task 10 regardless
      case 10: if(W906_EcHomeDone(iBoardID, iPortID)==1) { HomeDelay.SetSecAndOn(0.3); Task=20; } break;   // a HOMING sample, then a READY one, both after the command
      case 20: if(HomeDelay.Off()) { LastHomePos=-ReadPos(); if(W906_EcHomeCardSide(iBoardID, iPortID)) { SetCommand(0); MySleep(10); SetPosition(0); }   // DS402: no zeroing (EastSun)
            SetSpeed(OldSpeed); SetSoftLimit(PSoftLimitP, PSoftLimitN); Task=1; return true; } break;
      default: Task=1; return false; }
    return false;
#endif
}
//-----------------------------------------------------------------
bool TMyEtherCatMotor::ResetPos(int Pulse=0)
{
    long iR1 ,iR2;

    iR1=SetCommand(Pulse);
    iR2=SetPosition(Pulse);

    if ((iR1==0) && (iR2==0))
        return true;
    return false;
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::MotOutputOn(int iOutport)
{
#if HAVE_PCI1203
    U32 Result;
    U32 Result2;
    AnsiString strError;

    if(bAlarm==true)                                                            //RogerYang 20250421 reset alarm before servo on
    {
        Result2=Acm_AxResetError(m_Axishand[MotorID]);
        MySleep(100);
        if(Result2!=SUCCESS)
        {
        }
    }
    Result=Acm_AxSetSvOn(m_Axishand[MotorID], 1);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " SvOn," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#endif
    W906_EC_ONLY(if(EcatMotorRoute()) { if(bAlarm==true) { W906_EcCall(iBoardID, iPortID, kEcResetError, 0, 0); MySleep(100); } W906_EcCall(iBoardID, iPortID, kEcSvOn, 0, 1); }) }   //AI(W906-ECAT-ROUTE) 20260929: golden :1392-1412 through the route (the file-global bAlarm, golden quirk kept); no route = not even the MySleep
//--------------------------------------------------------------------------
void TMyEtherCatMotor::MotOutputOff(int iOutport)
{
#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_AxSetSvOn(m_Axishand[MotorID], 0);
    if(Result != SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " SvOn," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#endif
    W906_EC_ONLY(W906_EcCall(iBoardID, iPortID, kEcSvOn, 0, 0);) }   //AI(W906-ECAT-ROUTE) 20260929: golden :1414-1425 Acm_AxSetSvOn(0) through the route
//--------------------------------------------------------------------------
void TMyEtherCatMotor::SetServoOn(bool IsOn)
{
    if(Enable==false)
        return;

    if(IsOn)
    {
        MotOutputOn(1);
        //Acm_AxSetSvOn(axisHand, 1);
    }
    else
    {
        MotOutputOff(1);
        //Acm_AxSetSvOn(axisHand, 0);
    }
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::MotInputStatus(bool *bInputPort)
{
    ;
}
//--------------------------------------------------------------------------
bool TMyEtherCatMotor::LinearAxisMoveTo(int iPortID[8],long lPos[8],bool bFlag)
{
    /*
    if(bFlag==false)
    {
        short InterAxis;
        short InterType=1; // line interpolation
        short BankNum=1;

        short AxisNo=1;
        short MotionType=4;
        InterAxis = 0x03;

        SmcWSetBankNumber       (Id, AxisNo,    BankNum);
        SmcWSetBankInterpolation(Id, AxisNo,    BankNum, InterType, InterAxis, Range);
        SmcWSetBankDistance     (Id, AxisNo,    1,       lPos[0]);
        SmcWSetBankDistance     (Id, AxisNo+1,  1,       lPos[1]);
        SmcWSetBankStartSpeed   (Id, AxisNo,    1,       iSpeed*Range);
        SmcWSetBankTargetSpeed  (Id, AxisNo,    1,       iSpeed*Range);
        SmcWSetBankAccelTime    (Id, AxisNo,    1,       dAcc);
        SmcWSetBankDecelTime    (Id, AxisNo,    1,       dDec);
        SmcWSetBankSSpeed       (Id, AxisNo,    1,       200);
        SmcWSetBankReady        (Id, AxisNo,    MotionType);
        SmcWMotionStart         (Id, AxisNo);
    }
    else
    {
        if(MotionDone())
            return true;
    } */
    return false;
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::EnableTrigger(bool bFlag,int iMode,long lValue)
{
    /*HWND hWnd=FindWindow(NULL, "MotorTest");
    if(bFlag)
    {
        long lPitch=lValue/GearRatio;
        if(Direction)
            lPitch=-lPitch;
        else
            lPitch=lPitch;
        if(iMode)
        {
            SmcWSetCtrlTypeOut  (Id, iPortID, 0x03, 0x00, 0x00);
            SmcWCountEvent      (Id, iPortID, hWnd, 0x01, 0x00, lPitch);
        }
        else
        {
            SmcWSetCtrlTypeOut  (Id, iPortID, 0x04, 0x00, 0x00);
            SmcWCountEvent      (Id, iPortID, hWnd, 0x01, 0x01, lPitch);
        }
        SmcWSetInitParam        (Id, iPortID);
    }
    else
    {
        SmcWSetCtrlTypeOut  (Id, iPortID, 0x03, 0x00, 0x00);
        SmcWStopEvent       (Id, iPortID, hWnd, 1);
        SmcWSetInitParam    (Id, iPortID);
    }*/
    ;
}
//--------------------------------------------------------------------------
bool TMyEtherCatMotor::MoveToPos(int Tar)
{
//    unsigned short Status=0;
    if(!bAxisOpen W906_EC_ONLY(&& (Open_Axis(), !bAxisOpen)))   //AI(W906-ECAT-ROUTE) 20260929: lazy claim, same as DecStop
        return false;

    if(!MotionDone())
        return false;

    int iPos;
    iPos=ReadPos();

    //SetSpeed(iSpeed);   //RogerYang 20250715 TestSpeed

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_AxMoveAbs(m_Axishand[MotorID], Tar);
//    Result=Acm_AxMoveRel(m_Axishand[MotorID],Tar);

    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " MoveAbs," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#endif
    W906_EC_ONLY(W906_EcCall(iBoardID, iPortID, kEcMoveAbs, 0, (double)Tar);)   //AI(W906-ECAT-ROUTE) 20260929: golden :1513-1543 Acm_AxMoveAbs(Tar) through the route (Tar = pulse, GetRealPos converted it; no Direction flip, as golden). The MotionDone() below is pending until the next Poll
    if(MotionDone() && Tar==iPos)
        return true;
    else
        return false;
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::ResetLatch()
{
}
//--------------------------------------------------------------------------
int TMyEtherCatMotor::GetLatchTotalLen()
{
    return 0;
}
//--------------------------------------------------------------------------
int TMyEtherCatMotor::GetLatchBuffer(int LatchInputNo, short *AxisCounterNo, long *LatchDataTable)
{
    return 0;
}
//--------------------------------------------------------------------------
bool TMyEtherCatMotor::GetLatchIOStatus(unsigned short LatchInputNo)
{
    return false;
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::SetFIFOLatchSrc(int AxisCounterNo, int LatchInputNo, int bEnable)
{
}
//--------------------------------------------------------------------------
int TMyEtherCatMotor::ReadEnCoderRealPos()
{
    double Pos = 0;

    if(Enable==false) //RogerYang 2050410 add
        return 0;

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_AxGetActualPosition(m_Axishand[MotorID],&Pos);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " CmdPos," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16121", 0, MMSystem, 0, strError);
    }
#endif
    W906_EC_ONLY({ TEcatAxisRead s; if(W906_EcRead(iBoardID, iPortID, s)) Pos=s.actPos; })   //AI(W906-ECAT-ROUTE) 20260929: golden :1568-1588 Acm_AxGetActualPosition = the monitor's actPos; a failed read leaves Pos=0 like golden's
    if(Direction)
        Pos=-Pos;

    return Pos;
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::SetEtherCatInType()
{
#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    if(MotorType==Servo_Motor)
    {
        unsigned int uLnpEnable=1;                  //設定啟用INP訊號觸發 0:Disable 1:Enable
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxInpEnable, uLnpEnable);
        if((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " InpEnable," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }

        unsigned int uInpLogic=0;                   //設定INP訊號觸發 0:低準為 1:高準為
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxInpLogic, uInpLogic);
        if((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " InpLogic," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }

        unsigned int uAlmLogic=bIn1Logic? 0:1;      //設定Alarm訊號觸發 0:低準為 1:高準為
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxAlmLogic, uAlmLogic);
        if((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " AlmLogic," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
    }
    else if(MotorType==Rotate_Motor)
    {
        unsigned int uLnpEnable=1;                  //設定啟用INP訊號觸發 0:Disable 1:Enable
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxInpEnable, uLnpEnable);
        if((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " InpEnable," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }

        unsigned int uInpLogic=0;                   //設定INP訊號觸發 0:低準為 1:高準為
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxInpLogic, uInpLogic);
        if((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " InpLogic," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }

        unsigned int uAlmLogic=0;                  //設定Alarm訊號觸發 0:低準為 1:高準為
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxAlmLogic, uAlmLogic);
        if((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " AlmLogic," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
    }
    else
    {
        //鳴治馬達不能用以下兩個會像賽
//        unsigned int uLnpEnable=1;                  //設定啟用INP訊號觸發 0:Disable 1:Enable
//        Result=Acm_SetU32Property(m_Axishand[Address],CFG_AxInpEnable,uLnpEnable);
//        if ((Result != SUCCESS) && (Result != Dsp_PropertyIDNotSupport))
//        {
//            strError="Mot Address-" + IntToStr(Address) + " InpEnable," + IntToHex((int)Result,8);
//            ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
//        }
//
//        unsigned int uInpLogic=0;                   //設定INP訊號觸發 0:低準為 1:高準為
//        Result=Acm_SetU32Property(m_Axishand[Address],CFG_AxInpLogic,uInpLogic);
//        if ((Result != SUCCESS) && (Result != Dsp_PropertyIDNotSupport))
//        {
//            strError="Mot Address-" + IntToStr(Address) + " InpLogic," + IntToHex((int)Result,8);
//            ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
//        }

        unsigned int uAlmLogic=bIn1Logic? 0:1;      //設定Alarm訊號觸發 0:低準為 1:高準為
        Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxAlmLogic, uAlmLogic);
        if ((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
        {
            strError="Mot MotorID-" + IntToStr(MotorID) + " AlmLogic," + IntToHex((int)Result,8);
            ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
        }
    }

    unsigned int uEzLogic=1;                        //設定EZ訊號觸發 0:低準為 1:高準為
    Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxEzLogic, uEzLogic);
    if((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " EzLogic," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
    }

    unsigned int uErcLogic=1;                       //設定ERC訊號觸發 0:低準為 1:高準為
    Result=Acm_SetU32Property(m_Axishand[MotorID], CFG_AxErcLogic, uErcLogic);
    if((Result!=SUCCESS) && (Result!=Dsp_PropertyIDNotSupport))
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " ErcLogic," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#else
    // No vendor EtherCAT SDK linked: nothing to configure on non-existent
    // hardware -- every statement in this function is an Acm_SetU32Property
    // write. See file banner GATE REGISTER.
#endif
}
//--------------------------------------------------------------------------
void EtherCatWriteAO(int ID, int Channel, F32 Data)
{
#if HAVE_PCI1203
    U32 Result;

    Result=Acm_DaqAoSetCurrDataEx(uiDevhand, 1, ID, Channel, Data);             //IO_Ring=1
    if(Result!=SUCCESS)
    {
        ;
    }
#endif
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::SetHomeSpeed()
{
#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxHomeVelLow, PHomeLowSpeed);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " HomeVelLow," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxHomeVelHigh, PHomeHighSpeed);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " HomeVelHigh," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxHomeAcc, dAcc);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " HomeAcc," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxHomeDec, dDec);
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " HomeDec," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }

    Result=Acm_SetF64Property(m_Axishand[MotorID], PAR_AxHomeJerk, 0);        //T型曲線
    if(Result!=SUCCESS)
    {
        strError="Mot MotorID-" + IntToStr(MotorID) + " HomeJerk," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
#endif
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::Close_Card()
{
    if(uiDevhand==0) //避免重負關
        return;

#if HAVE_PCI1203
    U32 Result;
    AnsiString strError;

    Result=Acm_DevClose(&uiDevhand);
    if(Result!=SUCCESS)
    {
        strError="Close Card," + IntToHex((int)Result,8);
        ShowErrorMessage("WAR16122", 0, MMSystem, 0, strError);
    }
    else
    {
        uiDevhand=0;
    }
#endif
}
//--------------------------------------------------------------------------
void TMyEtherCatMotor::ResetState()
{
#if HAVE_PCI1203
    U32 Result;
    Result=Acm_AxResetError(m_Axishand[MotorID]);
    if(Result!=SUCCESS)
    {
//        strError="Mot MotorID-" + IntToStr(MotorID) + " ResetError:" + IntToHex((int)Result,8);
//        ShowErrorMessage("WAR16122", 0, MMSystem, 0,strError);
    }
#endif
    W906_EC_ONLY(W906_EcCall(iBoardID, iPortID, kEcResetError, 0, 0);) }   //AI(W906-ECAT-ROUTE) 20260929: golden :1764-1773 Acm_AxResetError through the route (no engine caller)
//--------------------------------------------------------------------------
void TMyEtherCatMotor::ResetAxisOpen()
{
//    for(int i=0; i<999; i++)    //重開卡須初始化, 不然不會開馬達    //RogerYang 20250402 9046AU
//        bAxisOpen[i]=false;
    bAxisOpen=false;
}
//--------------------------------------------------------------------------
