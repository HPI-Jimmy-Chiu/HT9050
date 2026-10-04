// ===========================================================================
//  FileRW/IniConfig_CounterSel.cpp -- golden TfCounterSel（cCounterSel.cpp）的 C 路入口：Status.CounterSel.html 的讀寫。
//  //AI(W906-E031) 20261003 [W906] (St01)：產生器對本結構改讀 golden 906 0618（E-031 全面切換第 1 批，tools/golden_root.py）；
//    cCounterSel.cpp／.h／.dfm 0618 與 V912 逐位元組相同 ⇒ 本檔的 cCounterSel.cpp 行號兩棵都對。下面檔頭其他 golden 檔的引用改成 0618、
//    括號裡是 V912（逐一照內容在兩棵找過；幾個舊號碼已過時，括號裡寫明）；「移植樹 X:N」是移植樹的行號，不動。
//  頁面補件：web/page/ht9045_countersel_c.js。
//
//  AI(W906-CRT-CounterSel) 20260926: 新檔（Steven 團隊）。設定：tools/editlist/IniConfig_CounterSel.py（每條 replace／block 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 IniConfig_CounterSel.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    建構子（:23）   NeedRef=false（門面 fCounterSel->NeedRef）。
//    FormShow（:29）＝開頁：ProcessLastSetIni_Visible(bReadFile)（cprod.cpp:2849，golden cprod.cpp:2702，V912 :2721）讀 config.ini [Visible]
//      11 鍵進 IniConfig.bShow*／iShowCateByArm（缺鍵時 golden CheckAndReadIniData 會用 LastSet.* 預設值補寫該鍵——golden 開頁本來就這樣）
//      → 4 個 TCheckBox、6 對 TRadioButton（On／Off）、rgTestCategory。
//    FormClose（:56）＝存檔：golden 沒有存檔鈕，Exit（spbExitClick :145 → Close()）觸發 OnClose=FormClose；替身 → IniConfig.bShow*
//      → fShowBinSelect->Tab_UPH->TabVisible（:77）→ ContactHeight 關掉時清 fMain->lbArm0Torque（:79-83）
//      → cbDefaultValue 勾選時清掉勾選（:90；FormPos.def 那段擋掉）→ ProcessLastSetIni_Visible(bWriteFile)（:106，無條件寫 11 鍵）
//      → NeedRef=true。golden 沒有權限守衛、沒有確認框、沒有 A02。
//    CheckFormIni（:111）不轉：本頁唯一的呼叫點（:91）連同 FormPos.def 一起擋掉（見 .py 檔頭）。
//  savedMark＝"FormClose"：golden FormClose 無條件寫檔。mustSend＝FormClose 讀的 12 個替身（沒有 HTEditList）。
//  reload＝golden FormShow（重讀 config.ini [Visible]）。
//
//  ---- 檔案擁有者 ----
//    config.ini 歸 FileRW/IniConfig.cpp（tools/wb_serve.cpp CRouteOwner kOwned 第一筆）；golden SaveLastSetIni（cprod.cpp:3281，
//    golden cprod.cpp:3092，V912 :3111）也寫同一個 [Visible]。兩條 C 路都經同一份記憶體 IniConfig，各自先 FormShow 讀檔再寫，不互蓋。
//    kOwned 的 config.ini 說明字串要加上本檔（插入片段見交件報告，共用檔由主 session 改）。
//
//  ---- bShow* 的讀者（主畫面哪些顯示會變；golden（20260926 grep 的是 V912，E-031 改成 0618、括號 V912）→ 移植樹）----
//    bShowLoaderCT／bShowContactCT／bShowBinCT  golden main.cpp:8729-8774（V912 :9154-9199）DoShowUserDefFrom：fSortCT／fContactCT／fShowBinSelect 開或關
//                                               → 移植樹沒有 DoShowUserDefFrom（IniConfig.gen.inc 同樣記 todo）；網頁視窗由 background.html 開關
//    bShowTestCate／iShowCateByArm              golden main.cpp:8745（V912 :9170）同上＋cTestCategory.cpp:513 SetShowCateMode
//                                               → 移植樹只在開機跑一次（cTestCategory.cpp:643 W906_BootTestCategory → tag tcat.show／tcat.cateByArm）
//    bShowUPH                                   golden cShowBinSelect.cpp:802（V912 :863；舊寫的 :857 已過時）Tab_UPH->TabVisible → 移植樹 cShowBinSelect.cpp:2171；本頁存檔 :77 也設
//    bShowIndexTime                             golden csystem.cpp:19766（V912 :20760）ShowIndexTime → 移植樹 csystem.cpp:23983（主畫面狀態列那段在移植樹 GATE H1-05）
//    bShowContactHeight                         golden cinitial.cpp:13859／:13900（V912 :13871／:13912）扭力欄字串 → 移植樹 cinitial.cpp:19090／:19137（fMain->lbArm0Torque）
//    bShowTimeInfo                              golden cObserver.cpp:2109（V912 :2268；這個號碼本來就是 906 的）、FormClose :74-75 → 移植樹沒有讀者
//    bShowTemper                                golden cTemperFrom.cpp:1825／:1833（V912 :1835／:1843）→ 移植樹沒有讀者
//    bShowScanCate                              golden 只有本表單與 ProcessLastSetIni_Visible 讀寫（DFM gbScanner Enabled=False，操作員改不到）
//    另：golden main.cpp:26998-27015（V912 :28017-28034；舊寫的 :27987-28004 已過時）OnHotKey Ctrl+A 會一起切 bShowUPH／bShowIndexTime／bShowTimeInfo（不寫檔）。
//  ⚠ golden 關窗後 TfMain::Timer1Timer（main.cpp:3208-3213，V912 :3299-3304）看到 NeedRef 會重跑 SetShowCateMode＋DoShowUserDefFrom；
//    移植樹沒有這個讀者 → 本頁存檔後 tcat.cateByArm 等要到下次開機才變（列給 Steven 決定，見交件報告）。
//
//  ---- 開機（golden HT9045.cpp:202（V912 :203）CreateForm(TfCounterSel)）----
//    FileRW_CounterSel_Boot()：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身 → 容器替身與父子。不讀檔（讀 [Visible] 是
//    golden ReadLastSetIni 的事，移植樹 wb_serve 開機已跑）。沒有 HTEditList，CreateForm 先後不影響註冊。冪等。
//
//  ---- 沒接的（golden 呼叫端，不是本表單）----
//    main.cpp:27568（V912 :28588；舊寫的 :28556 已過時）sbSeleteClick：fSecurity->Insufficient(25) 入口權限、NewRecordProcess("MES2181")、ShowModal 後 ShowFormPos()。
//    網頁主選單（web/page/main.html DFM_MAP sbSelete）直接開視窗，所有 C 路頁面都沒有接主畫面入口鈕的權限閘（列給 Steven 決定）。
// ===========================================================================
#include "FileRW/IniConfig_CounterSel.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"
#include "forms/fTestCategory.h"   // AI(W906-CSEL-NEEDREF) 20261001: fTestCategory (golden main.cpp:3211 / :8744-8752, see FileRW_CounterSel_NeedRefTick)
namespace {
bool g_booted = false;
bool Booted() { return g_booted; }  void EvB10CSaveFlow();   //AI(W906-EVB10C) 20260929 [W906]: saveFlow 包一層（檔尾：golden FormClose＋記「這一次開窗 FormClose 跑過了」）；同一行附加

const filerw::PageDesc kPage = {
    "IniConfig_CounterSel", "TfCounterSel", "Status.CounterSel.html",
    nullptr, nullptr, 0,
    kCSL_SaveReads, (int)(sizeof(kCSL_SaveReads) / sizeof(kCSL_SaveReads[0])),
    &CSL_FormShow, &EvB10CSaveFlow, "FormClose", &CSL_FormShow, &Booted,   //AI(W906-EVB10C) 20260929 [W906]: saveFlow &CSL_FormClose → &EvB10CSaveFlow（檔尾，仍是 golden FormClose）；同一行改寫
    nullptr, nullptr,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfCounterSel 建構（HT9045.cpp:202 CreateForm，V912 :203）：DFM 設計期狀態 → 建構子（:23）→ 存檔流程讀的替身 → 容器替身與父子。不讀檔。冪等。
void FileRW_CounterSel_Boot()
{
    if (g_booted) return;
    CSL_DfmItems();
    CSL_DfmState();
    CSL_TfCounterSel();
    CSL_CreateSaveProxies();
    CSL_CreateContainerProxies();
    std::printf("FileRW IniConfig_CounterSel: TfCounterSel proxies ready (%d save reads) -- golden cCounterSel.cpp\n",
                (int)(sizeof(kCSL_SaveReads) / sizeof(kCSL_SaveReads[0])));
    g_booted = true;
}

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 17 列；ST01-E 20260929 B1 放行「照 BCB」）。
//  golden TfCounterSel 沒有存檔鈕、也沒有 ✕（cCounterSel.dfm:4 BorderIcons=[]）：唯一的關法是 Exit（spbExitClick :145 → Close()）→
//    OnClose＝FormClose（:56-109）＝存檔（檔頭）。網頁：
//    (1) Exit 鈕 ＝ editlist.save（D:\HT9045\web\page\ht9045_countersel_c.js 攔下 .exitbtn）→ saveFlow＝下面 EvB10CSaveFlow：golden FormClose，
//        再記 filerw::PageFormCloseRan ⇒ 存完頁面關視窗（或引擎自動重讀之後才關）時，關窗邊緣不再跑第二次；
//    (2) 外框的 ✕（golden 沒有）＝沒送存檔就關 ⇒ 頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge）呼叫
//        FileRW_CounterSel_WindowEdge：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過（filerw::PageCloseEdgeRefused）才跑 golden
//        FormClose —— golden 關這個視窗一定跑它。運轉中不跑（ShowModal main.cpp:27575，V912 :28595；舊寫的 :28565 已過時）。
//  (2) 跑的是伺服器端替身（＝開頁 FormShow 讀的 config.ini [Visible] 值；網頁沒存的勾選伺服器看不到）⇒ 寫回的是同一份值。
//    不是同值的副作用（照 golden）：config.ini [Visible] 11 鍵重寫（ProcessLastSetIni_Visible(bWriteFile)，寫真檔）；
//    fShowBinSelect->Tab_UPH->TabVisible＝bShowUPH；bShowContactHeight 關著時清 fMain->lbArm0Torque；NeedRef=true；
//    cbDefaultValue：FormShow 不設它、FormClose 每次清成 false ⇒ ✕ 時伺服器端替身是 false，:87-104 不進去（FormPos.def 那段本來就閘掉）。
//    例外：開著視窗時 golden OnHotKey Ctrl+A（main.cpp:26998-27015，V912 :28017-28034，只改記憶體）切過 bShowUPH／bShowIndexTime／bShowTimeInfo 的話，
//    golden 關窗會照畫面上的勾選（＝開窗時的值）寫回、把它切回來 —— 這裡一樣。
//  主畫面尾段 sbSeleteClick :28567 ShowFormPos()（BCB 視窗位置）網頁不用，照檔頭不接。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

// AI(W906-CSEL-NEEDREF) 20261001: golden TfMain::Timer1Timer (906 main.cpp:3208-3213) reads NeedRef on its next tick after this form
//   closed: `if(fCounterSel->NeedRef){ fCounterSel->NeedRef=false; fTestCategory->SetShowCateMode(); DoShowUserDefFrom(); }`.
//   The port had no reader (cTestCategory.cpp:639-641), so rgTestCategory (Normal / By Arm) and rbTestCategory_On/Off only took
//   effect at the next boot. Run it here, right after FormClose (golden: the next 30 ms main tick). Only the halves that live in C++:
//   SetShowCateMode (bCateByArm / Height -> tags tcat.cateByArm / tcat.height) and DoShowUserDefFrom's fTestCategory branch
//   (906 main.cpp:8744-8752 -> FormShow / FormClose -> tag tcat.show). The main-screen windows DoShowUserDefFrom shows or closes
//   (fSortCT / fTestCategory / fContactCT / fShowBinSelect, :8734-8774) are web windows: ht9045_countersel_c.js asks background.html
//   to open / close them after the save; fTemperFrom Close/Show and fShowMessage->ShowMyMessage only re-place always-open windows.
//   Exported (not in the anonymous namespace) so tests/test_compk_datas.cpp can drive it.
void FileRW_CounterSel_NeedRefTick()
{
    if (!NeedRef || fTestCategory == nullptr)
        return;
    NeedRef=false;                                                              // golden main.cpp:3210
    fTestCategory->SetShowCateMode();                                           // golden main.cpp:3211
    if(IniConfig.bShowTestCate)                                                 // golden main.cpp:8744-8752 (DoShowUserDefFrom)
    {   bool W906_FormShowing(const char* goldenForm, bool member);   // AI(W906-MACH1002) 20261002: W906FormShowing.h (csystem.cpp body) -- the line below reads the show state through it
        if(W906_FormShowing("fTestCategory", fTestCategory->bShow)==false)   // AI(W906-MACH1002) 20261002: FShow_Audit -- golden reads bShow; the web window (background.html form:fTestCategory) counts too; ctest / no wb_serve = the member as before
            fTestCategory->FormShow(nullptr);                                   // Show() -> OnShow = FormShow
    }
    else
    {
        fTestCategory->FormClose();                                             // Close() -> OnClose = FormClose
    }
}

namespace {
void EvB10CSaveFlow()
{
    CSL_FormClose();                                                            // golden cCounterSel.cpp:56（Exit → Close() → OnClose）
    filerw::PageFormCloseRan("IniConfig_CounterSel");                           // 這一次開窗 FormClose 跑過了（關窗邊緣不再跑）
    FileRW_CounterSel_NeedRefTick();                                            // AI(W906-CSEL-NEEDREF) 20261001: golden main.cpp:3208-3213
}
}  // namespace

const char* FileRW_CounterSel_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfCounterSel proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("IniConfig_CounterSel")) return no;
    CSL_FormClose();                                                            // golden cCounterSel.cpp:56
    FileRW_CounterSel_NeedRefTick();                                            // AI(W906-CSEL-NEEDREF) 20261001: golden main.cpp:3208-3213
    return "ran golden TfCounterSel::FormClose (cCounterSel.cpp:56-109) with the server-side values read at open: IniConfig.bShow* -> config.ini [Visible] rewritten (ProcessLastSetIni_Visible), Tab_UPH, NeedRef=true";
}
