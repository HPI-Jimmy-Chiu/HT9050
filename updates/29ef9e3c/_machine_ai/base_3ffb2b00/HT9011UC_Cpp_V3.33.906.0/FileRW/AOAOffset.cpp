// ===========================================================================
//  FileRW/AOAOffset.cpp -- golden TfMain（main.cpp，V912）AOA offset 那一小塊的 C 路入口：Main.AOAInfo.html 的讀寫。
//  D:\HT9045\system\Gerneral.ini [System] AOA_InArm_*／AOA_OutArm_* _X/_Y（量產共用檔）。頁面補件：web/page/ht9045_aoaoffset_c.js。
//
//  //AI(W906-FRW-AOA) 20260926: 新檔（Steven 團隊）。補盤點 cmydef_io_audit.md 第四節 P6 的寫缺口：golden 唯一寫入點
//    TfMain::OffsetSaveClick 移植樹原本沒有、網頁也沒有這組元件。設定：tools/editlist/AOAOffset.py（每條 replace 附原因）。
//
//  golden 的四個位置（V912，20260926 重查）→ 移植後落點：
//    讀檔   SYSTEM_MODULAR::ReadGeneralIni database.cpp:1451-1498 → 移植樹 database.cpp:1576-1623（逐字、live；LoadMachineConfig 開機跑，
//           HSys 存檔後 HSys.ReadGeneralIni() 也重讀）。本檔不碰。
//    填元件 TfMain::FormShow main.cpp:11679-11724（MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF_File.bEnableAutoAlignment 才填）
//           ＋ :11736 pnlAOAForHT9011->Visible=(AUTO_EMPTY_COLOR>=3) → AOAOffset.gen.inc 的 AOA_FormShowAOA()，本檔開機呼叫一次。
//           主視窗 FormShow 只在開機跑；golden 切到 AOA Info 頁籤不跑任何程式（pgMotionView main.dfm:11927 沒有 OnChange），
//           換配方（ChangeSetUpFile → DoReadLastData）也不重填 ⇒ editlist.get 不重填（OpenPage 只做型號檢查）。
//    存檔   TfMain::OffsetSaveClick main.cpp:34956-35049 → AOA_OffsetSaveClick()（editlist.save）。38 個 Ed_* → iAOA_*（atoi）→
//           WriteIniDataGeneral ×26（AUTO_EMPTY_COLOR>=3 ×38）。golden 沒有權限檢查、沒有確認框、沒有 return、寫完不重讀。
//    用值   AutoAlignment\AutoAlignment.cpp:4664-5325 讀 fMain->Ed_*Offset_*->Text（元件字串，不是 iAOA_*）。移植樹沒有這段
//           （CCD 對位流程，Jimmy）；移植時讀本檔的具名替身 filerw::EL<TEdit>("TfMain","Ed_*Offset_*")（同一個物件）。
//  savedMark＝"OffsetSaveClick"：golden 存檔鈕進去就一路寫到底（沒有提早 return）；型號錯（下面）時不呼叫 → 沒有 mark ＝ 沒寫。
//  mustSend＝存檔流程讀的 38 個 TEdit（沒有 HTEditList）。reload＝OpenPage（golden 沒有「沒寫檔」的路徑，只在 PageSave 的防呆用）。
//
//  ---- golden 看起來不對的地方（照翻，不修；要改行為要 Steven 決定）----
//    1. main.dfm 38 個 Ed_*Offset_X/Y 全部 Visible = False，golden 全樹沒有地方把它們打開（20260926 grep `Offset_[XY]->` 只有 ->Text）
//       ⇒ 操作員在 golden 畫面上看不到、改不到；OffsetSave 鈕看得到。替身照 DFM ⇒ editable=false ⇒ 頁面送的值進 ack.ignored，
//       存檔寫的是開機時填進元件的值。Ed_LoaderOffset_XClick（:35051，小鍵盤）留著但元件看不見，等於沒有入口。
//    2. 開機時條件不成立（MACHINE_HAS_AUTO_ALIGNMENT_CCD=0，或配方 TestIF_File.bEnableAutoAlignment=false）⇒ 元件留在 DFM 的 '0'
//       ⇒ 按 OffsetSave 會把 [System] AOA_* 全部寫成 0、iAOA_* 也變 0。開機後才打開 AOA（換配方／HSys 改 MACHINE_HAS_AUTO_ALIGNMENT_CCD）
//       也一樣（golden 不重填）。本機 Gerneral.ini：MACHINE_HAS_AUTO_ALIGNMENT_CCD=0、26 鍵全 0 → 寫 0 不改值。
//       頁面補件在這種情況下把確認框的字改成警告（ExtraJson 的 gate.bootFilled／wouldChange），伺服器端照 golden 不擋。
//    3.（不在本檔）golden AutoAlignment.cpp:4971 `iYPos+atoi(Ed_Auto3Offset_X)`、Auto6 同型（Y 用了 X）——移植 CCD 對位時照翻並註記。
//
//  ---- Gerneral.ini 的擁有者是 HSys（FileRW/HSys.cpp，tools/wb_serve.cpp CRouteOwner "gerneral.ini"）----
//    重疊鍵：沒有。golden THandlerSystem（HandlerSys.cpp）寫的 Gerneral.ini 鍵裡沒有 AOA_*（20260926 grep "AOA" 0 筆；移植樹
//      HSys.gen.inc／HSys.cpp 也是 0 筆）。HSys 會寫本檔的兩個條件鍵：[System] AUTO_EMPTY_COLOR（HandlerSys.cpp:579）與
//      MACHINE_HAS_AUTO_ALIGNMENT_CCD（:840），存完 HSys.ReadGeneralIni() 重讀記憶體（iAOA_* 也重讀），但不重填本檔的元件（golden 同）。
//    golden 怎麼不讓舊值蓋回：兩個寫者都是「逐鍵」寫（TIniFile::WriteInteger → WritePrivateProfileString），各寫各的鍵，
//      沒有整檔回寫；iAOA_* 的寫者全樹只有 OffsetSaveClick（ReadGeneralIni 只補缺鍵）。移植樹 vclcompat TIniFile 同樣逐鍵就地改寫、
//      讀也每次重讀磁碟（tools/wb_serve.cpp 啟動訊息 AI(W906-T4-INIFMT2)／AI(W906-A5-INIREAD)）⇒ HSys 與本檔互不蓋。
//      B 路（system.file.put）寫 Gerneral.ini 已被 CRouteOwner 擋（409），不是第三個寫者。
//    採用的規則：本檔只寫 golden OffsetSaveClick 寫的那 26／38 鍵，照 golden 逐鍵 WriteIniDataGeneral（INIFileGeneral＝LoadMachineConfig
//      開的那一個；golden 是 SYSTEM_MODULAR 建構子 database.cpp:50 開、程式結束才關）；不讀回、不寫別的鍵。
//      殘餘風險（golden 同）：開機後有人在檔案外改 AOA_*（記事本）、再經 HSys 存檔重讀進記憶體 —— 元件仍是開機值，按 OffsetSave
//      會把開機值寫回去。ExtraJson 的 wouldChange 列出「存檔會改到檔案的鍵」，頁面補件在確認框前講出來。
//
//  ---- 開機（golden 呼叫點）----
//    golden TfMain::FormShow（main.cpp:9566）：:9589 bHandlerModel==false 就 Terminate＋return（走不到 :11679）→ 本檔同樣跳過填值；
//    :11679 填值要在 LoadMachineConfig（iAOA_*、MACHINE_HAS_AUTO_ALIGNMENT_CCD、AUTO_EMPTY_COLOR）與開機讀配方
//    W906_DoReadLastData(true)（fAutoAlignment->ReadFile → TestIF_File.bEnableAutoAlignment，golden DoReadLastData :9432）之後。
//    → FileRW_AOAOffset_Boot() 由 tools/wb_serve.cpp 在 W906_ShuttleMoveReadData（golden :11740-11741）那一行呼叫（整合者插入）。
//
//  ---- 不在本檔 ----
//    labXxx_X/Y（Panel14／pnlAOAForHT9011 的執行期 AOA 結果）、Memo3_AOA_IN／OUT：AutoAlignment CCD 對位流程寫（:3425、:4880、:5325），
//    移植樹沒有（Jimmy）。頁面既有的顯示保持原樣。
// ===========================================================================
#include "FileRW/AOAOffset.gen.inc"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"
#include "WebBridge/JsonWriter.h"

