// =============================================================================
//  forms/fSortCT.h  --  non-VCL stand-in for golden's fSortCT form pointer
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.h by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Content moved VERBATIM apart
//  from the F0-e virtual destructor; TfSortCTPanel is now a typedef onto
//  vclcompat::TPanel (see forms/FormWidgets.h).  Facade-wide contract: see
//  forms/fMain.h.
//
//  Steven 20260925 (Data.SortCT)：golden V912 cSortCT.cpp 的顯示／清除方法落地（porting-gaps.md 十一）。
//    golden：HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cSortCT.{h,cpp,dfm}（cp950，行號一律 V912）。
//    放置（照 fQwertyKey／fPassword 的切法，CMakeLists.txt ht9045_forms 段 :739-746）：
//      * 本檔＋forms/fSortCT.cpp（ht9045_forms）：全域 fSortCT、建構子、_MyCountPanel、FormShow、UpForm、
//        lblTotalClick —— 只碰 ht9045_globals（nm 量過：CosFunction／TrayForm／IniConfig／CUSTOMER_CODE／
//        USE_AUTO_RETEST／bAutoReTest_ART／iHWFix_BinBox／AUTO_EMPTY_COLOR／AUTO3_IS_MAGAZINE／
//        USE_Scanner_AOI_Inspection／USE_COVER_TRAYID 全在 libht9045_globals.a）。
//      * 根目錄 cSortCT.cpp（ht9045_sm）：ShowLoadingIC／ShowLoadingIC_ART／ShowSortIC／btnClearCountClick
//        的 golden 本體（會碰 fShowBinSelect／ArmData／fContactCT／fProductionInfo／EventReport……，
//        都在 ht9045_sm 以上，放進 ht9045_forms 會長出 forms->sm 的未宣告邊，見 CMakeLists.txt:618-660）。
//    接法：這四個方法在本檔仍是 virtual（vtable 的 key function 留在 forms/fSortCT.cpp），
//      forms/fSortCT.cpp 的本體是跳板 —— 查 g_W906_SortCTBodies（cSortCT.cpp 靜態初始化時填入，
//      WebBridgeTags.cpp 每拍呼叫 W906_SortCTInstall() 保證 cSortCT.cpp.obj 被連進 wb_serve）。
//      沒有連進 cSortCT.cpp.obj 的執行檔（多數 tests）行為與改前相同：四個呼叫仍是 no-op。
// =============================================================================
#ifndef FORMS_FSORTCT_H
#define FORMS_FSORTCT_H

#include "forms/FormWidgets.h"

// ---------------------------------------------------------------------------
//  Steven 20260925 (Data.SortCT)：UpForm（golden cSortCT.cpp:712-815）會寫的版面欄位。
//  vclcompat::TControl 只有 Left/Top（Controls.h:271-272），沒有 Height —— vclcompat 是共用元件，
//  這一波不動它，所以照 forms/fOffSet.h:173 TfOffSetPageControl 的做法在本檔加最小子類別。
//  ⚠ 只存值，沒有任何東西會移動（本樹沒有視窗）；網頁的列排版由 HTML 流式版面自己完成。
// ---------------------------------------------------------------------------
class TfSortCTBgPanel : public vclcompat::TPanel            // golden TPanel：pnlUnloadBG（h:44）／pnlUnloadingARTBG（h:164）
{
public:
    int Height = 0;
    virtual ~TfSortCTBgPanel() {}
};
class TfSortCTSpeedButton : public vclcompat::TSpeedButton  // golden TSpeedButton：btnClearCount（h:163）／spbClearAllCount（h:46）
{
public:
    int Height = 0;
    virtual ~TfSortCTSpeedButton() {}
};
class TfSortCTGroupBox : public vclcompat::TGroupBox        // golden TGroupBox：gbLotID（h:45）
{
public:
    int Height = 0;
    virtual ~TfSortCTGroupBox() {}
};
class TfSortCTPageControl : public vclcompat::TPageControl  // golden TPageControl：PageControl1（h:34）
{
public:
    // golden UpForm 比 `PageControl1->ActivePage==SortCount`（:795/:806）。vclcompat 只有
    // ActivePageIndex —— 與 forms/fOffSet.h:179 同形，只存值，不切頁，也不與 ActivePageIndex 連動。
    vclcompat::TTabSheet *ActivePage = nullptr;
    virtual ~TfSortCTPageControl() {}
};

