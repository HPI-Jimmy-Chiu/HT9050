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
#include <cstring>
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"
#include "Public/cJSON.h"   // AI(W906-FRW-S158) 20260927 [W906]：BeforeApply 讀頁面送的 widgets（同 FileRW/_EditPage.cpp）
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbTrayAssignClickTail（FileRW/MainClick.cpp；Q41 第 3 項 TA-6；只有宣告）；佔用原本的空行
// pgRunMode 目前分頁（golden ActivePage／ActivePageIndex）。頁面不送分頁 → 值是 DFM 的 tsReTestGroup（開機設）
// 或 golden 程式設的值。走函式而不是 EL<>()->ActivePageIndex：產生器會把後者算成「存檔讀的替身」（mustSend），
// 但 TPageControl 沒有頁面可送的值種類，頁面就永遠存不了檔。
static int TA_RunModePageIndex()
{
    return EL<TPageControl>("TfTrayAssignment", "pgRunMode")->ActivePageIndex;
}

// VCL TCustomRadioGroup.SetItemIndex（非讀 DFM 時）：v < -1 → -1；v >= Items.Count → Count-1。
// golden GraphicToRadio 的 Position&0x08 等給 2／4／8，VCL 靠這個夾回 0..Count-1；vclcompat TRadioGroup 不夾。
//AI(W906-EVB10B) 20260929 [W906] X-3（R100＝照 BCB）：值有變也照 VCL 觸發 DFM OnClick（filerw::ELClickIndex，登記在 TA_DfmState，
//   tools/editlist/TrayForm.py 'vcl_clicks'）—— 以前只夾不觸發（圖像模式拖捲軸 sbNormalTestChange → GraphicToRadio 設的 RGAuto1..3／
//   RGLoader 在 golden 會跑 cbEmptyChange／RGAuto2Click／RGLoaderClick）。夾值照舊先做（沒登記的群組 ELClickIndex 也夾）。
static void TA_SetRadioIndex(TRadioGroup* g, int v)
{
    if (v < -1) v = -1;
    if (v >= g->Items->Count) v = g->Items->Count - 1;
    filerw::ELClickIndex(g, v);
}

// ---------------------------------------------------------------------------
// AI(W906-FRW-S158) 20260927 [W906]：WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157；Q41 盤點 TA-1～TA-4，
// D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md §3.4）。golden 處理器由 tools/gen_editlist.py
// （tools/editlist/TrayForm.py 的 methods／events）轉進 TrayForm.gen.inc；這裡是它們要的三支小工具＋事件表註冊。
// ---------------------------------------------------------------------------

// TA-1 的 13 張方向圖，順序＝golden ShowTrayDirectIMG 的 MyImage[]（cTrayAssignment.cpp:1212-1213）＝ golden DFM Tag
// （imgLoader 0、imgAuto1..6 1..6、imgFix1..6 7..12，cTrayAssignment.dfm；golden 執行期從不改圖片的 Tag）＝ iTrayDirect[] 的索引。
// 這一頁的替身 Tag 不放這個索引，放「目前載入的圖號」（golden Picture＝type<n>.bmp 的 n）—— 理由見 tools/editlist/TrayForm.py 的 _IMG 註解。
static const char* const kTA_ImgName[13] = {
    "imgLoader", "imgAuto1", "imgAuto2", "imgAuto3", "imgAuto4", "imgAuto5", "imgAuto6",
    "imgFix1", "imgFix2", "imgFix3", "imgFix4", "imgFix5", "imgFix6",
};

static TControl* TA_Img(int i)
{
    return EL<TControl>("TfTrayAssignment", kTA_ImgName[i]);
}

// golden imgLoaderClick :976／:980 的 Ptr->Tag（DFM 設計期 Tag）
static int TA_ImgIndex(TObject* Sender)
{
    for (int i = 0; i < 13; ++i)
        if (filerw::ELFind("TfTrayAssignment", kTA_ImgName[i]) == Sender) return i;
    return -1;
}

