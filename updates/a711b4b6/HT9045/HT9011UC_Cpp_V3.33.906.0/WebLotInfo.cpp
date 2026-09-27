// =============================================================================
//  WebLotInfo.cpp -- Data.LotInfo.html 的 WS 指令 lotinfo.op（golden V912 TfLotInfo，uLotInfo.cpp）給網頁用。
//
//  Steven 20260925 (Data.LotInfo 其餘分頁)。NOT in golden as a file：golden 本體在 forms/fLotInfo.cpp 檔尾
//  （TfLotInfo::btClearBarcodeCountClick，逐行翻 V912 uLotInfo.cpp:10168-10184；W906_DoBarcodeCount，逐行翻
//  V912 BarCode/BarCode.cpp:5863-5917）；這裡只是 WS 的 JSON 包裝。放在 wb_serve 的來源清單（同 WebSortCT.cpp）：
//  它要用 WebBridge 的 JsonWriter／cJSON，而 ht9045_forms 不連 ht9045_webbridge。
//
//  分派：tools/wb_serve.cpp 的 lotinfo.op 一臂（Steven 20260925 暫時自己加、整合者確認；寫在原本 IsActionCommand 那一行的前面，不移動行號）；
//    CMakeLists.txt 的 wb_serve 來源清單在 WebSortCT.cpp 同一行加了 WebLotInfo.cpp。範例／形狀見 WebLotInfo.h。
//    網頁（web/page/ht9045_lotinfo_wire.js）送的是 lotinfo.op，value = JSON 字串。
//
//  op
//    "barcode.clearCount"  BarCode 分頁「Clear Count」（btClearBarcodeCount，V912 uLotInfo.dfm:4623）
//        golden 沒有確認框、沒有權限檢查、不看 SystemStart（:10168-10184 整段就是：開目錄 → SGDToXLS 存 .xls → 五組計數器歸零 →
//        DoBarcodeCount 重畫）。本函式照樣不加任何 golden 沒有的判斷，**只多一段確認**，理由：
//          golden :10176 SGDToXLS 會先把「清除前」的格子存成
//            D:\HT9045_Log\2DBarCode\YYYY_MM_DD\YYYY-MM-DD HH_MM_SS.xls
//          移植樹 SGDToXLS 是 no-op（SgdToXLS.cpp GATE (1)：golden 的 BIFF5 寫檔器 XLSFile.pas 是 Object Pascal，沒有移植），
//          所以在這裡按下去，清除前的計數**不會留下任何紀錄**。這是 golden 不會發生的資料遺失，操作員要先知道。
//        {"confirmed":false} → {"executed":false,"needConfirm":true,"prompt":[...],"before":{...},"archive":{"written":false,...}}
//        {"confirmed":true}  → 執行 golden 本體 → {"executed":true,"before":{...},"after":{...},"writes":[...],"archive":{...}}
//    "testerLog.get"       Tester Log 分頁 mmTesterLog 的全部行（唯讀；tag lot.testerLog.tail 只送最後 50 行）
//        → {"executed":true,"count":N,"text":"...\n..."}
//    "selection.get"       Selection 分頁：golden pgLotinfoChange（:7340-7385）讀 config\Security_new.def [Network] → 勾選框狀態
//        （缺鍵時 CheckAndReadIniData 回寫預設值 —— golden 就是這樣）。分頁 golden 看不到（SetSelectionVisible）→ guard "tab-hidden"。
//    "selection.save"      {"values":{"chkTempOffset":bool,...}}：先照 golden 切頁讀一次，再套用「看得見且沒反灰」的勾選，
//        最後 golden btnSaveClick（:7387-7415）寫回 Security_new.def。A75 權限（:1369-1378）反灰 Save → guard "not-authorized"。
//    "barcode.clearList"   BarCode 分頁「Clear List」（btClearBarcodeList，V912 uLotInfo.dfm:4632）—— AI(W906-FRW-S94) 20260926
//        golden btClearBarcodeListClick（:10186-10195）逐行翻在本檔 ClearBarcodeListBody（為什麼不在 forms/fLotInfo.cpp：
//        forms/fLotInfo.h 檔尾的安裝座註解）：iBarcodeReject=0 → 清 2D 重複碼記憶 map2DList 與清單 list2DByLot →
//        [:10191 SendCCDCommand 閘住，記 ELTodo] → RecordProcess → list2DByLot->SaveToFile（LotData.txt 寫成空檔）→
//        btClearBarcodeCount->Click()（＝上面 barcode.clearCount 的 golden 本體）。
//        兩段式確認，理由同 barcode.clearCount（最後一步會清計數，而 .xls 在移植樹不會存）；golden 沒有確認框。
//        {"confirmed":false} → {"executed":false,"needConfirm":true,"prompt":[...],"before":{...}}；{"confirmed":true} → 執行。
//        guard："not-installed"（wb_serve 開機沒呼叫 W906_InstallLotInfoClearListBody）、
//               "tab-hidden"（tsBarCode，FormShow :571／:590）、"button-hidden"（Timer2Timer :7064-7067，只有 CC_KYEC_XILINX）。
//    "ocr.clearList"       OCRBarCode 分頁「Clean List」（spOCRCleanList，V912 uLotInfo.dfm:5721）—— AI(W906-FRW-S94) 20260926
//        golden spOCRCleanListClick（:14461-14466，本體 forms/fLotInfo.cpp 檔尾）：OCRLot.txt 寫成空檔。一段式（golden 沒有確認框；
//        OCRLot.txt 在 golden 全樹只有寫者 OCRInsp.cpp:1135，沒有讀者）。Data.LotInfo.html 目前沒有這顆鈕（tsOCRBarCode 客戶專屬，頁面跳過）。
//        guard："tab-hidden"（tsOCRBarCode，Timer2Timer :7150）、"button-hidden"（:7285，要 IniConfig.bCompareOCRData）。
//
//    "lotEnd.state"        Lot 分頁 Lot End 鈕的狀態（唯讀）—— AI(W906-PROD-S117) 20260926
//        → {"executed":true,"tabVisible","panelVisible","buttonVisible","buttonEnabled","caption","systemStart",
//           "precheck":""|"system-running"|"must-clean-out-1"|"must-clean-out-2"|"oee-not-started","lot":{...}}
//        可見度照 golden 重算（dfm → FormShow :367-371／:407-412，forms/fLotInfo.cpp W906_LotEndPanelVisible）；precheck 只是預覽。
//    "lotEnd"              Lot 分頁「Lot End」（sbSECSLotEnd，V912 uLotInfo.dfm:341）—— AI(W906-PROD-S117) 20260926
//        golden sbSECSLotEndClick（:1387-1444）→ SetLotEnd（:2014-2379），本體在 forms/fLotInfo.cpp 檔尾。兩段式（理由見該分支的註解）：
//        {"confirmed":false} → {"executed":false,"needConfirm":true,"prompt":[...],"before":{...},"writes":[...]}；
//        {"confirmed":true}  → 執行 → {"executed":true|false,"refused"?,"before","after","writes","gated":[...],"session"}。
//        gated＝這一次被閘住、而且 golden 此刻會執行的敘述（同時轉成 ELTodo）。guard：tab-hidden／panel-hidden／button-hidden／button-disabled。
//
//  寫入
//    selection.get／selection.save：D:\HT9045\config\Security_new.def [Network]（save 寫 16 個鍵，SCK ART 多 "Auto Retest"、
//      VTEST 多 2 個；get 只在缺鍵時補寫）
//  寫入（barcode.clearCount confirmed=true）
//    D:\HT9045_Log\2DBarCode\YYYY_MM_DD\  目錄（golden :10174 MyForceDirectories；common.cpp 真本體）
//    （.xls 本身不會產生，見上）
//    記憶體：iNeedBarcodeCount／iBarcodeDuplicate／iBarcodeErrorCount／iBarcodePassCount／iBarcodeAutoRetry（cmydef.cpp:4237-4241）歸零、
//            fLotInfo->sgBarcode 重畫。這些計數器沒有落地檔（golden 也沒有），開機時 cmydef.cpp:6031 歸零。
//  寫入（barcode.clearList confirmed=true）AI(W906-FRW-S94) 20260926
//    D:\HT9045_Log\2DBarCode\LotData.txt（asBarCodeLot，common.cpp:157）—— 整檔寫成空的（0 byte；資料夾不在時 vclcompat 靜靜不寫，
//      golden 會丟 EFCreateError）。接著是 barcode.clearCount 的全部寫入（上面）。
//    記憶體：iBarcodeReject（cmydef.cpp:4503）＝0、map2DList／list2DByLot 清空 —— 之後同一個 2D 碼不再判成重複（golden 同，
//      換批時清就是這顆鈕的用途；V912 BarCode/BarCode.cpp:823／:830 golden 註解「這部分會影響分Bin」）。
//  寫入（ocr.clearList）D:\HT9045_Log\OCR\OCRLot.txt（asOCRLotPath，common.cpp:331）—— 整檔寫成空的。
//  寫入（lotEnd confirmed=true，golden 本體執行時）AI(W906-PROD-S117) 20260926
//    D:\HT9045_Log\LotInfo\<年>\LotInfo_<年>.csv 加一行（golden :2038-2039；新檔先寫表頭）
//    AuthPath+"config.ini" [Lot Info]：SetLotID("")（Lot ID=""、LotStartTime=現在、Customer Lot ID）＋ ReadWriteLotInfo(false)（全部清空；EventLogFile 保留）
//    D:\HT9045\system\ArmByLot0..2.dat／_backup.dat（清 0 後寫；W906_MACHINERECORD_DIR 有設時改到那裡；bLowYieldAlarmByBin 時另寫 .ini）
//    記憶體：RunInfo.bLotStart=false（之後 START 會被擋，要重新 lot.start）、RunInfo.LotNo=""、LotEndTime／dtEndLot=現在、Lot 分頁元件清空。
//  不做的鈕：Change File（btChangeFileClick :10394-10418 重連 CCD／換 CCD job）。
//    Clear List 以前也列在這裡（理由：會送 CCD 指令 "2DID by lot list"）。S94 重查 golden：:10191 那一行 SendCCDCommand 的 Msg2 是
//    空字串，而 golden BarCode.cpp:5327-5546 每一個 SendText 都在 `if(Msg2=="") {} else {…}` 的 else 裡 ⇒ 實際上**不送任何東西
//    給 CCD**，只寫一行 "Action, 2DID by lot list" 到 CCD 通訊 log（AddCCDCommunicationLog :5925：memoCCDCommLog＋
//    asBarCodeCommLogPath\YYYY_MM_DD\YYYY_MM_DD_HH.txt）。移植樹仍照 Steven 的指示把那一行閘住、記 ELTodo（交件報告列給 Jimmy）。
// =============================================================================
#include <cstdio>
#include <string>

