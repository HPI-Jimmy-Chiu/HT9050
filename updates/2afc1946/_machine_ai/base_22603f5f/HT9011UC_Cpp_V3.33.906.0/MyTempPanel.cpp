// =============================================================================
//  MyTempPanel.cpp  --  TMyTempPanel definitions (see MyTempPanel.h for the
//                        full ROLE / ZERO CONSUMERS / GATE REGISTER writeup --
//                        re-cited briefly at each call site here).
//
//  Faithful translation of golden MyTempPanel.cpp (615 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W3) 20260807
//  Translation wave: W906-PT-W3 group "panels".  Third file to land.
//
//  golden_fns = 11: ctor(1) dtor(2) SetParent(3) SetEnable(4)
//  edBaseMouseDown(5) edLimitMouseDown(6) edIndiviMouseDown(7)
//  edinitialMouseDown(8) SetCaption(9) SetIndexTag(10) GetCaption(11).
//  ported_fns = 7: ctor, dtor, SetParent, SetEnable, SetCaption, SetIndexTag,
//  GetCaption all get real bodies below. The 4 mouse-down handlers (5-8) are
//  OMITTED per MyTempPanel.h's GATE (1) -- their
//  `TMouseButton`/`TShiftState` parameter types cannot be spelled anywhere in
//  this port (the identical class of omission EJ1N/MyOmronPanel.cpp and
//  MyVacuumPanel.cpp already apply to their own mouse/click handlers of the
//  same golden shape).
//  AI(W906-FW-TAG1) 20260825 -- THE PARAGRAPH ABOVE IS STALE, kept for
//  provenance: PT-W8 landed all four on 20260811 as file-scope free functions
//  with trimmed signatures, so ported_fns is 11/11, not 7/11.
//  AI(W906-FW-SIG-W16) 20260826 -- 上面那段**也已經過期**了：本波把那四支
//  收回成 TMyTempPanel 的成員（TMyTempPanel::edBaseMouseDown 等），
//  並把簽章回填成 golden 原文（含 TMouseButton/TShiftState，靠
//  vclcompat/ShiftState.h，commit f184093）。舊的自由函式名
//  `TMyTempPanel_edXxxMouseDown` 已不存在；上面提到的行號也已失效，
//  刻意不再填新行號——行號會隨每次附加而漂移，用名字搜尋即可。 Their BODIES are still gated,
//  but on co-blockers other than the parameter types: a duplicate
//  `TEdit *Buffer`, an unqualified `edOffset` inside a free function, a null
//  `fTemp_Set`, and two missing includes. Tag stopped being one of those
//  blockers this wave.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "MyTempPanel.h"
//   #include "uTemp_Set.h" / "BarcodeReader.h" / "myQwertyKeyBoard.h" /
//     "ATC_Handler_Side.h" / "cmydef.h"
//   #pragma package(smart_init)
//
//   NOT re-included here, and why: "uTemp_Set.h" -- golden's `fTemp_Set` form;
//   its ONLY use in the ACTIVE bodies below was the ctor's
//   `new TPanel(fTemp_Set->tsUnUse)` parent argument, itself gated (see
//   ctor's own note -- vclcompat::TPanel takes no ctor argument at all).
//   Every OTHER `fTemp_Set` reference in golden (MaxTempSetting()/
//   MinTempSetting()) lives exclusively inside the 4 omitted mouse-down
//   handlers (GATE 1). "BarcodeReader.h" (Barcode_Reader) / "myQwertyKeyBoard.h"
//   (fQwertyKey) / "ATC_Handler_Side.h" -- referenced ONLY inside the same 4
//   omitted handlers. "cmydef.h" -- nothing in the 7 ACTIVE bodies below
//   reaches a cmydef.h-only symbol (MyDBIProcess is forward-declared locally,
//   matching this tree's established idiom -- see the ctor/dtor note below).
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella)
#include "MyTempPanel.h"            // this unit's own contract
// AI(W906-FW-SIG-W16) 20260826: 四支 MouseDown 回填 golden 完整簽章。
#include "vclcompat/ShiftState.h"

