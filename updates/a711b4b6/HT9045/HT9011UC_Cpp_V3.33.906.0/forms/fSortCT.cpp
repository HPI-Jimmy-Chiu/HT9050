// =============================================================================
//  forms/fSortCT.cpp  --  definitions for the fSortCT facade
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.cpp by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Body moved VERBATIM.
//
//  Steven 20260925 (Data.SortCT)：golden V912 cSortCT.cpp 裡只碰 ht9045_globals 的部分落在這裡：
//    建構子（:89-167）、_MyCountPanel（:39-87）、FormClose（:169-172）、FormShow（:174-208）、
//    UpForm（:712-815）、lblTotalClick（:1883-1886），加上四個跳板（ShowLoadingIC／ShowLoadingIC_ART／
//    ShowSortIC／btnClearCountClick → 根目錄 cSortCT.cpp 的 golden 本體）。分工理由見 forms/fSortCT.h 檔頭。
//  沒翻（理由逐條）：
//    * pnlLoaderMouseDown／pnlAuto1..3YieldMouseDown／pnlFix1..6*MouseDown／pnlMag1*MouseDown／pnlHP1/2MouseDown
//      （:444-583、:1822-1838、:1858-1931）：右鍵 EditTray 開盤面編輯視窗 —— 另一個表單（uTrayEditForm），不是這頁的資料。
//    * pnlAuto1DblClick（:1933-1950）：IniConfig.bO22_ClearSortCntByDoubleClick 才有的「點兩下清單站數量」，會寫 lastdata.dat；
//      網頁沒有雙擊通道，先不接（O22 預設關）。pnlAuto1CIDDblClick（:1952-1955）：NFC 機型的小鍵盤。
//      AI(W906-FRW-S65) 20260926: pnlAuto1DblClick 已翻（:1933-1950；D:\HT9045_ref 那份是 :1925-1942）—— 本體在根目錄 cSortCT.cpp 檔尾
//      W906_SortCTDblClickRun（自由函式，不加 TfSortCT 方法、不動本標頭）；網頁入口 WebSortCT.cpp（act.sortCT.clearCount
//      value={"op":"dblClick","panel":…}）。pnlAuto1CIDDblClick 仍沒翻。
//    * spbClearAllCountClick（:817-867）／Timer1Timer（:869-884）：gbLotID 只在 CC_ASE_M 顯示（:179-183）—— 客戶專屬，跳過。
//      AI(W906-FRW-S97) 20260926: Timer1Timer 已翻（RULINGS_20260926 S97）—— 本體在根目錄 cSortCT.cpp 檔尾 W906_SortCTTimer1Run，
//      網頁入口 WebSortCT.cpp（act.sortCT.clearCount value={"op":"lotId","text":…}；每拍 W906_SortCTTimer1Tick）。spbClearAllCountClick 仍沒翻。
//    * Timer2Timer（:886-901）：iHWFix_BinBox／bCancelErrorBin 變動時重跑 UpForm；網頁端每拍都呼叫 UpForm（WebBridgeTags.cpp 檔尾），
//      效果涵蓋它，所以不另外翻。
//    * IntervalTotalYieldDifference／ClearIntervalTotalYieldDifferenceCount／Alarm4Yield／Alarm5Yield／CheckTheYieldAfterPlaceAuto
//      （:903-1856）：生產中的良率告警，不是顯示／清除，不在本波範圍（asortarm.cpp GATE(11) 仍擋著 CheckTheYieldAfterPlaceAuto）。
// =============================================================================
#include "forms/fSortCT.h"

#include "MachineType.h"    // Steven 20260925 (Data.SortCT)：eTrayCount／e6TrayName／MAX_AUTO_TRAY／CC_*／eartInstall／tCID_NFC／eBtnAOI_TopBottomInstall
#include "cmydef.h"         // AUTO_EMPTY_COLOR／AUTO3_IS_MAGAZINE／iHWFix_BinBox／USE_Scanner_AOI_Inspection／USE_COVER_TRAYID／USE_AUTO_RETEST／bAutoReTest_ART／CUSTOMER_CODE
#include "cprod.h"          // TrayForm（iFixTrayMode／bTrayUpDownSet）
#include "Config.h"         // IniConfig（sLotID／bA10_AutoReTest）
#include "CosFunction.h"    // CosFunction（bUseTrayUpDownSet／bUseSCKART／bUseARTSortCount／bShowHPICCount）

// forms/fSortCT.h 用字面值 33／6（forms/ 標頭只 include FormWidgets.h，見 pnlTrayID[6] 的註解）；這裡對回 golden 的常數。
static_assert(eTrayCount == 33,   "forms/fSortCT.h myCountPanel[33]/TrayCTPanel[33] must equal golden eTrayCount (MachineType.h e6TrayName)");
static_assert(MAX_AUTO_TRAY == 6, "forms/fSortCT.h pnlTrayCnt[6]/pnlTrayID[6] must equal golden MAX_AUTO_TRAY");

