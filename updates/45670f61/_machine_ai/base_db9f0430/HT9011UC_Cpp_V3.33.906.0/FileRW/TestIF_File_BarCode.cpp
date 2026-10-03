// ===========================================================================
//  FileRW/TestIF_File_BarCode.cpp -- TestIF_File 的 TfBarCode 半邊（2DID／Bar Code／Shuttle Float Check），C 形狀（具名替身）。
//  <recipe>\HandlerCondition.Data [Configuration]／[Lot Verification]（CC_KYEC_XILINX：system\Barcode.ini
//  [Configuration_Barcode(XILINX)]）。頁面：web/page/Setup.BarCode.html（頁面補件 web/page/ht9045_barcode_c.js）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TestIF_File_BarCode.py（每條 replace 附原因）。
//
//  golden V912 TfBarCode（BarCode\BarCode.cpp）由 tools/gen_editlist.py 轉成 TestIF_File_BarCode.gen.inc：
//    建構子（:131）       ＝ 表單橋只留 bShow=false、Multi2DSiteCH[][]＝cbAa／cbAb／cbBa／cbBb、tsCCD_Unloader 顯示
//    FormShow（:332）     ＝ 開頁：DoIniDataToForm（:1055）→ 顯示／權限（含 SetVisible :628）
//    spbSaveClick（:1268）＝ 存檔鈕：A02 → Multi2D 檢查 → WriteIniData（:1303-:1456）→ ReadFile → spbStartCom->Click()
//                            → fMain->BackupSetupFile → Write／ReadUnloaderClipIni（客戶專屬）→ JCETWhite2DIDShow（CC_JCET）
//    ReadFile（:673）     ＝ 讀檔器（存檔後、開機／換配方都是這一支）
//    FormClose（:531）    ＝ DoIniDataToForm（JerryYang 20250411：離開頁面刷新一次，避免誤存檔）＝ reload
//  沒有 HTEditList（elUnloaderClip 客戶專屬、未移植，見設定檔）：存檔流程讀的替身全部是 mustSend。
//  savedMark＝"BC_WriteIniData"：golden 第一個 WriteIniData（:1303 "Bottom 2D"）之前；之後的 :1309 Multi2D 2CCD 檢查仍可能 return，
//    但那時檔案已經寫了一鍵，saved 照實回報。
//
//  ---- 開機與換配方（golden 呼叫點，行號＝D:\HT9045 的 V912 main.cpp）----------------------------------------------
//    golden TfMain::DoReadLastData（main.cpp:9264）：:9361 fBarCode->ReadFile()、:9405 fBarCode->DoIniDataToForm()。
//    DoReadLastData 的呼叫點：TfMain::FormShow :9995（開程式）、TfMain::cbSetupFileNameChange :25724／:25772（換工作檔）。
//    另 main.cpp:28724、SECSGEM/uHGemHT9045.cpp:930（S2F41 1701）也呼叫 fBarCode->ReadFile()（移植樹仍用 BarCode/BarCode.cpp
//    的 TfBarCode::ReadFile，V906 翻譯，與 912 只差 bEnableBarcodeCSVCompare 兩行）。
//    → FileRW_BarCode_BootReadFile()（開機／換配方讀檔鏈用，印 log）、FileRW_BarCode_ReadFile()（純 golden 呼叫）、
//      FileRW_BarCode_DoIniDataToForm()（:9405）。
//
//  ---- 存檔前重播的 golden 事件（PageDesc::beforeApply，同 FileRW/TestIF_File_TesterIF.cpp 的做法）--------------------
//    chkMulti2DID  OnClick → chkMulti2DIDClick（:8825）：tsMulti2DID->TabVisible（Multi2D 分頁的元件能不能改）
//    rgMulti2DType OnClick → rgMulti2DTypeClick（:8861）→ SetMulti2DMap：cbAa..cbBb 的 Items／ItemIndex=0／Visible
//    順序：chkMulti2DID 先（rgMulti2DType 在 tsMulti2DID 裡，要先知道分頁看不看得見）。其餘元件照舊「頁面最後狀態、不觸發事件」。
//    不重播：cbUsePinInspectionClick（:11826，CosFunction.b2DUsePinInspection＝KYEC；對 CCD 送 @FNPN+／@FN2D+，SendCCDCommand 未移植）。
// ===========================================================================
#include "Public/cJSON.h"             // 放在產生檔之前（同 FileRW/TestIF_File_TesterIF.cpp）

#include "FileRW/TestIF_File_BarCode.gen.inc"

#include <cstdio>
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// PageDesc::reload：沒寫檔時把替身還原成記憶體值＝golden FormClose（:531）。
void Reload() { BC_FormClose(); }