// MyDBIProcess -- golden home aHotPlateSubstrate.h:924. Forward-declared
// locally rather than pulling in that whole header, matching this tree's
// widely-used established idiom for this EXACT function (automation.cpp:72,
// CCLink/MyCCLink.cpp:36, OmronLaser/LaserSensor.cpp, SECSGEM/
// uHGemEquipment.cpp:93 all do the identical one-line forward declaration
// rather than #include-ing aHotPlateSubstrate.h whole).
extern void MyDBIProcess(AnsiString S1, AnsiString S2);
//---------------------------------------------------------------------------
TList *listNormal;
TList *listArm1;
TList *listArm2;
//TList *listDut;
//TList *listNoUse;
//TList *listHeater;
//---------------------------------------------------------------------------
TMyTempPanel::TMyTempPanel(AnsiString Alias, int iTag)
{
    AnsiString Str;
    //AI(W906-PT-W3) 20260807: golden `new TPanel(fTemp_Set->tsUnUse)` (golden
    //  :23) -- vclcompat::TPanel (and every other vclcompat control type used
    //  below) takes NO constructor argument at all (see vclcompat/
    //  Controls.h); golden's Owner/initial-Parent argument is dropped, same
    //  idiom EJ1N/MyOmronPanel.cpp and MyVacuumPanel.cpp already use for
    //  every one of their own `new TXxx(this)`/`new TXxx(palTemp)` calls.
    palTemp         =new TPanel();

    labName         =new TLabel();                                       //name

    edLow           =new TEdit();
    edMid           =new TEdit();
    edLowbase       =new TEdit();
    edBase          =new TEdit();
    edHighBase      =new TEdit();
    edSHighBase     =new TEdit();                                        //Ztex 2024.07.27 Add 6 Point Temperature Offset
    //==> Eastsun 20260526 #026-4.A7 Ifor 20241101 add :KYEC
    edPreOffset     =new TEdit();       //預先補償
    edPreOfsTime    =new TEdit();       //補償秒數
    edAfterOfs      =new TEdit();       //補償秒數結束後的補償值
    //<== Eastsun 20260526 #026-4.A7

    edKit_Low       =new TEdit();                                        //JerryYang 20221003 Analog要求新增Kit三點offset
    edKit_Mid       =new TEdit();
    edKit_Lowbase   =new TEdit();
    edKit_Base      =new TEdit();
    edKit_HighBase  =new TEdit();

    edOffset        =new TEdit();

    edSingleLimit   =new TEdit();
    edIndiTemp      =new TEdit();
    edInitTempOffset=new TEdit();
    edEOTTempOffset =new TEdit();
    palLine         =new TPanel();

    iIndexTag=iTag;

    //AI(W906-PT-W3) 20260807: golden's ->Left/->Top/->Width/->Height/
    //  ->DragKind/->BevelInner/->BevelOuter writes (golden :55-59,61-62) have
    //  no substrate (vclcompat::TPanel carries only Caption+Color, plus
    //  TControl's Visible/Enabled -- see Controls.h). ->Color and ->Visible
    //  ARE real and stay ACTIVE below.
#if 0
    palTemp->Left      =0;
    palTemp->Top       =0;
    palTemp->Width     =793;
    palTemp->Height    =27;
    palTemp->DragKind  =dkDock;
#endif
    palTemp->Color     =0x00C2B8A6;
#if 0
    palTemp->BevelInner=bvNone;
    palTemp->BevelOuter=bvNone;
#endif
    palTemp->Visible   =true;

    //AI(W906-PT-W3) 20260807: golden's ->Parent/->AutoSize/->Left/->Top/
    //  ->Width/->Height/->Font->.../->ParentFont writes on labName (golden
    //  :65,66-70,72-76) have no substrate (vclcompat::TLabel carries only
    //  Caption+Color, plus TControl's Visible/Enabled). ->Caption IS real and
    //  stays ACTIVE.
#if 0
    labName->Parent=palTemp;
    labName->AutoSize=false;
    labName->Left = 5;
    labName->Top = 2;
    labName->Width = 80;
    labName->Height = 16;
#endif
    labName->Caption=Alias;
#if 0
    labName->Font->Charset = DEFAULT_CHARSET;
    labName->Font->Color = clNavy;
    labName->Font->Size = 10;
    labName->Font->Name = "MS Sans Serif";
    labName->ParentFont = false;
#endif

    //AI(W906-PT-W3) 20260807: golden's ->Parent/Str-based ->Name/->AutoSize/
    //  ->Left/->Top/->Width/->Height/->ImeMode/->ParentFont writes (golden
    //  :78-87) have no substrate (vclcompat::TCustomEdit/TEdit carry only
    //  Text, plus TControl's Visible/Enabled -- no ->Name member anywhere in
    //  this port's TEdit chain either, unlike golden's TWinControl-derived
    //  original). ->Enabled IS real and stays ACTIVE. ->OnMouseDown has no
    //  substrate AND its target (edBaseMouseDown) is omitted -- GATE (1).
#if 0
    edLow->Parent=palTemp;
    Str.sprintf("edLow%03d", iTag);
    edLow->Name=Str;
    edLow->AutoSize=false;
    edLow->Left = 90;
    edLow->Top = 2;
    edLow->Width = 50;                                                          //60->50 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edLow->Height = 21;
    edLow->ImeMode= imClose;
    edLow->ParentFont = false;
#endif
    edLow->Enabled =true;
#if 0
    edLow->OnMouseDown=edBaseMouseDown;
#endif

    //AI(W906-PT-W3) 20260807: every edKit_Xxx block below (golden :91-101,
    //  116-126,141-151,166-176,191-201) repeats the SAME gated shape as
    //  edLow above, PLUS a `->Color=clMedGray;` golden write that also has no
    //  substrate (TCustomEdit/TEdit carry no Color member -- only TPanel/
    //  TLabel do). Only ->Enabled stays ACTIVE for each. Not re-annotated
    //  per-block below to keep this file proportional to golden's own
    //  density; this one note covers all 5 Kit_ blocks.
#if 0
    edKit_Low->Parent=palTemp;                                                  //JerryYang 20221003 Analog要求新增Kit三點offset
#endif
#if 0
    edKit_Low->AutoSize=false;
    edKit_Low->Left = 90;
    edKit_Low->Top = 2;
    edKit_Low->Width = 60;
    edKit_Low->Height = 21;
    edKit_Low->ImeMode= imClose;
    edKit_Low->ParentFont = false;
#endif
    edKit_Low->Enabled =true;
#if 0
    edKit_Low->OnMouseDown=edBaseMouseDown;
    edKit_Low->Color=clMedGray;
#endif

#if 0
    edMid->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edMid%03d", iTag);
    edMid->Name=Str;
    edMid->AutoSize=false;
    edMid->Left = 141;                                                          //151->141 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edMid->Top = 2;
    edMid->Width = 50;                                                          //60->50 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edMid->Height = 21;
    edMid->ImeMode= imClose;
    edMid->ParentFont = false;
#endif
    edMid->Enabled =true;
#if 0
    edMid->OnMouseDown=edBaseMouseDown;
#endif

#if 0
    edKit_Mid->Parent=palTemp;                                                  //JerryYang 20221003 Analog要求新增Kit三點offset
#endif
#if 0
    edKit_Mid->AutoSize=false;
    edKit_Mid->Left = 151;
    edKit_Mid->Top = 2;
    edKit_Mid->Width = 60;
    edKit_Mid->Height = 21;
    edKit_Mid->ImeMode= imClose;
    edKit_Mid->ParentFont = false;
#endif
    edKit_Mid->Enabled =true;
#if 0
    edKit_Mid->OnMouseDown=edBaseMouseDown;
    edKit_Mid->Color=clMedGray;
#endif

#if 0
    edLowbase->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edLowbase%03d", iTag);
    edLowbase->Name=Str;
    edLowbase->AutoSize=false;
    edLowbase->Left = 192;                                                      //192->212 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edLowbase->Top = 2;
    edLowbase->Width = 50;                                                      //60->50 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edLowbase->Height = 21;
    edLowbase->ImeMode= imClose;
    edLowbase->ParentFont = false;
#endif
    edLowbase->Enabled =true;
#if 0
    edLowbase->OnMouseDown=edBaseMouseDown;
#endif

#if 0
    edKit_Lowbase->Parent=palTemp;                                              //JerryYang 20221003 Analog要求新增Kit三點offset
#endif
#if 0
    edKit_Lowbase->AutoSize=false;
    edKit_Lowbase->Left = 212;
    edKit_Lowbase->Top = 2;
    edKit_Lowbase->Width = 60;
    edKit_Lowbase->Height = 21;
    edKit_Lowbase->ImeMode= imClose;
    edKit_Lowbase->ParentFont = false;
#endif
    edKit_Lowbase->Enabled =true;
#if 0
    edKit_Lowbase->OnMouseDown=edBaseMouseDown;
    edKit_Lowbase->Color=clMedGray;
#endif

#if 0
    edBase->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edBase%03d", iTag);
    edBase->Name=Str;
    edBase->AutoSize=false;
    edBase->Left = 243;                                                         //192->273 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edBase->Top = 2;
    edBase->Width = 50;                                                         //60->50 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edBase->Height = 21;
    edBase->ImeMode= imClose;
    edBase->ParentFont = false;
#endif
    edBase->Enabled =true;
#if 0
    edBase->OnMouseDown=edBaseMouseDown;
#endif

#if 0
    edKit_Base->Parent=palTemp;                                                 //JerryYang 20221003 Analog要求新增Kit三點offset
#endif
#if 0
    edKit_Base->AutoSize=false;
    edKit_Base->Left = 273;
    edKit_Base->Top = 2;
    edKit_Base->Width = 60;
    edKit_Base->Height = 21;
    edKit_Base->ImeMode= imClose;
    edKit_Base->ParentFont = false;
#endif
    edKit_Base->Enabled =true;
#if 0
    edKit_Base->OnMouseDown=edBaseMouseDown;
    edKit_Base->Color=clMedGray;
#endif

#if 0
    edHighBase->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edHighBase%03d", iTag);
    edHighBase->Name=Str;
    edHighBase->AutoSize=false;
    edHighBase->Left = 294;                                                     //294->334 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edHighBase->Top = 2;
    edHighBase->Width = 50;                                                     //60->50 //Ztex 2024.07.27 Add 6 Point Temperature Offset
    edHighBase->Height = 21;
    edHighBase->ImeMode= imClose;
    edHighBase->ParentFont = false;
#endif
    edHighBase->Enabled =true;
#if 0
    edHighBase->OnMouseDown=edBaseMouseDown;
#endif

#if 0
    edKit_HighBase->Parent=palTemp;                                             //JerryYang 20221003 Analog要求新增Kit三點offset
#endif
#if 0
    edKit_HighBase->AutoSize=false;
    edKit_HighBase->Left = 334;
    edKit_HighBase->Top = 2;
    edKit_HighBase->Width = 60;
    edKit_HighBase->Height = 21;
    edKit_HighBase->ImeMode= imClose;
    edKit_HighBase->ParentFont = false;
#endif
    edKit_HighBase->Enabled =true;
#if 0
    edKit_HighBase->OnMouseDown=edBaseMouseDown;
    edKit_HighBase->Color=clMedGray;
#endif

#if 0
    edSHighBase->Parent=palTemp;                                                //Ztex 2024.07.27 Add 6 Point Temperature Offset ==>
#endif
#if 0
    Str.sprintf("edSHighBase%03d", iTag);
    edSHighBase->Name=Str;
    edSHighBase->AutoSize=false;
    edSHighBase->Left = 345;
    edSHighBase->Top = 2;
    edSHighBase->Width = 50;
    edSHighBase->Height = 21;
    edSHighBase->ImeMode= imClose;
    edSHighBase->ParentFont = false;
#endif
    edSHighBase->Enabled =true;
#if 0
    edSHighBase->OnMouseDown=edBaseMouseDown;                                   //Ztex 2024.07.27 Add 6 Point Temperature Offset <==
#endif
    //==> Eastsun 20260526 #026-4.A7 Ifor 20241101 add :KYEC
#if 0
    edPreOffset->Parent=palTemp;
#endif
#if 0
    edPreOffset->AutoSize=false;
    edPreOffset->Left = 212;
    edPreOffset->Top = 2;
    edPreOffset->Width = 60;
    edPreOffset->Height = 21;
    edPreOffset->ImeMode= imClose;
    edPreOffset->ParentFont = false;
#endif
    edPreOffset->Enabled =true;
    edPreOffset->Visible =false;
#if 0
    edPreOffset->OnMouseDown=edBaseMouseDown;
    edPreOffset->Color=clMedGray;
#endif
#if 0
    edPreOfsTime->Parent=palTemp;
#endif
#if 0
    edPreOfsTime->AutoSize=false;
    edPreOfsTime->Left = 273;
    edPreOfsTime->Top = 2;
    edPreOfsTime->Width = 60;
    edPreOfsTime->Height = 21;
    edPreOfsTime->ImeMode= imClose;
    edPreOfsTime->ParentFont = false;
#endif
    edPreOfsTime->Enabled =true;
    edPreOfsTime->Visible =false;
#if 0
    edPreOfsTime->OnMouseDown=edBaseMouseDown;
    edPreOfsTime->Color=clMedGray;
#endif
#if 0
    edAfterOfs->Parent=palTemp;
#endif
#if 0
    edAfterOfs->AutoSize=false;
    edAfterOfs->Left = 334;
    edAfterOfs->Top = 2;
    edAfterOfs->Width = 60;
    edAfterOfs->Height = 21;
    edAfterOfs->ImeMode= imClose;
    edAfterOfs->ParentFont = false;
#endif
    edAfterOfs->Enabled =true;
    edAfterOfs->Visible =false;
#if 0
    edAfterOfs->OnMouseDown=edBaseMouseDown;
    edAfterOfs->Color=clMedGray;
#endif
    //<== Eastsun 20260526 #026-4.A7

#if 0
    edOffset->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edOffset%03d", iTag);
    edOffset->Name=Str;
    edOffset->AutoSize=false;
    edOffset->Left = 415;
    edOffset->Top = 2;
    edOffset->Width = 60;
    edOffset->Height = 21;
    edOffset->BorderStyle=bsNone;
#endif
    edOffset->Text = "0";
#if 0
    edOffset->Color = clBtnFace;
    edOffset->ParentFont = false;
#endif
    edOffset->Enabled =true;
#if 0
    edOffset->OnMouseDown=edBaseMouseDown;
#endif

#if 0
    edSingleLimit->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edSingleLimit%03d", iTag);
    edSingleLimit->Name=Str;
    edSingleLimit->AutoSize=false;
    edSingleLimit->Left = 494;
    edSingleLimit->Top = 2;
    edSingleLimit->Width = 60;
    edSingleLimit->Height = 21;
    edSingleLimit->ParentFont = false;
#endif
    edSingleLimit->Enabled =true;
#if 0
    edSingleLimit->OnMouseDown=edLimitMouseDown;
#endif

#if 0
    edIndiTemp->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edIndiTemp%03d", iTag);
    edIndiTemp->Name=Str;
    edIndiTemp->AutoSize=false;
    edIndiTemp->Left = 573;
    edIndiTemp->Top = 2;
    edIndiTemp->Width = 60;
    edIndiTemp->Height = 21;
    edIndiTemp->ParentFont = false;
#endif
    edIndiTemp->Enabled =true;
#if 0
    edIndiTemp->OnMouseDown=edIndiviMouseDown;
#endif

#if 0
    edInitTempOffset->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edInitTempOffset%03d", iTag);
    edInitTempOffset->Name=Str;
    edInitTempOffset->AutoSize=false;
    edInitTempOffset->Left = 652;
    edInitTempOffset->Top = 2;
    edInitTempOffset->Width = 60;
    edInitTempOffset->Height = 21;
    edInitTempOffset->ParentFont = false;
#endif
    edInitTempOffset->Enabled =true;
#if 0
    edInitTempOffset->OnMouseDown=edinitialMouseDown;                           //kevin 20210421 edBaseMouseDown; ->edinitialMouseDown 獨立 範圍
    //edInitTempOffset->OnMouseDown=edBaseMouseDown;
#endif

#if 0
    edEOTTempOffset->Parent=palTemp;
#endif
#if 0
    Str.sprintf("edEOTTempOffset%03d", iTag);
    edEOTTempOffset->Name=Str;
    edEOTTempOffset->AutoSize=false;
    edEOTTempOffset->Left = 731;
    edEOTTempOffset->Top = 2;
    edEOTTempOffset->Width = 60;
    edEOTTempOffset->Height = 21;
    edEOTTempOffset->ParentFont = false;
#endif
    edEOTTempOffset->Enabled =true;
#if 0
    edEOTTempOffset->OnMouseDown=edBaseMouseDown;
#endif

    palLine->Caption="";
#if 0
    palLine->Parent=palTemp;
    palLine->Height=1;
    palLine->BevelInner=bvNone;
    palLine->BevelOuter=bvNone;
    palLine->Align=alBottom;
#endif

    // AI(W906-FW-TAG1) 20260825: OPENED, and with REAL values.
    //   Tag exists on vclcompat::TControl since this wave, and the argument
    //   side was already live: uTemp_Set.cpp:339-341 builds one panel per
    //   channel passing the channel index (`new TMyTempPanel(asTempCtrl[i], i)`,
    //   golden uTemp_Set.cpp:152-155, same loop), so every edit below receives
    //   its eTempControll id exactly as golden does. This is the ONE Tag family
    //   in the tree whose provenance is C++ ctor assignment rather than the
    //   .dfm designer -- see the provenance note in vclcompat/Controls.h.
    //
    //   TWO CORRECTIONS to the note this replaces, recorded rather than
    //   silently dropped:
    //     * it said "19" writes -- golden has 18 (counted in the opened file,
    //       golden :325-346). `edSHighBase` deliberately never gets one, so
    //       golden itself runs that one widget at Tag==0.
    //     * it said the consumer handlers "do not exist" -- PT-W8 superseded
    //       that on 20260811: all four MouseDown handlers exist as file-scope
    //       free functions (:825, :950, :974, :1011). Their bodies stay gated,
    //       but on co-blockers OTHER than Tag (duplicate `TEdit *Buffer`,
    //       unqualified `edOffset`, a null `fTemp_Set`, missing includes).
    edLow->Tag           =iTag;
    edMid->Tag           =iTag;
    edLowbase->Tag       =iTag;
    edBase->Tag          =iTag;
    edHighBase->Tag      =iTag;
    // GOLDEN BUG (TAG1-a) -- edSHighBase is MISSING from this list, and it is
    //   the only one of the 19 edits that is.  Golden creates it (golden
    //   MyTempPanel.cpp:32), configures it fully (:203-213) and wires it to
    //   the SAME Tag-reading handler as its 16 siblings
    //   (`edSHighBase->OnMouseDown=edBaseMouseDown;`, golden :214) -- but
    //   golden's Tag block (:325-346) never assigns it, so it runs at the VCL
    //   default Tag==0 on every channel.  Since MachineType.h:637 makes
    //   tcHotPlate1==0, edBaseMouseDown's Tri-Temp arm then matches
    //   `(Tag>=tcHotPlate1 && Tag<=tcShuttle2)` for EVERY panel, and the
    //   6th-point offset field gets the HotPlate/Shuttle +/-30 clamp computed
    //   from Temperature.fWorkTemperBase instead of its own channel's
    //   SetHeaterTemp_Max* clamp.  Almost certainly an oversight when the
    //   6-point feature was added (golden's own marker on :32/:203/:214 is
    //   "Ztex 2024.07.27 Add 6 Point Temperature Offset", years after this
    //   Tag block).  Reproduced faithfully: this port does not assign it
    //   either.  Ruling needed before real hardware, like DEFECT (i).
    //==> Eastsun 20260526 #026-4.A7 Ifor 20241101 add :KYEC
    edPreOffset->Tag     =iTag;
    edPreOfsTime->Tag    =iTag;
    edAfterOfs->Tag      =iTag;
    //<== Eastsun 20260526 #026-4.A7

    edKit_Low->Tag       =iTag;                                                 //JerryYang 20221003 Analog要求新增Kit三點offset
    edKit_Mid->Tag       =iTag;
    edKit_Lowbase->Tag   =iTag;
    edKit_Base->Tag      =iTag;
    edKit_HighBase->Tag  =iTag;

    edOffset->Tag        =iTag;
    edSingleLimit->Tag   =iTag;
    edIndiTemp->Tag      =iTag;
    edInitTempOffset->Tag=iTag;
    edEOTTempOffset->Tag =iTag;
    (void)Str;
    iOffsetByRecipeMaxLimit = 60;
    iOffsetByRecipeMinLimit = -60;
}
//---------------------------------------------------------------------------
TMyTempPanel::~TMyTempPanel()
{
    try
    {
        delete labName;
        delete edLow;
        delete edMid;
        delete edLowbase;
        delete edBase;
        delete edHighBase;
        //==> Eastsun 20260526 #026-4.A7 Ifor 20241101 add :KYEC
        delete edPreOffset;
        delete edPreOfsTime;
        delete edAfterOfs;
        //<== Eastsun 20260526 #026-4.A7

        delete edKit_Low;                                                       //JerryYang 20221003 Analog要求新增Kit三點offset
        delete edKit_Mid;
        delete edKit_Lowbase;
        delete edKit_Base;
        delete edKit_HighBase;

        delete edOffset;
        delete edSingleLimit;
        delete edIndiTemp;
        delete edInitTempOffset;
        delete edEOTTempOffset;
        delete palLine;
        //AI(W906-PT-W3) 20260807: golden `palTemp->Parent=NULL;` (golden
        //  :379) has no substrate (no ->Parent member, see ctor's note).
#if 0
        palTemp->Parent=NULL;
#endif
        delete palTemp;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~TMyTempPanel");
    }
}
//---------------------------------------------------------------------------
void TMyTempPanel::SetParent(TTabSheet *tsTemp)                      //目前設定值
{
    //AI(W906-PT-W3) 20260807: golden `palTemp->Parent=tsTemp;
    //  palTemp->Align=alTop;` (golden :390-391) has no substrate (no
    //  ->Parent, no ->Align member on vclcompat::TPanel). ACTIVE arm: no-op.
#if 0
    palTemp->Parent=tsTemp;
    palTemp->Align=alTop;
#endif
    (void)tsTemp;
}
//---------------------------------------------------------------------------
void TMyTempPanel::SetEnable(bool bflag)
{
    edLow->Enabled=bflag;
    edMid->Enabled=bflag;
    edLowbase->Enabled=bflag;
    edBase->Enabled=bflag;
    edHighBase->Enabled=bflag;
    edSHighBase->Enabled=bflag;                                                 //Ztex 2024.07.27 Add 6 Point Temperature Offset

    edKit_Low->Enabled=bflag;                                                   //JerryYang 20221003 Analog要求新增Kit三點offset
    edKit_Mid->Enabled=bflag;
    edKit_Lowbase->Enabled=bflag;
    edKit_Base->Enabled=bflag;
    edKit_HighBase->Enabled=bflag;

    edOffset->Enabled=bflag;
    edSingleLimit->Enabled=bflag;
    edIndiTemp->Enabled=bflag;
    edInitTempOffset->Enabled=bflag;
    edEOTTempOffset->Enabled=bflag;
    labName->Enabled=bflag;
}
//---------------------------------------------------------------------------
//AI(W906-PT-W3) 20260807: GATE (1) -- golden `edBaseMouseDown`/
//  `edLimitMouseDown`/`edIndiviMouseDown`/`edinitialMouseDown` (golden
//  :417-599) OMITTED entirely: their `TMouseButton`/`TShiftState` parameter
//  types cannot be spelled anywhere in this port (see MyTempPanel.h's own
//  banner GATE (1)). Golden bodies kept in comment form, for provenance, in
//  MyTempPanel.h's own GATE (1) citation -- not repeated verbatim here a
//  second time to keep this already-large file proportional; see golden
//  MyTempPanel.cpp:417-599 directly for the full text if ever needed.
//---------------------------------------------------------------------------
void TMyTempPanel::SetCaption(AnsiString Name)
{
    labName->Caption=Name;
}
//---------------------------------------------------------------------------
void TMyTempPanel::SetIndexTag(int iTag)
{
    iIndexTag=iTag;
}
//---------------------------------------------------------------------------
AnsiString TMyTempPanel::GetCaption()
{
    return labName->Caption;
}
//---------------------------------------------------------------------------