// ---------------------------------------------------------------------------
//  _MyCountPanel -- golden cSortCT.h:13-28（一列 = 數量／良率／站名 × 一般／ART 兩份）。
//  偏離（純語法）：golden 是 `typedef struct {...} _MyCountPanel;`（匿名 struct 帶成員函式）；
//    這裡寫成具名 struct —— 匿名 struct 用 typedef 名當連結名時不可有成員函式（C++20 P1766 的
//    DR，新版 g++ 會拒絕），具名之後語意相同。名稱照抄 golden（底線開頭＋大寫在標準上保留給實作，
//    編得過，保留原名方便對照）。
//  本體：forms/fSortCT.cpp（golden cSortCT.cpp:39-87）。
// ---------------------------------------------------------------------------
struct _MyCountPanel
{
    TfSortCTPanel *pnlCount;                      // golden cSortCT.h:15
    TfSortCTPanel *pnlYield;                      // golden cSortCT.h:16
    TLabel        *lblName;                       // golden cSortCT.h:17
    TfSortCTPanel *pnlCountART;                   // golden cSortCT.h:18
    TfSortCTPanel *pnlYieldART;                   // golden cSortCT.h:19
    TLabel        *lblNameART;                    // golden cSortCT.h:20

    bool bVisible;                                // golden cSortCT.h:22
    int iTop;                                     // golden cSortCT.h:23
    int iTag;                                     // golden cSortCT.h:24
    void SetObject(TfSortCTPanel *_pnlCount, TfSortCTPanel *_pnlYield, TLabel *_lblName,
                   TfSortCTPanel *_pnlCountART, TfSortCTPanel *_pnlYieldART, TLabel *_lblNameART, int _iTag);   // golden cSortCT.h:25
    void SetTop(int _iTop);                       // golden cSortCT.h:26
    void SetVisible(bool _bVisible);              // golden cSortCT.h:27
};

// ===========================================================================
//  TfSortCT -- non-VCL stub (golden cSortCT.h).  pnlHP1/pnlHP2 are TPanel* in
//  the golden; the leaves only assign ->Caption an AnsiString (HowManyIC()).
//  `fSortCT->pnlHP1->Caption=...` compiles against the unified TPanel stand-in.
//  Used inside `if(CosFunction.bShowHPICCount)` (default false).
// ===========================================================================
class TfSortCT
{
public:
    TfSortCTPanel *pnlHP1;
    TfSortCTPanel *pnlHP2;
    // -- W6.3 ADD: tray panels the TRAY-ARM ENGINE (acatchtray.cpp) derefs -------
    //    golden cSortCT.h TPanel* members; the catchtray engine only assigns
    //    ->Caption (AnsiString; HowManyIC() returns int -> AnsiString int-ctor).
    TfSortCTPanel *pnlLoad;                       // [DATA] golden cSortCT.h:59  (loader IC-count panel)
    TfSortCTPanel *pnlLoadCID;                    // [DATA] golden cSortCT.h:265 (loader cover-ID panel)
    TfSortCTPanel *pnlCoverTrayD;                 // [DATA] golden cSortCT.h:280 (color cover-tray-ID panel)
    TfSortCTPanel *pnlTrayCnt[6];                 // [DATA] golden cSortCT.h:337 (per-Auto tray-count panels)
    // AI(W906-W7-L1-Wave0) 20260801: golden cSortCT.h:338 `TPanel
    // *pnlTrayID[MAX_AUTO_TRAY];` -- the per-Auto tray-ID caption panels.
    // asendic_Auto.cpp:1156 reads `fSortCT->pnlTrayID[Pos]->Caption` (read-only in
    // this family), so the ctor must allocate all 6.
    // DIMENSION: golden's MAX_AUTO_TRAY is 6 (golden MachineType.h:396, read this
    // pass).  Written as the literal 6 rather than the macro because forms/
    // headers include only forms/FormWidgets.h -> vclcompat/{vcl_compat,Controls,
    // StringGrid}.h and MAX_AUTO_TRAY is not visible there -- the same convention
    // the sibling pnlTrayCnt[6] one line up already follows.
    TfSortCTPanel *pnlTrayID[6];                  // [DATA] golden cSortCT.h:338 (TPanel*[MAX_AUTO_TRAY==6])
    // -- AI(W906-PT-W3-integrate) 20260808 ADD: the 2 methods the PT-W3 unit
    //    Automation/uRENESAS_Server.cpp calls exactly as golden does (its own
    //    "FACADE ADDITIONS NEEDED" banner, uRENESAS_Server.cpp:106-108, is the
    //    measurement).  golden cSortCT.cpp is UNPORTED as a whole -- there is no
    //    cSortCT.cpp in this tree -- so both bodies belong to that future wave and
    //    are offline no-ops here (forms/fSortCT.cpp).
    //
    //    BEHAVIOUR DELTA, STATED BECAUSE IT IS NOT NEUTRAL (unlike most facade
    //    sinks in this family, these two golden bodies are NOT UI-only):
    //      * ShowLoadingIC (golden cSortCT.cpp:210-279) resets LastSet.SendCT[2]
    //        (and SendCT_ART[2]) on Auto-Site-Map runs, and on the
    //        TestIF.bContinuousLoader / bContinuousLoader_RT arms either raises
    //        WAR07324 or fires ProcessPiggyBackFunction() with
    //        iWhoTriggerPiggyBack=pbtContinualLoader.  Offline: the piggy-back
    //        trigger and that alarm do not happen.
    //      * ShowSortIC (golden cSortCT.cpp:345+) aggregates LastSet.BinCT[0][]
    //        into RunInfo.iUnloadCount / iUnloadCount_ART, and sets iSECSGEMPass /
    //        iSECSGEMFail / iATRPassCount / iATRFailCount / iATRTotalCount.
    //        Offline: those counters keep whatever value they already held, so a
    //        test that expects unload counts to move after a sort must drive them
    //        directly rather than via this call.
    //    NOTE for whoever lands that wave: csystem.cpp:2820-2821 gates these same
    //    two golden calls behind its own TU-local W7C2_FSORTCT_SHOWLOADING() /
    //    W7C2_FSORTCT_SHOWSORT() macros, whose stated premise ("these specific
    //    members are absent") stops being true with this ADD.  Behaviour there is
    //    unchanged either way today (both routes are no-ops), so nothing is
    //    silently wrong -- but those two macros are now retirable and should go
    //    when the real bodies land, or they will hide them.
    //  Steven 20260925 (Data.SortCT)：⛔ 上面兩段已過期，更正不刪（本樹慣例）：
    //    * 本體**已經落地**（根目錄 cSortCT.cpp，經 g_W906_SortCTBodies 跳板），wb_serve 裡這兩個呼叫
    //      不再是 no-op —— RunInfo.iUnloadCount／iSECSGEMPass／iATR*Count 會照 golden 匯總，
    //      ShowLoadingIC 的 WAR07324／ProcessPiggyBackFunction 也會照 golden 觸發。
    //    * 「csystem.cpp:2820-2821」是過期行號（porting-gaps.md 十一已記）。20260925 實測：巨集定義在
    //      csystem.cpp:4099-4100，呼叫點 :5764-5765 與 :5849-5850。**它們仍然是 no-op**（csystem.cpp 不在這一波的
    //      寫入範圍），所以 csystem 那兩條路徑（Initial／Continue Start 前的清數）照舊不匯總 —— 交整合者退役。
    virtual void ShowLoadingIC();                 // [METHOD] golden cSortCT.h:326 (body cSortCT.cpp:210-279)
    virtual void ShowSortIC();                    // [METHOD] golden cSortCT.h:327 (body cSortCT.cpp:345+)
    TfSortCT();
    virtual ~TfSortCT() {}

