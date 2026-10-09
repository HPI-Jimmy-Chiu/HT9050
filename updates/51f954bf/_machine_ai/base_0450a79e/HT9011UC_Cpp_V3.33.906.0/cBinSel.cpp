// =============================================================================
//  cBinSel.cpp  --  FW-3 表單波 (FW-BinSel-WA): TfBinSel, WAVE A subset
//
//  Translation wave: FW-BinSel-WA
//  Translator: AI(W906-FW-BinSel-WA) 20260819
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/cBinSel.cpp (6,652 lines)
//  + cBinSel.h (285 lines), cp950/Big5. Decoded this wave with
//  `python3 -c "open(path,'rb').read().decode('cp950')"` -- 0 U+FFFD over
//  both files (measured before any line below was written).
//
//  See forms/fBinSel.h for the full WAVE A / WAVE B split, GATE REGISTER,
//  WRITE-PATH GATE TABLE, CTOR NOTE, DESIGN NOTE and STUB COLLISION SCAN --
//  not duplicated here to avoid the two files drifting apart.
//
//  ABSENCE-CLAIM TIMESTAMPS (commands + when run, this wave, before writing
//  the citing code below -- re-run at hand-off per project policy)
//  --------------------------------------------------------------------------
//    fQAMode           : `grep -rn "\bfQAMode\b" --include=*.h .` -- 0 hits
//AI(W906-FW3-BTQ1) 20260828: SUPERSEDED -- FW3-BTQ1 新增 forms/fQAMode.{h,cpp}，fQAMode 現在有宣告。
//                         (20260819).
//    AutoForm[]        : ⚠ AI(W906-FW-UNGATE-W29) 20260826 -- 下面這條
//                         claim **在寫下當時就是假的**，不是過期：
//                         cprod.h:1366 的 `extern TRAY_TYPE_PARA
//                         *AutoForm[eTrayCount];` 早於 20260626 就存在。
//                         GATE (G2) 已於本波開閘（見 ReadFile 內的註解）。
//                         原文保留為沿革：
//                       `grep -rn "AutoForm\[" --include=*.h .` -- 0 hits
//                         (20260819).
//    fMain->SetOpenBin : `grep -n "SetOpenBin" forms/fMain.h` -- 0 hits
//                         (20260819).
//    fShowBinSelect->ShowBinSel : forms/fShowBinSelect.h's OWN WAVE B QUEUE
//                         (that file's banner, golden :388-757) -- not this
//                         wave's gap, cited from there.
//    class TfBinSel / fBinSel : `grep -rn "class TfBinSel" --include=*.h .`
//                         and `grep -rn "\bfBinSel\b" --include=*.cpp
//                         --include=*.h .` -- every hit a comment or a
//                         pre-existing `#if 0` GATE awaiting this file (see
//                         forms/fBinSel.h STUB COLLISION SCAN for the full
//                         line-by-line list), 20260819.
// =============================================================================
#include "forms/fBinSel.h"
// AI(W906-FW-BINSEL-W20) 20260826: 本波部分 handler 保留 golden 完整簽章。
#include "vclcompat/ShiftState.h"

#include "MachineType.h"        // eBinType enum, CC_* customer codes, TEST_MAX_BIN, e3*/e6* position enums
#include "cmydef.h"              // AccessLevel/bCancelErrorBin/bSpecificBin[]/iBinModelPrime/iBinBoxAtFix/
                                   // bAutoReTest_ART/bPurgeOutAllDevice/bGPIBChangeAlarm/bSetByDLL/
                                   // bTCPIPChangeAlarm/bMOFileControlBinChangeAlarm/bSetBINCOUNT/bOLPSetCategroy/
                                   // s3TrayName[]/s6TrayName[]/iTo3Unload[]/iTo6Unload[]/iTo3PosUnload[]
#include "cprod.h"                // Prod/TestIF/TestIF_File/BinSelect[]/TrayForm/tAOISetup
#include "LastSet.h"               // LastSet
#include "Config.h"                 // IniConfig
#include "CosFunction.h"             // CosFunction
#include "cSocket.h"                  // LotSummary
#include "cinitial.h"                  // SetWorkParameter()
#include "common.h"                    // ReadIniData/WriteIniData/CheckAndReadIniData/CheckSectionExist/
                                         // GetRecipePath/GetRecipeFileName/GetLastOpenFN/DataPath/
                                         // MyForceDirectories/AddSpace
#include "canary_support.h"              // ShowMyMessage/RecordProcess
#include "ProductionInfo/FileInfo.h"      // FileInfo::PathCombin
#include "forms/fShowBinSelect.h"           // fShowBinSelect

#include <cstdlib>   // atoi

// S19 (see cSecurity.cpp's own identical note): `cMyDB.h` and
// `canary_support.h` both declare `RecordProcess`/`MyDBIProcessNew` with
// CONFLICTING default arguments -- including both in one TU errors either
// order. Targeted forward declaration instead of `#include "cMyDB.h"`,
// signature copied verbatim from cMyDB.h:81 (real body lives in
// SECSGEM/uHGemEquipment.cpp today, per that header's own HOMECOMING
// NOTICE).
void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2="");

//---------------------------------------------------------------------------
// golden TU-local declarations (cBinSel.cpp :47-121) -- reproduced here
// verbatim, same file-scope, same names, so every unqualified reference in
// the ported methods below resolves exactly as golden's own does.
//---------------------------------------------------------------------------
static const int iBinCount = TEST_MAX_BIN;                                     // 使用的Bin數量 (golden :121)

enum eBinSettingItems                                                          // golden :53-86 (TU-local)
{   eBinNo          =0,
    eScanning       =1,
    eDoubleContact  =2,
    eConsFail       =3,
    ePersentEnable  =4,
    ePersentIgnore  =5,
    ePersentNumber  =6,
    eCountEnable    =7,
    eCountIgnore    =8,
    eCountNumber    =9,

    eSpecialBinByArm            =10,
    eSpecialBinCountByArm       =11,
    eSpecialBinBySocket         =12,
    eSpecialBinCountBySocket    =13,

    eLowYield       =14,
    eArmYield       =15,
    eSiteYield      =16,
    eAutoCleanByBin =17,
    eAutoCleanBySite=18,

    eSpecBinBySiteCompareEnable      =19,
    eSpecBinBySiteCompareIgnore      =20,
    eSpecBinBySiteComparePercent     =21,
    eSpecBinByArmPerSiteCompareEnable=22,
    eSpecBinByArmPerSiteCompareIgnore=23,
    eSpecBinByArmPerSiteComparePercent=24,

    eBinNotUse      ,
    eBinSetting     ,
    eBinSetTotal    =(eBinSetting+eTrayCount)
};

bool bCanLinkT6[eTrayCount];                                                    // golden :52 (TU-local)
e6TrayName iLinkTrayMin =eFix2;                                                 // golden :49
e6TrayName iLinkTrayMax =eFix3;                                                 // golden :50

const AnsiString sTrayName[eTrayCount]={"Auto1", "Auto2", "Auto3",              // golden cBinSel.cpp :88-92
    "Fix1", "Fix2", "Fix3", "Fix4", "Fix5", "Fix6", "BulkBox",
    "Magazine1", "Magazine2","Magazine3", "Magazine4", "Magazine5", "Magazine6", "Magazine7",
    "Magazine8", "Magazine9","Magazine10", "Magazine11", "Magazine12", "Magazine13", "Magazine14"
};

// AI(W906-FW-BinSel-WC) 20260819: golden :97-118 (TU-local) -- the mtTrayName/
// mtTrayItem cell-index and colour-index enums. Needed by this wave's ctor
// cell-fill addition and by mtTrayNameSetColor's unlock (see forms/fBinSel.h
// GATE REGISTER G9, CLOSED this wave).
const int eItemName   =0;                                                       // golden :97 -- mtTrayItem's ONLY column
enum eTrayNameFunc                                                              // golden :98-105 -- mtTrayName's columns
{   eItemLink   =0,
    eItemPass   =1,
    eItemError  =2,
    eItemCateR  =3,
    eItemART    =4,
    eItemTotal
};

enum eTrayColorMap                                                              // golden :107-118
{   eCLWhite        =0,
    eCLGreen        =1,
    eCLRed          =2,
    eCLYellow       =3,
    eCLPurple       =4,
    eCLBlue         =5,
    eCLGray         =6,
    eCLSilver       =7,
    eCLBtnFace      =8,
    eCLOlive        =9
};

//---------------------------------------------------------------------------
// AI(W906-FW-BinSel-WA) 20260819: SIOF homecoming -- real instance (see
// forms/fBinSel.h STATIC-INIT SAFETY -- this ctor's own body was read in
// full against golden before this global was defined; no cross-TU / file-I/O
// dependency found, so no `INIFileGeneral!=0` guard is used).
//---------------------------------------------------------------------------
TfBinSel *fBinSel = new TfBinSel();

//---------------------------------------------------------------------------
//  TfBinSel::TfBinSel -- DEVIATION bootstrap ctor. See forms/fBinSel.h CTOR
//  NOTE for why this is not a verbatim translation of golden's ctor body
//  (golden :970-1117).
//---------------------------------------------------------------------------
TfBinSel::TfBinSel()
{
    for (int i = 0; i < eBinTypeTotal; i++)                                    // golden :984-1076
    {
        sBinDoubleContact[i]=new TStringList();
        sBinConsFail[i]     =new TStringList();
        sBinEnableFail[i]   =new TStringList();
        sBinFailPercent[i]  =new TStringList();

        sBinFailIgnore[i]   =new TStringList();
        sBinCountEnable[i]  =new TStringList();
        sBinCountIgnore[i]  =new TStringList();
        sBinCountNumber[i]  =new TStringList();

        sSpecialBinByArm[i]         =new TStringList();
        sSpecialBinCountByArm[i]    =new TStringList();
        sSpecialBinBySocket[i]      =new TStringList();
        sSpecialBinCountBySocket[i] =new TStringList();

        sLowYield[i]   =new TStringList();
        sArmYield[i]   =new TStringList();
        sSiteYield[i]  =new TStringList();

        sBinTraySetT3PosName[i]=new TStringList();
        sBinTraySetT3Pos[i] =new TStringList();
        sBinType[i]         =new TStringList();
        sT3TrayType[i]      =new TStringList();
        sT6Retest[i]        =new TStringList();
        sBySiteClean[i]     =new TStringList();
        sByBinClean[i]      =new TStringList();
        sT3CateR[i]         =new TStringList();

        sSpecBinBySiteCompareEnable[i]       =new TStringList();
        sSpecBinBySiteCompareIgnore[i]       =new TStringList();
        sSpecBinBySiteComparePercent[i]      =new TStringList();
        sSpecBinByArmPerSiteCompareEnable[i] =new TStringList();
        sSpecBinByArmPerSiteCompareIgnore[i] =new TStringList();
        sSpecBinByArmPerSiteComparePercent[i]=new TStringList();
        sBinTrayLinked[i]   =new TStringList();
        sBinLinked[i]       =new TStringList();
        sMagazineSetup[i]   =new TStringList();

        sAOIBinTraySetting[i]   =new TStringList();
        sAOIBinTraySetting[i]   ->Add("0");

        for (int j = 0; j < iTestBinCount; j++)
        {
            sBinDoubleContact[i]->Add("0");
            sBinConsFail[i]     ->Add("0");
            sBinEnableFail[i]   ->Add("0");
            sBinFailPercent[i]  ->Add("0");

            sBinFailIgnore[i]   ->Add("0");
            sBinCountEnable[i]  ->Add("0");
            sBinCountIgnore[i]  ->Add("0");
            sBinCountNumber[i]  ->Add("0");

            sSpecialBinByArm[i]         ->Add("0");
            sSpecialBinCountByArm[i]    ->Add("0");
            sSpecialBinBySocket[i]      ->Add("0");
            sSpecialBinCountBySocket[i] ->Add("0");

            sLowYield[i]   ->Add("0");
            sArmYield[i]   ->Add("0");
            sSiteYield[i]  ->Add("0");

            sBinTraySetT3PosName[i] ->Add("NotUse");
            sBinTraySetT3Pos[i]     ->Add("0");
            sBinType[i]             ->Add("0");
            sBySiteClean[i]         ->Add("0");
            sByBinClean[i]          ->Add("0");

            sSpecBinBySiteCompareEnable[i]->Add("0");
            sSpecBinBySiteCompareIgnore[i]->Add("0");
            sSpecBinBySiteComparePercent[i]->Add("0");
            sSpecBinByArmPerSiteCompareEnable[i]->Add("0");
            sSpecBinByArmPerSiteCompareIgnore[i]->Add("0");
            sSpecBinByArmPerSiteComparePercent[i]->Add("0");

            sBinTrayLinked[i]   ->Add("0");
            sBinLinked[i]       ->Add("0");
            sMagazineSetup[i]   ->Add("0");
        }

        for (int j = e3Auto1; j < eTrayCount; j++)
        {
            sT3TrayType[i]->Add("0");
            sT3CateR[i]->Add("0");
        }

        for (int j = eAuto1; j <= iAutoRight; j++)
        {
            sT6Retest[i]->Add("0");
        }
    }

    // golden :1098-1102/:1104-1115 -- MyBinPanel population. `->mtBinSelect->
    // Width/XItem`, `InitmtBinSelectData()` and the KYEC `->Panel->Color`
    // tint remain widget-half, no reader anywhere in this tree yet (GATE
    // G10) -- dropped. `->mtTrayName/mtTrayItem`'s own cell-fill (golden
    // TMyBinPanel::TMyBinPanel ctor :386-387/:417-418/:421-511) is FW-
    // BinSel-WC (this wave): the "解鎖鑰匙" mtTrayNameSetColor needed (see
    // that method's own banner) -- landed here, inline, per-tag, since this
    // tree's TMyBinPanelData is a plain struct (no per-instance ctor of its
    // own to carry this the way golden's TMyBinPanel::TMyBinPanel(index,...)
    // does).
    for (int tag = 0; tag < eBinTypeTotal; tag++)
    {
        MyBinPanel[tag] = new TMyBinPanelData();

        MyBinPanel[tag]->mtTrayName->SetYItem(eBinSetTotal);                    // golden :386
        MyBinPanel[tag]->mtTrayName->SetXItem(eItemTotal);                      // golden :387
        MyBinPanel[tag]->mtTrayItem->SetYItem(eBinSetTotal);                    // golden :417
        MyBinPanel[tag]->mtTrayItem->SetXItem(eItemName);                       // golden :418

        //Col Name----
        MyBinPanel[tag]->mtTrayName->SetCellNumber(eItemLink,  eBinNo,       "Link"    );
        MyBinPanel[tag]->mtTrayName->SetCellNumber(eItemPass,  eBinNo,       "Failed"  );
        MyBinPanel[tag]->mtTrayName->SetCellNumber(eItemError, eBinNo,       "Error"   );
        MyBinPanel[tag]->mtTrayName->SetCellNumber(eItemART,   eBinNo,       "Retest"  );            //ChungHung 20140317 add Auto Retest
        MyBinPanel[tag]->mtTrayName->SetCellNumber(eItemCateR, eBinNo,       "CateR"   );            //Steven 20161221 (wei) : Cate R for ART

        //Row Name----
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eBinNo,           "Items"         );
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eScanning,        "Scanning"      );
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eDoubleContact,   "Double Contact");
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eConsFail,        "Cons. Fail"    );

        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  ePersentEnable,   "Yield % Bin");     //Steven 20140925 : SPIL鄭世明說Pass也要設定 Fail --> Yield
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  ePersentIgnore,   "Yield Ignore Cnt");//JerryYang 20160802 修正文字避免誤解,Yield % Ignore改成Yield Ignore Cnt
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  ePersentNumber,   "Yield % Number");
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eCountEnable,     "Count Bin" );      //Steven 20140925 : SPIL鄭世明說Pass也要設定 Fail --> Count
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eCountIgnore,     "Count Ignored");
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eCountNumber,     "Count Number");

        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecialBinByArm,           "Spc. Bin By Arm");
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecialBinCountByArm,      "Spc. Cnt By Arm") ;
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecialBinBySocket,        "Spc. Bin By Socket");
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecialBinCountBySocket,   "Spc. Cnt By Socket");

        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eLowYield,    "Low Yield") ;          //Steven 20140828 : By Bin Yield Monitor
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eArmYield,    "By Arm Yield");        //Steven 20140828 : By Bin Arm Yield Monitor

        if (CUSTOMER_CODE==CC_Greatek)                                               //Sam 20201221 : 修正 Control Bin 顯示
        {
            MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,    eSiteYield,         "Control Bin Yield%");
            MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,    eAutoCleanByBin,    "Control Bin Count");
            MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,    eAutoCleanBySite,   "Control Bin S/S%");
        }
        else
        {
            MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,    eSiteYield,         "By Site Yield");   //Steven 20140828 : By Bin Site Yield Monitor
            MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,    eAutoCleanByBin,    "By Bin Cleaning"); //Steven 20160308 : By Bin count do auto clean
            MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,    eAutoCleanBySite,   "By Site Cleaning");//Steven 20160308 : By Bin pre site count do auto clean
        }

        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecBinBySiteCompareEnable,       "By Bin Site Gap");
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecBinBySiteCompareIgnore,       "Count Ignore") ;
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecBinBySiteComparePercent,      "Site Gap %");
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecBinByArmPerSiteCompareEnable, "By Arm By Bin Site Gap");
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecBinByArmPerSiteCompareIgnore, "Count Ignore") ;
        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eSpecBinByArmPerSiteComparePercent,"By Arm Site Gap%");

        MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  eBinNotUse,    "Not in use");

        for (int j = eBinNo; j < eBinSetting; j++)
        {
            for (int k = eItemLink; k < eItemTotal; k++)
            {
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(k,  j, eCLGray);
            }
            MyBinPanel[tag]->mtTrayItem->SetCellColorIndex(eItemName, j, eCLGray);
        }

        for (int j = eBinSetting; j < eBinSetTotal; j++)
        {
            int iT6=j-eBinSetting;

            // AI(W906-FW-BinSel-WC) 20260819: SIOF guard -- `s6TrayName[]` is
            // a cross-TU global with a non-trivial ctor (AnsiString array,
            // defined in cmydef.cpp:44, cmydef.h:138) and this ctor runs at
            // STATIC INIT (`fBinSel = new TfBinSel();` below, see forms/
            // fBinSel.h STATIC-INIT SAFETY). Inter-TU dynamic-init order is
            // unspecified, so reading it unconditionally here risks the SAME
            // class of bug cObserver.cpp's GetObserAuth() guard prevents
            // (AI(W906-FW-ObsSwap) 20260818) -- `INIFileGeneral!=0` is this
            // tree's "we are past static-init, real runtime" proxy (only
            // ever set non-null from inside main()/WinMain's own body).
            // Every OTHER statement in this loop is a literal string or a
            // plain-old extern global (same safe category as this ctor's
            // existing STATIC-INIT SAFETY note) -- only THIS one line needs
            // the guard.
            if (INIFileGeneral != 0)
                MyBinPanel[tag]->mtTrayItem->SetCellNumber(eItemName,  j, s6TrayName[iT6].c_str());
            MyBinPanel[tag]->mtTrayItem->SetCellColorIndex(eItemName,  j, eCLGray);
            MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemError, j, "Error");
            MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemError, j, eCLWhite);

            if (USE_AUTO_RETEST==eartInstall || IniConfig.bA38_SLT_Summary)          //JerryYang 20220923 : add for SLT lot summary
            {
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemART, j, "Retest");               //ChungHung 20140317 add Auto Retest
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemART, j, eCLWhite);               //ChungHung 20140317 add Auto Retest

                if (CosFunction.bUseSCKART)                                          //Steven 20161221 (wei) : Cate R for ART
                {
                    MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemCateR, j, "CateR");
                    MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemCateR, j, eCLWhite);
                }
                else
                {
                    MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemCateR, j, "");
                    MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemCateR, j, eCLGray);          //Steven 20161221 (wei) : Cate R for ART
                }
            }
            else
            {
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemART, j, "");                     //ChungHung 20140317 add Auto Retest
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemART, j, eCLGray);                //ChungHung 20140317 add Auto Retest
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemCateR, j, "");
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemCateR, j, eCLGray);              //Steven 20161221 (wei) : Cate R for ART
            }
        }

        // GATE (G10): InitmtBinSelectData() (golden :513) -- populates
        // MyBinPanel[tag]->mtBinSelect, still absent this wave.
    }

    if (CosFunction.bDisableRTBinSet)                                          // golden :1078-1079
        cbTestMode->Items->Delete(1);

    if (CosFunction.bOffLineBin || USE_AUTO_RETEST==eartInstall)                // golden :1081-1084
        cbTestMode->Items->Add("Off-Line");

    if (CosFunction.bUseSCKART==false && USE_AUTO_RETEST==eartInstall)         // golden :1086-1090
    {
        cbTestMode->Items->Add("ART Re-Test");
        cbTestMode->Items->Add("ART Normal");
    }

    if (CosFunction.bUseMRTMode==true)                                         // golden :1092-1096
    {
        cbTestMode->Items->Add("MRT Re-Test");
        cbTestMode->Items->Add("MRT Normal");
    }

    // AI(W906-FW-BinSel-WA) 20260819: hydration, not a golden line -- see
    // forms/fBinSel.h HYDRATION. `iTestBinCount` isn't visible to the
    // header's own default member initializer (cmydef.h/cprod.h not
    // included there), so the placeholder `(32,2)` construction is
    // corrected here, where it is in scope.
    sgSpecificBin->ColCount = iTestBinCount;
}

//---------------------------------------------------------------------------
//  FormDestroy -- golden :1995-2088
//---------------------------------------------------------------------------
void TfBinSel::FormDestroy(TObject * /*Sender*/)
{
    try
    {
        for (int i = eBinTypeTotal-1; i >= 0; i--)
            delete MyBinPanel[i];

        for (int i = eBinTypeTotal-1; i >= 0; i--)
        {
            if (sBinDoubleContact[i]!=NULL) delete sBinDoubleContact[i];
            if (sBinConsFail[i]!=NULL) delete sBinConsFail[i];
            if (sBinEnableFail[i]!=NULL) delete sBinEnableFail[i];
            if (sBinFailPercent[i]!=NULL) delete sBinFailPercent[i];

            if (sBinFailIgnore[i]!=NULL) delete sBinFailIgnore[i];
            if (sBinCountEnable[i]!=NULL) delete sBinCountEnable[i];
            if (sBinCountIgnore[i]!=NULL) delete sBinCountIgnore[i];
            if (sBinCountNumber[i]!=NULL) delete sBinCountNumber[i];

            if (sSpecialBinByArm[i]!=NULL) delete sSpecialBinByArm[i];
            if (sSpecialBinCountByArm[i]!=NULL) delete sSpecialBinCountByArm[i];
            if (sSpecialBinBySocket[i]!=NULL) delete sSpecialBinBySocket[i];
            if (sSpecialBinCountBySocket[i]!=NULL) delete sSpecialBinCountBySocket[i];

            if (sLowYield[i]!=NULL) delete sLowYield[i];
            if (sArmYield[i]!=NULL) delete sArmYield[i];
            if (sSiteYield[i]!=NULL) delete sSiteYield[i];

            if (sBinTraySetT3PosName[i]!=NULL) delete sBinTraySetT3PosName[i];
            if (sBinTraySetT3Pos[i]!=NULL) delete sBinTraySetT3Pos[i];
            if (sBinType[i]!=NULL) delete sBinType[i];
            if (sT3TrayType[i]!=NULL) delete sT3TrayType[i];
            if (sT6Retest[i]!=NULL) delete sT6Retest[i];
            if (sBySiteClean[i]!=NULL) delete sBySiteClean[i];
            if (sByBinClean[i]!=NULL) delete sByBinClean[i];
            if (sT3CateR[i]!=NULL) delete sT3CateR[i];

            if (sSpecBinBySiteCompareEnable[i]!=NULL) delete sSpecBinBySiteCompareEnable[i];
            if (sSpecBinBySiteCompareIgnore[i]!=NULL) delete sSpecBinBySiteCompareIgnore[i];
            if (sSpecBinBySiteComparePercent[i]!=NULL) delete sSpecBinBySiteComparePercent[i];
            if (sSpecBinByArmPerSiteCompareEnable[i]!=NULL) delete sSpecBinByArmPerSiteCompareEnable[i];
            if (sSpecBinByArmPerSiteCompareIgnore[i]!=NULL) delete sSpecBinByArmPerSiteCompareIgnore[i];
            if (sSpecBinByArmPerSiteComparePercent[i]!=NULL) delete sSpecBinByArmPerSiteComparePercent[i];

            if (sBinTrayLinked[i]!=NULL) delete sBinTrayLinked[i];
            if (sBinLinked[i]!=NULL) delete sBinLinked[i];
            if (sMagazineSetup[i]!=NULL) delete sMagazineSetup[i];

            if (sAOIBinTraySetting[i]!=NULL) delete sAOIBinTraySetting[i];
        }
    }
    catch (...)
    {
        MyDBIProcess("Exception", "TfBinSel::FormDestroy", "");
    }
    LogSoftwareOffTime("TfBinSel, FormDestroy");
}

//---------------------------------------------------------------------------
//  FormClose -- golden :2099-2107
//---------------------------------------------------------------------------
void TfBinSel::FormClose(TObject * /*Sender*/)
{
    bShow=false;
    ReadFile(false, false, "");
    fShowBinSelect->InitShowBinDigital();
}

//---------------------------------------------------------------------------
//  ReadParam -- golden :2090-2097 (pure read)
//---------------------------------------------------------------------------
void TfBinSel::ReadParam()
{
    AnsiString szDir=GetRecipeFileName("Tester.Data");
    bUseContinueFail=ReadIniData(szDir, "Alarm", "SocketEnable", 0);
    if (bUseContinueFail!=true)
        bUseContinueFail=ReadIniData(szDir, "Alarm", "HeadEnable", 0);
    bUseFailRate=ReadIniData(szDir, "Alarm", "RateEnable", 0);
}

