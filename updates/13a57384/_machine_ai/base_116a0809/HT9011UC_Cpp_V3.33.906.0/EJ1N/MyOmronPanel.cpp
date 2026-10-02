// =============================================================================
//  MyOmronPanel.cpp  --  TMyOmronPanel, a custom VCL panel component (see
//                         this unit's own MyOmronPanel.h banner for the full
//                         ROLE / ZERO CONSUMERS / GATE REGISTER writeup --
//                         not repeated here in full, only re-cited at each
//                         call site).
//
//  Faithful translation of golden EJ1N/MyOmronPanel.cpp (222 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 group "ej1n".  Third EJ1N/ file to land.
//
//  WAVE SCOPE -- ACTIVE vs GATED (golden line numbers; see MyOmronPanel.h
//  for the full 3-gate rationale -- restated briefly at each site below):
//   TMyOmronPanel::TMyOmronPanel   golden :10-65  -- ACTIVE: widget alloc +
//     the 8 SetXxx display calls (now state-only) + dPv=dSettingSV=0.0.
//     GATED: every Left/Top/Width/Height/Parent/Font/BorderStyle/AutoSize/
//     ParentFont/DragKind/OnMouseUp/OnMouseDown/OnMouseMove/OnClick
//     assignment and the TImage allocation (GATE 1+2).
//   TMyOmronPanel::~TMyOmronPanel  golden :67-69  -- ACTIVE, EXTENDED: golden
//     body is empty (relies on real VCL's TComponent-owns-children auto free,
//     which this port's vclcompat::TComponent does not implement -- it has
//     no owned-children list at all, see vclcompat/Comm.h). Explicitly
//     deletes GroupBox/edSV/cbEnableAT here instead of reproducing an
//     unreachable-in-this-substrate "leak nothing ever frees" -- same
//     correctness fix already applied by this wave's OWN substrate
//     (vclcompat/Controls.h's TMemo/TComboBox/TListBox dtors, each flagged
//     "AI(W906-F0fix) 20260728: LOW-6, was never freed... freed now for
//     correctness"). Zero consumers anywhere ever construct a
//     TMyOmronPanel, so this is unreachable in practice either way; recorded
//     for whichever future wave does exercise it.
//   TMyOmronPanel::GroupBox1MouseUp/MouseDown/MouseMove  golden :71-103  --
//     GATE (2): omitted entirely, see .h banner GATE (2).
//   TMyOmronPanel::SetPV           golden :105-119 -- ACTIVE: `dPv=atof(...)`.
//     GATED: the Canvas paint (GATE 1).
//   TMyOmronPanel::SetDegree/SetRunStop/SetAT(AnsiString,TColor)/
//     SetInputErr/SetEvent/SetSP  golden :121-191 -- 100% GATED (GATE 1):
//     each is PURE Canvas paint with no other state to preserve.
//   TMyOmronPanel::SetBG           golden :193-198 -- 100% GATED (GATE 1).
//   TMyOmronPanel::SetVisible      golden :200-203 -- ACTIVE, real
//     (vclcompat::TControl::Visible, inherited by TGroupBox, is real).
//   TMyOmronPanel::SetSettingSV    golden :205-209 -- ACTIVE: dSettingSV
//     assignment + edSV->Text write (vclcompat::TCustomEdit::Text is real).
//   TMyOmronPanel::SetSV           golden :211-215 -- ACTIVE: dSV assignment
//     + the SetSP(...) call (itself 100% gated, see above).
//   TMyOmronPanel::setEditValueClick golden :217-220 -- GATE (3): fQwertyKey
//     real since FW-QWKEY1; GATE (3) OPENED 20260824 (see .h banner GATE 3).
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped (golden :2 and :8). `__fastcall` dropped from every definition.
//  `myQwertyKeyBoard.h` (golden :5) not included -- its only referenced
//  symbol, fQwertyKey, now comes from forms/fQwertyKey.h (GATE 3 OPENED 20260824); `cmydef.h` (golden :6) IS included,
//  for the real N_INTEGER constant cited (unreachable) inside that same gate.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "MyOmronPanel.h"
//   #include "myQwertyKeyBoard.h"   -- ported as forms/fQwertyKey.h, included 20260824
//   #include "cmydef.h" ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella)
#include "EJ1N/MyOmronPanel.h"      // this unit's own contract
// AI(W906-FW-SIG-W17) 20260826: 三支 GroupBox1Mouse* 回填 golden 完整簽章。
#include "vclcompat/ShiftState.h"
#include "cmydef.h"                 // N_INTEGER (real) -- cited inside GATE (3) only
#include "forms/fQwertyKey.h"  // AI(W906-FW-QWKEY2) 20260824: fQwertyKey extern for un-gated ShowQwertyKey sites (real since FW-QWKEY1 fc08e09; latent until HTEdit GATE (6) wiring)
//---------------------------------------------------------------------------
TMyOmronPanel::TMyOmronPanel(TComponent* Owner) : TComponent(Owner)
{
    GroupBox        =new TGroupBox();
    edSV            =new TEdit();                                           //設定值
    cbEnableAT      =new TCheckBox();

    //AI(W906-PT-W2) 20260807: GATE (1)+(2) -- golden's TImage allocation,
    //  every widget geometry/style/Font/BorderStyle/AutoSize/ParentFont/
    //  DragKind property, and the 3 mouse-event + 1 click-event wirings
    //  (golden :14,17-30,32,42-55,57-63). None of Left/Top/Width/Height/
    //  Parent/Font/BorderStyle/AutoSize/ParentFont/DragKind exist on
    //  vclcompat::TControl by design (see MyOmronPanel.h banner GATE (1));
    //  GroupBox1MouseUp/Down/Move (GATE (2)) and setEditValueClick's OnClick
    //  wiring have no callee signature this tree can express / no callee
    //  substrate (GATE (3)) respectively. Preserved verbatim:
#if 0
    ImgOmronPanel   =new TImage(this);

    GroupBox->Left      =0;
    GroupBox->Top       =0;
    GroupBox->Width     =206;
    GroupBox->Height    =120;
    GroupBox->DragKind  =dkDock;
    GroupBox->OnMouseUp =GroupBox1MouseUp;
    GroupBox->OnMouseDown=GroupBox1MouseDown;
    GroupBox->OnMouseMove=GroupBox1MouseMove;

    ImgOmronPanel->Parent=GroupBox;
    ImgOmronPanel->Top=20;
    ImgOmronPanel->Left=4;
    ImgOmronPanel->Width=197;
    ImgOmronPanel->Height=95;

    ImgOmronPanel->Canvas->Font->Name ="Arial";
#endif
    SetBG        (clGray, clGray);                                              //設定背景
    SetPV        ("##.##",       clYellow);                                     //顯示量測值
    SetDegree    ("℃",          clYellow);                                     //顯示量測單位
    SetRunStop   ("STOP",        clBlack);                                      //顯示目前運作模式
    SetAT        ("AT",          clBlack);                                      //顯示AT是否啟動
    SetInputErr  ("Input Error", clBlack);                                      //感溫線異常
    SetEvent     ("Event",       clBlack);                                      //警報事件
    SetSP        ("SP",          TColor(0x004080FF));                           //目前設定值

#if 0
    edSV->Parent=GroupBox;
    edSV->AutoSize=false;
    edSV->Left = 72;
    edSV->Top = 85;
    edSV->Width = 128;
    edSV->Height = 28;
    edSV->BorderStyle=bsNone;
#endif
    edSV->Text = "0";
#if 0
    edSV->Font->Charset = DEFAULT_CHARSET;
    edSV->Font->Color = clBlack;
    edSV->Font->Size = 18;
    edSV->Font->Name = "Arial";
    edSV->ParentFont = false;
    edSV->OnClick=setEditValueClick;
#endif

#if 0
    cbEnableAT->Parent  =GroupBox;
    cbEnableAT->Left    =48;
    cbEnableAT->Top     =76;
    cbEnableAT->Width   =17;
    cbEnableAT->Height  =17;
    cbEnableAT->Color   =clGray;
#endif
    cbEnableAT->Caption ="";
    dPv=dSettingSV=0.0;
}
//---------------------------------------------------------------------------
TMyOmronPanel::~TMyOmronPanel()
{
    //AI(W906-PT-W2) 20260807: golden body is empty (golden :67-69), relying
    //  on real VCL's TComponent-owns-children auto-free; this port's
    //  vclcompat::TComponent has no owned-children list to reproduce that
    //  with (see MyOmronPanel.h banner, ~TMyOmronPanel entry). Explicitly
    //  freeing the 3 real substrate widgets here instead of leaving them
    //  permanently unreachable -- same correctness-fix class already applied
    //  by vclcompat/Controls.h's own TMemo/TComboBox/TListBox dtors.
    delete GroupBox;
    delete edSV;
    delete cbEnableAT;
}
//---------------------------------------------------------------------------
//AI(W906-PT-W2) 20260807: GATE (2) -- golden GroupBox1MouseUp/
//  GroupBox1MouseMove/GroupBox1MouseDown (golden :71-103) omitted entirely:
//  their `TMouseButton`/`TShiftState` parameter types have no port anywhere
//  in this tree. See MyOmronPanel.h banner GATE (2) for the full citation.
//  Golden bodies kept here verbatim, in comment form, for provenance:
//
//  void __fastcall TMyOmronPanel::GroupBox1MouseUp(TObject *Sender,
//        TMouseButton Button, TShiftState Shift, int X, int Y)
//  {
//      TGroupBox *Ptr;
//      Ptr=(TGroupBox *)Sender;
//      if(Ptr->Tag!=0)
//      {
//          Ptr->Tag=0;
//      }
//  }
//  void __fastcall TMyOmronPanel::GroupBox1MouseMove(TObject *Sender,
//        TShiftState Shift, int X, int Y)
//  {
//      TGroupBox *Ptr;
//      Ptr=(TGroupBox *)Sender;
//      if(Ptr->Tag==1)
//      {
//          Ptr->Left=Ptr->Left+(X-iStartX);
//          Ptr->Top =Ptr->Top+(Y-iStartY);
//      }
//  }
//  void __fastcall TMyOmronPanel::GroupBox1MouseDown(TObject *Sender,
//        TMouseButton Button, TShiftState Shift, int X, int Y)
//  {
//      TGroupBox *Ptr;
//      Ptr=(TGroupBox *)Sender;
//      if(Ptr->Tag!=1)
//          Ptr->Tag=1;
//      iStartX=X;
//      iStartY=Y;
//  }
//---------------------------------------------------------------------------
void TMyOmronPanel::SetPV(AnsiString PV, TColor Color)               //顯示量測值
{
    //AI(W906-PT-W2) 20260807: GATE (1) -- golden's Canvas paint (golden
    //  :107-117, drawing PV onto ImgOmronPanel) has no substrate (no
    //  TImage/TCanvas type in this port). The one state-carrying side
    //  effect, `dPv=atof(PV.c_str())`, is preserved ACTIVE below. Verbatim:
#if 0
    int iTextWidth, iTextHeight;
    //溫度--------------
    ImgOmronPanel->Canvas->Font->Color=Color;
    ImgOmronPanel->Canvas->Font->Size=36;
    iTextHeight=ImgOmronPanel->Canvas->TextHeight(PV);

    iTextWidth=120;                                                             //Ifor 20241129 add: 避免文字大小不同殘影問題
    ImgOmronPanel->Canvas->TextOutA(((129-iTextWidth)/2)+72, (44-iTextHeight)/2, "    ");
    iTextWidth=ImgOmronPanel->Canvas->TextWidth(PV);

    ImgOmronPanel->Canvas->TextOutA(((129-iTextWidth)/2)+72, (44-iTextHeight)/2, PV);
#endif
    dPv=atof(PV.c_str());                                                       //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetDegree(AnsiString Degree, TColor Color)       //顯示量測單位
{
    //AI(W906-PT-W2) 20260807: GATE (1) -- 100% Canvas paint, no other state.
#if 0
    int iTextWidth, iTextHeight;
    //單位---------------
    ImgOmronPanel->Canvas->Font->Color=Color;
    ImgOmronPanel->Canvas->Font->Size=10;
    iTextWidth=ImgOmronPanel->Canvas->TextWidth(Degree);
    iTextHeight=ImgOmronPanel->Canvas->TextHeight(Degree);
    ImgOmronPanel->Canvas->TextOutA(((69-iTextWidth)/2), (19-iTextHeight)/2, Degree);
#endif
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetRunStop(AnsiString RunStop, TColor Color)     //顯示目前運作模式
{
    //AI(W906-PT-W2) 20260807: GATE (1) -- 100% Canvas paint, no other state.
#if 0
    int iTextWidth, iTextHeight;
    //Run / Stop---------------
    ImgOmronPanel->Canvas->Font->Color=Color;
    ImgOmronPanel->Canvas->Font->Size=10;
    iTextWidth=ImgOmronPanel->Canvas->TextWidth(RunStop);
    iTextHeight=ImgOmronPanel->Canvas->TextHeight(RunStop);
    ImgOmronPanel->Canvas->TextOutA(((69-iTextWidth)/2), (19-iTextHeight)/2+20, RunStop);
#endif
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetAT(AnsiString AT, TColor Color)               //顯示AT是否啟動
{
    //AI(W906-PT-W2) 20260807: GATE (1) -- 100% Canvas paint, no other state.
#if 0
    int iTextWidth, iTextHeight;
    //AT---------------
    ImgOmronPanel->Canvas->Font->Color=Color;
    ImgOmronPanel->Canvas->Font->Size=10;
    iTextWidth=ImgOmronPanel->Canvas->TextWidth(AT);
    iTextHeight=ImgOmronPanel->Canvas->TextHeight(AT);
    ImgOmronPanel->Canvas->TextOutA(((69-iTextWidth)/2), (19-iTextHeight)/2+58, AT);
#endif
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetInputErr(AnsiString InputErr, TColor Color)   //感溫線異常
{
    //AI(W906-PT-W2) 20260807: GATE (1) -- 100% Canvas paint, no other state.
#if 0
    int iTextWidth, iTextHeight;
    //Input Error---------------
    ImgOmronPanel->Canvas->Font->Color=Color;
    ImgOmronPanel->Canvas->Font->Size=10;
    iTextWidth=ImgOmronPanel->Canvas->TextWidth(InputErr);
    iTextHeight=ImgOmronPanel->Canvas->TextHeight(InputErr);
    ImgOmronPanel->Canvas->TextOutA(((69-iTextWidth)/2), (19-iTextHeight)/2+39, InputErr);
#endif
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetEvent(AnsiString Event, TColor Color)         //警報事件
{
    //AI(W906-PT-W2) 20260807: GATE (1) -- 100% Canvas paint, no other state.
#if 0
    int iTextWidth, iTextHeight;
    //事件---------------
    ImgOmronPanel->Canvas->Font->Color=Color;
    ImgOmronPanel->Canvas->Font->Size=10;
    if(Event=="Event")                                                          //Ifor 20241129 add: 避免文字大小不同殘影問題
    {
        iTextWidth=80;
        iTextHeight=16;
        ImgOmronPanel->Canvas->TextOutA(((129-iTextWidth)/2)+72, (21-iTextHeight)/2+44, "                      ");
    }
    iTextWidth=ImgOmronPanel->Canvas->TextWidth(Event);
    iTextHeight=ImgOmronPanel->Canvas->TextHeight(Event);
    ImgOmronPanel->Canvas->TextOutA(((129-iTextWidth)/2)+72, (21-iTextHeight)/2+44, Event);
#endif
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetSP(AnsiString SP, TColor Color)               //目前設定值
{
    //AI(W906-PT-W2) 20260807: GATE (1) -- 100% Canvas paint, no other state.
#if 0
    int iTextWidth, iTextHeight;
    //SP---------------
    ImgOmronPanel->Canvas->Font->Color=Color;
    ImgOmronPanel->Canvas->Font->Size=10;
    iTextWidth=ImgOmronPanel->Canvas->TextWidth(SP);
    iTextHeight=ImgOmronPanel->Canvas->TextHeight(SP);
    ImgOmronPanel->Canvas->TextOutA(((69-iTextWidth)/2), (19-iTextHeight)/2+77, SP);
#endif
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetBG(TColor OutLine, TColor BackGround)         //設定背景
{
    //AI(W906-PT-W2) 20260807: GATE (1) -- 100% Canvas paint, no other state.
#if 0
    ImgOmronPanel->Canvas->Brush->Color=BackGround;
    ImgOmronPanel->Canvas->Pen->Color  =OutLine;
    ImgOmronPanel->Canvas->Rectangle(TRect(0, 0,ImgOmronPanel->Width, ImgOmronPanel->Height));
#endif
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetVisible(bool bSW)                                        //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
{
    GroupBox->Visible=bSW;
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetSettingSV(double dsv)                                    //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
{
    dSettingSV=dsv;
    edSV->Text=AnsiString(dSettingSV);
}
//---------------------------------------------------------------------------
void TMyOmronPanel::SetSV(double dsv)                                           //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
{
    dSV=dsv;
    SetSP(AnsiString().sprintf("%03.1f", dSV), TColor(0x004080FF));
}
//---------------------------------------------------------------------------
void TMyOmronPanel::setEditValueClick(TObject *Sender)
{
    //AI(W906-PT-W2) 20260807: GATE (3) -- golden `fQwertyKey->ShowQwertyKey(
    //  (TEdit*)Sender, N_INTEGER, 0, true, 0, 180);` (golden :219). fQwertyKey
    //  real since FW-QWKEY1 (fc08e09) -- GATE (3) OPENED 20260824 (FW-QWKEY2);
    //  call live below, headless no-op until HTEdit GATE (6) wires the
    //  runtime keyboard instance (see MyOmronPanel.h banner GATE (3)).
    fQwertyKey->ShowQwertyKey((TEdit*)Sender, N_INTEGER, 0, true, 0, 180);      //小鍵盤
}
//---------------------------------------------------------------------------

// =============================================================================
//  APPEND BLOCK -- BANNER EXTENSION
//  AI(W906-PT-W8) 20260811 -- the 3 golden mouse handlers this file's original
//  banner listed as "GATE (2): omitted entirely" are landed here as real,
//  compilable, callable bodies.  Nothing above this line is edited or
//  reordered (append-only).
//
//  WHY THE ORIGINAL OMISSION IS SUPERSEDED: identical argument to the one
//  MyTempPanel.cpp's own PT-W8 append block sets out.  `TMouseButton`/
//  `TShiftState` genuinely have no port (re-verified, GATE (W8-1) below), but
//  NEITHER PARAMETER IS EVER READ by any of these three golden bodies
//  (golden EJ1N/MyOmronPanel.cpp:71-103, read line by line) -- so dropping
//  them costs nothing and buys three real bodies.  `X`/`Y` are NOT dead here:
//  GroupBox1MouseMove and GroupBox1MouseDown both read them, and MouseDown's
//  reads land in REAL member state (see below).
//
//  SHAPE: file-scope free functions taking `TMyOmronPanel *Self`, not members,
//  because PT-W2 removed the three declarations from EJ1N/MyOmronPanel.h and
//  that header is outside this wave's write boundary.  See the HAND-OFF block
//  at the end of this file.  Until the loop acts on it, these three have
//  EXTERNAL LINKAGE AND NO CALLER anywhere in the tree.
//  ⚠ AI(W906-FW-SIG-W17) 20260826: 上面這段**已經過期**——本波執行了那個交接，
//  三支現在是 TMyOmronPanel 的真正成員，且簽章回填成 golden 原文
//  （含 TMouseButton/TShiftState）。原文保留是為了記錄「當時為什麼那樣做」。
//
//  WAVE SCOPE -- ONE LINE PER GOLDEN FUNCTION (all golden EJ1N/MyOmronPanel.cpp)
//   GroupBox1MouseUp    golden :71-80   -- GATED (body 100%, `Ptr->Tag` only).
//   GroupBox1MouseMove  golden :82-92   -- GATED (body 100%: Tag + Left + Top).
//   GroupBox1MouseDown  golden :94-103  -- PARTLY ACTIVE: the `Ptr->Tag` half
//                                          is gated, `iStartX=X; iStartY=Y;`
//                                          is REAL and kept.
//
//  GATE REGISTER -- 2 gates.
//   (W8-1) `TMouseButton Button` / `TShiftState Shift` (golden :72, :83, :95)
//       -- DROPPED FROM THE PORT SIGNATURE, not gated.
//       ABSENCE CLAIM: no type of either name exists anywhere in this port.
//         cmd:  grep -rn --include=*.h --include=*.cpp -E
//               '^[[:space:]]*(class|struct|enum|typedef|using)[^;]*
//               \b(TMouseButton|TShiftState)\b' .   (tree root, minus ./tools/)
//         run:  2026-08-11 11:22:14  -> 0 hits (exit 1)
//         re-run at end of wave: see RE-RUN LOG at the end of this file.
//       WHY FAITHFUL: both are DEAD in golden.  `Button` and `Shift` occur
//       only in the parameter lists of golden :71-103, never in a body.
//       NOTE, preserved: golden's MouseMove signature genuinely has NO
//       `TMouseButton Button` at all (golden :82-83) -- only Shift/X/Y.  That
//       asymmetry is real VCL (TMouseMoveEvent vs TMouseEvent) and is recorded
//       here rather than smoothed over, since the three ported signatures come
//       out identical.
//       REAL-MACHINE DIFFERENCE: none.
//
//   (W8-6) `Ptr->Tag`, `Ptr->Left`, `Ptr->Top` on the TGroupBox (golden
//       :76-79, :87-91, :99-100).  vclcompat::TGroupBox carries only
//       Caption + TControl's Visible/Enabled/hCtl -- no Tag, no geometry.
//         cmd:  grep -rn 'Tag' vclcompat/Controls.h
//         run:  2026-08-11 11:23:06  -> 0 hits
//         (geometry: vclcompat/Controls.h's own MEASURED PROPERTY COVERAGE
//          note lists Left/Top/Width/Height on the documented NOT-covered
//          side; EJ1N/MyOmronPanel.h's GATE (1) already cites this and it is
//          why this file's own ctor gates every geometry write.)
//       WHY FAITHFUL: these three golden bodies implement ONE feature --
//       click-and-drag repositioning of the panel frame.  `Tag` is the
//       drag-in-progress latch, `Left`/`Top` are the thing being dragged.
//       With no window there is no pointer, no drag gesture and no on-screen
//       position, so the faithful offline value of the latch is "no drag in
//       progress" -- exactly what an ungated-but-substrate-less body would
//       compute anyway.  This is the "user did nothing" default.
//       WHAT IS NOT GATED, AND WHY THAT MATTERS: `iStartX`/`iStartY` are REAL
//       (public ints, EJ1N/MyOmronPanel.h:218-219), so MouseDown's
//       `iStartX=X; iStartY=Y;` is kept ACTIVE.  It is the one piece of
//       genuine, observable state in the three bodies, and a future wave that
//       lands Tag/geometry needs it to already be recording correctly.
//       GOLDEN BUG NOTED, NOT FIXED: neither golden's ctor (golden :10-65) nor
//       this port's ctor initialises iStartX/iStartY, so golden's MouseMove
//       reads them uninitialised if a MouseMove ever arrives before the first
//       MouseDown.  Behaviour preserved -- no initialiser is added here.
//       REAL-MACHINE DIFFERENCE: once fDTME08.cpp / OmronEJ1N.cpp are ported
//       (both are live consumers -- see this file's header banner, which
//       corrects PT-W2's original false "ZERO CONSUMERS" claim), the
//       temperature-channel panel will not be draggable.
//
//  Big5: no Chinese text occurs in golden :71-103.  ZERO U+FFFD.
// =============================================================================
//---------------------------------------------------------------------------
//  golden EJ1N/MyOmronPanel.cpp:71-80  --  TMyOmronPanel::GroupBox1MouseUp
//  Ported signature drops `TMouseButton Button, TShiftState Shift` (GATE W8-1)
//  and takes `TMyOmronPanel *Self` instead of `this` (see BANNER EXTENSION).
//AI(W906-FW-SIG-W17) 20260826: 收回成員並回填 golden 完整簽章。
void TMyOmronPanel::GroupBox1MouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W17): golden 這兩支也沒讀這兩個
    TGroupBox *Ptr;
    Ptr=(TGroupBox *)Sender;

    // AI(W906-FW-TAG1) 20260825: GATE (W8-6) OPENED here. Tag is real since
    //   this wave (vclcompat::TControl) and this latch is SELF-PROVENANCED:
    //   the only writer is GroupBox1MouseDown below, also opened, so the pair
    //   needs no .dfm value. Offline the latch is simply never set, which is
    //   the faithful "no drag in progress" state golden starts from.
    if(Ptr->Tag!=0)
    {
        Ptr->Tag=0;
    }
}
//---------------------------------------------------------------------------
//  golden EJ1N/MyOmronPanel.cpp:82-92  --  TMyOmronPanel::GroupBox1MouseMove
//  golden's own signature carries NO `TMouseButton Button` here -- only
//  `TShiftState Shift, int X, int Y` (golden :83).  See GATE (W8-1).
//AI(W906-FW-SIG-W17) 20260826: 收回成員並回填 golden 完整簽章。
void TMyOmronPanel::GroupBox1MouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    (void)Shift;   //AI(W906-FW-SIG-W17): golden 的 MouseMove **沒有** TMouseButton 參數
                   // （VCL 的 OnMouseMove 本來就沒有），簽章照 golden :17 逐字。
    TGroupBox *Ptr;
    Ptr=(TGroupBox *)Sender;

    //AI(W906-PT-W8) 20260811: GATE (W8-6) -- golden moves the frame by the
    //  pointer delta since MouseDown.  All three properties it needs
    //  (`Tag` latch, `Left`, `Top`) had no substrate; `iStartX/iStartY`
    //  ARE real but are only READ here, so with the surrounding `if` gated
    //  there is no active remainder.  ACTIVE arm: no-op ("no drag in
    //  progress" -- the faithful offline value of the latch).
    //  AI(W906-FW-TAG1) 20260825: PARTIALLY EXPIRED -- `Tag` is real now (the
    //  latch pair MouseUp/MouseDown opened this wave), but `Left`/`Top` still
    //  have no port on vclcompat::TControl and `iStartX`/`iStartY` are
    //  unqualified here inside a free function, so this one STAYS SHUT.
    //  Golden preserved VERBATIM:
#if 0
    if(Ptr->Tag==1)
    {
        Ptr->Left=Ptr->Left+(X-iStartX);
        Ptr->Top =Ptr->Top+(Y-iStartY);
    }
#endif
}
//---------------------------------------------------------------------------
//  golden EJ1N/MyOmronPanel.cpp:94-103  --  TMyOmronPanel::GroupBox1MouseDown
//AI(W906-FW-SIG-W17) 20260826: 收回成員並回填 golden 完整簽章。
void TMyOmronPanel::GroupBox1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W17): golden 這兩支也沒讀這兩個
    TGroupBox *Ptr;
    Ptr=(TGroupBox *)Sender;

    // AI(W906-FW-TAG1) 20260825: GATE (W8-6) OPENED here -- the latch SET half.
    //   Tag is real since this wave; the matching clear is in GroupBox1MouseUp
    //   above. iStartX/iStartY below were already ACTIVE and stay so.
    if(Ptr->Tag!=1)
        Ptr->Tag=1;
    iStartX=X;
    iStartY=Y;
    (void)Ptr;
}
//---------------------------------------------------------------------------
//  HAND-OFF TO THE INTEGRATING LOOP -- ✅ 已於 FW-SIG-W17（20260826）執行
//  （下面原文保留。實際做法比它建議的更忠實：簽章回填成 golden 完整版，
//   而不是這裡寫的簡化版，因為 vclcompat/ShiftState.h（commit f184093）
//   已經讓 TMouseButton/TShiftState 拼得出來。）
//  HAND-OFF TO THE INTEGRATING LOOP -- DESCRIBED, DELIBERATELY NOT DONE HERE
//  (EJ1N/MyOmronPanel.h is outside this wave's write boundary.)
//
//  To collapse the three free functions above back into class members, add to
//  EJ1N/MyOmronPanel.h's `private:` block, replacing the 3 commented-out
//  golden signatures PT-W2 left at EJ1N/MyOmronPanel.h:193-195:
//
//      void GroupBox1MouseUp  (TObject *Sender, int X, int Y);
//      void GroupBox1MouseDown(TObject *Sender, int X, int Y);
//      void GroupBox1MouseMove(TObject *Sender, int X, int Y);
//
//  then rename each definition above to `TMyOmronPanel::<name>`, delete its
//  `TMyOmronPanel *Self` parameter and the matching `(void)Self;`, and change
//  MouseDown's two active lines from `iStartX=X;` / `iStartY=Y;`
//  back to golden's `iStartX=X;` / `iStartY=Y;`.
//
//  RE-RUN LOG -- every absence claim in the GATE REGISTER above was re-run
//  from the tree root at 2026-08-11 11:45, AFTER this block was written and
//  after all sibling files in this wave had landed, with the same results.
//---------------------------------------------------------------------------