// --- W6.2: TfSortCT --------------------------------------------------------
TfSortCT::TfSortCT()
{
    pnlHP1 = new TfSortCTPanel();
    pnlHP2 = new TfSortCTPanel();
    // -- W6.3 ADD --
    pnlLoad       = new TfSortCTPanel();
    pnlLoadCID    = new TfSortCTPanel();
    pnlCoverTrayD = new TfSortCTPanel();
    for(int i=0;i<6;i++) pnlTrayCnt[i] = new TfSortCTPanel();
    // AI(W906-W7-L1-Wave0) 20260801: golden cSortCT.h:338 pnlTrayID[MAX_AUTO_TRAY]
    // (MAX_AUTO_TRAY==6, golden MachineType.h:396) -- read by asendic_Auto.cpp:1156.
    for(int iW0=0;iW0<6;iW0++) pnlTrayID[iW0] = new TfSortCTPanel();

    // -----------------------------------------------------------------------
    //  Steven 20260925 (Data.SortCT)：其餘元件（golden 由 .dfm 建立；這裡照 dfm 的 Caption／Visible／版面值）。
    //  ⚠ vclcompat::TControl 預設 Visible=false（Controls.h:256），dfm 元件預設是可見 —— 所以每個都明寫 Visible=true，
    //    dfm 明寫 Visible=False 的（gbLotID :2058）才是 false。
    // -----------------------------------------------------------------------
    bShow = false;                                // golden :136（VCL 清零後再設一次）
    iVisibleHeight = 0;                           // VCL TObject 清零的對應
    Width  = 484;                                 // dfm ClientWidth（:8）
    Height = 1002;                                // dfm ClientHeight（:7）
    PageControl1 = new TfSortCTPageControl(); PageControl1->Visible = true;                                           // dfm:21
    SortCount    = new TTabSheet(); SortCount->Visible = true;    SortCount->TabVisible = true;    SortCount->Caption    = "SortCount";      // dfm:31
    ARTSortCount = new TTabSheet(); ARTSortCount->Visible = true; ARTSortCount->TabVisible = true; ARTSortCount->Caption = "ARTSortCount";   // dfm:2135（TabVisible 由 FormShow 決定）
    tsICCount    = new TTabSheet(); tsICCount->Visible = true;    tsICCount->TabVisible = true;    tsICCount->Caption    = "IC Count";       // dfm:3827（TabVisible 由 FormShow 決定）
    PageControl1->ActivePage = SortCount;         // dfm:26 ActivePage = SortCount
    pnlLoader     = new TfSortCTPanel(); pnlLoader->Visible = true; pnlLoader->Caption = "100";                        // dfm:54
    lblTotal      = new TLabel();        lblTotal->Visible = true;  lblTotal->Caption = "Total";                       // dfm:119
    pnlTotal      = new TfSortCTPanel(); pnlTotal->Visible = true;  pnlTotal->Caption = "100";                         // dfm:133
    pnlYield      = new TfSortCTPanel(); pnlYield->Visible = true;                                                     // dfm:151（無 Caption 行 = ""；Visible 由 uYieldMonitoring ReadFile :1726 決定）
    pnlUnloadBG   = new TfSortCTBgPanel(); pnlUnloadBG->Visible = true; pnlUnloadBG->Top = 72; pnlUnloadBG->Height = 849;   // dfm:188
    gbLotID       = new TfSortCTGroupBox(); gbLotID->Visible = false; gbLotID->Caption = "Lot ID"; gbLotID->Top = 921; gbLotID->Height = 86;   // dfm:2058
    spbClearAllCount = new TfSortCTSpeedButton(); spbClearAllCount->Visible = true; spbClearAllCount->Caption = "Clear All Count"; spbClearAllCount->Top = 51; spbClearAllCount->Height = 26;   // dfm:2073
    edLotID       = new TEdit();         edLotID->Visible = true;                                                      // dfm:2126
    pnlLoadingART = new TfSortCTPanel(); pnlLoadingART->Visible = true;                                                // dfm:2159
    pnlTotalART   = new TfSortCTPanel(); pnlTotalART->Visible = true;                                                  // dfm:2199
    pnlYieldART   = new TfSortCTPanel(); pnlYieldART->Visible = true;                                                  // dfm:2216
    btnClearCount = new TfSortCTSpeedButton(); btnClearCount->Visible = true; btnClearCount->Caption = "Clear Count"; btnClearCount->Left = 16; btnClearCount->Top = 801; btnClearCount->Height = 23;   // dfm:625
    pnlUnloadingARTBG = new TfSortCTBgPanel(); pnlUnloadingARTBG->Visible = true; pnlUnloadingARTBG->Top = 72; pnlUnloadingARTBG->Height = 801;   // dfm:2234
    pnlLoadTrayCt = new TfSortCTPanel(); pnlLoadTrayCt->Visible = true;                                                // dfm:73
    pnlTrayCount  = new TfSortCTPanel(); pnlTrayCount->Visible = true; pnlTrayCount->Caption = "Tray Count";          // dfm:3934
    lblAuto1 = new TLabel(); lblAuto1->Visible = true; lblAuto1->Caption = "Auto 1";    // dfm:196
    pnlAuto1Yield = new TfSortCTPanel(); pnlAuto1Yield->Visible = true;          // dfm:678 (no Caption line = "")
    pnlAuto1 = new TfSortCTPanel(); pnlAuto1->Visible = true; pnlAuto1->Caption = "100";       // dfm:696
    lblAuto2 = new TLabel(); lblAuto2->Visible = true; lblAuto2->Caption = "Auto 2";    // dfm:209
    pnlAuto2Yield = new TfSortCTPanel(); pnlAuto2Yield->Visible = true;          // dfm:716 (no Caption line = "")
    pnlAuto2 = new TfSortCTPanel(); pnlAuto2->Visible = true; pnlAuto2->Caption = "100";       // dfm:733
    lblAuto3 = new TLabel(); lblAuto3->Visible = true; lblAuto3->Caption = "Auto 3";    // dfm:222
    pnlAuto3Yield = new TfSortCTPanel(); pnlAuto3Yield->Visible = true;          // dfm:751 (no Caption line = "")
    pnlAuto3 = new TfSortCTPanel(); pnlAuto3->Visible = true; pnlAuto3->Caption = "100";       // dfm:768
    lblAuto4 = new TLabel(); lblAuto4->Visible = true; lblAuto4->Caption = "Auto 4";    // dfm:235
    pnlAuto4Yield = new TfSortCTPanel(); pnlAuto4Yield->Visible = true;          // dfm:786 (no Caption line = "")
    pnlAuto4 = new TfSortCTPanel(); pnlAuto4->Visible = true; pnlAuto4->Caption = "100";       // dfm:803
    lblAuto5 = new TLabel(); lblAuto5->Visible = true; lblAuto5->Caption = "Auto 5";    // dfm:248
    pnlAuto5Yield = new TfSortCTPanel(); pnlAuto5Yield->Visible = true;          // dfm:821 (no Caption line = "")
    pnlAuto5 = new TfSortCTPanel(); pnlAuto5->Visible = true; pnlAuto5->Caption = "100";       // dfm:838
    lblAuto6 = new TLabel(); lblAuto6->Visible = true; lblAuto6->Caption = "Auto 6";    // dfm:261
    pnlAuto6Yield = new TfSortCTPanel(); pnlAuto6Yield->Visible = true;          // dfm:856 (no Caption line = "")
    pnlAuto6 = new TfSortCTPanel(); pnlAuto6->Visible = true; pnlAuto6->Caption = "100";       // dfm:873
    lblFix1 = new TLabel(); lblFix1->Visible = true; lblFix1->Caption = "Fix 1";      // dfm:274
    pnlFix1Yield = new TfSortCTPanel(); pnlFix1Yield->Visible = true;            // dfm:891 (no Caption line = "")
    pnlFix1 = new TfSortCTPanel(); pnlFix1->Visible = true;                      // dfm:909 (no Caption line = "")
    lblFix2 = new TLabel(); lblFix2->Visible = true; lblFix2->Caption = "Fix 2";      // dfm:287
    pnlFix2Yield = new TfSortCTPanel(); pnlFix2Yield->Visible = true;            // dfm:926 (no Caption line = "")
    pnlFix2 = new TfSortCTPanel(); pnlFix2->Visible = true;                      // dfm:944 (no Caption line = "")
    lblFix4 = new TLabel(); lblFix4->Visible = true; lblFix4->Caption = "Fix 4";      // dfm:300
    pnlFix4Yield = new TfSortCTPanel(); pnlFix4Yield->Visible = true;            // dfm:961 (no Caption line = "")
    pnlFix4 = new TfSortCTPanel(); pnlFix4->Visible = true;                      // dfm:979 (no Caption line = "")
    lblFix3 = new TLabel(); lblFix3->Visible = true; lblFix3->Caption = "Fix 3";      // dfm:313
    pnlFix3Yield = new TfSortCTPanel(); pnlFix3Yield->Visible = true;            // dfm:996 (no Caption line = "")
    pnlFix3 = new TfSortCTPanel(); pnlFix3->Visible = true;                      // dfm:1014 (no Caption line = "")
    lblFix5 = new TLabel(); lblFix5->Visible = true; lblFix5->Caption = "Fix 5";      // dfm:326
    pnlFix5Yield = new TfSortCTPanel(); pnlFix5Yield->Visible = true;            // dfm:1031 (no Caption line = "")
    pnlFix5 = new TfSortCTPanel(); pnlFix5->Visible = true;                      // dfm:1049 (no Caption line = "")
    lblFix6 = new TLabel(); lblFix6->Visible = true; lblFix6->Caption = "Fix 6";      // dfm:339
    pnlFix6Yield = new TfSortCTPanel(); pnlFix6Yield->Visible = true;            // dfm:1066 (no Caption line = "")
    pnlFix6 = new TfSortCTPanel(); pnlFix6->Visible = true;                      // dfm:1084 (no Caption line = "")
    lblFix7 = new TLabel(); lblFix7->Visible = true; lblFix7->Caption = "Fix 7";      // dfm:352
    pnlFix7Yield = new TfSortCTPanel(); pnlFix7Yield->Visible = true;            // dfm:1101 (no Caption line = "")
    pnlFix7 = new TfSortCTPanel(); pnlFix7->Visible = true;                      // dfm:1118 (no Caption line = "")
    lblFix8 = new TLabel(); lblFix8->Visible = true; lblFix8->Caption = "Fix 8";      // dfm:365
    pnlFix8Yield = new TfSortCTPanel(); pnlFix8Yield->Visible = true;            // dfm:1135 (no Caption line = "")
    pnlFix8 = new TfSortCTPanel(); pnlFix8->Visible = true;                      // dfm:1152 (no Caption line = "")
    lblFix9 = new TLabel(); lblFix9->Visible = true; lblFix9->Caption = "Fix 9";      // dfm:378
    pnlFix9Yield = new TfSortCTPanel(); pnlFix9Yield->Visible = true;            // dfm:1169 (no Caption line = "")
    pnlFix9 = new TfSortCTPanel(); pnlFix9->Visible = true;                      // dfm:1186 (no Caption line = "")
    lblFix10 = new TLabel(); lblFix10->Visible = true; lblFix10->Caption = "Fix 10";    // dfm:391
    pnlFix10Yield = new TfSortCTPanel(); pnlFix10Yield->Visible = true;          // dfm:1203 (no Caption line = "")
    pnlFix10 = new TfSortCTPanel(); pnlFix10->Visible = true;                    // dfm:1220 (no Caption line = "")
    lblFix11 = new TLabel(); lblFix11->Visible = true; lblFix11->Caption = "Fix 11";    // dfm:404
    pnlFix11Yield = new TfSortCTPanel(); pnlFix11Yield->Visible = true;          // dfm:1237 (no Caption line = "")
    pnlFix11 = new TfSortCTPanel(); pnlFix11->Visible = true;                    // dfm:1254 (no Caption line = "")
    lblFix12 = new TLabel(); lblFix12->Visible = true; lblFix12->Caption = "Fix 12";    // dfm:417
    pnlFix12Yield = new TfSortCTPanel(); pnlFix12Yield->Visible = true;          // dfm:1271 (no Caption line = "")
    pnlFix12 = new TfSortCTPanel(); pnlFix12->Visible = true;                    // dfm:1288 (no Caption line = "")
    lblBinBox = new TLabel(); lblBinBox->Visible = true; lblBinBox->Caption = "Bulk Box"; // dfm:430
    pnlBinBoxYield = new TfSortCTPanel(); pnlBinBoxYield->Visible = true;        // dfm:1305 (no Caption line = "")
    pnlBinBox = new TfSortCTPanel(); pnlBinBox->Visible = true;                  // dfm:1322 (no Caption line = "")
    lblMag1 = new TLabel(); lblMag1->Visible = true; lblMag1->Caption = "Mag 1";      // dfm:443
    pnlMag1Yield = new TfSortCTPanel(); pnlMag1Yield->Visible = true;            // dfm:1339 (no Caption line = "")
    pnlMag1 = new TfSortCTPanel(); pnlMag1->Visible = true;                      // dfm:1356 (no Caption line = "")
    lblMag2 = new TLabel(); lblMag2->Visible = true; lblMag2->Caption = "Mag 2";      // dfm:456
    pnlMag2Yield = new TfSortCTPanel(); pnlMag2Yield->Visible = true;            // dfm:1373 (no Caption line = "")
    pnlMag2 = new TfSortCTPanel(); pnlMag2->Visible = true;                      // dfm:1391 (no Caption line = "")
    lblMag3 = new TLabel(); lblMag3->Visible = true; lblMag3->Caption = "Mag 3";      // dfm:469
    pnlMag3Yield = new TfSortCTPanel(); pnlMag3Yield->Visible = true;            // dfm:1409 (no Caption line = "")
    pnlMag3 = new TfSortCTPanel(); pnlMag3->Visible = true;                      // dfm:1427 (no Caption line = "")
    lblMag4 = new TLabel(); lblMag4->Visible = true; lblMag4->Caption = "Mag 4";      // dfm:482
    pnlMag4Yield = new TfSortCTPanel(); pnlMag4Yield->Visible = true;            // dfm:1445 (no Caption line = "")
    pnlMag4 = new TfSortCTPanel(); pnlMag4->Visible = true;                      // dfm:1463 (no Caption line = "")
    lblMag5 = new TLabel(); lblMag5->Visible = true; lblMag5->Caption = "Mag 5";      // dfm:495
    pnlMag5Yield = new TfSortCTPanel(); pnlMag5Yield->Visible = true;            // dfm:1481 (no Caption line = "")
    pnlMag5 = new TfSortCTPanel(); pnlMag5->Visible = true;                      // dfm:1499 (no Caption line = "")
    lblMag6 = new TLabel(); lblMag6->Visible = true; lblMag6->Caption = "Mag 6";      // dfm:508
    pnlMag6Yield = new TfSortCTPanel(); pnlMag6Yield->Visible = true;            // dfm:1517 (no Caption line = "")
    pnlMag6 = new TfSortCTPanel(); pnlMag6->Visible = true;                      // dfm:1535 (no Caption line = "")
    lblMag7 = new TLabel(); lblMag7->Visible = true; lblMag7->Caption = "Mag 7";      // dfm:521
    pnlMag7Yield = new TfSortCTPanel(); pnlMag7Yield->Visible = true;            // dfm:1553 (no Caption line = "")
    pnlMag7 = new TfSortCTPanel(); pnlMag7->Visible = true;                      // dfm:1571 (no Caption line = "")
    lblMag8 = new TLabel(); lblMag8->Visible = true; lblMag8->Caption = "Mag 8";      // dfm:534
    pnlMag8Yield = new TfSortCTPanel(); pnlMag8Yield->Visible = true;            // dfm:1589 (no Caption line = "")
    pnlMag8 = new TfSortCTPanel(); pnlMag8->Visible = true;                      // dfm:1607 (no Caption line = "")
    lblMag9 = new TLabel(); lblMag9->Visible = true; lblMag9->Caption = "Mag 9";      // dfm:547
    pnlMag9Yield = new TfSortCTPanel(); pnlMag9Yield->Visible = true;            // dfm:1625 (no Caption line = "")
    pnlMag9 = new TfSortCTPanel(); pnlMag9->Visible = true;                      // dfm:1643 (no Caption line = "")
    lblMag10 = new TLabel(); lblMag10->Visible = true; lblMag10->Caption = "Mag 10";    // dfm:560
    pnlMag10Yield = new TfSortCTPanel(); pnlMag10Yield->Visible = true;          // dfm:1661 (no Caption line = "")
    pnlMag10 = new TfSortCTPanel(); pnlMag10->Visible = true;                    // dfm:1679 (no Caption line = "")
    lblMag11 = new TLabel(); lblMag11->Visible = true; lblMag11->Caption = "Mag 11";    // dfm:573
    pnlMag11Yield = new TfSortCTPanel(); pnlMag11Yield->Visible = true;          // dfm:1697 (no Caption line = "")
    pnlMag11 = new TfSortCTPanel(); pnlMag11->Visible = true;                    // dfm:1715 (no Caption line = "")
    lblMag12 = new TLabel(); lblMag12->Visible = true; lblMag12->Caption = "Mag 12";    // dfm:586
    pnlMag12Yield = new TfSortCTPanel(); pnlMag12Yield->Visible = true;          // dfm:1733 (no Caption line = "")
    pnlMag12 = new TfSortCTPanel(); pnlMag12->Visible = true;                    // dfm:1751 (no Caption line = "")
    lblMag13 = new TLabel(); lblMag13->Visible = true; lblMag13->Caption = "Mag 13";    // dfm:599
    pnlMag13Yield = new TfSortCTPanel(); pnlMag13Yield->Visible = true;          // dfm:1769 (no Caption line = "")
    pnlMag13 = new TfSortCTPanel(); pnlMag13->Visible = true;                    // dfm:1787 (no Caption line = "")
    lblMag14 = new TLabel(); lblMag14->Visible = true; lblMag14->Caption = "Mag 14";    // dfm:612
    pnlMag14Yield = new TfSortCTPanel(); pnlMag14Yield->Visible = true;          // dfm:1805 (no Caption line = "")
    pnlMag14 = new TfSortCTPanel(); pnlMag14->Visible = true;                    // dfm:1823 (no Caption line = "")
    lblARTMag14 = new TLabel(); lblARTMag14->Visible = true; lblARTMag14->Caption = "Mag 14"; // dfm:2242
    pnARTMag14Yield = new TfSortCTPanel(); pnARTMag14Yield->Visible = true;      // dfm:2671 (no Caption line = "")
    pnARTMag14 = new TfSortCTPanel(); pnARTMag14->Visible = true;                // dfm:2689 (no Caption line = "")
    lblARTMag13 = new TLabel(); lblARTMag13->Visible = true; lblARTMag13->Caption = "Mag 13"; // dfm:2255
    pnARTMag13Yield = new TfSortCTPanel(); pnARTMag13Yield->Visible = true;      // dfm:2707 (no Caption line = "")
    pnARTMag13 = new TfSortCTPanel(); pnARTMag13->Visible = true;                // dfm:2725 (no Caption line = "")
    lblARTMag12 = new TLabel(); lblARTMag12->Visible = true; lblARTMag12->Caption = "Mag 12"; // dfm:2268
    pnARTMag12Yield = new TfSortCTPanel(); pnARTMag12Yield->Visible = true;      // dfm:2743 (no Caption line = "")
    pnARTMag12 = new TfSortCTPanel(); pnARTMag12->Visible = true;                // dfm:2761 (no Caption line = "")
    lblARTMag11 = new TLabel(); lblARTMag11->Visible = true; lblARTMag11->Caption = "Mag 11"; // dfm:2281
    pnARTMag11Yield = new TfSortCTPanel(); pnARTMag11Yield->Visible = true;      // dfm:2779 (no Caption line = "")
    pnARTMag11 = new TfSortCTPanel(); pnARTMag11->Visible = true;                // dfm:2797 (no Caption line = "")
    lblARTMag10 = new TLabel(); lblARTMag10->Visible = true; lblARTMag10->Caption = "Mag 10"; // dfm:2294
    pnARTMag10Yield = new TfSortCTPanel(); pnARTMag10Yield->Visible = true;      // dfm:2815 (no Caption line = "")
    pnARTMag10 = new TfSortCTPanel(); pnARTMag10->Visible = true;                // dfm:2833 (no Caption line = "")
    lblARTMag9 = new TLabel(); lblARTMag9->Visible = true; lblARTMag9->Caption = "Mag 9";   // dfm:2307
    pnARTMag9Yield = new TfSortCTPanel(); pnARTMag9Yield->Visible = true;        // dfm:2851 (no Caption line = "")
    pnARTMag9 = new TfSortCTPanel(); pnARTMag9->Visible = true;                  // dfm:2869 (no Caption line = "")
    lblARTMag8 = new TLabel(); lblARTMag8->Visible = true; lblARTMag8->Caption = "Mag 8";   // dfm:2320
    pnARTMag8Yield = new TfSortCTPanel(); pnARTMag8Yield->Visible = true;        // dfm:2887 (no Caption line = "")
    pnARTMag8 = new TfSortCTPanel(); pnARTMag8->Visible = true;                  // dfm:2905 (no Caption line = "")
    pnARTMag7 = new TfSortCTPanel(); pnARTMag7->Visible = true;                  // dfm:2923 (no Caption line = "")
    pnARTMag7Yield = new TfSortCTPanel(); pnARTMag7Yield->Visible = true;        // dfm:2941 (no Caption line = "")
    lblARTMag7 = new TLabel(); lblARTMag7->Visible = true; lblARTMag7->Caption = "Mag 7";   // dfm:2333
    lblARTMag6 = new TLabel(); lblARTMag6->Visible = true; lblARTMag6->Caption = "Mag 6";   // dfm:2346
    pnARTMag6Yield = new TfSortCTPanel(); pnARTMag6Yield->Visible = true;        // dfm:2959 (no Caption line = "")
    pnARTMag6 = new TfSortCTPanel(); pnARTMag6->Visible = true;                  // dfm:2977 (no Caption line = "")
    lblARTMag5 = new TLabel(); lblARTMag5->Visible = true; lblARTMag5->Caption = "Mag 5";   // dfm:2359
    pnARTMag5Yield = new TfSortCTPanel(); pnARTMag5Yield->Visible = true;        // dfm:2995 (no Caption line = "")
    pnARTMag5 = new TfSortCTPanel(); pnARTMag5->Visible = true;                  // dfm:3013 (no Caption line = "")
    lblARTMag4 = new TLabel(); lblARTMag4->Visible = true; lblARTMag4->Caption = "Mag 4";   // dfm:2372
    pnARTMag4Yield = new TfSortCTPanel(); pnARTMag4Yield->Visible = true;        // dfm:3031 (no Caption line = "")
    pnARTMag4 = new TfSortCTPanel(); pnARTMag4->Visible = true;                  // dfm:3049 (no Caption line = "")
    lblARTMag3 = new TLabel(); lblARTMag3->Visible = true; lblARTMag3->Caption = "Mag 3";   // dfm:2385
    pnARTMag3Yield = new TfSortCTPanel(); pnARTMag3Yield->Visible = true;        // dfm:3067 (no Caption line = "")
    pnARTMag3 = new TfSortCTPanel(); pnARTMag3->Visible = true;                  // dfm:3085 (no Caption line = "")
    lblARTMag2 = new TLabel(); lblARTMag2->Visible = true; lblARTMag2->Caption = "Mag 2";   // dfm:2398
    pnARTMag2Yield = new TfSortCTPanel(); pnARTMag2Yield->Visible = true;        // dfm:3103 (no Caption line = "")
    pnARTMag2 = new TfSortCTPanel(); pnARTMag2->Visible = true;                  // dfm:3121 (no Caption line = "")
    lblARTMag1 = new TLabel(); lblARTMag1->Visible = true; lblARTMag1->Caption = "Mag 1";   // dfm:2411
    pnARTMag1Yield = new TfSortCTPanel(); pnARTMag1Yield->Visible = true;        // dfm:3139 (no Caption line = "")
    pnARTMag1 = new TfSortCTPanel(); pnARTMag1->Visible = true;                  // dfm:3156 (no Caption line = "")
    lblARTBinBox = new TLabel(); lblARTBinBox->Visible = true; lblARTBinBox->Caption = "Bulk Box"; // dfm:2424
    pnARTBinBoxYield = new TfSortCTPanel(); pnARTBinBoxYield->Visible = true;    // dfm:3173 (no Caption line = "")
    pnARTBinBox = new TfSortCTPanel(); pnARTBinBox->Visible = true;              // dfm:3190 (no Caption line = "")
    lblARTFix12 = new TLabel(); lblARTFix12->Visible = true; lblARTFix12->Caption = "Fix 12"; // dfm:2437
    pnARTFix12Yield = new TfSortCTPanel(); pnARTFix12Yield->Visible = true;      // dfm:3207 (no Caption line = "")
    pnARTFix12 = new TfSortCTPanel(); pnARTFix12->Visible = true;                // dfm:3224 (no Caption line = "")
    lblARTFix11 = new TLabel(); lblARTFix11->Visible = true; lblARTFix11->Caption = "Fix 11"; // dfm:2450
    pnARTFix11Yield = new TfSortCTPanel(); pnARTFix11Yield->Visible = true;      // dfm:3241 (no Caption line = "")
    pnARTFix11 = new TfSortCTPanel(); pnARTFix11->Visible = true;                // dfm:3258 (no Caption line = "")
    lblARTFix10 = new TLabel(); lblARTFix10->Visible = true; lblARTFix10->Caption = "Fix 10"; // dfm:2463
    pnARTFix10Yield = new TfSortCTPanel(); pnARTFix10Yield->Visible = true;      // dfm:3275 (no Caption line = "")
    pnARTFix10 = new TfSortCTPanel(); pnARTFix10->Visible = true;                // dfm:3292 (no Caption line = "")
    lblARTFix9 = new TLabel(); lblARTFix9->Visible = true; lblARTFix9->Caption = "Fix 9";   // dfm:2476
    pnARTFix9Yield = new TfSortCTPanel(); pnARTFix9Yield->Visible = true;        // dfm:3309 (no Caption line = "")
    pnARTFix9 = new TfSortCTPanel(); pnARTFix9->Visible = true;                  // dfm:3326 (no Caption line = "")
    lblARTAuto4 = new TLabel(); lblARTAuto4->Visible = true; lblARTAuto4->Caption = "Auto 4"; // dfm:2489
    pnARTAuto4Yield = new TfSortCTPanel(); pnARTAuto4Yield->Visible = true;      // dfm:3343 (no Caption line = "")
    pnARTAuto4 = new TfSortCTPanel(); pnARTAuto4->Visible = true; pnARTAuto4->Caption = "100";     // dfm:3360
    lblARTFix8 = new TLabel(); lblARTFix8->Visible = true; lblARTFix8->Caption = "Fix 8";   // dfm:2502
    pnARTFix8Yield = new TfSortCTPanel(); pnARTFix8Yield->Visible = true;        // dfm:3378 (no Caption line = "")
    pnARTFix8 = new TfSortCTPanel(); pnARTFix8->Visible = true;                  // dfm:3395 (no Caption line = "")
    lblARTFix7 = new TLabel(); lblARTFix7->Visible = true; lblARTFix7->Caption = "Fix 7";   // dfm:2515
    pnARTFix7Yield = new TfSortCTPanel(); pnARTFix7Yield->Visible = true;        // dfm:3412 (no Caption line = "")
    pnARTFix7 = new TfSortCTPanel(); pnARTFix7->Visible = true;                  // dfm:3429 (no Caption line = "")
    lblARTFix6 = new TLabel(); lblARTFix6->Visible = true; lblARTFix6->Caption = "Fix 6";   // dfm:2528
    pnARTFix6Yield = new TfSortCTPanel(); pnARTFix6Yield->Visible = true;        // dfm:3446 (no Caption line = "")
    pnARTFix6 = new TfSortCTPanel(); pnARTFix6->Visible = true;                  // dfm:3463 (no Caption line = "")
    lblARTFix5 = new TLabel(); lblARTFix5->Visible = true; lblARTFix5->Caption = "Fix 5";   // dfm:2541
    pnARTFix5Yield = new TfSortCTPanel(); pnARTFix5Yield->Visible = true;        // dfm:3480 (no Caption line = "")
    pnARTFix5 = new TfSortCTPanel(); pnARTFix5->Visible = true;                  // dfm:3497 (no Caption line = "")
    lblARTFix4 = new TLabel(); lblARTFix4->Visible = true; lblARTFix4->Caption = "Fix 4";   // dfm:2554
    pnARTFix4Yield = new TfSortCTPanel(); pnARTFix4Yield->Visible = true;        // dfm:3514 (no Caption line = "")
    pnARTFix4 = new TfSortCTPanel(); pnARTFix4->Visible = true;                  // dfm:3531 (no Caption line = "")
    lblARTFix3 = new TLabel(); lblARTFix3->Visible = true; lblARTFix3->Caption = "Fix 3";   // dfm:2567
    pnARTFix3Yield = new TfSortCTPanel(); pnARTFix3Yield->Visible = true;        // dfm:3548 (no Caption line = "")
    pnARTFix3 = new TfSortCTPanel(); pnARTFix3->Visible = true;                  // dfm:3565 (no Caption line = "")
    lblARTFix2 = new TLabel(); lblARTFix2->Visible = true; lblARTFix2->Caption = "Fix 2";   // dfm:2580
    pnARTFix2Yield = new TfSortCTPanel(); pnARTFix2Yield->Visible = true;        // dfm:3582 (no Caption line = "")
    pnARTFix2 = new TfSortCTPanel(); pnARTFix2->Visible = true;                  // dfm:3599 (no Caption line = "")
    lblARTFix1 = new TLabel(); lblARTFix1->Visible = true; lblARTFix1->Caption = "Fix 1";   // dfm:2593
    pnARTFix1Yield = new TfSortCTPanel(); pnARTFix1Yield->Visible = true;        // dfm:3616 (no Caption line = "")
    pnARTFix1 = new TfSortCTPanel(); pnARTFix1->Visible = true;                  // dfm:3633 (no Caption line = "")
    lblARTAuto2 = new TLabel(); lblARTAuto2->Visible = true; lblARTAuto2->Caption = "Auto 2"; // dfm:2606
    pnARTAuto2Yield = new TfSortCTPanel(); pnARTAuto2Yield->Visible = true;      // dfm:3650 (no Caption line = "")
    pnARTAuto2 = new TfSortCTPanel(); pnARTAuto2->Visible = true; pnARTAuto2->Caption = "100";     // dfm:3667
    lblARTAuto1 = new TLabel(); lblARTAuto1->Visible = true; lblARTAuto1->Caption = "Auto 1"; // dfm:2619
    pnARTAuto1Yield = new TfSortCTPanel(); pnARTAuto1Yield->Visible = true;      // dfm:3685 (no Caption line = "")
    pnARTAuto1 = new TfSortCTPanel(); pnARTAuto1->Visible = true; pnARTAuto1->Caption = "100";     // dfm:3702
    lblARTAuto3 = new TLabel(); lblARTAuto3->Visible = true; lblARTAuto3->Caption = "Auto 3"; // dfm:2632
    pnARTAuto3Yield = new TfSortCTPanel(); pnARTAuto3Yield->Visible = true;      // dfm:3720 (no Caption line = "")
    pnARTAuto3 = new TfSortCTPanel(); pnARTAuto3->Visible = true; pnARTAuto3->Caption = "100";     // dfm:3737
    lblARTAuto5 = new TLabel(); lblARTAuto5->Visible = true; lblARTAuto5->Caption = "Auto 5"; // dfm:2645
    pnARTAuto5Yield = new TfSortCTPanel(); pnARTAuto5Yield->Visible = true;      // dfm:3755 (no Caption line = "")
    pnARTAuto5 = new TfSortCTPanel(); pnARTAuto5->Visible = true; pnARTAuto5->Caption = "100";     // dfm:3772
    lblARTAuto6 = new TLabel(); lblARTAuto6->Visible = true; lblARTAuto6->Caption = "Auto 6"; // dfm:2658
    pnARTAuto6Yield = new TfSortCTPanel(); pnARTAuto6Yield->Visible = true;      // dfm:3790 (no Caption line = "")
    pnARTAuto6 = new TfSortCTPanel(); pnARTAuto6->Visible = true; pnARTAuto6->Caption = "100";     // dfm:3807

    // ---- golden 建構子本體（cSortCT.cpp:92-166） ------------------------------
    myCountPanel[eAuto1     ].SetObject(pnlAuto1,  pnlAuto1Yield,  lblAuto1,  pnARTAuto1,  pnARTAuto1Yield,  lblARTAuto1,  eAuto1);   // golden cSortCT.cpp:92
    myCountPanel[eAuto2     ].SetObject(pnlAuto2,  pnlAuto2Yield,  lblAuto2,  pnARTAuto2,  pnARTAuto2Yield,  lblARTAuto2,  eAuto2);   // golden cSortCT.cpp:93
    myCountPanel[eAuto3     ].SetObject(pnlAuto3,  pnlAuto3Yield,  lblAuto3,  pnARTAuto3,  pnARTAuto3Yield,  lblARTAuto3,  eAuto3);   // golden cSortCT.cpp:94
    myCountPanel[eAuto4     ].SetObject(pnlAuto4,  pnlAuto4Yield,  lblAuto4,  pnARTAuto4,  pnARTAuto4Yield,  lblARTAuto4,  eAuto4);   // golden cSortCT.cpp:95
    myCountPanel[eAuto5     ].SetObject(pnlAuto5,  pnlAuto5Yield,  lblAuto5,  pnARTAuto5,  pnARTAuto5Yield,  lblARTAuto5,  eAuto5);   // golden cSortCT.cpp:96
    myCountPanel[eAuto6     ].SetObject(pnlAuto6,  pnlAuto6Yield,  lblAuto6,  pnARTAuto6,  pnARTAuto6Yield,  lblARTAuto6,  eAuto6);   // golden cSortCT.cpp:97
    myCountPanel[eFix1      ].SetObject(pnlFix1,   pnlFix1Yield,   lblFix1,   pnARTFix1,   pnARTFix1Yield,   lblARTFix1,   eFix1);   // golden cSortCT.cpp:98
    myCountPanel[eFix2      ].SetObject(pnlFix2,   pnlFix2Yield,   lblFix2,   pnARTFix2,   pnARTFix2Yield,   lblARTFix2,   eFix2);   // golden cSortCT.cpp:99
    myCountPanel[eFix3      ].SetObject(pnlFix3,   pnlFix3Yield,   lblFix3,   pnARTFix3,   pnARTFix3Yield,   lblARTFix3,   eFix3);   // golden cSortCT.cpp:100
    myCountPanel[eFix4      ].SetObject(pnlFix4,   pnlFix4Yield,   lblFix4,   pnARTFix4,   pnARTFix4Yield,   lblARTFix4,   eFix4);   // golden cSortCT.cpp:101
    myCountPanel[eFix5      ].SetObject(pnlFix5,   pnlFix5Yield,   lblFix5,   pnARTFix5,   pnARTFix5Yield,   lblARTFix5,   eFix5);   // golden cSortCT.cpp:102
    myCountPanel[eFix6      ].SetObject(pnlFix6,   pnlFix6Yield,   lblFix6,   pnARTFix6,   pnARTFix6Yield,   lblARTFix6,   eFix6);   // golden cSortCT.cpp:103
    myCountPanel[eFix7      ].SetObject(pnlFix7,   pnlFix7Yield,   lblFix7,   pnARTFix7,   pnARTFix7Yield,   lblARTFix7,   eFix7);   // golden cSortCT.cpp:104
    myCountPanel[eFix8      ].SetObject(pnlFix8,   pnlFix8Yield,   lblFix8,   pnARTFix8,   pnARTFix8Yield,   lblARTFix8,   eFix8);   // golden cSortCT.cpp:105
    myCountPanel[eFix9      ].SetObject(pnlFix9,   pnlFix9Yield,   lblFix9,   pnARTFix9,   pnARTFix9Yield,   lblARTFix9,   eFix9);   // golden cSortCT.cpp:106
    myCountPanel[eFix10     ].SetObject(pnlFix10,  pnlFix10Yield,  lblFix10,  pnARTFix10,  pnARTFix10Yield,  lblARTFix10,  eFix10);   // golden cSortCT.cpp:107
    myCountPanel[eFix11     ].SetObject(pnlFix11,  pnlFix11Yield,  lblFix11,  pnARTFix11,  pnARTFix11Yield,  lblARTFix11,  eFix11);   // golden cSortCT.cpp:108
    myCountPanel[eFix12     ].SetObject(pnlFix12,  pnlFix11Yield,  lblFix12,  pnARTFix12,  pnARTFix12Yield,  lblARTFix12,  eFix12);   // golden cSortCT.cpp:109
    myCountPanel[eBulkBox   ].SetObject(pnlBinBox, pnlBinBoxYield, lblBinBox, pnARTBinBox, pnARTBinBoxYield, lblARTBinBox, eBulkBox);   // golden cSortCT.cpp:110
    myCountPanel[eMag1      ].SetObject(pnlMag1,   pnlMag1Yield,   lblMag1,   pnARTMag1,   pnARTMag1Yield,   lblARTMag1,   eMag1);   // golden cSortCT.cpp:111
    myCountPanel[eMag2      ].SetObject(pnlMag2,   pnlMag2Yield,   lblMag2,   pnARTMag2,   pnARTMag2Yield,   lblARTMag2,   eMag2);   // golden cSortCT.cpp:112
    myCountPanel[eMag3      ].SetObject(pnlMag3,   pnlMag3Yield,   lblMag3,   pnARTMag3,   pnARTMag3Yield,   lblARTMag3,   eMag3);   // golden cSortCT.cpp:113
    myCountPanel[eMag4      ].SetObject(pnlMag4,   pnlMag4Yield,   lblMag4,   pnARTMag4,   pnARTMag4Yield,   lblARTMag4,   eMag4);   // golden cSortCT.cpp:114
    myCountPanel[eMag5      ].SetObject(pnlMag5,   pnlMag5Yield,   lblMag5,   pnARTMag5,   pnARTMag5Yield,   lblARTMag5,   eMag5);   // golden cSortCT.cpp:115
    myCountPanel[eMag6      ].SetObject(pnlMag6,   pnlMag6Yield,   lblMag6,   pnARTMag6,   pnARTMag6Yield,   lblARTMag6,   eMag6);   // golden cSortCT.cpp:116
    myCountPanel[eMag7      ].SetObject(pnlMag7,   pnlMag7Yield,   lblMag7,   pnARTMag7,   pnARTMag7Yield,   lblARTMag7,   eMag7);   // golden cSortCT.cpp:117
    myCountPanel[eMag8      ].SetObject(pnlMag8,   pnlMag8Yield,   lblMag8,   pnARTMag8,   pnARTMag8Yield,   lblARTMag8,   eMag8);   // golden cSortCT.cpp:118
    myCountPanel[eMag9      ].SetObject(pnlMag9,   pnlMag9Yield,   lblMag9,   pnARTMag9,   pnARTMag9Yield,   lblARTMag9,   eMag9);   // golden cSortCT.cpp:119
    myCountPanel[eMag10     ].SetObject(pnlMag10,  pnlMag10Yield,  lblMag10,  pnARTMag10,  pnARTMag10Yield,  lblARTMag10,  eMag10);   // golden cSortCT.cpp:120
    myCountPanel[eMag11     ].SetObject(pnlMag11,  pnlMag11Yield,  lblMag11,  pnARTMag11,  pnARTMag11Yield,  lblARTMag11,  eMag11);   // golden cSortCT.cpp:121
    myCountPanel[eMag12     ].SetObject(pnlMag12,  pnlMag12Yield,  lblMag12,  pnARTMag12,  pnARTMag12Yield,  lblARTMag12,  eMag12);   // golden cSortCT.cpp:122
    myCountPanel[eMag13     ].SetObject(pnlMag13,  pnlMag13Yield,  lblMag13,  pnARTMag13,  pnARTMag13Yield,  lblARTMag13,  eMag13);   // golden cSortCT.cpp:123
    myCountPanel[eMag14     ].SetObject(pnlMag14,  pnlMag14Yield,  lblMag14,  pnARTMag14,  pnARTMag14Yield,  lblARTMag14,  eMag14);   // golden cSortCT.cpp:124

    // golden :126-135：非 eBulkBox 的每列掛 OnMouseDown=pnlAuto1YieldMouseDown／OnDblClick=pnlAuto1DblClick。
    //   本樹的元件沒有事件（TControl 無 OnMouseDown）；那兩個處理器也沒翻（見檔頭「沒翻」）。
    bShow=false;                                                                // golden :136

    // golden :138-150：pnlTrayCnt[eAutoN]=pnlAutoNTrayCt、pnlTrayID[eAutoN]=pnlAutoNCID。
    //   本樹既有的 facade（上面 W6.3／W7-L1）已把兩個陣列各配成 6 個獨立元件，SM 端（acatchtray.cpp／asendic_Auto.cpp）
    //   都經陣列存取，沒有人用 pnlAutoNTrayCt／pnlAutoNCID 這些名字 —— 所以不另外宣告那 12 個具名成員。

    if(USE_COVER_TRAYID==tCID_NFC)                                              // golden :152
    {
        Width=500;                                                              // golden :154 `fSortCT->Width=500;`
    }
    else
    {
        Width=272;                                                              // golden :158
        pnlTrayCount->Visible=false;                                            // golden :159
        pnlLoadTrayCt->Visible=false;                                           // golden :160
        for(int i=0; i<MAX_AUTO_TRAY; i++)                                      // golden :161
        {
            pnlTrayCnt[i]->Visible=false;                                       // golden :163
            pnlTrayID[i]->Visible=false;                                        // golden :164
        }
    }
    // ⚠ 偏離（兩點，都只影響版面寬度與 NFC 機型的 Tray Count 欄）：
    //   (1) golden 寫 `fSortCT->Width`：VCL 的 Application->CreateForm 先把全域指標指向新物件再呼叫建構子，
    //       所以 golden 的 fSortCT 在建構子裡已有效；本樹是 `fSortCT = new TfSortCT()`，建構時 fSortCT 還是 NULL，
    //       所以寫成 this->Width（同一個物件）。
    //   (2) 本建構子跑在靜態初始化時，USE_COVER_TRAYID 還是 cmydef.cpp:5888 的初值 tCIDNotUse（Gerneral.ini 還沒讀）。
    //       NFC 機型（USE_COVER_TRAYID==tCID_NFC）在 golden 會走上面那支；本樹會走 else。NFC 是機型／客戶專屬，先註記。
}
// AI(W906-PT-W3-integrate) 20260808: uRENESAS_Server facade sinks.  Offline
// no-ops; the real bodies (golden cSortCT.cpp:210-279 / :345+) belong to the
// unported cSortCT.cpp wave.  See forms/fSortCT.h for the full behaviour delta
// -- these two are NOT UI-only in golden, so that note is worth reading before
// relying on unload counts or piggy-back triggers in an offline test.
// Steven 20260925 (Data.SortCT)：⛔ 上一段已過期（更正不刪）—— 那一波就是這一波。兩個方法改成跳板：
//   g_W906_SortCTBodies 有值（連進了根目錄 cSortCT.cpp，例如 wb_serve）就呼叫 golden 本體；
//   沒有（沒連 cSortCT.cpp.obj 的測試執行檔）仍是原本的 no-op，行為與改前相同。
const TfSortCTBodies *g_W906_SortCTBodies = 0;
void TfSortCT::ShowLoadingIC()     { if (g_W906_SortCTBodies) (this->*(g_W906_SortCTBodies->ShowLoadingIC))(); }
void TfSortCT::ShowSortIC()        { if (g_W906_SortCTBodies) (this->*(g_W906_SortCTBodies->ShowSortIC))(); }
void TfSortCT::ShowLoadingIC_ART() { if (g_W906_SortCTBodies) (this->*(g_W906_SortCTBodies->ShowLoadingIC_ART))(); }
void TfSortCT::btnClearCountClick(TObject *Sender)
{
    if (g_W906_SortCTBodies) (this->*(g_W906_SortCTBodies->btnClearCountClick))(Sender);
}

