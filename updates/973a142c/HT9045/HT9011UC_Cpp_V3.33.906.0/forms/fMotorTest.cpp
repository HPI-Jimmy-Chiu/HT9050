// =============================================================================
//  forms/fMotorTest.cpp  --  the READ-ONLY half of golden's TfMotorTest
//  (uMotorTest.cpp).  35 of golden's 93 method definitions.
//
//  AI(W906-FW-MOTORTEST-W24) 20260826: new file, translation wave
//  FW-MOTORTEST-W24.  Golden source of truth:
//  HT9011UC_Code_V3.33.906.0_20260618/uMotorTest.cpp (2,377 lines, cp950,
//  0 U+FFFD, this wave).  The full banner -- SAFETY POSTURE, denominator,
//  scope table, GATE REGISTER, GOLDEN NOTES, DEVIATIONS, CMAKE STATUS --
//  lives in forms/fMotorTest.h.  This file carries per-statement
//  pointer-back comments only.
//
//  ==========================================================================
//  *** HOW THE 58 GATES ARE ENFORCED IN THIS FILE -- READ BEFORE EDITING ***
//  ==========================================================================
//  The 52 motion / safety-critical methods (GATE S-01..S-52), the one
//  machine-config WRITE (W-01) and the five unportable ones (X-01..X-05) are
//  DECLARED in forms/fMotorTest.h and DELIBERATELY NOT DEFINED here.
//
//  That is the gate mechanism, chosen over the tree's usual
//  `#if 0`-around-a-verbatim-body idiom (Motor/mymotor.cpp:895/:1765/:2270)
//  precisely because this form commands motors:
//    * An EMPTY STUB would link, do nothing, and read to the next person as
//      "already translated" -- the worst of the three.
//    * An `#if 0` BODY is one keystroke away from moving a real axis, and the
//      body would be untranslated BCB6 text sitting in the port tree.
//    * DECLARED-BUT-UNDEFINED makes the LINKER the interlock: the moment any
//      future wave (or the web write path) tries to CALL one, the build fails
//      with `undefined reference to TfMotorTest::<method>` naming the exact
//      motion entry point.  Nothing can reach a motor by accident.
//  Neither class has any virtual member, so an undefined member function
//  costs nothing at link time until somebody calls it -- verified: no
//  `virtual` in TfMotorTest or TMotorTestClass (forms/fMotorTest.h).
//
//  UN-GATING PROTOCOL: translate the body in a wave that owns the interlock
//  story (who may issue it, what must be homed/servo-on/door-closed first,
//  and what re-checks it on the C++ side).  Do not un-gate as a side effect
//  of "finishing the facade".
//  ==========================================================================
// =============================================================================
#include "forms/fMotorTest.h"

#include "MachineDefine.h"      // machine-configuration macro layer (golden :1)
#include "cmydef.h"             // M* motor indices (:2101-), TOTAL_MOTOR (:46), N_INTEGER.. (:289-297)
#include "cprod.h"              // machine-configuration globals (golden :7)
#include "Motor/mymotor.h"      // MOT[] / TTrayMotor / HTMotor (golden :8)
#include "cpublic.h"            // shared machine globals (golden :11)
#include "cinitial.h"           // MotTablePath (golden :12)
#include "mysensor.h"           // Sen[] / TMySensor (golden :15 myswitch.h/:16 mysensor.h)
#include "myswitch.h"           // SW[]  / TMySwitch
#include "common.h"             // asSystemPath / CheckAndReadIniData / WriteIniData (golden :19)
#include "MachineType.h"        // machine-type enums used by the config globals
#include "LastSet.h"            // LastSet.iLanguageCountry (FormShow -- gated, kept for the include set golden has)
#include "canary_support.h"     // ShowMyMessage (golden's mymessbox.h:58 equivalent)
#include "forms/fQwertyKey.h"   // fQwertyKey->ShowQwertyKey (golden :22 myQwertyKeyBoard.h)

#include <cstdlib>              // atoi

// golden's colour / enum spellings kept VERBATIM in the bodies below.  These
// using-declarations are TU-local; vclcompat deliberately does not publish the
// colour constants globally (mykitsuck.h:109 sets the same `using` precedent).
using vclcompat::TColor;
using vclcompat::clBlack;
using vclcompat::clGreen;
using vclcompat::clRed;
using vclcompat::clWhite;
using vclcompat::clYellow;
using vclcompat::LEDHorizontal;
using vclcompat::bvNone;

// golden uMotorTest.h:18 carries a blanket `using namespace std;`.  This port
// narrows it to the ONE name golden's delivered bodies spell unqualified
// (FormDestroy's `vector<TMotorTestClass *>::iterator`, golden :1736) -- the
// same narrowing forms/fQwertyKey.h applied to myQwertyKeyBoard.h's copy.
// Explicit, so this translation does not silently depend on some other header
// leaking `using namespace std` into this TU.
using std::vector;

namespace {
// DEVIATION (D-3): golden `clWindow` (ShowMotorSelect, golden :693-694) has no
// vclcompat definition -- the shared TColor block (vclcompat/LedCore.h:52-63)
// stops at clBtnFace.  This is the real VCL Graphics.hpp value, in the same
// system-colour form as the existing `clBtnFace = TColor(0x8000000F)`.
// UNNAMED namespace = internal linkage, so it cannot collide with a future
// `vclcompat::clWindow`; golden's spelling stays verbatim at the use site.
const TColor clWindow = TColor(0x80000005);
}   // namespace

// golden :27-30 -- file-scope globals, ported verbatim.
//   NOTE, measured this wave: `iRotaterOldPos` / `iRotaterNewPos` /
//   `TargetPos` / `bRotaterPosReady` are referenced in EXACTLY ONE golden file
//   (grep over the whole golden tree: only uMotorTest.cpp) and are never read
//   or written even there -- they are leftovers of the deleted
//   `ProcessRotater()` (golden uMotorTest.h:373, "Steven 20100831 : 多了塊魚").
//   `TargetPos` is an unusually generic name for an external-linkage global;
//   verified unclaimed tree-wide 20260826 14:18 (`Grep "\bTargetPos\b"
//   --glob *.{h,cpp}` -> 0 hits).  A future collision would be a DUPLICATE
//   SYMBOL link error, i.e. loud, not silent.
TfMotorTest *fMotorTest;                                                        // golden :27
int iRotaterOldPos;                                                             // golden :28
int iRotaterNewPos,TargetPos;                                                   // golden :29
bool bRotaterPosReady=false;                                                    // golden :30
//------------------------------------------------------------------------------