//---------------------------------------------------------------------------
//  ChangeActivePageIndex -- golden :2194-2212. See forms/fBinSel.h DESIGN
//  NOTE for the ActivePageIndex integer convention (0..6, matching golden's
//  own SaveOther literal comparisons).
//---------------------------------------------------------------------------
void TfBinSel::ChangeActivePageIndex()
{
    if (iTestRunMode==eBinFT)
        PageControl1->ActivePageIndex=0;        // tsNormal
    else if (iTestRunMode==eBinRT)
        PageControl1->ActivePageIndex=1;        // tsRetest
    else if (iTestRunMode==eBinOffLine)
        PageControl1->ActivePageIndex=2;        // tsOffline
    else if (iTestRunMode==eBinFT_ART)
        PageControl1->ActivePageIndex=3;        // tsArtFT
    else if (iTestRunMode==eBinRT_ART)
        PageControl1->ActivePageIndex=4;        // tsArtRT
    else if (iTestRunMode==eBinFT_MRT)
        PageControl1->ActivePageIndex=5;        // tsMrtFT
    else if (iTestRunMode==eBinRT_MRT)
        PageControl1->ActivePageIndex=6;        // tsMrtRT
    else
        ShowMyMessage("ChangeActivePageIndex error", "ChangeActivePageIndex 錯誤");
}