// VCL TCustomRadioGroup.SetItemIndex（非讀 DFM 時）：夾在 -1..Items.Count-1；值有變 → TGroupButton.Checked:=True → Click
// → TCustomRadioGroup.ButtonClick → Click → OnClick（同 tools/editlist/TestIF_File_Cleaning.py 的 CL_RadioIndex、HSys.py:116）。
// OnClick 照 golden cTrayAssignment.dfm。只給本波新轉的處理器用（TrayForm.py 的 _EV_REPLACE）；既有的 TA_SetRadioIndex 不觸發（見交件）。
//AI(W906-EVB10B) 20260929 [W906] X-3：⛔ 更正 —— OnClick 對照表搬到產生器（TA_DfmState 照 golden DFM 登記 16 個 RadioGroup＋cbEnableAMR_KYEC，
//   'vcl_clicks'），這裡改走同一個登記表（filerw::ELClickIndex：夾值、值有變且 >=0 才 OnClick —— VCL 設成 -1 不 Click，以前這裡會）；
//   TA_SetRadioIndex 也觸發了，DoIniDataToForm／FormShow／存檔鈕／FormClose 的產生碼直接呼叫 ELClickIndex。
static void TA_RadioIndex(TRadioGroup* g, int v)
{
    filerw::ELClickIndex(g, v);
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
    // AI(W906-FRW-S158) 20260927 [W906]：13 張方向圖現在是替身（Tag＝圖號，TA-1）→ 也還原成檔案的方向。golden FormClose 沒有這一行
    //   （關窗後圖看不到，下次 FormShow :751 才重畫）；這裡頁面沒關、存檔失敗後可能直接再送 form.event，所以一起還原。
    TA_ShowTrayDirectIMG();
}
void SaveFlow();   // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden 存檔鈕＋TfMain::sbTrayAssignClick 關窗尾段，R85）；佔用原本的空行
// ---------------------------------------------------------------------------
// AI(W906-FRW-S158) 20260927 [W906]：editlist.save 套值前（_EditPage.h PageDesc::beforeApply）——頁面「沒送 form.event 就直接存」
// 時的兩道防線。頁面有送 form.event 時，伺服器端的值已經是 golden 處理器跑過的結果，頁面送回同一個值 ⇒ 兩道都不動作。
//   (1) TA-3 rgFixTrayMode：golden 使用者只能用點的換模式，點了就跑 rgFixTrayModeClick（:1166），Fix 盤有 IC 就改回
//       TrayForm.iFixTrayMode（:1182-1194）。頁面值跟伺服器端不同 ⇒ 照 VCL 點選（設 ItemIndex）＋跑 golden 處理器；
//       被改回時多記一句訊息（golden 沒有訊息，靠 labFix1..6 顯示「有 IC」；頁面存完會重讀、看得到 labFix）。
//       這是機台狀態（MOT[MManualTray1..6].TrayFeedHasIC()）的互鎖 —— 不信任前端，存檔一定重查。
//   (2) TA-1 方向圖：頁面引擎對 TImage 替身本來就會「點一下 tag+1（0..7 循環）」、存檔送 tag
//       （web/page/ht9045_wire_engine.js:1166-1175、:1246）。頁面值跟 iTrayDirect[] 不同 ⇒ 照 golden 一下一下點
//       （imgLoaderClick，最多 8 下；0..7 以外點不到 → 不動、記 todo）。點圖只改自己那一格（:976-984），跟點的先後無關。
//   其他有連動的控制項（rgLoaderType 的 KYEC 條碼、RGLoader／rgLoad_RT／RGAuto2／rgAuto2_RT 的 bin 檢查）不在這裡重播：
//   golden 的結果跟點的先後有關（BinTrayDetect :1753-1762 改回的是第一個值為 2 的群組），伺服器只看得到最後的值 ⇒ 列給 Steven（交件 R 題）。
// ---------------------------------------------------------------------------
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {   // PageSave 已驗過是物件；保險
        if (root) cJSON_Delete(root);
        return;
    }
    // (1) TA-3
    {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, "rgFixTrayMode");
        const cJSON* v = cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, "itemIndex") : nullptr;
        TRadioGroup* g = EL<TRadioGroup>("TfTrayAssignment", "rgFixTrayMode");
        if (v && cJSON_IsNumber(v) && v->valueint != g->ItemIndex && v->valueint >= -1 && v->valueint < g->Items->Count &&
            filerw::ELOperable("TfTrayAssignment", "rgFixTrayMode")) {   // 點不到（看不見／停用）的交給 PageSave 丟掉（ack.ignored）
            const int want = v->valueint;
            g->ItemIndex = want;                                         // VCL：使用者點選項 ⇒ ItemIndex 變 → OnClick
            TA_rgFixTrayModeClick();                                     // golden cTrayAssignment.cpp:1166
            if (g->ItemIndex != want)
                filerw::ELMessage("A Fix tray still has IC: Fix Tray Mode was not switched (BCB6 rgFixTrayModeClick puts it back).",
                                  "Fix 盤上還有 IC：Fix Tray Mode 沒有切換（BCB6 rgFixTrayModeClick 會改回原值）。");
            handled->push_back("rgFixTrayMode");
        }
    }
    // (2) TA-1
    for (int i = 0; i < 13; ++i) {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, kTA_ImgName[i]);
        const cJSON* v = cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, "tag") : nullptr;
        if (!v || !cJSON_IsNumber(v)) continue;                          // 沒送／型別不對 → PageSave（型別不對回 400）
        const int want = v->valueint;
        if (want == iTrayDirect[i] || !filerw::ELOperable("TfTrayAssignment", kTA_ImgName[i])) continue;
        handled->push_back(kTA_ImgName[i]);
        if (want < 0 || want > 7) {
            filerw::ELTodo("golden cTrayAssignment.cpp:977-979 imgLoaderClick cycles 0..7 -- the page sent a direction outside 0..7, not changed");
            continue;
        }
        for (int k = 0; k < 9 && iTrayDirect[i] != want; ++k) TA_imgLoaderClick(TA_Img(i));   // golden :971（iTrayDirect＝8 時第一下回 0）
    }
    cJSON_Delete(root);
}

