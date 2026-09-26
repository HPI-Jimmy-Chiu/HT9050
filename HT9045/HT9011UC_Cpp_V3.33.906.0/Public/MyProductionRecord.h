// =============================================================================
//  Public/MyProductionRecord.h  -- TMyProductionRecord forward-stub
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/Public/MyProductionRecord.h
//  Translation wave: W4 (HAL motor layer, needed by mytray.h)
//  Translator: AI(W4) 20260626
//
//  STATUS: CLASS DECLARATION ONLY.  All method bodies are deferred to W3
//  (Public utils wave) because the .cpp pulls cMyDB / handlerlog / etc.
//  TMyTray holds `TMyProductionRecord *PordRec[][]` as pointers only, so
//  a forward-declaration-level header is sufficient for W4.
//
//  AI(W906-MyProductionRecord) 20260721: the PURE-LOGIC half is now REAL --
//  see Public/MyProductionRecord.cpp.  Ctor, InitialRecord,
//  DeleteProductionRecord, AddArmSiteRecord, and ~28 more Add*Record/Get*
//  leaves that only read/write asBuffer (TStringList*, indexed by the new
//  eMyProdRec enum below) got faithful bodies (golden .cpp:218-654).  Every
//  pre-existing declaration/comment from the W4/W6.2/W6.2b-2x4_16/
//  W906-AutoCleanFoundation waves is left VERBATIM in place below (not
//  reformatted, not reordered) -- new declarations are added additively
//  around them, each under its own AI(W906-MyProductionRecord) tag.
//
//  STILL OUT OF SCOPE (unchanged from this stub's original deferral -- NOT
//  declared here, matching golden .h:49-56/79-80, PLUS SaveRecordCleanPad
//  which the AutoCleanFoundation wave already declared below): SaveRecord/
//  SaveDataForASE/SaveDataForGreatek/SaveGroundESDData/SaveRecordCleanPad/
//  SaveRecordForLeadYo/Save2DIDForJCET/SaveJHT_Record/GetProdLogFileName/
//  GetProdLogFilePath (golden .cpp:660-1384; pull ProductionInfo.h/
//  atester.h/MyKitSuck.h/VacuumUnit.h/uLotInfo.h/FileInfo.h/BarCode.h/
//  main.h/mymessbox.h -- 9 of 10 not in the target tree at all).
//  AddIndexPickVacuum (golden .h:47) also stays undeclared: its golden body
//  sits at .cpp:1110, physically inside the 660-1384 heavy block even though
//  it is itself a pure leaf -- left for whichever wave finishes that block,
//  matching atester_32Site.cpp's own W5_32S_ADD_INDEX_PICK_VACUUM no-op
//  stand-in (still needed at that call site).
//  AddErrorRecord (golden .cpp:561-573) is ALSO deliberately left declared-
//  only (no body) this wave -- see Public/MyProductionRecord.cpp's own note
//  at the top for why (its one golden action besides the pure leaf logic is
//  calling the still-out-of-scope SaveRecord()).
//
//  KNOWN CONCURRENT-WAVE OVERLAP (flagged, NOT resolved, by this wave -- see
//  Public/MyProductionRecord.cpp's file-head note for the full citation): a
//  same-day sibling wave (AI(W906-AutoCleanFoundation) 20260721) independently
//  added its OWN no-op stand-in BODIES for 9 of these same methods directly
//  inside aHotPlateSubstrate.cpp (ctor, AddErrorRecord, AddHPRecord,
//  AddInArmHotplatePickRecord, AddTestRecord, GetInRotationAngRecord,
//  AddPickCleanPad, AddPickCleanPadFormShuttle, AddPlaceCleanPad) to unblock
//  its own AutoClean translation, and (separately) added declarations to
//  THIS header (AddArmSiteRecord, AddPickCleanPad/AddPickCleanPadFormShuttle/
//  AddPlaceCleanPad/SaveRecordCleanPad, the asBuffer member) while this wave
//  was reading it -- those declarations are kept VERBATIM below,
//  merged with this wave's own additions. Public/MyProductionRecord.cpp is
//  NOT wired into any shared CMake target this wave specifically to avoid a
//  "multiple definition" collision with the aHotPlateSubstrate.cpp stand-ins
//  before a human/integrate-agent reconciles the two (expected resolution:
//  delete the 9 aHotPlateSubstrate.cpp stand-in bodies once this file's real
//  ones supersede them).
// =============================================================================
#ifndef MyProductionRecordH
#define MyProductionRecordH

#include "vclcompat/vcl_compat.h"   // AnsiString, TStringList