// PageDesc::beforeApply（見檔頭）
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root) return;
    auto field = [&](const char* id, const char* key) -> const cJSON* {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, id);
        return (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    };
    // (1) golden TCheckBox：使用者點 → Checked 變 → OnClick=chkMulti2DIDClick
    const cJSON* v = field("chkMulti2DID", "checked");
    if (v && cJSON_IsBool(v) && filerw::ELEditable("TfBarCode", "chkMulti2DID")) {
        TCheckBox* ck = EL<TCheckBox>("TfBarCode", "chkMulti2DID");
        if (cJSON_IsTrue(v) != ck->Checked) {
            ck->Checked = cJSON_IsTrue(v);
            BC_chkMulti2DIDClick();
            handled->push_back("chkMulti2DID");
        }
    }
    // (2) golden TRadioGroup：點另一個選項 → ItemIndex 變 → OnClick=rgMulti2DTypeClick（SetMulti2DMap 重建 cbAa..cbBb）
    v = field("rgMulti2DType", "itemIndex");
    if (v && cJSON_IsNumber(v) && filerw::ELEditable("TfBarCode", "rgMulti2DType")) {
        TRadioGroup* rg = EL<TRadioGroup>("TfBarCode", "rgMulti2DType");
        const int iv = v->valueint;
        if (iv != rg->ItemIndex && iv >= 0 && iv < rg->Items->Count) {
            rg->ItemIndex = iv;
            BC_rgMulti2DTypeClick();
            handled->push_back("rgMulti2DType");
        }
    }
    cJSON_Delete(root);
}

