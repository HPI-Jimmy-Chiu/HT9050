// ===========================================================================
//  FileRW/TestIF_File_TesterIF.cpp -- TestIF_File 的 TFTestIF 半邊（C 形狀：具名替身）
//  （<recipe>\Tester.Data；RS232 模式另寫 D:\RS232Standard\System\Setup.ini）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TestIF_File_TesterIF.py（每條 replace 附原因）。
//
//  golden V912 TFTestIF（cTesterIF.cpp）由 tools/gen_editlist.py 轉成 TestIF_File_TesterIF.gen.inc：
//    建構子（:40）      ＝ InitcbDIOType(false)（DIOCFGPath\*.ini → cbDIOType->Items）＋客戶碼的顯示調整
//    FormShow（:109）   ＝ 開頁：InitcbDIOType → ReadTestIFFile（尾端 DoIniDataToForm）→ 顯示／權限
//    spbSaveClick（:1320）＝ 存檔鈕：A02 → SaveSetupFile → SECS → ReadTestIFFile → 備份 → SetWorkParameter
//    ReadTestIFFile（:563）＝ 讀檔器（開頁、存檔後、開機／換配方都是這一支）
//  沒有 HTEditList：存檔流程讀的替身全部是 mustSend。結構名不是 TestIF_File：那個 tag 已被 A 形狀
//  FileRW/TestIF_File.cpp（kBridge_TFTestIF／TfSetup／TfYieldMonitoring）佔用。
//
//  開機／換配方（Steven 20260925：Setup 項目開程式與變更工作檔都要照 BCB 讀一次）：
//    golden TfMain::DoReadLastData（main.cpp:9262）在 :9327 呼叫 FTestIF->ReadTestIFFile()、:9387 FTestIF->DoIniDataToForm()；
//    golden 換配方 TfMain::ChangeSetUpFile（main.cpp:25653）→ WriteLastDataFN → DoReadLastData（:25722，:25770 再一次）。
//    → FileRW_TesterIF_Boot()（CreateForm）＋FileRW_TesterIF_BootReadTestIFFile()（DoReadLastData 那一格），整合者接到
//      wb_serve 的開機讀檔鏈與 ReloadRecipeDocAfterSave（取代 FileRW_TTLCfg_ReadDIOSection：它是這支的 [DIO] 子集）。
//    FileRW_TesterIF_ReadTestIFFile() 是不動 session 的純 golden 呼叫，給別的 golden 流程裡的 FTestIF->ReadTestIFFile() 用
//    （golden cConfiguration.cpp:7400、uTemp_Set.cpp:3186／:5162、SCK_ART.cpp:433 —— 目前各自是 GATE／子集）。
//
//  存檔前重播的 golden 事件（PageDesc::beforeApply → BeforeApply()，同 FileRW/TestIF_File_SetUp.cpp 的做法）：
//    頁面送來的值與伺服器端不同時，golden 是「使用者改了 → 觸發事件」，事件會改別的元件的可見／可改：
//      rgInterfaceType  OnClick  → rgInterfaceTypeClick（:1157）：Barcode／bflag 檢查、ShowPageControl2（四個分頁的 TabVisible）
//      cbRs232Type      OnChange → cbRs232TypeChange（:1312）：gbRs232BinCount 顯示（edMaxBinCount 能不能改）
//      cbDIOType        OnChange → cbDIOTypeChange（:1299）：GetDIOFileName＋LoadData（載入執行期 TTLCfg！）＋ShowTTLState
//      AntiSignalCBox   OnClick  → AntiSignalCBoxClick（:1307）：TestIF.bAntiSignal（執行期）
//      cbASEJPMode      OnClick  → AntiSignalCBoxClick（golden DFM :137 綁同一支，照抄）
//    事件跑完才判斷哪些元件可改、套頁面值；其餘元件照舊「頁面最後狀態、不觸發事件」。
//    cbbBaudRate 的 OnChange 也是 cbRs232TypeChange（DFM :477），但它只看 cbRs232Type → 重播與否結果相同，不重播。
//
//  存檔前的兩道網頁版檢查（golden 沒有，逐條寫在交件報告）：
//    (1) bDioListLost：golden InitcbDIOType 找不到 DIO 檔時 Application->Terminate()（程式結束，存不到檔）→ 拒存。
//    (2) 數字預檢：golden SaveSetupFile 對 33 個欄位做 ->Text.ToDouble()（kTIF_ToDoubleTexts），非數字時 BCB 丟
//        EConvertError、vclcompat 丟 std::runtime_error —— 都是寫到一半才停（Tester.Data 前面幾鍵已寫）。
//        golden 靠 fQwertyKey 小鍵盤擋住非數字輸入；網頁可以直接打字 → 先全部試轉，一個不過整筆不寫（G4 全有或全無）。
// ===========================================================================
#include "Public/cJSON.h"             // 放在產生檔之前（同 FileRW/TestIF_File_SetUp.cpp）
#include "WebBridge/JsonWriter.h"
#define __FUNC__ __func__   //AI(W906-GB-P6-FIX) 20260926: the gen.inc now keeps golden :557 `fMain->CloseGpibProgram(__FUNC__);` (BCB macro); same token list as the canary_support.h:45 shim, so an identical redefinition is legal. Was a blank line.
#include "FileRW/TestIF_File_TesterIF.gen.inc"
#undef fDIOFrom                        // 產生檔 members 的 #define（只給 golden ShowTTLState 用）

#include <cstdio>
#include <exception>
#include <string>
#include <vector>
#include "WebTeachLeave.h"             // AI(W906-Q41) 20260928 (St02-E helper): W906_WindowEdgeRegister (St01 5b73d905), used at :272. Was a blank line
#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }   void TIF_Q41_CloseTail(); void TIF_Q41_W6ApplyGpibRs232(const cJSON* r, std::vector<std::string>* h); bool TIF_Q41_W6Editable(const char* n); void TIF_Q41_DioPreview(webbridge::JsonWriter& jw);   // AI(W906-Q41) 20260927 (St02-E): 本體在檔尾