// ------------------------------------------------------------------------------
//Steven 20160113 : 記錄每一顆IC 狀態
// ------------------------------------------------------------------------------
class TMyProductionRecord
{
public:
    TMyProductionRecord();
    void DeleteProductionRecord();
    void InitialRecord();

    void AddLoadRecord(int iNum, int iR, int iC, int iXPos, int iYPos,
                       AnsiString Name, AnsiString Time);    // Sam 20250211
    void AddHPRecord(int iPlate, int iR, int iC);
    void AddArmSiteRecord(int iArm, int iSite);              // Steven 20201204
    // AI(W6.2b-2x4_16) 20260626: golden MyProductionRecord.h:19 declares this
    // COMMENTED-OUT (`//void AddTestRecord(int iArm, int iSite);`); the dead
    // 2x4_16 in-arm place-to-shuttle SM still calls it.  Offline no-op record.
    void AddTestRecord(int iArm, int iSite);                // golden MyProductionRecord.h:19 (commented-out/dead)
    void AddTestResultRecord(int iBin, AnsiString SBin, AnsiString ErrorLog = "");
    void AddUnloadRecord(int iWhichAuto, int iNum, int iTrayRow, int iTrayCol,
                         int iXPos, int iYPos, AnsiString ascode,
                         int iXPitch, int iXPitch2, int iYPitch,
                         AnsiString TrayID);                 // Frank 20160727
    void AddIndexCycleTimeRecord(double findexCycleTime);
    void AddOrderTestRecord(int iNum);
    void AddTestTime(AnsiString asStr);
    void AddTSDTime(AnsiString asStr);
    void Add2DIDRecord(AnsiString s2DID);
    void AddDataTimeRecord(AnsiString asStr);
    // AI(W906-MyProductionRecord) 20260721: golden .h:29 -- declared here
    // (pre-existing), body deliberately still deferred alongside SaveRecord
    // (see file-head banner + MyProductionRecord.cpp's matching note).
    void AddErrorRecord(AnsiString ErrorLog, bool bAddPosition = false,
                        int iXPos = 0, int iYPos = 0,
                        int iXPitch = 0, int iXPitch2 = 0, int iYPitch = 0);
    void AddErrorLog(AnsiString ErrorLog);                   // Steven 20200518
    // AI(W906-MyProductionRecord) 20260721: golden .h:31-36 -- AddErrorRecord's
    // sibling that skips the SaveRecord() tail (golden .cpp:575-587), plus the
    // Sam 20200623/20201216 "Add record" leaf family.  Pure leaves, real
    // bodies this wave.
    void AddErrorRecordNoSave(AnsiString ErrorLog, bool bAddPosition = false,
                        int iXPos = 0, int iYPos = 0,
                        int iXPitch = 0, int iXPitch2 = 0, int iYPitch = 0);   // Steven 20190110
    void AddInArmLoaderPickRecord(int iSuckRow, int iSuckCol);               // Sam 20200623
    void AddIndexPickShuttleRecord(int iSite, int iSiteRow, int iSiteCol);   // Sam 20201216/20200623
    void AddIndexPlaceShuttleRecord(int iSite, int iSiteRow, int iSiteCol);  // Sam 20201216/20200623
    void AddOutShuttleDetectRecord(int iSite, int iSiteRow, int iSiteCol);   // Sam 20201216/20200623
    void AddOutArmShuttlePickRecord(int iSuckRow, int iSuckCol);             // Sam 20200623
    // AI(W6.2-INARM) 20260626: two records the W6.2 HP pick/place geometry
    // leaves (ainarm_SearchPickPlate/PlacePlate) call.  Golden homes:
    //   AddInArmHotplatePickRecord -- golden MyProductionRecord.h (Sam 20200716)
    //   GetInRotationAngRecord     -- golden MyProductionRecord.h (Sam 20221103)
    void       AddInArmHotplatePickRecord(int iSuckR, int iSuckC);  // Sam 20200716
    AnsiString GetInRotationAngRecord();                            // Sam 20221103
    // AI(W906-MyProductionRecord) 20260721: golden .h:38-42 -- real bodies
    // this wave.
    void AddTestModeRecord(int iTestMode);                          // Sam 20201216
    void AddDataTimeEOTRecord(AnsiString asStr);                    // wei 20181211
    void AddGroundRecord(AnsiString sGround);                       // Sam 20211223
    void AddESDRecord(AnsiString sESD);                             // Sam 20211223
    void AddInRotationAngRecord(int iAngle);                        // Sam 20221103

