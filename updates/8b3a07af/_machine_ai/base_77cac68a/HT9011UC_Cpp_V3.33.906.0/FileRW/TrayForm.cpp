// ===========================================================================
//  FileRW/TrayForm.cpp -- 結構 TrayForm（Tray Assignment）的讀寫檔（<recipe>\Tray.Data，逐鍵 WriteIniData）。
//
//  Steven 團隊 20260925.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四。
//
//  golden TfTrayAssignment（cTrayAssignment.cpp，912）由 tools/gen_editlist.py（tools/editlist/TrayForm.py）
//  轉成 TrayForm.gen.inc（元件改成具名替身）：
//    建構子（:27）＝ RadioGroup 加項＋chkICSort[]／edtICSort[][] 指標表；
//    FormShow（:742）＝開頁（ReadFile（含 FixCanUse）＋DoIniDataToForm＋ShowCompnet＋權限）；
//    spbSaveClick（:1337）＝存檔鈕（A01_2 守衛 → bFTBin2RTBin → 圖像模式 → SECS → SaveSetupFile（:1409，
//    逐鍵寫 Tray.Data、存後 ReadFile）→ fLotInfo AMR 分頁 → BackupSetupFile）。
//  沒有 HTEditList —— 存檔流程讀的替身全部是 mustSend。
//
//  不 adopt 移植樹 fTrayAssignment（forms/fTrayAssignment.h）的真元件（Steven 團隊 20260925 量過）：
//    * 移植樹會「讀」這些元件的地方全部在 #if 0 裡：csystem.cpp:27068-27069（GATE H3-6）、forms/fBuilder.cpp:687/713/731
//      （GATE B-9）、Command.cpp:16890-16904、SECSGEM/uHGemHT9045_SV.cpp:848-855（GATE G13）。
//    * 活著的只有 cinitial.cpp:9342 fTrayAssignment->ReadFile()（移植樹自己的讀檔器，對元件只寫 Enabled）。
//    * 產生器的 adopt 只認 `*name = new vclcompat::T()`，forms/fTrayAssignment.h 寫的是 `new TPanel()`（不帶命名空間），
//      認得到的只有 sbNormalTest／sbNormalTest_RT，型別又對不上（golden TScrollBar → 產生器 TControl）⇒ 0 個。
//    * 移植樹的 Init()（golden 建構子本體）與本檔的 TA_TfTrayAssignment() 都對 RGLoader… 做 Items->Add，
//      共用物件會加兩次，除非 wb_serve 不再呼叫 fTrayAssignment->Init()。
//    ⇒ 等那些 GATE 解開時再 adopt（整合者待辦，見報告）。
//
//  開機：golden HT9045.cpp:188 CreateForm(TfTrayAssignment)（在 TfLd_ULd :191、TfTrayForm :192、TfSpeed :194、
//  TfConfiguration :207 之前）。建構子讀 bUseAuto2Empty／CosFunction.bLoaderTrayToAuto1 ⇒ 要在 LoadMachineConfig 之後。
// ===========================================================================
#include "FileRW/TrayForm.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"

// pgRunMode 目前分頁（golden ActivePage／ActivePageIndex）。頁面不送分頁 → 值是 DFM 的 tsReTestGroup（開機設）
// 或 golden 程式設的值。走函式而不是 EL<>()->ActivePageIndex：產生器會把後者算成「存檔讀的替身」（mustSend），
// 但 TPageControl 沒有頁面可送的值種類，頁面就永遠存不了檔。
static int TA_RunModePageIndex()
{
    return EL<TPageControl>("TfTrayAssignment", "pgRunMode")->ActivePageIndex;
}

// VCL TCustomRadioGroup.SetItemIndex（非讀 DFM 時）：v < -1 → -1；v >= Items.Count → Count-1。
// golden GraphicToRadio 的 Position&0x08 等給 2／4／8，VCL 靠這個夾回 0..Count-1；vclcompat TRadioGroup 不夾。
static void TA_SetRadioIndex(TRadioGroup* g, int v)
{
    if (v < -1) v = -1;
    if (v >= g->Items->Count) v = g->Items->Count - 1;
    g->ItemIndex = v;
}

