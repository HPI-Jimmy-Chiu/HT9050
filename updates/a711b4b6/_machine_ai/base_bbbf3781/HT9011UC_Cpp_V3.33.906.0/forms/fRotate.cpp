// =============================================================================
//  forms/fRotate.cpp  --  definitions for the FrmRotate facade
//
//  AI(W906-W7-L2) 20260803: first home for TFrmRotate; see forms/fRotate.h for
//  the golden citations, the TFrmRotate-vs-TfRotate naming deviation and the
//  branch the offline flag values select.  Follows forms/fNote.cpp.
// =============================================================================
#include "forms/fRotate.h"

// cmydef.h supplies the two motor-id tables the translated method bodies index:
//   extern int MInRotate [MAX_ARM_Row][MAX_ARM_Col];   (cmydef.h, golden :2190)
//   extern int MOutRotate[MAX_ARM_Row][MAX_ARM_Col];   (cmydef.h, golden :2191)
// Both live in cmydef.cpp, i.e. in ht9045_globals, which ht9045_forms already
// links (CMakeLists.txt `target_link_libraries(ht9045_forms PUBLIC vclcompat
// ht9045_globals)`).  forms/fNote.cpp sets the precedent for including cmydef.h
// from this library.  No new archive dependency comes from THIS include.
#include "cmydef.h"

// -----------------------------------------------------------------------------
//  AI(W906-W7-L2) 20260803: golden uhome.h:89 `void InitProcessSingleMotorTask
//  (int Index);` -- declared locally rather than by including a header, because
//  the only ported DEFINITION is acatchtray_shims.cpp:116 (`void
//  InitProcessSingleMotorTask(int) {}`, an offline no-op) whose header
//  acatchtray_shims.h belongs to the ht9045_sm archive.  Including that header
//  here would drag ht9045_sm declarations into a bottom-layer file for no
//  reason; the one-line extern is the smaller commitment.
//
//  LINK CONSEQUENCE, REPORTED TO THE INTEGRATOR RATHER THAN PAPERED OVER:
//  this is a genuine ht9045_forms -> ht9045_sm SYMBOL reference, the direction
//  the CMakeLists.txt "NO UNDECLARED BACK-EDGE" block (:528-566, banner at :528)
//  warns about.  It does NOT need a target_link_libraries edge (which that block
//  records as non-configurable anyway) and it does not create a CMake cycle: it
//  resolves lazily inside the existing $<LINK_GROUP:RESCAN,...> group, and this
//  object file is only pulled out of libht9045_forms.a when something actually
//  references FrmRotate/TFrmRotate -- which in practice means the ckernel side,
//  and that side always links ht9045_sm.  A forms-only test that never mentions
//  FrmRotate never pulls this object and never sees the symbol.  The clean
//  fixes, both CMakeLists-only and both the integrator's call, are (a) move
//  InitProcessSingleMotorTask's definition down into ht9045_forms the way
//  Automation/AGV_predicates.cpp was moved (CMakeLists.txt:542-547 argues that
//  exact precedent and :610 is the moved source line in ht9045_forms' own list),
//  or (b) list forms/fRotate.cpp in ht9045_sm instead.
//  Deliberately NOT worked around by deleting the call: golden really does call
//  it 8 times per invocation, and a body that skipped it would be a stub.
// -----------------------------------------------------------------------------
void InitProcessSingleMotorTask(int Index);

// Golden's `__fastcall TFrmRotate::TFrmRotate(TComponent* Owner)`
// (RotateKit/fRotate.cpp:42) assigns NONE of these four members -- it relies on
// the VCL zero-fill of the instance.  They are written out explicitly here so
// the offline value is reviewable at the point of definition and so this facade
// does not depend on static-storage zero-init for a heap-allocated object.  The
// value is identical to golden's either way.  See forms/fRotate.h for which
// arm of golden ckernel.cpp:413-439 the two false flags select.
TFrmRotate::TFrmRotate() : bRotateInHome(false), bRotateOutHome(false)
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            b8RotateInHome[i][j]=false;
            b8RotateOutHome[i][j]=false;
        }
    }
}

// -----------------------------------------------------------------------------
//  FAITHFUL translation of golden RotateKit/fRotate.cpp:1158-1168
//  `void TFrmRotate::InitialInRotateHome()`.  Loop bounds, statement order and
//  the golden trailing comment are preserved verbatim; nothing is elided.
//  Golden's literal `i<2` / `j<4` are kept as literals rather than swapped for
//  MAX_ARM_Row / MAX_ARM_Col: the two happen to be equal in this tree
//  (MachineType.h:387-388) but golden wrote literals, and silently "improving"
//  them would be an unrequested behaviour change if those macros ever differ.
//  Called from golden ckernel.cpp:421.
// -----------------------------------------------------------------------------
void TFrmRotate::InitialInRotateHome()                                          //Steven 20170329 : Add individual rotate motor
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            b8RotateInHome[i][j]=false;
            InitProcessSingleMotorTask(MInRotate[i][j]);
        }
    }
}

// -----------------------------------------------------------------------------
//  FAITHFUL translation of golden RotateKit/fRotate.cpp:1191-1201
//  `void TFrmRotate::InitialOutRotateHome()`.  Same shape as the IN variant with
//  b8RotateOutHome / MOutRotate.  Golden's own line comment at fRotate.cpp:1198
//  ("Eastsun 20260518 新增修正") records that the InitProcessSingleMotorTask
//  call was ADDED to this method later than to its IN twin -- carried over so
//  the asymmetry in golden's history stays visible.
//  Called from golden ckernel.cpp:425.
// -----------------------------------------------------------------------------
void TFrmRotate::InitialOutRotateHome()                                         //Steven 20170329 : Add individual rotate motor
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            b8RotateOutHome[i][j]=false;
            InitProcessSingleMotorTask(MOutRotate[i][j]);                       //Eastsun 20260518 新增修正
        }
    }
}

// Golden creates this form UNCONDITIONALLY at startup -- HT9045.cpp:226
// `Application->CreateForm(__classid(TFrmRotate), &FrmRotate);`, in the same
// flat unguarded CreateForm list as fHome and fSetup.  Golden ckernel.cpp:419
// and :423 dereference FrmRotate with no null guard, which is only sound
// because of that.  A live instance is therefore the faithful offline state.
TFrmRotate *FrmRotate = new TFrmRotate();