const filerw::PageDesc kPage = {
    "TestIF_File_BarCode", "TfBarCode", "Setup.BarCode.html",
    nullptr, nullptr, 0,
    kBC_SaveReads, (int)(sizeof(kBC_SaveReads) / sizeof(kBC_SaveReads[0])),
    &BC_FormShow, &BC_spbSaveClick, "BC_WriteIniData", &Reload, &Booted,
    &BeforeApply, nullptr,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace
static void BC_EvBootProxies();   //AI(W906-EVB3) 20260928 [W906]：form.event 控制項缺的替身（BC-3 btResetBarCodeCount），定義在檔尾；佔用原本的空行
// golden TfBarCode 建構（HT9045.cpp CreateForm）：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身 → 容器替身與父子。冪等。
// 不讀檔（golden 讀檔在 DoReadLastData，見檔頭）。前提：CosFunction／CUSTOMER_CODE 已定（建構子的 bReadClipCodeFromUnloader 分支）。
void FileRW_BarCode_Boot()
{
    if (g_booted) return;
    BC_DfmItems();
    BC_DfmState();
    BC_TfBarCode();
    BC_CreateSaveProxies();
    BC_CreateContainerProxies(); BC_EvBootProxies();   //AI(W906-EVB3) 20260928 [W906]：檔尾（form.event 控制項的替身＋DFM 父層）；同一行附加
    std::printf("FileRW TestIF_File_BarCode: TfBarCode proxies ready (%d save reads) -- golden BarCode.cpp ctor :131\n",
                (int)(sizeof(kBC_SaveReads) / sizeof(kBC_SaveReads[0])));
    g_booted = true;
}

// golden fBarCode->ReadFile()（BarCode.cpp:673，V912）—— 純 golden 呼叫，不動 filerw session（別的 golden 流程裡也能用）。
void FileRW_BarCode_ReadFile()
{
    FileRW_BarCode_Boot();
    BC_ReadFile();
}

// golden fBarCode->DoIniDataToForm()（main.cpp:9405，DoReadLastData 尾段）
void FileRW_BarCode_DoIniDataToForm()
{
    FileRW_BarCode_Boot();
    BC_DoIniDataToForm();
}

// 開機／換配方讀檔鏈用：golden TfMain::DoReadLastData main.cpp:9361 那一格。ReadFile 裡記的 golden 待辦印到 log
// （開機沒有頁面 session；這裡開一個新的 session 收，印完就丟）。不要在別的 editlist 存檔流程裡呼叫這支（會蓋掉那次的 session）。
void FileRW_BarCode_BootReadFile()
{
    FileRW_BarCode_Boot();
    filerw::SessionBegin("");
    BC_ReadFile();
    std::printf("barcode.* chain loaded: HandlerCondition.Data [Configuration] -> TestIF_File (golden 912 TfBarCode::ReadFile) "
                "EnableBarCode=%d Bottom2D=%d Multi2D=%d MinLen=%d MaxLen=%d Delay=%d NoCodeToErr=%d AutoSkip=%d Retry=%d "
                "ChkShuttle=%d ChkLot=%d CSVCompare=%d; session=%s\n",
                (int)TestIF_File.bEnableBarCode, (int)TestIF_File.bEnableBottom2D, (int)TestIF_File.bEnableMulti2D,
                TestIF_File.iBarCodeMinLength, TestIF_File.iBarCodeMaxLength, TestIF_File.iBarCodeDelay,
                TestIF_File.iNoCodeDeviceToErr, (int)TestIF_File.bNoCodeDeviceAutoSkip, TestIF_File.iBarcodeRetryCount,
                (int)TestIF_File.bCheckCodeByShuttle, (int)TestIF_File.bCheckCodeByLot, (int)TestIF_File.bEnableBarcodeCSVCompare,   //AI(W906-TIF912-R10) 20260926: static 拿掉，改讀 cprod.h 真欄位
                filerw::SessionJson().c_str());
    filerw::SessionBegin("");
}

// ===========================================================================
//  //AI(W906-EVB3) 20260928 [W906]：WS form.event（Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上,
//  如果沒有移植的, 我們直接實作」；批次 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B3 的 BC-1／BC-3）。
//  規格：D:\HT9045\.claude\skills\ht9045-html-json\references\route-c-golden-bridge.md §3.0g。
//    BC-1 chkMulti2DID  click → golden chkMulti2DIDClick（BarCode.cpp:8825，tsMulti2DID 頁籤看不看得見）
//         rgMulti2DType click → golden rgMulti2DTypeClick（:8861 → SetMulti2DMap :8830 重建 cbAa..cbBb 的 Items／ItemIndex=0／Visible）
//         —— 已翻（產生檔），原本只在存檔前重播（上面 BeforeApply）。頁面點的當下送 form.event 之後，伺服器端已經等於頁面值，
//         存檔時 BeforeApply 比對不到差異、不再重播（行為不變）；沒送事件的舊頁面照舊由 BeforeApply 補。
//    BC-3 btResetBarCodeCount click → golden btResetBarCodeCountClick（:2424，iBarCodeNo[4][8]＝0；移植樹的全域
//         BarCode/BarCode_Shuttle2_CCDScan.cpp:77，模擬讀碼編號 :373-390 用）。按下即生效、不寫檔（golden 同）；存檔補點做不到
//         （按鈕沒有值，伺服器看不出按過）。按鈕在 gbSimu → gbManualTest 裡，gbManualTest 只有 SOFT_SIMULTE 且
//         AccessLevel>=iDefHonPrecLevel（非模擬版：CC_HONPREC_QC）才看得見（golden FormShow :337-341）⇒ 其他情況 ELOperable 擋。
//  事件表 kBC_Events 由 tools/gen_editlist.py 產生（TestIF_File_BarCode.gen.inc 檔尾）；btResetBarCodeCount 不在產生器的
//  names_used（處理器本體沒提到它）⇒ 沒有替身、沒有父子 ⇒ 這裡開機補建並接上 DFM 父層（同 FileRW/ArmSpeed_File.cpp R119BootProxies，d7fa099a）。
//  頁面送出點：D:\HT9045\web\page\ht9045_barcode_ev.js。
// ===========================================================================
static void BC_EvBootProxies()
{
    EL<TButton>("TfBarCode", "btResetBarCodeCount");                          // golden BarCode.h:540 TButton（dfm:2069）
    EL<TSpeedButton>("TfBarCode", "sbtExit");                                 //AI(W906-EVB10C-BC) 20260930 [W906]：Exit 鈕（golden BarCode.dfm:7182 TSpeedButton，Panel1 裡；父層已在產生的 kBC_ParentOf）——見檔尾
    static const char* const kEvParents[][2] = {{"btResetBarCodeCount", "gbSimu"}};   // golden BarCode.dfm:2069 在 gbSimu 底下
    filerw::ELSetParents("TfBarCode", kEvParents, (int)(sizeof(kEvParents) / sizeof(kEvParents[0])));
    for (std::size_t i = 0; i < sizeof(kBC_Events) / sizeof(kBC_Events[0]); ++i)
        if (!filerw::ELFind("TfBarCode", kBC_Events[i].control))
            std::printf("FileRW TestIF_File_BarCode: WARNING form.event control %s has no proxy (add it to BC_EvBootProxies)\n",
                        kBC_Events[i].control);
}
namespace {
filerw::PageEventsRegistrar g_evreg("TestIF_File_BarCode", kBC_Events, (int)(sizeof(kBC_Events) / sizeof(kBC_Events[0])));
}  // namespace

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 13 列；Steven 20260928「如果已經有移植, 就接上」、
//    20260929「照 BCB 的邏輯」）。golden TfBarCode::FormClose（V912 BarCode\BarCode.cpp:531-548）：bShow=false; tmr1->Enabled=false;
//    （bStartTest[3] 分支：只有未轉的手動 GPIB 讀碼測試會設 true，FormShow :348 設 false ⇒ 產生檔已閘）DoIniDataToForm();
//    ＝產生檔的 BC_FormClose（也是本頁的 PageDesc::reload）。bShow 是本 TU 的（移植樹 fBarCode 門面沒有這個成員），不讀檔、不碰讀碼器。
//  頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge）呼叫：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過
//    （filerw::PageCloseEdgeRefused；A02 的 Close() 之後 PageSave 已經用 reload＝BC_FormClose、Exit 鈕的 form.event（檔尾 EVB10C-BC 段）
//    golden sbtExitClick 的 Close() 已經跑過 BC_FormClose ⇒ 都不跑第二次）才跑。
//    運轉中照跑（skipWhileRunning＝false）：golden 是非模態 fBarCode->Show()（main.cpp:29916；Contact 頁 cContact.cpp:17174 也是 Show）。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

const char* FileRW_BarCode_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfBarCode proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("TestIF_File_BarCode")) return no;
    BC_FormClose();                                                             // golden BarCode\BarCode.cpp:531
    return "ran golden TfBarCode::FormClose (BarCode.cpp:531-548): bShow=false, tmr1 off, DoIniDataToForm (bStartTest[3] branch never true in the port)";
}

