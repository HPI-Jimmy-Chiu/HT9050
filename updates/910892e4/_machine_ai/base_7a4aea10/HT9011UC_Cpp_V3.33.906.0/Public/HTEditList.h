// =============================================================================
//  Public/HTEditList.h  --  HTEditList (VCL widget<->parameter binder) plus the
//  HotPlate pick/place bookkeeping family (uBasicPickPlace / uHPSuckTeam /
//  uHPSuckGroup / uPlateInfo / uPlacePoint2D) and the Fill-The-Tray-After-
//  OutArm-Place geometry structs (ArmAndSuckInfo / SingleSuckPosOnTray /
//  MyFillTheTrayAfterOutArmPlace / PickPlaceByArm).
//
//  Faithful translation of golden Public/HTEditList.h (359 lines, BCB6,
//  Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 ("hteditlist" group).
//
//  ROLE: HTEditList is the config-screen glue (Add* one widget+one parameter,
//  then SaveEditTextToFile/ReadEditTextFromFile/InitialDataToEdit push data
//  both ways against a TMemIniFile). uPlateInfo (PickFromHPList/PlaceToHPList/
//  PlaceToCleanList) is the HotPlate multi-team pick/place "shopping list"
//  the in-arm engine consumes; it is pure list/array bookkeeping with a cJSON
//  persistence layer, no VCL dependency at all.
//
//  ============================================================================
//  INTEGRATION HAZARDS -- READ BEFORE WIRING THIS FILE INTO ANY CMake TARGET
//  ============================================================================
//  This header/its .cpp were written to be a byte-faithful, standalone mirror
//  of golden. They were NOT reconciled against three pre-existing NARROWER
//  shims that already occupy some of the same global names (rule: this wave
//  may create new mirrored files ONLY, never edit a pre-existing one -- so
//  none of the three below were touched):
//
//   (A) aHotPlateSubstrate.h (:80-94) already defines a global, non-namespaced
//       `class TList` (Add/Clear/Count/Items only -- no Move/Delete/Insert).
//       This file needs Move/Delete/Insert too (uBasicPickPlace::
//       DataForwardAndDelete, uHPSuckGroup::DelHPSuckTeam, uPlateInfo::
//       DelHPSuckGroup/InsertHPGroup), so it defines its OWN, differently-
//       shaped `class TList` below, under a DIFFERENT guard macro
//       (HTEDITLIST_TLIST_SHIM, not aHotPlateSubstrate.h's
//       HT9045_TLIST_SHIM) specifically so the two are never silently
//       conflated -- any TU that ends up including both headers gets a hard
//       "redefinition of class TList" compile error, not a silent narrower-
//       capability substitution. That compile error is the correct, safe
//       outcome; integration must then pick ONE TList (this file's is the
//       superset) and delete the other.
//   (B) aHotPlateSubstrate.h (:673-716) and aoutarm9045.h also already define
//       MINIMAL, narrower `class uPlateInfo` / forward-only `class
//       uHPSuckTeam;` shims (their own banners explicitly call these
//       placeholders meant to be replaced "when the real ... HTEditList.h ...
//       land"). aHotPlateSubstrate.cpp:831-840 additionally DEFINES (not just
//       declares) the globals `uPlateInfo *PickFromHPList` and `uPlateInfo
//       *PlaceToCleanList` against its narrower shim, with ~30 existing
//       consumer .cpp files (csystem.cpp, every ainarm9045_*.cpp variant,
//       AutoClean.cpp, cmydef.cpp, ...) built against that narrower shape.
//       THIS file also defines the full, real `uPlateInfo`/`uHPSuckTeam`/
//       `uHPSuckGroup` (golden's actual shape) and the same-named globals
//       `PickFromHPList`/`PlaceToCleanList` (golden HTEditList.cpp:56-59).
//       Landing BOTH aHotPlateSubstrate.cpp and this file's .cpp in the same
//       CMake target is a hard LINK-time duplicate-symbol error (two
//       `uPlateInfo *PickFromHPList` definitions) on top of the compile-time
//       class-redefinition error from (A)/(C). This is INTENTIONAL, not an
//       oversight: resolving it means deleting the substrate's narrower
//       uPlateInfo/uHPSuckTeam/TList and repointing its ~30 consumers at this
//       file's real ones -- a cross-cutting change explicitly out of this
//       wave's "create new mirrored files ONLY" mandate. `PlaceToHPList`
//       (golden HTEditList.cpp:57) is NOT pre-defined anywhere else and is
//       therefore collision-free today.
//   (C) cmydef.h (:17-29) already declares the FULL `class uPoint2D` (ctor(),
//       ctor(int,int), Clear(), fields X/Y) as a W0-TAIL stub, with its own
//       comment "ctor/Clear bodies live in HTEditList.cpp (W3)". This file
//       honours that plan exactly: it does NOT redeclare `class uPoint2D`
//       (golden HTEditList.h:249-259) -- it `#include`s cmydef.h and supplies
//       ONLY the 3 method bodies in HTEditList.cpp, matching cmydef.h's own
//       stated intent byte-for-byte. No collision here; this one was already
//       reconciled by an earlier wave.
//
//  None of (A)/(B) block THIS file's OWN standalone syntax-check (verified:
//  `g++ -fsyntax-only` on HTEditList.cpp alone is clean -- it never
//  #includes aHotPlateSubstrate.h or aoutarm9045.h). They block adding this
//  file's .cpp to ht9045_sm (or any target that also compiles
//  aHotPlateSubstrate.cpp) until the reconciliation above happens. Flagged
//  here, in the .cpp banner, and in this wave's final report -- not silently
//  left for the next person to discover at a failed build.
//  ============================================================================
//
//  WAVE SCOPE / GATE REGISTER: see Public/HTEditList.cpp's banner (this header
//  declares types only; every behavioural gate lives in the .cpp).
//
//  __property -> plain public field / explicit method, per vclcompat's own
//  "hand-translate to getter/setter" instruction -- see Public/HTEdit.h's
//  banner for the general rule (applies here too: HTEditList itself has no
//  __property of its own, but consumes THTEdit's).
//
//  TU-local minimal stand-in added here (see Public/HTEdit.h's file-head note
//  for the sibling TWinControl/TNotifyEvent/TKeyPressEvent/Variant additions):
//    * TDateTimePicker -- genuinely missing from vclcompat/Controls.h (grepped
//      20260807, zero hits tree-wide before this file). golden dynamic_casts
//      THTEdit::SourceControl to TDateTimePicker* in SaveEditTextToFile/
//      ReadEditTextFromFile/InitialDataToEdit; only ->DateTime (a double) is
//      ever touched. Since nothing anywhere in this tree ever constructs one
//      (matching vclcompat/Controls.h's own SCOPE BOUNDARY (1) precedent for
//      "no live widget graph exists yet"), every one of those dynamic_cast
//      branches is reachable-but-always-NULL -- faithful to golden's control
//      flow, harmless until real forms land.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#ifndef HTEditListH
#define HTEditListH
//---------------------------------------------------------------------------
#include "vclcompat/vcl_compat.h"
#include "vclcompat/Controls.h"
#include "Public/MyStringList.h"
#include "Public/cJSON.h"
#include "Public/HTEdit.h"
#include "cmydef.h"              // uPoint2D (already declared, see INTEGRATION HAZARDS (C)) + MAX_ARM_Row/Col + X_PITCH_COUNT