#include "WebLotInfo.h"
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fLotInfo.h"
#include "cmydef.h"        // SystemYear…SystemSec、iNeedBarcodeCount 等
#include "cpublic.h"       // GetTimeInfo()
#include "common.h"        // AuthPath（Security_new.def 的資料夾）；asBarCodeLot／asOCRLotPath（AI(W906-FRW-S94) 20260926）
#include "cMyDB.h"         // RecordProcess（本體 canary_support.cpp:117，印 stdout）                          AI(W906-FRW-S94) 20260926
#include "vclcompat/SysUtils.h"             // FileExists                                                     AI(W906-FRW-S94) 20260926
#include "BarCode/BarCode_Shuttle2_Scan.h"  // map2DList（本體 BarCode/BarCode_Shuttle2_Scan.cpp:48，ht9045_sm） AI(W906-FRW-S94) 20260926
#include "cprod.h"          // RunInfo（bLotStart／LotNo／LotStartTime／LotEndTime）、TestIF_File.bLowYieldAlarmByBin      AI(W906-PROD-S117) 20260926
#include "Config.h"         // AI(W906-PROD-S117) 20260927: IniConfig.bN09_LotCountAutoFunc／SocketHandlerID（Q37 writes 清單）
#include "CosFunction.h"    // AI(W906-PROD-S117) 20260927: CosFunction.bSortingBy2DList／bART_SECSGEM_93K（Q37）
#include "LastSet.h"        // AI(W906-PROD-S117) 20260927: LastSet.iTester（Q37）
#include <cstdlib>          // getenv（Q37）
AnsiString W906_MachineRecordRedirect(const AnsiString& goldenPath);       // cinitial.cpp:9470（cSocket.cpp:739 TArm::WriteFile 用同一支）AI(W906-PROD-S117) 20260926

namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp:149-150
// AI(W906-FRW-S94) 20260926：FileRW/_EditList.h:139-148 的三個宣告（本體 FileRW/_EditList.cpp，只在 wb_serve）。不 include 那個標頭：它帶 HTEditList.h。
namespace filerw { void SessionBegin(const std::string& answersJson); std::string SessionJson(); void ELTodo(const char* what); }
// AI(W906-FRW-S94) 20260926：golden fBarCode->list2DByLot 在移植樹是全域（BarCode/BarCode_Bottom2DID.h:144，本體 BarCode_Bottom2DID.cpp:60）。
//   不 include 那個標頭：它經 BarCode_Shuttle1_Scan.h 帶進 canary_support.h 與 aHotPlateSubstrate.h（陷阱 #3）。型別照原宣告一字不改。
extern TStringList *list2DByLot;