// ---------------------------------------------------------------------------
// golden TfMain::TfMain（main.cpp:1381）建構子 :1414-1497 的 Prod.iTrayType[] 預設（Auto／Fix／BulkBox／Magazine）。
// Steven 團隊 20260925：整合測試第一次原值存檔 Auto1..3 的 FromBuffer／FromBuffer_RT、Auto3 Direction 由 1 變 0。
// 根因：移植樹沒有這一段（20260925 grep `iTrayType[eAuto2`＝只有讀的地方，全樹沒有寫 tTrayAuto 的；forms/fTrayAssignment.cpp
// 只寫 eAuto1／eBulkBox），Prod.iTrayType[] 全是 0＝tNotUse ⇒ golden ReadFile :416-451 只對 tTrayAuto／tTrayFix 的盤讀
// "Tray Type"／"Direction"／"FromBuffer"／"FromBuffer_RT"，全部跳過；:428-432 還把 Auto2..6 的 Direction 抄成
// Auto[eAuto1].iTrayType（golden 原樣）⇒ DoIniDataToForm 把 0 放進 RGAuto*／rgAuto*_RT／iTrayDirect[]，存檔寫回 0。
// 移植樹自己的讀檔器（W906_BootReadTrayAssignment／cinitial.cpp:9342）同樣受害 —— 開機的 TrayForm 也一直是錯的。
// golden 時機：TfMain 建構（HT9045.cpp 第一個 CreateForm）早於 TfTrayAssignment（:188），所以放在本表單開機的最前面；
// 已有人設過（任一格非 0）就不重設（避免蓋掉 FixCanUse 之後的結果）。正式位置應是移植樹 TfMain 建構的翻譯（整合者待辦）。
void FileRW_TrayAssignment_InitProdTrayType()
{
    for (int i = 0; i <= eMag14; i++)
        if (Prod.iTrayType[i] != tNotUse) {
            std::printf("FileRW TrayForm: Prod.iTrayType[] already initialised (index %d) -- golden main.cpp:1414 defaults not applied\n", i);
            return;
        }
    Prod.iTrayType[eAuto1        ]=tTrayAuto;                                  // golden main.cpp:1414
    Prod.iTrayType[eAuto2        ]=tTrayAuto;
    Prod.iTrayType[eAuto3        ]=tTrayAuto;
    Prod.iTrayType[eFix1         ]=tTrayFix;
    Prod.iTrayType[eFix2         ]=tTrayFix;
    Prod.iTrayType[eFix3         ]=tTrayFix;

    if(AUTO_EMPTY_COLOR>=3)                                                     // golden main.cpp:1421
    {
        Prod.iTrayType[eAuto4    ]=tTrayAuto;
        Prod.iTrayType[eAuto5    ]=tTrayAuto;
        if(AUTO_EMPTY_COLOR>=4)
            Prod.iTrayType[eAuto6]=tTrayAuto;
        else
            Prod.iTrayType[eAuto6]=tNotUse;

        Prod.iTrayType[eFix4     ]=tTrayFix;
        Prod.iTrayType[eFix5     ]=tTrayFix;
        Prod.iTrayType[eFix6     ]=tTrayFix;
        Prod.iTrayType[eFix7     ]=tTrayFix;
        Prod.iTrayType[eFix8     ]=tTrayFix;
        Prod.iTrayType[eFix9     ]=tTrayFix;
        Prod.iTrayType[eFix10    ]=tTrayFix;
        Prod.iTrayType[eFix11    ]=tTrayFix;
        Prod.iTrayType[eFix12    ]=tTrayFix;
    }
    else
    {
        Prod.iTrayType[eAuto4    ]=tNotUse;
        Prod.iTrayType[eAuto5    ]=tNotUse;
        Prod.iTrayType[eAuto6    ]=tNotUse;
        Prod.iTrayType[eFix4     ]=tNotUse;
        Prod.iTrayType[eFix5     ]=tNotUse;
        Prod.iTrayType[eFix6     ]=tNotUse;
        Prod.iTrayType[eFix7     ]=tNotUse;
        Prod.iTrayType[eFix8     ]=tNotUse;
        Prod.iTrayType[eFix9     ]=tNotUse;
        Prod.iTrayType[eFix10    ]=tNotUse;
        Prod.iTrayType[eFix11    ]=tNotUse;
        Prod.iTrayType[eFix12    ]=tNotUse;
    }

    //==> Ifor 20260702 : Top and Bottom AOI installed occupies Fix2/Fix3 tray slots, disable at startup default
    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)              // golden main.cpp:1455
    {
        Prod.iTrayType[eFix2]=tNotUse;
        Prod.iTrayType[eFix3]=tNotUse;
    }
    //<== Ifor 20260702

    Prod.iTrayType[eBulkBox      ]=tTrayBox;                                   // golden main.cpp:1462
    if(AUTO3_IS_MAGAZINE==1)
    {
        Prod.iTrayType[eMag1     ]=tTrayMag;
        Prod.iTrayType[eMag2     ]=tTrayMag;
        Prod.iTrayType[eMag3     ]=tTrayMag;
        Prod.iTrayType[eMag4     ]=tTrayMag;
        Prod.iTrayType[eMag5     ]=tTrayMag;
        Prod.iTrayType[eMag6     ]=tTrayMag;
        Prod.iTrayType[eMag7     ]=tTrayMag;
        Prod.iTrayType[eMag8     ]=tTrayMag;
        Prod.iTrayType[eMag9     ]=tTrayMag;
        Prod.iTrayType[eMag10    ]=tTrayMag;
        Prod.iTrayType[eMag11    ]=tTrayMag;
        Prod.iTrayType[eMag12    ]=tTrayMag;
        Prod.iTrayType[eMag13    ]=tTrayMag;
        Prod.iTrayType[eMag14    ]=tTrayMag;
    }
    else
    {
        Prod.iTrayType[eMag1     ]=tNotUse;
        Prod.iTrayType[eMag2     ]=tNotUse;
        Prod.iTrayType[eMag3     ]=tNotUse;
        Prod.iTrayType[eMag4     ]=tNotUse;
        Prod.iTrayType[eMag5     ]=tNotUse;
        Prod.iTrayType[eMag6     ]=tNotUse;
        Prod.iTrayType[eMag7     ]=tNotUse;
        Prod.iTrayType[eMag8     ]=tNotUse;
        Prod.iTrayType[eMag9     ]=tNotUse;
        Prod.iTrayType[eMag10    ]=tNotUse;
        Prod.iTrayType[eMag11    ]=tNotUse;
        Prod.iTrayType[eMag12    ]=tNotUse;
        Prod.iTrayType[eMag13    ]=tNotUse;
        Prod.iTrayType[eMag14    ]=tNotUse;
    }                                                                           // golden main.cpp:1497
    std::printf("FileRW TrayForm: Prod.iTrayType[] golden TfMain defaults applied (main.cpp:1414-1497, AUTO_EMPTY_COLOR=%d)\n",
                (int)AUTO_EMPTY_COLOR);
}