extern AnsiString sHPPickRec;
extern AnsiString sHPPickRecException;
extern AnsiString sHPPlaceRec;
extern AnsiString sCleanPlaceRec;

//---------------------------------------------------------------------------
//  TList -- see INTEGRATION HAZARDS (A) above. Full VCL Classes.hpp TList
//  surface this file's bodies actually call: Add/Clear/Count/Items[]/Move/
//  Delete/Insert.
//---------------------------------------------------------------------------
#ifndef HTEDITLIST_TLIST_SHIM
#define HTEDITLIST_TLIST_SHIM
#include <vector>
class TList
{
    std::vector<void*> v;
public:
    int Count;
    TList() : Count(0) { Items.pv=&v; }
    int  Add(void* p)     { v.push_back(p); Count=(int)v.size(); return Count-1; }
    void Clear()          { v.clear(); Count=0; }
    void Delete(int idx)
    {
        if(idx<0 || idx>=(int)v.size()) return;
        v.erase(v.begin()+idx);
        Count=(int)v.size();
    }
    void Insert(int idx, void* p)
    {
        if(idx<0) idx=0;
        if(idx>(int)v.size()) idx=(int)v.size();
        v.insert(v.begin()+idx, p);
        Count=(int)v.size();
    }
    void Move(int curIndex, int newIndex)                                     // golden VCL TList::Move(CurIndex,NewIndex)
    {
        if(curIndex<0 || curIndex>=(int)v.size()) return;
        void* item=v[curIndex];
        v.erase(v.begin()+curIndex);
        if(newIndex<0) newIndex=0;
        if(newIndex>(int)v.size()) newIndex=(int)v.size();
        v.insert(v.begin()+newIndex, item);
        Count=(int)v.size();
    }
    struct ItemsProxy {
        const std::vector<void*>* pv;
        void* operator[](int i) const { return (*pv)[i]; }
    } Items;
};
#endif // HTEDITLIST_TLIST_SHIM