    // =======================================================================
    //  Steven 20260925 (Data.SortCT)：golden V912 cSortCT.h 其餘成員
    // =======================================================================
    // -- 跳板（forms/fSortCT.cpp），本體在根目錄 cSortCT.cpp --------------------
    virtual void ShowLoadingIC_ART();             // [METHOD] golden cSortCT.h:329 (body cSortCT.cpp:281-343)
    virtual void btnClearCountClick(TObject *Sender);   // [METHOD] golden cSortCT.h:297 (body cSortCT.cpp:585-708)
    // -- 只碰 ht9045_globals 的方法（本體 forms/fSortCT.cpp） ---------------------
    void FormClose();                             // [METHOD] golden cSortCT.h:281 (body :169-172；TCloseAction& 參數拿掉，本體沒讀它)
    void FormShow();                              // [METHOD] golden cSortCT.h:282 (body :174-208)
    void UpForm();                                // [METHOD] golden cSortCT.h:328 (body :712-815)
    void lblTotalClick(TObject *Sender);          // [METHOD] golden cSortCT.h:309 (body :1883-1886)
    void W906_SetTabVisibility();                 // [PORT-ONLY] golden FormShow :196-207 的三個分頁可見度敘述，抽出來給網頁每拍重算
    // -- golden 本體（根目錄 cSortCT.cpp，ht9045_sm）；經 g_W906_SortCTBodies 由上面的跳板呼叫 --
    void W906Body_ShowLoadingIC();                // golden cSortCT.cpp:210-279
    void W906Body_ShowLoadingIC_ART();            // golden cSortCT.cpp:281-343
    void W906Body_ShowSortIC();                   // golden cSortCT.cpp:345-442
    void W906Body_btnClearCountClick(TObject *Sender);   // golden cSortCT.cpp:585-708

