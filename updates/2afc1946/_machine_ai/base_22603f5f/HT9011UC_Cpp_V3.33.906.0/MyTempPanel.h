// =============================================================================
//  MyTempPanel.h  --  TMyTempPanel, one row of the Temperature Set dialog's
//                      channel table (name label + up to 19 numeric-entry
//                      TEdit "offset/limit" fields for one temperature
//                      channel: Low/Mid/Lowbase/Base/HighBase/SHighBase +
//                      their Kit_ counterparts + PreOffset/PreOfsTime/
//                      AfterOfs + Offset/SingleLimit/IndiTemp/InitTempOffset/
//                      EOTTempOffset, plus a hairline separator TPanel).
//
//  Faithful translation of golden MyTempPanel.h (69 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 group "panels".  Third file to land.
//
//  ZERO CONSUMERS: grepped the ENTIRE golden 906 tree for `MyTempPanel`/
//  `TMyTempPanel` outside its own .h/.cpp -- the only real consumer is golden
//  uTemp_Set.cpp/uTemp_Set.h (TfTemp_Set, listNormal/listArm1/listArm2
//  population), which this port has NOT translated (its own narrow TU-local
//  stand-in, `W5FA_TfTemp_SetExt`/`fTemp_Set`, is a different, much smaller
//  surface -- see Automation/auto9045.cpp and uHeaterThread.cpp's own GATE
//  REGISTER citations of it). In THIS port tree, grepped clean (zero hits
//  anywhere, not even a comment) -- confirmed before writing a single line.
//
//  ROLE: a plain (non-TComponent, non-TControl) value class -- matches
//  golden's own shape exactly (golden declares NO base class at all). Owns a
//  TPanel "row" (palTemp) with a TLabel name and up to 19 TEdit fields as
//  public members, laid out by the ctor purely for a real VCL form's visual
//  grid. `Caption` is golden's one `__property` (read=GetCaption,
//  write=SetCaption, both PRIVATE methods in golden -- see below).
//
//  WAVE SCOPE -- ACTIVE vs GATED (golden line numbers, all in the .cpp):
//   ACTIVE: ctor's widget allocation + the handful of REAL property writes
//     (golden :20-349, see .cpp for the precise line-by-line split),
//     ~TMyTempPanel (:351-386, fully -- every `delete` is real), SetParent
//     (:388-392, Parent/Align gated, body otherwise empty), SetEnable
//     (:394-415, fully -- every ->Enabled write is real), SetCaption/
//     GetCaption/SetIndexTag (:601-614/606-609, fully).
//   GATED, WHOLE METHOD OMITTED (golden :417-599): edBaseMouseDown/
//     edLimitMouseDown/edIndiviMouseDown/edinitialMouseDown -- see GATE (1).
//
//  GATE REGISTER -- 1 gate, grounded in the SAME tree-wide grep
//  EJ1N/MyOmronPanel.h's GATE (2) and MyVacuumPanel.h's GATE (2) already
//  performed and cite:
//   (1) `edBaseMouseDown`/`edLimitMouseDown`/`edIndiviMouseDown`/
//       `edinitialMouseDown` (golden :17-20, `void __fastcall Xxx(TObject
//       *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)`) --
//       their `TMouseButton`/`TShiftState` parameter types do not exist
//       ANYWHERE in this port (grepped tree-wide, same finding EJ1N/
//       MyOmronPanel.h's GATE (2) already recorded). The SIGNATURE itself
//       cannot be spelled, so all 4 declarations are OMITTED from the class
//       entirely (matching that exact precedent), together with the ctor's
//       `->OnMouseDown=edBaseMouseDown;` (etc) wiring, which has no substrate
//       either (vclcompat::TCustomEdit carries no OnMouseDown delegate slot).
//       Every one of these 4 handlers exists SOLELY to compute bounds for a
//       `fQwertyKey->ShowQwertyKey(...)` popup (fQwertyKey real since
//       FW-QWKEY1, 20260824; formerly cited by EJ1N/MyOmronPanel.h and
//       MyVacuumPanel.h GATE (3) for the identical golden idiom) -- so even
//       had their signature been spellable, their body would still have
//       reduced to a no-op. BEHAVIOUR DELTA: none observable (zero
//       consumers, see above); on a real machine these 19 TEdit fields
//       cannot be tapped to pop the numeric keypad in this build.
//
//  `Caption` (golden :57, `__property AnsiString Caption={read=GetCaption,
//  write=SetCaption}`): UNLIKE MyVacuumPanel.h's 18 transparent int
//  properties (read/write name the SAME backing field), this one wraps two
//  DISTINCT, already-real methods (`AnsiString GetCaption()` / `void
//  SetCaption(AnsiString)`, both golden-PRIVATE). Since C++ has no property
//  syntax, the property's only useful C++ shape is the plain method PAIR
//  itself -- and since the property golden exposes them THROUGH is public,
//  SetCaption/GetCaption are declared PUBLIC here (golden's private/public
//  split does not survive the __property->plain-methods collapse; matches
//  this tree's established handling of the same shape elsewhere, e.g.
//  MyOmronPanel.h's public GetSettingSV/GetSV/GetPV accessor methods for
//  golden properties of the identical read=Getter/write=Setter shape).
//
//  VCL/Borland conversions: `__fastcall` dropped from every method. `TList`
//  (golden :62-64, `extern TList *listNormal;` etc) is NEVER dereferenced
//  anywhere in this unit's .cpp (grepped) -- forward-declared only, avoiding
//  any dependency on which of this tree's several differently-shaped `TList`
//  shims (vclcompat/TList.h vs Public/HTEditList.h's HTEDITLIST_TLIST_SHIM vs
//  aHotPlateSubstrate.h's HT9045_TLIST_SHIM -- three incompatible
//  definitions exist tree-wide today, see Public/HTEditList.h's own
//  INTEGRATION HAZARDS banner) this TU would otherwise have to pick.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL header (mirrored as a comment for provenance):
//   //---------------------------------------------------------------------------
//   #ifndef MyTempPanelH / #define MyTempPanelH
//   #include <vcl.h> / <SysUtils.hpp> / <Classes.hpp> / <Controls.hpp> /
//     <StdCtrls.hpp>
// =============================================================================

