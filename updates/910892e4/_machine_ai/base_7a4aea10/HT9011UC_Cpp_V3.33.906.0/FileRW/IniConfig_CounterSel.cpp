// ===========================================================================
//  FileRW/IniConfig_CounterSel.cpp -- golden TfCounterSel（cCounterSel.cpp，V912）的 C 路入口：Status.CounterSel.html 的讀寫。
//  頁面補件：web/page/ht9045_countersel_c.js。
//
//  AI(W906-CRT-CounterSel) 20260926: 新檔（Steven 團隊）。設定：tools/editlist/IniConfig_CounterSel.py（每條 replace／block 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 IniConfig_CounterSel.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    建構子（:23）   NeedRef=false（門面 fCounterSel->NeedRef）。
//    FormShow（:29）＝開頁：ProcessLastSetIni_Visible(bReadFile)（cprod.cpp:2849，golden cprod.cpp:2721）讀 config.ini [Visible]
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
//    golden cprod.cpp:3111）也寫同一個 [Visible]。兩條 C 路都經同一份記憶體 IniConfig，各自先 FormShow 讀檔再寫，不互蓋。
//    kOwned 的 config.ini 說明字串要加上本檔（插入片段見交件報告，共用檔由主 session 改）。
//
//  ---- bShow* 的讀者（主畫面哪些顯示會變；golden V912 → 移植樹，20260926 grep）----
//    bShowLoaderCT／bShowContactCT／bShowBinCT  golden main.cpp:9158-9198 DoShowUserDefFrom：fSortCT／fContactCT／fShowBinSelect 開或關
//                                               → 移植樹沒有 DoShowUserDefFrom（IniConfig.gen.inc 同樣記 todo）；網頁視窗由 background.html 開關
//    bShowTestCate／iShowCateByArm              golden main.cpp:9168 同上＋cTestCategory.cpp:513 SetShowCateMode
//                                               → 移植樹只在開機跑一次（cTestCategory.cpp:643 W906_BootTestCategory → tag tcat.show／tcat.cateByArm）
//    bShowUPH                                   golden cShowBinSelect.cpp:857 Tab_UPH->TabVisible → 移植樹 cShowBinSelect.cpp:2171；本頁存檔 :77 也設
//    bShowIndexTime                             golden csystem.cpp:20760 ShowIndexTime → 移植樹 csystem.cpp:23983（主畫面狀態列那段在移植樹 GATE H1-05）
//    bShowContactHeight                         golden cinitial.cpp:13871／:13912 扭力欄字串 → 移植樹 cinitial.cpp:19090／:19137（fMain->lbArm0Torque）
//    bShowTimeInfo                              golden cObserver.cpp:2109、FormClose :74-75 → 移植樹沒有讀者
//    bShowTemper                                golden cTemperFrom.cpp:1835／:1843 → 移植樹沒有讀者
//    bShowScanCate                              golden 只有本表單與 ProcessLastSetIni_Visible 讀寫（DFM gbScanner Enabled=False，操作員改不到）
//    另：golden main.cpp:27987-28004 OnHotKey Ctrl+A 會一起切 bShowUPH／bShowIndexTime／bShowTimeInfo（不寫檔）。
//  ⚠ golden 關窗後 TfMain::Timer1Timer（main.cpp:3299-3304）看到 NeedRef 會重跑 SetShowCateMode＋DoShowUserDefFrom；
//    移植樹沒有這個讀者 → 本頁存檔後 tcat.cateByArm 等要到下次開機才變（列給 Steven 決定，見交件報告）。
//
//  ---- 開機（golden HT9045.cpp:203 CreateForm(TfCounterSel)）----
//    FileRW_CounterSel_Boot()：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身 → 容器替身與父子。不讀檔（讀 [Visible] 是
//    golden ReadLastSetIni 的事，移植樹 wb_serve 開機已跑）。沒有 HTEditList，CreateForm 先後不影響註冊。冪等。
//
//  ---- 沒接的（golden 呼叫端，不是本表單）----
//    main.cpp:28556 sbSeleteClick：fSecurity->Insufficient(25) 入口權限、NewRecordProcess("MES2181")、ShowModal 後 ShowFormPos()。
//    網頁主選單（web/page/main.html DFM_MAP sbSelete）直接開視窗，所有 C 路頁面都沒有接主畫面入口鈕的權限閘（列給 Steven 決定）。
// ===========================================================================
#include "FileRW/IniConfig_CounterSel.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

const filerw::PageDesc kPage = {
    "IniConfig_CounterSel", "TfCounterSel", "Status.CounterSel.html",
    nullptr, nullptr, 0,
    kCSL_SaveReads, (int)(sizeof(kCSL_SaveReads) / sizeof(kCSL_SaveReads[0])),
    &CSL_FormShow, &CSL_FormClose, "FormClose", &CSL_FormShow, &Booted,
    nullptr, nullptr,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfCounterSel 建構（HT9045.cpp:203 CreateForm）：DFM 設計期狀態 → 建構子（:23）→ 存檔流程讀的替身 → 容器替身與父子。不讀檔。冪等。
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
