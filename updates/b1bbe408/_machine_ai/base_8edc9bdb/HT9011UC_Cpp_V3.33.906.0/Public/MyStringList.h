// =============================================================================
//  Public/MyStringList.h  --  TMyStringList : the file-backed log-line accumulator
//
//  Faithful translation of golden Public/MyStringList.h (89 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-MyStringList) 20260807
//  Translation wave: W906-PT-MyStringList (pure-translation completion campaign;
//    Public/ leaf utility, direct sibling of Public/MyProductionRecord.{h,cpp},
//    Public/cBootLog.{h,cpp} and Public/ExternFunction.{h,cpp} already in this
//    directory).  This is the type behind cmydef.h:15's tree-wide OPAQUE forward
//    declaration `class TMyStringList;` and the five globals declared against it:
//    cmydef.h:117 slEventLog, :118 sl2DMappingLog, :140 slHanaTrayMap[eTrayCount],
//    :5033 tsSoftwareExeTime, :5231 slGroundManLog.
//
//  ROLE: a TStringList subclass that buffers log lines in a SEPARATE inner
//  TStringList (`MyList`, golden :72 -- see its own Chinese note there) and
//  flushes them to a date/hour-partitioned .csv/.TXT file whose name the object
//  computes itself (GetFileName).  It owns:
//    * the save-policy fields (Path / FileName / FirstRow / MaxLineCount /
//      SaveType / AutoSave / SaveSameFolder / SaveByLotID / FixedFile),
//    * the timestamp-prefixing Add* family (4 formats + a plain AddText),
//    * the file-name policy engine GetFileName() (11 TSaveType branches, the
//      08:00-vs-00:00 N10 upload split, and the FT/RT + Cypress-offline suffixes),
//    * the by-Lot-ID mirror (SetLotData / sLotFileName),
//    * four specialised writers: MySaveFileByFileNameAndType, MySaveFileByFileName,
//      MySaveSGJamCountToFile, MyInsert2DMappingToFile,
//    * and the free function SaveTryCatchLog (golden :17 / .cpp:671) -- which is
//      NOT a member; it is a plain file-append helper that lives in this unit.
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim): the WHOLE header.  All 28 member functions and
//     the 1 free function are declared here and DEFINED in MyStringList.cpp.
//   SATISFIED-BY-SHIM (so it LINKS): none -- this unit introduces no shim TU.
//   GATE (#if 0): NONE.
//
//  VCL/Borland conversions: golden has no #pragma in the header.  __fastcall is
//  DROPPED from every declaration (and correspondingly from every definition in
//  the .cpp) -- the house convention stated in acarry.h:8 / acatchtray.h:8 /
//  ainarm9045.h:9 ("No __fastcall / __property / PACKAGE in this header").
//  __fastcall is a REAL MinGW keyword, so header and .cpp MUST agree; they do.
//  __published is dropped.  __property is hand-translated -- see the PROPERTY
//  TRANSLATION block below, which is the one non-mechanical decision in this file.
//  Numeric semantics kept EXACT (no int/float rewrites).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
//
//  ---------------------------------------------------------------------------
//  TSaveType RELOCATED INTO THIS HEADER -- READ BEFORE ADDING MyMemo.h
//  ---------------------------------------------------------------------------
//  Golden MyStringList.h:6 is `#include "MyMemo.h"`, and MyMemo.h is where
//  `enum TSaveType` actually lives.  MyMemo.h is NOT in the golden source tree
//  at all: it is a Borland VCL *component package* header, and the only copy on
//  this machine is D:\HT9045\elec\Component\MyMemo.h (lines 11-24 -- verified by
//  reading it; an exhaustive grep over the whole golden 906 tree finds ZERO
//  other definition of TSaveType).  Its other content is `class PACKAGE TMyMemo
//  : public TMemo`, a pure-VCL TMemo descendant with no place in this port.
//
//  Rather than translate a whole VCL component package for one enum, the enum is
//  reproduced here VERBATIM (same names, same explicit values) behind its own
//  guard.  The guard is deliberately NOT `MyMemoH`: if a future wave really does
//  port MyMemo.h, whichever header is seen first wins and the other's copy is
//  skipped, with no redefinition error in either include order.
//
//  The explicit values are LOAD-BEARING, not decoration: MyStringList.cpp:487
//  and :491 compare with `>=` (`HTSaveType>=TByMonth`, `HTSaveType>=TByDay`), so
//  the ORDINAL ORDER decides which folder layout is used.  Note in particular
//  that TByMin=10 sorts ABOVE TByMonth=8, so a TByMin log gets the by-YEAR
//  folder -- see the fidelity note at that call site in the .cpp.
//
//  ---------------------------------------------------------------------------
//  PROPERTY TRANSLATION -- why a proxy, not the usual getter/setter pair
//  ---------------------------------------------------------------------------
//  The house convention for BCB6 `__property` is a plain getter method
//  (cpublic.h:346-358, EtherCAT/MyNUEC1.h:237, Automation/uRENESAS_Other.h:255).
//  Every one of those precedents is a READ-ONLY property whose call sites can be
//  rewritten to `X()`.  This class is different in a way that decides the design:
//
//    1. Golden's own bodies ASSIGN through the properties -- MyStringList.cpp:20-23
//       (`MaxLineCount=1000; Path="D:\\HandlerLog"; FirstRow=""; SaveType=TByDay;`)
//       and :37-47 -- so the write side is not optional even inside this unit.
//    2. External call sites that are gated TODAY purely because this type was
//       opaque also assign through them, and they are OUTSIDE this wave's write
//       boundary, so they must compile unchanged when they are un-gated:
//         cprod.cpp:2503-2545  slEventLog->FileName= / ->SaveType= /
//                              ->SaveSameFolder= / ->SaveByLotID=
//         cMyDB.cpp:350/402/450/747  slEventLog->AddTextWith*()
//         Automation/HANA_ART.cpp:1006  slHanaTrayMap[...]-> ...
//       Turning the properties into `SetFileName(...)` calls would require
//       editing those files -- exactly what this wave may not do.
//
//  So each property is a tiny proxy object that mirrors the BCB6 declaration
//  structurally: it holds a pointer-to-DATA-MEMBER for `read=<field>` and a
//  pointer-to-MEMBER-FUNCTION for `write=<setter>`, so a read returns the field
//  and a write really does route through golden's setter (SetPath, SetFileName,
//  ... ) rather than bypassing it.  This is also the pattern vclcompat itself
//  already uses one layer down for the same problem -- vclcompat/TStringList.h's
//  TextProxy / CommaTextProxy / StringsProxy (:111-140) exist for exactly this
//  "BCB6 property assigned at the call site" reason.
//
//  The `default=` clauses in golden's __property lines are BCB6 .dfm-streaming
//  hints, NOT initialisers -- they are preserved as comments only, and the real
//  initial values come from the constructors (see the .cpp).
// =============================================================================
//
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   //---------------------------------------------------------------------------
//   #ifndef MyStringListH
//   #define MyStringListH
//   #include "MyMemo.h"
// =============================================================================