    // AI(W906-AutoCleanFoundation) 20260721: golden Public/MyProductionRecord.h
    // (Sam 20230616, "Add Auto Clean Record") -- the 3 AutoClean pick/place
    // recorders + the CSV-log flush DoPlaceToKitSwapData/PickFromCleanKit/
    // PlaceToCleanKit call. Golden bodies do real file I/O (asBuffer->Strings[]
    // writes + a CSV-append with directory creation); this class is
    // "declaration-only, bodies deferred" for EVERY method (see file banner
    // above + aHotPlateSubstrate.cpp's existing AddErrorRecord/AddHPRecord
    // no-ops) -- these 4 are added as no-ops for the same reason, not a new gap.
    void AddPickCleanPad(int iSuckRow, int iSuckCol, int iKitRow, int iKitCol, int iData); // Sam 20230616
    // AI(W906-MyProductionRecord) 20260721: golden .h:44 -- the sibling
    // AddPlaceCleanPadToShuttle recorder the AutoCleanFoundation block above
    // did not declare; real body this wave (same shape as the 3 siblings here).
    void AddPlaceCleanPadToShuttle(int iSuckRow, int iSuckCol, int iShtRow, int iShtCol);  // Sam 20230616
    void AddPickCleanPadFormShuttle(int iSuckRow, int iSuckCol, int iShtRow, int iShtCol); // Sam 20230616
    void AddPlaceCleanPad(int iSuckRow, int iSuckCol, int iKitRow, int iKitCol);           // Sam 20230616
    void SaveRecordCleanPad(AnsiString ErrorLog = "na");                                   // Sam 20230616

    // =========================================================================
    // AI(W906-PT-MyProdRec-HeavyHalf) 20260811 -- HEAVY-HALF DECLARATIONS
    // -------------------------------------------------------------------------
    // SUPERSEDES this header's own head banner lines 23-39 ("STILL OUT OF
    // SCOPE ... SaveRecord/SaveDataForASE/SaveDataForGreatek/SaveGroundESDData/
    // SaveRecordCleanPad/SaveRecordForLeadYo/Save2DIDForJCET/SaveJHT_Record/
    // GetProdLogFileName/GetProdLogFilePath ... AddIndexPickVacuum also stays
    // undeclared ... AddErrorRecord is ALSO deliberately left declared-only").
    // Those statements are no longer true: Public/MyProductionRecord.cpp's
    // appended WAVE-2 section now carries real bodies for all of them except
    // the three noted below.  The old banner text is left VERBATIM above rather
    // than rewritten, so this block is the authority where the two disagree.
    //
    // Signatures below are golden Public/MyProductionRecord.h:47/49-56/79-80,
    // byte-for-byte including the `="na"` defaults.  Class layout is NOT
    // restructured: these are appended inside the same single `public:` region,
    // after the existing AutoClean block and before the `asBuffer` member, so
    // no existing declaration moved.
    //
    // BODY STATUS (see the .cpp's GATE REGISTER for the full entries):
    //   AddIndexPickVacuum / SaveRecord / SaveDataForASE / SaveDataForGreatek /
    //   SaveGroundESDData / SaveRecordForLeadYo / GetProdLogFileName /
    //   GetProdLogFilePath / SaveJHT_Record ......... REAL body, active.
    //   Save2DIDForJCET ............................. DEFINED but body gated
    //       (G-4: fBarCode->iJCETWhitelistSN absent from TfBarCode).
    //       Declaring it is safe -- it is defined, so no undefined reference.
    //   AddErrorRecord (already declared above) ..... REAL body, active since AI(W906-W2-PRODLOG) 20260926 (G-5 lifted; next line is history:)
    //       no-op stand-in remains at aHotPlateSubstrate.cpp:773.
    //   SaveRecordCleanPad (already declared above) . REAL body, active since AI(W906-W2-CLEANPADLOG) 20260926 (G-6 lifted; next line is history:)
    //       no-op stand-in remains at aHotPlateSubstrate.cpp:775.
    //
    // TRAP-2 NOTE FOR THE MAIN LOOP (a premise this block KILLS, in a file this
    // wave may not touch): asortarm.cpp:4177's gate says
    //   "#if 0 // GATE(10) ... TMyProductionRecord::SaveRecord is explicitly out
    //    of scope tree-wide and not even declared -- Public/MyProductionRecord.h:24-30"
    // and asortarm.cpp:377-381 repeats it.  SaveRecord IS declared as of this
    // block, so that stated premise is now FALSE.  The gate is still compiling
    // (it is #if 0) so nothing breaks, but the justification must be rewritten
    // or the gate opened -- and if opened, read the .cpp's G-5(b) first: it
    // makes OutArm2Suck rows append to the real D:\HT9045_Log\Production_Log.
    // Same for aTester_Front.cpp:11662 / aTester_Rear.cpp:12313 /
    // atester_32Site.cpp:233, whose FIRST premise ("AddIndexPickVacuum is not
    // even DECLARED") dies here -- but whose SECOND premise (TMySucker has no
    // GetIOValue()) is untouched and still holds, so those gates must STAY.
    // =========================================================================
    void AddIndexPickVacuum(double dKpa);                                   // golden .h:47  Sam 20230210
    void SaveRecord(AnsiString ErrorLog="na");                              // golden .h:49
    void SaveJHT_Record(AnsiString ErrorLog="na");                          // golden .h:50  JerryYang 20240405
    void SaveDataForASE(AnsiString sPcName);                                // golden .h:51
    void SaveDataForGreatek(AnsiString asPath);                             // golden .h:52  Sam 20200707
    void SaveGroundESDData(AnsiString sLot);                                // golden .h:53  Sam 20211223
    void SaveRecordForLeadYo();                                             // golden .h:55  KenHsieh 20230727
    void Save2DIDForJCET(AnsiString SubLOTID, AnsiString CustLOTID,
                         AnsiString Modecode);                              // golden .h:56  RogerYang 20251210
    AnsiString GetProdLogFileName(bool bTryCatch);                          // golden .h:79
    AnsiString GetProdLogFilePath();                                        // golden .h:80