//---------------------------------------------------------------------------
//  CheckFix2Tray -- golden :4785-4805 (pure, MyBinPanelData)
//---------------------------------------------------------------------------
bool TfBinSel::CheckFix2Tray()
{
    if (tAOISetup.bEnabledAOI)
    {
        for (int tag = 0; tag < 3; tag++)
        {
            for (int i = 0; i < iTestBinCount; i++)
            {
                for (int j = eBinNotUse; j < eBinSetTotal; j++)
                {
                    if (MyBinPanel[tag]->BackT6PosTray[i][j])
                    {
                        if ((j-eBinNotUse)==ePosFix2)
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------
//  CheckOSBin -- golden :6254-6266 (pure)
//---------------------------------------------------------------------------
bool TfBinSel::CheckOSBin()
{
    bool IsOK=true;
    if (CosFunction.bAutoSiteMappingSetOpenBIN==true)
    {
        if (Prod.iOpenBin>=0)
        {
            if (BinSelect[iTestRunMode].iCatDataT3Pos[Prod.iOpenBin]==0)
                IsOK=false;
        }
    }
    return IsOK;
}

//---------------------------------------------------------------------------
//  SetOSBin -- golden :6268-6284
//---------------------------------------------------------------------------
bool TfBinSel::SetOSBin(int iBin)
{
    bool IsOK=true;
    if (iBin>=-1 && iBin<16)
    {
        cbbAutoSiteMap->ItemIndex=iBin+1;
        ReadWriteMRTMode(1);
        ReadWriteMRTMode(0);
        if (CheckOSBin()==false)
            IsOK=false;
    }
    else
    {
        IsOK=false;
    }
    return IsOK;
}

//---------------------------------------------------------------------------
//  ReadWriteMRTMode -- golden :6133-6185. GATE (G4): fMain->SetOpenBin()
//  absent -- see forms/fBinSel.h GATE REGISTER. Write half (iMode==1) gated
//  per WRITE-PATH GATE TABLE.
//---------------------------------------------------------------------------
void TfBinSel::ReadWriteMRTMode(int iMode)
{
    AnsiString S=GetLastOpenFN();
    AnsiString szDir="";
    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath.c_str(), S.c_str());

    if (CosFunction.bUseMRTMode==true)
    {
        if (iMode==0)
        {
            TestIF_File.bEnableMRTMode=ReadIniData(szDir, "Configuration", "Enable MRT Mode", false);
            cbUseMRTMode->Checked     =TestIF_File.bEnableMRTMode;
        }
        else
        {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6149
            // GATE (write-policy): WriteIniData(szDir,"Configuration","Enable MRT Mode",...)
            WriteIniData(szDir, "Configuration", "Enable MRT Mode",(cbUseMRTMode->Checked)?1:0);
//#endif   //AI(W906-S09-B6) 20260929: see :610
            TestIF_File.bEnableMRTMode=cbUseMRTMode->Checked;
        }
    }
    else
    {
        TestIF_File.bEnableMRTMode=false;

        if (CosFunction.bAutoSiteMappingSetOpenBIN==true)
        {
            if (iMode==0)
            {
                cbbAutoSiteMap->ItemIndex=ReadIniData(szDir, "Configuration", "ASM Open Bin", 0);
                TestIF_File.iOpenBin=cbbAutoSiteMap->ItemIndex-1;
                Prod.iOpenBin=TestIF_File.iOpenBin;
                // GATE (G4): fMain->SetOpenBin(); -- forms/fMain.h has no such
                // member (`grep -n "SetOpenBin" forms/fMain.h` -- 0 hits,
                // 20260819). Prod.iOpenBin above (the real, independently
                // observable state) is set regardless.
            }
            else
            {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6168
                // GATE (write-policy)
                WriteIniData(szDir, "Configuration", "ASM Open Bin", cbbAutoSiteMap->ItemIndex);
//#endif   //AI(W906-S09-B6) 20260929: see :635
            }

            if (CosFunction.bAutoSiteMappingSetPassBIN==true)
            {
                if (iMode==0)
                {
                    cbbASMPassBin->ItemIndex=ReadIniData(szDir, "Configuration", "ASM Pass Bin", 2);
                    TestIF_File.iASMPassBin=cbbASMPassBin->ItemIndex-1;
                }
                else
                {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6180
                    // GATE (write-policy)
                    WriteIniData(szDir, "Configuration", "ASM Pass Bin", cbbASMPassBin->ItemIndex);
//#endif   //AI(W906-S09-B6) 20260929: see :650
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
//  ReadWriteSpecialFunction -- golden :6187-6252. Write half (bReadWrite==
//  true) gated per WRITE-PATH GATE TABLE.
//---------------------------------------------------------------------------
void TfBinSel::ReadWriteSpecialFunction(bool bReadWrite)
{
    if (IniConfig.bSPILFunction==true)
    {
        AnsiString S=GetLastOpenFN();
        AnsiString szDir="";
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath.c_str(), S.c_str());

        if (CosFunction.bOutShtLoseICSetErrUntilOneCycle)
        {
            if (bReadWrite==false)
            {
                TestIF_File.bOutShtLoseICSetErrUntilOneCycle=ReadIniData(szDir, "Configuration", "bOutShtLoseICSetErrUntilOneCycle", false);
                cbOutShtLoseICSetErrUntilOneCycle->Checked  =TestIF_File.bOutShtLoseICSetErrUntilOneCycle;
            }
            else
            {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6205
                // GATE (write-policy)
                WriteIniData(szDir, "Configuration", "bOutShtLoseICSetErrUntilOneCycle", cbOutShtLoseICSetErrUntilOneCycle->Checked);
//#endif   //AI(W906-S09-B6) 20260929: see :681
                TestIF_File.bOutShtLoseICSetErrUntilOneCycle=cbOutShtLoseICSetErrUntilOneCycle->Checked;
            }
        }
        else
        {
            TestIF_File.bOutShtLoseICSetErrUntilOneCycle=false;
        }

        if (CosFunction.bIndexDropICSetErrUntilOneCycle)
        {
            if (bReadWrite==false)
            {
                TestIF_File.bIndexDropICSetErrUntilOneCycle=ReadIniData(szDir, "Configuration", "bIndexDropICSetErrUntilOneCycle", false);
                cbIndexDropErrSetErrUntilOneCycle->Checked =TestIF_File.bIndexDropICSetErrUntilOneCycle;
            }
            else
            {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6223
                // GATE (write-policy)
                WriteIniData(szDir, "Configuration", "bIndexDropICSetErrUntilOneCycle", cbIndexDropErrSetErrUntilOneCycle->Checked);
//#endif   //AI(W906-S09-B6) 20260929: see :702
                TestIF_File.bIndexDropICSetErrUntilOneCycle=cbIndexDropErrSetErrUntilOneCycle->Checked;
            }
        }
        else
        {
            TestIF_File.bIndexDropICSetErrUntilOneCycle=false;
        }
    }
    else
    {
        TestIF_File.bOutShtLoseICSetErrUntilOneCycle  =CosFunction.bOutShtLoseICSetErrUntilOneCycle  ? true : false;
        TestIF_File.bIndexDropICSetErrUntilOneCycle   =CosFunction.bIndexDropICSetErrUntilOneCycle   ? true : false;
    }
}

//---------------------------------------------------------------------------
//  ReadPrimeDara -- golden :6044-6057 (pure read + Wave-B stub call)
//---------------------------------------------------------------------------
void TfBinSel::ReadPrimeDara()
{
    AnsiString S=GetLastOpenFN();
    AnsiString szDir;

    szDir.sprintf("%s%s\\Binasgn.Data", DataPath.c_str(), S.c_str());
    iBinModelPrime=ReadIniData(szDir, "BinModel", "bPrime", 0);
    SetPrimeButton();

    if (CosFunction.bShow0Xbin)
    {
        chkShow0Xbin->Checked=ReadIniData(szDir, "Other", "bShow0XBin", false);
    }
}

//---------------------------------------------------------------------------
//  WritePrimeDara -- golden :6059-6068. Write gated per WRITE-PATH GATE
//  TABLE; `iBinModelPrime=Mode;` (real, in-memory) stays ACTIVE.
//  AI(W906-FRW-S99) 20260926: 寫檔那一行的 GATE (write-policy) 退役（RULINGS_20260926 S99；行號 V912 cBinSel.cpp:6107-6116）。
//    當初的理由是 20260819 本波的一律政策「shared config 的寫入一律 #if 0」（forms/fBinSel.h WRITE-PATH GATE TABLE）——
//    已過期：S12／S52 起讀寫檔由 St01 照 golden 移植，Binasgn.Data 的 C 路（FileRW/BinSelect.cpp）存檔鈕已經寫同一鍵
//    （golden SaveOther :2783 FormSysTools->WriteIniData("BinModel","bPrime",iBinModelPrime)，FileRW/BinSelect.gen.inc）。
//    留著閘反而是陷阱：記憶體 iBinModelPrime 改了、檔案沒改，下一次 ReadFile→ReadPrimeDara（本檔上面）會把它讀回舊值。
//    本方法目前沒有呼叫者（golden 只有 spbNormalClick／spbPrimeClick 兩顆鈕呼叫，網頁那條路走 FileRW/BinSelect.gen.inc 的
//    BS_WritePrimeDara，同一份 golden 本體）；解閘不改變今天任何執行路徑。
//---------------------------------------------------------------------------
void TfBinSel::WritePrimeDara(int Mode)
{
    AnsiString S=GetLastOpenFN();
    AnsiString szDir;

    szDir.sprintf("%s%s\\Binasgn.Data", DataPath.c_str(), S.c_str());
    WriteIniData(szDir, "BinModel", "bPrime", Mode);                           // golden :6113（AI(W906-FRW-S99) 20260926 解閘，見上）
    iBinModelPrime=Mode;
    SetPrimeButton();
}

//---------------------------------------------------------------------------
//  ARTBinCheck -- golden :6381-6401 (pure, MyBinPanelData)
//---------------------------------------------------------------------------
void TfBinSel::ARTBinCheck(int tag)
{
    if (USE_AUTO_RETEST==eartInstall && IniConfig.bA10_AutoReTest &&
        (LastSet.iRunStartMode==rsmInitial_ART || LastSet.iRunStartMode==rsmContinuStart_ART || LastSet.iRunStartMode==rsmContinuRetest_ART))
    {
        for (int i = 0; i < 3; i++)
        {
            if (MyBinPanel[tag]->iT6IsFail[i]==0)
            {
                MyBinPanel[tag]->bT6ART[i]=0;
            }
        }

        if ((MyBinPanel[tag]->iT6IsFail[0]==0 && MyBinPanel[tag]->iT6IsFail[1]==0 && MyBinPanel[tag]->iT6IsFail[2]==0) ||
            (MyBinPanel[tag]->bT6ART[0]==0 && MyBinPanel[tag]->bT6ART[1]==0 && MyBinPanel[tag]->bT6ART[2]==0))
        {
            MyBinPanel[tag]->iT6IsFail[1]=1;
            MyBinPanel[tag]->bT6ART[1]=1;
        }
    }
}

//---------------------------------------------------------------------------
//  TransferBinTrayStrToName -- golden :6404-6423 (pure TStringList)
//---------------------------------------------------------------------------
void TfBinSel::TransferBinTrayStrToName(int iTag)
{
    int iT3;
    TStringList *sT3List;
    sT3List = new TStringList();
    sT3List->CommaText=sBinTraySetT3Pos[iTag]->CommaText;
    for (int i = 0; i < sT3List->Count; i++)
    {
        iT3=atoi(AnsiString(sT3List->Strings[i]).c_str());
        if (i<sBinTraySetT3PosName[iTag]->Count)
        {
            if (i>=iTestBinCount || iT3==0)
                sBinTraySetT3PosName[iTag]->Strings[i]="NotUse";
            else
                sBinTraySetT3PosName[iTag]->Strings[i]=s3TrayName[iT3-1];
        }
    }
    sT3List->Clear();
    delete sT3List;
}

//---------------------------------------------------------------------------
//  SetConFail -- golden :2274-2335. One WriteIniData gated per WRITE-PATH
//  GATE TABLE; MyForceDirectories also gated (directory-mutation prerequisite
//  of the gated write).
//---------------------------------------------------------------------------
void TfBinSel::SetConFail(int iBinasgnMode)
{
    int tag=0, iTray;
    AnsiString GroupStr;
    AnsiString szDir=GetRecipePath();
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2279
    // GATE (write-policy): directory-mutation prerequisite of the gated write below
    MyForceDirectories(szDir);
//#endif   //AI(W906-S09-B6) 20260929: see :820

    if (iBinasgnMode==eBinFT)
    {
        szDir=GetRecipeFileName("Binasgn.Data");
        tag=eBinFT;
    }
    else if (iBinasgnMode==eBinRT)
    {
        szDir=GetRecipeFileName("BinasgnOff.Data");
        tag=eBinRT;
    }
    else if (iBinasgnMode==eBinOffLine)
    {
        szDir=GetRecipeFileName("BinasgnOff-Line.Data");
        tag=eBinOffLine;
    }
    else if (iBinasgnMode==eBinFT_ART)
    {
        szDir=GetRecipeFileName("Binasgn_ART.Data");
        tag=eBinFT_ART;
    }
    else if (iBinasgnMode==eBinRT_ART)
    {
        szDir=GetRecipeFileName("BinasgnOff_ART.Data");
        tag=eBinRT_ART;
    }
    else if (iBinasgnMode==eBinFT_MRT)
    {
        szDir=GetRecipeFileName("Binasgn_MRT.Data");
        tag=eBinFT_MRT;
    }
    else if (iBinasgnMode==eBinRT_MRT)
    {
        szDir=GetRecipeFileName("Binasgn_MRT_RT.Data");
        tag=eBinRT_MRT;
    }
    (void)tag;   // only feeds the gated WriteIniData below in this wave

    bool bT3FailTray[e3TrayCount];
    for (int i = 0; i < e3TrayCount; i++)
    {
        bT3FailTray[i]=CheckAndReadIniData(szDir, s3TrayName[i], "Pass/Fail", 0);
    }

    for (int i = 0; i < iTestBinCount; i++)
    {
        GroupStr.sprintf("Category%d", i);
        iTray=CheckAndReadIniData(szDir, GroupStr, "Bin", (tag==eBinOffLine)?e3PosFix2:e3PosNoUse);
        if (iTray!=e3PosNoUse && bT3FailTray[iTray-1]==true)
        {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2331
            // GATE (write-policy)
            WriteIniData(szDir, GroupStr, "Cons.Fail", 1);
//#endif   //AI(W906-S09-B6) 20260929: see :874
        }
    }
}

//---------------------------------------------------------------------------
//  Save -- golden :6296-6361. Every WriteIniData gated per WRITE-PATH GATE
//  TABLE; GATE (G5) fShowBinSelect->ShowBinSel() (Wave B in that file).
//---------------------------------------------------------------------------
void TfBinSel::Save(int iECIDCode, int iFileCode)
{
    int tag=iFileCode;

    ReadPrimeDara();
    AnsiString SavePath[]={"BinasgnOff.Data",   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6355 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
                           "Binasgn.Data",   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6356 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
                           "BinasgnOff_ART.Data",   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6357 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
                           "Binasgn_ART.Data",   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6358 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
                           "Binasgn_MRT_RT.Data",   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6359 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
                           "Binasgn_MRT.Data"};   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6360 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)

    if (IniConfig.bA02BinModelPrime)
    {
        if (IniConfig.bFTBin2RTBin==true && (iBinModelPrime==0 || CosFunction.bDisableRTBinSet))
            SavePath[0]="Binasgn.Data";   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6366 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
        else
            SavePath[0]="BinasgnOff.Data";   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6370 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
    }
    else
    {
        if (IniConfig.bFTBin2RTBin==true || CosFunction.bDisableRTBinSet)
            SavePath[0]="Binasgn.Data";   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6377 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
        else
            SavePath[0]="BinasgnOff.Data";   //AI(W906-W191) 20261009 (St02-E): golden 913 cBinSel.cpp:6381 -- no leading backslash: GetRecipePath() already ends with one (common.cpp:2041)
    }

    AnsiString szDir=GetRecipePath()+SavePath[tag];
    (void)szDir;   // only feeds the gated writes below in this wave   [STALE 20261009 W-191: the gates below were retired in S09-B6 (20260929) -- these writes are live]

    switch (iECIDCode)
    {
        case 3616:
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6344-6345
            // GATE (write-policy)
            WriteIniData(szDir, "I/F Error", "Bin", BinSelect[eBinFT].IfErrorT3);
            WriteIniData(szDir, "I/F Error", "Tray", s3TrayName[BinSelect[eBinFT].IfErrorT3]);
//#endif   //AI(W906-S09-B6) 20260929: see :919
            ReadFile(false, false, "");
            // GATE (G5): fShowBinSelect->ShowBinSel(); -- forms/fShowBinSelect.h's
            // own WAVE B QUEUE (golden :388-757), not yet landed there.
            break;
        case 3617:
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6350
            // GATE (write-policy)
            WriteIniData(szDir, "Bin Func FT", "3617 BinTraySetting", sBinTraySetT3Pos[eBinFT]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :929
            TransferBinTrayStrToName(eBinFT);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6352
            // GATE (write-policy)
            WriteIniData(szDir, "Bin Func FT", "3617 BinTraySetName", sBinTraySetT3PosName[eBinFT]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :934
            break;
        case 3677:
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6355-6357
            // GATE (write-policy)
            for (int i = 0; i < eTrayCount; i++)
                WriteIniData(szDir, s3TrayName[i], "Pass/Fail", sT3TrayType[eBinFT]->Strings[i]);
//#endif   //AI(W906-S09-B6) 20260929: see :940
            break;
    }
}

//---------------------------------------------------------------------------
//  spbSaveClick -- golden :2214-2272. GATE (G6): fMain->SetStartModeData()/
//  BackupSetupFile() and the fLotInfo yield-monitor branch (WRITE-PATH GATE
//  TABLE + unverified fLotInfo surface). #ifdef ASE_KaohSiung block kept as
//  real conditional compilation (macro undefined in this CMake build, same
//  as golden's own BCB6-build-flag intent).
//---------------------------------------------------------------------------
void TfBinSel::spbSaveClick(TObject * /*Sender*/)
{
    if (IniConfig.bA02DisableSaveParsWhenSwitchToOp==true && AccessLevel==0)
    {
        ShowMyMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
        // GATE (cosmetic): golden calls Close() here (VCL TForm::Close, no
        // non-VCL TfBinSel equivalent exists) -- the message above is the
        // observable effect this wave preserves.
    }

    if (CheckFix2Tray())
    {
        ShowMyMessage("己開啟AOI功能，不能使用Fix2 Tray");
        return;
    }

    Label1->Caption="Saving Data..";

    AnsiString szDir=GetRecipePath();
    SaveOther(szDir);

    Label1->Caption="Refresh data..";
    ReadWriteMRTMode(1);
    ReadWriteSpecialFunction(true);
    ReadFile(false, false, "");

    if (CheckOSBin()==false)
    {
        ShowMyMessage("SetOSBin Category Error", IntToStr(Prod.iOpenBin));
    }

    if (bSaveBin==false)
    {
        bSaveBin=true;
        RecordProcess("Save Bin");
    }

    if (IniConfig.bEnable_SECS_GEM==true)
    {
        bHasSaveSet=true;
    }
    fShowBinSelect->InitShowBinDigital();
    palSpecificBin->Visible=false;
    Label1->Caption="Save..OK";
    spbSave->Down=false;

    // GATE (G6): if(CosFunction.bUseMRTMode==true) fMain->SetStartModeData();
    //            fMain->BackupSetupFile();
    //            if(IniConfig.bSIGURDFunction && fLotInfo->pgLotinfo->
    //               ActivePage==fLotInfo->tsYieldMonitior)
    //                fLotInfo->RefreshYieldMonitor();
}

//---------------------------------------------------------------------------
//  SaveOther -- golden :2339-2753 ("kevin 20150105 另存一個 jobfile"). Every
//  FormSysTools->WriteIniData / OpenFormData+write-session pair below is
//  gated per WRITE-PATH GATE TABLE; the tag-selection chain, the two
//  pre-save VALIDATION checks (bP28Auto1OnlyBin1 / bUsePassBinOnlyCanSetOneBin
//  -- reads + ShowMyMessage + return, no write), ARTBinCheck, and every
//  in-memory sXxx[]/MyBinPanel[]-> update stay ACTIVE.
//---------------------------------------------------------------------------
void TfBinSel::SaveOther(AnsiString szDir)
{
    int tag=0;
    AnsiString FileName="";
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2279
    // GATE (write-policy): directory-mutation prerequisite of the gated writes below
    MyForceDirectories(szDir);
//#endif   //AI(W906-S09-B6) 20260929: see :1021

    if (PageControl1->ActivePageIndex==1)
    {
        tag=eBinRT;
        if (IniConfig.bA02BinModelPrime)
        {
            if (IniConfig.bFTBin2RTBin==true && (iBinModelPrime==0 || CosFunction.bDisableRTBinSet))
                FileName=FileInfo().PathCombin(szDir, "Binasgn.Data");
            else
                FileName=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
        }
        else
        {
            if (IniConfig.bFTBin2RTBin==true || CosFunction.bDisableRTBinSet)
                FileName=FileInfo().PathCombin(szDir, "Binasgn.Data");
            else
                FileName=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
        }
    }
    else if (PageControl1->ActivePageIndex==2)
    {
        FileName=FileInfo().PathCombin(szDir, "BinasgnOff-Line.Data");
        tag=eBinOffLine;
    }
    else if (PageControl1->ActivePageIndex==3)
    {
        FileName=FileInfo().PathCombin(szDir, "Binasgn_ART.Data");
        tag=eBinFT_ART;
    }
    else if (PageControl1->ActivePageIndex==4)
    {
        FileName=FileInfo().PathCombin(szDir, "BinasgnOff_ART.Data");
        tag=eBinRT_ART;
    }
    else if (PageControl1->ActivePageIndex==5)
    {
        FileName=FileInfo().PathCombin(szDir, "Binasgn_MRT.Data");
        tag=eBinFT_MRT;
    }
    else if (PageControl1->ActivePageIndex==6)
    {
        FileName=FileInfo().PathCombin(szDir, "Binasgn_MRT_RT.Data");
        tag=eBinRT_MRT;
    }
    else
    {
        FileName=FileInfo().PathCombin(szDir, "Binasgn.Data");
        tag=eBinFT;
    }
    // AI(W906-FW-BinSel-WB) 20260819: `FileName` is genuinely consumed below
    // by `SaveFunctionData(tag, FileName)` now that WAVE B lands its real
    // body -- the WAVE A `(void)FileName;` placeholder cast is removed (no
    // longer accurate: it is not "only" feeding gated sessions any more).

    if (IniConfig.bP28Auto1OnlyBin1==true)
    {
        int iErrCode=0;
        TStringList *sT3List;
        sT3List = new TStringList();
        sT3List->CommaText=sBinTraySetT3Pos[tag]->CommaText;
        if (CUSTOMER_CODE==CC_UNISEM_M && LastSet.iTester==OFF_LINE)
        {
        }
        else
        {
            for (int i = 0; i < iTestBinCount; i++)
            {
                if (i==1)
                {
                    if (sT3List->Strings[i]!=AnsiString("1"))
                    {
                        iErrCode=1;
                        break;
                    }
                }
                else if (sT3List->Strings[i]==AnsiString("1"))
                {
                    iErrCode=2;
                    break;
                }
            }
        }
        sT3List->Clear();
        delete sT3List;
        if (iErrCode==1)
        {
            ShowMyMessage("Bin1未被設定在Auto1 !! 請確認 !!\n Bin1 is not set to Auto1 !! Please Check !!");
            return;
        }
        else if (iErrCode==2)
        {
            ShowMyMessage("只有Bin 1被允許設為 Auto1 !! 請確認 !!\n Only Bin 1 is allowed to set to Auto 1!!! Please check !!");
            return;
        }
    }

    if (CosFunction.bUsePassBinOnlyCanSetOneBin==true)
    {
        int iBinCt=0;
        AnsiString str1, str2;
        TStringList *sT3List;
        sT3List = new TStringList();
        sT3List->CommaText=sBinTraySetT3Pos[tag]->CommaText;
        for (int i = 0; i < iAutoRight; i++)
        {
            if (MyBinPanel[tag]->iT6IsFail[i]==0)
            {
                iBinCt=0;
                for (int j = 0; j < iTestBinCount; j++)
                {
                    if (sT3List->Strings[j]==AnsiString(iTo3Unload[i]+1))
                    {
                        iBinCt++;
                        if (iBinCt>1)
                        {
                            str1.sprintf("%s Pass Bin Only Can Set One Bin.", s6TrayName[i].c_str());
                            str2.sprintf("%s Pass Bin 僅可設定一個Bin", s6TrayName[i].c_str());
                            ShowMyMessage(str1, str2);
                            sT3List->Clear();
                            delete sT3List;
                            return;
                        }
                    }
                }
            }
        }
        sT3List->Clear();
        delete sT3List;
    }

    if (CUSTOMER_CODE==CC_KYEC_LEE && (tag==eBinFT_ART || tag==eBinOffLine))
    {
        ARTBinCheck(tag);
    }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2488-2509
    // GATE (write-policy): bI32CanCelErrorBin / bI33ErrorBinBox "I/F Error"
    // WriteIniData session (golden :2488-2509)
    if (IniConfig.bI32CanCelErrorBin)
    {
        WriteIniData(FileInfo().PathCombin(szDir,"BinasgnOff.Data"), "I/F Error", "CancelError", CancelErrorBin->Checked);
    }
    else if (IniConfig.bI33ErrorBinBox)
    {
        AnsiString FileName2=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
        WriteIniData(FileName2, "I/F Error", "Bin",             iTo3PosUnload[MyBinPanel[tag]->iErrorT6]);
        WriteIniData(FileName2, "I/F Error", "Tray",            s6TrayName[MyBinPanel[tag]->iErrorT6]);
        WriteIniData(FileName2, "I/F Error", "ErrorBinBox",     rg_FixBinBox->ItemIndex);
        if (rg_FixBinBox->ItemIndex==1)
        {
            if (ed_FixBinBoxAlarmCount->Text.ToIntDef(0)<=2)
                ed_FixBinBoxAlarmCount->Text="2";
        }
        WriteIniData(FileName2, "I/F Error", "Alarm Count",     ed_FixBinBoxAlarmCount->Text);
    }
//#endif   //AI(W906-S09-B6) 20260929: see :1160

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2511-2540
    // GATE (write-policy): bAllSiteSameFailBinShowAlarm session (golden :2511-2540)
    if (CosFunction.bAllSiteSameFailBinShowAlarm)
    {
        AnsiString sBuffer1="", sBuffer="";
        for (int i = 0; i < iTestBinCount; i++)
        {
            sBuffer.sprintf("%d,", sgSpecificBin->Cells[i][1]=="V");
            sBuffer1+=sBuffer;
        }
        WriteIniData(FileInfo().PathCombin(szDir,"BinasgnOff.Data"), "AllSiteSameBin", "bSpecificFailBin", sBuffer1);

        if (CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            AnsiString FileName2=FileInfo().PathCombin(szDir, "BinasgnOff.Data");
            WriteIniData(FileName2, "I/F Error", "Bin",             iTo3PosUnload[MyBinPanel[tag]->iErrorT6]);
            WriteIniData(FileName2, "I/F Error", "Tray",            s6TrayName[MyBinPanel[tag]->iErrorT6]);
            WriteIniData(FileName2, "I/F Error", "ErrorBinBox",     rg_FixBinBox->ItemIndex);
            if (rg_FixBinBox->ItemIndex==1)
            {
                if (ed_FixBinBoxAlarmCount->Text.ToIntDef(0)<=2)
                    ed_FixBinBoxAlarmCount->Text="2";
            }
            WriteIniData(FileName2, "I/F Error", "Alarm Count",     ed_FixBinBoxAlarmCount->Text);
        }
    }
//#endif   //AI(W906-S09-B6) 20260929: see :1182

    if (bGPIBChangeAlarm)                                                      // golden :2543-2551 -- in-memory only
    {
        for (int i = 0; i < iTestBinCount; i++)
        {
            sBinConsFail[tag]->Strings[i]   =(BinSelect[tag].bConsFail[i])?"1":"0";
            sBinEnableFail[tag]->Strings[i] =(BinSelect[tag].bFailure[i])?"1":"0";
            sBinFailPercent[tag]->Strings[i]=BinSelect[tag].dFailureLimit[i];
        }
    }

    if (bTCPIPChangeAlarm)                                                     // golden :2553-2559 -- in-memory only
    {
        for (int i = 0; i < iTestBinCount; i++)
            sBinDoubleContact[tag]->Strings[i]=BinSelect[tag].iDBContact[i];
    }

    if (bSetBINCOUNT)                                                          // golden :2561-2567 -- in-memory only
    {
        for (int i = 0; i < iTestBinCount; i++)
            sBinFailIgnore[tag]->Strings[i]=BinSelect[tag].iPersentIgnore[i];
    }

    if (CUSTOMER_CODE==CC_Greatek)                                             // golden :2569-2579 -- in-memory only
    {
        if (bMOFileControlBinChangeAlarm)
        {
            for (int i = 0; i < iTestBinCount; i++)
            {
                sSpecialBinBySocket[tag]->Strings[i]     =(BinSelect[tag].bSpecialBinBySocket[i])?"1":"0";
                sSpecialBinCountBySocket[tag]->Strings[i]=IntToStr(BinSelect[tag].iSpecialBinCountBySocket[i]);
            }
        }
    }

    if (bOLPSetCategroy)                                                       // golden :2581-2589 -- in-memory only
    {
        MyBinPanel[tag]->iErrorT6=iTo6Unload[BinSelect[iTestRunMode].IfErrorT3];
        for (int i = 0; i < iTestBinCount; i++)
            sBinTraySetT3Pos[tag]->Strings[i]=BinSelect[tag].iCatDataT3Pos[i];

        for (int i = 0; i < eTrayCount; i++)
            MyBinPanel[tag]->iT6IsFail[i]=Prod.iIsFailT6[i];
    }

    SaveFunctionData(tag, FileName);   // FW-BinSel-WB: real body (writes gated inside)

    if (bSetByDLL==true)                                                       // golden :2593-2596 -- in-memory only
    {
        MyBinPanel[tag]->iErrorT6=iTo6Unload[BinSelect[iTestRunMode].IfErrorT3];
    }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2598-2622
    // GATE (write-policy): "I/F Error" Bin/Tray/Contact/Cons.Fail/Fail Percent/
    // Scan/Fail Count WriteIniData block (golden :2598-2622)
    if (CUSTOMER_CODE==CC_TFME_CHINA)
    {
        if (MyBinPanel[tag]->iErrorT6==2)
            WriteIniData(FileName, "I/F Error", "Bin", iTo3Unload[6]);
        else if (MyBinPanel[tag]->iErrorT6==11)
            WriteIniData(FileName, "I/F Error", "Bin", iTo3Unload[18]);
        else
            WriteIniData(FileName, "I/F Error", "Bin", iTo3Unload[MyBinPanel[tag]->iErrorT6+1]);
    }
    else
    {
        WriteIniData(FileName, "I/F Error", "Bin", iTo3Unload[MyBinPanel[tag]->iErrorT6]);
    }
    WriteIniData(FileName, "I/F Error", "Tray",         s6TrayName[MyBinPanel[tag]->iErrorT6]);
    WriteIniData(FileName, "I/F Error", "Contact",      0);
    WriteIniData(FileName, "I/F Error", "Cons.Fail",    0);
    WriteIniData(FileName, "I/F Error", "Fail Percent", 0);
    WriteIniData(FileName, "I/F Error", "Scan",         0);
    WriteIniData(FileName, "I/F Error", "Fail Count",   0);
//#endif   //AI(W906-S09-B6) 20260929: see :1261

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2624-2690
    // GATE (write-policy): per-Category WriteIniData loop (golden :2624-2690)
    // -- 100% WriteIniData sink; every local (`iResult`) it computes is only
    // ever consumed as the very next WriteIniData argument, no
    // independently-observable side effect survives gating this loop whole.
    AnsiString GroupStr;
    int iResult;
    for (int i = 0; i < iTestBinCount; i++)
    {
        GroupStr.sprintf("Category%d", i);
        for (int j = eBinNotUse; j < eBinSetTotal; j++)
        {
            if (MyBinPanel[tag]->BackT6PosTray[i][j])
            {
                WriteIniData(FileName, GroupStr, "Bin", iTo3PosUnload[j-eBinNotUse]);
                if (j==eBinNotUse)
                    WriteIniData(FileName, GroupStr, "Tray", "NotUse");
                else
                {
                    if (j==32)
                        WriteIniData(FileName, GroupStr, "Tray", s3TrayName[3]);
                    else
                        WriteIniData(FileName, GroupStr, "Tray", s3TrayName[iTo3PosUnload[j-eBinNotUse-1]]);
                }
                break;
            }
        }

        iResult=MyBinPanel[tag]->i2Contact[i];
        if (Prod.bD22SupportMultiDoubleContact==false)
            iResult=0;
        WriteIniData(FileName, GroupStr, "Contact", iResult);

        if (bGPIBChangeAlarm)
        {
            MyBinPanel[tag]->bConFail[i]=BinSelect[iTestRunMode].bFailure[i];
            MyBinPanel[tag]->bPersentEnable[i]=BinSelect[iTestRunMode].bFailure[i];
            MyBinPanel[tag]->dPersentNumber[i]=Prod.dFailureLimit[i];
        }

        iResult=(MyBinPanel[tag]->bConFail[i])?1:0;
        WriteIniData(FileName, GroupStr, "Cons.Fail", iResult);

        iResult=(MyBinPanel[tag]->bPersentEnable[i])?1:0;
        WriteIniData(FileName, GroupStr, "Fail Percent",        iResult);
        WriteIniData(FileName, GroupStr, "Fail Percent Ignore", MyBinPanel[tag]->iPersentIgnore[i]);
        WriteIniData(FileName, GroupStr, "Fail Percent Limit",  MyBinPanel[tag]->dPersentNumber[i]);

        iResult=(MyBinPanel[tag]->bCountEnable[i])?1:0;
        WriteIniData(FileName, GroupStr, "Fail Count",        iResult);
        WriteIniData(FileName, GroupStr, "Fail Count Ignore", MyBinPanel[tag]->iCountIgnore[i]);
        WriteIniData(FileName, GroupStr, "Fail Count Limit",  MyBinPanel[tag]->iCountNumber[i]);

        iResult=(MyBinPanel[tag]->bSpecialBinByArm[i])?1:0;
        WriteIniData(FileName, GroupStr, "Special Bin By Arm",        iResult);
        WriteIniData(FileName, GroupStr, "Special Bin Count By Arm", (int)MyBinPanel[tag]->iSpecialBinCountByArm[i]);   //AI(W906-S09-B6) 20260929: (int) = golden systools.h unsigned overload -> WriteInteger (FileRW/BinSelect.gen.inc:44)
        iResult=(MyBinPanel[tag]->bSpecialBinBySocket[i])?1:0;
        WriteIniData(FileName, GroupStr, "Special Bin By Socket",        iResult);
        WriteIniData(FileName, GroupStr, "Special Bin Count By Socket",  (int)MyBinPanel[tag]->iSpecialBinCountBySocket[i]);   //AI(W906-S09-B6) 20260929: (int) = golden systools.h unsigned overload -> WriteInteger (FileRW/BinSelect.gen.inc:44)

        WriteIniData(FileName, GroupStr, "By Bin Low Yield",                  MyBinPanel[tag]->bLowYield[i]);
        WriteIniData(FileName, GroupStr, "By Bin and Site Compare Arm Yield", MyBinPanel[tag]->bArmYield[i]);
        WriteIniData(FileName, GroupStr, "By Bin Compare Site Yield",         MyBinPanel[tag]->bSiteYield[i]);

        iResult=(MyBinPanel[tag]->bScan[i])?1:0;
        WriteIniData(FileName, GroupStr, "Scan", iResult);
    }
//#endif   //AI(W906-S09-B6) 20260929: see :1285

    if (bGPIBChangeAlarm || bSetByDLL)                                         // golden :2692-2697 -- in-memory only
    {
        for (int i = 0; i < eTrayCount; i++)
            MyBinPanel[tag]->iT6IsFail[i]=BinSelect[iTestRunMode].iStackDefFailCate[iTo3Unload[i]];
    }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2699-2739
    // GATE (write-policy): per-tray Pass/Fail/Link/AutoRetest/Cate R
    // WriteIniData block (golden :2699-2739)
    for (int i = 0; i < eTrayCount; i++)
    {
        WriteIniData(FileName, s6TrayName[i], "Pass/Fail", MyBinPanel[tag]->iT6IsFail[i]);
        WriteIniData(FileName, s6TrayName[i], "Link",      MyBinPanel[tag]->bT6Link[i]);
    }

    if (USE_AUTO_RETEST==eartInstall || IniConfig.bA38_SLT_Summary)
    {
        for (int i = eAuto1; i <= iAutoRight; i++)
        {
            if (UNLOADER_ART[i]==eartInstall)
                WriteIniData(FileName, s6TrayName[i], "AutoRetest", MyBinPanel[tag]->bT6ART[i]);
            else
                WriteIniData(FileName, s6TrayName[i], "AutoRetest", false);
        }

        if ((USE_AUTO_RETEST==eartInstall && CosFunction.bUseSCKART && TestIF_File.iSCKART_SortMode==1 &&
             (tag==eBinFT || tag==eBinOffLine)) || IniConfig.bA38_SLT_Summary)
        {
            for (int i = 0; i < eTrayCount; i++)
                WriteIniData(FileName, s6TrayName[i], "Cate R", MyBinPanel[tag]->bT6CateR[i]);
        }
        else
        {
            for (int i = 0; i < eTrayCount; i++)
                WriteIniData(FileName, s6TrayName[i], "Cate R", false);
        }
    }
    else
    {
        for (int i = eAuto1; i <= iAutoRight; i++)
            WriteIniData(FileName, s6TrayName[i], "AutoRetest", false);
    }
//#endif   //AI(W906-S09-B6) 20260929: see :1360

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:2742-2750
    // GATE (write-policy): "BinModel"/"bPrime" + "Other"/"bShow0XBin" session
    // (golden :2742-2750)
    AnsiString FileName2=FileInfo().PathCombin(szDir, "Binasgn.Data");
    WriteIniData(FileName2, "BinModel", "bPrime", iBinModelPrime);
    if (CosFunction.bShow0Xbin)
        WriteIniData(FileName2, "Other", "bShow0XBin", chkShow0Xbin->Checked);
//#endif   //AI(W906-S09-B6) 20260929: see :1398

    AddSpace(szDir);   // common.h documented no-op stub, not config content mutation
}

//---------------------------------------------------------------------------
//  ReadFile -- golden :1119-1596. The MUST-HAVE method. See forms/fBinSel.h
//  GATE REGISTER (G1)/(G2)/(G3) and WRITE-PATH GATE TABLE for every gate
//  below.
//---------------------------------------------------------------------------
void TfBinSel::ReadFile(bool bDelOffline, bool bChangeNeme, AnsiString /*sFileName*/)
{
    AnsiString szDir="";

    // GATE (G1): fQAMode->ReadFile(); -- `grep -rn "\bfQAMode\b"
    //AI(W906-FW3-BTQ1) 20260828: SUPERSEDED -- fQAMode 自 FW3-BTQ1 起有宣告（forms/fQAMode.h）。
    //    本 gate 仍為 #if 0；解閘要先確認 ReadFile 的寫入語意是否可接受。
    // --include=*.h .` -- 0 hits (20260819).
    ReadPrimeDara();
    AnsiString SavePath[]={"\\BinasgnOff.Data", "\\Binasgn.Data", "\\BinasgnOff-Line.Data", "\\BinasgnOff_ART.Data", "\\Binasgn_ART.Data", "\\Binasgn_MRT_RT.Data", "\\Binasgn_MRT.Data"};

    if (IniConfig.bA02BinModelPrime)
    {
        if (IniConfig.bFTBin2RTBin==true && (iBinModelPrime==0 || CosFunction.bDisableRTBinSet))
            SavePath[0]="\\Binasgn.Data";
        else
            SavePath[0]="\\BinasgnOff.Data";
    }
    else
    {
        if (IniConfig.bFTBin2RTBin==true || CosFunction.bDisableRTBinSet)
            SavePath[0]="\\Binasgn.Data";
        else
            SavePath[0]="\\BinasgnOff.Data";
    }

    for (int i = 0; i < eTrayCount; i++)
    {
        if (i==eFix1 || i==eAuto1)
            bCanLinkT6[i]=false;
        else if (IniConfig.bAutoTrayLink==false && i<=iAutoRight)
            bCanLinkT6[i]=false;
        else if (AUTO3_IS_MAGAZINE==1 && (i==iMagAtAuto))
            bCanLinkT6[i]=false;
        else if (AUTO_EMPTY_COLOR!=4 && i==eAuto6)
            bCanLinkT6[i]=false;
        else if (i==eBulkBox)
            bCanLinkT6[i]=false;
        else if (Prod.iTrayType[i]==tNotUse)
            bCanLinkT6[i]=false;
        else
            bCanLinkT6[i]=true;
    }

    if (bDelOffline==true)
    {
        if (bPurgeOutAllDevice)
        {
        }
        else
        {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:1191-1192
            // GATE (write-policy): destructive delete
            szDir=GetRecipeFileName(SavePath[2]);
            DeleteFile(szDir.c_str());
//#endif   //AI(W906-S09-B6) 20260929: see :1466
        }
    }

    if (IniConfig.bI32CanCelErrorBin && bChangeNeme==false)
    {
        szDir=GetRecipeFileName("BinasgnOff.Data");
        bCancelErrorBin=CheckAndReadIniData(szDir, "I/F Error", "CancelError", false);
    }
    else
    {
        bCancelErrorBin=false;
        if (IniConfig.bI33ErrorBinBox && bChangeNeme==false)
        {
            szDir=GetRecipeFileName("BinasgnOff.Data");
            iHWFix_BinBox=CheckAndReadIniData(szDir, "I/F Error", "ErrorBinBox", 0);
            iErrorBinBoxAlarm=CheckAndReadIniData(szDir, "I/F Error", "Alarm Count", 0);
            if (iHWFix_BinBox==1 && iErrorBinBoxAlarm<=2)
                iErrorBinBoxAlarm=2;
        }
        else
        {
            iHWFix_BinBox=0;
        }
    }

    if (CosFunction.bAllSiteSameFailBinShowAlarm)
    {
        AnsiString SBuffer1="", SBuffer="";
        szDir=GetRecipeFileName(SavePath[0]);
        SBuffer1=CheckAndReadIniData(szDir, "AllSiteSameBin", "bSpecificFailBin", AnsiString("5"));
        if (SBuffer1=="5")
        {
            SBuffer1="";
            for (int i = 0; i < iTestBinCount; i++)
            {
                bSpecificBin[i]=CheckAndReadIniData(szDir, "AllSiteSameBin", "bSpecificFailBin"+IntToStr(i), false);
                SBuffer.sprintf("%d,", bSpecificBin[i]);
                SBuffer1+=SBuffer;
            }
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:1239
            // GATE (write-policy)
            WriteIniData(szDir, "AllSiteSameBin", "bSpecificFailBin", SBuffer1);
//#endif   //AI(W906-S09-B6) 20260929: see :1510
            SBuffer1 = CheckAndReadIniData(szDir, "AllSiteSameBin", "bSpecificFailBin", AnsiString("0"));
        }

        for (int i = 0; i < iTestBinCount; i++)
            bSpecificBin[i]=(SBuffer1.SubString(i*2+1, 1)=="1");
    }

    for (int tag = 0; tag < eBinTypeTotal; tag++)
    {
        if (CosFunction.bUseMRTMode==false)
        {
            if (tag==eBinRT_MRT || tag==eBinFT_MRT)
                continue;
        }

        if (USE_AUTO_RETEST==0)
        {
            if (tag==eBinRT_ART || tag==eBinFT_ART)
                continue;
        }
        else if (USE_AUTO_RETEST==1 && CosFunction.bUseSCKART)
        {
            if (tag==eBinRT_ART || tag==eBinFT_ART)
                continue;
        }

        szDir=GetRecipeFileName(SavePath[tag]);

        if (bChangeNeme)
        {
            if (tag!=1)
                continue;
        }

        if (CUSTOMER_CODE==CC_TFME_CHINA)
        {
            BinSelect[tag].IfErrorT3Define=CheckAndReadIniData(szDir, "I/F Error", "BinDefine", 0);
            BinSelect[tag].IfErrorT3=CheckAndReadIniData(szDir, "I/F Error", "Bin", e3Fix2);
            if (BinSelect[tag].IfErrorT3Define==0)
            {
#if 0
                // GATE (write-policy)
                WriteIniData(szDir, "I/F Error", "Bin", BinSelect[tag].IfErrorT3+1);
                WriteIniData(szDir, "I/F Error", "BinDefine", 1);
#endif
            }
            else
            {
                BinSelect[tag].IfErrorT3=BinSelect[tag].IfErrorT3-1;
            }
        }
        else
        {
            BinSelect[tag].IfErrorT3=CheckAndReadIniData(szDir, "I/F Error", "Bin", e3Fix2);
        }

        if (bCancelErrorBin || iHWFix_BinBox==1)
        {
            BinSelect[tag].IfErrorT3=iTo3Unload[eBulkBox];
        }
        // GATE (G2) OPENED 20260826 (FW-UNGATE-W29) -- golden cBinSel.cpp:1309-1312.
        // 舊理由是一條 absence claim：`grep -rn "AutoForm\[" --include=*.h .`
        // -- 0 hits (20260819)。**那條在寫下當時就是假的**：cprod.h:1366 的
        // `extern TRAY_TYPE_PARA *AutoForm[eTrayCount];` 早在 20260626 就已存在
        // （八週前），且本檔 :49 就 include 了 cprod.h。
        // ⚠ 撞名：cprod.h:374 另有結構成員 `TRAY_DATA AutoForm[eTrayCount];`
        //   （SYSTEM_TRAY_FORM 內，即 TrayForm.AutoForm），型別與這裡不同；
        //   本 gate 的主體是 :1366 的那個全域指標陣列。
        // 指標由 cinitial.cpp:14426 `AutoForm[i]=&TrayForm.Auto[i];` 填入
        // （cprod.cpp:21 是定義，靜態零初始化，填入前為 NULL）。
        // 同一個運算式 `AutoForm[iBinBoxAtFix]->iTrayType` 已在 6 個編進去的
        // 站點 live 且無 NULL 守衛（重跑 20260826，掃描器排除 build*/.git/.svn
        // 且會判 #if 0，0 個 gated）：aoutarm.cpp:1621、:1826、:1897、:3953、
        // asortarm.cpp:4255、csystem.cpp:16158。（asortarm.cpp:4257 是緊接的
        // `AutoForm[iBinBoxAtFix]->iBinBoxAlarm`，同一顆指標、不同成員。）
        //
        // 開閘後這條分支實際會做什麼：只在 IniConfig.bBinBox 為真、且
        // iBinBoxAtFix（cmydef.cpp:3042，值 2）那盤的 iTrayType==3 時，把
        // BinSelect[tag].IfErrorT3 直接設成 iTo3Unload[iBinBoxAtFix]，也就是
        // 把 I/F Error 的 IC 導到 Bin Box。純記憶體賦值，不寫檔（本檔所有
        // WriteIniData/DeleteFile 仍照 WRITE-PATH GATE TABLE 全部 gated）、
        // 不動馬達、不送對外命令。
        // ⚠ 這是行為變更：BinBox 機台先前會落到下面的 else 長鏈（Fix2/Mag14
        //   等 fallback），開閘後才會照 golden 導到 Bin Box。
        // ⚠ 求值順序：`IniConfig.bBinBox &&` 在前，非 BinBox 機台永遠不會
        //   deref AutoForm[]；與 golden 完全一致。
        // 原本被複製到這個 arm 的 58 行（與下面的 else arm 逐字相同，這是
        // fail-closed 的實作方式）在本波移除，只留 else arm 一份。
        else if (IniConfig.bBinBox && AutoForm[iBinBoxAtFix]->iTrayType==3)   //jou 2012-12-11 support Bin Box -- golden :1309
        {
            BinSelect[tag].IfErrorT3=iTo3Unload[iBinBoxAtFix];                // golden :1311
        }
        else
        {
            if (BinSelect[tag].IfErrorT3<0)
            {
                BinSelect[tag].IfErrorT3=e3Fix2;
            }

            if (CosFunction.bLoaderTrayToAuto1 && TrayForm.LoaderToEmptyColor[iRunStartMode]==2)
            {
                if (iTo6Unload[BinSelect[tag].IfErrorT3]==eAuto1)
                    BinSelect[tag].IfErrorT3=iTo3Unload[eMag14];
            }

            if (AUTO3_IS_MAGAZINE==1)
            {
                if (iTo6Unload[BinSelect[tag].IfErrorT3]==eAuto3)
                    BinSelect[tag].IfErrorT3=iTo3Unload[eMag14];

                if (TestIF_File.iMagFixTrayType==1 &&
                    iTo6Unload[BinSelect[tag].IfErrorT3]>=iFixMin &&
                    iTo6Unload[BinSelect[tag].IfErrorT3]<=iFixRightHalf)
                {
                    BinSelect[tag].IfErrorT3=iTo3Unload[eMag14];
                }
            }
            else
            {
                if (TrayForm.iFixTrayMode==false &&
                    iTo6Unload[BinSelect[tag].IfErrorT3]>=iFixMax &&
                    iTo6Unload[BinSelect[tag].IfErrorT3]<=iFixRightHalf)
                {
                    BinSelect[tag].IfErrorT3=iTo3Unload[iFixMax];
                }
            }

            if (TestIF_File.bEnableQASampling &&
                iTo6Unload[BinSelect[tag].IfErrorT3]==iTo6Unload[TestIF_File.iQASamplingT3Pos])
            {
                if (iTo6Unload[TestIF_File.iQASamplingT3Pos]==iFixMax)
                    BinSelect[tag].IfErrorT3=TestIF_File.iQASamplingT3Pos-1;
                else if (iTo6Unload[TestIF_File.iQASamplingT3Pos]==iFixRightHalf)
                    BinSelect[tag].IfErrorT3=TestIF_File.iQASamplingT3Pos-1;
                else
                    BinSelect[tag].IfErrorT3=TestIF_File.iQASamplingT3Pos+1;
            }

            if (USE_ROTATE==eCynRot)
            {
                if (iTo6Unload[BinSelect[tag].IfErrorT3]==iRotate_Out_Tray6)
                    BinSelect[tag].IfErrorT3=iTo3Unload[eFix2];
            }

            if (MachineTypeChoice==Type_HT9045 &&
                FIX3_FULL_PLACE==Fix3K_UseCylinder &&
                USE_ROTATE==eMotRot &&
                iTo6Unload[BinSelect[tag].IfErrorT3]==e3Fix3)
            {
                BinSelect[tag].IfErrorT3=iTo3Unload[eFix2];
            }
        }

        MyBinPanel[tag]->iErrorT6=iTo6Unload[BinSelect[tag].IfErrorT3];

        for (int i = 0; i < iTestBinCount; i++)
        {
            for (int j = eBinNotUse; j < eBinSetTotal; j++)
                MyBinPanel[tag]->BackT6PosTray[i][j]=0;
        }

        ReadFunctionData(tag, szDir);   // FW-BinSel-WB: real body (see its own banner)

        for (int i = 0; i < eTrayCount; i++)
        {
            MyBinPanel[tag]->iT6IsFail[i]=CheckAndReadIniData(szDir, s6TrayName[i], "Pass/Fail", 0);
            if (MyBinPanel[tag]->iT6IsFail[i]==0 && MyBinPanel[tag]->iErrorT6==i)
                MyBinPanel[tag]->iT6IsFail[i]=1;

            BinSelect[tag].iStackDefFailCate[iTo3Unload[i]]=MyBinPanel[tag]->iT6IsFail[i];

            if (IniConfig.bAutoTrayLink==true)
            {
                if ((i>=eAuto2 && i<=iAutoRight) || (AUTO3_IS_MAGAZINE==1 && i>=iMagMin && i<=iMagMax))
                {
                    MyBinPanel[tag]->bT6Link[i]=CheckAndReadIniData(szDir, s6TrayName[i], "Link", 0);
                    BinSelect[tag].bAutoLink[iTo3Unload[i]]=MyBinPanel[tag]->bT6Link[i];
                }
            }

            if (i>=eFix2 && i<=iFixRight)
            {
                MyBinPanel[tag]->bT6Link[i]=CheckAndReadIniData(szDir, s6TrayName[i], "Link", 0);
                BinSelect[tag].bAutoLink[iTo3Unload[i]]=MyBinPanel[tag]->bT6Link[i];
            }

            if (AUTO3_IS_MAGAZINE==1)
            {
                if (i>=eMag1 && i<=eMag14)
                {
                    iLinkTrayMax=eMag14;
                    BinSelect[tag].bMagazineLink[i-eMag1]=CheckAndReadIniData(szDir, s6TrayName[i], "Link", 0);
                    MyBinPanel[tag]->bT6Link[i]=BinSelect[tag].bMagazineLink[i-eMag1];
                }
            }

            if (CosFunction.bUseSCKART)
            {
                if (i<=iAutoRight)
                {
                    if (USE_AUTO_RETEST==eartInstall && (tag==eBinFT || tag==eBinOffLine))
                    {
                        MyBinPanel[tag]->bT6ART[i]=CheckAndReadIniData(szDir, s6TrayName[i], "AutoRetest", false);
                        if (MyBinPanel[tag]->bT6ART[i])
                            BinSelect[tag].iARTTrack=i;
                    }
                    else
                    {
                        MyBinPanel[tag]->bT6ART[i]=false;
                    }
                }
                else
                {
                    MyBinPanel[tag]->bT6ART[i]=false;
                }

                if ((USE_AUTO_RETEST==eartInstall && TestIF_File.iSCKART_SortMode==1 && (tag==eBinFT || tag==eBinOffLine)) ||
                    IniConfig.bA38_SLT_Summary)
                {
                    MyBinPanel[tag]->bT6CateR[i]=CheckAndReadIniData(szDir, s6TrayName[i], "Cate R", false);
                }
                else
                {
                    MyBinPanel[tag]->bT6CateR[i]=false;
                }
                BinSelect[tag].bAutoRetest[iTo3Unload[i]]=MyBinPanel[tag]->bT6ART[i];
                BinSelect[tag].bCateR[iTo3Unload[i]]=MyBinPanel[tag]->bT6CateR[i];
            }
            else
            {
                if (i<=iAutoRight)
                {
                    if (USE_AUTO_RETEST==eartInstall && (IniConfig.bA10_AutoReTest || bAutoReTest_ART) && tag==eBinFT_ART)
                    {
                        MyBinPanel[tag]->bT6ART[i]=CheckAndReadIniData(szDir, s6TrayName[i], "AutoRetest", false);

                        if (CUSTOMER_CODE==CC_TSMC_TAINAN)
                        {
                            if (BinSelect[tag].bAutoRetest[iTo3Unload[i]]==true && MyBinPanel[tag]->iT6IsFail[i]==0)
                                MyBinPanel[tag]->bT6ART[i]=false;
                            MyBinPanel[tag]->iT6IsFail[eAuto3]=1;
                            BinSelect[tag].bAutoRetest[e3Auto3]=true;
                            MyBinPanel[tag]->bT6ART[eAuto3]=true;
                        }
                        else if (CUSTOMER_CODE!=CC_ASE_KaohSiung && CUSTOMER_CODE!=CC_KYEC_LEE)
                        {
                            MyBinPanel[tag]->iT6IsFail[eAuto2]=1;
                            BinSelect[tag].bAutoRetest[e3Auto2]=true;
                            MyBinPanel[tag]->bT6ART[eAuto2]=true;
                        }
                    }
                    else
                    {
                        MyBinPanel[tag]->bT6ART[i]=false;
                    }
                }
                else
                {
                    MyBinPanel[tag]->bT6ART[i]=false;
                }
                BinSelect[tag].bAutoRetest[iTo3Unload[i]]=MyBinPanel[tag]->bT6ART[i];
            }
        }

        if (CUSTOMER_CODE==CC_KYEC_LEE && (tag==eBinFT_ART || tag==eBinOffLine))
        {
            ARTBinCheck(tag);
        }

        mtTrayNameSetColor(tag);   // FW-BinSel-WB: real body landed, entire body gated (see its own banner)
    }

    ReadWriteMRTMode(0);
    ReadWriteSpecialFunction(false);

    for (int tag = 0; tag < eBinTypeTotal; tag++)
        BinSelect[tag].iStackDefFailCate[BinSelect[tag].IfErrorT3]=1;

    CancelErrorBin->Checked = bCancelErrorBin;
    rg_FixBinBox->ItemIndex = iHWFix_BinBox;
    ed_FixBinBoxAlarmCount->Text=iErrorBinBoxAlarm;

    SetWorkParameter();

    for (int i = 0; i < eBinTypeTotal; i++)
    {
        sT3TrayType[i]->Clear();
        sT3CateR[i]->Clear();
        sT6Retest[i]->Clear();

        for (int j = e3Auto1; j < eTrayCount; j++)
        {
            sT3TrayType[i]->Add("0");
            sT3CateR[i]->Add("0");
        }

        for (int j = eAuto1; j <= iAutoRight; j++)
            sT6Retest[i]->Add("0");
    }

    for (int tag = 0; tag < eBinTypeTotal; tag++)
    {
        for (int i = 0; i < iTestBinCount; i++)
        {
            if (sBinType[tag]->Count<=i)
                sBinType[tag]->Add(AnsiString(Prod.bIsPassBin[i]?"0":"1"));
            else
                sBinType[tag]->Strings[i]=AnsiString(Prod.bIsPassBin[i]?"0":"1");
        }

        for (int i = 0; i < eTrayCount; i++)
        {
            if (iTo3Unload[i]>=0 && iTo3Unload[i]<e3TrayCount)
            {
                sT3TrayType[tag]->Strings[iTo3Unload[i]]=BinSelect[tag].iStackDefFailCate[iTo3Unload[i]];
                sT3CateR[tag]->Strings[iTo3Unload[i]]=(BinSelect[tag].bCateR[iTo3Unload[i]])?"1":"0";
            }
        }

        for (int i = eAuto1; i <= iAutoRight; i++)
        {
            if (iTo3Unload[i]>=0 && iTo3Unload[i]<e3TrayCount)
                sT6Retest[tag]->Strings[i]=(BinSelect[tag].bAutoRetest[iTo3Unload[i]])?"1":"0";
        }
    }

    if (CUSTOMER_CODE==CC_KYEC_LEE)
    {
        if (IniConfig.bA73UserOverrideFTRTBin)
        {
            IniConfig.bFTBin2RTBin       = false;
            CosFunction.bDisableRTBinSet = false;
        }
        else
        {
            IniConfig.bFTBin2RTBin       = true;
            CosFunction.bDisableRTBinSet = true;
        }
    }
    SetPrimeButton();   // FW-BinSel-WB: real body landed, entire body gated (see its own banner)
    LotSummary.SetIsRTBin();
    // GATE (write-policy): ATKRecipeInfo->SaveFile(); -- writes a
    // work-file-compare file (golden comment "為了加快ini讀取速度"/"要新增
    // 工作檔比對用的檔案"); treated as a write per WRITE-PATH GATE TABLE
    // regardless of ATKRecipeInfo's own landed status.
}

//---------------------------------------------------------------------------
//  ReadFunctionData -- golden :4807-5667. FW-BinSel-WB (this wave). ACTIVE:
//  every CheckAndReadIniData/CheckSectionExist call (reads are POLICY-ALLOWED,
//  see forms/fBinSel.h WRITE-PATH GATE TABLE) plus the full old/new-format
//  BinSelect[tag]/MyBinPanel[tag] in-memory post-processing golden's own body
//  does after the per-tag read cascade (golden :5356-5636 -- NOT mechanical
//  reads, real validation logic, kept ACTIVE). GATE: the `MyBinPanel[tag]->
//  ed*->Text=...` widget-mirror block (golden :5637-5661, see inline GATE
//  below) and, transitively, `SaveFunctionData`'s own writes (called from
//  within this method too, golden :5508/:5664 -- see that function's own
//  banner). DEVIATION: signature gains an `AnsiString szDir` parameter, the
//  same substitution WAVE A already used throughout `ReadFile`/`SaveOther` --
//  golden relies on `FormSysTools` (a `TFormSysTools*` ini-file cache object,
//  systools.h/.cpp, `OpenFormData(szDir)`'d by the CALLER just before this
//  method runs, golden :1277) staying implicitly bound to the caller's file;
//  `TFormSysTools` has no port anywhere in this tree (`grep -rln
//  "TFormSysTools\|OpenFormData" --include=*.h --include=*.cpp .` -- only
//  dfm2rc layout-generator artifacts under tools/, no real class, 20260819),
//  so `szDir` (the SAME value `ReadFile` already computes right before
//  calling this, unchanged) is threaded through explicitly instead, matching
//  every other `FormSysTools->CheckAndReadIniData(...)` substitution already
//  established in this file.
//---------------------------------------------------------------------------
void TfBinSel::ReadFunctionData(int tag, AnsiString szDir)
{
    bool bHasNewSetupData=false;
    AnsiString GroupName, GroupStr;
    int iData, iT3, iT6;
    int iNewBinFormat=0;                                                        //kevin 20160928 判斷 Bin format 是否有轉過記錄

    if(CUSTOMER_CODE!=CC_AMKOR_China)                                           //JerryYang 20180207 (Steven) Amkor上海要求使用舊的讀檔方式
    {
        if(tag==eBinRT)
        {
            //Steven 20160623 : 當IniConfig.bFTBin2RTBin==true && 讀檔時, RT Bin要跟FT Bin一樣
            if((IniConfig.bA02BinModelPrime && IniConfig.bFTBin2RTBin==true && (iBinModelPrime==0 || CosFunction.bDisableRTBinSet)) ||
               (IniConfig.bA02BinModelPrime==false && (IniConfig.bFTBin2RTBin==true || CosFunction.bDisableRTBinSet)))
            {
                GroupName="Bin Func FT";
                if(CheckSectionExist(szDir, GroupName))
                {
                    iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat",77);//kevin 2016028 使用新的bin format
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                         //kevin 2016028 add 使用新的bin format
                    {
                        if(iNewBinFormat!=0 &&iNewBinFormat!=77)                //kevin 20161108
                           bHasNewSetupData=true;
                    }
                    else
                    {
                        bHasNewSetupData=true;
                    }

                    if(bHasNewSetupData)                                        //kevin 20161108
                    {
                        sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3617 BinTraySetting", AnsiString(""));
                        sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3656 BinType", AnsiString(""));
                        sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3636 BinDoubleContac", AnsiString(""));
                        sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3676 BinConsFail", AnsiString(""));
                        sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10641 BinEnableFail", AnsiString(""));
                        sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10642 BinFailPercent", AnsiString(""));
                        sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10643 BinFailIgnore", AnsiString(""));
                        sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10644 BinCountEnable", AnsiString(""));
                        sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10646 BinCountIgnore", AnsiString(""));
                        sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10645 BinCountNumber", AnsiString(""));
                        sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10647 SpecialBinByArm", AnsiString(""));
                        sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10648 SpecialBinCountByArm", AnsiString(""));
                        sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10649 SpecialBinBySocket", AnsiString(""));
                        sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10650 SpecialBinCountBySocket", AnsiString(""));
                        sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10680 LowYield", AnsiString(""));
                        sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10681 ArmYield", AnsiString(""));
                        sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10682 SiteYield", AnsiString(""));
                        sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10683 BySiteClean", AnsiString(""));
                        sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10684 ByBinClean", AnsiString(""));
                        //JerryYang 20170712 (Steven) by site by bin compare percent
                        //==>
                        sSpecBinBySiteCompareEnable       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10685 SpecBinBySiteCompareEnable", AnsiString(""));
                        sSpecBinBySiteCompareIgnore       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10686 SpecBinBySiteCompareIgnore", AnsiString(""));
                        sSpecBinBySiteComparePercent      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10687 SpecBinBySiteComparePercent",  AnsiString(""));
                        sSpecBinByArmPerSiteCompareEnable [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10688 SpecBinByArmPerSiteCompareEnable", AnsiString(""));
                        sSpecBinByArmPerSiteCompareIgnore [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10689 SpecBinByArmPerSiteCompareIgnore", AnsiString(""));
                        sSpecBinByArmPerSiteComparePercent[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10690 SpecBinByArmPerSiteComparePercent",  AnsiString(""));
                        //<==
                        //JerryYang 20170712 (Steven) by site by bin compare percent

                        sBinTrayLinked  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(10698)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                        sBinLinked      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(10699)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                        sMagazineSetup  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(10700)", AnsiString("0"));

                        sAOIBinTraySetting[tag]->CommaText=CheckAndReadIniData(szDir, GroupName,"AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                    }
                }
            }
            else
            {
                GroupName="Bin Func RT";
                if(CheckSectionExist(szDir, GroupName))
                {
                    iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat", 77);//kevin 2016028 使用新的bin format
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                         //kevin 2016028 add 使用新的bin format
                    {
                        if(iNewBinFormat!=77)                                   //kevin 20160928
                            bHasNewSetupData=true;
                    }
                    else
                    {
                        bHasNewSetupData=true;
                    }

                    if(bHasNewSetupData)                                        //kevin 20161108
                    {
                        sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3717 BinTraySetting", AnsiString(""));
                        sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3718 BinDoubleContac", AnsiString(""));
                        sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3719 BinType", AnsiString(""));
                        sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3720 BinConsFail", AnsiString(""));
                        sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10741 BinEnableFail", AnsiString(""));
                        sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10742 BinFailPercent", AnsiString(""));
                        sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10743 BinFailIgnore", AnsiString(""));
                        sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10744 BinCountEnable", AnsiString(""));
                        sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10745 BinCountNumber", AnsiString(""));
                        sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10746 BinCountIgnore", AnsiString(""));
                        sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10747 SpecialBinByArm", AnsiString(""));
                        sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10748 SpecialBinCountByArm", AnsiString(""));
                        sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10749 SpecialBinBySocket", AnsiString(""));
                        sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10750 SpecialBinCountBySocket", AnsiString(""));
                        sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10780 LowYield", AnsiString(""));
                        sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10781 ArmYield", AnsiString(""));
                        sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10782 SiteYield", AnsiString(""));
                        sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10783 BySiteClean", AnsiString(""));
                        sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10784 ByBinClean", AnsiString(""));
                        //JerryYang 20170712 (Steven) by site by bin compare percent
                        //==>
                        sSpecBinBySiteCompareEnable       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10785 SpecBinBySiteCompareEnable", AnsiString(""));
                        sSpecBinBySiteCompareIgnore       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10786 SpecBinBySiteCompareIgnore", AnsiString(""));
                        sSpecBinBySiteComparePercent      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10787 SpecBinBySiteComparePercent",  AnsiString(""));
                        sSpecBinByArmPerSiteCompareEnable [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10788 SpecBinByArmPerSiteCompareEnable", AnsiString(""));
                        sSpecBinByArmPerSiteCompareIgnore [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10789 SpecBinByArmPerSiteCompareIgnore", AnsiString(""));
                        sSpecBinByArmPerSiteComparePercent[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10790 SpecBinByArmPerSiteComparePercent",  AnsiString(""));
                        //<==
                        //JerryYang 20170712 (Steven) by site by bin compare percent

                        sBinTrayLinked  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(10798)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                        sBinLinked      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(10799)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                        sMagazineSetup  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(10800)", AnsiString("0"));

                        sAOIBinTraySetting[tag]->CommaText=CheckAndReadIniData(szDir, GroupName,"AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                    }
                }
            }
        }
        else if(tag==eBinFT)
        {
            GroupName="Bin Func FT";
            if(CheckSectionExist(szDir, GroupName))
            {
                iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat",77);//kevin 2016028 使用新的bin format
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 2016028 add 使用新的bin format
                {
                    if(iNewBinFormat!=77)                                       //kevin 20160928
                        bHasNewSetupData=true;
                }
                else
                {
                    bHasNewSetupData=true;
                }

                if(bHasNewSetupData)                                            //kevin 20161108
                {
                    sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3617 BinTraySetting", AnsiString(""));
                    sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3656 BinType", AnsiString(""));
                    sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3636 BinDoubleContac", AnsiString(""));
                    sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3676 BinConsFail", AnsiString(""));
                    sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10641 BinEnableFail", AnsiString(""));
                    sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10642 BinFailPercent", AnsiString(""));
                    sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10643 BinFailIgnore", AnsiString(""));
                    sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10644 BinCountEnable", AnsiString(""));
                    sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10646 BinCountIgnore", AnsiString(""));
                    sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10645 BinCountNumber", AnsiString(""));
                    sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10647 SpecialBinByArm", AnsiString(""));
                    sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10648 SpecialBinCountByArm", AnsiString(""));
                    sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10649 SpecialBinBySocket", AnsiString(""));
                    sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10650 SpecialBinCountBySocket", AnsiString(""));
                    sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10680 LowYield", AnsiString(""));
                    sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10681 ArmYield", AnsiString(""));
                    sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10682 SiteYield", AnsiString(""));
                    sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10683 BySiteClean", AnsiString(""));
                    sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10684 ByBinClean", AnsiString(""));
                    //JerryYang 20170712 (Steven) by site by bin compare percent
                    //==>
                    sSpecBinBySiteCompareEnable       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10685 SpecBinBySiteCompareEnable", AnsiString(""));
                    sSpecBinBySiteCompareIgnore       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10686 SpecBinBySiteCompareIgnore", AnsiString(""));
                    sSpecBinBySiteComparePercent      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10687 SpecBinBySiteComparePercent",  AnsiString(""));
                    sSpecBinByArmPerSiteCompareEnable [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10688 SpecBinByArmPerSiteCompareEnable", AnsiString(""));
                    sSpecBinByArmPerSiteCompareIgnore [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10689 SpecBinByArmPerSiteCompareIgnore", AnsiString(""));
                    sSpecBinByArmPerSiteComparePercent[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10690 SpecBinByArmPerSiteComparePercent", AnsiString(""));
                    //<==
                    //JerryYang 20170712 (Steven) by site by bin compare percent

                    sBinTrayLinked  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(10698)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                    sBinLinked      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(10699)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                    sMagazineSetup  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(10700)", AnsiString("0"));

                    sAOIBinTraySetting[tag]->CommaText=CheckAndReadIniData(szDir, GroupName,"AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                }
            }
        }
        else if(tag==eBinOffLine)
        {
            GroupName="Bin Func OffLine";
            if(CheckSectionExist(szDir, GroupName))
            {
                iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat",77);//kevin 2016028 使用新的bin format
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 2016028 add 使用新的bin format
                {
                    if(iNewBinFormat!=77)                                       //kevin 20160928
                        bHasNewSetupData=true;
                }
                else
                {
                    bHasNewSetupData=true;
                }

                if(bHasNewSetupData)                                            //kevin 20161108
                {
                    sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3722 BinTraySetting", AnsiString(""));
                    sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3723 BinDoubleContac", AnsiString(""));
                    sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3724 BinType", AnsiString(""));
                    sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3725 BinConsFail", AnsiString(""));
                    sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10941 BinEnableFail", AnsiString(""));
                    sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10942 BinFailPercent", AnsiString(""));
                    sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10943 BinFailIgnore", AnsiString(""));
                    sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10944 BinCountEnable", AnsiString(""));
                    sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10945 BinCountNumber", AnsiString(""));
                    sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10946 BinCountIgnore", AnsiString(""));
                    sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10947 SpecialBinByArm", AnsiString(""));
                    sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10948 SpecialBinCountByArm", AnsiString(""));
                    sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10949 SpecialBinBySocket", AnsiString(""));
                    sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10950 SpecialBinCountBySocket", AnsiString(""));
                    sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10980 LowYield", AnsiString(""));
                    sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10981 ArmYield", AnsiString(""));
                    sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10982 SiteYield", AnsiString(""));
                    sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10983 BySiteClean", AnsiString(""));
                    sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10984 ByBinClean", AnsiString(""));
                    //JerryYang 20170712 (Steven) by site by bin compare percent
                    //==>
                    sSpecBinBySiteCompareEnable       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10985 SpecBinBySiteCompareEnable", AnsiString(""));
                    sSpecBinBySiteCompareIgnore       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10986 SpecBinBySiteCompareIgnore", AnsiString(""));
                    sSpecBinBySiteComparePercent      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10987 SpecBinBySiteComparePercent",  AnsiString(""));
                    sSpecBinByArmPerSiteCompareEnable [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10988 SpecBinByArmPerSiteCompareEnable", AnsiString(""));
                    sSpecBinByArmPerSiteCompareIgnore [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10989 SpecBinByArmPerSiteCompareIgnore", AnsiString(""));
                    sSpecBinByArmPerSiteComparePercent[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "10990 SpecBinByArmPerSiteComparePercent",  AnsiString(""));
                    //<==
                    //JerryYang 20170712 (Steven) by site by bin compare percent

                    sBinTrayLinked  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(10998)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                    sBinLinked      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(10999)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                    sMagazineSetup  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(11000)", AnsiString("0"));

                    sAOIBinTraySetting[tag]->CommaText=CheckAndReadIniData(szDir, GroupName,"AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                }
            }
        }
        else if(tag==eBinFT_ART)                                                //Steven 20161123 : eBinRT_ART --> eBinFT_ART
        {
            GroupName="Bin Func ART_FT";                                        //wei 20161124 修改eBinFT_ART存成Bin Func ART_RT問題
            if(CheckSectionExist(szDir, GroupName))
            {
                iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat",77);//kevin 2016028 使用新的bin format
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 2016028 add 使用新的bin format
                {
                    if(iNewBinFormat!=77)                                       //kevin 20160928
                        bHasNewSetupData=true;
                }
                else
                {
                    bHasNewSetupData=true;
                }

                if(bHasNewSetupData)                                            //kevin 20161108
                {
                    sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3801 BinTraySetting", AnsiString(""));
                    sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3802 BinDoubleContac", AnsiString(""));
                    sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3803 BinType", AnsiString(""));
                    sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3804 BinConsFail", AnsiString(""));
                    sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11041 BinEnableFail", AnsiString(""));
                    sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11042 BinFailPercent", AnsiString(""));
                    sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11043 BinFailIgnore", AnsiString(""));
                    sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11044 BinCountEnable", AnsiString(""));
                    sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11045 BinCountNumber", AnsiString(""));
                    sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11046 BinCountIgnore", AnsiString(""));
                    sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11047 SpecialBinByArm", AnsiString(""));
                    sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11048 SpecialBinCountByArm", AnsiString(""));
                    sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11049 SpecialBinBySocket", AnsiString(""));
                    sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11050 SpecialBinCountBySocket", AnsiString(""));
                    sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11080 LowYield", AnsiString(""));
                    sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11081 ArmYield", AnsiString(""));
                    sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11082 SiteYield", AnsiString(""));
                    sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11083 BySiteClean", AnsiString(""));
                    sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11084 ByBinClean", AnsiString(""));
                    //JerryYang 20170712 (Steven) by site by bin compare percent
                    //==>
                    sSpecBinBySiteCompareEnable       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11085 SpecBinBySiteCompareEnable", AnsiString(""));
                    sSpecBinBySiteCompareIgnore       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11086 SpecBinBySiteCompareIgnore", AnsiString(""));
                    sSpecBinBySiteComparePercent      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11087 SpecBinBySiteComparePercent",  AnsiString(""));
                    sSpecBinByArmPerSiteCompareEnable [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11088 SpecBinByArmPerSiteCompareEnable", AnsiString(""));
                    sSpecBinByArmPerSiteCompareIgnore [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11089 SpecBinByArmPerSiteCompareIgnore", AnsiString(""));
                    sSpecBinByArmPerSiteComparePercent[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11090 SpecBinByArmPerSiteComparePercent",  AnsiString(""));
                    //<==
                    //JerryYang 20170712 (Steven) by site by bin compare percent

                    sBinTrayLinked  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(11098)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                    sBinLinked      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(11099)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                    sMagazineSetup  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(11100)", AnsiString("0"));

                    sAOIBinTraySetting[tag]->CommaText=CheckAndReadIniData(szDir, GroupName,"AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                }
            }
            else
            {
                GroupName="Bin Func ART_RT";
                if(CheckSectionExist(szDir, GroupName))
                {
                    iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat",77);//kevin 2016028 使用新的bin format
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                         //kevin 2016028 add 使用新的bin format
                    {
                        if(iNewBinFormat!=77)                                   //kevin 20160928
                            bHasNewSetupData=true;
                    }
                    else
                    {
                        bHasNewSetupData=true;
                    }

                    if(bHasNewSetupData)                                        //kevin 20161108
                    {
                        sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3901 BinTraySetting", AnsiString(""));
                        sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3902 BinDoubleContac", AnsiString(""));
                        sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3903 BinType", AnsiString(""));
                        sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3904 BinConsFail", AnsiString(""));
                        sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11141 BinEnableFail", AnsiString(""));
                        sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11142 BinFailPercent", AnsiString(""));
                        sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11143 BinFailIgnore", AnsiString(""));
                        sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11144 BinCountEnable", AnsiString(""));
                        sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11145 BinCountNumber", AnsiString(""));
                        sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11146 BinCountIgnore", AnsiString(""));
                        sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11147 SpecialBinByArm", AnsiString(""));
                        sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11148 SpecialBinCountByArm", AnsiString(""));
                        sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11149 SpecialBinBySocket", AnsiString(""));
                        sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11150 SpecialBinCountBySocket", AnsiString(""));
                        sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11180 LowYield", AnsiString(""));
                        sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11181 ArmYield", AnsiString(""));
                        sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11182 SiteYield", AnsiString(""));
                        sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11183 BySiteClean", AnsiString(""));     //Ifor 20170316 (wei) BinRT_ART BySiteClean 11083 --> 11183
                        sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11184 ByBinClean", AnsiString(""));      //Ifor 20170316 (wei) BinRT_ART ByBinClean  11084 --> 11184
                        //JerryYang 20170712 (Steven) by site by bin compare percent
                        //==>
                        sSpecBinBySiteCompareEnable       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11185 SpecBinBySiteCompareEnable", AnsiString(""));
                        sSpecBinBySiteCompareIgnore       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11186 SpecBinBySiteCompareIgnore", AnsiString(""));
                        sSpecBinBySiteComparePercent      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11187 SpecBinBySiteComparePercent",  AnsiString(""));
                        sSpecBinByArmPerSiteCompareEnable [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11188 SpecBinByArmPerSiteCompareEnable", AnsiString(""));
                        sSpecBinByArmPerSiteCompareIgnore [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11189 SpecBinByArmPerSiteCompareIgnore", AnsiString(""));
                        sSpecBinByArmPerSiteComparePercent[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11190 SpecBinByArmPerSiteComparePercent",  AnsiString(""));
                        //<==
                        //JerryYang 20170712 (Steven) by site by bin compare percent

                        sBinTrayLinked  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(11198)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                        sBinLinked      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(11199)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                        sMagazineSetup  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(11200)", AnsiString("0"));

                        sAOIBinTraySetting[tag]->CommaText=CheckAndReadIniData(szDir, GroupName,"AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                    }
                }
            }
        }
        else if(tag==eBinRT_ART)                                                //Steven 20161123 : eBinFT_ART --> eBinRT_ART
        {
            GroupName="Bin Func ART_RT";                                        //wei 20161124 修改eBinRT_ART存成Bin Func ART_FT問題
            if(CheckSectionExist(szDir, GroupName))
            {
                iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat", 77);//kevin 2016028 使用新的bin format
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 2016028 add 使用新的bin format
                {
                    if(iNewBinFormat!=77)                                       //kevin 20160928
                        bHasNewSetupData=true;
                }
                else
                {
                    bHasNewSetupData=true;
                }

                if(bHasNewSetupData)                                            //kevin 20161108
                {
                    sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3901 BinTraySetting", AnsiString(""));
                    sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3902 BinDoubleContac", AnsiString(""));
                    sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3903 BinType", AnsiString(""));
                    sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3904 BinConsFail", AnsiString(""));
                    sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11141 BinEnableFail", AnsiString(""));
                    sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11142 BinFailPercent", AnsiString(""));
                    sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11143 BinFailIgnore", AnsiString(""));
                    sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11144 BinCountEnable", AnsiString(""));
                    sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11145 BinCountNumber", AnsiString(""));
                    sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11146 BinCountIgnore", AnsiString(""));
                    sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11147 SpecialBinByArm", AnsiString(""));
                    sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11148 SpecialBinCountByArm", AnsiString(""));
                    sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11149 SpecialBinBySocket", AnsiString(""));
                    sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11150 SpecialBinCountBySocket", AnsiString(""));
                    sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11180 LowYield", AnsiString(""));
                    sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11181 ArmYield", AnsiString(""));
                    sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11182 SiteYield", AnsiString(""));
                    sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11183 BySiteClean", AnsiString(""));    //wei 20161124 11083 --> 11183
                    sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11184 ByBinClean", AnsiString(""));     //wei 20161124 11084 --> 11184
                    //JerryYang 20170712 (Steven) by site by bin compare percent
                    //==>
                    sSpecBinBySiteCompareEnable       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11185 SpecBinBySiteCompareEnable", AnsiString(""));
                    sSpecBinBySiteCompareIgnore       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11186 SpecBinBySiteCompareIgnore", AnsiString(""));
                    sSpecBinBySiteComparePercent      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11187 SpecBinBySiteComparePercent",  AnsiString(""));
                    sSpecBinByArmPerSiteCompareEnable [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11188 SpecBinByArmPerSiteCompareEnable", AnsiString(""));
                    sSpecBinByArmPerSiteCompareIgnore [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11189 SpecBinByArmPerSiteCompareIgnore", AnsiString(""));
                    sSpecBinByArmPerSiteComparePercent[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11190 SpecBinByArmPerSiteComparePercent",  AnsiString(""));
                    //<==
                    //JerryYang 20170712 (Steven) by site by bin compare percent

                    sBinTrayLinked  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(11198)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                    sBinLinked      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(11199)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                    sMagazineSetup  [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(11200)", AnsiString("0"));

                    sAOIBinTraySetting[tag]->CommaText=CheckAndReadIniData(szDir, GroupName,"AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                }
            }
            else
            {
                GroupName="Bin Func ART_FT";
                if(CheckSectionExist(szDir, GroupName))
                {
                    iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat",77);//kevin 2016028 使用新的bin format
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                         //kevin 2016028 add 使用新的bin format
                    {
                        if(iNewBinFormat!=77)                                   //kevin 20160928
                            bHasNewSetupData=true;
                    }
                    else
                    {
                        bHasNewSetupData=true;
                    }

                    if(bHasNewSetupData)                                        //kevin 20161108
                    {
                        sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3801 BinTraySetting", AnsiString(""));
                        sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3802 BinDoubleContac", AnsiString(""));
                        sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3803 BinType", AnsiString(""));
                        sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "3804 BinConsFail", AnsiString(""));
                        sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11041 BinEnableFail", AnsiString(""));
                        sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11042 BinFailPercent", AnsiString(""));
                        sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11043 BinFailIgnore", AnsiString(""));
                        sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11044 BinCountEnable", AnsiString(""));
                        sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11045 BinCountNumber", AnsiString(""));
                        sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11046 BinCountIgnore", AnsiString(""));
                        sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11047 SpecialBinByArm", AnsiString(""));
                        sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11048 SpecialBinCountByArm", AnsiString(""));
                        sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11049 SpecialBinBySocket", AnsiString(""));
                        sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11050 SpecialBinCountBySocket", AnsiString(""));
                        sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11080 LowYield", AnsiString(""));
                        sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11081 ArmYield", AnsiString(""));
                        sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11082 SiteYield", AnsiString(""));
                        sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11083 BySiteClean", AnsiString(""));
                        sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11084 ByBinClean", AnsiString(""));

                        sBinTrayLinked          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(11098)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                        sBinLinked              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(11099)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                        sMagazineSetup          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(11100)", AnsiString("0"));

                        sAOIBinTraySetting      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                    }
                }
            }
        }
        else if(tag==eBinFT_MRT)                                                //Steven 20161123 : eBinRT_ART --> eBinFT_ART
        {                                                                       //Ifor 20170316 (wei) add KYEC MRT Mode
            GroupName="Bin Func MRT_FT";                                        //wei 20161124 修改eBinFT_ART存成Bin Func ART_RT問題
            if(CheckSectionExist(szDir, GroupName))
            {
                iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat", 77);//kevin 2016028 使用新的bin format
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 2016028 add 使用新的bin format
                {
                    if(iNewBinFormat!=77)                                       //kevin 20160928
                        bHasNewSetupData=true;
                }
                else
                {
                    bHasNewSetupData=true;
                }

                if(bHasNewSetupData)                                            //kevin 20161108
                {
                    sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "4001 BinTraySetting", AnsiString(""));
                    sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "4002 BinDoubleContac", AnsiString(""));
                    sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "4003 BinType", AnsiString(""));
                    sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "4004 BinConsFail", AnsiString(""));
                    sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11241 BinEnableFail", AnsiString(""));
                    sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11242 BinFailPercent", AnsiString(""));
                    sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11243 BinFailIgnore", AnsiString(""));
                    sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11244 BinCountEnable", AnsiString(""));
                    sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11245 BinCountNumber", AnsiString(""));
                    sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11246 BinCountIgnore", AnsiString(""));
                    sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11247 SpecialBinByArm", AnsiString(""));
                    sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11248 SpecialBinCountByArm", AnsiString(""));
                    sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11249 SpecialBinBySocket", AnsiString(""));
                    sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11250 SpecialBinCountBySocket", AnsiString(""));
                    sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11280 LowYield", AnsiString(""));
                    sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11281 ArmYield", AnsiString(""));
                    sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11282 SiteYield", AnsiString(""));
                    sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11283 BySiteClean", AnsiString(""));
                    sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11284 ByBinClean", AnsiString(""));

                    sBinTrayLinked          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(11298)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                    sBinLinked              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(11299)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                    sMagazineSetup          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(11300)", AnsiString("0"));

                    // (B14) GOLDEN BUG (golden cBinSel.cpp :5301): every other tag's
                    // AOIBinTraySetting key is the bare literal "AOIBinTraySetting" -- only
                    // FT_MRT concatenates GroupName onto it ("Bin Func MRT_FTAOIBinTraySetting"),
                    // so this tag's AOI-bin setting silently reads/writes a DIFFERENT ini key
                    // than every sibling tag. Preserved verbatim (not "fixed" -- see project
                    // translation policy).
                    sAOIBinTraySetting      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, GroupName + "AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                }
            }
        }
        else if(tag==eBinRT_MRT)                                                //Steven 20161123 : eBinFT_ART --> eBinRT_ART
        {
            GroupName="Bin Func MRT_RT";                                        //wei 20161124 修改eBinRT_ART存成Bin Func ART_FT問題
            if(CheckSectionExist(szDir, GroupName))
            {
                iNewBinFormat=CheckAndReadIniData(szDir, GroupName, "NewBinFormat",77);//kevin 2016028 使用新的bin format
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 2016028 add 使用新的bin format
                {
                    if(iNewBinFormat!=77)                                       //kevin 20160928
                        bHasNewSetupData=true;
                }
                else
                {
                    bHasNewSetupData=true;
                }

                if(bHasNewSetupData)                                            //kevin 20161108
                {
                    sBinTraySetT3Pos        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "4101 BinTraySetting", AnsiString(""));
                    sBinDoubleContact       [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "4102 BinDoubleContac", AnsiString(""));
                    sBinType                [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "4103 BinType", AnsiString(""));
                    sBinConsFail            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "4104 BinConsFail", AnsiString(""));
                    sBinEnableFail          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11341 BinEnableFail", AnsiString(""));
                    sBinFailPercent         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11342 BinFailPercent", AnsiString(""));
                    sBinFailIgnore          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11343 BinFailIgnore", AnsiString(""));
                    sBinCountEnable         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11344 BinCountEnable", AnsiString(""));
                    sBinCountNumber         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11345 BinCountNumber", AnsiString(""));
                    sBinCountIgnore         [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11346 BinCountIgnore", AnsiString(""));
                    sSpecialBinByArm        [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11347 SpecialBinByArm", AnsiString(""));
                    sSpecialBinCountByArm   [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11348 SpecialBinCountByArm", AnsiString(""));
                    sSpecialBinBySocket     [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11349 SpecialBinBySocket", AnsiString(""));
                    sSpecialBinCountBySocket[tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11350 SpecialBinCountBySocket", AnsiString(""));
                    sLowYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11380 LowYield", AnsiString(""));
                    sArmYield               [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11381 ArmYield", AnsiString(""));
                    sSiteYield              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11382 SiteYield", AnsiString(""));
                    sBySiteClean            [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11383 BySiteClean", AnsiString(""));
                    sByBinClean             [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "11384 ByBinClean", AnsiString(""));

                    sBinTrayLinked          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinTrayLinked(11398)",  AnsiString("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));    //Ifor 20240730 add : BinTrayLinked
                    sBinLinked              [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "BinLinked(11399)",  AnsiString("0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;"));    //Ifor 20240730 add : BinLinked
                    sMagazineSetup          [tag]->CommaText=CheckAndReadIniData(szDir, GroupName, "MagazineSetup(11400)", AnsiString("0"));

                    // (B15) GOLDEN BUG (golden cBinSel.cpp :5347): same shape as (B14) above,
                    // but with a leading space ("Bin Func MRT_RT AOIBinTraySetting") -- a
                    // second, independently-inconsistent copy-paste variant, not the same key
                    // as (B14)'s FT_MRT variant either. Preserved verbatim.
                    sAOIBinTraySetting      [tag]->CommaText=CheckAndReadIniData(szDir, GroupName,GroupName + " AOIBinTraySetting", AnsiString(""));//Eastsun 20260316 : AOI 要分bin
                }
            }
        }
    }
//    QQ2=MyQQTimer.LatchCycleTime()/1000.0;
//    Memo1->Lines->Add(" ReadFunctionDataB"+AnsiString(tag)+": "+AnsiString(QQ2)+" ="+AnsiString(QQ2-QQ1));
//    QQ1=QQ2;

    if(bHasNewSetupData==false)                                                 //這裡是為了跟舊版的相容, 所以新的功能不需要加在這邊
    {
        for(int i=0; i<iTestBinCount; i++)
        {                                                                       //QQQ
            GroupStr.sprintf("Category%d", i);                                  //kevin 20140317

            //Scan-------------
            BinSelect[tag].bScanner[i]=0;
            MyBinPanel[tag]->bScan[i]=BinSelect[tag].bScanner[i];

            if(IniConfig.bA30SetupTeachFunction && tag==OffT && LastSet.bNeedSetupTeach && (i>=1 && i<=iFixRight))  //JerryYang 20180921 Setup Teach功能
            {
                BinSelect[tag].iCatDataT3Pos[i]=CheckAndReadIniData(szDir, GroupStr, "Bin", i);      //Bin 1..6 to Auto 123 Fix 123
            }
            else
            {
                BinSelect[tag].iCatDataT3Pos[i]=CheckAndReadIniData(szDir, GroupStr, "Bin", (tag==OffT)?e3PosFix2:e3PosNoUse);      //Steven 20150209 : ePosFix1 --> ePosFix2 for HT9046LS + Rotate
            }

            if(BinSelect[tag].iCatDataT3Pos[i]<e3PosNoUse)
                BinSelect[tag].iCatDataT3Pos[i]= e3PosNoUse;

            if(USE_ROTATE==eCynRot && BinSelect[tag].iCatDataT3Pos[i]-1==iRotate_Out_Tray6) //kevin 20130812  氣缸版   //Steven 20121001 : 旋轉Kit
                BinSelect[tag].iCatDataT3Pos[i]= e3PosNoUse;

            if(AUTO3_IS_MAGAZINE==1 && BinSelect[tag].iCatDataT3Pos[i]==e3PosAuto3)         //JerryYang 20221207 : Magazine not use AUTO3
                BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;

            if(CosFunction.bLoaderTrayToAuto1 && TrayForm.LoaderToEmptyColor[iRunStartMode]==2 && BinSelect[tag].iCatDataT3Pos[i]==e3PosAuto1)
                BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;

            if(AUTO3_IS_MAGAZINE==1 && TestIF_File.iMagFixTrayType==1 && BinSelect[tag].iCatDataT3Pos[i]>=e3PosFix1 && BinSelect[tag].iCatDataT3Pos[i]<=e3PosFix6)  //JerryYang 20221215 : Magazine把fix區當buffer區功能
                BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;

            if(TrayForm.iFixTrayMode==false && BinSelect[tag].iCatDataT3Pos[i]>e3PosFix3)   //Fix滿盤
                BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;

            if(CosFunction.bUseTrayUpDownSet)                                   //wei 20160224 TSMC FIX UPDOWN
            {
                if(TrayForm.iFixTrayMode && BinSelect[tag].iCatDataT3Pos[i]==e3PosFix4 && TrayForm.bTrayUpDownSet[e3PosFix4-e3PosFix4]==false)
                    BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;
                else if(TrayForm.iFixTrayMode && BinSelect[tag].iCatDataT3Pos[i]==e3PosFix5 && TrayForm.bTrayUpDownSet[e3PosFix5-e3PosFix4]==false)
                    BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;
                else if(TrayForm.iFixTrayMode && BinSelect[tag].iCatDataT3Pos[i]==e3PosFix6 && TrayForm.bTrayUpDownSet[e3PosFix6-e3PosFix4]==false)
                    BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;
            }

            if(TestIF_File.bEnableQASampling &&                                 //Steven 20190326 : QA Sampling
               BinSelect[tag].iCatDataT3Pos[i]==TestIF_File.iQASamplingT3Pos)
            {
                BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;
            }

            MyBinPanel[tag]->BackT6PosTray[i][eBinNotUse+BinSelect[tag].iCatDataT3Pos[i]]=1;

            //Double Contact------
            if(Prod.bD22SupportMultiDoubleContact==false)                       //Steven 20170920 : 重新整理Double Contact選項
               //IniConfig.bD22VerifyMode==false)                               //Sam 20231117 : 整合到 QA 模式      //Sam 20221012 : 新增 VerifyMode 功能
            {
                BinSelect[tag].iDBContact[i]    =0;
            }
            else
            {
                BinSelect[tag].iDBContact[i]    =CheckAndReadIniData(szDir, GroupStr, "Contact", 0);
            }
            MyBinPanel[tag]->i2Contact[i]       =BinSelect[tag].iDBContact[i];

            //Con. Fail--------
            BinSelect[tag].bConsFail[i]         =CheckAndReadIniData(szDir, GroupStr, "Cons.Fail", 0);
            MyBinPanel[tag]->bConFail[i]        =BinSelect[tag].bConsFail[i];

            //Persent Enable------                                              //Steven 20140529 Start: Fail Persent & Count同時存在
            BinSelect[tag].bFailure[i]          =CheckAndReadIniData(szDir, GroupStr, "Fail Percent", 0);
            MyBinPanel[tag]->bPersentEnable[i]  =BinSelect[tag].bFailure[i];

            //Persent Ignore------
            BinSelect[tag].iPersentIgnore[i]    =CheckAndReadIniData(szDir, GroupStr, "Fail Percent Ignore", 0);
            MyBinPanel[tag]->iPersentIgnore[i]  =BinSelect[tag].iPersentIgnore[i];

            //Persent Number------
            BinSelect[tag].dFailureLimit[i]     =CheckAndReadIniData(szDir, GroupStr, "Fail Percent Limit", 0.0);
            MyBinPanel[tag]->dPersentNumber[i]  =BinSelect[tag].dFailureLimit[i];

            //Count Enable------
            BinSelect[tag].bFailCountEnable[i]  =CheckAndReadIniData(szDir, GroupStr, "Fail Count", 0);
            MyBinPanel[tag]->bCountEnable[i]    =BinSelect[tag].bFailCountEnable[i];

            //Count Ignore------
            BinSelect[tag].iFailCountIgnore[i]  =CheckAndReadIniData(szDir, GroupStr, "Fail Count Ignore", 0);
            MyBinPanel[tag]->iCountIgnore[i]    =BinSelect[tag].iFailCountIgnore[i];

            //Count Number------
            BinSelect[tag].iFailCountLimit[i]   =CheckAndReadIniData(szDir, GroupStr, "Fail Count Limit", 0.0);
            MyBinPanel[tag]->iCountNumber[i]    =BinSelect[tag].iFailCountLimit[i];

            BinSelect[tag].bSpecialBinByArm[i]          =CheckAndReadIniData(szDir, GroupStr, "Special Bin By Arm", false);
            MyBinPanel[tag]->bSpecialBinByArm[i]        =BinSelect[tag].bSpecialBinByArm[i];

            BinSelect[tag].iSpecialBinCountByArm[i]     =CheckAndReadIniData(szDir, GroupStr, "Special Bin Count By Arm", 0);
            MyBinPanel[tag]->iSpecialBinCountByArm[i]   =BinSelect[tag].iSpecialBinCountByArm[i];

            BinSelect[tag].bSpecialBinBySocket[i]       =CheckAndReadIniData(szDir, GroupStr, "Special Bin By Socket", false);
            MyBinPanel[tag]->bSpecialBinBySocket[i]     =BinSelect[tag].bSpecialBinBySocket[i];

            BinSelect[tag].iSpecialBinCountBySocket[i]  =CheckAndReadIniData(szDir, GroupStr, "Special Bin Count By Socket", 0.0);
            MyBinPanel[tag]->iSpecialBinCountBySocket[i]=BinSelect[tag].iSpecialBinCountBySocket[i];

            BinSelect[tag].bLowYield[i]     =CheckAndReadIniData(szDir, GroupStr, "By Bin Low Yield", false);                    //Steven 20140828 : By Bin Yield Monitor
            MyBinPanel[tag]->bLowYield[i]   =BinSelect[tag].bLowYield[i];

            BinSelect[tag].bArmYield[i]     =CheckAndReadIniData(szDir, GroupStr, "By Bin and Site Compare Arm Yield", false);   //Steven 20140828 : By Bin Arm Yield Monitor
            MyBinPanel[tag]->bArmYield[i]   =BinSelect[tag].bArmYield[i];

            BinSelect[tag].bSiteYield[i]    =CheckAndReadIniData(szDir, GroupStr, "By Bin Compare Site Yield", false);           //Steven 20140828 : By Bin Site Yield Monitor
            MyBinPanel[tag]->bSiteYield[i]  =BinSelect[tag].bSiteYield[i];
        }

        for(int i=0; i<iTestBinCount; i++)
        {
            if(i<sBinDoubleContact[tag]->Count)
                sBinDoubleContact       [tag]->Strings[i]=BinSelect[tag].iDBContact[i];
            if(i<sBinConsFail[tag]->Count)
                sBinConsFail            [tag]->Strings[i]=(BinSelect[tag].bConsFail[i])?"1":"0";
            if(i<sBinEnableFail[tag]->Count)
                sBinEnableFail          [tag]->Strings[i]=(BinSelect[tag].bFailure[i]  )?"1":"0";
            if(i<sBinFailPercent[tag]->Count)
                sBinFailPercent         [tag]->Strings[i]=AnsiString(BinSelect[tag].dFailureLimit[i]);
            if(i<sBinFailIgnore[tag]->Count)
                sBinFailIgnore          [tag]->Strings[i]=AnsiString(BinSelect[tag].iPersentIgnore[i]);
            if(i<sBinCountEnable[tag]->Count)
                sBinCountEnable         [tag]->Strings[i]=(BinSelect[tag].bFailCountEnable[i])?"1":"0";
            if(i<sBinCountIgnore[tag]->Count)
                sBinCountIgnore         [tag]->Strings[i]=AnsiString(BinSelect[tag].iFailCountIgnore[i]);
            if(i<sBinCountNumber[tag]->Count)
                sBinCountNumber         [tag]->Strings[i]=AnsiString(BinSelect[tag].iFailCountLimit[i]);
            if(i<sSpecialBinByArm[tag]->Count)
                sSpecialBinByArm        [tag]->Strings[i]=(BinSelect[tag].bSpecialBinByArm[i]?"1":"0");
            if(i<sSpecialBinCountByArm[tag]->Count)
                sSpecialBinCountByArm   [tag]->Strings[i]=AnsiString(BinSelect[tag].iSpecialBinCountByArm[i]);
            if(i<sSpecialBinBySocket[tag]->Count)
                sSpecialBinBySocket     [tag]->Strings[i]=(BinSelect[tag].bSpecialBinBySocket[i]?"1":"0");
            if(i<sSpecialBinCountBySocket[tag]->Count)
                sSpecialBinCountBySocket[tag]->Strings[i]=AnsiString(BinSelect[tag].iSpecialBinCountBySocket[i]);
            if(i<sLowYield[tag]->Count)                                         //Steven 20140828 : By Bin Yield Monitor
                sLowYield               [tag]->Strings[i]=(BinSelect[tag].bLowYield[i]?"1":"0");
            if(i<sArmYield[tag]->Count)                                         //Steven 20140828 : By Bin Arm Yield Monitor
                sArmYield               [tag]->Strings[i]=(BinSelect[tag].bArmYield[i]?"1":"0");
            if(i<sSiteYield[tag]->Count)                                        //Steven 20140828 : By Bin Site Yield Monitor
                sSiteYield              [tag]->Strings[i]=(BinSelect[tag].bSiteYield[i]?"1":"0");
            if(i<sBinTraySetT3Pos[tag]->Count)
                sBinTraySetT3Pos        [tag]->Strings[i]=AnsiString(BinSelect[tag].iCatDataT3Pos[i]);
        }
        SaveFunctionData(tag, szDir);
    }
    else
    {
        for(int i=0; i<iTestBinCount; i++)
        {
            if(IniConfig.bD22SupportMultiDoubleContact==false &&                //Steven 20170920 : 重新整理Double Contact選項
               IniConfig.bD22VerifyMode==false)                                 //Sam 20221012 : 新增 VerifyMode 功能
            {
                sBinDoubleContact[tag]->Clear();
            }

            BinSelect[tag].iDBContact[i]                =(i<sBinDoubleContact[tag]->Count       )?atoi(AnsiString(sBinDoubleContact        [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].bConsFail[i]                 =(i<sBinConsFail[tag]->Count            )?atoi(AnsiString(sBinConsFail             [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].bFailure[i]                  =(i<sBinEnableFail[tag]->Count          )?atoi(AnsiString(sBinEnableFail           [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].dFailureLimit[i]             =(i<sBinFailPercent[tag]->Count         )?atof(AnsiString(sBinFailPercent          [tag]->Strings[i]).c_str()):0.0;         //JerryYang 20160613 修正By Bin Yield要能設定到小數點
            BinSelect[tag].iPersentIgnore[i]            =(i<sBinFailIgnore[tag]->Count          )?atoi(AnsiString(sBinFailIgnore           [tag]->Strings[i]).c_str()):0;
            BinSelect[tag].bFailCountEnable[i]          =(i<sBinCountEnable[tag]->Count         )?atoi(AnsiString(sBinCountEnable          [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].iFailCountIgnore[i]          =(i<sBinCountIgnore[tag]->Count         )?atoi(AnsiString(sBinCountIgnore          [tag]->Strings[i]).c_str()):0;
            BinSelect[tag].iFailCountLimit[i]           =(i<sBinCountNumber[tag]->Count         )?atoi(AnsiString(sBinCountNumber          [tag]->Strings[i]).c_str()):0;
            BinSelect[tag].bSpecialBinByArm[i]          =(i<sSpecialBinByArm[tag]->Count        )?atoi(AnsiString(sSpecialBinByArm         [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].iSpecialBinCountByArm[i]     =(i<sSpecialBinCountByArm[tag]->Count   )?atoi(AnsiString(sSpecialBinCountByArm    [tag]->Strings[i]).c_str()):0;
            BinSelect[tag].bSpecialBinBySocket[i]       =(i<sSpecialBinBySocket[tag]->Count     )?atoi(AnsiString(sSpecialBinBySocket      [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].iSpecialBinCountBySocket[i]  =(i<sSpecialBinCountBySocket[tag]->Count)?atoi(AnsiString(sSpecialBinCountBySocket [tag]->Strings[i]).c_str()):0;
            BinSelect[tag].bLowYield[i]                 =(i<sLowYield[tag]->Count               )?atoi(AnsiString(sLowYield                [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].bArmYield[i]                 =(i<sArmYield[tag]->Count               )?atoi(AnsiString(sArmYield                [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].bSiteYield[i]                =(i<sSiteYield[tag]->Count              )?atoi(AnsiString(sSiteYield               [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].iAutoCleanByBin[i]           =(i<sBySiteClean[tag]->Count            )?atoi(AnsiString(sBySiteClean             [tag]->Strings[i]).c_str()):0;
            BinSelect[tag].iAutoCleanBySite[i]          =(i<sByBinClean[tag]->Count             )?atoi(AnsiString(sByBinClean              [tag]->Strings[i]).c_str()):0;
            BinSelect[tag].iCatDataT3Pos[i]             =(i<sBinTraySetT3Pos[tag]->Count        )?atoi(AnsiString(sBinTraySetT3Pos         [tag]->Strings[i]).c_str()):e3PosNoUse;
            BinSelect[tag].bBinTrayLinked[i]            =(i<sBinTrayLinked[tag]->Count          )?atoi(AnsiString(sBinTrayLinked           [tag]->Strings[i]).c_str()):false;
            BinSelect[tag].iAOICategData[i]             =(i<sAOIBinTraySetting[tag]->Count      )?atoi(AnsiString(sAOIBinTraySetting       [tag]->Strings[i]).c_str()):ePosNoUse;   //Eastsun 20260316 : AOI 分BIN

            if(CosFunction.bBySiteByBinPercentCompare==true &&                  //JerryYang 20170712 (Steven) by site by bin compare percent
               CosFunction.bByBinAlarmFromYieldForm==false)                     //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
            {
                BinSelect[tag].bSpecBinBySiteCompareEnable[i]       =(i<sSpecBinBySiteCompareEnable[tag]->Count       )?atoi(AnsiString(sSpecBinBySiteCompareEnable[tag]->Strings[i]).c_str()):false;
                BinSelect[tag].iSpecBinBySiteCompareIgnore[i]       =(i<sSpecBinBySiteCompareIgnore[tag]->Count       )?atoi(AnsiString(sSpecBinBySiteCompareIgnore[tag]->Strings[i]).c_str()):0;
                BinSelect[tag].dSpecBinBySiteComparePercent[i]      =(i<sSpecBinBySiteComparePercent[tag]->Count      )?atof(AnsiString(sSpecBinBySiteComparePercent[tag]->Strings[i]).c_str()):0.0;
                BinSelect[tag].bSpecBinByArmPerSiteCompareEnable[i] =(i<sSpecBinByArmPerSiteCompareEnable[tag]->Count )?atoi(AnsiString(sSpecBinByArmPerSiteCompareEnable[tag]->Strings[i]).c_str()):false;
                BinSelect[tag].iSpecBinByArmPerSiteCompareIgnore[i] =(i<sSpecBinByArmPerSiteCompareIgnore[tag]->Count )?atoi(AnsiString(sSpecBinByArmPerSiteCompareIgnore[tag]->Strings[i]).c_str()):0;
                BinSelect[tag].dSpecBinByArmPerSiteComparePercent[i]=(i<sSpecBinByArmPerSiteComparePercent[tag]->Count)?atof(AnsiString(sSpecBinByArmPerSiteComparePercent[tag]->Strings[i]).c_str()):0.0;
            }

            if(i<sBinTraySetT3Pos[tag]->Count)
            {
                iData=atoi(AnsiString(sBinTraySetT3Pos[tag]->Strings[i]).c_str());

                if(iData<e3PosNoUse)
                    BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;

                iT3=BinSelect[tag].iCatDataT3Pos[i]-1;
                if(iT3>=e3Auto1)
                {
                    iT6=iTo6Unload[iT3];
                    if(Prod.iTrayType[iT6]==tNotUse)
                        iData=e3PosNoUse;

                    if(CosFunction.bLoaderTrayToAuto1 && TrayForm.LoaderToEmptyColor[iRunStartMode]==2)
                    {
                        if(BinSelect[tag].iCatDataT3Pos[i]==e3PosAuto1)
                            iData=e3PosNoUse;
                    }

                    if(AUTO3_IS_MAGAZINE==1)
                    {
                        if(BinSelect[tag].iCatDataT3Pos[i]==e3PosAuto3)         //JerryYang 20221207 : Magazine not use AUTO3
                            iData=e3PosNoUse;

                        if(TestIF_File.iMagFixTrayType==1 && iT6>=iFixMin && iT6<=iFixRightHalf)
                            iData=e3PosNoUse;
                    }
                }
                else
                {
                    iData=e3PosNoUse;
                }

                BinSelect[tag].iCatDataT3Pos[i]=iData;
                sBinTraySetT3Pos[tag]->Strings[i]=AnsiString(iData);
            }
            else
            {
                BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;
            }
        }

        for(int i=0; i<iTestBinCount; i++)
        {
            if(TestIF_File.bEnableQASampling &&                                 //Steven 20190326 : QA Sampling
               BinSelect[tag].iCatDataT3Pos[i]==TestIF_File.iQASamplingT3Pos)
            {
                BinSelect[tag].iCatDataT3Pos[i]=e3PosNoUse;
            }

            int iUnloader=0;
            iUnloader=iTo6PosUnload[BinSelect[tag].iCatDataT3Pos[i]];           //JerryYang 20230915 : test     //JerryYang 20230926

            MyBinPanel[tag]->BackT6PosTray[i][eBinNotUse+iUnloader]=1;
            MyBinPanel[tag]->i2Contact[i]               =BinSelect[tag].iDBContact[i];
            MyBinPanel[tag]->bConFail[i]                =BinSelect[tag].bConsFail[i];
            MyBinPanel[tag]->bPersentEnable[i]          =BinSelect[tag].bFailure[i];
            MyBinPanel[tag]->iPersentIgnore[i]          =BinSelect[tag].iPersentIgnore[i];
            MyBinPanel[tag]->dPersentNumber[i]          =BinSelect[tag].dFailureLimit[i];
            MyBinPanel[tag]->bCountEnable[i]            =BinSelect[tag].bFailCountEnable[i];
            MyBinPanel[tag]->iCountIgnore[i]            =BinSelect[tag].iFailCountIgnore[i];
            MyBinPanel[tag]->iCountNumber[i]            =BinSelect[tag].iFailCountLimit[i];
            MyBinPanel[tag]->bSpecialBinByArm[i]        =BinSelect[tag].bSpecialBinByArm[i];
            MyBinPanel[tag]->iSpecialBinCountByArm[i]   =BinSelect[tag].iSpecialBinCountByArm[i];
            MyBinPanel[tag]->bSpecialBinBySocket[i]     =BinSelect[tag].bSpecialBinBySocket[i];
            MyBinPanel[tag]->iSpecialBinCountBySocket[i]=BinSelect[tag].iSpecialBinCountBySocket[i];
            MyBinPanel[tag]->bLowYield[i]               =BinSelect[tag].bLowYield[i];
            MyBinPanel[tag]->bArmYield[i]               =BinSelect[tag].bArmYield[i];
            MyBinPanel[tag]->bSiteYield[i]              =BinSelect[tag].bSiteYield[i];
            MyBinPanel[tag]->iAutoCleanByBin[i]         =BinSelect[tag].iAutoCleanByBin[i];             //Steven 20160308 : By Bin count do auto clean
            MyBinPanel[tag]->iAutoCleanBySite[i]        =BinSelect[tag].iAutoCleanBySite[i];            //Steven 20160308 : By Bin pre site count do auto clean

            if(CosFunction.bBySiteByBinPercentCompare==true &&                  //JerryYang 20170712 (Steven) by site by bin compare percent
               CosFunction.bByBinAlarmFromYieldForm==false)                     //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
            {
                MyBinPanel[tag]->bSpecBinBySiteCompareEnable[i]       =BinSelect[tag].bSpecBinBySiteCompareEnable[i];
                MyBinPanel[tag]->iSpecBinBySiteCompareIgnore[i]       =BinSelect[tag].iSpecBinBySiteCompareIgnore[i];
                MyBinPanel[tag]->dSpecBinBySiteComparePercent[i]      =BinSelect[tag].dSpecBinBySiteComparePercent[i];
                MyBinPanel[tag]->bSpecBinByArmPerSiteCompareEnable[i] =BinSelect[tag].bSpecBinByArmPerSiteCompareEnable[i];
                MyBinPanel[tag]->iSpecBinByArmPerSiteCompareIgnore[i] =BinSelect[tag].iSpecBinByArmPerSiteCompareIgnore[i];
                MyBinPanel[tag]->dSpecBinByArmPerSiteComparePercent[i]=BinSelect[tag].dSpecBinByArmPerSiteComparePercent[i];
            }
        }

        // AI(W906-FW-BinSel-WB) 20260819: GATE -- MyBinPanel[tag]->ed* (TEdit*,
        // golden TMyBinPanel widget half, cBinSel.h public block) has no
        // TMyBinPanelData counterpart (`grep -n "edBinSetT3Pos\|edBinSettingConFail\|edSpecBinByArmPerSiteComparePercent\|edBinSetT6Link" forms/fBinSel.h`
        // -- 0 hits, 20260819; TMyBinPanelData is the data-only stand-in declared
        // by WAVE A's CTOR NOTE, no `TEdit *ed*` members). These 22 assignments
        // are pure widget-Text mirrors of the sXxx[tag]->CommaText this method
        // already populated above (no further state derives from them anywhere
        // in this wave's ACTIVE call graph) -- gated as a block rather than
        // fabricating 22 facade members with no reader.
#if 0
        MyBinPanel[tag]->edBinSetT3Pos->Text                =sBinTraySetT3Pos[tag]->CommaText;
        MyBinPanel[tag]->edBinSetting2Contact->Text         =sBinDoubleContact[tag]->CommaText;
        MyBinPanel[tag]->edBinSettingConFail->Text          =sBinConsFail[tag]->CommaText;
        MyBinPanel[tag]->edBinSettingFailPersentEnable->Text=sBinEnableFail[tag]->CommaText;
        MyBinPanel[tag]->edBinSettingFailPersentIgnore->Text=sBinFailPercent[tag]->CommaText;
        MyBinPanel[tag]->edBinSettingFailPersentNumber->Text=sBinFailIgnore[tag]->CommaText;
        MyBinPanel[tag]->edBinSettingFailCountEnable->Text  =sBinCountEnable[tag]->CommaText;
        MyBinPanel[tag]->edBinSettingFailCountIgnore->Text  =sBinCountIgnore[tag]->CommaText;
        MyBinPanel[tag]->edBinSettingFailCountNumber->Text  =sBinCountNumber[tag]->CommaText;
        MyBinPanel[tag]->edSpecialBinByArm->Text            =sSpecialBinByArm[tag]->CommaText;
        MyBinPanel[tag]->edSpecialBinCountByArm->Text       =sSpecialBinCountByArm[tag]->CommaText;
        MyBinPanel[tag]->edSpecialBinBySocket->Text         =sSpecialBinBySocket[tag]->CommaText;
        MyBinPanel[tag]->edSpecialBinCountBySocket->Text    =sSpecialBinCountBySocket[tag]->CommaText;
        MyBinPanel[tag]->edLowYield->Text                   =sLowYield[tag]->CommaText;
        MyBinPanel[tag]->edArmYield->Text                   =sArmYield[tag]->CommaText;
        MyBinPanel[tag]->edSiteYield->Text                  =sSiteYield[tag]->CommaText;
        MyBinPanel[tag]->edBySiteClean->Text                =sBySiteClean[tag]->CommaText;
        MyBinPanel[tag]->edByBinClean->Text                 =sByBinClean[tag]->CommaText;

        MyBinPanel[tag]->edSpecBinBySiteCompareEnable->Text         =sSpecBinBySiteCompareEnable[tag]->CommaText;
        MyBinPanel[tag]->edSpecBinBySiteCompareIgnore->Text         =sSpecBinBySiteCompareIgnore[tag]->CommaText;
        MyBinPanel[tag]->edSpecBinBySiteComparePercent->Text        =sSpecBinBySiteComparePercent[tag]->CommaText;
        MyBinPanel[tag]->edSpecBinByArmPerSiteCompareEnable->Text   =sSpecBinByArmPerSiteCompareEnable[tag]->CommaText;
        MyBinPanel[tag]->edSpecBinByArmPerSiteCompareIgnore->Text   =sSpecBinByArmPerSiteCompareIgnore[tag]->CommaText;
        MyBinPanel[tag]->edSpecBinByArmPerSiteComparePercent->Text  =sSpecBinByArmPerSiteComparePercent[tag]->CommaText;
        MyBinPanel[tag]->edBinSetT6Link->Text                       =sBinTrayLinked[tag]->CommaText;
#endif

        SaveFunctionData(tag, szDir);                                                  //wei 20161124 修改存檔錯誤的問題
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  SaveFunctionData -- golden :5668-6009. FW-BinSel-WB (this wave). GATE:
//  100% of this method's real effect is `FormSysTools->WriteIniData(...)` --
//  every site becomes `WriteIniData(FileName,...)` verbatim and is `#if 0`-
//  gated per WRITE-PATH GATE TABLE (project policy: writes to shared
//  recipe-config are gated regardless of transcription effort). ACTIVE: the
//  `TransferBinTrayStrToName(tag)` calls interleaved between gated write runs
//  (golden :5679/:5722/:5766/:5809/:5852/:5895/:5927/:5970 -- already-real,
//  in-memory-only, no file I/O) and the full if/else-if tag-selection
//  structure (本體結構照翻 -- kept so a future wave can un-gate write-by-
//  write without re-deriving which GroupName/key belongs to which tag).
//  DEVIATION: signature gains an `AnsiString FileName` parameter -- same
//  `FormSysTools->OpenFormData(FileName)`-elision substitution as
//  ReadFunctionData above (golden :2542, `SaveOther` opens `FileName` right
//  before calling this); `ReadFunctionData`'s own two internal calls
//  (golden :5508/:5664) now pass its own `szDir` as this `FileName` --
//  same file, same FormSysTools object in golden, so this is the same
//  parameter under two names, not two different files.
//---------------------------------------------------------------------------
void TfBinSel::SaveFunctionData(int tag, AnsiString FileName)
{
    AnsiString GroupName;
    (void)FileName;   // only feeds the gated WriteIniData calls below in this wave
    (void)GroupName;  // ditto -- read only inside the #if 0 blocks below
    if(tag==eBinRT)
    {
        if((IniConfig.bA02BinModelPrime==true  && IniConfig.bFTBin2RTBin==true && (iBinModelPrime==0 || CosFunction.bDisableRTBinSet)) ||        //Steven 20160623 : 當IniConfig.bFTBin2RTBin==true && 讀檔時, RT Bin要跟FT Bin一樣
           (IniConfig.bA02BinModelPrime==false && (IniConfig.bFTBin2RTBin==true || CosFunction.bDisableRTBinSet)))
        {
            GroupName="Bin Func FT";
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5677-5678
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "NewBinFormat",               1);   //kevin 20160928 使用新的bin format
            WriteIniData(FileName, GroupName, "3617 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2797
            TransferBinTrayStrToName(tag);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5680-5698
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "3617 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
            WriteIniData(FileName, GroupName, "3636 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
            WriteIniData(FileName, GroupName, "3656 BinType",               sBinType                [tag]->CommaText);
            WriteIniData(FileName, GroupName, "3676 BinConsFail",           sBinConsFail            [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10641 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10642 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10643 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10644 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10646 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10645 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10647 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10648 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10649 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10650 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
            WriteIniData(FileName, GroupName, "10680 LowYield",             sLowYield               [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10681 ArmYield",             sArmYield               [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10682 SiteYield",            sSiteYield              [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10683 BySiteClean",          sBySiteClean            [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10684 ByBinClean",           sByBinClean             [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2803

            if(CosFunction.bBySiteByBinPercentCompare==true &&                  //JerryYang 20170712 (Steven) by site by bin compare percent
               CosFunction.bByBinAlarmFromYieldForm==false)                     //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
            {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5703-5708
                // GATE (write-policy)
                WriteIniData(FileName, GroupName, "10685 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10686 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10687 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10688 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10689 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10690 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2829
            }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5711-5715
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "BinTrayLinked(10698)", sBinTrayLinked        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "BinLinked(10699)",     sBinLinked            [tag]->CommaText);
            WriteIniData(FileName, GroupName, "MagazineSetup(10700)", sMagazineSetup        [tag]->CommaText);

            WriteIniData(FileName, GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
//#endif   //AI(W906-S09-B6) 20260929: see :2840
        }
        else
        {
            GroupName="Bin Func RT";
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5720-5721
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "NewBinFormat",               1);   //kevin 20160928 使用新的bin format
            WriteIniData(FileName, GroupName, "3717 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2852
            TransferBinTrayStrToName(tag);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5723-5741
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "3717 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
            WriteIniData(FileName, GroupName, "3718 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
            WriteIniData(FileName, GroupName, "3719 BinType",               sBinType                [tag]->CommaText);
            WriteIniData(FileName, GroupName, "3720 BinConsFail",           sBinConsFail            [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10741 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10742 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10743 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10744 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10745 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10746 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10747 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10748 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10749 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10750 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
            WriteIniData(FileName, GroupName, "10780 LowYield",             sLowYield               [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10781 ArmYield",             sArmYield               [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10782 SiteYield",            sSiteYield              [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10783 BySiteClean",          sBySiteClean            [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10784 ByBinClean",           sByBinClean             [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2858

            if(CosFunction.bBySiteByBinPercentCompare==true &&                  //JerryYang 20170712 (Steven) by site by bin compare percent
               CosFunction.bByBinAlarmFromYieldForm==false)                     //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
            {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5746-5751
                // GATE (write-policy)
                WriteIniData(FileName, GroupName, "10785 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10786 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10787 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10788 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10789 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
                WriteIniData(FileName, GroupName, "10790 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2884
            }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5754-5758
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "BinTrayLinked(10798)", sBinTrayLinked        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "BinLinked(10799)",     sBinLinked            [tag]->CommaText);
            WriteIniData(FileName, GroupName, "MagazineSetup(10800)", sMagazineSetup        [tag]->CommaText);

            WriteIniData(FileName, GroupName, "AOIBinTraySetting", sAOIBinTraySetting        [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
//#endif   //AI(W906-S09-B6) 20260929: see :2895
        }
    }
    else if(tag==eBinFT)
    {
        GroupName="Bin Func FT";
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5764-5765
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "NewBinFormat",               1); //kevin 20160928 使用新的bin format
        WriteIniData(FileName, GroupName, "3617 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2908
        TransferBinTrayStrToName(tag);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5767-5785
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "3617 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3636 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3656 BinType",               sBinType                [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3676 BinConsFail",           sBinConsFail            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10641 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10642 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10643 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10644 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10645 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10646 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10647 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10648 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10649 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10650 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        WriteIniData(FileName, GroupName, "10680 LowYield",             sLowYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10681 ArmYield",             sArmYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10682 SiteYield",            sSiteYield              [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10683 BySiteClean",          sBySiteClean            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10684 ByBinClean",           sByBinClean             [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2914

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5790-5795
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "10685 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10686 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10687 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10688 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10689 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10690 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2940
        }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5798-5802
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "BinTrayLinked(10698)", sBinTrayLinked        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "BinLinked(10699)",     sBinLinked            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "MagazineSetup(10700)", sMagazineSetup        [tag]->CommaText);

        WriteIniData(FileName, GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
//#endif   //AI(W906-S09-B6) 20260929: see :2951
    }
    else if(tag==eBinOffLine)
    {
        GroupName="Bin Func OffLine";
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5807-5808
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "NewBinFormat",               1); //kevin 20160928 使用新的bin format
        WriteIniData(FileName, GroupName, "3722 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2963
        TransferBinTrayStrToName(tag);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5810-5828
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "3722 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3723 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3724 BinType",               sBinType                [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3725 BinConsFail",           sBinConsFail            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10941 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10942 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10943 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10944 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10945 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10946 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10947 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10948 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10949 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10950 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        WriteIniData(FileName, GroupName, "10980 LowYield",             sLowYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10981 ArmYield",             sArmYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10982 SiteYield",            sSiteYield              [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10983 BySiteClean",          sBySiteClean            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "10984 ByBinClean",           sByBinClean             [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2969

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5833-5838
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "10985 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10986 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10987 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10988 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10989 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "10990 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :2995
        }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5841-5845
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "BinTrayLinked(10998)", sBinTrayLinked        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "BinLinked(10999)",     sBinLinked            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "MagazineSetup(11000)", sMagazineSetup        [tag]->CommaText);

        WriteIniData(FileName, GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
//#endif   //AI(W906-S09-B6) 20260929: see :3006
    }
    else if(tag==eBinFT_ART)
    {
        GroupName="Bin Func ART_FT";                                            //wei 20161124 修改eBinFT_ART存成Bin Func ART_RT問題
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5850-5851
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "NewBinFormat",               1); //kevin 20160928 使用新的bin format
        WriteIniData(FileName, GroupName, "3801 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3018
        TransferBinTrayStrToName(tag);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5853-5871
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "3801 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3802 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3803 BinType",               sBinType                [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3804 BinConsFail",           sBinConsFail            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11041 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11042 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11043 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11044 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11045 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11046 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11047 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11048 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11049 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11050 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        WriteIniData(FileName, GroupName, "11080 LowYield",             sLowYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11081 ArmYield",             sArmYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11082 SiteYield",            sSiteYield              [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11083 BySiteClean",          sBySiteClean            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11084 ByBinClean",           sByBinClean             [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3024

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5876-5881
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "11085 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11086 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11087 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11088 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11089 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11090 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3050
        }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5884-5888
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "BinTrayLinked(11098)", sBinTrayLinked        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "BinLinked(11099)",     sBinLinked            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "MagazineSetup(11100)", sMagazineSetup        [tag]->CommaText);

        WriteIniData(FileName, GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
//#endif   //AI(W906-S09-B6) 20260929: see :3061
    }
    else if(tag==eBinRT_ART)
    {
        GroupName="Bin Func ART_RT";                                            //wei 20161124 修改eBinRT_ART存成Bin Func ART_FT問題
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5893-5894
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "NewBinFormat",               1); //kevin 20160928 使用新的bin format
        WriteIniData(FileName, GroupName, "3901 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3073
        TransferBinTrayStrToName(tag);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5896-5920
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "3901 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3902 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3903 BinType",               sBinType                [tag]->CommaText);
        WriteIniData(FileName, GroupName, "3904 BinConsFail",           sBinConsFail            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11141 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11142 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11143 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11144 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11145 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11146 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11147 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11148 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11149 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11150 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        WriteIniData(FileName, GroupName, "11180 LowYield",             sLowYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11181 ArmYield",             sArmYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11182 SiteYield",            sSiteYield              [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11183 BySiteClean",          sBySiteClean            [tag]->CommaText);   //wei 20161124 11083 --> 11183
        WriteIniData(FileName, GroupName, "11184 ByBinClean",           sByBinClean             [tag]->CommaText);   //wei 20161124 11084 --> 11184

        WriteIniData(FileName, GroupName, "BinTrayLinked(11198)", sBinTrayLinked        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "BinLinked(11199)",     sBinLinked            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "MagazineSetup(11200)", sMagazineSetup        [tag]->CommaText);

        WriteIniData(FileName, GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
//#endif   //AI(W906-S09-B6) 20260929: see :3079
    }
    else if(tag==eBinFT_MRT)
    {
        GroupName="Bin Func MRT_FT";                                            //Ifor 20170316 (wei) add KYEC MRT Mode
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5925-5926
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "NewBinFormat",               1);
        WriteIniData(FileName, GroupName, "4001 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3111
        TransferBinTrayStrToName(tag);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5928-5946
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "4001 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        WriteIniData(FileName, GroupName, "4002 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        WriteIniData(FileName, GroupName, "4003 BinType",               sBinType                [tag]->CommaText);
        WriteIniData(FileName, GroupName, "4004 BinConsFail",           sBinConsFail            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11241 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11242 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11243 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11244 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11245 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11246 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11247 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11248 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11249 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11250 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        WriteIniData(FileName, GroupName, "11280 LowYield",             sLowYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11281 ArmYield",             sArmYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11282 SiteYield",            sSiteYield              [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11283 BySiteClean",          sBySiteClean            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11284 ByBinClean",           sByBinClean             [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3117

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5951-5956
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "11285 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11286 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11287 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11288 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11289 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11290 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3143
        }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5959-5963
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "BinTrayLinked(11298)", sBinTrayLinked        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "BinLinked(11299)",     sBinLinked            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "MagazineSetup(11300)", sMagazineSetup        [tag]->CommaText);

        WriteIniData(FileName, GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
//#endif   //AI(W906-S09-B6) 20260929: see :3154
    }
    else if(tag==eBinRT_MRT)
    {
        GroupName="Bin Func MRT_RT";
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5968-5969
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "NewBinFormat",               1);
        WriteIniData(FileName, GroupName, "4101 BinTraySetting",        sBinTraySetT3Pos        [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3166
        TransferBinTrayStrToName(tag);
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5971-5989
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "4101 BinTraySetName",        sBinTraySetT3PosName    [tag]->CommaText);
        WriteIniData(FileName, GroupName, "4102 BinDoubleContac",       sBinDoubleContact       [tag]->CommaText);
        WriteIniData(FileName, GroupName, "4103 BinType",               sBinType                [tag]->CommaText);
        WriteIniData(FileName, GroupName, "4104 BinConsFail",           sBinConsFail            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11341 BinEnableFail",        sBinEnableFail          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11342 BinFailPercent",       sBinFailPercent         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11343 BinFailIgnore",        sBinFailIgnore          [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11344 BinCountEnable",       sBinCountEnable         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11345 BinCountNumber",       sBinCountNumber         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11346 BinCountIgnore",       sBinCountIgnore         [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11347 SpecialBinByArm",      sSpecialBinByArm        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11348 SpecialBinCountByArm", sSpecialBinCountByArm   [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11349 SpecialBinBySocket",   sSpecialBinBySocket     [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11350 SpecialBinCountBySocket", sSpecialBinCountBySocket[tag]->CommaText);
        WriteIniData(FileName, GroupName, "11380 LowYield",             sLowYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11381 ArmYield",             sArmYield               [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11382 SiteYield",            sSiteYield              [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11383 BySiteClean",          sBySiteClean            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "11384 ByBinClean",           sByBinClean             [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3172

        if(CosFunction.bBySiteByBinPercentCompare==true &&                      //JerryYang 20170712 (Steven) by site by bin compare percent
           CosFunction.bByBinAlarmFromYieldForm==false)                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        {
//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:5994-5999
            // GATE (write-policy)
            WriteIniData(FileName, GroupName, "11385 SpecBinBySiteCompareEnable",        sSpecBinBySiteCompareEnable         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11386 SpecBinBySiteCompareIgnore",        sSpecBinBySiteCompareIgnore         [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11387 SpecBinBySiteComparePercent",       sSpecBinBySiteComparePercent        [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11388 SpecBinByArmPerSiteCompareEnable",  sSpecBinByArmPerSiteCompareEnable   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11389 SpecBinByArmPerSiteCompareIgnore",  sSpecBinByArmPerSiteCompareIgnore   [tag]->CommaText);
            WriteIniData(FileName, GroupName, "11390 SpecBinByArmPerSiteComparePercent", sSpecBinByArmPerSiteComparePercent  [tag]->CommaText);
//#endif   //AI(W906-S09-B6) 20260929: see :3198
        }

//#if 0   //AI(W906-S09-B6) 20260929: gate retired -- the 20260819 write-policy (forms/fBinSel.h:330) is obsolete (RULINGS_20260926 S52; S99 precedent :739); body = golden 906_0625_Steven cBinSel.cpp:6002-6006
        // GATE (write-policy)
        WriteIniData(FileName, GroupName, "BinTrayLinked(11398)", sBinTrayLinked        [tag]->CommaText);
        WriteIniData(FileName, GroupName, "BinLinked(11399)",     sBinLinked            [tag]->CommaText);
        WriteIniData(FileName, GroupName, "MagazineSetup(11400)", sMagazineSetup        [tag]->CommaText);

        WriteIniData(FileName, GroupName,"AOIBinTraySetting", sAOIBinTraySetting    [tag]->CommaText);  //Eastsun 20260316 : AOI 要分bin
//#endif   //AI(W906-S09-B6) 20260929: see :3209
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  SetPrimeButton -- golden :6010-6042 (33 lines). FW-BinSel-WB (this wave).
//  GATE: entire body -- "純 widget" per this wave's own brief, confirmed by
//  grep: `spbNormal`/`spbPrime`/`tsRetest` are absent from TfBinSel's facade
//  (`grep -n "spbNormal\|spbPrime\|tsRetest" forms/fBinSel.h` -- only
//  comment hits, 20260819) and `fMain->SetNormalOrPrime()` is absent
//  (`grep -n "SetNormalOrPrime" forms/fMain.h` -- 0 hits, 20260819).   [AI(W906-NL-ABSENCE) 20260926: that half is now false -- TfMain::SetNormalOrPrime exists since OPMODE 7304dcef (forms/fMain_OperateMode.cpp:574), but every statement in its port body is a web-display gate (palPrime/palNormal), so calling it is a no-op; the widget half (spbNormal/spbPrime/tsRetest) alone keeps this block gated, behaviour-identical. Found by tools/absence_sentinel.py]
//  `cbTestMode->Enabled` (the one statement whose target DOES exist on the
//  facade) is left gated alongside its sibling widgets rather than partially
//  wired: toggling Enabled without the paired spbNormal/spbPrime
//  Visible/Down state would show an incomplete, possibly-misleading UI
//  state, not golden's real behaviour.
//---------------------------------------------------------------------------
void TfBinSel::SetPrimeButton()
{
#if 0
    if(IniConfig.bFTBin2RTBin && IniConfig.bA02BinModelPrime)                   //ChungHung 20120912 add Bin Setting like Epson
    {
        spbNormal->Visible=true;
        spbPrime->Visible=true;

        if(iBinModelPrime==1)
        {
            tsRetest->Enabled=true;
            spbPrime->Down=true;
            cbTestMode->Enabled=true;
        }
        else
        {
            tsRetest->Enabled=false;
            spbNormal->Down=true;
            cbTestMode->Enabled=false;
        }
    }
    else
    {
        if(IniConfig.bFTBin2RTBin && IniConfig.bA02BinModelPrime==false)        //jou 2014-08-26 修正 BIN select FT/RT 顯示異常
        {
            tsRetest->Enabled=false;
            cbTestMode->Enabled=false;
        }

        spbNormal->Visible=false;
        spbPrime->Visible=false;
    }
    fMain->SetNormalOrPrime();                                                  //Steven 20160817 : 必須在讀完Bin別設定之後
#endif
}

//---------------------------------------------------------------------------
//  mtTrayNameSetColor -- golden :4033-4140 (108 lines). FW-BinSel-WC (this
//  wave) unlocks the FULL body -- FW-BinSel-WB gated it whole pending the
//  widget pair FW-BinSel-WC now lands (see forms/fBinSel.h GATE REGISTER
//  G9, CLOSED this wave). `.YItem` (golden's own __published property READ)
//  translates to `.FYItem` (the underlying field vclcompat exposes directly
//  -- Tray256Core has no property-getter emulation, see vclcompat/
//  TrayCore.h; property WRITES already translate to `Set*()` calls
//  elsewhere in this file, e.g. this wave's own ctor addition).
//
//  HISTORICAL (WAVE B's own reasoning for gating -- kept for context, no
//  longer the current status): WAVE A's STUB COLLISION SCAN asserted `grep
//  -rn "class TTMyTray256" vclcompat/` -- 0 hits (20260819) and concluded
//  "no vclcompat stand-in exists anywhere in the tree". That literal-string
//  grep was correct but MISLEADING -- W7-C1 (20260728, BEFORE WAVE A's own
//  banner date) already added `vclcompat::Tray256Core`/`vclcompat::TrayCore`
//  (vclcompat/TrayCore.h/.cpp, linked into ht9045_sm per CMakeLists.txt:256)
//  as this tree's RENAMED stand-in for golden's TTMyTray256/TTMyTray
//  (vclcompat/Controls.h:79's own cross-reference). WAVE B wiring THIS
//  method alone would have needed the SAME three additions FW-BinSel-WC now
//  makes: (1) `TMyBinPanelData::mtTrayName`/`mtTrayItem` (per-tag
//  Tray256Core widgets), (2) `TfBinSel::mtTrayName`/`mtTrayItem` (the
//  SEPARATE class-level TrayCore pair; see (B16) below), (3) the ~90-line
//  cell-text population golden's OWN `TMyBinPanel::TMyBinPanel` ctor does
//  for these widgets (golden :420-509, landed this wave in `TfBinSel::
//  TfBinSel`'s own per-tag loop) -- wiring only this method without (3)
//  would have left eItemART/eItemCateR/eItemPass's cell TEXT blank forever
//  while COLOUR rendered fine, a half-populated grid WAVE B judged worse
//  than a documented gate.
//
//  (B16) GOLDEN QUIRK (unaffected by this wave's unlock -- preserved
//  verbatim, not "fixed"): the two calls at golden :4094/:4106
//  (`mtTrayName->SetCellNumber(eItemART,j,"Retest")` / `mtTrayName->
//  SetCellNumber(eItemCateR,j,"CateR")`) omit the `MyBinPanel[tag]->` prefix
//  every sibling call in this SAME method has. Confirmed against golden
//  cBinSel.h:73-74: `TfBinSel` has its OWN `TTMyTray *mtTrayName`/
//  `*mtTrayItem` (a DIFFERENT, class-level, tag-invariant pair -- NOT
//  `MyBinPanel[tag]`'s per-tag one, see this class's own TrayCore members) --
//  so these two calls write the (tag-invariant) label text "Retest"/"CateR"
//  onto that shared widget instead of the per-tag one that every other line
//  in this method targets. A separate finding from the ReadFunctionData
//  (B14)/(B15) AOI-key bugs above.
//---------------------------------------------------------------------------
void TfBinSel::mtTrayNameSetColor(int tag)
{
    int iT6, iT6Pos;
    for(int j=eBinSetting; j<eBinSetTotal; j++)
    {
        if(j>=MyBinPanel[tag]->mtTrayName->FYItem)
            continue;

        iT6   =j-eBinSetting;
        iT6Pos=j-eBinNotUse;

        if(Prod.iTrayType[iT6]==tNotUse)
        {
            MyBinPanel[tag]->mtTrayItem->SetCellNumber    (eItemError,  j, "");
            MyBinPanel[tag]->mtTrayItem->SetCellColorIndex(eItemError,  j, eCLBtnFace);
            MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemPass,   j, "");
            MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemPass,   j, eCLBtnFace);
            MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemART,    j, "");
            MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemART,    j, eCLBtnFace);
            MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemCateR,  j, "");
            MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemCateR,  j, eCLBtnFace);
            MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemLink,   j, "");
            MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemLink,   j, eCLBtnFace);
        }
        else
        {
            //Error----------------
            if(MyBinPanel[tag]->iErrorT6==iT6)                                      //Error     //JerryYang 20230926
            {
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemError, j, "Error");
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemError, j, eCLRed);
            }
            else
            {
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemError, j, eCLWhite);
            }

            //Pass/Fail------------
            if(TestIF_File.bEnableQASampling &&                                     //Steven 20190326 : QA Sampling
               iT6Pos==TestIF_File.iQASamplingT3Pos)
            {
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemPass, j, "QA");
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemPass, j, eCLBlue);
            }
            else if(MyBinPanel[tag]->iT6IsFail[iT6]>0)                              //0=Pass / 1=Fail
            {
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemPass, j, "Failed");
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemPass, j, MyBinPanel[tag]->iT6IsFail[j-eBinSetting]+1);  //Steven 20240702 : fixed for bin color
            }
            else
            {
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemPass, j, "Pass");
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemPass, j, eCLGreen);
            }

            //ART------------------
            if(USE_AUTO_RETEST==eartInstall ||                                  //ChungHung 20140317 add Auto Retest
               IniConfig.bA38_SLT_Summary)                                      //JerryYang 20220923 : add for SLT lot summary
            {
                if(iT6<=iAutoRight)
                {
                    mtTrayName->SetCellNumber(eItemART, j, "Retest");
                    if(UNLOADER_ART[iT6]==eartInstall &&                        //Steven 20161221 (wei) : for SCK only Auto 2 has ART
                       MyBinPanel[tag]->bT6ART[iT6])
                    {
                        MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemART, j, eCLRed);
                    }
                    else
                    {
                        MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemART, j, eCLWhite);
                    }
                }

                mtTrayName->SetCellNumber(eItemCateR, j, "CateR");
                if(CosFunction.bUseSCKART &&
                   (TestIF_File.iSCKART_SortMode==1 ||                          //Steven 20161221 (wei) : Cate R for ART
                    IniConfig.bA38_SLT_Summary) &&                              //JerryYang 20220923 : add for SLT lot summary
                    MyBinPanel[tag]->bT6CateR[iT6])
                {
                    MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemCateR, j, eCLRed);
                }
                else
                {
                    MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemCateR, j, eCLWhite);
                }
            }
            else
            {
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemART,   j, eCLWhite);
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemCateR, j, eCLWhite);
            }

            //Link-----------------
            if(MyBinPanel[tag]->bT6Link[iT6] && bCanLinkT6[iT6]==true)
            {
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemLink, j, "Linked");
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemLink, j, eCLGreen);
            }
            else
            {
                MyBinPanel[tag]->mtTrayName->SetCellNumber    (eItemLink, j, "");
                MyBinPanel[tag]->mtTrayName->SetCellColorIndex(eItemLink, j, eCLWhite);
            }
        }
    }

    InitDataToEdit(tag);   // golden :4139 -- FW-BinSel-WC lands its real body below
}

//---------------------------------------------------------------------------
//  InitDataToEdit -- golden :4142-4784 (643 lines). FW-BinSel-WC (this
//  wave), newly declared (WAVE B's own "最後大宗" backlog item).
//
//  ACTIVE/GATE SPLIT (per-block; see forms/fBinSel.h GATE REGISTER G7/G10):
//  every golden statement here either (a) writes a `MyBinPanel[tag]->ed*`
//  TEdit widget that ONLY ever feeds `TfBinSel::sXxx[tag]->CommaText` at
//  THIS SAME function's own tail (golden :4685-4711) -- ported ACTIVE, S20
//  (see below), by writing the computed CommaText DIRECTLY into `sXxx[tag]`
//  at the point golden itself computes it, instead of round-tripping
//  through the (absent) `ed*->Text` scratch buffer; (b) writes an ed* with
//  NO sXxx[tag] tail consumer at all (edBinSetT6Error/edBinSetT6PassFail/
//  edT6BinART/edT6CateR/edBinSettingEnableScan) -- pure widget display
//  text, GATE (G7), nothing to elide TO; or (c) writes `MyBinPanel[tag]->
//  mtBinSelect` (grid-cell rendering) -- GATE (G10): a THIRD Tray256Core*,
//  distinct from the mtTrayName/mtTrayItem pair this wave's ctor addition
//  DOES land (see TfBinSel::TfBinSel's own banner). A few statements are
//  neither -- genuine DATA mutations independent of any widget
//  (bSpecialBinByArm/bSpecialBinBySocket stale-flag resets, golden
//  :4404-4420/:4442-4458) and the entire tail Link-chain computation
//  (golden :4712-4775, pure TStringList arithmetic over
//  sBinTraySetT3Pos[tag]/sBinTrayLinked[tag] -- both real members, already
//  set earlier in THIS SAME call -- into sBinLinked[tag]) -- these are
//  ACTIVE unconditionally, the same as any other pure-data golden logic in
//  this file.
//
//  (S20) DEVIATION: golden's own `MyBinPanel[tag]->ed*->Text=<value>;` ..
//  (a few hundred lines later, SAME function call) .. `sXxx[tag]->
//  CommaText=MyBinPanel[tag]->ed*->Text;` two-step round-trip collapses to
//  one `sXxx[tag]->CommaText=<value>;` at the point golden itself computes
//  <value> -- BIT-FOR-BIT the same final `sXxx[tag]` content golden's own
//  two-step assignment produces. Verified by grepping every ed* name this
//  function touches against the FULL golden cBinSel.cpp text: each has
//  exactly ONE writer (this function, or ReadFunctionData's own
//  already-gated G7 mirror) and AT MOST the one tail reader golden
//  :4685-4711 itself (the 4 names under GATE (b) above have no tail reader
//  at all, per forms/fBinSel.h GATE REGISTER G7's own citation list).
//  golden's `ZeroMemory(asBinLinked, sizeof(asBinLinked));` (golden :4148)
//  is dropped: `vclcompat::AnsiString` wraps a `std::string` (see
//  AnsiString.h) -- raw-memory-zeroing a non-POD C++ object is undefined
//  behaviour here (unlike BCB6's own reference-counted AnsiString, where
//  this was a harmless, if redundant, defensive idiom) -- every element's
//  default ctor already produces "" (AnsiString.h:58), the SAME end state
//  golden's ZeroMemory call was defensively ensuring.
//
//  golden's ByBin/BySite tray-clean NAME SWAP (`iAutoCleanByBin` feeds
//  `sBySiteClean`, `iAutoCleanBySite` feeds `sByBinClean` -- golden
//  :4503-4517 below) is NOT a new finding: ReadFunctionData's own ALREADY-
//  ACTIVE code in this file (its `BinSelect[tag].iAutoCleanByBin[i]=...
//  sBySiteClean[tag]...` / `...iAutoCleanBySite[i]=...sByBinClean[tag]...`
//  pairing) established the identical convention first -- confusingly
//  named, but self-consistent throughout golden, not re-flagged here.
//---------------------------------------------------------------------------
void TfBinSel::InitDataToEdit(int tag)
{
    AnsiString Str="", Temp="";
    TStringList *sList=new TStringList();
    TStringList *sLinkedList=new TStringList();
    AnsiString asBinLinked[eTrayCount];   // golden ZeroMemory dropped -- see banner above (S20 area)

    // GATE (G7): edBinSetT6Error absent, no sXxx[tag] tail consumer either
    // (see banner above) -- pure widget display text.
#if 0
    //Error-----------
    Str.sprintf("%d", MyBinPanel[tag]->iErrorT6);
    MyBinPanel[tag]->edBinSetT6Error->Text=Str;

    //Pass Fail-------
    sList->Clear();
    for(int i=0; i<eTrayCount; i++)
    {
        sList->Add(MyBinPanel[tag]->iT6IsFail[i]);
    }
    MyBinPanel[tag]->edBinSetT6PassFail->Text=sList->CommaText;

    //AutoRetest------
    sList->Clear();
    for(int i=eAuto1; i<=iAutoRight; i++)
    {
        sList->Add((MyBinPanel[tag]->bT6ART[i])?"1":"0");
    }
    MyBinPanel[tag]->edT6BinART->Text=sList->CommaText;

    //AutoRetest Cate R------                                                   //Steven 20161221 (wei) : for SCK only Auto 2 has ART
    sList->Clear();
    for(int i=0; i<eTrayCount; i++)
    {
        sList->Add((MyBinPanel[tag]->bT6CateR[i])?"1":"0");
    }
    MyBinPanel[tag]->edT6CateR->Text=sList->CommaText;
#endif

    //Link------- (golden :4180-4189) ACTIVE: edBinSetT6Link's ONLY tail
    // consumer is sBinTrayLinked[tag] (golden :4711, unconditional) -- S20.
    sList->Clear();
    for(int i=0; i<eTrayCount; i++)
    {
        if(bCanLinkT6[i]==false)
            sList->Add("0");
        else
            sList->Add((MyBinPanel[tag]->bT6Link[i])?"1":"0");
    }
    sBinTrayLinked[tag]->CommaText=sList->CommaText;   // S20: was edBinSetT6Link->Text, read back at golden :4711
#if 0
    // GATE (G7): the widget mirror itself.
    MyBinPanel[tag]->edBinSetT6Link->Text=sList->CommaText;
#endif

    //Bin------------- (golden :4191-4249)
    // part 1 (:4192-4204) ACTIVE: edBinSetT3Pos's ONLY tail consumer is
    // sBinTraySetT3Pos[tag] (golden :4685, unconditional) -- S20.
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        for(int j=eBinNotUse; j<eBinSetTotal; j++)
        {
            if(MyBinPanel[tag]->BackT6PosTray[i][j]==1)
            {
                Temp.sprintf("%d", iTo3PosUnload[j-eBinNotUse]);                //第一排不算,所以要-1
                sList->Add(Temp);
            }
        }
    }
    sBinTraySetT3Pos[tag]->CommaText=sList->CommaText;   // S20: was edBinSetT3Pos->Text, read back at golden :4685
#if 0
    // GATE (G7)
    MyBinPanel[tag]->edBinSetT3Pos->Text=sList->CommaText;
#endif

    // part 2 (:4206-4249) GATE (G10): pure mtBinSelect grid-cell colouring,
    // no ed*, no other consumer.
#if 0
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        for(int j=eBinNotUse; j<eBinSetTotal; j++)
        {
            iT6=j-eBinSetting;
            if(j>MyBinPanel[tag]->mtBinSelect->YItem)
                continue;

            if(MyBinPanel[tag]->BackT6PosTray[i][j])
            {
                if(j==eBinNotUse)                                               //No USE
                {
                    MyBinPanel[tag]->mtBinSelect->SetCellColorIndex(i, j, eCLSilver);
                }
                else
                {
                    if(MyBinPanel[tag]->iT6IsFail[iT6]>0)
                    {
                        MyBinPanel[tag]->mtBinSelect->SetCellColorIndex(i, j, eCLRed);
                    }
                    else
                    {
                        MyBinPanel[tag]->mtBinSelect->SetCellColorIndex(i, j, eCLGreen);
                    }
                }
            }
            else if(AUTO3_IS_MAGAZINE==1 && iT6==eAuto3)                        //JerryYang 20221207 : Magazine not use AUTO3
            {
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex(i, j, eCLBtnFace);
            }
            else if(CosFunction.bLoaderTrayToAuto1 && TrayForm.LoaderToEmptyColor[iRunStartMode]==2 && iT6==eAuto1)
            {
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex(i, j, eCLBtnFace);
            }
            else if(Prod.iTrayType[iT6]==tNotUse)                               //JerryYang 20221215 : Magazine當fix區當buffer使用功能
            {
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex(i, j, eCLBtnFace);
            }
            else
            {
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex(i, j, eCLWhite);
            }
        }
    }
#endif

    // GATE (G7)+(G10): Scan block (golden :4251-4259) -- unconditionally
    // "0" for every bin, edBinSettingEnableScan has NO sXxx[tag] tail
    // consumer -- pure dead-end widget churn, nothing to elide TO.
#if 0
    //Scan-------------
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        sList->Add("0");
        MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eScanning, "");
        MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eScanning, eCLGray);
    }
    MyBinPanel[tag]->edBinSettingEnableScan->Text=sList->CommaText;
#endif

    //Double Contact--- (golden :4261-4278) ACTIVE sList compute -> S20 ->
    // sBinDoubleContact[tag] (golden :4686). GATE (G10) mtBinSelect writes.
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        sList->Add(MyBinPanel[tag]->i2Contact[i]);                              //Steven 20231205 : b2Contact --> i2Contact
#if 0
        if(Prod.bD22SupportMultiDoubleContact)
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eDoubleContact, MyBinPanel[tag]->i2Contact[i]);
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eDoubleContact, (MyBinPanel[tag]->i2Contact[i])?eCLOlive:eCLBtnFace);
        }
        else
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eDoubleContact, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eDoubleContact, eCLGray);
        }
#endif
    }
    sBinDoubleContact[tag]->CommaText=sList->CommaText;

    //Con. Fail-------- (golden :4280-4288) ACTIVE -> S20 -> sBinConsFail[tag]
    // (golden :4687). GATE (G10) mtBinSelect writes.
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        sList->Add((MyBinPanel[tag]->bConFail[i])?"1":"0");
#if 0
        MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eConsFail, (MyBinPanel[tag]->bConFail[i])?"V":"");
        MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eConsFail, (MyBinPanel[tag]->bConFail[i])?eCLOlive:eCLBtnFace);
#endif
    }
    sBinConsFail[tag]->CommaText=sList->CommaText;

    // Persent Enable/Ignore/Number (golden :4290-4354) ACTIVE per-branch ->
    // S20 -> sBinEnableFail[tag]/sBinFailIgnore[tag]/sBinFailPercent[tag]
    // (golden :4688-4690). GATE (G10) mtBinSelect writes, both branches.
    if(CosFunction.bByBinAlarmFromYieldForm)                                    //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
    {
#if 0
        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20251104 : iBinCount --> iTestBinCount
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, ePersentEnable, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, ePersentEnable, eCLGray);
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, ePersentIgnore, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, ePersentIgnore, eCLGray);
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, ePersentNumber, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, ePersentNumber, eCLGray);
        }
#endif
        sBinEnableFail[tag]->CommaText  ="";
        sBinFailIgnore[tag]->CommaText  ="";
        sBinFailPercent[tag]->CommaText ="";
    }
    else
    {
        //Persent Enable------
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20251104 : iBinCount --> iTestBinCount
        {
            sList->Add((MyBinPanel[tag]->bPersentEnable[i])?"1":"0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, ePersentEnable, (MyBinPanel[tag]->bPersentEnable[i])?"V":"");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, ePersentEnable, (MyBinPanel[tag]->bPersentEnable[i])?eCLOlive:eCLBtnFace);
#endif
        }
        sBinEnableFail[tag]->CommaText=sList->CommaText;

        //Persent Ignore------
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20251104 : iBinCount --> iTestBinCount
        {
            sList->Add(MyBinPanel[tag]->iPersentIgnore[i]);
#if 0
            if(MyBinPanel[tag]->bPersentEnable[i])
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, ePersentIgnore, MyBinPanel[tag]->iPersentIgnore[i]);
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, ePersentIgnore, eCLOlive);
            }
            else
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, ePersentIgnore, "");
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, ePersentIgnore, eCLBtnFace);
            }
#endif
        }
        sBinFailIgnore[tag]->CommaText=sList->CommaText;

        //Persent Number------
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20251104 : iBinCount --> iTestBinCount
        {
            sList->Add(MyBinPanel[tag]->dPersentNumber[i]);
#if 0
            if(MyBinPanel[tag]->bPersentEnable[i])
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, ePersentNumber, MyBinPanel[tag]->dPersentNumber[i]);
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, ePersentNumber, eCLOlive);
            }
            else
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, ePersentNumber, "");
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, ePersentNumber, eCLBtnFace);
            }
#endif
        }
        sBinFailPercent[tag]->CommaText=sList->CommaText;
    }

    //Count Enable------ (golden :4356-4364) ACTIVE -> S20 -> sBinCountEnable[tag]
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        sList->Add((MyBinPanel[tag]->bCountEnable[i])?"1":"0");
#if 0
        MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eCountEnable, (MyBinPanel[tag]->bCountEnable[i])?"V":"");
        MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eCountEnable, (MyBinPanel[tag]->bCountEnable[i])?eCLOlive:eCLBtnFace);
#endif
    }
    sBinCountEnable[tag]->CommaText=sList->CommaText;

    //Count Ignore------ (golden :4366-4383) ACTIVE -> S20 -> sBinCountIgnore[tag]
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        sList->Add(MyBinPanel[tag]->iCountIgnore[i]);
#if 0
        if(MyBinPanel[tag]->bCountEnable[i])
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eCountIgnore, MyBinPanel[tag]->iCountIgnore[i]);
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eCountIgnore, eCLOlive);
        }
        else
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eCountIgnore, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eCountIgnore, eCLBtnFace);
        }
#endif
    }
    sBinCountIgnore[tag]->CommaText=sList->CommaText;

    //Count Number------ (golden :4385-4402) ACTIVE -> S20 -> sBinCountNumber[tag]
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        sList->Add(MyBinPanel[tag]->iCountNumber[i]);
#if 0
        if(MyBinPanel[tag]->bCountEnable[i])
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eCountNumber, MyBinPanel[tag]->iCountNumber[i]);
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eCountNumber, eCLOlive);
        }
        else
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eCountNumber, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eCountNumber, eCLBtnFace);
        }
#endif
    }
    sBinCountNumber[tag]->CommaText=sList->CommaText;

    //Special Bin By Arm Enable------ (golden :4404-4421) ACTIVE: the
    // stale-flag-reset mutation is real data logic (no widget involved) +
    // sList -> S20 -> sSpecialBinByArm[tag]. GATE (G10) mtBinSelect writes.
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        for(int j=eBinSetting; j<eBinSetTotal; j++)
        {
            if(MyBinPanel[tag]->BackT6PosTray[i][j]==1)
            {
                if(MyBinPanel[tag]->iT6IsFail[j-eBinSetting]==0)
                    MyBinPanel[tag]->bSpecialBinByArm[i]=false;
            }
        }

        sList->Add((MyBinPanel[tag]->bSpecialBinByArm[i])?"1":"0");
#if 0
        MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecialBinByArm, (MyBinPanel[tag]->bSpecialBinByArm[i])?"V":"");
        MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecialBinByArm, (MyBinPanel[tag]->bSpecialBinByArm[i])?eCLOlive:eCLBtnFace);
#endif
    }
    sSpecialBinByArm[tag]->CommaText=sList->CommaText;

    //Special Bin Count by Arm ------ (golden :4423-4440) ACTIVE -> S20 -> sSpecialBinCountByArm[tag]
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        sList->Add(MyBinPanel[tag]->iSpecialBinCountByArm[i]);
#if 0
        if(MyBinPanel[tag]->bSpecialBinByArm[i])
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecialBinCountByArm, int(MyBinPanel[tag]->iSpecialBinCountByArm[i]));
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecialBinCountByArm, eCLOlive);
        }
        else
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecialBinCountByArm, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecialBinCountByArm, eCLBtnFace);
        }
#endif
    }
    sSpecialBinCountByArm[tag]->CommaText=sList->CommaText;

    //Special Bin By Socket Enable------ (golden :4442-4459) ACTIVE: same
    // stale-flag-reset shape as "By Arm" above, for Socket -> S20 ->
    // sSpecialBinBySocket[tag].
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        for(int j=eBinSetting; j<eBinSetTotal; j++)
        {
            if(MyBinPanel[tag]->BackT6PosTray[i][j]==1)
            {
                if(MyBinPanel[tag]->iT6IsFail[j-eBinSetting]==0)
                    MyBinPanel[tag]->bSpecialBinBySocket[i]=false;
            }
        }

        sList->Add((MyBinPanel[tag]->bSpecialBinBySocket[i])?"1":"0");
#if 0
        MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecialBinBySocket, (MyBinPanel[tag]->bSpecialBinBySocket[i])?"V":"");
        MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecialBinBySocket, (MyBinPanel[tag]->bSpecialBinBySocket[i])?eCLOlive:eCLBtnFace);
#endif
    }
    sSpecialBinBySocket[tag]->CommaText=sList->CommaText;

    //Special Bin Count by Socket ------ (golden :4461-4478) ACTIVE -> S20 -> sSpecialBinCountBySocket[tag]
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20251104 : iBinCount --> iTestBinCount
    {
        sList->Add(MyBinPanel[tag]->iSpecialBinCountBySocket[i]);
#if 0
        if(MyBinPanel[tag]->bSpecialBinBySocket[i])
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecialBinCountBySocket, int(MyBinPanel[tag]->iSpecialBinCountBySocket[i]));
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecialBinCountBySocket, eCLOlive);
        }
        else
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecialBinCountBySocket, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecialBinCountBySocket, eCLBtnFace);
        }
#endif
    }
    sSpecialBinCountBySocket[tag]->CommaText=sList->CommaText;

    //Low Yield (golden :4480-4496) ACTIVE: BOTH branches feed the SAME
    // sLowYield[tag] with DIFFERENT content (Prod.bLowYieldAlarmByBin gates
    // whether the real bLowYield[] flag or a hardcoded "0" is used) -> S20.
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20140828 : By Bin Yield Monitor
    {
        if(Prod.bLowYieldAlarmByBin)
        {
            sList->Add((MyBinPanel[tag]->bLowYield[i])?"1":"0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eLowYield, (MyBinPanel[tag]->bLowYield[i])?"V":"");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eLowYield, (MyBinPanel[tag]->bLowYield[i])?eCLOlive:eCLBtnFace);
#endif
        }
        else
        {
            sList->Add("0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eLowYield, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eLowYield, eCLGray);
#endif
        }
    }
    sLowYield[tag]->CommaText=sList->CommaText;

    //AutoClean (golden :4498-4528) ACTIVE only in the iAutoClean_Function
    // branch -> S20 -> sBySiteClean[tag]/sByBinClean[tag] (ByBin/BySite NAME
    // SWAP -- see banner above, not a new bug). The `else` arm touches ONLY
    // mtBinSelect (GATE, G10) and sets neither ed*, so golden's own tail
    // read-back is a no-op there -- ported as "leave sBySiteClean[tag]/
    // sByBinClean[tag] unchanged" (no assignment in that branch).
    if(TestIF_File.iAutoClean_Function)
    {
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)
        {
            sList->Add(MyBinPanel[tag]->iAutoCleanByBin[i]);
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eAutoCleanByBin, (MyBinPanel[tag]->iAutoCleanByBin[i]!=0)?AnsiString(MyBinPanel[tag]->iAutoCleanByBin[i]).c_str():"");    //Steven 20160308 : By Bin pre site count do auto clean
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eAutoCleanByBin, (MyBinPanel[tag]->iAutoCleanByBin[i]!=0)?eCLOlive:eCLBtnFace);
#endif
        }
        sBySiteClean[tag]->CommaText=sList->CommaText;

        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)
        {
            sList->Add(MyBinPanel[tag]->iAutoCleanBySite[i]);
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eAutoCleanBySite, (MyBinPanel[tag]->iAutoCleanBySite[i]!=0)?AnsiString(MyBinPanel[tag]->iAutoCleanBySite[i]).c_str():""); //Steven 20160308 : By Bin pre site count do auto clean
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eAutoCleanBySite, (MyBinPanel[tag]->iAutoCleanBySite[i]!=0)?eCLOlive:eCLBtnFace);
#endif
        }
        sByBinClean[tag]->CommaText=sList->CommaText;
    }
#if 0
    else
    {
        for(int i=0; i<iTestBinCount; i++)
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eAutoCleanByBin, "");
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eAutoCleanBySite, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eAutoCleanByBin, eCLGray);      //Steven 20160308 : By Bin count do auto clean
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eAutoCleanBySite, eCLGray);     //Steven 20160308 : By Bin pre site count do auto clean
        }
    }
#endif

    //By Arm Yield (golden :4530-4546) ACTIVE, same Prod.bLowYieldAlarmByBin
    // shape as Low Yield above -> S20 -> sArmYield[tag]
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20140828 : By Bin Arm Yield Monitor
    {
        if(Prod.bLowYieldAlarmByBin)
        {
            sList->Add((MyBinPanel[tag]->bArmYield[i])?"1":"0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eArmYield, (MyBinPanel[tag]->bArmYield[i])?"V":"");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eArmYield, (MyBinPanel[tag]->bArmYield[i])?eCLOlive:eCLBtnFace);
#endif
        }
        else
        {
            sList->Add("0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eArmYield, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eArmYield, eCLGray);
#endif
        }
    }
    sArmYield[tag]->CommaText=sList->CommaText;

    //By Site Yield (golden :4548-4564) ACTIVE, same shape -> S20 -> sSiteYield[tag]
    sList->Clear();
    for(int i=0; i<iTestBinCount; i++)                                          //Steven 20140828 : By Bin Site Yield Monitor
    {
        if(Prod.bLowYieldAlarmByBin)
        {
            sList->Add((MyBinPanel[tag]->bSiteYield[i])?"1":"0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSiteYield, (MyBinPanel[tag]->bSiteYield[i])?"V":"");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSiteYield, (MyBinPanel[tag]->bSiteYield[i])?eCLOlive:eCLBtnFace);
#endif
        }
        else
        {
            sList->Add("0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSiteYield, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSiteYield, eCLGray);
#endif
        }
    }
    sSiteYield[tag]->CommaText=sList->CommaText;

    //SpecBin BySite/ByArm Compare (golden :4566-4683) ACTIVE both arms ->
    // S20 -> the 6 sSpecBinXxx[tag] members.
    if(CosFunction.bBySiteByBinPercentCompare==false ||                         //JerryYang 20170712 (Steven) by site by bin compare percent
       (CosFunction.bBySiteByBinPercentCompare==true &&                         //jou 20180113 (Steven) : By Site By Bin Percent Compare From Yield form
        CosFunction.bByBinAlarmFromYieldForm))
    {
#if 0
        for(int i=0; i<iTestBinCount; i++)
        {
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinBySiteCompareEnable, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinBySiteCompareEnable, eCLGray);
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinBySiteCompareIgnore, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinBySiteCompareIgnore, eCLGray);
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinBySiteComparePercent, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinBySiteComparePercent, eCLGray);
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinByArmPerSiteCompareEnable, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinByArmPerSiteCompareEnable, eCLGray);
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinByArmPerSiteCompareIgnore, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinByArmPerSiteCompareIgnore, eCLGray);
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinByArmPerSiteComparePercent, "");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinByArmPerSiteComparePercent, eCLGray);
        }
#endif
        sSpecBinBySiteCompareEnable[tag]->CommaText        ="";
        sSpecBinBySiteCompareIgnore[tag]->CommaText        ="";
        sSpecBinBySiteComparePercent[tag]->CommaText       ="";
        sSpecBinByArmPerSiteCompareEnable[tag]->CommaText  ="";
        sSpecBinByArmPerSiteCompareIgnore[tag]->CommaText  ="";
        sSpecBinByArmPerSiteComparePercent[tag]->CommaText ="";
    }
    else
    {
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)
        {
            sList->Add((MyBinPanel[tag]->bSpecBinBySiteCompareEnable[i])?"1":"0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinBySiteCompareEnable, (MyBinPanel[tag]->bSpecBinBySiteCompareEnable[i])?"V":"");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinBySiteCompareEnable, (MyBinPanel[tag]->bSpecBinBySiteCompareEnable[i])?eCLOlive:eCLBtnFace);
#endif
        }
        sSpecBinBySiteCompareEnable[tag]->CommaText=sList->CommaText;

        //Ignore
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)
        {
            sList->Add(MyBinPanel[tag]->iSpecBinBySiteCompareIgnore[i]);
#if 0
            if(MyBinPanel[tag]->bSpecBinBySiteCompareEnable[i])
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinBySiteCompareIgnore, MyBinPanel[tag]->iSpecBinBySiteCompareIgnore[i]);
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinBySiteCompareIgnore, eCLOlive);
            }
            else
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinBySiteCompareIgnore, "");
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinBySiteCompareIgnore, eCLBtnFace);
            }