// ===========================================================================
//  AI(W906-EVB10C-BC) 20260930 [W906]：B10c 後續（D:\docs\ChangeLog\CHANGES_20260929_Steven.md §11.58g「後續」；Steven 20260928
//    「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」）—— Exit 鈕 sbtExit ＝ golden TfBarCode::sbtExitClick
//    （V912 BarCode\BarCode.cpp:2408-2417；DFM BarCode.dfm:7182 在 Panel1 裡，OnClick＝sbtExitClick :7246）：
//      :2410 sbtExit->Down=false（純畫面，不轉）
//      :2411 Close() → VCL 同步觸發 OnClose＝FormClose（:531：bShow=false、tmr1 關、DoIniDataToForm）＝產生檔的 BC_FormClose；
//            同時記 "closed" ⇒ ack.closed＝true（頁面才關視窗）、filerw::PageFormCloseRan（關窗邊緣不再跑第二次 FormClose）
//      :2412 bShow=false
//      :2413-2414 i2DIDCheckSH1Task=1、i2DIDCheckSH2Task=1 —— golden BarCode.cpp:46-47 的全域，移植樹 20260930 補在
//            D:\HT9045\HT9011UC_Cpp_V3.33.906.0\BarCode\BarCode.cpp 檔尾（golden BarCode.h:1008-1009 的 extern 沒補進移植樹 BarCode\BarCode.h ——
//            它經 aHotPlateSubstrate.h 進 221 支 TU；產生檔自己前置宣告）。golden 全樹只有 State Record 的 task 環
//            讀它（main.cpp:10412-10413）；移植樹那兩列是 Jimmy 的 GATE(W906-TASKLIST)（cStateRecord.cpp:1944-1949，理由「沒有這個變數」
//            現在不成立），本次不動，列給 Jimmy。
//      :2415-2416 btStart2DIDCheckSh1／Sh2->Enabled=true —— 替身照 golden 設（手動 2DID 檢查 Do2DIDCheck／btStart2DIDCheckSh1Click
//            沒翻：csystem.cpp:31425 GATE(W906-HOME-W1-BARCODEARM)）。
//    處理器由產生器照 golden 轉（TestIF_File_BarCode.gen.inc 的 BC_sbtExitClick、kBC_Events；設定 tools\editlist\TestIF_File_BarCode.py
//    的 _BC_EX* 與 events）。頁面送出點：D:\HT9045\web\page\ht9045_barcode_ev.js 的 Exit 段（捕獲階段攔 .exitbtn，送
//    {"form":"TfBarCode","control":"sbtExit","event":"click"}；ack.closed 才關視窗）。golden 視窗的 ✕ 只跑 FormClose（上面 FileRW_BarCode_WindowEdge）。
//  ⚠ 跟 golden 不同（寫明）：運轉中（SystemStart||SoftStart）form.event 一律回 running（FileRW/_FormEvent.cpp:77，RULINGS_20260927 第 2 條第 7 題 A）
//    ⇒ 頁面照樣關視窗，關窗邊緣跑 FormClose（B10c 本頁運轉中照跑），:2413-2416 那四行沒跑。今天沒有影響：那兩個全域沒有讀者（State Record
//    那兩列閘著）、兩顆 2DID 檢查鈕的處理器沒翻。
// ===========================================================================