//---------------------------------------------------------------------------
//  _MyCountPanel -- golden cSortCT.cpp:39-87（逐行）
//---------------------------------------------------------------------------
void _MyCountPanel::SetObject(TfSortCTPanel *_pnlCount,    TfSortCTPanel *_pnlYield,    TLabel *_lblName,
                              TfSortCTPanel *_pnlCountART, TfSortCTPanel *_pnlYieldART, TLabel *_lblNameART,
                              int _iTag)
{
    pnlCount        =_pnlCount;
    pnlYield        =_pnlYield;
    lblName         =_lblName;
    pnlCountART     =_pnlCountART;
    pnlYieldART     =_pnlYieldART;
    lblNameART      =_lblNameART;

    iTag            =_iTag;
    pnlCount->Tag   =iTag;
    pnlYield->Tag   =iTag;
    lblName ->Tag   =iTag;
    pnlCountART->Tag=iTag;
    pnlYieldART->Tag=iTag;
    lblNameART->Tag =iTag;
    SetVisible(true);
}
//---------------------------------------------------------------------------
void _MyCountPanel::SetTop(int _iTop)
{
    iTop            =_iTop;
    pnlCount->Top   =iTop+2;
    pnlYield->Top   =iTop+2;
    lblName->Top    =iTop+6;
    pnlCountART->Top=iTop+2;
    pnlYieldART->Top=iTop+2;
    lblNameART->Top =iTop+6;

    lblName->Left    =7;
    pnlYield->Left   =60;
    pnlCount->Left   =116;
    lblNameART->Left =7;
    pnlYieldART->Left=60;
    pnlCountART->Left=116;
}
//---------------------------------------------------------------------------
void _MyCountPanel::SetVisible(bool _bVisible)
{
    bVisible            =_bVisible;
    pnlCount->Visible   =bVisible;
    pnlYield->Visible   =bVisible;
    lblName->Visible    =bVisible;
    pnlCountART->Visible=bVisible;
    pnlYieldART->Visible=bVisible;
    lblNameART->Visible =bVisible;
}