const filerw::PageDesc kPage = {
    "TrayForm", "TfTrayAssignment", "Setup.TrayAssignment.html",
    nullptr, nullptr, 0,
    kTA_SaveReads, (int)(sizeof(kTA_SaveReads) / sizeof(kTA_SaveReads[0])),
    &TA_FormShow, &SaveFlow, "SaveSetupFile", &Reload, &Booted,   // AI(W906-FRW-S158) 20260927 [W906]: saveFlow &TA_spbSaveClick → &SaveFlow（檔尾：存檔鈕之後補關窗尾段；上面的 BeforeApply 照舊在套值前跑）；同一行改寫
    &BeforeApply, nullptr,   // AI(W906-FRW-S158) 20260927 [W906]：beforeApply（上面）；extraJson 不用
};
filerw::PageRegistrar g_reg(&kPage);

// ---------------------------------------------------------------------------
// AI(W906-FRW-S158) 20260927 [W906]：WS form.event 事件表註冊（Steven ★ Q40＝A）。表是產生的 kTA_Events（TrayForm.gen.inc 檔尾，
// tools/editlist/TrayForm.py 'events'），照抄一份；只有 pgRunMode 分頁上的 14 個 RadioGroup 換成 TA_EvOnTab：
//   golden 使用者只點得到「目前分頁」上的元件 ⇒ 點 RGLoader／RGAuto1..6（tsNormalTestGroup）時 ActivePageIndex 一定是 0、
//   點 rgLoad_RT／rgAuto1..6_RT（tsReTestGroup）時一定是 1。處理器會看分頁（RGLoaderClick :1235-1236 的 FT 連動、
//   rgLoad_RTClick :1277-1278 的 RT 連動、ShowCompnet :1033-1050 的 edAutoNType），但頁面不送分頁、伺服器端一直是 DFM 的
//   tsReTestGroup（本檔 FileRW_TrayAssignment_Boot）⇒ 不換的話點 RGLoader 的 Empty／Color 連動永遠不會發生。
//   處理器跑完把分頁還原（存檔鈕 :1355 的 FT→RT 複製看分頁，不能因為點過 FT 頁就改變存檔行為）。
//   cbLoader（分頁外）跑的 rgLoad_RTClick＋RGLoaderClick 看的是「使用者當下在哪一頁」—— 伺服器不知道，照 DFM 的 1（交件：機制缺口）。
// ---------------------------------------------------------------------------
const int kTA_NEvents = (int)(sizeof(kTA_Events) / sizeof(kTA_Events[0]));
const struct { const char* name; int page; } kTA_OnTab[] = {
    {"RGLoader", 0}, {"RGAuto1", 0}, {"RGAuto2", 0}, {"RGAuto3", 0}, {"RGAuto4", 0}, {"RGAuto5", 0}, {"RGAuto6", 0},   // tsNormalTestGroup
    {"rgLoad_RT", 1}, {"rgAuto1_RT", 1}, {"rgAuto2_RT", 1}, {"rgAuto3_RT", 1}, {"rgAuto4_RT", 1}, {"rgAuto5_RT", 1}, {"rgAuto6_RT", 1}, {"sbNormalTest", 2}, {"sbNormalTest_RT", 3},   // tsReTestGroup   //AI(W906-TA5) 20261001 [W906]: + the two graphic-mode scroll bars (golden cTrayAssignment.dfm:569 sbNormalTest on tsNormalTestGraph = page 2, :592 sbNormalTest_RT on tsReTestGraph = page 3). Same reason as the 14 groups: the golden handler's OnClick chain reads the page -- GraphicToRadio sets RGLoader -> RGLoaderClick :1235-1236 (page 0||2), GraphicToRadio_RT sets rgLoad_RT -> rgLoad_RTClick :1277-1278 (page 1||3), and GraphicToRadio_RT :1687 copies sbNormalTest only when the page is tsNormalTestGraph (never true for a user dragging sbNormalTest_RT on page 3). Same line, nothing below moves
};
int TA_PageOf(const char* control)
{
    for (std::size_t i = 0; i < sizeof(kTA_OnTab) / sizeof(kTA_OnTab[0]); ++i)
        if (std::strcmp(kTA_OnTab[i].name, control) == 0) return kTA_OnTab[i].page;
    return -1;
}
void TA_EvOnTab(TControl* Sender)
{
    for (int i = 0; i < kTA_NEvents; ++i) {
        if (filerw::ELFind("TfTrayAssignment", kTA_Events[i].control) != Sender) continue;
        TPageControl* pc = EL<TPageControl>("TfTrayAssignment", "pgRunMode");
        const int old = pc->ActivePageIndex;
        pc->ActivePageIndex = TA_PageOf(kTA_Events[i].control);
        try {
            kTA_Events[i].handler(Sender);
        } catch (...) {
            pc->ActivePageIndex = old;
            throw;
        }
        pc->ActivePageIndex = old;
        return;
    }
}
filerw::PageEvent g_events[sizeof(kTA_Events) / sizeof(kTA_Events[0])];
const filerw::PageEvent* Events()
{
    for (int i = 0; i < kTA_NEvents; ++i) {
        g_events[i] = kTA_Events[i];
        if (TA_PageOf(kTA_Events[i].control) >= 0) g_events[i].handler = &TA_EvOnTab;
    }
    return g_events;
}
filerw::PageEventsRegistrar g_evreg("TrayForm", Events(), kTA_NEvents);
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
    // AI(W906-FRW-S158) 20260927 [W906]：TA_DfmState 把 form.event 控制項的 DFM Tag 帶進替身（產生器的規則；13 張方向圖是 1..12）。
    //   這一頁方向圖替身的 Tag 意思是「目前載入的圖號」（TA-1，見 kTA_ImgName 註解），golden 的索引由 TA_ImgIndex 查表 ⇒ 開機先照
    //   golden DFM 的圖（type0）歸 0；開頁 FormShow :745 LoadImage／:751 ShowTrayDirectIMG 再換成檔案的方向。
    for (int i = 0; i < 13; ++i) TA_Img(i)->Tag = 0;
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

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 第 3 項 TA-6（decisions-pending R85＝A：RULINGS_20260926 S107-1「存檔後就跑，不改成等 Exit」
//    的延伸）—— PageDesc::saveFlow 包一層：golden 存檔鈕 spbSaveClick（cTrayAssignment.cpp:1337）跑完之後，補 golden 主畫面
//    TfMain::sbTrayAssignClick 的關窗尾段（V912 main.cpp:28434-28437：DoStructUnitConvert、SetWorkParameter、fShowBinSelect->ShowBinSel、
//    fSortCT->UpForm；本體 FileRW/MainClick.cpp W906_Main_sbTrayAssignClickTail，運轉中不跑 R86）。照 FileRW/Ld_UldDelayTime.cpp 的做法：
//      (1) A02 權限不足（"closed"）—— golden :1343 Close() → FormClose（cTrayAssignment.cpp:1325：rgLoaderTrayMode->Enabled=true、ReadFile、
//          DoIniDataToForm、fShow=false）→ ShowModal 回來 → 尾段：與 golden 相同。跑的是完整的 golden FormClose（TA_FormClose），不是上面的
//          Reload()：多出來的 rgLoaderTrayMode->Enabled=true 無害 —— closed 之後 PageSave 會清掉「開過頁」
//          （FileRW/_EditPage.cpp PageSave 的 closed → shown=false），下一次存檔前一定重新 FormShow（golden :873-930 依開門／權限重設 Enabled），不會帶到沒開頁的存檔。ReadFile 先把 TrayForm 還原成檔案值，
//          尾段的 ShowBinSel 才不會用到 BeforeApply（TA-1／TA-3）改過、但沒存的方向或 Fix 模式。方向圖替身由 PageSave 接著跑的 Reload()
//          還原（PageSave 的 `if (!saved) d.reload();`；重讀兩次無害）。
//      (2) 真的寫了檔（"SaveSetupFile"）—— golden 存完視窗還開著、等 Exit 才跑 FormClose＋尾段；網頁 Exit 不送伺服器 → 存完就跑（S107-1）。
//          FormClose 不跑（頁面還開著；SaveSetupFile 最後已 ReadFile，cTrayAssignment.cpp:1629）。
//    差別：golden「開頁、沒存就關」也跑尾段；這裡不跑（值沒變，結果相同）。
//    ⚠ ShowBinSel 重算分 bin 用的 iBinTray 等（見本體註解），START 時 golden 不會重跑 —— 這一頁最需要尾段。
//    BeforeApply（上面，TA-1／TA-3 重播）與 form.event 事件表不經 saveFlow，照舊。描述檔 tools/editlist/TrayForm.py 不改、gen.inc 不重產。
// ---------------------------------------------------------------------------
namespace {
void SaveFlow()
{
    TA_spbSaveClick();                                                          // golden cTrayAssignment.cpp:1337
    if (filerw::ELMarked("closed")) {
        TA_FormClose();                                                         // golden cTrayAssignment.cpp:1325
        if (const char* w = W906_Main_sbTrayAssignClickTail()) filerw::ELTodo(w);   // golden main.cpp:28434-28437
    } else if (filerw::ELMarked("SaveSetupFile")) {
        if (const char* w = W906_Main_sbTrayAssignClickTail()) filerw::ELTodo(w);   // golden main.cpp:28434-28437（時機見上）
    }
}
}  // namespace

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 9 列；Steven 20260928「如果已經有移植, 就接上」、
//    20260929「照 BCB 的邏輯」）。golden TfTrayAssignment::FormClose（V912 cTrayAssignment.cpp:1325-1335）：
//    rgLoaderTrayMode->Enabled=true; ReadFile(); DoIniDataToForm(); fShow=false; ＝產生檔的 TA_FormClose。
//    重讀 <配方>\Tray.Data（＋P46 時 config.ini 的 Skip Manual Remove Tray）進 TrayForm.*，並照 golden ReadFile 重算 Prod.iTrayType[eFix*]
//    ＝丟掉這一次開窗裡 form.event（rgFixTrayModeClick 等）改過、但沒存的值；跟開頁 FormShow 的 ReadFile 同一段。
//  頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge）呼叫：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過
//    （filerw::PageCloseEdgeRefused；A02 的 Close() 上面 SaveFlow 已經跑過 TA_FormClose ⇒ 不跑第二次）才跑；運轉中不跑（ShowModal
//    main.cpp:28433 —— 運轉中重算 Prod.iTrayType 的正是 golden 做不到的情況）。
//  主畫面尾段 sbTrayAssignClick :28434-28437 照 R85／S107-1 留在存檔之後（上面 SaveFlow），關窗不再跑。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

const char* FileRW_TrayAssignment_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfTrayAssignment proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("TrayForm")) return no;
    TA_FormClose();                                                             // golden cTrayAssignment.cpp:1325
    return "ran golden TfTrayAssignment::FormClose (cTrayAssignment.cpp:1325-1335): rgLoaderTrayMode->Enabled=true, ReadFile (Tray.Data), DoIniDataToForm, fShow=false";
}