const char* const kPC2Tabs[4] = {"tsDio", "tsGpib", "tsRs232", "tsTCPIP"};   // golden ShowPageControl2 :1194 tsTemp[]

TComboBox* Cb(const char* n) { return EL<TComboBox>("TFTestIF", n); }

// (2) 數字預檢：golden SaveSetupFile 的 ->Text.ToDouble()（同一個 vclcompat 轉換，試轉不寫）
bool NumbersOk(std::string* bad)
{
    for (const char* n : kTIF_ToDoubleTexts) {
        TCustomEdit* e = dynamic_cast<TCustomEdit*>(filerw::ELFind("TFTestIF", n));
        if (!e) { *bad += std::string(bad->empty() ? "" : ", ") + n + "(no proxy)"; continue; }
        try {
            (void)e->Text.ToDouble();
        } catch (const std::exception&) {
            *bad += std::string(bad->empty() ? "" : ", ") + n + "=\"" + e->Text.c_str() + "\"";
        }
    }
    return bad->empty();
}

// PageDesc::saveFlow：golden 存檔鈕 spbSaveClick，前面是兩道網頁版檢查（檔頭 (1)(2)）
void SaveFlow()
{
    if (bDioListLost) {
        filerw::ELMessage("refused: DIO data has been lossed (no *.ini in DIOCFGPath) -- golden terminates the program at "
                          "TFTestIF::InitcbDIOType; nothing written",
                          "DIO 資料遺失（DIOCFGPath 沒有 *.ini）：golden 會結束程式，網頁版拒絕存檔，未寫檔");
        return;
    }
    std::string bad;
    if (!NumbersOk(&bad)) {
        AnsiString en, zh;
        en.sprintf("refused: not a number (golden SaveSetupFile ->Text.ToDouble() would stop half-way): %s -- nothing written", bad.c_str());
        zh.sprintf("拒絕寫入：這些欄位不是數字（golden 存檔會寫到一半中斷）：%s —— 未寫檔", bad.c_str());
        filerw::ELMessage(en, zh);
        return;
    }
    {
        // AI(W906-GB-P6) 20260926: decision #5 Q3(1) 暫照建議 A -- refuse the RS232 framings Windows' SetCommState rejects
        //   (5 data bits + 2 stop, 6/7/8 data bits + 1.5 stop): SPComm / vclcompat ignore that failure, so the port would
        //   silently keep its old framing.  Only when the interface is RS232 -- the only case golden writes these values to
        //   Setup.ini (CheckRs232StandardIni) and opens the port.  AI(W906-Q41) 20260927 (St02-E): and GPIB mode -- W6 makes the four editable there and P6 Q2 (a) opens the GPIB program's extra port with them.
        TRadioGroup* itf = EL<TRadioGroup>("TFTestIF", "rgInterfaceType");
        const int bl = EL<TRadioGroup>("TFTestIF", "rgBitLength")->ItemIndex;
        const int sb = EL<TRadioGroup>("TFTestIF", "rgStopBit")->ItemIndex;
        if (itf && (itf->ItemIndex == RS232_MODE || itf->ItemIndex == GPIB_MODE) && !W906_Rs232FramingValid(bl, sb)) {   // AI(W906-Q41) 20260927: GPIB_MODE added (W6; was RS232 only)
            filerw::ELMessage("refused: RS232 framing not accepted by Windows (5 data bits + 2 stop bits, or 6/7/8 data bits + "
                              "1.5 stop bits) -- nothing written",
                              "拒絕寫入：Windows 不接受這組 RS232 格式（5 bits 配 2 stop，或 6／7／8 bits 配 1.5 stop）—— 未寫檔");
            return;
        }
    }
    TIF_spbSaveClick();
    if (filerw::ELMarked("SaveSetupFile")) {
        // golden 在「關表單」才做的執行期收尾：TFTestIF::FormClose（golden 906_0625_Steven cTesterIF.cpp:1197-1217；912 :1215-1235）
        // ＋主畫面 TfMain::sbTesterClick 在 ShowModal 回來之後的尾段（golden 906_0625_Steven main.cpp:27425-27432；912 :28337-28344）。
        // AI(W906-Q41) 20260927 (St02-E): Q41 TI-4／TI-5。網頁沒有關表單事件 → 照 S88 目前的做法（SetUp／Temp_Set 同）存檔成功就跑
        //   （觸發時機仍是 S88 待決 Q1）。以前這裡只記兩筆 ELTodo（oldLastiTestMode／TTL 收尾）；本體在檔尾 TIF_Q41_CloseTail，逐行照 golden。
        // TIF_Q41_CloseTail：本體在檔尾（匿名 namespace；前置宣告在 :54）
        TIF_Q41_CloseTail();
        //   沒存檔就關頁（golden 一樣會跑 FormClose）：AI(W906-Q41) 20260927 Q41 (d) 檔尾 TIF_OnPageClosed（視窗總表的關掉邊緣），一次開→關只跑一次（存檔跑過就不再跑）
        //   -- registered with St01's W906_WindowEdgeRegister (WebTeachLeave.h, 5b73d905) in FileRW_TesterIF_Boot (:272). AI(W906-Q41) 20260928 (St02-E helper): was "waits for 5b73d905 to reach main".
    }
}

// PageDesc::reload：沒寫檔時把替身還原成檔案值＝golden FormClose（:1217）的 ReadTestIFFile()（尾端 DoIniDataToForm）。
// FormClose 其餘（fShow／bflag 清除、oldLastiTestMode、TTL 收尾）是關表單的事，不在「還原替身」的範圍。
void Reload() { TIF_ReadTestIFFile(); }

