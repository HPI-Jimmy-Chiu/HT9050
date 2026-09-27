// =============================================================================
//  forms/fRotate.h  --  non-VCL stand-in for golden's FrmRotate form pointer
//
//  AI(W906-W7-L2) 20260803: FIRST home for TFrmRotate anywhere in this migrated
//  tree (golden RotateKit/fRotate.h:60 `class TFrmRotate : public TForm`).
//  Landed by the W7-L2 substrate pass ahead of the translation of golden
//  ckernel.cpp.  forms/fNote.{h,cpp} convention; the facade contract in
//  forms/fMain.h binds this file too.
//
//  ---------------------------------------------------------------------------
//  CLASS NAME: `TFrmRotate`, NOT `TfRotate`.  DEVIATION FROM THE BRIEF, ON
//  GOLDEN'S AUTHORITY.
//  ---------------------------------------------------------------------------
//  The W7-L2 assignment asked for "TfRotate / global FrmRotate".  Golden
//  RotateKit/fRotate.h:60 says `class TFrmRotate : public TForm` and
//  RotateKit/fRotate.h:147 says `extern PACKAGE TFrmRotate *FrmRotate;`, and
//  every golden method definition is spelled `TFrmRotate::` (fRotate.cpp:42,
//  :1158, :1170, :1191, :1203, :1220).  Golden is the only authority, so the
//  class is TFrmRotate here.  Flagged rather than silently corrected.
//
//  ---------------------------------------------------------------------------
//  GOLDEN ckernel.cpp CENSUS FOR FrmRotate -- MEASURED OVER ALL 2589 LINES
//  ---------------------------------------------------------------------------
//    :419  else if(FrmRotate->bRotateInHome)        (read)  ScanSystemSensor
//    :421  FrmRotate->InitialInRotateHome();        (call)  ScanSystemSensor
//    :423  else if(FrmRotate->bRotateOutHome)       (read)  ScanSystemSensor
//    :425  FrmRotate->InitialOutRotateHome();       (call)  ScanSystemSensor
//  Exactly 4 sites, 2 flags + 2 methods -- all inside the :413-439 home ladder.
//  Golden's TFrmRotate also declares DoInRotateHome / DoOutRotateHome /
//  SetInRotateSpeed / SetOutRotateSpeed etc.; none is reachable from ckernel.cpp
//  and none is landed.
//
//  b8RotateInHome / b8RotateOutHome ARE landed even though ckernel.cpp never
//  names them: they are not speculative growth, they are the STORAGE the two
//  landed methods write (golden fRotate.cpp:1164 and :1197).  Without them the
//  methods could only have been fake-success stubs, which this wave does not
//  emit.
// =============================================================================
#ifndef FORMS_FROTATE_H
#define FORMS_FROTATE_H

// Dependency-free header: bools and method declarations only.  The method
// BODIES (forms/fRotate.cpp) need cmydef.h for the MInRotate/MOutRotate motor-id
// tables, but that include belongs in the .cpp, not here -- see forms/fHome.h
// for the same reasoning.

// -----------------------------------------------------------------------------
//  //AI(W906-FRW-NoPage) 20260926: golden RotateKit/fRotate.h:22-59 的 enum tDutType_* 與 struct TRotate
//  （全域 tRotate 的完整型別）搬進本檔，逐字。原因：golden 讀 Rotate.Data 的 TFrmRotate::fRotate_ReadFile
//  （現在是 FileRW/Rotate.gen.inc 的 RT_fRotate_ReadFile）要寫 TRotate 的全部成員，而移植樹原本的
//  tRotate 是 aHotPlateSubstrate.h 的 tRotateShim（只有 ActiveRotate／RotateDutDate／bRotateUseRTmode）。
//  aHotPlateSubstrate.h 改成 `typedef TRotate tRotateShim;`——同一個型別、同一份儲存
//  （定義仍在 aHotPlateSubstrate.cpp:1077；初值由 `= { false }` 改成同值的 `= {}`，免得多出 -Wmissing-field-initializers），不是第二份宣告。
//  本檔仍不 include 任何東西（TRotate 只有 bool／int／double）。
//  enum 的守衛巨集沿用 RotateKit/aRotateKIT_In.cpp:329／aRotateKIT_Out.cpp:323 已經在用的
//  HT9045_TDUTTYPE_ENUM_DEFINED：那兩份 TU 內副本在 include 本檔（經 aHotPlateSubstrate.h）之後自動跳過。
// -----------------------------------------------------------------------------
#ifndef HT9045_TDUTTYPE_ENUM_DEFINED
#define HT9045_TDUTTYPE_ENUM_DEFINED
enum {tDutType_4=0,
      tDutType_8=1,
      tDutType_1=2,
      tDutType_2=3,
      tDutType_Total};