//---------------------------------------------------------------------------
#ifndef MyTempPanelH
#define MyTempPanelH

#include "vclcompat/vcl_compat.h"
#include "vclcompat/Controls.h"
#include "vclcompat/ShiftState.h"   // AI(W906-FW-SIG-W16) 20260826

//---------------------------------------------------------------------------
class TMyTempPanel
{
    private:    // User declarations
        //AI(W906-PT-W3) 20260807: GATE (1) -- golden `edBaseMouseDown`/
        //  `edLimitMouseDown`/`edIndiviMouseDown`/`edinitialMouseDown`
        //  (golden .h :17-20) omitted: their `TMouseButton`/`TShiftState`
        //  parameter types have no port anywhere in this tree (see this
        //  header's own banner GATE (1)). Golden signatures kept here
        //  verbatim, in comment form, for provenance:
        //AI(W906-FW-SIG-W16) 20260826: GATE (1) 的前提已經不成立——
        //  `TMouseButton`/`TShiftState` 現在有 port（vclcompat/ShiftState.h，
        //  commit f184093，量測先行：golden 全樹 358 支帶該參數、只有 1 支真的讀它）。
        //  四支因此改回**真正的宣告**，簽章與 golden 逐字相同；本體早在
        //  PT-W8（20260811）就翻好了，只是當時 header 在該波寫入邊界外，
        //  只能落成檔案層自由函式並在 .cpp 檔尾留下交接說明。本波執行那個交接。
        void edBaseMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
        void edLimitMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
        void edIndiviMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
        void edinitialMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);   //kevin 20210421 initial temp 獨立範圍

    public:
        //AI(W906-PT-W3) 20260807: golden declares SetCaption/GetCaption
        //  PRIVATE (golden .h :16,21) but exposes them PUBLICLY through
        //  `__property Caption` (golden .h :57) -- moved public here, see
        //  this header's own banner note on the `Caption` property.
        void SetCaption(AnsiString Name);
        AnsiString GetCaption();

        TMyTempPanel(AnsiString Alias, int iTag);
        ~TMyTempPanel();
        void SetParent(TTabSheet *tsTemp);
        void SetIndexTag(int iTag);
        void SetEnable(bool bflag);

        int iIndexTag;
        TPanel    *palTemp;
        TLabel    *labName;
        TEdit     *edLow;            //設定值
        TEdit     *edMid;            //設定值
        TEdit     *edLowbase;        //設定值
        TEdit     *edBase;           //設定值
        TEdit     *edHighBase;       //設定值
        TEdit     *edSHighBase;                                                 //Ztex 2024.07.27 Add 6 Point Temperature Offset
        //==> Eastsun 20260526 #026-4.A7 Ifor 20241101 add :KYEC
        TEdit     *edPreOffset;      //預先補償
        TEdit     *edPreOfsTime;     //補償秒數
        TEdit     *edAfterOfs;       //補償秒數結束後的補償值
        //<== Eastsun 20260526 #026-4.A7

        TEdit     *edKit_Low;            //設定值
        TEdit     *edKit_Mid;            //設定值
        TEdit     *edKit_Lowbase;        //設定值
        TEdit     *edKit_Base;           //設定值
        TEdit     *edKit_HighBase;       //設定值

        TEdit     *edOffset;         //設定值
        TEdit     *edSingleLimit;    //設定值
        TEdit     *edIndiTemp;       //設定值
        TEdit     *edInitTempOffset; //設定值
        TEdit     *edEOTTempOffset;  //設定值
        TPanel    *palLine;

        //AI(W906-PT-W3) 20260807: golden `__property AnsiString Caption=
        //  {read=GetCaption, write=SetCaption};` (golden .h :57) -- no C++
        //  property syntax; SetCaption/GetCaption above ARE the collapsed
        //  form (see this header's own banner note).
        int        iOffsetByRecipeMaxLimit;
        int        iOffsetByRecipeMinLimit;
};

// golden `extern TList *listNormal;` etc (golden .h :62-64) -- forward
// declaration only, never dereferenced in this unit's .cpp; see this
// header's own banner note on why the full type is deliberately not pulled in.
class TList;
extern TList *listNormal;
extern TList *listArm1;
extern TList *listArm2;
//extern TList *listDut;
//extern TList *listNoUse;
//extern TList *listHeater;

#endif