// PageDesc::beforeApply（見檔頭）。順序：rgInterfaceType → cbRs232Type → cbDIOType → AntiSignalCBox → cbASEJPMode
// （分頁先定，cbRs232Type 能不能改才對；AntiSignal 在 cbASEJPMode 之前，最後 TestIF.bAntiSignal 都＝新的 AntiSignalCBox）。
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root) return;
    auto num = [&](const char* id, const char* key, int* out) -> bool {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, id);
        const cJSON* v = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
        if (!v || !cJSON_IsNumber(v)) return false;
        *out = v->valueint;
        return true;
    };
    auto boolv = [&](const char* id, bool* out) -> bool {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, id);
        const cJSON* v = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, "checked") : nullptr;
        if (!v || !cJSON_IsBool(v)) return false;
        *out = cJSON_IsTrue(v);
        return true;
    };
    int iv = 0;
    bool bv = false;
    // (1) golden TRadioGroup：使用者點另一個選項 → ItemIndex 變 → OnClick=rgInterfaceTypeClick
    if (num("rgInterfaceType", "itemIndex", &iv) && filerw::ELEditable("TFTestIF", "rgInterfaceType")) {
        TRadioGroup* rg = EL<TRadioGroup>("TFTestIF", "rgInterfaceType");
        if (iv != rg->ItemIndex && iv >= 0 && iv < rg->Items->Count) {
            rg->ItemIndex = iv;
            TIF_rgInterfaceTypeClick();       // golden 可能把它改回 TestIF_File.iTestType（Barcode／bflag）→ 結果以 golden 為準
            handled->push_back("rgInterfaceType");
        }
    }
    // (2) golden TComboBox：選別的項 → OnChange=cbRs232TypeChange
    if (num("cbRs232Type", "itemIndex", &iv) && filerw::ELEditable("TFTestIF", "cbRs232Type")) {
        TComboBox* cb = Cb("cbRs232Type");
        if (iv != cb->ItemIndex) {
            filerw::ELComboIndex(cb, iv);
            TIF_cbRs232TypeChange();
            handled->push_back("cbRs232Type");
        }
    }
    // (3) golden TComboBox cbDIOType：選別的 DIO 檔 → OnChange=cbDIOTypeChange（LoadData 載入執行期 TTLCfg，golden 也是一選就載）
    if (num("cbDIOType", "itemIndex", &iv) && filerw::ELEditable("TFTestIF", "cbDIOType")) {
        TComboBox* cb = Cb("cbDIOType");
        if (iv != cb->ItemIndex && iv >= 0 && iv < cb->Items->Count) {
            filerw::ELComboIndex(cb, iv);
            TIF_cbDIOTypeChange();
            handled->push_back("cbDIOType");
        }
    }
    // (4)(5) golden TCheckBox OnClick=AntiSignalCBoxClick（AntiSignalCBox 與 cbASEJPMode 都綁這支，DFM :128／:137）
    for (const char* id : {"AntiSignalCBox", "cbASEJPMode"}) {
        if (!boolv(id, &bv) || !filerw::ELEditable("TFTestIF", id)) continue;
        TCheckBox* ck = EL<TCheckBox>("TFTestIF", id);
        if (bv != ck->Checked) {
            ck->Checked = bv;
            TIF_AntiSignalCBoxClick();
            handled->push_back(id);
        }
    }
    TIF_Q41_W6ApplyGpibRs232(root, handled);  cJSON_Delete(root);   // AI(W906-Q41) 20260927 (St02-E): W6（檔尾）；在 rgInterfaceType 重播之後
}

// 頁面在瀏覽器端照 golden ShowPageControl2＋cbRs232TypeChange 切分頁時，哪些存檔必送的元件可以改（伺服器端同一個 ELEditable 判斷）：
// 對 4 種 Interface Type × cbRs232Type 是否為 0，暫時照 golden 設四個分頁的 TabVisible 與 gbRs232BinCount->Visible，算完還原。
void EditableMatrix(webbridge::JsonWriter& w)
{
    TTabSheet* tabs[4];
    bool keepTab[4];
    for (int i = 0; i < 4; ++i) { tabs[i] = EL<TTabSheet>("TFTestIF", kPC2Tabs[i]); keepTab[i] = tabs[i]->TabVisible; }
    TGroupBox* bin = EL<TGroupBox>("TFTestIF", "gbRs232BinCount");
    const bool keepBin = bin->Visible;
    w.Key("editable").BeginObject();
    for (int t = 0; t < 4; ++t)
        for (int rs = 0; rs < 2; ++rs) {
            for (int i = 0; i < 4; ++i) tabs[i]->TabVisible = (i == t);   // golden ShowPageControl2 :1195-1197
            bin->Visible = (rs != 0);                                      // golden cbRs232TypeChange :1314-1317
            char key[8];
            std::snprintf(key, sizeof(key), "%d:%d", t, rs);
            w.Key(key).BeginArray();
            for (const char* n : kTIF_SaveReads)
                if (filerw::ELEditable("TFTestIF", n) || (t == GPIB_MODE && TIF_Q41_W6Editable(n))) w.String(n);   // AI(W906-Q41) 20260927: W6 GPIB 列多四個
            w.EndArray();
        }
    w.EndObject();
    for (int i = 0; i < 4; ++i) tabs[i]->TabVisible = keepTab[i];
    bin->Visible = keepBin;
}

