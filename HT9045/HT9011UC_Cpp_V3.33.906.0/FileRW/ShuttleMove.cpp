// ===========================================================================
//  FileRW/ShuttleMove.cpp -- golden TfShuttleMove（ShuttleMove.cpp，V912）的 C 路入口：HW.ShuttleMove.html 的「讀寫段」。
//  頁面補件：web/page/ht9045_shuttlemove_c.js。
//
//  Steven 團隊 20260926.  設定：tools/editlist/ShuttleMove.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 ShuttleMove.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    FormShow（:67）＝開頁：十個 Shuttle 教導點 Tech.* → 替身；latch 機台（In_Shuttle_Auto_Latch==eInSHAutoLtc）另外
//      fShuttleMove->ReadData()（HandlerCondition.Data [Shuttle]）→ edInSH?SenICDetectZPos；ShowShuttleSensorPosition（格子數字）。
//    sbUpdateClick（:1965）＝存檔鈕：替身 → Tech.*；latch 機台寫 HandlerCondition.Data [Shuttle] InSH1SenICAddPos／InSH2SenICAddPos
//      並 ReadData 重讀；fTeach->SaveFile(true)（FileRW/Teach.cpp FileRW_Teach_SaveFile → system\teach.ini）；
//      InitShuttleThreadParameter；ShowShuttleSensorPosition（內含 SetTechDataToProd）。golden 這顆鈕沒有權限守衛、沒有確認框。
//  savedMark＝"sbUpdateClick"：golden 存檔鈕無條件寫檔。mustSend＝存檔流程讀的 12 個 TEdit（沒有 HTEditList）。
//  reload＝golden FormShow（不是 FormClose：golden FormClose :2132 會 SystemStart=false、暫停 EtherCAT，是機台動作）。
//
//  ---- Steven 20260926 裁決 S47：「只有latch才會用到，無latch的機台就不用讀寫」 ----
//    HandlerCondition.Data [Shuttle] 的讀寫全部照 golden 的 In_Shuttle_Auto_Latch 條件（開頁、存檔、開機、換配方）。
//    本機 system\Gerneral.ini In_Shuttle_Auto_Latch=0 ⇒ 這一頁只讀寫 teach.ini；gbInFiberCheckShtSnLct 照 golden 隱藏。
//
//  ---- 開機與換配方（golden 呼叫點，D:\HT9045 V912 行號）----
//    main.cpp:11741 TfMain::FormShow        if(latch) fShuttleMove->ReadData();  → tools/wb_serve.cpp（SetTechDataToProd 之前）
//    main.cpp:25814 TfMain::ChangeSetUpFile if(latch) fShuttleMove->ReadData();  → WebRecipeChange.cpp（CopyRecipeToTester 之後）
//    cinitial.cpp:11230 SetTechDataToProd_Shuttle if(latch) fShuttleMove->ReadData(); → 移植樹 cinitial.cpp GATE N3-G5（底層，列給 Jimmy）
//    ReadData 本體在門面 forms/fShuttleMove.cpp（全樹一份）。
//
//  ---- 機台動作不在本檔（網頁停用，見 ht9045_shuttlemove_c.js）----
//    ShuttleMoveClick（:1410，移動／掃描／latch 自動教導）、btnTStepClick（:2172 fMain->Start）、btStartClick（:2247）、
//    mtOutSHDetectPosMouseDown（:2178 雙擊移動）、sbShuttleSensorClick（:1769）、sbSensorLatchClick（:2144）、btRetryClick（:1405）、
//    sbtExitClick（:1956 fAllMotorHome=false＋Close → FormClose SystemStart=false）。
//    DoInShuttleChkStackAction（:2387）的 case 9100（:2745-2756）寫檔是 latch 自動教導流程的一部分（底層，列給 Jimmy）。
// ===========================================================================
#include "FileRW/ShuttleMove.gen.inc"

#include <cstdio>
#include <string>

#include "FileRW/_EditPage.h"
#include "WebBridge/JsonWriter.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

void Tray(webbridge::JsonWriter& w, const char* name, const SM_TTMyTray& t, int yItem)
{
    w.Key(name).BeginObject();
    w.Key("xItem").Number((wb_int64)t.XItem);
    w.Key("yItem").Number((wb_int64)yItem);
    w.Key("portGap").Bool(t.bPortGap);
    w.Key("cells").BeginArray();
    for (int x = 0; x < t.XItem && x < 16; ++x)
        for (int y = 0; y < yItem && y < 4; ++y)
            if (t.Has[x][y]) {
                w.BeginArray();
                w.Number((wb_int64)x);
                w.Number((wb_int64)y);
                w.Number((wb_int64)t.Cell[x][y]);
                w.EndArray();
            }
    w.EndArray();
    w.EndObject();
}

// PageDesc::extraJson：golden ShowShuttleSensorPosition 的四個 TTMyTray 格子（DFM YItem：mtOutSHDetectPos 4，其餘 2）、
// 以及通用 proxies 不帶的 TPanel／TGroupBox Caption（palSh?Encoder＝golden FormShow :137-138 讀編碼器；gbScanOutShuttle :144）。
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("trays").BeginObject();
    Tray(w, "mtedInSHSen7DetectPos", SM_mtedInSHSen7DetectPos, 2);
    Tray(w, "mtOutSHDetectPos", SM_mtOutSHDetectPos, 4);
    Tray(w, "mtedInSHSenICDetectPos", SM_mtedInSHSenICDetectPos, 2);
    Tray(w, "mtInSHBarCodePos", SM_mtInSHBarCodePos, 2);
    w.EndObject();
    w.Key("captions").BeginObject();
    for (const char* n : {"palSh1Encoder", "palSh2Encoder"})
        w.Key(n).String(EL<TPanel>("TfShuttleMove", n)->Caption.c_str());
    w.Key("gbScanOutShuttle").String(EL<TGroupBox>("TfShuttleMove", "gbScanOutShuttle")->Caption.c_str());
    w.EndObject();
    w.Key("inShuttleAutoLatch").Bool(In_Shuttle_Auto_Latch == eInSHAutoLtc);
    w.EndObject();
    return w.Str();
}

const filerw::PageDesc kPage = {
    "ShuttleMove", "TfShuttleMove", "HW.ShuttleMove.html",
    nullptr, nullptr, 0,
    kSM_SaveReads, (int)(sizeof(kSM_SaveReads) / sizeof(kSM_SaveReads[0])),
    &SM_FormShow, &SM_sbUpdateClick, "sbUpdateClick", &SM_FormShow, &Booted,
    nullptr, &ExtraJson,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfShuttleMove 建構（HT9045.cpp CreateForm）：DFM 設計期狀態 → 建構子（:59）→ 存檔流程讀的替身 → 容器替身與父子。不讀檔。冪等。
void FileRW_ShuttleMove_Boot()
{
    if (g_booted) return;
    SM_DfmItems();
    SM_DfmState();
    EL<TGroupBox>("TfShuttleMove", "gbScanOutShuttle")->Caption = "Detect Out Shuttle device";   // golden ShuttleMove.dfm gbScanOutShuttle.Caption
    SM_TfShuttleMove();
    SM_CreateSaveProxies();
    SM_CreateContainerProxies();
    std::printf("FileRW ShuttleMove: TfShuttleMove proxies ready (%d save reads) -- golden ShuttleMove.cpp\n",
                (int)(sizeof(kSM_SaveReads) / sizeof(kSM_SaveReads[0])));
    g_booted = true;
}
