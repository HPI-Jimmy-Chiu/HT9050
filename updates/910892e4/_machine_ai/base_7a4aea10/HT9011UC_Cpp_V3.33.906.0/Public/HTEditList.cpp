// =============================================================================
//  Public/HTEditList.cpp  --  HTEditList + the HotPlate pick/place bookkeeping
//  family (uBasicPickPlace / uHPSuckTeam / uHPSuckGroup / uPlateInfo /
//  uPoint2D) method bodies.
//
//  Faithful translation of golden Public/HTEditList.cpp (3135 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 ("hteditlist" group).
//
//  ROLE: see Public/HTEditList.h banner. That header ALSO documents 3
//  INTEGRATION HAZARDS (pre-existing narrower TList/uPlateInfo/uHPSuckTeam
//  shims + already-defined PickFromHPList/PlaceToCleanList globals in
//  aHotPlateSubstrate.h/.cpp) that block adding this .cpp to any CMake target
//  that also compiles aHotPlateSubstrate.cpp until a future wave reconciles
//  them -- read that header banner first.
//
//  WAVE SCOPE -- every golden function, ACTIVE vs GATED vs NEVER-DEFINED-IN-
//  GOLDEN-EITHER (golden line numbers cited against the 3135-line original):
//
//  HTEditList class:
//    ACTIVE (real, faithful bodies):
//      HTEditList()                    golden :82-89
//      ~HTEditList()                   golden :91-103
//      Add(...)                        golden :105-348   (GATED interior, see GATE (1)/(2))
//      ClearALL()                      golden :558-562
//      SetBarcodeType(int)             golden :564-572
//      SaveEditTextToFile(...)         golden :574-1007  (GATED interior, see GATE (1)/(3)/(4))
//      ReadEditTextFromFile(...)       golden :1009-1321 (GATED interior, see GATE (1)/(3))
//      InitialDataToEdit()             golden :1323-1445 (GATED interior, see GATE (5))
//      ExportCustomLimitToCSV(...)     golden :1463-1534 (GATED interior, see GATE (3))
//      ImportCustomLimitationFromCSV() golden :1539-1586 (GATED interior, see GATE (1))
//      ReadCustomLimitationFromIni()   golden :1624-1664 (GATED interior, see GATE (1)/(3))
//      SetFontBlue()                   golden :1666-1669
//      ReadAndCompareDataFromFile(...) golden :1671-1743 (GATED interior, see GATE (6))
//      AddDiffData(...)                golden :1745-1752
//      SetItemLimit(...)               golden :1754-1805
//      AddInteger/AddInteGerP/AddIntegerN/AddDouble/AddDoubleP/AddDoubleN/
//        AddPercent                    golden :350-556 -- golden's OWN bodies
//        are 100% commented out (dead/retired code IN GOLDEN ITSELF, not a
//        translation gap). Mirrored as genuinely-empty functions, exactly
//        matching golden's own no-op behaviour; the retired logic is kept as
//        a comment for provenance, same as golden.
//    DECLARED IN golden HTEditList.h, NEVER DEFINED ANYWHERE IN golden
//    HTEditList.cpp (verified: grepped the full 3135-line file for
//    "SaveDefaultLimitationToIni"/"ReadDefaultLimitationFromIni"/
//    "SaveCustomLimitationToIni" -- zero hits for the first two, and the third
//    appears ONLY inside two `//`-commented blocks, golden :1590/:1809. This is
//    a genuine gap in golden itself: the real BCB6 project links fine only
//    because nothing anywhere calls these 3 methods.) Ported as documented
//    no-op stubs (ODR-safe placeholders for golden's own unimplemented
//    surface, not a gate against a missing PORT capability):
//      SaveDefaultLimitationToIni(AnsiString)
//      ReadDefaultLimitationFromIni(AnsiString)
//      SaveCustomLimitationToIni(AnsiString)
//
//  uBasicPickPlace: GetIntegerFromJSON/GetDoubleFromJSON/GetStringFromJSON/
//    DataForwardAndDelete -- golden :2133-2177, all ACTIVE (pure cJSON/TList
//    logic, no VCL dependency).
//
//  uHPSuckTeam (golden :2179-2434) -- ALL 16 methods ACTIVE, pure array/cJSON
//    logic, no VCL dependency: ctor()/ctor(5-arg)/dtor, GetUseSuckCol,
//    ClearHPSuckTeam, Clone, SaveJSONFile/LoadJSONFile, SavePlateRow/Col/Site/
//    Suck x4, ReadPlateRow/Col/Site/SuckFromJSON x4.
//
//  uHPSuckGroup (golden :2435-2552) -- ALL 13 methods ACTIVE: ctor/dtor,
//    ClearTeamList, AddHPSuckTeam x2, DelHPSuckTeam, ExtractLastTeam/
//    ExtractSuckTeam, GetTeamCount, DataForward, Clone, SaveJSONFile/
//    LoadJSONFile.
//
//  uPlateInfo (golden :2554-3116) -- ALL 33 methods ACTIVE except ResetFile
//    (GATED interior, see GATE (7)): ctor/dtor, GetHPSuckGroupCount,
//    SetTempSuckTeam/GetTempSuckTeam, AddHPSuckGroup x2, DelHPSuckGroup,
//    ClearGroupList, ExtractLastGroup/ExtractSuckGroup, ExtractFirstTeam/
//    ExtractLastTeam/ExtractLastTeamHasData/ExtractTeamByIndex,
//    SetArrPlateXY, UpdateHPSuckGroup, SearchHPSuckTeamByCoordinate,
//    SetPlateSuck, SaveFile, LoadFile, ResetFile (GATE 7),
//    DataForwardAndNextTeam, GetHPFirstTeamSuckUse/MotUse/SuckCol/Plate/
//    GetHPFirstTeam, GetHPTeamSuck, GetHPFirstTeamToList, InsertHPGroup,
//    TransferByuPlacePoint2DList, AddDataByPlacePoint2D.
//
//  uPoint2D (golden :3118-3135) -- ALL 3 methods ACTIVE. NOTE: golden declares
//    `class uPoint2D` itself at HTEditList.h:249-259; this port does NOT
//    redeclare it (cmydef.h already carries an identical W0-TAIL stub
//    declaration, per that file's own "bodies live in HTEditList.cpp (W3)"
//    comment) -- see HTEditList.h's INTEGRATION HAZARDS (C).
//
//  GATE REGISTER -- 7 gates. Every one exists because vclcompat/Controls.h's
//  widget stand-ins carry none of Parent/Name/Tag/Font/ShowHint/Hint
//  (verified: read the whole file; its own MEASURED PROPERTY COVERAGE note
//  lists exactly what IS modeled, and none of these are on it), Application
//  (TApplication) does not exist anywhere in the port tree (grepped 20260807),
//  and `class FileInfo` does not exist anywhere in the port tree either
//  (grepped 20260807; matches Public/MyStringList.cpp's own prior finding,
//  cited there).
//   (1) Application->MessageBoxA(...) (golden :621/664/718/773/997/1052/1186/
//       1229/1251/1312/1547/1632 -- 12 call sites across Add/SaveEditTextToFile/
//       ReadEditTextFromFile/ImportCustomLimitationFromCSV/
//       ReadCustomLimitationFromIni). No TApplication object exists anywhere
//       in this port. FAITHFUL DEFAULT: routed through one static helper,
//       HTEL_ShowAlarmBox(AnsiString), which calls the tree's existing
//       MyDBIProcess("WARNING", msg) diagnostic sink (aHotPlateSubstrate.cpp:
//       1030, the same sink this file's own catch(...) blocks already use) --
//       the message text is NOT silently dropped, it is logged instead of
//       popped up. BEHAVIOUR DELTA: no interactive dialog blocks the caller
//       offline (there is no window subsystem to block); every one of these
//       12 sites is additionally gated behind `bAlarmLimitation` (default
//       false, and grepped: nothing in this file ever sets it true), so today
//       none of them fire at all regardless of this gate.
//   (2) `Item->SourceControl->Tag=FEditList->Count-1;` (golden :149/272/298/
//       323/346, one per widget-type branch of Add()). RESTORED 20260825 by
//       AI(W906-FW-CFG-W4a). It had been dropped because vclcompat::TControl
//       carried no Tag; AI(W906-FW-TAG1) added one the same day
//       (Controls.h:255), so all five sites now write golden's own line and
//       the widget-side mirror is real again. cConfiguration's
//       ChangeCBListProperty indexes this list THROUGH the widget Tag, so the
//       mirror is load-bearing, not decorative. `Item->iTag` still carries the
//       same value alongside it, exactly as golden does.
//   (3) `Temp->SourceControl->Parent`/`->Name` (TWinControl* Parent, AnsiString
//       Name) -- golden :601-609/610/1032-1040/1042/1505-1509/1649-1650, used
//       to build a diagnostic "FormName::WidgetName" label and (in
//       ReadCustomLimitationFromIni) an actual ini section/key. Neither Parent
//       nor Name exists on vclcompat::TControl. FAITHFUL DEFAULT:
//       sParentName="NoParent" (literally golden's OWN existing fallback
//       string for the "no parent" case, golden :608/1039 -- reused here
//       because a parent can never be observed offline either way) and
//       sEditName=""/sGroup=""/sKey=""("" per no-Name-modeled-offline).
//       BEHAVIOUR DELTA: SaveEditTextToFile/ReadEditTextFromFile's diagnostic
//       labels are less specific (every widget looks like "NoParent::");
//       ReadCustomLimitationFromIni's per-widget custom-limit ini lookups all
//       collapse onto the same blank group/key, so that ADVANCED feature
//       (customer-editable per-widget min/max override) is inert until real
//       widget names exist -- its caller-visible contract (best-effort ini
//       read, defaults preserved on miss) still holds, just always misses.
//   (4) ExportCustomLimitToCSV's `Temp->SourceControl->Parent->Name` /
//       `Temp->SourceControl->Name` (golden :1505-1509, both already NULL-
//       guarded by golden itself). FAITHFUL DEFAULT: guard conditions
//       preserved verbatim; only the (missing) member access inside is
//       dropped, so slFormat's Form-Name/Edit-Name columns keep golden's own
//       already-initialised blank (" ") placeholder instead.
//   (5) InitialDataToEdit's bFontBlue styling block (golden :1403-1443):
//       `->Font->Color=clBlue; ->ShowHint=true; ->Hint="...";` on
//       TEdit/TLabeledEdit/TComboBox/TRadioGroup/TCheckBox/TDateTimePicker.
//       None of Font/ShowHint/Hint exist on any vclcompat widget stand-in
//       (Controls.h's own file-head note: Font is reached only via a NAMED
//       facade member like fMain->pnlCleanCountFont in ported forms, never as
//       a `.Font` member on the control itself). FAITHFUL DEFAULT: no-op --
//       bFontBlue is still real, settable storage (SetFontBlue() works), the
//       styling side effect is simply unobservable with no live widget to
//       paint.
//   (6) ReadAndCompareDataFromFile's `RdG->Caption` (golden :1726, one call
//       site) -- vclcompat::TRadioGroup carries ItemIndex+Items only, no
//       Caption (only TCheckBox/TLabel/TPanel/TButton/... have Caption in
//       Controls.h). FAITHFUL DEFAULT: AnsiString("") in its place (an empty
//       diff-row label; the two compared values i1/i2 themselves are
//       unaffected).
//   (7) uPlateInfo::ResetFile's `FileInfo().IsFilePathExist(sFileName)` (golden
//       :2825) -- `class FileInfo` does not exist anywhere in the port tree
//       (same finding Public/MyStringList.cpp already recorded for golden's
//       FileInfo::PathCombin). FAITHFUL SUBSTITUTION: vclcompat::FileExists(),
//       which is the exact same "does this path exist" predicate FileInfo's
//       real golden body (ProductionInfo/FileInfo.cpp) itself would have
//       delegated to -- not a behaviour change, just skipping the wrapper
//       class this port does not have.
//
//  ADAPTATIONS (not gates -- compile-compatibility fixes with NO behaviour
//  change, each isolated to one spot):
//   * `IncludeTrailingPathDelimiter(Path);` (golden :584/1019) is a
//     RETURN-VALUE-DISCARDING statement in golden -- real BCB6
//     IncludeTrailingPathDelimiter returns a NEW AnsiString and does not
//     mutate its argument, so this call is a literal no-op in golden too
//     (Path is used unmodified on the very next line, `Path+FileName`).
//     Mapped to vclcompat's `IncludeTrailingBackslash(Path);` with the SAME
//     discarded-return-value shape, preserving golden's (inert) behaviour
//     exactly rather than "fixing" it into `Path=IncludeTrailingBackslash(Path);`.
//   * SourceControl->Parent's old-config-migration snippet in Add() (golden
//     :165-171, `if(CEd->Parent!=NULL){ sOldData=ReadIniData(...,CEd->Parent->
//     Name,CEd->Name,...); ... }`) -- same missing-Parent/Name issue as GATE
//     (3)/(4), but scoped as a standalone adaptation note here because the
//     whole block is unconditionally skippable: golden's own guard
//     (`CEd->Parent!=NULL`) is what decides whether the migration runs at
//     all, and since Parent can never be observed offline, "never runs" IS
//     the guard's faithful evaluation -- so the block is simply omitted
//     (`if(false)`-equivalent), not a separate numbered gate.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"
#include "HTEditList.h"
#include "cMyDB.h"          // MyDBIProcess (aHotPlateSubstrate.cpp:1030) + RecordChangeLogProcess (cMyDB.cpp:1850)
#include "cpublic.h"        // ConvertDoubleStrToInt (cpublic.cpp:425) / ConvertIntStrToDouble (cpublic.cpp:443)
#include "common.h"         // AuthPath, ReadIniData/WriteIniData, MyForceDirectories, WriteDataToFile/ReadDataFromFile
#include "ProductionInfo/FileInfo.h"   //AI(W906-PT-W2-integrate) 20260807: class FileInfo for golden :2825 (GATE (7) retired)
//---------------------------------------------------------------------------
HTEditList *elConfig;                                                           //這個是可以從別的機台copy的
HTEditList *elUdUld;
HTEditList *elTrayForm;
HTEditList *elContact;
HTEditList *elLaser;
HTEditList *elConfig_byRecipe;                                                  //Sam 20220921 : config儲存跟隨recipe
HTEditList *cbLastSet;                                                          //這個是不可以從別的機台copy的
HTEditList *elVacuumUnit;                                                       //Sam 20230210 : 新增 VacuumUnit 通訊模組
HTEditList *cbConfig_byRecipe;                                              //Eastsun 20251216 : config 存讀 recipe
HTEditList *elTeach;