// PageDesc::extraJson：
//   items   —— golden 執行期的下拉選項（cbDIOType＝InitcbDIOType 列的 DIOCFGPath\*.ini；cbGPIBType 在 CC_ASE_KaohSiung 被建構子改名）。
//              頁面的 cbDIOType 只有一個佔位選項（DFM 沒有 Items），要照這份重建。
//   lstTTL  —— golden ShowTTLState（:1238）填的 TTL 設定摘要（TListBox 的 Items；通用 proxies 不帶清單內容）。
//   editable —— 見 EditableMatrix。
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("items").BeginObject();
    for (const char* n : {"cbDIOType", "cbGPIBType", "cbRs232Type", "cbbBaudRate", "iStartDelayCount"}) {
        TStringList* it = Cb(n)->Items;
        w.Key(n).BeginArray();
        for (int i = 0; i < it->Count; ++i) w.String(it->Strings[i].c_str());
        w.EndArray();
    }
    w.EndObject();
    w.Key("lstTTL").BeginArray();
    {
        TStringList* it = EL<TListBox>("TFTestIF", "lstTTL")->Items;
        for (int i = 0; i < it->Count; ++i) w.String(it->Strings[i].c_str());
    }
    w.EndArray();
    EditableMatrix(w);
    w.Key("dioListLost").Bool(bDioListLost); TIF_Q41_DioPreview(w);   // AI(W906-Q41) 20260927 (St02-E): TI-1（檔尾）
    w.EndObject();
    return w.Str();
}

filerw::PageDesc g_page = {
    "TestIF_File_TesterIF", "TFTestIF", "Setup.TesterIF.html",
    nullptr, nullptr, 0,
    kTIF_SaveReads, (int)(sizeof(kTIF_SaveReads) / sizeof(kTIF_SaveReads[0])),
    &TIF_FormShow, &SaveFlow, "SaveSetupFile", &Reload, &Booted,
    &BeforeApply, &ExtraJson,
};
filerw::PageRegistrar g_reg(&g_page);
}  // namespace
static void TIF_OnPageClosed();   // AI(W906-Q41) 20260927 (St02-E): Q41 (d) 關頁掛勾，本體在檔尾
// golden TFTestIF 建構（HT9045.cpp:189 CreateForm，TfSetup :184／TfContact :185 之後、TfDIOFrom :196 之前）：
// DFM 設計期狀態 → 建構子（InitcbDIOType(false)）→ 存檔流程讀的替身 → 容器替身與父子。冪等。
// 前提：DIOCFGPath 已定（LoadMachineConfig）、CUSTOMER_CODE 已定（建構子的客戶碼分支）。
void FileRW_TesterIF_Boot()
{
    if (g_booted) return;
    TIF_DfmItems();
    // AI(W906-GB-P6) 20260926: user ruling 5B (暫照建議，待使用者確認 decision #5): the RS232 options golden's DFM lacks,
    //   APPENDED so recipe indices keep their meaning (TesterComm/Rs232SetupCodes.h has the Setup.ini mapping).
    //   The page (web/page/Setup.TesterIF.html:56) has them since AI(W906-Q41) 20260927 (St02-E; W6 = A, Steven 20260927).
    EL<TRadioGroup>("TFTestIF", "rgBitLength")->Items->Add("5 Bits");     // index 2
    EL<TRadioGroup>("TFTestIF", "rgBitLength")->Items->Add("6 Bits");     // index 3
    EL<TRadioGroup>("TFTestIF", "rgStopBit")->Items->Add("1.5 Bits");     // index 2
    EL<TRadioGroup>("TFTestIF", "rgParity")->Items->Add("Mark");          // index 3
    EL<TRadioGroup>("TFTestIF", "rgParity")->Items->Add("Space");         // index 4
    TIF_DfmState();
    TIF_TFTestIF();
    TIF_CreateSaveProxies();
    TIF_CreateContainerProxies();
    std::printf("FileRW TestIF_File_TesterIF: TFTestIF proxies ready (%d save reads, %d DIO files in DIOCFGPath) -- golden cTesterIF.cpp ctor :40\n",
                (int)(sizeof(kTIF_SaveReads) / sizeof(kTIF_SaveReads[0])), Cb("cbDIOType")->Items->Count);
    g_booted = true;   W906_WindowEdgeRegister("FTestIF", nullptr, &TIF_OnPageClosed, false);   // AI(W906-Q41) 20260928 (St02-E helper): Q41 (d) close hook registered (St01 WebTeachLeave.h, 5b73d905; include :49); was a no-op address-of until 5b73d905 was in this branch. "FTestIF" = background.html:447 form; onOpen unused (TIF_FormShow already runs at editlist.get); false = golden FormClose has no run-state condition; a refusal prints "[WinEdge] register refused" (WebTeachLeave.cpp)
}

// golden FTestIF->ReadTestIFFile()（cTesterIF.cpp:563）—— 純 golden 呼叫，不動 filerw session（別的 golden 存檔流程裡也能用）。
void FileRW_TesterIF_ReadTestIFFile()
{
    FileRW_TesterIF_Boot();
    TIF_ReadTestIFFile();
}

// golden FTestIF->DoIniDataToForm()（main.cpp:9387，DoReadLastData 尾段）
void FileRW_TesterIF_DoIniDataToForm()
{
    FileRW_TesterIF_Boot();
    TIF_DoIniDataToForm();
}