#endif
//---------------------------------------------------------------------------
struct TRotate
{
    bool ActiveRotate;
    bool bUseDifferentAngle;
    double FromTrayAngle;
    int RotateDutDate[4][4][8];                                                 //Ifor 20170406 add In/Out Rotate  //0:In 1:out 2:In RT 3:Out RT
    int DutNum;
    int ColCount;
    int RowCount;
    int RotationTimeIn;                                                         //Ifor 20170414 (Steven) RotationTime ==> RotationTimeIn
    int RotationTimeOut;                                                        //Ifor 20170414 (Steven) add Out Rotate 旋轉次數
//    int iRotateDegree_In[4][8];
//    int iRotateDegree_Out[4][8];
    int RotationCount[8];                                                       //最小角度是90,360/90 最多4次    //Ifor 20170407 (Steven) 4 => 8
    int iCurrentRotDegree_In[2][4];
    int iCurrentRotDegree_Out[2][4];

    double RotationSequence[4][8];
    double RotateKit_PitchX;
    double RotateKit_PitchY;
    int OutRotationCount[8];                                                    //kevin 20131003 out rotate 最小角度是90,360/90 最多4次   //Ifor 20170407 4 => 8

    bool bPassBinNoRotate;                                                      //jou 20231020 : Pass bin no rotate
    bool bRotateUseRTmode;                                                      //jou 20231122 : Rotate Use RT mode
    int RotationTimeInRT;                                                       //jou 20231122 : Rotate Use RT mode
    int RotationTimeOutRT;                                                      //jou 20231122 : Rotate Use RT mode
    int RotationCountRT[8];                                                     //jou 20231122 : Rotate Use RT mode
    int OutRotationCountRT[8];                                                  //jou 20231122 : Rotate Use RT mode
    bool bART_RT_NoRotate;                                                      //Sam 20240809 : ART RT No Rotate
    int iRotateOffset[2];                                                       //Ifor 20251210 add:Rotate Offset
    int iPreRotate[2];                                                          //Ifor 20260901 add:Pre Rotate degree before pick (0:In@Loader 1:Out@OutShuttle)
};