//Steven 20260924 (S12-C)：控制項 → HTML 元件名稱登記表（見 HTEditList.h 宣告處的說明）
#include <map>
static std::map<TControl*, AnsiString> &ControlNames()
{
    static std::map<TControl*, AnsiString> m;
    return m;
}
void HTEditList_RegisterControlName(TControl *Ctrl, const AnsiString &Name)
{
    if (Ctrl) ControlNames()[Ctrl] = Name;
}
AnsiString HTEditList_ControlName(TControl *Ctrl)
{
    std::map<TControl*, AnsiString>::const_iterator it = ControlNames().find(Ctrl);
    return it == ControlNames().end() ? AnsiString("") : it->second;
}

//Steven 20260924 (S12-C)：cprod.cpp 用的轉接（它不能 include 本檔的 header —— 與 aHotPlateSubstrate.h 的
//   TList／uPlateInfo 重複定義，見 cprod.cpp 開頭的說明）。放在本檔是因為 cprod.cpp 所在的 ht9045_globals 與
//   ctest 目標都連得到這裡。
bool HTEditList_ReadEditTextFromFile(HTEditList *el, AnsiString Path, AnsiString FileName)
{
    return el ? el->ReadEditTextFromFile(Path, FileName) : false;
}
bool HTEditList_SaveEditTextToFile(HTEditList *el, AnsiString Path, AnsiString FileName)
{
    return el ? el->SaveEditTextToFile(Path, FileName) : false;
}
void HTEditList_InitialDataToEdit(HTEditList *el)
{
    if (el) el->InitialDataToEdit();
}

uPlateInfo *PickFromHPList;
uPlateInfo *PlaceToHPList;

uPlateInfo *PlaceToCleanList;

// AI(W906-BA-MW6) 20260911: factory, so that uPlateInfo's ALLOCATION and its
// CONSTRUCTION both happen in **this** translation unit -- the one that sees the
// real class. Migrated from the D:\HT9050 tree's AI(W906-MW6) 20260910
// (commit 76158c0, which exists only in that repo; this tree still had the bug).
//
// uPlateInfo has two same-named definitions: aHotPlateSubstrate.h:673 is a
// zero-data-member facade (sizeof == 1); Public/HTEditList.h:321 is the real one
// (: public uBasicPickPlace + HPSuckGroupList/iLimitcount/TempSuckTeam,
// x64 sizeof == 24, 32-bit == 12). The ctor body is compiled against the real
// one, so any TU that can only see the facade and writes `new uPlateInfo()`
// emits operator new(1) and hands that single byte to a constructor that writes
// out to offset 23 -- 23 bytes out of bounds on x64, 11 on 32-bit.
// aHotPlateSubstrate.cpp:857-858 did exactly that, twice per boot: MW-6.
//
// This factory moves ONLY the allocation point; no class definition changes.
// The facade has no data members, so all you can do through it is call methods,
// and those were always correct -- the mangled names agree and the bodies are
// compiled against the real layout. Only the allocation-site sizeof was wrong.
uPlateInfo *W906_NewPlateInfo()
{
    return new uPlateInfo();
}