//---------------------------------------------------------------------------
//  TDateTimePicker -- see file-head note.
//---------------------------------------------------------------------------
class TDateTimePicker : public TControl
{
public:
    double DateTime;
    TDateTimePicker() : DateTime(0.0) {}
    virtual ~TDateTimePicker() {}
};

//---------------------------------------------------------------------------
class HTEditList
{
    public:
        TList *FEditList;
        int iDecimalPoint;
        int iBarcodeReadType;
        HTEditList();
        ~HTEditList();

        void Add(TControl  *SoureCtrl,                                          //Steven 20230224 : Edit List改支援多型態元件
                 void *Par,
                 TEditContent Content=ECText,
                 AnsiString GroupName="",
                 AnsiString KeyName="",
                 bool Visible=true,
                 bool Enable=true,
                 bool ReadFromFile=true,                                        //是否從檔案讀取
                 AnsiString DefValue="",
                 bool bDisableEventOverlap=false,
                 AnsiString Min="",
                 AnsiString Max="",
                 int iTransform=0);

        void AddInteger (TEdit *SoureEdit, int    *Par, int Min=-9999999,       int Max=9999999,      int DefValue=0,      AnsiString GroupName="", AnsiString KeyName="");
        void AddInteGerP(TEdit *SoureEdit, int    *Par, int Min=0,              int Max=9999999,      int DefValue=0,      AnsiString GroupName="", AnsiString KeyName="");
        void AddIntegerN(TEdit *SoureEdit, int    *Par, int Min=-9999999,       int Max=0,            int DefValue=0,      AnsiString GroupName="", AnsiString KeyName="");
        void AddDouble  (TEdit *SoureEdit, double *Par, double Min=-9999999.0,  double Max=9999999.0, double DefValue=0.0, AnsiString GroupName="", AnsiString KeyName="");
        void AddDoubleP (TEdit *SoureEdit, double *Par, double Min=0.0,         double Max=9999999.0, double DefValue=0.0, AnsiString GroupName="", AnsiString KeyName="");
        void AddDoubleN (TEdit *SoureEdit, double *Par, double Min=-9999999.0,  double Max=0.0,       double DefValue=0.0, AnsiString GroupName="", AnsiString KeyName="");
        void AddPercent (TEdit *SoureEdit, double *Par, double Min=0.0,         double Max=100.0,     double DefValue=0.0, AnsiString GroupName="", AnsiString KeyName="");

        void ClearALL();
        bool SaveEditTextToFile(AnsiString Path, AnsiString FileName);
        bool ReadEditTextFromFile(AnsiString Path, AnsiString FileName);
        void InitialDataToEdit();
        void SetBarcodeType(int iBarcodeType);

        void ExportCustomLimitToCSV(AnsiString CsvFileName);
        void ImportCustomLimitationFromCSV(AnsiString CSVFileName, AnsiString IniFileName);
        void SaveCustomLimitationToIni(AnsiString IniFileName);
        void ReadCustomLimitationFromIni(AnsiString IniFileName);

        void SaveDefaultLimitationToIni(AnsiString IniFileName);
        void ReadDefaultLimitationFromIni(AnsiString IniFileName);