namespace {

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

// golden FormShow :642-647 的列名／:636-641 的欄名（sgBarcode 的表頭，Cells[0][r]／Cells[c][0]）
const char* const kRow[7] = { "Shuttle", "Load", "Pass", "Fail", "Rate(%)", "Retry", "Duplicate" };
const char* const kCol[6] = { "Shuttle", "1_A", "1_B", "2_A", "2_B", "Total" };

void WriteGrid(webbridge::JsonWriter& w)
{
    w.BeginObject();
    for (int r = 1; r <= 6; ++r) {
        w.Key(kRow[r]).BeginObject();
        for (int c = 1; c <= 5; ++c)
            w.Key(kCol[c]).String(std::string(fLotInfo->sgBarcode->Cells[c][r].c_str()));
        w.EndObject();
    }
    w.EndObject();
}

// golden :10173／:10175 的兩個 sprintf（同一拍的時鐘）
void ArchivePath(AnsiString* folder, AnsiString* file)
{
    folder->sprintf("D:\\HT9045_Log\\2DBarCode\\%04d_%02d_%02d\\", SystemYear, SystemMonth, SystemDate);
    file->sprintf("%04d-%02d-%02d %02d_%02d_%02d.xls", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
}

void WriteArchive(webbridge::JsonWriter& w, const AnsiString& folder, const AnsiString& file)
{
    w.Key("archive").BeginObject();
    w.Key("path").String(std::string((folder + file).c_str()));
    w.Key("written").Bool(false);
    w.Key("reason").String("golden uLotInfo.cpp:10176 SGDToXLS 在移植樹是 no-op（SgdToXLS.cpp GATE (1)：XLSFile.pas 沒有移植）"
                           " —— 清除前的計數不會存檔");
    w.EndObject();
}

// Selection 分頁的勾選框（V912 uLotInfo.dfm:4369-4553）與 golden 讀／存用的鍵（pgLotinfoChange :7340-7385／btnSaveClick :7387-7415）
struct SelBox { const char* name; TCheckBox* TfLotInfo::* member; const char* readKey; const char* saveKey; };
const SelBox kSel[] = {
    { "chkTempOffset",             &TfLotInfo::chkTempOffset,             "Temp Offset",               "Temp Offset" },
    { "chkContactHigh",            &TfLotInfo::chkContactHigh,            "Contact High",              "Contact High" },
    { "chkContactForce",           &TfLotInfo::chkContactForce,           "Contact Force",             "Contact Force" },
    { "chkContactMode",            &TfLotInfo::chkContactMode,            "Contact Mode",              "Contact Mode" },
    { "chkHotPlate",               &TfLotInfo::chkHotPlate,               "HotPlate",                  "HotPlate" },
    { "chkLoadUnload",             &TfLotInfo::chkLoadUnload,             "Load Unload",               "Load Unload" },
    { "chkSpeedSetting",           &TfLotInfo::chkSpeedSetting,           "Speed Setting",             "Speed Setting" },
    { "chkShuttleMode",            &TfLotInfo::chkShuttleMode,            "Shuttle Mode",              "Shuttle Mode" },
    { "chkTestMode",               &TfLotInfo::chkTestMode,               "Test Mode",                 "Test Mode" },
    { "chkBinasgn",                &TfLotInfo::chkBinasgn,                "Binasgn",                   "Binasgn" },
    { "chkBinasgnOff",             &TfLotInfo::chkBinasgnOff,             "BinasgnOff",                "BinasgnOff" },
    { "checkbAutoClean",           &TfLotInfo::checkbAutoClean,           "Auto Clean",                "Auto Clean" },
    { "chkIndexHeatingMode",       &TfLotInfo::chkIndexHeatingMode,       "Index Heat Mode",           "Heat Mode" },     // golden 讀寫不同鍵（:7371／:7407），照留
    { "chkART",                    &TfLotInfo::chkART,                    "Auto Retest",               "Auto Retest" },   // 只在 CosFunction.bUseSCKART
    { "chkART_RTCount",            &TfLotInfo::chkART_RTCount,            "ART_RT_Count ",             "" },              // golden 只讀不存（:7369）
    { "cbBottom2DOffset",          &TfLotInfo::cbBottom2DOffset,          "Bottom 2D Offset",          "Bottom 2D Offset" },
    { "chkCleanCount",             &TfLotInfo::chkCleanCount,             "Cleaning Count",            "Cleaning Count" },
    { "chkAutoCleanContactHeight", &TfLotInfo::chkAutoCleanContactHeight, "Auto Clean Contact Height", "Auto Clean Contact Height" },
    { "chkStopYield",              &TfLotInfo::chkStopYield,              "",                          "Stop Yield" },    // grpMesCheck，只在 bVTESTFunction 存
    { "chkConsecutiveFailure",     &TfLotInfo::chkConsecutiveFailure,     "",                          "Consecutive Failure" },
};
const std::size_t kSelCount = sizeof(kSel) / sizeof(kSel[0]);

void WriteSelection(webbridge::JsonWriter& w)
{
    TfLotInfo* f = fLotInfo;
    w.Key("file").String(std::string((AuthPath + "Security_new.def").c_str()));
    w.Key("section").String("Network");
    w.Key("tabVisible").Bool(f->tsSelection->TabVisible);
    w.Key("groupVisible").Bool(f->groupbDownloadItem->Visible);
    w.Key("mesCheckVisible").Bool(f->grpMesCheck->Visible);
    w.Key("accessWarningVisible").Bool(f->lblDownloadAccessWarning->Visible);
    w.Key("saveEnabled").Bool(f->btnSave->Enabled);
    w.Key("boxes").BeginObject();
    for (std::size_t i = 0; i < kSelCount; ++i) {
        TCheckBox* b = f->*(kSel[i].member);
        w.Key(kSel[i].name).BeginObject();
        w.Key("checked").Bool(b->Checked);
        w.Key("visible").Bool(b->Visible);
        w.Key("enabled").Bool(b->Enabled);
        w.Key("readKey").String(kSel[i].readKey);
        w.Key("saveKey").String(kSel[i].saveKey);
        w.EndObject();
    }
    w.EndObject();
}

// golden：看不到的分頁切不過去，pgLotinfoChange 不會讀、Save 按不到
std::string SelectionGuard(const std::string& op, const char* guard, const char* golden, const char* detail)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("op").String(op);
    w.Key("guard").String(guard);
    w.Key("goldenLine").String(golden);
    w.Key("detail").String(detail);
    w.EndObject();
    return w.Str();
}

std::string BadPayload(const char* detail)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("guard").String("bad-payload");
    w.Key("detail").String(detail);
    w.EndObject();
    return w.Str();
}