    bool bShow;                                   // [DATA] golden cSortCT.h:325
    _MyCountPanel myCountPanel[33];               // [DATA] golden cSortCT.h:335 [eTrayCount]；33 用字面值，理由同上面 pnlTrayCnt[6]（forms/fSortCT.cpp 以 static_assert 對 eTrayCount）
    int iVisibleHeight;                           // [DATA] golden cSortCT.h:339
    int Width;                                    // golden TForm::Width（只存值；建構子 :154/:158 與 dfm ClientWidth）
    int Height;                                   // golden TForm::Height（只存值；UpForm :715/:799/:803/:808/:813）

    // -- 表單層元件（golden cSortCT.h:33-63，只列翻譯本體會碰到的） -------------
    TfSortCTPageControl *PageControl1;            // [DATA] golden cSortCT.h:34
    TTabSheet           *SortCount;               // [DATA] golden cSortCT.h:35
    TTabSheet           *ARTSortCount;            // [DATA] golden cSortCT.h:36
    TfSortCTPanel       *pnlLoader;               // [DATA] golden cSortCT.h:39
    TLabel              *lblTotal;                // [DATA] golden cSortCT.h:41
    TfSortCTPanel       *pnlTotal;                // [DATA] golden cSortCT.h:42
    TfSortCTPanel       *pnlYield;                // [DATA] golden cSortCT.h:43
    TfSortCTBgPanel     *pnlUnloadBG;             // [DATA] golden cSortCT.h:44
    TfSortCTGroupBox    *gbLotID;                 // [DATA] golden cSortCT.h:45
    TfSortCTSpeedButton *spbClearAllCount;        // [DATA] golden cSortCT.h:46
    TEdit               *edLotID;                 // [DATA] golden cSortCT.h:47
    TfSortCTPanel       *pnlLoadingART;           // [DATA] golden cSortCT.h:50
    TfSortCTPanel       *pnlTotalART;             // [DATA] golden cSortCT.h:53
    TfSortCTPanel       *pnlYieldART;             // [DATA] golden cSortCT.h:54
    TTabSheet           *tsICCount;               // [DATA] golden cSortCT.h:56
    TfSortCTSpeedButton *btnClearCount;           // [DATA] golden cSortCT.h:163
    TfSortCTBgPanel     *pnlUnloadingARTBG;       // [DATA] golden cSortCT.h:164
    TfSortCTPanel       *pnlLoadTrayCt;           // [DATA] golden cSortCT.h:264
    TfSortCTPanel       *pnlTrayCount;            // [DATA] golden cSortCT.h:268