class TFrmRotate
{
public:
    // -----------------------------------------------------------------------
    //  [DATA] golden RotateKit/fRotate.h:137 `bool bRotateInHome;`
    //         golden RotateKit/fRotate.h:138 `bool bRotateOutHome;`
    //  "a single-motor home of the IN (resp. OUT) rotate kit has been requested
    //  and is still pending".
    //
    //  OFFLINE VALUE false for both.  GOLDEN HAS NO CTOR ASSIGNMENT FOR THESE --
    //  `__fastcall TFrmRotate::TFrmRotate(TComponent* Owner)` (fRotate.cpp:42)
    //  does not mention either flag; golden relies on the VCL guarantee that
    //  TObject::NewInstance zero-fills the instance before the constructor runs,
    //  so an unassigned bool member of a TForm is false at construction.  false
    //  is therefore golden's real initial value, arrived at a different way from
    //  fHome/fSetup (whose ctors assign explicitly), and it is worth saying so
    //  rather than quietly writing false.
    //  It is corroborated operationally, not just by the ABI: golden
    //  csystem.cpp:17863-17864 clears BOTH flags at the top of the home-all
    //  sequence, and the ONLY places that ever set them true are the rotate-kit
    //  SMs -- RotateKit/aRotateKIT_In.cpp:1404 and :3264 (bRotateInHome=true)
    //  and RotateKit/aRotateKIT_Out.cpp:1414 and :3356 (bRotateOutHome=true).
    //  Neither SM is translated, so offline they stay false permanently.
    //  AI(W906-T6-MAINPROC) 20260923: ⚠ 上一句**已過期** —— 兩支旋轉站狀態機後來都翻成活的
    //  （RotateKit/aRotateKIT_In.cpp:1806/:3712、aRotateKIT_Out.cpp:1858/:3827 會設 true），
    //  而 MainProc 照 golden 翻活後由 DoIn/OutRotateHome() 清掉（見下面兩支方法）。
    //
    //  THE BRANCH false SELECTS -- and it is a consequential one:
    //  golden ckernel.cpp:413-439 is a five-way ladder inside `if(fAllMotorHome)`
    //  (itself reached only because fSetup->fShow is false; see forms/fSetup.h):
    //        :415 if(bNeedArmZHome)              -> InitDoArmZHome()
    //        :419 else if(bRotateInHome)         -> InitialInRotateHome()
    //        :423 else if(bRotateOutHome)        -> InitialOutRotateHome()
    //        :427 else if(bYpitchNeddHome)       -> bYpitchNeddHome=true
    //        :431 else -> if(CheckMotorHome()==false)
    //                        ShowMyMessage("Must home again"); return false;
    //  With BOTH flags false the two rotate rungs are skipped and control falls
    //  through to the final else, i.e. START is gated on the FULL CheckMotorHome
    //  verification and can be REFUSED (`return false`).  Setting either flag
    //  true instead would short-circuit the ladder into a single-axis rotate
    //  re-home and let START past without the full check.  false is thus the
    //  strict arm, and the faithful one for a machine that is not mid-rotate-kit
    //  recovery.
    // -----------------------------------------------------------------------
    bool bRotateInHome;
    bool bRotateOutHome;

    // -----------------------------------------------------------------------
    //  [DATA] golden RotateKit/fRotate.h:135 `bool b8RotateInHome[2][4];`
    //         golden RotateKit/fRotate.h:136 `bool b8RotateOutHome[2][4];`
    //  Per-nozzle "this rotate motor has finished homing" grid, 2 rows x 4 cols
    //  (golden's own literal loop bounds at fRotate.cpp:1160/1162, which match
    //  this tree's MAX_ARM_Row=2 / MAX_ARM_Col=4 in MachineType.h:387-388).
    //  Written false by the two methods below; read back by golden's
    //  DoInRotateHome/DoOutRotateHome (fRotate.cpp:1179/:1210), which are NOT
    //  landed -- so offline these are write-only, exactly as the landed methods
    //  leave them.  Stated rather than glossed.
    //  AI(W906-T6-MAINPROC) 20260923: ⚠ 已過期 —— DoInRotateHome/DoOutRotateHome 已落地（RotateKit/fRotate.cpp），
    //  這兩格現在有讀者。
    //  OFFLINE VALUE: all false, via the same VCL zero-fill argument as the two
    //  flags above; the ctor in forms/fRotate.cpp writes them explicitly so the
    //  value is reviewable instead of implied.
    // -----------------------------------------------------------------------
    bool b8RotateInHome[2][4];
    bool b8RotateOutHome[2][4];