// golden :32-128.  PORT: `: TComponent(Owner)` dropped (GOLDEN NOTE
// G-OwnerInInitList); every pointer NULL-initialised to reproduce BCB6
// TObject zero-fill across golden's `bView==false` early return (GOLDEN NOTE
// G-ZeroFill); widget owner arguments dropped (DEVIATION D-2).
TMotorTestClass::TMotorTestClass(int MotNo, bool bView)                         //Steven 20120831 : MotorTest元件改用Vector新增
    : index(0), Visible(false),
      labName(NULL), edPos1(NULL), edPos2(NULL), cbUsing(NULL),
      MotorPanel(NULL), MotorLabel(NULL), MotorHomeLed(NULL),
      AllMotMoveFlag(false)
{
    for(int i=0; i<10; i++)                                                     // PORT (G-ZeroFill): golden relies on TObject zero-fill here
        MotorLed[i]=NULL;

    AnsiString MotName;
    index           =MotNo;
    Visible         =bView;
    AllMotMoveFlag  =true;

    MotName.sprintf("M%02d", MotNo);

    if(Visible==false)                                                          //Steven 20240822 : 沒用到的馬達不new
        return;

    labName =new TfMotorTestLabel();
    edPos1  =new TfMotorTestEdit();
    edPos2  =new TfMotorTestEdit();
    cbUsing =new TfMotorTestCheckBox();

//GATE(M-Parent) labName ->Parent        =fMotorTest->pnlMotor;
    labName ->Name          ="labName"+AnsiString(MotNo);
    labName ->Color         =TColor(0x00C2B8A6);
    labName ->Height        =18;
    labName ->Width         =31;
    labName ->Font->Color   =clBlack;
    labName ->Font->Size    =10;
    labName ->Font->Name    ="Arial";
    labName ->Caption       =MotName;
//GATE(M-OnClick) labName ->OnClick       =fMotorTest->lM00Click;
    labName ->Visible       =Visible;
    labName ->Tag           =index;

//GATE(M-Parent) edPos1  ->Parent    =fMotorTest->pnlMotor;
    edPos1  ->Name      ="edPos1_"+AnsiString(MotNo);
    edPos1  ->Height    =24;
    edPos1  ->Width     =65;
    edPos1  ->Text      =0;
//GATE(M-OnClick) edPos1  ->OnClick   =fMotorTest->lpA00Click;
    edPos1  ->Visible   =Visible;
    edPos1  ->Tag       =index;

//GATE(M-Parent) edPos2  ->Parent    =fMotorTest->pnlMotor;
    edPos2  ->Name      ="edPos2_"+AnsiString(MotNo);
    edPos2  ->Height    =24;
    edPos2  ->Width     =65;
    edPos2  ->Text      =0;
//GATE(M-OnClick) edPos2  ->OnClick   =fMotorTest->lpA00Click;
    edPos2  ->Visible   =Visible;
    edPos2  ->Tag       =index;

//GATE(M-Parent) cbUsing ->Parent    =fMotorTest->pnlMotor;
    cbUsing ->Name      ="cbUsing"+AnsiString(MotNo);
    cbUsing ->Height    =17;
    cbUsing ->Width     =15;
    cbUsing ->Caption   ="";
    cbUsing ->Visible   =Visible;
    cbUsing ->Tag       =index;
//GATE(M-OnClick) cbUsing ->OnClick   =fMotorTest->ck00Click;

    MotorPanel =new TfMotorTestPanel();
//GATE(M-Parent) MotorPanel->Parent        =fMotorTest->pnlMotor;
    MotorPanel->Name          ="MotorPanel"+AnsiString(MotNo);
    MotorPanel->Color         =TColor(0x00C2B8A6);
    MotorPanel->Height        =19;
    MotorPanel->Width         =469;
    MotorPanel->BevelOuter    =bvNone;
    MotorPanel->Caption       ="";
    MotorPanel->Visible       =Visible;
    MotorPanel->Tag           =index;

    int iYTop=3;
    MotorLabel=new TfMotorTestLabel();
//GATE(M-Parent) MotorLabel->Parent      =MotorPanel;
    MotorLabel->Top         =iYTop;
    MotorLabel->Left        =4;
    MotorLabel->Font->Size  =9;
    MotorLabel->Font->Name  ="MS Scan Serief";
    MotorLabel->Caption     =MotName;

    MotorHomeLed            =new TfMotorTestLed();
//GATE(M-Parent) MotorHomeLed->Parent    =MotorPanel;
    MotorHomeLed->Top       =iYTop;
    MotorHomeLed->Left      =46;
    MotorHomeLed->LEDStyle  =LEDHorizontal;
    MotorHomeLed->Blink     =false;
    MotorHomeLed->Value     =false;
    MotorHomeLed->TrueColor =clGreen;
    for(int i=0; i<10; i++)
    {
        MotorLed[i]=new TfMotorTestLed();
//GATE(M-Parent) MotorLed[i]->Parent     =MotorPanel;
        MotorLed[i]->Top        =iYTop;
        MotorLed[i]->Left       =46+39+i*39;
        MotorLed[i]->LEDStyle   =LEDHorizontal;
        MotorLed[i]->Blink      =false;
        MotorLed[i]->Value      =false;
        MotorLed[i]->TrueColor  =clRed;
    }
}
//---------------------------------------------------------------------------
// PORT-ONLY (DEVIATION D-2): golden's TMotorTestClass has no destructor
// because the VCL owner (fMotorTest->pnlMotor / MotorPanel) frees these
// widgets when the form is destroyed.  The owner argument is dropped here, so
// this destructor is the faithful equivalent -- and it is safe because in this
// tree these widgets have exactly one owner.  `delete NULL` is well-defined,
// so the `bView==false` rows (every pointer still NULL, G-ZeroFill) are fine.
TMotorTestClass::~TMotorTestClass()
{
    for(int i=0; i<10; i++)
        delete MotorLed[i];
    delete MotorHomeLed;
    delete MotorLabel;
    delete MotorPanel;
    delete cbUsing;
    delete edPos2;
    delete edPos1;
    delete labName;
}
//---------------------------------------------------------------------------
// golden :130-143
void TMotorTestClass::SetPosition(int Top, int Left)
{
    if(Visible==false)                                                          //Steven 20240822 : 沒用到的馬達不new
        return;

    cbUsing ->Top    =Top;
    cbUsing ->Left   =Left;
    labName ->Top    =Top;
    labName ->Left   =Left+16;
    edPos1  ->Top    =Top-3;
    edPos1  ->Left   =Left+50;
    edPos2  ->Top    =Top-3;
    edPos2  ->Left   =Left+117;
}
//---------------------------------------------------------------------------
// AI(W906-FW-MOTORTEST-W24) 20260826: PORT-ONLY ctor -- FIELDS ONLY.
// Golden's ctor body (uMotorTest.cpp:148-389) is Init(); see forms/
// fMotorTest.h "CTOR / Init() SPLIT" for why that split is load-bearing here.
// Values below are the BCB6-TObject zero-fill equivalents, EXCEPT ActiveIndex:
// golden's own first ctor statement is `ActiveIndex=-1;` (:149) and a 0
// default would mean "MInArmX is the selected motor" -- exactly the class of
// silent-wrong-zero this tree has been burned by (docs/KNOWLEDGE.md,
// "解gate前先查值從哪來").  Init() still carries golden's `ActiveIndex=-1;`
// verbatim; the duplication is deliberate and idempotent.
TfMotorTest::TfMotorTest()
    : SelMotSpeed(0), iSelectRow(0), iSelectCol(0), iDragCol(0), iDragRow(0),
      ActiveIndex(-1), LoopTask(0), fShow(false), bSingleHome(false),
      iSingleHomeIndex(0), dwLoopCount(0), iUseAxis(0), Average(0)
{
    // DEVIATION (D-7): .dfm-sourced initial values that translated code READS.
    strngrdMotor->DefaultColWidth     = 80;     // uMotorTest.dfm:1874 (ColCount=2/RowCount=11 are the ctor args in the header)
    strngrdMotorData->DefaultColWidth = 75;     // uMotorTest.dfm:3664
    strngrdMotorData->FixedCols       = 0;      // uMotorTest.dfm:3665
}
//---------------------------------------------------------------------------
// PORT-ONLY dtor: frees the widget stand-ins this facade allocated (same
// owner-frees rationale as TMotorTestClass::~TMotorTestClass, DEVIATION D-2).
// MotorTestClass entries are NOT freed here -- golden frees those in
// FormDestroy (:1736-1740), which stays faithful and is translated below.
TfMotorTest::~TfMotorTest()
{
    delete PageControl1;      delete TabSheet6;         delete Panel15;
    delete TabSheet7;         delete Panel24;           delete Bevel5;
    delete SpeedButton44;     delete SpeedButton45;     delete SpeedButton46;
    delete SpeedButton47;     delete Label82;           delete Label83;
    delete Label84;           delete Label85;           delete Label87;
    delete Label88;           delete Label79;           delete Label80;
    delete Label81;           delete Label97;           delete SpeedButton4;
    delete SpeedButton5;      delete Label102;          delete SpeedButton8;
    delete Label105;          delete Label106;          delete Label107;
    delete ScrollBar2;        delete ComboBox4;         delete Edit14;
    delete Edit16;            delete Edit18;            delete edtSpeedIndex1;
    delete Edit3;             delete Edit4;             delete Edit5;
    delete Edit6;             delete Edit12;            delete Edit13;
    delete Panel25;           delete Panel26;           delete Bevel6;
    delete SpeedButton50;     delete SpeedButton51;     delete SpeedButton52;
    delete SpeedButton53;     delete Label86;           delete Label89;
    delete Label90;           delete Label91;           delete Label92;
    delete Label93;           delete Label98;           delete Label99;
    delete Label100;          delete Label101;          delete SpeedButton6;
    delete SpeedButton7;      delete Label103;          delete SpeedButton9;
    delete Label108;          delete Label109;          delete Label110;
    delete ScrollBar3;        delete ComboBox5;         delete Edit24;
    delete Edit102;           delete Edit103;           delete edtSpeedIndex2;
    delete Edit7;             delete Edit8;             delete Edit10;
    delete Edit11;            delete Edit15;            delete Edit17;
    delete Panel27;           delete Timer1;            delete Edit2;
    delete palExit;           delete Panel2;            delete Panel23;
    delete btnHighSpeed;      delete btnHomeHigh;       delete btnHomeLow;
    delete btnLowSpeed;       delete btnSoftPPos;       delete btnSoftNPos;
    delete btnRange;          delete btnRate;           delete btnReloadMotorData;
    delete btnSetRate;        delete btnSetRange;       delete edtMotorRate;
    delete edtMotorRange;     delete btResetMNet;       delete Panel20;
    delete Label38;           delete sbMotorTest_JogN;  delete sbMotorTest_JogP;
    delete Label40;           delete Label41;           delete sbMotorTest_MoveN;
    delete sbMotorTest_MoveP; delete btnLoopMove;       delete btnSetPosP;
    delete btnSetPosN;        delete Label39;           delete btnHome;
    delete btnGoSoftP;        delete btnGoSoftN;        delete Label112;
    delete Label114;          delete Label115;          delete Label116;
    delete Label117;          delete Label118;          delete Label119;
    delete Label120;          delete Label121;          delete Label122;
    delete ALed1;             delete ALed2;             delete ALed3;
    delete ALed4;             delete ALed5;             delete ALed6;
    delete ALed7;             delete ALed8;             delete ALed9;
    delete Label123;          delete Label1;            delete lblLoopCount;
    delete lblAvgTime;        delete Label4;            delete Label5;
    delete btnGo;             delete btnServoOff;       delete btnMotorPower;
    delete lblJogPTime;       delete lblJogNTime;       delete ALed10;
    delete Label10;           delete Label12;           delete pnlMotorAlias;
    delete cbbInterval;       delete cbbLoopWaitTime;   delete scrlbrMotorSpeed;
    delete edtCommandPos;     delete edtSpeed;          delete edtHomeOffset;
    delete select;            delete pnlEncoderPos;     delete pnlMotor;
    delete BitBtn1;           delete cbbMotorName;      delete strngrdMotor;
    delete cbUsing;           delete labName;           delete Edit9;
    delete Edit26;            delete Panel4;            delete TabSheet1;
    delete ts1;               delete Timer2;            delete edDelayTime;
    delete edPitech;          delete rgMoveType;        delete BitBtn2;
    delete Memo1;             delete BitBtn3;           delete rgAxis;
    delete btnSaveLogLightScaleData;                    delete InArmX_Go;
    delete lbl1;              delete lbl2;              delete lbl3;
    delete lbl4;              delete lbl5;              delete lbl6;
    delete lbl7;              delete mmo1;              delete mmo2;
    delete mmo3;              delete mmo4;              delete mmo5;
    delete mmo6;              delete mmo7;              delete mmo8;
    delete lblRealSpeed;      delete tsMotorDatabase;   delete Panel1;
    delete sbtReload;         delete sbUpdate;          delete strngrdMotorData;
    delete btnAddMotor;       delete btnDeleteMotor;    delete btnModify;
    delete lblMotorName;      delete pnlStop;           delete btnStop;
}
//---------------------------------------------------------------------------
// golden :145-390 -- golden's ENTIRE ctor body, VERBATIM, moved into an
// explicit Init() (task rule 2 / forms/fQwertyKey.h convention).  It reads ~40
// cross-TU machine-configuration globals, which is exactly why it must not run
// from a static initialiser.  GOLDEN NOTE (G-VecIndex): the 164 push_backs
// below land in motor-id order, so `MotorTestClass[<motorId>]` is valid --
// machine-checked this wave, zero divergence over all 164.  Do not reorder.
void TfMotorTest::Init()
{
    AnsiString str="";
    ActiveIndex=-1;

    MotorTestClass.push_back(new TMotorTestClass(MInArmX));
    MotorTestClass.push_back(new TMotorTestClass(MInArmY));
    MotorTestClass.push_back(new TMotorTestClass(MInArmPitch));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZA));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZB, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZC, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZD, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZE, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZF, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZG, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZH, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MInShuttle1));
    MotorTestClass.push_back(new TMotorTestClass(MInShuttle2));
    MotorTestClass.push_back(new TMotorTestClass(MTestY1));
    MotorTestClass.push_back(new TMotorTestClass(MTestZ1));
    MotorTestClass.push_back(new TMotorTestClass(MTestZ2));
    MotorTestClass.push_back(new TMotorTestClass(MTestY2));