#endif
        }
        sSpecBinBySiteCompareIgnore[tag]->CommaText=sList->CommaText;

        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)
        {
            sList->Add(MyBinPanel[tag]->dSpecBinBySiteComparePercent[i]);
#if 0
            if(MyBinPanel[tag]->bSpecBinBySiteCompareEnable[i])
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinBySiteComparePercent, MyBinPanel[tag]->dSpecBinBySiteComparePercent[i]);
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinBySiteComparePercent, eCLOlive);
            }
            else
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinBySiteComparePercent, "");
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinBySiteComparePercent, eCLBtnFace);
            }
#endif
        }
        sSpecBinBySiteComparePercent[tag]->CommaText=sList->CommaText;

        //Percent Enable------
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)
        {
            sList->Add((MyBinPanel[tag]->bSpecBinByArmPerSiteCompareEnable[i])?"1":"0");
#if 0
            MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinByArmPerSiteCompareEnable, (MyBinPanel[tag]->bSpecBinByArmPerSiteCompareEnable[i])?"V":"");
            MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinByArmPerSiteCompareEnable, (MyBinPanel[tag]->bSpecBinByArmPerSiteCompareEnable[i])?eCLOlive:eCLBtnFace);
#endif
        }
        sSpecBinByArmPerSiteCompareEnable[tag]->CommaText=sList->CommaText;

        //Ignore
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)
        {
            sList->Add(MyBinPanel[tag]->iSpecBinByArmPerSiteCompareIgnore[i]);
#if 0
            if(MyBinPanel[tag]->bSpecBinByArmPerSiteCompareEnable[i])
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinByArmPerSiteCompareIgnore, MyBinPanel[tag]->iSpecBinByArmPerSiteCompareIgnore[i]);
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinByArmPerSiteCompareIgnore, eCLOlive);
            }
            else
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinByArmPerSiteCompareIgnore, "");
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinByArmPerSiteCompareIgnore, eCLBtnFace);
            }