// =============================================================================
//  AI(W906-FRW-S94) 20260926（Steven 團隊）：golden V912 uLotInfo.cpp:10186-10195 TfLotInfo::btClearBarcodeListClick —— 逐行照翻。
//  呼叫路徑：TfLotInfo::btClearBarcodeListClick()（forms/fLotInfo.cpp 檔尾）→ W906_ClearBarcodeListBody → 這裡
//    （W906_InstallLotInfoClearListBody 裝上；為什麼本體在這裡而不在 forms/fLotInfo.cpp：forms/fLotInfo.h 檔尾的安裝座註解）。
//  ⓐ golden fBarCode->map2DList／fBarCode->list2DByLot → 兩個全域（上面 include 的註解；FileRW/MainBoot.cpp W906_FRWBoot_LotListsRead 同）。
//    BarCode/BarCode_Shuttle1_CCDScan.cpp:211 另一份 TU 內的 map2DList 碰不到（該檔 SHARED-STATE CAVEAT，既有差異）。
//  ⓑ :10191 SendCCDCommand 閘住（ELTodo）：見檔頭「不做的鈕」—— golden 這一行只寫 CCD 通訊 log、不送 CCD。
//  ⓒ :10194 btClearBarcodeCount->Click() → 直接呼叫本體 btClearBarcodeCountClick()（vclcompat TControl::Click() 是 no-op）。
//  自己持 FormLock（CRITICAL_SECTION，同執行緒可重入）：GPIB／SECS 路徑（wb_serve 主迴圈）來呼叫時，也跟 HTTP 執行緒讀表單互斥；
//    lotinfo.op 外層已經持著，再進一次不會死結。
// =============================================================================
void ClearBarcodeListBody(TfLotInfo* self)
{
    FormLockGuard lock;
    iBarcodeReject=0;                                                           //JerryYang 20200520 清除2DID累加fail數量
    map2DList.clear();                                                          // golden fBarCode->map2DList.clear();（ⓐ）
    list2DByLot->Clear();                                                       //Steven 20160429 : 清空2D List, 要存到檔案裡
#if 0 // AI(W906-FRW-S94) 20260926: GATE（Steven 指示：這一行閘住、記 ELTodo、列給 Jimmy）。golden :10191。Msg2 是空字串 ⇒ golden 不送 CCD，
      //   只寫 CCD 通訊 log（BarCode.cpp:5327 起；:5925 AddCCDCommunicationLog）。移植樹真本體 ::SendCCDCommand（BarCode/BarCode_8CCD_Glue.cpp:178）
      //   的 log 改走 RecordProcess（stdout），不是 golden 的 CCD 通訊 log 檔 —— 打開前先決定要不要那一行。
    fBarCode->SendCCDCommand(fBarCode->iBarCode1_1, "2DID by lot list");        //Steven 20160926 : 補上清除資料的紀錄
#endif
    filerw::ELTodo("golden uLotInfo.cpp:10191 fBarCode->SendCCDCommand(iBarCode1_1, \"2DID by lot list\") not run (gated) -- "
                   "Msg2 is empty, so golden sends nothing to the CCD; it only appends \"Action, 2DID by lot list\" to the CCD "
                   "communication log (BarCode.cpp:5925 AddCCDCommunicationLog) -- Jimmy");
    std::printf("btClearBarcodeListClick: golden :10191 SendCCDCommand(\"2DID by lot list\") GATED (CCD comm-log line only; ELTodo)\n");
    RecordProcess("Clear barcode list.");                                       //Alick 20170202(jou) add 紀錄按下claer button
    list2DByLot->SaveToFile(asBarCodeLot);
    self->btClearBarcodeCountClick();                                           // golden btClearBarcodeCount->Click();（ⓒ）
}

// barcode.clearList 回應裡的清單狀態（網頁只顯示，不判斷）
void WriteLotListState(webbridge::JsonWriter& w, const char* key)
{
    w.Key(key).BeginObject();
    w.Key("file").String(std::string(asBarCodeLot.c_str()));
    w.Key("fileExists").Bool(FileExists(asBarCodeLot));
    w.Key("list2DByLot").Number((wb_int64)list2DByLot->Count);
    w.Key("map2DList").Number((wb_int64)map2DList.size());
    w.Key("iBarcodeReject").Number((wb_int64)iBarcodeReject);
    w.Key("grid"); WriteGrid(w);
    w.EndObject();
}

// AI(W906-PROD-S117) 20260926：Lot End（lotEnd.state／lotEnd）回應裡的工單狀態。網頁只顯示，不判斷。
void WriteLotSnapshot(webbridge::JsonWriter& w, const char* key)
{
    TfLotInfo* f = fLotInfo;
    w.Key(key).BeginObject();
    w.Key("lotStart").Bool(RunInfo.bLotStart != 0);                             // SetLotComponents :2383（golden 的「開批中」旗標）
    w.Key("lotNo").String(std::string(RunInfo.LotNo.c_str()));
    w.Key("lotStartTime").String(std::string(RunInfo.LotStartTime.c_str()));
    w.Key("lotEndTime").String(std::string(RunInfo.LotEndTime.c_str()));
    w.Key("lotId").String(std::string(f->edtSysLotID->Text.c_str()));
    w.Key("operator").String(std::string(f->edtSysOperatorID->Text.c_str()));
    w.Key("runMode").String(std::string(f->cbRunMode->Text.c_str()));
    w.Key("startTime").String(std::string(f->lbledtStarTime->Text.c_str()));
    w.Key("endTime").String(std::string(f->lbledtEndTime->Text.c_str()));
    w.Key("device").String(std::string(f->lbledtDeviceName->Text.c_str()));
    w.Key("lotStartDown").Bool(f->sbSECSLotStart->Down);
    w.Key("lotEndDown").Bool(f->sbSECSLotEnd->Down);
    w.EndObject();
}

// golden sbSECSLotEndClick 停下來的地方 → 給人看的一句話（W906_LotEndResult／W906_LotEndPrecheck 的值）
const char* LotEndReason(const AnsiString& r)
{
    if (r == "system-running")   return "機台運轉中（SystemStart）：golden :1393-1397 直接 return，不跳訊息";
    if (r == "must-clean-out-1") return "機台內還有 IC（CheckCanChangeRealDummy()==false）：golden :1401-1407 跳 MES1646「Must finish [Clean out]!!」";
    if (r == "must-clean-out-2") return "iTestHeadMotorTask!=1 且 fAllMotorHome：golden :1409-1415 跳 MES1646「Must finish [Clean out]!!」";
    if (r == "oee-not-started")  return "OEE 機台還沒 OEE Lot Start：golden :1420-1424 跳「Please Lot Start!」";
    if (r == "executed")         return "已執行 SetLotEnd";
    return "";
}