#ifndef MyStringListH
#define MyStringListH

#include "vclcompat/vcl_compat.h"   // AnsiString, TStrings/TStringList, Word (all `using`-imported into the global namespace)

//---------------------------------------------------------------------------
/*enum TMySaveType                                                              //AI(W906-PT-MyStringList) 20260807: golden :8-15, already commented out in golden -- kept as a comment, verbatim
{
    TByMaxLineCount=0,
    TByHour=1,
    TByDay=2,
    TByMonth=3,
    TByYear=4
};*/
//---------------------------------------------------------------------------

//AI(W906-PT-MyStringList) 20260807: TSaveType hoisted here from the untranslated
//  VCL component header MyMemo.h (D:\HT9045\elec\Component\MyMemo.h:11-24) --
//  verbatim names and values.  See the "TSaveType RELOCATED" block in the file
//  banner for why, and for why the guard is not spelled MyMemoH.
#ifndef HT9045_TSAVETYPE_DEFINED
#define HT9045_TSAVETYPE_DEFINED
enum TSaveType
{
    TByMaxLineCount =0,
    TByHour         =1,
    TBy2Hour        =2,
    TBy4Hour        =3,
    TBy6Hour        =4,
    TBy8Hour        =5,
    TBy12Hour       =6,
    TByDay          =7,
    TByMonth        =8,
    TByYear         =9,
    TByMin          =10         //Steven 20210520 : 加入By分鐘存檔
};
#endif // HT9045_TSAVETYPE_DEFINED
//---------------------------------------------------------------------------
void SaveTryCatchLog(AnsiString asMessage, AnsiString Func);                    //KaiChen 20200706 ：增加 Try Catch 抓問題