//  #ifdef Carry4
//    MotorTestClass.push_back(new TMotorTestClass(MOutShuttle1));
//    MotorTestClass.push_back(new TMotorTestClass(MOutShuttle2));
//  #else
    MotorTestClass.push_back(new TMotorTestClass(MOutShuttle1,   false));
    MotorTestClass.push_back(new TMotorTestClass(MOutShuttle2,   false));
//  #endif
    MotorTestClass.push_back(new TMotorTestClass(MOutArmX));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmY));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmPitch));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZA));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZB, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZC, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZD, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZE, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZF, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZG, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZH, (InOutArmPickerUseMotor==eptUseMot)));
    MotorTestClass.push_back(new TMotorTestClass(MTrayX));
    MotorTestClass.push_back(new TMotorTestClass(MInArmPitchY,   (USE_IN_Y_IS_AUTO_PITCH==true)));                      //Steven 20131002 : XY變距  //JerryYang 20251218 : IN/OUT ARM支援不同模組
    MotorTestClass.push_back(new TMotorTestClass(MInArmPitchX2,  (USE_IN_Y_IS_AUTO_PITCH==true)));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmPitchY,  (USE_OUT_Y_IS_AUTO_PITCH==true)));                     //Steven 20131002 : XY變距
    MotorTestClass.push_back(new TMotorTestClass(MOutArmPitchX2, (USE_OUT_Y_IS_AUTO_PITCH==true)));
    MotorTestClass.push_back(new TMotorTestClass(MLoaderZ,       LOAD_Z_USE_MOTOR[0]));
    MotorTestClass.push_back(new TMotorTestClass(MEmptyZ,        LOAD_Z_USE_MOTOR[1]));
    MotorTestClass.push_back(new TMotorTestClass(MColorZ,        LOAD_Z_USE_MOTOR[2]));
    MotorTestClass.push_back(new TMotorTestClass(MAuto1Z,       (LOAD_Z_USE_MOTOR[3] || USE_LdUldCassetteMode==1)));    //Ifor 20251216 add:Boat Carrier
    MotorTestClass.push_back(new TMotorTestClass(MAuto2Z,       (LOAD_Z_USE_MOTOR[4] || USE_LdUldCassetteMode==1)));    //Ifor 20251216 add:Boat Carrier
    MotorTestClass.push_back(new TMotorTestClass(MAuto3Z,        LOAD_Z_USE_MOTOR[5]));
    MotorTestClass.push_back(new TMotorTestClass(MInRotateKit,   (USE_ROTATE_KIT && iRotate_Type!=eCynRotate)));        //2013-04-12    Dell :旋轉站;馬達版
    MotorTestClass.push_back(new TMotorTestClass(MOutRotateKit,  (USE_ROTATE_KIT && iRotate_Type!=eCynRotate && iRotate_Type!=e4MotRotate && iRotate_Type!=e2MotRotate2Dut)));  //2013-04-12    Dell :旋轉站;馬達版
    MotorTestClass.push_back(new TMotorTestClass(MAOIKit,        USE_AOI_Inspection));                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    MotorTestClass.push_back(new TMotorTestClass(MLoaderY,       ((INSTALL_OCR_YMot==eocrYMotInstal ||                  //RogerYang 20250903 Fix For 殘料檢    //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
                                                                  (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR) ||//KenHsieh 20260514 : 補上OCR+皮帶
                                                                   USE_LdUldCassetteMode==1))));                        //Ifor 20251216 add:Boat Carrier

    MotorTestClass.push_back(new TMotorTestClass(MEmptyY,        false));
    MotorTestClass.push_back(new TMotorTestClass(MColorY,        false));
    MotorTestClass.push_back(new TMotorTestClass(MAuto1Y,        USE_LdUldCassetteMode==1));                            //Ifor 20251216 add:Boat Carrier
    MotorTestClass.push_back(new TMotorTestClass(MAuto2Y,        USE_LdUldCassetteMode==1));                            //Ifor 20251216 add:Boat Carrier
    MotorTestClass.push_back(new TMotorTestClass(MAuto3Y,        false));

    MotorTestClass.push_back(new TMotorTestClass(MInArmZAe,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));  //Steven 20230323 : For HT1032  //Steven 20260316 : Fix operator precedence
    MotorTestClass.push_back(new TMotorTestClass(MInArmPitchX3,  (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmPitchX4,  (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZAf,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));  //Steven 20260316 : Fix operator precedence
    MotorTestClass.push_back(new TMotorTestClass(MOutArmPitchX3, (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmPitchX4, (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)));
    MotorTestClass.push_back(new TMotorTestClass(MTrayZ,         (TRAY_ARM_MODE==eUnderCoveyor || USE_LdUldCassetteMode==1)));                                  //Ifor 20251216 add:Boat Carrier
    MotorTestClass.push_back(new TMotorTestClass(MOutSortAa,     (USE_OUT_SORT_ARM==iOutSortX40mm)));                   //Steven 20240822 : For HT-9046AU
    MotorTestClass.push_back(new TMotorTestClass(MOutSortAb,     (USE_OUT_SORT_ARM==iOutSortX40mm)));
    MotorTestClass.push_back(new TMotorTestClass(MInArmXScale,   USE_MAGNETIC_SCALE));
    MotorTestClass.push_back(new TMotorTestClass(MInArmYScale,   USE_MAGNETIC_SCALE));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmXScale,  USE_MAGNETIC_SCALE));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmYScale,  USE_MAGNETIC_SCALE));
    MotorTestClass.push_back(new TMotorTestClass(MShuttle1Pitch, AUTO_SENSOR_INSTALL));                                 //wei 20160914 Auto Shuttle Sensor
    MotorTestClass.push_back(new TMotorTestClass(MShuttle2Pitch, AUTO_SENSOR_INSTALL));                                 //wei 20160914 Auto Shuttle Sensor
    //Steven 20170329 (Wei) : Add individual rotate motor
    //==>
    MotorTestClass.push_back(new TMotorTestClass(MInRotateB,  (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    MotorTestClass.push_back(new TMotorTestClass(MInRotateC,  (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    MotorTestClass.push_back(new TMotorTestClass(MInRotateD,  (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    MotorTestClass.push_back(new TMotorTestClass(MInRotateE,  (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));
    MotorTestClass.push_back(new TMotorTestClass(MInRotateF,  (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    MotorTestClass.push_back(new TMotorTestClass(MInRotateG,  (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    MotorTestClass.push_back(new TMotorTestClass(MInRotateH,  (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    MotorTestClass.push_back(new TMotorTestClass(MOutRotateB, (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    MotorTestClass.push_back(new TMotorTestClass(MOutRotateC, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));
    MotorTestClass.push_back(new TMotorTestClass(MOutRotateD, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    MotorTestClass.push_back(new TMotorTestClass(MOutRotateE, (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    MotorTestClass.push_back(new TMotorTestClass(MOutRotateF, (USE_ROTATE_KIT &&  iRotate_Type==e8MotRotate)));
    MotorTestClass.push_back(new TMotorTestClass(MOutRotateG, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut))));
    MotorTestClass.push_back(new TMotorTestClass(MOutRotateH, (USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate))));
    //<==
    //Steven 20170329 (Wei) : Add individual rotate motor

    MotorTestClass.push_back(new TMotorTestClass(MLightScale,    USE_TRAY_ROBOT));
//    MotorTestClass.push_back(new TMotorTestClass(MTrayRobotX,    USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MTrayRobotY,    USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MTrayRobotZ,    USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
    MotorTestClass.push_back(new TMotorTestClass(MInArmZAg,       (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZAh,       (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));

    MotorTestClass.push_back(new TMotorTestClass(MArmAlignment,       false));
    MotorTestClass.push_back(new TMotorTestClass(MLoadHingeR,    USE_LOADER_HINGE));                                    //Steven 20170330 (Wei) : For TSMC
    MotorTestClass.push_back(new TMotorTestClass(MLoadHingeZ,    USE_LOADER_HINGE));                                    //Steven 20170330 (Wei) : For TSMC
    MotorTestClass.push_back(new TMotorTestClass(MPreciser,      USE_PRECISER));                                        //Frank 20180410 (Steven) : InArm Preciser Station

//    MotorTestClass.push_back(new TMotorTestClass(MSortX,         USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortY,         USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortPitch,     USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortPitch2,    USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortPitchY,    USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortZA,        USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortZB,        USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortZC,        USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortZD,        USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortZE,        USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortZF,        USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortZG,        USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM
//    MotorTestClass.push_back(new TMotorTestClass(MSortZH,        USE_TRAY_ROBOT));   //Steven 20170330 (Wei) : For HT-9046LM

//   MotorTestClass.push_back(new TMotorTestClass("M99",  MCaselevatorZ,     USE_MR_SYSTEM));   //wei 20180702 MR
//    MotorTestClass.push_back(new TMotorTestClass(MLoadRobotZ,       USE_MR_SYSTEM));   //wei 20180702 MR
    MotorTestClass.push_back(new TMotorTestClass(MInArmZBe,       (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZBf,       (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZBg,       (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MInArmZBh,       (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZAe,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZAf,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZAg,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZAh,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZBe,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZBf,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZBg,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    MotorTestClass.push_back(new TMotorTestClass(MOutArmZBh,      (InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));

    MotorTestClass.push_back(new TMotorTestClass(MOutSortX,       (USE_OUT_SORT_ARM==iOutSortX40mm)));                  //Steven 20240822 : For HT-9046AU
    MotorTestClass.push_back(new TMotorTestClass(MCaselevatorZ,   USE_MR_SYSTEM));                                      //Sam 20190112 LM

    MotorTestClass.push_back(new TMotorTestClass(MCasArmX,        USE_MR_SYSTEM));                                      //wei 20180702 MR
    MotorTestClass.push_back(new TMotorTestClass(MCasArmZ,        USE_MR_SYSTEM));                                      //wei 20180702 MR
    MotorTestClass.push_back(new TMotorTestClass(MTrayBracketZ,   USE_MR_SYSTEM));                                      //wei 20180702 MR
    MotorTestClass.push_back(new TMotorTestClass(MStackedTrayX,   USE_MR_SYSTEM));                                      //wei 20180702 MR
    MotorTestClass.push_back(new TMotorTestClass(MStackedTrayZ,   USE_MR_SYSTEM));                                      //wei 20180702 MR
    MotorTestClass.push_back(new TMotorTestClass(MUnloadRobotZ,   USE_MR_SYSTEM));                                      //wei 20180702 MR

    MotorTestClass.push_back(new TMotorTestClass(MOutSortY,       (USE_OUT_SORT_ARM==iOutSortX40mm)));                  //Steven 20240822 : For HT-9046AU

    MotorTestClass.push_back(new TMotorTestClass(MCCDX, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(MCCDY, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(MCCDZ, USE_FINE_PITCH));

    MotorTestClass.push_back(new TMotorTestClass(MInFlipper1,  USE_DEVICE_FLIPPER));                                    //Frank 20210612 : Flipper Function
    MotorTestClass.push_back(new TMotorTestClass(MInFlipper2,  USE_DEVICE_FLIPPER));
    MotorTestClass.push_back(new TMotorTestClass(MInFlipper3,  USE_DEVICE_FLIPPER));
    MotorTestClass.push_back(new TMotorTestClass(MOutFlipper1, USE_DEVICE_FLIPPER));
    MotorTestClass.push_back(new TMotorTestClass(MOutFlipper2, USE_DEVICE_FLIPPER));
    MotorTestClass.push_back(new TMotorTestClass(MOutFlipper3, USE_DEVICE_FLIPPER));

    MotorTestClass.push_back(new TMotorTestClass(MLdCarRotArm, USE_LD_Rot_Arm));                                        //RogerYang 20250828 add for Loader Rotate Arm    //Steven 20210624 : Fine Pitch
    MotorTestClass.push_back(new TMotorTestClass(MLoaderY_CCW, USE_LdUldCassetteMode==1));                              //Ifor 20251216 add:Boat Carrier
    MotorTestClass.push_back(new TMotorTestClass(MAuto1Y_CCW , USE_LdUldCassetteMode==1));                              //Ifor 20251216 add:Boat Carrier
    MotorTestClass.push_back(new TMotorTestClass(MAuto2Y_CCW , USE_LdUldCassetteMode==1));                              //Ifor 20251216 add:Boat Carrier
    MotorTestClass.push_back(new TMotorTestClass(MAuto3Y_CCW , false));
    MotorTestClass.push_back(new TMotorTestClass(MAuto4Y_CCW , false));
    MotorTestClass.push_back(new TMotorTestClass(MAuto5Y_CCW , false));
    MotorTestClass.push_back(new TMotorTestClass(MAuto6Y_CCW , false));
    MotorTestClass.push_back(new TMotorTestClass(M1_3R, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_4X, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_4Y, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_4R, USE_FINE_PITCH));

    MotorTestClass.push_back(new TMotorTestClass(M1_5X, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_5Y, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_5R, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_6X, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_6Y, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_6R, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_7X, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_7Y, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_7R, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_8X, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_8Y, USE_FINE_PITCH));
    MotorTestClass.push_back(new TMotorTestClass(M1_8R, USE_FINE_PITCH));

    MotorTestClass.push_back(new TMotorTestClass(MMagazine, (AUTO3_IS_MAGAZINE==1)));                                   //JerryYang 20220909 : add magazine
    MotorTestClass.push_back(new TMotorTestClass(MCatchMgzTray, (AUTO3_IS_MAGAZINE==1)));                               //JerryYang 20220909 : add magazine
    MotorTestClass.push_back(new TMotorTestClass(MMagYTrayOut, false));
    MotorTestClass.push_back(new TMotorTestClass(MFix3Full, (FIX3_FULL_PLACE==Fix3K_UseStepperMotor)));                 //Steven 20221122 : Add Fix3 Motor

    MotorTestClass.push_back(new TMotorTestClass(MAuto4Z, LOAD_Z_USE_MOTOR[6]));                                        //Steven 20230907 : For HT-9011UC
    MotorTestClass.push_back(new TMotorTestClass(MAuto5Z, LOAD_Z_USE_MOTOR[7]));
    MotorTestClass.push_back(new TMotorTestClass(MAuto6Z, LOAD_Z_USE_MOTOR[8]));
    MotorTestClass.push_back(new TMotorTestClass(MAuto4Y, false));
    MotorTestClass.push_back(new TMotorTestClass(MAuto5Y, false));
    MotorTestClass.push_back(new TMotorTestClass(MAuto6Y, false));

    MotorTestClass.push_back(new TMotorTestClass(MTopAOIArmX  , false));
    MotorTestClass.push_back(new TMotorTestClass(MTopAOIArmY  , false));
    MotorTestClass.push_back(new TMotorTestClass(MTopAOIArmR  , false));
    MotorTestClass.push_back(new TMotorTestClass(MTopAOICCDZ  , (USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)));
    MotorTestClass.push_back(new TMotorTestClass(MTopAOIElevZ1, false));
    MotorTestClass.push_back(new TMotorTestClass(MTopAOIElevZ2, false));

    MotorTestClass.push_back(new TMotorTestClass(MOutSortPitchX, (USE_OUT_SORT_ARM==iOutSortX40mm)));                   //Steven 20240822 : For HT-9046AU
    MotorTestClass.push_back(new TMotorTestClass(MOutSortSht, (USE_OUT_SORT_ARM==iOutSortX40mm)));
    MotorTestClass.push_back(new TMotorTestClass(MLoad2Z, USE_2nd_LOADER));     //RogerYang 20250828 add
    MotorTestClass.push_back(new TMotorTestClass(MLoad2Y, false));              //RogerYang 20250828 add for Loader Rotate Arm

    MotorTestClass.push_back(new TMotorTestClass(MInSh1LtcSenZ1, In_Shuttle_Auto_Latch));                               //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    MotorTestClass.push_back(new TMotorTestClass(MInSh1LtcSenZ2, In_Shuttle_Auto_Latch));                               //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    MotorTestClass.push_back(new TMotorTestClass(MInSh2LtcSenZ1, In_Shuttle_Auto_Latch));                               //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    MotorTestClass.push_back(new TMotorTestClass(MInSh2LtcSenZ2, In_Shuttle_Auto_Latch));                               //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料

    int iLPitch =192,   iTPitch =28;                                            //調整畫面位置
    int iLeft   =4,     iTop    =4;
    int iMaxRowItem=12;
    int ColItem=0, RowItem=0;

    for(int i=0; i<TOTAL_MOTOR; i++)                                            //Steven 20240322 : 按照馬達編號顯示
    {
        for(unsigned int j=0; j<MotorTestClass.size(); j++)
        {
            if(MotorTestClass[j]->index==i &&                                   //Steven 20240322 : 按照馬達編號顯示
               MotorTestClass[j]->Visible)
            {
                MotorTestClass[j]->SetPosition(iTop+iTPitch*RowItem, iLeft+iLPitch*ColItem);
                RowItem++;

                if(RowItem>=iMaxRowItem)
                {
                    RowItem=0;
                    ColItem++;
                }
            }
        }
    }

    strngrdMotor->ColWidths[0] = strngrdMotor->DefaultColWidth+20;
    strngrdMotor->ColWidths[1] = strngrdMotor->DefaultColWidth-20;
    SelMotSpeed=0;
}
//---------------------------------------------------------------------------
// golden :655-669
void TfMotorTest::UpdateMotorParameter()
{
    if(ActiveIndex==-1)                                                         //Steven 20230731 : Add protection
        return;
    strngrdMotor->Cells[1][ 1]=MOT[ActiveIndex].Motor->ReadInitSpeed();
    strngrdMotor->Cells[1][ 2]=MOT[ActiveIndex].Motor->PJogHighSpeed;
    strngrdMotor->Cells[1][ 3]=MOT[ActiveIndex].Motor->PJogLowSpeed;
    strngrdMotor->Cells[1][ 4]=MOT[ActiveIndex].Motor->PHomeHighSpeed;
    strngrdMotor->Cells[1][ 5]=MOT[ActiveIndex].Motor->PHomeLowSpeed;
    strngrdMotor->Cells[1][ 6]=MOT[ActiveIndex].Motor->PSoftLimitP;
    strngrdMotor->Cells[1][ 7]=MOT[ActiveIndex].Motor->PSoftLimitN;
    strngrdMotor->Cells[1][ 8]=MOT[ActiveIndex].Motor->ReadAcc();
    strngrdMotor->Cells[1][ 9]=MOT[ActiveIndex].Motor->ReadDec();
    strngrdMotor->Cells[1][10]=MOT[ActiveIndex].Motor->ReadRange();
}
//---------------------------------------------------------------------------
// golden :671-698  (uses clWindow -- DEVIATION D-3)
void TfMotorTest::ShowMotorSelect(int Index, int Attr)
{
    if(Index==-1)                                                               //Steven 20230731 : Add protection
        return;

    if(MotorTestClass[Index]->Visible)                                          //Steven 20240822 : 沒用到的馬達不new
    {
        pnlMotorAlias->Caption=MotorTestClass[Index]->labName->Caption+MOT[Index].Alias;

        if(Attr)
        {
            MotorTestClass[Index]->labName->Font->Color =clRed;
            MotorTestClass[Index]->edPos1->Font->Color  =clWhite;
            MotorTestClass[Index]->edPos2->Font->Color  =clWhite;
            MotorTestClass[Index]->edPos1->Color        =clRed;
            MotorTestClass[Index]->edPos2->Color        =clRed;
        }
        else
        {
            MotorTestClass[Index]->labName->Font->Color =clBlack;
            MotorTestClass[Index]->edPos1->Font->Color  =clBlack;
            MotorTestClass[Index]->edPos2->Font->Color  =clBlack;
            MotorTestClass[Index]->edPos1->Color        =clWindow;
            MotorTestClass[Index]->edPos2->Color        =clWindow;
        }
    }
    UpdateMotorParameter();
}
//---------------------------------------------------------------------------
// golden :700-732  (GOLDEN NOTE G-DeadAvg: T[][] is never written; G-Gali0: Gali_ReadPos is a {return 0;} stub in this tree)
void TfMotorTest::UpdatePreasureScreen()
{
    AnsiString str;
    static int ct=0;
    ct++;
    if(ct>3)
    {
        ct=0;
        Edit14->Text        =MOT[MTestZ1].Gali_ReadPos();
        Edit24->Text        =MOT[MTestZ2].Gali_ReadPos();
        edtSpeedIndex1->Text=MOT[MTestZ1].Gali_ReadPos();
        edtSpeedIndex2->Text=MOT[MTestZ2].Gali_ReadPos();
    }
    static double T[2][256];
    static int p=0;
    double sum0, sum1;
    p++;
    if(p>=50)
        p=0;
    sum0=0;
    sum1=0;
    for(int i=0; i<50; i++)
    {
        sum0+=T[0][i];
        sum1+=T[1][i];
    }
    sum0/=50.0;
    sum1/=50.0;
    str.sprintf("%6.3f", sum0);
    Edit16->Text=str;
    str.sprintf("%6.3f", sum1);
    Edit102->Text=str;
}
//---------------------------------------------------------------------------
// golden :1079-1086
void TfMotorTest::btnSetPosPClick(TObject *Sender)
{
    if(ActiveIndex==-1 || btnHome->Down || btnLoopMove->Down)                   //Steven 20230731 : Add protection
        return;

    if(MotorTestClass[ActiveIndex]->Visible==true)                              //Steven 20240822 : 沒用到的馬達不new
        MotorTestClass[ActiveIndex]->edPos1->Text=edtCommandPos->Text;
}
//---------------------------------------------------------------------------
// golden :1088-1095
void TfMotorTest::btnSetPosNClick(TObject *Sender)
{
    if(ActiveIndex==-1 || btnHome->Down || btnLoopMove->Down)                   //Steven 20230731 : Add protection
        return;

    if(MotorTestClass[ActiveIndex]->Visible==true)                              //Steven 20240822 : 沒用到的馬達不new
        MotorTestClass[ActiveIndex]->edPos2->Text=edtCommandPos->Text;
}
//---------------------------------------------------------------------------
// golden :1280-1298  pure EMG predicate -- reads Sen[] only, issues no command
bool TfMotorTest::IsMotorCanRun(bool attr)
{
    bool flag=true;
    if(Sen[SnFrontLeftEMG].IsOff())
        flag=false;
    if(Sen[SnFrontRightEMG].IsOff())
        flag=false;
    if(Sen[SnRearLeftEMG].IsOff())
        flag=false;
    if(Sen[SnRearRightEMG].IsOff())
        flag=false;
    if(IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff())                            //KenHsieh 20250212 : 新增PLC 斷線可瞬間判斷EMG及安全門  AI(W906-W217) 20261010 (Ifor01): golden 913 uMotorTest.cpp:1296
        flag=false;
    if(Sen[SnServo].Enable && Sen[SnServo].IsOff())                             //kevin 20140121  EMG沒有完全按下去
        flag=false;
    if(attr && flag==false)
        ShowMyMessage("EMG Stop", "");
    return flag;
}
//---------------------------------------------------------------------------
// golden :1347-1362  TCloseAction dropped (DEVIATION D-9); the whole system\MotorTest.ini WRITE block is GATE (M-IniWrite)
void TfMotorTest::FormClose(TObject *Sender)
{
//GATE(M-IniWrite)     AnsiString sPath=asSystemPath+"MotorTest.ini";
    fShow=false;
//GATE(M-IniWrite)     for(int i=0; i<int(MotorTestClass.size()); i++)
//GATE(M-IniWrite)     {
//GATE(M-IniWrite)         if(MotorTestClass[i]->Visible==true)                                    //Steven 20240822 : 沒用到的馬達不new
//GATE(M-IniWrite)         {
//GATE(M-IniWrite)             WriteIniData(sPath, MotorTestClass[i]->labName->Caption, "Position 1", MotorTestClass[i]->edPos1->Text);    //Steven 20120831 : 怕會記憶體破壞，改用ini
//GATE(M-IniWrite)             WriteIniData(sPath, MotorTestClass[i]->labName->Caption, "Position 2", MotorTestClass[i]->edPos2->Text);
//GATE(M-IniWrite)         }
//GATE(M-IniWrite)     }

    PauseUT150Polling=false;
}
//---------------------------------------------------------------------------
// golden :1451-1456
void TfMotorTest::btnRangeClick(TObject *Sender)
{
    if(ActiveIndex==-1)
        return;
    UpdateMotorParameter();
}
//---------------------------------------------------------------------------
// golden :1458-1463
void TfMotorTest::btnRateClick(TObject *Sender)
{
    if(ActiveIndex==-1)
        return;
    UpdateMotorParameter();
}
//---------------------------------------------------------------------------
// golden :1465-1469
void TfMotorTest::SpeedButton4Click(TObject *Sender)
{
    Edit3->Text=Edit16->Text;
    Edit4->Text=Edit18->Text;
}
//---------------------------------------------------------------------------
// golden :1471-1475
void TfMotorTest::SpeedButton5Click(TObject *Sender)
{
    Edit5->Text=Edit16->Text;
    Edit6->Text=Edit18->Text;
}
//---------------------------------------------------------------------------
// golden :1477-1481
void TfMotorTest::SpeedButton8Click(TObject *Sender)
{
    Edit12->Text=Edit16->Text;
    Edit13->Text=Edit18->Text;
}
//---------------------------------------------------------------------------
// golden :1483-1487
void TfMotorTest::SpeedButton6Click(TObject *Sender)
{
    Edit8->Text=Edit102->Text;
    Edit10->Text=Edit103->Text;
}
//---------------------------------------------------------------------------
// golden :1489-1493
void TfMotorTest::SpeedButton7Click(TObject *Sender)
{
    Edit7->Text=Edit102->Text;
    Edit11->Text=Edit103->Text;
}
//---------------------------------------------------------------------------
// golden :1495-1499
void TfMotorTest::SpeedButton9Click(TObject *Sender)
{
    Edit15->Text=Edit102->Text;
    Edit17->Text=Edit103->Text;
}
//---------------------------------------------------------------------------
// golden :1673-1685  (G-VecIndex: indexes MotorTestClass by MOTOR ID)
void TfMotorTest::ck00Click(TObject *Sender)
{
    if(MotorTestClass[MTestY1]->cbUsing->Checked ||
       MotorTestClass[MTestZ1]->cbUsing->Checked ||
       MotorTestClass[MTestZ2]->cbUsing->Checked ||
       MotorTestClass[MTestY2]->cbUsing->Checked)
    {
        MotorTestClass[MTestY1]->cbUsing->Checked=false;
        MotorTestClass[MTestZ1]->cbUsing->Checked=false;
        MotorTestClass[MTestZ2]->cbUsing->Checked=false;
        MotorTestClass[MTestY2]->cbUsing->Checked=false;
    }
}
//---------------------------------------------------------------------------
// golden :1713-1716
void TfMotorTest::edtSpeedClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 1, true, 100, 1);
}
//---------------------------------------------------------------------------
// golden :1727-1730  Close() is the documented offline no-op (DEVIATION D-8)
void TfMotorTest::palExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
// golden :1732-1747  the two ht9045_sm-layer calls are GATE (M-Destroy)
void TfMotorTest::FormDestroy(TObject *Sender)
{
    try
    {
        for(vector<TMotorTestClass *>::iterator iter=MotorTestClass.begin(); iter!=MotorTestClass.end(); ++iter)
        {
            delete *iter;
        }
        vec_clr(MotorTestClass);
    }
    catch(...)
    {
//GATE(M-Destroy) MyDBIProcess("Exception", "TfMotorTest::FormDestroy");
    }
//GATE(M-Destroy) LogSoftwareOffTime("TfMotorTest, FormDestroy");                             //Steven 20210526 : 紀錄軟體執行時間
}
//---------------------------------------------------------------------------
// golden :2039-2043
void TfMotorTest::edDelayTimeMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 1000, 0);
}
//---------------------------------------------------------------------------
// golden :2045-2049
void TfMotorTest::edPitechMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 10000, 1);
}
//---------------------------------------------------------------------------
// golden :2121-2133
void TfMotorTest::rgAxisClick(TObject *Sender)
{
    if(rgAxis->ItemIndex==1)
        iUseAxis=0;
    else if(rgAxis->ItemIndex==2)
        iUseAxis=1;
    else if(rgAxis->ItemIndex==3)
        iUseAxis=19;
    else if(rgAxis->ItemIndex==4)
        iUseAxis=20;
    else
        iUseAxis=-1;
}
//---------------------------------------------------------------------------
// golden :2135-2179  Mot_Table.csv READ only -- the matching SAVE (sbUpdateClick) is GATE (W-01)
void TfMotorTest::sbtReloadClick(TObject *Sender)
{
    TStringList *sList=new TStringList();
    TStringList *SL=new TStringList();

    int iCol=0, iRow=0;

    for(int i=0; i<strngrdMotorData->RowCount; i++)
    {
        for(int j=0; j<strngrdMotorData->ColCount; j++)
        {
            strngrdMotorData->Cells[j][i]="";
        }
    }

    if(FileExists(MotTablePath))
    {
        sList->LoadFromFile(MotTablePath);
        iRow=sList->Count;
        if(iRow>0)
        {
            strngrdMotorData->RowCount=iRow;
            for(int i=0; i<iRow; i++)
            {
                SL->CommaText=sList->Strings[i];

                if(SL->Count>iCol)
                {
                    iCol=SL->Count;
                    strngrdMotorData->ColCount=iCol;
                }

                for(int j=0; j<SL->Count; j++)
                {
                    strngrdMotorData->Cells[j][i]=SL->Strings[j];
                }
            }
        }
    }
    sList->Clear();
    SL->Clear();
    delete sList;
    delete SL;
    sbtReload->Down=false;
}
//---------------------------------------------------------------------------
// golden :2181-2186
void TfMotorTest::strngrdMotorDataSelectCell(TObject *Sender,
      int ACol, int ARow, bool &CanSelect)
{
    iSelectRow=ARow;
    iSelectCol=ACol;
}
//---------------------------------------------------------------------------
// golden :2188-2197
void TfMotorTest::btnAddMotorClick(TObject *Sender)
{
    int iRow=strngrdMotorData->RowCount;
    strngrdMotorData->RowCount=iRow+1;
    for(int j=0; j<strngrdMotorData->ColCount; j++)
    {
        strngrdMotorData->Cells[j][iRow]="";
    }
    btnAddMotor->Down=false;
}
//---------------------------------------------------------------------------
// golden :2199-2212
void TfMotorTest::btnDeleteMotorClick(TObject *Sender)
{
    if(iSelectRow==0)
        return;
    for(int i=iSelectRow; i<strngrdMotorData->RowCount-1; i++)
    {
        for(int j=0; j<strngrdMotorData->ColCount; j++)
        {
            strngrdMotorData->Cells[j][i]=strngrdMotorData->Cells[j][i+1];
        }
    }
    strngrdMotorData->RowCount=strngrdMotorData->RowCount-1;
    btnDeleteMotor->Down=false;
}
//---------------------------------------------------------------------------
// golden :2214-2238
void TfMotorTest::btnModifyClick(TObject *Sender)
{
    if(iSelectRow>0)
    {
        Edit2->Text=strngrdMotorData->Cells[iSelectCol][iSelectRow];
        if(strngrdMotorData->Cells[iSelectCol][0].AnsiPos(AnsiString("Motorname"))!=0 ||
           strngrdMotorData->Cells[iSelectCol][0].AnsiPos(AnsiString("Alias"))!=0     ||
           strngrdMotorData->Cells[iSelectCol][0].AnsiPos(AnsiString("CardModel"))!=0)
        {
            fQwertyKey->ShowQwertyKey(Edit2, N_NO_SYMBOL|N_NO_SPACE);
        }
        else if(strngrdMotorData->Cells[iSelectCol][0].AnsiPos(AnsiString("GearRatio"))!=0 ||
                strngrdMotorData->Cells[iSelectCol][0].AnsiPos(AnsiString("Acc"))!=0     ||
                strngrdMotorData->Cells[iSelectCol][0].AnsiPos(AnsiString("Dec"))!=0)
        {
            fQwertyKey->ShowQwertyKey(Edit2, N_DOUBLE);
        }
        else
        {
            fQwertyKey->ShowQwertyKey(Edit2, N_INTEGER);
        }
        strngrdMotorData->Cells[iSelectCol][iSelectRow]=Edit2->Text;
    }
    btnModify->Down=false;
}
//---------------------------------------------------------------------------
// golden :2270-2273  Click() is the documented offline no-op (vclcompat/Controls.h TControl::Click)
void TfMotorTest::strngrdMotorDataDblClick(TObject *Sender)
{
    btnModify->Click();
}
//---------------------------------------------------------------------------
// golden :2346-2349
void TfMotorTest::edtMotorRateClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 100, 1);
}
//---------------------------------------------------------------------------
// golden :2351-2354
void TfMotorTest::edtMotorRangeClick(TObject *Sender)
{
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 1, 1000);
}
//---------------------------------------------------------------------------
// golden :2356-2364
void TfMotorTest::LockAllButton(bool bLock)                                     //Steven 20230815 : Add for MotorTest
{
//    sbMotorTest_JogN->Enabled   =!bLock;                                      //不能鎖定Jog, 會導致功能失效
//    sbMotorTest_JogP->Enabled   =!bLock;
    sbMotorTest_MoveN->Enabled  =!bLock;
    sbMotorTest_MoveP->Enabled  =!bLock;
    btnLoopMove->Enabled        =!bLock;
    pnlStop->Color=(bLock)?clYellow:(TColor)0x00DFD9CC;                         //Steven 20230815 : Add for MotorTest
}
//---------------------------------------------------------------------------
// golden :2366-2375
void TfMotorTest::AllBtnUp()                                                    //Steven 20230815 : Add for MotorTest
{
    btnLoopMove->Down       =false;
    btnHome->Down           =false;
    btnGoSoftP->Down        =false;
    btnGoSoftN->Down        =false;
    sbMotorTest_MoveN->Down =false;
    sbMotorTest_MoveP->Down =false;
    btnGo->Down             =false;
}
//---------------------------------------------------------------------------
// =============================================================================
//  END OF DELIVERED BODIES.
//
//  The 58 gated methods have NO definition in this file, on purpose -- see the
//  gate-mechanism note at the head of this file and the GATE REGISTER in
//  forms/fMotorTest.h.  If you came here looking for one of them:
//    S-01..S-52  motion / servo / motor-power / motor-parameter WRITES
//    W-01        sbUpdateClick -- rewrites system\Mot_Table.csv
//    X-01..X-03  need TStringGrid::MouseToCell (no headless meaning)
//    X-04..X-05  need ShowMessage (no port) + .csv export of gated data
// =============================================================================

// =============================================================================
//  AI(W906-MT-E1) 20260925: golden Motor Test motor-list visibility, for the web page.
//  User EastSun 20260925: 「Motor Test 要顯示哪些馬達？」 -> 「照舊版程式的規則」.
//  wb_serve never builds fMotorTest, so the bView expressions of TfMotorTest::Init() above (golden
//  uMotorTest.cpp:145-390, e.g. MOutShuttle1/2 = false, MInArmZB..ZH on InOutArmPickerUseMotor, MLoaderZ on
//  LOAD_Z_USE_MOTOR[0]) are repeated here -- GENERATED from the Init() push_backs by a script (same order,
//  expressions copied verbatim), evaluated when called, so the page follows the machine's current flags.
//  JsonBridge/ChanMotorPoints.cpp MotorConfigJson publishes it per Mot_Table row as "motorTestVisible".
//  ⚠ Re-generate when Init() changes (the two must stay identical).
// =============================================================================
std::vector<std::pair<int, bool> > W906_MotorTestVisibility()
{
    std::vector<std::pair<int, bool> > v;
    v.push_back(std::make_pair((int)(MInArmX), (bool)(true)));
    v.push_back(std::make_pair((int)(MInArmY), (bool)(true)));
    v.push_back(std::make_pair((int)(MInArmPitch), (bool)(true)));
    v.push_back(std::make_pair((int)(MInArmZA), (bool)(true)));
    v.push_back(std::make_pair((int)(MInArmZB), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MInArmZC), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MInArmZD), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MInArmZE), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MInArmZF), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MInArmZG), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MInArmZH), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MInShuttle1), (bool)(true)));
    v.push_back(std::make_pair((int)(MInShuttle2), (bool)(true)));
    v.push_back(std::make_pair((int)(MTestY1), (bool)(true)));
    v.push_back(std::make_pair((int)(MTestZ1), (bool)(true)));
    v.push_back(std::make_pair((int)(MTestZ2), (bool)(true)));
    v.push_back(std::make_pair((int)(MTestY2), (bool)(true)));
    v.push_back(std::make_pair((int)(MOutShuttle1), (bool)(false)));
    v.push_back(std::make_pair((int)(MOutShuttle2), (bool)(false)));
    v.push_back(std::make_pair((int)(MOutArmX), (bool)(true)));
    v.push_back(std::make_pair((int)(MOutArmY), (bool)(true)));
    v.push_back(std::make_pair((int)(MOutArmPitch), (bool)(true)));
    v.push_back(std::make_pair((int)(MOutArmZA), (bool)(true)));
    v.push_back(std::make_pair((int)(MOutArmZB), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MOutArmZC), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MOutArmZD), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MOutArmZE), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MOutArmZF), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MOutArmZG), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MOutArmZH), (bool)((InOutArmPickerUseMotor==eptUseMot))));
    v.push_back(std::make_pair((int)(MTrayX), (bool)(true)));
    v.push_back(std::make_pair((int)(MInArmPitchY), (bool)((USE_IN_Y_IS_AUTO_PITCH==true))));
    v.push_back(std::make_pair((int)(MInArmPitchX2), (bool)((USE_IN_Y_IS_AUTO_PITCH==true))));
    v.push_back(std::make_pair((int)(MOutArmPitchY), (bool)((USE_OUT_Y_IS_AUTO_PITCH==true))));
    v.push_back(std::make_pair((int)(MOutArmPitchX2), (bool)((USE_OUT_Y_IS_AUTO_PITCH==true))));
    v.push_back(std::make_pair((int)(MLoaderZ), (bool)(LOAD_Z_USE_MOTOR[0])));
    v.push_back(std::make_pair((int)(MEmptyZ), (bool)(LOAD_Z_USE_MOTOR[1])));
    v.push_back(std::make_pair((int)(MColorZ), (bool)(LOAD_Z_USE_MOTOR[2])));
    v.push_back(std::make_pair((int)(MAuto1Z), (bool)((LOAD_Z_USE_MOTOR[3] || USE_LdUldCassetteMode==1))));
    v.push_back(std::make_pair((int)(MAuto2Z), (bool)((LOAD_Z_USE_MOTOR[4] || USE_LdUldCassetteMode==1))));
    v.push_back(std::make_pair((int)(MAuto3Z), (bool)(LOAD_Z_USE_MOTOR[5])));
    v.push_back(std::make_pair((int)(MInRotateKit), (bool)((USE_ROTATE_KIT && iRotate_Type!=eCynRotate))));
    v.push_back(std::make_pair((int)(MOutRotateKit), (bool)((USE_ROTATE_KIT && iRotate_Type!=eCynRotate && iRotate_Type!=e4MotRotate && iRotate_Type!=e2MotRotate2Dut))));
    v.push_back(std::make_pair((int)(MAOIKit), (bool)(USE_AOI_Inspection)));
    v.push_back(std::make_pair((int)(MLoaderY), (bool)(((INSTALL_OCR_YMot==eocrYMotInstal || (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR) || USE_LdUldCassetteMode==1)))));
    v.push_back(std::make_pair((int)(MEmptyY), (bool)(false)));
    v.push_back(std::make_pair((int)(MColorY), (bool)(false)));
    v.push_back(std::make_pair((int)(MAuto1Y), (bool)(USE_LdUldCassetteMode==1)));
    v.push_back(std::make_pair((int)(MAuto2Y), (bool)(USE_LdUldCassetteMode==1)));
    v.push_back(std::make_pair((int)(MAuto3Y), (bool)(false)));
    v.push_back(std::make_pair((int)(MInArmZAe), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MInArmPitchX3), (bool)((USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    v.push_back(std::make_pair((int)(MInArmPitchX4), (bool)((USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    v.push_back(std::make_pair((int)(MInArmZAf), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmPitchX3), (bool)((USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    v.push_back(std::make_pair((int)(MOutArmPitchX4), (bool)((USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be))));
    v.push_back(std::make_pair((int)(MTrayZ), (bool)((TRAY_ARM_MODE==eUnderCoveyor || USE_LdUldCassetteMode==1))));
    v.push_back(std::make_pair((int)(MOutSortAa), (bool)((USE_OUT_SORT_ARM==iOutSortX40mm))));
    v.push_back(std::make_pair((int)(MOutSortAb), (bool)((USE_OUT_SORT_ARM==iOutSortX40mm))));
    v.push_back(std::make_pair((int)(MInArmXScale), (bool)(USE_MAGNETIC_SCALE)));
    v.push_back(std::make_pair((int)(MInArmYScale), (bool)(USE_MAGNETIC_SCALE)));
    v.push_back(std::make_pair((int)(MOutArmXScale), (bool)(USE_MAGNETIC_SCALE)));
    v.push_back(std::make_pair((int)(MOutArmYScale), (bool)(USE_MAGNETIC_SCALE)));
    v.push_back(std::make_pair((int)(MShuttle1Pitch), (bool)(AUTO_SENSOR_INSTALL)));
    v.push_back(std::make_pair((int)(MShuttle2Pitch), (bool)(AUTO_SENSOR_INSTALL)));
    v.push_back(std::make_pair((int)(MInRotateB), (bool)((USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate)))));
    v.push_back(std::make_pair((int)(MInRotateC), (bool)((USE_ROTATE_KIT && iRotate_Type==e8MotRotate))));
    v.push_back(std::make_pair((int)(MInRotateD), (bool)((USE_ROTATE_KIT && iRotate_Type==e8MotRotate))));
    v.push_back(std::make_pair((int)(MInRotateE), (bool)((USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut)))));
    v.push_back(std::make_pair((int)(MInRotateF), (bool)((USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate)))));
    v.push_back(std::make_pair((int)(MInRotateG), (bool)((USE_ROTATE_KIT && iRotate_Type==e8MotRotate))));
    v.push_back(std::make_pair((int)(MInRotateH), (bool)((USE_ROTATE_KIT && iRotate_Type==e8MotRotate))));
    v.push_back(std::make_pair((int)(MOutRotateB), (bool)((USE_ROTATE_KIT && iRotate_Type==e8MotRotate))));
    v.push_back(std::make_pair((int)(MOutRotateC), (bool)((USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut)))));
    v.push_back(std::make_pair((int)(MOutRotateD), (bool)((USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate)))));
    v.push_back(std::make_pair((int)(MOutRotateE), (bool)((USE_ROTATE_KIT && iRotate_Type==e8MotRotate))));
    v.push_back(std::make_pair((int)(MOutRotateF), (bool)((USE_ROTATE_KIT && iRotate_Type==e8MotRotate))));
    v.push_back(std::make_pair((int)(MOutRotateG), (bool)((USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate || iRotate_Type==e2MotRotate2Dut)))));
    v.push_back(std::make_pair((int)(MOutRotateH), (bool)((USE_ROTATE_KIT && (iRotate_Type==e8MotRotate || iRotate_Type==e4MotRotate)))));
    v.push_back(std::make_pair((int)(MLightScale), (bool)(USE_TRAY_ROBOT)));
    v.push_back(std::make_pair((int)(MInArmZAg), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MInArmZAh), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MArmAlignment), (bool)(false)));
    v.push_back(std::make_pair((int)(MLoadHingeR), (bool)(USE_LOADER_HINGE)));
    v.push_back(std::make_pair((int)(MLoadHingeZ), (bool)(USE_LOADER_HINGE)));
    v.push_back(std::make_pair((int)(MPreciser), (bool)(USE_PRECISER)));
    v.push_back(std::make_pair((int)(MInArmZBe), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MInArmZBf), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MInArmZBg), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MInArmZBh), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmZAe), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmZAf), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmZAg), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmZAh), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmZBe), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmZBf), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmZBg), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutArmZBh), (bool)((InOutArmPickerUseMotor==eptUseMot && (USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker || USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)))));
    v.push_back(std::make_pair((int)(MOutSortX), (bool)((USE_OUT_SORT_ARM==iOutSortX40mm))));
    v.push_back(std::make_pair((int)(MCaselevatorZ), (bool)(USE_MR_SYSTEM)));
    v.push_back(std::make_pair((int)(MCasArmX), (bool)(USE_MR_SYSTEM)));
    v.push_back(std::make_pair((int)(MCasArmZ), (bool)(USE_MR_SYSTEM)));
    v.push_back(std::make_pair((int)(MTrayBracketZ), (bool)(USE_MR_SYSTEM)));
    v.push_back(std::make_pair((int)(MStackedTrayX), (bool)(USE_MR_SYSTEM)));
    v.push_back(std::make_pair((int)(MStackedTrayZ), (bool)(USE_MR_SYSTEM)));
    v.push_back(std::make_pair((int)(MUnloadRobotZ), (bool)(USE_MR_SYSTEM)));
    v.push_back(std::make_pair((int)(MOutSortY), (bool)((USE_OUT_SORT_ARM==iOutSortX40mm))));
    v.push_back(std::make_pair((int)(MCCDX), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(MCCDY), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(MCCDZ), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(MInFlipper1), (bool)(USE_DEVICE_FLIPPER)));
    v.push_back(std::make_pair((int)(MInFlipper2), (bool)(USE_DEVICE_FLIPPER)));
    v.push_back(std::make_pair((int)(MInFlipper3), (bool)(USE_DEVICE_FLIPPER)));
    v.push_back(std::make_pair((int)(MOutFlipper1), (bool)(USE_DEVICE_FLIPPER)));
    v.push_back(std::make_pair((int)(MOutFlipper2), (bool)(USE_DEVICE_FLIPPER)));
    v.push_back(std::make_pair((int)(MOutFlipper3), (bool)(USE_DEVICE_FLIPPER)));
    v.push_back(std::make_pair((int)(MLdCarRotArm), (bool)(USE_LD_Rot_Arm)));
    v.push_back(std::make_pair((int)(MLoaderY_CCW), (bool)(USE_LdUldCassetteMode==1)));
    v.push_back(std::make_pair((int)(MAuto1Y_CCW), (bool)(USE_LdUldCassetteMode==1)));
    v.push_back(std::make_pair((int)(MAuto2Y_CCW), (bool)(USE_LdUldCassetteMode==1)));
    v.push_back(std::make_pair((int)(MAuto3Y_CCW), (bool)(false)));
    v.push_back(std::make_pair((int)(MAuto4Y_CCW), (bool)(false)));
    v.push_back(std::make_pair((int)(MAuto5Y_CCW), (bool)(false)));
    v.push_back(std::make_pair((int)(MAuto6Y_CCW), (bool)(false)));
    v.push_back(std::make_pair((int)(M1_3R), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_4X), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_4Y), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_4R), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_5X), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_5Y), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_5R), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_6X), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_6Y), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_6R), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_7X), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_7Y), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_7R), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_8X), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_8Y), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(M1_8R), (bool)(USE_FINE_PITCH)));
    v.push_back(std::make_pair((int)(MMagazine), (bool)((AUTO3_IS_MAGAZINE==1))));
    v.push_back(std::make_pair((int)(MCatchMgzTray), (bool)((AUTO3_IS_MAGAZINE==1))));
    v.push_back(std::make_pair((int)(MMagYTrayOut), (bool)(false)));
    v.push_back(std::make_pair((int)(MFix3Full), (bool)((FIX3_FULL_PLACE==Fix3K_UseStepperMotor))));
    v.push_back(std::make_pair((int)(MAuto4Z), (bool)(LOAD_Z_USE_MOTOR[6])));
    v.push_back(std::make_pair((int)(MAuto5Z), (bool)(LOAD_Z_USE_MOTOR[7])));
    v.push_back(std::make_pair((int)(MAuto6Z), (bool)(LOAD_Z_USE_MOTOR[8])));
    v.push_back(std::make_pair((int)(MAuto4Y), (bool)(false)));
    v.push_back(std::make_pair((int)(MAuto5Y), (bool)(false)));
    v.push_back(std::make_pair((int)(MAuto6Y), (bool)(false)));
    v.push_back(std::make_pair((int)(MTopAOIArmX), (bool)(false)));
    v.push_back(std::make_pair((int)(MTopAOIArmY), (bool)(false)));
    v.push_back(std::make_pair((int)(MTopAOIArmR), (bool)(false)));
    v.push_back(std::make_pair((int)(MTopAOICCDZ), (bool)((USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall))));
    v.push_back(std::make_pair((int)(MTopAOIElevZ1), (bool)(false)));
    v.push_back(std::make_pair((int)(MTopAOIElevZ2), (bool)(false)));
    v.push_back(std::make_pair((int)(MOutSortPitchX), (bool)((USE_OUT_SORT_ARM==iOutSortX40mm))));
    v.push_back(std::make_pair((int)(MOutSortSht), (bool)((USE_OUT_SORT_ARM==iOutSortX40mm))));
    v.push_back(std::make_pair((int)(MLoad2Z), (bool)(USE_2nd_LOADER)));
    v.push_back(std::make_pair((int)(MLoad2Y), (bool)(false)));
    v.push_back(std::make_pair((int)(MInSh1LtcSenZ1), (bool)(In_Shuttle_Auto_Latch)));
    v.push_back(std::make_pair((int)(MInSh1LtcSenZ2), (bool)(In_Shuttle_Auto_Latch)));
    v.push_back(std::make_pair((int)(MInSh2LtcSenZ1), (bool)(In_Shuttle_Auto_Latch)));
    v.push_back(std::make_pair((int)(MInSh2LtcSenZ2), (bool)(In_Shuttle_Auto_Latch)));
    return v;
}