//---------------------------------------------------------------------------
//  FormClose -- golden cSortCT.cpp:169-172
//---------------------------------------------------------------------------
void TfSortCT::FormClose()
{
    bShow=false;
}

//---------------------------------------------------------------------------
//  FormShow -- golden cSortCT.cpp:174-208
//  ⚠ golden 開機會 fSortCT->Show()（main.cpp:9162-9163）⇒ 這一支在 golden 開機就跑一次；
//    本樹 wb_serve 目前**沒有**呼叫它（tools/wb_serve.cpp 不在這一波的寫入範圍）—— 交整合者評估接進開機序列。
//    它會跑 ShowLoadingIC（Auto-Site-Map 時清 SendCT[2]；Continuous Loader 到數時 WAR07324／Piggy-Back），
//    所以接進開機的時機要照 golden（DoReadLastData 之後）。網頁的顯示**不依賴**它：tag 直接由 LastSet 算。
//---------------------------------------------------------------------------
void TfSortCT::FormShow()
{
    bShow=true;

    edLotID->Text=IniConfig.sLotID;                                             //Steven 20140814
    if(CUSTOMER_CODE==CC_ASE_M)                                                 //Steven 20140814 : For ASE-M
    {
        gbLotID->Visible=true;
        // GATE (客戶專屬 CC_ASE_M)：golden :182 `Timer1->Enabled=true;` —— Timer1Timer（:869-884）會在
        //   SystemStart 時把 edLotID 寫回 config.ini；本樹沒有 TTimer 替身也沒翻 Timer1Timer。客戶專屬，先註記。
        //   AI(W906-FRW-S97) 20260926: Timer1Timer 已翻（cSortCT.cpp 檔尾 W906_SortCTTimer1Run）；Timer1 的開關在 WebSortCT.cpp
        //   （s_w906SortCTTimer1Enabled，網頁第一次送 op=lotId 時照本段 :178／:182 打開）—— 本樹 wb_serve 不呼叫這支 FormShow，
        //   而且 forms 不能呼叫 wb_serve 專用的 WebSortCT.cpp，所以這裡仍不設。
    }

    ShowLoadingIC();
    ShowSortIC();

    if((USE_AUTO_RETEST==eartInstall &&
       (bAutoReTest_ART ||
        IniConfig.bA10_AutoReTest)) ||                                          //kevin 20150615
       CosFunction.bUseARTSortCount)                                            //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
    {
        ShowLoadingIC_ART();
    }
    UpForm();
    W906_SetTabVisibility();                                                    // golden :196-204、:207（抽成一支，網頁每拍也呼叫；見下）
    PageControl1->ActivePageIndex=0;                                            // golden :206
    PageControl1->ActivePage=SortCount;                                         // [PORT-ONLY] VCL 設 ActivePageIndex 會連帶換 ActivePage；替身不連動，手動補
}