// =============================================================================
//  APPEND BLOCK -- BANNER EXTENSION
//  AI(W906-PT-W8) 20260811 -- the 4 golden mouse-down handlers this file's
//  original banner listed as "OMITTED per MyTempPanel.h's GATE (1)" are landed
//  here as real, compilable, callable bodies.  Nothing above this line is
//  edited or reordered (append-only).
//
//  WHY THE ORIGINAL OMISSION IS SUPERSEDED, AND WHAT REPLACES IT
//  -------------------------------------------------------------------------
//  The PT-W3 reasoning ("`TMouseButton`/`TShiftState` cannot be spelled, so
//  the SIGNATURE cannot exist") is still a TRUE fact about those two types --
//  re-verified this wave, see GATE (W8-1) -- but it does not force the whole
//  handler to disappear, because NEITHER PARAMETER IS EVER READ.  All four
//  golden bodies (golden MyTempPanel.cpp:417-599) were read line by line:
//  `Button` and `Shift` appear ONLY in the parameter lists.  Dropping two
//  provably-dead parameters costs zero behaviour, and buys four real bodies.
//
//  SHAPE, AND WHY IT IS NOT A MEMBER FUNCTION
//  -------------------------------------------------------------------------
//  These are landed as file-scope free functions taking an explicit
//  `TMyTempPanel *Self`, NOT as `TMyTempPanel::edBaseMouseDown(...)`.  The
//  reason is mechanical, not a design opinion: PT-W3 removed the four
//  declarations from MyTempPanel.h's class body, and MyTempPanel.h is OUTSIDE
//  this wave's write boundary (this wave may append to this .cpp only).  A
//  member definition without a matching in-class declaration does not compile.
//  See the HAND-OFF block at the end of this file for the exact 4 lines the
//  integrating loop should publish in MyTempPanel.h to collapse these back
//  into members.  Until it does, these four have EXTERNAL LINKAGE AND NO
//  CALLER anywhere in the tree -- stated plainly rather than implied.
//
//  WAVE SCOPE -- ONE LINE PER GOLDEN FUNCTION (all golden MyTempPanel.cpp)
//   edBaseMouseDown      golden :417-526  -- GATED (body 100%); ACTIVE arm is
//                                            the Sender downcast + faithful
//                                            "user did nothing" no-op.
//   edLimitMouseDown     golden :528-532  -- GATED (body 100%, single
//                                            ShowQwertyKey call).
//   edIndiviMouseDown    golden :534-560  -- GATED (body 100%).
//   edinitialMouseDown   golden :562-599  -- GATED (body 100%).
//
//  GATE REGISTER -- 5 gates.  Every absence claim below carries the command
//  and the time it was run; all of them were re-run immediately before this
//  block was written (see the RE-RUN LOG at the end of this file).
//
//   (W8-1) `TMouseButton Button` / `TShiftState Shift` (golden :418, :529,
//       :535, :563) -- DROPPED FROM THE PORT SIGNATURE, not gated.
//       ⚠ AI(W906-FW-SIG-W16) 20260826: 下面這條 absence claim **已經過期**。
//       `vclcompat/ShiftState.h`（commit f184093）補上了這兩個型別，本波因此
//       把四支的簽章回填成 golden 原文，參數不再被丟掉。
//       底下的原文保留，因為它記錄的是「當時為什麼那樣做」——那個判斷在當時
//       是對的（而且它自己就註明兩個參數在 golden 裡是死的，所以丟掉零風險）。
//       ABSENCE CLAIM: no type of either name exists anywhere in this port.
//         cmd:  grep -rn --include=*.h --include=*.cpp -E
//               '^[[:space:]]*(class|struct|enum|typedef|using)[^;]*
//               \b(TMouseButton|TShiftState)\b' .   (from the tree root,
//               excluding ./tools/)
//         run:  2026-08-11 11:22:14  -> 0 hits (exit 1)
//         re-run at end of wave: see RE-RUN LOG.
//       WHY FAITHFUL: both parameters are DEAD in golden itself.  Verified by
//       reading golden :417-599 in full: the identifiers `Button` and `Shift`
//       occur exactly 4 times each, all of them in a parameter list, never in
//       a body.  A dropped parameter that is never read cannot change any
//       observable behaviour.  `X`/`Y` are equally dead in all four of THESE
//       handlers but are KEPT, because they are spellable ints and because
//       the sibling EJ1N/MyOmronPanel handlers landed by this same wave DO
//       read them -- one uniform ported signature across the wave.
//       REAL-MACHINE DIFFERENCE: none.
//
//   (W8-2) `Buffer->Tag` -- every branch CONDITION in edBaseMouseDown
//       (golden :425,433,441-445,457,482-485) and edIndiviMouseDown
//       (golden :543,547).  vclcompat::TControl (and therefore TEdit) carried
//       NO `Tag` member.
//         cmd:  grep -rn 'Tag' vclcompat/Controls.h
//         run:  2026-08-11 11:23:06  -> 0 hits
//       EXPIRED 20260825 (FW-TAG1): Tag is now a real member on
//       vclcompat::TControl, and this panel's 18 ctor writes above are LIVE
//       with golden's own per-channel values.  What still keeps THIS block
//       shut is everything else the note below lists (W8-3 fTemp_Set setters,
//       W8-5 Barcode_Reader) plus three port-side obstacles: a duplicate
//       `TEdit *Buffer`, an unqualified `edOffset` inside a free function,
//       and two missing includes.
//       WHY FAITHFUL: this is a branch selector, not a leaf, so it cannot be
//       reduced -- but EVERY leaf it selects between is itself a
//       `fQwertyKey->ShowQwertyKey(...)` call (GATE W8-4), i.e. all arms have
//       the same, empty, ported effect.  Choosing "no arm" is therefore
//       observationally identical to choosing any arm.
//       A SHORTCUT THAT WAS CONSIDERED AND REJECTED, recorded so the next
//       wave does not re-derive it wrongly: `iIndexTag` looks like a
//       drop-in for `Buffer->Tag`, because golden's ctor sets BOTH from the
//       same `iTag` argument (golden :53 and :325-346).  IT IS NOT.
//       golden uTemp_Set.cpp:154 constructs each panel as
//       `new TMyTempPanel(asTempCtrl[i], i)` -- so the 18 widgets' `->Tag`
//       (18, not 19: edSHighBase never gets one -- GOLDEN BUG (TAG1-a) at the
//       ctor block above)
//       hold the eTempControll CHANNEL index -- and then uTemp_Set.cpp:267-283
//       calls `SetIndexTag(-1)` / `SetIndexTag(0..N)` on the same panels,
//       which (golden :606-609) rewrites `iIndexTag` ONLY and never touches
//       any widget's `->Tag`.  After that call the two hold DIFFERENT
//       numbering systems (channel index vs heater ordinal), on every machine
//       where CosFunction.bUseOldATCTempOffset==false.  Substituting one for
//       the other would silently select the wrong branch.
//       REAL-MACHINE DIFFERENCE: on real hardware the Tag decides which
//       min/max pair the pop-up keypad clamps to; here no keypad opens at all.
//
//   (W8-3) `dTempMax=fTemp_Set->MaxTempSetting();` /
//       `dTempMin=fTemp_Set->MinTempSetting();` (golden :422-423, :539-540).
//       `dTempMax`/`dTempMin` THEMSELVES ARE REAL in this port (cmydef.h:3437-
//       3438, defined cmydef.cpp:3649-3650 = 135.0 / 20) -- it is the SOURCE,
//       the `fTemp_Set` form, that has no port.
//         cmd:  grep -rn --include=*.cpp --include=*.h -E
//               'fTemp_Set[[:space:]]*(;|=)|\*[[:space:]]*fTemp_Set|TfTemp_Set' .
//         run:  2026-08-11 11:25:53  -> only Automation/auto9045.cpp's
//               TU-LOCAL stand-in `W5FA_TfTemp_SetExt W5FA_FTemp_Set`
//               (edSoakTime/edWorkTemp only -- no MaxTempSetting/
//               MinTempSetting), plus ./build/* generated layout tables and
//               comments.  The atester.cpp:2245-2248 hits are INSIDE the
//               `#if 0` that opens at atester.cpp:834, confirmed by scanning
//               that file's preprocessor directives -- inert text, not a port.
//       WHY FAITHFUL: the ACTIVE arm leaves both globals at whatever the rest
//       of the system last wrote.  This is the SAME treatment bthermo.cpp's
//       own GATE W7-UI G27 already applies to the same missing form
//       (bthermo.cpp:4536-4541).  Not writing a value we cannot compute is
//       strictly safer than writing a guessed one, and dTempMax/dTempMin are
//       shared globals other units read.
//       REAL-MACHINE DIFFERENCE: on real hardware these two globals are
//       refreshed from the Temp_Set form on every such click; here they keep
//       their cmydef.cpp defaults unless some other ported unit sets them.
//
//   (W8-4) `fQwertyKey->ShowQwertyKey(...)` -- the ONLY leaf action in all
//       four handlers (golden :435,449,454,461,466,473,478,491,496,503,508,
//       518,522,531,545,549,553,558,580,584,592,596).  fQwertyKey has no port. [EXPIRED 20260824, see UPDATE]
//         cmd:  grep -rn --include=*.h --include=*.cpp -E
//               'fQwertyKey|ShowQwertyKey' .   (excluding ./tools/, ./build/)
//         run:  2026-08-11 11:24:21  -> zero declarations/definitions; only
//               comments and generated .dfm layout tables under ./build/.
//         UPDATE 20260824 (FW-QWKEY2): claim EXPIRED -- FW-QWKEY1 (fc08e09)
//         landed forms/fQwertyKey.{h,cpp}. The single-call site
//         edLimitMouseDown is un-gated and live; the branch-tree blocks
//         below STAY gated on their other co-blockers (W8-2 Buffer->Tag /
//         W8-3 fTemp_Set setters / W8-5 Barcode_Reader).
//       WHY FAITHFUL: this is the "keyboard/dialog interaction with no
//       offline equivalent" case, and the faithful offline default is
//       USER DID NOTHING.  ShowQwertyKey's entire contract is "pop a modal
//       on-screen keypad, and if the operator commits a value, write it into
//       the TEdit passed as argument 1".  With no window subsystem there is no
//       operator and no commit, so "no keypad, no edit" is not a degraded
//       stand-in -- it is exactly what happens.  Established precedent in this
//       tree for the identical golden idiom: Public/HTEdit.cpp GATE (6)
//       (EditClick), EJ1N/MyOmronPanel.cpp GATE (3) (setEditValueClick),
//       VacuumUnit/MyVacuumPanel.cpp GATE (3) (edSVClick),
//       ATC/ATCInterface.h GATE (4), OmronLaser/LaserSensor.h.
//       REAL-MACHINE DIFFERENCE: an operator cannot tap these 19 TEdit fields
//       to enter a temperature offset in this build.
//
//   (W8-5) `Barcode_Reader(bcTemperature)` (golden :427, :569).
//         cmd:  grep -rn --include=*.h --include=*.cpp -E
//               '\bBarcode_Reader[[:space:]]*\(' .
//         run:  2026-08-11 11:24:21  -> exactly ONE hit, Public/HTEdit.cpp:300,
//               and that line is inside that file's own `#if 0` GATE (6).
//               No declaration, no definition, anywhere.
//       `bcTemperature` itself IS real (MachineType.h:921) -- only the
//       function is missing.  WHY FAITHFUL: golden uses it as an early-return
//       guard whose ONLY effect is to skip the ShowQwertyKey call below it,
//       which is already gated (W8-4).  Both the "returned early" and the
//       "fell through" paths therefore have identical ported effect (none).
//       REAL-MACHINE DIFFERENCE: on a KYEC machine the operator must scan a
//       barcode before the keypad opens; here neither happens.
//
//  Big5: every Chinese comment below is transcribed VERBATIM from golden via
//  cp950, character for character.  Final gate: ZERO U+FFFD.
// =============================================================================
#include "cmydef.h"        // dTempMax/dTempMin (:3437-3438), N_DOUBLE (:290),
                           //   INSTALL_HEAT_GUN (:3366), Tri_Temp_Machine (:5536),
                           //   SetHeaterTemp_MaxOutSht/MaxIndex/MaxBase (:5543-5545),
                           //   CUSTOMER_CODE (:3181) -- ALL REAL in this port