        bool bAlarmLimitation;
        void SetFontBlue();                                                     //Sam 20220921 : config儲存跟隨recipe
        bool ReadAndCompareDataFromFile(AnsiString asNameWithPath1, AnsiString asNameWithPath2, TList *tlDiff, AnsiString &asError);//JimmyChiu 20230504 : 新增資料比對顯示function
    private:
        bool bFontBlue;                                                         //Sam 20220921 : config儲存跟隨recipe
        void AddDiffData(TList *tdiff, AnsiString asContent, AnsiString a1, AnsiString a2);     //JimmyChiu 20230504 : 新增資料比對顯示function
        void SetItemLimit(THTEdit *Item, const TEditContent &Content, const AnsiString &sMin, const AnsiString &sMax);
};
//---------------------------------------------------------------------------
// golden's commented-out class HTCheckBoxList (golden HTEditList.h) is dead
// code in golden itself (every line already `//`) -- not ported, nothing to
// mirror.
//---------------------------------------------------------------------------
class uPlacePoint2D;
//---------------------------------------------------------------------------
class uBasicPickPlace                                                           //Jimmychiu 20230417 : Record the position after placing the IC
{
public:     // User declarations
    int    GetIntegerFromJSON(cJSON* root, AnsiString TagName);
    double GetDoubleFromJSON(cJSON* root, AnsiString TagName);
    void   DataForwardAndDelete(TList* list);
    AnsiString GetStringFromJSON(cJSON* root, AnsiString TagName);
};
//---------------------------------------------------------------------------
extern HTEditList *elConfig;
extern HTEditList *elUdUld;
extern HTEditList *elTrayForm;
extern HTEditList *elContact;
extern HTEditList *elLaser;
extern HTEditList *elConfig_byRecipe;                                           //Sam 20220921 : config儲存跟隨recipe
extern HTEditList *cbLastSet;
extern HTEditList *elVacuumUnit;                                                //Sam 20230210 : 新增 VacuumUnit 通訊模組
extern HTEditList *cbConfig_byRecipe;                                       //Eastsun 20251216 : config 存讀 recipe
extern HTEditList *elTeach;

//Steven 20260924 (S12-C)：控制項 → HTML 元件名稱。FileRW/_EditList.cpp 的具名替身 EL<T>("名稱") 建立時登記，
//   HTEditList::Add() 建立每一筆 THTEdit 時查表填入 THTEdit::ControlName。沒登記過的控制項回空字串。
void       HTEditList_RegisterControlName(TControl *Ctrl, const AnsiString &Name);
AnsiString HTEditList_ControlName(TControl *Ctrl);
//---------------------------------------------------------------------------
class uHPSuckTeam : public uBasicPickPlace                                      //大群組所用, 為機台的, 同一台叫的為同一批
{
private:    // User declarations
    void SavePlateRow(cJSON* root);
    void SavePlateCol(cJSON* root);
    void SavePlateSite(cJSON* root);
    void SavePlateSuck(cJSON* root);
    void ReadPlateRowFromJSON(cJSON* root);
    void ReadPlateColFromJSON(cJSON* root);
    void ReadPlateSiteFromJSON(cJSON* root);
    void ReadPlateSuckFromJSON(cJSON* root);
public:     // User declarations
    uHPSuckTeam();
    uHPSuckTeam(int iP, int iR, int iC, int iSht, int iKit);
    ~uHPSuckTeam();
    //
    int iP;
    int iR;
    int iC;
    int iSht;
    int iKit;
    int iSite[MAX_ARM_Row][MAX_ARM_Col];
    int iPlateR[MAX_ARM_Row][MAX_ARM_Col];
    int iPlateC[MAX_ARM_Row][MAX_ARM_Col];
    bool bSuck[MAX_ARM_Row][MAX_ARM_Col];

    int  GetUseSuckCol();                                                       //取得目前是哪個Col要吸嘴
    void ClearHPSuckTeam();
    //
    uHPSuckTeam* Clone();

    //To JSON
    void SaveJSONFile(cJSON* root);
    void LoadJSONFile(cJSON* root);
};
//---------------------------------------------------------------------------
class uHPSuckGroup:public uBasicPickPlace                                       //大群組, 一組就是 從吸嘴到放到HP 到下盤吸嘴放料 的資料
{
private:    // User declarations
    TList *HPSuckTeamList;
public:     // User declarations
    uHPSuckGroup();
    ~uHPSuckGroup();