#endif
        }
        sSpecBinByArmPerSiteCompareIgnore[tag]->CommaText=sList->CommaText;

        //Percent Number------
        sList->Clear();
        for(int i=0; i<iTestBinCount; i++)
        {
            sList->Add(MyBinPanel[tag]->dSpecBinByArmPerSiteComparePercent[i]);
#if 0
            if(MyBinPanel[tag]->bSpecBinByArmPerSiteCompareEnable[i])
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinByArmPerSiteComparePercent, MyBinPanel[tag]->dSpecBinByArmPerSiteComparePercent[i]);
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinByArmPerSiteComparePercent, eCLOlive);
            }
            else
            {
                MyBinPanel[tag]->mtBinSelect->SetCellNumber     (i, eSpecBinByArmPerSiteComparePercent, "");
                MyBinPanel[tag]->mtBinSelect->SetCellColorIndex (i, eSpecBinByArmPerSiteComparePercent, eCLBtnFace);
            }
#endif
        }
        sSpecBinByArmPerSiteComparePercent[tag]->CommaText=sList->CommaText;
    }

    // golden :4685-4711's own tail round-trip block is now REDUNDANT (every
    // sXxx[tag] it sets is already set directly above, S20) EXCEPT for the
    // 4 GATE (G7)-only ed*/sXxx pairs which have no sXxx target at all (see
    // banner above) -- nothing further to do here.

    //Link chain (golden :4712-4775) ACTIVE: pure TStringList arithmetic
    // over sBinTraySetT3Pos[tag]/sBinTrayLinked[tag] (both real members,
    // already set above in THIS SAME call, same order golden itself uses)
    // -> sBinLinked[tag]. No widget involved anywhere in this block.
    sList->Clear();
    sLinkedList->Clear();

    sList->CommaText=sBinTraySetT3Pos[tag]->CommaText;
    sLinkedList->CommaText=sBinTrayLinked[tag]->CommaText;

    int iadd=0;
    for(int i=0; i<eTrayCount; i++)
    {
        if(sLinkedList->Strings[i]!=0)
        {
            Str="";
            for(int j=i; j<eTrayCount; j++)
            {
                 if(sLinkedList->Strings[j]!=0)
                 {
                    if(j+1>=eTrayCount || sLinkedList->Strings[j+1]==0)
                    {
                        Str=Str+IntToStr(j+1)+";";
                        asBinLinked[i]=Str;
                        i=j;
                        break;
                    }
                    else
                    {
                        Str=Str+IntToStr(j+1)+",";
                    }
                 }
                 else
                 {
                    asBinLinked[i]=Str;
                    i=j;
                    break;
                 }
            }
        }
    }

    Str="";
    for(int i=0; i<sList->Count; i++)
    {
        if(sList->Strings[i]!=0)
        {
            iadd=StrToInt(sList->Strings[i]);

            if(iadd>=eTrayCount)
            {
                Str=Str+"0;";
            }
            else if(sLinkedList->Strings[iadd]!=0)
            {
                Str=Str+asBinLinked[iadd];
            }
            else
            {
                Str=Str+"0;";
            }
        }
        else
        {
            Str=Str+"0;";
        }
    }
    sBinLinked[tag]->CommaText          =Str;                                   //Ifor 20240730 add : BinTrayLinked

    sList->Clear();                                                             //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete sList;
    sList=NULL;                                                                 //kevin 20161108
    sLinkedList->Clear();
    delete sLinkedList;
    sLinkedList=NULL;//kevin 20161108
}