// Lot End 會寫的真實檔（golden 路徑；執行後第一項換成實際寫入的檔名）
void WriteLotEndWrites(webbridge::JsonWriter& w, const AnsiString& lotInfoLog)
{
    w.Key("writes").BeginArray();
    w.String(std::string(lotInfoLog.c_str()) + "（golden :2038-2039 Lot Info log 加一行 16 欄；新檔先寫表頭）");
    w.String(std::string((AuthPath + "config.ini").c_str()) + " [Lot Info]（:2345 SetLotID(\"\")：Lot ID=\"\"、LotStartTime=現在、Customer Lot ID；"
             ":2376 ReadWriteLotInfo(false)：Start Time／End Time／Operator／Lot No／Run Mode… 全部寫成空字串，EventLogFile 保留原值）");
    for (int i = 0; i < 3; ++i) {
        const AnsiString n = "D:\\HT9045\\system\\ArmByLot" + AnsiString(i);
        w.String(std::string(W906_MachineRecordRedirect(n + ".dat").c_str()) + "、" +
                 std::string(W906_MachineRecordRedirect(n + "_backup.dat").c_str()) +
                 "（:2373 ArmDataLot[i]->ClearALLCT() 清成 0 → :2376 ArmDataLot[i]->WriteFile()）");
        if (TestIF_File.bLowYieldAlarmByBin)
            w.String(std::string(W906_MachineRecordRedirect(n + ".ini").c_str()) + "（TestIF_File.bLowYieldAlarmByBin：By Bin 良率計數）");
    }
    // AI(W906-PROD-S117) 20260927 (Steven 團隊)：todo ★ Q37＝A（Steven 20260927「A, 根據golden的方式處理」，RULINGS_20260926 S155）——
    //   Lot End 的測試總表（G-030，forms/fLotInfo.cpp SetLotEnd → W906_SckArt_SaveTestSummary(1)）也列進 writes。條件照 golden
    //   uLotInfo.cpp:2258-2261 → SCK_ART.cpp:1620-1646：[N09] 沒開、不是 2D 排序、不是 93K SECS ART；Summary_Lot 那一份只在非 TCP/IP 測試
    //   （golden :1639-1645 SaveSummaryTrayFeed 只在一般分支）。檔名裡的時間在真的寫檔那一刻才決定，這裡用樣式字表示。
    if (IniConfig.bN09_LotCountAutoFunc == false &&
        !(CosFunction.bSortingBy2DList && LastSet.iTester == _2D_SORT && TestIF_File.bSortingBy2DIDList) &&
        !CosFunction.bART_SECSGEM_93K) {                                                         // golden uLotInfo.cpp:2258-2261 -> SCK_ART.cpp:1620-1646
        if (TestIF_File.iTestType != TCP_IP_MODE) {                                              // SaveSummaryTrayFeed only on the general branch (golden :1639-1645)
            const char* sumRoot = getenv("W906_SUMMARYLOT_ROOT");
            AnsiString a;
            a.sprintf("%s\\%04d%02d\\%s YYYYMMDD-hhnnss %s Summary.txt", sumRoot ? sumRoot : "D:\\HT9045_Log\\Summary_Lot",
                      SystemYear, SystemMonth, IniConfig.SocketHandlerID.c_str(), RunInfo.LotNo.c_str());
            w.String(std::string(a.c_str()) + "（golden SCK_ART.cpp:3130-3400 SaveSummaryTrayFeed：Site／Bin 計數表）");
        }
        AnsiString b;
        b.sprintf("%s\\%04d\\%02d\\<LotID>_<Process>_FT_ 或 _RT<n>_YYYYMMDDhhnn.txt", asSummaryPath.c_str(), SystemYear, SystemMonth);
        w.String(std::string(b.c_str()) + "（golden SCK_ART.cpp:2806-3128 SaveTestSummaryTSV：Hard Bin Summary）");
    }
    w.EndArray();
}

}  // namespace

// AI(W906-FRW-S94) 20260926：安裝座（宣告 WebLotInfo.h；指標在 forms/fLotInfo.h 檔尾）。wb_serve 開機呼叫一次。
void W906_InstallLotInfoClearListBody()
{
    W906_ClearBarcodeListBody = &ClearBarcodeListBody;
    std::printf("TfLotInfo::btClearBarcodeListClick body installed (WebLotInfo.cpp; golden uLotInfo.cpp:10186-10195)\n");
}

bool W906_LotInfoClearListBodyInstalled()
{
    return W906_ClearBarcodeListBody == &ClearBarcodeListBody;
}

