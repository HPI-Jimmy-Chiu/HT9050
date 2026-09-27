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

#endif // FORMS_FROTATE_H