    // -- 33 站 × 6 個元件（golden cSortCT.h:64-263，原宣告順序；產生器 scratchpad/sortct/gen_facade.py） --
    TLabel        *lblAuto1;              // [DATA] golden cSortCT.h:64
    TfSortCTPanel *pnlAuto1Yield;         // [DATA] golden cSortCT.h:65
    TfSortCTPanel *pnlAuto1;              // [DATA] golden cSortCT.h:66
    TLabel        *lblAuto2;              // [DATA] golden cSortCT.h:67
    TfSortCTPanel *pnlAuto2Yield;         // [DATA] golden cSortCT.h:68
    TfSortCTPanel *pnlAuto2;              // [DATA] golden cSortCT.h:69
    TLabel        *lblAuto3;              // [DATA] golden cSortCT.h:70
    TfSortCTPanel *pnlAuto3Yield;         // [DATA] golden cSortCT.h:71
    TfSortCTPanel *pnlAuto3;              // [DATA] golden cSortCT.h:72
    TLabel        *lblAuto4;              // [DATA] golden cSortCT.h:73
    TfSortCTPanel *pnlAuto4Yield;         // [DATA] golden cSortCT.h:74
    TfSortCTPanel *pnlAuto4;              // [DATA] golden cSortCT.h:75
    TLabel        *lblAuto5;              // [DATA] golden cSortCT.h:76
    TfSortCTPanel *pnlAuto5Yield;         // [DATA] golden cSortCT.h:77
    TfSortCTPanel *pnlAuto5;              // [DATA] golden cSortCT.h:78
    TLabel        *lblAuto6;              // [DATA] golden cSortCT.h:79
    TfSortCTPanel *pnlAuto6Yield;         // [DATA] golden cSortCT.h:80
    TfSortCTPanel *pnlAuto6;              // [DATA] golden cSortCT.h:81
    TLabel        *lblFix1;               // [DATA] golden cSortCT.h:82
    TfSortCTPanel *pnlFix1Yield;          // [DATA] golden cSortCT.h:83
    TfSortCTPanel *pnlFix1;               // [DATA] golden cSortCT.h:84
    TLabel        *lblFix2;               // [DATA] golden cSortCT.h:85
    TfSortCTPanel *pnlFix2Yield;          // [DATA] golden cSortCT.h:86
    TfSortCTPanel *pnlFix2;               // [DATA] golden cSortCT.h:87
    TLabel        *lblFix4;               // [DATA] golden cSortCT.h:88
    TfSortCTPanel *pnlFix4Yield;          // [DATA] golden cSortCT.h:89
    TfSortCTPanel *pnlFix4;               // [DATA] golden cSortCT.h:90
    TLabel        *lblFix3;               // [DATA] golden cSortCT.h:91
    TfSortCTPanel *pnlFix3Yield;          // [DATA] golden cSortCT.h:92
    TfSortCTPanel *pnlFix3;               // [DATA] golden cSortCT.h:93
    TLabel        *lblFix5;               // [DATA] golden cSortCT.h:94
    TfSortCTPanel *pnlFix5Yield;          // [DATA] golden cSortCT.h:95
    TfSortCTPanel *pnlFix5;               // [DATA] golden cSortCT.h:96
    TLabel        *lblFix6;               // [DATA] golden cSortCT.h:97
    TfSortCTPanel *pnlFix6Yield;          // [DATA] golden cSortCT.h:98
    TfSortCTPanel *pnlFix6;               // [DATA] golden cSortCT.h:99
    TLabel        *lblFix7;               // [DATA] golden cSortCT.h:100
    TfSortCTPanel *pnlFix7Yield;          // [DATA] golden cSortCT.h:101
    TfSortCTPanel *pnlFix7;               // [DATA] golden cSortCT.h:102
    TLabel        *lblFix8;               // [DATA] golden cSortCT.h:103
    TfSortCTPanel *pnlFix8Yield;          // [DATA] golden cSortCT.h:104
    TfSortCTPanel *pnlFix8;               // [DATA] golden cSortCT.h:105
    TLabel        *lblFix9;               // [DATA] golden cSortCT.h:106
    TfSortCTPanel *pnlFix9Yield;          // [DATA] golden cSortCT.h:107
    TfSortCTPanel *pnlFix9;               // [DATA] golden cSortCT.h:108
    TLabel        *lblFix10;              // [DATA] golden cSortCT.h:109
    TfSortCTPanel *pnlFix10Yield;         // [DATA] golden cSortCT.h:110
    TfSortCTPanel *pnlFix10;              // [DATA] golden cSortCT.h:111
    TLabel        *lblFix11;              // [DATA] golden cSortCT.h:112
    TfSortCTPanel *pnlFix11Yield;         // [DATA] golden cSortCT.h:113
    TfSortCTPanel *pnlFix11;              // [DATA] golden cSortCT.h:114
    TLabel        *lblFix12;              // [DATA] golden cSortCT.h:115
    TfSortCTPanel *pnlFix12Yield;         // [DATA] golden cSortCT.h:116
    TfSortCTPanel *pnlFix12;              // [DATA] golden cSortCT.h:117
    TLabel        *lblBinBox;             // [DATA] golden cSortCT.h:118
    TfSortCTPanel *pnlBinBoxYield;        // [DATA] golden cSortCT.h:119
    TfSortCTPanel *pnlBinBox;             // [DATA] golden cSortCT.h:120
    TLabel        *lblMag1;               // [DATA] golden cSortCT.h:121
    TfSortCTPanel *pnlMag1Yield;          // [DATA] golden cSortCT.h:122
    TfSortCTPanel *pnlMag1;               // [DATA] golden cSortCT.h:123
    TLabel        *lblMag2;               // [DATA] golden cSortCT.h:124
    TfSortCTPanel *pnlMag2Yield;          // [DATA] golden cSortCT.h:125
    TfSortCTPanel *pnlMag2;               // [DATA] golden cSortCT.h:126
    TLabel        *lblMag3;               // [DATA] golden cSortCT.h:127
    TfSortCTPanel *pnlMag3Yield;          // [DATA] golden cSortCT.h:128
    TfSortCTPanel *pnlMag3;               // [DATA] golden cSortCT.h:129
    TLabel        *lblMag4;               // [DATA] golden cSortCT.h:130
    TfSortCTPanel *pnlMag4Yield;          // [DATA] golden cSortCT.h:131
    TfSortCTPanel *pnlMag4;               // [DATA] golden cSortCT.h:132
    TLabel        *lblMag5;               // [DATA] golden cSortCT.h:133
    TfSortCTPanel *pnlMag5Yield;          // [DATA] golden cSortCT.h:134
    TfSortCTPanel *pnlMag5;               // [DATA] golden cSortCT.h:135
    TLabel        *lblMag6;               // [DATA] golden cSortCT.h:136
    TfSortCTPanel *pnlMag6Yield;          // [DATA] golden cSortCT.h:137
    TfSortCTPanel *pnlMag6;               // [DATA] golden cSortCT.h:138
    TLabel        *lblMag7;               // [DATA] golden cSortCT.h:139
    TfSortCTPanel *pnlMag7Yield;          // [DATA] golden cSortCT.h:140
    TfSortCTPanel *pnlMag7;               // [DATA] golden cSortCT.h:141
    TLabel        *lblMag8;               // [DATA] golden cSortCT.h:142
    TfSortCTPanel *pnlMag8Yield;          // [DATA] golden cSortCT.h:143
    TfSortCTPanel *pnlMag8;               // [DATA] golden cSortCT.h:144
    TLabel        *lblMag9;               // [DATA] golden cSortCT.h:145
    TfSortCTPanel *pnlMag9Yield;          // [DATA] golden cSortCT.h:146
    TfSortCTPanel *pnlMag9;               // [DATA] golden cSortCT.h:147
    TLabel        *lblMag10;              // [DATA] golden cSortCT.h:148
    TfSortCTPanel *pnlMag10Yield;         // [DATA] golden cSortCT.h:149
    TfSortCTPanel *pnlMag10;              // [DATA] golden cSortCT.h:150
    TLabel        *lblMag11;              // [DATA] golden cSortCT.h:151
    TfSortCTPanel *pnlMag11Yield;         // [DATA] golden cSortCT.h:152
    TfSortCTPanel *pnlMag11;              // [DATA] golden cSortCT.h:153
    TLabel        *lblMag12;              // [DATA] golden cSortCT.h:154
    TfSortCTPanel *pnlMag12Yield;         // [DATA] golden cSortCT.h:155
    TfSortCTPanel *pnlMag12;              // [DATA] golden cSortCT.h:156
    TLabel        *lblMag13;              // [DATA] golden cSortCT.h:157
    TfSortCTPanel *pnlMag13Yield;         // [DATA] golden cSortCT.h:158
    TfSortCTPanel *pnlMag13;              // [DATA] golden cSortCT.h:159
    TLabel        *lblMag14;              // [DATA] golden cSortCT.h:160
    TfSortCTPanel *pnlMag14Yield;         // [DATA] golden cSortCT.h:161
    TfSortCTPanel *pnlMag14;              // [DATA] golden cSortCT.h:162
    TLabel        *lblARTMag14;           // [DATA] golden cSortCT.h:165
    TfSortCTPanel *pnARTMag14Yield;       // [DATA] golden cSortCT.h:166
    TfSortCTPanel *pnARTMag14;            // [DATA] golden cSortCT.h:167
    TLabel        *lblARTMag13;           // [DATA] golden cSortCT.h:168
    TfSortCTPanel *pnARTMag13Yield;       // [DATA] golden cSortCT.h:169
    TfSortCTPanel *pnARTMag13;            // [DATA] golden cSortCT.h:170
    TLabel        *lblARTMag12;           // [DATA] golden cSortCT.h:171
    TfSortCTPanel *pnARTMag12Yield;       // [DATA] golden cSortCT.h:172
    TfSortCTPanel *pnARTMag12;            // [DATA] golden cSortCT.h:173
    TLabel        *lblARTMag11;           // [DATA] golden cSortCT.h:174
    TfSortCTPanel *pnARTMag11Yield;       // [DATA] golden cSortCT.h:175
    TfSortCTPanel *pnARTMag11;            // [DATA] golden cSortCT.h:176
    TLabel        *lblARTMag10;           // [DATA] golden cSortCT.h:177
    TfSortCTPanel *pnARTMag10Yield;       // [DATA] golden cSortCT.h:178
    TfSortCTPanel *pnARTMag10;            // [DATA] golden cSortCT.h:179
    TLabel        *lblARTMag9;            // [DATA] golden cSortCT.h:180
    TfSortCTPanel *pnARTMag9Yield;        // [DATA] golden cSortCT.h:181
    TfSortCTPanel *pnARTMag9;             // [DATA] golden cSortCT.h:182
    TLabel        *lblARTMag8;            // [DATA] golden cSortCT.h:183
    TfSortCTPanel *pnARTMag8Yield;        // [DATA] golden cSortCT.h:184
    TfSortCTPanel *pnARTMag8;             // [DATA] golden cSortCT.h:185
    TfSortCTPanel *pnARTMag7;             // [DATA] golden cSortCT.h:186
    TfSortCTPanel *pnARTMag7Yield;        // [DATA] golden cSortCT.h:187
    TLabel        *lblARTMag7;            // [DATA] golden cSortCT.h:188
    TLabel        *lblARTMag6;            // [DATA] golden cSortCT.h:189
    TfSortCTPanel *pnARTMag6Yield;        // [DATA] golden cSortCT.h:190
    TfSortCTPanel *pnARTMag6;             // [DATA] golden cSortCT.h:191
    TLabel        *lblARTMag5;            // [DATA] golden cSortCT.h:192
    TfSortCTPanel *pnARTMag5Yield;        // [DATA] golden cSortCT.h:193
    TfSortCTPanel *pnARTMag5;             // [DATA] golden cSortCT.h:194
    TLabel        *lblARTMag4;            // [DATA] golden cSortCT.h:195
    TfSortCTPanel *pnARTMag4Yield;        // [DATA] golden cSortCT.h:196
    TfSortCTPanel *pnARTMag4;             // [DATA] golden cSortCT.h:197
    TLabel        *lblARTMag3;            // [DATA] golden cSortCT.h:198
    TfSortCTPanel *pnARTMag3Yield;        // [DATA] golden cSortCT.h:199
    TfSortCTPanel *pnARTMag3;             // [DATA] golden cSortCT.h:200
    TLabel        *lblARTMag2;            // [DATA] golden cSortCT.h:201
    TfSortCTPanel *pnARTMag2Yield;        // [DATA] golden cSortCT.h:202
    TfSortCTPanel *pnARTMag2;             // [DATA] golden cSortCT.h:203
    TLabel        *lblARTMag1;            // [DATA] golden cSortCT.h:204
    TfSortCTPanel *pnARTMag1Yield;        // [DATA] golden cSortCT.h:205
    TfSortCTPanel *pnARTMag1;             // [DATA] golden cSortCT.h:206
    TLabel        *lblARTBinBox;          // [DATA] golden cSortCT.h:207
    TfSortCTPanel *pnARTBinBoxYield;      // [DATA] golden cSortCT.h:208
    TfSortCTPanel *pnARTBinBox;           // [DATA] golden cSortCT.h:209
    TLabel        *lblARTFix12;           // [DATA] golden cSortCT.h:210
    TfSortCTPanel *pnARTFix12Yield;       // [DATA] golden cSortCT.h:211
    TfSortCTPanel *pnARTFix12;            // [DATA] golden cSortCT.h:212
    TLabel        *lblARTFix11;           // [DATA] golden cSortCT.h:213
    TfSortCTPanel *pnARTFix11Yield;       // [DATA] golden cSortCT.h:214
    TfSortCTPanel *pnARTFix11;            // [DATA] golden cSortCT.h:215
    TLabel        *lblARTFix10;           // [DATA] golden cSortCT.h:216
    TfSortCTPanel *pnARTFix10Yield;       // [DATA] golden cSortCT.h:217
    TfSortCTPanel *pnARTFix10;            // [DATA] golden cSortCT.h:218
    TLabel        *lblARTFix9;            // [DATA] golden cSortCT.h:219
    TfSortCTPanel *pnARTFix9Yield;        // [DATA] golden cSortCT.h:220
    TfSortCTPanel *pnARTFix9;             // [DATA] golden cSortCT.h:221
    TLabel        *lblARTAuto4;           // [DATA] golden cSortCT.h:222
    TfSortCTPanel *pnARTAuto4Yield;       // [DATA] golden cSortCT.h:223
    TfSortCTPanel *pnARTAuto4;            // [DATA] golden cSortCT.h:224
    TLabel        *lblARTFix8;            // [DATA] golden cSortCT.h:225
    TfSortCTPanel *pnARTFix8Yield;        // [DATA] golden cSortCT.h:226
    TfSortCTPanel *pnARTFix8;             // [DATA] golden cSortCT.h:227
    TLabel        *lblARTFix7;            // [DATA] golden cSortCT.h:228
    TfSortCTPanel *pnARTFix7Yield;        // [DATA] golden cSortCT.h:229
    TfSortCTPanel *pnARTFix7;             // [DATA] golden cSortCT.h:230
    TLabel        *lblARTFix6;            // [DATA] golden cSortCT.h:231
    TfSortCTPanel *pnARTFix6Yield;        // [DATA] golden cSortCT.h:232
    TfSortCTPanel *pnARTFix6;             // [DATA] golden cSortCT.h:233
    TLabel        *lblARTFix5;            // [DATA] golden cSortCT.h:234
    TfSortCTPanel *pnARTFix5Yield;        // [DATA] golden cSortCT.h:235
    TfSortCTPanel *pnARTFix5;             // [DATA] golden cSortCT.h:236
    TLabel        *lblARTFix4;            // [DATA] golden cSortCT.h:237
    TfSortCTPanel *pnARTFix4Yield;        // [DATA] golden cSortCT.h:238
    TfSortCTPanel *pnARTFix4;             // [DATA] golden cSortCT.h:239
    TLabel        *lblARTFix3;            // [DATA] golden cSortCT.h:240
    TfSortCTPanel *pnARTFix3Yield;        // [DATA] golden cSortCT.h:241
    TfSortCTPanel *pnARTFix3;             // [DATA] golden cSortCT.h:242
    TLabel        *lblARTFix2;            // [DATA] golden cSortCT.h:243
    TfSortCTPanel *pnARTFix2Yield;        // [DATA] golden cSortCT.h:244
    TfSortCTPanel *pnARTFix2;             // [DATA] golden cSortCT.h:245
    TLabel        *lblARTFix1;            // [DATA] golden cSortCT.h:246
    TfSortCTPanel *pnARTFix1Yield;        // [DATA] golden cSortCT.h:247
    TfSortCTPanel *pnARTFix1;             // [DATA] golden cSortCT.h:248
    TLabel        *lblARTAuto2;           // [DATA] golden cSortCT.h:249
    TfSortCTPanel *pnARTAuto2Yield;       // [DATA] golden cSortCT.h:250
    TfSortCTPanel *pnARTAuto2;            // [DATA] golden cSortCT.h:251
    TLabel        *lblARTAuto1;           // [DATA] golden cSortCT.h:252
    TfSortCTPanel *pnARTAuto1Yield;       // [DATA] golden cSortCT.h:253
    TfSortCTPanel *pnARTAuto1;            // [DATA] golden cSortCT.h:254
    TLabel        *lblARTAuto3;           // [DATA] golden cSortCT.h:255
    TfSortCTPanel *pnARTAuto3Yield;       // [DATA] golden cSortCT.h:256
    TfSortCTPanel *pnARTAuto3;            // [DATA] golden cSortCT.h:257
    TLabel        *lblARTAuto5;           // [DATA] golden cSortCT.h:258
    TfSortCTPanel *pnARTAuto5Yield;       // [DATA] golden cSortCT.h:259
    TfSortCTPanel *pnARTAuto5;            // [DATA] golden cSortCT.h:260
    TLabel        *lblARTAuto6;           // [DATA] golden cSortCT.h:261
    TfSortCTPanel *pnARTAuto6Yield;       // [DATA] golden cSortCT.h:262
    TfSortCTPanel *pnARTAuto6;            // [DATA] golden cSortCT.h:263

private:
    // golden cSortCT.h:319-322（private）。⚠ golden **從來沒有指派**這四個陣列（四棵 golden 樹 899／908／910／912
    //   的 cSortCT.cpp 全文只有 ShowSortIC :356/:362 兩處寫入 TrayCTPanel／TrayCTART，沒有任何 `TrayCTPanel[i]=`）。
    //   VCL 物件記憶體由 TObject::NewInstance 清零，所以它們是 NULL；見 cSortCT.cpp ShowSortIC 的處理。
    TfSortCTPanel *TrayCTPanel[33] = {};          // [DATA] golden cSortCT.h:319 [eTrayCount]
    TfSortCTPanel *TrayCTART[33]   = {};          // [DATA] golden cSortCT.h:320
    TfSortCTPanel *pnlSortCnt[33]  = {};          // [DATA] golden cSortCT.h:321（golden 全檔未使用）
    TfSortCTPanel *pnlSortART[33]  = {};          // [DATA] golden cSortCT.h:322（golden 全檔未使用）
};
extern TfSortCT *fSortCT;

// ---------------------------------------------------------------------------
//  Steven 20260925 (Data.SortCT)：跳板表。forms/fSortCT.cpp 定義指標（初值 NULL），
//  根目錄 cSortCT.cpp 在靜態初始化時（與 W906_SortCTInstall()）填入。
//  指向成員函式 —— 取址只發生在 cSortCT.cpp，所以 ht9045_forms 對 ht9045_sm 沒有任何未定義符號。
// ---------------------------------------------------------------------------
struct TfSortCTBodies
{
    void (TfSortCT::*ShowLoadingIC)();
    void (TfSortCT::*ShowLoadingIC_ART)();
    void (TfSortCT::*ShowSortIC)();
    void (TfSortCT::*btnClearCountClick)(TObject *Sender);
};
extern const TfSortCTBodies *g_W906_SortCTBodies;

#endif // FORMS_FSORTCT_H