std::string W906_LotInfoOp(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;

    std::string op;
    bool confirmed = false;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0)
            return BadPayload("value 不是合法 JSON（要 {\"op\":\"barcode.clearCount\",\"confirmed\":bool} 或 {\"op\":\"testerLog.get\"}）");
        const cJSON* jo = cJSON_GetObjectItemCaseSensitive(root, "op");
        if (jo && cJSON_IsString(jo) && jo->valuestring) op = jo->valuestring;
        const cJSON* jc = cJSON_GetObjectItemCaseSensitive(root, "confirmed");
        confirmed = (jc && cJSON_IsBool(jc) && cJSON_IsTrue(jc));             // 明確 true 才算確認過
        cJSON_Delete(root);
    }
    if (fLotInfo == 0)
        return BadPayload("fLotInfo 是 NULL");

    webbridge::JsonWriter w;
    FormLockGuard lock;

    if (op == "testerLog.get") {
        int count = -1;
        const AnsiString text = fLotInfo->W906_TesterLogTail(0x7fffffff, &count);
        w.BeginObject();
        w.Key("executed").Bool(true);
        w.Key("op").String(op);
        w.Key("count").Number((wb_int64)count);
        w.Key("text").String(std::string(text.c_str()));
        w.Key("goldenLine").String("V912 uLotInfo.dfm:11276 mmTesterLog（Interface/TesterTCP_Socket.cpp:394／:399 寫入）");
        w.EndObject();
        if (ok) *ok = true;
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    if (op == "selection.get" || op == "selection.save") {
        // 先把 Selection 分頁元件回到 golden 開窗後的狀態（dfm → FormShow :631 → SetSelectionVisible，含 A75 權限）
        fLotInfo->W906_RefreshTabVisible();
        fLotInfo->W906_SelectionDfmDefaults();
        if (!fLotInfo->tsSelection->TabVisible)
            return SelectionGuard(op, "tab-hidden", "V912 uLotInfo.cpp:1303-1383 SetSelectionVisible",
                                  "golden 這台機台看不到 Selection 分頁（bEnableRms／bEnableErms／bEnableFTP／bSPILFunction 都沒開，或 KYEC），"
                                  "切不過去就不會讀、Save 也按不到");
        // golden pgLotinfoChange（切到 tsSelection 時，:7340-7385；本體 forms/fLotInfo.cpp:2052）：CheckAndReadIniData 讀 Security_new.def
        fLotInfo->pgLotinfo->ActivePage = fLotInfo->tsSelection;
        fLotInfo->pgLotinfoChange();

        std::string ignored;
        if (op == "selection.save") {
            if (!fLotInfo->btnSave->Enabled)
                return SelectionGuard(op, "not-authorized", "V912 uLotInfo.cpp:1369-1378",
                                      "IniConfig.bA75DownloadItemByAccessLevel 且 AccessLevel==0（OP）：golden 把 Download 群組（含 Save）整個反灰");
            cJSON* root = cJSON_Parse(payloadJson.c_str());
            const cJSON* jv = root ? cJSON_GetObjectItemCaseSensitive(root, "values") : 0;
            if (jv == 0 || !cJSON_IsObject(jv)) {
                if (root) cJSON_Delete(root);
                return BadPayload("selection.save 要 {\"op\":\"selection.save\",\"values\":{\"chkTempOffset\":bool,...}}");
            }
            for (const cJSON* it = jv->child; it; it = it->next) {
                const SelBox* sb = 0;
                for (std::size_t i = 0; i < kSelCount; ++i)
                    if (it->string && std::string(it->string) == kSel[i].name) { sb = &kSel[i]; break; }
                TCheckBox* b = sb ? fLotInfo->*(sb->member) : 0;
                // golden 操作員只按得到看得見、沒反灰的勾選框；其他的照 golden 維持剛讀進來的值
                if (b == 0 || !cJSON_IsBool(it) || !b->Visible || !b->Enabled) {
                    if (!ignored.empty()) ignored += ",";
                    ignored += it->string ? it->string : "?";
                    continue;
                }
                b->Checked = cJSON_IsTrue(it) ? true : false;
            }
            cJSON_Delete(root);
            fLotInfo->btnSaveClick();                                           // golden 本體（forms/fLotInfo.cpp 檔尾）
            std::printf("lotinfo.op selection.save -> %sSecurity_new.def [Network] written (ignored: %s)\n",
                        AuthPath.c_str(), ignored.empty() ? "-" : ignored.c_str());
        }
        w.BeginObject();
        w.Key("executed").Bool(true);
        w.Key("op").String(op);
        WriteSelection(w);
        if (op == "selection.save") {
            w.Key("ignored").String(ignored);
            w.Key("goldenLine").String("V912 uLotInfo.cpp:7340-7385 pgLotinfoChange（讀）→ 套用勾選 → :7387-7415 btnSaveClick（寫）");
        } else {
            w.Key("goldenLine").String("V912 uLotInfo.cpp:7340-7385 pgLotinfoChange（缺鍵時 CheckAndReadIniData 會回寫預設值）");
        }
        w.EndObject();
        if (ok) *ok = true;
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    // AI(W906-FRW-S94) 20260926：BarCode 分頁 Clear List（golden btClearBarcodeListClick :10186-10195）。兩段式，理由見檔頭。
    if (op == "barcode.clearList") {
        if (!W906_LotInfoClearListBodyInstalled())
            return SelectionGuard(op, "not-installed", "V912 uLotInfo.cpp:10186-10195",
                                  "wb_serve 開機沒有呼叫 W906_InstallLotInfoClearListBody()（tools/wb_serve.cpp 的片段還沒套）—— 什麼都沒清");
        fLotInfo->W906_RefreshTabVisible();
        if (!fLotInfo->tsBarCode->TabVisible)
            return SelectionGuard(op, "tab-hidden", "V912 uLotInfo.cpp:571／:590 tsBarCode->TabVisible",
                                  "golden 這台機台看不到 BarCode 分頁（BAR_CODE_INSTALL 不是 CCD／InShtIntel／EtherNetCCD），按不到 Clear List");
        fLotInfo->W906_ClearListButtonsVisible();
        if (!fLotInfo->btClearBarcodeList->Visible)
            return SelectionGuard(op, "button-hidden", "V912 uLotInfo.cpp:7064-7067",
                                  "CC_KYEC_XILINX 且 AccessLevel 低於 iDefHonPrecLevel：golden 把 Clear List 藏起來");

        GetTimeInfo();                                                          // 同 barcode.clearCount：:10194 會跑 btClearBarcodeCountClick
        W906_DoBarcodeCount();
        AnsiString folder, file;
        ArchivePath(&folder, &file);

        w.BeginObject();
        w.Key("op").String(op);
        if (!confirmed) {
            w.Key("executed").Bool(false);
            w.Key("needConfirm").Bool(true);
            w.Key("prompt").BeginArray();
            w.String("Clear barcode list? LotData.txt (2D duplicate-code list of this lot) is emptied, then Clear Count runs "
                     "(no .xls backup in this build: SGDToXLS is not ported)");
            w.String("確定要清除 2D 重複碼清單？LotData.txt 會清空（之後同一個 2D 碼不再判成重複），接著清除 Barcode 計數"
                     "（這個版本不會先存 .xls：SGDToXLS 未移植）");
            w.EndArray();
            WriteLotListState(w, "before");
            WriteArchive(w, folder, file);
            w.Key("goldenLine").String("V912 uLotInfo.cpp:10186-10195（golden 沒有確認框 —— 同 barcode.clearCount：最後一步 :10194 會清計數而 SGDToXLS 缺件）");
            w.EndObject();
            if (ok) *ok = true;
            return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
        }

        filerw::SessionBegin("");                                               // 收 ClearBarcodeListBody 的 ELTodo
        WriteLotListState(w, "before");
        fLotInfo->btClearBarcodeListClick();                                    // → W906_ClearBarcodeListBody（本檔 ClearBarcodeListBody）
        w.Key("executed").Bool(true);
        w.Key("confirmed").Bool(true);
        WriteLotListState(w, "after");
        w.Key("writes").BeginArray();
        w.String(std::string(asBarCodeLot.c_str()) + "（整檔寫成空的；golden :10193 list2DByLot->SaveToFile）");
        w.String(std::string(folder.c_str()) + "（目錄；:10194 → btClearBarcodeCountClick :10174 MyForceDirectories）");
        w.EndArray();
        WriteArchive(w, folder, file);
        w.Key("gated").BeginArray();
        w.String("V912 uLotInfo.cpp:10191 fBarCode->SendCCDCommand(fBarCode->iBarCode1_1, \"2DID by lot list\")"
                 " —— 閘住（golden 這一行只寫 CCD 通訊 log，Msg2 空字串不送 CCD）");
        w.EndArray();
        w.Key("session").RawValue(filerw::SessionJson());
        w.Key("goldenLine").String("V912 uLotInfo.cpp:10186-10195");
        w.EndObject();
        if (ok) *ok = true;
        std::printf("lotinfo.op barcode.clearList -> executed (%s emptied; .xls NOT written: SGDToXLS no-op)\n", asBarCodeLot.c_str());
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    // AI(W906-FRW-S94) 20260926：OCRBarCode 分頁 Clean List（golden spOCRCleanListClick :14461-14466）。一段式（golden 沒有確認框）。
    if (op == "ocr.clearList") {
        fLotInfo->W906_RefreshTabVisible();
        if (!fLotInfo->tsOCRBarCode->TabVisible)
            return SelectionGuard(op, "tab-hidden", "V912 uLotInfo.cpp:7150 tsOCRBarCode->TabVisible",
                                  "golden 這台機台看不到 OCRBarCode 分頁（INSTALL_OCR 沒裝或 CosFunction.bTrayOCR 沒開），按不到 Clean List");
        fLotInfo->W906_ClearListButtonsVisible();
        if (!fLotInfo->spOCRCleanList->Visible)
            return SelectionGuard(op, "button-hidden", "V912 uLotInfo.cpp:7285",
                                  "golden 只在 INSTALL_OCR 且 CosFunction.bTrayOCR 且 IniConfig.bCompareOCRData 時顯示 Clean List");
        const bool existed = FileExists(asOCRLotPath);
        int linesBefore = -1;
        if (existed) { TStringList cur; cur.LoadFromFile(asOCRLotPath); linesBefore = cur.Count; }
        fLotInfo->spOCRCleanListClick();                                        // golden 本體（forms/fLotInfo.cpp 檔尾）
        w.BeginObject();
        w.Key("executed").Bool(true);
        w.Key("op").String(op);
        w.Key("file").String(std::string(asOCRLotPath.c_str()));
        w.Key("existedBefore").Bool(existed);
        w.Key("linesBefore").Number((wb_int64)linesBefore);
        w.Key("fileExistsAfter").Bool(FileExists(asOCRLotPath));
        w.Key("goldenLine").String("V912 uLotInfo.cpp:14461-14466");
        w.EndObject();
        if (ok) *ok = true;
        std::printf("lotinfo.op ocr.clearList -> executed (%s emptied)\n", asOCRLotPath.c_str());
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    // AI(W906-PROD-S117) 20260926（Steven 團隊，St01）：Lot 分頁「Lot End」—— RULINGS_20260926 S117／S120-4。
    //   golden 本體（forms/fLotInfo.cpp 檔尾）：sbSECSLotEndClick（V912 uLotInfo.cpp:1387-1444）→ SetLotEnd（:2014-2379）。
    //   lotEnd.state：只讀（元件可見度照 golden 重算＋工單狀態＋「golden 會不會拒絕」的預覽），不寫檔、不跳框。
    //   lotEnd：兩段式。golden 按下去就做（只有 OEE 機台有 Yes/No 框，移植樹缺 Application->MessageBox，見 GATE W906-PROD-S117-OEEASK）；
    //     這裡多一段確認，理由：會寫 config.ini [Lot Info]（清空）、LotInfo log、ArmByLot*.dat（清 0），而且 RunInfo.bLotStart 變 false
    //     之後 START 會被擋（WebStart.cpp golden :5010 起），要重新 lot.start 才能再跑 —— 按錯不能復原。
    //   guard（golden 操作員按不到的情況，按 VCL 行為判斷；不是 golden 本體的判斷）：
    //     tab-hidden（tsLotID，W906_RefreshTabVisible）、panel-hidden（palSecsGem，FormShow :367-371／:407-412）、
    //     button-hidden／button-disabled（sbSECSLotEnd 的 Visible／Enabled；dfm 預設 true，移植樹沒有執行期寫者）。
    //   golden 本體自己的拒絕（SystemStart、MES1646 兩處、OEE）照 golden 在本體裡發生；這裡回 executed:false＋refused。
    //   ⚠ MES1646 是 ShowErrorMessage(kcode 0)：wb_serve 的 ForwardShowErrorMessage 會照 golden note.cpp:795-801 先 StopAllMotor、
    //     SoftStop／SoftStart／SystemStart=false（這時機台本來就沒在跑：前一個判斷已經擋掉 SystemStart），再發不阻塞的告警。
    if (op == "lotEnd.state" || op == "lotEnd") {
        fLotInfo->W906_RefreshTabVisible();                                     // tsLotID（只有 CC_PANTHER :392、CC_GIGAS :1035 會藏）
        fLotInfo->W906_LotEndDfmDefaults();
        const bool panelVis = fLotInfo->W906_LotEndPanelVisible();
        const bool tabVis   = fLotInfo->tsLotID->TabVisible;
        const bool btnVis   = fLotInfo->sbSECSLotEnd->Visible;
        const bool btnEn    = fLotInfo->sbSECSLotEnd->Enabled;
        GetTimeInfo();                                                          // SetLotEnd 用 SystemYear…（同 barcode.clearCount 的理由）
        const AnsiString pre = fLotInfo->W906_LotEndPrecheck();
        AnsiString logPreview;
        logPreview.sprintf("D:\\HT9045_Log\\LotInfo\\%04d\\LotInfo_%04d.csv", SystemYear, SystemYear);   // TMyStringList TByYear 的檔名規則（Public/MyStringList.cpp:752／:896）

        if (op == "lotEnd.state") {
            w.BeginObject();
            w.Key("executed").Bool(true);
            w.Key("op").String(op);
            w.Key("tabVisible").Bool(tabVis);
            w.Key("panelVisible").Bool(panelVis);
            w.Key("buttonVisible").Bool(btnVis);
            w.Key("buttonEnabled").Bool(btnEn);
            w.Key("caption").String(std::string(fLotInfo->sbSECSLotEnd->Caption.c_str()));
            w.Key("systemStart").Bool(SystemStart != 0);
            w.Key("precheck").String(std::string(pre.c_str()));
            w.Key("precheckText").String(LotEndReason(pre));
            WriteLotSnapshot(w, "lot");
            w.Key("goldenLine").String("V912 uLotInfo.dfm:314-349 palSecsGem／sbSECSLotEnd；uLotInfo.cpp:367-371／:407-412 FormShow；:1387-1444 sbSECSLotEndClick");
            w.EndObject();
            if (ok) *ok = true;
            return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
        }

        if (!tabVis)
            return SelectionGuard(op, "tab-hidden", "V912 uLotInfo.cpp:392／:1035 tsLotID->TabVisible",
                                  "golden 這台機台看不到 Lot 分頁（CC_PANTHER／CC_GIGAS），按不到 Lot End");
        if (!panelVis)
            return SelectionGuard(op, "panel-hidden", "V912 uLotInfo.cpp:367-371／:407-412 palSecsGem->Visible",
                                  "CC_MTI 且沒開 SECS/GEM、也沒開 ATP 鎖參數：golden 把 Lot ID／Lot Start／Lot End 那一整塊藏起來");
        if (!btnVis || !btnEn)
            return SelectionGuard(op, btnVis ? "button-disabled" : "button-hidden", "V912 uLotInfo.dfm:341-349 sbSECSLotEnd",
                                  "sbSECSLotEnd 看不到或反灰：golden 按不到");

        w.BeginObject();
        w.Key("op").String(op);
        if (!confirmed) {
            w.Key("executed").Bool(false);
            w.Key("needConfirm").Bool(true);
            w.Key("prompt").BeginArray();
            w.String("Lot End? Lot info in config.ini is cleared, a line is added to the Lot Info log, the by-lot arm counters are reset, "
                     "and START is blocked until the next Lot Start; the lot test summary files are written under D:\\HT9045_Log\\Summary_Lot and D:\\HT9045_Log\\Summary.");   // AI(W906-PROD-S117) 20260926: G-030 SaveTestSummary(1) now live (fLotInfo.cpp S117-SUMMARY)
            w.String("確定要 Lot End（結批）？config.ini 的 Lot 資訊會清空、Lot Info log 加一行、By Lot 的 Arm 計數清 0；"
                     "之後要重新 Lot Start 才能再按 START；測試總表寫到 D:\\HT9045_Log\\Summary_Lot 與 D:\\HT9045_Log\\Summary。");
            if (pre != "") {
                std::string p = std::string("⚠ 預覽：golden 這一刻會拒絕 —— ") + LotEndReason(pre);
                w.String(p);
            }
            w.EndArray();
            WriteLotSnapshot(w, "before");
            w.Key("precheck").String(std::string(pre.c_str()));
            w.Key("precheckText").String(LotEndReason(pre));
            WriteLotEndWrites(w, logPreview);
            w.Key("goldenLine").String("V912 uLotInfo.cpp:1387-1444 → :2014-2379（golden 只有 OEE 機台有 Yes/No 框 —— 這一段確認是移植樹加的，理由見 WebLotInfo.cpp）");
            w.EndObject();
            if (ok) *ok = true;
            return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
        }

        filerw::SessionBegin("");                                               // 收下面轉的 ELTodo
        WriteLotSnapshot(w, "before");
        const int n0 = fLotInfo->W906_SetLotEndCount;
        fLotInfo->W906_LotEndSkipped->Clear();
        fLotInfo->sbSECSLotEndClick(fLotInfo->sbSECSLotEnd);                    // golden 本體（forms/fLotInfo.cpp 檔尾）；Sender＝按鈕本身（Tag 0，dfm :341-349）
        const bool executed = (fLotInfo->W906_SetLotEndCount != n0);
        const AnsiString result = fLotInfo->W906_LotEndResult;
        w.Key("executed").Bool(executed);
        w.Key("confirmed").Bool(true);
        if (!executed) {
            w.Key("guard").String("refused");
            w.Key("refused").String(std::string(result.c_str()));
            w.Key("detail").String(LotEndReason(result));
        }
        WriteLotSnapshot(w, "after");
        if (executed) {
            const AnsiString real = W906_LotInfoLogFileName();
            WriteLotEndWrites(w, real != "" ? real : logPreview);
        }
        w.Key("gated").BeginArray();
        for (int i = 0; i < fLotInfo->W906_LotEndSkipped->Count; ++i) {
            const AnsiString s = fLotInfo->W906_LotEndSkipped->Strings[i];
            filerw::ELTodo(s.c_str());
            w.String(std::string(s.c_str()));
        }
        w.EndArray();
        w.Key("session").RawValue(filerw::SessionJson());
        w.Key("goldenLine").String("V912 uLotInfo.cpp:1387-1444 sbSECSLotEndClick → :2014-2379 SetLotEnd");
        w.EndObject();
        if (ok) *ok = true;
        std::printf("lotinfo.op lotEnd -> %s (%s; %d gated line(s) -> ELTodo)\n", executed ? "executed" : "refused",
                    result.c_str(), fLotInfo->W906_LotEndSkipped->Count);
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    if (op != "barcode.clearCount")
        return BadPayload("op 只有 barcode.clearCount／barcode.clearList／ocr.clearList／testerLog.get／selection.get／selection.save／lotEnd.state／lotEnd");

    GetTimeInfo();                                                              // golden 的 SystemYear… 由計時器每拍更新；這裡先更新一次
    W906_DoBarcodeCount();                                                      // 讓格子＝計數器（golden 每次改計數器都接著 DoBarcodeCount）
    AnsiString folder, file;
    ArchivePath(&folder, &file);

    w.BeginObject();
    w.Key("op").String(op);
    if (!confirmed) {
        w.Key("executed").Bool(false);
        w.Key("needConfirm").Bool(true);
        w.Key("prompt").BeginArray();
        w.String("Clear barcode count? (no .xls backup in this build: SGDToXLS is not ported)");
        w.String("確定要清除 Barcode 計數？（這個版本不會先存 .xls：SGDToXLS 未移植，清除前的數字不會留下紀錄）");
        w.EndArray();
        w.Key("before"); WriteGrid(w);
        WriteArchive(w, folder, file);
        w.Key("goldenLine").String("V912 uLotInfo.cpp:10168-10184（golden 沒有確認框 —— 這一段是移植樹因為 SGDToXLS 缺件加的）");
        w.EndObject();
        if (ok) *ok = true;
        return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
    }

    w.Key("before"); WriteGrid(w);
    fLotInfo->btClearBarcodeCountClick();                                       // golden 本體（forms/fLotInfo.cpp 檔尾）
    w.Key("executed").Bool(true);
    w.Key("confirmed").Bool(true);
    w.Key("after"); WriteGrid(w);
    w.Key("writes").BeginArray();
    w.String(std::string(folder.c_str()) + "（目錄；golden :10174 MyForceDirectories）");
    w.EndArray();
    WriteArchive(w, folder, file);
    w.Key("memory").String("iNeedBarcodeCount／iBarcodeDuplicate／iBarcodeErrorCount／iBarcodePassCount／iBarcodeAutoRetry 歸零（:10178-10182），"
                           "sgBarcode 重畫（:10183 DoBarcodeCount）");
    w.Key("goldenLine").String("V912 uLotInfo.cpp:10168-10184");
    w.EndObject();
    if (ok) *ok = true;

    std::printf("lotinfo.op barcode.clearCount -> executed (dir %s; .xls NOT written: SGDToXLS no-op)\n", folder.c_str());
    return w.Ok() ? w.Str() : std::string("{\"executed\":false,\"guard\":\"json-writer-misuse\"}");
}