// ---------------------------------------------------------------------------
// golden TfMain::TfMain 建構子 main.cpp:1830-1897：依 AUTO_EMPTY_COLOR 設 Auto／Fix／Magazine 的「最右側位置」與數量
// （iAutoCnt、iAutoRight、iFixRight、iFixRightHalf、iMagAtAuto、iBinBoxAtFix、iMMAuto[]、iAutoIndex[]…）。原樣照抄。
// Steven 團隊 20260925：重測第一次原值存檔 Fix3/Direction 1 → 0。根因：移植樹沒有這一段（20260925 grep `iFixRight\s*=`
// 只有 cmydef.cpp:3037 的靜態初值 5 —— 那是舊 3＋3 盤型編號（eFix3 當時＝5）；現行 enum eFix3＝8，MachineType.h:1200），
// golden ReadFile :416 `for(i=0; i<=iFixRight; i++)` 停在 5（eAuto6）⇒ Fix1..3 的 "Tray Type"／"Direction" 從沒讀，
// TrayForm.Auto[eFix3].Direction 停在 0，SaveSetupFile :1576 寫回 0。golden 在這台（AUTO_EMPTY_COLOR=1）設 iFixRight=eFix3（:1878），
// 所以 golden 會讀到 Direction=1 —— 這不是 golden 行為，是移植樹少了輸入。
// 同一段的其他全域（iAutoRight :506/:524/:706、iMagAtAuto :440、iFixRightHalf :111、iAutoCnt :117）本表單也用到。
// ⚠ 這些是全機共用的出料臂全域：正式位置應是移植樹 TfMain 建構的翻譯（整合者待辦）。只設一次。
void FileRW_TrayAssignment_InitTrayLayout()
{
    static bool done = false;
    if (done) return;
    done = true;
    if(AUTO_EMPTY_COLOR>=3)                                                     //Steven 20230907 : For HT-9011UC
    {
        iAutoCnt        =6;                                                     //Steven 20230907 : 給out arm計算要放的是auto還是fix
        if(AUTO_EMPTY_COLOR>=4)
        {
            if(USE_OUT_SORT_ARM!=eartUninstall)                                 //RogerYang 20250516 add for 9046AU
            {
                iAutoRight  =eAuto6;                                            //數量不能亂動，先改回來
                iMagAtAuto  =eAuto6;
                iMMAutoRight=5;
            }
            else
            {
                iAutoRight  =eAuto6;                                            //Steven 20230907 : 給out arm計算機台到Auto最右側的位置
                iMagAtAuto  =eAuto6;                                            //Steven 20230907 : 給out arm計算要放的Magazine對應Auto位置
                iMMAutoRight=5;                                                 //Steven 20230919 : Auto最右側的陣列位置
            }
        }
        else
        {
            iAutoRight  =eAuto5;
            iMagAtAuto  =eAuto5;
            iMMAutoRight=4;                                                     //Steven 20230919 : Auto最右側的陣列位置
        }

        iFixCnt         =6;                                                     //Steven 20230907 : 給out arm計算要放的Fix總數量
        iFixMin         =eFix1;                                                 //Steven 20230907 : 給out arm計算要放的fix盤起始位置
        iFixMax         =eFix6;                                                 //Steven 20230907 : 給out arm計算要放的fix盤結束位置
        iFixRight       =eFix6;                                                 //Steven 20230907 : 給out arm計算機台到Fix最右側的位置
        iFixRightHalf   =eFix12;                                                //Steven 20230907 : 給out arm計算機台到Fix最右側的位置(分半盤)
        iFixPosMin      =ePosFix1;                                              //Steven 20230907 : 給out arm計算要放的fix盤起始位置
        iFixPosMax      =ePosFix6;                                              //Steven 20230907 : 給out arm計算要放的fix盤結束位置
        iFixPosHalf     =ePosFix12;
        iMMFixRight     =5;                                                     //Steven 20230919 : Fix最右側的陣列位置
        iMMAoi          =eFix2;                                                 //Steven 20230919 : AOI的陣列位置
        iMMBinBox       =2;                                                     //Steven 20230907 : Bin Box在陣列的位置
        iBinBoxAtFix    =eFix3;
        iMagMin         =eMag1;                                                 //Steven 20230907 : Bin Box在Fix Tray的位置  0=Auto 1
        iMagMax         =eMag14;                                                //Steven 20230907 : Bin Box在Fix Tray的位置  0=Auto 1
    }
    else
    {
        iAutoCnt        =3;
        iAutoRight      =eAuto3;                                                //Steven 20230907 : 給out arm計算機台到Auto最右側的位置
        iMMAutoRight    =2;                                                     //Steven 20230919 : Auto最右側的陣列位置
        iFixCnt         =3;                                                     //Steven 20230907 : 給out arm計算要放的Fix總數量
        iFixMin         =eFix1;                                                 //Steven 20230907 : 給out arm計算要放的fix盤起始位置
        iFixMax         =eFix3;                                                 //Steven 20230907 : 給out arm計算要放的fix盤結束位置
        iFixRight       =eFix3;                                                 //Steven 20230907 : 給out arm計算機台到Fix最右側的位置
        iFixRightHalf   =eFix6;                                                 //Steven 20230907 : 給out arm計算機台到Fix最右側的位置(分半盤)
        iFixPosMin      =ePosFix1;                                              //Steven 20230907 : 給out arm計算要放的fix盤起始位置
        iFixPosMax      =ePosFix3;                                              //Steven 20230907 : 給out arm計算要放的fix盤結束位置
        iFixPosHalf     =ePosFix6;
        iMMFixRight     =2;                                                     //Steven 20230919 : Fix最右側的陣列位置
        iMMAoi          =eFix2;                                                 //Steven 20230919 : AOI的陣列位置
        iBinBoxAtFix    =eFix3;                                                 //Steven 20230907 : Bin Box在Fix Tray的位置
        iMMBinBox       =2;                                                     //Steven 20230907 : Bin Box在陣列的位置
        iMagMin         =eMag1;                                                 //Steven 20230907 : Bin Box在Fix Tray的位置  0=Auto 1
        iMagMax         =eMag14;                                                //Steven 20230907 : Bin Box在Fix Tray的位置  0=Auto 1
        iMagAtAuto      =eAuto3;                                                //Steven 20230907 : 給out arm計算要放的Magazine對應Auto位置

        iMMAuto[eFix4]  =MManualTray1;
        iMMAuto[eFix5]  =MManualTray2;
        iMMAuto[eFix6]  =MManualTray3;
        iAutoIndex[eFix4]=0;
        iAutoIndex[eFix5]=1;
        iAutoIndex[eFix6]=2;
    }
    std::printf("FileRW TrayForm: tray layout globals set from golden main.cpp:1830-1897 (iFixRight=%d iAutoRight=%d)\n", iFixRight, iAutoRight);
}

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// 沒寫檔時把替身還原成檔案值 —— golden FormClose（:1325）的資料那一半（ReadFile＋DoIniDataToForm）；
// 不做它的 rgLoaderTrayMode->Enabled=true（權限只由 FormShow 決定，下次存檔仍沿用開頁時的權限）
void Reload()
{
    TA_ReadFile();
    TA_DoIniDataToForm();
}