// AI(W906-GB-P6) 20260926: ruling 2A + Q2 (a) -- TesterComm/Handler/HandlerGpibAux.cpp seeds a GPIB-mode recipe's [RS-232C]
//   from Setup.ini once, then calls this so the page's widgets match the file (a later page save writes every widget).
//   Installed at static init: FileRW is compiled into wb_serve only, ht9045_testercomm_handler cannot call it directly.
extern void (*W906_GpibAuxRefreshTesterIfPage)();
namespace { struct TIF_GpibAuxHook { TIF_GpibAuxHook() { W906_GpibAuxRefreshTesterIfPage = &FileRW_TesterIF_DoIniDataToForm; } } g_tifGpibAuxHook; }  namespace ela { typedef bool (*EffectiveO10Getter)(); void SetEffectiveO10Getter(EffectiveO10Getter g); }  namespace { struct TIF_ElaO10Seat { TIF_ElaO10Seat() { ela::SetEffectiveO10Getter([]() { return IniConfig.bO10UseEventLogSaver; }); } } g_tifElaO10Seat; }   // AI(W906-ELA-R5) 20260927 (St02-E): the ELA O10 gate reads the Handler's live IniConfig.bO10UseEventLogSaver (V906 cprod.cpp:2519 = golden cprod.cpp:2379-2394 already applied) at every decision -- ElaReports.h EffectiveO10.  Here, like the hook left of it, because FileRW is compiled into wb_serve only (wb_serve links ht9045_ela; TesterCommWiring.cpp is in ht9045_testercomm_handler, which test_testercomm_handler links without ht9045_ela).

// 開機／換配方讀檔鏈用：golden TfMain::DoReadLastData main.cpp:9327 那一格。ReadTestIFFile 裡記的 golden 訊息／待辦印到 log
// （開機沒有頁面 session；這裡開一個新的 session 收，印完就丟）。不要在別的 editlist 存檔流程裡呼叫這支（會蓋掉那次的 session）。
void FileRW_TesterIF_BootReadTestIFFile()
{
    FileRW_TesterIF_Boot();
    filerw::SessionBegin("");
    TIF_ReadTestIFFile();
    std::printf("tester.* chain loaded: Tester.Data -> TestIF_File (golden ReadTestIFFile) TesterType=%d DIO=\"%s\"(%d) GPIB=%d/%d "
                "RS232 Mode=%d Baud=%d MaxTime=%.2f InitMaxTime=%.2f iTestBinCount=%d; session=%s\n",
                TestIF_File.iTestType, TestIF_File.sDioName.c_str(), TestIF_File.iDioMode, TestIF_File.iGpibMode,
                TestIF_File.iGpibAddress, (int)TestIF_File.iRs232Mode, TestIF_File.Rs232_Data.Baud_Rate,
                (double)TestIF_File.iMaxTime, (double)TestIF_File.iInitialMaxTime, iTestBinCount,
                filerw::SessionJson().c_str());
    filerw::SessionBegin("");
}
// ===========================================================================
//  AI(W906-Q41) 20260927 (St02-E): Q41（Steven 20260927 裁決 S158）Setup.TesterIF 的 TI-1／TI-4／TI-5 與 W6（Steven 20260927 W6＝A）。
//  清單：St01 盤點 docs/Q41_INVENTORY_20260927.md §3.7（23efc733）。golden 引用寫樹名；906_0625_Steven 與 912 的這幾支本體相同，只差行號。
//
//  TI-1 TIF_Q41_DioPreview —— golden TFTestIF::cbDIOTypeChange（906_0625_Steven cTesterIF.cpp:1281-1287；912 :1299-1305）
//    ＝ GetDIOFileName＋LoadData（載入執行期 TTLCfg）＋ShowTTLState（lstTTL 摘要）。C 路只有開頁／存檔兩個入口（C-4 form.event 還沒有），
//    所以：執行期那一半照舊在存檔前重播（BeforeApply (3)）；摘要那一半在開頁時先替 cbDIOType 的每一項算好（extra.dioPreview），
//    頁面一換就照表換 lstTTL。算法是 golden 原文的機械複製，差別只在「寫到區域變數、不碰執行期」：
//      * GetDIOFileName（DIOInterFaceCFG.cpp:47-70，906／912 同行號）不做 :56-63 的 CopyFile（預覽不寫檔；golden 複製的就是
//        DIOCFGPath 那一份，讀它內容相同）；
//      * LoadData（:72-131）讀進 TTL_DATA 區域副本，不呼叫尾端 :130 DoIniDataToForm（那是 TfDIOFrom 的替身，不能被預覽改掉）；
//        檔案不在 → golden :76-84 直接 return（TTLCfg 不變，清單照舊）＋TTL 模式時 ShowMyMessage＋SystemStart=false —— 預覽只帶
//        missing／lostMsg，SystemStart 等存檔重播時照 golden 清；
//      * ShowTTLState 的清單段（cTesterIF.cpp:1222-1259；912 :1240-1277）逐行複製（產生檔 :1781-1816 機械轉換）；
//        尾段 :1261-1278 的檢查：TTL_CARD_TYPE 2／3 的 CheckTTLBoardBitMode 讀的是 Prod.DIOCfg（不是剛讀的 TTLCfg），
//        跟選哪一個檔無關、開頁 FormShow 已跑過 → 預覽不跑（它會跳 ShowMyMessage）；其他卡的 8／10 bit 提示只看剛讀的值 → 帶 ttlWarn，
//        頁面在 rgInterfaceType 是 DIO 時顯示（golden 的條件 :1261）。
//
//  TI-4／TI-5 TIF_Q41_CloseTail —— SaveFlow 存檔成功後呼叫（見 SaveFlow 那段的註解；時機＝S88 目前做法，S88 Q1 待決）。
//
//  W6 TIF_Q41_W6Editable／TIF_Q41_W6ApplyGpibRs232 —— golden 在 GPIB 模式只顯示 GPIB 分頁（ShowPageControl2），pnlRS232 那四個
//    （cbbBaudRate／rgBitLength／rgStopBit／rgParity）在 golden 改不到；P6 Q2 (a) 之後 GPIB 程式的額外 RS232 port 用配方這四個值，
//    Steven 20260927 W6＝A：GPIB 模式也要能改。刻意偏離 golden，只放寬這四個，而且只在「假設 golden 把 RS232 分頁打開時它可改」
//    的情況（等級／Enabled 照 golden FormShow，例 CC_SCC／CC_SCK 的 tsRs232->Enabled=false，golden 906_0625_Steven cTesterIF.cpp:208）。
//    golden SaveSetupFile 本來就不分模式寫 [RS-232C] 這四鍵（906_0625_Steven cTesterIF.cpp:397-400）。
// ===========================================================================
#include "TesterComm/Handler/HandlerTesterSide.h"   // fTesterSide->oldLastiTestMode（golden main.h:1566 的 TfMain 成員，移植在 THandlerTesterSide）
void DoStructUnitConvert();                          // cUnitConvert.h:118（golden cUnitConvert.h:5）；同 FileRW/MainClick.cpp（St01）的做法，只宣告
void FileRW_TTLCfg_DoReadLastDataLoad();             // FileRW/TTLCfg.cpp（St01 的檔，只呼叫）：golden S=fDIOFrom->GetDIOFileName(); fDIOFrom->LoadData(S);