AnsiString sHPPickRec                   =AnsiString("d:\\HT9045\\system\\PickHPRec.json");
AnsiString sHPPickRecException          =AnsiString("d:\\HT9045\\system\\PickHPRecException.json");
AnsiString sHPPlaceRec                  =AnsiString("d:\\HT9045\\system\\PlaceHPRec.json");
AnsiString sCleanPlaceRec               =AnsiString("d:\\HT9045\\system\\PlaceCleanRec.json");
//---------------------------------------------------------------------------
//  GATE (1) helper -- see file banner. Every `Application->MessageBoxA(...)`
//  call site in golden routes through here.
//---------------------------------------------------------------------------
static void HTEL_ShowAlarmBox(const AnsiString &msg)
{
    MyDBIProcess("WARNING", msg);
}
//==============================================================================
//HTEditList
//==============================================================================
HTEditList::HTEditList()                                                        // golden :82-89
{
    FEditList=new TList;
    bAlarmLimitation=false;
    iDecimalPoint=6;                                                            //Steven 20200514 : 統一浮點數
    iBarcodeReadType=bcTotal;
    bFontBlue=false;                                                            //Sam 20220921 : config儲存跟隨recipe
}
//---------------------------------------------------------------------------
HTEditList::~HTEditList()                                                       // golden :91-103
{
    try
    {
        ClearALL();
        delete FEditList;
        FEditList=NULL;
    }
    catch(...)
    {
        MyDBIProcess("Exception", "~HTEditList");
    }
}
//---------------------------------------------------------------------------
void HTEditList::Add(TControl  *SoureCtrl,                                      // golden :105-348
                     void *Par,
                     TEditContent Content,
                     AnsiString GroupName,
                     AnsiString KeyName,
                     bool bVisible,
                     bool bEnable,
                     bool bReadFromFile,                                        //是否從檔案讀取
                     AnsiString DefValue,
                     bool bDisableEventOverlap,
                     AnsiString Min,
                     AnsiString Max,
                     int iTransform)
{
    TCustomEdit  *CEd = dynamic_cast <TCustomEdit *>(SoureCtrl);
    TComboBox    *Cbb = dynamic_cast <TComboBox   *>(SoureCtrl);
    TRadioGroup  *RdG = dynamic_cast <TRadioGroup *>(SoureCtrl);
    TCheckBox    *Chb = dynamic_cast <TCheckBox   *>(SoureCtrl);                //Steven 20230224 : Edit List改支援多型態元件
    TDateTimePicker *Ttp = dynamic_cast <TDateTimePicker   *>(SoureCtrl);
    AnsiString sPath=AuthPath+"config.ini";
    AnsiString sOldData="", sNewData="";
    if(Chb!=NULL)
    {
        THTEdit *Item           =new THTEdit;
        Item->Content           =ECBool;
        Item->SourceControl     =Chb;
        Item->ControlName       =HTEditList_ControlName(Chb);                     //Steven 20260924 (S12-C)：HTML 元件 id
        Item->DefaultValue      =DefValue;

        bool *SS;
        SS=(bool *)Par;
        Item->bParameter        =SS;
        Item->SetFileInformation(GroupName, KeyName);
        Chb->Checked            =*Item->bParameter;                             //Steven 20230502 : 修正bool沒有跟著存讀檔
        Item->GetDefaultPosition();
        Item->SetMax(AnsiString("1"));
        Item->SetMin(AnsiString("0"));
        Item->CheckRange        =true;
        FEditList->Add(Item);

        Item->iTransformType    =iTransform;
        Item->bVisible          =bVisible;
        Item->bEnable           =bEnable;
        Item->bReadFromFile     =bReadFromFile;
        Item->iTag              =FEditList->Count-1;
        // AI(W906-FW-CFG-W4a) 20260825: GATE (2) RETIRED. It said "TControl has
        // no Tag", which was true when it was written and stopped being true
        // EARLIER THE SAME DAY -- AI(W906-FW-TAG1) added `int Tag` to
        // vclcompat::TControl (Controls.h:255). The premise was invalidated by
        // this project's own change, which is why it survived that wave's sweep.
        //
        // Restoring it is not tidiness. cConfiguration's ChangeCBListProperty
        // (golden :266-363) looks its entries up as
        // `elConfig->FEditList->Items[cbA09->Tag]` -- through the WIDGET's Tag,
        // not through Item->iTag. With the mirror dropped every one of those 14
        // lookups reads Tag==0 and rewrites the properties of list item 0
        // instead of its own control. That is the "0 is not a neutral value"
        // failure the Tag substrate note warns about, and it would have been
        // silent: correct types, clean link, wrong widget.
        Item->SourceControl->Tag=FEditList->Count-1;                            //Steven 20230218 : Edit List改支援多型態元件
    }
    else if(CEd!=NULL)
    {
        THTEdit *Item           =new THTEdit;
        if(Content==ECText || Content==ECPassword)
        {
            Item->Content           =Content;
            Item->SourceControl     =CEd;
            Item->ControlName       =HTEditList_ControlName(CEd);                     //Steven 20260924 (S12-C)：HTML 元件 id
            Item->DefaultValue      =DefValue;

            Item->OnWriteEdit(CEd);
            AnsiString *SS;
            SS=(AnsiString *)Par;
            Item->sParameter=SS;
            Item->GetDefaultPosition();
            // ADAPTATION (see file banner): golden's old-config-migration
            // snippet here reads CEd->Parent->Name / CEd->Name (neither exists
            // on vclcompat::TCustomEdit/TControl); golden's own guard
            // (`CEd->Parent!=NULL`) can never be true offline either way, so
            // the block is omitted rather than gated.
            Item->SetFileInformation(GroupName, KeyName);                       //Jimmy 20230417 : Fixed for 存檔異常

            if(Max==AnsiString("") && Min==AnsiString(""))
            {
                Item->CheckRange=false;
            }
            else
            {
                Item->CheckRange=true;
                if(atof(Min.c_str())>atof(Max.c_str()))
                {
                    Item->SetMax(Min);
                    Item->SetMin(Max);
                }
                else
                {
                    Item->SetMax(Max);
                    Item->SetMin(Min);
                }
            }
            FEditList->Add(Item);
        }
        else
        {
            Item->Content           =Content;
            Item->SourceControl     =CEd;
            Item->ControlName       =HTEditList_ControlName(CEd);                     //Steven 20260924 (S12-C)：HTML 元件 id
            Item->DefaultValue      =DefValue;

            Item->OnWriteEdit(CEd);
            if(Content==ECText            ||    //純文字
               Content==ECFileName        ||    //檔案名稱或路徑
               Content==ECPassword)             //密碼
            {
                AnsiString *SS;
                SS=(AnsiString *)Par;
                Item->sParameter=SS;
            }
            else if(Content==ECInteger    ||    //數字
                    Content==ECPosInt     ||    //正整數
                    Content==ECNegInt     ||    //負數
                    Content==ECPort)            //通訊埠
            {
                int *SS;
                SS=(int *)Par;
                Item->iParameter=SS;
            }
            else if(Content==ECDouble     ||    //浮點數
                    Content==ECPosDouble  ||    //正浮點數
                    Content==ECNegDouble  ||    //負浮點數
                    Content==ECPercent)         //百分比
            {
                double *SS;
                SS=(double *)Par;
                Item->dParameter=SS;
            }
            else if(Content==ECBool)        //Steven 20230502 : 修正bool沒有跟著存讀檔
            {
                bool *SS;
                SS=(bool *)Par;
                Item->bParameter=SS;
            }

            Item->SetFileInformation(GroupName, KeyName);
            if(Content==ECPort)            //通訊埠
            {
                Item->SetMax(AnsiString("65535"));
                Item->SetMin(AnsiString("0"));
            }
            else if(Max==AnsiString("") && Min==AnsiString(""))
            {
                Item->CheckRange=false;
            }
            else
            {
                Item->CheckRange=true;
                if(atof(Min.c_str())>atof(Max.c_str()))
                {
                    Item->SetMax(Min);
                    Item->SetMin(Max);
                }
                else
                {
                    Item->SetMax(Max);
                    Item->SetMin(Min);
                }
            }
            Item->GetDefaultPosition();
            FEditList->Add(Item);
        }

        if(bDisableEventOverlap)
        {
            Item->DisableEventOverlap();
        }

        Item->bVisible          =bVisible;
        Item->bEnable           =bEnable;
        Item->bReadFromFile     =bReadFromFile;
        Item->iTransformType    =iTransform;
        Item->iTag              =FEditList->Count-1;
        // AI(W906-FW-CFG-W4a) 20260825: GATE (2) retired -- see the first site.
        Item->SourceControl->Tag=FEditList->Count-1;                            //Steven 20230218 : Edit List改支援多型態元件
    }
    else if(Cbb!=NULL)                                                          //Steven 20230224 : Edit List改支援多型態元件
    {
        THTEdit *Item       =new THTEdit;
        Item->Content       =ECPosInt;
        Item->SourceControl =Cbb;
        Item->ControlName       =HTEditList_ControlName(Cbb);                     //Steven 20260924 (S12-C)：HTML 元件 id
        Item->DefaultValue  =DefValue;

        int *SS;
        SS=(int *)Par;
        Item->iParameter=SS;

        Item->SetFileInformation(GroupName, KeyName);
        Item->SetMax(AnsiString(Cbb->Items->Count-1));
        Item->SetMin(AnsiString("0"));
        Item->CheckRange        =true;
        Cbb->ItemIndex          =*Item->iParameter;
        Item->GetDefaultPosition();
        FEditList->Add(Item);

        Item->bVisible          =bVisible;
        Item->bEnable           =bEnable;
        Item->bReadFromFile     =bReadFromFile;
        Item->iTransformType    =iTransform;
        Item->iTag              =FEditList->Count-1;
        // AI(W906-FW-CFG-W4a) 20260825: GATE (2) retired -- see the first site.
        Item->SourceControl->Tag=FEditList->Count-1;                            //Steven 20230218 : Edit List改支援多型態元件
    }
    else if(RdG!=NULL)                                                          //Steven 20230224 : Edit List改支援多型態元件
    {
        THTEdit *Item           =new THTEdit;
        Item->Content           =ECPosInt;
        Item->SourceControl     =RdG;
        Item->ControlName       =HTEditList_ControlName(RdG);                     //Steven 20260924 (S12-C)：HTML 元件 id
        Item->DefaultValue      =DefValue;

        int *SS;
        SS=(int *)Par;
        Item->iParameter=SS;
        Item->SetFileInformation(GroupName, KeyName);
        RdG->ItemIndex          =*Item->iParameter;
        // AI(W906-BA-FW1k) 20260911: `Count > 0` added so this expression keeps its
        //   CURRENT value now that TRadioGroup's ctor allocates Items.
        //   Verified in THIS tree: vclcompat/Controls.h's
        //   `TRadioGroup() : ItemIndex(0) { Items = new TStringList(); }` means
        //   `Items != NULL` is ALWAYS true, so an EMPTY list gave Count-1 == -1 --
        //   i.e. SetMax("-1") against the SetMin("0") on the next line with
        //   CheckRange=true two lines down: an inverted range.
        //   With this guard every case returns exactly what it returned before the
        //   ctor started allocating, so the edit is observably a pure crash fix and
        //   not a behaviour change.
        Item->SetMax(AnsiString(RdG->Items!=NULL && RdG->Items->Count > 0 ? RdG->Items->Count-1 : 0));
        Item->SetMin(AnsiString("0"));
        Item->CheckRange        =true;
        Item->GetDefaultPosition();
        FEditList->Add(Item);

        Item->bVisible          =bVisible;
        Item->bEnable           =bEnable;
        Item->bReadFromFile     =bReadFromFile;
        Item->iTransformType    =iTransform;
        Item->iTag              =FEditList->Count-1;
        // AI(W906-FW-CFG-W4a) 20260825: GATE (2) retired -- see the first site.
        Item->SourceControl->Tag=FEditList->Count-1;                            //Steven 20230218 : Edit List改支援多型態元件
    }
    else if(Ttp!=NULL)
    {
        THTEdit *Item           =new THTEdit;
        Item->Content           =Content;
        Item->SourceControl     =Ttp;
        Item->ControlName       =HTEditList_ControlName(Ttp);                     //Steven 20260924 (S12-C)：HTML 元件 id
        Item->DefaultValue      =DefValue;

        double *SS;
        SS=(double *)Par;
        Item->dParameter=SS;

        Item->SetFileInformation(GroupName, KeyName);
        SetItemLimit(Item,Content,Min,Max);
        Item->GetDefaultPosition();
        FEditList->Add(Item);

        Item->bVisible          =bVisible;
        Item->bEnable           =bEnable;
        Item->bReadFromFile     =bReadFromFile;
        Item->iTransformType    =iTransform;
        Item->iTag              =FEditList->Count-1;
        // AI(W906-FW-CFG-W4a) 20260825: GATE (2) retired -- see the first site.
        Item->SourceControl->Tag=FEditList->Count-1;                            //Steven 20230218 : Edit List改支援多型態元件
    }
}
//---------------------------------------------------------------------------
void HTEditList::AddInteger(TEdit * /*SoureEdit*/, int * /*Par*/, int /*Min*/, int /*Max*/, int /*DefValue*/, AnsiString /*GroupName*/, AnsiString /*KeyName*/)   // golden :350-382 -- golden's own body is 100% commented out (retired, not a translation gap)
{
//    if(SoureEdit!=NULL)
//    {
//        THTEdit *Item       =new THTEdit;
//        Item->Content       =ECInteger;
//        ... (golden body retired verbatim; see HTEditList.cpp golden :350-382)
//    }
}
//---------------------------------------------------------------------------
void HTEditList::AddInteGerP(TEdit * /*SoureEdit*/, int * /*Par*/, int /*Min*/, int /*Max*/, int /*DefValue*/, AnsiString /*GroupName*/, AnsiString /*KeyName*/)  // golden :384-410 -- retired, see above
{
}
//---------------------------------------------------------------------------
void HTEditList::AddIntegerN(TEdit * /*SoureEdit*/, int * /*Par*/, int /*Min*/, int /*Max*/, int /*DefValue*/, AnsiString /*GroupName*/, AnsiString /*KeyName*/)  // golden :412-438 -- retired, see above
{
}
//---------------------------------------------------------------------------
void HTEditList::AddDouble(TEdit * /*SoureEdit*/, double * /*Par*/, double /*Min*/, double /*Max*/, double /*DefValue*/, AnsiString /*GroupName*/, AnsiString /*KeyName*/)  // golden :440-472 -- retired, see above
{
}
//---------------------------------------------------------------------------
void HTEditList::AddDoubleP(TEdit * /*SoureEdit*/, double * /*Par*/, double /*Min*/, double /*Max*/, double /*DefValue*/, AnsiString /*GroupName*/, AnsiString /*KeyName*/) // golden :474-500 -- retired, see above
{
}
//---------------------------------------------------------------------------
void HTEditList::AddDoubleN(TEdit * /*SoureEdit*/, double * /*Par*/, double /*Min*/, double /*Max*/, double /*DefValue*/, AnsiString /*GroupName*/, AnsiString /*KeyName*/) // golden :502-528 -- retired, see above
{
}
//---------------------------------------------------------------------------
void HTEditList::AddPercent(TEdit * /*SoureEdit*/, double * /*Par*/, double /*Min*/, double /*Max*/, double /*DefValue*/, AnsiString /*GroupName*/, AnsiString /*KeyName*/) // golden :530-556 -- retired, see above
{
}
//---------------------------------------------------------------------------
void HTEditList::ClearALL()                                                     // golden :558-562
{
    if(FEditList!=NULL)
        FEditList->Clear();
}
//---------------------------------------------------------------------------
void HTEditList::SetBarcodeType(int iBarcodeType)                               // golden :564-572
{
    for(int i=0; i<FEditList->Count; i++)
    {
        THTEdit *Temp;
        Temp=(THTEdit*)FEditList->Items[i];
        Temp->iBarcodeReadType=iBarcodeType;
    }
}
//---------------------------------------------------------------------------
bool HTEditList::SaveEditTextToFile(AnsiString Path, AnsiString FileName)       // golden :574-1007
{
    bool bResult=true, bRet;
    AnsiString sParentName, sEditName;
    AnsiString Str4, Str5, Ret;
    int iCurr, iMax, iMin, iRet;
    double dCurr, dMax, dMin;
    TEditContent ContentType;
    MyForceDirectories(Path);

    IncludeTrailingBackslash(Path);                                             // ADAPTATION (see file banner): golden calls IncludeTrailingPathDelimiter(Path) and discards the return value -- Path is unchanged either way
    AnsiString sFullFileName=Path+FileName;

    TMemIniFile *ini;
    ini = new TMemIniFile(sFullFileName);

    AnsiString Str, Str2, Str3;
    map<AnsiString, AnsiString>mapKeyValue;
    map<AnsiString, AnsiString>::iterator iterKeyValue;

    mapKeyValue.clear();
    for(int i=0; i<FEditList->Count; i++)
    {
        bRet=false;
        THTEdit *Temp;
        Temp=(THTEdit*)FEditList->Items[i];

        // GATE (3): golden reads Temp->SourceControl->Parent->Name here;
        // vclcompat::TControl has no Parent/Name. Faithful default: golden's
        // own "no parent" fallback string (golden :608).
        sParentName="NoParent";
        sEditName  ="";
        ContentType=Temp->Content;

        if(Temp->IniGroupName=="" || Temp->IniKeyName=="")
        {
            if(bAlarmLimitation)
            {
                if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                    Str2.sprintf("Please check the group and key for Edit. @%s, [%s]->%s", sParentName, Temp->IniGroupName, Temp->IniGroupName);
                else
                    Str2.sprintf("Please check the group and key for Edit. %s->%s", sParentName, sEditName);
                HTEL_ShowAlarmBox(Str2);                                        // GATE (1)
            }
            bResult=false;
        }
        else
        {
            Str.sprintf("%s_%s", Temp->IniGroupName, Temp->IniKeyName);
            iterKeyValue=mapKeyValue.find(Str);

            if(iterKeyValue==mapKeyValue.end() || mapKeyValue.begin()==mapKeyValue.end())  //確認是不是不同元件在同一個位置存檔
            {
                iCurr=0, iMax=0, iMin=0;
                dCurr=0, dMax=0, dMin=0;
                Str2.sprintf("%s::%s", sParentName, sEditName);
                mapKeyValue[Str]=Str2;
                TCustomEdit *CEd = dynamic_cast <TCustomEdit *>(Temp->SourceControl);
                TComboBox   *Cbb = dynamic_cast <TComboBox   *>(Temp->SourceControl);
                TRadioGroup *RdG = dynamic_cast <TRadioGroup *>(Temp->SourceControl);
                TCheckBox   *Chb = dynamic_cast <TCheckBox   *>(Temp->SourceControl);   //Steven 20230224 : Edit List改支援多型態元件
                TDateTimePicker *Ttp = dynamic_cast <TDateTimePicker   *>(Temp->SourceControl);

                if(Temp->bReadFromFile==false)
                {
                    bRet=false;
                }
                else if(Temp->CheckRange)
                {
                    if(ContentType==ECBool)
                    {
                        iCurr=Chb->Checked;
                        if(iCurr==0 || iCurr==1)
                        {
                            bRet=true;
                        }
                        else
                        {
                            bResult=false;
                            if(bAlarmLimitation)
                            {
                                if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                                    Str2.sprintf("Please check the data range. @%s, [%s]->%s, max:1, min:0, current:%d", sParentName, Temp->IniGroupName, Temp->IniGroupName,  iCurr);
                                else
                                    Str2.sprintf("Please check the data range. %s->%s, max:1, min:0, current:%d", sParentName, sEditName, iCurr);
                                HTEL_ShowAlarmBox(Str2);                        // GATE (1)
                            }
                        }
                    }
                    else if(ContentType==ECInteger || ContentType==ECPosInt || ContentType==ECNegInt || ContentType==ECPort)        //數字
                    {
                        if(CEd!=NULL)
                        {
                            dCurr=atof(CEd->Text.c_str());
                            if(Temp->iTransformType==EUuMToMM)                  //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                            {
                                dCurr=(double)dCurr*(double)100.0;
                                iMax =atof(Temp->MaxValue.c_str())*(double)100.0;
                                iMin =atof(Temp->MinValue.c_str())*(double)100.0;
                            }
                            else if(Temp->iTransformType==EUMSToSec)            //顯示為Sec, 存檔為MS
                            {
                                dCurr=(double)dCurr*(double)1000.0;
                                iMax =atof(Temp->MaxValue.c_str())*(double)1000.0;
                                iMin =atof(Temp->MinValue.c_str())*(double)1000.0;
                            }
                            else
                            {
                                iMax =atoi(Temp->MaxValue.c_str());
                                iMin =atoi(Temp->MinValue.c_str());
                            }
                            iCurr=dCurr;
                        }
                        else if(Cbb!=NULL)
                        {
                            iCurr=Cbb->ItemIndex;
                            iMax =atoi(Temp->MaxValue.c_str());
                            iMin =atoi(Temp->MinValue.c_str());
                        }
                        else if(RdG!=NULL)
                        {
                            iCurr=RdG->ItemIndex;
                            iMax =atoi(Temp->MaxValue.c_str());
                            iMin =atoi(Temp->MinValue.c_str());
                        }

                        if(iCurr<=iMax && iCurr>=iMin)
                        {
                            bRet=true;
                        }
                        else
                        {
                            bResult=false;
                            if(bAlarmLimitation)
                            {
                                if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                                    Str2.sprintf("Please check the data range. @%s, [%s]->%s, max:%d, min:%d, current:%d", sParentName, Temp->IniGroupName, Temp->IniGroupName, iMax, iMin, iCurr);
                                else
                                    Str2.sprintf("Please check the data range. %s->%s, max:%d, min:%d, current:%d", sParentName, sEditName, iMax, iMin, iCurr);
                                HTEL_ShowAlarmBox(Str2);                        // GATE (1)
                            }
                        }
                    }
                    else if(ContentType==ECDouble || ContentType==ECPosDouble || ContentType==ECNegDouble || ContentType==ECPercent)         //浮點數
                    {
                        if(CEd!=NULL)
                        {
                            if(Temp->iTransformType==EUuMToMM)                  //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                            {
                                dCurr=atof(CEd->Text.c_str())*(double)100.0;
                                dMax =atof(Temp->MaxValue.c_str())*(double)100.0;
                                dMin =atof(Temp->MinValue.c_str())*(double)100.0;
                            }
                            else if(Temp->iTransformType==EUMSToSec)            //顯示為Sec, 存檔為MS
                            {
                                dCurr=atof(CEd->Text.c_str())*(double)1000.0;   //Jerry Yang 20240314 : iCurr --> dCurr
                                dMax =atof(Temp->MaxValue.c_str())*(double)1000.0;
                                dMin =atof(Temp->MinValue.c_str())*(double)1000.0;
                            }
                            else
                            {
                                dCurr=atof(CEd->Text.c_str());
                                dMax =atof(Temp->MaxValue.c_str());
                                dMin =atof(Temp->MinValue.c_str());
                            }

                            if(dCurr>=dMin && dCurr<=dMax)
                            {
                                if(iDecimalPoint<=1)                            //Steven 20200514 : 統一浮點數
                                    Str2=CEd->Text;
                                else if(iDecimalPoint==1)                       //Steven 20200514 : 統一浮點數
                                    Str2.sprintf("%0.1f", dCurr);
                                else if(iDecimalPoint==2)
                                    Str2.sprintf("%0.2f", dCurr);
                                else if(iDecimalPoint==3)
                                    Str2.sprintf("%0.3f", dCurr);
                                else if(iDecimalPoint==4)
                                    Str2.sprintf("%0.4f", dCurr);
                                else if(iDecimalPoint==5)
                                    Str2.sprintf("%0.5f", dCurr);
                                else
                                    Str2.sprintf("%0.6f", dCurr);
                                CEd->Text=Str2;
                                bRet=true;
                            }
                            else
                            {
                                bResult=false;
                                if(bAlarmLimitation)
                                {
                                    if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                                        Str2.sprintf("Please check the data range. @%s, [%s]->%s, max:%f, min:%f, current:%f", sParentName, Temp->IniGroupName, Temp->IniGroupName, dMax, dMin, dCurr);
                                    else
                                        Str2.sprintf("Please check the data range. %s->%s, max:%f, min:%f, current:%f", sParentName, sEditName, dMax, dMin, dCurr);
                                    HTEL_ShowAlarmBox(Str2);                    // GATE (1)
                                }
                            }
                        }
                        else if(Ttp!=NULL)
                        {
                            dCurr=Ttp->DateTime;
                            dMax =atof(Temp->MaxValue.c_str());
                            dMin =atof(Temp->MinValue.c_str());
                            if(dCurr>=dMin && dCurr<=dMax)
                            {
                                bRet=true;
                            }
                            else
                            {
                                bResult=false;
                                if(bAlarmLimitation)
                                {
                                    if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                                        Str2.sprintf("Please check the data range. @%s, [%s]->%s, max:%f, min:%f, current:%f", sParentName, Temp->IniGroupName, Temp->IniGroupName, dMax, dMin, dCurr);
                                    else
                                        Str2.sprintf("Please check the data range. %s->%s, max:%f, min:%f, current:%f", sParentName, sEditName, dMax, dMin, dCurr);
                                    HTEL_ShowAlarmBox(Str2);                    // GATE (1)
                                }
                            }
                        }
                    }
                }
                else
                {
                    if(ContentType==ECDouble || ContentType==ECPosDouble || ContentType==ECNegDouble || ContentType==ECPercent)         //Steven 20200514 : 統一浮點數
                    {
                        if(CEd!=NULL)
                        {
                            dCurr=atof(CEd->Text.c_str());
                            if(iDecimalPoint<=1)                                //Steven 20200514 : 統一浮點數
                                Str2=CEd->Text;
                            if(iDecimalPoint==1)
                                Str2.sprintf("%0.1f", dCurr);
                            else if(iDecimalPoint==2)
                                Str2.sprintf("%0.2f", dCurr);
                            else if(iDecimalPoint==3)
                                Str2.sprintf("%0.3f", dCurr);
                            else if(iDecimalPoint==4)
                                Str2.sprintf("%0.4f", dCurr);
                            else if(iDecimalPoint==5)
                                Str2.sprintf("%0.5f", dCurr);
                            else
                                Str2.sprintf("%0.6f", dCurr);
                            CEd->Text=Str2;
                        }
                    }
                    bRet=true;
                }

                if(bRet)
                {
                    if(Chb!=NULL)
                    {
                        //Steven 20200514 : 加上Change Log
                        //==>
                        Ret=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                        iRet=atoi(Ret.c_str());
                        if(iRet!=Chb->Checked)
                        {
                            Str4.sprintf("%s_%s change Value", Temp->IniGroupName , Temp->IniKeyName);
                            Str5.sprintf("%d==>%d", iRet, Chb->Checked);
                            RecordChangeLogProcess(Str4, Str5);       //wei 20180625 offset Change log紀錄
                        }
                        //<==
                        //Steven 20200514 : 加上Change Log
                        ini->WriteInteger(Temp->IniGroupName, Temp->IniKeyName, Chb->Checked?1:0);
                    }
                    else if(CEd!=NULL)
                    {
                        if(Temp->iTransformType==EUuMToMM)                      //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                        {
                            dCurr=atof(CEd->Text.c_str())*(double)100.0;
                            iCurr=dCurr;
                            Ret=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                            if(atoi(Ret.c_str())!=iCurr)
                            {
                                Str4.sprintf("%s_%s change Value", Temp->IniGroupName , Temp->IniKeyName);
                                Str5.sprintf("%s==>%d", Ret, iCurr);
                                RecordChangeLogProcess(Str4, Str5);             //wei 20180625 offset Change log紀錄
                            }

                            ini->WriteString(Temp->IniGroupName, Temp->IniKeyName, AnsiString(iCurr));
                        }
                        else if(Temp->iTransformType==EUMSToSec)                //顯示為Sec, 存檔為MS
                        {
                            dCurr=atof(CEd->Text.c_str())*(double)1000.0;
                            iCurr=dCurr;
                            Ret=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                            if(atoi(Ret.c_str())!=iCurr)
                            {
                                Str4.sprintf("%s_%s change Value", Temp->IniGroupName , Temp->IniKeyName);
                                Str5.sprintf("%s==>%d", Ret, iCurr);
                                RecordChangeLogProcess(Str4, Str5);             //wei 20180625 offset Change log紀錄
                            }
                            ini->WriteString(Temp->IniGroupName, Temp->IniKeyName, AnsiString(iCurr));
                        }
                        else
                        {
                            //Steven 20200514 : 加上Change Log
                            //==>
                            Ret=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                            if(Ret!=CEd->Text)
                            {
                                Str4.sprintf("%s_%s change Value", Temp->IniGroupName , Temp->IniKeyName);
                                Str5.sprintf("%s==>%s", Ret, CEd->Text);
                                RecordChangeLogProcess(Str4, Str5);             //wei 20180625 offset Change log紀錄
                            }
                            //<==
                            //Steven 20200514 : 加上Change Log
                            ini->WriteString(Temp->IniGroupName, Temp->IniKeyName, CEd->Text);
                        }
                    }
                    else if(Cbb!=NULL)
                    {
                        //Steven 20200514 : 加上Change Log
                        //==>
                        Ret=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                        iRet=atoi(Ret.c_str());
                        if(iRet!=Cbb->ItemIndex)
                        {
                            Str4.sprintf("%s_%s change Value", Temp->IniGroupName , Temp->IniKeyName);
                            Str5.sprintf("%d==>%d", iRet, Cbb->ItemIndex);
                            RecordChangeLogProcess(Str4, Str5);       //wei 20180625 offset Change log紀錄
                        }
                        //<==
                        //Steven 20200514 : 加上Change Log
                        ini->WriteInteger(Temp->IniGroupName, Temp->IniKeyName, Cbb->ItemIndex);
                    }
                    else if(RdG!=NULL)
                    {
                        //Steven 20200514 : 加上Change Log
                        //==>
                        Ret=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                        iRet=atoi(Ret.c_str());
                        if(iRet!=RdG->ItemIndex)
                        {
                            Str4.sprintf("%s_%s change Value", Temp->IniGroupName , Temp->IniKeyName);
                            Str5.sprintf("%d==>%d", iRet, RdG->ItemIndex);
                            RecordChangeLogProcess(Str4, Str5);                 //wei 20180625 offset Change log紀錄
                        }
                        //<==
                        //Steven 20200514 : 加上Change Log
                        ini->WriteInteger(Temp->IniGroupName, Temp->IniKeyName, RdG->ItemIndex);
                    }
                    else if(Ttp!=NULL)
                    {
                        AnsiString sNowStr=FloatToStr(Ttp->DateTime);
                        Ret=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                        if(Ret!=sNowStr)
                        {
                            Str4.sprintf("%s_%s change Value", Temp->IniGroupName , Temp->IniKeyName);
                            Str5.sprintf("%s==>%s", Ret, sNowStr);
                            RecordChangeLogProcess(Str4, Str5);             //wei 20180625 offset Change log紀錄
                        }
                        ini->WriteString(Temp->IniGroupName, Temp->IniKeyName, sNowStr);
                    }

                    if(ContentType==ECBool)
                    {
                        if(Chb!=NULL)
                            *Temp->bParameter=Chb->Checked;                     //Steven 20230502 : 修正bool沒有跟著存讀檔
                    }
                    else if(ContentType==ECInteger || ContentType==ECPosInt || ContentType==ECNegInt || ContentType==ECPort)
                    {
                        if(Temp->iParameter!=NULL)
                        {
                            if(CEd!=NULL)
                            {
                                if(Temp->iTransformType==EUuMToMM)              //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                                    dCurr=atof(CEd->Text.c_str())*(double)100.0;
                                else if(Temp->iTransformType==EUMSToSec)        //顯示為Sec, 存檔為MS
                                    dCurr=atof(CEd->Text.c_str())*(double)1000.0;
                                else
                                    dCurr=atoi(CEd->Text.c_str());
                                iCurr=dCurr;
                                *Temp->iParameter=iCurr;
                            }
                            else if(Cbb!=NULL)
                            {
                                *Temp->iParameter=Cbb->ItemIndex;
                            }
                            else if(RdG!=NULL)
                            {
                                *Temp->iParameter=RdG->ItemIndex;
                            }
                        }
                    }
                    else if(ContentType==ECDouble || ContentType==ECPosDouble || ContentType==ECNegDouble || ContentType==ECPercent)
                    {
                        if(Temp->dParameter!=NULL && CEd!=NULL)
                        {
                            if(Temp->iTransformType==EUuMToMM)                  //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                                dCurr=atof(CEd->Text.c_str())*(double)100.0;
                            else if(Temp->iTransformType==EUMSToSec)            //顯示為Sec, 存檔為MS
                                dCurr=atof(CEd->Text.c_str())*(double)1000.0;
                            else
                                dCurr=atof(CEd->Text.c_str());
                            *Temp->dParameter=dCurr;
                        }
                        else if(Temp->dParameter!=NULL && Ttp!=NULL)
                        {
                            *Temp->dParameter=Ttp->DateTime;
                        }
                    }
                    else
                    {
                        if(CEd!=NULL)
                            *Temp->sParameter=CEd->Text;
                    }
                }
            }
            else
            {
                bResult=false;
                Str2=mapKeyValue[Str];
                if(bAlarmLimitation)
                {
                    Str3.sprintf("%s %s got duplicate save position with %s::%s!!", Str, Str2, sParentName, sEditName);
                    HTEL_ShowAlarmBox(Str3);                                    // GATE (1)
                }
            }
        }
    }

    ini->UpdateFile();
    delete ini;

    return bResult;
}
//---------------------------------------------------------------------------
bool HTEditList::ReadEditTextFromFile(AnsiString Path, AnsiString FileName)     // golden :1009-1321
{
    bool bResult=true;
    AnsiString Str, Str2, Str3;
    AnsiString Ret;
    AnsiString sParentName, sEditName;
    int iCurr=0, iMax=0, iMin=0, iDef=0;
    double dCurr=0, dMax=0, dMin=0;
    TEditContent ContentType;

    IncludeTrailingBackslash(Path);                                             // ADAPTATION -- see SaveEditTextToFile's own note above
    AnsiString sFullFileName=Path+FileName;

    TMemIniFile *ini;
    ini = new TMemIniFile(sFullFileName);
    map<AnsiString, AnsiString>mapKeyValue;
    map<AnsiString, AnsiString>::iterator iterKeyValue;

    mapKeyValue.clear();
    for(int i=0; i<FEditList->Count; i++)
    {
        THTEdit *Temp;
        Temp=(THTEdit*)FEditList->Items[i];
        // GATE (3): see SaveEditTextToFile's own note above.
        sParentName="NoParent";
        ContentType=Temp->Content;
        sEditName  ="";

        if(Temp->IniGroupName=="" || Temp->IniKeyName=="")
        {
            if(bAlarmLimitation)
            {
                if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                    Str2.sprintf("Please check the group and key for Edit. @%s, [%s]->%s", sParentName, Temp->IniGroupName, Temp->IniGroupName);
                else
                    Str2.sprintf("Please check the group and key for Edit. %s->%s", sParentName, sEditName);
                HTEL_ShowAlarmBox(Str2);                                        // GATE (1)
            }
        }
        else
        {
            Str.sprintf("%s_%s", Temp->IniGroupName, Temp->IniKeyName);
            iterKeyValue=mapKeyValue.find(Str);
            if(iterKeyValue==mapKeyValue.end() || mapKeyValue.begin()==mapKeyValue.end())  //確認是不是不同元件在同一個位置存檔
            {
                iCurr=0, iMax=0, iMin=0, iDef=0;
                dCurr=0, dMax=0, dMin=0;
                TCustomEdit *CEd = dynamic_cast <TCustomEdit *>(Temp->SourceControl);
                TComboBox   *Cbb = dynamic_cast <TComboBox   *>(Temp->SourceControl);
                TRadioGroup *RdG = dynamic_cast <TRadioGroup *>(Temp->SourceControl);
                TCheckBox   *Chb = dynamic_cast <TCheckBox   *>(Temp->SourceControl);
                TDateTimePicker *Ttp = dynamic_cast <TDateTimePicker   *>(Temp->SourceControl);
                Str2.sprintf("%s::%s", sParentName, sEditName);
                mapKeyValue[Str]=Str2;

                if(Temp->bReadFromFile==false)
                {
                    if(CEd!=NULL)
                    {
                        CEd->Text=Temp->DefaultValue;
                    }
                    else if(Cbb!=NULL)
                    {
                        Cbb->ItemIndex=atoi(Temp->DefaultValue.c_str());
                    }
                    else if(RdG!=NULL)
                    {
                        RdG->ItemIndex=atoi(Temp->DefaultValue.c_str());
                    }
                    else if(Chb!=NULL)
                    {
                        Chb->Checked=(atoi(Temp->DefaultValue.c_str())==1)?true:false;  //Steven 20230502 : 修正bool沒有跟著存讀檔
                    }
                }
                else
                {
                    if(CEd!=NULL)
                    {
                        Str=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                        if(ContentType==ECInteger || ContentType==ECPosInt || ContentType==ECNegInt || ContentType==ECPort)        //數字
                        {
                            dCurr=atof(Str.c_str());
                            if(Temp->iTransformType==EUuMToMM)                  //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                                Str.sprintf("%0.3f", (double)dCurr/(double)100.0);
                            else if(Temp->iTransformType==EUMSToSec)            //顯示為Sec, 存檔為MS
                                Str.sprintf("%0.3f", (double)dCurr/(double)1000.0);
                        }
                        else if(ContentType==ECDouble || ContentType==ECPosDouble || ContentType==ECNegDouble || ContentType==ECPercent)    //浮點數
                        {
                            dCurr=atof(Str.c_str());
                            if(Temp->iTransformType==EUuMToMM)                  //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                                Str.sprintf("%0.3f", (double)dCurr/(double)100.0);
                            else if(Temp->iTransformType==EUMSToSec)            //顯示為Sec, 存檔為MS
                                Str.sprintf("%0.3f", (double)dCurr/(double)1000.0);
//                            else                                              //Steven 20240614 : Mark
//                                Str.sprintf("%0.3f", dCurr);
                        }
                        CEd->Text=Str;
                    }
                    else if(Cbb!=NULL)
                    {
                        iDef=atoi(Temp->DefaultValue.c_str());
                        Cbb->ItemIndex=ini->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, iDef);
                    }
                    else if(RdG!=NULL)
                    {
                        iDef=atoi(Temp->DefaultValue.c_str());
                        RdG->ItemIndex=ini->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, iDef);
                    }
                    else if(Chb!=NULL)
                    {
                        iDef=atoi(Temp->DefaultValue.c_str());
                        iDef=ini->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, iDef);
                        Chb->Checked=(iDef==1)?true:false;                      //Steven 20230502 : fixed for EditList
                    }
                    else if(Ttp!=NULL)
                    {
                        Str=ini->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                        dCurr=atof(Str.c_str());
                        Ttp->DateTime=dCurr;
                    }
                }

                if(Temp->CheckRange)
                {
                    if(ContentType==ECInteger || ContentType==ECPosInt || ContentType==ECNegInt || ContentType==ECPort)        //數字
                    {
                        if(Temp->iTransformType==EUuMToMM)                      //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                        {
                            if(CEd!=NULL)
                            {
                                iCurr=ConvertDoubleStrToInt(CEd->Text, 100.0);
                                iMax =ConvertDoubleStrToInt(Temp->MaxValue, 100.0);
                                iMin =ConvertDoubleStrToInt(Temp->MinValue, 100.0);
                            }
                        }
                        else if(Temp->iTransformType==EUMSToSec)                //顯示為Sec, 存檔為MS
                        {
                            if(CEd!=NULL)
                            {
                                iCurr=ConvertDoubleStrToInt(CEd->Text, 1000.0);
                                iMax =ConvertDoubleStrToInt(Temp->MaxValue, 1000.0);
                                iMin =ConvertDoubleStrToInt(Temp->MinValue, 1000.0);
                            }
                        }
                        else
                        {
                            if(CEd!=NULL)
                                iCurr=atoi(CEd->Text.c_str());
                            else if(Cbb!=NULL)
                                iCurr=Cbb->ItemIndex;
                            else if(RdG!=NULL)
                                iCurr=RdG->ItemIndex;
                            iMax =atoi(Temp->MaxValue.c_str());
                            iMin =atoi(Temp->MinValue.c_str());
                        }

                        if(iCurr<=iMax && iCurr>=iMin)
                        {
                            *Temp->iParameter=iCurr;
                        }
                        else
                        {
                            bResult=false;
                            if(bAlarmLimitation)
                            {
                                if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                                    Str2.sprintf("Please check the data range. @%s, [%s]->%s, max:%d, min:%d, current:%d", sParentName, Temp->IniGroupName, Temp->IniGroupName, iMax, iMin, iCurr);
                                else
                                    Str2.sprintf("Please check the data range. %s->%s, max:%d, min:%d, current:%d", sParentName, sEditName, iMax, iMin, iCurr);
                                HTEL_ShowAlarmBox(Str2);                        // GATE (1)
                            }
                        }
                    }
                    else if(ContentType==ECDouble || ContentType==ECPosDouble || ContentType==ECNegDouble || ContentType==ECPercent)    //浮點數
                    {
                        if(CEd!=NULL)
                        {
                            dCurr=atof(CEd->Text.c_str());
                            if(Temp->iTransformType==EUuMToMM)                      //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                            {
                                dMax =atof(Temp->MaxValue.c_str())*(double)100.0;
                                dMin =atof(Temp->MinValue.c_str())*(double)100.0;
                                dCurr=(double)dCurr*(double)100.0;
                            }
                            else if(Temp->iTransformType==EUMSToSec)                //顯示為Sec, 存檔為MS
                            {
                                dMax =atof(Temp->MaxValue.c_str())*(double)1000.0;
                                dMin =atof(Temp->MinValue.c_str())*(double)1000.0;
                                dCurr=(double)dCurr*(double)1000.0;
                            }
                            else
                            {
                                dMax =atof(Temp->MaxValue.c_str());
                                dMin =atof(Temp->MinValue.c_str());
                            }

                            if(dCurr<=dMax && dCurr>=dMin)
                            {
                                Str2.sprintf("%0.3f", dCurr);                       //小數點最少顯示三位
                                if(CEd!=NULL)
                                    CEd->Text=Str2;
                                *Temp->dParameter=dCurr;
                            }
                            else
                            {
                                bResult=false;
                                if(bAlarmLimitation)
                                {
                                    if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                                        Str2.sprintf("Please check the data range. @%s, [%s]->%s, max:%f, min:%f, current:%f", sParentName, Temp->IniGroupName, Temp->IniGroupName, dMax, dMin, dCurr);
                                    else
                                        Str2.sprintf("Please check the data range. %s->%s, max:%f, min:%f, current:%f", sParentName, sEditName, dMax, dMin, dCurr);
                                    HTEL_ShowAlarmBox(Str2);                    // GATE (1)
                                }
                            }
                        }
                        else if(Ttp!=NULL)
                        {
                            dCurr=Ttp->DateTime;
                            dMax =atof(Temp->MaxValue.c_str());
                            dMin =atof(Temp->MinValue.c_str());
                            if(dCurr<=dMax && dCurr>=dMin)
                            {
                                *Temp->dParameter=dCurr;
                            }
                            else
                            {
                                bResult=false;
                                if(bAlarmLimitation)
                                {
                                    if(Temp->IniGroupName!="" && Temp->IniGroupName!="")
                                        Str2.sprintf("Please check the data range. @%s, [%s]->%s, max:%f, min:%f, current:%f", sParentName, Temp->IniGroupName, Temp->IniGroupName, dMax, dMin, dCurr);
                                    else
                                        Str2.sprintf("Please check the data range. %s->%s, max:%f, min:%f, current:%f", sParentName, sEditName, dMax, dMin, dCurr);
                                    HTEL_ShowAlarmBox(Str2);                    // GATE (1)
                                }
                            }
                        }
                    }
                    else if(ContentType==ECBool)
                    {
                        if(CEd!=NULL)                                           //Steven 20230502 : 修正bool沒有跟著存讀檔
                            *Temp->bParameter=(atoi(CEd->Text.c_str())==0)?false:true;
                        else if(Cbb!=NULL)
                            *Temp->bParameter=(Cbb->ItemIndex==0)?false:true;
                        else if(RdG!=NULL)
                            *Temp->bParameter=(RdG->ItemIndex==0)?false:true;
                        else if(Chb!=NULL)
                            *Temp->bParameter=Chb->Checked;
                    }
                    else
                    {
                        if(CEd!=NULL)
                            *Temp->sParameter=CEd->Text;
                    }
                }
                else
                {
                    if(ContentType==ECInteger || ContentType==ECPosInt || ContentType==ECNegInt || ContentType==ECPort)
                    {
                        if(CEd!=NULL)
                            *Temp->iParameter=atoi(CEd->Text.c_str());
                    }
                    else if(ContentType==ECDouble || ContentType==ECPosDouble || ContentType==ECNegDouble || ContentType==ECPercent)    //浮點數
                    {
                        if(CEd!=NULL)
                            *Temp->dParameter=atof(CEd->Text.c_str());
                        else if(Ttp!=NULL)
                            *Temp->dParameter=Ttp->DateTime;
                    }
                    else if(ContentType==ECBool)
                    {
                        if(CEd!=NULL)                                           //Steven 20230502 : 修正bool沒有跟著存讀檔
                            *Temp->bParameter=(atoi(CEd->Text.c_str())==0)?false:true;
                        else if(Cbb!=NULL)
                            *Temp->bParameter=(Cbb->ItemIndex==0)?false:true;
                        else if(RdG!=NULL)
                            *Temp->bParameter=(RdG->ItemIndex==0)?false:true;
                        else if(Chb!=NULL)
                            *Temp->bParameter=Chb->Checked;
                    }
                    else
                    {
                        if(CEd!=NULL)
                            *Temp->sParameter=CEd->Text;
                    }
                }
            }
            else
            {
                bResult=false;
                if(bAlarmLimitation)
                {
                    Str2=mapKeyValue[Str];
                    Str3.sprintf("%s %s got duplicate read position with %s::%s!!", Str, Str2, sParentName, sEditName);
                    HTEL_ShowAlarmBox(Str3);                                    // GATE (1)
                }
            }
        }
    }

    ini->UpdateFile();
    delete ini;
    return bResult;
}
//---------------------------------------------------------------------------
void HTEditList::InitialDataToEdit()                                            // golden :1323-1445
{
    double dCurr;
    AnsiString Str;
    TEditContent ContentType;
    for(int i=0; i<FEditList->Count; i++)
    {
        THTEdit *Temp;
        Temp=(THTEdit*)FEditList->Items[i];

        ContentType=Temp->Content;
        TCustomEdit *CEd = dynamic_cast <TCustomEdit *>(Temp->SourceControl);
        TComboBox   *Cbb = dynamic_cast <TComboBox   *>(Temp->SourceControl);
        TRadioGroup *RdG = dynamic_cast <TRadioGroup *>(Temp->SourceControl);
        TCheckBox   *Chb = dynamic_cast <TCheckBox   *>(Temp->SourceControl);   //Steven 20230224 : Edit List改支援多型態元件
        TDateTimePicker *Ttp = dynamic_cast <TDateTimePicker   *>(Temp->SourceControl);

        if(ContentType==ECInteger || ContentType==ECPosInt || ContentType==ECNegInt || ContentType==ECPort)        //數字
        {
            if(CEd!=NULL)
            {
                if(Temp->iTransformType==EUuMToMM)                              //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                {
                    Str=(AnsiString)*Temp->iParameter;
                    dCurr=ConvertIntStrToDouble(Str, 100.0);
                    Str.sprintf("%0.3f", dCurr);
                }
                else if(Temp->iTransformType==EUMSToSec)                        //顯示為Sec, 存檔為MS
                {
                    Str=(AnsiString)*Temp->iParameter;
                    dCurr=ConvertIntStrToDouble(Str, 1000.0);
                    Str.sprintf("%0.3f", dCurr);
                }
                else
                {
                    Str=AnsiString(*Temp->iParameter);
                }
                CEd->Text=AnsiString(Str);
            }
            else if(Cbb!=NULL)
            {
                Cbb->ItemIndex=(*Temp->iParameter);
            }
            else if(RdG!=NULL)
            {
                RdG->ItemIndex=(*Temp->iParameter);
            }
        }
        else if(ContentType==ECDouble || ContentType==ECPosDouble || ContentType==ECNegDouble || ContentType==ECPercent)    //浮點數
        {
            if(CEd!=NULL)
            {
                if(Temp->iTransformType==EUuMToMM)                              //顯示為MM, 存檔為uM //Steven 20230905 : HTEditList加入型態轉換的Flag
                    Str.sprintf("%0.3f", (double)*Temp->dParameter/(double)100.0);
                else if(Temp->iTransformType==EUMSToSec)                        //顯示為Sec, 存檔為MS
                    Str.sprintf("%0.3f", (double)*Temp->dParameter/(double)1000.0);
                else
                    Str=AnsiString(*Temp->dParameter);

                CEd->Text=Str;
            }
            else if(Ttp!=NULL)
            {
                Ttp->DateTime=*Temp->dParameter;
            }
        }
        else if(ContentType==ECBool)
        {
            if(Chb!=NULL)
                Chb->Checked=(*Temp->bParameter);                               //Steven 20230502 : 修正bool沒有跟著存讀檔
        }
        else
        {
            if(CEd!=NULL)
                CEd->Text=AnsiString(*Temp->sParameter);
        }

        Temp->SourceControl->Visible=Temp->bVisible;
        Temp->SourceControl->Enabled=Temp->bEnable;

        // GATE (5): golden's bFontBlue styling block (Font->Color/ShowHint/
        // Hint on Ed/LEd/Cbb/RdG/Chb/Ttp) is unconditionally no-op'd here --
        // none of those 3 members exist on any vclcompat widget stand-in.
        // bFontBlue itself is still real, settable storage (SetFontBlue()).
        (void)Cbb; (void)RdG; (void)Chb; (void)Ttp;
    }
}
//---------------------------------------------------------------------------
enum eCustomLimit
{
    eFormName   =0,
    eEditName   =1,
    eDataType   =2,
    eDefaultMin =3,
    eMinimum    =4,
    eDefaultMax =5,
    eMaxmum     =6,