class TMyStringList;                                                            //AI(W906-PT-MyStringList) 20260807: forward decl for the property proxy below

//---------------------------------------------------------------------------
//AI(W906-PT-MyStringList) 20260807: TMSLProperty<V> -- the hand translation of
//  BCB6 `__property V Name={read=<field>, write=<Setter>}` (golden :78-86).
//  Reading yields the field; writing calls the setter, exactly as BCB6 does.
//  Access note: the pointers-to-member are FORMED inside TMyStringList (in the
//  private BindProperties() member, .cpp), which is where C++ applies access
//  control; USING them through ->* here is not a named access and is therefore
//  legal even though the targets are golden-private.
//  The member functions are only instantiated at their point of use, by which
//  time TMyStringList is complete -- so this template may sit above the class.
//---------------------------------------------------------------------------
template <typename V>
class TMSLProperty
{
    public:
        typedef V     TMyStringList::*TReadField;                               // mirrors  read=<field>
        typedef void (TMyStringList::*TWriteMethod)(V);                         // mirrors write=<setter>

        TMSLProperty() : Owner(0), ReadField(0), WriteMethod(0) {}

        void Bind(TMyStringList *AOwner, TReadField AField, TWriteMethod AMethod)
        {
            Owner       =AOwner;
            ReadField   =AField;
            WriteMethod =AMethod;
        }

        operator V() const { return Owner->*ReadField; }                        // read=<field>

        TMSLProperty& operator=(const V &AValue)                                // write=<setter>
        {
            (Owner->*WriteMethod)(AValue);
            return *this;
        }
        TMSLProperty& operator=(const TMSLProperty &AOther)                     // property-to-property; must NOT copy Owner/ReadField/WriteMethod
        {
            (Owner->*WriteMethod)(static_cast<V>(AOther));
            return *this;
        }

    private:
        TMyStringList *Owner;
        TReadField     ReadField;
        TWriteMethod   WriteMethod;
};

//---------------------------------------------------------------------------
class TMyStringList : public TStringList
{
    private:
        AnsiString  HTPath;
        AnsiString  HTFileName;
        AnsiString  HTFirstRow;
        AnsiString  HTLotID;                                                    //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
        AnsiString  HTLotStartTime;                                             //Steven 20250520 : 加入By Lot存檔
        int         HTMaxLineCount;
        TSaveType   HTSaveType;
        bool        HTAutoSave;
        bool        HTSaveSameFolder;                                           //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾
        bool        HTSaveByLotID;                                              //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
        bool        HTSaveFixedFile;                                            //Steven 20250716 : 固定檔名存檔
        void SetPath(AnsiString P);
        void SetFirstRow(AnsiString P);
        void SetMaxLineCount(int Cnt);
        void SetSaveType(TSaveType Type);
        void SetFileName(AnsiString P);
        void SetAutoSave(bool P);
        void SetSaveSameFolder(bool P);                                         //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾
        void SetSaveByLotID(bool P);                                            //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
        void SetSaveFixedFile(bool P);                                          //Steven 20250716 : 固定檔名存檔
        Word SystemHour, SystemMin, SystemSec, SystemMSec;
        Word SystemYear, SystemMonth, SystemDate;
        void GetTimeInfo();

        //AI(W906-PT-MyStringList) 20260807: two ctor helpers with NO golden
        //  counterpart -- they exist only to reproduce BCB6 semantics this port's
        //  plain-C++ TStringList base does not provide.  Both are called as the
        //  FIRST statements of both constructors; see their bodies in the .cpp.
        void BindProperties();                                                  // binds the __property proxies to this instance
        void ZeroInitVclFields();                                               // reproduces BCB6 TObject's zero-fill of the instance
    protected:
    public:
        TMyStringList();
        TMyStringList(AnsiString sPath, AnsiString sFileName, AnsiString sFirstRow);
        ~TMyStringList();
        void AddText(AnsiString Msg);
        void AddTextWithLineNo(AnsiString Msg);
        void AddTextWithDateTime(AnsiString Msg);
        AnsiString AddTextWithDateTime2(AnsiString Msg);
        void AddTextWithDateTime3(AnsiString Msg, AnsiString Msg2="");          //Steven 20231120 : 把MSec分開存檔
        void MySaveToFile();
        void MySaveToFileShareMode();                                           //Jimmychiu 20250313 : Write data in shared mode
        void MyInsertToFile(AnsiString Msg, int iCount);                        //Steven 20191016 : 紀錄目前Alarm在檔案裡面的行數
        int GetLastLine();                                                      //Steven 20191016 : 取得目前檔案的行數

