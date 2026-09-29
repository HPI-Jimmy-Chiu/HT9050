// ===========================================================================
//  JsonBridge/actions/MainClarnData.cpp
//
//  AI(W906-SJSON-S11) 20260923.
//  golden：V912 main.cpp:15458-15648（191 行）。理由與偏離清單在 .h 檔頭。
//
//  行對行對照：下面每個分支上方的 `// golden :NNNNN` 就是 V912 的行號，
//  可以直接 `sed -n 'NNNNNp' main.cpp | iconv -f CP950 -t UTF-8` 覆核。
// ===========================================================================
#include "JsonBridge/actions/MainClarnData.h"

#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"      // eClearType / CC_* / rsmContinuRetest / tNotUse
#include "cmydef.h"           // SystemYear.. / iTrayTotal / TrayID / s6TrayName / PC_NAME
#include "cprod.h"            // Prod / RunInfo / TestIF_File / WriteLastDataFile
#include "LastSet.h"          // LastSet（不在 cprod.h）
#include "Config.h"           // IniConfig
#include "common.h"           // asQtyDataPath / MyForceDirectories
#include "cMyDB.h"            // MyDBIProcess
#include "Public/MyStringList.h"
#include "forms/fMain.h"
#include "forms/fCounterClear.h"
#include "forms/fSortCT.h"
#include "forms/fContactCT.h"
#include "forms/fYieldMonitoring.h"
#include "forms/fSCKART.h"