// =============================================================================
// FW-BINSEL-W20 -- TfBinSel 的顯示/設定側方法（15 支）
//
// 選批依據（tools/wavescan 五步）：
//   1. header（forms/fBinSel.h）的 WAVE A/B/C banner 與 GATE REGISTER 先讀過，
//      本檔沒有整段封殺的排除區，剩下的是逐支各有理由。
//   2. survey（區塊註解感知 + 同類別跨檔感知）：真正缺 45 支 / 1,673 行。
//   3. screen_methods（含跟進自由函式的 deep pass）：乾淨 ∩ 真正缺 = 21 支 / 861 行。
//   4/5. 取其中 <=48 行的 16 支，再剔除 FormShortCut（golden :6363，簽章要
//        `TWMKey &Msg`，本樹零 port，且 Msg.CharCode 有被讀 -> 不是可丟的參數
//        ——與 forms/fSetup.h:120-123、forms/fTemp_Set.h:82 同一個已知缺口）。
//      -> 本波 15 支 / 297 行。
//
// 這一批全是「讀狀態 -> 設 widget 的 Enabled/Caption/Color」與小型判斷述詞，
// 不觸及機台動作、不寫檔、不送命令。
// =============================================================================
// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:2805-2838, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
bool TfBinSel::bCheckTrayCanUse(int iT6Tray)
{
    if(iT6Tray<0 || iT6Tray>=eTrayCount)                                        //保護避免超出陣列
        return false;

    if(Prod.iTrayType[iT6Tray]==tNotUse)
        return false;

    if(CosFunction.bLoaderTrayToAuto1 && TrayForm.LoaderToEmptyColor[iRunStartMode]==2)
    {
        if(iT6Tray==eAuto1)
            return false;
    }

    if(AUTO3_IS_MAGAZINE==1)                                                    //JerryYang 20221215 : add Magazine
    {
        if(iT6Tray==iMagAtAuto)
            return false;

        if(TestIF_File.iMagFixTrayType==1)                                      //使用FIX當buffer
        {
            if(iT6Tray>=iFixMin && iT6Tray<=iFixRightHalf)
                return false;
        }
    }

    if(TestIF_File.bEnableQASampling &&                                         //Steven 20190326 : QA Sampling
       (iT6Tray-1)==TestIF_File.iQASamplingT3Pos)
    {
        return false;
    }

    return true;
}

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:3333-3380, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
bool TfBinSel::SeteConsFail(int X, int tag)
{
    static bool bEnable=true;
    if(X>=0)
    {
        for(int j=eBinSetting; j<eBinSetTotal; j++)
        {
            if(MyBinPanel[tag]->BackT6PosTray[X][j]==1)
            {
                if(MyBinPanel[tag]->iT6IsFail[j-eBinSetting]>0)
                {
                    if(CUSTOMER_CODE==CC_KYEC_LEE &&                            //wei 20160418 強制打開continue fial
                       bEnablePEModel==false)                                   //Ifor 20160825 add PE模式可修改
                        MyBinPanel[tag]->bConFail[X]=true;
                    else
                        MyBinPanel[tag]->bConFail[X]=!MyBinPanel[tag]->bConFail[X];
                }
                else
                {
                    MyBinPanel[tag]->bConFail[X]=false;
                }
            }
        }
    }
    else
    {
        for(int i=0; i<iTestBinCount; i++)
        {
            for(int j=eBinSetting; j<eBinSetTotal; j++)
            {
                if(MyBinPanel[tag]->BackT6PosTray[i][j]==1)
                {
                    if(MyBinPanel[tag]->iT6IsFail[j-eBinSetting]>0)
                    {
                        MyBinPanel[tag]->bConFail[i]=bEnable;
                    }
                    else
                    {
                        MyBinPanel[tag]->bConFail[i]=false;
                    }
                }
            }
        }
        bEnable=false;
    }

    return true;
}

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:3655-3664, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
bool TfBinSel::SeteLowYield(int X, int tag)
{
    if(X>=0)
    {
        if(MyBinPanel[tag]->BackT6PosTray[X][eBinNotUse]==0)
            MyBinPanel[tag]->bLowYield[X]=!MyBinPanel[tag]->bLowYield[X];
    }

    return true;
}

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:3666-3675, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
bool TfBinSel::SeteArmYield(int X, int tag)
{
    if(X>=0)
    {
        if(MyBinPanel[tag]->BackT6PosTray[X][eBinNotUse]==0)
            MyBinPanel[tag]->bArmYield[X]=!MyBinPanel[tag]->bArmYield[X];
    }

    return true;
}

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:3677-3686, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
bool TfBinSel::SeteSiteYield(int X, int tag)
{
    if(X>=0)
    {
        if(MyBinPanel[tag]->BackT6PosTray[X][eBinNotUse]==0)
            MyBinPanel[tag]->bSiteYield[X]=!MyBinPanel[tag]->bSiteYield[X];
    }

    return true;
}