//---------------------------------------------------------------------------
//  W906_SetTabVisibility -- [PORT-ONLY] golden FormShow :196-204 與 :207，逐字。
//  抽出來的理由：網頁要照 golden 決定 ART／IC Count 分頁顯不顯示，但 FormShow 本身有副作用
//  （ShowLoadingIC／ShowSortIC），不能每拍呼叫。這三個敘述只讀設定、只寫分頁屬性，所以單獨抽出。
//  golden 原順序是 :196-204 → :206 ActivePageIndex=0 → :207；:207 挪到 :206 之前，兩者互不相依。
//---------------------------------------------------------------------------
void TfSortCT::W906_SetTabVisibility()
{
    ARTSortCount->TabVisible=(CosFunction.bUseSCKART==false &&                  //Steven 20161201 : For SCK 93K ART
                              USE_AUTO_RETEST==eartInstall &&
                              (bAutoReTest_ART ||                               //wei 20160614 alex open
                               IniConfig.bA10_AutoReTest));                     //wei 20150331 打開功能就顯示
    if(CosFunction.bUseARTSortCount)                                            //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
    {
        ARTSortCount->TabVisible=true;
        ARTSortCount->Caption="Auto Sort Count";
    }

    tsICCount->TabVisible=CosFunction.bShowHPICCount;                           //Steven 20221228 : 計算加熱盤IC數量
}