const filerw::PageDesc kPage = {
    "TrayForm", "TfTrayAssignment", "Setup.TrayAssignment.html",
    nullptr, nullptr, 0,
    kTA_SaveReads, (int)(sizeof(kTA_SaveReads) / sizeof(kTA_SaveReads[0])),
    &TA_FormShow, &TA_spbSaveClick, "SaveSetupFile", &Reload, &Booted,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfTrayAssignment 建構（HT9045.cpp:188）：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身
void FileRW_TrayAssignment_Boot()
{
    if (g_booted) return;
    FileRW_TrayAssignment_InitProdTrayType();   // golden TfMain 建構 main.cpp:1414-1497（早於 CreateForm(TfTrayAssignment) :188）
    FileRW_TrayAssignment_InitTrayLayout();     // golden TfMain 建構 main.cpp:1830-1897（同上）
    // TScrollBar 替身先以 ELTrackBar 型別建（之後產生碼的 EL<TControl>() 取到的是同一個物件，基底轉型合法）。
    // golden DFM：sbNormalTest／sbNormalTest_RT Min=0（預設）Max=15 Position=0（預設），OnChange＝sbNormalTest*Change。
    {
        filerw::ELTrackBar* a = EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest");
        a->DfmInit(0, 15, 0);
        a->OnChange = &TA_sbNormalTestChange;
        filerw::ELTrackBar* b = EL<filerw::ELTrackBar>("TfTrayAssignment", "sbNormalTest_RT");
        b->DfmInit(0, 15, 0);
        b->OnChange = &TA_sbNormalTest_RTChange;
    }
    TA_DfmItems();
    TA_DfmState();
    // golden DFM cTrayAssignment.dfm:253 pgRunMode ActivePage = tsReTestGroup（頁序：0 tsNormalTestGroup、1 tsReTestGroup、
    // 2 tsNormalTestGraph、3 tsReTestGraph）。產生器的 DFM 設計期值不收 ActivePage。
    EL<TPageControl>("TfTrayAssignment", "pgRunMode")->ActivePageIndex = 1;
    TA_TfTrayAssignment();
    TA_CreateSaveProxies();
    TA_CreateContainerProxies();
    std::printf("FileRW TrayForm: TfTrayAssignment proxies ready (%d save reads) -- golden cTrayAssignment.cpp\n",
                (int)(sizeof(kTA_SaveReads) / sizeof(kTA_SaveReads[0])));
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:8925 fTrayAssignment->ReadFile() 的 golden 912 版。
// Steven 20260925 裁決：取代移植樹 906 版——wb_serve 把 forms/fTrayAssignment.cpp 的 g_W906_TrayAssignmentReadFileHook
// 指到這裡，所以開機、reload 鏈、cinitial.cpp:9342（每次 SetWorkParameter）都走這支。
void FileRW_TrayAssignment_ReadFile()
{
    if (!g_booted) return;
    TA_ReadFile();
    static int n = 0;
    if (n++ == 0)
        std::printf("FileRW TrayForm: TfTrayAssignment::ReadFile -> golden 912 reader (Tray.Data; LodareType=%d)\n", (int)TrayForm.LodareType);
}
