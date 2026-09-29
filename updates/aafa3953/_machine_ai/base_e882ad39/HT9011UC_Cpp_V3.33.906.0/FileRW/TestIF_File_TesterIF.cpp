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

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

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
        //   Setup.ini (CheckRs232StandardIni) and opens the port.
        TRadioGroup* itf = EL<TRadioGroup>("TFTestIF", "rgInterfaceType");
        const int bl = EL<TRadioGroup>("TFTestIF", "rgBitLength")->ItemIndex;
        const int sb = EL<TRadioGroup>("TFTestIF", "rgStopBit")->ItemIndex;
        if (itf && itf->ItemIndex == RS232_MODE && !W906_Rs232FramingValid(bl, sb)) {
            filerw::ELMessage("refused: RS232 framing not accepted by Windows (5 data bits + 2 stop bits, or 6/7/8 data bits + "
                              "1.5 stop bits) -- nothing written",
                              "拒絕寫入：Windows 不接受這組 RS232 格式（5 bits 配 2 stop，或 6／7／8 bits 配 1.5 stop）—— 未寫檔");
            return;
        }
    }
    TIF_spbSaveClick();
    if (filerw::ELMarked("SaveSetupFile")) {
        // golden 在「關表單」FormClose（:1215）才做的執行期收尾；網頁沒有關表單事件，下次開頁（FormShow :331）ioldTestType 就換成新值了
        // → 存檔當下判斷、照實回報（移植樹 TfMain 門面沒有 oldLastiTestMode／CloseGpibProgram）
        if (ioldTestType != TestIF_File.iTestType || ioldDIOType != Cb("cbDIOType")->ItemIndex)
            filerw::ELTodo("golden cTesterIF.cpp:1221-1225 FormClose: fMain->oldLastiTestMode=-1 (tester type / DIO changed -> main screen "
                           "re-inits the tester link) -- TfMain facade has no oldLastiTestMode; the web has no form-close event");
        if (TestIF_File.iTestType == TTL_MODE)
            filerw::ELTodo("golden cTesterIF.cpp:1227-1231 FormClose (TTL): CheckTTLBoardBitMode() + fMain->CloseGpibProgram(__FUNC__) "
                           "-- not replayed (no form-close event; TfMain facade has no CloseGpibProgram)");
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
    cJSON_Delete(root);
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
                if (filerw::ELEditable("TFTestIF", n)) w.String(n);
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
    w.Key("dioListLost").Bool(bDioListLost);
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

// golden TFTestIF 建構（HT9045.cpp:189 CreateForm，TfSetup :184／TfContact :185 之後、TfDIOFrom :196 之前）：
// DFM 設計期狀態 → 建構子（InitcbDIOType(false)）→ 存檔流程讀的替身 → 容器替身與父子。冪等。
// 前提：DIOCFGPath 已定（LoadMachineConfig）、CUSTOMER_CODE 已定（建構子的客戶碼分支）。
void FileRW_TesterIF_Boot()
{
    if (g_booted) return;
    TIF_DfmItems();
    // AI(W906-GB-P6) 20260926: user ruling 5B (暫照建議，待使用者確認 decision #5): the RS232 options golden's DFM lacks,
    //   APPENDED so recipe indices keep their meaning (TesterComm/Rs232SetupCodes.h has the Setup.ini mapping).
    //   The page (web/page/Setup.TesterIF.html:56) is NOT changed yet (decision #5 Q5 pending).
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
    g_booted = true;
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
namespace { struct TIF_GpibAuxHook { TIF_GpibAuxHook() { W906_GpibAuxRefreshTesterIfPage = &FileRW_TesterIF_DoIniDataToForm; } } g_tifGpibAuxHook; }

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