    // AI(W906-AutoCleanFoundation) 20260721: golden TMyProductionRecord has an
    // `asBuffer` TStringList* member (per-record scratch buffer that AddXxxRecord
    // methods write AnsiString fields into, and SaveRecord* later flush to CSV).
    // Needed ONLY so TMyKitSuck::MoveSuckDataDiff's faithful
    // `PordRec[Target].asBuffer->CommaText=Source.PordRec[Source].asBuffer->CommaText;`
    // copy line (golden MyKitSuck.cpp:1528) compiles+behaves correctly; the
    // AddXxxRecord/SaveRecord* bodies that would populate it stay no-op/deferred
    // per this class's existing convention -- copying two empty lists is a safe,
    // deterministic no-op, consistent with the rest of this stub class.
    // AI(W906-MyProductionRecord) 20260721: asBuffer is now the REAL scratch
    // buffer (ctor allocates + fills it; every Add*Record/Get* leaf above
    // reads/writes it for real) -- the comment above's "copying two empty
    // lists" premise no longer holds once bodies are real, but the member
    // declaration itself is unchanged (kept verbatim; still the sole
    // `TStringList *asBuffer;` for the class).
    TStringList *asBuffer;

    // AI(W906-MyProductionRecord) 20260721: golden .h:58 `bool bUse;` member --
    // AddLoadRecord (sets true)/InitialRecord (sets false) need it; SaveRecord's
    // own bUse read stays moot (out of scope). Declared here, matching golden's
    // member position (right after the Add* block, before the Get* family).
    bool bUse;

    // AI(W906-MyProductionRecord) 20260721: golden .h:60-72 Get* accessor
    // family -- real bodies this wave (all read-only asBuffer[]->atoi/string).
    int GetAutoX();
    int GetAutoY();
    int GetWhcihAuto();
    int GetLoaderX();
    int GetLoaderY();
    int GetLoaderNum();                                             // Frank 20181121
    int GetSiteNo();
    int GetOrderOfContact();
    AnsiString GetSOTTime();
    AnsiString GetEOTTime();
    AnsiString GetTestTime();
    AnsiString GetErrorLog();
    // NOTE: GetInRotationAngRecord (golden .h:72) is already declared above,
    // grouped with AddInArmHotplatePickRecord by the pre-existing W6.2-INARM
    // block -- not redeclared here.

    // AI(W906-MyProductionRecord) 20260721: golden .h:73-78 title-string
    // members the ctor (real this wave) initializes.
    AnsiString asDataTitle;                                         // Steven 20200416
    AnsiString asDataTitleGreatek;
    AnsiString asDataTitle_SG;                                       // Sam 20211223
    AnsiString asDataTitleCleanPad;                                  // Sam 20230616
    AnsiString asJCET2DIDList;                                       // RogerYang 20251210