namespace {

const char* const kQ41W6Rs232[4] = {"cbbBaudRate", "rgBitLength", "rgStopBit", "rgParity"};   // golden pnlRS232／gbBaudRate 的四個存檔值

bool TIF_Q41_IsW6(const char* n)
{
    for (const char* k : kQ41W6Rs232)
        if (std::strcmp(k, n) == 0) return true;
    return false;
}

// W6：n 是那四個之一，而且 golden 若把 tsRs232 分頁顯示出來它就可改（其他條件照 golden：自己／祖先的 Enabled、ReadOnly…）
bool TIF_Q41_W6Editable(const char* n)
{
    if (!TIF_Q41_IsW6(n)) return false;
    TTabSheet* ts = EL<TTabSheet>("TFTestIF", "tsRs232");
    const bool keep = ts->TabVisible;
    ts->TabVisible = true;
    const bool ok = filerw::ELEditable("TFTestIF", n);
    ts->TabVisible = keep;
    return ok;
}

// W6：BeforeApply 的最後一步（rgInterfaceType 已照 golden 重播，golden 可能把它改回檔案值）。介面是 GPIB 時，頁面送來、跟伺服器端
// 不同的四個值自己套（套法同 FileRW/_EditList.cpp ELApplyProxies 的 PK::Index），回報成 handled（PageSave 不再丟它們）。
void TIF_Q41_W6ApplyGpibRs232(const cJSON* root, std::vector<std::string>* handled)
{
    TRadioGroup* itf = EL<TRadioGroup>("TFTestIF", "rgInterfaceType");
    if (!root || !itf || itf->ItemIndex != GPIB_MODE) return;
    for (const char* n : kQ41W6Rs232) {
        if (!TIF_Q41_W6Editable(n)) continue;
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, n);
        const cJSON* v = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, "itemIndex") : nullptr;
        if (!v || !cJSON_IsNumber(v)) continue;
        if (std::strcmp(n, "cbbBaudRate") == 0) {
            TComboBox* cb = Cb(n);
            const cJSON* t = cJSON_GetObjectItemCaseSensitive(o, "text");
            const bool hasText = t && cJSON_IsString(t);
            if (v->valueint == cb->ItemIndex && (!hasText || cb->Text == AnsiString(t->valuestring))) continue;
            cb->ItemIndex = v->valueint;
            if (hasText) cb->Text = AnsiString(t->valuestring);
        } else {
            TRadioGroup* rg = EL<TRadioGroup>("TFTestIF", n);
            if (v->valueint == rg->ItemIndex || v->valueint < 0 || v->valueint >= rg->Items->Count) continue;
            rg->ItemIndex = v->valueint;
        }
        handled->push_back(n);
    }
}