// AI(W906-FW-BINSEL-W20) 20260826: mtBinSelectMouseMove（golden :3834-3858）本波不翻。
// 它的本體走 `MyBinPanel[tag]->mtBinSelect`，而那正是本檔 header
// （forms/fBinSel.h 的 WAVE C banner）已經登記的 **GATE G10**：
// 「G10 opens for `MyBinPanel[tag]->mtBinSelect` (a THIRD, still-absent
//   Tray256Core*, InitDataToEdit's own remaining gap)」。
// 同時它還需要 `TTMyTray` 與 `bMouseDown`，兩者都是同一個未建的 tray 物件家族。
// 這不是簽章問題，是缺一個資料結構——留給處理 G10 的那一波。

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:3934-3971, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfBinSel::SetBinTray(int tag)                                   // set tray data //
{
    Change();
    for(int i=iStartX; i<=iEndX; i++)
    {
        if(CosFunction.bBin1CanNotInFix && tag!=OffT)                           //Steven 20150427 : Bin1不能放到Fix盤
        {
            if(iStartX==1 && iStartY>=eBinSetting+ePosFix1-1)
            {
                iStartY=eBinNotUse;
            }
        }
        MyBinPanel[tag]->BackT6PosTray[i][iStartY]=1;

        for(int j=eBinNotUse; j<eBinSetTotal; j++)
        {
            if(j==iStartY)                                                      //如果是同一ROW
            {
            }
            else
            {
                if(MyBinPanel[tag]->BackT6PosTray[i][iStartY]==1)               //如果選定的那一個是True, 同一Col的其他ROW都要變成False
                    MyBinPanel[tag]->BackT6PosTray[i][j]=0;
            }

            if(MyBinPanel[tag]->BackT6PosTray[i][j]==1)
            {
                if(j!=eBinNotUse &&                                             //Steven 20240814 : 針對No Use要直接false
                   MyBinPanel[tag]->iT6IsFail[j-eBinSetting]>0)                 //ChungHung 20140414 選擇fail bin 預設 continue fial 為 true
                    MyBinPanel[tag]->bConFail[i]=true;
                else
                    MyBinPanel[tag]->bConFail[i]=false;
            }
        }
    }

    InitDataToEdit(tag);
}