        AnsiString sLastFileName;                                               //Steven 20191107 : 紀錄現在的檔名
        AnsiString sLastNameNoPath;                                             //Steven 20191107 : 紀錄現在的檔名
        AnsiString sLastPathName;
        AnsiString sPrevFileName;                                               //Steven 20250809 : 紀錄前一個的檔名
        AnsiString sLotFileName;                                                //Steven 20250809 : 修正event log上傳
        bool bChangeFile;
        AnsiString GetFileName();
        bool bFilePathWithDate;
        bool bUseFTRT;
        bool bHanaTrayMap;                                                      //Steven 20250414 : HANA ART Function
        void MySaveFileByFileNameAndType(AnsiString asFileName, AnsiString asLotId, AnsiString asFileType);  //Sam 20171215 (wei) : 檔名尾巴增加JAM/INPUT //Sam 20170428 (wei) 增加帶入 LotId //RogerYAng 20170406 帶入檔案名稱
        void MySaveFileByFileName(AnsiString asPathName, AnsiString asFileName);   //Sam 20170516 (wei) 力成 SocketID Count 保存
        TStringList *MyList;                                                    //因為建構跟解構時,還沒有畫面,用到VCL得東西會死翹翹,所以用StringList做中介
        void MySaveSGJamCountToFile(bool bDelete, bool bflag);                  //KaiChen 20200618 ：矽格，增加Jam統計頁面
        void MyInsert2DMappingToFile(AnsiString s2DID, int iPlaceToUnload=-1);  //JerryYang 20230322 : add 2D mapping result
        void SetLotData(AnsiString ID, AnsiString Time, AnsiString LotFileName="");

    //AI(W906-PT-MyStringList) 20260807: golden's `__published:` section (golden
    //  :77-86).  __published is dropped (no RTTI streaming in this port); each
    //  __property becomes a TMSLProperty proxy of the SAME NAME and SAME type,
    //  bound in BindProperties() to the SAME read-field / write-setter pair the
    //  golden declaration names.  Golden lines kept verbatim above each proxy so
    //  the read/write/default triple stays reverse-lookupable.
    // __property AnsiString   Path                ={read=HTPath,                  write=SetPath,                  default=NULL};
    // __property AnsiString   FileName            ={read=HTFileName,              write=SetFileName,              default=NULL};
    // __property AnsiString   FirstRow            ={read=HTFirstRow,              write=SetFirstRow,              default=NULL};
    // __property int          MaxLineCount        ={read=HTMaxLineCount,          write=SetMaxLineCount,          default=1};
    // __property TSaveType    SaveType            ={read=HTSaveType,              write=SetSaveType,              default=TByDay};
    // __property bool         AutoSave            ={read=HTAutoSave,              write=SetAutoSave,              default=true};
    // __property bool         FixedFile           ={read=HTSaveFixedFile,         write=SetSaveFixedFile,         default=false};     //Steven 20250716 : 固定檔名存檔
    // __property bool         SaveSameFolder      ={read=HTSaveSameFolder,        write=SetSaveSameFolder,        default=false};   //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾
    // __property bool         SaveByLotID         ={read=HTSaveByLotID,           write=SetSaveByLotID,           default=false};   //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
        TMSLProperty<AnsiString>    Path;
        TMSLProperty<AnsiString>    FileName;
        TMSLProperty<AnsiString>    FirstRow;
        TMSLProperty<int>           MaxLineCount;
        TMSLProperty<TSaveType>     SaveType;
        TMSLProperty<bool>          AutoSave;
        TMSLProperty<bool>          FixedFile;                                  //Steven 20250716 : 固定檔名存檔
        TMSLProperty<bool>          SaveSameFolder;                             //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾
        TMSLProperty<bool>          SaveByLotID;                                //KaiChen 20181121 ：矽格-北興 Save Event Log by Lot ID
};

#endif