// TI-1（見上方區塊註解）。頁面讀 extra.dioPreview[k]，k 對 extra.items.cbDIOType 的第 k 項（兩者同一次 ExtraJson、同一份 Items）。
void TIF_Q41_DioPreview(webbridge::JsonWriter& jw)
{
    TComboBox* cbDio = Cb("cbDIOType");
    jw.Key("dioPreview").BeginArray();
    for (int k = 0; k < cbDio->Items->Count; ++k) {
        const AnsiString name = cbDio->Items->Strings[k];
        // golden GetDIOFileName（DIOInterFaceCFG.cpp:47-70），FTestIF->cbDIOType->Text＝第 k 項；:56-63 的 CopyFile 不做
        AnsiString S, S1;
        AnsiString szDir="";
        if(IniConfig.bI16TTLSaveInSetupFile)
        {
            S=GetLastOpenFN();

            S1.sprintf("%s%s\\%s.ini", DataPath, S, name);
            if(FileExists(S1)==false)        //檔案不存在的話,就去複製一份過來
            {
                szDir.sprintf("%s%s.ini", DIOCFGPath, name);
                if(FileExists(szDir))
                {
                    S1=szDir;                //AI(W906-Q41) 20260927: golden CopyFile(szDir, S1) 之後讀 S1 ＝ 讀 szDir（預覽不寫檔）
                }
            }
        }
        else
        {
            S1.sprintf("%s%s.ini", DIOCFGPath, name);
        }

        jw.BeginObject();
        jw.Key("name").String(name.c_str());
        // golden LoadData（DIOInterFaceCFG.cpp:72-131）→ 區域副本 T
        TTL_DATA T = TTLCfg;
        if(FileExists(S1)==false)    //Steven 20180620 (Jou) : 加上DIO檔案遺失的保護判斷
        {
            jw.Key("missing").Bool(true);
            jw.Key("lostMsg").Bool(TestIF_File.iTestType==TTL_MODE);   // golden :78-82 ShowMyMessage("Current DIO data has been lossed, please check!!")＋SystemStart=false
            jw.EndObject();
            continue;
        }
        AnsiString asString="";
        S=S1;
        T.iSTLogicMode         =ReadIniData(S, "Start Signal",  "Logic", 0);
        T.iStartType           =ReadIniData(S, "Start Signal",  "Channel", 0);
        T.iOneSTChannel        =ReadIniData(S, "Start Signal",  "SelSignalCH", 0);
        T.iSTPluseWidth        =ReadIniData(S, "Start Signal",  "Pluse Width", 0);
        T.iDutType             =ReadIniData(S, "DUT Signal",    "Type", 0);
        if(T.iDutType<0)   //Steven 20110105
            T.iDutType=0;

        T.iDutBfOnTime         =ReadIniData(S, "DUT Signal",    "Before On", 0);
        T.iDutAfOffTime        =ReadIniData(S, "DUT Signal",    "After Off", 0);

        T.iCateLogicMode       =ReadIniData(S, "Cate Signal",   "Logic", 0);
        if(JCET_FOR_EVAN==1)        //Steven 20220506 : 吳如春要求只用10 bit bit
        {
            T.iCateBitLength       =3;
            T.iCateDataType        =0;
        }
        else
        {
            T.iCateBitLength       =ReadIniData(S, "Cate Signal",   "Channel status", 0);
            T.iCateDataType        =ReadIniData(S, "Cate Signal",   "Data Type", 0);
        }

        asString=ReadIniData(S, "Name",          "Data Type", AnsiString("AAA"));
        strncpy(T.cModeName, asString.c_str(), sizeof(T.cModeName));
        if(T.iCateBitLength==_5BitPE || T.iCateBitLength==_10BitPE)
            T.iCateParity=2;//even
        else if(T.iCateBitLength==_5BitPO || T.iCateBitLength==_10BitPO)
            T.iCateParity=1;//odd
        else
            T.iCateParity=0;//not use
        if(CosFunction.bTTLUseUSec)     //Steven 20180808 (wei) : TTL的時間單位改成microsecond
        {
            T.iSTPluseWidth    =CheckRange((int)T.iSTPluseWidth, 1, 500000);
            T.iDutBfOnTime     =CheckRange(T.iDutBfOnTime, 1, 500000);
            T.iDutAfOffTime    =CheckRange(T.iDutAfOffTime, 1, 500000);
        }
        else
        {
            T.iSTPluseWidth    =CheckRange((int)T.iSTPluseWidth, 10, 500);
            T.iDutBfOnTime     =CheckRange(T.iDutBfOnTime, 10, 500);
            T.iDutAfOffTime    =CheckRange(T.iDutAfOffTime, 10, 500);
        }
        // （golden :130 DoIniDataToForm 不做：TfDIOFrom 的替身）

        // golden ShowTTLState 的清單段（cTesterIF.cpp:1222-1259；912 :1240-1277；產生檔 :1781-1816 機械轉換：lstTTL→Lst、TTLCfg→T）
        TStringList* Lst = new TStringList();
        char cStr[128]="";
        Lst->Add("[Start Signal]");
        Lst->Add("Logic                      : "+TIF_DIOFrom()->rgStartLogic->Items->Strings[T.iSTLogicMode]);
        Lst->Add("Chanel                    : "+TIF_DIOFrom()->rgStartChannel->Items->Strings[T.iStartType]);
        if(CosFunction.bTTLUseUSec)                                                 //Steven 20180808 (wei) : TTL的時間單位改成microsecond
            sprintf(cStr,        "Pulse Width           : %d μs.\n", T.iSTPluseWidth);
        else
            sprintf(cStr,        "Pulse Width           : %d msec.\n", T.iSTPluseWidth);
        Lst->Add(cStr);

        Lst->Add("");

        Lst->Add("[DUT Signal]");
        if(T.iDutType<0)                                                       //Steven 20100105
            T.iDutType=0;
        Lst->Add("Type                       : "+TIF_DIOFrom()->cbSignalType->Items->Strings[T.iDutType]);
        if(CosFunction.bTTLUseUSec)                                                 //Steven 20180808 (wei) : TTL的時間單位改成microsecond
            sprintf(cStr,        "Before ON             : %d μs.", T.iDutBfOnTime);
        else
            sprintf(cStr,        "Before ON             : %d msec.", T.iDutBfOnTime);
        Lst->Add(cStr);
        if(CosFunction.bTTLUseUSec)                                                 //Steven 20180808 (wei) : TTL的時間單位改成microsecond
            sprintf(cStr,        "After OFF              : %d μs.", T.iDutAfOffTime);
        else
            sprintf(cStr,        "After OFF              : %d msec.", T.iDutAfOffTime);
        Lst->Add(cStr);

        Lst->Add("");

        Lst->Add("[END Signal]");
        Lst->Add("Logic                      : "+TIF_DIOFrom()->rgBinLogic->Items->Strings[T.iCateLogicMode]);

        Lst->Add("");

        Lst->Add("Channel & Bit Length  : "+TIF_DIOFrom()->rgBinBitLength->Items->Strings[T.iCateBitLength]);
        Lst->Add("Data Type                    : "+TIF_DIOFrom()->rgBinDataType->Items->Strings[T.iCateDataType]);

        jw.Key("lines").BeginArray();
        for (int i = 0; i < Lst->Count; ++i) jw.String(Lst->Strings[i].c_str());
        jw.EndArray();
        delete Lst;
        // golden ShowTTLState 尾段（cTesterIF.cpp:1261-1278；912 :1279-1296）的非 2／3 卡提示；rgInterfaceType==0 由頁面判斷
        bool ttlWarn = false;
        if(!(TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))
        {
            if(T.iCateBitLength==_8Bit    || T.iCateBitLength==_10Bit ||
               T.iCateBitLength==_10BitPE || T.iCateBitLength==_10BitPO )
            {
                if(TestIF.iTestMode >= QualSite1X4)
                    ttlWarn = true;             // golden ShowMyMessage("TTL 8bit & 10bit only support less than 2 site!", "TTL 2 Site 以上不支援 8bit & 10bit 模式")
            }
        }
        jw.Key("ttlWarn").Bool(ttlWarn);
        jw.EndObject();
    }
    jw.EndArray();
}