#include "cprod.h"         // Temperature (SYSTEM_TEMPERATURE, :1646) /
                           //   InputLimit (INPUT_LIMIT, :3058) -- both REAL
#include "CosFunction.h"   // CosFunction (:488) -- REAL
#include "MachineType.h"   // eTempControll tcXxx (:637-652), CC_ASE_KaohSiung
#include "forms/fQwertyKey.h"  // AI(W906-FW-QWKEY2) 20260824: fQwertyKey extern for un-gated ShowQwertyKey sites (real since FW-QWKEY1 fc08e09; latent until HTEdit GATE (6) wiring)
                           //   (:308), bcTemperature (:921) -- ALL REAL
//---------------------------------------------------------------------------
//  golden MyTempPanel.cpp:417-526  --  TMyTempPanel::edBaseMouseDown
//  Ported signature drops `TMouseButton Button, TShiftState Shift` (GATE W8-1)
//  and takes `TMyTempPanel *Self` instead of `this` (see BANNER EXTENSION).
//AI(W906-FW-SIG-W16) 20260826: 收回成員並回填 golden 完整簽章。
void TMyTempPanel::edBaseMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W16): golden 這四支也沒讀這兩個
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;

    //AI(W906-PT-W8) 20260811: GATES (W8-2)/(W8-3)/(W8-4)/(W8-5) -- golden's
    //  whole body is a `Buffer->Tag` branch tree (no Tag member here) whose
    //  every leaf is `fQwertyKey->ShowQwertyKey(...)` (real since FW-QWKEY1), fed by
    //  `fTemp_Set->MaxTempSetting()/MinTempSetting()` (no port) and guarded by
    //  `Barcode_Reader(...)` (no port -- ⚠ 20260826 更正：真本體在
    //  BarcodeReader.cpp:445，這條不再是阻塞物；承重的是 Tag，見
    //  edinitialMouseDown 上方的 FW-SETUP-W19 註記).  ACTIVE arm: the Sender downcast above
    //  (real -- vclcompat TEdit:TCustomEdit:TControl:TObject is a single
    //  non-virtual chain) and then the faithful "user did nothing" no-op.
    //  Golden preserved VERBATIM, ready to un-gate as one block:
#if 0
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;
    dTempMax=fTemp_Set->MaxTempSetting();                                       //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    dTempMin=fTemp_Set->MinTempSetting();                                       //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    double fTemp = 0;                                                           //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    if(Buffer->Tag==1 && Tri_Temp_Machine!=1)                                   //20140320 wei //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }

    if(INSTALL_HEAT_GUN && (Buffer->Tag==tcHeatGun1 || Buffer->Tag==tcHeatGun2))                                        //kevin 20200101  hot gun
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iHeaterGunH , (double)InputLimit.iHeaterGunL);
    }
    else
    {
        if(Tri_Temp_Machine==1)                                                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        {
            if((Buffer->Tag>=tcDUT1 && Buffer->Tag<=tcDUT4) ||
               (Buffer->Tag>=tcOutSht1 && Buffer->Tag<=tcBase6) ||
               (Buffer->Tag>=tcDoor1 && Buffer->Tag<=tcDoor2) )                 //Ztex 2023.10.23 Add Index Door Heater
            {
                if(Buffer->Tag>=tcDUT1 && Buffer->Tag<=tcDUT4)
                {
                    if(SetHeaterTemp_MaxIndex - Temperature.iTriTempDefault_Ini[1] > 30)
                    {
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.00, 0.00);
                    }
                    else
                    {
                        fTemp = SetHeaterTemp_MaxIndex - Temperature.iTriTempDefault_Ini[1];
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, fTemp, 0.00);
                    }
                }
                else if(Buffer->Tag>=tcOutSht1 && Buffer->Tag<=tcOutSht2)
                {
                    if(SetHeaterTemp_MaxOutSht - Temperature.iTriTempDefault_Ini[0] > 30)
                    {
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.00, 0.00);
                    }
                    else
                    {
                        fTemp = SetHeaterTemp_MaxOutSht - Temperature.iTriTempDefault_Ini[0];
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, fTemp, 0.00);
                    }
                }
                else
                {
                    if(SetHeaterTemp_MaxBase - Temperature.iTriTempDefault_Ini[2] > 30)
                    {
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.00, 0.00);
                    }
                    else
                    {
                        fTemp = SetHeaterTemp_MaxBase - Temperature.iTriTempDefault_Ini[2];
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, fTemp, 0.00);
                    }
                }
            }
            else if(((Buffer->Tag>=tcAa1 && Buffer->Tag<=tcBd2) ||
                     (Buffer->Tag>=tcAe1 && Buffer->Tag<=tcBh2)) ||
                     (Buffer->Tag>=tcHotPlate1 && Buffer->Tag<=tcShuttle2) ||
                     (Buffer->Tag>=tcHotPlate3 && Buffer->Tag<=tcShuttle4))
            {
                if(Temperature.fWorkTemperBase>0)                               //Temperature > 0
                {
                    if((dTempMax - Temperature.fWorkTemperBase)>30)
                    {
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.00, -30.00);
                    }
                    else
                    {
                        fTemp = dTempMax - Temperature.fWorkTemperBase;
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, fTemp, (-1)*fTemp);
                    }
                }
                else                                                            //Temperature < 0
                {
                    if(abs(dTempMin - Temperature.fWorkTemperBase)>30)
                    {
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 30.00, -30.00);
                    }
                    else
                    {
                        fTemp = dTempMin - Temperature.fWorkTemperBase;
                        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, fTemp, (-1)*fTemp);
                    }
                }
            }
        }
        else
        {
            if(CosFunction.bOffsetTempByRecipeMinMaxLimit &&
               Sender==edOffset)
            {
                fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)Temperature.iOffsetByRecipeMaxLimit, (double)Temperature.iOffsetByRecipeMinLimit);
            }
            else
            {
                fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
            }
        }
    }