    // AI(W906-MyProductionRecord) 20260721: golden .h:49-56/79-80 declares the
    // ~9 Save* methods + GetProdLogFileName/GetProdLogFilePath here too --
    // deliberately NOT declared (see file-head banner: OUT OF SCOPE, 9 of 10
    // needed headers not in the target tree at all). SaveRecordCleanPad is the
    // one exception already declared above (AutoCleanFoundation wave); left as-is.
};
// ------------------------------------------------------------------------------
// AI(W906-MyProductionRecord) 20260721: golden Public/MyProductionRecord.h:83-157
// verbatim (every enumerator value byte-for-byte identical to golden -- asBuffer
// is indexed by this enum, so the numeric values are load-bearing, not just names).
enum eMyProdRec
{
    eScheduleName           =0,
    eStartTime              =1,
    eLoadNum                =2,
    eLoadX                  =3,
    eLoadY                  =4,
    eLoadTime               =5,
    eHotX                   =6,
    eHotY                   =7,
    eHotTime                =8,
    e2DCode                 =9,
    eSiteNO                 =10,                                                //1-32
    eArmNO                  =11,                                                //0: arm1  1: arm2
    eArmTime                =12,
    eOrderTest              =13,                                                //touch-down
    eSOTTime                =14,                                                //kevin 20140918  yyyymmdd_hhmmss
    eIndexCycleTime         =15,                                                //第一顆測試 是 0 (1st device)
    eTestCate               =16,                                                //測試 BIN碼
    eWhichAuto              =17,
    eOutTrayNum             =18,                                                //Auto1 :O1-1  Auto2 :2-1  Auto3 :3-1  Fix 1:4-1 Fix 2:5-1 Fix 3:6-1
    eUnloadX                =19,
    eUnloadY                =20,
    eUnloadTime             =21,
    eUnloadXPos             =22,                                                //Frank 20160727 add
    eUnloadYPos             =23,                                                //Frank 20160727 add
    eUnloadPosTime          =24,
    eErrorCode              =25,
    eOCRCode                =26,                                                //wei 20161005
    eTestTime               =27,
    eTSDTime                =28,
    eEOTTime                =29,                                                //wei 20181211 更換設定SOT
    eInArmLoaderPick        =30,                                                //Sam 20200623 : Add record
    eIndexPickSHTSiteNo     =31,                                                //Sam 20200623 : Add record
    eIndexPlaceSHTSiteNo    =32,                                                //Sam 20200623 : Add record
    eOutSHTDetectSiteNo     =33,                                                //Sam 20200623 : Add record
    eOutArmShuttlePick      =34,                                                //Sam 20200623 : Add record
    eHotplateNo             =35,                                                //Sam 20200716 : Add record
    eInArmHotplatePick      =36,                                                //Sam 20200716 : Add record
    eTestMode               =37,                                                //Sam 20201216 : Add record
    eUnloadXpPos            =38,
    eUnloadXp2Pos           =39,
    eUnloadYpPos            =40,
    eTrayForm               =41,                                                //Sam 20210907 : Add record
    eHotPlateForm           =42,                                                //Sam 20210907 : Add record
    eTrayID                 =43,                                                //Steven 20220120 : Amlogic需要輸出SBIN
    eSBin                   =44,                                                //Steven 20220120 : Amlogic需要輸出SBIN
    eGround                 =45,                                                //Sam 20211223 : 每顆 IC 測試完成後要記錄測試當時的 Ground & ESD 數值。
    eESD                    =46,                                                //Sam 20211223 : 每顆 IC 測試完成後要記錄測試當時的 Ground & ESD 數值。
    eInRotationAngle        =47,                                                //Sam 20221103 : 新增In料旋轉角度
    ePickCleanPadData       =48,                                                //Sam 20230616 : Add Auto Clean Record
    ePickCleanPadTime       =49,
    ePickCleanPadSuck       =50,
    ePickCleanPadRow        =51,
    ePickCleanPadCol        =52,
    ePlaceCleanPadToShtTime =53,
    ePlaceCleanPadToShtSuck =54,
    ePlaceCleanPadToShtRow  =55,
    ePlaceCleanPadToShtCol  =56,
    ePickCleanPadFormShtTime=57,
    ePickCleanPadFormShtSuck=58,
    ePickCleanPadFormShtRow =59,
    ePickCleanPadFormShtCol =60,
    ePlaceCleanPadTime      =61,
    ePlaceCleanPadSuck      =62,
    ePlaceCleanPadRow       =63,
    ePlaceCleanPadCol       =64,
    eLoaderPickVacuum       =65,                                                //Sam 20230210 : 新增 VacuumUnit 通訊模組
    eHPPickVacuum           =66,
    eOSHTPickVacuum         =67,
    eIndexPickVacuum        =68,
    eLoadXPos               =69,                                                //Sam 20250211 : Add Load Pos
    eLoadYPos               =70,
    eDataTotal                                                                  //扣在最下面
};
// ------------------------------------------------------------------------------
#endif // MyProductionRecordH