    void AddHPSuckTeam(uHPSuckTeam* info);
    void AddHPSuckTeam(int iP, int iR, int iC, int iSht, int iKit);

    void DelHPSuckTeam(int iIndex);

    void ClearTeamList();
    int  GetTeamCount();                                                        //有幾組吸嘴組

    void DataForward();

    uHPSuckGroup*   Clone();
    uHPSuckTeam* ExtractLastTeam();
    uHPSuckTeam* ExtractSuckTeam(int iIndex);

    //To JSON
    void SaveJSONFile(cJSON* root);
    void LoadJSONFile(cJSON* root);
};
//---------------------------------------------------------------------------
class uPlateInfo:public uBasicPickPlace                                         //Jimmychiu 20230417 : Record the position after placing the IC
{
private:
    TList *HPSuckGroupList;
    int iLimitcount;
    uHPSuckTeam* SearchHPSuckTeamByCoordinate(int &iSuckRow, int &iSuckCol, int _iP, int _iRow, int _iCol); //根據加熱盤座標, 找哪組要使用的吸盤
    uHPSuckTeam* TempSuckTeam;
public:
    uPlateInfo();
    ~uPlateInfo();

    void AddHPSuckGroup();
    void AddHPSuckGroup(uHPSuckGroup* info);
    void UpdateHPSuckGroup(int iP, int iR, int iC, int iSht, int iKit);

    void ClearGroupList();
    void DelHPSuckGroup(int iIndex);

    bool SetPlateSuck(int _iP, int _iRow, int _iCol, bool bSuck);               //編輯已儲存的資料
    void SetArrPlateXY(int iSuckRow, int iSuckCol, int _iP, int iPlateR, int iPlateC, int _iSite=-1);

    int  GetHPSuckGroupCount();

    bool GetHPFirstTeam(int *iP, int (*PlateR)[MAX_ARM_Col], int (*PlateC)[MAX_ARM_Col], bool (*Suck)[MAX_ARM_Col]);    //取得第一筆資料的全部吸嘴資料
    bool GetHPFirstTeamPlate(int &iP, int &iR, int &iC, int &iSht, int &iKit);  //取得第一筆資料的加熱盤座標與Shuttle位置
    int  GetHPFirstTeamSuckCol();                                               //取得第一筆資料中, 最先的吸嘴的Col位置, 以便轉換的偏移位置
    bool GetHPFirstTeamSuckUse(bool bSuck[MAX_ARM_Row][MAX_ARM_Col]);           //取得第一筆資料中, 吸嘴資料是否需要交換
    bool GetHPFirstTeamMotUse(bool bSuck[MAX_ARM_Row][MAX_ARM_Col]);            //取得第一筆資料中, 吸嘴Z軸使用與否的資料
    bool GetHPFirstTeamToList(int *iP, TList* lsPoint2D);                       //取得第一筆資料, 並轉換成陣列

    bool GetHPTeamSuck(int iGroupIndex, int iTeamIndex, int *iP, int (*PlateR)[MAX_ARM_Col], int (*PlateC)[MAX_ARM_Col], bool (*Suck)[MAX_ARM_Col]);

    bool DataForwardAndNextTeam();                                              //確認是否需要多次吸放料, 並刪除舊資料

    void InsertHPGroup(int Index, void *Item);
    void TransferByuPlacePoint2DList(TList *Listpp2D);
    void AddDataByPlacePoint2D(uPlacePoint2D* pp2D);

    void SetTempSuckTeam(uHPSuckTeam* HPTeam);
    uHPSuckGroup*   ExtractLastGroup();                                         //新的, 同一新需要同時吸放
    uHPSuckGroup*   ExtractSuckGroup(int iIndex);
    uHPSuckTeam*    GetTempSuckTeam();
    uHPSuckTeam*    ExtractFirstTeam();                                         //舊的, 新組中可以分開取用吸嘴
    uHPSuckTeam*    ExtractLastTeam();
    uHPSuckTeam*    ExtractLastTeamHasData();                                   //最後一個可能是空的, 所以先檢查第二個
    uHPSuckTeam*    ExtractTeamByIndex(int iGroupIndex, int iTeamIndex);