// AI(W906-FW-BINSEL-W20) 20260826: ShowBinTray（golden :3973-4014）本波不翻。
// 它的整個迴圈都在讀 `MyBinPanel[tag]->mtBinSelect->XItem/YItem` 與該物件的
// 格子內容——那正是 forms/fBinSel.h 的 WAVE C banner 已登記的 **GATE G10**
// （「a THIRD, still-absent Tray256Core*」）。拿掉那些讀取之後這支不剩什麼，
// 所以整支留給處理 G10 的那一波，不做成一個空殼。

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:4016-4031, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfBinSel::Change()                                              // check change pos //
{
    int iSX, iEX;
    if(iStartX>iEndX)
    {
        iSX=iEndX;
        iEX=iStartX;
    }
    else
    {
        iSX=iStartX;
        iEX=iEndX;
    }
    iStartX=iSX;
    iEndX=iEX;
}

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:6076-6086, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfBinSel::CancelErrorBinClick(TObject *Sender)
{
    for(int tag=0; tag<eBinTypeTotal; tag++)
    {
        if(CancelErrorBin->Checked==false)                                      //kevin 20160819 error bin 要放到 Bin Box //kevin 20160724 取消 ERROR BIN
        {
            MyBinPanel[tag]->iErrorT6=iFixRight;
            BinSelect[tag].IfErrorT3 =iTo6Unload[iFixRight];
        }
    }
}

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:6088-6098, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfBinSel::rg_FixBinBoxClick(TObject *Sender)
{
    for(int tag=0; tag<eBinTypeTotal; tag++)
    {
        if(rg_FixBinBox->ItemIndex==0)                                          //kevin 20160819 error bin 要放到 Bin Box //kevin 20160724 取消 ERROR BIN
        {
            MyBinPanel[tag]->iErrorT6=iFixRight;
            BinSelect[tag].IfErrorT3 =iTo6Unload[iFixRight];
        }
    }
}

// AI(W906-FW-BINSEL-W20) 20260826: sgSpecificBinMouseDown（golden :6100-6116）本波不翻。
// 它的第一行就是 `sgSpecificBin->MouseToCell(X, Y, Column, Row)`，而
// vclcompat::TStringGrid **刻意不模型化滑鼠/繪圖面**（該 facade header 自己
// 就是這樣寫的；uTemp_Set.cpp 的 GATE(G-Grid) 為了同一個方法名做過同樣的判斷）。
// 沒有 MouseToCell 就算不出 Column/Row，本體其餘每一行都靠那兩個值。

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:6118-6131, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfBinSel::btnSettingSpecificBinClick(TObject *Sender)
{
    if(palSpecificBin->Visible==false)                                          //JerryYang 20170301 (wei) specific bin
    {
        palSpecificBin->Visible=true;
    #if 0 // GATE (W20-Parent) -- golden 原文保留
    // `vclcompat::TControl` 沒有 `Parent`（TWinControl 控制項樹本樹未模型化，
    // 同 cConfiguration 的 GATE (W5-CCE) 與 cObserver 的 GATE (W7-Search)）。
    // 這一行是「把設定面板掛到 PageControl1 底下」，純版面；同方法其餘各行
    // （Visible/Left/Top/Caption 等）都保持 live，所以面板的狀態仍照 golden 設定，
    // 只是沒有被 reparent。方向是收窄。
    palSpecificBin->Parent=PageControl1;
#endif
        palSpecificBin->BringToFront();
        palSpecificBin->Top=625;                                                //JerryYang 20220923 : 650->625
    }
    else
    {
        palSpecificBin->Visible=false;
    }
}

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:6291-6294, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfBinSel::palSpecificBinClick(TObject *Sender)
{
    palSpecificBin->Visible=false;                                              //JerryYang 20170301 (wei) 存檔後不顯示
}

// AI(W906-FW-BINSEL-W20) 20260826: golden cBinSel.cpp:6372-6378, transcribed VERBATIM
// (cp950 -> UTF-8) unless a deviation is marked inline.
void TfBinSel::btnSetAll2NotUseClick(TObject *Sender)
{
    iStartX=0;                                                                  //JerryYang 20221215 : 一鍵把所有bin設定成Not use
    iEndX=iTestBinCount;
    iStartY=eBinNotUse;
    SetBinTray(iTestRunMode);
}