    // -----------------------------------------------------------------------
    //  //AI(W906-FRW-NoPage) 20260926: golden RotateKit/fRotate.h:134-142 的 public 非元件成員，逐字（型別、名稱）。
    //  放在全樹共用的門面（不是 FileRW TU 的 static），因為 golden 表單外有讀者：
    //    iFromTrayAngle     ainarm9045.cpp:2712（移植樹 ainarm9045.cpp:6333 GATE W7D-K2-G06 等它）
    //    bIsAngleZero       csystem.cpp:18865（移植樹 csystem.cpp:31405 仍在 #if 0）
    //    bShowRotateBySite  main.cpp:32717、aRotateKIT_In.cpp:1310/2455/2511/2939/3032、aRotateKIT_Out.cpp:2422/2488/3055/4033
    //                       （移植樹 aRotateKIT_In.cpp:370／aRotateKIT_Out.cpp:369 的巨集仍在 #if 0）
    //    fShow              Command.cpp:7358／:9958（「有表單開著」的判斷）
    //    iRotateDutDate     aRotateDegreeClass.cpp:100（TfrmRot 點選角度，移植樹沒有 TfrmRot）
    //  寫者：FileRW/Rotate.gen.inc（golden 建構子 :42、fRotate_ReadFile :241、DoIniDataToForm :586、spbSaveClick :1036；
    //  tools/editlist/Rotate.py 以 #define 接到 FrmRotate->…）。解開上面那幾個 GATE 是 Jimmy 的底層流程，本次不動。
    //  初值：golden 靠 VCL zero-fill（golden 建構子只設 bShowRotateBySite／iRotateDutDate／iFromTrayAngle），
    //  forms/fRotate.cpp 的建構子明寫成 false／0。
    // -----------------------------------------------------------------------
    bool fShow;                                                                 // golden h:134
    bool bIsAngleZero;                                                          // golden h:136 //Eastsun 20260515 F022: D12
    int  iRotateDutDate[4][4][8];                                               // golden h:138 //Ifor 20170406 add In/Out Rotate
    int  iFromTrayAngle;                                                        // golden h:139
    bool bShowRotateBySite;                                                     // golden h:142

    // -----------------------------------------------------------------------
    //  [METHOD] golden RotateKit/fRotate.h:139 / :140.  These are FAITHFUL
    //  TRANSLATIONS of golden fRotate.cpp:1158-1168 and :1191-1201, not offline
    //  no-ops -- every statement golden executes is executed here.  They are
    //  `virtual` per facade-contract rule 1 (forms/fMain.h) so an MFC binder can
    //  still override them later, but nothing about the body is placeholder.
    // -----------------------------------------------------------------------
    virtual void InitialInRotateHome();
    virtual void InitialOutRotateHome();

    // -----------------------------------------------------------------------
    //  [METHOD] golden RotateKit/fRotate.h / fRotate.cpp:1170-1189 / :1203-1218.
    //  AI(W906-T6-MAINPROC) 20260923: 逐字翻譯，本體在 **RotateKit/fRotate.cpp（ht9045_sm）**，不在 forms/fRotate.cpp ——
    //  它們要 ProcessSingleMotorHome（acatchtray_shims.cpp，ht9045_sm）。
    //  ⚠ 刻意**不是 virtual**（偏離 facade 規則 1）：TFrmRotate 的 vtable 產生在 key function
    //    （InitialInRotateHome）所在的 forms/fRotate.cpp；若這兩支是 virtual，vtable 會引用到 sm 裡的定義，
    //    等於一條沒宣告的 forms→sm 連結邊（T5 審查 M3 指出過同一種形狀）。非虛擬成員只在呼叫點（csystem.cpp，sm）解析。
    // -----------------------------------------------------------------------
    bool DoInRotateHome();
    bool DoOutRotateHome();

    TFrmRotate();
    virtual ~TFrmRotate() {}
};

// golden: extern PACKAGE TFrmRotate *FrmRotate;   (RotateKit/fRotate.h:147)
extern TFrmRotate *FrmRotate;
extern TRotate tRotate;         // golden RotateKit/fRotate.h:157 //AI(W906-FRW-NoPage) 20260926: 定義在 aHotPlateSubstrate.cpp:1077（型別 tRotateShim＝本檔 TRotate 的 typedef）

#endif // FORMS_FROTATE_H