//---------------------------------------------------------------------------
//  UpForm -- golden cSortCT.cpp:710-815「fix bin 分割tray更新」（逐行）
//  只寫元件的 Visible／Top／Left／Height —— 不碰任何機台狀態，所以網頁端可以每拍呼叫
//  （WebBridgeTags.cpp 檔尾讀 myCountPanel[i].bVisible 決定哪幾列顯示）。
//---------------------------------------------------------------------------
void TfSortCT::UpForm()
{
    int iTop=2;
    fSortCT->Height=1000;
    pnlUnloadBG->Height=800;

    for(int i=0; i<eTrayCount; i++)
    {
        myCountPanel[i].SetVisible(true);
    }


    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)
    {
        myCountPanel[eAuto3 ].SetVisible(false);
        myCountPanel[eFix2  ].SetVisible(false);
        myCountPanel[eFix3  ].SetVisible(false);
    }
    myCountPanel[eBulkBox ].SetVisible(iHWFix_BinBox==1);
    myCountPanel[eAuto4   ].SetVisible(AUTO_EMPTY_COLOR>=3);
    myCountPanel[eAuto5   ].SetVisible(AUTO_EMPTY_COLOR>=3);
    myCountPanel[eAuto6   ].SetVisible(AUTO_EMPTY_COLOR>=4);

    for(int i=eMag1; i<=eMag14; i++)                                            //JerryYang 20220909 : add magazine
    {
        myCountPanel[i].SetVisible(AUTO3_IS_MAGAZINE>0);
    }

    if(AUTO_EMPTY_COLOR>=3)                                                     //Steven 20230907 : For HT-9011UC
    {
        if(CosFunction.bUseTrayUpDownSet)                                       //wei 20160224 TSMC FIX UPDOWN
        {
            myCountPanel[eFix7 ].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix1]);
            myCountPanel[eFix8 ].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix2]);
            myCountPanel[eFix9 ].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix3]);
            myCountPanel[eFix10].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix4]);
            myCountPanel[eFix11].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix5]);
            myCountPanel[eFix12].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix6]);
        }
        else
        {
            myCountPanel[eFix7 ].SetVisible(TrayForm.iFixTrayMode);
            myCountPanel[eFix8 ].SetVisible(TrayForm.iFixTrayMode);
            myCountPanel[eFix9 ].SetVisible(TrayForm.iFixTrayMode);
            myCountPanel[eFix10].SetVisible(TrayForm.iFixTrayMode);
            myCountPanel[eFix11].SetVisible(TrayForm.iFixTrayMode);
            myCountPanel[eFix12].SetVisible(TrayForm.iFixTrayMode);
        }
    }
    else
    {
        for(int i=eFix7; i<=eFix12; i++)
        {
            myCountPanel[i     ].SetVisible(false);
        }

        if(CosFunction.bUseTrayUpDownSet)                                       //wei 20160224 TSMC FIX UPDOWN
        {
            myCountPanel[eFix4 ].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix1]);
            myCountPanel[eFix5 ].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix2]);
            myCountPanel[eFix6 ].SetVisible(TrayForm.iFixTrayMode && TrayForm.bTrayUpDownSet[eFix3]);
        }
        else
        {
            myCountPanel[eFix4 ].SetVisible(TrayForm.iFixTrayMode);
            myCountPanel[eFix5 ].SetVisible(TrayForm.iFixTrayMode);
            myCountPanel[eFix6 ].SetVisible(TrayForm.iFixTrayMode);
        }
    }

    iTop=2;
    for(int i=0; i<eTrayCount; i++)                                             //JerryYang 20220909 : add magazine
    {
        if(myCountPanel[i].bVisible==true)
        {
            myCountPanel[i].SetTop(iTop);
            iTop=iTop+24;
        }
    }
    iVisibleHeight=iTop;
    btnClearCount->Top=iTop;
    pnlUnloadBG->Height=btnClearCount->Top+btnClearCount->Height+10;

    if(PageControl1->ActivePage==SortCount)
    {
        if(gbLotID->Visible==false)
        {
            fSortCT->Height=pnlUnloadBG->Top+pnlUnloadBG->Height+40+btnClearCount->Height;
        }
        else
        {
            fSortCT->Height=gbLotID->Top+gbLotID->Height+40;
        }
    }
    else if(PageControl1->ActivePage==tsICCount)
    {
        fSortCT->Height=120;
    }
    else
    {
        pnlUnloadingARTBG->Height=iVisibleHeight+4;
        fSortCT->Height=pnlUnloadingARTBG->Top+pnlUnloadingARTBG->Height+40;
    }
}

//---------------------------------------------------------------------------
//  lblTotalClick -- golden cSortCT.cpp:1883-1886
//---------------------------------------------------------------------------
void TfSortCT::lblTotalClick(TObject * /*Sender*/)
{
    UpForm();
}
TfSortCT *fSortCT = new TfSortCT();