namespace ht9045 {
namespace sjson {

namespace {

// -------------------------------------------------------------------------
//  golden 在 TfMain 的建構子裡建 slQtyLog（main.cpp:1622-1639）。
//  這裡延遲建構 —— 理由 D2，寫在 .h 檔頭。
//  表頭那 15 行是 golden :1622-1634 的逐字翻譯。
// -------------------------------------------------------------------------
TMyStringList* g_slQtyLog = 0;

TMyStringList* QtyLog()
{
    if (fMain != 0 && fMain->slQtyLog != 0) return fMain->slQtyLog;   if (g_slQtyLog != 0) return g_slQtyLog;   //AI(W906-LOGOBJ-W7) 20260927 (St02-E): W7=A -- the golden member (LogObjects.cpp, wb_serve boot) first; the lazy copy below only where it is not built (ctests)

    AnsiString Buffer1 = "";
    AnsiString Str1;
    for (int i = 0; i < eTrayCount; i++)                                        // golden :1623
    {
        if (Prod.iTrayType[i] != tNotUse &&                                     // golden :1625
            Prod.iTrayType[i] != tTrayBox)                                      // golden :1626
        {
            if (Buffer1 == "")
                Str1.sprintf("%s", s6TrayName[i]);                              // golden :1629
            else
                Str1.sprintf(", %s", s6TrayName[i]);                            // golden :1631
            Buffer1 = Buffer1 + Str1;
        }
    }
    AnsiString str = AnsiString("Date, Time, Action, Loading, Total,") + Buffer1; // golden :1634
    g_slQtyLog = new TMyStringList(asQtyDataPath,                               // golden :1635
                                   "QtyLog",
                                   str);
    g_slQtyLog->SaveType = TByMonth;                                            // golden :1639
    return g_slQtyLog;
}

// 對照表：ct* 常數 -> 名字。dryRun 的預覽與測試用，golden 沒有這張表。
const char* ClearTypeName(int ct)
{
    switch (ct) {
        case ctLoadingCounts:    return "ctLoadingCounts";
        case ctTesterCategory:   return "ctTesterCategory";
        case ctContactCounts:    return "ctContactCounts";
        case ctTraySortCount:    return "ctTraySortCount";
        case ctScannerCategory:  return "ctScannerCategory";
        case ctAlarmData:        return "ctAlarmData";
        case ctTimeData:         return "ctTimeData";
        case ctContactCountsHis: return "ctContactCountsHis";
        case ctBinCount:         return "ctBinCount";
        case ctIndexCount:       return "ctIndexCount";
        case ctAutoRetestCount:  return "ctAutoRetestCount";
        case ctFailBinCount:     return "ctFailBinCount";
        default:                 return "ct?";
    }
}

bool g_installed = false;

// -------------------------------------------------------------------------
//  ⚠ 偏離 D4（動手時才發現的，補進這裡而不是只寫在 .h）：
//    btClearCountClick 的 Sender。
//
//  golden（V912 main.cpp:15593／:15608）傳的是 `this`，也就是 fMain
//  —— 在 BCB6 裡是 TObject 的後裔，所以型別對得上。
//  收端 **移植樹 cContactCT.cpp:1254** 做
//      Ptr = static_cast<TfContactCTButton*>(Sender);
//      if (Ptr->Name == "btClearCount")            // :1257
//  ⇒ golden 的**可觀察結果**是：`this` 的 Name 是 "fMain" 不是
//    "btClearCount"，所以 :1257 那個 bClearData 分支**不會**進去。
//    （golden 那一側 cast 的目標型別還寫錯成 TSpeedButton*，BCB6 因為
//      ->Name 在 TComponent 根部共用才沒出事；forms/fContactCT.h:196
//      的 DESIGN NOTE 記著這件事。）
//
//  移植樹的 TfMain **不是** vclcompat::TObject 的後裔
//  （`class TfMain` — forms/fMain.h:163，沒有基底類別；20260923 實測），
//  所以 `btClearCountClick(fMain)` 連編都編不過；就算硬轉，那個
//  static_cast 會是未定義行為，`Ptr->Name` 讀到的是別人的位元組 ——
//  而它**剛好**等於 "btClearCount" 的機率不是零，一旦等於，機台會多清一次
//  Yield 資料，而且看不出來。
//
//  ⇒ 傳一個 Name 為空的 TfContactCTButton。它重現的是 golden 的
//    **可觀察結果**（Name != "btClearCount"），不是 golden 的物件身分。
//    這是本檔唯一一處「不能逐字」的地方，理由是型別系統，不是偏好。
//
//  ⓘ 實際影響範圍比看起來小（20260923 量）：兩個呼叫點裡，Tag==8 那個被
//    `CUSTOMER_CODE==CC_KYEC_LEE` 包著，而移植樹的 btClearCountClick 在
//    CC_KYEC_LEE 分支是**直接 return**（cContactCT.cpp:1213-1240，GATE C4
//    fail-closed），Sender 根本沒被讀到。只有 Tag==11 那個會真的走到
//    :1254。⇒ 本偏離今天唯一可觀察的位置是 act.main.clarnData{tag:11}。
TfContactCTButton g_clarnSender;   // Name 預設空字串

}  // namespace

// ===========================================================================
//  golden main.cpp:15458-15648 -- TfMain::Clarn_Data(int Tag, AnsiString Msg)
// ===========================================================================
void ClarnDataBody(int Tag, AnsiString Msg)
{
    int iMode;                                                                  // golden :15460

    // ⚠ golden 的這兩個是**函式內 static**（:15462-15463），而且是整支的核心：
    //   FileName 在函式**尾端**才被設定，所以**第一次呼叫時 FileName 還是 ""**
    //   ⇒ 開頭那段 QtyLog 寫檔整段被跳過。第二次呼叫起才會寫。
    //   這不是缺陷，是 golden 刻意的「先記上一輪的結果、再算這一輪的檔名」。
    //   翻成檔案範圍的 static 會改變語意（多個 TfMain 實例會共用），
    //   但這棵樹只有一個 fMain，而且 C++ 的 function-local static 語意與
    //   BCB6 相同，所以照原樣放在函式內。
    static AnsiString LotID = "";                                               // golden :15462
    static AnsiString FileName = "";                                            // golden :15463
    AnsiString asPath, Buffer1, str;                                            // golden :15464
    asPath.sprintf("%s%04d%02d\\", asQtyDataPath, SystemYear, SystemMonth);     // golden :15465
    MyForceDirectories(asPath);                                                 // golden :15466

    if (IniConfig.bA61DisableCleanMUBA == true)                                 // golden :15468
        return;                                                                 // golden :15469

    if (FileName != "")                                                         // golden :15471
    {
        for (int i = 0; i < eTrayCount; i++)                                    // golden :15473
        {
            if (Buffer1 == "")
                str.sprintf("%d",   LastSet.BinCT[0][iTo3Unload[i]]);           // golden :15476
            else
                str.sprintf(", %d", LastSet.BinCT[0][iTo3Unload[i]]);           // golden :15478
            Buffer1 = Buffer1 + str;
        }

        str.sprintf("%s, %d, %d, %s",                                           // golden :15482
                    Msg,
                    LastSet.SendCT[0],
                    RunInfo.iUnloadCount,
                    Buffer1);
        QtyLog()->AddTextWithDateTime(str);                                     // golden :15487
        QtyLog()->MySaveFileByFileName(asPath, FileName);                       // golden :15488
    }

    if (Tag == 0)                                                               // golden :15491
    {
        iMode = (LastSet.iRunStartMode <= rsmContinuRetest) ? 0 : 1;            // golden :15493

        if (LastSet.bCTClear[iMode][ctLoadingCounts])                           // golden :15495  Loading Counts
            fCounterClear->ClearCount(ctLoadingCounts);

        if (LastSet.bCTClear[iMode][ctTesterCategory])                          // golden :15498  Tester Category
            fCounterClear->ClearCount(ctTesterCategory);

        if (LastSet.bCTClear[iMode][ctContactCounts])                           // golden :15501  Contact Counts
            fCounterClear->ClearCount(ctContactCounts);

        if (LastSet.bCTClear[iMode][ctTraySortCount])                           // golden :15504  Tray Sort Count
            fCounterClear->ClearCount(ctTraySortCount);

        if (LastSet.bCTClear[iMode][ctScannerCategory])                         // golden :15507  Scanner Category
            fCounterClear->ClearCount(ctScannerCategory);

        if (LastSet.bCTClear[iMode][ctAlarmData])                               // golden :15510  Alarm Data
            fCounterClear->ClearCount(ctAlarmData);

        if (LastSet.bCTClear[iMode][ctTimeData])                                // golden :15513  Time Data
            fCounterClear->ClearCount(ctTimeData);

        // golden :15516  fContactCT->sgYield->Refresh();
        //   偏離 D1：VCL 重繪，新架構沒有控制項。見 .h 檔頭。
        for (int i = 0; i < 32; i++)                                            // golden :15517
            fYieldMonitoring->bShowSiteYield[i] = false;                         // golden :15518

        fYieldMonitoring->ClearYieldCount();                                    // golden :15520
    }
    else if (Tag == 1)                                                          // golden :15522
    {
        fCounterClear->ClearCount(ctLoadingCounts);
        fCounterClear->ClearCount(ctTraySortCount);
        fCounterClear->ClearCount(ctTesterCategory);
        fCounterClear->ClearCount(ctIndexCount);
    }
    else if (Tag == 2)                                                          // golden :15529
    {
        fCounterClear->ClearCount(ctLoadingCounts);
        //      fCounterClear->ClearCount(ctTraySortCount);                     // golden :15532 已被 golden 自己註解掉
        fCounterClear->ClearCount(ctTesterCategory);
        fCounterClear->ClearCount(ctIndexCount);
    }
    else if (Tag == 3)                                                          // golden :15536
    {
        fCounterClear->ClearCount(ctLoadingCounts);
        fCounterClear->ClearCount(ctIndexCount);                                // kevin 20130125
        fCounterClear->ClearCount(ctContactCounts);
        fCounterClear->ClearCount(ctContactCountsHis);
        fCounterClear->ClearCount(ctTraySortCount);
    }
    else if (Tag == 4)                                                          // golden :15544
    {
        fCounterClear->ClearCount(ctLoadingCounts);
        fCounterClear->ClearCount(ctTraySortCount);
        fCounterClear->ClearCount(ctTesterCategory);                            // jou 2011-07-26
        fCounterClear->ClearCount(ctIndexCount);                                // kevin 20130125

        fCounterClear->ClearCount(ctContactCounts);                             // Rogeryang 20170704
        fCounterClear->ClearCount(ctContactCountsHis);                          // Rogeryang 20170704
    }
    else if (Tag == 5)                                                          // golden :15554
    {
        fCounterClear->ClearCount(ctAutoRetestCount);                           // wei 20150923
        fCounterClear->ClearCount(ctLoadingCounts);
        fCounterClear->ClearCount(ctIndexCount);
        fCounterClear->ClearCount(ctTesterCategory);
    }
    else if (Tag == 6)                                                          // golden :15561
    {
        fCounterClear->ClearCount(ctLoadingCounts);
        fCounterClear->ClearCount(ctIndexCount);
        fCounterClear->ClearCount(ctTesterCategory);
    }
    else if (Tag == 7)                                                          // golden :15567
    {
        //      fCounterClear->ClearCount(ctLoadingCounts);                     // golden :15569 已被 golden 自己註解掉
        //      fCounterClear->ClearCount(ctTraySortCount);                     // golden :15570 同上
        fCounterClear->ClearCount(ctTesterCategory);
        fCounterClear->ClearCount(ctIndexCount);
    }
    else if (Tag == 8)                                                          // golden :15574
    {
        if (!(CUSTOMER_CODE == CC_SCS ||                                        // golden :15576  jou 2012-05-04
              CUSTOMER_CODE == CC_ASE_Korea ||                                  // golden :15577  jou 2013-07-08
              CUSTOMER_CODE == CC_AMKOR_Korea))                                 // golden :15578  Steven 20110725
        {
            fCounterClear->ClearCount(ctLoadingCounts);
        }
        fCounterClear->ClearCount(ctTraySortCount);                             // golden :15582

        if (IniConfig.bVTESTFunction == false)                                  // golden :15584
        {
            fCounterClear->ClearCount(ctTesterCategory);                        // golden :15586  jou 2011-07-26
        }

        fCounterClear->ClearCount(ctIndexCount);                                // golden :15589  kevin 20130125
        if (CUSTOMER_CODE == CC_KYEC_LEE)                                       // golden :15590  wei 20151116
        {
            fMain->bHasCleanCount = true;                                       // golden :15592  Ifor 20191016
            fContactCT->btClearCountClick(&g_clarnSender);                               // golden :15593
            //   ⚠ 偏離 D4（理由與複驗證據在上面 g_clarnSender 的宣告處）。
            //     golden 傳 `this`（TfMain*）；這裡傳一個 Name 為空的
            //     TfContactCTButton，因為**可觀察結果相同**：兩者的 Name
            //     都不等於 "btClearCount"，所以 cContactCT.cpp:1257 的
            //     bClearData 分支兩邊都不會進去。
            fMain->bHasCleanCount = false;                                      // golden :15594  Ifor 20191016
        }
    }
    else if (Tag == 9)                                                          // golden :15597
    {
        fCounterClear->AutoClear();                                             // golden :15599
    }
    else if (Tag == 10)                                                         // golden :15601
    {
        // golden 這裡就是空的（:15602-15603）。不是漏翻。
        // 它存在的意義是「跑共用前置與尾段，但不清任何 ct*」——
        // cCounterClear.cpp:449/:475 的手動清除前後各叫一次，用的就是它。
    }
    else if (Tag == 11)                                                         // golden :15605  Sam 20230728
    {
        fCounterClear->ClearCount(ctTraySortCount);
        fContactCT->btClearCountClick(&g_clarnSender);                                   // golden :15608  kevin 20170814
    }
    else if (Tag == 12)                                                         // golden :15610  RogerYang 20260703
    {
        fCounterClear->ClearCount(ctLoadingCounts);
        fCounterClear->ClearCount(ctFailBinCount);
    }

    for (int j = 0; j < 3; j++)                                                 // golden :15616  kevin 20210623
    {
        iTrayTotal[j] = 0;                                                      // golden :15618
        for (int i = 0; i < 6; i++)
            TrayID[i][j] = "";                                                  // golden :15620
    }

    for (int i = 0; i < ePosTrayCount; i++)                                     // golden :15623  JerryYang 20220909
    {
        iAutoTrayPlaceCount[i] = 0;                                             // golden :15625
        iOneTrayPickCount[i] = 0;                                               // golden :15626
    }

    WriteLastDataFile(false);                                                   // golden :15629  kevin 20141030
    fSortCT->ShowLoadingIC();                                                   // golden :15630
    fSortCT->ShowSortIC();                                                      // golden :15631
    MyDBIProcess("Process", Msg);                                               // golden :15632

    if (TestIF_File.bSCKART_EnableART)                                          // golden :15634
    {
        if (LotID == "" || LotID != fSCKART->palLotNumber->Caption)             // golden :15636
        {
            LotID = fSCKART->palLotNumber->Caption;                             // golden :15638
        }
        FileName.sprintf("%s_%s.csv", PC_NAME, LotID);                          // golden :15640
    }
    else
    {
        if (LotID != "")                                                        // golden :15644
            FileName.sprintf("%s_%s_%04d%02d%02d.csv", PC_NAME, LotID,
                             SystemYear, SystemMonth, SystemDate);              // golden :15645
        else
            FileName.sprintf("%s_%04d%02d%02d.csv", PC_NAME,
                             SystemYear, SystemMonth, SystemDate);              // golden :15647
    }
}

// ===========================================================================
//  以下不是 golden，是本波次的基礎建設。
// ===========================================================================

void InstallClarnDataBody()
{
    W906_ClarnDataBody = &ClarnDataBody;
    g_installed = true;
}

bool ClarnDataBodyInstalled() { return g_installed; }

std::vector<std::string> ClarnDataPreview(int Tag)
{
    std::vector<std::string> out;

    // 守衛先看。擋下來的話一個 ct* 都不會清 —— 連共用尾段都不跑。
    if (IniConfig.bA61DisableCleanMUBA == true) return out;

    switch (Tag) {
        case 0: {
            const int iMode = (LastSet.iRunStartMode <= rsmContinuRetest) ? 0 : 1;
            static const int kTag0[] = { ctLoadingCounts, ctTesterCategory,
                                         ctContactCounts, ctTraySortCount,
                                         ctScannerCategory, ctAlarmData,
                                         ctTimeData };
            for (std::size_t i = 0; i < sizeof(kTag0)/sizeof(kTag0[0]); ++i)
                if (LastSet.bCTClear[iMode][kTag0[i]])
                    out.push_back(ClearTypeName(kTag0[i]));
            // 這兩個不是 ct*，但是 Tag==0 真的會做的事，預覽不能漏。
            out.push_back("fYieldMonitoring.bShowSiteYield[0..31]=false");
            out.push_back("fYieldMonitoring.ClearYieldCount()");
            break;
        }
        case 1:
            out.push_back(ClearTypeName(ctLoadingCounts));
            out.push_back(ClearTypeName(ctTraySortCount));
            out.push_back(ClearTypeName(ctTesterCategory));
            out.push_back(ClearTypeName(ctIndexCount));
            break;
        case 2:
            out.push_back(ClearTypeName(ctLoadingCounts));
            out.push_back(ClearTypeName(ctTesterCategory));
            out.push_back(ClearTypeName(ctIndexCount));
            break;
        case 3:
            out.push_back(ClearTypeName(ctLoadingCounts));
            out.push_back(ClearTypeName(ctIndexCount));
            out.push_back(ClearTypeName(ctContactCounts));
            out.push_back(ClearTypeName(ctContactCountsHis));
            out.push_back(ClearTypeName(ctTraySortCount));
            break;
        case 4:
            out.push_back(ClearTypeName(ctLoadingCounts));
            out.push_back(ClearTypeName(ctTraySortCount));
            out.push_back(ClearTypeName(ctTesterCategory));
            out.push_back(ClearTypeName(ctIndexCount));
            out.push_back(ClearTypeName(ctContactCounts));
            out.push_back(ClearTypeName(ctContactCountsHis));
            break;
        case 5:
            out.push_back(ClearTypeName(ctAutoRetestCount));
            out.push_back(ClearTypeName(ctLoadingCounts));
            out.push_back(ClearTypeName(ctIndexCount));
            out.push_back(ClearTypeName(ctTesterCategory));
            break;
        case 6:
            out.push_back(ClearTypeName(ctLoadingCounts));
            out.push_back(ClearTypeName(ctIndexCount));
            out.push_back(ClearTypeName(ctTesterCategory));
            break;
        case 7:
            out.push_back(ClearTypeName(ctTesterCategory));
            out.push_back(ClearTypeName(ctIndexCount));
            break;
        case 8:
            if (!(CUSTOMER_CODE == CC_SCS ||
                  CUSTOMER_CODE == CC_ASE_Korea ||
                  CUSTOMER_CODE == CC_AMKOR_Korea))
                out.push_back(ClearTypeName(ctLoadingCounts));
            out.push_back(ClearTypeName(ctTraySortCount));
            if (IniConfig.bVTESTFunction == false)
                out.push_back(ClearTypeName(ctTesterCategory));
            out.push_back(ClearTypeName(ctIndexCount));
            if (CUSTOMER_CODE == CC_KYEC_LEE)
                out.push_back("fContactCT.btClearCountClick()");
            break;
        case 9:
            // AutoClear 依 authCounterClr[] 決定清哪些，是執行期權限的函式，
            // 這裡不重算 —— 重算一次就是第二份實作。誠實標示。
            out.push_back("fCounterClear.AutoClear() (依 authCounterClr[] 決定，預覽不重算)");
            break;
        case 10:
            break;   // golden 的分支是空的
        case 11:
            out.push_back(ClearTypeName(ctTraySortCount));
            out.push_back("fContactCT.btClearCountClick()");
            break;
        case 12:
            out.push_back(ClearTypeName(ctLoadingCounts));
            out.push_back(ClearTypeName(ctFailBinCount));
            break;
        default:
            break;   // 0..12 以外由呼叫端擋
    }

    // 共用尾段 —— 每個 Tag 都會做，預覽不能只講分支。
    out.push_back("iTrayTotal[0..2]=0 / TrayID[0..5][0..2]=\"\"");
    out.push_back("iAutoTrayPlaceCount[]=0 / iOneTrayPickCount[]=0");
    out.push_back("WriteLastDataFile(false)  -> D:\\HT9045\\system\\lastdata.dat");
    out.push_back("fSortCT.ShowLoadingIC() / ShowSortIC()");
    out.push_back("MyDBIProcess(\"Process\", msg)");
    return out;
}

}  // namespace sjson
}  // namespace ht9045