// TI-4＋TI-5 的共用本體。golden 順序：FormClose 整支，再主畫面 sbTesterClick 在 ShowModal 之後的尾段。
// fromSave：存檔流程裡（有 editlist session，訊息進 ack）；false＝關頁掛勾（不在任何 WS 指令裡，改 printf）。
bool g_q41CloseTailRan = false;   // AI(W906-Q41) 20260927 Q41 (d)：這一次開→關已經跑過（存檔時跑的）；關頁時看、看完清掉
void TIF_Q41_CloseTailBody(bool fromSave)
{
    // ---- golden TFTestIF::FormClose（906_0625_Steven cTesterIF.cpp:1197-1217；912 :1215-1235）----
    TIF_ReadTestIFFile();
    fShow=false;
    bflag=false;

    if(ioldTestType!=TestIF_File.iTestType ||
       ioldDIOType!=EL<TComboBox>("TFTestIF", "cbDIOType")->ItemIndex)          //Isaac 20200903 :TTL RS232通訊
    {
        if (fTesterSide) fTesterSide->oldLastiTestMode=-1;                      // golden fMain->oldLastiTestMode=-1;（成員在 THandlerTesterSide，HandlerTesterSide.h:65）
        else if (fromSave) filerw::ELTodo("golden cTesterIF.cpp:1206 fMain->oldLastiTestMode=-1 skipped: tester comm not initialised (fTesterSide==NULL)");
        else std::printf("[TesterIF] page closed: golden cTesterIF.cpp:1206 fMain->oldLastiTestMode=-1 skipped (fTesterSide==NULL)\n");
    }

    if(TestIF_File.iTestType==TTL_MODE)                                         //Isaac 20210309 :TTL RS232兩塊板子
    {
        CheckTTLBoardBitMode();
        fMain->CloseGpibProgram(__FUNC__);                                      //20210920 Isaac : 兩塊板子必定帶站號
    }

    // ---- golden TfMain::sbTesterClick 尾段（906_0625_Steven main.cpp:27425-27432；912 :28337-28344）----
    FileRW_TTLCfg_DoReadLastDataLoad();      // S=fDIOFrom->GetDIOFileName(); fDIOFrom->LoadData(S);   //Steven 20180626 (wei) : TTL設定存到工作檔裡面
    DoStructUnitConvert();
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數

    fMain->LoadTestModePicture();            // 移植樹門面是空殼（forms/fMain.cpp:457，圖示歸網頁）；照 golden 呼叫

    bEnterTestIF=true;                                                          //ChungHung 20121221 add
}

// 存檔成功後（SaveFlow :114；Steven S107-1「存檔後就跑」）
void TIF_Q41_CloseTail()
{
    filerw::ELMark("Q41 FormClose + sbTesterClick tail");   // ack.trace
    TIF_Q41_CloseTailBody(true);
    g_q41CloseTailRan = true;
}

}  // namespace

// AI(W906-Q41) 20260927 (St02-E): Q41 (d) —— 關頁掛勾（St01 視窗邊緣 API W906_WindowEdgeRegister 的 onClose；AI(W906-Q41) 20260928 (St02-E helper): registered at :272 since 5b73d905 is in this branch,
//   見 FileRW_TesterIF_Boot）。golden 關表單一定跑一次 FormClose＋呼叫端尾段（存不存檔都跑）；網頁版存檔成功時已經跑過（S107-1），
//   所以一次「開→關」只跑一次：
//     * 存檔跑過（g_q41CloseTailRan）→ 關頁不再跑（不會 ReadTestIFFile／CloseGpibProgram 兩次），清掉旗標；
//     * 沒存檔就關（使用者改了沒存、或只是看）→ 在這裡跑：ReadTestIFFile 把替身還原成檔案值（golden :1217 丟掉沒存的改動）、
//       TTL 模式 CheckTTLBoardBitMode＋CloseGpibProgram、主畫面尾段、bEnterTestIF=true。
//   旗標在「關」清，不在開頁 TIF_FormShow 清：引擎存檔後一定重讀（editlist.get＝FormShow，route-c §5 規則 3），在那裡清會讓存完再關又跑一次。
//   在 wb_serve 主迴圈（500 ms 拍子）上跑、不在任何 WS 指令裡 → 沒有回覆可以附；golden 訊息收進一個臨時 session 再 printf
//   （同 FileRW_TesterIF_BootReadTestIFFile 的做法）。CheckTTLBoardBitMode 的 ShowMyMessage 照 golden 跳網頁對話框。
static void TIF_OnPageClosed()
{
    if (!g_booted) { std::printf("[TesterIF] page closed: TFTestIF proxies not booted -- nothing to do\n"); return; }
    if (g_q41CloseTailRan) {
        g_q41CloseTailRan = false;
        std::printf("[TesterIF] page closed: golden FormClose + sbTesterClick tail already ran at save (Steven S107-1) -- not run again\n");
        return;
    }
    filerw::SessionBegin("");
    TIF_Q41_CloseTailBody(false);
    std::printf("[TesterIF] page closed without save: golden FormClose (906_0625_Steven cTesterIF.cpp:1197-1217; 912 :1215-1235) + sbTesterClick tail "
                "(906_0625_Steven main.cpp:27425-27432; 912 :28337-28344) done; TesterType=%d DIO=\"%s\"; session=%s\n",
                TestIF_File.iTestType, TestIF_File.sDioName.c_str(), filerw::SessionJson().c_str());
    filerw::SessionBegin("");
}