namespace {
bool g_booted = false;
bool g_filled = false;      // 開機時 golden FormShow :11679 的條件成立、元件填了 iAOA_*（否則元件是 DFM 的 "0"）
bool Booted() { return g_booted; }

// golden TfMain::FormShow main.cpp:9589-9594 的網頁版（同 FileRW/HSys.cpp HS_ModelReadError）。回 true ＝ golden 在這裡停機。
bool AOA_ModelReadError()
{
    if(bHandlerModel==false)                                                    //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
    {
        // golden: MessageDlg("D:\\GPIB9045\\system\\general.ini \"Model\" read error!!", mtConfirmation, TMsgDlgButtons()<<mbOK, 0);
        //         Application->Terminate(); return;
        filerw::ELMessage("D:\\GPIB9045\\system\\general.ini \"Model\" read error!!",
                          "D:\\GPIB9045\\system\\general.ini 的 \"Model\" 讀取失敗（golden 會結束程式，走不到 AOA Info；網頁版不存檔）");
        filerw::ELMark("model_read_error");
        return true;
    }
    return false;
}

// editlist.get：golden 切到 AOA Info 頁籤不跑程式（元件值是開機 FormShow 填的）。只做型號檢查（訊息進 session）。
void OpenPage() { AOA_ModelReadError(); }

// editlist.save：golden OffsetSaveClick（main.cpp:34956）。型號錯時 golden 已停機，走不到。
void SaveFlow()
{
    if (AOA_ModelReadError()) return;
    AOA_OffsetSaveClick();
}

// 沒寫檔時的還原：golden 沒有「沒寫檔」的路徑（型號錯除外，那時替身根本沒被改：頁面值全在 ignored）。
void Reload() {}

// OffsetSaveClick 寫的鍵 ↔ 讀的元件 ↔ 全域（golden main.cpp:34958-34999／:35001-35048 的配對；bHT9011＝在 AUTO_EMPTY_COLOR>=3 那兩段裡）。
// 只給 ExtraJson 用（診斷／頁面警告）；存檔本體是 gen.inc 的 golden 原文。
struct AOAKey { const char* key; const char* edit; const int* var; bool bHT9011; };
const AOAKey kKeys[] = {
    {"AOA_InArm_Loader_X",    "Ed_LoaderOffset_X", &iAOA_InArm_Loader_X,    false},
    {"AOA_InArm_Loader_Y",    "Ed_LoaderOffset_Y", &iAOA_InArm_Loader_Y,    false},
    {"AOA_InArm_Shuttle1_X",  "Ed_InSH1Offset_X",  &iAOA_InArm_Shuttle1_X,  false},
    {"AOA_InArm_Shuttle1_Y",  "Ed_InSH1Offset_Y",  &iAOA_InArm_Shuttle1_Y,  false},
    {"AOA_InArm_Shuttle2_X",  "Ed_InSH2Offset_X",  &iAOA_InArm_Shuttle2_X,  false},
    {"AOA_InArm_Shuttle2_Y",  "Ed_InSH2Offset_Y",  &iAOA_InArm_Shuttle2_Y,  false},
    {"AOA_InArm_Hotplate1_X", "Ed_HP1Offset_X",    &iAOA_InArm_Hotplate1_X, false},
    {"AOA_InArm_Hotplate1_Y", "Ed_HP1Offset_Y",    &iAOA_InArm_Hotplate1_Y, false},
    {"AOA_InArm_Hotplate2_X", "Ed_HP2Offset_X",    &iAOA_InArm_Hotplate2_X, false},
    {"AOA_InArm_Hotplate2_Y", "Ed_HP2Offset_Y",    &iAOA_InArm_Hotplate2_Y, false},
    {"AOA_OutArm_Auto1_X",    "Ed_Auto1Offset_X",  &iAOA_OutArm_Auto1_X,    false},
    {"AOA_OutArm_Auto1_Y",    "Ed_Auto1Offset_Y",  &iAOA_OutArm_Auto1_Y,    false},
    {"AOA_OutArm_Auto2_X",    "Ed_Auto2Offset_X",  &iAOA_OutArm_Auto2_X,    false},
    {"AOA_OutArm_Auto2_Y",    "Ed_Auto2Offset_Y",  &iAOA_OutArm_Auto2_Y,    false},
    {"AOA_OutArm_Auto3_X",    "Ed_Auto3Offset_X",  &iAOA_OutArm_Auto3_X,    false},
    {"AOA_OutArm_Auto3_Y",    "Ed_Auto3Offset_Y",  &iAOA_OutArm_Auto3_Y,    false},
    {"AOA_OutArm_Auto4_X",    "Ed_Auto4Offset_X",  &iAOA_OutArm_Auto4_X,    true},
    {"AOA_OutArm_Auto4_Y",    "Ed_Auto4Offset_Y",  &iAOA_OutArm_Auto4_Y,    true},
    {"AOA_OutArm_Auto5_X",    "Ed_Auto5Offset_X",  &iAOA_OutArm_Auto5_X,    true},
    {"AOA_OutArm_Auto5_Y",    "Ed_Auto5Offset_Y",  &iAOA_OutArm_Auto5_Y,    true},
    {"AOA_OutArm_Auto6_X",    "Ed_Auto6Offset_X",  &iAOA_OutArm_Auto6_X,    true},
    {"AOA_OutArm_Auto6_Y",    "Ed_Auto6Offset_Y",  &iAOA_OutArm_Auto6_Y,    true},
    {"AOA_OutArm_Fix1_X",     "Ed_Fix1Offset_X",   &iAOA_OutArm_Fix1_X,     false},
    {"AOA_OutArm_Fix1_Y",     "Ed_Fix1Offset_Y",   &iAOA_OutArm_Fix1_Y,     false},
    {"AOA_OutArm_Fix2_X",     "Ed_Fix2Offset_X",   &iAOA_OutArm_Fix2_X,     false},
    {"AOA_OutArm_Fix2_Y",     "Ed_Fix2Offset_Y",   &iAOA_OutArm_Fix2_Y,     false},
    {"AOA_OutArm_Fix3_X",     "Ed_Fix3Offset_X",   &iAOA_OutArm_Fix3_X,     false},
    {"AOA_OutArm_Fix3_Y",     "Ed_Fix3Offset_Y",   &iAOA_OutArm_Fix3_Y,     false},
    {"AOA_OutArm_Fix4_X",     "Ed_Fix4Offset_X",   &iAOA_OutArm_Fix4_X,     true},
    {"AOA_OutArm_Fix4_Y",     "Ed_Fix4Offset_Y",   &iAOA_OutArm_Fix4_Y,     true},
    {"AOA_OutArm_Fix5_X",     "Ed_Fix5Offset_X",   &iAOA_OutArm_Fix5_X,     true},
    {"AOA_OutArm_Fix5_Y",     "Ed_Fix5Offset_Y",   &iAOA_OutArm_Fix5_Y,     true},
    {"AOA_OutArm_Fix6_X",     "Ed_Fix6Offset_X",   &iAOA_OutArm_Fix6_X,     true},
    {"AOA_OutArm_Fix6_Y",     "Ed_Fix6Offset_Y",   &iAOA_OutArm_Fix6_Y,     true},
    {"AOA_OutArm_Shuttle1_X", "Ed_OutSH1Offset_X", &iAOA_OutArm_Shuttle1_X, false},
    {"AOA_OutArm_Shuttle1_Y", "Ed_OutSH1Offset_Y", &iAOA_OutArm_Shuttle1_Y, false},
    {"AOA_OutArm_Shuttle2_X", "Ed_OutSH2Offset_X", &iAOA_OutArm_Shuttle2_X, false},
    {"AOA_OutArm_Shuttle2_Y", "Ed_OutSH2Offset_Y", &iAOA_OutArm_Shuttle2_Y, false},
};
const int kNKeys = (int)(sizeof(kKeys) / sizeof(kKeys[0]));

// PageDesc::extraJson：通用 proxies 帶不到的東西（純讀：記憶體＋Gerneral.ini 磁碟值，不寫檔）。
//   gate：golden FormShow :11679 的條件（現在值）、開機時有沒有填（bootFilled）、AUTO_EMPTY_COLOR、存檔會寫幾鍵、型號。
//   keys：每一鍵 {key, edit, machine＝iAOA_* 記憶體, file＝磁碟值（沒有這鍵＝null）, writes＝OffsetSaveClick 現在會不會寫它}。
//   wouldChange：存檔會把檔案值改掉的鍵（檔案值 ≠ atoi(元件文字)，或檔案沒有這鍵）—— 頁面補件在確認框講出來。
std::string ExtraJson()
{
    const bool ht9011 = (AUTO_EMPTY_COLOR>=3);
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("gate").BeginObject();
    w.Key("MACHINE_HAS_AUTO_ALIGNMENT_CCD").Bool(MACHINE_HAS_AUTO_ALIGNMENT_CCD);
    w.Key("bEnableAutoAlignment").Bool(TestIF_File.bEnableAutoAlignment);
    w.Key("fillNow").Bool(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF_File.bEnableAutoAlignment==true);
    w.Key("bootFilled").Bool(g_filled);
    w.Key("AUTO_EMPTY_COLOR").Number((wb_int64)AUTO_EMPTY_COLOR);
    w.Key("writes").Number((wb_int64)(ht9011 ? 38 : 26));
    w.Key("bHandlerModel").Bool(bHandlerModel);
    w.EndObject();
    std::vector<std::string> change;
    w.Key("keys").BeginArray();
    for (int i = 0; i < kNKeys; ++i) {
        const AOAKey& k = kKeys[i];
        const bool writes = !k.bHT9011 || ht9011;
        const int edit = std::atoi(EL<TEdit>("TfMain", k.edit)->Text.c_str());
        const bool has = INIFileGeneral && INIFileGeneral->ValueExists("System", k.key);
        const int file = has ? INIFileGeneral->ReadInteger("System", k.key, 0) : 0;
        w.BeginObject();
        w.Key("key").String(k.key);
        w.Key("edit").String(k.edit);
        w.Key("editValue").Number((wb_int64)edit);
        w.Key("machine").Number((wb_int64)*k.var);
        w.Key("file");
        if (has) w.Number((wb_int64)file); else w.RawValue("null");
        w.Key("writes").Bool(writes);
        w.EndObject();
        if (writes && (!has || file != edit)) change.push_back(k.key);
    }
    w.EndArray();
    w.Key("wouldChange").BeginArray();
    for (std::size_t i = 0; i < change.size(); ++i) w.String(change[i]);
    w.EndArray();
    w.Key("runtime").BeginObject();
    w.Key("wired").Bool(false);
    w.Key("why").String("labXxx_X/Y and Memo3_AOA_IN/OUT are written by the AutoAlignment CCD process (golden AutoAlignment.cpp:3425/:4880/:5325), "
                        "not ported (Jimmy)");
    w.EndObject();
    w.EndObject();
    return w.Str();
}

const filerw::PageDesc kPage = {
    "AOAOffset", "TfMain", "Main.AOAInfo.html",
    nullptr, nullptr, 0,
    kAOA_SaveReads, (int)(sizeof(kAOA_SaveReads) / sizeof(kAOA_SaveReads[0])),
    &OpenPage, &SaveFlow, "OffsetSaveClick", &Reload, &Booted,
    nullptr, &ExtraJson,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfMain 建構（DFM 設計期狀態：38 個 Ed_*Offset_X/Y Text='0'、Visible=False）→ 存檔流程讀的替身 → 容器替身與父子
// → golden TfMain::FormShow main.cpp:11679-11724、:11736（AOA_FormShowAOA）。冪等。
// 前提：LoadMachineConfig（iAOA_*、MACHINE_HAS_AUTO_ALIGNMENT_CCD、AUTO_EMPTY_COLOR、bHandlerModel）與 W906_DoReadLastData(true)
//   （TestIF_File.bEnableAutoAlignment）之後。不讀檔、不寫檔（Gerneral.ini 的 AOA_* 由 LoadMachineConfig 讀好）。
// AOA_DfmItems（產生器照 main.dfm 給 TfMain 全部 11 個 TComboBox／TRadioGroup 的 Items）刻意不呼叫：本結構沒有 HTEditList、
//   沒用到任何下拉，呼叫只會替主畫面其他 11 個元件建出無關的替身（editlist.get 的 proxies 會多出來）。
void FileRW_AOAOffset_Boot()
{
    if (g_booted) return;
    (void)&AOA_DfmItems;
    AOA_DfmState();
    AOA_CreateSaveProxies();
    AOA_CreateContainerProxies();
    if (bHandlerModel == false) {
        std::printf("FileRW AOAOffset: golden TfMain::FormShow :11679-11724 fill skipped -- bHandlerModel=false "
                    "(golden main.cpp:9589-9594 terminates before reaching it); editors keep DFM \"0\"\n");
    } else {
        g_filled = (MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF_File.bEnableAutoAlignment==true);
        AOA_FormShowAOA();
    }
    std::printf("FileRW AOAOffset: TfMain AOA offset proxies ready (%d save reads) -- golden main.cpp FormShow :11679 fill=%s "
                "(MACHINE_HAS_AUTO_ALIGNMENT_CCD=%d bEnableAutoAlignment=%d) AUTO_EMPTY_COLOR=%d -> OffsetSaveClick writes %d keys\n",
                (int)(sizeof(kAOA_SaveReads) / sizeof(kAOA_SaveReads[0])), g_filled ? "yes" : "no",
                (int)MACHINE_HAS_AUTO_ALIGNMENT_CCD, (int)TestIF_File.bEnableAutoAlignment, AUTO_EMPTY_COLOR,
                (AUTO_EMPTY_COLOR>=3) ? 38 : 26);
    g_booted = true;
}