    //To File
    void LoadFile(AnsiString sFileName);
    void SaveFile(AnsiString sFileName);
    void ResetFile(AnsiString sFileName);
};
//---------------------------------------------------------------------------
extern uPlateInfo *PickFromHPList;
extern uPlateInfo *PlaceToHPList;

extern uPlateInfo *PlaceToCleanList;

// AI(W906-BA-MW6) 20260911: allocate uPlateInfo ONLY through this factory, never
// with a bare `new uPlateInfo()` from a TU that cannot see the real class.
// See the long note at its definition in Public/HTEditList.cpp.
uPlateInfo *W906_NewPlateInfo();
//---------------------------------------------------------------------------
// golden `class uPoint2D` (HTEditList.h:249-259) is NOT redeclared here --
// see INTEGRATION HAZARDS (C) above. cmydef.h already declares it identically
// (fields X/Y, ctor()/ctor(int,int)/Clear()); this file supplies the 3 method
// bodies in HTEditList.cpp.
//---------------------------------------------------------------------------
class uPlacePoint2D                                                             //Jimmychiu 20230417 : Record the position after placing the IC
{
private:
public:
    uPlacePoint2D()
    {
        Clear();
    }
    ~uPlacePoint2D(){}
    int iPlate;
    int iPlateRow;                                                              //紀錄加熱盤的位置
    int iPlateCol;
    int iSuckRow;                                                               //紀錄吸嘴的位置
    int iSuckCol;
    int iSubStep;                                                               //吸嘴上IC對應加熱盤吸放料的次數
    int iHPStep;                                                                //加熱盤放料陣列的Index
    void Clear()
    {
        iPlate      =0;
        iPlateRow   =0;
        iPlateCol   =0;
        iSuckRow    =0;
        iSuckCol    =0;
        iSubStep    =0;
        iHPStep     =0;
    }
};
//---------------------------------------------------------------------------
typedef struct                                                                  //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    bool bZActByMode[MAX_ARM_Row][MAX_ARM_Col];
    bool bZActByOnOffSite[MAX_ARM_Row][MAX_ARM_Col];
    bool bZActByLimit[MAX_ARM_Row][MAX_ARM_Col];
    bool bZActive[MAX_ARM_Row][MAX_ARM_Col];
    int iYVariable;
    int iXVariable[X_PITCH_COUNT];
    int iZPosition[MAX_ARM_Row][MAX_ARM_Col];
    uPoint2D pArmXYPos;
    void Clear()
    {
        pArmXYPos.Clear();
        ZeroMemory(iXVariable , sizeof(iXVariable));
        iYVariable=0;
        ZeroMemory(bZActive   , sizeof(bZActive));
        ZeroMemory(iZPosition , sizeof(iZPosition));
    }
    bool GetZActive(int irow,int icol)
    {
        bZActive[irow][icol]=bZActByMode[irow][icol] && bZActByOnOffSite[irow][icol] && bZActByLimit[irow][icol];
        return bZActive[irow][icol];
    }
}ArmAndSuckInfo;
//---------------------------------------------------------------------------
typedef struct                                                                  //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    int iWhichAuto;
    uPoint2D pTrayRowCol;
    uPoint2D pArmXYPos;
    int iArmXV1;
    int iArmXV2;
    int iArmYV1;
    bool bPlace;
    void Clear()
    {
        iWhichAuto=-1;
        pTrayRowCol.Clear();
        pArmXYPos.Clear();
        iArmXV1=0;
        iArmXV2=0;
        iArmYV1=0;
        bPlace=false;
    }
}SingleSuckPosOnTray;
//---------------------------------------------------------------------------
typedef struct                                                                  //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    SingleSuckPosOnTray ssTarget;
    SingleSuckPosOnTray ssSource;
    uPoint2D pArmSuckActive;
    void Clear()
    {
        ssTarget.Clear();
        ssSource.Clear();
        pArmSuckActive.Clear();
    }
}MyFillTheTrayAfterOutArmPlace;
//---------------------------------------------------------------------------
typedef struct                                                                  //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
{
    ArmAndSuckInfo ssTarget;
    ArmAndSuckInfo ssSource;
    void Clear()
    {
        ssTarget.Clear();
        ssSource.Clear();
    }
}PickPlaceByArm;
//---------------------------------------------------------------------------
#endif