#endif
}
//---------------------------------------------------------------------------
//  golden MyTempPanel.cpp:528-532  --  TMyTempPanel::edLimitMouseDown
//AI(W906-FW-SIG-W16) 20260826: 收回成員並回填 golden 完整簽章。
void TMyTempPanel::edLimitMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W16): golden 這四支也沒讀這兩個
    //AI(W906-PT-W8) 20260811: GATE (W8-4) OPENED 20260824 -- golden's ENTIRE body is one
    //  ShowQwertyKey call, with hard-coded bounds 0.0 / 12.0.
    //  CORRECTION TO A CLAIM THIS COMMENT ORIGINALLY MADE, recorded rather
    //  than silently deleted: an earlier draft of this note called the
    //  argument pair a GOLDEN BUG ("max below min").  THAT WAS WRONG, and the
    //  check that disproved it is worth writing down, because the SAME
    //  reasoning applies to every other ShowQwertyKey site in this file.
    //  golden myQwertyKeyBoard.h:153 declares
    //     ShowQwertyKey(TWinControl*, int iFunction, int iDP=0,
    //                   bool bCheckRange=false, double min=0, double max=0)
    //  -- so the trailing pair is (min, max), NOT (max, min); and golden
    //  myQwertyKeyBoard.cpp:259-271 then NORMALISES it anyway
    //  (`if(max>min){...}else{ swap }`), so the pair is order-insensitive at
    //  the callee.  0.0/12.0 here, and the (High, Low) pairs the other three
    //  handlers pass, are all correct as written.  No bug; nothing to preserve
    //  or correct.
    //  Live since 20260824 (FW-QWKEY2); headless no-op until keyboard wiring.
    fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, 0.0, 12.0);
}
//---------------------------------------------------------------------------
//  golden MyTempPanel.cpp:534-560  --  TMyTempPanel::edIndiviMouseDown
//AI(W906-FW-SIG-W16) 20260826: 收回成員並回填 golden 完整簽章。
void TMyTempPanel::edIndiviMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W16): golden 這四支也沒讀這兩個
    TEdit *Buffer;                                                              //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    Buffer=(TEdit *)Sender;                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    //AI(W906-PT-W8) 20260811: GATES (W8-2)/(W8-3)/(W8-4) -- same three
    //  missing surfaces as edBaseMouseDown above (Tag / fTemp_Set /
    //  fQwertyKey).  ACTIVE arm: the real Sender downcast, then the faithful
    //  "user did nothing" no-op.  Golden preserved VERBATIM:
#if 0
    dTempMax=fTemp_Set->MaxTempSetting();                                       //Steven 20170427 : 回傳機台可以用的最大溫度值
    dTempMin=fTemp_Set->MinTempSetting();
    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        if(Buffer->Tag>=tcDUT1 && Buffer->Tag<=tcDUT4)
        {
            fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)SetHeaterTemp_MaxIndex, 20.00);
        }
        else if(Buffer->Tag>=tcOutSht1 && Buffer->Tag<=tcOutSht2)
        {
            fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)SetHeaterTemp_MaxOutSht, 20.00);
        }
        else
        {
            fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)SetHeaterTemp_MaxBase, 20.00);
        }
    }
    else
    {
        fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, dTempMax, dTempMin);
    }
#endif
}
//---------------------------------------------------------------------------
//  golden MyTempPanel.cpp:562-599  --  TMyTempPanel::edinitialMouseDown
//  golden :562 trailing comment: //kevin 20210421 獨立offset range
//AI(W906-FW-SIG-W16) 20260826: 收回成員並回填 golden 完整簽章。
void TMyTempPanel::edinitialMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Button; (void)Shift;   //AI(W906-FW-SIG-W16): golden 這四支也沒讀這兩個
    TEdit *Buffer;
    Buffer=(TEdit *)Sender;

    //AI(W906-PT-W8) 20260811: GATES (W8-2)/(W8-4)/(W8-5) -- `Buffer->Tag`
    //  (no Tag member), `fQwertyKey` (real since FW-QWKEY1) and `Barcode_Reader` (no port).
    //  ⚠ AI(W906-FW-SETUP-W19) 20260826 -- 上面那句「Barcode_Reader (no port)」
    //  已經過期（真本體在 BarcodeReader.cpp:445）。**但這個 gate 仍然不能開**，
    //  而且真正的阻塞物比原註記寫的更嚴重：
    //    `Tag` 在本樹沒有任何載入路徑（.dfm 的設計期值不會進來），恆為 0。
    //    `tcHeatGun1=27` / `tcHeatGun2=28`（MachineType.h:641）**都非零**，
    //    所以 `Tag==tcHeatGun1 || Tag==tcHeatGun2` 恆為 false ->
    //    **每一個 widget（含兩個熱風槍的）都會落到 else 臂**，
    //    跳出「非熱風槍」的限值鍵盤。那是靜默用錯限值，不是少做一件事。
    //  這正是「解 gate 前先查值從哪來」那條規則講的形狀：
    //  少一個前提死掉不代表可以開，要看**剩下的前提是不是承重的**。
    //  NOTE for whoever un-gates this: golden's CUSTOMER_CODE==CC_ASE_KaohSiung
    //  branch and its `else` branch differ ONLY in which InputLimit pair the
    //  non-heat-gun arm uses (iIlitialTempHigh/Low vs iTempHigh/Low); the
    //  heat-gun arm is byte-identical in both.  That duplication is golden's
    //  own and is preserved, not folded.  CUSTOMER_CODE, CC_ASE_KaohSiung,
    //  INSTALL_HEAT_GUN, tcHeatGun1/2 and the whole InputLimit struct are ALL
    //  REAL in this port -- ONLY Tag/Barcode_Reader are still missing
    //  (fQwertyKey real since FW-QWKEY1, 20260824), so this handler is the
    //  closest of the four to being un-gateable.
    //  ACTIVE arm: faithful "user did nothing" no-op.  Golden VERBATIM:
#if 0
    if(Buffer->Tag==1)
    {
        if(Barcode_Reader(bcTemperature)==0)                                    // 20140103 wei KYEC Barcode Reader
        {
            return;
        }
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(INSTALL_HEAT_GUN &&
           (Buffer->Tag==tcHeatGun1 ||Buffer->Tag==tcHeatGun2))                 //kevin 20200101  hot gun
        {
            fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iHeaterGunH , (double)InputLimit.iHeaterGunL);
        }
        else
        {
            fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iIlitialTempHigh, (double)InputLimit.iIlitialTempLow);
        }
    }
    else
    {
        if(INSTALL_HEAT_GUN &&
           (Buffer->Tag==tcHeatGun1 ||Buffer->Tag==tcHeatGun2))                 //kevin 20200101  hot gun
        {
            fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iHeaterGunH , (double)InputLimit.iHeaterGunL);
        }
        else
        {
            fQwertyKey->ShowQwertyKey((TEdit *)Sender, N_DOUBLE, 2, true, (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
        }
    }
#endif
}
//---------------------------------------------------------------------------
//  HAND-OFF TO THE INTEGRATING LOOP -- ✅ 已於 FW-SIG-W16（20260826）執行
//  （下面原文保留。實際做法比它建議的更忠實：簽章回填成 golden 完整版，
//   而不是這裡寫的簡化版 `(TObject *Sender, int X, int Y)`，因為
//   vclcompat/ShiftState.h 已經讓那兩個參數拼得出來。）
//  HAND-OFF TO THE INTEGRATING LOOP -- DESCRIBED, DELIBERATELY NOT DONE HERE
//  (MyTempPanel.h is outside this wave's write boundary.)
//
//  To collapse the four free functions above back into class members, add to
//  MyTempPanel.h's `private:` block, replacing the 4 commented-out golden
//  signatures PT-W3 left at MyTempPanel.h:111-114:
//
//      void edBaseMouseDown   (TObject *Sender, int X, int Y);
//      void edLimitMouseDown  (TObject *Sender, int X, int Y);
//      void edIndiviMouseDown (TObject *Sender, int X, int Y);
//      void edinitialMouseDown(TObject *Sender, int X, int Y);   //kevin 20210421 initial temp 獨立範圍值
//
//  then rename each definition above to `TMyTempPanel::<name>` and delete its
//  `TMyTempPanel *Self` parameter and the matching `(void)Self;`.  No other
//  edit is required -- no body above reads any PRIVATE member of the class
//  (`edOffset`, the one member golden's edBaseMouseDown compares `Sender`
//  against at golden :516, is PUBLIC in this port's MyTempPanel.h:151, and
//  that comparison is inside GATE W8-4 anyway).
//
//  Also worth doing at the same time, but NOT required for the above:
//  MyTempPanel.h's banner GATE (1) currently says these 4 are omitted because
//  their signature "cannot be spelled".  That premise is now superseded by
//  GATE (W8-1) here (the two unspellable parameters are provably dead in
//  golden).  Nothing in the header is WRONG today -- it is describing a state
//  this .cpp has just moved past.
//
//  RE-RUN LOG -- every absence claim in the GATE REGISTER above was re-run
//  from the tree root at 2026-08-11 11:45, AFTER this block was written and
//  after all sibling files in this wave had landed, and every one still
//  returned the same result recorded at its own gate.  See this wave's report
//  for the verbatim command list.
//---------------------------------------------------------------------------