    eCustomLimitTotal
};
//---------------------------------------------------------------------------
//把元件設定存成CSV, 方便客戶使用Excel進行編輯
//FileName由外部的 SaveFileDialog去取得
//---------------------------------------------------------------------------
void HTEditList::ExportCustomLimitToCSV(AnsiString CsvFileName)                 // golden :1463-1534
{
    TStringList *sList;
    sList=new TStringList();
    sList->Clear();

    TStringList *slFormat;
    slFormat=new TStringList();
    slFormat->Clear();

    //第一行的標題列-------------------
    for(int j=0; j<eCustomLimitTotal; j++)                                      //預設填入空字串
    {
        slFormat->Add(" ");
    }
    slFormat->Strings[eFormName]    ="Form Name";
    slFormat->Strings[eEditName]    ="Edit Name";
    slFormat->Strings[eDataType]    ="Data Type";
    slFormat->Strings[eDefaultMin]  ="Default Minmum Value";
    slFormat->Strings[eDefaultMax]  ="Default Maxmum Value";
    slFormat->Strings[eMinimum]     ="Custom Minmum Value";
    slFormat->Strings[eMaxmum]      ="Custom Maxmum Value";
    //第一行的標題列-------------------

    for(int i=0; i<FEditList->Count; i++)
    {
        THTEdit *Temp;
        Temp=(THTEdit*)FEditList->Items[i];

        if(Temp->Content==ECInteger    ||           //只存入有數值的
           Temp->Content==ECDouble     ||
           Temp->Content==ECPosInt     ||
           Temp->Content==ECPosDouble  ||
           Temp->Content==ECNegInt     ||
           Temp->Content==ECNegDouble)
        {
            slFormat->Clear();
            for(int j=0; j<eCustomLimitTotal; j++)  //預設填入空字串
            {
                slFormat->Add(" ");
            }

            // GATE (4): golden reads Temp->SourceControl->Parent->Name /
            // ->Name here (both already NULL-guarded by golden itself);
            // neither Parent nor Name exists on vclcompat::TControl, so both
            // guarded assignments are dropped and slFormat keeps its
            // already-initialised " " placeholder for these two columns.

            switch(Temp->Content)                   //註記資料的型態
            {
                case ECText:        slFormat->Strings[eDataType]="Text";             break;
                case ECInteger:     slFormat->Strings[eDataType]="Integer";          break;
                case ECDouble:      slFormat->Strings[eDataType]="Double";           break;
                case ECPosInt:      slFormat->Strings[eDataType]="Postive Integer";  break;
                case ECPosDouble:   slFormat->Strings[eDataType]="Postive Double";   break;
                case ECNegInt:      slFormat->Strings[eDataType]="Negtive Integer";  break;
                case ECNegDouble:   slFormat->Strings[eDataType]="Negtive Double";   break;
                case ECFileName:    slFormat->Strings[eDataType]="FileName";         break;
                case ECPassword:    slFormat->Strings[eDataType]="Password";         break;
                default: break;
            }

            slFormat->Strings[eDefaultMin]  =Temp->MinValue;
            slFormat->Strings[eDefaultMax]  =Temp->MaxValue;
            slFormat->Strings[eMinimum]     =Temp->CustomMaxValue;
            slFormat->Strings[eMaxmum]      =Temp->CustomMinValue;
            sList->Add(slFormat->CommaText);
        }
    }
    sList->SaveToFile(CsvFileName);
    delete sList;
    delete slFormat;
}
//---------------------------------------------------------------------------
//把元件設定從CSV轉成INI, 方便客戶使用Excel進行編輯完之後, 放到機台上使用
//FileName由外部的 OpenFileDialog去取得
//---------------------------------------------------------------------------
void HTEditList::ImportCustomLimitationFromCSV(AnsiString CSVFileName, AnsiString IniFileName)  // golden :1539-1586
{
    TMemIniFile *MyIniFile;
    AnsiString sGroup, sKey, sMax, sMin, sKeyName;

    if(FileExists(CSVFileName)==false)
    {
        sKeyName.sprintf("CSV file not exists!! (%s)", CSVFileName);
        HTEL_ShowAlarmBox(sKeyName);                                            // GATE (1)
        return;
    }

    TStringList *sList;
    sList=new TStringList();
    sList->Clear();

    TStringList *slFormat;
    slFormat=new TStringList();
    slFormat->Clear();

    if(FileExists(IniFileName))
        DeleteFile(IniFileName);

    MyIniFile=new TMemIniFile(IniFileName);
    sList->LoadFromFile(CSVFileName);
    for(int i=0; i<FEditList->Count; i++)
    {
        slFormat->Clear();
        slFormat->Text=sList->Strings[i];

        if(slFormat->Count==eCustomLimitTotal)                                  //數量一定要符合
        {
            sGroup  =slFormat->Strings[eFormName];
            sKey    =slFormat->Strings[eEditName];
            sMax    =slFormat->Strings[eMaxmum];
            sMin    =slFormat->Strings[eMinimum];

            sKeyName.sprintf("%s_CustomMax", sKey);
            MyIniFile->WriteString(sGroup, sKeyName, sMax);

            sKeyName.sprintf("%s_CustomMin", sKey);
            MyIniFile->WriteString(sGroup, sKeyName, sMin);
        }
    }
    delete sList;
    delete slFormat;
    delete MyIniFile;
}
//---------------------------------------------------------------------------
//把客戶設定的最大最小值寫到INI檔案
//---------------------------------------------------------------------------
void HTEditList::SaveCustomLimitationToIni(AnsiString /*IniFileName*/)          // declared golden HTEditList.h, NEVER defined anywhere in golden HTEditList.cpp (verified: only appears inside 2 `//`-commented blocks, golden :1590/:1809) -- ODR-safe no-op stub, not a port gap
{
}
//---------------------------------------------------------------------------
//把客戶設定的最大最小值讀進來
//---------------------------------------------------------------------------
void HTEditList::ReadCustomLimitationFromIni(AnsiString IniFileName)            // golden :1624-1664
{
    TMemIniFile *MyIniFile;
    AnsiString sGroup, sKey, sMax, sMin, sKeyName, sCusMax, sCusMin;

    if(FileExists(IniFileName)==false)
    {
        sKeyName.sprintf("INI file not exists!! (%s)", IniFileName);
        HTEL_ShowAlarmBox(sKeyName);                                            // GATE (1)
        return;
    }

    MyIniFile=new TMemIniFile(IniFileName);
    for(int i=0; i<FEditList->Count; i++)
    {
        THTEdit *Temp;
        Temp=(THTEdit*)FEditList->Items[i];

        if(Temp->Content==ECInteger    ||                                       //只針對格式是數值的
           Temp->Content==ECDouble     ||
           Temp->Content==ECPosInt     ||
           Temp->Content==ECPosDouble  ||
           Temp->Content==ECNegInt     ||
           Temp->Content==ECNegDouble  )
        {
            // GATE (3): golden reads Temp->SourceControl->Parent->Name /
            // ->Name here, UNGUARDED (no NULL check in golden either).
            // Faithful default: both collapse to "" -- see file banner.
            sGroup  ="";
            sKey    ="";
            sMax    =Temp->MaxValue;
            sMin    =Temp->MinValue;

            sKeyName.sprintf("%s_CustomMax", sKey);
            sCusMax=MyIniFile->ReadString(sGroup, sKeyName, Temp->MaxValue);

            sKeyName.sprintf("%s_CustomMin", sKey);
            sCusMin=MyIniFile->ReadString(sGroup, sKeyName, Temp->MinValue);

            Temp->SetCustomLimitation(sCusMax, sCusMin);
        }
    }
    delete MyIniFile;
}
//---------------------------------------------------------------------------
void HTEditList::SaveDefaultLimitationToIni(AnsiString /*IniFileName*/)         // declared golden HTEditList.h, NEVER defined anywhere in golden HTEditList.cpp (grepped: zero hits) -- ODR-safe no-op stub, not a port gap
{
}
//---------------------------------------------------------------------------
void HTEditList::ReadDefaultLimitationFromIni(AnsiString /*IniFileName*/)       // declared golden HTEditList.h, NEVER defined anywhere in golden HTEditList.cpp (grepped: zero hits) -- ODR-safe no-op stub, not a port gap
{
}
//---------------------------------------------------------------------------
void HTEditList::SetFontBlue()                                                  // golden :1666-1669
{
    bFontBlue=true;
}
//---------------------------------------------------------------------------
bool HTEditList::ReadAndCompareDataFromFile(AnsiString asNameWithPath1, AnsiString asNameWithPath2,TList *tlDiff,AnsiString &asError)     // golden :1671-1743
{
    if(FileExists(asNameWithPath1)==false)
    {
        asError=AnsiString("File does not exist:")+asNameWithPath1;
        return false;
    }

    if(FileExists(asNameWithPath2)==false)
    {
        asError=AnsiString("File does not exist:")+asNameWithPath2;
        return false;
    }

    if(tlDiff==NULL)
        tlDiff=new TList;
    //
    TMemIniFile *file1=new TMemIniFile(asNameWithPath1);
    TMemIniFile *file2=new TMemIniFile(asNameWithPath2);
    AnsiString as1="",as2="",asGet="";
    int i1=0,i2=0;
    for(int i=0; i<FEditList->Count; i++)
    {
        THTEdit *Temp;
        Temp=(THTEdit*)FEditList->Items[i];
        if(Temp->bReadFromFile==true)
        {
            TCustomEdit *CEd = dynamic_cast <TCustomEdit *>(Temp->SourceControl);
            TComboBox   *Cbb = dynamic_cast <TComboBox   *>(Temp->SourceControl);
            TRadioGroup *RdG = dynamic_cast <TRadioGroup *>(Temp->SourceControl);
            TCheckBox   *Chb = dynamic_cast <TCheckBox   *>(Temp->SourceControl);
            if(CEd!=NULL)
            {
                as1=file1->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                as2=file2->ReadString(Temp->IniGroupName, Temp->IniKeyName, Temp->DefaultValue);
                if(as1!=as2)
                {
                    AddDiffData(tlDiff,Temp->IniKeyName,as1,as2);
                }
            }
            else if(Cbb!=NULL)
            {
                i1=file1->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, 0);
                i2=file2->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, 0);
                if(i1!=i2)
                {
                    AddDiffData(tlDiff,Cbb->Text,IntToStr(i1),IntToStr(i2));
                }
            }
            else if(RdG!=NULL)
            {
                i1=file1->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, 0);
                i2=file2->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, 0);
                if(i1!=i2)
                {
                    // GATE (6): golden reads RdG->Caption here; vclcompat::
                    // TRadioGroup carries no Caption. Faithful default: "".
                    AddDiffData(tlDiff,AnsiString(""),IntToStr(i1),IntToStr(i2));
                }
            }
            else if(Chb!=NULL)
            {
                i1=file1->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, 0);
                i2=file2->ReadInteger(Temp->IniGroupName, Temp->IniKeyName, 0);
                if(i1!=i2)
                {
                    AddDiffData(tlDiff,Chb->Caption,IntToStr(i1),IntToStr(i2));
                }
            }
        }
    }
    delete file1;
    delete file2;
    return true;
}
//---------------------------------------------------------------------------
void HTEditList::AddDiffData(TList *tdiff,AnsiString asContent,AnsiString a1,AnsiString a2)     // golden :1745-1752
{
    TStringList* tslDiffcontent=new TStringList;
    tslDiffcontent->Add(asContent);
    tslDiffcontent->Add(a1);
    tslDiffcontent->Add(a2);
    tdiff->Add(tslDiffcontent);
}
//---------------------------------------------------------------------------
void HTEditList::SetItemLimit(THTEdit *Item,const TEditContent &Content,const AnsiString &sMin,const AnsiString &sMax)  // golden :1754-1805
{
    if(sMax==AnsiString("") && sMin==AnsiString(""))
    {
        Item->CheckRange=false;
    }
    else if(Content==ECPort)                                                    //通訊埠
    {
        Item->CheckRange=true;
        Item->SetMax(AnsiString("65535"));
        Item->SetMin(AnsiString("0"));
    }
    else if(Content==ECBool)                                                    //Steven 20230502 : 修正bool沒有跟著存讀檔
    {
        Item->CheckRange=true;
        Item->SetMax(AnsiString("1"));
        Item->SetMin(AnsiString("0"));
    }
    else if(Content==ECInteger    ||    //數字
            Content==ECPosInt     ||    //正整數
            Content==ECNegInt)          //通訊埠
    {
        Item->CheckRange=true;
        if(atoi(sMin.c_str())>atoi(sMax.c_str()))
        {
            Item->SetMax(sMin);
            Item->SetMin(sMax);
        }
        else
        {
            Item->SetMax(sMax);
            Item->SetMin(sMin);
        }
    }
    else if(Content==ECDouble     ||    //浮點數
            Content==ECPosDouble  ||    //正浮點數
            Content==ECNegDouble  ||    //負浮點數
            Content==ECPercent)         //百分比
    {
        Item->CheckRange=true;
        if(atof(sMin.c_str())>atof(sMax.c_str()))
        {
            Item->SetMax(sMin);
            Item->SetMin(sMax);
        }
        else
        {
            Item->SetMax(sMax);
            Item->SetMin(sMin);
        }
    }
}
//---------------------------------------------------------------------------
int uBasicPickPlace::GetIntegerFromJSON(cJSON* root, AnsiString TagName)         // golden :2133-2142
{
    cJSON* _obj=cJSON_GetObjectItem(root, TagName.c_str());
    int iReturn=0;
    if(_obj!=NULL && _obj->type==cJSON_Number)
    {
        iReturn=_obj->valueint;
    }
    return iReturn;
}
//---------------------------------------------------------------------------
double uBasicPickPlace::GetDoubleFromJSON(cJSON* root, AnsiString TagName)       // golden :2144-2153
{
    cJSON* _obj=cJSON_GetObjectItem(root, TagName.c_str());
    double dReturn=0.0;
    if(_obj!=NULL && _obj->type==cJSON_Number)
    {
        dReturn=_obj->valuedouble;
    }
    return dReturn;
}
//---------------------------------------------------------------------------
void uBasicPickPlace::DataForwardAndDelete(TList* list)                         // golden :2155-2166
{
    if(list->Count==0)
    {
        return;
    }
    for(int i=1; i<list->Count; i++)
    {
        list->Move(i, i-1);
    }
    list->Delete(list->Count-1);
}
//---------------------------------------------------------------------------
AnsiString uBasicPickPlace::GetStringFromJSON(cJSON* root, AnsiString TagName)   // golden :2168-2177
{
    cJSON* _obj=cJSON_GetObjectItem(root, TagName.c_str());
    AnsiString sReturn="";
    if(_obj!=NULL && cJSON_IsString(_obj)==true)
    {
        sReturn=_obj->valuestring;
    }
    return sReturn;
}
//---------------------------------------------------------------------------
uHPSuckTeam::uHPSuckTeam()                                                       // golden :2179-2182
{
    ClearHPSuckTeam();
}
//---------------------------------------------------------------------------
uHPSuckTeam::uHPSuckTeam(int _iP, int _iR, int _iC, int _iSht, int _iKit)        // golden :2184-2192
{
    ClearHPSuckTeam();
    iP  =_iP;
    iR  =_iR;
    iC  =_iC;
    iSht=_iSht;
    iKit=_iKit;
}
//---------------------------------------------------------------------------
uHPSuckTeam::~uHPSuckTeam()                                                      // golden :2194-2203
{
    try
    {
        ClearHPSuckTeam();
    }
    catch(...)
    {
    }
}
//---------------------------------------------------------------------------
int uHPSuckTeam::GetUseSuckCol()                                                 // golden :2205-2218
{
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            if(bSuck[i][j]==true)
            {
                return j;
            }
        }
    }
    return 0;
}
//---------------------------------------------------------------------------
void uHPSuckTeam::ClearHPSuckTeam()                                              // golden :2220-2238
{
    iP      =0;
    iR      =0;
    iC      =0;
    iSht    =0;
    iKit    =0;

    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            iSite[i][j]     =-1;
            iPlateR[i][j]   =-1;
            iPlateC[i][j]   =-1;
            bSuck[i][j]     =false;
        }
    }
}
//---------------------------------------------------------------------------
uHPSuckTeam* uHPSuckTeam::Clone()                                                // golden :2240-2259
{
    uHPSuckTeam* clonedTeam = new uHPSuckTeam();
    clonedTeam->iP   =this->iP;
    clonedTeam->iR   =this->iR;
    clonedTeam->iC   =this->iC;
    clonedTeam->iSht =this->iSht;
    clonedTeam->iKit =this->iKit;
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            clonedTeam->iSite[i][j]  =this->iSite[i][j];
            clonedTeam->iPlateR[i][j]=this->iPlateR[i][j];
            clonedTeam->iPlateC[i][j]=this->iPlateC[i][j];
            clonedTeam->bSuck[i][j]  =this->bSuck[i][j];
        }
    }
    return clonedTeam;
}
//---------------------------------------------------------------------------
void uHPSuckTeam::SaveJSONFile(cJSON* root)                                      // golden :2261-2274
{
    if(root==NULL)
        return;
    cJSON_AddNumberToObject(root, "P",   iP);
    cJSON_AddNumberToObject(root, "Col", iC);
    cJSON_AddNumberToObject(root, "Row", iR);
    cJSON_AddNumberToObject(root, "Sht", iSht);
    cJSON_AddNumberToObject(root, "Kit", iKit);
    SavePlateRow(root);
    SavePlateCol(root);
    SavePlateSite(root);
    SavePlateSuck(root);
}
//---------------------------------------------------------------------------
void uHPSuckTeam::LoadJSONFile(cJSON* root)                                      // golden :2276-2289
{
    if(root==NULL)
        return;
    iP      =GetIntegerFromJSON(root, "P");
    iC      =GetIntegerFromJSON(root, "Col");
    iR      =GetIntegerFromJSON(root, "Row");
    iSht    =GetIntegerFromJSON(root, "Sht");
    iKit    =GetIntegerFromJSON(root, "Kit");
    ReadPlateRowFromJSON(root);
    ReadPlateColFromJSON(root);
    ReadPlateSiteFromJSON(root);
    ReadPlateSuckFromJSON(root);
}
//---------------------------------------------------------------------------
void uHPSuckTeam::SavePlateCol(cJSON* root)                                      // golden :2291-2306
{
    if(root==NULL)
        return;
    AnsiString asVarName="";
    int iArr[MAX_ARM_Row*MAX_ARM_Col];
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            iArr[i*MAX_ARM_Col+j]=iPlateC[i][j];
        }
    }
    cJSON *jarr=cJSON_CreateIntArray(&iArr[0], MAX_ARM_Row*MAX_ARM_Col);
    cJSON_AddItemToObject(root, "PlateCol", jarr);
}
//---------------------------------------------------------------------------
void uHPSuckTeam::ReadPlateColFromJSON(cJSON* root)                              // golden :2308-2325
{
    cJSON *array_obj=cJSON_GetObjectItem(root, "PlateCol");
    if(cJSON_IsArray(array_obj))
    {
        int array_size=cJSON_GetArraySize(array_obj);
        if(array_size!=MAX_ARM_Row*MAX_ARM_Col)
            return;
        for(int i=0; i<array_size; i++)
        {
            cJSON *item=cJSON_GetArrayItem(array_obj, i);
            if (cJSON_IsNumber(item))
            {
                iPlateC[i/MAX_ARM_Col][i%MAX_ARM_Col]=item->valueint;
            }
        }
    }
}
//---------------------------------------------------------------------------
void uHPSuckTeam::SavePlateRow(cJSON* root)                                      // golden :2327-2342
{
    if(root==NULL)
        return;
    AnsiString asVarName="";
    int iArr[MAX_ARM_Row*MAX_ARM_Col];
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            iArr[i*MAX_ARM_Col+j]=iPlateR[i][j];
        }
    }
    cJSON *jarr=cJSON_CreateIntArray(&iArr[0], MAX_ARM_Row*MAX_ARM_Col);
    cJSON_AddItemToObject(root, "PlateRow", jarr);
}
//---------------------------------------------------------------------------
void uHPSuckTeam::ReadPlateRowFromJSON(cJSON* root)                              // golden :2344-2361
{
    cJSON *array_obj=cJSON_GetObjectItem(root, "PlateRow");
    if(cJSON_IsArray(array_obj))
    {
        int array_size=cJSON_GetArraySize(array_obj);
        if(array_size!=MAX_ARM_Row*MAX_ARM_Col)
            return;
        for(int i=0; i<array_size; i++)
        {
            cJSON *item=cJSON_GetArrayItem(array_obj, i);
            if (cJSON_IsNumber(item))
            {
                iPlateR[i/MAX_ARM_Col][i%MAX_ARM_Col]=item->valueint;
            }
        }
    }
}
//---------------------------------------------------------------------------
void uHPSuckTeam::SavePlateSite(cJSON* root)                                     // golden :2363-2378
{
    if(root==NULL)
        return;
    AnsiString asVarName="";
    int iArr[MAX_ARM_Row*MAX_ARM_Col];
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            iArr[i*MAX_ARM_Col+j]=iSite[i][j];
        }
    }
    cJSON *jarr=cJSON_CreateIntArray(&iArr[0], MAX_ARM_Row*MAX_ARM_Col);
    cJSON_AddItemToObject(root, "Site", jarr);
}
//---------------------------------------------------------------------------
void uHPSuckTeam::ReadPlateSiteFromJSON(cJSON* root)                             // golden :2380-2397
{
    cJSON *array_obj=cJSON_GetObjectItem(root, "Site");
    if(cJSON_IsArray(array_obj))
    {
        int array_size=cJSON_GetArraySize(array_obj);
        if(array_size!=MAX_ARM_Row*MAX_ARM_Col)
            return;
        for(int i=0; i<array_size; i++)
        {
            cJSON *item=cJSON_GetArrayItem(array_obj, i);
            if (cJSON_IsNumber(item))
            {
                iSite[i/MAX_ARM_Col][i%MAX_ARM_Col]=item->valueint;
            }
        }
    }
}
//---------------------------------------------------------------------------
void uHPSuckTeam::SavePlateSuck(cJSON* root)                                     // golden :2399-2414
{
    if(root==NULL)
        return;
    AnsiString asVarName="";
    int iArr[MAX_ARM_Row*MAX_ARM_Col];
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            iArr[i*MAX_ARM_Col+j]=(bSuck[i][j])?1:0;
        }
    }
    cJSON *jarr=cJSON_CreateIntArray(&iArr[0], MAX_ARM_Row*MAX_ARM_Col);
    cJSON_AddItemToObject(root, "Suck", jarr);
}
//---------------------------------------------------------------------------
void uHPSuckTeam::ReadPlateSuckFromJSON(cJSON* root)                             // golden :2416-2433
{
    cJSON *array_obj=cJSON_GetObjectItem(root, "Suck");
    if(cJSON_IsArray(array_obj))
    {
        int array_size=cJSON_GetArraySize(array_obj);
        if(array_size!=MAX_ARM_Row*MAX_ARM_Col)
            return;
        for(int i=0; i<array_size; i++)
        {
            cJSON *item=cJSON_GetArrayItem(array_obj, i);
            if (cJSON_IsNumber(item))
            {
                bSuck[i/MAX_ARM_Col][i%MAX_ARM_Col]=(item->valueint==1)?true:false;
            }
        }
    }
}
//---------------------------------------------------------------------------
uHPSuckGroup::uHPSuckGroup()                                                     // golden :2435-2439
{
    HPSuckTeamList=new TList;
    HPSuckTeamList->Clear();
}
//---------------------------------------------------------------------------
uHPSuckGroup::~uHPSuckGroup()                                                    // golden :2441-2452
{
    try
    {
        HPSuckTeamList->Clear();
        delete HPSuckTeamList;
        HPSuckTeamList=NULL;
    }
    catch(...)
    {
    }
}
//---------------------------------------------------------------------------
void uHPSuckGroup::ClearTeamList()                                               // golden :2454-2457
{
    HPSuckTeamList->Clear();
}
//---------------------------------------------------------------------------
void uHPSuckGroup::AddHPSuckTeam(uHPSuckTeam* HPTeam)                            // golden :2459-2462
{
    HPSuckTeamList->Add(HPTeam);
}
//---------------------------------------------------------------------------
void uHPSuckGroup::AddHPSuckTeam(int iP, int iR, int iC, int iSht, int iKit)     // golden :2464-2468
{
    uHPSuckTeam* HPTeam=new uHPSuckTeam(iP, iR, iC, iSht, iKit);
    HPSuckTeamList->Add(HPTeam);
}
//---------------------------------------------------------------------------
void uHPSuckGroup::DelHPSuckTeam(int iIndex)                                     // golden :2470-2477
{
    if(HPSuckTeamList->Count==0 || iIndex>=HPSuckTeamList->Count)
    {
        return;
    }
    HPSuckTeamList->Delete(iIndex);
}
//---------------------------------------------------------------------------
uHPSuckTeam* uHPSuckGroup::ExtractLastTeam()                                     // golden :2479-2482
{
    return ExtractSuckTeam(HPSuckTeamList->Count-1);
}
//---------------------------------------------------------------------------
uHPSuckTeam* uHPSuckGroup::ExtractSuckTeam(int iIndex)                           // golden :2484-2492
{
    if(HPSuckTeamList->Count==0 || iIndex<0)
    {
        return NULL;
    }
    uHPSuckTeam* HPTeam=(uHPSuckTeam*)HPSuckTeamList->Items[iIndex];
    return HPTeam;
}
//---------------------------------------------------------------------------
int uHPSuckGroup::GetTeamCount()                                                 // golden :2494-2504 -- 分幾次吸放料
{
    if(this==NULL)
    {
        return 0;
    }
    else
    {
        return HPSuckTeamList->Count;
    }
}
//---------------------------------------------------------------------------
void uHPSuckGroup::DataForward()                                                 // golden :2506-2509
{
    DataForwardAndDelete(HPSuckTeamList);
}
//---------------------------------------------------------------------------
uHPSuckGroup* uHPSuckGroup::Clone()                                              // golden :2511-2519
{
    uHPSuckGroup* clonedGroup=new uHPSuckGroup();
    for(int i=0; i<GetTeamCount(); i++)
    {
        clonedGroup->HPSuckTeamList->Add(ExtractSuckTeam(i)->Clone());
    }
    return clonedGroup;
}
//---------------------------------------------------------------------------
void uHPSuckGroup::SaveJSONFile(cJSON* root)                                     // golden :2521-2534
{
    if(root==NULL)
        return;
    cJSON* rArr=cJSON_CreateArray();
    for(int i=0; i<HPSuckTeamList->Count; i++)
    {
        cJSON* jsonObj=cJSON_CreateObject();
        uHPSuckTeam* HPTeam=(uHPSuckTeam*)HPSuckTeamList->Items[i];
        HPTeam->SaveJSONFile(jsonObj);
        cJSON_AddItemToArray(rArr, jsonObj);
    }
    cJSON_AddItemToObject(root, "HPSuckTeamList", rArr);
}
//---------------------------------------------------------------------------
void uHPSuckGroup::LoadJSONFile(cJSON* root)                                     // golden :2536-2552
{
    if(root==NULL)
        return;
    cJSON *array_obj=cJSON_GetObjectItem(root, "HPSuckTeamList");
    if(cJSON_IsArray(array_obj))
    {
        int array_size=cJSON_GetArraySize(array_obj);
        for(int i=0; i<array_size; i++)
        {
            cJSON *item=cJSON_GetArrayItem(array_obj, i);
            uHPSuckTeam* HPTeam=new uHPSuckTeam();
            HPTeam->LoadJSONFile(item);
            HPSuckTeamList->Add(HPTeam);
        }
    }
}
//---------------------------------------------------------------------------
uPlateInfo::uPlateInfo()                                                         // golden :2554-2560
{
    HPSuckGroupList =new TList;
    iLimitcount     =0;
    TempSuckTeam    =NULL;
    HPSuckGroupList->Clear();
}
//---------------------------------------------------------------------------
uPlateInfo::~uPlateInfo()                                                        // golden :2562-2573
{
    try
    {
        HPSuckGroupList->Clear();
        delete HPSuckGroupList;
        HPSuckGroupList=NULL;
    }
    catch(...)
    {
    }
}
//---------------------------------------------------------------------------
int  uPlateInfo::GetHPSuckGroupCount()                                           // golden :2575-2578
{
    return HPSuckGroupList->Count;
}
//---------------------------------------------------------------------------
void uPlateInfo::SetTempSuckTeam(uHPSuckTeam* HPTeam)                            // golden :2580-2583
{
    TempSuckTeam=HPTeam;
}
//---------------------------------------------------------------------------
uHPSuckTeam* uPlateInfo::GetTempSuckTeam()                                       // golden :2585-2588
{
    return TempSuckTeam;
}
//---------------------------------------------------------------------------
void uPlateInfo::AddHPSuckGroup()                                                // golden :2590-2594
{
    uHPSuckGroup* HPGroup=new uHPSuckGroup();
    AddHPSuckGroup(HPGroup);
}
//---------------------------------------------------------------------------
void uPlateInfo::AddHPSuckGroup(uHPSuckGroup* HPGroup)                           // golden :2596-2606
{
    uHPSuckGroup* HPGroupTmp=ExtractLastGroup();                                //RogerYang 20250814 : 如果HPSuckGroupList裡面的上一個Group->HPSuckTeamList內容為空，就不要再新增Group
    if(HPGroupTmp!=NULL)
    {
        uHPSuckTeam* HPTeam=HPGroupTmp->ExtractLastTeam();
        if(HPTeam==NULL)
            return;
    }
    HPSuckGroupList->Add(HPGroup);
}
//---------------------------------------------------------------------------
void uPlateInfo::DelHPSuckGroup(int iIndex)                                      // golden :2608-2615
{
    if(HPSuckGroupList->Count==0 || iIndex>=HPSuckGroupList->Count)
    {
        return;
    }
    HPSuckGroupList->Delete(iIndex);
}
//---------------------------------------------------------------------------
void uPlateInfo::ClearGroupList()                                                // golden :2617-2628
{
    for(int i=0; i<HPSuckGroupList->Count; i++)
    {
        uHPSuckGroup* HPGroup=ExtractSuckGroup(i);
        if(HPGroup==NULL)
            continue;

        HPGroup->ClearTeamList();
    }
    HPSuckGroupList->Clear();
}
//---------------------------------------------------------------------------
uHPSuckGroup* uPlateInfo::ExtractLastGroup()                                     // golden :2630-2633
{
    return ExtractSuckGroup(HPSuckGroupList->Count-1);
}
//---------------------------------------------------------------------------
uHPSuckGroup* uPlateInfo::ExtractSuckGroup(int iIndex)                           // golden :2635-2643
{
    if(HPSuckGroupList->Count==0 || iIndex<0)
    {
        return NULL;
    }
    uHPSuckGroup* info=(uHPSuckGroup*)HPSuckGroupList->Items[iIndex];
    return info;
}
//---------------------------------------------------------------------------
uHPSuckTeam* uPlateInfo::ExtractFirstTeam()                                      // golden :2645-2648
{
    return ExtractTeamByIndex(0, 0);
}
//---------------------------------------------------------------------------
uHPSuckTeam* uPlateInfo::ExtractLastTeam()                                       // golden :2650-2653
{
    return ExtractTeamByIndex(GetHPSuckGroupCount()-1, 0);
}
//---------------------------------------------------------------------------
uHPSuckTeam* uPlateInfo::ExtractLastTeamHasData()                                // golden :2655-2671
{
    uHPSuckTeam *HPTeam=ExtractTeamByIndex(GetHPSuckGroupCount()-1, 0);
    if(HPTeam==NULL)
    {
        HPTeam=ExtractTeamByIndex(GetHPSuckGroupCount()-2, 0);
    }

    if(HPTeam==NULL)
    {
        return NULL;
    }
    else
    {
        return HPTeam;
    }
}
//---------------------------------------------------------------------------
uHPSuckTeam* uPlateInfo::ExtractTeamByIndex(int iGroupIndex, int iTeamIndex)     // golden :2673-2692
{
    uHPSuckGroup *HPGroup=ExtractSuckGroup(iGroupIndex);
    if(HPGroup==NULL)
    {
        return NULL;
    }
    else
    {
        uHPSuckTeam *HPTeam=HPGroup->ExtractSuckTeam(iTeamIndex);
        if(HPTeam==NULL)
        {
            return NULL;
        }
        else
        {
            return HPTeam;
        }
    }
}
//---------------------------------------------------------------------------
void uPlateInfo::SetArrPlateXY(int iSuckRow, int iSuckCol, int _iP, int _iRow, int _iCol, int _Site)  // golden :2694-2710
{
    uHPSuckGroup* HPGroup=ExtractLastGroup();
    if(HPGroup!=NULL)
    {
        uHPSuckTeam* HPTeam=HPGroup->ExtractLastTeam();
        if(HPTeam!=NULL)
        {
            HPTeam->iP                           =_iP;
//            HPTeam->dTime                        =Now();
            HPTeam->iPlateC[iSuckRow][iSuckCol]  =_iCol;
            HPTeam->iPlateR[iSuckRow][iSuckCol]  =_iRow;
            HPTeam->iSite[iSuckRow][iSuckCol]    =_Site;
            HPTeam->bSuck[iSuckRow][iSuckCol]    =true;
        }
    }
}
//---------------------------------------------------------------------------
void uPlateInfo::UpdateHPSuckGroup(int iP, int iR, int iC, int iSht, int iKit)   // golden :2712-2718
{
    if(GetHPSuckGroupCount()<=0)
        AddHPSuckGroup();
    uHPSuckGroup *HPGroup=ExtractLastGroup();
    HPGroup->AddHPSuckTeam(iP, iR, iC, iSht, iKit);
}
//---------------------------------------------------------------------------
uHPSuckTeam* uPlateInfo::SearchHPSuckTeamByCoordinate(int &iSuckRow, int &iSuckCol, int _iP, int _iRow, int _iCol)  // golden :2720-2756
{
    for(int i=0; i<HPSuckGroupList->Count; i++)
    {
        uHPSuckGroup* HPGroup=ExtractSuckGroup(i);
        if(HPGroup==NULL)
            continue;
        for(int j=0; j<HPGroup->GetTeamCount(); j++)
        {
            uHPSuckTeam* HPTeam=HPGroup->ExtractSuckTeam(j);
            if(HPTeam==NULL)
            {
                continue;
            }
            else
            {
                if(HPTeam->iP==_iP)
                {
                    for(int ii=0; ii<MAX_ARM_Row; ii++)
                    {
                        for(int jj=0; jj<MAX_ARM_Col; jj++)
                        {
                            if(HPTeam->iPlateC[ii][jj]==_iCol &&
                               HPTeam->iPlateR[ii][jj]==_iRow)
                            {
                                iSuckRow=ii;
                                iSuckCol=jj;
                                return HPTeam;
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}
//---------------------------------------------------------------------------
bool uPlateInfo::SetPlateSuck(int _iP, int _iRow, int _iCol, bool bSuck)         // golden :2758-2771
{
    if(HPSuckGroupList->Count==0)
    {
        return false;
    }
    int iSuckRow=0, iSuckCol=0;
    uHPSuckTeam *HPTeam=SearchHPSuckTeamByCoordinate(iSuckRow, iSuckCol, _iP, _iRow, _iCol);
    if(HPTeam!=NULL)
    {
        HPTeam->bSuck[iSuckRow][iSuckCol]=bSuck;
    }
    return true;
}
//---------------------------------------------------------------------------
void uPlateInfo::SaveFile(AnsiString sFileName)                                  // golden :2774-2788
{
    AnsiString asGroupName="";
    cJSON* root=cJSON_CreateObject();
    for(int i=0; i<HPSuckGroupList->Count; i++)
    {
        cJSON* r_single=cJSON_CreateObject();
        uHPSuckGroup* HPGroup=(uHPSuckGroup*)HPSuckGroupList->Items[i];
        HPGroup->SaveJSONFile(r_single);
        asGroupName=AnsiString().sprintf("HPSuckGroupList_%d", i);
        cJSON_AddItemToObject(root, asGroupName.c_str(), r_single);
    }
    WriteDataToFile(sFileName.c_str(), cJSON_Print(root), true);
    cJSON_Delete(root);
}
//---------------------------------------------------------------------------
void uPlateInfo::LoadFile(AnsiString sFileName)                                  // golden :2790-2820
{
    if(HPSuckGroupList->Count>0)
    {
        HPSuckGroupList->Clear();
    }
    char* file_buf=ReadDataFromFile(sFileName);
    cJSON* loaded_root=cJSON_Parse(file_buf);
    if(!loaded_root)
    {
        return;
    }
    int iGroupNum=cJSON_GetArraySize(loaded_root);
    for(int i=0; i<iGroupNum; i++)
    {
        cJSON* r_single=cJSON_GetArrayItem(loaded_root, i);
        cJSON *array_obj=cJSON_GetObjectItem(r_single, "HPSuckTeamList");
        if(cJSON_IsArray(array_obj) &&
           cJSON_GetArraySize(array_obj)==0 &&
           i!=iGroupNum-1)                                                      //RogerYang 20250815 避免刪除掉最後一筆剛好Group內容是空的
        {
            //NULL
        }
        else
        {
            uHPSuckGroup* HPGroup=new uHPSuckGroup();
            HPGroup->LoadJSONFile(r_single);
            AddHPSuckGroup(HPGroup);
        }
    }
}
//---------------------------------------------------------------------------
void uPlateInfo::ResetFile(AnsiString sFileName)                                 // golden :2822-2830 -- GATE (7)
{
    ClearGroupList();                                                           //Steven 20250519 : 避免有殘留的資料
    //AI(W906-PT-W2-integrate) 20260807: GATE (7) RETIRED -- golden's own call
    //  restored.  The gate substituted FileExists() and justified it with
    //  "`class FileInfo` does not exist in this port", which was false by the
    //  time this file landed: ProductionInfo/FileInfo.h:129 defines class
    //  FileInfo with a default ctor (:137) and IsFilePathExist (:147), body at
    //  ProductionInfo/FileInfo.cpp:243.  It was translated by the same wave
    //  (PT-W2, group "prodinfo") while this unit was being written, so the
    //  whole-tree grep behind the gate was stale, not wrong-at-the-time.
    //  No behaviour change either way -- the real body is
    //  `sPath!="" && FileExists(sPath)` -- but a GATE REGISTER entry asserting
    //  a symbol is absent when it is present is the defect, independent of
    //  whether the substitute happened to agree.
    if(FileInfo().IsFilePathExist(sFileName))                                   // golden :2825
    {
        DeleteFile(sFileName);
    }
    SaveFile(sFileName);
}
//---------------------------------------------------------------------------
bool uPlateInfo::DataForwardAndNextTeam()                                        // golden :2832-2852
{
    if(GetHPSuckGroupCount()==0)
        return false;
    uHPSuckGroup *HPGroup=ExtractSuckGroup(0);
    if(HPGroup->GetTeamCount()<=0)
    {
        return false;
    }
    else if(HPGroup->GetTeamCount()>1)
    {
        HPGroup->DataForward();
        return true;
    }
    else
    {
        HPGroup->DataForward();
        DataForwardAndDelete(HPSuckGroupList);
        return false;
    }
}
//---------------------------------------------------------------------------
bool uPlateInfo::GetHPFirstTeamSuckUse(bool bSuck[MAX_ARM_Row][MAX_ARM_Col])     // golden :2854-2879
{
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            bSuck[i][j]=false;
        }
    }
    uHPSuckTeam *HPTeam=ExtractFirstTeam();
    if(HPTeam==NULL)
    {
        return false;
    }
    else
    {
        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=0; j<MAX_ARM_Col; j++)
            {
                bSuck[i][j]=HPTeam->bSuck[i][j];
            }
        }
        return true;
    }
}
//---------------------------------------------------------------------------
bool uPlateInfo::GetHPFirstTeamMotUse(bool bSuck[MAX_ARM_Row][MAX_ARM_Col])      // golden :2881-2913
{
    for(int i=0; i<MAX_ARM_Row; i++)
    {
        for(int j=0; j<MAX_ARM_Col; j++)
        {
            bSuck[i][j]=false;
        }
    }
    uHPSuckTeam *HPTeam=ExtractFirstTeam();
    if(HPTeam==NULL)
    {
        return false;
    }
    else
    {
        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=0; j<MAX_ARM_Col; j++)
            {
                if(HPTeam->iSite[i][j]>0)                                       //JerryYang 202050813 : fix錯誤吸嘴下降
                {
                    bSuck[i][j]=true;                                           //RogerYang 20250613 這裡是決定基準軸的位置，只看HP的suck資料可能會導致基準軸計算錯誤
                }
                else
                {
                    bSuck[i][j]=false;
                }
            }
        }
        return true;
    }
}
//---------------------------------------------------------------------------
int uPlateInfo::GetHPFirstTeamSuckCol()                                          // golden :2915-2936
{
    uHPSuckTeam *HPTeam=ExtractFirstTeam();
    if(HPTeam==NULL)
    {
        return 0;
    }
    else
    {
        for(int i=0; i<MAX_ARM_Row; i++)
        {
            for(int j=0; j<MAX_ARM_Col; j++)
            {
                if(HPTeam->bSuck[i][j]==true)
                {
                    return j;
                }
            }
        }
        return 0;
    }
}
//---------------------------------------------------------------------------
bool uPlateInfo::GetHPFirstTeamPlate(int &iP, int &iR, int &iC, int &iSht, int &iKit)  // golden :2938-2982
{
    uHPSuckGroup* HPGroup=ExtractSuckGroup(0);
    int iStep=0;
    if(HPGroup==NULL)
    {
        return false;
    }
    else
    {
        iStep=HPGroup->GetTeamCount();
    }
    while(iStep==0 &&                                                           //沒有Step資料的, 表示是空集合
          HPGroup!=NULL &&
          iLimitcount<500)                                                      //JerryYang 20250410 : Protect for while loop
    {
        HPGroup->DataForward();
        HPGroup=ExtractSuckGroup(0);
        if(HPGroup==NULL) break;                                                //AI(JimmyChiu) 20260421: Fix C NULL guard
        iStep=HPGroup->GetTeamCount();
        iLimitcount++;
    };

    if(iLimitcount>=500)
    {
        iLimitcount=0;                                                          //AI(JimmyChiu) 20260421: Fix C reset counter on failure
        MyDBIProcess("Exception", "GetHPFirstTeamPlate iLimitcount>=500, list maybe corrupted"); //AI(JimmyChiu) 20260421: Fix C diag log
        return false;
    }
    iLimitcount=0;
    uHPSuckTeam *HPTeam=ExtractFirstTeam();
    if(HPTeam==NULL)
    {
        return false;
    }
    else
    {
        iP  =HPTeam->iP;
        iR  =HPTeam->iR;
        iC  =HPTeam->iC;
        iSht=HPTeam->iSht;
        iKit=HPTeam->iKit;
        return true;
    }
}
//---------------------------------------------------------------------------
bool uPlateInfo::GetHPFirstTeam(int *iP, int (*PlateR)[MAX_ARM_Col], int (*PlateC)[MAX_ARM_Col], bool (*Suck)[MAX_ARM_Col])  // golden :2984-2987
{
    return GetHPTeamSuck(0, 0, iP, PlateR, PlateC, Suck);
}
//---------------------------------------------------------------------------
bool uPlateInfo::GetHPTeamSuck(int iGroupIndex, int iTeamIndex, int *iP, int (*PlateR)[MAX_ARM_Col], int (*PlateC)[MAX_ARM_Col], bool (*Suck)[MAX_ARM_Col])  // golden :2989-3026
{
    uHPSuckGroup *HPGroup=ExtractSuckGroup(iGroupIndex);
    if(HPGroup==NULL)
    {
        return false;
    }
    else
    {
        uHPSuckTeam *HPTeam=HPGroup->ExtractSuckTeam(iTeamIndex);
        if(HPTeam==NULL)
        {
            return false;
        }
        else
        {
            *iP=HPTeam->iP;
            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    PlateR[i][j]=HPTeam->iPlateR[i][j];
                    PlateC[i][j]=HPTeam->iPlateC[i][j];
//                    if(HPTeam->iSite[i][j]==-1 && HPTeam->bSuck[i][j]==false)
//                    {
//                        Suck[i][j]=false;
//                    }
//                    else
//                    {
//                        Suck[i][j]=true;                                           //RogerYang 20250613 這裡是決定基準軸的位置，只看HP的suck資料可能會導致基準軸計算錯誤
//                    }
                    Suck[i][j]  =HPTeam->bSuck[i][j];                           //RogerYang 20250819 改回來
                }
            }
            return true;
        }
    }
}
//---------------------------------------------------------------------------
bool uPlateInfo::GetHPFirstTeamToList(int *iP, TList* lsPoint2D)                 // golden :3028-3058
{
    uHPSuckGroup *HPGroup=ExtractSuckGroup(0);
    if(HPGroup==NULL)
    {
        return false;
    }
    else
    {
        if(lsPoint2D==NULL)
        {
            return false;
        }
        for(int i=0; i<HPGroup->GetTeamCount(); i++)
        {
            uHPSuckTeam *HPTeam=HPGroup->ExtractSuckTeam(i);
            *iP=HPTeam->iP;
            for(int ii=0; ii<MAX_ARM_Row; ii++)
            {
                for(int jj=0; jj<MAX_ARM_Col; jj++)
                {
                    if(HPTeam->bSuck[ii][jj]==true)
                    {
                        lsPoint2D->Add(new uPoint2D(HPTeam->iPlateC[ii][jj], HPTeam->iPlateR[ii][jj]));
                    }
                }
            }
        }
    }
    return true;
}
//---------------------------------------------------------------------------
void uPlateInfo::InsertHPGroup(int Index, void *Item)                           // golden :3060-3063
{
    HPSuckGroupList->Insert(Index, Item);
}
//---------------------------------------------------------------------------
void uPlateInfo::TransferByuPlacePoint2DList(TList *Listpp2D)                   // golden :3065-3073
{
    uPlacePoint2D* pp2D;
    for(int i=0; i<Listpp2D->Count; i++)
    {
        pp2D=(uPlacePoint2D*)Listpp2D->Items[i];
        AddDataByPlacePoint2D(pp2D);
    }
}
//---------------------------------------------------------------------------
void uPlateInfo::AddDataByPlacePoint2D(uPlacePoint2D* pp2D)                     // golden :3075-3116
{
    int iPlateStep=pp2D->iHPStep;
    while(iPlateStep>=GetHPSuckGroupCount())
    {
        AddHPSuckGroup();
    }
    uHPSuckGroup* HPGroup=ExtractSuckGroup(iPlateStep);
    int iLoaderStep=pp2D->iSubStep;
    while(iLoaderStep>=HPGroup->GetTeamCount())
    {
        HPGroup->AddHPSuckTeam(-1, -1, -1, -1, -1);
    }
    uHPSuckTeam* HPTeam=HPGroup->ExtractSuckTeam(iLoaderStep);
    HPTeam->iP=pp2D->iPlate;
    if(HPTeam->iR>=0)
    {
        if(HPTeam->iR>pp2D->iPlateRow)
        {
            HPTeam->iR=pp2D->iPlateRow;
        }
    }
    else
    {
        HPTeam->iR=pp2D->iPlateRow;
    }

    if(HPTeam->iC>=0)
    {
        if(HPTeam->iC>pp2D->iPlateCol)
        {
            HPTeam->iC=pp2D->iPlateCol;
        }
    }
    else
    {
        HPTeam->iC=pp2D->iPlateCol;
    }
    HPTeam->iPlateR[pp2D->iSuckRow][pp2D->iSuckCol] =pp2D->iPlateRow;
    HPTeam->iPlateC[pp2D->iSuckRow][pp2D->iSuckCol] =pp2D->iPlateCol;
    HPTeam->bSuck[pp2D->iSuckRow][pp2D->iSuckCol]   =true;
}
//---------------------------------------------------------------------------
// uPoint2D -- see file banner / HTEditList.h INTEGRATION HAZARDS (C): the
// class itself is declared in cmydef.h, not redeclared here. These 3 bodies
// are exactly what cmydef.h's own comment says belongs in this file.
//---------------------------------------------------------------------------
uPoint2D::uPoint2D()                                                             // golden :3118-3121
{
    Clear();
}
//---------------------------------------------------------------------------
uPoint2D::uPoint2D(int x, int y)                                                 // golden :3123-3128
{
    Clear();
    X=x;
    Y=y;
}
//---------------------------------------------------------------------------
void uPoint2D::Clear()                                                          // golden :3130-3134
{
    X=0;
    Y=0;
}
//---------------------------------------------------------------------------
